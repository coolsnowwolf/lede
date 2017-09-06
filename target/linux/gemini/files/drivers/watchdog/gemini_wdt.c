/*
 *  Watchdog driver for Cortina Systems Gemini SoC
 *
 *  Copyright (C) 2009 Paulius Zaleckas <paulius.zaleckas@teltonika.lt>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/io.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/watchdog.h>
#include <linux/slab.h>

#define GEMINI_WDCOUNTER	0x0
#define GEMINI_WDLOAD		0x4
#define GEMINI_WDRESTART	0x8

#define WDRESTART_MAGIC		0x5AB9

#define GEMINI_WDCR		0xC

#define WDCR_CLOCK_5MHZ		(1 << 4)
#define WDCR_SYS_RST		(1 << 1)
#define WDCR_ENABLE		(1 << 0)

#define WDT_CLOCK		5000000		/* 5 MHz */
#define WDT_DEFAULT_TIMEOUT	13
#define WDT_MAX_TIMEOUT		(0xFFFFFFFF / WDT_CLOCK)

/* status bits */
#define WDT_ACTIVE		0
#define WDT_OK_TO_CLOSE		1

static unsigned int timeout = WDT_DEFAULT_TIMEOUT;
static int nowayout = WATCHDOG_NOWAYOUT;

static DEFINE_SPINLOCK(gemini_wdt_lock);

static struct platform_device *gemini_wdt_dev;

struct gemini_wdt_struct {
	struct resource		*res;
	struct device		*dev;
	void __iomem		*base;
	unsigned long		status;
};

static struct watchdog_info gemini_wdt_info = {
	.identity	= "Gemini watchdog",
	.options	= WDIOF_MAGICCLOSE | WDIOF_KEEPALIVEPING |
			  WDIOF_SETTIMEOUT,
};

/* Disable the watchdog. */
static void gemini_wdt_stop(struct gemini_wdt_struct *gemini_wdt)
{
	spin_lock(&gemini_wdt_lock);

	__raw_writel(0, gemini_wdt->base + GEMINI_WDCR);

	clear_bit(WDT_ACTIVE, &gemini_wdt->status);

	spin_unlock(&gemini_wdt_lock);
}

/* Service the watchdog */
static void gemini_wdt_service(struct gemini_wdt_struct *gemini_wdt)
{
	__raw_writel(WDRESTART_MAGIC, gemini_wdt->base + GEMINI_WDRESTART);
}

/* Enable and reset the watchdog. */
static void gemini_wdt_start(struct gemini_wdt_struct *gemini_wdt)
{
	spin_lock(&gemini_wdt_lock);

	__raw_writel(timeout * WDT_CLOCK, gemini_wdt->base + GEMINI_WDLOAD);

	gemini_wdt_service(gemini_wdt);

	/* set clock before enabling */
	__raw_writel(WDCR_CLOCK_5MHZ | WDCR_SYS_RST,
			gemini_wdt->base + GEMINI_WDCR);

	__raw_writel(WDCR_CLOCK_5MHZ | WDCR_SYS_RST | WDCR_ENABLE,
			gemini_wdt->base + GEMINI_WDCR);

	set_bit(WDT_ACTIVE, &gemini_wdt->status);

	spin_unlock(&gemini_wdt_lock);
}

/* Watchdog device is opened, and watchdog starts running. */
static int gemini_wdt_open(struct inode *inode, struct file *file)
{
	struct gemini_wdt_struct *gemini_wdt = platform_get_drvdata(gemini_wdt_dev);

	if (test_bit(WDT_ACTIVE, &gemini_wdt->status))
		return -EBUSY;

	file->private_data = gemini_wdt;

	gemini_wdt_start(gemini_wdt);

	return nonseekable_open(inode, file);
}

/* Close the watchdog device. */
static int gemini_wdt_close(struct inode *inode, struct file *file)
{
	struct gemini_wdt_struct *gemini_wdt = file->private_data;

	/* Disable the watchdog if possible */
	if (test_bit(WDT_OK_TO_CLOSE, &gemini_wdt->status))
		gemini_wdt_stop(gemini_wdt);
	else
		dev_warn(gemini_wdt->dev, "Device closed unexpectedly - timer will not stop\n");

	return 0;
}

/* Handle commands from user-space. */
static long gemini_wdt_ioctl(struct file *file, unsigned int cmd,
			     unsigned long arg)
{
	struct gemini_wdt_struct *gemini_wdt = file->private_data;

	int value;

	switch (cmd) {
	case WDIOC_KEEPALIVE:
		gemini_wdt_service(gemini_wdt);
		return 0;

	case WDIOC_GETSUPPORT:
		return copy_to_user((struct watchdog_info *)arg, &gemini_wdt_info,
			sizeof(gemini_wdt_info)) ? -EFAULT : 0;

	case WDIOC_SETTIMEOUT:
		if (get_user(value, (int *)arg))
			return -EFAULT;

		if ((value < 1) || (value > WDT_MAX_TIMEOUT))
			return -EINVAL;

		timeout = value;

		/* restart wdt to use new timeout */
		gemini_wdt_stop(gemini_wdt);
		gemini_wdt_start(gemini_wdt);

		/* Fall through */
	case WDIOC_GETTIMEOUT:
		return put_user(timeout, (int *)arg);

	case WDIOC_GETTIMELEFT:
		value = __raw_readl(gemini_wdt->base + GEMINI_WDCOUNTER);
		return put_user(value / WDT_CLOCK, (int *)arg);

	default:
		return -ENOTTY;
	}
}

/* Refresh the watchdog whenever device is written to. */
static ssize_t gemini_wdt_write(struct file *file, const char *data,
						size_t len, loff_t *ppos)
{
	struct gemini_wdt_struct *gemini_wdt = file->private_data;

	if (len) {
		if (!nowayout) {
			size_t i;

			clear_bit(WDT_OK_TO_CLOSE, &gemini_wdt->status);
			for (i = 0; i != len; i++) {
				char c;

				if (get_user(c, data + i))
					return -EFAULT;
				if (c == 'V')
					set_bit(WDT_OK_TO_CLOSE,
						&gemini_wdt->status);
			}
		}
		gemini_wdt_service(gemini_wdt);
	}

	return len;
}

static const struct file_operations gemini_wdt_fops = {
	.owner		= THIS_MODULE,
	.llseek		= no_llseek,
	.unlocked_ioctl = gemini_wdt_ioctl,
	.open		= gemini_wdt_open,
	.release	= gemini_wdt_close,
	.write		= gemini_wdt_write,
};

static struct miscdevice gemini_wdt_miscdev = {
	.minor		= WATCHDOG_MINOR,
	.name		= "watchdog",
	.fops		= &gemini_wdt_fops,
};

static void gemini_wdt_shutdown(struct platform_device *pdev)
{
	struct gemini_wdt_struct *gemini_wdt = platform_get_drvdata(pdev);

	gemini_wdt_stop(gemini_wdt);
}

static int gemini_wdt_probe(struct platform_device *pdev)
{
	int ret;
	int res_size;
	struct resource *res;
	void __iomem *base;
	struct gemini_wdt_struct *gemini_wdt;
	unsigned int reg;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(&pdev->dev, "can't get device resources\n");
		return -ENODEV;
	}

	res_size = resource_size(res);
	if (!request_mem_region(res->start, res_size, res->name)) {
		dev_err(&pdev->dev, "can't allocate %d bytes at %d address\n",
			res_size, res->start);
		return -ENOMEM;
	}

	base = ioremap(res->start, res_size);
	if (!base) {
		dev_err(&pdev->dev, "ioremap failed\n");
		ret = -EIO;
		goto fail0;
	}

	gemini_wdt = kzalloc(sizeof(struct gemini_wdt_struct), GFP_KERNEL);
	if (!gemini_wdt) {
		dev_err(&pdev->dev, "can't allocate interface\n");
		ret = -ENOMEM;
		goto fail1;
	}

	/* Setup gemini_wdt driver structure */
	gemini_wdt->base = base;
	gemini_wdt->res = res;

	/* Set up platform driver data */
	platform_set_drvdata(pdev, gemini_wdt);
	gemini_wdt_dev = pdev;

	if (gemini_wdt_miscdev.parent) {
		ret = -EBUSY;
		goto fail2;
	}

	gemini_wdt_miscdev.parent = &pdev->dev;

	reg = __raw_readw(gemini_wdt->base + GEMINI_WDCR);
	if (reg & WDCR_ENABLE) {
		/* Watchdog was enabled by the bootloader, disable it. */
		reg &= ~(WDCR_ENABLE);
		__raw_writel(reg, gemini_wdt->base + GEMINI_WDCR);
	}

	ret = misc_register(&gemini_wdt_miscdev);
	if (ret)
		goto fail2;

	return 0;

fail2:
	platform_set_drvdata(pdev, NULL);
	kfree(gemini_wdt);
fail1:
	iounmap(base);
fail0:
	release_mem_region(res->start, res_size);

	return ret;
}

static int gemini_wdt_remove(struct platform_device *pdev)
{
	struct gemini_wdt_struct *gemini_wdt = platform_get_drvdata(pdev);

	platform_set_drvdata(pdev, NULL);
	misc_deregister(&gemini_wdt_miscdev);
	gemini_wdt_dev = NULL;
	iounmap(gemini_wdt->base);
	release_mem_region(gemini_wdt->res->start, resource_size(gemini_wdt->res));

	kfree(gemini_wdt);

	return 0;
}

#ifdef CONFIG_PM
static int gemini_wdt_suspend(struct platform_device *pdev, pm_message_t message)
{
	struct gemini_wdt_struct *gemini_wdt = platform_get_drvdata(pdev);
	unsigned int reg;

	reg = __raw_readw(gemini_wdt->base + GEMINI_WDCR);
	reg &= ~(WDCR_WDENABLE);
	__raw_writel(reg, gemini_wdt->base + GEMINI_WDCR);

	return 0;
}

static int gemini_wdt_resume(struct platform_device *pdev)
{
	struct gemini_wdt_struct *gemini_wdt = platform_get_drvdata(pdev);
	unsigned int reg;

	if (gemini_wdt->status) {
		reg = __raw_readw(gemini_wdt->base + GEMINI_WDCR);
		reg |= WDCR_WDENABLE;
		__raw_writel(reg, gemini_wdt->base + GEMINI_WDCR);
	}

	return 0;
}
#else
#define gemini_wdt_suspend	NULL
#define gemini_wdt_resume	NULL
#endif

static struct platform_driver gemini_wdt_driver = {
	.probe		= gemini_wdt_probe,
	.remove		= gemini_wdt_remove,
	.shutdown	= gemini_wdt_shutdown,
	.suspend	= gemini_wdt_suspend,
	.resume		= gemini_wdt_resume,
	.driver		= {
		.name	= "gemini-wdt",
		.owner	= THIS_MODULE,
	},
};

static int __init gemini_wdt_init(void)
{
	return platform_driver_probe(&gemini_wdt_driver, gemini_wdt_probe);
}

static void __exit gemini_wdt_exit(void)
{
	platform_driver_unregister(&gemini_wdt_driver);
}

module_init(gemini_wdt_init);
module_exit(gemini_wdt_exit);

module_param(timeout, uint, 0);
MODULE_PARM_DESC(timeout, "Watchdog timeout in seconds");

module_param(nowayout, int, 0);
MODULE_PARM_DESC(nowayout, "Watchdog cannot be stopped once started");

MODULE_AUTHOR("Paulius Zaleckas");
MODULE_DESCRIPTION("Watchdog driver for Gemini");
MODULE_LICENSE("GPL");
MODULE_ALIAS_MISCDEV(WATCHDOG_MINOR);
MODULE_ALIAS("platform:gemini-wdt");

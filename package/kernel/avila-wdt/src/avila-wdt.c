/*
 * avila-wdt.c 
 * Copyright (C) 2009 Felix Fietkau <nbd@nbd.name>
 *
 * based on:
 * drivers/char/watchdog/ixp4xx_wdt.c
 *
 * Watchdog driver for Intel IXP4xx network processors
 *
 * Author: Deepak Saxena <dsaxena@plexity.net>
 *
 * Copyright 2004 (c) MontaVista, Software, Inc.
 * Based on sa1100 driver, Copyright (C) 2000 Oleg Drokin <green@crimea.edu>
 *
 * This file is licensed under  the terms of the GNU General Public
 * License version 2. This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/jiffies.h>
#include <linux/timer.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/watchdog.h>
#include <linux/init.h>
#include <linux/bitops.h>
#include <linux/uaccess.h>
#include <mach/hardware.h>

static int nowayout = WATCHDOG_NOWAYOUT;
static int heartbeat = 20;	/* (secs) Default is 20 seconds */
static unsigned long wdt_status;
static atomic_t wdt_counter;
struct timer_list wdt_timer;

#define	WDT_IN_USE		0
#define	WDT_OK_TO_CLOSE		1
#define WDT_RUNNING		2

static void wdt_refresh(unsigned long data)
{
	if (test_bit(WDT_RUNNING, &wdt_status)) {
		if (atomic_dec_and_test(&wdt_counter)) {
			printk(KERN_WARNING "Avila watchdog expired, expect a reboot soon!\n");
			clear_bit(WDT_RUNNING, &wdt_status);
			return;
		}
	}

	/* strobe to the watchdog */
	gpio_line_set(14, IXP4XX_GPIO_HIGH);
	gpio_line_set(14, IXP4XX_GPIO_LOW);

	mod_timer(&wdt_timer, jiffies + msecs_to_jiffies(500));
}

static void wdt_enable(void)
{
	atomic_set(&wdt_counter, heartbeat * 2);

	/* Disable clock generator output on GPIO 14/15 */
	*IXP4XX_GPIO_GPCLKR &= ~(1 << 8);

	/* activate GPIO 14 out */
	gpio_line_config(14, IXP4XX_GPIO_OUT);
	gpio_line_set(14, IXP4XX_GPIO_LOW);

	if (!test_bit(WDT_RUNNING, &wdt_status))
		wdt_refresh(0);
	set_bit(WDT_RUNNING, &wdt_status);
}

static void wdt_disable(void)
{
	/* Re-enable clock generator output on GPIO 14/15 */
	*IXP4XX_GPIO_GPCLKR |= (1 << 8);
}

static int avila_wdt_open(struct inode *inode, struct file *file)
{
	if (test_and_set_bit(WDT_IN_USE, &wdt_status))
		return -EBUSY;

	clear_bit(WDT_OK_TO_CLOSE, &wdt_status);
	wdt_enable();
	return nonseekable_open(inode, file);
}

static ssize_t
avila_wdt_write(struct file *file, const char *data, size_t len, loff_t *ppos)
{
	if (len) {
		if (!nowayout) {
			size_t i;

			clear_bit(WDT_OK_TO_CLOSE, &wdt_status);

			for (i = 0; i != len; i++) {
				char c;

				if (get_user(c, data + i))
					return -EFAULT;
				if (c == 'V')
					set_bit(WDT_OK_TO_CLOSE, &wdt_status);
			}
		}
		wdt_enable();
	}
	return len;
}

static struct watchdog_info ident = {
	.options	= WDIOF_CARDRESET | WDIOF_MAGICCLOSE |
			  WDIOF_SETTIMEOUT | WDIOF_KEEPALIVEPING,
	.identity	= "Avila Watchdog",
};


static long avila_wdt_ioctl(struct file *file, unsigned int cmd,
							unsigned long arg)
{
	int ret = -ENOTTY;
	int time;

	switch (cmd) {
	case WDIOC_GETSUPPORT:
		ret = copy_to_user((struct watchdog_info *)arg, &ident,
				   sizeof(ident)) ? -EFAULT : 0;
		break;

	case WDIOC_GETSTATUS:
		ret = put_user(0, (int *)arg);
		break;

	case WDIOC_KEEPALIVE:
		wdt_enable();
		ret = 0;
		break;

	case WDIOC_SETTIMEOUT:
		ret = get_user(time, (int *)arg);
		if (ret)
			break;

		if (time <= 0 || time > 60) {
			ret = -EINVAL;
			break;
		}

		heartbeat = time;
		wdt_enable();
		/* Fall through */

	case WDIOC_GETTIMEOUT:
		ret = put_user(heartbeat, (int *)arg);
		break;
	}
	return ret;
}

static int avila_wdt_release(struct inode *inode, struct file *file)
{
	if (test_bit(WDT_OK_TO_CLOSE, &wdt_status))
		wdt_disable();
	else
		printk(KERN_CRIT "WATCHDOG: Device closed unexpectedly - "
					"timer will not stop\n");
	clear_bit(WDT_IN_USE, &wdt_status);
	clear_bit(WDT_OK_TO_CLOSE, &wdt_status);

	return 0;
}


static const struct file_operations avila_wdt_fops = {
	.owner		= THIS_MODULE,
	.llseek		= no_llseek,
	.write		= avila_wdt_write,
	.unlocked_ioctl	= avila_wdt_ioctl,
	.open		= avila_wdt_open,
	.release	= avila_wdt_release,
};

static struct miscdevice avila_wdt_miscdev = {
	.minor		= WATCHDOG_MINOR + 1,
	.name		= "avila_watchdog",
	.fops		= &avila_wdt_fops,
};

static int __init avila_wdt_init(void)
{
	int ret;

	init_timer(&wdt_timer);
	wdt_timer.expires = 0;
	wdt_timer.data = 0;
	wdt_timer.function = wdt_refresh;
	ret = misc_register(&avila_wdt_miscdev);
	if (ret == 0)
		printk(KERN_INFO "Avila Watchdog Timer: heartbeat %d sec\n",
			heartbeat);
	return ret;
}

static void __exit avila_wdt_exit(void)
{
	misc_deregister(&avila_wdt_miscdev);
	del_timer(&wdt_timer);
	wdt_disable();
}


module_init(avila_wdt_init);
module_exit(avila_wdt_exit);

MODULE_AUTHOR("Felix Fietkau <nbd@nbd.name>");
MODULE_DESCRIPTION("Gateworks Avila Hardware Watchdog");

module_param(heartbeat, int, 0);
MODULE_PARM_DESC(heartbeat, "Watchdog heartbeat in seconds (default 20s)");

module_param(nowayout, int, 0);
MODULE_PARM_DESC(nowayout, "Watchdog cannot be stopped once started");

MODULE_LICENSE("GPL");
MODULE_ALIAS_MISCDEV(WATCHDOG_MINOR);


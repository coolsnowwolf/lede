/*
 *  A low-level PATA driver to handle a Compact Flash connected on the
 *  Mikrotik's RouterBoard 153 board.
 *
 *  Copyright (C) 2007-2008 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This file was based on: drivers/ata/pata_ixp4xx_cf.c
 *	Copyright (C) 2006-07 Tower Technologies
 *	Author: Alessandro Zummo <a.zummo@towertech.it>
 *
 *  Also was based on the driver for Linux 2.4.xx published by Mikrotik for
 *  their RouterBoard 1xx and 5xx series devices. The original Mikrotik code
 *  seems not to have a license.
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/io.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>

#include <linux/libata.h>
#include <scsi/scsi_host.h>

#define DRV_NAME	"pata-rb153-cf"
#define DRV_VERSION	"0.5.0"
#define DRV_DESC	"PATA driver for RouterBOARD 153 Compact Flash"

#define RB153_CF_MAXPORTS	1
#define RB153_CF_IO_DELAY	100

#define RB153_CF_REG_CMD	0x0800
#define RB153_CF_REG_CTRL	0x080E
#define RB153_CF_REG_DATA	0x0C00

struct rb153_cf_info {
	void __iomem	*iobase;
	unsigned int	gpio_line;
	int		frozen;
	unsigned int	irq;
};

static inline void rb153_pata_finish_io(struct ata_port *ap)
{
	struct rb153_cf_info *info = ap->host->private_data;

	/* FIXME: Keep previous delay. If this is merely a fence then
	 * ata_sff_sync might be sufficient. */
	ata_sff_dma_pause(ap);
	ndelay(RB153_CF_IO_DELAY);

	irq_set_irq_type(info->irq, IRQ_TYPE_LEVEL_HIGH);
}

static void rb153_pata_exec_command(struct ata_port *ap,
					const struct ata_taskfile *tf)
{
	writeb(tf->command, ap->ioaddr.command_addr);
	rb153_pata_finish_io(ap);
}

static unsigned int rb153_pata_data_xfer(struct ata_device *adev,
					unsigned char *buf,
					unsigned int buflen,
					int write_data)
{
	void __iomem *ioaddr = adev->link->ap->ioaddr.data_addr;
	unsigned int t;

	t = buflen;
	if (write_data) {
		for (; t > 0; t--, buf++)
			writeb(*buf, ioaddr);
	} else {
		for (; t > 0; t--, buf++)
			*buf = readb(ioaddr);
	}

	rb153_pata_finish_io(adev->link->ap);
	return buflen;
}

static void rb153_pata_freeze(struct ata_port *ap)
{
	struct rb153_cf_info *info = ap->host->private_data;

	info->frozen = 1;
}

static void rb153_pata_thaw(struct ata_port *ap)
{
	struct rb153_cf_info *info = ap->host->private_data;

	info->frozen = 0;
}

static irqreturn_t rb153_pata_irq_handler(int irq, void *dev_instance)
{
	struct ata_host *ah = dev_instance;
	struct rb153_cf_info *info = ah->private_data;

	if (gpio_get_value(info->gpio_line)) {
		irq_set_irq_type(info->irq, IRQ_TYPE_LEVEL_LOW);
		if (!info->frozen)
			ata_sff_interrupt(irq, dev_instance);
	} else {
		irq_set_irq_type(info->irq, IRQ_TYPE_LEVEL_HIGH);
	}

	return IRQ_HANDLED;
}

static struct ata_port_operations rb153_pata_port_ops = {
	.inherits		= &ata_sff_port_ops,
	.sff_exec_command	= rb153_pata_exec_command,
	.sff_data_xfer		= rb153_pata_data_xfer,
	.freeze			= rb153_pata_freeze,
	.thaw			= rb153_pata_thaw,
};

static struct scsi_host_template rb153_pata_sht = {
	ATA_PIO_SHT(DRV_NAME),
};

static void rb153_pata_setup_port(struct ata_host *ah)
{
	struct rb153_cf_info *info = ah->private_data;
	struct ata_port *ap;

	ap = ah->ports[0];

	ap->ops		= &rb153_pata_port_ops;
	ap->pio_mask	= 0x1f; /* PIO4 */

	ap->ioaddr.cmd_addr	= info->iobase + RB153_CF_REG_CMD;
	ap->ioaddr.ctl_addr	= info->iobase + RB153_CF_REG_CTRL;
	ap->ioaddr.altstatus_addr = info->iobase + RB153_CF_REG_CTRL;

	ata_sff_std_ports(&ap->ioaddr);

	ap->ioaddr.data_addr = info->iobase + RB153_CF_REG_DATA;
}

static int rb153_pata_driver_probe(struct platform_device *pdev)
{
	unsigned int irq;
	int gpio;
	struct resource *res;
	struct ata_host *ah;
	struct rb153_cf_info *info;
	int ret;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(&pdev->dev, "no IOMEM resource found\n");
		return -EINVAL;
	}

	irq = platform_get_irq(pdev, 0);
	if (irq <= 0) {
		dev_err(&pdev->dev, "no IRQ resource found\n");
		return -ENOENT;
	}

	gpio = irq_to_gpio(irq);
	if (gpio < 0) {
		dev_err(&pdev->dev, "no GPIO found for irq%d\n", irq);
		return -ENOENT;
	}

	ret = gpio_request(gpio, DRV_NAME);
	if (ret) {
		dev_err(&pdev->dev, "GPIO request failed\n");
		return ret;
	}

	ah = ata_host_alloc(&pdev->dev, RB153_CF_MAXPORTS);
	if (!ah)
		return -ENOMEM;

	platform_set_drvdata(pdev, ah);

	info = devm_kzalloc(&pdev->dev, sizeof(*info), GFP_KERNEL);
	if (!info)
		return -ENOMEM;

	ah->private_data = info;
	info->gpio_line = gpio;
	info->irq = irq;

	info->iobase = devm_ioremap_nocache(&pdev->dev, res->start,
				res->end - res->start + 1);
	if (!info->iobase)
		return -ENOMEM;

	ret = gpio_direction_input(gpio);
	if (ret) {
		dev_err(&pdev->dev, "unable to set GPIO direction, err=%d\n",
				ret);
		goto err_free_gpio;
	}

	rb153_pata_setup_port(ah);

	ret = ata_host_activate(ah, irq, rb153_pata_irq_handler,
				IRQF_TRIGGER_LOW, &rb153_pata_sht);
	if (ret)
		goto err_free_gpio;

	return 0;

err_free_gpio:
	gpio_free(gpio);

	return ret;
}

static int rb153_pata_driver_remove(struct platform_device *pdev)
{
	struct ata_host *ah = platform_get_drvdata(pdev);
	struct rb153_cf_info *info = ah->private_data;

	ata_host_detach(ah);
	gpio_free(info->gpio_line);

	return 0;
}

static struct platform_driver rb153_pata_platform_driver = {
	.probe		= rb153_pata_driver_probe,
	.remove		= rb153_pata_driver_remove,
	.driver	 = {
		.name   = DRV_NAME,
		.owner  = THIS_MODULE,
	},
};

/* ------------------------------------------------------------------------ */

#define DRV_INFO DRV_DESC " version " DRV_VERSION

static int __init rb153_pata_module_init(void)
{
	printk(KERN_INFO DRV_INFO "\n");

	return platform_driver_register(&rb153_pata_platform_driver);
}

static void __exit rb153_pata_module_exit(void)
{
	platform_driver_unregister(&rb153_pata_platform_driver);
}

MODULE_AUTHOR("Gabor Juhos <juhosg@openwrt.org>");
MODULE_DESCRIPTION(DRV_DESC);
MODULE_VERSION(DRV_VERSION);
MODULE_LICENSE("GPL v2");

module_init(rb153_pata_module_init);
module_exit(rb153_pata_module_exit);

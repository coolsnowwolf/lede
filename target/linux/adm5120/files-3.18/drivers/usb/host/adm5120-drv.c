/*
 * ADM5120 HCD (Host Controller Driver) for USB
 *
 * Copyright (C) 2007-2008 Gabor Juhos <juhosg@openwrt.org>
 *
 * This file was derived from: drivers/usb/host/ohci-au1xxx.c
 *   (C) Copyright 1999 Roman Weissgaerber <weissg@vienna.at>
 *   (C) Copyright 2000-2002 David Brownell <dbrownell@users.sourceforge.net>
 *   (C) Copyright 2002 Hewlett-Packard Company
 *
 *   Written by Christopher Hoover <ch@hpl.hp.com>
 *   Based on fragments of previous driver by Russell King et al.
 *
 *   Modified for LH7A404 from ahcd-sa1111.c
 *    by Durgesh Pattamatta <pattamattad@sharpsec.com>
 *   Modified for AMD Alchemy Au1xxx
 *    by Matt Porter <mporter@kernel.crashing.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 */

#include <linux/platform_device.h>
#include <linux/signal.h>

#include <asm/bootinfo.h>
#include <asm/mach-adm5120/adm5120_defs.h>

#ifdef DEBUG
#define HCD_DBG(f, a...)	printk(KERN_DEBUG "%s: " f, hcd_name, ## a)
#else
#define HCD_DBG(f, a...)	do {} while (0)
#endif
#define HCD_ERR(f, a...)	printk(KERN_ERR "%s: " f, hcd_name, ## a)
#define HCD_INFO(f, a...)	printk(KERN_INFO "%s: " f, hcd_name, ## a)

/*-------------------------------------------------------------------------*/

static int admhc_adm5120_probe(const struct hc_driver *driver,
		  struct platform_device *dev)
{
	int retval;
	struct usb_hcd *hcd;
	int irq;
	struct resource *regs;

	/* sanity checks */
	regs = platform_get_resource(dev, IORESOURCE_MEM, 0);
	if (!regs) {
		HCD_DBG("no IOMEM resource found\n");
		return -ENODEV;
	}

	irq = platform_get_irq(dev, 0);
	if (irq < 0) {
		HCD_DBG("no IRQ resource found\n");
		return -ENODEV;
	}

	hcd = usb_create_hcd(driver, &dev->dev, "ADM5120");
	if (!hcd)
		return -ENOMEM;

	hcd->rsrc_start = regs->start;
	hcd->rsrc_len = regs->end - regs->start + 1;

	if (!request_mem_region(hcd->rsrc_start, hcd->rsrc_len, hcd_name)) {
		HCD_DBG("request_mem_region failed\n");
		retval = -EBUSY;
		goto err_dev;
	}

	hcd->regs = ioremap(hcd->rsrc_start, hcd->rsrc_len);
	if (!hcd->regs) {
		HCD_DBG("ioremap failed\n");
		retval = -ENOMEM;
		goto err_mem;
	}

	admhc_hcd_init(hcd_to_admhcd(hcd));

	retval = usb_add_hcd(hcd, irq, 0);
	if (retval)
		goto err_io;

	return 0;

err_io:
	iounmap(hcd->regs);
err_mem:
	release_mem_region(hcd->rsrc_start, hcd->rsrc_len);
err_dev:
	usb_put_hcd(hcd);
	return retval;
}


/* may be called without controller electrically present */
/* may be called with controller, bus, and devices active */

static void admhc_adm5120_remove(struct usb_hcd *hcd,
		struct platform_device *dev)
{
	usb_remove_hcd(hcd);
	iounmap(hcd->regs);
	release_mem_region(hcd->rsrc_start, hcd->rsrc_len);
	usb_put_hcd(hcd);
}

static int admhc_adm5120_start(struct usb_hcd *hcd)
{
	struct admhcd	*ahcd = hcd_to_admhcd(hcd);
	int		ret;

	ret = admhc_init(ahcd);
	if (ret < 0) {
		HCD_ERR("unable to init %s\n", hcd->self.bus_name);
		goto err;
	}

	ret = admhc_run(ahcd);
	if (ret < 0) {
		HCD_ERR("unable to run %s\n", hcd->self.bus_name);
		goto err_stop;
	}

	return 0;

err_stop:
	admhc_stop(hcd);
err:
	return ret;
}

static const struct hc_driver adm5120_hc_driver = {
	.description =		hcd_name,
	.product_desc =		"ADM5120 built-in USB 1.1 Host Controller",
	.hcd_priv_size =	sizeof(struct admhcd),

	/*
	 * generic hardware linkage
	 */
	.irq =			admhc_irq,
	.flags =		HCD_USB11 | HCD_MEMORY,

	/*
	 * basic lifecycle operations
	 */
	.start =		admhc_adm5120_start,
	.stop =			admhc_stop,
	.shutdown =		admhc_shutdown,

	/*
	 * managing i/o requests and associated device resources
	 */
	.urb_enqueue =		admhc_urb_enqueue,
	.urb_dequeue =		admhc_urb_dequeue,
	.endpoint_disable =	admhc_endpoint_disable,

	/*
	 * scheduling support
	 */
	.get_frame_number =	admhc_get_frame_number,

	/*
	 * root hub support
	 */
	.hub_status_data =	admhc_hub_status_data,
	.hub_control =		admhc_hub_control,
#ifdef CONFIG_PM
	.bus_suspend =		admhc_bus_suspend,
	.bus_resume =		admhc_bus_resume,
#endif
	.start_port_reset =	admhc_start_port_reset,
};

static int usb_hcd_adm5120_probe(struct platform_device *pdev)
{
	int ret;

	ret = admhc_adm5120_probe(&adm5120_hc_driver, pdev);

	return ret;
}

static int usb_hcd_adm5120_remove(struct platform_device *pdev)
{
	struct usb_hcd *hcd = platform_get_drvdata(pdev);

	admhc_adm5120_remove(hcd, pdev);

	return 0;
}

#ifdef CONFIG_PM
/* TODO */
static int usb_hcd_adm5120_suspend(struct platform_device *dev)
{
	struct usb_hcd *hcd = platform_get_drvdata(dev);

	return 0;
}

static int usb_hcd_adm5120_resume(struct platform_device *dev)
{
	struct usb_hcd *hcd = platform_get_drvdata(dev);

	return 0;
}
#else
#define usb_hcd_adm5120_suspend	NULL
#define usb_hcd_adm5120_resume	NULL
#endif /* CONFIG_PM */

static struct platform_driver usb_hcd_adm5120_driver = {
	.probe		= usb_hcd_adm5120_probe,
	.remove		= usb_hcd_adm5120_remove,
	.shutdown	= usb_hcd_platform_shutdown,
	.suspend	= usb_hcd_adm5120_suspend,
	.resume		= usb_hcd_adm5120_resume,
	.driver		= {
		.name	= "adm5120-hcd",
		.owner	= THIS_MODULE,
	},
};


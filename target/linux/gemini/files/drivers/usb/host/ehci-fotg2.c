/*
 *  Gemini EHCI Host Controller driver
 *
 *  Copyright (C) 2014 Roman Yeryomin <roman@advem.lv>
 *  Copyright (C) 2012 Tobias Waldvogel
 *  based on GPLd code from Sony Computer Entertainment Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 */
#include <linux/kernel.h>
#include <linux/hrtimer.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/usb.h>
#include <linux/usb/hcd.h>
#include <linux/usb/ehci_pdriver.h>

#include <mach/hardware.h>
#include <mach/global_reg.h>

#include "ehci.h"

#define DRV_NAME			"ehci-fotg2"

#define HCD_MISC			0x40

#define OTGC_SCR			0x80
#define OTGC_INT_STS			0x84
#define OTGC_INT_EN			0x88

#define GLOBAL_ISR			0xC0
#define GLOBAL_ICR			0xC4

#define GLOBAL_INT_POLARITY		(1 << 3)
#define GLOBAL_INT_MASK_HC		(1 << 2)
#define GLOBAL_INT_MASK_OTG		(1 << 1)
#define GLOBAL_INT_MASK_DEV		(1 << 0)

#define OTGC_SCR_ID			(1 << 21)
#define OTGC_SCR_CROLE			(1 << 20)
#define OTGC_SCR_VBUS_VLD		(1 << 19)
#define OTGC_SCR_A_SRP_RESP_TYPE	(1 << 8)
#define OTGC_SCR_A_SRP_DET_EN		(1 << 7)
#define OTGC_SCR_A_SET_B_HNP_EN		(1 << 6)
#define OTGC_SCR_A_BUS_DROP		(1 << 5)
#define OTGC_SCR_A_BUS_REQ		(1 << 4)

#define OTGC_INT_APLGRMV		(1 << 12)
#define OTGC_INT_BPLGRMV		(1 << 11)
#define OTGC_INT_OVC			(1 << 10)
#define OTGC_INT_IDCHG			(1 << 9)
#define OTGC_INT_RLCHG			(1 << 8)
#define OTGC_INT_AVBUSERR		(1 << 5)
#define OTGC_INT_ASRPDET		(1 << 4)
#define OTGC_INT_BSRPDN			(1 << 0)

#define OTGC_INT_A_TYPE	(		\
 		OTGC_INT_ASRPDET |	\
		OTGC_INT_AVBUSERR |	\
		OTGC_INT_OVC |		\
		OTGC_INT_RLCHG |	\
		OTGC_INT_IDCHG |	\
		OTGC_INT_APLGRMV	\
	)
#define OTGC_INT_B_TYPE	(		\
		OTGC_INT_AVBUSERR |	\
		OTGC_INT_OVC |		\
		OTGC_INT_RLCHG |	\
		OTGC_INT_IDCHG		\
	)


static void fotg2_otg_init(struct usb_hcd *hcd)
{
	u32 val;

	writel(GLOBAL_INT_POLARITY | GLOBAL_INT_MASK_HC |
	       GLOBAL_INT_MASK_OTG | GLOBAL_INT_MASK_DEV,
		hcd->regs + GLOBAL_ICR);

	val = readl(hcd->regs + OTGC_SCR);
	val &= ~(OTGC_SCR_A_SRP_RESP_TYPE | OTGC_SCR_A_SRP_DET_EN |
		 OTGC_SCR_A_BUS_DROP      | OTGC_SCR_A_SET_B_HNP_EN);
	val |= OTGC_SCR_A_BUS_REQ;
	writel(val, hcd->regs + OTGC_SCR);

	writel(OTGC_INT_A_TYPE, hcd->regs + OTGC_INT_EN);

	/* setup MISC register, fixes timing problems */
	val = readl(hcd->regs + HCD_MISC);
	val |= 0xD;
	writel(val, hcd->regs + HCD_MISC);

	writel(~0, hcd->regs + GLOBAL_ISR);
	writel(~0, hcd->regs + OTGC_INT_STS);
}

static int fotg2_ehci_reset(struct usb_hcd *hcd)
{
	int retval;

	retval = ehci_setup(hcd);
	if (retval)
		return retval;

	writel(GLOBAL_INT_POLARITY, hcd->regs + GLOBAL_ICR);
	return 0;
}

static const struct hc_driver fotg2_ehci_hc_driver = {
	.description		= hcd_name,
	.product_desc		= "FOTG2 EHCI Host Controller",
	.hcd_priv_size		= sizeof(struct ehci_hcd),
	.irq			= ehci_irq,
	.flags			= HCD_MEMORY | HCD_USB2,
	.reset			= fotg2_ehci_reset,
	.start			= ehci_run,
	.stop			= ehci_stop,
	.shutdown		= ehci_shutdown,
	.urb_enqueue		= ehci_urb_enqueue,
	.urb_dequeue		= ehci_urb_dequeue,
	.endpoint_disable	= ehci_endpoint_disable,
	.endpoint_reset		= ehci_endpoint_reset,
	.get_frame_number	= ehci_get_frame,
	.hub_status_data	= ehci_hub_status_data,
	.hub_control		= ehci_hub_control,
#if defined(CONFIG_PM)
	.bus_suspend		= ehci_bus_suspend,
	.bus_resume		= ehci_bus_resume,
#endif
	.relinquish_port	= ehci_relinquish_port,
	.port_handed_over	= ehci_port_handed_over,

	.clear_tt_buffer_complete = ehci_clear_tt_buffer_complete,
};

static irqreturn_t fotg2_ehci_irq(int irq, void *data)
{
	struct usb_hcd *hcd = data;
	u32 icr, sts;
	irqreturn_t retval;

	icr = readl(hcd->regs + GLOBAL_ICR);
	writel(GLOBAL_INT_POLARITY | GLOBAL_INT_MASK_HC |
	       GLOBAL_INT_MASK_OTG | GLOBAL_INT_MASK_DEV,
		hcd->regs + GLOBAL_ICR);

	retval = IRQ_NONE;

	sts = ~icr;
	sts &= GLOBAL_INT_MASK_HC | GLOBAL_INT_MASK_OTG | GLOBAL_INT_MASK_DEV;
	sts &= readl(hcd->regs + GLOBAL_ISR);
	writel(sts, hcd->regs + GLOBAL_ISR);

	if (unlikely(sts & GLOBAL_INT_MASK_DEV)) {
		ehci_warn(hcd_to_ehci(hcd),
			"Received unexpected irq for device role\n");
		retval = IRQ_HANDLED;
	}

	if (unlikely(sts & GLOBAL_INT_MASK_OTG)) {
		u32	otg_sts;

		otg_sts = readl(hcd->regs + OTGC_INT_STS);
		writel(otg_sts, hcd->regs + OTGC_INT_STS);

		ehci_warn(hcd_to_ehci(hcd),
			"Received unexpected irq for OTG management\n");
		retval = IRQ_HANDLED;
	}

	if (sts & GLOBAL_INT_MASK_HC) {
		retval = IRQ_NONE;
	}

	writel(icr, hcd->regs + GLOBAL_ICR);
	return retval;
}

static int fotg2_ehci_probe(struct platform_device *pdev)
{
	struct usb_hcd *hcd;
	struct resource *res;
	int irq , err;

	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
		pr_err("no irq provided");
		return irq;
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		pr_err("no memory resource provided");
		return -ENXIO;
	}

	hcd = usb_create_hcd(&fotg2_ehci_hc_driver, &pdev->dev,
			     dev_name(&pdev->dev));
	if (!hcd)
		return -ENOMEM;

	hcd->rsrc_start = res->start;
	hcd->rsrc_len = resource_size(res);

	hcd->regs = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(hcd->regs)) {
		err = -ENOMEM;
		goto err_put_hcd;
	}

	hcd->has_tt = 1;
	hcd_to_ehci(hcd)->caps = hcd->regs;

	fotg2_otg_init(hcd);

	err = request_irq(irq, &fotg2_ehci_irq, IRQF_SHARED, "fotg2", hcd);
	if (err)
		goto err_put_hcd;

	err = usb_add_hcd(hcd, irq, IRQF_SHARED);
	if (err)
		goto err_put_hcd;

	platform_set_drvdata(pdev, hcd);
	return 0;

err_put_hcd:
	usb_put_hcd(hcd);
	return err;
}

static int fotg2_ehci_remove(struct platform_device *pdev)
{
	struct usb_hcd *hcd = platform_get_drvdata(pdev);

	writel(GLOBAL_INT_POLARITY | GLOBAL_INT_MASK_HC |
	       GLOBAL_INT_MASK_OTG | GLOBAL_INT_MASK_DEV,
		hcd->regs + GLOBAL_ICR);

	free_irq(hcd->irq, hcd);
	usb_remove_hcd(hcd);
	usb_put_hcd(hcd);
	platform_set_drvdata(pdev, NULL);

	return 0;
}

MODULE_ALIAS("platform:" DRV_NAME);

static struct platform_driver ehci_fotg2_driver = {
	.probe		= fotg2_ehci_probe,
	.remove		= fotg2_ehci_remove,
	.driver.name	= DRV_NAME,
};

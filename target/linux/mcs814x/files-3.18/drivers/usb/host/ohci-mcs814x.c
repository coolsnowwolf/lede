/*
 * OHCI HCD (Host Controller Driver) for USB.
 *
 * (C) Copyright 1999 Roman Weissgaerber <weissg@vienna.at>
 * (C) Copyright 2000-2002 David Brownell <dbrownell@users.sourceforge.net>
 * (C) Copyright 2002 Hewlett-Packard Company
 *
 * Bus Glue for Moschip MCS814x.
 *
 * Written by Christopher Hoover <ch@hpl.hp.com>
 * Based on fragments of previous driver by Russell King et al.
 *
 * Modified for LH7A404 from ohci-sa1111.c
 *  by Durgesh Pattamatta <pattamattad@sharpsec.com>
 *
 * Modified for pxa27x from ohci-lh7a404.c
 *  by Nick Bane <nick@cecomputing.co.uk> 26-8-2004
 *
 * Modified for mcs814x from ohci-mcs814x.c
 *  by Lennert Buytenhek <buytenh@wantstofly.org> 28-2-2006
 *  Based on an earlier driver by Ray Lehtiniemi
 *
 * This file is licenced under the GPL.
 */

#include <linux/device.h>
#include <linux/signal.h>
#include <linux/platform_device.h>
#include <linux/of.h>

static int usb_hcd_mcs814x_probe(const struct hc_driver *driver,
			 struct platform_device *pdev)
{
	int retval;
	struct usb_hcd *hcd;

	if (pdev->resource[1].flags != IORESOURCE_IRQ) {
		pr_debug("resource[1] is not IORESOURCE_IRQ");
		return -ENOMEM;
	}

	pdev->dev.coherent_dma_mask = DMA_BIT_MASK(32);
	pdev->dev.dma_mask = &pdev->dev.coherent_dma_mask;

	hcd = usb_create_hcd(driver, &pdev->dev, "mcs814x");
	if (hcd == NULL)
		return -ENOMEM;

	hcd->rsrc_start = pdev->resource[0].start;
	hcd->rsrc_len = pdev->resource[0].end - pdev->resource[0].start + 1;
	if (!request_mem_region(hcd->rsrc_start, hcd->rsrc_len, hcd_name)) {
		usb_put_hcd(hcd);
		retval = -EBUSY;
		goto err1;
	}

	hcd->regs = ioremap(hcd->rsrc_start, hcd->rsrc_len);
	if (hcd->regs == NULL) {
		pr_debug("ioremap failed");
		retval = -ENOMEM;
		goto err2;
	}

	ohci_hcd_init(hcd_to_ohci(hcd));

	retval = usb_add_hcd(hcd, pdev->resource[1].start, IRQF_DISABLED);
	if (retval == 0)
		return retval;

	iounmap(hcd->regs);
err2:
	release_mem_region(hcd->rsrc_start, hcd->rsrc_len);
err1:
	usb_put_hcd(hcd);

	return retval;
}

static void usb_hcd_mcs814x_remove(struct usb_hcd *hcd,
			struct platform_device *pdev)
{
	usb_remove_hcd(hcd);
	iounmap(hcd->regs);
	release_mem_region(hcd->rsrc_start, hcd->rsrc_len);
	usb_put_hcd(hcd);
}

static int ohci_mcs814x_start(struct usb_hcd *hcd)
{
	struct ohci_hcd *ohci = hcd_to_ohci(hcd);
	int ret;

	ret = ohci_init(ohci);
	if (ret < 0)
		return ret;

	ret = ohci_run(ohci);
	if (ret < 0) {
		ohci_err(ohci, "can't start %s", hcd->self.bus_name);
		ohci_stop(hcd);
		return ret;
	}

	return 0;
}

static struct hc_driver ohci_mcs814x_hc_driver = {
	.description		= hcd_name,
	.product_desc		= "MCS814X OHCI",
	.hcd_priv_size		= sizeof(struct ohci_hcd),
	.irq			= ohci_irq,
	.flags			= HCD_USB11 | HCD_MEMORY,
	.start			= ohci_mcs814x_start,
	.stop			= ohci_stop,
	.shutdown		= ohci_shutdown,
	.urb_enqueue		= ohci_urb_enqueue,
	.urb_dequeue		= ohci_urb_dequeue,
	.endpoint_disable	= ohci_endpoint_disable,
	.get_frame_number	= ohci_get_frame,
	.hub_status_data	= ohci_hub_status_data,
	.hub_control		= ohci_hub_control,
#ifdef CONFIG_PM
	.bus_suspend		= ohci_bus_suspend,
	.bus_resume		= ohci_bus_resume,
#endif
	.start_port_reset	= ohci_start_port_reset,
};

extern int usb_disabled(void);

static int ohci_hcd_mcs814x_drv_probe(struct platform_device *pdev)
{
	int ret;

	ret = -ENODEV;
	if (!usb_disabled())
		ret = usb_hcd_mcs814x_probe(&ohci_mcs814x_hc_driver, pdev);

	return ret;
}

static int ohci_hcd_mcs814x_drv_remove(struct platform_device *pdev)
{
	struct usb_hcd *hcd = platform_get_drvdata(pdev);

	usb_hcd_mcs814x_remove(hcd, pdev);

	return 0;
}

#ifdef CONFIG_PM
static int ohci_hcd_mcs814x_drv_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct usb_hcd *hcd = platform_get_drvdata(pdev);
	struct ohci_hcd *ohci = hcd_to_ohci(hcd);

	if (time_before(jiffies, ohci->next_statechange))
		msleep(5);
	ohci->next_statechange = jiffies;

	hcd->state = HC_STATE_SUSPENDED;

	return 0;
}

static int ohci_hcd_mcs814x_drv_resume(struct platform_device *pdev)
{
	struct usb_hcd *hcd = platform_get_drvdata(pdev);
	struct ohci_hcd *ohci = hcd_to_ohci(hcd);
	int status;

	if (time_before(jiffies, ohci->next_statechange))
		msleep(5);
	ohci->next_statechange = jiffies;

	ohci_finish_controller_resume(hcd);
	return 0;
}
#endif

static const struct of_device_id mcs814x_ohci_id[] = {
	{ .compatible = "moschip,mcs814x-ohci" },
	{ .compatible = "ohci-le" },
	{ /* sentinel */ },
};

static struct platform_driver ohci_hcd_mcs814x_driver = {
	.probe		= ohci_hcd_mcs814x_drv_probe,
	.remove		= ohci_hcd_mcs814x_drv_remove,
	.shutdown	= usb_hcd_platform_shutdown,
#ifdef CONFIG_PM
	.suspend	= ohci_hcd_mcs814x_drv_suspend,
	.resume		= ohci_hcd_mcs814x_drv_resume,
#endif
	.driver		= {
		.name	= "mcs814x-ohci",
		.owner	= THIS_MODULE,
		.of_match_table = mcs814x_ohci_id,
	},
};

MODULE_ALIAS("platform:mcs814x-ohci");

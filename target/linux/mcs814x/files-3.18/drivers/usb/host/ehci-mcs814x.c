/*
 * MCS814X EHCI Host Controller Driver
 *
 * Based on "ehci-fsl.c" by Randy Vinson <rvinson@mvista.com>
 *
 * 2007 (c) MontaVista Software, Inc. This file is licensed under
 * the terms of the GNU General Public License version 2. This program
 * is licensed "as is" without any warranty of any kind, whether express
 * or implied.
 */

#include <linux/platform_device.h>
#include <linux/of.h>

#define MCS814X_EHCI_CAPS_OFFSET	0x68

static int mcs814x_ehci_init(struct usb_hcd *hcd)
{
	struct ehci_hcd *ehci = hcd_to_ehci(hcd);
	int retval = 0;

	ehci->caps = hcd->regs + MCS814X_EHCI_CAPS_OFFSET;
	ehci->regs = hcd->regs
		+ HC_LENGTH(ehci, ehci_readl(ehci, &ehci->caps->hc_capbase));
	ehci->hcs_params = ehci_readl(ehci, &ehci->caps->hcs_params);
	ehci_reset(ehci);

	retval = ehci_init(hcd);
	if (retval) {
		pr_err("ehci_init failed\n");
		return retval;
	}

	return retval;
}

static const struct hc_driver mcs814x_ehci_hc_driver = {
	.description		= hcd_name,
	.product_desc		= "MCS814X EHCI Host Controller",
	.hcd_priv_size		= sizeof(struct ehci_hcd),
	.irq			= ehci_irq,
	.flags			= HCD_MEMORY | HCD_USB2,
	.reset			= mcs814x_ehci_init,
	.start			= ehci_run,
	.stop			= ehci_stop,
	.shutdown		= ehci_shutdown,
	.urb_enqueue		= ehci_urb_enqueue,
	.urb_dequeue		= ehci_urb_dequeue,
	.endpoint_disable	= ehci_endpoint_disable,
	.get_frame_number	= ehci_get_frame,
	.hub_status_data	= ehci_hub_status_data,
	.hub_control		= ehci_hub_control,
#if defined(CONFIG_PM)
	.bus_suspend		= ehci_bus_suspend,
	.bus_resume		= ehci_bus_resume,
#endif
	.relinquish_port	= ehci_relinquish_port,
	.port_handed_over	= ehci_port_handed_over,

	.clear_tt_buffer_complete	= ehci_clear_tt_buffer_complete,
};

static int mcs814x_ehci_probe(struct platform_device *pdev)
{
	struct usb_hcd *hcd;
	const struct hc_driver *driver = &mcs814x_ehci_hc_driver;
	struct resource *res;
	int irq;
	int retval;

	if (usb_disabled())
		return -ENODEV;

	res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	if (!res) {
		dev_err(&pdev->dev,
			"Found HC with no IRQ. Check %s setup!\n",
			dev_name(&pdev->dev));
		return -ENODEV;
	}
	irq = res->start;

	pdev->dev.coherent_dma_mask = DMA_BIT_MASK(32);
	pdev->dev.dma_mask = &pdev->dev.coherent_dma_mask;

	hcd = usb_create_hcd(driver, &pdev->dev, dev_name(&pdev->dev));
	if (!hcd) {
		retval = -ENOMEM;
		goto fail_create_hcd;
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(&pdev->dev,
			"Found HC with no register addr. Check %s setup!\n",
			dev_name(&pdev->dev));
		retval = -ENODEV;
		goto fail_request_resource;
	}
	hcd->rsrc_start = res->start;
	hcd->rsrc_len = resource_size(res);

	if (!request_mem_region(hcd->rsrc_start, hcd->rsrc_len,
				driver->description)) {
		dev_dbg(&pdev->dev, "controller already in use\n");
		retval = -EBUSY;
		goto fail_request_resource;
	}

	hcd->regs = ioremap_nocache(hcd->rsrc_start, hcd->rsrc_len);
	if (hcd->regs == NULL) {
		dev_dbg(&pdev->dev, "error mapping memory\n");
		retval = -EFAULT;
		goto fail_ioremap;
	}

	retval = usb_add_hcd(hcd, irq, IRQF_SHARED);
	if (retval)
		goto fail_add_hcd;

	dev_info(&pdev->dev, "added MCS814X EHCI driver\n");

	return retval;

fail_add_hcd:
	iounmap(hcd->regs);
fail_ioremap:
	release_mem_region(hcd->rsrc_start, hcd->rsrc_len);
fail_request_resource:
	usb_put_hcd(hcd);
fail_create_hcd:
	dev_err(&pdev->dev, "init %s fail, %d\n", dev_name(&pdev->dev), retval);
	return retval;
}

static int mcs814x_ehci_remove(struct platform_device *pdev)
{
	struct usb_hcd *hcd = platform_get_drvdata(pdev);

	usb_remove_hcd(hcd);
	iounmap(hcd->regs);
	release_mem_region(hcd->rsrc_start, hcd->rsrc_len);
	usb_put_hcd(hcd);

	return 0;
}

MODULE_ALIAS("platform:mcs814x-ehci");

static const struct of_device_id mcs814x_ehci_id[] = {
	{ .compatible = "moschip,mcs814x-ehci" },
	{ .compatible = "usb-ehci" },
	{ /* sentinel */ },
};

static struct platform_driver mcs814x_ehci_driver = {
	.probe = mcs814x_ehci_probe,
	.remove = mcs814x_ehci_remove,
	.driver = {
		.name = "mcs814x-ehci",
		.of_match_table = mcs814x_ehci_id,
	},
};

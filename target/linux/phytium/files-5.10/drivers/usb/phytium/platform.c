// SPDX-License-Identifier: GPL-3.0

#include <linux/module.h>
#include <linux/kernel.h>
//#include <linux/usb/hcd.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/of_platform.h>
#include <linux/dma-mapping.h>
#include <linux/usb/otg.h>
#include <linux/acpi.h>
#include "core.h"
#include "hw-regs.h"

#define PHYTIUM_OTG_USB_LOADED  3
#define USB2_2_BASE_ADDRESS 0x31800000

static const struct of_device_id phytium_otg_of_match[] = {
	{
		.compatible = "phytium,usb2",
	},
	{},
};

#ifdef CONFIG_ACPI
static const struct acpi_device_id phytium_otg_acpi_match[] = {
	{ "PHYT0037", 0 },
	{ }
};
#endif

static int phytium_get_dr_mode(struct phytium_cusb *config)
{
	config->dr_mode = usb_get_dr_mode(config->dev);
	if (config->dr_mode == USB_DR_MODE_UNKNOWN)
		config->dr_mode = USB_DR_MODE_PERIPHERAL;

	return 0;
}

static irqreturn_t platform_usb_irq(int irq, void *dev_id)
{
	unsigned long flags;
	uint8_t otgstate;

	struct phytium_cusb *config = (struct phytium_cusb *)dev_id;
	struct GADGET_CTRL *gadget_ctrl = config->gadget_priv;
	struct HOST_CTRL *host_ctrl = config->host_priv;

	if (gadget_ctrl || host_ctrl) {
		if (host_ctrl)
			otgstate = phytium_read8(&host_ctrl->regs->otgstate);
		else
			otgstate = phytium_read8(&gadget_ctrl->regs->otgstate);

		spin_lock_irqsave(&config->lock, flags);
		if (otgstate > HOST_OTG_STATE_A_WAIT_VFALL)
			config->gadget_obj->gadget_isr(config->gadget_priv);
		else
			config->host_obj->host_isr(config->host_priv);
		spin_unlock_irqrestore(&config->lock, flags);
	}

	return IRQ_HANDLED;
}

static int phytium_driver_probe(struct platform_device *pdev)
{
	struct phytium_cusb *config;
	struct resource *res, *phy_res;
	int retval = 0;

	config = devm_kzalloc(&pdev->dev, sizeof(*config), GFP_KERNEL);
	if (!config)
		return -ENOMEM;

	spin_lock_init(&config->lock);
	config->dev = &pdev->dev;
	config->isVhubHost = false;

	config->irq = platform_get_irq(pdev, 0);
	if (config->irq <= 0) {
		dev_err(config->dev, "getting usb irq failed\n");
		return config->irq;
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	config->regs = devm_ioremap(&pdev->dev, res->start, resource_size(res));
	if (IS_ERR(config->regs)) {
		dev_err(config->dev, "map IOMEM resource failed\n");
		return -1;
	}

	phy_res = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	config->phy_regs = devm_ioremap(&pdev->dev, phy_res->start, resource_size(phy_res));
	if (IS_ERR(config->phy_regs)) {
		dev_err(config->dev, "map IOMEM phy resource failed\n");
		return -1;
	}

	phytium_get_dr_mode(config);

	phytium_core_reset(config, false);

	if (config->dr_mode == USB_DR_MODE_HOST ||
	    config->dr_mode == USB_DR_MODE_OTG) {
		if (res->start == USB2_2_BASE_ADDRESS)
			config->isVhubHost = true;

		phytium_host_init(config);
	}

	if (config->dr_mode == USB_DR_MODE_PERIPHERAL ||
			config->dr_mode == USB_DR_MODE_OTG)
		phytium_gadget_init(config);

	dev_set_drvdata(&pdev->dev, config);

	if (config->irq > 0) {
		retval = devm_request_irq(config->dev, config->irq, platform_usb_irq,
				IRQF_SHARED, "phytium_otg", config);
		if (retval != 0) {
			dev_err(config->dev, "request irq %d err %d\n", config->irq, retval);
			config->irq = 0;
		}
	}

	return retval;
}

static int phytium_driver_remove(struct platform_device *dev)
{
	struct phytium_cusb *config = platform_get_drvdata(dev);

	if (!config)
		return 0;

	phytium_get_dr_mode(config);

	if (config->dr_mode == USB_DR_MODE_HOST ||
			config->dr_mode == USB_DR_MODE_OTG)
		phytium_host_uninit(config);

	if (config->dr_mode == USB_DR_MODE_PERIPHERAL ||
			config->dr_mode == USB_DR_MODE_OTG)
		phytium_gadget_uninit(config);

	dev_set_drvdata(&dev->dev, NULL);
	return 0;
}

static void phytium_driver_shutdown(struct platform_device *dev)
{
	pr_info("%s %d\n", __func__, __LINE__);
}

#ifdef CONFIG_PM
static int phytium_driver_suspend(struct device *dev)
{
	struct phytium_cusb *config;
	int ret = 0;

	config = dev_get_drvdata(dev);

	if (config->dr_mode == USB_DR_MODE_HOST ||
			config->dr_mode == USB_DR_MODE_OTG)
		ret = phytium_host_suspend(config);

	if (config->dr_mode == USB_DR_MODE_PERIPHERAL ||
			config->dr_mode == USB_DR_MODE_OTG)
		ret = phytium_gadget_suspend(config);

	return ret;
}

static int phytium_driver_resume(struct device *dev)
{
	struct phytium_cusb *config;
	int ret = 0;

	config = dev_get_drvdata(dev);
	if (config->dr_mode == USB_DR_MODE_HOST ||
			config->dr_mode == USB_DR_MODE_OTG)
		ret = phytium_host_resume(config);

	if (config->dr_mode == USB_DR_MODE_PERIPHERAL ||
			config->dr_mode == USB_DR_MODE_OTG)
		ret = phytium_gadget_resume(config);

	return ret;
}

static const struct dev_pm_ops phytium_usb_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(phytium_driver_suspend, phytium_driver_resume)
};
#endif

static struct platform_driver phytium_otg_driver = {
	.driver	=	{
		.name	= "phytium-otg",
		.of_match_table	= of_match_ptr(phytium_otg_of_match),
		.acpi_match_table = ACPI_PTR(phytium_otg_acpi_match),
#ifdef CONFIG_PM
		.pm	= &phytium_usb_pm_ops,
#endif
	},
	.probe	= phytium_driver_probe,
	.remove	= phytium_driver_remove,
	.shutdown	= phytium_driver_shutdown,
};

module_platform_driver(phytium_otg_driver);

MODULE_AUTHOR("Chen Zhenhua <chenzhenhua@phytium.com.cn>");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Phytium usb platform wrapper");

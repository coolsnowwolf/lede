// SPDX-License-Identifier: GPL-2.0

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/io.h>
#include <linux/pci.h>
#include <linux/of_platform.h>
#include <linux/dma-mapping.h>
#include <linux/usb/otg.h>
#include "core.h"

#define PHYTIUM_OTG_USB_LOADED  3

static bool phytium_hw_is_device(struct phytium_cusb *config)
{
	pr_info("%s %d\n", __func__, __LINE__);

	return false;
}

static bool phytium_hw_is_host(struct phytium_cusb *config)
{
	pr_info("%s %d\n", __func__, __LINE__);

	return true;
}

static int phytium_get_dr_mode(struct phytium_cusb *config)
{
	enum usb_dr_mode mode;

	config->dr_mode = usb_get_dr_mode(config->dev);
	if (config->dr_mode == USB_DR_MODE_UNKNOWN)
		config->dr_mode = USB_DR_MODE_OTG;

	mode = config->dr_mode;
	if (phytium_hw_is_device(config)) {
		if (IS_ENABLED(CONFIG_USB_PHYTIUM_HOST)) {
			dev_err(config->dev, "Controller does not support host mode.\n");
			return -EINVAL;
		}

		mode = USB_DR_MODE_PERIPHERAL;
	} else if (phytium_hw_is_host(config)) {
		if (IS_ENABLED(CONFIG_USB_PHYTIUM_PERIPHERAL)) {
			dev_err(config->dev, "Controller does not support device mode.\n");
			return -EINVAL;
		}
		mode = USB_DR_MODE_HOST;
	} else {
		if (IS_ENABLED(CONFIG_USB_PHYTIUM_HOST))
			mode = USB_DR_MODE_HOST;
		else if (IS_ENABLED(CONFIG_USB_PHYTIUM_PERIPHERAL))
			mode = USB_DR_MODE_PERIPHERAL;
	}

	if (mode != config->dr_mode) {
		dev_warn(config->dev, "Configuration mismatch. dr_mode forced to %s\n",
		mode == USB_DR_MODE_HOST ? "host" : "device");
		config->dr_mode = mode;
	}

	return 0;
}

static int phytium_pci_probe(struct pci_dev *pdev, const struct pci_device_id *pid)
{
	struct phytium_cusb *config;
	int retval = 0;

	if (usb_disabled())
		return -ENODEV;

	retval = pcim_enable_device(pdev);
	if (retval < 0) {
		dev_err(&pdev->dev, "pcim_enable_device failed\n");
		return -ENODEV;
	}
	pci_set_master(pdev);

	config = devm_kzalloc(&pdev->dev, sizeof(*config), GFP_KERNEL);
	if (!config)
		return -ENOMEM;

	spin_lock_init(&config->lock);
	config->dev = &pdev->dev;

	config->irq = pdev->irq;
	if (config->irq <= 0) {
		dev_err(config->dev, "getting usb irq failed\n");
		return config->irq;
	}

	config->regs = pci_iomap(pdev, 0, 0);
	if (IS_ERR(config->regs)) {
		dev_err(config->dev, "map IOMEM resource failed\n");
		return -1;
	}

	if (!pdev->dev.dma_mask)
		pdev->dev.dma_mask = &pdev->dev.coherent_dma_mask;

	if (dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(64)))
		dev_err(&pdev->dev, "failed to set 64-bit dma\n");
	else if (dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(32)))
		dev_err(&pdev->dev, "failed to set 32-bit dma\n");

	pci_enable_msi(pdev);

	phytium_get_dr_mode(config);

	phytium_core_reset(config, false);

	if (config->dr_mode == USB_DR_MODE_HOST || config->dr_mode == USB_DR_MODE_OTG)
		phytium_host_init(config);

	if (config->dr_mode == USB_DR_MODE_PERIPHERAL || config->dr_mode == USB_DR_MODE_OTG)
		phytium_gadget_init(config);

	dev_set_drvdata(config->dev, config);

	return 0;
}

static void phytium_pci_remove(struct pci_dev *pdev)
{
	struct phytium_cusb *config = dev_get_drvdata(&pdev->dev);

	phytium_get_dr_mode(config);
	if (config->dr_mode == USB_DR_MODE_HOST || config->dr_mode == USB_DR_MODE_OTG)
		phytium_host_uninit(config);

	if (config->dr_mode == USB_DR_MODE_PERIPHERAL || config->dr_mode == USB_DR_MODE_OTG)
		phytium_gadget_uninit(config);

	if (config->dr_mode == USB_DR_MODE_PERIPHERAL || config->dr_mode == USB_DR_MODE_OTG)
		usb_del_gadget_udc(&config->gadget);

	dev_set_drvdata(&pdev->dev, NULL);
	pr_info("%s %d\n", __func__, __LINE__);
}

static void phytium_pci_shutdown(struct pci_dev *pdev)
{
	struct phytium_cusb *config;

	config = dev_get_drvdata(&pdev->dev);

	phytium_get_dr_mode(config);

	if (config->dr_mode == USB_DR_MODE_PERIPHERAL || config->dr_mode == USB_DR_MODE_OTG)
		usb_del_gadget_udc(&config->gadget);
}

#ifdef CONFIG_PM
static int phytium_pci_resume(struct pci_dev *pdev)
{
	unsigned long flags = 0;
	struct phytium_cusb *config;
	int ret = 0;

	config = dev_get_drvdata(&pdev->dev);

	spin_lock_irqsave(&config->lock, flags);
	ret = phytium_host_resume(config);
	spin_unlock_irqrestore(&config->lock, flags);

	return ret;
}

static int phytium_pci_suspend(struct pci_dev *pdev, pm_message_t state)
{
	unsigned long flags = 0;
	struct phytium_cusb *config;
	int ret;

	config = dev_get_drvdata(&pdev->dev);

	spin_lock_irqsave(&config->lock, flags);
	ret = phytium_host_suspend(config);
	spin_unlock_irqrestore(&config->lock, flags);

	return 0;
}
#endif

const struct pci_device_id phytium_pci_id_table[] = {
	{0x10ee, 0x8012, PCI_ANY_ID, PCI_ANY_ID, 0, 0, 0},
	{}
};

static struct pci_driver phytium_otg_driver = {
	.name = "phytium_usb",
	.id_table = phytium_pci_id_table,
	.probe = phytium_pci_probe,
	.remove = phytium_pci_remove,
	.shutdown = phytium_pci_shutdown,
#ifdef CONFIG_PM
	.resume	= phytium_pci_resume,
	.suspend = phytium_pci_suspend,
#endif
};

module_pci_driver(phytium_otg_driver);

MODULE_AUTHOR("Chen Zhenhua <chenzhenhua@phytium.com.cn>");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Phytium usb pci wrapper");

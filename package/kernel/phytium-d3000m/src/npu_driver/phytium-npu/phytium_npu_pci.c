// SPDX-License-Identifier: GPL-2.0
/* NPU pci driver for Phytium NPU controller
 *
 * Copyright (C) 2023 Phytium Technology Co., Ltd.
 */
#include <linux/interrupt.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/io.h>
#include <linux/pm.h>
#include <linux/version.h>
#include <linux/pci.h>
#include "phytium_npu.h"
#include "phytium_npu_px210_reg.h"

#define PHYTIUM_PCI_VENDOR_ID 0x1DB7
#define PHYTIUM_PCI_DEVICE_ID 0xDC24
#define PCI_BAR_DEV       0
/* PCI and PCI-E do not support 64bit devices on BAR1: 64bit uses 2 bars */
#define PCI_BAR_RAM       2
#define NUM_PCI_BARS      3

static const struct pci_device_id pci_pci_ids[] = {
	{ PCI_DEVICE(PHYTIUM_PCI_VENDOR_ID, PHYTIUM_PCI_DEVICE_ID), .class = 0xb, },
	{ 0, }
};
MODULE_DEVICE_TABLE(pci, pci_pci_ids);

static irqreturn_t phytium_npu_thread_irq(int irq, void *dev_id)
{
	struct pci_dev *pci_dev = (struct pci_dev *)dev_id;

	return phytium_npu_handle_thread_irq(&pci_dev->dev);
}

static irqreturn_t phytium_npu_handle_isr(int irq, void *dev_id)
{
	struct pci_dev *pci_dev = (struct pci_dev *)dev_id;

	if (!pci_dev)
		return IRQ_NONE;

	return phytium_npu_handle_irq(&pci_dev->dev);
}

static int phytium_npu_pci_probe(struct pci_dev *pci_dev,
				 const struct pci_device_id *id)
{
	struct phytium_npu_dev *npu_dev;
	struct device *dev = &pci_dev->dev;
	void __iomem *reg_addr = NULL;
	int ret = 0;
	int irq = 0;
	int bar = 0;

	dev_info(dev, "probed a NPU device, pci_dev: %x:%x\n", pci_dev->vendor, pci_dev->device);

	/* Enable the device */
	if (pci_enable_device(pci_dev))
		goto out_disable;

	pci_set_master(pci_dev);

	if (dev->dma_mask) {
		dev_info(dev, "%s dev->dma_mask : %p\n", __func__, dev->dma_mask);
	} else {
		dev_info(dev, "%s mask unset, setting coherent\n", __func__);
		dev->dma_mask = &dev->coherent_dma_mask;
	}
	dev_info(dev, "%s dma_set_mask %#llx,new 40bit\n", __func__, dma_get_mask(dev));
	ret = dma_set_mask(dev, DMA_BIT_MASK(40));
	if (ret) {
		dev_err(dev, "%s failed to set dma mask\n", __func__);
		goto out_disable;
	}

	/* Reserve PCI I/O and memory resources */
	if (pci_request_regions(pci_dev, "npupci"))
		goto out_add_dev;

	for (bar = 0; bar < NUM_PCI_BARS; bar++) {
		if (bar == PCI_BAR_RAM) {
			/* Don't ioremap pci memory - it is mapped on demand */
			continue;
		}
		if (bar == PCI_BAR_DEV) {
			reg_addr = devm_ioremap(dev,
						pci_resource_start(pci_dev, bar),
						pci_resource_len(pci_dev, bar));
			dev_info(dev, "[bar %u] addr: %#llx size: %#llx km: 0x%p\n",
				 bar, pci_resource_start(pci_dev, bar),
				 pci_resource_len(pci_dev, bar),
				 reg_addr);
			break;
		}
	}
	if (!reg_addr)
		goto out_release;

	ret = pci_enable_msi(pci_dev);
	if (ret)
		dev_err(dev, "Failed to enable MSI IRQ.");

	/* Get the IRQ...*/
	irq = pci_dev->irq;

	ret = phytium_npu_register_dev(dev, NULL, reg_addr);
	if (ret) {
		dev_err(&pci_dev->dev, "failed to initialize driver core!\n");
		goto out_add_dev;
	}

	npu_dev = dev_get_drvdata(dev);
	npu_dev->irq = irq;
	npu_dev->clock_freq = 800000000;
	npu_dev->is_platform_dev = FALSE;
	npu_dev->power_reg_base = NULL;
	ret = devm_request_threaded_irq(&pci_dev->dev, irq, phytium_npu_handle_isr,
					phytium_npu_thread_irq, IRQF_SHARED,
					"phytium-npu", pci_dev);

	if (ret) {
		dev_err(&pci_dev->dev, "failed to request irq\n");
		goto out_irq;
	}
	pr_info("%s: registered irq: %d\n", __func__, irq);

	return ret;

out_irq:
	phytium_npu_unregister_dev(&pci_dev->dev);
out_add_dev:
	devm_iounmap(&pci_dev->dev, reg_addr);
out_release:
	pci_release_regions(pci_dev);
out_disable:
	pci_disable_device(pci_dev);
	return ret;
}

static void phytium_npu_remove(struct pci_dev *pci_dev)
{
	struct phytium_npu_dev *npu_dev = dev_get_drvdata(&pci_dev->dev);

	if (npu_dev->irq)
		devm_free_irq(&pci_dev->dev, npu_dev->irq, (void *)pci_dev);
	pci_disable_msi(pci_dev);
	pci_release_regions(pci_dev);
	pci_disable_device(pci_dev);
	phytium_npu_unregister_dev(&pci_dev->dev);
	pr_info("unregister dev");
}

#ifdef CONFIG_PM
static int phytium_npu_suspend(struct device *dev)
{
	return phytium_npu_common_suspend(dev);
}

static int phytium_npu_resume(struct device *dev)
{
	return phytium_npu_common_resume(dev);
}

static int phytium_npu_runtime_suspend(struct device *dev)
{
	return 0;
}

static int phytium_npu_runtime_resume(struct device *dev)
{
	return 0;
}

#endif

static const struct dev_pm_ops phytium_npu_pm_ops = {
	SET_RUNTIME_PM_OPS(phytium_npu_runtime_suspend,
			   phytium_npu_runtime_resume, NULL)
	SET_SYSTEM_SLEEP_PM_OPS(phytium_npu_suspend, phytium_npu_resume)
};

static ssize_t info_show(struct device_driver *drv, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "NPU PCI driver version: %s\n", NPU_FTN300_DRIVER_VERSION);
}

static DRIVER_ATTR_RO(info);
static struct attribute *npu_pci_attrs[] = {
	&driver_attr_info.attr,
	NULL
};
ATTRIBUTE_GROUPS(npu_pci);

static struct pci_driver phytium_npu_pci_drv = {
	.name = NPU_FTN300_PCI_DRV_NAME,
	.id_table = pci_pci_ids,
	.probe = phytium_npu_pci_probe,
	.remove = phytium_npu_remove,
	.driver = {
		.groups = npu_pci_groups,
		.pm = &phytium_npu_pm_ops,
	}
};
module_pci_driver(phytium_npu_pci_drv);

MODULE_AUTHOR("Cheng Quan <chengquan@phytium.com.cn>");
MODULE_DESCRIPTION("Phytium NPU driver for PCI device controllers");
MODULE_LICENSE("GPL");

// SPDX-License-Identifier: GPL-2.0
/*
 * PCI driver for Phytium NAND flash controller
 *
 * Copyright (c) 2021-2023 Phytium Technology Co., Ltd.
 */
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/device.h>

#include "phytium_nand.h"

#define DRV_NAME	"phytium_nand_pci"

static struct mtd_partition partition_info[] = {
	{
		.name = "Flash partition 1",
		.offset =  0x0000000,
		.size =    0x4000000 },
	{
		.name = "Flash partition 2",
		.offset =  0x4000000,
		.size =    0x8000000 },
	{
		.name = "Flash partition 3",
		.offset =  0x8000000,
		.size =    0x10000000 },
	{
		.name = "Flash partition 4",
		.offset =  0x10000000,
		.size =    0x12000000 },
	{
		.name = "Flash partition 5",
		.offset =  0x12000000,
		.size =    0x14000000 },
};

static struct phytium_nfc_caps x100_nfc_caps = {
	.hw_ver = 1,
	.int_mask_bits = 13,
	.max_cs_nb = 2,
	.max_rb_nb = 1,
	.legacy_of_bindings = true,
	.ecc_strength = 4,
	.ecc_step_size = 512,
	.nr_parts = 5,
	.parts = partition_info,
};

static int phytium_pci_probe(struct pci_dev *pdev, const struct pci_device_id *pid)
{
	struct phytium_nfc *nfc;
	int ret;

	ret = pcim_enable_device(pdev);
	if (ret)
		return ret;

	ret = pcim_iomap_regions(pdev, 0x1, pci_name(pdev));
	if (ret) {
		dev_err(&pdev->dev, "I/O memory remapping failed\n");
		return ret;
	}

	pci_set_master(pdev);
	pci_try_set_mwi(pdev);

	ret = pci_set_dma_mask(pdev, DMA_BIT_MASK(64));
	if (ret)
		return ret;

	ret = pci_set_consistent_dma_mask(pdev, DMA_BIT_MASK(64));
	if (ret)
		return ret;

	nfc = devm_kzalloc(&pdev->dev, sizeof(struct phytium_nfc),
			   GFP_KERNEL);
	if (!nfc)
		return -ENOMEM;

	nfc->dev = &pdev->dev;
	nfc->regs = pcim_iomap_table(pdev)[0];
	nfc->irq = pdev->irq;
	nfc->caps = &x100_nfc_caps;

	ret = devm_request_irq(nfc->dev, nfc->irq, phytium_nfc_isr,
			       IRQF_SHARED, "phytium-nfc-pci", nfc);
	if (ret) {
		dev_err(nfc->dev, "Failed to register NFC interrupt.\n");
		return ret;
	}

	ret = phytium_nand_init(nfc);
	if (ret)
		return ret;

	pci_set_drvdata(pdev, nfc);

	return ret;
}

static void phytium_pci_remove(struct pci_dev *pdev)
{
	struct phytium_nfc *nfc = pci_get_drvdata(pdev);
	int ret;

	ret = phytium_nand_remove(nfc);
	if (ret)
		dev_warn(&pdev->dev, "can't remove device properly: %d\n", ret);
}

static int __maybe_unused phytium_nfc_prepare(struct device *dev)
{
	struct pci_dev *pci = to_pci_dev(dev);
	struct phytium_nfc *nfc = pci_get_drvdata(pci);
	int ret;

	ret = phytium_nand_prepare(nfc);

	return 0;
}

static int __maybe_unused phytium_nfc_resume(struct device *dev)
{
	struct pci_dev *pci = to_pci_dev(dev);
	struct phytium_nfc *nfc = pci_get_drvdata(pci);
	int ret;

	ret = phytium_nand_resume(nfc);

	return ret;
}

static const struct dev_pm_ops phytium_pci_dev_pm_ops = {
	.prepare = phytium_nfc_prepare,
	.resume = phytium_nfc_resume,
};

static const struct pci_device_id phytium_pci_id_table[] = {
	{ PCI_VDEVICE(PHYTIUM, 0xdc29) },
	{ }
};
MODULE_DEVICE_TABLE(pci, phytium_pci_id_table);

static struct pci_driver phytium_pci_driver = {
	.name		= DRV_NAME,
	.id_table	= phytium_pci_id_table,
	.probe		= phytium_pci_probe,
	.remove		= phytium_pci_remove,
	.driver	= {
		.pm	= &phytium_pci_dev_pm_ops,
	},
};
module_pci_driver(phytium_pci_driver);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("PCI driver for Phytium NAND controller");
MODULE_AUTHOR("Zhu Mingshuai <zhumingshuai@phytium.com.cn>");

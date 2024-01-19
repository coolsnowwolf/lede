// SPDX-License-Identifier: GPL-2.0
/*
 * Phytium I2S LSD MFD driver over PCI bus
 *
 * Copyright (c) 2020-2023 Phytium Technology Co., Ltd.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/mfd/core.h>

struct phytium_px210_mfd {
	struct device		*dev;
};

struct pdata_px210_mfd {
	struct device		*dev;
	char			*name;
	int			clk_base;
};

static struct resource phytium_px210_i2s_res0[] = {
	[0] = {
		.flags = IORESOURCE_MEM,
	},
	[1] = {
		.flags = IORESOURCE_MEM,
	},
	[2] = {
		.flags = IORESOURCE_IRQ,
	},
};

static struct mfd_cell phytium_px210_mfd_cells[] = {
	{
		.id = 0,
		.name = "phytium-i2s",
		.of_compatible = "phytium,i2s",
		.resources = phytium_px210_i2s_res0,
		.num_resources = ARRAY_SIZE(phytium_px210_i2s_res0),
		.ignore_resource_conflicts = true,
	},
};

static void phytium_px210_i2s_setup(struct pci_dev *pdev)
{
	struct mfd_cell *cell = &phytium_px210_mfd_cells[0];
	struct resource *res = (struct resource *)cell->resources;
	struct pdata_px210_mfd *pdata;

	res[0].start = pci_resource_start(pdev, 0);
	res[0].end = pci_resource_start(pdev, 0) + 0x0fff;

	res[1].start = pci_resource_start(pdev, 0) + 0x1000;
	res[1].end = pci_resource_start(pdev, 0) + 0x1fff;

	res[2].start = pdev->irq;
	res[2].end = pdev->irq;

	pdata = devm_kzalloc(&pdev->dev, sizeof(*pdata), GFP_KERNEL);

	pdata->dev = &pdev->dev;
	pdata->name = "phytium-i2s-lsd";
	pdata->clk_base = 480000000;

	cell->platform_data = pdata;
	cell->pdata_size = sizeof(*pdata);
}

static int phytium_px210_mfd_probe(struct pci_dev *pdev,
				 const struct pci_device_id *id)
{
	struct phytium_px210_mfd *phytium_mfd;
	int ret;

	ret = pcim_enable_device(pdev);
	if (ret)
		return ret;

	pci_set_master(pdev);

	phytium_mfd = devm_kzalloc(&pdev->dev, sizeof(*phytium_mfd), GFP_KERNEL);
	if (!phytium_mfd)
		return -ENOMEM;

	phytium_mfd->dev = &pdev->dev;
	dev_set_drvdata(&pdev->dev, phytium_mfd);

	phytium_px210_i2s_setup(pdev);

	ret = mfd_add_devices(&pdev->dev, 0, phytium_px210_mfd_cells,
			      ARRAY_SIZE(phytium_px210_mfd_cells), NULL, 0,
			      NULL);
	if (ret)
		return 0;

	return 0;
}


static void phytium_px210_mfd_remove(struct pci_dev *pdev)
{
	mfd_remove_devices(&pdev->dev);
}

static const struct pci_device_id phytium_px210_mfd_ids[] = {
	{
		.vendor		= 0x1DB7,
		.device		= 0xDC2B,
		.subvendor	= PCI_ANY_ID,
		.subdevice	= PCI_ANY_ID,
		.class		= 0x3,
		.class_mask	= 0,
	},
	{},
};
MODULE_DEVICE_TABLE(pci, phytium_px210_mfd_ids);

static struct pci_driver phytium_i2s_lsd_mfd_driver = {
	.name		= "phytium_px210_mfd_i2s",
	.id_table	= phytium_px210_mfd_ids,
	.probe		= phytium_px210_mfd_probe,
	.remove		= phytium_px210_mfd_remove,
};

module_pci_driver(phytium_i2s_lsd_mfd_driver);

MODULE_AUTHOR("Yiqun Zhang <zhangyiqun@phytium.com.cn>");
MODULE_DESCRIPTION("Phytium Px210 MFD PCI driver for I2S-LSD");
MODULE_LICENSE("GPL v2");

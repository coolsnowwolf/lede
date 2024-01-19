// SPDX-License-Identifier: GPL-2.0
/*
 * Phytium SPI core controller PCI driver.
 *
 * Copyright (c) 2019-2023 Phytium Technology Co., Ltd.
 */

#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/gpio.h>
#include <linux/highmem.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/spi/spi.h>
#include <linux/scatterlist.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/property.h>

#include "spi-phytium.h"

#define DRIVER_NAME "phytium_spi_pci"

static int phytium_spi_pci_probe(struct pci_dev *pdev,
			    const struct pci_device_id *id)
{
	struct phytium_spi *fts;
	int pci_bar = 0;
	int ret;

	fts = devm_kzalloc(&pdev->dev, sizeof(struct phytium_spi),
			GFP_KERNEL);
	if (!fts)
		return -ENOMEM;

	ret = pcim_enable_device(pdev);
	if (ret)
		return ret;

	ret = pcim_iomap_regions(pdev, 1 << pci_bar, pci_name(pdev));
	if (ret) {
		dev_err(&pdev->dev, "pci iomap failed?\n");
		return ret;
	}

	fts->regs = pcim_iomap_table(pdev)[pci_bar];
	if (IS_ERR(fts->regs)) {
		dev_err(&pdev->dev, "SPI region map failed\n");
		return PTR_ERR(fts->regs);
	}

	fts->irq = pdev->irq;
	if (fts->irq < 0) {
		dev_err(&pdev->dev, "no irq resource?\n");
		return fts->irq; /* -ENXIO */
	}

	fts->bus_num = -1;

	fts->max_freq = 48000000;

	fts->num_cs = 4;

	fts->global_cs = 1;

	ret = phytium_spi_add_host(&pdev->dev, fts);
	if (ret)
		return ret;

	pci_set_drvdata(pdev, fts);
	return 0;
}

static void phytium_spi_pci_remove(struct pci_dev *pdev)
{
	struct phytium_spi *fts = pci_get_drvdata(pdev);

	phytium_spi_remove_host(fts);
}


#ifdef CONFIG_PM_SLEEP
static int spi_suspend(struct device *dev)
{
	struct phytium_spi *fts = dev_get_drvdata(dev);

	return phytium_spi_suspend_host(fts);
}

static int spi_resume(struct device *dev)
{
	struct phytium_spi *fts = dev_get_drvdata(dev);

	return phytium_spi_resume_host(fts);
}
#endif

static SIMPLE_DEV_PM_OPS(phytium_spi_pm_ops, spi_suspend, spi_resume);

static const struct pci_device_id phytium_device_pci_tbl[] = {
	{ PCI_VDEVICE(PHYTIUM, 0xdc2c) },
	{},
};

static struct pci_driver phytium_spi_pci_driver = {
	.name		= DRIVER_NAME,
	.id_table	= phytium_device_pci_tbl,
	.probe		= phytium_spi_pci_probe,
	.remove		= phytium_spi_pci_remove,
	.driver		= {
		.pm = &phytium_spi_pm_ops,
	}
};

module_pci_driver(phytium_spi_pci_driver);

MODULE_AUTHOR("Yiqun Zhang <zhangyiqun@phytium.com.cn>");
MODULE_DESCRIPTION("PCI Driver for Phytium SPI controller core");
MODULE_LICENSE("GPL v2");

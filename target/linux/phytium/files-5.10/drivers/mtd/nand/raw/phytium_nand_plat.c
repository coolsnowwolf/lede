// SPDX-License-Identifier: GPL-2.0
/*
 * Core driver for Phytium NAND flash controller
 *
 * Copyright (c) 2020-2023 Phytium Technology Co., Ltd.
 */

#include <linux/module.h>
#include <linux/device.h>
#include <linux/clk.h>
#include <linux/pm_runtime.h>
#include <linux/platform_device.h>
#include <linux/dmaengine.h>
#include <linux/dma-mapping.h>
#include <linux/of.h>
#include <linux/of_dma.h>
#include <linux/acpi.h>
#include <linux/acpi_dma.h>

#include "phytium_nand.h"

#define DRV_NAME	"phytium_nand_plat"

static int phytium_nfc_plat_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct resource *r;
	struct phytium_nfc *nfc;
	unsigned int ecc_strength;
	unsigned int ecc_step_size;
	int ret;

	nfc = devm_kzalloc(&pdev->dev, sizeof(struct phytium_nfc),
			   GFP_KERNEL);
	if (!nfc)
		return -ENOMEM;

	nfc->dev = dev;

	r = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	nfc->regs = devm_ioremap_resource(dev, r);
	if (IS_ERR(nfc->regs))
		return PTR_ERR(nfc->regs);

	dev_info(nfc->dev, "NFC register address :%p, phy address:%llx\n",
				nfc->regs, r->start);

	nfc->irq = platform_get_irq(pdev, 0);
	if (nfc->irq < 0) {
		dev_err(dev, "failed to retrieve irq\n");
		return nfc->irq;
	}

	ret = devm_request_irq(dev, nfc->irq, phytium_nfc_isr, 0,
			       "phytium-nfc-plat", nfc);
	if (ret) {
		dev_err(nfc->dev, "Failed to register NFC interrupt.\n");
		return ret;
	}

	nfc->caps = devm_kzalloc(dev, sizeof(struct phytium_nfc_caps), GFP_KERNEL);
	if (!nfc->caps)
		return -ENOMEM;

	/* Currently hard-coded parameters */
	nfc->caps->hw_ver = 2;
	nfc->caps->int_mask_bits = 17;
	nfc->caps->max_cs_nb = 4;
	nfc->caps->max_rb_nb = 4;
	nfc->caps->nr_parts = 0;
	nfc->caps->parts = NULL;

	device_property_read_u32(&pdev->dev, "nand-ecc-strength", &ecc_strength);
	nfc->caps->ecc_strength = ecc_strength ? ecc_strength : 8;

	device_property_read_u32(&pdev->dev, "nand-ecc-step-size", &ecc_step_size);
	nfc->caps->ecc_step_size = ecc_step_size ? ecc_step_size : 512;

	ret = phytium_nand_init(nfc);
	if (ret)
		return ret;

	platform_set_drvdata(pdev, nfc);

	return ret;
}

static int phytium_nfc_plat_remove(struct platform_device *pdev)
{
	struct phytium_nfc *nfc = platform_get_drvdata(pdev);

	return phytium_nand_remove(nfc);
}

static int __maybe_unused phytium_nfc_plat_prepare(struct device *dev)
{
	struct phytium_nfc *nfc = dev_get_drvdata(dev);

	return phytium_nand_prepare(nfc);
}

static int __maybe_unused phytium_nfc_plat_resume(struct device *dev)
{
	struct phytium_nfc *nfc = dev_get_drvdata(dev);
	int ret;

	ret = phytium_nand_resume(nfc);

	return ret;
}

static const struct dev_pm_ops phytium_dev_pm_ops = {
	.prepare = phytium_nfc_plat_prepare,
	.resume = phytium_nfc_plat_resume,
};

#ifdef CONFIG_OF
static const struct of_device_id phytium_nfc_of_ids[] = {
	{ .compatible = "phytium,nfc", },
	{ /* sentinel */ },
};
MODULE_DEVICE_TABLE(of, phytium_nfc_of_ids);
#endif

static struct platform_driver phytium_nfc_plat_driver = {
	.driver	= {
		.name		= DRV_NAME,
		.of_match_table = phytium_nfc_of_ids,
		.pm		= &phytium_dev_pm_ops,
	},
	.probe = phytium_nfc_plat_probe,
	.remove	= phytium_nfc_plat_remove,
};
module_platform_driver(phytium_nfc_plat_driver)

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Phytium NAND controller Platform driver");
MODULE_AUTHOR("Zhu Mingshuai <zhumingshuai@phytium.com.cn>");

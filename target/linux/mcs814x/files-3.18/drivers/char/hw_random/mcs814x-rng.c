/*
 * RNG driver for Moschip MCS814x SoC
 *
 * Copyright 2012 (C), Florian Fainelli <florian@openwrt.org>
 *
 * This file is licensed under  the terms of the GNU General Public
 * License version 2. This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/platform_device.h>
#include <linux/hw_random.h>
#include <linux/io.h>
#include <linux/of.h>

#define STAT	0x00
#define RND	0x04

struct mcs814x_rng_priv {
	void __iomem	*regs;
};

static int mcs814x_rng_data_read(struct hwrng *rng, u32 *buffer)
{
	struct mcs814x_rng_priv *priv = (struct mcs814x_rng_priv *)rng->priv;

	*buffer = readl_relaxed(priv->regs + RND);

	return 4;
}

static int mcs814x_rng_probe(struct platform_device *pdev)
{
	struct resource *res;
	struct mcs814x_rng_priv *priv;
	struct hwrng *rng;
	int ret;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res)
		return -ENODEV;

	priv = kzalloc(sizeof(*priv), GFP_KERNEL);
	if (!priv) {
		ret = -ENOMEM;
		goto out;
	}

	rng = kzalloc(sizeof(*rng), GFP_KERNEL);
	if (!rng) {
		ret = -ENOMEM;
		goto out_priv;
	}

	platform_set_drvdata(pdev, rng);
	rng->priv = (unsigned long)priv;
	rng->name = pdev->name;
	rng->data_read = mcs814x_rng_data_read;

	priv->regs = devm_ioremap_resource(&pdev->dev, res);
	if (!priv->regs) {
		ret = -ENOMEM;
		goto out_rng;
	}

	ret = hwrng_register(rng);
	if (ret) {
		dev_err(&pdev->dev, "failed to register hwrng driver\n");
		goto out;
	}

	dev_info(&pdev->dev, "registered\n");

	return ret;

out_rng:
	platform_set_drvdata(pdev, NULL);
	kfree(rng);
out_priv:
	kfree(priv);
out:
	return ret;
}

static int mcs814x_rng_remove(struct platform_device *pdev)
{
	struct hwrng *rng = platform_get_drvdata(pdev);
	struct mcs814x_rng_priv *priv = (struct mcs814x_rng_priv *)rng->priv;

	hwrng_unregister(rng);
	kfree(priv);
	kfree(rng);
	platform_set_drvdata(pdev, NULL);

	return 0;
}

static const struct of_device_id mcs814x_rng_ids[] = {
	{ .compatible = "moschip,mcs814x-rng", },
	{ /* sentinel */ },
};

static struct platform_driver mcs814x_rng_driver = {
	.driver	= {
		.name	= "mcs814x-rng",
		.owner	= THIS_MODULE,
		.of_match_table = mcs814x_rng_ids,
	},
	.probe	= mcs814x_rng_probe,
	.remove	= mcs814x_rng_remove,
};

module_platform_driver(mcs814x_rng_driver);

MODULE_AUTHOR("Florian Fainelli <florian@openwrt.org>");
MODULE_DESCRIPTION("H/W Random Number Generator (RNG) for Moschip MCS814x");
MODULE_LICENSE("GPL");

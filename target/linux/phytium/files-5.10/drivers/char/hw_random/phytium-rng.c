// SPDX-License-Identifier: GPL-2.0
/*
 * Phytium SoC RNG Driver
 *
 * Copyright (c) 2021-2023 Phytium Technology Co., Ltd.
 */

#include <linux/bits.h>
#include <linux/err.h>
#include <linux/hw_random.h>
#include <linux/io.h>
#include <linux/iopoll.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/random.h>
#include <linux/string.h>

#define	TRNG_CR			0x00
#define  TRNG_CR_RNGEN		BIT(0)
#define  TRNG_CR_ROSEN_MASK	GENMASK(7, 4)
#define  TRNG_CR_DIEN		BIT(16)
#define  TRNG_CR_ERIEN		BIT(17)
#define  TRNG_CR_IRQEN		BIT(24)
#define TRNG_MSEL		0x04
#define  TRNG_MSEL_MSEL		BIT(0)
#define TRNG_SR			0x08
#define  TRNG_SR_HTF		BIT(0)
#define  TRNG_SR_DRDY		BIT(1)
#define  TRNG_SR_ERERR		BIT(3)
#define TRNG_DR			0x0C
#define TRNG_RESEED		0x40
#define  TRNG_RESEED_RSED	BIT(0)

#define DELAY	10
#define TIMEOUT	100

static int msel;
module_param(msel, int, 0444);
MODULE_PARM_DESC(msel, "Phytium RNG mode selection: 0 - TRNG. 1 - PRNG.");

struct phytium_rng {
	struct hwrng rng;
	void __iomem *base;
};

static int phytium_rng_init(struct hwrng *rng)
{
	struct phytium_rng *priv = container_of(rng, struct phytium_rng, rng);
	u32 reg;

	/* Mode Selection */
	reg = msel ? TRNG_MSEL_MSEL : 0;
	writel(reg, priv->base + TRNG_MSEL);

	/* If PRGN mode is on, do reseed operations */
	if (msel)
		writel(TRNG_RESEED_RSED, priv->base + TRNG_RESEED);

	/* Clear status */
	writel(0x7, priv->base + TRNG_SR);

	/* Enable TRNG */
	reg = readl(priv->base + TRNG_CR) | TRNG_CR_ROSEN_MASK | TRNG_CR_RNGEN;
	writel(reg, priv->base + TRNG_CR);

	return 0;
}

static void phytium_rng_cleanup(struct hwrng *rng)
{
	struct phytium_rng *priv = container_of(rng, struct phytium_rng, rng);

	writel(0x7, priv->base + TRNG_SR);
}

static int phytium_rng_read(struct hwrng *rng, void *buf, size_t max, bool wait)
{
	struct phytium_rng *priv = container_of(rng, struct phytium_rng, rng);
	u32 reg;
	int ret = 0;

	/* TRNG can generate at most 8*32bit random number per time */
	max = max > 32 ? 32 : max;

	reg = readl(priv->base + TRNG_SR);
	if (!(reg & TRNG_SR_DRDY) && wait) {
		ret = readl_poll_timeout(priv->base + TRNG_SR, reg,
					 reg & TRNG_SR_DRDY, DELAY, TIMEOUT);
		if (ret) {
			dev_err((struct device *)priv->rng.priv,
				"%s: timeout %x!\n", __func__, reg);
			return -EIO;
		}
	}

	while (max >= 4) {
		*(u32 *)buf = readl(priv->base + TRNG_DR);

		ret += sizeof(u32);
		buf += sizeof(u32);
		max -= sizeof(u32);
	}

	/* Clear DRDY by writing 1 */
	writel(reg | TRNG_SR_DRDY, priv->base + TRNG_SR);

	return ret;
}

static int phytium_rng_probe(struct platform_device *pdev)
{
	struct phytium_rng *priv;
	struct resource *mem;

	priv = devm_kzalloc(&pdev->dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	platform_set_drvdata(pdev, priv);

	mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	priv->base = devm_ioremap_resource(&pdev->dev, mem);
	if (IS_ERR(priv->base))
		return PTR_ERR(priv->base);

	priv->rng.name = pdev->name;
	priv->rng.init = phytium_rng_init;
	priv->rng.cleanup = phytium_rng_cleanup;
	priv->rng.read = phytium_rng_read;
	priv->rng.priv = (unsigned long)&pdev->dev;
	priv->rng.quality = 1000;

	return devm_hwrng_register(&pdev->dev, &priv->rng);
}

static const struct of_device_id phytium_rng_dt_ids[] = {
	{ .compatible = "phytium,rng" },
	{ }
};
MODULE_DEVICE_TABLE(of, phytium_rng_dt_ids);

static struct platform_driver phytium_rng_driver = {
	.probe = phytium_rng_probe,
	.driver = {
		.name = "phytium-rng",
		.of_match_table = of_match_ptr(phytium_rng_dt_ids),
	}
};
module_platform_driver(phytium_rng_driver);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Phytium random number generator driver");
MODULE_AUTHOR("Chen Baozi <chenbaozi@phytium.com.cn>");

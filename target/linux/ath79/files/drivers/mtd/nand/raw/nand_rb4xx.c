// SPDX-License-Identifier: GPL-2.0-only
/*
 * NAND driver for the MikroTik RouterBoard 4xx series
 *
 * Copyright (C) 2008-2011 Gabor Juhos <juhosg@openwrt.org>
 * Copyright (C) 2008 Imre Kaloz <kaloz@openwrt.org>
 * Copyright (C) 2015 Bert Vermeulen <bert@biot.com>
 * Copyright (C) 2020 Christopher Hill <ch6574@gmail.com>
 *
 * This file was based on the driver for Linux 2.6.22 published by
 * MikroTik for their RouterBoard 4xx series devices.
 *
 * N.B. driver probe reports "DMA mask not set" warnings which are
 * an artifact of using a platform_driver as an MFD device child.
 * See conversation here https://lkml.org/lkml/2020/4/28/675
 */
#include <linux/platform_device.h>
#include <linux/mtd/rawnand.h>
#include <linux/gpio/consumer.h>
#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/version.h>

#include <mfd/rb4xx-cpld.h>

struct rb4xx_nand {
	struct rb4xx_cpld *cpld;
	struct device *dev;

	struct nand_chip chip;
	struct gpio_desc *ale;
	struct gpio_desc *cle;
	struct gpio_desc *nce;
	struct gpio_desc *rdy;
};

static int rb4xx_ooblayout_ecc(struct mtd_info *mtd, int section,
			       struct mtd_oob_region *oobregion)
{
	switch (section) {
	case 0:
		oobregion->offset = 8;
		oobregion->length = 3;
		return 0;
	case 1:
		oobregion->offset = 13;
		oobregion->length = 3;
		return 0;
	default:
		return -ERANGE;
	}
}

static int rb4xx_ooblayout_free(struct mtd_info *mtd, int section,
				struct mtd_oob_region *oobregion)
{
	switch (section) {
	case 0:
		oobregion->offset = 0;
		oobregion->length = 4;
		return 0;
	case 1:
		oobregion->offset = 4;
		oobregion->length = 1;
		return 0;
	case 2:
		oobregion->offset = 6;
		oobregion->length = 2;
		return 0;
	case 3:
		oobregion->offset = 11;
		oobregion->length = 2;
		return 0;
	default:
		return -ERANGE;
	}
}

static const struct mtd_ooblayout_ops rb4xx_nand_ecclayout_ops = {
	.ecc = rb4xx_ooblayout_ecc,
	.free = rb4xx_ooblayout_free,
};

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,20,0)
static uint8_t rb4xx_nand_read_byte(struct mtd_info *mtd)
{
	struct rb4xx_nand *nand = mtd->priv;
#else
static u8 rb4xx_nand_read_byte(struct nand_chip *chip)
{
	struct rb4xx_nand *nand = chip->priv;
#endif
	struct rb4xx_cpld *cpld = nand->cpld;
	u8 data;
	int ret;

	ret = cpld->read_nand(cpld, &data, 1);
	if (unlikely(ret))
		return 0xff;

	return data;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,20,0)
static void rb4xx_nand_write_buf(struct mtd_info *mtd, const uint8_t *buf,
				 int len)
{
	struct rb4xx_nand *nand = mtd->priv;
#else
static void rb4xx_nand_write_buf(struct nand_chip *chip, const u8 *buf, int len)
{
	struct rb4xx_nand *nand = chip->priv;
#endif
	struct rb4xx_cpld *cpld = nand->cpld;

	cpld->write_nand(cpld, buf, len);
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,20,0)
static void rb4xx_nand_read_buf(struct mtd_info *mtd, uint8_t *buf, int len)
{
	struct rb4xx_nand *nand = mtd->priv;
#else
static void rb4xx_nand_read_buf(struct nand_chip *chip, u8 *buf, int len)
{
	struct rb4xx_nand *nand = chip->priv;
#endif
	struct rb4xx_cpld *cpld = nand->cpld;

	cpld->read_nand(cpld, buf, len);
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,20,0)
static void rb4xx_nand_cmd_ctrl(struct mtd_info *mtd, int dat, unsigned int ctrl)
{
	struct rb4xx_nand *nand = mtd->priv;
#else
static void rb4xx_nand_cmd_ctrl(struct nand_chip *chip, int dat,
				unsigned int ctrl)
{
	struct rb4xx_nand *nand = chip->priv;
#endif
	struct rb4xx_cpld *cpld = nand->cpld;
	u8 data = dat;

	if (ctrl & NAND_CTRL_CHANGE) {
		gpiod_set_value_cansleep(nand->cle, !!(ctrl & NAND_CLE));
		gpiod_set_value_cansleep(nand->ale, !!(ctrl & NAND_ALE));
		gpiod_set_value_cansleep(nand->nce,  !(ctrl & NAND_NCE));
	}

	if (dat != NAND_CMD_NONE)
		cpld->write_nand(cpld, &data, 1);
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,20,0)
static int rb4xx_nand_dev_ready(struct mtd_info *mtd)
{
	struct rb4xx_nand *nand = mtd->priv;
#else
static int rb4xx_nand_dev_ready(struct nand_chip *chip)
{
	struct rb4xx_nand *nand = chip->priv;
#endif

	return gpiod_get_value_cansleep(nand->rdy);
}

static int rb4xx_nand_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device *parent = dev->parent;
	struct rb4xx_nand *nand;
	struct mtd_info *mtd;
	int ret;

	if (!parent)
		return -ENODEV;

	nand = devm_kzalloc(dev, sizeof(*nand), GFP_KERNEL);
	if (!nand)
		return -ENOMEM;

	platform_set_drvdata(pdev, nand);
	nand->cpld	= dev_get_drvdata(parent);
	nand->dev	= dev;

	nand->ale = devm_gpiod_get_index(dev, NULL, 0, GPIOD_OUT_LOW);
	if (IS_ERR(nand->ale))
		dev_err(dev, "missing gpio ALE: %ld\n", PTR_ERR(nand->ale));

	nand->cle = devm_gpiod_get_index(dev, NULL, 1, GPIOD_OUT_LOW);
	if (IS_ERR(nand->cle))
		dev_err(dev, "missing gpio CLE: %ld\n", PTR_ERR(nand->cle));

	nand->nce = devm_gpiod_get_index(dev, NULL, 2, GPIOD_OUT_LOW);
	if (IS_ERR(nand->nce))
		dev_err(dev, "missing gpio nCE: %ld\n", PTR_ERR(nand->nce));

	nand->rdy = devm_gpiod_get_index(dev, NULL, 3, GPIOD_IN);
	if (IS_ERR(nand->rdy))
		dev_err(dev, "missing gpio RDY: %ld\n", PTR_ERR(nand->rdy));

	if (IS_ERR(nand->ale) || IS_ERR(nand->cle) ||
	    IS_ERR(nand->nce) || IS_ERR(nand->rdy))
		return -ENOENT;

	gpiod_set_consumer_name(nand->ale, "mikrotik:nand:ALE");
	gpiod_set_consumer_name(nand->cle, "mikrotik:nand:CLE");
	gpiod_set_consumer_name(nand->nce, "mikrotik:nand:nCE");
	gpiod_set_consumer_name(nand->rdy, "mikrotik:nand:RDY");

	mtd = nand_to_mtd(&nand->chip);
	mtd->priv	= nand;
	mtd->owner	= THIS_MODULE;
	mtd->dev.parent	= dev;
	mtd_set_of_node(mtd, dev->of_node);

	if (mtd->writesize == 512)
		mtd_set_ooblayout(mtd, &rb4xx_nand_ecclayout_ops);

	nand->chip.ecc.mode	= NAND_ECC_SOFT;
	nand->chip.ecc.algo	= NAND_ECC_HAMMING;
	nand->chip.options	= NAND_NO_SUBPAGE_WRITE;
	nand->chip.priv		= nand;

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,20,0)
	nand->chip.read_byte	= rb4xx_nand_read_byte;
	nand->chip.write_buf	= rb4xx_nand_write_buf;
	nand->chip.read_buf	= rb4xx_nand_read_buf;
	nand->chip.cmd_ctrl	= rb4xx_nand_cmd_ctrl;
	nand->chip.dev_ready	= rb4xx_nand_dev_ready;
	nand->chip.chip_delay	= 25;

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,19,130)
	ret = nand_scan(mtd, 1);
#else
	ret = nand_scan(&nand->chip, 1);
#endif
#else
	nand->chip.legacy.read_byte	= rb4xx_nand_read_byte;
	nand->chip.legacy.write_buf	= rb4xx_nand_write_buf;
	nand->chip.legacy.read_buf	= rb4xx_nand_read_buf;
	nand->chip.legacy.cmd_ctrl	= rb4xx_nand_cmd_ctrl;
	nand->chip.legacy.dev_ready	= rb4xx_nand_dev_ready;
	nand->chip.legacy.chip_delay	= 25;

	ret = nand_scan(&nand->chip, 1);
#endif
	if (ret)
		return -ENXIO;

	ret = mtd_device_register(mtd, NULL, 0);
	if (ret) {
		nand_release(&nand->chip);
		return ret;
	}

	return 0;
}

static int rb4xx_nand_remove(struct platform_device *pdev)
{
	struct rb4xx_nand *nand = platform_get_drvdata(pdev);

	nand_release(&nand->chip);

	return 0;
}

static const struct platform_device_id rb4xx_nand_id_table[] = {
	{ "mikrotik,rb4xx-nand", },
	{ },
};
MODULE_DEVICE_TABLE(platform, rb4xx_nand_id_table);

static struct platform_driver rb4xx_nand_driver = {
	.probe = rb4xx_nand_probe,
	.remove = rb4xx_nand_remove,
	.id_table = rb4xx_nand_id_table,
	.driver = {
		.name = "rb4xx-nand",
	},
};

module_platform_driver(rb4xx_nand_driver);

MODULE_DESCRIPTION("Mikrotik RB4xx NAND driver");
MODULE_AUTHOR("Gabor Juhos <juhosg@openwrt.org>");
MODULE_AUTHOR("Imre Kaloz <kaloz@openwrt.org>");
MODULE_AUTHOR("Bert Vermeulen <bert@biot.com>");
MODULE_AUTHOR("Christopher Hill <ch6574@gmail.com");
MODULE_LICENSE("GPL v2");

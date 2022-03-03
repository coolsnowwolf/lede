// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *  MikroTik RB91x NAND flash driver
 *
 *  Main part is copied from original driver written by Gabor Juhos.
 *
 *  Copyright (C) 2013-2014 Gabor Juhos <juhosg@openwrt.org>
 */

/*
 * WARNING: to speed up NAND reading/writing we are working with SoC GPIO
 * controller registers directly -- not through standard GPIO API.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mtd/rawnand.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/platform_device.h>
#include <linux/gpio/consumer.h>
#include <linux/version.h>
#include <linux/of_platform.h>

#include <asm/mach-ath79/ar71xx_regs.h>

/* Bit masks for NAND data lines in ath79 gpio 32-bit register */
#define RB91X_NAND_NRW_BIT BIT(12)
#define RB91X_NAND_DATA_BITS (BIT(0) | BIT(1) | BIT(2) | BIT(3) | BIT(4) \
			    | BIT(13) | BIT(14) | BIT(15))
#define RB91X_NAND_LOW_DATA_MASK	0x1f
#define RB91X_NAND_HIGH_DATA_MASK	0xe0
#define RB91X_NAND_HIGH_DATA_SHIFT	8

enum rb91x_nand_gpios {
	RB91X_NAND_READ,/* Read */
	RB91X_NAND_RDY, /* NAND Ready */
	RB91X_NAND_NCE, /* Chip Enable. Active low */
	RB91X_NAND_CLE, /* Command Latch Enable */
	RB91X_NAND_ALE, /* Address Latch Enable */
	RB91X_NAND_NRW, /* Read/Write. Active low */
	RB91X_NAND_NLE, /* Latch Enable. Active low */

	RB91X_NAND_GPIOS,
};

struct rb91x_nand_drvdata {
	struct nand_chip chip;
	struct device *dev;
	struct gpio_desc **gpio;
	void __iomem *ath79_gpio_base;
};

static inline void rb91x_nand_latch_lock(struct rb91x_nand_drvdata *drvdata,
					 int lock)
{
	gpiod_set_value_cansleep(drvdata->gpio[RB91X_NAND_NLE], lock);
}

static int rb91x_ooblayout_ecc(struct mtd_info *mtd, int section,
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

static int rb91x_ooblayout_free(struct mtd_info *mtd, int section,
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

static const struct mtd_ooblayout_ops rb91x_nand_ecclayout_ops = {
	.ecc = rb91x_ooblayout_ecc,
	.free = rb91x_ooblayout_free,
};

static void rb91x_nand_write(struct rb91x_nand_drvdata *drvdata,
			     const u8 *buf,
			     unsigned len)
{
	void __iomem *base = drvdata->ath79_gpio_base;
	u32 oe_reg;
	u32 out_reg;
	u32 out;
	unsigned i;

	rb91x_nand_latch_lock(drvdata, 1);

	oe_reg = __raw_readl(base + AR71XX_GPIO_REG_OE);
	out_reg = __raw_readl(base + AR71XX_GPIO_REG_OUT);

	/* Set data lines to output mode */
	__raw_writel(oe_reg & ~(RB91X_NAND_DATA_BITS | RB91X_NAND_NRW_BIT),
		     base + AR71XX_GPIO_REG_OE);

	out = out_reg & ~(RB91X_NAND_DATA_BITS | RB91X_NAND_NRW_BIT);
	for (i = 0; i != len; i++) {
		u32 data;

		data = (buf[i] & RB91X_NAND_HIGH_DATA_MASK) <<
			RB91X_NAND_HIGH_DATA_SHIFT;
		data |= buf[i] & RB91X_NAND_LOW_DATA_MASK;
		data |= out;
		__raw_writel(data, base + AR71XX_GPIO_REG_OUT);

		/* Deactivate WE line */
		data |= RB91X_NAND_NRW_BIT;
		__raw_writel(data, base + AR71XX_GPIO_REG_OUT);
		/* Flush write */
		__raw_readl(base + AR71XX_GPIO_REG_OUT);
	}

	/* Restore registers */
	__raw_writel(out_reg, base + AR71XX_GPIO_REG_OUT);
	__raw_writel(oe_reg, base + AR71XX_GPIO_REG_OE);
	/* Flush write */
	__raw_readl(base + AR71XX_GPIO_REG_OUT);

	rb91x_nand_latch_lock(drvdata, 0);
}

static void rb91x_nand_read(struct rb91x_nand_drvdata *drvdata,
			    u8 *read_buf,
			    unsigned len)
{
	void __iomem *base = drvdata->ath79_gpio_base;
	u32 oe_reg;
	u32 out_reg;
	unsigned i;

	/* Enable read mode */
	gpiod_set_value_cansleep(drvdata->gpio[RB91X_NAND_READ], 1);

	rb91x_nand_latch_lock(drvdata, 1);

	/* Save registers */
	oe_reg = __raw_readl(base + AR71XX_GPIO_REG_OE);
	out_reg = __raw_readl(base + AR71XX_GPIO_REG_OUT);

	/* Set data lines to input mode */
	__raw_writel(oe_reg | RB91X_NAND_DATA_BITS,
		     base + AR71XX_GPIO_REG_OE);

	for (i = 0; i < len; i++) {
		u32 in;
		u8 data;

		/* Activate RE line */
		__raw_writel(RB91X_NAND_NRW_BIT, base + AR71XX_GPIO_REG_CLEAR);
		/* Flush write */
		__raw_readl(base + AR71XX_GPIO_REG_CLEAR);

		/* Read input lines */
		in = __raw_readl(base + AR71XX_GPIO_REG_IN);

		/* Deactivate RE line */
		__raw_writel(RB91X_NAND_NRW_BIT, base + AR71XX_GPIO_REG_SET);

		data = (in & RB91X_NAND_LOW_DATA_MASK);
		data |= (in >> RB91X_NAND_HIGH_DATA_SHIFT) &
			RB91X_NAND_HIGH_DATA_MASK;

		read_buf[i] = data;
	}

	/* Restore  registers */
	__raw_writel(out_reg, base + AR71XX_GPIO_REG_OUT);
	__raw_writel(oe_reg, base + AR71XX_GPIO_REG_OE);
	/* Flush write */
	__raw_readl(base + AR71XX_GPIO_REG_OUT);

	rb91x_nand_latch_lock(drvdata, 0);

	/* Disable read mode */
	gpiod_set_value_cansleep(drvdata->gpio[RB91X_NAND_READ], 0);
}

static int rb91x_nand_dev_ready(struct nand_chip *chip)
{
	struct rb91x_nand_drvdata *drvdata = (struct rb91x_nand_drvdata *)(chip->priv);

	return gpiod_get_value_cansleep(drvdata->gpio[RB91X_NAND_RDY]);
}

static void rb91x_nand_cmd_ctrl(struct nand_chip *chip, int cmd,
				unsigned int ctrl)
{
	struct rb91x_nand_drvdata *drvdata = chip->priv;

	if (ctrl & NAND_CTRL_CHANGE) {
		gpiod_set_value_cansleep(drvdata->gpio[RB91X_NAND_CLE],
					(ctrl & NAND_CLE) ? 1 : 0);
		gpiod_set_value_cansleep(drvdata->gpio[RB91X_NAND_ALE],
					(ctrl & NAND_ALE) ? 1 : 0);
		gpiod_set_value_cansleep(drvdata->gpio[RB91X_NAND_NCE],
					(ctrl & NAND_NCE) ? 1 : 0);
	}

	if (cmd != NAND_CMD_NONE) {
		u8 t = cmd;

		rb91x_nand_write(drvdata, &t, 1);
	}
}

static u8 rb91x_nand_read_byte(struct nand_chip *chip)
{
	u8 data = 0xff;

	rb91x_nand_read(chip->priv, &data, 1);

	return data;
}

static void rb91x_nand_read_buf(struct nand_chip *chip, u8 *buf, int len)
{
	rb91x_nand_read(chip->priv, buf, len);
}

static void rb91x_nand_write_buf(struct nand_chip *chip, const u8 *buf, int len)
{
	rb91x_nand_write(chip->priv, buf, len);
}

static void rb91x_nand_release(struct rb91x_nand_drvdata *drvdata)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,8,0)
	mtd_device_unregister(nand_to_mtd(&drvdata->chip));
	nand_cleanup(&drvdata->chip);
#else
	nand_release(&drvdata->chip);
#endif
}

static int rb91x_nand_probe(struct platform_device *pdev)
{
	struct rb91x_nand_drvdata *drvdata;
	struct mtd_info *mtd;
	int r;
	struct device *dev = &pdev->dev;
	struct gpio_descs *gpios;

	drvdata = devm_kzalloc(dev, sizeof(*drvdata), GFP_KERNEL);
	if (!drvdata)
		return -ENOMEM;

	platform_set_drvdata(pdev, drvdata);

	gpios = gpiod_get_array(dev, NULL, GPIOD_OUT_LOW);
	if (IS_ERR(gpios)) {
		dev_err(dev, "failed to get gpios: %d\n", (int)gpios);
		return -EINVAL;
	}

	if (gpios->ndescs != RB91X_NAND_GPIOS) {
		dev_err(dev, "expected %d gpios\n", RB91X_NAND_GPIOS);
		return -EINVAL;
	}

	drvdata->gpio = gpios->desc;

	gpiod_direction_input(drvdata->gpio[RB91X_NAND_RDY]);

	drvdata->ath79_gpio_base = ioremap(AR71XX_GPIO_BASE, AR71XX_GPIO_SIZE);

	drvdata->dev = dev;

	drvdata->chip.priv = drvdata;

	drvdata->chip.legacy.cmd_ctrl = rb91x_nand_cmd_ctrl;
	drvdata->chip.legacy.dev_ready = rb91x_nand_dev_ready;
	drvdata->chip.legacy.read_byte = rb91x_nand_read_byte;
	drvdata->chip.legacy.write_buf = rb91x_nand_write_buf;
	drvdata->chip.legacy.read_buf = rb91x_nand_read_buf;

	drvdata->chip.legacy.chip_delay = 25;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,9,0)
	drvdata->chip.ecc.engine_type      = NAND_ECC_ENGINE_TYPE_SOFT;
	drvdata->chip.ecc.algo             = NAND_ECC_ALGO_HAMMING;
#else
	drvdata->chip.ecc.mode             = NAND_ECC_SOFT;
	drvdata->chip.ecc.algo             = NAND_ECC_HAMMING;
#endif
	drvdata->chip.options = NAND_NO_SUBPAGE_WRITE;

	r = nand_scan(&drvdata->chip, 1);
	if (r) {
		dev_err(dev, "nand_scan() failed: %d\n", r);
		return r;
	}

	mtd = nand_to_mtd(&drvdata->chip);
	mtd->dev.parent = dev;
	mtd_set_of_node(mtd, dev->of_node);
	mtd->owner = THIS_MODULE;
	if (mtd->writesize == 512)
		mtd_set_ooblayout(mtd, &rb91x_nand_ecclayout_ops);

	r = mtd_device_register(mtd, NULL, 0);
	if (r) {
		dev_err(dev, "mtd_device_register() failed: %d\n",
			r);
		goto err_release_nand;
	}

	return 0;

err_release_nand:
	rb91x_nand_release(drvdata);
	return r;
}

static int rb91x_nand_remove(struct platform_device *pdev)
{
	struct rb91x_nand_drvdata *drvdata = platform_get_drvdata(pdev);

	rb91x_nand_release(drvdata);

	return 0;
}

static const struct of_device_id rb91x_nand_match[] = {
	{ .compatible = "mikrotik,rb91x-nand" },
	{},
};

MODULE_DEVICE_TABLE(of, rb91x_nand_match);

static struct platform_driver rb91x_nand_driver = {
	.probe	= rb91x_nand_probe,
	.remove	= rb91x_nand_remove,
	.driver	= {
		.name	= "rb91x-nand",
		.owner	= THIS_MODULE,
		.of_match_table = rb91x_nand_match,
	},
};

module_platform_driver(rb91x_nand_driver);

MODULE_DESCRIPTION("MikrotTik RB91x NAND flash driver");
MODULE_VERSION(DRV_VERSION);
MODULE_AUTHOR("Gabor Juhos <juhosg@openwrt.org>");
MODULE_AUTHOR("Denis Kalashnikov <denis281089@gmail.com>");
MODULE_LICENSE("GPL v2");

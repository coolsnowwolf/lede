/*
 *  NAND flash driver for the MikroTik RouterBOARD 91x series
 *
 *  Copyright (C) 2013-2014 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/module.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0)
#include <linux/mtd/nand.h>
#else
#include <linux/mtd/rawnand.h>
#endif
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/platform_data/rb91x_nand.h>

#include <asm/mach-ath79/ar71xx_regs.h>
#include <asm/mach-ath79/ath79.h>

#define DRV_DESC	"NAND flash driver for the RouterBOARD 91x series"

#define RB91X_NAND_NRWE		BIT(12)

#define RB91X_NAND_DATA_BITS	(BIT(0) | BIT(1) | BIT(2) | BIT(3) | BIT(4) |\
				 BIT(13) | BIT(14) | BIT(15))

#define RB91X_NAND_INPUT_BITS	(RB91X_NAND_DATA_BITS | RB91X_NAND_RDY)
#define RB91X_NAND_OUTPUT_BITS	(RB91X_NAND_DATA_BITS | RB91X_NAND_NRWE)

#define RB91X_NAND_LOW_DATA_MASK	0x1f
#define RB91X_NAND_HIGH_DATA_MASK	0xe0
#define RB91X_NAND_HIGH_DATA_SHIFT	8

struct rb91x_nand_info {
	struct nand_chip chip;
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,6,0)
	struct mtd_info mtd;
#endif
	struct device *dev;

	int gpio_nce;
	int gpio_ale;
	int gpio_cle;
	int gpio_rdy;
	int gpio_read;
	int gpio_nrw;
	int gpio_nle;
};

static inline struct rb91x_nand_info *mtd_to_rbinfo(struct mtd_info *mtd)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,6,0)
	return container_of(mtd, struct rb91x_nand_info, mtd);
#else
	struct nand_chip *chip = mtd_to_nand(mtd);

	return container_of(chip, struct rb91x_nand_info, chip);
#endif
}

static struct mtd_info *rbinfo_to_mtd(struct rb91x_nand_info *nfc)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,6,0)
	return &nfc->mtd;
#else
	return nand_to_mtd(&nfc->chip);
#endif
}


#if LINUX_VERSION_CODE < KERNEL_VERSION(4,6,0)
/*
 * We need to use the OLD Yaffs-1 OOB layout, otherwise the RB bootloader
 * will not be able to find the kernel that we load.
 */
static struct nand_ecclayout rb91x_nand_ecclayout = {
	.eccbytes	= 6,
	.eccpos		= { 8, 9, 10, 13, 14, 15 },
	.oobavail	= 9,
	.oobfree	= { { 0, 4 }, { 6, 2 }, { 11, 2 }, { 4, 1 } }
};

#else

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
#endif /* < 4.6 */

static struct mtd_partition rb91x_nand_partitions[] = {
	{
		.name	= "booter",
		.offset	= 0,
		.size	= (256 * 1024),
		.mask_flags = MTD_WRITEABLE,
	}, {
		.name	= "kernel",
		.offset	= (256 * 1024),
		.size	= (4 * 1024 * 1024) - (256 * 1024),
	}, {
		.name	= "ubi",
		.offset	= MTDPART_OFS_NXTBLK,
		.size	= MTDPART_SIZ_FULL,
	},
};

static void rb91x_nand_write(struct rb91x_nand_info *rbni,
			     const u8 *buf,
			     unsigned len)
{
	void __iomem *base = ath79_gpio_base;
	u32 oe_reg;
	u32 out_reg;
	u32 out;
	unsigned i;

	/* enable the latch */
	gpio_set_value_cansleep(rbni->gpio_nle, 0);

	oe_reg = __raw_readl(base + AR71XX_GPIO_REG_OE);
	out_reg = __raw_readl(base + AR71XX_GPIO_REG_OUT);

	/* set data lines to output mode */
	__raw_writel(oe_reg & ~(RB91X_NAND_DATA_BITS | RB91X_NAND_NRWE),
		     base + AR71XX_GPIO_REG_OE);

	out = out_reg & ~(RB91X_NAND_DATA_BITS | RB91X_NAND_NRWE);
	for (i = 0; i != len; i++) {
		u32 data;

		data = (buf[i] & RB91X_NAND_HIGH_DATA_MASK) <<
			RB91X_NAND_HIGH_DATA_SHIFT;
		data |= buf[i] & RB91X_NAND_LOW_DATA_MASK;
		data |= out;
		__raw_writel(data, base + AR71XX_GPIO_REG_OUT);

		/* deactivate WE line */
		data |= RB91X_NAND_NRWE;
		__raw_writel(data, base + AR71XX_GPIO_REG_OUT);
		/* flush write */
		__raw_readl(base + AR71XX_GPIO_REG_OUT);
	}

	/* restore  registers */
	__raw_writel(out_reg, base + AR71XX_GPIO_REG_OUT);
	__raw_writel(oe_reg, base + AR71XX_GPIO_REG_OE);
	/* flush write */
	__raw_readl(base + AR71XX_GPIO_REG_OUT);

	/* disable the latch */
	gpio_set_value_cansleep(rbni->gpio_nle, 1);
}

static void rb91x_nand_read(struct rb91x_nand_info *rbni,
			    u8 *read_buf,
			    unsigned len)
{
	void __iomem *base = ath79_gpio_base;
	u32 oe_reg;
	u32 out_reg;
	unsigned i;

	/* enable read mode */
	gpio_set_value_cansleep(rbni->gpio_read, 1);

	/* enable latch */
	gpio_set_value_cansleep(rbni->gpio_nle, 0);

	/* save registers */
	oe_reg = __raw_readl(base + AR71XX_GPIO_REG_OE);
	out_reg = __raw_readl(base + AR71XX_GPIO_REG_OUT);

	/* set data lines to input mode */
	__raw_writel(oe_reg | RB91X_NAND_DATA_BITS,
		     base + AR71XX_GPIO_REG_OE);

	for (i = 0; i < len; i++) {
		u32 in;
		u8 data;

		/* activate RE line */
		__raw_writel(RB91X_NAND_NRWE, base + AR71XX_GPIO_REG_CLEAR);
		/* flush write */
		__raw_readl(base + AR71XX_GPIO_REG_CLEAR);

		/* read input lines */
		in = __raw_readl(base + AR71XX_GPIO_REG_IN);

		/* deactivate RE line */
		__raw_writel(RB91X_NAND_NRWE, base + AR71XX_GPIO_REG_SET);

		data = (in & RB91X_NAND_LOW_DATA_MASK);
		data |= (in >> RB91X_NAND_HIGH_DATA_SHIFT) &
			RB91X_NAND_HIGH_DATA_MASK;

		read_buf[i] = data;
	}

	/* restore  registers */
	__raw_writel(out_reg, base + AR71XX_GPIO_REG_OUT);
	__raw_writel(oe_reg, base + AR71XX_GPIO_REG_OE);
	/* flush write */
	__raw_readl(base + AR71XX_GPIO_REG_OUT);

	/* disable latch */
	gpio_set_value_cansleep(rbni->gpio_nle, 1);

	/* disable read mode */
	gpio_set_value_cansleep(rbni->gpio_read, 0);
}

static int rb91x_nand_dev_ready(struct mtd_info *mtd)
{
	struct rb91x_nand_info *rbni = mtd_to_rbinfo(mtd);

	return gpio_get_value_cansleep(rbni->gpio_rdy);
}

static void rb91x_nand_cmd_ctrl(struct mtd_info *mtd, int cmd,
				unsigned int ctrl)
{
	struct rb91x_nand_info *rbni = mtd_to_rbinfo(mtd);

	if (ctrl & NAND_CTRL_CHANGE) {
		gpio_set_value_cansleep(rbni->gpio_cle,
					(ctrl & NAND_CLE) ? 1 : 0);
		gpio_set_value_cansleep(rbni->gpio_ale,
					(ctrl & NAND_ALE) ? 1 : 0);
		gpio_set_value_cansleep(rbni->gpio_nce,
					(ctrl & NAND_NCE) ? 0 : 1);
	}

	if (cmd != NAND_CMD_NONE) {
		u8 t = cmd;

		rb91x_nand_write(rbni, &t, 1);
	}
}

static u8 rb91x_nand_read_byte(struct mtd_info *mtd)
{
	struct rb91x_nand_info *rbni = mtd_to_rbinfo(mtd);
	u8 data = 0xff;

	rb91x_nand_read(rbni, &data, 1);

	return data;
}

static void rb91x_nand_read_buf(struct mtd_info *mtd, u8 *buf, int len)
{
	struct rb91x_nand_info *rbni = mtd_to_rbinfo(mtd);

	rb91x_nand_read(rbni, buf, len);
}

static void rb91x_nand_write_buf(struct mtd_info *mtd, const u8 *buf, int len)
{
	struct rb91x_nand_info *rbni = mtd_to_rbinfo(mtd);

	rb91x_nand_write(rbni, buf, len);
}

static int rb91x_nand_gpio_init(struct rb91x_nand_info *info)
{
	int ret;

	/*
	 * Ensure that the LATCH is disabled before initializing
	 * control lines.
	 */
	ret = devm_gpio_request_one(info->dev, info->gpio_nle,
				    GPIOF_OUT_INIT_HIGH, "LATCH enable");
	if (ret)
		return ret;

	ret = devm_gpio_request_one(info->dev, info->gpio_nce,
				    GPIOF_OUT_INIT_HIGH, "NAND nCE");
	if (ret)
		return ret;

	ret = devm_gpio_request_one(info->dev, info->gpio_nrw,
				    GPIOF_OUT_INIT_HIGH, "NAND nRW");
	if (ret)
		return ret;

	ret = devm_gpio_request_one(info->dev, info->gpio_cle,
				    GPIOF_OUT_INIT_LOW, "NAND CLE");
	if (ret)
		return ret;

	ret = devm_gpio_request_one(info->dev, info->gpio_ale,
				    GPIOF_OUT_INIT_LOW, "NAND ALE");
	if (ret)
		return ret;

	ret = devm_gpio_request_one(info->dev, info->gpio_read,
				    GPIOF_OUT_INIT_LOW, "NAND READ");
	if (ret)
		return ret;

	ret = devm_gpio_request_one(info->dev, info->gpio_rdy,
				    GPIOF_IN, "NAND RDY");
	return ret;
}

static int rb91x_nand_probe(struct platform_device *pdev)
{
	struct rb91x_nand_info	*rbni;
	struct rb91x_nand_platform_data *pdata;
	struct mtd_info *mtd;
	int ret;

	pr_info(DRV_DESC "\n");

	pdata = dev_get_platdata(&pdev->dev);
	if (!pdata)
		return -EINVAL;

	rbni = devm_kzalloc(&pdev->dev, sizeof(*rbni), GFP_KERNEL);
	if (!rbni)
		return -ENOMEM;

	rbni->dev = &pdev->dev;
	rbni->gpio_nce = pdata->gpio_nce;
	rbni->gpio_ale = pdata->gpio_ale;
	rbni->gpio_cle = pdata->gpio_cle;
	rbni->gpio_read = pdata->gpio_read;
	rbni->gpio_nrw = pdata->gpio_nrw;
	rbni->gpio_rdy = pdata->gpio_rdy;
	rbni->gpio_nle = pdata->gpio_nle;

	rbni->chip.priv	= &rbni;
	mtd = rbinfo_to_mtd(rbni);

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,6,0)
	mtd->priv	= &rbni->chip;
#endif
	mtd->owner	= THIS_MODULE;

	rbni->chip.cmd_ctrl	= rb91x_nand_cmd_ctrl;
	rbni->chip.dev_ready	= rb91x_nand_dev_ready;
	rbni->chip.read_byte	= rb91x_nand_read_byte;
	rbni->chip.write_buf	= rb91x_nand_write_buf;
	rbni->chip.read_buf	= rb91x_nand_read_buf;

	rbni->chip.chip_delay	= 25;
	rbni->chip.ecc.mode	= NAND_ECC_SOFT;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,6,0)
	rbni->chip.ecc.algo = NAND_ECC_HAMMING;
#endif
	rbni->chip.options = NAND_NO_SUBPAGE_WRITE;

	platform_set_drvdata(pdev, rbni);

	ret = rb91x_nand_gpio_init(rbni);
	if (ret)
		return ret;

	ret = nand_scan_ident(mtd, 1, NULL);
	if (ret)
		return ret;

	if (mtd->writesize == 512)
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,6,0)
		rbni->chip.ecc.layout = &rb91x_nand_ecclayout;
#else
		mtd_set_ooblayout(mtd, &rb91x_nand_ecclayout_ops);
#endif

	ret = nand_scan_tail(mtd);
	if (ret)
		return ret;

	ret = mtd_device_register(mtd, rb91x_nand_partitions,
				 ARRAY_SIZE(rb91x_nand_partitions));
	if (ret)
		goto err_release_nand;

	return 0;

err_release_nand:
	nand_release(mtd);
	return ret;
}

static int rb91x_nand_remove(struct platform_device *pdev)
{
	struct rb91x_nand_info *info = platform_get_drvdata(pdev);

	nand_release(rbinfo_to_mtd(info));

	return 0;
}

static struct platform_driver rb91x_nand_driver = {
	.probe	= rb91x_nand_probe,
	.remove	= rb91x_nand_remove,
	.driver	= {
		.name	= RB91X_NAND_DRIVER_NAME,
		.owner	= THIS_MODULE,
	},
};

module_platform_driver(rb91x_nand_driver);

MODULE_DESCRIPTION(DRV_DESC);
MODULE_VERSION(DRV_VERSION);
MODULE_AUTHOR("Gabor Juhos <juhosg@openwrt.org>");
MODULE_LICENSE("GPL v2");

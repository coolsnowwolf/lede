/*
 *  NAND flash driver for the MikroTik RouterBOARD 750
 *
 *  Copyright (C) 2010-2012 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/version.h>
#include <linux/kernel.h>
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

#include <asm/mach-ath79/ar71xx_regs.h>
#include <asm/mach-ath79/ath79.h>
#include <asm/mach-ath79/mach-rb750.h>

#define DRV_NAME	"rb750-nand"
#define DRV_VERSION	"0.1.0"
#define DRV_DESC	"NAND flash driver for the RouterBOARD 750"

#define RB750_NAND_IO0		BIT(RB750_GPIO_NAND_IO0)
#define RB750_NAND_ALE		BIT(RB750_GPIO_NAND_ALE)
#define RB750_NAND_CLE		BIT(RB750_GPIO_NAND_CLE)
#define RB750_NAND_NRE		BIT(RB750_GPIO_NAND_NRE)
#define RB750_NAND_NWE		BIT(RB750_GPIO_NAND_NWE)
#define RB750_NAND_RDY		BIT(RB750_GPIO_NAND_RDY)

#define RB750_NAND_DATA_SHIFT	1
#define RB750_NAND_DATA_BITS	(0xff << RB750_NAND_DATA_SHIFT)
#define RB750_NAND_INPUT_BITS	(RB750_NAND_DATA_BITS | RB750_NAND_RDY)
#define RB750_NAND_OUTPUT_BITS	(RB750_NAND_ALE | RB750_NAND_CLE | \
				 RB750_NAND_NRE | RB750_NAND_NWE)

struct rb750_nand_info {
	struct nand_chip	chip;
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,6,0)
	struct mtd_info		mtd;
#endif
	struct rb7xx_nand_platform_data *pdata;
};

static inline struct rb750_nand_info *mtd_to_rbinfo(struct mtd_info *mtd)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,6,0)
	return container_of(mtd, struct rb750_nand_info, mtd);
#else
	struct nand_chip *chip = mtd_to_nand(mtd);

	return container_of(chip, struct rb750_nand_info, chip);
#endif
}

static struct mtd_info *rbinfo_to_mtd(struct rb750_nand_info *nfc)
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
static struct nand_ecclayout rb750_nand_ecclayout = {
	.eccbytes	= 6,
	.eccpos		= { 8, 9, 10, 13, 14, 15 },
	.oobavail	= 9,
	.oobfree	= { { 0, 4 }, { 6, 2 }, { 11, 2 }, { 4, 1 } }
};

#else

static int rb750_ooblayout_ecc(struct mtd_info *mtd, int section,
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

static int rb750_ooblayout_free(struct mtd_info *mtd, int section,
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

static const struct mtd_ooblayout_ops rb750_nand_ecclayout_ops = {
	.ecc = rb750_ooblayout_ecc,
	.free = rb750_ooblayout_free,
};
#endif /* < 4.6 */

static struct mtd_partition rb750_nand_partitions[] = {
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

static void rb750_nand_write(const u8 *buf, unsigned len)
{
	void __iomem *base = ath79_gpio_base;
	u32 out;
	u32 t;
	unsigned i;

	/* set data lines to output mode */
	t = __raw_readl(base + AR71XX_GPIO_REG_OE);
	__raw_writel(t | RB750_NAND_DATA_BITS, base + AR71XX_GPIO_REG_OE);

	out = __raw_readl(base + AR71XX_GPIO_REG_OUT);
	out &= ~(RB750_NAND_DATA_BITS | RB750_NAND_NWE);
	for (i = 0; i != len; i++) {
		u32 data;

		data = buf[i];
		data <<= RB750_NAND_DATA_SHIFT;
		data |= out;
		__raw_writel(data, base + AR71XX_GPIO_REG_OUT);

		__raw_writel(data | RB750_NAND_NWE, base + AR71XX_GPIO_REG_OUT);
		/* flush write */
		__raw_readl(base + AR71XX_GPIO_REG_OUT);
	}

	/* set data lines to input mode */
	t = __raw_readl(base + AR71XX_GPIO_REG_OE);
	__raw_writel(t & ~RB750_NAND_DATA_BITS, base + AR71XX_GPIO_REG_OE);
	/* flush write */
	__raw_readl(base + AR71XX_GPIO_REG_OE);
}

static void rb750_nand_read(u8 *read_buf, unsigned len)
{
	void __iomem *base = ath79_gpio_base;
	unsigned i;

	for (i = 0; i < len; i++) {
		u8 data;

		/* activate RE line */
		__raw_writel(RB750_NAND_NRE, base + AR71XX_GPIO_REG_CLEAR);
		/* flush write */
		__raw_readl(base + AR71XX_GPIO_REG_CLEAR);

		/* read input lines */
		data = __raw_readl(base + AR71XX_GPIO_REG_IN) >>
		       RB750_NAND_DATA_SHIFT;

		/* deactivate RE line */
		__raw_writel(RB750_NAND_NRE, base + AR71XX_GPIO_REG_SET);

		read_buf[i] = data;
	}
}

static void rb750_nand_select_chip(struct mtd_info *mtd, int chip)
{
	struct rb750_nand_info *rbinfo = mtd_to_rbinfo(mtd);
	void __iomem *base = ath79_gpio_base;
	u32 t;

	if (chip >= 0) {
		rbinfo->pdata->enable_pins();

		/* set input mode for data lines */
		t = __raw_readl(base + AR71XX_GPIO_REG_OE);
		__raw_writel(t & ~RB750_NAND_INPUT_BITS,
			     base + AR71XX_GPIO_REG_OE);

		/* deactivate RE and WE lines */
		__raw_writel(RB750_NAND_NRE | RB750_NAND_NWE,
			     base + AR71XX_GPIO_REG_SET);
		/* flush write */
		(void) __raw_readl(base + AR71XX_GPIO_REG_SET);

		/* activate CE line */
		__raw_writel(rbinfo->pdata->nce_line,
			     base + AR71XX_GPIO_REG_CLEAR);
	} else {
		/* deactivate CE line */
		__raw_writel(rbinfo->pdata->nce_line,
			     base + AR71XX_GPIO_REG_SET);
		/* flush write */
		(void) __raw_readl(base + AR71XX_GPIO_REG_SET);

		t = __raw_readl(base + AR71XX_GPIO_REG_OE);
		__raw_writel(t | RB750_NAND_IO0 | RB750_NAND_RDY,
			     base + AR71XX_GPIO_REG_OE);

		rbinfo->pdata->disable_pins();
	}
}

static int rb750_nand_dev_ready(struct mtd_info *mtd)
{
	void __iomem *base = ath79_gpio_base;

	return !!(__raw_readl(base + AR71XX_GPIO_REG_IN) & RB750_NAND_RDY);
}

static void rb750_nand_cmd_ctrl(struct mtd_info *mtd, int cmd,
				unsigned int ctrl)
{
	if (ctrl & NAND_CTRL_CHANGE) {
		void __iomem *base = ath79_gpio_base;
		u32 t;

		t = __raw_readl(base + AR71XX_GPIO_REG_OUT);

		t &= ~(RB750_NAND_CLE | RB750_NAND_ALE);
		t |= (ctrl & NAND_CLE) ? RB750_NAND_CLE : 0;
		t |= (ctrl & NAND_ALE) ? RB750_NAND_ALE : 0;

		__raw_writel(t, base + AR71XX_GPIO_REG_OUT);
		/* flush write */
		__raw_readl(base + AR71XX_GPIO_REG_OUT);
	}

	if (cmd != NAND_CMD_NONE) {
		u8 t = cmd;
		rb750_nand_write(&t, 1);
	}
}

static u8 rb750_nand_read_byte(struct mtd_info *mtd)
{
	u8 data = 0;
	rb750_nand_read(&data, 1);
	return data;
}

static void rb750_nand_read_buf(struct mtd_info *mtd, u8 *buf, int len)
{
	rb750_nand_read(buf, len);
}

static void rb750_nand_write_buf(struct mtd_info *mtd, const u8 *buf, int len)
{
	rb750_nand_write(buf, len);
}

static void __init rb750_nand_gpio_init(struct rb750_nand_info *info)
{
	void __iomem *base = ath79_gpio_base;
	u32 out;
	u32 t;

	out = __raw_readl(base + AR71XX_GPIO_REG_OUT);

	/* setup output levels */
	__raw_writel(RB750_NAND_NCE | RB750_NAND_NRE | RB750_NAND_NWE,
		     base + AR71XX_GPIO_REG_SET);

	__raw_writel(RB750_NAND_ALE | RB750_NAND_CLE,
		     base + AR71XX_GPIO_REG_CLEAR);

	/* setup input lines */
	t = __raw_readl(base + AR71XX_GPIO_REG_OE);
	__raw_writel(t & ~(RB750_NAND_INPUT_BITS), base + AR71XX_GPIO_REG_OE);

	/* setup output lines */
	t = __raw_readl(base + AR71XX_GPIO_REG_OE);
	t |= RB750_NAND_OUTPUT_BITS;
	t |= info->pdata->nce_line;
	__raw_writel(t, base + AR71XX_GPIO_REG_OE);

	info->pdata->latch_change(~out & RB750_NAND_IO0, out & RB750_NAND_IO0);
}

static int rb750_nand_probe(struct platform_device *pdev)
{
	struct rb750_nand_info	*info;
	struct rb7xx_nand_platform_data *pdata;
	struct mtd_info *mtd;
	int ret;

	printk(KERN_INFO DRV_DESC " version " DRV_VERSION "\n");

	pdata = pdev->dev.platform_data;
	if (!pdata)
		return -EINVAL;

	info = kzalloc(sizeof(*info), GFP_KERNEL);
	if (!info)
		return -ENOMEM;

	info->chip.priv	= &info;

	mtd = rbinfo_to_mtd(info);
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,6,0)
	mtd->priv	= &info->chip;
#endif
	mtd->owner	= THIS_MODULE;

	info->chip.select_chip	= rb750_nand_select_chip;
	info->chip.cmd_ctrl	= rb750_nand_cmd_ctrl;
	info->chip.dev_ready	= rb750_nand_dev_ready;
	info->chip.read_byte	= rb750_nand_read_byte;
	info->chip.write_buf	= rb750_nand_write_buf;
	info->chip.read_buf	= rb750_nand_read_buf;

	info->chip.chip_delay	= 25;
	info->chip.ecc.mode	= NAND_ECC_SOFT;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,6,0)
	info->chip.ecc.algo = NAND_ECC_HAMMING;
#endif
	info->chip.options = NAND_NO_SUBPAGE_WRITE;

	info->pdata = pdata;

	platform_set_drvdata(pdev, info);

	rb750_nand_gpio_init(info);

	ret = nand_scan_ident(mtd, 1, NULL);
	if (ret) {
		ret = -ENXIO;
		goto err_free_info;
	}

	if (mtd->writesize == 512)
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,6,0)
		info->chip.ecc.layout = &rb750_nand_ecclayout;
#else
		mtd_set_ooblayout(mtd, &rb750_nand_ecclayout_ops);
#endif

	ret = nand_scan_tail(mtd);
	if (ret) {
		return -ENXIO;
		goto err_set_drvdata;
	}

	ret = mtd_device_register(mtd, rb750_nand_partitions,
				 ARRAY_SIZE(rb750_nand_partitions));
	if (ret)
		goto err_release_nand;

	return 0;

err_release_nand:
	nand_release(&info->chip);
err_set_drvdata:
	platform_set_drvdata(pdev, NULL);
err_free_info:
	kfree(info);
	return ret;
}

static int rb750_nand_remove(struct platform_device *pdev)
{
	struct rb750_nand_info *info = platform_get_drvdata(pdev);

	nand_release(&info->chip);
	platform_set_drvdata(pdev, NULL);
	kfree(info);

	return 0;
}

static struct platform_driver rb750_nand_driver = {
	.probe	= rb750_nand_probe,
	.remove	= rb750_nand_remove,
	.driver	= {
		.name	= DRV_NAME,
		.owner	= THIS_MODULE,
	},
};

static int __init rb750_nand_init(void)
{
	return platform_driver_register(&rb750_nand_driver);
}

static void __exit rb750_nand_exit(void)
{
	platform_driver_unregister(&rb750_nand_driver);
}

module_init(rb750_nand_init);
module_exit(rb750_nand_exit);

MODULE_DESCRIPTION(DRV_DESC);
MODULE_VERSION(DRV_VERSION);
MODULE_AUTHOR("Gabor Juhos <juhosg@openwrt.org>");
MODULE_LICENSE("GPL v2");

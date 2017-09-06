/*
 *  NAND flash driver for the MikroTik RouterBoard 4xx series
 *
 *  Copyright (C) 2008-2011 Gabor Juhos <juhosg@openwrt.org>
 *  Copyright (C) 2008 Imre Kaloz <kaloz@openwrt.org>
 *
 *  This file was based on the driver for Linux 2.6.22 published by
 *  MikroTik for their RouterBoard 4xx series devices.
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/slab.h>

#include <asm/mach-ath79/ath79.h>
#include <asm/mach-ath79/rb4xx_cpld.h>

#define DRV_NAME        "rb4xx-nand"
#define DRV_VERSION     "0.2.0"
#define DRV_DESC        "NAND flash driver for RouterBoard 4xx series"

#define RB4XX_NAND_GPIO_READY	5
#define RB4XX_NAND_GPIO_ALE	37
#define RB4XX_NAND_GPIO_CLE	38
#define RB4XX_NAND_GPIO_NCE	39

struct rb4xx_nand_info {
	struct nand_chip	chip;
	struct mtd_info		mtd;
};

/*
 * We need to use the OLD Yaffs-1 OOB layout, otherwise the RB bootloader
 * will not be able to find the kernel that we load.
 */
static struct nand_ecclayout rb4xx_nand_ecclayout = {
	.eccbytes	= 6,
	.eccpos		= { 8, 9, 10, 13, 14, 15 },
	.oobavail	= 9,
	.oobfree	= { { 0, 4 }, { 6, 2 }, { 11, 2 }, { 4, 1 } }
};

static struct mtd_partition rb4xx_nand_partitions[] = {
	{
		.name	= "booter",
		.offset	= 0,
		.size	= (256 * 1024),
		.mask_flags = MTD_WRITEABLE,
	},
	{
		.name	= "kernel",
		.offset	= (256 * 1024),
		.size	= (4 * 1024 * 1024) - (256 * 1024),
	},
	{
		.name	= "ubi",
		.offset	= MTDPART_OFS_NXTBLK,
		.size	= MTDPART_SIZ_FULL,
	},
};

static int rb4xx_nand_dev_ready(struct mtd_info *mtd)
{
	return gpio_get_value_cansleep(RB4XX_NAND_GPIO_READY);
}

static void rb4xx_nand_write_cmd(unsigned char cmd)
{
	unsigned char data = cmd;
	int err;

	err = rb4xx_cpld_write(&data, 1);
	if (err)
		pr_err("rb4xx_nand: write cmd failed, err=%d\n", err);
}

static void rb4xx_nand_cmd_ctrl(struct mtd_info *mtd, int cmd,
				unsigned int ctrl)
{
	if (ctrl & NAND_CTRL_CHANGE) {
		gpio_set_value_cansleep(RB4XX_NAND_GPIO_CLE,
					(ctrl & NAND_CLE) ? 1 : 0);
		gpio_set_value_cansleep(RB4XX_NAND_GPIO_ALE,
					(ctrl & NAND_ALE) ? 1 : 0);
		gpio_set_value_cansleep(RB4XX_NAND_GPIO_NCE,
					(ctrl & NAND_NCE) ? 0 : 1);
	}

	if (cmd != NAND_CMD_NONE)
		rb4xx_nand_write_cmd(cmd);
}

static unsigned char rb4xx_nand_read_byte(struct mtd_info *mtd)
{
	unsigned char data = 0;
	int err;

	err = rb4xx_cpld_read(&data, 1);
	if (err) {
		pr_err("rb4xx_nand: read data failed, err=%d\n", err);
		data = 0xff;
	}

	return data;
}

static void rb4xx_nand_write_buf(struct mtd_info *mtd, const unsigned char *buf,
				 int len)
{
	int err;

	err = rb4xx_cpld_write(buf, len);
	if (err)
		pr_err("rb4xx_nand: write buf failed, err=%d\n", err);
}

static void rb4xx_nand_read_buf(struct mtd_info *mtd, unsigned char *buf,
				int len)
{
	int err;

	err = rb4xx_cpld_read(buf, len);
	if (err)
		pr_err("rb4xx_nand: read buf failed, err=%d\n", err);
}

static int rb4xx_nand_probe(struct platform_device *pdev)
{
	struct rb4xx_nand_info	*info;
	int ret;

	printk(KERN_INFO DRV_DESC " version " DRV_VERSION "\n");

	ret = gpio_request(RB4XX_NAND_GPIO_READY, "NAND RDY");
	if (ret) {
		dev_err(&pdev->dev, "unable to request gpio %d\n",
			RB4XX_NAND_GPIO_READY);
		goto err;
	}

	ret = gpio_direction_input(RB4XX_NAND_GPIO_READY);
	if (ret) {
		dev_err(&pdev->dev, "unable to set input mode on gpio %d\n",
			RB4XX_NAND_GPIO_READY);
		goto err_free_gpio_ready;
	}

	ret = gpio_request(RB4XX_NAND_GPIO_ALE, "NAND ALE");
	if (ret) {
		dev_err(&pdev->dev, "unable to request gpio %d\n",
			RB4XX_NAND_GPIO_ALE);
		goto err_free_gpio_ready;
	}

	ret = gpio_direction_output(RB4XX_NAND_GPIO_ALE, 0);
	if (ret) {
		dev_err(&pdev->dev, "unable to set output mode on gpio %d\n",
			RB4XX_NAND_GPIO_ALE);
		goto err_free_gpio_ale;
	}

	ret = gpio_request(RB4XX_NAND_GPIO_CLE, "NAND CLE");
	if (ret) {
		dev_err(&pdev->dev, "unable to request gpio %d\n",
			RB4XX_NAND_GPIO_CLE);
		goto err_free_gpio_ale;
	}

	ret = gpio_direction_output(RB4XX_NAND_GPIO_CLE, 0);
	if (ret) {
		dev_err(&pdev->dev, "unable to set output mode on gpio %d\n",
			RB4XX_NAND_GPIO_CLE);
		goto err_free_gpio_cle;
	}

	ret = gpio_request(RB4XX_NAND_GPIO_NCE, "NAND NCE");
	if (ret) {
		dev_err(&pdev->dev, "unable to request gpio %d\n",
			RB4XX_NAND_GPIO_NCE);
		goto err_free_gpio_cle;
	}

	ret = gpio_direction_output(RB4XX_NAND_GPIO_NCE, 1);
	if (ret) {
		dev_err(&pdev->dev, "unable to set output mode on gpio %d\n",
			RB4XX_NAND_GPIO_ALE);
		goto err_free_gpio_nce;
	}

	info = kzalloc(sizeof(*info), GFP_KERNEL);
	if (!info) {
		dev_err(&pdev->dev, "rb4xx-nand: no memory for private data\n");
		ret = -ENOMEM;
		goto err_free_gpio_nce;
	}

	info->chip.priv	= &info;
	info->mtd.priv	= &info->chip;
	info->mtd.owner	= THIS_MODULE;

	info->chip.cmd_ctrl	= rb4xx_nand_cmd_ctrl;
	info->chip.dev_ready	= rb4xx_nand_dev_ready;
	info->chip.read_byte	= rb4xx_nand_read_byte;
	info->chip.write_buf	= rb4xx_nand_write_buf;
	info->chip.read_buf	= rb4xx_nand_read_buf;

	info->chip.chip_delay	= 25;
	info->chip.ecc.mode	= NAND_ECC_SOFT;
	info->chip.options = NAND_NO_SUBPAGE_WRITE;

	platform_set_drvdata(pdev, info);

	ret = nand_scan_ident(&info->mtd, 1, NULL);
	if (ret) {
		ret = -ENXIO;
		goto err_free_info;
	}

	if (info->mtd.writesize == 512)
		info->chip.ecc.layout = &rb4xx_nand_ecclayout;

	ret = nand_scan_tail(&info->mtd);
	if (ret) {
		return -ENXIO;
		goto err_set_drvdata;
	}

	mtd_device_register(&info->mtd, rb4xx_nand_partitions,
				ARRAY_SIZE(rb4xx_nand_partitions));
	if (ret)
		goto err_release_nand;

	return 0;

err_release_nand:
	nand_release(&info->mtd);
err_set_drvdata:
	platform_set_drvdata(pdev, NULL);
err_free_info:
	kfree(info);
err_free_gpio_nce:
	gpio_free(RB4XX_NAND_GPIO_NCE);
err_free_gpio_cle:
	gpio_free(RB4XX_NAND_GPIO_CLE);
err_free_gpio_ale:
	gpio_free(RB4XX_NAND_GPIO_ALE);
err_free_gpio_ready:
	gpio_free(RB4XX_NAND_GPIO_READY);
err:
	return ret;
}

static int rb4xx_nand_remove(struct platform_device *pdev)
{
	struct rb4xx_nand_info *info = platform_get_drvdata(pdev);

	nand_release(&info->mtd);
	platform_set_drvdata(pdev, NULL);
	kfree(info);
	gpio_free(RB4XX_NAND_GPIO_NCE);
	gpio_free(RB4XX_NAND_GPIO_CLE);
	gpio_free(RB4XX_NAND_GPIO_ALE);
	gpio_free(RB4XX_NAND_GPIO_READY);

	return 0;
}

static struct platform_driver rb4xx_nand_driver = {
	.probe	= rb4xx_nand_probe,
	.remove	= rb4xx_nand_remove,
	.driver	= {
		.name	= DRV_NAME,
		.owner	= THIS_MODULE,
	},
};

static int __init rb4xx_nand_init(void)
{
	return platform_driver_register(&rb4xx_nand_driver);
}

static void __exit rb4xx_nand_exit(void)
{
	platform_driver_unregister(&rb4xx_nand_driver);
}

module_init(rb4xx_nand_init);
module_exit(rb4xx_nand_exit);

MODULE_DESCRIPTION(DRV_DESC);
MODULE_VERSION(DRV_VERSION);
MODULE_AUTHOR("Gabor Juhos <juhosg@openwrt.org>");
MODULE_AUTHOR("Imre Kaloz <kaloz@openwrt.org>");
MODULE_LICENSE("GPL v2");

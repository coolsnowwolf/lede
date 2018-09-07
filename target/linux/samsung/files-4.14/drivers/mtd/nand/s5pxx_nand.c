/*
 * Copyright (c) 2018 Jianhui Zhao <jianhuizhao329@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/delay.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/ioport.h>
#include <linux/of_platform.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/jiffies.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/io.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/rawnand.h>
#include <linux/mtd/nand_ecc.h>
#include <linux/mtd/partitions.h>

#define S5P_NFCONF			0x00
#define S5P_NFCONT			0x04
#define S5P_NFCMD			0x08
#define S5P_NFADDR			0x0c
#define S5P_NFDATA			0x10
#define S5P_NFMECCDATA0		0x14
#define S5P_NFMECCDATA1		0x18
#define S5P_NFSECCDATA		0x1c
#define S5P_NFSBLK			0x20
#define S5P_NFEBLK			0x24
#define S5P_NFSTAT			0x28
#define S5P_NFMECCERR0		0x2c
#define S5P_NFMECCERR1		0x30
#define S5P_NFMECC0			0x34
#define S5P_NFMECC1			0x38
#define S5P_NFSECC			0x3c
#define S5P_NFMLCBITPT		0x40
#define S5P_NF8ECCERR0		0x44 
#define S5P_NF8ECCERR1  	0x48
#define S5P_NF8ECCERR2		0x4C           
#define S5P_NFM8ECC0		0x50         
#define S5P_NFM8ECC1		0x54        
#define S5P_NFM8ECC2		0x58       
#define S5P_NFM8ECC3		0x5C
#define S5P_NFMLC8BITPT0	0x60
#define S5P_NFMLC8BITPT1	0x64

#define S5P_NFECCCONF		0x00
#define S5P_NFECCCONT		0x20
#define S5P_NFECCSTAT		0x30
#define S5P_NFECCSECSTAT	0x40
#define S5P_NFECCPRGECC		0x90
#define S5P_NFECCERL		0xC0
#define S5P_NFECCERP		0xF0

#define S5P_NFCONF_NANDBOOT		(1 << 31)
#define S5P_NFCONF_ECCCLKCON	(1 << 30)
#define S5P_NFCONF_ECC_MLC		(1 << 24)
#define	S5P_NFCONF_ECC_1BIT		(0 << 23)
#define	S5P_NFCONF_ECC_4BIT		(2 << 23)
#define	S5P_NFCONF_ECC_8BIT		(1 << 23)
#define S5P_NFCONF_TACLS(x)		((x) << 12)
#define S5P_NFCONF_TWRPH0(x)	((x) << 8)
#define S5P_NFCONF_TWRPH1(x)	((x) << 4)
#define S5P_NFCONF_MLC			(1 << 3)
#define S5P_NFCONF_PAGESIZE		(1 << 2)
#define S5P_NFCONF_ADDRCYCLE	(1 << 1)
#define S5P_NFCONF_BUSWIDTH		(1 << 0)

#define S5P_NFCONT_ECC_ENC		(1 << 18)
#define S5P_NFCONT_LOCKTGHT		(1 << 17)
#define S5P_NFCONT_LOCKSOFT		(1 << 16)
#define S5P_NFCONT_MECCLOCK		(1 << 7)
#define S5P_NFCONT_SECCLOCK		(1 << 6)
#define S5P_NFCONT_INITMECC		(1 << 5)
#define S5P_NFCONT_INITSECC		(1 << 4)
#define S5P_NFCONT_nFCE1		(1 << 2)
#define S5P_NFCONT_nFCE0		(1 << 1)
#define S5P_NFCONT_MODE			(1 << 0)

#define S5P_NFSTAT_READY		(1 << 0)

#define S5P_NFECCCONT_MECCRESET	(1 << 0)
#define S5P_NFECCCONT_MECCINIT	(1 << 2)
#define S5P_NFECCCONT_ECCDIRWR	(1 << 16)

#define S5P_NFECCSTAT_ECCBUSY	(1 << 31)

enum s5p_cpu_type {
	TYPE_S5PV210,
};

struct s5p_nand_host {
	struct nand_chip	nand_chip;
	void __iomem		*nf_base;
	void __iomem		*ecc_base;
	struct clk 			*clk[2];
	enum s5p_cpu_type	cpu_type;
};

/*
 * See "S5PV210 iROM Application Note" for recommended ECC layout
 * ECC layout for 8-bit ECC (13 bytes/page)
 * Compatible with bl0 bootloader, see iROM appnote
 */
/* new oob placement block for use with hardware ecc generation
 */
static int s5pcxx_ooblayout_ecc(struct mtd_info *mtd, int section,
				 struct mtd_oob_region *oobregion)
{
	if (section)
		return -ERANGE;

	oobregion->offset = 12;
	oobregion->length = 52;

	return 0;
}

static int s5pcxx_ooblayout_free(struct mtd_info *mtd, int section,
				  struct mtd_oob_region *oobregion)
{
	if (section)
		return -ERANGE;

	oobregion->offset = 2;
	oobregion->length = 10;

	return 0;
}

static const struct mtd_ooblayout_ops s5pcxx_ooblayout_ops = {
	.ecc = s5pcxx_ooblayout_ecc,
	.free = s5pcxx_ooblayout_free,
};

static inline void rwl(void *reg, uint32_t rst, uint32_t set)
{
	uint32_t r;
	r = readl(reg);
	r &= ~rst;
	r |= set;
	writel(r, reg);
}

/*
 * Hardware specific access to control-lines function
 */
static void s5p_cmd_ctrl(struct mtd_info *mtd, int dat, unsigned int ctrl)
{
	struct nand_chip *nand_chip = mtd->priv;
	struct s5p_nand_host *host = nand_chip->priv;

	if (dat == NAND_CMD_NONE)
		return;
	
	if (ctrl & NAND_CLE)
		writeb(dat, host->nf_base + S5P_NFCMD);
	else
		writeb(dat, host->nf_base + S5P_NFADDR);
}

/*
 * Function for checking device ready pin
 */
static int s5p_nand_device_ready(struct mtd_info *mtd)
{
	struct nand_chip *nand_chip = mtd->priv;
	struct s5p_nand_host *host = nand_chip->priv;

	/* it's to check the RnB nand signal bit and
	 * return to device ready condition in nand_base.c
	 */
	return readl(host->nf_base + S5P_NFSTAT) & S5P_NFSTAT_READY;
}

static void s3_nand_select_chip(struct mtd_info *mtd, int chip)
{
	struct nand_chip *nand_chip = mtd->priv;
	struct s5p_nand_host *host = nand_chip->priv;
	u32 value = readl(host->nf_base + S5P_NFCONT);

	if (chip == -1)
		value |= S5P_NFCONT_nFCE0;	/* deselect */
	else
		value &= ~S5P_NFCONT_nFCE0;	/* select */

	writel(value, host->nf_base + S5P_NFCONT);
}

static void s5pcxx_nand_enable_hwecc(struct mtd_info *mtd, int mode)
{
	struct nand_chip *chip = mtd->priv;
	struct s5p_nand_host *host = chip->priv;

	uint32_t reg;

	/* Set ECC mode */
	reg = 3; /* 8-bit */
	reg |= (chip->ecc.size - 1) << 16;
	writel(reg, host->ecc_base + S5P_NFECCCONF);

	/* Set ECC direction */
	rwl(host->ecc_base + S5P_NFECCCONT, S5P_NFECCCONT_ECCDIRWR,
	    (mode == NAND_ECC_WRITE) ? S5P_NFECCCONT_ECCDIRWR : 0);

	/* Reset status bits */
	rwl(host->ecc_base + S5P_NFECCSTAT, 0, (1 << 24) | (1 << 25));

	/* Unlock ECC */
	rwl(host->nf_base + S5P_NFCONT, S5P_NFCONT_MECCLOCK, 0);

	/* Initialize ECC */
	rwl(host->ecc_base +S5P_NFECCCONT, 0, S5P_NFECCCONT_MECCINIT);
}

static void readecc(void *eccbase, uint8_t *ecc_code, unsigned ecc_len)
{
	uint32_t i, j, reg;

	for (i = 0; i < ecc_len; i += 4) {
		reg = readl(eccbase + i);
		for (j = 0; (j < 4) && (i + j < ecc_len); ++j) {
			ecc_code[i + j] = reg & 0xFF;
			reg >>= 8;
		}
	}
}

static int s5pcxx_nand_calculate_ecc(struct mtd_info *mtd, const uint8_t *dat, uint8_t *ecc_code)
{
	struct nand_chip *chip = mtd->priv;
	struct s5p_nand_host *host = chip->priv;

	/* Lock ECC */
	rwl(host->nf_base + S5P_NFCONT, 0, S5P_NFCONT_MECCLOCK);

	if (ecc_code)   /* NAND_ECC_WRITE */ {
		/* ECC encoding is completed  */
		while (!(readl(host->ecc_base + S5P_NFECCSTAT) & (1 << 25)));
		readecc(host->ecc_base + S5P_NFECCPRGECC, ecc_code, chip->ecc.bytes);
	} else {	/* NAND_ECC_READ */
		/* ECC decoding is completed  */
        while (!(readl(host->ecc_base + S5P_NFECCSTAT) & (1 << 24)));
	}

	return 0;
}

static int s5pcxx_nand_correct_data(struct mtd_info *mtd, u8 *dat,
				     u8 *read_ecc, u8 *calc_ecc)
{
	int ret = 0;
    u32 errNo;
    u32 erl0, erl1, erl2, erl3, erp0, erp1;
    struct nand_chip *chip = mtd->priv;
	struct s5p_nand_host *host = chip->priv;
 
    /* Wait until the 8-bit ECC decoding engine is Idle */
    while (readl(host->ecc_base + S5P_NFECCSTAT) & (1 << 31));
     
    errNo = readl(host->ecc_base + S5P_NFECCSECSTAT) & 0x1F;
    erl0 = readl(host->ecc_base + S5P_NFECCERL);
    erl1 = readl(host->ecc_base + S5P_NFECCERL + 0x04);
    erl2 = readl(host->ecc_base + S5P_NFECCERL + 0x08);
    erl3 = readl(host->ecc_base + S5P_NFECCERL + 0x0C);
     
    erp0 = readl(host->ecc_base + S5P_NFECCERP);
    erp1 = readl(host->ecc_base + S5P_NFECCERP + 0x04);
     
    switch (errNo) {
    case 8:
        dat[(erl3 >> 16) & 0x3FF] ^= (erp1 >> 24) & 0xFF;
    case 7:
        dat[erl3 & 0x3FF] ^= (erp1 >> 16) & 0xFF;
    case 6:
        dat[(erl2 >> 16) & 0x3FF] ^= (erp1 >> 8) & 0xFF;
    case 5:
        dat[erl2 & 0x3FF] ^= erp1 & 0xFF;
    case 4:
        dat[(erl1 >> 16) & 0x3FF] ^= (erp0 >> 24) & 0xFF;
    case 3:
        dat[erl1 & 0x3FF] ^= (erp0 >> 16) & 0xFF;
    case 2:
        dat[(erl0 >> 16) & 0x3FF] ^= (erp0 >> 8) & 0xFF;
    case 1:
        dat[erl0 & 0x3FF] ^= erp0 & 0xFF;
    case 0:
        break;
    default:
        ret = -1;
        printk("ECC uncorrectable error detected:%d\n", errNo);
        break;
    }
     
    return ret;
}

static int s5pcxx_nand_read_page_hwecc(struct mtd_info *mtd, struct nand_chip *chip,
				uint8_t *buf, int oob_required, int page)
{
	struct mtd_oob_region oobregion = { };
 	int i, eccsize = chip->ecc.size;
    int eccbytes = chip->ecc.bytes;
    int eccsteps = chip->ecc.steps;
    uint8_t *oobecc;
    int col, stat;
     
    /* Read the OOB area first */
    chip->ecc.read_oob(mtd, chip, page);
    mtd_ooblayout_ecc(mtd, 0, &oobregion);
 	oobecc = chip->oob_poi + oobregion.offset;

    for (i = 0, col = 0; eccsteps; eccsteps--, i += eccbytes, buf += eccsize, col += eccsize) {
        chip->cmdfunc(mtd, NAND_CMD_RNDOUT, col, -1);
        chip->ecc.hwctl(mtd, NAND_ECC_READ);
        chip->read_buf(mtd, buf, eccsize);
        chip->write_buf(mtd, oobecc + i, eccbytes);
        chip->ecc.calculate(mtd, NULL, NULL);
        stat = chip->ecc.correct(mtd, buf, NULL, NULL);
        if (stat < 0)
            mtd->ecc_stats.failed++;
        else
            mtd->ecc_stats.corrected += stat;
    }
    return 0;
}

static void s5p_nand_inithw_later(struct mtd_info *mtd)
{
	struct nand_chip *chip = mtd->priv;
	struct s5p_nand_host *host = chip->priv;
	u32 value;
	
	value = readl(host->nf_base + S5P_NFCONF);
	
	if (nand_is_slc(chip)) {
		value &= ~S5P_NFCONF_MLC;

		if (mtd->writesize == 512) {
			value |= S5P_NFCONF_PAGESIZE;
			
		} else {
			value &= ~S5P_NFCONF_PAGESIZE;
		}
	} else {
		value |= S5P_NFCONF_MLC;

		if (mtd->writesize == 4096)
			value &= ~S5P_NFCONF_PAGESIZE;
		else
			value |= S5P_NFCONF_PAGESIZE;
	}
}

static void s5p_nand_inithw(struct s5p_nand_host  *host)
{
	u32 value;

	/* Enable NAND Flash Controller */
	value = readl(host->nf_base + S5P_NFCONT);
	writel(value | S5P_NFCONT_MODE, host->nf_base + S5P_NFCONT);
}

static void s5p_nand_parse_dt(struct s5p_nand_host *host, struct device *dev)
{
	host->cpu_type = (enum s5p_cpu_type)of_device_get_match_data(dev);
}

static int s5p_nand_probe(struct platform_device *pdev)
{
	int ret;
	struct s5p_nand_host *host;
	struct nand_chip *nand_chip;
	struct mtd_info *mtd;
	struct resource *mem;

	/* Allocate memory for the device structure (and zero it) */
	host = devm_kzalloc(&pdev->dev, sizeof(struct s5p_nand_host), GFP_KERNEL);
	if (!host)
		return -ENOMEM;
	
	mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	host->nf_base = devm_ioremap_resource(&pdev->dev, mem);
	if (IS_ERR(host->nf_base))
		return PTR_ERR(host->nf_base);

	mem = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	host->ecc_base = devm_ioremap_resource(&pdev->dev, mem);
	if (IS_ERR(host->ecc_base))
		return PTR_ERR(host->ecc_base);

	nand_chip = &host->nand_chip;
	nand_chip->priv = host;
    nand_set_flash_node(nand_chip, pdev->dev.of_node);

    mtd = nand_to_mtd(nand_chip);
	mtd->dev.parent = &pdev->dev;
	mtd->priv = nand_chip;

	/* Disable chip select and Enable NAND Flash Controller */
	writel((0x1 << 1) | (0x1 << 0), host->nf_base + S5P_NFCONT);

	/* Set address of NAND IO lines */
	nand_chip->IO_ADDR_R = host->nf_base + S5P_NFDATA;
	nand_chip->IO_ADDR_W = host->nf_base + S5P_NFDATA;

	platform_set_drvdata(pdev, host);
	
	/* get the clock source and enable it */
	host->clk[0] = devm_clk_get(&pdev->dev, "nandxl");
	if (IS_ERR(host->clk[0])) {
		dev_err(&pdev->dev, "cannot get clock of nandxl\n");
		return -ENOENT;
	}
	clk_prepare_enable(host->clk[0]);

	host->clk[1] = devm_clk_get(&pdev->dev, "nand");
	if (IS_ERR(host->clk[1])) {
		dev_err(&pdev->dev, "cannot get clock of nand\n");
		return -ENOENT;
	}
	clk_prepare_enable(host->clk[1]);

	s5p_nand_parse_dt(host, &pdev->dev);

	nand_chip->select_chip = s3_nand_select_chip;
	nand_chip->cmd_ctrl = s5p_cmd_ctrl;
	nand_chip->dev_ready = s5p_nand_device_ready;

	s5p_nand_inithw(host);

	ret = nand_scan_ident(mtd, 1, NULL);
	if (ret)
		return ret;

	if (nand_chip->ecc.mode == NAND_ECC_HW) {
		nand_chip->ecc.correct = s5pcxx_nand_correct_data;
		nand_chip->ecc.calculate = s5pcxx_nand_calculate_ecc;
		nand_chip->ecc.hwctl = s5pcxx_nand_enable_hwecc;
		nand_chip->ecc.read_page = s5pcxx_nand_read_page_hwecc;

		nand_chip->ecc.size = 512;
		nand_chip->ecc.bytes = 13;

		mtd_set_ooblayout(nand_to_mtd(nand_chip), &s5pcxx_ooblayout_ops);
	}

	ret = nand_scan_tail(mtd);
	if (ret)
		return ret;

	/* After you get the actual hardware information */
	s5p_nand_inithw_later(mtd);

	return mtd_device_parse_register(mtd, NULL, NULL, NULL, 0);
}

static int s5p_nand_remove(struct platform_device *pdev)
{
	struct s5p_nand_host *host = platform_get_drvdata(pdev);
	struct mtd_info *mtd = nand_to_mtd(&host->nand_chip);

	nand_release(mtd);
	clk_disable_unprepare(host->clk[0]);	/* nandxl */
	clk_disable_unprepare(host->clk[1]);	/* nand */

	return 0;
}

static const struct of_device_id s5p_nand_match[] = {
	{ .compatible = "samsung,s5pv210-nand", .data = TYPE_S5PV210 },
	{},
};
MODULE_DEVICE_TABLE(of, s5p_nand_match);

static struct platform_driver s5p_nand_driver = {
	.probe          = s5p_nand_probe,
	.remove         = s5p_nand_remove,
	.driver         = {
		.name   = "s5p-nand",
		.owner  = THIS_MODULE,
		.of_match_table = s5p_nand_match,
	},
};
module_platform_driver(s5p_nand_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jianhui Zhao <jianhuizhao329@gmail.com>");
MODULE_DESCRIPTION("S5Pxx MTD NAND driver");

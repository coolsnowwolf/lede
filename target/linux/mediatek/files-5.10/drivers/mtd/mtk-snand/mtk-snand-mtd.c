// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2020 MediaTek Inc. All Rights Reserved.
 *
 * Author: Weijie Gao <weijie.gao@mediatek.com>
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/mutex.h>
#include <linux/clk.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/dma-mapping.h>
#include <linux/wait.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/of_platform.h>

#include "mtk-snand.h"
#include "mtk-snand-os.h"

struct mtk_snand_of_id {
	enum mtk_snand_soc soc;
};

struct mtk_snand_mtd {
	struct mtk_snand_plat_dev pdev;

	struct clk *nfi_clk;
	struct clk *pad_clk;
	struct clk *ecc_clk;

	void __iomem *nfi_regs;
	void __iomem *ecc_regs;

	int irq;

	bool quad_spi;
	enum mtk_snand_soc soc;

	struct mtd_info mtd;
	struct mtk_snand *snf;
	struct mtk_snand_chip_info cinfo;
	uint8_t *page_cache;
	struct mutex lock;
};

#define mtd_to_msm(mtd) container_of(mtd, struct mtk_snand_mtd, mtd)

static int mtk_snand_mtd_erase(struct mtd_info *mtd, struct erase_info *instr)
{
	struct mtk_snand_mtd *msm = mtd_to_msm(mtd);
	u64 start_addr, end_addr;
	int ret;

	/* Do not allow write past end of device */
	if ((instr->addr + instr->len) > msm->cinfo.chipsize) {
		dev_err(msm->pdev.dev,
			"attempt to erase beyond end of device\n");
		return -EINVAL;
	}

	start_addr = instr->addr & (~mtd->erasesize_mask);
	end_addr = instr->addr + instr->len;
	if (end_addr & mtd->erasesize_mask) {
		end_addr = (end_addr + mtd->erasesize_mask) &
			   (~mtd->erasesize_mask);
	}

	mutex_lock(&msm->lock);

	while (start_addr < end_addr) {
		if (mtk_snand_block_isbad(msm->snf, start_addr)) {
			instr->fail_addr = start_addr;
			ret = -EIO;
			break;
		}

		ret = mtk_snand_erase_block(msm->snf, start_addr);
		if (ret) {
			instr->fail_addr = start_addr;
			break;
		}

		start_addr += mtd->erasesize;
	}

	mutex_unlock(&msm->lock);

	return ret;
}

static int mtk_snand_mtd_read_data(struct mtk_snand_mtd *msm, uint64_t addr,
				   struct mtd_oob_ops *ops)
{
	struct mtd_info *mtd = &msm->mtd;
	size_t len, ooblen, maxooblen, chklen;
	uint32_t col, ooboffs;
	uint8_t *datcache, *oobcache;
	bool ecc_failed = false, raw = ops->mode == MTD_OPS_RAW ? true : false;
	int ret, max_bitflips = 0;

	col = addr & mtd->writesize_mask;
	addr &= ~mtd->writesize_mask;
	maxooblen = mtd_oobavail(mtd, ops);
	ooboffs = ops->ooboffs;
	ooblen = ops->ooblen;
	len = ops->len;

	datcache = len ? msm->page_cache : NULL;
	oobcache = ooblen ? msm->page_cache + mtd->writesize : NULL;

	ops->oobretlen = 0;
	ops->retlen = 0;

	while (len || ooblen) {
		if (ops->mode == MTD_OPS_AUTO_OOB)
			ret = mtk_snand_read_page_auto_oob(msm->snf, addr,
				datcache, oobcache, maxooblen, NULL, raw);
		else
			ret = mtk_snand_read_page(msm->snf, addr, datcache,
				oobcache, raw);

		if (ret < 0 && ret != -EBADMSG)
			return ret;

		if (ret == -EBADMSG) {
			mtd->ecc_stats.failed++;
			ecc_failed = true;
		} else {
			mtd->ecc_stats.corrected += ret;
			max_bitflips = max_t(int, ret, max_bitflips);
		}

		if (len) {
			/* Move data */
			chklen = mtd->writesize - col;
			if (chklen > len)
				chklen = len;

			memcpy(ops->datbuf + ops->retlen, datcache + col,
			       chklen);
			len -= chklen;
			col = 0; /* (col + chklen) %  */
			ops->retlen += chklen;
		}

		if (ooblen) {
			/* Move oob */
			chklen = maxooblen - ooboffs;
			if (chklen > ooblen)
				chklen = ooblen;

			memcpy(ops->oobbuf + ops->oobretlen, oobcache + ooboffs,
			       chklen);
			ooblen -= chklen;
			ooboffs = 0; /* (ooboffs + chklen) % maxooblen; */
			ops->oobretlen += chklen;
		}

		addr += mtd->writesize;
	}

	return ecc_failed ? -EBADMSG : max_bitflips;
}

static int mtk_snand_mtd_read_oob(struct mtd_info *mtd, loff_t from,
				  struct mtd_oob_ops *ops)
{
	struct mtk_snand_mtd *msm = mtd_to_msm(mtd);
	uint32_t maxooblen;
	int ret;

	if (!ops->oobbuf && !ops->datbuf) {
		if (ops->ooblen || ops->len)
			return -EINVAL;

		return 0;
	}

	switch (ops->mode) {
	case MTD_OPS_PLACE_OOB:
	case MTD_OPS_AUTO_OOB:
	case MTD_OPS_RAW:
		break;
	default:
		dev_err(msm->pdev.dev, "unsupported oob mode: %u\n", ops->mode);
		return -EINVAL;
	}

	maxooblen = mtd_oobavail(mtd, ops);

	/* Do not allow read past end of device */
	if (ops->datbuf && (from + ops->len) > msm->cinfo.chipsize) {
		dev_err(msm->pdev.dev,
			"attempt to read beyond end of device\n");
		return -EINVAL;
	}

	if (unlikely(ops->ooboffs >= maxooblen)) {
		dev_err(msm->pdev.dev, "attempt to start read outside oob\n");
		return -EINVAL;
	}

	if (unlikely(from >= msm->cinfo.chipsize ||
		     ops->ooboffs + ops->ooblen >
			     ((msm->cinfo.chipsize >> mtd->writesize_shift) -
			      (from >> mtd->writesize_shift)) *
				     maxooblen)) {
		dev_err(msm->pdev.dev,
			"attempt to read beyond end of device\n");
		return -EINVAL;
	}

	mutex_lock(&msm->lock);
	ret = mtk_snand_mtd_read_data(msm, from, ops);
	mutex_unlock(&msm->lock);

	return ret;
}

static int mtk_snand_mtd_write_data(struct mtk_snand_mtd *msm, uint64_t addr,
				    struct mtd_oob_ops *ops)
{
	struct mtd_info *mtd = &msm->mtd;
	size_t len, ooblen, maxooblen, chklen, oobwrlen;
	uint32_t col, ooboffs;
	uint8_t *datcache, *oobcache;
	bool raw = ops->mode == MTD_OPS_RAW ? true : false;
	int ret;

	col = addr & mtd->writesize_mask;
	addr &= ~mtd->writesize_mask;
	maxooblen = mtd_oobavail(mtd, ops);
	ooboffs = ops->ooboffs;
	ooblen = ops->ooblen;
	len = ops->len;

	datcache = len ? msm->page_cache : NULL;
	oobcache = ooblen ? msm->page_cache + mtd->writesize : NULL;

	ops->oobretlen = 0;
	ops->retlen = 0;

	while (len || ooblen) {
		if (len) {
			/* Move data */
			chklen = mtd->writesize - col;
			if (chklen > len)
				chklen = len;

			memset(datcache, 0xff, col);
			memcpy(datcache + col, ops->datbuf + ops->retlen,
			       chklen);
			memset(datcache + col + chklen, 0xff,
			       mtd->writesize - col - chklen);
			len -= chklen;
			col = 0; /* (col + chklen) %  */
			ops->retlen += chklen;
		}

		oobwrlen = 0;
		if (ooblen) {
			/* Move oob */
			chklen = maxooblen - ooboffs;
			if (chklen > ooblen)
				chklen = ooblen;

			memset(oobcache, 0xff, ooboffs);
			memcpy(oobcache + ooboffs,
			       ops->oobbuf + ops->oobretlen, chklen);
			memset(oobcache + ooboffs + chklen, 0xff,
			       mtd->oobsize - ooboffs - chklen);
			oobwrlen = chklen + ooboffs;
			ooblen -= chklen;
			ooboffs = 0; /* (ooboffs + chklen) % maxooblen; */
			ops->oobretlen += chklen;
		}

		if (ops->mode == MTD_OPS_AUTO_OOB)
			ret = mtk_snand_write_page_auto_oob(msm->snf, addr,
				datcache, oobcache, oobwrlen, NULL, raw);
		else
			ret = mtk_snand_write_page(msm->snf, addr, datcache,
				oobcache, raw);

		if (ret)
			return ret;

		addr += mtd->writesize;
	}

	return 0;
}

static int mtk_snand_mtd_write_oob(struct mtd_info *mtd, loff_t to,
				   struct mtd_oob_ops *ops)
{
	struct mtk_snand_mtd *msm = mtd_to_msm(mtd);
	uint32_t maxooblen;
	int ret;

	if (!ops->oobbuf && !ops->datbuf) {
		if (ops->ooblen || ops->len)
			return -EINVAL;

		return 0;
	}

	switch (ops->mode) {
	case MTD_OPS_PLACE_OOB:
	case MTD_OPS_AUTO_OOB:
	case MTD_OPS_RAW:
		break;
	default:
		dev_err(msm->pdev.dev, "unsupported oob mode: %u\n", ops->mode);
		return -EINVAL;
	}

	maxooblen = mtd_oobavail(mtd, ops);

	/* Do not allow write past end of device */
	if (ops->datbuf && (to + ops->len) > msm->cinfo.chipsize) {
		dev_err(msm->pdev.dev,
			"attempt to write beyond end of device\n");
		return -EINVAL;
	}

	if (unlikely(ops->ooboffs >= maxooblen)) {
		dev_err(msm->pdev.dev,
			"attempt to start write outside oob\n");
		return -EINVAL;
	}

	if (unlikely(to >= msm->cinfo.chipsize ||
		     ops->ooboffs + ops->ooblen >
			     ((msm->cinfo.chipsize >> mtd->writesize_shift) -
			      (to >> mtd->writesize_shift)) *
				     maxooblen)) {
		dev_err(msm->pdev.dev,
			"attempt to write beyond end of device\n");
		return -EINVAL;
	}

	mutex_lock(&msm->lock);
	ret = mtk_snand_mtd_write_data(msm, to, ops);
	mutex_unlock(&msm->lock);

	return ret;
}

static int mtk_snand_mtd_block_isbad(struct mtd_info *mtd, loff_t offs)
{
	struct mtk_snand_mtd *msm = mtd_to_msm(mtd);
	int ret;

	mutex_lock(&msm->lock);
	ret = mtk_snand_block_isbad(msm->snf, offs);
	mutex_unlock(&msm->lock);

	return ret;
}

static int mtk_snand_mtd_block_markbad(struct mtd_info *mtd, loff_t offs)
{
	struct mtk_snand_mtd *msm = mtd_to_msm(mtd);
	int ret;

	mutex_lock(&msm->lock);
	ret = mtk_snand_block_markbad(msm->snf, offs);
	mutex_unlock(&msm->lock);

	return ret;
}

static int mtk_snand_ooblayout_ecc(struct mtd_info *mtd, int section,
				   struct mtd_oob_region *oobecc)
{
	struct mtk_snand_mtd *msm = mtd_to_msm(mtd);

	if (section)
		return -ERANGE;

	oobecc->offset = msm->cinfo.fdm_size * msm->cinfo.num_sectors;
	oobecc->length = mtd->oobsize - oobecc->offset;

	return 0;
}

static int mtk_snand_ooblayout_free(struct mtd_info *mtd, int section,
				    struct mtd_oob_region *oobfree)
{
	struct mtk_snand_mtd *msm = mtd_to_msm(mtd);

	if (section >= msm->cinfo.num_sectors)
		return -ERANGE;

	oobfree->length = msm->cinfo.fdm_size - 1;
	oobfree->offset = section * msm->cinfo.fdm_size + 1;

	return 0;
}

static irqreturn_t mtk_snand_irq(int irq, void *id)
{
	struct mtk_snand_mtd *msm = id;
	int ret;

	ret = mtk_snand_irq_process(msm->snf);
	if (ret > 0)
		return IRQ_HANDLED;

	return IRQ_NONE;
}

static int mtk_snand_enable_clk(struct mtk_snand_mtd *msm)
{
	int ret;

	ret = clk_prepare_enable(msm->nfi_clk);
	if (ret) {
		dev_err(msm->pdev.dev, "unable to enable nfi clk\n");
		return ret;
	}

	ret = clk_prepare_enable(msm->pad_clk);
	if (ret) {
		dev_err(msm->pdev.dev, "unable to enable pad clk\n");
		clk_disable_unprepare(msm->nfi_clk);
		return ret;
	}

	ret = clk_prepare_enable(msm->ecc_clk);
	if (ret) {
		dev_err(msm->pdev.dev, "unable to enable ecc clk\n");
		clk_disable_unprepare(msm->nfi_clk);
		clk_disable_unprepare(msm->pad_clk);
		return ret;
	}

	return 0;
}

static void mtk_snand_disable_clk(struct mtk_snand_mtd *msm)
{
	clk_disable_unprepare(msm->nfi_clk);
	clk_disable_unprepare(msm->pad_clk);
	clk_disable_unprepare(msm->ecc_clk);
}

static const struct mtd_ooblayout_ops mtk_snand_ooblayout = {
	.ecc = mtk_snand_ooblayout_ecc,
	.free = mtk_snand_ooblayout_free,
};

static struct mtk_snand_of_id mt7622_soc_id = { .soc = SNAND_SOC_MT7622 };
static struct mtk_snand_of_id mt7629_soc_id = { .soc = SNAND_SOC_MT7629 };

static const struct of_device_id mtk_snand_ids[] = {
	{ .compatible = "mediatek,mt7622-snand", .data = &mt7622_soc_id },
	{ .compatible = "mediatek,mt7629-snand", .data = &mt7629_soc_id },
	{ },
};

MODULE_DEVICE_TABLE(of, mtk_snand_ids);

static int mtk_snand_probe(struct platform_device *pdev)
{
	struct mtk_snand_platdata mtk_snand_pdata = {};
	struct device_node *np = pdev->dev.of_node;
	const struct of_device_id *of_soc_id;
	const struct mtk_snand_of_id *soc_id;
	struct mtk_snand_mtd *msm;
	struct mtd_info *mtd;
	struct resource *r;
	uint32_t size;
	int ret;

	of_soc_id = of_match_node(mtk_snand_ids, np);
	if (!of_soc_id)
		return -EINVAL;

	soc_id = of_soc_id->data;

	msm = devm_kzalloc(&pdev->dev, sizeof(*msm), GFP_KERNEL);
	if (!msm)
		return -ENOMEM;

	r = platform_get_resource_byname(pdev, IORESOURCE_MEM, "nfi");
	msm->nfi_regs = devm_ioremap_resource(&pdev->dev, r);
	if (IS_ERR(msm->nfi_regs)) {
		ret = PTR_ERR(msm->nfi_regs);
		goto errout1;
	}

	r = platform_get_resource_byname(pdev, IORESOURCE_MEM, "ecc");
	msm->ecc_regs = devm_ioremap_resource(&pdev->dev, r);
	if (IS_ERR(msm->ecc_regs)) {
		ret = PTR_ERR(msm->ecc_regs);
		goto errout1;
	}

	msm->pdev.dev = &pdev->dev;
	msm->quad_spi = of_property_read_bool(np, "mediatek,quad-spi");
	msm->soc = soc_id->soc;

	msm->nfi_clk = devm_clk_get(msm->pdev.dev, "nfi_clk");
	if (IS_ERR(msm->nfi_clk)) {
		ret = PTR_ERR(msm->nfi_clk);
		dev_err(msm->pdev.dev, "unable to get nfi_clk, err = %d\n",
			ret);
		goto errout1;
	}

	msm->ecc_clk = devm_clk_get(msm->pdev.dev, "ecc_clk");
	if (IS_ERR(msm->ecc_clk)) {
		ret = PTR_ERR(msm->ecc_clk);
		dev_err(msm->pdev.dev, "unable to get ecc_clk, err = %d\n",
			ret);
		goto errout1;
	}

	msm->pad_clk = devm_clk_get(msm->pdev.dev, "pad_clk");
	if (IS_ERR(msm->pad_clk)) {
		ret = PTR_ERR(msm->pad_clk);
		dev_err(msm->pdev.dev, "unable to get pad_clk, err = %d\n",
			ret);
		goto errout1;
	}

	ret = mtk_snand_enable_clk(msm);
	if (ret)
		goto errout1;

	/* Probe SPI-NAND Flash */
	mtk_snand_pdata.soc = msm->soc;
	mtk_snand_pdata.quad_spi = msm->quad_spi;
	mtk_snand_pdata.nfi_base = msm->nfi_regs;
	mtk_snand_pdata.ecc_base = msm->ecc_regs;

	ret = mtk_snand_init(&msm->pdev, &mtk_snand_pdata, &msm->snf);
	if (ret)
		goto errout1;

	msm->irq = platform_get_irq(pdev, 0);
	if (msm->irq >= 0) {
		ret = devm_request_irq(msm->pdev.dev, msm->irq, mtk_snand_irq,
				       0x0, "mtk-snand", msm);
		if (ret) {
			dev_err(msm->pdev.dev, "failed to request snfi irq\n");
			goto errout2;
		}

		ret = dma_set_mask(msm->pdev.dev, DMA_BIT_MASK(32));
		if (ret) {
			dev_err(msm->pdev.dev, "failed to set dma mask\n");
			goto errout3;
		}
	}

	mtk_snand_get_chip_info(msm->snf, &msm->cinfo);

	size = msm->cinfo.pagesize + msm->cinfo.sparesize;
	msm->page_cache = devm_kmalloc(msm->pdev.dev, size, GFP_KERNEL);
	if (!msm->page_cache) {
		dev_err(msm->pdev.dev, "failed to allocate page cache\n");
		ret = -ENOMEM;
		goto errout3;
	}

	mutex_init(&msm->lock);

	dev_info(msm->pdev.dev,
		 "chip is %s, size %lluMB, page size %u, oob size %u\n",
		 msm->cinfo.model, msm->cinfo.chipsize >> 20,
		 msm->cinfo.pagesize, msm->cinfo.sparesize);

	/* Initialize mtd for SPI-NAND */
	mtd = &msm->mtd;

	mtd->owner = THIS_MODULE;
	mtd->dev.parent = &pdev->dev;
	mtd->type = MTD_NANDFLASH;
	mtd->flags = MTD_CAP_NANDFLASH;

	mtd_set_of_node(mtd, np);

	mtd->size = msm->cinfo.chipsize;
	mtd->erasesize = msm->cinfo.blocksize;
	mtd->writesize = msm->cinfo.pagesize;
	mtd->writebufsize = mtd->writesize;
	mtd->oobsize = msm->cinfo.sparesize;
	mtd->oobavail = msm->cinfo.num_sectors * (msm->cinfo.fdm_size - 1);

	mtd->erasesize_shift = ffs(mtd->erasesize) - 1;
	mtd->writesize_shift = ffs(mtd->writesize) - 1;
	mtd->erasesize_mask = (1 << mtd->erasesize_shift) - 1;
	mtd->writesize_mask = (1 << mtd->writesize_shift) - 1;

	mtd->ooblayout = &mtk_snand_ooblayout;

	mtd->ecc_strength = msm->cinfo.ecc_strength;
	mtd->bitflip_threshold = (mtd->ecc_strength * 3) / 4;
	mtd->ecc_step_size = msm->cinfo.sector_size;

	mtd->_erase = mtk_snand_mtd_erase;
	mtd->_read_oob = mtk_snand_mtd_read_oob;
	mtd->_write_oob = mtk_snand_mtd_write_oob;
	mtd->_block_isbad = mtk_snand_mtd_block_isbad;
	mtd->_block_markbad = mtk_snand_mtd_block_markbad;

	ret = mtd_device_register(mtd, NULL, 0);
	if (ret) {
		dev_err(msm->pdev.dev, "failed to register mtd partition\n");
		goto errout4;
	}

	platform_set_drvdata(pdev, msm);

	return 0;

errout4:
	devm_kfree(msm->pdev.dev, msm->page_cache);

errout3:
	if (msm->irq >= 0)
		devm_free_irq(msm->pdev.dev, msm->irq, msm);

errout2:
	mtk_snand_cleanup(msm->snf);

errout1:
	devm_kfree(msm->pdev.dev, msm);

	platform_set_drvdata(pdev, NULL);

	return ret;
}

static int mtk_snand_remove(struct platform_device *pdev)
{
	struct mtk_snand_mtd *msm = platform_get_drvdata(pdev);
	struct mtd_info *mtd = &msm->mtd;
	int ret;

	ret = mtd_device_unregister(mtd);
	if (ret)
		return ret;

	mtk_snand_cleanup(msm->snf);

	if (msm->irq >= 0)
		devm_free_irq(msm->pdev.dev, msm->irq, msm);

	mtk_snand_disable_clk(msm);

	devm_kfree(msm->pdev.dev, msm->page_cache);
	devm_kfree(msm->pdev.dev, msm);

	platform_set_drvdata(pdev, NULL);

	return 0;
}

static struct platform_driver mtk_snand_driver = {
	.probe = mtk_snand_probe,
	.remove = mtk_snand_remove,
	.driver = {
		.name = "mtk-snand",
		.of_match_table = mtk_snand_ids,
	},
};

module_platform_driver(mtk_snand_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Weijie Gao <weijie.gao@mediatek.com>");
MODULE_DESCRIPTION("MeidaTek SPI-NAND Flash Controller Driver");

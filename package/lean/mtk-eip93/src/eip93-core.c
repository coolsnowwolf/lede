/* SPDX-License-Identifier: GPL-2.0
 *
 * Copyright (C) 2019 - 2020
 *
 * Richard van Schagen <vschagen@cs.com>
 */
//#define DEBUG 1
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/dmapool.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/spinlock.h>
#include <linux/types.h>

#include <crypto/internal/aead.h>
#include <crypto/internal/skcipher.h>
#include <crypto/internal/hash.h>

#include "eip93-regs.h"
#include "eip93-common.h"
#include "eip93-core.h"
#include "eip93-ring.h"
#include "eip93-cipher.h"
#include "eip93-prng.h"

static struct mtk_alg_template *mtk_algs[] = {
	&mtk_alg_ecb_des,
	&mtk_alg_cbc_des,
	&mtk_alg_ecb_des3_ede,
	&mtk_alg_cbc_des3_ede,
	&mtk_alg_ecb_aes,
	&mtk_alg_cbc_aes,
	&mtk_alg_ctr_aes,
	&mtk_alg_rfc3686_aes,
	&mtk_alg_authenc_hmac_md5_cbc_des,
	&mtk_alg_authenc_hmac_sha1_cbc_des,
	&mtk_alg_authenc_hmac_sha224_cbc_des,
	&mtk_alg_authenc_hmac_sha256_cbc_des,
	&mtk_alg_authenc_hmac_md5_cbc_des3_ede,
	&mtk_alg_authenc_hmac_sha1_cbc_des3_ede,
	&mtk_alg_authenc_hmac_sha224_cbc_des3_ede,
	&mtk_alg_authenc_hmac_sha256_cbc_des3_ede,
	&mtk_alg_authenc_hmac_md5_cbc_aes,
	&mtk_alg_authenc_hmac_sha1_cbc_aes,
	&mtk_alg_authenc_hmac_sha224_cbc_aes,
	&mtk_alg_authenc_hmac_sha256_cbc_aes,
	&mtk_alg_authenc_hmac_md5_rfc3686_aes,
	&mtk_alg_authenc_hmac_sha1_rfc3686_aes,
	&mtk_alg_authenc_hmac_sha224_rfc3686_aes,
	&mtk_alg_authenc_hmac_sha256_rfc3686_aes,
	&mtk_alg_authenc_hmac_md5_ecb_null,
	&mtk_alg_authenc_hmac_sha1_ecb_null,
	&mtk_alg_authenc_hmac_sha224_ecb_null,
	&mtk_alg_authenc_hmac_sha256_ecb_null,
	&mtk_alg_echainiv_authenc_hmac_md5_cbc_des,
	&mtk_alg_echainiv_authenc_hmac_sha1_cbc_aes,
	&mtk_alg_echainiv_authenc_hmac_sha256_cbc_aes,
//	&mtk_alg_seqiv_authenc_hmac_sha1_rfc3686_aes,
//	&mtk_alg_seqiv_authenc_hmac_sha256_rfc3686_aes,
//	&mtk_alg_prng,
//	&mtk_alg_cprng,
};

static void mtk_unregister_algs(struct mtk_device *mtk, int i)
{
	int j;

	for (j = 0; j < i; j++) {
		switch (mtk_algs[j]->type) {
		case MTK_ALG_TYPE_SKCIPHER:
			dev_dbg(mtk->dev, "unregistering: %s",
				mtk_algs[j]->alg.skcipher.base.cra_name);
			crypto_unregister_skcipher(&mtk_algs[j]->alg.skcipher);
			break;
		case MTK_ALG_TYPE_AEAD:
			dev_dbg(mtk->dev, "unregistering: %s",
				mtk_algs[j]->alg.aead.base.cra_name);
			crypto_unregister_aead(&mtk_algs[j]->alg.aead);
			break;
		case MTK_ALG_TYPE_AHASH:
			dev_dbg(mtk->dev, "unregistering: %s",
				mtk_algs[j]->alg.ahash.halg.base.cra_name);
			crypto_unregister_ahash(&mtk_algs[j]->alg.ahash);
			break;
		case MTK_ALG_TYPE_PRNG:
			dev_dbg(mtk->dev, "unregistering: %s",
				mtk_algs[j]->alg.rng.base.cra_name);
			crypto_unregister_rng(&mtk_algs[j]->alg.rng);
		}
	}
}

static int mtk_register_algs(struct mtk_device *mtk)
{
	int i, ret = 0;

	for (i = 0; i < ARRAY_SIZE(mtk_algs); i++) {
		mtk_algs[i]->mtk = mtk;

		switch (mtk_algs[i]->type) {
		case MTK_ALG_TYPE_SKCIPHER:
			dev_dbg(mtk->dev, "registering: %s",
				mtk_algs[i]->alg.skcipher.base.cra_name);
			ret = crypto_register_skcipher(&mtk_algs[i]->alg.skcipher);
			break;
		case MTK_ALG_TYPE_AEAD:
			dev_dbg(mtk->dev, "registering: %s",
				mtk_algs[i]->alg.aead.base.cra_name);
			ret = crypto_register_aead(&mtk_algs[i]->alg.aead);
			break;
		case MTK_ALG_TYPE_AHASH:
			dev_dbg(mtk->dev, "registering: %s",
				mtk_algs[i]->alg.ahash.halg.base.cra_name);
			ret = crypto_register_ahash(&mtk_algs[i]->alg.ahash);
			break;
		case MTK_ALG_TYPE_PRNG:
			dev_dbg(mtk->dev, "registering: %s",
				mtk_algs[i]->alg.rng.base.cra_name);
			ret = crypto_register_rng(&mtk_algs[i]->alg.rng);
		}
		if (ret)
			goto fail;
	}

	return 0;

fail:
	mtk_unregister_algs(mtk, i);

	return ret;
}

static inline void mtk_irq_disable(struct mtk_device *mtk, u32 mask)
{
	__raw_writel(mask, mtk->base + EIP93_REG_MASK_DISABLE);
	__raw_readl(mtk->base + EIP93_REG_MASK_DISABLE);
}

static inline void mtk_irq_enable(struct mtk_device *mtk, u32 mask)
{
	__raw_writel(mask, mtk->base + EIP93_REG_MASK_ENABLE);
	__raw_readl(mtk->base + EIP93_REG_MASK_ENABLE);
}

static inline void mtk_irq_clear(struct mtk_device *mtk, u32 mask)
{
	__raw_writel(mask, mtk->base + EIP93_REG_INT_CLR);
	__raw_readl(mtk->base + EIP93_REG_INT_CLR);
}

inline void mtk_push_request(struct mtk_device *mtk, int DescriptorPendingCount)
{
	int DescriptorCountDone = MTK_RING_SIZE - 1;
	int DescriptorDoneTimeout = 3;

	DescriptorPendingCount = min_t(int, mtk->ring->requests, 32);

	if (!DescriptorPendingCount)
		return;

	writel(BIT(31) | (DescriptorCountDone & GENMASK(10, 0)) |
		(((DescriptorPendingCount - 1) & GENMASK(10, 0)) << 16) |
		((DescriptorDoneTimeout  & GENMASK(4, 0)) << 26),
		mtk->base + EIP93_REG_PE_RING_THRESH);
}

static void mtk_handle_result_descriptor(struct mtk_device *mtk)
{
	struct crypto_async_request *async = NULL;
	struct eip93_descriptor_s *rdesc;
	int handled = 0, nreq;
	int try, ret, err = 0;
	volatile int done1, done2;
	bool last_entry = false;
	bool complete = false;
	u32 flags;

get_more:
	nreq = readl(mtk->base + EIP93_REG_PE_RD_COUNT) & GENMASK(10, 0);

	while (nreq) {
		rdesc = mtk_get_descriptor(mtk);
		if (IS_ERR(rdesc)) {
			dev_err(mtk->dev, "Ndesc: %d nreq: %d\n", handled, nreq);
			ret = -EIO;
			break;
		}
		/* make sure EIP93 finished writing all data
		 * (volatile int) used since bits will be updated via DMA
		*/
		try = 0;
		while (try < 1000) {
			done1 = (volatile int)rdesc->peCrtlStat.bits.peReady;
			done2 = (volatile int)rdesc->peLength.bits.peReady;
			if ((!done1) || (!done2)) {
					try++;
					cpu_relax();
					continue;
			}
			break;
		}
		/*
		if (try)
			dev_err(mtk->dev, "EIP93 try-count: %d", try);
		*/
		err = rdesc->peCrtlStat.bits.errStatus;
		if (err) {
			dev_err(mtk->dev, "Err: %02x\n", err);
		}

		handled++;

		flags = rdesc->userId;
		if (flags & MTK_DESC_FINISH)
			complete = true;

		if (flags & MTK_DESC_LAST) {
			last_entry = true;
			break;
		}
		nreq--;
	}

	if (last_entry) {
		last_entry = false;
		if (flags & MTK_DESC_PRNG)
			mtk_prng_done(mtk, err);

		if (flags & MTK_DESC_SKCIPHER) {
			async = (struct crypto_async_request *)rdesc->arc4Addr;
			mtk_skcipher_handle_result(mtk, async, complete, err);
		}

		if (flags & MTK_DESC_AEAD) {
			async = (struct crypto_async_request *)rdesc->arc4Addr;
			mtk_aead_handle_result(mtk, async, complete, err);
		}

	}

	if (handled) {
		writel(handled, mtk->base + EIP93_REG_PE_RD_COUNT);
		spin_lock(&mtk->ring->lock);
		mtk->ring->requests -= handled;
		if (!mtk->ring->requests) {
			mtk->ring->busy = false;
			spin_unlock(&mtk->ring->lock);
			goto queue_done;
		}
		spin_unlock(&mtk->ring->lock);
		handled = 0;
		goto get_more;
	}

	spin_lock(&mtk->ring->lock);
	if (mtk->ring->requests)
		mtk_push_request(mtk, mtk->ring->requests);
	else
		mtk->ring->busy = false;

	spin_unlock(&mtk->ring->lock);
queue_done:
	mtk_irq_enable(mtk, BIT(1));
}

static irqreturn_t mtk_irq_handler(int irq, void *dev_id)
{
	struct mtk_device *mtk = (struct mtk_device *)dev_id;
	u32 irq_status;

	irq_status = readl(mtk->base + EIP93_REG_INT_MASK_STAT);

	if (irq_status & BIT(1)) {
		mtk_irq_clear(mtk, BIT(1));
		mtk_irq_disable(mtk, BIT(1));
		tasklet_hi_schedule(&mtk->done);
		return IRQ_HANDLED;
	}

/* TODO: error handler; for now just clear ALL */
	dev_err(mtk->dev, "IRQ: %08x\n", irq_status);
	mtk_irq_clear(mtk, irq_status);
	if (irq_status) {
		printk("disable irq\n");
		mtk_irq_disable(mtk, irq_status);
	}
	return IRQ_NONE;
}

static void mtk_done_tasklet(unsigned long data)
{
	struct mtk_device *mtk = (struct mtk_device *)data;

	mtk_handle_result_descriptor(mtk);
}

void mtk_initialize(struct mtk_device *mtk)
{
	uint8_t fRstPacketEngine = 1;
	uint8_t fResetRing = 1;
	uint8_t PE_Mode = 3;
	uint8_t fBO_PD_en = 0;
	uint8_t fBO_SA_en = 0;
	uint8_t fBO_Data_en = 0;
	uint8_t fBO_TD_en = 0;
	uint8_t fEnablePDRUpdate = 1;
	int InputThreshold = 128;
	int OutputThreshold = 128;
	int DescriptorCountDone = MTK_RING_SIZE - 1;
	int DescriptorPendingCount = 1;
	int DescriptorDoneTimeout = 3;
	u32 regVal;

	writel((fRstPacketEngine & 1) |
		((fResetRing & 1) << 1) |
		((PE_Mode & GENMASK(2, 0)) << 8) |
		((fBO_PD_en & 1) << 16) |
		((fBO_SA_en & 1) << 17) |
		((fBO_Data_en  & 1) << 18) |
		((fBO_TD_en & 1) << 20) |
		((fEnablePDRUpdate & 1) << 10),
		mtk->base + EIP93_REG_PE_CONFIG);

	udelay(10);

	fRstPacketEngine = 0;
	fResetRing = 0;

	writel((fRstPacketEngine & 1) |
		((fResetRing & 1) << 1) |
		((PE_Mode & GENMASK(2, 0)) << 8) |
		((fBO_PD_en & 1) << 16) |
		((fBO_SA_en & 1) << 17) |
		((fBO_Data_en  & 1) << 18) |
		((fBO_TD_en & 1) << 20) |
		((fEnablePDRUpdate & 1) << 10),
		mtk->base + EIP93_REG_PE_CONFIG);

	/* Initialize the BYTE_ORDER_CFG register */
	writel((EIP93_BYTE_ORDER_PD & GENMASK(4, 0)) |
		((EIP93_BYTE_ORDER_SA & GENMASK(4, 0)) << 4) |
		((EIP93_BYTE_ORDER_DATA & GENMASK(4, 0)) << 8) |
		((EIP93_BYTE_ORDER_TD & GENMASK(2, 0)) << 16),
		mtk->base + EIP93_REG_PE_ENDIAN_CONFIG);
	/* Initialize the INT_CFG register */
	writel((EIP93_INT_HOST_OUTPUT_TYPE & 1) |
		((EIP93_INT_PULSE_CLEAR << 1) & 1),
		mtk->base + EIP93_REG_INT_CFG);
	/* Clock Control, must for DHM, optional for ARM
	 * 0x1 Only enable Packet Engine Clock
	 *     AES, DES and HASH clocks on demand
	 * Activating all clocks per performance
	 */
	regVal = BIT(0) | BIT(1) | BIT(2) | BIT(4);
	writel(regVal, mtk->base + EIP93_REG_PE_CLOCK_CTRL);

	writel(BIT(31) | (InputThreshold & GENMASK(10, 0)) |
		((OutputThreshold & GENMASK(10, 0)) << 16),
		mtk->base + EIP93_REG_PE_BUF_THRESH);

	/* Clear/ack all interrupts before disable all */
	mtk_irq_clear(mtk, 0xFFFFFFFF);
	mtk_irq_disable(mtk, 0xFFFFFFFF);

	writel((DescriptorCountDone & GENMASK(10, 0)) |
		(((DescriptorPendingCount - 1) & GENMASK(10, 0)) << 16) |
		((DescriptorDoneTimeout  & GENMASK(4, 0)) << 26),
		mtk->base + EIP93_REG_PE_RING_THRESH);

	regVal = readl(mtk->base + EIP93_REG_PE_REVISION);
	dev_dbg(mtk->dev, "Rev: %08x", regVal);
	regVal = readl(mtk->base + EIP93_REG_PE_OPTION_1);
	dev_dbg(mtk->dev, "Opt1: %08x", regVal);
	regVal = readl(mtk->base + EIP93_REG_PE_OPTION_0);
	dev_dbg(mtk->dev, "Opt0: %08x", regVal);

}

static void mtk_desc_free(struct mtk_device *mtk,
				struct mtk_desc_ring *cdr,
				struct mtk_desc_ring *rdr)
{
	writel(0, mtk->base + EIP93_REG_PE_RING_CONFIG);
	writel(0, mtk->base + EIP93_REG_PE_CDR_BASE);
	writel(0, mtk->base + EIP93_REG_PE_RDR_BASE);
}

static int mtk_desc_init(struct mtk_device *mtk,
			struct mtk_desc_ring *cdr,
			struct mtk_desc_ring *rdr)
{
	int RingOffset, RingSize;

	cdr->offset = sizeof(struct eip93_descriptor_s);
	cdr->base = dmam_alloc_coherent(mtk->dev, cdr->offset * MTK_RING_SIZE,
					&cdr->base_dma, GFP_KERNEL);
	if (!cdr->base)
		return -ENOMEM;

	cdr->write = cdr->base;
	cdr->base_end = cdr->base + cdr->offset * (MTK_RING_SIZE - 1);
	cdr->read  = cdr->base;

	dev_dbg(mtk->dev, "CD Ring : %08X\n", cdr->base_dma);

	rdr->offset = sizeof(struct eip93_descriptor_s);
	rdr->base = dmam_alloc_coherent(mtk->dev, rdr->offset * MTK_RING_SIZE,
					&rdr->base_dma, GFP_KERNEL);
	if (!rdr->base)
		return -ENOMEM;

	rdr->write = rdr->base;
	rdr->base_end = rdr->base + rdr->offset * (MTK_RING_SIZE - 1);
	rdr->read  = rdr->base;

	dev_dbg(mtk->dev, "RD Ring : %08X\n", rdr->base_dma);

	writel((u32)cdr->base_dma, mtk->base + EIP93_REG_PE_CDR_BASE);
	writel((u32)rdr->base_dma, mtk->base + EIP93_REG_PE_RDR_BASE);

	RingOffset = 8; /* 8 words per descriptor */
	RingSize = MTK_RING_SIZE - 1;

	writel(((RingOffset & GENMASK(8, 0)) << 16) |
		(RingSize & GENMASK(10, 0)),
		mtk->base + EIP93_REG_PE_RING_CONFIG);

	/* Create Sa and State record DMA pool */

	mtk->saRecord_pool = dmam_pool_create("eip93-saRecord",
				mtk->dev, sizeof(struct saRecord_s), 32, 0);

	if (!mtk->saRecord_pool) {
		dev_err(mtk->dev, "Unable to allocate saRecord DMA pool\n");
		return -ENOMEM;
	}

	mtk->saState_pool = dmam_pool_create("eip93-saState",
				mtk->dev, sizeof(struct saState_s), 32, 0);

	if (!mtk->saState_pool) {
		dev_err(mtk->dev, "Unable to allocate saState DMA pool\n");
		return -ENOMEM;
	}

	return 0;
}

static int mtk_crypto_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct mtk_device *mtk;
	struct resource *res;
	int ret;

	mtk = devm_kzalloc(dev, sizeof(*mtk), GFP_KERNEL);
	if (!mtk)
		return -ENOMEM;

	mtk->dev = dev;
	platform_set_drvdata(pdev, mtk);

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	mtk->base = devm_ioremap_resource(&pdev->dev, res);

	if (IS_ERR(mtk->base))
		return PTR_ERR(mtk->base);

	mtk->irq = platform_get_irq(pdev, 0);

	if (mtk->irq < 0) {
		dev_err(mtk->dev, "Cannot get IRQ resource\n");
		return mtk->irq;
	}
	dev_dbg(mtk->dev, "Assigning IRQ: %d", mtk->irq);

	ret = devm_request_irq(mtk->dev, mtk->irq, mtk_irq_handler,
				IRQF_TRIGGER_HIGH, dev_name(mtk->dev), mtk);

	mtk->ring = devm_kcalloc(mtk->dev, 1, sizeof(*mtk->ring), GFP_KERNEL);

	if (!mtk->ring) {
		dev_err(mtk->dev, "Can't allocate Ring memory\n");
	}

	ret = mtk_desc_init(mtk, &mtk->ring->cdr, &mtk->ring->rdr);

	if (ret == -ENOMEM)
		return -ENOMEM;

	mtk->prng = devm_kcalloc(mtk->dev, 1, sizeof(*mtk->prng), GFP_KERNEL);
	if (!mtk->prng) {
		dev_err(mtk->dev, "Can't allocate PRNG memory\n");
		return -ENOMEM;
	}

	mtk->ring->requests = 0;
	mtk->ring->busy = false;

	spin_lock_init(&mtk->ring->lock);
	spin_lock_init(&mtk->ring->read_lock);
	spin_lock_init(&mtk->ring->write_lock);

	/* Init tasklet for bottom half processing */
	tasklet_init(&mtk->done, mtk_done_tasklet, (unsigned long)mtk);

	mtk_initialize(mtk);
	/* Init. finished, enable RDR interupt */
	mtk_irq_enable(mtk, BIT(1));

	ret = mtk_prng_init(mtk, true);
	if (ret)
		dev_info(mtk->dev, "PRNG initialized");
	else
		dev_err(mtk->dev, "Could not initialize PRNG");

	ret = mtk_register_algs(mtk);

	dev_info(mtk->dev, "EIP93 initialized succesfull\n");

	return 0;
}

static int mtk_crypto_remove(struct platform_device *pdev)
{
	struct mtk_device *mtk = platform_get_drvdata(pdev);

	mtk_unregister_algs(mtk, ARRAY_SIZE(mtk_algs));

	/* Clear/ack all interrupts before disable all */
	mtk_irq_clear(mtk, 0xFFFFFFFF);
	mtk_irq_disable(mtk, 0xFFFFFFFF);

	writel(0, mtk->base + EIP93_REG_PE_CLOCK_CTRL);

	tasklet_kill(&mtk->done);

	mtk_desc_free(mtk, &mtk->ring->cdr, &mtk->ring->rdr);
	dev_info(mtk->dev, "EIP93 removed.\n");

	return 0;
}

static const struct of_device_id mtk_crypto_of_match[] = {
	{ .compatible = "mediatek,mtk-eip93", },
	{}
};
MODULE_DEVICE_TABLE(of, mtk_crypto_of_match);

static struct platform_driver mtk_crypto_driver = {
	.probe = mtk_crypto_probe,
	.remove = mtk_crypto_remove,
	.driver = {
		.name = "mtk-eip93",
		.of_match_table = mtk_crypto_of_match,
	},
};
module_platform_driver(mtk_crypto_driver);

MODULE_AUTHOR("Richard van Schagen <vschagen@cs.com>");
MODULE_ALIAS("platform:" KBUILD_MODNAME);
MODULE_DESCRIPTION("Mediatek EIP-93 crypto engine driver");
MODULE_LICENSE("GPL v2");

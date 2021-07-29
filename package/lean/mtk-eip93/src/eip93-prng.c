/* SPDX-License-Identifier: GPL-2.0
 *
 * Copyright (C) 2019 - 2020
 *
 * Richard van Schagen <vschagen@cs.com>
 */

#include "eip93-common.h"
#include "eip93-core.h"
#include "eip93-regs.h"
#include "eip93-ring.h"
#include "eip93-prng.h"

static int mtk_prng_push_job(struct mtk_device *mtk, bool reset)
{
	struct mtk_prng_device *prng = mtk->prng;
	struct eip93_descriptor_s cdesc;
	int cur = prng->cur_buf;
	int len, mode, err;

	if (reset) {
		len = 0;
		mode = 1;
	} else {
		len = 4080;
		mode = 2;
	}

	init_completion(&prng->Filled);
	atomic_set(&prng->State, BUF_EMPTY);

	memset(&cdesc, 0, sizeof(struct eip93_descriptor_s));
	cdesc.peCrtlStat.bits.hostReady = 1;
	cdesc.peCrtlStat.bits.prngMode = mode;
	cdesc.peCrtlStat.bits.hashFinal = 0;
	cdesc.peCrtlStat.bits.padCrtlStat = 0;
	cdesc.peCrtlStat.bits.peReady = 0;
	cdesc.srcAddr = 0;
	cdesc.dstAddr = (u32)prng->PRNGBuffer_dma[cur];
	cdesc.saAddr = (u32)prng->PRNGSaRecord_dma;
	cdesc.stateAddr = 0;
	cdesc.arc4Addr = 0;
	cdesc.userId = MTK_DESC_PRNG | MTK_DESC_LAST | MTK_DESC_FINISH;
	cdesc.peLength.bits.byPass = 0;
	cdesc.peLength.bits.length = 4080;
	cdesc.peLength.bits.hostReady = 1;

	err = mtk_put_descriptor(mtk, cdesc);
	if (err)
		dev_err(mtk->dev, "PRNG: No Descriptor space");

	/*   */
	spin_lock(&mtk->ring->lock);
	mtk->ring[0].requests += 1;
	mtk->ring[0].busy = true;
	spin_unlock(&mtk->ring->lock);

	writel(1, mtk->base + EIP93_REG_PE_CD_COUNT);

	wait_for_completion(&prng->Filled);

	if (atomic_read(&prng->State) == PRNG_NEED_RESET)
		return false;

	return true;
}

/*----------------------------------------------------------------------------
 * mtk_prng_init
 *
 * This function initializes the PE PRNG for the ARM mode.
 *
 * Return Value
 *      true: PRNG is initialized
 *     false: PRNG initialization failed
 */
bool mtk_prng_init(struct mtk_device *mtk, bool fLongSA)
{
	struct mtk_prng_device *prng = mtk->prng;
	int i;
	struct saRecord_s *saRecord;
	const uint32_t PRNGKey[]  = {0xe0fc631d, 0xcbb9fb9a,
					0x869285cb, 0xcbb9fb9a};
	const uint32_t PRNGSeed[]  = {0x758bac03, 0xf20ab39e,
					0xa569f104, 0x95dfaea6};
	const uint32_t PRNGDateTime[] = {0, 0, 0, 0};

	if (!mtk)
		return -ENODEV;

	prng->cur_buf = 0;
	prng->PRNGBuffer[0] = devm_kzalloc(mtk->dev, 4080, GFP_KERNEL);
	prng->PRNGBuffer_dma[0] = (u32)dma_map_single(mtk->dev,
				(void *)prng->PRNGBuffer[0],
				4080, DMA_FROM_DEVICE);

	prng->PRNGBuffer[1] = devm_kzalloc(mtk->dev, 4080, GFP_KERNEL);
	prng->PRNGBuffer_dma[1] = (u32)dma_map_single(mtk->dev,
				(void *)prng->PRNGBuffer[1],
				4080, DMA_FROM_DEVICE);

	prng->PRNGSaRecord = dmam_alloc_coherent(mtk->dev,
				sizeof(struct saRecord_s),
				&prng->PRNGSaRecord_dma, GFP_KERNEL);

	if (!prng->PRNGSaRecord) {
		dev_err(mtk->dev, "PRNG dma_alloc for saRecord failed\n");
		return -ENOMEM;
	}

	saRecord = &prng->PRNGSaRecord[0];

	saRecord->saCmd0.word = 0x00001307;
	saRecord->saCmd1.word = 0x02000000;

	for (i = 0; i < 4; i++) {
		saRecord->saKey[i] = PRNGKey[i];
		saRecord->saIDigest[i] = PRNGSeed[i];
		saRecord->saODigest[i] = PRNGDateTime[i];
	}

	return mtk_prng_push_job(mtk, true);
}

void mtk_prng_done(struct mtk_device *mtk, u32 err)
{
	struct mtk_prng_device *prng = mtk->prng;
	int cur = prng->cur_buf;

	if (err) {
		dev_err(mtk->dev, "PRNG error: %d\n", err);
		atomic_set(&prng->State, PRNG_NEED_RESET);
	}

	/* Buffer refilled, invalidate cache */
	dma_unmap_single(mtk->dev, prng->PRNGBuffer_dma[cur],
							4080, DMA_FROM_DEVICE);

	complete(&prng->Filled);
}

static int get_prng_bytes(char *buf, size_t nbytes, struct mtk_prng_ctx *ctx,
				int do_cont_test)
{
	int err;

	spin_lock_bh(&ctx->prng_lock);

	err = -EINVAL;
	if (ctx->flags & PRNG_NEED_RESET)
		goto done;

done:
	spin_unlock_bh(&ctx->prng_lock);
	return err;
}

static int mtk_prng_generate(struct crypto_rng *tfm, const u8 *src,
			   unsigned int slen, u8 *dst, unsigned int dlen)
{
	struct mtk_prng_ctx *prng = crypto_rng_ctx(tfm);

	return get_prng_bytes(dst, dlen, prng, 1);
}

static int mtk_prng_seed(struct crypto_rng *tfm, const u8 *seed,
		       unsigned int slen)
{
	struct rng_alg *alg = crypto_rng_alg(tfm);
	struct mtk_alg_template *tmpl = container_of(alg,
				struct mtk_alg_template, alg.rng);
	struct mtk_device *mtk = tmpl->mtk;

	return 0;
}

static bool mtk_prng_fill_buffer(struct mtk_device *mtk)
{
	struct mtk_prng_device *prng = mtk->prng;
	int cur = prng->cur_buf;
	int ret;

	if (!mtk)
		return -ENODEV;

	/* add logic for 2 buffers and swap */
	prng->PRNGBuffer_dma[cur] = (u32)dma_map_single(mtk->dev,
					(void *)prng->PRNGBuffer[cur],
					4080, DMA_FROM_DEVICE);

	ret = mtk_prng_push_job(mtk, false);

	return ret;
}

static int reset_prng_context(struct mtk_prng_ctx *ctx,
				const unsigned char *key,
				const unsigned char *V,
				const unsigned char *DT)
{
	spin_lock_bh(&ctx->prng_lock);
	ctx->flags |= PRNG_NEED_RESET;

	if (key)
		memcpy(ctx->PRNGKey, key, DEFAULT_PRNG_KSZ);
	else
		memcpy(ctx->PRNGKey, DEFAULT_PRNG_KEY, DEFAULT_PRNG_KSZ);


	if (V)
		memcpy(ctx->PRNGSeed, V, DEFAULT_BLK_SZ);
	else
		memcpy(ctx->PRNGSeed, DEFAULT_V_SEED, DEFAULT_BLK_SZ);

	if (DT)
		memcpy(ctx->PRNGDateTime, DT, DEFAULT_BLK_SZ);
	else
		memset(ctx->PRNGDateTime, 0, DEFAULT_BLK_SZ);

	memset(ctx->rand_data, 0, DEFAULT_BLK_SZ);
	memset(ctx->last_rand_data, 0, DEFAULT_BLK_SZ);

	ctx->rand_data_valid = DEFAULT_BLK_SZ;

	ctx->flags &= ~PRNG_NEED_RESET;
	spin_unlock_bh(&ctx->prng_lock);
	return 0;
}

/*
 *  This is the cprng_registered reset method the seed value is
 *  interpreted as the tuple { V KEY DT}
 *  V and KEY are required during reset, and DT is optional, detected
 *  as being present by testing the length of the seed
 */
static int cprng_reset(struct crypto_rng *tfm,
		       const u8 *seed, unsigned int slen)
{
	struct mtk_prng_ctx *prng = crypto_rng_ctx(tfm);
	const u8 *key = seed + DEFAULT_BLK_SZ;
	const u8 *dt = NULL;

	if (slen < DEFAULT_PRNG_KSZ + DEFAULT_BLK_SZ)
		return -EINVAL;

	if (slen >= (2 * DEFAULT_BLK_SZ + DEFAULT_PRNG_KSZ))
		dt = key + DEFAULT_PRNG_KSZ;

	reset_prng_context(prng, key, seed, dt);

	if (prng->flags & PRNG_NEED_RESET)
		return -EINVAL;
	return 0;
}


static void free_prng_context(struct mtk_prng_ctx *ctx)
{
	crypto_free_cipher(ctx->tfm);
}

static int cprng_init(struct crypto_tfm *tfm)
{
	struct mtk_prng_ctx *ctx = crypto_tfm_ctx(tfm);

	spin_lock_init(&ctx->prng_lock);

	if (reset_prng_context(ctx, NULL, NULL, NULL) < 0)
		return -EINVAL;

	/*
	 * after allocation, we should always force the user to reset
	 * so they don't inadvertently use the insecure default values
	 * without specifying them intentially
	 */
	ctx->flags |= PRNG_NEED_RESET;
	return 0;
}

static void cprng_exit(struct crypto_tfm *tfm)
{
	free_prng_context(crypto_tfm_ctx(tfm));
}

struct mtk_alg_template mtk_alg_prng = {
	.type = MTK_ALG_TYPE_PRNG,
	.flags = 0,
	.alg.rng = {
		.generate = mtk_prng_generate,
		.seed = mtk_prng_seed,
		.seedsize = 0,
		.base = {
			.cra_name = "stdrng",
			.cra_driver_name = "eip93-prng",
			.cra_priority = 200,
			.cra_ctxsize = sizeof(struct mtk_prng_ctx),
			.cra_module = THIS_MODULE,
			.cra_init = cprng_init,
			.cra_exit = cprng_exit,
		},
	},
};

//#ifdef CONFIG_CRYPTO_FIPS
static int fips_cprng_get_random(struct crypto_rng *tfm,
				 const u8 *src, unsigned int slen,
				 u8 *rdata, unsigned int dlen)
{
	struct mtk_prng_ctx *prng = crypto_rng_ctx(tfm);

	return get_prng_bytes(rdata, dlen, prng, 1);
}

static int fips_cprng_reset(struct crypto_rng *tfm,
			    const u8 *seed, unsigned int slen)
{
	struct mtk_prng_ctx *prng = crypto_rng_ctx(tfm);
	u8 rdata[DEFAULT_BLK_SZ];
	const u8 *key = seed + DEFAULT_BLK_SZ;
	int rc;

	if (slen < DEFAULT_PRNG_KSZ + DEFAULT_BLK_SZ)
		return -EINVAL;

	/* fips strictly requires seed != key */
	if (!memcmp(seed, key, DEFAULT_PRNG_KSZ))
		return -EINVAL;

	rc = cprng_reset(tfm, seed, slen);

	if (!rc)
		goto out;

	/* this primes our continuity test */
	rc = get_prng_bytes(rdata, DEFAULT_BLK_SZ, prng, 0);
	prng->rand_data_valid = DEFAULT_BLK_SZ;

out:
	return rc;
}

struct mtk_alg_template mtk_alg_cprng = {
	.type = MTK_ALG_TYPE_PRNG,
	.flags = 0,
	.alg.rng = {
		.generate = fips_cprng_get_random,
		.seed = fips_cprng_reset,
		.seedsize = DEFAULT_PRNG_KSZ + 2 * DEFAULT_BLK_SZ,
		.base = {
			.cra_name = "fips(ansi_cprng)",
			.cra_driver_name = "eip93-fips_ansi_cprng",
			.cra_priority = 300,
			.cra_ctxsize = sizeof(struct mtk_prng_ctx),
			.cra_module = THIS_MODULE,
			.cra_init = cprng_init,
			.cra_exit = cprng_exit,
		},
	},
};
//#endif

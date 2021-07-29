/* SPDX-License-Identifier: GPL-2.0
 *
 * Copyright (C) 2019 - 2020
 *
 * Richard van Schagen <vschagen@cs.com>
 */
#ifndef _CORE_H_
#define _CORE_H_

#include <linux/atomic.h>
#include <linux/completion.h>
#include <linux/dmapool.h>
#include <crypto/aead.h>
#include <crypto/internal/hash.h>
#include <crypto/internal/rng.h>
#include <crypto/internal/skcipher.h>

/**
 * struct mtk_device - crypto engine device structure
 */
struct mtk_device {
	void __iomem		*base;
	struct device		*dev;
	struct clk		*clk;
	int			irq;

	struct tasklet_struct	dequeue;
	struct tasklet_struct	done;

	struct mtk_ring		*ring;

	struct dma_pool		*saRecord_pool;
	struct dma_pool		*saState_pool;

	struct mtk_prng_device	*prng;
};

struct mtk_prng_device {
	struct saRecord_s	*PRNGSaRecord;
	dma_addr_t		PRNGSaRecord_dma;
	void			*PRNGBuffer[2];
	dma_addr_t		PRNGBuffer_dma[2];
	uint32_t		cur_buf;
	struct completion	Filled;
	atomic_t		State;
};

struct mtk_desc_ring {
	void			*base;
	void			*base_end;
	dma_addr_t		base_dma;
	/* write and read pointers */
	void			*read;
	void			*write;
	/* descriptor element offset */
	u32			offset;
};

struct mtk_ring {
	spinlock_t			lock;
	/* command/result rings */
	struct mtk_desc_ring		cdr;
	struct mtk_desc_ring		rdr;
	spinlock_t			write_lock;
	spinlock_t			read_lock;
	/* Number of request in the engine. */
	int				requests;
	/* The rings is handling at least one request */
	bool				busy;
};

enum mtk_alg_type {
	MTK_ALG_TYPE_SKCIPHER,
	MTK_ALG_TYPE_AEAD,
	MTK_ALG_TYPE_AHASH,
	MTK_ALG_TYPE_PRNG,
};

struct mtk_alg_template {
	struct mtk_device	*mtk;
	enum mtk_alg_type	type;
	unsigned long		flags;
	union {
		struct skcipher_alg	skcipher;
		struct aead_alg		aead;
		struct ahash_alg	ahash;
		struct rng_alg		rng;
	} alg;
};

#endif /* _CORE_H_ */

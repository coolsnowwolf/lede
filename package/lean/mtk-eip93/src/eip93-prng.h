/* SPDX-License-Identifier: GPL-2.0
 *
 * Copyright (C) 2019 - 2020
 *
 * Richard van Schagen <vschagen@cs.com>
 */
#define DEFAULT_PRNG_KEY "0123456789abcdef"
#define DEFAULT_PRNG_KSZ 16
#define DEFAULT_BLK_SZ 16
#define DEFAULT_V_SEED "zaybxcwdveuftgsh"

#define BUF_NOT_EMPTY 0
#define BUF_EMPTY 1
#define BUF_PENDING 2
#define PRNG_NEED_RESET 3

extern struct mtk_alg_template mtk_alg_prng;
extern struct mtk_alg_template mtk_alg_cprng;

bool mtk_prng_init(struct mtk_device *mtk, bool fLongSA);

void mtk_prng_done(struct mtk_device *mtk, u32 err);

struct mtk_prng_ctx {
	spinlock_t 		prng_lock;
	unsigned char		rand_data[DEFAULT_BLK_SZ];
	unsigned char		last_rand_data[DEFAULT_BLK_SZ];
	uint32_t		PRNGKey[4];
	uint32_t		PRNGSeed[4];
	uint32_t		PRNGDateTime[4];
	struct crypto_cipher	*tfm;
	uint32_t		rand_data_valid;
	uint32_t		flags;
};

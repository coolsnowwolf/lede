
/*dh_key.h START*/
#define GETPID_IS_MEANINGLESS
#include "rt_config.h"

#ifdef WSC_INCLUDED

#ifndef DHPRINT
#define DHPRINT(fmt, args...) MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_OFF, (fmt, ## args))
#endif

#define BN_MUL_COMBA
#define BN_SQR_COMBA

#define BN_ULLONG	unsigned long
#define BN_ULONG	unsigned int
#define BN_BITS		64
#define BN_BYTES	4
#define BN_BITS2	32
#define BN_BITS4	16
#define BN_MASK		(0xffffffffffffffffLL)
#define BN_MASK2	(0xffffffffL)
#define BN_MASK2l	(0xffff)
#define BN_MASK2h1	(0xffff8000L)
#define BN_MASK2h	(0xffff0000L)
#define BN_TBIT		(0x80000000L)
#define BN_FLG_MALLOCED			0x01
#define BN_FLG_STATIC_DATA		0x02
#define BN_FLG_EXP_CONSTTIME	0x04 /* avoid leaking exponent information through timings
				      * (BN_mod_exp_mont() will call BN_mod_exp_mont_consttime) */
#define BN_set_flags(b, n)	((b)->flags |= (n))
#define BN_get_flags(b, n)	((b)->flags&(n))
/* get a clone of a struct bignum_st with changed flags, for *temporary* use only
 * (the two BIGNUMs cannot not be used in parallel!) */
#define BN_with_flags(dest, b, n)  ((dest)->d = (b)->d, \
									(dest)->top = (b)->top, \
									(dest)->dmax = (b)->dmax, \
									(dest)->flags = (((dest)->flags & BN_FLG_MALLOCED) \
											|  ((b)->flags & ~BN_FLG_MALLOCED) \
											|  BN_FLG_STATIC_DATA \
											|  (n)))

struct bignum_st {
	BN_ULONG *d;	/* Pointer to an array of 'BN_BITS2' bit chunks. */
	int top;	/* Index of last used d +1. */
	/* The next are internal book keeping for bn_expand. */
	int dmax;	/* Size of the d array. */
	int flags;
};

/* Used for montgomery multiplication */
struct bn_mont_ctx_st {
	int ri;        /* number of bits in R */
	struct bignum_st RR;     /* used to convert to montgomery form */
	struct bignum_st N;      /* The modulus */
	BN_ULONG n0;   /* least significant word of Ni */
	int flags;
};

#define BN_num_bytes(a)	((BN_num_bits(a)+7)/8)
#define BN_is_zero(a)       ((a)->top == 0)
#define BN_is_one(a)        (((a)->top == 1) && ((a)->d[0] == (BN_ULONG)(1)))
#define BN_is_odd(a)	    (((a)->top > 0) && ((a)->d[0] & 1))
#define BN_one(a)	(BN_set_word((a), 1))
#define BN_zero(a)	(BN_set_word((a), 0))
#define bn_wexpand(a, words) (((words) <= (a)->dmax)?(a):bn_expand2((a), (words)))
#define bn_correct_top(a) \
	{ \
		BN_ULONG *ftl; \
		if ((a)->top > 0) { \
			for (ftl = &((a)->d[(a)->top-1]); (a)->top > 0; (a)->top--) \
				if (*(ftl--))
break;
\
} \
}

/*
 * BN_window_bits_for_exponent_size -- macro for sliding window mod_exp functions
 *
 *
 * For window size 'w' (w >= 2) and a random 'b' bits exponent,
 * the number of multiplications is a constant plus on average
 *
 *    2^(w-1) + (b-w)/(w+1);
 *
 * here  2^(w-1)  is for precomputing the table (we actually need
 * entries only for windows that have the lowest bit set), and
 * (b-w)/(w+1)  is an approximation for the expected number of
 * w-bit windows, not counting the first one.
 *
 * Thus we should use
 *
 *    w >= 6  if        b > 671
 *     w = 5  if  671 > b > 239
 *     w = 4  if  239 > b >  79
 *     w = 3  if   79 > b >  23
 *    w <= 2  if   23 > b
 *
 * (with draws in between).  Very small exponents are often selected
 * with low Hamming weight, so we use  w = 1  for b <= 23.
 */
#define BN_window_bits_for_exponent_size(b) \
	((b) > 671 ? 6 : \
	 (b) > 239 ? 5 : \
	 (b) >  79 ? 4 : \
	 (b) >  23 ? 3 : 1)
/* BN_mod_exp_mont_conttime is based on the assumption that the
 * L1 data cache line width of the target processor is at least
 * the following value.
 */
#define MOD_EXP_CTIME_MIN_CACHE_LINE_WIDTH	(64)
#define MOD_EXP_CTIME_MIN_CACHE_LINE_MASK	(MOD_EXP_CTIME_MIN_CACHE_LINE_WIDTH - 1)
/* Window sizes optimized for fixed window size modular exponentiation
 * algorithm (BN_mod_exp_mont_consttime).
 *
 * To achieve the security goals of BN_mode_exp_mont_consttime, the
 * maximum size of the window must not exceed
 * log_2(MOD_EXP_CTIME_MIN_CACHE_LINE_WIDTH).
 *
 * Window size thresholds are defined for cache line sizes of 32 and 64,
 * cache line sizes where log_2(32)=5 and log_2(64)=6 respectively. A
 * window size of 7 should only be used on processors that have a 128
 * byte or greater cache line size.
 */
#if MOD_EXP_CTIME_MIN_CACHE_LINE_WIDTH == 64
#  define BN_window_bits_for_ctime_exponent_size(b) \
	((b) > 937 ? 6 : \
	 (b) > 306 ? 5 : \
	 (b) >  89 ? 4 : \
	 (b) >  22 ? 3 : 1)
#  define BN_MAX_WINDOW_BITS_FOR_CTIME_EXPONENT_SIZE	(6)
#elif MOD_EXP_CTIME_MIN_CACHE_LINE_WIDTH == 32
#  define BN_window_bits_for_ctime_exponent_size(b) \
	((b) > 306 ? 5 : \
	 (b) >  89 ? 4 : \
	 (b) >  22 ? 3 : 1)
#  define BN_MAX_WINDOW_BITS_FOR_CTIME_EXPONENT_SIZE	(5)
#endif
#define BN_MULL_SIZE_NORMAL			(16) /* 32 */
#define BN_MUL_RECURSIVE_SIZE_NORMAL		(16) /* 32 less than */
#define BN_SQR_RECURSIVE_SIZE_NORMAL		(16) /* 32 */

/*************************************************************
 * Using the long long type
 */
#define Lw(t)    (((BN_ULONG)(t))&BN_MASK2)
#define Hw(t)    (((BN_ULONG)((t)>>BN_BITS2))&BN_MASK2)

/*************************************************************
 * No long long type
 */

#define LBITS(a)	((a)&BN_MASK2l)
#define HBITS(a)	(((a)>>BN_BITS4)&BN_MASK2l)
#define	L2HBITS(a)	(((a)<<BN_BITS4)&BN_MASK2)

#define LLBITS(a)	((a)&BN_MASKl)
#define LHBITS(a)	(((a)>>BN_BITS2)&BN_MASKl)
#define	LL2HBITS(a)	((BN_ULLONG)((a)&BN_MASKl)<<BN_BITS2)

#define mul64(l, h, bl, bh) \
	{ \
		BN_ULONG m, m1, lt, ht; \
		\
		lt = l; \
		ht = h; \
		m = (bh)*(lt); \
		lt = (bl)*(lt); \
		m1 = (bl)*(ht); \
		ht = (bh)*(ht); \
		m = (m+m1)&BN_MASK2; \
		if (m < m1)	\
			ht += L2HBITS((BN_ULONG)1); \
		ht += HBITS(m); \
		m1 = L2HBITS(m); \
		lt = (lt+m1)&BN_MASK2; \
		if (lt < m1)	\
			ht++; \
		(l) = lt; \
		(h) = ht; \
	}

#define sqr64(lo, ho, in) \
	{ \
		BN_ULONG l, h, m; \
		\
		h = (in); \
		l = LBITS(h); \
		h = HBITS(h); \
		m = (l)*(h); \
		l *= l; \
		h *= h; \
		h += (m&BN_MASK2h1)>>(BN_BITS4-1); \
		m = (m&BN_MASK2l)<<(BN_BITS4+1); \
		l = (l+m)&BN_MASK2; \
		if (l < m)	\
			h++; \
		(lo) = l; \
		(ho) = h; \
	}

#define mul_add(r, a, bl, bh, c) { \
		BN_ULONG l, h; \
		\
		h = (a); \
		l = LBITS(h); \
		h = HBITS(h); \
		mul64(l, h, (bl), (bh)); \
		\
		/* non-multiply part */ \
		l = (l+(c))&BN_MASK2;	\
		if (l < (c))		\
			h++; \
		(c) = (r); \
		l = (l+(c))&BN_MASK2; \
		if (l < (c))		\
			h++; \
		(c) = h&BN_MASK2; \
		(r) = l; \
	}

#define mul(r, a, bl, bh, c) { \
		BN_ULONG l, h; \
		\
		h = (a); \
		l = LBITS(h); \
		h = HBITS(h); \
		mul64(l, h, (bl), (bh)); \
		\
		/* non-multiply part */ \
		l += (c);		\
		if ((l&BN_MASK2) < (c))	\
			h++; \
		(c) = h&BN_MASK2; \
		(r) = l&BN_MASK2; \
	}


#define INIT_DATA_h0 0x67452301UL
#define INIT_DATA_h1 0xefcdab89UL
#define INIT_DATA_h2 0x98badcfeUL
#define INIT_DATA_h3 0x10325476UL
#define INIT_DATA_h4 0xc3d2e1f0UL
#define STATE_SIZE	1023
#define EXP_TABLE_SIZE			32
#define BN_mod(rem, m, d, ctx) BN_div(NULL, (rem), (m), (d), (ctx))
#define BN_CTX_POOL_SIZE	16
#define MOD_EXP_CTIME_ALIGN(x_) ((unsigned char *)(x_) + (MOD_EXP_CTIME_MIN_CACHE_LINE_WIDTH - (((BN_ULONG)(x_)) & (MOD_EXP_CTIME_MIN_CACHE_LINE_MASK))))
#define ENTROPY_NEEDED 32  /* require 256 bits = 32 bytes of randomness */
#define DH_FLAG_CACHE_MONT_P     0x01
#define DH_FLAG_NO_EXP_CONSTTIME 0x02 /* new with 0.9.7h; the built-in struct dh_st
				       * implementation now uses constant time
				       * modular exponentiation for secret exponents
				       * by default. This flag causes the
				       * faster variable sliding window method to
				       * be used for all exponents.
				       */
#define OPENSSL_DH_MAX_MODULUS_BITS	10000

struct dh_st {
	struct bignum_st *p;
	struct bignum_st *g;
	struct bignum_st *pub_key;	/* g^x */
	struct bignum_st *priv_key;	/* x */
	int flags;
};

struct env_md_st {
	int type;
	int md_size;
	unsigned long flags;
	int (*init)(struct env_md_ctx_st *ctx);
	int (*update)(struct env_md_ctx_st *ctx, const void *data, size_t count);
	int (*final)(struct env_md_ctx_st *ctx, unsigned char *md);
	int ctx_size; /* how big does the ctx->md_data need to be */
} /* struct env_md_st */;

struct env_md_ctx_st {
	const struct env_md_st *digest;
	void *md_data;
} /* struct env_md_ctx_st */;

/* A bundle of bignums that can be linked with other bundles */
struct bignum_pool_item {
	/* The bignum values */
	struct bignum_st vals[BN_CTX_POOL_SIZE];
	/* Linked-list admin */
	struct bignum_pool_item *prev, *next;
};

/* A linked-list of bignums grouped in bundles */
struct bignum_pool {
	/* Linked-list admin */
	struct bignum_pool_item *head, *currentvar, *tail;
	/* Stack depth and allocation size */
	unsigned used, size;
};
/*dh_key.h END*/

/*sha_locl.h START*/
#define SHA_LONG unsigned int
#define SHA_LBLOCK	16
#define SHA_CBLOCK	(SHA_LBLOCK*4)	/* SHA treats input data as a contiguous array of 32 bit wide big-endian values. */
#define SHA_DIGEST_LENGTH 20
struct SHAstate_st {
	SHA_LONG h0, h1, h2, h3, h4;
	SHA_LONG Nl, Nh;
	SHA_LONG data[SHA_LBLOCK];
	unsigned int num;
};
#define Xupdate(a, ix, ia, ib, ic, id)	((a) = (ia^ib^ic^id), ix = (a) = ROTATE((a), 1))

#ifndef ROTATE
#define ROTATE(a, n)     (((a)<<(n))|(((a)&0xffffffff)>>(32-(n))))
#endif

#define HASH_MAKE_STRING(c, s)   do {	\
		unsigned long ll;		\
		ll = (c)->h0; HOST_l2c(ll, (s));	\
		ll = (c)->h1; HOST_l2c(ll, (s));	\
		ll = (c)->h2; HOST_l2c(ll, (s));	\
		ll = (c)->h3; HOST_l2c(ll, (s));	\
		ll = (c)->h4; HOST_l2c(ll, (s));	\
	} while (0)

#ifndef HOST_c2l
#define HOST_c2l(c, l)	(l = (((unsigned long)(*((c)++)))<<24),		\
						 l |= (((unsigned long)(*((c)++)))<<16),		\
						 l |= (((unsigned long)(*((c)++))) << 8),		\
						 l |= (((unsigned long)(*((c)++)))),		\
						 l)
#endif

#define HOST_p_c2l(c, l, n)	{					\
		switch (n) {					\
		case 0:						\
			l = ((unsigned long)(*((c)++)))<<24;	\
		case 1:						\
			l |= ((unsigned long)(*((c)++)))<<16;	\
		case 2:						\
			l |= ((unsigned long)(*((c)++))) << 8;	\
		case 3:						\
			l |= ((unsigned long)(*((c)++)));	\
		} }
#define HOST_p_c2l_p(c, l, sc, len) {					\
		switch (sc) {					\
		case 0:						\
			l = ((unsigned long)(*((c)++)))<<24;	\
			if (--len == 0)				\
				break;				\
		case 1:						\
			l |= ((unsigned long)(*((c)++)))<<16;	\
			if (--len == 0)				\
				break;				\
		case 2:						\
			l |= ((unsigned long)(*((c)++))) << 8;	\
		} }
/* NOTE the pointer is not incremented at the end of this */
#define HOST_c2l_p(c, l, n)	{					\
		l = 0; (c) += n;					\
		switch (n) {					\
		case 3:						\
			l = ((unsigned long)(*(--(c)))) << 8;	\
		case 2:						\
			l |= ((unsigned long)(*(--(c))))<<16;	\
		case 1:						\
			l |= ((unsigned long)(*(--(c))))<<24;	\
		} }
#ifndef HOST_l2c
#define HOST_l2c(l, c)	(*((c)++) = (unsigned char)(((l)>>24)&0xff),	\
						 *((c)++) = (unsigned char)(((l)>>16)&0xff),	\
						 *((c)++) = (unsigned char)(((l) >> 8)&0xff),	\
						 *((c)++) = (unsigned char)(((l))&0xff),	\
						 l)
#endif

#define K_00_19	0x5a827999UL
#define K_20_39 0x6ed9eba1UL
#define K_40_59 0x8f1bbcdcUL
#define K_60_79 0xca62c1d6UL

/* As  pointed out by Wei Dai <weidai@eskimo.com>, F() below can be
 * simplified to the code in F_00_19.  Wei attributes these optimisations
 * to Peter Gutmann's SHS code, and he attributes it to Rich Schroeppel.
 * #define F(x,y,z) (((x) & (y))  |  ((~(x)) & (z)))
 * I've just become aware of another tweak to be made, again from Wei Dai,
 * in F_40_59, (x&a)|(y&a) -> (x|y)&a
 */
#define	F_00_19(b, c, d)	((((c) ^ (d)) & (b)) ^ (d))
#define	F_20_39(b, c, d)	((b) ^ (c) ^ (d))
#define F_40_59(b, c, d)	(((b) & (c)) | (((b)|(c)) & (d)))
#define	F_60_79(b, c, d)	F_20_39(b, c, d)

#define BODY_00_15(xi)		 do {	\
		T = E+K_00_19+F_00_19(B, C, D);	\
		E = D, D = C, C = ROTATE(B, 30), B = A;	\
		A = ROTATE(A, 5)+T+xi;			\
	} while (0)
#define BODY_16_19(xa, xb, xc, xd)	 do {	\
		Xupdate(T, xa, xa, xb, xc, xd);	\
		T += E+K_00_19+F_00_19(B, C, D);	\
		E = D, D = C, C = ROTATE(B, 30), B = A;	\
		A = ROTATE(A, 5)+T;			\
	} while (0)
#define BODY_20_39(xa, xb, xc, xd)	 do {	\
		Xupdate(T, xa, xa, xb, xc, xd);	\
		T += E+K_20_39+F_20_39(B, C, D);	\
		E = D, D = C, C = ROTATE(B, 30), B = A;	\
		A = ROTATE(A, 5)+T;			\
	} while (0)
#define BODY_40_59(xa, xb, xc, xd)	 do {	\
		Xupdate(T, xa, xa, xb, xc, xd);	\
		T += E+K_40_59+F_40_59(B, C, D);	\
		E = D, D = C, C = ROTATE(B, 30), B = A;	\
		A = ROTATE(A, 5)+T;			\
	} while (0)
#define BODY_60_79(xa, xb, xc, xd)	 do {	\
		Xupdate(T, xa, xa, xb, xc, xd);	\
		T = E+K_60_79+F_60_79(B, C, D);	\
		E = D, D = C, C = ROTATE(B, 30), B = A;	\
		A = ROTATE(A, 5)+T+xa;		\
	} while (0)
static void sha1_block_host_order(struct SHAstate_st *c, const void *d, size_t num)
{
	const SHA_LONG *W = d;

	register unsigned long A, B, C, D, E, T;
	int i;
	SHA_LONG	X[16];

	A = c->h0;
	B = c->h1;
	C = c->h2;
	D = c->h3;
	E = c->h4;

	for (;;) {
		for (i = 0; i < 16; i++) {
			X[i] = W[i];
			BODY_00_15(X[i]);
		}

		for (i = 0; i < 4; i++)
			BODY_16_19(X[i],       X[i + 2],      X[i + 8],     X[(i + 13) & 15]);

		for (; i < 24; i++)
			BODY_20_39(X[i & 15],    X[(i + 2) & 15], X[(i + 8) & 15], X[(i + 13) & 15]);

		for (i = 0; i < 20; i++)
			BODY_40_59(X[(i + 8) & 15], X[(i + 10) & 15], X[i & 15],    X[(i + 5) & 15]);

		for (i = 4; i < 24; i++)
			BODY_60_79(X[(i + 8) & 15], X[(i + 10) & 15], X[i & 15],    X[(i + 5) & 15]);

		c->h0 = (c->h0 + A) & 0xffffffffL;
		c->h1 = (c->h1 + B) & 0xffffffffL;
		c->h2 = (c->h2 + C) & 0xffffffffL;
		c->h3 = (c->h3 + D) & 0xffffffffL;
		c->h4 = (c->h4 + E) & 0xffffffffL;

		if (--num == 0)
			break;

		A = c->h0;
		B = c->h1;
		C = c->h2;
		D = c->h3;
		E = c->h4;
		W += SHA_LBLOCK;
	}
}

static void sha1_block_data_order(struct SHAstate_st *c, const void *p, size_t num)
{
	const unsigned char *data = p;
	register unsigned long A, B, C, D, E, T, l;
	int i;
	SHA_LONG	X[16];

	A = c->h0;
	B = c->h1;
	C = c->h2;
	D = c->h3;
	E = c->h4;

	for (;;) {
		for (i = 0; i < 16; i++) {
			HOST_c2l(data, l);
			X[i] = l;
			BODY_00_15(X[i]);
		}

		for (i = 0; i < 4; i++)
			BODY_16_19(X[i],       X[i + 2],      X[i + 8],     X[(i + 13) & 15]);

		for (; i < 24; i++)
			BODY_20_39(X[i & 15],    X[(i + 2) & 15], X[(i + 8) & 15], X[(i + 13) & 15]);

		for (i = 0; i < 20; i++)
			BODY_40_59(X[(i + 8) & 15], X[(i + 10) & 15], X[i & 15],    X[(i + 5) & 15]);

		for (i = 4; i < 24; i++)
			BODY_60_79(X[(i + 8) & 15], X[(i + 10) & 15], X[i & 15],    X[(i + 5) & 15]);

		c->h0 = (c->h0 + A) & 0xffffffffL;
		c->h1 = (c->h1 + B) & 0xffffffffL;
		c->h2 = (c->h2 + C) & 0xffffffffL;
		c->h3 = (c->h3 + D) & 0xffffffffL;
		c->h4 = (c->h4 + E) & 0xffffffffL;

		if (--num == 0)
			break;

		A = c->h0;
		B = c->h1;
		C = c->h2;
		D = c->h3;
		E = c->h4;
	}
}
/*sha_locl.h END*/

#define NID_sha1			64

static int g_state_num = 0, g_state_index;
static unsigned char g_md[SHA_DIGEST_LENGTH];
static unsigned char g_state[STATE_SIZE + SHA_DIGEST_LENGTH];
static long g_md_count[2] = {0, 0};
static long g_entropy;
static int RAND_initialized;

static struct bignum_st *bn_expand2(struct bignum_st *b, int words);
static int HASH_FINAL(unsigned char *md, struct SHAstate_st *c);
static int HASH_UPDATE(struct SHAstate_st *c, const void *data_, size_t len);
static int HASH_INIT(struct SHAstate_st *c);
static int RAND_poll(void);
static int RAND_init(struct env_md_ctx_st *ctx)
{
	return HASH_INIT(ctx->md_data);
}
static int RAND_update(struct env_md_ctx_st *ctx, const void *data, size_t count)
{
	return HASH_UPDATE(ctx->md_data, data, count);
}
static int RAND_final(struct env_md_ctx_st *ctx, unsigned char *md)
{
	return HASH_FINAL(md, ctx->md_data);
}
static void bn_mul_normal(BN_ULONG *r, BN_ULONG *a, int na, BN_ULONG *b, int nb);
#ifdef BN_RECURSION
static void bn_mul_recursive(BN_ULONG *r, BN_ULONG *a, BN_ULONG *b, int n2,
							 int dna, int dnb, BN_ULONG *t);
static void bn_mul_part_recursive(BN_ULONG *r, BN_ULONG *a, BN_ULONG *b, int n,
								  int tna, int tnb, BN_ULONG *t);
static void bn_mul_comba4(BN_ULONG *r, BN_ULONG *a, BN_ULONG *b);
#endif /* BN_RECURSION */
static int BN_mod_exp_mont(struct bignum_st *rr, const struct bignum_st *a, const struct bignum_st *p,
						   const struct bignum_st *m, struct bignum_pool *ctx, struct bn_mont_ctx_st *in_mont);
static int BN_rshift1(struct bignum_st *r, const struct bignum_st *a);
static BN_ULONG bn_mul_add_words(BN_ULONG *rp, const BN_ULONG *ap, int num, BN_ULONG w);
static BN_ULONG bn_mul_words(BN_ULONG *rp, const BN_ULONG *ap, int num, BN_ULONG w);
static BN_ULONG bn_add_words(BN_ULONG *r, const BN_ULONG *a, const BN_ULONG *b, int n);
static BN_ULONG bn_sub_words(BN_ULONG *r, const BN_ULONG *a, const BN_ULONG *b, int n);
static void bn_mul_comba8(BN_ULONG *r, BN_ULONG *a, BN_ULONG *b);
static void bn_sqr_comba8(BN_ULONG *r, const BN_ULONG *a);
static void bn_sqr_comba4(BN_ULONG *r, const BN_ULONG *a);




static int HASH_FINAL(unsigned char *md, struct SHAstate_st *c)
{
	register SHA_LONG *p;
	register unsigned long l;
	register int i, j;
	static const unsigned char end[4] = {0x80, 0x00, 0x00, 0x00};
	const unsigned char *cp = end;
	/* c->num should definitly have room for at least one more byte. */
	p = c->data;
	i = c->num >> 2;
	j = c->num & 0x03;
	l = (j == 0) ? 0 : p[i];
	HOST_p_c2l(cp, l, j);
	p[i++] = l; /* i is the next 'undefined word' */

	if (i > (SHA_LBLOCK - 2)) { /* save room for Nl and Nh */
		if (i < SHA_LBLOCK)
			p[i] = 0;

		sha1_block_host_order(c, p, 1);
		i = 0;
	}

	for (; i < (SHA_LBLOCK - 2); i++)
		p[i] = 0;

	p[SHA_LBLOCK - 2] = c->Nh;
	p[SHA_LBLOCK - 1] = c->Nl;
	sha1_block_host_order(c, p, 1);
#ifndef HASH_MAKE_STRING
#error "HASH_MAKE_STRING must be defined!"
#else
	HASH_MAKE_STRING(c, md);
#endif
	c->num = 0;
	return 1;
}

static int HASH_UPDATE(struct SHAstate_st *c, const void *data_, size_t len)
{
	const unsigned char *data = data_;
	register SHA_LONG *p;
	register SHA_LONG l;
	size_t sw, sc, ew, ec;

	if (len == 0)
		return 1;

	l = (c->Nl + (((SHA_LONG)len) << 3)) & 0xffffffffUL;

	/* 95-05-24 eay Fixed a bug with the overflow handling, thanks to
	 * Wei Dai <weidai@eskimo.com> for pointing it out. */
	if (l < c->Nl) /* overflow */
		c->Nh++;

	c->Nh += (len >> 29);	/* might cause compiler warning on 16-bit */
	c->Nl = l;

	if (c->num != 0) {
		p = c->data;
		sw = c->num >> 2;
		sc = c->num & 0x03;

		if ((c->num + len) >= SHA_CBLOCK) {
			l = p[sw];
			HOST_p_c2l(data, l, sc);
			p[sw++] = l;

			for (; sw < SHA_LBLOCK; sw++) {
				HOST_c2l(data, l);
				p[sw] = l;
			}

			sha1_block_host_order(c, p, 1);
			len -= (SHA_CBLOCK - c->num);
			c->num = 0;
			/* drop through and do the rest */
		} else {
			c->num += (unsigned int)len;

			if ((sc + len) < 4) { /* ugly, add char's to a word */
				l = p[sw];
				HOST_p_c2l_p(data, l, sc, len);
				p[sw] = l;
			} else {
				ew = (c->num >> 2);
				ec = (c->num & 0x03);

				if (sc)
					l = p[sw];

				HOST_p_c2l(data, l, sc);
				p[sw++] = l;

				for (; sw < ew; sw++) {
					HOST_c2l(data, l);
					p[sw] = l;
				}

				if (ec) {
					HOST_c2l_p(data, l, ec);
					p[sw] = l;
				}
			}

			return 1;
		}
	}

	sw = len / SHA_CBLOCK;

	if (sw > 0) {
		{
			sha1_block_data_order(c, data, sw);
			sw *= SHA_CBLOCK;
			data += sw;
			len -= sw;
		}
	}

	if (len != 0) {
		p = c->data;
		c->num = len;
		ew = len >> 2;	/* words to copy */
		ec = len & 0x03;

		for (; ew; ew--, p++) {
			HOST_c2l(data, l);
			*p = l;
		}

		HOST_c2l_p(data, l, ec);
		*p = l;
	}

	return 1;
}

static int HASH_INIT(struct SHAstate_st *c)
{
	c->h0 = INIT_DATA_h0;
	c->h1 = INIT_DATA_h1;
	c->h2 = INIT_DATA_h2;
	c->h3 = INIT_DATA_h3;
	c->h4 = INIT_DATA_h4;
	c->Nl = 0;
	c->Nh = 0;
	c->num = 0;
	return 1;
}

static struct bignum_pool *BN_CTX_new(void)
{
	struct bignum_pool *ret;

	os_alloc_mem(NULL, (UCHAR **)&ret, sizeof(struct bignum_pool));

	if (!ret)
		return NULL;

	/* Initialise the structure */
	ret->head = ret->currentvar = ret->tail = NULL;
	ret->used = ret->size = 0;
	return ret;
}

static const struct bignum_st *BN_value_one(void)
{
	static BN_ULONG data_one = 1L;
	static struct bignum_st const_one = {&data_one, 1, 1, BN_FLG_STATIC_DATA};

	return &const_one;
}

static int BN_num_bits_word(BN_ULONG l)
{
	static const char bits[256] = {
		0, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4,
		5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
		6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
		6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
		7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
		7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
		7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
		7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
		8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
		8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
		8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
		8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
		8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
		8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
		8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
		8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
	};
	{
		if (l & 0xffff0000L) {
			if (l & 0xff000000L)
				return (bits[(int)(l >> 24L)] + 24);
			else
				return (bits[(int)(l >> 16L)] + 16);
		} else {
			if (l & 0xff00L)
				return (bits[(int)(l >> 8)] + 8);
			else
				return bits[(int)(l)];
		}
	}
}

static int BN_num_bits(const struct bignum_st *a)
{
	int i = a->top - 1;

	if (BN_is_zero(a))
		return 0;

	return ((i * BN_BITS2) + BN_num_bits_word(a->d[i]));
}

static void BN_clear_free(struct bignum_st *a)
{
	int i;

	if (a == NULL)
		return;

	if (a->d != NULL) {
		if (!(BN_get_flags(a, BN_FLG_STATIC_DATA)))
			os_free_mem(a->d);
	}

	i = BN_get_flags(a, BN_FLG_MALLOCED);

	if (i)
		os_free_mem(a);
}

static void BN_free(struct bignum_st *a)
{
	if (a == NULL)
		return;

	if ((a->d != NULL) && !(BN_get_flags(a, BN_FLG_STATIC_DATA)))
		os_free_mem(a->d);

	if (a->flags & BN_FLG_MALLOCED)
		os_free_mem(a);
	else
		a->d = NULL;
}

static void BN_init(struct bignum_st *a)
{
	memset(a, 0, sizeof(struct bignum_st));
}

static struct bignum_st *BN_new(void)
{
	struct bignum_st *ret;

	os_alloc_mem(NULL, (UCHAR **)&ret, sizeof(struct bignum_st));

	if (ret == NULL)
		return NULL;

	ret->flags = BN_FLG_MALLOCED;
	ret->top = 0;
	ret->dmax = 0;
	ret->d = NULL;
	return ret;
}

/* This is used both by bn_expand2() and bn_dup_expand() */
/* The caller MUST check that words > b->dmax before calling this */
static BN_ULONG *bn_expand_internal(const struct bignum_st *b, int words)
{
	BN_ULONG *A, *a = NULL;
	const BN_ULONG *B;
	int i;

	if (words > (INT_MAX / (4 * BN_BITS2)))
		return NULL;

	if (BN_get_flags(b, BN_FLG_STATIC_DATA))
		return NULL;

	os_alloc_mem(NULL, (UCHAR **)&A, sizeof(BN_ULONG)*words);
	a = A;

	if (A == NULL)
		return NULL;

	B = b->d;

	/* Check if the previous number needs to be copied */
	if (B != NULL) {
		for (i = b->top >> 2; i > 0; i--, A += 4, B += 4) {
			/*
			 * The fact that the loop is unrolled
			 * 4-wise is a tribute to Intel. It's
			 * the one that doesn't have enough
			 * registers to accomodate more data.
			 * I'd unroll it 8-wise otherwise:-)
			 *
			 *		<appro@fy.chalmers.se>
			 */
			BN_ULONG a0, a1, a2, a3;

			a0 = B[0];
			a1 = B[1];
			a2 = B[2];
			a3 = B[3];
			A[0] = a0;
			A[1] = a1;
			A[2] = a2;
			A[3] = a3;
		}

		switch (b->top & 3) {
		case 3:
			A[2] = B[2];

		case 2:
			A[1] = B[1];

		case 1:
			A[0] = B[0];

		case 0: /* workaround for ultrix cc: without 'case 0', the optimizer does
			 * the switch table by doing a=top&3; a--; goto jump_table[a];
			 * which fails for top== 0 */
			;
		}
	}

	return a;
}

/* This is an internal function that should not be used in applications.
 * It ensures that 'b' has enough room for a 'words' word number
 * and initialises any unused part of b->d with leading zeros.
 * It is mostly used by the various struct bignum_st routines. If there is an error,
 * NULL is returned. If not, 'b' is returned. */

static struct bignum_st *bn_expand2(struct bignum_st *b, int words)
{
	if (words > b->dmax) {
		BN_ULONG *a = bn_expand_internal(b, words);

		if (!a)
			return NULL;

		if (b->d)
			os_free_mem(b->d);

		b->d = a;
		b->dmax = words;
	}

	return b;
}

static void BN_clear(struct bignum_st *a)
{
	if (a->d != NULL)
		memset(a->d, 0, a->dmax * sizeof(a->d[0]));

	a->top = 0;
}

static int BN_set_word(struct bignum_st *a, BN_ULONG w)
{
	if (NULL == ((1	<= (a)->dmax) ? (a) : bn_expand2((a), 1))) {
		DHPRINT("BN_set_word NULL\n");
		return 0;
	}

	a->d[0] = w;
	a->top = (w ? 1 : 0);
	return 1;
}

static struct bignum_st *BN_bin2bn(const unsigned char *s, int len, struct bignum_st *ret)
{
	unsigned int i, m;
	unsigned int n;
	BN_ULONG l;
	struct bignum_st  *bn = NULL;

	if (ret == NULL)
		ret = bn = BN_new();

	if (ret == NULL)
		return NULL;

	l = 0;
	n = len;

	if (n == 0) {
		ret->top = 0;
		return ret;
	}

	i = ((n - 1) / BN_BYTES) + 1;
	m = ((n - 1) % (BN_BYTES));

	if (bn_wexpand(ret, (int)i) == NULL) {
		BN_free(bn);
		return NULL;
	}

	ret->top = i;

	while (n--) {
		l = (l << 8L) | *(s++);

		if (m-- == 0) {
			ret->d[--i] = l;
			l = 0;
			m = BN_BYTES - 1;
		}
	}

	/* need to call this due to clear byte at top if avoiding
	 * having the top bit set (-ve number) */
	bn_correct_top(ret);
	return ret;
}

static int BN_ucmp(const struct bignum_st *a, const struct bignum_st *b)
{
	int i;
	BN_ULONG t1, t2, *ap, *bp;

	i = a->top - b->top;

	if (i != 0)
		return i;

	ap = a->d;
	bp = b->d;

	for (i = a->top - 1; i >= 0; i--) {
		t1 = ap[i];
		t2 = bp[i];

		if (t1 != t2)
			return ((t1 > t2) ? 1 : -1);
	}

	return 0;
}

/* ignore negative */
static int BN_bn2bin(const struct bignum_st *a, unsigned char *to)
{
	int n, i;
	BN_ULONG l;

	n = i = BN_num_bytes(a);

	while (i--) {
		l = a->d[i / BN_BYTES];
		*(to++) = (unsigned char)(l >> (8 * (i % BN_BYTES))) & 0xff;
	}

	return n;
}
#ifdef BN_RECURSION
static int BN_cmp(const struct bignum_st *a, const struct bignum_st *b)
{
	int i;
	int gt, lt;
	BN_ULONG t1, t2;

	if ((a == NULL) || (b == NULL)) {
		if (a != NULL)
			return -1;
		else if (b != NULL)
			return 1;
		else
			return 0;
	}

	{
		gt = 1;
		lt = -1;
	}

	if (a->top > b->top)
		return gt;

	if (a->top < b->top)
		return lt;

	for (i = a->top - 1; i >= 0; i--) {
		t1 = a->d[i];
		t2 = b->d[i];

		if (t1 > t2)
			return gt;

		if (t1 < t2)
			return lt;
	}

	return 0;
}
#endif /*BN_RECURSION */
static int BN_set_bit(struct bignum_st *a, int n)
{
	int i, j, k;

	if (n < 0)
		return 0;

	i = n / BN_BITS2;
	j = n % BN_BITS2;

	if (a->top <= i) {
		if (bn_wexpand(a, i + 1) == NULL)
			return 0;

		for (k = a->top; k < i + 1; k++)
			a->d[k] = 0;

		a->top = i + 1;
	}

	a->d[i] |= (((BN_ULONG)1) << j);
	return 1;
}

static int BN_is_bit_set(const struct bignum_st *a, int n)
{
	int i, j;

	if (n < 0)
		return 0;

	i = n / BN_BITS2;
	j = n % BN_BITS2;

	if (a->top <= i)
		return 0;

	return ((a->d[i] & (((BN_ULONG)1) << j)) ? 1 : 0);
}

#ifdef BN_RECURSION
static int bn_cmp_words(const BN_ULONG *a, const BN_ULONG *b, int n)
{
	int i;
	BN_ULONG aa, bb;

	aa = a[n - 1];
	bb = b[n - 1];

	if (aa != bb)
		return ((aa > bb) ? 1 : -1);

	for (i = n - 2; i >= 0; i--) {
		aa = a[i];
		bb = b[i];

		if (aa != bb)
			return ((aa > bb) ? 1 : -1);
	}

	return 0;
}

/* Here follows a specialised variants of bn_cmp_words().  It has the
   property of performing the operation on arrays of different sizes.
   The sizes of those arrays is expressed through cl, which is the
   common length ( basicall, min(len(a),len(b)) ), and dl, which is the
   delta between the two lengths, calculated as len(a)-len(b).
   All lengths are the number of BN_ULONGs...  */
static int bn_cmp_part_words(const BN_ULONG *a, const BN_ULONG *b,
							 int cl, int dl)
{
	int n, i;

	n = cl - 1;

	if (dl < 0) {
		for (i = dl; i < 0; i++) {
			if (b[n - i] != 0)
				return -1; /* a < b */
		}
	}

	if (dl > 0) {
		for (i = dl; i > 0; i--) {
			if (a[n + i] != 0)
				return 1; /* a > b */
		}
	}

	return bn_cmp_words(a, b, cl);
}
#endif /*BN_RECURSION */
static int BN_rshift1(struct bignum_st *r, const struct bignum_st *a)
{
	BN_ULONG *ap, *rp, t, c;
	int i;

	if (BN_is_zero(a)) {
		BN_zero(r);
		return 1;
	}

	if (a != r) {
		if (bn_wexpand(r, a->top) == NULL)
			return 0;

		r->top = a->top;
	}

	ap = a->d;
	rp = r->d;
	c = 0;

	for (i = a->top - 1; i >= 0; i--) {
		t = ap[i];
		rp[i] = ((t >> 1)&BN_MASK2) | c;
		c = (t & 1) ? BN_TBIT : 0;
	}

	bn_correct_top(r);
	return 1;
}

static int BN_lshift(struct bignum_st *r, const struct bignum_st *a, int n)
{
	int i, nw, lb, rb;
	BN_ULONG *t, *f;
	BN_ULONG l;

	nw = n / BN_BITS2;

	if (bn_wexpand(r, a->top + nw + 1) == NULL)
		return 0;

	lb = n % BN_BITS2;
	rb = BN_BITS2 - lb;
	f = a->d;
	t = r->d;
	t[a->top + nw] = 0;

	if (lb == 0)
		for (i = a->top - 1; i >= 0; i--)
			t[nw + i] = f[i];
	else
		for (i = a->top - 1; i >= 0; i--) {
			l = f[i];
			t[nw + i + 1] |= (l >> rb)&BN_MASK2;
			t[nw + i] = (l << lb)&BN_MASK2;
		}

	memset(t, 0, nw * sizeof(t[0]));
	/*	for (i=0; i<nw; i++)
			t[i]=0;*/
	r->top = a->top + nw + 1;
	bn_correct_top(r);
	return 1;
}

static int BN_rshift(struct bignum_st *r, const struct bignum_st *a, int n)
{
	int i, j, nw, lb, rb;
	BN_ULONG *t, *f;
	BN_ULONG l, tmp;

	nw = n / BN_BITS2;
	rb = n % BN_BITS2;
	lb = BN_BITS2 - rb;

	if (nw > a->top || a->top == 0) {
		BN_zero(r);
		return 1;
	}

	if (r != a) {
		if (bn_wexpand(r, a->top - nw + 1) == NULL)
			return 0;
	} else {
		if (n == 0)
			return 1; /* or the copying loop will go berserk */
	}

	f = &(a->d[nw]);
	t = r->d;
	j = a->top - nw;
	r->top = j;

	if (rb == 0) {
		for (i = j; i != 0; i--)
			*(t++) = *(f++);
	} else {
		l = *(f++);

		for (i = j - 1; i != 0; i--) {
			tmp = (l >> rb)&BN_MASK2;
			l = *(f++);
			*(t++) = (tmp | (l << lb))&BN_MASK2;
		}

		*(t++) = (l >> rb)&BN_MASK2;
	}

	bn_correct_top(r);
	return 1;
}

static void BN_POOL_release(struct bignum_pool *p, unsigned int num)
{
	unsigned int offset = (p->used - 1) % BN_CTX_POOL_SIZE;

	p->used -= num;

	while (num--) {
		if (!offset) {
			offset = BN_CTX_POOL_SIZE - 1;
			p->currentvar = p->currentvar->prev;
		} else
			offset--;
	}
}

static void BN_POOL_finish(struct bignum_pool *p)
{
	while (p->head) {
		struct bignum_st *bn = p->head->vals;
		unsigned int loop = 0;

		while (loop++ < BN_CTX_POOL_SIZE) {
			if (bn->d)
				BN_clear_free(bn);

			bn++;
		}

		p->currentvar = p->head->next;
		os_free_mem(p->head);
		p->head = p->currentvar;
	}
}


static struct bignum_st *BN_POOL_get(struct bignum_pool *p)
{
	struct bignum_st *ret = NULL;

	if (p->used == p->size) {
		struct bignum_st *bn;
		unsigned int loop = 0;
		struct bignum_pool_item *item;

		os_alloc_mem(NULL, (UCHAR **)&item, sizeof(struct bignum_pool_item));

		if (!item)
			goto end;

		/* Initialise the structure */
		bn = item->vals;

		while (loop++ < BN_CTX_POOL_SIZE)
			BN_init(bn++);

		item->prev = p->tail;
		item->next = NULL;

		/* Link it in */
		if (!p->head)
			p->head = p->currentvar = p->tail = item;
		else {
			p->tail->next = item;
			p->tail = item;
			p->currentvar = item;
		}

		p->size += BN_CTX_POOL_SIZE;
		p->used++;
		/* Return the first bignum from the new pool */
		ret = item->vals;
	}

	if (!p->used)
		p->currentvar = p->head;
	else if ((p->used % BN_CTX_POOL_SIZE) == 0)
		p->currentvar = p->currentvar->next;

	ret = p->currentvar->vals + ((p->used) % BN_CTX_POOL_SIZE);
	p->used++;
end:

	if (ret == NULL)
		DHPRINT("BN_POOL_get ERROR !!!");
	else {
		/* OK, make sure the returned bignum is "zero" */
		BN_zero(ret);
	}

	return ret;
}

static struct bignum_st *BN_copy(struct bignum_st *a, const struct bignum_st *b)
{
	int i;
	BN_ULONG *A;
	const BN_ULONG *B;

	if (BN_ucmp(a, b) == 0)
		return a;

	if (bn_wexpand(a, b->top) == NULL)
		return NULL;

	A = a->d;
	B = b->d;

	for (i = b->top >> 2; i > 0; i--, A += 4, B += 4) {
		BN_ULONG a0, a1, a2, a3;

		a0 = B[0];
		a1 = B[1];
		a2 = B[2];
		a3 = B[3];
		A[0] = a0;
		A[1] = a1;
		A[2] = a2;
		A[3] = a3;
	}

	switch (b->top & 3) {
	case 3:
		A[2] = B[2];

	case 2:
		A[1] = B[1];

	case 1:
		A[0] = B[0];

	case 0:
		; /* ultrix cc workaround, see comments in bn_expand_internal */
	}

	a->top = b->top;
	return a;
}

/* Divide h,l by d and return the result. */
/* I need to test this some more :-( */
BN_ULONG bn_div_words(BN_ULONG h, BN_ULONG l, BN_ULONG d)
{
	BN_ULONG dh, dl, q, ret = 0, th, tl, t;
	int i, count = 2;

	if (d == 0)
		return BN_MASK2;

	i = BN_num_bits_word(d);
	i = BN_BITS2 - i;

	if (h >= d)
		h -= d;

	if (i) {
		d <<= i;
		h = (h << i) | (l >> (BN_BITS2 - i));
		l <<= i;
	}

	dh = (d & BN_MASK2h) >> BN_BITS4;
	dl = (d & BN_MASK2l);

	for (;;) {
		if ((h >> BN_BITS4) == dh)
			q = BN_MASK2l;
		else
			q = h / dh;

		th = q * dh;
		tl = dl * q;

		for (;;) {
			t = h - th;

			if ((t & BN_MASK2h) ||
				((tl) <= (
					 (t << BN_BITS4) |
					 ((l & BN_MASK2h) >> BN_BITS4))))
				break;

			q--;
			th -= dh;
			tl -= dl;
		}

		t = (tl >> BN_BITS4);
		tl = (tl << BN_BITS4)&BN_MASK2h;
		th += t;

		if (l < tl)
			th++;

		l -= tl;

		if (h < th) {
			h += d;
			q--;
		}

		h -= th;

		if (--count == 0)
			break;

		ret = q << BN_BITS4;
		h = ((h << BN_BITS4) | (l >> BN_BITS4))&BN_MASK2;
		l = (l & BN_MASK2l) << BN_BITS4;
	}

	ret |= q;
	return ret;
}

/* BN_div computes  dv := num / divisor,  rounding towards zero, and sets up
 * rm  such that  dv*divisor + rm = num  holds.
 * If 'dv' or 'rm' is NULL, the respective value is not returned.
 */
static int BN_div(struct bignum_st *dv, struct bignum_st *rm, const struct bignum_st *num, const struct bignum_st *divisor,
				  struct bignum_pool *ctx)
{
	int release = 0;
	int norm_shift, i, loop;
	struct bignum_st *tmp, wnum, *snum, *sdiv, *res;
	BN_ULONG *resp, *wnump;
	BN_ULONG d0, d1;
	int num_n, div_n;

	if (BN_is_zero(divisor))
		return 0;

	if (BN_ucmp(num, divisor) < 0) {
		if (rm != NULL) {
			if (BN_copy(rm, num) == NULL)
				return 0;
		}

		if (dv != NULL)
			BN_zero(dv);

		return 1;
	}

	tmp = BN_POOL_get(ctx);
	snum = BN_POOL_get(ctx);
	sdiv = BN_POOL_get(ctx);
	release += 3;

	if (dv == NULL) {
		res = BN_POOL_get(ctx);
		release++;
	} else
		res = dv;

	if (sdiv == NULL || res == NULL)
		goto err;

	/* First we normalise the numbers */
	norm_shift = BN_BITS2 - ((BN_num_bits(divisor)) % BN_BITS2);

	if (!(BN_lshift(sdiv, divisor, norm_shift)))
		goto err;

	norm_shift += BN_BITS2;

	if (!(BN_lshift(snum, num, norm_shift)))
		goto err;

	div_n = sdiv->top;
	num_n = snum->top;
	loop = num_n - div_n;
	/* Lets setup a 'window' into snum
	 * This is the part that corresponds to the current
	 * 'area' being divided */
	wnum.d     = &(snum->d[loop]);
	wnum.top   = div_n;
	/* only needed when BN_ucmp messes up the values between top and max */
	wnum.dmax  = snum->dmax - loop; /* so we don't step out of bounds */
	/* Get the top 2 words of sdiv */
	/* div_n=sdiv->top; */
	d0 = sdiv->d[div_n - 1];
	d1 = (div_n == 1) ? 0 : sdiv->d[div_n - 2];
	/* pointer to the 'top' of snum */
	wnump = &(snum->d[num_n - 1]);

	/* Setup to 'res' */
	if (!bn_wexpand(res, (loop + 1)))
		goto err;

	res->top = loop;
	resp = &(res->d[loop - 1]);

	/* space for temp */
	if (!bn_wexpand(tmp, (div_n + 1)))
		goto err;

	if (BN_ucmp(&wnum, sdiv) >= 0) {
		bn_sub_words(wnum.d, wnum.d, sdiv->d, div_n);
		*resp = 1;
	} else
		res->top--;

	/* if res->top == 0 then clear the neg value otherwise decrease
	 * the resp pointer */
	if (res->top != 0)
		resp--;

	for (i = 0; i < loop - 1; i++, wnump--, resp--) {
		BN_ULONG q, l0;
		/* the first part of the loop uses the top two words of
		 * snum and sdiv to calculate a BN_ULONG q such that
		 * | wnum - sdiv * q | < sdiv */
		BN_ULONG n0, n1, rem = 0;

		n0 = wnump[0];
		n1 = wnump[-1];

		if (n0 == d0)
			q = BN_MASK2;
		else {		/* n0 < d0 */
			BN_ULONG t2l, t2h, ql, qh;

			q = bn_div_words(n0, n1, d0);
			rem = (n1 - q * d0)&BN_MASK2;
			t2l = LBITS(d1);
			t2h = HBITS(d1);
			ql = LBITS(q);
			qh = HBITS(q);
			mul64(t2l, t2h, ql, qh); /* t2=(BN_ULLONG)d1*q; */

			for (;;) {
				if ((t2h < rem) ||
					((t2h == rem) && (t2l <= wnump[-2])))
					break;

				q--;
				rem += d0;

				if (rem < d0)
					break; /* don't let rem overflow */

				if (t2l < d1)
					t2h--;

				t2l -= d1;
			}
		}

		l0 = bn_mul_words(tmp->d, sdiv->d, div_n, q);
		tmp->d[div_n] = l0;
		wnum.d--;

		/* ingore top values of the bignums just sub the two
		 * BN_ULONG arrays with bn_sub_words */
		if (bn_sub_words(wnum.d, wnum.d, tmp->d, div_n + 1)) {
			/* Note: As we have considered only the leading
			 * two BN_ULONGs in the calculation of q, sdiv * q
			 * might be greater than wnum (but then (q-1) * sdiv
			 * is less or equal than wnum)
			 */
			q--;

			if (bn_add_words(wnum.d, wnum.d, sdiv->d, div_n))
				/* we can't have an overflow here (assuming
				 * that q != 0, but if q == 0 then tmp is
				 * zero anyway) */
				(*wnump)++;
		}

		/* store part of the result */
		*resp = q;
	}

	bn_correct_top(snum);

	if (rm != NULL) {
		/* Keep a copy of the neg flag in num because if rm==num
		 * BN_rshift() will overwrite it.
		 */
		BN_rshift(rm, snum, norm_shift);
	}

	BN_POOL_release(ctx, release);
	return 1;
err:
	BN_POOL_release(ctx, release);
	return 0;
}
/*bn_div END*/

/*bn_asm START*/
static BN_ULONG bn_mul_add_words(BN_ULONG *rp, const BN_ULONG *ap, int num, BN_ULONG w)
{
	BN_ULONG c = 0;
	BN_ULONG bl, bh;

	if (num <= 0)
		return ((BN_ULONG)0);

	bl = LBITS(w);
	bh = HBITS(w);

	for (;;) {
		mul_add(rp[0], ap[0], bl, bh, c);

		if (--num == 0)
			break;

		mul_add(rp[1], ap[1], bl, bh, c);

		if (--num == 0)
			break;

		mul_add(rp[2], ap[2], bl, bh, c);

		if (--num == 0)
			break;

		mul_add(rp[3], ap[3], bl, bh, c);

		if (--num == 0)
			break;

		ap += 4;
		rp += 4;
	}

	return c;
}

static BN_ULONG bn_mul_words(BN_ULONG *rp, const BN_ULONG *ap, int num, BN_ULONG w)
{
	BN_ULONG carry = 0;
	BN_ULONG bl, bh;

	if (num <= 0)
		return ((BN_ULONG)0);

	bl = LBITS(w);
	bh = HBITS(w);

	for (;;) {
		mul(rp[0], ap[0], bl, bh, carry);

		if (--num == 0)
			break;

		mul(rp[1], ap[1], bl, bh, carry);

		if (--num == 0)
			break;

		mul(rp[2], ap[2], bl, bh, carry);

		if (--num == 0)
			break;

		mul(rp[3], ap[3], bl, bh, carry);

		if (--num == 0)
			break;

		ap += 4;
		rp += 4;
	}

	return carry;
}

static void bn_sqr_words(BN_ULONG *r, const BN_ULONG *a, int n)
{
	if (n <= 0)
		return;

	for (;;) {
		sqr64(r[0], r[1], a[0]);

		if (--n == 0)
			break;

		sqr64(r[2], r[3], a[1]);

		if (--n == 0)
			break;

		sqr64(r[4], r[5], a[2]);

		if (--n == 0)
			break;

		sqr64(r[6], r[7], a[3]);

		if (--n == 0)
			break;

		a += 4;
		r += 8;
	}
}

static BN_ULONG bn_add_words(BN_ULONG *r, const BN_ULONG *a, const BN_ULONG *b, int n)
{
	BN_ULONG c, l, t;

	if (n <= 0)
		return ((BN_ULONG)0);

	c = 0;

	for (;;) {
		t = a[0];
		t = (t + c)&BN_MASK2;
		c = (t < c);
		l = (t + b[0])&BN_MASK2;
		c += (l < t);
		r[0] = l;

		if (--n <= 0)
			break;

		t = a[1];
		t = (t + c)&BN_MASK2;
		c = (t < c);
		l = (t + b[1])&BN_MASK2;
		c += (l < t);
		r[1] = l;

		if (--n <= 0)
			break;

		t = a[2];
		t = (t + c)&BN_MASK2;
		c = (t < c);
		l = (t + b[2])&BN_MASK2;
		c += (l < t);
		r[2] = l;

		if (--n <= 0)
			break;

		t = a[3];
		t = (t + c)&BN_MASK2;
		c = (t < c);
		l = (t + b[3])&BN_MASK2;
		c += (l < t);
		r[3] = l;

		if (--n <= 0)
			break;

		a += 4;
		b += 4;
		r += 4;
	}

	return ((BN_ULONG)c);
}

static BN_ULONG bn_sub_words(BN_ULONG *r, const BN_ULONG *a, const BN_ULONG *b, int n)
{
	BN_ULONG t1, t2;
	int c = 0;

	if (n <= 0)
		return ((BN_ULONG)0);

	for (;;) {
		t1 = a[0];
		t2 = b[0];
		r[0] = (t1 - t2 - c)&BN_MASK2;

		if (t1 != t2)
			c = (t1 < t2);

		if (--n <= 0)
			break;

		t1 = a[1];
		t2 = b[1];
		r[1] = (t1 - t2 - c)&BN_MASK2;

		if (t1 != t2)
			c = (t1 < t2);

		if (--n <= 0)
			break;

		t1 = a[2];
		t2 = b[2];
		r[2] = (t1 - t2 - c)&BN_MASK2;

		if (t1 != t2)
			c = (t1 < t2);

		if (--n <= 0)
			break;

		t1 = a[3];
		t2 = b[3];
		r[3] = (t1 - t2 - c)&BN_MASK2;

		if (t1 != t2)
			c = (t1 < t2);

		if (--n <= 0)
			break;

		a += 4;
		b += 4;
		r += 4;
	}

	return c;
}

#ifdef BN_MUL_COMBA

#undef bn_mul_comba8
#undef bn_mul_comba4
#undef bn_sqr_comba8
#undef bn_sqr_comba4

/* mul_add_c(a,b,c0,c1,c2)  -- c+=a*b for three word number c=(c2,c1,c0) */
/* mul_add_c2(a,b,c0,c1,c2) -- c+=2*a*b for three word number c=(c2,c1,c0) */
/* sqr_add_c(a,i,c0,c1,c2)  -- c+=a[i]^2 for three word number c=(c2,c1,c0) */
/* sqr_add_c2(a,i,c0,c1,c2) -- c+=2*a[i]*a[j] for three word number c=(c2,c1,c0) */

#define mul_add_c(a, b, c0, c1, c2) \
	do {			    \
		t1 = LBITS(a); t2 = HBITS(a); \
		bl = LBITS(b); bh = HBITS(b); \
		mul64(t1, t2, bl, bh); \
		c0 = (c0+t1) & BN_MASK2; \
		if ((c0) < t1)		\
			t2++;		\
		c1 = (c1+t2) & BN_MASK2;\
		if ((c1) < t2)		\
			c2++;		\
	} while (0)

#define mul_add_c2(a, b, c0, c1, c2) \
	do {			    \
		t1 = LBITS(a); t2 = HBITS(a); \
		bl = LBITS(b); bh = HBITS(b); \
		mul64(t1, t2, bl, bh); \
		if (t2 & BN_TBIT)	\
			c2++;		\
		t2 = (t2+t2)&BN_MASK2; \
		if (t1 & BN_TBIT)	\
			t2++;		\
		t1 = (t1+t1)&BN_MASK2; \
		c0 = (c0+t1)&BN_MASK2;  \
		if ((c0 < t1) && (((++t2)&BN_MASK2) == 0)) \
			c2++;					\
		c1 = (c1+t2)&BN_MASK2;		\
		if ((c1) < t2)			\
			c2++;		\
	} while (0)

#define sqr_add_c(a, i, c0, c1, c2) \
	do {			    \
		sqr64(t1, t2, (a)[i]); \
		c0 = (c0+t1)&BN_MASK2; \
		if ((c0) < t1)		\
			t2++;		\
		c1 = (c1+t2)&BN_MASK2;  \
		if ((c1) < t2)		\
			c2++;		\
	} while (0)

#define sqr_add_c2(a, i, j, c0, c1, c2) \
	mul_add_c2((a)[i], (a)[j], c0, c1, c2)

static void bn_mul_comba8(BN_ULONG *r, BN_ULONG *a, BN_ULONG *b)
{
	BN_ULONG bl, bh;
	BN_ULONG t1, t2;
	BN_ULONG c1, c2, c3;

	c1 = 0;
	c2 = 0;
	c3 = 0;
	mul_add_c(a[0], b[0], c1, c2, c3);
	r[0] = c1;
	c1 = 0;
	mul_add_c(a[0], b[1], c2, c3, c1);
	mul_add_c(a[1], b[0], c2, c3, c1);
	r[1] = c2;
	c2 = 0;
	mul_add_c(a[2], b[0], c3, c1, c2);
	mul_add_c(a[1], b[1], c3, c1, c2);
	mul_add_c(a[0], b[2], c3, c1, c2);
	r[2] = c3;
	c3 = 0;
	mul_add_c(a[0], b[3], c1, c2, c3);
	mul_add_c(a[1], b[2], c1, c2, c3);
	mul_add_c(a[2], b[1], c1, c2, c3);
	mul_add_c(a[3], b[0], c1, c2, c3);
	r[3] = c1;
	c1 = 0;
	mul_add_c(a[4], b[0], c2, c3, c1);
	mul_add_c(a[3], b[1], c2, c3, c1);
	mul_add_c(a[2], b[2], c2, c3, c1);
	mul_add_c(a[1], b[3], c2, c3, c1);
	mul_add_c(a[0], b[4], c2, c3, c1);
	r[4] = c2;
	c2 = 0;
	mul_add_c(a[0], b[5], c3, c1, c2);
	mul_add_c(a[1], b[4], c3, c1, c2);
	mul_add_c(a[2], b[3], c3, c1, c2);
	mul_add_c(a[3], b[2], c3, c1, c2);
	mul_add_c(a[4], b[1], c3, c1, c2);
	mul_add_c(a[5], b[0], c3, c1, c2);
	r[5] = c3;
	c3 = 0;
	mul_add_c(a[6], b[0], c1, c2, c3);
	mul_add_c(a[5], b[1], c1, c2, c3);
	mul_add_c(a[4], b[2], c1, c2, c3);
	mul_add_c(a[3], b[3], c1, c2, c3);
	mul_add_c(a[2], b[4], c1, c2, c3);
	mul_add_c(a[1], b[5], c1, c2, c3);
	mul_add_c(a[0], b[6], c1, c2, c3);
	r[6] = c1;
	c1 = 0;
	mul_add_c(a[0], b[7], c2, c3, c1);
	mul_add_c(a[1], b[6], c2, c3, c1);
	mul_add_c(a[2], b[5], c2, c3, c1);
	mul_add_c(a[3], b[4], c2, c3, c1);
	mul_add_c(a[4], b[3], c2, c3, c1);
	mul_add_c(a[5], b[2], c2, c3, c1);
	mul_add_c(a[6], b[1], c2, c3, c1);
	mul_add_c(a[7], b[0], c2, c3, c1);
	r[7] = c2;
	c2 = 0;
	mul_add_c(a[7], b[1], c3, c1, c2);
	mul_add_c(a[6], b[2], c3, c1, c2);
	mul_add_c(a[5], b[3], c3, c1, c2);
	mul_add_c(a[4], b[4], c3, c1, c2);
	mul_add_c(a[3], b[5], c3, c1, c2);
	mul_add_c(a[2], b[6], c3, c1, c2);
	mul_add_c(a[1], b[7], c3, c1, c2);
	r[8] = c3;
	c3 = 0;
	mul_add_c(a[2], b[7], c1, c2, c3);
	mul_add_c(a[3], b[6], c1, c2, c3);
	mul_add_c(a[4], b[5], c1, c2, c3);
	mul_add_c(a[5], b[4], c1, c2, c3);
	mul_add_c(a[6], b[3], c1, c2, c3);
	mul_add_c(a[7], b[2], c1, c2, c3);
	r[9] = c1;
	c1 = 0;
	mul_add_c(a[7], b[3], c2, c3, c1);
	mul_add_c(a[6], b[4], c2, c3, c1);
	mul_add_c(a[5], b[5], c2, c3, c1);
	mul_add_c(a[4], b[6], c2, c3, c1);
	mul_add_c(a[3], b[7], c2, c3, c1);
	r[10] = c2;
	c2 = 0;
	mul_add_c(a[4], b[7], c3, c1, c2);
	mul_add_c(a[5], b[6], c3, c1, c2);
	mul_add_c(a[6], b[5], c3, c1, c2);
	mul_add_c(a[7], b[4], c3, c1, c2);
	r[11] = c3;
	c3 = 0;
	mul_add_c(a[7], b[5], c1, c2, c3);
	mul_add_c(a[6], b[6], c1, c2, c3);
	mul_add_c(a[5], b[7], c1, c2, c3);
	r[12] = c1;
	c1 = 0;
	mul_add_c(a[6], b[7], c2, c3, c1);
	mul_add_c(a[7], b[6], c2, c3, c1);
	r[13] = c2;
	c2 = 0;
	mul_add_c(a[7], b[7], c3, c1, c2);
	r[14] = c3;
	r[15] = c1;
}
#ifdef BN_RECURSION
static void bn_mul_comba4(BN_ULONG *r, BN_ULONG *a, BN_ULONG *b)
{
	BN_ULONG bl, bh;
	BN_ULONG t1, t2;
	BN_ULONG c1, c2, c3;

	c1 = 0;
	c2 = 0;
	c3 = 0;
	mul_add_c(a[0], b[0], c1, c2, c3);
	r[0] = c1;
	c1 = 0;
	mul_add_c(a[0], b[1], c2, c3, c1);
	mul_add_c(a[1], b[0], c2, c3, c1);
	r[1] = c2;
	c2 = 0;
	mul_add_c(a[2], b[0], c3, c1, c2);
	mul_add_c(a[1], b[1], c3, c1, c2);
	mul_add_c(a[0], b[2], c3, c1, c2);
	r[2] = c3;
	c3 = 0;
	mul_add_c(a[0], b[3], c1, c2, c3);
	mul_add_c(a[1], b[2], c1, c2, c3);
	mul_add_c(a[2], b[1], c1, c2, c3);
	mul_add_c(a[3], b[0], c1, c2, c3);
	r[3] = c1;
	c1 = 0;
	mul_add_c(a[3], b[1], c2, c3, c1);
	mul_add_c(a[2], b[2], c2, c3, c1);
	mul_add_c(a[1], b[3], c2, c3, c1);
	r[4] = c2;
	c2 = 0;
	mul_add_c(a[2], b[3], c3, c1, c2);
	mul_add_c(a[3], b[2], c3, c1, c2);
	r[5] = c3;
	c3 = 0;
	mul_add_c(a[3], b[3], c1, c2, c3);
	r[6] = c1;
	r[7] = c2;
}
#endif /*BN_RECURSION */
static void bn_sqr_comba8(BN_ULONG *r, const BN_ULONG *a)
{
	BN_ULONG bl, bh;
	BN_ULONG t1, t2;
	BN_ULONG c1, c2, c3;

	c1 = 0;
	c2 = 0;
	c3 = 0;
	sqr_add_c(a, 0, c1, c2, c3);
	r[0] = c1;
	c1 = 0;
	sqr_add_c2(a, 1, 0, c2, c3, c1);
	r[1] = c2;
	c2 = 0;
	sqr_add_c(a, 1, c3, c1, c2);
	sqr_add_c2(a, 2, 0, c3, c1, c2);
	r[2] = c3;
	c3 = 0;
	sqr_add_c2(a, 3, 0, c1, c2, c3);
	sqr_add_c2(a, 2, 1, c1, c2, c3);
	r[3] = c1;
	c1 = 0;
	sqr_add_c(a, 2, c2, c3, c1);
	sqr_add_c2(a, 3, 1, c2, c3, c1);
	sqr_add_c2(a, 4, 0, c2, c3, c1);
	r[4] = c2;
	c2 = 0;
	sqr_add_c2(a, 5, 0, c3, c1, c2);
	sqr_add_c2(a, 4, 1, c3, c1, c2);
	sqr_add_c2(a, 3, 2, c3, c1, c2);
	r[5] = c3;
	c3 = 0;
	sqr_add_c(a, 3, c1, c2, c3);
	sqr_add_c2(a, 4, 2, c1, c2, c3);
	sqr_add_c2(a, 5, 1, c1, c2, c3);
	sqr_add_c2(a, 6, 0, c1, c2, c3);
	r[6] = c1;
	c1 = 0;
	sqr_add_c2(a, 7, 0, c2, c3, c1);
	sqr_add_c2(a, 6, 1, c2, c3, c1);
	sqr_add_c2(a, 5, 2, c2, c3, c1);
	sqr_add_c2(a, 4, 3, c2, c3, c1);
	r[7] = c2;
	c2 = 0;
	sqr_add_c(a, 4, c3, c1, c2);
	sqr_add_c2(a, 5, 3, c3, c1, c2);
	sqr_add_c2(a, 6, 2, c3, c1, c2);
	sqr_add_c2(a, 7, 1, c3, c1, c2);
	r[8] = c3;
	c3 = 0;
	sqr_add_c2(a, 7, 2, c1, c2, c3);
	sqr_add_c2(a, 6, 3, c1, c2, c3);
	sqr_add_c2(a, 5, 4, c1, c2, c3);
	r[9] = c1;
	c1 = 0;
	sqr_add_c(a, 5, c2, c3, c1);
	sqr_add_c2(a, 6, 4, c2, c3, c1);
	sqr_add_c2(a, 7, 3, c2, c3, c1);
	r[10] = c2;
	c2 = 0;
	sqr_add_c2(a, 7, 4, c3, c1, c2);
	sqr_add_c2(a, 6, 5, c3, c1, c2);
	r[11] = c3;
	c3 = 0;
	sqr_add_c(a, 6, c1, c2, c3);
	sqr_add_c2(a, 7, 5, c1, c2, c3);
	r[12] = c1;
	c1 = 0;
	sqr_add_c2(a, 7, 6, c2, c3, c1);
	r[13] = c2;
	c2 = 0;
	sqr_add_c(a, 7, c3, c1, c2);
	r[14] = c3;
	r[15] = c1;
}

static void bn_sqr_comba4(BN_ULONG *r, const BN_ULONG *a)
{
	BN_ULONG bl, bh;
	BN_ULONG t1, t2;
	BN_ULONG c1, c2, c3;

	c1 = 0;
	c2 = 0;
	c3 = 0;
	sqr_add_c(a, 0, c1, c2, c3);
	r[0] = c1;
	c1 = 0;
	sqr_add_c2(a, 1, 0, c2, c3, c1);
	r[1] = c2;
	c2 = 0;
	sqr_add_c(a, 1, c3, c1, c2);
	sqr_add_c2(a, 2, 0, c3, c1, c2);
	r[2] = c3;
	c3 = 0;
	sqr_add_c2(a, 3, 0, c1, c2, c3);
	sqr_add_c2(a, 2, 1, c1, c2, c3);
	r[3] = c1;
	c1 = 0;
	sqr_add_c(a, 2, c2, c3, c1);
	sqr_add_c2(a, 3, 1, c2, c3, c1);
	r[4] = c2;
	c2 = 0;
	sqr_add_c2(a, 3, 2, c3, c1, c2);
	r[5] = c3;
	c3 = 0;
	sqr_add_c(a, 3, c1, c2, c3);
	r[6] = c1;
	r[7] = c2;
}
#endif /* !BN_MUL_COMBA */
/*bn_asm END*/

/*bn_mul START*/
/* Here follows specialised variants of bn_add_words() and
   bn_sub_words().  They have the property performing operations on
   arrays of different sizes.  The sizes of those arrays is expressed through
   cl, which is the common length ( basicall, min(len(a),len(b)) ), and dl,
   which is the delta between the two lengths, calculated as len(a)-len(b).
   All lengths are the number of BN_ULONGs...  For the operations that require
   a result array as parameter, it must have the length cl+abs(dl).
   These functions should probably end up in bn_asm.c as soon as there are
   assembler counterparts for the systems that use assembler files.  */
#ifdef BN_RECURSION
static BN_ULONG bn_sub_part_words(BN_ULONG *r,
								  const BN_ULONG *a, const BN_ULONG *b,
								  int cl, int dl)
{
	BN_ULONG c, t;

	c = bn_sub_words(r, a, b, cl);

	if (dl == 0)
		return c;

	r += cl;
	a += cl;
	b += cl;

	if (dl < 0) {
		for (;;) {
			t = b[0];
			r[0] = (0 - t - c)&BN_MASK2;

			if (t != 0)
				c = 1;

			if (++dl >= 0)
				break;

			t = b[1];
			r[1] = (0 - t - c)&BN_MASK2;

			if (t != 0)
				c = 1;

			if (++dl >= 0)
				break;

			t = b[2];
			r[2] = (0 - t - c)&BN_MASK2;

			if (t != 0)
				c = 1;

			if (++dl >= 0)
				break;

			t = b[3];
			r[3] = (0 - t - c)&BN_MASK2;

			if (t != 0)
				c = 1;

			if (++dl >= 0)
				break;

			b += 4;
			r += 4;
		}
	} else {
		int save_dl = dl;

		while (c) {
			t = a[0];
			r[0] = (t - c)&BN_MASK2;

			if (t != 0)
				c = 0;

			if (--dl <= 0)
				break;

			t = a[1];
			r[1] = (t - c)&BN_MASK2;

			if (t != 0)
				c = 0;

			if (--dl <= 0)
				break;

			t = a[2];
			r[2] = (t - c)&BN_MASK2;

			if (t != 0)
				c = 0;

			if (--dl <= 0)
				break;

			t = a[3];
			r[3] = (t - c)&BN_MASK2;

			if (t != 0)
				c = 0;

			if (--dl <= 0)
				break;

			save_dl = dl;
			a += 4;
			r += 4;
		}

		if (dl > 0) {
			if (save_dl > dl) {
				switch (save_dl - dl) {
				case 1:
					r[1] = a[1];

					if (--dl <= 0)
						break;

				case 2:
					r[2] = a[2];

					if (--dl <= 0)
						break;

				case 3:
					r[3] = a[3];

					if (--dl <= 0)
						break;
				}

				a += 4;
				r += 4;
			}
		}

		if (dl > 0) {
			for (;;) {
				r[0] = a[0];

				if (--dl <= 0)
					break;

				r[1] = a[1];

				if (--dl <= 0)
					break;

				r[2] = a[2];

				if (--dl <= 0)
					break;

				r[3] = a[3];

				if (--dl <= 0)
					break;

				a += 4;
				r += 4;
			}
		}
	}

	return c;
}

/* Karatsuba recursive multiplication algorithm
 * (cf. Knuth, The Art of Computer Programming, Vol. 2) */

/* r is 2*n2 words in size,
 * a and b are both n2 words in size.
 * n2 must be a power of 2.
 * We multiply and return the result.
 * t must be 2*n2 words in size
 * We calculate
 * a[0]*b[0]
 * a[0]*b[0]+a[1]*b[1]+(a[0]-a[1])*(b[1]-b[0])
 * a[1]*b[1]
 */
static void bn_mul_recursive(BN_ULONG *r, BN_ULONG *a, BN_ULONG *b, int n2,
							 int dna, int dnb, BN_ULONG *t)
{
	int n = n2 / 2, c1, c2;
	int tna = n + dna, tnb = n + dnb;
	unsigned int neg, zero;
	BN_ULONG ln, lo, *p;
# ifdef BN_MUL_COMBA

	/* Only call bn_mul_comba 8 if n2 == 8 and the
	 * two arrays are complete [steve]
	 */
	if (n2 == 8 && dna == 0 && dnb == 0) {
		bn_mul_comba8(r, a, b);
		return;
	}

# endif /* BN_MUL_COMBA */

	/* Else do normal multiply */
	if (n2 < BN_MUL_RECURSIVE_SIZE_NORMAL) {
		bn_mul_normal(r, a, n2 + dna, b, n2 + dnb);

		if ((dna + dnb) < 0)
			memset(&r[2 * n2 + dna + dnb], 0,
				   sizeof(BN_ULONG) * -(dna + dnb));

		return;
	}

	/* r=(a[0]-a[1])*(b[1]-b[0]) */
	c1 = bn_cmp_part_words(a, &(a[n]), tna, n - tna);
	c2 = bn_cmp_part_words(&(b[n]), b, tnb, tnb - n);
	zero = neg = 0;

	switch (c1 * 3 + c2) {
	case -4:
		bn_sub_part_words(t,      &(a[n]), a,      tna, tna - n); /* - */
		bn_sub_part_words(&(t[n]), b,      &(b[n]), tnb, n - tnb); /* - */
		break;

	case -3:
		zero = 1;
		break;

	case -2:
		bn_sub_part_words(t,      &(a[n]), a,      tna, tna - n); /* - */
		bn_sub_part_words(&(t[n]), &(b[n]), b,      tnb, tnb - n); /* + */
		neg = 1;
		break;

	case -1:
	case 0:
	case 1:
		zero = 1;
		break;

	case 2:
		bn_sub_part_words(t,      a,      &(a[n]), tna, n - tna); /* + */
		bn_sub_part_words(&(t[n]), b,      &(b[n]), tnb, n - tnb); /* - */
		neg = 1;
		break;

	case 3:
		zero = 1;
		break;

	case 4:
		bn_sub_part_words(t,      a,      &(a[n]), tna, n - tna);
		bn_sub_part_words(&(t[n]), &(b[n]), b,      tnb, tnb - n);
		break;
	}

# ifdef BN_MUL_COMBA

	if (n == 4 && dna == 0 && dnb == 0) /* XXX: bn_mul_comba4 could take
					       extra args to do this well */
	{
		if (!zero)
			bn_mul_comba4(&(t[n2]), t, &(t[n]));
		else
			memset(&(t[n2]), 0, 8 * sizeof(BN_ULONG));

		bn_mul_comba4(r, a, b);
		bn_mul_comba4(&(r[n2]), &(a[n]), &(b[n]));
	} else if (n == 8 && dna == 0 && dnb == 0) /* XXX: bn_mul_comba8 could
						    take extra args to do this
						    well */
	{
		if (!zero)
			bn_mul_comba8(&(t[n2]), t, &(t[n]));
		else
			memset(&(t[n2]), 0, 16 * sizeof(BN_ULONG));

		bn_mul_comba8(r, a, b);
		bn_mul_comba8(&(r[n2]), &(a[n]), &(b[n]));
	} else
# endif /* BN_MUL_COMBA */
	{
		p = &(t[n2 * 2]);

		if (!zero)
			bn_mul_recursive(&(t[n2]), t, &(t[n]), n, 0, 0, p);
		else
			memset(&(t[n2]), 0, n2 * sizeof(BN_ULONG));

		bn_mul_recursive(r, a, b, n, 0, 0, p);
		bn_mul_recursive(&(r[n2]), &(a[n]), &(b[n]), n, dna, dnb, p);
	}

	/* t[32] holds (a[0]-a[1])*(b[1]-b[0]), c1 is the sign
	 * r[10] holds (a[0]*b[0])
	 * r[32] holds (b[1]*b[1])
	 */
	c1 = (int)(bn_add_words(t, r, &(r[n2]), n2));

	if (neg) /* if t[32] is negative */
		c1 -= (int)(bn_sub_words(&(t[n2]), t, &(t[n2]), n2));
	else {
		/* Might have a carry */
		c1 += (int)(bn_add_words(&(t[n2]), &(t[n2]), t, n2));
	}

	/* t[32] holds (a[0]-a[1])*(b[1]-b[0])+(a[0]*b[0])+(a[1]*b[1])
	 * r[10] holds (a[0]*b[0])
	 * r[32] holds (b[1]*b[1])
	 * c1 holds the carry bits
	 */
	c1 += (int)(bn_add_words(&(r[n]), &(r[n]), &(t[n2]), n2));

	if (c1) {
		p = &(r[n + n2]);
		lo = *p;
		ln = (lo + c1)&BN_MASK2;
		*p = ln;

		/* The overflow will stop before we over write
		 * words we should not overwrite */
		if (ln < (BN_ULONG)c1) {
			do	{
				p++;
				lo = *p;
				ln = (lo + 1)&BN_MASK2;
				*p = ln;
			} while (ln == 0);
		}
	}
}

/* n+tn is the word length
 * t needs to be n*4 is size, as does r */
static void bn_mul_part_recursive(BN_ULONG *r, BN_ULONG *a, BN_ULONG *b, int n,
								  int tna, int tnb, BN_ULONG *t)
{
	int i, j, n2 = n * 2;
	int c1, c2, neg, zero;
	BN_ULONG ln, lo, *p;

	if (n < 8) {
		bn_mul_normal(r, a, n + tna, b, n + tnb);
		return;
	}

	/* r=(a[0]-a[1])*(b[1]-b[0]) */
	c1 = bn_cmp_part_words(a, &(a[n]), tna, n - tna);
	c2 = bn_cmp_part_words(&(b[n]), b, tnb, tnb - n);
	zero = neg = 0;

	switch (c1 * 3 + c2) {
	case -4:
		bn_sub_part_words(t,      &(a[n]), a,      tna, tna - n); /* - */
		bn_sub_part_words(&(t[n]), b,      &(b[n]), tnb, n - tnb); /* - */
		break;

	case -3:
		zero = 1;

	/* break; */
	case -2:
		bn_sub_part_words(t,      &(a[n]), a,      tna, tna - n); /* - */
		bn_sub_part_words(&(t[n]), &(b[n]), b,      tnb, tnb - n); /* + */
		neg = 1;
		break;

	case -1:
	case 0:
	case 1:
		zero = 1;

	/* break; */
	case 2:
		bn_sub_part_words(t,      a,      &(a[n]), tna, n - tna); /* + */
		bn_sub_part_words(&(t[n]), b,      &(b[n]), tnb, n - tnb); /* - */
		neg = 1;
		break;

	case 3:
		zero = 1;

	/* break; */
	case 4:
		bn_sub_part_words(t,      a,      &(a[n]), tna, n - tna);
		bn_sub_part_words(&(t[n]), &(b[n]), b,      tnb, tnb - n);
		break;
	}

	if (n == 8) {
		bn_mul_comba8(&(t[n2]), t, &(t[n]));
		bn_mul_comba8(r, a, b);
		bn_mul_normal(&(r[n2]), &(a[n]), tna, &(b[n]), tnb);
		memset(&(r[n2 + tna + tnb]), 0, sizeof(BN_ULONG) * (n2 - tna - tnb));
	} else {
		p = &(t[n2 * 2]);
		bn_mul_recursive(&(t[n2]), t, &(t[n]), n, 0, 0, p);
		bn_mul_recursive(r, a, b, n, 0, 0, p);
		i = n / 2;

		/* If there is only a bottom half to the number,
		 * just do it */
		if (tna > tnb)
			j = tna - i;
		else
			j = tnb - i;

		if (j == 0) {
			bn_mul_recursive(&(r[n2]), &(a[n]), &(b[n]),
							 i, tna - i, tnb - i, p);
			memset(&(r[n2 + i * 2]), 0, sizeof(BN_ULONG) * (n2 - i * 2));
		} else if (j > 0) { /* eg, n == 16, i == 8 and tn == 11 */
			bn_mul_part_recursive(&(r[n2]), &(a[n]), &(b[n]),
								  i, tna - i, tnb - i, p);
			memset(&(r[n2 + tna + tnb]), 0,
				   sizeof(BN_ULONG) * (n2 - tna - tnb));
		} else { /* (j < 0) eg, n == 16, i == 8 and tn == 5 */
			memset(&(r[n2]), 0, sizeof(BN_ULONG)*n2);

			if (tna < BN_MUL_RECURSIVE_SIZE_NORMAL
				&& tnb < BN_MUL_RECURSIVE_SIZE_NORMAL)
				bn_mul_normal(&(r[n2]), &(a[n]), tna, &(b[n]), tnb);
			else {
				for (;;) {
					i /= 2;

					if (i < tna && i < tnb) {
						bn_mul_part_recursive(&(r[n2]),
											  &(a[n]), &(b[n]),
											  i, tna - i, tnb - i, p);
						break;
					} else if (i <= tna && i <= tnb) {
						bn_mul_recursive(&(r[n2]),
										 &(a[n]), &(b[n]),
										 i, tna - i, tnb - i, p);
						break;
					}
				}
			}
		}
	}

	/* t[32] holds (a[0]-a[1])*(b[1]-b[0]), c1 is the sign
	 * r[10] holds (a[0]*b[0])
	 * r[32] holds (b[1]*b[1])
	 */
	c1 = (int)(bn_add_words(t, r, &(r[n2]), n2));

	if (neg) /* if t[32] is negative */
		c1 -= (int)(bn_sub_words(&(t[n2]), t, &(t[n2]), n2));
	else {
		/* Might have a carry */
		c1 += (int)(bn_add_words(&(t[n2]), &(t[n2]), t, n2));
	}

	/* t[32] holds (a[0]-a[1])*(b[1]-b[0])+(a[0]*b[0])+(a[1]*b[1])
	 * r[10] holds (a[0]*b[0])
	 * r[32] holds (b[1]*b[1])
	 * c1 holds the carry bits
	 */
	c1 += (int)(bn_add_words(&(r[n]), &(r[n]), &(t[n2]), n2));

	if (c1) {
		p = &(r[n + n2]);
		lo = *p;
		ln = (lo + c1)&BN_MASK2;
		*p = ln;

		/* The overflow will stop before we over write
		 * words we should not overwrite */
		if (ln < (BN_ULONG)c1) {
			do	{
				p++;
				lo = *p;
				ln = (lo + 1)&BN_MASK2;
				*p = ln;
			} while (ln == 0);
		}
	}
}
#endif /* BN_RECURSION */

static int BN_mul(struct bignum_st *r, const struct bignum_st *a, const struct bignum_st *b, struct bignum_pool *ctx)
{
	int release = 0;
	int ret = 0;
	int top, al, bl;
	struct bignum_st *rr;
#if defined(BN_MUL_COMBA) || defined(BN_RECURSION)
	int i;
#endif
#ifdef BN_RECURSION
	struct bignum_st *t = NULL;
	int j = 0, k;
#endif
	al = a->top;
	bl = b->top;

	if ((al == 0) || (bl == 0)) {
		BN_zero(r);
		return 1;
	}

	top = al + bl;

	if ((r == a) || (r == b)) {
		rr = BN_POOL_get(ctx);

		if (rr == NULL)
			goto err;

		release++;
	} else
		rr = r;

#if defined(BN_MUL_COMBA) || defined(BN_RECURSION)
	i = al - bl;
#endif
#ifdef BN_MUL_COMBA

	if (i == 0) {
		if (al == 8) {
			if (bn_wexpand(rr, 16) == NULL)
				goto err;

			rr->top = 16;
			bn_mul_comba8(rr->d, a->d, b->d);
			goto end;
		}
	}

#endif /* BN_MUL_COMBA */
#ifdef BN_RECURSION

	if ((al >= BN_MULL_SIZE_NORMAL) && (bl >= BN_MULL_SIZE_NORMAL)) {
		if (i >= -1 && i <= 1) {
			int sav_j = 0;

			/* Find out the power of two lower or equal
			   to the longest of the two numbers */
			if (i >= 0)
				j = BN_num_bits_word((BN_ULONG)al);

			if (i == -1)
				j = BN_num_bits_word((BN_ULONG)bl);

			sav_j = j;
			j = 1 << (j - 1);
			k = j + j;
			t = BN_POOL_get(ctx);
			release++;

			if (al > j || bl > j) {
				bn_wexpand(t, k * 4);
				bn_wexpand(rr, k * 4);
				bn_mul_part_recursive(rr->d, a->d, b->d,
									  j, al - j, bl - j, t->d);
			} else {	/* al <= j || bl <= j */
				bn_wexpand(t, k * 2);
				bn_wexpand(rr, k * 2);
				bn_mul_recursive(rr->d, a->d, b->d,
								 j, al - j, bl - j, t->d);
			}

			rr->top = top;
			goto end;
		}
	}

#endif /* BN_RECURSION */

	if (bn_wexpand(rr, top) == NULL)
		goto err;

	rr->top = top;
	bn_mul_normal(rr->d, a->d, al, b->d, bl);
#if defined(BN_MUL_COMBA) || defined(BN_RECURSION)
end:
#endif
	bn_correct_top(rr);

	if (r != rr)
		BN_copy(r, rr);

	ret = 1;
err:
	BN_POOL_release(ctx, release);
	return ret;
}

static void bn_mul_normal(BN_ULONG *r, BN_ULONG *a, int na, BN_ULONG *b, int nb)
{
	BN_ULONG *rr;

	if (na < nb) {
		int itmp;
		BN_ULONG *ltmp;

		itmp = na;
		na = nb;
		nb = itmp;
		ltmp = a;
		a = b;
		b = ltmp;
	}

	rr = &(r[na]);

	if (nb <= 0) {
		(void)bn_mul_words(r, a, na, 0);
		return;
	} else
		rr[0] = bn_mul_words(r, a, na, b[0]);

	for (;;) {
		if (--nb <= 0)
			return;

		rr[1] = bn_mul_add_words(&(r[1]), a, na, b[1]);

		if (--nb <= 0)
			return;

		rr[2] = bn_mul_add_words(&(r[2]), a, na, b[2]);

		if (--nb <= 0)
			return;

		rr[3] = bn_mul_add_words(&(r[3]), a, na, b[3]);

		if (--nb <= 0)
			return;

		rr[4] = bn_mul_add_words(&(r[4]), a, na, b[4]);
		rr += 4;
		r += 4;
		b += 4;
	}
}
/*bn_mul END*/

/* tmp must have 2*n words */
static void bn_sqr_normal(BN_ULONG *r, const BN_ULONG *a, int n, BN_ULONG *tmp)
{
	int i, j, max;
	const BN_ULONG *ap;
	BN_ULONG *rp;

	max = n * 2;
	ap = a;
	rp = r;
	rp[0] = rp[max - 1] = 0;
	rp++;
	j = n;

	if (--j > 0) {
		ap++;
		rp[j] = bn_mul_words(rp, ap, j, ap[-1]);
		rp += 2;
	}

	for (i = n - 2; i > 0; i--) {
		j--;
		ap++;
		rp[j] = bn_mul_add_words(rp, ap, j, ap[-1]);
		rp += 2;
	}

	bn_add_words(r, r, r, max);
	/* There will not be a carry */
	bn_sqr_words(tmp, a, n);
	bn_add_words(r, r, tmp, max);
}

#ifdef BN_RECURSION
/* r is 2*n words in size,
 * a and b are both n words in size.    (There's not actually a 'b' here ...)
 * n must be a power of 2.
 * We multiply and return the result.
 * t must be 2*n words in size
 * We calculate
 * a[0]*b[0]
 * a[0]*b[0]+a[1]*b[1]+(a[0]-a[1])*(b[1]-b[0])
 * a[1]*b[1]
 */
static void bn_sqr_recursive(BN_ULONG *r, const BN_ULONG *a, int n2, BN_ULONG *t)
{
	int n = n2 / 2;
	int zero, c1;
	BN_ULONG ln, lo, *p;

	if (n2 == 4) {
#ifndef BN_SQR_COMBA
		bn_sqr_normal(r, a, 4, t);
#else
		bn_sqr_comba4(r, a);
#endif
		return;
	} else if (n2 == 8) {
#ifndef BN_SQR_COMBA
		bn_sqr_normal(r, a, 8, t);
#else
		bn_sqr_comba8(r, a);
#endif
		return;
	}

	if (n2 < BN_SQR_RECURSIVE_SIZE_NORMAL) {
		bn_sqr_normal(r, a, n2, t);
		return;
	}

	/* r=(a[0]-a[1])*(a[1]-a[0]) */
	c1 = bn_cmp_words(a, &(a[n]), n);
	zero = 0;

	if (c1 > 0)
		bn_sub_words(t, a, &(a[n]), n);
	else if (c1 < 0)
		bn_sub_words(t, &(a[n]), a, n);
	else
		zero = 1;

	/* The result will always be negative unless it is zero */
	p = &(t[n2 * 2]);

	if (!zero)
		bn_sqr_recursive(&(t[n2]), t, n, p);
	else
		memset(&(t[n2]), 0, n2 * sizeof(BN_ULONG));

	bn_sqr_recursive(r, a, n, p);
	bn_sqr_recursive(&(r[n2]), &(a[n]), n, p);
	/* t[32] holds (a[0]-a[1])*(a[1]-a[0]), it is negative or zero
	 * r[10] holds (a[0]*b[0])
	 * r[32] holds (b[1]*b[1])
	 */
	c1 = (int)(bn_add_words(t, r, &(r[n2]), n2));
	/* t[32] is negative */
	c1 -= (int)(bn_sub_words(&(t[n2]), t, &(t[n2]), n2));
	/* t[32] holds (a[0]-a[1])*(a[1]-a[0])+(a[0]*a[0])+(a[1]*a[1])
	 * r[10] holds (a[0]*a[0])
	 * r[32] holds (a[1]*a[1])
	 * c1 holds the carry bits
	 */
	c1 += (int)(bn_add_words(&(r[n]), &(r[n]), &(t[n2]), n2));

	if (c1) {
		p = &(r[n + n2]);
		lo = *p;
		ln = (lo + c1)&BN_MASK2;
		*p = ln;

		/* The overflow will stop before we over write
		 * words we should not overwrite */
		if (ln < (BN_ULONG)c1) {
			do	{
				p++;
				lo = *p;
				ln = (lo + 1)&BN_MASK2;
				*p = ln;
			} while (ln == 0);
		}
	}
}
#endif /*BN_RECURSION */

/* unsigned subtraction of b from a, a must be larger than b. */
static int BN_usub(struct bignum_st *r, const struct bignum_st *a, const struct bignum_st *b)
{
	int max, min, dif;
	register BN_ULONG t1, t2, *ap, *bp, *rp;
	int i, carry;

	max = a->top;
	min = b->top;
	dif = max - min;

	if (dif < 0)	/* hmm... should not be happening */
		return 0;

	if (bn_wexpand(r, max) == NULL)
		return 0;

	ap = a->d;
	bp = b->d;
	rp = r->d;
	carry = 0;

	for (i = min; i != 0; i--) {
		t1 = *(ap++);
		t2 = *(bp++);

		if (carry) {
			carry = (t1 <= t2);
			t1 = (t1 - t2 - 1)&BN_MASK2;
		} else {
			carry = (t1 < t2);
			t1 = (t1 - t2)&BN_MASK2;
		}

		*(rp++) = t1 & BN_MASK2;
	}

	if (carry) { /* subtracted */
		if (!dif)
			/* error: a < b */
			return 0;

		while (dif) {
			dif--;
			t1 = *(ap++);
			t2 = (t1 - 1)&BN_MASK2;
			*(rp++) = t2;

			if (t1)
				break;
		}
	}

	if (rp != ap) {
		for (;;) {
			if (!dif--)
				break;

			rp[0] = ap[0];

			if (!dif--)
				break;

			rp[1] = ap[1];

			if (!dif--)
				break;

			rp[2] = ap[2];

			if (!dif--)
				break;

			rp[3] = ap[3];
			rp += 4;
			ap += 4;
		}
	}

	r->top = max;
	bn_correct_top(r);
	return 1;
}

static int BN_from_montgomery(struct bignum_st *ret, const struct bignum_st *a, struct bn_mont_ctx_st *mont, struct bignum_pool *ctx)
{
	int retn = 0;
	struct bignum_st *n, *r;
	BN_ULONG *ap, *np, *rp, n0, v, *nrp;
	int al, nl, max, i, x, ri;

	r = BN_POOL_get(ctx);

	if (r == NULL)
		goto err;

	if (!BN_copy(r, a))
		goto err;

	n = &(mont->N);
	ap = a->d;
	/* mont->ri is the size of mont->N in bits (rounded up
	   to the word size) */
	al = ri = mont->ri / BN_BITS2;
	nl = n->top;

	if ((al == 0) || (nl == 0)) {
		r->top = 0;
		return 1;
	}

	max = (nl + al + 1); /* allow for overflow (no?) XXX */

	if (bn_wexpand(r, max) == NULL)
		goto err;

	if (bn_wexpand(ret, max) == NULL)
		goto err;

	np = n->d;
	rp = r->d;
	nrp = &(r->d[nl]);

	/* clear the top words of T */
	for (i = r->top; i < max; i++) /* memset? XXX */
		r->d[i] = 0;

	r->top = max;
	n0 = mont->n0;

	for (i = 0; i < nl; i++) {
		v = bn_mul_add_words(rp, np, nl, (rp[0] * n0)&BN_MASK2);
		nrp++;
		rp++;
		nrp[-1] += v;

		if (((nrp[-1]) & BN_MASK2) >= v)
			continue;
		else {
			if (((++nrp[0]) & BN_MASK2) != 0)
				continue;

			if (((++nrp[1]) & BN_MASK2) != 0)
				continue;

			for (x = 2; (((++nrp[x])&BN_MASK2) == 0); x++)
				;
		}
	}

	bn_correct_top(r);
	/* mont->ri will be a multiple of the word size */
	x = ri;
	rp = ret->d;
	ap = &(r->d[x]);

	if (r->top < x)
		al = 0;
	else
		al = r->top - x;

	ret->top = al;
	al -= 4;

	for (i = 0; i < al; i += 4) {
		BN_ULONG t1, t2, t3, t4;

		t1 = ap[i + 0];
		t2 = ap[i + 1];
		t3 = ap[i + 2];
		t4 = ap[i + 3];
		rp[i + 0] = t1;
		rp[i + 1] = t2;
		rp[i + 2] = t3;
		rp[i + 3] = t4;
	}

	al += 4;

	for (; i < al; i++)
		rp[i] = ap[i];

	if (BN_ucmp(ret, &(mont->N)) >= 0) {
		if (!BN_usub(ret, ret, &(mont->N)))
			goto err;
	}

	retn = 1;
err:
	BN_POOL_release(ctx, 1);
	return retn;
}

/*SQR*/
/* r must not be a */
/* I've just gone over this and it is now %20 faster on x86 - eay - 27 Jun 96 */
static int BN_sqr(struct bignum_st *r, const struct bignum_st *a, struct bignum_pool *ctx)
{
	int release = 0;
	int max, al;
	int ret = 0;
	struct bignum_st *tmp, *rr;

	al = a->top;

	if (al <= 0) {
		r->top = 0;
		return 1;
	}

	if (a != r)
		rr = r;
	else {
		rr = BN_POOL_get(ctx);
		release++;
	}

	tmp = BN_POOL_get(ctx);
	release++;

	if (!rr || !tmp)
		goto err;

	max = 2 * al; /* Non-zero (from above) */

	if (bn_wexpand(rr, max) == NULL)
		goto err;

	if (al == 4) {
#ifndef BN_SQR_COMBA
		BN_ULONG t[8];

		bn_sqr_normal(rr->d, a->d, 4, t);
#else
		bn_sqr_comba4(rr->d, a->d);
#endif
	} else if (al == 8) {
#ifndef BN_SQR_COMBA
		BN_ULONG t[16];

		bn_sqr_normal(rr->d, a->d, 8, t);
#else
		bn_sqr_comba8(rr->d, a->d);
#endif
	} else {
#if defined(BN_RECURSION)

		if (al < BN_SQR_RECURSIVE_SIZE_NORMAL) {
			BN_ULONG t[BN_SQR_RECURSIVE_SIZE_NORMAL * 2];

			bn_sqr_normal(rr->d, a->d, al, t);
		} else {
			int j, k;

			j = BN_num_bits_word((BN_ULONG)al);
			j = 1 << (j - 1);
			k = j + j;

			if (al == j) {
				if (bn_wexpand(tmp, k * 2) == NULL)
					goto err;

				bn_sqr_recursive(rr->d, a->d, al, tmp->d);
			} else {
				if (bn_wexpand(tmp, max) == NULL)
					goto err;

				bn_sqr_normal(rr->d, a->d, al, tmp->d);
			}
		}

#else

		if (bn_wexpand(tmp, max) == NULL)
			goto err;

		bn_sqr_normal(rr->d, a->d, al, tmp->d);
#endif
	}

	/* If the most-significant half of the top word of 'a' is zero, then
	 * the square of 'a' will max-1 words. */
	if (a->d[al - 1] == (a->d[al - 1] & BN_MASK2l))
		rr->top = max - 1;
	else
		rr->top = max;

	if (rr != r)
		BN_copy(r, rr);

	ret = 1;
err:
	BN_POOL_release(ctx, release);
	return ret;
}

static int BN_mod_mul_montgomery(struct bignum_st *r, const struct bignum_st *a, const struct bignum_st *b, struct bn_mont_ctx_st *mont, struct bignum_pool *ctx)
{
	struct bignum_st *tmp;
	int ret = 0;

	tmp = BN_POOL_get(ctx);

	if (tmp == NULL)
		goto err;

	if (BN_ucmp(a, b) == 0) {
		if (!BN_sqr(tmp, a, ctx))
			goto err;
	} else {
		if (!BN_mul(tmp, a, b, ctx))
			goto err;
	}

	/* reduce from aRR to aR */
	if (!BN_from_montgomery(r, tmp, mont, ctx))
		goto err;

	ret = 1;
err:
	BN_POOL_release(ctx, 1);
	return ret;
}

/* unsigned add of b to a */
static int BN_uadd(struct bignum_st *r, const struct bignum_st *a, const struct bignum_st *b)
{
	int max, min, dif;
	BN_ULONG *ap, *bp, *rp, carry, t1, t2;
	const struct bignum_st *tmp;

	if (a->top < b->top) {
		tmp = a;
		a = b;
		b = tmp;
	}

	max = a->top;
	min = b->top;
	dif = max - min;

	if (bn_wexpand(r, max + 1) == NULL)
		return 0;

	r->top = max;
	ap = a->d;
	bp = b->d;
	rp = r->d;
	carry = bn_add_words(rp, ap, bp, min);
	rp += min;
	ap += min;
	bp += min;

	if (carry) {
		while (dif) {
			dif--;
			t1 = *(ap++);
			t2 = (t1 + 1) & BN_MASK2;
			*(rp++) = t2;

			if (t2) {
				carry = 0;
				break;
			}
		}

		if (carry) {
			/* carry != 0 => dif == 0 */
			*rp = 1;
			r->top++;
		}
	}

	if (dif && rp != ap)
		while (dif--)
			/* copy remaining words if ap != rp */
			*(rp++) = *(ap++);

	return 1;
}

static int BN_sub(struct bignum_st *r, const struct bignum_st *a, const struct bignum_st *b)
{
	int max;
	/* We are actually doing a - b :-) */
	max = (a->top > b->top) ? a->top : b->top;

	if (bn_wexpand(r, max) == NULL)
		return 0;

	if (BN_ucmp(a, b) < 0) {
		if (!BN_usub(r, b, a))
			return 0;

		DHPRINT("----\n");
	} else {
		if (!BN_usub(r, a, b))
			return 0;
	}

	return 1;
}

static int BN_mul_word(struct bignum_st *a, BN_ULONG w)
{
	BN_ULONG ll;

	w &= BN_MASK2;

	if (a->top) {
		if (w == 0)
			BN_zero(a);
		else {
			ll = bn_mul_words(a->d, a->d, a->top, w);

			if (ll) {
				if (bn_wexpand(a, a->top + 1) == NULL)
					return 0;

				a->d[a->top++] = ll;
			}
		}
	}

	return 1;
}

/* solves ax == 1 (mod n) */
static struct bignum_st *BN_mod_inverse(struct bignum_st *in,
										const struct bignum_st *a, const struct bignum_st *n, struct bignum_pool *ctx)
{
	struct bignum_st *A, *B, *X, *Y, *M, *D, *T, *R = NULL;
	struct bignum_st *ret = NULL;
	int sign;

	A = BN_POOL_get(ctx);
	B = BN_POOL_get(ctx);
	X = BN_POOL_get(ctx);
	D = BN_POOL_get(ctx);
	M = BN_POOL_get(ctx);
	Y = BN_POOL_get(ctx);
	T = BN_POOL_get(ctx);

	if (T == NULL)
		goto err;

	if (in == NULL)
		R = BN_new();
	else
		R = in;

	if (R == NULL)
		goto err;

	BN_one(X);
	BN_zero(Y);

	if (BN_copy(B, a) == NULL)
		goto err;

	if (BN_copy(A, n) == NULL)
		goto err;

	if (BN_ucmp(B, A) >= 0) {
		if (!(BN_mod(B, B, A, ctx)))
			goto err;
	}

	sign = -1;
	/* From  B = a mod |n|,  A = |n|  it follows that
	 *
	 *      0 <= B < A,
	 *     -sign*X*a  ==  B   (mod |n|),
	 *      sign*Y*a  ==  A   (mod |n|).
	 */

	if (BN_is_odd(n) && (BN_num_bits(n) <= (BN_BITS <= 32 ? 450 : 2048))) {
		/* Binary inversion algorithm; requires odd modulus.
		 * This is faster than the general algorithm if the modulus
		 * is sufficiently small (about 400 .. 500 bits on 32-bit
		 * sytems, but much more on 64-bit systems) */
		int shift;

		while (!BN_is_zero(B)) {
			/*
			 *      0 < B < |n|,
			 *      0 < A <= |n|,
			 * (1) -sign*X*a  ==  B   (mod |n|),
			 * (2)  sign*Y*a  ==  A   (mod |n|)
			 */
			/* Now divide  B  by the maximum possible power of two in the integers,
			 * and divide  X  by the same value mod |n|.
			 * When we're done, (1) still holds. */
			shift = 0;

			while (!BN_is_bit_set(B, shift)) { /* note that 0 < B */
				shift++;

				if (BN_is_odd(X)) {
					if (!BN_uadd(X, X, n))
						goto err;
				}

				/* now X is even, so we can easily divide it by two */
				if (!BN_rshift1(X, X))
					goto err;
			}

			if (shift > 0) {
				if (!BN_rshift(B, B, shift))
					goto err;
			}

			/* Same for  A  and  Y.  Afterwards, (2) still holds. */
			shift = 0;

			while (!BN_is_bit_set(A, shift)) { /* note that 0 < A */
				shift++;

				if (BN_is_odd(Y)) {
					if (!BN_uadd(Y, Y, n))
						goto err;
				}

				/* now Y is even */
				if (!BN_rshift1(Y, Y))
					goto err;
			}

			if (shift > 0) {
				if (!BN_rshift(A, A, shift))
					goto err;
			}

			/* We still have (1) and (2).
			 * Both  A  and  B  are odd.
			 * The following computations ensure that
			 *
			 *     0 <= B < |n|,
			 *      0 < A < |n|,
			 * (1) -sign*X*a  ==  B   (mod |n|),
			 * (2)  sign*Y*a  ==  A   (mod |n|),
			 *
			 * and that either  A  or  B  is even in the next iteration.
			 */
			if (BN_ucmp(B, A) >= 0) {
				/* -sign*(X + Y)*a == B - A  (mod |n|) */
				if (!BN_uadd(X, X, Y))
					goto err;

				/* NB: we could use BN_mod_add_quick(X, X, Y, n), but that
				 * actually makes the algorithm slower */
				if (!BN_usub(B, B, A))
					goto err;
			} else {
				/*  sign*(X + Y)*a == A - B  (mod |n|) */
				if (!BN_uadd(Y, Y, X))
					goto err;

				/* as above, BN_mod_add_quick(Y, Y, X, n) would slow things down */
				if (!BN_usub(A, A, B))
					goto err;
			}
		}
	} else
		goto err;

	/*
	 * The while loop (Euclid's algorithm) ends when
	 *      A == gcd(a,n);
	 * we have
	 *       sign*Y*a  ==  A  (mod |n|),
	 * where  Y  is non-negative.
	 */

	if (sign < 0) {
		if (!BN_sub(Y, n, Y))
			goto err;
	}

	/* Now  Y*a  ==  A  (mod |n|).  */

	if (BN_is_one(A)) {
		/* Y*a == 1  (mod |n|) */
		if (BN_ucmp(Y, n) < 0) {
			if (!BN_copy(R, Y))
				goto err;
		} else {
			if (!(BN_mod(R, Y, n, ctx)))
				goto err;
		}
	} else
		goto err;

	ret = R;
err:

	if ((ret == NULL) && (in == NULL))
		BN_free(R);

	BN_POOL_release(ctx, 7);
	return ret;
}

static int BN_sub_word(struct bignum_st *a, BN_ULONG w)
{
	int i;

	w &= BN_MASK2;

	/* degenerate case: w is zero */
	if (!w)
		return 1;

	/* degenerate case: a is zero */
	if (BN_is_zero(a)) {
		i = BN_set_word(a, w);

		if (i != 0) {
			DHPRINT("BN_set_negative\n");
			/*BN_set_negative(a, 1); */
		}

		return i;
	}

	if ((a->top == 1) && (a->d[0] < w)) {
		a->d[0] = w - a->d[0];
		DHPRINT("--------\n");
		return 1;
	}

	i = 0;

	for (;;) {
		if (a->d[i] >= w) {
			a->d[i] -= w;
			break;
		} else {
			a->d[i] = (a->d[i] - w)&BN_MASK2;
			i++;
			w = 1;
		}
	}

	if ((a->d[i] == 0) && (i == (a->top - 1)))
		a->top--;

	return 1;
}

static int BN_MONT_CTX_set(struct bn_mont_ctx_st *mont, const struct bignum_st *mod, struct bignum_pool *ctx)
{
	int ret = 0;
	struct bignum_st *Ri, *R;
	struct bignum_st tmod;
	BN_ULONG buf[2];

	Ri = BN_POOL_get(ctx);

	if (Ri == NULL)
		goto err;

	R = &(mont->RR);					/* grab RR as a temp */

	if (!BN_copy(&(mont->N), mod))
		goto err;		/* Set N */

	/*start*/
	mont->ri = (BN_num_bits(mod) + (BN_BITS2 - 1)) / BN_BITS2 * BN_BITS2;
	BN_zero(R);

	if (!(BN_set_bit(R, BN_BITS2)))
		goto err;	/* R */

	buf[0] = mod->d[0]; /* tmod = N mod word size */
	buf[1] = 0;
	tmod.d = buf;
	tmod.top = buf[0] != 0 ? 1 : 0;
	tmod.dmax = 2;

	/* Ri = R^-1 mod N*/
	if ((BN_mod_inverse(Ri, R, &tmod, ctx)) == NULL)
		goto err;

	if (!BN_lshift(Ri, Ri, BN_BITS2))
		goto err; /* R*Ri */

	if (!BN_is_zero(Ri)) {
		if (!BN_sub_word(Ri, 1))
			goto err;
	} else { /* if N mod word size == 1 */
		if (!BN_set_word(Ri, BN_MASK2))
			goto err; /* Ri-- (mod word size) */
	}

	if (!BN_div(Ri, NULL, Ri, &tmod, ctx))
		goto err;

	/* Ni = (R*Ri-1)/N,
	 * keep only least significant word: */
	mont->n0 = (Ri->top > 0) ? Ri->d[0] : 0;
	/*end*/
	/* setup RR for conversions */
	BN_zero(&(mont->RR));

	if (!BN_set_bit(&(mont->RR), mont->ri * 2))
		goto err;

	if (!BN_mod(&(mont->RR), &(mont->RR), &(mont->N), ctx))
		goto err;

	ret = 1;
err:
	return ret;
}

static const struct env_md_st sha1_md = {
	NID_sha1,
	SHA_DIGEST_LENGTH,
	0,
	RAND_init,
	RAND_update,
	RAND_final,
	sizeof(struct env_md_st *) + sizeof(struct SHAstate_st),
};

/* This call frees resources associated with the context */
static int EVP_MD_CTX_cleanup(struct env_md_ctx_st *ctx)
{
	/* Don't assume ctx->md_data was cleaned in EVP_Digest_Final,
	 * because sometimes only copies of the context are ever finalised.
	 */
	if (ctx->digest && ctx->digest->ctx_size && ctx->md_data)
		os_free_mem(ctx->md_data);

	memset(ctx, '\0', sizeof(*ctx));
	return 1;
}

/* The caller can assume that this removes any secret data from the context */
static int EVP_DigestFinal_ex(struct env_md_ctx_st *ctx, unsigned char *md, unsigned int *size)
{
	int ret;

	ret = ctx->digest->final(ctx, md);

	if (size != NULL)
		*size = ctx->digest->md_size;

	memset(ctx->md_data, 0, ctx->digest->ctx_size);
	return ret;
}

static void EVP_MD_CTX_init(struct env_md_ctx_st *ctx)
{
	memset(ctx, '\0', sizeof(*ctx));
}

static int EVP_DigestInit_ex(struct env_md_ctx_st *ctx, const struct env_md_st *type)
{
	if (ctx->digest != type) {
		if (ctx->digest && ctx->digest->ctx_size)
			os_free_mem(ctx->md_data);

		ctx->digest = type;

		if (type->ctx_size)
			os_alloc_mem(NULL, (UCHAR **) &(ctx->md_data), type->ctx_size);
	}

	return ctx->digest->init(ctx);
}

static int EVP_DigestUpdate(struct env_md_ctx_st *ctx, const void *data,
							size_t count)
{
	return ctx->digest->update(ctx, data, count);
}

static void ssleay_rand_add(const void *buf, int num, long add)
{
	int i, j, k, st_idx;
	long md_c[2];
	unsigned char local_md[SHA_DIGEST_LENGTH];
	struct env_md_ctx_st m;
	/*
	 * (Based on the rand(3) manpage)
	 *
	 * The input is chopped up into units of 20 bytes (or less for
	 * the last block).  Each of these blocks is run through the hash
	 * function as follows:  The data passed to the hash function
	 * is the current 'md', the same number of bytes from the 'state'
	 * (the location determined by in incremented looping index) as
	 * the current 'block', the new key data 'block', and 'count'
	 * (which is incremented after each use).
	 * The result of this is kept in 'md' and also xored into the
	 * 'state' at the same locations that were used as input into the
	     * hash function.
	 */
	st_idx = g_state_index;
	/* use our own copies of the counters so that even
	 * if a concurrent thread seeds with exactly the
	 * same data and uses the same subarray there's _some_
	 * difference */
	md_c[0] = g_md_count[0];
	md_c[1] = g_md_count[1];
	memcpy(local_md, g_md, sizeof(g_md));
	/* g_state_index <= g_state_num <= STATE_SIZE */
	g_state_index += num;

	if (g_state_index >= STATE_SIZE) {
		g_state_index %= STATE_SIZE;
		g_state_num = STATE_SIZE;
	} else if (g_state_num < STATE_SIZE) {
		if (g_state_index > g_state_num)
			g_state_num = g_state_index;
	}

	/* g_state_index <= g_state_num <= STATE_SIZE */
	/* g_state[st_idx], ..., g_state[(st_idx + num - 1) % STATE_SIZE]
	 * are what we will use now, but other threads may use them
	 * as well */
	g_md_count[1] += (num / SHA_DIGEST_LENGTH) + (num % SHA_DIGEST_LENGTH > 0);
	EVP_MD_CTX_init(&m);

	for (i = 0; i < num; i += SHA_DIGEST_LENGTH) {
		j = (num - i);
		j = (j > SHA_DIGEST_LENGTH) ? SHA_DIGEST_LENGTH : j;
		EVP_DigestInit_ex(&m, &sha1_md);
		EVP_DigestUpdate(&m, local_md, SHA_DIGEST_LENGTH);
		k = (st_idx + j) - STATE_SIZE;

		if (k > 0) {
			EVP_DigestUpdate(&m, &(g_state[st_idx]), j - k);
			EVP_DigestUpdate(&m, &(g_state[0]), k);
		} else
			EVP_DigestUpdate(&m, &(g_state[st_idx]), j);

		EVP_DigestUpdate(&m, buf, j);
		EVP_DigestUpdate(&m, (unsigned char *) &(md_c[0]), sizeof(md_c));
		EVP_DigestFinal_ex(&m, local_md, NULL);
		md_c[1]++;
		buf = (const char *)buf + j;

		for (k = 0; k < j; k++) {
			/* Parallel threads may interfere with this,
			 * but always each byte of the new state is
			 * the XOR of some previous value of its
			 * and local_md (itermediate values may be lost).
			 * Alway using locking could hurt performance more
			 * than necessary given that conflicts occur only
			 * when the total seeding is longer than the random
			 * state. */
			g_state[st_idx++] ^= local_md[k];

			if (st_idx >= STATE_SIZE)
				st_idx = 0;
		}
	}

	EVP_MD_CTX_cleanup(&m);

	/* Don't just copy back local_md into md -- this could mean that
	 * other thread's seeding remains without effect (except for
	 * the incremented counter).  By XORing it we keep at least as
	 * much entropy as fits into md. */
	for (k = 0; k < (int)sizeof(g_md); k++)
		g_md[k] ^= local_md[k];

	if (g_entropy < ENTROPY_NEEDED) /* stop counting when we have enough */
		g_entropy += add;
}

static int ssleay_rand_bytes(unsigned char *buf, int num)
{
	static volatile int stirred_pool;
	int i, j, k, st_num, st_idx;
	int num_ceil;
	int ok;
	long md_c[2];
	unsigned char local_md[SHA_DIGEST_LENGTH];
	struct env_md_ctx_st m;
#ifndef GETPID_IS_MEANINGLESS
	pid_t curr_pid = getpid();
#endif
	int do_stir_pool = 0;

	if (num <= 0)
		return 1;

	EVP_MD_CTX_init(&m);
	/* round upwards to multiple of SHA_DIGEST_LENGTH/2 */
	num_ceil = (1 + (num - 1) / (SHA_DIGEST_LENGTH / 2)) * (SHA_DIGEST_LENGTH / 2);

	/*
	 * (Based on the rand(3) manpage:)
	 *
	 * For each group of 10 bytes (or less), we do the following:
	 *
	 * Input into the hash function the local 'md' (which is RAND_initialized from
	 * the global 'md' before any bytes are generated), the bytes that are to
	 * be overwritten by the random bytes, and bytes from the 'state'
	 * (incrementing looping index). From this digest output (which is kept
	 * in 'md'), the top (up to) 10 bytes are returned to the caller and the
	 * bottom 10 bytes are xored into the 'state'.
	 *
	 * Finally, after we have finished 'num' random bytes for the
	 * caller, 'count' (which is incremented) and the local and global 'md'
	 * are fed into the hash function and the results are kept in the
	 * global 'md'.
	 */

	if (!RAND_initialized) {
		RAND_poll();
		RAND_initialized = 1;
	}

	if (!stirred_pool)
		do_stir_pool = 1;

	ok = (g_entropy >= ENTROPY_NEEDED);

	if (!ok) {
		/* If the PRNG state is not yet unpredictable, then seeing
		 * the PRNG output may help attackers to determine the new
		 * state; thus we have to decrease the entropy estimate.
		 * Once we've had enough initial seeding we don't bother to
		 * adjust the entropy count, though, because we're not ambitious
		 * to provide *information-theoretic* randomness.
		 *
		 * NOTE: This approach fails if the program forks before
		 * we have enough entropy. Entropy should be collected
		 * in a separate input pool and be transferred to the
		 * output pool only when the entropy limit has been reached.
		 */
		g_entropy -= num;

		if (g_entropy < 0)
			g_entropy = 0;
	}

	if (do_stir_pool) {
		/* In the output function only half of 'md' remains secret,
		 * so we better make sure that the required entropy gets
		 * 'evenly distributed' through 'state', our randomness pool.
		 * The input function (ssleay_rand_add) chains all of 'md',
		 * which makes it more suitable for this purpose.
		 */
		int n = STATE_SIZE; /* so that the complete pool gets accessed */

		while (n > 0) {
			/* Note that the seed does not matter, it's just that
			 * ssleay_rand_add expects to have something to hash. */
			/* at least SHA_DIGEST_LENGTH "...................." */
			ssleay_rand_add("....................", SHA_DIGEST_LENGTH, 0);
			n -= SHA_DIGEST_LENGTH;
		}

		if (ok)
			stirred_pool = 1;
	}

	st_idx = g_state_index;
	st_num = g_state_num;
	md_c[0] = g_md_count[0];
	md_c[1] = g_md_count[1];
	memcpy(local_md, g_md, sizeof(g_md));
	g_state_index += num_ceil;

	if (g_state_index > g_state_num)
		g_state_index %= g_state_num;

	/* g_state[st_idx], ..., g_state[(st_idx + num_ceil - 1) % st_num]
	 * are now ours (but other threads may use them too) */
	g_md_count[0] += 1;

	while (num > 0) {
		/* num_ceil -= SHA_DIGEST_LENGTH/2 */
		j = (num >= SHA_DIGEST_LENGTH / 2) ? SHA_DIGEST_LENGTH / 2 : num;
		num -= j;
		EVP_DigestInit_ex(&m, &sha1_md);
#ifndef GETPID_IS_MEANINGLESS

		if (curr_pid) { /* just in the first iteration to save time */
			EVP_DigestUpdate(&m, (unsigned char *)&curr_pid, sizeof(curr_pid));
			curr_pid = 0;
		}

#endif
		EVP_DigestUpdate(&m, local_md, SHA_DIGEST_LENGTH);
		EVP_DigestUpdate(&m, (unsigned char *) &(md_c[0]), sizeof(md_c));
#ifndef PURIFY
		EVP_DigestUpdate(&m, buf, j); /* purify complains */
#endif
		k = (st_idx + SHA_DIGEST_LENGTH / 2) - st_num;

		if (k > 0) {
			EVP_DigestUpdate(&m, &(g_state[st_idx]), SHA_DIGEST_LENGTH / 2 - k);
			EVP_DigestUpdate(&m, &(g_state[0]), k);
		} else
			EVP_DigestUpdate(&m, &(g_state[st_idx]), SHA_DIGEST_LENGTH / 2);

		EVP_DigestFinal_ex(&m, local_md, NULL);

		for (i = 0; i < SHA_DIGEST_LENGTH / 2; i++) {
			g_state[st_idx++] ^= local_md[i]; /* may compete with other threads */

			if (st_idx >= st_num)
				st_idx = 0;

			if (i < j)
				*(buf++) = local_md[i + SHA_DIGEST_LENGTH / 2];
		}
	}

	EVP_DigestInit_ex(&m, &sha1_md);
	EVP_DigestUpdate(&m, (unsigned char *) &(md_c[0]), sizeof(md_c));
	EVP_DigestUpdate(&m, local_md, SHA_DIGEST_LENGTH);
	EVP_DigestUpdate(&m, g_md, SHA_DIGEST_LENGTH);
	EVP_DigestFinal_ex(&m, g_md, NULL);
	EVP_MD_CTX_cleanup(&m);

	if (ok)
		return 1;
	else
		return 0;
}

static int RAND_poll(void)
{
	unsigned long l = 100;
	unsigned char tmpbuf[ENTROPY_NEEDED];
	int n = 0;

	n = ENTROPY_NEEDED;

	if (n > 0)
		ssleay_rand_add(tmpbuf, sizeof(tmpbuf), (long)n);

	/* put in some default random data, we need more than just this */
	l = 100; /*curr_pid; */
	ssleay_rand_add(&l, sizeof(l), 0);
	l = 200; /*getuid(); */
	ssleay_rand_add(&l, sizeof(l), 0);
#ifdef USERSPACE
	l = time(NULL);
#else
	NdisGetSystemUpTime(&l);
#endif /*USERSPACE */
	ssleay_rand_add(&l, sizeof(l), 0);
	return 0;
}

static int bnrand(int pseudorand, struct bignum_st *rnd, int bits, int top, int bottom)
{
	unsigned char *buf = NULL;
	int ret = 0, bit, bytes, mask;
	time_t tim;

	if (bits == 0) {
		BN_zero(rnd);
		return 1;
	}

	bytes = (bits + 7) / 8;
	bit = (bits - 1) % 8;
	mask = 0xff << (bit + 1);
	os_alloc_mem(NULL, (UCHAR **)&buf, bytes);

	if (buf == NULL)
		goto err;

	/* make a random number and set the top and bottom bits */
#ifdef USERSPACE
	time(&tim);
#else
	NdisGetSystemUpTime((ULONG *)(&tim));
#endif /*USERSPACE */
	ssleay_rand_add(&tim, sizeof(tim), 0);

	if (pseudorand) {
		if (ssleay_rand_bytes(buf, bytes) == -1)
			goto err;
	} else {
		if (ssleay_rand_bytes(buf, bytes) <= 0)
			goto err;
	}

	if (pseudorand == 2) {
		/* generate patterns that are more likely to trigger BN
		   library bugs */
		int i;
		unsigned char c;

		for (i = 0; i < bytes; i++) {
			ssleay_rand_bytes(&c, 1);

			if (c >= 128 && i > 0)
				buf[i] = buf[i - 1];
			else if (c < 42)
				buf[i] = 0;
			else if (c < 84)
				buf[i] = 255;
		}
	}

	if (top != -1) {
		if (top) {
			if (bit == 0) {
				buf[0] = 1;
				buf[1] |= 0x80;
			} else
				buf[0] |= (3 << (bit - 1));
		} else
			buf[0] |= (1 << bit);
	}

	buf[0] &= ~mask;

	if (bottom) /* set bottom bit if requested */
		buf[bytes - 1] |= 1;

	if (!BN_bin2bn(buf, bytes, rnd))
		goto err;

	ret = 1;
err:

	if (buf != NULL)
		os_free_mem(buf);

	return ret;
}

static struct bn_mont_ctx_st *BN_MONT_CTX_new(void)
{
	struct bn_mont_ctx_st *ret;

	os_alloc_mem(NULL, (UCHAR **)&ret, sizeof(struct bn_mont_ctx_st));

	if (ret == NULL)
		return NULL;

	ret->ri = 0;
	BN_init(&(ret->RR));
	BN_init(&(ret->N));
	ret->flags = BN_FLG_MALLOCED;
	return ret;
}

static void BN_CTX_free(struct bignum_pool *ctx)
{
	if (ctx == NULL)
		return;

	BN_POOL_finish(ctx);
	os_free_mem(ctx);
}

static void BN_MONT_CTX_free(struct bn_mont_ctx_st *mont)
{
	if (mont == NULL)
		return;

	BN_free(&(mont->RR));
	BN_free(&(mont->N));

	if (mont->flags & BN_FLG_MALLOCED)
		os_free_mem(mont);
}

#define BN_MOD_MUL_WORD(r, w, m) (BN_mul_word(r, (w)) && ((BN_mod(t, r, m, ctx) && (swap_tmp = r, r = t, t = swap_tmp, 1))))
static int BN_mod_exp_mont_word(struct bignum_st *rr, BN_ULONG a, const struct bignum_st *p,
								const struct bignum_st *m, struct bignum_pool *ctx, struct bn_mont_ctx_st *in_mont)
{
	struct bn_mont_ctx_st *mont = NULL;
	int b, bits, ret = 0;
	int r_is_one;
	BN_ULONG w, next_w;
	struct bignum_st *d, *r, *t;
	struct bignum_st *swap_tmp;

	if (BN_get_flags(p, BN_FLG_EXP_CONSTTIME) != 0) {
		/* BN_FLG_EXP_CONSTTIME only supported by BN_mod_exp_mont() */
		return -1;
	}

	if (!BN_is_odd(m))
		return 0;

	if (m->top == 1)
		a %= m->d[0]; /* make sure that 'a' is reduced */

	bits = BN_num_bits(p);

	if (bits == 0) {
		ret = BN_one(rr);
		return ret;
	}

	if (a == 0) {
		BN_zero(rr);
		ret = 1;
		return ret;
	}

	d = BN_POOL_get(ctx);
	r = BN_POOL_get(ctx);
	t = BN_POOL_get(ctx);

	if (d == NULL || r == NULL || t == NULL)
		goto err;

	if (in_mont != NULL)
		mont = in_mont;
	else {
		mont = BN_MONT_CTX_new();

		if (mont == NULL)
			goto err;

		if (!BN_MONT_CTX_set(mont, m, ctx))
			goto err;
	}

	r_is_one = 1; /* except for Montgomery factor */
	/* bits-1 >= 0 */
	/* The result is accumulated in the product r*w. */
	w = a; /* bit 'bits-1' of 'p' is always set */

	for (b = bits - 2; b >= 0; b--) {
		/* First, square r*w. */
		next_w = w * w;

		if ((next_w / w) != w) { /* overflow */
			if (r_is_one) {
				if (!(BN_set_word(r, (w)) && BN_mod_mul_montgomery((r), (r), &((mont)->RR), (mont), (ctx))))
					goto err;

				r_is_one = 0;
			} else {
				if (!BN_MOD_MUL_WORD(r, w, m))
					goto err;
			}

			next_w = 1;
		}

		w = next_w;

		if (!r_is_one) {
			if (!BN_mod_mul_montgomery(r, r, r, mont, ctx))
				goto err;
		}

		/* Second, multiply r*w by 'a' if exponent bit is set. */
		if (BN_is_bit_set(p, b)) {
			next_w = w * a;

			if ((next_w / a) != w) { /* overflow */
				if (r_is_one) {
					if (!(BN_set_word(r, (w)) && BN_mod_mul_montgomery((r), (r), &((mont)->RR), (mont), (ctx))))
						goto err;

					r_is_one = 0;
				} else {
					if (!BN_MOD_MUL_WORD(r, w, m))
						goto err;
				}

				next_w = a;
			}

			w = next_w;
		}
	}

	/* Finally, set r:=r*w. */
	if (w != 1) {
		if (r_is_one) {
			if (!(BN_set_word(r, (w)) && BN_mod_mul_montgomery((r), (r), &((mont)->RR), (mont), (ctx))))
				goto err;

			r_is_one = 0;
		} else {
			if (!BN_MOD_MUL_WORD(r, w, m))
				goto err;
		}
	}

	if (r_is_one) { /* can happen only if a == 1*/
		if (!BN_one(rr))
			goto err;
	} else {
		if (!BN_from_montgomery(rr, r, mont, ctx))
			goto err;
	}

	ret = 1;
err:

	if ((in_mont == NULL) && (mont != NULL))
		BN_MONT_CTX_free(mont);

	BN_POOL_release(ctx, 3);
	return ret;
}

/* BN_mod_exp_mont_consttime() stores the precomputed powers in a specific layout
 * so that accessing any of these table values shows the same access pattern as far
 * as cache lines are concerned.  The following functions are used to transfer a struct bignum_st
 * from/to that table. */
static int MOD_EXP_CTIME_COPY_TO_PREBUF(struct bignum_st *b, int top, unsigned char *buf, int idx, int width)
{
	size_t i, j;

	if (bn_wexpand(b, top) == NULL)
		return 0;

	while (b->top < top)
		b->d[b->top++] = 0;

	for (i = 0, j = idx; i < top * sizeof(b->d[0]); i++, j += width)
		buf[j] = ((unsigned char *)b->d)[i];

	bn_correct_top(b);
	return 1;
}

static int MOD_EXP_CTIME_COPY_FROM_PREBUF(struct bignum_st *b, int top, unsigned char *buf, int idx, int width)
{
	size_t i, j;

	if (bn_wexpand(b, top) == NULL)
		return 0;

	for (i = 0, j = idx; i < top * sizeof(b->d[0]); i++, j += width)
		((unsigned char *)b->d)[i] = buf[j];

	b->top = top;
	bn_correct_top(b);
	return 1;
}

/* This variant of BN_mod_exp_mont() uses fixed windows and the special
 * precomputation memory layout to limit data-dependency to a minimum
 * to protect secret exponents (cf. the hyper-threading timing attacks
 * pointed out by Colin Percival,
 * http:/*www.daemonology.net/hyperthreading-considered-harmful/) */
*/
static int BN_mod_exp_mont_consttime(struct bignum_st *rr, const struct bignum_st *a, const struct bignum_st *p,
									 const struct bignum_st *m, struct bignum_pool *ctx, struct bn_mont_ctx_st *in_mont)
{
	int i, bits, ret = 0, idx, window, wvalue;
	int top;
	struct bignum_st *r;
	const struct bignum_st *aa;
	struct bn_mont_ctx_st *mont = NULL;
	int numPowers;
	unsigned char *powerbufFree = NULL;
	int powerbufLen = 0;
	unsigned char *powerbuf = NULL;
	struct bignum_st *computeTemp = NULL, *am = NULL;

	top = m->top;

	if (!(m->d[0] & 1))
		return 0;

	bits = BN_num_bits(p);

	if (bits == 0) {
		ret = BN_one(rr);
		return ret;
	}

	r = BN_POOL_get(ctx);

	if (r == NULL)
		goto err;

	/* Allocate a montgomery context if it was not supplied by the caller.
	 * If this is not done, things will break in the montgomery part.
	 */
	if (in_mont != NULL)
		mont = in_mont;
	else {
		mont = BN_MONT_CTX_new();

		if (mont == NULL)
			goto err;

		if (!BN_MONT_CTX_set(mont, m, ctx)
			goto err;
	}

/* Get the window size to use with size of p. */
window = BN_window_bits_for_ctime_exponent_size(bits);
	/* Allocate a buffer large enough to hold all of the pre-computed
	 * powers of a.
	 */
	numPowers = 1 << window;
	powerbufLen = sizeof(m->d[0]) * top * numPowers;
	os_alloc_mem(NULL, (UCHAR **)&powerbufFree, powerbufLen + MOD_EXP_CTIME_MIN_CACHE_LINE_WIDTH);

	if (powerbufFree == NULL)
		goto err;

	powerbuf = MOD_EXP_CTIME_ALIGN(powerbufFree);
	memset(powerbuf, 0, powerbufLen);

	/* Initialize the intermediate result. Do this early to save double conversion,
	 * once each for a^0 and intermediate result.
	 */
	if (!BN_mod_mul_montgomery((r), (BN_value_one()), &((mont)->RR), (mont), (ctx)))
		goto err;

	if (!MOD_EXP_CTIME_COPY_TO_PREBUF(r, top, powerbuf, 0, numPowers))
		goto err;

	/* Initialize computeTemp as a^1 with montgomery precalcs */
	computeTemp = BN_POOL_get(ctx);
	am = BN_POOL_get(ctx);

	if (computeTemp == NULL || am == NULL)
		goto err;

	if (BN_ucmp(a, m) >= 0) {
		if (!BN_mod(am, a, m, ctx))
			goto err;

		aa = am;
	} else
		aa = a;

	if (!BN_mod_mul_montgomery((am), (aa), &((mont)->RR), (mont), (ctx)))
		goto err;

	if (!BN_copy(computeTemp, am))
		goto err;

	if (!MOD_EXP_CTIME_COPY_TO_PREBUF(am, top, powerbuf, 1, numPowers))
		goto err;

	/* If the window size is greater than 1, then calculate
	 * val[i=2..2^winsize-1]. Powers are computed as a*a^(i-1)
	 * (even powers could instead be computed as (a^(i/2))^2
	 * to use the slight performance advantage of sqr over mul).
	 */
	if (window > 1) {
		for (i = 2; i < numPowers; i++) {
			/* Calculate a^i = a^(i-1) * a */
			if (!BN_mod_mul_montgomery(computeTemp, am, computeTemp, mont, ctx))
				goto err;

			if (!MOD_EXP_CTIME_COPY_TO_PREBUF(computeTemp, top, powerbuf, i, numPowers))
				goto err;
		}
	}

	/* Adjust the number of bits up to a multiple of the window size.
	 * If the exponent length is not a multiple of the window size, then
	 * this pads the most significant bits with zeros to normalize the
	 * scanning loop to there's no special cases.
	 *
	 * * NOTE: Making the window size a power of two less than the native
	 * * word size ensures that the padded bits won't go past the last
	 * * word in the internal struct bignum_st structure. Going past the end will
	 * * still produce the correct result, but causes a different branch
	 * * to be taken in the BN_is_bit_set function.
	 */
	bits = ((bits + window - 1) / window) *window;
	idx = bits - 1;	/* The top bit of the window */

	/* Scan the exponent one window at a time starting from the most
	 * significant bits.
	 */
	while (idx >= 0) {
		wvalue = 0; /* The 'value' of the window */

		/* Scan the window, squaring the result as we go */
		for (i = 0; i < window; i++, idx--) {
			if (!BN_mod_mul_montgomery(r, r, r, mont, ctx))
				goto err;

			wvalue = (wvalue << 1) + BN_is_bit_set(p, idx);
		}

		/* Fetch the appropriate pre-computed value from the pre-buf */
		if (!MOD_EXP_CTIME_COPY_FROM_PREBUF(computeTemp, top, powerbuf, wvalue, numPowers))
			goto err;

		/* Multiply the result into the intermediate result */
		if (!BN_mod_mul_montgomery(r, r, computeTemp, mont, ctx))
			goto err;
	}

	/* Convert the final result from montgomery to standard format */
	if (!BN_from_montgomery(rr, r, mont, ctx))
		goto err;

	ret = 1;
err:

	if ((in_mont == NULL) && (mont != NULL))
		BN_MONT_CTX_free(mont);

	if (powerbuf != NULL)
		os_free_mem(powerbufFree);

	if (am != NULL)
		BN_clear(am);

	if (computeTemp != NULL)
		BN_clear(computeTemp);

	BN_POOL_release(ctx, 3);
	return ret;
}

static int BN_mod_exp_mont(struct bignum_st *rr, const struct bignum_st *a, const struct bignum_st *p,
						   const struct bignum_st *m, struct bignum_pool *ctx, struct bn_mont_ctx_st *in_mont)
{
	int release = 0;
	int i, j, bits, ret = 0, wstart, wend, window, wvalue;
	int start = 1;
	struct bignum_st *d, *r;
	const struct bignum_st *aa;
	/* Table of variables obtained from 'ctx' */
	struct bignum_st *val[EXP_TABLE_SIZE];
	struct bn_mont_ctx_st *mont = NULL;

	if (BN_get_flags(p, BN_FLG_EXP_CONSTTIME) != 0)
		return BN_mod_exp_mont_consttime(rr, a, p, m, ctx, in_mont);

	if (!BN_is_odd(m))
		return 0;

	bits = BN_num_bits(p);

	if (bits == 0) {
		ret = BN_one(rr);
		return ret;
	}

	d = BN_POOL_get(ctx);
	r = BN_POOL_get(ctx);
	val[0] = BN_POOL_get(ctx);
	release = 3;

	if (!d || !r || !val[0])
		goto err;

	/* If this is not done, things will break in the montgomery
	 * part */

	if (in_mont != NULL)
		mont = in_mont;
	else {
		mont = BN_MONT_CTX_new();

		if (mont == NULL)
			goto err;

		if (!BN_MONT_CTX_set(mont, m, ctx))
			goto err;
	}

	if (BN_ucmp(a, m) >= 0) {
		if (!(BN_mod(val[0], a, m, ctx)))
			goto err;

		aa = val[0];
	} else
		aa = a;

	if (BN_is_zero(aa)) {
		BN_zero(rr);
		ret = 1;
		goto err;
	}

	if (!BN_mod_mul_montgomery((val[0]), (aa), &((mont)->RR), (mont), (ctx)))
		goto err; /* 1 */

	window = BN_window_bits_for_exponent_size(bits);

	if (window > 1) {
		if (!BN_mod_mul_montgomery(d, val[0], val[0], mont, ctx))
			goto err; /* 2 */

		j = 1 << (window - 1);

		for (i = 1; i < j; i++) {
			val[i] = BN_POOL_get(ctx);

			if ((val[i] == NULL) ||
				!BN_mod_mul_montgomery(val[i], val[i - 1],
									   d, mont, ctx))
				goto err;

			release++;
		}
	}

	start = 1;	/* This is used to avoid multiplication etc
			 * when there is only the value '1' in the
			 * buffer. */
	wvalue = 0;	/* The 'value' of the window */
	wstart = bits - 1;	/* The top bit of the window */
	wend = 0;		/* The bottom bit of the window */

	if (!BN_mod_mul_montgomery((r), (BN_value_one()), &((mont)->RR), (mont), (ctx)))
		goto err;

	for (;;) {
		if (BN_is_bit_set(p, wstart) == 0) {
			if (!start) {
				if (!BN_mod_mul_montgomery(r, r, r, mont, ctx))
					goto err;
			}

			if (wstart == 0)
				break;

			wstart--;
			continue;
		}

		/* We now have wstart on a 'set' bit, we now need to work out
		 * how bit a window to do.  To do this we need to scan
		 * forward until the last set bit before the end of the
		 * window */
		j = wstart;
		wvalue = 1;
		wend = 0;

		for (i = 1; i < window; i++) {
			if (wstart - i < 0)
				break;

			if (BN_is_bit_set(p, wstart - i)) {
				wvalue <<= (i - wend);
				wvalue |= 1;
				wend = i;
			}
		}

		/* wend is the size of the current window */
		j = wend + 1;

		/* add the 'bytes above' */
		if (!start)
			for (i = 0; i < j; i++) {
				if (!BN_mod_mul_montgomery(r, r, r, mont, ctx))
					goto err;
			}

		/* wvalue will be an odd number < 2^window */
		if (!BN_mod_mul_montgomery(r, r, val[wvalue >> 1], mont, ctx))
			goto err;

		/* move the 'window' down further */
		wstart -= wend + 1;
		wvalue = 0;
		start = 0;

		if (wstart < 0)
			break;
	}

	if (!BN_from_montgomery(rr, r, mont, ctx))
		goto err;

	ret = 1;
err:

	if ((in_mont == NULL) && (mont != NULL))
		BN_MONT_CTX_free(mont);

	BN_POOL_release(ctx, release);
	return ret;
}

/*KERNEL MODULE TRACE*/
static unsigned char DH_P_VALUE[DH_KEY_LEN] = {
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xC9, 0x0F, 0xDA, 0xA2, 0x21, 0x68, 0xC2, 0x34,
	0xC4, 0xC6, 0x62, 0x8B, 0x80, 0xDC, 0x1C, 0xD1,
	0x29, 0x02, 0x4E, 0x08, 0x8A, 0x67, 0xCC, 0x74,
	0x02, 0x0B, 0xBE, 0xA6, 0x3B, 0x13, 0x9B, 0x22,
	0x51, 0x4A, 0x08, 0x79, 0x8E, 0x34, 0x04, 0xDD,
	0xEF, 0x95, 0x19, 0xB3, 0xCD, 0x3A, 0x43, 0x1B,
	0x30, 0x2B, 0x0A, 0x6D, 0xF2, 0x5F, 0x14, 0x37,
	0x4F, 0xE1, 0x35, 0x6D, 0x6D, 0x51, 0xC2, 0x45,
	0xE4, 0x85, 0xB5, 0x76, 0x62, 0x5E, 0x7E, 0xC6,
	0xF4, 0x4C, 0x42, 0xE9, 0xA6, 0x37, 0xED, 0x6B,
	0x0B, 0xFF, 0x5C, 0xB6, 0xF4, 0x06, 0xB7, 0xED,
	0xEE, 0x38, 0x6B, 0xFB, 0x5A, 0x89, 0x9F, 0xA5,
	0xAE, 0x9F, 0x24, 0x11, 0x7C, 0x4B, 0x1F, 0xE6,
	0x49, 0x28, 0x66, 0x51, 0xEC, 0xE4, 0x5B, 0x3D,
	0xC2, 0x00, 0x7C, 0xB8, 0xA1, 0x63, 0xBF, 0x05,
	0x98, 0xDA, 0x48, 0x36, 0x1C, 0x55, 0xD3, 0x9A,
	0x69, 0x16, 0x3F, 0xA8, 0xFD, 0x24, 0xCF, 0x5F,
	0x83, 0x65, 0x5D, 0x23, 0xDC, 0xA3, 0xAD, 0x96,
	0x1C, 0x62, 0xF3, 0x56, 0x20, 0x85, 0x52, 0xBB,
	0x9E, 0xD5, 0x29, 0x07, 0x70, 0x96, 0x96, 0x6D,
	0x67, 0x0C, 0x35, 0x4E, 0x4A, 0xBC, 0x98, 0x04,
	0xF1, 0x74, 0x6C, 0x08, 0xCA, 0x23, 0x73, 0x27,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

static struct dh_st *DH_new_method(void)
{
	struct dh_st *ret;

	os_alloc_mem(NULL, (UCHAR **)&ret, sizeof(struct dh_st));

	if (ret == NULL)
		return NULL;

	ret->p = NULL;
	ret->g = NULL;
	ret->pub_key = NULL;
	ret->priv_key = NULL;
	ret->flags = 0;
	return ret;
}

static int dh_bn_mod_exp(const struct dh_st *dh, struct bignum_st *r,
						 const struct bignum_st *a, const struct bignum_st *p,
						 const struct bignum_st *m, struct bignum_pool *ctx,
						 struct bn_mont_ctx_st *m_ctx)
{
	/* If a is only one word long and constant time is false, use the faster
	 * exponenentiation function.
	 */
	if (a->top == 1 && ((dh->flags & DH_FLAG_NO_EXP_CONSTTIME) != 0)) {
		BN_ULONG A = a->d[0];

		return BN_mod_exp_mont_word(r, A, p, m, ctx, m_ctx);
	} else
		return BN_mod_exp_mont(r, a, p, m, ctx, m_ctx);
}

static int generate_key(struct dh_st *dh)
{
	int ok = 0;
	int generate_new_key = 0;
	unsigned lvar;
	struct bignum_pool *ctx;
	struct bn_mont_ctx_st *mont = NULL;
	struct bignum_st *pub_key = NULL, *priv_key = NULL;

	ctx = BN_CTX_new();

	if (ctx == NULL)
		goto err;

	if (dh->priv_key == NULL) {
		priv_key = BN_new();

		if (priv_key == NULL)
			goto err;

		generate_new_key = 1;
	} else
		priv_key = dh->priv_key;

	if (dh->pub_key == NULL) {
		pub_key = BN_new();

		if (pub_key == NULL)
			goto err;
	} else
		pub_key = dh->pub_key;

	if (generate_new_key) {
		lvar = BN_num_bits(dh->p) - 1; /* secret exponent length */

		if (!bnrand(0, priv_key, lvar, 0, 0))
			goto err;
	}

	{
		struct bignum_st local_prk;
		struct bignum_st *prk;

		if ((dh->flags & DH_FLAG_NO_EXP_CONSTTIME) == 0) {
			BN_init(&local_prk);
			prk = &local_prk;
			BN_with_flags(prk, priv_key, BN_FLG_EXP_CONSTTIME);
		} else
			prk = priv_key;

		if (!dh_bn_mod_exp(dh, pub_key, dh->g, prk, dh->p, ctx, mont))
			goto err;
	}

	dh->pub_key = pub_key;
	dh->priv_key = priv_key;
	ok = 1;
err:

	if ((pub_key != NULL)  && (dh->pub_key == NULL))
		BN_free(pub_key);

	if ((priv_key != NULL) && (dh->priv_key == NULL))
		BN_free(priv_key);

	BN_CTX_free(ctx);
	return ok;
}

static void DH_free(struct dh_st *r)
{
	if (r == NULL)
		return;

	if (r->p != NULL)
		BN_clear_free(r->p);

	if (r->g != NULL)
		BN_clear_free(r->g);

	if (r->pub_key != NULL)
		BN_clear_free(r->pub_key);

	if (r->priv_key != NULL)
		BN_clear_free(r->priv_key);

	os_free_mem(r);
}

static int compute_key(unsigned char *key, const struct bignum_st *pub_key, struct dh_st *dh)
{
	struct bignum_pool *ctx = NULL;
	struct bn_mont_ctx_st *mont = NULL;
	struct bignum_st *tmp;
	int ret = -1;

	if (BN_num_bits(dh->p) > OPENSSL_DH_MAX_MODULUS_BITS)
		goto err;

	ctx = BN_CTX_new();

	if (ctx == NULL)
		goto err;

	tmp = BN_POOL_get(ctx);

	if (dh->priv_key == NULL)
		goto err;

	if (!dh_bn_mod_exp(dh, tmp, pub_key, dh->priv_key, dh->p, ctx, mont))
		goto err;

	ret = BN_bn2bin(tmp, key);
err:
	BN_CTX_free(ctx);
	return ret;
}

static struct dh_st *DHSecret;
static struct dh_st *DHSecretR;

void GenerateDHPublicKey(unsigned char *memPtr, unsigned char *ran_buf, int ran_len,
						 unsigned char *dhkey, int *dhkey_len)
{
	unsigned int DH_G_VALUE = 2;
	unsigned int g = htonl(DH_G_VALUE);
	*dhkey_len = 0;

	if (DHSecret == NULL) {
		DHSecret = DH_new_method();

		if (DHSecret == NULL)
			goto err;

		DHSecret->p = BN_new();
		DHSecret->g = BN_new();
	} else if ((DHSecret != NULL) && (DHSecretR == NULL)) {
		DH_free(DHSecret);
		DHSecret = NULL;
		DHSecret = DH_new_method();

		if (DHSecret == NULL)
			goto err;

		DHSecret->p = BN_new();
		DHSecret->g = BN_new();
	} else if ((DHSecret != NULL) && (DHSecretR != NULL))
		DHPRINT("reuse DHSecret\n");

	BN_bin2bn(DH_P_VALUE, DH_KEY_LEN, DHSecret->p);
	BN_bin2bn((const unsigned char *)&g, 4, DHSecret->g);
	/*
		if (!DH_check(DHSecret, &i))
			goto err;
		if (i & DH_CHECK_P_NOT_PRIME)
			DHPRINT( "p value is not prime\n");
		if (i & DH_CHECK_P_NOT_SAFE_PRIME)
			DHPRINT( "p value is not a safe prime\n");
		if (i & DH_UNABLE_TO_CHECK_GENERATOR)
			DHPRINT( "unable to check the generator value\n");
		if (i & DH_NOT_SUITABLE_GENERATOR)
			DHPRINT( "the g value is not a generator\n");
	*/
	/*DHSecret->flags &= ~DH_FLAG_NO_EXP_CONSTTIME; */
	DHSecret->flags |= DH_FLAG_NO_EXP_CONSTTIME;

	if (!generate_key(DHSecret))
		goto err;

	BN_bn2bin(DHSecret->pub_key, dhkey);
	*dhkey_len = BN_num_bytes(DHSecret->pub_key);
	DHPRINT("GenerateDHPublicKey Success dhkey_len%d\n", *dhkey_len);
	/* DHSecret will use and free on GenerateDHSecreteKey. NO free it */
	return;
err:
	DH_free(DHSecret);
	DHSecret = NULL;
	DHPRINT("GenerateDHPublicKey EXIT\n");
	return;
}

/******* KDK generation ****** */
/*generate the struct dh_st shared secret */
void GenerateDHSecreteKey(unsigned char *memPtr, unsigned char *ran_buf, int ran_len,
						  unsigned char *peer_dhkey, int peer_dhkey_len,
						  unsigned char *secrete_dhkey, int *secrete_dhkey_len)
{
	struct bignum_st DH_PubKey_Peer;
	*secrete_dhkey_len = 0;

	if (secrete_dhkey == NULL)
		goto err;

	if (DHSecret == NULL)
		goto err;

	DH_PubKey_Peer.flags = BN_FLG_MALLOCED;
	DH_PubKey_Peer.top = 0;
	DH_PubKey_Peer.dmax = 0;
	DH_PubKey_Peer.d = NULL;
	BN_bin2bn(peer_dhkey, peer_dhkey_len, &DH_PubKey_Peer);
	*secrete_dhkey_len = compute_key(secrete_dhkey, &DH_PubKey_Peer, DHSecret);

	if (DH_PubKey_Peer.d != NULL)
		os_free_mem(DH_PubKey_Peer.d);

	DHPRINT("GenerateDHSecreteKey Success\n");
err:
	DH_free(DHSecret);
	DHSecret = NULL;
	DHPRINT("GenerateDHSecreteKey EXIT\n");
	return;
}

extern void DH_freeall(void)
{
	DH_free(DHSecret);
	DHSecret = NULL;
	return;
}

#endif /* WSC_INCLUDED */

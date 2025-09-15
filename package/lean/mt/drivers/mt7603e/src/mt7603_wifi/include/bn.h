/* ====================================================================
 * Copyright (c) 1998-2018 The OpenSSL Project.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit. (http://www.openssl.org/)"
 *
 * 4. The names "OpenSSL Toolkit" and "OpenSSL Project" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. For written permission, please contact
 *    openssl-core@openssl.org.
 *
 * 5. Products derived from this software may not be called "OpenSSL"
 *    nor may "OpenSSL" appear in their names without prior written
 *    permission of the OpenSSL Project.
 *
 * 6. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit (http://www.openssl.org/)"
 *
 * THIS SOFTWARE IS PROVIDED BY THE OpenSSL PROJECT ``AS IS'' AND ANY
 * EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE OpenSSL PROJECT OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 * ====================================================================
 *
 * This product includes cryptographic software written by Eric Young
 * (eay@cryptsoft.com).  This product includes software written by Tim
 * Hudson (tjh@cryptsoft.com).
 *
 */

/* Copyright (C) 1995-1998 Eric Young (eay@cryptsoft.com)
 * All rights reserved.
 *
 * This package is an SSL implementation written
 * by Eric Young (eay@cryptsoft.com).
 * The implementation was written so as to conform with Netscapes SSL.
 *
 * This library is free for commercial and non-commercial use as long as
 * the following conditions are aheared to.  The following conditions
 * apply to all code found in this distribution, be it the RC4, RSA,
 * lhash, DES, etc., code; not just the SSL code.  The SSL documentation
 * included with this distribution is covered by the same copyright terms
 * except that the holder is Tim Hudson (tjh@cryptsoft.com).
 *
 * Copyright remains Eric Young's, and as such any Copyright notices in
 * the code are not to be removed.
 * If this package is used in a product, Eric Young should be given attribution
 * as the author of the parts of the library used.
 * This can be in the form of a textual message at program startup or
 * in documentation (online or textual) provided with the package.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgment:
 *    "This product includes cryptographic software written by
 *     Eric Young (eay@cryptsoft.com)"
 *    The word 'cryptographic' can be left out if the rouines from the library
 *    being used are not cryptographic related :-).
 * 4. If you include any Windows specific code (or a derivative thereof) from
 *    the apps directory (application code) you must include an acknowledgment:
 *    "This product includes software written by Tim Hudson (tjh@cryptsoft.com)"
 *
 * THIS SOFTWARE IS PROVIDED BY ERIC YOUNG ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * The licence and distribution terms for any publically available version or
 * derivative of this code cannot be changed.  i.e. this code cannot simply be
 * copied and put under another distribution licence
 * [including the GNU Public Licence.]
 */

#ifndef HEADER_BN_H
#define HEADER_BN_H

#ifdef DOT11_SAE_OPENSSL_BN

typedef struct bignum_ctx BN_CTX;
typedef struct bn_gencb_st BN_GENCB;
typedef struct bignum_st BIGNUM;
typedef struct bn_mont_ctx_st BN_MONT_CTX;
typedef struct bn_recp_ctx_st BN_RECP_CTX;



#include "rtmp_type.h"
#include "rt_config.h"

typedef UINT32 BN_ULONG;

struct bignum_st {
	BN_ULONG *d;                /* Pointer to an array of 'BN_BITS2' bit chunks. */
	int top;                    /* Index of last used d +1. */
	/* The next are internal book keeping for bn_expand. */
	int dmax;                   /* Size of the d array. */
	int neg;                    /* one if the number is negative */
	int flags;
};

/* Used for montgomery multiplication */
struct bn_mont_ctx_st {
	int ri;                     /* number of bits in R */
	BIGNUM RR;                  /* used to convert to montgomery form */
	BIGNUM N;                   /* The modulus */
	BIGNUM Ni; /* R*(1/R mod N) - N*Ni = 1 (Ni is only stored for bignum algorithm) */
	BN_ULONG n0[2]; /* least significant word(s) of Ni; (type changed with 0.9.9, was "BN_ULONG n0;" before) */
	int flags;
};

/*
 * Used for reciprocal division/mod functions It cannot be shared between
 * threads
 */
struct bn_recp_ctx_st {
	BIGNUM N;                   /* the divisor */
	BIGNUM Nr;                  /* the reciprocal */
	int num_bits;
	int shift;
	int flags;
};

/* Used for slow "generation" functions. */
struct bn_gencb_st {
	unsigned int ver;           /* To handle binary (in)compatibility */
	void *arg;                  /* callback-specific data */
	union {
		/* if (ver==1) - handles old style callbacks */
		void (*cb_1) (int, int, void *);
		/* if (ver==2) - new callback style */
		int (*cb_2) (int, int, BN_GENCB *);
	} cb;
};

/* How many bignums are in each "pool item"; */
#define BN_CTX_POOL_SIZE        16
/* The stack frame info is resizing, set a first-time expansion size; */
#define BN_CTX_START_FRAMES     32

typedef struct bignum_pool_item {
	/* The bignum values */
	BIGNUM vals[BN_CTX_POOL_SIZE];
	/* Linked-list admin */
	struct bignum_pool_item *prev, *next;
} BN_POOL_ITEM;

typedef struct bignum_pool {
	/* Linked-list admin */
	BN_POOL_ITEM *head;
	BN_POOL_ITEM *curr;
	BN_POOL_ITEM *tail;
	/* Stack depth and allocation size */
	unsigned used;
	unsigned size;
} BN_POOL;

/* A wrapper to manage the "stack frames" */
typedef struct bignum_ctx_stack {
	/* Array of indexes into the bignum stack */
	unsigned int *indexes;
	/* Number of stack frames, and the size of the allocated array */
	unsigned int depth;
	unsigned int size;
} BN_STACK;



/* The opaque BN_CTX type */
struct bignum_ctx {
	/* The bignum bundles */
	BN_POOL pool;
	/* The "stack frames", if you will */
	BN_STACK stack;
	/* The number of bignums currently assigned */
	unsigned int used;
	/* Depth of stack overflow */
	int err_stack;
	/* Block "gets" until an "end" (compatibility behaviour) */
	int too_many;
	/* Flags. */
	int flags;
};



#  define bn_pollute(a)
#  define bn_check_top(a)
#  define bn_fix_top(a)           bn_correct_top(a)
#  define bn_check_size(bn, bits)
#  define bn_wcheck_size(bn, words)

# define BNerr(f, r)

#define BN_BYTES (4)
#define BN_MASK2 (~((BN_ULONG)0))
#define BN_MASK2l (0xffff)
#define BN_MASK2h1	(0xffff8000L)
#define BN_MASK2h	(0xffff0000L)
#define BN_BITS4	16



# define BN_BITS2       (BN_BYTES * 8)
# define BN_BITS        (BN_BITS2 * 2)
# define BN_TBIT        ((ULONG)1 << (BN_BITS2 - 1))

# define BN_FLG_MALLOCED         0x01
# define BN_FLG_STATIC_DATA      0x02

/*
 * avoid leaking exponent information through timing,
 * BN_mod_exp_mont() will call BN_mod_exp_mont_consttime,
 * BN_div() will call BN_div_no_branch,
 * BN_mod_inverse() will call BN_mod_inverse_no_branch.
 */
# define BN_FLG_CONSTTIME        0x04
# define BN_FLG_SECURE           0x08

#  define BN_FLG_EXP_CONSTTIME BN_FLG_CONSTTIME
#  define BN_FLG_FREE            0x8000	/* used for debugging */


/* Values for |top| in BN_rand() */
#define BN_RAND_TOP_ANY    -1
#define BN_RAND_TOP_ONE     0
#define BN_RAND_TOP_TWO     1

/* Values for |bottom| in BN_rand() */
#define BN_RAND_BOTTOM_ANY  0
#define BN_RAND_BOTTOM_ODD  1

#define BN_prime_checks 0 /* default: select number of iterations based on the size of the number */

/*
 * number of Miller-Rabin iterations for an error rate of less than 2^-80 for
 * random 'b'-bit input, b >= 100 (taken from table 4.4 in the Handbook of
 * Applied Cryptography [Menezes, van Oorschot, Vanstone; CRC Press 1996];
 * original paper: Damgaard, Landrock, Pomerance: Average case error
 * estimates for the strong probable prime test. -- Math. Comp. 61 (1993)
 * 177-194)
 */
# define BN_prime_checks_for_size(b) ((b) >= 1300 ?  2 : \
									  (b) >=  850 ?  3 : \
									  (b) >=  650 ?  4 : \
									  (b) >=  550 ?  5 : \
									  (b) >=  450 ?  6 : \
									  (b) >=  400 ?  7 : \
									  (b) >=  350 ?  8 : \
									  (b) >=  300 ?  9 : \
									  (b) >=  250 ? 12 : \
									  (b) >=  200 ? 15 : \
									  (b) >=  150 ? 18 : \
									  /* b >= 100 */ 27)

#define BN_num_bytes(a) ((BN_num_bits(a) + 7) / 8)

#define BN_one(a)       (BN_set_word((a), 1))

#define BN_zero(a)      BN_zero_ex(a)

/* BN_BLINDING flags */
# define BN_BLINDING_NO_UPDATE   0x00000001
# define BN_BLINDING_NO_RECREATE 0x00000002


/*-
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
# define BN_window_bits_for_exponent_size(b) \
	((b) > 671 ? 6 : \
	 (b) > 239 ? 5 : \
	 (b) >  79 ? 4 : \
	 (b) >  23 ? 3 : 1)

/*
 * BN_mod_exp_mont_conttime is based on the assumption that the L1 data cache
 * line width of the target processor is at least the following value.
 */
#define MOD_EXP_CTIME_MIN_CACHE_LINE_WIDTH      (64)
#define MOD_EXP_CTIME_MIN_CACHE_LINE_MASK       (MOD_EXP_CTIME_MIN_CACHE_LINE_WIDTH - 1)

/*
 * Window sizes optimized for fixed window size modular exponentiation
 * algorithm (BN_mod_exp_mont_consttime). To achieve the security goals of
 * BN_mode_exp_mont_consttime, the maximum size of the window must not exceed
 * log_2(MOD_EXP_CTIME_MIN_CACHE_LINE_WIDTH). Window size thresholds are
 * defined for cache line sizes of 32 and 64, cache line sizes where
 * log_2(32)=5 and log_2(64)=6 respectively. A window size of 7 should only be
 * used on processors that have a 128 byte or greater cache line size.
 */
# if MOD_EXP_CTIME_MIN_CACHE_LINE_WIDTH == 64

#  define BN_window_bits_for_ctime_exponent_size(b) \
	((b) > 937 ? 6 : \
	 (b) > 306 ? 5 : \
	 (b) >  89 ? 4 : \
	 (b) >  22 ? 3 : 1)
#  define BN_MAX_WINDOW_BITS_FOR_CTIME_EXPONENT_SIZE    (6)

# elif MOD_EXP_CTIME_MIN_CACHE_LINE_WIDTH == 32

#  define BN_window_bits_for_ctime_exponent_size(b) \
	((b) > 306 ? 5 : \
	 (b) >  89 ? 4 : \
	 (b) >  22 ? 3 : 1)
#  define BN_MAX_WINDOW_BITS_FOR_CTIME_EXPONENT_SIZE    (5)

# endif


void BN_set_flags(BIGNUM *b, int n);
int BN_get_flags(const BIGNUM *b, int n);
/*
 * get a clone of a BIGNUM with changed flags, for *temporary* use only (the
 * two BIGNUMs cannot be used in parallel!). Also only for *read only* use. The
 * value |dest| should be a newly allocated BIGNUM obtained via BN_new() that
 * has not been otherwise initialised or used.
 */
void BN_with_flags(BIGNUM *dest, const BIGNUM *b, int flags);


void BN_zero_ex(BIGNUM *a);
int BN_abs_is_word(const BIGNUM *a, const BN_ULONG w);
int BN_is_zero(const BIGNUM *a);
int BN_is_one(const BIGNUM *a);
int BN_is_word(const BIGNUM *a, const BN_ULONG w);
int BN_is_odd(const BIGNUM *a);

int BN_num_bits(const BIGNUM *a);
int BN_num_bits_word(BN_ULONG l);
int BN_security_bits(int L, int N);

const BIGNUM *BN_value_one(void);

BIGNUM *BN_new(void);
BIGNUM *BN_secure_new(void);
void BN_clear_free(BIGNUM *a);
BIGNUM *BN_dup(const BIGNUM *a);
BIGNUM *BN_copy(BIGNUM *a, const BIGNUM *b);
void BN_swap(BIGNUM *a, BIGNUM *b);
BIGNUM *BN_bin2bn(const unsigned char *s, int len, BIGNUM *ret);
int BN_bn2bin(const BIGNUM *a, unsigned char *to);
int BN_bn2binpad(const BIGNUM *a, unsigned char *to, int tolen);
BIGNUM *BN_lebin2bn(const unsigned char *s, int len, BIGNUM *ret);
int BN_bn2lebinpad(const BIGNUM *a, unsigned char *to, int tolen);
BIGNUM *bn_wexpand(BIGNUM *a, int words);
void bn_correct_top(BIGNUM *a);
int BN_set_word(BIGNUM *a, BN_ULONG w);

int BN_mod_mul_montgomery(BIGNUM *r, const BIGNUM *a, const BIGNUM *b,
						  BN_MONT_CTX *mont, BN_CTX *ctx);
int BN_to_montgomery(BIGNUM *r, const BIGNUM *a, BN_MONT_CTX *mont,
					 BN_CTX *ctx);
int BN_from_montgomery(BIGNUM *r, const BIGNUM *a, BN_MONT_CTX *mont,
					   BN_CTX *ctx);

BN_CTX *BN_CTX_new(void);
BN_CTX *BN_CTX_secure_new(void);
void BN_CTX_free(BN_CTX *c);
void BN_CTX_start(BN_CTX *ctx);
BIGNUM *BN_CTX_get(BN_CTX *ctx);
void BN_CTX_end(BN_CTX *ctx);
void BN_MONT_CTX_init(BN_MONT_CTX *ctx);
BN_MONT_CTX *BN_MONT_CTX_new(void);
int BN_MONT_CTX_set(BN_MONT_CTX *mont, const BIGNUM *mod, BN_CTX *ctx);
void BN_MONT_CTX_free(BN_MONT_CTX *mont);

void BN_STACK_init(BN_STACK *);
void BN_STACK_finish(BN_STACK *);
int BN_STACK_push(BN_STACK *, unsigned int);
unsigned int BN_STACK_pop(BN_STACK *);
void BN_POOL_init(BN_POOL *);
void BN_POOL_finish(BN_POOL *);
BIGNUM *BN_POOL_get(BN_POOL *, int);
void BN_POOL_release(BN_POOL *, unsigned int);

int BN_mul(BIGNUM *r, const BIGNUM *a, const BIGNUM *b, BN_CTX *ctx);
int BN_sqr(BIGNUM *r, const BIGNUM *a, BN_CTX *ctx);
void bn_sqr_normal(BN_ULONG *r, const BN_ULONG *a, int n, BN_ULONG *tmp);

BIGNUM *BN_mod_inverse(BIGNUM *in,
					   const BIGNUM *a, const BIGNUM *n, BN_CTX *ctx);

int BN_lshift(BIGNUM *r, const BIGNUM *a, int n);
int BN_lshift1(BIGNUM *r, const BIGNUM *a);
int BN_rshift(BIGNUM *r, const BIGNUM *a, int n);
int BN_rshift1(BIGNUM *r, const BIGNUM *a);

int BN_nnmod(BIGNUM *r, const BIGNUM *m, const BIGNUM *d, BN_CTX *ctx);
int BN_mod_add(BIGNUM *r, const BIGNUM *a, const BIGNUM *b, const BIGNUM *m,
			   BN_CTX *ctx);
/*
 * BN_mod_add variant that may be used if both a and b are non-negative and
 * less than m
 */
int BN_mod_add_quick(BIGNUM *r, const BIGNUM *a, const BIGNUM *b,
					 const BIGNUM *m);
int BN_mod_sub(BIGNUM *r, const BIGNUM *a, const BIGNUM *b, const BIGNUM *m,
			   BN_CTX *ctx);
/*
 * BN_mod_sub variant that may be used if both a and b are non-negative and
 * less than m
 */
int BN_mod_sub_quick(BIGNUM *r, const BIGNUM *a, const BIGNUM *b,
					 const BIGNUM *m);
/* slow but works */
int BN_mod_mul(BIGNUM *r, const BIGNUM *a, const BIGNUM *b, const BIGNUM *m,
			   BN_CTX *ctx);
int BN_mod_sqr(BIGNUM *r, const BIGNUM *a, const BIGNUM *m, BN_CTX *ctx);
int BN_mod_lshift1(BIGNUM *r, const BIGNUM *a, const BIGNUM *m, BN_CTX *ctx);
/*
 * BN_mod_lshift1 variant that may be used if a is non-negative and less than
 * m
 */
int BN_mod_lshift1_quick(BIGNUM *r, const BIGNUM *a, const BIGNUM *m);
int BN_mod_lshift(BIGNUM *r, const BIGNUM *a, int n, const BIGNUM *m,
				  BN_CTX *ctx);
/*
 * BN_mod_lshift variant that may be used if a is non-negative and less than
 * m
 */
int BN_mod_lshift_quick(BIGNUM *r, const BIGNUM *a, int n, const BIGNUM *m);

int BN_add(BIGNUM *r, const BIGNUM *a, const BIGNUM *b);
/* unsigned add of b to a */
int BN_uadd(BIGNUM *r, const BIGNUM *a, const BIGNUM *b);
/* unsigned subtraction of b from a, a must be larger than b. */
int BN_usub(BIGNUM *r, const BIGNUM *a, const BIGNUM *b);
int BN_sub(BIGNUM *r, const BIGNUM *a, const BIGNUM *b);

BN_ULONG BN_mod_word(const BIGNUM *a, BN_ULONG w);
BN_ULONG BN_div_word(BIGNUM *a, BN_ULONG w);
int BN_add_word(BIGNUM *a, BN_ULONG w);
int BN_sub_word(BIGNUM *a, BN_ULONG w);
int BN_mul_word(BIGNUM *a, BN_ULONG w);

#define BN_mod(rem, m, d, ctx) BN_div(NULL, (rem), (m), (d), (ctx))
int BN_div(BIGNUM *dv, BIGNUM *rm, const BIGNUM *num, const BIGNUM *divisor,
		   BN_CTX *ctx);


int BN_exp(BIGNUM *r, const BIGNUM *a, const BIGNUM *p, BN_CTX *ctx);

int BN_mod_exp(BIGNUM *r, const BIGNUM *a, const BIGNUM *p,
			   const BIGNUM *m, BN_CTX *ctx);
int BN_mod_exp_mont(BIGNUM *r, const BIGNUM *a, const BIGNUM *p,
					const BIGNUM *m, BN_CTX *ctx, BN_MONT_CTX *m_ctx);
int BN_mod_exp_mont_consttime(BIGNUM *rr, const BIGNUM *a, const BIGNUM *p,
							  const BIGNUM *m, BN_CTX *ctx,
							  BN_MONT_CTX *in_mont);
int BN_mod_exp_mont_word(BIGNUM *r, BN_ULONG a, const BIGNUM *p,
						 const BIGNUM *m, BN_CTX *ctx, BN_MONT_CTX *m_ctx);
int BN_mod_exp2_mont(BIGNUM *r, const BIGNUM *a1, const BIGNUM *p1,
					 const BIGNUM *a2, const BIGNUM *p2, const BIGNUM *m,
					 BN_CTX *ctx, BN_MONT_CTX *m_ctx);
int BN_mod_exp_simple(BIGNUM *r, const BIGNUM *a, const BIGNUM *p,
					  const BIGNUM *m, BN_CTX *ctx);

int BN_div_recp(BIGNUM *dv, BIGNUM *rem, const BIGNUM *m,
				BN_RECP_CTX *recp, BN_CTX *ctx);

int BN_reciprocal(BIGNUM *r, const BIGNUM *m, int len, BN_CTX *ctx);

int BN_mod_mul_reciprocal(BIGNUM *r, const BIGNUM *x, const BIGNUM *y,
						  BN_RECP_CTX *recp, BN_CTX *ctx);
int BN_mod_exp_recp(BIGNUM *r, const BIGNUM *a, const BIGNUM *p,
					const BIGNUM *m, BN_CTX *ctx);

/** BN_set_negative sets sign of a BIGNUM
 * \param  b  pointer to the BIGNUM object
 * \param  n  0 if the BIGNUM b should be positive and a value != 0 otherwise
 */
void BN_set_negative(BIGNUM *b, int n);

/** BN_is_negative returns 1 if the BIGNUM is negative
 * \param  b  pointer to the BIGNUM object
 * \return 1 if a < 0 and 0 otherwise
 */
int BN_is_negative(const BIGNUM *b);

int BN_cmp(const BIGNUM *a, const BIGNUM *b);
void BN_free(BIGNUM *a);
int BN_ucmp(const BIGNUM *a, const BIGNUM *b);

int BN_rand(BIGNUM *rnd, int bits, int top, int bottom);
int BN_pseudo_rand(BIGNUM *rnd, int bits, int top, int bottom);
int BN_rand_range(BIGNUM *rnd, const BIGNUM *range);
char *BN_bn2hex(const BIGNUM *a);

BIGNUM *BN_mod_sqrt(BIGNUM *ret,
					const BIGNUM *a, const BIGNUM *n, BN_CTX *ctx);

int BN_kronecker(const BIGNUM *a, const BIGNUM *b, BN_CTX *ctx);


/* BEGIN ERROR CODES */
/*
 * The following lines are auto generated by the script mkerr.pl. Any changes
 * made after this point may be overwritten when the script is next run.
 */

int ERR_load_BN_strings(void);

/* Error codes for the BN functions. */

/* Function codes. */
# define BN_F_BNRAND                                      127
# define BN_F_BN_BLINDING_CONVERT_EX                      100
# define BN_F_BN_BLINDING_CREATE_PARAM                    128
# define BN_F_BN_BLINDING_INVERT_EX                       101
# define BN_F_BN_BLINDING_NEW                             102
# define BN_F_BN_BLINDING_UPDATE                          103
# define BN_F_BN_BN2DEC                                   104
# define BN_F_BN_BN2HEX                                   105
# define BN_F_BN_COMPUTE_WNAF                             142
# define BN_F_BN_CTX_GET                                  116
# define BN_F_BN_CTX_NEW                                  106
# define BN_F_BN_CTX_START                                129
# define BN_F_BN_DIV                                      107
# define BN_F_BN_DIV_RECP                                 130
# define BN_F_BN_EXP                                      123
# define BN_F_BN_EXPAND_INTERNAL                          120
# define BN_F_BN_GENCB_NEW                                143
# define BN_F_BN_GENERATE_DSA_NONCE                       140
# define BN_F_BN_GENERATE_PRIME_EX                        141
# define BN_F_BN_GF2M_MOD                                 131
# define BN_F_BN_GF2M_MOD_EXP                             132
# define BN_F_BN_GF2M_MOD_MUL                             133
# define BN_F_BN_GF2M_MOD_SOLVE_QUAD                      134
# define BN_F_BN_GF2M_MOD_SOLVE_QUAD_ARR                  135
# define BN_F_BN_GF2M_MOD_SQR                             136
# define BN_F_BN_GF2M_MOD_SQRT                            137
# define BN_F_BN_LSHIFT                                   145
# define BN_F_BN_MOD_EXP2_MONT                            118
# define BN_F_BN_MOD_EXP_MONT                             109
# define BN_F_BN_MOD_EXP_MONT_CONSTTIME                   124
# define BN_F_BN_MOD_EXP_MONT_WORD                        117
# define BN_F_BN_MOD_EXP_RECP                             125
# define BN_F_BN_MOD_EXP_SIMPLE                           126
# define BN_F_BN_MOD_INVERSE                              110
# define BN_F_BN_MOD_INVERSE_NO_BRANCH                    139
# define BN_F_BN_MOD_LSHIFT_QUICK                         119
# define BN_F_BN_MOD_SQRT                                 121
# define BN_F_BN_MPI2BN                                   112
# define BN_F_BN_NEW                                      113
# define BN_F_BN_RAND                                     114
# define BN_F_BN_RAND_RANGE                               122
# define BN_F_BN_RSHIFT                                   146
# define BN_F_BN_SET_WORDS                                144
# define BN_F_BN_USUB                                     115

/* Reason codes. */
# define BN_R_ARG2_LT_ARG3                                100
# define BN_R_BAD_RECIPROCAL                              101
# define BN_R_BIGNUM_TOO_LONG                             114
# define BN_R_BITS_TOO_SMALL                              118
# define BN_R_CALLED_WITH_EVEN_MODULUS                    102
# define BN_R_DIV_BY_ZERO                                 103
# define BN_R_ENCODING_ERROR                              104
# define BN_R_EXPAND_ON_STATIC_BIGNUM_DATA                105
# define BN_R_INPUT_NOT_REDUCED                           110
# define BN_R_INVALID_LENGTH                              106
# define BN_R_INVALID_RANGE                               115
# define BN_R_INVALID_SHIFT                               119
# define BN_R_NOT_A_SQUARE                                111
# define BN_R_NOT_INITIALIZED                             107
# define BN_R_NO_INVERSE                                  108
# define BN_R_NO_SOLUTION                                 116
# define BN_R_PRIVATE_KEY_TOO_LARGE                       117
# define BN_R_P_IS_NOT_PRIME                              112
# define BN_R_TOO_MANY_ITERATIONS                         113
# define BN_R_TOO_MANY_TEMPORARY_VARIABLES                109

/* fatal error */
# define ERR_R_FATAL                             64
# define ERR_R_MALLOC_FAILURE                    (1|ERR_R_FATAL)
# define ERR_R_SHOULD_NOT_HAVE_BEEN_CALLED       (2|ERR_R_FATAL)
# define ERR_R_PASSED_NULL_PARAMETER             (3|ERR_R_FATAL)
# define ERR_R_INTERNAL_ERROR                    (4|ERR_R_FATAL)
# define ERR_R_DISABLED                          (5|ERR_R_FATAL)
# define ERR_R_INIT_FAIL                         (6|ERR_R_FATAL)
# define ERR_R_PASSED_INVALID_ARGUMENT           (7)

#endif

#endif

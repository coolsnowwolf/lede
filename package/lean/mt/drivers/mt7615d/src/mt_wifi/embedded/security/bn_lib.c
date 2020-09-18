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


#ifdef DOT11_SAE_OPENSSL_BN

#include "security/bn.h"

static BIGNUM *BN_mod_inverse_no_branch(BIGNUM *in,
						const BIGNUM *a, const BIGNUM *n,
						BN_CTX *ctx);


void OPENSSL_cleanse(void *ptr, ULONG len)
{
	memset(ptr, 0, len);
}

const BIGNUM *BN_value_one(void)
{
	static const BN_ULONG data_one = 1L;
	static const BIGNUM const_one = {(BN_ULONG *)&data_one, 1, 1, 0, BN_FLG_STATIC_DATA};
	return &const_one;
}

int BN_num_bits_word(BN_ULONG l)
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

int BN_num_bits(const BIGNUM *a)
{
	int i = a->top - 1;
	bn_check_top(a);

	if (BN_is_zero(a))
		return 0;

	return ((i * BN_BITS2) + BN_num_bits_word(a->d[i]));
}

static void bn_free_d(BIGNUM *a)
{
	if (a->d)
		os_free_mem(a->d);
}


void BN_clear_free(BIGNUM *a)
{
	int i;

	if (a == NULL)
		return;

	bn_check_top(a);

	if (a->d != NULL) {
		memset(a->d, 0, a->dmax * sizeof(a->d[0]));

		if (!BN_get_flags(a, BN_FLG_STATIC_DATA))
			bn_free_d(a);
	}

	i = BN_get_flags(a, BN_FLG_MALLOCED);
	memset(a, 0, sizeof(*a));

	if (i)
		os_free_mem(a);
}

void BN_free(BIGNUM *a)
{
	if (a == NULL)
		return;

	bn_check_top(a);

	if (!BN_get_flags(a, BN_FLG_STATIC_DATA))
		bn_free_d(a);

	if (a->flags & BN_FLG_MALLOCED)
		os_free_mem(a);
	else {
		a->d = NULL;
	}
}

void bn_init(BIGNUM *a)
{
	static BIGNUM nilbn;
	*a = nilbn;
	bn_check_top(a);
}

BIGNUM *BN_new(void)
{
	BIGNUM *ret;
	os_alloc_mem(NULL, (UCHAR **)&ret, sizeof(struct bignum_st));

	if (ret == NULL)
		return NULL;

	ret->flags = BN_FLG_MALLOCED;
	/* ellis */
	ret->top = 0;
	ret->dmax = 0;
	ret->d = NULL;
	ret->neg = 0;
	bn_check_top(ret);
	return ret;
}

BIGNUM *BN_secure_new(void)
{
	BIGNUM *ret = BN_new();

	if (ret != NULL)
		ret->flags |= BN_FLG_SECURE;

	return ret;
}

/* This is used by bn_expand2() */
/* The caller MUST check that words > b->dmax before calling this */
static BN_ULONG *bn_expand_internal(const BIGNUM *b, int words)
{
	BN_ULONG *A, *a = NULL;
	const BN_ULONG *B;
	int i;
	bn_check_top(b);

	if (words > (INT_MAX / (4 * BN_BITS2))) {
		BNerr(BN_F_BN_EXPAND_INTERNAL, BN_R_BIGNUM_TOO_LONG);
		return NULL;
	}

	if (BN_get_flags(b, BN_FLG_STATIC_DATA)) {
		BNerr(BN_F_BN_EXPAND_INTERNAL, BN_R_EXPAND_ON_STATIC_BIGNUM_DATA);
		return NULL;
	}

	os_alloc_mem(NULL, (UCHAR **)&A, sizeof(BN_ULONG)*words);
	memset(A, 0, sizeof(BN_ULONG)*words);
	a = A;

	if (A == NULL) {
		BNerr(BN_F_BN_EXPAND_INTERNAL, ERR_R_MALLOC_FAILURE);
		return NULL;
	}

	B = b->d;

	/* Check if the previous number needs to be copied */
	if (B != NULL) {
		for (i = b->top >> 2; i > 0; i--, A += 4, B += 4) {
			/*
			 * The fact that the loop is unrolled
			 * 4-wise is a tribute to Intel. It's
			 * the one that doesn't have enough
			 * registers to accommodate more data.
			 * I'd unroll it 8-wise otherwise:-)
			 *
			 *              <appro@fy.chalmers.se>
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

		/* fall thru */
		case 2:
			A[1] = B[1];

		/* fall thru */
		case 1:
			A[0] = B[0];

		/* fall thru */
		case 0:
			/* Without the "case 0" some old optimizers got this wrong. */
			;
		}
	}

	return a;
}


/*
 * This is an internal function that should not be used in applications. It
 * ensures that 'b' has enough room for a 'words' word number and initialises
 * any unused part of b->d with leading zeros. It is mostly used by the
 * various BIGNUM routines. If there is an error, NULL is returned. If not,
 * 'b' is returned.
 */

BIGNUM *bn_expand2(BIGNUM *b, int words)
{
	bn_check_top(b);

	if (words > b->dmax) {
		BN_ULONG *a = bn_expand_internal(b, words);

		if (!a)
			return NULL;

		if (b->d) {
			memset(b->d, 0, b->dmax * sizeof(b->d[0]));
			bn_free_d(b);
		}

		b->d = a;
		b->dmax = words;
	}

	bn_check_top(b);
	return b;
}

static inline BIGNUM *bn_expand(BIGNUM *a, int bits)
{
	if (bits > (INT_MAX - BN_BITS2 + 1))
		return NULL;

	if (((bits + BN_BITS2 - 1) / BN_BITS2) <= (a)->dmax)
		return a;

	return bn_expand2((a), (bits + BN_BITS2 - 1) / BN_BITS2);
}


BIGNUM *BN_dup(const BIGNUM *a)
{
	BIGNUM *t;

	if (a == NULL)
		return NULL;

	bn_check_top(a);
	t = BN_get_flags(a, BN_FLG_SECURE) ? BN_secure_new() : BN_new();

	if (t == NULL)
		return NULL;

	if (!BN_copy(t, a)) {
		BN_free(t);
		return NULL;
	}

	bn_check_top(t);
	return t;
}

BIGNUM *BN_copy(BIGNUM *a, const BIGNUM *b)
{
	int i;
	BN_ULONG *A;
	const BN_ULONG *B;
	bn_check_top(b);

	if (a == b)
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

	/* ultrix cc workaround, see comments in bn_expand_internal */
	switch (b->top & 3) {
	case 3:
		A[2] = B[2];

	/* fall thru */
	case 2:
		A[1] = B[1];

	/* fall thru */
	case 1:
		A[0] = B[0];

	/* fall thru */
	case 0:
		;
	}


	if (BN_get_flags(b, BN_FLG_CONSTTIME) != 0)
		BN_set_flags(a, BN_FLG_CONSTTIME);

	a->top = b->top;
	a->neg = b->neg;
	bn_check_top(a);
	return a;
}

void BN_swap(BIGNUM *a, BIGNUM *b)
{
	int flags_old_a, flags_old_b;
	BN_ULONG *tmp_d;
	int tmp_top, tmp_dmax, tmp_neg;
	bn_check_top(a);
	bn_check_top(b);
	flags_old_a = a->flags;
	flags_old_b = b->flags;
	tmp_d = a->d;
	tmp_top = a->top;
	tmp_dmax = a->dmax;
	tmp_neg = a->neg;
	a->d = b->d;
	a->top = b->top;
	a->dmax = b->dmax;
	a->neg = b->neg;
	b->d = tmp_d;
	b->top = tmp_top;
	b->dmax = tmp_dmax;
	b->neg = tmp_neg;
	a->flags =
		(flags_old_a & BN_FLG_MALLOCED) | (flags_old_b & BN_FLG_STATIC_DATA);
	b->flags =
		(flags_old_b & BN_FLG_MALLOCED) | (flags_old_a & BN_FLG_STATIC_DATA);
	bn_check_top(a);
	bn_check_top(b);
}

void BN_clear(BIGNUM *a)
{
	bn_check_top(a);

	if (a->d != NULL)
		memset(a->d, 0, sizeof(*a->d) * a->dmax);

	a->top = 0;
	a->neg = 0;
}

BN_ULONG BN_get_word(const BIGNUM *a)
{
	if (a->top > 1)
		return BN_MASK2;
	else if (a->top == 1)
		return a->d[0];

	/* a->top == 0 */
	return 0;
}

int BN_set_word(BIGNUM *a, BN_ULONG w)
{
	bn_check_top(a);

	if (bn_expand(a, (int)sizeof(BN_ULONG) * 8) == NULL)
		return 0;

	a->neg = 0;
	a->d[0] = w;
	a->top = (w ? 1 : 0);
	bn_check_top(a);
	return 1;
}

BIGNUM *BN_bin2bn(const unsigned char *s, int len, BIGNUM *ret)
{
	unsigned int i, m;
	unsigned int n;
	BN_ULONG l;
	BIGNUM *bn = NULL;

	if (ret == NULL)
		ret = bn = BN_new();

	if (ret == NULL)
		return NULL;

	bn_check_top(ret);

	/* Skip leading zero's. */
	for ( ; len > 0 && *s == 0; s++, len--)
		continue;

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
	ret->neg = 0;
	l = 0;

	while (n--) {
		l = (l << 8L) | *(s++);

		if (m-- == 0) {
			ret->d[--i] = l;
			l = 0;
			m = BN_BYTES - 1;
		}
	}

	/*
	 * need to call this due to clear byte at top if avoiding having the top
	 * bit set (-ve number)
	 */
	bn_correct_top(ret);
	return ret;
}

/* ignore negative */
static int bn2binpad(const BIGNUM *a, unsigned char *to, int tolen)
{
	int i;
	BN_ULONG l;
	bn_check_top(a);
	i = BN_num_bytes(a);

	if (tolen == -1)
		tolen = i;
	else if (tolen < i)
		return -1;

	/* Add leading zeroes if necessary */
	if (tolen > i) {
		memset(to, 0, tolen - i);
		to += tolen - i;
	}

	while (i--) {
		l = a->d[i / BN_BYTES];
		*(to++) = (unsigned char)(l >> (8 * (i % BN_BYTES))) & 0xff;
	}

	return tolen;
}

int BN_bn2binpad(const BIGNUM *a, unsigned char *to, int tolen)
{
	if (tolen < 0)
		return -1;

	return bn2binpad(a, to, tolen);
}

int BN_bn2bin(const BIGNUM *a, unsigned char *to)
{
	return bn2binpad(a, to, -1);
}

BIGNUM *BN_lebin2bn(const unsigned char *s, int len, BIGNUM *ret)
{
	unsigned int i, m;
	unsigned int n;
	BN_ULONG l;
	BIGNUM *bn = NULL;

	if (ret == NULL)
		ret = bn = BN_new();

	if (ret == NULL)
		return NULL;

	bn_check_top(ret);
	s += len;

	/* Skip trailing zeroes. */
	for ( ; len > 0 && s[-1] == 0; s--, len--)
		continue;

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
	ret->neg = 0;
	l = 0;

	while (n--) {
		s--;
		l = (l << 8L) | *s;

		if (m-- == 0) {
			ret->d[--i] = l;
			l = 0;
			m = BN_BYTES - 1;
		}
	}

	/*
	 * need to call this due to clear byte at top if avoiding having the top
	 * bit set (-ve number)
	 */
	bn_correct_top(ret);
	return ret;
}

int BN_bn2lebinpad(const BIGNUM *a, unsigned char *to, int tolen)
{
	int i;
	BN_ULONG l;
	bn_check_top(a);
	i = BN_num_bytes(a);

	if (tolen < i)
		return -1;

	/* Add trailing zeroes if necessary */
	if (tolen > i)
		memset(to + i, 0, tolen - i);

	to += i;

	while (i--) {
		l = a->d[i / BN_BYTES];
		to--;
		*to = (unsigned char)(l >> (8 * (i % BN_BYTES))) & 0xff;
	}

	return tolen;
}

int BN_ucmp(const BIGNUM *a, const BIGNUM *b)
{
	int i;
	BN_ULONG t1, t2, *ap, *bp;
	bn_check_top(a);
	bn_check_top(b);
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

int BN_cmp(const BIGNUM *a, const BIGNUM *b)
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

	bn_check_top(a);
	bn_check_top(b);

	if (a->neg != b->neg) {
		if (a->neg)
			return -1;
		else
			return 1;
	}

	if (a->neg == 0) {
		gt = 1;
		lt = -1;
	} else {
		gt = -1;
		lt = 1;
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

int BN_set_bit(BIGNUM *a, int n)
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
	bn_check_top(a);
	return 1;
}

int BN_clear_bit(BIGNUM *a, int n)
{
	int i, j;
	bn_check_top(a);

	if (n < 0)
		return 0;

	i = n / BN_BITS2;
	j = n % BN_BITS2;

	if (a->top <= i)
		return 0;

	a->d[i] &= (~(((BN_ULONG)1) << j));
	bn_correct_top(a);
	return 1;
}

int BN_is_bit_set(const BIGNUM *a, int n)
{
	int i, j;
	bn_check_top(a);

	if (n < 0)
		return 0;

	i = n / BN_BITS2;
	j = n % BN_BITS2;

	if (a->top <= i)
		return 0;

	return (int)(((a->d[i]) >> j) & ((BN_ULONG)1));
}

int BN_mask_bits(BIGNUM *a, int n)
{
	int b, w;
	bn_check_top(a);

	if (n < 0)
		return 0;

	w = n / BN_BITS2;
	b = n % BN_BITS2;

	if (w >= a->top)
		return 0;

	if (b == 0)
		a->top = w;
	else {
		a->top = w + 1;
		a->d[w] &= ~(BN_MASK2 << b);
	}

	bn_correct_top(a);
	return 1;
}

void BN_set_negative(BIGNUM *a, int b)
{
	if (b && !BN_is_zero(a))
		a->neg = 1;
	else
		a->neg = 0;
}

int bn_cmp_words(const BN_ULONG *a, const BN_ULONG *b, int n)
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

/*
 * Here follows a specialised variants of bn_cmp_words().  It has the
 * capability of performing the operation on arrays of different sizes. The
 * sizes of those arrays is expressed through cl, which is the common length
 * ( basically, min(len(a),len(b)) ), and dl, which is the delta between the
 * two lengths, calculated as len(a)-len(b). All lengths are the number of
 * BN_ULONGs...
 */

int bn_cmp_part_words(const BN_ULONG *a, const BN_ULONG *b, int cl, int dl)
{
	int n, i;
	n = cl - 1;

	if (dl < 0) {
		for (i = dl; i < 0; i++) {
			if (b[n - i] != 0)
				return -1;      /* a < b */
		}
	}

	if (dl > 0) {
		for (i = dl; i > 0; i--) {
			if (a[n + i] != 0)
				return 1;       /* a > b */
		}
	}

	return bn_cmp_words(a, b, cl);
}

/*
 * Constant-time conditional swap of a and b.
 * a and b are swapped if condition is not 0.  The code assumes that at most one bit of condition is set.
 * nwords is the number of words to swap.  The code assumes that at least nwords are allocated in both a and b,
 * and that no more than nwords are used by either a or b.
 * a and b cannot be the same number
 */
void BN_consttime_swap(BN_ULONG condition, BIGNUM *a, BIGNUM *b, int nwords)
{
	BN_ULONG t;
	int i;
	bn_wcheck_size(a, nwords);
	bn_wcheck_size(b, nwords);
	ASSERT(a != b);
	ASSERT((condition & (condition - 1)) == 0);
	ASSERT(sizeof(BN_ULONG) >= sizeof(int));
	condition = ((condition - 1) >> (BN_BITS2 - 1)) - 1;
	t = (a->top ^ b->top) & condition;
	a->top ^= t;
	b->top ^= t;
#define BN_CONSTTIME_SWAP(ind) \
	do { \
		t = (a->d[ind] ^ b->d[ind]) & condition; \
		a->d[ind] ^= t; \
		b->d[ind] ^= t; \
	} while (0)

	switch (nwords) {
	default:
		for (i = 10; i < nwords; i++)
			BN_CONSTTIME_SWAP(i);

	/* Fallthrough */
	case 10:
		BN_CONSTTIME_SWAP(9);   /* Fallthrough */

	case 9:
		BN_CONSTTIME_SWAP(8);   /* Fallthrough */

	case 8:
		BN_CONSTTIME_SWAP(7);   /* Fallthrough */

	case 7:
		BN_CONSTTIME_SWAP(6);   /* Fallthrough */

	case 6:
		BN_CONSTTIME_SWAP(5);   /* Fallthrough */

	case 5:
		BN_CONSTTIME_SWAP(4);   /* Fallthrough */

	case 4:
		BN_CONSTTIME_SWAP(3);   /* Fallthrough */

	case 3:
		BN_CONSTTIME_SWAP(2);   /* Fallthrough */

	case 2:
		BN_CONSTTIME_SWAP(1);   /* Fallthrough */

	case 1:
		BN_CONSTTIME_SWAP(0);
	}

#undef BN_CONSTTIME_SWAP
}

/* Bits of security, see SP800-57 */

int BN_security_bits(int L, int N)
{
	int secbits, bits;

	if (L >= 15360)
		secbits = 256;
	else if (L >= 7680)
		secbits = 192;
	else if (L >= 3072)
		secbits = 128;
	else if (L >= 2048)
		secbits = 112;
	else if (L >= 1024)
		secbits = 80;
	else
		return 0;

	if (N == -1)
		return secbits;

	bits = N / 2;

	if (bits < 80)
		return 0;

	return bits >= secbits ? secbits : bits;
}

void BN_zero_ex(BIGNUM *a)
{
	a->top = 0;
	a->neg = 0;
}

int BN_abs_is_word(const BIGNUM *a, const BN_ULONG w)
{
	return ((a->top == 1) && (a->d[0] == w)) || ((w == 0) && (a->top == 0));
}

int BN_is_zero(const BIGNUM *a)
{
	return a->top == 0;
}

int BN_is_one(const BIGNUM *a)
{
	return BN_abs_is_word(a, 1) && !a->neg;
}

int BN_is_word(const BIGNUM *a, const BN_ULONG w)
{
	return BN_abs_is_word(a, w) && (!w || !a->neg);
}

int BN_is_odd(const BIGNUM *a)
{
	return (a->top > 0) && (a->d[0] & 1);
}

int BN_is_negative(const BIGNUM *a)
{
	return (a->neg != 0);
}

int BN_to_montgomery(BIGNUM *r, const BIGNUM *a, BN_MONT_CTX *mont,
					 BN_CTX *ctx)
{
	return BN_mod_mul_montgomery(r, a, &(mont->RR), mont, ctx);
}

void BN_with_flags(BIGNUM *dest, const BIGNUM *b, int flags)
{
	dest->d = b->d;
	dest->top = b->top;
	dest->dmax = b->dmax;
	dest->neg = b->neg;
	dest->flags = ((dest->flags & BN_FLG_MALLOCED)
				   | (b->flags & ~BN_FLG_MALLOCED)
				   | BN_FLG_STATIC_DATA | flags);
}

BN_GENCB *BN_GENCB_new(void)
{
	BN_GENCB *ret;
	os_alloc_mem(NULL, (UCHAR **)&ret, sizeof(BN_GENCB));

	if (ret == NULL)
		return NULL;

	return ret;
}

void BN_GENCB_free(BN_GENCB *cb)
{
	if (cb == NULL)
		return;

	os_free_mem(cb);
}

void BN_set_flags(BIGNUM *b, int n)
{
	b->flags |= n;
}

int BN_get_flags(const BIGNUM *b, int n)
{
	return b->flags & n;
}

/* Populate a BN_GENCB structure with an "old"-style callback */
void BN_GENCB_set_old(BN_GENCB *gencb, void (*callback) (int, int, void *),
					  void *cb_arg)
{
	BN_GENCB *tmp_gencb = gencb;
	tmp_gencb->ver = 1;
	tmp_gencb->arg = cb_arg;
	tmp_gencb->cb.cb_1 = callback;
}

/* Populate a BN_GENCB structure with a "new"-style callback */
void BN_GENCB_set(BN_GENCB *gencb, int (*callback) (int, int, BN_GENCB *),
				  void *cb_arg)
{
	BN_GENCB *tmp_gencb = gencb;
	tmp_gencb->ver = 2;
	tmp_gencb->arg = cb_arg;
	tmp_gencb->cb.cb_2 = callback;
}

void *BN_GENCB_get_arg(BN_GENCB *cb)
{
	return cb->arg;
}

BIGNUM *bn_wexpand(BIGNUM *a, int words)
{
	return (words <= a->dmax) ? a : bn_expand2(a, words);
}

void bn_correct_top(BIGNUM *a)
{
	BN_ULONG *ftl;
	int tmp_top = a->top;

	if (tmp_top > 0) {
		for (ftl = &(a->d[tmp_top]); tmp_top > 0; tmp_top--) {
			ftl--;

			if (*ftl != 0)
				break;
		}

		a->top = tmp_top;
	}

	if (a->top == 0)
		a->neg = 0;

	bn_pollute(a);
}

#define LBITS(a) ((a)&BN_MASK2l)
#define HBITS(a) (((a)>>BN_BITS4)&BN_MASK2l)
#define	L2HBITS(a) (((a)<<BN_BITS4)&BN_MASK2)

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
		if (m < m1) \
			ht += L2HBITS((BN_ULONG)1); \
		ht += HBITS(m); \
		m1 = L2HBITS(m); \
		lt = (lt+m1)&BN_MASK2; \
		if (lt < m1) \
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
		if (l < m) \
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
		l = (l+(c))&BN_MASK2; \
		if (l < (c)) \
			h++; \
		(c) = (r); \
		l = (l+(c))&BN_MASK2; \
		if (l < (c)) \
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
		l += (c); \
		if ((l&BN_MASK2) < (c)) \
			h++; \
		(c) = h&BN_MASK2; \
		(r) = l&BN_MASK2; \
	}



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

/* Divide h,l by d and return the result. */
/* I need to test this some more :-( */
BN_ULONG bn_div_words(BN_ULONG h, BN_ULONG l, BN_ULONG d)
{
	BN_ULONG dh, dl, q, ret = 0, th, tl, t;
	int i, count = 2;

	if (d == 0)
		return BN_MASK2;

	i = BN_num_bits_word(d);
	/* ASSERT((i == BN_BITS2) || (h <= (BN_ULONG)1 << i)); check point */
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
				((tl) <= ((t << BN_BITS4) | ((l & BN_MASK2h) >> BN_BITS4))))
				break;

			q--;
			th -= dh;
			tl -= dl;
		}

		t = (tl >> BN_BITS4);
		tl = (tl << BN_BITS4) & BN_MASK2h;
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
		h = ((h << BN_BITS4) | (l >> BN_BITS4)) & BN_MASK2;
		l = (l & BN_MASK2l) << BN_BITS4;
	}

	ret |= q;
	return ret;
}

#define BN_MUL_COMBA
#define BN_SQR_COMBA

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


#define MONT_WORD               /* use the faster word-based algorithm */

#ifdef MONT_WORD
static int BN_from_montgomery_word(BIGNUM *ret, BIGNUM *r, BN_MONT_CTX *mont);
#endif


int BN_mod_mul_montgomery(BIGNUM *r, const BIGNUM *a, const BIGNUM *b,
						  BN_MONT_CTX *mont, BN_CTX *ctx)
{
	BIGNUM *tmp;
	int ret = 0;
#if defined(OPENSSL_BN_ASM_MONT) && defined(MONT_WORD)
	int num = mont->N.top;

	if (num > 1 && a->top == num && b->top == num) {
		if (bn_wexpand(r, num) == NULL)
			return 0;

		if (bn_mul_mont(r->d, a->d, b->d, mont->N.d, mont->n0, num)) {
			r->neg = a->neg ^ b->neg;
			r->top = num;
			bn_correct_top(r);
			return 1;
		}
	}

#endif
	BN_CTX_start(ctx);
	tmp = BN_CTX_get(ctx);

	if (tmp == NULL)
		goto err;

	bn_check_top(tmp);

	if (a == b) {
		if (!BN_sqr(tmp, a, ctx))
			goto err;
	} else {
		if (!BN_mul(tmp, a, b, ctx))
			goto err;
	}

	/* reduce from aRR to aR */
#ifdef MONT_WORD

	if (!BN_from_montgomery_word(r, tmp, mont))
		goto err;

#else

	if (!BN_from_montgomery(r, tmp, mont, ctx))
		goto err;

#endif
	bn_check_top(r);
	ret = 1;
err:
	BN_CTX_end(ctx);
	return ret;
}

#ifdef MONT_WORD
static int BN_from_montgomery_word(BIGNUM *ret, BIGNUM *r, BN_MONT_CTX *mont)
{
	BIGNUM *n;
	BN_ULONG *ap, *np, *rp, n0, v, carry;
	int nl, max, i;
	n = &(mont->N);
	nl = n->top;

	if (nl == 0) {
		ret->top = 0;
		return 1;
	}

	max = (2 * nl);             /* carry is stored separately */

	if (bn_wexpand(r, max) == NULL)
		return 0;

	r->neg ^= n->neg;
	np = n->d;
	rp = r->d;
	/* clear the top words of T */
	i = max - r->top;

	if (i)
		memset(&rp[r->top], 0, sizeof(*rp) * i);

	r->top = max;
	n0 = mont->n0[0];

	for (carry = 0, i = 0; i < nl; i++, rp++) {
		v = bn_mul_add_words(rp, np, nl, (rp[0] * n0) & BN_MASK2);
		v = (v + carry + rp[nl]) & BN_MASK2;
		carry |= (v != rp[nl]);
		carry &= (v <= rp[nl]);
		rp[nl] = v;
	}

	if (bn_wexpand(ret, nl) == NULL)
		return 0;

	ret->top = nl;
	ret->neg = r->neg;
	rp = ret->d;
	ap = &(r->d[nl]);
# define BRANCH_FREE 0
# if BRANCH_FREE
	{
		BN_ULONG *nrp;
		size_t m;
		v = bn_sub_words(rp, ap, np, nl) - carry;
		/*
		 * if subtraction result is real, then trick unconditional memcpy
		 * below to perform in-place "refresh" instead of actual copy.
		 */
		m = (0 - (size_t)v);
		nrp =
			(BN_ULONG *)(((PTR_SIZE_INT) rp & ~m) | ((PTR_SIZE_INT) ap & m));

		for (i = 0, nl -= 4; i < nl; i += 4) {
			BN_ULONG t1, t2, t3, t4;
			t1 = nrp[i + 0];
			t2 = nrp[i + 1];
			t3 = nrp[i + 2];
			ap[i + 0] = 0;
			t4 = nrp[i + 3];
			ap[i + 1] = 0;
			rp[i + 0] = t1;
			ap[i + 2] = 0;
			rp[i + 1] = t2;
			ap[i + 3] = 0;
			rp[i + 2] = t3;
			rp[i + 3] = t4;
		}

		for (nl += 4; i < nl; i++)
			rp[i] = nrp[i], ap[i] = 0;
	}
# else

	if (bn_sub_words(rp, ap, np, nl) - carry)
		memcpy(rp, ap, nl * sizeof(BN_ULONG));

# endif
	bn_correct_top(r);
	bn_correct_top(ret);
	bn_check_top(ret);
	return 1;
}
#endif                          /* MONT_WORD */

int BN_from_montgomery(BIGNUM *ret, const BIGNUM *a, BN_MONT_CTX *mont,
					   BN_CTX *ctx)
{
	int retn = 0;
#ifdef MONT_WORD
	BIGNUM *t;
	BN_CTX_start(ctx);

	t = BN_CTX_get(ctx);
	if (t && BN_copy(t, a))
		retn = BN_from_montgomery_word(ret, t, mont);

	BN_CTX_end(ctx);
#else                           /* !MONT_WORD */
	BIGNUM *t1, *t2;
	BN_CTX_start(ctx);
	t1 = BN_CTX_get(ctx);
	t2 = BN_CTX_get(ctx);

	if (t1 == NULL || t2 == NULL)
		goto err;

	if (!BN_copy(t1, a))
		goto err;

	BN_mask_bits(t1, mont->ri);

	if (!BN_mul(t2, t1, &mont->Ni, ctx))
		goto err;

	BN_mask_bits(t2, mont->ri);

	if (!BN_mul(t1, t2, &mont->N, ctx))
		goto err;

	if (!BN_add(t2, a, t1))
		goto err;

	if (!BN_rshift(ret, t2, mont->ri))
		goto err;

	if (BN_ucmp(ret, &(mont->N)) >= 0) {
		if (!BN_usub(ret, ret, &(mont->N)))
			goto err;
	}

	retn = 1;
	bn_check_top(ret);
err:
	BN_CTX_end(ctx);
#endif                          /* MONT_WORD */
	return retn;
}

BN_MONT_CTX *BN_MONT_CTX_new(void)
{
	BN_MONT_CTX *ret;
	os_alloc_mem(NULL, (UCHAR **)&ret, sizeof(BN_MONT_CTX));

	if (ret == NULL)
		return NULL;

	BN_MONT_CTX_init(ret);
	ret->flags = BN_FLG_MALLOCED;
	return ret;
}

void BN_MONT_CTX_init(BN_MONT_CTX *ctx)
{
	ctx->ri = 0;
	bn_init(&(ctx->RR));
	bn_init(&(ctx->N));
	bn_init(&(ctx->Ni));
	ctx->n0[0] = ctx->n0[1] = 0;
	ctx->flags = 0;
}

void BN_MONT_CTX_free(BN_MONT_CTX *mont)
{
	if (mont == NULL)
		return;

	BN_clear_free(&(mont->RR));
	BN_clear_free(&(mont->N));
	BN_clear_free(&(mont->Ni));

	if (mont->flags & BN_FLG_MALLOCED)
		os_free_mem(mont);
}

int BN_MONT_CTX_set(BN_MONT_CTX *mont, const BIGNUM *mod, BN_CTX *ctx)
{
	int ret = 0;
	BIGNUM *Ri, *R;

	if (BN_is_zero(mod))
		return 0;

	BN_CTX_start(ctx);

	Ri = BN_CTX_get(ctx);
	if (Ri == NULL)
		goto err;

	R = &(mont->RR);            /* grab RR as a temp */

	if (!BN_copy(&(mont->N), mod))
		goto err;               /* Set N */

	mont->N.neg = 0;
#ifdef MONT_WORD
	{
		BIGNUM tmod;
		BN_ULONG buf[2];
		bn_init(&tmod);
		tmod.d = buf;
		tmod.dmax = 2;
		tmod.neg = 0;

		if (BN_get_flags(mod, BN_FLG_CONSTTIME) != 0)
			BN_set_flags(&tmod, BN_FLG_CONSTTIME);

		mont->ri = (BN_num_bits(mod) + (BN_BITS2 - 1)) / BN_BITS2 * BN_BITS2;
# if defined(OPENSSL_BN_ASM_MONT) && (BN_BITS2 <= 32)
		/*
		 * Only certain BN_BITS2<=32 platforms actually make use of n0[1],
		 * and we could use the #else case (with a shorter R value) for the
		 * others.  However, currently only the assembler files do know which
		 * is which.
		 */
		BN_zero(R);

		if (!(BN_set_bit(R, 2 * BN_BITS2)))
			goto err;

		tmod.top = 0;

		buf[0] = mod->d[0];
		if (buf[0])
			tmod.top = 1;

		buf[1] = mod->top > 1 ? mod->d[1] : 0;
		if (buf[1])
			tmod.top = 2;

		if ((BN_mod_inverse(Ri, R, &tmod, ctx)) == NULL)
			goto err;

		if (!BN_lshift(Ri, Ri, 2 * BN_BITS2))
			goto err;           /* R*Ri */

		if (!BN_is_zero(Ri)) {
			if (!BN_sub_word(Ri, 1))
				goto err;
		} else {                /* if N mod word size == 1 */
			if (bn_expand(Ri, (int)sizeof(BN_ULONG) * 2) == NULL)
				goto err;

			/* Ri-- (mod double word size) */
			Ri->neg = 0;
			Ri->d[0] = BN_MASK2;
			Ri->d[1] = BN_MASK2;
			Ri->top = 2;
		}

		if (!BN_div(Ri, NULL, Ri, &tmod, ctx))
			goto err;

		/*
		 * Ni = (R*Ri-1)/N, keep only couple of least significant words:
		 */
		mont->n0[0] = (Ri->top > 0) ? Ri->d[0] : 0;
		mont->n0[1] = (Ri->top > 1) ? Ri->d[1] : 0;
# else
		BN_zero(R);

		if (!(BN_set_bit(R, BN_BITS2)))
			goto err;           /* R */

		buf[0] = mod->d[0];     /* tmod = N mod word size */
		buf[1] = 0;
		tmod.top = buf[0] != 0 ? 1 : 0;

		/* Ri = R^-1 mod N */
		if ((BN_mod_inverse(Ri, R, &tmod, ctx)) == NULL)
			goto err;

		if (!BN_lshift(Ri, Ri, BN_BITS2))
			goto err;           /* R*Ri */

		if (!BN_is_zero(Ri)) {
			if (!BN_sub_word(Ri, 1))
				goto err;
		} else {                /* if N mod word size == 1 */
			if (!BN_set_word(Ri, BN_MASK2))
				goto err;       /* Ri-- (mod word size) */
		}

		if (!BN_div(Ri, NULL, Ri, &tmod, ctx))
			goto err;

		/*
		 * Ni = (R*Ri-1)/N, keep only least significant word:
		 */
		mont->n0[0] = (Ri->top > 0) ? Ri->d[0] : 0;
		mont->n0[1] = 0;
# endif
	}
#else                           /* !MONT_WORD */
	{                           /* bignum version */
		mont->ri = BN_num_bits(&mont->N);
		BN_zero(R);

		if (!BN_set_bit(R, mont->ri))
			goto err;           /* R = 2^ri */

		/* Ri = R^-1 mod N */
		if ((BN_mod_inverse(Ri, R, &mont->N, ctx)) == NULL)
			goto err;

		if (!BN_lshift(Ri, Ri, mont->ri))
			goto err;           /* R*Ri */

		if (!BN_sub_word(Ri, 1))
			goto err;

		/*
		 * Ni = (R*Ri-1) / N
		 */
		if (!BN_div(&(mont->Ni), NULL, Ri, &mont->N, ctx))
			goto err;
	}
#endif
	/* setup RR for conversions */
	BN_zero(&(mont->RR));

	if (!BN_set_bit(&(mont->RR), mont->ri * 2))
		goto err;

	if (!BN_mod(&(mont->RR), &(mont->RR), &(mont->N), ctx))
		goto err;

	ret = 1;
err:
	BN_CTX_end(ctx);
	return ret;
}

BN_MONT_CTX *BN_MONT_CTX_copy(BN_MONT_CTX *to, BN_MONT_CTX *from)
{
	if (to == from)
		return to;

	if (!BN_copy(&(to->RR), &(from->RR)))
		return NULL;

	if (!BN_copy(&(to->N), &(from->N)))
		return NULL;

	if (!BN_copy(&(to->Ni), &(from->Ni)))
		return NULL;

	to->ri = from->ri;
	to->n0[0] = from->n0[0];
	to->n0[1] = from->n0[1];
	return to;
}

void BN_STACK_init(BN_STACK *st)
{
	st->indexes = NULL;
	st->depth = st->size = 0;
}

void BN_STACK_finish(BN_STACK *st)
{
	if (st->indexes)
		os_free_mem(st->indexes);

	st->indexes = NULL;
}


int BN_STACK_push(BN_STACK *st, unsigned int idx)
{
	if (st->depth == st->size) {
		/* Need to expand */
		unsigned int newsize =
			st->size ? (st->size * 3 / 2) : BN_CTX_START_FRAMES;
		unsigned int *newitems;
		os_alloc_mem(NULL, (UCHAR **)&newitems, sizeof(*newitems) * newsize);

		if (newitems == NULL)
			return 0;

		if (st->depth)
			memcpy(newitems, st->indexes, sizeof(*newitems) * st->depth);

		if (st->indexes)
			os_free_mem(st->indexes);

		st->indexes = newitems;
		st->size = newsize;
	}

	st->indexes[(st->depth)++] = idx;
	return 1;
}

unsigned int BN_STACK_pop(BN_STACK *st)
{
	return st->indexes[--(st->depth)];
}

void BN_POOL_init(BN_POOL *p)
{
	p->head = NULL;
	p->curr = NULL;
	p->tail = NULL;
	p->used = 0;
	p->size = 0;
}

void BN_POOL_finish(BN_POOL *p)
{
	unsigned int loop;
	BIGNUM *bn;

	while (p->head) {
		for (loop = 0, bn = p->head->vals; loop++ < BN_CTX_POOL_SIZE; bn++)
			if (bn->d)
				BN_clear_free(bn);

		p->curr = p->head->next;

		if (p->head)
			os_free_mem(p->head);

		p->head = p->curr;
	}
}


BIGNUM *BN_POOL_get(BN_POOL *p, int flag)
{
	BIGNUM *bn;
	unsigned int loop;

	/* Full; allocate a new pool item and link it in. */
	if (p->used == p->size) {
		BN_POOL_ITEM *item;
		os_alloc_mem(NULL, (UCHAR **)&item, sizeof(BN_POOL_ITEM));

		if (item == NULL)
			return NULL;

		for (loop = 0, bn = item->vals; loop++ < BN_CTX_POOL_SIZE; bn++) {
			bn_init(bn);

			if ((flag & BN_FLG_SECURE) != 0)
				BN_set_flags(bn, BN_FLG_SECURE);
		}

		item->prev = p->tail;
		item->next = NULL;

		if (p->head == NULL)
			p->head = p->curr = p->tail = item;
		else {
			p->tail->next = item;
			p->tail = item;
			p->curr = item;
		}

		p->size += BN_CTX_POOL_SIZE;
		p->used++;
		/* Return the first bignum from the new pool */
		return item->vals;
	}

	if (!p->used)
		p->curr = p->head;
	else if ((p->used % BN_CTX_POOL_SIZE) == 0)
		p->curr = p->curr->next;

	return p->curr->vals + ((p->used++) % BN_CTX_POOL_SIZE);
}

void BN_POOL_release(BN_POOL *p, unsigned int num)
{
	unsigned int offset = (p->used - 1) % BN_CTX_POOL_SIZE;
	p->used -= num;

	while (num--) {
		bn_check_top(p->curr->vals + offset);

		if (offset == 0) {
			offset = BN_CTX_POOL_SIZE - 1;
			p->curr = p->curr->prev;
		} else
			offset--;
	}
}


BN_CTX *BN_CTX_new(void)
{
	BN_CTX *ret;
	os_alloc_mem(NULL, (UCHAR **)&ret, sizeof(BN_CTX));
	memset(ret, 0, sizeof(BN_CTX));

	if (ret == NULL)
		return NULL;

	/* Initialise the structure */
	BN_POOL_init(&ret->pool);
	BN_STACK_init(&ret->stack);
	return ret;
}

BN_CTX *BN_CTX_secure_new(void)
{
	BN_CTX *ret = BN_CTX_new();

	if (ret != NULL)
		ret->flags = BN_FLG_SECURE;

	return ret;
}

void BN_CTX_free(BN_CTX *ctx)
{
	if (ctx == NULL)
		return;

	BN_STACK_finish(&ctx->stack);
	BN_POOL_finish(&ctx->pool);

	if (ctx)
		os_free_mem(ctx);
}

void BN_CTX_start(BN_CTX *ctx)
{
	/* If we're already overflowing ... */
	if (ctx->err_stack || ctx->too_many)
		ctx->err_stack++;
	/* (Try to) get a new frame pointer */
	else if (!BN_STACK_push(&ctx->stack, ctx->used)) {
		BNerr(BN_F_BN_CTX_START, BN_R_TOO_MANY_TEMPORARY_VARIABLES);
		ctx->err_stack++;
	}
}

void BN_CTX_end(BN_CTX *ctx)
{
	if (ctx->err_stack)
		ctx->err_stack--;
	else {
		unsigned int fp = BN_STACK_pop(&ctx->stack);

		/* Does this stack frame have anything to release? */
		if (fp < ctx->used)
			BN_POOL_release(&ctx->pool, ctx->used - fp);

		ctx->used = fp;
		/* Unjam "too_many" in case "get" had failed */
		ctx->too_many = 0;
	}
}

BIGNUM *BN_CTX_get(BN_CTX *ctx)
{
	BIGNUM *ret;

	if (ctx->err_stack || ctx->too_many)
		return NULL;

	ret = BN_POOL_get(&ctx->pool, ctx->flags);
	if (ret == NULL) {
		/*
		 * Setting too_many prevents repeated "get" attempts from cluttering
		 * the error stack.
		 */
		ctx->too_many = 1;
		BNerr(BN_F_BN_CTX_GET, BN_R_TOO_MANY_TEMPORARY_VARIABLES);
		return NULL;
	}

	/* OK, make sure the returned bignum is "zero" */
	BN_zero(ret);
	ctx->used++;
	return ret;
}

void BN_RECP_CTX_init(BN_RECP_CTX *recp)
{
	memset(recp, 0, sizeof(*recp));
	bn_init(&(recp->N));
	bn_init(&(recp->Nr));
}

BN_RECP_CTX *BN_RECP_CTX_new(void)
{
	BN_RECP_CTX *ret;
	os_alloc_mem(NULL, (UCHAR **)&ret, sizeof(BN_RECP_CTX));

	if (ret == NULL)
		return NULL;

	bn_init(&(ret->N));
	bn_init(&(ret->Nr));
	ret->flags = BN_FLG_MALLOCED;
	return ret;
}

void BN_RECP_CTX_free(BN_RECP_CTX *recp)
{
	if (recp == NULL)
		return;

	BN_free(&(recp->N));
	BN_free(&(recp->Nr));

	if (recp->flags & BN_FLG_MALLOCED)
		os_free_mem(recp);
}

int BN_RECP_CTX_set(BN_RECP_CTX *recp, const BIGNUM *d, BN_CTX *ctx)
{
	if (!BN_copy(&(recp->N), d))
		return 0;

	BN_zero(&(recp->Nr));
	recp->num_bits = BN_num_bits(d);
	recp->shift = 0;
	return 1;
}

int BN_mod_mul_reciprocal(BIGNUM *r, const BIGNUM *x, const BIGNUM *y,
						  BN_RECP_CTX *recp, BN_CTX *ctx)
{
	int ret = 0;
	BIGNUM *a;
	const BIGNUM *ca;
	BN_CTX_start(ctx);

	a = BN_CTX_get(ctx);
	if (a == NULL)
		goto err;

	if (y != NULL) {
		if (x == y) {
			if (!BN_sqr(a, x, ctx))
				goto err;
		} else {
			if (!BN_mul(a, x, y, ctx))
				goto err;
		}

		ca = a;
	} else
		ca = x;                 /* Just do the mod */

	ret = BN_div_recp(NULL, r, ca, recp, ctx);
err:
	BN_CTX_end(ctx);
	bn_check_top(r);
	return ret;
}

int BN_div_recp(BIGNUM *dv, BIGNUM *rem, const BIGNUM *m,
				BN_RECP_CTX *recp, BN_CTX *ctx)
{
	int i, j, ret = 0;
	BIGNUM *a, *b, *d, *r;
	BN_CTX_start(ctx);
	a = BN_CTX_get(ctx);
	b = BN_CTX_get(ctx);

	if (dv != NULL)
		d = dv;
	else
		d = BN_CTX_get(ctx);

	if (rem != NULL)
		r = rem;
	else
		r = BN_CTX_get(ctx);

	if (a == NULL || b == NULL || d == NULL || r == NULL)
		goto err;

	if (BN_ucmp(m, &(recp->N)) < 0) {
		BN_zero(d);

		if (!BN_copy(r, m)) {
			BN_CTX_end(ctx);
			return 0;
		}

		BN_CTX_end(ctx);
		return 1;
	}

	/*
	 * We want the remainder Given input of ABCDEF / ab we need multiply
	 * ABCDEF by 3 digests of the reciprocal of ab
	 */
	/* i := max(BN_num_bits(m), 2*BN_num_bits(N)) */
	i = BN_num_bits(m);
	j = recp->num_bits << 1;

	if (j > i)
		i = j;

	/* Nr := round(2^i / N) */
	if (i != recp->shift)
		recp->shift = BN_reciprocal(&(recp->Nr), &(recp->N), i, ctx);

	/* BN_reciprocal could have returned -1 for an error */
	if (recp->shift == -1)
		goto err;

	/*-
	 * d := |round(round(m / 2^BN_num_bits(N)) * recp->Nr / 2^(i - BN_num_bits(N)))|
	 *    = |round(round(m / 2^BN_num_bits(N)) * round(2^i / N) / 2^(i - BN_num_bits(N)))|
	 *   <= |(m / 2^BN_num_bits(N)) * (2^i / N) * (2^BN_num_bits(N) / 2^i)|
	 *    = |m/N|
	 */
	if (!BN_rshift(a, m, recp->num_bits))
		goto err;

	if (!BN_mul(b, a, &(recp->Nr), ctx))
		goto err;

	if (!BN_rshift(d, b, i - recp->num_bits))
		goto err;

	d->neg = 0;

	if (!BN_mul(b, &(recp->N), d, ctx))
		goto err;

	if (!BN_usub(r, m, b))
		goto err;

	r->neg = 0;
	j = 0;

	while (BN_ucmp(r, &(recp->N)) >= 0) {
		if (j++ > 2) {
			BNerr(BN_F_BN_DIV_RECP, BN_R_BAD_RECIPROCAL);
			goto err;
		}

		if (!BN_usub(r, r, &(recp->N)))
			goto err;

		if (!BN_add_word(d, 1))
			goto err;
	}

	r->neg = BN_is_zero(r) ? 0 : m->neg;
	d->neg = m->neg ^ recp->N.neg;
	ret = 1;
err:
	BN_CTX_end(ctx);
	bn_check_top(dv);
	bn_check_top(rem);
	return ret;
}

/*
 * len is the expected size of the result We actually calculate with an extra
 * word of precision, so we can do faster division if the remainder is not
 * required.
 */
/* r := 2^len / m */
int BN_reciprocal(BIGNUM *r, const BIGNUM *m, int len, BN_CTX *ctx)
{
	int ret = -1;
	BIGNUM *t;
	BN_CTX_start(ctx);

	t = BN_CTX_get(ctx);
	if (t == NULL)
		goto err;

	if (!BN_set_bit(t, len))
		goto err;

	if (!BN_div(r, NULL, t, m, ctx))
		goto err;

	ret = len;
err:
	bn_check_top(r);
	BN_CTX_end(ctx);
	return ret;
}

int BN_sqr(BIGNUM *r, const BIGNUM *a, BN_CTX *ctx)
{
	int max, al;
	int ret = 0;
	BIGNUM *tmp, *rr;
	bn_check_top(a);
	al = a->top;

	if (al <= 0) {
		r->top = 0;
		r->neg = 0;
		return 1;
	}

	BN_CTX_start(ctx);
	rr = (a != r) ? r : BN_CTX_get(ctx);
	tmp = BN_CTX_get(ctx);

	if (!rr || !tmp)
		goto err;

	max = 2 * al;               /* Non-zero (from above) */

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

	rr->neg = 0;

	/*
	 * If the most-significant half of the top word of 'a' is zero, then the
	 * square of 'a' will max-1 words.
	 */
	if (a->d[al - 1] == (a->d[al - 1] & BN_MASK2l))
		rr->top = max - 1;
	else
		rr->top = max;

	if (r != rr && BN_copy(r, rr) == NULL)
		goto err;

	ret = 1;
err:
	bn_check_top(rr);
	bn_check_top(tmp);
	BN_CTX_end(ctx);
	return ret;
}

/* tmp must have 2*n words */
void bn_sqr_normal(BN_ULONG *r, const BN_ULONG *a, int n, BN_ULONG *tmp)
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
/*-
 * r is 2*n words in size,
 * a and b are both n words in size.    (There's not actually a 'b' here ...)
 * n must be a power of 2.
 * We multiply and return the result.
 * t must be 2*n words in size
 * We calculate
 * a[0]*b[0]
 * a[0]*b[0]+a[1]*b[1]+(a[0]-a[1])*(b[1]-b[0])
 * a[1]*b[1]
 */
void bn_sqr_recursive(BN_ULONG *r, const BN_ULONG *a, int n2, BN_ULONG *t)
{
	int n = n2 / 2;
	int zero, c1;
	BN_ULONG ln, lo, *p;

	if (n2 == 4) {
# ifndef BN_SQR_COMBA
		bn_sqr_normal(r, a, 4, t);
# else
		bn_sqr_comba4(r, a);
# endif
		return;
	} else if (n2 == 8) {
# ifndef BN_SQR_COMBA
		bn_sqr_normal(r, a, 8, t);
# else
		bn_sqr_comba8(r, a);
# endif
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
		memset(&t[n2], 0, sizeof(*t) * n2);

	bn_sqr_recursive(r, a, n, p);
	bn_sqr_recursive(&(r[n2]), &(a[n]), n, p);
	/*-
	 * t[32] holds (a[0]-a[1])*(a[1]-a[0]), it is negative or zero
	 * r[10] holds (a[0]*b[0])
	 * r[32] holds (b[1]*b[1])
	 */
	c1 = (int)(bn_add_words(t, r, &(r[n2]), n2));
	/* t[32] is negative */
	c1 -= (int)(bn_sub_words(&(t[n2]), t, &(t[n2]), n2));
	/*-
	 * t[32] holds (a[0]-a[1])*(a[1]-a[0])+(a[0]*a[0])+(a[1]*a[1])
	 * r[10] holds (a[0]*a[0])
	 * r[32] holds (a[1]*a[1])
	 * c1 holds the carry bits
	 */
	c1 += (int)(bn_add_words(&(r[n]), &(r[n]), &(t[n2]), n2));

	if (c1) {
		p = &(r[n + n2]);
		lo = *p;
		ln = (lo + c1) & BN_MASK2;
		*p = ln;

		/*
		 * The overflow will stop before we over write words we should not
		 * overwrite
		 */
		if (ln < (BN_ULONG)c1) {
			do {
				p++;
				lo = *p;
				ln = (lo + 1) & BN_MASK2;
				*p = ln;
			} while (ln == 0);
		}
	}
}
#endif

/*
 * BN_mod_inverse_no_branch is a special version of BN_mod_inverse. It does
 * not contain branches that may leak sensitive information.
 */
static BIGNUM *BN_mod_inverse_no_branch(BIGNUM *in,
										const BIGNUM *a, const BIGNUM *n,
										BN_CTX *ctx)
{
	BIGNUM *A, *B, *X, *Y, *M, *D, *T, *R = NULL;
	BIGNUM *ret = NULL;
	int sign;
	bn_check_top(a);
	bn_check_top(n);
	BN_CTX_start(ctx);
	A = BN_CTX_get(ctx);
	B = BN_CTX_get(ctx);
	X = BN_CTX_get(ctx);
	D = BN_CTX_get(ctx);
	M = BN_CTX_get(ctx);
	Y = BN_CTX_get(ctx);
	T = BN_CTX_get(ctx);

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

	A->neg = 0;

	if (B->neg || (BN_ucmp(B, A) >= 0)) {
		/*
		 * Turn BN_FLG_CONSTTIME flag on, so that when BN_div is invoked,
		 * BN_div_no_branch will be called eventually.
		 */
		{
			BIGNUM local_B;
			bn_init(&local_B);
			BN_with_flags(&local_B, B, BN_FLG_CONSTTIME);

			if (!BN_nnmod(B, &local_B, A, ctx))
				goto err;

			/* Ensure local_B goes out of scope before any further use of B */
		}
	}

	sign = -1;
	/*-
	 * From  B = a mod |n|,  A = |n|  it follows that
	 *
	 *      0 <= B < A,
	 *     -sign*X*a  ==  B   (mod |n|),
	 *      sign*Y*a  ==  A   (mod |n|).
	 */

	while (!BN_is_zero(B)) {
		BIGNUM *tmp;
		/*-
		 *      0 < B < A,
		 * (*) -sign*X*a  ==  B   (mod |n|),
		 *      sign*Y*a  ==  A   (mod |n|)
		 */
		/*
		 * Turn BN_FLG_CONSTTIME flag on, so that when BN_div is invoked,
		 * BN_div_no_branch will be called eventually.
		 */
		{
			BIGNUM local_A;
			bn_init(&local_A);
			BN_with_flags(&local_A, A, BN_FLG_CONSTTIME);

			/* (D, M) := (A/B, A%B) ... */
			if (!BN_div(D, M, &local_A, B, ctx))
				goto err;

			/* Ensure local_A goes out of scope before any further use of A */
		}
		/*-
		 * Now
		 *      A = D*B + M;
		 * thus we have
		 * (**)  sign*Y*a  ==  D*B + M   (mod |n|).
		 */
		tmp = A; /* keep the BIGNUM object, the value does not matter */
		/* (A, B) := (B, A mod B) ... */
		A = B;
		B = M;
		/* ... so we have  0 <= B < A  again */

		/*-
		 * Since the former  M  is now  B  and the former  B  is now  A,
		 * (**) translates into
		 *       sign*Y*a  ==  D*A + B    (mod |n|),
		 * i.e.
		 *       sign*Y*a - D*A  ==  B    (mod |n|).
		 * Similarly, (*) translates into
		 *      -sign*X*a  ==  A          (mod |n|).
		 *
		 * Thus,
		 *   sign*Y*a + D*sign*X*a  ==  B  (mod |n|),
		 * i.e.
		 *        sign*(Y + D*X)*a  ==  B  (mod |n|).
		 *
		 * So if we set  (X, Y, sign) := (Y + D*X, X, -sign), we arrive back at
		 *      -sign*X*a  ==  B   (mod |n|),
		 *       sign*Y*a  ==  A   (mod |n|).
		 * Note that  X  and  Y  stay non-negative all the time.
		 */

		if (!BN_mul(tmp, D, X, ctx))
			goto err;

		if (!BN_add(tmp, tmp, Y))
			goto err;

		M = Y; /* keep the BIGNUM object, the value does not matter */
		Y = X;
		X = tmp;
		sign = -sign;
	}

	/*-
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
		if (!Y->neg && BN_ucmp(Y, n) < 0) {
			if (!BN_copy(R, Y))
				goto err;
		} else {
			if (!BN_nnmod(R, Y, n, ctx))
				goto err;
		}
	} else {
		BNerr(BN_F_BN_MOD_INVERSE_NO_BRANCH, BN_R_NO_INVERSE);
		goto err;
	}

	ret = R;
err:

	if ((ret == NULL) && (in == NULL))
		BN_free(R);

	BN_CTX_end(ctx);
	bn_check_top(ret);
	return ret;
}


BIGNUM *int_bn_mod_inverse(BIGNUM *in,
						   const BIGNUM *a, const BIGNUM *n, BN_CTX *ctx,
						   int *pnoinv)
{
	BIGNUM *A, *B, *X, *Y, *M, *D, *T, *R = NULL;
	BIGNUM *ret = NULL;
	int sign;

	if (pnoinv)
		*pnoinv = 0;

	if ((BN_get_flags(a, BN_FLG_CONSTTIME) != 0)
		|| (BN_get_flags(n, BN_FLG_CONSTTIME) != 0))
		return BN_mod_inverse_no_branch(in, a, n, ctx);

	bn_check_top(a);
	bn_check_top(n);
	BN_CTX_start(ctx);
	A = BN_CTX_get(ctx);
	B = BN_CTX_get(ctx);
	X = BN_CTX_get(ctx);
	D = BN_CTX_get(ctx);
	M = BN_CTX_get(ctx);
	Y = BN_CTX_get(ctx);
	T = BN_CTX_get(ctx);

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

	A->neg = 0;

	if (B->neg || (BN_ucmp(B, A) >= 0)) {
		if (!BN_nnmod(B, B, A, ctx))
			goto err;
	}

	sign = -1;
	/*-
	 * From  B = a mod |n|,  A = |n|  it follows that
	 *
	 *      0 <= B < A,
	 *     -sign*X*a  ==  B   (mod |n|),
	 *      sign*Y*a  ==  A   (mod |n|).
	 */

	if (BN_is_odd(n) && (BN_num_bits(n) <= 2048)) {
		/*
		 * Binary inversion algorithm; requires odd modulus. This is faster
		 * than the general algorithm if the modulus is sufficiently small
		 * (about 400 .. 500 bits on 32-bit systems, but much more on 64-bit
		 * systems)
		 */
		int shift;

		while (!BN_is_zero(B)) {
			/*-
			 *      0 < B < |n|,
			 *      0 < A <= |n|,
			 * (1) -sign*X*a  ==  B   (mod |n|),
			 * (2)  sign*Y*a  ==  A   (mod |n|)
			 */
			/*
			 * Now divide B by the maximum possible power of two in the
			 * integers, and divide X by the same value mod |n|. When we're
			 * done, (1) still holds.
			 */
			shift = 0;

			while (!BN_is_bit_set(B, shift)) { /* note that 0 < B */
				shift++;

				if (BN_is_odd(X)) {
					if (!BN_uadd(X, X, n))
						goto err;
				}

				/*
				 * now X is even, so we can easily divide it by two
				 */
				if (!BN_rshift1(X, X))
					goto err;
			}

			if (shift > 0) {
				if (!BN_rshift(B, B, shift))
					goto err;
			}

			/*
			 * Same for A and Y.  Afterwards, (2) still holds.
			 */
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

			/*-
			 * We still have (1) and (2).
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

				/*
				 * NB: we could use BN_mod_add_quick(X, X, Y, n), but that
				 * actually makes the algorithm slower
				 */
				if (!BN_usub(B, B, A))
					goto err;
			} else {
				/*  sign*(X + Y)*a == A - B  (mod |n|) */
				if (!BN_uadd(Y, Y, X))
					goto err;

				/*
				 * as above, BN_mod_add_quick(Y, Y, X, n) would slow things down
				 */
				if (!BN_usub(A, A, B))
					goto err;
			}
		}
	} else {
		/* general inversion algorithm */
		while (!BN_is_zero(B)) {
			BIGNUM *tmp;

			/*-
			 *      0 < B < A,
			 * (*) -sign*X*a  ==  B   (mod |n|),
			 *      sign*Y*a  ==  A   (mod |n|)
			 */

			/* (D, M) := (A/B, A%B) ... */
			if (BN_num_bits(A) == BN_num_bits(B)) {
				if (!BN_one(D))
					goto err;

				if (!BN_sub(M, A, B))
					goto err;
			} else if (BN_num_bits(A) == BN_num_bits(B) + 1) {
				/* A/B is 1, 2, or 3 */
				if (!BN_lshift1(T, B))
					goto err;

				if (BN_ucmp(A, T) < 0) {
					/* A < 2*B, so D=1 */
					if (!BN_one(D))
						goto err;

					if (!BN_sub(M, A, B))
						goto err;
				} else {
					/* A >= 2*B, so D=2 or D=3 */
					if (!BN_sub(M, A, T))
						goto err;

					if (!BN_add(D, T, B))
						goto err; /* use D (:= 3*B) as temp */

					if (BN_ucmp(A, D) < 0) {
						/* A < 3*B, so D=2 */
						if (!BN_set_word(D, 2))
							goto err;

						/*
						 * M (= A - 2*B) already has the correct value
						 */
					} else {
						/* only D=3 remains */
						if (!BN_set_word(D, 3))
							goto err;

						/*
						 * currently M = A - 2*B, but we need M = A - 3*B
						 */
						if (!BN_sub(M, M, B))
							goto err;
					}
				}
			} else {
				if (!BN_div(D, M, A, B, ctx))
					goto err;
			}

			/*-
			 * Now
			 *      A = D*B + M;
			 * thus we have
			 * (**)  sign*Y*a  ==  D*B + M   (mod |n|).
			 */
			tmp = A;    /* keep the BIGNUM object, the value does not matter */
			/* (A, B) := (B, A mod B) ... */
			A = B;
			B = M;
			/* ... so we have  0 <= B < A  again */

			/*-
			 * Since the former  M  is now  B  and the former  B  is now  A,
			 * (**) translates into
			 *       sign*Y*a  ==  D*A + B    (mod |n|),
			 * i.e.
			 *       sign*Y*a - D*A  ==  B    (mod |n|).
			 * Similarly, (*) translates into
			 *      -sign*X*a  ==  A          (mod |n|).
			 *
			 * Thus,
			 *   sign*Y*a + D*sign*X*a  ==  B  (mod |n|),
			 * i.e.
			 *        sign*(Y + D*X)*a  ==  B  (mod |n|).
			 *
			 * So if we set  (X, Y, sign) := (Y + D*X, X, -sign), we arrive back at
			 *      -sign*X*a  ==  B   (mod |n|),
			 *       sign*Y*a  ==  A   (mod |n|).
			 * Note that  X  and  Y  stay non-negative all the time.
			 */

			/*
			 * most of the time D is very small, so we can optimize tmp := D*X+Y
			 */
			if (BN_is_one(D)) {
				if (!BN_add(tmp, X, Y))
					goto err;
			} else {
				if (BN_is_word(D, 2)) {
					if (!BN_lshift1(tmp, X))
						goto err;
				} else if (BN_is_word(D, 4)) {
					if (!BN_lshift(tmp, X, 2))
						goto err;
				} else if (D->top == 1) {
					if (!BN_copy(tmp, X))
						goto err;

					if (!BN_mul_word(tmp, D->d[0]))
						goto err;
				} else {
					if (!BN_mul(tmp, D, X, ctx))
						goto err;
				}

				if (!BN_add(tmp, tmp, Y))
					goto err;
			}

			M = Y;      /* keep the BIGNUM object, the value does not matter */
			Y = X;
			X = tmp;
			sign = -sign;
		}
	}

	/*-
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
		if (!Y->neg && BN_ucmp(Y, n) < 0) {
			if (!BN_copy(R, Y))
				goto err;
		} else {
			if (!BN_nnmod(R, Y, n, ctx))
				goto err;
		}
	} else {
		if (pnoinv)
			*pnoinv = 1;

		goto err;
	}

	ret = R;
err:

	if ((ret == NULL) && (in == NULL))
		BN_free(R);

	BN_CTX_end(ctx);
	bn_check_top(ret);
	return ret;
}

BIGNUM *BN_mod_inverse(BIGNUM *in,
					   const BIGNUM *a, const BIGNUM *n, BN_CTX *ctx)
{
	BIGNUM *rv;
	int noinv;
	rv = int_bn_mod_inverse(in, a, n, ctx, &noinv);

	if (noinv)
		BNerr(BN_F_BN_MOD_INVERSE, BN_R_NO_INVERSE);

	return rv;
}

int BN_lshift1(BIGNUM *r, const BIGNUM *a)
{
	register BN_ULONG *ap, *rp, t, c;
	int i;
	bn_check_top(r);
	bn_check_top(a);

	if (r != a) {
		r->neg = a->neg;

		if (bn_wexpand(r, a->top + 1) == NULL)
			return 0;

		r->top = a->top;
	} else {
		if (bn_wexpand(r, a->top + 1) == NULL)
			return 0;
	}

	ap = a->d;
	rp = r->d;
	c = 0;

	for (i = 0; i < a->top; i++) {
		t = *(ap++);
		*(rp++) = ((t << 1) | c) & BN_MASK2;
		c = (t & BN_TBIT) ? 1 : 0;
	}

	if (c) {
		*rp = 1;
		r->top++;
	}

	bn_check_top(r);
	return 1;
}

int BN_rshift1(BIGNUM *r, const BIGNUM *a)
{
	BN_ULONG *ap, *rp, t, c;
	int i, j;
	bn_check_top(r);
	bn_check_top(a);

	if (BN_is_zero(a)) {
		BN_zero(r);
		return 1;
	}

	i = a->top;
	ap = a->d;
	j = i - (ap[i - 1] == 1);

	if (a != r) {
		if (bn_wexpand(r, j) == NULL)
			return 0;

		r->neg = a->neg;
	}

	rp = r->d;
	t = ap[--i];
	c = (t & 1) ? BN_TBIT : 0;

	if (t >>= 1)
		rp[i] = t;

	while (i > 0) {
		t = ap[--i];
		rp[i] = ((t >> 1) & BN_MASK2) | c;
		c = (t & 1) ? BN_TBIT : 0;
	}

	r->top = j;

	if (!r->top)
		r->neg = 0; /* don't allow negative zero */

	bn_check_top(r);
	return 1;
}

int BN_lshift(BIGNUM *r, const BIGNUM *a, int n)
{
	int i, nw, lb, rb;
	BN_ULONG *t, *f;
	BN_ULONG l;
	bn_check_top(r);
	bn_check_top(a);

	if (n < 0) {
		BNerr(BN_F_BN_LSHIFT, BN_R_INVALID_SHIFT);
		return 0;
	}

	nw = n / BN_BITS2;

	if (bn_wexpand(r, a->top + nw + 1) == NULL)
		return 0;

	r->neg = a->neg;
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
			t[nw + i + 1] |= (l >> rb) & BN_MASK2;
			t[nw + i] = (l << lb) & BN_MASK2;
		}

	memset(t, 0, sizeof(*t) * nw);
	r->top = a->top + nw + 1;
	bn_correct_top(r);
	bn_check_top(r);
	return 1;
}

int BN_rshift(BIGNUM *r, const BIGNUM *a, int n)
{
	int i, j, nw, lb, rb;
	BN_ULONG *t, *f;
	BN_ULONG l, tmp;
	bn_check_top(r);
	bn_check_top(a);

	if (n < 0) {
		BNerr(BN_F_BN_RSHIFT, BN_R_INVALID_SHIFT);
		return 0;
	}

	nw = n / BN_BITS2;
	rb = n % BN_BITS2;
	lb = BN_BITS2 - rb;

	if (nw >= a->top || a->top == 0) {
		BN_zero(r);
		return 1;
	}

	i = (BN_num_bits(a) - n + (BN_BITS2 - 1)) / BN_BITS2;

	if (r != a) {
		if (bn_wexpand(r, i) == NULL)
			return 0;

		r->neg = a->neg;
	} else {
		if (n == 0)
			return 1;           /* or the copying loop will go berserk */
	}

	f = &(a->d[nw]);
	t = r->d;
	j = a->top - nw;
	r->top = i;

	if (rb == 0) {
		for (i = j; i != 0; i--)
			*(t++) = *(f++);
	} else {
		l = *(f++);

		for (i = j - 1; i != 0; i--) {
			tmp = (l >> rb) & BN_MASK2;
			l = *(f++);
			*(t++) = (tmp | (l << lb)) & BN_MASK2;
		}
		l = (l >> rb);
		if ((l & BN_MASK2))
			*(t) = l;
	}

	if (!r->top)
		r->neg = 0; /* don't allow negative zero */

	bn_check_top(r);
	return 1;
}

int BN_nnmod(BIGNUM *r, const BIGNUM *m, const BIGNUM *d, BN_CTX *ctx)
{
	/*
	 * like BN_mod, but returns non-negative remainder (i.e., 0 <= r < |d|
	 * always holds)
	 */
	if (!(BN_mod(r, m, d, ctx)))
		return 0;

	if (!r->neg)
		return 1;

	/* now   -|d| < r < 0,  so we have to set  r := r + |d| */
	return (d->neg ? BN_sub : BN_add) (r, r, d);
}

int BN_mod_add(BIGNUM *r, const BIGNUM *a, const BIGNUM *b, const BIGNUM *m,
			   BN_CTX *ctx)
{
	if (!BN_add(r, a, b))
		return 0;

	return BN_nnmod(r, r, m, ctx);
}

/*
 * BN_mod_add variant that may be used if both a and b are non-negative and
 * less than m
 */
int BN_mod_add_quick(BIGNUM *r, const BIGNUM *a, const BIGNUM *b,
					 const BIGNUM *m)
{
	if (!BN_uadd(r, a, b))
		return 0;

	if (BN_ucmp(r, m) >= 0)
		return BN_usub(r, r, m);

	return 1;
}

int BN_mod_sub(BIGNUM *r, const BIGNUM *a, const BIGNUM *b, const BIGNUM *m,
			   BN_CTX *ctx)
{
	if (!BN_sub(r, a, b))
		return 0;

	return BN_nnmod(r, r, m, ctx);
}

/*
 * BN_mod_sub variant that may be used if both a and b are non-negative and
 * less than m
 */
int BN_mod_sub_quick(BIGNUM *r, const BIGNUM *a, const BIGNUM *b,
					 const BIGNUM *m)
{
	if (!BN_sub(r, a, b))
		return 0;

	if (r->neg)
		return BN_add(r, r, m);

	return 1;
}

/* slow but works */
int BN_mod_mul(BIGNUM *r, const BIGNUM *a, const BIGNUM *b, const BIGNUM *m,
			   BN_CTX *ctx)
{
	BIGNUM *t;
	int ret = 0;
	bn_check_top(a);
	bn_check_top(b);
	bn_check_top(m);
	BN_CTX_start(ctx);

	t = BN_CTX_get(ctx);
	if (t == NULL)
		goto err;

	if (a == b) {
		if (!BN_sqr(t, a, ctx))
			goto err;
	} else {
		if (!BN_mul(t, a, b, ctx))
			goto err;
	}

	if (!BN_nnmod(r, t, m, ctx))
		goto err;

	bn_check_top(r);
	ret = 1;
err:
	BN_CTX_end(ctx);
	return ret;
}

int BN_mod_sqr(BIGNUM *r, const BIGNUM *a, const BIGNUM *m, BN_CTX *ctx)
{
	if (!BN_sqr(r, a, ctx))
		return 0;

	/* r->neg == 0,  thus we don't need BN_nnmod */
	return BN_mod(r, r, m, ctx);
}

int BN_mod_lshift1(BIGNUM *r, const BIGNUM *a, const BIGNUM *m, BN_CTX *ctx)
{
	if (!BN_lshift1(r, a))
		return 0;

	bn_check_top(r);
	return BN_nnmod(r, r, m, ctx);
}

/*
 * BN_mod_lshift1 variant that may be used if a is non-negative and less than
 * m
 */
int BN_mod_lshift1_quick(BIGNUM *r, const BIGNUM *a, const BIGNUM *m)
{
	if (!BN_lshift1(r, a))
		return 0;

	bn_check_top(r);

	if (BN_cmp(r, m) >= 0)
		return BN_sub(r, r, m);

	return 1;
}

int BN_mod_lshift(BIGNUM *r, const BIGNUM *a, int n, const BIGNUM *m,
				  BN_CTX *ctx)
{
	BIGNUM *abs_m = NULL;
	int ret;

	if (!BN_nnmod(r, a, m, ctx))
		return 0;

	if (m->neg) {
		abs_m = BN_dup(m);

		if (abs_m == NULL)
			return 0;

		abs_m->neg = 0;
	}

	ret = BN_mod_lshift_quick(r, r, n, (abs_m ? abs_m : m));
	bn_check_top(r);
	BN_free(abs_m);
	return ret;
}

/*
 * BN_mod_lshift variant that may be used if a is non-negative and less than
 * m
 */
int BN_mod_lshift_quick(BIGNUM *r, const BIGNUM *a, int n, const BIGNUM *m)
{
	if (r != a) {
		if (BN_copy(r, a) == NULL)
			return 0;
	}

	while (n > 0) {
		int max_shift;
		/* 0 < r < m */
		max_shift = BN_num_bits(m) - BN_num_bits(r);
		/* max_shift >= 0 */

		if (max_shift < 0) {
			BNerr(BN_F_BN_MOD_LSHIFT_QUICK, BN_R_INPUT_NOT_REDUCED);
			return 0;
		}

		if (max_shift > n)
			max_shift = n;

		if (max_shift) {
			if (!BN_lshift(r, r, max_shift))
				return 0;

			n -= max_shift;
		} else {
			if (!BN_lshift1(r, r))
				return 0;

			--n;
		}

		/* BN_num_bits(r) <= BN_num_bits(m) */

		if (BN_cmp(r, m) >= 0) {
			if (!BN_sub(r, r, m))
				return 0;
		}
	}

	bn_check_top(r);
	return 1;
}

/* r can == a or b */
int BN_add(BIGNUM *r, const BIGNUM *a, const BIGNUM *b)
{
	int a_neg = a->neg, ret;
	bn_check_top(a);
	bn_check_top(b);

	/*-
	 *  a +  b      a+b
	 *  a + -b      a-b
	 * -a +  b      b-a
	 * -a + -b      -(a+b)
	 */
	if (a_neg ^ b->neg) {
		/* only one is negative */
		if (a_neg) {
			const BIGNUM *tmp;
			tmp = a;
			a = b;
			b = tmp;
		}

		/* we are now a - b */

		if (BN_ucmp(a, b) < 0) {
			if (!BN_usub(r, b, a))
				return 0;

			r->neg = 1;
		} else {
			if (!BN_usub(r, a, b))
				return 0;

			r->neg = 0;
		}

		return 1;
	}

	ret = BN_uadd(r, a, b);
	r->neg = a_neg;
	bn_check_top(r);
	return ret;
}

/* unsigned add of b to a */
int BN_uadd(BIGNUM *r, const BIGNUM *a, const BIGNUM *b)
{
	int max, min, dif;
	const BN_ULONG *ap, *bp;
	BN_ULONG *rp, carry, t1, t2;
	bn_check_top(a);
	bn_check_top(b);

	if (a->top < b->top) {
		const BIGNUM *tmp;
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

	while (dif) {
		dif--;
		t1 = *(ap++);
		t2 = (t1 + carry) & BN_MASK2;
		*(rp++) = t2;
		carry &= (t2 == 0);
	}

	*rp = carry;
	r->top += carry;
	r->neg = 0;
	bn_check_top(r);
	return 1;
}

/* unsigned subtraction of b from a, a must be larger than b. */
int BN_usub(BIGNUM *r, const BIGNUM *a, const BIGNUM *b)
{
	int max, min, dif;
	BN_ULONG t1, t2, borrow, *rp;
	const BN_ULONG *ap, *bp;
	bn_check_top(a);
	bn_check_top(b);
	max = a->top;
	min = b->top;
	dif = max - min;

	if (dif < 0) {              /* hmm... should not be happening */
		BNerr(BN_F_BN_USUB, BN_R_ARG2_LT_ARG3);
		return 0;
	}

	if (bn_wexpand(r, max) == NULL)
		return 0;

	ap = a->d;
	bp = b->d;
	rp = r->d;
	borrow = bn_sub_words(rp, ap, bp, min);
	ap += min;
	rp += min;

	while (dif) {
		dif--;
		t1 = *(ap++);
		t2 = (t1 - borrow) & BN_MASK2;
		*(rp++) = t2;
		borrow &= (t1 == 0);
	}

	r->top = max;
	r->neg = 0;
	bn_correct_top(r);
	return 1;
}

int BN_sub(BIGNUM *r, const BIGNUM *a, const BIGNUM *b)
{
	int max;
	int add = 0, neg = 0;
	bn_check_top(a);
	bn_check_top(b);

	/*-
	 *  a -  b      a-b
	 *  a - -b      a+b
	 * -a -  b      -(a+b)
	 * -a - -b      b-a
	 */
	if (a->neg) {
		if (b->neg) {
			const BIGNUM *tmp;
			tmp = a;
			a = b;
			b = tmp;
		} else {
			add = 1;
			neg = 1;
		}
	} else {
		if (b->neg) {
			add = 1;
			neg = 0;
		}
	}

	if (add) {
		if (!BN_uadd(r, a, b))
			return 0;

		r->neg = neg;
		return 1;
	}

	/* We are actually doing a - b :-) */
	max = (a->top > b->top) ? a->top : b->top;

	if (bn_wexpand(r, max) == NULL)
		return 0;

	if (BN_ucmp(a, b) < 0) {
		if (!BN_usub(r, b, a))
			return 0;

		r->neg = 1;
	} else {
		if (!BN_usub(r, a, b))
			return 0;

		r->neg = 0;
	}

	bn_check_top(r);
	return 1;
}

BN_ULONG BN_mod_word(const BIGNUM *a, BN_ULONG w)
{
#ifndef BN_LLONG
	BN_ULONG ret = 0;
#else
	BN_ULLONG ret = 0;
#endif
	int i;

	if (w == 0)
		return (BN_ULONG)-1;

#ifndef BN_LLONG

	/*
	 * If |w| is too long and we don't have BN_ULLONG then we need to fall
	 * back to using BN_div_word
	 */
	if (w > ((BN_ULONG)1 << BN_BITS4)) {
		BIGNUM *tmp = BN_dup(a);

		if (tmp == NULL)
			return (BN_ULONG)-1;

		ret = BN_div_word(tmp, w);
		BN_free(tmp);
		return ret;
	}

#endif
	bn_check_top(a);
	w &= BN_MASK2;

	for (i = a->top - 1; i >= 0; i--) {
#ifndef BN_LLONG
		/*
		 * We can assume here that | w <= ((BN_ULONG)1 << BN_BITS4) | and so
		 * | ret < ((BN_ULONG)1 << BN_BITS4) | and therefore the shifts here are
		 * safe and will not overflow
		 */
		ret = ((ret << BN_BITS4) | ((a->d[i] >> BN_BITS4) & BN_MASK2l)) % w;
		ret = ((ret << BN_BITS4) | (a->d[i] & BN_MASK2l)) % w;
#else
		ret = (BN_ULLONG) (((ret << (BN_ULLONG) BN_BITS2) | a->d[i]) %
						   (BN_ULLONG) w);
#endif
	}

	return ((BN_ULONG)ret);
}

BN_ULONG BN_div_word(BIGNUM *a, BN_ULONG w)
{
	BN_ULONG ret = 0;
	int i, j;
	bn_check_top(a);
	w &= BN_MASK2;

	if (!w)
		/* actually this an error (division by zero) */
		return (BN_ULONG)-1;

	if (a->top == 0)
		return 0;

	/* normalize input (so bn_div_words doesn't complain) */
	j = BN_BITS2 - BN_num_bits_word(w);
	w <<= j;

	if (!BN_lshift(a, a, j))
		return (BN_ULONG)-1;

	for (i = a->top - 1; i >= 0; i--) {
		BN_ULONG l, d;
		l = a->d[i];
		d = bn_div_words(ret, l, w);
		ret = (l - ((d * w) & BN_MASK2)) & BN_MASK2;
		a->d[i] = d;
	}

	if ((a->top > 0) && (a->d[a->top - 1] == 0))
		a->top--;

	ret >>= j;

	if (!a->top)
		a->neg = 0; /* don't allow negative zero */

	bn_check_top(a);
	return ret;
}

int BN_add_word(BIGNUM *a, BN_ULONG w)
{
	BN_ULONG l;
	int i;
	bn_check_top(a);
	w &= BN_MASK2;

	/* degenerate case: w is zero */
	if (!w)
		return 1;

	/* degenerate case: a is zero */
	if (BN_is_zero(a))
		return BN_set_word(a, w);

	/* handle 'a' when negative */
	if (a->neg) {
		a->neg = 0;
		i = BN_sub_word(a, w);

		if (!BN_is_zero(a))
			a->neg = !(a->neg);

		return i;
	}

	for (i = 0; w != 0 && i < a->top; i++) {
		a->d[i] = l = (a->d[i] + w) & BN_MASK2;
		w = (w > l) ? 1 : 0;
	}

	if (w && i == a->top) {
		if (bn_wexpand(a, a->top + 1) == NULL)
			return 0;

		a->top++;
		a->d[i] = w;
	}

	bn_check_top(a);
	return 1;
}

int BN_sub_word(BIGNUM *a, BN_ULONG w)
{
	int i;
	bn_check_top(a);
	w &= BN_MASK2;

	/* degenerate case: w is zero */
	if (!w)
		return 1;

	/* degenerate case: a is zero */
	if (BN_is_zero(a)) {
		i = BN_set_word(a, w);

		if (i != 0)
			BN_set_negative(a, 1);

		return i;
	}

	/* handle 'a' when negative */
	if (a->neg) {
		a->neg = 0;
		i = BN_add_word(a, w);
		a->neg = 1;
		return i;
	}

	if ((a->top == 1) && (a->d[0] < w)) {
		a->d[0] = w - a->d[0];
		a->neg = 1;
		return 1;
	}

	i = 0;

	for (;;) {
		if (a->d[i] >= w) {
			a->d[i] -= w;
			break;
		} else {
			a->d[i] = (a->d[i] - w) & BN_MASK2;
			i++;
			w = 1;
		}
	}

	if ((a->d[i] == 0) && (i == (a->top - 1)))
		a->top--;

	bn_check_top(a);
	return 1;
}

int BN_mul_word(BIGNUM *a, BN_ULONG w)
{
	BN_ULONG ll;
	bn_check_top(a);
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

	bn_check_top(a);
	return 1;
}

/*-
 * BN_div computes  dv := num / divisor, rounding towards
 * zero, and sets up rm  such that  dv*divisor + rm = num  holds.
 * Thus:
 *     dv->neg == num->neg ^ divisor->neg  (unless the result is zero)
 *     rm->neg == num->neg                 (unless the remainder is zero)
 * If 'dv' or 'rm' is NULL, the respective value is not returned.
 */
int BN_div(BIGNUM *dv, BIGNUM *rm, const BIGNUM *num, const BIGNUM *divisor,
		   BN_CTX *ctx)
{
	int norm_shift, i, loop;
	BIGNUM *tmp, wnum, *snum, *sdiv, *res;
	BN_ULONG *resp, *wnump;
	BN_ULONG d0, d1;
	int num_n, div_n;
	int no_branch = 0;

	/*
	 * Invalid zero-padding would have particularly bad consequences so don't
	 * just rely on bn_check_top() here (bn_check_top() works only for
	 * BN_DEBUG builds)
	 */
	if ((num->top > 0 && num->d[num->top - 1] == 0) ||
		(divisor->top > 0 && divisor->d[divisor->top - 1] == 0)) {
		BNerr(BN_F_BN_DIV, BN_R_NOT_INITIALIZED);
		return 0;
	}

	bn_check_top(num);
	bn_check_top(divisor);

	if ((BN_get_flags(num, BN_FLG_CONSTTIME) != 0)
		|| (BN_get_flags(divisor, BN_FLG_CONSTTIME) != 0))
		no_branch = 1;

	bn_check_top(dv);
	bn_check_top(rm);
	/*- bn_check_top(num); *//*
	 * 'num' has been checked already
	 */
	/*- bn_check_top(divisor); *//*
	 * 'divisor' has been checked already
	 */

	if (BN_is_zero(divisor)) {
		BNerr(BN_F_BN_DIV, BN_R_DIV_BY_ZERO);
		return 0;
	}

	if (!no_branch && BN_ucmp(num, divisor) < 0) {
		if (rm != NULL) {
			if (BN_copy(rm, num) == NULL)
				return 0;
		}

		if (dv != NULL)
			BN_zero(dv);

		return 1;
	}

	BN_CTX_start(ctx);
	tmp = BN_CTX_get(ctx);
	snum = BN_CTX_get(ctx);
	sdiv = BN_CTX_get(ctx);

	if (dv == NULL)
		res = BN_CTX_get(ctx);
	else
		res = dv;

	if (sdiv == NULL || res == NULL || tmp == NULL || snum == NULL)
		goto err;

	/* First we normalise the numbers */
	norm_shift = BN_BITS2 - ((BN_num_bits(divisor)) % BN_BITS2);

	if (!(BN_lshift(sdiv, divisor, norm_shift)))
		goto err;

	sdiv->neg = 0;
	norm_shift += BN_BITS2;

	if (!(BN_lshift(snum, num, norm_shift)))
		goto err;

	snum->neg = 0;

	if (no_branch) {
		/*
		 * Since we don't know whether snum is larger than sdiv, we pad snum
		 * with enough zeroes without changing its value.
		 */
		if (snum->top <= sdiv->top + 1) {
			if (bn_wexpand(snum, sdiv->top + 2) == NULL)
				goto err;

			for (i = snum->top; i < sdiv->top + 2; i++)
				snum->d[i] = 0;

			snum->top = sdiv->top + 2;
		} else {
			if (bn_wexpand(snum, snum->top + 1) == NULL)
				goto err;

			snum->d[snum->top] = 0;
			snum->top++;
		}
	}

	div_n = sdiv->top;
	num_n = snum->top;
	loop = num_n - div_n;
	/*
	 * Lets setup a 'window' into snum This is the part that corresponds to
	 * the current 'area' being divided
	 */
	wnum.neg = 0;
	wnum.d = &(snum->d[loop]);
	wnum.top = div_n;
	/*
	 * only needed when BN_ucmp messes up the values between top and max
	 */
	wnum.dmax = snum->dmax - loop; /* so we don't step out of bounds */
	/* Get the top 2 words of sdiv */
	/* div_n=sdiv->top; */
	d0 = sdiv->d[div_n - 1];
	d1 = (div_n == 1) ? 0 : sdiv->d[div_n - 2];
	/* pointer to the 'top' of snum */
	wnump = &(snum->d[num_n - 1]);

	/* Setup to 'res' */
	if (!bn_wexpand(res, (loop + 1)))
		goto err;

	res->neg = (num->neg ^ divisor->neg);
	res->top = loop - no_branch;
	resp = &(res->d[loop - 1]);

	/* space for temp */
	if (!bn_wexpand(tmp, (div_n + 1)))
		goto err;

	if (!no_branch) {
		if (BN_ucmp(&wnum, sdiv) >= 0) {
			/*
			 * If BN_DEBUG_RAND is defined BN_ucmp changes (via bn_pollute)
			 * the const bignum arguments => clean the values between top and
			 * max again
			 */
			bn_sub_words(wnum.d, wnum.d, sdiv->d, div_n);
			*resp = 1;
		} else
			res->top--;
	}

	/* Increase the resp pointer so that we never create an invalid pointer. */
	resp++;

	/*
	 * if res->top == 0 then clear the neg value otherwise decrease the resp
	 * pointer
	 */
	if (res->top == 0)
		res->neg = 0;
	else
		resp--;

	for (i = 0; i < loop - 1; i++, wnump--) {
		BN_ULONG q, l0;
		/*
		 * the first part of the loop uses the top two words of snum and sdiv
		 * to calculate a BN_ULONG q such that | wnum - sdiv * q | < sdiv
		 */
# if defined(BN_DIV3W) && !defined(OPENSSL_NO_ASM)
		BN_ULONG bn_div_3_words(BN_ULONG *, BN_ULONG, BN_ULONG);
		q = bn_div_3_words(wnump, d1, d0);
# else
		BN_ULONG n0, n1, rem = 0;
		n0 = wnump[0];
		n1 = wnump[-1];

		if (n0 == d0)
			q = BN_MASK2;
		else {                  /* n0 < d0 */
#  ifdef BN_LLONG
			BN_ULLONG t2;
#   if defined(BN_LLONG) && defined(BN_DIV2W) && !defined(bn_div_words)
			q = (BN_ULONG)(((((BN_ULLONG) n0) << BN_BITS2) | n1) / d0);
#   else
			q = bn_div_words(n0, n1, d0);
#   endif
#   ifndef REMAINDER_IS_ALREADY_CALCULATED
			/*
			 * rem doesn't have to be BN_ULLONG. The least we
			 * know it's less that d0, isn't it?
			 */
			rem = (n1 - q * d0) & BN_MASK2;
#   endif
			t2 = (BN_ULLONG) d1 * q;

			for (;;) {
				if (t2 <= ((((BN_ULLONG) rem) << BN_BITS2) | wnump[-2]))
					break;

				q--;
				rem += d0;

				if (rem < d0)
					break;      /* don't let rem overflow */

				t2 -= d1;
			}

#  else                         /* !BN_LLONG */
			BN_ULONG t2l, t2h;
			q = bn_div_words(n0, n1, d0);
#   ifndef REMAINDER_IS_ALREADY_CALCULATED
			rem = (n1 - q * d0) & BN_MASK2;
#   endif
#   if defined(BN_UMULT_LOHI)
			BN_UMULT_LOHI(t2l, t2h, d1, q);
#   elif defined(BN_UMULT_HIGH)
			t2l = d1 * q;
			t2h = BN_UMULT_HIGH(d1, q);
#   else
			{
				BN_ULONG ql, qh;
				t2l = LBITS(d1);
				t2h = HBITS(d1);
				ql = LBITS(q);
				qh = HBITS(q);
				mul64(t2l, t2h, ql, qh); /* t2=(BN_ULLONG)d1*q; */
			}
#   endif

			for (;;) {
				if ((t2h < rem) || ((t2h == rem) && (t2l <= wnump[-2])))
					break;

				q--;
				rem += d0;

				if (rem < d0)
					break;      /* don't let rem overflow */

				if (t2l < d1)
					t2h--;

				t2l -= d1;
			}

#  endif                        /* !BN_LLONG */
		}

# endif                         /* !BN_DIV3W */
		l0 = bn_mul_words(tmp->d, sdiv->d, div_n, q);
		tmp->d[div_n] = l0;
		wnum.d--;

		/*
		 * ingore top values of the bignums just sub the two BN_ULONG arrays
		 * with bn_sub_words
		 */
		if (bn_sub_words(wnum.d, wnum.d, tmp->d, div_n + 1)) {
			/*
			 * Note: As we have considered only the leading two BN_ULONGs in
			 * the calculation of q, sdiv * q might be greater than wnum (but
			 * then (q-1) * sdiv is less or equal than wnum)
			 */
			q--;

			if (bn_add_words(wnum.d, wnum.d, sdiv->d, div_n))
				/*
				 * we can't have an overflow here (assuming that q != 0, but
				 * if q == 0 then tmp is zero anyway)
				 */
				(*wnump)++;
		}

		/* store part of the result */
		resp--;
		*resp = q;
	}

	bn_correct_top(snum);

	if (rm != NULL) {
		/*
		 * Keep a copy of the neg flag in num because if rm==num BN_rshift()
		 * will overwrite it.
		 */
		int neg = num->neg;
		BN_rshift(rm, snum, norm_shift);

		if (!BN_is_zero(rm))
			rm->neg = neg;

		bn_check_top(rm);
	}

	if (no_branch)
		bn_correct_top(res);

	BN_CTX_end(ctx);
	return 1;
err:
	bn_check_top(rm);
	BN_CTX_end(ctx);
	return 0;
}

static inline unsigned int constant_time_msb(unsigned int a)
{
	return 0 - (a >> (sizeof(a) * 8 - 1));
}


static inline unsigned int constant_time_is_zero(unsigned int a)
{
	return constant_time_msb(~a & (a - 1));
}


static inline unsigned int constant_time_eq(unsigned int a,
		unsigned int b)
{
	return constant_time_is_zero(a ^ b);
}

static inline unsigned int constant_time_eq_int(int a, int b)
{
	return constant_time_eq((unsigned)(a), (unsigned)(b));
}


/* maximum precomputation table size for *variable* sliding windows */
#define TABLESIZE      32

/* this one works - simple but works */
int BN_exp(BIGNUM *r, const BIGNUM *a, const BIGNUM *p, BN_CTX *ctx)
{
	int i, bits, ret = 0;
	BIGNUM *v, *rr;

	if (BN_get_flags(p, BN_FLG_CONSTTIME) != 0
		|| BN_get_flags(a, BN_FLG_CONSTTIME) != 0) {
		/* BN_FLG_CONSTTIME only supported by BN_mod_exp_mont() */
		BNerr(BN_F_BN_EXP, ERR_R_SHOULD_NOT_HAVE_BEEN_CALLED);
		return 0;
	}

	BN_CTX_start(ctx);

	if ((r == a) || (r == p))
		rr = BN_CTX_get(ctx);
	else
		rr = r;

	v = BN_CTX_get(ctx);

	if (rr == NULL || v == NULL)
		goto err;

	if (BN_copy(v, a) == NULL)
		goto err;

	bits = BN_num_bits(p);

	if (BN_is_odd(p)) {
		if (BN_copy(rr, a) == NULL)
			goto err;
	} else {
		if (!BN_one(rr))
			goto err;
	}

	for (i = 1; i < bits; i++) {
		if (!BN_sqr(v, v, ctx))
			goto err;

		if (BN_is_bit_set(p, i)) {
			if (!BN_mul(rr, rr, v, ctx))
				goto err;
		}
	}

	if (r != rr && BN_copy(r, rr) == NULL)
		goto err;

	ret = 1;
err:
	BN_CTX_end(ctx);
	bn_check_top(r);
	return ret;
}

int BN_mod_exp(BIGNUM *r, const BIGNUM *a, const BIGNUM *p, const BIGNUM *m,
			   BN_CTX *ctx)
{
	int ret;
	bn_check_top(a);
	bn_check_top(p);
	bn_check_top(m);
	/*-
	 * For even modulus  m = 2^k*m_odd, it might make sense to compute
	 * a^p mod m_odd  and  a^p mod 2^k  separately (with Montgomery
	 * exponentiation for the odd part), using appropriate exponent
	 * reductions, and combine the results using the CRT.
	 *
	 * For now, we use Montgomery only if the modulus is odd; otherwise,
	 * exponentiation using the reciprocal-based quick remaindering
	 * algorithm is used.
	 *
	 * (Timing obtained with expspeed.c [computations  a^p mod m
	 * where  a, p, m  are of the same length: 256, 512, 1024, 2048,
	 * 4096, 8192 bits], compared to the running time of the
	 * standard algorithm:
	 *
	 *   BN_mod_exp_mont   33 .. 40 %  [AMD K6-2, Linux, debug configuration]
	 *                     55 .. 77 %  [UltraSparc processor, but
	 *                                  debug-solaris-sparcv8-gcc conf.]
	 *
	 *   BN_mod_exp_recp   50 .. 70 %  [AMD K6-2, Linux, debug configuration]
	 *                     62 .. 118 % [UltraSparc, debug-solaris-sparcv8-gcc]
	 *
	 * On the Sparc, BN_mod_exp_recp was faster than BN_mod_exp_mont
	 * at 2048 and more bits, but at 512 and 1024 bits, it was
	 * slower even than the standard algorithm!
	 *
	 * "Real" timings [linux-elf, solaris-sparcv9-gcc configurations]
	 * should be obtained when the new Montgomery reduction code
	 * has been integrated into OpenSSL.)
	 */
#define MONT_MUL_MOD
#define MONT_EXP_WORD
#define RECP_MUL_MOD
#ifdef MONT_MUL_MOD
	/*
	 * I have finally been able to take out this pre-condition of the top bit
	 * being set.  It was caused by an error in BN_div with negatives.  There
	 * was also another problem when for a^b%m a >= m.  eay 07-May-97
	 */
	/* if ((m->d[m->top-1]&BN_TBIT) && BN_is_odd(m)) */

	if (BN_is_odd(m)) {
# ifdef MONT_EXP_WORD

		if (a->top == 1 && !a->neg
			&& (BN_get_flags(p, BN_FLG_CONSTTIME) == 0)
			&& (BN_get_flags(a, BN_FLG_CONSTTIME) == 0)
			&& (BN_get_flags(m, BN_FLG_CONSTTIME) == 0)) {
			BN_ULONG A = a->d[0];
			ret = BN_mod_exp_mont_word(r, A, p, m, ctx, NULL);
		} else
# endif
			ret = BN_mod_exp_mont(r, a, p, m, ctx, NULL);
	} else
#endif
#ifdef RECP_MUL_MOD
	{
		ret = BN_mod_exp_recp(r, a, p, m, ctx);
	}

#else
	{
		ret = BN_mod_exp_simple(r, a, p, m, ctx);
	}
#endif
	bn_check_top(r);
	return ret;
}

int BN_mod_exp_recp(BIGNUM *r, const BIGNUM *a, const BIGNUM *p,
					const BIGNUM *m, BN_CTX *ctx)
{
	int i, j, bits, ret = 0, wstart, wend, window, wvalue;
	int start = 1;
	BIGNUM *aa;
	/* Table of variables obtained from 'ctx' */
	BIGNUM *val[TABLESIZE];
	BN_RECP_CTX recp;

	if (BN_get_flags(p, BN_FLG_CONSTTIME) != 0
		|| BN_get_flags(a, BN_FLG_CONSTTIME) != 0
		|| BN_get_flags(m, BN_FLG_CONSTTIME) != 0) {
		/* BN_FLG_CONSTTIME only supported by BN_mod_exp_mont() */
		BNerr(BN_F_BN_MOD_EXP_RECP, ERR_R_SHOULD_NOT_HAVE_BEEN_CALLED);
		return 0;
	}

	bits = BN_num_bits(p);

	if (bits == 0) {
		/* x**0 mod 1 is still zero. */
		if (BN_is_one(m)) {
			ret = 1;
			BN_zero(r);
		} else
			ret = BN_one(r);

		return ret;
	}

	BN_CTX_start(ctx);
	aa = BN_CTX_get(ctx);
	val[0] = BN_CTX_get(ctx);

	if (!aa || !val[0])
		goto err;

	BN_RECP_CTX_init(&recp);

	if (m->neg) {
		/* ignore sign of 'm' */
		if (!BN_copy(aa, m))
			goto err;

		aa->neg = 0;

		if (BN_RECP_CTX_set(&recp, aa, ctx) <= 0)
			goto err;
	} else {
		if (BN_RECP_CTX_set(&recp, m, ctx) <= 0)
			goto err;
	}

	if (!BN_nnmod(val[0], a, m, ctx))
		goto err;               /* 1 */

	if (BN_is_zero(val[0])) {
		BN_zero(r);
		ret = 1;
		goto err;
	}

	window = BN_window_bits_for_exponent_size(bits);

	if (window > 1) {
		if (!BN_mod_mul_reciprocal(aa, val[0], val[0], &recp, ctx))
			goto err;           /* 2 */

		j = 1 << (window - 1);

		for (i = 1; i < j; i++) {
			val[i] = BN_CTX_get(ctx);
			if ((val[i] == NULL) ||
				!BN_mod_mul_reciprocal(val[i], val[i - 1], aa, &recp, ctx))
				goto err;
		}
	}

	start = 1; /* This is used to avoid multiplication etc when there is only the value '1' in the buffer. */
	wvalue = 0;                 /* The 'value' of the window */
	wstart = bits - 1;          /* The top bit of the window */
	wend = 0;                   /* The bottom bit of the window */

	if (!BN_one(r))
		goto err;

	for (;;) {
		if (BN_is_bit_set(p, wstart) == 0) {
			if (!start)
				if (!BN_mod_mul_reciprocal(r, r, r, &recp, ctx))
					goto err;

			if (wstart == 0)
				break;

			wstart--;
			continue;
		}

		/*
		 * We now have wstart on a 'set' bit, we now need to work out how bit
		 * a window to do.  To do this we need to scan forward until the last
		 * set bit before the end of the window
		 */
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
				if (!BN_mod_mul_reciprocal(r, r, r, &recp, ctx))
					goto err;
			}

		/* wvalue will be an odd number < 2^window */
		if (!BN_mod_mul_reciprocal(r, r, val[wvalue >> 1], &recp, ctx))
			goto err;

		/* move the 'window' down further */
		wstart -= wend + 1;
		wvalue = 0;
		start = 0;

		if (wstart < 0)
			break;
	}

	ret = 1;
err:
	BN_CTX_end(ctx);
	BN_RECP_CTX_free(&recp);
	bn_check_top(r);
	return ret;
}

int BN_mod_exp_mont(BIGNUM *rr, const BIGNUM *a, const BIGNUM *p,
					const BIGNUM *m, BN_CTX *ctx, BN_MONT_CTX *in_mont)
{
	int i, j, bits, ret = 0, wstart, wend, window, wvalue;
	int start = 1;
	BIGNUM *d, *r;
	const BIGNUM *aa;
	/* Table of variables obtained from 'ctx' */
	BIGNUM *val[TABLESIZE];
	BN_MONT_CTX *mont = NULL;

	if (BN_get_flags(p, BN_FLG_CONSTTIME) != 0
		|| BN_get_flags(a, BN_FLG_CONSTTIME) != 0
		|| BN_get_flags(m, BN_FLG_CONSTTIME) != 0)
		return BN_mod_exp_mont_consttime(rr, a, p, m, ctx, in_mont);

	bn_check_top(a);
	bn_check_top(p);
	bn_check_top(m);

	if (!BN_is_odd(m)) {
		BNerr(BN_F_BN_MOD_EXP_MONT, BN_R_CALLED_WITH_EVEN_MODULUS);
		return 0;
	}

	bits = BN_num_bits(p);

	if (bits == 0) {
		/* x**0 mod 1 is still zero. */
		if (BN_is_one(m)) {
			ret = 1;
			BN_zero(rr);
		} else
			ret = BN_one(rr);

		return ret;
	}

	BN_CTX_start(ctx);
	d = BN_CTX_get(ctx);
	r = BN_CTX_get(ctx);
	val[0] = BN_CTX_get(ctx);

	if (!d || !r || !val[0])
		goto err;

	/*
	 * If this is not done, things will break in the montgomery part
	 */

	if (in_mont != NULL)
		mont = in_mont;
	else {
		mont = BN_MONT_CTX_new();
		if (mont == NULL)
			goto err;

		if (!BN_MONT_CTX_set(mont, m, ctx))
			goto err;
	}

	if (a->neg || BN_ucmp(a, m) >= 0) {
		if (!BN_nnmod(val[0], a, m, ctx))
			goto err;

		aa = val[0];
	} else
		aa = a;

	if (BN_is_zero(aa)) {
		BN_zero(rr);
		ret = 1;
		goto err;
	}

	if (!BN_to_montgomery(val[0], aa, mont, ctx))
		goto err;               /* 1 */

	window = BN_window_bits_for_exponent_size(bits);

	if (window > 1) {
		if (!BN_mod_mul_montgomery(d, val[0], val[0], mont, ctx))
			goto err;           /* 2 */

		j = 1 << (window - 1);

		for (i = 1; i < j; i++) {
			val[i] = BN_CTX_get(ctx);
			if ((val[i] == NULL) ||
				!BN_mod_mul_montgomery(val[i], val[i - 1], d, mont, ctx))
				goto err;
		}
	}

	start = 1; /* This is used to avoid multiplication etc when there is only the value '1' in the buffer. */
	wvalue = 0;                 /* The 'value' of the window */
	wstart = bits - 1;          /* The top bit of the window */
	wend = 0;                   /* The bottom bit of the window */
	j = m->top;                 /* borrow j */

	if (m->d[j - 1] & (((BN_ULONG)1) << (BN_BITS2 - 1))) {
		if (bn_wexpand(r, j) == NULL)
			goto err;

		/* 2^(top*BN_BITS2) - m */
		r->d[0] = (0 - m->d[0]) & BN_MASK2;

		for (i = 1; i < j; i++)
			r->d[i] = (~m->d[i]) & BN_MASK2;

		r->top = j;
		/*
		 * Upper words will be zero if the corresponding words of 'm' were
		 * 0xfff[...], so decrement r->top accordingly.
		 */
		bn_correct_top(r);
	} else
		if (!BN_to_montgomery(r, BN_value_one(), mont, ctx))
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

		/*
		 * We now have wstart on a 'set' bit, we now need to work out how bit
		 * a window to do.  To do this we need to scan forward until the last
		 * set bit before the end of the window
		 */
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

#if defined(SPARC_T4_MONT)

	if (OPENSSL_sparcv9cap_P[0] & (SPARCV9_VIS3 | SPARCV9_PREFER_FPU)) {
		j = mont->N.top;        /* borrow j */
		val[0]->d[0] = 1;       /* borrow val[0] */

		for (i = 1; i < j; i++)
			val[0]->d[i] = 0;

		val[0]->top = j;

		if (!BN_mod_mul_montgomery(rr, r, val[0], mont, ctx))
			goto err;
	} else
#endif
		if (!BN_from_montgomery(rr, r, mont, ctx))
			goto err;

	ret = 1;
err:

	if (in_mont == NULL)
		BN_MONT_CTX_free(mont);

	BN_CTX_end(ctx);
	bn_check_top(rr);
	return ret;
}

#if defined(SPARC_T4_MONT)
static BN_ULONG bn_get_bits(const BIGNUM *a, int bitpos)
{
	BN_ULONG ret = 0;
	int wordpos;
	wordpos = bitpos / BN_BITS2;
	bitpos %= BN_BITS2;

	if (wordpos >= 0 && wordpos < a->top) {
		ret = a->d[wordpos] & BN_MASK2;

		if (bitpos) {
			ret >>= bitpos;

			if (++wordpos < a->top)
				ret |= a->d[wordpos] << (BN_BITS2 - bitpos);
		}
	}

	return ret & BN_MASK2;
}
#endif

/*
 * BN_mod_exp_mont_consttime() stores the precomputed powers in a specific
 * layout so that accessing any of these table values shows the same access
 * pattern as far as cache lines are concerned.  The following functions are
 * used to transfer a BIGNUM from/to that table.
 */

static int MOD_EXP_CTIME_COPY_TO_PREBUF(const BIGNUM *b, int top,
										unsigned char *buf, int idx,
										int window)
{
	int i, j;
	int width = 1 << window;
	BN_ULONG *table = (BN_ULONG *)buf;

	if (top > b->top)
		top = b->top; /* this works because 'buf' is explicitly zeroed */

	for (i = 0, j = idx; i < top; i++, j += width)
		table[j] = b->d[i];

	return 1;
}

static int MOD_EXP_CTIME_COPY_FROM_PREBUF(BIGNUM *b, int top,
		unsigned char *buf, int idx,
		int window)
{
	int i, j;
	int width = 1 << window;
	/*
	 * We declare table 'volatile' in order to discourage compiler
	 * from reordering loads from the table. Concern is that if
	 * reordered in specific manner loads might give away the
	 * information we are trying to conceal. Some would argue that
	 * compiler can reorder them anyway, but it can as well be
	 * argued that doing so would be violation of standard...
	 */
	volatile BN_ULONG *table = (volatile BN_ULONG *)buf;

	if (bn_wexpand(b, top) == NULL)
		return 0;

	if (window <= 3) {
		for (i = 0; i < top; i++, table += width) {
			BN_ULONG acc = 0;

			for (j = 0; j < width; j++) {
				acc |= table[j] &
					   ((BN_ULONG)0 - (constant_time_eq_int(j, idx) & 1));
			}

			b->d[i] = acc;
		}
	} else {
		int xstride = 1 << (window - 2);
		BN_ULONG y0, y1, y2, y3;
		i = idx >> (window - 2);        /* equivalent of idx / xstride */
		idx &= xstride - 1;             /* equivalent of idx % xstride */
		y0 = (BN_ULONG)0 - (constant_time_eq_int(i, 0) & 1);
		y1 = (BN_ULONG)0 - (constant_time_eq_int(i, 1) & 1);
		y2 = (BN_ULONG)0 - (constant_time_eq_int(i, 2) & 1);
		y3 = (BN_ULONG)0 - (constant_time_eq_int(i, 3) & 1);

		for (i = 0; i < top; i++, table += width) {
			BN_ULONG acc = 0;

			for (j = 0; j < xstride; j++) {
				acc |= ((table[j + 0 * xstride] & y0) |
						 (table[j + 1 * xstride] & y1) |
						 (table[j + 2 * xstride] & y2) |
						 (table[j + 3 * xstride] & y3))
					   & ((BN_ULONG)0 - (constant_time_eq_int(j, idx) & 1));
			}

			b->d[i] = acc;
		}
	}

	b->top = top;
	bn_correct_top(b);
	return 1;
}

/*
 * Given a pointer value, compute the next address that is a cache line
 * multiple.
 */
#define MOD_EXP_CTIME_ALIGN(x_) \
	((unsigned char *)(x_) + (MOD_EXP_CTIME_MIN_CACHE_LINE_WIDTH - (((size_t)(x_)) & (MOD_EXP_CTIME_MIN_CACHE_LINE_MASK))))

/*
 * This variant of BN_mod_exp_mont() uses fixed windows and the special
 * precomputation memory layout to limit data-dependency to a minimum to
 * protect secret exponents (cf. the hyper-threading timing attacks pointed
 * out by Colin Percival,
 * http://www.daemonology.net/hyperthreading-considered-harmful/)
 */
int BN_mod_exp_mont_consttime(BIGNUM *rr, const BIGNUM *a, const BIGNUM *p,
							  const BIGNUM *m, BN_CTX *ctx,
							  BN_MONT_CTX *in_mont)
{
	int i, bits, ret = 0, window, wvalue;
	int top;
	BN_MONT_CTX *mont = NULL;
	int numPowers;
	unsigned char *powerbufFree = NULL;
	int powerbufLen = 0;
	unsigned char *powerbuf = NULL;
	BIGNUM tmp, am;
#if defined(SPARC_T4_MONT)
	unsigned int t4 = 0;
#endif
	bn_check_top(a);
	bn_check_top(p);
	bn_check_top(m);

	if (!BN_is_odd(m)) {
		BNerr(BN_F_BN_MOD_EXP_MONT_CONSTTIME, BN_R_CALLED_WITH_EVEN_MODULUS);
		return 0;
	}

	top = m->top;
	bits = BN_num_bits(p);

	if (bits == 0) {
		/* x**0 mod 1 is still zero. */
		if (BN_is_one(m)) {
			ret = 1;
			BN_zero(rr);
		} else
			ret = BN_one(rr);

		return ret;
	}

	BN_CTX_start(ctx);

	/*
	 * Allocate a montgomery context if it was not supplied by the caller. If
	 * this is not done, things will break in the montgomery part.
	 */
	if (in_mont != NULL)
		mont = in_mont;
	else {
		mont = BN_MONT_CTX_new();
		if (mont == NULL)
			goto err;

		if (!BN_MONT_CTX_set(mont, m, ctx))
			goto err;
	}

#ifdef RSAZ_ENABLED

	/*
	 * If the size of the operands allow it, perform the optimized
	 * RSAZ exponentiation. For further information see
	 * crypto/bn/rsaz_exp.c and accompanying assembly modules.
	 */
	if ((16 == a->top) && (16 == p->top) && (BN_num_bits(m) == 1024)
		&& rsaz_avx2_eligible()) {
		if (NULL == bn_wexpand(rr, 16))
			goto err;

		RSAZ_1024_mod_exp_avx2(rr->d, a->d, p->d, m->d, mont->RR.d,
							   mont->n0[0]);
		rr->top = 16;
		rr->neg = 0;
		bn_correct_top(rr);
		ret = 1;
		goto err;
	} else if ((8 == a->top) && (8 == p->top) && (BN_num_bits(m) == 512)) {
		if (NULL == bn_wexpand(rr, 8))
			goto err;

		RSAZ_512_mod_exp(rr->d, a->d, p->d, m->d, mont->n0[0], mont->RR.d);
		rr->top = 8;
		rr->neg = 0;
		bn_correct_top(rr);
		ret = 1;
		goto err;
	}

#endif
	/* Get the window size to use with size of p. */
	window = BN_window_bits_for_ctime_exponent_size(bits);
#if defined(SPARC_T4_MONT)

	if (window >= 5 && (top & 15) == 0 && top <= 64 &&
		(OPENSSL_sparcv9cap_P[1] & (CFR_MONTMUL | CFR_MONTSQR)) ==
		(CFR_MONTMUL | CFR_MONTSQR) && (OPENSSL_sparcv9cap_P[0])) {
		t4 = OPENSSL_sparcv9cap_P[0];
		window = 5;
	} else
#endif
#if defined(OPENSSL_BN_ASM_MONT5)
		if (window >= 5) {
			window = 5;             /* ~5% improvement for RSA2048 sign, and even for RSA4096 */
			/* reserve space for mont->N.d[] copy */
			powerbufLen += top * sizeof(mont->N.d[0]);
		}

#endif
	(void)0;
	/*
	 * Allocate a buffer large enough to hold all of the pre-computed powers
	 * of am, am itself and tmp.
	 */
	numPowers = 1 << window;
	powerbufLen += sizeof(m->d[0]) * (top * numPowers +
									  ((2 * top) >
									   numPowers ? (2 * top) : numPowers));
#ifdef alloca

	if (powerbufLen < 3072)
		powerbufFree =
			alloca(powerbufLen + MOD_EXP_CTIME_MIN_CACHE_LINE_WIDTH);
	else
#endif
		os_alloc_mem(NULL, (UCHAR **) &powerbufFree, powerbufLen + MOD_EXP_CTIME_MIN_CACHE_LINE_WIDTH);

	if (powerbufFree == NULL)
		goto err;

	powerbuf = MOD_EXP_CTIME_ALIGN(powerbufFree);
	memset(powerbuf, 0, powerbufLen);
#ifdef alloca

	if (powerbufLen < 3072)
		powerbufFree = NULL;

#endif
	/* lay down tmp and am right after powers table */
	tmp.d = (BN_ULONG *)(powerbuf + sizeof(m->d[0]) * top * numPowers);
	am.d = tmp.d + top;
	tmp.top = am.top = 0;
	tmp.dmax = am.dmax = top;
	tmp.neg = am.neg = 0;
	tmp.flags = am.flags = BN_FLG_STATIC_DATA;
	/* prepare a^0 in Montgomery domain */

	if (m->d[top - 1] & (((BN_ULONG)1) << (BN_BITS2 - 1))) {
		/* 2^(top*BN_BITS2) - m */
		tmp.d[0] = (0 - m->d[0]) & BN_MASK2;

		for (i = 1; i < top; i++)
			tmp.d[i] = (~m->d[i]) & BN_MASK2;

		tmp.top = top;
	} else
		if (!BN_to_montgomery(&tmp, BN_value_one(), mont, ctx))
			goto err;

	/* prepare a^1 in Montgomery domain */
	if (a->neg || BN_ucmp(a, m) >= 0) {
		if (!BN_mod(&am, a, m, ctx))
			goto err;

		if (!BN_to_montgomery(&am, &am, mont, ctx))
			goto err;
	} else if (!BN_to_montgomery(&am, a, mont, ctx))
		goto err;

#if defined(SPARC_T4_MONT)

	if (t4) {
		typedef int (*bn_pwr5_mont_f) (BN_ULONG *tp, const BN_ULONG *np,
									   const BN_ULONG *n0, const void *table,
									   int power, int bits);
		int bn_pwr5_mont_t4_8(BN_ULONG *tp, const BN_ULONG *np,
							  const BN_ULONG *n0, const void *table,
							  int power, int bits);
		int bn_pwr5_mont_t4_16(BN_ULONG *tp, const BN_ULONG *np,
							   const BN_ULONG *n0, const void *table,
							   int power, int bits);
		int bn_pwr5_mont_t4_24(BN_ULONG *tp, const BN_ULONG *np,
							   const BN_ULONG *n0, const void *table,
							   int power, int bits);
		int bn_pwr5_mont_t4_32(BN_ULONG *tp, const BN_ULONG *np,
							   const BN_ULONG *n0, const void *table,
							   int power, int bits);
		static const bn_pwr5_mont_f pwr5_funcs[4] = {
			bn_pwr5_mont_t4_8, bn_pwr5_mont_t4_16,
			bn_pwr5_mont_t4_24, bn_pwr5_mont_t4_32
		};
		bn_pwr5_mont_f pwr5_worker = pwr5_funcs[top / 16 - 1];
		typedef int (*bn_mul_mont_f) (BN_ULONG *rp, const BN_ULONG *ap,
									  const void *bp, const BN_ULONG *np,
									  const BN_ULONG *n0);
		int bn_mul_mont_t4_8(BN_ULONG *rp, const BN_ULONG *ap, const void *bp,
							 const BN_ULONG *np, const BN_ULONG *n0);
		int bn_mul_mont_t4_16(BN_ULONG *rp, const BN_ULONG *ap,
							  const void *bp, const BN_ULONG *np,
							  const BN_ULONG *n0);
		int bn_mul_mont_t4_24(BN_ULONG *rp, const BN_ULONG *ap,
							  const void *bp, const BN_ULONG *np,
							  const BN_ULONG *n0);
		int bn_mul_mont_t4_32(BN_ULONG *rp, const BN_ULONG *ap,
							  const void *bp, const BN_ULONG *np,
							  const BN_ULONG *n0);
		static const bn_mul_mont_f mul_funcs[4] = {
			bn_mul_mont_t4_8, bn_mul_mont_t4_16,
			bn_mul_mont_t4_24, bn_mul_mont_t4_32
		};
		bn_mul_mont_f mul_worker = mul_funcs[top / 16 - 1];
		void bn_mul_mont_vis3(BN_ULONG *rp, const BN_ULONG *ap,
							  const void *bp, const BN_ULONG *np,
							  const BN_ULONG *n0, int num);
		void bn_mul_mont_t4(BN_ULONG *rp, const BN_ULONG *ap,
							const void *bp, const BN_ULONG *np,
							const BN_ULONG *n0, int num);
		void bn_mul_mont_gather5_t4(BN_ULONG *rp, const BN_ULONG *ap,
									const void *table, const BN_ULONG *np,
									const BN_ULONG *n0, int num, int power);
		void bn_flip_n_scatter5_t4(const BN_ULONG *inp, size_t num,
								   void *table, size_t power);
		void bn_gather5_t4(BN_ULONG *out, size_t num,
						   void *table, size_t power);
		void bn_flip_t4(BN_ULONG *dst, BN_ULONG *src, size_t num);
		BN_ULONG *np = mont->N.d, *n0 = mont->n0;
		int stride = 5 * (6 - (top / 16 - 1)); /* multiple of 5, but less than 32 */

		/*
		 * BN_to_montgomery can contaminate words above .top [in
		 * BN_DEBUG[_DEBUG] build]...
		 */
		for (i = am.top; i < top; i++)
			am.d[i] = 0;

		for (i = tmp.top; i < top; i++)
			tmp.d[i] = 0;

		bn_flip_n_scatter5_t4(tmp.d, top, powerbuf, 0);
		bn_flip_n_scatter5_t4(am.d, top, powerbuf, 1);

		if (!(*mul_worker) (tmp.d, am.d, am.d, np, n0) &&
			!(*mul_worker) (tmp.d, am.d, am.d, np, n0))
			bn_mul_mont_vis3(tmp.d, am.d, am.d, np, n0, top);

		bn_flip_n_scatter5_t4(tmp.d, top, powerbuf, 2);

		for (i = 3; i < 32; i++) {
			/* Calculate a^i = a^(i-1) * a */
			if (!(*mul_worker) (tmp.d, tmp.d, am.d, np, n0) &&
				!(*mul_worker) (tmp.d, tmp.d, am.d, np, n0))
				bn_mul_mont_vis3(tmp.d, tmp.d, am.d, np, n0, top);

			bn_flip_n_scatter5_t4(tmp.d, top, powerbuf, i);
		}

		/* switch to 64-bit domain */
		np = alloca(top * sizeof(BN_ULONG));
		top /= 2;
		bn_flip_t4(np, mont->N.d, top);
		bits--;

		for (wvalue = 0, i = bits % 5; i >= 0; i--, bits--)
			wvalue = (wvalue << 1) + BN_is_bit_set(p, bits);

		bn_gather5_t4(tmp.d, top, powerbuf, wvalue);

		/*
		 * Scan the exponent one window at a time starting from the most
		 * significant bits.
		 */
		while (bits >= 0) {
			if (bits < stride)
				stride = bits + 1;

			bits -= stride;
			wvalue = bn_get_bits(p, bits + 1);

			if ((*pwr5_worker) (tmp.d, np, n0, powerbuf, wvalue, stride))
				continue;

			/* retry once and fall back */
			if ((*pwr5_worker) (tmp.d, np, n0, powerbuf, wvalue, stride))
				continue;

			bits += stride - 5;
			wvalue >>= stride - 5;
			wvalue &= 31;
			bn_mul_mont_t4(tmp.d, tmp.d, tmp.d, np, n0, top);
			bn_mul_mont_t4(tmp.d, tmp.d, tmp.d, np, n0, top);
			bn_mul_mont_t4(tmp.d, tmp.d, tmp.d, np, n0, top);
			bn_mul_mont_t4(tmp.d, tmp.d, tmp.d, np, n0, top);
			bn_mul_mont_t4(tmp.d, tmp.d, tmp.d, np, n0, top);
			bn_mul_mont_gather5_t4(tmp.d, tmp.d, powerbuf, np, n0, top,
								   wvalue);
		}

		bn_flip_t4(tmp.d, tmp.d, top);
		top *= 2;
		/* back to 32-bit domain */
		tmp.top = top;
		bn_correct_top(&tmp);
		memset(np, 0, top * sizeof(BN_ULONG));
	} else
#endif
#if defined(OPENSSL_BN_ASM_MONT5)
		if (window == 5 && top > 1) {
			/*
			 * This optimization uses ideas from http://eprint.iacr.org/2011/239,
			 * specifically optimization of cache-timing attack countermeasures
			 * and pre-computation optimization.
			 */
			/*
			 * Dedicated window==4 case improves 512-bit RSA sign by ~15%, but as
			 * 512-bit RSA is hardly relevant, we omit it to spare size...
			 */
			void bn_mul_mont_gather5(BN_ULONG *rp, const BN_ULONG *ap,
									 const void *table, const BN_ULONG *np,
									 const BN_ULONG *n0, int num, int power);
			void bn_scatter5(const BN_ULONG *inp, size_t num,
							 void *table, size_t power);
			void bn_gather5(BN_ULONG *out, size_t num, void *table, size_t power);
			void bn_power5(BN_ULONG *rp, const BN_ULONG *ap,
						   const void *table, const BN_ULONG *np,
						   const BN_ULONG *n0, int num, int power);
			int bn_get_bits5(const BN_ULONG *ap, int off);
			int bn_from_montgomery(BN_ULONG *rp, const BN_ULONG *ap,
								   const BN_ULONG *not_used, const BN_ULONG *np,
								   const BN_ULONG *n0, int num);
			BN_ULONG *n0 = mont->n0, *np;

			/*
			 * BN_to_montgomery can contaminate words above .top [in
			 * BN_DEBUG[_DEBUG] build]...
			 */
			for (i = am.top; i < top; i++)
				am.d[i] = 0;

			for (i = tmp.top; i < top; i++)
				tmp.d[i] = 0;

			/*
			 * copy mont->N.d[] to improve cache locality
			 */
			for (np = am.d + top, i = 0; i < top; i++)
				np[i] = mont->N.d[i];

			bn_scatter5(tmp.d, top, powerbuf, 0);
			bn_scatter5(am.d, am.top, powerbuf, 1);
			bn_mul_mont(tmp.d, am.d, am.d, np, n0, top);
			bn_scatter5(tmp.d, top, powerbuf, 2);
# if 0

			for (i = 3; i < 32; i++) {
				/* Calculate a^i = a^(i-1) * a */
				bn_mul_mont_gather5(tmp.d, am.d, powerbuf, np, n0, top, i - 1);
				bn_scatter5(tmp.d, top, powerbuf, i);
			}

# else

			/* same as above, but uses squaring for 1/2 of operations */
			for (i = 4; i < 32; i *= 2) {
				bn_mul_mont(tmp.d, tmp.d, tmp.d, np, n0, top);
				bn_scatter5(tmp.d, top, powerbuf, i);
			}

			for (i = 3; i < 8; i += 2) {
				int j;
				bn_mul_mont_gather5(tmp.d, am.d, powerbuf, np, n0, top, i - 1);
				bn_scatter5(tmp.d, top, powerbuf, i);

				for (j = 2 * i; j < 32; j *= 2) {
					bn_mul_mont(tmp.d, tmp.d, tmp.d, np, n0, top);
					bn_scatter5(tmp.d, top, powerbuf, j);
				}
			}

			for (; i < 16; i += 2) {
				bn_mul_mont_gather5(tmp.d, am.d, powerbuf, np, n0, top, i - 1);
				bn_scatter5(tmp.d, top, powerbuf, i);
				bn_mul_mont(tmp.d, tmp.d, tmp.d, np, n0, top);
				bn_scatter5(tmp.d, top, powerbuf, 2 * i);
			}

			for (; i < 32; i += 2) {
				bn_mul_mont_gather5(tmp.d, am.d, powerbuf, np, n0, top, i - 1);
				bn_scatter5(tmp.d, top, powerbuf, i);
			}

# endif
			bits--;

			for (wvalue = 0, i = bits % 5; i >= 0; i--, bits--)
				wvalue = (wvalue << 1) + BN_is_bit_set(p, bits);

			bn_gather5(tmp.d, top, powerbuf, wvalue);

			/*
			 * Scan the exponent one window at a time starting from the most
			 * significant bits.
			 */
			if (top & 7)
				while (bits >= 0) {
					for (wvalue = 0, i = 0; i < 5; i++, bits--)
						wvalue = (wvalue << 1) + BN_is_bit_set(p, bits);

					bn_mul_mont(tmp.d, tmp.d, tmp.d, np, n0, top);
					bn_mul_mont(tmp.d, tmp.d, tmp.d, np, n0, top);
					bn_mul_mont(tmp.d, tmp.d, tmp.d, np, n0, top);
					bn_mul_mont(tmp.d, tmp.d, tmp.d, np, n0, top);
					bn_mul_mont(tmp.d, tmp.d, tmp.d, np, n0, top);
					bn_mul_mont_gather5(tmp.d, tmp.d, powerbuf, np, n0, top,
										wvalue);
				}
			else {
				while (bits >= 0) {
					wvalue = bn_get_bits5(p->d, bits - 4);
					bits -= 5;
					bn_power5(tmp.d, tmp.d, powerbuf, np, n0, top, wvalue);
				}
			}

			ret = bn_from_montgomery(tmp.d, tmp.d, NULL, np, n0, top);
			tmp.top = top;
			bn_correct_top(&tmp);

			if (ret) {
				if (!BN_copy(rr, &tmp))
					ret = 0;

				goto err;           /* non-zero ret means it's not error */
			}
		} else
#endif
		{
			if (!MOD_EXP_CTIME_COPY_TO_PREBUF(&tmp, top, powerbuf, 0, window))
				goto err;

			if (!MOD_EXP_CTIME_COPY_TO_PREBUF(&am, top, powerbuf, 1, window))
				goto err;

			/*
			 * If the window size is greater than 1, then calculate
			 * val[i=2..2^winsize-1]. Powers are computed as a*a^(i-1) (even
			 * powers could instead be computed as (a^(i/2))^2 to use the slight
			 * performance advantage of sqr over mul).
			 */
			if (window > 1) {
				if (!BN_mod_mul_montgomery(&tmp, &am, &am, mont, ctx))
					goto err;

				if (!MOD_EXP_CTIME_COPY_TO_PREBUF(&tmp, top, powerbuf, 2,
												  window))
					goto err;

				for (i = 3; i < numPowers; i++) {
					/* Calculate a^i = a^(i-1) * a */
					if (!BN_mod_mul_montgomery(&tmp, &am, &tmp, mont, ctx))
						goto err;

					if (!MOD_EXP_CTIME_COPY_TO_PREBUF(&tmp, top, powerbuf, i,
													  window))
						goto err;
				}
			}

			bits--;

			for (wvalue = 0, i = bits % window; i >= 0; i--, bits--)
				wvalue = (wvalue << 1) + BN_is_bit_set(p, bits);

			if (!MOD_EXP_CTIME_COPY_FROM_PREBUF(&tmp, top, powerbuf, wvalue,
												window))
				goto err;

			/*
			 * Scan the exponent one window at a time starting from the most
			 * significant bits.
			 */
			while (bits >= 0) {
				wvalue = 0;         /* The 'value' of the window */

				/* Scan the window, squaring the result as we go */
				for (i = 0; i < window; i++, bits--) {
					if (!BN_mod_mul_montgomery(&tmp, &tmp, &tmp, mont, ctx))
						goto err;

					wvalue = (wvalue << 1) + BN_is_bit_set(p, bits);
				}

				/*
				 * Fetch the appropriate pre-computed value from the pre-buf
				 */
				if (!MOD_EXP_CTIME_COPY_FROM_PREBUF(&am, top, powerbuf, wvalue,
													window))
					goto err;

				/* Multiply the result into the intermediate result */
				if (!BN_mod_mul_montgomery(&tmp, &tmp, &am, mont, ctx))
					goto err;
			}
		}

	/* Convert the final result from montgomery to standard format */
#if defined(SPARC_T4_MONT)

	if (OPENSSL_sparcv9cap_P[0] & (SPARCV9_VIS3 | SPARCV9_PREFER_FPU)) {
		am.d[0] = 1;            /* borrow am */

		for (i = 1; i < top; i++)
			am.d[i] = 0;

		if (!BN_mod_mul_montgomery(rr, &tmp, &am, mont, ctx))
			goto err;
	} else
#endif
		if (!BN_from_montgomery(rr, &tmp, mont, ctx))
			goto err;

	ret = 1;
err:

	if (in_mont == NULL)
		BN_MONT_CTX_free(mont);

	if (powerbuf != NULL) {
		memset(powerbuf, 0, powerbufLen);
		os_free_mem(powerbufFree);
	}

	BN_CTX_end(ctx);
	return ret;
}

int BN_mod_exp_mont_word(BIGNUM *rr, BN_ULONG a, const BIGNUM *p,
						 const BIGNUM *m, BN_CTX *ctx, BN_MONT_CTX *in_mont)
{
	BN_MONT_CTX *mont = NULL;
	int b, bits, ret = 0;
	int r_is_one;
	BN_ULONG w, next_w;
	BIGNUM *d, *r, *t;
	BIGNUM *swap_tmp;
#define BN_MOD_MUL_WORD(r, w, m) \
	(BN_mul_word(r, (w)) && \
	 (/* BN_ucmp(r, (m)) < 0 ? 1 :*/  \
									  (BN_mod(t, r, m, ctx) && (swap_tmp = r, r = t, t = swap_tmp, 1))))
	/*
	 * BN_MOD_MUL_WORD is only used with 'w' large, so the BN_ucmp test is
	 * probably more overhead than always using BN_mod (which uses BN_copy if
	 * a similar test returns true).
	 */
	/*
	 * We can use BN_mod and do not need BN_nnmod because our accumulator is
	 * never negative (the result of BN_mod does not depend on the sign of
	 * the modulus).
	 */
#define BN_TO_MONTGOMERY_WORD(r, w, mont) \
	(BN_set_word(r, (w)) && BN_to_montgomery(r, r, (mont), ctx))

	if (BN_get_flags(p, BN_FLG_CONSTTIME) != 0
		|| BN_get_flags(m, BN_FLG_CONSTTIME) != 0) {
		/* BN_FLG_CONSTTIME only supported by BN_mod_exp_mont() */
		BNerr(BN_F_BN_MOD_EXP_MONT_WORD, ERR_R_SHOULD_NOT_HAVE_BEEN_CALLED);
		return 0;
	}

	bn_check_top(p);
	bn_check_top(m);

	if (!BN_is_odd(m)) {
		BNerr(BN_F_BN_MOD_EXP_MONT_WORD, BN_R_CALLED_WITH_EVEN_MODULUS);
		return 0;
	}

	if (m->top == 1)
		a %= m->d[0];           /* make sure that 'a' is reduced */

	bits = BN_num_bits(p);

	if (bits == 0) {
		/* x**0 mod 1 is still zero. */
		if (BN_is_one(m)) {
			ret = 1;
			BN_zero(rr);
		} else
			ret = BN_one(rr);

		return ret;
	}

	if (a == 0) {
		BN_zero(rr);
		ret = 1;
		return ret;
	}

	BN_CTX_start(ctx);
	d = BN_CTX_get(ctx);
	r = BN_CTX_get(ctx);
	t = BN_CTX_get(ctx);

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

	r_is_one = 1;               /* except for Montgomery factor */
	/* bits-1 >= 0 */
	/* The result is accumulated in the product r*w. */
	w = a;                      /* bit 'bits-1' of 'p' is always set */

	for (b = bits - 2; b >= 0; b--) {
		/* First, square r*w. */
		next_w = w * w;

		if ((next_w / w) != w) { /* overflow */
			if (r_is_one) {
				if (!BN_TO_MONTGOMERY_WORD(r, w, mont))
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
					if (!BN_TO_MONTGOMERY_WORD(r, w, mont))
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
			if (!BN_TO_MONTGOMERY_WORD(r, w, mont))
				goto err;

			r_is_one = 0;
		} else {
			if (!BN_MOD_MUL_WORD(r, w, m))
				goto err;
		}
	}

	if (r_is_one) {             /* can happen only if a == 1 */
		if (!BN_one(rr))
			goto err;
	} else {
		if (!BN_from_montgomery(rr, r, mont, ctx))
			goto err;
	}

	ret = 1;
err:

	if (in_mont == NULL)
		BN_MONT_CTX_free(mont);

	BN_CTX_end(ctx);
	bn_check_top(rr);
	return ret;
}

/* The old fallback, simple version :-) */
int BN_mod_exp_simple(BIGNUM *r, const BIGNUM *a, const BIGNUM *p,
					  const BIGNUM *m, BN_CTX *ctx)
{
	int i, j, bits, ret = 0, wstart, wend, window, wvalue;
	int start = 1;
	BIGNUM *d;
	/* Table of variables obtained from 'ctx' */
	BIGNUM *val[TABLESIZE];

	if (BN_get_flags(p, BN_FLG_CONSTTIME) != 0
		|| BN_get_flags(a, BN_FLG_CONSTTIME) != 0
		|| BN_get_flags(m, BN_FLG_CONSTTIME) != 0) {
		/* BN_FLG_CONSTTIME only supported by BN_mod_exp_mont() */
		BNerr(BN_F_BN_MOD_EXP_SIMPLE, ERR_R_SHOULD_NOT_HAVE_BEEN_CALLED);
		return 0;
	}

	bits = BN_num_bits(p);

	if (bits == 0) {
		/* x**0 mod 1 is still zero. */
		if (BN_is_one(m)) {
			ret = 1;
			BN_zero(r);
		} else
			ret = BN_one(r);

		return ret;
	}

	BN_CTX_start(ctx);
	d = BN_CTX_get(ctx);
	val[0] = BN_CTX_get(ctx);

	if (!d || !val[0])
		goto err;

	if (!BN_nnmod(val[0], a, m, ctx))
		goto err;               /* 1 */

	if (BN_is_zero(val[0])) {
		BN_zero(r);
		ret = 1;
		goto err;
	}

	window = BN_window_bits_for_exponent_size(bits);

	if (window > 1) {
		if (!BN_mod_mul(d, val[0], val[0], m, ctx))
			goto err;           /* 2 */

		j = 1 << (window - 1);

		for (i = 1; i < j; i++) {
			val[i] = BN_CTX_get(ctx);
			if ((val[i] == NULL) ||
				!BN_mod_mul(val[i], val[i - 1], d, m, ctx))
				goto err;
		}
	}

	start = 1; /* This is used to avoid multiplication etc when there is only the value '1' in the buffer. */
	wvalue = 0;                 /* The 'value' of the window */
	wstart = bits - 1;          /* The top bit of the window */
	wend = 0;                   /* The bottom bit of the window */

	if (!BN_one(r))
		goto err;

	for (;;) {
		if (BN_is_bit_set(p, wstart) == 0) {
			if (!start)
				if (!BN_mod_mul(r, r, r, m, ctx))
					goto err;

			if (wstart == 0)
				break;

			wstart--;
			continue;
		}

		/*
		 * We now have wstart on a 'set' bit, we now need to work out how bit
		 * a window to do.  To do this we need to scan forward until the last
		 * set bit before the end of the window
		 */
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
				if (!BN_mod_mul(r, r, r, m, ctx))
					goto err;
			}

		/* wvalue will be an odd number < 2^window */
		if (!BN_mod_mul(r, r, val[wvalue >> 1], m, ctx))
			goto err;

		/* move the 'window' down further */
		wstart -= wend + 1;
		wvalue = 0;
		start = 0;

		if (wstart < 0)
			break;
	}

	ret = 1;
err:
	BN_CTX_end(ctx);
	bn_check_top(r);
	return ret;
}

void bn_mul_normal(BN_ULONG *r, BN_ULONG *a, int na, BN_ULONG *b, int nb)
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



int BN_mul(BIGNUM *r, const BIGNUM *a, const BIGNUM *b, BN_CTX *ctx)
{
	int ret = 0;
	int top, al, bl;
	BIGNUM *rr;
#if defined(BN_MUL_COMBA) || defined(BN_RECURSION)
	int i;
#endif
#ifdef BN_RECURSION
	BIGNUM *t = NULL;
	int j = 0, k;
#endif
	bn_check_top(a);
	bn_check_top(b);
	bn_check_top(r);
	al = a->top;
	bl = b->top;

	if ((al == 0) || (bl == 0)) {
		BN_zero(r);
		return 1;
	}

	top = al + bl;
	BN_CTX_start(ctx);

	if ((r == a) || (r == b)) {
		rr = BN_CTX_get(ctx);
		if (rr == NULL)
			goto err;
	} else
		rr = r;

#if defined(BN_MUL_COMBA) || defined(BN_RECURSION)
	i = al - bl;
#endif
#ifdef BN_MUL_COMBA

	if (i == 0) {
# if 0

		if (al == 4) {
			if (bn_wexpand(rr, 8) == NULL)
				goto err;

			rr->top = 8;
			bn_mul_comba4(rr->d, a->d, b->d);
			goto end;
		}

# endif

		if (al == 8) {
			if (bn_wexpand(rr, 16) == NULL)
				goto err;

			rr->top = 16;
			bn_mul_comba8(rr->d, a->d, b->d);
			goto end;
		}
	}

#endif                          /* BN_MUL_COMBA */
#ifdef BN_RECURSION

	if ((al >= BN_MULL_SIZE_NORMAL) && (bl >= BN_MULL_SIZE_NORMAL)) {
		if (i >= -1 && i <= 1) {
			/*
			 * Find out the power of two lower or equal to the longest of the
			 * two numbers
			 */
			if (i >= 0)
				j = BN_num_bits_word((BN_ULONG)al);

			if (i == -1)
				j = BN_num_bits_word((BN_ULONG)bl);

			j = 1 << (j - 1);
			assert(j <= al || j <= bl);
			k = j + j;
			t = BN_CTX_get(ctx);

			if (t == NULL)
				goto err;

			if (al > j || bl > j) {
				if (bn_wexpand(t, k * 4) == NULL)
					goto err;

				if (bn_wexpand(rr, k * 4) == NULL)
					goto err;

				bn_mul_part_recursive(rr->d, a->d, b->d,
									  j, al - j, bl - j, t->d);
			} else {            /* al <= j || bl <= j */
				if (bn_wexpand(t, k * 2) == NULL)
					goto err;

				if (bn_wexpand(rr, k * 2) == NULL)
					goto err;

				bn_mul_recursive(rr->d, a->d, b->d, j, al - j, bl - j, t->d);
			}

			rr->top = top;
			goto end;
		}
	}

#endif                          /* BN_RECURSION */

	if (bn_wexpand(rr, top) == NULL)
		goto err;

	rr->top = top;
	bn_mul_normal(rr->d, a->d, al, b->d, bl);
#if defined(BN_MUL_COMBA) || defined(BN_RECURSION)
end:
#endif
	rr->neg = a->neg ^ b->neg;
	bn_correct_top(rr);

	if (r != rr && BN_copy(r, rr) == NULL)
		goto err;

	ret = 1;
err:
	bn_check_top(r);
	BN_CTX_end(ctx);
	return ret;
}

/* least significant word */
#define BN_lsw(n) (((n)->top == 0) ? (BN_ULONG) 0 : (n)->d[0])

/* Returns -2 for errors because both -1 and 0 are valid results. */
int BN_kronecker(const BIGNUM *a, const BIGNUM *b, BN_CTX *ctx)
{
	int i;
	int ret = -2;               /* avoid 'uninitialized' warning */
	int err = 0;
	BIGNUM *A, *B, *tmp;
	/*-
	 * In 'tab', only odd-indexed entries are relevant:
	 * For any odd BIGNUM n,
	 *     tab[BN_lsw(n) & 7]
	 * is $(-1)^{(n^2-1)/8}$ (using TeX notation).
	 * Note that the sign of n does not matter.
	 */
	static const int tab[8] = { 0, 1, 0, -1, 0, -1, 0, 1 };
	bn_check_top(a);
	bn_check_top(b);
	BN_CTX_start(ctx);
	A = BN_CTX_get(ctx);
	B = BN_CTX_get(ctx);

	if (B == NULL)
		goto end;

	err = !BN_copy(A, a);

	if (err)
		goto end;

	err = !BN_copy(B, b);

	if (err)
		goto end;

	/*
	 * Kronecker symbol, implemented according to Henri Cohen,
	 * "A Course in Computational Algebraic Number Theory"
	 * (algorithm 1.4.10).
	 */

	/* Cohen's step 1: */

	if (BN_is_zero(B)) {
		ret = BN_abs_is_word(A, 1);
		goto end;
	}

	/* Cohen's step 2: */

	if (!BN_is_odd(A) && !BN_is_odd(B)) {
		ret = 0;
		goto end;
	}

	/* now  B  is non-zero */
	i = 0;

	while (!BN_is_bit_set(B, i))
		i++;

	err = !BN_rshift(B, B, i);

	if (err)
		goto end;

	if (i & 1) {
		/* i is odd */
		/* (thus  B  was even, thus  A  must be odd!)  */
		/* set 'ret' to $(-1)^{(A^2-1)/8}$ */
		ret = tab[BN_lsw(A) & 7];
	} else {
		/* i is even */
		ret = 1;
	}

	if (B->neg) {
		B->neg = 0;

		if (A->neg)
			ret = -ret;
	}

	/*
	 * now B is positive and odd, so what remains to be done is to compute
	 * the Jacobi symbol (A/B) and multiply it by 'ret'
	 */

	while (1) {
		/* Cohen's step 3: */

		/*  B  is positive and odd */
		if (BN_is_zero(A)) {
			ret = BN_is_one(B) ? ret : 0;
			goto end;
		}

		/* now  A  is non-zero */
		i = 0;

		while (!BN_is_bit_set(A, i))
			i++;

		err = !BN_rshift(A, A, i);

		if (err)
			goto end;

		if (i & 1) {
			/* i is odd */
			/* multiply 'ret' by  $(-1)^{(B^2-1)/8}$ */
			ret = ret * tab[BN_lsw(B) & 7];
		}

		/* Cohen's step 4: */
		/* multiply 'ret' by  $(-1)^{(A-1)(B-1)/4}$ */
		if ((A->neg ? ~BN_lsw(A) : BN_lsw(A)) & BN_lsw(B) & 2)
			ret = -ret;

		/* (A, B) := (B mod |A|, |A|) */
		err = !BN_nnmod(B, B, A, ctx);

		if (err)
			goto end;

		tmp = A;
		A = B;
		B = tmp;
		tmp->neg = 0;
	}

end:
	BN_CTX_end(ctx);

	if (err)
		return -2;
	else
		return ret;
}

static UCHAR randombyte(void)
{
	ULONG i;
	UCHAR R, Result;
	static ULONG ShiftReg;
	R = 0;

	if (ShiftReg == 0)
		NdisGetSystemUpTime((ULONG *)&ShiftReg);

	for (i = 0; i < 8; i++) {
		if (ShiftReg & 0x00000001) {
			ShiftReg = ((ShiftReg ^ LFSR_MASK) >> 1) | 0x80000000;
			Result = 1;
		} else {
			ShiftReg = ShiftReg >> 1;
			Result = 0;
		}

		R = (R << 1) | Result;
	}

	return R;
}

static int bnrand(int pseudorand, BIGNUM *rnd, int bits, int top, int bottom)
{
	unsigned char *buf = NULL;
	int ret = 0, bit, bytes, mask;
	time_t tim;
	UINT32 i;

	if (bits == 0) {
		if (top != BN_RAND_TOP_ANY || bottom != BN_RAND_BOTTOM_ANY)
			goto toosmall;

		BN_zero(rnd);
		return 1;
	}

	if (bits < 0 || (bits == 1 && top > 0))
		goto toosmall;

	bytes = (bits + 7) / 8;
	bit = (bits - 1) % 8;
	mask = 0xff << (bit + 1);
	os_alloc_mem(NULL, (UCHAR **) &buf, bytes);

	if (buf == NULL) {
		BNerr(BN_F_BNRAND, ERR_R_MALLOC_FAILURE);
		goto err;
	}

	/* make a random number and set the top and bottom bits */
	NdisGetSystemUpTime((ULONG *)(&tim));

	for (i = 0; i < bytes; i++)
		buf[i] = randombyte();

	if (pseudorand == 2) {
		/*
		 * generate patterns that are more likely to trigger BN library bugs
		 */
		int i;
		unsigned char c;

		for (i = 0; i < bytes; i++) {
			c = randombyte();

			if (c >= 128 && i > 0)
				buf[i] = buf[i - 1];
			else if (c < 42)
				buf[i] = 0;
			else if (c < 84)
				buf[i] = 255;
		}
	}

	if (top >= 0) {
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

	if (bottom)                 /* set bottom bit if requested */
		buf[bytes - 1] |= 1;

	if (!BN_bin2bn(buf, bytes, rnd))
		goto err;

	ret = 1;
err:
	os_free_mem(buf);
	bn_check_top(rnd);
	return ret;
toosmall:
	BNerr(BN_F_BNRAND, BN_R_BITS_TOO_SMALL);
	return 0;
}

/* random number r:  0 <= r < range */
static int bn_rand_range(int pseudo, BIGNUM *r, const BIGNUM *range)
{
	int (*bn_rand)(BIGNUM *, int, int, int) = pseudo ? BN_pseudo_rand : BN_rand;
	int n;
	int count = 100;

	if (range->neg || BN_is_zero(range))
		return 0;

	n = BN_num_bits(range);     /* n > 0 */

	/* BN_is_bit_set(range, n - 1) always holds */

	if (n == 1)
		BN_zero(r);
	else if (!BN_is_bit_set(range, n - 2) && !BN_is_bit_set(range, n - 3)) {
	/*
	 * range = 100..._2, so 3*range (= 11..._2) is exactly one bit longer
	 * than range
	 */
		do {
			if (!bn_rand(r, n + 1, -1, 0))
				return 0;
		/*
		 * If r < 3*range, use r := r MOD range (which is either r, r -
		 * range, or r - 2*range). Otherwise, iterate once more. Since
		 * 3*range = 11..._2, each iteration succeeds with probability >=
		 * .75.
		 */
			if (BN_cmp(r, range) >= 0) {
				if (!BN_sub(r, r, range))
					return 0;
				if (BN_cmp(r, range) >= 0)
					if (!BN_sub(r, r, range))
						return 0;
			}

			if (!--count)
				return 0;
		} while (BN_cmp(r, range) >= 0);
	} else {
		do {
		/* range = 11..._2  or  range = 101..._2 */
			if (!bn_rand(r, n, -1, 0))
				return 0;

			if (!--count)
				return 0;
		} while (BN_cmp(r, range) >= 0);
	}

	bn_check_top(r);
	return 1;
}

int BN_rand(BIGNUM *rnd, int bits, int top, int bottom)
{
	return bnrand(0, rnd, bits, top, bottom);
}

int BN_pseudo_rand(BIGNUM *rnd, int bits, int top, int bottom)
{
	return bnrand(1, rnd, bits, top, bottom);
}

int BN_bntest_rand(BIGNUM *rnd, int bits, int top, int bottom)
{
	return bnrand(2, rnd, bits, top, bottom);
}

int BN_rand_range(BIGNUM *r, const BIGNUM *range)
{
	return bn_rand_range(0, r, range);
}

BIGNUM *BN_mod_sqrt(BIGNUM *in, const BIGNUM *a, const BIGNUM *p, BN_CTX *ctx)
/*
 * Returns 'ret' such that ret^2 == a (mod p), using the Tonelli/Shanks
 * algorithm (cf. Henri Cohen, "A Course in Algebraic Computational Number
 * Theory", algorithm 1.5.1). 'p' must be prime!
 */
{
	BIGNUM *ret = in;
	int err = 1;
	int r;
	BIGNUM *A, *b, *q, *t, *x, *y;
	int e, i, j;

	if (!BN_is_odd(p) || BN_abs_is_word(p, 1)) {
		if (BN_abs_is_word(p, 2)) {
			if (ret == NULL)
				ret = BN_new();

			if (ret == NULL)
				goto end;

			if (!BN_set_word(ret, BN_is_bit_set(a, 0))) {
				if (ret != in)
					BN_free(ret);

				return NULL;
			}

			bn_check_top(ret);
			return ret;
		}

		BNerr(BN_F_BN_MOD_SQRT, BN_R_P_IS_NOT_PRIME);
		return NULL;
	}

	if (BN_is_zero(a) || BN_is_one(a)) {
		if (ret == NULL)
			ret = BN_new();

		if (ret == NULL)
			goto end;

		if (!BN_set_word(ret, BN_is_one(a))) {
			if (ret != in)
				BN_free(ret);

			return NULL;
		}

		bn_check_top(ret);
		return ret;
	}

	BN_CTX_start(ctx);
	A = BN_CTX_get(ctx);
	b = BN_CTX_get(ctx);
	q = BN_CTX_get(ctx);
	t = BN_CTX_get(ctx);
	x = BN_CTX_get(ctx);
	y = BN_CTX_get(ctx);

	if (y == NULL)
		goto end;

	if (ret == NULL)
		ret = BN_new();

	if (ret == NULL)
		goto end;

	/* A = a mod p */
	if (!BN_nnmod(A, a, p, ctx))
		goto end;

	/* now write  |p| - 1  as  2^e*q  where  q  is odd */
	e = 1;

	while (!BN_is_bit_set(p, e))
		e++;

	/* we'll set  q  later (if needed) */

	if (e == 1) {
		/*-
		 * The easy case:  (|p|-1)/2  is odd, so 2 has an inverse
		 * modulo  (|p|-1)/2,  and square roots can be computed
		 * directly by modular exponentiation.
		 * We have
		 *     2 * (|p|+1)/4 == 1   (mod (|p|-1)/2),
		 * so we can use exponent  (|p|+1)/4,  i.e.  (|p|-3)/4 + 1.
		 */
		if (!BN_rshift(q, p, 2))
			goto end;

		q->neg = 0;

		if (!BN_add_word(q, 1))
			goto end;

		if (!BN_mod_exp(ret, A, q, p, ctx))
			goto end;

		err = 0;
		goto vrfy;
	}

	if (e == 2) {
		/*-
		 * |p| == 5  (mod 8)
		 *
		 * In this case  2  is always a non-square since
		 * Legendre(2,p) = (-1)^((p^2-1)/8)  for any odd prime.
		 * So if  a  really is a square, then  2*a  is a non-square.
		 * Thus for
		 *      b := (2*a)^((|p|-5)/8),
		 *      i := (2*a)*b^2
		 * we have
		 *     i^2 = (2*a)^((1 + (|p|-5)/4)*2)
		 *         = (2*a)^((p-1)/2)
		 *         = -1;
		 * so if we set
		 *      x := a*b*(i-1),
		 * then
		 *     x^2 = a^2 * b^2 * (i^2 - 2*i + 1)
		 *         = a^2 * b^2 * (-2*i)
		 *         = a*(-i)*(2*a*b^2)
		 *         = a*(-i)*i
		 *         = a.
		 *
		 * (This is due to A.O.L. Atkin,
		 * <URL: http://listserv.nodak.edu/scripts/wa.exe?A2=ind9211&L=nmbrthry&O=T&P=562>,
		 * November 1992.)
		 */

		/* t := 2*a */
		if (!BN_mod_lshift1_quick(t, A, p))
			goto end;

		/* b := (2*a)^((|p|-5)/8) */
		if (!BN_rshift(q, p, 3))
			goto end;

		q->neg = 0;

		if (!BN_mod_exp(b, t, q, p, ctx))
			goto end;

		/* y := b^2 */
		if (!BN_mod_sqr(y, b, p, ctx))
			goto end;

		/* t := (2*a)*b^2 - 1 */
		if (!BN_mod_mul(t, t, y, p, ctx))
			goto end;

		if (!BN_sub_word(t, 1))
			goto end;

		/* x = a*b*t */
		if (!BN_mod_mul(x, A, b, p, ctx))
			goto end;

		if (!BN_mod_mul(x, x, t, p, ctx))
			goto end;

		if (!BN_copy(ret, x))
			goto end;

		err = 0;
		goto vrfy;
	}

	/*
	 * e > 2, so we really have to use the Tonelli/Shanks algorithm. First,
	 * find some y that is not a square.
	 */
	if (!BN_copy(q, p))
		goto end;               /* use 'q' as temp */

	q->neg = 0;
	i = 2;

	do {
		/*
		 * For efficiency, try small numbers first; if this fails, try random
		 * numbers.
		 */
		if (i < 22) {
			if (!BN_set_word(y, i))
				goto end;
		} else {
			if (!BN_pseudo_rand(y, BN_num_bits(p), 0, 0))
				goto end;

			if (BN_ucmp(y, p) >= 0) {
				if (!(p->neg ? BN_add : BN_sub) (y, y, p))
					goto end;
			}

			/* now 0 <= y < |p| */
			if (BN_is_zero(y))
				if (!BN_set_word(y, i))
					goto end;
		}

		r = BN_kronecker(y, q, ctx); /* here 'q' is |p| */

		if (r < -1)
			goto end;

		if (r == 0) {
			/* m divides p */
			BNerr(BN_F_BN_MOD_SQRT, BN_R_P_IS_NOT_PRIME);
			goto end;
		}
	} while (r == 1 && ++i < 82);

	if (r != -1) {
		/*
		 * Many rounds and still no non-square -- this is more likely a bug
		 * than just bad luck. Even if p is not prime, we should have found
		 * some y such that r == -1.
		 */
		BNerr(BN_F_BN_MOD_SQRT, BN_R_TOO_MANY_ITERATIONS);
		goto end;
	}

	/* Here's our actual 'q': */
	if (!BN_rshift(q, q, e))
		goto end;

	/*
	 * Now that we have some non-square, we can find an element of order 2^e
	 * by computing its q'th power.
	 */
	if (!BN_mod_exp(y, y, q, p, ctx))
		goto end;

	if (BN_is_one(y)) {
		BNerr(BN_F_BN_MOD_SQRT, BN_R_P_IS_NOT_PRIME);
		goto end;
	}

	/*-
	 * Now we know that (if  p  is indeed prime) there is an integer
	 * k,  0 <= k < 2^e,  such that
	 *
	 *      a^q * y^k == 1   (mod p).
	 *
	 * As  a^q  is a square and  y  is not,  k  must be even.
	 * q+1  is even, too, so there is an element
	 *
	 *     X := a^((q+1)/2) * y^(k/2),
	 *
	 * and it satisfies
	 *
	 *     X^2 = a^q * a     * y^k
	 *         = a,
	 *
	 * so it is the square root that we are looking for.
	 */

	/* t := (q-1)/2  (note that  q  is odd) */
	if (!BN_rshift1(t, q))
		goto end;

	/* x := a^((q-1)/2) */
	if (BN_is_zero(t)) {        /* special case: p = 2^e + 1 */
		if (!BN_nnmod(t, A, p, ctx))
			goto end;

		if (BN_is_zero(t)) {
			/* special case: a == 0  (mod p) */
			BN_zero(ret);
			err = 0;
			goto end;
		} else if (!BN_one(x))
			goto end;
	} else {
		if (!BN_mod_exp(x, A, t, p, ctx))
			goto end;

		if (BN_is_zero(x)) {
			/* special case: a == 0  (mod p) */
			BN_zero(ret);
			err = 0;
			goto end;
		}
	}

	/* b := a*x^2  (= a^q) */
	if (!BN_mod_sqr(b, x, p, ctx))
		goto end;

	if (!BN_mod_mul(b, b, A, p, ctx))
		goto end;

	/* x := a*x    (= a^((q+1)/2)) */
	if (!BN_mod_mul(x, x, A, p, ctx))
		goto end;

	while (1) {
		/*-
		 * Now  b  is  a^q * y^k  for some even  k  (0 <= k < 2^E
		 * where  E  refers to the original value of  e,  which we
		 * don't keep in a variable),  and  x  is  a^((q+1)/2) * y^(k/2).
		 *
		 * We have  a*b = x^2,
		 *    y^2^(e-1) = -1,
		 *    b^2^(e-1) = 1.
		 */
		if (BN_is_one(b)) {
			if (!BN_copy(ret, x))
				goto end;

			err = 0;
			goto vrfy;
		}

		/* find smallest  i  such that  b^(2^i) = 1 */
		i = 1;

		if (!BN_mod_sqr(t, b, p, ctx))
			goto end;

		while (!BN_is_one(t)) {
			i++;

			if (i == e) {
				BNerr(BN_F_BN_MOD_SQRT, BN_R_NOT_A_SQUARE);
				goto end;
			}

			if (!BN_mod_mul(t, t, t, p, ctx))
				goto end;
		}

		/* t := y^2^(e - i - 1) */
		if (!BN_copy(t, y))
			goto end;

		for (j = e - i - 1; j > 0; j--) {
			if (!BN_mod_sqr(t, t, p, ctx))
				goto end;
		}

		if (!BN_mod_mul(y, t, t, p, ctx))
			goto end;

		if (!BN_mod_mul(x, x, t, p, ctx))
			goto end;

		if (!BN_mod_mul(b, b, y, p, ctx))
			goto end;

		e = i;
	}

vrfy:

	if (!err) {
		/*
		 * verify the result -- the input might have been not a square (test
		 * added in 0.9.8)
		 */
		if (!BN_mod_sqr(x, ret, p, ctx))
			err = 1;

		if (!err && 0 != BN_cmp(x, A)) {
			BNerr(BN_F_BN_MOD_SQRT, BN_R_NOT_A_SQUARE);
			err = 1;
		}
	}

end:

	if (err) {
		if (ret != in)
			BN_clear_free(ret);

		ret = NULL;
	}

	BN_CTX_end(ctx);
	bn_check_top(ret);
	return ret;
}

static const char Hex[] = "0123456789ABCDEF";

char *BN_bn2hex(const BIGNUM *a)
{
	int i, j, v, z = 0;
	char *buf;
	char *p;

	if (BN_is_zero(a))
		return NULL;

	os_alloc_mem(NULL, (UCHAR **) &buf, a->top * BN_BYTES * 2 + 2);

	if (buf == NULL)
		goto err
;
	p = buf;
	if (a->neg)
		*(p++) = '-';
	for (i = a->top - 1; i >= 0; i--) {
		for (j = BN_BITS2 - 8; j >= 0; j -= 8) {
			/* strip leading zeros */
			v = ((int)(a->d[i] >> (long)j)) & 0xff;
			if (z || (v != 0)) {
				*(p++) = Hex[v >> 4];
				*(p++) = Hex[v & 0x0f];
				z = 1;
			}
		}
	}
	*p = '\0';
err:
	return buf;
}

#endif

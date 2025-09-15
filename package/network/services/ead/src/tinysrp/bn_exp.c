/* crypto/bn/bn_exp.c */
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
 *    must display the following acknowledgement:
 *    "This product includes cryptographic software written by
 *     Eric Young (eay@cryptsoft.com)"
 *    The word 'cryptographic' can be left out if the rouines from the library
 *    being used are not cryptographic related :-).
 * 4. If you include any Windows specific code (or a derivative thereof) from
 *    the apps directory (application code) you must include an acknowledgement:
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
/* ====================================================================
 * Copyright (c) 1998-2000 The OpenSSL Project.  All rights reserved.
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


#include <stdio.h>
#include "bn_lcl.h"

#define TABLE_SIZE      32

/* slow but works */
int BN_mod_mul(BIGNUM *ret, BIGNUM *a, BIGNUM *b, const BIGNUM *m, BN_CTX *ctx)
	{
	BIGNUM *t;
	int r=0;

	bn_check_top(a);
	bn_check_top(b);
	bn_check_top(m);

	BN_CTX_start(ctx);
	if ((t = BN_CTX_get(ctx)) == NULL) goto err;
	if (a == b)
		{ if (!BN_sqr(t,a,ctx)) goto err; }
	else
		{ if (!BN_mul(t,a,b,ctx)) goto err; }
	if (!BN_mod(ret,t,m,ctx)) goto err;
	r=1;
err:
	BN_CTX_end(ctx);
	return(r);
	}

int BN_mod_exp(BIGNUM *r, BIGNUM *a, const BIGNUM *p, const BIGNUM *m,
	       BN_CTX *ctx)
	{
	int ret;

	bn_check_top(a);
	bn_check_top(p);
	bn_check_top(m);

#ifdef MONT_MUL_MOD
	/* I have finally been able to take out this pre-condition of
	 * the top bit being set.  It was caused by an error in BN_div
	 * with negatives.  There was also another problem when for a^b%m
	 * a >= m.  eay 07-May-97 */
/*      if ((m->d[m->top-1]&BN_TBIT) && BN_is_odd(m)) */

	if (BN_is_odd(m))
		{
		if (a->top == 1)
			{
			BN_ULONG A = a->d[0];
			ret=BN_mod_exp_mont_word(r,A,p,m,ctx,NULL);
			}
		else
			ret=BN_mod_exp_mont(r,a,p,m,ctx,NULL);
		}
	else
#endif
#ifdef RECP_MUL_MOD
		{ ret=BN_mod_exp_recp(r,a,p,m,ctx); }
#else
		{ ret=BN_mod_exp_simple(r,a,p,m,ctx); }
#endif

	return(ret);
	}


#ifdef RECP_MUL_MOD
int BN_mod_exp_recp(BIGNUM *r, const BIGNUM *a, const BIGNUM *p,
		    const BIGNUM *m, BN_CTX *ctx)
	{
	int i,j,bits,ret=0,wstart,wend,window,wvalue;
	int start=1,ts=0;
	BIGNUM *aa;
	BIGNUM val[TABLE_SIZE];
	BN_RECP_CTX recp;

	bits=BN_num_bits(p);

	if (bits == 0)
		{
		BN_one(r);
		return(1);
		}

	BN_CTX_start(ctx);
	if ((aa = BN_CTX_get(ctx)) == NULL) goto err;

	BN_RECP_CTX_init(&recp);
	if (BN_RECP_CTX_set(&recp,m,ctx) <= 0) goto err;

	BN_init(&(val[0]));
	ts=1;

	if (!BN_mod(&(val[0]),a,m,ctx)) goto err;               /* 1 */

	window = BN_window_bits_for_exponent_size(bits);
	if (window > 1)
		{
		if (!BN_mod_mul_reciprocal(aa,&(val[0]),&(val[0]),&recp,ctx))
			goto err;                               /* 2 */
		j=1<<(window-1);
		for (i=1; i<j; i++)
			{
			BN_init(&val[i]);
			if (!BN_mod_mul_reciprocal(&(val[i]),&(val[i-1]),aa,&recp,ctx))
				goto err;
			}
		ts=i;
		}

	start=1;        /* This is used to avoid multiplication etc
			 * when there is only the value '1' in the
			 * buffer. */
	wvalue=0;       /* The 'value' of the window */
	wstart=bits-1;  /* The top bit of the window */
	wend=0;         /* The bottom bit of the window */

	if (!BN_one(r)) goto err;

	for (;;)
		{
		if (BN_is_bit_set(p,wstart) == 0)
			{
			if (!start)
				if (!BN_mod_mul_reciprocal(r,r,r,&recp,ctx))
				goto err;
			if (wstart == 0) break;
			wstart--;
			continue;
			}
		/* We now have wstart on a 'set' bit, we now need to work out
		 * how bit a window to do.  To do this we need to scan
		 * forward until the last set bit before the end of the
		 * window */
		j=wstart;
		wvalue=1;
		wend=0;
		for (i=1; i<window; i++)
			{
			if (wstart-i < 0) break;
			if (BN_is_bit_set(p,wstart-i))
				{
				wvalue<<=(i-wend);
				wvalue|=1;
				wend=i;
				}
			}

		/* wend is the size of the current window */
		j=wend+1;
		/* add the 'bytes above' */
		if (!start)
			for (i=0; i<j; i++)
				{
				if (!BN_mod_mul_reciprocal(r,r,r,&recp,ctx))
					goto err;
				}

		/* wvalue will be an odd number < 2^window */
		if (!BN_mod_mul_reciprocal(r,r,&(val[wvalue>>1]),&recp,ctx))
			goto err;

		/* move the 'window' down further */
		wstart-=wend+1;
		wvalue=0;
		start=0;
		if (wstart < 0) break;
		}
	ret=1;
err:
	BN_CTX_end(ctx);
	for (i=0; i<ts; i++)
		BN_clear_free(&(val[i]));
	BN_RECP_CTX_free(&recp);
	return(ret);
	}
#else

/* The old fallback, simple version :-) */
int BN_mod_exp_simple(BIGNUM *r, const BIGNUM *a, const BIGNUM *p,
	     const BIGNUM *m, BN_CTX *ctx)
	{
	int i,j,bits,ret=0,wstart,wend,window,wvalue,ts=0;
	int start=1;
	BIGNUM *d;
	BIGNUM val[TABLE_SIZE];

	bits=BN_num_bits(p);

	if (bits == 0)
		{
		BN_one(r);
		return(1);
		}

	BN_CTX_start(ctx);
	if ((d = BN_CTX_get(ctx)) == NULL) goto err;

	BN_init(&(val[0]));
	ts=1;
	if (!BN_mod(&(val[0]),a,m,ctx)) goto err;               /* 1 */

	window = BN_window_bits_for_exponent_size(bits);
	if (window > 1)
		{
		if (!BN_mod_mul(d,&(val[0]),&(val[0]),m,ctx))
			goto err;                               /* 2 */
		j=1<<(window-1);
		for (i=1; i<j; i++)
			{
			BN_init(&(val[i]));
			if (!BN_mod_mul(&(val[i]),&(val[i-1]),d,m,ctx))
				goto err;
			}
		ts=i;
		}

	start=1;        /* This is used to avoid multiplication etc
			 * when there is only the value '1' in the
			 * buffer. */
	wvalue=0;       /* The 'value' of the window */
	wstart=bits-1;  /* The top bit of the window */
	wend=0;         /* The bottom bit of the window */

	if (!BN_one(r)) goto err;

	for (;;)
		{
		if (BN_is_bit_set(p,wstart) == 0)
			{
			if (!start)
				if (!BN_mod_mul(r,r,r,m,ctx))
				goto err;
			if (wstart == 0) break;
			wstart--;
			continue;
			}
		/* We now have wstart on a 'set' bit, we now need to work out
		 * how bit a window to do.  To do this we need to scan
		 * forward until the last set bit before the end of the
		 * window */
		j=wstart;
		wvalue=1;
		wend=0;
		for (i=1; i<window; i++)
			{
			if (wstart-i < 0) break;
			if (BN_is_bit_set(p,wstart-i))
				{
				wvalue<<=(i-wend);
				wvalue|=1;
				wend=i;
				}
			}

		/* wend is the size of the current window */
		j=wend+1;
		/* add the 'bytes above' */
		if (!start)
			for (i=0; i<j; i++)
				{
				if (!BN_mod_mul(r,r,r,m,ctx))
					goto err;
				}

		/* wvalue will be an odd number < 2^window */
		if (!BN_mod_mul(r,r,&(val[wvalue>>1]),m,ctx))
			goto err;

		/* move the 'window' down further */
		wstart-=wend+1;
		wvalue=0;
		start=0;
		if (wstart < 0) break;
		}
	ret=1;
err:
	BN_CTX_end(ctx);
	for (i=0; i<ts; i++)
		BN_clear_free(&(val[i]));
	return(ret);
	}
#endif

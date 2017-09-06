/* crypto/bn/bn_sqr.c */
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

#include <stdio.h>
#include <string.h>
#include "bn_lcl.h"

/* r must not be a */
/* I've just gone over this and it is now %20 faster on x86 - eay - 27 Jun 96 */
int BN_sqr(BIGNUM *r, BIGNUM *a, BN_CTX *ctx)
	{
	int max,al;
	int ret = 0;
	BIGNUM *tmp,*rr;

#ifdef BN_COUNT
printf("BN_sqr %d * %d\n",a->top,a->top);
#endif
	bn_check_top(a);

	al=a->top;
	if (al <= 0)
		{
		r->top=0;
		return(1);
		}

	BN_CTX_start(ctx);
	rr=(a != r) ? r : BN_CTX_get(ctx);
	tmp=BN_CTX_get(ctx);
	if (tmp == NULL) goto err;

	max=(al+al);
	if (bn_wexpand(rr,max+1) == NULL) goto err;

	r->neg=0;
	if (al == 4)
		{
#ifndef BN_SQR_COMBA
		BN_ULONG t[8];
		bn_sqr_normal(rr->d,a->d,4,t);
#else
		bn_sqr_comba4(rr->d,a->d);
#endif
		}
	else if (al == 8)
		{
#ifndef BN_SQR_COMBA
		BN_ULONG t[16];
		bn_sqr_normal(rr->d,a->d,8,t);
#else
		bn_sqr_comba8(rr->d,a->d);
#endif
		}
	else
		{
		if (bn_wexpand(tmp,max) == NULL) goto err;
		bn_sqr_normal(rr->d,a->d,al,tmp->d);
		}

	rr->top=max;
	if ((max > 0) && (rr->d[max-1] == 0)) rr->top--;
	if (rr != r) BN_copy(r,rr);
	ret = 1;
 err:
	BN_CTX_end(ctx);
	return(ret);
	}

/* tmp must have 2*n words */
void bn_sqr_normal(BN_ULONG *r, BN_ULONG *a, int n, BN_ULONG *tmp)
	{
	int i,j,max;
	BN_ULONG *ap,*rp;

	max=n*2;
	ap=a;
	rp=r;
	rp[0]=rp[max-1]=0;
	rp++;
	j=n;

	if (--j > 0)
		{
		ap++;
		rp[j]=bn_mul_words(rp,ap,j,ap[-1]);
		rp+=2;
		}

	for (i=n-2; i>0; i--)
		{
		j--;
		ap++;
		rp[j]=bn_mul_add_words(rp,ap,j,ap[-1]);
		rp+=2;
		}

	bn_add_words(r,r,r,max);

	/* There will not be a carry */

	bn_sqr_words(tmp,a,n);

	bn_add_words(r,r,tmp,max);
	}

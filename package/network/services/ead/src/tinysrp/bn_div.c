/* crypto/bn/bn_div.c */
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
#include "bn_lcl.h"

#define NO_ASM

/* The old slow way */
#if 0
int BN_div(BIGNUM *dv, BIGNUM *rem, const BIGNUM *m, const BIGNUM *d,
	   BN_CTX *ctx)
	{
	int i,nm,nd;
	int ret = 0;
	BIGNUM *D;

	bn_check_top(m);
	bn_check_top(d);
	if (BN_is_zero(d))
		{
		return(0);
		}

	if (BN_ucmp(m,d) < 0)
		{
		if (rem != NULL)
			{ if (BN_copy(rem,m) == NULL) return(0); }
		if (dv != NULL) BN_zero(dv);
		return(1);
		}

	BN_CTX_start(ctx);
	D = BN_CTX_get(ctx);
	if (dv == NULL) dv = BN_CTX_get(ctx);
	if (rem == NULL) rem = BN_CTX_get(ctx);
	if (D == NULL || dv == NULL || rem == NULL)
		goto end;

	nd=BN_num_bits(d);
	nm=BN_num_bits(m);
	if (BN_copy(D,d) == NULL) goto end;
	if (BN_copy(rem,m) == NULL) goto end;

	/* The next 2 are needed so we can do a dv->d[0]|=1 later
	 * since BN_lshift1 will only work once there is a value :-) */
	BN_zero(dv);
	bn_wexpand(dv,1);
	dv->top=1;

	if (!BN_lshift(D,D,nm-nd)) goto end;
	for (i=nm-nd; i>=0; i--)
		{
		if (!BN_lshift1(dv,dv)) goto end;
		if (BN_ucmp(rem,D) >= 0)
			{
			dv->d[0]|=1;
			if (!BN_usub(rem,rem,D)) goto end;
			}
/* CAN IMPROVE (and have now :=) */
		if (!BN_rshift1(D,D)) goto end;
		}
	rem->neg=BN_is_zero(rem)?0:m->neg;
	dv->neg=m->neg^d->neg;
	ret = 1;
 end:
	BN_CTX_end(ctx);
	return(ret);
	}

#else

#if !defined(NO_ASM) && !defined(NO_INLINE_ASM) && !defined(PEDANTIC) && !defined(BN_DIV3W)
# if defined(__GNUC__) && __GNUC__>=2
#  if defined(__i386)
   /*
    * There were two reasons for implementing this template:
    * - GNU C generates a call to a function (__udivdi3 to be exact)
    *   in reply to ((((BN_ULLONG)n0)<<BN_BITS2)|n1)/d0 (I fail to
    *   understand why...);
    * - divl doesn't only calculate quotient, but also leaves
    *   remainder in %edx which we can definitely use here:-)
    *
    *                                   <appro@fy.chalmers.se>
    */
#  define bn_div_words(n0,n1,d0)                \
	({  asm volatile (                      \
		"divl   %4"                     \
		: "=a"(q), "=d"(rem)            \
		: "a"(n1), "d"(n0), "g"(d0)     \
		: "cc");                        \
	    q;                                  \
	})
#  define REMAINDER_IS_ALREADY_CALCULATED
#  endif /* __<cpu> */
# endif /* __GNUC__ */
#endif /* NO_ASM */

int BN_div(BIGNUM *dv, BIGNUM *rm, const BIGNUM *num, const BIGNUM *divisor,
	   BN_CTX *ctx)
	{
	int norm_shift,i,j,loop;
	BIGNUM *tmp,wnum,*snum,*sdiv,*res;
	BN_ULONG *resp,*wnump;
	BN_ULONG d0,d1;
	int num_n,div_n;

	bn_check_top(num);
	bn_check_top(divisor);

	if (BN_is_zero(divisor))
		{
		return(0);
		}

	if (BN_ucmp(num,divisor) < 0)
		{
		if (rm != NULL)
			{ if (BN_copy(rm,num) == NULL) return(0); }
		if (dv != NULL) BN_zero(dv);
		return(1);
		}

	BN_CTX_start(ctx);
	tmp=BN_CTX_get(ctx);
	tmp->neg=0;
	snum=BN_CTX_get(ctx);
	sdiv=BN_CTX_get(ctx);
	if (dv == NULL)
		res=BN_CTX_get(ctx);
	else    res=dv;
	if (res == NULL) goto err;

	/* First we normalise the numbers */
	norm_shift=BN_BITS2-((BN_num_bits(divisor))%BN_BITS2);
	BN_lshift(sdiv,divisor,norm_shift);
	sdiv->neg=0;
	norm_shift+=BN_BITS2;
	BN_lshift(snum,num,norm_shift);
	snum->neg=0;
	div_n=sdiv->top;
	num_n=snum->top;
	loop=num_n-div_n;

	/* Lets setup a 'window' into snum
	 * This is the part that corresponds to the current
	 * 'area' being divided */
	BN_init(&wnum);
	wnum.d=  &(snum->d[loop]);
	wnum.top= div_n;
	wnum.dmax= snum->dmax+1; /* a bit of a lie */

	/* Get the top 2 words of sdiv */
	/* i=sdiv->top; */
	d0=sdiv->d[div_n-1];
	d1=(div_n == 1)?0:sdiv->d[div_n-2];

	/* pointer to the 'top' of snum */
	wnump= &(snum->d[num_n-1]);

	/* Setup to 'res' */
	res->neg= (num->neg^divisor->neg);
	if (!bn_wexpand(res,(loop+1))) goto err;
	res->top=loop;
	resp= &(res->d[loop-1]);

	/* space for temp */
	if (!bn_wexpand(tmp,(div_n+1))) goto err;

	if (BN_ucmp(&wnum,sdiv) >= 0)
		{
		if (!BN_usub(&wnum,&wnum,sdiv)) goto err;
		*resp=1;
		res->d[res->top-1]=1;
		}
	else
		res->top--;
	resp--;

	for (i=0; i<loop-1; i++)
		{
		BN_ULONG q,l0;
#ifdef BN_DIV3W
		q=bn_div_3_words(wnump,d1,d0);
#else
		BN_ULONG n0,n1,rem=0;

		n0=wnump[0];
		n1=wnump[-1];
		if (n0 == d0)
			q=BN_MASK2;
		else                    /* n0 < d0 */
			{
#ifdef BN_LLONG
			BN_ULLONG t2;

#if defined(BN_LLONG) && defined(BN_DIV2W) && !defined(bn_div_words)
			q=(BN_ULONG)(((((BN_ULLONG)n0)<<BN_BITS2)|n1)/d0);
#else
			q=bn_div_words(n0,n1,d0);
#endif

#ifndef REMAINDER_IS_ALREADY_CALCULATED
			/*
			 * rem doesn't have to be BN_ULLONG. The least we
			 * know it's less that d0, isn't it?
			 */
			rem=(n1-q*d0)&BN_MASK2;
#endif
			t2=(BN_ULLONG)d1*q;

			for (;;)
				{
				if (t2 <= ((((BN_ULLONG)rem)<<BN_BITS2)|wnump[-2]))
					break;
				q--;
				rem += d0;
				if (rem < d0) break; /* don't let rem overflow */
				t2 -= d1;
				}
#else /* !BN_LLONG */
			BN_ULONG t2l,t2h,ql,qh;

			q=bn_div_words(n0,n1,d0);
#ifndef REMAINDER_IS_ALREADY_CALCULATED
			rem=(n1-q*d0)&BN_MASK2;
#endif

#ifdef BN_UMULT_HIGH
			t2l = d1 * q;
			t2h = BN_UMULT_HIGH(d1,q);
#else
			t2l=LBITS(d1); t2h=HBITS(d1);
			ql =LBITS(q);  qh =HBITS(q);
			mul64(t2l,t2h,ql,qh); /* t2=(BN_ULLONG)d1*q; */
#endif

			for (;;)
				{
				if ((t2h < rem) ||
					((t2h == rem) && (t2l <= wnump[-2])))
					break;
				q--;
				rem += d0;
				if (rem < d0) break; /* don't let rem overflow */
				if (t2l < d1) t2h--; t2l -= d1;
				}
#endif /* !BN_LLONG */
			}
#endif /* !BN_DIV3W */

		l0=bn_mul_words(tmp->d,sdiv->d,div_n,q);
		wnum.d--; wnum.top++;
		tmp->d[div_n]=l0;
		for (j=div_n+1; j>0; j--)
			if (tmp->d[j-1]) break;
		tmp->top=j;

		j=wnum.top;
		BN_sub(&wnum,&wnum,tmp);

		snum->top=snum->top+wnum.top-j;

		if (wnum.neg)
			{
			q--;
			j=wnum.top;
			BN_add(&wnum,&wnum,sdiv);
			snum->top+=wnum.top-j;
			}
		*(resp--)=q;
		wnump--;
		}
	if (rm != NULL)
		{
		BN_rshift(rm,snum,norm_shift);
		rm->neg=num->neg;
		}
	BN_CTX_end(ctx);
	return(1);
err:
	BN_CTX_end(ctx);
	return(0);
	}

#endif

/* rem != m */
int BN_mod(BIGNUM *rem, const BIGNUM *m, const BIGNUM *d, BN_CTX *ctx)
	{
#if 0 /* The old slow way */
	int i,nm,nd;
	BIGNUM *dv;

	if (BN_ucmp(m,d) < 0)
		return((BN_copy(rem,m) == NULL)?0:1);

	BN_CTX_start(ctx);
	dv=BN_CTX_get(ctx);

	if (!BN_copy(rem,m)) goto err;

	nm=BN_num_bits(rem);
	nd=BN_num_bits(d);
	if (!BN_lshift(dv,d,nm-nd)) goto err;
	for (i=nm-nd; i>=0; i--)
		{
		if (BN_cmp(rem,dv) >= 0)
			{
			if (!BN_sub(rem,rem,dv)) goto err;
			}
		if (!BN_rshift1(dv,dv)) goto err;
		}
	BN_CTX_end(ctx);
	return(1);
 err:
	BN_CTX_end(ctx);
	return(0);
#else
	return(BN_div(NULL,rem,m,d,ctx));
#endif
	}


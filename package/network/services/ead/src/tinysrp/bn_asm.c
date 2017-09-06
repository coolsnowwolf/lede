/* crypto/bn/bn_asm.c */
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

#ifndef BN_DEBUG
# undef NDEBUG /* avoid conflicting definitions */
# define NDEBUG
#endif

#include <stdio.h>
#include <assert.h>
#include "bn_lcl.h"

#if defined(BN_LLONG) || defined(BN_UMULT_HIGH)

BN_ULONG bn_mul_add_words(BN_ULONG *rp, BN_ULONG *ap, int num, BN_ULONG w)
	{
	BN_ULONG c1=0;

	assert(num >= 0);
	if (num <= 0) return(c1);

	while (num&~3)
		{
		mul_add(rp[0],ap[0],w,c1);
		mul_add(rp[1],ap[1],w,c1);
		mul_add(rp[2],ap[2],w,c1);
		mul_add(rp[3],ap[3],w,c1);
		ap+=4; rp+=4; num-=4;
		}
	if (num)
		{
		mul_add(rp[0],ap[0],w,c1); if (--num==0) return c1;
		mul_add(rp[1],ap[1],w,c1); if (--num==0) return c1;
		mul_add(rp[2],ap[2],w,c1); return c1;
		}

	return(c1);
	}

BN_ULONG bn_mul_words(BN_ULONG *rp, BN_ULONG *ap, int num, BN_ULONG w)
	{
	BN_ULONG c1=0;

	assert(num >= 0);
	if (num <= 0) return(c1);

	while (num&~3)
		{
		mul(rp[0],ap[0],w,c1);
		mul(rp[1],ap[1],w,c1);
		mul(rp[2],ap[2],w,c1);
		mul(rp[3],ap[3],w,c1);
		ap+=4; rp+=4; num-=4;
		}
	if (num)
		{
		mul(rp[0],ap[0],w,c1); if (--num == 0) return c1;
		mul(rp[1],ap[1],w,c1); if (--num == 0) return c1;
		mul(rp[2],ap[2],w,c1);
		}
	return(c1);
	}

void bn_sqr_words(BN_ULONG *r, BN_ULONG *a, int n)
	{
	assert(n >= 0);
	if (n <= 0) return;
	while (n&~3)
		{
		sqr(r[0],r[1],a[0]);
		sqr(r[2],r[3],a[1]);
		sqr(r[4],r[5],a[2]);
		sqr(r[6],r[7],a[3]);
		a+=4; r+=8; n-=4;
		}
	if (n)
		{
		sqr(r[0],r[1],a[0]); if (--n == 0) return;
		sqr(r[2],r[3],a[1]); if (--n == 0) return;
		sqr(r[4],r[5],a[2]);
		}
	}

#else /* !(defined(BN_LLONG) || defined(BN_UMULT_HIGH)) */

BN_ULONG bn_mul_add_words(BN_ULONG *rp, BN_ULONG *ap, int num, BN_ULONG w)
	{
	BN_ULONG c=0;
	BN_ULONG bl,bh;

	assert(num >= 0);
	if (num <= 0) return((BN_ULONG)0);

	bl=LBITS(w);
	bh=HBITS(w);

	for (;;)
		{
		mul_add(rp[0],ap[0],bl,bh,c);
		if (--num == 0) break;
		mul_add(rp[1],ap[1],bl,bh,c);
		if (--num == 0) break;
		mul_add(rp[2],ap[2],bl,bh,c);
		if (--num == 0) break;
		mul_add(rp[3],ap[3],bl,bh,c);
		if (--num == 0) break;
		ap+=4;
		rp+=4;
		}
	return(c);
	}

BN_ULONG bn_mul_words(BN_ULONG *rp, BN_ULONG *ap, int num, BN_ULONG w)
	{
	BN_ULONG carry=0;
	BN_ULONG bl,bh;

	assert(num >= 0);
	if (num <= 0) return((BN_ULONG)0);

	bl=LBITS(w);
	bh=HBITS(w);

	for (;;)
		{
		mul(rp[0],ap[0],bl,bh,carry);
		if (--num == 0) break;
		mul(rp[1],ap[1],bl,bh,carry);
		if (--num == 0) break;
		mul(rp[2],ap[2],bl,bh,carry);
		if (--num == 0) break;
		mul(rp[3],ap[3],bl,bh,carry);
		if (--num == 0) break;
		ap+=4;
		rp+=4;
		}
	return(carry);
	}

void bn_sqr_words(BN_ULONG *r, BN_ULONG *a, int n)
	{
	assert(n >= 0);
	if (n <= 0) return;
	for (;;)
		{
		sqr64(r[0],r[1],a[0]);
		if (--n == 0) break;

		sqr64(r[2],r[3],a[1]);
		if (--n == 0) break;

		sqr64(r[4],r[5],a[2]);
		if (--n == 0) break;

		sqr64(r[6],r[7],a[3]);
		if (--n == 0) break;

		a+=4;
		r+=8;
		}
	}

#endif /* !(defined(BN_LLONG) || defined(BN_UMULT_HIGH)) */

#if defined(BN_LLONG) && defined(BN_DIV2W)

BN_ULONG bn_div_words(BN_ULONG h, BN_ULONG l, BN_ULONG d)
	{
	return((BN_ULONG)(((((BN_ULLONG)h)<<BN_BITS2)|l)/(BN_ULLONG)d));
	}

#else

/* Divide h,l by d and return the result. */
/* I need to test this some more :-( */
BN_ULONG bn_div_words(BN_ULONG h, BN_ULONG l, BN_ULONG d)
	{
	BN_ULONG dh,dl,q,ret=0,th,tl,t;
	int i,count=2;

	if (d == 0) return(BN_MASK2);

	i=BN_num_bits_word(d);
	assert((i == BN_BITS2) || (h > (BN_ULONG)1<<i));

	i=BN_BITS2-i;
	if (h >= d) h-=d;

	if (i)
		{
		d<<=i;
		h=(h<<i)|(l>>(BN_BITS2-i));
		l<<=i;
		}
	dh=(d&BN_MASK2h)>>BN_BITS4;
	dl=(d&BN_MASK2l);
	for (;;)
		{
		if ((h>>BN_BITS4) == dh)
			q=BN_MASK2l;
		else
			q=h/dh;

		th=q*dh;
		tl=dl*q;
		for (;;)
			{
			t=h-th;
			if ((t&BN_MASK2h) ||
				((tl) <= (
					(t<<BN_BITS4)|
					((l&BN_MASK2h)>>BN_BITS4))))
				break;
			q--;
			th-=dh;
			tl-=dl;
			}
		t=(tl>>BN_BITS4);
		tl=(tl<<BN_BITS4)&BN_MASK2h;
		th+=t;

		if (l < tl) th++;
		l-=tl;
		if (h < th)
			{
			h+=d;
			q--;
			}
		h-=th;

		if (--count == 0) break;

		ret=q<<BN_BITS4;
		h=((h<<BN_BITS4)|(l>>BN_BITS4))&BN_MASK2;
		l=(l&BN_MASK2l)<<BN_BITS4;
		}
	ret|=q;
	return(ret);
	}
#endif /* !defined(BN_LLONG) && defined(BN_DIV2W) */

#ifdef BN_LLONG
BN_ULONG bn_add_words(BN_ULONG *r, BN_ULONG *a, BN_ULONG *b, int n)
	{
	BN_ULLONG ll=0;

	assert(n >= 0);
	if (n <= 0) return((BN_ULONG)0);

	for (;;)
		{
		ll+=(BN_ULLONG)a[0]+b[0];
		r[0]=(BN_ULONG)ll&BN_MASK2;
		ll>>=BN_BITS2;
		if (--n <= 0) break;

		ll+=(BN_ULLONG)a[1]+b[1];
		r[1]=(BN_ULONG)ll&BN_MASK2;
		ll>>=BN_BITS2;
		if (--n <= 0) break;

		ll+=(BN_ULLONG)a[2]+b[2];
		r[2]=(BN_ULONG)ll&BN_MASK2;
		ll>>=BN_BITS2;
		if (--n <= 0) break;

		ll+=(BN_ULLONG)a[3]+b[3];
		r[3]=(BN_ULONG)ll&BN_MASK2;
		ll>>=BN_BITS2;
		if (--n <= 0) break;

		a+=4;
		b+=4;
		r+=4;
		}
	return((BN_ULONG)ll);
	}
#else /* !BN_LLONG */
BN_ULONG bn_add_words(BN_ULONG *r, BN_ULONG *a, BN_ULONG *b, int n)
	{
	BN_ULONG c,l,t;

	assert(n >= 0);
	if (n <= 0) return((BN_ULONG)0);

	c=0;
	for (;;)
		{
		t=a[0];
		t=(t+c)&BN_MASK2;
		c=(t < c);
		l=(t+b[0])&BN_MASK2;
		c+=(l < t);
		r[0]=l;
		if (--n <= 0) break;

		t=a[1];
		t=(t+c)&BN_MASK2;
		c=(t < c);
		l=(t+b[1])&BN_MASK2;
		c+=(l < t);
		r[1]=l;
		if (--n <= 0) break;

		t=a[2];
		t=(t+c)&BN_MASK2;
		c=(t < c);
		l=(t+b[2])&BN_MASK2;
		c+=(l < t);
		r[2]=l;
		if (--n <= 0) break;

		t=a[3];
		t=(t+c)&BN_MASK2;
		c=(t < c);
		l=(t+b[3])&BN_MASK2;
		c+=(l < t);
		r[3]=l;
		if (--n <= 0) break;

		a+=4;
		b+=4;
		r+=4;
		}
	return((BN_ULONG)c);
	}
#endif /* !BN_LLONG */

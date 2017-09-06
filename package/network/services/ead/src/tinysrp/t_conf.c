/*
 * Copyright (c) 1997-1999  The Stanford SRP Authentication Project
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.
 *
 * IN NO EVENT SHALL STANFORD BE LIABLE FOR ANY SPECIAL, INCIDENTAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND, OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER OR NOT ADVISED OF
 * THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF LIABILITY, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * In addition, the following conditions apply:
 *
 * 1. Any software that incorporates the SRP authentication technology
 *    must display the following acknowlegment:
 *    "This product uses the 'Secure Remote Password' cryptographic
 *     authentication system developed by Tom Wu (tjw@CS.Stanford.EDU)."
 *
 * 2. Any software that incorporates all or part of the SRP distribution
 *    itself must also display the following acknowledgment:
 *    "This product includes software developed by Tom Wu and Eugene
 *     Jhong for the SRP Distribution (http://srp.stanford.edu/srp/)."
 *
 * 3. Redistributions in source or binary form must retain an intact copy
 *    of this copyright notice and list of conditions.
 */

#include <stdio.h>

#include "t_defines.h"
#include "t_pwd.h"
#include "t_read.h"
#include "bn.h"
#include "bn_lcl.h"
#include "bn_prime.h"

#define TABLE_SIZE      32

static int witness(BIGNUM *w, const BIGNUM *a, const BIGNUM *a1,
	const BIGNUM *a1_odd, int k, BN_CTX *ctx, BN_MONT_CTX *mont);

/*
 * This is the safe prime generation logic.
 * To generate a safe prime p (where p = 2q+1 and q is prime), we start
 * with a random odd q that is one bit shorter than the desired length
 * of p.  We use a simple 30-element sieve to filter the values of q
 * and consider only those that are 11, 23, or 29 (mod 30).  (If q were
 * anything else, either q or p would be divisible by 2, 3, or 5).
 * For the values of q that are left, we apply the following tests in
 * this order:
 *
 *   trial divide q
 *   let p = 2q + 1
 *   trial divide p
 *   apply Fermat test to q (2^q == 2 (mod q))
 *   apply Fermat test to p (2^p == 2 (mod p))
 *   apply real probablistic primality test to q
 *   apply real probablistic primality test to p
 *
 * A number that passes all these tests is considered a safe prime for
 * our purposes.  The tests are ordered this way for efficiency; the
 * slower tests are run rarely if ever at all.
 */

static int
trialdiv(x)
     const BigInteger x;
{
  static int primes[] = {               /* All odd primes < 256 */
      3,   5,   7,  11,  13,  17,  19,  23,  29,
     31,  37,  41,  43,  47,  53,  59,  61,  67,
     71,  73,  79,  83,  89,  97, 101, 103,
    107, 109, 113, 127, 131, 137, 139, 149, 151,
    157, 163, 167, 173, 179, 181, 191, 193, 197,
    199, 211, 223, 227, 229, 233, 239, 241, 251
  };
  static int nprimes = sizeof(primes) / sizeof(int);
  int i;

  for(i = 0; i < nprimes; ++i) {
    if(BigIntegerModInt(x, primes[i]) == 0)
      return primes[i];
  }
  return 1;
}

/* x + sieve30[x%30] == 11, 23, or 29 (mod 30) */

static int sieve30[] =
{  11, 10,  9,  8,  7,  6,  5,  4,  3,  2,
    1, 12, 11, 10,  9,  8,  7,  6,  5,  4,
    3,  2,  1,  6,  5,  4,  3,  2,  1, 12
};

/* Find a Sophie-Germain prime between "lo" and "hi".  NOTE: this is not
   a "safe prime", but the smaller prime.  Take 2q+1 to get the safe prime. */

static void
sophie_germain(q, lo, hi)
     BigInteger q;              /* assumed initialized */
     const BigInteger lo;
     const BigInteger hi;
{
  BigInteger m, p, r;
  char parambuf[MAXPARAMLEN];
  int foundprime = 0;
  int i, mod30;

  m = BigIntegerFromInt(0);
  BigIntegerSub(m, hi, lo);
  i = (BigIntegerBitLen(m) + 7) / 8;
  t_random(parambuf, i);
  r = BigIntegerFromBytes(parambuf, i);
  BigIntegerMod(r, r, m);

  BigIntegerAdd(q, r, lo);
  if(BigIntegerModInt(q, 2) == 0)
    BigIntegerAddInt(q, q, 1);          /* make q odd */

  mod30 = BigIntegerModInt(q, 30);      /* mod30 = q % 30 */

  BigIntegerFree(m);
  m = BigIntegerFromInt(2);                     /* m = 2 */
  p = BigIntegerFromInt(0);

  while(BigIntegerCmp(q, hi) < 0) {
    if(trialdiv(q) < 2) {
      BigIntegerMulInt(p, q, 2);                        /* p = 2 * q */
      BigIntegerAddInt(p, p, 1);                /* p += 1 */
      if(trialdiv(p) < 2) {
	BigIntegerModExp(r, m, q, q);           /* r = 2^q % q */
	if(BigIntegerCmpInt(r, 2) == 0) {       /* if(r == 2) */
	  BigIntegerModExp(r, m, p, p);         /* r = 2^p % p */
	  if(BigIntegerCmpInt(r, 2) == 0) {     /* if(r == 2) */
	    if(BigIntegerCheckPrime(q) && BigIntegerCheckPrime(p)) {
	      ++foundprime;
	      break;
	    }
	  }
	}
      }
    }

    i = sieve30[mod30];
    BigIntegerAddInt(q, q, i);          /* q += i */
    mod30 = (mod30 + i) % 30;
  }

  /* should wrap around on failure */
  if(!foundprime) {
    fprintf(stderr, "Prime generation failed!\n");
    exit(1);
  }

  BigIntegerFree(r);
  BigIntegerFree(m);
  BigIntegerFree(p);
}

_TYPE( struct t_confent * )
t_makeconfent(tc, nsize)
     struct t_conf * tc;
     int nsize;
{
  BigInteger n, g, q, t, u;

  t = BigIntegerFromInt(0);
  u = BigIntegerFromInt(1);             /* u = 1 */
  BigIntegerLShift(t, u, nsize - 2);    /* t = 2^(nsize-2) */
  BigIntegerMulInt(u, t, 2);            /* u = 2^(nsize-1) */

  q = BigIntegerFromInt(0);
  sophie_germain(q, t, u);

  n = BigIntegerFromInt(0);
  BigIntegerMulInt(n, q, 2);
  BigIntegerAddInt(n, n, 1);

  /* Look for a generator mod n */
  g = BigIntegerFromInt(2);
  while(1) {
    BigIntegerModExp(t, g, q, n);               /* t = g^q % n */
    if(BigIntegerCmpInt(t, 1) == 0)             /* if(t == 1) */
      BigIntegerAddInt(g, g, 1);                /* ++g */
    else
      break;
  }
  BigIntegerFree(t);
  BigIntegerFree(u);
  BigIntegerFree(q);

  tc->tcbuf.modulus.data = tc->modbuf;
  tc->tcbuf.modulus.len = BigIntegerToBytes(n, tc->tcbuf.modulus.data);
  BigIntegerFree(n);

  tc->tcbuf.generator.data = tc->genbuf;
  tc->tcbuf.generator.len = BigIntegerToBytes(g, tc->tcbuf.generator.data);
  BigIntegerFree(g);

  tc->tcbuf.index = 1;
  return &tc->tcbuf;
}

_TYPE( struct t_confent * )
t_makeconfent_c(tc, nsize)
     struct t_conf * tc;
     int nsize;
{
  BigInteger g, n, p, q, j, k, t, u;
  int psize, qsize;

  psize = nsize / 2;
  qsize = nsize - psize;

  t = BigIntegerFromInt(1);             /* t = 1 */
  u = BigIntegerFromInt(0);
  BigIntegerLShift(u, t, psize - 3);    /* u = t*2^(psize-3) = 2^(psize-3) */
  BigIntegerMulInt(t, u, 3);                    /* t = 3*u = 1.5*2^(psize-2) */
  BigIntegerAdd(u, u, t);                       /* u += t [u = 2^(psize-1)] */
  j = BigIntegerFromInt(0);
  sophie_germain(j, t, u);

  k = BigIntegerFromInt(0);
  if(qsize != psize) {
    BigIntegerFree(t);
    t = BigIntegerFromInt(1);           /* t = 1 */
    BigIntegerLShift(u, t, qsize - 3);  /* u = t*2^(qsize-3) = 2^(qsize-3) */
    BigIntegerMulInt(t, u, 3);          /* t = 3*u = 1.5*2^(qsize-2) */
    BigIntegerAdd(u, u, t);             /* u += t [u = 2^(qsize-1)] */
  }
  sophie_germain(k, t, u);

  p = BigIntegerFromInt(0);
  BigIntegerMulInt(p, j, 2);            /* p = 2 * j */
  BigIntegerAddInt(p, p, 1);            /* p += 1 */

  q = BigIntegerFromInt(0);
  BigIntegerMulInt(q, k, 2);            /* q = 2 * k */
  BigIntegerAddInt(q, q, 1);            /* q += 1 */

  n = BigIntegerFromInt(0);
  BigIntegerMul(n, p, q);               /* n = p * q */
  BigIntegerMul(u, j, k);               /* u = j * k */

  BigIntegerFree(p);
  BigIntegerFree(q);
  BigIntegerFree(j);
  BigIntegerFree(k);

  g = BigIntegerFromInt(2);             /* g = 2 */

  /* Look for a generator mod n */
  while(1) {
    BigIntegerModExp(t, g, u, n);       /* t = g^u % n */
    if(BigIntegerCmpInt(t, 1) == 0)
      BigIntegerAddInt(g, g, 1);        /* ++g */
    else
      break;
  }

  BigIntegerFree(u);
  BigIntegerFree(t);

  tc->tcbuf.modulus.data = tc->modbuf;
  tc->tcbuf.modulus.len = BigIntegerToBytes(n, tc->tcbuf.modulus.data);
  BigIntegerFree(n);

  tc->tcbuf.generator.data = tc->genbuf;
  tc->tcbuf.generator.len = BigIntegerToBytes(g, tc->tcbuf.generator.data);
  BigIntegerFree(g);

  tc->tcbuf.index = 1;
  return &tc->tcbuf;
}

_TYPE( struct t_confent * )
t_newconfent(tc)
    struct t_conf * tc;
{
  tc->tcbuf.index = 0;
  tc->tcbuf.modulus.data = tc->modbuf;
  tc->tcbuf.modulus.len = 0;
  tc->tcbuf.generator.data = tc->genbuf;
  tc->tcbuf.generator.len = 0;
  return &tc->tcbuf;
}

_TYPE( void )
t_putconfent(ent, fp)
     const struct t_confent * ent;
     FILE * fp;
{
  char strbuf[MAXB64PARAMLEN];

  fprintf(fp, "%d:%s:", ent->index,
	  t_tob64(strbuf, ent->modulus.data, ent->modulus.len));
  fprintf(fp, "%s\n",
	  t_tob64(strbuf, ent->generator.data, ent->generator.len));
}

int
BigIntegerBitLen(b)
     BigInteger b;
{
  return BN_num_bits(b);
}

int
BigIntegerCheckPrime(n)
     BigInteger n;
{
  BN_CTX * ctx = BN_CTX_new();
  int rv = BN_is_prime(n, 25, NULL, ctx, NULL);
  BN_CTX_free(ctx);
  return rv;
}

unsigned int
BigIntegerModInt(d, m)
     BigInteger d;
     unsigned int m;
{
  return BN_mod_word(d, m);
}

void
BigIntegerMod(result, d, m)
     BigInteger result, d, m;
{
  BN_CTX * ctx = BN_CTX_new();
  BN_mod(result, d, m, ctx);
  BN_CTX_free(ctx);
}

void
BigIntegerMul(result, m1, m2)
     BigInteger result, m1, m2;
{
  BN_CTX * ctx = BN_CTX_new();
  BN_mul(result, m1, m2, ctx);
  BN_CTX_free(ctx);
}

void
BigIntegerLShift(result, x, bits)
     BigInteger result, x;
     unsigned int bits;
{
  BN_lshift(result, x, bits);
}

int BN_is_prime(const BIGNUM *a, int checks, void (*callback)(int,int,void *),
	BN_CTX *ctx_passed, void *cb_arg)
	{
	return BN_is_prime_fasttest(a, checks, callback, ctx_passed, cb_arg, 0);
	}

int BN_is_prime_fasttest(const BIGNUM *a, int checks,
		void (*callback)(int,int,void *),
		BN_CTX *ctx_passed, void *cb_arg,
		int do_trial_division)
	{
	int i, j, ret = -1;
	int k;
	BN_CTX *ctx = NULL;
	BIGNUM *A1, *A1_odd, *check; /* taken from ctx */
	BN_MONT_CTX *mont = NULL;
	const BIGNUM *A = NULL;

	if (checks == BN_prime_checks)
		checks = BN_prime_checks_for_size(BN_num_bits(a));

	/* first look for small factors */
	if (!BN_is_odd(a))
		return(0);
	if (do_trial_division)
		{
		for (i = 1; i < NUMPRIMES; i++)
			if (BN_mod_word(a, primes[i]) == 0)
				return 0;
		if (callback != NULL) callback(1, -1, cb_arg);
		}

	if (ctx_passed != NULL)
		ctx = ctx_passed;
	else
		if ((ctx=BN_CTX_new()) == NULL)
			goto err;
	BN_CTX_start(ctx);

	/* A := abs(a) */
	if (a->neg)
		{
		BIGNUM *t;
		if ((t = BN_CTX_get(ctx)) == NULL) goto err;
		BN_copy(t, a);
		t->neg = 0;
		A = t;
		}
	else
		A = a;
	A1 = BN_CTX_get(ctx);
	A1_odd = BN_CTX_get(ctx);
	check = BN_CTX_get(ctx);
	if (check == NULL) goto err;

	/* compute A1 := A - 1 */
	if (!BN_copy(A1, A))
		goto err;
	if (!BN_sub_word(A1, 1))
		goto err;
	if (BN_is_zero(A1))
		{
		ret = 0;
		goto err;
		}

	/* write  A1  as  A1_odd * 2^k */
	k = 1;
	while (!BN_is_bit_set(A1, k))
		k++;
	if (!BN_rshift(A1_odd, A1, k))
		goto err;

	/* Montgomery setup for computations mod A */
	mont = BN_MONT_CTX_new();
	if (mont == NULL)
		goto err;
	if (!BN_MONT_CTX_set(mont, A, ctx))
		goto err;

	for (i = 0; i < checks; i++)
		{
		if (!BN_pseudo_rand(check, BN_num_bits(A1), 0, 0))
			goto err;
		if (BN_cmp(check, A1) >= 0)
			if (!BN_sub(check, check, A1))
				goto err;
		if (!BN_add_word(check, 1))
			goto err;
		/* now 1 <= check < A */

		j = witness(check, A, A1, A1_odd, k, ctx, mont);
		if (j == -1) goto err;
		if (j)
			{
			ret=0;
			goto err;
			}
		if (callback != NULL) callback(1,i,cb_arg);
		}
	ret=1;
err:
	if (ctx != NULL)
		{
		BN_CTX_end(ctx);
		if (ctx_passed == NULL)
			BN_CTX_free(ctx);
		}
	if (mont != NULL)
		BN_MONT_CTX_free(mont);

	return(ret);
	}

static int witness(BIGNUM *w, const BIGNUM *a, const BIGNUM *a1,
	const BIGNUM *a1_odd, int k, BN_CTX *ctx, BN_MONT_CTX *mont)
	{
	if (!BN_mod_exp_mont(w, w, a1_odd, a, ctx, mont)) /* w := w^a1_odd mod a */
		return -1;
	if (BN_is_one(w))
		return 0; /* probably prime */
	if (BN_cmp(w, a1) == 0)
		return 0; /* w == -1 (mod a),  'a' is probably prime */
	while (--k)
		{
		if (!BN_mod_mul(w, w, w, a, ctx)) /* w := w^2 mod a */
			return -1;
		if (BN_is_one(w))
			return 1; /* 'a' is composite, otherwise a previous 'w' would
				   * have been == -1 (mod 'a') */
		if (BN_cmp(w, a1) == 0)
			return 0; /* w == -1 (mod a), 'a' is probably prime */
		}
	/* If we get here, 'w' is the (a-1)/2-th power of the original 'w',
	 * and it is neither -1 nor +1 -- so 'a' cannot be prime */
	return 1;
	}

int BN_mod_exp_mont(BIGNUM *rr, BIGNUM *a, const BIGNUM *p,
		    const BIGNUM *m, BN_CTX *ctx, BN_MONT_CTX *in_mont)
	{
	int i,j,bits,ret=0,wstart,wend,window,wvalue;
	int start=1,ts=0;
	BIGNUM *d,*r;
	BIGNUM *aa;
	BIGNUM val[TABLE_SIZE];
	BN_MONT_CTX *mont=NULL;

	bn_check_top(a);
	bn_check_top(p);
	bn_check_top(m);

	if (!(m->d[0] & 1))
		{
		return(0);
		}
	bits=BN_num_bits(p);
	if (bits == 0)
		{
		BN_one(rr);
		return(1);
		}
	BN_CTX_start(ctx);
	d = BN_CTX_get(ctx);
	r = BN_CTX_get(ctx);
	if (d == NULL || r == NULL) goto err;

	/* If this is not done, things will break in the montgomery
	 * part */

	if (in_mont != NULL)
		mont=in_mont;
	else
		{
		if ((mont=BN_MONT_CTX_new()) == NULL) goto err;
		if (!BN_MONT_CTX_set(mont,m,ctx)) goto err;
		}

	BN_init(&val[0]);
	ts=1;
	if (BN_ucmp(a,m) >= 0)
		{
		if (!BN_mod(&(val[0]),a,m,ctx))
			goto err;
		aa= &(val[0]);
		}
	else
		aa=a;
	if (!BN_to_montgomery(&(val[0]),aa,mont,ctx)) goto err; /* 1 */

	window = BN_window_bits_for_exponent_size(bits);
	if (window > 1)
		{
		if (!BN_mod_mul_montgomery(d,&(val[0]),&(val[0]),mont,ctx)) goto err; /* 2 */
		j=1<<(window-1);
		for (i=1; i<j; i++)
			{
			BN_init(&(val[i]));
			if (!BN_mod_mul_montgomery(&(val[i]),&(val[i-1]),d,mont,ctx))
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

	if (!BN_to_montgomery(r,BN_value_one(),mont,ctx)) goto err;
	for (;;)
		{
		if (BN_is_bit_set(p,wstart) == 0)
			{
			if (!start)
				{
				if (!BN_mod_mul_montgomery(r,r,r,mont,ctx))
				goto err;
				}
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
				if (!BN_mod_mul_montgomery(r,r,r,mont,ctx))
					goto err;
				}

		/* wvalue will be an odd number < 2^window */
		if (!BN_mod_mul_montgomery(r,r,&(val[wvalue>>1]),mont,ctx))
			goto err;

		/* move the 'window' down further */
		wstart-=wend+1;
		wvalue=0;
		start=0;
		if (wstart < 0) break;
		}
	if (!BN_from_montgomery(rr,r,mont,ctx)) goto err;
	ret=1;
err:
	if ((in_mont == NULL) && (mont != NULL)) BN_MONT_CTX_free(mont);
	BN_CTX_end(ctx);
	for (i=0; i<ts; i++)
		BN_clear_free(&(val[i]));
	return(ret);
	}

BN_ULONG BN_mod_word(const BIGNUM *a, BN_ULONG w)
	{
#ifndef BN_LLONG
	BN_ULONG ret=0;
#else
	BN_ULLONG ret=0;
#endif
	int i;

	w&=BN_MASK2;
	for (i=a->top-1; i>=0; i--)
		{
#ifndef BN_LLONG
		ret=((ret<<BN_BITS4)|((a->d[i]>>BN_BITS4)&BN_MASK2l))%w;
		ret=((ret<<BN_BITS4)|(a->d[i]&BN_MASK2l))%w;
#else
		ret=(BN_ULLONG)(((ret<<(BN_ULLONG)BN_BITS2)|a->d[i])%
			(BN_ULLONG)w);
#endif
		}
	return((BN_ULONG)ret);
	}

static int bnrand(int pseudorand, BIGNUM *rnd, int bits, int top, int bottom)
	{
	unsigned char *buf=NULL;
	int ret=0,bit,bytes,mask;

	if (bits == 0)
		{
		BN_zero(rnd);
		return 1;
		}

	bytes=(bits+7)/8;
	bit=(bits-1)%8;
	mask=0xff<<bit;

	buf=(unsigned char *)malloc(bytes);
	if (buf == NULL)
		{
		goto err;
		}

	/* make a random number and set the top and bottom bits */
	/* this ignores the pseudorand flag */

	t_random(buf, bytes);

	if (top)
		{
		if (bit == 0)
			{
			buf[0]=1;
			buf[1]|=0x80;
			}
		else
			{
			buf[0]|=(3<<(bit-1));
			buf[0]&= ~(mask<<1);
			}
		}
	else
		{
		buf[0]|=(1<<bit);
		buf[0]&= ~(mask<<1);
		}
	if (bottom) /* set bottom bits to whatever odd is */
		buf[bytes-1]|=1;
	if (!BN_bin2bn(buf,bytes,rnd)) goto err;
	ret=1;
err:
	if (buf != NULL)
		{
		memset(buf,0,bytes);
		free(buf);
		}
	return(ret);
	}

/* BN_pseudo_rand is the same as BN_rand, now. */

int     BN_pseudo_rand(BIGNUM *rnd, int bits, int top, int bottom)
	{
	return bnrand(1, rnd, bits, top, bottom);
	}

#define MONT_WORD /* use the faster word-based algorithm */

int BN_mod_mul_montgomery(BIGNUM *r, BIGNUM *a, BIGNUM *b,
			  BN_MONT_CTX *mont, BN_CTX *ctx)
	{
	BIGNUM *tmp,*tmp2;
	int ret=0;

	BN_CTX_start(ctx);
	tmp = BN_CTX_get(ctx);
	tmp2 = BN_CTX_get(ctx);
	if (tmp == NULL || tmp2 == NULL) goto err;

	bn_check_top(tmp);
	bn_check_top(tmp2);

	if (a == b)
		{
		if (!BN_sqr(tmp,a,ctx)) goto err;
		}
	else
		{
		if (!BN_mul(tmp,a,b,ctx)) goto err;
		}
	/* reduce from aRR to aR */
	if (!BN_from_montgomery(r,tmp,mont,ctx)) goto err;
	ret=1;
err:
	BN_CTX_end(ctx);
	return(ret);
	}

int BN_from_montgomery(BIGNUM *ret, BIGNUM *a, BN_MONT_CTX *mont,
	     BN_CTX *ctx)
	{
	int retn=0;

#ifdef MONT_WORD
	BIGNUM *n,*r;
	BN_ULONG *ap,*np,*rp,n0,v,*nrp;
	int al,nl,max,i,x,ri;

	BN_CTX_start(ctx);
	if ((r = BN_CTX_get(ctx)) == NULL) goto err;

	if (!BN_copy(r,a)) goto err;
	n= &(mont->N);

	ap=a->d;
	/* mont->ri is the size of mont->N in bits (rounded up
	   to the word size) */
	al=ri=mont->ri/BN_BITS2;

	nl=n->top;
	if ((al == 0) || (nl == 0)) { r->top=0; return(1); }

	max=(nl+al+1); /* allow for overflow (no?) XXX */
	if (bn_wexpand(r,max) == NULL) goto err;
	if (bn_wexpand(ret,max) == NULL) goto err;

	r->neg=a->neg^n->neg;
	np=n->d;
	rp=r->d;
	nrp= &(r->d[nl]);

	/* clear the top words of T */
#if 1
	for (i=r->top; i<max; i++) /* memset? XXX */
		r->d[i]=0;
#else
	memset(&(r->d[r->top]),0,(max-r->top)*sizeof(BN_ULONG));
#endif

	r->top=max;
	n0=mont->n0;

#ifdef BN_COUNT
	printf("word BN_from_montgomery %d * %d\n",nl,nl);
#endif
	for (i=0; i<nl; i++)
		{
#ifdef __TANDEM
		{
		   long long t1;
		   long long t2;
		   long long t3;
		   t1 = rp[0] * (n0 & 0177777);
		   t2 = 037777600000l;
		   t2 = n0 & t2;
		   t3 = rp[0] & 0177777;
		   t2 = (t3 * t2) & BN_MASK2;
		   t1 = t1 + t2;
		   v=bn_mul_add_words(rp,np,nl,(BN_ULONG) t1);
		}
#else
		v=bn_mul_add_words(rp,np,nl,(rp[0]*n0)&BN_MASK2);
#endif
		nrp++;
		rp++;
		if (((nrp[-1]+=v)&BN_MASK2) >= v)
			continue;
		else
			{
			if (((++nrp[0])&BN_MASK2) != 0) continue;
			if (((++nrp[1])&BN_MASK2) != 0) continue;
			for (x=2; (((++nrp[x])&BN_MASK2) == 0); x++) ;
			}
		}
	bn_fix_top(r);

	/* mont->ri will be a multiple of the word size */
#if 0
	BN_rshift(ret,r,mont->ri);
#else
	ret->neg = r->neg;
	x=ri;
	rp=ret->d;
	ap= &(r->d[x]);
	if (r->top < x)
		al=0;
	else
		al=r->top-x;
	ret->top=al;
	al-=4;
	for (i=0; i<al; i+=4)
		{
		BN_ULONG t1,t2,t3,t4;

		t1=ap[i+0];
		t2=ap[i+1];
		t3=ap[i+2];
		t4=ap[i+3];
		rp[i+0]=t1;
		rp[i+1]=t2;
		rp[i+2]=t3;
		rp[i+3]=t4;
		}
	al+=4;
	for (; i<al; i++)
		rp[i]=ap[i];
#endif
#else /* !MONT_WORD */
	BIGNUM *t1,*t2;

	BN_CTX_start(ctx);
	t1 = BN_CTX_get(ctx);
	t2 = BN_CTX_get(ctx);
	if (t1 == NULL || t2 == NULL) goto err;

	if (!BN_copy(t1,a)) goto err;
	BN_mask_bits(t1,mont->ri);

	if (!BN_mul(t2,t1,&mont->Ni,ctx)) goto err;
	BN_mask_bits(t2,mont->ri);

	if (!BN_mul(t1,t2,&mont->N,ctx)) goto err;
	if (!BN_add(t2,a,t1)) goto err;
	BN_rshift(ret,t2,mont->ri);
#endif /* MONT_WORD */

	if (BN_ucmp(ret, &(mont->N)) >= 0)
		{
		BN_usub(ret,ret,&(mont->N));
		}
	retn=1;
 err:
	BN_CTX_end(ctx);
	return(retn);
	}

void BN_MONT_CTX_init(BN_MONT_CTX *ctx)
	{
	ctx->ri=0;
	BN_init(&(ctx->RR));
	BN_init(&(ctx->N));
	BN_init(&(ctx->Ni));
	ctx->flags=0;
	}

BN_MONT_CTX *BN_MONT_CTX_new(void)
	{
	BN_MONT_CTX *ret;

	if ((ret=(BN_MONT_CTX *)malloc(sizeof(BN_MONT_CTX))) == NULL)
		return(NULL);

	BN_MONT_CTX_init(ret);
	ret->flags=BN_FLG_MALLOCED;
	return(ret);
	}

void BN_MONT_CTX_free(BN_MONT_CTX *mont)
	{
	if(mont == NULL)
	    return;

	BN_free(&(mont->RR));
	BN_free(&(mont->N));
	BN_free(&(mont->Ni));
	if (mont->flags & BN_FLG_MALLOCED)
		free(mont);
	}

int BN_MONT_CTX_set(BN_MONT_CTX *mont, const BIGNUM *mod, BN_CTX *ctx)
	{
	BIGNUM Ri,*R;

	BN_init(&Ri);
	R= &(mont->RR);                                 /* grab RR as a temp */
	BN_copy(&(mont->N),mod);                        /* Set N */

#ifdef MONT_WORD
		{
		BIGNUM tmod;
		BN_ULONG buf[2];

		mont->ri=(BN_num_bits(mod)+(BN_BITS2-1))/BN_BITS2*BN_BITS2;
		BN_zero(R);
		BN_set_bit(R,BN_BITS2);                 /* R */

		buf[0]=mod->d[0]; /* tmod = N mod word size */
		buf[1]=0;
		tmod.d=buf;
		tmod.top=1;
		tmod.dmax=2;
		tmod.neg=mod->neg;
							/* Ri = R^-1 mod N*/
		if ((BN_mod_inverse(&Ri,R,&tmod,ctx)) == NULL)
			goto err;
		BN_lshift(&Ri,&Ri,BN_BITS2);            /* R*Ri */
		if (!BN_is_zero(&Ri))
			BN_sub_word(&Ri,1);
		else /* if N mod word size == 1 */
			BN_set_word(&Ri,BN_MASK2);  /* Ri-- (mod word size) */
		BN_div(&Ri,NULL,&Ri,&tmod,ctx); /* Ni = (R*Ri-1)/N,
						 * keep only least significant word: */
		mont->n0=Ri.d[0];
		BN_free(&Ri);
		}
#else /* !MONT_WORD */
		{ /* bignum version */
		mont->ri=BN_num_bits(mod);
		BN_zero(R);
		BN_set_bit(R,mont->ri);                 /* R = 2^ri */
							/* Ri = R^-1 mod N*/
		if ((BN_mod_inverse(&Ri,R,mod,ctx)) == NULL)
			goto err;
		BN_lshift(&Ri,&Ri,mont->ri);            /* R*Ri */
		BN_sub_word(&Ri,1);
							/* Ni = (R*Ri-1) / N */
		BN_div(&(mont->Ni),NULL,&Ri,mod,ctx);
		BN_free(&Ri);
		}
#endif

	/* setup RR for conversions */
	BN_zero(&(mont->RR));
	BN_set_bit(&(mont->RR),mont->ri*2);
	BN_mod(&(mont->RR),&(mont->RR),&(mont->N),ctx);

	return(1);
err:
	return(0);
	}

BIGNUM *BN_value_one(void)
	{
	static BN_ULONG data_one=1L;
	static BIGNUM const_one={&data_one,1,1,0};

	return(&const_one);
	}

/* solves ax == 1 (mod n) */
BIGNUM *BN_mod_inverse(BIGNUM *in, BIGNUM *a, const BIGNUM *n, BN_CTX *ctx)
	{
	BIGNUM *A,*B,*X,*Y,*M,*D,*R=NULL;
	BIGNUM *T,*ret=NULL;
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
	if (Y == NULL) goto err;

	if (in == NULL)
		R=BN_new();
	else
		R=in;
	if (R == NULL) goto err;

	BN_zero(X);
	BN_one(Y);
	if (BN_copy(A,a) == NULL) goto err;
	if (BN_copy(B,n) == NULL) goto err;
	sign=1;

	while (!BN_is_zero(B))
		{
		if (!BN_div(D,M,A,B,ctx)) goto err;
		T=A;
		A=B;
		B=M;
		/* T has a struct, M does not */

		if (!BN_mul(T,D,X,ctx)) goto err;
		if (!BN_add(T,T,Y)) goto err;
		M=Y;
		Y=X;
		X=T;
		sign= -sign;
		}
	if (sign < 0)
		{
		if (!BN_sub(Y,n,Y)) goto err;
		}

	if (BN_is_one(A))
		{ if (!BN_mod(R,Y,n,ctx)) goto err; }
	else
		{
		goto err;
		}
	ret=R;
err:
	if ((ret == NULL) && (in == NULL)) BN_free(R);
	BN_CTX_end(ctx);
	return(ret);
	}

int BN_set_bit(BIGNUM *a, int n)
	{
	int i,j,k;

	i=n/BN_BITS2;
	j=n%BN_BITS2;
	if (a->top <= i)
		{
		if (bn_wexpand(a,i+1) == NULL) return(0);
		for(k=a->top; k<i+1; k++)
			a->d[k]=0;
		a->top=i+1;
		}

	a->d[i]|=(((BN_ULONG)1)<<j);
	return(1);
	}


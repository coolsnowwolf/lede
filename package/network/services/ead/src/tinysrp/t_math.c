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
#include <sys/types.h>

#include "config.h"

#include "bn.h"
typedef BIGNUM * BigInteger;
#define MATH_PRIV

#include "t_defines.h"
#include "t_pwd.h"

/* Math library interface stubs */

BigInteger
BigIntegerFromInt(n)
     unsigned int n;
{
  BIGNUM * a = BN_new();
  BN_set_word(a, n);
  return a;
}

BigInteger
BigIntegerFromBytes(bytes, length)
     unsigned char * bytes;
     int length;
{
  BIGNUM * a = BN_new();
  BN_bin2bn(bytes, length, a);
  return a;
}

int
BigIntegerToBytes(src, dest)
     BigInteger src;
     unsigned char * dest;
{
  return BN_bn2bin(src, dest);
}

int
BigIntegerCmp(c1, c2)
     BigInteger c1, c2;
{
  return BN_cmp(c1, c2);
}

int
BigIntegerCmpInt(c1, c2)
     BigInteger c1;
     unsigned int c2;
{
  BIGNUM * a = BN_new();
  int rv;
  BN_set_word(a, c2);
  rv = BN_cmp(c1, a);
  BN_free(a);
  return rv;
}

void
BigIntegerAdd(result, a1, a2)
     BigInteger result, a1, a2;
{
  BN_add(result, a1, a2);
}

void
BigIntegerAddInt(result, a1, a2)
     BigInteger result, a1;
     unsigned int a2;
{
  BIGNUM * a = BN_new();
  BN_set_word(a, a2);
  BN_add(result, a1, a);
  BN_free(a);
}

void
BigIntegerSub(result, s1, s2)
     BigInteger result, s1, s2;
{
  BN_sub(result, s1, s2);
}

void
BigIntegerMulInt(result, m1, m2)
     BigInteger result, m1;
     unsigned int m2;
{
  BN_CTX * ctx = BN_CTX_new();
  BIGNUM * m = BN_new();
  BN_set_word(m, m2);
  BN_mul(result, m1, m, ctx);
  BN_CTX_free(ctx);
}

void
BigIntegerModMul(r, m1, m2, modulus)
     BigInteger r, m1, m2, modulus;
{
  BN_CTX * ctx = BN_CTX_new();
  BN_mod_mul(r, m1, m2, modulus, ctx);
  BN_CTX_free(ctx);
}

void
BigIntegerModExp(r, b, e, m)
     BigInteger r, b, e, m;
{
  BN_CTX * ctx = BN_CTX_new();
  BN_mod_exp(r, b, e, m, ctx);
  BN_CTX_free(ctx);
}

void
BigIntegerModExpInt(r, b, e, m)
     BigInteger r, b;
     unsigned int e;
     BigInteger m;
{
  BN_CTX * ctx = BN_CTX_new();
  BIGNUM * p = BN_new();
  BN_set_word(p, e);
  BN_mod_exp(r, b, p, m, ctx);
  BN_free(p);
  BN_CTX_free(ctx);
}

void
BigIntegerFree(b)
     BigInteger b;
{
  BN_free(b);
}

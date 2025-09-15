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
#include "t_server.h"

_TYPE( struct t_server * )
t_serveropenraw(ent, tce)
     struct t_pwent * ent;
     struct t_confent * tce;
{
  struct t_server * ts;
  unsigned char buf1[SHA_DIGESTSIZE], buf2[SHA_DIGESTSIZE];
  SHA1_CTX ctxt;
  int i;

  if((ts = malloc(sizeof(struct t_server))) == 0)
    return 0;

  SHA1Init(&ts->ckhash);

  ts->index = ent->index;
  ts->n.len = tce->modulus.len;
  ts->n.data = ts->nbuf;
  memcpy(ts->n.data, tce->modulus.data, ts->n.len);

  SHA1Init(&ctxt);
  SHA1Update(&ctxt, ts->n.data, ts->n.len);
  SHA1Final(buf1, &ctxt);

  ts->g.len = tce->generator.len;
  ts->g.data = ts->gbuf;
  memcpy(ts->g.data, tce->generator.data, ts->g.len);

  SHA1Init(&ctxt);
  SHA1Update(&ctxt, ts->g.data, ts->g.len);
  SHA1Final(buf2, &ctxt);

  for(i = 0; i < sizeof(buf1); ++i)
    buf1[i] ^= buf2[i];

  SHA1Update(&ts->ckhash, buf1, sizeof(buf1));

  SHA1Init(&ctxt);
  SHA1Update(&ctxt, ent->name, strlen(ent->name));
  SHA1Final(buf1, &ctxt);

  SHA1Update(&ts->ckhash, buf1, sizeof(buf1));

  ts->v.len = ent->password.len;
  ts->v.data = ts->vbuf;
  memcpy(ts->v.data, ent->password.data, ts->v.len);

  ts->s.len = ent->salt.len;
  ts->s.data = ts->saltbuf;
  memcpy(ts->s.data, ent->salt.data, ts->s.len);

  SHA1Update(&ts->ckhash, ts->s.data, ts->s.len);

  ts->b.data = ts->bbuf;
  ts->B.data = ts->Bbuf;

  SHA1Init(&ts->hash);
  SHA1Init(&ts->oldhash);
  SHA1Init(&ts->oldckhash);

  return ts;
}

_TYPE( struct t_num * )
t_servergenexp(ts)
     struct t_server * ts;
{
  BigInteger b, B, v, n, g;

  if(ts->n.len < BLEN)
    ts->b.len = ts->n.len;
  else
    ts->b.len = BLEN;

  t_random(ts->b.data, ts->b.len);
  b = BigIntegerFromBytes(ts->b.data, ts->b.len);
  n = BigIntegerFromBytes(ts->n.data, ts->n.len);
  g = BigIntegerFromBytes(ts->g.data, ts->g.len);
  B = BigIntegerFromInt(0);
  BigIntegerModExp(B, g, b, n);

  v = BigIntegerFromBytes(ts->v.data, ts->v.len);
  BigIntegerAdd(B, B, v);
  if(BigIntegerCmp(B, n) > 0)
    BigIntegerSub(B, B, n);

  ts->B.len = BigIntegerToBytes(B, ts->B.data);

  BigIntegerFree(v);
  BigIntegerFree(B);
  BigIntegerFree(b);
  BigIntegerFree(g);
  BigIntegerFree(n);

  SHA1Update(&ts->oldckhash, ts->B.data, ts->B.len);

  return &ts->B;
}

_TYPE( unsigned char * )
t_servergetkey(ts, clientval)
     struct t_server * ts;
     struct t_num * clientval;
{
  BigInteger n, v, A, b, prod, res, S;
  SHA1_CTX ctxt;
  unsigned char sbuf[MAXPARAMLEN];
  unsigned char dig[SHA_DIGESTSIZE];
  unsigned slen;
  unsigned int u;

  SHA1Update(&ts->ckhash, clientval->data, clientval->len);
  SHA1Update(&ts->ckhash, ts->B.data, ts->B.len);

  SHA1Init(&ctxt);
  SHA1Update(&ctxt, ts->B.data, ts->B.len);
  SHA1Final(dig, &ctxt);
  u = (dig[0] << 24) | (dig[1] << 16) | (dig[2] << 8) | dig[3];

  SHA1Update(&ts->oldhash, clientval->data, clientval->len);
  SHA1Update(&ts->hash, clientval->data, clientval->len);

  n = BigIntegerFromBytes(ts->n.data, ts->n.len);
  b = BigIntegerFromBytes(ts->b.data, ts->b.len);
  v = BigIntegerFromBytes(ts->v.data, ts->v.len);
  A = BigIntegerFromBytes(clientval->data, clientval->len);

  prod = BigIntegerFromInt(0);
  BigIntegerModExpInt(prod, v, u, n);
  res = BigIntegerFromInt(0);
  BigIntegerModMul(res, prod, A, n);

  BigIntegerFree(A);
  BigIntegerFree(v);
  BigIntegerFree(prod);

  if(BigIntegerCmpInt(res, 1) <= 0) {   /* Check for Av^u == 1 (mod n) */
    BigIntegerFree(res);
    BigIntegerFree(b);
    BigIntegerFree(n);
    return NULL;
  }

  S = BigIntegerFromInt(0);

  BigIntegerAddInt(S, res, 1);
  if(BigIntegerCmp(S, n) == 0) {        /* Check for Av^u == -1 (mod n) */
    BigIntegerFree(res);
    BigIntegerFree(b);
    BigIntegerFree(n);
    BigIntegerFree(S);
    return NULL;
  }

  BigIntegerModExp(S, res, b, n);
  slen = BigIntegerToBytes(S, sbuf);

  BigIntegerFree(S);
  BigIntegerFree(res);
  BigIntegerFree(b);
  BigIntegerFree(n);

  t_sessionkey(ts->session_key, sbuf, slen);
  memset(sbuf, 0, slen);

  SHA1Update(&ts->oldhash, ts->session_key, sizeof(ts->session_key));
  SHA1Update(&ts->oldckhash, ts->session_key, sizeof(ts->session_key));
  SHA1Update(&ts->ckhash, ts->session_key, sizeof(ts->session_key));

  return ts->session_key;
}

_TYPE( int )
t_serververify(ts, resp)
    struct t_server * ts;
    unsigned char * resp;
{
  unsigned char expected[SHA_DIGESTSIZE];
  int i;

  SHA1Final(expected, &ts->oldckhash);
  i = memcmp(expected, resp, sizeof(expected));
  if(i == 0) {
    SHA1Final(ts->session_response, &ts->oldhash);
    return 0;
  }
  SHA1Final(expected, &ts->ckhash);
  i = memcmp(expected, resp, sizeof(expected));
  if(i == 0) {
    SHA1Update(&ts->hash, expected, sizeof(expected));
    SHA1Update(&ts->hash, ts->session_key, sizeof(ts->session_key));
    SHA1Final(ts->session_response, &ts->hash);
  }
  return i;
}

_TYPE( unsigned char * )
t_serverresponse(ts)
    struct t_server * ts;
{
  return ts->session_response;
}

_TYPE( void )
t_serverclose(ts)
     struct t_server * ts;
{
  memset(ts->bbuf, 0, sizeof(ts->bbuf));
  memset(ts->vbuf, 0, sizeof(ts->vbuf));
  memset(ts->saltbuf, 0, sizeof(ts->saltbuf));
  memset(ts->session_key, 0, sizeof(ts->session_key));
  free(ts);
}

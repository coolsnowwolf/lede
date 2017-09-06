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
#include "t_client.h"
#include "t_sha.h"

_TYPE( struct t_client * )
t_clientopen(u, n, g, s)
     const char * u;
     struct t_num * n;
     struct t_num * g;
     struct t_num * s;
{
  struct t_client * tc;
  unsigned char buf1[SHA_DIGESTSIZE], buf2[SHA_DIGESTSIZE];
  SHA1_CTX ctxt;
  int i, validated;
  struct t_preconf * tpc;

  validated = 0;
  if(n->len < MIN_MOD_BYTES)
    return 0;
  for(i = 0; i < t_getprecount(); ++i) {
    tpc = t_getpreparam(i);
    if(tpc->modulus.len == n->len && tpc->generator.len == g->len &&
       memcmp(tpc->modulus.data, n->data, n->len) == 0 &&
       memcmp(tpc->generator.data, g->data, g->len) == 0) {
      validated = 1;    /* Match found, done */
      break;
    }
  }

  if(validated == 0)
    return 0;

  if((tc = malloc(sizeof(struct t_client))) == 0)
    return 0;

  strncpy(tc->username, u, MAXUSERLEN);

  SHA1Init(&tc->hash);

  tc->n.len = n->len;
  tc->n.data = tc->nbuf;
  memcpy(tc->n.data, n->data, tc->n.len);

  SHA1Init(&ctxt);
  SHA1Update(&ctxt, tc->n.data, tc->n.len);
  SHA1Final(buf1, &ctxt);

  tc->g.len = g->len;
  tc->g.data = tc->gbuf;
  memcpy(tc->g.data, g->data, tc->g.len);

  SHA1Init(&ctxt);
  SHA1Update(&ctxt, tc->g.data, tc->g.len);
  SHA1Final(buf2, &ctxt);

  for(i = 0; i < sizeof(buf1); ++i)
    buf1[i] ^= buf2[i];

  SHA1Update(&tc->hash, buf1, sizeof(buf1));

  SHA1Init(&ctxt);
  SHA1Update(&ctxt, tc->username, strlen(tc->username));
  SHA1Final(buf1, &ctxt);

  SHA1Update(&tc->hash, buf1, sizeof(buf1));

  tc->s.len = s->len;
  tc->s.data = tc->sbuf;
  memcpy(tc->s.data, s->data, tc->s.len);

  SHA1Update(&tc->hash, tc->s.data, tc->s.len);

  tc->a.data = tc->abuf;
  tc->A.data = tc->Abuf;
  tc->p.data = tc->pbuf;
  tc->v.data = tc->vbuf;

  SHA1Init(&tc->ckhash);

  return tc;
}

_TYPE( struct t_num * )
t_clientgenexp(tc)
     struct t_client * tc;
{
  BigInteger a, A, n, g;

  if(tc->n.len < ALEN)
    tc->a.len = tc->n.len;
  else
    tc->a.len = ALEN;

  t_random(tc->a.data, tc->a.len);
  a = BigIntegerFromBytes(tc->a.data, tc->a.len);
  n = BigIntegerFromBytes(tc->n.data, tc->n.len);
  g = BigIntegerFromBytes(tc->g.data, tc->g.len);
  A = BigIntegerFromInt(0);
  BigIntegerModExp(A, g, a, n);
  tc->A.len = BigIntegerToBytes(A, tc->A.data);

  BigIntegerFree(A);
  BigIntegerFree(a);
  BigIntegerFree(g);
  BigIntegerFree(n);

  SHA1Update(&tc->hash, tc->A.data, tc->A.len);
  SHA1Update(&tc->ckhash, tc->A.data, tc->A.len);

  return &tc->A;
}

_TYPE( void )
t_clientpasswd(tc, password)
     struct t_client * tc;
     char * password;
{
  BigInteger n, g, p, v;
  SHA1_CTX ctxt;
  unsigned char dig[SHA_DIGESTSIZE];

  n = BigIntegerFromBytes(tc->n.data, tc->n.len);
  g = BigIntegerFromBytes(tc->g.data, tc->g.len);

  SHA1Init(&ctxt);
  SHA1Update(&ctxt, tc->username, strlen(tc->username));
  SHA1Update(&ctxt, ":", 1);
  SHA1Update(&ctxt, password, strlen(password));
  SHA1Final(dig, &ctxt);

  SHA1Init(&ctxt);
  SHA1Update(&ctxt, tc->s.data, tc->s.len);
  SHA1Update(&ctxt, dig, sizeof(dig));
  SHA1Final(dig, &ctxt);

  p = BigIntegerFromBytes(dig, sizeof(dig));

  v = BigIntegerFromInt(0);
  BigIntegerModExp(v, g, p, n);

  tc->p.len = BigIntegerToBytes(p, tc->p.data);
  BigIntegerFree(p);

  tc->v.len = BigIntegerToBytes(v, tc->v.data);
  BigIntegerFree(v);
}

_TYPE( unsigned char * )
t_clientgetkey(tc, serverval)
     struct t_client * tc;
     struct t_num * serverval;
{
  BigInteger n, B, v, p, a, sum, S;
  unsigned char sbuf[MAXPARAMLEN];
  unsigned char dig[SHA_DIGESTSIZE];
  unsigned slen;
  unsigned int u;
  SHA1_CTX ctxt;

  SHA1Init(&ctxt);
  SHA1Update(&ctxt, serverval->data, serverval->len);
  SHA1Final(dig, &ctxt);
  u = (dig[0] << 24) | (dig[1] << 16) | (dig[2] << 8) | dig[3];
  if(u == 0)
    return NULL;

  SHA1Update(&tc->hash, serverval->data, serverval->len);

  B = BigIntegerFromBytes(serverval->data, serverval->len);
  n = BigIntegerFromBytes(tc->n.data, tc->n.len);

  if(BigIntegerCmp(B, n) >= 0 || BigIntegerCmpInt(B, 0) == 0) {
    BigIntegerFree(B);
    BigIntegerFree(n);
    return NULL;
  }
  v = BigIntegerFromBytes(tc->v.data, tc->v.len);
  if(BigIntegerCmp(B, v) < 0)
    BigIntegerAdd(B, B, n);
  BigIntegerSub(B, B, v);
  BigIntegerFree(v);

  a = BigIntegerFromBytes(tc->a.data, tc->a.len);
  p = BigIntegerFromBytes(tc->p.data, tc->p.len);

  sum = BigIntegerFromInt(0);
  BigIntegerMulInt(sum, p, u);
  BigIntegerAdd(sum, sum, a);

  BigIntegerFree(p);
  BigIntegerFree(a);

  S = BigIntegerFromInt(0);
  BigIntegerModExp(S, B, sum, n);
  slen = BigIntegerToBytes(S, sbuf);

  BigIntegerFree(S);
  BigIntegerFree(sum);
  BigIntegerFree(B);
  BigIntegerFree(n);

  t_sessionkey(tc->session_key, sbuf, slen);
  memset(sbuf, 0, slen);

  SHA1Update(&tc->hash, tc->session_key, sizeof(tc->session_key));

  SHA1Final(tc->session_response, &tc->hash);
  SHA1Update(&tc->ckhash, tc->session_response, sizeof(tc->session_response));
  SHA1Update(&tc->ckhash, tc->session_key, sizeof(tc->session_key));

  return tc->session_key;
}

_TYPE( int )
t_clientverify(tc, resp)
    struct t_client * tc;
    unsigned char * resp;
{
  unsigned char expected[SHA_DIGESTSIZE];

  SHA1Final(expected, &tc->ckhash);
  return memcmp(expected, resp, sizeof(expected));
}

_TYPE( unsigned char * )
t_clientresponse(tc)
    struct t_client * tc;
{
  return tc->session_response;
}

_TYPE( void )
t_clientclose(tc)
     struct t_client * tc;
{
  memset(tc->abuf, 0, sizeof(tc->abuf));
  memset(tc->pbuf, 0, sizeof(tc->pbuf));
  memset(tc->vbuf, 0, sizeof(tc->vbuf));
  memset(tc->session_key, 0, sizeof(tc->session_key));
  free(tc);
}

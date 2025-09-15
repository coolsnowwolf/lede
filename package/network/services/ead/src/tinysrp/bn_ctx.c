/* crypto/bn/bn_ctx.c */
/* Written by Ulf Moeller for the OpenSSL project. */
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

#ifndef BN_CTX_DEBUG
# undef NDEBUG /* avoid conflicting definitions */
# define NDEBUG
#endif

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <bn.h>


BN_CTX *BN_CTX_new(void)
	{
	BN_CTX *ret;

	ret=(BN_CTX *)malloc(sizeof(BN_CTX));
	if (ret == NULL)
		{
		return(NULL);
		}

	BN_CTX_init(ret);
	ret->flags=BN_FLG_MALLOCED;
	return(ret);
	}

void BN_CTX_init(BN_CTX *ctx)
	{
	int i;
	ctx->tos = 0;
	ctx->flags = 0;
	ctx->depth = 0;
	ctx->too_many = 0;
	for (i = 0; i < BN_CTX_NUM; i++)
		BN_init(&(ctx->bn[i]));
	}

void BN_CTX_free(BN_CTX *ctx)
	{
	int i;

	if (ctx == NULL) return;
	assert(ctx->depth == 0);

	for (i=0; i < BN_CTX_NUM; i++)
		BN_clear_free(&(ctx->bn[i]));
	if (ctx->flags & BN_FLG_MALLOCED)
		free(ctx);
	}

void BN_CTX_start(BN_CTX *ctx)
	{
	if (ctx->depth < BN_CTX_NUM_POS)
		ctx->pos[ctx->depth] = ctx->tos;
	ctx->depth++;
	}

BIGNUM *BN_CTX_get(BN_CTX *ctx)
	{
	if (ctx->depth > BN_CTX_NUM_POS || ctx->tos >= BN_CTX_NUM)
		{
		if (!ctx->too_many)
			{
			/* disable error code until BN_CTX_end is called: */
			ctx->too_many = 1;
			}
		return NULL;
		}
	return (&(ctx->bn[ctx->tos++]));
	}

void BN_CTX_end(BN_CTX *ctx)
	{
	if (ctx == NULL) return;
	assert(ctx->depth > 0);
	if (ctx->depth == 0)
		/* should never happen, but we can tolerate it if not in
		 * debug mode (could be a 'goto err' in the calling function
		 * before BN_CTX_start was reached) */
		BN_CTX_start(ctx);

	ctx->too_many = 0;
	ctx->depth--;
	if (ctx->depth < BN_CTX_NUM_POS)
		ctx->tos = ctx->pos[ctx->depth];
	}

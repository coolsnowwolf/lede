
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

/****************************************************************************
* Mediatek Inc.
* 5F., No.5, Taiyuan 1st St., Zhubei City,
* Hsinchu County 302, Taiwan, R.O.C.
* (c) Copyright 2014, Mediatek, Inc.
*
* All rights reserved. Mediatek's source code is an unpublished work and the
* use of a copyright notice does not imply otherwise. This source code
* contains confidential trade secret material of Mediatek. Any attemp
* or participation in deciphering, decoding, reverse engineering or in any
* way altering the source code is stricitly prohibited, unless the prior
* written consent of Mediatek, Inc. is obtained.
****************************************************************************

	Module Name:
	crypt_bignum.c

	Abstract:
	BN wrepper for reference OPENSSL funcitons

	Revision History:
	Who         When          What
	--------    ----------    ---------------------------------------------
	Ellis       2018.3.1      Initial version
*/


#ifdef DOT11_SAE_OPENSSL_BN
#include "security/crypt_bignum.h"
#include "rt_config.h"
#include <linux/time.h>



#ifdef __KERNEL__
#define DEBUGPRINT(fmt, args...) printk(fmt, ## args)
#else
#define DEBUGPRINT(fmt, args...) printf(fmt, ## args)
#endif /* __KERNEL__ */

VOID Bignum_Print (
	IN BIGNUM *pBI)
{
	UINT32 i, j = 0;

	DEBUGPRINT("dmax = %d, top = %d\n", pBI->dmax, pBI->top);

	for (i = pBI->dmax - 1; j < pBI->dmax; i--, j++) {
		if (j/4 > 0 && j % 4 == 0)
			DEBUGPRINT("\n");
		DEBUGPRINT("%02x %02x %02x %02x ", (pBI->d[i] & 0xff000000) >> 24,
			(pBI->d[i] & 0xff0000) >> 16, (pBI->d[i] & 0xff00) >> 8, (pBI->d[i] & 0xff)) ;
	}

	DEBUGPRINT("\n");
	return;
}



VOID Bignum_Init(
	INOUT BIGNUM **pBI)
{
	if (*pBI == NULL) {
		*pBI = BN_new();
	}
}

VOID Bignum_Free(
	IN BIGNUM **pBI)
{
	if (*pBI != NULL)
		BN_free(*pBI);
	*pBI = NULL;
}

UINT32 Bignum_getlen(
	IN BIGNUM *pBI)
{
	return pBI->top * 4;
}

INT Bignum_Get_rand_range(IN BIGNUM * range, INOUT BIGNUM * r)
{
	return BN_rand_range(r, (const BIGNUM *)range);
}

VOID Bignum_BI2Bin(
	IN BIGNUM *pBI,
	OUT UINT8 *pValue,
	OUT UINT *Length)
{
	*Length = BN_num_bytes(pBI);
	BN_bn2bin(pBI, pValue);
} /* End of Bignum_BI2Bin */


VOID Bignum_BI2Bin_with_pad(
	IN BIGNUM *pBI,
	OUT UINT8 *pValue,
	IN UINT *Length,
	IN UINT32 PadLen)
{
	UINT32 num_bytes, offset;

	num_bytes = BN_num_bytes((const BIGNUM *) pBI);

	if (PadLen > num_bytes)
		offset = PadLen - num_bytes;
	else
		offset = 0;

	NdisZeroMemory(pValue, offset);
	Bignum_BI2Bin(pBI, pValue + offset, Length);
	*Length += offset;
}


VOID Bignum_Bin2BI(
	IN UINT8 *pValue,
	IN UINT Length,
	OUT BIGNUM **pBI)
{
	if (*pBI)
		BN_free(*pBI);
	*pBI = BN_bin2bn(pValue, Length, NULL);
} /* End of Bignum_Bin2BI */


VOID Bignum_Copy(
	IN  BIGNUM *pBI_Copied,
	OUT BIGNUM **pBI_Result)
{
	if (*pBI_Result)
		BN_free(*pBI_Result);
	*pBI_Result = BN_dup(pBI_Copied);
} /* End of Bignum_Copy */


INT Bignum_UnsignedCompare(
	IN BIGNUM *pFirstOperand,
	IN BIGNUM *pSecondOperand)
{
	return BN_ucmp(pFirstOperand, pSecondOperand);
} /* End of Bignum_Compare */


VOID Bignum_Add(
	IN BIGNUM *pFirstOperand,
	IN BIGNUM *pSecondOperand,
	OUT BIGNUM **pBI_Result)
{
	if (*pBI_Result == NULL)
		*pBI_Result = BN_new();
	BN_add(*pBI_Result, pFirstOperand, pSecondOperand);
} /* End of Bignum_Add */


VOID Bignum_Sub(
	IN BIGNUM *pFirstOperand,
	IN BIGNUM *pSecondOperand,
	OUT BIGNUM **pBI_Result)
{
	if (*pBI_Result == NULL)
		*pBI_Result = BN_new();
	BN_sub(*pBI_Result, pFirstOperand, pSecondOperand);
} /* End of Bignum_Sub */

VOID Bignum_Mod(
	IN BIGNUM *pFirstOperand,
	IN BIGNUM *pSecondOperand,
	OUT BIGNUM **pBI_Remainder)
{
	BN_CTX *bnctx;

	bnctx = BN_CTX_new();
	if (*pBI_Remainder == NULL)
		*pBI_Remainder = BN_new();

	BN_mod(*pBI_Remainder, pFirstOperand, pSecondOperand, bnctx);

	BN_CTX_free(bnctx);
}

VOID Bignum_Mod_Mul(
	IN BIGNUM *pFirstOperand,
	IN BIGNUM *pSecondOperand,
	IN BIGNUM *pBI_P,
	OUT BIGNUM **pBI_Result)
{
	BN_CTX *bnctx;

	bnctx = BN_CTX_new();
	if (*pBI_Result == NULL)
		*pBI_Result = BN_new();

	BN_mod_mul(*pBI_Result, pFirstOperand, pSecondOperand, pBI_P, bnctx);

	BN_CTX_free(bnctx);
}


UCHAR Bignum_is_zero(
	IN BIGNUM *pBI)
{
	return (UCHAR)BN_is_zero(pBI);
}

UCHAR Bignum_is_one(
	IN BIGNUM *pBI)
{
	return (UCHAR)BN_is_one(pBI);
}

UCHAR Bignum_is_odd(
	IN BIGNUM *pBI)
{
	return (UCHAR)BN_is_odd(pBI);
}


VOID Bignum_Mod_Square(
	IN BIGNUM *pFirstOperand,
	IN BIGNUM *pBI_P,
	OUT BIGNUM **pBI_Result)
{
	BN_CTX *bnctx;

	bnctx = BN_CTX_new();
	if (*pBI_Result == NULL)
		*pBI_Result = BN_new();

	BN_mod_sqr(*pBI_Result, pFirstOperand, pBI_P, bnctx);
	BN_CTX_free(bnctx);
}

VOID Bignum_Mod_Add(
	IN BIGNUM *pFirstOperand,
	IN BIGNUM *pSecondOperand,
	IN BIGNUM *pBI_P,
	OUT BIGNUM **pBI_Result)
{
	BN_CTX *bnctx;

	bnctx = BN_CTX_new();
	if (*pBI_Result == NULL)
		*pBI_Result = BN_new();

	BN_mod_add(*pBI_Result, pFirstOperand, pSecondOperand, pBI_P, bnctx);
	BN_CTX_free(bnctx);
}


VOID Bignum_Mod_Sub(
	IN BIGNUM *pFirstOperand,
	IN BIGNUM *pSecondOperand,
	IN BIGNUM *pBI_P,
	OUT BIGNUM **pBI_Result)
{
	BN_CTX *bnctx;

	bnctx = BN_CTX_new();
	if (*pBI_Result == NULL)
		*pBI_Result = BN_new();

	BN_mod_sub(*pBI_Result, pFirstOperand, pSecondOperand, pBI_P, bnctx);
	BN_CTX_free(bnctx);
}

VOID Bignum_Mod_Add_quick(
	IN BIGNUM *pFirstOperand,
	IN BIGNUM *pSecondOperand,
	IN BIGNUM *pBI_P,
	OUT BIGNUM **pBI_Result)
{
	if (*pBI_Result == NULL)
		*pBI_Result = BN_new();

	BN_mod_add_quick(*pBI_Result, pFirstOperand, pSecondOperand, pBI_P);
}


VOID Bignum_Mod_Sub_quick(
	IN BIGNUM *pFirstOperand,
	IN BIGNUM *pSecondOperand,
	IN BIGNUM *pBI_P,
	OUT BIGNUM **pBI_Result)
{
	if (*pBI_Result == NULL)
		*pBI_Result = BN_new();

	BN_mod_sub_quick(*pBI_Result, pFirstOperand, pSecondOperand, pBI_P);
}


VOID Bignum_Mod_Div(
	IN BIGNUM *pFirstOperand,
	IN BIGNUM *pSecondOperand,
	IN BIGNUM *pBI_P,
	OUT BIGNUM **pBI_Result)
{
	BIGNUM *tmp = NULL;
	BN_CTX *bnctx;

	bnctx = BN_CTX_new();

	tmp = BN_CTX_get(bnctx);

	BN_mod_inverse(tmp, pSecondOperand, pBI_P, bnctx);

	Bignum_Mod_Mul(pFirstOperand, tmp, pBI_P, pBI_Result);

	BN_CTX_free(bnctx);
}



/* Tonelli¡VShanks algorithm*/
/* reference: https://en.wikipedia.org/wiki/Tonelli%E2%80%93Shanks_algorithm */
/* pBI_P must be prime */
VOID Bignum_Mod_Sqrt(
	IN BIGNUM *pFirstOperand,
	IN BIGNUM *pBI_P,
	OUT BIGNUM **pBI_Result)
{
	BN_CTX *bnctx;

	bnctx = BN_CTX_new();
	if (*pBI_Result == NULL)
		*pBI_Result = BN_new();

	BN_mod_sqrt(*pBI_Result, pFirstOperand, pBI_P, bnctx);
	BN_CTX_free(bnctx);
}

VOID Bignum_Shift_Right1(
	IN BIGNUM *pBI,
	OUT BIGNUM **pBI_Result)
{
	BN_copy(*pBI_Result, pBI);
	BN_div_word(*pBI_Result, 2);
}

VOID Bignum_Mod_Shift_Left1(
	IN BIGNUM *pBI,
	IN BIGNUM *pBI_P,
	OUT BIGNUM **pBI_Result)
{
	BN_mod_lshift1_quick(*pBI_Result, pBI, pBI_P);
}

VOID Bignum_Mod_Shift_Left(
	IN BIGNUM *pBI,
	IN UCHAR bit,
	IN BIGNUM *pBI_P,
	OUT BIGNUM **pBI_Result)
{
	BN_mod_lshift_quick(*pBI_Result, pBI, bit, pBI_P);
}


/* an integer q is called a quadratic residue modulo n if it is congruent to a perfect square modulo n */
UCHAR Bignum_is_quadratic_residue(
	IN BIGNUM *q,
	IN BIGNUM *prime)
{
	BIGNUM *p = NULL;
	BIGNUM *res = NULL;
	UCHAR ret;
	BN_CTX *bnctx;

	bnctx = BN_CTX_new();
	res = BN_CTX_get(bnctx);

	Bignum_Copy(prime, &p);
	BN_sub_word(p, 1);
	Bignum_Shift_Right1(p, &p);
	BN_mod_exp_mont(res, q, p, prime, bnctx, NULL);
	ret = Bignum_is_one(res);

	BN_CTX_free(bnctx);
	BN_free(p);
	return ret;
}

VOID Bignum_Add_DW(
	INOUT BIGNUM *pBI,
	IN UINT32 value)
{
	return;
}

VOID Bignum_Mod_DW(
	INOUT BIGNUM *pBI,
	IN UINT32 value,
	OUT UINT32 *rem)
{
	*rem = pBI->d[0] & (value - 1);
	pBI->d[0] &= ~(value - 1);
}

VOID Bignum_Montgomery_ExpMod(
	IN BIGNUM *pBI_G,
	IN BIGNUM *pBI_E,
	IN BIGNUM *pBI_P,
	OUT BIGNUM **pBI_Result)
{
	BN_CTX *bnctx;

	bnctx = BN_CTX_new();

	if (*pBI_Result == NULL)
		*pBI_Result = BN_new();

	BN_mod_exp_mont(*pBI_Result, pBI_G, pBI_E, pBI_P, bnctx, NULL);

	BN_CTX_free(bnctx);
}

VOID Bignum_Mod_Mul_Inverse(
	IN BIGNUM *pBI,
	IN BIGNUM *pBI_P,
	OUT BIGNUM **pBI_Result)
{
	BN_CTX *bnctx;

	bnctx = BN_CTX_new();

	if (*pBI_Result == NULL)
		*pBI_Result = BN_new();

	BN_mod_inverse(*pBI_Result, pBI, pBI_P, bnctx);

	BN_CTX_free(bnctx);
}



/* End of crypt_Bignum.c */

#endif /* DOT11_SAE_OPENSSL_BN */

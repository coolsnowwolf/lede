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
    Module Name:
    Bignum


    Revision History:
    Who         When            What
    --------    ----------      ------------------------------------------
    Eddy        2009/01/12      Create
***************************************************************************/

#ifndef __CRYPT_BIGNUM_H__
#define __CRYPT_BIGNUM_H__


#include "rtmp_type.h"
#include "bn.h"

#ifndef IN
#define IN
#endif
#ifndef OUT
#define OUT
#endif
#ifndef INOUT
#define INOUT
#endif


/* Bignum operations */
VOID Bignum_Print (
	IN BIGNUM *pBI);


VOID Bignum_Init(
	INOUT BIGNUM **pBI);

VOID Bignum_Free(
	IN BIGNUM **pBI);

UINT32 Bignum_getlen(
	IN BIGNUM *pBI);

INT Bignum_Get_rand_range(IN BIGNUM * range, INOUT BIGNUM * r);

VOID Bignum_BI2Bin(
	IN BIGNUM *pBI,
	OUT UINT8 *pValue,
	OUT UINT *Length);

VOID Bignum_BI2Bin_with_pad(
	IN BIGNUM *pBI,
	OUT UINT8 *pValue,
	OUT UINT *Length,
	IN UINT32 PadLen);

VOID Bignum_Bin2BI(
	IN UINT8 *pValue,
	IN UINT Length,
	OUT BIGNUM **pBI);

VOID Bignum_Copy(
	IN BIGNUM *pBI_Copied,
	OUT BIGNUM **pBI_Result);

INT Bignum_UnsignedCompare(
	IN BIGNUM *pFirstOperand,
	IN BIGNUM *pSecondOperand);

VOID Bignum_Add(
	IN BIGNUM *pFirstOperand,
	IN BIGNUM *pSecondOperand,
	OUT BIGNUM **pBI_Result);

VOID Bignum_Sub(
	IN BIGNUM *pFirstOperand,
	IN BIGNUM *pSecondOperand,
	OUT BIGNUM **pBI_Result);

VOID Bignum_Mod(
	IN BIGNUM *pFirstOperand,
	IN BIGNUM *pSecondOperand,
	OUT BIGNUM **pBI_Result);

VOID Bignum_Mod_Mul(
	IN BIGNUM *pFirstOperand,
	IN BIGNUM *pSecondOperand,
	IN BIGNUM *pBI_P,
	OUT BIGNUM **pBI_Result);


VOID Bignum_Montgomery_ExpMod(
	IN BIGNUM *pBI_G,
	IN BIGNUM *pBI_E,
	IN BIGNUM *pBI_P,
	OUT BIGNUM **pBI_Result);

UCHAR Bignum_is_zero(
	IN BIGNUM *pBI);

UCHAR Bignum_is_one(
	IN BIGNUM *pBI);

UCHAR Bignum_is_odd(
	IN BIGNUM *pBI);

VOID Bignum_Mod_Square(
	IN BIGNUM *pFirstOperand,
	IN BIGNUM *pBI_P,
	OUT BIGNUM **pBI_Result);


VOID Bignum_Mod_Add(
	IN BIGNUM *pFirstOperand,
	IN BIGNUM *pSecondOperand,
	IN BIGNUM *pBI_P,
	OUT BIGNUM **pBI_Result);

VOID Bignum_Mod_Sub(
	IN BIGNUM *pFirstOperand,
	IN BIGNUM *pSecondOperand,
	IN BIGNUM *pBI_P,
	OUT BIGNUM **pBI_Result);


VOID Bignum_Mod_Add_quick(
	IN BIGNUM *pFirstOperand,
	IN BIGNUM *pSecondOperand,
	IN BIGNUM *pBI_P,
	OUT BIGNUM **pBI_Result);

VOID Bignum_Mod_Sub_quick(
	IN BIGNUM *pFirstOperand,
	IN BIGNUM *pSecondOperand,
	IN BIGNUM *pBI_P,
	OUT BIGNUM **pBI_Result);


VOID Bignum_Mod_Div(
	IN BIGNUM *pFirstOperand,
	IN BIGNUM *pSecondOperand,
	IN BIGNUM *pBI_P,
	OUT BIGNUM **pBI_Result);


VOID Bignum_Mod_Sqrt(
	IN BIGNUM *pFirstOperand,
	IN BIGNUM *pBI_P,
	OUT BIGNUM **pBI_Result);


VOID Bignum_Mod_Mul_Inverse(
	IN BIGNUM *pBI,
	IN BIGNUM *pBI_P,
	OUT BIGNUM **pBI_Result);


UCHAR Bignum_Sqrt(
	IN BIGNUM *pBI,
	OUT BIGNUM **pBI_Result);

VOID Bignum_Shift_Right1(
	IN BIGNUM *pBI,
	OUT BIGNUM **pBI_Result);

VOID Bignum_Mod_Shift_Left1(
	IN BIGNUM *pBI,
	IN BIGNUM *pBI_P,
	OUT BIGNUM **pBI_Result);

VOID Bignum_Mod_Shift_Left(
	IN BIGNUM *pBI,
	IN UCHAR bit,
	IN BIGNUM *pBI_P,
	OUT BIGNUM **pBI_Result);

VOID Bignum_Shift_Left(
	IN BIGNUM *pBI,
	IN UINT32 shift_bit,
	OUT BIGNUM **pBI_Result);


/* treat pBi is positve and non-zero */
VOID Bignum_minus_one(
	INOUT BIGNUM *pBI);


/* treat pBi is positve and non-zero */
VOID Bignum_plus_one(
	INOUT BIGNUM *pBI);


/* an integer q is called a quadratic residue modulo n if it is congruent to a perfect square modulo n */
UCHAR Bignum_is_quadratic_residue(
	IN BIGNUM *q,
	IN BIGNUM *prime);

VOID Bignum_DWtoBI(
	IN UINT32 value,
	OUT BIGNUM **pBI);

VOID Bignum_Add_DW(
	INOUT BIGNUM *pBI,
	IN UINT32 value);

VOID Bignum_Mod_DW(
	INOUT BIGNUM *pBI,
	IN UINT32 value,
	OUT UINT32 *rem);


VOID Bignum_DtoH(
	INOUT BIGNUM *pBI);


#endif /* __CRYPT_BIGNUM_H__ */


/****************************************************************************
 * Ralink Tech Inc.
 * Taiwan, R.O.C.
 *
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************/

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
#include "security/bn.h"

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


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
    BigInteger

    
    Revision History:
    Who         When            What
    --------    ----------      ------------------------------------------
    Eddy        2009/01/12      Create
***************************************************************************/

#ifndef __CRYPT_BIGINTEGER_H__
#define __CRYPT_BIGINTEGER_H__

#include "rt_config.h"


/* BigInteger definition & structure */
#define SLIDING_WINDOW 16
typedef struct _BIG_INTEGER_STRUC
{
	STRING Name[10];
	UINT32 *pIntegerArray;
	UINT AllocSize;
	UINT ArrayLength;
	UINT IntegerLength;
	INT  Signed;
} BIG_INTEGER, *PBIG_INTEGER;


/* BigInteger operations */
VOID BigInteger_Print (
    IN PBIG_INTEGER pBI);
    
VOID BigInteger_Init (
    INOUT PBIG_INTEGER *pBI);

VOID BigInteger_Free_AllocSize (
    IN PBIG_INTEGER *pBI);

VOID BigInteger_Free (
    IN PBIG_INTEGER *pBI);

VOID BigInteger_AllocSize (
    IN PBIG_INTEGER *pBI,
    IN INT Length);

VOID BigInteger_ClearHighBits (
    IN PBIG_INTEGER pBI);

VOID BigInteger_BI2Bin (
    IN PBIG_INTEGER pBI, 
    OUT UINT8 *pValue,
    OUT UINT *Length);

VOID BigInteger_Bin2BI (
    IN UINT8 *pValue,
    IN UINT Length,
    OUT PBIG_INTEGER *pBI);

VOID BigInteger_BitsOfBI (
    IN PBIG_INTEGER pBI,
    OUT UINT *Bits_Of_P);

INT BigInteger_GetBitValue (
    IN PBIG_INTEGER pBI,
    IN UINT Index);

UINT8 BigInteger_GetByteValue (
    IN PBIG_INTEGER pBI,
    IN UINT Index);

VOID BigInteger_Copy (
    IN PBIG_INTEGER pBI_Copied,
    OUT PBIG_INTEGER *pBI_Result);

INT BigInteger_UnsignedCompare (
    IN PBIG_INTEGER pFirstOperand,
    IN PBIG_INTEGER pSecondOperand);

VOID BigInteger_Add (
    IN PBIG_INTEGER pFirstOperand,
    IN PBIG_INTEGER pSecondOperand, 
    OUT PBIG_INTEGER *pBI_Result);

VOID BigInteger_Sub (
    IN PBIG_INTEGER pFirstOperand, 
    IN PBIG_INTEGER pSecondOperand, 
    OUT PBIG_INTEGER *pBI_Result);

VOID BigInteger_Mul (
    IN PBIG_INTEGER pFirstOperand, 
    IN PBIG_INTEGER pSecondOperand, 
    OUT PBIG_INTEGER *pBI_Result);

VOID BigInteger_Square (
    IN PBIG_INTEGER pBI, 
    OUT PBIG_INTEGER *pBI_Result);
    
VOID BigInteger_Div (
    IN PBIG_INTEGER pFirstOperand, 
    IN PBIG_INTEGER pSecondOperand, 
    OUT PBIG_INTEGER *pBI_Result,
    OUT PBIG_INTEGER *pBI_Remainder);

VOID BigInteger_Montgomery_Reduction (
    IN PBIG_INTEGER pBI_A,
    IN PBIG_INTEGER pBI_P,
    IN PBIG_INTEGER pBI_R,
    OUT PBIG_INTEGER *pBI_Result);

VOID BigInteger_Montgomery_ExpMod (
    IN PBIG_INTEGER pBI_G,
    IN PBIG_INTEGER pBI_E,
    IN PBIG_INTEGER pBI_P,
    OUT PBIG_INTEGER *pBI_Result);


#endif /* __CRYPT_BIGINTEGER_H__ */


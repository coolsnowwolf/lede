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

#if defined(DOT11_SAE_SUPPORT) || defined(CONFIG_OWE_SUPPORT)
#include "rtmp_type.h"
#ifndef IN
#define IN
#endif
#ifndef OUT
#define OUT
#endif
#ifndef INOUT
#define INOUT
#endif

typedef struct _BIG_INTEGER_STRUC BIG_INTEGER, *PBIG_INTEGER;
typedef struct _BI_OP_TIME_INTERVAL BI_OP_TIME_INTERVAL;
typedef struct _MONT_STRUC  MONT_STRUC, *PMONT_STRUC;



/* BigInteger definition & structure */
#define SLIDING_WINDOW 16
#ifndef DOT11_SAE_OPENSSL_BN
/*#define BI_POOL*/
#endif
/*#define BI_TIME_REC */
/*#define BI_POOL_DBG*/

#ifdef BI_POOL_DBG
#define POOL_COUNTER_CHECK_BEGIN(_expect_cnt) {_expect_cnt = sae_dump_pool_info_check(0, FALSE, FALSE); }
#define POOL_COUNTER_CHECK_END(_expect_cnt) {_expect_cnt = sae_dump_pool_info_check(_expect_cnt, TRUE, FALSE); }
#else
#define POOL_COUNTER_CHECK_BEGIN(_expect_cnt)
#define POOL_COUNTER_CHECK_END(_expect_cnt)
#endif

#ifdef BI_POOL
#define GET_BI_INS_FROM_POOL(_ptr) {_ptr = get_temporal_usage_big_interger(); }
#else
#define GET_BI_INS_FROM_POOL(_ptr)
#endif


struct _BIG_INTEGER_STRUC {
	RTMP_STRING Name[10];
	UINT32 *pIntegerArray;
	UINT AllocSize;
	UINT ArrayLength;
	UINT IntegerLength;
	INT  Signed;
	UCHAR invalid;
};



struct _BI_OP_TIME_INTERVAL {
	ULONG avg_time_interval;
	ULONG time_interval;
	UINT32 exe_times;
};

struct _MONT_STRUC {
	UINT16 Bits_Of_R;
	BIG_INTEGER *pBI_X;
	BIG_INTEGER *pBI_R;
	BIG_INTEGER *pBI_PInverse;
};


typedef struct _BI_OP_TIME_RECORD {
	BI_OP_TIME_INTERVAL add_op;
	BI_OP_TIME_INTERVAL sub_op;
	BI_OP_TIME_INTERVAL mul_op;
	BI_OP_TIME_INTERVAL div_op;
	BI_OP_TIME_INTERVAL mod_op;
	BI_OP_TIME_INTERVAL square_op;
	BI_OP_TIME_INTERVAL sqrt_op;
	BI_OP_TIME_INTERVAL exp_mod_op;
	BI_OP_TIME_INTERVAL div_mod_op;
	BI_OP_TIME_INTERVAL sqrt_mod_op;
	BI_OP_TIME_INTERVAL mod_mul_inv_op;
	BI_OP_TIME_INTERVAL simple_exp_mod_op;
} BI_OP_TIME_RECORD, *PBI_OP_TIME_RECORD;

typedef struct _EC_POINT_OP_TIME_RECORD {
	BI_OP_TIME_INTERVAL add_op;
	BI_OP_TIME_INTERVAL dbl_op;
	BI_OP_TIME_INTERVAL mul_op;
	BI_OP_TIME_INTERVAL find_y_op;
	BI_OP_TIME_INTERVAL on_curve_check_op;
} EC_POINT_OP_TIME_RECORD, *PEC_POINT_OP_TIME_RECORD;


#ifdef BI_POOL
VOID big_integer_pool_init(
	VOID);

VOID big_integer_pool_deinit(
	VOID);


BIG_INTEGER *get_temporal_usage_big_interger(
	VOID);


UINT32 sae_dump_pool_info_check(
	INT32 expect_cnt,
	UCHAR is_check,
	UCHAR is_print);
#endif

VOID release_temporal_usage_big_interger(
	IN BIG_INTEGER **pBI);


VOID BigInteger_record_time_begin(
	BI_OP_TIME_INTERVAL *time_rec);

VOID BigInteger_record_time_end(
	BI_OP_TIME_INTERVAL *time_rec);


/* BigInteger operations */
VOID BigInteger_Print (
	IN PBIG_INTEGER pBI);

VOID BigInteger_Init(
	INOUT PBIG_INTEGER * pBI);

VOID BigInteger_Free_AllocSize(
	IN PBIG_INTEGER * pBI);

VOID BigInteger_Free(
	IN PBIG_INTEGER * pBI);

VOID BigInteger_AllocSize(
	IN PBIG_INTEGER * pBI,
	IN INT Length);

VOID BigInteger_ClearHighBits(
	IN PBIG_INTEGER pBI);

UINT32 BigInteger_getlen(
	IN BIG_INTEGER * pBI);


VOID BigInteger_BI2Bin(
	IN PBIG_INTEGER pBI,
	OUT UINT8 * pValue,
	OUT UINT *Length);

VOID BigInteger_BI2Bin_with_pad(
	IN PBIG_INTEGER pBI,
	OUT UINT8 * pValue,
	OUT UINT *Length,
	IN UINT32 PadLen);

VOID BigInteger_Bin2BI(
	IN UINT8 * pValue,
	IN UINT Length,
	OUT PBIG_INTEGER * pBI);

VOID BigInteger_BitsOfBI(
	IN PBIG_INTEGER pBI,
	OUT UINT *Bits_Of_P);

INT BigInteger_GetBitValue(
	IN PBIG_INTEGER pBI,
	IN UINT Index);

UINT8 BigInteger_GetByteValue(
	IN PBIG_INTEGER pBI,
	IN UINT Index);

VOID BigInteger_Copy(
	IN PBIG_INTEGER pBI_Copied,
	OUT PBIG_INTEGER * pBI_Result);

INT BigInteger_UnsignedCompare(
	IN PBIG_INTEGER pFirstOperand,
	IN PBIG_INTEGER pSecondOperand);

VOID BigInteger_Add(
	IN PBIG_INTEGER pFirstOperand,
	IN PBIG_INTEGER pSecondOperand,
	OUT PBIG_INTEGER * pBI_Result);

VOID BigInteger_Sub(
	IN PBIG_INTEGER pFirstOperand,
	IN PBIG_INTEGER pSecondOperand,
	OUT PBIG_INTEGER * pBI_Result);

VOID BigInteger_Mul(
	IN PBIG_INTEGER pFirstOperand,
	IN PBIG_INTEGER pSecondOperand,
	OUT PBIG_INTEGER * pBI_Result);

VOID BigInteger_Square(
	IN PBIG_INTEGER pBI,
	OUT PBIG_INTEGER * pBI_Result);

VOID BigInteger_Div(
	IN PBIG_INTEGER pFirstOperand,
	IN PBIG_INTEGER pSecondOperand,
	OUT PBIG_INTEGER * pBI_Result,
	OUT PBIG_INTEGER * pBI_Remainder);


VOID BigInteger_Montgomery_MulMod(
	IN PBIG_INTEGER pBI_A,
	IN PBIG_INTEGER pBI_B,
	IN PBIG_INTEGER pBI_P,
	IN PBIG_INTEGER pBI_PInverse,
	IN UINT Bits_Of_R,
	OUT PBIG_INTEGER * pBI_Result);


VOID BigInteger_Montgomery_ExpMod(
	IN PBIG_INTEGER pBI_G,
	IN PBIG_INTEGER pBI_E,
	IN PBIG_INTEGER pBI_P,
	OUT PBIG_INTEGER * pBI_Result);

VOID BigInteger_Montgomery_MulMod_with_mont(
	IN PBIG_INTEGER pBI_A,
	IN PBIG_INTEGER pBI_B,
	IN PBIG_INTEGER pBI_P,
	IN MONT_STRUC * mont,
	OUT PBIG_INTEGER * pBI_Result);


VOID BigInteger_Montgomery_ExpMod_with_mont(
	IN PBIG_INTEGER pBI_G,
	IN PBIG_INTEGER pBI_E,
	IN PBIG_INTEGER pBI_P,
	IN MONT_STRUC * mont,
	OUT PBIG_INTEGER * pBI_Result);

UCHAR BigInteger_is_zero(IN PBIG_INTEGER pBI);

UCHAR BigInteger_is_one(IN PBIG_INTEGER pBI);

VOID BigInteger_Mod(
	IN PBIG_INTEGER pFirstOperand,
	IN PBIG_INTEGER pSecondOperand,
	OUT PBIG_INTEGER *pBI_Result);

VOID BigInteger_Mod_Mul(
	IN PBIG_INTEGER pFirstOperand,
	IN PBIG_INTEGER pSecondOperand,
	IN PBIG_INTEGER pBI_P,
	OUT PBIG_INTEGER *pBI_Result);

UCHAR BigInteger_is_odd(
	IN PBIG_INTEGER pBI);

VOID BigInteger_Mod_Square(
	IN PBIG_INTEGER pFirstOperand,
	IN PBIG_INTEGER pBI_P,
	OUT PBIG_INTEGER * pBI_Result);


VOID BigInteger_Mod_Add(
	IN PBIG_INTEGER pFirstOperand,
	IN PBIG_INTEGER pSecondOperand,
	IN PBIG_INTEGER pBI_P,
	OUT PBIG_INTEGER * pBI_Result);

VOID BigInteger_Mod_Sub(
	IN PBIG_INTEGER pFirstOperand,
	IN PBIG_INTEGER pSecondOperand,
	IN PBIG_INTEGER pBI_P,
	OUT PBIG_INTEGER * pBI_Result);


VOID BigInteger_Mod_Add_quick(
	IN PBIG_INTEGER pFirstOperand,
	IN PBIG_INTEGER pSecondOperand,
	IN PBIG_INTEGER pBI_P,
	OUT PBIG_INTEGER * pBI_Result);

VOID BigInteger_Mod_Sub_quick(
	IN PBIG_INTEGER pFirstOperand,
	IN PBIG_INTEGER pSecondOperand,
	IN PBIG_INTEGER pBI_P,
	OUT PBIG_INTEGER * pBI_Result);


VOID BigInteger_Mod_Div(
	IN PBIG_INTEGER pFirstOperand,
	IN PBIG_INTEGER pSecondOperand,
	IN PBIG_INTEGER pBI_P,
	OUT PBIG_INTEGER * pBI_Result);


VOID BigInteger_Mod_Sqrt(
	IN PBIG_INTEGER pFirstOperand,
	IN PBIG_INTEGER pBI_P,
	IN MONT_STRUC * mont,
	OUT PBIG_INTEGER * pBI_Result);


VOID BigInteger_Euclidean_Div(
	IN PBIG_INTEGER pFirstOperand,
	IN PBIG_INTEGER pSecondOperand,
	IN PBIG_INTEGER prev_u,
	IN PBIG_INTEGER prev_v,
	INOUT PBIG_INTEGER curr_u,
	INOUT PBIG_INTEGER curr_v);


VOID BigInteger_Mod_Mul_Inverse(
	IN PBIG_INTEGER pBI,
	IN PBIG_INTEGER pBI_P,
	OUT PBIG_INTEGER * pBI_Result);

VOID BigInteger_Mod_Mul_Inverse2(
	IN PBIG_INTEGER pBI,
	IN PBIG_INTEGER pBI_P,
	OUT PBIG_INTEGER * pBI_Result);


UCHAR BigInteger_Sqrt(
	IN PBIG_INTEGER pBI,
	OUT PBIG_INTEGER * pBI_Result);

VOID BigInteger_Shift_Right1(
	IN PBIG_INTEGER pBI,
	OUT PBIG_INTEGER * pBI_Result);

VOID BigInteger_Mod_Shift_Left(
	IN PBIG_INTEGER pBI,
	IN UCHAR bit,
	IN PBIG_INTEGER pBI_P,
	OUT PBIG_INTEGER * pBI_Result);


VOID BigInteger_Shift_Left(
	IN PBIG_INTEGER pBI,
	IN UINT32 shift_bit,
	OUT PBIG_INTEGER * pBI_Result);


/* treat pBi is positve and non-zero */
VOID BigInteger_minus_one(
	INOUT PBIG_INTEGER pBI);


/* treat pBi is positve and non-zero */
VOID BigInteger_plus_one(
	INOUT PBIG_INTEGER pBI);


/* an integer q is called a quadratic residue modulo n if it is congruent to a perfect square modulo n */
UCHAR BigInteger_is_quadratic_residue(
	IN BIG_INTEGER *q,
	IN BIG_INTEGER *prime,
	IN MONT_STRUC * mont);

VOID BigInteger_Montgomery_ExpMod32(
	IN PBIG_INTEGER pBI_G,
	IN PBIG_INTEGER pBI_E,
	IN PBIG_INTEGER pBI_P,
	OUT PBIG_INTEGER * pBI_Result);


VOID BigInteger_DWtoBI(
	IN UINT32 value,
	OUT PBIG_INTEGER * pBI);

VOID BigInteger_Add_DW(
	INOUT PBIG_INTEGER pBI,
	IN UINT32 value);

VOID BigInteger_Mod_DW(
	INOUT PBIG_INTEGER pBI,
	IN UINT32 value,
	OUT UINT32 *rem);


VOID BigInteger_DtoH(
	INOUT PBIG_INTEGER pBI);

VOID BigInteger_dump_time(
	VOID);

INT BigInteger_rand_range(IN BIG_INTEGER * range, INOUT BIG_INTEGER * r);

#else

#include "rt_config.h"


/* BigInteger definition & structure */
#define SLIDING_WINDOW 16
typedef struct _BIG_INTEGER_STRUC
{
	RTMP_STRING Name[10];
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


#endif /* defined(DOT11_SAE_SUPPORT) || defined(CONFIG_OWE_SUPPORT) */


#endif /* __CRYPT_BIGINTEGER_H__ */

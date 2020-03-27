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
#include "security/crypt_biginteger.h"
#include "rt_config.h"
#include <linux/time.h>



#ifdef __KERNEL__
#define DEBUGPRINT(fmt, args...) printk(fmt, ## args)
#else
#define DEBUGPRINT(fmt, args...) printf(fmt, ## args)
#endif /* __KERNEL__ */

#define UINT32_HBITS(value)	(((value) >> 0x10) & 0xffff)
#define UINT32_LBITS(value)	((value) & 0xffff)
#define UINT32_GETBYTE(value, index)	(((value) >> ((index)*8)) & 0xff)
#define UINT64_HBITS(value)	(((value) >> 0x20) & 0xffffffff)
#define UINT64_LBITS(value)	((value) & 0xffffffff)
#ifdef WSC_INCLUDED
static UINT8 WPS_DH_R_VALUE[193] = {
	0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00,
};

static UINT8 WPS_DH_X_VALUE[184] = {
	0x36, 0xf0, 0x25, 0x5d, 0xde, 0x97, 0x3d, 0xcb,
	0x3b, 0x39, 0x9d, 0x74, 0x7f, 0x23, 0xe3, 0x2e,
	0xd6, 0xfd, 0xb1, 0xf7, 0x75, 0x98, 0x33, 0x8b,
	0xfd, 0xf4, 0x41, 0x59, 0xc4, 0xec, 0x64, 0xdd,
	0xae, 0xb5, 0xf7, 0x86, 0x71, 0xcb, 0xfb, 0x22,
	0x10, 0x6a, 0xe6, 0x4c, 0x32, 0xc5, 0xbc, 0xe4,
	0xcf, 0xd4, 0xf5, 0x92, 0x0d, 0xa0, 0xeb, 0xc8,
	0xb0, 0x1e, 0xca, 0x92, 0x92, 0xae, 0x3d, 0xba,
	0x1b, 0x7a, 0x4a, 0x89, 0x9d, 0xa1, 0x81, 0x39,
	0x0b, 0xb3, 0xbd, 0x16, 0x59, 0xc8, 0x12, 0x94,
	0xf4, 0x00, 0xa3, 0x49, 0x0b, 0xf9, 0x48, 0x12,
	0x11, 0xc7, 0x94, 0x04, 0xa5, 0x76, 0x60, 0x5a,
	0x51, 0x60, 0xdb, 0xee, 0x83, 0xb4, 0xe0, 0x19,
	0xb6, 0xd7, 0x99, 0xae, 0x13, 0x1b, 0xa4, 0xc2,
	0x3d, 0xff, 0x83, 0x47, 0x5e, 0x9c, 0x40, 0xfa,
	0x67, 0x25, 0xb7, 0xc9, 0xe3, 0xaa, 0x2c, 0x65,
	0x96, 0xe9, 0xc0, 0x57, 0x02, 0xdb, 0x30, 0xa0,
	0x7c, 0x9a, 0xa2, 0xdc, 0x23, 0x5c, 0x52, 0x69,
	0xe3, 0x9d, 0x0c, 0xa9, 0xdf, 0x7a, 0xad, 0x44,
	0x61, 0x2a, 0xd6, 0xf8, 0x8f, 0x69, 0x69, 0x92,
	0x98, 0xf3, 0xca, 0xb1, 0xb5, 0x43, 0x67, 0xfb,
	0x0e, 0x8b, 0x93, 0xf7, 0x35, 0xdc, 0x8c, 0xd8,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
};
#endif

static UINT8 Value_0[1] = {0x00};
static UINT8 Value_1[1] = {0x01};
static PBIG_INTEGER pBI_S = NULL, pBI_O;

BI_OP_TIME_RECORD bi_op_ti_rec;
UCHAR is_time_rec = FALSE;
ULONG alloc_cnt = 0;
UCHAR is_add_alloc_rec = FALSE;
UCHAR is_sub_alloc_rec = FALSE;
UCHAR is_mul_alloc_rec = FALSE;
UCHAR is_div_alloc_rec = FALSE;
UCHAR is_mod_alloc_rec = FALSE;
ULONG alloc_cnt_add = 0;
ULONG alloc_cnt_sub = 0;
ULONG alloc_cnt_mul = 0;
ULONG alloc_cnt_div = 0;
ULONG alloc_cnt_mod = 0;
#ifdef BI_POOL_DBG
UINT32 expected_cnt[25]; /* 0~21 is used */
#endif

#ifdef BI_POOL
#define TEMP_USAGE_POOL_SIZE 500
BIG_INTEGER *temp_usage_pool[TEMP_USAGE_POOL_SIZE] = {NULL};
static UINT16 pool_cur_index;
static UCHAR pool_is_small = FALSE;
#endif
NDIS_SPIN_LOCK bi_pool_lock;


#ifdef BI_POOL
VOID big_integer_pool_init(
	VOID)
{
	UINT16 i = 0;
	NdisAcquireSpinLock(&bi_pool_lock);

	for (i = 0; i < TEMP_USAGE_POOL_SIZE; i++) {
		BigInteger_Init(&temp_usage_pool[i]);
		BigInteger_AllocSize(&temp_usage_pool[i], 300);
	}

	NdisReleaseSpinLock(&bi_pool_lock);
}

VOID big_integer_pool_deinit(
	VOID)
{
	UINT16 i = 0;
	NdisAcquireSpinLock(&bi_pool_lock);

	for (i = 0; i < TEMP_USAGE_POOL_SIZE; i++)
		BigInteger_Free(&temp_usage_pool[i]);

	NdisReleaseSpinLock(&bi_pool_lock);
}

BIG_INTEGER *get_temporal_usage_big_interger(
	VOID)
{
	UINT16 i = 0;
	NdisAcquireSpinLock(&bi_pool_lock);

	for (i = 0; i < TEMP_USAGE_POOL_SIZE; i++) {
		BIG_INTEGER *tmp = temp_usage_pool[(i + pool_cur_index) % TEMP_USAGE_POOL_SIZE];

		if (tmp->invalid == FALSE) {
			tmp->invalid = TRUE;
			tmp->Signed = 1;
			pool_cur_index = (i + pool_cur_index + 1) % TEMP_USAGE_POOL_SIZE;
			NdisReleaseSpinLock(&bi_pool_lock);
			return tmp;
		}
	}

	if (pool_is_small == FALSE)
		MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("\x1b[35m%s :pool is too small\x1b[m\n", __func__));

	pool_is_small = TRUE;
	NdisReleaseSpinLock(&bi_pool_lock);
	return NULL;
}

UINT32 sae_dump_pool_info_check(
	INT32 expect_cnt,
	UCHAR is_check,
	UCHAR is_print)
{
	UINT32 cnt = 0;
	UINT i = 0;

	for (i = 0; i < TEMP_USAGE_POOL_SIZE; i++)
		if (temp_usage_pool[i]->invalid)
			cnt++;

	if (is_check
		&& expect_cnt != cnt)
		MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("\x1b[31mError: invalid pool cnt = %d, Expected Cnt = %d, caller = %pS\x1b[m\n",
				  cnt, expect_cnt, OS_TRACE));

	if (is_print && cnt != 0)
		MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\x1b[31m"));

	if (is_print)
		MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("invalid pool cnt = %d, pool idx = %d, pool_is_small = %d\x1b[m\n", cnt, pool_cur_index, pool_is_small));

	return cnt;
}
#endif

VOID release_temporal_usage_big_interger(
	IN BIG_INTEGER **pBI)
{
	if (*pBI && (*pBI)->invalid == TRUE) {
		NdisAcquireSpinLock(&bi_pool_lock);
		(*pBI)->invalid = FALSE;
		*pBI = NULL;
		NdisReleaseSpinLock(&bi_pool_lock);
	} else {
		/*MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s :pool is too small\n", __func__));*/
		BigInteger_Free(pBI);
	}
}


static inline ULONG show_float(
	ULONG a,
	ULONG b,
	INT32 x)
{
	ULONG c = a / b;
	UCHAR i = 0;
	ULONG d = 1;
	ULONG e = 0;

	for (i = 0; i < x; i++)
		d *= 10;

	e = d * a / b;
	return e - c * d;
}

VOID BigInteger_record_time_begin(
	BI_OP_TIME_INTERVAL * time_rec)
{
	NdisGetSystemUpTime(&time_rec->time_interval);
}

VOID BigInteger_record_time_end(
	BI_OP_TIME_INTERVAL * time_rec)
{
	ULONG temp;
	NdisGetSystemUpTime(&temp);
	time_rec->time_interval = temp - time_rec->time_interval;

	if (time_rec->exe_times + 1 == 0)
		return;

	time_rec->avg_time_interval = ((time_rec->avg_time_interval * time_rec->exe_times)
								   + time_rec->time_interval) / (time_rec->exe_times + 1);
	time_rec->exe_times++;
}

VOID BigInteger_record_time_ns(
	BI_OP_TIME_INTERVAL * time_rec,
	ULONG time_interval)
{
	time_rec->time_interval += time_interval;

	if (time_rec->exe_times + 1 == 0)
		return;

	time_rec->avg_time_interval = time_rec->time_interval / (time_rec->exe_times + 1);
	time_rec->exe_times++;
}



VOID BigInteger_Print (
	IN PBIG_INTEGER pBI)
{
	int i = 0, j = 0;

	if ((pBI == NULL) || (pBI->pIntegerArray == NULL))
		return;

	if (strlen(pBI->Name) != 0)
		DEBUGPRINT("Name=%s\n", pBI->Name);

	DEBUGPRINT("AllocSize=%d, ArrayLength=%d, IntegerLength=%d, Signed=%d\n", pBI->AllocSize, pBI->ArrayLength, pBI->IntegerLength, pBI->Signed);

	for (i = (pBI->ArrayLength - 1), j = 0; i >= 0; i--, j++) {
		DEBUGPRINT("%08x, ", pBI->pIntegerArray[i]);

		if ((j % 8) == 7)
			DEBUGPRINT("\n");
	}

	DEBUGPRINT("\n\n");
}


VOID BigInteger_Init(
	INOUT PBIG_INTEGER *pBI)
{
	if (*pBI != NULL) {
		if ((*pBI)->invalid == FALSE)
			BigInteger_Free_AllocSize(pBI);

		return;
	} else {
		if (is_time_rec)
			alloc_cnt++;

		if (is_add_alloc_rec)
			alloc_cnt_add++;

		if (is_sub_alloc_rec)
			alloc_cnt_sub++;

		if (is_mul_alloc_rec)
			alloc_cnt_mul++;

		if (is_div_alloc_rec)
			alloc_cnt_div++;

		if (is_mod_alloc_rec)
			alloc_cnt_mod++;

		os_alloc_mem(NULL, (UCHAR **)pBI, sizeof(BIG_INTEGER));
	}

	if (*pBI == NULL) {
		DEBUGPRINT("BigInteger_Init: allocate %zu bytes memory failure.\n", sizeof(BIG_INTEGER));
		return;
	}

	NdisZeroMemory(*pBI, sizeof(BIG_INTEGER));
	(*pBI)->pIntegerArray = NULL;
	(*pBI)->Signed = 1;
}


VOID BigInteger_Free_AllocSize(
	IN PBIG_INTEGER *pBI)
{
	if ((*pBI != NULL) && ((*pBI)->pIntegerArray != NULL)) {
		UCHAR invalid = (*pBI)->invalid;
		os_free_mem((*pBI)->pIntegerArray);
		NdisZeroMemory(*pBI, sizeof(BIG_INTEGER));
		(*pBI)->pIntegerArray = NULL;
		(*pBI)->Signed = 1;
		(*pBI)->invalid = invalid;
	}
}


VOID BigInteger_Free(
	IN PBIG_INTEGER *pBI)
{
	if (*pBI != NULL) {
		BigInteger_Free_AllocSize(pBI);
		os_free_mem(*pBI);
	}

	*pBI = NULL;
}


VOID BigInteger_AllocSize(
	IN PBIG_INTEGER *pBI,
	IN INT Length)
{
	UINT ArrayLength = 0;

	if (Length <= 0)
		return;

	if (*pBI == NULL)
		BigInteger_Init(pBI);

	/* Caculate array size */
	ArrayLength = Length >> 0x2;

	if ((Length & 0x3) != 0)
		ArrayLength++;

	if (((*pBI)->pIntegerArray != NULL) && ((*pBI)->AllocSize < (sizeof(UINT32)*ArrayLength)))
		BigInteger_Free_AllocSize(pBI);

	if ((*pBI)->pIntegerArray == NULL) {
		if (is_time_rec)
			alloc_cnt++;

		if (is_add_alloc_rec)
			alloc_cnt_add++;

		if (is_sub_alloc_rec)
			alloc_cnt_sub++;

		if (is_mul_alloc_rec)
			alloc_cnt_mul++;

		if (is_div_alloc_rec)
			alloc_cnt_div++;

		if (is_mod_alloc_rec)
			alloc_cnt_mod++;

		os_alloc_mem(NULL, (UCHAR **) &((*pBI)->pIntegerArray), sizeof(UINT32)*ArrayLength);

		if ((*pBI)->pIntegerArray == NULL) {
			DEBUGPRINT("BigInteger_AllocSize: allocate %zu bytes memory failure.\n", sizeof(UINT32)*ArrayLength);
			return;
		}

		(*pBI)->AllocSize = sizeof(UINT32) * ArrayLength;
	}

	NdisZeroMemory((*pBI)->pIntegerArray, (*pBI)->AllocSize);
	(*pBI)->ArrayLength = ArrayLength;
	(*pBI)->IntegerLength = Length;
}


VOID BigInteger_ClearHighBits(
	IN PBIG_INTEGER pBI)
{
	INT BIArrayIndex, ShiftIndex = 0;
	UINT8 value;

	if ((pBI == NULL) || (pBI->pIntegerArray == NULL))
		return;

	BIArrayIndex = pBI->ArrayLength - 1;

	while ((BIArrayIndex >= 0) && (pBI->pIntegerArray[BIArrayIndex] == 0))
		BIArrayIndex--;

	if (BIArrayIndex >= 0) {
		value = 0;
		ShiftIndex = 4;

		while (value == 0) {
			ShiftIndex--;
			value = UINT32_GETBYTE(pBI->pIntegerArray[BIArrayIndex], ShiftIndex);
		} /* End of while */
	} /* End of if */

	if (BIArrayIndex < 0) {
		pBI->IntegerLength = 1;
		pBI->ArrayLength = 1;
		pBI->Signed = 1;
	} else {
		pBI->IntegerLength = (BIArrayIndex * 4) + ShiftIndex + 1;
		pBI->ArrayLength = BIArrayIndex + 1;
	} /* End of if */
} /* End of BigInteger_ClearHighBits */

UINT32 BigInteger_getlen(
	IN BIG_INTEGER * pBI)
{
	return pBI->IntegerLength;
}



VOID BigInteger_BI2Bin(
	IN PBIG_INTEGER pBI,
	OUT UINT8 *pValue,
	OUT UINT *Length)
{
	INT  ValueIndex, BIArrayIndex, ShiftIndex;
	UINT32  Number;

	if (pBI == NULL) {
		DEBUGPRINT("BigInteger_BI2Bin: pBI is NUll\n");
		*Length = 0;
		return;
	} /* End of if */

	if (*Length < (sizeof(UINT8) * pBI->IntegerLength)) {
		DEBUGPRINT("BigInteger_BI2Bin: length(%d) is not enough.\n", *Length);
		*Length = 0;
		return;
	} /* End of if */

	if (pBI->pIntegerArray == NULL) {
		*Length = 0;
		return;
	} /* End of if */

	BigInteger_ClearHighBits(pBI);
	ShiftIndex = pBI->IntegerLength & 0x3;

	if (ShiftIndex == 0)
		ShiftIndex = 4;

	BIArrayIndex = pBI->ArrayLength - 1;
	ValueIndex = 0;
	Number = pBI->pIntegerArray[BIArrayIndex];

	while (ValueIndex < pBI->IntegerLength) {
		pValue[ValueIndex++] = (UINT8) UINT32_GETBYTE(Number, ShiftIndex - 1);

		if ((--ShiftIndex) == 0) {
			ShiftIndex = 4;
			BIArrayIndex--;
			Number = pBI->pIntegerArray[BIArrayIndex];
		} /* End of if */
	} /* End of while */

	*Length = pBI->IntegerLength;
} /* End of BigInteger_BI2Bin */


VOID BigInteger_BI2Bin_with_pad(
	IN PBIG_INTEGER pBI,
	OUT UINT8 *pValue,
	OUT UINT *Length,
	IN UINT32 PadLen)
{
	UINT32 offset;

	if (!pBI)
		return;

	offset = (pBI->IntegerLength > PadLen) ? 0 : PadLen - pBI->IntegerLength;
	NdisZeroMemory(pValue, offset);
	BigInteger_BI2Bin(pBI, pValue + offset, Length);
	*Length += offset;
}



VOID BigInteger_Bin2BI(
	IN UINT8 *pValue,
	IN UINT Length,
	OUT PBIG_INTEGER *pBI)
{
	INT  ValueIndex, BIArrayIndex, ShiftIndex;
	UINT32  Number;
	BigInteger_AllocSize(pBI, Length);

	if ((*pBI)->pIntegerArray != NULL) {
		Number = 0;
		ShiftIndex = Length & 0x3;

		if (ShiftIndex == 0)
			ShiftIndex = 4;

		BIArrayIndex = (*pBI)->ArrayLength - 1;
		ValueIndex = 0;

		while (ValueIndex < Length) {
			Number = (Number << 8) | (UINT8) pValue[ValueIndex++];

			if ((--ShiftIndex) == 0) {
				(*pBI)->pIntegerArray[BIArrayIndex] = Number;
				ShiftIndex = 4;
				BIArrayIndex--;
				Number = 0;
			} /* End of if */
		} /* End of while */
	} /* End of if */

	BigInteger_ClearHighBits(*pBI);
} /* End of BigInteger_Bin2BI */


/* Calculate the bits of BigInteger, the highest bit is 1 */
VOID BigInteger_BitsOfBI(
	IN PBIG_INTEGER pBI,
	OUT UINT *Bits_Of_P)
{
	UINT32 Number, Index;
	Number = pBI->pIntegerArray[pBI->ArrayLength - 1];
	Index = 0;

	while ((!(Number & 0x80000000)) && (Index < 32)) {
		Number <<= 1;
		Index++;
	} /* End of while */

	*Bits_Of_P = (pBI->ArrayLength * 32) - Index;
} /* End of BigInteger_BitsOfBN */


INT BigInteger_GetBitValue(
	IN PBIG_INTEGER pBI,
	IN UINT Index)
{
	UINT Array = 0;
	UINT Shift = 0;

	if (Index > 0) {
		Array = (Index - 1) >> 0x5;
		Shift = (Index - 1) & 0x1F;
	}

	if (Array > pBI->ArrayLength)
		return 0;

	return (pBI->pIntegerArray[Array] >> Shift) & 0x1;
} /* End of BigInteger_GetBitValue */


UINT8 BigInteger_GetByteValue(
	IN PBIG_INTEGER pBI,
	IN UINT Index)
{
	UINT Array = 0;
	UINT Shift = 0;

	if (Index > 0) {
		Array = (Index - 1) >> 0x2;
		Shift = (Index - 1) & 0x3;
	}

	if ((Array > pBI->ArrayLength) || (Index > pBI->IntegerLength))
		return 0;

	return (UINT8) UINT32_GETBYTE(pBI->pIntegerArray[Array], Shift);
} /* End of BigInteger_GetByteValue */


VOID BigInteger_Copy(
	IN PBIG_INTEGER pBI_Copied,
	OUT PBIG_INTEGER *pBI_Result)
{
	BigInteger_AllocSize(pBI_Result, pBI_Copied->IntegerLength);
	NdisCopyMemory((*pBI_Result)->pIntegerArray, pBI_Copied->pIntegerArray, (sizeof(UINT32) * (*pBI_Result)->ArrayLength));
	(*pBI_Result)->ArrayLength = pBI_Copied->ArrayLength;
	(*pBI_Result)->IntegerLength = pBI_Copied->IntegerLength;
	(*pBI_Result)->Signed = pBI_Copied->Signed;
} /* End of BigInteger_Copy */


INT BigInteger_UnsignedCompare(
	IN PBIG_INTEGER pFirstOperand,
	IN PBIG_INTEGER pSecondOperand)
{
	INT BIArrayIndex;
	BigInteger_ClearHighBits(pFirstOperand);
	BigInteger_ClearHighBits(pSecondOperand);

	if (pFirstOperand->IntegerLength == pSecondOperand->IntegerLength) {
		for (BIArrayIndex = (pFirstOperand->ArrayLength - 1); BIArrayIndex >= 0; BIArrayIndex--) {
			if (pFirstOperand->pIntegerArray[BIArrayIndex] > pSecondOperand->pIntegerArray[BIArrayIndex])
				return 1;
			else if (pFirstOperand->pIntegerArray[BIArrayIndex] < pSecondOperand->pIntegerArray[BIArrayIndex])
				return -1;
		} /* End of for */
	} /* End of if */

	if (pFirstOperand->IntegerLength > pSecondOperand->IntegerLength)
		return 1;

	if (pFirstOperand->IntegerLength < pSecondOperand->IntegerLength)
		return -1;

	return 0;
} /* End of BigInteger_Compare */


VOID BigInteger_Add(
	IN PBIG_INTEGER pFirstOperand,
	IN PBIG_INTEGER pSecondOperand,
	OUT PBIG_INTEGER *pBI_Result)
{
	INT CompareResult;
	UINT32 BIArrayIndex;
	UINT64 Sum, Carry;
	PBIG_INTEGER pTempBI = NULL;
#ifdef BI_TIME_REC
	struct timeval t1;
	struct timeval t2;
#endif

	if  ((pFirstOperand == NULL) || (pFirstOperand->pIntegerArray == NULL)
		 || (pSecondOperand == NULL) || (pSecondOperand->pIntegerArray == NULL)) {
		DEBUGPRINT("BigInteger_Add: first or second operand is NULL.\n");
		return;
	} /* End of if */

#ifdef BI_TIME_REC

	if (is_time_rec) {
		do_gettimeofday(&t1);
		is_add_alloc_rec = TRUE;
	}

#endif

	if (*pBI_Result == NULL)
		BigInteger_Init(pBI_Result);

	CompareResult = BigInteger_UnsignedCompare(pFirstOperand, pSecondOperand);

	if ((CompareResult == 0) & ((pFirstOperand->Signed * pSecondOperand->Signed) < 0)) {
		BigInteger_AllocSize(pBI_Result, 1);
		return;
	} /* End of if */

	POOL_COUNTER_CHECK_BEGIN(expected_cnt[0]);
	GET_BI_INS_FROM_POOL(pTempBI);

	/*
	 *  Singed table
	 *  A + B || A > B || A < B
	 *  ------------------------
	 *  +   + ||   +   ||   +
	 *  +   - ||   +   ||   -
	 *  -   + ||   -   ||   +
	 *  -   - ||   -   ||   -
	 */
	if ((pFirstOperand->Signed * pSecondOperand->Signed) > 0) {
		if (pFirstOperand->IntegerLength > pSecondOperand->IntegerLength)
			BigInteger_AllocSize(pBI_Result, pFirstOperand->IntegerLength + 1);
		else
			BigInteger_AllocSize(pBI_Result, pSecondOperand->IntegerLength + 1); /* End of if */

		Carry = 0;

		for (BIArrayIndex = 0; BIArrayIndex < (*pBI_Result)->ArrayLength; BIArrayIndex++) {
			Sum = 0;

			if (BIArrayIndex < pFirstOperand->ArrayLength)
				Sum += (UINT64) pFirstOperand->pIntegerArray[BIArrayIndex];

			if (BIArrayIndex < pSecondOperand->ArrayLength)
				Sum += (UINT64) pSecondOperand->pIntegerArray[BIArrayIndex];

			Sum += Carry;
			Carry = Sum  >> 32;
			(*pBI_Result)->pIntegerArray[BIArrayIndex] = (UINT32) (Sum & 0xffffffffUL);
		} /* End of for */

		(*pBI_Result)->Signed = pFirstOperand->Signed;
		BigInteger_ClearHighBits(*pBI_Result);
	} else {
		if  ((pFirstOperand->Signed == 1) & (pSecondOperand->Signed == -1)) {
			BigInteger_Copy(pSecondOperand, &pTempBI);
			pTempBI->Signed = 1;
			BigInteger_Sub(pFirstOperand, pTempBI, pBI_Result);
		} else if ((pFirstOperand->Signed == -1) & (pSecondOperand->Signed == 1)) {
			BigInteger_Copy(pFirstOperand, &pTempBI);
			pTempBI->Signed = 1;
			BigInteger_Sub(pSecondOperand, pTempBI, pBI_Result);
		} /* End of if */
	} /* End of if */

	release_temporal_usage_big_interger(&pTempBI);
	POOL_COUNTER_CHECK_END(expected_cnt[0]);
#ifdef BI_TIME_REC

	if (is_time_rec) {
		ULONG time_interval = 0;
		do_gettimeofday(&t2);

		if (t1.tv_usec > t2.tv_usec)
			time_interval = (t2.tv_sec - t1.tv_sec) * 1000000 - t1.tv_usec + t2.tv_usec;
		else
			time_interval = t2.tv_usec - t1.tv_usec;

		BigInteger_record_time_ns(&bi_op_ti_rec.add_op, time_interval);
		is_add_alloc_rec = FALSE;
	}

#endif
} /* End of BigInteger_Add */


VOID BigInteger_Sub(
	IN PBIG_INTEGER pFirstOperand,
	IN PBIG_INTEGER pSecondOperand,
	OUT PBIG_INTEGER *pBI_Result)
{
	INT CompareResult;
	UINT32 BIArrayIndex, Carry;
	PBIG_INTEGER pTempBI2 = NULL;
#ifdef BI_TIME_REC
	struct timeval t1;
	struct timeval t2;
#endif

	if  ((pFirstOperand == NULL) || (pFirstOperand->pIntegerArray == NULL)
		 || (pSecondOperand == NULL) || (pSecondOperand->pIntegerArray == NULL)) {
		DEBUGPRINT("BigInteger_Sub: first or second operand is NULL.\n");
		return;
	} /* End of if */

#ifdef BI_TIME_REC

	if (is_time_rec) {
		do_gettimeofday(&t1);
		is_sub_alloc_rec = TRUE;
	}

#endif

	if (*pBI_Result == NULL)
		BigInteger_Init(pBI_Result);

	CompareResult = BigInteger_UnsignedCompare(pFirstOperand, pSecondOperand);

	if ((CompareResult == 0) & ((pFirstOperand->Signed * pSecondOperand->Signed) > 0)) {
		BigInteger_AllocSize(pBI_Result, 1);
		return;
	} /* End of if */

	/*
	 *  Singed table
	 *  A - B || A > B || A < B
	 *  ------------------------
	 *  +   + ||   +   ||   -
	 *  +   - ||   +   ||   +
	 *  -   + ||   -   ||   -
	 *  -   - ||   -   ||   +
	 */
	if ((pFirstOperand->Signed * pSecondOperand->Signed) > 0) {
		UINT32 *pResultArray = NULL;
		UINT32 *pTempArray = NULL;

		if (CompareResult == 1) {
			BigInteger_Copy(pFirstOperand, pBI_Result);
			pTempBI2 = pSecondOperand;
		} else if (CompareResult == -1) {
			BigInteger_Copy(pSecondOperand, pBI_Result);
			pTempBI2 = pFirstOperand;
		} /* End of if */

		pResultArray = (*pBI_Result)->pIntegerArray;
		pTempArray = pTempBI2->pIntegerArray;
		Carry = 0;

		for (BIArrayIndex = 0; BIArrayIndex < (*pBI_Result)->ArrayLength; BIArrayIndex++) {
			if (BIArrayIndex < pTempBI2->ArrayLength) {
				if (((pResultArray[BIArrayIndex] - Carry) < pTempArray[BIArrayIndex])
					|| ((pResultArray[BIArrayIndex] == 0) && (Carry == 1))) {
					pResultArray[BIArrayIndex] = 0xffffffffUL - pTempArray[BIArrayIndex] - Carry + pResultArray[BIArrayIndex] + 1;
					Carry = 1;
				} else {
					pResultArray[BIArrayIndex] = pResultArray[BIArrayIndex] - pTempArray[BIArrayIndex] - Carry;
					Carry = 0;
				} /* End of if */
			} else {
				if (pResultArray[BIArrayIndex] >= Carry) {
					pResultArray[BIArrayIndex] -= Carry;
					Carry = 0;
					break;
				} else {
					pResultArray[BIArrayIndex] = 0xffffffffUL;
					Carry = 1;
				} /* End of if */
			} /* End of if */
		} /* End of for */


		if  (((pFirstOperand->Signed == 1) & (pSecondOperand->Signed == 1) & (CompareResult == -1))
			 || ((pFirstOperand->Signed == -1) & (pSecondOperand->Signed == -1) & (CompareResult == 1)))
			(*pBI_Result)->Signed = -1;

		BigInteger_ClearHighBits(*pBI_Result);
	} else {
		PBIG_INTEGER pTempBI = NULL;
		POOL_COUNTER_CHECK_BEGIN(expected_cnt[7]);
		GET_BI_INS_FROM_POOL(pTempBI);
		BigInteger_Init(&pTempBI);

		if  ((pFirstOperand->Signed == 1) & (pSecondOperand->Signed == -1)) {
			BigInteger_Copy(pSecondOperand, &pTempBI);
			pTempBI->Signed = 1;
			BigInteger_Add(pFirstOperand, pTempBI, pBI_Result);
		} else if ((pFirstOperand->Signed == -1) & (pSecondOperand->Signed == 1)) {
			BigInteger_Copy(pFirstOperand, &pTempBI);
			pTempBI->Signed = 1;
			BigInteger_Add(pTempBI, pSecondOperand, pBI_Result);
			(*pBI_Result)->Signed = -1;
		} /* End of if */

		release_temporal_usage_big_interger(&pTempBI);
		POOL_COUNTER_CHECK_END(expected_cnt[7]);
	} /* End of if */

#ifdef BI_TIME_REC

	if (is_time_rec) {
		ULONG time_interval = 0;
		do_gettimeofday(&t2);

		if (t1.tv_usec > t2.tv_usec)
			time_interval = (t2.tv_sec - t1.tv_sec) * 1000000 - t1.tv_usec + t2.tv_usec;
		else
			time_interval = t2.tv_usec - t1.tv_usec;

		BigInteger_record_time_ns(&bi_op_ti_rec.sub_op, time_interval);
		is_sub_alloc_rec = FALSE;
	}

#endif
} /* End of BigInteger_Sub */


VOID BigInteger_Mul(
	IN PBIG_INTEGER pFirstOperand,
	IN PBIG_INTEGER pSecondOperand,
	OUT PBIG_INTEGER *pBI_Result)
{
	UINT32 BIFirstIndex, BISecondIndex;
	UINT64 FirstValue, SecondValue, Sum, Carry;
#ifdef BI_TIME_REC
	struct timeval t1;
	struct timeval t2;
#endif

	if  ((pFirstOperand == NULL) || (pFirstOperand->pIntegerArray == NULL)
		 || (pSecondOperand == NULL) || (pSecondOperand->pIntegerArray == NULL)) {
		DEBUGPRINT("BigInteger_Mul: first or second operand is NULL.\n");
		return;
	} /* End of if */

#ifdef BI_TIME_REC
	if (is_time_rec) {
		do_gettimeofday(&t1);
		is_mul_alloc_rec = TRUE;
	}
#endif

	/* The first or second operand is zero */
	if  (((pFirstOperand->IntegerLength  == 1) && (pFirstOperand->pIntegerArray[0]  == 0))
		 || ((pSecondOperand->IntegerLength == 1) && (pSecondOperand->pIntegerArray[0] == 0))) {
		BigInteger_AllocSize(pBI_Result, 1);
		goto output;
	} /* End of if */

	/* The first or second operand is one */
	if  ((pFirstOperand->IntegerLength  == 1) && (pFirstOperand->pIntegerArray[0]  == 1)) {
		BigInteger_Copy(pSecondOperand, pBI_Result);
		goto output;
	} /* End of if */

	if  ((pSecondOperand->IntegerLength  == 1) && (pSecondOperand->pIntegerArray[0]  == 1)) {
		BigInteger_Copy(pFirstOperand, pBI_Result);
		goto output;
	} /* End of if */

	BigInteger_AllocSize(pBI_Result, pFirstOperand->IntegerLength + pSecondOperand->IntegerLength);

	for (BIFirstIndex = 0; BIFirstIndex < pFirstOperand->ArrayLength; BIFirstIndex++) {
		Carry = 0;
		FirstValue = (UINT64) pFirstOperand->pIntegerArray[BIFirstIndex];

		if (FirstValue == 0)
			continue;
		else {
			for (BISecondIndex = 0; BISecondIndex < pSecondOperand->ArrayLength; BISecondIndex++) {
				SecondValue = ((UINT64) pSecondOperand->pIntegerArray[BISecondIndex]) * FirstValue;
				Sum = (UINT64) ((*pBI_Result)->pIntegerArray[BIFirstIndex + BISecondIndex] + SecondValue + Carry);
				Carry = Sum >> 32;
				(*pBI_Result)->pIntegerArray[BIFirstIndex + BISecondIndex] = (UINT32) (Sum & 0xffffffffUL);
			} /* End of for */

			while (Carry != 0) {
				Sum = (UINT64) (*pBI_Result)->pIntegerArray[BIFirstIndex + BISecondIndex];
				Sum += Carry;
				Carry = Sum >> 32;
				(*pBI_Result)->pIntegerArray[BIFirstIndex + BISecondIndex] = (UINT32) (Sum & 0xffffffffUL);
				BISecondIndex++;
			} /* End of while */
		} /* End of if */
	} /* End of for */

output:
	(*pBI_Result)->Signed = pFirstOperand->Signed * pSecondOperand->Signed;
	BigInteger_ClearHighBits(*pBI_Result);
#ifdef BI_TIME_REC

	if (is_time_rec) {
		ULONG time_interval = 0;
		do_gettimeofday(&t2);

		if (t1.tv_usec > t2.tv_usec)
			time_interval = (t2.tv_sec - t1.tv_sec) * 1000000 - t1.tv_usec + t2.tv_usec;
		else
			time_interval = t2.tv_usec - t1.tv_usec;

		BigInteger_record_time_ns(&bi_op_ti_rec.mul_op, time_interval);
		is_mul_alloc_rec = FALSE;
	}

#endif
} /* End of BigInteger_Mul */


VOID BigInteger_Square(
	IN PBIG_INTEGER pBI,
	OUT PBIG_INTEGER *pBI_Result)
{
	INT BIFirstIndex, BISecondIndex;
	UINT32 HBITS_Value, LBITS_Value, Temp1_Value, Temp2_Value, Carry32;
	UINT32 *Point_Of_S, *Point_Of_Result, *Point_Of_BI;
	UINT64 Result64_1, Result64_2, Carry64, TempValue64;
#ifdef BI_TIME_REC
	struct timeval t1;
	struct timeval t2;
#endif

	if ((pBI == NULL) || (pBI->pIntegerArray == NULL)) {
		DEBUGPRINT("\tBigInteger_Square: the operand is NULL.\n");
		return;
	} /* End of if */

#ifdef BI_TIME_REC

	if (is_time_rec)
		do_gettimeofday(&t1);

#endif

	/* The operand is zero */
	if  ((pBI->IntegerLength  == 1) && (pBI->pIntegerArray[0]  ==  0)) {
		BigInteger_AllocSize(pBI_Result, 1);
		goto output;
	} /* End of if */

	BigInteger_AllocSize(pBI_Result, (pBI->IntegerLength * 2) + 20);
	BigInteger_AllocSize(&pBI_S, (pBI->IntegerLength * 2) + 20);
	BigInteger_AllocSize(&pBI_O, (pBI->IntegerLength * 2) + 20);
	/*
	 * Input: pBI = {a_0, a_1, a_2, a_3, ..., a_n}
	 * Step1. calculate a_0^2, a_1^2, a_2^2, a_3^2 ... a_n^2
	 */
	Point_Of_S = pBI_S->pIntegerArray;

	for (BIFirstIndex = 0; BIFirstIndex < pBI->ArrayLength; BIFirstIndex++) {
		HBITS_Value = UINT32_HBITS(pBI->pIntegerArray[BIFirstIndex]);
		LBITS_Value = UINT32_LBITS(pBI->pIntegerArray[BIFirstIndex]);
		Temp1_Value = HBITS_Value * LBITS_Value;
		Temp2_Value = (Temp1_Value & 0x7fff) << 0x11;
		Point_Of_S[0] = (LBITS_Value * LBITS_Value) + Temp2_Value;
		Point_Of_S[1] = (HBITS_Value * HBITS_Value) + ((Temp1_Value >> 0xf) & 0x1ffff);

		if (Point_Of_S[0] < Temp2_Value)
			Point_Of_S[1] += 1;

		Point_Of_S += 2;
	} /* End of for */

	/*
	 * Step2. calculate a_0*{a_1, a_2, a_3, a_4, ..., a_n}
	 */
	Point_Of_BI = pBI->pIntegerArray;
	Point_Of_Result = (*pBI_Result)->pIntegerArray;
	Point_Of_Result[0] = 0;
	TempValue64 = (UINT64) Point_Of_BI[0];
	Point_Of_Result++;
	Carry64 = 0;

	for (BIFirstIndex = 1; BIFirstIndex < pBI->ArrayLength; BIFirstIndex++) {
		Result64_1 =  (UINT64) Point_Of_BI[BIFirstIndex] * TempValue64;
		Result64_1 += Carry64;
		Carry64 = (Result64_1 >> 32);
		Point_Of_Result[0] = (UINT32) (Result64_1 & 0xffffffffUL);
		Point_Of_Result++;
	} /* End of for */

	if (Carry64 > 0)
		Point_Of_Result[0] = (UINT32) (Carry64 & 0xffffffffUL);

	/*
	 * Step3. calculate
	 *           a_1*{a_2, a_3, a_4, ..., a_n}
	 *           a_2*{a_3, a_4, a_5, ..., a_n}
	 *           a_3*{a_4, a_5, a_6, ..., a_n}
	 *           a_4*{a_5, a_6, a_7, ..., a_n}
	 *           ...
	 *           a_n-1*{a_n}
	 */
	Point_Of_BI = pBI->pIntegerArray;

	for (BIFirstIndex = 1; BIFirstIndex < (pBI->ArrayLength - 1); BIFirstIndex++) {
		Point_Of_Result = (*pBI_Result)->pIntegerArray;
		Point_Of_Result += (BIFirstIndex * 2) + 1;
		TempValue64 = (UINT64) Point_Of_BI[BIFirstIndex];
		Carry64 = 0;

		for (BISecondIndex = (BIFirstIndex + 1); BISecondIndex < pBI->ArrayLength; BISecondIndex++) {
			Result64_1 = ((UINT64) Point_Of_Result[0]) + Carry64;
			Result64_2 = (UINT64) Point_Of_BI[BISecondIndex] * TempValue64;
			Carry64 = (Result64_1 >> 32);
			Result64_1 = (Result64_1 & 0xffffffffUL);
			Result64_1 = Result64_1 + Result64_2;
			Carry64 += (Result64_1 >> 32);
			Point_Of_Result[0] = (UINT32) (Result64_1 & 0xffffffffUL);
			Point_Of_Result++;
		} /* End of for */

		if (Carry64 > 0)
			Point_Of_Result[0] += (UINT32) (Carry64 & 0xffffffffUL);
	} /* End of for */

	BigInteger_ClearHighBits(*pBI_Result);
	BigInteger_Copy(*pBI_Result, &pBI_O);
	Carry32 = 0;

	for (BIFirstIndex = 0; BIFirstIndex < pBI_O->ArrayLength; BIFirstIndex++) {
		pBI_O->pIntegerArray[BIFirstIndex] = (pBI_O->pIntegerArray[BIFirstIndex] << 1) | Carry32;

		if (pBI_O->pIntegerArray[BIFirstIndex] < (*pBI_Result)->pIntegerArray[BIFirstIndex])
			Carry32 = 1;
		else
			Carry32 = 0;
	} /* End of for */

	pBI_O->pIntegerArray[BIFirstIndex] = Carry32;
	pBI_O->IntegerLength++;
	pBI_O->ArrayLength++;
	BigInteger_ClearHighBits(pBI_O);
	BigInteger_Add(pBI_O, pBI_S, pBI_Result);
output:
	(*pBI_Result)->Signed = 1;
	BigInteger_ClearHighBits(*pBI_Result);
	BigInteger_Free(&pBI_O);
	BigInteger_Free(&pBI_S);
#ifdef BI_TIME_REC

	if (is_time_rec) {
		ULONG time_interval = 0;
		do_gettimeofday(&t2);

		if (t1.tv_usec > t2.tv_usec)
			time_interval = (t2.tv_sec - t1.tv_sec) * 1000000 - t1.tv_usec + t2.tv_usec;
		else
			time_interval = t2.tv_usec - t1.tv_usec;

		BigInteger_record_time_ns(&bi_op_ti_rec.square_op, time_interval);
	}

#endif
} /* End of BigInteger_Square */


VOID BigInteger_Div(
	IN PBIG_INTEGER pFirstOperand,
	IN PBIG_INTEGER pSecondOperand,
	OUT PBIG_INTEGER *pBI_Result,
	OUT PBIG_INTEGER *pBI_Remainder)
{
	INT CompareResult;
	INT Index, MulIndex, ComputeSize;
	UINT32 MulLoopStart, MulLoopEnd;
	UINT AllocLength, ArrayIndex, ShiftIndex;
	UINT8 SecondHighByte;
#ifdef BI_TIME_REC
	struct timeval t1;
	struct timeval t2;
#endif
	PBIG_INTEGER pTempBI = NULL;
	PBIG_INTEGER pTempBI2 = NULL;
	PBIG_INTEGER pMulBI = NULL;
#ifdef BI_TIME_REC

	if (is_time_rec) {
		do_gettimeofday(&t1);
		is_div_alloc_rec = TRUE;
	}

#endif

	if  ((pFirstOperand == NULL) || (pFirstOperand->pIntegerArray == NULL)
		 || (pSecondOperand == NULL) || (pSecondOperand->pIntegerArray == NULL)) {
		DEBUGPRINT("BigInteger_Div: first or second operand is NULL.\n");
		return;
	} /* End of if */

	/* The second operand is zero */
	if ((pSecondOperand->IntegerLength == 1) && (pSecondOperand->pIntegerArray[0] == 0)) {
		DEBUGPRINT("BigInteger_Div: second operand is zero.\n");
		return;
	} /* End of if */

	POOL_COUNTER_CHECK_BEGIN(expected_cnt[16]);
	GET_BI_INS_FROM_POOL(pTempBI);
	GET_BI_INS_FROM_POOL(pTempBI2);
	GET_BI_INS_FROM_POOL(pMulBI);

	if (*pBI_Result == NULL)
		BigInteger_Init(pBI_Result);

	if (*pBI_Remainder == NULL)
		BigInteger_Init(pBI_Remainder);

	/* The second operand is one */
	if  ((pSecondOperand->IntegerLength  == 1) && (pSecondOperand->pIntegerArray[0]  == 1)) {
		BigInteger_Copy(pFirstOperand, pBI_Result);
		BigInteger_Bin2BI(Value_0, 1, pBI_Remainder);
		goto output;
	} /* End of if */

	CompareResult = BigInteger_UnsignedCompare(pFirstOperand, pSecondOperand);

	if (CompareResult == 0) {
		BigInteger_Bin2BI(Value_1, 1, pBI_Result);
		BigInteger_Bin2BI(Value_0, 1, pBI_Remainder);
		goto output;
	} else if (CompareResult == -1) {
		BigInteger_Bin2BI(Value_0, 1, pBI_Result);
		BigInteger_Copy(pFirstOperand, pBI_Remainder);
		goto output;
	} /* End of if */

	BigInteger_AllocSize(pBI_Result, pFirstOperand->IntegerLength - pSecondOperand->IntegerLength + 1);
	BigInteger_AllocSize(pBI_Remainder, pSecondOperand->IntegerLength);
	AllocLength = (UINT) (pFirstOperand->IntegerLength << 1);
	BigInteger_AllocSize(&pTempBI, AllocLength);
	BigInteger_AllocSize(&pTempBI2, AllocLength);
	BigInteger_Copy(pFirstOperand, pBI_Remainder);
	SecondHighByte = (BigInteger_GetByteValue(pSecondOperand, pSecondOperand->IntegerLength) & 0xFF);
	ComputeSize = (INT) pFirstOperand->IntegerLength - pSecondOperand->IntegerLength + 1;

	for (Index = (INT) ComputeSize; Index >= 0; Index--) {
		if (BigInteger_UnsignedCompare(*pBI_Remainder, pSecondOperand) == -1)
			break;

		if (((pSecondOperand->IntegerLength + Index) - (*pBI_Remainder)->IntegerLength) <= 1) {
			BigInteger_AllocSize(&pMulBI, Index + 1);
			ArrayIndex = 0;

			if (Index > 0)
				ArrayIndex = (UINT) (Index - 1) >> 2;

			ShiftIndex = (Index & 0x03);

			if (ShiftIndex == 0)
				ShiftIndex = 4;

			ShiftIndex--;
			MulLoopStart = (BigInteger_GetByteValue((*pBI_Remainder), pFirstOperand->IntegerLength + Index - ComputeSize + 1) & 0xFF) << 8;
			MulLoopStart = MulLoopStart | (BigInteger_GetByteValue((*pBI_Remainder), pFirstOperand->IntegerLength + Index - ComputeSize) & 0xFF);

			if (MulLoopStart < (UINT32) SecondHighByte)
				continue;

			MulLoopEnd = (MulLoopStart / (UINT32) SecondHighByte) + 1;
			MulLoopStart = MulLoopStart / (UINT32) (SecondHighByte + 1);

			for (MulIndex = (INT) MulLoopStart; MulIndex <= MulLoopEnd; MulIndex++) { /* 0xFFFF / 0x01 = 0xFFFF */
				NdisZeroMemory(pMulBI->pIntegerArray, pMulBI->AllocSize);

				if ((MulIndex > 0xff) && (ShiftIndex == 3)) {
					pMulBI->pIntegerArray[ArrayIndex] = ((UINT) (MulIndex & 0xff) << (8 * ShiftIndex));
					pMulBI->pIntegerArray[ArrayIndex + 1] = (UINT) (MulIndex >> 8) & 0xff;
				} else
					pMulBI->pIntegerArray[ArrayIndex] = ((UINT) (MulIndex) << (8 * ShiftIndex));

				/* BigInteger_ClearHighBits(pMulBI); */
				BigInteger_Mul(pSecondOperand, pMulBI, &pTempBI);
				CompareResult = BigInteger_UnsignedCompare(*pBI_Remainder, pTempBI);

				if (CompareResult < 1) {
					if (CompareResult == -1) {
						MulIndex = MulIndex - 1;
						BigInteger_AllocSize(&pMulBI, Index + 1);
						NdisZeroMemory(pMulBI->pIntegerArray, pMulBI->AllocSize);

						if ((MulIndex > 0xff) && (ShiftIndex == 3)) {
							pMulBI->pIntegerArray[ArrayIndex] = ((UINT) (MulIndex & 0xff) << (8 * ShiftIndex));
							pMulBI->pIntegerArray[ArrayIndex + 1] = (UINT) (MulIndex >> 8) & 0xff;
						} else
							pMulBI->pIntegerArray[ArrayIndex] = ((UINT) (MulIndex) << (8 * ShiftIndex));

						BigInteger_Mul(pSecondOperand, pMulBI, &pTempBI);
					}

					BigInteger_Sub(*pBI_Remainder, pTempBI, &pTempBI2);
					BigInteger_Copy(pTempBI2, pBI_Remainder);
					BigInteger_Add(*pBI_Result, pMulBI, &pTempBI2);
					BigInteger_Copy(pTempBI2, pBI_Result);
					break;
				} /* End of if */
			} /* End of for */
		} /* End of if */
	} /* End of for */

output:
	(*pBI_Result)->Signed = pFirstOperand->Signed * pSecondOperand->Signed;
	(*pBI_Remainder)->Signed = pFirstOperand->Signed * pSecondOperand->Signed;
	BigInteger_ClearHighBits(*pBI_Result);
	BigInteger_ClearHighBits(*pBI_Remainder);
	release_temporal_usage_big_interger(&pTempBI);
	release_temporal_usage_big_interger(&pTempBI2);
	release_temporal_usage_big_interger(&pMulBI);
	POOL_COUNTER_CHECK_END(expected_cnt[16]);
#ifdef BI_TIME_REC

	if (is_time_rec) {
		ULONG time_interval = 0;
		do_gettimeofday(&t2);

		if (t1.tv_usec > t2.tv_usec)
			time_interval = (t2.tv_sec - t1.tv_sec) * 1000000 - t1.tv_usec + t2.tv_usec;
		else
			time_interval = t2.tv_usec - t1.tv_usec;

		BigInteger_record_time_ns(&bi_op_ti_rec.div_op, time_interval);
		is_div_alloc_rec = FALSE;
	}

#endif
} /* End of BigInteger_Div */

#ifdef WSC_INCLUDED
static UINT8 WPS_DH_PInverse_VALUE[192] = {
	0xd9, 0xc7, 0xd8, 0x95, 0xed, 0x5a, 0xa0, 0xac,
	0xa4, 0xb4, 0xbc, 0x87, 0x43, 0x7d, 0x88, 0x13,
	0x0f, 0xce, 0xd9, 0xcc, 0x8f, 0x13, 0x84, 0x81,
	0xf5, 0x32, 0x2b, 0x5c, 0xfd, 0x0b, 0x7b, 0xd7,
	0x22, 0x57, 0x31, 0xdb, 0xb9, 0x2e, 0x86, 0x4c,
	0xab, 0xa2, 0x74, 0xbb, 0x68, 0x04, 0xd4, 0x4f,
	0x32, 0x46, 0xbb, 0xae, 0x44, 0x6d, 0xfb, 0xdf,
	0x7b, 0x2c, 0xaa, 0x2f, 0x7f, 0xfb, 0xa3, 0x2d,
	0xa3, 0x88, 0x05, 0x0d, 0xbc, 0xd5, 0xe5, 0xac,
	0xcb, 0x79, 0xe6, 0xa5, 0xa5, 0x0f, 0x3e, 0xd6,
	0x01, 0xe2, 0x38, 0x7b, 0xfb, 0x98, 0x61, 0x69,
	0x01, 0xaa, 0x42, 0xec, 0xcd, 0x35, 0xc5, 0x13,
	0x0d, 0x4d, 0xd6, 0x0b, 0x30, 0x93, 0x4a, 0xd4,
	0x7b, 0xb1, 0x12, 0xfa, 0xe1, 0x78, 0x10, 0xca,
	0x67, 0x6e, 0xc3, 0xcc, 0x40, 0xb6, 0xa9, 0x05,
	0x74, 0x5e, 0xf2, 0x05, 0xa8, 0xf0, 0x90, 0x88,
	0x5f, 0xee, 0x48, 0xe1, 0xfb, 0x0a, 0xae, 0x62,
	0xcf, 0x9d, 0xe9, 0x4f, 0x4d, 0x2f, 0x59, 0x54,
	0x7f, 0x2e, 0xaf, 0x68, 0xe7, 0xe8, 0x23, 0xa3,
	0x21, 0x21, 0x56, 0x15, 0x96, 0x31, 0x09, 0xb9,
	0x1a, 0xf2, 0x5b, 0x38, 0x6a, 0x3c, 0x43, 0x7a,
	0x3e, 0x49, 0x1e, 0x72, 0x1b, 0x79, 0x8e, 0x44,
	0xf1, 0x74, 0x6c, 0x08, 0xca, 0x23, 0x73, 0x28,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
};
#endif


static VOID BigInteger_Inverse(
	IN PBIG_INTEGER pFirstOperand,
	IN PBIG_INTEGER pSecondOperand,
	OUT PBIG_INTEGER *pFirstInverse,
	OUT PBIG_INTEGER *pSecondInverse)
{
	PBIG_INTEGER pBI_A = NULL, pBI_B = NULL;
	PBIG_INTEGER pBI_X = NULL, pBI_X1 = NULL, pBI_X2 = NULL;
	PBIG_INTEGER pBI_Y = NULL, pBI_Y1 = NULL, pBI_Y2 = NULL;
	PBIG_INTEGER pBI_Q = NULL, pBI_R = NULL;
	PBIG_INTEGER pBI_Temp = NULL;
	POOL_COUNTER_CHECK_BEGIN(expected_cnt[15]);
	GET_BI_INS_FROM_POOL(pBI_A);
	GET_BI_INS_FROM_POOL(pBI_B);
	GET_BI_INS_FROM_POOL(pBI_X);
	GET_BI_INS_FROM_POOL(pBI_X1);
	GET_BI_INS_FROM_POOL(pBI_X2);
	GET_BI_INS_FROM_POOL(pBI_Y);
	GET_BI_INS_FROM_POOL(pBI_Y1);
	GET_BI_INS_FROM_POOL(pBI_Y2);
	GET_BI_INS_FROM_POOL(pBI_Q);
	GET_BI_INS_FROM_POOL(pBI_R);
	GET_BI_INS_FROM_POOL(pBI_Temp);
	BigInteger_Init(&pBI_A);
	BigInteger_Init(&pBI_B);
	BigInteger_Init(&pBI_X);
	BigInteger_Init(&pBI_X1);
	BigInteger_Init(&pBI_X2);
	BigInteger_Init(&pBI_Y);
	BigInteger_Init(&pBI_Y1);
	BigInteger_Init(&pBI_Y2);
	BigInteger_Init(&pBI_Q);
	BigInteger_Init(&pBI_R);
	BigInteger_Init(&pBI_Temp);
	BigInteger_Copy(pFirstOperand, &pBI_A);
	BigInteger_Copy(pSecondOperand, &pBI_B);
	BigInteger_AllocSize(&pBI_Q, 1);
	BigInteger_AllocSize(&pBI_R, 1);
	BigInteger_AllocSize(&pBI_X, 1);
	BigInteger_AllocSize(&pBI_Y, 1);
	BigInteger_AllocSize(&pBI_X2, 1);
	BigInteger_AllocSize(&pBI_X1, 1);
	BigInteger_AllocSize(&pBI_Y2, 1);
	BigInteger_AllocSize(&pBI_Y1, 1);
	pBI_X2->pIntegerArray[0] = 0x1;
	pBI_Y1->pIntegerArray[0] = 0x1;

	while (((pBI_B->IntegerLength > 1) || ((pBI_B->IntegerLength == 1) && (pBI_B->pIntegerArray[0] > 0))) && (pBI_B->Signed == 1)) {
		BigInteger_Div(pBI_A, pBI_B, &pBI_Q, &pBI_R);
		BigInteger_Mul(pBI_Q, pBI_X1, &pBI_Temp);
		BigInteger_Sub(pBI_X2, pBI_Temp, &pBI_X);
		BigInteger_Mul(pBI_Q, pBI_Y1, &pBI_Temp);
		BigInteger_Sub(pBI_Y2, pBI_Temp, &pBI_Y);
		BigInteger_Copy(pBI_B, &pBI_A);
		BigInteger_Copy(pBI_R, &pBI_B);
		BigInteger_Copy(pBI_X1, &pBI_X2);
		BigInteger_Copy(pBI_X, &pBI_X1);
		BigInteger_Copy(pBI_Y1, &pBI_Y2);
		BigInteger_Copy(pBI_Y, &pBI_Y1);
	} /* End of while */

	if (((pBI_X2->IntegerLength > 1) || ((pBI_X2->IntegerLength == 1) && (pBI_X2->pIntegerArray[0] > 0))) && (pBI_X2->Signed == -1)) {
		BigInteger_Add(pSecondOperand, pBI_X2, &pBI_Temp);
		BigInteger_Copy(pBI_Temp, &pBI_X2);
		BigInteger_Sub(pFirstOperand, pBI_Y2, &pBI_Temp);
		BigInteger_Copy(pBI_Temp, &pBI_Y2);
	} /* End of if */

	if (((pBI_Y2->IntegerLength > 1) || ((pBI_Y2->IntegerLength == 1) && (pBI_Y2->pIntegerArray[0] > 0))) && (pBI_Y2->Signed == -1))
		pBI_Y2->Signed = 1; /* End of if */

	BigInteger_Copy(pBI_X2, pFirstInverse);
	BigInteger_Copy(pBI_Y2, pSecondInverse);
	release_temporal_usage_big_interger(&pBI_A);
	release_temporal_usage_big_interger(&pBI_B);
	release_temporal_usage_big_interger(&pBI_X);
	release_temporal_usage_big_interger(&pBI_X1);
	release_temporal_usage_big_interger(&pBI_X2);
	release_temporal_usage_big_interger(&pBI_Y);
	release_temporal_usage_big_interger(&pBI_Y1);
	release_temporal_usage_big_interger(&pBI_Y2);
	release_temporal_usage_big_interger(&pBI_Q);
	release_temporal_usage_big_interger(&pBI_R);
	release_temporal_usage_big_interger(&pBI_Temp);
	POOL_COUNTER_CHECK_END(expected_cnt[15]);
} /* End of BigInteger_Inverse */




static VOID BigInteger_Div2Exponent(
	IN PBIG_INTEGER pBI,
	IN UINT Exponent,
	OUT PBIG_INTEGER *pBI_Result,
	OUT PBIG_INTEGER *pBI_Remainder)
{
	UINT Bites_Of_BI, Bits_Of_Result, Bits_Of_Remainder;
	UINT ResultLength, ResultArrayIndex, ResultShiftIndex;
	UINT RemainderLength, RemainderArrayIndex, RemainderShiftIndex;
	UINT BIArrayIndex, ShiftIndex;
	UINT ReadShift = 0, WriteShift = 0, ReadBits = 0;
	UINT32 Value, TempValue;

	if (pBI == NULL) {
		DEBUGPRINT("BigInteger_Div2Exponent: pBI is NUll\n");
		(*pBI_Result) = NULL;
		return;
	} /* End of if */

	BigInteger_BitsOfBI(pBI, &Bites_Of_BI);

	if (Bites_Of_BI < Exponent) {
		BigInteger_Bin2BI(Value_0, 1, pBI_Result);
		BigInteger_Copy(pBI, pBI_Remainder);
		return;
	} /* End of if */

	Bits_Of_Result = Bites_Of_BI - Exponent + 1;
	ResultLength = Bits_Of_Result  >> 0x3L;

	if ((Bits_Of_Result & 0x7) > 0)
		ResultLength++;

	BigInteger_AllocSize(pBI_Result, ResultLength);

	ResultShiftIndex = Bits_Of_Result & 0x1FL;
	if (ResultShiftIndex == 0)
		ResultShiftIndex = 32;

	ResultArrayIndex = (*pBI_Result)->ArrayLength - 1;
	Bits_Of_Remainder = Exponent - 1;
	RemainderLength = Bits_Of_Remainder >> 0x3L;

	if ((Bits_Of_Remainder & 0x7) > 0)
		RemainderLength++;

	BigInteger_AllocSize(pBI_Remainder, RemainderLength);

	RemainderShiftIndex = Bits_Of_Remainder & 0x1FL;
	if (RemainderShiftIndex == 0)
		RemainderShiftIndex = 32;

	RemainderArrayIndex = (*pBI_Remainder)->ArrayLength - 1;

	ShiftIndex = Bites_Of_BI & 0x1FL;
	if (ShiftIndex == 0)
		ShiftIndex = 32;

	BIArrayIndex = pBI->ArrayLength - 1;
	Value = 0;

	while (Bits_Of_Result > 0) {
		if ((ResultShiftIndex >= ShiftIndex)  && (ShiftIndex <= Bits_Of_Result)) {
			ReadShift = 0;
			WriteShift = ResultShiftIndex - ShiftIndex;
			ReadBits = ShiftIndex;
		} else if ((ResultShiftIndex < ShiftIndex)  && (ResultShiftIndex <= Bits_Of_Result)) {
			ReadShift = ShiftIndex - ResultShiftIndex;
			WriteShift = 0;
			ReadBits = ResultShiftIndex;
		} else if ((ResultShiftIndex > Bits_Of_Result)  && (ShiftIndex > Bits_Of_Result)) {
			ReadShift = ShiftIndex - Bits_Of_Result;
			WriteShift = ResultShiftIndex - Bits_Of_Result;
			ReadBits = Bits_Of_Result;
		} /* End of if */

		TempValue = (UINT32)  ((pBI->pIntegerArray[BIArrayIndex] & (0xFFFFFFFFL >> (32 - ShiftIndex))) >> ReadShift);
		Value = Value | (TempValue << WriteShift);
		ResultShiftIndex -= ReadBits;
		ShiftIndex -= ReadBits;
		Bits_Of_Result -= ReadBits;

		if (ResultShiftIndex == 0) {
			(*pBI_Result)->pIntegerArray[ResultArrayIndex] = Value;
			ResultShiftIndex = 32;
			ResultArrayIndex--;
			Value = 0;
		} /* End of if */

		if (ShiftIndex == 0) {
			ShiftIndex = 32;
			BIArrayIndex--;
		} /* End of if */
	} /* End of while */

	if (ResultShiftIndex != 32)
		(*pBI_Result)->pIntegerArray[ResultArrayIndex] = Value;

	Value = 0;

	while (Bits_Of_Remainder > 0) {
		if ((RemainderShiftIndex >= ShiftIndex)  && (ShiftIndex <= Bits_Of_Remainder)) {
			ReadShift = 0;
			WriteShift = RemainderShiftIndex - ShiftIndex;
			ReadBits = ShiftIndex;
		} else if ((RemainderShiftIndex < ShiftIndex)  && (RemainderShiftIndex <= Bits_Of_Remainder)) {
			ReadShift = ShiftIndex - RemainderShiftIndex;
			WriteShift = 0;
			ReadBits = RemainderShiftIndex;
		} else if ((RemainderShiftIndex > Bits_Of_Remainder)  && (ShiftIndex > Bits_Of_Remainder)) {
			ReadShift = ShiftIndex - Bits_Of_Remainder;
			WriteShift = RemainderShiftIndex - Bits_Of_Remainder;
			ReadBits = Bits_Of_Remainder;
		} /* End of if */

		TempValue = (UINT32)  ((pBI->pIntegerArray[BIArrayIndex] & (0xFFFFFFFFL >> (32 - ShiftIndex))) >> ReadShift);
		Value = Value | (TempValue << WriteShift);
		RemainderShiftIndex -= ReadBits;
		ShiftIndex -= ReadBits;
		Bits_Of_Remainder -= ReadBits;

		if (RemainderShiftIndex == 0) {
			(*pBI_Remainder)->pIntegerArray[RemainderArrayIndex] = Value;
			RemainderShiftIndex = 32;
			RemainderArrayIndex--;
			Value = 0;
		} /* End of if */

		if (ShiftIndex == 0) {
			ShiftIndex = 32;
			BIArrayIndex--;
		} /* End of if */
	} /* End of while */

	if (RemainderShiftIndex != 32)
		(*pBI_Remainder)->pIntegerArray[RemainderArrayIndex] = Value;

	(*pBI_Result)->Signed = pBI->Signed;
	(*pBI_Remainder)->Signed = pBI->Signed;
	BigInteger_ClearHighBits(*pBI_Result);
	BigInteger_ClearHighBits(*pBI_Remainder);
} /* End of BigInteger_Div2Exponent */


static VOID BigInteger_Square2(
	IN PBIG_INTEGER pBI,
	OUT PBIG_INTEGER *pBI_Result)
{
	INT BIFirstIndex, BISecondIndex;
	UINT32 HBITS_Value, LBITS_Value, Temp1_Value, Temp2_Value, Carry32;
	UINT32 *Point_Of_S, *Point_Of_Result, *Point_Of_BI;
	UINT64 Result64_1, Result64_2, Carry64, TempValue64;
	PBIG_INTEGER pBI_Square_1 = NULL;
	PBIG_INTEGER pBI_Square_2 = NULL;

	if ((pBI == NULL) || (pBI->pIntegerArray == NULL)) {
		DEBUGPRINT("\tBigInteger_Square: the operand is NULL.\n");
		return;
	} /* End of if */

	/* The operand is zero */
	if  ((pBI->IntegerLength  == 1) && (pBI->pIntegerArray[0]  ==  0)) {
		BigInteger_AllocSize(pBI_Result, 1);
		goto output;
	} /* End of if */

	POOL_COUNTER_CHECK_BEGIN(expected_cnt[19]);
	GET_BI_INS_FROM_POOL(pBI_Square_1);
	GET_BI_INS_FROM_POOL(pBI_Square_2);
	BigInteger_AllocSize(pBI_Result, (pBI->IntegerLength * 2) + 20);
	BigInteger_AllocSize(&pBI_Square_1, (pBI->IntegerLength * 2) + 20);
	BigInteger_AllocSize(&pBI_Square_2, (pBI->IntegerLength * 2) + 20);
	/*
	 * Input: pBI = {a_0, a_1, a_2, a_3, ..., a_n}
	 * Step1. calculate a_0^2, a_1^2, a_2^2, a_3^2 ... a_n^2
	 */
	Point_Of_S = pBI_Square_2->pIntegerArray;

	for (BIFirstIndex = 0; BIFirstIndex < pBI->ArrayLength; BIFirstIndex++) {
		HBITS_Value = UINT32_HBITS(pBI->pIntegerArray[BIFirstIndex]);
		LBITS_Value = UINT32_LBITS(pBI->pIntegerArray[BIFirstIndex]);
		Temp1_Value = HBITS_Value * LBITS_Value;
		Temp2_Value = (Temp1_Value & 0x7fff) << 0x11;
		Point_Of_S[0] = (LBITS_Value * LBITS_Value) + Temp2_Value;
		Point_Of_S[1] = (HBITS_Value * HBITS_Value) + ((Temp1_Value >> 0xf) & 0x1ffff);

		if (Point_Of_S[0] < Temp2_Value)
			Point_Of_S[1] += 1;

		Point_Of_S += 2;
	} /* End of for */

	/*
	 * Step2. calculate a_0*{a_1, a_2, a_3, a_4, ..., a_n}
	 */
	Point_Of_BI = pBI->pIntegerArray;
	Point_Of_Result = (*pBI_Result)->pIntegerArray;
	Point_Of_Result[0] = 0;
	TempValue64 = (UINT64) Point_Of_BI[0];
	Point_Of_Result++;
	Carry64 = 0;

	for (BIFirstIndex = 1; BIFirstIndex < pBI->ArrayLength; BIFirstIndex++) {
		Result64_1 =  (UINT64) Point_Of_BI[BIFirstIndex] * TempValue64;
		Result64_1 += Carry64;
		Carry64 = (Result64_1 >> 32);
		Point_Of_Result[0] = (UINT32) (Result64_1 & 0xffffffffUL);
		Point_Of_Result++;
	} /* End of for */

	if (Carry64 > 0)
		Point_Of_Result[0] = (UINT32) (Carry64 & 0xffffffffUL);

	/*
	 * Step3. calculate
	 *           a_1*{a_2, a_3, a_4, ..., a_n}
	 *           a_2*{a_3, a_4, a_5, ..., a_n}
	 *           a_3*{a_4, a_5, a_6, ..., a_n}
	 *           a_4*{a_5, a_6, a_7, ..., a_n}
	 *           ...
	 *           a_n-1*{a_n}
	 */
	Point_Of_BI = pBI->pIntegerArray;

	for (BIFirstIndex = 1; BIFirstIndex < (pBI->ArrayLength - 1); BIFirstIndex++) {
		Point_Of_Result = (*pBI_Result)->pIntegerArray;
		Point_Of_Result += (BIFirstIndex * 2) + 1;
		TempValue64 = (UINT64) Point_Of_BI[BIFirstIndex];
		Carry64 = 0;

		for (BISecondIndex = (BIFirstIndex + 1); BISecondIndex < pBI->ArrayLength; BISecondIndex++) {
			Result64_1 = ((UINT64) Point_Of_Result[0]) + Carry64;
			Result64_2 = (UINT64) Point_Of_BI[BISecondIndex] * TempValue64;
			Carry64 = (Result64_1 >> 32);
			Result64_1 = (Result64_1 & 0xffffffffUL);
			Result64_1 = Result64_1 + Result64_2;
			Carry64 += (Result64_1 >> 32);
			Point_Of_Result[0] = (UINT32) (Result64_1 & 0xffffffffUL);
			Point_Of_Result++;
		} /* End of for */

		if (Carry64 > 0)
			Point_Of_Result[0] += (UINT32) (Carry64 & 0xffffffffUL);
	} /* End of for */

	BigInteger_ClearHighBits(*pBI_Result);
	BigInteger_Copy(*pBI_Result, &pBI_Square_1);
	Carry32 = 0;

	for (BIFirstIndex = 0; BIFirstIndex < pBI_Square_1->ArrayLength; BIFirstIndex++) {
		pBI_Square_1->pIntegerArray[BIFirstIndex] = (pBI_Square_1->pIntegerArray[BIFirstIndex] << 1) | Carry32;

		if (pBI_Square_1->pIntegerArray[BIFirstIndex] < (*pBI_Result)->pIntegerArray[BIFirstIndex])
			Carry32 = 1;
		else
			Carry32 = 0;
	} /* End of for */

	pBI_Square_1->pIntegerArray[BIFirstIndex] = Carry32;
	pBI_Square_1->IntegerLength++;
	pBI_Square_1->ArrayLength++;
	BigInteger_ClearHighBits(pBI_Square_1);
	BigInteger_Add(pBI_Square_1, pBI_Square_2, pBI_Result);
	release_temporal_usage_big_interger(&pBI_Square_1);
	release_temporal_usage_big_interger(&pBI_Square_2);
	POOL_COUNTER_CHECK_END(expected_cnt[19]);
output:
	(*pBI_Result)->Signed = 1;
	BigInteger_ClearHighBits(*pBI_Result);
} /* End of BigInteger_Square */


VOID BigInteger_Montgomery_MulMod(
	IN PBIG_INTEGER pBI_A,
	IN PBIG_INTEGER pBI_B,
	IN PBIG_INTEGER pBI_P,
	IN PBIG_INTEGER pBI_PInverse,
	IN UINT Bits_Of_R,
	OUT PBIG_INTEGER *pBI_Result)
{
	PBIG_INTEGER pBI_MulMod_1 = NULL;
	PBIG_INTEGER pBI_MulMod_2 = NULL;
	PBIG_INTEGER pBI_MulMod_3 = NULL;

	if (*pBI_Result == NULL)
		BigInteger_Init(pBI_Result);

	POOL_COUNTER_CHECK_BEGIN(expected_cnt[1]);
	GET_BI_INS_FROM_POOL(pBI_MulMod_1);
	GET_BI_INS_FROM_POOL(pBI_MulMod_2);
	GET_BI_INS_FROM_POOL(pBI_MulMod_3);
	BigInteger_Mul(pBI_A, pBI_B, &pBI_MulMod_1);
	BigInteger_Mul(pBI_MulMod_1, pBI_PInverse, &pBI_MulMod_2);
	BigInteger_Div2Exponent(pBI_MulMod_2, Bits_Of_R, pBI_Result, &pBI_MulMod_3);
	BigInteger_Mul(pBI_MulMod_3, pBI_P, pBI_Result);
	BigInteger_Add(pBI_MulMod_1, *pBI_Result, &pBI_MulMod_3);
	BigInteger_Div2Exponent(pBI_MulMod_3, Bits_Of_R,  &pBI_MulMod_2, pBI_Result);

	if (BigInteger_UnsignedCompare(pBI_MulMod_2, pBI_P) >= 0)
		BigInteger_Sub(pBI_MulMod_2, pBI_P, pBI_Result);
	else
		BigInteger_Copy(pBI_MulMod_2, pBI_Result);

	release_temporal_usage_big_interger(&pBI_MulMod_1);
	release_temporal_usage_big_interger(&pBI_MulMod_2);
	release_temporal_usage_big_interger(&pBI_MulMod_3);
	POOL_COUNTER_CHECK_END(expected_cnt[1]);
} /* End of BigInteger_Montgomery_MulMod */





VOID BigInteger_Montgomery_ExpMod(
	IN PBIG_INTEGER pBI_G,
	IN PBIG_INTEGER pBI_E,
	IN PBIG_INTEGER pBI_P,
	OUT PBIG_INTEGER *pBI_Result)
{
	UINT Bits_Of_P, Bits_Of_E, Bits_Of_R;
	UINT32 Index, AllocLength;
	PBIG_INTEGER pBI_Temp1 = NULL, pBI_Temp2 = NULL;
	PBIG_INTEGER pBI_A = NULL, pBI_X = NULL, pBI_R = NULL, pBI_1 = NULL, pBI_PInverse = NULL;
	/* For workaround */
	/*if (pBI_P->IntegerLength == 32) {
		BigInteger_Montgomery_ExpMod32(pBI_G, pBI_E, pBI_P, pBI_Result);
		return;
	}*/
	BigInteger_record_time_begin(&bi_op_ti_rec.exp_mod_op);
	POOL_COUNTER_CHECK_BEGIN(expected_cnt[18]);
	GET_BI_INS_FROM_POOL(pBI_Temp1);
	GET_BI_INS_FROM_POOL(pBI_Temp2);
	AllocLength = (pBI_G->IntegerLength + pBI_E->IntegerLength + pBI_P->IntegerLength) + 50;
	BigInteger_AllocSize(&pBI_Temp1, AllocLength);
	BigInteger_AllocSize(&pBI_Temp2, AllocLength);

	/* -- For spicial case --- */
	if ((pBI_G->IntegerLength == 1) && (pBI_G->pIntegerArray[0] == 0)) {
		BigInteger_Bin2BI(Value_0, 1, pBI_Result);
		goto memory_free;
	}

	if ((pBI_G->IntegerLength == 1) && (pBI_G->pIntegerArray[0] == 1)) {
		BigInteger_Div(pBI_G, pBI_P, &pBI_Temp1, pBI_Result);
		goto memory_free;
	}

	if ((pBI_E->IntegerLength == 1) && (pBI_E->pIntegerArray[0] == 1)) {
		BigInteger_Div(pBI_G, pBI_P, &pBI_Temp1, pBI_Result);
		goto memory_free;
	}

	if ((pBI_E->IntegerLength == 1) && (pBI_E->pIntegerArray[0] == 2)) {
		BigInteger_Square2(pBI_G, &pBI_Temp1);
		BigInteger_Div(pBI_Temp1, pBI_P, &pBI_Temp2, pBI_Result);
		goto memory_free;
	}

	GET_BI_INS_FROM_POOL(pBI_A);
	GET_BI_INS_FROM_POOL(pBI_X);
	GET_BI_INS_FROM_POOL(pBI_R);
	GET_BI_INS_FROM_POOL(pBI_PInverse);
	GET_BI_INS_FROM_POOL(pBI_1);
	/* ---Main algorithm --- */
	BigInteger_Init(&pBI_A);
	BigInteger_Init(&pBI_X);
	BigInteger_Init(&pBI_R);
	BigInteger_Init(&pBI_PInverse);
	BigInteger_Bin2BI(Value_1, 1, &pBI_1);
	BigInteger_AllocSize(&pBI_X, AllocLength);
#ifdef WSC_INCLUDED
	BigInteger_Bin2BI(WPS_DH_P_VALUE, 192, &pBI_Temp1);

	if (NdisCmpMemory(pBI_P->pIntegerArray, pBI_Temp1->pIntegerArray, pBI_P->IntegerLength) == 0) {
		BigInteger_Bin2BI(WPS_DH_X_VALUE, 184, &pBI_X);
		BigInteger_Bin2BI(WPS_DH_R_VALUE, 193, &pBI_R);
		BigInteger_Bin2BI(WPS_DH_PInverse_VALUE, 192, &pBI_PInverse);
	} else
#endif
	{
		UINT8 *pRValue = NULL;
		BigInteger_BitsOfBI(pBI_P, &Bits_Of_P);

		if ((Bits_Of_P % 8) == 0)
			AllocLength = pBI_P->IntegerLength + 1;
		else
			AllocLength = pBI_P->IntegerLength;

		os_alloc_mem(NULL, (UCHAR **)&pRValue, sizeof(UINT8)*AllocLength);
		NdisZeroMemory(pRValue, sizeof(UINT8)*AllocLength);
		pRValue[0] = (UINT8) (1 << (Bits_Of_P & 0x7));
		BigInteger_Bin2BI(pRValue, AllocLength , &pBI_R);
		os_free_mem(pRValue);
		BigInteger_Inverse(pBI_R, pBI_P, &pBI_Temp1, &pBI_PInverse); /* Calculate the inverse of P */
		BigInteger_Div(pBI_R, pBI_P, &pBI_Temp2, &pBI_X); /* X = 1*R (mod P) */
	} /* End of if */

	/* A = G*R (mod P) */
	BigInteger_Mul(pBI_G, pBI_R, &pBI_Temp1);
	BigInteger_Div(pBI_Temp1, pBI_P, &pBI_Temp2, &pBI_A);
	/* Calculate (G^E) * R mod P */
	BigInteger_BitsOfBI(pBI_E, &Bits_Of_E); /* Calculate the bits of E */
	BigInteger_BitsOfBI(pBI_R, &Bits_Of_R); /* Calculate the bits of R */

	for (Index = Bits_Of_E; Index > 0; Index--) {
		BigInteger_Montgomery_MulMod(pBI_X, pBI_X, pBI_P, pBI_PInverse, Bits_Of_R, &pBI_Temp1);

		if (BigInteger_GetBitValue(pBI_E, Index))
			BigInteger_Montgomery_MulMod(pBI_A, pBI_Temp1, pBI_P, pBI_PInverse, Bits_Of_R, &pBI_X);
		else
			BigInteger_Copy(pBI_Temp1, &pBI_X); /* End of if */
	} /* End of for */

	BigInteger_Montgomery_MulMod(pBI_X, pBI_1, pBI_P, pBI_PInverse, Bits_Of_R, pBI_Result);
	release_temporal_usage_big_interger(&pBI_A);
	release_temporal_usage_big_interger(&pBI_X);
	release_temporal_usage_big_interger(&pBI_R);
	release_temporal_usage_big_interger(&pBI_PInverse);
	release_temporal_usage_big_interger(&pBI_1);
memory_free:
	release_temporal_usage_big_interger(&pBI_Temp1);
	release_temporal_usage_big_interger(&pBI_Temp2);
	POOL_COUNTER_CHECK_END(expected_cnt[18]);
	BigInteger_record_time_end(&bi_op_ti_rec.exp_mod_op);
} /* End of BigInteger_Montgomery_ExpMod */


VOID BigInteger_Montgomery_MulMod_with_mont(
	IN PBIG_INTEGER pBI_A,
	IN PBIG_INTEGER pBI_B,
	IN PBIG_INTEGER pBI_P,
	IN MONT_STRUC * mont,
	OUT PBIG_INTEGER *pBI_Result)
{
	PBIG_INTEGER pBI_MulMod_1 = NULL;
	PBIG_INTEGER pBI_MulMod_2 = NULL;
	PBIG_INTEGER pBI_MulMod_3 = NULL;

	if (*pBI_Result == NULL)
		BigInteger_Init(pBI_Result);

	POOL_COUNTER_CHECK_BEGIN(expected_cnt[20]);
	GET_BI_INS_FROM_POOL(pBI_MulMod_1);
	GET_BI_INS_FROM_POOL(pBI_MulMod_2);
	GET_BI_INS_FROM_POOL(pBI_MulMod_3);
	BigInteger_Mul(pBI_A, pBI_B, &pBI_MulMod_1);
	BigInteger_Mul(pBI_MulMod_1, mont->pBI_PInverse, &pBI_MulMod_2);
	BigInteger_Div2Exponent(pBI_MulMod_2, mont->Bits_Of_R, pBI_Result, &pBI_MulMod_3);
	BigInteger_Mul(pBI_MulMod_3, pBI_P, pBI_Result);
	BigInteger_Add(pBI_MulMod_1, *pBI_Result, &pBI_MulMod_3);
	BigInteger_Div2Exponent(pBI_MulMod_3, mont->Bits_Of_R,  &pBI_MulMod_2, pBI_Result);

	if (BigInteger_UnsignedCompare(pBI_MulMod_2, pBI_P) >= 0)
		BigInteger_Sub(pBI_MulMod_2, pBI_P, pBI_Result);
	else
		BigInteger_Copy(pBI_MulMod_2, pBI_Result);

	release_temporal_usage_big_interger(&pBI_MulMod_1);
	release_temporal_usage_big_interger(&pBI_MulMod_2);
	release_temporal_usage_big_interger(&pBI_MulMod_3);
	POOL_COUNTER_CHECK_END(expected_cnt[20]);
} /* End of BigInteger_Montgomery_MulMod */


VOID BigInteger_Montgomery_ExpMod_with_mont(
	IN PBIG_INTEGER pBI_G,
	IN PBIG_INTEGER pBI_E,
	IN PBIG_INTEGER pBI_P,
	IN MONT_STRUC * mont,
	OUT PBIG_INTEGER *pBI_Result)
{
	UINT Bits_Of_P, Bits_Of_E, Bits_Of_R;
	UINT32 Index, AllocLength;
	PBIG_INTEGER pBI_Temp1 = NULL, pBI_Temp2 = NULL;
	PBIG_INTEGER pBI_A = NULL, pBI_X = NULL, pBI_R = NULL, pBI_1 = NULL, pBI_PInverse = NULL;
	/* For workaround */
	/*if (pBI_P->IntegerLength == 32) {
		BigInteger_Montgomery_ExpMod32(pBI_G, pBI_E, pBI_P, pBI_Result);
		return;
	}*/
	BigInteger_record_time_begin(&bi_op_ti_rec.exp_mod_op);
	POOL_COUNTER_CHECK_BEGIN(expected_cnt[21]);
	GET_BI_INS_FROM_POOL(pBI_Temp1);
	GET_BI_INS_FROM_POOL(pBI_Temp2);
	AllocLength = (pBI_G->IntegerLength + pBI_E->IntegerLength + pBI_P->IntegerLength) + 50;
	BigInteger_AllocSize(&pBI_Temp1, AllocLength);
	BigInteger_AllocSize(&pBI_Temp2, AllocLength);

	/* -- For spicial case --- */
	if ((pBI_G->IntegerLength == 1) && (pBI_G->pIntegerArray[0] == 0)) {
		BigInteger_Bin2BI(Value_0, 1, pBI_Result);
		goto memory_free;
	}

	if ((pBI_G->IntegerLength == 1) && (pBI_G->pIntegerArray[0] == 1)) {
		BigInteger_Div(pBI_G, pBI_P, &pBI_Temp1, pBI_Result);
		goto memory_free;
	}

	if ((pBI_E->IntegerLength == 1) && (pBI_E->pIntegerArray[0] == 1)) {
		BigInteger_Div(pBI_G, pBI_P, &pBI_Temp1, pBI_Result);
		goto memory_free;
	}

	if ((pBI_E->IntegerLength == 1) && (pBI_E->pIntegerArray[0] == 2)) {
		BigInteger_Square2(pBI_G, &pBI_Temp1);
		BigInteger_Div(pBI_Temp1, pBI_P, &pBI_Temp2, pBI_Result);
		goto memory_free;
	}

	GET_BI_INS_FROM_POOL(pBI_A);
	GET_BI_INS_FROM_POOL(pBI_X);
	GET_BI_INS_FROM_POOL(pBI_R);
	GET_BI_INS_FROM_POOL(pBI_PInverse);
	GET_BI_INS_FROM_POOL(pBI_1);
	/* ---Main algorithm --- */
	BigInteger_Init(&pBI_A);
	BigInteger_Init(&pBI_X);
	BigInteger_Init(&pBI_R);
	BigInteger_Init(&pBI_PInverse);
	BigInteger_Bin2BI(Value_1, 1, &pBI_1);
	BigInteger_AllocSize(&pBI_X, AllocLength);
#ifdef WSC_INCLUDED
	BigInteger_Bin2BI(WPS_DH_P_VALUE, 192, &pBI_Temp1);
#endif
	if (mont != NULL) {
		BigInteger_Copy(mont->pBI_X, &pBI_X);
		BigInteger_Copy(mont->pBI_R, &pBI_R);
		BigInteger_Copy(mont->pBI_PInverse, &pBI_PInverse);
		Bits_Of_R = mont->Bits_Of_R;
	}

#ifdef WSC_INCLUDED
	if (NdisCmpMemory(pBI_P->pIntegerArray, pBI_Temp1->pIntegerArray, pBI_P->IntegerLength) == 0) {
		BigInteger_Bin2BI(WPS_DH_X_VALUE, 184, &pBI_X);
		BigInteger_Bin2BI(WPS_DH_R_VALUE, 193, &pBI_R);
		BigInteger_Bin2BI(WPS_DH_PInverse_VALUE, 192, &pBI_PInverse);
		BigInteger_BitsOfBI(pBI_R, &Bits_Of_R); /* Calculate the bits of R */
	} else
#endif
	{
		UINT8 *pRValue = NULL;
		BigInteger_BitsOfBI(pBI_P, &Bits_Of_P);

		if ((Bits_Of_P % 8) == 0)
			AllocLength = pBI_P->IntegerLength + 1;
		else
			AllocLength = pBI_P->IntegerLength;

		os_alloc_mem(NULL, (UCHAR **)&pRValue, sizeof(UINT8)*AllocLength);
		NdisZeroMemory(pRValue, sizeof(UINT8)*AllocLength);
		pRValue[0] = (UINT8) (1 << (Bits_Of_P & 0x7));
		BigInteger_Bin2BI(pRValue, AllocLength , &pBI_R);
		os_free_mem(pRValue);
		BigInteger_Inverse(pBI_R, pBI_P, &pBI_Temp1, &pBI_PInverse); /* Calculate the inverse of P */
		BigInteger_Div(pBI_R, pBI_P, &pBI_Temp2, &pBI_X); /* X = 1*R (mod P) */
		BigInteger_BitsOfBI(pBI_R, &Bits_Of_R); /* Calculate the bits of R */
	} /* End of if */

	/* A = G*R (mod P) */
	BigInteger_Mul(pBI_G, pBI_R, &pBI_Temp1);
	BigInteger_Div(pBI_Temp1, pBI_P, &pBI_Temp2, &pBI_A);
	/* Calculate (G^E) * R mod P */
	BigInteger_BitsOfBI(pBI_E, &Bits_Of_E); /* Calculate the bits of E */

	for (Index = Bits_Of_E; Index > 0; Index--) {
		BigInteger_Montgomery_MulMod_with_mont(pBI_X, pBI_X, pBI_P, mont, &pBI_Temp1);

		if (BigInteger_GetBitValue(pBI_E, Index))
			BigInteger_Montgomery_MulMod_with_mont(pBI_A, pBI_Temp1, pBI_P, mont, &pBI_X);
		else
			BigInteger_Copy(pBI_Temp1, &pBI_X); /* End of if */
	} /* End of for */

	BigInteger_Montgomery_MulMod_with_mont(pBI_X, pBI_1, pBI_P, mont, pBI_Result);
	release_temporal_usage_big_interger(&pBI_A);
	release_temporal_usage_big_interger(&pBI_X);
	release_temporal_usage_big_interger(&pBI_R);
	release_temporal_usage_big_interger(&pBI_PInverse);
	release_temporal_usage_big_interger(&pBI_1);
memory_free:
	release_temporal_usage_big_interger(&pBI_Temp1);
	release_temporal_usage_big_interger(&pBI_Temp2);
	POOL_COUNTER_CHECK_END(expected_cnt[21]);
	BigInteger_record_time_end(&bi_op_ti_rec.exp_mod_op);
} /* End of BigInteger_Montgomery_ExpMod */



#ifdef DOT11_SAE_SUPPORT
VOID BigInteger_Mod(
	IN PBIG_INTEGER pFirstOperand,
	IN PBIG_INTEGER pSecondOperand,
	OUT PBIG_INTEGER *pBI_Remainder)
{
	INT CompareResult;
	INT Index, MulIndex, ComputeSize;
	UINT32 MulLoopStart, MulLoopEnd;
	UINT AllocLength, ArrayIndex, ShiftIndex;
	UINT8 SecondHighByte;
	PBIG_INTEGER pTempBI = NULL;
	PBIG_INTEGER pTempBI2 = NULL;
	PBIG_INTEGER pMulBI = NULL;
#ifdef BI_TIME_REC
	struct timeval t1;
	struct timeval t2;

	if (is_time_rec) {
		do_gettimeofday(&t1);
		is_mod_alloc_rec = TRUE;
	}

#endif

	if  ((pFirstOperand == NULL) || (pFirstOperand->pIntegerArray == NULL)
		 || (pSecondOperand == NULL) || (pSecondOperand->pIntegerArray == NULL)) {
		DEBUGPRINT("BigInteger_Div: first or second operand is NULL.\n");
		return;
	} /* End of if */

	/* The second operand is zero */
	if ((pSecondOperand->IntegerLength == 1) && (pSecondOperand->pIntegerArray[0] == 0)) {
		DEBUGPRINT("BigInteger_Div: second operand is zero.\n");
		return;
	} /* End of if */

	POOL_COUNTER_CHECK_BEGIN(expected_cnt[17]);
	GET_BI_INS_FROM_POOL(pTempBI);
	GET_BI_INS_FROM_POOL(pTempBI2);
	GET_BI_INS_FROM_POOL(pMulBI);

	if (*pBI_Remainder == NULL)
		BigInteger_Init(pBI_Remainder);

	/* The second operand is one */
	if  ((pSecondOperand->IntegerLength  == 1) && (pSecondOperand->pIntegerArray[0]  == 1)) {
		BigInteger_Bin2BI(Value_0, 1, pBI_Remainder);
		goto output;
	} /* End of if */

	CompareResult = BigInteger_UnsignedCompare(pFirstOperand, pSecondOperand);

	if (CompareResult == 0) {
		BigInteger_Bin2BI(Value_0, 1, pBI_Remainder);
		goto output;
	} else if (CompareResult == -1) {
		BigInteger_Copy(pFirstOperand, pBI_Remainder);
		goto output;
	} /* End of if */

	BigInteger_AllocSize(pBI_Remainder, pSecondOperand->IntegerLength);
	AllocLength = (UINT) (pFirstOperand->IntegerLength << 1);
	BigInteger_AllocSize(&pTempBI, AllocLength);
	BigInteger_AllocSize(&pTempBI2, AllocLength);
	BigInteger_AllocSize(&pMulBI, AllocLength);
	BigInteger_Copy(pFirstOperand, pBI_Remainder);
	SecondHighByte = (BigInteger_GetByteValue(pSecondOperand, pSecondOperand->IntegerLength) & 0xFF);
	ComputeSize = (INT) pFirstOperand->IntegerLength - pSecondOperand->IntegerLength + 1;

	for (Index = (INT) ComputeSize; Index >= 0; Index--) {
		if (BigInteger_UnsignedCompare(*pBI_Remainder, pSecondOperand) == -1)
			break;

		if (((pSecondOperand->IntegerLength + Index) - (*pBI_Remainder)->IntegerLength) <= 1) {
			BigInteger_AllocSize(&pMulBI, Index + 1);
			ArrayIndex = 0;

			if (Index > 0)
				ArrayIndex = (UINT) (Index - 1) >> 2;

			ShiftIndex = (Index & 0x03);

			if (ShiftIndex == 0)
				ShiftIndex = 4;

			ShiftIndex--;
			MulLoopStart = (BigInteger_GetByteValue((*pBI_Remainder), pFirstOperand->IntegerLength + Index - ComputeSize + 1) & 0xFF) << 8;
			MulLoopStart = MulLoopStart | (BigInteger_GetByteValue((*pBI_Remainder), pFirstOperand->IntegerLength + Index - ComputeSize) & 0xFF);

			if (MulLoopStart < (UINT32) SecondHighByte)
				continue;

			MulLoopEnd = (MulLoopStart / (UINT32) SecondHighByte) + 1;
			MulLoopStart = MulLoopStart / (UINT32) (SecondHighByte + 1);

			for (MulIndex = (INT) MulLoopStart; MulIndex <= MulLoopEnd; MulIndex++) { /* 0xFFFF / 0x01 = 0xFFFF */
				NdisZeroMemory(pMulBI->pIntegerArray, pMulBI->AllocSize);

				if ((MulIndex > 0xff) && (ShiftIndex == 3)) {
					pMulBI->pIntegerArray[ArrayIndex] = ((UINT) (MulIndex & 0xff) << (8 * ShiftIndex));
					pMulBI->pIntegerArray[ArrayIndex + 1] = (UINT) (MulIndex >> 8) & 0xff;
				} else
					pMulBI->pIntegerArray[ArrayIndex] = ((UINT) (MulIndex) << (8 * ShiftIndex));

				BigInteger_Mul(pSecondOperand, pMulBI, &pTempBI);
				CompareResult = BigInteger_UnsignedCompare(*pBI_Remainder, pTempBI);

				if (CompareResult < 1) {
					if (CompareResult == -1) {
						MulIndex = MulIndex - 1;
						NdisZeroMemory(pMulBI->pIntegerArray, pMulBI->AllocSize);

						if ((MulIndex > 0xff) && (ShiftIndex == 3)) {
							pMulBI->pIntegerArray[ArrayIndex] = ((UINT) (MulIndex & 0xff) << (8 * ShiftIndex));
							pMulBI->pIntegerArray[ArrayIndex + 1] = (UINT) (MulIndex >> 8) & 0xff;
						} else
							pMulBI->pIntegerArray[ArrayIndex] = ((UINT) (MulIndex) << (8 * ShiftIndex));

						BigInteger_Mul(pSecondOperand, pMulBI, &pTempBI);
					}

					BigInteger_Sub(*pBI_Remainder, pTempBI, &pTempBI2);
					BigInteger_Copy(pTempBI2, pBI_Remainder);
					break;
				} /* End of if */
			} /* End of for */
		} /* End of if */
	} /* End of for */

output:
	(*pBI_Remainder)->Signed = pFirstOperand->Signed * pSecondOperand->Signed;
	BigInteger_ClearHighBits(*pBI_Remainder);
	release_temporal_usage_big_interger(&pTempBI);
	release_temporal_usage_big_interger(&pTempBI2);
	release_temporal_usage_big_interger(&pMulBI);
	POOL_COUNTER_CHECK_END(expected_cnt[17]);
#ifdef BI_TIME_REC

	if (is_time_rec) {
		ULONG time_interval = 0;
		do_gettimeofday(&t2);

		if (t1.tv_usec > t2.tv_usec)
			time_interval = (t2.tv_sec - t1.tv_sec) * 1000000 - t1.tv_usec + t2.tv_usec;
		else
			time_interval = t2.tv_usec - t1.tv_usec;

		BigInteger_record_time_ns(&bi_op_ti_rec.mod_op, time_interval);
		is_mod_alloc_rec = FALSE;
	}

#endif
}

VOID BigInteger_Mod_Mul(
	IN PBIG_INTEGER pFirstOperand,
	IN PBIG_INTEGER pSecondOperand,
	IN PBIG_INTEGER pBI_P,
	OUT PBIG_INTEGER *pBI_Result)
{
	BIG_INTEGER *tmp = NULL;
	POOL_COUNTER_CHECK_BEGIN(expected_cnt[8]);
	GET_BI_INS_FROM_POOL(tmp);
	BigInteger_Init(&tmp);
	BigInteger_Mul(pFirstOperand, pSecondOperand, &tmp);
	BigInteger_Mod(tmp, pBI_P, pBI_Result);
	release_temporal_usage_big_interger(&tmp);
	POOL_COUNTER_CHECK_END(expected_cnt[8]);
}

UCHAR BigInteger_is_zero(
	IN PBIG_INTEGER pBI)
{
	BigInteger_ClearHighBits(pBI);

	if (pBI->IntegerLength == 1
		&& pBI->Signed == 1
		&& pBI->pIntegerArray[0] == 0)
		return TRUE;
	else
		return FALSE;
}

UCHAR BigInteger_is_one(
	IN PBIG_INTEGER pBI)
{
	BigInteger_ClearHighBits(pBI);

	if (pBI->IntegerLength == 1
		&& pBI->Signed == 1
		&& pBI->pIntegerArray[0] == 1)
		return TRUE;
	else
		return FALSE;
}

UCHAR BigInteger_is_odd(
	IN PBIG_INTEGER pBI)
{
	if (!pBI)
		return FALSE;

	if (pBI->IntegerLength == 0)
		return FALSE;

	if (pBI->pIntegerArray[0] & BIT0)
		return TRUE;
	else
		return FALSE;
}


VOID BigInteger_Mod_Square(
	IN PBIG_INTEGER pFirstOperand,
	IN PBIG_INTEGER pBI_P,
	OUT PBIG_INTEGER *pBI_Result)
{
	BigInteger_Mod_Mul(pFirstOperand, pFirstOperand, pBI_P, pBI_Result);
}

VOID BigInteger_Mod_Add(
	IN PBIG_INTEGER pFirstOperand,
	IN PBIG_INTEGER pSecondOperand,
	IN PBIG_INTEGER pBI_P,
	OUT PBIG_INTEGER *pBI_Result)
{
	BIG_INTEGER *tmp = NULL;
	POOL_COUNTER_CHECK_BEGIN(expected_cnt[2]);
	GET_BI_INS_FROM_POOL(tmp);
	BigInteger_Add(pFirstOperand, pSecondOperand, &tmp);
	BigInteger_Mod(tmp, pBI_P, pBI_Result);
	release_temporal_usage_big_interger(&tmp);
	POOL_COUNTER_CHECK_END(expected_cnt[2]);
}


VOID BigInteger_Mod_Sub(
	IN PBIG_INTEGER pFirstOperand,
	IN PBIG_INTEGER pSecondOperand,
	IN PBIG_INTEGER pBI_P,
	OUT PBIG_INTEGER *pBI_Result)
{
	BIG_INTEGER *tmp = NULL;
	BIG_INTEGER *res = NULL;
	POOL_COUNTER_CHECK_BEGIN(expected_cnt[3]);
	GET_BI_INS_FROM_POOL(tmp);
	GET_BI_INS_FROM_POOL(res);
	BigInteger_Sub(pFirstOperand, pSecondOperand, &tmp);

	if (tmp->Signed == 1)
		BigInteger_Mod(tmp, pBI_P, &res);
	else {
		BIG_INTEGER *tmp2 = NULL;
		GET_BI_INS_FROM_POOL(tmp2);
		tmp->Signed = 1;
		BigInteger_Mod(tmp, pBI_P, &tmp2);
		BigInteger_Sub(pBI_P, tmp2, &res);
		release_temporal_usage_big_interger(&tmp2);
	}

	BigInteger_Copy(res, pBI_Result);
	release_temporal_usage_big_interger(&tmp);
	release_temporal_usage_big_interger(&res);
	POOL_COUNTER_CHECK_END(expected_cnt[3]);
}


VOID BigInteger_Mod_Add_quick(
	IN PBIG_INTEGER pFirstOperand,
	IN PBIG_INTEGER pSecondOperand,
	IN PBIG_INTEGER pBI_P,
	OUT PBIG_INTEGER *pBI_Result)
{
	BIG_INTEGER *tmp = NULL;
	POOL_COUNTER_CHECK_BEGIN(expected_cnt[4]);
	GET_BI_INS_FROM_POOL(tmp);
	BigInteger_Add(pFirstOperand, pSecondOperand, &tmp);

	if (BigInteger_UnsignedCompare(tmp, pBI_P) >= 0)
		BigInteger_Sub(tmp, pBI_P, pBI_Result);
	else
		BigInteger_Copy(tmp, pBI_Result);

	release_temporal_usage_big_interger(&tmp);
	POOL_COUNTER_CHECK_END(expected_cnt[4]);
}


VOID BigInteger_Mod_Sub_quick(
	IN PBIG_INTEGER pFirstOperand,
	IN PBIG_INTEGER pSecondOperand,
	IN PBIG_INTEGER pBI_P,
	OUT PBIG_INTEGER *pBI_Result)
{
	BIG_INTEGER *tmp = NULL;
	POOL_COUNTER_CHECK_BEGIN(expected_cnt[5]);
	GET_BI_INS_FROM_POOL(tmp);
	BigInteger_Sub(pFirstOperand, pSecondOperand, &tmp);

	if (tmp->Signed == -1)
		BigInteger_Add(tmp, pBI_P, pBI_Result);
	else
		BigInteger_Copy(tmp, pBI_Result);

	release_temporal_usage_big_interger(&tmp);
	POOL_COUNTER_CHECK_END(expected_cnt[5]);
}


VOID BigInteger_Mod_Div(
	IN PBIG_INTEGER pFirstOperand,
	IN PBIG_INTEGER pSecondOperand,
	IN PBIG_INTEGER pBI_P,
	OUT PBIG_INTEGER *pBI_Result)
{
	BIG_INTEGER *tmp = NULL;
	POOL_COUNTER_CHECK_BEGIN(expected_cnt[6]);
	GET_BI_INS_FROM_POOL(tmp);
	BigInteger_record_time_begin(&bi_op_ti_rec.div_mod_op);
	BigInteger_Mod_Mul_Inverse(pSecondOperand, pBI_P, &tmp);

	if (tmp == NULL) {
		release_temporal_usage_big_interger(pBI_Result);
		DEBUGPRINT("BigInteger_Mod_Mul_Inverse fail!!\n");
		return;
	}

	BigInteger_Mod_Mul(pFirstOperand, tmp, pBI_P, &tmp);
	BigInteger_Copy(tmp, pBI_Result);
	release_temporal_usage_big_interger(&tmp);
	POOL_COUNTER_CHECK_END(expected_cnt[6]);
	BigInteger_record_time_end(&bi_op_ti_rec.div_mod_op);
}



/* Tonelli¡VShanks algorithm*/
/* reference: https://en.wikipedia.org/wiki/Tonelli%E2%80%93Shanks_algorithm */
/* pBI_P must be prime */
VOID BigInteger_Mod_Sqrt(
	IN PBIG_INTEGER pFirstOperand,
	IN PBIG_INTEGER pBI_P,
	IN MONT_STRUC * mont,
	OUT PBIG_INTEGER *pBI_Result)
{
	BIG_INTEGER *p = NULL;
	BIG_INTEGER *Q = NULL;
	BIG_INTEGER *R = NULL;
	BIG_INTEGER *t = NULL;
	BIG_INTEGER *c = NULL;
	BIG_INTEGER *z = NULL;
	UINT32 S = 0;
	UINT32 M = 0;
	UINT32 i;
	BIG_INTEGER *pone = NULL;
	BIG_INTEGER *tmp = NULL;
	BIG_INTEGER *b = NULL;
	MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("==> %s()\n", __func__));
	BigInteger_record_time_begin(&bi_op_ti_rec.sqrt_mod_op);

	if (BigInteger_is_quadratic_residue(pFirstOperand, pBI_P, mont) == FALSE) {
		/* MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: is_not_quadratic_residue\n", __FUNCTION__)); */
		release_temporal_usage_big_interger(pBI_Result);
		return;
	}

	POOL_COUNTER_CHECK_BEGIN(expected_cnt[9]);
	GET_BI_INS_FROM_POOL(p);
	GET_BI_INS_FROM_POOL(Q);
	GET_BI_INS_FROM_POOL(R);
	GET_BI_INS_FROM_POOL(t);
	GET_BI_INS_FROM_POOL(c);
	GET_BI_INS_FROM_POOL(z);
	GET_BI_INS_FROM_POOL(tmp);
	GET_BI_INS_FROM_POOL(b);
	/* fix me */
	GET_BI_INS_FROM_POOL(pone);
	/* MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\x1b[31m%s: is_quadratic_residue\x1b[m\n", __FUNCTION__)); */
	/*DEBUGPRINT("pFirstOperand\n");
	BigInteger_Print(pFirstOperand);*/
	BigInteger_Copy(pBI_P, &p);
	BigInteger_minus_one(p);

	/* p - 1 = Q * 2^S */
	while (BigInteger_is_odd(p) == FALSE) {
		S++;
		BigInteger_Shift_Right1(p, &p);
	}

	BigInteger_Copy(p, &Q);
	/* R = n^((Q+1)/2) (mod P) */
	BigInteger_Init(&R);
	BigInteger_Copy(Q, &p);
	BigInteger_plus_one(p);
	BigInteger_Shift_Right1(p, &p);
	BigInteger_Montgomery_ExpMod_with_mont(pFirstOperand, p, pBI_P, mont, &R);
	/*BigInteger_Mod_Mul(R, R, pBI_P, &tmp);
	DEBUGPRINT("R:\n");
	BigInteger_Print(R);
	DEBUGPRINT("R^2:\n");
	BigInteger_Print(tmp);*/
	/* t = n^Q (mod P) */
	BigInteger_Init(&t);
	BigInteger_Montgomery_ExpMod_with_mont(pFirstOperand, Q, pBI_P, mont, &t); /* ellis */
	/*DEBUGPRINT("pFirstOperand:\n");
	BigInteger_Print(pFirstOperand);
	DEBUGPRINT("Q:\n");
	BigInteger_Print(Q);
	DEBUGPRINT("t:\n");
	BigInteger_Print(t);
	BigInteger_Mod_Mul(t, pFirstOperand, pBI_P, &tmp);
	DEBUGPRINT("t*n:\n");
	BigInteger_Print(tmp);*/
	/* M = S */
	M = S;

	if (BigInteger_is_one(t))
		goto Find;

	/* Find the first quadratic non-residue z by brute-force search */
	BigInteger_Init(&c);
	BigInteger_Init(&z);
	BigInteger_DWtoBI(1, &z);
	BigInteger_Copy(pBI_P, &p);
	BigInteger_minus_one(p);
	BigInteger_Shift_Right1(p, &p);

	do {
		BigInteger_plus_one(z);
		BigInteger_Montgomery_ExpMod_with_mont(z, p, pBI_P, mont, &tmp);
		BigInteger_plus_one(tmp);
	} while (BigInteger_UnsignedCompare(tmp, pBI_P) != 0);

	BigInteger_Montgomery_ExpMod_with_mont(z, Q, pBI_P, mont, &c);
	BigInteger_DWtoBI(1, &pone);

	while (BigInteger_is_one(t) == FALSE) {
		BigInteger_Copy(t, &tmp);

		/*  find the lowest i, 0 < i <  M, such that t^(2^i) = 1 (mod P) */
		for (i = 1; i < M; i++) {
			BigInteger_Mod_Mul(tmp, tmp, pBI_P, &tmp);

			if (BigInteger_is_one(tmp))
				break;
		}

		if (i == M) {
			DEBUGPRINT("\x1b[33msearch fail\x1b[m\n");
			BigInteger_Free(pBI_Result);
			goto Free;
		}

		/* b = c^2^(M-i-1) (mod P)*/
		BigInteger_Shift_Left(pone, M - i - 1, &tmp);
		BigInteger_Montgomery_ExpMod_with_mont(c, tmp, pBI_P, mont, &b);
		/* R = R*b (mod P)*/
		BigInteger_Mod_Mul(R, b, pBI_P, &R);
		/* t = t*b^2 (mod P), c = b^2 (mod P) */
		BigInteger_Mod_Mul(b, b, pBI_P, &c);
		BigInteger_Mod_Mul(t, c, pBI_P, &t);
		/* M = i */
		M = i;
	}

Find:
	BigInteger_Copy(pFirstOperand, &p);
	BigInteger_Copy(R, pBI_Result);
Free:
	release_temporal_usage_big_interger(&p);
	release_temporal_usage_big_interger(&Q);
	release_temporal_usage_big_interger(&R);
	release_temporal_usage_big_interger(&t);
	release_temporal_usage_big_interger(&pone);
	release_temporal_usage_big_interger(&tmp);
	release_temporal_usage_big_interger(&b);
	release_temporal_usage_big_interger(&c);
	release_temporal_usage_big_interger(&z);
	POOL_COUNTER_CHECK_END(expected_cnt[9]);
	BigInteger_record_time_end(&bi_op_ti_rec.sqrt_mod_op);
}

VOID BigInteger_Euclidean_Div(
	IN PBIG_INTEGER pFirstOperand,
	IN PBIG_INTEGER pSecondOperand,
	IN PBIG_INTEGER prev_u,
	IN PBIG_INTEGER prev_v,
	INOUT PBIG_INTEGER curr_u,
	INOUT PBIG_INTEGER curr_v)
{
	PBIG_INTEGER div_result = NULL;
	PBIG_INTEGER div_Remainder = NULL;
	PBIG_INTEGER tmp2 = NULL;
	PBIG_INTEGER tmp = NULL;
	MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("==> %s()\n", __func__));
	POOL_COUNTER_CHECK_BEGIN(expected_cnt[13]);
	GET_BI_INS_FROM_POOL(div_result);
	GET_BI_INS_FROM_POOL(div_Remainder);
	GET_BI_INS_FROM_POOL(tmp);
	GET_BI_INS_FROM_POOL(tmp2);

	while (!BigInteger_is_one(pSecondOperand)
		   && !BigInteger_is_zero(pSecondOperand)) {
		BigInteger_Div(pFirstOperand, pSecondOperand, &div_result, &div_Remainder); /* pFirstOperand = pSecondOperand * div_result + div_Remainder */
		/* new_u = prev_u - curr_u * div_result */
		BigInteger_Mul(curr_u, div_result, &tmp);
		BigInteger_Sub(prev_u, tmp, &tmp2);
		BigInteger_Copy(curr_u, &prev_u);
		BigInteger_Copy(tmp2, &curr_u);
		/* new_v = prev_v - curr_v * div_result */
		BigInteger_Mul(curr_v, div_result, &tmp);
		BigInteger_Sub(prev_v, tmp, &tmp2);
		BigInteger_Copy(curr_v, &prev_v);
		BigInteger_Copy(tmp2, &curr_v);
		BigInteger_Copy(pSecondOperand, &pFirstOperand);
		BigInteger_Copy(div_Remainder, &pSecondOperand);
	};

	release_temporal_usage_big_interger(&div_result);

	release_temporal_usage_big_interger(&div_Remainder);

	release_temporal_usage_big_interger(&tmp2);

	release_temporal_usage_big_interger(&tmp);

	POOL_COUNTER_CHECK_END(expected_cnt[13]);
}

/*
UCHAR test[7] = {
	0x72, 0xf8, 0x45, 0x05, 0x69, 0xb0, 0x4d
};*/

UCHAR test[192] = {
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xC9, 0x0F, 0xDA, 0xA2, 0x21, 0x68, 0xC2, 0x34,
	0xC4, 0xC6, 0x62, 0x8B, 0x80, 0xDC, 0x1C, 0xD1,
	0x29, 0x02, 0x4E, 0x08, 0x8A, 0x67, 0xCC, 0x74,
	0x02, 0x0B, 0xBE, 0xA6, 0x3B, 0x13, 0x9B, 0x22,
	0x51, 0x4A, 0x08, 0x79, 0x8E, 0x34, 0x04, 0xDD,
	0xEF, 0x95, 0x19, 0xB3, 0xCD, 0x3A, 0x43, 0x1B,
	0x30, 0x2B, 0x0A, 0x6D, 0xF2, 0x5F, 0x14, 0x37,
	0x4F, 0xE1, 0x35, 0x6D, 0x6D, 0x51, 0xC2, 0x45,
	0xE4, 0x85, 0xB5, 0x76, 0x62, 0x5E, 0x7E, 0xC6,
	0xF4, 0x4C, 0x42, 0xE9, 0xA6, 0x37, 0xED, 0x6B,
	0x0B, 0xFF, 0x5C, 0xB6, 0xF4, 0x06, 0xB7, 0xED,
	0xEE, 0x38, 0x6B, 0xFB, 0x5A, 0x89, 0x9F, 0xA5,
	0xAE, 0x9F, 0x24, 0x11, 0x7C, 0x4B, 0x1F, 0xE6,
	0x49, 0x28, 0x66, 0x51, 0xEC, 0xE4, 0x5B, 0x3D,
	0xC2, 0x00, 0x7C, 0xB8, 0xA1, 0x63, 0xBF, 0x05,
	0x98, 0xDA, 0x48, 0x36, 0x1C, 0x55, 0xD3, 0x9A,
	0x69, 0x16, 0x3F, 0xA8, 0xFD, 0x24, 0xCF, 0x5F,
	0x83, 0x65, 0x5D, 0x23, 0xDC, 0xA3, 0xAD, 0x96,
	0x1C, 0x62, 0xF3, 0x56, 0x20, 0x85, 0x52, 0xBB,
	0x9E, 0xD5, 0x29, 0x07, 0x70, 0x96, 0x96, 0x6D,
	0x67, 0x0C, 0x35, 0x4E, 0x4A, 0xBC, 0x98, 0x04,
	0xF1, 0x74, 0x6C, 0x08, 0xCA, 0x23, 0x73, 0x27,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};


UCHAR test2[] = {0x72, 0xf8, 0x45, 0x03, 0x69, 0xb0, 0x45, 0xff, 0xfa, 0xc3, 0x21};


VOID BigInteger_Mod_Mul_Inverse(
	IN PBIG_INTEGER pBI,
	IN PBIG_INTEGER pBI_P,
	OUT PBIG_INTEGER *pBI_Result)
{
	PBIG_INTEGER pFirstOperand = NULL;
	PBIG_INTEGER pSecondOperand = NULL;
	PBIG_INTEGER res = NULL;
	PBIG_INTEGER prev_u = NULL;
	PBIG_INTEGER prev_v = NULL;
	PBIG_INTEGER curr_u = NULL;
	PBIG_INTEGER curr_v = NULL;
	MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("==> %s()\n", __func__));
	BigInteger_record_time_begin(&bi_op_ti_rec.mod_mul_inv_op);
	POOL_COUNTER_CHECK_BEGIN(expected_cnt[14]);
	GET_BI_INS_FROM_POOL(pFirstOperand);
	GET_BI_INS_FROM_POOL(pSecondOperand);
	GET_BI_INS_FROM_POOL(res);
	GET_BI_INS_FROM_POOL(prev_u);
	GET_BI_INS_FROM_POOL(prev_v);
	GET_BI_INS_FROM_POOL(curr_u);
	GET_BI_INS_FROM_POOL(curr_v);
	BigInteger_Init(&pFirstOperand);
	BigInteger_Init(&pSecondOperand);

	if (pBI == NULL)
		BigInteger_Bin2BI(test, sizeof(test), &pFirstOperand);
	else
		BigInteger_Copy(pBI_P, &pFirstOperand);

	if (pBI_P == NULL)
		BigInteger_Bin2BI(test2, sizeof(test2), &pSecondOperand);
	else
		BigInteger_Mod(pBI, pBI_P, &pSecondOperand);

	/*DEBUGPRINT("P:");
	BigInteger_Print(ptest);*/
	/*DEBUGPRINT("INPUT:");
	BigInteger_Print(ptest2);*/
	BigInteger_DWtoBI(1, &prev_u);
	BigInteger_DWtoBI(0, &curr_u);
	BigInteger_DWtoBI(0, &prev_v);
	BigInteger_DWtoBI(1, &curr_v);
	BigInteger_Euclidean_Div(pFirstOperand, pSecondOperand, prev_u, prev_v, curr_u, curr_v);

	if (BigInteger_is_one(pSecondOperand) ==  FALSE) {
		MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("\x1b[31mpSecondOperand is not one!!!!!!\x1b[m\n"));
		release_temporal_usage_big_interger(pBI_Result);
		goto Free;
	}


	if (curr_v->Signed == -1) {
		BigInteger_Add(curr_v, pBI_P, &res);
		BigInteger_Copy(res, pBI_Result);
	} else
		BigInteger_Copy(curr_v, pBI_Result);

	/*DEBUGPRINT("BigInteger_Modular_Mul_Inverse result:");
	BigInteger_Print(*pBI_Result);*/
Free:
	release_temporal_usage_big_interger(&pFirstOperand);
	release_temporal_usage_big_interger(&pSecondOperand);
	release_temporal_usage_big_interger(&res);
	release_temporal_usage_big_interger(&prev_u);
	release_temporal_usage_big_interger(&prev_v);
	release_temporal_usage_big_interger(&curr_u);
	release_temporal_usage_big_interger(&curr_v);
	POOL_COUNTER_CHECK_END(expected_cnt[14]);
	BigInteger_record_time_end(&bi_op_ti_rec.mod_mul_inv_op);
}

VOID BigInteger_Mod_Mul_Inverse2(
	IN PBIG_INTEGER pBI,
	IN PBIG_INTEGER pBI_P,
	OUT PBIG_INTEGER *pBI_Result)
{
	BIG_INTEGER *tmp = NULL;
	BigInteger_record_time_begin(&bi_op_ti_rec.mod_mul_inv_op);
	BigInteger_Inverse(pBI, pBI_P, pBI_Result, &tmp);
	release_temporal_usage_big_interger(&tmp);
	BigInteger_record_time_end(&bi_op_ti_rec.mod_mul_inv_op);
}



UCHAR testa[] = {0x38, 0xa0, 0x39, 0xeb, 0x50, 0x4f, 0x4f, 0x50};


UCHAR BigInteger_Sqrt(
	IN PBIG_INTEGER pBI,
	OUT PBIG_INTEGER *pBI_Result)
{
	PBIG_INTEGER x = NULL;
	UINT32 x_len;
	UCHAR *x_buf = NULL;
	PBIG_INTEGER ptest = NULL;
	UINT8 first_byte;
	UINT8 x_first_byte;
	UCHAR res;
	UINT32 i = 0;
	BigInteger_record_time_begin(&bi_op_ti_rec.sqrt_op);

	/*
	  * Let S = a * 0x10^2n, where 0x1 <= a <= 0xff
	  * X ={ 0x2 * 0x10^n, where 0x1 <= a <= 0x10 ,note: sqrt(sqrt(0x1)*sqrt(0x10)) = 0x2
	  *      { 0x7 * 0x20^n, where 0x10 < a <= 0xff   ,note: sqrt(sqrt(0x10)*sqrt(0x100)) ~= 0x7
	  */
	if (pBI == NULL)
		BigInteger_Bin2BI(testa, sizeof(testa), &ptest);
	else
		BigInteger_Copy(pBI, &ptest);

	/*DEBUGPRINT("input:");
	BigInteger_Print(ptest);*/
	x_len = (ptest->IntegerLength + 1) / 2;
	os_alloc_mem(NULL, &x_buf, x_len);

	if (x_buf == NULL) {
		res = FALSE;
		goto Free;
	}

	NdisZeroMemory(x_buf, x_len);
	first_byte = BigInteger_GetByteValue(ptest, ptest->IntegerLength);

	if (first_byte > 0x10)
		x_first_byte = 0x7;
	else
		x_first_byte = 0x2;

	if (ptest->IntegerLength % 2 == 0)
		x_first_byte <<= 4;

	x_buf[0] = x_first_byte;
	BigInteger_Bin2BI(x_buf, x_len, &x);
	BigInteger_is_one(x);

	/* S is input, use Xi+1 = 1/2 * (Xi + S/Xi), if Xn+1 == Xn, sqrt(S) ~= Xn */
	do {
		PBIG_INTEGER div_res = NULL;
		PBIG_INTEGER div_rem = NULL;
		PBIG_INTEGER tmp = NULL;
		i++;
		/*DEBUGPRINT("x:");
		BigInteger_Print(x);*/
		BigInteger_Div(ptest, x, &div_res, &div_rem);

		/*DEBUGPRINT("div_res:");
		BigInteger_Print(div_res);
		DEBUGPRINT("div_rem:");
		BigInteger_Print(div_rem);*/
		if (BigInteger_UnsignedCompare(x, div_res) == 0) {
			if (BigInteger_is_zero(div_rem)) {
				DEBUGPRINT("\x1b[33m%s: found!!!\x1b[m\n", __func__);
				res = TRUE;
			} else
				res = FALSE;

			BigInteger_Free(&div_res);
			BigInteger_Free(&div_rem);
			break;
		}

		if (BigInteger_UnsignedCompare(x, div_rem) != 1) { /* ellis: if BigInteger_Div work, remove it */
			res = FALSE;
			BigInteger_Free(&div_res);
			BigInteger_Free(&div_rem);
			DEBUGPRINT("%s: div fail!!!\n", __func__);
			break;
		}

		BigInteger_Add(x, div_res, &tmp);
		BigInteger_Shift_Right1(tmp, &div_res);

		if (BigInteger_UnsignedCompare(x, div_res) == 0) {
			DEBUGPRINT("diff == 1\n");
			res = FALSE;
			BigInteger_Free(&div_res);
			BigInteger_Free(&div_rem);
			BigInteger_Free(&tmp);
			break;
		}

		BigInteger_Copy(div_res, &x);
		/*DEBUGPRINT("new x:");
		BigInteger_Print(x);*/
		BigInteger_Free(&div_res);
		BigInteger_Free(&div_rem);
		BigInteger_Free(&tmp);
	} while (1);

	DEBUGPRINT("%s: total loop %d\n", __func__ , i);

	if (res) {
		if (*pBI_Result == NULL)
			BigInteger_Init(pBI_Result);

		BigInteger_Copy(x, pBI_Result);
	} else
		BigInteger_Free(pBI_Result);

	os_free_mem(x_buf);
Free:
	BigInteger_Free(&ptest);
	BigInteger_Free(&x);
	BigInteger_record_time_end(&bi_op_ti_rec.sqrt_op);
	return res;
}

VOID BigInteger_Shift_Right1(
	IN PBIG_INTEGER pBI,
	OUT PBIG_INTEGER *pBI_Result)
{
	BIG_INTEGER *res = NULL;
	UINT32 len;
	UINT32 shift_bit = 1;
	INT16 i;
	UINT32 shift_tmp;
	BIG_INTEGER *ptest = NULL;

	POOL_COUNTER_CHECK_BEGIN(expected_cnt[10]);
	GET_BI_INS_FROM_POOL(res);
	GET_BI_INS_FROM_POOL(ptest);

	if (pBI == NULL)
		BigInteger_Bin2BI(test, sizeof(test), &ptest); /* debug usage */
	else
		BigInteger_Copy(pBI, &ptest);

	len = ptest->IntegerLength;
	BigInteger_AllocSize(&res, len);
	NdisCopyMemory(res->pIntegerArray, ptest->pIntegerArray, len);
	res->Signed = ptest->Signed;

	/* i = res->ArrayLength - 1 */
	shift_tmp = res->pIntegerArray[res->ArrayLength - 1] & ((1 << shift_bit) - 1);
	res->pIntegerArray[res->ArrayLength - 1] >>= shift_bit;

	for (i = res->ArrayLength - 2; i >= 0; i--) {
		UINT32 shift_tmp2 =  res->pIntegerArray[i] & ((1 << shift_bit) - 1);
		res->pIntegerArray[i] = (shift_tmp << (32 - shift_bit))
								| (res->pIntegerArray[i] >> shift_bit);
		shift_tmp = shift_tmp2;
	}

	BigInteger_ClearHighBits(res);
	BigInteger_Copy(res, pBI_Result);
	release_temporal_usage_big_interger(&ptest);
	release_temporal_usage_big_interger(&res);
	POOL_COUNTER_CHECK_END(expected_cnt[10]);
}

VOID BigInteger_Mod_Shift_Left(
	IN PBIG_INTEGER pBI,
	IN UCHAR bit,
	IN PBIG_INTEGER pBI_P,
	OUT PBIG_INTEGER *pBI_Result)
{
	BIG_INTEGER *res = NULL;
	BIG_INTEGER *tmp = NULL;
	UCHAR two[] = {0x1};

	two[0] <<= bit;

	GET_BI_INS_FROM_POOL(res);
	GET_BI_INS_FROM_POOL(tmp);

	BigInteger_Bin2BI(two, sizeof(two), &tmp);
	BigInteger_Mod_Mul(pBI, tmp, pBI_P, &res);

	BigInteger_Copy(res, pBI_Result);
	release_temporal_usage_big_interger(&tmp);
	release_temporal_usage_big_interger(&res);
}

/* do not use this api(unknown bug) */
VOID BigInteger_Shift_Left(
	IN PBIG_INTEGER pBI,
	IN UINT32 bit,
	OUT PBIG_INTEGER *pBI_Result)
{
	BIG_INTEGER *res = NULL;
	UINT32 len;
	UINT32 shift_bit = bit % 32;
	UINT32 shift_dword = bit / 32;
	UINT32 shift_byte = (shift_bit + 7) / 8;
	INT16 i;
	UINT32 shift_tmp = 0;
	BIG_INTEGER *ptest = NULL;

	if (bit == 0) {
		BigInteger_Copy(pBI, pBI_Result);
		return;
	}

	POOL_COUNTER_CHECK_BEGIN(expected_cnt[11]);
	GET_BI_INS_FROM_POOL(res);
	GET_BI_INS_FROM_POOL(ptest);

	if (pBI == NULL)
		BigInteger_Bin2BI(test, sizeof(test), &ptest); /* debug usage */
	else
		BigInteger_Copy(pBI, &ptest);

	len = ptest->IntegerLength + shift_dword * 4;

	if (shift_bit)
		len += shift_byte;

	BigInteger_AllocSize(&res, len);
	res->Signed = ptest->Signed;

	if (shift_bit) {
		/* i = 0 */
		shift_tmp = ptest->pIntegerArray[0] >> (32 - shift_bit);
		res->pIntegerArray[shift_dword] = ptest->pIntegerArray[0] << shift_bit;

		for (i = 1; i < ptest->ArrayLength; i++) {
			UINT32 shift_tmp2 = ptest->pIntegerArray[i] >> (32 - shift_bit);
			res->pIntegerArray[i + shift_dword] = shift_tmp | (ptest->pIntegerArray[i] << shift_bit);
			shift_tmp = shift_tmp2;
		}

		if (shift_bit)
			res->pIntegerArray[ptest->ArrayLength + shift_dword] = shift_tmp;
	}

	BigInteger_Copy(res, pBI_Result);
	release_temporal_usage_big_interger(&ptest);
	release_temporal_usage_big_interger(&res);
	POOL_COUNTER_CHECK_END(expected_cnt[11]);
}

/* treat pBi is positve and non-zero */
VOID BigInteger_minus_one(
	INOUT PBIG_INTEGER pBI)
{
	INT16 i;

	if (pBI == NULL)
		return;

	for (i = 0; i < pBI->ArrayLength; i++) {
		if (pBI->pIntegerArray[i]) {
			pBI->pIntegerArray[i]--;
			return;
		} else
			pBI->pIntegerArray[i] = ~0;
	}
}

/* treat pBi is positve and non-zero */
VOID BigInteger_plus_one(
	INOUT PBIG_INTEGER pBI)
{
	INT16 i;

	if (pBI == NULL)
		return;

	for (i = 0; i < pBI->ArrayLength; i++) {
		if (pBI->pIntegerArray[i] != ~0) {
			pBI->pIntegerArray[i]++;
			return;
		} else
			pBI->pIntegerArray[i] = 0;
	}

	/* todo: length update */
	panic("BigInteger_plus_one: todo: length update");
}


/* an integer q is called a quadratic residue modulo n if it is congruent to a perfect square modulo n */
UCHAR BigInteger_is_quadratic_residue(
	IN BIG_INTEGER *q,
	IN BIG_INTEGER *prime,
	IN MONT_STRUC * mont)
{
	BIG_INTEGER *p = NULL;
	BIG_INTEGER *res = NULL;
	UCHAR ret;
	POOL_COUNTER_CHECK_BEGIN(expected_cnt[12]);
	GET_BI_INS_FROM_POOL(p);
	GET_BI_INS_FROM_POOL(res);
	BigInteger_Init(&res);
	BigInteger_Copy(prime, &p);
	BigInteger_minus_one(p);
	BigInteger_Shift_Right1(p, &p);
	BigInteger_Montgomery_ExpMod_with_mont(q, p, prime, mont, &res);
	ret = BigInteger_is_one(res);
	release_temporal_usage_big_interger(&p);
	release_temporal_usage_big_interger(&res);
	POOL_COUNTER_CHECK_END(expected_cnt[12]);
	/* if (ret == TRUE) DEBUGPRINT("\x1b[33mgot it\x1b[m\n"); */
	return ret;
}

VOID BigInteger_Montgomery_ExpMod32(
	IN PBIG_INTEGER pBI_G,
	IN PBIG_INTEGER pBI_E,
	IN PBIG_INTEGER pBI_P,
	OUT PBIG_INTEGER *pBI_Result)
{
	BIG_INTEGER *g = NULL;
	BIG_INTEGER *e = NULL;
	BIG_INTEGER *res = NULL;
	UINT i = 0;
	UINT j = 0;
	MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("==> %s()\n", __func__));
	BigInteger_record_time_begin(&bi_op_ti_rec.simple_exp_mod_op);
	BigInteger_Copy(pBI_G, &g);
	BigInteger_Copy(pBI_E, &e);
	BigInteger_Bin2BI(Value_1, 1, &res);

	while (BigInteger_is_zero(e) == FALSE) {
		if (BigInteger_is_odd(e)) {
			BigInteger_Mod_Mul(res, g, pBI_P, &res);
			BigInteger_minus_one(e);
		} else {
			BigInteger_Mod_Mul(g, g, pBI_P, &g);
			BigInteger_Shift_Right1(e, &e);
			j++;
		}

		i++;
	}

	BigInteger_Copy(res, pBI_Result);
	BigInteger_Free(&g);
	BigInteger_Free(&e);
	BigInteger_Free(&res);
	BigInteger_record_time_end(&bi_op_ti_rec.simple_exp_mod_op);
}

VOID BigInteger_DWtoBI(
	IN UINT32 value,
	OUT PBIG_INTEGER *pBI)
{
	BigInteger_AllocSize(pBI, 4);
	(*pBI)->pIntegerArray[0] = value;
	BigInteger_ClearHighBits(*pBI);
}

VOID BigInteger_Add_DW(
	INOUT PBIG_INTEGER pBI,
	IN UINT32 value)
{
	UINT32 copy = pBI->pIntegerArray[0];
	UINT8 i;
	pBI->pIntegerArray[0] += value;

	if (pBI->pIntegerArray[0] < copy) {
		for (i = 1; i < pBI->ArrayLength; i++) {
			if (pBI->pIntegerArray[i] != ~0) {
				pBI->pIntegerArray[i]++;
				BigInteger_ClearHighBits(pBI);
				return;
			} else
				pBI->pIntegerArray[i] = 0;
		}
	}

	/* todo: length update */
	panic("BigInteger_Add_DW: todo: length update");
}


VOID BigInteger_Mod_DW(
	INOUT PBIG_INTEGER pBI,
	IN UINT32 value,
	OUT UINT32 *rem)
{
	*rem = pBI->pIntegerArray[0] % value;
	pBI->pIntegerArray[0] -= *rem;
}



VOID BigInteger_DtoH(
	INOUT PBIG_INTEGER pBI)
{
	BIG_INTEGER *res = NULL;
	BIG_INTEGER *tmp = NULL;
	BIG_INTEGER *tmp2 = NULL;
	BIG_INTEGER *tmp3 = NULL;
	BIG_INTEGER *hundred = NULL;
	UINT i;
	UINT8 byte_value;
	BigInteger_DWtoBI(100, &hundred);
	BigInteger_DWtoBI((UINT32)1, &tmp2);
	BigInteger_DWtoBI((UINT32)0, &res);

	for (i = 1; i <= pBI->IntegerLength; i++) {
		byte_value = BigInteger_GetByteValue(pBI, i);
		BigInteger_DWtoBI((UINT32)byte_value, &tmp);
		BigInteger_Mul(tmp, tmp2, &tmp3);
		BigInteger_Copy(tmp3, &tmp);
		BigInteger_Add(res, tmp, &tmp3);
		BigInteger_Copy(tmp3, &res);
		BigInteger_Mul(tmp2, hundred, &tmp);
		BigInteger_Copy(tmp, &tmp2);
	}

	BigInteger_Copy(res, &pBI);
	BigInteger_Free(&res);
	BigInteger_Free(&tmp);
	BigInteger_Free(&tmp2);
	BigInteger_Free(&tmp3);
	BigInteger_Free(&hundred);
}

VOID BigInteger_dump_time(
	VOID)
{
	if (DebugLevel < DBG_LVL_TRACE)
		return;

	DEBUGPRINT("BigInteger time record: (allocated cnt = %lu)\n", alloc_cnt);
	DEBUGPRINT("add allocated cnt = %lu\n", alloc_cnt_add);
	DEBUGPRINT("sub allocated cnt = %lu\n", alloc_cnt_sub);
	DEBUGPRINT("mul allocated cnt = %lu\n", alloc_cnt_mul);
	DEBUGPRINT("div allocated cnt = %lu\n", alloc_cnt_div);
	DEBUGPRINT("mod allocated cnt = %lu\n", alloc_cnt_mod);

	if (bi_op_ti_rec.add_op.exe_times) {
		DEBUGPRINT("add_op time record:\n");
		DEBUGPRINT("\tavg_time=%lu.%03lu nsec", (bi_op_ti_rec.add_op.time_interval / bi_op_ti_rec.add_op.exe_times),
				   show_float(bi_op_ti_rec.add_op.time_interval, bi_op_ti_rec.add_op.exe_times, 3));
		DEBUGPRINT(", %lu nsec", bi_op_ti_rec.add_op.time_interval);
		DEBUGPRINT("\n\texe_times=%u\n", bi_op_ti_rec.add_op.exe_times);
		bi_op_ti_rec.add_op.time_interval = 0;
		bi_op_ti_rec.add_op.exe_times = 0;
		bi_op_ti_rec.add_op.avg_time_interval = 0;
	}

	if (bi_op_ti_rec.sub_op.exe_times) {
		DEBUGPRINT("sub_op time record:\n");
		DEBUGPRINT("\tavg_time=%lu.%03lu nsec", (bi_op_ti_rec.sub_op.time_interval / bi_op_ti_rec.sub_op.exe_times),
				   show_float(bi_op_ti_rec.sub_op.time_interval, bi_op_ti_rec.sub_op.exe_times, 3));
		DEBUGPRINT(", %lu nsec", bi_op_ti_rec.sub_op.time_interval);
		DEBUGPRINT(", add:sub = 1:%lu.%02lu", (bi_op_ti_rec.sub_op.time_interval / bi_op_ti_rec.add_op.time_interval),
				   show_float(bi_op_ti_rec.sub_op.time_interval, bi_op_ti_rec.add_op.time_interval, 2));
		DEBUGPRINT("\n\texe_times=%u\n", bi_op_ti_rec.sub_op.exe_times);
		bi_op_ti_rec.sub_op.time_interval = 0;
		bi_op_ti_rec.sub_op.exe_times = 0;
		bi_op_ti_rec.sub_op.avg_time_interval = 0;
	}

	if (bi_op_ti_rec.mul_op.exe_times) {
		DEBUGPRINT("mul_op time record:\n");
		DEBUGPRINT("\tavg_time=%lu.%03lu nsec", (bi_op_ti_rec.mul_op.time_interval / bi_op_ti_rec.mul_op.exe_times),
				   show_float(bi_op_ti_rec.mul_op.time_interval, bi_op_ti_rec.mul_op.exe_times, 3));
		DEBUGPRINT(", %lu nsec", bi_op_ti_rec.mul_op.time_interval);
		DEBUGPRINT(", add:mul = 1:%lu.%02lu", (bi_op_ti_rec.mul_op.time_interval / bi_op_ti_rec.add_op.time_interval),
				   show_float(bi_op_ti_rec.mul_op.time_interval, bi_op_ti_rec.add_op.time_interval, 2));
		DEBUGPRINT("\n\texe_times=%u\n", bi_op_ti_rec.mul_op.exe_times);
		bi_op_ti_rec.mul_op.time_interval = 0;
		bi_op_ti_rec.mul_op.exe_times = 0;
		bi_op_ti_rec.mul_op.avg_time_interval = 0;
	}

	if (bi_op_ti_rec.div_op.exe_times) {
		DEBUGPRINT("div_op time record:\n");
		DEBUGPRINT("\tavg_time=%lu.%03lu nsec", (bi_op_ti_rec.div_op.time_interval / bi_op_ti_rec.div_op.exe_times),
			show_float(bi_op_ti_rec.div_op.time_interval, bi_op_ti_rec.div_op.exe_times, 3));
		DEBUGPRINT(", %lu nsec", bi_op_ti_rec.div_op.time_interval);
		DEBUGPRINT(", add:div = 1:%lu.%02lu", (bi_op_ti_rec.div_op.time_interval / bi_op_ti_rec.add_op.time_interval),
			show_float(bi_op_ti_rec.div_op.time_interval, bi_op_ti_rec.add_op.time_interval, 2));
		DEBUGPRINT("\n\texe_times=%u\n", bi_op_ti_rec.div_op.exe_times);
		bi_op_ti_rec.div_op.avg_time_interval = 0;
		bi_op_ti_rec.div_op.time_interval = 0;
		bi_op_ti_rec.div_op.exe_times = 0;
	}

	if (bi_op_ti_rec.mod_op.exe_times) {
		DEBUGPRINT("mod_op time record:\n");
		DEBUGPRINT("\tavg_time=%lu.%03lu nsec", (bi_op_ti_rec.mod_op.time_interval / bi_op_ti_rec.mod_op.exe_times),
			show_float(bi_op_ti_rec.mod_op.time_interval, bi_op_ti_rec.mod_op.exe_times, 3));
		DEBUGPRINT(", %lu nsec", bi_op_ti_rec.mod_op.time_interval);
		if (bi_op_ti_rec.add_op.time_interval)
			DEBUGPRINT(", add:mod = 1:%lu.%02lu", (bi_op_ti_rec.mod_op.time_interval / bi_op_ti_rec.add_op.time_interval),
				show_float(bi_op_ti_rec.mod_op.time_interval, bi_op_ti_rec.add_op.time_interval, 2));
		DEBUGPRINT("\n\texe_times=%u\n", bi_op_ti_rec.mod_op.exe_times);
		bi_op_ti_rec.mod_op.avg_time_interval = 0;
		bi_op_ti_rec.mod_op.time_interval = 0;
		bi_op_ti_rec.mod_op.exe_times = 0;
	}

	if (bi_op_ti_rec.square_op.exe_times) {
		DEBUGPRINT("square_op time record:\n");
		DEBUGPRINT("\tavg_time=%lu.%03lu nsec", (bi_op_ti_rec.square_op.time_interval / bi_op_ti_rec.square_op.exe_times),
			show_float(bi_op_ti_rec.square_op.time_interval, bi_op_ti_rec.square_op.exe_times, 3));
		DEBUGPRINT(", %lu nsec", bi_op_ti_rec.square_op.time_interval);
		DEBUGPRINT("\n\texe_times=%u\n", bi_op_ti_rec.square_op.exe_times);
		bi_op_ti_rec.square_op.avg_time_interval = 0;
		bi_op_ti_rec.square_op.time_interval = 0;
		bi_op_ti_rec.square_op.exe_times = 0;
	}

	alloc_cnt = 0;
	alloc_cnt_add = 0;
	alloc_cnt_sub = 0;
	alloc_cnt_mul = 0;
	alloc_cnt_div = 0;
	alloc_cnt_mod = 0;

	if (bi_op_ti_rec.sqrt_op.exe_times) {
		DEBUGPRINT("sqrt_op time record:\n");
		DEBUGPRINT("\tavg_time=%lu jiffies", bi_op_ti_rec.sqrt_op.avg_time_interval);
#ifdef LINUX
		DEBUGPRINT(", %u msec", jiffies_to_msecs(bi_op_ti_rec.sqrt_op.avg_time_interval));
#endif
		DEBUGPRINT("\n\texe_times=%u\n", bi_op_ti_rec.sqrt_op.exe_times);
		bi_op_ti_rec.sqrt_op.avg_time_interval = 0;
		bi_op_ti_rec.sqrt_op.time_interval = 0;
		bi_op_ti_rec.sqrt_op.exe_times = 0;
	}

	if (bi_op_ti_rec.exp_mod_op.exe_times) {
		DEBUGPRINT("exp_mod_op time record:\n");
		DEBUGPRINT("\tavg_time=%lu jiffies", bi_op_ti_rec.exp_mod_op.avg_time_interval);
#ifdef LINUX
		DEBUGPRINT(", %u msec", jiffies_to_msecs(bi_op_ti_rec.exp_mod_op.avg_time_interval));
#endif
		DEBUGPRINT("\n\texe_times=%u\n", bi_op_ti_rec.exp_mod_op.exe_times);
		bi_op_ti_rec.exp_mod_op.avg_time_interval = 0;
		bi_op_ti_rec.exp_mod_op.time_interval = 0;
		bi_op_ti_rec.exp_mod_op.exe_times = 0;
	}

	if (bi_op_ti_rec.div_mod_op.exe_times) {
		DEBUGPRINT("div_mod_op time record:\n");
		DEBUGPRINT("\tavg_time=%lu jiffies", bi_op_ti_rec.div_mod_op.avg_time_interval);
#ifdef LINUX
		DEBUGPRINT(", %u msec", jiffies_to_msecs(bi_op_ti_rec.div_mod_op.avg_time_interval));
#endif
		DEBUGPRINT("\n\texe_times=%u\n", bi_op_ti_rec.div_mod_op.exe_times);
		bi_op_ti_rec.div_mod_op.avg_time_interval = 0;
		bi_op_ti_rec.div_mod_op.time_interval = 0;
		bi_op_ti_rec.div_mod_op.exe_times = 0;
	}

	if (bi_op_ti_rec.sqrt_mod_op.exe_times) {
		DEBUGPRINT("sqrt_mod_op time record:\n");
		DEBUGPRINT("\tavg_time=%lu jiffies", bi_op_ti_rec.sqrt_mod_op.avg_time_interval);
#ifdef LINUX
		DEBUGPRINT(", %u msec", jiffies_to_msecs(bi_op_ti_rec.sqrt_mod_op.avg_time_interval));
#endif
		DEBUGPRINT("\n\texe_times=%u\n", bi_op_ti_rec.sqrt_mod_op.exe_times);
		bi_op_ti_rec.sqrt_mod_op.avg_time_interval = 0;
		bi_op_ti_rec.sqrt_mod_op.time_interval = 0;
		bi_op_ti_rec.sqrt_mod_op.exe_times = 0;
	}

	if (bi_op_ti_rec.mod_mul_inv_op.exe_times) {
		DEBUGPRINT("mod_mul_inv_op time record:\n");
		DEBUGPRINT("\tavg_time=%lu jiffies", bi_op_ti_rec.mod_mul_inv_op.avg_time_interval);
#ifdef LINUX
		DEBUGPRINT(", %u msec", jiffies_to_msecs(bi_op_ti_rec.mod_mul_inv_op.avg_time_interval));
#endif
		DEBUGPRINT("\n\texe_times=%u\n", bi_op_ti_rec.mod_mul_inv_op.exe_times);
		bi_op_ti_rec.mod_mul_inv_op.avg_time_interval = 0;
		bi_op_ti_rec.mod_mul_inv_op.time_interval = 0;
		bi_op_ti_rec.mod_mul_inv_op.exe_times = 0;
	}

	if (bi_op_ti_rec.simple_exp_mod_op.exe_times) {
		DEBUGPRINT("simple_exp_mod_op time record:\n");
		DEBUGPRINT("\tavg_time=%lu jiffies", bi_op_ti_rec.simple_exp_mod_op.avg_time_interval);
#ifdef LINUX
		DEBUGPRINT(", %u msec", jiffies_to_msecs(bi_op_ti_rec.simple_exp_mod_op.avg_time_interval));
#endif
		DEBUGPRINT("\n\texe_times=%u\n", bi_op_ti_rec.simple_exp_mod_op.exe_times);
		bi_op_ti_rec.simple_exp_mod_op.avg_time_interval = 0;
		bi_op_ti_rec.simple_exp_mod_op.time_interval = 0;
		bi_op_ti_rec.simple_exp_mod_op.exe_times = 0;
	}
}

#endif

/* End of crypt_biginteger.c */


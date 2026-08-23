/*
 ***************************************************************************
 * MediaTeK Inc.
 * 4F, No. 2 Technology	5th	Rd.
 * Science-based Industrial	Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2012, MTK.
 *
 * All rights reserved.	MediaTeK's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of MediaTeK Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of MediaTeK, Inc. is obtained.
 ***************************************************************************

	Module Name:
	crypt_aes.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	lisa.xue	2012/7/28	Create
*/


#include <string.h>
#if !defined(TCSUPPORT_3_18_21_KERNEL)
#include <linux/mm.h>
#endif
#include "omci_me_crypt_aes.h"

extern void omcidbgPrintf(uint8 debugLevel, char *fmt,...);

/* The value given by [x^(i-1),{00},{00},{00}], with x^(i-1) being powers of x in the field GF(2^8). */
static const uint32 aes_rcon[] = {
	0x00000000, 0x01000000, 0x02000000, 0x04000000, 
    0x08000000, 0x10000000, 0x20000000, 0x40000000, 
    0x80000000, 0x1B000000, 0x36000000};

static const uint8 aes_sbox_enc[] = {
  /*  0     1     2     3     4     5     6     7     8     9     a     b     c     d     e     f    */
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7 ,0xab, 0x76, /* 0 */
    0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4 ,0x72, 0xc0, /* 1 */
    0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8 ,0x31, 0x15, /* 2 */
    0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27 ,0xb2, 0x75, /* 3 */
    0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3 ,0x2f, 0x84, /* 4 */
    0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c ,0x58, 0xcf, /* 5 */
    0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c ,0x9f, 0xa8, /* 6 */
    0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff ,0xf3, 0xd2, /* 7 */
    0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d ,0x19, 0x73, /* 8 */
    0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e ,0x0b, 0xdb, /* 9 */
    0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95 ,0xe4, 0x79, /* a */
    0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a ,0xae, 0x08, /* b */
    0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd ,0x8b, 0x8a, /* c */
    0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1 ,0x1d, 0x9e, /* d */
    0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55 ,0x28, 0xdf, /* e */
    0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54 ,0xbb, 0x16, /* f */
};

/* ArrayIndex*{02} */
static const uint8 aes_mul_2[] = {
  /*  0     1     2     3     4     5     6     7     8     9     a     b     c     d     e     f    */
    0x00, 0x02, 0x04, 0x06, 0x08, 0x0a, 0x0c, 0x0e, 0x10, 0x12, 0x14, 0x16, 0x18, 0x1a, 0x1c, 0x1e, /* 0 */
    0x20, 0x22, 0x24, 0x26, 0x28, 0x2a, 0x2c, 0x2e, 0x30, 0x32, 0x34, 0x36, 0x38, 0x3a, 0x3c, 0x3e, /* 1 */
    0x40, 0x42, 0x44, 0x46, 0x48, 0x4a, 0x4c, 0x4e, 0x50, 0x52, 0x54, 0x56, 0x58, 0x5a, 0x5c, 0x5e, /* 2 */
    0x60, 0x62, 0x64, 0x66, 0x68, 0x6a, 0x6c, 0x6e, 0x70, 0x72, 0x74, 0x76, 0x78, 0x7a, 0x7c, 0x7e, /* 3 */
    0x80, 0x82, 0x84, 0x86, 0x88, 0x8a, 0x8c, 0x8e, 0x90, 0x92, 0x94, 0x96, 0x98, 0x9a, 0x9c, 0x9e, /* 4 */
    0xa0, 0xa2, 0xa4, 0xa6, 0xa8, 0xaa, 0xac, 0xae, 0xb0, 0xb2, 0xb4, 0xb6, 0xb8, 0xba, 0xbc, 0xbe, /* 5 */
    0xc0, 0xc2, 0xc4, 0xc6, 0xc8, 0xca, 0xcc, 0xce, 0xd0, 0xd2, 0xd4, 0xd6, 0xd8, 0xda, 0xdc, 0xde, /* 6 */
    0xe0, 0xe2, 0xe4, 0xe6, 0xe8, 0xea, 0xec, 0xee, 0xf0, 0xf2, 0xf4, 0xf6, 0xf8, 0xfa, 0xfc, 0xfe, /* 7 */
    0x1b, 0x19, 0x1f, 0x1d, 0x13, 0x11, 0x17, 0x15, 0x0b, 0x09, 0x0f, 0x0d, 0x03, 0x01, 0x07, 0x05, /* 8 */
    0x3b, 0x39, 0x3f, 0x3d, 0x33, 0x31, 0x37, 0x35, 0x2b, 0x29, 0x2f, 0x2d, 0x23, 0x21, 0x27, 0x25, /* 9 */
    0x5b, 0x59, 0x5f, 0x5d, 0x53, 0x51, 0x57, 0x55, 0x4b, 0x49, 0x4f, 0x4d, 0x43, 0x41, 0x47, 0x45, /* a */
    0x7b, 0x79, 0x7f, 0x7d, 0x73, 0x71, 0x77, 0x75, 0x6b, 0x69, 0x6f, 0x6d, 0x63, 0x61, 0x67, 0x65, /* b */
    0x9b, 0x99, 0x9f, 0x9d, 0x93, 0x91, 0x97, 0x95, 0x8b, 0x89, 0x8f, 0x8d, 0x83, 0x81, 0x87, 0x85, /* c */
    0xbb, 0xb9, 0xbf, 0xbd, 0xb3, 0xb1, 0xb7, 0xb5, 0xab, 0xa9, 0xaf, 0xad, 0xa3, 0xa1, 0xa7, 0xa5, /* d */
    0xdb, 0xd9, 0xdf, 0xdd, 0xd3, 0xd1, 0xd7, 0xd5, 0xcb, 0xc9, 0xcf, 0xcd, 0xc3, 0xc1, 0xc7, 0xc5, /* e */
    0xfb, 0xf9, 0xff, 0xfd, 0xf3, 0xf1, 0xf7, 0xf5, 0xeb, 0xe9, 0xef, 0xed, 0xe3, 0xe1, 0xe7, 0xe5, /* f */
};

/* ArrayIndex*{03} */
static const uint8 aes_mul_3[] = {
  /*  0     1     2     3     4     5     6     7     8     9     a     b     c     d     e     f    */
    0x00, 0x03, 0x06, 0x05, 0x0c, 0x0f, 0x0a, 0x09, 0x18, 0x1b, 0x1e, 0x1d, 0x14, 0x17, 0x12, 0x11, /* 0 */
    0x30, 0x33, 0x36, 0x35, 0x3c, 0x3f, 0x3a, 0x39, 0x28, 0x2b, 0x2e, 0x2d, 0x24, 0x27, 0x22, 0x21, /* 1 */
    0x60, 0x63, 0x66, 0x65, 0x6c, 0x6f, 0x6a, 0x69, 0x78, 0x7b, 0x7e, 0x7d, 0x74, 0x77, 0x72, 0x71, /* 2 */
    0x50, 0x53, 0x56, 0x55, 0x5c, 0x5f, 0x5a, 0x59, 0x48, 0x4b, 0x4e, 0x4d, 0x44, 0x47, 0x42, 0x41, /* 3 */
    0xc0, 0xc3, 0xc6, 0xc5, 0xcc, 0xcf, 0xca, 0xc9, 0xd8, 0xdb, 0xde, 0xdd, 0xd4, 0xd7, 0xd2, 0xd1, /* 4 */
    0xf0, 0xf3, 0xf6, 0xf5, 0xfc, 0xff, 0xfa, 0xf9, 0xe8, 0xeb, 0xee, 0xed, 0xe4, 0xe7, 0xe2, 0xe1, /* 5 */
    0xa0, 0xa3, 0xa6, 0xa5, 0xac, 0xaf, 0xaa, 0xa9, 0xb8, 0xbb, 0xbe, 0xbd, 0xb4, 0xb7, 0xb2, 0xb1, /* 6 */
    0x90, 0x93, 0x96, 0x95, 0x9c, 0x9f, 0x9a, 0x99, 0x88, 0x8b, 0x8e, 0x8d, 0x84, 0x87, 0x82, 0x81, /* 7 */
    0x9b, 0x98, 0x9d, 0x9e, 0x97, 0x94, 0x91, 0x92, 0x83, 0x80, 0x85, 0x86, 0x8f, 0x8c, 0x89, 0x8a, /* 8 */
    0xab, 0xa8, 0xad, 0xae, 0xa7, 0xa4, 0xa1, 0xa2, 0xb3, 0xb0, 0xb5, 0xb6, 0xbf, 0xbc, 0xb9, 0xba, /* 9 */
    0xfb, 0xf8, 0xfd, 0xfe, 0xf7, 0xf4, 0xf1, 0xf2, 0xe3, 0xe0, 0xe5, 0xe6, 0xef, 0xec, 0xe9, 0xea, /* a */
    0xcb, 0xc8, 0xcd, 0xce, 0xc7, 0xc4, 0xc1, 0xc2, 0xd3, 0xd0, 0xd5, 0xd6, 0xdf, 0xdc, 0xd9, 0xda, /* b */
    0x5b, 0x58, 0x5d, 0x5e, 0x57, 0x54, 0x51, 0x52, 0x43, 0x40, 0x45, 0x46, 0x4f, 0x4c, 0x49, 0x4a, /* c */
    0x6b, 0x68, 0x6d, 0x6e, 0x67, 0x64, 0x61, 0x62, 0x73, 0x70, 0x75, 0x76, 0x7f, 0x7c, 0x79, 0x7a, /* d */
    0x3b, 0x38, 0x3d, 0x3e, 0x37, 0x34, 0x31, 0x32, 0x23, 0x20, 0x25, 0x26, 0x2f, 0x2c, 0x29, 0x2a, /* e */
    0x0b, 0x08, 0x0d, 0x0e, 0x07, 0x04, 0x01, 0x02, 0x13, 0x10, 0x15, 0x16, 0x1f, 0x1c, 0x19, 0x1a, /* f */
};

/* For AES_CMAC */
#define AES_MAC_LENGTH 16 /* 128-bit string */
static uint8 Const_Zero[16] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static uint8 Const_Rb[16] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x87};
   
/*
========================================================================
Routine Description:
    AES key expansion (key schedule)

Arguments:
    Key              Cipher key, it may be 16, 24, or 32 bytes (128, 192, or 256 bits)
    KeyLength        The length of cipher key in bytes
    paes_ctx         Pointer to AES_CTX_STRUC

Return Value:
    paes_ctx         Retrun the KeyWordExpansion of AES_CTX_STRUC

Note:
    Pseudo code for key expansion
    ------------------------------------------
       Nk = (key length/4);
       
       while (i < Nk)
           KeyWordExpansion[i] = word(key[4*i], key[4*i + 1], key[4*i + 2], key[4*i + 3]);
           i++;
       end while

       while (i < ((key length/4 + 6 + 1)*4) )
           temp = KeyWordExpansion[i - 1];
           if (i % Nk ==0)
               temp = SubWord(RotWord(temp)) ^ Rcon[i/Nk];
           else if ((Nk > 6) && (i % 4 == 4))
               temp = SubWord(temp);
           end if

           KeyWordExpansion[i] = KeyWordExpansion[i - Nk]^ temp;
           i++;
       end while
========================================================================
*/
void RT_AES_KeyExpansion (
    IN uint8 Key[],
    IN uint32 KeyLength,
    IN_OUT AES_CTX_STRUC *paes_ctx)
{
	uint32 KeyIndex = 0;
	uint32 NumberOfWordOfKey = 0, NumberOfWordOfKeyExpansion = 0;
	uint8  TempWord[AES_KEY_ROWS] = {0}, Temp = 0;
	uint32 Temprcon = 0;

	if((Key == NULL) || (paes_ctx == NULL))
		return;
	
	NumberOfWordOfKey = KeyLength >> 2;
	while (KeyIndex < NumberOfWordOfKey)
	{
		paes_ctx->KeyWordExpansion[0][KeyIndex] = Key[4*KeyIndex];
		paes_ctx->KeyWordExpansion[1][KeyIndex] = Key[4*KeyIndex + 1];
		paes_ctx->KeyWordExpansion[2][KeyIndex] = Key[4*KeyIndex + 2];
		paes_ctx->KeyWordExpansion[3][KeyIndex] = Key[4*KeyIndex + 3];
		KeyIndex++;
	}

	NumberOfWordOfKeyExpansion = ((uint32) AES_KEY_ROWS) * ((KeyLength >> 2) + 6 + 1);    
	while (KeyIndex < NumberOfWordOfKeyExpansion)
	{
		TempWord[0] = paes_ctx->KeyWordExpansion[0][KeyIndex - 1];
		TempWord[1] = paes_ctx->KeyWordExpansion[1][KeyIndex - 1];
		TempWord[2] = paes_ctx->KeyWordExpansion[2][KeyIndex - 1];
		TempWord[3] = paes_ctx->KeyWordExpansion[3][KeyIndex - 1];
		if ((KeyIndex % NumberOfWordOfKey) == 0) {
			Temprcon = aes_rcon[KeyIndex/NumberOfWordOfKey];
			Temp = aes_sbox_enc[TempWord[1]]^((Temprcon >> 24) & 0xff);
			TempWord[1] = aes_sbox_enc[TempWord[2]]^((Temprcon >> 16) & 0xff);
			TempWord[2] = aes_sbox_enc[TempWord[3]]^((Temprcon >>  8) & 0xff);
			TempWord[3] = aes_sbox_enc[TempWord[0]]^((Temprcon      ) & 0xff);
			TempWord[0] = Temp;
		}
		else if ((NumberOfWordOfKey > 6) && ((KeyIndex % NumberOfWordOfKey) == 4)) {
			Temp = aes_sbox_enc[TempWord[0]];
			TempWord[1] = aes_sbox_enc[TempWord[1]];
			TempWord[2] = aes_sbox_enc[TempWord[2]];
			TempWord[3] = aes_sbox_enc[TempWord[3]];
			TempWord[0] = Temp;
		}
		paes_ctx->KeyWordExpansion[0][KeyIndex] = paes_ctx->KeyWordExpansion[0][KeyIndex - NumberOfWordOfKey]^TempWord[0];
		paes_ctx->KeyWordExpansion[1][KeyIndex] = paes_ctx->KeyWordExpansion[1][KeyIndex - NumberOfWordOfKey]^TempWord[1];
		paes_ctx->KeyWordExpansion[2][KeyIndex] = paes_ctx->KeyWordExpansion[2][KeyIndex - NumberOfWordOfKey]^TempWord[2];
		paes_ctx->KeyWordExpansion[3][KeyIndex] = paes_ctx->KeyWordExpansion[3][KeyIndex - NumberOfWordOfKey]^TempWord[3];
		KeyIndex++;
	} /* End of while */

	return;
} /* End of RT_AES_KeyExpansion */


/*
========================================================================
Routine Description:
    AES encryption

Arguments:
    PlainBlock       The block of plain text, 16 bytes(128 bits) each block
    PlainBlockSize   The length of block of plain text in bytes
    Key              Cipher key, it may be 16, 24, or 32 bytes (128, 192, or 256 bits)
    KeyLength        The length of cipher key in bytes
    CipherBlockSize  The length of allocated cipher block in bytes

Return Value:
    CipherBlock      Return cipher text
    CipherBlockSize  Return the length of real used cipher block in bytes

Note:
    Reference to FIPS-PUB 197
    1. Check if block size is 16 bytes(128 bits) and if key length is 16, 24, or 32 bytes(128, 192, or 256 bits)
    2. Transfer the plain block to state block 
    3. Main encryption rounds
    4. Transfer the state block to cipher block
    ------------------------------------------
       NumberOfRound = (key length / 4) + 6;
       state block = plain block;
       
       AddRoundKey(state block, key);
       for round = 1 to NumberOfRound
           SubBytes(state block)
           ShiftRows(state block)
           MixColumns(state block)
           AddRoundKey(state block, key);
       end for

       SubBytes(state block)
       ShiftRows(state block)
       AddRoundKey(state block, key);

       cipher block = state block;
========================================================================
*/
void RT_AES_Encrypt (
    IN uint8 PlainBlock[],
    IN uint32 PlainBlockSize,
    IN uint8 Key[],
    IN uint32 KeyLength,
    OUT uint8 CipherBlock[],
    IN_OUT uint32 *CipherBlockSize)
{
	AES_CTX_STRUC aes_ctx;
	uint32 RowIndex = 0, ColumnIndex = 0;
	uint32 RoundIndex = 0, NumberOfRound = 0;
	uint8 Temp = 0, Row0 = 0, Row1 = 0, Row2 = 0, Row3 = 0;

	if((PlainBlock == NULL) || (Key == NULL) || (CipherBlock == NULL) || (CipherBlockSize == NULL))
		return;
	
	/*   
     * 1. Check if block size is 16 bytes(128 bits) and if key length is 16, 24, or 32 bytes(128, 192, or 256 bits) 
     */
	if (PlainBlockSize != AES_BLOCK_SIZES) {
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n RT_AES_Encrypt: plain block size must be 16 bytes(128 bits).\n\r\n");
		return;
	}
	if ((KeyLength != AES_KEY128_LENGTH) && (KeyLength != AES_KEY192_LENGTH) && (KeyLength != AES_KEY256_LENGTH)) {
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n RT_AES_Encrypt:  key length must be 16, 24 or 32 bytes(128, 192, or 256 bits).\n\r\n");
		return;
	}
	if (*CipherBlockSize < AES_BLOCK_SIZES) {
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n RT_AES_Encrypt:  cipher block size must be 16 bytes(128 bits).\n\r\n");
		return;
	}
	
	/* 
     * 2. Transfer the plain block to state block 
     */
     	memset(&aes_ctx, 0, sizeof(aes_ctx));
	for (RowIndex = 0; RowIndex < AES_STATE_ROWS;RowIndex++)
		for (ColumnIndex = 0; ColumnIndex < AES_STATE_COLUMNS;ColumnIndex++)
			aes_ctx.State[RowIndex][ColumnIndex] = PlainBlock[RowIndex + 4*ColumnIndex];

	/* 
     *  3. Main encryption rounds
     */
	RT_AES_KeyExpansion(Key, KeyLength, &aes_ctx);
	NumberOfRound = (KeyLength >> 2) + 6;
	
	/* AES_AddRoundKey */
	RoundIndex = 0;
	for (RowIndex = 0; RowIndex < AES_STATE_ROWS;RowIndex++)
		for (ColumnIndex = 0; ColumnIndex < AES_STATE_COLUMNS;ColumnIndex++)
			aes_ctx.State[RowIndex][ColumnIndex] ^= aes_ctx.KeyWordExpansion[RowIndex][(RoundIndex*((uint32) AES_STATE_COLUMNS)) + ColumnIndex];

	for (RoundIndex = 1; RoundIndex < NumberOfRound;RoundIndex++)
	{
		/* AES_SubBytes */
		for (RowIndex = 0; RowIndex < AES_STATE_ROWS;RowIndex++)
			for (ColumnIndex = 0; ColumnIndex < AES_STATE_COLUMNS;ColumnIndex++)
				aes_ctx.State[RowIndex][ColumnIndex] = aes_sbox_enc[aes_ctx.State[RowIndex][ColumnIndex]];

		/* AES_ShiftRows */
		Temp = aes_ctx.State[1][0];
		aes_ctx.State[1][0] = aes_ctx.State[1][1];
		aes_ctx.State[1][1] = aes_ctx.State[1][2];
		aes_ctx.State[1][2] = aes_ctx.State[1][3];
		aes_ctx.State[1][3] = Temp;
		Temp = aes_ctx.State[2][0];
		aes_ctx.State[2][0] = aes_ctx.State[2][2];
		aes_ctx.State[2][2] = Temp;
		Temp = aes_ctx.State[2][1];
		aes_ctx.State[2][1] = aes_ctx.State[2][3];
		aes_ctx.State[2][3] = Temp;
		Temp = aes_ctx.State[3][3];
		aes_ctx.State[3][3] = aes_ctx.State[3][2];
		aes_ctx.State[3][2] = aes_ctx.State[3][1];
		aes_ctx.State[3][1] = aes_ctx.State[3][0];
		aes_ctx.State[3][0] = Temp;

		/* AES_MixColumns */
		for (ColumnIndex = 0; ColumnIndex < AES_STATE_COLUMNS;ColumnIndex++)
		{
			Row0 = aes_ctx.State[0][ColumnIndex];
			Row1 = aes_ctx.State[1][ColumnIndex];
			Row2 = aes_ctx.State[2][ColumnIndex];
			Row3 = aes_ctx.State[3][ColumnIndex];
			aes_ctx.State[0][ColumnIndex] = aes_mul_2[Row0]^aes_mul_3[Row1]^Row2^Row3;
			aes_ctx.State[1][ColumnIndex] = Row0^aes_mul_2[Row1]^aes_mul_3[Row2]^Row3;
			aes_ctx.State[2][ColumnIndex] = Row0^Row1^aes_mul_2[Row2]^aes_mul_3[Row3];
			aes_ctx.State[3][ColumnIndex] = aes_mul_3[Row0]^Row1^Row2^aes_mul_2[Row3];
		}

		/* AES_AddRoundKey */
		for (RowIndex = 0; RowIndex < AES_STATE_ROWS;RowIndex++)
			for (ColumnIndex = 0; ColumnIndex < AES_STATE_COLUMNS;ColumnIndex++)
				aes_ctx.State[RowIndex][ColumnIndex] ^= aes_ctx.KeyWordExpansion[RowIndex][(RoundIndex*((uint32) AES_STATE_COLUMNS)) + ColumnIndex];
	} /* End of for */

	/* AES_SubBytes */
	for (RowIndex = 0; RowIndex < AES_STATE_ROWS;RowIndex++)
		for (ColumnIndex = 0; ColumnIndex < AES_STATE_COLUMNS;ColumnIndex++)
			aes_ctx.State[RowIndex][ColumnIndex] = aes_sbox_enc[aes_ctx.State[RowIndex][ColumnIndex]];

	/* AES_ShiftRows */
	Temp = aes_ctx.State[1][0];
	aes_ctx.State[1][0] = aes_ctx.State[1][1];
	aes_ctx.State[1][1] = aes_ctx.State[1][2];
	aes_ctx.State[1][2] = aes_ctx.State[1][3];
	aes_ctx.State[1][3] = Temp;
	Temp = aes_ctx.State[2][0];
	aes_ctx.State[2][0] = aes_ctx.State[2][2];
	aes_ctx.State[2][2] = Temp;
	Temp = aes_ctx.State[2][1];
	aes_ctx.State[2][1] = aes_ctx.State[2][3];
	aes_ctx.State[2][3] = Temp;
	Temp = aes_ctx.State[3][3];
	aes_ctx.State[3][3] = aes_ctx.State[3][2];
	aes_ctx.State[3][2] = aes_ctx.State[3][1];
	aes_ctx.State[3][1] = aes_ctx.State[3][0];
	aes_ctx.State[3][0] = Temp;
	/* AES_AddRoundKey */
	for (RowIndex = 0; RowIndex < AES_STATE_ROWS;RowIndex++)
		for (ColumnIndex = 0; ColumnIndex < AES_STATE_COLUMNS;ColumnIndex++)
			aes_ctx.State[RowIndex][ColumnIndex] ^= aes_ctx.KeyWordExpansion[RowIndex][(RoundIndex*((uint32) AES_STATE_COLUMNS)) + ColumnIndex];

	/* 
     * 4. Transfer the state block to cipher block 
     */
	for (RowIndex = 0; RowIndex < AES_STATE_ROWS;RowIndex++)
		for (ColumnIndex = 0; ColumnIndex < AES_STATE_COLUMNS;ColumnIndex++)
			CipherBlock[RowIndex + 4*ColumnIndex] = aes_ctx.State[RowIndex][ColumnIndex];

	*CipherBlockSize = ((uint32) AES_STATE_ROWS)*((uint32) AES_STATE_COLUMNS);

	return;
} /* End of RT_AES_Encrypt */


/*
========================================================================
Routine Description:
    AES-CMAC generate subkey

Arguments:
    Key        Cipher key 128 bits
    KeyLength  The length of Cipher key in bytes

Return Value:
    SubKey1    SubKey 1 128 bits
    SubKey2    SubKey 2 128 bits

Note:
    Reference to RFC 4493
    
    Step 1.  L := AES-128(K, const_Zero);
    Step 2.  if MSB(L) is equal to 0
                then    K1 := L << 1;
                else    K1 := (L << 1) XOR const_Rb;
    Step 3.  if MSB(K1) is equal to 0
                then    K2 := K1 << 1;
                else    K2 := (K1 << 1) XOR const_Rb;
    Step 4.  return K1, K2;
========================================================================
*/
void AES_CMAC_GenerateSubKey (
    IN uint8 Key[],
    IN uint32 KeyLength,
    OUT uint8 SubKey1[],
    OUT uint8 SubKey2[])
{
	uint8 MSB_L = 0, MSB_K1 = 0, Top_Bit = 0;
	uint32  SubKey1_Length = 0;
	int   Index = 0;

	if((Key == NULL) || (SubKey1 == NULL) || (SubKey2 == NULL))
		return;
	
	if (KeyLength != AES_KEY128_LENGTH) {
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n AES_CMAC_GenerateSubKey:key length must be 16 bytes(128 bits).\n\r\n");
		return;
	}

	/* Step 1: L := AES-128(K, const_Zero); */
	SubKey1_Length = 16;
	RT_AES_Encrypt(Const_Zero, sizeof(Const_Zero), Key, KeyLength, SubKey1, &SubKey1_Length);

	/*
	* Step 2.  if MSB(L) is equal to 0
     *           then    K1 := L << 1;
     *           else    K1 := (L << 1) XOR const_Rb;
     */
	MSB_L = SubKey1[0] & 0x80;    
	for(Index = 0; Index < 15; Index++) {
		Top_Bit = (SubKey1[Index + 1] & 0x80)?1:0;
		SubKey1[Index] <<= 1;
		SubKey1[Index] |= Top_Bit;
	}
	SubKey1[15] <<= 1;
	if (MSB_L > 0) {
		for(Index = 0; Index < 16; Index++)
			SubKey1[Index] ^= Const_Rb[Index];
	} 

	/*
     * Step 3.  if MSB(K1) is equal to 0
     *           then    K2 := K1 << 1;
     *           else    K2 := (K1 << 1) XOR const_Rb;
     */
	MSB_K1 = SubKey1[0] & 0x80;
	for(Index = 0; Index < 15; Index++) {
		Top_Bit = (SubKey1[Index + 1] & 0x80)?1:0;
		SubKey2[Index] = SubKey1[Index] << 1;
		SubKey2[Index] |= Top_Bit;
	}
	SubKey2[15] = SubKey1[15] << 1;
	if (MSB_K1 > 0) {
		for(Index = 0; Index < 16; Index++)
			SubKey2[Index] ^= Const_Rb[Index];
	} 

	return;
} /* End of AES_CMAC_GenerateSubKey */


/*
========================================================================
Routine Description:
    AES-CMAC

Arguments:
    PlainText        Plain text
    PlainTextLength  The length of plain text in bytes
    Key              Cipher key, it may be 16, 24, or 32 bytes (128, 192, or 256 bits)
    KeyLength        The length of cipher key in bytes
    MACTextLength    The length of allocated memory spaces in bytes

Return Value:
    MACText       Message authentication code (128-bit string)
    MACTextLength Return the length of Message authentication code in bytes

Note:
    Reference to RFC 4493
========================================================================
*/
void AES_CMAC (
    IN uint8 PlainText[],
    IN uint32 PlainTextLength,
    IN uint8 Key[],
    IN uint32 KeyLength,
    OUT uint8 MACText[],
    IN_OUT uint32 *MACTextLength)
{
	uint32  PlainBlockStart = 0;
	uint8 X[AES_BLOCK_SIZES] = {0}, Y[AES_BLOCK_SIZES] = {0};
	uint8 SubKey1[16] = {0};
	uint8 SubKey2[16] = {0};
	int Index = 0;
	uint32 X_Length = 0;

	if((PlainText == NULL) || (Key == NULL) || (MACText == NULL) || (MACTextLength == NULL))
		return;
	
	if (*MACTextLength < AES_MAC_LENGTH) {
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n AES_CMAC:MAC text length is less than 16 bytes.\n\r\n");
		return;
	}

	if (KeyLength != AES_KEY128_LENGTH) {
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n AES_CMAC:key length must be 16 bytes(128 bits).\n\r\n");
		return;
	} 
	
	/* Step 1.  (K1,K2) := Generate_Subkey(K); */
	memset(SubKey1, 0, 16);
	memset(SubKey2, 0, 16);  
	AES_CMAC_GenerateSubKey(Key, KeyLength, SubKey1, SubKey2);
	/*   
     * 2. Main algorithm
     *    - Plain text divide into serveral blocks (16 bytes/block)
     *    - If plain text is not divided with no remainder by block, padding size = (block - remainder plain text)
     *    - Execute RT_AES_Encrypt procedure.
     */
	PlainBlockStart = 0;
	memmove(X, Const_Zero, AES_BLOCK_SIZES);
	while ((PlainTextLength - PlainBlockStart) > AES_BLOCK_SIZES)
	{
		for (Index = 0; Index < AES_BLOCK_SIZES; Index++){
			Y[Index] = PlainText[PlainBlockStart + Index]^X[Index];	
	}
	X_Length = sizeof(X);
	RT_AES_Encrypt(Y, sizeof(Y) , Key, KeyLength, X, (uint32 *)&X_Length);
	PlainBlockStart += ((uint32) AES_BLOCK_SIZES);
	}
	
	if ((PlainTextLength - PlainBlockStart) == AES_BLOCK_SIZES) {
		for (Index = 0; Index < AES_BLOCK_SIZES; Index++)
			Y[Index] = PlainText[PlainBlockStart + Index]^X[Index]^SubKey1[Index];        
	}
	else
	{
		memset(Y, 0, AES_BLOCK_SIZES);
		memmove(Y, &PlainText[PlainBlockStart], (PlainTextLength - PlainBlockStart));
		Y[(PlainTextLength - PlainBlockStart)] = 0x80;
		for (Index = 0; Index < AES_BLOCK_SIZES; Index++)
			Y[Index] = Y[Index]^X[Index]^SubKey2[Index];
	}
	RT_AES_Encrypt(Y, sizeof(Y) , Key, KeyLength, MACText, MACTextLength);

	return;
} /* End of AES_CMAC */




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
	crypt_aes.h

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	lisa.xue	2012/7/28	Create
*/

#ifndef __CRYPT_AES_H__
#define __CRYPT_AES_H__

#include "omci_types.h"

/* AES definition & structure */
#define AES_STATE_ROWS 4     /* Block size: 4*4*8 = 128 bits */
#define AES_STATE_COLUMNS 4
#define AES_BLOCK_SIZES AES_STATE_ROWS*AES_STATE_COLUMNS
#define AES_KEY_ROWS 4
#define AES_KEY_COLUMNS 8    /*Key length: 4*{4,6,8}*8 = 128, 192, 256 bits */
#define AES_KEY128_LENGTH 16
#define AES_KEY192_LENGTH 24
#define AES_KEY256_LENGTH 32

typedef struct {
    uint8 State[AES_STATE_ROWS][AES_STATE_COLUMNS];
    uint8 KeyWordExpansion[AES_KEY_ROWS][AES_KEY_ROWS*((AES_KEY256_LENGTH >> 2) + 6 + 1)];
} AES_CTX_STRUC, *PAES_CTX_STRUC;


/* AES operations */
void RT_AES_KeyExpansion (
    IN uint8 Key[],
    IN uint32 KeyLength,
    IN_OUT AES_CTX_STRUC *paes_ctx);

void RT_AES_Encrypt (
    IN uint8 PlainBlock[],
    IN uint32 PlainBlockSize,
    IN uint8 Key[],
    IN uint32 KeyLength,
    OUT uint8 CipherBlock[],
    IN_OUT uint32 *CipherBlockSize);


/* AES-CMAC operations */
void AES_CMAC_GenerateSubKey (
    IN uint8 Key[],
    IN uint32 KeyLength,
    OUT uint8 SubKey1[],
    OUT uint8 SubKey2[]);

void AES_CMAC (
    IN uint8 PlainText[],
    IN uint32 PlainTextLength,
    IN uint8 Key[],
    IN uint32 KeyLength,
    OUT uint8 MACText[],
    IN_OUT uint32 *MACTextLength);

#endif


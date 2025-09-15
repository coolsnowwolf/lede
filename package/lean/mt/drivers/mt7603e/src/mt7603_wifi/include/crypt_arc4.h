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
    RC4

    Abstract:
    
    Revision History:
    Who         When            What
    --------    ----------      ------------------------------------------
    Eddy        2009/05/13      ARC4
***************************************************************************/

#ifndef __CRYPT_ARC4_H__
#define __CRYPT_ARC4_H__

#include "rt_config.h"

/* ARC4 definition & structure */
#define ARC4_KEY_BLOCK_SIZE 256

typedef struct {
	UINT BlockIndex1;
	UINT BlockIndex2;
	UINT8 KeyBlock[256];
} ARC4_CTX_STRUC, *PARC4_CTX_STRUC;

/* ARC4 operations */
VOID ARC4_INIT(
	IN ARC4_CTX_STRUC * pARC4_CTX,
	IN PUCHAR pKey,
	IN UINT KeyLength);

VOID ARC4_Compute(
	IN ARC4_CTX_STRUC * pARC4_CTX,
	IN UINT8 InputBlock[],
	IN UINT InputBlockSize,
	OUT UINT8 OutputBlock[]);

VOID ARC4_Discard_KeyLength(
	IN ARC4_CTX_STRUC * pARC4_CTX,
	IN UINT Length);

#endif /* __CRYPT_ARC4_H__ */

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
    DH

    Abstract:
    RFC 2631: Diffie-Hellman Key Agreement Method
    
    Revision History:
    Who         When            What
    --------    ----------      ------------------------------------------
    Eddy        2009/01/21      Create Diffie-Hellman, Montgomery Algorithm
***************************************************************************/

#ifndef __CRYPT_DH_H__
#define __CRYPT_DH_H__

#include "rt_config.h"


/* DH operations */
void DH_PublicKey_Generate (
    IN UINT8 GValue[],
    IN UINT GValueLength,
    IN UINT8 PValue[],
    IN UINT PValueLength,
    IN UINT8 PrivateKey[],
    IN UINT PrivateKeyLength,
    OUT UINT8 PublicKey[],
    INOUT UINT *PublicKeyLength);

void DH_SecretKey_Generate (
    IN UINT8 PublicKey[],
    IN UINT PublicKeyLength,
    IN UINT8 PValue[],
    IN UINT PValueLength,
    IN UINT8 PrivateKey[],
    IN UINT PrivateKeyLength,
    OUT UINT8 SecretKey[],
    INOUT UINT *SecretKeyLength);

#define RT_DH_PublicKey_Generate(GK, GKL, PV, PVL, PriK, PriKL, PubK, PubKL) \
    DH_PublicKey_Generate((GK), (GKL), (PV), (PVL), (PriK), (PriKL), (UINT8 *) (PubK), (UINT *) (PubKL))

#define RT_DH_SecretKey_Generate(PubK, PubKL, PV, PVL, PriK, PriKL, SecK, SecKL) \
    DH_SecretKey_Generate((PubK), (PubKL), (PV), (PVL), (PriK), (PriKL), (UINT8 *) (SecK), (UINT *) (SecKL))

#define RT_DH_FREE_ALL()

    
#endif /* __CRYPT_DH_H__ */


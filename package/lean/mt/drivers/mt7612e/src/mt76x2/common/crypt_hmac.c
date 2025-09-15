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
    HMAC

    Abstract:
    FIPS 198: The Keyed-Hash Message Authentication Code (HMAC)
    
    Revision History:
    Who         When            What
    --------    ----------      ------------------------------------------
    Eddy        2008/11/24      Create HMAC-SHA1, HMAC-SHA256
***************************************************************************/

#include "crypt_hmac.h"


#ifdef HMAC_SHA1_SUPPORT
/*
========================================================================
Routine Description:
    HMAC using SHA1 hash function

Arguments:
    key             Secret key
    key_len         The length of the key in bytes   
    message         Message context
    message_len     The length of message in bytes
    macLen          Request the length of message authentication code

Return Value:
    mac             Message authentication code

Note:
    None
========================================================================
*/
VOID RT_HMAC_SHA1 (
    IN  const UINT8 Key[], 
    IN  UINT KeyLen, 
    IN  const UINT8 Message[], 
    IN  UINT MessageLen, 
    OUT UINT8 MAC[],
    IN  UINT MACLen)    
{
    SHA1_CTX_STRUC sha_ctx1;
    SHA1_CTX_STRUC sha_ctx2;
    UINT8 K0[SHA1_BLOCK_SIZE];
    UINT8 Digest[SHA1_DIGEST_SIZE];    
    UINT index;

    NdisZeroMemory(&sha_ctx1, sizeof(SHA1_CTX_STRUC));
    NdisZeroMemory(&sha_ctx2, sizeof(SHA1_CTX_STRUC));    
    /*
     * If the length of K = B(Block size): K0 = K.
     * If the length of K > B: hash K to obtain an L byte string, 
     * then append (B-L) zeros to create a B-byte string K0 (i.e., K0 = H(K) || 00...00).
     * If the length of K < B: append zeros to the end of K to create a B-byte string K0
     */
    NdisZeroMemory(K0, SHA1_BLOCK_SIZE);
    if (KeyLen <= SHA1_BLOCK_SIZE)
        NdisMoveMemory(K0, Key, KeyLen);
    else
        RT_SHA1(Key, KeyLen, K0);
    /* End of if */

    /* Exclusive-Or K0 with ipad */
    /* ipad: Inner pad; the byte x・36・ repeated B times. */
    for (index = 0; index < SHA1_BLOCK_SIZE; index++)
        K0[index] ^= 0x36;
        /* End of for */

    RT_SHA1_Init(&sha_ctx1);
    /* H(K0^ipad) */
    RT_SHA1_Append(&sha_ctx1, K0, sizeof(K0));
    /* H((K0^ipad)||text) */
    RT_SHA1_Append(&sha_ctx1, Message, MessageLen);
    RT_SHA1_End(&sha_ctx1, Digest);

    /* Exclusive-Or K0 with opad and remove ipad */
    /* opad: Outer pad; the byte x・5c・ repeated B times. */
    for (index = 0; index < SHA1_BLOCK_SIZE; index++)
        K0[index] ^= 0x36^0x5c;
        /* End of for */

    RT_SHA1_Init(&sha_ctx2);
    /* H(K0^opad) */
    RT_SHA1_Append(&sha_ctx2, K0, sizeof(K0));
    /* H( (K0^opad) || H((K0^ipad)||text) ) */
    RT_SHA1_Append(&sha_ctx2, Digest, SHA1_DIGEST_SIZE);
    RT_SHA1_End(&sha_ctx2, Digest);

    if (MACLen > SHA1_DIGEST_SIZE)
        NdisMoveMemory(MAC, Digest, SHA1_DIGEST_SIZE);
    else
        NdisMoveMemory(MAC, Digest, MACLen);    
} /* End of RT_HMAC_SHA1 */
#endif /* HMAC_SHA1_SUPPORT */


#ifdef HMAC_SHA256_SUPPORT
/*
========================================================================
Routine Description:
    HMAC using SHA256 hash function

Arguments:
    key             Secret key
    key_len         The length of the key in bytes   
    message         Message context
    message_len     The length of message in bytes
    macLen          Request the length of message authentication code

Return Value:
    mac             Message authentication code

Note:
    None
========================================================================
*/
VOID RT_HMAC_SHA256 (
    IN  const UINT8 Key[], 
    IN  UINT KeyLen, 
    IN  const UINT8 Message[], 
    IN  UINT MessageLen, 
    OUT UINT8 MAC[],
    IN  UINT MACLen)
{
    SHA256_CTX_STRUC sha_ctx1;
    SHA256_CTX_STRUC sha_ctx2;
    UINT8 K0[SHA256_BLOCK_SIZE];
    UINT8 Digest[SHA256_DIGEST_SIZE];
    UINT index;

    NdisZeroMemory(&sha_ctx1, sizeof(SHA256_CTX_STRUC));
    NdisZeroMemory(&sha_ctx2, sizeof(SHA256_CTX_STRUC));
    /*
     * If the length of K = B(Block size): K0 = K.
     * If the length of K > B: hash K to obtain an L byte string, 
     * then append (B-L) zeros to create a B-byte string K0 (i.e., K0 = H(K) || 00...00).
     * If the length of K < B: append zeros to the end of K to create a B-byte string K0
     */
    NdisZeroMemory(K0, SHA256_BLOCK_SIZE);
    if (KeyLen <= SHA256_BLOCK_SIZE) {
        NdisMoveMemory(K0, Key, KeyLen);
    } else {
        RT_SHA256(Key, KeyLen, K0);
    }

    /* Exclusive-Or K0 with ipad */
    /* ipad: Inner pad; the byte x・36・ repeated B times. */
    for (index = 0; index < SHA256_BLOCK_SIZE; index++)
        K0[index] ^= 0x36;
        /* End of for */
        
    RT_SHA256_Init(&sha_ctx1);
    /* H(K0^ipad) */
    RT_SHA256_Append(&sha_ctx1, K0, sizeof(K0));
    /* H((K0^ipad)||text) */
    RT_SHA256_Append(&sha_ctx1, Message, MessageLen);  
    RT_SHA256_End(&sha_ctx1, Digest);

    /* Exclusive-Or K0 with opad and remove ipad */
    /* opad: Outer pad; the byte x・5c・ repeated B times. */
    for (index = 0; index < SHA256_BLOCK_SIZE; index++)
        K0[index] ^= 0x36^0x5c;
        /* End of for */
        
    RT_SHA256_Init(&sha_ctx2);
    /* H(K0^opad) */
    RT_SHA256_Append(&sha_ctx2, K0, sizeof(K0));
    /* H( (K0^opad) || H((K0^ipad)||text) ) */
    RT_SHA256_Append(&sha_ctx2, Digest, SHA256_DIGEST_SIZE);
    RT_SHA256_End(&sha_ctx2, Digest);

    if (MACLen > SHA256_DIGEST_SIZE)
        NdisMoveMemory(MAC, Digest,SHA256_DIGEST_SIZE);
    else
        NdisMoveMemory(MAC, Digest, MACLen);
    
} /* End of RT_HMAC_SHA256 */
#endif /* HMAC_SHA256_SUPPORT */


#ifdef HMAC_MD5_SUPPORT
/*
========================================================================
Routine Description:
    HMAC using MD5 hash function

Arguments:
    key             Secret key
    key_len         The length of the key in bytes   
    message         Message context
    message_len     The length of message in bytes
    macLen          Request the length of message authentication code

Return Value:
    mac             Message authentication code

Note:
    None
========================================================================
*/
VOID RT_HMAC_MD5(
    IN  const UINT8 Key[], 
    IN  UINT KeyLen, 
    IN  const UINT8 Message[], 
    IN  UINT MessageLen, 
    OUT UINT8 MAC[],
    IN  UINT MACLen)    
{
    MD5_CTX_STRUC md5_ctx1;
    MD5_CTX_STRUC md5_ctx2;
    UINT8 K0[MD5_BLOCK_SIZE];
    UINT8 Digest[MD5_DIGEST_SIZE];    
    UINT index;

    NdisZeroMemory(&md5_ctx1, sizeof(MD5_CTX_STRUC));
    NdisZeroMemory(&md5_ctx2, sizeof(MD5_CTX_STRUC));
    /*
     * If the length of K = B(Block size): K0 = K.
     * If the length of K > B: hash K to obtain an L byte string, 
     * then append (B-L) zeros to create a B-byte string K0 (i.e., K0 = H(K) || 00...00).
     * If the length of K < B: append zeros to the end of K to create a B-byte string K0
     */
    NdisZeroMemory(K0, MD5_BLOCK_SIZE);
    if (KeyLen <= MD5_BLOCK_SIZE) {
        NdisMoveMemory(K0, Key, KeyLen);
    } else {
        RT_MD5(Key, KeyLen, K0);
    }

    /* Exclusive-Or K0 with ipad */
    /* ipad: Inner pad; the byte x・36・ repeated B times. */
    for (index = 0; index < MD5_BLOCK_SIZE; index++)
        K0[index] ^= 0x36;
        /* End of for */
        
    RT_MD5_Init(&md5_ctx1);
    /* H(K0^ipad) */
    RT_MD5_Append(&md5_ctx1, K0, sizeof(K0));
    /* H((K0^ipad)||text) */
    RT_MD5_Append(&md5_ctx1, Message, MessageLen);  
    RT_MD5_End(&md5_ctx1, Digest);

    /* Exclusive-Or K0 with opad and remove ipad */
    /* opad: Outer pad; the byte x・5c・ repeated B times. */
    for (index = 0; index < MD5_BLOCK_SIZE; index++)
        K0[index] ^= 0x36^0x5c;
        /* End of for */
        
    RT_MD5_Init(&md5_ctx2);
    /* H(K0^opad) */
    RT_MD5_Append(&md5_ctx2, K0, sizeof(K0));
    /* H( (K0^opad) || H((K0^ipad)||text) ) */
    RT_MD5_Append(&md5_ctx2, Digest, MD5_DIGEST_SIZE);
    RT_MD5_End(&md5_ctx2, Digest);

    if (MACLen > MD5_DIGEST_SIZE)
        NdisMoveMemory(MAC, Digest, MD5_DIGEST_SIZE);
    else
        NdisMoveMemory(MAC, Digest, MACLen);    
} /* End of RT_HMAC_SHA256 */
#endif /* HMAC_MD5_SUPPORT */


/* End of crypt_hmac.c */


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
    SHA2

    Abstract:
    FIPS 180-2: Secure Hash Standard (SHS)
    
    Revision History:
    Who         When            What
    --------    ----------      ------------------------------------------
    Eddy        2008/11/24      Create SHA1
    Eddy        2008/07/23      Create SHA256
***************************************************************************/

#include "rt_config.h"


/* Basic operations */
#define SHR(x,n) (x >> n) /* SHR(x)^n, right shift n bits , x is w-bit word, 0 <= n <= w */
#define ROTR(x,n,w) ((x >> n) | (x << (w - n))) /* ROTR(x)^n, circular right shift n bits , x is w-bit word, 0 <= n <= w */
#define ROTL(x,n,w) ((x << n) | (x >> (w - n))) /* ROTL(x)^n, circular left shift n bits , x is w-bit word, 0 <= n <= w */
#define ROTR32(x,n) ROTR(x,n,32) /* 32 bits word */
#define ROTL32(x,n) ROTL(x,n,32) /* 32 bits word */ 
#if defined(DOT11_SAE_SUPPORT) || defined(CONFIG_OWE_SUPPORT)
#define ROTR64(x, n) ROTR(x, n, 64) /* 64 bits word */
#define ROTL64(x, n) ROTL(x, n, 64) /* 64 bits word */
#endif

/* Basic functions */
#define Ch(x,y,z) ((x & y) ^ ((~x) & z))
#define Maj(x,y,z) ((x & y) ^ (x & z) ^ (y & z))
#define Parity(x,y,z) (x ^ y ^ z)

#ifdef SHA1_SUPPORT
/* SHA1 constants */
#define SHA1_MASK 0x0000000f
static const UINT32 SHA1_K[4] = {
    0x5a827999UL, 0x6ed9eba1UL, 0x8f1bbcdcUL, 0xca62c1d6UL
};
static const UINT32 SHA1_DefaultHashValue[5] = {
    0x67452301UL, 0xefcdab89UL, 0x98badcfeUL, 0x10325476UL, 0xc3d2e1f0UL
};
#endif /* SHA1_SUPPORT */


#ifdef SHA256_SUPPORT
/* SHA256 functions */
#define Zsigma_256_0(x) (ROTR32(x,2) ^ ROTR32(x,13) ^ ROTR32(x,22))
#define Zsigma_256_1(x) (ROTR32(x,6) ^ ROTR32(x,11) ^ ROTR32(x,25))
#define Sigma_256_0(x)  (ROTR32(x,7) ^ ROTR32(x,18) ^ SHR(x,3))
#define Sigma_256_1(x)  (ROTR32(x,17) ^ ROTR32(x,19) ^ SHR(x,10))
/* SHA256 constants */
static const UINT32 SHA256_K[64] = {
    0x428a2f98UL, 0x71374491UL, 0xb5c0fbcfUL, 0xe9b5dba5UL, 
    0x3956c25bUL, 0x59f111f1UL, 0x923f82a4UL, 0xab1c5ed5UL, 
    0xd807aa98UL, 0x12835b01UL, 0x243185beUL, 0x550c7dc3UL, 
    0x72be5d74UL, 0x80deb1feUL, 0x9bdc06a7UL, 0xc19bf174UL, 
    0xe49b69c1UL, 0xefbe4786UL, 0x0fc19dc6UL, 0x240ca1ccUL, 
    0x2de92c6fUL, 0x4a7484aaUL, 0x5cb0a9dcUL, 0x76f988daUL, 
    0x983e5152UL, 0xa831c66dUL, 0xb00327c8UL, 0xbf597fc7UL, 
    0xc6e00bf3UL, 0xd5a79147UL, 0x06ca6351UL, 0x14292967UL, 
    0x27b70a85UL, 0x2e1b2138UL, 0x4d2c6dfcUL, 0x53380d13UL, 
    0x650a7354UL, 0x766a0abbUL, 0x81c2c92eUL, 0x92722c85UL,
    0xa2bfe8a1UL, 0xa81a664bUL, 0xc24b8b70UL, 0xc76c51a3UL, 
    0xd192e819UL, 0xd6990624UL, 0xf40e3585UL, 0x106aa070UL, 
    0x19a4c116UL, 0x1e376c08UL, 0x2748774cUL, 0x34b0bcb5UL, 
    0x391c0cb3UL, 0x4ed8aa4aUL, 0x5b9cca4fUL, 0x682e6ff3UL, 
    0x748f82eeUL, 0x78a5636fUL, 0x84c87814UL, 0x8cc70208UL, 
    0x90befffaUL, 0xa4506cebUL, 0xbef9a3f7UL, 0xc67178f2UL
};
static const UINT32 SHA256_DefaultHashValue[8] = {
    0x6a09e667UL, 0xbb67ae85UL, 0x3c6ef372UL, 0xa54ff53aUL,
    0x510e527fUL, 0x9b05688cUL, 0x1f83d9abUL, 0x5be0cd19UL
};
#endif /* SHA256_SUPPORT */

#if defined(DOT11_SAE_SUPPORT) || defined(CONFIG_OWE_SUPPORT)
#ifdef SHA384_SUPPORT
/* SHA384 functions */
#define Zsigma_512_0(x) (ROTR64(x, 28) ^ ROTR64(x, 34) ^ ROTR64(x, 39))
#define Zsigma_512_1(x) (ROTR64(x, 14) ^ ROTR64(x, 18) ^ ROTR64(x, 41))
#define Sigma_512_0(x)  (ROTR64(x, 1) ^ ROTR64(x, 8) ^ SHR(x, 7))
#define Sigma_512_1(x)  (ROTR64(x, 19) ^ ROTR64(x, 61) ^ SHR(x, 6))
/* SHA384 constants */
static const UINT64 SHA384_K[80] = {
	0x428a2f98d728ae22, 0x7137449123ef65cd, 0xb5c0fbcfec4d3b2f, 0xe9b5dba58189dbbc,
	0x3956c25bf348b538, 0x59f111f1b605d019, 0x923f82a4af194f9b, 0xab1c5ed5da6d8118,
	0xd807aa98a3030242, 0x12835b0145706fbe, 0x243185be4ee4b28c, 0x550c7dc3d5ffb4e2,
	0x72be5d74f27b896f, 0x80deb1fe3b1696b1, 0x9bdc06a725c71235, 0xc19bf174cf692694,
	0xe49b69c19ef14ad2, 0xefbe4786384f25e3, 0x0fc19dc68b8cd5b5, 0x240ca1cc77ac9c65,
	0x2de92c6f592b0275, 0x4a7484aa6ea6e483, 0x5cb0a9dcbd41fbd4, 0x76f988da831153b5,
	0x983e5152ee66dfab, 0xa831c66d2db43210, 0xb00327c898fb213f, 0xbf597fc7beef0ee4,
	0xc6e00bf33da88fc2, 0xd5a79147930aa725, 0x06ca6351e003826f, 0x142929670a0e6e70,
	0x27b70a8546d22ffc, 0x2e1b21385c26c926, 0x4d2c6dfc5ac42aed, 0x53380d139d95b3df,
	0x650a73548baf63de, 0x766a0abb3c77b2a8, 0x81c2c92e47edaee6, 0x92722c851482353b,
	0xa2bfe8a14cf10364, 0xa81a664bbc423001, 0xc24b8b70d0f89791, 0xc76c51a30654be30,
	0xd192e819d6ef5218, 0xd69906245565a910, 0xf40e35855771202a, 0x106aa07032bbd1b8,
	0x19a4c116b8d2d0c8, 0x1e376c085141ab53, 0x2748774cdf8eeb99, 0x34b0bcb5e19b48a8,
	0x391c0cb3c5c95a63, 0x4ed8aa4ae3418acb, 0x5b9cca4f7763e373, 0x682e6ff3d6b2b8a3,
	0x748f82ee5defb2fc, 0x78a5636f43172f60, 0x84c87814a1f0ab72, 0x8cc702081a6439ec,
	0x90befffa23631e28, 0xa4506cebde82bde9, 0xbef9a3f7b2c67915, 0xc67178f2e372532b,
	0xca273eceea26619c, 0xd186b8c721c0c207, 0xeada7dd6cde0eb1e, 0xf57d4f7fee6ed178,
	0x06f067aa72176fba, 0x0a637dc5a2c898a6, 0x113f9804bef90dae, 0x1b710b35131c471b,
	0x28db77f523047d84, 0x32caab7b40c72493, 0x3c9ebe0a15c9bebc, 0x431d67c49c100d4c,
	0x4cc5d4becb3e42b6, 0x597f299cfc657e2a, 0x5fcb6fab3ad6faec, 0x6c44198c4a475817
};
static const UINT64 SHA384_DefaultHashValue[8] = {
	0xcbbb9d5dc1059ed8, 0x629a292a367cd507, 0x9159015a3070dd17, 0x152fecd8f70e5939,
	0x67332667ffc00b31, 0x8eb44a8768581511, 0xdb0c2e0d64f98fa7, 0x47b5481dbefa4fa4
};
#endif /* SHA384_SUPPORT */
#endif

#ifdef SHA1_SUPPORT
/*
========================================================================
Routine Description:
    Initial SHA1_CTX_STRUC

Arguments:
    pSHA_CTX        Pointer to SHA1_CTX_STRUC

Return Value:
    None

Note:
    None
========================================================================
*/
VOID RT_SHA1_Init (
    IN  SHA1_CTX_STRUC *pSHA_CTX)
{
    NdisMoveMemory(pSHA_CTX->HashValue, SHA1_DefaultHashValue, 
        sizeof(SHA1_DefaultHashValue));
    NdisZeroMemory(pSHA_CTX->Block, SHA1_BLOCK_SIZE);
    pSHA_CTX->MessageLen = 0;
    pSHA_CTX->BlockLen   = 0;
} /* End of RT_SHA1_Init */


/*
========================================================================
Routine Description:
    SHA1 computation for one block (512 bits)

Arguments:
    pSHA_CTX        Pointer to SHA1_CTX_STRUC

Return Value:
    None

Note:
    None
========================================================================
*/
VOID RT_SHA1_Hash (
    IN  SHA1_CTX_STRUC *pSHA_CTX)
{
    UINT32 W_i,t;
    UINT32 W[80];
    UINT32 a,b,c,d,e,T,f_t = 0;
  
    /* Prepare the message schedule, {W_i}, 0 < t < 15 */
    NdisMoveMemory(W, pSHA_CTX->Block, SHA1_BLOCK_SIZE);
    for (W_i = 0; W_i < 16; W_i++) {
        W[W_i] = cpu2be32(W[W_i]); /* Endian Swap */
    } /* End of for */

    for (W_i = 16; W_i < 80; W_i++) {
        W[W_i] = ROTL32((W[W_i - 3] ^ W[W_i - 8] ^ W[W_i - 14] ^ W[W_i - 16]),1);
    } /* End of for */

        
    /* SHA256 hash computation */
    /* Initialize the working variables */
    a = pSHA_CTX->HashValue[0];
    b = pSHA_CTX->HashValue[1];
    c = pSHA_CTX->HashValue[2];
    d = pSHA_CTX->HashValue[3];
    e = pSHA_CTX->HashValue[4];

    /* 80 rounds */
    for (t = 0;t < 20;t++) {
        f_t = Ch(b,c,d);
        T = ROTL32(a,5) + f_t + e + SHA1_K[0] + W[t];
        e = d;
        d = c;
        c = ROTL32(b,30);
        b = a;
        a = T;
     } /* End of for */
    for (t = 20;t < 40;t++) {
        f_t = Parity(b,c,d);
        T = ROTL32(a,5) + f_t + e + SHA1_K[1] + W[t];
        e = d;
        d = c;
        c = ROTL32(b,30);
        b = a;
        a = T;
     } /* End of for */
    for (t = 40;t < 60;t++) {
        f_t = Maj(b,c,d);
        T = ROTL32(a,5) + f_t + e + SHA1_K[2] + W[t];
        e = d;
        d = c;
        c = ROTL32(b,30);
        b = a;
        a = T;
     } /* End of for */
    for (t = 60;t < 80;t++) {
        f_t = Parity(b,c,d);
        T = ROTL32(a,5) + f_t + e + SHA1_K[3] + W[t];
        e = d;
        d = c;
        c = ROTL32(b,30);
        b = a;
        a = T;
     } /* End of for */


     /* Compute the i^th intermediate hash value H^(i) */
     pSHA_CTX->HashValue[0] += a;
     pSHA_CTX->HashValue[1] += b;
     pSHA_CTX->HashValue[2] += c;
     pSHA_CTX->HashValue[3] += d;
     pSHA_CTX->HashValue[4] += e;

    NdisZeroMemory(pSHA_CTX->Block, SHA1_BLOCK_SIZE);
    pSHA_CTX->BlockLen = 0;
} /* End of RT_SHA1_Hash */


/*
========================================================================
Routine Description:
    The message is appended to block. If block size > 64 bytes, the SHA1_Hash 
will be called.

Arguments:
    pSHA_CTX        Pointer to SHA1_CTX_STRUC
    message         Message context
    messageLen      The length of message in bytes

Return Value:
    None

Note:
    None
========================================================================
*/
VOID RT_SHA1_Append (
    IN  SHA1_CTX_STRUC *pSHA_CTX, 
    IN  const UINT8 Message[], 
    IN  UINT MessageLen)
{
    UINT appendLen = 0;
    UINT diffLen   = 0;
    
    while (appendLen != MessageLen) {
        diffLen = MessageLen - appendLen;
        if ((pSHA_CTX->BlockLen + diffLen) <  SHA1_BLOCK_SIZE) {
            NdisMoveMemory(pSHA_CTX->Block + pSHA_CTX->BlockLen, 
                Message + appendLen, diffLen);
            pSHA_CTX->BlockLen += diffLen;
            appendLen += diffLen;
        } 
        else
        {
            NdisMoveMemory(pSHA_CTX->Block + pSHA_CTX->BlockLen, 
                Message + appendLen, SHA1_BLOCK_SIZE - pSHA_CTX->BlockLen);
            appendLen += (SHA1_BLOCK_SIZE - pSHA_CTX->BlockLen);
            pSHA_CTX->BlockLen = SHA1_BLOCK_SIZE;
            RT_SHA1_Hash(pSHA_CTX);
        } /* End of if */
    } /* End of while */
    pSHA_CTX->MessageLen += MessageLen;
} /* End of RT_SHA1_Append */


/*
========================================================================
Routine Description:
    1. Append bit 1 to end of the message
    2. Append the length of message in rightmost 64 bits
    3. Transform the Hash Value to digest message

Arguments:
    pSHA_CTX        Pointer to SHA1_CTX_STRUC

Return Value:
    digestMessage   Digest message

Note:
    None
========================================================================
*/
VOID RT_SHA1_End (
    IN  SHA1_CTX_STRUC *pSHA_CTX, 
    OUT UINT8 DigestMessage[])
{
    UINT index;
    UINT64 message_length_bits;

    /* Append bit 1 to end of the message */
    NdisFillMemory(pSHA_CTX->Block + pSHA_CTX->BlockLen, 1, 0x80);

    /* 55 = 64 - 8 - 1: append 1 bit(1 byte) and message length (8 bytes) */
    if (pSHA_CTX->BlockLen > 55)
        RT_SHA1_Hash(pSHA_CTX);
    /* End of if */

    /* Append the length of message in rightmost 64 bits */
    message_length_bits = pSHA_CTX->MessageLen*8;
    message_length_bits = cpu2be64(message_length_bits);       
    NdisMoveMemory(&pSHA_CTX->Block[56], &message_length_bits, 8);
    RT_SHA1_Hash(pSHA_CTX);

    /* Return message digest, transform the UINT32 hash value to bytes */
    for (index = 0; index < 5;index++)
        pSHA_CTX->HashValue[index] = cpu2be32(pSHA_CTX->HashValue[index]);
        /* End of for */
    NdisMoveMemory(DigestMessage, pSHA_CTX->HashValue, SHA1_DIGEST_SIZE);
} /* End of RT_SHA1_End */


/*
========================================================================
Routine Description:
    SHA1 algorithm

Arguments:
    message         Message context
    messageLen      The length of message in bytes

Return Value:
    digestMessage   Digest message

Note:
    None
========================================================================
*/
VOID RT_SHA1 (
    IN  const UINT8 Message[], 
    IN  UINT MessageLen, 
    OUT UINT8 DigestMessage[])
{

    SHA1_CTX_STRUC sha_ctx;

    NdisZeroMemory(&sha_ctx, sizeof(SHA1_CTX_STRUC));
    RT_SHA1_Init(&sha_ctx);
    RT_SHA1_Append(&sha_ctx, Message, MessageLen);
    RT_SHA1_End(&sha_ctx, DigestMessage);
} /* End of RT_SHA1 */
#endif /* SHA1_SUPPORT */


#ifdef SHA256_SUPPORT
/*
========================================================================
Routine Description:
    Initial SHA256_CTX_STRUC

Arguments:
    pSHA_CTX    Pointer to SHA256_CTX_STRUC

Return Value:
    None

Note:
    None
========================================================================
*/
VOID RT_SHA256_Init (
    IN  SHA256_CTX_STRUC *pSHA_CTX)
{
    NdisMoveMemory(pSHA_CTX->HashValue, SHA256_DefaultHashValue, 
        sizeof(SHA256_DefaultHashValue));
    NdisZeroMemory(pSHA_CTX->Block, SHA256_BLOCK_SIZE);
    pSHA_CTX->MessageLen = 0;
    pSHA_CTX->BlockLen   = 0;
} /* End of RT_SHA256_Init */


/*
========================================================================
Routine Description:
    SHA256 computation for one block (512 bits)

Arguments:
    pSHA_CTX    Pointer to SHA256_CTX_STRUC

Return Value:
    None

Note:
    None
========================================================================
*/
VOID RT_SHA256_Hash (
    IN  SHA256_CTX_STRUC *pSHA_CTX)
{
    UINT32 W_i,t;
    UINT32 W[64];
    UINT32 a,b,c,d,e,f,g,h,T1,T2;
    
    /* Prepare the message schedule, {W_i}, 0 < t < 15 */
    NdisMoveMemory(W, pSHA_CTX->Block, SHA256_BLOCK_SIZE);
    for (W_i = 0; W_i < 16; W_i++)
        W[W_i] = cpu2be32(W[W_i]); /* Endian Swap */
        /* End of for */
 
    /* SHA256 hash computation */
    /* Initialize the working variables */
    a = pSHA_CTX->HashValue[0];
    b = pSHA_CTX->HashValue[1];
    c = pSHA_CTX->HashValue[2];
    d = pSHA_CTX->HashValue[3];
    e = pSHA_CTX->HashValue[4];
    f = pSHA_CTX->HashValue[5];
    g = pSHA_CTX->HashValue[6];
    h = pSHA_CTX->HashValue[7];

    /* 64 rounds */
    for (t = 0;t < 64;t++) {
        if (t > 15) /* Prepare the message schedule, {W_i}, 16 < t < 63 */
            W[t] = Sigma_256_1(W[t-2]) + W[t-7] + Sigma_256_0(W[t-15]) + W[t-16];
            /* End of if */
        T1 = h + Zsigma_256_1(e) + Ch(e,f,g) + SHA256_K[t] + W[t];
        T2 = Zsigma_256_0(a) + Maj(a,b,c);
        h = g;
        g = f;
        f = e;
        e = d + T1;
        d = c;
        c = b;
        b = a;
        a = T1 + T2;
     } /* End of for */

     /* Compute the i^th intermediate hash value H^(i) */
     pSHA_CTX->HashValue[0] += a;
     pSHA_CTX->HashValue[1] += b;
     pSHA_CTX->HashValue[2] += c;
     pSHA_CTX->HashValue[3] += d;
     pSHA_CTX->HashValue[4] += e;
     pSHA_CTX->HashValue[5] += f;
     pSHA_CTX->HashValue[6] += g;
     pSHA_CTX->HashValue[7] += h;

    NdisZeroMemory(pSHA_CTX->Block, SHA256_BLOCK_SIZE);
    pSHA_CTX->BlockLen = 0;
} /* End of RT_SHA256_Hash */


/*
========================================================================
Routine Description:
    The message is appended to block. If block size > 64 bytes, the SHA256_Hash 
will be called.

Arguments:
    pSHA_CTX    Pointer to SHA256_CTX_STRUC
    message     Message context
    messageLen  The length of message in bytes

Return Value:
    None

Note:
    None
========================================================================
*/
VOID RT_SHA256_Append (
    IN  SHA256_CTX_STRUC *pSHA_CTX, 
    IN  const UINT8 Message[], 
    IN  UINT MessageLen)
{
    UINT appendLen = 0;
    UINT diffLen   = 0;
    
    while (appendLen != MessageLen) {
        diffLen = MessageLen - appendLen;
        if ((pSHA_CTX->BlockLen + diffLen) <  SHA256_BLOCK_SIZE) {
            NdisMoveMemory(pSHA_CTX->Block + pSHA_CTX->BlockLen, 
                Message + appendLen, diffLen);
            pSHA_CTX->BlockLen += diffLen;
            appendLen += diffLen;
        } 
        else
        {
            NdisMoveMemory(pSHA_CTX->Block + pSHA_CTX->BlockLen, 
                Message + appendLen, SHA256_BLOCK_SIZE - pSHA_CTX->BlockLen);
            appendLen += (SHA256_BLOCK_SIZE - pSHA_CTX->BlockLen);
            pSHA_CTX->BlockLen = SHA256_BLOCK_SIZE;
            RT_SHA256_Hash(pSHA_CTX);
        } /* End of if */
    } /* End of while */
    pSHA_CTX->MessageLen += MessageLen;
} /* End of RT_SHA256_Append */


/*
========================================================================
Routine Description:
    1. Append bit 1 to end of the message
    2. Append the length of message in rightmost 64 bits
    3. Transform the Hash Value to digest message

Arguments:
    pSHA_CTX        Pointer to SHA256_CTX_STRUC

Return Value:
    digestMessage   Digest message

Note:
    None
========================================================================
*/
VOID RT_SHA256_End (
    IN  SHA256_CTX_STRUC *pSHA_CTX, 
    OUT UINT8 DigestMessage[])
{
    UINT index;
    UINT64 message_length_bits;

    /* Append bit 1 to end of the message */
    NdisFillMemory(pSHA_CTX->Block + pSHA_CTX->BlockLen, 1, 0x80);

    /* 55 = 64 - 8 - 1: append 1 bit(1 byte) and message length (8 bytes) */
    if (pSHA_CTX->BlockLen > 55)
        RT_SHA256_Hash(pSHA_CTX);
    /* End of if */

    /* Append the length of message in rightmost 64 bits */
    message_length_bits = pSHA_CTX->MessageLen*8;
    message_length_bits = cpu2be64(message_length_bits);       
    NdisMoveMemory(&pSHA_CTX->Block[56], &message_length_bits, 8);
    RT_SHA256_Hash(pSHA_CTX);

    /* Return message digest, transform the UINT32 hash value to bytes */
    for (index = 0; index < 8;index++)
        pSHA_CTX->HashValue[index] = cpu2be32(pSHA_CTX->HashValue[index]);
        /* End of for */
    NdisMoveMemory(DigestMessage, pSHA_CTX->HashValue, SHA256_DIGEST_SIZE);
} /* End of RT_SHA256_End */


/*
========================================================================
Routine Description:
    SHA256 algorithm

Arguments:
    message         Message context
    messageLen      The length of message in bytes

Return Value:
    digestMessage   Digest message

Note:
    None
========================================================================
*/
VOID RT_SHA256 (
    IN  const UINT8 Message[], 
    IN  UINT MessageLen, 
    OUT UINT8 DigestMessage[])
{
    SHA256_CTX_STRUC sha_ctx;

    NdisZeroMemory(&sha_ctx, sizeof(SHA256_CTX_STRUC));
    RT_SHA256_Init(&sha_ctx);
    RT_SHA256_Append(&sha_ctx, Message, MessageLen);
    RT_SHA256_End(&sha_ctx, DigestMessage);
} /* End of RT_SHA256 */
#if defined(DOT11_SAE_SUPPORT) || defined(CONFIG_OWE_SUPPORT)
VOID rt_sha256_vector(
	IN UCHAR num,
	IN const unsigned char **message,
	IN UINT *messageLen,
	OUT UINT8 *digestmessage)
{
	SHA256_CTX_STRUC sha_ctx;
	UCHAR i;

	NdisZeroMemory(&sha_ctx, sizeof(SHA256_CTX_STRUC));
	RT_SHA256_Init(&sha_ctx);
	for (i = 0; i < num; i++)
		RT_SHA256_Append(&sha_ctx, message[i], messageLen[i]);
	RT_SHA256_End(&sha_ctx, digestmessage);
}
#endif /* defined(DOT11_SAE_SUPPORT) || defined(CONFIG_OWE_SUPPORT) */
#endif /* SHA256_SUPPORT */

#if defined(DOT11_SAE_SUPPORT) || defined(CONFIG_OWE_SUPPORT)
#ifdef SHA384_SUPPORT
/*
========================================================================
Routine Description:
    Initial SHA384_CTX_STRUC

Arguments:
    pSHA_CTX    Pointer to SHA384_CTX_STRUC

Return Value:
    None

Note:
    None
========================================================================
*/
VOID RT_SHA384_Init(
	IN  SHA384_CTX_STRUC * pSHA_CTX)
{
	NdisMoveMemory(pSHA_CTX->HashValue, SHA384_DefaultHashValue, sizeof(SHA384_DefaultHashValue));
	NdisZeroMemory(pSHA_CTX->Block, SHA384_BLOCK_SIZE);
	pSHA_CTX->MessageLen = 0;
	pSHA_CTX->BlockLen = 0;
} /* End of RT_SHA384_Init */


/*
========================================================================
Routine Description:
    SHA384 computation for one block (1024 bits)

Arguments:
    pSHA_CTX    Pointer to SHA384_CTX_STRUC

Return Value:
    None

Note:
    None
========================================================================
*/
VOID RT_SHA384_Hash(
	IN  SHA384_CTX_STRUC * pSHA_CTX)
{
	UINT32 W_i, t;
	UINT64 W[80];
	UINT64 a, b, c, d, e, f, g, h, T1, T2;
	/* Prepare the message schedule, {W_i}, 0 < t < 15 */
	NdisMoveMemory(W, pSHA_CTX->Block, SHA384_BLOCK_SIZE);

	for (W_i = 0; W_i < 16; W_i++)
		W[W_i] = cpu2be64(W[W_i]); /* Endian Swap */

	/* End of for */
	/* SHA384 hash computation */
	/* Initialize the working variables */
	a = pSHA_CTX->HashValue[0];
	b = pSHA_CTX->HashValue[1];
	c = pSHA_CTX->HashValue[2];
	d = pSHA_CTX->HashValue[3];
	e = pSHA_CTX->HashValue[4];
	f = pSHA_CTX->HashValue[5];
	g = pSHA_CTX->HashValue[6];
	h = pSHA_CTX->HashValue[7];

	/* 64 rounds */
	for (t = 0; t < 80; t++) {
		if (t > 15) /* Prepare the message schedule, {W_i}, 16 < t < 79 */
			W[t] = Sigma_512_1(W[t-2]) + W[t-7] + Sigma_512_0(W[t-15]) + W[t-16];

		T1 = h + Zsigma_512_1(e) + Ch(e, f, g) + SHA384_K[t] + W[t];
		T2 = Zsigma_512_0(a) + Maj(a, b, c);
		h = g;
		g = f;
		f = e;
		e = d + T1;
		d = c;
		c = b;
		b = a;
		a = T1 + T2;
	} /* End of for */

	/* Compute the i^th intermediate hash value H^(i) */
	pSHA_CTX->HashValue[0] += a;
	pSHA_CTX->HashValue[1] += b;
	pSHA_CTX->HashValue[2] += c;
	pSHA_CTX->HashValue[3] += d;
	pSHA_CTX->HashValue[4] += e;
	pSHA_CTX->HashValue[5] += f;
	pSHA_CTX->HashValue[6] += g;
	pSHA_CTX->HashValue[7] += h;
	NdisZeroMemory(pSHA_CTX->Block, SHA384_BLOCK_SIZE);
	pSHA_CTX->BlockLen = 0;
} /* End of RT_SHA384_Hash */


/*
========================================================================
Routine Description:
    The message is appended to block. If block size > 128 bytes, the SHA384_Hash
will be called.

Arguments:
    pSHA_CTX    Pointer to SHA384_CTX_STRUC
    message     Message context
    messageLen  The length of message in bytes

Return Value:
    None

Note:
    None
========================================================================
*/
VOID RT_SHA384_Append(
	IN  SHA384_CTX_STRUC * pSHA_CTX,
	IN  const UINT8 Message[],
	IN  UINT MessageLen)
{
	UINT appendLen = 0;
	UINT diffLen   = 0;

	while (appendLen != MessageLen) {
		diffLen = MessageLen - appendLen;

		if ((pSHA_CTX->BlockLen + diffLen) <  SHA384_BLOCK_SIZE) {
			NdisMoveMemory(pSHA_CTX->Block + pSHA_CTX->BlockLen,
						   Message + appendLen, diffLen);
			pSHA_CTX->BlockLen += diffLen;
			appendLen += diffLen;
		} else {
			NdisMoveMemory(pSHA_CTX->Block + pSHA_CTX->BlockLen,
						   Message + appendLen, SHA384_BLOCK_SIZE - pSHA_CTX->BlockLen);
			appendLen += (SHA384_BLOCK_SIZE - pSHA_CTX->BlockLen);
			pSHA_CTX->BlockLen = SHA384_BLOCK_SIZE;
			RT_SHA384_Hash(pSHA_CTX);
		} /* End of if */
	} /* End of while */

	pSHA_CTX->MessageLen += MessageLen;
} /* End of RT_SHA384_Append */


/*
========================================================================
Routine Description:
    1. Append bit 1 to end of the message
    2. Append the length of message in rightmost 128 bits
    3. Transform the Hash Value to digest message

Arguments:
    pSHA_CTX        Pointer to SHA384_CTX_STRUC

Return Value:
    digestMessage   Digest message

Note:
    None
========================================================================
*/
VOID RT_SHA384_End(
	IN  SHA384_CTX_STRUC * pSHA_CTX,
	OUT UINT8 DigestMessage[])
{
	UINT index;
	UINT64 message_length_bits;
	/* Append bit 1 to end of the message */
	NdisFillMemory(pSHA_CTX->Block + pSHA_CTX->BlockLen, 1, 0x80);

	/* 119 = 128(SHA256_BLOCK_SIZE) - 16 - 1: append 1 bit(1 byte) and message length (16 bytes) */
	if (pSHA_CTX->BlockLen > 119)
		RT_SHA384_Hash(pSHA_CTX);

	/* End of if */
	/* Append the length of message in rightmost 128 bits */
	message_length_bits = pSHA_CTX->MessageLen*8;
	message_length_bits = cpu2be64(message_length_bits);
	NdisMoveMemory(&pSHA_CTX->Block[120], &message_length_bits, 8);
	RT_SHA384_Hash(pSHA_CTX);

	/* Return message digest, transform the UINT64 hash value to bytes */
	for (index = 0; index < 8; index++)
		pSHA_CTX->HashValue[index] = cpu2be64(pSHA_CTX->HashValue[index]);

	/* End of for */
	NdisMoveMemory(DigestMessage, pSHA_CTX->HashValue, SHA384_DIGEST_SIZE);
} /* End of RT_SHA384_End */


/*
========================================================================
Routine Description:
    SHA384 algorithm

Arguments:
    message         Message context
    messageLen      The length of message in bytes

Return Value:
    digestMessage   Digest message

Note:
    None
========================================================================
*/
VOID RT_SHA384(
	IN  const UINT8 Message[],
	IN  UINT MessageLen,
	OUT UINT8 DigestMessage[])
{
	SHA384_CTX_STRUC sha_ctx;

	NdisZeroMemory(&sha_ctx, sizeof(SHA384_CTX_STRUC));
	RT_SHA384_Init(&sha_ctx);
	RT_SHA384_Append(&sha_ctx, Message, MessageLen);
	RT_SHA384_End(&sha_ctx, DigestMessage);
} /* End of RT_SHA384 */

VOID rt_sha384_vector(
	IN UCHAR num,
	IN const unsigned char **message,
	IN UINT *messageLen,
	OUT UINT8 *digestmessage)
{
	SHA384_CTX_STRUC sha_ctx;
	UCHAR i;

	NdisZeroMemory(&sha_ctx, sizeof(SHA384_CTX_STRUC));
	RT_SHA384_Init(&sha_ctx);
	for (i = 0; i < num; i++)
		RT_SHA384_Append(&sha_ctx, message[i], messageLen[i]);
	RT_SHA384_End(&sha_ctx, digestmessage);
}

#endif /* SHA384_SUPPORT */
#endif /* defined(DOT11_SAE_SUPPORT) || defined(CONFIG_OWE_SUPPORT) */

/* End of crypt_sha2.c */


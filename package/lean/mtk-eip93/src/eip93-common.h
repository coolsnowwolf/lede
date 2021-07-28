/* SPDX-License-Identifier: GPL-2.0
 *
 * Copyright (C) 2019 - 2020
 *
 * Richard van Schagen <vschagen@cs.com>
 */

#ifndef _COMMON_H_
#define _COMMON_H_

#include <linux/bits.h>

#define MTK_RING_SIZE			256
#define MTK_RING_BUSY			224
#define NUM_AES_BYPASS			256
#define MTK_QUEUE_LENGTH		128
#define MTK_CRA_PRIORITY		1500

#define MTK_DESC_ASYNC			BIT(0)
#define MTK_DESC_SKCIPHER		BIT(1)
#define MTK_DESC_AEAD			BIT(2)
#define MTK_DESC_AHASH			BIT(3)
#define MTK_DESC_PRNG			BIT(4)
#define MTK_DESC_FAKE_HMAC		BIT(5)
#define MTK_DESC_LAST			BIT(6)
#define MTK_DESC_FINISH			BIT(7)

/* cipher algorithms */
#define MTK_ALG_DES			BIT(0)
#define MTK_ALG_3DES			BIT(1)
#define MTK_ALG_AES			BIT(2)
#define MTK_ALG_MASK			GENMASK(2, 0)
/* hash and hmac algorithms */
#define MTK_HASH_MD5			BIT(3)
#define MTK_HASH_SHA1			BIT(4)
#define MTK_HASH_SHA224			BIT(5)
#define MTK_HASH_SHA256			BIT(6)
#define MTK_HASH_HMAC			BIT(7)
#define MTK_HASH_MASK			GENMASK(6, 3)
/* cipher modes */
#define MTK_MODE_CBC			BIT(8)
#define MTK_MODE_ECB			BIT(9)
#define MTK_MODE_CTR			BIT(10)
#define MTK_MODE_RFC3686		BIT(11)
#define MTK_MODE_MASK			GENMASK(10, 8)

/* cipher encryption/decryption operations */
#define MTK_ENCRYPT			BIT(12)
#define MTK_DECRYPT			BIT(13)

#define MTK_GENIV			BIT(14)
#define MTK_BUSY			BIT(15)

#define IS_DES(flags)			(flags & MTK_ALG_DES)
#define IS_3DES(flags)			(flags & MTK_ALG_3DES)
#define IS_AES(flags)			(flags & MTK_ALG_AES)

#define IS_HASH_MD5(flags)		(flags & MTK_HASH_MD5)
#define IS_HASH_SHA1(flags)		(flags & MTK_HASH_SHA1)
#define IS_HASH_SHA224(flags)		(flags & MTK_HASH_SHA224)
#define IS_HASH_SHA256(flags)		(flags & MTK_HASH_SHA256)
#define IS_HMAC(flags)			(flags & MTK_HASH_HMAC)

#define IS_CBC(mode)			(mode & MTK_MODE_CBC)
#define IS_ECB(mode)			(mode & MTK_MODE_ECB)
#define IS_CTR(mode)			(mode & MTK_MODE_CTR)
#define IS_RFC3686(mode)		(mode & MTK_MODE_RFC3686)
#define IS_GENIV(flags)			(flags & MTK_GENIV)

#define IS_BUSY(flags)			(flags & MTK_BUSY)

#define IS_ENCRYPT(dir)			(dir & MTK_ENCRYPT)
#define IS_DECRYPT(dir)			(dir & MTK_DECRYPT)

#define IS_CIPHER(flags)		(flags & (MTK_ALG_DES || \
						MTK_ALG_3DES ||  \
						MTK_ALG_AES))

#define IS_HASH(flags)			(flags & (MTK_HASH_MD5 ||  \
						MTK_HASH_SHA1 ||   \
						MTK_HASH_SHA224 || \
						MTK_HASH_SHA256))

/*
 * Interrupts of EIP93
 */

typedef enum {
	EIP93_INT_PE_CDRTHRESH_REQ =	BIT(0),
	EIP93_INT_PE_RDRTHRESH_REQ =	BIT(1),
	EIP93_INT_PE_OPERATION_DONE =	BIT(9),
	EIP93_INT_PE_INBUFTHRESH_REQ =	BIT(10),
	EIP93_INT_PE_OUTBURTHRSH_REQ =	BIT(11),
	EIP93_INT_PE_PRNG_IRQ =		BIT(12),
	EIP93_INT_PE_ERR_REG =		BIT(13),
	EIP93_INT_PE_RD_DONE_IRQ =	BIT(16),
} EIP93_InterruptSource_t;

typedef union {
	struct {
		unsigned int opCode		:3;
		unsigned int direction		:1;
		unsigned int opGroup		:2;
		unsigned int padType		:2;
		unsigned int cipher		:4;
		unsigned int hash		:4;
		unsigned int reserved2		:1;
		unsigned int scPad		:1;
		unsigned int extPad		:1;
		unsigned int hdrProc		:1;
		unsigned int digestLength	:4;
		unsigned int ivSource		:2;
		unsigned int hashSource		:2;
		unsigned int saveIv		:1;
		unsigned int saveHash		:1;
		unsigned int reserved1		:2;
	} bits;
	unsigned int word;

} saCmd0_t;

typedef union {
	struct {
		unsigned int copyDigest		:1;
		unsigned int copyHeader		:1;
		unsigned int copyPayload	:1;
		unsigned int copyPad		:1;
		unsigned int reserved4		:4;
		unsigned int cipherMode		:2;
		unsigned int reserved3		:1;
		unsigned int sslMac		:1;
		unsigned int hmac		:1;
		unsigned int byteOffset		:1;
		unsigned int reserved2		:2;
		unsigned int hashCryptOffset	:8;
		unsigned int aesKeyLen		:3;
		unsigned int reserved1		:1;
		unsigned int aesDecKey		:1;
		unsigned int seqNumCheck	:1;
		unsigned int reserved0		:2;
	} bits;
	unsigned int word;

} saCmd1_t;

typedef struct saRecord_s {
	saCmd0_t	saCmd0;
	saCmd1_t	saCmd1;
	unsigned int	saKey[8];
	unsigned int	saIDigest[8];
	unsigned int	saODigest[8];
	unsigned int	saSpi;
	unsigned int	saSeqNum[2];
	unsigned int	saSeqNumMask[2];
	unsigned int	saNonce;
} saRecord_t;

typedef struct saState_s {
	unsigned int	stateIv[4];
	unsigned int	stateByteCnt[2];
	unsigned int	stateIDigest[8];
} saState_t;

typedef union {
	struct {
		unsigned int hostReady		:1;
		unsigned int peReady		:1;
		unsigned int reserved		:1;
		unsigned int initArc4		:1;
		unsigned int hashFinal		:1;
		unsigned int haltMode		:1;
		unsigned int prngMode		:2;
		unsigned int padValue		:8;
		unsigned int errStatus		:8;
		unsigned int padCrtlStat	:8;
	} bits;
	unsigned int word;
} peCrtlStat_t;

typedef union {
	struct {
		unsigned int length		:20;
		unsigned int reserved		:2;
		unsigned int hostReady		:1;
		unsigned int peReady		:1;
		unsigned int byPass		:8;
	} bits;
	unsigned int word;
} peLength_t;

typedef struct eip93_descriptor_s {
	peCrtlStat_t		peCrtlStat;
	unsigned int		srcAddr;
	unsigned int		dstAddr;
	unsigned int		saAddr;
	unsigned int		stateAddr;
	unsigned int		arc4Addr;
	unsigned int		userId;
	peLength_t		peLength;
} eip93_descriptor_t;

#endif /* _COMMON_H_ */

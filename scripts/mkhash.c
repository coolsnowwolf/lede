/*
 * Copyright (C) 2016 Felix Fietkau <nbd@nbd.name>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * -- MD5 code:
 *
 * This is an OpenSSL-compatible implementation of the RSA Data Security, Inc.
 * MD5 Message-Digest Algorithm (RFC 1321).
 *
 * Homepage:
 * http://openwall.info/wiki/people/solar/software/public-domain-source-code/md5
 *
 * Author:
 * Alexander Peslyak, better known as Solar Designer <solar at openwall.com>
 *
 * This software was written by Alexander Peslyak in 2001.  No copyright is
 * claimed, and the software is hereby placed in the public domain.
 * In case this attempt to disclaim copyright and place the software in the
 * public domain is deemed null and void, then the software is
 * Copyright (c) 2001 Alexander Peslyak and it is hereby released to the
 * general public under the following terms:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted.
 *
 * There's ABSOLUTELY NO WARRANTY, express or implied.
 *
 * (This is a heavily cut-down "BSD license".)
 *
 * This differs from Colin Plumb's older public domain implementation in that
 * no exactly 32-bit integer data type is required (any 32-bit or wider
 * unsigned integer data type will do), there's no compile-time endianness
 * configuration, and the function prototypes match OpenSSL's.  No code from
 * Colin Plumb's implementation has been reused; this comment merely compares
 * the properties of the two independent implementations.
 *
 * The primary goals of this implementation are portability and ease of use.
 * It is meant to be fast, but not as fast as possible.  Some known
 * optimizations are not included to reduce source code size and avoid
 * compile-time configuration.
 *
 * -- SHA256 Code:
 *
 * Copyright 2005 Colin Percival
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */



#ifndef __FreeBSD__
#include <endian.h>
#else
#include <sys/endian.h>
#endif

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/stat.h>

#define ARRAY_SIZE(_n) (sizeof(_n) / sizeof((_n)[0]))

#ifndef __FreeBSD__
static void
be32enc(void *buf, uint32_t u)
{
	uint8_t *p = buf;

	p[0] = ((uint8_t) ((u >> 24) & 0xff));
	p[1] = ((uint8_t) ((u >> 16) & 0xff));
	p[2] = ((uint8_t) ((u >> 8) & 0xff));
	p[3] = ((uint8_t) (u & 0xff));
}

static void
be64enc(void *buf, uint64_t u)
{
	uint8_t *p = buf;

	be32enc(p, ((uint32_t) (u >> 32)));
	be32enc(p + 4, ((uint32_t) (u & 0xffffffffULL)));
}


static uint16_t
be16dec(const void *buf)
{
	const uint8_t *p = buf;

	return (((uint16_t) p[0]) << 8) | p[1];
}

static uint32_t
be32dec(const void *buf)
{
	const uint8_t *p = buf;

	return (((uint32_t) be16dec(p)) << 16) | be16dec(p + 2);
}
#endif

#define MD5_DIGEST_LENGTH	16

typedef struct MD5_CTX {
	uint32_t lo, hi;
	uint32_t a, b, c, d;
	unsigned char buffer[64];
} MD5_CTX;

/*
 * The basic MD5 functions.
 *
 * F and G are optimized compared to their RFC 1321 definitions for
 * architectures that lack an AND-NOT instruction, just like in Colin Plumb's
 * implementation.
 */
#define F(x, y, z)			((z) ^ ((x) & ((y) ^ (z))))
#define G(x, y, z)			((y) ^ ((z) & ((x) ^ (y))))
#define H(x, y, z)			(((x) ^ (y)) ^ (z))
#define H2(x, y, z)			((x) ^ ((y) ^ (z)))
#define I(x, y, z)			((y) ^ ((x) | ~(z)))

/*
 * The MD5 transformation for all four rounds.
 */
#define STEP(f, a, b, c, d, x, t, s) \
	(a) += f((b), (c), (d)) + (x) + (t); \
	(a) = (((a) << (s)) | (((a) & 0xffffffff) >> (32 - (s)))); \
	(a) += (b);

/*
 * SET reads 4 input bytes in little-endian byte order and stores them
 * in a properly aligned word in host byte order.
 */
#if __BYTE_ORDER == __LITTLE_ENDIAN
#define SET(n) \
	(*(uint32_t *)&ptr[(n) * 4])
#define GET(n) \
	SET(n)
#else
#define SET(n) \
	(block[(n)] = \
	(uint32_t)ptr[(n) * 4] | \
	((uint32_t)ptr[(n) * 4 + 1] << 8) | \
	((uint32_t)ptr[(n) * 4 + 2] << 16) | \
	((uint32_t)ptr[(n) * 4 + 3] << 24))
#define GET(n) \
	(block[(n)])
#endif

/*
 * This processes one or more 64-byte data blocks, but does NOT update
 * the bit counters.  There are no alignment requirements.
 */
static const void *MD5_body(MD5_CTX *ctx, const void *data, unsigned long size)
{
	const unsigned char *ptr;
	uint32_t a, b, c, d;
	uint32_t saved_a, saved_b, saved_c, saved_d;
#if __BYTE_ORDER != __LITTLE_ENDIAN
	uint32_t block[16];
#endif

	ptr = (const unsigned char *)data;

	a = ctx->a;
	b = ctx->b;
	c = ctx->c;
	d = ctx->d;

	do {
		saved_a = a;
		saved_b = b;
		saved_c = c;
		saved_d = d;

/* Round 1 */
		STEP(F, a, b, c, d, SET(0), 0xd76aa478, 7)
		STEP(F, d, a, b, c, SET(1), 0xe8c7b756, 12)
		STEP(F, c, d, a, b, SET(2), 0x242070db, 17)
		STEP(F, b, c, d, a, SET(3), 0xc1bdceee, 22)
		STEP(F, a, b, c, d, SET(4), 0xf57c0faf, 7)
		STEP(F, d, a, b, c, SET(5), 0x4787c62a, 12)
		STEP(F, c, d, a, b, SET(6), 0xa8304613, 17)
		STEP(F, b, c, d, a, SET(7), 0xfd469501, 22)
		STEP(F, a, b, c, d, SET(8), 0x698098d8, 7)
		STEP(F, d, a, b, c, SET(9), 0x8b44f7af, 12)
		STEP(F, c, d, a, b, SET(10), 0xffff5bb1, 17)
		STEP(F, b, c, d, a, SET(11), 0x895cd7be, 22)
		STEP(F, a, b, c, d, SET(12), 0x6b901122, 7)
		STEP(F, d, a, b, c, SET(13), 0xfd987193, 12)
		STEP(F, c, d, a, b, SET(14), 0xa679438e, 17)
		STEP(F, b, c, d, a, SET(15), 0x49b40821, 22)

/* Round 2 */
		STEP(G, a, b, c, d, GET(1), 0xf61e2562, 5)
		STEP(G, d, a, b, c, GET(6), 0xc040b340, 9)
		STEP(G, c, d, a, b, GET(11), 0x265e5a51, 14)
		STEP(G, b, c, d, a, GET(0), 0xe9b6c7aa, 20)
		STEP(G, a, b, c, d, GET(5), 0xd62f105d, 5)
		STEP(G, d, a, b, c, GET(10), 0x02441453, 9)
		STEP(G, c, d, a, b, GET(15), 0xd8a1e681, 14)
		STEP(G, b, c, d, a, GET(4), 0xe7d3fbc8, 20)
		STEP(G, a, b, c, d, GET(9), 0x21e1cde6, 5)
		STEP(G, d, a, b, c, GET(14), 0xc33707d6, 9)
		STEP(G, c, d, a, b, GET(3), 0xf4d50d87, 14)
		STEP(G, b, c, d, a, GET(8), 0x455a14ed, 20)
		STEP(G, a, b, c, d, GET(13), 0xa9e3e905, 5)
		STEP(G, d, a, b, c, GET(2), 0xfcefa3f8, 9)
		STEP(G, c, d, a, b, GET(7), 0x676f02d9, 14)
		STEP(G, b, c, d, a, GET(12), 0x8d2a4c8a, 20)

/* Round 3 */
		STEP(H, a, b, c, d, GET(5), 0xfffa3942, 4)
		STEP(H2, d, a, b, c, GET(8), 0x8771f681, 11)
		STEP(H, c, d, a, b, GET(11), 0x6d9d6122, 16)
		STEP(H2, b, c, d, a, GET(14), 0xfde5380c, 23)
		STEP(H, a, b, c, d, GET(1), 0xa4beea44, 4)
		STEP(H2, d, a, b, c, GET(4), 0x4bdecfa9, 11)
		STEP(H, c, d, a, b, GET(7), 0xf6bb4b60, 16)
		STEP(H2, b, c, d, a, GET(10), 0xbebfbc70, 23)
		STEP(H, a, b, c, d, GET(13), 0x289b7ec6, 4)
		STEP(H2, d, a, b, c, GET(0), 0xeaa127fa, 11)
		STEP(H, c, d, a, b, GET(3), 0xd4ef3085, 16)
		STEP(H2, b, c, d, a, GET(6), 0x04881d05, 23)
		STEP(H, a, b, c, d, GET(9), 0xd9d4d039, 4)
		STEP(H2, d, a, b, c, GET(12), 0xe6db99e5, 11)
		STEP(H, c, d, a, b, GET(15), 0x1fa27cf8, 16)
		STEP(H2, b, c, d, a, GET(2), 0xc4ac5665, 23)

/* Round 4 */
		STEP(I, a, b, c, d, GET(0), 0xf4292244, 6)
		STEP(I, d, a, b, c, GET(7), 0x432aff97, 10)
		STEP(I, c, d, a, b, GET(14), 0xab9423a7, 15)
		STEP(I, b, c, d, a, GET(5), 0xfc93a039, 21)
		STEP(I, a, b, c, d, GET(12), 0x655b59c3, 6)
		STEP(I, d, a, b, c, GET(3), 0x8f0ccc92, 10)
		STEP(I, c, d, a, b, GET(10), 0xffeff47d, 15)
		STEP(I, b, c, d, a, GET(1), 0x85845dd1, 21)
		STEP(I, a, b, c, d, GET(8), 0x6fa87e4f, 6)
		STEP(I, d, a, b, c, GET(15), 0xfe2ce6e0, 10)
		STEP(I, c, d, a, b, GET(6), 0xa3014314, 15)
		STEP(I, b, c, d, a, GET(13), 0x4e0811a1, 21)
		STEP(I, a, b, c, d, GET(4), 0xf7537e82, 6)
		STEP(I, d, a, b, c, GET(11), 0xbd3af235, 10)
		STEP(I, c, d, a, b, GET(2), 0x2ad7d2bb, 15)
		STEP(I, b, c, d, a, GET(9), 0xeb86d391, 21)

		a += saved_a;
		b += saved_b;
		c += saved_c;
		d += saved_d;

		ptr += 64;
	} while (size -= 64);

	ctx->a = a;
	ctx->b = b;
	ctx->c = c;
	ctx->d = d;

	return ptr;
}

void MD5_begin(MD5_CTX *ctx)
{
	ctx->a = 0x67452301;
	ctx->b = 0xefcdab89;
	ctx->c = 0x98badcfe;
	ctx->d = 0x10325476;

	ctx->lo = 0;
	ctx->hi = 0;
}

static void
MD5_hash(const void *data, size_t size, MD5_CTX *ctx)
{
	uint32_t saved_lo;
	unsigned long used, available;

	saved_lo = ctx->lo;
	if ((ctx->lo = (saved_lo + size) & 0x1fffffff) < saved_lo)
		ctx->hi++;
	ctx->hi += size >> 29;

	used = saved_lo & 0x3f;

	if (used) {
		available = 64 - used;

		if (size < available) {
			memcpy(&ctx->buffer[used], data, size);
			return;
		}

		memcpy(&ctx->buffer[used], data, available);
		data = (const unsigned char *)data + available;
		size -= available;
		MD5_body(ctx, ctx->buffer, 64);
	}

	if (size >= 64) {
		data = MD5_body(ctx, data, size & ~((size_t) 0x3f));
		size &= 0x3f;
	}

	memcpy(ctx->buffer, data, size);
}

static void
MD5_end(void *resbuf, MD5_CTX *ctx)
{
	unsigned char *result = resbuf;
	unsigned long used, available;

	used = ctx->lo & 0x3f;

	ctx->buffer[used++] = 0x80;

	available = 64 - used;

	if (available < 8) {
		memset(&ctx->buffer[used], 0, available);
		MD5_body(ctx, ctx->buffer, 64);
		used = 0;
		available = 64;
	}

	memset(&ctx->buffer[used], 0, available - 8);

	ctx->lo <<= 3;
	ctx->buffer[56] = ctx->lo;
	ctx->buffer[57] = ctx->lo >> 8;
	ctx->buffer[58] = ctx->lo >> 16;
	ctx->buffer[59] = ctx->lo >> 24;
	ctx->buffer[60] = ctx->hi;
	ctx->buffer[61] = ctx->hi >> 8;
	ctx->buffer[62] = ctx->hi >> 16;
	ctx->buffer[63] = ctx->hi >> 24;

	MD5_body(ctx, ctx->buffer, 64);

	result[0] = ctx->a;
	result[1] = ctx->a >> 8;
	result[2] = ctx->a >> 16;
	result[3] = ctx->a >> 24;
	result[4] = ctx->b;
	result[5] = ctx->b >> 8;
	result[6] = ctx->b >> 16;
	result[7] = ctx->b >> 24;
	result[8] = ctx->c;
	result[9] = ctx->c >> 8;
	result[10] = ctx->c >> 16;
	result[11] = ctx->c >> 24;
	result[12] = ctx->d;
	result[13] = ctx->d >> 8;
	result[14] = ctx->d >> 16;
	result[15] = ctx->d >> 24;

	memset(ctx, 0, sizeof(*ctx));
}

#define SHA256_BLOCK_LENGTH		64
#define SHA256_DIGEST_LENGTH		32
#define SHA256_DIGEST_STRING_LENGTH	(SHA256_DIGEST_LENGTH * 2 + 1)

typedef struct SHA256Context {
	uint32_t state[8];
	uint64_t count;
	uint8_t buf[SHA256_BLOCK_LENGTH];
} SHA256_CTX;

#if BYTE_ORDER == BIG_ENDIAN

/* Copy a vector of big-endian uint32_t into a vector of bytes */
#define be32enc_vect(dst, src, len)	\
	memcpy((void *)dst, (const void *)src, (size_t)len)

/* Copy a vector of bytes into a vector of big-endian uint32_t */
#define be32dec_vect(dst, src, len)	\
	memcpy((void *)dst, (const void *)src, (size_t)len)

#else /* BYTE_ORDER != BIG_ENDIAN */

/*
 * Encode a length len/4 vector of (uint32_t) into a length len vector of
 * (unsigned char) in big-endian form.  Assumes len is a multiple of 4.
 */
static void
be32enc_vect(unsigned char *dst, const uint32_t *src, size_t len)
{
	size_t i;

	for (i = 0; i < len / 4; i++)
		be32enc(dst + i * 4, src[i]);
}

/*
 * Decode a big-endian length len vector of (unsigned char) into a length
 * len/4 vector of (uint32_t).  Assumes len is a multiple of 4.
 */
static void
be32dec_vect(uint32_t *dst, const unsigned char *src, size_t len)
{
	size_t i;

	for (i = 0; i < len / 4; i++)
		dst[i] = be32dec(src + i * 4);
}

#endif /* BYTE_ORDER != BIG_ENDIAN */


/* Elementary functions used by SHA256 */
#define Ch(x, y, z)	((x & (y ^ z)) ^ z)
#define Maj(x, y, z)	((x & (y | z)) | (y & z))
#define ROTR(x, n)	((x >> n) | (x << (32 - n)))

/*
 * SHA256 block compression function.  The 256-bit state is transformed via
 * the 512-bit input block to produce a new state.
 */
static void
SHA256_Transform(uint32_t * state, const unsigned char block[64])
{
	/* SHA256 round constants. */
	static const uint32_t K[64] = {
		0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
		0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
		0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
		0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
		0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
		0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
		0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
		0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
		0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
		0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
		0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
		0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
		0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
		0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
		0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
		0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
	};
	uint32_t W[64];
	uint32_t S[8];
	int i;

#define S0(x)		(ROTR(x, 2) ^ ROTR(x, 13) ^ ROTR(x, 22))
#define S1(x)		(ROTR(x, 6) ^ ROTR(x, 11) ^ ROTR(x, 25))
#define s0(x)		(ROTR(x, 7) ^ ROTR(x, 18) ^ (x >> 3))
#define s1(x)		(ROTR(x, 17) ^ ROTR(x, 19) ^ (x >> 10))

/* SHA256 round function */
#define RND(a, b, c, d, e, f, g, h, k)			\
	h += S1(e) + Ch(e, f, g) + k;			\
	d += h;						\
	h += S0(a) + Maj(a, b, c);

/* Adjusted round function for rotating state */
#define RNDr(S, W, i, ii)			\
	RND(S[(64 - i) % 8], S[(65 - i) % 8],	\
	    S[(66 - i) % 8], S[(67 - i) % 8],	\
	    S[(68 - i) % 8], S[(69 - i) % 8],	\
	    S[(70 - i) % 8], S[(71 - i) % 8],	\
	    W[i + ii] + K[i + ii])

/* Message schedule computation */
#define MSCH(W, ii, i)				\
	W[i + ii + 16] = s1(W[i + ii + 14]) + W[i + ii + 9] + s0(W[i + ii + 1]) + W[i + ii]

	/* 1. Prepare the first part of the message schedule W. */
	be32dec_vect(W, block, 64);

	/* 2. Initialize working variables. */
	memcpy(S, state, 32);

	/* 3. Mix. */
	for (i = 0; i < 64; i += 16) {
		RNDr(S, W, 0, i);
		RNDr(S, W, 1, i);
		RNDr(S, W, 2, i);
		RNDr(S, W, 3, i);
		RNDr(S, W, 4, i);
		RNDr(S, W, 5, i);
		RNDr(S, W, 6, i);
		RNDr(S, W, 7, i);
		RNDr(S, W, 8, i);
		RNDr(S, W, 9, i);
		RNDr(S, W, 10, i);
		RNDr(S, W, 11, i);
		RNDr(S, W, 12, i);
		RNDr(S, W, 13, i);
		RNDr(S, W, 14, i);
		RNDr(S, W, 15, i);

		if (i == 48)
			break;
		MSCH(W, 0, i);
		MSCH(W, 1, i);
		MSCH(W, 2, i);
		MSCH(W, 3, i);
		MSCH(W, 4, i);
		MSCH(W, 5, i);
		MSCH(W, 6, i);
		MSCH(W, 7, i);
		MSCH(W, 8, i);
		MSCH(W, 9, i);
		MSCH(W, 10, i);
		MSCH(W, 11, i);
		MSCH(W, 12, i);
		MSCH(W, 13, i);
		MSCH(W, 14, i);
		MSCH(W, 15, i);
	}

#undef S0
#undef s0
#undef S1
#undef s1
#undef RND
#undef RNDr
#undef MSCH

	/* 4. Mix local working variables into global state */
	for (i = 0; i < 8; i++)
		state[i] += S[i];
}

static unsigned char PAD[64] = {
	0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/* Add padding and terminating bit-count. */
static void
SHA256_Pad(SHA256_CTX * ctx)
{
	size_t r;

	/* Figure out how many bytes we have buffered. */
	r = (ctx->count >> 3) & 0x3f;

	/* Pad to 56 mod 64, transforming if we finish a block en route. */
	if (r < 56) {
		/* Pad to 56 mod 64. */
		memcpy(&ctx->buf[r], PAD, 56 - r);
	} else {
		/* Finish the current block and mix. */
		memcpy(&ctx->buf[r], PAD, 64 - r);
		SHA256_Transform(ctx->state, ctx->buf);

		/* The start of the final block is all zeroes. */
		memset(&ctx->buf[0], 0, 56);
	}

	/* Add the terminating bit-count. */
	be64enc(&ctx->buf[56], ctx->count);

	/* Mix in the final block. */
	SHA256_Transform(ctx->state, ctx->buf);
}

/* SHA-256 initialization.  Begins a SHA-256 operation. */
static void
SHA256_Init(SHA256_CTX * ctx)
{

	/* Zero bits processed so far */
	ctx->count = 0;

	/* Magic initialization constants */
	ctx->state[0] = 0x6A09E667;
	ctx->state[1] = 0xBB67AE85;
	ctx->state[2] = 0x3C6EF372;
	ctx->state[3] = 0xA54FF53A;
	ctx->state[4] = 0x510E527F;
	ctx->state[5] = 0x9B05688C;
	ctx->state[6] = 0x1F83D9AB;
	ctx->state[7] = 0x5BE0CD19;
}

/* Add bytes into the hash */
static void
SHA256_Update(SHA256_CTX * ctx, const void *in, size_t len)
{
	uint64_t bitlen;
	uint32_t r;
	const unsigned char *src = in;

	/* Number of bytes left in the buffer from previous updates */
	r = (ctx->count >> 3) & 0x3f;

	/* Convert the length into a number of bits */
	bitlen = len << 3;

	/* Update number of bits */
	ctx->count += bitlen;

	/* Handle the case where we don't need to perform any transforms */
	if (len < 64 - r) {
		memcpy(&ctx->buf[r], src, len);
		return;
	}

	/* Finish the current block */
	memcpy(&ctx->buf[r], src, 64 - r);
	SHA256_Transform(ctx->state, ctx->buf);
	src += 64 - r;
	len -= 64 - r;

	/* Perform complete blocks */
	while (len >= 64) {
		SHA256_Transform(ctx->state, src);
		src += 64;
		len -= 64;
	}

	/* Copy left over data into buffer */
	memcpy(ctx->buf, src, len);
}

/*
 * SHA-256 finalization.  Pads the input data, exports the hash value,
 * and clears the context state.
 */
static void
SHA256_Final(unsigned char digest[static SHA256_DIGEST_LENGTH], SHA256_CTX *ctx)
{
	/* Add padding */
	SHA256_Pad(ctx);

	/* Write the hash */
	be32enc_vect(digest, ctx->state, SHA256_DIGEST_LENGTH);

	/* Clear the context state */
	memset(ctx, 0, sizeof(*ctx));
}

static void *hash_buf(FILE *f, int *len)
{
	static char buf[1024];

	*len = fread(buf, 1, sizeof(buf), f);

	return *len > 0 ? buf : NULL;
}

static char *hash_string(unsigned char *buf, int len)
{
	static char str[SHA256_DIGEST_LENGTH * 2 + 1];
	int i;

	if (len * 2 + 1 > sizeof(str))
		return NULL;

	for (i = 0; i < len; i++)
		sprintf(&str[i * 2], "%02x", buf[i]);

	return str;
}

static const char *md5_hash(FILE *f)
{
	MD5_CTX ctx;
	unsigned char val[MD5_DIGEST_LENGTH];
	void *buf;
	int len;

	MD5_begin(&ctx);
	while ((buf = hash_buf(f, &len)) != NULL)
		MD5_hash(buf, len, &ctx);
	MD5_end(val, &ctx);

	return hash_string(val, MD5_DIGEST_LENGTH);
}

static const char *sha256_hash(FILE *f)
{
	SHA256_CTX ctx;
	unsigned char val[SHA256_DIGEST_LENGTH];
	void *buf;
	int len;

	SHA256_Init(&ctx);
	while ((buf = hash_buf(f, &len)) != NULL)
		SHA256_Update(&ctx, buf, len);
	SHA256_Final(val, &ctx);

	return hash_string(val, SHA256_DIGEST_LENGTH);
}


struct hash_type {
	const char *name;
	const char *(*func)(FILE *f);
	int len;
};

struct hash_type types[] = {
	{ "md5", md5_hash, MD5_DIGEST_LENGTH },
	{ "sha256", sha256_hash, SHA256_DIGEST_LENGTH },
};


static int usage(const char *progname)
{
	int i;

	fprintf(stderr, "Usage: %s <hash type> [options] [<file>...]\n"
		"Options:\n"
		"	-n		Print filename(s)\n"
		"	-N		Suppress trailing newline\n"
		"\n"
		"Supported hash types:", progname);

	for (i = 0; i < ARRAY_SIZE(types); i++)
		fprintf(stderr, "%s %s", i ? "," : "", types[i].name);

	fprintf(stderr, "\n");
	return 1;
}

static struct hash_type *get_hash_type(const char *name)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(types); i++) {
		struct hash_type *t = &types[i];

		if (!strcmp(t->name, name))
			return t;
	}
	return NULL;
}


static int hash_file(struct hash_type *t, const char *filename, bool add_filename,
	bool no_newline)
{
	const char *str;

	if (!filename || !strcmp(filename, "-")) {
		str = t->func(stdin);
	} else {
		struct stat path_stat;
		stat(filename, &path_stat);
		if (S_ISDIR(path_stat.st_mode)) {
			fprintf(stderr, "Failed to open '%s': Is a directory\n", filename);
			return 1;
		}

		FILE *f = fopen(filename, "r");

		if (!f) {
			fprintf(stderr, "Failed to open '%s'\n", filename);
			return 1;
		}
		str = t->func(f);
		fclose(f);
	}

	if (!str) {
		fprintf(stderr, "Failed to generate hash\n");
		return 1;
	}

	if (add_filename)
		printf("%s %s%s", str, filename ? filename : "-",
			no_newline ? "" : "\n");
	else
		printf("%s%s", str, no_newline ? "" : "\n");
	return 0;
}


int main(int argc, char **argv)
{
	struct hash_type *t;
	const char *progname = argv[0];
	int i, ch;
	bool add_filename = false, no_newline = false;

	while ((ch = getopt(argc, argv, "nN")) != -1) {
		switch (ch) {
		case 'n':
			add_filename = true;
			break;
		case 'N':
			no_newline = true;
			break;
		default:
			return usage(progname);
		}
	}

	argc -= optind;
	argv += optind;

	if (argc < 1)
		return usage(progname);

	t = get_hash_type(argv[0]);
	if (!t)
		return usage(progname);

	if (argc < 2)
		return hash_file(t, NULL, add_filename, no_newline);

	for (i = 0; i < argc - 1; i++) {
		int ret = hash_file(t, argv[1 + i], add_filename, no_newline);
		if (ret)
			return ret;
	}

	return 0;
}

/*
 * Copyright (C) 2009-2011 Gabor Juhos <juhosg@openwrt.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>

#include "buffalo-lib.h"

static uint32_t crc32_table[256] =
{
	0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9,
	0x130476dc, 0x17c56b6b, 0x1a864db2, 0x1e475005,
	0x2608edb8, 0x22c9f00f, 0x2f8ad6d6, 0x2b4bcb61,
	0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd,
	0x4c11db70, 0x48d0c6c7, 0x4593e01e, 0x4152fda9,
	0x5f15adac, 0x5bd4b01b, 0x569796c2, 0x52568b75,
	0x6a1936c8, 0x6ed82b7f, 0x639b0da6, 0x675a1011,
	0x791d4014, 0x7ddc5da3, 0x709f7b7a, 0x745e66cd,
	0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039,
	0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5,
	0xbe2b5b58, 0xbaea46ef, 0xb7a96036, 0xb3687d81,
	0xad2f2d84, 0xa9ee3033, 0xa4ad16ea, 0xa06c0b5d,
	0xd4326d90, 0xd0f37027, 0xddb056fe, 0xd9714b49,
	0xc7361b4c, 0xc3f706fb, 0xceb42022, 0xca753d95,
	0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1,
	0xe13ef6f4, 0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d,
	0x34867077, 0x30476dc0, 0x3d044b19, 0x39c556ae,
	0x278206ab, 0x23431b1c, 0x2e003dc5, 0x2ac12072,
	0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16,
	0x018aeb13, 0x054bf6a4, 0x0808d07d, 0x0cc9cdca,
	0x7897ab07, 0x7c56b6b0, 0x71159069, 0x75d48dde,
	0x6b93dddb, 0x6f52c06c, 0x6211e6b5, 0x66d0fb02,
	0x5e9f46bf, 0x5a5e5b08, 0x571d7dd1, 0x53dc6066,
	0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba,
	0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e,
	0xbfa1b04b, 0xbb60adfc, 0xb6238b25, 0xb2e29692,
	0x8aad2b2f, 0x8e6c3698, 0x832f1041, 0x87ee0df6,
	0x99a95df3, 0x9d684044, 0x902b669d, 0x94ea7b2a,
	0xe0b41de7, 0xe4750050, 0xe9362689, 0xedf73b3e,
	0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2,
	0xc6bcf05f, 0xc27dede8, 0xcf3ecb31, 0xcbffd686,
	0xd5b88683, 0xd1799b34, 0xdc3abded, 0xd8fba05a,
	0x690ce0ee, 0x6dcdfd59, 0x608edb80, 0x644fc637,
	0x7a089632, 0x7ec98b85, 0x738aad5c, 0x774bb0eb,
	0x4f040d56, 0x4bc510e1, 0x46863638, 0x42472b8f,
	0x5c007b8a, 0x58c1663d, 0x558240e4, 0x51435d53,
	0x251d3b9e, 0x21dc2629, 0x2c9f00f0, 0x285e1d47,
	0x36194d42, 0x32d850f5, 0x3f9b762c, 0x3b5a6b9b,
	0x0315d626, 0x07d4cb91, 0x0a97ed48, 0x0e56f0ff,
	0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623,
	0xf12f560e, 0xf5ee4bb9, 0xf8ad6d60, 0xfc6c70d7,
	0xe22b20d2, 0xe6ea3d65, 0xeba91bbc, 0xef68060b,
	0xd727bbb6, 0xd3e6a601, 0xdea580d8, 0xda649d6f,
	0xc423cd6a, 0xc0e2d0dd, 0xcda1f604, 0xc960ebb3,
	0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7,
	0xae3afba2, 0xaafbe615, 0xa7b8c0cc, 0xa379dd7b,
	0x9b3660c6, 0x9ff77d71, 0x92b45ba8, 0x9675461f,
	0x8832161a, 0x8cf30bad, 0x81b02d74, 0x857130c3,
	0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640,
	0x4e8ee645, 0x4a4ffbf2, 0x470cdd2b, 0x43cdc09c,
	0x7b827d21, 0x7f436096, 0x7200464f, 0x76c15bf8,
	0x68860bfd, 0x6c47164a, 0x61043093, 0x65c52d24,
	0x119b4be9, 0x155a565e, 0x18197087, 0x1cd86d30,
	0x029f3d35, 0x065e2082, 0x0b1d065b, 0x0fdc1bec,
	0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088,
	0x2497d08d, 0x2056cd3a, 0x2d15ebe3, 0x29d4f654,
	0xc5a92679, 0xc1683bce, 0xcc2b1d17, 0xc8ea00a0,
	0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb, 0xdbee767c,
	0xe3a1cbc1, 0xe760d676, 0xea23f0af, 0xeee2ed18,
	0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4,
	0x89b8fd09, 0x8d79e0be, 0x803ac667, 0x84fbdbd0,
	0x9abc8bd5, 0x9e7d9662, 0x933eb0bb, 0x97ffad0c,
	0xafb010b1, 0xab710d06, 0xa6322bdf, 0xa2f33668,
	0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4
};

int bcrypt_init(struct bcrypt_ctx *ctx, void *key, int keylen,
		unsigned long state_len)
{
	unsigned char *state;
	unsigned char *p = key;
	unsigned long i, j;
	unsigned long k = 0;

	state = malloc(state_len);
	if (state == NULL)
		return -1;

	ctx->i = 0;
	ctx->j = 0;
	ctx->state = state;
	ctx->state_len = state_len;

	for (i = 0; i < state_len; i++)
		state[i] = i;

	for(i = 0, j = 0; i < state_len; i++, j = (j + 1) % keylen) {
		unsigned char t;

		t = state[i];
		k = (k + p[j] + t) % state_len;
		state[i] = state[k];
		state[k] = t;
	}

	return 0;
}

int bcrypt_process(struct bcrypt_ctx *ctx, unsigned char *src,
		   unsigned char *dst, unsigned long len)
{
	unsigned char *state = ctx->state;
	unsigned long state_len = ctx->state_len;
	unsigned char i, j;
	unsigned long k;

	i = ctx->i;
	j = ctx->j;

	for (k = 0; k < len; k++) {
		unsigned char t;

		i = (i + 1) % state_len;
		j = (j + state[i]) % state_len;
		t = state[j];
		state[j] = state[i];
		state[i] = t;

		dst[k] = src[k] ^ state[(state[i] + state[j]) % state_len];
	}

	ctx->i = i;
	ctx->j = j;

	return len;
}

void bcrypt_finish(struct bcrypt_ctx *ctx)
{
	if (ctx->state)
		free(ctx->state);
}

int bcrypt_buf(unsigned char seed, unsigned char *key, unsigned char *src,
	       unsigned char *dst, unsigned long len, int longstate)
{
	unsigned char bckey[BCRYPT_MAX_KEYLEN + 1];
	unsigned int keylen;
	struct bcrypt_ctx ctx;
	int ret;

	/* setup decryption key */
	keylen = strlen((char *) key);
	bckey[0] = seed;
	memcpy(&bckey[1], key, keylen);

	keylen++;

	ret = bcrypt_init(&ctx, bckey, keylen,
			  (longstate) ? len : BCRYPT_DEFAULT_STATE_LEN);
	if (ret)
		return ret;

	bcrypt_process(&ctx, src, dst, len);
	bcrypt_finish(&ctx);

	return 0;
}

uint32_t buffalo_csum(uint32_t csum, void *buf, unsigned long len)
{
	signed char *p = buf;

	while (len--) {
		int i;

		csum ^= *p++;
		for (i = 0; i < 8; i++)
			csum = (csum >> 1) ^ ((csum & 1) ? 0xedb88320ul : 0);
	}

	return csum;
}

uint32_t buffalo_crc(void *buf, unsigned long len)
{
	unsigned char *p = buf;
	unsigned long t = len;
	uint32_t crc = 0;

	while (len--)
		crc = (crc << 8) ^ crc32_table[((crc >> 24) ^ *p++) & 0xFF];

	while (t) {
		crc = (crc << 8) ^ crc32_table[((crc >> 24) ^ t) & 0xFF];
		t >>= 8;
	}

	return ~crc;
}

unsigned long enc_compute_header_len(char *product, char *version)
{
	return ENC_MAGIC_LEN + 1 + strlen(product) + 1 +
	       strlen(version) + 1 + 3 * sizeof(uint32_t);
}

unsigned long enc_compute_buf_len(char *product, char *version,
				  unsigned long datalen)
{
	unsigned long ret;

	ret = enc_compute_header_len(product, version);
	ret += datalen + sizeof(uint32_t);
	ret += (4 - ret % 4);

	return ret;
}

static void put_be32(void *data, uint32_t val)
{
	unsigned char *p = data;

	p[0] = (val >> 24) & 0xff;
	p[1] = (val >> 16) & 0xff;
	p[2] = (val >> 8) & 0xff;
	p[3] = val & 0xff;
}

static uint32_t get_be32(void *data)
{
	unsigned char *p = data;

	return (((uint32_t)p[0]) << 24) |
	       (((uint32_t)p[1]) << 16) |
	       (((uint32_t)p[2]) << 8) |
	       ((uint32_t)p[3]);
}

static int check_magic(void *magic)
{
	if (!memcmp("start", magic, ENC_MAGIC_LEN))
		return 0;

	if (!memcmp("asar1", magic, ENC_MAGIC_LEN))
		return 0;

	return -1;
}

int encrypt_buf(struct enc_param *ep, unsigned char *hdr,
		unsigned char *data)
{
	unsigned char *p;
	uint32_t len;
	int err;
	int ret = -1;
	unsigned char s;

	p = (unsigned char *) hdr;

	/* setup magic */
	len = strlen((char *) ep->magic) + 1;
	memcpy(p, ep->magic, len);
	p += len;

	/* setup seed */
	*p++ = ep->seed;

	/* put product len */
	len = strlen((char *) ep->product) + 1;
	put_be32(p, len);
	p += sizeof(uint32_t);

	/* copy and crypt product name */
	memcpy(p, ep->product, len);
	err = bcrypt_buf(ep->seed, ep->key, p, p, len, ep->longstate);
	if (err)
		goto out;
	s = *p;
	p += len;

	/* put version length */
	len = strlen((char *) ep->version) + 1;
	put_be32(p, len);
	p += sizeof(uint32_t);

	/* copy and crypt version */
	memcpy(p, ep->version, len);
	err = bcrypt_buf(s, ep->key, p, p, len, ep->longstate);
	if (err)
		goto out;
	s = *p;
	p += len;

	/* put data length */
	put_be32(p, ep->datalen);

	/* encrypt data */
	err = bcrypt_buf(s, ep->key, data, data, ep->datalen, ep->longstate);
	if (err)
		goto out;

	/* put checksum */
	put_be32(&data[ep->datalen], ep->csum);

	ret = 0;

out:
	return ret;
}

int decrypt_buf(struct enc_param *ep, unsigned char *data,
		unsigned long datalen)
{
	unsigned char *p;
	uint32_t prod_len;
	uint32_t ver_len;
	uint32_t len;
	uint32_t csum;
	ssize_t remain;
	int err;
	int ret = -1;

#define CHECKLEN(_l) do {		\
	len = (_l);			\
	if (remain < len) {		\
		goto out;		\
	}				\
} while (0)

#define INCP() do {			\
	p += len;			\
	remain -= len;			\
} while (0)

	remain = datalen;
	p = data;

	CHECKLEN(ENC_MAGIC_LEN);
	err = check_magic(p);
	if (err)
		goto out;
	memcpy(ep->magic, p, ENC_MAGIC_LEN);
	INCP();

	CHECKLEN(1);
	ep->seed = *p;
	INCP();

	CHECKLEN(sizeof(uint32_t));
	prod_len = get_be32(p);
	if (prod_len > ENC_PRODUCT_LEN)
		goto out;
	INCP();

	CHECKLEN(prod_len);
	memcpy(ep->product, p, prod_len);
	INCP();

	CHECKLEN(sizeof(uint32_t));
	ver_len = get_be32(p);
	if (ver_len > ENC_VERSION_LEN)
		goto out;
	INCP();

	CHECKLEN(ver_len);
	memcpy(ep->version, p, ver_len);
	INCP();

	CHECKLEN(sizeof(uint32_t));
	ep->datalen = get_be32(p);
	INCP();

	/* decrypt data */
	CHECKLEN(ep->datalen);
	err = bcrypt_buf(ep->version[0], ep->key, p, data, ep->datalen,
			 ep->longstate);
	if (err)
		goto out;
	INCP();

	CHECKLEN(sizeof(uint32_t));
	ep->csum = get_be32(p);
	INCP();

	csum = buffalo_csum(ep->datalen, data, ep->datalen);
	if (csum != ep->csum)
		goto out;

	/* decrypt product name */
	err = bcrypt_buf(ep->product[0], ep->key, ep->version, ep->version,
			 ver_len, ep->longstate);
	if (err)
		goto out;

	/* decrypt version */
	err = bcrypt_buf(ep->seed, ep->key, ep->product, ep->product, prod_len,
			 ep->longstate);
	if (err)
		goto out;

	ret = 0;
out:
	return ret;

#undef CHECKLEN
#undef INCP
}

ssize_t get_file_size(char *name)
{
	struct stat st;
	int err;

	err = stat(name, &st);
	if (err)
		return -1;

	return st.st_size;
}

int read_file_to_buf(char *name, void *buf, ssize_t buflen)
{
	FILE *f;
	size_t done;
	int ret = -1;

	f = fopen(name, "r");
	if (f == NULL)
		goto out;

	errno = 0;
	done = fread(buf, buflen, 1, f);
	if (done != 1)
		goto close;

	ret = 0;

close:
	fclose(f);
out:
	return ret;
}

int write_buf_to_file(char *name, void *buf, ssize_t buflen)
{
	FILE *f;
	size_t done;
	int ret = -1;

	f = fopen(name, "w");
	if (f == NULL)
		goto out;

	errno = 0;
	done = fwrite(buf, buflen, 1, f);
	if (done != 1)
		goto close;

	ret = 0;

close:
	fflush(f);
	fclose(f);
	if (ret)
		unlink(name);
out:
	return ret;
}

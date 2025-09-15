/*
 * Copyright (C) 2008 Felix Fietkau <nbd@nbd.name>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include "ead.h"

#include "sha1.c"
#include "aes.c"

#if EAD_DEBUGLEVEL >= 1
#define DEBUG(n, format, ...) do { \
	if (EAD_DEBUGLEVEL >= n) \
		fprintf(stderr, format, ##__VA_ARGS__); \
} while (0);

#else
#define DEBUG(n, format, ...) do {} while(0)
#endif


static uint32_t aes_enc_ctx[AES_PRIV_SIZE];
static uint32_t aes_dec_ctx[AES_PRIV_SIZE];
static uint32_t ead_rx_iv;
static uint32_t ead_tx_iv;
static uint32_t ivofs_vec;
static unsigned int ivofs_idx = 0;
static uint32_t W[80]; /* work space for sha1 */

#define EAD_ENC_PAD	64

void
ead_set_key(unsigned char *skey)
{
	uint32_t *ivp = (uint32_t *)skey;

	memset(aes_enc_ctx, 0, sizeof(aes_enc_ctx));
	memset(aes_dec_ctx, 0, sizeof(aes_dec_ctx));

	/* first 32 bytes of skey are used as aes key for
	 * encryption and decryption */
	rijndaelKeySetupEnc(aes_enc_ctx, skey);
	rijndaelKeySetupDec(aes_dec_ctx, skey);

	/* the following bytes are used as initialization vector for messages
	 * (highest byte cleared to avoid overflow) */
	ivp += 8;
	ead_rx_iv = ntohl(*ivp) & 0x00ffffff;
	ead_tx_iv = ead_rx_iv;

	/* the last bytes are used to feed the random iv increment */
	ivp++;
	ivofs_vec = *ivp;
}


static bool
ead_check_rx_iv(uint32_t iv)
{
	if (iv <= ead_rx_iv)
		return false;

	if (iv > ead_rx_iv + EAD_MAX_IV_INCR)
		return false;

	ead_rx_iv = iv;
	return true;
}


static uint32_t
ead_get_tx_iv(void)
{
	unsigned int ofs;

	ofs = 1 + ((ivofs_vec >> 2 * ivofs_idx) & 0x3);
	ivofs_idx = (ivofs_idx + 1) % 16;
	ead_tx_iv += ofs;

	return ead_tx_iv;
}

static void
ead_hash_message(struct ead_msg_encrypted *enc, uint32_t *hash, int len)
{
	unsigned char *data = (unsigned char *) enc;

	/* hash the packet with the stored hash part initialized to zero */
	sha_init(hash);
	memset(enc->hash, 0, sizeof(enc->hash));
	while (len > 0) {
		sha_transform(hash, data, W);
		len -= 64;
		data += 64;
	}
}

void
ead_encrypt_message(struct ead_msg *msg, unsigned int len)
{
	struct ead_msg_encrypted *enc = EAD_DATA(msg, enc);
	unsigned char *data = (unsigned char *) enc;
	uint32_t hash[5];
	int enclen, i;

	len += sizeof(struct ead_msg_encrypted);
	enc->pad = (EAD_ENC_PAD - (len % EAD_ENC_PAD)) % EAD_ENC_PAD;
	enclen = len + enc->pad;
	msg->len = htonl(enclen);
	enc->iv = htonl(ead_get_tx_iv());

	ead_hash_message(enc, hash, enclen);
	for (i = 0; i < 5; i++)
		enc->hash[i] = htonl(hash[i]);
	DEBUG(2, "SHA1 generate (0x%08x), len=%d\n", enc->hash[0], enclen);

	while (enclen > 0) {
		rijndaelEncrypt(aes_enc_ctx, data, data);
		data += 16;
		enclen -= 16;
	}
}

int
ead_decrypt_message(struct ead_msg *msg)
{
	struct ead_msg_encrypted *enc = EAD_DATA(msg, enc);
	unsigned char *data = (unsigned char *) enc;
	uint32_t hash_old[5], hash_new[5];
	int len = ntohl(msg->len);
	int i, enclen = len;

	if (!len || (len % EAD_ENC_PAD > 0))
		return 0;

	while (len > 0) {
		rijndaelDecrypt(aes_dec_ctx, data, data);
		data += 16;
		len -= 16;
	}

	data = (unsigned char *) enc;

	if (enc->pad >= EAD_ENC_PAD) {
		DEBUG(2, "Invalid padding length\n");
		return 0;
	}

	if (!ead_check_rx_iv(ntohl(enc->iv))) {
		DEBUG(2, "RX IV mismatch (0x%08x <> 0x%08x)\n", ead_rx_iv, ntohl(enc->iv));
		return 0;
	}

	for (i = 0; i < 5; i++)
		hash_old[i] = ntohl(enc->hash[i]);
	ead_hash_message(enc, hash_new, enclen);
	if (memcmp(hash_old, hash_new, sizeof(hash_old)) != 0) {
		DEBUG(2, "SHA1 mismatch (0x%08x != 0x%08x), len=%d\n", hash_old[0], hash_new[0], enclen);
		return 0;
	}

	enclen -= enc->pad + sizeof(struct ead_msg_encrypted);
	return enclen;
}

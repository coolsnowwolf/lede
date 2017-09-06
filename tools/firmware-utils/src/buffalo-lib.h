/*
 *  Copyright (C) 2009-2011 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 */

#ifndef _BUFFALO_LIB_H
#define _BUFFALO_LIB_H

#include <stdint.h>

#define ARRAY_SIZE(_a)	(sizeof((_a)) / sizeof((_a)[0]))
#define BIT(_x)		(1UL << (_x))

#define TAG_BRAND_LEN		32
#define TAG_PRODUCT_LEN		32
#define TAG_VERSION_LEN		8
#define TAG_REGION_LEN		2
#define TAG_LANGUAGE_LEN	8
#define TAG_PLATFORM_LEN	8
#define TAG_HWVER_LEN		4
#define TAG_HWVER_VAL_LEN	4

struct buffalo_tag {
	unsigned char	product[TAG_PRODUCT_LEN];
	unsigned char	brand[TAG_BRAND_LEN];
	unsigned char	ver_major[TAG_VERSION_LEN];
	unsigned char	ver_minor[TAG_VERSION_LEN];
	unsigned char	region_code[2];
	uint32_t	region_mask;
	unsigned char	unknown0[2];
	unsigned char	language[TAG_LANGUAGE_LEN];
	unsigned char	platform[TAG_PLATFORM_LEN];
	unsigned char	hwv[TAG_HWVER_LEN];
	unsigned char	hwv_val[TAG_HWVER_VAL_LEN];
	uint8_t		unknown1[24];

	uint32_t	len;
	uint32_t	crc;
	uint32_t	base1;
	uint32_t	base2;
	uint32_t	data_len;
	uint8_t		flag;
	uint8_t		unknown2[3];
} __attribute ((packed));

struct buffalo_tag2 {
	unsigned char	product[TAG_PRODUCT_LEN];
	unsigned char	brand[TAG_BRAND_LEN];
	unsigned char	ver_major[TAG_VERSION_LEN];
	unsigned char	ver_minor[TAG_VERSION_LEN];
	unsigned char	region_code[2];
	uint32_t	region_mask;
	unsigned char	unknown0[2];
	unsigned char	language[TAG_LANGUAGE_LEN];
	unsigned char	platform[TAG_PLATFORM_LEN];
	unsigned char	hwv[TAG_HWVER_LEN];
	unsigned char	hwv_val[TAG_HWVER_VAL_LEN];
	uint8_t		unknown1[24];

	uint32_t	total_len;
	uint32_t	crc;
	uint32_t	len1;
	uint32_t	len2;
	uint8_t		flag;
	uint8_t		unknown2[3];
} __attribute ((packed));

struct buffalo_tag3 {
	unsigned char	product[TAG_PRODUCT_LEN];
	unsigned char	brand[TAG_BRAND_LEN];
	unsigned char	ver_major[TAG_VERSION_LEN];
	unsigned char	ver_minor[TAG_VERSION_LEN];
	unsigned char	region_code[2];
	uint32_t	region_mask;
	unsigned char	unknown0[2];
	unsigned char	language[TAG_LANGUAGE_LEN];
	unsigned char	platform[TAG_PLATFORM_LEN];
	unsigned char	hwv[TAG_HWVER_LEN];
	unsigned char	hwv_val[TAG_HWVER_VAL_LEN];
	uint8_t		unknown1[24];

	uint32_t	total_len;
	uint32_t	crc;
	uint32_t	len1;
	uint32_t	base2;
} __attribute ((packed));

#define ENC_PRODUCT_LEN		32
#define ENC_VERSION_LEN		8
#define ENC_MAGIC_LEN		6

unsigned long enc_compute_header_len(char *product, char *version);
unsigned long enc_compute_buf_len(char *product, char *version,
				  unsigned long datalen);

struct enc_param {
	unsigned char *key;
	unsigned char magic[ENC_MAGIC_LEN];
	unsigned char product[ENC_PRODUCT_LEN];
	unsigned char version[ENC_VERSION_LEN];
	unsigned char seed;
	int longstate;
	unsigned datalen;
	uint32_t csum;
};

int encrypt_buf(struct enc_param *ep, unsigned char *hdr,
	        unsigned char *data);
int decrypt_buf(struct enc_param *ep, unsigned char *data,
		unsigned long datalen);

#define BCRYPT_DEFAULT_STATE_LEN	256
#define BCRYPT_MAX_KEYLEN		254

struct bcrypt_ctx {
	unsigned long i;
	unsigned long j;
	unsigned char *state;
	unsigned long state_len;
};

int bcrypt_init(struct bcrypt_ctx *ctx, void *key, int keylen,
		unsigned long state_len);
int bcrypt_process(struct bcrypt_ctx *ctx, unsigned char *src,
		   unsigned char *dst, unsigned long len);
void bcrypt_finish(struct bcrypt_ctx *ctx);
int bcrypt_buf(unsigned char seed, unsigned char *key, unsigned char *src,
	       unsigned char *dst, unsigned long len, int longstate);

uint32_t buffalo_csum(uint32_t csum, void *buf, unsigned long len);
uint32_t buffalo_crc(void *buf, unsigned long len);

ssize_t get_file_size(char *name);
int read_file_to_buf(char *name, void *buf, ssize_t buflen);
int write_buf_to_file(char *name, void *buf, ssize_t buflen);

#endif /* _BUFFALO_LIB_H */

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
#include <stdint.h>
#include <string.h>
#include <libgen.h>
#include <getopt.h>     /* for getopt() */
#include <stdarg.h>

#include "buffalo-lib.h"

#define ERR(fmt, args...) do { \
	fflush(0); \
	fprintf(stderr, "[%s] *** error: " fmt "\n", \
			progname, ## args ); \
} while (0)

static char *progname;
static char *ifname;
static char *ofname;
static char *crypt_key = "Buffalo";
static char *magic = "start";
static int longstate;
static unsigned char seed = 'O';

static char *product;
static char *version;
static int do_decrypt;
static int offset;
static int size;

void usage(int status)
{
	FILE *stream = (status != EXIT_SUCCESS) ? stderr : stdout;

	fprintf(stream, "Usage: %s [OPTIONS...]\n", progname);
	fprintf(stream,
"\n"
"Options:\n"
"  -d              decrypt instead of encrypt\n"
"  -i <file>       read input from the file <file>\n"
"  -o <file>       write output to the file <file>\n"
"  -l              use longstate {en,de}cryption method\n"
"  -k <key>        use <key> for encryption (default: Buffalo)\n"
"  -m <magic>      set magic to <magic>\n"
"  -p <product>    set product name to <product>\n"
"  -v <version>    set version to <version>\n"
"  -h              show this screen\n"
"  -O              Offset of encrypted data in file (decryption)\n"
"  -S              Size of unencrypted data in file (encryption)\n"
	);

	exit(status);
}

static int decrypt_file(void)
{
	struct enc_param ep;
	ssize_t src_len;
	unsigned char *buf = NULL;
	int err;
	int ret = -1;

	src_len = get_file_size(ifname);
	if (src_len < 0) {
		ERR("unable to get size of '%s'", ifname);
		goto out;
	}

	buf = malloc(src_len);
	if (buf == NULL) {
		ERR("no memory for the buffer");
		goto out;
	}

	err = read_file_to_buf(ifname, buf, src_len);
	if (err) {
		ERR("unable to read from file '%s'", ifname);
		goto out;
	}

	memset(&ep, '\0', sizeof(ep));
	ep.key = (unsigned char *) crypt_key;
	ep.longstate = longstate;

	err = decrypt_buf(&ep, buf + offset, src_len - offset);
	if (err) {
		ERR("unable to decrypt '%s'", ifname);
		goto out;
	}

	printf("Magic\t\t: '%s'\n", ep.magic);
	printf("Seed\t\t: 0x%02x\n", ep.seed);
	printf("Product\t\t: '%s'\n", ep.product);
	printf("Version\t\t: '%s'\n", ep.version);
	printf("Data len\t: %u\n", ep.datalen);
	printf("Checksum\t: 0x%08x\n", ep.csum);

	err = write_buf_to_file(ofname, buf + offset, ep.datalen);
	if (err) {
		ERR("unable to write to file '%s'", ofname);
		goto out;
	}

	ret = 0;

out:
	free(buf);
	return ret;
}

static int encrypt_file(void)
{
	struct enc_param ep;
	ssize_t src_len, tail_dst, tail_len, tail_src;
	unsigned char *buf;
	uint32_t hdrlen;
	ssize_t totlen = 0;
	int err;
	int ret = -1;

	src_len = get_file_size(ifname);
	if (src_len < 0) {
		ERR("unable to get size of '%s'", ifname);
		goto out;
	}

	if (size) {
		tail_dst = enc_compute_buf_len(product, version, size);
		tail_len = src_len - size;
		totlen = tail_dst + tail_len;
	} else
		totlen = enc_compute_buf_len(product, version, src_len);

	buf = malloc(totlen);
	if (buf == NULL) {
		ERR("no memory for the buffer");
		goto out;
	}

	hdrlen = enc_compute_header_len(product, version);

	err = read_file_to_buf(ifname, &buf[hdrlen], src_len);
	if (err) {
		ERR("unable to read from file '%s'", ofname);
		goto free_buf;
	}

	if (size) {
		tail_src = hdrlen + size;
		memmove(&buf[tail_dst], &buf[tail_src], tail_len);
		memset(&buf[tail_src], 0, tail_dst - tail_src);
		src_len = size;
	}

	memset(&ep, '\0', sizeof(ep));
	ep.key = (unsigned char *) crypt_key;
	ep.seed = seed;
	ep.longstate = longstate;
	ep.csum = buffalo_csum(src_len, &buf[hdrlen], src_len);
	ep.datalen = src_len;
	strcpy((char *) ep.magic, magic);
	strcpy((char *) ep.product, product);
	strcpy((char *) ep.version, version);

	err = encrypt_buf(&ep, buf, &buf[hdrlen]);
	if (err) {
		ERR("invalid input file");
		goto free_buf;
	}

	err = write_buf_to_file(ofname, buf, totlen);
	if (err) {
		ERR("unable to write to file '%s'", ofname);
		goto free_buf;
	}

	ret = 0;

free_buf:
 	free(buf);
out:
 	return ret;
}

static int check_params(void)
{
	int ret = -1;

	if (ifname == NULL) {
		ERR("no input file specified");
		goto out;
	}

	if (ofname == NULL) {
		ERR("no output file specified");
		goto out;
	}

	if (crypt_key == NULL) {
		ERR("no key specified");
		goto out;
	} else if (strlen(crypt_key) > BCRYPT_MAX_KEYLEN) {
		ERR("key '%s' is too long", crypt_key);
		goto out;
	}

	if (strlen(magic) != (ENC_MAGIC_LEN - 1)) {
		ERR("length of magic must be %d", ENC_MAGIC_LEN - 1);
		goto out;
	}

	if (!do_decrypt) {
		if (product == NULL) {
			ERR("no product specified");
			goto out;
		}

		if (version == NULL) {
			ERR("no version specified");
			goto out;
		}

		if (strlen(product) > (ENC_PRODUCT_LEN - 1)) {
			ERR("product name '%s' is too long", product);
			goto out;
		}

		if (strlen(version) > (ENC_VERSION_LEN - 1)) {
			ERR("version '%s' is too long", version);
			goto out;
		}
	}

	ret = 0;

out:
	return ret;
}

int main(int argc, char *argv[])
{
	int res = EXIT_FAILURE;
	int err;

	progname = basename(argv[0]);

	while ( 1 ) {
		int c;

		c = getopt(argc, argv, "adi:m:o:hlp:v:k:O:r:s:S:");
		if (c == -1)
			break;

		switch (c) {
		case 'd':
			do_decrypt = 1;
			break;
		case 'i':
			ifname = optarg;
			break;
		case 'l':
			longstate = 1;
			break;
		case 'm':
			magic = optarg;
			break;
		case 'o':
			ofname = optarg;
			break;
		case 'p':
			product = optarg;
			break;
		case 'v':
			version = optarg;
			break;
		case 'k':
			crypt_key = optarg;
			break;
		case 's':
			seed = strtoul(optarg, NULL, 16);
			break;
		case 'O':
			offset = strtoul(optarg, NULL, 0);
			break;
		case 'S':
			size = strtoul(optarg, NULL, 0);
			break;
		case 'h':
			usage(EXIT_SUCCESS);
			break;
		default:
			usage(EXIT_FAILURE);
			break;
		}
	}

	err = check_params();
	if (err)
		goto out;

	if (do_decrypt)
		err = decrypt_file();
	else
		err = encrypt_file();

	if (err)
		goto out;

	res = EXIT_SUCCESS;

out:
	return res;
}

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
static int do_decrypt;

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
"  -h              show this screen\n"
	);

	exit(status);
}

static const unsigned char *crypt_key1 = (unsigned char *)
	"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
static const unsigned char *crypt_key2 = (unsigned char *)
	"XYZ0123hijklmnopqABCDEFGHrstuvabcdefgwxyzIJKLMSTUVW456789NOPQR";

static void crypt_header(unsigned char *buf, ssize_t len,
			 const unsigned char *key1, const unsigned char *key2)
{
	ssize_t i;

	for (i = 0; i < len; i++) {
		unsigned int j;

		for (j = 0; key1[j]; j++)
			if (buf[i] == key1[j]) {
				buf[i] = key2[j];
				break;
			}
	}
}

static int crypt_file(void)
{
	unsigned char *buf = NULL;
	ssize_t src_len;
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

	if (do_decrypt)
		crypt_header(buf, 512, crypt_key2, crypt_key1);
	else
		crypt_header(buf, 512, crypt_key1, crypt_key2);

	err = write_buf_to_file(ofname, buf, src_len);
	if (err) {
		ERR("unable to write to file '%s'", ofname);
		goto out;
	}

	ret = 0;

out:
	free(buf);
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

		c = getopt(argc, argv, "di:o:h");
		if (c == -1)
			break;

		switch (c) {
		case 'd':
			do_decrypt = 1;
			break;
		case 'i':
			ifname = optarg;
			break;
		case 'o':
			ofname = optarg;
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

	err = crypt_file();
	if (err)
		goto out;

	res = EXIT_SUCCESS;

out:
	return res;
}

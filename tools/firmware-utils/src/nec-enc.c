// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * nec-enc.c - encode/decode nec firmware with key
 *
 * based on xorimage.c in OpenWrt
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

#define KEY_LEN     16
#define PATTERN_LEN 251

static int
xor_pattern(uint8_t *data, size_t len, const char *key, int k_len, int k_off)
{
	int offset = k_off;

	while (len--) {
		*data ^= key[offset];
		data++;
		offset = (offset + 1) % k_len;
	}

	return offset;
}

static void xor_data(uint8_t *data, size_t len, const uint8_t *pattern)
{
	for (int i = 0; i < len; i++) {
		*data ^= pattern[i];
		data++;
	}
}

static void __attribute__((noreturn)) usage(void)
{
	fprintf(stderr, "Usage: nec-enc -i infile -o outfile -k <key>\n");
	exit(EXIT_FAILURE);
}

static unsigned char buf_pattern[4096], buf[4096];

int main(int argc, char **argv)
{
	int k_off = 0, ptn = 1, c, ret = EXIT_SUCCESS;
	char *ifn = NULL, *ofn = NULL, *key = NULL;
	size_t n, k_len;
	FILE *out, *in;

	while ((c = getopt(argc, argv, "i:o:k:h")) != -1) {
		switch (c) {
		case 'i':
			ifn = optarg;
			break;
		case 'o':
			ofn = optarg;
			break;
		case 'k':
			key = optarg;
			break;
		case 'h':
		default:
			usage();
		}
	}

	if (optind != argc || optind == 1) {
		fprintf(stderr, "illegal arg \"%s\"\n", argv[optind]);
		usage();
	}

	in = fopen(ifn, "r");
	if (!in) {
		perror("can not open input file");
		usage();
	}

	out = fopen(ofn, "w");
	if (!out) {
		perror("can not open output file");
		usage();
	}

	if (!key) {
		fprintf(stderr, "key is not specified\n");
		usage();
	}

	k_len = strnlen(key, KEY_LEN + 1);
	if (k_len == 0 || k_len > KEY_LEN) {
		fprintf(stderr, "key length is not in range (0,%d)\n", KEY_LEN);
		usage();
	}

	while ((n = fread(buf, 1, sizeof(buf), in)) > 0) {
		for (int i = 0; i < n; i++) {
			buf_pattern[i] = ptn;
			ptn++;

			if (ptn > PATTERN_LEN)
				ptn = 1;
		}

		k_off = xor_pattern(buf_pattern, n, key, k_len, k_off);
		xor_data(buf, n, buf_pattern);

		if (fwrite(buf, 1, n, out) != n) {
			perror("failed to write");
			ret = EXIT_FAILURE;
			goto out;
		}
	}

	if (ferror(in)) {
		perror("failed to read");
		ret = EXIT_FAILURE;
		goto out;
	}

out:
	fclose(in);
	fclose(out);
	return ret;
}

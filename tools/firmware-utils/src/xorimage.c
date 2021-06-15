/*
 * xorimage.c - partially based on OpenWrt's addpattern.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/stat.h>

static char default_pattern[] = "12345678";
static int is_hex_pattern;


int xor_data(void *data, size_t len, const void *pattern, int p_len, int p_off)
{
	const uint8_t *key = pattern;
	uint8_t *d = data;

	while (len--) {
		*d ^= key[p_off];
		d++;
		p_off = (p_off + 1) % p_len;
	}
	return p_off;
}


void usage(void) __attribute__ (( __noreturn__ ));

void usage(void)
{
	fprintf(stderr, "Usage: xorimage [-i infile] [-o outfile] [-p <pattern>] [-x]\n");
	exit(EXIT_FAILURE);
}


int main(int argc, char **argv)
{
	char buf[1024];	/* keep this at 1k or adjust garbage calc below */
	FILE *in = stdin;
	FILE *out = stdout;
	char *ifn = NULL;
	char *ofn = NULL;
	const char *pattern = default_pattern;
	char hex_pattern[128];
	unsigned int hex_buf;
	int c;
	size_t n;
	int p_len, p_off = 0;

	while ((c = getopt(argc, argv, "i:o:p:xh")) != -1) {
		switch (c) {
			case 'i':
				ifn = optarg;
				break;
			case 'o':
				ofn = optarg;
				break;
			case 'p':
				pattern = optarg;
				break;
			case 'x':
				is_hex_pattern = true;
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

	if (ifn && !(in = fopen(ifn, "r"))) {
		fprintf(stderr, "can not open \"%s\" for reading\n", ifn);
		usage();
	}

	if (ofn && !(out = fopen(ofn, "w"))) {
		fprintf(stderr, "can not open \"%s\" for writing\n", ofn);
		usage();
	}

	p_len = strlen(pattern);

	if (p_len == 0) {
		fprintf(stderr, "pattern cannot be empty\n");
		usage();
	}

	if (is_hex_pattern) {
		int i;

		if ((p_len / 2) > sizeof(hex_pattern)) {
			fprintf(stderr, "provided hex pattern is too long\n");
			usage();
		}

		if (p_len % 2 != 0) {
			fprintf(stderr, "the number of characters (hex) is incorrect\n");
			usage();
		}

		for (i = 0; i < (p_len / 2); i++) {
			if (sscanf(pattern + (i * 2), "%2x", &hex_buf) < 0) {
				fprintf(stderr, "invalid hex digit around %d\n", i * 2);
				usage();
			}
			hex_pattern[i] = (char)hex_buf;
		}
	}

	while ((n = fread(buf, 1, sizeof(buf), in)) > 0) {
		if (n < sizeof(buf)) {
			if (ferror(in)) {
			FREAD_ERROR:
				fprintf(stderr, "fread error\n");
				return EXIT_FAILURE;
			}
		}

		if (is_hex_pattern) {
			p_off = xor_data(buf, n, hex_pattern, (p_len / 2),
					 p_off);
		} else {
			p_off = xor_data(buf, n, pattern, p_len, p_off);
		}

		if (!fwrite(buf, n, 1, out)) {
		FWRITE_ERROR:
			fprintf(stderr, "fwrite error\n");
			return EXIT_FAILURE;
		}
	}

	if (ferror(in)) {
		goto FREAD_ERROR;
	}

	if (fflush(out)) {
		goto FWRITE_ERROR;
	}

	fclose(in);
	fclose(out);

	return EXIT_SUCCESS;
}

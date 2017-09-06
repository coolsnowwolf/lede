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
#include <stdint.h>
#include <unistd.h>
#include <sys/stat.h>

static char default_pattern[] = "12345678";


int xor_data(uint8_t *data, size_t len, const uint8_t *pattern, int p_len, int p_off)
{
	int offset = p_off;
	while (len--) {
		*data ^= pattern[offset];
		data++;
		offset = (offset + 1) % p_len;
	}
	return offset;
}


void usage(void) __attribute__ (( __noreturn__ ));

void usage(void)
{
	fprintf(stderr, "Usage: xorimage [-i infile] [-o outfile] [-p <pattern>]\n");
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
	int c;
	int v0, v1, v2;
	size_t n;
	int p_len, p_off = 0;

	while ((c = getopt(argc, argv, "i:o:p:h")) != -1) {
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


	while ((n = fread(buf, 1, sizeof(buf), in)) > 0) {
		if (n < sizeof(buf)) {
			if (ferror(in)) {
			FREAD_ERROR:
				fprintf(stderr, "fread error\n");
				return EXIT_FAILURE;
			}
		}

		p_off = xor_data(buf, n, pattern, p_len, p_off);

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

/*
 * mkbrnimg.c - partially based on OpenWrt's wndr3700.c
 *
 * Copyright (C) 2011 Tobias Diedrich <ranma+openwrt@tdiedrich.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License,
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <netinet/in.h>
#include <inttypes.h>

#define BPB 8 /* bits/byte */

static uint32_t crc32[1<<BPB];

static char *output_file = "default-brnImage";
static uint32_t magic = 0x12345678;
static char *signature = "BRNDTW502";
static uint32_t crc32_poly = 0x2083b8ed;

static void init_crc32()
{
	const uint32_t poly = ntohl(crc32_poly);
	int n;

	for (n = 0; n < 1<<BPB; n++) {
		uint32_t crc = n;
		int bit;

		for (bit = 0; bit < BPB; bit++)
			crc = (crc & 1) ? (poly ^ (crc >> 1)) : (crc >> 1);
		crc32[n] = crc;
	}
}

static uint32_t crc32buf(const void *buf, size_t len)
{
	uint32_t crc = 0xFFFFFFFF;
	const uint8_t *in = buf;

	for (; len; len--, in++)
		crc = crc32[(uint8_t)crc ^ *in] ^ (crc >> BPB);
	return ~crc;
}

static void usage(const char *) __attribute__ (( __noreturn__ ));

static void usage(const char *mess)
{
	fprintf(stderr, "Error: %s\n", mess);
	fprintf(stderr, "Usage: mkbrnimg [-o output_file] [-m magic] [-s signature] [-p crc32 poly] kernel_file [additional files]\n");
	fprintf(stderr, "\n");
	exit(1);
}

static void parseopts(int *argc, char ***argv)
{
	char *endptr;
	int res;

	while ((res = getopt(*argc, *argv, "o:m:s:p:")) != -1) {
		switch (res) {
		default:
			usage("Unknown option");
			break;
		case 'o':
			output_file = optarg;
			break;
		case 'm':
			magic = strtoul(optarg, &endptr, 0);
			if (endptr == optarg || *endptr != 0)
				usage("magic must be a decimal or hexadecimal 32-bit value");
			break;
		case 's':
			signature = optarg;
			break;
		case 'p':
			crc32_poly = strtoul(optarg, &endptr, 0);
			if (endptr == optarg || *endptr != 0)
				usage("'crc32 poly' must be a decimal or hexadecimal 32-bit value");
			break;
		}
	}
	*argc -= optind;
	*argv += optind;
}

static void appendfile(int outfd, char *path, int kernel) {
	int fd;
	size_t len, padded_len;
	char *input_file;
	uint32_t crc;
	char padding[0x400];
	char footer[12];

	memset(padding, 0xff, sizeof(padding));

	// mmap input_file
	if ((fd = open(path, O_RDONLY))  < 0
	|| (len = lseek(fd, 0, SEEK_END)) < 0
	|| (input_file = mmap(0, len, PROT_READ, MAP_SHARED, fd, 0)) == (void *) (-1)
	|| close(fd) < 0)
	{
		fprintf(stderr, "Error mapping file '%s': %s\n", path, strerror(errno));
		exit(1);
	}

	// kernel should be lzma compressed image, not uImage
	if (kernel &&
	    (input_file[0] != (char)0x5d ||
	     input_file[1] != (char)0x00 ||
	     input_file[2] != (char)0x00 ||
	     input_file[3] != (char)0x80)) {
		fprintf(stderr, "lzma signature not found on kernel image.\n");
		exit(1);
	}

	init_crc32();
	crc = crc32buf(input_file, len);
	fprintf(stderr, "crc32 for '%s' is %08x.\n", path, crc);

	// write the file
	write(outfd, input_file, len);

	// write padding
	padded_len = ((len + sizeof(footer) + sizeof(padding) - 1) & ~(sizeof(padding) - 1)) - sizeof(footer);
	fprintf(stderr, "len=%08zx padded_len=%08zx\n", len, padded_len);
	write(outfd, padding, padded_len - len);

	// write footer
	footer[0]  = (len   >>  0) & 0xff;
	footer[1]  = (len   >>  8) & 0xff;
	footer[2]  = (len   >> 16) & 0xff;
	footer[3]  = (len   >> 24) & 0xff;
	footer[4]  = (magic >>  0) & 0xff;
	footer[5]  = (magic >>  8) & 0xff;
	footer[6]  = (magic >> 16) & 0xff;
	footer[7]  = (magic >> 24) & 0xff;
	footer[8]  = (crc   >>  0) & 0xff;
	footer[9]  = (crc   >>  8) & 0xff;
	footer[10] = (crc   >> 16) & 0xff;
	footer[11] = (crc   >> 24) & 0xff;
	write(outfd, footer, sizeof(footer));

	munmap(input_file, len);
}

int main(int argc, char **argv)
{
	int outfd;
	int i;

	parseopts(&argc, &argv);

	if (argc < 1)
		usage("wrong number of arguments");

	if ((outfd = open(output_file, O_WRONLY|O_CREAT|O_TRUNC, 0644)) == -1)
	{
		fprintf(stderr, "Error opening '%s' for writing: %s\n", output_file, strerror(errno));
		exit(1);
	}

	for (i=0; i<argc; i++) {
		appendfile(outfd, argv[i], i == 0);
	}
	write(outfd, signature, strlen(signature)+1);
	close(outfd);

	return 0;
}

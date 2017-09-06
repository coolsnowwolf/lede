/*
 * add_header.c - partially based on OpenWrt's motorola-bin.c
 *
 * Copyright (C) 2008 Imre Kaloz  <kaloz@openwrt.org>
 *                    Gabor Juhos <juhosg@openwrt.org>
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

/*
 * The add_header utility used by various vendors preprends the buf
 * image with a header containing a CRC32 value which is generated for the
 * model id + reserved space for CRC32 + buf, then replaces the reserved
 * area with the actual CRC32. This replacement tool mimics this behavior.
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

static void init_crc32()
{
	const uint32_t poly = ntohl(0x2083b8ed);
	int n;

	for (n = 0; n < 1<<BPB; n++) {
		uint32_t crc = n;
		int bit;

		for (bit = 0; bit < BPB; bit++)
			crc = (crc & 1) ? (poly ^ (crc >> 1)) : (crc >> 1);
		crc32[n] = crc;
	}
}

static uint32_t crc32buf(unsigned char *buf, size_t len)
{
	uint32_t crc = 0xFFFFFFFF;

	for (; len; len--, buf++)
		crc = crc32[(uint8_t)crc ^ *buf] ^ (crc >> BPB);
	return ~crc;
}

struct header {
	unsigned char model[16];
	uint32_t crc;
};

static void usage(const char *) __attribute__ (( __noreturn__ ));

static void usage(const char *mess)
{
	fprintf(stderr, "Error: %s\n", mess);
	fprintf(stderr, "Usage: add_header model_id input_file output_file\n");
	fprintf(stderr, "\n");
	exit(1);
}

int main(int argc, char **argv)
{
	off_t len;			// of original buf
	off_t buflen;			// of the output file
	int fd;
	void *input_file;		// pointer to the input file (mmmapped)
	struct header header;
	unsigned char *buf;	// pointer to prefix + copy of original buf

	// verify parameters

	if (argc != 4)
		usage("wrong number of arguments");

	// mmap input_file
	if ((fd = open(argv[2], O_RDONLY))  < 0
	|| (len = lseek(fd, 0, SEEK_END)) < 0
	|| (input_file = mmap(0, len, PROT_READ, MAP_SHARED, fd, 0)) == (void *) (-1)
	|| close(fd) < 0)
	{
		fprintf(stderr, "Error loading file %s: %s\n", argv[2], strerror(errno));
		exit(1);
	}

	buflen = len + sizeof(header);

	init_crc32();

	// copy model name into header
	strncpy(header.model, argv[1], sizeof(header.model));
	header.crc = 0;

	// create a firmware image in memory and copy the input_file to it
	buf = malloc(buflen);
	memcpy(buf, &header, sizeof(header));
	memcpy(&buf[sizeof(header)], input_file, len);

	// CRC of temporary header + buf
	header.crc = htonl(crc32buf(buf, buflen));

	memcpy(buf, &header, sizeof(header));

	// write the buf
	if ((fd = open(argv[3], O_CREAT|O_WRONLY|O_TRUNC,0644)) < 0
	|| write(fd, buf, buflen) != buflen
	|| close(fd) < 0)
	{
		fprintf(stderr, "Error storing file %s: %s\n", argv[3], strerror(errno));
		exit(2);
 	}

	free(buf);

	munmap(input_file,len);

	return 0;
}

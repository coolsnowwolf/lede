/*
 * Copyright (c) 2016 FUKAUMI Naoki <naobsd@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 *
 */

#include <sys/stat.h>
#include <err.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "buffalo-lib.h"

#define DHP_HEADER_SIZE	20

static char *progname;

static void
usage(void)
{

	fprintf(stderr, "usage: %s <in> <out>\n", progname);
	exit(EXIT_FAILURE);
}

int
main(int argc, char *argv[])
{
	struct stat in_st;
	size_t size;
	uint32_t crc;
	int in, out;
	uint8_t *buf;

	progname = argv[0];

	if (argc != 3)
		usage();

	if ((in = open(argv[1], O_RDONLY)) == -1)
		err(EXIT_FAILURE, "%s", argv[1]);

	if (fstat(in, &in_st) == -1)
		err(EXIT_FAILURE, "%s", argv[1]);

	size = DHP_HEADER_SIZE + in_st.st_size;

	if ((buf = malloc(size)) == NULL)
		err(EXIT_FAILURE, "malloc");

	memset(buf, 0, DHP_HEADER_SIZE);
	buf[0x0] = 0x62;
	buf[0x1] = 0x67;
	buf[0x2] = 0x6e;
	buf[0xb] = 0xb1;
	buf[0xc] = (size >> 24) & 0xff;
	buf[0xd] = (size >> 16) & 0xff;
	buf[0xe] = (size >> 8) & 0xff;
	buf[0xf] = size & 0xff;

	read(in, &buf[DHP_HEADER_SIZE], in_st.st_size);
	close(in);

	crc = buffalo_crc(buf, size);
	buf[0x10] = (crc >> 24) & 0xff;
	buf[0x11] = (crc >> 16) & 0xff;
	buf[0x12] = (crc >> 8) & 0xff;
	buf[0x13] = crc & 0xff;

	if ((out = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644)) == -1)
		err(EXIT_FAILURE, "%s", argv[2]);
	write(out, buf, size);
	close(out);

	free(buf);

	return EXIT_SUCCESS;
}

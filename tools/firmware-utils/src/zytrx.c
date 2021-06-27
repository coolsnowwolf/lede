// SPDX-License-Identifier: GPL-2.0-only
/*
 * zytrx - add header to images for ZyXEL NR7101
 *
 * Based on add_header.c - partially based on OpenWrt's
 * motorola-bin.c
 *
 * Copyright (C) 2008 Imre Kaloz  <kaloz@openwrt.org>
 *                    Gabor Juhos <juhosg@openwrt.org>
 * Copyright (C) 2021 Bjørn Mork <bjorn@mork.no>

 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License,
 * version 2 as published by the Free Software Foundation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <string.h>
#include <netinet/in.h>
#include <inttypes.h>

#define BPB 8 /* bits/byte */

static uint32_t crc32[1<<BPB];

static void init_crc32(void)
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

static uint32_t crc32buf(const unsigned char *buf, size_t len)
{
	uint32_t crc = 0xFFFFFFFF;

	for (; len; len--, buf++)
		crc = crc32[(uint8_t)crc ^ *buf] ^ (crc >> BPB);
	return ~crc;
}

/* HDR0 reversed, to be stored as BE */
#define MAGIC		0x30524448  /* HDR0 reversed, to be stored as BE */

/* All numbers are stored as BE */
struct zytrx_t {
	uint32_t magic;
	uint32_t len_h;		/* Length of this header */
	uint32_t len_t;		/* Total length of file  */
	uint32_t crc32_p;	/* Bit inverted 32-bit CRC of image payload */
	uint8_t  verInt[32];	/* String "5.0.0.0\n" zero padded */
	uint8_t  verExt[32];	/* String "\n" zero padded */
	uint32_t len_p;		/* Length of image payload */
	uint8_t  pad1[12];	/* zero padding(?) */
	uint8_t  code[164];	/* string "3 6035 122 0\n" zero padded */
	uint8_t  chipid[8];	/* string "MT7621A" zero padded */
	uint8_t  boardid[16];	/* string "NR7101" zero padded */
	uint32_t modelid;	/* modelid as 4 BCD digits: 0x07010001 */
	uint8_t  pad2[8];	/* zero padding(?) */
	uint8_t  swVersionInt[32];	/* ZyXEL version string: "1.00(ABUV.0)D0" zero padded */
	uint8_t  swVersionExt[32];	/* identical to swVersionInt  */
	uint8_t  pad4[4];	/* zero padding(?) */
	uint32_t kernelChksum;	/* no idea how this is computed - reported but not validated */
	uint8_t  pad5[4];	/* zero padding(?) */
	uint32_t crc32_h;	/* Bit inverted 32-bit CRC of this header payload */
	uint8_t  pad6[4];	/* zero padding(?) */
};

/* static?() field values of unknown meaning - maybe ove to board
 * table when we know the significance
 */
#define VER_INT		"5.0.0.0\n"
#define VER_EXT		"\n"
#define CODE		"3 6035 122 0\n"
#define KERNELCHKSUM	0x12345678

/* table of supported devices using this header format */
static struct board_t {
	uint8_t  chipid[8];
	uint8_t  boardid[16];
	uint32_t modelid;
} boards[] = {
	{ "MT7621A", "NR7101", 0x07010001 },
	{}
};

static int find_board(struct zytrx_t *h, char *board)
{
	struct board_t *p;

	for (p = boards; p->modelid; p++) {
		if (strncmp((const char *)p->boardid, board, sizeof(p->boardid)))
			continue;
		memcpy(h->chipid, p->chipid, sizeof(h->chipid));
		memcpy(h->boardid, p->boardid, sizeof(h->boardid));
		h->modelid = htonl(p->modelid);
		return 0;
	}
	return -1;
}

static void usage(const char *name)
{
	struct board_t *p;

	fprintf(stderr, "Usage:\n");
	fprintf(stderr, " %s -B <board> -v <versionstr> -i <file> [-o <outputfile>]\n\n", name);
	fprintf(stderr, "Supported <board> values:\n");
	for (p = boards; p->modelid; p++)
		fprintf(stderr, "\t%-12s\n", p->boardid);
	fprintf(stderr, "\nExample:\n");
	fprintf(stderr, " %s -B %s -v foobar-1.0 -i my.img -o out.img\n\n", name,
		boards[0].boardid);
	exit(EXIT_FAILURE);
}

static void errexit(const char *msg)
{
	fprintf(stderr, "ERR: %s: %s\n", msg, errno ? strerror(errno) : "unknown");
	exit(EXIT_FAILURE);
}

static void *map_input(const char *name, size_t *len)
{
	struct stat stat;
	void *mapped;
	int fd;

	fd = open(name, O_RDONLY);
	if (fd < 0)
		return NULL;
	if (fstat(fd, &stat) < 0) {
		close(fd);
		return NULL;
	}
	*len = stat.st_size;
	mapped = mmap(NULL, stat.st_size, PROT_READ, MAP_SHARED, fd, 0);
	if (close(fd) < 0) {
		(void) munmap(mapped, stat.st_size);
		return NULL;
	}
	return mapped;
}

int main(int argc, char **argv)
{
	int c, fdout = STDOUT_FILENO;
	void *input_file = NULL;
	size_t file_len, len;
	uint32_t crc;
	struct zytrx_t h = {
		.magic		= htonl(MAGIC),
		.len_h		= htonl(sizeof(h)),
		.verInt		= VER_INT,
		.verExt		= VER_EXT,
		.code		= CODE,
		.kernelChksum	= htonl(KERNELCHKSUM),
	};

	while ((c = getopt(argc, argv, "B:v:i:o:")) != -1) {
		switch (c) {
		case 'B':
			if (find_board(&h, optarg) < 0)
				errexit("unsupported board");
			break;
		case 'v':
			len = strlen(optarg);
			if (len > sizeof(h.swVersionInt))
				errexit("version string too long");
			memcpy(h.swVersionInt, optarg, len);
			memcpy(h.swVersionExt, optarg, len);
			break;
		case 'i':
			input_file = map_input(optarg, &file_len);
			if (!input_file)
				errexit(optarg);
			break;
		case 'o':
			fdout = open(optarg, O_WRONLY | O_CREAT, 0644);
			if (fdout < 0)
				errexit(optarg);
			break;
		default:
			usage(argv[0]);
		}
	}

	/* required paremeters */
	if (!input_file || !h.modelid || !h.swVersionInt[0])
		usage(argv[0]);

	/* length fields */
	h.len_t = htonl(sizeof(h) + file_len);
	h.len_p = htonl(file_len);

	/* crc fields */
	init_crc32();
	crc = crc32buf(input_file, file_len);
	h.crc32_p = htonl(~crc);
	crc = crc32buf((unsigned char *)&h, sizeof(h));
	h.crc32_h = htonl(~crc);

	/* dump new image */
	write(fdout, &h, sizeof(h));
	write(fdout, input_file, file_len);

	/* close files */
	munmap(input_file, file_len);
	if (fdout != STDOUT_FILENO)
		close(fdout);

	return EXIT_SUCCESS;
}

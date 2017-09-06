/*
 * Copyright (C) 2015 Thomas Hebb <tommyhebb@gmail.com>
 *
 * The format of the header this tool generates was first documented by
 * Chris Blake <chrisrblake93 (at) gmail.com> in a shell script of the
 * same purpose. I have created this reimplementation at his request. The
 * original script can be found at:
 * <https://github.com/riptidewave93/meraki-partbuilder>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <libgen.h>
#include <getopt.h>
#include <errno.h>
#include <arpa/inet.h>

#include "sha1.h"

#define PADDING_BYTE		0xff

#define HDR_LENGTH		0x00000400
#define HDR_OFF_MAGIC1		0
#define HDR_OFF_HDRLEN		4
#define HDR_OFF_IMAGELEN	8
#define HDR_OFF_CHECKSUM	12
#define HDR_OFF_MAGIC2		32
#define HDR_OFF_MAGIC3		36
#define HDR_OFF_STATICHASH	40
#define HDR_OFF_KERNEL_OFFSET	40
#define HDR_OFF_RAMDISK_OFFSET	44
#define HDR_OFF_FDT_OFFSET	48
#define HDR_OFF_UNKNOWN_OFFSET	52

struct board_info {
	uint32_t magic1;
	uint32_t magic2;
	uint32_t magic3;
	uint32_t imagelen;
	union {
		unsigned char statichash[20];
		struct {
			uint32_t kernel_offset;
			uint32_t ramdisk_offset;
			uint32_t fdt_offset;
			uint32_t unknown_offset;
		} mx60;
	} u;
	char *id;
	char *description;
};

/*
 * Globals
 */
static char *progname;

static char *board_id;
static const struct board_info *board;

static const struct board_info boards[] = {
	{
		.id		= "mr18",
		.description	= "Meraki MR18 Access Point",
		.magic1		= 0x8e73ed8a,
		.magic2		= 0x8e73ed8a,
		.imagelen	= 0x00800000,
		.u.statichash	= {0xda, 0x39, 0xa3, 0xee, 0x5e,
				   0x6b, 0x4b, 0x0d, 0x32, 0x55,
				   0xbf, 0xef, 0x95, 0x60, 0x18,
				   0x90, 0xaf, 0xd8, 0x07, 0x09},
	}, {
		.id		= "mr24",
		.description	= "Meraki MR24 Access Point",
		.magic1		= 0x8e73ed8a,
		.magic2		= 0x8e73ed8a,
		.imagelen	= 0x00800000,
		.u.statichash	= {0xff, 0xff, 0xff, 0xff, 0xff,
				   0xff, 0xff, 0xff, 0xff, 0xff,
				   0xff, 0xff, 0xff, 0xff, 0xff,
				   0xff, 0xff, 0xff, 0xff, 0xff},
	}, {
		.id		= "mx60",
		.description	= "Meraki MX60/MX60W Security Appliance",
		.magic1		= 0x8e73ed8a,
		.magic2		= 0xa1f0beef, /* Enables use of load addr in statichash */
		.magic3		= 0x00060001, /* This goes along with magic2 */
		.imagelen	= 0x3fd00000,
		/* The static hash below does the following:
		 * 1st Row: Kernel Offset
		 * 2nd Row: Ramdisk Offset
		 * 3rd Row: FDT Offset
		 * 4th Row: ? Unused/Unknown ?
		 * 5th Row: ? Unused/Unknown ?
		 */
		.u.mx60		= {
			.kernel_offset	= 0x10000,
			.ramdisk_offset	= 0x3FFC00,
			.fdt_offset	= 0x0400,
			.unknown_offset	= 0x0400,
		},
	}, {
		/* terminating entry */
	}
};

/*
 * Message macros
 */
#define ERR(fmt, ...) do { \
	fflush(0); \
	fprintf(stderr, "[%s] *** error: " fmt "\n", \
			progname, ## __VA_ARGS__); \
} while (0)

#define ERRS(fmt, ...) do { \
	int save = errno; \
	fflush(0); \
	fprintf(stderr, "[%s] *** error: " fmt "\n", \
			progname, ## __VA_ARGS__, strerror(save)); \
} while (0)

static const struct board_info *find_board(const char *id)
{
	const struct board_info *ret;
	const struct board_info *board;

	ret = NULL;
	for (board = boards; board->id != NULL; board++) {
		if (strcasecmp(id, board->id) == 0) {
			ret = board;
			break;
		}
	}

	return ret;
}

static void usage(int status)
{
	FILE *stream = (status != EXIT_SUCCESS) ? stderr : stdout;
	const struct board_info *board;

	fprintf(stream, "Usage: %s [OPTIONS...]\n", progname);
	fprintf(stream,
"\n"
"Options:\n"
"  -B <board>      create image for the board specified with <board>\n"
"  -i <file>       read kernel image from the file <file>\n"
"  -o <file>       write output to the file <file>\n"
"  -s              strip padding from the end of the image\n"
"  -h              show this screen\n"
	);

	fprintf(stream, "\nBoards:\n");
	for (board = boards; board->id != NULL; board++)
		fprintf(stream, "  %-16s%s\n", board->id, board->description);

	exit(status);
}

void writel(unsigned char *buf, size_t offset, uint32_t value)
{
	value = htonl(value);
	memcpy(buf + offset, &value, sizeof(uint32_t));
}

int main(int argc, char *argv[])
{
	int ret = EXIT_FAILURE;
	long klen;
	size_t kspace;
	unsigned char *kernel;
	size_t buflen;
	unsigned char *buf;
	bool strip_padding = false;
	char *ofname = NULL, *ifname = NULL;
	FILE *out, *in;

	progname = basename(argv[0]);

	while (1) {
		int c;

		c = getopt(argc, argv, "B:i:o:sh");
		if (c == -1)
			break;

		switch (c) {
		case 'B':
			board_id = optarg;
			break;
		case 'i':
			ifname = optarg;
			break;
		case 'o':
			ofname = optarg;
			break;
		case 's':
			strip_padding = true;
			break;
		case 'h':
			usage(EXIT_SUCCESS);
			break;
		default:
			usage(EXIT_FAILURE);
			break;
		}
	}

	if (board_id == NULL) {
		ERR("no board specified");
		goto err;
	}

	board = find_board(board_id);
	if (board == NULL) {
		ERR("unknown board \"%s\"", board_id);
		goto err;
	}

	if (ifname == NULL) {
		ERR("no input file specified");
		goto err;
	}

	if (ofname == NULL) {
		ERR("no output file specified");
		goto err;
	}

	in = fopen(ifname, "r");
	if (in == NULL) {
		ERRS("could not open \"%s\" for reading: %s", ifname);
		goto err;
	}

	buflen = board->imagelen;
	kspace = buflen - HDR_LENGTH;

	/* Get kernel length */
	fseek(in, 0, SEEK_END);
	klen = ftell(in);
	rewind(in);

	if (klen > kspace) {
		ERR("file \"%s\" is too big - max size: 0x%08lX\n",
		    ifname, kspace);
		goto err_close_in;
	}

	/* If requested, resize buffer to remove padding */
	if (strip_padding)
		buflen = klen + HDR_LENGTH;

	/* Allocate and initialize buffer for final image */
	buf = malloc(buflen);
	if (buf == NULL) {
		ERRS("no memory for buffer: %s\n");
		goto err_close_in;
	}
	memset(buf, PADDING_BYTE, buflen);

	/* Load kernel */
	kernel = buf + HDR_LENGTH;
	fread(kernel, klen, 1, in);

	/* Write magic values */
	writel(buf, HDR_OFF_MAGIC1, board->magic1);
	writel(buf, HDR_OFF_MAGIC2, board->magic2);
	writel(buf, HDR_OFF_MAGIC3, board->magic3);

	/* Write header and image length */
	writel(buf, HDR_OFF_HDRLEN, HDR_LENGTH);
	writel(buf, HDR_OFF_IMAGELEN, klen);

	/* Write checksum and static hash */
	sha1_csum(kernel, klen, buf + HDR_OFF_CHECKSUM);

	switch (board->magic2) {
	case 0xa1f0beef:
		writel(buf, HDR_OFF_KERNEL_OFFSET, board->u.mx60.kernel_offset);
		writel(buf, HDR_OFF_RAMDISK_OFFSET, board->u.mx60.ramdisk_offset);
		writel(buf, HDR_OFF_FDT_OFFSET, board->u.mx60.fdt_offset),
		writel(buf, HDR_OFF_UNKNOWN_OFFSET, board->u.mx60.unknown_offset);
		break;

	case 0x8e73ed8a:
		memcpy(buf + HDR_OFF_STATICHASH, board->u.statichash, 20);
		break;
	}

	/* Save finished image */
	out = fopen(ofname, "w");
	if (out == NULL) {
		ERRS("could not open \"%s\" for writing: %s", ofname);
		goto err_free;
	}
	fwrite(buf, buflen, 1, out);

	ret = EXIT_SUCCESS;

	fclose(out);

err_free:
	free(buf);

err_close_in:
	fclose(in);

err:
	return ret;
}

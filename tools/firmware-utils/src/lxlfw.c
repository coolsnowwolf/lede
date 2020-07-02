// SPDX-License-Identifier: GPL-2.0-or-later OR MIT
/*
 * Luxul's firmware container format
 *
 * Copyright 2020 Legrand AV Inc.
 */

#define _GNU_SOURCE

#include <byteswap.h>
#include <endian.h>
#include <errno.h>
#include <libgen.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#if __BYTE_ORDER == __BIG_ENDIAN
#define cpu_to_le32(x)	bswap_32(x)
#define cpu_to_le16(x)	bswap_16(x)
#define le32_to_cpu(x)	bswap_32(x)
#define le16_to_cpu(x)	bswap_16(x)
#elif __BYTE_ORDER == __LITTLE_ENDIAN
#define cpu_to_le32(x)	(x)
#define cpu_to_le16(x)	(x)
#define le32_to_cpu(x)	(x)
#define le16_to_cpu(x)	(x)
#endif

#define min(a, b)				\
	({					\
		__typeof__ (a) _a = (a);	\
		__typeof__ (b) _b = (b);	\
		_a < _b ? _a : _b;		\
	})

#define max(a, b)				\
	({					\
		__typeof__ (a) _a = (a);	\
		__typeof__ (b) _b = (b);	\
		_a > _b ? _a : _b;		\
	})

#define LXL_FLAGS_VENDOR_LUXUL			0x00000001

struct lxl_hdr {
	char		magic[4];	/* "LXL#" */
	uint32_t	version;
	uint32_t	hdr_len;
	uint8_t		v0_end[0];
	/* Version: 1+ */
	uint32_t	flags;
	char		board[16];
	uint8_t		v1_end[0];
	/* Version: 2+ */
	uint8_t		release[4];
	uint8_t		v2_end[0];
} __packed;

static uint32_t lxlfw_hdr_len(uint32_t version)
{
	switch (version) {
	case 0:
		return offsetof(struct lxl_hdr, v0_end);
	case 1:
		return offsetof(struct lxl_hdr, v1_end);
	case 2:
		return offsetof(struct lxl_hdr, v2_end);
	default:
		fprintf(stderr, "Unsupported version %d\n", version);
		return 0;
	}
}

/**************************************************
 * Info
 **************************************************/

static int lxlfw_info(int argc, char **argv) {
	struct lxl_hdr hdr;
	uint32_t version;
	uint32_t hdr_len;
	char board[17];
	size_t bytes;
	int err = 0;
	FILE *lxl;
	int flags;

	if (argc < 3) {
		fprintf(stderr, "Missing <file> argument\n");
		err = -EINVAL;
		goto out;
	}

	lxl = fopen(argv[2], "r");
	if (!lxl) {
		fprintf(stderr, "Could not open \"%s\" file\n", argv[2]);
		err = -ENOENT;
		goto out;
	}

	bytes = fread(&hdr, 1, sizeof(hdr), lxl);
	if (bytes < offsetof(struct lxl_hdr, v0_end)) {
		fprintf(stderr, "Input file too small to use Luxul format\n");
		err = -ENXIO;
		goto err_close;
	}

	if (memcmp(hdr.magic, "LXL#", 4)) {
		fprintf(stderr, "File <file> does not use Luxul's format\n");
		err =  -EINVAL;
		goto err_close;
	}

	version = le32_to_cpu(hdr.version);
	hdr_len = lxlfw_hdr_len(version);
	if (bytes < hdr_len) {
		fprintf(stderr, "Input file too small for header version %d\n", version);
		err = -ENXIO;
		goto err_close;
	}

	printf("Format version:\t%d\n", version);
	printf("Header length:\t%d\n", le32_to_cpu(hdr.hdr_len));
	if (version >= 1) {
		printf("Flags:\t\t");
		flags = le32_to_cpu(hdr.flags);
		if (flags & LXL_FLAGS_VENDOR_LUXUL)
			printf("VENDOR_LUXUL ");
		printf("\n");
		memcpy(board, hdr.board, sizeof(hdr.board));
		board[16] = '\0';
		printf("Board:\t\t%s\n", board);
	}
	if (version >= 2) {
		printf("Release:\t");
		if (hdr.release[0] || hdr.release[1] || hdr.release[2] || hdr.release[3]) {
			printf("%hu.%hu.%hu", hdr.release[0], hdr.release[1], hdr.release[2]);
			if (hdr.release[3])
				printf(".%hu", hdr.release[3]);
		}
		printf("\n");
	}

err_close:
	fclose(lxl);
out:
	return err;
}

/**************************************************
 * Create
 **************************************************/

static int lxlfw_create(int argc, char **argv) {
	struct lxl_hdr hdr = {
		.magic = { 'L', 'X', 'L', '#' },
	};
	char *in_path = NULL;
	uint32_t version = 0;
	uint32_t hdr_len;
	ssize_t bytes;
	char buf[512];
	int err = 0;
	FILE *lxl;
	FILE *in;
	int c;

	if (argc < 3) {
		fprintf(stderr, "Missing <file> argument\n");
		err = -EINVAL;
		goto out;
	}

	optind = 3;
	while ((c = getopt(argc, argv, "i:lb:r:")) != -1) {
		switch (c) {
		case 'i':
			in_path = optarg;
			break;
		case 'l':
			hdr.flags |= cpu_to_le32(LXL_FLAGS_VENDOR_LUXUL);
			version = max(version, 1);
			break;
		case 'b':
			memcpy(hdr.board, optarg, strlen(optarg) > 16 ? 16 : strlen(optarg));
			version = max(version, 1);
			break;
		case 'r':
			if (sscanf(optarg, "%hhu.%hhu.%hhu.%hhu", &hdr.release[0], &hdr.release[1], &hdr.release[2], &hdr.release[3]) < 1) {
				fprintf(stderr, "Failed to parse release number \"%s\"\n", optarg);
				err = -EINVAL;
				goto out;
			}
			version = max(version, 2);
			break;
		}
	}

	hdr.version = cpu_to_le32(version);
	hdr_len = lxlfw_hdr_len(version);
	if (!hdr_len) {
		err = -EINVAL;
		goto out;
	}
	hdr.hdr_len = cpu_to_le32(hdr_len);

	if (!in_path) {
		fprintf(stderr, "Missing input file argument\n");
		err = -EINVAL;
		goto out;
	}

	in = fopen(in_path, "r");
	if (!in) {
		fprintf(stderr, "Could not open input file %s\n", in_path);
		err = -EIO;
		goto out;
	}

	lxl = fopen(argv[2], "w+");
	if (!lxl) {
		fprintf(stderr, "Could not open \"%s\" file\n", argv[2]);
		err = -EIO;
		goto err_close_in;
	}

	bytes = fwrite(&hdr, 1, hdr_len, lxl);
	if (bytes != hdr_len) {
		fprintf(stderr, "Could not write Luxul's header\n");
		err = -EIO;
		goto err_close_lxl;
	}

	while ((bytes = fread(buf, 1, sizeof(buf), in)) > 0) {
		if (fwrite(buf, 1, bytes, lxl) != bytes) {
			fprintf(stderr, "Could not copy %zu bytes from input file\n", bytes);
			err = -EIO;
			goto err_close_lxl;
		}
	}

err_close_lxl:
	fclose(lxl);
err_close_in:
	fclose(in);
out:
	return err;
}

/**************************************************
 * Start
 **************************************************/

static void usage() {
	printf("Usage:\n");
	printf("\n");
	printf("Get info about Luxul firmware:\n");
	printf("\tlxlfw info <file>\n");
	printf("\n");
	printf("Create new Luxul firmware:\n");
	printf("\tlxlfw create <file> [options]\n");
	printf("\t-i file\t\t\t\tinput file for Luxul's firmware container\n");
	printf("\t-l\t\t\t\tmark firmware as created by Luxul company (DON'T USE)\n");
	printf("\t-b board\t\t\tboard (device) name\n");
	printf("\t-r release\t\t\trelease number (e.g. 5.1.0, 7.1.0.2)\n");
}

int main(int argc, char **argv) {
	if (argc > 1) {
		if (!strcmp(argv[1], "info"))
			return lxlfw_info(argc, argv);
		else if (!strcmp(argv[1], "create"))
			return lxlfw_create(argc, argv);
	}

	usage();
	return 0;
}

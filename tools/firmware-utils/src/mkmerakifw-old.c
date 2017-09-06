/*
 * Copyright (C) 2015 Thomas Hebb <tommyhebb@gmail.com>
 * Copyright (C) 2016 Christian Lamparter <chunkeey@googlemail.com>
 *
 * The format of the header this tool generates was first documented by
 * Chris Blake <chrisrblake93 (at) gmail.com> in a shell script of the
 * same purpose. I have created this reimplementation at his request. The
 * original script can be found at:
 * <https://github.com/riptidewave93/meraki-partbuilder>
 *
 * Support for the old header format, which is used by the Cisco Z1 AP
 * has been reverse engineered from the nandloader's nand_load_bk function.
 * The original code is part of Cisco's GPL code and can be found at:
 * <https://github.com/riptidewave93/meraki-linux>
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
#include <endian.h>
#include <getopt.h>
#include <errno.h>
#include <arpa/inet.h>

#define PADDING_BYTE		0xff

#define HDR_LENGTH		0x00000020
#define HDR_OFF_MAGIC1	0
#define HDR_OFF_LOAD_ADDR	4
#define HDR_OFF_IMAGELEN	8
#define HDR_OFF_ENTRY	12
#define HDR_OFF_CHECKSUM	16
#define HDR_OFF_FILLER0	20
#define HDR_OFF_FILLER1	24
#define HDR_OFF_FILLER2	28

struct board_info {
	char *id;
	char *description;
	uint32_t magic;
	uint32_t imagelen;
	uint32_t load_addr;
	uint32_t entry;
};

/*
 * Globals
 */
static char *progname;
static bool strip_padding;

static char *board_id;
static const struct board_info *board;

static const struct board_info boards[] = {
	{
		.id		= "z1",
		.description	= "Meraki Z1 Access Point",
		.magic		= 0x4d495053,
		.imagelen	= 0x007e0000,
		.load_addr	= 0x80060000,
		.entry		= 0x80060000
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

static void writel(unsigned char *buf, size_t offset, uint32_t value)
{
	value = htonl(value);
	memcpy(buf + offset, &value, sizeof(uint32_t));
}

static const uint32_t crc32_table[] = {
	0x00000000, 0x77073096, 0xee0e612c, 0x990951ba,
	0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3,
	0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
	0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91,
	0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de,
	0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
	0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec,
	0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5,
	0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
	0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
	0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940,
	0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
	0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116,
	0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,
	0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
	0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,
	0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a,
	0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
	0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818,
	0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
	0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
	0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457,
	0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c,
	0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
	0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2,
	0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb,
	0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
	0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9,
	0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086,
	0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
	0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4,
	0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad,
	0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
	0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683,
	0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8,
	0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
	0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe,
	0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7,
	0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
	0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
	0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252,
	0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
	0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60,
	0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79,
	0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
	0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f,
	0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04,
	0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
	0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a,
	0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
	0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
	0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21,
	0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e,
	0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
	0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c,
	0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45,
	0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
	0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db,
	0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0,
	0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
	0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6,
	0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf,
	0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
	0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d,
};

static inline uint32_t crc32_accumulate_8(const uint32_t crc, const uint8_t ch)
{
	return crc32_table[(crc ^ ch) & 0xff] ^ (crc >> 8);
}

static void crc32_csum(uint8_t *buf, const size_t len)
{
	uint32_t crc;
	size_t i;

	crc = ~0;
	for (i = 0; i < len; i += 4) {
		crc = crc32_accumulate_8(crc, buf[i + 3]);
		crc = crc32_accumulate_8(crc, buf[i + 2]);
		crc = crc32_accumulate_8(crc, buf[i + 1]);
		crc = crc32_accumulate_8(crc, buf[i]);
	}
	crc = ~crc;

	writel(buf, HDR_OFF_CHECKSUM, crc);
}


static int meraki_build_hdr(const struct board_info *board, const size_t klen,
			    FILE *out, FILE *in)
{
	unsigned char *kernel;
	unsigned char *buf;
	size_t buflen;
	size_t kspace;

	size_t rc;
	buflen = board->imagelen;
	kspace = buflen - HDR_LENGTH;

	if (klen > kspace) {
		ERR("kernel file is too big - max size: 0x%08lX\n", kspace);
		return EXIT_FAILURE;
	}

	/* If requested, resize buffer to remove padding */
	if (strip_padding)
		buflen = klen + HDR_LENGTH;

	/* Allocate and initialize buffer for final image */
	buf = malloc(buflen);
	if (buf == NULL) {
		ERRS("no memory for buffer: %s\n");
		return EXIT_FAILURE;
	}
	memset(buf, PADDING_BYTE, buflen);

	/* Load kernel */
	kernel = buf + HDR_LENGTH;
	fread(kernel, klen, 1, in);

	/* Write magic values and filler */
	writel(buf, HDR_OFF_MAGIC1, board->magic);
	writel(buf, HDR_OFF_FILLER0, 0);
	writel(buf, HDR_OFF_FILLER1, 0);
	writel(buf, HDR_OFF_FILLER2, 0);

	/* Write load and kernel entry point address */
	writel(buf, HDR_OFF_LOAD_ADDR, board->load_addr);
	writel(buf, HDR_OFF_ENTRY, board->entry);

	/* Write header and image length */
	writel(buf, HDR_OFF_IMAGELEN, klen);

	/* this gets replaced later, after the checksum has been calculated */
	writel(buf, HDR_OFF_CHECKSUM, 0);

	/* Write checksum */
	crc32_csum(buf, klen + HDR_LENGTH);

	rc = fwrite(buf, buflen, 1, out);

	free(buf);

	return rc == 1 ? EXIT_SUCCESS : EXIT_FAILURE;
}

int main(int argc, char *argv[])
{
	int ret = EXIT_FAILURE;
	char *ofname = NULL, *ifname = NULL;
	FILE *out, *in;
	size_t klen;

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

	/* Get kernel length */
	fseek(in, 0, SEEK_END);
	klen = ftell(in);
	rewind(in);

	out = fopen(ofname, "w");
	if (out == NULL) {
		ERRS("could not open \"%s\" for writing: %s", ofname);
		goto err_close_in;
	}

	ret = meraki_build_hdr(board, klen, out, in);
	fclose(out);

err_close_in:
	fclose(in);

err:
	return ret;
}

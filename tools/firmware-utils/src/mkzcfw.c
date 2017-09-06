/*
 * Copyright (C) 2010 Gabor Juhos <juhosg@openwrt.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>     /* for unlink() */
#include <libgen.h>
#include <getopt.h>     /* for getopt() */
#include <stdarg.h>
#include <errno.h>
#include <sys/stat.h>

#include "cyg_crc.h"

#if (__BYTE_ORDER == __BIG_ENDIAN)
#  define HOST_TO_BE32(x)	(x)
#  define BE32_TO_HOST(x)	(x)
#  define HOST_TO_LE32(x)	bswap_32(x)
#  define LE32_TO_HOST(x)	bswap_32(x)
#else
#  define HOST_TO_BE32(x)	bswap_32(x)
#  define BE32_TO_HOST(x)	bswap_32(x)
#  define HOST_TO_LE32(x)	(x)
#  define LE32_TO_HOST(x)	(x)
#endif

#define MAGIC_FIRMWARE	0x6d726966	/* 'firm' */
#define MAGIC_KERNEL	0x676d694b	/* 'Kimg' */
#define MAGIC_ROOTFS	0x676d6952	/* 'Rimg' */

struct file_info {
	char		*file_name;	/* name of the file */
	uint32_t	file_size;	/* length of the file */
};

struct fw_header {
	uint32_t	magic;
	uint32_t	length;
	uint32_t	unk1;
	uint32_t	unk2;
} __attribute__ ((packed));

struct fw_tail {
	uint32_t	hw_id;
	uint32_t	crc;
} __attribute__ ((packed));

struct board_info {
	char		*id;
	uint32_t	hw_id;
	uint32_t	kernel_len;
	uint32_t	rootfs_len;
};

/*
 * Globals
 */
static char *ofname;
static char *progname;

static char *board_id;
static struct board_info *board;
static struct file_info kernel_info;
static struct file_info rootfs_info;


static struct board_info boards[] = {
	{
		.id		= "ZCN-1523H-2-8",
		.hw_id		= 0x66661523,
		.kernel_len	= 0x170000,
		.rootfs_len	= 0x610000,
	}, {
		.id		= "ZCN-1523H-5-16",
		.hw_id		= 0x6615235A,
		.kernel_len	= 0x170000,
		.rootfs_len	= 0x610000,
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
			progname, ## __VA_ARGS__ ); \
} while (0)

#define ERRS(fmt, ...) do { \
	int save = errno; \
	fflush(0); \
	fprintf(stderr, "[%s] *** error: " fmt ": %s\n", \
			progname, ## __VA_ARGS__, strerror(save)); \
} while (0)

#define DBG(fmt, ...) do { \
	fprintf(stderr, "[%s] " fmt "\n", progname, ## __VA_ARGS__ ); \
} while (0)

static struct board_info *find_board(char *id)
{
	struct board_info *ret;
	struct board_info *board;

	ret = NULL;
	for (board = boards; board->id != NULL; board++){
		if (strcasecmp(id, board->id) == 0) {
			ret = board;
			break;
		}
	};

	return ret;
}

static void usage(int status)
{
	FILE *stream = (status != EXIT_SUCCESS) ? stderr : stdout;
	struct board_info *board;

	fprintf(stream, "Usage: %s [OPTIONS...]\n", progname);
	fprintf(stream,
"\n"
"Options:\n"
"  -B <board>      create image for the board specified with <board>\n"
"  -k <file>       read kernel image from the file <file>\n"
"  -r <file>       read rootfs image from the file <file>\n"
"  -o <file>       write output to the file <file>\n"
"  -h              show this screen\n"
	);

	exit(status);
}

static int get_file_stat(struct file_info *fdata)
{
	struct stat st;
	int res;

	if (fdata->file_name == NULL)
		return 0;

	res = stat(fdata->file_name, &st);
	if (res){
		ERRS("stat failed on %s", fdata->file_name);
		return res;
	}

	fdata->file_size = st.st_size;
	return 0;
}

static int read_to_buf(struct file_info *fdata, char *buf)
{
	FILE *f;
	int ret = EXIT_FAILURE;

	f = fopen(fdata->file_name, "r");
	if (f == NULL) {
		ERRS("could not open \"%s\" for reading", fdata->file_name);
		goto out;
	}

	errno = 0;
	fread(buf, fdata->file_size, 1, f);
	if (errno != 0) {
		ERRS("unable to read from file \"%s\"", fdata->file_name);
		goto out_close;
	}

	ret = EXIT_SUCCESS;

 out_close:
	fclose(f);
 out:
	return ret;
}

static int check_options(void)
{
	int ret;

	if (board_id == NULL) {
		ERR("no board specified");
		return -1;
	}

	board = find_board(board_id);
	if (board == NULL) {
		ERR("unknown/unsupported board id \"%s\"", board_id);
		return -1;
	}

	if (kernel_info.file_name == NULL) {
		ERR("no kernel image specified");
		return -1;
	}

	ret = get_file_stat(&kernel_info);
	if (ret)
		return ret;

	if (kernel_info.file_size > board->kernel_len) {
		ERR("kernel image is too big");
		return -1;
	}

	if (rootfs_info.file_name == NULL) {
		ERR("no rootfs image specified");
		return -1;
	}

	ret = get_file_stat(&rootfs_info);
	if (ret)
		return ret;

	if (rootfs_info.file_size > board->rootfs_len) {
		ERR("rootfs image is too big");
		return -1;
	}

	if (ofname == NULL) {
		ERR("no output file specified");
		return -1;
	}

	return 0;
}

static int write_fw(char *data, int len)
{
	FILE *f;
	int ret = EXIT_FAILURE;

	f = fopen(ofname, "w");
	if (f == NULL) {
		ERRS("could not open \"%s\" for writing", ofname);
		goto out;
	}

	errno = 0;
	fwrite(data, len, 1, f);
	if (errno) {
		ERRS("unable to write output file");
		goto out_flush;
	}

	DBG("firmware file \"%s\" completed", ofname);

	ret = EXIT_SUCCESS;

 out_flush:
	fflush(f);
	fclose(f);
	if (ret != EXIT_SUCCESS) {
		unlink(ofname);
	}
 out:
	return ret;
}

static int build_fw(void)
{
	int buflen;
	char *buf;
	char *p;
	int ret = EXIT_FAILURE;
	int writelen = 0;
	uint32_t crc;
	struct fw_header *hdr;
	struct fw_tail *tail;

	buflen = 3 * sizeof(struct fw_header) +
		 kernel_info.file_size + rootfs_info.file_size +
		 3 * sizeof(struct fw_tail);

	buf = malloc(buflen);
	if (!buf) {
		ERR("no memory for buffer\n");
		goto out;
	}

	p = buf;
	memset(p, 0, buflen);

	/* fill firmware header */
	hdr = (struct fw_header *) p;
	hdr->magic = HOST_TO_LE32(MAGIC_FIRMWARE);
	hdr->length = HOST_TO_LE32(buflen - sizeof(struct fw_header));
	p += sizeof(struct fw_header);

	/* fill kernel block header */
	hdr = (struct fw_header *) p;
	hdr->magic = HOST_TO_LE32(MAGIC_KERNEL);
	hdr->length = HOST_TO_LE32(kernel_info.file_size +
				   sizeof(struct fw_tail));
	p += sizeof(struct fw_header);

	/* read kernel data */
	ret = read_to_buf(&kernel_info, p);
	if (ret)
		goto out_free_buf;

	/* fill firmware tail */
	tail = (struct fw_tail *) (p + kernel_info.file_size);
	tail->hw_id = HOST_TO_BE32(board->hw_id);
	tail->crc = HOST_TO_BE32(cyg_crc32(p, kernel_info.file_size +
				 	   sizeof(struct fw_tail) - 4));

	p += kernel_info.file_size + sizeof(struct fw_tail);

	/* fill rootfs block header */
	hdr = (struct fw_header *) p;
	hdr->magic = HOST_TO_LE32(MAGIC_ROOTFS);
	hdr->length = HOST_TO_LE32(rootfs_info.file_size +
				   sizeof(struct fw_tail));
	p += sizeof(struct fw_header);

	/* read rootfs data */
	ret = read_to_buf(&rootfs_info, p);
	if (ret)
		goto out_free_buf;

	/* fill firmware tail */
	tail = (struct fw_tail *) (p + rootfs_info.file_size);
	tail->hw_id = HOST_TO_BE32(board->hw_id);
	tail->crc = HOST_TO_BE32(cyg_crc32(p, rootfs_info.file_size +
				 	   sizeof(struct fw_tail) - 4));

	p += rootfs_info.file_size + sizeof(struct fw_tail);

	/* fill firmware tail */
	tail = (struct fw_tail *) p;
	tail->hw_id = HOST_TO_BE32(board->hw_id);
	tail->crc = HOST_TO_BE32(cyg_crc32(buf + sizeof(struct fw_header),
				 buflen - sizeof(struct fw_header) - 4));

	ret = write_fw(buf, buflen);
	if (ret)
		goto out_free_buf;

	ret = EXIT_SUCCESS;

 out_free_buf:
	free(buf);
 out:
	return ret;
}

int main(int argc, char *argv[])
{
	int ret = EXIT_FAILURE;
	int err;

	FILE *outfile;

	progname = basename(argv[0]);

	while ( 1 ) {
		int c;

		c = getopt(argc, argv, "B:k:r:o:h");
		if (c == -1)
			break;

		switch (c) {
		case 'B':
			board_id = optarg;
			break;
		case 'k':
			kernel_info.file_name = optarg;
			break;
		case 'r':
			rootfs_info.file_name = optarg;
			break;
		case 'o':
			ofname = optarg;
			break;
		case 'h':
			usage(EXIT_SUCCESS);
			break;
		default:
			usage(EXIT_FAILURE);
			break;
		}
	}

	ret = check_options();
	if (ret)
		goto out;

	ret = build_fw();

 out:
	return ret;
}


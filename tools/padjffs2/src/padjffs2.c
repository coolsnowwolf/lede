/*
 * Copyright (C) 2011 Gabor Juhos <juhosg@openwrt.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 *
 */

#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>

static char *progname;
static unsigned int xtra_offset;
static unsigned char eof_mark[4] = {0xde, 0xad, 0xc0, 0xde};
static unsigned char jffs2_pad_be[] = "\x19\x85\x20\x04\x04\x00\x00\x00\xc4\x94\xdb\xf4";
static unsigned char jffs2_pad_le[] = "\x85\x19\x04\x20\x00\x00\x00\x04\xa8\xfb\xa0\xb4";
static unsigned char *pad = eof_mark;
static int pad_len = sizeof(eof_mark);
static bool pad_to_stdout = false;

#define ERR(fmt, ...) do { \
	fflush(0); \
	fprintf(stderr, "[%s] *** error: " fmt "\n", \
			progname, ## __VA_ARGS__ ); \
} while (0)

#define ERRS(fmt, ...) do { \
	int save = errno; \
	fflush(0); \
	fprintf(stderr, "[%s] *** error: " fmt ", %s\n", \
			progname, ## __VA_ARGS__, strerror(save)); \
} while (0)

#define BUF_SIZE	(64 * 1024)
#define ALIGN(_x,_y)	(((_x) + ((_y) - 1)) & ~((_y) - 1))

static int pad_image(char *name, uint32_t pad_mask)
{
	char *buf;
	int fd;
	int outfd;
	ssize_t in_len;
	ssize_t out_len;
	int ret = -1;

	buf = malloc(BUF_SIZE);
	if (!buf) {
		ERR("No memory for buffer");
		goto out;
	}

	fd = open(name, O_RDWR);
	if (fd < 0) {
		ERRS("Unable to open %s", name);
		goto free_buf;
	}

	in_len = lseek(fd, 0, SEEK_END);
	if (in_len < 0)
		goto close;

	if (!pad_to_stdout)
		outfd = fd;
	else
		outfd = STDOUT_FILENO;

	memset(buf, '\xff', BUF_SIZE);

	in_len += xtra_offset;

	out_len = in_len;
	while (pad_mask) {
		uint32_t mask;
		ssize_t t;
		int i;

		for (i = 10; i < 32; i++) {
			mask = 1UL << i;
			if (pad_mask & mask)
				break;
		}

		in_len = ALIGN(in_len, mask);

		for (i = 10; i < 32; i++) {
			mask = 1UL << i;
			if ((in_len & (mask - 1)) == 0)
				pad_mask &= ~mask;
		}

		fprintf(stderr, "padding image to %08x\n", (unsigned int) in_len - xtra_offset);

		while (out_len < in_len) {
			ssize_t len;

			len = in_len - out_len;
			if (len > BUF_SIZE)
				len = BUF_SIZE;

			t = write(outfd, buf, len);
			if (t != len) {
				ERRS("Unable to write to %s", name);
				goto close;
			}

			out_len += len;
		}

		/* write out the JFFS end-of-filesystem marker */
		t = write(outfd, pad, pad_len);
		if (t != pad_len) {
			ERRS("Unable to write to %s", name);
			goto close;
		}
		out_len += pad_len;
	}

	ret = 0;

close:
	close(fd);
free_buf:
	free(buf);
out:
	return ret;
}

static int usage(void)
{
	fprintf(stderr,
		"Usage: %s file [<options>] [pad0] [pad1] [padN]\n"
		"Options:\n"
		"  -x <offset>:          Add an extra offset for padding data\n"
		"  -J:                   Use a fake big-endian jffs2 padding element instead of EOF\n"
		"                        This is used to work around broken boot loaders that\n"
		"                        try to parse the entire firmware area as one big jffs2\n"
		"  -j:                   (like -J, but little-endian instead of big-endian)\n"
		"  -c:                   write padding to stdout\n"
		"\n",
		progname);
	return EXIT_FAILURE;
}

int main(int argc, char* argv[])
{
	char *image;
	uint32_t pad_mask;
	int ret = EXIT_FAILURE;
	int err;
	int ch, i;

	progname = basename(argv[0]);

	if (argc < 2)
		return usage();

	image = argv[1];
	argv++;
	argc--;

	pad_mask = 0;
	while ((ch = getopt(argc, argv, "x:Jjc")) != -1) {
		switch (ch) {
		case 'x':
			xtra_offset = strtoul(optarg, NULL, 0);
			fprintf(stderr, "assuming %u bytes offset\n",
				xtra_offset);
			break;
		case 'J':
			pad = jffs2_pad_be;
			pad_len = sizeof(jffs2_pad_be) - 1;
			break;
		case 'j':
			pad = jffs2_pad_le;
			pad_len = sizeof(jffs2_pad_le) - 1;
			break;
		case 'c':
			pad_to_stdout = true;
			break;
		default:
			return usage();
		}
	}

	for (i = optind; i < argc; i++)
		pad_mask |= strtoul(argv[i], NULL, 0) * 1024;

	if (pad_mask == 0)
		pad_mask = (4 * 1024) | (8 * 1024) | (64 * 1024) |
			   (128 * 1024);

	err = pad_image(image, pad_mask);
	if (err)
		goto out;

	ret = EXIT_SUCCESS;

out:
	return ret;
}

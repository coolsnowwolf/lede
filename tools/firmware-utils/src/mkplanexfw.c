/*
 *  Copyright (C) 2009 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
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

#include "sha1.h"

#if (__BYTE_ORDER == __BIG_ENDIAN)
#  define HOST_TO_BE32(x)	(x)
#  define BE32_TO_HOST(x)	(x)
#else
#  define HOST_TO_BE32(x)	bswap_32(x)
#  define BE32_TO_HOST(x)	bswap_32(x)
#endif


struct planex_hdr {
	uint8_t		sha1sum[20];
	char		version[8];
	uint8_t		unk1[2];
	uint32_t	datalen;
} __attribute__ ((packed));

struct board_info {
	char		*id;
	uint32_t	seed;
	uint8_t		unk[2];
	uint32_t	datalen;
};

/*
 * Globals
 */
static char *ifname;
static char *progname;
static char *ofname;
static char *version = "1.00.00";

static char *board_id;
static struct board_info *board;

static struct board_info boards[] = {
	{
		.id		= "MZK-W04NU",
		.seed		= 2,
		.unk		= {0x04, 0x08},
		.datalen	= 0x770000,
	}, {
		.id		= "MZK-W300NH",
		.seed		= 4,
		.unk		= {0x00, 0x00},
		.datalen	= 0x770000,
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

void usage(int status)
{
	FILE *stream = (status != EXIT_SUCCESS) ? stderr : stdout;

	fprintf(stream, "Usage: %s [OPTIONS...]\n", progname);
	fprintf(stream,
"\n"
"Options:\n"
"  -B <board>      create image for the board specified with <board>\n"
"  -i <file>       read input from the file <file>\n"
"  -o <file>       write output to the file <file>\n"
"  -v <version>    set image version to <version>\n"
"  -h              show this screen\n"
	);

	exit(status);
}

int main(int argc, char *argv[])
{
	int res = EXIT_FAILURE;
	int buflen;
	int err;
	struct stat st;
	char *buf;
	struct planex_hdr *hdr;
	sha1_context ctx;
	uint32_t seed;

	FILE *outfile, *infile;

	progname = basename(argv[0]);

	while ( 1 ) {
		int c;

		c = getopt(argc, argv, "B:i:o:v:h");
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
		case 'v':
			version = optarg;
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
		ERR("unknown board '%s'", board_id);
		goto err;
	};

	if (ifname == NULL) {
		ERR("no input file specified");
		goto err;
	}

	if (ofname == NULL) {
		ERR("no output file specified");
		goto err;
	}

	err = stat(ifname, &st);
	if (err){
		ERRS("stat failed on %s", ifname);
		goto err;
	}

	if (st.st_size > board->datalen) {
		ERR("file '%s' is too big - max size: 0x%08X (exceeds %lu bytes)\n",
		    ifname, board->datalen, st.st_size - board->datalen);
		goto err;
	}

	buflen = board->datalen + 0x10000;
	buf = malloc(buflen);
	if (!buf) {
		ERR("no memory for buffer\n");
		goto err;
	}

	memset(buf, 0xff, buflen);
	hdr = (struct planex_hdr *)buf;

	hdr->datalen = HOST_TO_BE32(board->datalen);
	hdr->unk1[0] = board->unk[0];
	hdr->unk1[1] = board->unk[1];

	snprintf(hdr->version, sizeof(hdr->version), "%s", version);

	infile = fopen(ifname, "r");
	if (infile == NULL) {
		ERRS("could not open \"%s\" for reading", ifname);
		goto err_free;
	}

	errno = 0;
	fread(buf +  sizeof(*hdr), st.st_size, 1, infile);
	if (errno != 0) {
		ERRS("unable to read from file %s", ifname);
		goto err_close_in;
	}

	seed = HOST_TO_BE32(board->seed);
	sha1_starts(&ctx);
	sha1_update(&ctx, (uchar *) &seed, sizeof(seed));
	sha1_update(&ctx, buf + sizeof(*hdr), board->datalen);
	sha1_finish(&ctx, hdr->sha1sum);

	outfile = fopen(ofname, "w");
	if (outfile == NULL) {
		ERRS("could not open \"%s\" for writing", ofname);
		goto err_close_in;
	}

	errno = 0;
	fwrite(buf, buflen, 1, outfile);
	if (errno) {
		ERRS("unable to write to file %s", ofname);
		goto err_close_out;
	}

	res = EXIT_SUCCESS;

	fflush(outfile);

 err_close_out:
	fclose(outfile);
	if (res != EXIT_SUCCESS) {
		unlink(ofname);
	}

 err_close_in:
	fclose(infile);

 err_free:
	free(buf);

 err:
	return res;
}


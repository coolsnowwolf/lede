/*
 * A tool for reading the zlib compressed calibration data
 * found in AVM Fritz!Box based devices).
 *
 * Copyright (c) 2017 Christian Lamparter <chunkeey@googlemail.com>
 *
 * Based on zpipe, which is an example of proper use of zlib's inflate().
 * that is Not copyrighted -- provided to the public domain
 * Version 1.4  11 December 2005  Mark Adler
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <endian.h>
#include <errno.h>
#include "zlib.h"

#define CHUNK 1024

static inline size_t special_min(size_t a, size_t b)
{
	return a == 0 ? b : (a < b ? a : b);
}

/* Decompress from file source to file dest until stream ends or EOF.
   inf() returns Z_OK on success, Z_MEM_ERROR if memory could not be
   allocated for processing, Z_DATA_ERROR if the deflate data is
   invalid or incomplete, Z_VERSION_ERROR if the version of zlib.h and
   the version of the library linked do not match, or Z_ERRNO if there
   is an error reading or writing the files. */
static int inf(FILE *source, FILE *dest, size_t limit, size_t skip)
{
    int ret;
    size_t have;
    z_stream strm;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];

    /* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit(&strm);
    if (ret != Z_OK)
        return ret;

    /* decompress until deflate stream ends or end of file */
    do {
        strm.avail_in = fread(in, 1, CHUNK, source);
        if (ferror(source)) {
            (void)inflateEnd(&strm);
            return Z_ERRNO;
        }
        if (strm.avail_in == 0)
            break;
        strm.next_in = in;

        /* run inflate() on input until output buffer not full */
        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = inflate(&strm, Z_NO_FLUSH);
            assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
            switch (ret) {
            case Z_NEED_DICT:
                ret = Z_DATA_ERROR;     /* and fall through */
            case Z_DATA_ERROR:
            case Z_MEM_ERROR:
                (void)inflateEnd(&strm);
                return ret;
            }
            have = special_min(limit, CHUNK - strm.avail_out) - skip;
            if (fwrite(&out[skip], have, 1, dest) != 1 || ferror(dest)) {
                (void)inflateEnd(&strm);
                return Z_ERRNO;
            }
	    skip = 0;
	    limit -= have;
        } while (strm.avail_out == 0 && limit > 0);

        /* done when inflate() says it's done */
    } while (ret != Z_STREAM_END && limit > 0);

    /* clean up and return */
    (void)inflateEnd(&strm);
    return (limit == 0 ? Z_OK : (ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR));
}

/* report a zlib or i/o error */
static void zerr(int ret)
{
    switch (ret) {
    case Z_ERRNO:
        if (ferror(stdin))
            fputs("error reading stdin\n", stderr);
        if (ferror(stdout))
            fputs("error writing stdout\n", stderr);
        break;
    case Z_STREAM_ERROR:
        fputs("invalid compression level\n", stderr);
        break;
    case Z_DATA_ERROR:
        fputs("invalid or incomplete deflate data\n", stderr);
        break;
    case Z_MEM_ERROR:
        fputs("out of memory\n", stderr);
        break;
    case Z_VERSION_ERROR:
        fputs("zlib version mismatch!\n", stderr);
    }
}

static unsigned int get_num(char *str)
{
	if (!strncmp("0x", str, 2))
		return strtoul(str+2, NULL, 16);
	else
		return strtoul(str, NULL, 10);
}

static void usage(void)
{
	fprintf(stderr, "Usage: fritz_cal_extract [-s seek offset] [-i skip] [-o output file] [-l limit] [infile] -e entry_id\n"
			"Finds and extracts zlib compressed calibration data in the EVA loader\n");
	exit(EXIT_FAILURE);
}

struct cal_entry {
	uint16_t id;
	uint16_t len;
} __attribute__((packed));

/* compress or decompress from stdin to stdout */
int main(int argc, char **argv)
{
	struct cal_entry cal = { .len = 0 };
	FILE *in = stdin;
	FILE *out = stdout;
	size_t limit = 0, skip = 0;
	int initial_offset = 0;
	int entry = -1;
	int ret;
	int opt;

	while ((opt = getopt(argc, argv, "s:e:o:l:i:")) != -1) {
		switch (opt) {
		case 's':
			initial_offset = (int)get_num(optarg);
			if (errno) {
				perror("Failed to parse seek offset");
				goto out_bad;
			}
			break;
		case 'e':
			entry = (int) htobe16(get_num(optarg));
			if (errno) {
				perror("Failed to entry id");
				goto out_bad;
			}
			break;
		case 'o':
			out = fopen(optarg, "w");
			if (!out) {
				perror("Failed to create output file");
				goto out_bad;
			}
			break;
		case 'l':
			limit = (size_t)get_num(optarg);
			if (errno) {
				perror("Failed to parse limit");
				goto out_bad;
			}
			break;
		case 'i':
			skip = (size_t)get_num(optarg);
			if (errno) {
				perror("Failed to parse skip");
				goto out_bad;
			}
			break;
		default: /* '?' */
			usage();
		}
	}

	if (entry == -1)
		usage();

	if (argc > 1 && optind <= argc) {
		in = fopen(argv[optind], "r");
		if (!in) {
			perror("Failed to create output file");
			goto out_bad;
		}
	}

	if (initial_offset) {
		ret = fseek(in, initial_offset, SEEK_CUR);
		if (ret) {
			perror("Failed to seek to calibration table");
			goto out_bad;
		}
	}

	do {
		ret = fseek(in, be16toh(cal.len), SEEK_CUR);
		if (feof(in)) {
			fprintf(stderr, "Reached end of file, but didn't find the matching entry\n");
			goto out_bad;
		} else if (ferror(in)) {
			perror("Failure during seek");
			goto out_bad;
		}

		ret = fread(&cal, 1, sizeof cal, in);
		if (ret != sizeof cal)
			goto out_bad;
	} while (entry != cal.id || cal.id == 0xffff);

	if (cal.id == 0xffff) {
		fprintf(stderr, "Reached end of filesystem, but didn't find the matching entry\n");
		goto out_bad;
	}

	ret = inf(in, out, limit, skip);
	if (ret == Z_OK)
		goto out;

	zerr(ret);

out_bad:
	ret = EXIT_FAILURE;

out:
	fclose(in);
	fclose(out);
	return ret;
}

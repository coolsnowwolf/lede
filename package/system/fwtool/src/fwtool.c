/*
 * Copyright (C) 2016 Felix Fietkau <nbd@nbd.name>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#include <sys/types.h>
#include <stdio.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "fwimage.h"
#include "utils.h"
#include "crc32.h"

#define METADATA_MAXLEN		30 * 1024
#define SIGNATURE_MAXLEN	1 * 1024

#define BUFLEN			(METADATA_MAXLEN + SIGNATURE_MAXLEN + 1024)

enum {
	MODE_DEFAULT = -1,
	MODE_EXTRACT = 0,
	MODE_APPEND = 1,
};

struct data_buf {
	char *cur;
	char *prev;
	int cur_len;
	int file_len;
};

static FILE *signature_file, *metadata_file, *firmware_file;
static int file_mode = MODE_DEFAULT;
static bool truncate_file;
static bool quiet = false;

static uint32_t crc_table[256];

#define msg(...)					\
	do {						\
		if (!quiet)				\
			fprintf(stderr, __VA_ARGS__);	\
	} while (0)

static int
usage(const char *progname)
{
	fprintf(stderr, "Usage: %s <options> <firmware>\n"
		"\n"
		"Options:\n"
		"  -S <file>:		Append signature file to firmware image\n"
		"  -I <file>:		Append metadata file to firmware image\n"
		"  -s <file>:		Extract signature file from firmware image\n"
		"  -i <file>:		Extract metadata file from firmware image\n"
		"  -t:			Remove extracted chunks from firmare image (using -s, -i)\n"
		"  -q:			Quiet (suppress error messages)\n"
		"\n", progname);
	return 1;
}

static FILE *
open_file(const char *name, bool write)
{
	FILE *ret;

	if (!strcmp(name, "-"))
		return write ? stdout : stdin;

	ret = fopen(name, write ? "w" : "r+");
	if (!ret && !write)
		ret = fopen(name, "r");

	return ret;
}

static int
set_file(FILE **file, const char *name, int mode)
{
	if (file_mode < 0)
		file_mode = mode;
	else if (file_mode != mode) {
		msg("Error: mixing appending and extracting data is not supported\n");
		return 1;
	}

	if (*file) {
		msg("Error: the same append/extract option cannot be used multiple times\n");
		return 1;
	}

	*file = open_file(name, mode == MODE_EXTRACT);
	return !*file;
}

static void
trailer_update_crc(struct fwimage_trailer *tr, void *buf, int len)
{
	tr->crc32 = cpu_to_be32(crc32_block(be32_to_cpu(tr->crc32), buf, len, crc_table));
}

static int
append_data(FILE *in, FILE *out, struct fwimage_trailer *tr, int maxlen)
{
	while (1) {
		char buf[512];
		int len;

		len = fread(buf, 1, sizeof(buf), in);
		if (!len)
			break;

		maxlen -= len;
		if (maxlen < 0)
			return 1;

		tr->size += len;
		trailer_update_crc(tr, buf, len);
		fwrite(buf, len, 1, out);
	}

	return 0;
}

static void
append_trailer(FILE *out, struct fwimage_trailer *tr)
{
	tr->size = cpu_to_be32(tr->size);
	fwrite(tr, sizeof(*tr), 1, out);
	trailer_update_crc(tr, tr, sizeof(*tr));
}

static int
add_metadata(struct fwimage_trailer *tr)
{
	struct fwimage_header hdr = {};

	tr->type = FWIMAGE_INFO;
	tr->size = sizeof(hdr) + sizeof(*tr);

	trailer_update_crc(tr, &hdr, sizeof(hdr));
	fwrite(&hdr, sizeof(hdr), 1, firmware_file);

	if (append_data(metadata_file, firmware_file, tr, METADATA_MAXLEN))
		return 1;

	append_trailer(firmware_file, tr);

	return 0;
}

static int
add_signature(struct fwimage_trailer *tr)
{
	if (!signature_file)
		return 0;

	tr->type = FWIMAGE_SIGNATURE;
	tr->size = sizeof(*tr);

	if (append_data(signature_file, firmware_file, tr, SIGNATURE_MAXLEN))
		return 1;

	append_trailer(firmware_file, tr);

	return 0;
}

static int
add_data(const char *name)
{
	struct fwimage_trailer tr = {
		.magic = cpu_to_be32(FWIMAGE_MAGIC),
		.crc32 = ~0,
	};
	int file_len = 0;
	int ret = 0;

	firmware_file = fopen(name, "r+");
	if (!firmware_file) {
		msg("Failed to open firmware file\n");
		return 1;
	}

	while (1) {
		char buf[512];
		int len;

		len = fread(buf, 1, sizeof(buf), firmware_file);
		if (!len)
			break;

		file_len += len;
		trailer_update_crc(&tr, buf, len);
	}

	if (metadata_file)
		ret = add_metadata(&tr);
	else if (signature_file)
		ret = add_signature(&tr);

	if (ret) {
		fflush(firmware_file);
		ftruncate(fileno(firmware_file), file_len);
	}

	return ret;
}

static void
remove_tail(struct data_buf *dbuf, int len)
{
	dbuf->cur_len -= len;
	dbuf->file_len -= len;

	if (dbuf->cur_len)
		return;

	free(dbuf->cur);
	dbuf->cur = dbuf->prev;
	dbuf->prev = NULL;
	dbuf->cur_len = BUFLEN;
}

static int
extract_tail(struct data_buf *dbuf, void *dest, int len)
{
	int cur_len = dbuf->cur_len;

	if (!dbuf->cur)
		return 1;

	if (cur_len >= len)
		cur_len = len;

	memcpy(dest + (len - cur_len), dbuf->cur + dbuf->cur_len - cur_len, cur_len);
	remove_tail(dbuf, cur_len);

	cur_len = len - cur_len;
	if (cur_len && !dbuf->cur)
		return 1;

	memcpy(dest, dbuf->cur + dbuf->cur_len - cur_len, cur_len);
	remove_tail(dbuf, cur_len);

	return 0;
}

static uint32_t
tail_crc32(struct data_buf *dbuf, uint32_t crc32)
{
	if (dbuf->prev)
		crc32 = crc32_block(crc32, dbuf->prev, BUFLEN, crc_table);

	return crc32_block(crc32, dbuf->cur, dbuf->cur_len, crc_table);
}

static int
validate_metadata(struct fwimage_header *hdr, int data_len)
{
	 if (hdr->version != 0)
		 return 1;
	 return 0;
}

static int
extract_data(const char *name)
{
	struct fwimage_header *hdr;
	struct fwimage_trailer tr;
	struct data_buf dbuf = {};
	uint32_t crc32 = ~0;
	int ret = 1;
	void *buf;

	firmware_file = open_file(name, false);
	if (!firmware_file) {
		msg("Failed to open firmware file\n");
		return 1;
	}

	if (truncate_file && firmware_file == stdin) {
		msg("Cannot truncate file when reading from stdin\n");
		return 1;
	}

	buf = malloc(BUFLEN);
	if (!buf)
		return 1;

	do {
		char *tmp = dbuf.cur;

		dbuf.cur = dbuf.prev;
		dbuf.prev = tmp;

		if (dbuf.cur)
			crc32 = crc32_block(crc32, dbuf.cur, BUFLEN, crc_table);
		else
			dbuf.cur = malloc(BUFLEN);

		if (!dbuf.cur)
			goto out;

		dbuf.cur_len = fread(dbuf.cur, 1, BUFLEN, firmware_file);
		dbuf.file_len += dbuf.cur_len;
	} while (dbuf.cur_len == BUFLEN);

	while (1) {
		int data_len;

		if (extract_tail(&dbuf, &tr, sizeof(tr)))
			break;

		data_len = be32_to_cpu(tr.size) - sizeof(tr);
		if (tr.magic != cpu_to_be32(FWIMAGE_MAGIC)) {
			msg("Data not found\n");
			break;
		}

		if (be32_to_cpu(tr.crc32) != tail_crc32(&dbuf, crc32)) {
			msg("CRC error\n");
			break;
		}

		if (data_len > BUFLEN) {
			msg("Size error\n");
			break;
		}

		extract_tail(&dbuf, buf, data_len);

		if (tr.type == FWIMAGE_SIGNATURE) {
			if (!signature_file)
				continue;
			fwrite(buf, data_len, 1, signature_file);
			ret = 0;
			break;
		} else if (tr.type == FWIMAGE_INFO) {
			if (!metadata_file)
				break;

			hdr = buf;
			data_len -= sizeof(*hdr);
			if (validate_metadata(hdr, data_len))
				continue;

			fwrite(hdr + 1, data_len, 1, metadata_file);
			ret = 0;
			break;
		} else {
			continue;
		}
	}

	if (!ret && truncate_file)
		ftruncate(fileno(firmware_file), dbuf.file_len);

out:
	free(buf);
	free(dbuf.cur);
	free(dbuf.prev);
	return ret;
}

static void cleanup(void)
{
	if (signature_file)
		fclose(signature_file);
	if (metadata_file)
		fclose(metadata_file);
	if (firmware_file)
		fclose(firmware_file);
}

int main(int argc, char **argv)
{
	const char *progname = argv[0];
	int ret, ch;

	crc32_filltable(crc_table);

	while ((ch = getopt(argc, argv, "i:I:qs:S:t")) != -1) {
		ret = 0;
		switch(ch) {
		case 'S':
			ret = set_file(&signature_file, optarg, MODE_APPEND);
			break;
		case 'I':
			ret = set_file(&metadata_file, optarg, MODE_APPEND);
			break;
		case 's':
			ret = set_file(&signature_file, optarg, MODE_EXTRACT);
			break;
		case 'i':
			ret = set_file(&metadata_file, optarg, MODE_EXTRACT);
			break;
		case 't':
			truncate_file = true;
			break;
		case 'q':
			quiet = true;
			break;
		}

		if (ret)
			goto out;
	}

	if (optind >= argc) {
		ret = usage(progname);
		goto out;
	}

	if (file_mode == MODE_DEFAULT) {
		ret = usage(progname);
		goto out;
	}

	if (signature_file && metadata_file) {
		msg("Cannot append/extract metadata and signature in one run\n");
		return 1;
	}

	if (file_mode)
		ret = add_data(argv[optind]);
	else
		ret = extract_data(argv[optind]);

out:
	cleanup();
	return ret;
}

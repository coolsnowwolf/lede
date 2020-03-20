/*
 * mkdlinkfw
 *
 * Copyright (C) 2018 Paweł Dembicki <paweldembicki@gmail.com>
 *
 * This tool is based on mktplinkfw.
 * Copyright (C) 2009 Gabor Juhos <juhosg@openwrt.org>
 * Copyright (C) 2008,2009 Wang Jian <lark@linux.net.cn>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>		/* for unlink() */
#include <libgen.h>
#include <getopt.h>		/* for getopt() */
#include <stdarg.h>
#include <stdbool.h>
#include <endian.h>
#include <errno.h>
#include <time.h>
#include <sys/stat.h>
#include <zlib.h>		/*for crc32 */

#include "mkdlinkfw-lib.h"

extern char *progname;

uint32_t jboot_timestamp(void)
{
	char *env = getenv("SOURCE_DATE_EPOCH");
	char *endptr = env;
	time_t fixed_timestamp = -1;
	errno = 0;

	if (env && *env) {
		fixed_timestamp = strtoull(env, &endptr, 10);

		if (errno || (endptr && *endptr != '\0')) {
			fprintf(stderr, "Invalid SOURCE_DATE_EPOCH");
			fixed_timestamp = -1;
		}
	}

	if (fixed_timestamp == -1)
		time(&fixed_timestamp);

	return (((uint32_t) fixed_timestamp) - TIMESTAMP_MAGIC) >> 2;
}

uint16_t jboot_checksum(uint16_t start_val, uint16_t *data, int size)
{
	uint32_t counter = start_val;
	uint16_t *ptr = data;

	while (size > 1) {
		counter += *ptr;
		++ptr;
		while (counter >> 16)
			counter = (uint16_t) counter + (counter >> 16);
		size -= 2;
	}
	if (size > 0) {
		counter += *(uint8_t *) ptr;
		counter -= 0xFF;
	}
	while (counter >> 16)
		counter = (uint16_t) counter + (counter >> 16);
	return counter;
}

int get_file_stat(struct file_info *fdata)
{
	struct stat st;
	int res;

	if (fdata->file_name == NULL)
		return 0;

	res = stat(fdata->file_name, &st);
	if (res) {
		ERRS("stat failed on %s", fdata->file_name);
		return res;
	}

	fdata->file_size = st.st_size;
	return 0;
}

int read_to_buf(const struct file_info *fdata, char *buf)
{
	FILE *f;
	int ret = EXIT_FAILURE;
	size_t read;

	f = fopen(fdata->file_name, "r");
	if (f == NULL) {
		ERRS("could not open \"%s\" for reading", fdata->file_name);
		goto out;
	}

	read = fread(buf, fdata->file_size, 1, f);
	if (ferror(f) || read != 1) {
		ERRS("unable to read from file \"%s\"", fdata->file_name);
		goto out_close;
	}

	ret = EXIT_SUCCESS;

 out_close:
	fclose(f);
 out:
	return ret;
}

int write_fw(const char *ofname, const char *data, int len)
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
	if (ret != EXIT_SUCCESS)
		unlink(ofname);
 out:
	return ret;
}

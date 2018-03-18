/*
 * Copyright (C) 2009 Gabor Juhos <juhosg@openwrt.org>
 *
 * This tool was based on:
 *   TP-Link WR941 V2 firmware checksum fixing tool.
 *   Copyright (C) 2008,2009 Wang Jian <lark@linux.net.cn>
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
#include <stdbool.h>
#include <endian.h>
#include <errno.h>
#include <sys/stat.h>

#include <arpa/inet.h>
#include <netinet/in.h>

#include "mktplinkfw-lib.h"
#include "md5.h"

extern char *ofname;
extern char *progname;
extern uint32_t kernel_len;
extern struct file_info kernel_info;
extern struct file_info rootfs_info;
extern struct flash_layout *layout;
extern uint32_t rootfs_ofs;
extern uint32_t rootfs_align;
extern int combined;
extern int strip_padding;
extern int add_jffs2_eof;

static unsigned char jffs2_eof_mark[4] = {0xde, 0xad, 0xc0, 0xde};

void fill_header(char *buf, int len);

struct flash_layout *find_layout(struct flash_layout *layouts, const char *id)
{
	struct flash_layout *ret;
	struct flash_layout *l;

	ret = NULL;
	for (l = layouts; l->id != NULL; l++){
		if (strcasecmp(id, l->id) == 0) {
			ret = l;
			break;
		}
	};

	return ret;
}

void get_md5(const char *data, int size, uint8_t *md5)
{
	MD5_CTX ctx;

	MD5_Init(&ctx);
	MD5_Update(&ctx, data, size);
	MD5_Final(md5, &ctx);
}

int get_file_stat(struct file_info *fdata)
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

int read_to_buf(const struct file_info *fdata, char *buf)
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

static int pad_jffs2(char *buf, int currlen, int maxlen)
{
	int len;
	uint32_t pad_mask;

	len = currlen;
	pad_mask = (4 * 1024) | (64 * 1024);	/* EOF at 4KB and at 64KB */
	while ((len < maxlen) && (pad_mask != 0)) {
		uint32_t mask;
		int i;

		for (i = 10; i < 32; i++) {
			mask = 1 << i;
			if (pad_mask & mask)
				break;
		}

		len = ALIGN(len, mask);

		for (i = 10; i < 32; i++) {
			mask = 1 << i;
			if ((len & (mask - 1)) == 0)
				pad_mask &= ~mask;
		}

		for (i = 0; i < sizeof(jffs2_eof_mark); i++)
			buf[len + i] = jffs2_eof_mark[i];

		len += sizeof(jffs2_eof_mark);
	}

	return len;
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
	if (ret != EXIT_SUCCESS) {
		unlink(ofname);
	}
out:
	return ret;
}

/* Helper functions to inspect_fw() representing different output formats */
inline void inspect_fw_pstr(const char *label, const char *str)
{
	printf("%-23s: %s\n", label, str);
}

inline void inspect_fw_phex(const char *label, uint32_t val)
{
	printf("%-23s: 0x%08x\n", label, val);
}

inline void inspect_fw_phexdec(const char *label, uint32_t val)
{
	printf("%-23s: 0x%08x / %8u bytes\n", label, val, val);
}

inline void inspect_fw_pmd5sum(const char *label, const uint8_t *val, const char *text)
{
	int i;

	printf("%-23s:", label);
	for (i=0; i<MD5SUM_LEN; i++)
		printf(" %02x", val[i]);
	printf(" %s\n", text);
}

// header_size = sizeof(struct fw_header)
int build_fw(size_t header_size)
{
	int buflen;
	char *buf;
	char *p;
	int ret = EXIT_FAILURE;
	int writelen = 0;

	writelen = header_size + kernel_len;

	if (combined)
		buflen = writelen;
	else
		buflen = layout->fw_max_len;

	buf = malloc(buflen);
	if (!buf) {
		ERR("no memory for buffer\n");
		goto out;
	}

	memset(buf, 0xff, buflen);
	p = buf + header_size;
	ret = read_to_buf(&kernel_info, p);
	if (ret)
		goto out_free_buf;

	if (!combined) {
		p = buf + rootfs_ofs;

		ret = read_to_buf(&rootfs_info, p);
		if (ret)
			goto out_free_buf;

		writelen = rootfs_ofs + rootfs_info.file_size;

		if (add_jffs2_eof)
			writelen = pad_jffs2(buf, writelen, layout->fw_max_len);
	}

	if (!strip_padding)
		writelen = buflen;

	fill_header(buf, writelen);
	ret = write_fw(ofname, buf, writelen);
	if (ret)
		goto out_free_buf;

	ret = EXIT_SUCCESS;

out_free_buf:
	free(buf);
out:
	return ret;
}

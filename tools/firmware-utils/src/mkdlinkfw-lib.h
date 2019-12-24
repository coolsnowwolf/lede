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

#ifndef mkdlinkfw_lib_h
#define mkdlinkfw_lib_h

#define AUH_MAGIC "DLK"
#define AUH_SIZE 80
#define AUH_LVPS 0x01
#define AUH_HDR_ID 0x4842
#define AUH_HDR_VER 0x02
#define AUH_SEC_ID 0x04
#define AUH_INFO_TYPE 0x04

#define STAG_SIZE 16
#define STAG_ID 0x04
#define STAG_MAGIC 0x2B24
#define STAG_CMARK_FACTORY 0xFF

#define SCH2_SIZE 40
#define SCH2_MAGIC 0x2124
#define SCH2_VER 0x02

/*
 * compression type values in the header
 * so far onlysupport for LZMA is added
 */
#define FLAT 0
#define JZ 1
#define GZIP 2
#define LZMA 3

#define RAM_ENTRY_ADDR 0x80000000
#define RAM_LOAD_ADDR 0x80000000
#define JBOOT_SIZE 0x10000

#define ALL_HEADERS_SIZE (AUH_SIZE + STAG_SIZE + SCH2_SIZE)
#define MAX_HEADER_COUNTER 10
#define TIMESTAMP_MAGIC 0x35016f00L

#define FACTORY 0
#define SYSUPGRADE 1

#define ERR(fmt, ...) do { \
	fflush(0); \
	fprintf(stderr, "[%s] *** error: " fmt "\n", \
			progname, ## __VA_ARGS__); \
} while (0)

#define ERRS(fmt, ...) do { \
	int save = errno; \
	fflush(0); \
	fprintf(stderr, "[%s] *** error: " fmt ": %s\n", \
			progname, ## __VA_ARGS__, strerror(save)); \
} while (0)

#define DBG(fmt, ...) do { \
	fprintf(stderr, "[%s] " fmt "\n", progname, ## __VA_ARGS__); \
} while (0)

struct file_info {
	char *file_name;	/* name of the file */
	uint32_t file_size;	/* length of the file */
};

uint32_t jboot_timestamp(void);
uint16_t jboot_checksum(uint16_t start_val, uint16_t *data, int size);
int get_file_stat(struct file_info *fdata);
int read_to_buf(const struct file_info *fdata, char *buf);
int write_fw(const char *ofname, const char *data, int len);

#endif				/* mkdlinkfw_lib_h */

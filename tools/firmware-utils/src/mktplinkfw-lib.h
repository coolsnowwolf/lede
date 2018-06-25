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


#ifndef mktplinkfw_lib_h
#define mktplinkfw_lib_h

#define ALIGN(x,a) ({ typeof(a) __a = (a); (((x) + __a - 1) & ~(__a - 1)); })
#define ARRAY_SIZE(a) (sizeof((a)) / sizeof((a)[0]))

#define MD5SUM_LEN	16

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


struct file_info {
	char		*file_name;	/* name of the file */
	uint32_t	file_size;	/* length of the file */
};

struct flash_layout {
	char		*id;
	uint32_t	fw_max_len;
	uint32_t	kernel_la;
	uint32_t	kernel_ep;
	uint32_t	rootfs_ofs;
};

struct flash_layout *find_layout(struct flash_layout *layouts, const char *id);
void get_md5(const char *data, int size, uint8_t *md5);
int get_file_stat(struct file_info *fdata);
int read_to_buf(const struct file_info *fdata, char *buf);
int write_fw(const char *ofname, const char *data, int len);
inline void inspect_fw_pstr(const char *label, const char *str);
inline void inspect_fw_phex(const char *label, uint32_t val);
inline void inspect_fw_phexdec(const char *label, uint32_t val);
inline void inspect_fw_pmd5sum(const char *label, const uint8_t *val, const char *text);
int build_fw(size_t header_size);

#endif /* mktplinkfw_lib_h */

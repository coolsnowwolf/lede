/*
 * Builder/viewer/extractor utility for Poray firmware image files
 *
 * Copyright (C) 2013 Michel Stempin <michel.stempin@wanadoo.fr>
 * Copyright (C) 2013 Felix Kaechele <felix@fetzig.org>
 * Copyright (C) 2013 <admin@openschemes.com>
 *
 * This tool is based on:
 *   TP-Link firmware upgrade tool.
 *   Copyright (C) 2009 Gabor Juhos <juhosg@openwrt.org>
 *
 * Itself based on:
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
#include <unistd.h>
#include <libgen.h>
#include <getopt.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <netinet/in.h>

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

/* Fixed header flags */
#define HEADER_FLAGS		0x020e0000

/* Recognized Hardware ID magic */
#define HWID_HAME_MPR_A1_L8	0x32473352
#define HWID_PORAY_R50B		0x31353033
#define HWID_PORAY_R50D		0x33353033
#define HWID_PORAY_R50E		0x34353033
#define HWID_PORAY_M3		0x31353335
#define HWID_PORAY_M4		0x32353335
#define HWID_PORAY_Q3		0x33353335
#define HWID_PORAY_X5_X6	0x35353335
#define HWID_PORAY_X8		0x36353335
#define HWID_PORAY_X1		0x38353335
#define HWID_NEXX_WT1520	0x30353332
#define HWID_NEXX_WT3020	0x30323033
#define HWID_A5_V11		0x32473352

/* Recognized XOR obfuscation keys */
#define KEY_HAME		0
#define KEY_PORAY_1		1
#define KEY_PORAY_2		2
#define KEY_PORAY_3		3
#define KEY_PORAY_4		4
#define KEY_NEXX_1		5
#define KEY_NEXX_2		6
#define KEY_A5_V11		7

/* XOR key length */
#define KEY_LEN			15

struct file_info {
	char		*file_name;	/* Name of the file */
	uint32_t	file_size;	/* Length of the file */
};

struct fw_header {
	uint32_t	hw_id;		/* Hardware id */
	uint32_t	firmware_len;	/* Firmware data length */
	uint32_t	flags;		/* Header flags */
	uint8_t		pad[16];
} __attribute__ ((packed));

struct flash_layout {
	char		*id;
	uint32_t	fw_max_len;
};

struct board_info {
	char		*id;
	uint32_t	hw_id;
	char		*layout_id;
	uint32_t	key;
};

/*
 * Globals
 */
static char *ofname;
static char *progname;

static char *board_id;
static struct board_info *board;
static char *layout_id;
static struct flash_layout *layout;
static char *opt_hw_id;
static uint32_t hw_id;
static struct file_info firmware_info;
static uint32_t firmware_len = 0;

static int inspect = 0;
static int extract = 0;

static uint8_t key[][KEY_LEN] = {
  {0xC8, 0x3C, 0x3A, 0x93, 0xA2, 0x95, 0xC3, 0x63, 0x48, 0x45, 0x58, 0x09, 0x12, 0x03, 0x08},
  {0x89, 0x6B, 0x5A, 0x93, 0x92, 0x95, 0xC3, 0x63, 0xD0, 0xA3, 0x9C, 0x92, 0x2E, 0xE6, 0xC7},
  {0xC9, 0x1C, 0x3A, 0x93, 0x92, 0x95, 0xC3, 0x63, 0xD0, 0xA3, 0x9C, 0x92, 0x2E, 0xE6, 0xC7},
  {0x19, 0x1B, 0x3A, 0x93, 0x92, 0x95, 0xC3, 0x63, 0xD0, 0xA3, 0x9C, 0x92, 0x2E, 0xE6, 0xC7},
  {0x79, 0x7B, 0x7A, 0x93, 0x92, 0x95, 0xC3, 0x63, 0xD0, 0xA3, 0x9C, 0x92, 0x2E, 0xE6, 0xC7},
  {0x19, 0x1C, 0x4A, 0x93, 0x96, 0x95, 0xC3, 0x63, 0xD0, 0xA3, 0x9C, 0x92, 0x2E, 0x16, 0xC6},
  {0x39, 0x1C, 0x4A, 0x93, 0x96, 0x95, 0xC3, 0x63, 0xD0, 0xA3, 0x9C, 0x92, 0x2E, 0x16, 0xC6},
  {0xC8, 0x3C, 0x3A, 0x93, 0xA2, 0x95, 0xC3, 0x63, 0x48, 0x45, 0x58, 0x09, 0x20, 0x11, 0x08},
};

static struct flash_layout layouts[] = {
	{
		.id		= "4M",
		.fw_max_len	= 0x3c0000,
	}, {
		.id		= "8M",
		.fw_max_len	= 0x7c0000,
	}, {
		/* terminating entry */
	}
};

static struct board_info boards[] = {
	{
		.id             = "A5-V11",
		.hw_id          = HWID_A5_V11,
		.layout_id      = "4M",
		.key            = KEY_A5_V11,
        }, {
		.id		= "MPR-A1",
		.hw_id		= HWID_HAME_MPR_A1_L8,
		.layout_id	= "4M",
		.key		= KEY_HAME,
	}, {
		.id		= "MPR-L8",
		.hw_id		= HWID_HAME_MPR_A1_L8,
		.layout_id	= "4M",
		.key		= KEY_HAME,
	}, {
		.id		= "R50B",
		.hw_id		= HWID_PORAY_R50B,
		.layout_id	= "4M",
		.key		= KEY_PORAY_2,
	}, {
		.id		= "R50D",
		.hw_id		= HWID_PORAY_R50D,
		.layout_id	= "4M",
		.key		= KEY_PORAY_3,
	}, {
		.id		= "R50E",
		.hw_id		= HWID_PORAY_R50E,
		.layout_id	= "4M",
		.key		= KEY_PORAY_4,
	}, {
		.id		= "M3",
		.hw_id		= HWID_PORAY_M3,
		.layout_id	= "4M",
		.key		= KEY_PORAY_1,
	}, {
		.id		= "M4",
		.hw_id		= HWID_PORAY_M4,
		.layout_id	= "4M",
		.key		= KEY_PORAY_1,
	}, {
		.id		= "Q3",
		.hw_id		= HWID_PORAY_Q3,
		.layout_id	= "4M",
		.key		= KEY_PORAY_1,
	}, {
		.id		= "X5 or X6",
		.hw_id		= HWID_PORAY_X5_X6,
		.layout_id	= "8M",
		.key		= KEY_PORAY_1,
	}, {
		.id		= "X5",
		.hw_id		= HWID_PORAY_X5_X6,
		.layout_id	= "8M",
		.key		= KEY_PORAY_1,
	}, {
		.id		= "X6",
		.hw_id		= HWID_PORAY_X5_X6,
		.layout_id	= "8M",
		.key		= KEY_PORAY_1,
	}, {
		.id		= "X8",
		.hw_id		= HWID_PORAY_X8,
		.layout_id	= "8M",
		.key		= KEY_PORAY_1,
	}, {
		.id		= "X1",
		.hw_id		= HWID_PORAY_X1,
		.layout_id	= "8M",
		.key		= KEY_PORAY_1,
	}, {
		.id		= "WT1520",
		.hw_id		= HWID_NEXX_WT1520,
		.layout_id	= "4M",
		.key		= KEY_NEXX_1,
	}, {
		.id		= "WT1520",
		.hw_id		= HWID_NEXX_WT1520,
		.layout_id	= "8M",
		.key		= KEY_NEXX_1,
        }, {
                .id             = "WT3020",
                .hw_id          = HWID_NEXX_WT3020,
                .layout_id      = "4M",
                .key            = KEY_NEXX_2,
        }, {
                .id             = "WT3020",
                .hw_id          = HWID_NEXX_WT3020,
                .layout_id      = "8M",
                .key            = KEY_NEXX_2,
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
	fprintf(stderr, "[%s] *** error: " fmt ":%s\n", \
			progname, ## __VA_ARGS__, strerror(save)); \
} while (0)

#define DBG(fmt, ...) do { \
	fprintf(stderr, "[%s] " fmt "\n", progname, ## __VA_ARGS__ ); \
} while (0)

/*
 * Find a board by its name
 */
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

/*
 * Find a board by its hardware ID
 */
static struct board_info *find_board_by_hwid(uint32_t hw_id)
{
	struct board_info *board;

	for (board = boards; board->id != NULL; board++) {
		if (hw_id == board->hw_id)
			return board;
	};

	return NULL;
}

/*
 * Find a Flash memory layout by its name
 */
static struct flash_layout *find_layout(char *id)
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

/*
 * Display usage
 */
static void usage(int status)
{
	FILE *stream = (status != EXIT_SUCCESS) ? stderr : stdout;

	fprintf(stream, "Usage: %s [OPTIONS...]\n", progname);
	fprintf(stream,
"\n"
"Options:\n"
"  -B <board>      create image for the board specified with <board>\n"
"  -H <hwid>       use hardware id specified with <hwid>\n"
"  -F <id>         use flash layout specified with <id>\n"
"  -f <file>       read firmware image from the file <file>\n"
"  -o <file>       write output to the file <file>\n"
"  -i              inspect given firmware file (requires -f)\n"
"  -x              extract combined kernel and rootfs while inspecting (implies -i)\n"
"  -h              show this screen\n"
	);

	exit(status);
}

/*
 * Get file statistics
 */
static int get_file_stat(struct file_info *fdata)
{
	struct stat st;
	int res;

	if (fdata->file_name == NULL) {
		return 0;
	}
	res = stat(fdata->file_name, &st);
	if (res){
		ERRS("stat failed on %s", fdata->file_name);
		return res;
	}

	fdata->file_size = st.st_size;
	return 0;
}

/*
 * Read file into buffer
 */
static int read_to_buf(struct file_info *fdata, uint8_t *buf)
{
	FILE *f;
	int ret = EXIT_FAILURE;

	f = fopen(fdata->file_name, "rb");
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

/*
 * Check command line options
 */
static int check_options(void)
{
	int ret;

	if (firmware_info.file_name == NULL) {
		ERR("no firmware image specified");
		return -1;
	}

	ret = get_file_stat(&firmware_info);
	if (ret)
		return ret;

	if (inspect)
		return 0;

	if (board_id == NULL && opt_hw_id == NULL) {
		ERR("either board or hardware id must be specified");
		return -1;
	}

	if (board_id) {
		board = find_board(board_id);
		if (board == NULL) {
			ERR("unknown/unsupported board id \"%s\"", board_id);
			return -1;
		}
		if (layout_id == NULL) {
			layout_id = board->layout_id;
		}
		hw_id = board->hw_id;
	} else {
		hw_id = strtoul(opt_hw_id, NULL, 0);
		board = find_board_by_hwid(hw_id);
		if (layout_id == NULL) {
			layout_id = board->layout_id;
		}
	}

	layout = find_layout(layout_id);
	if (layout == NULL) {
		ERR("unknown flash layout \"%s\"", layout_id);
		return -1;
	}

	firmware_len = firmware_info.file_size;

	if (firmware_info.file_size >
		layout->fw_max_len - sizeof (struct fw_header)) {
		ERR("firmware image is too big");
		return -1;
	}

	if (ofname == NULL) {
		ERR("no output file specified");
		return -1;
	}
	return 0;
}

/*
 * Fill in firmware header
 */
static void fill_header(uint8_t *buf)
{
	struct fw_header *hdr = (struct fw_header *) buf;

	memset(hdr, 0, sizeof (struct fw_header));
	hdr->hw_id = HOST_TO_LE32(hw_id);
	hdr->firmware_len = HOST_TO_LE32(firmware_len);
	hdr->flags = HOST_TO_LE32(HEADER_FLAGS);
}

/*
 * Compute firmware checksum
 */
static uint16_t checksum_fw(uint8_t *data, int len)
{
	int i;
	int32_t checksum = 0;

	for (i = 0; i < len - 1; i += 2) {
		checksum += (data[i + 1] << 8) | data[i];
	}
	if (i < len) {
		checksum += data[i];
	}
	checksum = checksum + (checksum >> 16) + 0xffff;
	checksum = ~(checksum + (checksum >> 16)) & 0xffff;
	return (uint16_t) checksum;
}

/*
 * (De)obfuscate firmware using an XOR operation with a fixed length key
 */
static void xor_fw(uint8_t *data, int len)
{
	int i;

	for (i = 0; i <= len; i++) {
		data[i] ^= key[board->key][i % KEY_LEN];
	}
}

/*
 * Write firmware to file
 */
static int write_fw(uint8_t *data, int len)
{
	FILE *f;
	int ret = EXIT_FAILURE;

	f = fopen(ofname, "wb");
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

/*
 * Build firmware file
 */
static int build_fw(void)
{
	int buflen;
	uint8_t *buf, *p;
	int ret = EXIT_FAILURE;
	int writelen = 0;
	uint16_t checksum;

	buflen = layout->fw_max_len;

	buf = (uint8_t *) malloc(buflen);
	if (!buf) {
		ERR("no memory for buffer\n");
		goto out;
	}

	memset(buf, 0xff, buflen);
	p = buf + sizeof (struct fw_header);
	ret = read_to_buf(&firmware_info, p);
	if (ret) {
		goto out_free_buf;
	}
	writelen = sizeof (struct fw_header) + firmware_len + 2;

	/* Fill in header */
	fill_header(buf);

	/* Compute firmware checksum */
	checksum = checksum_fw(buf + sizeof (struct fw_header), firmware_len);

	/* Cannot use network order function because checksum is not word-aligned */
	buf[writelen - 1] = checksum >> 8;
	buf[writelen - 2] = checksum & 0xff;

	/* XOR obfuscate firmware */
	xor_fw(buf + sizeof (struct fw_header), firmware_len + 2);

	/* Write firmware file */
	ret = write_fw(buf, writelen);
	if (ret) {
		goto out_free_buf;
	}
	ret = EXIT_SUCCESS;

 out_free_buf:
	free(buf);
 out:
	return ret;
}

/* Helper functions to inspect_fw() representing different output formats */
static inline void inspect_fw_pstr(char *label, char *str)
{
	printf("%-23s: %s\n", label, str);
}

static inline void inspect_fw_phex(char *label, uint32_t val)
{
	printf("%-23s: 0x%08x\n", label, val);
}

static inline void inspect_fw_phexpost(char *label,
                                       uint32_t val, char *post)
{
	printf("%-23s: 0x%08x (%s)\n", label, val, post);
}

static inline void inspect_fw_phexdef(char *label,
                                      uint32_t val, uint32_t defval)
{
	printf("%-23s: 0x%08x                  ", label, val);

	if (val == defval) {
		printf("(== OpenWrt default)\n");
	} else {
		printf("(OpenWrt default: 0x%08x)\n", defval);
	}
}

static inline void inspect_fw_phexexp(char *label,
                                      uint32_t val, uint32_t expval)
{
	printf("%-23s: 0x%08x ", label, val);

	if (val == expval) {
		printf("(ok)\n");
	} else {
		printf("(expected: 0x%08x)\n", expval);
	}
}

static inline void inspect_fw_phexdec(char *label, uint32_t val)
{
	printf("%-23s: 0x%08x / %8u bytes\n", label, val, val);
}

static inline void inspect_fw_pchecksum(char *label,
					uint16_t val, uint16_t expval)
{
	printf("%-23s: 0x%04x     ", label, val);
	if (val == expval) {
		printf("(ok)\n");
	} else {
		printf("(expected: 0x%04x)\n", expval);
	}
}

static int inspect_fw(void)
{
	uint8_t *buf;
	struct fw_header *hdr;
	int ret = EXIT_FAILURE;
	uint16_t computed_checksum, file_checksum;

	buf = (uint8_t *) malloc(firmware_info.file_size);
	if (!buf) {
		ERR("no memory for buffer!\n");
		goto out;
	}

	ret = read_to_buf(&firmware_info, buf);
	if (ret) {
		goto out_free_buf;
	}
	hdr = (struct fw_header *)buf;

	inspect_fw_pstr("File name", firmware_info.file_name);
	inspect_fw_phexdec("File size", firmware_info.file_size);

	printf("\n");

	inspect_fw_phexdec("Header size", sizeof (struct fw_header));
	board = find_board_by_hwid(LE32_TO_HOST(hdr->hw_id));
	if (board) {
		layout = find_layout(board->layout_id);
		inspect_fw_phexpost("Hardware ID",
				    LE32_TO_HOST( hdr->hw_id), board->id);
	} else {
		inspect_fw_phexpost("Hardware ID",
		                    LE32_TO_HOST(hdr->hw_id), "unknown");
	}
	inspect_fw_phexdec("Firmware data length",
	                   LE32_TO_HOST(hdr->firmware_len));

	inspect_fw_phexexp("Flags",
			   LE32_TO_HOST(hdr->flags), HEADER_FLAGS);
	printf("\n");

	/* XOR unobfuscate firmware */
	xor_fw(buf + sizeof (struct fw_header), LE32_TO_HOST(hdr->firmware_len) + 2);

	/* Compute firmware checksum */
	computed_checksum = checksum_fw(buf + sizeof (struct fw_header), LE32_TO_HOST(hdr->firmware_len));

	/* Cannot use network order function because checksum is not word-aligned */
	file_checksum = (buf[firmware_info.file_size - 1] << 8) | buf[firmware_info.file_size - 2];
	inspect_fw_pchecksum("Firmware checksum", computed_checksum, file_checksum);

	/* Verify checksum */
	if (computed_checksum != file_checksum) {
		ret = -1;
		ERR("checksums do not match");
		goto out_free_buf;
	}

	printf("\n");

	if (extract) {
		FILE *fp;
		char *filename;

		if (ofname == NULL) {
			filename = malloc(strlen(firmware_info.file_name) + 10);
			sprintf(filename, "%s-firmware", firmware_info.file_name);
		} else {
			filename = ofname;
		}
		printf("Extracting firmware to \"%s\"...\n", filename);
		fp = fopen(filename, "wb");
		if (fp)	{
		  if (!fwrite(buf + sizeof (struct fw_header),
			            LE32_TO_HOST(hdr->firmware_len), 1, fp)) {
				ERRS("error in fwrite(): %s", strerror(errno));
			}
			fclose(fp);
		} else {
			ERRS("error in fopen(): %s", strerror(errno));
		}
		if (ofname == NULL) {
			free(filename);
		}
		printf("\n");
	}

 out_free_buf:
	free(buf);
 out:
	return ret;
}

/*
 * Main entry point
 */
int main(int argc, char *argv[])
{
	int ret = EXIT_FAILURE;

	progname = basename(argv[0]);

	int c;

	while ((c = getopt(argc, argv, "B:H:F:f:o:ixh")) != -1) {
		switch (c) {
		case 'B':
			board_id = optarg;
			break;
		case 'H':
			opt_hw_id = optarg;
			break;
		case 'F':
			layout_id = optarg;
			break;
		case 'f':
			firmware_info.file_name = optarg;
			break;
		case 'o':
			ofname = optarg;
			break;
		case 'i':
			inspect = 1;
			break;
		case 'x':
			inspect = 1;
			extract = 1;
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
	if (ret) {
		goto out;
	}
	if (!inspect) {
		ret = build_fw();
	} else {
		ret = inspect_fw();
	}

 out:
	return ret;
}

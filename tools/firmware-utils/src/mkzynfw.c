/*
 *
 *  Copyright (C) 2007-2008 OpenWrt.org
 *  Copyright (C) 2007-2008 Gabor Juhos <juhosg at openwrt.org>
 *
 *  This code was based on the information of the ZyXEL's firmware
 *  image format written by Kolja Waschk, can be found at:
 *  http://www.ixo.de/info/zyxel_uclinux
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
#include <unistd.h>	/* for unlink() */
#include <libgen.h>
#include <getopt.h>	/* for getopt() */
#include <stdarg.h>
#include <errno.h>
#include <sys/stat.h>
#include <endian.h>	/* for __BYTE_ORDER */
#if defined(__CYGWIN__)
#  include <byteswap.h>
#endif
#include <inttypes.h>

#include "zynos.h"

#if (__BYTE_ORDER == __LITTLE_ENDIAN)
#  define HOST_TO_LE16(x)	(x)
#  define HOST_TO_LE32(x)	(x)
#  define LE16_TO_HOST(x)	(x)
#  define LE32_TO_HOST(x)	(x)
#  define HOST_TO_BE16(x)	bswap_16(x)
#  define HOST_TO_BE32(x)	bswap_32(x)
#  define BE16_TO_HOST(x)	bswap_16(x)
#  define BE32_TO_HOST(x)	bswap_32(x)
#else
#  define HOST_TO_BE16(x)	(x)
#  define HOST_TO_BE32(x)	(x)
#  define BE16_TO_HOST(x)	(x)
#  define BE32_TO_HOST(x)	(x)
#  define HOST_TO_LE16(x)	bswap_16(x)
#  define HOST_TO_LE32(x)	bswap_32(x)
#  define LE16_TO_HOST(x)	bswap_16(x)
#  define LE32_TO_HOST(x)	bswap_32(x)
#endif

#define ALIGN(x,y)	(((x)+((y)-1)) & ~((y)-1))

#define MAX_NUM_BLOCKS	8
#define MAX_ARG_COUNT	32
#define MAX_ARG_LEN	1024
#define FILE_BUF_LEN	(16*1024)


struct csum_state{
	int		odd;
	uint32_t	sum;
	uint32_t	tmp;
};

struct fw_block {
	uint32_t	align;		/* alignment of this block */
	char		*file_name;	/* name of the file */
	uint32_t	file_size;	/* length of the file */
	char		*mmap_name;	/* name in the MMAP table */
	int		type;		/* block type */
	uint32_t	padlen;
	uint8_t		padc;
};

#define BLOCK_TYPE_BOOTEXT	0
#define BLOCK_TYPE_RAW		1

struct fw_mmap {
	uint32_t	addr;
	uint32_t	size;
	uint32_t	user_addr;
	uint32_t	user_size;
};
#define MMAP_DATA_SIZE	1024
#define MMAP_ALIGN	16

struct board_info {
	char *name;		/* model name */
	char *desc;		/* description */
	uint16_t vendor;	/* vendor id */
	uint16_t model;		/* model id */
	uint32_t flash_base;	/* flash base address */
	uint32_t flash_size;	/* board flash size */
	uint32_t code_start;	/* code start address */
	uint32_t romio_offs;	/* offset of the firmware within the flash */
	uint32_t bootext_size;	/* maximum size of bootext block */
};

/*
 * Globals
 */
char *progname;
char *ofname = NULL;
int verblevel = 0;

struct board_info *board = NULL;

struct fw_block blocks[MAX_NUM_BLOCKS];
struct fw_block *bootext_block = NULL;
int num_blocks = 0;

#define ADM5120_FLASH_BASE	0xBFC00000
#define ADM5120_CODE_START	0x80008000

/* TODO: check values for AR7 */
#define AR7_FLASH_BASE		0xB0000000
#define AR7_CODE_START		0x94008000

#define ATHEROS_FLASH_BASE	0xBFC00000
#define ATHEROS_CODE_START	0x80e00000

#define AR71XX_FLASH_BASE	0xBFC00000
#define AR71XX_CODE_START	0x81E00000

#define BOARD(n, d, v, m, fb, fs, cs, fo) {		\
	.name = (n), .desc=(d),				\
	.vendor = (v), .model = (m),			\
	.flash_base = (fb), .flash_size = (fs)<<20,	\
	.code_start = (cs), .romio_offs = (fo),		\
	.bootext_size = BOOTEXT_DEF_SIZE		\
	}

#define ADMBOARD1(n, d, m, fs) BOARD(n, d, ZYNOS_VENDOR_ID_ZYXEL, m, \
	ADM5120_FLASH_BASE, fs, ADM5120_CODE_START, 0x8000)

#define ADMBOARD2(n, d, m, fs) BOARD(n, d, ZYNOS_VENDOR_ID_ZYXEL, m, \
	ADM5120_FLASH_BASE, fs, ADM5120_CODE_START, 0x10000)

#define AR7BOARD1(n, d, m, fs) BOARD(n, d, ZYNOS_VENDOR_ID_ZYXEL, m, \
	AR7_FLASH_BASE, fs, AR7_CODE_START, 0x8000)

#define ATHEROSBOARD1(n, d, m, fs) BOARD(n, d, ZYNOS_VENDOR_ID_ZYXEL, m, \
	ATHEROS_FLASH_BASE, fs, ATHEROS_CODE_START, 0x30000)

#define AR71XXBOARD1(n, d, m, fs) {		\
	.name = (n), .desc=(d),				\
	.vendor = (ZYNOS_VENDOR_ID_ZYXEL), .model = (m),			\
	.flash_base = (AR71XX_FLASH_BASE), .flash_size = (fs)<<20,	\
	.code_start = (AR71XX_CODE_START), .romio_offs = (0x40000),		\
	.bootext_size = 0x30000		\
	}


static struct board_info boards[] = {
	/*
	 * Infineon/ADMtek ADM5120 based boards
	 */
	ADMBOARD2("ES-2024A",	"ZyXEL ES-2024A", ZYNOS_MODEL_ES_2024A, 4),
	ADMBOARD2("ES-2024PWR",	"ZyXEL ES-2024PWR", ZYNOS_MODEL_ES_2024PWR, 4),
	ADMBOARD2("ES-2108",	"ZyXEL ES-2108", ZYNOS_MODEL_ES_2108, 4),
	ADMBOARD2("ES-2108-F",	"ZyXEL ES-2108-F", ZYNOS_MODEL_ES_2108_F, 4),
	ADMBOARD2("ES-2108-G",	"ZyXEL ES-2108-G", ZYNOS_MODEL_ES_2108_G, 4),
	ADMBOARD2("ES-2108-LC",	"ZyXEL ES-2108-LC", ZYNOS_MODEL_ES_2108_LC, 4),
	ADMBOARD2("ES-2108PWR",	"ZyXEL ES-2108PWR", ZYNOS_MODEL_ES_2108PWR, 4),
	ADMBOARD1("HS-100",	"ZyXEL HomeSafe 100", ZYNOS_MODEL_HS_100, 2),
	ADMBOARD1("HS-100W",	"ZyXEL HomeSafe 100W", ZYNOS_MODEL_HS_100W, 2),
	ADMBOARD1("P-334",	"ZyXEL Prestige 334", ZYNOS_MODEL_P_334, 2),
	ADMBOARD1("P-334U",	"ZyXEL Prestige 334U", ZYNOS_MODEL_P_334U, 4),
	ADMBOARD1("P-334W",	"ZyXEL Prestige 334W", ZYNOS_MODEL_P_334W, 2),
	ADMBOARD1("P-334WH",	"ZyXEL Prestige 334WH", ZYNOS_MODEL_P_334WH, 4),
	ADMBOARD1("P-334WHD",	"ZyXEL Prestige 334WHD", ZYNOS_MODEL_P_334WHD, 4),
	ADMBOARD1("P-334WT",	"ZyXEL Prestige 334WT", ZYNOS_MODEL_P_334WT, 4),
	ADMBOARD1("P-335",	"ZyXEL Prestige 335", ZYNOS_MODEL_P_335, 4),
	ADMBOARD1("P-335Plus",	"ZyXEL Prestige 335Plus", ZYNOS_MODEL_P_335PLUS, 4),
	ADMBOARD1("P-335U",	"ZyXEL Prestige 335U", ZYNOS_MODEL_P_335U, 4),
	ADMBOARD1("P-335WT",	"ZyXEL Prestige 335WT", ZYNOS_MODEL_P_335WT, 4),

	{
		.name		= "P-2602HW-D1A",
		.desc		= "ZyXEL P-2602HW-D1A",
		.vendor		= ZYNOS_VENDOR_ID_ZYXEL,
		.model		= ZYNOS_MODEL_P_2602HW_D1A,
		.flash_base	= AR7_FLASH_BASE,
		.flash_size	= 4*1024*1024,
		.code_start	= 0x94008000,
		.romio_offs	= 0x20000,
		.bootext_size	= BOOTEXT_DEF_SIZE,
	},

#if 0
	/*
	 * Texas Instruments AR7 based boards
	 */
	AR7BOARD1("P-660H-61",  "ZyXEL P-660H-61", ZYNOS_MODEL_P_660H_61, 2),
	AR7BOARD1("P-660H-63",  "ZyXEL P-660H-63", ZYNOS_MODEL_P_660H_63, 2),
	AR7BOARD1("P-660H-D1",  "ZyXEL P-660H-D1", ZYNOS_MODEL_P_660H_D1, 2),
	AR7BOARD1("P-660H-D3",  "ZyXEL P-660H-D3", ZYNOS_MODEL_P_660H_D3, 2),
	AR7BOARD1("P-660HW-61", "ZyXEL P-660HW-61", ZYNOS_MODEL_P_660HW_61, 2),
	AR7BOARD1("P-660HW-63", "ZyXEL P-660HW-63", ZYNOS_MODEL_P_660HW_63, 2),
	AR7BOARD1("P-660HW-67", "ZyXEL P-660HW-67", ZYNOS_MODEL_P_660HW_67, 2),
	AR7BOARD1("P-660HW-D1", "ZyXEL P-660HW-D1", ZYNOS_MODEL_P_660HW_D1, 2),
	AR7BOARD1("P-660HW-D3", "ZyXEL P-660HW-D3", ZYNOS_MODEL_P_660HW_D3, 2),
	AR7BOARD1("P-660R-61",  "ZyXEL P-660R-61", ZYNOS_MODEL_P_660R_61, 2),
	AR7BOARD1("P-660R-61C", "ZyXEL P-660R-61C", ZYNOS_MODEL_P_660R_61C, 2),
	AR7BOARD1("P-660R-63",  "ZyXEL P-660R-63", ZYNOS_MODEL_P_660R_63, 2),
	AR7BOARD1("P-660R-63C", "ZyXEL P-660R-63C", ZYNOS_MODEL_P_660R_63C, 2),
	AR7BOARD1("P-660R-67",  "ZyXEL P-660R-67", ZYNOS_MODEL_P_660R_67, 2),
	AR7BOARD1("P-660R-D1",  "ZyXEL P-660R-D1", ZYNOS_MODEL_P_660R_D1, 2),
	AR7BOARD1("P-660R-D3",  "ZyXEL P-660R-D3", ZYNOS_MODEL_P_660R_D3, 2),
#endif
	{
		.name		= "O2SURF",
		.desc		= "O2 DSL Surf & Phone",
		.vendor		= ZYNOS_VENDOR_ID_O2,
		.model		= ZYNOS_MODEL_O2SURF,
		.flash_base	= AR7_FLASH_BASE,
		.flash_size	= 8*1024*1024,
		.code_start	= 0x94014000,
		.romio_offs	= 0x40000,
		.bootext_size	= BOOTEXT_DEF_SIZE,
	},

	/*
:x
	 */
	ATHEROSBOARD1("NBG-318S", "ZyXEL NBG-318S", ZYNOS_MODEL_NBG_318S, 4),

	/*
	 * Atheros ar71xx based boards
	 */
	AR71XXBOARD1("NBG-460N", "ZyXEL NBG-460N", ZYNOS_MODEL_NBG_460N, 4),

	{.name = NULL}
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
	fprintf(stderr, "[%s] *** error: " fmt ", %s\n", \
			progname, ## __VA_ARGS__, strerror(save)); \
} while (0)

#define WARN(fmt, ...) do { \
	fprintf(stderr, "[%s] *** warning: " fmt "\n", \
			progname, ## __VA_ARGS__ ); \
} while (0)

#define DBG(lev, fmt, ...) do { \
	if (verblevel < lev) \
		break;\
	fprintf(stderr, "[%s] " fmt "\n", progname, ## __VA_ARGS__ ); \
} while (0)

#define ERR_FATAL		-1
#define ERR_INVALID_IMAGE	-2

/*
 * Helper routines
 */
void
usage(int status)
{
	FILE *stream = (status != EXIT_SUCCESS) ? stderr : stdout;
	struct board_info *board;

	fprintf(stream, "Usage: %s [OPTIONS...]\n", progname);
	fprintf(stream,
"\n"
"Options:\n"
"  -B <board>      create image for the board specified with <board>.\n"
"                  valid <board> values:\n"
	);
	for (board = boards; board->name != NULL; board++){
		fprintf(stream,
"                    %-12s= %s\n",
		 board->name, board->desc);
	};
	fprintf(stream,
"  -b <file>[:<align>]\n"
"                  add boot extension block to the image\n"
"  -r <file>[:<align>]\n"
"                  add raw block to the image\n"
"  -o <file>       write output to the file <file>\n"
"  -h              show this screen\n"
	);

	exit(status);
}


/*
 * argument parsing
 */
int
str2u32(char *arg, uint32_t *val)
{
	char *err = NULL;
	uint32_t t;

	errno=0;
	t = strtoul(arg, &err, 0);
	if (errno || (err==arg) || ((err != NULL) && *err)) {
		return -1;
	}

	*val = t;
	return 0;
}


int
str2u16(char *arg, uint16_t *val)
{
	char *err = NULL;
	uint32_t t;

	errno=0;
	t = strtoul(arg, &err, 0);
	if (errno || (err==arg) || ((err != NULL) && *err) || (t >= 0x10000)) {
		return -1;
	}

	*val = t & 0xFFFF;
	return 0;
}

int
str2u8(char *arg, uint8_t *val)
{
	char *err = NULL;
	uint32_t t;

	errno=0;
	t = strtoul(arg, &err, 0);
	if (errno || (err==arg) || ((err != NULL) && *err) || (t >= 0x100)) {
		return -1;
	}

	*val = t & 0xFF;
	return 0;
}

int
str2sig(char *arg, uint32_t *sig)
{
	if (strlen(arg) != 4)
		return -1;

	*sig = arg[0] | (arg[1] << 8) | (arg[2] << 16) | (arg[3] << 24);

	return 0;
}


int
parse_arg(char *arg, char *buf, char *argv[])
{
	int res = 0;
	size_t argl;
	char *tok;
	char **ap = &buf;
	int i;

	memset(argv, 0, MAX_ARG_COUNT * sizeof(void *));

	if ((arg == NULL)) {
		/* no arguments */
		return 0;
	}

	argl = strlen(arg);
	if (argl == 0) {
		/* no arguments */
		return 0;
	}

	if (argl >= MAX_ARG_LEN) {
		/* argument is too long */
		argl = MAX_ARG_LEN-1;
	}

	memcpy(buf, arg, argl);
	buf[argl] = '\0';

	for (i = 0; i < MAX_ARG_COUNT; i++) {
		tok = strsep(ap, ":");
		if (tok == NULL) {
			break;
		}
#if 0
		else if (tok[0] == '\0') {
			break;
		}
#endif
		argv[i] = tok;
		res++;
	}

	return res;
}


int
required_arg(char c, char *arg)
{
	if (arg == NULL || *arg != '-')
		return 0;

	ERR("option -%c requires an argument\n", c);
	return -1;
}


int
is_empty_arg(char *arg)
{
	int ret = 1;
	if (arg != NULL) {
		if (*arg) ret = 0;
	};
	return ret;
}


void
csum_init(struct csum_state *css)
{
	css->odd = 0;
	css->sum = 0;
	css->tmp = 0;
}


void
csum_update(void *data, uint32_t len, struct csum_state *css)
{
	uint8_t *p = data;

	if (len == 0)
		return;

	if (css->odd) {
		css->sum += (css->tmp << 8) + p[0];
		if (css->sum > 0xFFFF) {
			css->sum += 1;
			css->sum &= 0xFFFF;
		}
		css->odd = 0;
		len--;
		p++;
	}

	for ( ; len > 1; len -= 2, p +=2 ) {
		css->sum  += (p[0] << 8) + p[1];
		if (css->sum > 0xFFFF) {
			css->sum += 1;
			css->sum &= 0xFFFF;
		}
	}

	if (len == 1){
		css->tmp = p[0];
		css->odd = 1;
	}
}


uint16_t
csum_get(struct csum_state *css)
{
	char pad = 0;

	csum_update(&pad, 1, css);
	return css->sum;
}

uint16_t
csum_buf(uint8_t *p, uint32_t len)
{
	struct csum_state css;

	csum_init(&css);
	csum_update(p, len, &css);
	return csum_get(&css);

}

/*
 * routines to write data to the output file
 */
int
write_out_data(FILE *outfile, void *data, size_t len,
		struct csum_state *css)
{
	uint8_t *ptr = data;

	errno = 0;

	fwrite(ptr, len, 1, outfile);
	if (errno) {
		ERR("unable to write output file");
		return -1;
	}

	if (css) {
		csum_update(ptr, len, css);
	}

	return 0;
}


int
write_out_padding(FILE *outfile, size_t len, uint8_t padc,
		 struct csum_state *css)
{
	uint8_t buf[512];
	size_t buflen = sizeof(buf);

	memset(buf, padc, buflen);
	while (len > 0) {
		if (len < buflen)
			buflen = len;

		if (write_out_data(outfile, buf, buflen, css))
			return -1;

		len -= buflen;
	}

	return 0;
}


int
write_out_data_align(FILE *outfile, void *data, size_t len, size_t align,
		struct csum_state *css)
{
	size_t padlen;
	int res;

	res = write_out_data(outfile, data, len, css);
	if (res)
		return res;

	padlen = ALIGN(len,align) - len;
	res = write_out_padding(outfile, padlen, 0xFF, css);

	return res;
}


int
write_out_header(FILE *outfile, struct zyn_rombin_hdr *hdr)
{
	struct zyn_rombin_hdr t;

	errno = 0;
	if (fseek(outfile, 0, SEEK_SET) != 0) {
		ERRS("fseek failed on output file");
		return -1;
	}

	/* setup temporary header fields */
	memset(&t, 0, sizeof(t));
	t.addr = HOST_TO_BE32(hdr->addr);
	memcpy(&t.sig, ROMBIN_SIGNATURE, ROMBIN_SIG_LEN);
	t.type = hdr->type;
	t.flags = hdr->flags;
	t.osize = HOST_TO_BE32(hdr->osize);
	t.csize = HOST_TO_BE32(hdr->csize);
	t.ocsum = HOST_TO_BE16(hdr->ocsum);
	t.ccsum = HOST_TO_BE16(hdr->ccsum);
	t.mmap_addr = HOST_TO_BE32(hdr->mmap_addr);

	DBG(2, "hdr.addr      = 0x%08x", hdr->addr);
	DBG(2, "hdr.type      = 0x%02x", hdr->type);
	DBG(2, "hdr.osize     = 0x%08x", hdr->osize);
	DBG(2, "hdr.csize     = 0x%08x", hdr->csize);
	DBG(2, "hdr.flags     = 0x%02x", hdr->flags);
	DBG(2, "hdr.ocsum     = 0x%04x", hdr->ocsum);
	DBG(2, "hdr.ccsum     = 0x%04x", hdr->ccsum);
	DBG(2, "hdr.mmap_addr = 0x%08x", hdr->mmap_addr);

	return write_out_data(outfile, (uint8_t *)&t, sizeof(t), NULL);
}


int
write_out_mmap(FILE *outfile, struct fw_mmap *mmap, struct csum_state *css)
{
	struct zyn_mmt_hdr *mh;
	uint8_t buf[MMAP_DATA_SIZE];
	uint32_t user_size;
	char *data;
	int res;

	memset(buf, 0, sizeof(buf));

	mh = (struct zyn_mmt_hdr *)buf;

	/* TODO: needs to recreate the memory map too? */
	mh->count=0;

	/* Build user data section */
	data = (char *)buf + sizeof(*mh);
	data += sprintf(data, "Vendor 1 %d", board->vendor);
	*data++ = '\0';
	data += sprintf(data, "Model 1 %d", BE16_TO_HOST(board->model));
	*data++ = '\0';
	/* TODO: make hardware version configurable? */
	data += sprintf(data, "HwVerRange 2 %d %d", 0, 0);
	*data++ = '\0';

	user_size = (uint8_t *)data - buf;
	mh->user_start= HOST_TO_BE32(mmap->addr+sizeof(*mh));
	mh->user_end= HOST_TO_BE32(mmap->addr+user_size);
	mh->csum = HOST_TO_BE16(csum_buf(buf+sizeof(*mh), user_size));

	res = write_out_data(outfile, buf, sizeof(buf), css);

	return res;
}


int
block_stat_file(struct fw_block *block)
{
	struct stat st;
	int res;

	if (block->file_name == NULL)
		return 0;

	res = stat(block->file_name, &st);
	if (res){
		ERRS("stat failed on %s", block->file_name);
		return res;
	}

	block->file_size = st.st_size;
	return 0;
}


int
read_magic(uint16_t *magic)
{
	FILE *f;
	int res;

	errno = 0;
	f = fopen(bootext_block->file_name,"r");
	if (errno) {
		ERRS("unable to open file: %s", bootext_block->file_name);
		return -1;
	}

	errno = 0;
	fread(magic, 2, 1, f);
	if (errno != 0) {
		ERRS("unable to read from file: %s", bootext_block->file_name);
		res = -1;
		goto err;
	}

	res = 0;

err:
	fclose(f);
	return res;
}


int
write_out_file(FILE *outfile, char *name, size_t len, struct csum_state *css)
{
	char buf[FILE_BUF_LEN];
	size_t buflen = sizeof(buf);
	FILE *f;
	int res;

	DBG(2, "writing out file, name=%s, len=%zu",
		name, len);

	errno = 0;
	f = fopen(name,"r");
	if (errno) {
		ERRS("unable to open file: %s", name);
		return -1;
	}

	while (len > 0) {
		if (len < buflen)
			buflen = len;

		/* read data from source file */
		errno = 0;
		fread(buf, buflen, 1, f);
		if (errno != 0) {
			ERRS("unable to read from file: %s",name);
			res = -1;
			break;
		}

		res = write_out_data(outfile, buf, buflen, css);
		if (res)
			break;

		len -= buflen;
	}

	fclose(f);
	return res;
}


int
write_out_block(FILE *outfile, struct fw_block *block, struct csum_state *css)
{
	int res;

	if (block == NULL)
		return 0;

	if (block->file_name == NULL)
		return 0;

	if (block->file_size == 0)
		return 0;

	res = write_out_file(outfile, block->file_name,
			block->file_size, css);
	return res;
}


int
write_out_image(FILE *outfile)
{
	struct fw_block *block;
	struct fw_mmap mmap;
	struct zyn_rombin_hdr hdr;
	struct csum_state css;
	int i, res;
	uint32_t offset;
	uint32_t padlen;
	uint16_t csum;
	uint16_t t;

	/* setup header fields */
	memset(&hdr, 0, sizeof(hdr));
	hdr.addr = board->code_start;
	hdr.type = OBJECT_TYPE_BOOTEXT;
	hdr.flags = ROMBIN_FLAG_OCSUM;

	offset = board->romio_offs;

	res = write_out_header(outfile, &hdr);
	if (res)
		return res;

	offset += sizeof(hdr);

	csum_init(&css);
	res = write_out_block(outfile, bootext_block, &css);
	if (res)
		return res;

	offset += bootext_block->file_size;
	if (offset > (board->romio_offs + board->bootext_size)) {
		ERR("bootext file '%s' is too big", bootext_block->file_name);
		return -1;
	}

	padlen = ALIGN(offset, MMAP_ALIGN) - offset;
	res = write_out_padding(outfile, padlen, 0xFF, &css);
	if (res)
		return res;

	offset += padlen;

	mmap.addr = board->flash_base + offset;
	res = write_out_mmap(outfile, &mmap, &css);
	if (res)
		return res;

	offset += MMAP_DATA_SIZE;

	if ((offset - board->romio_offs) < board->bootext_size) {
		padlen = board->romio_offs + board->bootext_size - offset;
		res = write_out_padding(outfile, padlen, 0xFF, &css);
		if (res)
			return res;
		offset += padlen;

		DBG(2, "bootext end at %08x", offset);
	}

	for (i = 0; i < num_blocks; i++) {
		block = &blocks[i];

		if (block->type == BLOCK_TYPE_BOOTEXT)
			continue;

		padlen = ALIGN(offset, block->align) - offset;
		res = write_out_padding(outfile, padlen, 0xFF, &css);
		if (res)
			return res;
		offset += padlen;

		res = write_out_block(outfile, block, &css);
		if (res)
			return res;
		offset += block->file_size;
	}

	padlen = ALIGN(offset, 4) - offset;
	res = write_out_padding(outfile, padlen, 0xFF, &css);
	if (res)
		return res;
	offset += padlen;

	csum = csum_get(&css);
	hdr.mmap_addr = mmap.addr;
	hdr.osize = 2;

	res = read_magic(&hdr.ocsum);
	if (res)
		return res;
	hdr.ocsum = BE16_TO_HOST(hdr.ocsum);

	if (csum <= hdr.ocsum)
		t = hdr.ocsum - csum;
	else
		t = hdr.ocsum - csum - 1;

	DBG(2, "ocsum=%04x, csum=%04x, fix=%04x", hdr.ocsum, csum, t);

	t = HOST_TO_BE16(t);
	res = write_out_data(outfile, (uint8_t *)&t, 2, NULL);
	if (res)
		return res;


	res = write_out_header(outfile, &hdr);

	return res;
}


struct board_info *
find_board(char *name)
{
	struct board_info *ret;
	struct board_info *board;

	ret = NULL;
	for (board = boards; board->name != NULL; board++){
		if (strcasecmp(name, board->name) == 0) {
			ret = board;
			break;
		}
	};

	return ret;
}


int
parse_opt_board(char ch, char *arg)
{

	DBG(1,"parsing board option: -%c %s", ch, arg);

	if (board != NULL) {
		ERR("only one board option allowed");
		return -1;
	}

	if (required_arg(ch, arg))
		return -1;

	board = find_board(arg);
	if (board == NULL){
		ERR("invalid/unknown board specified: %s", arg);
		return -1;
	}

	return 0;
}


int
parse_opt_ofname(char ch, char *arg)
{

	if (ofname != NULL) {
		ERR("only one output file allowed");
		return -1;
	}

	if (required_arg(ch, arg))
		return -1;

	ofname = arg;

	return 0;
}


int
parse_opt_block(char ch, char *arg)
{
	char buf[MAX_ARG_LEN];
	char *argv[MAX_ARG_COUNT];
	char *p;
	struct fw_block *block;
	int i;

	if ( num_blocks >= MAX_NUM_BLOCKS ) {
		ERR("too many blocks specified");
		return -1;
	}

	block = &blocks[num_blocks++];

	/* setup default field values */
	block->padc = 0xFF;

	switch(ch) {
	case 'b':
		if (bootext_block) {
			ERR("only one boot extension block allowed");
			break;
		}
		block->type = BLOCK_TYPE_BOOTEXT;
		bootext_block = block;
		break;
	case 'r':
		block->type = BLOCK_TYPE_RAW;
		break;
	}

	parse_arg(arg, buf, argv);

	i = 0;
	p = argv[i++];
	if (is_empty_arg(p)) {
		ERR("no file specified in %s", arg);
		return -1;
	} else {
		block->file_name = strdup(p);
		if (block->file_name == NULL) {
			ERR("not enough memory");
			return -1;
		}
	}

	if (block->type == BLOCK_TYPE_BOOTEXT)
		return 0;

	p = argv[i++];
	if (!is_empty_arg(p)) {
		if (str2u32(p, &block->align) != 0) {
			ERR("invalid block align in %s", arg);
			return -1;
		}
	}

	return 0;
}


int
calc_block_offsets(int type, uint32_t *offset)
{
	struct fw_block *block;
	uint32_t next_offs;
	uint32_t avail;
	int i, res;

	DBG(1,"calculating block offsets, starting with %" PRIu32,
		*offset);

	res = 0;
	for (i = 0; i < num_blocks; i++) {
		block = &blocks[i];

		if (block->type != type)
			continue;

		next_offs = ALIGN(*offset, block->align);
		avail = board->flash_size - next_offs;
		if (block->file_size > avail) {
			ERR("file %s is too big, offset = %u, size=%u,"
				" avail = %u, align = %u", block->file_name,
				(unsigned)next_offs,
				(unsigned)block->file_size,
				(unsigned)avail,
				(unsigned)block->align);
			res = -1;
			break;
		}

		block->padlen = next_offs - *offset;
		*offset += block->file_size;
	}

	return res;
}

int
process_blocks(void)
{
	struct fw_block *block;
	uint32_t offset;
	int i;
	int res;

	/* collecting file stats */
	for (i = 0; i < num_blocks; i++) {
		block = &blocks[i];
		res = block_stat_file(block);
		if (res)
			return res;
	}

	offset = board->romio_offs + bootext_block->file_size;
	res = calc_block_offsets(BLOCK_TYPE_RAW, &offset);

	return res;
}


int
main(int argc, char *argv[])
{
	int optinvalid = 0;   /* flag for invalid option */
	int c;
	int res = EXIT_FAILURE;

	FILE *outfile;

	progname=basename(argv[0]);

	opterr = 0;  /* could not print standard getopt error messages */
	while ( 1 ) {
		optinvalid = 0;

		c = getopt(argc, argv, "b:B:ho:r:v");
		if (c == -1)
			break;

		switch (c) {
		case 'b':
		case 'r':
			optinvalid = parse_opt_block(c,optarg);
			break;
		case 'B':
			optinvalid = parse_opt_board(c,optarg);
			break;
		case 'o':
			optinvalid = parse_opt_ofname(c,optarg);
			break;
		case 'v':
			verblevel++;
			break;
		case 'h':
			usage(EXIT_SUCCESS);
			break;
		default:
			optinvalid = 1;
			break;
		}
		if (optinvalid != 0 ) {
			ERR("invalid option: -%c", optopt);
			goto out;
		}
	}

	if (board == NULL) {
		ERR("no board specified");
		goto out;
	}

	if (ofname == NULL) {
		ERR("no output file specified");
		goto out;
	}

	if (optind < argc) {
		ERR("invalid option: %s", argv[optind]);
		goto out;
	}

	if (process_blocks() != 0) {
		goto out;
	}

	outfile = fopen(ofname, "w");
	if (outfile == NULL) {
		ERRS("could not open \"%s\" for writing", ofname);
		goto out;
	}

	if (write_out_image(outfile) != 0)
		goto out_flush;

	DBG(1,"Image file %s completed.", ofname);

	res = EXIT_SUCCESS;

out_flush:
	fflush(outfile);
	fclose(outfile);
	if (res != EXIT_SUCCESS) {
		unlink(ofname);
	}
out:
	return res;
}

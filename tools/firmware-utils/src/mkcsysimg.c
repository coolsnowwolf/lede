/*
 *
 *  Copyright (C) 2007-2009 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program was based on the code found in various Linux
 *  source tarballs released by Edimax for it's devices.
 *  Original author: David Hsu <davidhsu@realtek.com.tw>
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the
 *  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA  02110-1301, USA.
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
#include <endian.h>     /* for __BYTE_ORDER */
#if defined(__CYGWIN__)
#  include <byteswap.h>
#endif

#include "csysimg.h"

#if (__BYTE_ORDER == __LITTLE_ENDIAN)
#  define HOST_TO_LE16(x)	(x)
#  define HOST_TO_LE32(x)	(x)
#  define LE16_TO_HOST(x)	(x)
#  define LE32_TO_HOST(x)	(x)
#else
#  define HOST_TO_LE16(x)	bswap_16(x)
#  define HOST_TO_LE32(x)	bswap_32(x)
#  define LE16_TO_HOST(x)	bswap_16(x)
#  define LE32_TO_HOST(x)	bswap_32(x)
#endif

#define MAX_NUM_BLOCKS	8
#define MAX_ARG_COUNT	32
#define MAX_ARG_LEN	1024
#define FILE_BUF_LEN	(16*1024)
#define CSYS_PADC	0xFF

#define BLOCK_TYPE_BOOT	0
#define BLOCK_TYPE_CONF	1
#define BLOCK_TYPE_WEBP	2
#define BLOCK_TYPE_CODE	3
#define BLOCK_TYPE_XTRA	4

#define DEFAULT_BLOCK_ALIGN	0x10000U

#define CSUM_SIZE_NONE	0
#define CSUM_SIZE_8	1
#define CSUM_SIZE_16	2


struct csum_state{
	int	size;
	uint16_t val;
	uint16_t tmp;
	int	odd;
};


struct csys_block {
	int		type;	/* type of the block */

	int		need_file;
	char		*file_name;	/* name of the file */
	uint32_t	file_size;	/* length of the file */

	unsigned char	sig[SIG_LEN];
	uint32_t	addr;
	int		addr_set;
	uint32_t	align;
	int		align_set;
	uint8_t		padc;

	uint32_t	size;
	uint32_t	size_hdr;
	uint32_t	size_csum;
	uint32_t	size_avail;

	struct csum_state *css;
};


struct board_info {
	char *model;
	char *name;
	uint32_t flash_size;

	char sig_boot[SIG_LEN];
	char sig_conf[SIG_LEN];
	char sig_webp[SIG_LEN];

	uint32_t boot_size;
	uint32_t conf_size;
	uint32_t webp_size;
	uint32_t webp_size_max;
	uint32_t code_size;

	uint32_t addr_code;
	uint32_t addr_webp;
};

#define BOARD(m, n, f, sigb, sigw, bs, cs, ws, ac, aw) {\
	.model = m, .name = n, .flash_size = f<<20, \
	.sig_boot = sigb, .sig_conf = SIG_CONF, .sig_webp = sigw, \
	.boot_size = bs, .conf_size = cs, \
	.webp_size = ws, .webp_size_max = 3*0x10000, \
	.addr_code = ac, .addr_webp = aw \
	}

#define BOARD_ADM(m,n,f, sigw) BOARD(m,n,f, ADM_BOOT_SIG, sigw, \
	ADM_BOOT_SIZE, ADM_CONF_SIZE, ADM_WEBP_SIZE, \
	ADM_CODE_ADDR, ADM_WEBP_ADDR)


/*
 * Globals
 */
char *progname;
char *ofname = NULL;
int verblevel = 0;
int invalid_causes_error = 1;
int keep_invalid_images = 0;

struct board_info *board = NULL;

struct csys_block *boot_block = NULL;
struct csys_block *conf_block = NULL;
struct csys_block *webp_block = NULL;
struct csys_block *code_block = NULL;

struct csys_block blocks[MAX_NUM_BLOCKS];
int num_blocks = 0;

static struct board_info boards[] = {
	/* The original Edimax products */
	BOARD_ADM("BR-6104K", "Edimax BR-6104K", 2, SIG_BR6104K),
	BOARD_ADM("BR-6104KP", "Edimax BR-6104KP", 2, SIG_BR6104KP),
	BOARD_ADM("BR-6104Wg", "Edimax BR-6104Wg", 2, SIG_BR6104Wg),
	BOARD_ADM("BR-6114WG", "Edimax BR-6114WG", 2, SIG_BR6114WG),
	BOARD_ADM("BR-6524K", "Edimax BR-6524K", 2, SIG_BR6524K),
	BOARD_ADM("BR-6524KP", "Edimax BR-6524KP", 2, SIG_BR6524KP),
	BOARD_ADM("BR-6524N", "Edimax BR-6524N", 2, SIG_BR6524N),
	BOARD_ADM("BR-6524WG", "Edimax BR-6524WG", 4, SIG_BR6524WG),
	BOARD_ADM("BR-6524WP", "Edimax BR-6524WP", 4, SIG_BR6524WP),
	BOARD_ADM("BR-6541K", "Edimax BR-6541K", 2, SIG_BR6541K),
	BOARD_ADM("BR-6541KP", "Edimax BR-6541K", 2, SIG_BR6541KP),
	BOARD_ADM("BR-6541WP", "Edimax BR-6541WP", 4, SIG_BR6541WP),
	BOARD_ADM("EW-7207APg", "Edimax EW-7207APg", 2, SIG_EW7207APg),
	BOARD_ADM("PS-1205UWg", "Edimax PS-1205UWg", 2, SIG_PS1205UWg),
	BOARD_ADM("PS-3205U", "Edimax PS-3205U", 2, SIG_PS3205U),
	BOARD_ADM("PS-3205UWg", "Edimax PS-3205UWg", 2, SIG_PS3205UWg),

	/* Hawking products */
	BOARD_ADM("H2BR4", "Hawking H2BR4", 2, SIG_H2BR4),
	BOARD_ADM("H2WR54G", "Hawking H2WR54G", 4, SIG_H2WR54G),

	/* Planet products */
	BOARD_ADM("XRT-401D", "Planet XRT-401D", 2, SIG_XRT401D),
	BOARD_ADM("XRT-402D", "Planet XRT-402D", 2, SIG_XRT402D),

	/* Conceptronic products */
	BOARD_ADM("C54BSR4", "Conceptronic C54BSR4", 2, SIG_C54BSR4),

	/* OSBRiDGE products */
	BOARD_ADM("5GXi", "OSBDRiDGE 5GXi", 2, SIG_5GXI),

	{.model = NULL}
};

/*
 * Message macros
 */
#define ERR(fmt, ...) do { \
	fflush(0); \
	fprintf(stderr, "[%s] *** error: " fmt "\n", progname, ## __VA_ARGS__ ); \
} while (0)

#define ERRS(fmt, ...) do { \
	int save = errno; \
	fflush(0); \
	fprintf(stderr, "[%s] *** error: " fmt ": %s\n", progname, ## __VA_ARGS__ \
		, strerror(save)); \
} while (0)

#define WARN(fmt, ...) do { \
	fprintf(stderr, "[%s] *** warning: " fmt "\n", progname, ## __VA_ARGS__ ); \
} while (0)

#define DBG(lev, fmt, ...) do { \
	if (verblevel < lev) \
		break;\
	fprintf(stderr, "[%s] " fmt "\n", progname, ## __VA_ARGS__ ); \
} while (0)

#define ERR_FATAL		-1
#define ERR_INVALID_IMAGE	-2

void
usage(int status)
{
	FILE *stream = (status != EXIT_SUCCESS) ? stderr : stdout;
	struct board_info *board;

	fprintf(stream, "Usage: %s [OPTIONS...] <file>\n", progname);
	fprintf(stream,
"\n"
"Options:\n"
"  -B <board>      create image for the board specified with <board>.\n"
"                  valid <board> values:\n"
	);
	for (board = boards; board->model != NULL; board++){
		fprintf(stream,
"                  %-12s: %s\n",
		 board->model, board->name);
	};
	fprintf(stream,
"  -d              don't throw error on invalid images\n"
"  -k              keep invalid images\n"
"  -b <file>[:<align>[:<padc>]]\n"
"                  add boot code to the image\n"
"  -c <file>[:<align>[:<padc>]]\n"
"                  add configuration settings to the image\n"
"  -r <file>:[<addr>][:<align>[:<padc>]]\n"
"                  add runtime code to the image\n"
"  -w [<file>:[<addr>][:<align>[:<padc>]]]\n"
"                  add webpages to the image\n"
"  -x <file>[:<align>[:<padc>]]\n"
"                  add extra data at the end of the image\n"
"  -h              show this screen\n"
"Parameters:\n"
"  <file>          write output to the file <file>\n"
	);

	exit(status);
}

static inline uint32_t align(uint32_t base, uint32_t alignment)
{
	uint32_t ret;

	if (alignment) {
		ret = (base + alignment - 1);
		ret &= ~(alignment-1);
	} else {
		ret = base;
	}

	return ret;
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
	return ERR_FATAL;
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
csum8_update(uint8_t *p, uint32_t len, struct csum_state *css)
{
	for ( ; len > 0; len --) {
		css->val += *p++;
	}
}


uint16_t
csum8_get(struct csum_state *css)
{
	uint8_t t;

	t = css->val;
	return ~t + 1;
}


void
csum16_update(void *data, uint32_t len, struct csum_state *css)
{
	uint8_t *p = data;
	uint16_t t;

	if (css->odd) {
		t = css->tmp + (p[0]<<8);
		css->val += LE16_TO_HOST(t);
		css->odd = 0;
		len--;
		p++;
	}

	for ( ; len > 1; len -= 2, p +=2 ) {
		t = p[0] + (p[1] << 8);
		css->val += LE16_TO_HOST(t);
	}

	if (len == 1) {
		css->tmp = p[0];
		css->odd = 1;
	}
}


uint16_t
csum16_get(struct csum_state *css)
{
	char pad = 0;

	csum16_update(&pad, 1, css);
	return ~css->val + 1;
}


void
csum_init(struct csum_state *css, int size)
{
	css->val = 0;
	css->tmp = 0;
	css->odd = 0;
	css->size = size;
}


void
csum_update(void *data, uint32_t len, struct csum_state *css)
{
	uint8_t *p = data;

	switch (css->size) {
	case CSUM_SIZE_8:
		csum8_update(p,len,css);
		break;
	case CSUM_SIZE_16:
		csum16_update(p,len,css);
		break;
	}
}


uint16_t
csum_get(struct csum_state *css)
{
	uint16_t ret;

	switch (css->size) {
	case CSUM_SIZE_8:
		ret = csum8_get(css);
		break;
	case CSUM_SIZE_16:
		ret = csum16_get(css);
		break;
	default:
		ERR("invalid checksum size\n");
		return 0;
	}

	return ret;
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
		ERRS("unable to write output file");
		return ERR_FATAL;
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
	int err;

	memset(buf, padc, buflen);
	while (len > 0) {
		if (len < buflen)
			buflen = len;

		err = write_out_data(outfile, buf, buflen, css);
		if (err)
			return err;

		len -= buflen;
	}

	return 0;
}


int
block_stat_file(struct csys_block *block)
{
	struct stat st;
	int err;

	if (block->file_name == NULL)
		return 0;

	err = stat(block->file_name, &st);
	if (err){
		ERRS("stat failed on %s", block->file_name);
		return ERR_FATAL;
	}

	block->file_size = st.st_size;
	return 0;
}


int
block_writeout_hdr(FILE *outfile, struct csys_block *block)
{
	struct csys_header hdr;
	int res;

	if (block->size_hdr == 0)
		return 0;

	/* setup header fields */
	memcpy(hdr.sig, block->sig, 4);
	hdr.addr = HOST_TO_LE32(block->addr);
	hdr.size = HOST_TO_LE32(block->size - block->size_hdr - block->size_csum);

	DBG(1,"writing header for block");
	res = write_out_data(outfile, (uint8_t *)&hdr, sizeof(hdr),NULL);
	return res;

}


int
block_writeout_file(FILE *outfile, struct csys_block *block)
{
	char buf[FILE_BUF_LEN];
	size_t buflen = sizeof(buf);
	FILE *f;
	size_t len;
	int res;

	if (block->file_name == NULL)
		return 0;

	if (block->file_size == 0)
		return 0;

	errno = 0;
	f = fopen(block->file_name,"r");
	if (errno) {
		ERRS("unable to open file: %s", block->file_name);
		return ERR_FATAL;
	}

	len = block->file_size;
	while (len > 0) {
		if (len < buflen)
			buflen = len;

		/* read data from source file */
		errno = 0;
		fread(buf, buflen, 1, f);
		if (errno != 0) {
			ERRS("unable to read from file: %s", block->file_name);
			res = ERR_FATAL;
			break;
		}

		res = write_out_data(outfile, buf, buflen, block->css);
		if (res)
			break;

		len -= buflen;
	}

	fclose(f);
	return res;
}


int
block_writeout_data(FILE *outfile, struct csys_block *block)
{
	int res;
	size_t padlen;

	res = block_writeout_file(outfile, block);
	if (res)
		return res;

	/* write padding data if neccesary */
	padlen = block->size_avail - block->file_size;
	DBG(1,"padding block, length=%zu", padlen);
	res = write_out_padding(outfile, padlen, block->padc, block->css);

	return res;
}


int
block_writeout_csum(FILE *outfile, struct csys_block *block)
{
	uint16_t csum;
	int res;

	if (block->size_csum == 0)
		return 0;

	DBG(1,"writing checksum for block");
	csum = HOST_TO_LE16(csum_get(block->css));
	res = write_out_data(outfile, (uint8_t *)&csum, block->size_csum, NULL);

	return res;
}


int
block_writeout(FILE *outfile, struct csys_block *block)
{
	int res;
	struct csum_state css;

	res = 0;

	if (block == NULL)
		return res;

	block->css = NULL;

	DBG(2, "writing block, file=%s, file_size=%d, space=%d",
		block->file_name, block->file_size, block->size_avail);
	res = block_writeout_hdr(outfile, block);
	if (res)
		return res;

	if (block->size_csum != 0) {
		block->css = &css;
		csum_init(&css, block->size_csum);
	}

	res = block_writeout_data(outfile, block);
	if (res)
		return res;

	res = block_writeout_csum(outfile, block);
	if (res)
		return res;

	return res;
}


int
write_out_blocks(FILE *outfile)
{
	struct csys_block *block;
	int i, res;

	res = block_writeout(outfile, boot_block);
	if (res)
		return res;

	res = block_writeout(outfile, conf_block);
	if (res)
		return res;

	res = block_writeout(outfile, webp_block);
	if (res)
		return res;

	res = block_writeout(outfile, code_block);
	if (res)
		return res;

	res = 0;
	for (i=0; i < num_blocks; i++) {
		block = &blocks[i];

		if (block->type != BLOCK_TYPE_XTRA)
			continue;

		res = block_writeout(outfile, block);
		if (res)
			break;
	}

	return res;
}


struct board_info *
find_board(char *model)
{
	struct board_info *ret;
	struct board_info *board;

	ret = NULL;
	for (board = boards; board->model != NULL; board++){
		if (strcasecmp(model, board->model) == 0) {
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
		return ERR_FATAL;
	}

	if (required_arg(ch, arg))
		return ERR_FATAL;

	board = find_board(arg);
	if (board == NULL){
		ERR("invalid/unknown board specified: %s", arg);
		return ERR_FATAL;
	}

	return 0;
}


int
parse_opt_block(char ch, char *arg)
{
	char buf[MAX_ARG_LEN];
	char *argv[MAX_ARG_COUNT];
	char *p;
	struct csys_block *block;
	int i;

	if ( num_blocks > MAX_NUM_BLOCKS ) {
		ERR("too many blocks specified");
		return ERR_FATAL;
	}

	block = &blocks[num_blocks];

	/* setup default field values */
	block->need_file = 1;
	block->padc = 0xFF;

	switch (ch) {
	case 'b':
		if (boot_block) {
			WARN("only one boot block allowed");
			break;
		}
		block->type = BLOCK_TYPE_BOOT;
		boot_block = block;
		break;
	case 'c':
		if (conf_block) {
			WARN("only one config block allowed");
			break;
		}
		block->type = BLOCK_TYPE_CONF;
		conf_block = block;
		break;
	case 'w':
		if (webp_block) {
			WARN("only one web block allowed");
			break;
		}
		block->type = BLOCK_TYPE_WEBP;
		block->size_hdr = sizeof(struct csys_header);
		block->size_csum = CSUM_SIZE_8;
		block->need_file = 0;
		webp_block = block;
		break;
	case 'r':
		if (code_block) {
			WARN("only one runtime block allowed");
			break;
		}
		block->type = BLOCK_TYPE_CODE;
		block->size_hdr = sizeof(struct csys_header);
		block->size_csum = CSUM_SIZE_16;
		code_block = block;
		break;
	case 'x':
		block->type = BLOCK_TYPE_XTRA;
		break;
	default:
		ERR("unknown block type \"%c\"", ch);
		return ERR_FATAL;
	}

	parse_arg(arg, buf, argv);

	i = 0;
	p = argv[i++];
	if (!is_empty_arg(p)) {
		block->file_name = strdup(p);
		if (block->file_name == NULL) {
			ERR("not enough memory");
			return ERR_FATAL;
		}
	} else if (block->need_file){
		ERR("no file specified in %s", arg);
		return ERR_FATAL;
	}

	if (block->size_hdr) {
		p = argv[i++];
		if (!is_empty_arg(p)) {
			if (str2u32(p, &block->addr) != 0) {
				ERR("invalid start address in %s", arg);
				return ERR_FATAL;
			}
			block->addr_set = 1;
		}
	}

	p = argv[i++];
	if (!is_empty_arg(p)) {
		if (str2u32(p, &block->align) != 0) {
			ERR("invalid alignment value in %s", arg);
			return ERR_FATAL;
		}
		block->align_set = 1;
	}

	p = argv[i++];
	if (!is_empty_arg(p) && (str2u8(p, &block->padc) != 0)) {
		ERR("invalid paddig character in %s", arg);
		return ERR_FATAL;
	}

	num_blocks++;

	return 0;
}


int
process_blocks(void)
{
	struct csys_block *block;
	uint32_t offs = 0;
	int i;
	int res;

	res = 0;
	/* collecting stats */
	for (i = 0; i < num_blocks; i++) {
		block = &blocks[i];
		res = block_stat_file(block);
		if (res)
			return res;
	}

	/* bootloader */
	block = boot_block;
	if (block) {
		block->size = board->boot_size;
		if (block->file_size > board->boot_size) {
			WARN("boot block is too big");
			res = ERR_INVALID_IMAGE;
		}
	}
	offs += board->boot_size;

	/* configuration data */
	block = conf_block;
	if (block) {
		block->size = board->conf_size;
		if (block->file_size > board->conf_size) {
			WARN("config block is too big");
			res = ERR_INVALID_IMAGE;
		}
	}
	offs += board->conf_size;

	/* webpages */
	block = webp_block;
	if (block) {

		memcpy(block->sig, board->sig_webp, 4);

		if (block->addr_set == 0)
			block->addr = board->addr_webp;

		if (block->align_set == 0)
			block->align = DEFAULT_BLOCK_ALIGN;

		block->size = align(offs + block->file_size + block->size_hdr +
				block->size_csum, block->align) - offs;

		if (block->size > board->webp_size_max) {
			WARN("webpages block is too big");
			res = ERR_INVALID_IMAGE;
		}

		DBG(2,"webpages start at %08x, size=%08x", offs,
				block->size);

		offs += block->size;
		if (offs > board->flash_size) {
			WARN("webp block is too big");
			res = ERR_INVALID_IMAGE;
		}
	}

	/* runtime code */
	block = code_block;
	if (block) {
		memcpy(code_block->sig, SIG_CSYS, 4);

		if (block->addr_set == 0)
			block->addr = board->addr_code;

		if (block->align_set == 0)
			block->align = DEFAULT_BLOCK_ALIGN;

		block->size = align(offs + block->file_size +
				block->size_hdr + block->size_csum,
				block->align) - offs;

		DBG(2,"code block start at %08x, size=%08x", offs,
				block->size);

		offs += block->size;
		if (offs > board->flash_size) {
			WARN("code block is too big");
			res = ERR_INVALID_IMAGE;
		}
	}

	for (i = 0; i < num_blocks; i++) {
		block = &blocks[i];

		if (block->type != BLOCK_TYPE_XTRA)
			continue;

		if (block->align_set == 0)
			block->align = DEFAULT_BLOCK_ALIGN;

		block->size = align(offs + block->file_size,
				block->align) - offs;

		DBG(2,"file %s start at %08x, size=%08x, align=%08x",
			block->file_name, offs, block->size, block->align);

		offs += block->size;
		if (offs > board->flash_size) {
			WARN("file %s is too big, size=%d, avail=%d",
				block->file_name, block->file_size,
				board->flash_size - offs);
			res = ERR_INVALID_IMAGE;
		}
	}

	for (i = 0; i < num_blocks; i++) {
		block = &blocks[i];

		block->size_avail = block->size - block->size_hdr -
			block->size_csum;

		if (block->size_avail < block->file_size) {
			WARN("file %s is too big, size=%d, avail=%d",
				block->file_name, block->file_size,
				block->size_avail);
			res = ERR_INVALID_IMAGE;
		}
	}

	return res;
}


int
main(int argc, char *argv[])
{
	int optinvalid = 0;   /* flag for invalid option */
	int c;
	int res = ERR_FATAL;

	FILE *outfile;

	progname=basename(argv[0]);

	opterr = 0;  /* could not print standard getopt error messages */
	while ( 1 ) {
		optinvalid = 0;

		c = getopt(argc, argv, "b:B:c:dhkr:vw:x:");
		if (c == -1)
			break;

		switch (c) {
		case 'b':
		case 'c':
		case 'r':
		case 'x':
			optinvalid = parse_opt_block(c,optarg);
			break;
		case 'w':
			if (optarg != NULL && *optarg == '-') {
				/* rollback */
				optind--;
				optarg = NULL;
			}
			optinvalid = parse_opt_block(c,optarg);
			break;
		case 'd':
			invalid_causes_error = 0;
			break;
		case 'k':
			keep_invalid_images = 1;
			break;
		case 'B':
			optinvalid = parse_opt_board(c,optarg);
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
		if (optinvalid != 0 ){
			ERR("invalid option: -%c", optopt);
			goto out;
		}
	}

	if (board == NULL) {
		ERR("no board specified");
		goto out;
	}

	if (optind == argc) {
		ERR("no output file specified");
		goto out;
	}

	ofname = argv[optind++];

	if (optind < argc) {
		ERR("invalid option: %s", argv[optind]);
		goto out;
	}

	res = process_blocks();
	if (res == ERR_FATAL)
		goto out;

	if (res == ERR_INVALID_IMAGE) {
		if (invalid_causes_error)
			res = ERR_FATAL;

		if (keep_invalid_images == 0) {
			WARN("generation of invalid images \"%s\" disabled", ofname);
			goto out;
		}

		WARN("generating invalid image: \"%s\"", ofname);
	}

	outfile = fopen(ofname, "w");
	if (outfile == NULL) {
		ERRS("could not open \"%s\" for writing", ofname);
		res = ERR_FATAL;
		goto out;
	}

	if (write_out_blocks(outfile) != 0) {
		res = ERR_FATAL;
		goto out_flush;
	}

	DBG(1,"Image file %s completed.", ofname);

out_flush:
	fflush(outfile);
	fclose(outfile);
	if (res == ERR_FATAL) {
		unlink(ofname);
	}
out:
	if (res == ERR_FATAL)
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}

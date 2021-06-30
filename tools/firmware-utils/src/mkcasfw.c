/*
 *
 *  Copyright (C) 2007 OpenWrt.org
 *  Copyright (C) 2007 Gabor Juhos <juhosg at openwrt.org>
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
#include <endian.h>     /* for __BYTE_ORDER */
#if defined(__CYGWIN__)
#  include <byteswap.h>
#endif

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

#define MAX_NUM_BLOCKS	2
#define MAX_ARG_COUNT	32
#define MAX_ARG_LEN	1024
#define FILE_BUF_LEN	(16*1024)
#define DEFAULT_PADC	0xFF

#define DEFAULT_BLOCK_ALIGN	0x10000U

#define CSUM_TYPE_NONE	0
#define CSUM_TYPE_8	1
#define CSUM_TYPE_16	2
#define CSUM_TYPE_32	3

struct csum_state{
	int	size;
	uint32_t val;
	uint32_t tmp;
	int	odd;
};

struct image_desc {
	int		need_file;
	char		*file_name;	/* name of the file */
	uint32_t	file_size;	/* length of the file */

	uint32_t	csum;
	uint32_t	out_size;
	uint8_t		padc;
};

struct fwhdr_nfs {
	uint32_t	type;
	uint32_t	kernel_offs;
	uint32_t	kernel_size;
	uint32_t	fs_offs;
	uint32_t	fs_size;
	uint32_t	kernel_csum;
	uint32_t	fs_csum;
	uint32_t	id;
} __attribute__ ((packed));

struct fwhdr_cas {
	uint32_t	type;
	uint32_t	kernel_offs;
	uint32_t	kernel_size;
	uint32_t	id;
	uint32_t	kernel_csum;
	uint32_t	magic1;
	uint32_t	magic2;
	uint32_t	magic3;
} __attribute__ ((packed));

union file_hdr {
	struct fwhdr_cas cas;
	struct fwhdr_nfs nfs;
};

struct board_info {
	char		*model;
	char		*name;
	int		header_type;
	uint32_t	id;
	uint32_t	max_kernel_size;
	uint32_t	max_fs_size;
};

#define HEADER_TYPE_NFS		0
#define HEADER_TYPE_CAS		1

#define KERNEL_SIZE_CAS		(61*64*1024)
#define KERNEL_SIZE_NFS		(52*64*1024)
#define FS_SIZE_NFS		(9*64*1024)

#define CAS_MAGIC1	0x5241AA55
#define CAS_MAGIC2	0x524F4741
#define CAS_MAGIC3	0xD3F22D4E

/* Cellvision/SparkLAN products */
#define MODEL_CAS_630		0x01000000
#define MODEL_CAS_630W		0x01000000
#define MODEL_CAS_670		0x01000000
#define MODEL_CAS_670W		0x01000000
#define MODEL_NFS_101U		0x01000000
#define MODEL_NFS_101WU		0x01000003
#define MODEL_NFS_202U		0x01000001
#define MODEL_NFS_202WU		0x01000002

/* Corega products */
#define MODEL_CG_NSADP		0x01000020 /* NFS-101U */
#define MODEL_CG_NSADPCR	0x01000021 /* NFS-202U */

/* D-Link products */
#define MODEL_DCS_950		0x01010102 /* CAS-630 */
#define MODEL_DCS_950G		0x01020102 /* CAS-630W */
#define MODEL_DNS_120		0x01000030 /* NFS-101U */
#define MODEL_DNS_G120		0x01000032 /* NFS-101WU */

/* Digitus products */
#define MODEL_DN_16021		MODEL_CAS_630
#define MODEL_DN_16022		MODEL_CAS_630W
#define MODEL_DN_16030		MODEL_CAS_670
#define MODEL_DN_16031		MODEL_CAS_670W
#define MODEL_DN_7013		MODEL_NFS_101U

/* Lobos products */
#define MODEL_LB_SS01TXU	0x00000000

/* Neu-Fusion products */

/* Ovislink products */
#define MODEL_MU_5000FS		0x01000040 /* NFS-101U */
#define MODEL_WL_5420CAM	0x020B0101 /* CAS-630W? */
#define MODEL_WL_5460CAM	0x020B0001 /* CAS-670W */

/* Repotec products */

/* Sitecom products */
#define MODEL_LN_350		/* unknown */
#define MODEL_LN_403		0x01020402
#define MODEL_WL_401		0x01010402

/* Surecom products */
#define MODEL_EP_4001_MM	0x01030A02 /* CAS-630 */
#define MODEL_EP_4002_MM	0x01020A02 /* CAS-630W */
#define MODEL_EP_4011_MM	0x01010A02 /* CAS-670 */
#define MODEL_EP_4012_MM	0x01000A02 /* CAS-670W */
#define MODEL_EP_9812_U		/* unknown */

/* Trendnet products */
#define MODEL_TN_U100		0x01000081 /* NFS-101U */
#define MODEL_TN_U200		0x01000082 /* NFS-202U */

/*
 * Globals
 */
char *progname;
char *ofname;
int verblevel;
int keep_invalid_images;
int invalid_causes_error = 1;
union file_hdr header;

struct image_desc kernel_image;
struct image_desc fs_image;

struct board_info *board = NULL;

#define BOARD(m, n, i, ks, fs, h) {		\
		.model = (m),			\
		.name = (n),			\
		.id = (i),			\
		.max_kernel_size = (ks), 	\
		.max_fs_size = (fs), 		\
		.header_type = (h)		\
	}

#define BOARD_CAS(m,n,i) \
		BOARD(m, n, i, KERNEL_SIZE_CAS, 0, HEADER_TYPE_CAS)
#define BOARD_NFS(m,n,i) \
		BOARD(m, n, i, KERNEL_SIZE_NFS, FS_SIZE_NFS, HEADER_TYPE_NFS)

static struct board_info boards[] = {
	/* Cellvision/Sparklan products */
	BOARD_CAS("CAS-630", "Cellvision CAS-630", MODEL_CAS_630),
	BOARD_CAS("CAS-630W", "Cellvision CAS-630W", MODEL_CAS_630W),
	BOARD_CAS("CAS-670", "Cellvision CAS-670", MODEL_CAS_670),
	BOARD_CAS("CAS-670W", "Cellvision CAS-670W", MODEL_CAS_670W),
	BOARD_NFS("NFS-101U", "Cellvision NFS-101U", MODEL_NFS_101U),
	BOARD_NFS("NFS-101WU", "Cellvision NFS-101WU", MODEL_NFS_101WU),
	BOARD_NFS("NFS-202U", "Cellvision NFS-202U", MODEL_NFS_202U),
	BOARD_NFS("NFS-202WU", "Cellvision NFS-202WU", MODEL_NFS_202WU),

	/* Corega products */
	BOARD_NFS("CG-NSADP", "Corega CG-NSADP", MODEL_CG_NSADP),
	BOARD_NFS("CG-NSADPCR", "Corega CG-NSADPCR", MODEL_CG_NSADPCR),

	/* D-Link products */
	BOARD_CAS("DCS-950", "D-Link DCS-950", MODEL_DCS_950),
	BOARD_CAS("DCS-950G", "D-Link DCS-950G", MODEL_DCS_950G),
	BOARD_NFS("DNS-120", "D-Link DNS-120", MODEL_DNS_120),
	BOARD_NFS("DNS-G120", "D-Link DNS-G120", MODEL_DNS_G120),

	/* Digitus products */
	BOARD_NFS("DN-7013", "Digitus DN-7013", MODEL_DN_7013),

	/* Lobos products */
	BOARD_NFS("LB-SS01TXU", "Lobos LB-SS01TXU", MODEL_LB_SS01TXU),

	/* Ovislink products */
	BOARD_NFS("MU-5000FS", "Ovislink MU-5000FS", MODEL_MU_5000FS),
	BOARD_CAS("WL-5420CAM", "Ovislink WL-5420 CAM", MODEL_WL_5420CAM),
	BOARD_CAS("WL-5460CAM", "Ovislink WL-5460 CAM", MODEL_WL_5460CAM),

	/* Sitecom products */
	BOARD_CAS("LN-403", "Sitecom LN-403", MODEL_LN_403),
	BOARD_CAS("WL-401", "Sitecom WL-401", MODEL_WL_401),

	/* Surecom products */
	BOARD_CAS("EP-4001-MM", "Surecom EP-4001-MM", MODEL_EP_4001_MM),
	BOARD_CAS("EP-4002-MM", "Surecom EP-4002-MM", MODEL_EP_4002_MM),
	BOARD_CAS("EP-4011-MM", "Surecom EP-4011-MM", MODEL_EP_4011_MM),
	BOARD_CAS("EP-4012-MM", "Surecom EP-4012-MM", MODEL_EP_4012_MM),

	/* TrendNET products */
	BOARD_NFS("TN-U100", "TrendNET TN-U100", MODEL_TN_U100),
	BOARD_NFS("TN-U200", "TrendNET TN-U200", MODEL_TN_U200),

	{.model = NULL}
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
"  -K <file>       add kernel to the image\n"
"  -C <file>       add custom filesystem to the image\n"
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
csum32_update(uint8_t *p, uint32_t len, struct csum_state *css)
{
	uint32_t t;

	for ( ; len > 3; len -= 4, p += 4 ) {
		t = p[0] + (p[1] << 8) + (p[2] << 16) + (p[3] << 24);
		css->val ^= t;
	}
}

uint32_t
csum32_get(struct csum_state *css)
{
	return css->val;
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
	case CSUM_TYPE_8:
		csum8_update(p,len,css);
		break;
	case CSUM_TYPE_16:
		csum16_update(p,len,css);
		break;
	case CSUM_TYPE_32:
		csum32_update(p,len,css);
		break;
	}
}


uint32_t
csum_get(struct csum_state *css)
{
	uint32_t ret;

	switch (css->size) {
	case CSUM_TYPE_8:
		ret = csum8_get(css);
		break;
	case CSUM_TYPE_16:
		ret = csum16_get(css);
		break;
	case CSUM_TYPE_32:
		ret = csum32_get(css);
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
image_stat_file(struct image_desc *desc)
{
	struct stat st;
	int err;

	if (desc->file_name == NULL)
		return 0;

	err = stat(desc->file_name, &st);
	if (err){
		ERRS("stat failed on %s", desc->file_name);
		return ERR_FATAL;
	}

	if (st.st_size > desc->out_size) {
		WARN("file %s is too big, will be truncated to %d bytes\n",
			desc->file_name, desc->out_size);
		desc->file_size = desc->out_size;
		return ERR_INVALID_IMAGE;
	}


	desc->file_size = st.st_size;
	desc->out_size = align(desc->file_size,1);
	return 0;
}


int
image_writeout_file(FILE *outfile, struct image_desc *desc,
			struct csum_state *css)
{
	char buf[FILE_BUF_LEN];
	size_t buflen = sizeof(buf);
	FILE *f;
	size_t len;
	int res;

	if (desc->file_name == NULL)
		return 0;

	if (desc->file_size == 0)
		return 0;

	errno = 0;
	f = fopen(desc->file_name,"r");
	if (errno) {
		ERRS("unable to open file: %s", desc->file_name);
		return ERR_FATAL;
	}

	len = desc->file_size;
	while (len > 0) {
		if (len < buflen)
			buflen = len;

		/* read data from source file */
		errno = 0;
		fread(buf, buflen, 1, f);
		if (errno != 0) {
			ERRS("unable to read from file: %s", desc->file_name);
			res = ERR_FATAL;
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
image_writeout(FILE *outfile, struct image_desc *desc)
{
	int res;
	struct csum_state css;
	size_t padlen;

	res = 0;

	if (!desc->file_size)
		return 0;

	DBG(2, "writing image, file=%s, file_size=%d\n",
		desc->file_name, desc->file_size);

	csum_init(&css, CSUM_TYPE_32);

	res = image_writeout_file(outfile, desc, &css);
	if (res)
		return res;

	/* write padding data if neccesary */
	padlen = desc->out_size - desc->file_size;
	DBG(1,"padding desc, length=%zu", padlen);
	res = write_out_padding(outfile, padlen, desc->padc, &css);

	desc->csum = csum_get(&css);

	return res;
}


int
write_out_header(FILE *outfile)
{
	union file_hdr tmp;
	int res;

	errno = 0;
	if (fseek(outfile, 0, SEEK_SET) != 0) {
		ERRS("fseek failed on output file");
		return ERR_FATAL;
	}

	switch (board->header_type) {
	case HEADER_TYPE_CAS:
		tmp.cas.type = HOST_TO_LE32(header.cas.type);
		tmp.cas.id = HOST_TO_LE32(header.cas.id);
		tmp.cas.kernel_offs = HOST_TO_LE32(sizeof(tmp.cas));
		tmp.cas.kernel_size = HOST_TO_LE32(kernel_image.out_size);
		tmp.cas.kernel_csum = HOST_TO_LE32(kernel_image.csum);
		tmp.cas.magic1 = HOST_TO_LE32(CAS_MAGIC1);
		tmp.cas.magic2 = HOST_TO_LE32(CAS_MAGIC2);
		tmp.cas.magic3 = HOST_TO_LE32(CAS_MAGIC3);
		res = write_out_data(outfile, (uint8_t *)&tmp.cas,
					sizeof(tmp.cas), NULL);
		break;
	case HEADER_TYPE_NFS:
		tmp.nfs.type = HOST_TO_LE32(header.nfs.type);
		tmp.nfs.id = HOST_TO_LE32(header.nfs.id);
		tmp.nfs.kernel_offs = HOST_TO_LE32(sizeof(tmp.nfs));
		tmp.nfs.kernel_size = HOST_TO_LE32(kernel_image.out_size);
		tmp.nfs.kernel_csum = HOST_TO_LE32(kernel_image.csum);
		tmp.nfs.fs_offs = HOST_TO_LE32(sizeof(tmp.nfs)
					+ kernel_image.out_size);
		tmp.nfs.fs_size = HOST_TO_LE32(fs_image.out_size);
		tmp.nfs.fs_csum = HOST_TO_LE32(fs_image.csum);
		res = write_out_data(outfile, (uint8_t *)&tmp.nfs,
					sizeof(tmp.nfs), NULL);
		break;
	default:
		ERR("invalid header type\n");
		return -EINVAL;
	}

	return res;
}

int
write_out_images(FILE *outfile)
{
	int res;

	res = image_writeout(outfile, &kernel_image);
	if (res)
		return res;

	res = image_writeout(outfile, &fs_image);
	if (res)
		return res;

	return 0;
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

	switch (board->header_type) {
	case HEADER_TYPE_CAS:
		header.cas.type = HEADER_TYPE_CAS;
		header.cas.id = board->id;
		break;
	case HEADER_TYPE_NFS:
		header.nfs.type = HEADER_TYPE_NFS;
		header.nfs.id = board->id;
		break;
	default:
		ERR("internal error, unknown header type\n");
		return ERR_FATAL;
	}

	return 0;
}


int
parse_opt_image(char ch, char *arg)
{
	char buf[MAX_ARG_LEN];
	char *argv[MAX_ARG_COUNT];
	char *p;
	struct image_desc *desc = NULL;
	int i;

	switch (ch) {
	case 'K':
		if (kernel_image.file_name) {
			WARN("only one kernel option allowed");
			break;
		}
		desc = &kernel_image;
		break;
	case 'F':
		if (fs_image.file_name) {
			WARN("only one fs option allowed");
			break;
		}
		desc = &fs_image;
		break;
	}

	if (!desc)
		return ERR_FATAL;

	parse_arg(arg, buf, argv);

	i = 0;
	p = argv[i++];
	if (!is_empty_arg(p)) {
		desc->file_name = strdup(p);
		if (desc->file_name == NULL) {
			ERR("not enough memory");
			return ERR_FATAL;
		}
	} else {
		ERR("no file specified for option %c", ch);
		return ERR_FATAL;
	}

	return 0;
}


int
process_images(void)
{
	int res;

	kernel_image.out_size = board->max_kernel_size;
	kernel_image.padc = DEFAULT_PADC;
	res = image_stat_file(&kernel_image);
	if (res)
		return res;

	if (!fs_image.file_name)
		return 0;

	fs_image.out_size = board->max_fs_size;
	fs_image.padc = DEFAULT_PADC;
	res = image_stat_file(&fs_image);
	if (res)
		return res;

	return 0;
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

		c = getopt(argc, argv, "B:C:dhK:r:vw:x:");
		if (c == -1)
			break;

		switch (c) {
		case 'B':
			optinvalid = parse_opt_board(c,optarg);
			break;
		case 'd':
			invalid_causes_error = 0;
			break;
		case 'C':
		case 'K':
			optinvalid = parse_opt_image(c,optarg);
			break;
		case 'k':
			keep_invalid_images = 1;
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

	res = process_images();
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

	if (write_out_header(outfile) != 0) {
		res = ERR_FATAL;
		goto out_flush;
	}

	if (write_out_images(outfile) != 0) {
		res = ERR_FATAL;
		goto out_flush;
	}

	if (write_out_header(outfile) != 0) {
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

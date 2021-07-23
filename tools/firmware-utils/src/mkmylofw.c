/*
 *  Copyright (C) 2006-2008 Gabor Juhos <juhosg@openwrt.org>
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
#else
#  define HOST_TO_LE16(x)	bswap_16(x)
#  define HOST_TO_LE32(x)	bswap_32(x)
#endif

#include "myloader.h"

#define MAX_FW_BLOCKS  	32
#define MAX_ARG_COUNT   32
#define MAX_ARG_LEN     1024
#define FILE_BUF_LEN    (16*1024)
#define PART_NAME_LEN	32

struct fw_block {
	uint32_t	addr;
	uint32_t	blocklen; /* length of the block */
	uint32_t	flags;

	char		*name;  /* name of the file */
	uint32_t	size;  	/* length of the file */
	uint32_t	crc;    /* crc value of the file */
};

struct fw_part {
	struct mylo_partition	mylo;
	char			name[PART_NAME_LEN];
};

#define BLOCK_FLAG_HAVEHDR    0x0001

struct cpx_board {
	char		*model; /* model number*/
	char		*name;	/* model name*/
	char		*desc;  /* description */
	uint16_t        vid;    /* vendor id */
	uint16_t        did;    /* device id */
	uint16_t        svid;   /* sub vendor id */
	uint16_t        sdid;   /* sub device id */
	uint32_t        flash_size;     /* size of flash */
	uint32_t	part_offset;	/* offset of the partition_table */
	uint32_t	part_size;	/* size of the partition_table */
};

#define BOARD(_vid, _did, _svid, _sdid, _flash, _mod, _name, _desc, _po, _ps) {		\
	.model = _mod, .name = _name, .desc = _desc,   			\
	.vid = _vid, .did = _did, .svid = _svid, .sdid = _sdid,         \
	.flash_size = (_flash << 20),					\
	.part_offset = _po, .part_size = _ps }

#define CPX_BOARD(_did, _flash, _mod, _name, _desc, _po, _ps) \
	BOARD(VENID_COMPEX, _did, VENID_COMPEX, _did, _flash, _mod, _name, _desc, _po, _ps)

#define CPX_BOARD_ADM(_did, _flash, _mod, _name, _desc) \
	CPX_BOARD(_did, _flash, _mod, _name, _desc, 0x10000, 0x10000)

#define CPX_BOARD_AR71XX(_did, _flash, _mod, _name, _desc) \
	CPX_BOARD(_did, _flash, _mod, _name, _desc, 0x20000, 0x8000)

#define CPX_BOARD_AR23XX(_did, _flash, _mod, _name, _desc) \
	CPX_BOARD(_did, _flash, _mod, _name, _desc, 0x10000, 0x10000)

#define ALIGN(x,y)	(((x)+((y)-1)) & ~((y)-1))

char	*progname;
char	*ofname = NULL;

uint32_t flash_size = 0;
int	fw_num_partitions = 0;
int	fw_num_blocks = 0;
int	verblevel = 0;

struct mylo_fw_header fw_header;
struct fw_part fw_parts[MYLO_MAX_PARTITIONS];
struct fw_block fw_blocks[MAX_FW_BLOCKS];
struct cpx_board *board;

struct cpx_board boards[] = {
	CPX_BOARD_ADM(DEVID_COMPEX_NP18A, 4,
		"NP18A", "Compex NetPassage 18A",
		"Dualband Wireless A+G Internet Gateway"),
	CPX_BOARD_ADM(DEVID_COMPEX_NP26G8M, 2,
		"NP26G8M", "Compex NetPassage 26G (8M)",
		"Wireless-G Broadband Multimedia Gateway"),
	CPX_BOARD_ADM(DEVID_COMPEX_NP26G16M, 4,
		"NP26G16M", "Compex NetPassage 26G (16M)",
		"Wireless-G Broadband Multimedia Gateway"),
	CPX_BOARD_ADM(DEVID_COMPEX_NP27G, 4,
		"NP27G", "Compex NetPassage 27G",
		"Wireless-G 54Mbps eXtended Range Router"),
	CPX_BOARD_ADM(DEVID_COMPEX_NP28G, 4,
		"NP28G", "Compex NetPassage 28G",
		"Wireless 108Mbps Super-G XR Multimedia Router with 4 USB Ports"),
	CPX_BOARD_ADM(DEVID_COMPEX_NP28GHS, 4,
		"NP28GHS", "Compex NetPassage 28G (HotSpot)",
		"HotSpot Solution"),
	CPX_BOARD_ADM(DEVID_COMPEX_WP18, 4,
		"WP18", "Compex NetPassage WP18",
		"Wireless-G 54Mbps A+G Dualband Access Point"),
	CPX_BOARD_ADM(DEVID_COMPEX_WP54G, 4,
		"WP54G", "Compex WP54G",
		"Wireless-G 54Mbps XR Access Point"),
	CPX_BOARD_ADM(DEVID_COMPEX_WP54Gv1C, 2,
		"WP54Gv1C", "Compex WP54G rev.1C",
		"Wireless-G 54Mbps XR Access Point"),
	CPX_BOARD_ADM(DEVID_COMPEX_WP54AG, 4,
		"WP54AG", "Compex WP54AG",
		"Wireless-AG 54Mbps XR Access Point"),
	CPX_BOARD_ADM(DEVID_COMPEX_WPP54G, 4,
		"WPP54G", "Compex WPP54G",
		"Outdoor Access Point"),
	CPX_BOARD_ADM(DEVID_COMPEX_WPP54AG, 4,
		"WPP54AG", "Compex WPP54AG",
		"Outdoor Access Point"),

	CPX_BOARD_AR71XX(DEVID_COMPEX_WP543, 2,
		"WP543", "Compex WP543",
		"BareBoard"),
	CPX_BOARD_AR71XX(DEVID_COMPEX_WPE72, 8,
		"WPE72", "Compex WPE72",
		"BareBoard"),

	CPX_BOARD_AR23XX(DEVID_COMPEX_NP25G, 4,
		"NP25G", "Compex NetPassage 25G",
		"Wireless 54Mbps XR Router"),
	CPX_BOARD_AR23XX(DEVID_COMPEX_WPE53G, 4,
		"WPE53G", "Compex NetPassage 25G",
		"Wireless 54Mbps XR Access Point"),
	{.model = NULL}
};

void
errmsgv(int syserr, const char *fmt, va_list arg_ptr)
{
	int save = errno;

	fflush(0);
	fprintf(stderr, "[%s] Error: ", progname);
	vfprintf(stderr, fmt, arg_ptr);
	if (syserr != 0) {
		fprintf(stderr, ": %s", strerror(save));
	}
	fprintf(stderr, "\n");
}

void
errmsg(int syserr, const char *fmt, ...)
{
	va_list arg_ptr;
	va_start(arg_ptr, fmt);
	errmsgv(syserr, fmt, arg_ptr);
	va_end(arg_ptr);
}

void
dbgmsg(int level, const char *fmt, ...)
{
	va_list arg_ptr;
	if (verblevel >= level) {
		fflush(0);
		va_start(arg_ptr, fmt);
		vfprintf(stderr, fmt, arg_ptr);
		fprintf(stderr, "\n");
		va_end(arg_ptr);
	}
}


void
usage(int status)
{
	FILE *stream = (status != EXIT_SUCCESS) ? stderr : stdout;
	struct cpx_board *board;

	fprintf(stream, "Usage: %s [OPTION...] <file>\n", progname);
	fprintf(stream,
"\n"
"  <file>          write output to the <file>\n"
"\n"
"Options:\n"
"  -B <board>      create firmware for the board specified with <board>.\n"
"                  This option set vendor id, device id, subvendor id,\n"
"                  subdevice id, and flash size options to the right value.\n"
"                  valid <board> values:\n");
	for (board = boards; board->model != NULL; board++){
		fprintf(stream,
"                      %-12s: %s\n",
		 board->model, board->name);
	};
	fprintf(stream,
"  -i <vid>:<did>[:<svid>[:<sdid>]]\n"
"                  create firmware for board with vendor id <vid>, device\n"
"                  id <did>, subvendor id <svid> and subdevice id <sdid>.\n"
"  -r <rev>        set board revision to <rev>.\n"
"  -s <size>       set flash size to <size>\n"
"  -b <addr>:<len>[:[<flags>]:<file>]\n"
"                  define block at <addr> with length of <len>.\n"
"                  valid <flag> values:\n"
"                      h : add crc header before the file data.\n"
"  -p <addr>:<len>[:<flags>[:<param>[:<name>[:<file>]]]]\n"
"                  add partition at <addr>, with size of <len> to the\n"
"                  partition table, set partition name to <name>, partition \n"
"                  flags to <flags> and partition parameter to <param>.\n"
"                  If the <file> is specified content of the file will be \n"
"                  added to the firmware image.\n"
"                  valid <flag> values:\n"
"                      a:  this is the active partition. The bootloader loads\n"
"                          the firmware from this partition.\n"
"                      h:  the partition data have a header.\n"
"                      l:  the partition data uses LZMA compression.\n"
"                      p:  the bootloader loads data from this partition to\n"
"                          the RAM before decompress it.\n"
"  -h              show this screen\n"
	);

	exit(status);
}

/*
 * Code to compute the CRC-32 table. Borrowed from
 * gzip-1.0.3/makecrc.c.
 */

static uint32_t crc_32_tab[256];

void
init_crc_table(void)
{
	/* Not copyrighted 1990 Mark Adler	*/

	uint32_t c;      /* crc shift register */
	uint32_t e;      /* polynomial exclusive-or pattern */
	int i;           /* counter for all possible eight bit values */
	int k;           /* byte being shifted into crc apparatus */

	/* terms of polynomial defining this crc (except x^32): */
	static const int p[] = {0,1,2,4,5,7,8,10,11,12,16,22,23,26};

	/* Make exclusive-or pattern from polynomial */
	e = 0;
	for (i = 0; i < sizeof(p)/sizeof(int); i++)
		e |= 1L << (31 - p[i]);

	crc_32_tab[0] = 0;

	for (i = 1; i < 256; i++) {
		c = 0;
		for (k = i | 256; k != 1; k >>= 1) {
			c = c & 1 ? (c >> 1) ^ e : c >> 1;
			if (k & 1)
				c ^= e;
		}
		crc_32_tab[i] = c;
	}
}


void
update_crc(uint8_t *p, uint32_t len, uint32_t *crc)
{
	uint32_t t;

	t = *crc ^ 0xFFFFFFFFUL;
	while (len--) {
		t = crc_32_tab[(t ^ *p++) & 0xff] ^ (t >> 8);
	}
	*crc = t ^ 0xFFFFFFFFUL;
}


uint32_t
get_crc(uint8_t *p, uint32_t len)
{
	uint32_t crc;

	crc = 0;
	update_crc(p ,len , &crc);
	return crc;
}


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


struct cpx_board *
find_board(char *model){
	struct cpx_board *board;
	struct cpx_board *tmp;

	board = NULL;
	for (tmp = boards; tmp->model != NULL; tmp++){
		if (strcasecmp(model, tmp->model) == 0) {
			board = tmp;
			break;
		}
	};

	return board;
}


int
get_file_crc(struct fw_block *ff)
{
	FILE *f;
	uint8_t buf[FILE_BUF_LEN];
	uint32_t readlen = sizeof(buf);
	int res = -1;
	size_t len;

	if ((ff->flags & BLOCK_FLAG_HAVEHDR) == 0) {
		res = 0;
		goto out;
	}

	errno = 0;
	f = fopen(ff->name,"r");
	if (errno) {
		errmsg(1,"unable to open file %s", ff->name);
		goto out;
	}

	ff->crc = 0;
	len = ff->size;
	while (len > 0) {
		if (len < readlen)
			readlen = len;

		errno = 0;
		fread(buf, readlen, 1, f);
		if (errno) {
			errmsg(1,"unable to read from file %s",	ff->name);
			goto out_close;
		}

		update_crc(buf, readlen, &ff->crc);
		len -= readlen;
	}

	res = 0;

out_close:
	fclose(f);
out:
	return res;
}


int
process_files(void)
{
	struct fw_block *b;
	struct stat st;
	int i;

	for (i = 0; i < fw_num_blocks; i++) {
		b = &fw_blocks[i];
		if ((b->addr + b->blocklen) > flash_size) {
			errmsg(0, "block at 0x%08X is too big", b->addr);
			return -1;
		}
		if (b->name == NULL)
			continue;

		if (stat(b->name, &st) < 0) {
			errmsg(0, "stat failed on %s",b->name);
			return -1;
		}
		if (b->blocklen == 0) {
			b->blocklen = flash_size - b->addr;
		}
		if (st.st_size > b->blocklen) {
			errmsg(0,"file %s is too big",b->name);
			return -1;
		}

		b->size = st.st_size;
	}

	return 0;
}


int
process_partitions(void)
{
	struct mylo_partition *part;
	int i;

	for (i = 0; i < fw_num_partitions; i++) {
		part = &fw_parts[i].mylo;

		if (part->addr > flash_size) {
			errmsg(0, "invalid partition at 0x%08X", part->addr);
			return -1;
		}

		if ((part->addr + part->size) > flash_size) {
			errmsg(0, "partition at 0x%08X is too big", part->addr);
			return -1;
		}
	}

	return 0;
}


/*
 * routines to write data to the output file
 */
int
write_out_data(FILE *outfile, void *data, size_t len, uint32_t *crc)
{
	uint8_t *ptr = data;

	errno = 0;

	fwrite(ptr, len, 1, outfile);
	if (errno) {
		errmsg(1,"unable to write output file");
		return -1;
	}

	if (crc) {
		update_crc(ptr, len, crc);
	}

	return 0;
}


int
write_out_desc(FILE *outfile, struct mylo_fw_blockdesc *desc, uint32_t *crc)
{
	return write_out_data(outfile, (uint8_t *)desc,
		sizeof(*desc), crc);
}


int
write_out_padding(FILE *outfile, size_t len, uint8_t padc, uint32_t *crc)
{
	uint8_t buff[512];
	size_t  buflen = sizeof(buff);

	memset(buff, padc, buflen);

	while (len > 0) {
		if (len < buflen)
			buflen = len;

		if (write_out_data(outfile, buff, buflen, crc))
			return -1;

		len -= buflen;
	}

	return 0;
}


int
write_out_file(FILE *outfile, struct fw_block *block, uint32_t *crc)
{
	char buff[FILE_BUF_LEN];
	size_t  buflen = sizeof(buff);
	FILE *f;
	size_t len;

	errno = 0;

	if (block->name == NULL) {
		return 0;
	}

	if ((block->flags & BLOCK_FLAG_HAVEHDR) != 0) {
		struct mylo_partition_header ph;

		if (get_file_crc(block) != 0)
		        return -1;

		ph.crc = HOST_TO_LE32(block->crc);
		ph.len = HOST_TO_LE32(block->size);

		if (write_out_data(outfile, (uint8_t *)&ph, sizeof(ph), crc) != 0)
			return -1;
	}

	f = fopen(block->name,"r");
	if (errno) {
		errmsg(1,"unable to open file: %s", block->name);
		return -1;
	}

	len = block->size;
	while (len > 0) {
		if (len < buflen)
			buflen = len;

		/* read data from source file */
		errno = 0;
		fread(buff, buflen, 1, f);
		if (errno != 0) {
			errmsg(1,"unable to read from file: %s",block->name);
			return -1;
		}

		if (write_out_data(outfile, buff, buflen, crc) != 0)
			return -1;

		len -= buflen;
	}

	fclose(f);

	/* align next block on a 4 byte boundary */
	len = block->size % 4;
	if (write_out_padding(outfile, len, 0xFF, crc))
		return -1;

	dbgmsg(1,"file %s written out", block->name);
	return 0;
}


int
write_out_header(FILE *outfile, uint32_t *crc)
{
	struct mylo_fw_header hdr;

	memset(&hdr, 0, sizeof(hdr));

	hdr.magic = HOST_TO_LE32(MYLO_MAGIC_FIRMWARE);
	hdr.crc = HOST_TO_LE32(fw_header.crc);
	hdr.vid = HOST_TO_LE16(fw_header.vid);
	hdr.did = HOST_TO_LE16(fw_header.did);
	hdr.svid = HOST_TO_LE16(fw_header.svid);
	hdr.sdid = HOST_TO_LE16(fw_header.sdid);
	hdr.rev = HOST_TO_LE32(fw_header.rev);
	hdr.fwhi = HOST_TO_LE32(fw_header.fwhi);
	hdr.fwlo = HOST_TO_LE32(fw_header.fwlo);
	hdr.flags = HOST_TO_LE32(fw_header.flags);

	if (fseek(outfile, 0, SEEK_SET) != 0) {
		errmsg(1,"fseek failed on output file");
		return -1;
	}

	return write_out_data(outfile, (uint8_t *)&hdr, sizeof(hdr), crc);
}


int
write_out_partitions(FILE *outfile, uint32_t *crc)
{
	struct mylo_partition_table p;
	char part_names[MYLO_MAX_PARTITIONS][PART_NAME_LEN];
	int ret;
	int i;

	if (fw_num_partitions == 0)
		return 0;

	memset(&p, 0, sizeof(p));
	memset(part_names, 0, sizeof(part_names));

	p.magic = HOST_TO_LE32(MYLO_MAGIC_PARTITIONS);
	for (i = 0; i < fw_num_partitions; i++) {
		struct mylo_partition *mp;
		struct fw_part *fp;

		mp = &p.partitions[i];
		fp = &fw_parts[i];
		mp->flags = HOST_TO_LE16(fp->mylo.flags);
		mp->type = HOST_TO_LE16(PARTITION_TYPE_USED);
		mp->addr = HOST_TO_LE32(fp->mylo.addr);
		mp->size = HOST_TO_LE32(fp->mylo.size);
		mp->param = HOST_TO_LE32(fp->mylo.param);

		memcpy(part_names[i], fp->name, PART_NAME_LEN);
	}

	ret = write_out_data(outfile, (uint8_t *)&p, sizeof(p), crc);
	if (ret)
		return ret;

	ret = write_out_data(outfile, (uint8_t *)part_names, sizeof(part_names),
				crc);
	return ret;
}


int
write_out_blocks(FILE *outfile, uint32_t *crc)
{
	struct mylo_fw_blockdesc desc;
	struct fw_block *b;
	uint32_t dlen;
	int i;

	/*
	 * if at least one partition specified, write out block descriptor
	 * for the partition table
	 */
	if (fw_num_partitions > 0) {
		desc.type = HOST_TO_LE32(FW_DESC_TYPE_USED);
		desc.addr = HOST_TO_LE32(board->part_offset);
		desc.dlen = HOST_TO_LE32(sizeof(struct mylo_partition_table) +
					(MYLO_MAX_PARTITIONS * PART_NAME_LEN));
		desc.blen = HOST_TO_LE32(board->part_size);

		if (write_out_desc(outfile, &desc, crc) != 0)
		        return -1;
	}

	/*
	 * write out block descriptors for each files
	 */
	for (i = 0; i < fw_num_blocks; i++) {
		b = &fw_blocks[i];

		/* detect block size */
		dlen = b->size;
		if ((b->flags & BLOCK_FLAG_HAVEHDR) != 0) {
			dlen += sizeof(struct mylo_partition_header);
		}

		/* round up to 4 bytes */
		dlen = ALIGN(dlen, 4);

		/* setup the descriptor */
		desc.type = HOST_TO_LE32(FW_DESC_TYPE_USED);
		desc.addr = HOST_TO_LE32(b->addr);
		desc.dlen = HOST_TO_LE32(dlen);
		desc.blen = HOST_TO_LE32(b->blocklen);

		if (write_out_desc(outfile, &desc, crc) != 0)
			return -1;
	}

	/*
	 * write out the null block descriptor
	 */
	memset(&desc, 0, sizeof(desc));
	if (write_out_desc(outfile, &desc, crc) != 0)
		return -1;

	if (write_out_partitions(outfile, crc) != 0)
		return -1;

	/*
	 * write out data for each blocks
	 */
	for (i = 0; i < fw_num_blocks; i++) {
		b = &fw_blocks[i];
		if (write_out_file(outfile, b, crc) != 0)
		        return -1;
	}

	return 0;
}


/*
 * argument parsing
 */
int
parse_arg(char *arg, char *buf, char *argv[])
{
	int res = 0;
	size_t argl;
	char *tok;
	char **ap = &buf;
	int i;

	if ((arg == NULL)) {
		/* invalid argument string */
		return -1;
	}

	argl = strlen(arg);
	if (argl == 0) {
		/* no arguments */
		return res;
	}

	if (argl >= MAX_ARG_LEN) {
		/* argument is too long */
		argl = MAX_ARG_LEN-1;
	}

	memset(argv, 0, MAX_ARG_COUNT * sizeof(void *));
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
	if ((optarg != NULL) && (*arg == '-')){
		errmsg(0,"option %c requires an argument\n", c);
		return -1;
	}

	return 0;
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


int
parse_opt_flags(char ch, char *arg)
{
	if (required_arg(ch, arg)) {
		goto err_out;
	}

	if (str2u32(arg, &fw_header.flags) != 0) {
		errmsg(0,"invalid firmware flags: %s", arg);
		goto err_out;
	}

	dbgmsg(1, "firmware flags set to %X bytes", fw_header.flags);

	return 0;

err_out:
	return -1;
}


int
parse_opt_size(char ch, char *arg)
{
	if (required_arg(ch, arg)) {
		goto err_out;
	}

	if (str2u32(arg, &flash_size) != 0) {
		errmsg(0,"invalid flash size: %s", arg);
		goto err_out;
	}

	dbgmsg(1, "flash size set to %d bytes", flash_size);

	return 0;

err_out:
	return -1;
}


int
parse_opt_id(char ch, char *arg)
{
	char buf[MAX_ARG_LEN];
	char *argv[MAX_ARG_COUNT];
	char *p;

	if (required_arg(ch, arg)) {
		goto err_out;
	}

	parse_arg(arg, buf, argv);

	/* processing vendor ID*/
	p = argv[0];
	if (is_empty_arg(p)) {
		errmsg(0,"vendor id is missing from -%c %s",ch, arg);
		goto err_out;
	} else if (str2u16(p, &fw_header.vid) != 0) {
		errmsg(0,"invalid vendor id: %s", p);
		goto err_out;
	}

	dbgmsg(1, "vendor id is set to 0x%04X", fw_header.vid);

	/* processing device ID*/
	p = argv[1];
	if (is_empty_arg(p)) {
		errmsg(0,"device id is missing from -%c %s",ch, arg);
		goto err_out;
	} else if (str2u16(p, &fw_header.did) != 0) {
		errmsg(0,"invalid device id: %s", p);
		goto err_out;
	}

	dbgmsg(1, "device id is set to 0x%04X", fw_header.did);

	/* processing sub vendor ID*/
	p = argv[2];
	if (is_empty_arg(p)) {
		fw_header.svid = fw_header.vid;
	} else if (str2u16(p, &fw_header.svid) != 0) {
		errmsg(0,"invalid sub vendor id: %s", p);
		goto err_out;
	}

	dbgmsg(1, "sub vendor id is set to 0x%04X", fw_header.svid);

	/* processing device ID*/
	p = argv[3];
	if (is_empty_arg(p)) {
		fw_header.sdid = fw_header.did;
	} else if (str2u16(p, &fw_header.sdid) != 0) {
		errmsg(0,"invalid sub device id: %s", p);
		goto err_out;
	}

	dbgmsg(1, "sub device id is set to 0x%04X", fw_header.sdid);

	/* processing revision */
	p = argv[4];
	if (is_empty_arg(p)) {
		fw_header.rev = 0;
	} else if (str2u32(arg, &fw_header.rev) != 0) {
		errmsg(0,"invalid revision number: %s", p);
		goto err_out;
	}

	dbgmsg(1, "board revision is set to 0x%08X", fw_header.rev);

	return 0;

err_out:
	return -1;
}


int
parse_opt_block(char ch, char *arg)
{
	char buf[MAX_ARG_LEN];
	char *argv[MAX_ARG_COUNT];
	int argc;
	struct fw_block *b;
	char *p;

	if (required_arg(ch, arg)) {
		goto err_out;
	}

	if (fw_num_blocks >= MAX_FW_BLOCKS) {
		errmsg(0,"too many blocks specified");
		goto err_out;
	}

	argc = parse_arg(arg, buf, argv);
	dbgmsg(1,"processing block option %s, count %d", arg, argc);

	b = &fw_blocks[fw_num_blocks++];

	/* processing block address */
	p = argv[0];
	if (is_empty_arg(p)) {
		errmsg(0,"no block address specified in %s", arg);
		goto err_out;
	} else if (str2u32(p, &b->addr) != 0) {
		errmsg(0,"invalid block address: %s", p);
		goto err_out;
	}

	/* processing block length */
	p = argv[1];
	if (is_empty_arg(p)) {
		errmsg(0,"no block length specified in %s", arg);
		goto err_out;
	} else if (str2u32(p, &b->blocklen) != 0) {
		errmsg(0,"invalid block length: %s", p);
		goto err_out;
	}

	if (argc < 3) {
		dbgmsg(1,"empty block %s", arg);
		goto success;
	}

	/* processing flags */
	p = argv[2];
	if (is_empty_arg(p) == 0) {
		for ( ; *p != '\0'; p++) {
			switch (*p) {
			case 'h':
				b->flags |= BLOCK_FLAG_HAVEHDR;
				break;
			default:
				errmsg(0, "invalid block flag \"%c\"", *p);
				goto err_out;
			}
		}
	}

	/* processing file name */
	p = argv[3];
	if (is_empty_arg(p)) {
		errmsg(0,"file name missing in %s", arg);
		goto err_out;
	}

	b->name = strdup(p);
	if (b->name == NULL) {
		errmsg(0,"not enough memory");
		goto err_out;
	}

success:

	return 0;

err_out:
	return -1;
}


int
parse_opt_partition(char ch, char *arg)
{
	char buf[MAX_ARG_LEN];
	char *argv[MAX_ARG_COUNT];
	char *p;
	struct mylo_partition *part;
	struct fw_part *fp;

	if (required_arg(ch, arg)) {
		goto err_out;
	}

	if (fw_num_partitions >= MYLO_MAX_PARTITIONS) {
		errmsg(0, "too many partitions specified");
		goto err_out;
	}

	fp = &fw_parts[fw_num_partitions++];
	part = &fp->mylo;

	parse_arg(arg, buf, argv);

	/* processing partition address */
	p = argv[0];
	if (is_empty_arg(p)) {
		errmsg(0,"partition address missing in -%c %s",ch, arg);
		goto err_out;
	} else if (str2u32(p, &part->addr) != 0) {
		errmsg(0,"invalid partition address: %s", p);
		goto err_out;
	}

	/* processing partition size */
	p = argv[1];
	if (is_empty_arg(p)) {
		errmsg(0,"partition size missing in -%c %s",ch, arg);
		goto err_out;
	} else if (str2u32(p, &part->size) != 0) {
		errmsg(0,"invalid partition size: %s", p);
		goto err_out;
	}

	/* processing partition flags */
	p = argv[2];
	if (is_empty_arg(p) == 0) {
		for ( ; *p != '\0'; p++) {
			switch (*p) {
			case 'a':
				part->flags |= PARTITION_FLAG_ACTIVE;
				break;
			case 'p':
				part->flags |= PARTITION_FLAG_PRELOAD;
				break;
			case 'l':
				part->flags |= PARTITION_FLAG_LZMA;
				break;
			case 'h':
				part->flags |= PARTITION_FLAG_HAVEHDR;
				break;
			default:
				errmsg(0, "invalid partition flag \"%c\"", *p);
				goto err_out;
			}
		}
	}

	/* processing partition parameter */
	p = argv[3];
	if (is_empty_arg(p)) {
		/* set default partition parameter */
		part->param = 0;
	} else if (str2u32(p, &part->param) != 0) {
		errmsg(0,"invalid partition parameter: %s", p);
		goto err_out;
	}

	p = argv[4];
	if (is_empty_arg(p)) {
		/* set default partition parameter */
		fp->name[0] = '\0';
	} else {
		strncpy(fp->name, p, PART_NAME_LEN);
	}

#if 1
	if (part->size == 0) {
		part->size = flash_size - part->addr;
	}

	/* processing file parameter */
	p = argv[5];
	if (is_empty_arg(p) == 0) {
		struct fw_block *b;

		if (fw_num_blocks == MAX_FW_BLOCKS) {
			errmsg(0,"too many blocks specified", p);
			goto err_out;
		}
		b = &fw_blocks[fw_num_blocks++];
		b->name = strdup(p);
		b->addr = part->addr;
		b->blocklen = part->size;
		if (part->flags & PARTITION_FLAG_HAVEHDR) {
			b->flags |= BLOCK_FLAG_HAVEHDR;
		}
	}
#endif

	return 0;

err_out:
	return -1;
}


int
parse_opt_board(char ch, char *arg)
{
	if (required_arg(ch, arg)) {
		goto err_out;
	}

	board = find_board(arg);
	if (board == NULL){
		errmsg(0,"invalid/unknown board specified: %s", arg);
		goto err_out;
	}

	fw_header.vid = board->vid;
	fw_header.did = board->did;
	fw_header.svid = board->svid;
	fw_header.sdid = board->sdid;

	flash_size = board->flash_size;

	return 0;

err_out:
	return -1;
}


int
parse_opt_rev(char ch, char *arg)
{
	if (required_arg(ch, arg)) {
		return -1;
	}

	if (str2u32(arg, &fw_header.rev) != 0) {
		errmsg(0,"invalid revision number: %s", arg);
		return -1;
	}

	return 0;
}


/*
 * main
 */
int
main(int argc, char *argv[])
{
	int optinvalid = 0;   /* flag for invalid option */
	int c;
	int res = EXIT_FAILURE;

	FILE  *outfile;
	uint32_t crc;

	progname=basename(argv[0]);

	memset(&fw_header, 0, sizeof(fw_header));

	/* init header defaults */
	fw_header.vid = VENID_COMPEX;
	fw_header.did = DEVID_COMPEX_WP54G;
	fw_header.svid = VENID_COMPEX;
	fw_header.sdid = DEVID_COMPEX_WP54G;
	fw_header.fwhi = 0x20000;
	fw_header.fwlo = 0x20000;
	fw_header.flags = 0;

	opterr = 0;  /* could not print standard getopt error messages */
	while ((c = getopt(argc, argv, "b:B:f:hi:p:r:s:v")) != -1) {
		optinvalid = 0;
		switch (c) {
		case 'b':
			optinvalid = parse_opt_block(c,optarg);
			break;
		case 'B':
			optinvalid = parse_opt_board(c,optarg);
			break;
		case 'f':
			optinvalid = parse_opt_flags(c,optarg);
			break;
		case 'h':
			usage(EXIT_SUCCESS);
			break;
		case 'i':
			optinvalid = parse_opt_id(c,optarg);
			break;
		case 'p':
			optinvalid = parse_opt_partition(c,optarg);
			break;
		case 'r':
			optinvalid = parse_opt_rev(c,optarg);
			break;
		case 's':
			optinvalid = parse_opt_size(c,optarg);
			break;
		case 'v':
			verblevel++;
			break;
		default:
			optinvalid = 1;
			break;
		}
		if (optinvalid != 0 ){
			errmsg(0, "invalid option: -%c", optopt);
			goto out;
		}
	}

	if (optind == argc) {
		errmsg(0, "no output file specified");
		goto out;
	}

	ofname = argv[optind++];

	if (optind < argc) {
		errmsg(0, "invalid option: %s", argv[optind]);
		goto out;
	}

	if (!board) {
		errmsg(0, "no board specified");
		goto out;
	}

	if (flash_size == 0) {
		errmsg(0, "no flash size specified");
		goto out;
	}

	if (process_files() != 0) {
		goto out;
	}

	if (process_partitions() != 0) {
		goto out;
	}

	outfile = fopen(ofname, "w");
	if (outfile == NULL) {
		errmsg(1, "could not open \"%s\" for writing", ofname);
		goto out;
	}

	crc = 0;
	init_crc_table();

	if (write_out_header(outfile, &crc) != 0)
		goto out_flush;

	if (write_out_blocks(outfile, &crc) != 0)
		goto out_flush;

	fw_header.crc = crc;
	if (write_out_header(outfile, NULL) != 0)
		goto out_flush;

	dbgmsg(1,"Firmware file %s completed.", ofname);

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

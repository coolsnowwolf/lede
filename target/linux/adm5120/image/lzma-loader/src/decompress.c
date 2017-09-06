/*
 *
 * LZMA compressed kernel decompressor for ADM5120 boards
 *
 * Copyright (C) 2005 by Oleg I. Vdovikin <oleg@cs.msu.su>
 * Copyright (C) 2007-2008 OpenWrt.org
 * Copyright (C) 2007-2008 Gabor Juhos <juhosg@openwrt.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 *
 * Please note, this was code based on the bunzip2 decompressor code
 * by Manuel Novoa III  (mjn3@codepoet.org), although the only thing left
 * is an idea and part of original vendor code
 *
 *
 * 12-Mar-2005  Mineharu Takahara <mtakahar@yahoo.com>
 *   pass actual output size to decoder (stream mode
 *   compressed input is not a requirement anymore)
 *
 * 24-Apr-2005 Oleg I. Vdovikin
 *   reordered functions using lds script, removed forward decl
 *
 * 24-Mar-2007 Gabor Juhos
 *   pass original values of the a0,a1,a2,a3 registers to the kernel
 *
 * 19-May-2007 Gabor Juhos
 *   endiannes related cleanups
 *   add support for decompressing an embedded kernel
 *
 */

#include <stddef.h>

#include "config.h"
#include "printf.h"
#include "LzmaDecode.h"

#define ADM5120_FLASH_START	0x1fc00000	/* Flash start */
#define ADM5120_FLASH_END	0x1fe00000	/* Flash end */

#define KSEG0			0x80000000
#define KSEG1			0xa0000000

#define KSEG1ADDR(a)		((((unsigned)(a)) & 0x1fffffffU) | KSEG1)

#define Index_Invalidate_I	0x00
#define Index_Writeback_Inv_D   0x01

#define cache_unroll(base,op)	\
	__asm__ __volatile__(		\
		".set noreorder;\n"		\
		".set mips3;\n"			\
		"cache %1, (%0);\n"		\
		".set mips0;\n"			\
		".set reorder\n"		\
		:				\
		: "r" (base),			\
		  "i" (op));

#ifdef LZMA_DEBUG
#  define DBG(f, a...)	printf(f, ## a)
#else
#  define DBG(f, a...)	do {} while (0)
#endif

static __inline__ void blast_icache(unsigned long size, unsigned long lsize)
{
	unsigned long start = KSEG0;
	unsigned long end = (start + size);

	while(start < end) {
		cache_unroll(start,Index_Invalidate_I);
		start += lsize;
	}
}

static __inline__ void blast_dcache(unsigned long size, unsigned long lsize)
{
	unsigned long start = KSEG0;
	unsigned long end = (start + size);

	while(start < end) {
		cache_unroll(start,Index_Writeback_Inv_D);
		start += lsize;
	}
}

#define TRX_MAGIC       0x30524448      /* "HDR0" */
#define TRX_ALIGN	0x1000

struct trx_header {
	unsigned int magic;		/* "HDR0" */
	unsigned int len;		/* Length of file including header */
	unsigned int crc32;		/* 32-bit CRC from flag_version to end of file */
	unsigned int flag_version;	/* 0:15 flags, 16:31 version */
	unsigned int offsets[3];	/* Offsets of partitions from start of header */
};

struct env_var {
	char	*name;
	char	*value;
};

/* beyound the image end, size not known in advance */
extern unsigned char workspace[];
extern void board_init(void);

static CLzmaDecoderState lzma_state;

typedef void (*kernel_entry)(unsigned long reg_a0, unsigned long reg_a1,
	unsigned long reg_a2, unsigned long reg_a3);

static int decompress_data(CLzmaDecoderState *vs, unsigned char *outStream,
			UInt32 outSize);

#ifdef CONFIG_PASS_KARGS
#define ENVV(n,v)	{.name = (n), .value = (v)}
struct env_var env_vars[] = {
	ENVV("board_name",	CONFIG_BOARD_NAME),
	ENVV(NULL, NULL)
};
#endif

static void halt(void)
{
	printf("\nSystem halted!\n");
	for(;;);
}

#if (LZMA_WRAPPER)
extern unsigned char _lzma_data_start[];
extern unsigned char _lzma_data_end[];

unsigned char *data;
unsigned long datalen;

static __inline__ unsigned char get_byte(void)
{
	datalen--;
	return *data++;
}

static void decompress_init(void)
{
	data = _lzma_data_start;
	datalen = _lzma_data_end - _lzma_data_start;
}

static int decompress_data(CLzmaDecoderState *vs, unsigned char *outStream,
			SizeT outSize)
{
	SizeT ip, op;

	return LzmaDecode(vs, data, datalen, &ip, outStream, outSize, &op);
}
#endif /* LZMA_WRAPPER */

#if !(LZMA_WRAPPER)

#define FLASH_BANK_SIZE	(2<<20)

static unsigned char *flash_base = (unsigned char *) KSEG1ADDR(ADM5120_FLASH_START);
static unsigned long flash_ofs = 0;
static unsigned long flash_max = 0;
static unsigned long flash_ofs_mask = (FLASH_BANK_SIZE-1);

static __inline__ unsigned char get_byte(void)
{
	return *(flash_base+flash_ofs++);
}

static int lzma_read_byte(void *object, const unsigned char **buffer,
				SizeT *bufferSize)
{
	unsigned long len;

	if (flash_ofs >= flash_max)
		return LZMA_RESULT_DATA_ERROR;

	len = flash_max-flash_ofs;

#if (CONFIG_FLASH_SIZE > FLASH_BANK_SIZE)
	if (flash_ofs < FLASH_BANK_SIZE) {
		/* switch to bank 0 */
		DBG("lzma_read_byte: switch to bank 0\n");

		if (len > FLASH_BANK_SIZE-flash_ofs)
			len = FLASH_BANK_SIZE-flash_ofs;
	} else {
		/* switch to bank 1 */
		DBG("lzma_read_byte: switch to bank 1\n");
	}
#endif
	DBG("lzma_read_byte: ofs=%08X, len=%08X\n", flash_ofs, len);

	*buffer = flash_base+(flash_ofs & flash_ofs_mask);
	*bufferSize = len;
	flash_ofs += len;

	return LZMA_RESULT_OK;
}

static ILzmaInCallback lzma_callback = {
	.Read	= lzma_read_byte,
};

static __inline__ unsigned int read_le32(void *buf)
{
	unsigned char *p = buf;

	return ((unsigned int)p[0] + ((unsigned int)p[1] << 8) +
		((unsigned int)p[2] << 16) +((unsigned int)p[3] << 24));
}

static void decompress_init(void)
{
	struct trx_header *hdr = NULL;
	unsigned long kofs,klen;

    	printf("Looking for TRX header... ");
	/* look for trx header, 32-bit data access */
	for (flash_ofs = 0; flash_ofs < FLASH_BANK_SIZE; flash_ofs += TRX_ALIGN) {
		if (read_le32(&flash_base[flash_ofs]) == TRX_MAGIC) {
			hdr = (struct trx_header *)&flash_base[flash_ofs];
			break;
		}
	}

	if (hdr == NULL) {
		printf("not found!\n");
		/* no compressed kernel found, halting */
		halt();
	}

	/* compressed kernel is in the partition 0 or 1 */
	kofs = read_le32(&hdr->offsets[1]);
	if (kofs == 0 || kofs > 65536) {
		klen = kofs-read_le32(&hdr->offsets[0]);
		kofs = read_le32(&hdr->offsets[0]);
	} else {
		klen = read_le32(&hdr->offsets[2]);
		if (klen > kofs)
			klen -= kofs;
		else
			klen = read_le32(&hdr->len)-kofs;
	}

	printf("found at %08X, kernel:%08X len:%08X\n", flash_ofs,
		kofs, klen);

	flash_ofs += kofs;
	flash_max = flash_ofs+klen;
}

static int decompress_data(CLzmaDecoderState *vs, unsigned char *outStream,
			SizeT outSize)
{
	SizeT op;

#if 0
	vs->Buffer = data;
	vs->BufferLim = datalen;
#endif

	return LzmaDecode(vs, &lzma_callback, outStream, outSize, &op);
}
#endif /* !(LZMA_WRAPPER) */

/* should be the first function */
void decompress_entry(unsigned long reg_a0, unsigned long reg_a1,
	unsigned long reg_a2, unsigned long reg_a3,
	unsigned long icache_size, unsigned long icache_lsize,
	unsigned long dcache_size, unsigned long dcache_lsize)
{
	unsigned char props[LZMA_PROPERTIES_SIZE];
	unsigned int i;  /* temp value */
	SizeT osize; /* uncompressed size */
	int res;

	board_init();

	printf("\n\nLZMA loader for " CONFIG_BOARD_NAME
			", Copyright (C) 2007-2008 OpenWrt.org\n\n");

	decompress_init();

	/* lzma args */
	for (i = 0; i < LZMA_PROPERTIES_SIZE; i++)
		props[i] = get_byte();

	/* skip rest of the LZMA coder property */
	/* read the lower half of uncompressed size in the header */
	osize = ((SizeT)get_byte()) +
		((SizeT)get_byte() << 8) +
		((SizeT)get_byte() << 16) +
		((SizeT)get_byte() << 24);

	/* skip rest of the header (upper half of uncompressed size) */
	for (i = 0; i < 4; i++)
		get_byte();

	res = LzmaDecodeProperties(&lzma_state.Properties, props,
					LZMA_PROPERTIES_SIZE);
	if (res != LZMA_RESULT_OK) {
		printf("Incorrect LZMA stream properties!\n");
		halt();
	}

	printf("decompressing kernel... ");

	lzma_state.Probs = (CProb *)workspace;
	res = decompress_data(&lzma_state, (unsigned char *)LOADADDR, osize);

	if (res != LZMA_RESULT_OK) {
		printf("failed, ");
		switch (res) {
		case LZMA_RESULT_DATA_ERROR:
			printf("data error!\n");
			break;
		default:
			printf("unknown error %d!\n", res);
		}
		halt();
	} else
		printf("done!\n");

	blast_dcache(dcache_size, dcache_lsize);
	blast_icache(icache_size, icache_lsize);

	printf("launching kernel...\n\n");

#ifdef CONFIG_PASS_KARGS
	reg_a0 = 0;
	reg_a1 = 0;
	reg_a2 = (unsigned long)env_vars;
	reg_a3 = 0;
#endif
	/* Jump to load address */
	((kernel_entry) LOADADDR)(reg_a0, reg_a1, reg_a2, reg_a3);
}

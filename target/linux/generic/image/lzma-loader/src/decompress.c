/*
 * LZMA compressed kernel decompressor for bcm947xx boards
 *
 * Copyright (C) 2005 by Oleg I. Vdovikin <oleg@cs.msu.su>
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
 * ??-Nov-2005 Mike Baker
 *   reorder the script as an lzma wrapper; do not depend on flash access
 */

#include "LzmaDecode.h"

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
		:						\
		: "r" (base),			\
		  "i" (op));


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

unsigned char *data;

static int read_byte(void *object, unsigned char **buffer, UInt32 *bufferSize)
{
	*bufferSize = 1;
	*buffer = data;
	++data;
	return LZMA_RESULT_OK;
}

static __inline__ unsigned char get_byte(void)
{
	unsigned char *buffer;
	UInt32 fake;
	
	return read_byte(0, &buffer, &fake), *buffer;
}

/* This puts lzma workspace 128k below RAM end. 
 * That should be enough for both lzma and stack
 */
static char *buffer = (char *)(RAMSTART + RAMSIZE - 0x00020000);
extern char lzma_start[];
extern char lzma_end[];

/* should be the first function */
void entry(unsigned long icache_size, unsigned long icache_lsize, 
	unsigned long dcache_size, unsigned long dcache_lsize)
{
	unsigned int i;  /* temp value */
	unsigned int osize; /* uncompressed size */
	volatile unsigned int arg0, arg1, arg2, arg3;

	/* restore argument registers */
	__asm__ __volatile__ ("ori %0, $12, 0":"=r"(arg0));
	__asm__ __volatile__ ("ori %0, $13, 0":"=r"(arg1));
	__asm__ __volatile__ ("ori %0, $14, 0":"=r"(arg2));
	__asm__ __volatile__ ("ori %0, $15, 0":"=r"(arg3));

	ILzmaInCallback callback;
	CLzmaDecoderState vs;
	callback.Read = read_byte;

	data = lzma_start;

	/* lzma args */
	i = get_byte();
	vs.Properties.lc = i % 9, i = i / 9;
	vs.Properties.lp = i % 5, vs.Properties.pb = i / 5;

	vs.Probs = (CProb *)buffer;

	/* skip rest of the LZMA coder property */
	for (i = 0; i < 4; i++)
		get_byte();

	/* read the lower half of uncompressed size in the header */
	osize = ((unsigned int)get_byte()) +
		((unsigned int)get_byte() << 8) +
		((unsigned int)get_byte() << 16) +
		((unsigned int)get_byte() << 24);

	/* skip rest of the header (upper half of uncompressed size) */
	for (i = 0; i < 4; i++) 
		get_byte();

	/* decompress kernel */
	if ((i = LzmaDecode(&vs, &callback,
	(unsigned char*)KERNEL_ENTRY, osize, &osize)) == LZMA_RESULT_OK)
	{
		blast_dcache(dcache_size, dcache_lsize);
		blast_icache(icache_size, icache_lsize);

		/* Jump to load address */
		((void (*)(int a0, int a1, int a2, int a3)) KERNEL_ENTRY)(arg0, arg1, arg2, arg3);
	}
}

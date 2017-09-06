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
#include <exports.h>

#define	KSEG0ADDR(addr)	(0x80000000|addr)

register volatile gd_t *gd asm ("k0");
unsigned char *data;

static __inline__ unsigned char get_byte()
{
	unsigned char *buffer;

	buffer = data;
	data++;

	return *buffer;
}

/* This puts lzma workspace 128k below RAM end.
 * That should be enough for both lzma and stack
 */
static char *buffer = (char *)(RAMSTART + RAMSIZE - 0x00020000);
extern char _binary_vmlinux_lzma_start[];
extern char _binary_vmlinux_lzma_end[];
extern char lzma_start[];
extern char lzma_end[];

/* should be the first function */
void entry(unsigned int arg0, unsigned int arg1,
	unsigned int arg2, unsigned int arg3)
{
	unsigned int i;  /* temp value */
	unsigned int isize; /* compressed size */
	unsigned int osize; /* uncompressed size */
	int argc = arg0;
	char **argv = (char **)arg1;
	char **envp = (char **)arg2;

	CLzmaDecoderState vs;

	data = (unsigned char *)_binary_vmlinux_lzma_start;
	isize = _binary_vmlinux_lzma_end - _binary_vmlinux_lzma_start + 1;

	puts("\nLZMA kernel loader\n");

	printf("lzma data @ %#x - %#x\n", _binary_vmlinux_lzma_start, _binary_vmlinux_lzma_end);
	printf("load addr @ %#x\n\n", KERNEL_ENTRY);
	printf("jump table @ %#x\n", gd->jt[3]);

	/* lzma args */
	i = get_byte();
	vs.Properties.lc = i % 9, i = i / 9;
	vs.Properties.lp = i % 5, vs.Properties.pb = i / 5;

	vs.Probs = (CProb *)buffer;

	/* skip rest of the LZMA coder property */
	data += 4;

	/* read the lower half of uncompressed size in the header */
	osize = ((unsigned int)get_byte()) +
		((unsigned int)get_byte() << 8) +
		((unsigned int)get_byte() << 16) +
		((unsigned int)get_byte() << 24);

	/* skip rest of the header (upper half of uncompressed size) */
	data += 4;

	/* decompress kernel */
	puts("\nDecompressing kernel...");
	if ((i = LzmaDecode(&vs,
	(unsigned char*)data, isize, &isize,
	(unsigned char*)KERNEL_ENTRY, osize, &osize)) == LZMA_RESULT_OK)
	{
		puts("success!\n");

		/* Jump to load address */
//		((void (*)(int a0, int a1, int a2, int a3))KERNEL_ENTRY)(0,0,0,0);
		((void (*)(int a0, int a1, int a2, int a3))KERNEL_ENTRY)(arg0, arg1, arg2, arg3);
	}
	puts("failure!\n");
}

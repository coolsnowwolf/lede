// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * LZMA compressed kernel loader for Atheros AR7XXX/AR9XXX based boards
 *
 * Copyright (C) 2011 Gabor Juhos <juhosg@openwrt.org>
 *
 * The cache manipulation routine has been taken from the U-Boot project.
 *	(C) Copyright 2003
 *	Wolfgang Denk, DENX Software Engineering, <wd@denx.de>
 */

#include "cache.h"
#include "cacheops.h"
#include "config.h"
#include "printf.h"

#define cache_op(op,addr)						\
	__asm__ __volatile__(						\
	"	.set	push					\n"	\
	"	.set	noreorder				\n"	\
	"	.set	mips3\n\t				\n"	\
	"	cache	%0, %1					\n"	\
	"	.set	pop					\n"	\
	:								\
	: "i" (op), "R" (*(unsigned char *)(addr)))

void flush_cache(unsigned long start_addr, unsigned long size)
{
	unsigned long lsize = CONFIG_CACHELINE_SIZE;
	unsigned long addr = start_addr & ~(lsize - 1);
	unsigned long aend = (start_addr + size + (lsize - 1)) & ~(lsize - 1);

	printf("blasting from 0x%08x to 0x%08x (0x%08x - 0x%08x)\n", start_addr, size, addr, aend);

	while (1) {
		cache_op(Hit_Writeback_Inv_D, addr);
		cache_op(Hit_Invalidate_I, addr);
		if (addr == aend)
			break;
		addr += lsize;
	}
}

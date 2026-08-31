/*
 * LZMA compressed kernel loader for Atheros AR7XXX/AR9XXX based boards
 *
 * Copyright (C) 2011 Gabor Juhos <juhosg@openwrt.org>
 *
 * Some parts of this code was based on the OpenWrt specific lzma-loader
 * for the BCM47xx and ADM5120 based boards:
 *	Copyright (C) 2004 Manuel Novoa III (mjn3@codepoet.org)
 *	Copyright (C) 2005 by Oleg I. Vdovikin <oleg@cs.msu.su>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 */

#include <asm/asm.h>
#include <asm/regdef.h>
#include "cp0regdef.h"
#include "cacheops.h"
#include "config.h"

#define KSEG0		0x80000000

	.macro	ehb
	sll     zero, 3
	.endm

	.text

LEAF(startup)
	.set noreorder
	.set mips32

	mtc0	zero, CP0_WATCHLO	# clear watch registers
	mtc0	zero, CP0_WATCHHI
	mtc0	zero, CP0_CAUSE		# clear before writing status register

	mfc0	t0, CP0_STATUS
	li	t1, 0x1000001f
	or	t0, t1
	xori	t0, 0x1f
	mtc0	t0, CP0_STATUS
	ehb

	/*
	 * Some bootloaders set the 'Kseg0 coherency algorithm' to
	 * 'Cacheable, noncoherent, write-through, no write allocate'
	 * and this cause performance issues. Let's go and change it to
	 * 'Cacheable, noncoherent, write-back, write allocate'
	 */
	mfc0	t0, CP0_CONFIG
	li	t1, ~7			#~CONF_CM_CMASK
	and	t0, t1
	ori	t0, 3			#CONF_CM_CACHABLE_NONCOHERENT
	mtc0	t0, CP0_CONFIG
	nop

	mtc0	zero, CP0_COUNT
	mtc0	zero, CP0_COMPARE
	ehb

	la	t0, __reloc_label	# get linked address of label
	bal	__reloc_label		# branch and link to label to
	nop				# get actual address
__reloc_label:
	subu	t0, ra, t0		# get reloc_delta

	beqz	t0, __reloc_done         # if delta is 0 we are in the right place
	nop

	/* Copy our code to the right place */
	la	t1, _code_start		# get linked address of _code_start
	la	t2, _code_end		# get linked address of _code_end
	addu	t0, t0, t1		# calculate actual address of _code_start

__reloc_copy:
	lw	t3, 0(t0)
	sw	t3, 0(t1)
	add	t1, 4
	blt	t1, t2, __reloc_copy
	add	t0, 4

	/* flush cache */
	la	t0, _code_start
	la	t1, _code_end

	li	t2, ~(CONFIG_CACHELINE_SIZE - 1)
	and	t0, t2
	and	t1, t2
	li	t2, CONFIG_CACHELINE_SIZE

	b	__flush_check
	nop

__flush_line:
	cache	Hit_Writeback_Inv_D, 0(t0)
	cache	Hit_Invalidate_I, 0(t0)
	add	t0, t2

__flush_check:
	bne	t0, t1, __flush_line
	nop

	sync

__reloc_done:

	/* clear bss */
	la	t0, _bss_start
	la	t1, _bss_end
	b	__bss_check
	nop

__bss_fill:
	sw	zero, 0(t0)
	addi	t0, 4

__bss_check:
	bne	t0, t1, __bss_fill
	nop

	/* Setup new "C" stack */
	la	sp, _stack

	/* reserve stack space for a0-a3 registers */
	subu	sp, 16

	/* jump to the decompressor routine */
	la	t0, loader_main
	jr	t0
	nop

	.set reorder
END(startup)

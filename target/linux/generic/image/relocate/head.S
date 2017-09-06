/*
 * Kernel relocation stub for MIPS devices
 *
 * Copyright (C) 2015 Felix Fietkau <nbd@nbd.name>
 *
 * Based on:
 *
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

#define KSEG0		0x80000000

	.macro	ehb
	sll     zero, 3
	.endm

	.macro reset
	li t0, 0xbe000034
	lw t1, 0(t0)
	ori t1, 1
	sw t1, 0(t0)
	.endm

	.text

LEAF(startup)
	.set noreorder
	.set mips32

	.fill 0x10000

	mtc0	zero, CP0_WATCHLO	# clear watch registers
	mtc0	zero, CP0_WATCHHI
	mtc0	zero, CP0_CAUSE		# clear before writing status register

	mfc0	t0, CP0_STATUS
	li	t1, 0x1000001f
	or	t0, t1
	xori	t0, 0x1f
	mtc0	t0, CP0_STATUS
	ehb

	mtc0	zero, CP0_COUNT
	mtc0	zero, CP0_COMPARE
	ehb

	la	t0, __reloc_label	# get linked address of label
	bal	__reloc_label		# branch and link to label to
	nop				# get actual address
__reloc_label:
	subu	t0, ra, t0		# get reloc_delta

	/* Copy our code to the right place */
	la	t1, _code_start		# get linked address of _code_start
	la	t2, _code_end		# get linked address of _code_end

	addu	t4, t2, t0		# calculate actual address of _code_end
	lw	t5, 0(t4)		# get extra data size

	add	t2, t5
	add	t2, 4

	add	t0, t1			# calculate actual address of _code_start

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

	la	t0, __reloc_back
	j	t0
	nop

__reloc_back:
	la	t0, _code_end
	add	t0, 4

	addu	t1, t0, t5

	li	t2, KERNEL_ADDR

__kernel_copy:
	lw	t3, 0(t0)
	sw	t3, 0(t2)
	add	t0, 4
	blt	t0, t1, __kernel_copy
	add	t2, 4

	/* flush cache */
	li	t0, KERNEL_ADDR
	addu	t1, t0, t5

	add t1, CONFIG_CACHELINE_SIZE - 1
	li	t2, ~(CONFIG_CACHELINE_SIZE - 1)
	and	t0, t2
	and	t1, t2
	li	t2, CONFIG_CACHELINE_SIZE

	b	__kernel_flush_check
	nop

__kernel_flush_line:
	cache	Hit_Writeback_Inv_D, 0(t0)
	cache	Hit_Invalidate_I, 0(t0)
	add	t0, t2

__kernel_flush_check:
	bne	t0, t1, __kernel_flush_line
	nop

	sync

	li	t0, KERNEL_ADDR
	jr	t0
	nop

	.set reorder
END(startup)

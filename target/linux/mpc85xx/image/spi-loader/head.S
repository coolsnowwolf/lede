/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (C) 2022 Matthias Schiffer <mschiffer@universe-factory.net>
 *
 * Based on Linux arch/powerpc/boot/crt0.S, which is:
 *
 * Copyright (C) Paul Mackerras 1997.
 */

#include <ppc_asm.h>

	.text
	.global _start
_start:
	/* Do a cache flush for our text, in case the loader didn't */
	lis	r3,_text_start@ha
	addi	r3,r3,_text_start@l
	li	r4,_text_len
	bl	flush_cache

	/* Clear the BSS */
	lis	r3,_bss_start@ha
	addi	r3,r3,_bss_start@l
	li	r4,0
	li	r5,_bss_len
	bl	memset

	/* Set up stack */
	lis	r1,_stack_top@ha
	addi	r1,r1,_stack_top@l
	/* Establish a stack frame */
	li	r0,0
	stwu	r0,-16(r1)

	/* Call start */
	b	start

/*
 *  Copyright (C) 2012 Gateworks Corporation
 *      Chris Lang <clang@gateworks.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/linkage.h>
#include <asm/assembler.h>
#include <asm/asm-offsets.h>

#define D_CACHE_LINE_SIZE 32

	.text

/*
 * R8  - DMA Start Address
 * R9  - DMA Length
 * R10 - DMA Direction
 * R11 - DMA type
 * R12 - fiq_buffer Address
*/

	.global	cns3xxx_fiq_end
ENTRY(cns3xxx_fiq_start)
	str	r8, [r13]

	ldmia	r12, {r8, r9, r10}
	and	r11, r10, #0x3000000
	and	r10, r10, #0xff

	teq	r11, #0x1000000
	beq	cns3xxx_dma_map_area
	teq	r11, #0x2000000
	beq	cns3xxx_dma_unmap_area
	/* fall through */
cns3xxx_dma_flush_range:
	bic	r8, r8, #D_CACHE_LINE_SIZE - 1
1:
	mcr	p15, 0, r8, c7, c14, 1		@ clean & invalidate D line
	add	r8, r8, #D_CACHE_LINE_SIZE
	cmp	r8, r9
	blo	1b
	/* fall through */
cns3xxx_fiq_exit:
	mov	r8, #0
	str	r8, [r12, #8]
	mcr	p15, 0, r8, c7, c10, 4		@ drain write buffer
	subs	pc, lr, #4

cns3xxx_dma_map_area:
	add	r9, r9, r8
	teq	r10, #DMA_FROM_DEVICE
	beq	cns3xxx_dma_inv_range
	teq	r10, #DMA_TO_DEVICE
	bne	cns3xxx_dma_flush_range
	/* fall through */
cns3xxx_dma_clean_range:
	bic	r8, r8, #D_CACHE_LINE_SIZE - 1
1:
	mcr	p15, 0, r8, c7, c10, 1		@ clean D line
	add	r8, r8, #D_CACHE_LINE_SIZE
	cmp	r8, r9
	blo	1b
	b	cns3xxx_fiq_exit

cns3xxx_dma_unmap_area:
	add	r9, r9, r8
	teq	r10, #DMA_TO_DEVICE
	beq	cns3xxx_fiq_exit
	/* fall through */
cns3xxx_dma_inv_range:
	tst	r8, #D_CACHE_LINE_SIZE - 1
	bic	r8, r8, #D_CACHE_LINE_SIZE - 1
	mcrne	p15, 0, r8, c7, c10, 1		@ clean D line
	tst	r9, #D_CACHE_LINE_SIZE - 1
	bic	r9, r9, #D_CACHE_LINE_SIZE - 1
	mcrne	p15, 0, r9, c7, c14, 1		@ clean & invalidate D line
1:
	mcr	p15, 0, r8, c7, c6, 1		@ invalidate D line
	add	r8, r8, #D_CACHE_LINE_SIZE
	cmp	r8, r9
	blo	1b
	b	cns3xxx_fiq_exit

cns3xxx_fiq_end:

/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Realtek RTL838X SRAM clock setters
 * Copyright (C) 2022 Markus Stockhausen <markus.stockhausen@gmx.de>
 */

#include <dt-bindings/clock/rtl83xx-clk.h>

#include "clk-rtl83xx.h"

#define rGLB	$t0
#define rCTR	$t1
#define rMSK	$t2
#define rSLP	$t3
#define rTMP	$t4

.set	noreorder

.globl	rtcl_838x_dram_start
rtcl_838x_dram_start:

/*
 * Functions start here and should avoid access to normal memory. REMARK! Do not forget about
 * stack pointer and dirty caches that might interfere.
 */

.globl	rtcl_838x_dram_set_rate
.ent	rtcl_838x_dram_set_rate
rtcl_838x_dram_set_rate:

#ifdef CONFIG_RTL838X

	li	rCTR, RTL_SW_CORE_BASE
	addiu	rGLB, rCTR, RTL838X_PLL_GLB_CTRL
	ori	rTMP, $0, CLK_CPU
	beq	$a0, rTMP, pre_cpu
	ori	rTMP, $0, CLK_MEM
	beq	$a0, rTMP, pre_mem
	nop
pre_lxb:
	ori	rSLP, $0, RTL838X_GLB_CTRL_LXB_PLL_READY_MASK
	addiu	rCTR, rCTR, RTL838X_PLL_LXB_CTRL0
	b	main_set
	ori	rMSK, $0, RTL838X_GLB_CTRL_EN_LXB_PLL_MASK
pre_mem:
	/* simple 64K data cache flush to avoid unexpected memory access */
	li	rMSK, RTL_SRAM_BASE
	li	rTMP, 2048
pre_flush:
	lw	$0, 0(rMSK)
	addiu	rMSK, rMSK, 32
	addiu	rTMP, rTMP, -1
	bne	rTMP, $0, pre_flush
	lw	$0, -4(rMSK)

	ori	rSLP, $0, RTL838X_GLB_CTRL_MEM_PLL_READY_MASK
	addiu	rCTR, rCTR, RTL838X_PLL_MEM_CTRL0
	b	main_set
	ori	rMSK, $0, RTL838X_GLB_CTRL_EN_MEM_PLL_MASK
pre_cpu:
	/* switch CPU to LXB clock */
	ori	rMSK, $0, RTL838X_GLB_CTRL_CPU_PLL_SC_MUX_MASK
	nor	rMSK, rMSK, $0
	sync
	lw	rTMP, 0(rGLB)
	and	rTMP, rTMP, rMSK
	sw	rTMP, 0(rGLB)
	sync

	ori	rSLP, $0, RTL838X_GLB_CTRL_CPU_PLL_READY_MASK
	addiu	rCTR, rCTR, RTL838X_PLL_CPU_CTRL0
	ori	rMSK, $0, RTL838X_GLB_CTRL_EN_CPU_PLL_MASK
main_set:
	/* disable PLL */
	nor	rMSK, rMSK, 0
	sync
	lw	rTMP, 0(rGLB)
	sync
	and	rTMP, rTMP, rMSK
	sync
	sw	rTMP, 0(rGLB)

	/* set new PLL values */
	sync
	sw	$a1, 0(rCTR)
	sw	$a2, 4(rCTR)
	sync

	/* enable PLL (will reset it and clear ready status) */
	nor	rMSK, rMSK, 0
	sync
	lw	rTMP, 0(rGLB)
	sync
	or	rTMP, rTMP, rMSK
	sync
	sw	rTMP, 0(rGLB)

	/* wait for PLL to become ready */
wait_ready:
	lw	rTMP, 0(rGLB)
	and	rTMP, rTMP, rSLP
	bne	rTMP, $0, wait_ready
	sync

	/* branch to post processing */
	ori	rTMP, $0, CLK_CPU
	beq	$a0, rTMP, post_cpu
	ori	rTMP, $0, CLK_MEM
	beq	$a0, rTMP, post_mem
	nop
post_lxb:
	jr	$ra
	nop
post_mem:
	jr	$ra
	nop
post_cpu:
	/* stabilize clock to avoid crash, empirically determined */
	ori	rSLP, $0, 0x3000
wait_cpu:
	bnez	rSLP, wait_cpu
	addiu	rSLP, rSLP, -1

	/* switch CPU to PLL clock */
	ori	rMSK, $0, RTL838X_GLB_CTRL_CPU_PLL_SC_MUX_MASK
	sync
	lw	rTMP, 0(rGLB)
	or	rTMP, rTMP, rMSK
	sw	rTMP, 0(rGLB)
	sync
	jr	$ra
	nop

#else /* !CONFIG_RTL838X */

	jr	$ra
	nop

#endif

.end	rtcl_838x_dram_set_rate

/*
 * End marker. Do not delete.
 */
	.word RTL_SRAM_MARKER
.globl	rtcl_838x_dram_size
rtcl_838x_dram_size:
	.word .-rtcl_838x_dram_start

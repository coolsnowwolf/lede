/* Copyright 2005 Oleg I. Vdovikin (oleg@cs.msu.su)	*/
/* cache manipulation adapted from Broadcom code 	*/
/* idea taken from original bunzip2 decompressor code	*/
/* Copyright 2004 Manuel Novoa III (mjn3@codepoet.org)	*/
/* Licensed under the linux kernel's version of the GPL.*/

#include <asm/asm.h>
#include <asm/regdef.h>

#define KSEG0		0x80000000

#define C0_CONFIG	$16
#define C0_TAGLO	$28
#define C0_TAGHI	$29

#define	CONF1_DA_SHIFT	7			/* D$ associativity */
#define CONF1_DA_MASK	0x00000380
#define CONF1_DA_BASE	1
#define CONF1_DL_SHIFT	10			/* D$ line size */
#define CONF1_DL_MASK	0x00001c00
#define CONF1_DL_BASE	2
#define CONF1_DS_SHIFT	13			/* D$ sets/way */
#define CONF1_DS_MASK	0x0000e000
#define CONF1_DS_BASE	64
#define CONF1_IA_SHIFT	16			/* I$ associativity */
#define CONF1_IA_MASK	0x00070000
#define CONF1_IA_BASE	1
#define CONF1_IL_SHIFT	19			/* I$ line size */
#define CONF1_IL_MASK	0x00380000
#define CONF1_IL_BASE	2
#define CONF1_IS_SHIFT	22			/* Instruction cache sets/way */
#define CONF1_IS_MASK	0x01c00000
#define CONF1_IS_BASE	64

#define Index_Invalidate_I	0x00
#define Index_Writeback_Inv_D   0x01

	.text
	LEAF(startup)
	.set noreorder
	li	sp, BZ_STACK_START
	addi    sp, -48
	sw      a0, 16(sp)
	sw      a1, 20(sp)
	sw      a2, 24(sp)
	sw      a3, 28(sp)

	/* Copy decompressor code to the right place */
	li	t2, BZ_TEXT_START
	add	a0, t2, 0
	la      a1, code_start
	la	a2, code_stop
$L1:
	lw	t0, 0(a1)
	sw	t0, 0(a0)
	add	a1, 4
	add	a0, 4
	blt	a1, a2, $L1
	nop

	/* At this point we need to invalidate dcache and */
	/* icache before jumping to new code */

1:	/* Get cache sizes */
	.set	mips32
	mfc0	s0,C0_CONFIG,1
	.set	mips0

	li	s1,CONF1_DL_MASK
	and	s1,s0
	beq	s1,zero,nodc
	nop

	srl	s1,CONF1_DL_SHIFT
	li	t0,CONF1_DL_BASE
	sll	s1,t0,s1		/* s1 has D$ cache line size */

	li	s2,CONF1_DA_MASK
	and	s2,s0
	srl	s2,CONF1_DA_SHIFT
	addiu	s2,CONF1_DA_BASE	/* s2 now has D$ associativity */

	li	t0,CONF1_DS_MASK
	and	t0,s0
	srl	t0,CONF1_DS_SHIFT
	li	s3,CONF1_DS_BASE
	sll	s3,s3,t0		/* s3 has D$ sets per way */

	multu	s2,s3			/* sets/way * associativity */
	mflo	t0			/* total cache lines */

	multu	s1,t0			/* D$ linesize * lines */
	mflo	s2			/* s2 is now D$ size in bytes */

	/* Initilize the D$: */
	mtc0	zero,C0_TAGLO
	mtc0	zero,C0_TAGHI

	li	t0,KSEG0		/* Just an address for the first $ line */
	addu	t1,t0,s2		/*  + size of cache == end */

	.set	mips3
1:	cache	Index_Writeback_Inv_D,0(t0)
	.set	mips0
	bne	t0,t1,1b
	addu	t0,s1

nodc:
	/* Now we get to do it all again for the I$ */

	move	s3,zero			/* just in case there is no icache */
	move	s4,zero

	li	t0,CONF1_IL_MASK
	and	t0,s0
	beq	t0,zero,noic
	nop

	srl	t0,CONF1_IL_SHIFT
	li	s3,CONF1_IL_BASE
	sll	s3,t0			/* s3 has I$ cache line size */

	li	t0,CONF1_IA_MASK
	and	t0,s0
	srl	t0,CONF1_IA_SHIFT
	addiu	s4,t0,CONF1_IA_BASE	/* s4 now has I$ associativity */

	li	t0,CONF1_IS_MASK
	and	t0,s0
	srl	t0,CONF1_IS_SHIFT
	li	s5,CONF1_IS_BASE
	sll	s5,t0			/* s5 has I$ sets per way */

	multu	s4,s5			/* sets/way * associativity */
	mflo	t0			/* s4 is now total cache lines */

	multu	s3,t0			/* I$ linesize * lines */
	mflo	s4			/* s4 is cache size in bytes */

	/* Initilize the I$: */
	mtc0	zero,C0_TAGLO
	mtc0	zero,C0_TAGHI

	li	t0,KSEG0		/* Just an address for the first $ line */
	addu	t1,t0,s4		/*  + size of cache == end */

	.set	mips3
1:	cache	Index_Invalidate_I,0(t0)
	.set	mips0
	bne	t0,t1,1b
	addu	t0,s3

noic:
	move	a0,s4			/* icache size */
	move	a1,s3			/* icache line size */
	move	a2,s2			/* dcache size */
	jal	t2
	move	a3,s1			/* dcache line size */

	.set reorder
	END(startup)

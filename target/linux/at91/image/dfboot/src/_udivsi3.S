/* # 1 "libgcc1.S" */
@ libgcc1 routines for ARM cpu.
@ Division routines, written by Richard Earnshaw, (rearnsha@armltd.co.uk)
dividend	.req	r0
divisor		.req	r1
result		.req	r2
curbit		.req	r3
/* ip		.req	r12	*/
/* sp		.req	r13	*/
/* lr		.req	r14	*/
/* pc		.req	r15	*/
	.text
	.globl	 __udivsi3
	.type  __udivsi3       ,function
	.align	0
 __udivsi3      :
	cmp	divisor, #0
	beq	Ldiv0
	mov	curbit, #1
	mov	result, #0
	cmp	dividend, divisor
	bcc	Lgot_result
Loop1:
	@ Unless the divisor is very big, shift it up in multiples of
	@ four bits, since this is the amount of unwinding in the main
	@ division loop.  Continue shifting until the divisor is
	@ larger than the dividend.
	cmp	divisor, #0x10000000
	cmpcc	divisor, dividend
	movcc	divisor, divisor, lsl #4
	movcc	curbit, curbit, lsl #4
	bcc	Loop1
Lbignum:
	@ For very big divisors, we must shift it a bit at a time, or
	@ we will be in danger of overflowing.
	cmp	divisor, #0x80000000
	cmpcc	divisor, dividend
	movcc	divisor, divisor, lsl #1
	movcc	curbit, curbit, lsl #1
	bcc	Lbignum
Loop3:
	@ Test for possible subtractions, and note which bits
	@ are done in the result.  On the final pass, this may subtract
	@ too much from the dividend, but the result will be ok, since the
	@ "bit" will have been shifted out at the bottom.
	cmp	dividend, divisor
	subcs	dividend, dividend, divisor
	orrcs	result, result, curbit
	cmp	dividend, divisor, lsr #1
	subcs	dividend, dividend, divisor, lsr #1
	orrcs	result, result, curbit, lsr #1
	cmp	dividend, divisor, lsr #2
	subcs	dividend, dividend, divisor, lsr #2
	orrcs	result, result, curbit, lsr #2
	cmp	dividend, divisor, lsr #3
	subcs	dividend, dividend, divisor, lsr #3
	orrcs	result, result, curbit, lsr #3
	cmp	dividend, #0			@ Early termination?
	movnes	curbit, curbit, lsr #4		@ No, any more bits to do?
	movne	divisor, divisor, lsr #4
	bne	Loop3
Lgot_result:
	mov	r0, result
	mov 	pc, lr
Ldiv0:
	str	lr, [sp, #-4]!
	bl	 __div0       (PLT)
	mov	r0, #0			@ about as wrong as it could be
	ldmia	sp!, {pc}
	.size  __udivsi3       , . -  __udivsi3
/* # 235 "libgcc1.S" */
/* # 320 "libgcc1.S" */
/* # 421 "libgcc1.S" */
/* # 433 "libgcc1.S" */
/* # 456 "libgcc1.S" */
/* # 500 "libgcc1.S" */
/* # 580 "libgcc1.S" */

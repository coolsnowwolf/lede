/* # 1 "libgcc1.S" */
@ libgcc1 routines for ARM cpu.
@ Division routines, written by Richard Earnshaw, (rearnsha@armltd.co.uk)
/* # 145 "libgcc1.S" */
dividend	.req	r0
divisor		.req	r1
overdone	.req	r2
curbit		.req	r3
/* ip		.req	r12	*/
/* sp		.req	r13	*/
/* lr		.req	r14	*/
/* pc		.req	r15	*/
	.text
	.globl	 __umodsi3
	.type  __umodsi3       ,function
	.align 0
 __umodsi3      :
	cmp	divisor, #0
	beq	Ldiv0
	mov	curbit, #1
	cmp	dividend, divisor
	movcc  	pc, lr
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
	@ Test for possible subtractions.  On the final pass, this may
	@ subtract too much from the dividend, so keep track of which
	@ subtractions are done, we can fix them up afterwards...
	mov	overdone, #0
	cmp	dividend, divisor
	subcs	dividend, dividend, divisor
	cmp	dividend, divisor, lsr #1
	subcs	dividend, dividend, divisor, lsr #1
	orrcs	overdone, overdone, curbit, ror #1
	cmp	dividend, divisor, lsr #2
	subcs	dividend, dividend, divisor, lsr #2
	orrcs	overdone, overdone, curbit, ror #2
	cmp	dividend, divisor, lsr #3
	subcs	dividend, dividend, divisor, lsr #3
	orrcs	overdone, overdone, curbit, ror #3
	mov	ip, curbit
	cmp	dividend, #0			@ Early termination?
	movnes	curbit, curbit, lsr #4		@ No, any more bits to do?
	movne	divisor, divisor, lsr #4
	bne	Loop3
	@ Any subtractions that we should not have done will be recorded in
	@ the top three bits of "overdone".  Exactly which were not needed
	@ are governed by the position of the bit, stored in ip.
	@ If we terminated early, because dividend became zero,
	@ then none of the below will match, since the bit in ip will not be
	@ in the bottom nibble.
	ands	overdone, overdone, #0xe0000000
	moveq  	pc, lr				@ No fixups needed
	tst	overdone, ip, ror #3
	addne	dividend, dividend, divisor, lsr #3
	tst	overdone, ip, ror #2
	addne	dividend, dividend, divisor, lsr #2
	tst	overdone, ip, ror #1
	addne	dividend, dividend, divisor, lsr #1
	mov 	pc, lr
Ldiv0:
	str	lr, [sp, #-4]!
	bl	 __div0       (PLT)
	mov	r0, #0			@ about as wrong as it could be
	ldmia	sp!, {pc}
	.size  __umodsi3       , . -  __umodsi3
/* # 320 "libgcc1.S" */
/* # 421 "libgcc1.S" */
/* # 433 "libgcc1.S" */
/* # 456 "libgcc1.S" */
/* # 500 "libgcc1.S" */
/* # 580 "libgcc1.S" */

#include <config.h>
#ifndef CONFIG_SPL_BUILD

.globl lowlevel_init
lowlevel_init:
	/*
	 * Copy exception table to relocated address in internal SRAM
	 */
	ldr	r0, src		/* Address of exception table in flash */
	ldr	r1, dest	/* Relocated address of exception table */
	ldmia	r0!, {r3-r10}	/* Copy exception table and jump values from */
	stmia	r1!, {r3-r10}	/* FLASH to relocated address */
	ldmia	r0!, {r3-r10}
	stmia	r1!, {r3-r10}
	mov	pc, lr

src:	.word CONFIG_SYS_TEXT_BASE
dest:	.word CONFIG_SRAM_BASE

#endif
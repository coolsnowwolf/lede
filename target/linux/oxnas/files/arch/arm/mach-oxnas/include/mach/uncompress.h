/* linux/include/asm-arm/arch-oxnas/uncompress.h
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifndef __ASM_ARCH_UNCOMPRESS_H
#define __ASM_ARCH_UNCOMPRESS_H

#define OXNAS_UART1_BASE 0x44200000

static inline void putc(int c)
{
	static volatile unsigned char *uart =
		(volatile unsigned char *)OXNAS_UART1_BASE;

	while (!(uart[5] & 0x20)) {	/* LSR reg THR empty bit */
		barrier();
	}
	uart[0] = c;			/* THR register */
}

static inline void flush(void)
{
}

#define arch_decomp_setup()

#define arch_decomp_wdog()

#endif /* __ASM_ARCH_UNCOMPRESS_H */

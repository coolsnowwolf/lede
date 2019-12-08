/*
 * Arch specific code for Ralink based boards
 *
 * Copyright (C) 2013 John Crispin <blogic@openwrt.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 */

#include <stddef.h>
#include "config.h"

#define READREG(r)		*(volatile unsigned int *)(r)
#define WRITEREG(r,v)		*(volatile unsigned int *)(r) = v

#define KSEG1ADDR(_x)		(((_x) & 0x1fffffff) | 0xa0000000)

#ifdef CONFIG_SOC_RT288X
#define UART_BASE		0xb0300c00
#else
#define UART_BASE		0xb0000c00
#endif

#define UART_TX			1
#define UART_LSR		7

#define UART_LSR_THRE		0x20

#define UART_READ(r)		READREG(UART_BASE + 4 * (r))
#define UART_WRITE(r,v)		WRITEREG(UART_BASE + 4 * (r), (v))

void board_putc(int ch)
{
	while (((UART_READ(UART_LSR)) & UART_LSR_THRE) == 0);
	UART_WRITE(UART_TX, ch);
	while (((UART_READ(UART_LSR)) & UART_LSR_THRE) == 0);
}

void board_init(void)
{
}

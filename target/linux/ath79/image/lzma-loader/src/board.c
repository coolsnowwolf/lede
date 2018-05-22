/*
 * LZMA compressed kernel loader for Atheros AR7XXX/AR9XXX based boards
 *
 * Copyright (C) 2011 Gabor Juhos <juhosg@openwrt.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 */

#include <stddef.h>
#include "config.h"
#include "ar71xx_regs.h"

#define READREG(r)	*(volatile unsigned int *)(r)
#define WRITEREG(r,v)	*(volatile unsigned int *)(r) = v

#define KSEG1ADDR(_x)	(((_x) & 0x1fffffff) | 0xa0000000)

#define UART_BASE	0xb8020000

#define UART_TX		0
#define UART_LSR	5

#define UART_LSR_THRE   0x20

#define UART_READ(r)		READREG(UART_BASE + 4 * (r))
#define UART_WRITE(r,v)		WRITEREG(UART_BASE + 4 * (r), (v))

void board_putc(int ch)
{
	while (((UART_READ(UART_LSR)) & UART_LSR_THRE) == 0);
	UART_WRITE(UART_TX, ch);
	while (((UART_READ(UART_LSR)) & UART_LSR_THRE) == 0);
}

#ifdef CONFIG_BOARD_TL_WR1043ND_V1
static void tlwr1043nd_init(void)
{
	unsigned int reg = KSEG1ADDR(AR71XX_RESET_BASE);
	unsigned int t;

	t = READREG(reg + AR913X_RESET_REG_RESET_MODULE);
	t |= AR71XX_RESET_GE0_PHY;
	WRITEREG(reg + AR913X_RESET_REG_RESET_MODULE, t);
	/* flush write */
	t = READREG(reg + AR913X_RESET_REG_RESET_MODULE);
}
#else
static inline void tlwr1043nd_init(void) {}
#endif

void board_init(void)
{
	tlwr1043nd_init();
}

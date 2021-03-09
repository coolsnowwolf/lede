/*
 * BCM63XX specific implementation parts
 *
 * Copyright (C) 2020 Álvaro Fernández Rojas <noltari@gmail.com>
 * Copyright (C) 2014 Jonas Gorski <jogo@openwrt.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 */

#include <stddef.h>
#include "config.h"

#define READREG(r)	*(volatile unsigned int *)(r)
#define WRITEREG(r,v)	*(volatile unsigned int *)(r) = v

#define UART_IR_REG	0x10
#define UART_FIFO_REG	0x14

static void wait_xfered(void)
{
        unsigned int val;

        do {
                val = READREG(UART_BASE + UART_IR_REG);
                if (val & (1 << 5))
                        break;
        } while (1);
}

void board_putc(int ch)
{
	if (!UART_BASE)
		return;

	wait_xfered();
	WRITEREG(UART_BASE + UART_FIFO_REG, ch);
	wait_xfered();
}

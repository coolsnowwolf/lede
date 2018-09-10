/*
 *  ADM5120 specific early printk support
 *
 *  Copyright (C) 2007-2008 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 */

#include <linux/init.h>
#include <linux/io.h>

#include <asm/addrspace.h>
#include <asm/mach-adm5120/adm5120_defs.h>
#include <asm/mach-adm5120/adm5120_uart.h>

#define UART_READ(r) \
	__raw_readl((void __iomem *)(KSEG1ADDR(ADM5120_UART0_BASE)+(r)))
#define UART_WRITE(r, v) \
	__raw_writel((v), (void __iomem *)(KSEG1ADDR(ADM5120_UART0_BASE)+(r)))

void __init prom_putchar(char ch)
{
	while ((UART_READ(UART_REG_FLAG) & UART_FLAG_TXFE) == 0)
		;
	UART_WRITE(UART_REG_DATA, ch);
	while ((UART_READ(UART_REG_FLAG) & UART_FLAG_TXFE) == 0)
		;
}

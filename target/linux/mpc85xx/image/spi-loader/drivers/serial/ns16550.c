// SPDX-License-Identifier: GPL-2.0
/*
 * 16550 serial console support.
 *
 * Original copied from <file:arch/ppc/boot/common/ns16550.c>
 * (which had no copyright)
 * Modifications: 2006 (c) MontaVista Software, Inc.
 *
 * Modified by: Mark A. Greer <mgreer@mvista.com>
 *
 * Adapted by:
 *
 * Copyright (c) 2022 Matthias Schiffer <mschiffer@universe-factory.net>
 */

#include <io.h>
#include <serial.h>

#define UART_DLL	0	/* Out: Divisor Latch Low */
#define UART_DLM	1	/* Out: Divisor Latch High */
#define UART_FCR	2	/* Out: FIFO Control Register */
#define UART_LCR	3	/* Out: Line Control Register */
#define UART_MCR	4	/* Out: Modem Control Register */
#define UART_LSR	5	/* In:  Line Status Register */
#define UART_LSR_THRE	0x20	/* Transmit-hold-register empty */
#define UART_LSR_DR	0x01	/* Receiver data ready */
#define UART_MSR	6	/* In:  Modem Status Register */
#define UART_SCR	7	/* I/O: Scratch Register */

static uint8_t *const reg_base = (uint8_t *)CONFIG_SERIAL_NS16550_REG_BASE;
static const int reg_shift = CONFIG_SERIAL_NS16550_REG_SHIFT;

void serial_console_putchar(char c)
{
	while ((in_8(reg_base + (UART_LSR << reg_shift)) & UART_LSR_THRE) == 0);
	out_8(reg_base, c);
}

int serial_console_getc(void)
{
	while ((in_8(reg_base + (UART_LSR << reg_shift)) & UART_LSR_DR) == 0);
	return in_8(reg_base);
}

int serial_console_tstc(void)
{
	return ((in_8(reg_base + (UART_LSR << reg_shift)) & UART_LSR_DR) != 0);
}

void serial_console_init(void)
{
	out_8(reg_base + (UART_FCR << reg_shift), 0x06);
}

/*
 *  ADM5120 UART definitions
 *
 *  This header file defines the hardware registers of the ADM5120 SoC
 *  built-in UARTs.
 *
 *  Copyright (C) 2007 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 */

#ifndef _MACH_ADM5120_UART_H
#define _MACH_ADM5120_UART_H

#define UART_BAUDDIV(clk, baud) ((clk/(16 * (baud)))-1)

#define UART_REG_DATA	0x00
#define UART_REG_RSR	0x04
#define UART_REG_ECR	UART_REG_RSR
#define UART_REG_LCRH	0x08
#define UART_REG_LCRM	0x0C
#define UART_REG_LCRL	0x10
#define UART_REG_CTRL	0x14
#define UART_REG_FLAG	0x18

/* Receive Status Register bits */
#define UART_RSR_FE	(1 << 0)
#define UART_RSR_PE	(1 << 1)
#define UART_RSR_BE	(1 << 2)
#define UART_RSR_OE	(1 << 3)
#define UART_RSR_ERR	(UART_RSR_FE | UART_RSR_PE | UART_RSR_BE)

#define UART_ECR_ALL	0xFF

/* Line Control High register bits */
#define UART_LCRH_BRK	(1 << 0) /* send break */
#define UART_LCRH_PEN	(1 << 1) /* parity enable */
#define UART_LCRH_EPS	(1 << 2) /* even parity select */
#define UART_LCRH_STP1	(0 << 3) /* one stop bits select */
#define UART_LCRH_STP2	(1 << 3) /* two stop bits select */
#define UART_LCRH_FEN	(1 << 4) /* FIFO enable */

#define UART_LCRH_WLEN5	(0 << 5)
#define UART_LCRH_WLEN6	(1 << 5)
#define UART_LCRH_WLEN7	(2 << 5)
#define UART_LCRH_WLEN8	(3 << 5)

/* Control register bits */
#define UART_CTRL_EN	(1 << 0)

/* Flag register bits */
#define UART_FLAG_CTS	(1 << 0)
#define UART_FLAG_DSR	(1 << 1)
#define UART_FLAG_DCD	(1 << 2)
#define UART_FLAG_BUSY	(1 << 3)
#define UART_FLAG_RXFE	(1 << 4)
#define UART_FLAG_TXFF	(1 << 5)
#define UART_FLAG_RXFF	(1 << 6)
#define UART_FLAG_TXFE	(1 << 7)

#endif /* _MACH_ADM5120_UART_H */

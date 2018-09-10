/*
 * ADM5120 specific board support for LZMA decompressor
 *
 * Copyright (C) 2007-2008 OpenWrt.org
 * Copyright (C) 2007-2008 Gabor Juhos <juhosg@openwrt.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "config.h"
#include <stddef.h>

#define READREG(r)	*(volatile unsigned int *)(r)
#define WRITEREG(r,v)	*(volatile unsigned int *)(r) = v

/*
 * INTC definitions
 */
#define INTC_BASE	0xB2200000

/* INTC registers */
#define INTC_REG_IRQ_DISABLE	0x0C

/*
 * UART definitions
 */
#define UART0_BASE	0xB2600000
#define UART1_BASE	0xB2800000
/* UART registers */
#define UART_REG_DATA	0x00	/* Data register */
#define UART_REG_ECR	0x04	/* Error Clear register */
#define UART_REG_LCRH	0x08	/* Line Control High register */
#define UART_REG_LCRM	0x0C	/* Line Control Middle register */
#define UART_REG_LCRL	0x10	/* Line Control Low register */
#define UART_REG_CTRL	0x14	/* Control register */
#define UART_REG_FLAG   0x18	/* Flag register */

/* Control register bits */
#define UART_CTRL_EN	( 1 << 0 )	/* UART enable */

/* Line Control High register bits */
#define UART_LCRH_FEN	( 1 << 4 )	/* FIFO enable */

/* Flag register bits */
#define UART_FLAG_CTS	( 1 << 0 )
#define UART_FLAG_DSR	( 1 << 1 )
#define UART_FLAG_DCD	( 1 << 2 )
#define UART_FLAG_BUSY	( 1 << 3 )
#define UART_FLAG_RXFE	( 1 << 4 )	/* RX FIFO empty */
#define UART_FLAG_TXFF	( 1 << 5 )	/* TX FIFO full */
#define UART_FLAG_RXFF	( 1 << 6 )	/* RX FIFO full */
#define UART_FLAG_TXFE	( 1 << 7 )	/* TX FIFO empty */

/*
 * SWITCH definitions
 */
#define SWITCH_BASE	0xB2000000

#define SWITCH_REG_CPUP_CONF	0x0024
#define SWITCH_REG_PORT_CONF0	0x0028

#define SWITCH_REG_GPIO_CONF0	0x00B8
#define SWITCH_REG_GPIO_CONF2	0x00BC

#define SWITCH_REG_PORT0_LED	0x0100
#define SWITCH_REG_PORT1_LED	0x0104
#define SWITCH_REG_PORT2_LED	0x0108
#define SWITCH_REG_PORT3_LED	0x010C
#define SWITCH_REG_PORT4_LED	0x0110

#define SWITCH_PORTS_HW		0x3F		/* Hardware Ports */

/* CPUP_CONF register bits */
#define CPUP_CONF_DCPUP		( 1 << 0 )	/* Disable CPU port */

/* PORT_CONF0 register bits */
#define PORT_CONF0_DP_SHIFT	0	/* disable port shift*/


/*
 * UART routines
 */

#if defined(CONFIG_USE_UART0)
#  define UART_READ(r)		READREG(UART0_BASE+(r))
#  define UART_WRITE(r,v)	WRITEREG(UART0_BASE+(r),(v))
#else
#  define UART_READ(r)		READREG(UART1_BASE+(r))
#  define UART_WRITE(r,v)	WRITEREG(UART1_BASE+(r),(v))
#endif

static void uart_init(void)
{
#if 0
	unsigned int t;

	/* disable uart */
	UART_WRITE(UART_REG_CTRL, 0);

	/* keep current baud rate */
	t = UART_READ(UART_REG_LCRM);
	UART_WRITE(UART_REG_LCRM, t);
	t = UART_READ(UART_REG_LCRL);
	UART_WRITE(UART_REG_LCRL, t);

	/* keep data, stop, and parity bits, but disable FIFO */
	t = UART_READ(UART_REG_LCRH);
	t &= ~(UART_LCRH_FEN);
	UART_WRITE(UART_REG_LCRH, t );

	/* clear error bits */
	UART_WRITE(UART_REG_ECR, 0xFF);

	/* enable uart, and disable interrupts */
	UART_WRITE(UART_REG_CTRL, UART_CTRL_EN);
#endif
}

/*
 * INTC routines
 */

#define INTC_READ(r)	READREG(INTC_BASE+(r))
#define INTC_WRITE(r,v)	WRITEREG(INTC_BASE+(r),v)

static void intc_init(void)
{
	INTC_WRITE(INTC_REG_IRQ_DISABLE, 0xFFFFFFFF);
}

/*
 * SWITCH routines
 */

#define SWITCH_READ(r)		READREG(SWITCH_BASE+(r))
#define SWITCH_WRITE(r,v)	WRITEREG(SWITCH_BASE+(r),v)

static void switch_init(void)
{
	/* disable PHYS ports */
	SWITCH_WRITE(SWITCH_REG_PORT_CONF0,
	    (SWITCH_PORTS_HW << PORT_CONF0_DP_SHIFT));

	/* disable CPU port */
	SWITCH_WRITE(SWITCH_REG_CPUP_CONF, CPUP_CONF_DCPUP);

	/* disable GPIO lines */
	SWITCH_WRITE(SWITCH_REG_GPIO_CONF0, 0);
	SWITCH_WRITE(SWITCH_REG_GPIO_CONF2, 0);

	/* disable LED lines */
	SWITCH_WRITE(SWITCH_REG_PORT0_LED, 0);
	SWITCH_WRITE(SWITCH_REG_PORT1_LED, 0);
	SWITCH_WRITE(SWITCH_REG_PORT2_LED, 0);
	SWITCH_WRITE(SWITCH_REG_PORT3_LED, 0);
	SWITCH_WRITE(SWITCH_REG_PORT4_LED, 0);
}

void board_putc(int ch)
{
	while ((UART_READ(UART_REG_FLAG) & UART_FLAG_TXFE) == 0);

	UART_WRITE(UART_REG_DATA, ch);

	while ((UART_READ(UART_REG_FLAG) & UART_FLAG_TXFE) == 0);
}

void board_init(void)
{
	intc_init();
	switch_init();
	uart_init();
}

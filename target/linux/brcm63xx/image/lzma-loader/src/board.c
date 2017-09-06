/*
 * BCM63XX specific implementation parts
 *
 * Copyright (C) 2014 Jonas Gorski <jogo@openwrt.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 */

#include <stddef.h>
#include "config.h"
#include "cp0regdef.h"

#define READREG(r)	*(volatile unsigned int *)(r)
#define WRITEREG(r,v)	*(volatile unsigned int *)(r) = v

#define UART_IR_REG	0x10
#define UART_FIFO_REG	0x14

unsigned long uart_base;

static void wait_xfered(void)
{
        unsigned int val;

        do {
                val = READREG(uart_base + UART_IR_REG);
                if (val & (1 << 5))
                        break;
        } while (1);
}

void board_putc(int ch)
{
	if (!uart_base)
		return;

	wait_xfered();
        WRITEREG(uart_base + UART_FIFO_REG, ch);
	wait_xfered();
}

#define PRID_IMP_BMIPS32_REV4	0x4000
#define PRID_IMP_BMIPS32_REV8	0x8000
#define PRID_IMP_BMIPS3300	0x9000
#define PRID_IMP_BMIPS3300_ALT	0x9100
#define PRID_IMP_BMIPS3300_BUG	0x0000
#define PRID_IMP_BMIPS43XX	0xa000

void board_init(void)
{
	unsigned long prid, chipid, chipid_reg;

	prid = read_32bit_c0_register($15, 0);

	switch (prid & 0xff00) {
	case PRID_IMP_BMIPS32_REV4:
	case PRID_IMP_BMIPS32_REV8:
	case PRID_IMP_BMIPS3300_ALT:
	case PRID_IMP_BMIPS3300_BUG:
		chipid_reg = 0xfffe0000;
		break;
	case PRID_IMP_BMIPS3300:
		if ((prid & 0xff) >= 0x33)
			chipid_reg = 0xb0000000;
		else
			chipid_reg = 0xfffe0000;
		break;
	case PRID_IMP_BMIPS43XX:
		if ((prid & 0xff) == 0x04)
			chipid_reg = 0xfff8c000;
		else if ((prid & 0xff) == 0x70)
			return;	/* FIXME: 0002a070 can be 6362 and 3380 */
		else if ((prid & 0xff) >= 0x30)
			chipid_reg = 0xb0000000;
		else
			chipid_reg = 0xfffe0000;
		break;
	default:
		return;
	}

	chipid = READREG(chipid_reg);

	switch (chipid >> 16) {
	case 0x3368:
	case 0x6318:
	case 0x6328:
	case 0x6358:
	case 0x6362:
	case 0x6368:
	case 0x6369:
		uart_base = chipid_reg + 0x100;
		break;
	case 0x6316:
	case 0x6326:
		uart_base = chipid_reg + 0x180;
		break;
	case 0x3380:
		uart_base = chipid_reg + 0x200;
		break;
	case 0x6338:
	case 0x6345:
	case 0x6348:
		uart_base = chipid_reg + 0x300;
		break;
	default:
		return;
	}
}

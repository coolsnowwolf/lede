/*
 *  Copyright (C) 2007-2008 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 */

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/io.h>

#include <asm/addrspace.h>

#include <asm/mach-adm5120/adm5120_info.h>
#include <asm/mach-adm5120/adm5120_defs.h>
#include <asm/mach-adm5120/adm5120_switch.h>

unsigned int adm5120_product_code;
unsigned int adm5120_revision;
unsigned int adm5120_package;
unsigned int adm5120_nand_boot;
unsigned long adm5120_speed;

/*
 * CPU settings detection
 */
#define CODE_GET_PC(c)		((c) & CODE_PC_MASK)
#define CODE_GET_REV(c)		(((c) >> CODE_REV_SHIFT) & CODE_REV_MASK)
#define CODE_GET_PK(c)		(((c) >> CODE_PK_SHIFT) & CODE_PK_MASK)
#define CODE_GET_CLKS(c)	(((c) >> CODE_CLKS_SHIFT) & CODE_CLKS_MASK)
#define CODE_GET_NAB(c)		(((c) & CODE_NAB) != 0)

void adm5120_ndelay(u32 ns)
{
	u32	t;

	SW_WRITE_REG(SWITCH_REG_TIMER, TIMER_PERIOD_DEFAULT);
	SW_WRITE_REG(SWITCH_REG_TIMER_INT, (TIMER_INT_TOS | TIMER_INT_TOM));

	t = (ns+640) / 640;
	t &= TIMER_PERIOD_MASK;
	SW_WRITE_REG(SWITCH_REG_TIMER, t | TIMER_TE);

	/* wait until the timer expires */
	do {
		t = SW_READ_REG(SWITCH_REG_TIMER_INT);
	} while ((t & TIMER_INT_TOS) == 0);

	/* leave the timer disabled */
	SW_WRITE_REG(SWITCH_REG_TIMER, TIMER_PERIOD_DEFAULT);
	SW_WRITE_REG(SWITCH_REG_TIMER_INT, (TIMER_INT_TOS | TIMER_INT_TOM));
}

void __init adm5120_soc_init(void)
{
	u32 code;
	u32 clks;

	code = SW_READ_REG(SWITCH_REG_CODE);

	adm5120_product_code = CODE_GET_PC(code);
	adm5120_revision = CODE_GET_REV(code);
	adm5120_package = (CODE_GET_PK(code) == CODE_PK_BGA) ?
		ADM5120_PACKAGE_BGA : ADM5120_PACKAGE_PQFP;
	adm5120_nand_boot = CODE_GET_NAB(code);

	clks = CODE_GET_CLKS(code);
	adm5120_speed = ADM5120_SPEED_175;
	if (clks & 1)
		adm5120_speed += 25000000;
	if (clks & 2)
		adm5120_speed += 50000000;
}

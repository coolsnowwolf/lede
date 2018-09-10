/*
 *  Mikrotik RouterBOARD 192 support
 *
 *  Copyright (C) 2007-2008 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 */

#include "rb-1xx.h"

static u8 rb192_vlans[6] __initdata = {
	0x7F, 0x00, 0x00, 0x00, 0x00, 0x00
};

static void __init rb192_setup(void)
{
	rb1xx_generic_setup();
	rb1xx_add_device_nand();

	adm5120_add_device_switch(6, rb192_vlans);
}

MIPS_MACHINE(MACH_ADM5120_RB_192, "192", "Mikrotik RouterBOARD 192",
	     rb192_setup);

/*
 *  ZyXEL Prestige P-335/335WT support
 *
 *  Copyright (C) 2007-2008 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 */

#include "p-33x.h"

static void __init p335_setup(void)
{
	p33x_generic_setup();
	adm5120_add_device_usb();
}

MIPS_MACHINE(MACH_ADM5120_P335, "P-335", "ZyXEL Prestige 335/335WT",
	     p335_setup);

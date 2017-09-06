/*
 *  Infineon EASY 83000 Reference Board support
 *
 *  Copyright (C) 2007-2008 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 */

#include "infineon.h"

static void __init easy83000_setup(void)
{
	easy_setup_pqfp();
	adm5120_add_device_switch(6, NULL);

	/* TODO: add VINAX device */
}

MIPS_MACHINE(MACH_ADM5120_EASY83000, "EASY8300",
	     "Infineon EASY 83000 Reference Board", easy83000_setup);

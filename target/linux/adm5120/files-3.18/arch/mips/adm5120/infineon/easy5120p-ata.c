/*
 *  Infineon EASY 5120P-ATA Reference Board support
 *
 *  Copyright (C) 2007-2008 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 */

#include "infineon.h"

static void __init easy5120pata_setup(void)
{
	easy_setup_pqfp();

	adm5120_add_device_switch(6, NULL);
}

MIPS_MACHINE(MACH_ADM5120_EASY5120PATA, "EASY5120P-ATA",
	     "Infineon EASY 5120P-ATA Reference Board", easy5120pata_setup);

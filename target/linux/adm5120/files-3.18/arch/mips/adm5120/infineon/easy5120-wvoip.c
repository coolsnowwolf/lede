/*
 *  Infineon EASY 5120-WVoIP Reference Board support
 *
 *  Copyright (C) 2007-2008 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 */

#include "infineon.h"

static void __init easy5120wvoip_setup(void)
{
	easy_setup_bga();
	adm5120_add_device_switch(6, NULL);

	/* TODO: add VINETIC2 device */
	/* TODO: setup PCI IRQ map */
}

MIPS_MACHINE(MACH_ADM5120_EASY5120WVOIP, "EASY5120WVoIP",
	     "Infineon EASY 5120-WVoIP Reference Board", easy5120wvoip_setup);

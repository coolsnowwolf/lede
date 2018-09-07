/*
 *  Compex NP27G board support
 *
 *  Copyright (C) 2007-2008 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 */

#include "compex.h"

static u8 np27g_vlans[6] __initdata = {
	/* FIXME: untested */
	0x41, 0x42, 0x44, 0x48, 0x50, 0x00
};

static void __init np27g_setup(void)
{
	compex_generic_setup();
	adm5120_add_device_switch(5, np27g_vlans);
	adm5120_add_device_usb();

	/* TODO: add PCI IRQ map */
}

MIPS_MACHINE(MACH_ADM5120_NP27G, "NP27G", "Compex NetPassage 27G", np27g_setup);

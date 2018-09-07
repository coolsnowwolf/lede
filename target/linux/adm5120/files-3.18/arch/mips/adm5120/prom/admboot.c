/*
 *  ADMBoot specific prom routines
 *
 *  Copyright (C) 2008 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 */

#include <linux/types.h>
#include <linux/init.h>
#include <linux/errno.h>

#include <asm/addrspace.h>
#include <asm/byteorder.h>

#include <asm/mach-adm5120/adm5120_defs.h>
#include <prom/admboot.h>
#include "prom_read.h"

#define ADMBOOT_MAGIC_MAC_BASE		0x636D676D	/* 'mgmc' */
#define ADMBOOT_MAGIC_MAC_BASE_BR6104XX 0x31305348	/* 'HS01' */

int __init admboot_get_mac_base(u32 offset, u32 len, u8 *mac)
{
	u8 *cfg;
	int i;

	cfg = (u8 *) KSEG1ADDR(ADM5120_SRAM0_BASE + offset);
	for (i = 0; i < len; i += 4) {
		u32 magic;

		magic = prom_read_le32(cfg + i);
		if (magic == ADMBOOT_MAGIC_MAC_BASE) {
			int j;

			for (j = 0; j < 6; j++)
				mac[j] = cfg[i + 4 + j];

			return 0;
		}
		if (magic == ADMBOOT_MAGIC_MAC_BASE_BR6104XX) {
			int j;

			for (j = 0; j < 6; j++)
				mac[j] = cfg[i + 7 + j];

			return 0;
		}
	}

	return -ENXIO;
}

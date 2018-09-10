/*
 *  Mikrotik's RouterBOOT specific prom routines
 *
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
#include <linux/string.h>
#include <linux/module.h>
#include <linux/routerboot.h>

#include <asm/bootinfo.h>
#include <asm/addrspace.h>

#include <asm/mach-adm5120/adm5120_defs.h>
#include <prom/routerboot.h>
#include "prom_read.h"

struct rb_hard_settings rb_hs;
static int rb_found;

static int __init routerboot_load_hs(u8 *buf, u16 buflen)
{
	u16 id, len;

	memset(&rb_hs, 0, sizeof(rb_hs));

	if (buflen < 4)
		return -1;

	if (prom_read_le32(buf) != RB_MAGIC_HARD)
		return -1;

	/* skip magic value */
	buf += 4;
	buflen -= 4;

	while (buflen > 2) {
		id = prom_read_le16(buf);
		buf += 2;
		buflen -= 2;
		if (id == RB_ID_TERMINATOR || buflen < 2)
			break;

		len = prom_read_le16(buf);
		buf += 2;
		buflen -= 2;

		if (buflen < len)
			break;

		switch (id) {
		case RB_ID_BIOS_VERSION:
			rb_hs.bios_ver = (char *)buf;
			break;
		case RB_ID_BOARD_NAME:
			rb_hs.name = (char *)buf;
			break;
		case RB_ID_MEMORY_SIZE:
			rb_hs.mem_size = prom_read_le32(buf);
			break;
		case RB_ID_MAC_ADDRESS_COUNT:
			rb_hs.mac_count = prom_read_le32(buf);
			break;
		case RB_ID_MAC_ADDRESS_PACK:
			if ((len / RB_MAC_SIZE) > 0)
				rb_hs.mac_base = buf;
			break;
		}

		buf += len;
		buflen -= len;

	}

	return 0;
}

#define RB_BS_OFFS	0x14
#define RB_OFFS_MAX	(128*1024)

int __init routerboot_present(void)
{
	struct rb_bios_settings	*bs;
	u8 *base;
	u32 off, len;

	if (rb_found)
		goto out;

	base = (u8 *)KSEG1ADDR(ADM5120_SRAM0_BASE);
	bs = (struct rb_bios_settings *)(base + RB_BS_OFFS);

	off = prom_read_le32(&bs->hs_offs);
	len = prom_read_le32(&bs->hs_size);
	if (off > RB_OFFS_MAX)
		goto out;

	if (routerboot_load_hs(base+off, len) != 0)
		goto out;

	rb_found = 1;

out:
	return rb_found;
}

char *routerboot_get_boardname(void)
{
	if (rb_found == 0)
		return NULL;

	return rb_hs.name;
}

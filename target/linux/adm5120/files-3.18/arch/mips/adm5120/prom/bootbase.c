/*
 *  ZyXEL's Bootbase specific prom routines
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

#include <asm/bootinfo.h>
#include <asm/addrspace.h>
#include <asm/byteorder.h>

#include <asm/mach-adm5120/adm5120_defs.h>
#include <prom/zynos.h>
#include "prom_read.h"

#define ZYNOS_INFO_ADDR		KSEG1ADDR(ADM5120_SRAM0_BASE+0x3F90)
#define ZYNOS_HDBG_ADDR		KSEG1ADDR(ADM5120_SRAM0_BASE+0x4000)
#define BOOTEXT_ADDR_MIN	KSEG1ADDR(ADM5120_SRAM0_BASE)
#define BOOTEXT_ADDR_MAX	(BOOTEXT_ADDR_MIN + (2*1024*1024))

static int bootbase_found;
static struct zynos_board_info *board_info;

struct bootbase_info bootbase_info;

static inline int bootbase_dbgarea_present(u8 *data)
{
	u32 t;

	t = prom_read_be32(data+5);
	if (t != ZYNOS_MAGIC_DBGAREA1)
		return 0;

	t = prom_read_be32(data+9);
	if (t != ZYNOS_MAGIC_DBGAREA2)
		return 0;

	return 1;
}

static inline u32 bootbase_get_bootext_addr(void)
{
	return prom_read_be32(&board_info->bootext_addr);
}

static inline void bootbase_get_mac(u8 *mac)
{
	int	i;

	for (i = 0; i < 6; i++)
		mac[i] = board_info->mac[i];
}

static inline u16 bootbase_get_vendor_id(void)
{
#define CHECK_VENDOR(n) (strnicmp(board_info->vendor, (n), strlen(n)) == 0)
	unsigned char vendor[ZYNOS_NAME_LEN];
	int i;

	for (i = 0; i < ZYNOS_NAME_LEN; i++)
		vendor[i] = board_info->vendor[i];

	if CHECK_VENDOR(ZYNOS_VENDOR_ZYXEL)
		return ZYNOS_VENDOR_ID_ZYXEL;

	if CHECK_VENDOR(ZYNOS_VENDOR_DLINK)
		return ZYNOS_VENDOR_ID_DLINK;

	if CHECK_VENDOR(ZYNOS_VENDOR_LUCENT)
		return ZYNOS_VENDOR_ID_LUCENT;

	if CHECK_VENDOR(ZYNOS_VENDOR_NETGEAR)
		return ZYNOS_VENDOR_ID_NETGEAR;

	return ZYNOS_VENDOR_ID_OTHER;
}

static inline u16 bootbase_get_board_id(void)
{
	return prom_read_be16(&board_info->board_id);
}

int __init bootbase_present(void)
{
	u32	t;

	if (bootbase_found)
		goto out;

	/* check presence of the dbgarea */
	if (bootbase_dbgarea_present((u8 *)ZYNOS_HDBG_ADDR) == 0)
		goto out;

	board_info = (struct zynos_board_info *)(ZYNOS_INFO_ADDR);

	/* check for a valid BootExt address */
	t = bootbase_get_bootext_addr();
	if ((t < BOOTEXT_ADDR_MIN) || (t > BOOTEXT_ADDR_MAX))
		goto out;

	bootbase_info.vendor_id = bootbase_get_vendor_id();
	bootbase_info.board_id = bootbase_get_board_id();
	bootbase_get_mac(bootbase_info.mac);

	bootbase_found = 1;

out:
	return bootbase_found;
}


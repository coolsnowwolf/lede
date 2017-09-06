/*
 *  Compex's MyLoader specific prom routines
 *
 *  Copyright (C) 2007-2008 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/string.h>

#include <asm/addrspace.h>
#include <asm/fw/myloader/myloader.h>

#define SYS_PARAMS_ADDR		KSEG1ADDR(0x80000800)
#define BOARD_PARAMS_ADDR	KSEG1ADDR(0x80000A00)
#define PART_TABLE_ADDR		KSEG1ADDR(0x80000C00)
#define BOOT_PARAMS_ADDR	KSEG1ADDR(0x80000E00)

static struct myloader_info myloader_info __initdata;
static int myloader_found __initdata;

struct myloader_info * __init myloader_get_info(void)
{
	struct mylo_system_params *sysp;
	struct mylo_board_params *boardp;
	struct mylo_partition_table *parts;

	if (myloader_found)
		return &myloader_info;

	sysp = (struct mylo_system_params *)(SYS_PARAMS_ADDR);
	boardp = (struct mylo_board_params *)(BOARD_PARAMS_ADDR);
	parts = (struct mylo_partition_table *)(PART_TABLE_ADDR);

	printk(KERN_DEBUG "MyLoader: sysp=%08x, boardp=%08x, parts=%08x\n",
		sysp->magic, boardp->magic, parts->magic);

	/* Check for some magic numbers */
	if (sysp->magic != MYLO_MAGIC_SYS_PARAMS ||
	    boardp->magic != MYLO_MAGIC_BOARD_PARAMS ||
	    le32_to_cpu(parts->magic) != MYLO_MAGIC_PARTITIONS)
		return NULL;

	printk(KERN_DEBUG "MyLoader: id=%04x:%04x, sub_id=%04x:%04x\n",
		sysp->vid, sysp->did, sysp->svid, sysp->sdid);

	myloader_info.vid = sysp->vid;
	myloader_info.did = sysp->did;
	myloader_info.svid = sysp->svid;
	myloader_info.sdid = sysp->sdid;

	memcpy(myloader_info.macs, boardp->addr, sizeof(myloader_info.macs));

	myloader_found = 1;

	return &myloader_info;
}

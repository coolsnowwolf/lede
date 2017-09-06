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

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/string.h>

#include <asm/bootinfo.h>
#include <asm/addrspace.h>
#include <asm/byteorder.h>

#include <asm/mach-adm5120/adm5120_defs.h>
#include <prom/myloader.h>
#include "prom_read.h"

#define SYS_PARAMS_ADDR		KSEG1ADDR(ADM5120_SRAM0_BASE+0x0F000)
#define BOARD_PARAMS_ADDR	KSEG1ADDR(ADM5120_SRAM0_BASE+0x0F800)
#define PART_TABLE_ADDR		KSEG1ADDR(ADM5120_SRAM0_BASE+0x10000)

static int myloader_found;

struct myloader_info myloader_info;

int __init myloader_present(void)
{
	struct mylo_system_params *sysp;
	struct mylo_board_params *boardp;
	struct mylo_partition_table *parts;
	int i;

	if (myloader_found)
		goto out;

	sysp = (struct mylo_system_params *)(SYS_PARAMS_ADDR);
	boardp = (struct mylo_board_params *)(BOARD_PARAMS_ADDR);
	parts = (struct mylo_partition_table *)(PART_TABLE_ADDR);

	/* Check for some magic numbers */
	if ((le32_to_cpu(sysp->magic) != MYLO_MAGIC_SYS_PARAMS) ||
	   (le32_to_cpu(boardp->magic) != MYLO_MAGIC_BOARD_PARAMS) ||
	   (le32_to_cpu(parts->magic) != MYLO_MAGIC_PARTITIONS))
		goto out;

	myloader_info.vid = le32_to_cpu(sysp->vid);
	myloader_info.did = le32_to_cpu(sysp->did);
	myloader_info.svid = le32_to_cpu(sysp->svid);
	myloader_info.sdid = le32_to_cpu(sysp->sdid);

	for (i = 0; i < MYLO_ETHADDR_COUNT; i++) {
		int j;
		for (j = 0; j < 6; j++)
			myloader_info.macs[i][j] = boardp->addr[i].mac[j];
	}

	myloader_found = 1;

out:
	return myloader_found;
}

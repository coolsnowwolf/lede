// SPDX-License-Identifier: GPL-2.0+

/*
 * Sophos RED 15w Rev.1 Board Setup
 *
 * Copyright (C) 2019 David Bauer <mail@david-bauer.net>
 *
 * Based on:
 *   p1010rdb.c:
 *      P1010 RDB Board Setup
 *      Copyright 2011 Freescale Semiconductor Inc.
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */

#include <linux/stddef.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/of_platform.h>

#include <asm/time.h>
#include <asm/machdep.h>
#include <asm/pci-bridge.h>
#include <mm/mmu_decl.h>
#include <asm/prom.h>
#include <asm/udbg.h>
#include <asm/mpic.h>

#include <sysdev/fsl_soc.h>
#include <sysdev/fsl_pci.h>

#include "mpc85xx.h"

void __init red_15w_rev1_pic_init(void)
{
	struct mpic *mpic;

	mpic = mpic_alloc(NULL, 0,
	  MPIC_BIG_ENDIAN | MPIC_SINGLE_DEST_CPU,
	  0, 256, " OpenPIC  ");

	BUG_ON(mpic == NULL);
	mpic_init(mpic);
}

/*
 * Setup the architecture
 */
static void __init red_15w_rev1_setup_arch(void)
{
	if (ppc_md.progress)
		ppc_md.progress("red_15w_rev1_setup_arch()", 0);

	fsl_pci_assign_primary();

	pr_info("RED 15w Rev.1 from Sophos\n");
}

machine_arch_initcall(red_15w_rev1, mpc85xx_common_publish_devices);

/*
 * Called very early, device-tree isn't unflattened
 */
static int __init red_15w_rev1_probe(void)
{
	if (of_machine_is_compatible("sophos,red-15w-rev1"))
		return 1;
	return 0;
}

define_machine(red_15w_rev1) {
	.name			= "P1010 RDB",
	.probe			= red_15w_rev1_probe,
	.setup_arch		= red_15w_rev1_setup_arch,
	.init_IRQ		= red_15w_rev1_pic_init,
#ifdef CONFIG_PCI
	.pcibios_fixup_bus	= fsl_pcibios_fixup_bus,
	.pcibios_fixup_phb      = fsl_pcibios_fixup_phb,
#endif
	.get_irq		= mpic_get_irq,
	.calibrate_decr		= generic_calibrate_decr,
	.progress		= udbg_progress,
};

// SPDX-License-Identifier: GPL-2.0-or-later

/*
 * Enterasys WS-AP3715i Board Setup
 *
 * Copyright (C) 2023 David Bauer <mail@david-bauer.net>
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

static void __init wsap3715i_pic_init(void)
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
static void __init wsap3715i_setup_arch(void)
{
	if (ppc_md.progress)
		ppc_md.progress("wsap3715i_setup_arch()", 0);

	fsl_pci_assign_primary();

	pr_info("WS-AP3715i from Enterasys\n");
}

machine_arch_initcall(wsap3715i, mpc85xx_common_publish_devices);

define_machine(wsap3715i) {
	.name			= "P1010 RDB",
	.compatible		= "enterasys,ws-ap3715i",
	.setup_arch		= wsap3715i_setup_arch,
	.init_IRQ		= wsap3715i_pic_init,
#ifdef CONFIG_PCI
	.pcibios_fixup_bus	= fsl_pcibios_fixup_bus,
	.pcibios_fixup_phb      = fsl_pcibios_fixup_phb,
#endif
	.get_irq		= mpic_get_irq,
	.progress		= udbg_progress,
};

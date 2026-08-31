/*
 * TL-WDR4900 v1 board setup
 *
 * Copyright (c) 2013 Gabor Juhos <juhosg@openwrt.org>
 *
 * Based on:
 *   p1010rdb.c:
 *      P1010RDB Board Setup
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

static void __init tl_wdr4900_v1_pic_init(void)
{
	struct mpic *mpic = mpic_alloc(NULL, 0, MPIC_BIG_ENDIAN |
	  MPIC_SINGLE_DEST_CPU,
	  0, 256, " OpenPIC  ");

	BUG_ON(mpic == NULL);

	mpic_init(mpic);
}

/*
 * Setup the architecture
 */
static void __init tl_wdr4900_v1_setup_arch(void)
{
	if (ppc_md.progress)
		ppc_md.progress("tl_wdr4900_v1_setup_arch()", 0);

	fsl_pci_assign_primary();

	printk(KERN_INFO "TL-WDR4900 v1 board from TP-Link\n");
}

machine_arch_initcall(tl_wdr4900_v1, mpc85xx_common_publish_devices);

define_machine(tl_wdr4900_v1) {
	.name			= "Freescale P1014",
	.compatible		= "tplink,tl-wdr4900-v1",
	.setup_arch		= tl_wdr4900_v1_setup_arch,
	.init_IRQ		= tl_wdr4900_v1_pic_init,
#ifdef CONFIG_PCI
	.pcibios_fixup_bus	= fsl_pcibios_fixup_bus,
#endif
	.get_irq		= mpic_get_irq,
	.progress		= udbg_progress,
};

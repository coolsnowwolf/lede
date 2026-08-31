// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Aerohive BR200-WP Board Setup
 * Copyright (C) 2023 Pawel Dembicki <paweldembicki@gmail.com>
 *
 * Based on:
 *   hiveap-330.c:
 *      Aerohive HiveAP-330 Board Setup
 *      Copyright (C) 2017 Chris Blake <chrisrblake93@gmail.com>
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
#include "smp.h"

#include "mpc85xx.h"

static void __init br200_wp_pic_init(void)
{
	struct mpic *mpic;

	mpic = mpic_alloc(NULL, 0,
	  MPIC_BIG_ENDIAN |
	  MPIC_SINGLE_DEST_CPU,
	  0, 256, " OpenPIC  ");

	BUG_ON(mpic == NULL);
	mpic_init(mpic);
}

/*
 * Setup the architecture
 */
static void __init br200_wp_setup_arch(void)
{
	if (ppc_md.progress)
		ppc_md.progress("br200_wp_setup_arch()", 0);

	fsl_pci_assign_primary();

	pr_info("br200-wp board from Aerohive\n");
}

machine_arch_initcall(br200_wp, mpc85xx_common_publish_devices);

define_machine(br200_wp) {
	.name			= "P1020 RDB",
	.compatible		= "aerohive,br200-wp",
	.setup_arch		= br200_wp_setup_arch,
	.init_IRQ		= br200_wp_pic_init,
#ifdef CONFIG_PCI
	.pcibios_fixup_bus	= fsl_pcibios_fixup_bus,
	.pcibios_fixup_phb      = fsl_pcibios_fixup_phb,
#endif
	.get_irq		= mpic_get_irq,
	.progress		= udbg_progress,
};

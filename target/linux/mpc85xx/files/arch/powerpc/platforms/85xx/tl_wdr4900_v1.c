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
#include <linux/pci.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/of_platform.h>
#include <linux/ath9k_platform.h>
#include <linux/leds.h>

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

void __init tl_wdr4900_v1_pic_init(void)
{
	struct mpic *mpic = mpic_alloc(NULL, 0, MPIC_BIG_ENDIAN |
	  MPIC_SINGLE_DEST_CPU,
	  0, 256, " OpenPIC  ");

	BUG_ON(mpic == NULL);

	mpic_init(mpic);
}

#ifdef CONFIG_PCI
static struct gpio_led tl_wdr4900_v1_wmac_leds_gpio[] = {
	{
		.name		= "tp-link:blue:wps",
		.gpio		= 1,
		.active_low	= 1,
	},
};

static struct ath9k_platform_data tl_wdr4900_v1_wmac0_data = {
	.led_pin = 0,
	.eeprom_name = "pci_wmac0.eeprom",
	.leds = tl_wdr4900_v1_wmac_leds_gpio,
	.num_leds = ARRAY_SIZE(tl_wdr4900_v1_wmac_leds_gpio),
};

static struct ath9k_platform_data tl_wdr4900_v1_wmac1_data = {
	.led_pin = 0,
	.eeprom_name = "pci_wmac1.eeprom",
};

static void tl_wdr4900_v1_pci_wmac_fixup(struct pci_dev *dev)
{
	if (!machine_is(tl_wdr4900_v1))
		return;

	if (dev->bus->number == 1 &&
	    PCI_SLOT(dev->devfn) == 0) {
		dev->dev.platform_data = &tl_wdr4900_v1_wmac0_data;
		return;
	}

	if (dev->bus->number == 3 &&
	    PCI_SLOT(dev->devfn) == 0 &&
	    dev->device == 0xabcd) {
		dev->dev.platform_data = &tl_wdr4900_v1_wmac1_data;

		/*
		 * The PCI header of the AR9381 chip is not programmed
		 * correctly by the bootloader and the device uses wrong
		 * data due to that. Replace the broken values with the
		 * correct ones.
		 */
		dev->device = 0x30;
		dev->class = 0x028000;

		pr_info("pci %s: AR9381 fixup applied\n", pci_name(dev));
	}
}

DECLARE_PCI_FIXUP_EARLY(PCI_VENDOR_ID_ATHEROS, PCI_ANY_ID,
			tl_wdr4900_v1_pci_wmac_fixup);
#endif /* CONFIG_PCI */

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

/*
 * Called very early, device-tree isn't unflattened
 */
static int __init tl_wdr4900_v1_probe(void)
{
	unsigned long root = of_get_flat_dt_root();

	if (of_flat_dt_is_compatible(root, "tplink,tl-wdr4900-v1"))
		return 1;

	return 0;
}

define_machine(tl_wdr4900_v1) {
	.name			= "Freescale P1014",
	.probe			= tl_wdr4900_v1_probe,
	.setup_arch		= tl_wdr4900_v1_setup_arch,
	.init_IRQ		= tl_wdr4900_v1_pic_init,
#ifdef CONFIG_PCI
	.pcibios_fixup_bus	= fsl_pcibios_fixup_bus,
#endif
	.get_irq		= mpic_get_irq,
	.calibrate_decr		= generic_calibrate_decr,
	.progress		= udbg_progress,
};

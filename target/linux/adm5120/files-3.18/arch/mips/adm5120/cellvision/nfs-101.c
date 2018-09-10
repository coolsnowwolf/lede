/*
 *  Cellvision/SparkLAN NFS-101U/WU support
 *
 *  Copyright (C) 2007-2009 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 */

#include "cellvision.h"

static struct adm5120_pci_irq nfs101_pci_irqs[] __initdata = {
	/* miniPCI slot */
	PCIIRQ(2, 0, 1, ADM5120_IRQ_PCI0),

	/* ALi USB controller */
	PCIIRQ(3, 0, 2, ADM5120_IRQ_PCI2),
	PCIIRQ(3, 3, 1, ADM5120_IRQ_PCI1),

	/* NEC USB controller */
	PCIIRQ(3, 0, 1, ADM5120_IRQ_PCI1),
	PCIIRQ(3, 1, 2, ADM5120_IRQ_PCI2),
	PCIIRQ(3, 2, 3, ADM5120_IRQ_PCI2),
};

static u8 nfs101_vlans[6] __initdata = {
	/* FIXME: not tested */
	0x5f, 0x00, 0x00, 0x00, 0x00, 0x00
};

static void __init nfs101_setup(void)
{
	cas6xx_flash_setup();
	cellvision_mac_setup();

	adm5120_add_device_uart(0);
	adm5120_add_device_uart(1);
	adm5120_add_device_switch(1, nfs101_vlans);

	adm5120_pci_set_irq_map(ARRAY_SIZE(nfs101_pci_irqs),
				nfs101_pci_irqs);
}

MIPS_MACHINE(MACH_ADM5120_NFS101U, "NFS-101U", "Cellvision NFS-101U/101WU",
	     nfs101_setup);

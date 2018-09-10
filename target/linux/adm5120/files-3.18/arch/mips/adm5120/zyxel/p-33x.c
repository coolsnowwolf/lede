/*
 *  ZyXEL Prestige P-33x boards support
 *
 *  Copyright (C) 2007-2008 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 */

#include "p-33x.h"

#include <prom/zynos.h>

#define P33X_GPIO_FLASH_A20	ADM5120_GPIO_PIN5
static struct mtd_partition p33x_partitions[] = {
	{
		.name	= "bootbase",
		.offset	= 0,
		.size	= 16*1024,
		.mask_flags = MTD_WRITEABLE,
	} , {
		.name	= "rom",
		.offset	= MTDPART_OFS_APPEND,
		.size	= 16*1024,
		.mask_flags = MTD_WRITEABLE,
	} , {
		.name	= "bootext1",
		.offset	= MTDPART_OFS_APPEND,
		.size	= 32*1024,
	} , {
		.name	= "bootext2",
		.offset	= MTDPART_OFS_APPEND,
		.size	= 64*1024,
	} , {
		.name	= "trx",
		.offset	= MTDPART_OFS_APPEND,
		.size	= MTDPART_SIZ_FULL,
	} , {
		.name	= "firmware",
		.offset	= 32*1024,
		.size	= MTDPART_SIZ_FULL,
	}
};

static struct adm5120_pci_irq p33x_pci_irqs[] __initdata = {
	PCIIRQ(2, 0, 1, ADM5120_IRQ_PCI0),
};

static u8 p33x_vlans[6] __initdata = {
	/* FIXME: untested */
	0x50, 0x48, 0x44, 0x42, 0x41, 0x00
};

static void switch_bank_gpio5(unsigned bank)
{
	switch (bank) {
	case 0:
		gpio_set_value(P33X_GPIO_FLASH_A20, 0);
		break;
	case 1:
		gpio_set_value(P33X_GPIO_FLASH_A20, 1);
		break;
	}
}

void __init p33x_generic_setup(void)
{
	/* setup data for flash0 device */
	gpio_request(P33X_GPIO_FLASH_A20, NULL); /* for flash A20 line */
	gpio_direction_output(P33X_GPIO_FLASH_A20, 0);
	adm5120_flash0_data.switch_bank = switch_bank_gpio5;
	adm5120_flash0_data.nr_parts = ARRAY_SIZE(p33x_partitions);
	adm5120_flash0_data.parts = p33x_partitions;
	adm5120_add_device_flash(0);

	adm5120_add_device_uart(0);
	adm5120_add_device_uart(1);

	adm5120_setup_eth_macs(bootbase_info.mac);
	adm5120_add_device_switch(6, p33x_vlans);

	adm5120_pci_set_irq_map(ARRAY_SIZE(p33x_pci_irqs), p33x_pci_irqs);
}

/*
 *  OSBRiDGE 5GXi/5XLi board support
 *
 *  Copyright (C) 2009 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 */
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/irq.h>
#include <linux/etherdevice.h>

#include <asm/mips_machine.h>

#include <asm/mach-adm5120/adm5120_defs.h>
#include <asm/mach-adm5120/adm5120_platform.h>
#include <asm/mach-adm5120/adm5120_info.h>

static struct mtd_partition osbridge_5gxi_partitions[] = {
	{
		.name	= "bootloader",
		.offset	= 0,
		.size	= 64*1024,
		.mask_flags = MTD_WRITEABLE,
	} , {
		.name	= "boardcfg",
		.offset	= 64*1024,
		.size	= 64*1024,
	} , {
		.name	= "firmware",
		.offset	= MTDPART_OFS_APPEND,
		.size	= MTDPART_SIZ_FULL,
	}
};

static struct gpio_led osbridge_5gxi_gpio_leds[] __initdata = {
	GPIO_LED_INV(ADM5120_GPIO_PIN6, "5gxi:green:user",	NULL),
	GPIO_LED_INV(ADM5120_GPIO_P0L0, "5gxi:yellow:lan",	NULL),
};

static struct adm5120_pci_irq osbridge_5gxi_pci_irqs[] __initdata = {
	PCIIRQ(2, 0, 1, ADM5120_IRQ_PCI0),
};

static u8 osbridge_5gxi_vlans[6] __initdata = {
	0x41, 0x00, 0x00, 0x00, 0x00, 0x00
};

static void __init osbridge_5gxi_setup(void)
{
	adm5120_flash0_data.nr_parts = ARRAY_SIZE(osbridge_5gxi_partitions);
	adm5120_flash0_data.parts = osbridge_5gxi_partitions;

	adm5120_add_device_uart(0);
	adm5120_add_device_uart(1);

	adm5120_add_device_flash(0);

	adm5120_add_device_switch(1, osbridge_5gxi_vlans);
	adm5120_add_device_gpio_leds(ARRAY_SIZE(osbridge_5gxi_gpio_leds),
					osbridge_5gxi_gpio_leds);
	adm5120_pci_set_irq_map(ARRAY_SIZE(osbridge_5gxi_pci_irqs),
				osbridge_5gxi_pci_irqs);
}

MIPS_MACHINE(MACH_ADM5120_5GXI, "5GXi", "OSBRiDGE 5GXi/5XLi board",
	     osbridge_5gxi_setup);

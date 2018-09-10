/*
 *  EB-214A board support
 *
 *  Copyright (C) 2007-2008 Gabor Juhos <juhosg@openwrt.org>
 *  Copyright (C) 2010 Cezary Jackiewicz <cezary@eko.one.pl>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 */

#include <asm/addrspace.h>
#include <asm/byteorder.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/irq.h>
#include <linux/etherdevice.h>

#include <asm/mips_machine.h>

#include <asm/mach-adm5120/adm5120_info.h>
#include <asm/mach-adm5120/adm5120_platform.h>
#include <asm/mach-adm5120/adm5120_defs.h>


#define EB214A_CONFIG_OFFSET	0x4000

#define EB214A_KEYS_POLL_INTERVAL	20
#define EB214A_KEYS_DEBOUNCE_INTERVAL	(3 * EB214A_KEYS_POLL_INTERVAL)

static struct mtd_partition eb214a_partitions[] = {
	{
		.name	= "bootloader",
		.offset	= 0,
		.size	= 32*1024,
		.mask_flags = MTD_WRITEABLE,
	} , {
		.name	= "config",
		.offset	= MTDPART_OFS_APPEND,
		.size	= 32*1024,
	} , {
		.name	= "firmware",
		.offset	= MTDPART_OFS_APPEND,
		.size	= MTDPART_SIZ_FULL,
	}
};

static struct adm5120_pci_irq eb214a_pci_irqs[] __initdata = {
	PCIIRQ(4, 0, 1, ADM5120_IRQ_PCI0),
	PCIIRQ(4, 1, 2, ADM5120_IRQ_PCI0),
	PCIIRQ(4, 2, 3, ADM5120_IRQ_PCI0),
};

static struct gpio_led eb214a_gpio_leds[] __initdata = {
	GPIO_LED_INV(ADM5120_GPIO_PIN7, "power",	NULL),
	GPIO_LED_INV(ADM5120_GPIO_P0L0, "lan",		NULL),
	GPIO_LED_INV(ADM5120_GPIO_P4L0, "usb1",		NULL),
	GPIO_LED_INV(ADM5120_GPIO_P4L1, "usb2",		NULL),
	GPIO_LED_INV(ADM5120_GPIO_P4L2, "usb3",		NULL),
	GPIO_LED_INV(ADM5120_GPIO_P3L0, "usb4",		NULL),
};

static struct gpio_keys_button eb214a_gpio_buttons[] __initdata = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code           = KEY_RESTART,
		.debounce_interval = EB214A_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= ADM5120_GPIO_PIN1,
	}
};

static u8 eb214a_vlans[6] __initdata = {
	0x41, 0x42, 0x44, 0x48, 0x50, 0x00
};

static void __init eb214a_mac_setup(void)
{
	u8 mac_base[6];
	u8 *cfg;
	int i;

	cfg = (u8 *) KSEG1ADDR(ADM5120_SRAM0_BASE + EB214A_CONFIG_OFFSET);
	for (i = 0; i < 6; i++)
		mac_base[i] = cfg[i];

	if (!is_valid_ether_addr(mac_base))
		random_ether_addr(mac_base);

	adm5120_setup_eth_macs(mac_base);
}

static void __init eb214a_setup(void)
{
	adm5120_flash0_data.nr_parts = ARRAY_SIZE(eb214a_partitions);
	adm5120_flash0_data.parts = eb214a_partitions;
	adm5120_add_device_flash(0);

	adm5120_add_device_uart(0);
	/* adm5120_add_device_uart(1); */

	adm5120_add_device_switch(5, eb214a_vlans);

	eb214a_mac_setup();

	adm5120_register_gpio_buttons(-1, EB214A_KEYS_POLL_INTERVAL,
				      ARRAY_SIZE(eb214a_gpio_buttons),
				      eb214a_gpio_buttons);

	adm5120_add_device_gpio_leds(ARRAY_SIZE(eb214a_gpio_leds),
					eb214a_gpio_leds);

	adm5120_pci_set_irq_map(ARRAY_SIZE(eb214a_pci_irqs),
					eb214a_pci_irqs);
	/* adm5120_add_device_usb(); */
}

MIPS_MACHINE(MACH_ADM5120_EB_214A, "EB-214A", "Generic EB-214A", eb214a_setup);

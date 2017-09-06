/*
 *  Compex WP54 board support
 *
 *  Copyright (C) 2007-2008 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 */

#include "compex.h"

#define WP54_KEYS_POLL_INTERVAL		20
#define WP54_KEYS_DEBOUNCE_INTERVAL	(3 * WP54_KEYS_POLL_INTERVAL)

static struct mtd_partition wp54g_wrt_partitions[] = {
	{
		.name	= "cfe",
		.offset	= 0,
		.size	= 0x050000,
		.mask_flags = MTD_WRITEABLE,
	} , {
		.name	= "trx",
		.offset	= MTDPART_OFS_APPEND,
		.size	= 0x3A0000,
	} , {
		.name	= "nvram",
		.offset	= MTDPART_OFS_APPEND,
		.size	= 0x010000,
	}
};

static struct adm5120_pci_irq wp54_pci_irqs[] __initdata = {
	PCIIRQ(2, 0, 1, ADM5120_IRQ_PCI0),
};

static struct gpio_keys_button wp54_gpio_buttons[] __initdata = {
	{
		.desc		= "reset_button",
		.type		= EV_KEY,
		.code           = KEY_RESTART,
		.debounce_interval = WP54_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= ADM5120_GPIO_PIN4,
	}
};

static struct gpio_led wp54_gpio_leds[] __initdata = {
	GPIO_LED_INV(ADM5120_GPIO_PIN2, "diag",	NULL),
	GPIO_LED_INV(ADM5120_GPIO_PIN6, "wlan",	NULL),
	GPIO_LED_INV(ADM5120_GPIO_PIN7, "wan",	NULL),
	GPIO_LED_INV(ADM5120_GPIO_P0L0, "lan1",	NULL),
	GPIO_LED_INV(ADM5120_GPIO_P1L0, "lan2",	NULL),
};

static u8 wp54_vlans[6] __initdata = {
	0x41, 0x42, 0x00, 0x00, 0x00, 0x00
};

static void wp54_reset(void)
{
	gpio_set_value(ADM5120_GPIO_PIN3, 0);
}

static void __init wp54_setup(void)
{
	compex_generic_setup();

	/* setup reset line */
	gpio_request(ADM5120_GPIO_PIN3, NULL);
	gpio_direction_output(ADM5120_GPIO_PIN3, 1);
	adm5120_board_reset = wp54_reset;

	adm5120_add_device_switch(2, wp54_vlans);
	adm5120_register_gpio_buttons(-1, WP54_KEYS_POLL_INTERVAL,
				      ARRAY_SIZE(wp54_gpio_buttons),
				      wp54_gpio_buttons);
	adm5120_add_device_gpio_leds(ARRAY_SIZE(wp54_gpio_leds),
					wp54_gpio_leds);

	adm5120_pci_set_irq_map(ARRAY_SIZE(wp54_pci_irqs), wp54_pci_irqs);
}

MIPS_MACHINE(MACH_ADM5120_WP54, "WP54", "Compex WP54 family", wp54_setup);

static void __init wp54_wrt_setup(void)
{
	adm5120_flash0_data.nr_parts = ARRAY_SIZE(wp54g_wrt_partitions);
	adm5120_flash0_data.parts = wp54g_wrt_partitions;

	wp54_setup();
}

MIPS_MACHINE(MACH_ADM5120_WP54G_WRT, "WP54G-WRT", "Compex WP54G-WRT",
	     wp54_wrt_setup);

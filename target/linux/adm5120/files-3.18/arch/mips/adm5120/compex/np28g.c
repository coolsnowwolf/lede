/*
 *  Compex NP28G board support
 *
 *  Copyright (C) 2007-2008 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 */

#include "compex.h"

static struct adm5120_pci_irq np28g_pci_irqs[] __initdata = {
	PCIIRQ(2, 0, 1, ADM5120_IRQ_PCI0),
	PCIIRQ(3, 0, 1, ADM5120_IRQ_PCI0),
	PCIIRQ(3, 1, 2, ADM5120_IRQ_PCI1),
	PCIIRQ(3, 2, 3, ADM5120_IRQ_PCI2)
};

static struct gpio_led np28g_gpio_leds[] __initdata = {
	GPIO_LED_INV(ADM5120_GPIO_PIN2, "diag",		NULL),
	GPIO_LED_INV(ADM5120_GPIO_PIN3, "power",	NULL),
	GPIO_LED_INV(ADM5120_GPIO_PIN6, "wan_cond",	NULL),
	GPIO_LED_INV(ADM5120_GPIO_PIN7, "wifi",		NULL),
	GPIO_LED_INV(ADM5120_GPIO_P0L2, "usb1",		NULL),
	GPIO_LED_INV(ADM5120_GPIO_P1L0, "lan1",		NULL),
	GPIO_LED_INV(ADM5120_GPIO_P1L2, "usb2",		NULL),
	GPIO_LED_INV(ADM5120_GPIO_P2L0, "lan2",		NULL),
	GPIO_LED_INV(ADM5120_GPIO_P2L2, "usb3",		NULL),
	GPIO_LED_INV(ADM5120_GPIO_P3L0, "lan3",		NULL),
	GPIO_LED_INV(ADM5120_GPIO_P3L2, "usb4",		NULL),
	GPIO_LED_INV(ADM5120_GPIO_P4L0, "wan",		NULL),
};

static u8 np28g_vlans[6] __initdata = {
	0x50, 0x42, 0x44, 0x48, 0x00, 0x00
};

static void np28g_reset(void)
{
	gpio_set_value(ADM5120_GPIO_PIN4, 0);
}

static void __init np28g_setup(void)
{
	compex_generic_setup();

	/* setup reset line */
	gpio_request(ADM5120_GPIO_PIN4, NULL);
	gpio_direction_output(ADM5120_GPIO_PIN4, 1);
	adm5120_board_reset = np28g_reset;

	adm5120_add_device_switch(4, np28g_vlans);
	adm5120_add_device_usb();

	adm5120_add_device_gpio_leds(ARRAY_SIZE(np28g_gpio_leds),
					np28g_gpio_leds);

	adm5120_pci_set_irq_map(ARRAY_SIZE(np28g_pci_irqs), np28g_pci_irqs);
}

MIPS_MACHINE(MACH_ADM5120_NP28G, "NP28G", "Compex NetPassage 28G", np28g_setup);

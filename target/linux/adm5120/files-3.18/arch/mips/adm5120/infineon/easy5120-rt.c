/*
 *  Infineon EASY 5120-RT Reference Board support
 *
 *  Copyright (C) 2007-2008 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 */

#include "infineon.h"

static struct gpio_led easy5120_rt_gpio_leds[] __initdata = {
	GPIO_LED_INV(ADM5120_GPIO_PIN6, "user",		NULL),
	GPIO_LED_INV(ADM5120_GPIO_P0L0, "lan0_led1",	NULL),
	GPIO_LED_INV(ADM5120_GPIO_P0L1, "lan0_led2",	NULL),
	GPIO_LED_INV(ADM5120_GPIO_P1L0, "lan1_led1",	NULL),
	GPIO_LED_INV(ADM5120_GPIO_P1L1, "lan1_led2",	NULL),
	GPIO_LED_INV(ADM5120_GPIO_P2L0, "lan2_led1",	NULL),
	GPIO_LED_INV(ADM5120_GPIO_P2L1, "lan2_led2",	NULL),
	GPIO_LED_INV(ADM5120_GPIO_P3L0, "lan3_led1",	NULL),
	GPIO_LED_INV(ADM5120_GPIO_P3L1, "lan3_led2",	NULL),
	GPIO_LED_INV(ADM5120_GPIO_P4L0, "wan",		NULL),
};

static struct adm5120_pci_irq easy5120_rt_pci_irqs[] __initdata = {
	PCIIRQ(2, 0, 1, ADM5120_IRQ_PCI0),
};

static u8 easy5120_rt_vlans[6] __initdata = {
	0x41, 0x42, 0x44, 0x48, 0x50, 0x00
};

static void __init easy5120_rt_setup(void)
{
	easy_setup_bga();

	adm5120_add_device_switch(5, easy5120_rt_vlans);
	adm5120_add_device_usb();
	adm5120_add_device_gpio_leds(ARRAY_SIZE(easy5120_rt_gpio_leds),
					easy5120_rt_gpio_leds);
	adm5120_pci_set_irq_map(ARRAY_SIZE(easy5120_rt_pci_irqs),
				easy5120_rt_pci_irqs);
}

MIPS_MACHINE(MACH_ADM5120_EASY5120RT, "EASY5120-RT",
	     "Infineon EASY 5120-RT Reference Board", easy5120_rt_setup);

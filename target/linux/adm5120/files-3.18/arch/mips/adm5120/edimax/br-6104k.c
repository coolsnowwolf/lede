/*
 *  Edimax BR-6104K board support
 *
 *  Copyright (C) 2007-2008 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 */

#include "br-61xx.h"

static struct gpio_led br6104k_gpio_leds[] __initdata = {
	GPIO_LED_STD(ADM5120_GPIO_PIN0, "power",	NULL),
	GPIO_LED_INV(ADM5120_GPIO_P0L1, "wan_speed",	NULL),
	GPIO_LED_INV(ADM5120_GPIO_P0L0, "wan_lnkact",	NULL),
	GPIO_LED_INV(ADM5120_GPIO_P1L1, "lan1_speed",	NULL),
	GPIO_LED_INV(ADM5120_GPIO_P1L0, "lan1_lnkact",	NULL),
	GPIO_LED_INV(ADM5120_GPIO_P2L1, "lan2_speed",	NULL),
	GPIO_LED_INV(ADM5120_GPIO_P2L0, "lan2_lnkact",	NULL),
	GPIO_LED_INV(ADM5120_GPIO_P3L1, "lan3_speed",	NULL),
	GPIO_LED_INV(ADM5120_GPIO_P3L0, "lan3_lnkact",	NULL),
	GPIO_LED_INV(ADM5120_GPIO_P4L1, "lan4_speed",	NULL),
	GPIO_LED_INV(ADM5120_GPIO_P4L0, "lan4_lnkact",	NULL),
};

static void __init br6104k_setup(void)
{
	br61xx_generic_setup();
	adm5120_add_device_gpio_leds(ARRAY_SIZE(br6104k_gpio_leds),
					br6104k_gpio_leds);
}

MIPS_MACHINE(MACH_ADM5120_BR6104K, "BR-6104K", "Edimax BR-6104K",
	     br6104k_setup);

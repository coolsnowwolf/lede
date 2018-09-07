/*
 *  Mikrotik RouterBOARD 111/112 support
 *
 *  Copyright (C) 2007-2008 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 */

#include "rb-1xx.h"

static struct gpio_led rb11x_gpio_leds[] __initdata = {
	GPIO_LED_STD(ADM5120_GPIO_PIN3, "user",		NULL),
	GPIO_LED_INV(ADM5120_GPIO_P0L1,	"lan_speed",	NULL),
	GPIO_LED_INV(ADM5120_GPIO_P0L0, "lan_lnkact",	NULL),
};

static u8 rb11x_vlans[6] __initdata = {
	0x7F, 0x00, 0x00, 0x00, 0x00, 0x00
};

static void __init rb11x_setup(void)
{
	rb1xx_generic_setup();
	rb1xx_add_device_nand();

	adm5120_add_device_switch(1, rb11x_vlans);
	adm5120_add_device_gpio_leds(ARRAY_SIZE(rb11x_gpio_leds),
					rb11x_gpio_leds);
}

MIPS_MACHINE(MACH_ADM5120_RB_11X, "11x", "Mikrotik RouterBOARD 111/112",
	     rb11x_setup);

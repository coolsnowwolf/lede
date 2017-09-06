/*
 *  Mikrotik RouterBOARD 133 support
 *
 *  Copyright (C) 2007-2008 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 */

#include "rb-1xx.h"

static struct gpio_led rb133_gpio_leds[] __initdata = {
	GPIO_LED_STD(ADM5120_GPIO_PIN6, "power",	NULL),
	GPIO_LED_STD(ADM5120_GPIO_PIN5, "user",		NULL),
	GPIO_LED_INV(ADM5120_GPIO_P2L1,	"lan1_speed",	NULL),
	GPIO_LED_INV(ADM5120_GPIO_P2L0,	"lan1_lnkact",	NULL),
	GPIO_LED_INV(ADM5120_GPIO_P1L1,	"lan2_speed",	NULL),
	GPIO_LED_INV(ADM5120_GPIO_P1L0,	"lan2_lnkact",	NULL),
	GPIO_LED_INV(ADM5120_GPIO_P0L1,	"lan3_speed",	NULL),
	GPIO_LED_INV(ADM5120_GPIO_P0L0,	"lan3_lnkact",	NULL),
};

static u8 rb133_vlans[6] __initdata = {
	0x7F, 0x00, 0x00, 0x00, 0x00, 0x00
};

static void __init rb133_setup(void)
{
	rb1xx_generic_setup();
	rb1xx_add_device_nand();

	adm5120_add_device_switch(3, rb133_vlans);
	adm5120_add_device_gpio_leds(ARRAY_SIZE(rb133_gpio_leds),
					rb133_gpio_leds);
}

MIPS_MACHINE(MACH_ADM5120_RB_133, "133", "Mikrotik RouterBOARD 133",
	     rb133_setup);

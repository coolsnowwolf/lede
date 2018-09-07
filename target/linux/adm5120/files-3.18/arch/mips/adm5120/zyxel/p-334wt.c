/*
 *  ZyXEL Prestige P-334WT support
 *
 *  Copyright (C) 2007-2008 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 */

#include "p-33x.h"

static struct gpio_led p334wt_gpio_leds[] __initdata = {
	GPIO_LED_INV(ADM5120_GPIO_PIN2, "power",	NULL),
	GPIO_LED_INV(ADM5120_GPIO_P3L0, "lan1",		NULL),
	GPIO_LED_INV(ADM5120_GPIO_P2L0, "lan2",		NULL),
	GPIO_LED_INV(ADM5120_GPIO_P1L0, "lan3",		NULL),
	GPIO_LED_INV(ADM5120_GPIO_P0L0, "lan4",		NULL),
	GPIO_LED_INV(ADM5120_GPIO_P4L0, "wan",		NULL),
	GPIO_LED_INV(ADM5120_GPIO_P4L2, "wlan",		NULL),
	GPIO_LED_INV(ADM5120_GPIO_P2L2, "otist",	NULL),
	GPIO_LED_INV(ADM5120_GPIO_P1L2, "hidden",	NULL),
};

static void __init p334wt_setup(void)
{
	p33x_generic_setup();
	adm5120_add_device_gpio_leds(ARRAY_SIZE(p334wt_gpio_leds),
					p334wt_gpio_leds);
}

MIPS_MACHINE(MACH_ADM5120_P334WT, "P-334WT", "ZyXEL Prestige 334WT",
	     p334wt_setup);

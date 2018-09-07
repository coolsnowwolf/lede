/*
 *  Mikrotik RouterBOARD 153 support
 *
 *  Copyright (C) 2007-2008 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 */

#include "rb-1xx.h"

static struct resource rb153_cf_resources[] __initdata = {
	{
		.name	= "cf_membase",
		.start	= ADM5120_EXTIO1_BASE,
		.end	= ADM5120_EXTIO1_BASE + ADM5120_EXTIO1_SIZE-1 ,
		.flags	= IORESOURCE_MEM
	}, {
		.name	= "cf_irq",
		.start	= ADM5120_IRQ_GPIO4,
		.end	= ADM5120_IRQ_GPIO4,
		.flags	= IORESOURCE_IRQ
	}
};

static struct gpio_led rb153_gpio_leds[] __initdata = {
	GPIO_LED_STD(ADM5120_GPIO_PIN5, "user",		NULL),
	GPIO_LED_INV(ADM5120_GPIO_P0L1, "lan1_speed",	NULL),
	GPIO_LED_INV(ADM5120_GPIO_P0L0, "lan1_lnkact",	NULL),
	GPIO_LED_INV(ADM5120_GPIO_P1L1, "lan5_speed",	NULL),
	GPIO_LED_INV(ADM5120_GPIO_P1L0, "lan5_lnkact",	NULL),
	GPIO_LED_INV(ADM5120_GPIO_P2L1, "lan4_speed",	NULL),
	GPIO_LED_INV(ADM5120_GPIO_P2L0, "lan4_lnkact",	NULL),
	GPIO_LED_INV(ADM5120_GPIO_P3L1, "lan3_speed",	NULL),
	GPIO_LED_INV(ADM5120_GPIO_P3L0, "lan3_lnkact",	NULL),
	GPIO_LED_INV(ADM5120_GPIO_P4L1, "lan2_speed",	NULL),
	GPIO_LED_INV(ADM5120_GPIO_P4L0, "lan2_lnkact",	NULL),
};

static u8 rb153_vlans[6] __initdata = {
	0x7F, 0x00, 0x00, 0x00, 0x00, 0x00
};

static void __init rb153_add_device_cf(void)
{
	/* enable CSX1:INTX1 on GPIO[3:4] for the CF slot */
	adm5120_gpio_csx1_enable();

	/* enable the wait state pin GPIO[0] for external I/O control */
	adm5120_gpio_ew_enable();

	platform_device_register_simple("pata-rb153-cf", -1,
			rb153_cf_resources, ARRAY_SIZE(rb153_cf_resources));
}

static void __init rb153_setup(void)
{
	rb1xx_generic_setup();
	rb1xx_add_device_nand();
	rb153_add_device_cf();

	adm5120_add_device_gpio_leds(ARRAY_SIZE(rb153_gpio_leds),
					rb153_gpio_leds);
	adm5120_add_device_switch(5, rb153_vlans);
}

MIPS_MACHINE(MACH_ADM5120_RB_153, "150", "Mikrotik RouterBOARD 153",
	     rb153_setup);

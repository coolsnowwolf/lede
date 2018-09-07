/*
 *  Edimax BR-61xx support
 *
 *  Copyright (C) 2007-2008 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 */

#include "br-61xx.h"

#include <prom/admboot.h>

#define BR61XX_CONFIG_OFFSET	0x8000
#define BR61XX_CONFIG_SIZE		0x1000

#define BR61XX_KEYS_POLL_INTERVAL	20
#define BR61XX_KEYS_DEBOUNCE_INTERVAL	(3 * BR61XX_KEYS_POLL_INTERVAL)

static struct mtd_partition br61xx_partitions[] = {
	{
		.name	= "admboot",
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

static struct gpio_keys_button br61xx_gpio_buttons[] __initdata = {
	{
		.desc		= "reset_button",
		.type		= EV_KEY,
		.code           = KEY_RESTART,
		.debounce_interval = BR61XX_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= ADM5120_GPIO_PIN2,
	}
};

static u8 br61xx_vlans[6] __initdata = {
	0x41, 0x42, 0x44, 0x48, 0x50, 0x00
};

static void __init br61xx_mac_setup(void)
{
	u8 mac_base[6];
	int err;

	err = admboot_get_mac_base(BR61XX_CONFIG_OFFSET,
				   BR61XX_CONFIG_SIZE, mac_base);

	if ((err) || !is_valid_ether_addr(mac_base))
		random_ether_addr(mac_base);

	adm5120_setup_eth_macs(mac_base);
}

void __init br61xx_generic_setup(void)
{

	adm5120_flash0_data.nr_parts = ARRAY_SIZE(br61xx_partitions);
	adm5120_flash0_data.parts = br61xx_partitions;
	adm5120_add_device_flash(0);

	adm5120_add_device_uart(0);
	adm5120_add_device_uart(1);

	adm5120_add_device_switch(5, br61xx_vlans);

	adm5120_register_gpio_buttons(-1, BR61XX_KEYS_POLL_INTERVAL,
				      ARRAY_SIZE(br61xx_gpio_buttons),
				      br61xx_gpio_buttons);

	br61xx_mac_setup();
}

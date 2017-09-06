/*
 *  Infineon Reference Boards
 *
 *  Copyright (C) 2007-2008 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 */

#include "infineon.h"

#include <prom/admboot.h>

#define EASY_CONFIG_OFFSET	0x10000
#define EASY_CONFIG_SIZE	0x1000

static struct mtd_partition easy_partitions[] = {
	{
		.name	= "admboot",
		.offset	= 0,
		.size	= 64*1024,
		.mask_flags = MTD_WRITEABLE,
	} , {
		.name	= "boardcfg",
		.offset	= MTDPART_OFS_APPEND,
		.size	= 64*1024,
	} , {
		.name	= "firmware",
		.offset	= MTDPART_OFS_APPEND,
		.size	= MTDPART_SIZ_FULL,
	}
};

static __init void easy_setup_mac(void)
{
	u8 mac_base[6];
	int err;

	err = admboot_get_mac_base(EASY_CONFIG_OFFSET,
				   EASY_CONFIG_SIZE, mac_base);

	if ((err) || !is_valid_ether_addr(mac_base))
		random_ether_addr(mac_base);

	adm5120_setup_eth_macs(mac_base);
}

static void switch_bank_gpio3(unsigned bank)
{
	switch (bank) {
	case 0:
		gpio_set_value(ADM5120_GPIO_PIN3, 0);
		break;
	case 1:
		gpio_set_value(ADM5120_GPIO_PIN3, 1);
		break;
	}
}

void __init easy_setup_pqfp(void)
{
	/* setup flash A20 line */
	gpio_request(ADM5120_GPIO_PIN3, NULL);
	gpio_direction_output(ADM5120_GPIO_PIN3, 0);
	adm5120_flash0_data.switch_bank = switch_bank_gpio3;

	adm5120_flash0_data.nr_parts = ARRAY_SIZE(easy_partitions);
	adm5120_flash0_data.parts = easy_partitions;

	adm5120_add_device_uart(0);
	adm5120_add_device_uart(1);

	adm5120_add_device_flash(0);

	easy_setup_mac();
}

static void switch_bank_gpio5(unsigned bank)
{
	switch (bank) {
	case 0:
		gpio_set_value(ADM5120_GPIO_PIN5, 0);
		break;
	case 1:
		gpio_set_value(ADM5120_GPIO_PIN5, 1);
		break;
	}
}

void __init easy_setup_bga(void)
{
	/* setup flash A20 line */
	gpio_request(ADM5120_GPIO_PIN5, NULL);
	gpio_direction_output(ADM5120_GPIO_PIN5, 0);
	adm5120_flash0_data.switch_bank = switch_bank_gpio5;

	adm5120_flash0_data.nr_parts = ARRAY_SIZE(easy_partitions);
	adm5120_flash0_data.parts = easy_partitions;

	adm5120_add_device_uart(0);
	adm5120_add_device_uart(1);

	adm5120_add_device_flash(0);

	easy_setup_mac();
}

/*
 *  Compex boards
 *
 *  Copyright (C) 2007-2008 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 */

#include "compex.h"

#include <asm/mach-adm5120/prom/myloader.h>

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

void __init compex_mac_setup(void)
{
	if (myloader_present()) {
		int i;

		for (i = 0; i < 6; i++) {
			if (is_valid_ether_addr(myloader_info.macs[i]))
				memcpy(adm5120_eth_macs[i],
					myloader_info.macs[i], ETH_ALEN);
			else
				random_ether_addr(adm5120_eth_macs[i]);
		}
	} else {
		u8 mac[ETH_ALEN];

		random_ether_addr(mac);
		adm5120_setup_eth_macs(mac);
	}
}

void __init compex_generic_setup(void)
{
	gpio_request(ADM5120_GPIO_PIN5, NULL); /* for flash A20 line */
	gpio_direction_output(ADM5120_GPIO_PIN5, 0);

	adm5120_flash0_data.switch_bank = switch_bank_gpio5;
	adm5120_add_device_flash(0);

	adm5120_add_device_uart(0);
	adm5120_add_device_uart(1);

	compex_mac_setup();
}

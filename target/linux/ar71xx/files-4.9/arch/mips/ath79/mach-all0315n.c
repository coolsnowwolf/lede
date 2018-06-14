/*
 *  Allnet ALL0315N support
 *
 *  Copyright (C) 2012 Daniel Golle <dgolle@allnet.de>
 *
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <asm/mach-ath79/ath79.h>
#include <asm/mach-ath79/ar71xx_regs.h>

#include "common.h"
#include "dev-eth.h"
#include "dev-ap9x-pci.h"
#include "dev-gpio-buttons.h"
#include "dev-m25p80.h"
#include "dev-leds-gpio.h"
#include "machtypes.h"
#include "pci.h"

#define ALL0315N_GPIO_BTN_RESET		0
#define ALL0315N_GPIO_LED_RSSIHIGH	14
#define ALL0315N_GPIO_LED_RSSIMEDIUM	15
#define ALL0315N_GPIO_LED_RSSILOW	16

#define ALL0315N_KEYS_POLL_INTERVAL	20	/* msecs */
#define ALL0315N_KEYS_DEBOUNCE_INTERVAL	(3 * ALL0315N_KEYS_POLL_INTERVAL)

static struct gpio_led all0315n_leds_gpio[] __initdata = {
	{
		.name		= "all0315n:green:rssihigh",
		.gpio		= ALL0315N_GPIO_LED_RSSIHIGH,
		.active_low	= 1,
	}, {
		.name		= "all0315n:yellow:rssimedium",
		.gpio		= ALL0315N_GPIO_LED_RSSIMEDIUM,
		.active_low	= 1,
	}, {
		.name		= "all0315n:red:rssilow",
		.gpio		= ALL0315N_GPIO_LED_RSSILOW,
		.active_low	= 1,
	}
};

static struct gpio_keys_button all0315n_gpio_keys[] __initdata = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = ALL0315N_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= ALL0315N_GPIO_BTN_RESET,
		.active_low	= 1,
	}
};

static void __init all0315n_setup(void)
{
	u8 *mac = (u8 *) KSEG1ADDR(0x1ffc0000);
	u8 *ee =  (u8 *) KSEG1ADDR(0x1ffc1000);

	ath79_register_m25p80(NULL);

	ath79_init_mac(ath79_eth0_data.mac_addr, mac, 0);
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth0_data.phy_mask = BIT(0);

	ath79_register_mdio(0, 0x0);
	ath79_register_eth(0);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(all0315n_leds_gpio),
					all0315n_leds_gpio);

	ath79_register_gpio_keys_polled(-1, ALL0315N_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(all0315n_gpio_keys),
					all0315n_gpio_keys);

	ap9x_pci_setup_wmac_led_pin(0, 1);
	ap91_pci_init(ee, NULL);
}

MIPS_MACHINE(ATH79_MACH_ALL0315N, "ALL0315N", "Allnet ALL0315N",
	     all0315n_setup);

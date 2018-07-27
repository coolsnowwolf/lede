/*
 *  Allnet ALL0258N support
 *
 *  Copyright (C) 2011 Daniel Golle <dgolle@allnet.de>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <asm/mach-ath79/ath79.h>

#include "dev-eth.h"
#include "dev-ap9x-pci.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "machtypes.h"

/* found via /sys/gpio/... try and error */
#define ALL0258N_GPIO_BTN_RESET		1
#define ALL0258N_GPIO_LED_RSSIHIGH	13
#define ALL0258N_GPIO_LED_RSSIMEDIUM	15
#define ALL0258N_GPIO_LED_RSSILOW	14

/* defaults taken from others machs */
#define ALL0258N_KEYS_POLL_INTERVAL	20	/* msecs */
#define ALL0258N_KEYS_DEBOUNCE_INTERVAL (3 * ALL0258N_KEYS_POLL_INTERVAL)

/* showed up in the original firmware's bootlog */
#define ALL0258N_SEC_PHYMASK BIT(3)

static struct gpio_led all0258n_leds_gpio[] __initdata = {
	{
		.name		= "all0258n:green:rssihigh",
		.gpio		= ALL0258N_GPIO_LED_RSSIHIGH,
		.active_low	= 1,
	}, {
		.name		= "all0258n:yellow:rssimedium",
		.gpio		= ALL0258N_GPIO_LED_RSSIMEDIUM,
		.active_low	= 1,
	}, {
		.name		= "all0258n:red:rssilow",
		.gpio		= ALL0258N_GPIO_LED_RSSILOW,
		.active_low	= 1,
	}
};

static struct gpio_keys_button all0258n_gpio_keys[] __initdata = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = ALL0258N_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= ALL0258N_GPIO_BTN_RESET,
		.active_low	= 1,
	}
};

static void __init all0258n_setup(void)
{
	u8 *mac = (u8 *) KSEG1ADDR(0x1f7f0000);
	u8 *ee =  (u8 *) KSEG1ADDR(0x1f7f1000);

	ath79_register_m25p80(NULL);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(all0258n_leds_gpio),
				 all0258n_leds_gpio);

	ath79_register_gpio_keys_polled(-1, ALL0258N_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(all0258n_gpio_keys),
					all0258n_gpio_keys);

	ath79_init_mac(ath79_eth0_data.mac_addr, mac, 0);
	ath79_init_mac(ath79_eth1_data.mac_addr, mac, 0);

	ath79_eth1_data.phy_mask = ALL0258N_SEC_PHYMASK;

	ath79_register_mdio(0, 0x0);

	ath79_register_eth(0);
	ath79_register_eth(1);

	ap91_pci_init(ee, mac);
}

MIPS_MACHINE(ATH79_MACH_ALL0258N, "ALL0258N", "Allnet ALL0258N",
	     all0258n_setup);

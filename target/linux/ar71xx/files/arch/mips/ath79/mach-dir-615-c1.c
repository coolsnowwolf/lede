/*
 *  D-Link DIR-615 rev C1 board support
 *
 *  Copyright (C) 2008-2012 Gabor Juhos <juhosg@openwrt.org>
 *  Copyright (C) 2008 Imre Kaloz <kaloz@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <asm/mach-ath79/ath79.h>

#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-wmac.h"
#include "machtypes.h"
#include "nvram.h"

#define DIR_615C1_GPIO_LED_ORANGE_STATUS 1	/* ORANGE:STATUS:TRICOLOR */
#define DIR_615C1_GPIO_LED_BLUE_WPS	3	/* BLUE:WPS */
#define DIR_615C1_GPIO_LED_GREEN_WAN	4       /* GREEN:WAN:TRICOLOR */
#define DIR_615C1_GPIO_LED_GREEN_WANCPU	5       /* GREEN:WAN:CPU:TRICOLOR */
#define DIR_615C1_GPIO_LED_GREEN_WLAN	6	/* GREEN:WLAN */
#define DIR_615C1_GPIO_LED_GREEN_STATUS	14	/* GREEN:STATUS:TRICOLOR */
#define DIR_615C1_GPIO_LED_ORANGE_WAN	15	/* ORANGE:WAN:TRICOLOR */

/* buttons may need refinement */

#define DIR_615C1_GPIO_BTN_WPS		12
#define DIR_615C1_GPIO_BTN_RESET	21

#define DIR_615C1_KEYS_POLL_INTERVAL	20	/* msecs */
#define DIR_615C1_KEYS_DEBOUNCE_INTERVAL (3 * DIR_615C1_KEYS_POLL_INTERVAL)

#define DIR_615C1_CONFIG_ADDR		0x1f020000
#define DIR_615C1_CONFIG_SIZE		0x10000

#define DIR_615C1_WLAN_MAC_ADDR		0x1f3fffb4

static struct gpio_led dir_615c1_leds_gpio[] __initdata = {
	{
		.name		= "d-link:orange:status",
		.gpio		= DIR_615C1_GPIO_LED_ORANGE_STATUS,
		.active_low	= 1,
	}, {
		.name		= "d-link:blue:wps",
		.gpio		= DIR_615C1_GPIO_LED_BLUE_WPS,
		.active_low	= 1,
	}, {
		.name		= "d-link:green:wan",
		.gpio		= DIR_615C1_GPIO_LED_GREEN_WAN,
		.active_low	= 1,
	}, {
		.name		= "d-link:green:wancpu",
		.gpio		= DIR_615C1_GPIO_LED_GREEN_WANCPU,
		.active_low	= 1,
	}, {
		.name		= "d-link:green:wlan",
		.gpio		= DIR_615C1_GPIO_LED_GREEN_WLAN,
		.active_low	= 1,
	}, {
		.name		= "d-link:green:status",
		.gpio		= DIR_615C1_GPIO_LED_GREEN_STATUS,
		.active_low     = 1,
	}, {
		.name		= "d-link:orange:wan",
		.gpio		= DIR_615C1_GPIO_LED_ORANGE_WAN,
		.active_low	= 1,
	}

};

static struct gpio_keys_button dir_615c1_gpio_keys[] __initdata = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = DIR_615C1_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= DIR_615C1_GPIO_BTN_RESET,
	}, {
		.desc		= "wps",
		.type		= EV_KEY,
		.code		= KEY_WPS_BUTTON,
		.debounce_interval = DIR_615C1_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= DIR_615C1_GPIO_BTN_WPS,
	}
};

#define DIR_615C1_LAN_PHYMASK	BIT(0)
#define DIR_615C1_WAN_PHYMASK	BIT(4)
#define DIR_615C1_MDIO_MASK	(~(DIR_615C1_LAN_PHYMASK | \
				   DIR_615C1_WAN_PHYMASK))

static void __init dir_615c1_setup(void)
{
	const char *config = (char *) KSEG1ADDR(DIR_615C1_CONFIG_ADDR);
	u8 *eeprom = (u8 *) KSEG1ADDR(0x1fff1000);
	u8 mac[ETH_ALEN], wlan_mac[ETH_ALEN];

	if (ath79_nvram_parse_mac_addr(config, DIR_615C1_CONFIG_SIZE,
				       "lan_mac=", mac) == 0) {
		ath79_init_mac(ath79_eth0_data.mac_addr, mac, 0);
		ath79_init_mac(ath79_eth1_data.mac_addr, mac, 1);
	}

	ath79_parse_ascii_mac((char *) KSEG1ADDR(DIR_615C1_WLAN_MAC_ADDR), wlan_mac);

	ath79_register_mdio(0, DIR_615C1_MDIO_MASK);

	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RMII;
	ath79_eth0_data.phy_mask = DIR_615C1_LAN_PHYMASK;

	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_RMII;
	ath79_eth1_data.phy_mask = DIR_615C1_WAN_PHYMASK;

	ath79_register_eth(0);
	ath79_register_eth(1);

	ath79_register_m25p80(NULL);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(dir_615c1_leds_gpio),
				 dir_615c1_leds_gpio);

	ath79_register_gpio_keys_polled(-1, DIR_615C1_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(dir_615c1_gpio_keys),
					dir_615c1_gpio_keys);

	ath79_register_wmac(eeprom, wlan_mac);
}

MIPS_MACHINE(ATH79_MACH_DIR_615_C1, "DIR-615-C1", "D-Link DIR-615 rev. C1",
	     dir_615c1_setup);

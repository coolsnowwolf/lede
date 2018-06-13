/*
 *  TrendNET TEW-632BRP board support
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

#define TEW_632BRP_GPIO_LED_STATUS	1
#define TEW_632BRP_GPIO_LED_WPS		3
#define TEW_632BRP_GPIO_LED_WLAN	6
#define TEW_632BRP_GPIO_BTN_WPS		12
#define TEW_632BRP_GPIO_BTN_RESET	21

#define TEW_632BRP_KEYS_POLL_INTERVAL	20	/* msecs */
#define TEW_632BRP_KEYS_DEBOUNCE_INTERVAL (3 * TEW_632BRP_KEYS_POLL_INTERVAL)

#define TEW_632BRP_CONFIG_ADDR	0x1f020000
#define TEW_632BRP_CONFIG_SIZE	0x10000

static struct gpio_led tew_632brp_leds_gpio[] __initdata = {
	{
		.name		= "tew-632brp:green:status",
		.gpio		= TEW_632BRP_GPIO_LED_STATUS,
		.active_low	= 1,
	}, {
		.name		= "tew-632brp:blue:wps",
		.gpio		= TEW_632BRP_GPIO_LED_WPS,
		.active_low	= 1,
	}, {
		.name		= "tew-632brp:green:wlan",
		.gpio		= TEW_632BRP_GPIO_LED_WLAN,
		.active_low	= 1,
	}
};

static struct gpio_keys_button tew_632brp_gpio_keys[] __initdata = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = TEW_632BRP_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= TEW_632BRP_GPIO_BTN_RESET,
		.active_low	= 1,
	}, {
		.desc		= "wps",
		.type		= EV_KEY,
		.code		= KEY_WPS_BUTTON,
		.debounce_interval = TEW_632BRP_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= TEW_632BRP_GPIO_BTN_WPS,
		.active_low	= 1,
	}
};

#define TEW_632BRP_LAN_PHYMASK	BIT(0)
#define TEW_632BRP_WAN_PHYMASK	BIT(4)
#define TEW_632BRP_MDIO_MASK	(~(TEW_632BRP_LAN_PHYMASK | \
				   TEW_632BRP_WAN_PHYMASK))

static void __init tew_632brp_setup(void)
{
	const char *config = (char *) KSEG1ADDR(TEW_632BRP_CONFIG_ADDR);
	u8 *eeprom = (u8 *) KSEG1ADDR(0x1fff1000);
	u8 mac[6];
	u8 *wlan_mac = NULL;

	if (ath79_nvram_parse_mac_addr(config, TEW_632BRP_CONFIG_SIZE,
				       "lan_mac=", mac) == 0) {
		ath79_init_mac(ath79_eth0_data.mac_addr, mac, 0);
		ath79_init_mac(ath79_eth1_data.mac_addr, mac, 1);
		wlan_mac = mac;
	}

	ath79_register_mdio(0, TEW_632BRP_MDIO_MASK);

	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RMII;
	ath79_eth0_data.phy_mask = TEW_632BRP_LAN_PHYMASK;

	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_RMII;
	ath79_eth1_data.phy_mask = TEW_632BRP_WAN_PHYMASK;

	ath79_register_eth(0);
	ath79_register_eth(1);

	ath79_register_m25p80(NULL);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(tew_632brp_leds_gpio),
				 tew_632brp_leds_gpio);

	ath79_register_gpio_keys_polled(-1, TEW_632BRP_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(tew_632brp_gpio_keys),
					tew_632brp_gpio_keys);

	ath79_register_wmac(eeprom, wlan_mac);
}

MIPS_MACHINE(ATH79_MACH_TEW_632BRP, "TEW-632BRP", "TRENDnet TEW-632BRP",
	     tew_632brp_setup);

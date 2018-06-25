/*
 *  Linksys WRT160NL board support
 *
 *  Copyright (C) 2009-2012 Gabor Juhos <juhosg@openwrt.org>
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
#include "dev-usb.h"
#include "dev-wmac.h"
#include "nvram.h"
#include "machtypes.h"

#define WRT160NL_GPIO_LED_POWER		14
#define WRT160NL_GPIO_LED_WPS_AMBER	9
#define WRT160NL_GPIO_LED_WPS_BLUE	8
#define WRT160NL_GPIO_LED_WLAN		6

#define WRT160NL_GPIO_BTN_WPS		7
#define WRT160NL_GPIO_BTN_RESET		21

#define WRT160NL_KEYS_POLL_INTERVAL	20	/* msecs */
#define WRT160NL_KEYS_DEBOUNCE_INTERVAL	(3 * WRT160NL_KEYS_POLL_INTERVAL)

#define WRT160NL_NVRAM_ADDR	0x1f7e0000
#define WRT160NL_NVRAM_SIZE	0x10000

static const char *wrt160nl_part_probes[] = {
	"cybertan",
	NULL,
};

static struct flash_platform_data wrt160nl_flash_data = {
	.part_probes	= wrt160nl_part_probes,
};

static struct gpio_led wrt160nl_leds_gpio[] __initdata = {
	{
		.name		= "wrt160nl:blue:power",
		.gpio		= WRT160NL_GPIO_LED_POWER,
		.active_low	= 1,
		.default_trigger = "default-on",
	}, {
		.name		= "wrt160nl:amber:wps",
		.gpio		= WRT160NL_GPIO_LED_WPS_AMBER,
		.active_low	= 1,
	}, {
		.name		= "wrt160nl:blue:wps",
		.gpio		= WRT160NL_GPIO_LED_WPS_BLUE,
		.active_low	= 1,
	}, {
		.name		= "wrt160nl:blue:wlan",
		.gpio		= WRT160NL_GPIO_LED_WLAN,
		.active_low	= 1,
	}
};

static struct gpio_keys_button wrt160nl_gpio_keys[] __initdata = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = WRT160NL_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= WRT160NL_GPIO_BTN_RESET,
		.active_low	= 1,
	}, {
		.desc		= "wps",
		.type		= EV_KEY,
		.code		= KEY_WPS_BUTTON,
		.debounce_interval = WRT160NL_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= WRT160NL_GPIO_BTN_WPS,
		.active_low	= 1,
	}
};

static void __init wrt160nl_setup(void)
{
	const char *nvram = (char *) KSEG1ADDR(WRT160NL_NVRAM_ADDR);
	u8 *eeprom = (u8 *) KSEG1ADDR(0x1fff1000);
	u8 mac[6];

	if (ath79_nvram_parse_mac_addr(nvram, WRT160NL_NVRAM_SIZE,
				       "lan_hwaddr=", mac) == 0) {
		ath79_init_mac(ath79_eth0_data.mac_addr, mac, 0);
		ath79_init_mac(ath79_eth1_data.mac_addr, mac, 1);
	}

	ath79_register_mdio(0, 0x0);

	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RMII;
	ath79_eth0_data.phy_mask = 0x01;

	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_RMII;
	ath79_eth1_data.phy_mask = 0x10;

	ath79_register_eth(0);
	ath79_register_eth(1);

	ath79_register_m25p80(&wrt160nl_flash_data);

	ath79_register_usb();

	if (ath79_nvram_parse_mac_addr(nvram, WRT160NL_NVRAM_SIZE,
				       "wl0_hwaddr=", mac) == 0)
		ath79_register_wmac(eeprom, mac);
	else
		ath79_register_wmac(eeprom, NULL);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(wrt160nl_leds_gpio),
				 wrt160nl_leds_gpio);

	ath79_register_gpio_keys_polled(-1, WRT160NL_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(wrt160nl_gpio_keys),
					wrt160nl_gpio_keys);
}

MIPS_MACHINE(ATH79_MACH_WRT160NL, "WRT160NL", "Linksys WRT160NL",
	     wrt160nl_setup);

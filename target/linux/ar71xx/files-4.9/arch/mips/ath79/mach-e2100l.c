/*
 *  Linksys E2100L board support
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

#define E2100L_GPIO_LED_POWER		14
#define E2100L_GPIO_LED_WPS_AMBER	9
#define E2100L_GPIO_LED_WPS_BLUE	8
#define E2100L_GPIO_LED_WLAN		6

#define E2100L_GPIO_BTN_WPS		7
#define E2100L_GPIO_BTN_RESET		21

#define E2100L_KEYS_POLL_INTERVAL	20	/* msecs */
#define E2100L_KEYS_DEBOUNCE_INTERVAL	(3 * E2100L_KEYS_POLL_INTERVAL)

#define E2100L_NVRAM_ADDR	0x1f7e0000
#define E2100L_NVRAM_SIZE	0x10000

static const char *e2100l_part_probes[] = {
	"cybertan",
	NULL,
};

static struct flash_platform_data e2100l_flash_data = {
	.part_probes	= e2100l_part_probes,
};

static struct gpio_led e2100l_leds_gpio[] __initdata = {
	{
		.name		= "e2100l:blue:power",
		.gpio		= E2100L_GPIO_LED_POWER,
		.active_low	= 1,
		.default_trigger = "default-on",
	}, {
		.name		= "e2100l:amber:wps",
		.gpio		= E2100L_GPIO_LED_WPS_AMBER,
		.active_low	= 1,
	}, {
		.name		= "e2100l:blue:wps",
		.gpio		= E2100L_GPIO_LED_WPS_BLUE,
		.active_low	= 1,
	}, {
		.name		= "e2100l:blue:wlan",
		.gpio		= E2100L_GPIO_LED_WLAN,
		.active_low	= 1,
	}
};

static struct gpio_keys_button e2100l_gpio_keys[] __initdata = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = E2100L_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= E2100L_GPIO_BTN_RESET,
		.active_low	= 1,
	}, {
		.desc		= "wps",
		.type		= EV_KEY,
		.code		= KEY_WPS_BUTTON,
		.debounce_interval = E2100L_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= E2100L_GPIO_BTN_WPS,
		.active_low	= 1,
	}
};

static void __init e2100l_setup(void)
{
	const char *nvram = (char *) KSEG1ADDR(E2100L_NVRAM_ADDR);
	u8 *eeprom = (u8 *) KSEG1ADDR(0x1fff1000);
	u8 mac[6];

	if (ath79_nvram_parse_mac_addr(nvram, E2100L_NVRAM_SIZE,
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

	ath79_register_m25p80(&e2100l_flash_data);

	ath79_register_usb();

	if (ath79_nvram_parse_mac_addr(nvram, E2100L_NVRAM_SIZE,
				       "wl0_hwaddr=", mac) == 0)
		ath79_register_wmac(eeprom, mac);
	else
		ath79_register_wmac(eeprom, NULL);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(e2100l_leds_gpio),
				 e2100l_leds_gpio);

	ath79_register_gpio_keys_polled(-1, E2100L_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(e2100l_gpio_keys),
					e2100l_gpio_keys);
}

MIPS_MACHINE(ATH79_MACH_E2100L, "E2100L", "Linksys E2100L",
	     e2100l_setup);

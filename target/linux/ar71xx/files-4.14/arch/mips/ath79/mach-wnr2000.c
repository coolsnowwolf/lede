/*
 *  NETGEAR WNR2000 board support
 *
 *  Copyright (C) 2008-2009 Gabor Juhos <juhosg@openwrt.org>
 *  Copyright (C) 2008 Imre Kaloz <kaloz@openwrt.org>
 *  Copyright (C) 2008-2009 Andy Boyett <agb@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>

#include <asm/mach-ath79/ath79.h>

#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-wmac.h"
#include "machtypes.h"

#define WNR2000_GPIO_LED_PWR_GREEN	14
#define WNR2000_GPIO_LED_PWR_AMBER	7
#define WNR2000_GPIO_LED_WPS		4
#define WNR2000_GPIO_LED_WLAN		6
#define WNR2000_GPIO_BTN_RESET		21
#define WNR2000_GPIO_BTN_WPS		8

#define WNR2000_KEYS_POLL_INTERVAL	20	/* msecs */
#define WNR2000_KEYS_DEBOUNCE_INTERVAL	(3 * WNR2000_KEYS_POLL_INTERVAL)

static struct gpio_led wnr2000_leds_gpio[] __initdata = {
	{
		.name		= "netgear:green:power",
		.gpio		= WNR2000_GPIO_LED_PWR_GREEN,
		.active_low	= 1,
	}, {
		.name		= "netgear:amber:power",
		.gpio		= WNR2000_GPIO_LED_PWR_AMBER,
		.active_low	= 1,
	}, {
		.name		= "netgear:green:wps",
		.gpio		= WNR2000_GPIO_LED_WPS,
		.active_low	= 1,
	}, {
		.name		= "netgear:blue:wlan",
		.gpio		= WNR2000_GPIO_LED_WLAN,
		.active_low	= 1,
	}
};

static struct gpio_keys_button wnr2000_gpio_keys[] __initdata = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = WNR2000_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= WNR2000_GPIO_BTN_RESET,
	}, {
		.desc		= "wps",
		.type		= EV_KEY,
		.code		= KEY_WPS_BUTTON,
		.debounce_interval = WNR2000_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= WNR2000_GPIO_BTN_WPS,
	}
};

static void __init wnr2000_setup(void)
{
	u8 *eeprom = (u8 *) KSEG1ADDR(0x1fff1000);

	ath79_register_mdio(0, 0x0);

	ath79_init_mac(ath79_eth0_data.mac_addr, eeprom, 0);
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RMII;
	ath79_eth0_data.speed = SPEED_100;
	ath79_eth0_data.duplex = DUPLEX_FULL;
	ath79_eth0_data.has_ar8216 = 1;

	ath79_init_mac(ath79_eth1_data.mac_addr, eeprom, 1);
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_RMII;
	ath79_eth1_data.phy_mask = 0x10;

	ath79_register_eth(0);
	ath79_register_eth(1);

	ath79_register_m25p80(NULL);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(wnr2000_leds_gpio),
				 wnr2000_leds_gpio);

	ath79_register_gpio_keys_polled(-1, WNR2000_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(wnr2000_gpio_keys),
					wnr2000_gpio_keys);

	ath79_register_wmac(eeprom, NULL);
}

MIPS_MACHINE(ATH79_MACH_WNR2000, "WNR2000", "NETGEAR WNR2000", wnr2000_setup);

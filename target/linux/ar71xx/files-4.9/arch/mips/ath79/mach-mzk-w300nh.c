/*
 *  Planex MZK-W300NH board support
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

#define MZK_W300NH_GPIO_LED_STATUS	1
#define MZK_W300NH_GPIO_LED_WPS		3
#define MZK_W300NH_GPIO_LED_WLAN	6
#define MZK_W300NH_GPIO_LED_AP_GREEN	15
#define MZK_W300NH_GPIO_LED_AP_AMBER	16

#define MZK_W300NH_GPIO_BTN_APROUTER	5
#define MZK_W300NH_GPIO_BTN_WPS		12
#define MZK_W300NH_GPIO_BTN_RESET	21

#define MZK_W300NH_KEYS_POLL_INTERVAL	20	/* msecs */
#define MZK_W300NH_KEYS_DEBOUNCE_INTERVAL (3 * MZK_W300NH_KEYS_POLL_INTERVAL)

static struct gpio_led mzk_w300nh_leds_gpio[] __initdata = {
	{
		.name		= "planex:green:status",
		.gpio		= MZK_W300NH_GPIO_LED_STATUS,
		.active_low	= 1,
	}, {
		.name		= "planex:blue:wps",
		.gpio		= MZK_W300NH_GPIO_LED_WPS,
		.active_low	= 1,
	}, {
		.name		= "planex:green:wlan",
		.gpio		= MZK_W300NH_GPIO_LED_WLAN,
		.active_low	= 1,
	}, {
		.name		= "planex:green:aprouter",
		.gpio		= MZK_W300NH_GPIO_LED_AP_GREEN,
	}, {
		.name		= "planex:amber:aprouter",
		.gpio		= MZK_W300NH_GPIO_LED_AP_AMBER,
	}
};

static struct gpio_keys_button mzk_w300nh_gpio_keys[] __initdata = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = MZK_W300NH_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= MZK_W300NH_GPIO_BTN_RESET,
		.active_low	= 1,
	}, {
		.desc		= "wps",
		.type		= EV_KEY,
		.code		= KEY_WPS_BUTTON,
		.debounce_interval = MZK_W300NH_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= MZK_W300NH_GPIO_BTN_WPS,
		.active_low	= 1,
	}, {
		.desc		= "aprouter",
		.type		= EV_KEY,
		.code		= BTN_2,
		.debounce_interval = MZK_W300NH_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= MZK_W300NH_GPIO_BTN_APROUTER,
		.active_low	= 0,
	}
};

#define MZK_W300NH_WAN_PHYMASK	BIT(4)
#define MZK_W300NH_MDIO_MASK	(~MZK_W300NH_WAN_PHYMASK)

static void __init mzk_w300nh_setup(void)
{
	u8 *eeprom = (u8 *) KSEG1ADDR(0x1fff1000);

	ath79_register_mdio(0, MZK_W300NH_MDIO_MASK);

	ath79_init_mac(ath79_eth0_data.mac_addr, eeprom, 0);
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RMII;
	ath79_eth0_data.speed = SPEED_100;
	ath79_eth0_data.duplex = DUPLEX_FULL;
	ath79_eth0_data.has_ar8216 = 1;

	ath79_init_mac(ath79_eth1_data.mac_addr, eeprom, 1);
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_RMII;
	ath79_eth1_data.phy_mask = MZK_W300NH_WAN_PHYMASK;

	ath79_register_eth(0);
	ath79_register_eth(1);

	ath79_register_m25p80(NULL);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(mzk_w300nh_leds_gpio),
				 mzk_w300nh_leds_gpio);

	ath79_register_gpio_keys_polled(-1, MZK_W300NH_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(mzk_w300nh_gpio_keys),
					mzk_w300nh_gpio_keys);
	ath79_register_wmac(eeprom, NULL);
}

MIPS_MACHINE(ATH79_MACH_MZK_W300NH, "MZK-W300NH", "Planex MZK-W300NH",
	     mzk_w300nh_setup);

/*
 *  D-Link DIR-615 rev. I1 board support
 *  Copyright (C) 2013-2015 Jaehoon You <teslamint@gmail.com>
 *
 *  based on the DIR-600 rev. A1 board support code
 *    Copyright (C) 2010-2012 Gabor Juhos <juhosg@openwrt.org>
 *    Copyright (C) 2012 Vadim Girlin <vadimgirlin@gmail.com>
 *
 *  based on the TP-LINK TL-WR841N/ND v8/TL-MR3420 v2 board support code
 *    Copyright (C) 2012 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/platform_device.h>

#include <asm/mach-ath79/ath79.h>
#include <asm/mach-ath79/ar71xx_regs.h>

#include "common.h"
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-wmac.h"
#include "machtypes.h"

#define DIR_615_I1_GPIO_LED_WPS				15
#define DIR_615_I1_GPIO_LED_POWER_AMBER		14
#define DIR_615_I1_GPIO_LED_POWER_GREEN		4
#define DIR_615_I1_GPIO_LED_WAN_AMBER		22
#define DIR_615_I1_GPIO_LED_WAN_GREEN		12
#define DIR_615_I1_GPIO_LED_WLAN_GREEN		13

#define DIR_615_I1_GPIO_BTN_WPS				16
#define DIR_615_I1_GPIO_BTN_RESET			17

#define DIR_615_I1_KEYS_POLL_INTERVAL		20 /* msecs */
#define DIR_615_I1_KEYS_DEBOUNCE_INTERVAL	(3 * DIR_615_I1_KEYS_POLL_INTERVAL)

#define DIR_615_I1_LAN_PHYMASK	BIT(0)
#define DIR_615_I1_WAN_PHYMASK	BIT(4)
#define DIR_615_I1_WLAN_MAC_ADDR	0x1fffffb4

static struct gpio_led dir_615_i1_leds_gpio[] __initdata = {
	{
		.name		= "d-link:green:power",
		.gpio		= DIR_615_I1_GPIO_LED_POWER_GREEN,
	}, {
		.name		= "d-link:amber:power",
		.gpio		= DIR_615_I1_GPIO_LED_POWER_AMBER,
	}, {
		.name		= "d-link:amber:wan",
		.gpio		= DIR_615_I1_GPIO_LED_WAN_AMBER,
	}, {
		.name		= "d-link:green:wan",
		.gpio		= DIR_615_I1_GPIO_LED_WAN_GREEN,
		.active_low = 1,
	}, {
		.name		= "d-link:green:wlan",
		.gpio		= DIR_615_I1_GPIO_LED_WLAN_GREEN,
		.active_low = 1,
	}, {
		.name		= "d-link:blue:wps",
		.gpio		= DIR_615_I1_GPIO_LED_WPS,
		.active_low = 1,
	}
};

static struct gpio_keys_button dir_615_i1_gpio_keys[] __initdata = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = DIR_615_I1_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= DIR_615_I1_GPIO_BTN_RESET,
		.active_low	= 1,
	}, {
		.desc		= "wps",
		.type		= EV_KEY,
		.code		= KEY_WPS_BUTTON,
		.debounce_interval = DIR_615_I1_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= DIR_615_I1_GPIO_BTN_WPS,
		.active_low	= 1,
	}
};

static void __init dir_615_i1_setup(void)
{
	u8 *eeprom  = (u8 *) KSEG1ADDR(0x1fff1000);
	u8 mac[ETH_ALEN];

	ath79_register_mdio(0, 0x0);
	ath79_register_mdio(1, ~(DIR_615_I1_WAN_PHYMASK));

	ath79_parse_ascii_mac((char *) KSEG1ADDR(DIR_615_I1_WLAN_MAC_ADDR), mac);
	ath79_init_mac(ath79_eth1_data.mac_addr, mac, 0);
	ath79_init_mac(ath79_eth0_data.mac_addr, mac, 1);

	/* GMAC0 is connected to the PHY0 of the internal switch */
	ath79_switch_data.phy4_mii_en = 1;
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ath79_eth0_data.phy_mask = DIR_615_I1_WAN_PHYMASK;
	ath79_eth0_data.mii_bus_dev = &ath79_mdio1_device.dev;

	/* GMAC1 is connected to the internal switch */
	ath79_eth1_data.phy_mask = DIR_615_I1_LAN_PHYMASK;
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_GMII;

	ath79_register_eth(0);
	ath79_register_eth(1);

	ath79_register_m25p80(NULL);

	/* Disable JTAG, enabling GPIOs 0-3 */
	/* Configure OBS4 line, for GPIO 4*/
	ath79_gpio_function_setup(AR934X_GPIO_FUNC_JTAG_DISABLE,
				 AR934X_GPIO_FUNC_CLK_OBS4_EN);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(dir_615_i1_leds_gpio),
					dir_615_i1_leds_gpio);

	ath79_register_gpio_keys_polled(-1, DIR_615_I1_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(dir_615_i1_gpio_keys),
					dir_615_i1_gpio_keys);

	ath79_register_wmac(eeprom, mac);
}

MIPS_MACHINE(ATH79_MACH_DIR_615_I1, "DIR-615-I1", "D-Link DIR-615 rev. I1",
		dir_615_i1_setup);

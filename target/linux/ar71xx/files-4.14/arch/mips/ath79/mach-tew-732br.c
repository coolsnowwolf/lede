/*
 *  TRENDnet TEW-732BR board support
 *
 *  Copyright (C) 2013 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/gpio.h>
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

#define TEW_732BR_GPIO_BTN_WPS		16
#define TEW_732BR_GPIO_BTN_RESET	17

#define TEW_732BR_GPIO_LED_POWER_GREEN	4
#define TEW_732BR_GPIO_LED_POWER_AMBER	14
#define TEW_732BR_GPIO_LED_PLANET_GREEN	12
#define TEW_732BR_GPIO_LED_PLANET_AMBER 22

#define TEW_732BR_KEYS_POLL_INTERVAL	20	/* msecs */
#define TEW_732BR_KEYS_DEBOUNCE_INTERVAL (3 * TEW_732BR_KEYS_POLL_INTERVAL)

#define TEW_732BR_ART_ADDRESS		0x1fff0000
#define TEW_732BR_CALDATA_OFFSET	0x1000
#define TEW_732BR_LAN_MAC_OFFSET	0xffa0
#define TEW_732BR_WAN_MAC_OFFSET	0xffb4

static struct gpio_led tew_732br_leds_gpio[] __initdata = {
	{
		.name		= "trendnet:green:power",
		.gpio		= TEW_732BR_GPIO_LED_POWER_GREEN,
		.active_low	= 0,
	},
	{
		.name		= "trendnet:amber:power",
		.gpio		= TEW_732BR_GPIO_LED_POWER_AMBER,
		.active_low	= 0,
	},
	{
		.name		= "trendnet:green:wan",
		.gpio		= TEW_732BR_GPIO_LED_PLANET_GREEN,
		.active_low	= 1,
	},
	{
		.name		= "trendnet:amber:wan",
		.gpio		= TEW_732BR_GPIO_LED_PLANET_AMBER,
		.active_low	= 0,
	},
};

static struct gpio_keys_button tew_732br_gpio_keys[] __initdata = {
	{
		.desc		= "Reset button",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = TEW_732BR_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= TEW_732BR_GPIO_BTN_RESET,
		.active_low	= 1,
	},
	{
		.desc		= "WPS button",
		.type		= EV_KEY,
		.code		= KEY_WPS_BUTTON,
		.debounce_interval = TEW_732BR_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= TEW_732BR_GPIO_BTN_WPS,
		.active_low	= 1,
	},
};

static void __init tew_732br_setup(void)
{
	u8 *art = (u8 *) KSEG1ADDR(TEW_732BR_ART_ADDRESS);
	u8 lan_mac[ETH_ALEN];
	u8 wan_mac[ETH_ALEN];

	ath79_register_leds_gpio(-1, ARRAY_SIZE(tew_732br_leds_gpio),
				 tew_732br_leds_gpio);

	ath79_register_gpio_keys_polled(1, TEW_732BR_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(tew_732br_gpio_keys),
					tew_732br_gpio_keys);

	ath79_register_m25p80(NULL);

	ath79_parse_ascii_mac(art + TEW_732BR_LAN_MAC_OFFSET, lan_mac);
	ath79_parse_ascii_mac(art + TEW_732BR_WAN_MAC_OFFSET, wan_mac);

	ath79_register_wmac(art + TEW_732BR_CALDATA_OFFSET, lan_mac);

	ath79_register_mdio(1, 0x0);

	ath79_setup_ar934x_eth_cfg(AR934X_ETH_CFG_SW_ONLY_MODE);

	/* LAN: GMAC1 is connected to the internal switch */
	ath79_init_mac(ath79_eth1_data.mac_addr, lan_mac, 0);
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_GMII;

	ath79_register_eth(1);

	/* WAN: GMAC0 is connected to the PHY4 of the internal switch */
	ath79_init_mac(ath79_eth0_data.mac_addr, wan_mac, 0);

	ath79_switch_data.phy4_mii_en = 1;
	ath79_switch_data.phy_poll_mask = BIT(4);

	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ath79_eth0_data.phy_mask = BIT(4);
	ath79_eth0_data.mii_bus_dev = &ath79_mdio1_device.dev;

	ath79_register_eth(0);
}

MIPS_MACHINE(ATH79_MACH_TEW_732BR, "TEW-732BR", "TRENDnet TEW-732BR",
	     tew_732br_setup);

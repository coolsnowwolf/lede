/*
 *  TRENDnet TEW-712BR board support
 *
 *  Copyright (C) 2012 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/gpio.h>

#include <asm/mach-ath79/ath79.h>
#include <asm/mach-ath79/ar71xx_regs.h>

#include "common.h"
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-wmac.h"
#include "machtypes.h"

#define TEW_712BR_GPIO_BTN_WPS		11
#define TEW_712BR_GPIO_BTN_RESET	12

#define TEW_712BR_GPIO_LED_LAN1		13
#define TEW_712BR_GPIO_LED_LAN2		14
#define TEW_712BR_GPIO_LED_LAN3		15
#define TEW_712BR_GPIO_LED_LAN4		16
#define TEW_712BR_GPIO_LED_POWER_GREEN	20
#define TEW_712BR_GPIO_LED_POWER_ORANGE	27
#define TEW_712BR_GPIO_LED_WAN_GREEN	17
#define TEW_712BR_GPIO_LED_WAN_ORANGE	23
#define TEW_712BR_GPIO_LED_WLAN		0
#define TEW_712BR_GPIO_LED_WPS		26

#define TEW_712BR_GPIO_WAN_LED_ENABLE	1

#define TEW_712BR_KEYS_POLL_INTERVAL	20	/* msecs */
#define TEW_712BR_KEYS_DEBOUNCE_INTERVAL (3 * TEW_712BR_KEYS_POLL_INTERVAL)

#define TEW_712BR_ART_ADDRESS		0x1f010000
#define TEW_712BR_CALDATA_OFFSET	0x1000

#define TEW_712BR_MAC_PART_ADDRESS	0x1f020000
#define TEW_712BR_LAN_MAC_OFFSET	0x04
#define TEW_712BR_WAN_MAC_OFFSET	0x16

static struct gpio_led tew_712br_leds_gpio[] __initdata = {
	{
		.name		= "trendnet:green:lan1",
		.gpio		= TEW_712BR_GPIO_LED_LAN1,
		.active_low	= 0,
	}, {
		.name		= "trendnet:green:lan2",
		.gpio		= TEW_712BR_GPIO_LED_LAN2,
		.active_low	= 0,
	}, {
		.name		= "trendnet:green:lan3",
		.gpio		= TEW_712BR_GPIO_LED_LAN3,
		.active_low	= 0,
	}, {
		.name		= "trendnet:green:lan4",
		.gpio		= TEW_712BR_GPIO_LED_LAN4,
		.active_low	= 0,
	}, {
		.name		= "trendnet:blue:wps",
		.gpio		= TEW_712BR_GPIO_LED_WPS,
		.active_low	= 1,
	}, {
		.name		= "trendnet:green:power",
		.gpio		= TEW_712BR_GPIO_LED_POWER_GREEN,
		.active_low	= 0,
	}, {
		.name		= "trendnet:orange:power",
		.gpio		= TEW_712BR_GPIO_LED_POWER_ORANGE,
		.active_low	= 0,
	}, {
		.name		= "trendnet:green:wan",
		.gpio		= TEW_712BR_GPIO_LED_WAN_GREEN,
		.active_low	= 1,
	}, {
		.name		= "trendnet:orange:wan",
		.gpio		= TEW_712BR_GPIO_LED_WAN_ORANGE,
		.active_low	= 0,
	}, {
		.name		= "trendnet:green:wlan",
		.gpio		= TEW_712BR_GPIO_LED_WLAN,
		.active_low	= 0,
	},
};

static struct gpio_keys_button tew_712br_gpio_keys[] __initdata = {
	{
		.desc		= "Reset button",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = TEW_712BR_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= TEW_712BR_GPIO_BTN_RESET,
		.active_low	= 1,
	}, {
		.desc		= "WPS button",
		.type		= EV_KEY,
		.code		= KEY_WPS_BUTTON,
		.debounce_interval = TEW_712BR_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= TEW_712BR_GPIO_BTN_WPS,
		.active_low	= 1,
	}
};

static void __init tew_712br_setup(void)
{
	u8 *art = (u8 *) KSEG1ADDR(TEW_712BR_ART_ADDRESS);
	u8 *mac = (u8 *) KSEG1ADDR(TEW_712BR_MAC_PART_ADDRESS);
	u8 lan_mac[ETH_ALEN];
	u8 wan_mac[ETH_ALEN];

	ath79_setup_ar933x_phy4_switch(false, false);

	ath79_gpio_function_disable(AR933X_GPIO_FUNC_ETH_SWITCH_LED0_EN |
				    AR933X_GPIO_FUNC_ETH_SWITCH_LED1_EN |
				    AR933X_GPIO_FUNC_ETH_SWITCH_LED2_EN |
				    AR933X_GPIO_FUNC_ETH_SWITCH_LED3_EN |
				    AR933X_GPIO_FUNC_ETH_SWITCH_LED4_EN);

	gpio_request_one(TEW_712BR_GPIO_WAN_LED_ENABLE,
			 GPIOF_OUT_INIT_LOW, "WAN LED enable");

	ath79_register_leds_gpio(-1, ARRAY_SIZE(tew_712br_leds_gpio),
				 tew_712br_leds_gpio);

	ath79_register_gpio_keys_polled(1, TEW_712BR_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(tew_712br_gpio_keys),
					tew_712br_gpio_keys);

	ath79_register_m25p80(NULL);

	ath79_parse_ascii_mac(mac + TEW_712BR_LAN_MAC_OFFSET, lan_mac);
	ath79_parse_ascii_mac(mac + TEW_712BR_WAN_MAC_OFFSET, wan_mac);

	ath79_init_mac(ath79_eth0_data.mac_addr, wan_mac, 0);
	ath79_init_mac(ath79_eth1_data.mac_addr, lan_mac, 0);

	ath79_register_mdio(0, 0x0);
	ath79_register_eth(1);
	ath79_register_eth(0);

	ath79_register_wmac(art + TEW_712BR_CALDATA_OFFSET, wan_mac);
}

MIPS_MACHINE(ATH79_MACH_TEW_712BR, "TEW-712BR",
	     "TRENDnet TEW-712BR", tew_712br_setup);

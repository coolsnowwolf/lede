/*
 *  DLink DIR-505 A1 board support
 *
 *  Copyright (C) 2013 Gabor Juhos <juhosg@openwrt.org>
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
#include "dev-usb.h"
#include "machtypes.h"

#define DIR_505A1_GPIO_BTN_WPS         11 /* verify */
#define DIR_505A1_GPIO_BTN_RESET       12 /* verify */

#define DIR_505A1_GPIO_LED_RED         26 /* unused, fyi */
#define DIR_505A1_GPIO_LED_GREEN       27

#define DIR_505A1_GPIO_WAN_LED_ENABLE  1

#define DIR_505A1_KEYS_POLL_INTERVAL   20      /* msecs */
#define DIR_505A1_KEYS_DEBOUNCE_INTERVAL (3 * DIR_505A1_KEYS_POLL_INTERVAL)

#define DIR_505A1_ART_ADDRESS          0x1f010000
#define DIR_505A1_CALDATA_OFFSET       0x1000

#define DIR_505A1_MAC_PART_ADDRESS	0x1f020000
#define DIR_505A1_LAN_MAC_OFFSET	0x04
#define DIR_505A1_WAN_MAC_OFFSET	0x16

static struct gpio_led dir_505_a1_leds_gpio[] __initdata = {
	{
		.name		= "d-link:green:power",
		.gpio		= DIR_505A1_GPIO_LED_GREEN,
		.active_low	= 1,
	}, {
		.name		= "d-link:red:status",
		.gpio		= DIR_505A1_GPIO_LED_RED,
		.active_low	= 1,
	},
};

static struct gpio_keys_button dir_505_a1_gpio_keys[] __initdata = {
	{
		.desc		= "Reset button",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = DIR_505A1_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= DIR_505A1_GPIO_BTN_RESET,
		.active_low	= 0,
	}, {
		.desc		= "WPS button",
		.type		= EV_KEY,
		.code		= KEY_WPS_BUTTON,
		.debounce_interval = DIR_505A1_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= DIR_505A1_GPIO_BTN_WPS,
		.active_low	= 1,
	}
};

static void __init dir_505_a1_setup(void)
{
	u8 *art = (u8 *) KSEG1ADDR(DIR_505A1_ART_ADDRESS);
	u8 *mac = (u8 *) KSEG1ADDR(DIR_505A1_MAC_PART_ADDRESS);
	u8 lan_mac[ETH_ALEN];
	u8 wan_mac[ETH_ALEN];

	ath79_setup_ar933x_phy4_switch(false, false);

	ath79_gpio_function_disable(AR933X_GPIO_FUNC_ETH_SWITCH_LED0_EN |
		AR933X_GPIO_FUNC_ETH_SWITCH_LED1_EN |
		AR933X_GPIO_FUNC_ETH_SWITCH_LED2_EN |
		AR933X_GPIO_FUNC_ETH_SWITCH_LED3_EN |
		AR933X_GPIO_FUNC_ETH_SWITCH_LED4_EN);

	gpio_request_one(DIR_505A1_GPIO_WAN_LED_ENABLE,
		GPIOF_OUT_INIT_LOW, "WAN LED enable");

	ath79_register_leds_gpio(-1, ARRAY_SIZE(dir_505_a1_leds_gpio),
		dir_505_a1_leds_gpio);

	ath79_register_gpio_keys_polled(1, DIR_505A1_KEYS_POLL_INTERVAL,
		ARRAY_SIZE(dir_505_a1_gpio_keys),
		dir_505_a1_gpio_keys);

	ath79_register_m25p80(NULL);

	ath79_register_usb();

	ath79_parse_ascii_mac(mac + DIR_505A1_LAN_MAC_OFFSET, lan_mac);
	ath79_parse_ascii_mac(mac + DIR_505A1_WAN_MAC_OFFSET, wan_mac);

	ath79_init_mac(ath79_eth0_data.mac_addr, wan_mac, 0);
	ath79_init_mac(ath79_eth1_data.mac_addr, lan_mac, 0);

	ath79_register_mdio(0, 0x0);
	ath79_register_eth(1);
	ath79_register_eth(0);

	ath79_register_wmac(art + DIR_505A1_CALDATA_OFFSET, lan_mac);
}

MIPS_MACHINE(ATH79_MACH_DIR_505_A1, "DIR-505-A1",
		"D-Link DIR-505 rev. A1", dir_505_a1_setup);

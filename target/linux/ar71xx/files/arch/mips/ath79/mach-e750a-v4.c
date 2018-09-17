/*
 *  Qxwlan E750A v4 board support
 *
 *  Copyright (C) 2017 Peng Zhang <sd20@qxwlan.com>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/phy.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>
#include <linux/ath9k_platform.h>
#include <linux/ar8216_platform.h>

#include <asm/mach-ath79/ar71xx_regs.h>

#include "common.h"
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-usb.h"
#include "dev-wmac.h"
#include "machtypes.h"
#include "pci.h"

#define E750A_V4_GPIO_LED_SYS	14
#define E750A_V4_GPIO_LED_LAN	19
#define E750A_V4_GPIO_LED_WAN	18
#define E750A_V4_GPIO_LED_DS10	15
#define E750A_V4_GPIO_LED_DS20	20
#define E750A_V4_GPIO_LED_WLAN	21

#define E750A_V4_GPIO_BTN_RESET	12

#define E750A_V4_KEYS_POLL_INTERVAL	20	/* msecs */
#define E750A_V4_KEYS_DEBOUNCE_INTERVAL	(3 * E750A_V4_KEYS_POLL_INTERVAL)

static struct gpio_led e750a_v4_leds_gpio[] __initdata = {
	{
		.name		= "e750a-v4:green:system",
		.gpio		= E750A_V4_GPIO_LED_SYS,
		.active_low	= 1,
	},
	{
		.name		= "e750a-v4:green:lan",
		.gpio		= E750A_V4_GPIO_LED_LAN,
		.active_low	= 1,
	},
	{
		.name		= "e750a-v4:green:wan",
		.gpio		= E750A_V4_GPIO_LED_WAN,
		.active_low	= 1,
	},
	{
		.name		= "e750a-v4:green:wlan",
		.gpio		= E750A_V4_GPIO_LED_WLAN,
		.active_low	= 1,
	},
	{
		.name		= "e750a-v4:green:ds10",
		.gpio		= E750A_V4_GPIO_LED_DS10,
		.active_low	= 1,
	},
	{
		.name		= "e750a-v4:green:ds20",
		.gpio		= E750A_V4_GPIO_LED_DS20,
		.active_low	= 1,
	},
};

static struct gpio_keys_button e750a_v4_gpio_keys[] __initdata = {
	{
		.desc           = "reset",
		.type           = EV_KEY,
		.code           = KEY_RESTART,
		.debounce_interval = E750A_V4_KEYS_DEBOUNCE_INTERVAL,
		.gpio           = E750A_V4_GPIO_BTN_RESET,
		.active_low     = 1,
	},
};


static void __init e750a_v4_setup(void)
{
	u8 *mac = (u8 *) KSEG1ADDR(0x1f050400);
	u8 *art = (u8 *) KSEG1ADDR(0x1f061000);

	ath79_register_m25p80(NULL);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(e750a_v4_leds_gpio),
			e750a_v4_leds_gpio);
	ath79_register_gpio_keys_polled(-1, E750A_V4_KEYS_POLL_INTERVAL,
			ARRAY_SIZE(e750a_v4_gpio_keys),
			e750a_v4_gpio_keys);

	ath79_register_mdio(1, 0x0);
	ath79_setup_ar934x_eth_cfg(AR934X_ETH_CFG_SW_ONLY_MODE);

	/* GMAC1 is connected to the internal switch */
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_GMII;
	ath79_init_mac(ath79_eth1_data.mac_addr, mac, 0);
	ath79_register_eth(1);

	/* GMAC0 is connected to the PHY4 of the internal switch */
	ath79_switch_data.phy4_mii_en = 1;
	ath79_switch_data.phy_poll_mask = BIT(4);
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ath79_eth0_data.phy_mask = BIT(4);
	ath79_eth0_data.mii_bus_dev = &ath79_mdio1_device.dev;
	ath79_init_mac(ath79_eth0_data.mac_addr, mac, 1);
	ath79_register_eth(0);

	ath79_register_pci();
	ath79_register_usb();
	ath79_register_wmac(art, NULL);
}

MIPS_MACHINE(ATH79_MACH_E750A_V4, "E750A-V4", "Qxlan E750A v4",
		e750a_v4_setup);

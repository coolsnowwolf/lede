/*
 * ALFA Network N5Q board support
 *
 * Copyright (C) 2018 Piotr Dymacz <pepe2k@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
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

#define N5Q_GPIO_LED_LAN	19
#define N5Q_GPIO_LED_SIGNAL1	0
#define N5Q_GPIO_LED_SIGNAL2	1
#define N5Q_GPIO_LED_SIGNAL3	2
#define N5Q_GPIO_LED_SIGNAL4	3
#define N5Q_GPIO_LED_WAN	18
#define N5Q_GPIO_LED_WLAN	12

#define N5Q_GPIO_WDT_EN		16
#define N5Q_GPIO_WDT_IN		17

#define N5Q_GPIO_BTN_RESET	11

#define N5Q_MAC0_OFFSET		0
#define N5Q_MAC1_OFFSET		6
#define N5Q_WMAC_CALDATA_OFFSET	0x1000

#define N5Q_KEYS_POLL_INTERVAL		20
#define N5Q_KEYS_DEBOUNCE_INTERVAL	(3 * N5Q_KEYS_POLL_INTERVAL)

static struct gpio_led n5q_leds_gpio[] __initdata = {
	{
		.name		= "n5q:green:lan",
		.gpio		= N5Q_GPIO_LED_LAN,
		.active_low	= 1,
	}, {
		.name		= "n5q:red:signal1",
		.gpio		= N5Q_GPIO_LED_SIGNAL1,
		.active_low	= 1,
	}, {
		.name		= "n5q:orange:signal2",
		.gpio		= N5Q_GPIO_LED_SIGNAL2,
		.active_low	= 1,
	}, {
		.name		= "n5q:green:signal3",
		.gpio		= N5Q_GPIO_LED_SIGNAL3,
		.active_low	= 1,
	}, {
		.name		= "n5q:green:signal4",
		.gpio		= N5Q_GPIO_LED_SIGNAL4,
		.active_low	= 1,
	}, {
		.name		= "n5q:green:wan",
		.gpio		= N5Q_GPIO_LED_WAN,
		.active_low	= 1,
	}, {
		.name		= "n5q:green:wlan",
		.gpio		= N5Q_GPIO_LED_WLAN,
		.active_low	= 1,
	},
};

static struct gpio_keys_button n5q_gpio_keys[] __initdata = {
	{
		.desc			= "reset",
		.type			= EV_KEY,
		.code			= KEY_RESTART,
		.debounce_interval	= N5Q_KEYS_DEBOUNCE_INTERVAL,
		.gpio			= N5Q_GPIO_BTN_RESET,
		.active_low		= 1,
	},
};

static void __init n5q_setup(void)
{
	u8 *art = (u8 *) KSEG1ADDR(0x1f070000);

	ath79_register_m25p80(NULL);

	ath79_register_mdio(1, 0x0);

	ath79_switch_data.phy4_mii_en = 1;
	ath79_switch_data.phy_poll_mask = 0xf7;

	/* LAN */
	ath79_eth1_data.duplex = DUPLEX_FULL;
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_GMII;
	ath79_init_mac(ath79_eth1_data.mac_addr, art + N5Q_MAC1_OFFSET, 0);
	ath79_register_eth(1);

	/* WAN */
	ath79_eth0_data.duplex = DUPLEX_FULL;
	ath79_eth0_data.mii_bus_dev = &ath79_mdio1_device.dev;
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ath79_eth0_data.phy_mask = BIT(4);
	ath79_eth0_data.speed = SPEED_100;
	ath79_init_mac(ath79_eth0_data.mac_addr, art + N5Q_MAC0_OFFSET, 0);
	ath79_register_eth(0);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(n5q_leds_gpio),
				 n5q_leds_gpio);

	ath79_register_gpio_keys_polled(-1, N5Q_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(n5q_gpio_keys),
					n5q_gpio_keys);

	gpio_request_one(N5Q_GPIO_WDT_IN,
			 GPIOF_OUT_INIT_LOW | GPIOF_EXPORT_DIR_FIXED,
			 "WDT input");

	gpio_request_one(N5Q_GPIO_WDT_EN,
			 GPIOF_OUT_INIT_LOW | GPIOF_EXPORT_DIR_FIXED,
			 "WDT enable");

	ath79_register_wmac(art + N5Q_WMAC_CALDATA_OFFSET, NULL);
}

MIPS_MACHINE(ATH79_MACH_N5Q, "N5Q", "ALFA Network N5Q", n5q_setup);

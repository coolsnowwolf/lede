/*
 * ALFA Network AP91-5G board support
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
#include "dev-ap9x-pci.h"
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "machtypes.h"

#define AP91_5G_GPIO_LED_LAN		17
#define AP91_5G_GPIO_LED_SIGNAL1	12
#define AP91_5G_GPIO_LED_SIGNAL2	8
#define AP91_5G_GPIO_LED_SIGNAL3	6
#define AP91_5G_GPIO_LED_SIGNAL4	7

#define AP91_5G_GPIO_WDT_EN		1
#define AP91_5G_GPIO_WDT_IN		0

#define AP91_5G_GPIO_BTN_RESET		11

#define AP91_5G_KEYS_POLL_INTERVAL	20
#define AP91_5G_KEYS_DEBOUNCE_INTERVAL	(3 * AP91_5G_KEYS_POLL_INTERVAL)

#define AP91_5G_WMAC_CALDATA_OFFSET	0x1000

static struct gpio_led ap91_5g_leds_gpio[] __initdata = {
	{
		.name		= "ap91-5g:green:lan",
		.gpio		= AP91_5G_GPIO_LED_LAN,
		.active_low	= 1,
	}, {
		.name		= "ap91-5g:red:signal1",
		.gpio		= AP91_5G_GPIO_LED_SIGNAL1,
		.active_low	= 1,
	}, {
		.name		= "ap91-5g:orange:signal2",
		.gpio		= AP91_5G_GPIO_LED_SIGNAL2,
		.active_low	= 1,
	}, {
		.name		= "ap91-5g:green:signal3",
		.gpio		= AP91_5G_GPIO_LED_SIGNAL3,
		.active_low	= 1,
	}, {
		.name		= "ap91-5g:green:signal4",
		.gpio		= AP91_5G_GPIO_LED_SIGNAL4,
		.active_low	= 1,
	},
};

static struct gpio_keys_button ap91_5g_gpio_keys[] __initdata = {
	{
		.desc			= "reset",
		.type			= EV_KEY,
		.code			= KEY_RESTART,
		.debounce_interval	= AP91_5G_KEYS_DEBOUNCE_INTERVAL,
		.gpio			= AP91_5G_GPIO_BTN_RESET,
		.active_low		= 1,
	},
};

static void __init ap91_5g_setup(void)
{
	u8 *art = (u8 *) KSEG1ADDR(0x1fff0000);

	ath79_gpio_function_setup(AR724X_GPIO_FUNC_JTAG_DISABLE,
				  AR724X_GPIO_FUNC_ETH_SWITCH_LED4_EN);

	gpio_set_value(AP91_5G_GPIO_LED_LAN, 1);
	gpio_set_value(AP91_5G_GPIO_LED_SIGNAL3, 1);
	gpio_set_value(AP91_5G_GPIO_LED_SIGNAL4, 1);

	ath79_register_m25p80(NULL);

	ath79_register_mdio(0, 0x0);

	/* LAN */
	ath79_eth0_data.duplex = DUPLEX_FULL;
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ath79_eth0_data.phy_mask = BIT(4);
	ath79_eth0_data.speed = SPEED_100;
	ath79_init_mac(ath79_eth0_data.mac_addr, art, 0);
	ath79_register_eth(0);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(ap91_5g_leds_gpio),
				 ap91_5g_leds_gpio);

	ath79_register_gpio_keys_polled(-1, AP91_5G_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(ap91_5g_gpio_keys),
					ap91_5g_gpio_keys);

	gpio_request_one(AP91_5G_GPIO_WDT_IN,
			 GPIOF_OUT_INIT_LOW | GPIOF_EXPORT_DIR_FIXED,
			 "WDT input");

	gpio_request_one(AP91_5G_GPIO_WDT_EN,
			 GPIOF_OUT_INIT_LOW | GPIOF_EXPORT_DIR_FIXED,
			 "WDT enable");

	ap91_pci_init(art + AP91_5G_WMAC_CALDATA_OFFSET, NULL);
}

MIPS_MACHINE(ATH79_MACH_AP91_5G, "AP91-5G", "ALFA Network AP91-5G",
	     ap91_5g_setup);

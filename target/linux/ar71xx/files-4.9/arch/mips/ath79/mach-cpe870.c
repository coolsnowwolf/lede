/*
 * YunCore CPE870 board support
 *
 * Copyright (C) 2016 Piotr Dymacz <pepe2k@gmail.com>
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
#include "dev-usb.h"
#include "dev-wmac.h"
#include "machtypes.h"

#define CPE870_GPIO_LED_LINK1	0
#define CPE870_GPIO_LED_LINK2	1
#define CPE870_GPIO_LED_LINK3	2
#define CPE870_GPIO_LED_LINK4	3
#define CPE870_GPIO_LED_WLAN	13
#define CPE870_GPIO_LED_WAN	19
#define CPE870_GPIO_LED_LAN	20

#define CPE870_GPIO_BTN_RESET	16

#define CPE870_KEYS_POLL_INTERVAL	20
#define CPE870_KEYS_DEBOUNCE_INTERVAL	(3 * CPE870_KEYS_POLL_INTERVAL)

static struct gpio_led cpe870_leds_gpio[] __initdata = {
	{
		.name		= "cpe870:green:lan",
		.gpio		= CPE870_GPIO_LED_LAN,
		.active_low	= 1,
	},
	{
		.name		= "cpe870:green:wan",
		.gpio		= CPE870_GPIO_LED_WAN,
		.active_low	= 1,
	},
	{
		.name		= "cpe870:green:wlan",
		.gpio		= CPE870_GPIO_LED_WLAN,
		.active_low	= 1,
	},
	{
		.name		= "cpe870:green:link1",
		.gpio		= CPE870_GPIO_LED_LINK1,
		.active_low	= 1,
	},
	{
		.name		= "cpe870:green:link2",
		.gpio		= CPE870_GPIO_LED_LINK2,
		.active_low	= 1,
	},
	{
		.name		= "cpe870:green:link3",
		.gpio		= CPE870_GPIO_LED_LINK3,
		.active_low	= 1,
	},
	{
		.name		= "cpe870:green:link4",
		.gpio		= CPE870_GPIO_LED_LINK4,
		.active_low	= 1,
	},
};

static struct gpio_keys_button cpe870_gpio_keys[] __initdata = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = CPE870_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= CPE870_GPIO_BTN_RESET,
		.active_low	= 1,
	},
};

static void __init cpe870_gpio_setup(void)
{
	/* Disable JTAG (enables GPIO0-3) */
	ath79_gpio_function_enable(AR934X_GPIO_FUNC_JTAG_DISABLE);

	ath79_gpio_direction_select(CPE870_GPIO_LED_LINK1, true);
	ath79_gpio_direction_select(CPE870_GPIO_LED_LINK2, true);
	ath79_gpio_direction_select(CPE870_GPIO_LED_LINK3, true);
	ath79_gpio_direction_select(CPE870_GPIO_LED_LINK4, true);

	/* Mute LEDs on boot */
	gpio_set_value(CPE870_GPIO_LED_LAN, 1);
	gpio_set_value(CPE870_GPIO_LED_WAN, 1);
	gpio_set_value(CPE870_GPIO_LED_LINK1, 1);
	gpio_set_value(CPE870_GPIO_LED_LINK2, 1);
	gpio_set_value(CPE870_GPIO_LED_LINK3, 1);
	gpio_set_value(CPE870_GPIO_LED_LINK4, 1);

	ath79_gpio_output_select(CPE870_GPIO_LED_LINK1, 0);
	ath79_gpio_output_select(CPE870_GPIO_LED_LINK2, 0);
	ath79_gpio_output_select(CPE870_GPIO_LED_LINK3, 0);
	ath79_gpio_output_select(CPE870_GPIO_LED_LINK4, 0);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(cpe870_leds_gpio),
				 cpe870_leds_gpio);

	ath79_register_gpio_keys_polled(-1, CPE870_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(cpe870_gpio_keys),
					cpe870_gpio_keys);
}

static void __init cpe870_setup(void)
{
	u8 *art = (u8 *) KSEG1ADDR(0x1fff1000);
	u8 *mac = (u8 *) KSEG1ADDR(0x1fff0000);

	ath79_register_m25p80(NULL);

	cpe870_gpio_setup();

	ath79_register_mdio(1, 0x0);

	ath79_switch_data.phy4_mii_en = 1;
	ath79_switch_data.phy_poll_mask = BIT(4);

	/* LAN */
	ath79_eth1_data.duplex = DUPLEX_FULL;
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_GMII;
	ath79_eth1_data.speed = SPEED_1000;
	ath79_init_mac(ath79_eth1_data.mac_addr, mac, 1);
	ath79_register_eth(1);

	/* WAN */
	ath79_eth0_data.duplex = DUPLEX_FULL;
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ath79_eth0_data.speed = SPEED_100;
	ath79_eth0_data.phy_mask = BIT(4);
	ath79_eth0_data.mii_bus_dev = &ath79_mdio1_device.dev;
	ath79_init_mac(ath79_eth0_data.mac_addr, mac, 0);
	ath79_register_eth(0);

	ath79_register_wmac(art, NULL);
}

MIPS_MACHINE(ATH79_MACH_CPE870, "CPE870", "YunCore CPE870", cpe870_setup);

/*
 * Support for YunCore boards:
 * - AP80Q/AP90Q
 * - CPE830
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

/* AP90Q */
#define AP90Q_GPIO_LED_WAN	4
#define AP90Q_GPIO_LED_WLAN	12
#define AP90Q_GPIO_LED_LAN	16

#define AP90Q_GPIO_BTN_RESET	17

#define AP90Q_KEYS_POLL_INTERVAL	20
#define AP90Q_KEYS_DEBOUNCE_INTERVAL	(3 * AP90Q_KEYS_POLL_INTERVAL)

static struct gpio_led ap90q_leds_gpio[] __initdata = {
	{
		.name		= "ap90q:green:lan",
		.gpio		= AP90Q_GPIO_LED_LAN,
		.active_low	= 1,
	},
	{
		.name		= "ap90q:green:wan",
		.gpio		= AP90Q_GPIO_LED_WAN,
		.active_low	= 1,
	},
	{
		.name		= "ap90q:green:wlan",
		.gpio		= AP90Q_GPIO_LED_WLAN,
		.active_low	= 1,
	},
};

static struct gpio_keys_button ap90q_gpio_keys[] __initdata = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = AP90Q_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= AP90Q_GPIO_BTN_RESET,
		.active_low	= 1,
	},
};

/* CPE830 */
#define CPE830_GPIO_LED_LINK4	0
#define CPE830_GPIO_LED_LINK1	1
#define CPE830_GPIO_LED_LINK2	2
#define CPE830_GPIO_LED_LINK3	3
#define CPE830_GPIO_LED_WAN	4
#define CPE830_GPIO_LED_WLAN	12
#define CPE830_GPIO_LED_LAN	16

#define CPE830_GPIO_BTN_RESET	17

static struct gpio_led cpe830_leds_gpio[] __initdata = {
	{
		.name		= "cpe830:green:lan",
		.gpio		= CPE830_GPIO_LED_LAN,
		.active_low	= 1,
	},
	{
		.name		= "cpe830:green:wan",
		.gpio		= CPE830_GPIO_LED_WAN,
		.active_low	= 1,
	},
	{
		.name		= "cpe830:green:wlan",
		.gpio		= CPE830_GPIO_LED_WLAN,
		.active_low	= 1,
	},
	{
		.name		= "cpe830:green:link1",
		.gpio		= CPE830_GPIO_LED_LINK1,
		.active_low	= 1,
	},
	{
		.name		= "cpe830:green:link2",
		.gpio		= CPE830_GPIO_LED_LINK2,
		.active_low	= 1,
	},
	{
		.name		= "cpe830:green:link3",
		.gpio		= CPE830_GPIO_LED_LINK3,
		.active_low	= 1,
	},
	{
		.name		= "cpe830:green:link4",
		.gpio		= CPE830_GPIO_LED_LINK4,
		.active_low	= 1,
	},
};

static void __init ap90q_cpe830_common_setup(void)
{
	u8 *art = (u8 *) KSEG1ADDR(0x1fff1000);
	u8 *mac = (u8 *) KSEG1ADDR(0x1fff0000);

	ath79_register_m25p80(NULL);

	ath79_setup_ar933x_phy4_switch(false, false);

	ath79_register_mdio(0, 0x0);

	ath79_switch_data.phy4_mii_en = 1;
	ath79_switch_data.phy_poll_mask |= BIT(4);

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
	ath79_init_mac(ath79_eth0_data.mac_addr, mac, 0);
	ath79_register_eth(0);

	ath79_register_wmac(art, NULL);

	/* For LED on GPIO4 */
	ath79_gpio_function_disable(AR934X_GPIO_FUNC_CLK_OBS4_EN);

	ath79_gpio_direction_select(AP90Q_GPIO_LED_LAN, true);
	ath79_gpio_direction_select(AP90Q_GPIO_LED_WAN, true);
	ath79_gpio_direction_select(AP90Q_GPIO_LED_WLAN, true);

	/* Mute LEDs on boot */
	gpio_set_value(AP90Q_GPIO_LED_LAN, 1);
	gpio_set_value(AP90Q_GPIO_LED_WAN, 1);

	ath79_gpio_output_select(AP90Q_GPIO_LED_LAN, 0);
	ath79_gpio_output_select(AP90Q_GPIO_LED_WAN, 0);
	ath79_gpio_output_select(AP90Q_GPIO_LED_WLAN, 0);

	ath79_register_gpio_keys_polled(-1, AP90Q_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(ap90q_gpio_keys),
					ap90q_gpio_keys);
}

static void __init ap90q_setup(void)
{
	ap90q_cpe830_common_setup();

	ath79_register_leds_gpio(-1, ARRAY_SIZE(ap90q_leds_gpio),
				 ap90q_leds_gpio);
}

MIPS_MACHINE(ATH79_MACH_AP90Q, "AP90Q", "YunCore AP80Q/AP90Q", ap90q_setup);

static void __init cpe830_setup(void)
{
	ap90q_cpe830_common_setup();

	ath79_gpio_direction_select(CPE830_GPIO_LED_LINK1, true);
	ath79_gpio_direction_select(CPE830_GPIO_LED_LINK2, true);
	ath79_gpio_direction_select(CPE830_GPIO_LED_LINK3, true);
	ath79_gpio_direction_select(CPE830_GPIO_LED_LINK4, true);

	/* Mute LEDs on boot */
	gpio_set_value(CPE830_GPIO_LED_LINK1, 1);
	gpio_set_value(CPE830_GPIO_LED_LINK2, 1);
	gpio_set_value(CPE830_GPIO_LED_LINK3, 1);
	gpio_set_value(CPE830_GPIO_LED_LINK4, 1);

	ath79_gpio_output_select(CPE830_GPIO_LED_LINK1, 0);
	ath79_gpio_output_select(CPE830_GPIO_LED_LINK2, 0);
	ath79_gpio_output_select(CPE830_GPIO_LED_LINK3, 0);
	ath79_gpio_output_select(CPE830_GPIO_LED_LINK4, 0);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(cpe830_leds_gpio),
				 cpe830_leds_gpio);
}

MIPS_MACHINE(ATH79_MACH_CPE830, "CPE830", "YunCore CPE830", cpe830_setup);

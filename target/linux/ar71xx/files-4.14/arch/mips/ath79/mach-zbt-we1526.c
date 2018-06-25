/*
 * Zbtlink ZBT-WE1526 board support
 *
 * Copyright (C) 2016 Piotr Dymacz <pepe2k@gmail.com>
 *
 * Based on mach-dr531.c and mach-tl-wr841n-v9.c
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

#define ZBT_WE1526_GPIO_LED_STATUS	13
#define ZBT_WE1526_GPIO_LED_LAN1	16
#define ZBT_WE1526_GPIO_LED_LAN2	15
#define ZBT_WE1526_GPIO_LED_LAN3	14
#define ZBT_WE1526_GPIO_LED_LAN4	11
#define ZBT_WE1526_GPIO_LED_WAN		4
#define ZBT_WE1526_GPIO_LED_WLAN	12

#define ZBT_WE1526_GPIO_BTN_RESET	17

#define ZBT_WE1526_KEYS_POLL_INTERVAL	20	/* msecs */
#define ZBT_WE1526_KEYS_DEBOUNCE_INTERVAL	\
	(3 * ZBT_WE1526_KEYS_POLL_INTERVAL)

#define ZBT_WE1526_MAC0_OFFSET		0x0
#define ZBT_WE1526_MAC1_OFFSET		0x6
#define ZBT_WE1526_WMAC_CALDATA_OFFSET	0x1000

static struct gpio_led zbt_we1526_leds_gpio[] __initdata = {
	{
		.name		= "zbt-we1526:green:status",
		.gpio		= ZBT_WE1526_GPIO_LED_STATUS,
		.active_low	= 1,
	},
	{
		.name		= "zbt-we1526:green:lan1",
		.gpio		= ZBT_WE1526_GPIO_LED_LAN1,
		.active_low	= 1,
	},
	{
		.name		= "zbt-we1526:green:lan2",
		.gpio		= ZBT_WE1526_GPIO_LED_LAN2,
		.active_low	= 1,
	},
	{
		.name		= "zbt-we1526:green:lan3",
		.gpio		= ZBT_WE1526_GPIO_LED_LAN3,
		.active_low	= 1,
	},
	{
		.name		= "zbt-we1526:green:lan4",
		.gpio		= ZBT_WE1526_GPIO_LED_LAN4,
		.active_low	= 1,
	},
	{
		.name		= "zbt-we1526:green:wan",
		.gpio		= ZBT_WE1526_GPIO_LED_WAN,
		.active_low	= 1,
	},
	{
		.name		= "zbt-we1526:green:wlan",
		.gpio		= ZBT_WE1526_GPIO_LED_WLAN,
		.active_low	= 1,
	},
};

static struct gpio_keys_button zbt_we1526_gpio_keys[] __initdata = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = ZBT_WE1526_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= ZBT_WE1526_GPIO_BTN_RESET,
		.active_low	= 1,
	},
};

static void __init zbt_we1526_gpio_setup(void)
{
	/* For LED on GPIO4 */
	ath79_gpio_function_disable(AR934X_GPIO_FUNC_CLK_OBS4_EN);
	ath79_gpio_output_select(ZBT_WE1526_GPIO_LED_WAN, 0);

	ath79_gpio_direction_select(ZBT_WE1526_GPIO_LED_STATUS, true);
	ath79_gpio_direction_select(ZBT_WE1526_GPIO_LED_LAN1, true);
	ath79_gpio_direction_select(ZBT_WE1526_GPIO_LED_LAN2, true);
	ath79_gpio_direction_select(ZBT_WE1526_GPIO_LED_LAN3, true);
	ath79_gpio_direction_select(ZBT_WE1526_GPIO_LED_LAN4, true);
	ath79_gpio_direction_select(ZBT_WE1526_GPIO_LED_WAN, true);
	ath79_gpio_direction_select(ZBT_WE1526_GPIO_LED_WLAN, true);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(zbt_we1526_leds_gpio),
				 zbt_we1526_leds_gpio);

	ath79_register_gpio_keys_polled(-1, ZBT_WE1526_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(zbt_we1526_gpio_keys),
					zbt_we1526_gpio_keys);
}

static void __init zbt_we1526_setup(void)
{
	u8 *art = (u8 *) KSEG1ADDR(0x1fff0000);

	ath79_register_m25p80(NULL);

	zbt_we1526_gpio_setup();

	ath79_setup_ar933x_phy4_switch(false, false);

	ath79_register_mdio(0, 0x0);

	/* LAN */
	ath79_eth1_data.duplex = DUPLEX_FULL;
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_GMII;
	ath79_switch_data.phy_poll_mask |= BIT(4);
	ath79_init_mac(ath79_eth1_data.mac_addr,
		       art + ZBT_WE1526_MAC0_OFFSET, 0);
	ath79_register_eth(1);

	/* WAN */
	ath79_switch_data.phy4_mii_en = 1;
	ath79_eth0_data.duplex = DUPLEX_FULL;
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ath79_eth0_data.phy_mask = BIT(4);
	ath79_eth0_data.speed = SPEED_100;
	ath79_init_mac(ath79_eth0_data.mac_addr,
		       art + ZBT_WE1526_MAC1_OFFSET, 0);
	ath79_register_eth(0);

	ath79_register_wmac(art + ZBT_WE1526_WMAC_CALDATA_OFFSET, NULL);

	ath79_register_usb();
}

MIPS_MACHINE(ATH79_MACH_ZBT_WE1526, "ZBT-WE1526", "Zbtlink ZBT-WE1526",
	     zbt_we1526_setup);

/*
 * ALFA Network R36A board support
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
#include "dev-usb.h"
#include "dev-wmac.h"
#include "machtypes.h"

#define R36A_GPIO_LED_LAN	4
#define R36A_GPIO_LED_STATUS	14
#define R36A_GPIO_LED_USB	12
#define R36A_GPIO_LED_WAN	16
#define R36A_GPIO_LED_WLAN	15

#define R36A_GPIO_WDT_EN	1
#define R36A_GPIO_WDT_IN	0
#define R36A_GPIO_USB_PWR	3

#define R36A_GPIO_BTN_RESET	2
#define R36A_GPIO_BTN_RFKILL	17

#define R36A_KEYS_POLL_INTERVAL		20
#define R36A_KEYS_DEBOUNCE_INTERVAL	(3 * R36A_KEYS_POLL_INTERVAL)

#define R36A_WMAC_CALDATA_OFFSET	0x1000

static struct gpio_led r36a_leds_gpio[] __initdata = {
	{
		.name		= "r36a:blue:lan",
		.gpio		= R36A_GPIO_LED_LAN,
		.active_low	= 1,
	}, {
		.name		= "r36a:blue:status",
		.gpio		= R36A_GPIO_LED_STATUS,
		.active_low	= 1,
	}, {
		.name		= "r36a:blue:usb",
		.gpio		= R36A_GPIO_LED_USB,
		.active_low	= 1,
	}, {
		.name		= "r36a:blue:wan",
		.gpio		= R36A_GPIO_LED_WAN,
		.active_low	= 1,
	}, {
		.name		= "r36a:blue:wlan",
		.gpio		= R36A_GPIO_LED_WLAN,
		.active_low	= 1,
	},
};

static struct gpio_keys_button r36a_gpio_keys[] __initdata = {
	{
		.desc			= "reset",
		.type			= EV_KEY,
		.code			= KEY_RESTART,
		.debounce_interval	= R36A_KEYS_DEBOUNCE_INTERVAL,
		.gpio			= R36A_GPIO_BTN_RESET,
		.active_low		= 1,
	}, {
		.desc			= "rfkill",
		.type			= EV_KEY,
		.code			= KEY_RFKILL,
		.debounce_interval	= R36A_KEYS_DEBOUNCE_INTERVAL,
		.gpio			= R36A_GPIO_BTN_RFKILL,
		.active_low		= 1,
	},
};

static void __init r36a_setup(void)
{
	u8 *art = (u8 *) KSEG1ADDR(0x1f070000);

	ath79_register_m25p80(NULL);

	ath79_setup_ar933x_phy4_switch(false, false);

	ath79_register_mdio(0, 0x0);

	ath79_switch_data.phy4_mii_en = 1;
	ath79_switch_data.phy_poll_mask = 0xf7;

	/* LAN */
	ath79_eth0_data.duplex = DUPLEX_FULL;
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ath79_eth0_data.phy_mask = BIT(4);
	ath79_eth0_data.speed = SPEED_100;
	ath79_init_mac(ath79_eth0_data.mac_addr, art, 0);
	ath79_register_eth(0);

	/* WAN */
	ath79_eth1_data.duplex = DUPLEX_FULL;
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_GMII;
	ath79_init_mac(ath79_eth1_data.mac_addr, art, 1);
	ath79_register_eth(1);

	/* Disable JTAG (enables GPIO0-3) */
	ath79_gpio_function_enable(AR934X_GPIO_FUNC_JTAG_DISABLE);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(r36a_leds_gpio),
				 r36a_leds_gpio);

	ath79_register_gpio_keys_polled(-1, R36A_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(r36a_gpio_keys),
					r36a_gpio_keys);

	gpio_request_one(R36A_GPIO_WDT_IN,
			 GPIOF_OUT_INIT_LOW | GPIOF_EXPORT_DIR_FIXED,
			 "WDT input");

	gpio_request_one(R36A_GPIO_WDT_EN,
			 GPIOF_OUT_INIT_LOW | GPIOF_EXPORT_DIR_FIXED,
			 "WDT enable");

	gpio_request_one(R36A_GPIO_USB_PWR,
			 GPIOF_OUT_INIT_HIGH | GPIOF_EXPORT_DIR_FIXED,
			 "USB power");

	ath79_register_wmac(art + R36A_WMAC_CALDATA_OFFSET, NULL);

	ath79_register_usb();
}

MIPS_MACHINE(ATH79_MACH_R36A, "R36A", "ALFA Network R36A", r36a_setup);

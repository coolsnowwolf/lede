/*
 * ALFA Network AP121F board support
 *
 * Copyright (C) 2017 Piotr Dymacz <pepe2k@gmail.com>
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

#define AP121F_GPIO_LED_LAN	17
#define AP121F_GPIO_LED_VPN	27
#define AP121F_GPIO_LED_WLAN	0

#define AP121F_GPIO_MICROSD_EN	26

#define AP121F_GPIO_BTN_RESET	12
#define AP121F_GPIO_BTN_SWITCH	21

#define AP121F_KEYS_POLL_INTERVAL	20
#define AP121F_KEYS_DEBOUNCE_INTERVAL	(3 * AP121F_KEYS_POLL_INTERVAL)

#define AP121F_WMAC_CALDATA_OFFSET	0x1000

static struct gpio_led ap121f_leds_gpio[] __initdata = {
	{
		.name		= "ap121f:green:lan",
		.gpio		= AP121F_GPIO_LED_LAN,
		.active_low	= 1,
	}, {
		.name		= "ap121f:green:vpn",
		.gpio		= AP121F_GPIO_LED_VPN,
		.active_low	= 1,
	}, {
		.name		= "ap121f:green:wlan",
		.gpio		= AP121F_GPIO_LED_WLAN,
		.active_low	= 0,
	},
};

static struct gpio_keys_button ap121f_gpio_keys[] __initdata = {
	{
		.desc			= "reset",
		.type			= EV_KEY,
		.code			= KEY_RESTART,
		.debounce_interval	= AP121F_KEYS_DEBOUNCE_INTERVAL,
		.gpio			= AP121F_GPIO_BTN_RESET,
		.active_low		= 1,
	}, {
		.desc			= "switch",
		.type			= EV_KEY,
		.code			= BTN_0,
		.debounce_interval	= AP121F_KEYS_DEBOUNCE_INTERVAL,
		.gpio			= AP121F_GPIO_BTN_SWITCH,
		.active_low		= 0,
	},
};

static void __init ap121f_setup(void)
{
	u8 *art = (u8 *) KSEG1ADDR(0x1f040000);

	ath79_register_m25p80(NULL);

	ath79_setup_ar933x_phy4_switch(false, false);

	/* LAN */
	ath79_register_mdio(0, 0x0);
	ath79_init_mac(ath79_eth0_data.mac_addr, art, 0);
	ath79_register_eth(0);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(ap121f_leds_gpio),
				 ap121f_leds_gpio);

	ath79_register_gpio_keys_polled(-1, AP121F_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(ap121f_gpio_keys),
					ap121f_gpio_keys);

	gpio_request_one(AP121F_GPIO_MICROSD_EN,
			 GPIOF_OUT_INIT_HIGH | GPIOF_EXPORT_DIR_FIXED,
			 "microSD enable");

	ath79_register_wmac(art + AP121F_WMAC_CALDATA_OFFSET, NULL);

	ath79_register_usb();
}

MIPS_MACHINE(ATH79_MACH_AP121F, "AP121F", "ALFA Network AP121F", ap121f_setup);

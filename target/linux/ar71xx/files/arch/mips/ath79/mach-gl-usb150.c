/*
 * GL.iNet GL-USB150 board support
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
#include "dev-wmac.h"
#include "machtypes.h"

#define GL_USB150_GPIO_LED_POWER	13
#define GL_USB150_GPIO_LED_WLAN		0
#define GL_USB150_GPIO_LAN_RESET	7
#define GL_USB150_GPIO_BTN_RESET	11

#define GL_USB150_KEYS_POLL_INTERVAL		20
#define GL_USB150_KEYS_DEBOUNCE_INTERVAL	\
					(3 * GL_USB150_KEYS_POLL_INTERVAL)

#define GL_USB150_WMAC_CALDATA_OFFSET	0x1000

static struct gpio_led gl_usb150_leds_gpio[] __initdata = {
	{
		.name		= "gl-usb150:green:power",
		.gpio		= GL_USB150_GPIO_LED_POWER,
		.active_low	= 0,
	}, {
		.name		= "gl-usb150:green:wlan",
		.gpio		= GL_USB150_GPIO_LED_WLAN,
		.active_low	= 0,
	},
};

static struct gpio_keys_button gl_usb150_gpio_keys[] __initdata = {
	{
		.desc			= "reset",
		.type			= EV_KEY,
		.code			= KEY_RESTART,
		.debounce_interval	= GL_USB150_KEYS_DEBOUNCE_INTERVAL,
		.gpio			= GL_USB150_GPIO_BTN_RESET,
		.active_low		= 0,
	},
};

static void __init gl_usb150_setup(void)
{
	u8 *art = (u8 *) KSEG1ADDR(0x1fff0000);

	ath79_register_m25p80(NULL);

	ath79_setup_ar933x_phy4_switch(false, false);

	/* LAN (PHY4 connected with Realtek RTL8152B) */
	ath79_init_mac(ath79_eth0_data.mac_addr, art, 0);
	ath79_register_mdio(0, 0x0);
	ath79_register_eth(0);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(gl_usb150_leds_gpio),
				 gl_usb150_leds_gpio);

	ath79_register_gpio_keys_polled(-1, GL_USB150_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(gl_usb150_gpio_keys),
					gl_usb150_gpio_keys);

	gpio_request_one(GL_USB150_GPIO_LAN_RESET,
			 GPIOF_OUT_INIT_LOW | GPIOF_EXPORT_DIR_FIXED,
			 "LAN reset");

	ath79_register_wmac(art + GL_USB150_WMAC_CALDATA_OFFSET, NULL);
}

MIPS_MACHINE(ATH79_MACH_GL_USB150, "GL-USB150", "GL.iNet GL-USB150",
	     gl_usb150_setup);

/*
 * TP-Link TL-WR902AC v1 board support
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
#include "dev-ap9x-pci.h"
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-usb.h"
#include "dev-wmac.h"
#include "machtypes.h"


#define TL_WR902AC_V1_GPIO_LED_INTERNET	12
#define TL_WR902AC_V1_GPIO_LED_LAN	15
#define TL_WR902AC_V1_GPIO_LED_POWER	13
#define TL_WR902AC_V1_GPIO_LED_USB	4
#define TL_WR902AC_V1_GPIO_LED_WLAN2G	11
#define TL_WR902AC_V1_GPIO_LED_WPS	0

#define TL_WR902AC_V1_GPIO_BTN_RESET	3
#define TL_WR902AC_V1_GPIO_BTN_SW1	17
#define TL_WR902AC_V1_GPIO_BTN_SW2	14
#define TL_WR902AC_V1_GPIO_BTN_WPS	2

#define TL_WR902AC_V1_GPIO_USB_POWER	1

#define TL_WR902AC_V1_KEYS_POLL_INTERVAL	20 /* msecs */
#define TL_WR902AC_V1_KEYS_DEBOUNCE_INTERVAL	\
	(3 * TL_WR902AC_V1_KEYS_POLL_INTERVAL)

static struct gpio_led tl_wr902ac_v1_leds_gpio[] __initdata = {
	{
		.name		= "tl-wr902ac-v1:green:internet",
		.gpio		= TL_WR902AC_V1_GPIO_LED_INTERNET,
		.active_low	= 1,
	}, {
		.name		= "tl-wr902ac-v1:green:lan",
		.gpio		= TL_WR902AC_V1_GPIO_LED_LAN,
		.active_low	= 0,
	}, {
		.name		= "tl-wr902ac-v1:green:power",
		.gpio		= TL_WR902AC_V1_GPIO_LED_POWER,
		.active_low	= 0,
	}, {
		.name		= "tl-wr902ac-v1:green:usb",
		.gpio		= TL_WR902AC_V1_GPIO_LED_USB,
		.active_low	= 0,
	}, {
		.name		= "tl-wr902ac-v1:green:wlan2g",
		.gpio		= TL_WR902AC_V1_GPIO_LED_WLAN2G,
		.active_low	= 1,
	}, {
		.name		= "tl-wr902ac-v1:green:wps",
		.gpio		= TL_WR902AC_V1_GPIO_LED_WPS,
		.active_low	= 1,
	}
};

static struct gpio_keys_button tl_wr902ac_v1_gpio_keys[] __initdata = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = TL_WR902AC_V1_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= TL_WR902AC_V1_GPIO_BTN_RESET,
		.active_low	= 1,
	}, {
		.desc		= "sw1",
		.type		= EV_KEY,
		.code		= BTN_0,
		.debounce_interval = TL_WR902AC_V1_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= TL_WR902AC_V1_GPIO_BTN_SW1,
		.active_low	= 1,
	}, {
		.desc		= "sw2",
		.type		= EV_KEY,
		.code		= BTN_1,
		.debounce_interval = TL_WR902AC_V1_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= TL_WR902AC_V1_GPIO_BTN_SW2,
		.active_low	= 1,
	}, {
		.desc		= "wps",
		.type		= EV_KEY,
		.code		= KEY_WPS_BUTTON,
		.debounce_interval = TL_WR902AC_V1_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= TL_WR902AC_V1_GPIO_BTN_WPS,
		.active_low	= 1,
	}
};

static void __init tl_wr902ac_v1_setup(void)
{
	u8 *art = (u8 *) KSEG1ADDR(0x1f7f0000);
	u8 *mac = (u8 *) KSEG1ADDR(0x1f750008);

	ath79_register_m25p80(NULL);

	ath79_setup_ar933x_phy4_switch(false, false);
	ath79_register_mdio(0, 0x0);

	/* LAN */
	ath79_switch_data.phy4_mii_en = 1;
	ath79_eth0_data.duplex = DUPLEX_FULL;
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ath79_eth0_data.phy_mask = BIT(4);
	ath79_eth0_data.speed = SPEED_100;

	ath79_init_mac(ath79_eth0_data.mac_addr, mac, 1);
	ath79_register_eth(0);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(tl_wr902ac_v1_leds_gpio),
				 tl_wr902ac_v1_leds_gpio);

	ath79_register_gpio_keys_polled(-1, TL_WR902AC_V1_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(tl_wr902ac_v1_gpio_keys),
					tl_wr902ac_v1_gpio_keys);

	gpio_request_one(TL_WR902AC_V1_GPIO_USB_POWER,
			 GPIOF_OUT_INIT_HIGH | GPIOF_EXPORT_DIR_FIXED,
			 "USB power");

	ath79_register_usb();

	ath79_register_wmac(art + 0x1000, mac);

	ap91_pci_init(art + 0x5000, NULL);
}

MIPS_MACHINE(ATH79_MACH_TL_WR902AC_V1, "TL-WR902AC-V1", "TP-LINK TL-WR902AC v1",
	     tl_wr902ac_v1_setup);

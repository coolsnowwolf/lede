/*
 *  devolo dLAN Hotspot board support
 *
 *  Copyright (C) 2015 Torsten Schnuis <torsten.schnuis@gik.de>
 *  Copyright (C) 2015 devolo AG
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/gpio.h>

#include <asm/mach-ath79/ath79.h>

#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-usb.h"
#include "dev-wmac.h"
#include "machtypes.h"

#define DLAN_HOTSPOT_GPIO_LED_WIFI                0

#define DLAN_HOTSPOT_GPIO_BTN_RESET              11
#define DLAN_HOTSPOT_GPIO_BTN_PLC_PAIRING        12
#define DLAN_HOTSPOT_GPIO_BTN_WIFI               21

#define DLAN_HOTSPOT_GPIO_PLC_POWER              22
#define DLAN_HOTSPOT_GPIO_PLC_RESET              20
#define DLAN_HOTSPOT_GPIO_PLC_DISABLE_LEDS       18

#define DLAN_HOTSPOT_KEYS_POLL_INTERVAL          20    /* msecs */
#define DLAN_HOTSPOT_KEYS_DEBOUNCE_INTERVAL      (3 * DLAN_HOTSPOT_KEYS_POLL_INTERVAL)

#define DLAN_HOTSPOT_ART_ADDRESS                 0x1fff0000
#define DLAN_HOTSPOT_CALDATA_OFFSET              0x00001000
#define DLAN_HOTSPOT_MAC_ADDRESS_OFFSET          0x00001002

static struct gpio_led dlan_hotspot_leds_gpio[] __initdata = {
	{
		.name		= "devolo:green:wifi",
		.gpio		= DLAN_HOTSPOT_GPIO_LED_WIFI,
		.active_low	= 0,
	}
};

static struct gpio_keys_button dlan_hotspot_gpio_keys[] __initdata = {
	{
		.desc		= "Reset button",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = DLAN_HOTSPOT_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= DLAN_HOTSPOT_GPIO_BTN_RESET,
		.active_low     = 0,
	},
	{
		.desc		= "Pairing button",
		.type		= EV_KEY,
		.code		= BTN_0,
		.debounce_interval = DLAN_HOTSPOT_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= DLAN_HOTSPOT_GPIO_BTN_PLC_PAIRING,
		.active_low	= 0,
	},
	{
		.desc		= "WLAN button",
		.type		= EV_KEY,
		.code		= KEY_WPS_BUTTON,
		.debounce_interval = DLAN_HOTSPOT_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= DLAN_HOTSPOT_GPIO_BTN_WIFI,
		.active_low	= 0,
	}
};

static void __init dlan_hotspot_setup(void)
{
	u8 *art = (u8 *) KSEG1ADDR(DLAN_HOTSPOT_ART_ADDRESS);
	u8 *cal = art + DLAN_HOTSPOT_CALDATA_OFFSET;
	u8 *wifi_mac = art + DLAN_HOTSPOT_MAC_ADDRESS_OFFSET;

	/* disable PHY_SWAP and PHY_ADDR_SWAP bits */
	ath79_setup_ar933x_phy4_switch(false, false);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(dlan_hotspot_leds_gpio),
				 dlan_hotspot_leds_gpio);

	ath79_register_gpio_keys_polled(-1, DLAN_HOTSPOT_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(dlan_hotspot_gpio_keys),
					dlan_hotspot_gpio_keys);

	gpio_request_one(DLAN_HOTSPOT_GPIO_PLC_POWER,
			 GPIOF_OUT_INIT_HIGH | GPIOF_EXPORT_DIR_FIXED,
			 "PLC power");
	gpio_request_one(DLAN_HOTSPOT_GPIO_PLC_RESET,
			 GPIOF_OUT_INIT_LOW | GPIOF_EXPORT_DIR_FIXED,
			 "PLC reset");
	gpio_request_one(DLAN_HOTSPOT_GPIO_PLC_DISABLE_LEDS,
			 GPIOF_OUT_INIT_LOW | GPIOF_EXPORT_DIR_FIXED,
			 "PLC LEDs");

	ath79_register_usb();

	ath79_register_m25p80(NULL);

	ath79_init_mac(ath79_eth0_data.mac_addr, wifi_mac, 1);
	ath79_init_mac(ath79_eth1_data.mac_addr, wifi_mac, 2);

	ath79_register_mdio(0, 0x0);
	ath79_register_eth(0);
	ath79_register_eth(1);

	ath79_register_wmac(cal, wifi_mac);
}

MIPS_MACHINE(ATH79_MACH_DLAN_HOTSPOT, "dLAN-Hotspot",
	     "dLAN Hotspot", dlan_hotspot_setup);

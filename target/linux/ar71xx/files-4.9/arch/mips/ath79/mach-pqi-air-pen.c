/*
 *  PQI Air Pen stick support
 *
 *  Copyright (C) 2016 YuheiOKAWA <tochiro.srchack@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
*/

#include <linux/gpio.h>

#include <asm/mach-ath79/ath79.h>

#include "common.h"
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-usb.h"
#include "dev-wmac.h"
#include "machtypes.h"

#define PQI_AIR_PEN_GPIO_LED_WLAN		0
#define PQI_AIR_PEN_GPIO_LED_WPS		23

#define PQI_AIR_PEN_GPIO_BTN_WPS		22
#define PQI_AIR_PEN_GPIO_BTN_RESET		12

#define PQI_AIR_PEN_KEYS_POLL_INTERVAL		20 /* msecs */
#define PQI_AIR_PEN_KEYS_DEBOUNCE_INTERVAL	(3 * PQI_AIR_PEN_KEYS_POLL_INTERVAL)

#define PQI_AIR_PEN_WMAC_CALDATA_OFFSET		0x1000
#define PQI_AIR_PEN_LAN_MAC_OFFSET		0x1002
#define PQI_AIR_PEN_WMAC_MAC_OFFSET		0x1002

static struct gpio_led pqi_air_pen_leds_gpio[] __initdata = {
	{
		.name = "pqi-air-pen:blue:wlan",
		.gpio = PQI_AIR_PEN_GPIO_LED_WLAN,
		.active_low = 0,
	},
	{
		.name = "pqi-air-pen:blue:wps",
		.gpio = PQI_AIR_PEN_GPIO_LED_WPS,
		.active_low = 0,
	},
};

static struct gpio_keys_button pqi_air_pen_gpio_keys[] __initdata = {
	{
		.desc = "wps",
		.type = EV_KEY,
		.code = KEY_WPS_BUTTON,
		.debounce_interval = PQI_AIR_PEN_KEYS_DEBOUNCE_INTERVAL,
		.gpio = PQI_AIR_PEN_GPIO_BTN_WPS,
		.active_low = 0,
	},
	{
		.desc = "reset",
		.type = EV_KEY,
		.code = KEY_RESTART,
		.debounce_interval = PQI_AIR_PEN_KEYS_DEBOUNCE_INTERVAL,
		.gpio = PQI_AIR_PEN_GPIO_BTN_RESET,
		.active_low = 0,
	},
};

static void __init pqi_air_pen_setup(void)
{
	/* ART base address */
	u8 *art = (u8 *) KSEG1ADDR(0x9f050000);

	/* register flash. */
	ath79_register_m25p80(NULL);

	/* register wireless mac with cal data */
	ath79_register_wmac(art + PQI_AIR_PEN_WMAC_CALDATA_OFFSET, art + PQI_AIR_PEN_WMAC_MAC_OFFSET);

	/* false PHY_SWAP and PHY_ADDR_SWAP bits */
	ath79_setup_ar933x_phy4_switch(false, false);

	/* register gpio LEDs and keys */
	ath79_register_leds_gpio(-1, ARRAY_SIZE(pqi_air_pen_leds_gpio),
				 pqi_air_pen_leds_gpio);
	ath79_register_gpio_keys_polled(-1, PQI_AIR_PEN_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(pqi_air_pen_gpio_keys),
					pqi_air_pen_gpio_keys);

	/* enable usb */
	ath79_register_usb();
	
	/* register eth0 as LAN */
	ath79_init_mac(ath79_eth0_data.mac_addr, art + PQI_AIR_PEN_LAN_MAC_OFFSET, 0);
	ath79_register_mdio(0, 0x0);
	ath79_register_eth(0);
}

MIPS_MACHINE(ATH79_MACH_PQI_AIR_PEN, "PQI-AIR-PEN", "PQI Air Pen",pqi_air_pen_setup);

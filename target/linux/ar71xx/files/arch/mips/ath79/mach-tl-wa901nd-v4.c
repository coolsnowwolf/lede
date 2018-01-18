/*
 *  TP-LINK TL-WA901ND v4, v5 board
 *
 *  Copyright (C) 2015 Matthias Schiffer <mschiffer@universe-factory.net>
 *  Copyright (C) 2016 Tiziano Bacocco <tizbac2@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
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


#define TL_WA901ND_V4_GPIO_LED_QSS		3
#define TL_WA901ND_V4_GPIO_LED_LAN		7
#define TL_WA901ND_V4_GPIO_LED_WLAN		8
#define TL_WA901ND_V4_GPIO_LED_SYSTEM		18

#define TL_WA901ND_V4_GPIO_BTN_RESET		1

#define TL_WA901ND_V4_KEYS_POLL_INTERVAL	20
#define TL_WA901ND_V4_KEYS_DEBOUNCE_INTERVAL	(3 * TL_WA901ND_V4_KEYS_POLL_INTERVAL)


static struct gpio_led TL_WA901ND_V4_leds_gpio[] __initdata = {
	{
		.name		= "tp-link:green:qss",
		.gpio		= TL_WA901ND_V4_GPIO_LED_QSS,
		.active_low	= 1,
	},
	{
		.name		= "tp-link:green:lan",
		.gpio		= TL_WA901ND_V4_GPIO_LED_LAN,
		.active_low	= 1,
	},
	{
		.name		= "tp-link:green:wlan",
		.gpio		= TL_WA901ND_V4_GPIO_LED_WLAN,
		.active_low	= 1,
	},
	{
		.name		= "tp-link:green:system",
		.gpio		= TL_WA901ND_V4_GPIO_LED_SYSTEM,
		.active_low	= 1,
	},
};

static struct gpio_keys_button TL_WA901ND_V4_gpio_keys[] __initdata = {
	{
		.desc		= "Reset button",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = TL_WA901ND_V4_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= TL_WA901ND_V4_GPIO_BTN_RESET,
		.active_low	= 1,
	}
};


static const char *tl_wa901nd_v4_part_probes[] = {
	"tp-link",
	NULL,
};

static struct flash_platform_data tl_wa901nd_v4_flash_data = {
	.part_probes	= tl_wa901nd_v4_part_probes,
};


static void __init TL_WA901ND_V4_setup(void)
{
	u8 *mac = (u8 *) KSEG1ADDR(0x1f01fc00);
	u8 *ee = (u8 *) KSEG1ADDR(0x1fff1000);

	ath79_register_m25p80(&tl_wa901nd_v4_flash_data);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(TL_WA901ND_V4_leds_gpio),
				 TL_WA901ND_V4_leds_gpio);

	ath79_register_gpio_keys_polled(-1, TL_WA901ND_V4_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(TL_WA901ND_V4_gpio_keys),
					TL_WA901ND_V4_gpio_keys);

	ath79_register_mdio(0, 0x0);

	ath79_init_mac(ath79_eth0_data.mac_addr, mac, 1);
	ath79_init_mac(ath79_eth1_data.mac_addr, mac, -1);

	ath79_switch_data.phy4_mii_en = 1;

	ath79_register_eth(0);
	ath79_register_eth(1);

	ath79_register_wmac(ee, mac);

}

MIPS_MACHINE(ATH79_MACH_TL_WA901ND_V4, "TL-WA901ND-v4", "TP-LINK TL-WA901ND v4",
	     TL_WA901ND_V4_setup);

MIPS_MACHINE(ATH79_MACH_TL_WA901ND_V5, "TL-WA901ND-v5", "TP-LINK TL-WA901ND v5",
	     TL_WA901ND_V4_setup);

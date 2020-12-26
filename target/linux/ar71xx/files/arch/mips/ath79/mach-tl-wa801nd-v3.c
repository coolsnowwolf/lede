/*
 *  TP-LINK TL-WA801ND v3 adapted from TP-LINK TL-WR841N/ND v9
 *  TP-LINK TL-WA801ND v4
 *
 *  Copyright (C) 2014 Matthias Schiffer <mschiffer@universe-factory.net>
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

#define TL_WA801NDV3_GPIO_LED_WLAN	12
#define TL_WA801NDV3_GPIO_LED_SYSTEM	13
#define TL_WA801NDV3_GPIO_LED_SECURITY_RED 11
#define TL_WA801NDV3_GPIO_LED_SECURITY_GREEN 15
#define TL_WA801NDV3_GPIO_LED_LAN 3

#define TL_WA801NDV3_GPIO_BTN_RESET	2
#define TL_WA801NDV3_GPIO_BTN_WIFI	1

#define TL_WA801NDV3_KEYS_POLL_INTERVAL	20	/* msecs */
#define TL_WA801NDV3_KEYS_DEBOUNCE_INTERVAL (3 * TL_WA801NDV3_KEYS_POLL_INTERVAL)

static const char *tl_wa801n_v3_part_probes[] = {
	"tp-link",
	NULL,
};

static struct flash_platform_data tl_wa801n_v3_flash_data = {
	.part_probes	= tl_wa801n_v3_part_probes,
};

static struct gpio_led tl_wa801n_v3_leds_gpio[] __initdata = {
  {
		.name		= "tp-link:green:system",
		.gpio		= TL_WA801NDV3_GPIO_LED_SYSTEM,
		.active_low	= 1,
	}, {
		.name		= "tp-link:green:lan",
		.gpio		= TL_WA801NDV3_GPIO_LED_LAN,
		.active_low	= 1,
	}, {
		.name		= "tp-link:green:wlan",
		.gpio		= TL_WA801NDV3_GPIO_LED_WLAN,
		.active_low	= 1,
	}, {
		.name		= "tp-link:red:security",
		.gpio		= TL_WA801NDV3_GPIO_LED_SECURITY_RED,
		.active_low	= 0,
	}, {
		.name		= "tp-link:green:security",
		.gpio		= TL_WA801NDV3_GPIO_LED_SECURITY_GREEN,
		.active_low	= 0,
	}

};

static struct gpio_keys_button tl_wa801n_v3_gpio_keys[] __initdata = {
	{
		.desc		= "Reset button",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = TL_WA801NDV3_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= TL_WA801NDV3_GPIO_BTN_RESET,
		.active_low	= 1,
	}, {
		.desc		= "WIFI button",
		.type		= EV_KEY,
		.code		= KEY_RFKILL,
		.debounce_interval = TL_WA801NDV3_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= TL_WA801NDV3_GPIO_BTN_WIFI,
		.active_low	= 1,
	}
};


static void __init tl_ap143_setup(void)
{
	u8 *mac = (u8 *) KSEG1ADDR(0x1f01fc00);
	u8 *ee = (u8 *) KSEG1ADDR(0x1fff1000);
	u8 tmpmac[ETH_ALEN];

	ath79_register_m25p80(&tl_wa801n_v3_flash_data);

	ath79_setup_ar933x_phy4_switch(false, false);

	ath79_register_mdio(0, 0x0);

	/* LAN */
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_GMII;
	ath79_eth1_data.duplex = DUPLEX_FULL;
	ath79_switch_data.phy_poll_mask |= BIT(4);
	ath79_init_mac(ath79_eth1_data.mac_addr, mac, 0);
	ath79_register_eth(1);

	/* WAN */
	ath79_switch_data.phy4_mii_en = 1;
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ath79_eth0_data.duplex = DUPLEX_FULL;
	ath79_eth0_data.speed = SPEED_100;
	ath79_eth0_data.phy_mask = BIT(4);
	ath79_init_mac(ath79_eth0_data.mac_addr, mac, 1);
	ath79_register_eth(0);

	ath79_init_mac(tmpmac, mac, 0);
	ath79_register_wmac(ee, tmpmac);
}

static void __init tl_wa801n_v3_setup(void)
{
	tl_ap143_setup();

	ath79_register_leds_gpio(-1, ARRAY_SIZE(tl_wa801n_v3_leds_gpio),
				 tl_wa801n_v3_leds_gpio);

	ath79_register_gpio_keys_polled(1, TL_WA801NDV3_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(tl_wa801n_v3_gpio_keys),
					tl_wa801n_v3_gpio_keys);
}

MIPS_MACHINE(ATH79_MACH_TL_WA801ND_V3, "TL-WA801ND-v3", "TP-LINK TL-WA801ND v3",
	     tl_wa801n_v3_setup);

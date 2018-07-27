/*
 *  TP-LINK TL-WA901N/ND v2 board support
 *
 *  Copyright (C) 2009-2012 Gabor Juhos <juhosg@openwrt.org>
 *  Copyright (C) 2010 Pieter Hollants <pieter@hollants.com>
 *  Copyright (C) 2011 Jonathan Bennett <jbscience87@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <asm/mach-ath79/ath79.h>
#include <asm/mach-ath79/ar71xx_regs.h>

#include "dev-eth.h"
#include "dev-m25p80.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-wmac.h"
#include "machtypes.h"

#define TL_WA901ND_V2_GPIO_LED_QSS		4
#define TL_WA901ND_V2_GPIO_LED_SYSTEM		2
#define TL_WA901ND_V2_GPIO_LED_WLAN		9

#define TL_WA901ND_V2_GPIO_BTN_RESET		3
#define TL_WA901ND_V2_GPIO_BTN_QSS		7

#define TL_WA901ND_V2_KEYS_POLL_INTERVAL	20	/* msecs */
#define TL_WA901ND_V2_KEYS_DEBOUNCE_INTERVAL	\
					(3 * TL_WA901ND_V2_KEYS_POLL_INTERVAL)

static const char *tl_wa901nd_v2_part_probes[] = {
	"tp-link",
	NULL,
};

static struct flash_platform_data tl_wa901nd_v2_flash_data = {
	.part_probes	= tl_wa901nd_v2_part_probes,
};

static struct gpio_led tl_wa901nd_v2_leds_gpio[] __initdata = {
	{
		.name		= "tp-link:green:system",
		.gpio		= TL_WA901ND_V2_GPIO_LED_SYSTEM,
		.active_low	= 1,
	}, {
		.name		= "tp-link:green:qss",
		.gpio		= TL_WA901ND_V2_GPIO_LED_QSS,
	}, {
		.name		= "tp-link:green:wlan",
		.gpio		= TL_WA901ND_V2_GPIO_LED_WLAN,
		.active_low	= 1,
	}
};

static struct gpio_keys_button tl_wa901nd_v2_gpio_keys[] __initdata = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = TL_WA901ND_V2_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= TL_WA901ND_V2_GPIO_BTN_RESET,
		.active_low	= 1,
	}, {
		.desc		= "qss",
		.type		= EV_KEY,
		.code		= KEY_WPS_BUTTON,
		.debounce_interval = TL_WA901ND_V2_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= TL_WA901ND_V2_GPIO_BTN_QSS,
		.active_low	= 1,
	}
};

static void __init tl_wa901nd_v2_setup(void)
{
	u8 *mac = (u8 *) KSEG1ADDR(0x1f01fc00);
	u8 *eeprom  = (u8 *) KSEG1ADDR(0x1fff1000);

	ath79_init_mac(ath79_eth0_data.mac_addr, mac, 0);

	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ath79_eth0_data.phy_mask = 0x00001000;
	ath79_register_mdio(0, 0x0);

	ath79_eth0_data.reset_bit = AR71XX_RESET_GE0_MAC |
				    AR71XX_RESET_GE0_PHY;
	ath79_register_eth(0);

	ath79_register_m25p80(&tl_wa901nd_v2_flash_data);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(tl_wa901nd_v2_leds_gpio),
				 tl_wa901nd_v2_leds_gpio);

	ath79_register_gpio_keys_polled(-1, TL_WA901ND_V2_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(tl_wa901nd_v2_gpio_keys),
					tl_wa901nd_v2_gpio_keys);

	ath79_register_wmac(eeprom, mac);
}

MIPS_MACHINE(ATH79_MACH_TL_WA901ND_V2, "TL-WA901ND-v2",
	     "TP-LINK TL-WA901ND v2", tl_wa901nd_v2_setup);

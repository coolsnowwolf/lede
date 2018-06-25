/*
 *  AzureWave AW-NR580 board support
 *
 *  Copyright (C) 2008-2012 Gabor Juhos <juhosg@openwrt.org>
 *  Copyright (C) 2008 Imre Kaloz <kaloz@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <asm/mach-ath79/ath79.h>

#include "dev-eth.h"
#include "dev-m25p80.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "machtypes.h"
#include "pci.h"

#define AW_NR580_GPIO_LED_READY_RED	0
#define AW_NR580_GPIO_LED_WLAN		1
#define AW_NR580_GPIO_LED_READY_GREEN	2
#define AW_NR580_GPIO_LED_WPS_GREEN	4
#define AW_NR580_GPIO_LED_WPS_AMBER	5

#define AW_NR580_GPIO_BTN_WPS		3
#define AW_NR580_GPIO_BTN_RESET		11

#define AW_NR580_KEYS_POLL_INTERVAL	20	/* msecs */
#define AW_NR580_KEYS_DEBOUNCE_INTERVAL	(3 * AW_NR580_KEYS_POLL_INTERVAL)

static struct gpio_led aw_nr580_leds_gpio[] __initdata = {
	{
		.name		= "aw-nr580:red:ready",
		.gpio		= AW_NR580_GPIO_LED_READY_RED,
		.active_low	= 0,
	}, {
		.name		= "aw-nr580:green:ready",
		.gpio		= AW_NR580_GPIO_LED_READY_GREEN,
		.active_low	= 0,
	}, {
		.name		= "aw-nr580:green:wps",
		.gpio		= AW_NR580_GPIO_LED_WPS_GREEN,
		.active_low	= 0,
	}, {
		.name		= "aw-nr580:amber:wps",
		.gpio		= AW_NR580_GPIO_LED_WPS_AMBER,
		.active_low	= 0,
	}, {
		.name		= "aw-nr580:green:wlan",
		.gpio		= AW_NR580_GPIO_LED_WLAN,
		.active_low	= 0,
	}
};

static struct gpio_keys_button aw_nr580_gpio_keys[] __initdata = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = AW_NR580_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= AW_NR580_GPIO_BTN_RESET,
		.active_low	= 1,
	}, {
		.desc		= "wps",
		.type		= EV_KEY,
		.code		= KEY_WPS_BUTTON,
		.debounce_interval = AW_NR580_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= AW_NR580_GPIO_BTN_WPS,
		.active_low	= 1,
	}
};

static const char *aw_nr580_part_probes[] = {
	"RedBoot",
	NULL,
};

static struct flash_platform_data aw_nr580_flash_data = {
	.part_probes	= aw_nr580_part_probes,
};

static void __init aw_nr580_setup(void)
{
	ath79_register_mdio(0, 0x0);

	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ath79_eth0_data.speed = SPEED_100;
	ath79_eth0_data.duplex = DUPLEX_FULL;

	ath79_register_eth(0);

	ath79_register_pci();

	ath79_register_m25p80(&aw_nr580_flash_data);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(aw_nr580_leds_gpio),
				 aw_nr580_leds_gpio);

	ath79_register_gpio_keys_polled(-1, AW_NR580_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(aw_nr580_gpio_keys),
					aw_nr580_gpio_keys);
}

MIPS_MACHINE(ATH79_MACH_AW_NR580, "AW-NR580", "AzureWave AW-NR580",
	     aw_nr580_setup);

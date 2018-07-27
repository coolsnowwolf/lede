/*
 *  Compex WPE72 board support
 *
 *  Copyright (C) 2012 Johnathan Boyce<jon.boyce@globalreach.eu.com>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <asm/mach-ath79/ath79.h>

#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-usb.h"
#include "machtypes.h"
#include "pci.h"

#define WPE72_GPIO_RESET	12
#define WPE72_GPIO_LED_DIAG	13
#define WPE72_GPIO_LED_1	14
#define WPE72_GPIO_LED_2	15
#define WPE72_GPIO_LED_3	16
#define WPE72_GPIO_LED_4	17

#define WPE72_KEYS_POLL_INTERVAL	20	/* msecs */
#define WPE72_KEYS_DEBOUNCE_INTERVAL	(3 * WPE72_KEYS_POLL_INTERVAL)

static struct gpio_led wpe72_leds_gpio[] __initdata = {
	{
		.name		= "wpe72:green:led1",
		.gpio		= WPE72_GPIO_LED_1,
		.active_low	= 1,
	}, {
		.name		= "wpe72:green:led2",
		.gpio		= WPE72_GPIO_LED_2,
		.active_low	= 1,
	}, {
		.name		= "wpe72:green:led3",
		.gpio		= WPE72_GPIO_LED_3,
		.active_low	= 1,
	}, {
		.name		= "wpe72:green:led4",
		.gpio		= WPE72_GPIO_LED_4,
		.active_low	= 1,
	}, {
		.name		= "wpe72:green:diag",
		.gpio		= WPE72_GPIO_LED_DIAG,
		.active_low	= 1,
	}
};

static struct gpio_keys_button wpe72_gpio_keys[] __initdata = {
	{
		.desc           = "reset",
		.type           = EV_KEY,
		.code           = KEY_RESTART,
		.debounce_interval = WPE72_KEYS_DEBOUNCE_INTERVAL,
		.gpio           = WPE72_GPIO_RESET,
		.active_low	= 1,
	}
};

static const char *wpe72_part_probes[] = {
	"MyLoader",
	NULL,
};

static struct flash_platform_data wpe72_flash_data = {
	.part_probes	= wpe72_part_probes,
};

static void __init wpe72_setup(void)
{
	ath79_register_m25p80(&wpe72_flash_data);
	ath79_register_mdio(0, 0x0);

	ath79_init_mac(ath79_eth0_data.mac_addr, ath79_mac_base, 0);
	ath79_init_mac(ath79_eth1_data.mac_addr, ath79_mac_base, 1);

	ath79_register_eth(0);
	ath79_register_eth(1);

	ath79_register_usb();
	ath79_register_pci();

	ath79_register_leds_gpio(-1, ARRAY_SIZE(wpe72_leds_gpio),
				 wpe72_leds_gpio);

	ath79_register_gpio_keys_polled(-1, WPE72_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(wpe72_gpio_keys),
					wpe72_gpio_keys);
}

MIPS_MACHINE(ATH79_MACH_WPE72, "WPE72", "Compex WPE72", wpe72_setup);

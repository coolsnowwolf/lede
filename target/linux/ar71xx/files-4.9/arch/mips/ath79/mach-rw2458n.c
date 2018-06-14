/*
 *  Redwave RW2458N support
 *
 *  Copyright (C) 2011-2013 Cezary Jackiewicz <cezary@eko.one.pl>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <asm/mach-ath79/ath79.h>

#include "dev-eth.h"
#include "dev-ap9x-pci.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-usb.h"
#include "machtypes.h"
#include "pci.h"

#define RW2458N_GPIO_LED_D3	1
#define RW2458N_GPIO_LED_D4	0
#define RW2458N_GPIO_LED_D5	11
#define RW2458N_GPIO_LED_D6	7
#define RW2458N_GPIO_BTN_RESET	12

#define RW2458N_KEYS_POLL_INTERVAL	20	/* msecs */
#define RW2458N_KEYS_DEBOUNCE_INTERVAL	(3 * RW2458N_KEYS_POLL_INTERVAL)

static struct gpio_keys_button rw2458n_gpio_keys[] __initdata = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = RW2458N_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= RW2458N_GPIO_BTN_RESET,
		.active_low	= 1,
	}
};

#define RW2458N_WAN_PHYMASK	BIT(4)

static struct gpio_led rw2458n_leds_gpio[] __initdata = {
	{
		.name		= "rw2458n:green:d3",
		.gpio		= RW2458N_GPIO_LED_D3,
		.active_low	= 1,
	}, {
		.name		= "rw2458n:green:d4",
		.gpio		= RW2458N_GPIO_LED_D4,
		.active_low	= 1,
	}, {
		.name		= "rw2458n:green:d5",
		.gpio		= RW2458N_GPIO_LED_D5,
		.active_low	= 1,
	}, {
		.name		= "rw2458n:green:d6",
		.gpio		= RW2458N_GPIO_LED_D6,
		.active_low	= 1,
	}
};

static void __init rw2458n_setup(void)
{
	u8 *mac1 = (u8 *) KSEG1ADDR(0x1fff0000);
	u8 *mac2 = (u8 *) KSEG1ADDR(0x1fff0000 + ETH_ALEN);

	ath79_register_m25p80(NULL);

	ath79_register_mdio(0, ~RW2458N_WAN_PHYMASK);

	ath79_init_mac(ath79_eth0_data.mac_addr, mac1, 0);
	ath79_init_mac(ath79_eth1_data.mac_addr, mac2, 0);

	ath79_register_eth(0);
	ath79_register_eth(1);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(rw2458n_leds_gpio),
				 rw2458n_leds_gpio);

	ath79_register_gpio_keys_polled(-1, RW2458N_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(rw2458n_gpio_keys),
					rw2458n_gpio_keys);
	ath79_register_usb();

	ath79_register_pci();
}

MIPS_MACHINE(ATH79_MACH_RW2458N, "RW2458N", "Redwave RW2458N",
	    rw2458n_setup);

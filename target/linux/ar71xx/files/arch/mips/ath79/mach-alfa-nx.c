/*
 *  ALFA Network N2/N5 board support
 *
 *  Copyright (C) 2011-2012 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <asm/mach-ath79/ar71xx_regs.h>
#include <asm/mach-ath79/ath79.h>

#include "common.h"
#include "dev-eth.h"
#include "dev-ap9x-pci.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "machtypes.h"

#define ALFA_NX_GPIO_LED_2		17
#define ALFA_NX_GPIO_LED_3		16
#define ALFA_NX_GPIO_LED_5		12
#define ALFA_NX_GPIO_LED_6		8
#define ALFA_NX_GPIO_LED_7		6
#define ALFA_NX_GPIO_LED_8		7

#define ALFA_NX_GPIO_BTN_RESET		11

#define ALFA_NX_KEYS_POLL_INTERVAL	20	/* msecs */
#define ALFA_NX_KEYS_DEBOUNCE_INTERVAL (3 * ALFA_NX_KEYS_POLL_INTERVAL)

#define ALFA_NX_MAC0_OFFSET		0
#define ALFA_NX_MAC1_OFFSET		6
#define ALFA_NX_CALDATA_OFFSET		0x1000

static struct gpio_keys_button alfa_nx_gpio_keys[] __initdata = {
	{
		.desc		= "Reset button",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = ALFA_NX_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= ALFA_NX_GPIO_BTN_RESET,
		.active_low	= 1,
	}
};

static struct gpio_led alfa_nx_leds_gpio[] __initdata = {
	{
		.name		= "alfa:green:led_2",
		.gpio		= ALFA_NX_GPIO_LED_2,
		.active_low	= 1,
	}, {
		.name		= "alfa:green:led_3",
		.gpio		= ALFA_NX_GPIO_LED_3,
		.active_low	= 1,
	}, {
		.name		= "alfa:red:led_5",
		.gpio		= ALFA_NX_GPIO_LED_5,
		.active_low	= 1,
	}, {
		.name		= "alfa:amber:led_6",
		.gpio		= ALFA_NX_GPIO_LED_6,
		.active_low	= 1,
	}, {
		.name		= "alfa:green:led_7",
		.gpio		= ALFA_NX_GPIO_LED_7,
		.active_low	= 1,
	}, {
		.name		= "alfa:green:led_8",
		.gpio		= ALFA_NX_GPIO_LED_8,
		.active_low	= 1,
	}
};

static void __init alfa_nx_setup(void)
{
	u8 *art = (u8 *) KSEG1ADDR(0x1fff0000);

	ath79_gpio_function_setup(AR724X_GPIO_FUNC_JTAG_DISABLE,
				  AR724X_GPIO_FUNC_ETH_SWITCH_LED0_EN |
				  AR724X_GPIO_FUNC_ETH_SWITCH_LED1_EN |
				  AR724X_GPIO_FUNC_ETH_SWITCH_LED2_EN |
				  AR724X_GPIO_FUNC_ETH_SWITCH_LED3_EN |
				  AR724X_GPIO_FUNC_ETH_SWITCH_LED4_EN);

	ath79_register_m25p80(NULL);

	ath79_register_leds_gpio(0, ARRAY_SIZE(alfa_nx_leds_gpio),
				 alfa_nx_leds_gpio);

	ath79_register_gpio_keys_polled(-1, ALFA_NX_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(alfa_nx_gpio_keys),
					alfa_nx_gpio_keys);

	ath79_register_mdio(0, 0x0);

	ath79_init_mac(ath79_eth0_data.mac_addr,
		       art + ALFA_NX_MAC0_OFFSET, 0);
	ath79_init_mac(ath79_eth1_data.mac_addr,
		       art + ALFA_NX_MAC1_OFFSET, 0);

	/* WAN port */
	ath79_register_eth(0);
	/* LAN port */
	ath79_register_eth(1);

	ap91_pci_init(art + ALFA_NX_CALDATA_OFFSET, NULL);
}

MIPS_MACHINE(ATH79_MACH_ALFA_NX, "ALFA-NX", "ALFA Network N2/N5",
	     alfa_nx_setup);

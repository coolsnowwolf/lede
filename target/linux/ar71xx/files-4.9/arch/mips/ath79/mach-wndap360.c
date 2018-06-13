/*
 *  Netgear WNDAP360 board support (proper leds / button support missing)
 *
 *  Based on AP96
 *  Copyright (C) 2013 Jacek Kikiewicz
 *  Copyright (C) 2009 Marco Porsch
 *  Copyright (C) 2009-2012 Gabor Juhos <juhosg@openwrt.org>
 *  Copyright (C) 2010 Atheros Communications
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/platform_device.h>
#include <linux/delay.h>

#include <asm/mach-ath79/ath79.h>

#include "dev-ap9x-pci.h"
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "machtypes.h"

#define WNDAP360_GPIO_LED_POWER_ORANGE		0
#define WNDAP360_GPIO_LED_POWER_GREEN		2

/* Reset button - next to the power connector */
#define WNDAP360_GPIO_BTN_RESET			8

#define WNDAP360_KEYS_POLL_INTERVAL		20	/* msecs */
#define WNDAP360_KEYS_DEBOUNCE_INTERVAL	(3 * WNDAP360_KEYS_POLL_INTERVAL)

#define WNDAP360_WMAC0_MAC_OFFSET		0x120c
#define WNDAP360_WMAC1_MAC_OFFSET		0x520c
#define WNDAP360_CALDATA0_OFFSET		0x1000
#define WNDAP360_CALDATA1_OFFSET		0x5000

/*
 * WNDAP360 this still uses leds definitions from AP96
 *
 */
static struct gpio_led wndap360_leds_gpio[] __initdata = {
	{
		.name		= "netgear:green:power",
		.gpio		= WNDAP360_GPIO_LED_POWER_GREEN,
		.active_low	= 1,
	}, {
		.name		= "netgear:orange:power",
		.gpio		= WNDAP360_GPIO_LED_POWER_ORANGE,
		.active_low	= 1,
        }
};

static struct gpio_keys_button wndap360_gpio_keys[] __initdata = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = WNDAP360_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= WNDAP360_GPIO_BTN_RESET,
		.active_low	= 1,
	}
};

#define WNDAP360_LAN_PHYMASK 0x0f

static void __init wndap360_setup(void)
{
	u8 *art = (u8 *) KSEG1ADDR(0x1fff0000);

	ath79_register_mdio(0, ~(WNDAP360_LAN_PHYMASK));

	/* Reusing wifi MAC with offset of 1 as eth0 MAC */
	ath79_init_mac(ath79_eth0_data.mac_addr,
		       art + WNDAP360_WMAC0_MAC_OFFSET, 1);
	ath79_eth0_pll_data.pll_1000 = 0x11110000;
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth0_data.phy_mask = WNDAP360_LAN_PHYMASK;
	ath79_eth0_data.speed = SPEED_1000;
	ath79_eth0_data.duplex = DUPLEX_FULL;

	ath79_register_eth(0);

	ath79_register_m25p80(NULL);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(wndap360_leds_gpio),
					wndap360_leds_gpio);

	ath79_register_gpio_keys_polled(-1, WNDAP360_KEYS_POLL_INTERVAL,
					 ARRAY_SIZE(wndap360_gpio_keys),
					 wndap360_gpio_keys);

	ap9x_pci_setup_wmac_led_pin(0, 5);
	ap9x_pci_setup_wmac_led_pin(1, 5);

	ap94_pci_init(art + WNDAP360_CALDATA0_OFFSET,
		      art + WNDAP360_WMAC0_MAC_OFFSET,
		      art + WNDAP360_CALDATA1_OFFSET,
		      art + WNDAP360_WMAC1_MAC_OFFSET);
}

MIPS_MACHINE(ATH79_MACH_WNDAP360, "WNDAP360", "NETGEAR WNDAP360", wndap360_setup);

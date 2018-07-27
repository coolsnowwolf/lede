/*
 *  Atheros AP96 board support
 *
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
#include "dev-usb.h"
#include "machtypes.h"

#define AP96_GPIO_LED_12_GREEN		0
#define AP96_GPIO_LED_3_GREEN		1
#define AP96_GPIO_LED_2_GREEN		2
#define AP96_GPIO_LED_WPS_GREEN		4
#define AP96_GPIO_LED_5_GREEN		5
#define AP96_GPIO_LED_4_ORANGE		6

/* Reset button - next to the power connector */
#define AP96_GPIO_BTN_RESET		3
/* WPS button - next to a led on right */
#define AP96_GPIO_BTN_WPS		8

#define AP96_KEYS_POLL_INTERVAL		20	/* msecs */
#define AP96_KEYS_DEBOUNCE_INTERVAL	(3 * AP96_KEYS_POLL_INTERVAL)

#define AP96_WMAC0_MAC_OFFSET		0x120c
#define AP96_WMAC1_MAC_OFFSET		0x520c
#define AP96_CALDATA0_OFFSET		0x1000
#define AP96_CALDATA1_OFFSET		0x5000

/*
 * AP96 has 12 unlabeled leds in the front; these are numbered from 1 to 12
 * below (from left to right on the board). Led 1 seems to be on whenever the
 * board is powered. Led 11 shows LAN link activity actity. Led 3 is orange;
 * others are green.
 *
 * In addition, there is one led next to a button on the right side for WPS.
 */
static struct gpio_led ap96_leds_gpio[] __initdata = {
	{
		.name		= "ap96:green:led2",
		.gpio		= AP96_GPIO_LED_2_GREEN,
		.active_low	= 1,
	}, {
		.name		= "ap96:green:led3",
		.gpio		= AP96_GPIO_LED_3_GREEN,
		.active_low	= 1,
	}, {
		.name		= "ap96:orange:led4",
		.gpio		= AP96_GPIO_LED_4_ORANGE,
		.active_low	= 1,
	}, {
		.name		= "ap96:green:led5",
		.gpio		= AP96_GPIO_LED_5_GREEN,
		.active_low	= 1,
	}, {
		.name		= "ap96:green:led12",
		.gpio		= AP96_GPIO_LED_12_GREEN,
		.active_low	= 1,
	}, { /* next to a button on right */
		.name		= "ap96:green:wps",
		.gpio		= AP96_GPIO_LED_WPS_GREEN,
		.active_low	= 1,
	}
};

static struct gpio_keys_button ap96_gpio_keys[] __initdata = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = AP96_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= AP96_GPIO_BTN_RESET,
		.active_low	= 1,
	}, {
		.desc		= "wps",
		.type		= EV_KEY,
		.code		= KEY_WPS_BUTTON,
		.debounce_interval = AP96_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= AP96_GPIO_BTN_WPS,
		.active_low	= 1,
	}
};

#define AP96_WAN_PHYMASK 0x10
#define AP96_LAN_PHYMASK 0x0f

static void __init ap96_setup(void)
{
	u8 *art = (u8 *) KSEG1ADDR(0x1fff0000);

	ath79_register_mdio(0, ~(AP96_WAN_PHYMASK | AP96_LAN_PHYMASK));

	ath79_init_mac(ath79_eth0_data.mac_addr, art, 0);
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth0_data.phy_mask = AP96_LAN_PHYMASK;
	ath79_eth0_data.speed = SPEED_1000;
	ath79_eth0_data.duplex = DUPLEX_FULL;

	ath79_register_eth(0);

	ath79_init_mac(ath79_eth1_data.mac_addr, art, 1);
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth1_data.phy_mask = AP96_WAN_PHYMASK;

	ath79_eth1_pll_data.pll_1000 = 0x1f000000;

	ath79_register_eth(1);

	ath79_register_usb();

	ath79_register_m25p80(NULL);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(ap96_leds_gpio),
					ap96_leds_gpio);

	ath79_register_gpio_keys_polled(-1, AP96_KEYS_POLL_INTERVAL,
					 ARRAY_SIZE(ap96_gpio_keys),
					 ap96_gpio_keys);

	ap94_pci_init(art + AP96_CALDATA0_OFFSET,
		      art + AP96_WMAC0_MAC_OFFSET,
		      art + AP96_CALDATA1_OFFSET,
		      art + AP96_WMAC1_MAC_OFFSET);
}

MIPS_MACHINE(ATH79_MACH_AP96, "AP96", "Atheros AP96", ap96_setup);

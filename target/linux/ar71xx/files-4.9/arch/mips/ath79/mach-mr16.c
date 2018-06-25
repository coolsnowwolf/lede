/*
 *  Cisco Meraki MR16 board support
 *
 *  Copyright (C) 2015 Chris Blake <chrisrblake93@gmail.com>
 *
 *  Based on Atheros AP96 board support configuration
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
#include "machtypes.h"

#define MR16_GPIO_LED_W4_GREEN		3
#define MR16_GPIO_LED_W3_GREEN		2
#define MR16_GPIO_LED_W2_GREEN		1
#define MR16_GPIO_LED_W1_GREEN		0

#define MR16_GPIO_LED_WAN		4

#define MR16_GPIO_LED_POWER_ORANGE		5
#define MR16_GPIO_LED_POWER_GREEN		6

#define MR16_GPIO_BTN_RESET		7
#define MR16_KEYS_POLL_INTERVAL		20	/* msecs */
#define MR16_KEYS_DEBOUNCE_INTERVAL	(3 * MR16_KEYS_POLL_INTERVAL)

#define MR16_WAN_PHYMASK    BIT(0)

#define MR16_CALDATA0_OFFSET		0x21000
#define MR16_CALDATA1_OFFSET		0x25000

static struct gpio_led MR16_leds_gpio[] __initdata = {
	{
		.name		= "mr16:green:wan",
		.gpio		= MR16_GPIO_LED_WAN,
		.active_low	= 1,
	}, {
		.name		= "mr16:orange:power",
		.gpio		= MR16_GPIO_LED_POWER_ORANGE,
		.active_low	= 1,
	}, {
		.name		= "mr16:green:power",
		.gpio		= MR16_GPIO_LED_POWER_GREEN,
		.active_low	= 1,
	}, {
		.name		= "mr16:green:wifi4",
		.gpio		= MR16_GPIO_LED_W4_GREEN,
		.active_low	= 1,
	}, {
		.name		= "mr16:green:wifi3",
		.gpio		= MR16_GPIO_LED_W3_GREEN,
		.active_low	= 1,
	}, {
		.name		= "mr16:green:wifi2",
		.gpio		= MR16_GPIO_LED_W2_GREEN,
		.active_low	= 1,
	}, {
		.name		= "mr16:green:wifi1",
		.gpio		= MR16_GPIO_LED_W1_GREEN,
		.active_low	= 1,
	}
};

static struct gpio_keys_button MR16_gpio_keys[] __initdata = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = MR16_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= MR16_GPIO_BTN_RESET,
		.active_low	= 1,
	}
};

static void __init MR16_setup(void)
{
	u8 *mac = (u8 *) KSEG1ADDR(0xbffd0000);
	u8 wlan0_mac[ETH_ALEN];
	u8 wlan1_mac[ETH_ALEN];

	ath79_register_mdio(0,0x0);

	ath79_init_mac(ath79_eth0_data.mac_addr, mac, 0);
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth0_data.phy_mask = MR16_WAN_PHYMASK;
	ath79_register_eth(0);

	ath79_register_m25p80(NULL);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(MR16_leds_gpio),
					MR16_leds_gpio);
	ath79_register_gpio_keys_polled(-1, MR16_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(MR16_gpio_keys),
					MR16_gpio_keys);

	ath79_init_mac(wlan0_mac, mac, 1);
	ath79_init_mac(wlan1_mac, mac, 2);
	ap94_pci_init(mac + MR16_CALDATA0_OFFSET, wlan0_mac,
		    mac + MR16_CALDATA1_OFFSET, wlan1_mac);
}

MIPS_MACHINE(ATH79_MACH_MR16, "MR16", "Meraki MR16", MR16_setup);

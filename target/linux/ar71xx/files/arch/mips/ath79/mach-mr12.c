/*
 *  Cisco Meraki MR12 board support
 *
 *  Copyright (C) 2014-2015 Chris Blake <chrisrblake93@gmail.com>
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

#define MR12_GPIO_LED_W4_GREEN		14
#define MR12_GPIO_LED_W3_GREEN		13
#define MR12_GPIO_LED_W2_GREEN		12
#define MR12_GPIO_LED_W1_GREEN		11

#define MR12_GPIO_LED_WAN		15

#define MR12_GPIO_LED_POWER_ORANGE		16
#define MR12_GPIO_LED_POWER_GREEN		17

#define MR12_GPIO_BTN_RESET		8
#define MR12_KEYS_POLL_INTERVAL		20	/* msecs */
#define MR12_KEYS_DEBOUNCE_INTERVAL	(3 * MR12_KEYS_POLL_INTERVAL)

#define MR12_WAN_PHYMASK    BIT(4)

#define MR12_CALDATA0_OFFSET            0x21000

static struct gpio_led MR12_leds_gpio[] __initdata = {
	{
		.name		= "mr12:green:wan",
		.gpio		= MR12_GPIO_LED_WAN,
		.active_low	= 1,
	}, {
		.name		= "mr12:orange:power",
		.gpio		= MR12_GPIO_LED_POWER_ORANGE,
		.active_low	= 1,
	}, {
		.name		= "mr12:green:power",
		.gpio		= MR12_GPIO_LED_POWER_GREEN,
		.active_low	= 1,
	}, {
		.name		= "mr12:green:wifi4",
		.gpio		= MR12_GPIO_LED_W4_GREEN,
		.active_low	= 1,
	}, {
		.name		= "mr12:green:wifi3",
		.gpio		= MR12_GPIO_LED_W3_GREEN,
		.active_low	= 1,
	}, {
		.name		= "mr12:green:wifi2",
		.gpio		= MR12_GPIO_LED_W2_GREEN,
		.active_low	= 1,
	}, {
		.name		= "mr12:green:wifi1",
		.gpio		= MR12_GPIO_LED_W1_GREEN,
		.active_low	= 1,
	}
};

static struct gpio_keys_button MR12_gpio_keys[] __initdata = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = MR12_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= MR12_GPIO_BTN_RESET,
		.active_low	= 1,
	}
};

static void __init MR12_setup(void)
{
	u8 *mac = (u8 *) KSEG1ADDR(0xbffd0000);
	u8 wlan_mac[ETH_ALEN];

	ath79_register_mdio(0,0x0);

	ath79_init_mac(ath79_eth0_data.mac_addr, mac, 0);
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth0_data.phy_mask = MR12_WAN_PHYMASK;
	ath79_register_eth(0);

	ath79_register_m25p80(NULL);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(MR12_leds_gpio),
					MR12_leds_gpio);
	ath79_register_gpio_keys_polled(-1, MR12_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(MR12_gpio_keys),
					MR12_gpio_keys);

	ath79_init_mac(wlan_mac, mac, 1);
	ap91_pci_init(mac + MR12_CALDATA0_OFFSET, wlan_mac);
}

MIPS_MACHINE(ATH79_MACH_MR12, "MR12", "Meraki MR12", MR12_setup);

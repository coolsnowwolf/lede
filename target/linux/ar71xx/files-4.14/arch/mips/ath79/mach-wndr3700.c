/*
 *  Netgear WNDR3700 board support
 *
 *  Copyright (C) 2009 Marco Porsch
 *  Copyright (C) 2009-2012 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/platform_device.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/delay.h>
#include <linux/rtl8366.h>

#include <asm/mach-ath79/ath79.h>

#include "dev-ap9x-pci.h"
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-usb.h"
#include "machtypes.h"

#define WNDR3700_GPIO_LED_WPS_ORANGE	0
#define WNDR3700_GPIO_LED_POWER_ORANGE	1
#define WNDR3700_GPIO_LED_POWER_GREEN	2
#define WNDR3700_GPIO_LED_WPS_GREEN	4
#define WNDR3700_GPIO_LED_WAN_GREEN	6

#define WNDR3700_GPIO_BTN_WPS		3
#define WNDR3700_GPIO_BTN_RESET		8
#define WNDR3700_GPIO_BTN_RFKILL	11

#define WNDR3700_GPIO_RTL8366_SDA	5
#define WNDR3700_GPIO_RTL8366_SCK	7

#define WNDR3700_KEYS_POLL_INTERVAL	20	/* msecs */
#define WNDR3700_KEYS_DEBOUNCE_INTERVAL (3 * WNDR3700_KEYS_POLL_INTERVAL)

#define WNDR3700_ETH0_MAC_OFFSET	0
#define WNDR3700_ETH1_MAC_OFFSET	0x6

#define WNDR3700_WMAC0_MAC_OFFSET	0
#define WNDR3700_WMAC1_MAC_OFFSET	0xc
#define WNDR3700_CALDATA0_OFFSET	0x1000
#define WNDR3700_CALDATA1_OFFSET	0x5000

static struct gpio_led wndr3700_leds_gpio[] __initdata = {
	{
		.name		= "netgear:green:power",
		.gpio		= WNDR3700_GPIO_LED_POWER_GREEN,
		.active_low	= 1,
	}, {
		.name		= "netgear:orange:power",
		.gpio		= WNDR3700_GPIO_LED_POWER_ORANGE,
		.active_low	= 1,
	}, {
		.name		= "netgear:green:wps",
		.gpio		= WNDR3700_GPIO_LED_WPS_GREEN,
		.active_low	= 1,
	}, {
		.name		= "netgear:orange:wps",
		.gpio		= WNDR3700_GPIO_LED_WPS_ORANGE,
		.active_low	= 1,
	}, {
		.name		= "netgear:green:wan",
		.gpio		= WNDR3700_GPIO_LED_WAN_GREEN,
		.active_low	= 1,
	}
};

static struct gpio_keys_button wndr3700_gpio_keys[] __initdata = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = WNDR3700_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= WNDR3700_GPIO_BTN_RESET,
		.active_low	= 1,
	}, {
		.desc		= "wps",
		.type		= EV_KEY,
		.code		= KEY_WPS_BUTTON,
		.debounce_interval = WNDR3700_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= WNDR3700_GPIO_BTN_WPS,
		.active_low	= 1,
	}, {
		.desc		= "rfkill",
		.type		= EV_KEY,
		.code		= KEY_RFKILL,
		.debounce_interval = WNDR3700_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= WNDR3700_GPIO_BTN_RFKILL,
		.active_low	= 1,
	}
};

static struct rtl8366_platform_data wndr3700_rtl8366s_data = {
	.gpio_sda	= WNDR3700_GPIO_RTL8366_SDA,
	.gpio_sck	= WNDR3700_GPIO_RTL8366_SCK,
};

static struct platform_device wndr3700_rtl8366s_device = {
	.name		= RTL8366S_DRIVER_NAME,
	.id		= -1,
	.dev = {
		.platform_data	= &wndr3700_rtl8366s_data,
	}
};

static void __init wndr3700_setup(void)
{
	u8 *art = (u8 *) KSEG1ADDR(0x1fff0000);

	/*
	 * The eth0 and wmac0 interfaces share the same MAC address which
	 * can lead to problems if operated unbridged. Set the locally
	 * administered bit on the eth0 MAC to make it unique.
	 */
	ath79_init_local_mac(ath79_eth0_data.mac_addr,
			     art + WNDR3700_ETH0_MAC_OFFSET);
	ath79_eth0_pll_data.pll_1000 = 0x11110000;
	ath79_eth0_data.mii_bus_dev = &wndr3700_rtl8366s_device.dev;
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth0_data.speed = SPEED_1000;
	ath79_eth0_data.duplex = DUPLEX_FULL;

	ath79_init_mac(ath79_eth1_data.mac_addr,
			art + WNDR3700_ETH1_MAC_OFFSET, 0);
	ath79_eth1_pll_data.pll_1000 = 0x11110000;
	ath79_eth1_data.mii_bus_dev = &wndr3700_rtl8366s_device.dev;
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth1_data.phy_mask = 0x10;

	ath79_register_eth(0);
	ath79_register_eth(1);

	ath79_register_usb();

	ath79_register_m25p80(NULL);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(wndr3700_leds_gpio),
				 wndr3700_leds_gpio);

	ath79_register_gpio_keys_polled(-1, WNDR3700_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(wndr3700_gpio_keys),
					wndr3700_gpio_keys);

	platform_device_register(&wndr3700_rtl8366s_device);
	platform_device_register_simple("wndr3700-led-usb", -1, NULL, 0);

	ap9x_pci_setup_wmac_led_pin(0, 5);
	ap9x_pci_setup_wmac_led_pin(1, 5);

	/* 2.4 GHz uses the first fixed antenna group (1, 0, 1, 0) */
	ap9x_pci_setup_wmac_gpio(0, (0xf << 6), (0xa << 6));

	/* 5 GHz uses the second fixed antenna group (0, 1, 1, 0) */
	ap9x_pci_setup_wmac_gpio(1, (0xf << 6), (0x6 << 6));

	ap94_pci_init(art + WNDR3700_CALDATA0_OFFSET,
		      art + WNDR3700_WMAC0_MAC_OFFSET,
		      art + WNDR3700_CALDATA1_OFFSET,
		      art + WNDR3700_WMAC1_MAC_OFFSET);
}

MIPS_MACHINE(ATH79_MACH_WNDR3700, "WNDR3700",
	     "NETGEAR WNDR3700/WNDR3800/WNDRMAC",
	     wndr3700_setup);

/*
 *  D-Link DIR-825 rev. B1 board support
 *
 *  Copyright (C) 2009-2011 Lukas Kuna, Evkanet, s.r.o.
 *
 *  based on mach-wndr3700.c
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/rtl8366.h>

#include <asm/mach-ath79/ath79.h>

#include "dev-eth.h"
#include "dev-ap9x-pci.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-usb.h"
#include "machtypes.h"

#define DIR825B1_GPIO_LED_BLUE_USB		0
#define DIR825B1_GPIO_LED_ORANGE_POWER		1
#define DIR825B1_GPIO_LED_BLUE_POWER		2
#define DIR825B1_GPIO_LED_BLUE_WPS		4
#define DIR825B1_GPIO_LED_ORANGE_PLANET		6
#define DIR825B1_GPIO_LED_BLUE_PLANET		11

#define DIR825B1_GPIO_BTN_RESET			3
#define DIR825B1_GPIO_BTN_WPS			8

#define DIR825B1_GPIO_RTL8366_SDA		5
#define DIR825B1_GPIO_RTL8366_SCK		7

#define DIR825B1_KEYS_POLL_INTERVAL		20	/* msecs */
#define DIR825B1_KEYS_DEBOUNCE_INTERVAL		(3 * DIR825B1_KEYS_POLL_INTERVAL)

#define DIR825B1_CAL0_OFFSET			0x1000
#define DIR825B1_CAL1_OFFSET			0x5000
#define DIR825B1_MAC0_OFFSET			0xffa0
#define DIR825B1_MAC1_OFFSET			0xffb4

#define DIR825B1_CAL_LOCATION_0			0x1f660000
#define DIR825B1_CAL_LOCATION_1			0x1f7f0000

static struct gpio_led dir825b1_leds_gpio[] __initdata = {
	{
		.name		= "d-link:blue:usb",
		.gpio		= DIR825B1_GPIO_LED_BLUE_USB,
		.active_low	= 1,
	}, {
		.name		= "d-link:orange:power",
		.gpio		= DIR825B1_GPIO_LED_ORANGE_POWER,
		.active_low	= 1,
	}, {
		.name		= "d-link:blue:power",
		.gpio		= DIR825B1_GPIO_LED_BLUE_POWER,
		.active_low	= 1,
	}, {
		.name		= "d-link:blue:wps",
		.gpio		= DIR825B1_GPIO_LED_BLUE_WPS,
		.active_low	= 1,
	}, {
		.name		= "d-link:orange:planet",
		.gpio		= DIR825B1_GPIO_LED_ORANGE_PLANET,
		.active_low	= 1,
	}, {
		.name		= "d-link:blue:planet",
		.gpio		= DIR825B1_GPIO_LED_BLUE_PLANET,
		.active_low	= 1,
	}
};

static struct gpio_keys_button dir825b1_gpio_keys[] __initdata = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = DIR825B1_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= DIR825B1_GPIO_BTN_RESET,
		.active_low	= 1,
	}, {
		.desc		= "wps",
		.type		= EV_KEY,
		.code		= KEY_WPS_BUTTON,
		.debounce_interval = DIR825B1_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= DIR825B1_GPIO_BTN_WPS,
		.active_low	= 1,
	}
};

static struct rtl8366_initval dir825b1_rtl8366s_initvals[] = {
	{ .reg = 0x06, .val = 0x0108 },
};

static struct rtl8366_platform_data dir825b1_rtl8366s_data = {
	.gpio_sda	= DIR825B1_GPIO_RTL8366_SDA,
	.gpio_sck	= DIR825B1_GPIO_RTL8366_SCK,
	.num_initvals	= ARRAY_SIZE(dir825b1_rtl8366s_initvals),
	.initvals	= dir825b1_rtl8366s_initvals,
};

static struct platform_device dir825b1_rtl8366s_device = {
	.name		= RTL8366S_DRIVER_NAME,
	.id		= -1,
	.dev = {
		.platform_data	= &dir825b1_rtl8366s_data,
	}
};

static bool __init dir825b1_is_caldata_valid(u8 *p)
{
	u16 *magic0, *magic1;

	magic0 = (u16 *)(p + DIR825B1_CAL0_OFFSET);
	magic1 = (u16 *)(p + DIR825B1_CAL1_OFFSET);

	return (*magic0 == 0xa55a && *magic1 == 0xa55a);
}

static void __init dir825b1_wlan_init(void)
{
	u8 *caldata;
	u8 mac0[ETH_ALEN], mac1[ETH_ALEN];
	u8 wmac0[ETH_ALEN], wmac1[ETH_ALEN];

	caldata = (u8 *) KSEG1ADDR(DIR825B1_CAL_LOCATION_0);
	if (!dir825b1_is_caldata_valid(caldata)) {
		caldata = (u8 *)KSEG1ADDR(DIR825B1_CAL_LOCATION_1);
		if (!dir825b1_is_caldata_valid(caldata)) {
			pr_err("no calibration data found\n");
			return;
		}
	}

	ath79_parse_ascii_mac(caldata + DIR825B1_MAC0_OFFSET, mac0);
	ath79_parse_ascii_mac(caldata + DIR825B1_MAC1_OFFSET, mac1);

	ath79_init_mac(ath79_eth0_data.mac_addr, mac0, 0);
	ath79_init_mac(ath79_eth1_data.mac_addr, mac1, 0);
	ath79_init_mac(wmac0, mac0, 0);
	ath79_init_mac(wmac1, mac1, 1);

	ap9x_pci_setup_wmac_led_pin(0, 5);
	ap9x_pci_setup_wmac_led_pin(1, 5);

	ap94_pci_init(caldata + DIR825B1_CAL0_OFFSET, wmac0,
		      caldata + DIR825B1_CAL1_OFFSET, wmac1);
}

static void __init dir825b1_setup(void)
{
	dir825b1_wlan_init();

	ath79_register_mdio(0, 0x0);

	ath79_eth0_data.mii_bus_dev = &dir825b1_rtl8366s_device.dev;
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth0_data.speed = SPEED_1000;
	ath79_eth0_data.duplex = DUPLEX_FULL;
	ath79_eth0_pll_data.pll_1000 = 0x11110000;

	ath79_eth1_data.mii_bus_dev = &dir825b1_rtl8366s_device.dev;
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth1_data.phy_mask = 0x10;
	ath79_eth1_pll_data.pll_1000 = 0x11110000;

	ath79_register_eth(0);
	ath79_register_eth(1);

	ath79_register_m25p80(NULL);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(dir825b1_leds_gpio),
				 dir825b1_leds_gpio);

	ath79_register_gpio_keys_polled(-1, DIR825B1_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(dir825b1_gpio_keys),
					dir825b1_gpio_keys);

	ath79_register_usb();

	platform_device_register(&dir825b1_rtl8366s_device);
}

MIPS_MACHINE(ATH79_MACH_DIR_825_B1, "DIR-825-B1", "D-Link DIR-825 rev. B1",
	     dir825b1_setup);

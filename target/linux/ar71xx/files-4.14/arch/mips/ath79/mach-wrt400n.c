/*
 *  Linksys WRT400N board support
 *
 *  Copyright (C) 2009-2012 Gabor Juhos <juhosg@openwrt.org>
 *  Copyright (C) 2009 Imre Kaloz <kaloz@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>

#include <asm/mach-ath79/ath79.h>

#include "dev-ap9x-pci.h"
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "machtypes.h"

#define WRT400N_GPIO_LED_POWER		1
#define WRT400N_GPIO_LED_WPS_BLUE	4
#define WRT400N_GPIO_LED_WPS_AMBER	5
#define WRT400N_GPIO_LED_WLAN		6

#define WRT400N_GPIO_BTN_RESET		8
#define WRT400N_GPIO_BTN_WLSEC		3

#define WRT400N_KEYS_POLL_INTERVAL	20	/* msecs */
#define WRT400N_KEYS_DEBOUNE_INTERVAL	(3 * WRT400N_KEYS_POLL_INTERVAL)

#define WRT400N_MAC_ADDR_OFFSET		0x120c
#define WRT400N_CALDATA0_OFFSET		0x1000
#define WRT400N_CALDATA1_OFFSET		0x5000

static struct mtd_partition wrt400n_partitions[] = {
	{
		.name		= "uboot",
		.offset		= 0,
		.size		= 0x030000,
		.mask_flags	= MTD_WRITEABLE,
	}, {
		.name		= "env",
		.offset		= 0x030000,
		.size		= 0x010000,
		.mask_flags	= MTD_WRITEABLE,
	}, {
		.name		= "linux",
		.offset		= 0x040000,
		.size		= 0x140000,
	}, {
		.name		= "rootfs",
		.offset		= 0x180000,
		.size		= 0x630000,
	}, {
		.name		= "nvram",
		.offset		= 0x7b0000,
		.size		= 0x010000,
		.mask_flags	= MTD_WRITEABLE,
	}, {
		.name		= "factory",
		.offset		= 0x7c0000,
		.size		= 0x010000,
		.mask_flags	= MTD_WRITEABLE,
	}, {
		.name		= "language",
		.offset		= 0x7d0000,
		.size		= 0x020000,
		.mask_flags	= MTD_WRITEABLE,
	}, {
		.name		= "caldata",
		.offset		= 0x7f0000,
		.size		= 0x010000,
		.mask_flags	= MTD_WRITEABLE,
	}, {
		.name		= "firmware",
		.offset		= 0x040000,
		.size		= 0x770000,
	}
};

static struct flash_platform_data wrt400n_flash_data = {
	.parts		= wrt400n_partitions,
	.nr_parts	= ARRAY_SIZE(wrt400n_partitions),
};

static struct gpio_led wrt400n_leds_gpio[] __initdata = {
	{
		.name		= "wrt400n:blue:wps",
		.gpio		= WRT400N_GPIO_LED_WPS_BLUE,
		.active_low	= 1,
	}, {
		.name		= "wrt400n:amber:wps",
		.gpio		= WRT400N_GPIO_LED_WPS_AMBER,
		.active_low	= 1,
	}, {
		.name		= "wrt400n:blue:wlan",
		.gpio		= WRT400N_GPIO_LED_WLAN,
		.active_low	= 1,
	}, {
		.name		= "wrt400n:blue:power",
		.gpio		= WRT400N_GPIO_LED_POWER,
		.active_low	= 0,
		.default_trigger = "default-on",
	}
};

static struct gpio_keys_button wrt400n_gpio_keys[] __initdata = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = WRT400N_KEYS_DEBOUNE_INTERVAL,
		.gpio		= WRT400N_GPIO_BTN_RESET,
		.active_low	= 1,
	}, {
		.desc		= "wlsec",
		.type		= EV_KEY,
		.code		= KEY_WPS_BUTTON,
		.debounce_interval = WRT400N_KEYS_DEBOUNE_INTERVAL,
		.gpio		= WRT400N_GPIO_BTN_WLSEC,
		.active_low	= 1,
	}
};

static void __init wrt400n_setup(void)
{
	u8 *art = (u8 *) KSEG1ADDR(0x1fff0000);
	u8 *mac = art + WRT400N_MAC_ADDR_OFFSET;

	ath79_register_mdio(0, 0x0);

	ath79_init_mac(ath79_eth0_data.mac_addr, mac, 1);
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RMII;
	ath79_eth0_data.speed = SPEED_100;
	ath79_eth0_data.duplex = DUPLEX_FULL;

	ath79_init_mac(ath79_eth1_data.mac_addr, mac, 2);
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_RMII;
	ath79_eth1_data.phy_mask = 0x10;

	ath79_register_eth(0);
	ath79_register_eth(1);

	ath79_register_m25p80(&wrt400n_flash_data);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(wrt400n_leds_gpio),
				 wrt400n_leds_gpio);

	ath79_register_gpio_keys_polled(-1, WRT400N_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(wrt400n_gpio_keys),
					wrt400n_gpio_keys);

	ap94_pci_init(art + WRT400N_CALDATA0_OFFSET, NULL,
		      art + WRT400N_CALDATA1_OFFSET, NULL);
}

MIPS_MACHINE(ATH79_MACH_WRT400N, "WRT400N", "Linksys WRT400N", wrt400n_setup);

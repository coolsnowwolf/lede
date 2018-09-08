/*
 *  D-Link DIR-869 A1 support
 *
 *  Copyright (C) 2015-2016 P. Wassi <p.wassi at gmx.at>
 *  Copyright (C) 2016 Matthias Schiffer <mschiffer@universe-factory.net>
 *
 *  Derived from: mach-ubnt-unifiac.c
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */


#include <linux/gpio.h>
#include <linux/init.h>
#include <linux/pci.h>
#include <linux/platform_device.h>
#include <linux/ath9k_platform.h>

#include <asm/mach-ath79/ath79.h>
#include <asm/mach-ath79/irq.h>
#include <asm/mach-ath79/ar71xx_regs.h>

#include <linux/platform_data/phy-at803x.h>
#include <linux/ar8216_platform.h>

#include "common.h"
#include "dev-ap9x-pci.h"
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-wmac.h"
#include "machtypes.h"
#include "nvram.h"


#define DIR869A1_GPIO_BTN_RESET		1
#define DIR869A1_GPIO_BTN_WPS		2
#define DIR869A1_GPIO_SWITCH_MODE	8

#define DIR869A1_GPIO_ENABLE_SWITCH	11

#define DIR869A1_GPIO_LED_ORANGE	15
#define DIR869A1_GPIO_LED_WHITE		16

#define DIR869A1_KEYS_POLL_INTERVAL	20 /* msecs */
#define DIR869A1_KEYS_DEBOUNCE_INTERVAL	(3 * DIR869A1_KEYS_POLL_INTERVAL)


#define DIR869A1_DEVDATA_ADDR		0x1f050000
#define DIR869A1_DEVDATA_SIZE		0x10000

#define DIR869A1_EEPROM_ADDR		0x1fff0000
#define DIR869A1_WMAC_CALDATA_OFFSET	0x1000
#define DIR869A1_PCI_CALDATA_OFFSET	0x5000


static struct gpio_led dir869a1_leds_gpio[] __initdata = {
	{
		.name		= "d-link:white:status",
		.gpio		= DIR869A1_GPIO_LED_WHITE,
		.active_low	= 1,
	},
	{
		.name		= "d-link:orange:status",
		.gpio		= DIR869A1_GPIO_LED_ORANGE,
		.active_low	= 1,
	},
};

static struct gpio_keys_button dir869a1_gpio_keys[] __initdata = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = DIR869A1_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= DIR869A1_GPIO_BTN_RESET,
		.active_low	= 1,
	},
	{
		.desc		= "wps",
		.type		= EV_KEY,
		.code		= KEY_WPS_BUTTON,
		.debounce_interval = DIR869A1_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= DIR869A1_GPIO_BTN_WPS,
		.active_low	= 1,
	},
	{
		.desc		= "mode",
		.type		= EV_SW,
		.code		= BTN_0,
		.debounce_interval = DIR869A1_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= DIR869A1_GPIO_SWITCH_MODE,
		.active_low	= 0,
	},
};


static struct ar8327_pad_cfg dir869a1_ar8327_pad0_cfg = {
	.mode = AR8327_PAD_MAC_SGMII,
	.sgmii_delay_en = true,
};

static struct ar8327_platform_data dir869a1_ar8327_data = {
	.pad0_cfg = &dir869a1_ar8327_pad0_cfg,
	.port0_cfg = {
		.force_link = 1,
		.speed = AR8327_PORT_SPEED_1000,
		.duplex = 1,
		.txpause = 1,
		.rxpause = 1,
	},
};


static struct mdio_board_info dir869a1_mdio0_info[] = {
	{
		.bus_id = "ag71xx-mdio.0",
		.mdio_addr = 0,
		.platform_data = &dir869a1_ar8327_data,
	},
};


static void dir869a1_get_mac(const char *name, char *mac)
{
	u8 *nvram = (u8 *) KSEG1ADDR(DIR869A1_DEVDATA_ADDR);
	int err;

	err = ath79_nvram_parse_mac_addr(nvram, DIR869A1_DEVDATA_SIZE,
					 name, mac);
	if (err)
		pr_err("no MAC address found for %s\n", name);
}

static void __init dir869a1_setup(void)
{
	u8 *eeprom = (u8 *) KSEG1ADDR(DIR869A1_EEPROM_ADDR);
	u8 wlan24mac[ETH_ALEN] = {}, wlan5mac[ETH_ALEN] = {};

	ath79_register_m25p80(NULL);

	gpio_request_one(DIR869A1_GPIO_ENABLE_SWITCH,
			 GPIOF_OUT_INIT_HIGH | GPIOF_EXPORT_DIR_FIXED,
			 "Switch power");

	dir869a1_get_mac("lanmac=", ath79_eth0_data.mac_addr);
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_SGMII;
	ath79_eth0_data.mii_bus_dev = &ath79_mdio0_device.dev;
	ath79_eth0_data.phy_mask = BIT(0);

	mdiobus_register_board_info(dir869a1_mdio0_info,
	                            ARRAY_SIZE(dir869a1_mdio0_info));

	ath79_register_mdio(0, 0);
	ath79_register_eth(0);

	dir869a1_get_mac("wlan24mac=", wlan24mac);
	ath79_register_wmac(eeprom + DIR869A1_WMAC_CALDATA_OFFSET, wlan24mac);

	dir869a1_get_mac("wlan5mac=", wlan5mac);
	ap91_pci_init(eeprom + DIR869A1_PCI_CALDATA_OFFSET, wlan5mac);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(dir869a1_leds_gpio),
	                         dir869a1_leds_gpio);

	ath79_register_gpio_keys_polled(-1, DIR869A1_KEYS_POLL_INTERVAL,
	                                ARRAY_SIZE(dir869a1_gpio_keys),
	                                dir869a1_gpio_keys);
}

MIPS_MACHINE(ATH79_MACH_DIR_869_A1, "DIR-869-A1", "D-Link DIR-869 rev. A1",
             dir869a1_setup);

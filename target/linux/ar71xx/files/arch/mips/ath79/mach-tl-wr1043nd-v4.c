/*
 *  TP-LINK WR1043 V4 support
 *
 *  Copyright (C) 2015-2016 P. Wassi <p.wassi at gmx.at>
 *  Copyright (C) 2016 Matthias Schiffer <mschiffer@universe-factory.net>
 *  Copyright (C) 2016 Andreas Ziegler <github@andreas-ziegler.de>
 *  Copyright (C) 2016 Ludwig Thomeczek <ledesrc@wxorx.net>
 *  Copyright (C) 2017 Tim Thorpe <tim@tfthorpe.net>
 *
 *  Derived from: mach-dir-869-a1.c
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
#include "dev-usb.h"
#include "machtypes.h"
#include "nvram.h"

#define TL_WR1043_V4_GPIO_BTN_RESET		2
#define TL_WR1043_V4_GPIO_BTN_RFKILL		5

#define TL_WR1043_V4_GPIO_LED_WLAN		19
#define TL_WR1043_V4_GPIO_LED_USB		7
#define TL_WR1043_V4_GPIO_LED_WPS		1
#define TL_WR1043_V4_GPIO_LED_SYSTEM		6

#define TL_WR1043_V4_GPIO_USB_POWER		8

#define TL_WR1043_V4_GPIO_LED_WAN		15
#define TL_WR1043_V4_GPIO_LED_LAN1		9
#define TL_WR1043_V4_GPIO_LED_LAN2		14
#define TL_WR1043_V4_GPIO_LED_LAN3		21
#define TL_WR1043_V4_GPIO_LED_LAN4		20

#define TL_WR1043_V4_KEYS_POLL_INTERVAL		20 /* msecs */
#define TL_WR1043_V4_KEYS_DEBOUNCE_INTERVAL	(3 * TL_WR1043_V4_KEYS_POLL_INTERVAL)

#define TL_WR1043_V4_MAC_LOCATION		0x1ff50008

#define TL_WR1043_V4_EEPROM_ADDR		0x1fff0000
#define TL_WR1043_V4_WMAC_CALDATA_OFFSET	0x1000

#define TL_WR1043N_V5_MAC_LOCATION		0x1ff00008

static struct gpio_led tl_wr1043nd_v4_leds_gpio[] __initdata = {
	{
		.name		= "tp-link:green:wps",
		.gpio		= TL_WR1043_V4_GPIO_LED_WPS,
		.active_low	= 1,
	},
	{
		.name		= "tp-link:green:system",
		.gpio		= TL_WR1043_V4_GPIO_LED_SYSTEM,
		.active_low	= 1,
	},
	{
		.name		= "tp-link:green:wlan",
		.gpio		= TL_WR1043_V4_GPIO_LED_WLAN,
		.active_low	= 1,
	},
	{
		.name		= "tp-link:green:usb",
		.gpio		= TL_WR1043_V4_GPIO_LED_USB,
		.active_low	= 1,
	},
	{
		.name		= "tp-link:green:wan",
		.gpio		= TL_WR1043_V4_GPIO_LED_WAN,
		.active_low	= 1,
	},
	{
		.name		= "tp-link:green:lan1",
		.gpio		= TL_WR1043_V4_GPIO_LED_LAN1,
		.active_low	= 1,
	},
	{
		.name		= "tp-link:green:lan2",
		.gpio		= TL_WR1043_V4_GPIO_LED_LAN2,
		.active_low	= 1,
	},
	{
		.name		= "tp-link:green:lan3",
		.gpio		= TL_WR1043_V4_GPIO_LED_LAN3,
		.active_low	= 1,
	},
	{
		.name		= "tp-link:green:lan4",
		.gpio		= TL_WR1043_V4_GPIO_LED_LAN4,
		.active_low	= 1,
	},
};

static struct gpio_keys_button tl_wr1043nd_v4_gpio_keys[] __initdata = {
	{
		.desc		= "Reset button",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = TL_WR1043_V4_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= TL_WR1043_V4_GPIO_BTN_RESET,
		.active_low	= 1,
	},
	{
		.desc		= "RFKILL button",
		.type		= EV_KEY,
		.code		= KEY_RFKILL,
		.debounce_interval = TL_WR1043_V4_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= TL_WR1043_V4_GPIO_BTN_RFKILL,
		.active_low	= 1,
	},
};

static struct ar8327_pad_cfg tl_wr1043nd_v4_ar8327_pad0_cfg = {
	.mode = AR8327_PAD_MAC_SGMII,
	.sgmii_delay_en = true,
};

static struct ar8327_platform_data tl_wr1043nd_v4_ar8327_data = {
	.pad0_cfg = &tl_wr1043nd_v4_ar8327_pad0_cfg,
	.port0_cfg = {
		.force_link = 1,
		.speed = AR8327_PORT_SPEED_1000,
		.duplex = 1,
		.txpause = 1,
		.rxpause = 1,
	},
};

static struct mdio_board_info tl_wr1043nd_v4_mdio0_info[] = {
	{
		.bus_id = "ag71xx-mdio.0",
		.phy_addr = 0,
		.platform_data = &tl_wr1043nd_v4_ar8327_data,
	},
};

static void __init tl_wr1043nd_v4_setup(void)
{
	u8 *mac = (u8 *) KSEG1ADDR(TL_WR1043_V4_MAC_LOCATION);
	u8 *eeprom = (u8 *) KSEG1ADDR(TL_WR1043_V4_EEPROM_ADDR);

	ath79_register_m25p80(NULL);

	ath79_init_mac(ath79_eth0_data.mac_addr, mac, 0);
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_SGMII;
	ath79_eth0_data.mii_bus_dev = &ath79_mdio0_device.dev;
	ath79_eth0_data.phy_mask = BIT(0);

	mdiobus_register_board_info(tl_wr1043nd_v4_mdio0_info,
	                            ARRAY_SIZE(tl_wr1043nd_v4_mdio0_info));

	ath79_register_usb();
	ath79_register_mdio(0, 0);
	ath79_register_eth(0);

	ath79_register_wmac(eeprom + TL_WR1043_V4_WMAC_CALDATA_OFFSET, mac);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(tl_wr1043nd_v4_leds_gpio),
	                         tl_wr1043nd_v4_leds_gpio);

	ath79_register_gpio_keys_polled(-1, TL_WR1043_V4_KEYS_POLL_INTERVAL,
	                                ARRAY_SIZE(tl_wr1043nd_v4_gpio_keys),
	                                tl_wr1043nd_v4_gpio_keys);

	gpio_request_one(TL_WR1043_V4_GPIO_USB_POWER,
			 GPIOF_OUT_INIT_HIGH | GPIOF_EXPORT_DIR_FIXED,
			 "USB power");
}

MIPS_MACHINE(ATH79_MACH_TL_WR1043ND_V4, "TL-WR1043ND-v4",
	     "TP-LINK TL-WR1043ND v4", tl_wr1043nd_v4_setup);

static struct gpio_led tl_wr1043n_v5_leds_gpio[] __initdata = {
	{
		.name		= "tp-link:green:wps",
		.gpio		= TL_WR1043_V4_GPIO_LED_WPS,
		.active_low	= 1,
	},
	{
		.name		= "tp-link:green:system",
		.gpio		= TL_WR1043_V4_GPIO_LED_SYSTEM,
		.active_low	= 1,
	},
	{
		.name		= "tp-link:green:wlan",
		.gpio		= TL_WR1043_V4_GPIO_LED_WLAN,
		.active_low	= 1,
	},
	{
		.name		= "tp-link:green:wan",
		.gpio		= TL_WR1043_V4_GPIO_LED_WAN,
		.active_low	= 1,
	},
	{
		.name		= "tp-link:green:lan1",
		.gpio		= TL_WR1043_V4_GPIO_LED_LAN1,
		.active_low	= 1,
	},
	{
		.name		= "tp-link:green:lan2",
		.gpio		= TL_WR1043_V4_GPIO_LED_LAN2,
		.active_low	= 1,
	},
	{
		.name		= "tp-link:green:lan3",
		.gpio		= TL_WR1043_V4_GPIO_LED_LAN3,
		.active_low	= 1,
	},
	{
		.name		= "tp-link:green:lan4",
		.gpio		= TL_WR1043_V4_GPIO_LED_LAN4,
		.active_low	= 1,
	},
};

/* The 1043Nv5 is identical to the 1043NDv4,
 *  only missing the usb and small firmware layout changes  */
static void __init tl_wr1043nv5_setup(void)
{
	u8 *art = (u8 *) KSEG1ADDR(TL_WR1043_V4_EEPROM_ADDR);
	u8 *mac = (u8 *) KSEG1ADDR(TL_WR1043N_V5_MAC_LOCATION);

	ath79_register_m25p80(NULL);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(tl_wr1043n_v5_leds_gpio),
				 tl_wr1043n_v5_leds_gpio);
	ath79_register_gpio_keys_polled(-1, TL_WR1043_V4_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(tl_wr1043nd_v4_gpio_keys),
					tl_wr1043nd_v4_gpio_keys);

	platform_device_register(&ath79_mdio0_device);

	mdiobus_register_board_info(tl_wr1043nd_v4_mdio0_info,
				    ARRAY_SIZE(tl_wr1043nd_v4_mdio0_info));

	ath79_register_wmac(art + TL_WR1043_V4_WMAC_CALDATA_OFFSET, mac);

	ath79_init_mac(ath79_eth0_data.mac_addr, mac, 0);

	/* GMAC0 is connected to an AR8337 switch */
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_SGMII;
	ath79_eth0_data.speed = SPEED_1000;
	ath79_eth0_data.duplex = DUPLEX_FULL;
	ath79_eth0_data.phy_mask = BIT(0);
	ath79_eth0_data.mii_bus_dev = &ath79_mdio0_device.dev;
	ath79_register_eth(0);
}

MIPS_MACHINE(ATH79_MACH_TL_WR1043N_V5, "TL-WR1043N-v5", "TP-LINK TL-WR1043N v5",
	     tl_wr1043nv5_setup);

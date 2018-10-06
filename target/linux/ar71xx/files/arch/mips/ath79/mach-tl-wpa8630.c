/*
 *  TP-Link TL-WPA8630 board support
 *
 *  Copyright (C) 2016 Henryk Heisig <hyniu@o2.pl>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */
#include <linux/platform_device.h>
#include <linux/ath9k_platform.h>
#include <linux/ar8216_platform.h>
#include <asm/mach-ath79/ar71xx_regs.h>

#include "common.h"
#include "dev-m25p80.h"
#include "machtypes.h"
#include "pci.h"
#include "dev-ap9x-pci.h"
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-spi.h"
#include "dev-wmac.h"

#define TL_WPA8630_KEYS_POLL_INTERVAL	20
#define TL_WPA8630_KEYS_DEBOUNCE_INTERVAL	(3 * TL_WPA8630_KEYS_POLL_INTERVAL)

#define TL_WPA8630_GPIO_LED_POWER		1
#define TL_WPA8630_GPIO_LED_LAN		5
#define TL_WPA8630_GPIO_LED_WLAN		19
#define TL_WPA8630_GPIO_LED_WLAN5		21

#define TL_WPA8630_GPIO_BTN_RESET		2
#define TL_WPA8630_GPIO_BTN_RFKILL		8
#define TL_WPA8630_GPIO_BTN_LED		6
#define TL_WPA8630_GPIO_BTN_PAIR		7

#define TL_WPA8630_MAC0_OFFSET		0x0000
#define TL_WPA8630_WMAC_CALDATA_OFFSET	0x1000
#define TL_WPA8630_PCI_CALDATA_OFFSET	0x5000

static const char *tl_wpa8630_part_probes[] = {
	"tp-link-64k",
	NULL,
};

static struct flash_platform_data tl_wpa8630_flash_data = {
	.part_probes	= tl_wpa8630_part_probes,
	.type		= "s25fl064k",
};

static struct gpio_led tl_wpa8630_leds_gpio[] __initdata = {
	{
		.name		= "tl-wpa8630:green:power",
		.gpio		= TL_WPA8630_GPIO_LED_POWER,
		.active_low	= 1,
	},
	{
		.name		= "tl-wpa8630:green:lan",
		.gpio		= TL_WPA8630_GPIO_LED_LAN,
		.active_low	= 1,
	},
	{
		.name		= "tl-wpa8630:green:wlan",
		.gpio		= TL_WPA8630_GPIO_LED_WLAN,
		.active_low	= 1,
	},
	{
		.name		= "tl-wpa8630:green:wlan5",
		.gpio		= TL_WPA8630_GPIO_LED_WLAN5,
		.active_low	= 1,
	},
};

static struct gpio_keys_button tl_wpa8630_gpio_keys[] __initdata = {
	{
		.desc			= "Reset button",
		.type			= EV_KEY,
		.code			= KEY_RESTART,
		.debounce_interval	= TL_WPA8630_KEYS_DEBOUNCE_INTERVAL,
		.gpio			= TL_WPA8630_GPIO_BTN_RESET,
		.active_low		= 1,
	},
	{
		.desc			= "RFKILL button",
		.type			= EV_KEY,
		.code			= KEY_RFKILL,
		.debounce_interval	= TL_WPA8630_KEYS_DEBOUNCE_INTERVAL,
		.gpio			= TL_WPA8630_GPIO_BTN_RFKILL,
		.active_low		= 1,
	},
	{
		.desc			= "LED",
		.type			= EV_KEY,
		.code			= BTN_0,
		.debounce_interval	= TL_WPA8630_KEYS_DEBOUNCE_INTERVAL,
		.gpio			= TL_WPA8630_GPIO_BTN_LED,
		.active_low		= 1,
	},
	{
		.desc			= "Pair",
		.type			= EV_KEY,
		.code			= BTN_1,
		.debounce_interval	= TL_WPA8630_KEYS_DEBOUNCE_INTERVAL,
		.gpio			= TL_WPA8630_GPIO_BTN_PAIR,
		.active_low		= 1,
	},
};

/* GMAC0 of the QCA8337 switch is connected to the QCA9563 SoC via SGMII */
static struct ar8327_pad_cfg tl_wpa8630_qca8337_pad0_cfg = {
	.mode = AR8327_PAD_MAC_SGMII,
	.sgmii_delay_en = true,
};

static struct ar8327_platform_data tl_wpa8630_qca8337_data = {
	.pad0_cfg = &tl_wpa8630_qca8337_pad0_cfg,
	.port0_cfg = {
		.force_link = 1,
		.speed = AR8327_PORT_SPEED_1000,
		.duplex = 1,
		.txpause = 1,
		.rxpause = 1,
	},
};

static struct mdio_board_info tl_wpa8630_mdio0_info[] = {
	{
		.bus_id = "ag71xx-mdio.0",
		.mdio_addr = 0,
		.platform_data = &tl_wpa8630_qca8337_data,
	},
};

static void __init tl_wpa8630_setup(void)
{
	u8 *mac = (u8 *) KSEG1ADDR(0x1f00fc00);
	u8 *art = (u8 *) KSEG1ADDR(0x1fff0000);

	ath79_register_m25p80(&tl_wpa8630_flash_data);

	ath79_init_mac(ath79_eth0_data.mac_addr,
		art + TL_WPA8630_MAC0_OFFSET, 0);

	platform_device_register(&ath79_mdio0_device);

	mdiobus_register_board_info(tl_wpa8630_mdio0_info,
				    ARRAY_SIZE(tl_wpa8630_mdio0_info));

	/* GMAC0 is connected to an AR8337 switch */
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_SGMII;
	ath79_eth0_data.mii_bus_dev = &ath79_mdio0_device.dev;
	ath79_eth0_data.phy_mask = ~BIT(4);
	ath79_init_mac(ath79_eth0_data.mac_addr, mac, 0);

	ath79_register_eth(0);

	ath79_register_wmac(art + TL_WPA8630_WMAC_CALDATA_OFFSET, mac);

	ap91_pci_init(art + TL_WPA8630_PCI_CALDATA_OFFSET, NULL);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(tl_wpa8630_leds_gpio),
				tl_wpa8630_leds_gpio);

	ath79_register_gpio_keys_polled(-1, TL_WPA8630_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(tl_wpa8630_gpio_keys),
					tl_wpa8630_gpio_keys);
}

MIPS_MACHINE(ATH79_MACH_TL_WPA8630, "TL-WPA8630", "TP-LINK TL-WPA8630",
	tl_wpa8630_setup);

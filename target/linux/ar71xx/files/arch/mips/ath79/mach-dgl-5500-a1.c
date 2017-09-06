/*
 *  D-Link DGL-5500 board support
 *
 *  Copyright (C) 2014 Gabor Juhos <juhosg@openwrt.org>
 *  Copyright (C) 2014 Imre Kaloz <kaloz@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/gpio.h>
#include <linux/platform_device.h>
#include <linux/ar8216_platform.h>

#include <asm/mach-ath79/ar71xx_regs.h>

#include "common.h"
#include "pci.h"
#include "dev-gpio-buttons.h"
#include "dev-eth.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-usb.h"
#include "dev-wmac.h"
#include "machtypes.h"

#define DGL_5500_A1_GPIO_LED_POWER_ORANGE	14
#define DGL_5500_A1_GPIO_LED_POWER_GREEN	19
#define DGL_5500_A1_GPIO_LED_PLANET_GREEN	22
#define DGL_5500_A1_GPIO_LED_PLANET_ORANGE	23

#define DGL_5500_A1_GPIO_BTN_WPS		16
#define DGL_5500_A1_GPIO_BTN_RESET		17

#define DGL_5500_A1_KEYS_POLL_INTERVAL		20	/* msecs */
#define DGL_5500_A1_KEYS_DEBOUNCE_INTERVAL	\
					(3 * DGL_5500_A1_KEYS_POLL_INTERVAL)

#define DGL_5500_A1_WMAC_CALDATA_OFFSET		0x1000

#define DGL_5500_A1_LAN_MAC_OFFSET	0x04
#define DGL_5500_A1_WAN_MAC_OFFSET	0x16

static struct gpio_led dgl_5500_a1_leds_gpio[] __initdata = {
	{
		.name		= "d-link:green:power",
		.gpio		= DGL_5500_A1_GPIO_LED_POWER_GREEN,
		.active_low	= 1,
	},
	{
		.name		= "d-link:orange:power",
		.gpio		= DGL_5500_A1_GPIO_LED_POWER_ORANGE,
		.active_low	= 1,
	},
	{
		.name		= "d-link:green:planet",
		.gpio		= DGL_5500_A1_GPIO_LED_PLANET_GREEN,
		.active_low	= 1,
	},
	{
		.name		= "d-link:orange:planet",
		.gpio		= DGL_5500_A1_GPIO_LED_PLANET_ORANGE,
		.active_low	= 1,
	},
};

static struct gpio_keys_button dgl_5500_a1_gpio_keys[] __initdata = {
	{
		.desc		= "Reset button",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = DGL_5500_A1_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= DGL_5500_A1_GPIO_BTN_RESET,
		.active_low	= 1,
	},
	{
		.desc		= "WPS button",
		.type		= EV_KEY,
		.code		= KEY_WPS_BUTTON,
		.debounce_interval = DGL_5500_A1_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= DGL_5500_A1_GPIO_BTN_WPS,
		.active_low	= 1,
	},
};

static struct ar8327_pad_cfg dgl_5500_a1_ar8327_pad0_cfg = {
	/* Use the SGMII interface for the GMAC0 of the AR8327 switch */
	.mode = AR8327_PAD_MAC_SGMII,
	.sgmii_delay_en = true,
};

static struct ar8327_platform_data dgl_5500_a1_ar8327_data = {
	.pad0_cfg = &dgl_5500_a1_ar8327_pad0_cfg,
	.port0_cfg = {
		.force_link = 1,
		.speed = AR8327_PORT_SPEED_1000,
		.duplex = 1,
		.txpause = 1,
		.rxpause = 1,
	},
};

static struct mdio_board_info dgl_5500_a1_mdio0_info[] = {
	{
		.bus_id = "ag71xx-mdio.0",
		.phy_addr = 0,
		.platform_data = &dgl_5500_a1_ar8327_data,
	},
};

static void __init dgl_5500_a1_setup(void)
{
	u8 *mac = (u8 *) KSEG1ADDR(0x1ffe0000);
	u8 *art = (u8 *) KSEG1ADDR(0x1fff0000);
	u8 lan_mac[ETH_ALEN];

	ath79_parse_ascii_mac(mac + DGL_5500_A1_LAN_MAC_OFFSET, lan_mac);

	ath79_register_m25p80(NULL);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(dgl_5500_a1_leds_gpio),
				 dgl_5500_a1_leds_gpio);
	ath79_register_gpio_keys_polled(-1, DGL_5500_A1_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(dgl_5500_a1_gpio_keys),
					dgl_5500_a1_gpio_keys);

	ath79_register_wmac(art + DGL_5500_A1_WMAC_CALDATA_OFFSET, lan_mac);

	ath79_register_mdio(0, 0x0);
	mdiobus_register_board_info(dgl_5500_a1_mdio0_info,
				    ARRAY_SIZE(dgl_5500_a1_mdio0_info));

	ath79_init_mac(ath79_eth1_data.mac_addr, lan_mac, 0);

	/* GMAC1 is connected to an AR8327N switch via the SMGII interface */
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_SGMII;
	ath79_eth1_data.phy_mask = BIT(0);
	ath79_eth1_data.mii_bus_dev = &ath79_mdio0_device.dev;

	ath79_eth1_pll_data.pll_1000 = 0x03000101;

	ath79_register_eth(1);

	ath79_register_usb();
	ath79_register_pci();
}

MIPS_MACHINE(ATH79_MACH_DGL_5500_A1, "DGL-5500-A1", "D-Link DGL-5500 rev. A1",
	     dgl_5500_a1_setup);

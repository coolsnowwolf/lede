/*
 *  D-Link DHP-1565 rev. A1 board support
 *
 *  Copyright (C) 2014 Jacek Kikiewicz
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/pci.h>
#include <linux/phy.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>
#include <linux/ath9k_platform.h>
#include <linux/ar8216_platform.h>

#include <asm/mach-ath79/ar71xx_regs.h>

#include "common.h"
#include "dev-ap9x-pci.h"
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-spi.h"
#include "dev-usb.h"
#include "dev-wmac.h"
#include "machtypes.h"

#define DHP1565A1_GPIO_LED_BLUE_USB		11
#define DHP1565A1_GPIO_LED_AMBER_POWER		14
#define DHP1565A1_GPIO_LED_BLUE_POWER		22
#define DHP1565A1_GPIO_LED_BLUE_WPS		15
#define DHP1565A1_GPIO_LED_AMBER_PLANET		19
#define DHP1565A1_GPIO_LED_BLUE_PLANET		18
#define DHP1565A1_GPIO_LED_WLAN_2G		13

#define DHP1565A1_GPIO_WAN_LED_ENABLE		20

#define DHP1565A1_GPIO_BTN_RESET			17
#define DHP1565A1_GPIO_BTN_WPS			16

#define DHP1565A1_KEYS_POLL_INTERVAL		20	/* msecs */
#define DHP1565A1_KEYS_DEBOUNCE_INTERVAL		(3 * DHP1565A1_KEYS_POLL_INTERVAL)

#define DHP1565A1_MAC0_OFFSET			0xFFA0
#define DHP1565A1_MAC1_OFFSET			0xFFB4
#define DHP1565A1_WMAC0_OFFSET			0x5
#define DHP1565A1_WMAC_CALDATA_OFFSET		0x1000
#define DHP1565A1_PCIE_CALDATA_OFFSET		0x5000

static struct gpio_led dhp1565a1_leds_gpio[] __initdata = {
	{
		.name		= "d-link:amber:power",
		.gpio		= DHP1565A1_GPIO_LED_AMBER_POWER,
		.active_low	= 1,
	},
	{
		.name		= "d-link:green:power",
		.gpio		= DHP1565A1_GPIO_LED_BLUE_POWER,
		.active_low	= 1,
	},
	{
		.name		= "d-link:amber:planet",
		.gpio		= DHP1565A1_GPIO_LED_AMBER_PLANET,
		.active_low	= 1,
	},
	{
		.name		= "d-link:green:planet",
		.gpio		= DHP1565A1_GPIO_LED_BLUE_PLANET,
		.active_low	= 1,
	},
};

static struct gpio_keys_button dhp1565a1_gpio_keys[] __initdata = {
	{
		.desc		= "Soft reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = DHP1565A1_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= DHP1565A1_GPIO_BTN_RESET,
		.active_low	= 1,
	},
	{
		.desc		= "WPS button",
		.type		= EV_KEY,
		.code		= KEY_WPS_BUTTON,
		.debounce_interval = DHP1565A1_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= DHP1565A1_GPIO_BTN_WPS,
		.active_low	= 1,
	},
};

static struct ar8327_pad_cfg dhp1565a1_ar8327_pad0_cfg = {
	.mode = AR8327_PAD_MAC_RGMII,
	.txclk_delay_en = true,
	.rxclk_delay_en = true,
	.txclk_delay_sel = AR8327_CLK_DELAY_SEL1,
	.rxclk_delay_sel = AR8327_CLK_DELAY_SEL2,
};

static struct ar8327_platform_data dhp1565a1_ar8327_data = {
	.pad0_cfg = &dhp1565a1_ar8327_pad0_cfg,
	.port0_cfg = {
		.force_link = 1,
		.speed = AR8327_PORT_SPEED_1000,
		.duplex = 1,
		.txpause = 1,
		.rxpause = 1,
	},
};

static struct mdio_board_info dhp1565a1_mdio0_info[] = {
	{
		.bus_id = "ag71xx-mdio.0",
		.phy_addr = 0,
		.platform_data = &dhp1565a1_ar8327_data,
	},
};

static void __init dhp1565a1_generic_setup(void)
{
	u8 *mac = (u8 *) KSEG1ADDR(0x1ffe0000);
	u8 *art = (u8 *) KSEG1ADDR(0x1fff0000);
	u8 mac0[ETH_ALEN], mac1[ETH_ALEN];
	u8 wmac0[ETH_ALEN];

	ath79_parse_ascii_mac(mac + DHP1565A1_MAC0_OFFSET, mac0);
	ath79_parse_ascii_mac(mac + DHP1565A1_MAC1_OFFSET, mac1);

	ath79_register_m25p80(NULL);

	ath79_register_gpio_keys_polled(-1, DHP1565A1_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(dhp1565a1_gpio_keys),
					dhp1565a1_gpio_keys);

	ath79_init_mac(wmac0, mac0, 0);
	ath79_register_wmac(art + DHP1565A1_WMAC_CALDATA_OFFSET, wmac0);

	ath79_setup_ar934x_eth_cfg(AR934X_ETH_CFG_RGMII_GMAC0);

	mdiobus_register_board_info(dhp1565a1_mdio0_info,
				    ARRAY_SIZE(dhp1565a1_mdio0_info));

	ath79_register_mdio(0, 0x0);

	ath79_init_mac(ath79_eth0_data.mac_addr, mac0, 1);

	/* GMAC0 is connected to an AR8327N switch */
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth0_data.phy_mask = BIT(0);
	ath79_eth0_data.mii_bus_dev = &ath79_mdio0_device.dev;
	ath79_eth0_pll_data.pll_1000 = 0x06000000;
	ath79_register_eth(0);

	ath79_register_usb();
}

static void __init dhp1565a1_setup(void)
{
	ath79_register_leds_gpio(-1, ARRAY_SIZE(dhp1565a1_leds_gpio),
				 dhp1565a1_leds_gpio);

	dhp1565a1_generic_setup();
}

MIPS_MACHINE(ATH79_MACH_DHP_1565_A1, "DHP-1565-A1",
	     "D-Link DHP-1565 rev. A1",
	     dhp1565a1_setup);

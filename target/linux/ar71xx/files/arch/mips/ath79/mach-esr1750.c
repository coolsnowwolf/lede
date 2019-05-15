/*
 *  EnGenius ESR1750 board support
 *
 *  Copyright (c) 2014 Jon Suphammer <jon@suphammer.net>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/platform_device.h>
#include <linux/ar8216_platform.h>

#include <asm/mach-ath79/ar71xx_regs.h>

#include "common.h"
#include "pci.h"
#include "dev-ap9x-pci.h"
#include "dev-gpio-buttons.h"
#include "dev-eth.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-usb.h"
#include "dev-wmac.h"
#include "machtypes.h"
#include "nvram.h"

#define ESR1750_GPIO_LED_WLAN_5G	23
#define ESR1750_GPIO_LED_WLAN_2G	13
#define ESR1750_GPIO_LED_POWER_AMBER	2
#define ESR1750_GPIO_LED_WPS_AMBER	22
#define ESR1750_GPIO_LED_WPS_BLUE	19

#define ESR1750_GPIO_BTN_WPS		16
#define ESR1750_GPIO_BTN_RESET		17

#define ESR1750_KEYS_POLL_INTERVAL	20	/* msecs */
#define ESR1750_KEYS_DEBOUNCE_INTERVAL	(3 * ESR1750_KEYS_POLL_INTERVAL)

#define ESR1750_CALDATA_ADDR 0x1fff0000
#define ESR1750_WMAC_CALDATA_OFFSET	0x1000
#define ESR1750_PCIE_CALDATA_OFFSET	0x5000

#define ESR1750_NVRAM_ADDR	0x1f030000
#define ESR1750_NVRAM_SIZE	0x10000

static struct gpio_led esr1750_leds_gpio[] __initdata = {
	{
		.name		= "esr1750:amber:power",
		.gpio		= ESR1750_GPIO_LED_POWER_AMBER,
		.active_low	= 1,
	},
	{
		.name		= "esr1750:blue:wps",
		.gpio		= ESR1750_GPIO_LED_WPS_BLUE,
		.active_low	= 1,
	},
	{
		.name		= "esr1750:amber:wps",
		.gpio		= ESR1750_GPIO_LED_WPS_AMBER,
		.active_low	= 1,
	},
	{
		.name		= "esr1750:blue:wlan-2g",
		.gpio		= ESR1750_GPIO_LED_WLAN_2G,
		.active_low	= 1,
	},
	{
		.name		= "esr1750:blue:wlan-5g",
		.gpio		= ESR1750_GPIO_LED_WLAN_5G,
		.active_low	= 1,
	}
};

static struct gpio_keys_button esr1750_gpio_keys[] __initdata = {
	{
		.desc		= "WPS button",
		.type		= EV_KEY,
		.code		= KEY_WPS_BUTTON,
		.debounce_interval = ESR1750_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= ESR1750_GPIO_BTN_WPS,
		.active_low	= 1,
	},
	{
		.desc		= "Reset button",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = ESR1750_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= ESR1750_GPIO_BTN_RESET,
		.active_low	= 1,
	},
};

static struct ar8327_pad_cfg esr1750_ar8327_pad0_cfg = {
	.mode = AR8327_PAD_MAC_RGMII,
	.txclk_delay_en = true,
	.rxclk_delay_en = true,
	.txclk_delay_sel = AR8327_CLK_DELAY_SEL2,
	.rxclk_delay_sel = AR8327_CLK_DELAY_SEL2,
};

static struct ar8327_platform_data esr1750_ar8327_data = {
	.pad0_cfg = &esr1750_ar8327_pad0_cfg,
	.port0_cfg = {
		.force_link = 1,
		.speed = AR8327_PORT_SPEED_1000,
		.duplex = 1,
		.txpause = 1,
		.rxpause = 1,
	},
};

static struct mdio_board_info esr1750_mdio0_info[] = {
	{
		.bus_id = "ag71xx-mdio.0",
		.mdio_addr = 0,
		.platform_data = &esr1750_ar8327_data,
	},
};

static int esr1750_get_mac(const char *name, char *mac)
{
	u8 *nvram = (u8 *) KSEG1ADDR(ESR1750_NVRAM_ADDR);
	int err;

	err = ath79_nvram_parse_mac_addr(nvram, ESR1750_NVRAM_SIZE,
					 name, mac);
	if (err) {
		pr_err("no MAC address found for %s\n", name);
		return false;
	}

	return true;
}

static void __init esr1750_setup(void)
{
	u8 *caldata = (u8 *) KSEG1ADDR(ESR1750_CALDATA_ADDR);
	u8 mac1[ETH_ALEN];

	ath79_register_m25p80(NULL);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(esr1750_leds_gpio),
					esr1750_leds_gpio);
	ath79_register_gpio_keys_polled(-1, ESR1750_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(esr1750_gpio_keys),
					esr1750_gpio_keys);

	ath79_register_usb();

	ath79_setup_qca955x_eth_cfg(QCA955X_ETH_CFG_RGMII_EN);

	ath79_register_mdio(0, 0x0);

	mdiobus_register_board_info(esr1750_mdio0_info,
					ARRAY_SIZE(esr1750_mdio0_info));

	/* GMAC0 is connected to an QCA8327N switch */
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth0_data.phy_mask = BIT(0);
	ath79_eth0_data.mii_bus_dev = &ath79_mdio0_device.dev;

	if (esr1750_get_mac("ethaddr=", mac1))
		ath79_init_mac(ath79_eth0_data.mac_addr, mac1, 0);

	ath79_eth0_pll_data.pll_1000 = 0xa6000000;
	ath79_register_eth(0);

	ath79_register_wmac(caldata + ESR1750_WMAC_CALDATA_OFFSET, mac1);

	ath79_register_pci();
}

MIPS_MACHINE(ATH79_MACH_ESR1750, "ESR1750",
	     "EnGenius ESR1750",
	     esr1750_setup);

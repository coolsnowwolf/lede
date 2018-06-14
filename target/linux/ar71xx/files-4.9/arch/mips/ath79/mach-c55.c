/*
 *  AirTight Networks C-55 board support
 *
 *  Copyright (C) 2014-2015 Chris Blake <chrisrblake93@gmail.com>
 *
 *  Based on Senao CAP4200AG board support
 *
 *  Copyright (C) 2012 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/pci.h>
#include <linux/phy.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/mtd/physmap.h>
#include <linux/platform_device.h>
#include <linux/ath9k_platform.h>

#include <asm/mach-ath79/ar71xx_regs.h>

#include "common.h"
#include "pci.h"
#include "dev-ap9x-pci.h"
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-spi.h"
#include "dev-wmac.h"
#include "machtypes.h"

#define C55_GPIO_LED_PWR_GREEN	12
#define C55_GPIO_LED_PWR_AMBER	13
#define C55_GPIO_LED_LAN_GREEN	14
#define C55_GPIO_LED_LAN_AMBER	15
#define C55_GPIO_LED_WLAN_GREEN	18
#define C55_GPIO_LED_WLAN_AMBER	19

#define C55_GPIO_BTN_RESET	17

#define C55_KEYS_POLL_INTERVAL	20	/* msecs */
#define C55_KEYS_DEBOUNCE_INTERVAL (3 * C55_KEYS_POLL_INTERVAL)

#define C55_MAC_OFFSET		0
#define C55_WMAC_CALDATA_OFFSET	0x1000
#define C55_PCIE_CALDATA_OFFSET	0x5000

static struct gpio_led c55_leds_gpio[] __initdata = {
	{
		.name		= "c-55:green:pwr",
		.gpio		= C55_GPIO_LED_PWR_GREEN,
		.active_low	= 1,
	},
	{
		.name		= "c-55:amber:pwr",
		.gpio		= C55_GPIO_LED_PWR_AMBER,
		.active_low	= 1,
	},
	{
		.name		= "c-55:green:lan",
		.gpio		= C55_GPIO_LED_LAN_GREEN,
		.active_low	= 1,
	},
	{
		.name		= "c-55:amber:lan",
		.gpio		= C55_GPIO_LED_LAN_AMBER,
		.active_low	= 1,
	},
	{
		.name		= "c-55:green:wlan",
		.gpio		= C55_GPIO_LED_WLAN_GREEN,
		.active_low	= 1,
	},
	{
		.name		= "c-55:amber:wlan",
		.gpio		= C55_GPIO_LED_WLAN_AMBER,
		.active_low	= 1,
	},
};

static struct gpio_keys_button c55_gpio_keys[] __initdata = {
	{
		.desc		= "Reset button",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = C55_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= C55_GPIO_BTN_RESET,
		.active_low	= 1,
	},
};

static void __init c55_setup(void)
{
	/* SPI Storage*/
	ath79_register_m25p80(NULL);

	/* MDIO Interface */
	ath79_register_mdio(0, 0x0);

	/* AR8035-A Ethernet */
	ath79_setup_ar934x_eth_cfg(AR934X_ETH_CFG_RGMII_GMAC0 |
				   AR934X_ETH_CFG_SW_ONLY_MODE);
	ath79_init_mac(ath79_eth0_data.mac_addr, NULL, 0);
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth0_data.phy_mask = BIT(0);
	ath79_eth0_data.mii_bus_dev = &ath79_mdio0_device.dev;
	ath79_eth0_pll_data.pll_1000 = 0x06000000;
	ath79_register_eth(0);

	/* LEDs & GPIO */
	ath79_gpio_output_select(C55_GPIO_LED_LAN_GREEN,
				 AR934X_GPIO_OUT_GPIO);
	ath79_gpio_output_select(C55_GPIO_LED_LAN_AMBER,
				 AR934X_GPIO_OUT_GPIO);
	ath79_register_leds_gpio(-1, ARRAY_SIZE(c55_leds_gpio),
				 c55_leds_gpio);
	ath79_register_gpio_keys_polled(-1, C55_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(c55_gpio_keys),
					c55_gpio_keys);

	/* WiFi */
	ath79_wmac_disable_2ghz();
	ath79_register_wmac_simple();
	ap91_pci_init_simple();

}
MIPS_MACHINE(ATH79_MACH_C55, "C-55", "AirTight Networks C-55",
	     c55_setup);

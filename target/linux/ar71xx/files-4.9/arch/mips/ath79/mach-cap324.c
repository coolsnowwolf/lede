/*
 *  PowerCloud Systems CAP324 board support
 *
 *  Copyright (C) 2012 Gabor Juhos <juhosg@openwrt.org>
 *  Copyright (C) 2012-2013 PowerCloud Systems
 *  Copyright (C) 2015 Daniel Dickinson
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/pci.h>
#include <linux/phy.h>
#include <linux/platform_device.h>
#include <linux/ath9k_platform.h>

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

#define CAP324_GPIO_LED_POWER_GREEN	12
#define CAP324_GPIO_LED_POWER_AMBER	13
#define CAP324_GPIO_LED_LAN_GREEN	14
#define CAP324_GPIO_LED_LAN_AMBER	15
#define CAP324_GPIO_LED_WLAN_GREEN	18
#define CAP324_GPIO_LED_WLAN_AMBER	19

#define CAP324_GPIO_BTN_RESET	17

#define CAP324_KEYS_POLL_INTERVAL	20	/* msecs */
#define CAP324_KEYS_DEBOUNCE_INTERVAL (3 * CAP324_KEYS_POLL_INTERVAL)

#define CAP324_MAC_OFFSET		0
#define CAP324_WMAC_CALDATA_OFFSET	0x1000
#define CAP324_PCIE_CALDATA_OFFSET	0x5000

static struct gpio_led cap324_leds_gpio[] __initdata = {
	{
		.name		= "pcs:green:power",
		.gpio		= CAP324_GPIO_LED_POWER_GREEN,
		.active_low	= 1,
	},
	{
		.name		= "pcs:amber:power",
		.gpio		= CAP324_GPIO_LED_POWER_AMBER,
		.active_low	= 1,
	},
	{
		.name		= "pcs:green:lan",
		.gpio		= CAP324_GPIO_LED_LAN_GREEN,
		.active_low	= 1,
	},
	{
		.name		= "pcs:amber:lan",
		.gpio		= CAP324_GPIO_LED_LAN_AMBER,
		.active_low	= 1,
	},
	{
		.name		= "pcs:green:wlan",
		.gpio		= CAP324_GPIO_LED_WLAN_GREEN,
		.active_low	= 1,
	},
	{
		.name		= "pcs:amber:wlan",
		.gpio		= CAP324_GPIO_LED_WLAN_AMBER,
		.active_low	= 1,
	},
};

static struct gpio_keys_button cap324_gpio_keys[] __initdata = {
	{
		.desc		= "Reset button",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = CAP324_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= CAP324_GPIO_BTN_RESET,
		.active_low	= 1,
	},
};

static void __init cap324_setup(void)
{
	u8 *art = (u8 *) KSEG1ADDR(0x1fff0000);
	u8 mac[6];

	ath79_gpio_output_select(CAP324_GPIO_LED_LAN_GREEN,
				 AR934X_GPIO_OUT_GPIO);
	ath79_gpio_output_select(CAP324_GPIO_LED_LAN_AMBER,
				 AR934X_GPIO_OUT_GPIO);

	ath79_register_m25p80(NULL);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(cap324_leds_gpio),
				 cap324_leds_gpio);
	ath79_register_gpio_keys_polled(-1, CAP324_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(cap324_gpio_keys),
					cap324_gpio_keys);

	ath79_init_mac(mac, art + CAP324_MAC_OFFSET, -1);
	ath79_wmac_disable_2ghz();
	ath79_register_wmac(art + CAP324_WMAC_CALDATA_OFFSET, mac);

	ath79_init_mac(mac, art + CAP324_MAC_OFFSET, -2);
	ap91_pci_init(art + CAP324_PCIE_CALDATA_OFFSET, mac);

	ath79_setup_ar934x_eth_cfg(AR934X_ETH_CFG_RGMII_GMAC0 |
				   AR934X_ETH_CFG_SW_ONLY_MODE);

	ath79_register_mdio(0, 0x0);

	ath79_init_mac(ath79_eth0_data.mac_addr,
		       art + CAP324_MAC_OFFSET, -2);

	/* GMAC0 is connected to an external PHY */
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth0_data.phy_mask = BIT(0);
	ath79_eth0_data.mii_bus_dev = &ath79_mdio0_device.dev;
	ath79_eth0_pll_data.pll_1000 = 0x06000000;
	ath79_register_eth(0);
}

MIPS_MACHINE(ATH79_MACH_CAP324, "CAP324", "PowerCloud CAP324",
	     cap324_setup);

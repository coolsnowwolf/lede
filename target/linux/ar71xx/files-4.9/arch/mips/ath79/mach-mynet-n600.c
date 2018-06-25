/*
 *  WD My Net N600 board support
 *
 *  Copyright (C) 2013 Gabor Juhos <juhosg@openwrt.org>
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
#include "nvram.h"

#define MYNET_N600_GPIO_LED_WIFI	0
#define MYNET_N600_GPIO_LED_POWER	11
#define MYNET_N600_GPIO_LED_INTERNET	12
#define MYNET_N600_GPIO_LED_WPS		13

#define MYNET_N600_GPIO_LED_LAN1	4
#define MYNET_N600_GPIO_LED_LAN2	3
#define MYNET_N600_GPIO_LED_LAN3	2
#define MYNET_N600_GPIO_LED_LAN4	1

#define MYNET_N600_GPIO_BTN_RESET	16
#define MYNET_N600_GPIO_BTN_WPS		17

#define MYNET_N600_GPIO_EXTERNAL_LNA0	14
#define MYNET_N600_GPIO_EXTERNAL_LNA1	15

#define MYNET_N600_KEYS_POLL_INTERVAL	20	/* msecs */
#define MYNET_N600_KEYS_DEBOUNCE_INTERVAL (3 * MYNET_N600_KEYS_POLL_INTERVAL)

#define MYNET_N600_MAC0_OFFSET		0
#define MYNET_N600_MAC1_OFFSET		6
#define MYNET_N600_WMAC_CALDATA_OFFSET	0x1000
#define MYNET_N600_PCIE_CALDATA_OFFSET	0x5000

#define MYNET_N600_NVRAM_ADDR		0x1f058010
#define MYNET_N600_NVRAM_SIZE		0x7ff0

static struct gpio_led mynet_n600_leds_gpio[] __initdata = {
	{
		.name		= "wd:blue:power",
		.gpio		= MYNET_N600_GPIO_LED_POWER,
		.active_low	= 0,
	},
	{
		.name		= "wd:blue:wps",
		.gpio		= MYNET_N600_GPIO_LED_WPS,
		.active_low	= 0,
	},
	{
		.name		= "wd:blue:wireless",
		.gpio		= MYNET_N600_GPIO_LED_WIFI,
		.active_low	= 0,
	},
	{
		.name		= "wd:blue:internet",
		.gpio		= MYNET_N600_GPIO_LED_INTERNET,
		.active_low	= 0,
	},
	{
		.name		= "wd:green:lan1",
		.gpio		= MYNET_N600_GPIO_LED_LAN1,
		.active_low	= 1,
	},
	{
		.name		= "wd:green:lan2",
		.gpio		= MYNET_N600_GPIO_LED_LAN2,
		.active_low	= 1,
	},
	{
		.name		= "wd:green:lan3",
		.gpio		= MYNET_N600_GPIO_LED_LAN3,
		.active_low	= 1,
	},
	{
		.name		= "wd:green:lan4",
		.gpio		= MYNET_N600_GPIO_LED_LAN4,
		.active_low	= 1,
	},
};

static struct gpio_keys_button mynet_n600_gpio_keys[] __initdata = {
	{
		.desc		= "Reset button",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = MYNET_N600_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= MYNET_N600_GPIO_BTN_RESET,
		.active_low	= 1,
	},
	{
		.desc		= "WPS button",
		.type		= EV_KEY,
		.code		= KEY_WPS_BUTTON,
		.debounce_interval = MYNET_N600_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= MYNET_N600_GPIO_BTN_WPS,
		.active_low	= 1,
	},
};

static void mynet_n600_get_mac(const char *name, char *mac)
{
	u8 *nvram = (u8 *) KSEG1ADDR(MYNET_N600_NVRAM_ADDR);
	int err;

	err = ath79_nvram_parse_mac_addr(nvram, MYNET_N600_NVRAM_SIZE,
					 name, mac);
	if (err)
		pr_err("no MAC address found for %s\n", name);
}

#define MYNET_N600_WAN_PHY_MASK	BIT(0)

static void __init mynet_n600_setup(void)
{
	u8 *art = (u8 *) KSEG1ADDR(0x1fff0000);
	u8 tmpmac[ETH_ALEN];

	ath79_register_m25p80(NULL);

	ath79_gpio_output_select(MYNET_N600_GPIO_LED_LAN1,
				 AR934X_GPIO_OUT_GPIO);
	ath79_gpio_output_select(MYNET_N600_GPIO_LED_LAN2,
				 AR934X_GPIO_OUT_GPIO);
	ath79_gpio_output_select(MYNET_N600_GPIO_LED_LAN3,
				 AR934X_GPIO_OUT_GPIO);
	ath79_gpio_output_select(MYNET_N600_GPIO_LED_LAN4,
				 AR934X_GPIO_OUT_GPIO);
	ath79_gpio_output_select(MYNET_N600_GPIO_LED_INTERNET,
				 AR934X_GPIO_OUT_GPIO);
	ath79_register_leds_gpio(-1, ARRAY_SIZE(mynet_n600_leds_gpio),
				 mynet_n600_leds_gpio);

	ath79_register_gpio_keys_polled(-1, MYNET_N600_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(mynet_n600_gpio_keys),
					mynet_n600_gpio_keys);

	/*
	 * Control signal for external LNAs 0 and 1
	 * Taken from GPL bootloader source:
	 *   board/ar7240/db12x/alpha_gpio.c
	 */
	ath79_wmac_set_ext_lna_gpio(0, MYNET_N600_GPIO_EXTERNAL_LNA0);
	ath79_wmac_set_ext_lna_gpio(1, MYNET_N600_GPIO_EXTERNAL_LNA1);

	mynet_n600_get_mac("wlan24mac=", tmpmac);
	ath79_register_wmac(art + MYNET_N600_WMAC_CALDATA_OFFSET, tmpmac);

	mynet_n600_get_mac("wlan5mac=", tmpmac);
	ap91_pci_init(art + MYNET_N600_PCIE_CALDATA_OFFSET, tmpmac);

	ath79_setup_ar934x_eth_cfg(AR934X_ETH_CFG_SW_ONLY_MODE |
				   AR934X_ETH_CFG_SW_PHY_SWAP);

	ath79_register_mdio(1, 0x0);

	/* LAN */
	mynet_n600_get_mac("lanmac=", ath79_eth1_data.mac_addr);

	/* GMAC1 is connected to the internal switch */
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_GMII;

	ath79_register_eth(1);

	/* WAN */
	mynet_n600_get_mac("wanmac=", ath79_eth0_data.mac_addr);

	/* GMAC0 is connected to the PHY4 of the internal switch */
	ath79_switch_data.phy4_mii_en = 1;
	ath79_switch_data.phy_poll_mask = MYNET_N600_WAN_PHY_MASK;

	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ath79_eth0_data.phy_mask = MYNET_N600_WAN_PHY_MASK;
	ath79_eth0_data.mii_bus_dev = &ath79_mdio1_device.dev;

	ath79_register_eth(0);

	ath79_register_usb();
}

MIPS_MACHINE(ATH79_MACH_MYNET_N600, "MYNET-N600", "WD My Net N600",
	     mynet_n600_setup);

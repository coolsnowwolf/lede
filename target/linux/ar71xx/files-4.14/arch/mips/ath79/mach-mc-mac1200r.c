/*
 *  MERCURY MAC1200R board support
 *
 *  Copyright (C) 2012 Gabor Juhos <juhosg@openwrt.org>
 *  Copyright (C) 2013 Gui Iribarren <gui@altermundi.net>
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
#include "dev-wmac.h"
#include "machtypes.h"

#define MAC1200R_GPIO_LED_WLAN2G	13
#define MAC1200R_GPIO_LED_WLAN5G	17
#define MAC1200R_GPIO_LED_SYSTEM	14
#define MAC1200R_GPIO_LED_WPS		11
#define MAC1200R_GPIO_LED_WAN		12
#define MAC1200R_GPIO_LED_LAN1		15
#define MAC1200R_GPIO_LED_LAN2		21
#define MAC1200R_GPIO_LED_LAN3		22
#define MAC1200R_GPIO_LED_LAN4		20

#define MAC1200R_GPIO_BTN_WPS		16

#define MAC1200R_KEYS_POLL_INTERVAL	20	/* msecs */
#define MAC1200R_KEYS_DEBOUNCE_INTERVAL	(3 * MAC1200R_KEYS_POLL_INTERVAL)

#define MAC1200R_MAC0_OFFSET		0
#define MAC1200R_MAC1_OFFSET		6
#define MAC1200R_WMAC_CALDATA_OFFSET	0x1000
#define MAC1200R_PCIE_CALDATA_OFFSET	0x5000

static const char *mac1200r_part_probes[] = {
	"tp-link",
	NULL,
};

static struct flash_platform_data mac1200r_flash_data = {
	.part_probes	= mac1200r_part_probes,
};

static struct gpio_led mac1200r_leds_gpio[] __initdata = {
	{
		.name		= "mercury:green:wps",
		.gpio		= MAC1200R_GPIO_LED_WPS,
		.active_low	= 1,
	},
	{
		.name		= "mercury:green:system",
		.gpio		= MAC1200R_GPIO_LED_SYSTEM,
		.active_low	= 1,
	},
	{
		.name		= "mercury:green:wlan2g",
		.gpio		= MAC1200R_GPIO_LED_WLAN2G,
		.active_low	= 1,
	},
	{
		.name		= "mercury:green:wlan5g",
		.gpio		= MAC1200R_GPIO_LED_WLAN5G,
		.active_low	= 1,
	},
};

static struct gpio_keys_button mac1200r_gpio_keys[] __initdata = {
	{
		.desc		= "Reset button",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = MAC1200R_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= MAC1200R_GPIO_BTN_WPS,
		.active_low	= 1,
	},
};


static void __init mac1200r_setup(void)
{
	u8 *mac = (u8 *) KSEG1ADDR(0x1f01fc00);
	u8 *art = (u8 *) KSEG1ADDR(0x1fff0000);
	u8 tmpmac[ETH_ALEN];

	ath79_register_m25p80(&mac1200r_flash_data);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(mac1200r_leds_gpio),
					mac1200r_leds_gpio);

	ath79_register_gpio_keys_polled(-1, MAC1200R_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(mac1200r_gpio_keys),
					mac1200r_gpio_keys);

	ath79_init_mac(tmpmac, mac, 0);
	ath79_wmac_disable_5ghz();
	ath79_register_wmac(art + MAC1200R_WMAC_CALDATA_OFFSET, tmpmac);

	ath79_init_mac(tmpmac, mac, 1);
	ap91_pci_init(art + MAC1200R_PCIE_CALDATA_OFFSET, tmpmac);

	ath79_setup_ar934x_eth_cfg(AR934X_ETH_CFG_SW_ONLY_MODE);

	ath79_register_mdio(1, 0x0);

	/* LAN */
	ath79_init_mac(ath79_eth1_data.mac_addr, mac, -1);

	/* GMAC1 is connected to the internal switch */
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_GMII;

	ath79_register_eth(1);

	/* WAN */
	ath79_init_mac(ath79_eth0_data.mac_addr, mac, 2);

	/* GMAC0 is connected to the PHY4 of the internal switch */
	ath79_switch_data.phy4_mii_en = 1;
	ath79_switch_data.phy_poll_mask = BIT(4);
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ath79_eth0_data.phy_mask = BIT(4);
	ath79_eth0_data.mii_bus_dev = &ath79_mdio1_device.dev;

	ath79_register_eth(0);

	ath79_gpio_output_select(MAC1200R_GPIO_LED_LAN1,
				 AR934X_GPIO_OUT_LED_LINK3);
	ath79_gpio_output_select(MAC1200R_GPIO_LED_LAN2,
				 AR934X_GPIO_OUT_LED_LINK2);
	ath79_gpio_output_select(MAC1200R_GPIO_LED_LAN3,
				 AR934X_GPIO_OUT_LED_LINK1);
	ath79_gpio_output_select(MAC1200R_GPIO_LED_LAN4,
				 AR934X_GPIO_OUT_LED_LINK0);
	ath79_gpio_output_select(MAC1200R_GPIO_LED_WAN,
				 AR934X_GPIO_OUT_LED_LINK4);
}

MIPS_MACHINE(ATH79_MACH_MC_MAC1200R, "MC-MAC1200R",
	     "MERCURY MAC1200R",
	     mac1200r_setup);

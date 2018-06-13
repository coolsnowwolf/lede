/*
 *  TP-LINK TL-WDR3320 v2 board support
 *
 *  Copyright (C) 2012 Gabor Juhos <juhosg@openwrt.org>
 *  Copyright (C) 2015 Weijie Gao <hackpascal@gmail.com>
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

#define WDR3320_GPIO_LED_WLAN5G		12
#define WDR3320_GPIO_LED_SYSTEM		14
#define WDR3320_GPIO_LED_QSS		15
#define WDR3320_GPIO_LED_WAN		4
#define WDR3320_GPIO_LED_LAN1		18
#define WDR3320_GPIO_LED_LAN2		20
#define WDR3320_GPIO_LED_LAN3		21
#define WDR3320_GPIO_LED_LAN4		22

#define WDR3320_GPIO_BTN_RESET		16

#define WDR3320_KEYS_POLL_INTERVAL	20	/* msecs */
#define WDR3320_KEYS_DEBOUNCE_INTERVAL	(3 * WDR3320_KEYS_POLL_INTERVAL)

#define WDR3320_WMAC_CALDATA_OFFSET	0x1000
#define WDR3320_PCIE_CALDATA_OFFSET	0x5000

static const char *wdr3320_part_probes[] = {
	"tp-link",
	NULL,
};

static struct flash_platform_data wdr3320_flash_data = {
	.part_probes	= wdr3320_part_probes,
};

static struct gpio_led wdr3320_leds_gpio[] __initdata = {
	{
		.name		= "tp-link:green:qss",
		.gpio		= WDR3320_GPIO_LED_QSS,
		.active_low	= 1,
	},
	{
		.name		= "tp-link:green:system",
		.gpio		= WDR3320_GPIO_LED_SYSTEM,
		.active_low	= 1,
	},
	{
		.name		= "tp-link:green:wlan5g",
		.gpio		= WDR3320_GPIO_LED_WLAN5G,
		.active_low	= 1,
	},
};

static struct gpio_keys_button wdr3320_gpio_keys[] __initdata = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = WDR3320_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= WDR3320_GPIO_BTN_RESET,
		.active_low	= 1,
	},
};

static void __init wdr3320_setup(void)
{
	u8 *mac = (u8 *) KSEG1ADDR(0x1f01fc00);
	u8 *art = (u8 *) KSEG1ADDR(0x1fff0000);
	u8 tmpmac[ETH_ALEN];

	ath79_register_m25p80(&wdr3320_flash_data);
	ath79_register_leds_gpio(-1, ARRAY_SIZE(wdr3320_leds_gpio),
				 wdr3320_leds_gpio);
	ath79_register_gpio_keys_polled(-1, WDR3320_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(wdr3320_gpio_keys),
					wdr3320_gpio_keys);

	ath79_init_mac(tmpmac, mac, 0);
	ath79_register_wmac(art + WDR3320_WMAC_CALDATA_OFFSET, tmpmac);

	ath79_init_mac(tmpmac, mac, -1);
	ap9x_pci_setup_wmac_led_pin(0, 0);
	ap91_pci_init(art + WDR3320_PCIE_CALDATA_OFFSET, tmpmac);

	ath79_setup_ar934x_eth_cfg(AR934X_ETH_CFG_SW_ONLY_MODE);

	ath79_register_mdio(1, 0x0);

	/* LAN */
	ath79_init_mac(ath79_eth1_data.mac_addr, mac, 0);

	/* GMAC1 is connected to the internal switch */
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_GMII;

	ath79_register_eth(1);

	/* WAN */
	ath79_init_mac(ath79_eth0_data.mac_addr, mac, 1);

	/* GMAC0 is connected to the PHY4 of the internal switch */
	ath79_switch_data.phy4_mii_en = 1;
	ath79_switch_data.phy_poll_mask = BIT(4);
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ath79_eth0_data.phy_mask = BIT(4);
	ath79_eth0_data.mii_bus_dev = &ath79_mdio1_device.dev;

	ath79_register_eth(0);

	ath79_register_usb();

	ath79_gpio_output_select(WDR3320_GPIO_LED_LAN1,
				 AR934X_GPIO_OUT_LED_LINK0);
	ath79_gpio_output_select(WDR3320_GPIO_LED_LAN2,
				 AR934X_GPIO_OUT_LED_LINK1);
	ath79_gpio_output_select(WDR3320_GPIO_LED_LAN3,
				 AR934X_GPIO_OUT_LED_LINK2);
	ath79_gpio_output_select(WDR3320_GPIO_LED_LAN4,
				 AR934X_GPIO_OUT_LED_LINK3);
	ath79_gpio_output_select(WDR3320_GPIO_LED_WAN,
				 AR934X_GPIO_OUT_LED_LINK4);
}

MIPS_MACHINE(ATH79_MACH_TL_WDR3320_V2, "TL-WDR3320-v2",
	     "TP-LINK TL-WDR3320 v2",
	     wdr3320_setup);

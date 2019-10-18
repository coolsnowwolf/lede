/*
 *  TP-LINK TL-WDR3227 board support
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
#include <linux/platform_data/phy-at803x.h>
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

#define WDR3227_GPIO_LED_USB		11
#define WDR3227_GPIO_LED_WLAN2G		13
#define WDR3227_GPIO_LED_SYSTEM		14
#define WDR3227_GPIO_LED_QSS		15
#define WDR3227_GPIO_LED_WAN		21
#define WDR3227_GPIO_LED_LAN1		19
#define WDR3227_GPIO_LED_LAN2		20
#define WDR3227_GPIO_LED_LAN3		18
#define WDR3227_GPIO_LED_LAN4		22
#define WDR3227_GPIO_BTN_RESET		16

#define WDR3227_GPIO_BTN_RFKILL		17

#define WDR3227_GPIO_USB_POWER		12

#define WDR3227_KEYS_POLL_INTERVAL	20	/* msecs */
#define WDR3227_KEYS_DEBOUNCE_INTERVAL	(3 * WDR3227_KEYS_POLL_INTERVAL)

#define WDR3227_MAC0_OFFSET		0
#define WDR3227_MAC1_OFFSET		6
#define WDR3227_WMAC_CALDATA_OFFSET	0x1000
#define WDR3227_PCIE_CALDATA_OFFSET	0x5000

static const char *WDR3227_part_probes[] = {
	"tp-link",
	NULL,
};

static struct flash_platform_data WDR3227_flash_data = {
	.part_probes	= WDR3227_part_probes,
};

static struct gpio_led WDR3227_leds_gpio[] __initdata = {
	{
		.name		= "tp-link:green:qss",
		.gpio		= WDR3227_GPIO_LED_QSS,
		.active_low	= 1,
	},
	{
		.name		= "tp-link:green:system",
		.gpio		= WDR3227_GPIO_LED_SYSTEM,
		.active_low	= 1,
	},
	{
		.name		= "tp-link:green:usb",
		.gpio		= WDR3227_GPIO_LED_USB,
		.active_low	= 1,
	},
	{
		.name		= "tp-link:green:wlan2g",
		.gpio		= WDR3227_GPIO_LED_WLAN2G,
		.active_low	= 1,
	},
};

static struct gpio_keys_button WDR3227_gpio_keys[] __initdata = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = WDR3227_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= WDR3227_GPIO_BTN_RESET,
		.active_low	= 1,
	},
	{
		.desc		= "RFKILL switch",
		.type		= EV_SW,
		.code		= KEY_RFKILL,
		.debounce_interval = WDR3227_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= WDR3227_GPIO_BTN_RFKILL,
	},
};

static struct at803x_platform_data WDR3227_ar8035_data = {
        .enable_rgmii_tx_delay = 1,
        .enable_rgmii_rx_delay = 1,
};

static struct mdio_board_info WDR3227_mdio0_info[] = {
        {
                .bus_id = "ag71xx-mdio.0",
                .phy_addr = 0,
                .platform_data = &WDR3227_ar8035_data,
        },
};

static void __init WDR3227_setup(void)
{
	u8 *mac = (u8 *) KSEG1ADDR(0x1f01fc00);
	u8 *art = (u8 *) KSEG1ADDR(0x1fff0000);
	u8 tmpmac[ETH_ALEN];

	ath79_register_m25p80(&WDR3227_flash_data);
	ath79_register_leds_gpio(-1, ARRAY_SIZE(WDR3227_leds_gpio),
				 WDR3227_leds_gpio);
	ath79_register_gpio_keys_polled(-1, WDR3227_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(WDR3227_gpio_keys),
					WDR3227_gpio_keys);

	ath79_init_mac(tmpmac, mac, 0);
	ath79_register_wmac(art + WDR3227_WMAC_CALDATA_OFFSET, tmpmac);

	ath79_init_mac(tmpmac, mac, 1);
	ap9x_pci_setup_wmac_led_pin(0, 0);
	ap91_pci_init(art + WDR3227_PCIE_CALDATA_OFFSET, tmpmac);

        ath79_register_mdio(0, 0x0);

        mdiobus_register_board_info(WDR3227_mdio0_info, ARRAY_SIZE(WDR3227_mdio0_info));
	ath79_init_mac(ath79_eth0_data.mac_addr, mac, 1);
	ath79_setup_ar934x_eth_cfg(AR934X_ETH_CFG_RGMII_GMAC0);
        /* GMAC0 is connected to an AR8035 Gigabit PHY */
        ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
        ath79_eth0_data.phy_mask = BIT(0);
        ath79_eth0_data.mii_bus_dev = &ath79_mdio0_device.dev;
        ath79_eth0_pll_data.pll_1000 = 0x0e000000;
        ath79_eth0_pll_data.pll_100 = 0x0101;
        ath79_eth0_pll_data.pll_10 = 0x1313;
        ath79_register_eth(0);
#if 0
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
#endif
	ath79_gpio_output_select(WDR3227_GPIO_LED_WAN,
				 AR934X_GPIO_OUT_LED_LINK4);
	gpio_request_one(WDR3227_GPIO_USB_POWER,
			 GPIOF_OUT_INIT_HIGH | GPIOF_EXPORT_DIR_FIXED,
			 "USB power");
	ath79_register_usb();
}

MIPS_MACHINE(ATH79_MACH_TL_WDR3227_V2, "TL-WDR3227-v2",
	     "TP-LINK TL-WDR3227 v2",
	     WDR3227_setup);

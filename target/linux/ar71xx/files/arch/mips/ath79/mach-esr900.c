/*
 *  EnGenius ESR900 board support
 *
 *  Copyright (C) 2008-2012 Gabor Juhos <juhosg@openwrt.org>
 *  Copyright (C) 2008 Imre Kaloz <kaloz@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#define pr_fmt(fmt) "esr900: " fmt

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

#define ESR900_GPIO_LED_POWER           2
#define ESR900_GPIO_LED_WLAN_2G         13
#define ESR900_GPIO_LED_WPS_BLUE        19
#define ESR900_GPIO_LED_WPS_AMBER       22
#define ESR900_GPIO_LED_WLAN_5G         23

#define ESR900_GPIO_BTN_WPS             16
#define ESR900_GPIO_BTN_RESET           17

#define ESR900_KEYS_POLL_INTERVAL       20 /* msecs */
#define ESR900_KEYS_DEBOUNCE_INTERVAL   (3 * ESR900_KEYS_POLL_INTERVAL)

#define ESR900_CALDATA_ADDR             0x1fff0000
#define ESR900_WMAC_CALDATA_OFFSET      0x1000
#define ESR900_PCIE_CALDATA_OFFSET      0x5000

#define ESR900_CONFIG_ADDR              0x1f030000
#define ESR900_CONFIG_SIZE              0x10000

#define ESR900_LAN_PHYMASK              BIT(0)
#define ESR900_WAN_PHYMASK              BIT(5)
#define ESR900_MDIO_MASK                (~(ESR900_LAN_PHYMASK | ESR900_WAN_PHYMASK))

static struct gpio_led esr900_leds_gpio[] __initdata = {
	{
		.name		= "engenius:amber:power",
		.gpio		= ESR900_GPIO_LED_POWER,
		.active_low	= 1,
	},
	{
		.name		= "engenius:blue:wlan-2g",
		.gpio		= ESR900_GPIO_LED_WLAN_2G,
		.active_low	= 1,
	},
	{
		.name		= "engenius:blue:wps",
		.gpio		= ESR900_GPIO_LED_WPS_BLUE,
		.active_low	= 1,
	},
	{
		.name		= "engenius:amber:wps",
		.gpio		= ESR900_GPIO_LED_WPS_AMBER,
		.active_low	= 1,
	},
	{
		.name		= "engenius:blue:wlan-5g",
		.gpio		= ESR900_GPIO_LED_WLAN_5G,
		.active_low	= 1,
	}
};

static struct gpio_keys_button esr900_gpio_keys[] __initdata = {
	{
		.desc		= "WPS button",
		.type		= EV_KEY,
		.code		= KEY_WPS_BUTTON,
		.debounce_interval = ESR900_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= ESR900_GPIO_BTN_WPS,
		.active_low	= 1,
	},
	{
		.desc		= "Reset button",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = ESR900_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= ESR900_GPIO_BTN_RESET,
		.active_low	= 1,
	},
};

static struct ar8327_pad_cfg esr900_ar8327_pad0_cfg = {
	/* GMAC0 of the AR8337 switch is connected to GMAC0 via RGMII */
	.mode = AR8327_PAD_MAC_RGMII,
	.txclk_delay_en = true,
	.rxclk_delay_en = true,
	.txclk_delay_sel = AR8327_CLK_DELAY_SEL1,
	.rxclk_delay_sel = AR8327_CLK_DELAY_SEL2,
};

static struct ar8327_pad_cfg esr900_ar8327_pad6_cfg = {
	/* GMAC6 of the AR8337 switch is connected to GMAC1 via SGMII */
	.mode = AR8327_PAD_MAC_SGMII,
	.rxclk_delay_en = true,
	.rxclk_delay_sel = AR8327_CLK_DELAY_SEL0,
};

static struct ar8327_platform_data esr900_ar8327_data = {
	.pad0_cfg = &esr900_ar8327_pad0_cfg,
	.pad6_cfg = &esr900_ar8327_pad6_cfg,
	.port0_cfg = {
		.force_link = 1,
		.speed = AR8327_PORT_SPEED_1000,
		.duplex = 1,
		.txpause = 1,
		.rxpause = 1,
	},
	.port6_cfg = {
		.force_link = 1,
		.speed = AR8327_PORT_SPEED_1000,
		.duplex = 1,
		.txpause = 1,
		.rxpause = 1,
	},
};

static struct mdio_board_info esr900_mdio0_info[] = {
	{
		.bus_id = "ag71xx-mdio.0",
		.mdio_addr = 0,
		.platform_data = &esr900_ar8327_data,
	},
};

static void __init esr900_setup(void)
{
	const char *config = (char *) KSEG1ADDR(ESR900_CONFIG_ADDR);
	u8 *art = (u8 *) KSEG1ADDR(ESR900_CALDATA_ADDR);
	u8 lan_mac[ETH_ALEN];
	u8 wlan0_mac[ETH_ALEN];
	u8 wlan1_mac[ETH_ALEN];

	if (ath79_nvram_parse_mac_addr(config, ESR900_CONFIG_SIZE,
				       "ethaddr=", lan_mac) == 0) {
		ath79_init_local_mac(ath79_eth0_data.mac_addr, lan_mac);
		ath79_init_mac(wlan0_mac, lan_mac, 0);
		ath79_init_mac(wlan1_mac, lan_mac, 1);
	} else {
		pr_err("could not find ethaddr in u-boot environment\n");
	}

	ath79_register_m25p80(NULL);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(esr900_leds_gpio),
					esr900_leds_gpio);
	ath79_register_gpio_keys_polled(-1, ESR900_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(esr900_gpio_keys),
					esr900_gpio_keys);

	ath79_register_usb();

	ath79_register_wmac(art + ESR900_WMAC_CALDATA_OFFSET, wlan0_mac);

	ath79_setup_qca955x_eth_cfg(QCA955X_ETH_CFG_RGMII_EN);

	ath79_register_mdio(0, 0x0);

	mdiobus_register_board_info(esr900_mdio0_info,
				    ARRAY_SIZE(esr900_mdio0_info));

	/* GMAC0 is connected to the RMGII interface */
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth0_data.phy_mask = ESR900_LAN_PHYMASK;
	ath79_eth0_data.mii_bus_dev = &ath79_mdio0_device.dev;

	ath79_eth0_pll_data.pll_1000 = 0xa6000000;
	ath79_register_eth(0);

	/* GMAC1 is connected to the SGMII interface */
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_SGMII;
	ath79_eth1_data.speed = SPEED_1000;
	ath79_eth1_data.duplex = DUPLEX_FULL;

	ath79_eth1_pll_data.pll_1000 = 0x03000101;
	ath79_register_eth(1);

	ap91_pci_init(art + ESR900_PCIE_CALDATA_OFFSET, wlan1_mac);
}

MIPS_MACHINE(ATH79_MACH_ESR900, "ESR900", "EnGenius ESR900", esr900_setup);


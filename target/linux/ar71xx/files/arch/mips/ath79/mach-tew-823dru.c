/*
 *  TRENDnet TEW-823DRU board support
 *
 *  Copyright (C) 2015 Cezary Jackiewicz <cezary.jackiewicz@gmail.com>
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

#define TEW_823DRU_GPIO_LED_POWER_ORANGE	14
#define TEW_823DRU_GPIO_LED_POWER_GREEN		19
#define TEW_823DRU_GPIO_LED_PLANET_GREEN	22
#define TEW_823DRU_GPIO_LED_PLANET_ORANGE	23

#define TEW_823DRU_GPIO_BTN_WPS			16
#define TEW_823DRU_GPIO_BTN_RESET		17

#define TEW_823DRU_KEYS_POLL_INTERVAL		20	/* msecs */
#define TEW_823DRU_KEYS_DEBOUNCE_INTERVAL	\
					(3 * TEW_823DRU_KEYS_POLL_INTERVAL)

#define TEW_823DRU_WMAC_CALDATA_OFFSET		0x1000

#define TEW_823DRU_LAN_MAC_OFFSET	0x04
#define TEW_823DRU_WAN_MAC_OFFSET	0x18

static struct gpio_led tew_823dru_leds_gpio[] __initdata = {
	{
		.name		= "trendnet:green:power",
		.gpio		= TEW_823DRU_GPIO_LED_POWER_GREEN,
		.active_low	= 1,
	},
	{
		.name		= "trendnet:orange:power",
		.gpio		= TEW_823DRU_GPIO_LED_POWER_ORANGE,
		.active_low	= 1,
	},
	{
		.name		= "trendnet:green:planet",
		.gpio		= TEW_823DRU_GPIO_LED_PLANET_GREEN,
		.active_low	= 1,
	},
	{
		.name		= "trendnet:orange:planet",
		.gpio		= TEW_823DRU_GPIO_LED_PLANET_ORANGE,
		.active_low	= 1,
	},
};

static struct gpio_keys_button tew_823dru_gpio_keys[] __initdata = {
	{
		.desc		= "Reset button",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = TEW_823DRU_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= TEW_823DRU_GPIO_BTN_RESET,
		.active_low	= 1,
	},
	{
		.desc		= "WPS button",
		.type		= EV_KEY,
		.code		= KEY_WPS_BUTTON,
		.debounce_interval = TEW_823DRU_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= TEW_823DRU_GPIO_BTN_WPS,
		.active_low	= 1,
	},
};

/* GMAC0 of the AR8327 switch is connected to the QCA9558 SoC via SGMII */
static struct ar8327_pad_cfg tew_823dru_ar8327_pad0_cfg = {
	.mode = AR8327_PAD_MAC_SGMII,
	.sgmii_delay_en = true,
};

/* GMAC6 of the AR8327 switch is connected to the QCA9558 SoC via RGMII */
static struct ar8327_pad_cfg tew_823dru_ar8327_pad6_cfg = {
	.mode = AR8327_PAD_MAC_RGMII,
	.txclk_delay_en = true,
	.rxclk_delay_en = true,
	.txclk_delay_sel = AR8327_CLK_DELAY_SEL1,
	.rxclk_delay_sel = AR8327_CLK_DELAY_SEL2,
};

static struct ar8327_platform_data tew_823dru_ar8327_data = {
	.pad0_cfg = &tew_823dru_ar8327_pad0_cfg,
	.pad6_cfg = &tew_823dru_ar8327_pad6_cfg,
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

static struct mdio_board_info tew_823dru_mdio0_info[] = {
	{
		.bus_id = "ag71xx-mdio.0",
		.mdio_addr = 0,
		.platform_data = &tew_823dru_ar8327_data,
	},
};

static void __init tew_823dru_setup(void)
{
	u8 *mac = (u8 *) KSEG1ADDR(0x1ffe0000);
	u8 *art = (u8 *) KSEG1ADDR(0x1fff0000);
	u8 lan_mac[ETH_ALEN];
	u8 wan_mac[ETH_ALEN];

	ath79_parse_ascii_mac(mac + TEW_823DRU_LAN_MAC_OFFSET, lan_mac);
	ath79_parse_ascii_mac(mac + TEW_823DRU_WAN_MAC_OFFSET, wan_mac);

	ath79_register_m25p80(NULL);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(tew_823dru_leds_gpio),
				 tew_823dru_leds_gpio);
	ath79_register_gpio_keys_polled(-1, TEW_823DRU_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(tew_823dru_gpio_keys),
					tew_823dru_gpio_keys);

	ath79_register_wmac(art + TEW_823DRU_WMAC_CALDATA_OFFSET, lan_mac);

	ath79_init_mac(ath79_eth1_data.mac_addr, lan_mac, 0);
	ath79_init_mac(ath79_eth0_data.mac_addr, wan_mac, 0);


	mdiobus_register_board_info(tew_823dru_mdio0_info,
			ARRAY_SIZE(tew_823dru_mdio0_info));
	ath79_register_mdio(0, 0x0);

	ath79_setup_qca955x_eth_cfg(QCA955X_ETH_CFG_RGMII_EN);

	/* GMAC0 is connected to the RMGII interface */
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth0_data.phy_mask = BIT(0);
	ath79_eth0_data.mii_bus_dev = &ath79_mdio0_device.dev;
	ath79_eth0_pll_data.pll_1000 = 0x56000000;

	ath79_register_eth(0);

	/* GMAC1 is connected to the SGMII interface */
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_SGMII;
	ath79_eth1_data.speed = SPEED_1000;
	ath79_eth1_data.duplex = DUPLEX_FULL;
	ath79_eth1_pll_data.pll_1000 = 0x03000101;

	ath79_register_eth(1);

	ath79_register_usb();
	ath79_register_pci();
}

MIPS_MACHINE(ATH79_MACH_TEW_823DRU, "TEW-823DRU", "TRENDnet TEW-823DRU",
	     tew_823dru_setup);

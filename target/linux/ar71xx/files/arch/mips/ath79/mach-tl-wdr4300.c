/*
 *  TP-LINK TL-WDR4300 board support
 *
 *  Copyright (C) 2012 Gabor Juhos <juhosg@openwrt.org>
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

#define WDR4300_GPIO_LED_USB1		11
#define WDR4300_GPIO_LED_USB2		12
#define WDR4300_GPIO_LED_WLAN2G		13
#define WDR4300_GPIO_LED_SYSTEM		14
#define WDR4300_GPIO_LED_QSS		15

#define WDR4300_GPIO_BTN_WPS		16
#define WDR4300_GPIO_BTN_RFKILL		17

#define WDR4300_GPIO_EXTERNAL_LNA0	18
#define WDR4300_GPIO_EXTERNAL_LNA1	19

#define WDR4300_GPIO_USB1_POWER		22
#define WDR4300_GPIO_USB2_POWER		21

#define WDR4300_KEYS_POLL_INTERVAL	20	/* msecs */
#define WDR4300_KEYS_DEBOUNCE_INTERVAL	(3 * WDR4300_KEYS_POLL_INTERVAL)

#define WDR4300_MAC0_OFFSET		0
#define WDR4300_MAC1_OFFSET		6
#define WDR4300_WMAC_CALDATA_OFFSET	0x1000
#define WDR4300_PCIE_CALDATA_OFFSET	0x5000

static const char *wdr4300_part_probes[] = {
	"tp-link",
	NULL,
};

static struct flash_platform_data wdr4300_flash_data = {
	.part_probes	= wdr4300_part_probes,
};

static struct gpio_led wdr4300_leds_gpio[] __initdata = {
	{
		.name		= "tp-link:blue:qss",
		.gpio		= WDR4300_GPIO_LED_QSS,
		.active_low	= 1,
	},
	{
		.name		= "tp-link:blue:system",
		.gpio		= WDR4300_GPIO_LED_SYSTEM,
		.active_low	= 1,
	},
	{
		.name		= "tp-link:green:usb1",
		.gpio		= WDR4300_GPIO_LED_USB1,
		.active_low	= 1,
	},
	{
		.name		= "tp-link:green:usb2",
		.gpio		= WDR4300_GPIO_LED_USB2,
		.active_low	= 1,
	},
	{
		.name		= "tp-link:blue:wlan2g",
		.gpio		= WDR4300_GPIO_LED_WLAN2G,
		.active_low	= 1,
	},
};

static struct gpio_keys_button wdr4300_gpio_keys[] __initdata = {
	{
		.desc		= "QSS button",
		.type		= EV_KEY,
		.code		= KEY_WPS_BUTTON,
		.debounce_interval = WDR4300_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= WDR4300_GPIO_BTN_WPS,
		.active_low	= 1,
	},
	{
		.desc		= "RFKILL switch",
		.type		= EV_SW,
		.code		= KEY_RFKILL,
		.debounce_interval = WDR4300_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= WDR4300_GPIO_BTN_RFKILL,
		.active_low	= 1,
	},
};

static const struct ar8327_led_info wdr4300_leds_ar8327[] = {
	AR8327_LED_INFO(PHY0_0, HW, "tp-link:blue:wan"),
	AR8327_LED_INFO(PHY1_0, HW, "tp-link:blue:lan1"),
	AR8327_LED_INFO(PHY2_0, HW, "tp-link:blue:lan2"),
	AR8327_LED_INFO(PHY3_0, HW, "tp-link:blue:lan3"),
	AR8327_LED_INFO(PHY4_0, HW, "tp-link:blue:lan4"),
};

static struct ar8327_pad_cfg wdr4300_ar8327_pad0_cfg = {
	.mode = AR8327_PAD_MAC_RGMII,
	.txclk_delay_en = true,
	.rxclk_delay_en = true,
	.txclk_delay_sel = AR8327_CLK_DELAY_SEL1,
	.rxclk_delay_sel = AR8327_CLK_DELAY_SEL2,
};

static struct ar8327_led_cfg wdr4300_ar8327_led_cfg = {
	.led_ctrl0 = 0xc737c737,
	.led_ctrl1 = 0x00000000,
	.led_ctrl2 = 0x00000000,
	.led_ctrl3 = 0x0030c300,
	.open_drain = false,
};

static struct ar8327_platform_data wdr4300_ar8327_data = {
	.pad0_cfg = &wdr4300_ar8327_pad0_cfg,
	.port0_cfg = {
		.force_link = 1,
		.speed = AR8327_PORT_SPEED_1000,
		.duplex = 1,
		.txpause = 1,
		.rxpause = 1,
	},
	.led_cfg = &wdr4300_ar8327_led_cfg,
	.num_leds = ARRAY_SIZE(wdr4300_leds_ar8327),
	.leds = wdr4300_leds_ar8327,
};

static struct mdio_board_info wdr4300_mdio0_info[] = {
	{
		.bus_id = "ag71xx-mdio.0",
		.phy_addr = 0,
		.platform_data = &wdr4300_ar8327_data,
	},
};

static void __init wdr4300_setup(void)
{
	u8 *mac = (u8 *) KSEG1ADDR(0x1f01fc00);
	u8 *art = (u8 *) KSEG1ADDR(0x1fff0000);
	u8 tmpmac[ETH_ALEN];

	ath79_register_m25p80(&wdr4300_flash_data);
	ath79_register_leds_gpio(-1, ARRAY_SIZE(wdr4300_leds_gpio),
				 wdr4300_leds_gpio);
	ath79_register_gpio_keys_polled(-1, WDR4300_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(wdr4300_gpio_keys),
					wdr4300_gpio_keys);

	ath79_wmac_set_ext_lna_gpio(0, WDR4300_GPIO_EXTERNAL_LNA0);
	ath79_wmac_set_ext_lna_gpio(1, WDR4300_GPIO_EXTERNAL_LNA1);

	ath79_init_mac(tmpmac, mac, -1);
	ath79_register_wmac(art + WDR4300_WMAC_CALDATA_OFFSET, tmpmac);

	ath79_init_mac(tmpmac, mac, 0);
	ap9x_pci_setup_wmac_led_pin(0, 0);
	ap91_pci_init(art + WDR4300_PCIE_CALDATA_OFFSET, tmpmac);

	ath79_setup_ar934x_eth_cfg(AR934X_ETH_CFG_RGMII_GMAC0);

	mdiobus_register_board_info(wdr4300_mdio0_info,
				    ARRAY_SIZE(wdr4300_mdio0_info));

	ath79_register_mdio(0, 0x0);

	ath79_init_mac(ath79_eth0_data.mac_addr, mac, -2);

	/* GMAC0 is connected to an AR8327N switch */
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth0_data.phy_mask = BIT(0);
	ath79_eth0_data.mii_bus_dev = &ath79_mdio0_device.dev;
	ath79_eth0_pll_data.pll_1000 = 0x06000000;
	ath79_register_eth(0);

	gpio_request_one(WDR4300_GPIO_USB1_POWER,
			 GPIOF_OUT_INIT_HIGH | GPIOF_EXPORT_DIR_FIXED,
			 "USB1 power");
	gpio_request_one(WDR4300_GPIO_USB2_POWER,
			 GPIOF_OUT_INIT_HIGH | GPIOF_EXPORT_DIR_FIXED,
			 "USB2 power");
	ath79_register_usb();
}

MIPS_MACHINE(ATH79_MACH_TL_WDR4300, "TL-WDR4300",
	     "TP-LINK TL-WDR3600/4300/4310",
	     wdr4300_setup);

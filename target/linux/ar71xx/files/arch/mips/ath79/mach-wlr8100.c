/*
 * Sitecom X8 AC1750 WLR-8100 board support
 *
 * Based on the Qualcomm Atheros AP135/AP136 reference board support code
 * Copyright (c) 2012 Qualcomm Atheros
 * Copyright (c) 2012-2013 Gabor Juhos <juhosg@openwrt.org>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
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

#define WLR8100_GPIO_LED_USB		4
#define WLR8100_GPIO_LED_WLAN_5G	12
#define WLR8100_GPIO_LED_WLAN_2G	13
#define WLR8100_GPIO_LED_STATUS_RED	14
#define WLR8100_GPIO_LED_WPS_RED	15
#define WLR8100_GPIO_LED_STATUS_AMBER	19
#define WLR8100_GPIO_LED_WPS_GREEN	20

#define WLR8100_GPIO_BTN_WPS		16
#define WLR8100_GPIO_BTN_RFKILL		21

#define WLR8100_KEYS_POLL_INTERVAL	20	/* msecs */
#define WLR8100_KEYS_DEBOUNCE_INTERVAL	(3 * WLR8100_KEYS_POLL_INTERVAL)

static struct gpio_led wlr8100_leds_gpio[] __initdata = {
	{
		.name		= "wlr8100:amber:status",
		.gpio		= WLR8100_GPIO_LED_STATUS_AMBER,
		.active_low	= 1,
	},
	{
		.name		= "wlr8100:red:status",
		.gpio		= WLR8100_GPIO_LED_STATUS_RED,
		.active_low	= 1,
	},
	{
		.name		= "wlr8100:green:wps",
		.gpio		= WLR8100_GPIO_LED_WPS_GREEN,
		.active_low	= 1,
	},
	{
		.name		= "wlr8100:red:wps",
		.gpio		= WLR8100_GPIO_LED_WPS_RED,
		.active_low	= 1,
	},
	{
		.name		= "wlr8100:red:wlan-2g",
		.gpio		= WLR8100_GPIO_LED_WLAN_2G,
		.active_low	= 1,
	},
	{
		.name		= "wlr8100:red:usb",
		.gpio		= WLR8100_GPIO_LED_USB,
		.active_low	= 1,
	}
};

static struct gpio_keys_button wlr8100_gpio_keys[] __initdata = {
	{
		.desc		= "WPS button",
		.type		= EV_KEY,
		.code		= KEY_WPS_BUTTON,
		.debounce_interval = WLR8100_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= WLR8100_GPIO_BTN_WPS,
		.active_low	= 1,
	},
	{
		.desc		= "RFKILL button",
		.type		= EV_KEY,
		.code		= KEY_RFKILL,
		.debounce_interval = WLR8100_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= WLR8100_GPIO_BTN_RFKILL,
		.active_low	= 1,
	},
};

static struct ar8327_pad_cfg wlr8100_ar8327_pad0_cfg;
static struct ar8327_pad_cfg wlr8100_ar8327_pad6_cfg;

static struct ar8327_platform_data wlr8100_ar8327_data = {
	.pad0_cfg = &wlr8100_ar8327_pad0_cfg,
	.pad6_cfg = &wlr8100_ar8327_pad6_cfg,
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

static struct mdio_board_info wlr8100_mdio0_info[] = {
	{
		.bus_id = "ag71xx-mdio.0",
		.mdio_addr = 0,
		.platform_data = &wlr8100_ar8327_data,
	},
};

static void __init wlr8100_common_setup(void)
{

	ath79_register_m25p80(NULL);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(wlr8100_leds_gpio),
				 wlr8100_leds_gpio);
	ath79_register_gpio_keys_polled(-1, WLR8100_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(wlr8100_gpio_keys),
					wlr8100_gpio_keys);

	ath79_register_usb();

	ath79_register_wmac_simple();

	ath79_setup_qca955x_eth_cfg(QCA955X_ETH_CFG_RGMII_EN);

	ath79_register_mdio(0, 0x0);

	mdiobus_register_board_info(wlr8100_mdio0_info,
				    ARRAY_SIZE(wlr8100_mdio0_info));

	/* GMAC0 is connected to the RMGII interface */
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth0_data.phy_mask = BIT(0);
	ath79_eth0_data.mii_bus_dev = &ath79_mdio0_device.dev;

	ath79_register_eth(0);

	/* GMAC1 is connected tot eh SGMII interface */
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_SGMII;
	ath79_eth1_data.speed = SPEED_1000;
	ath79_eth1_data.duplex = DUPLEX_FULL;

	ath79_register_eth(1);
}

static void __init wlr8100_010_setup(void)
{
	/* GMAC0 of the AR8337 switch is connected to GMAC0 via RGMII */
	wlr8100_ar8327_pad0_cfg.mode = AR8327_PAD_MAC_RGMII;
	wlr8100_ar8327_pad0_cfg.txclk_delay_en = true;
	wlr8100_ar8327_pad0_cfg.rxclk_delay_en = true;
	wlr8100_ar8327_pad0_cfg.txclk_delay_sel = AR8327_CLK_DELAY_SEL1;
	wlr8100_ar8327_pad0_cfg.rxclk_delay_sel = AR8327_CLK_DELAY_SEL2;

	/* GMAC6 of the AR8337 switch is connected to GMAC1 via SGMII */
	wlr8100_ar8327_pad6_cfg.mode = AR8327_PAD_MAC_SGMII;
	wlr8100_ar8327_pad6_cfg.rxclk_delay_en = true;
	wlr8100_ar8327_pad6_cfg.rxclk_delay_sel = AR8327_CLK_DELAY_SEL0;

	ath79_eth0_pll_data.pll_1000 = 0xa6000000;
	ath79_eth1_pll_data.pll_1000 = 0x03000101;

	wlr8100_common_setup();
	ap91_pci_init_simple();
}

MIPS_MACHINE(ATH79_MACH_WLR8100, "WLR8100",
	     "Sitecom WLR-8100",
	     wlr8100_010_setup);


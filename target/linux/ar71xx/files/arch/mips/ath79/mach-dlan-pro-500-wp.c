/*
 * devolo dLAN pro 500 Wireless+ support
 *
 * Copyright (c) 2013-2015 devolo AG
 * Copyright (c) 2011-2012 Gabor Juhos <juhosg@openwrt.org>
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

#include <linux/pci.h>
#include <linux/phy.h>
#include <linux/platform_device.h>
#include <linux/ath9k_platform.h>
#include <linux/ar8216_platform.h>
#include <linux/gpio.h>

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

#define DLAN_PRO_500_WP_GPIO_DLAN_POWER_ENABLE		13
#define DLAN_PRO_500_WP_GPIO_DLAN_LED_ENABLE		17
#define DLAN_PRO_500_WP_GPIO_LED_WLAN_5G		11
#define DLAN_PRO_500_WP_GPIO_LED_WLAN_2G		12
#define DLAN_PRO_500_WP_GPIO_LED_STATUS			16
#define DLAN_PRO_500_WP_GPIO_LED_ETH			14

#define DLAN_PRO_500_WP_GPIO_BTN_WPS			20
#define DLAN_PRO_500_WP_GPIO_BTN_WLAN			22
#define DLAN_PRO_500_WP_GPIO_BTN_DLAN			21
#define DLAN_PRO_500_WP_GPIO_BTN_RESET			4

#define DLAN_PRO_500_WP_KEYS_POLL_INTERVAL		20	/* msecs */
#define DLAN_PRO_500_WP_KEYS_DEBOUNCE_INTERVAL		(3 * DLAN_PRO_500_WP_KEYS_POLL_INTERVAL)

#define DLAN_PRO_500_WP_ART_ADDRESS			0x1fff0000
#define DLAN_PRO_500_WP_CALDATA_OFFSET			0x1000
#define DLAN_PRO_500_WP_MAC_ADDRESS_OFFSET		0x1002
#define DLAN_PRO_500_WP_PCIE_CALDATA_OFFSET		0x5000

static struct gpio_led dlan_pro_500_wp_leds_gpio[] __initdata = {
	{
		.name		= "devolo:green:status",
		.gpio		= DLAN_PRO_500_WP_GPIO_LED_STATUS,
		.active_low	= 1,
	},
	{
		.name		= "devolo:green:eth",
		.gpio		= DLAN_PRO_500_WP_GPIO_LED_ETH,
		.active_low	= 1,
	},
	{
		.name		= "devolo:blue:wlan-5g",
		.gpio		= DLAN_PRO_500_WP_GPIO_LED_WLAN_5G,
		.active_low	= 1,
	},
	{
		.name		= "devolo:green:wlan-2g",
		.gpio		= DLAN_PRO_500_WP_GPIO_LED_WLAN_2G,
		.active_low	= 1,
	}
};

static struct gpio_keys_button dlan_pro_500_wp_gpio_keys[] __initdata = {
	{
		.desc		= "dLAN button",
		.type		= EV_KEY,
		.code		= BTN_0,
		.debounce_interval = DLAN_PRO_500_WP_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= DLAN_PRO_500_WP_GPIO_BTN_DLAN,
		.active_low	= 0,
	},
	{
		.desc		= "WPS button",
		.type		= EV_KEY,
		.code		= KEY_WPS_BUTTON,
		.debounce_interval = DLAN_PRO_500_WP_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= DLAN_PRO_500_WP_GPIO_BTN_WPS,
		.active_low	= 0,
	},
	{
		.desc		= "WLAN button",
		.type		= EV_KEY,
		.code		= BTN_2,
		.debounce_interval = DLAN_PRO_500_WP_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= DLAN_PRO_500_WP_GPIO_BTN_WLAN,
		.active_low	= 1,
	},
	{
		.desc		= "Reset button",
		.type		= EV_KEY,
		.code           = KEY_RESTART,
		.debounce_interval = DLAN_PRO_500_WP_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= DLAN_PRO_500_WP_GPIO_BTN_RESET,
		.active_low	= 1,
	}
};

static struct ar8327_pad_cfg dlan_pro_500_wp_ar8327_pad0_cfg = {
	.mode = AR8327_PAD_PHY_RGMII,
	.txclk_delay_en = false,
	.rxclk_delay_en = false,
	.txclk_delay_sel = AR8327_CLK_DELAY_SEL0,
	.rxclk_delay_sel = AR8327_CLK_DELAY_SEL0,
};

static struct ar8327_led_cfg dlan_pro_500_wp_ar8327_led_cfg = {
	.led_ctrl0 = 0x00000000,
	.led_ctrl1 = 0xc737c737,
	.led_ctrl2 = 0x00000000,
	.led_ctrl3 = 0x00c30c00,
	.open_drain = true,
};

static struct ar8327_platform_data dlan_pro_500_wp_ar8327_data = {
	.pad0_cfg = &dlan_pro_500_wp_ar8327_pad0_cfg,
	.port0_cfg = {
		.force_link = 1,
		.speed = AR8327_PORT_SPEED_1000,
		.duplex = 1,
		.txpause = 0,
		.rxpause = 0,
	},
	.led_cfg = &dlan_pro_500_wp_ar8327_led_cfg,
};

static struct mdio_board_info dlan_pro_500_wp_mdio0_info[] = {
	{
		.bus_id = "ag71xx-mdio.0",
		.phy_addr = 0,
		.platform_data = &dlan_pro_500_wp_ar8327_data,
	},
};

static void __init dlan_pro_500_wp_setup(void)
{
	u8 *art = (u8 *) KSEG1ADDR(DLAN_PRO_500_WP_ART_ADDRESS);
	u8 *cal = art + DLAN_PRO_500_WP_CALDATA_OFFSET;
	u8 *wifi_mac = art + DLAN_PRO_500_WP_MAC_ADDRESS_OFFSET;

	ath79_register_m25p80(NULL);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(dlan_pro_500_wp_leds_gpio),
				 dlan_pro_500_wp_leds_gpio);

	ath79_register_gpio_keys_polled(-1, DLAN_PRO_500_WP_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(dlan_pro_500_wp_gpio_keys),
					dlan_pro_500_wp_gpio_keys);

	gpio_request_one(DLAN_PRO_500_WP_GPIO_DLAN_POWER_ENABLE,
			 GPIOF_OUT_INIT_LOW | GPIOF_EXPORT_DIR_FIXED,
			 "PLC power");
	gpio_request_one(DLAN_PRO_500_WP_GPIO_DLAN_LED_ENABLE,
			 GPIOF_OUT_INIT_LOW | GPIOF_EXPORT_DIR_FIXED,
			 "PLC LEDs");

	ath79_register_wmac(cal, wifi_mac);

	ath79_setup_ar934x_eth_cfg(AR934X_ETH_CFG_RGMII_GMAC0);

	ath79_register_mdio(1, 0x0);
	ath79_register_mdio(0, 0x0);

	mdiobus_register_board_info(dlan_pro_500_wp_mdio0_info,
				    ARRAY_SIZE(dlan_pro_500_wp_mdio0_info));

	/* GMAC0 is connected to a AR7400 PLC in PHY mode */
	ath79_init_mac(ath79_eth0_data.mac_addr, wifi_mac, 2);
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth0_pll_data.pll_1000 = 0x0e000000;
	ath79_eth0_data.speed = SPEED_1000;
	ath79_eth0_data.duplex = DUPLEX_FULL;
	ath79_register_eth(0);

	/* GMAC1 is connected to the internal switch */
	ath79_init_mac(ath79_eth1_data.mac_addr, wifi_mac, 1);
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_GMII;
	ath79_eth1_data.speed = SPEED_1000;
	ath79_eth1_data.duplex = DUPLEX_FULL;
	ath79_register_eth(1);
}

MIPS_MACHINE(ATH79_MACH_DLAN_PRO_500_WP, "dLAN-pro-500-wp", "devolo dLAN pro 500 Wireless+",
	     dlan_pro_500_wp_setup);

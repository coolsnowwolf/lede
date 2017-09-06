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
#include "dev-nfc.h"
#include "dev-spi.h"
#include "dev-wmac.h"
#include "machtypes.h"

#define DLAN_PRO_1200_AC_GPIO_DLAN_POWER_ENABLE		13
#define DLAN_PRO_1200_AC_GPIO_WLAN_POWER_ENABLE		21
#define DLAN_PRO_1200_AC_GPIO_LED_WLAN		12
#define DLAN_PRO_1200_AC_GPIO_LED_DLAN			14
#define DLAN_PRO_1200_AC_GPIO_LED_DLAN_ERR			15

#define DLAN_PRO_1200_AC_GPIO_BTN_WLAN			20
#define DLAN_PRO_1200_AC_GPIO_BTN_DLAN			22
#define DLAN_PRO_1200_AC_GPIO_BTN_RESET			4
#define DLAN_PRO_1200_AC_GPIO_DLAN_IND          17
#define DLAN_PRO_1200_AC_GPIO_DLAN_ERR_IND      16

#define DLAN_PRO_1200_AC_KEYS_POLL_INTERVAL		20	/* msecs */
#define DLAN_PRO_1200_AC_KEYS_DEBOUNCE_INTERVAL		(3 * DLAN_PRO_1200_AC_KEYS_POLL_INTERVAL)

#define DLAN_PRO_1200_AC_ART_ADDRESS			0x1fff0000
#define DLAN_PRO_1200_AC_CALDATA_OFFSET			0x1000
#define DLAN_PRO_1200_AC_WIFIMAC_OFFSET			0x1002
#define DLAN_PRO_1200_AC_PCIE_CALDATA_OFFSET	0x5000

static struct gpio_led dlan_pro_1200_ac_leds_gpio[] __initdata = {
	{
		.name		        = "devolo:status:wlan",
		.gpio		        = DLAN_PRO_1200_AC_GPIO_LED_WLAN,
		.active_low	        = 1,
	},
	{
		.name		        = "devolo:status:dlan",
		.gpio		        = DLAN_PRO_1200_AC_GPIO_LED_DLAN,
		.active_low	        = 1,
	},
	{
		.name		        = "devolo:error:dlan",
		.gpio		        = DLAN_PRO_1200_AC_GPIO_LED_DLAN_ERR,
		.active_low	        = 0,
	}
};

static struct gpio_keys_button dlan_pro_1200_ac_gpio_keys[] __initdata = {
	{
		.desc		= "dLAN button",
		.type		= EV_KEY,
		.code		= BTN_0,
		.debounce_interval = DLAN_PRO_1200_AC_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= DLAN_PRO_1200_AC_GPIO_BTN_DLAN,
		.active_low	= 1,
	},
	{
		.desc		= "WLAN button",
		.type		= EV_KEY,
		.code		= KEY_WPS_BUTTON,
		.debounce_interval = DLAN_PRO_1200_AC_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= DLAN_PRO_1200_AC_GPIO_BTN_WLAN,
		.active_low	= 0,
	},
	{
		.desc		= "Reset button",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = DLAN_PRO_1200_AC_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= DLAN_PRO_1200_AC_GPIO_BTN_RESET,
		.active_low	= 1,
	}
};

static struct ar8327_pad_cfg dlan_pro_1200_ac_ar8327_pad0_cfg = {
	.mode = AR8327_PAD_MAC_RGMII,
	.txclk_delay_en = true,
	.rxclk_delay_en = false,
	.txclk_delay_sel = AR8327_CLK_DELAY_SEL1,
	.rxclk_delay_sel = AR8327_CLK_DELAY_SEL0,
	.mac06_exchange_dis = true,
};

static struct ar8327_pad_cfg dlan_pro_1200_ac_ar8327_pad5_cfg = {
	.mode = 0,
	.txclk_delay_en = 0,
	.rxclk_delay_en = 0,
	.txclk_delay_sel = 0,
	.rxclk_delay_sel = 0,
};

static struct ar8327_platform_data dlan_pro_1200_ac_ar8327_data = {
	.pad0_cfg = &dlan_pro_1200_ac_ar8327_pad0_cfg,
	.pad5_cfg = &dlan_pro_1200_ac_ar8327_pad5_cfg,
	.port0_cfg = {
		.force_link = 1,
		.speed = AR8327_PORT_SPEED_1000,
		.duplex = 1,
		.txpause = 1,
		.rxpause = 1,
	},
};

static struct mdio_board_info dlan_pro_1200_ac_mdio0_info[] = {
	{
		.bus_id = "ag71xx-mdio.0",
		.phy_addr = 0,
		.platform_data = &dlan_pro_1200_ac_ar8327_data,
	},
};

static void __init dlan_pro_1200_ac_setup(void)
{
	u8 *art = (u8 *) KSEG1ADDR(DLAN_PRO_1200_AC_ART_ADDRESS);
	u8 *cal = art + DLAN_PRO_1200_AC_CALDATA_OFFSET;
	u8 *wifi_mac = art + DLAN_PRO_1200_AC_WIFIMAC_OFFSET;

	ath79_register_m25p80(NULL);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(dlan_pro_1200_ac_leds_gpio),
				 dlan_pro_1200_ac_leds_gpio);

	ath79_register_gpio_keys_polled(-1, DLAN_PRO_1200_AC_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(dlan_pro_1200_ac_gpio_keys),
					dlan_pro_1200_ac_gpio_keys);

	/* dLAN power must be enabled from user-space as soon as the boot-from-host daemon is running */
	gpio_request_one(DLAN_PRO_1200_AC_GPIO_DLAN_POWER_ENABLE,
			 GPIOF_OUT_INIT_LOW | GPIOF_EXPORT_DIR_FIXED,
			 "dLAN power");

	/* WLAN power is turned on initially to allow the PCI bus scan to succeed */
	gpio_request_one(DLAN_PRO_1200_AC_GPIO_WLAN_POWER_ENABLE,
			 GPIOF_OUT_INIT_HIGH | GPIOF_EXPORT_DIR_FIXED,
			 "WLAN power");

	ath79_register_wmac(cal, wifi_mac);
	ap91_pci_init(art + DLAN_PRO_1200_AC_PCIE_CALDATA_OFFSET, NULL);

	ath79_setup_ar934x_eth_cfg(AR934X_ETH_CFG_RGMII_GMAC0 | AR934X_ETH_CFG_SW_ONLY_MODE);

	ath79_register_mdio(1, 0x0);
	ath79_register_mdio(0, 0x0);

	ath79_init_mac(ath79_eth0_data.mac_addr, wifi_mac, 2);

	mdiobus_register_board_info(dlan_pro_1200_ac_mdio0_info,
				    ARRAY_SIZE(dlan_pro_1200_ac_mdio0_info));

	/* GMAC0 is connected to an AR8337 */
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth0_data.phy_mask = BIT(0);
	ath79_eth0_data.mii_bus_dev = &ath79_mdio0_device.dev;
	ath79_eth0_pll_data.pll_1000 = 0x02000000;
	ath79_register_eth(0);
}

MIPS_MACHINE(ATH79_MACH_DLAN_PRO_1200_AC, "dLAN-pro-1200-ac", "devolo dLAN pro 1200+ WiFi ac",
	     dlan_pro_1200_ac_setup);

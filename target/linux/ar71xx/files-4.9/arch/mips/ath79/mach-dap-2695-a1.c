/*
 * D-Link DAP-2695 rev. A1 support
 *
 * Copyright (c) 2012 Qualcomm Atheros
 * Copyright (c) 2012-2013 Gabor Juhos <juhosg@openwrt.org>
 * Copyright (c) 2016 Stijn Tintel <stijn@linux-ipv6.be>
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

#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
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
#include "dev-spi.h"
#include "dev-wmac.h"
#include "machtypes.h"
#include "nvram.h"

#define DAP2695_GPIO_LED_GREEN_POWER	23
#define DAP2695_GPIO_LED_RED_POWER	14
#define DAP2695_GPIO_LED_WLAN_2G	13

#define DAP2695_GPIO_BTN_RESET		17

#define DAP2695_KEYS_POLL_INTERVAL	20	/* msecs */
#define DAP2695_KEYS_DEBOUNCE_INTERVAL	(3 * DAP2695_KEYS_POLL_INTERVAL)

#define DAP2695_NVRAM_ADDR		0x1f040000
#define DAP2695_NVRAM_SIZE		0x10000

#define DAP2695_MAC0_OFFSET		1
#define DAP2695_MAC1_OFFSET		2
#define DAP2695_WMAC_CALDATA_OFFSET	0x1000

static struct gpio_led dap2695_leds_gpio[] __initdata = {
	{
		.name		= "d-link:green:power",
		.gpio		= DAP2695_GPIO_LED_GREEN_POWER,
		.active_low	= 1,
	},
	{
		.name		= "d-link:red:power",
		.gpio		= DAP2695_GPIO_LED_RED_POWER,
		.active_low	= 1,
	},
	{
		.name		= "d-link:green:wlan2g",
		.gpio		= DAP2695_GPIO_LED_WLAN_2G,
		.active_low	= 1,
	},
};

static struct gpio_keys_button dap2695_gpio_keys[] __initdata = {
	{
		.desc			= "Soft reset",
		.type			= EV_KEY,
		.code			= KEY_RESTART,
		.debounce_interval	= DAP2695_KEYS_DEBOUNCE_INTERVAL,
		.gpio			= DAP2695_GPIO_BTN_RESET,
		.active_low		= 1,
	},
};

static struct ar8327_pad_cfg dap2695_ar8327_pad0_cfg = {
	.mode = AR8327_PAD_MAC_RGMII,
	.txclk_delay_en = true,
	.rxclk_delay_en = true,
	.txclk_delay_sel = AR8327_CLK_DELAY_SEL1,
	.rxclk_delay_sel = AR8327_CLK_DELAY_SEL2,
	.mac06_exchange_dis = true,
};

static struct ar8327_pad_cfg dap2695_ar8327_pad6_cfg = {
	.mode = AR8327_PAD_MAC_SGMII,
	.sgmii_delay_en = true,
};

static struct ar8327_platform_data dap2695_ar8327_data = {
	.pad0_cfg = &dap2695_ar8327_pad0_cfg,
	.pad6_cfg = &dap2695_ar8327_pad6_cfg,
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

static struct mdio_board_info dap2695_mdio0_info[] = {
	{
		.bus_id = "ag71xx-mdio.0",
		.phy_addr = 0,
		.platform_data = &dap2695_ar8327_data,
	},
};

static struct flash_platform_data dap2695_flash_data = {
	.type = "mx25l12805d",
};

static void dap2695_get_mac(const char *name, char *mac)
{
	u8 *nvram = (u8 *) KSEG1ADDR(DAP2695_NVRAM_ADDR);
	int err;

	err = ath79_nvram_parse_mac_addr(nvram, DAP2695_NVRAM_SIZE,
					 name, mac);
	if (err)
		pr_err("no MAC address found for %s\n", name);
}

static void __init dap2695_setup(void)
{
	u8 *art = (u8 *) KSEG1ADDR(0x1fff0000);
	u8 mac0[ETH_ALEN], mac1[ETH_ALEN], wmac0[ETH_ALEN];

	dap2695_get_mac("lanmac=", mac0);
	dap2695_get_mac("wanmac=", mac1);
	dap2695_get_mac("wlanmac=", wmac0);

	ath79_register_m25p80(&dap2695_flash_data);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(dap2695_leds_gpio),
				 dap2695_leds_gpio);
	ath79_register_gpio_keys_polled(-1, DAP2695_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(dap2695_gpio_keys),
					dap2695_gpio_keys);

	ath79_register_wmac(art + DAP2695_WMAC_CALDATA_OFFSET, wmac0);

	ath79_setup_qca955x_eth_cfg(QCA955X_ETH_CFG_RGMII_EN);

	ath79_register_mdio(0, 0x0);

	mdiobus_register_board_info(dap2695_mdio0_info,
					ARRAY_SIZE(dap2695_mdio0_info));

	/* GMAC0 is connected to the RGMII interface */
	ath79_init_mac(ath79_eth0_data.mac_addr, mac0, DAP2695_MAC0_OFFSET);
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth0_data.phy_mask = BIT(0);
	ath79_eth0_data.mii_bus_dev = &ath79_mdio0_device.dev;
	ath79_eth0_pll_data.pll_1000 = 0x56000000;

	ath79_register_eth(0);

	/* GMAC1 is connected to the SGMII interface */
	ath79_init_mac(ath79_eth1_data.mac_addr, mac1, DAP2695_MAC1_OFFSET);
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_SGMII;
	ath79_eth1_data.speed = SPEED_1000;
	ath79_eth1_data.duplex = DUPLEX_FULL;
	ath79_eth1_pll_data.pll_1000 = 0x03000101;

	ath79_register_eth(1);

	ath79_register_pci();
}

MIPS_MACHINE(ATH79_MACH_DAP_2695_A1, "DAP-2695-A1",
		"D-Link DAP-2695 rev. A1",
		dap2695_setup);

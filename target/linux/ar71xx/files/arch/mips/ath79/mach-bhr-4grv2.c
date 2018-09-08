/*
 * Buffalo BHR-4GRV2 board support
 *
 * Copyright (c) 2012 Qualcomm Atheros
 * Copyright (c) 2012-2013 Gabor Juhos <juhosg@openwrt.org>
 * Copyright (c) 2016 FUKAUMI Naoki <naobsd@gmail.com>
 *
 * Based on mach-ap136.c and mach-wzr-450hp2.c
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
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "machtypes.h"

#define BHR_4GRV2_GPIO_LED_VPN_RED	3
#define BHR_4GRV2_GPIO_LED_VPN_GREEN	18
#define BHR_4GRV2_GPIO_LED_POWER_GREEN	19
#define BHR_4GRV2_GPIO_LED_DIAG_RED	20

#define BHR_4GRV2_GPIO_BTN_RESET	17
#define BHR_4GRV2_GPIO_BTN_ECO		21

#define BHR_4GRV2_KEYS_POLL_INTERVAL	20	/* msecs */
#define BHR_4GRV2_KEYS_DEBOUNCE_INTERVAL	(3 * BHR_4GRV2_KEYS_POLL_INTERVAL)
#define BHR_4GRV2_MAC0_OFFSET		0
#define BHR_4GRV2_MAC1_OFFSET		6

static struct gpio_led bhr_4grv2_leds_gpio[] __initdata = {
	{
		.name		= "buffalo:red:vpn",
		.gpio		= BHR_4GRV2_GPIO_LED_VPN_RED,
		.active_low	= 1,
	},
	{
		.name		= "buffalo:green:vpn",
		.gpio		= BHR_4GRV2_GPIO_LED_VPN_GREEN,
		.active_low	= 1,
	},
	{
		.name		= "buffalo:green:power",
		.gpio		= BHR_4GRV2_GPIO_LED_POWER_GREEN,
		.active_low	= 1,
	},
	{
		.name		= "buffalo:red:diag",
		.gpio		= BHR_4GRV2_GPIO_LED_DIAG_RED,
		.active_low	= 1,
	}
};

static struct gpio_keys_button bhr_4grv2_gpio_keys[] __initdata = {
	{
		.desc		= "Reset button",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = BHR_4GRV2_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= BHR_4GRV2_GPIO_BTN_RESET,
		.active_low	= 1,
	},
	{
		.desc		= "ECO button",
		.type		= EV_KEY,
		.code		= BTN_0,
		.debounce_interval = BHR_4GRV2_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= BHR_4GRV2_GPIO_BTN_ECO,
		.active_low	= 1,
	},
};

/* GMAC0 of the AR8327 switch is connected to GMAC1 via SGMII */
static struct ar8327_pad_cfg bhr_4grv2_ar8327_pad0_cfg = {
	.mode = AR8327_PAD_MAC_SGMII,
	.sgmii_delay_en = true,
};

/* GMAC6 of the AR8327 switch is connected to GMAC0 via RGMII */
static struct ar8327_pad_cfg bhr_4grv2_ar8327_pad6_cfg = {
	.mode = AR8327_PAD_MAC_RGMII,
	.txclk_delay_en = true,
	.rxclk_delay_en = true,
	.txclk_delay_sel = AR8327_CLK_DELAY_SEL1,
	.rxclk_delay_sel = AR8327_CLK_DELAY_SEL2,
};

static struct ar8327_platform_data bhr_4grv2_ar8327_data = {
	.pad0_cfg = &bhr_4grv2_ar8327_pad0_cfg,
	.pad6_cfg = &bhr_4grv2_ar8327_pad6_cfg,
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

static struct mdio_board_info bhr_4grv2_mdio0_info[] = {
	{
		.bus_id = "ag71xx-mdio.0",
		.mdio_addr = 0,
		.platform_data = &bhr_4grv2_ar8327_data,
	},
};

static void __init bhr_4grv2_setup(void)
{
	u8 *art = (u8 *) KSEG1ADDR(0x1fff0000);

	ath79_register_m25p80(NULL);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(bhr_4grv2_leds_gpio),
				 bhr_4grv2_leds_gpio);
	ath79_register_gpio_keys_polled(-1, BHR_4GRV2_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(bhr_4grv2_gpio_keys),
					bhr_4grv2_gpio_keys);

	mdiobus_register_board_info(bhr_4grv2_mdio0_info,
				    ARRAY_SIZE(bhr_4grv2_mdio0_info));
	ath79_register_mdio(0, 0x0);

	ath79_setup_qca955x_eth_cfg(QCA955X_ETH_CFG_RGMII_EN);

	/* GMAC0 is connected to the RGMII interface */
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth0_data.phy_mask = BIT(0);
	ath79_eth0_data.mii_bus_dev = &ath79_mdio0_device.dev;
	ath79_eth0_pll_data.pll_1000 = 0x56000000;

	ath79_init_mac(ath79_eth0_data.mac_addr, art + BHR_4GRV2_MAC0_OFFSET, 0);
	ath79_register_eth(0);

	/* GMAC1 is connected to the SGMII interface */
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_SGMII;
	ath79_eth1_data.speed = SPEED_1000;
	ath79_eth1_data.duplex = DUPLEX_FULL;
	ath79_eth1_pll_data.pll_1000 = 0x03000101;

	ath79_init_mac(ath79_eth1_data.mac_addr, art + BHR_4GRV2_MAC1_OFFSET, 0);
	ath79_register_eth(1);
}

MIPS_MACHINE(ATH79_MACH_BHR_4GRV2, "BHR-4GRV2",
	     "Buffalo BHR-4GRV2", bhr_4grv2_setup);

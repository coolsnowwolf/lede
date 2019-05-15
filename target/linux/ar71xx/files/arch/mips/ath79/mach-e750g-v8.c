/*
 *  Qxwlan E750G v8 board support
 *
 *  Copyright (C) 2017 Peng Zhang <sd20@qxwlan.com>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/phy.h>
#include <linux/platform_device.h>
#include <linux/ath9k_platform.h>
#include <linux/ar8216_platform.h>

#include <asm/mach-ath79/ar71xx_regs.h>

#include "common.h"
#include "pci.h"
#include "dev-ap9x-pci.h"
#include "dev-gpio-buttons.h"
#include "dev-eth.h"
#include "dev-usb.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-spi.h"
#include "dev-wmac.h"
#include "machtypes.h"

#define E750G_V8_GPIO_LED_SYS	14
#define E750G_V8_GPIO_LED_DS20	15
#define E750G_V8_GPIO_LED_DS10	20
#define E750G_V8_GPIO_LED_WLAN	21

#define E750G_V8_GPIO_BTN_RESET	12

#define E750G_V8_KEYS_POLL_INTERVAL	20	/* msecs */
#define E750G_V8_KEYS_DEBOUNCE_INTERVAL	(3 * E750G_V8_KEYS_POLL_INTERVAL)

static struct gpio_led e750g_v8_leds_gpio[] __initdata = {
	{
		.name		= "e750g-v8:green:system",
		.gpio		= E750G_V8_GPIO_LED_SYS,
		.active_low	= 1,
	},
	{
		.name		= "e750g-v8:green:ds10",
		.gpio		= E750G_V8_GPIO_LED_DS10,
		.active_low	= 1,
	},
	{
		.name		= "e750g-v8:green:ds20",
		.gpio		= E750G_V8_GPIO_LED_DS20,
		.active_low	= 1,
	},
	{
		.name		= "e750g-v8:green:wlan",
		.gpio		= E750G_V8_GPIO_LED_WLAN,
		.active_low	= 1,
	},
};

static struct gpio_keys_button e750g_v8_gpio_keys[] __initdata = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = E750G_V8_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= E750G_V8_GPIO_BTN_RESET,
		.active_low	= 1,
	},
};

static const struct ar8327_led_info e750g_v8_leds_qca8334[] = {
	AR8327_LED_INFO(PHY1_0, HW, "e750g-v8:green:lan"),
	AR8327_LED_INFO(PHY2_0, HW, "e750g-v8:green:wan"),
};

static struct ar8327_pad_cfg e750g_v8_ar8327_pad0_cfg = {
	.mode = AR8327_PAD_MAC_RGMII,
	.txclk_delay_en = true,
	.rxclk_delay_en = true,
	.txclk_delay_sel = AR8327_CLK_DELAY_SEL1,
	.rxclk_delay_sel = AR8327_CLK_DELAY_SEL2,
	.mac06_exchange_dis = true,
};

static struct ar8327_led_cfg e750g_v8_ar8327_led_cfg = {
	.led_ctrl0 = 0x00000000,
	.led_ctrl1 = 0xc737c737,
	.led_ctrl2 = 0x00000000,
	.led_ctrl3 = 0x00c30c00,
	.open_drain = true,
};

static struct ar8327_platform_data e750g_v8_ar8327_data = {
	.pad0_cfg = &e750g_v8_ar8327_pad0_cfg,
	.port0_cfg = {
		.force_link = 1,
		.speed = AR8327_PORT_SPEED_1000,
		.duplex = 1,
		.txpause = 1,
		.rxpause = 1,
	},
	.led_cfg = &e750g_v8_ar8327_led_cfg,
	.leds = e750g_v8_leds_qca8334,
	.num_leds = ARRAY_SIZE(e750g_v8_leds_qca8334),
};

static struct mdio_board_info e750g_v8_mdio0_info[] = {
	{
		.bus_id = "ag71xx-mdio.0",
		.mdio_addr = 0,
		.platform_data = &e750g_v8_ar8327_data,
	},
};

static void __init e750g_v8_setup(void)
{
	u8 *mac = (u8 *) KSEG1ADDR(0x1f050400);
	u8 *art = (u8 *) KSEG1ADDR(0x1f061000);

	ath79_register_m25p80(NULL);
	ath79_register_leds_gpio(-1, ARRAY_SIZE(e750g_v8_leds_gpio),
				 e750g_v8_leds_gpio);
	ath79_register_gpio_keys_polled(-1, E750G_V8_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(e750g_v8_gpio_keys),
					e750g_v8_gpio_keys);

	mdiobus_register_board_info(e750g_v8_mdio0_info,
					ARRAY_SIZE(e750g_v8_mdio0_info));

	ath79_register_mdio(0, 0x0);
	ath79_setup_ar934x_eth_cfg(AR934X_ETH_CFG_RGMII_GMAC0 |
				   AR934X_ETH_CFG_SW_ONLY_MODE);

	/* GMAC0 is connected to an AR8327 switch */
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth0_data.phy_mask = BIT(0);
	ath79_eth0_data.mii_bus_dev = &ath79_mdio0_device.dev;
	ath79_eth0_pll_data.pll_1000 = 0x06000000;
	ath79_init_mac(ath79_eth0_data.mac_addr, mac, 0);
	ath79_register_eth(0);

	ath79_register_pci();
	ath79_register_usb();
	ath79_register_wmac(art, NULL);
}

MIPS_MACHINE(ATH79_MACH_E750G_V8, "E750G-V8", "Qxwlan E750G v8",
		e750g_v8_setup);

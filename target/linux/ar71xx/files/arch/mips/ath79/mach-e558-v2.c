/*
 * Qxwlan E558 v2 board support
 *
 *  Copyright (C) 2017 Peng Zhang <sd20@qxwlan.com>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

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
#include "pci.h"

#define E558_V2_GPIO_LED_WLAN	13
#define E558_V2_GPIO_LED_SYSTEM	14
#define E558_V2_GPIO_LED_QSS	15

#define E558_V2_GPIO_BTN_RESET	16

#define E558_V2_KEYS_POLL_INTERVAL	20	/* msecs */
#define E558_V2_KEYS_DEBOUNCE_INTERVAL (3 * E558_V2_KEYS_POLL_INTERVAL)

static struct gpio_led e558_v2_leds_gpio[] __initdata = {
	{
		.name		= "e558-v2:green:qss",
		.gpio		= E558_V2_GPIO_LED_QSS,
		.active_low	= 1,
	},
	{
		.name		= "e558-v2:green:system",
		.gpio		= E558_V2_GPIO_LED_SYSTEM,
		.active_low	= 1,
	},
	{
		.name		= "e558-v2:green:wlan",
		.gpio		= E558_V2_GPIO_LED_WLAN,
		.active_low	= 1,
	},
};

static struct gpio_keys_button e558_v2_gpio_keys[] __initdata = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = E558_V2_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= E558_V2_GPIO_BTN_RESET,
		.active_low	= 1,
	},
};

/* GMAC0 of the AR8327 switch is connected to the QCA9558 SoC via SGMII */
static struct ar8327_pad_cfg e558_v2_ar8327_pad0_cfg = {
	.mode = AR8327_PAD_MAC_SGMII,
	.sgmii_delay_en = true,
};

/* GMAC6 of the AR8327 switch is connected to the QCA9558 SoC via RGMII */
static struct ar8327_pad_cfg e558_v2_ar8327_pad6_cfg = {
	.mode = AR8327_PAD_MAC_RGMII,
	.txclk_delay_en = true,
	.rxclk_delay_en = true,
	.txclk_delay_sel = AR8327_CLK_DELAY_SEL1,
	.rxclk_delay_sel = AR8327_CLK_DELAY_SEL2,
};

static const struct ar8327_led_info e558_v2_leds_qca8334[] = {
	AR8327_LED_INFO(PHY2_0, HW, "e558-v2:green:wan"),
	AR8327_LED_INFO(PHY3_0, HW, "e558-v2:green:lan1"),
	AR8327_LED_INFO(PHY4_0, HW, "e558-v2:green:lan2"),
};

static struct ar8327_led_cfg e558_v2_ar8327_led_cfg = {
	.led_ctrl0 = 0xc737c737,
	.led_ctrl1 = 0x00000000,
	.led_ctrl2 = 0x00000000,
	.led_ctrl3 = 0x0030c300,
	.open_drain = false,
};

static struct ar8327_platform_data e558_v2_ar8327_data = {
	.pad0_cfg = &e558_v2_ar8327_pad0_cfg,
	.pad6_cfg = &e558_v2_ar8327_pad6_cfg,
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
	.led_cfg = &e558_v2_ar8327_led_cfg,
	.leds = e558_v2_leds_qca8334,
	.num_leds = ARRAY_SIZE(e558_v2_leds_qca8334),
};

static struct mdio_board_info e558_v2_mdio0_info[] = {
	{
		.bus_id = "ag71xx-mdio.0",
		.phy_addr = 0,
		.platform_data = &e558_v2_ar8327_data,
	},
};

static void __init e558_v2_setup(void)
{
	u8 *mac = (u8 *) KSEG1ADDR(0x1f050400);
	u8 *art = (u8 *) KSEG1ADDR(0x1f061000);

	ath79_register_m25p80(NULL);
	ath79_register_leds_gpio(-1, ARRAY_SIZE(e558_v2_leds_gpio),
				 e558_v2_leds_gpio);

	ath79_register_gpio_keys_polled(-1, E558_V2_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(e558_v2_gpio_keys),
					e558_v2_gpio_keys);

	ath79_register_mdio(0, 0x0);
	mdiobus_register_board_info(e558_v2_mdio0_info,
				    ARRAY_SIZE(e558_v2_mdio0_info));

	ath79_setup_qca955x_eth_cfg(QCA955X_ETH_CFG_RGMII_EN);

	/* GMAC0 is connected to the RMGII interface */
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth0_data.phy_mask = BIT(0);
	ath79_eth0_data.mii_bus_dev = &ath79_mdio0_device.dev;
	ath79_eth0_pll_data.pll_1000 = 0x56000000;
	ath79_init_mac(ath79_eth0_data.mac_addr, mac, 1);
	ath79_register_eth(0);

	/* GMAC1 is connected to the SGMII interface */
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_SGMII;
	ath79_eth1_data.speed = SPEED_1000;
	ath79_eth1_data.duplex = DUPLEX_FULL;
	ath79_eth1_pll_data.pll_1000 = 0x03000101;
	ath79_init_mac(ath79_eth1_data.mac_addr, mac, 0);
	ath79_register_eth(1);

	ath79_register_pci();
	ath79_register_usb();
	ath79_register_wmac(art, NULL);
}

MIPS_MACHINE(ATH79_MACH_E558_V2, "E558-V2", "Qxwlan E558 v2",
		e558_v2_setup);


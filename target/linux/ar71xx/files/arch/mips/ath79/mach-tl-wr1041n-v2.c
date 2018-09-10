/*
 *  TP-LINK TL-WR1041 v2 board support
 *
 *  Copyright (C) 2010-2012 Gabor Juhos <juhosg@openwrt.org>
 *  Copyright (C) 2011-2012 Anan Huang <axishero@foxmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/pci.h>
#include <linux/phy.h>
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
#include "dev-wmac.h"
#include "machtypes.h"

#define TL_WR1041NV2_GPIO_BTN_RESET	14
#define TL_WR1041NV2_GPIO_LED_WPS	13
#define TL_WR1041NV2_GPIO_LED_WLAN	11

#define TL_WR1041NV2_GPIO_LED_SYSTEM	12

#define TL_WR1041NV2_KEYS_POLL_INTERVAL		20	/* msecs */
#define TL_WR1041NV2_KEYS_DEBOUNCE_INTERVAL	(3 * TL_WR1041NV2_KEYS_POLL_INTERVAL)

#define TL_WR1041NV2_PCIE_CALDATA_OFFSET	0x5000

static const char *tl_wr1041nv2_part_probes[] = {
	"tp-link",
	NULL,
};

static struct flash_platform_data tl_wr1041nv2_flash_data = {
	.part_probes	= tl_wr1041nv2_part_probes,
};

static struct gpio_led tl_wr1041nv2_leds_gpio[] __initdata = {
	{
		.name		= "tp-link:green:system",
		.gpio		= TL_WR1041NV2_GPIO_LED_SYSTEM,
		.active_low	= 1,
	}, {
		.name		= "tp-link:green:wps",
		.gpio		= TL_WR1041NV2_GPIO_LED_WPS,
		.active_low	= 1,
	}, {
		.name		= "tp-link:green:wlan",
		.gpio		= TL_WR1041NV2_GPIO_LED_WLAN,
		.active_low	= 1,
	}
};

static struct gpio_keys_button tl_wr1041nv2_gpio_keys[] __initdata = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = TL_WR1041NV2_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= TL_WR1041NV2_GPIO_BTN_RESET,
		.active_low	= 1,
	}
};

static const struct ar8327_led_info tl_wr1041n_leds_ar8327[] = {
	AR8327_LED_INFO(PHY0_0, HW, "tp-link:green:wan"),
	AR8327_LED_INFO(PHY1_0, HW, "tp-link:green:lan1"),
	AR8327_LED_INFO(PHY2_0, HW, "tp-link:green:lan2"),
	AR8327_LED_INFO(PHY3_0, HW, "tp-link:green:lan3"),
	AR8327_LED_INFO(PHY4_0, HW, "tp-link:green:lan4"),
};

static struct ar8327_led_cfg wr1041n_v2_ar8327_led_cfg = {
	.led_ctrl0 = 0xcf35cf35,	/* LED0: blink at 10/100/1000M */
	.led_ctrl1 = 0xcf35cf35,	/* LED1: blink at 10/100/1000M: anyway, no LED1 on tl-wr1041n */
	.led_ctrl2 = 0xcf35cf35,	/* LED2: blink at 10/100/1000M: anyway, no LED2 on tl-wr1041n */
	.led_ctrl3 = 0x03ffff00,	/* Pattern enabled for LED 0-2 of port 1-3 */
	.open_drain = true,
};

static struct ar8327_pad_cfg db120_ar8327_pad0_cfg = {
	.mode = AR8327_PAD_MAC_RGMII,
	.txclk_delay_en = true,
	.rxclk_delay_en = true,
	.txclk_delay_sel = AR8327_CLK_DELAY_SEL1,
	.rxclk_delay_sel = AR8327_CLK_DELAY_SEL2,
};

static struct ar8327_platform_data db120_ar8327_data = {
	.pad0_cfg = &db120_ar8327_pad0_cfg,
	.port0_cfg = {
		.force_link = 1,
		.speed = AR8327_PORT_SPEED_1000,
		.duplex = 1,
		.txpause = 1,
		.rxpause = 1,
	},
	.led_cfg = &wr1041n_v2_ar8327_led_cfg,
	.num_leds = ARRAY_SIZE(tl_wr1041n_leds_ar8327),
	.leds = tl_wr1041n_leds_ar8327
};

static struct mdio_board_info db120_mdio0_info[] = {
	{
		.bus_id = "ag71xx-mdio.0",
		.mdio_addr = 0,
		.platform_data = &db120_ar8327_data,
	},
};

static void __init tl_wr1041nv2_setup(void)
{
	u8 *mac = (u8 *) KSEG1ADDR(0x1f01fc00);
	u8 *ee = (u8 *) KSEG1ADDR(0x1fff1000);

	ath79_register_m25p80(&tl_wr1041nv2_flash_data);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(tl_wr1041nv2_leds_gpio),
				 tl_wr1041nv2_leds_gpio);
	ath79_register_gpio_keys_polled(-1, TL_WR1041NV2_KEYS_POLL_INTERVAL,
					 ARRAY_SIZE(tl_wr1041nv2_gpio_keys),
					 tl_wr1041nv2_gpio_keys);
	ath79_register_wmac(ee, mac);

	ath79_setup_ar934x_eth_cfg(AR934X_ETH_CFG_RGMII_GMAC0 |
				   AR934X_ETH_CFG_SW_ONLY_MODE);

	ath79_register_mdio(1, 0x0);
	ath79_register_mdio(0, 0x0);

	ath79_init_mac(ath79_eth0_data.mac_addr, mac, 1);

	mdiobus_register_board_info(db120_mdio0_info,
				    ARRAY_SIZE(db120_mdio0_info));

	/* GMAC0 is connected to an AR8327 switch */
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth0_data.phy_mask = BIT(0);
	ath79_eth0_data.mii_bus_dev = &ath79_mdio0_device.dev;
	ath79_eth0_pll_data.pll_1000 = 0x06000000;
	ath79_register_eth(0);
}

MIPS_MACHINE(ATH79_MACH_TL_WR1041N_V2, "TL-WR1041N-v2",
	     "TP-LINK TL-WR1041N v2", tl_wr1041nv2_setup);

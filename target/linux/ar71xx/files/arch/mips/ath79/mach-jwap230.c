/*
 * jjPlus JWAP230 board support
 *
 * Copyright (C) 2016 Piotr Dymacz <pepe2k@gmail.com>
 *
 * Based on mach-wpj558.c and mach-tl-wr1043nd-v2.c
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 */

#include <linux/pci.h>
#include <linux/phy.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>
#include <linux/ar8216_platform.h>

#include <asm/mach-ath79/ath79.h>
#include <asm/mach-ath79/ar71xx_regs.h>

#include "common.h"
#include "dev-ap9x-pci.h"
#include "dev-eth.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-usb.h"
#include "dev-wmac.h"
#include "machtypes.h"
#include "pci.h"

#define JWAP230_GPIO_LED_LED1		23
#define JWAP230_GPIO_LED_LED2		22
#define JWAP230_GPIO_LED_LED3		21

#define JWAP230_MAC0_OFFSET		0x0
#define JWAP230_MAC1_OFFSET		0x6
#define JWAP230_WMAC_CALDATA_OFFSET	0x1000

static struct gpio_led jwap230_leds_gpio[] __initdata = {
	{
		.name		= "jwap230:green:led1",
		.gpio		= JWAP230_GPIO_LED_LED1,
		.active_low	= 1,
	},
	{
		.name		= "jwap230:green:led2",
		.gpio		= JWAP230_GPIO_LED_LED2,
		.active_low	= 1,
	},
	{
		.name		= "jwap230:green:led3",
		.gpio		= JWAP230_GPIO_LED_LED3,
		.active_low	= 1,
	}
};

static const struct ar8327_led_info jwap230_leds_qca8337[] = {
	AR8327_LED_INFO(PHY0_0, HW, "jwap230:green:lan"),
	AR8327_LED_INFO(PHY4_0, HW, "jwap230:green:wan"),
};

/* Blink rate: 1 Gbps -> 8 hz, 100 Mbs -> 4 Hz, 10 Mbps -> 2 Hz */
static struct ar8327_led_cfg jwap230_qca8337_led_cfg = {
	.led_ctrl0 = 0xcf37cf37,
	.led_ctrl1 = 0xcf37cf37,
	.led_ctrl2 = 0xcf37cf37,
	.led_ctrl3 = 0x0,
	.open_drain = true,
};

/* QCA8337 GMAC0 is connected with QCA9558 over RGMII */
static struct ar8327_pad_cfg jwap230_qca8337_pad0_cfg = {
	.mode = AR8327_PAD_MAC_RGMII,
	.txclk_delay_en = true,
	.rxclk_delay_en = true,
	.txclk_delay_sel = AR8327_CLK_DELAY_SEL1,
	.rxclk_delay_sel = AR8327_CLK_DELAY_SEL2,
	.mac06_exchange_dis = true,
};

/* QCA8337 GMAC6 is connected with QCA9558 over SGMII */
static struct ar8327_pad_cfg jwap230_qca8337_pad6_cfg = {
	.mode = AR8327_PAD_MAC_SGMII,
	.sgmii_delay_en = true,
	.rxclk_delay_sel = AR8327_CLK_DELAY_SEL0,
};

static struct ar8327_platform_data jwap230_qca8337_data = {
	.pad0_cfg = &jwap230_qca8337_pad0_cfg,
	.pad6_cfg = &jwap230_qca8337_pad6_cfg,
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
	.led_cfg = &jwap230_qca8337_led_cfg,
	.num_leds = ARRAY_SIZE(jwap230_leds_qca8337),
	.leds = jwap230_leds_qca8337,
};

static struct mdio_board_info jwap230_mdio0_info[] = {
	{
		.bus_id = "ag71xx-mdio.0",
		.mdio_addr = 0,
		.platform_data = &jwap230_qca8337_data,
	},
};

static void __init jwap230_setup(void)
{
	u8 *art = (u8 *) KSEG1ADDR(0x1fff0000);

	ath79_register_m25p80(NULL);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(jwap230_leds_gpio),
				 jwap230_leds_gpio);

	mdiobus_register_board_info(jwap230_mdio0_info,
				    ARRAY_SIZE(jwap230_mdio0_info));
	ath79_register_mdio(0, 0x0);

	ath79_setup_qca955x_eth_cfg(QCA955X_ETH_CFG_RGMII_EN);

	/* QCA9558 GMAC0 is connected to RMGII interface */
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth0_data.phy_mask = BIT(0);
	ath79_eth0_data.mii_bus_dev = &ath79_mdio0_device.dev;
	ath79_eth0_pll_data.pll_1000 = 0xa6000000;

	ath79_init_mac(ath79_eth0_data.mac_addr, art + JWAP230_MAC0_OFFSET, 0);
	ath79_register_eth(0);

	/* QCA9558 GMAC1 is connected to SGMII interface */
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_SGMII;
	ath79_eth1_data.speed = SPEED_1000;
	ath79_eth1_data.duplex = DUPLEX_FULL;
	ath79_eth1_pll_data.pll_1000 = 0x03000101;

	ath79_init_mac(ath79_eth1_data.mac_addr, art + JWAP230_MAC1_OFFSET, 0);
	ath79_register_eth(1);

	ath79_register_wmac(art + JWAP230_WMAC_CALDATA_OFFSET, NULL);

	ath79_register_pci();
	ath79_register_usb();
}

MIPS_MACHINE(ATH79_MACH_JWAP230, "JWAP230", "jjPlus JWAP230", jwap230_setup);

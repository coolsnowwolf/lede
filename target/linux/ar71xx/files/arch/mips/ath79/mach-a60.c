/*
 *  OpenMesh A60 support
 *
 *  Copyright (C) 2013 Marek Lindner <marek@open-mesh.com>
 *  Copyright (C) 2014-2017 Sven Eckelmann <sven@open-mesh.com>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/gpio.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/platform_device.h>
#include <linux/platform_data/phy-at803x.h>

#include <asm/mach-ath79/ar71xx_regs.h>
#include <asm/mach-ath79/ath79.h>

#include "common.h"
#include "dev-ap9x-pci.h"
#include "dev-eth.h"
#include "dev-leds-gpio.h"
#include "dev-gpio-buttons.h"
#include "dev-m25p80.h"
#include "dev-wmac.h"
#include "machtypes.h"
#include "pci.h"
#include "dev-usb.h"

#define A60_GPIO_LED_RED	22
#define A60_GPIO_LED_GREEN	23
#define A60_GPIO_LED_BLUE	13

#define A60_GPIO_BTN_RESET	17

#define A60_KEYS_POLL_INTERVAL	20	/* msecs */
#define A60_KEYS_DEBOUNCE_INTERVAL	(3 * A60_KEYS_POLL_INTERVAL)

#define A60_WMAC_CALDATA_OFFSET	0x1000

static struct gpio_led a40_leds_gpio[] __initdata = {
	{
		.name		= "a40:red:status",
		.gpio		= A60_GPIO_LED_RED,
	}, {
		.name		= "a40:green:status",
		.gpio		= A60_GPIO_LED_GREEN,
	}, {
		.name		= "a40:blue:status",
		.gpio		= A60_GPIO_LED_BLUE,
	}
};

static struct gpio_led a60_leds_gpio[] __initdata = {
	{
		.name		= "a60:red:status",
		.gpio		= A60_GPIO_LED_RED,
	}, {
		.name		= "a60:green:status",
		.gpio		= A60_GPIO_LED_GREEN,
	}, {
		.name		= "a60:blue:status",
		.gpio		= A60_GPIO_LED_BLUE,
	}
};

static struct gpio_keys_button a60_gpio_keys[] __initdata = {
	{
		.desc		= "Reset button",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = A60_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= A60_GPIO_BTN_RESET,
		.active_low	= 1,
	},
};

static struct at803x_platform_data a60_at803x_data = {
	.disable_smarteee = 1,
	.enable_rgmii_rx_delay = 1,
	.enable_rgmii_tx_delay = 1,
};

static struct mdio_board_info a60_mdio0_info[] = {
	{
		.bus_id = "ag71xx-mdio.0",
		.phy_addr = 1,
		.platform_data = &a60_at803x_data,
	},
	{
		.bus_id = "ag71xx-mdio.0",
		.phy_addr = 2,
		.platform_data = &a60_at803x_data,
	},
};

static void __init a60_setup_qca955x_eth_cfg(u32 mask,
					     unsigned int rxd,
					     unsigned int rxdv,
					     unsigned int txd,
					     unsigned int txe)
{
	void __iomem *base;
	u32 t;

	base = ioremap(QCA955X_GMAC_BASE, QCA955X_GMAC_SIZE);

	t = mask;
	t |= rxd << QCA955X_ETH_CFG_RXD_DELAY_SHIFT;
	t |= rxdv << QCA955X_ETH_CFG_RDV_DELAY_SHIFT;
	t |= txd << QCA955X_ETH_CFG_TXD_DELAY_SHIFT;
	t |= txe << QCA955X_ETH_CFG_TXE_DELAY_SHIFT;

	__raw_writel(t, base + QCA955X_GMAC_REG_ETH_CFG);

	iounmap(base);
}

static void __init a60_setup_common(void)
{
	u8 *art = (u8 *)KSEG1ADDR(0x1fff0000);
	u8 mac[6];

	ath79_register_usb();

	ath79_register_m25p80(NULL);
	ath79_register_gpio_keys_polled(-1, A60_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(a60_gpio_keys),
					a60_gpio_keys);

	ath79_init_mac(mac, art, 0x02);
	ath79_register_wmac(art + A60_WMAC_CALDATA_OFFSET, mac);

	a60_setup_qca955x_eth_cfg(QCA955X_ETH_CFG_RGMII_EN, 3, 3, 0, 0);
	ath79_register_mdio(0, 0x0);

	mdiobus_register_board_info(a60_mdio0_info, ARRAY_SIZE(a60_mdio0_info));

	ath79_init_mac(ath79_eth0_data.mac_addr, art, 0x00);
	ath79_init_mac(ath79_eth1_data.mac_addr, art, 0x01);

	/* GMAC0 is connected to the PHY1 */
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth0_data.mii_bus_dev = &ath79_mdio0_device.dev;
	ath79_eth0_data.phy_mask = BIT(1);
	ath79_eth0_pll_data.pll_1000 = 0x82000101;
	ath79_eth0_pll_data.pll_100 = 0x80000101;
	ath79_eth0_pll_data.pll_10 = 0x80001313;
	ath79_register_eth(0);

	/* GMAC1 is connected to MDIO1 in SGMII mode */
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_SGMII;
	ath79_eth1_data.mii_bus_dev = &ath79_mdio0_device.dev;
	ath79_eth1_data.phy_mask = BIT(2);
	ath79_eth1_pll_data.pll_1000 = 0x03000101;
	ath79_eth1_pll_data.pll_100 = 0x80000101;
	ath79_eth1_pll_data.pll_10 = 0x80001313;
	ath79_eth1_data.speed = SPEED_1000;
	ath79_eth1_data.duplex = DUPLEX_FULL;
	ath79_register_eth(1);

	ath79_register_pci();
}

static void __init a40_setup(void)
{
	ath79_register_leds_gpio(-1, ARRAY_SIZE(a40_leds_gpio), a40_leds_gpio);
	a60_setup_common();
}

MIPS_MACHINE(ATH79_MACH_A40, "A40", "OpenMesh A40", a40_setup);

static void __init a60_setup(void)
{
	ath79_register_leds_gpio(-1, ARRAY_SIZE(a60_leds_gpio), a60_leds_gpio);
	a60_setup_common();
}

MIPS_MACHINE(ATH79_MACH_A60, "A60", "OpenMesh A60", a60_setup);

/*
 *  OpenMesh OM5P-AC support
 *
 *  Copyright (C) 2013 Marek Lindner <marek@open-mesh.com>
 *  Copyright (C) 2014 Sven Eckelmann <sven@open-mesh.com>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/gpio.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/i2c-algo-bit.h>
#include <linux/i2c-gpio.h>
#include <linux/platform_data/phy-at803x.h>

#include <asm/mach-ath79/ar71xx_regs.h>
#include <asm/mach-ath79/ath79.h>

#include "common.h"
#include "dev-ap9x-pci.h"
#include "dev-eth.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-wmac.h"
#include "machtypes.h"
#include "pci.h"

#define OM5PAC_GPIO_LED_POWER	18
#define OM5PAC_GPIO_LED_GREEN	21
#define OM5PAC_GPIO_LED_RED	23
#define OM5PAC_GPIO_LED_YELLOW	22
#define OM5PAC_GPIO_LED_LAN	20
#define OM5PAC_GPIO_LED_WAN	19
#define OM5PAC_GPIO_I2C_SCL	12
#define OM5PAC_GPIO_I2C_SDA	11

#define OM5PAC_KEYS_POLL_INTERVAL	20	/* msecs */
#define OM5PAC_KEYS_DEBOUNCE_INTERVAL	(3 * OM5PAC_KEYS_POLL_INTERVAL)

#define OM5PAC_WMAC_CALDATA_OFFSET	0x1000

static struct gpio_led om5pac_leds_gpio[] __initdata = {
	{
		.name		= "om5pac:blue:power",
		.gpio		= OM5PAC_GPIO_LED_POWER,
		.active_low	= 1,
	}, {
		.name		= "om5pac:red:wifi",
		.gpio		= OM5PAC_GPIO_LED_RED,
		.active_low	= 1,
	}, {
		.name		= "om5pac:yellow:wifi",
		.gpio		= OM5PAC_GPIO_LED_YELLOW,
		.active_low	= 1,
	}, {
		.name		= "om5pac:green:wifi",
		.gpio		= OM5PAC_GPIO_LED_GREEN,
		.active_low	= 1,
	}, {
		.name		= "om5pac:blue:lan",
		.gpio		= OM5PAC_GPIO_LED_LAN,
		.active_low	= 1,
	}, {
		.name		= "om5pac:blue:wan",
		.gpio		= OM5PAC_GPIO_LED_WAN,
		.active_low	= 1,
	}
};

static struct flash_platform_data om5pac_flash_data = {
	.type = "mx25l12805d",
};

static struct i2c_gpio_platform_data om5pac_i2c_device_platdata = {
	.sda_pin		= OM5PAC_GPIO_I2C_SDA,
	.scl_pin		= OM5PAC_GPIO_I2C_SCL,
	.udelay			= 10,
	.sda_is_open_drain	= 1,
	.scl_is_open_drain	= 1,
};

static struct platform_device om5pac_i2c_device = {
	.name		= "i2c-gpio",
	.id		= 0,
	.dev		= {
		.platform_data	= &om5pac_i2c_device_platdata,
	},
};

static struct i2c_board_info om5pac_i2c_devs[] __initdata = {
	{
		I2C_BOARD_INFO("tmp423", 0x4c),
	},
};

static struct at803x_platform_data om5pac_at803x_data = {
	.disable_smarteee = 1,
	.enable_rgmii_rx_delay = 1,
	.enable_rgmii_tx_delay = 1,
};

static struct mdio_board_info om5pac_mdio0_info[] = {
	{
		.bus_id = "ag71xx-mdio.0",
		.mdio_addr = 1,
		.platform_data = &om5pac_at803x_data,
	},
	{
		.bus_id = "ag71xx-mdio.0",
		.mdio_addr = 2,
		.platform_data = &om5pac_at803x_data,
	},
};

static void __init om5p_ac_setup_qca955x_eth_cfg(u32 mask,
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

static void __init om5p_ac_setup(void)
{
	u8 *art = (u8 *)KSEG1ADDR(0x1fff0000);
	u8 mac[6];

	/* temperature sensor */
	platform_device_register(&om5pac_i2c_device);
	i2c_register_board_info(0, om5pac_i2c_devs,
				ARRAY_SIZE(om5pac_i2c_devs));

	ath79_gpio_output_select(OM5PAC_GPIO_LED_WAN, QCA955X_GPIO_OUT_GPIO);

	ath79_register_m25p80(&om5pac_flash_data);
	ath79_register_leds_gpio(-1, ARRAY_SIZE(om5pac_leds_gpio),
				 om5pac_leds_gpio);

	ath79_init_mac(mac, art, 0x02);
	ath79_register_wmac(art + OM5PAC_WMAC_CALDATA_OFFSET, mac);

	om5p_ac_setup_qca955x_eth_cfg(QCA955X_ETH_CFG_RGMII_EN, 3, 3, 0, 0);
	ath79_register_mdio(0, 0x0);

	mdiobus_register_board_info(om5pac_mdio0_info,
				    ARRAY_SIZE(om5pac_mdio0_info));

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

MIPS_MACHINE(ATH79_MACH_OM5P_AC, "OM5P-AC", "OpenMesh OM5P AC", om5p_ac_setup);

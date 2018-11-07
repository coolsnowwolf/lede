/*
 *  OpenMesh OM5P support
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
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-wmac.h"
#include "machtypes.h"

#define OM5P_GPIO_LED_POWER	13
#define OM5P_GPIO_LED_GREEN	16
#define OM5P_GPIO_LED_RED	19
#define OM5P_GPIO_LED_YELLOW	17
#define OM5P_GPIO_LED_LAN	14
#define OM5P_GPIO_LED_WAN	15
#define OM5P_GPIO_BTN_RESET	4
#define OM5P_GPIO_I2C_SCL	20
#define OM5P_GPIO_I2C_SDA	21

#define OM5P_KEYS_POLL_INTERVAL		20	/* msecs */
#define OM5P_KEYS_DEBOUNCE_INTERVAL	(3 * OM5P_KEYS_POLL_INTERVAL)

#define OM5P_WMAC_CALDATA_OFFSET	0x1000
#define OM5P_PCI_CALDATA_OFFSET		0x5000

static struct gpio_led om5p_leds_gpio[] __initdata = {
	{
		.name		= "om5p:blue:power",
		.gpio		= OM5P_GPIO_LED_POWER,
		.active_low	= 1,
	}, {
		.name		= "om5p:red:wifi",
		.gpio		= OM5P_GPIO_LED_RED,
		.active_low	= 1,
	}, {
		.name		= "om5p:yellow:wifi",
		.gpio		= OM5P_GPIO_LED_YELLOW,
		.active_low	= 1,
	}, {
		.name		= "om5p:green:wifi",
		.gpio		= OM5P_GPIO_LED_GREEN,
		.active_low	= 1,
	}, {
		.name		= "om5p:blue:lan",
		.gpio		= OM5P_GPIO_LED_LAN,
		.active_low	= 1,
	}, {
		.name		= "om5p:blue:wan",
		.gpio		= OM5P_GPIO_LED_WAN,
		.active_low	= 1,
	}
};

static struct gpio_keys_button om5p_gpio_keys[] __initdata = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = OM5P_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= OM5P_GPIO_BTN_RESET,
		.active_low	= 1,
	}
};

static struct flash_platform_data om5p_flash_data = {
	.type = "mx25l12805d",
};

static void __init om5p_setup(void)
{
	u8 *art = (u8 *)KSEG1ADDR(0x1fff0000);
	u8 mac[6];

	/* make lan / wan leds software controllable */
	ath79_gpio_output_select(OM5P_GPIO_LED_LAN, AR934X_GPIO_OUT_GPIO);
	ath79_gpio_output_select(OM5P_GPIO_LED_WAN, AR934X_GPIO_OUT_GPIO);

	ath79_register_m25p80(&om5p_flash_data);
	ath79_register_leds_gpio(-1, ARRAY_SIZE(om5p_leds_gpio),
				 om5p_leds_gpio);
	ath79_register_gpio_keys_polled(-1, OM5P_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(om5p_gpio_keys),
					om5p_gpio_keys);

	ath79_init_mac(mac, art, 2);
	ath79_register_wmac(art + OM5P_WMAC_CALDATA_OFFSET, mac);

	ath79_setup_ar934x_eth_cfg(AR934X_ETH_CFG_SW_PHY_SWAP);
	ath79_register_mdio(1, 0x0);

	ath79_init_mac(ath79_eth0_data.mac_addr, art, 0);
	ath79_init_mac(ath79_eth1_data.mac_addr, art, 1);

	/* GMAC0 is connected to the PHY0 of the internal switch */
	ath79_switch_data.phy4_mii_en = 1;
	ath79_switch_data.phy_poll_mask = BIT(0);
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ath79_eth0_data.phy_mask = BIT(0);
	ath79_eth0_data.mii_bus_dev = &ath79_mdio1_device.dev;
	ath79_register_eth(0);

	/* GMAC1 is connected to the internal switch */
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_GMII;
	ath79_register_eth(1);
}

MIPS_MACHINE(ATH79_MACH_OM5P, "OM5P", "OpenMesh OM5P", om5p_setup);

static struct i2c_gpio_platform_data om5pan_i2c_device_platdata = {
	.sda_pin		= OM5P_GPIO_I2C_SDA,
	.scl_pin		= OM5P_GPIO_I2C_SCL,
	.udelay			= 10,
	.sda_is_open_drain	= 1,
	.scl_is_open_drain	= 1,
};

static struct platform_device om5pan_i2c_device = {
	.name		= "i2c-gpio",
	.id		= 0,
	.dev		= {
		.platform_data	= &om5pan_i2c_device_platdata,
	},
};

static struct i2c_board_info om5pan_i2c_devs[] __initdata = {
	{
		I2C_BOARD_INFO("tmp423", 0x4c),
	},
};

static struct at803x_platform_data om5p_an_at803x_data = {
	.disable_smarteee = 1,
	.enable_rgmii_rx_delay = 1,
	.enable_rgmii_tx_delay = 1,
};

static struct mdio_board_info om5p_an_mdio0_info[] = {
	{
		.bus_id = "ag71xx-mdio.0",
		.mdio_addr = 7,
		.platform_data = &om5p_an_at803x_data,
	},
};

static void __init om5p_an_setup(void)
{
	u8 *art = (u8 *)KSEG1ADDR(0x1fff0000);
	u8 mac[6];

	/* temperature sensor */
	platform_device_register(&om5pan_i2c_device);
	i2c_register_board_info(0, om5pan_i2c_devs,
				ARRAY_SIZE(om5pan_i2c_devs));

	/* make lan / wan leds software controllable */
	ath79_gpio_output_select(OM5P_GPIO_LED_LAN, AR934X_GPIO_OUT_GPIO);
	ath79_gpio_output_select(OM5P_GPIO_LED_WAN, AR934X_GPIO_OUT_GPIO);

	ath79_register_m25p80(&om5p_flash_data);
	ath79_register_leds_gpio(-1, ARRAY_SIZE(om5p_leds_gpio),
				 om5p_leds_gpio);

	ath79_init_mac(mac, art, 0x02);
	ath79_register_wmac(art + OM5P_WMAC_CALDATA_OFFSET, mac);

	ath79_setup_ar934x_eth_cfg(AR934X_ETH_CFG_RGMII_GMAC0);
	ath79_setup_ar934x_eth_rx_delay(2, 2);
	ath79_register_mdio(0, 0x0);
	ath79_register_mdio(1, 0x0);

	mdiobus_register_board_info(om5p_an_mdio0_info,
				    ARRAY_SIZE(om5p_an_mdio0_info));

	ath79_init_mac(ath79_eth0_data.mac_addr, art, 0x00);
	ath79_init_mac(ath79_eth1_data.mac_addr, art, 0x01);

	/* GMAC0 is connected to the PHY7 */
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth0_data.mii_bus_dev = &ath79_mdio0_device.dev;
	ath79_eth0_data.phy_mask = BIT(7);
	ath79_eth0_pll_data.pll_1000 = 0x02000000;
	ath79_eth0_pll_data.pll_100 = 0x00000101;
	ath79_eth0_pll_data.pll_10 = 0x00001313;
	ath79_register_eth(0);

	/* GMAC1 is connected to the internal switch */
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_GMII;
	ath79_eth1_data.mii_bus_dev = &ath79_mdio1_device.dev;
	ath79_register_eth(1);

	ath79_init_mac(mac, art, 0x10);
	ap91_pci_init(art + OM5P_PCI_CALDATA_OFFSET, mac);
}

MIPS_MACHINE(ATH79_MACH_OM5P_AN, "OM5P-AN", "OpenMesh OM5P AN", om5p_an_setup);

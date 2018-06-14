/*
 *  OpenMesh OM2P support
 *
 *  Copyright (C) 2011 Marek Lindner <marek@open-mesh.com>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/gpio.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/platform_device.h>

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

#define OM2P_GPIO_LED_POWER	0
#define OM2P_GPIO_LED_GREEN	13
#define OM2P_GPIO_LED_RED	14
#define OM2P_GPIO_LED_YELLOW	15
#define OM2P_GPIO_LED_LAN	16
#define OM2P_GPIO_LED_WAN	17
#define OM2P_GPIO_BTN_RESET	1

#define OM2P_KEYS_POLL_INTERVAL		20	/* msecs */
#define OM2P_KEYS_DEBOUNCE_INTERVAL	(3 * OM2P_KEYS_POLL_INTERVAL)

#define OM2P_WAN_PHYMASK	BIT(4)

#define OM2P_LC_GPIO_LED_POWER	1
#define OM2P_LC_GPIO_LED_GREEN	15
#define OM2P_LC_GPIO_LED_RED	16
#define OM2P_LC_GPIO_LED_YELLOW	0
#define OM2P_LC_GPIO_LED_LAN	13
#define OM2P_LC_GPIO_LED_WAN	17
#define OM2P_LC_GPIO_BTN_RESET	12

#define OM2Pv4_GPIO_LED_POWER	0
#define OM2Pv4_GPIO_LED_GREEN	2
#define OM2Pv4_GPIO_LED_RED	4
#define OM2Pv4_GPIO_LED_YELLOW	3
#define OM2Pv4_GPIO_LED_LAN	14
#define OM2Pv4_GPIO_LED_WAN	13
#define OM2Pv4_GPIO_BTN_RESET	1

#define OM2P_WMAC_CALDATA_OFFSET	0x1000

static struct flash_platform_data om2p_flash_data = {
	.type = "s25sl12800",
	.name = "ar7240-nor0",
};

static struct gpio_led om2p_leds_gpio[] __initdata = {
	{
		.name		= "om2p:blue:power",
		.gpio		= OM2P_GPIO_LED_POWER,
		.active_low	= 1,
	}, {
		.name		= "om2p:red:wifi",
		.gpio		= OM2P_GPIO_LED_RED,
		.active_low	= 1,
	}, {
		.name		= "om2p:yellow:wifi",
		.gpio		= OM2P_GPIO_LED_YELLOW,
		.active_low	= 1,
	}, {
		.name		= "om2p:green:wifi",
		.gpio		= OM2P_GPIO_LED_GREEN,
		.active_low	= 1,
	}, {
		.name		= "om2p:blue:lan",
		.gpio		= OM2P_GPIO_LED_LAN,
		.active_low	= 1,
	}, {
		.name		= "om2p:blue:wan",
		.gpio		= OM2P_GPIO_LED_WAN,
		.active_low	= 1,
	}
};

static struct gpio_keys_button om2p_gpio_keys[] __initdata = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = OM2P_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= OM2P_GPIO_BTN_RESET,
		.active_low	= 1,
	}
};

static void __init om2p_setup(void)
{
	u8 *mac1 = (u8 *)KSEG1ADDR(0x1ffc0000);
	u8 *mac2 = (u8 *)KSEG1ADDR(0x1ffc0000 + ETH_ALEN);
	u8 *ee = (u8 *)KSEG1ADDR(0x1ffc1000);

	ath79_gpio_function_disable(AR724X_GPIO_FUNC_ETH_SWITCH_LED0_EN |
				    AR724X_GPIO_FUNC_ETH_SWITCH_LED1_EN |
				    AR724X_GPIO_FUNC_ETH_SWITCH_LED2_EN |
				    AR724X_GPIO_FUNC_ETH_SWITCH_LED3_EN |
				    AR724X_GPIO_FUNC_ETH_SWITCH_LED4_EN);

	ath79_register_m25p80(&om2p_flash_data);

	ath79_register_mdio(0, ~OM2P_WAN_PHYMASK);

	ath79_init_mac(ath79_eth0_data.mac_addr, mac1, 0);
	ath79_init_mac(ath79_eth1_data.mac_addr, mac2, 0);

	ath79_register_eth(0);
	ath79_register_eth(1);

	ap91_pci_init(ee, NULL);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(om2p_leds_gpio),
				 om2p_leds_gpio);

	ath79_register_gpio_keys_polled(-1, OM2P_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(om2p_gpio_keys),
					om2p_gpio_keys);
}

MIPS_MACHINE(ATH79_MACH_OM2P, "OM2P", "OpenMesh OM2P", om2p_setup);


static struct flash_platform_data om2p_lc_flash_data = {
	.type = "s25sl12800",
};

static void __init om2p_lc_setup(void)
{
	u8 *mac1 = (u8 *)KSEG1ADDR(0x1ffc0000);
	u8 *mac2 = (u8 *)KSEG1ADDR(0x1ffc0000 + ETH_ALEN);
	u8 *art = (u8 *)KSEG1ADDR(0x1ffc1000);
	u32 t;

	ath79_gpio_function_disable(AR933X_GPIO_FUNC_ETH_SWITCH_LED0_EN |
				    AR933X_GPIO_FUNC_ETH_SWITCH_LED1_EN |
				    AR933X_GPIO_FUNC_ETH_SWITCH_LED2_EN |
				    AR933X_GPIO_FUNC_ETH_SWITCH_LED3_EN |
				    AR933X_GPIO_FUNC_ETH_SWITCH_LED4_EN);

	t = ath79_reset_rr(AR933X_RESET_REG_BOOTSTRAP);
	t |= AR933X_BOOTSTRAP_MDIO_GPIO_EN;
	ath79_reset_wr(AR933X_RESET_REG_BOOTSTRAP, t);

	ath79_register_m25p80(&om2p_lc_flash_data);

	om2p_leds_gpio[0].gpio = OM2P_LC_GPIO_LED_POWER;
	om2p_leds_gpio[1].gpio = OM2P_LC_GPIO_LED_RED;
	om2p_leds_gpio[2].gpio = OM2P_LC_GPIO_LED_YELLOW;
	om2p_leds_gpio[3].gpio = OM2P_LC_GPIO_LED_GREEN;
	om2p_leds_gpio[4].gpio = OM2P_LC_GPIO_LED_LAN;
	om2p_leds_gpio[5].gpio = OM2P_LC_GPIO_LED_WAN;
	ath79_register_leds_gpio(-1, ARRAY_SIZE(om2p_leds_gpio),
				 om2p_leds_gpio);

	om2p_gpio_keys[0].gpio = OM2P_LC_GPIO_BTN_RESET;
	ath79_register_gpio_keys_polled(-1, OM2P_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(om2p_gpio_keys),
					om2p_gpio_keys);

	ath79_init_mac(ath79_eth0_data.mac_addr, mac1, 0);
	ath79_init_mac(ath79_eth1_data.mac_addr, mac2, 0);

	ath79_register_mdio(0, 0x0);

	ath79_register_eth(0);
	ath79_register_eth(1);

	ath79_register_wmac(art, NULL);
}

MIPS_MACHINE(ATH79_MACH_OM2P_LC, "OM2P-LC", "OpenMesh OM2P LC", om2p_lc_setup);
MIPS_MACHINE(ATH79_MACH_OM2Pv2, "OM2Pv2", "OpenMesh OM2Pv2", om2p_lc_setup);

static void __init om2p_hs_setup(void)
{
	u8 *mac1 = (u8 *)KSEG1ADDR(0x1ffc0000);
	u8 *mac2 = (u8 *)KSEG1ADDR(0x1ffc0000 + ETH_ALEN);
	u8 *art = (u8 *)KSEG1ADDR(0x1ffc1000);

	/* make lan / wan leds software controllable */
	ath79_gpio_output_select(OM2P_GPIO_LED_LAN, AR934X_GPIO_OUT_GPIO);
	ath79_gpio_output_select(OM2P_GPIO_LED_WAN, AR934X_GPIO_OUT_GPIO);

	/* enable reset button */
	ath79_gpio_output_select(OM2P_GPIO_BTN_RESET, AR934X_GPIO_OUT_GPIO);
	ath79_gpio_function_enable(AR934X_GPIO_FUNC_JTAG_DISABLE);

	om2p_leds_gpio[4].gpio = OM2P_GPIO_LED_WAN;
	om2p_leds_gpio[5].gpio = OM2P_GPIO_LED_LAN;

	ath79_register_m25p80(&om2p_lc_flash_data);
	ath79_register_leds_gpio(-1, ARRAY_SIZE(om2p_leds_gpio),
				 om2p_leds_gpio);
	ath79_register_gpio_keys_polled(-1, OM2P_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(om2p_gpio_keys),
					om2p_gpio_keys);

	ath79_register_wmac(art, NULL);

	ath79_setup_ar934x_eth_cfg(AR934X_ETH_CFG_SW_PHY_SWAP);
	ath79_register_mdio(1, 0x0);

	ath79_init_mac(ath79_eth0_data.mac_addr, mac1, 0);
	ath79_init_mac(ath79_eth1_data.mac_addr, mac2, 0);

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

MIPS_MACHINE(ATH79_MACH_OM2P_HS, "OM2P-HS", "OpenMesh OM2P HS", om2p_hs_setup);
MIPS_MACHINE(ATH79_MACH_OM2P_HSv2, "OM2P-HSv2", "OpenMesh OM2P HSv2", om2p_hs_setup);
MIPS_MACHINE(ATH79_MACH_OM2P_HSv3, "OM2P-HSv3", "OpenMesh OM2P HSv3", om2p_hs_setup);

static struct flash_platform_data om2pv4_flash_data = {
	.type = "s25sl12800",
};

static struct gpio_led om2pv4_leds_gpio[] __initdata = {
	{
		.name		= "om2p:blue:power",
		.gpio		= OM2Pv4_GPIO_LED_POWER,
		.active_low	= 1,
	}, {
		.name		= "om2p:red:wifi",
		.gpio		= OM2Pv4_GPIO_LED_RED,
		.active_low	= 1,
	}, {
		.name		= "om2p:yellow:wifi",
		.gpio		= OM2Pv4_GPIO_LED_YELLOW,
		.active_low	= 1,
	}, {
		.name		= "om2p:green:wifi",
		.gpio		= OM2Pv4_GPIO_LED_GREEN,
		.active_low	= 1,
	}, {
		.name		= "om2p:blue:lan",
		.gpio		= OM2Pv4_GPIO_LED_LAN,
		.active_low	= 1,
	}, {
		.name		= "om2p:blue:wan",
		.gpio		= OM2Pv4_GPIO_LED_WAN,
		.active_low	= 1,
	}
};

static struct gpio_keys_button om2pv4_gpio_keys[] __initdata = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = OM2P_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= OM2Pv4_GPIO_BTN_RESET,
		.active_low	= 1,
	}
};

static void __init om2pv4_setup(void)
{
	u8 *mac1 = (u8 *)KSEG1ADDR(0x1ffc0000);
	u8 *mac2 = (u8 *)KSEG1ADDR(0x1ffc0000 + ETH_ALEN);
	u8 *art = (u8 *)KSEG1ADDR(0x1ffc0000);
	u8 wmac[6];

	ath79_register_m25p80(&om2pv4_flash_data);
	ath79_register_leds_gpio(-1, ARRAY_SIZE(om2pv4_leds_gpio),
				 om2pv4_leds_gpio);
	ath79_register_gpio_keys_polled(-1, OM2P_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(om2pv4_gpio_keys),
					om2pv4_gpio_keys);

	ath79_init_mac(wmac, art, 0x02);
	ath79_register_wmac(art + OM2P_WMAC_CALDATA_OFFSET, wmac);

	ath79_setup_ar933x_phy4_switch(false, false);

	ath79_register_mdio(0, 0x0);

	/* LAN */
	ath79_switch_data.phy4_mii_en = 1;
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ath79_eth0_data.duplex = DUPLEX_FULL;
	ath79_eth0_data.speed = SPEED_100;
	ath79_eth0_data.phy_mask = BIT(4);
	ath79_init_mac(ath79_eth1_data.mac_addr, mac2, 0);
	ath79_register_eth(0);

	/* WAN */
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_GMII;
	ath79_eth1_data.duplex = DUPLEX_FULL;
	ath79_switch_data.phy_poll_mask |= BIT(4);
	ath79_init_mac(ath79_eth0_data.mac_addr, mac1, 0);
	ath79_register_eth(1);
}

MIPS_MACHINE(ATH79_MACH_OM2Pv4, "OM2Pv4", "OpenMesh OM2Pv4", om2pv4_setup);
MIPS_MACHINE(ATH79_MACH_OM2P_HSv4, "OM2P-HSv4", "OpenMesh OM2P HSv4", om2pv4_setup);

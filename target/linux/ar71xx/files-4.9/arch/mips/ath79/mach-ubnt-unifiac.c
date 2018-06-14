/*
 *  Ubiquiti UniFi AC (LITE) board support
 *
 *  Copyright (C) 2015-2016 P. Wassi <p.wassi at gmx.at>
 *
 *  Derived from: mach-ubnt-xm.c
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/pci.h>
#include <linux/platform_device.h>
#include <linux/ath9k_platform.h>
#include <linux/etherdevice.h>

#include <asm/mach-ath79/ath79.h>
#include <asm/mach-ath79/irq.h>
#include <asm/mach-ath79/ar71xx_regs.h>

#include <linux/platform_data/phy-at803x.h>
#include <linux/ar8216_platform.h>

#include "common.h"
#include "dev-ap9x-pci.h"
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-wmac.h"
#include "dev-usb.h"
#include "machtypes.h"


#define UNIFIAC_KEYS_POLL_INTERVAL	20
#define UNIFIAC_KEYS_DEBOUNCE_INTERVAL	(3 * UNIFIAC_KEYS_POLL_INTERVAL)

#define UNIFIAC_GPIO_LED_WHITE		7
#define UNIFIAC_GPIO_LED_BLUE		8

#define UNIFIAC_GPIO_BTN_RESET		2

#define UNIFIAC_MAC0_OFFSET             0x0000
#define UNIFIAC_WMAC_CALDATA_OFFSET     0x1000
#define UNIFIAC_PCI_CALDATA_OFFSET      0x5000


static struct flash_platform_data ubnt_unifiac_flash_data = {
	/* mx25l12805d and mx25l12835f have the same JEDEC ID */
	.type = "mx25l12805d",
};

static struct gpio_led ubnt_unifiac_leds_gpio[] __initdata = {
	{
		.name		= "ubnt:white:dome",
		.gpio		= UNIFIAC_GPIO_LED_WHITE,
		.active_low	= 0,
	}, {
		.name		= "ubnt:blue:dome",
		.gpio		= UNIFIAC_GPIO_LED_BLUE,
		.active_low	= 0,
	}
};

static struct gpio_keys_button ubnt_unifiac_gpio_keys[] __initdata = {
	{
		.desc			= "reset",
		.type			= EV_KEY,
		.code			= KEY_RESTART,
		.debounce_interval	= UNIFIAC_KEYS_DEBOUNCE_INTERVAL,
		.gpio			= UNIFIAC_GPIO_BTN_RESET,
		.active_low		= 1,
	}
};

static void __init ubnt_unifiac_lite_setup(void)
{
	u8 *eeprom = (u8 *) KSEG1ADDR(0x1fff0000);

	ath79_register_m25p80(&ubnt_unifiac_flash_data);


	ath79_init_mac(ath79_eth0_data.mac_addr,
	               eeprom + UNIFIAC_MAC0_OFFSET, 0);

	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_SGMII;
	ath79_eth0_data.mii_bus_dev = &ath79_mdio0_device.dev;
	ath79_eth0_data.phy_mask = BIT(4);
	ath79_eth0_pll_data.pll_10 = 0x00001313;

	ath79_register_mdio(0, ~BIT(4));
	ath79_register_eth(0);


	ath79_register_wmac(eeprom + UNIFIAC_WMAC_CALDATA_OFFSET, NULL);


	ap91_pci_init(eeprom + UNIFIAC_PCI_CALDATA_OFFSET, NULL);


	ath79_register_leds_gpio(-1, ARRAY_SIZE(ubnt_unifiac_leds_gpio),
	                         ubnt_unifiac_leds_gpio);

	ath79_register_gpio_keys_polled(-1, UNIFIAC_KEYS_POLL_INTERVAL,
	                                ARRAY_SIZE(ubnt_unifiac_gpio_keys),
	                                ubnt_unifiac_gpio_keys);
}

MIPS_MACHINE(ATH79_MACH_UBNT_UNIFIAC_LITE, "UBNT-UF-AC-LITE",
	     "Ubiquiti UniFi-AC-LITE/MESH", ubnt_unifiac_lite_setup);

static struct ar8327_pad_cfg ubnt_unifiac_pro_ar8327_pad0_cfg = {
	.mode = AR8327_PAD_MAC_SGMII,
	.sgmii_delay_en = true,
};

static struct ar8327_platform_data ubnt_unifiac_pro_ar8327_data = {
	.pad0_cfg = &ubnt_unifiac_pro_ar8327_pad0_cfg,
	.port0_cfg = {
		.force_link = 1,
		.speed = AR8327_PORT_SPEED_1000,
		.duplex = 1,
		.txpause = 1,
		.rxpause = 1,
	},
};


static struct mdio_board_info ubnt_unifiac_pro_mdio0_info[] = {
	{
		.bus_id = "ag71xx-mdio.0",
		.phy_addr = 0,
		.platform_data = &ubnt_unifiac_pro_ar8327_data,
	},
};

static void __init ubnt_unifiac_pro_setup(void)
{
	u8 *eeprom = (u8 *) KSEG1ADDR(0x1fff0000);

	ath79_register_m25p80(&ubnt_unifiac_flash_data);


	ath79_init_mac(ath79_eth0_data.mac_addr,
	               eeprom + UNIFIAC_MAC0_OFFSET, 0);

	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_SGMII;
	ath79_eth0_data.mii_bus_dev = &ath79_mdio0_device.dev;
	ath79_eth0_data.phy_mask = BIT(0);

	mdiobus_register_board_info(ubnt_unifiac_pro_mdio0_info,
	                            ARRAY_SIZE(ubnt_unifiac_pro_mdio0_info));

	ath79_register_mdio(0, 0x00);
	ath79_register_eth(0);


	ath79_register_usb();


	ath79_register_wmac(eeprom + UNIFIAC_WMAC_CALDATA_OFFSET, NULL);


	ap91_pci_init(eeprom + UNIFIAC_PCI_CALDATA_OFFSET, NULL);


	ath79_register_leds_gpio(-1, ARRAY_SIZE(ubnt_unifiac_leds_gpio),
	                         ubnt_unifiac_leds_gpio);

	ath79_register_gpio_keys_polled(-1, UNIFIAC_KEYS_POLL_INTERVAL,
	                                ARRAY_SIZE(ubnt_unifiac_gpio_keys),
	                                ubnt_unifiac_gpio_keys);
}


MIPS_MACHINE(ATH79_MACH_UBNT_UNIFIAC_PRO, "UBNT-UF-AC-PRO",
	     "Ubiquiti UniFi-AC-PRO", ubnt_unifiac_pro_setup);

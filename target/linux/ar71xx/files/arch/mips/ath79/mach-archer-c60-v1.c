/*
 *  TP-Link Archer C60 v1 board support
 *
 *  Copyright (C) 2017 Henryk Heisig <hyniu@o2.pl>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */
#include <linux/platform_device.h>
#include <linux/ath9k_platform.h>
#include <linux/ar8216_platform.h>
#include <asm/mach-ath79/ar71xx_regs.h>
#include <linux/gpio.h>

#include "common.h"
#include "dev-m25p80.h"
#include "machtypes.h"
#include "pci.h"
#include "dev-ap9x-pci.h"
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-spi.h"
#include "dev-usb.h"
#include "dev-wmac.h"

#define ARCHER_C60_V1_GPIO_LED_LAN		2
#define ARCHER_C60_V1_GPIO_LED_POWER		16
#define ARCHER_C60_V1_GPIO_LED_WLAN2		17
#define ARCHER_C60_V1_GPIO_LED_WLAN5		18
#define ARCHER_C60_V1_GPIO_LED_WPS		19
#define ARCHER_C60_V1_GPIO_LED_WAN_GREEN	20
#define ARCHER_C60_V1_GPIO_LED_WAN_AMBER	22


#define ARCHER_C60_V1_KEYS_POLL_INTERVAL	20
#define ARCHER_C60_V1_KEYS_DEBOUNCE_INTERVAL	(3 * ARCHER_C60_V1_KEYS_POLL_INTERVAL)

#define ARCHER_C60_V1_GPIO_BTN_RESET		21
#define ARCHER_C60_V1_GPIO_BTN_RFKILL		1



#define ARCHER_C60_V1_WMAC_CALDATA_OFFSET	0x1000
#define ARCHER_C60_V1_PCI_CALDATA_OFFSET	0x5000

static struct gpio_led archer_c60_v1_leds_gpio[] __initdata = {
	{
		.name		= "archer-c60-v1:green:power",
		.gpio		= ARCHER_C60_V1_GPIO_LED_POWER,
		.active_low	= 1,
	},
	{
		.name		= "archer-c60-v1:green:wlan2g",
		.gpio		= ARCHER_C60_V1_GPIO_LED_WLAN2,
		.active_low	= 1,
	},
	{
		.name		= "archer-c60-v1:green:wlan5g",
		.gpio		= ARCHER_C60_V1_GPIO_LED_WLAN5,
		.active_low	= 1,
	},
	{
		.name		= "archer-c60-v1:green:lan",
		.gpio		= ARCHER_C60_V1_GPIO_LED_LAN,
		.active_low	= 1,
	},
	{
		.name		= "archer-c60-v1:green:wan",
		.gpio		= ARCHER_C60_V1_GPIO_LED_WAN_GREEN,
		.active_low	= 1,
	},
	{
		.name		= "archer-c60-v1:amber:wan",
		.gpio		= ARCHER_C60_V1_GPIO_LED_WAN_AMBER,
		.active_low	= 1,
	},
	{
		.name		= "archer-c60-v1:green:wps",
		.gpio		= ARCHER_C60_V1_GPIO_LED_WPS,
		.active_low	= 1,
	},
};

static struct gpio_led archer_c60_v2_leds_gpio[] __initdata = {
	{
		.name		= "archer-c60-v2:green:power",
		.gpio		= ARCHER_C60_V1_GPIO_LED_POWER,
		.active_low	= 1,
	},
	{
		.name		= "archer-c60-v2:green:wlan2g",
		.gpio		= ARCHER_C60_V1_GPIO_LED_WLAN2,
		.active_low	= 1,
	},
	{
		.name		= "archer-c60-v2:green:wlan5g",
		.gpio		= ARCHER_C60_V1_GPIO_LED_WLAN5,
		.active_low	= 1,
	},
	{
		.name		= "archer-c60-v2:green:lan",
		.gpio		= ARCHER_C60_V1_GPIO_LED_LAN,
		.active_low	= 1,
	},
	{
		.name		= "archer-c60-v2:green:wan",
		.gpio		= ARCHER_C60_V1_GPIO_LED_WAN_GREEN,
		.active_low	= 1,
	},
	{
		.name		= "archer-c60-v2:amber:wan",
		.gpio		= ARCHER_C60_V1_GPIO_LED_WAN_AMBER,
		.active_low	= 1,
	},
	{
		.name		= "archer-c60-v2:green:wps",
		.gpio		= ARCHER_C60_V1_GPIO_LED_WPS,
		.active_low	= 1,
	},
};

static struct gpio_keys_button archer_c60_v1_gpio_keys[] __initdata = {
	{
		.desc			= "Reset button",
		.type			= EV_KEY,
		.code			= KEY_RESTART,
		.debounce_interval	= ARCHER_C60_V1_KEYS_DEBOUNCE_INTERVAL,
		.gpio			= ARCHER_C60_V1_GPIO_BTN_RESET,
		.active_low		= 1,
	},
	{
		.desc			= "RFKILL button",
		.type			= EV_KEY,
		.code			= KEY_RFKILL,
		.debounce_interval	= ARCHER_C60_V1_KEYS_DEBOUNCE_INTERVAL,
		.gpio			= ARCHER_C60_V1_GPIO_BTN_RFKILL,
		.active_low		= 1,
	},
};

static void __init archer_c60_v1_setup(void)
{
	u8 *mac = (u8 *) KSEG1ADDR(0x1f010008);
	u8 *art = (u8 *) KSEG1ADDR(0x1f7f0000);

	ath79_register_m25p80(NULL);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(archer_c60_v1_leds_gpio),
				archer_c60_v1_leds_gpio);

	ath79_register_gpio_keys_polled(-1, ARCHER_C60_V1_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(archer_c60_v1_gpio_keys),
					archer_c60_v1_gpio_keys);

	ath79_register_mdio(0, 0x0);
	ath79_register_mdio(1, 0x0);

	ath79_init_mac(ath79_eth1_data.mac_addr, mac, 0);
	ath79_init_mac(ath79_eth0_data.mac_addr, mac, 1);

	/* WAN port */
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ath79_eth0_data.speed = SPEED_100;
	ath79_eth0_data.duplex = DUPLEX_FULL;
	ath79_eth0_data.phy_mask = BIT(4);
	ath79_register_eth(0);

	/* LAN ports */
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_GMII;
	ath79_eth1_data.speed = SPEED_1000;
	ath79_eth1_data.duplex = DUPLEX_FULL;
	ath79_switch_data.phy_poll_mask |= BIT(4);
	ath79_switch_data.phy4_mii_en = 1;
	ath79_register_eth(1);

	ath79_register_wmac(art + ARCHER_C60_V1_WMAC_CALDATA_OFFSET, mac);
	ap91_pci_init(art + ARCHER_C60_V1_PCI_CALDATA_OFFSET, NULL);
}

MIPS_MACHINE(ATH79_MACH_ARCHER_C60_V1, "ARCHER-C60-V1",
	"TP-LINK Archer C60 v1", archer_c60_v1_setup);

static void __init archer_c60_v2_setup(void)
{
	u8 *mac = (u8 *) KSEG1ADDR(0x1f01fb08);
	u8 *art = (u8 *) KSEG1ADDR(0x1f7f0000);

	ath79_register_m25p80(NULL);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(archer_c60_v2_leds_gpio),
				archer_c60_v2_leds_gpio);

	ath79_register_gpio_keys_polled(-1, ARCHER_C60_V1_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(archer_c60_v1_gpio_keys),
					archer_c60_v1_gpio_keys);

	ath79_register_mdio(0, 0x0);
	ath79_register_mdio(1, 0x0);

	ath79_init_mac(ath79_eth1_data.mac_addr, mac, 0);
	ath79_init_mac(ath79_eth0_data.mac_addr, mac, 1);

	/* WAN port */
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ath79_eth0_data.speed = SPEED_100;
	ath79_eth0_data.duplex = DUPLEX_FULL;
	ath79_eth0_data.phy_mask = BIT(4);
	ath79_register_eth(0);

	/* LAN ports */
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_GMII;
	ath79_eth1_data.speed = SPEED_1000;
	ath79_eth1_data.duplex = DUPLEX_FULL;
	ath79_switch_data.phy_poll_mask |= BIT(4);
	ath79_switch_data.phy4_mii_en = 1;
	ath79_register_eth(1);

	ath79_register_wmac(art + ARCHER_C60_V1_WMAC_CALDATA_OFFSET, mac);
	ap91_pci_init(art + ARCHER_C60_V1_PCI_CALDATA_OFFSET, NULL);
}

MIPS_MACHINE(ATH79_MACH_ARCHER_C60_V2, "ARCHER-C60-V2",
	"TP-LINK Archer C60 v2", archer_c60_v2_setup);

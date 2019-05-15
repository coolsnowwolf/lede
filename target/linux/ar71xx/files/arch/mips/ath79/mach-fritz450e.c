/*
 *  AVM FRITZ!WLAN Repeater 450E board support
 *
 *  Copyright (C) 2018 David Bauer <mail@david-bauer.net>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/ath9k_platform.h>
#include <linux/etherdevice.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/platform_data/mdio-gpio.h>
#include <linux/platform_data/phy-at803x.h>
#include <linux/spi/spi_gpio.h>
#include <linux/spi/74x164.h>

#include <asm/mach-ath79/ath79.h>
#include <asm/mach-ath79/ar71xx_regs.h>

#include "common.h"
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-spi.h"
#include "dev-wmac.h"
#include "machtypes.h"

#define FRITZ450E_GPIO_LED_POWER		14
#define FRITZ450E_GPIO_LED_LAN			13
#define FRITZ450E_GPIO_LED_WLAN			15
#define FRITZ450E_GPIO_LED_RSSI2			16
#define FRITZ450E_GPIO_LED_RSSI3			17
#define FRITZ450E_GPIO_LED_RSSI4			18

#define FRITZ450E_GPIO_BTN_WPS			4
#define FRITZ450E_KEYS_POLL_INTERVAL		20 /* msecs */
#define FRITZ450E_KEYS_DEBOUNCE_INTERVAL	(3 * FRITZ450E_KEYS_POLL_INTERVAL)

#define FRITZ450E_PHY_ADDRESS			0
#define FRITZ450E_GPIO_PHY_RESET		11
#define FRITZ450E_GPIO_MDIO_CLK		12
#define FRITZ450E_GPIO_MDIO_DATA		19

#define FRITZ450E_OFFSET_URLADER_WIFI_MAC_REVERSE	0x1979


static struct mtd_partition fritz450E_flash_partitions[] = {
	{
		.name		= "urlader",
		.offset		= 0,
		.size		= 0x0020000,
		.mask_flags	= MTD_WRITEABLE,
	}, {
		.name		= "firmware",
		.offset		= 0x0020000,
		.size		= 0x0EE0000,
	}, {
		.name		= "tffs (1)",
		.offset		= 0x0f00000,
		.size		= 0x0080000,
		.mask_flags	= MTD_WRITEABLE,
	}, {
		.name		= "tffs (2)",
		.offset		= 0x0f80000,
		.size		= 0x0080000,
		.mask_flags	= MTD_WRITEABLE,
	}
};

static struct flash_platform_data fritz450E_flash_data = {
	.parts		= fritz450E_flash_partitions,
	.nr_parts	= ARRAY_SIZE(fritz450E_flash_partitions),
};

static struct gpio_led fritz450E_leds_gpio[] __initdata = {
	{
		.name		= "fritz450e:green:lan",
		.gpio		= FRITZ450E_GPIO_LED_LAN,
		.active_low	= 1,
	}, {
		.name		= "fritz450e:green:rssi2",
		.gpio		= FRITZ450E_GPIO_LED_RSSI2,
		.active_low	= 1,
	}, {
		.name		= "fritz450e:green:rssi3",
		.gpio		= FRITZ450E_GPIO_LED_RSSI3,
		.active_low	= 1,
	}, {
		.name		= "fritz450e:green:rssi4",
		.gpio		= FRITZ450E_GPIO_LED_RSSI4,
		.active_low	= 1,
	}, {
		.name		= "fritz450e:green:wlan",
		.gpio		= FRITZ450E_GPIO_LED_WLAN,
		.active_low	= 1,
	}, {
		.name		= "fritz450e:green:power",
		.gpio		= FRITZ450E_GPIO_LED_POWER,
		.active_low	= 0,
	},
};

static struct gpio_keys_button fritz450E_gpio_keys[] __initdata = {
	{
		.desc			= "WPS Button",
		.type			= EV_KEY,
		.code			= KEY_WPS_BUTTON,
		.debounce_interval	= FRITZ450E_KEYS_DEBOUNCE_INTERVAL,
		.gpio			= FRITZ450E_GPIO_BTN_WPS,
		.active_low		= 1,
	}
};

static struct at803x_platform_data fritz450E_at803x_data = {
	.disable_smarteee = 1,
	.has_reset_gpio = 1,
	.override_sgmii_aneg = 1,
	.reset_gpio = FRITZ450E_GPIO_PHY_RESET,
};

static struct mdio_board_info fritz450E_mdio_info[] = {
	{
		.bus_id = "ag71xx-mdio.1",
		.mdio_addr = FRITZ450E_PHY_ADDRESS,
		.platform_data = &fritz450E_at803x_data,
	},
};

static void __init fritz450E_setup(void) {
	u8 *urlader = (u8 *) KSEG1ADDR(0x1f000000);
	u8 wifi_mac[ETH_ALEN];

	ath79_register_m25p80(&fritz450E_flash_data);

	gpio_request_one(FRITZ450E_GPIO_MDIO_CLK, GPIOF_OUT_INIT_HIGH, "MDC Pull-UP");
	gpio_request_one(FRITZ450E_GPIO_MDIO_DATA, GPIOF_OUT_INIT_HIGH, "MDIO Pull-UP");
	gpio_request_one(FRITZ450E_GPIO_PHY_RESET, GPIOF_OUT_INIT_HIGH, "PHY reset");

	/* Register PHY device */
	mdiobus_register_board_info(fritz450E_mdio_info,
				    ARRAY_SIZE(fritz450E_mdio_info));

	/* Initialize Ethernet */
	ath79_extract_mac_reverse(urlader + FRITZ450E_OFFSET_URLADER_WIFI_MAC_REVERSE, wifi_mac);
	ath79_init_mac(ath79_eth0_data.mac_addr, wifi_mac, -2);

	ath79_register_mdio(1, ~BIT(FRITZ450E_PHY_ADDRESS));
	ath79_eth0_data.mii_bus_dev = &ath79_mdio1_device.dev;
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_SGMII;
	ath79_eth0_data.phy_mask = BIT(FRITZ450E_PHY_ADDRESS);
	ath79_eth0_data.enable_sgmii_fixup = 1;
	ath79_eth0_pll_data.pll_1000 = 0x03000000;
	ath79_eth0_pll_data.pll_100 = 0x00000101;
	ath79_eth0_pll_data.pll_10 = 0x00001313;
	ath79_register_eth(0);

	/* Initialize 2.4GHz WiFi */
	ath79_register_wmac_simple();

	/* Register GPIO buttons */
	ath79_register_gpio_keys_polled(-1, FRITZ450E_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(fritz450E_gpio_keys),
					fritz450E_gpio_keys);

	/* Register LEDs */
	ath79_register_leds_gpio(-1, ARRAY_SIZE(fritz450E_leds_gpio),
				 fritz450E_leds_gpio);
}

MIPS_MACHINE(ATH79_MACH_FRITZ450E, "FRITZ450E",
	     "AVM FRITZ!WLAN Repeater 450E", fritz450E_setup);

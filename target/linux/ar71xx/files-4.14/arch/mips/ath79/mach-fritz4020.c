/*
 *  AVM FRITZ!Box 4020 board support
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
#include "dev-usb.h"
#include "dev-wmac.h"
#include "machtypes.h"


#define FRITZ4020_GPIO_SHIFT_SER			19   /* DS,   Data Serial Input */
#define FRITZ4020_GPIO_SHIFT_SRCLK			20 /* SHCP, Shift Reg Clock Input */

#define FRITZ4020_SSR_BIT_0				0
#define FRITZ4020_SSR_BIT_1				1
#define FRITZ4020_SSR_BIT_2				2
#define FRITZ4020_SSR_BIT_3				3
#define FRITZ4020_SSR_BIT_4				4
#define FRITZ4020_SSR_BIT_5				5
#define FRITZ4020_SSR_BIT_6				6
#define FRITZ4020_SSR_BIT_7				7

#define FRITZ4020_74HC_GPIO_BASE			32
#define FRITZ4020_74HC_GPIO_LED_LAN			(FRITZ4020_74HC_GPIO_BASE + 0)
#define FRITZ4020_74HC_GPIO_LED_INFO_RED		(FRITZ4020_74HC_GPIO_BASE + 1)
#define FRITZ4020_74HC_GPIO_LED_POWER			(FRITZ4020_74HC_GPIO_BASE + 2)
#define FRITZ4020_74HC_GPIO_LED_WLAN			(FRITZ4020_74HC_GPIO_BASE + 3)
#define FRITZ4020_74HC_GPIO_LED_WAN			(FRITZ4020_74HC_GPIO_BASE + 4)
#define FRITZ4020_74HC_GPIO_USB_RST			(FRITZ4020_74HC_GPIO_BASE + 5)
#define FRITZ4020_74HC_GPIO_LED_INFO			(FRITZ4020_74HC_GPIO_BASE + 6)


#define FRITZ4020_GPIO_BTN_WPS				2
#define FRITZ4020_GPIO_BTN_WLAN				21
#define FRITZ4020_KEYS_POLL_INTERVAL			20 /* msecs */
#define FRITZ4020_KEYS_DEBOUNCE_INTERVAL		(3 * FRITZ4020_KEYS_POLL_INTERVAL)

#define FRTIZ4020_OFFSET_URLADER_WIFI_MAC_REVERSE	0x1979


static struct spi_gpio_platform_data fritz4020_spi_data = {
	.sck		= FRITZ4020_GPIO_SHIFT_SRCLK,
	.miso		= SPI_GPIO_NO_MISO,
	.mosi		= FRITZ4020_GPIO_SHIFT_SER,
	.num_chipselect	= 1,
};

static u8 fritz4020_ssr_initdata[] = {
	BIT(FRITZ4020_SSR_BIT_7) |
	BIT(FRITZ4020_SSR_BIT_6) |
	BIT(FRITZ4020_SSR_BIT_5) |
	BIT(FRITZ4020_SSR_BIT_4) |
	BIT(FRITZ4020_SSR_BIT_3) |
	BIT(FRITZ4020_SSR_BIT_2) |
	BIT(FRITZ4020_SSR_BIT_1)
};

static struct gen_74x164_chip_platform_data fritz4020_ssr_data = {
	.base = FRITZ4020_74HC_GPIO_BASE,
	.num_registers = ARRAY_SIZE(fritz4020_ssr_initdata),
	.init_data = fritz4020_ssr_initdata,
};

static struct platform_device fritz4020_spi_device = {
	.name		= "spi_gpio",
	.id		= 1,
	.dev = {
		.platform_data = &fritz4020_spi_data,
	},
};

static struct spi_board_info fritz4020_spi_info[] = {
	{
		.bus_num		= 1,
		.chip_select		= 0,
		.max_speed_hz		= 10000000,
		.modalias		= "74x164",
		.platform_data		= &fritz4020_ssr_data,
		.controller_data	= (void *) 0x0,
	},
};

static struct mtd_partition fritz4020_flash_partitions[] = {
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

static struct flash_platform_data fritz4020_flash_data = {
	.parts		= fritz4020_flash_partitions,
	.nr_parts	= ARRAY_SIZE(fritz4020_flash_partitions),
};

static struct gpio_led fritz4020_leds_gpio[] __initdata = {
	{
		.name		= "fritz4020:green:lan",
		.gpio		= FRITZ4020_74HC_GPIO_LED_LAN,
		.active_low	= 1,
	}, {
		.name		= "fritz4020:green:info",
		.gpio		= FRITZ4020_74HC_GPIO_LED_INFO,
		.active_low	= 1,
	}, {
		.name		= "fritz4020:red:info",
		.gpio		= FRITZ4020_74HC_GPIO_LED_INFO_RED,
		.active_low	= 1,
	}, {
		.name		= "fritz4020:green:power",
		.gpio		= FRITZ4020_74HC_GPIO_LED_POWER,
		.active_low	= 1,
	}, {
		.name		= "fritz4020:green:wlan",
		.gpio		= FRITZ4020_74HC_GPIO_LED_WLAN,
		.active_low	= 1,
	}, {
		.name		= "fritz4020:green:wan",
		.gpio		= FRITZ4020_74HC_GPIO_LED_WAN,
		.active_low	= 1,
	},
};

static struct gpio_keys_button fritz4020_gpio_keys[] __initdata = {
	{
		.desc			= "RFKILL button",
		.type			= EV_KEY,
		.code			= KEY_RFKILL,
		.debounce_interval	= FRITZ4020_KEYS_DEBOUNCE_INTERVAL,
		.gpio			= FRITZ4020_GPIO_BTN_WLAN,
		.active_low		= 0,
	},
	{
		.desc			= "WPS button",
		.type			= EV_KEY,
		.code			= KEY_WPS_BUTTON,
		.debounce_interval	= FRITZ4020_KEYS_DEBOUNCE_INTERVAL,
		.gpio			= FRITZ4020_GPIO_BTN_WPS,
		.active_low		= 0,
	},
};

static void __init fritz4020_setup(void) {
	u8 *urlader = (u8 *) KSEG1ADDR(0x1f000000);
	u8 wifi_mac[ETH_ALEN];

	ath79_register_m25p80(&fritz4020_flash_data);

	/* Initialize ethernet */
	ath79_extract_mac_reverse(urlader + FRTIZ4020_OFFSET_URLADER_WIFI_MAC_REVERSE, wifi_mac);
	ath79_setup_qca956x_eth_cfg(QCA956X_ETH_CFG_SW_PHY_SWAP |
				    QCA956X_ETH_CFG_SW_PHY_ADDR_SWAP);

	ath79_register_mdio(0, 0x0);
	ath79_register_mdio(1, 0x0);
	ath79_init_mac(ath79_eth0_data.mac_addr, wifi_mac, -1);
	ath79_init_mac(ath79_eth1_data.mac_addr, wifi_mac, -2);

	/* WAN port */
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ath79_eth0_data.speed = SPEED_100;
	ath79_eth0_data.duplex = DUPLEX_FULL;
	ath79_eth0_data.phy_mask = BIT(0);
	ath79_register_eth(0);

	/* LAN ports */
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_GMII;
	ath79_eth1_data.speed = SPEED_1000;
	ath79_eth1_data.duplex = DUPLEX_FULL;
	ath79_switch_data.phy_poll_mask |= BIT(0);
	ath79_switch_data.phy4_mii_en = 1;
	ath79_register_eth(1);

	/* Initialize 2.4GHz WiFi */
	ath79_register_wmac_simple();

	/* Activate USB Power */
	gpio_request_one(FRITZ4020_74HC_GPIO_USB_RST,
			 GPIOF_OUT_INIT_HIGH | GPIOF_EXPORT_DIR_FIXED,
			 "USB power");

	/* Initialize USB port */
	ath79_register_usb();

	/* Register LED shift-register */
	spi_register_board_info(fritz4020_spi_info,
				ARRAY_SIZE(fritz4020_spi_info));
	platform_device_register(&fritz4020_spi_device);

	/* Register GPIO buttons */
	ath79_register_gpio_keys_polled(-1, FRITZ4020_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(fritz4020_gpio_keys),
					fritz4020_gpio_keys);

	/* Register LEDs */
	ath79_register_leds_gpio(-1, ARRAY_SIZE(fritz4020_leds_gpio),
				 fritz4020_leds_gpio);
}

MIPS_MACHINE(ATH79_MACH_FRITZ4020, "FRITZ4020",
	     "AVM FRITZ!Box 4020", fritz4020_setup);

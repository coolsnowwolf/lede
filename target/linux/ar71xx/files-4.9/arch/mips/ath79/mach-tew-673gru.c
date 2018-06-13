/*
 *  TRENDnet TEW-673GRU board support
 *
 *  Copyright (C) 2012 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/rtl8366.h>
#include <linux/spi/spi.h>
#include <linux/spi/spi_gpio.h>

#include <asm/mach-ath79/ath79.h>

#include "dev-ap9x-pci.h"
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-usb.h"
#include "machtypes.h"

#define TEW673GRU_GPIO_LCD_SCK		0
#define TEW673GRU_GPIO_LCD_MOSI		1
#define TEW673GRU_GPIO_LCD_MISO		2
#define TEW673GRU_GPIO_LCD_CS		6

#define TEW673GRU_GPIO_LED_WPS		9

#define TEW673GRU_GPIO_BTN_RESET	3
#define TEW673GRU_GPIO_BTN_WPS		8

#define TEW673GRU_GPIO_RTL8366_SDA	5
#define TEW673GRU_GPIO_RTL8366_SCK	7

#define TEW673GRU_KEYS_POLL_INTERVAL	20 /* msecs */
#define TEW673GRU_KEYS_DEBOUNCE_INTERVAL (3 * TEW673GRU_KEYS_POLL_INTERVAL)

#define TEW673GRU_CAL0_OFFSET		0x1000
#define TEW673GRU_CAL1_OFFSET		0x5000
#define TEW673GRU_MAC0_OFFSET		0xffa0
#define TEW673GRU_MAC1_OFFSET		0xffb4

#define TEW673GRU_CAL_LOCATION_0	0x1f660000
#define TEW673GRU_CAL_LOCATION_1	0x1f7f0000

static struct gpio_led tew673gru_leds_gpio[] __initdata = {
	{
		.name		= "trendnet:blue:wps",
		.gpio		= TEW673GRU_GPIO_LED_WPS,
		.active_low	= 1,
	}
};

static struct gpio_keys_button tew673gru_gpio_keys[] __initdata = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = TEW673GRU_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= TEW673GRU_GPIO_BTN_RESET,
		.active_low	= 1,
	}, {
		.desc		= "wps",
		.type		= EV_KEY,
		.code		= KEY_WPS_BUTTON,
		.debounce_interval = TEW673GRU_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= TEW673GRU_GPIO_BTN_WPS,
		.active_low	= 1,
	}
};

static struct rtl8366_initval tew673gru_rtl8366s_initvals[] = {
	{ .reg = 0x06, .val = 0x0108 },
};

static struct rtl8366_platform_data tew673gru_rtl8366s_data = {
	.gpio_sda	= TEW673GRU_GPIO_RTL8366_SDA,
	.gpio_sck	= TEW673GRU_GPIO_RTL8366_SCK,
	.num_initvals	= ARRAY_SIZE(tew673gru_rtl8366s_initvals),
	.initvals	= tew673gru_rtl8366s_initvals,
};

static struct platform_device tew673gru_rtl8366s_device = {
	.name		= RTL8366S_DRIVER_NAME,
	.id		= -1,
	.dev = {
		.platform_data	= &tew673gru_rtl8366s_data,
	}
};

static struct spi_board_info tew673gru_spi_info[] = {
	{
		.bus_num	= 1,
		.chip_select	= 0,
		.max_speed_hz	= 400000,
		.modalias	= "spidev",
		.mode		= SPI_MODE_2,
		.controller_data = (void *) TEW673GRU_GPIO_LCD_CS,
	},
};

static struct spi_gpio_platform_data tew673gru_spi_data = {
	.sck		= TEW673GRU_GPIO_LCD_SCK,
	.miso		= TEW673GRU_GPIO_LCD_MISO,
	.mosi		= TEW673GRU_GPIO_LCD_MOSI,
	.num_chipselect = 1,
};

static struct platform_device tew673gru_spi_device = {
	.name		= "spi_gpio",
	.id		= 1,
	.dev = {
		.platform_data = &tew673gru_spi_data,
	},
};

static bool __init tew673gru_is_caldata_valid(u8 *p)
{
	u16 *magic0, *magic1;

	magic0 = (u16 *)(p + TEW673GRU_CAL0_OFFSET);
	magic1 = (u16 *)(p + TEW673GRU_CAL1_OFFSET);

	return (*magic0 == 0xa55a && *magic1 == 0xa55a);
}

static void __init tew673gru_wlan_init(void)
{
	u8 mac1[ETH_ALEN], mac2[ETH_ALEN];
	u8 *caldata;

	caldata = (u8 *) KSEG1ADDR(TEW673GRU_CAL_LOCATION_0);
	if (!tew673gru_is_caldata_valid(caldata)) {
		caldata = (u8 *)KSEG1ADDR(TEW673GRU_CAL_LOCATION_1);
		if (!tew673gru_is_caldata_valid(caldata)) {
			pr_err("no calibration data found\n");
			return;
		}
	}

	ath79_parse_ascii_mac(caldata + TEW673GRU_MAC0_OFFSET, mac1);
	ath79_parse_ascii_mac(caldata + TEW673GRU_MAC1_OFFSET, mac2);

	ath79_init_mac(ath79_eth0_data.mac_addr, mac1, 2);
	ath79_init_mac(ath79_eth1_data.mac_addr, mac1, 3);

	ap9x_pci_setup_wmac_led_pin(0, 5);
	ap9x_pci_setup_wmac_led_pin(1, 5);

	ap94_pci_init(caldata + TEW673GRU_CAL0_OFFSET, mac1,
		      caldata + TEW673GRU_CAL1_OFFSET, mac2);
}

static void __init tew673gru_setup(void)
{
	tew673gru_wlan_init();

	ath79_register_mdio(0, 0x0);

	ath79_eth0_data.mii_bus_dev = &tew673gru_rtl8366s_device.dev;
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth0_data.speed = SPEED_1000;
	ath79_eth0_data.duplex = DUPLEX_FULL;
	ath79_eth0_pll_data.pll_1000 = 0x11110000;

	ath79_eth1_data.mii_bus_dev = &tew673gru_rtl8366s_device.dev;
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth1_data.phy_mask = 0x10;
	ath79_eth1_pll_data.pll_1000 = 0x11110000;

	ath79_register_eth(0);
	ath79_register_eth(1);

	ath79_register_m25p80(NULL);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(tew673gru_leds_gpio),
				 tew673gru_leds_gpio);

	ath79_register_gpio_keys_polled(-1, TEW673GRU_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(tew673gru_gpio_keys),
					tew673gru_gpio_keys);

	ath79_register_usb();

	platform_device_register(&tew673gru_rtl8366s_device);

	spi_register_board_info(tew673gru_spi_info,
				ARRAY_SIZE(tew673gru_spi_info));
	platform_device_register(&tew673gru_spi_device);
}

MIPS_MACHINE(ATH79_MACH_TEW_673GRU, "TEW-673GRU", "TRENDnet TEW-673GRU",
	     tew673gru_setup);

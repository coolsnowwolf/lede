/*
 *  TP-Link Archer C58/C59 v1 board support
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
#include <linux/init.h>
#include <linux/spi/spi_gpio.h>
#include <linux/spi/74x164.h>

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

#define ARCHER_C59_V1_KEYS_POLL_INTERVAL	20
#define ARCHER_C59_V1_KEYS_DEBOUNCE_INTERVAL	(3 * ARCHER_C59_V1_KEYS_POLL_INTERVAL)

#define ARCHER_C59_V1_GPIO_BTN_RESET		21
#define ARCHER_C59_V1_GPIO_BTN_RFKILL		2
#define ARCHER_C59_V1_GPIO_BTN_WPS		1

#define ARCHER_C59_V1_GPIO_USB_POWER		22

#define ARCHER_C59_GPIO_SHIFT_OE		16
#define ARCHER_C59_GPIO_SHIFT_SER		17
#define ARCHER_C59_GPIO_SHIFT_SRCLK		18
#define ARCHER_C59_GPIO_SHIFT_SRCLR		19
#define ARCHER_C59_GPIO_SHIFT_RCLK		20

#define ARCHER_C59_74HC_GPIO_BASE		32
#define ARCHER_C59_74HC_GPIO_LED_POWER		(ARCHER_C59_74HC_GPIO_BASE + 0)
#define ARCHER_C59_74HC_GPIO_LED_WLAN2		(ARCHER_C59_74HC_GPIO_BASE + 1)
#define ARCHER_C59_74HC_GPIO_LED_WLAN5		(ARCHER_C59_74HC_GPIO_BASE + 2)
#define ARCHER_C59_74HC_GPIO_LED_LAN		(ARCHER_C59_74HC_GPIO_BASE + 3)
#define ARCHER_C59_74HC_GPIO_LED_WAN_GREEN	(ARCHER_C59_74HC_GPIO_BASE + 4)
#define ARCHER_C59_74HC_GPIO_LED_WAN_AMBER	(ARCHER_C59_74HC_GPIO_BASE + 5)
#define ARCHER_C59_74HC_GPIO_LED_WPS		(ARCHER_C59_74HC_GPIO_BASE + 6)
#define ARCHER_C59_74HC_GPIO_LED_USB		(ARCHER_C59_74HC_GPIO_BASE + 7)

#define ARCHER_C59_V1_SSR_BIT_0			0
#define ARCHER_C59_V1_SSR_BIT_1			1
#define ARCHER_C59_V1_SSR_BIT_2			2
#define ARCHER_C59_V1_SSR_BIT_3			3
#define ARCHER_C59_V1_SSR_BIT_4			4
#define ARCHER_C59_V1_SSR_BIT_5			5
#define ARCHER_C59_V1_SSR_BIT_6			6
#define ARCHER_C59_V1_SSR_BIT_7			7

#define ARCHER_C59_V1_WMAC_CALDATA_OFFSET	0x1000
#define ARCHER_C59_V1_PCI_CALDATA_OFFSET	0x5000

static struct gpio_led archer_c58_v1_leds_gpio[] __initdata = {
	{
		.name		= "archer-c58-v1:green:power",
		.gpio		= ARCHER_C59_74HC_GPIO_LED_POWER,
		.active_low	= 1,
	},
	{
		.name		= "archer-c58-v1:green:wlan2g",
		.gpio		= ARCHER_C59_74HC_GPIO_LED_WLAN2,
		.active_low	= 1,
	},
	{
		.name		= "archer-c58-v1:green:wlan5g",
		.gpio		= ARCHER_C59_74HC_GPIO_LED_WLAN5,
		.active_low	= 1,
	},
	{
		.name		= "archer-c58-v1:green:lan",
		.gpio		= ARCHER_C59_74HC_GPIO_LED_LAN,
		.active_low	= 1,
	},
	{
		.name		= "archer-c58-v1:green:wan",
		.gpio		= ARCHER_C59_74HC_GPIO_LED_WAN_GREEN,
		.active_low	= 1,
	},
	{
		.name		= "archer-c58-v1:amber:wan",
		.gpio		= ARCHER_C59_74HC_GPIO_LED_WAN_AMBER,
		.active_low	= 1,
	},
	{
		.name		= "archer-c58-v1:green:wps",
		.gpio		= ARCHER_C59_74HC_GPIO_LED_WPS,
		.active_low	= 1,
	},
};

static struct gpio_led archer_c59_v1_leds_gpio[] __initdata = {
	{
		.name		= "archer-c59-v1:green:power",
		.gpio		= ARCHER_C59_74HC_GPIO_LED_POWER,
		.active_low	= 1,
	},
	{
		.name		= "archer-c59-v1:green:wlan2g",
		.gpio		= ARCHER_C59_74HC_GPIO_LED_WLAN2,
		.active_low	= 1,
	},
	{
		.name		= "archer-c59-v1:green:wlan5g",
		.gpio		= ARCHER_C59_74HC_GPIO_LED_WLAN5,
		.active_low	= 1,
	},
	{
		.name		= "archer-c59-v1:green:lan",
		.gpio		= ARCHER_C59_74HC_GPIO_LED_LAN,
		.active_low	= 1,
	},
	{
		.name		= "archer-c59-v1:green:wan",
		.gpio		= ARCHER_C59_74HC_GPIO_LED_WAN_GREEN,
		.active_low	= 1,
	},
	{
		.name		= "archer-c59-v1:amber:wan",
		.gpio		= ARCHER_C59_74HC_GPIO_LED_WAN_AMBER,
		.active_low	= 1,
	},
	{
		.name		= "archer-c59-v1:green:wps",
		.gpio		= ARCHER_C59_74HC_GPIO_LED_WPS,
		.active_low	= 1,
	},
	{
		.name		= "archer-c59-v1:green:usb",
		.gpio		= ARCHER_C59_74HC_GPIO_LED_USB,
		.active_low	= 1,
	},
};

static struct gpio_keys_button archer_c59_v1_gpio_keys[] __initdata = {
	{
		.desc			= "Reset button",
		.type			= EV_KEY,
		.code			= KEY_RESTART,
		.debounce_interval	= ARCHER_C59_V1_KEYS_DEBOUNCE_INTERVAL,
		.gpio			= ARCHER_C59_V1_GPIO_BTN_RESET,
		.active_low		= 1,
	},
	{
		.desc			= "RFKILL button",
		.type			= EV_KEY,
		.code			= KEY_RFKILL,
		.debounce_interval	= ARCHER_C59_V1_KEYS_DEBOUNCE_INTERVAL,
		.gpio			= ARCHER_C59_V1_GPIO_BTN_RFKILL,
		.active_low		= 1,
	},
	{
		.desc			= "WPS button",
		.type			= EV_KEY,
		.code			= KEY_WPS_BUTTON,
		.debounce_interval	= ARCHER_C59_V1_KEYS_DEBOUNCE_INTERVAL,
		.gpio			= ARCHER_C59_V1_GPIO_BTN_WPS,
		.active_low		= 1,
	},
};

static struct spi_gpio_platform_data archer_c59_v1_spi_data = {
	.sck		= ARCHER_C59_GPIO_SHIFT_SRCLK,
	.miso		= SPI_GPIO_NO_MISO,
	.mosi		= ARCHER_C59_GPIO_SHIFT_SER,
	.num_chipselect = 1,
};

static u8 archer_c59_v1_ssr_initdata[] = {
	BIT(ARCHER_C59_V1_SSR_BIT_7) |
	BIT(ARCHER_C59_V1_SSR_BIT_6) |
	BIT(ARCHER_C59_V1_SSR_BIT_5) |
	BIT(ARCHER_C59_V1_SSR_BIT_4) |
	BIT(ARCHER_C59_V1_SSR_BIT_3) |
	BIT(ARCHER_C59_V1_SSR_BIT_2) |
	BIT(ARCHER_C59_V1_SSR_BIT_1)
};

static struct gen_74x164_chip_platform_data archer_c59_v1_ssr_data = {
	.base = ARCHER_C59_74HC_GPIO_BASE,
	.num_registers = ARRAY_SIZE(archer_c59_v1_ssr_initdata),
	.init_data = archer_c59_v1_ssr_initdata,
};

static struct platform_device archer_c59_v1_spi_device = {
	.name		= "spi_gpio",
	.id		= 1,
	.dev = {
		.platform_data = &archer_c59_v1_spi_data,
	},
};

static struct spi_board_info archer_c59_v1_spi_info[] = {
	{
		.bus_num	= 1,
		.chip_select	= 0,
		.max_speed_hz	= 10000000,
		.modalias	= "74x164",
		.platform_data	=  &archer_c59_v1_ssr_data,
		.controller_data = (void *) ARCHER_C59_GPIO_SHIFT_RCLK,
	},
};

static void __init archer_c5x_v1_setup(void)
{
	u8 *mac = (u8 *) KSEG1ADDR(0x1f010008);
	u8 *art = (u8 *) KSEG1ADDR(0x1fff0000);

	ath79_register_m25p80(NULL);
	spi_register_board_info(archer_c59_v1_spi_info,
			   ARRAY_SIZE(archer_c59_v1_spi_info));
	platform_device_register(&archer_c59_v1_spi_device);

	ath79_register_gpio_keys_polled(-1, ARCHER_C59_V1_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(archer_c59_v1_gpio_keys),
					archer_c59_v1_gpio_keys);

	ath79_setup_qca956x_eth_cfg(QCA956X_ETH_CFG_SW_PHY_SWAP |
				   QCA956X_ETH_CFG_SW_PHY_ADDR_SWAP);

	ath79_register_mdio(0, 0x0);
	ath79_register_mdio(1, 0x0);

	ath79_init_mac(ath79_eth0_data.mac_addr, mac, 1);
	ath79_init_mac(ath79_eth1_data.mac_addr, mac, 0);

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

	ath79_register_wmac(art + ARCHER_C59_V1_WMAC_CALDATA_OFFSET, mac);
	ap91_pci_init(art + ARCHER_C59_V1_PCI_CALDATA_OFFSET, NULL);

	ath79_register_usb();
	gpio_request_one(ARCHER_C59_V1_GPIO_USB_POWER,
			 GPIOF_OUT_INIT_HIGH | GPIOF_EXPORT_DIR_FIXED,
			 "USB power");
	gpio_request_one(ARCHER_C59_GPIO_SHIFT_OE,
			 GPIOF_OUT_INIT_LOW | GPIOF_EXPORT_DIR_FIXED,
			 "LED control");
	gpio_request_one(ARCHER_C59_GPIO_SHIFT_SRCLR,
			 GPIOF_OUT_INIT_HIGH | GPIOF_EXPORT_DIR_FIXED,
			 "LED reset");
}

static void __init archer_c58_v1_setup(void)
{
	archer_c5x_v1_setup();
	ath79_register_leds_gpio(-1, ARRAY_SIZE(archer_c58_v1_leds_gpio),
				archer_c58_v1_leds_gpio);
}

MIPS_MACHINE(ATH79_MACH_ARCHER_C58_V1, "ARCHER-C58-V1",
	"TP-LINK Archer C58 v1", archer_c58_v1_setup);

static void __init archer_c59_v1_setup(void)
{
	archer_c5x_v1_setup();
	ath79_register_leds_gpio(-1, ARRAY_SIZE(archer_c59_v1_leds_gpio),
				archer_c59_v1_leds_gpio);
}

MIPS_MACHINE(ATH79_MACH_ARCHER_C59_V1, "ARCHER-C59-V1",
	"TP-LINK Archer C59 v1", archer_c59_v1_setup);

/*
 *  TP-Link TL-WR942N(RU) v1 board support
 *
 *  Copyright (C) 2017 Sergey Studzinski <serguzhg@gmail.com>
 *  Thanks to Henryk Heisig <hyniu@o2.pl>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/platform_device.h>
#include <linux/ath9k_platform.h>
#include <asm/mach-ath79/ar71xx_regs.h>
#include <linux/gpio.h>
#include <linux/init.h>
#include <linux/spi/spi_gpio.h>
#include <linux/spi/74x164.h>

#include "common.h"
#include "dev-m25p80.h"
#include "machtypes.h"
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-spi.h"
#include "dev-usb.h"
#include "dev-wmac.h"
#include "nvram.h"

#define TL_WR942N_V1_KEYS_POLL_INTERVAL		20
#define TL_WR942N_V1_KEYS_DEBOUNCE_INTERVAL	\
					(3 * TL_WR942N_V1_KEYS_POLL_INTERVAL)

#define TL_WR942N_V1_GPIO_BTN_RESET		1
#define TL_WR942N_V1_GPIO_BTN_RFKILL		2

#define TL_WR942N_V1_GPIO_UART_TX		4
#define TL_WR942N_V1_GPIO_UART_RX		5

#define TL_WR942N_V1_GPIO_LED_USB2		14
#define TL_WR942N_V1_GPIO_LED_USB1		15

#define TL_WR942N_V1_GPIO_SHIFT_OE		16
#define TL_WR942N_V1_GPIO_SHIFT_SER		17
#define TL_WR942N_V1_GPIO_SHIFT_SRCLK		18
#define TL_WR942N_V1_GPIO_SHIFT_SRCLR		19
#define TL_WR942N_V1_GPIO_SHIFT_RCLK		20
#define TL_WR942N_V1_GPIO_LED_WPS		21
#define TL_WR942N_V1_GPIO_LED_STATUS		22

#define TL_WR942N_V1_74HC_GPIO_BASE		32
#define TL_WR942N_V1_74HC_GPIO_LED_LAN4		(TL_WR942N_V1_74HC_GPIO_BASE + 0)
#define TL_WR942N_V1_74HC_GPIO_LED_LAN3		(TL_WR942N_V1_74HC_GPIO_BASE + 1)
#define TL_WR942N_V1_74HC_GPIO_LED_LAN2		(TL_WR942N_V1_74HC_GPIO_BASE + 2)
#define TL_WR942N_V1_74HC_GPIO_LED_LAN1		(TL_WR942N_V1_74HC_GPIO_BASE + 3)
#define TL_WR942N_V1_74HC_GPIO_LED_WAN_GREEN	(TL_WR942N_V1_74HC_GPIO_BASE + 4)
#define TL_WR942N_V1_74HC_GPIO_LED_WAN_AMBER	(TL_WR942N_V1_74HC_GPIO_BASE + 5)
#define TL_WR942N_V1_74HC_GPIO_LED_WLAN		(TL_WR942N_V1_74HC_GPIO_BASE + 6)
#define TL_WR942N_V1_74HC_GPIO_HUB_RESET	(TL_WR942N_V1_74HC_GPIO_BASE + 7) /* from u-boot sources */

#define TL_WR942N_V1_SSR_BIT_0			0
#define TL_WR942N_V1_SSR_BIT_1			1
#define TL_WR942N_V1_SSR_BIT_2			2
#define TL_WR942N_V1_SSR_BIT_3			3
#define TL_WR942N_V1_SSR_BIT_4			4
#define TL_WR942N_V1_SSR_BIT_5			5
#define TL_WR942N_V1_SSR_BIT_6			6
#define TL_WR942N_V1_SSR_BIT_7			7

#define TL_WR942N_V1_WMAC_CALDATA_OFFSET	0x1000
#define TL_WR942N_V1_DEFAULT_MAC_ADDR		0x1fe40008
#define TL_WR942N_V1_DEFAULT_MAC_SIZE		0x200

#define GPIO_IN_ENABLE0_UART_SIN_LSB		8
#define GPIO_IN_ENABLE0_UART_SIN_MASK		0x0000ff00

static struct gpio_led tl_wr942n_v1_leds_gpio[] __initdata = {
	{
		.name		= "tl-wr942n-v1:green:status",
		.gpio		= TL_WR942N_V1_GPIO_LED_STATUS,
		.active_low	= 1,
	}, {
		.name		= "tl-wr942n-v1:green:wlan",
		.gpio		= TL_WR942N_V1_74HC_GPIO_LED_WLAN,
		.active_low	= 1,
	}, {
		.name		= "tl-wr942n-v1:green:lan1",
		.gpio		= TL_WR942N_V1_74HC_GPIO_LED_LAN1,
		.active_low	= 1,
	}, {
		.name		= "tl-wr942n-v1:green:lan2",
		.gpio		= TL_WR942N_V1_74HC_GPIO_LED_LAN2,
		.active_low	= 1,
	}, {
		.name		= "tl-wr942n-v1:green:lan3",
		.gpio		= TL_WR942N_V1_74HC_GPIO_LED_LAN3,
		.active_low	= 1,
	}, {
		.name		= "tl-wr942n-v1:green:lan4",
		.gpio		= TL_WR942N_V1_74HC_GPIO_LED_LAN4,
		.active_low	= 1,
	}, {
		.name		= "tl-wr942n-v1:green:wan",
		.gpio		= TL_WR942N_V1_74HC_GPIO_LED_WAN_GREEN,
		.active_low	= 1,
	}, {
		.name		= "tl-wr942n-v1:amber:wan",
		.gpio		= TL_WR942N_V1_74HC_GPIO_LED_WAN_AMBER,
		.active_low	= 1,
	}, {
		.name		= "tl-wr942n-v1:green:wps",
		.gpio		= TL_WR942N_V1_GPIO_LED_WPS,
		.active_low	= 1,
	}, {
		.name		= "tl-wr942n-v1:green:usb1",
		.gpio		= TL_WR942N_V1_GPIO_LED_USB1,
		.active_low	= 1,
	}, {
		.name		= "tl-wr942n-v1:green:usb2",
		.gpio		= TL_WR942N_V1_GPIO_LED_USB2,
		.active_low	= 1,
	},
};

static struct gpio_keys_button tl_wr942n_v1_gpio_keys[] __initdata = {
	{
		.desc			= "Reset button",
		.type			= EV_KEY,
		.code			= KEY_RESTART,
		.debounce_interval	= TL_WR942N_V1_KEYS_DEBOUNCE_INTERVAL,
		.gpio			= TL_WR942N_V1_GPIO_BTN_RESET,
		.active_low		= 1,
	}, {
		.desc			= "RFKILL button",
		.type			= EV_KEY,
		.code			= KEY_RFKILL,
		.debounce_interval	= TL_WR942N_V1_KEYS_DEBOUNCE_INTERVAL,
		.gpio			= TL_WR942N_V1_GPIO_BTN_RFKILL,
		.active_low		= 1,
	},
};

static struct spi_gpio_platform_data tl_wr942n_v1_spi_data = {
	.sck		= TL_WR942N_V1_GPIO_SHIFT_SRCLK,
	.miso		= SPI_GPIO_NO_MISO,
	.mosi		= TL_WR942N_V1_GPIO_SHIFT_SER,
	.num_chipselect	= 1,
};

static u8 tl_wr942n_v1_ssr_initdata[] = {
	BIT(TL_WR942N_V1_SSR_BIT_7) |
	BIT(TL_WR942N_V1_SSR_BIT_6) |
	BIT(TL_WR942N_V1_SSR_BIT_5) |
	BIT(TL_WR942N_V1_SSR_BIT_4) |
	BIT(TL_WR942N_V1_SSR_BIT_3) |
	BIT(TL_WR942N_V1_SSR_BIT_2) |
	BIT(TL_WR942N_V1_SSR_BIT_1) |
	BIT(TL_WR942N_V1_SSR_BIT_0)
};

static struct gen_74x164_chip_platform_data tl_wr942n_v1_ssr_data = {
	.base		= TL_WR942N_V1_74HC_GPIO_BASE,
	.num_registers	= ARRAY_SIZE(tl_wr942n_v1_ssr_initdata),
	.init_data	= tl_wr942n_v1_ssr_initdata,
};

static struct platform_device tl_wr942n_v1_spi_device = {
	.name		= "spi_gpio",
	.id		= 1,
	.dev = {
		.platform_data = &tl_wr942n_v1_spi_data,
	},
};

static struct spi_board_info tl_wr942n_v1_spi_info[] = {
	{
		.bus_num		= 1,
		.chip_select		= 0,
		.max_speed_hz		= 10000000,
		.modalias		= "74x164",
		.platform_data		= &tl_wr942n_v1_ssr_data,
		.controller_data	= (void *) TL_WR942N_V1_GPIO_SHIFT_RCLK,
	},
};

static void tl_wr942n_v1_get_mac(const char *name, char *mac)
{
	u8 *nvram = (u8 *) KSEG1ADDR(TL_WR942N_V1_DEFAULT_MAC_ADDR);
	int err;

	err = ath79_nvram_parse_mac_addr(nvram, TL_WR942N_V1_DEFAULT_MAC_SIZE,
					 name, mac);

	if (err)
		pr_err("no MAC address found for %s\n", name);
}

static void __init tl_wr942n_v1_setup(void)
{
	u8 *art = (u8 *) KSEG1ADDR(0x1fff0000);
	u8 tmpmac[ETH_ALEN];
	void __iomem *base;
	u32 t;

	ath79_register_m25p80(NULL);

	spi_register_board_info(tl_wr942n_v1_spi_info,
				ARRAY_SIZE(tl_wr942n_v1_spi_info));
	platform_device_register(&tl_wr942n_v1_spi_device);

	/* Check inherited UART RX GPIO definition */
	base = ioremap(AR71XX_GPIO_BASE, AR71XX_GPIO_SIZE);

	t = __raw_readl(base + QCA956X_GPIO_REG_IN_ENABLE0);
	if (((t & GPIO_IN_ENABLE0_UART_SIN_MASK)
	     >> GPIO_IN_ENABLE0_UART_SIN_LSB) == TL_WR942N_V1_GPIO_LED_USB1) {
		pr_warn("Active UART detected on USBLED's GPIOs!\n");

		tl_wr942n_v1_leds_gpio[9].gpio = TL_WR942N_V1_GPIO_UART_TX;
		tl_wr942n_v1_leds_gpio[10].gpio = TL_WR942N_V1_GPIO_UART_RX;
	}

	ath79_register_leds_gpio(-1, ARRAY_SIZE(tl_wr942n_v1_leds_gpio),
				 tl_wr942n_v1_leds_gpio);

	ath79_register_gpio_keys_polled(-1, TL_WR942N_V1_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(tl_wr942n_v1_gpio_keys),
					tl_wr942n_v1_gpio_keys);

	tl_wr942n_v1_get_mac("MAC:", tmpmac);

	/* swap PHYs */
	ath79_setup_qca956x_eth_cfg(QCA956X_ETH_CFG_SW_PHY_SWAP |
				    QCA956X_ETH_CFG_SW_PHY_ADDR_SWAP);

	ath79_register_mdio(0, 0x0);
	ath79_register_mdio(1, 0x0);

	/* WAN port */
	ath79_init_mac(ath79_eth0_data.mac_addr, tmpmac, 1);
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ath79_eth0_data.speed = SPEED_100;
	ath79_eth0_data.duplex = DUPLEX_FULL;

	/* swaped PHYs */
	ath79_eth0_data.phy_mask = BIT(0);
	ath79_register_eth(0);

	/* LAN ports */
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_GMII;
	ath79_init_mac(ath79_eth1_data.mac_addr, tmpmac, 0);
	ath79_eth1_data.speed = SPEED_1000;
	ath79_eth1_data.duplex = DUPLEX_FULL;

	/* swaped PHYs */
	ath79_switch_data.phy_poll_mask |= BIT(0);
	ath79_switch_data.phy4_mii_en = 1;
	ath79_register_eth(1);

	ath79_register_wmac(art + TL_WR942N_V1_WMAC_CALDATA_OFFSET, tmpmac);

	ath79_register_usb();

	gpio_request_one(TL_WR942N_V1_74HC_GPIO_HUB_RESET,
			 GPIOF_OUT_INIT_HIGH | GPIOF_EXPORT_DIR_FIXED,
			 "USB power");

	gpio_request_one(TL_WR942N_V1_GPIO_SHIFT_OE,
			 GPIOF_OUT_INIT_LOW | GPIOF_EXPORT_DIR_FIXED,
			 "LED control");

	gpio_request_one(TL_WR942N_V1_GPIO_SHIFT_SRCLR,
			 GPIOF_OUT_INIT_HIGH | GPIOF_EXPORT_DIR_FIXED,
			 "LED reset");
}

MIPS_MACHINE(ATH79_MACH_TL_WR942N_V1, "TL-WR942N-V1", "TP-LINK TL-WR942N v1",
	     tl_wr942n_v1_setup);

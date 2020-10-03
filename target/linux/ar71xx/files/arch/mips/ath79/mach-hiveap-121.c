/*
 *  Aerohive HiveAP 121 board support
 *
 *  Copyright (C) 2017 Chris Blake <chrisrblake93@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */
#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/i2c-gpio.h>
#include <linux/platform_device.h>
#include <linux/ath9k_platform.h>
#include <linux/platform/ar934x_nfc.h>
#include <asm/mach-ath79/ath79.h>
#include <asm/mach-ath79/ar71xx_regs.h>

#include "common.h"
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-nfc.h"
#include "dev-usb.h"
#include "dev-wmac.h"
#include "dev-ap9x-pci.h"
#include "dev-m25p80.h"
#include "machtypes.h"

#define HIVEAP_121_GPIO_LED_ORANGE	14
#define HIVEAP_121_GPIO_LED_WHITE	21
#define HIVEAP_121_GPIO_I2C_SCL		12
#define HIVEAP_121_GPIO_I2C_SDA		13
#define HIVEAP_121_GPIO_XLNA0		20
#define HIVEAP_121_GPIO_XLNA1		19
#define HIVEAP_121_GPIO_USB_POWER	15

#define HIVEAP_121_GPIO_BTN_RESET		4
#define HIVEAP_121_KEYS_POLL_INTERVAL		20 /* msecs */
#define HIVEAP_121_KEYS_DEBOUNCE_INTERVAL	\
					(3 * HIVEAP_121_KEYS_POLL_INTERVAL)

#define HIVEAP_121_MAC_OFFSET	0x90000

#define HIVEAP_121_LAN_PHYADDR	0

static struct gpio_led hiveap_121_leds_gpio[] __initdata = {
	{
		.name		= "hiveap-121:orange:power",
		.gpio		= HIVEAP_121_GPIO_LED_ORANGE,
		.active_low	= 1,
	}, {
		.name		= "hiveap-121:white:power",
		.gpio		= HIVEAP_121_GPIO_LED_WHITE,
		.active_low	= 1,
	}
};

static struct gpio_keys_button hiveap_121_gpio_keys[] __initdata = {
	{
		.desc = "reset",
		.type = EV_KEY,
		.code = KEY_RESTART,
		.debounce_interval = HIVEAP_121_KEYS_DEBOUNCE_INTERVAL,
		.gpio = HIVEAP_121_GPIO_BTN_RESET,
		.active_low  = 1,
	},
};

static struct i2c_gpio_platform_data hiveap_121_i2c_gpio_data = {
	.sda_pin = HIVEAP_121_GPIO_I2C_SDA,
	.scl_pin = HIVEAP_121_GPIO_I2C_SCL,
};

static struct platform_device hiveap_121_i2c_gpio_device = {
	.name	= "i2c-gpio",
	.id	= 0,
	.dev	= {
		.platform_data = &hiveap_121_i2c_gpio_data,
	}
};

static struct i2c_board_info tpm_i2c_info[] __initdata = {
	{
		I2C_BOARD_INFO("tpm_i2c_atmel", 0x29),
	}
};

static void __init hiveap_121_setup(void)
{
	u8 *base = (u8 *) KSEG1ADDR(0x1f000000);
	u8 wlan0_mac[ETH_ALEN];
	u8 wlan1_mac[ETH_ALEN];

	/* NAND */
	ath79_nfc_set_ecc_mode(AR934X_NFC_ECC_HW);
	ath79_register_nfc();

	/* SPI */
	ath79_register_m25p80(NULL);

	/* MDIO Interface */
	ath79_register_mdio(0, 0x0);
	ath79_setup_ar934x_eth_cfg(AR934X_ETH_CFG_RGMII_GMAC0 |
				   AR934X_ETH_CFG_RXD_DELAY |
				   AR934X_ETH_CFG_RDV_DELAY);

	/* GMAC0 is connected to the RGMII interface to an Atheros AR8035-A */
	ath79_init_mac(ath79_eth0_data.mac_addr,
		       base + HIVEAP_121_MAC_OFFSET, 0);
	ath79_eth0_data.mii_bus_dev = &ath79_mdio0_device.dev;
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth0_data.phy_mask = BIT(HIVEAP_121_LAN_PHYADDR);
	ath79_eth0_pll_data.pll_1000 = 0x06000000;
	ath79_eth0_pll_data.pll_100 = 0x00000101;
	ath79_eth0_pll_data.pll_10 = 0x00001313;
	ath79_register_eth(0);

	/* i2c */
	ath79_gpio_function_enable(AR934X_GPIO_FUNC_JTAG_DISABLE);
	platform_device_register(&hiveap_121_i2c_gpio_device);

	/* TPM */
	i2c_register_board_info(0, tpm_i2c_info, ARRAY_SIZE(tpm_i2c_info));

	/* LEDs and Buttons */
	ath79_register_leds_gpio(-1, ARRAY_SIZE(hiveap_121_leds_gpio),
				 hiveap_121_leds_gpio);
	ath79_register_gpio_keys_polled(-1, HIVEAP_121_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(hiveap_121_gpio_keys),
					hiveap_121_gpio_keys);

	/* USB */
	gpio_request_one(HIVEAP_121_GPIO_USB_POWER,
			 GPIOF_OUT_INIT_HIGH | GPIOF_EXPORT_DIR_FIXED,
			 "USB power");
	ath79_register_usb();

	/* XLNA - SoC Wireless */
	ath79_wmac_set_ext_lna_gpio(0, HIVEAP_121_GPIO_XLNA0);
	ath79_wmac_set_ext_lna_gpio(1, HIVEAP_121_GPIO_XLNA1);

	/* SoC Wireless */
	ath79_init_mac(wlan0_mac, base + HIVEAP_121_MAC_OFFSET, 1);
	ath79_register_wmac(NULL, wlan0_mac); /* Caldata in OTP */

	/* PCIe Wireless */
	ath79_init_mac(wlan1_mac, base + HIVEAP_121_MAC_OFFSET, 2);
	ap91_pci_init(NULL, wlan1_mac); /* Caldata in OTP */
}

MIPS_MACHINE(ATH79_MACH_HIVEAP_121, "HiveAP-121", "Aerohive HiveAP-121",
	     hiveap_121_setup);

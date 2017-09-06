/*
 *  Cisco Meraki Z1 board support
 *
 *  Copyright (C) 2016 Chris Blake <chrisrblake93@gmail.com>
 *  Copyright (C) 2016 Christian Lamparter <chunkeey@googlemail.com>
 *
 *  Based on Cisco Meraki GPL Release r23-20150601 Z1 Device Config
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */
#include <linux/platform_device.h>
#include <linux/ath9k_platform.h>
#include <linux/ar8216_platform.h>
#include <linux/platform/ar934x_nfc.h>

#include <asm/mach-ath79/ath79.h>
#include <asm/mach-ath79/ar71xx_regs.h>

#include <linux/leds-nu801.h>
#include <linux/firmware.h>
#include <linux/pci.h>

#include "common.h"
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-nfc.h"
#include "dev-usb.h"
#include "dev-wmac.h"
#include "dev-ap9x-pci.h"
#include "machtypes.h"

#define Z1_GPIO_LED_POWER_ORANGE	17

#define Z1_GPIO_NU801_CKI		14
#define Z1_GPIO_NU801_SDI		15

#define Z1_GPIO_XLNA0			18
#define Z1_GPIO_XLNA1			19

#define Z1_GPIO_BTN_RESET		12
#define Z1_KEYS_POLL_INTERVAL		20  /* msecs */
#define Z1_KEYS_DEBOUNCE_INTERVAL	(3 * Z1_KEYS_POLL_INTERVAL)

#define Z1_ETH_SWITCH_PHY		0

static struct gpio_led Z1_leds_gpio[] __initdata = {
	{
		.name = "z1:orange:power",
		.gpio = Z1_GPIO_LED_POWER_ORANGE,
		.active_low  = 1,
	},
};

static struct gpio_keys_button Z1_gpio_keys[] __initdata = {
	{
		.desc = "reset",
		.type = EV_KEY,
		.code = KEY_RESTART,
		.debounce_interval = Z1_KEYS_DEBOUNCE_INTERVAL,
		.gpio    = Z1_GPIO_BTN_RESET,
		.active_low  = 1,
	},
};

static struct led_nu801_template tricolor_led_template = {
	.device_name = "z1",
	.name = "tricolor",
	.num_leds = 1,
	.cki = Z1_GPIO_NU801_CKI,
	.sdi = Z1_GPIO_NU801_SDI,
	.lei = -1,
	.ndelay = 500,
	.init_brightness = {
		LED_OFF,
		LED_OFF,
		LED_OFF,
	},
	.default_trigger = "none",
};

static struct led_nu801_platform_data tricolor_led_data = {
	.num_controllers = 1,
	.template = &tricolor_led_template,
};

static struct platform_device tricolor_leds = {
	.name = "leds-nu801",
	.id = -1,
	.dev.platform_data = &tricolor_led_data,
};

static struct ar8327_pad_cfg z1_ar8327_pad0_cfg = {
	.mode = AR8327_PAD_MAC_RGMII,
	.txclk_delay_en = true,
	.rxclk_delay_en = true,
	.txclk_delay_sel = AR8327_CLK_DELAY_SEL1,
	.rxclk_delay_sel = AR8327_CLK_DELAY_SEL2,
};

static struct ar8327_platform_data z1_ar8327_data = {
	.pad0_cfg = &z1_ar8327_pad0_cfg,
	.port0_cfg = {
		.force_link = 1,
		.speed = AR8327_PORT_SPEED_1000,
		.duplex = 1,
		.txpause = 1,
		.rxpause = 1,
	},
};

static struct mdio_board_info z1_mdio0_info[] = {
	{
		.bus_id = "ag71xx-mdio.0",
		.phy_addr = Z1_ETH_SWITCH_PHY,
		.platform_data = &z1_ar8327_data,
	},
};

static void __init z1_setup(void)
{
	/* NAND */
	ath79_nfc_set_ecc_mode(AR934X_NFC_ECC_SOFT_BCH);
	ath79_register_nfc();

	/* Eth Config */
	ath79_setup_ar934x_eth_cfg(AR934X_ETH_CFG_RGMII_GMAC0 |
				   AR934X_ETH_CFG_SW_ONLY_MODE);

	/* MDIO Interface */
	ath79_register_mdio(1, 0x0);
	ath79_register_mdio(0, 0x0);
	mdiobus_register_board_info(z1_mdio0_info,
				    ARRAY_SIZE(z1_mdio0_info));

	/* GMAC0 is connected to an AR8327 switch */
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth0_data.phy_mask = BIT(Z1_ETH_SWITCH_PHY);
	ath79_eth0_data.mii_bus_dev = &ath79_mdio0_device.dev;
	ath79_eth0_pll_data.pll_1000 = 0x06000000;
	ath79_register_eth(0);

	/* XLNA */
	ath79_wmac_set_ext_lna_gpio(0, Z1_GPIO_XLNA0);
	ath79_wmac_set_ext_lna_gpio(1, Z1_GPIO_XLNA1);

	/* LEDs and Buttons */
	platform_device_register(&tricolor_leds);
	ath79_register_leds_gpio(-1, ARRAY_SIZE(Z1_leds_gpio),
				 Z1_leds_gpio);
	ath79_register_gpio_keys_polled(-1, Z1_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(Z1_gpio_keys),
					Z1_gpio_keys);

	/* USB */
	ath79_register_usb();

	/* Wireless */
	ath79_register_wmac_simple();
	ap91_pci_init_simple();
}
MIPS_MACHINE(ATH79_MACH_Z1, "Z1", "Meraki Z1", z1_setup);

/*
 *  NETGEAR WNDR3700v4/WNDR4300 board support
 *
 *  Copyright (C) 2012 Gabor Juhos <juhosg@openwrt.org>
 *  Copyright (C) 2014 Ralph Perlich <rpsoft@arcor.de>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/version.h>
#include <linux/pci.h>
#include <linux/phy.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>
#include <linux/ath9k_platform.h>
#include <linux/ar8216_platform.h>
#include <linux/mtd/mtd.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0)
#include <linux/mtd/nand.h>
#else
#include <linux/mtd/rawnand.h>
#endif
#include <linux/platform/ar934x_nfc.h>

#include <asm/mach-ath79/ar71xx_regs.h>

#include "common.h"
#include "dev-ap9x-pci.h"
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-nfc.h"
#include "dev-usb.h"
#include "dev-wmac.h"
#include "machtypes.h"

/* AR9344 GPIOs */
#define WNDR4300_GPIO_LED_POWER_GREEN	0
#define WNDR4300_GPIO_LED_POWER_AMBER	2
#define WNDR4300_GPIO_LED_USB		13
#define WNDR4300_GPIO_LED_WAN_GREEN	1
#define WNDR4300_GPIO_LED_WAN_AMBER	3
#define WNDR4300_GPIO_LED_WLAN2G	11
#define WNDR4300_GPIO_LED_WLAN5G	14
#define WNDR4300_GPIO_LED_WPS_GREEN	16
#define WNDR4300_GPIO_LED_WPS_AMBER	17

#define WNDR4300_GPIO_BTN_RESET		21
#define WNDR4300_GPIO_BTN_WIRELESS	15
#define WNDR4300_GPIO_BTN_WPS		12

/* AR9580 GPIOs */
#define WNDR4300_GPIO_USB_5V		0

#define WNDR4300_KEYS_POLL_INTERVAL	20	/* msecs */
#define WNDR4300_KEYS_DEBOUNCE_INTERVAL	(3 * WNDR4300_KEYS_POLL_INTERVAL)

static struct gpio_led wndr4300_leds_gpio[] __initdata = {
	{
		.name		= "netgear:green:power",
		.gpio		= WNDR4300_GPIO_LED_POWER_GREEN,
		.active_low	= 1,
	},
	{
		.name		= "netgear:amber:power",
		.gpio		= WNDR4300_GPIO_LED_POWER_AMBER,
		.active_low	= 1,
	},
	{
		.name		= "netgear:green:wan",
		.gpio		= WNDR4300_GPIO_LED_WAN_GREEN,
		.active_low	= 1,
	},
	{
		.name		= "netgear:amber:wan",
		.gpio		= WNDR4300_GPIO_LED_WAN_AMBER,
		.active_low	= 1,
	},
	{
		.name		= "netgear:green:usb",
		.gpio		= WNDR4300_GPIO_LED_USB,
		.active_low	= 1,
	},
	{
		.name		= "netgear:green:wps",
		.gpio		= WNDR4300_GPIO_LED_WPS_GREEN,
		.active_low	= 1,
	},
	{
		.name		= "netgear:amber:wps",
		.gpio		= WNDR4300_GPIO_LED_WPS_AMBER,
		.active_low	= 1,
	},
	{
		.name		= "netgear:green:wlan2g",
		.gpio		= WNDR4300_GPIO_LED_WLAN2G,
		.active_low	= 1,
	},
	{
		.name		= "netgear:blue:wlan5g",
		.gpio		= WNDR4300_GPIO_LED_WLAN5G,
		.active_low	= 1,
	},
};

static struct gpio_keys_button wndr4300_gpio_keys[] __initdata = {
	{
		.desc		= "Reset button",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = WNDR4300_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= WNDR4300_GPIO_BTN_RESET,
		.active_low	= 1,
	},
	{
		.desc		= "WPS button",
		.type		= EV_KEY,
		.code		= KEY_WPS_BUTTON,
		.debounce_interval = WNDR4300_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= WNDR4300_GPIO_BTN_WPS,
		.active_low	= 1,
	},
	{
		.desc		= "Wireless button",
		.type		= EV_KEY,
		.code		= KEY_RFKILL,
		.debounce_interval = WNDR4300_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= WNDR4300_GPIO_BTN_WIRELESS,
		.active_low	= 1,
	},
};

static struct ar8327_pad_cfg wndr4300_ar8327_pad0_cfg = {
	.mode = AR8327_PAD_MAC_RGMII,
	.txclk_delay_en = true,
	.rxclk_delay_en = true,
	.txclk_delay_sel = AR8327_CLK_DELAY_SEL1,
	.rxclk_delay_sel = AR8327_CLK_DELAY_SEL2,
};

static struct ar8327_led_cfg wndr4300_ar8327_led_cfg = {
	.led_ctrl0 = 0xcc35cc35,
	.led_ctrl1 = 0xcb37cb37,
	.led_ctrl2 = 0x00000000,
	.led_ctrl3 = 0x00f3cf00,
	.open_drain = true,
};

static struct ar8327_platform_data wndr4300_ar8327_data = {
	.pad0_cfg = &wndr4300_ar8327_pad0_cfg,
	.port0_cfg = {
		.force_link = 1,
		.speed = AR8327_PORT_SPEED_1000,
		.duplex = 1,
		.txpause = 1,
		.rxpause = 1,
	},
	.led_cfg = &wndr4300_ar8327_led_cfg,
};

static struct mdio_board_info wndr4300_mdio0_info[] = {
	{
		.bus_id = "ag71xx-mdio.0",
		.mdio_addr = 0,
		.platform_data = &wndr4300_ar8327_data,
	},
};

static void __init wndr4300_setup(void)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(wndr4300_leds_gpio); i++)
		ath79_gpio_output_select(wndr4300_leds_gpio[i].gpio,
					 AR934X_GPIO_OUT_GPIO);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(wndr4300_leds_gpio),
				 wndr4300_leds_gpio);
	ath79_register_gpio_keys_polled(-1, WNDR4300_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(wndr4300_gpio_keys),
					wndr4300_gpio_keys);

	ath79_setup_ar934x_eth_cfg(AR934X_ETH_CFG_RGMII_GMAC0);

	mdiobus_register_board_info(wndr4300_mdio0_info,
				    ARRAY_SIZE(wndr4300_mdio0_info));

	ath79_register_mdio(0, 0x0);

	/* GMAC0 is connected to an AR8327N switch */
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth0_data.phy_mask = BIT(0);
	ath79_eth0_data.mii_bus_dev = &ath79_mdio0_device.dev;
	ath79_eth0_pll_data.pll_1000 = 0x06000000;
	ath79_register_eth(0);

	ath79_nfc_set_ecc_mode(AR934X_NFC_ECC_HW);
	ath79_register_nfc();
	ath79_register_usb();

	ath79_register_wmac_simple();

	/* enable power for the USB port */
	ap9x_pci_setup_wmac_gpio(0, BIT(WNDR4300_GPIO_USB_5V),
				 BIT(WNDR4300_GPIO_USB_5V));

	ap91_pci_init_simple();
}

MIPS_MACHINE(ATH79_MACH_WNDR3700_V4, "WNDR3700_V4", "NETGEAR WNDR3700v4",
	     wndr4300_setup);
MIPS_MACHINE(ATH79_MACH_WNDR4300, "WNDR4300", "NETGEAR WNDR4300",
	     wndr4300_setup);

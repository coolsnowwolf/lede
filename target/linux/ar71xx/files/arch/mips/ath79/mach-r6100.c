/*
 *  NETGEAR R6100 board support
 *
 *  Copyright (C) 2014 Gabor Juhos <juhosg@openwrt.org>
 *  Copyright (C) 2014 Imre Kaloz <kaloz@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/pci.h>
#include <linux/phy.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>
#include <linux/ath9k_platform.h>
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

#define R6100_GPIO_LED_WLAN		0
#define R6100_GPIO_LED_USB		11
#define R6100_GPIO_LED_WAN_GREEN	13
#define R6100_GPIO_LED_POWER_AMBER	14
#define R6100_GPIO_LED_WAN_AMBER	15
#define R6100_GPIO_LED_POWER_GREEN	17

#define R6100_GPIO_BTN_WIRELESS		1
#define R6100_GPIO_BTN_WPS		3
#define R6100_GPIO_BTN_RESET		12

#define R6100_GPIO_USB_POWER		16

#define R6100_KEYS_POLL_INTERVAL	20	/* msecs */
#define R6100_KEYS_DEBOUNCE_INTERVAL	(3 * R6100_KEYS_POLL_INTERVAL)

static struct gpio_led r6100_leds_gpio[] __initdata = {
	{
		.name		= "netgear:green:power",
		.gpio		= R6100_GPIO_LED_POWER_GREEN,
		.active_low	= 1,
	},
	{
		.name		= "netgear:amber:power",
		.gpio		= R6100_GPIO_LED_POWER_AMBER,
		.active_low	= 1,
	},
	{
		.name		= "netgear:green:wan",
		.gpio		= R6100_GPIO_LED_WAN_GREEN,
		.active_low	= 1,
	},
	{
		.name		= "netgear:amber:wan",
		.gpio		= R6100_GPIO_LED_WAN_AMBER,
		.active_low	= 1,
	},
	{
		.name		= "netgear:blue:usb",
		.gpio		= R6100_GPIO_LED_USB,
		.active_low	= 1,
	},
	{
		.name		= "netgear:blue:wlan",
		.gpio		= R6100_GPIO_LED_WLAN,
		.active_low	= 1,
	},
};

static struct gpio_keys_button r6100_gpio_keys[] __initdata = {
	{
		.desc		= "Reset button",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = R6100_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= R6100_GPIO_BTN_RESET,
		.active_low	= 0,
	},
	{
		.desc		= "WPS button",
		.type		= EV_KEY,
		.code		= KEY_WPS_BUTTON,
		.debounce_interval = R6100_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= R6100_GPIO_BTN_WPS,
		.active_low	= 0,
	},
	{
		.desc		= "RFKILL switch",
		.type		= EV_SW,
		.code		= KEY_RFKILL,
		.debounce_interval = R6100_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= R6100_GPIO_BTN_WIRELESS,
		.active_low	= 0,
	},
};

static void __init r6100_setup(void)
{
	ath79_register_leds_gpio(-1, ARRAY_SIZE(r6100_leds_gpio),
				 r6100_leds_gpio);
	ath79_register_gpio_keys_polled(-1, R6100_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(r6100_gpio_keys),
					r6100_gpio_keys);

	ath79_setup_ar934x_eth_cfg(AR934X_ETH_CFG_SW_PHY_SWAP);

	ath79_register_mdio(1, 0x0);

	/* GMAC0 is connected to the PHY0 of the internal switch */
	ath79_switch_data.phy4_mii_en = 1;
	ath79_switch_data.phy_poll_mask = BIT(0);
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ath79_eth0_data.phy_mask = BIT(0);
	ath79_eth0_data.mii_bus_dev = &ath79_mdio1_device.dev;
	ath79_register_eth(0);

	/* GMAC1 is connected to the internal switch */
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_GMII;
	ath79_register_eth(1);

	gpio_request_one(R6100_GPIO_USB_POWER,
			 GPIOF_OUT_INIT_HIGH | GPIOF_EXPORT_DIR_FIXED,
			 "USB power");

	ath79_nfc_set_ecc_mode(AR934X_NFC_ECC_HW);
	ath79_register_nfc();

	ath79_register_usb();

	ath79_register_wmac_simple();

	ap91_pci_init_simple();
}

MIPS_MACHINE(ATH79_MACH_R6100, "R6100", "NETGEAR R6100",
	     r6100_setup);

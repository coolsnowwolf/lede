/*
 *  ALFA NETWORK Hornet-UB board support
 *
 *  Copyright (C) 2011-2012 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/gpio.h>

#include <asm/mach-ath79/ath79.h>
#include <asm/mach-ath79/ar71xx_regs.h>

#include "common.h"
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-usb.h"
#include "dev-wmac.h"
#include "machtypes.h"

#define HORNET_UB_GPIO_LED_WLAN		0
#define HORNET_UB_GPIO_LED_USB		1
#define HORNET_UB_GPIO_LED_LAN		13
#define HORNET_UB_GPIO_LED_WAN		17
#define HORNET_UB_GPIO_LED_WPS		27
#define HORNET_UB_GPIO_EXT_LNA		28

#define HORNET_UB_GPIO_BTN_RESET	12
#define HORNET_UB_GPIO_BTN_WPS		11

#define HORNET_UB_GPIO_USB_POWER	26

#define HORNET_UB_KEYS_POLL_INTERVAL	20	/* msecs */
#define HORNET_UB_KEYS_DEBOUNCE_INTERVAL	(3 * HORNET_UB_KEYS_POLL_INTERVAL)

#define HORNET_UB_MAC0_OFFSET		0x0000
#define HORNET_UB_MAC1_OFFSET		0x0006
#define HORNET_UB_CALDATA_OFFSET	0x1000

static struct gpio_led hornet_ub_leds_gpio[] __initdata = {
	{
		.name		= "alfa:blue:lan",
		.gpio		= HORNET_UB_GPIO_LED_LAN,
		.active_low	= 0,
	},
	{
		.name		= "alfa:blue:usb",
		.gpio		= HORNET_UB_GPIO_LED_USB,
		.active_low	= 0,
	},
	{
		.name		= "alfa:blue:wan",
		.gpio		= HORNET_UB_GPIO_LED_WAN,
		.active_low	= 1,
	},
	{
		.name		= "alfa:blue:wlan",
		.gpio		= HORNET_UB_GPIO_LED_WLAN,
		.active_low	= 0,
	},
	{
		.name		= "alfa:blue:wps",
		.gpio		= HORNET_UB_GPIO_LED_WPS,
		.active_low	= 1,
	},
};

static struct gpio_keys_button hornet_ub_gpio_keys[] __initdata = {
	{
		.desc		= "WPS button",
		.type		= EV_KEY,
		.code		= KEY_WPS_BUTTON,
		.debounce_interval = HORNET_UB_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= HORNET_UB_GPIO_BTN_WPS,
		.active_low	= 0,
	},
	{
		.desc		= "Reset button",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = HORNET_UB_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= HORNET_UB_GPIO_BTN_RESET,
		.active_low	= 1,
	}
};

static void __init hornet_ub_gpio_setup(void)
{
	u32 t;

	ath79_gpio_function_disable(AR933X_GPIO_FUNC_ETH_SWITCH_LED0_EN |
				     AR933X_GPIO_FUNC_ETH_SWITCH_LED1_EN |
				     AR933X_GPIO_FUNC_ETH_SWITCH_LED2_EN |
				     AR933X_GPIO_FUNC_ETH_SWITCH_LED3_EN |
				     AR933X_GPIO_FUNC_ETH_SWITCH_LED4_EN);

	t = ath79_reset_rr(AR933X_RESET_REG_BOOTSTRAP);
	t |= AR933X_BOOTSTRAP_MDIO_GPIO_EN;
	ath79_reset_wr(AR933X_RESET_REG_BOOTSTRAP, t);

	gpio_request_one(HORNET_UB_GPIO_USB_POWER,
			 GPIOF_OUT_INIT_HIGH | GPIOF_EXPORT_DIR_FIXED,
			 "USB power");
	gpio_request_one(HORNET_UB_GPIO_EXT_LNA,
			GPIOF_OUT_INIT_HIGH | GPIOF_EXPORT_DIR_FIXED,
			"external LNA0");

}

static void __init hornet_ub_setup(void)
{
	u8 *art = (u8 *) KSEG1ADDR(0x1fff0000);

	hornet_ub_gpio_setup();

	ath79_register_m25p80(NULL);
	ath79_register_leds_gpio(-1, ARRAY_SIZE(hornet_ub_leds_gpio),
					hornet_ub_leds_gpio);
	ath79_register_gpio_keys_polled(-1, HORNET_UB_KEYS_POLL_INTERVAL,
					 ARRAY_SIZE(hornet_ub_gpio_keys),
					 hornet_ub_gpio_keys);

	ath79_init_mac(ath79_eth1_data.mac_addr,
			art + HORNET_UB_MAC0_OFFSET, 0);
	ath79_init_mac(ath79_eth0_data.mac_addr,
			art + HORNET_UB_MAC1_OFFSET, 0);

	ath79_register_mdio(0, 0x0);

	ath79_register_eth(1);
	ath79_register_eth(0);

	ath79_register_wmac(art + HORNET_UB_CALDATA_OFFSET, NULL);
	ath79_register_usb();
}

MIPS_MACHINE(ATH79_MACH_HORNET_UB, "HORNET-UB", "ALFA NETWORK Hornet-UB",
	     hornet_ub_setup);

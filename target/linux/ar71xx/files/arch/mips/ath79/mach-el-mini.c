/*
 *  Easy-Link EL-MINI board support
 *
 *  Copyright (C) 2012 huangfc <huangfangcheng@163.com>
 *  Copyright (C) 2011 hys <550663898@qq.com>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/gpio.h>

#include <asm/mach-ath79/ath79.h>

#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-usb.h"
#include "dev-wmac.h"
#include "machtypes.h"

#define MINI_GPIO_LED_SYSTEM	27
#define MINI_GPIO_BTN_RESET	11

#define MINI_GPIO_USB_POWER	8

#define MINI_KEYS_POLL_INTERVAL	20	/* msecs */
#define MINI_KEYS_DEBOUNCE_INTERVAL	(3 * MINI_KEYS_POLL_INTERVAL)

static const char *mini_part_probes[] = {
	"tp-link",
	NULL,
};

static struct flash_platform_data mini_flash_data = {
	.part_probes	= mini_part_probes,
};

static struct gpio_led mini_leds_gpio[] __initdata = {
	{
		.name		= "easylink:green:system",
		.gpio		= MINI_GPIO_LED_SYSTEM,
		.active_low	= 1,
	},
};

static struct gpio_keys_button mini_gpio_keys[] __initdata = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = MINI_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= MINI_GPIO_BTN_RESET,
		.active_low	= 0,
	}
};

static void __init el_mini_setup(void)
{
	u8 *mac = (u8 *) KSEG1ADDR(0x1f01fc00);
	u8 *ee = (u8 *) KSEG1ADDR(0x1fff1000);

	ath79_register_m25p80(&mini_flash_data);
	ath79_register_leds_gpio(-1, ARRAY_SIZE(mini_leds_gpio),
				 mini_leds_gpio);
	ath79_register_gpio_keys_polled(-1, MINI_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(mini_gpio_keys),
					mini_gpio_keys);

	gpio_request_one(MINI_GPIO_USB_POWER,
			 GPIOF_OUT_INIT_HIGH | GPIOF_EXPORT_DIR_FIXED,
			 "USB power");
	ath79_register_usb();

	ath79_init_mac(ath79_eth0_data.mac_addr, mac, -1);

	ath79_register_mdio(0, 0x0);
	ath79_register_eth(0);

	ath79_register_wmac(ee, mac);
}

MIPS_MACHINE(ATH79_MACH_EL_MINI, "EL-MINI", "EasyLink EL-MINI",
	     el_mini_setup);

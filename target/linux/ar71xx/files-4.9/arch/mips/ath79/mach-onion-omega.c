/*
 *  Onion Omega board support
 *
 *  Copyright (C) 2015 Boken Lin <bl@onion.io>
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

#define OMEGA_GPIO_LED_SYSTEM	27
#define OMEGA_GPIO_BTN_RESET	11

#define OMEGA_GPIO_USB_POWER	8

#define OMEGA_KEYS_POLL_INTERVAL	20  /* msecs */
#define OMEGA_KEYS_DEBOUNCE_INTERVAL	(3 * OMEGA_KEYS_POLL_INTERVAL)

static const char *omega_part_probes[] = {
	"tp-link",
	NULL,
};

static struct flash_platform_data omega_flash_data = {
	.part_probes    = omega_part_probes,
};

static struct gpio_led omega_leds_gpio[] __initdata = {
	{
		.name       = "onion:amber:system",
		.gpio       = OMEGA_GPIO_LED_SYSTEM,
		.active_low = 1,
	},
};

static struct gpio_keys_button omega_gpio_keys[] __initdata = {
	{
		.desc       = "reset",
		.type       = EV_KEY,
		.code       = KEY_RESTART,
		.debounce_interval = OMEGA_KEYS_DEBOUNCE_INTERVAL,
		.gpio       = OMEGA_GPIO_BTN_RESET,
		.active_low = 0,
	}
};

static void __init onion_omega_setup(void)
{
	u8 *mac = (u8 *) KSEG1ADDR(0x1f01fc00);
	u8 *ee = (u8 *) KSEG1ADDR(0x1fff1000);

	ath79_register_m25p80(&omega_flash_data);
	ath79_register_leds_gpio(-1, ARRAY_SIZE(omega_leds_gpio),
				omega_leds_gpio);
	ath79_register_gpio_keys_polled(-1, OMEGA_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(omega_gpio_keys),
					omega_gpio_keys);

	gpio_request_one(OMEGA_GPIO_USB_POWER,
			GPIOF_OUT_INIT_HIGH | GPIOF_EXPORT_DIR_FIXED,
			"USB power");
	ath79_register_usb();

	ath79_init_mac(ath79_eth0_data.mac_addr, mac, -1);

	ath79_register_mdio(0, 0x0);
	ath79_register_eth(0);

	ath79_register_wmac(ee, mac);
}

MIPS_MACHINE(ATH79_MACH_ONION_OMEGA, "ONION-OMEGA", "Onion Omega", onion_omega_setup);

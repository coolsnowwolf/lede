/*
 *  Hak5 WiFi Pineapple NANO board support
 *
 *  Copyright (C) 2018 Sebastian Kinne <seb@hak5.org>
 *  Copyright (C) 2018 Piotr Dymacz <pepe2k@gmail.com>
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

#define WIFI_PINEAPPLE_NANO_GPIO_LED_SYSTEM	18

#define WIFI_PINEAPPLE_NANO_GPIO_BTN_RESET	12
#define WIFI_PINEAPPLE_NANO_GPIO_SD_DET		19
#define WIFI_PINEAPPLE_NANO_GPIO_USB_ALARM	20
#define WIFI_PINEAPPLE_NANO_GPIO_USB_POWER	23

#define HAK5_KEYS_POLL_INTERVAL			20 /* msecs */
#define HAK5_KEYS_DEBOUNCE_INTERVAL		(3 * HAK5_KEYS_POLL_INTERVAL)

#define WIFI_PINEAPPLE_NANO_MAC1_OFFSET		0x0006
#define WIFI_PINEAPPLE_NANO_CALDATA_OFFSET	0x1000

static const char *hak5_part_probes[] = {
	"tp-link",
	NULL,
};

static struct flash_platform_data hak5_flash_data = {
	.part_probes = hak5_part_probes,
};

static struct gpio_led wifi_pineapple_nano_leds_gpio[] __initdata = {
	{
		.name		= "wifi-pineapple-nano:blue:system",
		.gpio		= WIFI_PINEAPPLE_NANO_GPIO_LED_SYSTEM,
		.active_low	= 1,
	},
};

static struct gpio_keys_button wifi_pineapple_nano_gpio_keys[] __initdata = {
	{
		.desc			= "reset",
		.type			= EV_KEY,
		.code			= KEY_RESTART,
		.debounce_interval	= HAK5_KEYS_DEBOUNCE_INTERVAL,
		.gpio			= WIFI_PINEAPPLE_NANO_GPIO_BTN_RESET,
		.active_low		= 1,
	}
};

static void __init wifi_pineapple_nano_setup(void)
{
	u8 *art = (u8 *) KSEG1ADDR(0x1fff0000);

	ath79_register_m25p80(&hak5_flash_data);

	ath79_setup_ar933x_phy4_switch(false, false);
	ath79_register_mdio(0, 0x0);

	ath79_init_mac(ath79_eth0_data.mac_addr,
		       art + WIFI_PINEAPPLE_NANO_MAC1_OFFSET, 0);
	ath79_register_eth(0);

	/* GPIO11/12 */
	ath79_gpio_function_disable(AR933X_GPIO_FUNC_UART_RTS_CTS_EN);

	gpio_request_one(WIFI_PINEAPPLE_NANO_GPIO_SD_DET,
			 GPIOF_IN | GPIOF_EXPORT_DIR_FIXED | GPIOF_ACTIVE_LOW,
			 "SD card present");

	gpio_request_one(WIFI_PINEAPPLE_NANO_GPIO_USB_ALARM,
			 GPIOF_IN | GPIOF_EXPORT_DIR_FIXED | GPIOF_ACTIVE_LOW,
			 "USB alarm");

	gpio_request_one(WIFI_PINEAPPLE_NANO_GPIO_USB_POWER,
			 GPIOF_OUT_INIT_LOW | GPIOF_EXPORT_DIR_FIXED |
			 GPIOF_ACTIVE_LOW, "USB power");

	ath79_register_leds_gpio(-1, ARRAY_SIZE(wifi_pineapple_nano_leds_gpio),
				 wifi_pineapple_nano_leds_gpio);

	ath79_register_gpio_keys_polled(-1, HAK5_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(wifi_pineapple_nano_gpio_keys),
					wifi_pineapple_nano_gpio_keys);

	ath79_register_usb();
	ath79_register_wmac(art + WIFI_PINEAPPLE_NANO_CALDATA_OFFSET, NULL);
}

MIPS_MACHINE(ATH79_MACH_WIFI_PINEAPPLE_NANO, "WIFI-PINEAPPLE-NANO",
	     "Hak5 WiFi Pineapple NANO", wifi_pineapple_nano_setup);

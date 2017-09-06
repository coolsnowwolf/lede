/*
 *  Oolite board support
 *
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
#include "dev-wmac.h"
#include "machtypes.h"
#include "dev-usb.h"

#define GS_OOLITE_GPIO_BTN6		6
#define GS_OOLITE_GPIO_BTN7		7
#define GS_OOLITE_GPIO_BTN_RESET	11

#define GS_OOLITE_GPIO_LED_SYSTEM	27

#define GS_OOLITE_KEYS_POLL_INTERVAL	20	/* msecs */
#define GS_OOLITE_KEYS_DEBOUNCE_INTERVAL (3 * GS_OOLITE_KEYS_POLL_INTERVAL)

static const char *gs_oolite_part_probes[] = {
	"tp-link",
	NULL,
};

static struct flash_platform_data gs_oolite_flash_data = {
	.part_probes	= gs_oolite_part_probes,
};

static struct gpio_led gs_oolite_leds_gpio[] __initdata = {
	{
		.name		= "oolite:red:system",
		.gpio		= GS_OOLITE_GPIO_LED_SYSTEM,
		.active_low	= 1,
	},
};

static struct gpio_keys_button gs_oolite_gpio_keys[] __initdata = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = GS_OOLITE_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= GS_OOLITE_GPIO_BTN_RESET,
		.active_low	= 0,
	},
	{
		.desc		= "BTN_6",
		.type		= EV_KEY,
		.code		= BTN_6,
		.debounce_interval = GS_OOLITE_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= GS_OOLITE_GPIO_BTN6,
		.active_low	= 0,
	},
	{
		.desc		= "BTN_7",
		.type		= EV_KEY,
		.code		= BTN_7,
		.debounce_interval = GS_OOLITE_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= GS_OOLITE_GPIO_BTN7,
		.active_low	= 0,
	},
};

static void __init gs_oolite_setup(void)
{
	u8 *mac = (u8 *) KSEG1ADDR(0x1f01fc00);
	u8 *ee = (u8 *) KSEG1ADDR(0x1fff1000);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(gs_oolite_leds_gpio),
				 gs_oolite_leds_gpio);

	ath79_register_gpio_keys_polled(-1, GS_OOLITE_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(gs_oolite_gpio_keys),
					gs_oolite_gpio_keys);

	ath79_register_usb();

	ath79_register_m25p80(&gs_oolite_flash_data);
	ath79_init_mac(ath79_eth0_data.mac_addr, mac, 1);
	ath79_init_mac(ath79_eth1_data.mac_addr, mac, -1);

	ath79_register_mdio(0, 0x0);
	ath79_register_eth(1);
	ath79_register_eth(0);

	ath79_register_wmac(ee, mac);
}

MIPS_MACHINE(ATH79_MACH_GS_OOLITE, "GS-OOLITE",
	     "Oolite V1.0", gs_oolite_setup);

/*
 *  Telldus TellStick ZNet Lite board support
 *
 *  Copyright (C) 2016 Micke Prag <micke.prag@telldus.se>
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

#define TELLSTICK_GPIO_LED_SYSTEM	27
#define TELLSTICK_GPIO_LED_BLUE	0
#define TELLSTICK_GPIO_LED_RED	14
#define TELLSTICK_GPIO_LED_GREEN	15
#define TELLSTICK_GPIO_LED_LAN_GREEN	16
#define TELLSTICK_GPIO_LED_LAN_ORANGE	17

#define TELLSTICK_GPIO_BTN_RESET	11

#define TELLSTICK_GPIO_RF433_RESET	13

#define TELLSTICK_KEYS_POLL_INTERVAL	20	/* msecs */
#define TELLSTICK_KEYS_DEBOUNCE_INTERVAL	(3 * TELLSTICK_KEYS_POLL_INTERVAL)

static const char *tellstick_part_probes[] = {
	"tp-link",
	NULL,
};

static struct flash_platform_data tellstick_flash_data = {
	.part_probes	= tellstick_part_probes,
};

static struct gpio_led tellstick_leds_gpio[] __initdata = {
	{
		.name		= "tellstick:white:system",
		.gpio		= TELLSTICK_GPIO_LED_SYSTEM,
		.active_low	= 0,
	},
	{
		.name		= "tellstick:blue:status",
		.gpio		= TELLSTICK_GPIO_LED_BLUE,
		.active_low	= 0,
	},
	{
		.name		= "tellstick:red:status",
		.gpio		= TELLSTICK_GPIO_LED_RED,
		.active_low	= 0,
	},
	{
		.name		= "tellstick:green:status",
		.gpio		= TELLSTICK_GPIO_LED_GREEN,
		.active_low	= 0,
	},
	{
		.name		= "tellstick:green:lan",
		.gpio		= TELLSTICK_GPIO_LED_LAN_GREEN,
		.active_low	= 0,
	},
	{
		.name		= "tellstick:orange:lan",
		.gpio		= TELLSTICK_GPIO_LED_LAN_ORANGE,
		.active_low	= 0,
	},
};

static struct gpio_keys_button tellstick_gpio_keys[] __initdata = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = TELLSTICK_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= TELLSTICK_GPIO_BTN_RESET,
		.active_low	= 0,
	}
};

static void __init tellstick_znet_lite_setup(void)
{
	u8 *ee =  (u8 *) KSEG1ADDR(0x1fff1000);
	u8 mac[ETH_ALEN];
	memcpy(&mac, (u8 *) KSEG1ADDR(0x1f01fc00), sizeof(mac));

	ath79_gpio_function_disable(
		AR933X_GPIO_FUNC_ETH_SWITCH_LED0_EN |
		AR933X_GPIO_FUNC_ETH_SWITCH_LED1_EN |
		AR933X_GPIO_FUNC_ETH_SWITCH_LED2_EN |
		AR933X_GPIO_FUNC_ETH_SWITCH_LED3_EN |
		AR933X_GPIO_FUNC_ETH_SWITCH_LED4_EN
	);

	ath79_register_m25p80(&tellstick_flash_data);
	ath79_register_leds_gpio(-1, ARRAY_SIZE(tellstick_leds_gpio),
				 tellstick_leds_gpio);
	ath79_register_gpio_keys_polled(-1, TELLSTICK_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(tellstick_gpio_keys),
					tellstick_gpio_keys);

	gpio_request_one(TELLSTICK_GPIO_RF433_RESET,
			 GPIOF_OUT_INIT_LOW | GPIOF_EXPORT_DIR_FIXED,
			 "rf433 reset");
	ath79_register_usb();

	ath79_init_mac(ath79_eth0_data.mac_addr, (u8 *)mac, 0);

	ath79_register_mdio(0, 0x0);
	ath79_register_eth(0);

	// wlan0 mac needs to be different then eth0
	mac[3] += 1;
	ath79_register_wmac(ee, (u8 *)mac);
}

MIPS_MACHINE(ATH79_MACH_TELLSTICK_ZNET_LITE, "TELLSTICK-ZNET-LITE", "Telldus TellStick ZNet Lite",
	     tellstick_znet_lite_setup);

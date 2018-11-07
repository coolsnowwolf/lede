/*
 *  ALFA NETWORK Tube2H board support
 *
 *  Copyright (C) 2014 Gabor Juhos <juhosg@openwrt.org>
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

#define TUBE2H_GPIO_LED_SIGNAL4		0
#define TUBE2H_GPIO_LED_SIGNAL3		1
#define TUBE2H_GPIO_LED_SIGNAL2		13
#define TUBE2H_GPIO_LED_LAN		17
#define TUBE2H_GPIO_LED_SIGNAL1		27
#define TUBE2H_GPIO_EXT_LNA		28

#define TUBE2H_GPIO_WDT_EN		22
#define TUBE2H_GPIO_WDT_IN		18

#define TUBE2H_GPIO_BTN_RESET		12

#define TUBE2H_KEYS_POLL_INTERVAL	20	/* msecs */
#define TUBE2H_KEYS_DEBOUNCE_INTERVAL	(3 * TUBE2H_KEYS_POLL_INTERVAL)

#define TUBE2H_ART_ADDRESS		0x1fff0000
#define TUBE2H_LAN_MAC_OFFSET		0x06
#define TUBE2H_CALDATA_OFFSET		0x1000

static struct gpio_led tube2h_leds_gpio[] __initdata = {
	{
		.name		= "alfa:blue:lan",
		.gpio		= TUBE2H_GPIO_LED_LAN,
		.active_low	= 1,
	},
	{
		.name		= "alfa:red:signal1",
		.gpio		= TUBE2H_GPIO_LED_SIGNAL1,
		.active_low	= 1,
	},
	{
		.name		= "alfa:orange:signal2",
		.gpio		= TUBE2H_GPIO_LED_SIGNAL2,
		.active_low	= 0,
	},
	{
		.name		= "alfa:green:signal3",
		.gpio		= TUBE2H_GPIO_LED_SIGNAL3,
		.active_low	= 0,
	},
	{
		.name		= "alfa:green:signal4",
		.gpio		= TUBE2H_GPIO_LED_SIGNAL4,
		.active_low	= 0,
	},
};

static struct gpio_keys_button tube2h_gpio_keys[] __initdata = {
	{
		.desc		= "Reset button",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = TUBE2H_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= TUBE2H_GPIO_BTN_RESET,
		.active_low	= 1,
	},
};

static void __init tube2h_setup(void)
{
	u8 *art = (u8 *) KSEG1ADDR(TUBE2H_ART_ADDRESS);
	u32 t;

	ath79_gpio_function_disable(AR933X_GPIO_FUNC_JTAG_DISABLE |
				    AR933X_GPIO_FUNC_ETH_SWITCH_LED0_EN |
				    AR933X_GPIO_FUNC_ETH_SWITCH_LED1_EN |
				    AR933X_GPIO_FUNC_ETH_SWITCH_LED2_EN |
				    AR933X_GPIO_FUNC_ETH_SWITCH_LED3_EN |
				    AR933X_GPIO_FUNC_ETH_SWITCH_LED4_EN);

	/* Ensure that GPIO26 and GPIO27 are controllable by software */
	t = ath79_reset_rr(AR933X_RESET_REG_BOOTSTRAP);
	t |= AR933X_BOOTSTRAP_MDIO_GPIO_EN;
	ath79_reset_wr(AR933X_RESET_REG_BOOTSTRAP, t);

	gpio_request_one(TUBE2H_GPIO_EXT_LNA,
			GPIOF_OUT_INIT_HIGH | GPIOF_EXPORT_DIR_FIXED,
			"external LNA0");

	gpio_request_one(TUBE2H_GPIO_WDT_IN,
			 GPIOF_OUT_INIT_LOW | GPIOF_EXPORT_DIR_FIXED,
			 "WDT input");

	gpio_request_one(TUBE2H_GPIO_WDT_EN,
			 GPIOF_OUT_INIT_LOW | GPIOF_EXPORT_DIR_FIXED,
			 "WDT enable");

	ath79_register_wmac(art + TUBE2H_CALDATA_OFFSET, NULL);

	ath79_register_m25p80(NULL);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(tube2h_leds_gpio),
				 tube2h_leds_gpio);
	ath79_register_gpio_keys_polled(-1, TUBE2H_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(tube2h_gpio_keys),
					tube2h_gpio_keys);

	ath79_init_mac(ath79_eth0_data.mac_addr,
		       art + TUBE2H_LAN_MAC_OFFSET, 0);
	ath79_register_mdio(0, 0x0);
	ath79_register_eth(0);
}

MIPS_MACHINE(ATH79_MACH_TUBE2H, "TUBE2H", "ALFA NETWORK Tube2H",
	     tube2h_setup);


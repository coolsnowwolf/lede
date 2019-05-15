/*
 *  Zcomax ZCN-1523H-2-8/5-16 board support
 *
 *  Copyright (C) 2010-2012 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <asm/mach-ath79/ath79.h>
#include <asm/mach-ath79/ar71xx_regs.h>

#include "common.h"
#include "dev-eth.h"
#include "dev-m25p80.h"
#include "dev-ap9x-pci.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "machtypes.h"

#define ZCN_1523H_GPIO_BTN_RESET	0
#define ZCN_1523H_GPIO_LED_INIT		11
#define ZCN_1523H_GPIO_LED_LAN1		17

#define ZCN_1523H_2_GPIO_LED_WEAK	13
#define ZCN_1523H_2_GPIO_LED_MEDIUM	14
#define ZCN_1523H_2_GPIO_LED_STRONG	15

#define ZCN_1523H_5_GPIO_LAN2_POWER	1
#define ZCN_1523H_5_GPIO_LED_LAN2	13
#define ZCN_1523H_5_GPIO_LED_WEAK	14
#define ZCN_1523H_5_GPIO_LED_MEDIUM	15
#define ZCN_1523H_5_GPIO_LED_STRONG	16

#define ZCN_1523H_KEYS_POLL_INTERVAL	20	/* msecs */
#define ZCN_1523H_KEYS_DEBOUNCE_INTERVAL (3 * ZCN_1523H_KEYS_POLL_INTERVAL)

static struct gpio_keys_button zcn_1523h_gpio_keys[] __initdata = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = ZCN_1523H_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= ZCN_1523H_GPIO_BTN_RESET,
		.active_low	= 1,
	}
};

static struct gpio_led zcn_1523h_leds_gpio[] __initdata = {
	{
		.name		= "zcn-1523h:amber:init",
		.gpio		= ZCN_1523H_GPIO_LED_INIT,
		.active_low	= 1,
	}, {
		.name		= "zcn-1523h:green:lan1",
		.gpio		= ZCN_1523H_GPIO_LED_LAN1,
		.active_low	= 1,
	}
};

static struct gpio_led zcn_1523h_2_leds_gpio[] __initdata = {
	{
		.name		= "zcn-1523h:red:weak",
		.gpio		= ZCN_1523H_2_GPIO_LED_WEAK,
		.active_low	= 1,
	}, {
		.name		= "zcn-1523h:amber:medium",
		.gpio		= ZCN_1523H_2_GPIO_LED_MEDIUM,
		.active_low	= 1,
	}, {
		.name		= "zcn-1523h:green:strong",
		.gpio		= ZCN_1523H_2_GPIO_LED_STRONG,
		.active_low	= 1,
	}
};

static struct gpio_led zcn_1523h_5_leds_gpio[] __initdata = {
	{
		.name		= "zcn-1523h:red:weak",
		.gpio		= ZCN_1523H_5_GPIO_LED_WEAK,
		.active_low	= 1,
	}, {
		.name		= "zcn-1523h:amber:medium",
		.gpio		= ZCN_1523H_5_GPIO_LED_MEDIUM,
		.active_low	= 1,
	}, {
		.name		= "zcn-1523h:green:strong",
		.gpio		= ZCN_1523H_5_GPIO_LED_STRONG,
		.active_low	= 1,
	}, {
		.name		= "zcn-1523h:green:lan2",
		.gpio		= ZCN_1523H_5_GPIO_LED_LAN2,
		.active_low	= 1,
	}
};

static void __init zcn_1523h_generic_setup(void)
{
	u8 *mac = (u8 *) KSEG1ADDR(0x1f7e0004);
	u8 *ee = (u8 *) KSEG1ADDR(0x1fff1000);

	ath79_gpio_function_disable(AR724X_GPIO_FUNC_ETH_SWITCH_LED0_EN |
				    AR724X_GPIO_FUNC_ETH_SWITCH_LED1_EN |
				    AR724X_GPIO_FUNC_ETH_SWITCH_LED2_EN |
				    AR724X_GPIO_FUNC_ETH_SWITCH_LED3_EN |
				    AR724X_GPIO_FUNC_ETH_SWITCH_LED4_EN);

	ath79_register_m25p80(NULL);

	ath79_register_leds_gpio(0, ARRAY_SIZE(zcn_1523h_leds_gpio),
					zcn_1523h_leds_gpio);

	ath79_register_gpio_keys_polled(-1, ZCN_1523H_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(zcn_1523h_gpio_keys),
					zcn_1523h_gpio_keys);

	ap91_pci_init(ee, mac);

	ath79_init_mac(ath79_eth0_data.mac_addr, mac, 0);
	ath79_init_mac(ath79_eth1_data.mac_addr, mac, 1);

	ath79_register_mdio(0, 0x0);

	/* LAN1 port */
	ath79_register_eth(0);
}

static void __init zcn_1523h_2_setup(void)
{
	zcn_1523h_generic_setup();
	ap9x_pci_setup_wmac_gpio(0, BIT(9), 0);

	ath79_register_leds_gpio(1, ARRAY_SIZE(zcn_1523h_2_leds_gpio),
				 zcn_1523h_2_leds_gpio);
}

MIPS_MACHINE(ATH79_MACH_ZCN_1523H_2, "ZCN-1523H-2", "Zcomax ZCN-1523H-2",
	     zcn_1523h_2_setup);

static void __init zcn_1523h_5_setup(void)
{
	zcn_1523h_generic_setup();
	ap9x_pci_setup_wmac_gpio(0, BIT(8), 0);

	ath79_register_leds_gpio(1, ARRAY_SIZE(zcn_1523h_5_leds_gpio),
				 zcn_1523h_5_leds_gpio);

	/* LAN2 port */
	ath79_register_eth(1);
}

MIPS_MACHINE(ATH79_MACH_ZCN_1523H_5, "ZCN-1523H-5", "Zcomax ZCN-1523H-5",
	     zcn_1523h_5_setup);

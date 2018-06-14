/*
 *  Buffalo WHR-HP-G300N board support
 *
 *  based on ...
 *
 *  TP-LINK TL-WR741ND board support
 *
 *  Copyright (C) 2009-2010 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <asm/mach-ath79/ath79.h>
#include <asm/mach-ath79/ar71xx_regs.h>

#include "common.h"
#include "dev-ap9x-pci.h"
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "machtypes.h"

#define WHRHPG300N_GPIO_LED_SECURITY		0
#define WHRHPG300N_GPIO_LED_DIAG		1
#define WHRHPG300N_GPIO_LED_ROUTER		6

#define WHRHPG300N_GPIO_BTN_ROUTER_ON		7
#define WHRHPG300N_GPIO_BTN_ROUTER_AUTO		8
#define WHRHPG300N_GPIO_BTN_RESET		11
#define WHRHPG300N_GPIO_BTN_AOSS		12
#define WHRHPG300N_GPIO_LED_LAN1		13
#define WHRHPG300N_GPIO_LED_LAN2		14
#define WHRHPG300N_GPIO_LED_LAN3		15
#define WHRHPG300N_GPIO_LED_LAN4		16
#define WHRHPG300N_GPIO_LED_WAN			17

#define	WHRHPG300N_KEYS_POLL_INTERVAL	20	/* msecs */
#define WHRHPG300N_KEYS_DEBOUNCE_INTERVAL (3 * WHRHPG300N_KEYS_POLL_INTERVAL)

#define WHRHPG300N_MAC_OFFSET		0x20c

static struct gpio_led whrhpg300n_leds_gpio[] __initdata = {
	{
		.name		= "buffalo:orange:security",
		.gpio		= WHRHPG300N_GPIO_LED_SECURITY,
		.active_low	= 1,
	}, {
		.name		= "buffalo:red:diag",
		.gpio		= WHRHPG300N_GPIO_LED_DIAG,
		.active_low	= 1,
	}, {
		.name		= "buffalo:green:router",
		.gpio		= WHRHPG300N_GPIO_LED_ROUTER,
		.active_low	= 1,
	}, {
		.name		= "buffalo:green:wan",
		.gpio		= WHRHPG300N_GPIO_LED_WAN,
		.active_low	= 1,
	}, {
		.name		= "buffalo:green:lan1",
		.gpio		= WHRHPG300N_GPIO_LED_LAN1,
		.active_low	= 1,
	}, {
		.name		= "buffalo:green:lan2",
		.gpio		= WHRHPG300N_GPIO_LED_LAN2,
		.active_low	= 1,
	}, {
		.name		= "buffalo:green:lan3",
		.gpio		= WHRHPG300N_GPIO_LED_LAN3,
		.active_low	= 1,
	}, {
		.name		= "buffalo:green:lan4",
		.gpio		= WHRHPG300N_GPIO_LED_LAN4,
		.active_low	= 1,
	}
};

static struct gpio_keys_button whrhpg300n_gpio_keys[] __initdata = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = WHRHPG300N_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= WHRHPG300N_GPIO_BTN_RESET,
		.active_low	= 1,
	}, {
		.desc		= "aoss/wps",
		.type		= EV_KEY,
		.code		= KEY_WPS_BUTTON,
		.gpio		= WHRHPG300N_GPIO_BTN_AOSS,
		.debounce_interval = WHRHPG300N_KEYS_DEBOUNCE_INTERVAL,
		.active_low	= 1,
	}, {
		.desc		= "router_on",
		.type		= EV_KEY,
		.code		= BTN_2,
		.gpio		= WHRHPG300N_GPIO_BTN_ROUTER_ON,
		.debounce_interval = WHRHPG300N_KEYS_DEBOUNCE_INTERVAL,
		.active_low	= 1,
	}, {
		.desc		= "router_auto",
		.type		= EV_KEY,
		.code		= BTN_3,
		.gpio		= WHRHPG300N_GPIO_BTN_ROUTER_AUTO,
		.debounce_interval = WHRHPG300N_KEYS_DEBOUNCE_INTERVAL,
		.active_low	= 1,
	}
};

static void __init whrhpg300n_setup(void)
{
	u8 *ee = (u8 *) KSEG1ADDR(0x1fff1000);
	u8 *mac = (u8 *) KSEG1ADDR(ee + WHRHPG300N_MAC_OFFSET);

	ath79_register_m25p80(NULL);

	ath79_gpio_function_disable(AR724X_GPIO_FUNC_ETH_SWITCH_LED0_EN |
				    AR724X_GPIO_FUNC_ETH_SWITCH_LED1_EN |
				    AR724X_GPIO_FUNC_ETH_SWITCH_LED2_EN |
				    AR724X_GPIO_FUNC_ETH_SWITCH_LED3_EN |
				    AR724X_GPIO_FUNC_ETH_SWITCH_LED4_EN);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(whrhpg300n_leds_gpio),
				 whrhpg300n_leds_gpio);

	ath79_register_gpio_keys_polled(-1, WHRHPG300N_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(whrhpg300n_gpio_keys),
					whrhpg300n_gpio_keys);

	ath79_init_mac(ath79_eth0_data.mac_addr, mac, 0);
	ath79_init_mac(ath79_eth1_data.mac_addr, mac, 1);

	ath79_register_mdio(0, 0x0);

	/* LAN ports */
	ath79_register_eth(1);
	/* WAN port */
	ath79_register_eth(0);

	ap9x_pci_setup_wmac_led_pin(0, 1);

	ap91_pci_init(ee, mac);
}

MIPS_MACHINE(ATH79_MACH_WHR_HP_G300N, "WHR-HP-G300N", "Buffalo WHR-HP-G300N",
	     whrhpg300n_setup);

MIPS_MACHINE(ATH79_MACH_WHR_G301N, "WHR-G301N", "Buffalo WHR-G301N",
	     whrhpg300n_setup);

MIPS_MACHINE(ATH79_MACH_WHR_HP_GN, "WHR-HP-GN", "Buffalo WHR-HP-GN",
	     whrhpg300n_setup);

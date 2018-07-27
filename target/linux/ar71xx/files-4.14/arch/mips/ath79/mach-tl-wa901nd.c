/*
 *  TP-LINK TL-WA901N/ND v1, TL-WA7510N v1 board support
 *
 *  Copyright (C) 2009-2012 Gabor Juhos <juhosg@openwrt.org>
 *  Copyright (C) 2010 Pieter Hollants <pieter@hollants.com>
 *  Copyright (C) 2012 Stefan Helmert <helst_listen@aol.de>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <asm/mach-ath79/ar71xx_regs.h>
#include <asm/mach-ath79/ath79.h>

#include "common.h"
#include "dev-ap9x-pci.h"
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "machtypes.h"
#include "pci.h"

#define TL_WA901ND_GPIO_LED_QSS		0
#define TL_WA901ND_GPIO_LED_SYSTEM	1
#define TL_WA901ND_GPIO_LED_LAN		13

#define TL_WA901ND_GPIO_BTN_RESET	11
#define TL_WA901ND_GPIO_BTN_QSS		12

#define TL_WA901ND_KEYS_POLL_INTERVAL	20	/* msecs */
#define TL_WA901ND_KEYS_DEBOUNCE_INTERVAL (3 * TL_WA901ND_KEYS_POLL_INTERVAL)

static const char *tl_wa901nd_part_probes[] = {
	"tp-link",
	NULL,
};

static struct flash_platform_data tl_wa901nd_flash_data = {
	.part_probes	= tl_wa901nd_part_probes,
};

static struct gpio_led tl_wa901nd_leds_gpio[] __initdata = {
	{
		.name		= "tp-link:green:lan",
		.gpio		= TL_WA901ND_GPIO_LED_LAN,
		.active_low	= 1,
	}, {
		.name		= "tp-link:green:system",
		.gpio		= TL_WA901ND_GPIO_LED_SYSTEM,
		.active_low	= 1,
	}, {
		.name		= "tp-link:green:qss",
		.gpio		= TL_WA901ND_GPIO_LED_QSS,
		.active_low	= 1,
	}
};

static struct gpio_keys_button tl_wa901nd_gpio_keys[] __initdata = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = TL_WA901ND_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= TL_WA901ND_GPIO_BTN_RESET,
		.active_low	= 1,
	}, {
		.desc		= "qss",
		.type		= EV_KEY,
		.code		= KEY_WPS_BUTTON,
		.debounce_interval = TL_WA901ND_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= TL_WA901ND_GPIO_BTN_QSS,
		.active_low	= 1,
	}
};

static void __init common_setup(void)
{
	u8 *mac = (u8 *) KSEG1ADDR(0x1f01fc00);

	/*
	 * ath79_eth0 would be the WAN port, but is not connected.
	 * ath79_eth1 connects to the internal switch chip, however
	 * we have a single LAN port only.
	 */
	ath79_init_mac(ath79_eth1_data.mac_addr, mac, 0);
	ath79_register_mdio(0, 0x0);
	ath79_register_eth(1);

	ath79_register_m25p80(&tl_wa901nd_flash_data);
}

static void __init tl_wa901nd_setup(void)
{
	u8 *mac = (u8 *) KSEG1ADDR(0x1f01fc00);
	u8 *ee  = (u8 *) KSEG1ADDR(0x1fff1000);

	ath79_gpio_function_disable(AR724X_GPIO_FUNC_ETH_SWITCH_LED0_EN |
				    AR724X_GPIO_FUNC_ETH_SWITCH_LED1_EN |
				    AR724X_GPIO_FUNC_ETH_SWITCH_LED2_EN |
				    AR724X_GPIO_FUNC_ETH_SWITCH_LED3_EN |
				    AR724X_GPIO_FUNC_ETH_SWITCH_LED4_EN);

	common_setup();

	ath79_register_leds_gpio(-1, ARRAY_SIZE(tl_wa901nd_leds_gpio),
				 tl_wa901nd_leds_gpio);

	ath79_register_gpio_keys_polled(-1, TL_WA901ND_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(tl_wa901nd_gpio_keys),
					tl_wa901nd_gpio_keys);

	ap91_pci_init(ee, mac);
}

MIPS_MACHINE(ATH79_MACH_TL_WA901ND, "TL-WA901ND", "TP-LINK TL-WA901ND",
	     tl_wa901nd_setup);

static void __init tl_wa7510n_v1_setup(void)
{
	common_setup();
	ath79_register_pci();
}

MIPS_MACHINE(ATH79_MACH_TL_WA7510N_V1, "TL-WA7510N", "TP-LINK TL-WA7510N v1",
	     tl_wa7510n_v1_setup);

/*
 * eTactica EG-200 board, based on 8devices Carambola2 module
 *
 * Copyright (C) 2015 Karl Palsson <karlp@etactica.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 */

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

#define RME_EG200_GPIO_LED_WLAN			0
#define RME_EG200_GPIO_LED_ETH0			13
#define RME_EG200_GPIO_LED_ETACTICA		15
#define RME_EG200_GPIO_LED_MODBUS		16

#define RME_EG200_GPIO_BTN_RESTORE		11

#define RME_EG200_KEYS_POLL_INTERVAL		20	/* msecs */
#define RME_EG200_KEYS_DEBOUNCE_INTERVAL	(3 * RME_EG200_KEYS_POLL_INTERVAL)

#define RME_EG200_MAC0_OFFSET			0x0000
#define RME_EG200_CALDATA_OFFSET		0x1000
#define RME_EG200_WMAC_MAC_OFFSET		0x1002

static struct gpio_led rme_eg200_leds_gpio[] __initdata = {
	{
		.name		= "eg200:red:wlan",
		.gpio		= RME_EG200_GPIO_LED_WLAN,
		.active_low	= 1,
	}, {
		.name		= "eg200:red:eth0",
		.gpio		= RME_EG200_GPIO_LED_ETH0,
		.active_low	= 1,
	}, {
		.name		= "eg200:red:etactica",
		.gpio		= RME_EG200_GPIO_LED_ETACTICA,
		.active_low	= 0,
	}, {
		.name		= "eg200:red:modbus",
		.gpio		= RME_EG200_GPIO_LED_MODBUS,
		.active_low	= 0,
	}
};

static struct gpio_keys_button rme_eg200_keys[] __initdata = {
	{
		.desc		= "restore button",
		.type		= EV_KEY,
		.code		= KEY_WPS_BUTTON,
		.debounce_interval = RME_EG200_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= RME_EG200_GPIO_BTN_RESTORE,
		.active_low	= 1,
	},
};

static void __init rme_eg200_setup(void)
{
	u8 *art = (u8 *) KSEG1ADDR(0x1fff0000);

	ath79_register_m25p80(NULL);
	ath79_register_wmac(art + RME_EG200_CALDATA_OFFSET,
			    art + RME_EG200_WMAC_MAC_OFFSET);

	ath79_setup_ar933x_phy4_switch(true, true);

	ath79_init_mac(ath79_eth0_data.mac_addr, art + RME_EG200_MAC0_OFFSET, 0);

	ath79_register_mdio(0, 0x0);

	/* WAN port */
	ath79_register_eth(0);

	ath79_gpio_function_disable(AR724X_GPIO_FUNC_ETH_SWITCH_LED0_EN |
				AR724X_GPIO_FUNC_ETH_SWITCH_LED1_EN |
				AR724X_GPIO_FUNC_ETH_SWITCH_LED2_EN |
				AR724X_GPIO_FUNC_ETH_SWITCH_LED3_EN |
				AR724X_GPIO_FUNC_ETH_SWITCH_LED4_EN);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(rme_eg200_leds_gpio),
				 rme_eg200_leds_gpio);
	ath79_register_gpio_keys_polled(-1, RME_EG200_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(rme_eg200_keys),
					rme_eg200_keys);
	ath79_register_usb();
}

MIPS_MACHINE(ATH79_MACH_RME_EG200, "RME-EG200", "eTactica EG-200",
		rme_eg200_setup);

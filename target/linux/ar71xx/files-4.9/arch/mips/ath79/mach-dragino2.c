/*
 *  DRAGINO V2 board support, based on Atheros AP121 board support
 *
 *  Copyright (C) 2011-2012 Gabor Juhos <juhosg@openwrt.org>
 *  Copyright (C) 2012 Elektra Wagenrad <elektra@villagetelco.org>
 *  Copyright (C) 2014 Vittorio Gambaletta <openwrt@vittgam.net>
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
#include "dev-spi.h"
#include "dev-usb.h"
#include "dev-wmac.h"
#include "machtypes.h"

#define DRAGINO2_GPIO_LED_WLAN		0
#define DRAGINO2_GPIO_LED_LAN		13
#define DRAGINO2_GPIO_LED_WAN		17

/*
 * The following GPIO is named "SYS" on newer revisions of the the board.
 * It was previously used to indicate USB activity, even though it was
 * named "Router".
 */

#define DRAGINO2_GPIO_LED_SYS		28
#define DRAGINO2_GPIO_BTN_JUMPSTART	11
#define DRAGINO2_GPIO_BTN_RESET		12

#define DRAGINO2_KEYS_POLL_INTERVAL	20	/* msecs */
#define DRAGINO2_KEYS_DEBOUNCE_INTERVAL	(3 * DRAGINO2_KEYS_POLL_INTERVAL)

#define DRAGINO2_MAC0_OFFSET		0x0000
#define DRAGINO2_MAC1_OFFSET		0x0006
#define DRAGINO2_CALDATA_OFFSET		0x1000
#define DRAGINO2_WMAC_MAC_OFFSET	0x1002

static struct gpio_led dragino2_leds_gpio[] __initdata = {
	{
		.name		= "dragino2:red:wlan",
		.gpio		= DRAGINO2_GPIO_LED_WLAN,
		.active_low	= 0,
	},
	{
		.name		= "dragino2:red:wan",
		.gpio		= DRAGINO2_GPIO_LED_WAN,
		.active_low	= 1,
	},
	{
		.name		= "dragino2:red:lan",
		.gpio		= DRAGINO2_GPIO_LED_LAN,
		.active_low	= 1,
	},
	{
		.name		= "dragino2:red:system",
		.gpio		= DRAGINO2_GPIO_LED_SYS,
		.active_low	= 0,
	},
};

static struct gpio_keys_button dragino2_gpio_keys[] __initdata = {
	{
		.desc		= "jumpstart button",
		.type		= EV_KEY,
		.code		= KEY_WPS_BUTTON,
		.debounce_interval = DRAGINO2_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= DRAGINO2_GPIO_BTN_JUMPSTART,
		.active_low	= 1,
	},
	{
		.desc		= "reset button",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = DRAGINO2_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= DRAGINO2_GPIO_BTN_RESET,
		.active_low	= 1,
	}
};

static void __init dragino2_common_setup(void)
{
	u8 *art = (u8 *) KSEG1ADDR(0x1fff0000);

	ath79_register_m25p80(NULL);
	ath79_register_wmac(art + DRAGINO2_CALDATA_OFFSET,
			    art + DRAGINO2_WMAC_MAC_OFFSET);

	ath79_init_mac(ath79_eth0_data.mac_addr, art + DRAGINO2_MAC0_OFFSET, 0);
	ath79_init_mac(ath79_eth1_data.mac_addr, art + DRAGINO2_MAC1_OFFSET, 0);

	ath79_register_mdio(0, 0x0);

	/* Enable GPIO13, GPIO14, GPIO15, GPIO16 and GPIO17 */
	ath79_gpio_function_disable(AR933X_GPIO_FUNC_ETH_SWITCH_LED0_EN |
				    AR933X_GPIO_FUNC_ETH_SWITCH_LED1_EN |
				    AR933X_GPIO_FUNC_ETH_SWITCH_LED2_EN |
				    AR933X_GPIO_FUNC_ETH_SWITCH_LED3_EN |
				    AR933X_GPIO_FUNC_ETH_SWITCH_LED4_EN);

	/* LAN port */
	ath79_register_eth(1);

	/* WAN port */
	ath79_register_eth(0);

	/* Enable GPIO26 and GPIO27 */
	ath79_reset_wr(AR933X_RESET_REG_BOOTSTRAP,
		       ath79_reset_rr(AR933X_RESET_REG_BOOTSTRAP) |
		       AR933X_BOOTSTRAP_MDIO_GPIO_EN);
}

static void __init dragino2_setup(void)
{
	dragino2_common_setup();

	ath79_register_leds_gpio(-1, ARRAY_SIZE(dragino2_leds_gpio),
				 dragino2_leds_gpio);
	ath79_register_gpio_keys_polled(-1, DRAGINO2_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(dragino2_gpio_keys),
					dragino2_gpio_keys);
	ath79_register_usb();
}

MIPS_MACHINE(ATH79_MACH_DRAGINO2, "DRAGINO2", "Dragino Dragino v2",
	     dragino2_setup);


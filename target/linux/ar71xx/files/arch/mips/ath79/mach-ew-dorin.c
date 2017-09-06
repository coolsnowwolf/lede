/*
 *  EW Dorin board support
 *  (based on Atheros Ref. Design AP121)
 *  Copyright (C) 2011-2012 Gabor Juhos <juhosg@openwrt.org>
 *  Copyright (C) 2012-2015 Embedded Wireless GmbH    www.80211.de
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <asm/mach-ath79/ath79.h>
#include <asm/mach-ath79/ar71xx_regs.h>

#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-spi.h"
#include "dev-usb.h"
#include "dev-wmac.h"
#include "machtypes.h"

#define DORIN_KEYS_POLL_INTERVAL	20	/* msecs */
#define DORIN_KEYS_DEBOUNCE_INTERVAL	(3 * DORIN_KEYS_POLL_INTERVAL)

#define DORIN_CALDATA_OFFSET		0x1000
#define DORIN_WMAC_MAC_OFFSET		0x1002

#define DORIN_GPIO_LED_21		21
#define DORIN_GPIO_LED_22		22
#define DORIN_GPIO_LED_STATUS	23

#define DORIN_GPIO_BTN_JUMPSTART	11
#define DORIN_GPIO_BTN_RESET		6

static struct gpio_led dorin_leds_gpio[] __initdata = {
	{
		.name		= "dorin:green:led21",
		.gpio		= DORIN_GPIO_LED_21,
		.active_low	= 1,
	},
	{
		.name		= "dorin:green:led22",
		.gpio		= DORIN_GPIO_LED_22,
		.active_low	= 1,
	},
	{
		.name		= "dorin:green:status",
		.gpio		= DORIN_GPIO_LED_STATUS,
		.active_low	= 1,
	},
};

static struct gpio_keys_button dorin_gpio_keys[] __initdata = {
	{
		.desc		= "jumpstart button",
		.type		= EV_KEY,
		.code		= KEY_WPS_BUTTON,
		.debounce_interval = DORIN_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= DORIN_GPIO_BTN_JUMPSTART,
		.active_low	= 1,
	},
	{
		.desc		= "reset button",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = DORIN_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= DORIN_GPIO_BTN_RESET,
		.active_low	= 0,
	}
};

static void __init ew_dorin_setup(void)
{
	u8 *art = (u8 *) KSEG1ADDR(0x1fff0000);
	static u8 mac[6];

	ath79_register_m25p80(NULL);

	ath79_register_usb();

	if (ar93xx_wmac_read_mac_address(mac)) {
		ath79_register_wmac(NULL, NULL);
	} else {
		ath79_register_wmac(art + DORIN_CALDATA_OFFSET,
				    art + DORIN_WMAC_MAC_OFFSET);
		memcpy(mac, art + DORIN_WMAC_MAC_OFFSET, sizeof(mac));
	}

	mac[3] |= 0x40;
	ath79_init_mac(ath79_eth1_data.mac_addr, mac, 0);

	ath79_register_mdio(0, 0x0);

	/* LAN ports */
	ath79_register_eth(1);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(dorin_leds_gpio),
				 dorin_leds_gpio);
	ath79_register_gpio_keys_polled(-1, DORIN_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(dorin_gpio_keys),
					dorin_gpio_keys);
}

MIPS_MACHINE(ATH79_MACH_EW_DORIN, "EW-DORIN", "EmbWir-Dorin",
	     ew_dorin_setup);


static void __init ew_dorin_router_setup(void)
{
	u8 *art = (u8 *) KSEG1ADDR(0x1fff0000);
	static u8 mac[6];

	ath79_register_m25p80(NULL);

	ath79_register_usb();

	if (ar93xx_wmac_read_mac_address(mac)) {
		ath79_register_wmac(NULL, NULL);
	} else {
		ath79_register_wmac(art + DORIN_CALDATA_OFFSET,
				    art + DORIN_WMAC_MAC_OFFSET);
		memcpy(mac, art + DORIN_WMAC_MAC_OFFSET, sizeof(mac));
	}

	mac[3] |= 0x40;
	ath79_init_mac(ath79_eth1_data.mac_addr, mac, 0);

	mac[3] &= 0x3F;
	ath79_init_mac(ath79_eth0_data.mac_addr, mac, 0);
	ath79_setup_ar933x_phy4_switch(true, true);

	ath79_register_mdio(0, 0x0);

	/* LAN ports */
	ath79_register_eth(1);

	/* WAN port */
	ath79_register_eth(0);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(dorin_leds_gpio),
				 dorin_leds_gpio);
	ath79_register_gpio_keys_polled(-1, DORIN_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(dorin_gpio_keys),
					dorin_gpio_keys);
}

MIPS_MACHINE(ATH79_MACH_EW_DORIN_ROUTER, "EW-DORIN-ROUTER",
	     "EmbWir-Dorin-Router", ew_dorin_router_setup);

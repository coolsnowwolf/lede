/*
 * Samsung WAM250 board support
 *
 * Copyright (C) 2018 Piotr Dymacz <pepe2k@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 */

#include <linux/gpio.h>
#include <linux/platform_device.h>

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

#define WAM250_GPIO_LED_LAN		13
#define WAM250_GPIO_LED_POWER		15
#define WAM250_GPIO_LED_REPEATER	14
#define WAM250_GPIO_LED_WLAN		12

#define WAM250_GPIO_BTN_RESET		17
#define WAM250_GPIO_BTN_SPKADD		1

#define WAM250_GPIO_EXT_LNA		19

#define WAM250_MAC_OFFSET		2

#define WAM250_KEYS_POLL_INTERVAL	20
#define WAM250_KEYS_DEBOUNCE_INTERVAL	(3 * WAM250_KEYS_POLL_INTERVAL)

static struct gpio_led wam250_leds_gpio[] __initdata = {
	{
		.name		= "wam250:white:lan",
		.gpio		= WAM250_GPIO_LED_LAN,
		.active_low	= 1,
	}, {
		.name		= "wam250:white:power",
		.gpio		= WAM250_GPIO_LED_POWER,
		.default_state	= LEDS_GPIO_DEFSTATE_KEEP,
		.active_low	= 1,
	}, {
		.name		= "wam250:white:repeater",
		.gpio		= WAM250_GPIO_LED_REPEATER,
		.active_low	= 1,
	}, {
		.name		= "wam250:white:wlan",
		.gpio		= WAM250_GPIO_LED_WLAN,
		.active_low	= 1,
	},
};

static struct gpio_keys_button wam250_gpio_keys[] __initdata = {
	{
		.desc			= "reset",
		.type			= EV_KEY,
		.code			= KEY_RESTART,
		.debounce_interval	= WAM250_KEYS_DEBOUNCE_INTERVAL,
		.gpio			= WAM250_GPIO_BTN_RESET,
		.active_low		= 1,
	}, {
		.desc			= "wps",
		.type			= EV_KEY,
		.code			= KEY_WPS_BUTTON,
		.debounce_interval	= WAM250_KEYS_DEBOUNCE_INTERVAL,
		.gpio			= WAM250_GPIO_BTN_SPKADD,
		.active_low		= 1,
	},
};

static void __init wam250_setup(void)
{
	u8 *art = (u8 *) KSEG1ADDR(0x1fff1000);

	ath79_register_m25p80(NULL);

	ath79_register_mdio(1, 0x0);

	ath79_setup_ar934x_eth_cfg(AR934X_ETH_CFG_SW_PHY_SWAP);

	ath79_switch_data.phy4_mii_en = 1;
	ath79_switch_data.phy_poll_mask = 0xfd;

	/* LAN */
	ath79_eth1_data.duplex = DUPLEX_FULL;
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_GMII;
	ath79_eth1_data.phy_mask = BIT(1);
	ath79_init_mac(ath79_eth1_data.mac_addr, art + WAM250_MAC_OFFSET, 0);
	ath79_register_eth(1);

	/* WAN */
	ath79_eth0_data.duplex = DUPLEX_FULL;
	ath79_eth0_data.mii_bus_dev = &ath79_mdio1_device.dev;
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ath79_eth0_data.phy_mask = BIT(0);
	ath79_eth0_data.speed = SPEED_100;
	ath79_init_mac(ath79_eth0_data.mac_addr, art + WAM250_MAC_OFFSET, 1);
	ath79_register_eth(0);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(wam250_leds_gpio),
				 wam250_leds_gpio);

	ath79_register_gpio_keys_polled(-1, WAM250_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(wam250_gpio_keys),
					wam250_gpio_keys);

	ath79_wmac_set_ext_lna_gpio(0, WAM250_GPIO_EXT_LNA);

	ath79_register_usb();
	ath79_register_wmac(art, NULL);
}

MIPS_MACHINE(ATH79_MACH_WAM250, "WAM250", "Samsung WAM250", wam250_setup);

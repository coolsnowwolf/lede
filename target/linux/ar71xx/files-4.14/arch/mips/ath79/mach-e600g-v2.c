/*
 *  Qxwlan E600G/E600GAC v2 board support
 *
 *  Copyright (C) 2017 Peng Zhang <sd20@qxwlan.com>
 *  Copyright (C) 2018 Piotr Dymacz <pepe2k@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include "common.h"
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-usb.h"
#include "dev-wmac.h"
#include "machtypes.h"
#include "pci.h"

#define E600G_V2_GPIO_LED_LAN		16
#define E600G_V2_GPIO_LED_SYS		13
#define E600G_V2_GPIO_LED_WAN_B		4
#define E600G_V2_GPIO_LED_WAN_G		15

#define E600GAC_V2_GPIO_LED_CTRL_B	14
#define E600GAC_V2_GPIO_LED_CTRL_G	11
#define E600GAC_V2_GPIO_LED_CTRL_R	12
#define E600GAC_V2_GPIO_LED_LAN		16
#define E600GAC_V2_GPIO_LED_SYS		13
#define E600GAC_V2_GPIO_LED_WAN_G	15
#define E600GAC_V2_GPIO_LED_WAN_O	4

#define E600G_V2_GPIO_BTN_RESET		17
#define E600GAC_V2_GPIO_BTN_WPS		1

#define E600G_V2_KEYS_POLL_INTERVAL	20 /* msecs */
#define E600G_V2_KEYS_DEBOUNCE_INTERVAL	(3 * E600G_V2_KEYS_POLL_INTERVAL)

static struct gpio_led e600g_v2_leds_gpio[] __initdata = {
	{
		.name		= "e600g-v2:blue:system",
		.gpio		= E600G_V2_GPIO_LED_SYS,
		.active_low	= 1,
	}, {
		.name		= "e600g-v2:blue:wan",
		.gpio		= E600G_V2_GPIO_LED_WAN_B,
		.active_low	= 1,
	}, {
		.name		= "e600g-v2:green:lan",
		.gpio		= E600G_V2_GPIO_LED_LAN,
		.active_low	= 1,
	}, {
		.name		= "e600g-v2:green:wan",
		.gpio		= E600G_V2_GPIO_LED_WAN_G,
		.active_low	= 1,
	},
};

static struct gpio_led e600gac_v2_leds_gpio[] __initdata = {
	{
		.name		= "e600gac-v2:blue:control",
		.gpio		= E600GAC_V2_GPIO_LED_CTRL_B,
		.active_low	= 1,
	}, {
		.name		= "e600gac-v2:green:control",
		.gpio		= E600GAC_V2_GPIO_LED_CTRL_G,
		.active_low	= 1,
	}, {
		.name		= "e600gac-v2:red:control",
		.gpio		= E600GAC_V2_GPIO_LED_CTRL_R,
		.active_low	= 1,
	}, {
		.name		= "e600gac-v2:green:system",
		.gpio		= E600GAC_V2_GPIO_LED_SYS,
		.active_low	= 1,
	}, {
		.name		= "e600gac-v2:orange:wan",
		.gpio		= E600GAC_V2_GPIO_LED_WAN_O,
		.active_low	= 1,
	}, {
		.name		= "e600gac-v2:green:lan",
		.gpio		= E600GAC_V2_GPIO_LED_LAN,
		.active_low	= 1,
	}, {
		.name		= "e600gac-v2:green:wan",
		.gpio		= E600GAC_V2_GPIO_LED_WAN_G,
		.active_low	= 1,
	},
};

static struct gpio_keys_button e600g_v2_gpio_keys[] __initdata = {
	{
		.desc			= "reset",
		.type			= EV_KEY,
		.code			= KEY_RESTART,
		.debounce_interval	= E600G_V2_KEYS_DEBOUNCE_INTERVAL,
		.gpio			= E600G_V2_GPIO_BTN_RESET,
		.active_low		= 1,
	},
};

static struct gpio_keys_button e600gac_v2_gpio_keys[] __initdata = {
	{
		.desc			= "reset",
		.type			= EV_KEY,
		.code			= KEY_RESTART,
		.debounce_interval	= E600G_V2_KEYS_DEBOUNCE_INTERVAL,
		.gpio			= E600G_V2_GPIO_BTN_RESET,
		.active_low		= 1,
	}, {
		.desc			= "wps",
		.type			= EV_KEY,
		.code			= KEY_WPS_BUTTON,
		.debounce_interval	= E600G_V2_KEYS_DEBOUNCE_INTERVAL,
		.gpio			= E600GAC_V2_GPIO_BTN_WPS,
		.active_low		= 1,
	},
};

static void __init e600g_v2_common_setup(void)
{
	u8 *mac = (u8 *) KSEG1ADDR(0x1f050400);
	u8 *art = (u8 *) KSEG1ADDR(0x1f061000);

	ath79_register_m25p80(NULL);

	ath79_setup_ar933x_phy4_switch(false, false);

	ath79_register_mdio(0, 0x0);

	ath79_switch_data.phy4_mii_en = 1;
	ath79_switch_data.phy_poll_mask = 0xfe;

	/* LAN */
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ath79_eth0_data.phy_mask = BIT(4);
	ath79_eth0_data.speed = SPEED_100;
	ath79_init_mac(ath79_eth0_data.mac_addr, mac, 1);
	ath79_register_eth(0);

	/* WAN */
	ath79_eth1_data.duplex = DUPLEX_FULL;
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_GMII;
	ath79_eth1_data.phy_mask = BIT(0);
	ath79_eth1_data.speed = SPEED_1000;
	ath79_init_mac(ath79_eth1_data.mac_addr, mac, 0);
	ath79_register_eth(1);

	ath79_register_pci();
	ath79_register_usb();
	ath79_register_wmac(art, NULL);
}

static void __init e600g_v2_setup(void)
{
	e600g_v2_common_setup();

	ath79_register_leds_gpio(-1, ARRAY_SIZE(e600g_v2_leds_gpio),
				 e600g_v2_leds_gpio);

	ath79_register_gpio_keys_polled(-1, E600G_V2_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(e600g_v2_gpio_keys),
					e600g_v2_gpio_keys);
}

static void __init e600gac_v2_setup(void)
{
	e600g_v2_common_setup();

	ath79_register_leds_gpio(-1, ARRAY_SIZE(e600gac_v2_leds_gpio),
				 e600gac_v2_leds_gpio);

	ath79_register_gpio_keys_polled(-1, E600G_V2_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(e600gac_v2_gpio_keys),
					e600gac_v2_gpio_keys);
}

MIPS_MACHINE(ATH79_MACH_E600G_V2, "E600G-V2", "Qxwlan E600G v2",
	     e600g_v2_setup);

MIPS_MACHINE(ATH79_MACH_E600GAC_V2, "E600GAC-V2", "Qxwlan E600GAC v2",
	     e600gac_v2_setup);

/*
 * P&W (Shenzhen Progress&Win Technologies) R602N and CPE505N boards support
 *
 * Copyright (C) 2017 Piotr Dymacz <pepe2k@gmail.com>
 *
 * Based on mach-zbt-we1526.c
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

#define PW_GPIO_BTN_RESET	17

#define PW_KEYS_POLL_INTERVAL		20 /* msecs */
#define PW_KEYS_DEBOUNCE_INTERVAL	(3 * PW_KEYS_POLL_INTERVAL)

#define PW_MAC0_OFFSET		0x0
#define PW_MAC1_OFFSET		0x6
#define PW_WMAC_CALDATA_OFFSET	0x1000

/* CPE505N GPIO LEDs */
#define CPE505N_GPIO_LED_DIAG	12
#define CPE505N_GPIO_LED_LAN	11
#define CPE505N_GPIO_LED_STATUS	14
#define CPE505N_GPIO_LED_WAN	4
#define CPE505N_GPIO_LED_WLAN	15

static struct gpio_led cpe505n_leds_gpio[] __initdata = {
	{
		.name		= "cpe505n:red:diag",
		.gpio		= CPE505N_GPIO_LED_DIAG,
		.active_low	= 1,
	}, {
		.name		= "cpe505n:green:lan",
		.gpio		= CPE505N_GPIO_LED_LAN,
		.active_low	= 1,
	}, {
		.name		= "cpe505n:green:status",
		.gpio		= CPE505N_GPIO_LED_STATUS,
		.active_low	= 1,
	}, {
		.name		= "cpe505n:green:wan",
		.gpio		= CPE505N_GPIO_LED_WAN,
		.active_low	= 1,
	}, {
		.name		= "cpe505n:blue:wlan",
		.gpio		= CPE505N_GPIO_LED_WLAN,
		.active_low	= 1,
	},
};

static void __init cpe505n_gpio_setup(void)
{
	/* For LED on GPIO4 */
	ath79_gpio_function_disable(AR934X_GPIO_FUNC_CLK_OBS4_EN);
	ath79_gpio_output_select(CPE505N_GPIO_LED_WAN, 0);

	ath79_gpio_direction_select(CPE505N_GPIO_LED_DIAG, true);
	ath79_gpio_direction_select(CPE505N_GPIO_LED_LAN, true);
	ath79_gpio_direction_select(CPE505N_GPIO_LED_STATUS, true);
	ath79_gpio_direction_select(CPE505N_GPIO_LED_WAN, true);
	ath79_gpio_direction_select(CPE505N_GPIO_LED_WLAN, true);

	/* Mute LEDs */
	gpio_set_value(CPE505N_GPIO_LED_DIAG, 1);
	gpio_set_value(CPE505N_GPIO_LED_LAN, 1);
	gpio_set_value(CPE505N_GPIO_LED_STATUS, 1);
	gpio_set_value(CPE505N_GPIO_LED_WAN, 1);
	gpio_set_value(CPE505N_GPIO_LED_WLAN, 1);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(cpe505n_leds_gpio),
				 cpe505n_leds_gpio);
}

/* R602N GPIO LEDs */
#define R602N_GPIO_LED_LAN1	16
#define R602N_GPIO_LED_LAN2	15
#define R602N_GPIO_LED_LAN3	14
#define R602N_GPIO_LED_LAN4	11
#define R602N_GPIO_LED_WAN	4
#define R602N_GPIO_LED_WLAN	12

static struct gpio_led r602n_leds_gpio[] __initdata = {
	{
		.name		= "r602n:green:lan1",
		.gpio		= R602N_GPIO_LED_LAN1,
		.active_low	= 1,
	}, {
		.name		= "r602n:green:lan2",
		.gpio		= R602N_GPIO_LED_LAN2,
		.active_low	= 1,
	}, {
		.name		= "r602n:green:lan3",
		.gpio		= R602N_GPIO_LED_LAN3,
		.active_low	= 1,
	}, {
		.name		= "r602n:green:lan4",
		.gpio		= R602N_GPIO_LED_LAN4,
		.active_low	= 1,
	}, {
		.name		= "r602n:green:wan",
		.gpio		= R602N_GPIO_LED_WAN,
		.active_low	= 1,
	}, {
		.name		= "r602n:green:wlan",
		.gpio		= R602N_GPIO_LED_WLAN,
		.active_low	= 1,
	},
};

static void __init r602n_gpio_setup(void)
{
	/* For LED on GPIO4 */
	ath79_gpio_function_disable(AR934X_GPIO_FUNC_CLK_OBS4_EN);
	ath79_gpio_output_select(R602N_GPIO_LED_WAN, 0);

	ath79_gpio_direction_select(R602N_GPIO_LED_LAN1, true);
	ath79_gpio_direction_select(R602N_GPIO_LED_LAN2, true);
	ath79_gpio_direction_select(R602N_GPIO_LED_LAN3, true);
	ath79_gpio_direction_select(R602N_GPIO_LED_LAN4, true);
	ath79_gpio_direction_select(R602N_GPIO_LED_WAN, true);
	ath79_gpio_direction_select(R602N_GPIO_LED_WLAN, true);

	/* Mute LEDs */
	gpio_set_value(R602N_GPIO_LED_LAN1, 1);
	gpio_set_value(R602N_GPIO_LED_LAN2, 1);
	gpio_set_value(R602N_GPIO_LED_LAN3, 1);
	gpio_set_value(R602N_GPIO_LED_LAN4, 1);
	gpio_set_value(R602N_GPIO_LED_WAN, 1);
	gpio_set_value(R602N_GPIO_LED_WLAN, 1);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(r602n_leds_gpio),
				 r602n_leds_gpio);
}

static struct gpio_keys_button pw_gpio_keys[] __initdata = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = PW_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= PW_GPIO_BTN_RESET,
		.active_low	= 1,
	},
};

static void __init r602n_cpe505n_setup(void)
{
	u8 *art = (u8 *) KSEG1ADDR(0x1fff0000);

	ath79_register_m25p80(NULL);

	ath79_setup_ar933x_phy4_switch(false, false);

	ath79_register_mdio(0, 0x0);

	/* LAN */
	ath79_eth1_data.duplex = DUPLEX_FULL;
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_GMII;
	ath79_switch_data.phy_poll_mask |= BIT(4);
	ath79_init_mac(ath79_eth1_data.mac_addr, art + PW_MAC1_OFFSET, 0);
	ath79_register_eth(1);

	/* WAN */
	ath79_switch_data.phy4_mii_en = 1;
	ath79_eth0_data.duplex = DUPLEX_FULL;
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ath79_eth0_data.phy_mask = BIT(4);
	ath79_eth0_data.speed = SPEED_100;
	ath79_init_mac(ath79_eth0_data.mac_addr, art + PW_MAC0_OFFSET, 0);
	ath79_register_eth(0);

	ath79_register_wmac(art + PW_WMAC_CALDATA_OFFSET, NULL);

	ath79_register_gpio_keys_polled(-1, PW_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(pw_gpio_keys),
					pw_gpio_keys);
}

static void __init cpe505n_setup(void)
{
	r602n_cpe505n_setup();

	cpe505n_gpio_setup();
}

static void __init r602n_setup(void)
{
	r602n_cpe505n_setup();

	r602n_gpio_setup();

	ath79_register_usb();
}

MIPS_MACHINE(ATH79_MACH_CPE505N, "CPE505N", "P&W CPE505N", cpe505n_setup);
MIPS_MACHINE(ATH79_MACH_R602N, "R602N", "P&W R602N", r602n_setup);

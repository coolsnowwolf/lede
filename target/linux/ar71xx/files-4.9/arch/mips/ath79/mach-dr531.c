/*
 * Wallys DR531 board support
 *
 * Copyright (C) 2016 Piotr Dymacz <pepe2k@gmail.com>
 *
 * Based on mach-wpj531.c
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 */

#include <linux/pci.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>

#include <asm/mach-ath79/ath79.h>
#include <asm/mach-ath79/ar71xx_regs.h>

#include "common.h"
#include "dev-ap9x-pci.h"
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-usb.h"
#include "dev-wmac.h"
#include "machtypes.h"
#include "pci.h"

#define DR531_GPIO_BUZZER	4
#define DR531_GPIO_LED_WAN	11
#define DR531_GPIO_LED_LAN	14
#define DR531_GPIO_LED_SIG1	12
#define DR531_GPIO_LED_SIG2	16
#define DR531_GPIO_LED_SIG3	15
#define DR531_GPIO_LED_SIG4	13

#define DR531_GPIO_BTN_RESET	17

#define DR531_KEYS_POLL_INTERVAL	20	/* msecs */
#define DR531_KEYS_DEBOUNCE_INTERVAL	(3 * DR531_KEYS_POLL_INTERVAL)

#define DR531_MAC0_OFFSET		0x0
#define DR531_MAC1_OFFSET		0x8
#define DR531_WMAC_CALDATA_OFFSET	0x1000

static struct gpio_led dr531_leds_gpio[] __initdata = {
	{
		.name		= "dr531:green:wan",
		.gpio		= DR531_GPIO_LED_WAN,
		.active_low	= 1,
	},
	{
		.name		= "dr531:green:lan",
		.gpio		= DR531_GPIO_LED_LAN,
		.active_low	= 1,
	},
	{
		.name		= "dr531:green:sig1",
		.gpio		= DR531_GPIO_LED_SIG1,
		.active_low	= 1,
	},
	{
		.name		= "dr531:green:sig2",
		.gpio		= DR531_GPIO_LED_SIG2,
		.active_low	= 1,
	},
	{
		.name		= "dr531:green:sig3",
		.gpio		= DR531_GPIO_LED_SIG3,
		.active_low	= 1,
	},
	{
		.name		= "dr531:green:sig4",
		.gpio		= DR531_GPIO_LED_SIG4,
		.active_low	= 1,
	},
	{
		.name		= "dr531:buzzer",
		.gpio		= DR531_GPIO_BUZZER,
		.active_low	= 0,
	}
};

static struct gpio_keys_button dr531_gpio_keys[] __initdata = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = DR531_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= DR531_GPIO_BTN_RESET,
		.active_low	= 1,
	},
};

static void __init dr531_gpio_setup(void)
{
	ath79_gpio_direction_select(DR531_GPIO_BUZZER, true);
	ath79_gpio_direction_select(DR531_GPIO_LED_WAN, true);
	ath79_gpio_direction_select(DR531_GPIO_LED_LAN, true);
	ath79_gpio_direction_select(DR531_GPIO_LED_SIG1, true);
	ath79_gpio_direction_select(DR531_GPIO_LED_SIG2, true);
	ath79_gpio_direction_select(DR531_GPIO_LED_SIG3, true);
	ath79_gpio_direction_select(DR531_GPIO_LED_SIG4, true);

	ath79_gpio_output_select(DR531_GPIO_BUZZER, 0);
	ath79_gpio_output_select(DR531_GPIO_LED_WAN, 0);
	ath79_gpio_output_select(DR531_GPIO_LED_LAN, 0);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(dr531_leds_gpio),
				 dr531_leds_gpio);

	ath79_register_gpio_keys_polled(-1, DR531_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(dr531_gpio_keys),
					dr531_gpio_keys);
}

static void __init dr531_setup(void)
{
	u8 *art = (u8 *) KSEG1ADDR(0x1fff0000);
	u8 *mac = (u8 *) KSEG1ADDR(0x1f03f810);

	ath79_register_m25p80(NULL);

	dr531_gpio_setup();

	ath79_setup_ar933x_phy4_switch(false, false);

	ath79_register_mdio(0, 0x0);

	/* LAN */
	ath79_eth0_data.duplex = DUPLEX_FULL;
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ath79_eth0_data.speed = SPEED_100;
	ath79_eth0_data.phy_mask = BIT(4);
	ath79_init_mac(ath79_eth0_data.mac_addr, mac + DR531_MAC1_OFFSET, 0);
	ath79_register_eth(0);

	/* WAN */
	ath79_switch_data.phy4_mii_en = 1;
	ath79_eth1_data.duplex = DUPLEX_FULL;
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_GMII;
	ath79_eth1_data.speed = SPEED_1000;
	ath79_switch_data.phy_poll_mask |= BIT(4);
	ath79_init_mac(ath79_eth1_data.mac_addr, mac + DR531_MAC0_OFFSET, 0);
	ath79_register_eth(1);

	ath79_register_wmac(art + DR531_WMAC_CALDATA_OFFSET, NULL);

	ath79_register_pci();
	ath79_register_usb();
}

MIPS_MACHINE(ATH79_MACH_DR531, "DR531", "Wallys DR531", dr531_setup);

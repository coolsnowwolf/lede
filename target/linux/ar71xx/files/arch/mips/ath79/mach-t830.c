/*
 * YunCore T830 board support
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

#define T830_GPIO_LED_LAN1	16
#define T830_GPIO_LED_LAN2	15
#define T830_GPIO_LED_LAN3	14
#define T830_GPIO_LED_LAN4	11
#define T830_GPIO_LED_USB	13
#define T830_GPIO_LED_WAN	4
#define T830_GPIO_LED_WLAN	12

#define T830_GPIO_BTN_RESET	17

#define T830_KEYS_POLL_INTERVAL		20 /* msec */
#define T830_KEYS_DEBOUNCE_INTERVAL	(3 * T830_KEYS_POLL_INTERVAL)

#define T830_WMAC_CALDATA_OFFSET	0x1000

static struct gpio_led t830_gpio_leds[] __initdata = {
	{
		.name		= "t830:green:lan1",
		.gpio		= T830_GPIO_LED_LAN1,
		.active_low	= 1,
	}, {
		.name		= "t830:green:lan2",
		.gpio		= T830_GPIO_LED_LAN2,
		.active_low	= 1,
	}, {
		.name		= "t830:green:lan3",
		.gpio		= T830_GPIO_LED_LAN3,
		.active_low	= 1,
	}, {
		.name		= "t830:green:lan4",
		.gpio		= T830_GPIO_LED_LAN4,
		.active_low	= 1,
	}, {
		.name		= "t830:green:usb",
		.gpio		= T830_GPIO_LED_USB,
		.active_low	= 1,
		.default_state	= LEDS_GPIO_DEFSTATE_KEEP,
	}, {
		.name		= "t830:green:wan",
		.gpio		= T830_GPIO_LED_WAN,
		.active_low	= 1,
	}, {
		.name		= "t830:green:wlan",
		.gpio		= T830_GPIO_LED_WLAN,
		.active_low	= 1,
	},
};

static struct gpio_keys_button t830_gpio_keys[] __initdata = {
	{
		.desc			= "reset",
		.type			= EV_KEY,
		.code			= KEY_RESTART,
		.debounce_interval	= T830_KEYS_DEBOUNCE_INTERVAL,
		.gpio			= T830_GPIO_BTN_RESET,
		.active_low		= 1,
	},
};

static void __init t830_setup(void)
{
	u8 *art = (u8 *) KSEG1ADDR(0x1fff1000);
	u8 *mac = (u8 *) KSEG1ADDR(0x1fff0000);

	ath79_register_m25p80(NULL);

	ath79_setup_ar933x_phy4_switch(false, false);

	ath79_register_mdio(0, 0x0);

	ath79_switch_data.phy4_mii_en = 1;
	ath79_switch_data.phy_poll_mask |= BIT(4);

	/* LAN */
	ath79_eth1_data.duplex = DUPLEX_FULL;
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_GMII;
	ath79_init_mac(ath79_eth1_data.mac_addr, mac + 6, 0);
	ath79_register_eth(1);

	/* WAN */
	ath79_eth0_data.duplex = DUPLEX_FULL;
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ath79_eth0_data.phy_mask = BIT(4);
	ath79_eth0_data.speed = SPEED_100;
	ath79_init_mac(ath79_eth0_data.mac_addr, mac, 0);
	ath79_register_eth(0);

	ath79_gpio_function_setup(AR934X_GPIO_FUNC_JTAG_DISABLE,
				  AR934X_GPIO_FUNC_CLK_OBS4_EN);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(t830_gpio_leds),
				 t830_gpio_leds);

	ath79_register_gpio_keys_polled(-1, T830_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(t830_gpio_keys),
					t830_gpio_keys);

	ath79_register_usb();
	ath79_register_wmac(art, NULL);
}

MIPS_MACHINE(ATH79_MACH_T830, "T830", "YunCore T830", t830_setup);

/*
 *  Mercury MW316R v1
 *
 *  Copyright (C) 2014 Matthias Schiffer <mschiffer@universe-factory.net>
 *  Copyright (C) 2017 Weijie Gao <hackpascal@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
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
#include "dev-wmac.h"
#include "dev-usb.h"
#include "machtypes.h"

#define MW316RV1_GPIO_LED_SYS	13
#define MW316RV1_GPIO_LED_WAN	4
#define MW316RV1_GPIO_LED_LAN1	11
#define MW316RV1_GPIO_LED_LAN2	14
#define MW316RV1_GPIO_LED_LAN3	15
#define MW316RV1_GPIO_LED_LAN4	16
#define MW316RV1_GPIO_LED_TURBO	17

#define MW316RV1_GPIO_BTN_RESET	12
#define MW316RV1_GPIO_BTN_TURBO	1

#define MW316RV1_KEYS_POLL_INTERVAL	20	/* msecs */
#define MW316RV1_KEYS_DEBOUNCE_INTERVAL (3 * MW316RV1_KEYS_POLL_INTERVAL)

static const char *mw316r_v1_part_probes[] = {
	"tp-link",
	NULL,
};

static struct flash_platform_data mw316r_v1_flash_data = {
	.part_probes	= mw316r_v1_part_probes,
};

static struct gpio_led mw316r_v1_leds_gpio[] __initdata = {
	{
		.name		= "mercury:green:wan",
		.gpio		= MW316RV1_GPIO_LED_WAN,
		.active_low	= 1,
	}, {
		.name		= "mercury:green:lan1",
		.gpio		= MW316RV1_GPIO_LED_LAN1,
		.active_low	= 1,
	}, {
		.name		= "mercury:green:lan2",
		.gpio		= MW316RV1_GPIO_LED_LAN2,
		.active_low	= 1,
	}, {
		.name		= "mercury:green:lan3",
		.gpio		= MW316RV1_GPIO_LED_LAN3,
		.active_low	= 1,
	}, {
		.name		= "mercury:green:lan4",
		.gpio		= MW316RV1_GPIO_LED_LAN4,
		.active_low	= 1,
	}, {
		.name		= "mercury:green:system",
		.gpio		= MW316RV1_GPIO_LED_SYS,
		.active_low	= 1,
	}, {
		.name		= "mercury:green:turbo",
		.gpio		= MW316RV1_GPIO_LED_TURBO,
		.active_low	= 1,
	},
};

static struct gpio_keys_button mw316r_v1_gpio_keys[] __initdata = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = MW316RV1_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= MW316RV1_GPIO_BTN_RESET,
		.active_low	= 1,
	}, {
		.desc		= "turbo",
		.type		= EV_KEY,
		.code		= BTN_0,
		.debounce_interval = MW316RV1_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= MW316RV1_GPIO_BTN_TURBO,
		.active_low	= 1,
	}
};


static void __init mw316r_v1_setup(void)
{
	u8 *mac = (u8 *) KSEG1ADDR(0x1f01fc00);
	u8 *ee = (u8 *) KSEG1ADDR(0x1fff1000);
	u8 tmpmac[ETH_ALEN];

	ath79_register_m25p80(&mw316r_v1_flash_data);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(mw316r_v1_leds_gpio),
				 mw316r_v1_leds_gpio);

	ath79_register_gpio_keys_polled(1, MW316RV1_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(mw316r_v1_gpio_keys),
					mw316r_v1_gpio_keys);

	ath79_setup_ar933x_phy4_switch(false, false);

	ath79_register_mdio(0, 0x0);

	/* WAN */
	ath79_switch_data.phy_poll_mask |= BIT(4);
	ath79_switch_data.phy4_mii_en = 1;
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ath79_eth0_data.duplex = DUPLEX_FULL;
	ath79_eth0_data.speed = SPEED_100;
	ath79_eth0_data.phy_mask = BIT(4);
	ath79_init_mac(ath79_eth0_data.mac_addr, mac, 0);
	ath79_register_eth(0);

	/* LAN */
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_GMII;
	ath79_eth1_data.duplex = DUPLEX_FULL;
	ath79_init_mac(ath79_eth1_data.mac_addr, mac, 1);
	ath79_register_eth(1);

	ath79_init_mac(tmpmac, mac, 1);
	ath79_register_wmac(ee, tmpmac);

	ath79_register_usb();
}

MIPS_MACHINE(ATH79_MACH_MW316R_V1, "MW316R-v1", "Mercury MW316R v1",
	     mw316r_v1_setup);

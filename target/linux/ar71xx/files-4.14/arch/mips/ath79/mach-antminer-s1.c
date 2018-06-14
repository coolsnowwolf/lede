/*
 *   Bitmain Antminer S1 board support
 *
 *  Copyright (C) 2015 L. D. Pinney <ldpinney@gmail.com>
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
#include "dev-wmac.h"
#include "machtypes.h"
#include "dev-usb.h"

#define ANTMINER_S1_GPIO_BTN_RESET		11

#define ANTMINER_S1_GPIO_LED_SYSTEM		23
#define ANTMINER_S1_GPIO_LED_WLAN		0
#define ANTMINER_S1_GPIO_USB_POWER		26

#define ANTMINER_S1_KEYSPOLL_INTERVAL	20	/* msecs */
#define ANTMINER_S1_KEYSDEBOUNCE_INTERVAL (3 * ANTMINER_S1_KEYSPOLL_INTERVAL)

static const char *ANTMINER_S1_part_probes[] = {
	"tp-link",
	NULL,
};

static struct flash_platform_data ANTMINER_S1_flash_data = {
	.part_probes	= ANTMINER_S1_part_probes,
};

static struct gpio_led ANTMINER_S1_leds_gpio[] __initdata = {
	{
		.name		= "antminer-s1:green:system",
		.gpio		= ANTMINER_S1_GPIO_LED_SYSTEM,
		.active_low	= 0,
	},{
		.name		= "antminer-s1:green:wlan",
		.gpio		= ANTMINER_S1_GPIO_LED_WLAN,
		.active_low	= 0,
	},
};

static struct gpio_keys_button ANTMINER_S1_GPIO_keys[] __initdata = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = ANTMINER_S1_KEYSDEBOUNCE_INTERVAL,
		.gpio		= ANTMINER_S1_GPIO_BTN_RESET,
		.active_low	= 0,
	},
};

static void __init antminer_s1_setup(void)
{
	u8 *mac = (u8 *) KSEG1ADDR(0x1f01fc00);
	u8 *ee = (u8 *) KSEG1ADDR(0x1fff1000);

	/* disable PHY_SWAP and PHY_ADDR_SWAP bits */
	ath79_setup_ar933x_phy4_switch(false, false);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(ANTMINER_S1_leds_gpio),
				 ANTMINER_S1_leds_gpio);

	ath79_register_gpio_keys_polled(-1, ANTMINER_S1_KEYSPOLL_INTERVAL,
					ARRAY_SIZE(ANTMINER_S1_GPIO_keys),
					ANTMINER_S1_GPIO_keys);

	gpio_request_one(ANTMINER_S1_GPIO_USB_POWER,
			 GPIOF_OUT_INIT_HIGH | GPIOF_EXPORT_DIR_FIXED,
			 "USB power");
	ath79_register_usb();

	ath79_register_m25p80(&ANTMINER_S1_flash_data);
	ath79_init_mac(ath79_eth0_data.mac_addr, mac, 1);
	ath79_init_mac(ath79_eth1_data.mac_addr, mac, -1);

	ath79_register_mdio(0, 0x0);
	ath79_register_eth(0);
	ath79_register_eth(1);

	ath79_register_wmac(ee, mac);
}

MIPS_MACHINE(ATH79_MACH_ANTMINER_S1, "ANTMINER-S1",
	     "Antminer-S1", antminer_s1_setup);

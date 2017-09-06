/*
 *  Bitmain Antrouter R1 board support
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

#define ANTROUTER_R1_GPIO_BTN_RESET		11

#define ANTROUTER_R1_GPIO_LED_WLAN		0
#define ANTROUTER_R1_GPIO_LED_BTC		22
#define ANTROUTER_R1_GPIO_USB_POWER		18

#define ANTROUTER_R1_KEYSPOLL_INTERVAL	44	/* msecs */
#define ANTROUTER_R1_KEYSDEBOUNCE_INTERVAL (4 * ANTROUTER_R1_KEYSPOLL_INTERVAL)

static const char *ANTROUTER_R1_part_probes[] = {
	"tp-link",
	NULL,
};

static struct flash_platform_data ANTROUTER_R1_flash_data = {
	.part_probes	= ANTROUTER_R1_part_probes,
};

static struct gpio_led ANTROUTER_R1_leds_gpio[] __initdata = {
	{
		.name		= "antrouter-r1:green:wlan",
		.gpio		= ANTROUTER_R1_GPIO_LED_WLAN,
		.active_low	= 0,
	},{
		.name		= "antrouter-r1:green:system",
		.gpio		= ANTROUTER_R1_GPIO_LED_BTC,
		.active_low	= 0,
	},
};

static struct gpio_keys_button ANTROUTER_R1_GPIO_keys[] __initdata = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = ANTROUTER_R1_KEYSDEBOUNCE_INTERVAL,
		.gpio		= ANTROUTER_R1_GPIO_BTN_RESET,
		.active_low	= 0,
	},
};

static void __init antrouter_r1_setup(void)
{
	u8 *mac = (u8 *) KSEG1ADDR(0x1f01fc00);
	u8 *ee = (u8 *) KSEG1ADDR(0x1fff1000);

	/* disable PHY_SWAP and PHY_ADDR_SWAP bits */
	ath79_setup_ar933x_phy4_switch(false, false);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(ANTROUTER_R1_leds_gpio),
				 ANTROUTER_R1_leds_gpio);

	ath79_register_gpio_keys_polled(-1, ANTROUTER_R1_KEYSPOLL_INTERVAL,
					ARRAY_SIZE(ANTROUTER_R1_GPIO_keys),
					ANTROUTER_R1_GPIO_keys);

	gpio_request_one(ANTROUTER_R1_GPIO_USB_POWER,
			 GPIOF_OUT_INIT_HIGH | GPIOF_EXPORT_DIR_FIXED,
			 "USB power");
	ath79_register_usb();

	ath79_register_m25p80(&ANTROUTER_R1_flash_data);
	ath79_init_mac(ath79_eth0_data.mac_addr, mac, 1);
	ath79_init_mac(ath79_eth1_data.mac_addr, mac, -1);

	ath79_register_mdio(0, 0x0);
	ath79_register_eth(0);
	ath79_register_eth(1);

	ath79_register_wmac(ee, mac);
}

MIPS_MACHINE(ATH79_MACH_ANTROUTER_R1, "ANTROUTER-R1",
	     "Antrouter-R1", antrouter_r1_setup);

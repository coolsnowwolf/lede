/*
 *  Easy-Link EL-M150 board support
 *
 *  Copyright (C) 2012 huangfc <huangfangcheng@163.com>
 *  Copyright (C) 2012 HYS <550663898@qq.com>
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

#define EL_M150_GPIO_BTN6		6
#define EL_M150_GPIO_BTN7		7
#define EL_M150_GPIO_BTN_RESET		11

#define EL_M150_GPIO_LED_SYSTEM		27
#define EL_M150_GPIO_USB_POWER		8

#define EL_M150_KEYS_POLL_INTERVAL	20	/* msecs */
#define EL_M150_KEYS_DEBOUNCE_INTERVAL (3 * EL_M150_KEYS_POLL_INTERVAL)

static const char *EL_M150_part_probes[] = {
	"tp-link",
	NULL,
};

static struct flash_platform_data EL_M150_flash_data = {
	.part_probes	= EL_M150_part_probes,
};

static struct gpio_led EL_M150_leds_gpio[] __initdata = {
	{
		.name		= "easylink:green:system",
		.gpio		= EL_M150_GPIO_LED_SYSTEM,
		.active_low	= 1,
	},
};

static struct gpio_keys_button EL_M150_gpio_keys[] __initdata = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = EL_M150_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= EL_M150_GPIO_BTN_RESET,
		.active_low	= 0,
	},
	{
		.desc		= "BTN_6",
		.type		= EV_KEY,
		.code		= BTN_6,
		.debounce_interval = EL_M150_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= EL_M150_GPIO_BTN6,
		.active_low	= 1,
	},
	{
		.desc		= "BTN_7",
		.type		= EV_KEY,
		.code		= BTN_7,
		.debounce_interval = EL_M150_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= EL_M150_GPIO_BTN7,
		.active_low	= 1,
	},
};

static void __init el_m150_setup(void)
{
	u8 *mac = (u8 *) KSEG1ADDR(0x1f01fc00);
	u8 *ee = (u8 *) KSEG1ADDR(0x1fff1000);

	/* disable PHY_SWAP and PHY_ADDR_SWAP bits */
	ath79_setup_ar933x_phy4_switch(false, false);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(EL_M150_leds_gpio),
				 EL_M150_leds_gpio);

	ath79_register_gpio_keys_polled(-1, EL_M150_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(EL_M150_gpio_keys),
					EL_M150_gpio_keys);

	gpio_request_one(EL_M150_GPIO_USB_POWER,
			 GPIOF_OUT_INIT_HIGH | GPIOF_EXPORT_DIR_FIXED,
			 "USB power");
	ath79_register_usb();

	ath79_register_m25p80(&EL_M150_flash_data);
	ath79_init_mac(ath79_eth0_data.mac_addr, mac, 1);
	ath79_init_mac(ath79_eth1_data.mac_addr, mac, -1);

	ath79_register_mdio(0, 0x0);
	ath79_register_eth(0);
	ath79_register_eth(1);

	ath79_register_wmac(ee, mac);
}

MIPS_MACHINE(ATH79_MACH_EL_M150, "EL-M150",
	     "EasyLink EL-M150", el_m150_setup);

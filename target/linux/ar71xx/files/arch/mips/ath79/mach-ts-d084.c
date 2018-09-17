/*
 *  PISEN TS-D084 board support
 *  Based on TP-LINK TL-WR703N/TL-MR10U board support
 *
 *  Copyright (C) 2011 dongyuqi <729650915@qq.com>
 *  Copyright (C) 2011-2012 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/gpio.h>

#include <asm/mach-ath79/ath79.h>

#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-usb.h"
#include "dev-wmac.h"
#include "machtypes.h"

#define TS_D084_GPIO_LED_SYSTEM	0
#define TS_D084_GPIO_BTN_RESET	12


#define TS_D084_KEYS_POLL_INTERVAL	20	/* msecs */
#define TS_D084_KEYS_DEBOUNCE_INTERVAL	(3 * TS_D084_KEYS_POLL_INTERVAL)

static const char *ts_d084_part_probes[] = {
	"tp-link",
	NULL,
};

static struct flash_platform_data ts_d084_flash_data = {
	.part_probes	= ts_d084_part_probes,
};

static struct gpio_led ts_d084_leds_gpio[] __initdata = {
	{
		.name		= "ts-d084:blue:system",
		.gpio		= TS_D084_GPIO_LED_SYSTEM,
		.active_low	= 0,
	},
};

static struct gpio_keys_button ts_d084_gpio_keys[] __initdata = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = TS_D084_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= TS_D084_GPIO_BTN_RESET,
		.active_low	= 0,
	}
};

static void __init ts_d084_setup(void)
{
	u8 *mac = (u8 *) KSEG1ADDR(0x1f01fc00);
	u8 *ee = (u8 *) KSEG1ADDR(0x1fff1000);

	/* disable PHY_SWAP and PHY_ADDR_SWAP bits */
	ath79_setup_ar933x_phy4_switch(false, false);

	ath79_register_m25p80(&ts_d084_flash_data);
	ath79_register_leds_gpio(-1, ARRAY_SIZE(ts_d084_leds_gpio),
				 ts_d084_leds_gpio);
	ath79_register_gpio_keys_polled(-1, TS_D084_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(ts_d084_gpio_keys),
					ts_d084_gpio_keys);

	ath79_register_usb();

	ath79_init_mac(ath79_eth0_data.mac_addr, mac, 0);

	ath79_register_mdio(0, 0x0);
	ath79_register_eth(0);

	ath79_register_wmac(ee, mac);
}

MIPS_MACHINE(ATH79_MACH_TS_D084, "TS-D084", "PISEN TS-D084",
	     ts_d084_setup);

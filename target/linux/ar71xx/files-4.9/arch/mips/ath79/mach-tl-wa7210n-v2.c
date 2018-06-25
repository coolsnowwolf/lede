/*
 *  TP-LINK TL-WA7210N v2.1 board support
 *
 *  Copyright (C) 2011 dongyuqi <729650915@qq.com>
 *  Copyright (C) 2011-2012 Gabor Juhos <juhosg@openwrt.org>
 *  Copyright (C) 2014 Nicolas Braud-Santoni <nicolas@braud-santoni.eu>
 *  Copyright (C) 2014 Alexander List <alex@graz.funkfeuer.at>
 *  Copyright (C) 2015 Hendrik Frenzel <hfrenzel@scunc.net>
 *
 *  rebased on TL-WA7510Nv1 support,
 *    Copyright (C) 2012 Stefan Helmert <helst_listen@aol.de>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>

#include <asm/mach-ath79/ar71xx_regs.h>
#include <asm/mach-ath79/ath79.h>

#include "dev-dsa.h"
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-wmac.h"
#include "machtypes.h"
#include "pci.h"

#include "common.h"

#define TL_WA7210N_V2_GPIO_BTN_RESET    11
#define TL_WA7210N_V2_KEYS_POLL_INT     20
#define TL_WA7210N_V2_KEYS_DEBOUNCE_INT (3 * TL_WA7210N_V2_KEYS_POLL_INT)

#define TL_WA7210N_V2_GPIO_LED_LAN  17
#define TL_WA7210N_V2_GPIO_LED_SIG1 0
#define TL_WA7210N_V2_GPIO_LED_SIG2 1
#define TL_WA7210N_V2_GPIO_LED_SIG3 27
#define TL_WA7210N_V2_GPIO_LED_SIG4 26

#define  TL_WA7210N_V2_GPIO_LNA_EN 28

static const char *tl_wa7210n_v2_part_probes[] = {
	"tp-link",
	NULL,
};

static struct gpio_keys_button tl_wa7210n_v2_gpio_keys[] __initdata = {
	{
		.desc              = "reset",
		.type              = EV_KEY,
		.code              = KEY_RESTART,
		.debounce_interval = TL_WA7210N_V2_KEYS_DEBOUNCE_INT,
		.gpio              = TL_WA7210N_V2_GPIO_BTN_RESET,
		.active_low        = 0,
	},
};

static struct gpio_led tl_wa7210n_v2_leds_gpio[] __initdata = {
	{
		.name       = "tp-link:green:lan",
		.gpio       = TL_WA7210N_V2_GPIO_LED_LAN,
		.active_low = 1,
	}, {
		.name       = "tp-link:green:signal1",
		.gpio       = TL_WA7210N_V2_GPIO_LED_SIG1,
		.active_low = 0,
	}, {
		.name       = "tp-link:green:signal2",
		.gpio       = TL_WA7210N_V2_GPIO_LED_SIG2,
		.active_low = 0,
	}, {
		.name       = "tp-link:green:signal3",
		.gpio       = TL_WA7210N_V2_GPIO_LED_SIG3,
		.active_low = 1,
	}, {
		.name       = "tp-link:green:signal4",
		.gpio       = TL_WA7210N_V2_GPIO_LED_SIG4,
		.active_low = 1,
	},
};

static struct flash_platform_data tl_wa7210n_v2_flash_data = {
	.part_probes    = tl_wa7210n_v2_part_probes,
};

static void __init tl_wa7210n_v2_setup(void)
{
	u8 *mac = (u8 *) KSEG1ADDR(0x1f01fc00);
	u8 *ee = (u8 *) KSEG1ADDR(0x1fff1000);

	ath79_register_gpio_keys_polled(-1, TL_WA7210N_V2_KEYS_POLL_INT,
			ARRAY_SIZE(tl_wa7210n_v2_gpio_keys),
			tl_wa7210n_v2_gpio_keys);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(tl_wa7210n_v2_leds_gpio),
			tl_wa7210n_v2_leds_gpio);

	ath79_gpio_function_enable(TL_WA7210N_V2_GPIO_LNA_EN);

	ath79_setup_ar933x_phy4_switch(false, false);

	ath79_init_mac(ath79_eth0_data.mac_addr, mac, -1);
	ath79_init_mac(ath79_eth1_data.mac_addr, mac, 1);

	ath79_register_eth(0);
	ath79_register_eth(1);

	ath79_register_mdio(0, 0x0);

	ath79_register_wmac(ee, mac);

	ath79_register_m25p80(&tl_wa7210n_v2_flash_data);

	ath79_register_pci();
}

MIPS_MACHINE(ATH79_MACH_TL_WA7210N_V2, "TL-WA7210N-v2", "TP-LINK TL-WA7210N v2",
	tl_wa7210n_v2_setup);

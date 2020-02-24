/*
 *  TP-LINK TL-WR885N TP9343 board support
 *
 *  Copyright (C) 2020 haxc
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
#include "machtypes.h"


#define TL_WR885N_V1_GPIO_LED_WAN		2
#define TL_WR885N_V1_GPIO_LED_LAN1		4
#define TL_WR885N_V1_GPIO_LED_LAN2		5
#define TL_WR885N_V1_GPIO_LED_LAN3		6
#define TL_WR885N_V1_GPIO_LED_LAN4		7
#define TL_WR885N_V1_GPIO_LED_SYSTEM		8

#define TL_WR885N_V1_GPIO_BTN_RESET		1

#define TL_WR885N_KEYS_POLL_INTERVAL		20
#define TL_WR885N_KEYS_DEBOUNCE_INTERVAL	(3 * TL_WR885N_KEYS_POLL_INTERVAL)


static struct gpio_led tl_wr885n_v1_leds_gpio[] __initdata = {

	{
		.name		= "tp-link:green:wan",
		.gpio		= TL_WR885N_V1_GPIO_LED_WAN,
		.active_low	= 1,
	},
	{
		.name		= "tp-link:green:lan1",
		.gpio		= TL_WR885N_V1_GPIO_LED_LAN1,
		.active_low	= 1,
	},
	{
		.name		= "tp-link:green:lan2",
		.gpio		= TL_WR885N_V1_GPIO_LED_LAN2,
		.active_low	= 1,
	},
	{
		.name		= "tp-link:green:lan3",
		.gpio		= TL_WR885N_V1_GPIO_LED_LAN3,
		.active_low	= 1,
	},
	{
		.name		= "tp-link:green:lan4",
		.gpio		= TL_WR885N_V1_GPIO_LED_LAN4,
		.active_low	= 1,
	},
	{
		.name		= "tp-link:green:system",
		.gpio		= TL_WR885N_V1_GPIO_LED_SYSTEM,
		.active_low	= 1,
	},
};

static struct gpio_keys_button tl_wr885n_v1_gpio_keys[] __initdata = {
	{
		.desc		= "Reset button",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = TL_WR885N_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= TL_WR885N_V1_GPIO_BTN_RESET,
		.active_low	= 1,
	}
};

static const char *tl_wr885n_part_probes[] = {
	"tp-link",
	NULL,
};

static struct flash_platform_data tl_wr885n_flash_data = {
	.part_probes	= tl_wr885n_part_probes,
};


static void __init tl_wr885n_setup(void)
{
	u8 *mac = (u8 *) KSEG1ADDR(0x1f01fc00);
	u8 *ee = (u8 *) KSEG1ADDR(0x1fff1000);

	ath79_register_m25p80(&tl_wr885n_flash_data);

	ath79_register_mdio(0, 0x0);

	ath79_init_mac(ath79_eth0_data.mac_addr, mac, 1);
	ath79_init_mac(ath79_eth1_data.mac_addr, mac, -1);

	ath79_switch_data.phy4_mii_en = 1;

	ath79_register_eth(0);
	ath79_register_eth(1);

	ath79_register_wmac(ee, mac);

}

static void __init tl_wr885n_v1_setup(void)
{
	tl_wr885n_setup();

	ath79_register_leds_gpio(-1, ARRAY_SIZE(tl_wr885n_v1_leds_gpio),
				 tl_wr885n_v1_leds_gpio);

	ath79_register_gpio_keys_polled(-1, TL_WR885N_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(tl_wr885n_v1_gpio_keys),
					tl_wr885n_v1_gpio_keys);
}

MIPS_MACHINE(ATH79_MACH_TL_WR885N_V1, "TL-WR885N-v1", "TP-LINK TL-WR885N v1",
	     tl_wr885n_v1_setup);

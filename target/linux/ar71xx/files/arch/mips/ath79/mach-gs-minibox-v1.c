/*
 *  Gainstrong MiniBox V1.0 board support
 *
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
#include "dev-usb.h"
#include "dev-wmac.h"
#include "machtypes.h"

#define GS_MINIBOX_V1_GPIO_BTN_RESET	11

#define GS_MINIBOX_V1_GPIO_LED_SYSTEM	1

#define GS_MINIBOX_V1_KEYS_POLL_INTERVAL	20	/* msecs */
#define GS_MINIBOX_V1_KEYS_DEBOUNCE_INTERVAL (3 * GS_MINIBOX_V1_KEYS_POLL_INTERVAL)

static const char *gs_minibox_v1_part_probes[] = {
	"tp-link",
	NULL,
};

static struct flash_platform_data gs_minibox_v1_flash_data = {
	.part_probes	= gs_minibox_v1_part_probes,
};

static struct gpio_led gs_minibox_v1_leds_gpio[] __initdata = {
	{
		.name		= "minibox-v1:green:system",
		.gpio		= GS_MINIBOX_V1_GPIO_LED_SYSTEM,
		.active_low	= 1,
	},
};

static struct gpio_keys_button gs_minibox_v1_gpio_keys[] __initdata = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = GS_MINIBOX_V1_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= GS_MINIBOX_V1_GPIO_BTN_RESET,
		.active_low	= 0,
	},
};

static void __init gs_minibox_v1_setup(void)
{
	u8 *mac = (u8 *) KSEG1ADDR(0x1f01fc00);
	u8 *ee = (u8 *) KSEG1ADDR(0x1fff1000);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(gs_minibox_v1_leds_gpio),
				 gs_minibox_v1_leds_gpio);

	ath79_register_gpio_keys_polled(-1, GS_MINIBOX_V1_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(gs_minibox_v1_gpio_keys),
					gs_minibox_v1_gpio_keys);

	ath79_register_usb();

	ath79_register_m25p80(&gs_minibox_v1_flash_data);
	ath79_init_mac(ath79_eth0_data.mac_addr, mac, 1);
	ath79_init_mac(ath79_eth1_data.mac_addr, mac, -1);

	ath79_register_mdio(0, 0x0);
	ath79_register_eth(1);
	ath79_register_eth(0);

	ath79_register_wmac(ee, mac);
}

MIPS_MACHINE(ATH79_MACH_GS_MINIBOX_V1, "MINIBOX-V1",
	     "MiniBox V1.0", gs_minibox_v1_setup);

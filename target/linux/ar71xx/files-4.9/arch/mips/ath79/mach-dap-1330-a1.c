/*
 * D-Link DAP-1330
 *
 * Copyright (c) 2013-2015 The Linux Foundation. All rights reserved.
 * Copyright (c) 2017 Nicolò Veronese <nicveronese@gmail.com>
 * Copyright (c) 2017 Federico Cappon <dududede371@gmail.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
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
#include "nvram.h"

#include <linux/kernel.h>
#include <linux/vmalloc.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/string.h>

#define DAP_1330_GPIO_LED_GREEN_POWER	13
#define DAP_1330_GPIO_LED_RED_POWER	3
#define DAP_1330_GPIO_LED_GREEN_WIFI	14
#define DAP_1330_GPIO_LED_RED_WIFI	11
#define DAP_1330_GPIO_LED_SIGNAL1	15
#define DAP_1330_GPIO_LED_SIGNAL2	16

#define DAP_1330_GPIO_BTN_WPS		2
#define DAP_1330_GPIO_BTN_RESET		17

#define DAP_1330_KEYS_POLL_INTERVAL	20	/* msecs */
#define DAP_1330_KEYS_DEBOUNCE_INTERVAL	(3 * DAP_1330_KEYS_POLL_INTERVAL)

#define DAP1330_MAC_ADDR		0x1f020001

#define DAP1330_WMAC_CALDATA_ADDR	0x1f010000
#define DAP_1330_WMAC_CALDATA_OFFSET	0x1000

static struct gpio_led dap_1330_leds_gpio[] __initdata = {
		{
				.name		= "d-link:green:power",
				.gpio		= DAP_1330_GPIO_LED_GREEN_POWER,
				.active_low	= 1,
		},
		{
				.name		= "d-link:red:power",
				.gpio		= DAP_1330_GPIO_LED_RED_POWER,
				.active_low	= 1,
		},
		{
				.name		= "d-link:green:wifi",
				.gpio		= DAP_1330_GPIO_LED_GREEN_WIFI,
				.active_low	= 1,
		},
		{
				.name		= "d-link:red:wifi",
				.gpio		= DAP_1330_GPIO_LED_RED_WIFI,
				.active_low	= 1,
		},
		{
				.name		= "d-link:green:signal1",
				.gpio		= DAP_1330_GPIO_LED_SIGNAL1,
				.active_low	= 1,
		},
		{
				.name		= "d-link:green:signal2",
				.gpio		= DAP_1330_GPIO_LED_SIGNAL2,
				.active_low	= 1,
		}
};

static struct gpio_keys_button dap_1330_gpio_keys[] __initdata = {
	{
		.desc		= "WPS button",
		.type		= EV_KEY,
		.code		= KEY_WPS_BUTTON,
		.debounce_interval = DAP_1330_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= DAP_1330_GPIO_BTN_WPS,
		.active_low	= 1,
	},
	{
		.desc		= "Reset button",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = DAP_1330_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= DAP_1330_GPIO_BTN_RESET,
		.active_low	= 1,
	}
};

static void __init dap_1330_setup(void)
{
	u8 *art = (u8 *) KSEG1ADDR(DAP1330_WMAC_CALDATA_ADDR);
	u8 *mac_ptr = (u8 *) KSEG1ADDR(DAP1330_MAC_ADDR);
	u8 mac[ETH_ALEN];

	ath79_parse_ascii_mac((char *) mac_ptr, mac);

	ath79_register_m25p80(NULL);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(dap_1330_leds_gpio),
		dap_1330_leds_gpio);

	ath79_register_gpio_keys_polled(-1, DAP_1330_KEYS_POLL_INTERVAL,
			ARRAY_SIZE(dap_1330_gpio_keys),
			dap_1330_gpio_keys);

	ath79_register_wmac(art + DAP_1330_WMAC_CALDATA_OFFSET, mac);

	ath79_register_mdio(0, 0x0);

	ath79_init_mac(ath79_eth1_data.mac_addr, mac, 0);

	/* LAN ports */
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_GMII;
	ath79_eth1_data.speed = SPEED_1000;
	ath79_eth1_data.duplex = DUPLEX_FULL;
	ath79_switch_data.phy_poll_mask |= BIT(4);
	ath79_switch_data.phy4_mii_en = 1;
	ath79_register_eth(1);
}

MIPS_MACHINE(ATH79_MACH_DAP_1330_A1, "DAP-1330-A1",
             "D-Link DAP-1330 Rev. A1", dap_1330_setup);

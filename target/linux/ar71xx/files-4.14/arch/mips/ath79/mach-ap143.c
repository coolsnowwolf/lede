/*
 * Atheros AP143 reference board support
 *
 * Copyright (c) 2013-2015 The Linux Foundation. All rights reserved.
 * Copyright (c) 2012 Gabor Juhos <juhosg@openwrt.org>
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

#include <linux/platform_device.h>
#include <linux/ath9k_platform.h>
#include <linux/ar8216_platform.h>

#include <asm/mach-ath79/ar71xx_regs.h>

#include "common.h"
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-spi.h"
#include "dev-usb.h"
#include "dev-wmac.h"
#include "machtypes.h"

#define AP143_GPIO_LED_WLAN		12
#define AP143_GPIO_LED_WPS		13
#define AP143_GPIO_LED_STATUS		13

#define AP143_GPIO_LED_WAN		4
#define AP143_GPIO_LED_LAN1		16
#define AP143_GPIO_LED_LAN2		15
#define AP143_GPIO_LED_LAN3		14
#define AP143_GPIO_LED_LAN4		11

#define AP143_GPIO_BTN_WPS		17

#define AP143_KEYS_POLL_INTERVAL	20	/* msecs */
#define AP143_KEYS_DEBOUNCE_INTERVAL	(3 * AP143_KEYS_POLL_INTERVAL)

#define AP143_MAC0_OFFSET		0
#define AP143_MAC1_OFFSET		6
#define AP143_WMAC_CALDATA_OFFSET	0x1000

static struct gpio_led ap143_leds_gpio[] __initdata = {
	{
		.name		= "ap143:green:status",
		.gpio		= AP143_GPIO_LED_STATUS,
		.active_low	= 1,
	},
	{
		.name		= "ap143:green:wlan",
		.gpio		= AP143_GPIO_LED_WLAN,
		.active_low	= 1,
	}
};

static struct gpio_keys_button ap143_gpio_keys[] __initdata = {
	{
		.desc		= "WPS button",
		.type		= EV_KEY,
		.code		= KEY_WPS_BUTTON,
		.debounce_interval = AP143_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= AP143_GPIO_BTN_WPS,
		.active_low	= 1,
	},
};

static void __init ap143_gpio_led_setup(void)
{
	ath79_gpio_direction_select(AP143_GPIO_LED_WAN, true);
	ath79_gpio_direction_select(AP143_GPIO_LED_LAN1, true);
	ath79_gpio_direction_select(AP143_GPIO_LED_LAN2, true);
	ath79_gpio_direction_select(AP143_GPIO_LED_LAN3, true);
	ath79_gpio_direction_select(AP143_GPIO_LED_LAN4, true);

	ath79_gpio_output_select(AP143_GPIO_LED_WAN,
			QCA953X_GPIO_OUT_MUX_LED_LINK5);
	ath79_gpio_output_select(AP143_GPIO_LED_LAN1,
			QCA953X_GPIO_OUT_MUX_LED_LINK1);
	ath79_gpio_output_select(AP143_GPIO_LED_LAN2,
			QCA953X_GPIO_OUT_MUX_LED_LINK2);
	ath79_gpio_output_select(AP143_GPIO_LED_LAN3,
			QCA953X_GPIO_OUT_MUX_LED_LINK3);
	ath79_gpio_output_select(AP143_GPIO_LED_LAN4,
			QCA953X_GPIO_OUT_MUX_LED_LINK4);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(ap143_leds_gpio),
			ap143_leds_gpio);
	ath79_register_gpio_keys_polled(-1, AP143_KEYS_POLL_INTERVAL,
			ARRAY_SIZE(ap143_gpio_keys),
			ap143_gpio_keys);
}

static void __init ap143_setup(void)
{
	u8 *art = (u8 *) KSEG1ADDR(0x1fff0000);

	ath79_register_m25p80(NULL);

	ap143_gpio_led_setup();

	ath79_register_usb();

	ath79_wmac_set_led_pin(AP143_GPIO_LED_WLAN);
	ath79_register_wmac(art + AP143_WMAC_CALDATA_OFFSET, NULL);

	ath79_register_mdio(0, 0x0);
	ath79_register_mdio(1, 0x0);

	ath79_init_mac(ath79_eth0_data.mac_addr, art + AP143_MAC0_OFFSET, 0);
	ath79_init_mac(ath79_eth1_data.mac_addr, art + AP143_MAC1_OFFSET, 0);

	/* WAN port */
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ath79_eth0_data.speed = SPEED_100;
	ath79_eth0_data.duplex = DUPLEX_FULL;
	ath79_eth0_data.phy_mask = BIT(4);
	ath79_register_eth(0);

	/* LAN ports */
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_GMII;
	ath79_eth1_data.speed = SPEED_1000;
	ath79_eth1_data.duplex = DUPLEX_FULL;
	ath79_switch_data.phy_poll_mask |= BIT(4);
	ath79_switch_data.phy4_mii_en = 1;
	ath79_register_eth(1);
}

MIPS_MACHINE(ATH79_MACH_AP143, "AP143", "Qualcomm Atheros AP143 reference board",
	     ap143_setup);

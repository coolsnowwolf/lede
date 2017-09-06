/*
 * TP-LINK TL-WR810N board support
 *
 * Copyright (c) 2012 Qualcomm Atheros
 * Copyright (c) 2012 Gabor Juhos <juhosg@openwrt.org>
 * Copyright (c) 2016 Jens Steinhauser <jens.steinhauser@gmail.com>
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
 */

#include <linux/gpio.h>
#include <linux/platform_device.h>
#include <linux/ar8216_platform.h>

#include <asm/mach-ath79/ar71xx_regs.h>

#include "common.h"
#include "dev-gpio-buttons.h"
#include "dev-eth.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-usb.h"
#include "dev-wmac.h"
#include "machtypes.h"

#define TL_WR810N_GPIO_SWITCH_B1	0
#define TL_WR810N_GPIO_SWITCH_B0	1
#define TL_WR810N_GPIO_USB_POWER	11
#define TL_WR810N_GPIO_BTN_RESET	12
#define TL_WR810N_GPIO_LED_SYSTEM	13

#define TL_WR810N_KEYS_POLL_INTERVAL	20 /* msecs */
#define TL_WR810N_KEYS_DEBOUNCE_INTERVAL (3 * TL_WR810N_KEYS_POLL_INTERVAL)

#define TL_WR810N_WMAC_CALDATA_OFFSET	0x1000

static const char *tl_wr810n_part_probes[] = {
	"tp-link",
	NULL,
};

static struct flash_platform_data tl_wr810n_flash_data = {
	.part_probes = tl_wr810n_part_probes,
};

static struct gpio_led tl_wr810n_leds_gpio[] __initdata = {
	{
		.name		= "tp-link:blue:system",
		.gpio		= TL_WR810N_GPIO_LED_SYSTEM,
		.active_low	= 1,
	},
};

static struct gpio_keys_button tl_wr810n_gpio_keys[] __initdata = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = TL_WR810N_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= TL_WR810N_GPIO_BTN_RESET,
		.active_low	= 1,
	},
	{
		.desc		= "switch_b0",
		.type		= EV_SW,
		.code		= BTN_0,
		.debounce_interval = TL_WR810N_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= TL_WR810N_GPIO_SWITCH_B0,
		.active_low	= 0,
	},
	{
		.desc		= "switch_b1",
		.type		= EV_SW,
		.code		= BTN_1,
		.debounce_interval = TL_WR810N_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= TL_WR810N_GPIO_SWITCH_B1,
		.active_low	= 0,
	},
};

static void __init tl_wr810n_setup(void)
{
	u8 *mac = (u8 *) KSEG1ADDR(0x1f01fc00);
	u8 *art = (u8 *) KSEG1ADDR(0x1fff0000);

	ath79_setup_ar933x_phy4_switch(false, false);

	ath79_register_m25p80(&tl_wr810n_flash_data);
	ath79_register_leds_gpio(-1,
				 ARRAY_SIZE(tl_wr810n_leds_gpio),
				 tl_wr810n_leds_gpio);
	ath79_register_gpio_keys_polled(-1,
				        TL_WR810N_KEYS_POLL_INTERVAL,
				        ARRAY_SIZE(tl_wr810n_gpio_keys),
				        tl_wr810n_gpio_keys);

	ath79_register_mdio(0, 0x0);

	/* WAN */
	ath79_eth0_data.duplex = DUPLEX_FULL;
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ath79_eth0_data.speed = SPEED_100;
	ath79_eth0_data.phy_mask = BIT(4);
	ath79_init_mac(ath79_eth0_data.mac_addr, mac, 1);
	ath79_register_eth(0);

	/* LAN */
	ath79_switch_data.phy4_mii_en = 1;
	ath79_eth1_data.duplex = DUPLEX_FULL;
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_GMII;
	ath79_eth1_data.speed = SPEED_1000;
	ath79_switch_data.phy_poll_mask |= BIT(4);
	ath79_init_mac(ath79_eth1_data.mac_addr, mac, -1);
	ath79_register_eth(1);

	ath79_register_wmac(art + TL_WR810N_WMAC_CALDATA_OFFSET, mac);

	gpio_request_one(TL_WR810N_GPIO_USB_POWER,
			 GPIOF_OUT_INIT_HIGH | GPIOF_EXPORT_DIR_FIXED,
			 "USB power");
	ath79_register_usb();
}

MIPS_MACHINE(ATH79_MACH_TL_WR810N, "TL-WR810N", "TP-LINK TL-WR810N",
	     tl_wr810n_setup);

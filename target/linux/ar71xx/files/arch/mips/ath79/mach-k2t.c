
/*
 * Phicomm K2T A1/A2/A3 reference board support
 *
 * Copyright (c) 2018 Chen Minqiang <ptpt52@gmail.com>
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
#include "dev-m25p80.h"
#include "machtypes.h"
#include "pci.h"
#include "dev-eth.h"
#include "dev-ap9x-pci.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-spi.h"
#include "dev-wmac.h"

#define K2T_GPIO_LED_BLUE		6
#define K2T_GPIO_LED_RED		7
#define K2T_GPIO_LED_YELLOW		8

#define K2T_GPIO_BTN_RESET            2
#define K2T_KEYS_POLL_INTERVAL        20     /* msecs */
#define K2T_KEYS_DEBOUNCE_INTERVAL    (3 * K2T_KEYS_POLL_INTERVAL)

#define K2T_MAC0_OFFSET               0

static struct gpio_led k2t_leds_gpio[] __initdata = {
	{
		.name		= "k2t:blue:lan",
		.gpio		= K2T_GPIO_LED_BLUE,
		.active_low	= 1,
	},
	{
		.name		= "k2t:red:lan",
		.gpio		= K2T_GPIO_LED_RED,
		.active_low	= 0,
	},
	{
		.name		= "k2t:yellow:lan",
		.gpio		= K2T_GPIO_LED_YELLOW,
		.active_low	= 1,
	},
};

static struct gpio_keys_button k2t_gpio_keys[] __initdata = {
	{
		.desc		= "Reset button",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = K2T_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= K2T_GPIO_BTN_RESET,
		.active_low	= 1,
	},
};

static struct ar8327_pad_cfg k2t_ar8337_pad0_cfg = {
	.mode = AR8327_PAD_MAC_SGMII,
	.sgmii_delay_en = true,
};

static struct ar8327_platform_data k2t_ar8337_data = {
	.pad0_cfg = &k2t_ar8337_pad0_cfg,
	.port0_cfg = {
		.force_link = 1,
		.speed = AR8327_PORT_SPEED_1000,
		.duplex = 1,
		.txpause = 1,
		.rxpause = 1,
	},
};

static struct mdio_board_info k2t_mdio0_info[] = {
	{
		.bus_id = "ag71xx-mdio.0",
		.mdio_addr = 0,
		.platform_data = &k2t_ar8337_data,
	},
};

static void __init k2t_setup(void)
{
	u8 *art = (u8 *) KSEG1ADDR(0x1fff0000);

	ath79_register_m25p80(NULL);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(k2t_leds_gpio),
				 k2t_leds_gpio);
	ath79_register_gpio_keys_polled(-1, K2T_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(k2t_gpio_keys),
					k2t_gpio_keys);

	platform_device_register(&ath79_mdio0_device);

	mdiobus_register_board_info(k2t_mdio0_info,
				    ARRAY_SIZE(k2t_mdio0_info));

	ath79_init_mac(ath79_eth0_data.mac_addr, art + K2T_MAC0_OFFSET, 0);

	/* GMAC0 is connected to an AR8337 switch */
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_SGMII;
	ath79_eth0_data.speed = SPEED_1000;
	ath79_eth0_data.duplex = DUPLEX_FULL;
	ath79_eth0_data.phy_mask = BIT(0);
	ath79_eth0_data.mii_bus_dev = &ath79_mdio0_device.dev;

	ath79_register_eth(0);

	ath79_register_wmac_simple();
	ath79_register_pci();
}

MIPS_MACHINE(ATH79_MACH_K2T, "K2T", "Phicomm K2T A1/A2/A3 board",
	     k2t_setup);

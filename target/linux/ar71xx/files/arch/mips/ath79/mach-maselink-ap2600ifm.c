/*
 * Comba MASELink AP2600-IFM board support
 *
 * Copyright (c) 2017 Weijie Gao <hackpascal@gmail.com>
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

#include <linux/pci.h>
#include <linux/phy.h>
#include <linux/platform_device.h>

#include <asm/mach-ath79/ath79.h>
#include <asm/mach-ath79/ar71xx_regs.h>

#include "pci.h"
#include "common.h"
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-spi.h"
#include "dev-usb.h"
#include "machtypes.h"
#include "nvram.h"

#define AP2600IFM_GPIO_LED_RF1		2
#define AP2600IFM_GPIO_LED_RF1_TOP	7
#define AP2600IFM_GPIO_LED_RF2		4
#define AP2600IFM_GPIO_LED_RF2_TOP	5
#define AP2600IFM_GPIO_LED_D24		0
#define AP2600IFM_GPIO_LED_D24_TOP	3

#define AP2600IFM_GPIO_BUTTON_RESET	8

#define AP2600IFM_KEYS_POLL_INTERVAL	20	/* msecs */
#define AP2600IFM_KEYS_DEBOUNCE_INTERVAL	(3 * AP2600IFM_KEYS_POLL_INTERVAL)

#define AP2600IFM_NVRAM_ADDR			0x1f040004
#define AP2600IFM_NVRAM_SIZE			0x3fffc

static struct gpio_led ap2600ifm_leds_gpio[] __initdata = {
	{
		.name		= "ap2600ifm:green:rf1",
		.gpio		= AP2600IFM_GPIO_LED_RF1,
		.active_low	= 1,
	},
	{
		.name		= "ap2600ifm:green:rf1top",
		.gpio		= AP2600IFM_GPIO_LED_RF1_TOP,
		.active_low	= 1,
	},
	{
		.name		= "ap2600ifm:green:rf2",
		.gpio		= AP2600IFM_GPIO_LED_RF2,
		.active_low	= 1,
	},
	{
		.name		= "ap2600ifm:green:rf2top",
		.gpio		= AP2600IFM_GPIO_LED_RF2_TOP,
		.active_low	= 1,
	},
	{
		.name		= "ap2600ifm:green:d24",
		.gpio		= AP2600IFM_GPIO_LED_D24,
		.active_low	= 1,
	},
	{
		.name		= "ap2600ifm:green:d24top",
		.gpio		= AP2600IFM_GPIO_LED_D24_TOP,
		.active_low	= 1,
	}
};

static struct gpio_keys_button ap2600ifm_gpio_keys[] __initdata = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = AP2600IFM_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= AP2600IFM_GPIO_BUTTON_RESET,
		.active_low	= 1,
	},
};

static struct mdio_board_info ap2600ifm_mdio0_info[] = {
	{
		.bus_id = "ag71xx-mdio.0",
		.phy_addr = 20,
	},
};

static void ap2600ifm_get_mac(const char *name, char *mac)
{
	u8 *nvram = (u8 *) KSEG1ADDR(AP2600IFM_NVRAM_ADDR);
	int err;

	err = ath79_nvram_parse_mac_addr(nvram, AP2600IFM_NVRAM_SIZE,
					 name, mac);
	if (err)
		pr_err("%s not found in environment variables\n", name);
}

static void __init ap2600ifm_setup(void)
{
	ath79_register_m25p80(NULL);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(ap2600ifm_leds_gpio),
				 ap2600ifm_leds_gpio);
	ath79_register_gpio_keys_polled(-1, AP2600IFM_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(ap2600ifm_gpio_keys),
					ap2600ifm_gpio_keys);

	ath79_register_pci();

	ath79_register_mdio(0, 0);

	mdiobus_register_board_info(ap2600ifm_mdio0_info,
				    ARRAY_SIZE(ap2600ifm_mdio0_info));

	ap2600ifm_get_mac("macaddr=", ath79_eth0_data.mac_addr);

	ath79_eth0_data.mii_bus_dev = &ath79_mdio0_device.dev;
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth0_data.phy_mask = BIT(20);
	ath79_eth0_data.speed = SPEED_1000;
	ath79_eth0_data.duplex = DUPLEX_FULL;
	ath79_eth0_pll_data.pll_1000 = 0x00110000;
	ath79_eth0_pll_data.pll_100 = 0x0001099;
	ath79_eth0_pll_data.pll_10 = 0x00991099;

	ath79_register_eth(0);
}

MIPS_MACHINE(ATH79_MACH_MASELINK_AP2600IFM, "AP2600IFM", "Comba MASELink AP2600-IFM",
	     ap2600ifm_setup);

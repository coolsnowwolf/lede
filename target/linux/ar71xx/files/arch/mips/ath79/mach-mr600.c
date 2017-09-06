/*
 * OpenMesh OM2P board support
 *
 * Copyright (C) 2012 Marek Lindner <marek@open-mesh.com>
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
#include <linux/ath9k_platform.h>

#include <asm/mach-ath79/ar71xx_regs.h>

#include "common.h"
#include "dev-ap9x-pci.h"
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-spi.h"
#include "dev-wmac.h"
#include "machtypes.h"

#define MR600_GPIO_LED_WLAN58		12
#define MR600_GPIO_LED_WPS			13
#define MR600_GPIO_LED_POWER			14

#define MR600V2_GPIO_LED_WLAN58_RED		12
#define MR600V2_GPIO_LED_WPS			13
#define MR600V2_GPIO_LED_POWER			14
#define MR600V2_GPIO_LED_WLAN24_GREEN		18
#define MR600V2_GPIO_LED_WLAN24_YELLOW		19
#define MR600V2_GPIO_LED_WLAN24_RED		20
#define MR600V2_GPIO_LED_WLAN58_GREEN		21
#define MR600V2_GPIO_LED_WLAN58_YELLOW		22

#define MR600_GPIO_BTN_RESET		17

#define MR600_KEYS_POLL_INTERVAL	20	/* msecs */
#define MR600_KEYS_DEBOUNCE_INTERVAL (3 * MR600_KEYS_POLL_INTERVAL)

#define MR600_MAC_OFFSET		0
#define MR600_WMAC_CALDATA_OFFSET	0x1000
#define MR600_PCIE_CALDATA_OFFSET	0x5000

static struct gpio_led mr600_leds_gpio[] __initdata = {
	{
		.name		= "mr600:orange:power",
		.gpio		= MR600_GPIO_LED_POWER,
		.active_low	= 1,
	},
	{
		.name		= "mr600:blue:wps",
		.gpio		= MR600_GPIO_LED_WPS,
		.active_low	= 1,
	},
	{
		.name		= "mr600:green:wlan58",
		.gpio		= MR600_GPIO_LED_WLAN58,
		.active_low	= 1,
	},
};

static struct gpio_led mr600v2_leds_gpio[] __initdata = {
	{
		.name		= "mr600:blue:power",
		.gpio		= MR600V2_GPIO_LED_POWER,
		.active_low	= 1,
	},
	{
		.name		= "mr600:blue:wps",
		.gpio		= MR600V2_GPIO_LED_WPS,
		.active_low	= 1,
	},
	{
		.name		= "mr600:red:wlan24",
		.gpio		= MR600V2_GPIO_LED_WLAN24_RED,
		.active_low	= 1,
	},
	{
		.name		= "mr600:yellow:wlan24",
		.gpio		= MR600V2_GPIO_LED_WLAN24_YELLOW,
		.active_low	= 1,
	},
	{
		.name		= "mr600:green:wlan24",
		.gpio		= MR600V2_GPIO_LED_WLAN24_GREEN,
		.active_low	= 1,
	},
	{
		.name		= "mr600:red:wlan58",
		.gpio		= MR600V2_GPIO_LED_WLAN58_RED,
		.active_low	= 1,
	},
	{
		.name		= "mr600:yellow:wlan58",
		.gpio		= MR600V2_GPIO_LED_WLAN58_YELLOW,
		.active_low	= 1,
	},
	{
		.name		= "mr600:green:wlan58",
		.gpio		= MR600V2_GPIO_LED_WLAN58_GREEN,
		.active_low	= 1,
	},
};

static struct gpio_keys_button mr600_gpio_keys[] __initdata = {
	{
		.desc		= "Reset button",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = MR600_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= MR600_GPIO_BTN_RESET,
		.active_low	= 1,
	},
};

static void __init mr600_base_setup(unsigned num_leds, struct gpio_led *leds)
{
	u8 *art = (u8 *)KSEG1ADDR(0x1fff0000);
	u8 mac[6];

	ath79_register_m25p80(NULL);

	ath79_register_leds_gpio(-1, num_leds, leds);
	ath79_register_gpio_keys_polled(-1, MR600_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(mr600_gpio_keys),
					mr600_gpio_keys);

	ath79_init_mac(mac, art + MR600_MAC_OFFSET, 1);
	ath79_register_wmac(art + MR600_WMAC_CALDATA_OFFSET, mac);

	ath79_init_mac(mac, art + MR600_MAC_OFFSET, 8);
	ap91_pci_init(art + MR600_PCIE_CALDATA_OFFSET, mac);

	ath79_setup_ar934x_eth_cfg(AR934X_ETH_CFG_RGMII_GMAC0 |
				   AR934X_ETH_CFG_SW_ONLY_MODE);

	ath79_register_mdio(0, 0x0);

	ath79_init_mac(ath79_eth0_data.mac_addr, art + MR600_MAC_OFFSET, 0);

	/* GMAC0 is connected to an external PHY */
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth0_data.phy_mask = BIT(0);
	ath79_eth0_data.mii_bus_dev = &ath79_mdio0_device.dev;
	ath79_eth0_pll_data.pll_1000 = 0x06000000;
	ath79_register_eth(0);
}

static void __init mr600_setup(void)
{
	mr600_base_setup(ARRAY_SIZE(mr600_leds_gpio), mr600_leds_gpio);
	ap9x_pci_setup_wmac_led_pin(0, 0);
}

MIPS_MACHINE(ATH79_MACH_MR600, "MR600", "OpenMesh MR600", mr600_setup);

static void __init mr600v2_setup(void)
{
	mr600_base_setup(ARRAY_SIZE(mr600v2_leds_gpio), mr600v2_leds_gpio);
}

MIPS_MACHINE(ATH79_MACH_MR600V2, "MR600v2", "OpenMesh MR600v2", mr600v2_setup);

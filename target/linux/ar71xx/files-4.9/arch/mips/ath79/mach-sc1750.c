/*
 * Abicom International Scorpion SC1750 support.
 *
 * Copyright (c) 2012 Qualcomm Atheros
 * Copyright (c) 2012-2013 Gabor Juhos <juhosg@openwrt.org>
 * Copyright (c) 2017 Conor O'Gorman <i@conorogorman.net>
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
#include <linux/platform_data/phy-at803x.h>

#include <asm/mach-ath79/ar71xx_regs.h>

#include "common.h"
#include "pci.h"
#include "dev-ap9x-pci.h"
#include "dev-gpio-buttons.h"
#include "dev-eth.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-nfc.h"
#include "dev-usb.h"
#include "dev-wmac.h"
#include "machtypes.h"

#define SC1750_GPIO_LED_POWER		11
#define SC1750_GPIO_LED_ERR1		12
#define SC1750_GPIO_LED_WLAN		13
#define SC1750_GPIO_LED_ERR2		14
#define SC1750_GPIO_LED_OK3		15
#define SC1750_GPIO_LED_ERR3		16
#define SC1750_GPIO_BTN_RESET		4

#define SC1750_KEYS_POLL_INTERVAL	20	/* msecs */
#define SC1750_KEYS_DEBOUNCE_INTERVAL	(3 * SC1750_KEYS_POLL_INTERVAL)

#define SC1750_MAC0_OFFSET		0
#define SC1750_MAC1_OFFSET		6
#define SC1750_WMAC_CALDATA_OFFSET	0x1000
#define SC1750_PCIE_CALDATA_OFFSET	0x5000

static struct gpio_led sc1750_leds_gpio[] __initdata = {
	{
		.name		= "sc1750:green:power",
		.gpio		= SC1750_GPIO_LED_POWER,
		.active_low	= 1,
	},
	{
		.name		= "sc1750:red:power",
		.gpio		= SC1750_GPIO_LED_ERR1,
		.active_low	= 1,
	},
	{
		.name		= "sc1750:green:wlan",
		.gpio		= SC1750_GPIO_LED_WLAN,
		.active_low	= 1,
	},
	{
		.name		= "sc1750:red:wlan",
		.gpio		= SC1750_GPIO_LED_ERR2,
		.active_low	= 1,
	}
};

static struct gpio_keys_button sc1750_gpio_keys[] __initdata = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = SC1750_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= SC1750_GPIO_BTN_RESET,
		.active_low	= 0,
	}
};

static struct at803x_platform_data at803x_data = {
	.disable_smarteee = 1,
	.enable_rgmii_rx_delay = 0,
	.enable_rgmii_tx_delay = 0,
};

static struct mdio_board_info sc1750_mdio0_info[] = {
	{
		.bus_id = "ag71xx-mdio.0",
		.phy_addr = 1,
		.platform_data = &at803x_data,
	},
};

static void __init sc1750_setup(void)
{
	u8 *art = (u8 *) KSEG1ADDR(0x1fff0000);

	ath79_register_m25p80(NULL);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(sc1750_leds_gpio),
				 sc1750_leds_gpio);
	ath79_register_gpio_keys_polled(-1, SC1750_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(sc1750_gpio_keys),
					sc1750_gpio_keys);

	ath79_register_usb();
	ath79_register_nfc();

	ath79_register_wmac(art + SC1750_WMAC_CALDATA_OFFSET, NULL);

	ath79_register_mdio(0, 0);
	mdiobus_register_board_info(sc1750_mdio0_info,
				    ARRAY_SIZE(sc1750_mdio0_info));

	ath79_init_mac(ath79_eth0_data.mac_addr, art + SC1750_MAC0_OFFSET, 0);

	ath79_eth0_pll_data.pll_1000 = 0xa6000101;
	ath79_eth0_pll_data.pll_100 = 0xa4000101;
	/* GMAC0 is connected to the RMGII interface */
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth0_data.phy_mask = 0xF;
	ath79_eth0_data.mii_bus_dev = &ath79_mdio0_device.dev;

	ath79_register_eth(0);

	/* GMAC1 is connected to the SGMII interface */
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_SGMII;
	ath79_eth1_data.speed = SPEED_1000;
	ath79_eth1_data.duplex = DUPLEX_FULL;

	ath79_register_eth(1);

	ath79_register_pci();
}

MIPS_MACHINE(ATH79_MACH_SC1750, "SC1750", "Abicom SC1750", sc1750_setup);

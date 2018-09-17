/*
 * Wallys DR342/DR344 boards support
 *
 * Copyright (c) 2011 Qualcomm Atheros
 * Copyright (c) 2011-2012 Gabor Juhos <juhosg@openwrt.org>
 * Copyright (c) 2015 Philippe Duchein <wireless-dev@duchein.net>
 * Copyright (c) 2017 Piotr Dymacz <pepe2k@gmail.com>
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
#include <linux/phy.h>
#include <linux/platform_device.h>
#include <linux/ath9k_platform.h>
#include <linux/platform_data/phy-at803x.h>

#include <asm/mach-ath79/ar71xx_regs.h>

#include "common.h"
#include "pci.h"
#include "dev-ap9x-pci.h"
#include "dev-gpio-buttons.h"
#include "dev-eth.h"
#include "dev-usb.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-spi.h"
#include "dev-wmac.h"
#include "machtypes.h"

#define DR34X_GPIO_LED_SIG1		12
#define DR34X_GPIO_LED_SIG2		13
#define DR34X_GPIO_LED_SIG3		14
#define DR34X_GPIO_LED_SIG4		15
#define DR34X_GPIO_LED_STATUS		11
#define DR344_GPIO_LED_LAN		17
#define DR344_GPIO_EXTERNAL_LNA0	18
#define DR344_GPIO_EXTERNAL_LNA1	19

#define DR34X_GPIO_BTN_RESET		16

#define DR344_KEYS_POLL_INTERVAL	20	/* msecs */
#define DR344_KEYS_DEBOUNCE_INTERVAL	(3 * DR344_KEYS_POLL_INTERVAL)

#define DR34X_MAC0_OFFSET		0
#define DR34X_MAC1_OFFSET		8
#define DR34X_WMAC_CALDATA_OFFSET	0x1000

static struct gpio_led dr342_leds_gpio[] __initdata = {
	{
		.name		= "dr342:green:status",
		.gpio		= DR34X_GPIO_LED_STATUS,
		.active_low	= 1,
	},
	{
		.name		= "dr342:green:sig1",
		.gpio		= DR34X_GPIO_LED_SIG1,
		.active_low	= 1,
	},
	{
		.name		= "dr342:green:sig2",
		.gpio		= DR34X_GPIO_LED_SIG2,
		.active_low	= 1,
	},
	{
		.name		= "dr342:green:sig3",
		.gpio		= DR34X_GPIO_LED_SIG3,
		.active_low	= 1,
	},
	{
		.name		= "dr342:green:sig4",
		.gpio		= DR34X_GPIO_LED_SIG4,
		.active_low	= 1,
	}
};

static struct gpio_led dr344_leds_gpio[] __initdata = {
	{
		.name		= "dr344:green:lan",
		.gpio		= DR344_GPIO_LED_LAN,
		.active_low	= 1,
	},
	{
		.name		= "dr344:green:status",
		.gpio		= DR34X_GPIO_LED_STATUS,
		.active_low	= 1,
	},
	{
		.name		= "dr344:green:sig1",
		.gpio		= DR34X_GPIO_LED_SIG1,
		.active_low	= 1,
	},
	{
		.name		= "dr344:green:sig2",
		.gpio		= DR34X_GPIO_LED_SIG2,
		.active_low	= 1,
	},
	{
		.name		= "dr344:green:sig3",
		.gpio		= DR34X_GPIO_LED_SIG3,
		.active_low	= 1,
	},
	{
		.name		= "dr344:green:sig4",
		.gpio		= DR34X_GPIO_LED_SIG4,
		.active_low	= 1,
	}
};

static struct gpio_keys_button dr34x_gpio_keys[] __initdata = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = DR344_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= DR34X_GPIO_BTN_RESET,
		.active_low	= 1,
	},
};

static struct at803x_platform_data dr34x_at803x_data = {
	.disable_smarteee = 1,
	.enable_rgmii_rx_delay = 1,
	.enable_rgmii_tx_delay = 1,
};

static struct mdio_board_info dr34x_mdio0_info[] = {
	{
		.bus_id = "ag71xx-mdio.0",
		.mdio_addr = 0,
		.platform_data = &dr34x_at803x_data,
	},
};

static void __init dr34x_setup(void)
{
	u8 *art = (u8 *) KSEG1ADDR(0x1fff0000);
	u8 *mac = (u8 *) KSEG1ADDR(0x1f03f810);

	ath79_register_m25p80(NULL);

	ath79_gpio_direction_select(DR34X_GPIO_LED_STATUS, true);
	gpio_set_value(DR34X_GPIO_LED_STATUS, 1);
	ath79_gpio_output_select(DR34X_GPIO_LED_STATUS, 0);

	ath79_register_gpio_keys_polled(-1, DR344_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(dr34x_gpio_keys),
					dr34x_gpio_keys);

	ath79_register_usb();

	ath79_register_wmac(art + DR34X_WMAC_CALDATA_OFFSET, NULL);

	ath79_register_pci();

	mdiobus_register_board_info(dr34x_mdio0_info,
				    ARRAY_SIZE(dr34x_mdio0_info));

	ath79_register_mdio(0, 0x0);

	ath79_setup_ar934x_eth_cfg(AR934X_ETH_CFG_RGMII_GMAC0 |
				   AR934X_ETH_CFG_SW_ONLY_MODE);

	/* GMAC0 is connected to an AR8035 Gbps PHY */
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth0_data.phy_mask = BIT(0);
	ath79_eth0_data.mii_bus_dev = &ath79_mdio0_device.dev;
	ath79_eth0_pll_data.pll_1000 = 0x02000000;
	ath79_eth0_pll_data.pll_100 = 0x0101;
	ath79_eth0_pll_data.pll_10 = 0x1313;

	ath79_init_mac(ath79_eth0_data.mac_addr, mac + DR34X_MAC0_OFFSET, 0);
	ath79_register_eth(0);
}

static void __init dr342_setup(void)
{
	dr34x_setup();

	ath79_register_leds_gpio(-1, ARRAY_SIZE(dr342_leds_gpio),
				 dr342_leds_gpio);
}

static void __init dr344_setup(void)
{
	u8 *mac = (u8 *) KSEG1ADDR(0x1f03f810);

	dr34x_setup();

	ath79_gpio_direction_select(DR344_GPIO_LED_LAN, true);
	gpio_set_value(DR344_GPIO_LED_LAN, 1);
	ath79_gpio_output_select(DR344_GPIO_LED_LAN, 0);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(dr344_leds_gpio),
				 dr344_leds_gpio);

	ath79_wmac_set_ext_lna_gpio(0, DR344_GPIO_EXTERNAL_LNA0);
	ath79_wmac_set_ext_lna_gpio(1, DR344_GPIO_EXTERNAL_LNA1);

	ath79_register_mdio(1, 0x0);

	/* GMAC1 is connected to the internal switch */
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_GMII;
	ath79_eth1_data.speed = SPEED_1000;
	ath79_eth1_data.duplex = DUPLEX_FULL;

	ath79_init_mac(ath79_eth1_data.mac_addr, mac + DR34X_MAC1_OFFSET, 0);
	ath79_register_eth(1);
}

MIPS_MACHINE(ATH79_MACH_DR342, "DR342", "Wallys DR342", dr342_setup);
MIPS_MACHINE(ATH79_MACH_DR344, "DR344", "Wallys DR344", dr344_setup);

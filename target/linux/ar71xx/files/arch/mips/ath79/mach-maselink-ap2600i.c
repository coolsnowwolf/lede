/*
 * Comba MASELink AP2600-I board support
 *
 * Copyright (c) 2016 Weijie Gao <hackpascal@gmail.com>
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
#include <linux/platform_data/phy-at803x.h>

#include <asm/mach-ath79/ar71xx_regs.h>

#include "common.h"
#include "dev-ap9x-pci.h"
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-nfc.h"
#include "dev-spi.h"
#include "dev-usb.h"
#include "dev-wmac.h"
#include "machtypes.h"

#define AP2600I_GPIO_LED_POWER_RED	11
#define AP2600I_GPIO_LED_POWER_GREEN	12
#define AP2600I_GPIO_LED_RUN		13
#define AP2600I_GPIO_LED_WLAN		14

#define AP2600I_GPIO_BUTTON_RESET	15

#define AP2600I_KEYS_POLL_INTERVAL	20	/* msecs */
#define AP2600I_KEYS_DEBOUNCE_INTERVAL	(3 * AP2600I_KEYS_POLL_INTERVAL)

#define AP2600I_MAC0_OFFSET		0
#define AP2600I_WMAC_CALDATA_OFFSET	0x1000
#define AP2600I_PCIE_CALDATA_OFFSET	0x5000

static struct gpio_led ap2600i_leds_gpio[] __initdata = {
	{
		.name		= "ap2600i:red:power",
		.gpio		= AP2600I_GPIO_LED_POWER_RED,
		.active_low	= 1,
	},
	{
		.name		= "ap2600i:green:power",
		.gpio		= AP2600I_GPIO_LED_POWER_GREEN,
		.active_low	= 1,
	},
	{
		.name		= "ap2600i:green:run",
		.gpio		= AP2600I_GPIO_LED_RUN,
		.active_low	= 1,
	},
	{
		.name		= "ap2600i:green:wlan",
		.gpio		= AP2600I_GPIO_LED_WLAN,
		.active_low	= 1,
	}
};

static struct gpio_keys_button ap2600i_gpio_keys[] __initdata = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = AP2600I_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= AP2600I_GPIO_BUTTON_RESET,
		.active_low	= 1,
	},
};

static struct at803x_platform_data ap2600i_ar8035_data = {
	.enable_rgmii_rx_delay = 1,
	.fixup_rgmii_tx_delay = 1,
};

static struct mdio_board_info ap2600i_mdio0_info[] = {
	{
		.bus_id = "ag71xx-mdio.0",
		.phy_addr = 4,
		.platform_data = &ap2600i_ar8035_data,
	},
};

static void __init ap2600i_setup(void)
{
	u8 *art = (u8 *) KSEG1ADDR(0x1fff0000);

	ath79_register_m25p80(NULL);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(ap2600i_leds_gpio),
				 ap2600i_leds_gpio);
	ath79_register_gpio_keys_polled(-1, AP2600I_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(ap2600i_gpio_keys),
					ap2600i_gpio_keys);

	ath79_register_wmac(art + AP2600I_WMAC_CALDATA_OFFSET, NULL);
	ap91_pci_init(art + AP2600I_PCIE_CALDATA_OFFSET, NULL);

	ath79_setup_ar934x_eth_cfg(AR934X_ETH_CFG_RGMII_GMAC0 |
				   AR934X_ETH_CFG_SW_ONLY_MODE);

	ath79_register_mdio(0, 0x0);

	ath79_init_mac(ath79_eth0_data.mac_addr, art + AP2600I_MAC0_OFFSET, 0);

	mdiobus_register_board_info(ap2600i_mdio0_info,
				    ARRAY_SIZE(ap2600i_mdio0_info));

	/* GMAC0 is connected to an AR8035 Gigabit PHY */
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth0_data.phy_mask = BIT(4);
	ath79_eth0_data.mii_bus_dev = &ath79_mdio0_device.dev;
	ath79_eth0_pll_data.pll_1000 = 0x0e000000;
	ath79_eth0_pll_data.pll_100 = 0x0101;
	ath79_eth0_pll_data.pll_10 = 0x1313;
	ath79_register_eth(0);
}

MIPS_MACHINE(ATH79_MACH_MASELINK_AP2600I, "AP2600I", "Comba MASELink AP2600-I",
	     ap2600i_setup);

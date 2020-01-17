/*
 * GRENTECH SGR-W500-N85b v2.0 board support
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
#include <linux/gpio.h>
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

#define GRENTECH_GPIO_LED_STATUS		21
#define GRENTECH_GPIO_LED_WLAN2G		20

#define GRENTECH_GPIO_BUTTON_RESET		3

#define GRENTECH_GPIO_RTL8211E_RESET_L		11

#define GRENTECH_GPIO_EXTERNAL_LNA0		18
#define GRENTECH_GPIO_EXTERNAL_LNA1		19

#define GRENTECH_KEYS_POLL_INTERVAL		20	/* msecs */
#define GRENTECH_KEYS_DEBOUNCE_INTERVAL		(3 * GRENTECH_KEYS_POLL_INTERVAL)

#define GRENTECH_MAC_OFFSET			0
#define GRENTECH_WMAC_CALDATA_OFFSET		0x1000
#define GRENTECH_PCIE_CALDATA_OFFSET		0x5000

static struct gpio_led grentech_leds_gpio[] __initdata = {
	{
		.name		= "grentech:green:status",
		.gpio		= GRENTECH_GPIO_LED_STATUS,
		.active_low	= 1,
	},
	{
		.name		= "grentech:green:wlan2g",
		.gpio		= GRENTECH_GPIO_LED_WLAN2G,
		.active_low	= 1,
	}
};

static struct gpio_keys_button grentech_gpio_keys[] __initdata = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = GRENTECH_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= GRENTECH_GPIO_BUTTON_RESET,
		.active_low	= 1,
	},
};

static struct mdio_board_info grentech_mdio0_info[] = {
	{
		.bus_id = "ag71xx-mdio.0",
		.mdio_addr = 1,
	},
};

static void __init grentech_setup(void)
{
	u8 *art = (u8 *) KSEG1ADDR(0x1fff0000);
	u8 tmpmac[ETH_ALEN];

	ath79_register_m25p80(NULL);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(grentech_leds_gpio),
				 grentech_leds_gpio);
	ath79_register_gpio_keys_polled(-1, GRENTECH_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(grentech_gpio_keys),
					grentech_gpio_keys);

	gpio_request_one(GRENTECH_GPIO_RTL8211E_RESET_L,
			 GPIOF_OUT_INIT_HIGH | GPIOF_EXPORT_DIR_FIXED,
			 "RTL8211E reset pin");

	ath79_wmac_set_ext_lna_gpio(0, GRENTECH_GPIO_EXTERNAL_LNA0);
	ath79_wmac_set_ext_lna_gpio(1, GRENTECH_GPIO_EXTERNAL_LNA1);

	ath79_init_mac(tmpmac, art + GRENTECH_MAC_OFFSET, 1);
	ath79_register_wmac(art + GRENTECH_WMAC_CALDATA_OFFSET, tmpmac);

	/* AR9382 */
	ath79_init_mac(tmpmac, art + GRENTECH_MAC_OFFSET, 2);
	ap9x_pci_setup_wmac_led_pin(0, 6);
	ap91_pci_init(art + GRENTECH_PCIE_CALDATA_OFFSET, tmpmac);

	ath79_setup_ar934x_eth_cfg(AR934X_ETH_CFG_RGMII_GMAC0);

	ath79_register_mdio(0, 0x0);

	ath79_init_mac(ath79_eth0_data.mac_addr, art + GRENTECH_MAC_OFFSET, 0);

	mdiobus_register_board_info(grentech_mdio0_info,
				    ARRAY_SIZE(grentech_mdio0_info));

	/* GMAC0 is connected to a RTL8211E Gigabit PHY */
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth0_data.phy_mask = BIT(1);
	ath79_eth0_data.mii_bus_dev = &ath79_mdio0_device.dev;
	ath79_eth0_pll_data.pll_1000 = 0x46000000;
	ath79_eth0_pll_data.pll_100 = 0x0101;
	ath79_eth0_pll_data.pll_10 = 0x1313;
	ath79_register_eth(0);
}

MIPS_MACHINE(ATH79_MACH_SGR_W500_N85B_V2, "SGRW500N85BV2", "GRENTECH SGR-W500-N85b v2.0",
	     grentech_setup);

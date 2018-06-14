/*
 * Compex WPJ342 board support
 *
 * Copyright (c) 2011 Qualcomm Atheros
 * Copyright (c) 2011-2012 Gabor Juhos <juhosg@openwrt.org>
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

#include <linux/irq.h>
#include <linux/pci.h>
#include <linux/phy.h>
#include <linux/platform_device.h>
#include <linux/ath9k_platform.h>
#include <linux/ar8216_platform.h>
#include <linux/export.h>

#include <asm/mach-ath79/ar71xx_regs.h>

#include "pci.h"
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

#define WPJ342_GPIO_LED_STATUS		11
#define WPJ342_GPIO_LED_SIG1		14
#define WPJ342_GPIO_LED_SIG2		13
#define WPJ342_GPIO_LED_SIG3		12
#define WPJ342_GPIO_LED_SIG4		11
#define WPJ342_GPIO_BUZZER			15

#define WPJ342_GPIO_BTN_RESET		17

#define WPJ342_KEYS_POLL_INTERVAL	20 /* msecs */
#define WPJ342_KEYS_DEBOUNCE_INTERVAL	(3 * WPJ342_KEYS_POLL_INTERVAL)

#define WPJ342_MAC0_OFFSET		0x10
#define WPJ342_MAC1_OFFSET		0x18
#define WPJ342_WMAC_CALDATA_OFFSET	0x1000
#define WPJ342_PCIE_CALDATA_OFFSET	0x5000

#define WPJ342_ART_SIZE		0x8000

static struct gpio_led wpj342_leds_gpio[] __initdata = {
	{
		.name		= "wpj342:red:sig1",
		.gpio		= WPJ342_GPIO_LED_SIG1,
		.active_low	= 1,
	},
	{
		.name		= "wpj342:yellow:sig2",
		.gpio		= WPJ342_GPIO_LED_SIG2,
		.active_low	= 1,
	},
	{
		.name		= "wpj342:green:sig3",
		.gpio		= WPJ342_GPIO_LED_SIG3,
		.active_low	= 1,
	},
	{
		.name		= "wpj342:green:sig4",
		.gpio		= WPJ342_GPIO_LED_SIG4,
		.active_low	= 1,
	},
	{
		.name		= "wpj342:buzzer",
		.gpio		= WPJ342_GPIO_BUZZER,
		.active_low	= 0,
	}
};

static struct gpio_keys_button wpj342_gpio_keys[] __initdata = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = WPJ342_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= WPJ342_GPIO_BTN_RESET,
		.active_low	= 1,
	},
};

static struct ar8327_pad_cfg wpj342_ar8327_pad0_cfg = {
	.mode = AR8327_PAD_MAC_RGMII,
	.txclk_delay_en = true,
	.rxclk_delay_en = true,
	.txclk_delay_sel = AR8327_CLK_DELAY_SEL1,
	.rxclk_delay_sel = AR8327_CLK_DELAY_SEL2,
};

static struct ar8327_led_cfg wpj342_ar8327_led_cfg = {
	.led_ctrl0 = 0x00000000,
	.led_ctrl1 = 0xc737c737,
	.led_ctrl2 = 0x00000000,
	.led_ctrl3 = 0x00c30c00,
	.open_drain = true,
};

static struct ar8327_platform_data wpj342_ar8327_data = {
	.pad0_cfg = &wpj342_ar8327_pad0_cfg,
	.port0_cfg = {
		.force_link = 1,
		.speed = AR8327_PORT_SPEED_1000,
		.duplex = 1,
		.txpause = 1,
		.rxpause = 1,
	},
	.led_cfg = &wpj342_ar8327_led_cfg,
};

static struct mdio_board_info wpj342_mdio0_info[] = {
	{
		.bus_id = "ag71xx-mdio.0",
		.phy_addr = 0,
		.platform_data = &wpj342_ar8327_data,
	},
};


static void __init wpj342_setup(void)
{
	u8 *art = (u8 *) KSEG1ADDR(0x1fff0000);
	u8 *mac = (u8 *) KSEG1ADDR(0x1f02e000);

	ath79_register_m25p80(NULL);
	ath79_register_leds_gpio(-1, ARRAY_SIZE(wpj342_leds_gpio),
				wpj342_leds_gpio);

	ath79_register_gpio_keys_polled(-1, WPJ342_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(wpj342_gpio_keys),
					wpj342_gpio_keys);

	ath79_register_usb();

	ath79_register_wmac(art + WPJ342_WMAC_CALDATA_OFFSET, NULL);

	ath79_register_pci();

	mdiobus_register_board_info(wpj342_mdio0_info,
				ARRAY_SIZE(wpj342_mdio0_info));

	ath79_register_mdio(1, 0x0);
	ath79_register_mdio(0, 0x0);

	ath79_init_mac(ath79_eth0_data.mac_addr, mac + WPJ342_MAC0_OFFSET, 0);
	ath79_init_mac(ath79_eth1_data.mac_addr, mac + WPJ342_MAC1_OFFSET, 0);

	ath79_setup_ar934x_eth_cfg(AR934X_ETH_CFG_MII_GMAC0);

	/* GMAC0 is connected to an AR8236 switch */
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ath79_eth0_data.phy_mask = BIT(0);
	ath79_eth0_data.mii_bus_dev = &ath79_mdio0_device.dev;
	ath79_eth0_pll_data.pll_1000 = 0x06000000;

	ath79_register_eth(0);
}

MIPS_MACHINE(ATH79_MACH_WPJ342, "WPJ342", "Compex WPJ342", wpj342_setup);

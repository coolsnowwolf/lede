/*
 * PowerCloud Systems CR5000 support
 *
 * Copyright (c) 2011 Qualcomm Atheros
 * Copyright (c) 2011-2012 Gabor Juhos <juhosg@openwrt.org>
 * Copyright (c) 2012-2013 PowerCloud Systems
 * Copyright (c) 2015 Daniel Dickinson <openwrt@daniel.thecshore.com>
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
#include <linux/pci.h>
#include <linux/phy.h>
#include <linux/platform_device.h>
#include <linux/ath9k_platform.h>
#include <linux/ar8216_platform.h>

#include <asm/mach-ath79/ar71xx_regs.h>
#include <asm/mach-ath79/ath79.h>

#include "common.h"
#include "dev-ap9x-pci.h"
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-spi.h"
#include "dev-usb.h"
#include "dev-wmac.h"
#include "machtypes.h"

#define CR5000_GPIO_LED_WLAN_2G		14
#define CR5000_GPIO_LED_WPS		12
#define CR5000_GPIO_LED_POWER_AMBER      4
/* GPIO2 has to have JTAG disabled as it is also to
 * power led
 */
#define CR5000_GPIO_LED_POWER_ENABLE     2
#define CR5000_GPIO_BTN_WPS		16
#define CR5000_GPIO_BTN_RESET		17

#define CR5000_KEYS_POLL_INTERVAL	20	/* msecs */
#define CR5000_KEYS_DEBOUNCE_INTERVAL	(3 * CR5000_KEYS_POLL_INTERVAL)

#define CR5000_MAC0_OFFSET		0
#define CR5000_WMAC_CALDATA_OFFSET	0x1000
#define CR5000_WMAC_MAC_OFFSET	        0x1002
#define CR5000_PCIE_CALDATA_OFFSET	0x5000
#define CR5000_PCIE_WMAC_OFFSET		0x5002

static struct gpio_led cr5000_leds_gpio[] __initdata = {
	{
		.name		= "pcs:amber:power",
		.gpio		= CR5000_GPIO_LED_POWER_AMBER,
		.active_low	= 1,
	},
	{
		.name		= "pcs:white:wps",
		.gpio		= CR5000_GPIO_LED_WPS,
		.active_low	= 1,
	},
	{
		.name		= "pcs:blue:wlan",
		.gpio		= CR5000_GPIO_LED_WLAN_2G,
		.active_low	= 1,
	},
};

static struct gpio_keys_button cr5000_gpio_keys[] __initdata = {
	{
		.desc		= "WPS button",
		.type		= EV_KEY,
		.code		= KEY_WPS_BUTTON,
		.debounce_interval = CR5000_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= CR5000_GPIO_BTN_WPS,
		.active_low	= 1,
	},
	{
		.desc		= "Reset button",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = CR5000_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= CR5000_GPIO_BTN_RESET,
		.active_low	= 1,
	},
};

static struct ar8327_pad_cfg cr5000_ar8327_pad0_cfg = {
	.mode = AR8327_PAD_MAC_RGMII,
	.txclk_delay_en = true,
	.rxclk_delay_en = true,
	.txclk_delay_sel = AR8327_CLK_DELAY_SEL1,
	.rxclk_delay_sel = AR8327_CLK_DELAY_SEL2,
};

static struct ar8327_led_cfg cr5000_ar8327_led_cfg = {
	.led_ctrl0 = 0xcc35cc35,
	.led_ctrl1 = 0xca35ca35,
	.led_ctrl2 = 0xc935c935,
	.led_ctrl3 = 0x03ffff00,
	.open_drain = true,
};

static struct ar8327_platform_data cr5000_ar8327_data = {
	.pad0_cfg = &cr5000_ar8327_pad0_cfg,
	.port0_cfg = {
		.force_link = 1,
		.speed = AR8327_PORT_SPEED_1000,
		.duplex = 1,
		.txpause = 1,
		.rxpause = 1,
	},
	.led_cfg = &cr5000_ar8327_led_cfg,
};

static struct mdio_board_info cr5000_mdio0_info[] = {
	{
		.bus_id = "ag71xx-mdio.0",
		.mdio_addr = 0,
		.platform_data = &cr5000_ar8327_data,
	},
};

static void __init cr5000_setup(void)
{
	u8 *art = (u8 *) KSEG1ADDR(0x1fff0000);

	ath79_gpio_function_enable(AR934X_GPIO_FUNC_JTAG_DISABLE);
	gpio_request_one(CR5000_GPIO_LED_POWER_ENABLE,
	GPIOF_OUT_INIT_LOW, "Power LED enable");
	ath79_gpio_output_select(CR5000_GPIO_LED_POWER_AMBER, AR934X_GPIO_OUT_GPIO);
	ath79_gpio_output_select(CR5000_GPIO_LED_WLAN_2G, AR934X_GPIO_OUT_GPIO);
	ath79_gpio_output_select(CR5000_GPIO_LED_WPS, AR934X_GPIO_OUT_GPIO);

	ath79_register_m25p80(NULL);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(cr5000_leds_gpio),
				 cr5000_leds_gpio);
	ath79_register_gpio_keys_polled(-1, CR5000_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(cr5000_gpio_keys),
					cr5000_gpio_keys);
	ath79_register_usb();
	ath79_register_wmac(art + CR5000_WMAC_CALDATA_OFFSET, art + CR5000_WMAC_MAC_OFFSET);
	ap94_pci_init(NULL, NULL, NULL, art + CR5000_PCIE_WMAC_OFFSET);

        ath79_setup_ar934x_eth_cfg(AR934X_ETH_CFG_RGMII_GMAC0);

	ath79_register_mdio(0, 0x0);

	ath79_init_mac(ath79_eth0_data.mac_addr, art + CR5000_MAC0_OFFSET, 0);

	mdiobus_register_board_info(cr5000_mdio0_info,
				    ARRAY_SIZE(cr5000_mdio0_info));

	/* GMAC0 is connected to an AR8327 switch */
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth0_data.phy_mask = BIT(0);
	ath79_eth0_data.mii_bus_dev = &ath79_mdio0_device.dev;
	ath79_eth0_pll_data.pll_1000 = 0x06000000;
	ath79_register_eth(0);
}

MIPS_MACHINE(ATH79_MACH_CR5000, "CR5000", "PowerCloud Systems CR5000",
	     cr5000_setup);

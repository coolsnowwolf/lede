/*
 * ARRIS SBR-AC1750/TR3300 Router support
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

#include <linux/gpio.h>
#include <linux/platform_device.h>
#include <linux/ar8216_platform.h>
#include <linux/platform/ar934x_nfc.h>

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

#define ARRIS_GPIO_EXT_WDT		18
#define ARRIS_GPIO_LED_USB		22
#define ARRIS_GPIO_LED_WLAN_5G		21
#define ARRIS_GPIO_LED_WLAN_2G		2
#define ARRIS_GPIO_LED_WPS_GREEN	23

#define ARRIS_GPIO_BTN_WPS		19
#define ARRIS_GPIO_BTN_RESET		17

#define ARRIS_KEYS_POLL_INTERVAL	20	/* msecs */
#define ARRIS_KEYS_DEBOUNCE_INTERVAL	(3 * ARRIS_KEYS_POLL_INTERVAL)

#define ARRIS_MAC0_OFFSET		0
#define ARRIS_MAC1_OFFSET		6
#define ARRIS_WMAC_CALDATA_OFFSET	0x1000
#define ARRIS_PCIE_CALDATA_OFFSET	0x5000

static struct gpio_led arris_leds_gpio[] __initdata = {
	{
		.name		= "arris:green:wps",
		.gpio		= ARRIS_GPIO_LED_WPS_GREEN,
		.active_low	= 1,
	},
	{
		.name		= "wlan_2g",
		.gpio		= ARRIS_GPIO_LED_WLAN_2G,
		.active_low	= 1,
	},
	{
		.name		= "wlan_5g",
		.gpio		= ARRIS_GPIO_LED_WLAN_5G,
		.active_low	= 1,
	},
	{
		.name		= "arris:green:usb",
		.gpio		= ARRIS_GPIO_LED_USB,
		.active_low	= 1,
	}
};

static struct gpio_keys_button arris_gpio_keys[] __initdata = {
	{
		.desc		= "WPS button",
		.type		= EV_KEY,
		.code		= KEY_WPS_BUTTON,
		.debounce_interval = ARRIS_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= ARRIS_GPIO_BTN_WPS,
		.active_low	= 1,
	},
	{
		.desc		= "RESET button",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = ARRIS_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= ARRIS_GPIO_BTN_RESET,
		.active_low	= 1,
	},
};

static struct ar8327_pad_cfg arris_ar8327_pad0_cfg;
static struct ar8327_pad_cfg arris_ar8327_pad6_cfg;

static struct ar8327_platform_data arris_ar8327_data = {
	.pad0_cfg = &arris_ar8327_pad0_cfg,
	.pad6_cfg = &arris_ar8327_pad6_cfg,
	.port0_cfg = {
		.force_link = 1,
		.speed = AR8327_PORT_SPEED_1000,
		.duplex = 1,
		.txpause = 1,
		.rxpause = 1,
	},
	.port6_cfg = {
		.force_link = 1,
		.speed = AR8327_PORT_SPEED_1000,
		.duplex = 1,
		.txpause = 1,
		.rxpause = 1,
	},
};

static struct mdio_board_info arris_mdio0_info[] = {
	{
		.bus_id = "ag71xx-mdio.0",
		.mdio_addr = 0,
		.platform_data = &arris_ar8327_data,
	},
};

#define EXT_WDT_TIMEOUT_MS	300

static struct timer_list gpio_wdt_timer;

static void gpio_wdt_toggle(unsigned long gpio)
{
	static int state;

	state = !state;
	gpio_set_value(gpio, state);

	mod_timer(&gpio_wdt_timer,
		  jiffies + msecs_to_jiffies(EXT_WDT_TIMEOUT_MS));
}

static void __init feed_external_wdt(int gpio_wdt)
{
	if (gpio_wdt > -1) {
		gpio_request_one(gpio_wdt, GPIOF_OUT_INIT_HIGH,
				 "PT7A7514 watchdog");

		setup_timer(&gpio_wdt_timer, gpio_wdt_toggle, gpio_wdt);
		gpio_wdt_toggle(gpio_wdt);
	}
}

static void __init arris_setup(void)
{
	feed_external_wdt(ARRIS_GPIO_EXT_WDT);

	/* GMAC0 of the AR8327 switch is connected to GMAC1 via SGMII */
	arris_ar8327_pad0_cfg.mode = AR8327_PAD_MAC_SGMII;
	arris_ar8327_pad0_cfg.sgmii_delay_en = true;

	/* GMAC6 of the AR8327 switch is connected to GMAC0 via RGMII */
	arris_ar8327_pad6_cfg.mode = AR8327_PAD_MAC_RGMII;
	arris_ar8327_pad6_cfg.txclk_delay_en = true;
	arris_ar8327_pad6_cfg.rxclk_delay_en = true;
	arris_ar8327_pad6_cfg.txclk_delay_sel = AR8327_CLK_DELAY_SEL1;
	arris_ar8327_pad6_cfg.rxclk_delay_sel = AR8327_CLK_DELAY_SEL2;

	ath79_eth0_pll_data.pll_1000 = 0x56000000;
	ath79_eth1_pll_data.pll_1000 = 0x03000101;

	ath79_register_leds_gpio(-1, ARRAY_SIZE(arris_leds_gpio),
				 arris_leds_gpio);
	ath79_register_gpio_keys_polled(-1, ARRIS_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(arris_gpio_keys),
					arris_gpio_keys);

	ath79_register_usb();

	ath79_nfc_set_ecc_mode(AR934X_NFC_ECC_HW);
	ath79_register_nfc();

	ath79_register_wmac_simple();

	ath79_setup_qca955x_eth_cfg(QCA955X_ETH_CFG_RGMII_EN);

	ath79_register_mdio(0, 0x0);

	mdiobus_register_board_info(arris_mdio0_info,
				    ARRAY_SIZE(arris_mdio0_info));
#if 0
	/* GMAC0 is connected to the RMGII interface */
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth0_data.phy_mask = BIT(0);
	ath79_eth0_data.mii_bus_dev = &ath79_mdio0_device.dev;

	ath79_register_eth(0);
#endif
	/* GMAC1 is connected to the SGMII interface */
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_SGMII;
	ath79_eth1_data.speed = SPEED_1000;
	ath79_eth1_data.duplex = DUPLEX_FULL;

	ath79_register_eth(1);

	ath79_register_pci();
}

MIPS_MACHINE(ATH79_MACH_ARRIS_SBR_AC1750, "ARRIS-SBR-AC1750", "ARRIS SBR-AC1750/TR3300 Router", arris_setup);

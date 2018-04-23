/*
 * TP-LINK RE355/RE450 board support
 *
 * Copyright (c) 2013 Gabor Juhos <juhosg@openwrt.org>
 * Copyright (c) 2016 Tal Keren <kooolk@gmail.com>
 * Copyright (c) 2018 Henryk Heisig <hyniu@o2.pl>
 * 
 * Based on the Qualcomm Atheros AP135/AP136 reference board support code
 *   Copyright (c) 2012 Qualcomm Atheros
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
#include <linux/platform_data/mdio-gpio.h>
#include <linux/platform_device.h>
#include <linux/ar8216_platform.h>

#include <asm/mach-ath79/ath79.h>
#include <asm/mach-ath79/ar71xx_regs.h>

#include "common.h"
#include "dev-ap9x-pci.h"
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-wmac.h"
#include "machtypes.h"
#include "pci.h"

#define RE450_GPIO_LED_SYSTEM		12
#define RE450_GPIO_LED_WLAN2G		13
#define RE450_GPIO_LED_WLAN5G		14
#define RE450_GPIO_LED_LAN_DATA		17
#define RE450_GPIO_LED_JUMPSTART	21
#define RE450_GPIO_LED_JUMPSTART_RED	22
#define RE450_GPIO_LED_LAN_LINK		23

#define RE450_GPIO_BTN_RESET		18
#define RE450_GPIO_BTN_LED		19
#define RE450_GPIO_BTN_JUMPSTART	20

#define RE450_GPIO_SMI_MDIO		1
#define RE450_GPIO_SMI_MDC		3

#define RE450_LAN_PHYADDR		4

#define RE450_KEYS_POLL_INTERVAL	20	/* msecs */
#define RE450_KEYS_DEBOUNCE_INTERVAL (3 * RE450_KEYS_POLL_INTERVAL)

#define RE450_WMAC_CALDATA_OFFSET	0x1000

static const char *tl_re450_part_probes[] = {
	"cmdlinepart",
	NULL,
};

static struct flash_platform_data tl_re450_flash_data = {
	.part_probes	= tl_re450_part_probes,
};

static struct gpio_led re355_leds_gpio[] __initdata = {
	{
		.name		= "re355:blue:power",
		.gpio		= RE450_GPIO_LED_SYSTEM,
		.active_low	= 1,
	},
	{
		.name		= "re355:blue:wlan2g",
		.gpio		= RE450_GPIO_LED_WLAN2G,
		.active_low	= 1,
	},
	{
		.name		= "re355:blue:wlan5g",
		.gpio		= RE450_GPIO_LED_WLAN5G,
		.active_low	= 1,
	},
	{
		.name		= "re355:blue:wps",
		.gpio		= RE450_GPIO_LED_JUMPSTART,
	},
	{
		.name		= "re355:red:wps",
		.gpio		= RE450_GPIO_LED_JUMPSTART_RED,
	},
	{
		.name		= "re355:green:lan_data",
		.gpio		= RE450_GPIO_LED_LAN_DATA,
		.active_low	= 1,
	},
	{
		.name		= "re355:green:lan_link",
		.gpio		= RE450_GPIO_LED_LAN_LINK,
		.active_low	= 1,
	},
};

static struct gpio_led re450_leds_gpio[] __initdata = {
	{
		.name		= "re450:blue:power",
		.gpio		= RE450_GPIO_LED_SYSTEM,
		.active_low	= 1,
	},
	{
		.name		= "re450:blue:wlan2g",
		.gpio		= RE450_GPIO_LED_WLAN2G,
		.active_low	= 1,
	},
	{
		.name		= "re450:blue:wlan5g",
		.gpio		= RE450_GPIO_LED_WLAN5G,
		.active_low	= 1,
	},
	{
		.name		= "re450:blue:wps",
		.gpio		= RE450_GPIO_LED_JUMPSTART,
	},
	{
		.name		= "re450:red:wps",
		.gpio		= RE450_GPIO_LED_JUMPSTART_RED,
	},
	{
		.name		= "re450:green:lan_data",
		.gpio		= RE450_GPIO_LED_LAN_DATA,
		.active_low	= 1,
	},
	{
		.name		= "re450:green:lan_link",
		.gpio		= RE450_GPIO_LED_LAN_LINK,
		.active_low	= 1,
	},
};

static struct gpio_keys_button re450_gpio_keys[] __initdata = {
	{
		.desc		= "Reset button",
		.type		= EV_KEY,
		.code		= KEY_WPS_BUTTON,
		.debounce_interval = RE450_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= RE450_GPIO_BTN_RESET,
		.active_low	= 1,
	},
	{
		.desc		= "WPS button",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = RE450_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= RE450_GPIO_BTN_JUMPSTART,
		.active_low	= 1,
	},
	{
		.desc		= "Control LED button",
		.type		= EV_KEY,
		.code		= BTN_0,
		.debounce_interval = RE450_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= RE450_GPIO_BTN_LED,
		.active_low	= 1,
	},
};

static struct mdio_gpio_platform_data re450_mdio = {
	.mdc		= RE450_GPIO_SMI_MDC,
	.mdio		= RE450_GPIO_SMI_MDIO,
	.phy_mask	= ~BIT(RE450_LAN_PHYADDR),
};

static struct platform_device re450_phy_device = {
	.name	= "mdio-gpio",
	.id	= 0,
	.dev	= {
		.platform_data = &re450_mdio,
	},
};

static void __init rex5x_setup(void)
{
	u8 *mac = (u8 *) KSEG1ADDR(0x1f610008);
	u8 *art = (u8 *) KSEG1ADDR(0x1fff0000);
	u8 tmpmac[ETH_ALEN];

	ath79_register_m25p80(&tl_re450_flash_data);
	ath79_register_gpio_keys_polled(-1, RE450_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(re450_gpio_keys),
					re450_gpio_keys);

	ath79_init_mac(tmpmac, mac, -1);
	ath79_register_wmac(art + RE450_WMAC_CALDATA_OFFSET, tmpmac);

	ath79_register_pci();

	/* MDIO Interface */
	platform_device_register(&re450_phy_device);

	ath79_setup_qca955x_eth_cfg(QCA955X_ETH_CFG_RGMII_EN);

	/* GMAC0 is connected to the RGMII interface to an Atheros AR8035-A */
	ath79_init_mac(ath79_eth0_data.mac_addr, mac, 0);
	ath79_eth0_data.mii_bus_dev = &re450_phy_device.dev;
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth0_data.phy_mask = BIT(RE450_LAN_PHYADDR);
	ath79_eth0_pll_data.pll_1000 = 0xa6000000;
	ath79_eth0_pll_data.pll_100 = 0xa0000101;
	ath79_eth0_pll_data.pll_10 = 0x80001313;
	ath79_register_eth(0);
}

static void __init re355_setup(void)
{
	rex5x_setup();
	ath79_register_leds_gpio(-1, ARRAY_SIZE(re355_leds_gpio),
				 re355_leds_gpio);
}

MIPS_MACHINE(ATH79_MACH_RE355, "RE355", "TP-LINK RE355",
	     re355_setup)

static void __init re450_setup(void)
{
	rex5x_setup();
	ath79_register_leds_gpio(-1, ARRAY_SIZE(re450_leds_gpio),
				 re450_leds_gpio);
}

MIPS_MACHINE(ATH79_MACH_RE450, "RE450", "TP-LINK RE450",
	     re450_setup)

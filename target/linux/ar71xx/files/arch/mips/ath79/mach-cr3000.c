/*
 * PowerCloud Systems CR3000 support
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
#include <linux/phy.h>
#include <linux/platform_device.h>
#include <linux/ath9k_platform.h>
#include <linux/ar8216_platform.h>

#include <asm/mach-ath79/ar71xx_regs.h>
#include <asm/mach-ath79/ath79.h>

#include "common.h"
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-spi.h"
#include "dev-wmac.h"
#include "machtypes.h"

#define CR3000_GPIO_LED_WLAN_2G		13
#define CR3000_GPIO_LED_POWER_AMBER	15
#define CR3000_GPIO_LED_WAN             18
#define CR3000_GPIO_LED_LAN1            19
#define CR3000_GPIO_LED_LAN2            20
#define CR3000_GPIO_LED_LAN3            21
#define CR3000_GPIO_LED_LAN4            22

#define CR3000_GPIO_BTN_WPS		16
#define CR3000_GPIO_BTN_RESET		17

#define CR3000_KEYS_POLL_INTERVAL	20	/* msecs */
#define CR3000_KEYS_DEBOUNCE_INTERVAL	(3 * CR3000_KEYS_POLL_INTERVAL)

#define CR3000_MAC0_OFFSET		0
#define CR3000_MAC1_OFFSET		6
#define CR3000_WMAC_CALDATA_OFFSET	0x1000
#define CR3000_WMAC_MAC_OFFSET	        0x1002

static struct gpio_led cr3000_leds_gpio[] __initdata = {
	{
		.name		= "pcs:amber:power",
		.gpio		= CR3000_GPIO_LED_POWER_AMBER,
		.active_low	= 1,
	},
	{
		.name		= "pcs:blue:wlan",
		.gpio		= CR3000_GPIO_LED_WLAN_2G,
		.active_low	= 1,
	},
	{
		.name		= "pcs:blue:wan",
		.gpio		= CR3000_GPIO_LED_WAN,
		.active_low	= 1,
	},
	{
		.name		= "pcs:blue:lan1",
		.gpio		= CR3000_GPIO_LED_LAN1,
		.active_low	= 1,
	},
	{
		.name		= "pcs:blue:lan2",
		.gpio		= CR3000_GPIO_LED_LAN2,
		.active_low	= 1,
	},
	{
		.name		= "pcs:blue:lan3",
		.gpio		= CR3000_GPIO_LED_LAN3,
		.active_low	= 1,
	},
	{
		.name		= "pcs:blue:lan4",
		.gpio		= CR3000_GPIO_LED_LAN4,
		.active_low	= 1,
	},
};

static struct gpio_keys_button cr3000_gpio_keys[] __initdata = {
	{
		.desc		= "WPS button",
		.type		= EV_KEY,
		.code		= KEY_WPS_BUTTON,
		.debounce_interval = CR3000_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= CR3000_GPIO_BTN_WPS,
		.active_low	= 1,
	},
	{
		.desc		= "Reset button",
		.type		= EV_KEY,
		.code		= KEY_WPS_BUTTON,
		.debounce_interval = CR3000_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= CR3000_GPIO_BTN_RESET,
		.active_low	= 1,
	},
};

static void __init cr3000_setup(void)
{
	u8 *art = (u8 *) KSEG1ADDR(0x1fff0000);

	ath79_register_m25p80(NULL);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(cr3000_leds_gpio),
				 cr3000_leds_gpio);

	ath79_register_gpio_keys_polled(-1, CR3000_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(cr3000_gpio_keys),
					cr3000_gpio_keys);

	/* WLAN 2GHz onboard */
	ath79_register_wmac(art + CR3000_WMAC_CALDATA_OFFSET, art + CR3000_WMAC_MAC_OFFSET);

	/* FE Lan on first 4-ports of internal switch and attached to GMAC1
	 * WAN Fast Ethernet interface attached to GMAC0
	 * Could be configured as a 5-port switch, but we use
	 * the SoC capabilities to attach port 5 to a separate PHY/MAC
	 * theoretically this leaves future possibility of using SoC
	 * acceleration/offloading.
	 */
	ath79_setup_ar934x_eth_cfg(AR934X_ETH_CFG_SW_PHY_SWAP);

	/* GMAC0 attached to PHY4 (port 5 of the internal switch) */
	ath79_switch_data.phy4_mii_en = 1;
	/* For switch carrier ignore port 5 (wan) */
	ath79_switch_data.phy_poll_mask = 0x1;

	/* Register MII bus */
	ath79_register_mdio(1, 0x0);

	/* GMAC0 attached to PHY4 (port 5 of the internal switch) */
	ath79_switch_data.phy4_mii_en = 1;
	ath79_switch_data.phy_poll_mask = 0x1;

	/* LAN */
	ath79_init_mac(ath79_eth1_data.mac_addr, art + CR3000_MAC0_OFFSET, 0);
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_GMII;
	ath79_register_eth(1);

	/* Wan */
	ath79_init_mac(ath79_eth0_data.mac_addr, art + CR3000_MAC0_OFFSET, 1);
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ath79_eth0_data.phy_mask = BIT(0);
	ath79_eth0_data.mii_bus_dev = &ath79_mdio1_device.dev;
	ath79_register_eth(0);
}

MIPS_MACHINE(ATH79_MACH_CR3000, "CR3000", "PowerCloud Systems CR3000",
	     cr3000_setup);

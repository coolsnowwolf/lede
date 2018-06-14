/*
 * EW Balin board support
 * (based on Atheros DB120 reference board support)
 *
 * Copyright (c) 2011 Qualcomm Atheros
 * Copyright (c) 2011-2012 Gabor Juhos <juhosg@openwrt.org>
 * Copyright (C) 2017 Embedded Wireless GmbH    www.80211.de
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
#include <linux/ar8216_platform.h>

#include <asm/mach-ath79/ar71xx_regs.h>

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
#include "pci.h"

#define BALIN_GPIO_LED_STATUS	14

#define BALIN_GPIO_BTN_WPS		18

#define BALIN_KEYS_POLL_INTERVAL	20	/* msecs */
#define BALIN_KEYS_DEBOUNCE_INTERVAL	(3 * BALIN_KEYS_POLL_INTERVAL)

#define BALIN_CALDATA_OFFSET		0x1000
#define BALIN_WMAC_MAC_OFFSET		(BALIN_CALDATA_OFFSET + 0x02)

static struct gpio_led balin_leds_gpio[] __initdata = {
	{
		.name		= "balin:green:status",
		.gpio		= BALIN_GPIO_LED_STATUS,
		.active_low	= 1,
	},
};

static struct gpio_keys_button balin_gpio_keys[] __initdata = {
	{
		.desc		= "WPS button",
		.type		= EV_KEY,
		.code		= KEY_WPS_BUTTON,
		.debounce_interval = BALIN_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= BALIN_GPIO_BTN_WPS,
		.active_low	= 0,
	},
};


static void __init balin_setup(void)
{
	u8 *art = (u8 *) KSEG1ADDR(0x1fff0000);
	static u8 mac[6];

	ath79_register_m25p80(NULL);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(balin_leds_gpio),
				 balin_leds_gpio);

	ath79_register_gpio_keys_polled(-1, BALIN_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(balin_gpio_keys),
					balin_gpio_keys);

	ath79_register_usb();

	ath79_register_wmac(art + BALIN_CALDATA_OFFSET, NULL);

	ath79_register_pci();

	ath79_setup_ar934x_eth_cfg(AR934X_ETH_CFG_RGMII_GMAC0 |
				   AR934X_ETH_CFG_SW_ONLY_MODE);

	ath79_register_mdio(1, 0x0);

	/* GMAC1 is connected to the internal switch */
	memcpy(mac, art + BALIN_WMAC_MAC_OFFSET, sizeof(mac));
	mac[3] |= 0x40;
	ath79_init_mac(ath79_eth1_data.mac_addr, mac, 0);
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_GMII;
	ath79_eth1_data.speed = SPEED_1000;
	ath79_eth1_data.duplex = DUPLEX_FULL;

	ath79_register_eth(1);
}

MIPS_MACHINE(ATH79_MACH_EW_BALIN, "EW-BALIN", "EmbWir-Balin",
	     balin_setup);

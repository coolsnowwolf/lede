/*
 * Compex WPJ563 board support
 *
 * Copyright (c) 2015 Qualcomm Atheros
 * Copyright (c) 2012 Gabor Juhos <juhosg@openwrt.org>
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
#include <linux/ath9k_platform.h>
#include <linux/ar8216_platform.h>
#include <asm/mach-ath79/ar71xx_regs.h>

#include "common.h"
#include "dev-m25p80.h"
#include "machtypes.h"
#include "pci.h"
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-spi.h"
#include "dev-usb.h"
#include "dev-wmac.h"

#define WPJ563_GPIO_LED_SIG1     1
#define WPJ563_GPIO_LED_SIG2     5
#define WPJ563_GPIO_LED_SIG3     6
#define WPJ563_GPIO_LED_SIG4     7
#define WPJ563_GPIO_BUZZER      19

#define WPJ563_GPIO_BTN_RESET            2
#define WPJ563_KEYS_POLL_INTERVAL        20     /* msecs */
#define WPJ563_KEYS_DEBOUNCE_INTERVAL    (3 * WPJ563_KEYS_POLL_INTERVAL)

#define WPJ563_MAC0_OFFSET               0x10
#define WPJ563_MAC1_OFFSET               0x18
#define WPJ563_WMAC_CALDATA_OFFSET       0x1000

static struct gpio_led WPJ563_leds_gpio[] __initdata = {
	{
		.name		= "wpj563:green:sig1",
		.gpio		= WPJ563_GPIO_LED_SIG1,
		.active_low	= 1,
	},
	{
		.name		= "wpj563:green:sig2",
		.gpio		= WPJ563_GPIO_LED_SIG2,
		.active_low	= 1,
	},
	{
		.name		= "wpj563:green:sig3",
		.gpio		= WPJ563_GPIO_LED_SIG3,
		.active_low	= 1,
	},
	{
		.name		= "wpj563:green:sig4",
		.gpio		= WPJ563_GPIO_LED_SIG4,
		.active_low	= 1,
	},
	{
		.name		= "wpj563:buzzer",
		.gpio		= WPJ563_GPIO_BUZZER,
		.active_low	= 0,
	}
};

static struct gpio_keys_button WPJ563_gpio_keys[] __initdata = {
	{
		.desc		= "Reset button",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = WPJ563_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= WPJ563_GPIO_BTN_RESET,
		.active_low	= 1,
	},
};

static struct ar8327_pad_cfg WPJ563_ar8337_pad0_cfg = {
	.mode = AR8327_PAD_MAC_SGMII,
	.sgmii_delay_en = true,
};

static struct ar8327_platform_data WPJ563_ar8337_data = {
	.pad0_cfg = &WPJ563_ar8337_pad0_cfg,
	.port0_cfg = {
		.force_link = 1,
		.speed = AR8327_PORT_SPEED_1000,
		.duplex = 1,
		.txpause = 1,
		.rxpause = 1,
	},
};

static struct mdio_board_info WPJ563_mdio0_info[] = {
	{
		.bus_id = "ag71xx-mdio.0",
		.phy_addr = 0,
		.platform_data = &WPJ563_ar8337_data,
	},
};

static void __init WPJ563_setup(void)
{
	u8 *art = (u8 *) KSEG1ADDR(0x1fff0000);
	u8 *mac = (u8 *) KSEG1ADDR(0x1f02e000);

	ath79_register_m25p80(NULL);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(WPJ563_leds_gpio),
				 WPJ563_leds_gpio);
	ath79_register_gpio_keys_polled(-1, WPJ563_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(WPJ563_gpio_keys),
					WPJ563_gpio_keys);

	ath79_register_usb();
	
	ath79_register_wmac(art + WPJ563_WMAC_CALDATA_OFFSET, NULL);
	
	ath79_register_pci();

	mdiobus_register_board_info(WPJ563_mdio0_info,
				    ARRAY_SIZE(WPJ563_mdio0_info));
	ath79_register_mdio(0, 0x0);

	ath79_init_mac(ath79_eth0_data.mac_addr, mac + WPJ563_MAC0_OFFSET, 0);
	ath79_init_mac(ath79_eth1_data.mac_addr, mac + WPJ563_MAC1_OFFSET, 0);

	/* GMAC0 is connected to an QCA8334 switch */
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_SGMII;
	ath79_eth0_data.speed = SPEED_1000;
	ath79_eth0_data.duplex = DUPLEX_FULL;
	ath79_eth0_data.phy_mask = BIT(0);
	ath79_eth0_data.mii_bus_dev = &ath79_mdio0_device.dev;

	ath79_register_eth(0);
}

MIPS_MACHINE(ATH79_MACH_WPJ563, "WPJ563", "Compex WPJ563", WPJ563_setup);

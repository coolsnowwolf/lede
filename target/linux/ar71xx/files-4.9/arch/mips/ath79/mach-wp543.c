/*
 *  Compex WP543/WPJ543 board support
 *
 *  Copyright (C) 2008-2012 Gabor Juhos <juhosg@openwrt.org>
 *  Copyright (C) 2008 Imre Kaloz <kaloz@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <asm/mach-ath79/ar71xx_regs.h>
#include <asm/mach-ath79/ath79.h>

#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-usb.h"
#include "machtypes.h"
#include "pci.h"

#define WP543_GPIO_SW6		2
#define WP543_GPIO_LED_1	3
#define WP543_GPIO_LED_2	4
#define WP543_GPIO_LED_WLAN	5
#define WP543_GPIO_LED_CONN	6
#define WP543_GPIO_LED_DIAG	7
#define WP543_GPIO_SW4		8

#define WP543_KEYS_POLL_INTERVAL	20	/* msecs */
#define WP543_KEYS_DEBOUNCE_INTERVAL	(3 * WP543_KEYS_POLL_INTERVAL)

static struct gpio_led wp543_leds_gpio[] __initdata = {
	{
		.name		= "wp543:green:led1",
		.gpio		= WP543_GPIO_LED_1,
		.active_low	= 1,
	}, {
		.name		= "wp543:green:led2",
		.gpio		= WP543_GPIO_LED_2,
		.active_low	= 1,
	}, {
		.name		= "wp543:green:wlan",
		.gpio		= WP543_GPIO_LED_WLAN,
		.active_low	= 1,
	}, {
		.name		= "wp543:green:conn",
		.gpio		= WP543_GPIO_LED_CONN,
		.active_low	= 1,
	}, {
		.name		= "wp543:green:diag",
		.gpio		= WP543_GPIO_LED_DIAG,
		.active_low	= 1,
	}
};

static struct gpio_keys_button wp543_gpio_keys[] __initdata = {
	{
		.desc		= "sw6",
		.type		= EV_KEY,
		.code		= BTN_0,
		.debounce_interval = WP543_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= WP543_GPIO_SW6,
		.active_low	= 1,
	}, {
		.desc		= "sw4",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = WP543_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= WP543_GPIO_SW4,
		.active_low	= 1,
	}
};

static const char *wp543_part_probes[] = {
	"MyLoader",
	NULL,
};

static struct flash_platform_data wp543_flash_data = {
	.part_probes	= wp543_part_probes,
};

static void __init wp543_setup(void)
{
	ath79_register_m25p80(&wp543_flash_data);

	ath79_register_mdio(0, 0xfffffff0);

	ath79_init_mac(ath79_eth0_data.mac_addr, ath79_mac_base, 0);
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ath79_eth0_data.phy_mask = 0x0f;
	ath79_eth0_data.reset_bit = AR71XX_RESET_GE0_MAC |
				    AR71XX_RESET_GE0_PHY;
	ath79_register_eth(0);

	ath79_register_usb();
	ath79_register_pci();

	ath79_register_leds_gpio(-1, ARRAY_SIZE(wp543_leds_gpio),
					wp543_leds_gpio);

	ath79_register_gpio_keys_polled(-1, WP543_KEYS_POLL_INTERVAL,
					 ARRAY_SIZE(wp543_gpio_keys),
					 wp543_gpio_keys);
}

MIPS_MACHINE(ATH79_MACH_WP543, "WP543", "Compex WP543", wp543_setup);

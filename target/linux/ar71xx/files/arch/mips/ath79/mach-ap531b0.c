/*
 * Rockeetech AP531B0 11ng wireless AP board support
 *
 *  Copyright (C) 2013 Gabor Juhos <juhosg@openwrt.org>
 *  Copyright (C) 2016 Shuanglin Liu <roboidler@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/platform_device.h>
#include <linux/ath9k_platform.h>
#include <linux/ar8216_platform.h>

#include <asm/mach-ath79/ar71xx_regs.h>

#include "common.h"
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-spi.h"
#include "dev-usb.h"
#include "dev-wmac.h"
#include "machtypes.h"
#include "pci.h"
#include "dev-ap9x-pci.h"

#define AP531B0_GPIO_LED_WLAN		12
#define AP531B0_GPIO_LED_STATUS		11

#define AP531B0_GPIO_RST_BTN		17

#define AP531B0_KEYS_POLL_INTERVAL	20	/* msecs */
#define AP531B0_KEYS_DEBOUNCE_INTERVAL	(3 * AP531B0_KEYS_POLL_INTERVAL)

#define AP531B0_WMAC_CALDATA_OFFSET	0x1000


static struct gpio_led ap531b0_leds_gpio[] __initdata = {
	{
		.name		= "ap531b0:green:status",
		.gpio		= AP531B0_GPIO_LED_STATUS,
		.active_low	= 1,
	},
	{
		.name		= "ap531b0:green:wlan",
		.gpio		= AP531B0_GPIO_LED_WLAN,
		.active_low	= 1,
	}
};

static struct gpio_keys_button ap531b0_gpio_keys[] __initdata = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = AP531B0_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= AP531B0_GPIO_RST_BTN,
		.active_low	= 1,
	},
};


static void __init ap531b0_gpio_led_setup(void)
{

	ath79_register_leds_gpio(-1, ARRAY_SIZE(ap531b0_leds_gpio),
				 ap531b0_leds_gpio);

	ath79_register_gpio_keys_polled(-1, AP531B0_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(ap531b0_gpio_keys),
					ap531b0_gpio_keys);
}

static void __init ap531b0_setup(void)
{
	u8 *art = (u8 *) KSEG1ADDR(0x1fff0000);
	u8 *pmac;

	ath79_register_m25p80(NULL);
	ap531b0_gpio_led_setup();
	ath79_register_usb();
	ath79_register_pci();

	ath79_register_mdio(0, 0x0);

	pmac = art + AP531B0_WMAC_CALDATA_OFFSET + 2;
	ath79_init_mac(ath79_eth0_data.mac_addr, pmac, 1);
	ath79_init_mac(ath79_eth1_data.mac_addr, pmac, 2);

	ath79_register_wmac(art + AP531B0_WMAC_CALDATA_OFFSET, pmac);

	/* WAN port */
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ath79_eth0_data.speed = SPEED_100;
	ath79_eth0_data.duplex = DUPLEX_FULL;
	ath79_eth0_data.phy_mask = BIT(4);
	ath79_register_eth(0);

	/* LAN ports */
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_GMII;
	ath79_eth1_data.speed = SPEED_1000;
	ath79_eth1_data.duplex = DUPLEX_FULL;
	ath79_switch_data.phy_poll_mask |= BIT(4);
	ath79_switch_data.phy4_mii_en = 1;
	ath79_register_eth(1);
}

MIPS_MACHINE(ATH79_MACH_AP531B0, "AP531B0", "Rockeetech AP531B0",
	     ap531b0_setup);

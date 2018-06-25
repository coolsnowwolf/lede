/*
 *  TP-LINK TL-WR941ND board support
 *
 *  Copyright (C) 2009-2012 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/platform_device.h>

#include <asm/mach-ath79/ath79.h>

#include "dev-dsa.h"
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-wmac.h"
#include "machtypes.h"

#define TL_WR941ND_GPIO_LED_SYSTEM	2
#define TL_WR941ND_GPIO_LED_QSS_RED	4
#define TL_WR941ND_GPIO_LED_QSS_GREEN	5
#define TL_WR941ND_GPIO_LED_WLAN	9

#define TL_WR941ND_GPIO_BTN_RESET	3
#define TL_WR941ND_GPIO_BTN_QSS		7

#define TL_WR941ND_KEYS_POLL_INTERVAL	20	/* msecs */
#define TL_WR941ND_KEYS_DEBOUNCE_INTERVAL (3 * TL_WR941ND_KEYS_POLL_INTERVAL)

static const char *tl_wr941nd_part_probes[] = {
	"tp-link",
	NULL,
};

static struct flash_platform_data tl_wr941nd_flash_data = {
	.part_probes	= tl_wr941nd_part_probes,
};

static struct gpio_led tl_wr941nd_leds_gpio[] __initdata = {
	{
		.name		= "tp-link:green:system",
		.gpio		= TL_WR941ND_GPIO_LED_SYSTEM,
		.active_low	= 1,
	}, {
		.name		= "tp-link:red:qss",
		.gpio		= TL_WR941ND_GPIO_LED_QSS_RED,
	}, {
		.name		= "tp-link:green:qss",
		.gpio		= TL_WR941ND_GPIO_LED_QSS_GREEN,
	}, {
		.name		= "tp-link:green:wlan",
		.gpio		= TL_WR941ND_GPIO_LED_WLAN,
		.active_low	= 1,
	}
};

static struct gpio_keys_button tl_wr941nd_gpio_keys[] __initdata = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = TL_WR941ND_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= TL_WR941ND_GPIO_BTN_RESET,
		.active_low	= 1,
	}, {
		.desc		= "qss",
		.type		= EV_KEY,
		.code		= KEY_WPS_BUTTON,
		.debounce_interval = TL_WR941ND_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= TL_WR941ND_GPIO_BTN_QSS,
		.active_low	= 1,
	}
};

static struct dsa_chip_data tl_wr941nd_dsa_chip = {
	.port_names[0]  = "wan",
	.port_names[1]  = "lan1",
	.port_names[2]  = "lan2",
	.port_names[3]  = "lan3",
	.port_names[4]  = "lan4",
	.port_names[5]  = "cpu",
};

static struct dsa_platform_data tl_wr941nd_dsa_data = {
	.nr_chips	= 1,
	.chip		= &tl_wr941nd_dsa_chip,
};

static void __init tl_wr941nd_setup(void)
{
	u8 *mac = (u8 *) KSEG1ADDR(0x1f01fc00);
	u8 *eeprom = (u8 *) KSEG1ADDR(0x1fff1000);

	ath79_register_mdio(0, 0x0);

	ath79_init_mac(ath79_eth0_data.mac_addr, mac, 0);
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RMII;
	ath79_eth0_data.speed = SPEED_100;
	ath79_eth0_data.duplex = DUPLEX_FULL;

	ath79_register_eth(0);
	ath79_register_dsa(&ath79_eth0_device.dev, &ath79_mdio0_device.dev,
			   &tl_wr941nd_dsa_data);

	ath79_register_m25p80(&tl_wr941nd_flash_data);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(tl_wr941nd_leds_gpio),
				 tl_wr941nd_leds_gpio);

	ath79_register_gpio_keys_polled(-1, TL_WR941ND_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(tl_wr941nd_gpio_keys),
					tl_wr941nd_gpio_keys);
	ath79_register_wmac(eeprom, mac);
}

MIPS_MACHINE(ATH79_MACH_TL_WR941ND, "TL-WR941ND", "TP-LINK TL-WR941ND",
	     tl_wr941nd_setup);

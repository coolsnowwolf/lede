/*
 *  TP-LINK TL-WR1043N/ND board support
 *
 *  Copyright (C) 2009-2012 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/platform_device.h>
#include <linux/rtl8366.h>

#include <asm/mach-ath79/ath79.h>
#include <asm/mach-ath79/ar71xx_regs.h>

#include "dev-eth.h"
#include "dev-m25p80.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-usb.h"
#include "dev-wmac.h"
#include "machtypes.h"

#define TL_WR1043ND_GPIO_LED_USB        1
#define TL_WR1043ND_GPIO_LED_SYSTEM     2
#define TL_WR1043ND_GPIO_LED_QSS        5
#define TL_WR1043ND_GPIO_LED_WLAN       9

#define TL_WR1043ND_GPIO_BTN_RESET      3
#define TL_WR1043ND_GPIO_BTN_QSS        7

#define TL_WR1043ND_GPIO_RTL8366_SDA	18
#define TL_WR1043ND_GPIO_RTL8366_SCK	19

#define TL_WR1043ND_KEYS_POLL_INTERVAL	20	/* msecs */
#define TL_WR1043ND_KEYS_DEBOUNCE_INTERVAL (3 * TL_WR1043ND_KEYS_POLL_INTERVAL)

static const char *tl_wr1043nd_part_probes[] = {
	"tp-link",
	NULL,
};

static struct flash_platform_data tl_wr1043nd_flash_data = {
	.part_probes	= tl_wr1043nd_part_probes,
};

static struct gpio_led tl_wr1043nd_leds_gpio[] __initdata = {
	{
		.name		= "tp-link:green:usb",
		.gpio		= TL_WR1043ND_GPIO_LED_USB,
		.active_low	= 1,
	}, {
		.name		= "tp-link:green:system",
		.gpio		= TL_WR1043ND_GPIO_LED_SYSTEM,
		.active_low	= 1,
	}, {
		.name		= "tp-link:green:qss",
		.gpio		= TL_WR1043ND_GPIO_LED_QSS,
		.active_low	= 0,
	}, {
		.name		= "tp-link:green:wlan",
		.gpio		= TL_WR1043ND_GPIO_LED_WLAN,
		.active_low	= 1,
	}
};

static struct gpio_keys_button tl_wr1043nd_gpio_keys[] __initdata = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = TL_WR1043ND_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= TL_WR1043ND_GPIO_BTN_RESET,
		.active_low	= 1,
	}, {
		.desc		= "qss",
		.type		= EV_KEY,
		.code		= KEY_WPS_BUTTON,
		.debounce_interval = TL_WR1043ND_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= TL_WR1043ND_GPIO_BTN_QSS,
		.active_low	= 1,
	}
};

static void tl_wr1043nd_rtl8366rb_hw_reset(bool active)
{
	if (active)
		ath79_device_reset_set(AR71XX_RESET_GE0_PHY);
	else
		ath79_device_reset_clear(AR71XX_RESET_GE0_PHY);
}

static struct rtl8366_platform_data tl_wr1043nd_rtl8366rb_data = {
	.gpio_sda	= TL_WR1043ND_GPIO_RTL8366_SDA,
	.gpio_sck	= TL_WR1043ND_GPIO_RTL8366_SCK,
	.hw_reset	= tl_wr1043nd_rtl8366rb_hw_reset,
};

static struct platform_device tl_wr1043nd_rtl8366rb_device = {
	.name		= RTL8366RB_DRIVER_NAME,
	.id		= -1,
	.dev = {
		.platform_data	= &tl_wr1043nd_rtl8366rb_data,
	}
};

static void __init tl_wr1043nd_setup(void)
{
	u8 *mac = (u8 *) KSEG1ADDR(0x1f01fc00);
	u8 *eeprom = (u8 *) KSEG1ADDR(0x1fff1000);

	tl_wr1043nd_rtl8366rb_hw_reset(true);

	ath79_init_mac(ath79_eth0_data.mac_addr, mac, 0);
	ath79_eth0_data.mii_bus_dev = &tl_wr1043nd_rtl8366rb_device.dev;
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth0_data.speed = SPEED_1000;
	ath79_eth0_data.duplex = DUPLEX_FULL;
	ath79_eth0_pll_data.pll_1000 = 0x1a000000;

	ath79_register_eth(0);

	ath79_register_usb();

	ath79_register_m25p80(&tl_wr1043nd_flash_data);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(tl_wr1043nd_leds_gpio),
				 tl_wr1043nd_leds_gpio);

	platform_device_register(&tl_wr1043nd_rtl8366rb_device);

	ath79_register_gpio_keys_polled(-1, TL_WR1043ND_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(tl_wr1043nd_gpio_keys),
					tl_wr1043nd_gpio_keys);

	ath79_register_wmac(eeprom, mac);
}

MIPS_MACHINE(ATH79_MACH_TL_WR1043ND, "TL-WR1043ND", "TP-LINK TL-WR1043ND",
	     tl_wr1043nd_setup);

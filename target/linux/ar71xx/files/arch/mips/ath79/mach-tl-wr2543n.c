/*
 *  TP-LINK TL-WR2543N/ND board support
 *
 *  Copyright (C) 2011-2012 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/platform_device.h>
#include <linux/rtl8367.h>

#include <asm/mach-ath79/ath79.h>

#include "dev-eth.h"
#include "dev-ap9x-pci.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-usb.h"
#include "machtypes.h"

#define TL_WR2543N_GPIO_LED_WPS        0
#define TL_WR2543N_GPIO_LED_USB        8

/* The WLAN LEDs use GPIOs on the discrete AR9380 wmac */
#define TL_WR2543N_GPIO_WMAC_LED_WLAN2G 0
#define TL_WR2543N_GPIO_WMAC_LED_WLAN5G 1

#define TL_WR2543N_GPIO_BTN_RESET      11
#define TL_WR2543N_GPIO_BTN_WPS        12

#define TL_WR2543N_GPIO_RTL8367_SDA	1
#define TL_WR2543N_GPIO_RTL8367_SCK	6

#define TL_WR2543N_KEYS_POLL_INTERVAL	20	/* msecs */
#define TL_WR2543N_KEYS_DEBOUNCE_INTERVAL (3 * TL_WR2543N_KEYS_POLL_INTERVAL)

static const char *tl_wr2543n_part_probes[] = {
	"tp-link",
	NULL,
};

static struct flash_platform_data tl_wr2543n_flash_data = {
	.part_probes	= tl_wr2543n_part_probes,
};

static struct gpio_led tl_wr2543n_leds_gpio[] __initdata = {
	{
		.name		= "tp-link:green:usb",
		.gpio		= TL_WR2543N_GPIO_LED_USB,
		.active_low	= 1,
	}, {
		.name		= "tp-link:green:wps",
		.gpio		= TL_WR2543N_GPIO_LED_WPS,
		.active_low	= 1,
	}
};

static struct gpio_led tl_wr2543n_wmac_leds_gpio[] = {
	{
		.name		= "tp-link:green:wlan2g",
		.gpio		= TL_WR2543N_GPIO_WMAC_LED_WLAN2G,
		.active_low	= 1,
	},
	{
		.name		= "tp-link:green:wlan5g",
		.gpio		= TL_WR2543N_GPIO_WMAC_LED_WLAN5G,
		.active_low	= 1,
	},
};

static struct gpio_keys_button tl_wr2543n_gpio_keys[] __initdata = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = TL_WR2543N_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= TL_WR2543N_GPIO_BTN_RESET,
		.active_low	= 1,
	}, {
		.desc		= "wps",
		.type		= EV_KEY,
		.code		= KEY_WPS_BUTTON,
		.debounce_interval = TL_WR2543N_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= TL_WR2543N_GPIO_BTN_WPS,
		.active_low	= 1,
	}
};

static struct rtl8367_extif_config tl_wr2543n_rtl8367_extif0_cfg = {
	.mode = RTL8367_EXTIF_MODE_RGMII,
	.txdelay = 1,
	.rxdelay = 0,
	.ability = {
		.force_mode = 1,
		.txpause = 1,
		.rxpause = 1,
		.link = 1,
		.duplex = 1,
		.speed = RTL8367_PORT_SPEED_1000,
	},
};

static struct rtl8367_platform_data tl_wr2543n_rtl8367_data = {
	.gpio_sda	= TL_WR2543N_GPIO_RTL8367_SDA,
	.gpio_sck	= TL_WR2543N_GPIO_RTL8367_SCK,
	.extif0_cfg	= &tl_wr2543n_rtl8367_extif0_cfg,
};

static struct platform_device tl_wr2543n_rtl8367_device = {
	.name		= RTL8367_DRIVER_NAME,
	.id		= -1,
	.dev = {
		.platform_data	= &tl_wr2543n_rtl8367_data,
	}
};

static void __init tl_wr2543n_setup(void)
{
	u8 *mac = (u8 *) KSEG1ADDR(0x1f01fc00);
	u8 *eeprom = (u8 *) KSEG1ADDR(0x1fff1000);

	ath79_register_m25p80(&tl_wr2543n_flash_data);
	ath79_register_leds_gpio(-1, ARRAY_SIZE(tl_wr2543n_leds_gpio),
				 tl_wr2543n_leds_gpio);
	ath79_register_gpio_keys_polled(-1, TL_WR2543N_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(tl_wr2543n_gpio_keys),
					tl_wr2543n_gpio_keys);
	ath79_register_usb();

	ap9x_pci_setup_wmac_leds(0, tl_wr2543n_wmac_leds_gpio,
				 ARRAY_SIZE(tl_wr2543n_wmac_leds_gpio));
	ap91_pci_init(eeprom, mac);

	ath79_init_mac(ath79_eth0_data.mac_addr, mac, -1);
	ath79_eth0_data.mii_bus_dev = &tl_wr2543n_rtl8367_device.dev;
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth0_data.speed = SPEED_1000;
	ath79_eth0_data.duplex = DUPLEX_FULL;
	ath79_eth0_pll_data.pll_1000 = 0x1a000000;

	ath79_register_eth(0);

	platform_device_register(&tl_wr2543n_rtl8367_device);
}

MIPS_MACHINE(ATH79_MACH_TL_WR2543N, "TL-WR2543N", "TP-LINK TL-WR2543N/ND",
	     tl_wr2543n_setup);

/*
 *  Buffalo WZR-HP-G450G board support
 *
 *  Copyright (C) 2011 Felix Fietkau <nbd@nbd.name>
 *  Copyright (C) 2008-2012 Gabor Juhos <juhosg@openwrt.org>
 *  Copyright (C) 2008 Imre Kaloz <kaloz@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/gpio.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/ath9k_platform.h>

#include <asm/mach-ath79/ath79.h>

#include "dev-eth.h"
#include "dev-m25p80.h"
#include "dev-ap9x-pci.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-usb.h"
#include "machtypes.h"

#define WZRHPG450H_KEYS_POLL_INTERVAL     20      /* msecs */
#define WZRHPG450H_KEYS_DEBOUNCE_INTERVAL (3 * WZRHPG450H_KEYS_POLL_INTERVAL)

static struct mtd_partition wzrhpg450h_partitions[] = {
	{
		.name		= "u-boot",
		.offset		= 0,
		.size		= 0x0040000,
		.mask_flags	= MTD_WRITEABLE,
	}, {
		.name		= "u-boot-env",
		.offset		= 0x0040000,
		.size		= 0x0010000,
	}, {
		.name		= "ART",
		.offset		= 0x0050000,
		.size		= 0x0010000,
		.mask_flags	= MTD_WRITEABLE,
	}, {
		.name		= "firmware",
		.offset		= 0x0060000,
		.size		= 0x1f80000,
	}, {
		.name		= "user_property",
		.offset		= 0x1fe0000,
		.size		= 0x0020000,
	}
};

static struct flash_platform_data wzrhpg450h_flash_data = {
	.parts		= wzrhpg450h_partitions,
	.nr_parts	= ARRAY_SIZE(wzrhpg450h_partitions),
};

static struct gpio_led wzrhpg450h_leds_gpio[] __initdata = {
	{
		.name		= "buffalo:red:diag",
		.gpio		= 14,
		.active_low	= 1,
	},
	{
		.name		= "buffalo:orange:security",
		.gpio		= 13,
		.active_low	= 1,
	},
};


static struct gpio_led wzrhpg450h_wmac_leds_gpio[] = {
	{
		.name		= "buffalo:blue:movie_engine",
		.gpio		= 13,
		.active_low	= 1,
	},
	{
		.name		= "buffalo:green:router",
		.gpio		= 14,
		.active_low	= 1,
	},
	{
		.name		= "buffalo:green:wireless",
		.gpio		= 15,
		.active_low	= 1,
	},
};

static struct gpio_keys_button wzrhpg450h_gpio_keys[] __initdata = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = WZRHPG450H_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= 6,
		.active_low	= 1,
	}, {
		.desc		= "usb",
		.type		= EV_KEY,
		.code		= BTN_2,
		.debounce_interval = WZRHPG450H_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= 1,
		.active_low	= 1,
	}, {
		.desc		= "aoss",
		.type		= EV_KEY,
		.code		= KEY_WPS_BUTTON,
		.debounce_interval = WZRHPG450H_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= 8,
		.active_low	= 1,
	}, {
		.desc		= "movie_engine",
		.type		= EV_KEY,
		.code		= BTN_6,
		.debounce_interval = WZRHPG450H_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= 7,
		.active_low	= 0,
	}, {
		.desc		= "router_off",
		.type		= EV_KEY,
		.code		= BTN_5,
		.debounce_interval = WZRHPG450H_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= 12,
		.active_low	= 0,
	}
};


static void __init wzrhpg450h_init(void)
{
	u8 *ee = (u8 *) KSEG1ADDR(0x1f051000);
	u8 *mac = (u8 *) ee + 2;

	ath79_register_m25p80_multi(&wzrhpg450h_flash_data);

	ath79_register_mdio(0, ~BIT(0));
	ath79_init_mac(ath79_eth0_data.mac_addr, mac, 0);
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth0_data.speed = SPEED_1000;
	ath79_eth0_data.duplex = DUPLEX_FULL;
	ath79_eth0_data.phy_mask = BIT(0);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(wzrhpg450h_leds_gpio),
				 wzrhpg450h_leds_gpio);

	ath79_register_gpio_keys_polled(-1, WZRHPG450H_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(wzrhpg450h_gpio_keys),
					wzrhpg450h_gpio_keys);

	ath79_register_eth(0);

	gpio_request_one(16, GPIOF_OUT_INIT_HIGH | GPIOF_EXPORT_DIR_FIXED,
			 "USB power");
	ath79_register_usb();

	ap91_pci_init(ee, NULL);
	ap9x_pci_get_wmac_data(0)->tx_gain_buffalo = true;
	ap9x_pci_get_wmac_data(1)->tx_gain_buffalo = true;
	ap9x_pci_setup_wmac_leds(0, wzrhpg450h_wmac_leds_gpio,
				 ARRAY_SIZE(wzrhpg450h_wmac_leds_gpio));
}

MIPS_MACHINE(ATH79_MACH_WZR_HP_G450H, "WZR-HP-G450H", "Buffalo WZR-HP-G450H",
	     wzrhpg450h_init);

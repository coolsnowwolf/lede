/*
 *  AVM FRITZ!WLAN Repeater 300E board support
 *
 *  Copyright (C) 2017 Mathias Kresin <dev@kresin.me>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/gpio.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>

#include "dev-ap9x-pci.h"
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "machtypes.h"

#define FRITZ300E_KEYS_POLL_INTERVAL	 	20 /* msecs */
#define FRITZ300E_KEYS_DEBOUNCE_INTERVAL	(3 * FRITZ300E_KEYS_POLL_INTERVAL)

static struct mtd_partition fritz300e_flash_partitions[] = {
	{
		.name		= "urloader",
		.offset		= 0,
		.size		= 0x0020000,
		.mask_flags	= MTD_WRITEABLE,
	}, {
		.name		= "firmware",
		.offset		= 0x0020000,
		.size		= 0x0ee0000,
	}, {
		.name		= "tffs (1)",
		.offset		= 0x0f00000,
		.size		= 0x0080000,
		.mask_flags	= MTD_WRITEABLE,
	}, {
		.name           = "tffs (2)",
		.offset         = 0x0f80000,
		.size           = 0x0080000,
		.mask_flags     = MTD_WRITEABLE,
	}
};

static struct flash_platform_data fritz300e_flash_data = {
	.parts		= fritz300e_flash_partitions,
	.nr_parts	= ARRAY_SIZE(fritz300e_flash_partitions),
};

static struct gpio_led fritz300e_leds_gpio[] __initdata = {
	{
		.name		= "fritz300e:green:power",
		.gpio		= 13,
		.active_low	= 1,
	}, {
		.name		= "fritz300e:green:lan",
		.gpio		= 15,
		.active_low	= 1,
	}, {
		.name		= "fritz300e:green:wlan",
		.gpio		= 16,
		.active_low	= 1,
	}
};

static struct gpio_led fritz300e_wmac_leds_gpio[] = {
	{
		.name		= "fritz300e:green:rssi0",
		.gpio		= 10,
		.active_low	= 1,
	}, {
		.name		= "fritz300e:green:rssi1",
		.gpio		= 4,
		.active_low	= 1,
	}, {
		.name		= "fritz300e:green:rssi2",
		.gpio		= 6,
		.active_low	= 1,
	}, {
		.name		= "fritz300e:green:rssi3",
		.gpio		= 7,
		.active_low	= 1,
	}, {
		.name		= "fritz300e:green:rssi4",
		.gpio		= 5,
		.active_low	= 1,
	}
};

static struct gpio_keys_button fritz300e_gpio_keys[] __initdata = {
	{
		.desc		= "wps",
		.type		= EV_KEY,
		.code		= KEY_WPS_BUTTON,
		.debounce_interval = FRITZ300E_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= 12,
		.active_low	= 1,
	},
};

static void __init fritz300e_setup(void)
{
	/* get the Lantiq PEF7071V phy out of reset */
	gpio_request_one(11, GPIOF_OUT_INIT_HIGH, "phy reset");

	ath79_register_m25p80(&fritz300e_flash_data);

	ath79_register_mdio(0, ~(BIT(0)));

	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth0_data.speed = SPEED_1000;
	ath79_eth0_data.duplex = DUPLEX_FULL;
	ath79_eth0_data.phy_mask = BIT(0);

	ath79_register_eth(0);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(fritz300e_leds_gpio),
				 fritz300e_leds_gpio);
	ath79_register_gpio_keys_polled(-1, FRITZ300E_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(fritz300e_gpio_keys),
					fritz300e_gpio_keys);

	ap9x_pci_setup_wmac_leds(0, fritz300e_wmac_leds_gpio,
				 ARRAY_SIZE(fritz300e_wmac_leds_gpio));
	ap91_pci_init_simple();
}

MIPS_MACHINE(ATH79_MACH_FRITZ300E, "FRITZ300E",
	     "AVM FRITZ!WLAN Repeater 300E", fritz300e_setup);

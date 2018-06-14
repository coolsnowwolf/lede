/*
 *  Buffalo WZR-HP-G300NH2 board support
 *
 *  Copyright (C) 2011 Felix Fietkau <nbd@nbd.name>
 *  Copyright (C) 2011 Mark Deneen <mdeneen@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/gpio.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>

#include <asm/mach-ath79/ath79.h>

#include "dev-ap9x-pci.h"
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-usb.h"
#include "machtypes.h"

#define WZRHPG300NH2_MAC_OFFSET		0x20c
#define WZRHPG300NH2_KEYS_POLL_INTERVAL     20      /* msecs */
#define WZRHPG300NH2_KEYS_DEBOUNCE_INTERVAL (3 * WZRHPG300NH2_KEYS_POLL_INTERVAL)

static struct mtd_partition wzrhpg300nh2_flash_partitions[] = {
	{
		.name		= "u-boot",
		.offset		= 0,
		.size		= 0x0040000,
		.mask_flags	= MTD_WRITEABLE,
	}, {
		.name		= "u-boot-env",
		.offset		= 0x0040000,
		.size		= 0x0010000,
		.mask_flags	= MTD_WRITEABLE,
	}, {
		.name		= "art",
		.offset		= 0x0050000,
		.size		= 0x0010000,
		.mask_flags	= MTD_WRITEABLE,
	}, {
		.name		= "firmware",
		.offset		= 0x0060000,
		.size		= 0x1f90000,
	}, {
		.name		= "user_property",
		.offset		= 0x1ff0000,
		.size		= 0x0010000,
		.mask_flags	= MTD_WRITEABLE,
	}
};

static struct flash_platform_data wzrhpg300nh2_flash_data = {
	.parts          = wzrhpg300nh2_flash_partitions,
	.nr_parts       = ARRAY_SIZE(wzrhpg300nh2_flash_partitions),
};

static struct gpio_led wzrhpg300nh2_leds_gpio[] __initdata = {
	{
		.name		= "buffalo:red:diag",
		.gpio		= 16,
		.active_low	= 1,
	},
};

static struct gpio_led wzrhpg300nh2_wmac_leds_gpio[] = {
	{
		.name           = "buffalo:blue:usb",
		.gpio           = 4,
		.active_low     = 1,
	},
	{
		.name           = "buffalo:green:wireless",
		.gpio           = 5,
		.active_low     = 1,
	},
	{
		.name           = "buffalo:orange:security",
		.gpio           = 6,
		.active_low     = 1,
	},
	{
		.name           = "buffalo:green:router",
		.gpio           = 7,
		.active_low     = 1,
	},
	{
		.name           = "buffalo:blue:movie_engine_on",
		.gpio           = 8,
		.active_low     = 1,
	},
	{
		.name           = "buffalo:blue:movie_engine_off",
		.gpio           = 9,
		.active_low     = 1,
	},
};

/* The AOSS button is wmac gpio 12 */
static struct gpio_keys_button wzrhpg300nh2_gpio_keys[] __initdata = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = WZRHPG300NH2_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= 1,
		.active_low	= 1,
	}, {
		.desc		= "usb",
		.type		= EV_KEY,
		.code		= BTN_2,
		.debounce_interval = WZRHPG300NH2_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= 7,
		.active_low	= 1,
	}, {
		.desc		= "qos",
		.type		= EV_KEY,
		.code		= BTN_3,
		.debounce_interval = WZRHPG300NH2_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= 11,
		.active_low	= 0,
	}, {
		.desc		= "router_on",
		.type		= EV_KEY,
		.code		= BTN_5,
		.debounce_interval = WZRHPG300NH2_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= 8,
		.active_low	= 0,
	},
};

static void __init wzrhpg300nh2_setup(void)
{

	u8 *eeprom = (u8 *)   KSEG1ADDR(0x1f051000);
	u8 *mac0   = eeprom + WZRHPG300NH2_MAC_OFFSET;
	/* There is an eth1 but it is not connected to the switch */

	ath79_register_m25p80_multi(&wzrhpg300nh2_flash_data);

	ath79_init_mac(ath79_eth0_data.mac_addr, mac0, 0);
	ath79_register_mdio(0, ~(BIT(0)));

	ath79_init_mac(ath79_eth0_data.mac_addr, mac0, 0);
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth0_data.speed = SPEED_1000;
	ath79_eth0_data.duplex = DUPLEX_FULL;
	ath79_eth0_data.phy_mask = BIT(0);

	ath79_register_eth(0);

	/* gpio13 is usb power.  Turn it on. */
	gpio_request_one(13, GPIOF_OUT_INIT_HIGH | GPIOF_EXPORT_DIR_FIXED,
			 "USB power");
	ath79_register_usb();

	ath79_register_leds_gpio(-1, ARRAY_SIZE(wzrhpg300nh2_leds_gpio),
				 wzrhpg300nh2_leds_gpio);
	ath79_register_gpio_keys_polled(-1, WZRHPG300NH2_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(wzrhpg300nh2_gpio_keys),
					wzrhpg300nh2_gpio_keys);
	ap9x_pci_setup_wmac_leds(0, wzrhpg300nh2_wmac_leds_gpio,
				ARRAY_SIZE(wzrhpg300nh2_wmac_leds_gpio));

	ap91_pci_init(eeprom, mac0);
}

MIPS_MACHINE(ATH79_MACH_WZR_HP_G300NH2, "WZR-HP-G300NH2",
	     "Buffalo WZR-HP-G300NH2", wzrhpg300nh2_setup);

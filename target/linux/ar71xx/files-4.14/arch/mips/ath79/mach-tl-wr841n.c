/*
 *  TP-LINK TL-WR841N/ND v1 board support
 *
 *  Copyright (C) 2009-2012 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/platform_device.h>

#include <asm/mach-ath79/ath79.h>

#include "dev-dsa.h"
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "machtypes.h"
#include "pci.h"

#define TL_WR841ND_V1_GPIO_LED_SYSTEM		2
#define TL_WR841ND_V1_GPIO_LED_QSS_GREEN	4
#define TL_WR841ND_V1_GPIO_LED_QSS_RED		5

#define TL_WR841ND_V1_GPIO_BTN_RESET		3
#define TL_WR841ND_V1_GPIO_BTN_QSS		7

#define TL_WR841ND_V1_KEYS_POLL_INTERVAL	20	/* msecs */
#define TL_WR841ND_V1_KEYS_DEBOUNCE_INTERVAL \
				(3 * TL_WR841ND_V1_KEYS_POLL_INTERVAL)

static struct mtd_partition tl_wr841n_v1_partitions[] = {
	{
		.name		= "redboot",
		.offset		= 0,
		.size		= 0x020000,
		.mask_flags	= MTD_WRITEABLE,
	}, {
		.name		= "kernel",
		.offset		= 0x020000,
		.size		= 0x140000,
	}, {
		.name		= "rootfs",
		.offset		= 0x160000,
		.size		= 0x280000,
	}, {
		.name		= "config",
		.offset		= 0x3e0000,
		.size		= 0x020000,
		.mask_flags	= MTD_WRITEABLE,
	}, {
		.name		= "firmware",
		.offset		= 0x020000,
		.size		= 0x3c0000,
	}
};

static struct flash_platform_data tl_wr841n_v1_flash_data = {
	.parts		= tl_wr841n_v1_partitions,
	.nr_parts	= ARRAY_SIZE(tl_wr841n_v1_partitions),
};

static struct gpio_led tl_wr841n_v1_leds_gpio[] __initdata = {
	{
		.name		= "tp-link:green:system",
		.gpio		= TL_WR841ND_V1_GPIO_LED_SYSTEM,
		.active_low	= 1,
	}, {
		.name		= "tp-link:red:qss",
		.gpio		= TL_WR841ND_V1_GPIO_LED_QSS_RED,
	}, {
		.name		= "tp-link:green:qss",
		.gpio		= TL_WR841ND_V1_GPIO_LED_QSS_GREEN,
	}
};

static struct gpio_keys_button tl_wr841n_v1_gpio_keys[] __initdata = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = TL_WR841ND_V1_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= TL_WR841ND_V1_GPIO_BTN_RESET,
		.active_low	= 1,
	}, {
		.desc		= "qss",
		.type		= EV_KEY,
		.code		= KEY_WPS_BUTTON,
		.debounce_interval = TL_WR841ND_V1_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= TL_WR841ND_V1_GPIO_BTN_QSS,
		.active_low	= 1,
	}
};

static struct dsa_chip_data tl_wr841n_v1_dsa_chip = {
	.port_names[0]  = "wan",
	.port_names[1]  = "lan1",
	.port_names[2]  = "lan2",
	.port_names[3]  = "lan3",
	.port_names[4]  = "lan4",
	.port_names[5]  = "cpu",
};

static struct dsa_platform_data tl_wr841n_v1_dsa_data = {
	.nr_chips	= 1,
	.chip		= &tl_wr841n_v1_dsa_chip,
};

static void __init tl_wr841n_v1_setup(void)
{
	u8 *mac = (u8 *) KSEG1ADDR(0x1f01fc00);

	ath79_register_mdio(0, 0x0);

	ath79_init_mac(ath79_eth0_data.mac_addr, mac, 0);
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RMII;
	ath79_eth0_data.speed = SPEED_100;
	ath79_eth0_data.duplex = DUPLEX_FULL;

	ath79_register_eth(0);
	ath79_register_dsa(&ath79_eth0_device.dev, &ath79_mdio0_device.dev,
			   &tl_wr841n_v1_dsa_data);

	ath79_register_m25p80(&tl_wr841n_v1_flash_data);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(tl_wr841n_v1_leds_gpio),
				 tl_wr841n_v1_leds_gpio);

	ath79_register_gpio_keys_polled(-1, TL_WR841ND_V1_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(tl_wr841n_v1_gpio_keys),
					tl_wr841n_v1_gpio_keys);
	ath79_register_pci();
}

MIPS_MACHINE(ATH79_MACH_TL_WR841N_V1, "TL-WR841N-v1.5", "TP-LINK TL-WR841N v1",
	     tl_wr841n_v1_setup);

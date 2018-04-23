/*
 * GainStrong Oolite V5.2 module and development board support
 *
 * Copyright (C) 2018 Piotr Dymacz <pepe2k@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 */

#include <linux/gpio.h>
#include <linux/platform_device.h>

#include <asm/mach-ath79/ath79.h>
#include <asm/mach-ath79/ar71xx_regs.h>

#include "common.h"
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-usb.h"
#include "dev-wmac.h"
#include "machtypes.h"
#include "pci.h"

#define GS_OOLITE_V5_2_DEV_GPIO_BTN_RESET	17
#define GS_OOLITE_V5_2_DEV_GPIO_LED_SYSTEM	13

#define GS_KEYS_POLL_INTERVAL		20 /* msec */
#define GS_KEYS_DEBOUNCE_INTERVAL	(3 * GS_KEYS_POLL_INTERVAL)

#define GS_OOLITE_V5_2_WMAC_CALDATA_OFFSET	0x1000

static const char *gs_oolite_v5_2_part_probes[] = {
	"tp-link",
	NULL,
};

static struct flash_platform_data gs_oolite_v5_2_flash_data = {
	.part_probes = gs_oolite_v5_2_part_probes,
};

static struct gpio_led gs_oolite_v5_2_dev_gpio_leds[] __initdata = {
	{
		.name		= "oolite-v5.2-dev:blue:system",
		.gpio		= GS_OOLITE_V5_2_DEV_GPIO_LED_SYSTEM,
		.active_low	= 0,
	},
};

static struct gpio_keys_button gs_oolite_v5_2_dev_gpio_keys[] __initdata = {
	{
		.desc			= "reset",
		.type			= EV_KEY,
		.code			= KEY_RESTART,
		.debounce_interval	= GS_KEYS_DEBOUNCE_INTERVAL,
		.gpio			= GS_OOLITE_V5_2_DEV_GPIO_BTN_RESET,
		.active_low		= 1,
	},
};

static void __init gs_oolite_v5_2_setup(void)
{
	u8 *art = (u8 *) KSEG1ADDR(0x1fff0000);

	ath79_register_m25p80(&gs_oolite_v5_2_flash_data);

	ath79_setup_ar933x_phy4_switch(false, false);

	ath79_register_mdio(0, 0x0);

	ath79_switch_data.phy4_mii_en = 1;
	ath79_switch_data.phy_poll_mask |= BIT(4);

	/* LAN */
	ath79_eth1_data.duplex = DUPLEX_FULL;
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_GMII;
	ath79_init_mac(ath79_eth1_data.mac_addr, art + 6, 0);
	ath79_register_eth(1);

	/* WAN */
	ath79_eth0_data.duplex = DUPLEX_FULL;
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ath79_eth0_data.phy_mask = BIT(4);
	ath79_eth0_data.speed = SPEED_100;
	ath79_init_mac(ath79_eth0_data.mac_addr, art, 0);
	ath79_register_eth(0);

	ath79_register_pci();
	ath79_register_usb();
	ath79_register_wmac(art + GS_OOLITE_V5_2_WMAC_CALDATA_OFFSET, NULL);
}

static void __init gs_oolite_v5_2_dev_setup(void)
{
	gs_oolite_v5_2_setup();

	ath79_register_leds_gpio(-1, ARRAY_SIZE(gs_oolite_v5_2_dev_gpio_leds),
				 gs_oolite_v5_2_dev_gpio_leds);

	ath79_register_gpio_keys_polled(-1, GS_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(gs_oolite_v5_2_dev_gpio_keys),
					gs_oolite_v5_2_dev_gpio_keys);
}

MIPS_MACHINE(ATH79_MACH_GS_OOLITE_V5_2, "OOLITE-V5-2",
	     "GainStrong Oolite V5.2", gs_oolite_v5_2_setup);

MIPS_MACHINE(ATH79_MACH_GS_OOLITE_V5_2_DEV, "OOLITE-V5-2-DEV",
	     "GainStrong Oolite V5.2-Dev", gs_oolite_v5_2_dev_setup);

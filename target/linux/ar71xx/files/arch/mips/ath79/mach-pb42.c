/*
 *  Atheros PB42 board support
 *
 *  Copyright (C) 2008-2012 Gabor Juhos <juhosg@openwrt.org>
 *  Copyright (C) 2008 Imre Kaloz <kaloz@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <asm/mach-ath79/ath79.h>

#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-m25p80.h"
#include "dev-usb.h"
#include "machtypes.h"
#include "pci.h"

#define PB42_KEYS_POLL_INTERVAL		20	/* msecs */
#define PB42_KEYS_DEBOUNCE_INTERVAL	(3 * PB42_KEYS_POLL_INTERVAL)

#define PB42_GPIO_BTN_SW4	8
#define PB42_GPIO_BTN_SW5	3

static struct gpio_keys_button pb42_gpio_keys[] __initdata = {
	{
		.desc		= "sw4",
		.type		= EV_KEY,
		.code		= BTN_0,
		.debounce_interval = PB42_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= PB42_GPIO_BTN_SW4,
		.active_low	= 1,
	}, {
		.desc		= "sw5",
		.type		= EV_KEY,
		.code		= BTN_1,
		.debounce_interval = PB42_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= PB42_GPIO_BTN_SW5,
		.active_low	= 1,
	}
};

static const char *pb42_part_probes[] = {
	"RedBoot",
	NULL,
};

static struct flash_platform_data pb42_flash_data = {
	.part_probes	= pb42_part_probes,
};

#define PB42_WAN_PHYMASK	BIT(20)
#define PB42_LAN_PHYMASK	(BIT(16) | BIT(17) | BIT(18) | BIT(19))
#define PB42_MDIO_PHYMASK	(PB42_LAN_PHYMASK | PB42_WAN_PHYMASK)

static void __init pb42_init(void)
{
	ath79_register_m25p80(&pb42_flash_data);

	ath79_register_mdio(0, ~PB42_MDIO_PHYMASK);

	ath79_init_mac(ath79_eth0_data.mac_addr, ath79_mac_base, 0);
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ath79_eth0_data.phy_mask = PB42_WAN_PHYMASK;

	ath79_init_mac(ath79_eth1_data.mac_addr, ath79_mac_base, 1);
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_RMII;
	ath79_eth1_data.speed = SPEED_100;
	ath79_eth1_data.duplex = DUPLEX_FULL;

	ath79_register_eth(0);
	ath79_register_eth(1);

	ath79_register_gpio_keys_polled(-1, PB42_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(pb42_gpio_keys),
					pb42_gpio_keys);

	ath79_register_pci();
}

MIPS_MACHINE(ATH79_MACH_PB42, "PB42", "Atheros PB42", pb42_init);

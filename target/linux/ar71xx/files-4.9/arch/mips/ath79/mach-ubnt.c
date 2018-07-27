/*
 *  Ubiquiti RouterStation support
 *
 *  Copyright (C) 2008-2012 Gabor Juhos <juhosg@openwrt.org>
 *  Copyright (C) 2008 Imre Kaloz <kaloz@openwrt.org>
 *  Copyright (C) 2008 Ubiquiti <support@ubnt.com>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <asm/mach-ath79/ath79.h>

#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-usb.h"
#include "machtypes.h"
#include "pci.h"

#define UBNT_RS_GPIO_LED_RF	2
#define UBNT_RS_GPIO_SW4	8

#define UBNT_LS_SR71_GPIO_LED_D25	0
#define UBNT_LS_SR71_GPIO_LED_D26	1
#define UBNT_LS_SR71_GPIO_LED_D24	2
#define UBNT_LS_SR71_GPIO_LED_D23	4
#define UBNT_LS_SR71_GPIO_LED_D22	5
#define UBNT_LS_SR71_GPIO_LED_D27	6
#define UBNT_LS_SR71_GPIO_LED_D28	7

#define UBNT_KEYS_POLL_INTERVAL		20	/* msecs */
#define UBNT_KEYS_DEBOUNCE_INTERVAL	(3 * UBNT_KEYS_POLL_INTERVAL)

static struct gpio_led ubnt_rs_leds_gpio[] __initdata = {
	{
		.name		= "ubnt:green:rf",
		.gpio		= UBNT_RS_GPIO_LED_RF,
		.active_low	= 0,
	}
};

static struct gpio_led ubnt_ls_sr71_leds_gpio[] __initdata = {
	{
		.name		= "ubnt:green:d22",
		.gpio		= UBNT_LS_SR71_GPIO_LED_D22,
		.active_low	= 0,
	}, {
		.name		= "ubnt:green:d23",
		.gpio		= UBNT_LS_SR71_GPIO_LED_D23,
		.active_low	= 0,
	}, {
		.name		= "ubnt:green:d24",
		.gpio		= UBNT_LS_SR71_GPIO_LED_D24,
		.active_low	= 0,
	}, {
		.name		= "ubnt:red:d25",
		.gpio		= UBNT_LS_SR71_GPIO_LED_D25,
		.active_low	= 0,
	}, {
		.name		= "ubnt:red:d26",
		.gpio		= UBNT_LS_SR71_GPIO_LED_D26,
		.active_low	= 0,
	}, {
		.name		= "ubnt:green:d27",
		.gpio		= UBNT_LS_SR71_GPIO_LED_D27,
		.active_low	= 0,
	}, {
		.name		= "ubnt:green:d28",
		.gpio		= UBNT_LS_SR71_GPIO_LED_D28,
		.active_low	= 0,
	}
};

static struct gpio_keys_button ubnt_gpio_keys[] __initdata = {
	{
		.desc		= "sw4",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = UBNT_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= UBNT_RS_GPIO_SW4,
		.active_low	= 1,
	}
};

static const char *ubnt_part_probes[] = {
	"RedBoot",
	NULL,
};

static struct flash_platform_data ubnt_flash_data = {
	.part_probes	= ubnt_part_probes,
};

static void __init ubnt_generic_setup(void)
{
	ath79_register_m25p80(&ubnt_flash_data);

	ath79_register_gpio_keys_polled(-1, UBNT_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(ubnt_gpio_keys),
					ubnt_gpio_keys);
	ath79_register_pci();
}

#define UBNT_RS_WAN_PHYMASK	BIT(20)
#define UBNT_RS_LAN_PHYMASK	(BIT(16) | BIT(17) | BIT(18) | BIT(19))

static void __init ubnt_rs_setup(void)
{
	ubnt_generic_setup();

	ath79_register_mdio(0, ~(UBNT_RS_WAN_PHYMASK | UBNT_RS_LAN_PHYMASK));

	ath79_init_mac(ath79_eth0_data.mac_addr, ath79_mac_base, 0);
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ath79_eth0_data.phy_mask = UBNT_RS_WAN_PHYMASK;

	/*
	 * There is Secondary MAC address duplicate problem with some
	 * UBNT HW batches.  Do not increase Secondary MAC address by 1
	 * but do workaround with 'Locally Administrated' bit.
	 */
	ath79_init_local_mac(ath79_eth1_data.mac_addr, ath79_mac_base);
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_RMII;
	ath79_eth1_data.speed = SPEED_100;
	ath79_eth1_data.duplex = DUPLEX_FULL;

	ath79_register_eth(0);
	ath79_register_eth(1);

	ath79_register_usb();

	ath79_register_leds_gpio(-1, ARRAY_SIZE(ubnt_rs_leds_gpio),
				 ubnt_rs_leds_gpio);
}

MIPS_MACHINE(ATH79_MACH_UBNT_RS, "UBNT-RS", "Ubiquiti RouterStation",
	     ubnt_rs_setup);

#define UBNT_RSPRO_WAN_PHYMASK	BIT(4)
#define UBNT_RSPRO_LAN_PHYMASK	(BIT(0) | BIT(1) | BIT(2) | BIT(3))

static void __init ubnt_rspro_setup(void)
{
	ubnt_generic_setup();

	ath79_register_mdio(0, ~(UBNT_RSPRO_WAN_PHYMASK |
				 UBNT_RSPRO_LAN_PHYMASK));

	ath79_init_mac(ath79_eth0_data.mac_addr, ath79_mac_base, 0);
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth0_data.phy_mask = UBNT_RSPRO_WAN_PHYMASK;

	/*
	 * There is Secondary MAC address duplicate problem with some
	 * UBNT HW batches.  Do not increase Secondary MAC address by 1
	 * but do workaround with 'Locally Administrated' bit.
	 */
	ath79_init_local_mac(ath79_eth1_data.mac_addr, ath79_mac_base);
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth1_data.phy_mask = UBNT_RSPRO_LAN_PHYMASK;
	ath79_eth1_data.speed = SPEED_1000;
	ath79_eth1_data.duplex = DUPLEX_FULL;

	ath79_register_eth(0);
	ath79_register_eth(1);

	ath79_register_usb();

	ath79_register_leds_gpio(-1, ARRAY_SIZE(ubnt_rs_leds_gpio),
				 ubnt_rs_leds_gpio);
}

MIPS_MACHINE(ATH79_MACH_UBNT_RSPRO, "UBNT-RSPRO", "Ubiquiti RouterStation Pro",
	     ubnt_rspro_setup);

static void __init ubnt_lsx_setup(void)
{
	ubnt_generic_setup();
}

MIPS_MACHINE(ATH79_MACH_UBNT_LSX, "UBNT-LSX", "Ubiquiti LSX", ubnt_lsx_setup);

#define UBNT_LSSR71_PHY_MASK	BIT(1)

static void __init ubnt_lssr71_setup(void)
{
	ubnt_generic_setup();

	ath79_register_mdio(0, ~UBNT_LSSR71_PHY_MASK);

	ath79_init_mac(ath79_eth0_data.mac_addr, ath79_mac_base, 0);
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ath79_eth0_data.phy_mask = UBNT_LSSR71_PHY_MASK;

	ath79_register_eth(0);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(ubnt_ls_sr71_leds_gpio),
				 ubnt_ls_sr71_leds_gpio);
}

MIPS_MACHINE(ATH79_MACH_UBNT_LSSR71, "UBNT-LS-SR71", "Ubiquiti LS-SR71",
	     ubnt_lssr71_setup);

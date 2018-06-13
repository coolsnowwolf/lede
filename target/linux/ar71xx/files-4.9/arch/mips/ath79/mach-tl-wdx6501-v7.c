/*
 *  TP-LINK TL-WDX6501 v7
 *
 *  Copyright (C) 2015 Weijie Gao <hackpascal@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/pci.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>

#include <asm/mach-ath79/ath79.h>
#include <asm/mach-ath79/ar71xx_regs.h>

#include "common.h"
#include "dev-eth.h"
#include "dev-ap9x-pci.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-usb.h"
#include "dev-wmac.h"
#include "machtypes.h"
#include "pci.h"

#define TL_WDX6501_V7_GPIO_LED_SYS	21
#define TL_WDX6501_V7_GPIO_LED_WAN	18
#define TL_WDX6501_V7_GPIO_LED_LAN1	17
#define TL_WDX6501_V7_GPIO_LED_LAN2	16
#define TL_WDX6501_V7_GPIO_LED_LAN3	15
#define TL_WDX6501_V7_GPIO_LED_LAN4	14

#define TL_WDX6501_V7_GPIO_BTN_RESET	1

#define TL_WDX6501_V7_KEYS_POLL_INTERVAL	20	/* msecs */
#define TL_WDX6501_V7_KEYS_DEBOUNCE_INTERVAL	(3 * TL_WDX6501_V7_KEYS_POLL_INTERVAL)

#define TL_WDX6501_V7_MAC0_OFFSET               0
#define TL_WDX6501_V7_WMAC_CALDATA_OFFSET	0x1000
#define TL_WDX6501_V7_PCIE_CALDATA_OFFSET	0x2000

static const char *tl_wdx6501_v7_part_probes[] = {
	"tp-link-64k",
	NULL,
};

static struct flash_platform_data tl_wdx6501_v7_flash_data = {
	.part_probes	= tl_wdx6501_v7_part_probes,
};

static struct gpio_led tl_wdx6501_v7_leds_gpio[] __initdata = {
	{
		.name		= "tp-link:green:lan1",
		.gpio		= TL_WDX6501_V7_GPIO_LED_LAN1,
		.active_low	= 1,
	}, {
		.name		= "tp-link:green:lan2",
		.gpio		= TL_WDX6501_V7_GPIO_LED_LAN2,
		.active_low	= 1,
	}, {
		.name		= "tp-link:green:lan3",
		.gpio		= TL_WDX6501_V7_GPIO_LED_LAN3,
		.active_low	= 1,
	}, {
		.name		= "tp-link:green:lan4",
		.gpio		= TL_WDX6501_V7_GPIO_LED_LAN4,
		.active_low	= 1,
	}, {
		.name		= "tp-link:green:wan",
		.gpio		= TL_WDX6501_V7_GPIO_LED_WAN,
		.active_low	= 1,
	}, {
		.name		= "tp-link:white:system",
		.gpio		= TL_WDX6501_V7_GPIO_LED_SYS,
		.active_low	= 0,
	},
};

static struct gpio_keys_button tl_wdx6501_v7_gpio_keys[] __initdata = {
	{
		.desc		= "Reset button",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = TL_WDX6501_V7_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= TL_WDX6501_V7_GPIO_BTN_RESET,
		.active_low	= 1,
	}
};


static void __init tl_ap151_setup(void)
{
	u8 *art = (u8 *) KSEG1ADDR(0x1fff0000);

	ath79_register_m25p80(&tl_wdx6501_v7_flash_data);

	ath79_setup_ar933x_phy4_switch(false, false);

	ath79_register_mdio(1, 0x0);

	/* WAN */
	ath79_switch_data.phy4_mii_en = 1;
	ath79_switch_data.phy_poll_mask = BIT(4);
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ath79_eth0_data.phy_mask = BIT(4);
	ath79_eth0_data.mii_bus_dev = &ath79_mdio1_device.dev;
	ath79_init_mac(ath79_eth0_data.mac_addr, art + TL_WDX6501_V7_MAC0_OFFSET, 0);
	ath79_register_eth(0);

	/* LAN */
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_GMII;
	ath79_eth1_data.duplex = DUPLEX_FULL;
	ath79_eth1_data.speed = SPEED_1000;
	ath79_init_mac(ath79_eth1_data.mac_addr, art + TL_WDX6501_V7_MAC0_OFFSET, -1);
	ath79_register_eth(1);

	ath79_register_wmac(art + TL_WDX6501_V7_WMAC_CALDATA_OFFSET, NULL);

	ap91_pci_init(art + TL_WDX6501_V7_PCIE_CALDATA_OFFSET, NULL);
}

static void __init tl_wdx6501_v7_setup(void)
{
	tl_ap151_setup();

	ath79_register_leds_gpio(-1, ARRAY_SIZE(tl_wdx6501_v7_leds_gpio),
				 tl_wdx6501_v7_leds_gpio);

	ath79_register_gpio_keys_polled(1, TL_WDX6501_V7_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(tl_wdx6501_v7_gpio_keys),
					tl_wdx6501_v7_gpio_keys);
}

MIPS_MACHINE(ATH79_MACH_TL_WDX6501_V7, "TL-WDX6501-v7", "TP-LINK TL-WDX6501 v7",
	     tl_wdx6501_v7_setup);

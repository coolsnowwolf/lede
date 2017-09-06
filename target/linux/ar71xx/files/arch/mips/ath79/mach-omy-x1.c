/*
 *  OMYlink OMY-X1 board support
 *
 *  Copyright (C) 2016 L. D. Pinney <ldpinney@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
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
#include "dev-wmac.h"
#include "machtypes.h"



#define OMY_X1_GPIO_LED_POWER	19
#define OMY_X1_GPIO_LED_WAN	22

#define OMY_X1_GPIO_BTN_RESET	17

#define OMY_X1_KEYS_POLL_INTERVAL	20	/* msecs */
#define OMY_X1_KEYS_DEBOUNCE_INTERVAL (3 * OMY_X1_KEYS_POLL_INTERVAL)

static const char *omy_x1_part_probes[] = {
	"tp-link",
	NULL,
};

static struct flash_platform_data omy_x1_flash_data = {
	.part_probes	= omy_x1_part_probes,
};

static struct gpio_led omy_x1_leds_gpio[] __initdata = {
	{
		.name		= "omy:green:wan",
		.gpio		= OMY_X1_GPIO_LED_WAN,
		.active_low	= 1,
	}, {
		.name		= "omy:green:power",
		.gpio		= OMY_X1_GPIO_LED_POWER,
		.active_low	= 1,
	},
};

static struct gpio_keys_button omy_x1_gpio_keys[] __initdata = {
	{
		.desc		= "Reset button",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = OMY_X1_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= OMY_X1_GPIO_BTN_RESET,
		.active_low	= 1,
	}
};

static void __init omy_x1_setup(void)
{
	u8 *mac = (u8 *) KSEG1ADDR(0x1f01fc00);
	u8 *ee = (u8 *) KSEG1ADDR(0x1fff1000);

	ath79_gpio_function_setup(AR934X_GPIO_FUNC_JTAG_DISABLE,
				 AR934X_GPIO_FUNC_CLK_OBS4_EN);

	ath79_register_m25p80(&omy_x1_flash_data);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(omy_x1_leds_gpio),
				omy_x1_leds_gpio);

	ath79_register_gpio_keys_polled(1, OMY_X1_KEYS_POLL_INTERVAL,
				ARRAY_SIZE(omy_x1_gpio_keys),
				omy_x1_gpio_keys);

	ath79_setup_ar934x_eth_cfg(AR934X_ETH_CFG_SW_PHY_SWAP);

	ath79_register_mdio(1, 0x0);

	ath79_init_mac(ath79_eth0_data.mac_addr, mac, -1);
	ath79_init_mac(ath79_eth1_data.mac_addr, mac, 0);

	ath79_switch_data.phy4_mii_en = 1;
	ath79_switch_data.phy_poll_mask = BIT(0);
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ath79_eth0_data.phy_mask = BIT(0);
	ath79_eth0_data.mii_bus_dev = &ath79_mdio1_device.dev;
	ath79_register_eth(0);

	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_GMII;
	ath79_register_eth(1);

	ath79_register_wmac(ee, mac);

}

MIPS_MACHINE(ATH79_MACH_OMY_X1, "OMY-X1", "OMYlink OMY-X1",
	     omy_x1_setup);

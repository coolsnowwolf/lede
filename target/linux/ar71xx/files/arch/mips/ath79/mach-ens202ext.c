/*
 * EnGenius ENS202EXT board support
 *
 * Copyright (C) 2017 Marty Plummer <netz.kernel@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 */

#include <linux/gpio.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/platform_device.h>

#include <asm/mach-ath79/ar71xx_regs.h>
#include <asm/mach-ath79/ath79.h>

#include "common.h"
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-wmac.h"
#include "machtypes.h"
#include "nvram.h"

#define ENS202_GPIO_LED_WLAN4		0
#define ENS202_GPIO_LED_POWER		14
#define ENS202_GPIO_LED_WLAN2		16
#define ENS202_GPIO_LED_WLAN3		17
#define ENS202_GPIO_LED_WLAN1		18

#define ENS202_GPIO_BTN_RESET		1

#define ENS202_KEYS_POLL_INTERVAL	20	/* msecs */
#define ENS202_KEYS_DEBOUNCE_INTERVAL	(3 * ENS202_KEYS_POLL_INTERVAL)

static struct gpio_led ens202_leds_gpio[] __initdata = {
	{
		.name		= "engenius:amber:wlan1",
		.gpio		= ENS202_GPIO_LED_WLAN1,
		.active_low	= 1,
	}, {
		.name		= "engenius:red:wlan2",
		.gpio		= ENS202_GPIO_LED_WLAN2,
		.active_low	= 1,
	}, {
		.name		= "engenius:amber:wlan3",
		.gpio		= ENS202_GPIO_LED_WLAN3,
		.active_low	= 1,
	}, {
		.name		= "engenius:green:wlan4",
		.gpio		= ENS202_GPIO_LED_WLAN4,
		.active_low	= 1,
	}, {
		.name		= "engenius:amber:power",
		.gpio		= ENS202_GPIO_LED_POWER,
		.active_low	= 1,
	}
};

static struct gpio_keys_button ens202_gpio_keys[] __initdata = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = ENS202_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= ENS202_GPIO_BTN_RESET,
		.active_low	= 1,
	}
};

static void __init ens202_setup(void)
{
	const char *nvram = (char *) KSEG1ADDR(0x1f040000);
	u8 mac_buff[6];
	u8 *mac = NULL;
	u8 *art = (u8 *) KSEG1ADDR(0x1fff0000);

	if (ath79_nvram_parse_mac_addr(nvram, 0x10000,
				       "ethaddr=", mac_buff) == 0) {
		ath79_init_mac(ath79_eth0_data.mac_addr, mac_buff, 0);
		ath79_init_mac(ath79_eth1_data.mac_addr, mac_buff, 1);
		mac = mac_buff;
	}

	ath79_gpio_function_enable(AR934X_GPIO_FUNC_JTAG_DISABLE);

	ath79_gpio_output_select(ENS202_GPIO_LED_POWER, AR934X_GPIO_OUT_GPIO);
	ath79_gpio_output_select(ENS202_GPIO_LED_WLAN1, AR934X_GPIO_OUT_GPIO);
	ath79_gpio_output_select(ENS202_GPIO_LED_WLAN2, AR934X_GPIO_OUT_GPIO);
	ath79_gpio_output_select(ENS202_GPIO_LED_WLAN3, AR934X_GPIO_OUT_GPIO);
	ath79_gpio_output_select(ENS202_GPIO_LED_WLAN4, AR934X_GPIO_OUT_GPIO);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(ens202_leds_gpio),
				 ens202_leds_gpio);
	ath79_register_gpio_keys_polled(-1, ENS202_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(ens202_gpio_keys),
					ens202_gpio_keys);

	ath79_register_m25p80(NULL);

	ath79_register_wmac(art + 0x1000, NULL);

	ath79_register_mdio(1, 0);

	ath79_setup_ar934x_eth_cfg(AR934X_ETH_CFG_RGMII_GMAC0 |
				   AR934X_ETH_CFG_SW_ONLY_MODE);
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ath79_eth0_data.phy_mask = BIT(0);
	ath79_eth0_data.mii_bus_dev = &ath79_mdio1_device.dev;
	ath79_eth0_pll_data.pll_1000 = 0x06000000;

	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_GMII;
	ath79_eth1_data.speed = SPEED_1000;
	ath79_eth1_data.duplex = DUPLEX_FULL;

	ath79_register_eth(0);
	ath79_register_eth(1);
}

MIPS_MACHINE(ATH79_MACH_ENS202EXT, "ENS202EXT", "EnGenius ENS202EXT",
	     ens202_setup);

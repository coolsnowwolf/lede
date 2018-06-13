/*
 *  D-Link DIR-600 rev. A1 board support
 *
 *  Copyright (C) 2010-2012 Gabor Juhos <juhosg@openwrt.org>
 *  Copyright (C) 2012 Vadim Girlin <vadimgirlin@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <asm/mach-ath79/ath79.h>
#include <asm/mach-ath79/ar71xx_regs.h>

#include "common.h"
#include "dev-ap9x-pci.h"
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "machtypes.h"
#include "nvram.h"

#define DIR_600_A1_GPIO_LED_WPS			0
#define DIR_600_A1_GPIO_LED_POWER_AMBER		1
#define DIR_600_A1_GPIO_LED_POWER_GREEN		6
#define DIR_600_A1_GPIO_LED_LAN1		13
#define DIR_600_A1_GPIO_LED_LAN2		14
#define DIR_600_A1_GPIO_LED_LAN3		15
#define DIR_600_A1_GPIO_LED_LAN4		16
#define DIR_600_A1_GPIO_LED_WAN_AMBER		7
#define DIR_600_A1_GPIO_LED_WAN_GREEN		17

#define DIR_600_A1_GPIO_BTN_RESET		8
#define DIR_600_A1_GPIO_BTN_WPS			12

#define DIR_600_A1_KEYS_POLL_INTERVAL		20	/* msecs */
#define DIR_600_A1_KEYS_DEBOUNCE_INTERVAL (3 * DIR_600_A1_KEYS_POLL_INTERVAL)

#define DIR_600_A1_NVRAM_ADDR	0x1f030000
#define DIR_600_A1_NVRAM_SIZE	0x10000

static struct gpio_led dir_600_a1_leds_gpio[] __initdata = {
	{
		.name		= "d-link:green:power",
		.gpio		= DIR_600_A1_GPIO_LED_POWER_GREEN,
	}, {
		.name		= "d-link:amber:power",
		.gpio		= DIR_600_A1_GPIO_LED_POWER_AMBER,
	}, {
		.name		= "d-link:amber:wan",
		.gpio		= DIR_600_A1_GPIO_LED_WAN_AMBER,
	}, {
		.name		= "d-link:green:wan",
		.gpio		= DIR_600_A1_GPIO_LED_WAN_GREEN,
		.active_low	= 1,
	}, {
		.name		= "d-link:green:lan1",
		.gpio		= DIR_600_A1_GPIO_LED_LAN1,
		.active_low	= 1,
	}, {
		.name		= "d-link:green:lan2",
		.gpio		= DIR_600_A1_GPIO_LED_LAN2,
		.active_low	= 1,
	}, {
		.name		= "d-link:green:lan3",
		.gpio		= DIR_600_A1_GPIO_LED_LAN3,
		.active_low	= 1,
	}, {
		.name		= "d-link:green:lan4",
		.gpio		= DIR_600_A1_GPIO_LED_LAN4,
		.active_low	= 1,
	}, {
		.name		= "d-link:blue:wps",
		.gpio		= DIR_600_A1_GPIO_LED_WPS,
		.active_low	= 1,
	}
};

static struct gpio_keys_button dir_600_a1_gpio_keys[] __initdata = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = DIR_600_A1_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= DIR_600_A1_GPIO_BTN_RESET,
		.active_low	= 1,
	}, {
		.desc		= "wps",
		.type		= EV_KEY,
		.code		= KEY_WPS_BUTTON,
		.debounce_interval = DIR_600_A1_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= DIR_600_A1_GPIO_BTN_WPS,
		.active_low	= 1,
	}
};

static void __init dir_600_a1_setup(void)
{
	const char *nvram = (char *) KSEG1ADDR(DIR_600_A1_NVRAM_ADDR);
	u8 *ee = (u8 *) KSEG1ADDR(0x1fff1000);
	u8 mac_buff[6];
	u8 *mac = NULL;

	if (ath79_nvram_parse_mac_addr(nvram, DIR_600_A1_NVRAM_SIZE,
				       "lan_mac=", mac_buff) == 0) {
		ath79_init_mac(ath79_eth0_data.mac_addr, mac_buff, 0);
		ath79_init_mac(ath79_eth1_data.mac_addr, mac_buff, 1);
		mac = mac_buff;
	}

	ath79_register_m25p80(NULL);

	ath79_gpio_function_disable(AR724X_GPIO_FUNC_ETH_SWITCH_LED0_EN |
				    AR724X_GPIO_FUNC_ETH_SWITCH_LED1_EN |
				    AR724X_GPIO_FUNC_ETH_SWITCH_LED2_EN |
				    AR724X_GPIO_FUNC_ETH_SWITCH_LED3_EN |
				    AR724X_GPIO_FUNC_ETH_SWITCH_LED4_EN);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(dir_600_a1_leds_gpio),
				 dir_600_a1_leds_gpio);

	ath79_register_gpio_keys_polled(-1, DIR_600_A1_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(dir_600_a1_gpio_keys),
					dir_600_a1_gpio_keys);

	ath79_init_mac(ath79_eth0_data.mac_addr, mac, 0);
	ath79_init_mac(ath79_eth1_data.mac_addr, mac, 1);

	ath79_register_mdio(0, 0x0);

	/* LAN ports */
	ath79_register_eth(1);

	/* WAN port */
	ath79_register_eth(0);

	ap91_pci_init(ee, mac);
}

MIPS_MACHINE(ATH79_MACH_DIR_600_A1, "DIR-600-A1", "D-Link DIR-600 rev. A1",
	     dir_600_a1_setup);

MIPS_MACHINE(ATH79_MACH_EBR_2310_C1, "EBR-2310-C1", "D-Link EBR-2310 rev. C1",
	     dir_600_a1_setup);

static void __init dir_615_e1_setup(void)
{
	dir_600_a1_setup();
}

MIPS_MACHINE(ATH79_MACH_DIR_615_E1, "DIR-615-E1", "D-Link DIR-615 rev. E1",
	     dir_615_e1_setup);

static void __init dir_615_e4_setup(void)
{
	dir_600_a1_setup();
	ap9x_pci_setup_wmac_led_pin(0, 1);
}

MIPS_MACHINE(ATH79_MACH_DIR_615_E4, "DIR-615-E4", "D-Link DIR-615 rev. E4",
	     dir_615_e4_setup);

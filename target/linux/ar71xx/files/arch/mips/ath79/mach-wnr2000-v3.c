/*
 *  NETGEAR WNR2000v3/WNR612v2/WNR1000v2/WPN824N board support
 *
 *  Copyright (C) 2015 Hartmut Knaack <knaack.h@gmx.de>
 *  Copyright (C) 2013 Mathieu Olivari <mathieu.olivari@gmail.com>
 *  Copyright (C) 2008-2009 Gabor Juhos <juhosg@openwrt.org>
 *  Copyright (C) 2008 Imre Kaloz <kaloz@openwrt.org>
 *  Copyright (C) 2008-2009 Andy Boyett <agb@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/kernel.h> /* for max() macro */
#include <linux/platform_device.h> /* PLATFORM_DEVID_AUTO is defined here */

#include <asm/mach-ath79/ath79.h>
#include <asm/mach-ath79/ar71xx_regs.h> /* needed to disable switch LEDs */
#include "common.h" /* needed to disable switch LEDs */

#include "dev-ap9x-pci.h"
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "machtypes.h"

/* WNR2000v3 - connected through AR7241 */
#define WNR2000V3_GPIO_LED_WAN_GREEN	0
#define WNR2000V3_GPIO_LED_LAN1_AMBER	1
#define WNR2000V3_GPIO_LED_LAN2_AMBER	6
#define WNR2000V3_GPIO_LED_WPS_GREEN	7
#define WNR2000V3_GPIO_LED_LAN3_AMBER	8
#define WNR2000V3_GPIO_BTN_WPS		11
#define WNR2000V3_GPIO_LED_LAN4_AMBER	12
#define WNR2000V3_GPIO_LED_LAN1_GREEN	13
#define WNR2000V3_GPIO_LED_LAN2_GREEN	14
#define WNR2000V3_GPIO_LED_LAN3_GREEN	15
#define WNR2000V3_GPIO_LED_LAN4_GREEN	16
#define WNR2000V3_GPIO_LED_WAN_AMBER	17

/* WNR2000v3 - connected through AR9287 */
#define WNR2000V3_GPIO_WMAC_LED_WLAN_BLUE	1
#define WNR2000V3_GPIO_WMAC_LED_TEST_AMBER	2
#define WNR2000V3_GPIO_WMAC_LED_POWER_GREEN	3
#define WNR2000V3_GPIO_WMAC_BTN_RESET		8
#define WNR2000V3_GPIO_WMAC_BTN_RFKILL		9

/* WNR612v2 - connected through AR7241 */
#define WNR612V2_GPIO_LED_POWER_GREEN	11
#define WNR612V2_GPIO_LED_LAN1_GREEN	13
#define WNR612V2_GPIO_LED_LAN2_GREEN	14
#define WNR612V2_GPIO_LED_WAN_GREEN	17

/* WNR612v2 - connected through AR9285 */
#define WNR612V2_GPIO_WMAC_LED_WLAN_GREEN	1
#define WNR612V2_GPIO_WMAC_BTN_RESET		7

/* WNR1000v2 - connected through AR7240 */
#define WNR1000V2_GPIO_LED_WAN_AMBER	0
#define WNR1000V2_GPIO_LED_TEST_AMBER	1
#define WNR1000V2_GPIO_LED_LAN1_AMBER	6 /* AR724X_GPIO_FUNC_JTAG_DISABLE */
#define WNR1000V2_GPIO_LED_LAN2_AMBER	7 /* AR724X_GPIO_FUNC_JTAG_DISABLE */
#define WNR1000V2_GPIO_LED_LAN3_AMBER	8 /* AR724X_GPIO_FUNC_JTAG_DISABLE */
#define WNR1000V2_GPIO_LED_POWER_GREEN	11
#define WNR1000V2_GPIO_LED_LAN4_AMBER	12
#define WNR1000V2_GPIO_LED_LAN1_GREEN	13 /* AR724X_..._ETH_SWITCH_LED0 */
#define WNR1000V2_GPIO_LED_LAN2_GREEN	14 /* AR724X_..._ETH_SWITCH_LED1 */
#define WNR1000V2_GPIO_LED_LAN3_GREEN	15 /* AR724X_..._ETH_SWITCH_LED2 */
#define WNR1000V2_GPIO_LED_LAN4_GREEN	16 /* AR724X_..._ETH_SWITCH_LED3 */
#define WNR1000V2_GPIO_LED_WAN_GREEN	17 /* AR724X_..._ETH_SWITCH_LED4 */

/* WNR1000v2 - connected through AR9285 */
#define WNR1000V2_GPIO_WMAC_LED_WLAN_BLUE	1
#define WNR1000V2_GPIO_WMAC_LED_WPS_GREEN	5
#define WNR1000V2_GPIO_WMAC_BTN_WPS		6
#define WNR1000V2_GPIO_WMAC_BTN_RESET		7
#define WNR1000V2_GPIO_WMAC_BTN_RFKILL		8

/* WPN824N - connected through AR7240 */
#define WPN824N_GPIO_LED_WAN_AMBER	0
#define WPN824N_GPIO_LED_STATUS_AMBER	1
#define WPN824N_GPIO_LED_LAN1_AMBER	6 /* AR724X_GPIO_FUNC_JTAG_DISABLE */
#define WPN824N_GPIO_LED_LAN2_AMBER	7 /* AR724X_GPIO_FUNC_JTAG_DISABLE */
#define WPN824N_GPIO_LED_LAN3_AMBER	8 /* AR724X_GPIO_FUNC_JTAG_DISABLE */
#define WPN824N_GPIO_LED_LAN4_AMBER	12
#define WPN824N_GPIO_LED_LAN1_GREEN	13
#define WPN824N_GPIO_LED_LAN2_GREEN	14
#define WPN824N_GPIO_LED_LAN3_GREEN	15 /* AR724X_GPIO_FUNC_CLK_OBS3_EN */
#define WPN824N_GPIO_LED_LAN4_GREEN	16
#define WPN824N_GPIO_LED_WAN_GREEN	17

/* WPN824N - connected through AR9285 */
#define WPN824N_WGPIO_LED_PWR_GREEN	0
#define WPN824N_WGPIO_LED_WLAN_BLUE	1
#define WPN824N_WGPIO_LED_WPS1_BLUE	5
#define WPN824N_WGPIO_LED_WPS2_BLUE	9
#define WPN824N_WGPIO_LED_TEST_AMBER	10
#define WPN824N_WGPIO_BTN_WPS		6
#define WPN824N_WGPIO_BTN_RESET		7
#define WPN824N_WGPIO_BTN_WLAN		8

#define WNR2000V3_KEYS_POLL_INTERVAL	20	/* msecs */
#define WNR2000V3_KEYS_DEBOUNCE_INTERVAL	(3 * WNR2000V3_KEYS_POLL_INTERVAL)

/* ART offsets for: WNR2000v3, WNR612v2, WNR1000v2 */
#define WNR2000V3_MAC0_OFFSET		0
#define WNR2000V3_MAC1_OFFSET		6
#define WNR2000V3_PCIE_CALDATA_OFFSET	0x1000
#define WNR2000V3_WMAC_OFFSET		0x108c	/* wireless MAC is inside ART */

static struct gpio_led wnr2000v3_leds_gpio[] __initdata = {
	{
		.name		= "netgear:green:wan",
		.gpio		= WNR2000V3_GPIO_LED_WAN_GREEN,
		.active_low	= 1,
	}, {
		.name		= "netgear:amber:lan1",
		.gpio		= WNR2000V3_GPIO_LED_LAN1_AMBER,
		.active_low	= 1,
	}, {
		.name		= "netgear:amber:lan2",
		.gpio		= WNR2000V3_GPIO_LED_LAN2_AMBER,
		.active_low	= 1,
	}, {
		.name		= "netgear:amber:lan3",
		.gpio		= WNR2000V3_GPIO_LED_LAN3_AMBER,
		.active_low	= 1,
	}, {
		.name		= "netgear:amber:lan4",
		.gpio		= WNR2000V3_GPIO_LED_LAN4_AMBER,
		.active_low	= 1,
	}, {
		.name		= "netgear:green:wps",
		.gpio		= WNR2000V3_GPIO_LED_WPS_GREEN,
		.active_low	= 1,
	}, {
		.name		= "netgear:green:lan1",
		.gpio		= WNR2000V3_GPIO_LED_LAN1_GREEN,
		.active_low	= 1,
	}, {
		.name		= "netgear:green:lan2",
		.gpio		= WNR2000V3_GPIO_LED_LAN2_GREEN,
		.active_low	= 1,
	}, {
		.name		= "netgear:green:lan3",
		.gpio		= WNR2000V3_GPIO_LED_LAN3_GREEN,
		.active_low	= 1,
	}, {
		.name		= "netgear:green:lan4",
		.gpio		= WNR2000V3_GPIO_LED_LAN4_GREEN,
		.active_low	= 1,
	}, {
		.name		= "netgear:amber:wan",
		.gpio		= WNR2000V3_GPIO_LED_WAN_AMBER,
		.active_low	= 1,
	}
};

static struct gpio_led wnr2000v3_wmac_leds_gpio[] = {
	{
		.name		= "netgear:green:power",
		.gpio		= WNR2000V3_GPIO_WMAC_LED_POWER_GREEN,
		.active_low	= 1,
		.default_state	= LEDS_GPIO_DEFSTATE_ON,
	}, {
		.name		= "netgear:amber:test",
		.gpio		= WNR2000V3_GPIO_WMAC_LED_TEST_AMBER,
		.active_low	= 1,
	}, {
		.name		= "netgear:blue:wlan",
		.gpio		= WNR2000V3_GPIO_WMAC_LED_WLAN_BLUE,
		.active_low	= 1,
	}
};

static struct gpio_led wnr612v2_leds_gpio[] __initdata = {
	{
		.name		= "netgear:green:power",
		.gpio		= WNR612V2_GPIO_LED_POWER_GREEN,
		.active_low	= 1,
	}, {
		.name		= "netgear:green:lan1",
		.gpio		= WNR612V2_GPIO_LED_LAN1_GREEN,
		.active_low	= 1,
	}, {
		.name		= "netgear:green:lan2",
		.gpio		= WNR612V2_GPIO_LED_LAN2_GREEN,
		.active_low	= 1,
	}, {
		.name		= "netgear:green:wan",
		.gpio		= WNR612V2_GPIO_LED_WAN_GREEN,
		.active_low	= 1,
	}
};

static struct gpio_led wnr612v2_wmac_leds_gpio[] = {
	{
		.name		= "netgear:green:wlan",
		.gpio		= WNR612V2_GPIO_WMAC_LED_WLAN_GREEN,
		.active_low	= 1,
	}
};

static struct gpio_led wnr1000v2_leds_gpio[] __initdata = {
	{
		.name		= "netgear:amber:lan1",
		.gpio		= WNR1000V2_GPIO_LED_LAN1_AMBER,
		.active_low	= 1,
	}, {
		.name		= "netgear:amber:lan2",
		.gpio		= WNR1000V2_GPIO_LED_LAN2_AMBER,
		.active_low	= 1,
	}, {
		.name		= "netgear:amber:lan3",
		.gpio		= WNR1000V2_GPIO_LED_LAN3_AMBER,
		.active_low	= 1,
	}, {
		.name		= "netgear:amber:lan4",
		.gpio		= WNR1000V2_GPIO_LED_LAN4_AMBER,
		.active_low	= 1,
	}, {
		.name		= "netgear:amber:test",
		.gpio		= WNR1000V2_GPIO_LED_TEST_AMBER,
		.active_low	= 1,
	}, {
		.name		= "netgear:amber:wan",
		.gpio		= WNR1000V2_GPIO_LED_WAN_AMBER,
		.active_low	= 1,
	}, {
		.name		= "netgear:green:lan1",
		.gpio		= WNR1000V2_GPIO_LED_LAN1_GREEN,
		.active_low	= 1,
	}, {
		.name		= "netgear:green:lan2",
		.gpio		= WNR1000V2_GPIO_LED_LAN2_GREEN,
		.active_low	= 1,
	}, {
		.name		= "netgear:green:lan3",
		.gpio		= WNR1000V2_GPIO_LED_LAN3_GREEN,
		.active_low	= 1,
	}, {
		.name		= "netgear:green:lan4",
		.gpio		= WNR1000V2_GPIO_LED_LAN4_GREEN,
		.active_low	= 1,
	}, {
		.name		= "netgear:green:power",
		.gpio		= WNR1000V2_GPIO_LED_POWER_GREEN,
		.active_low	= 1,
	}, {
		.name		= "netgear:green:wan",
		.gpio		= WNR1000V2_GPIO_LED_WAN_GREEN,
		.active_low	= 1,
	}
};

static struct gpio_led wnr1000v2_wmac_leds_gpio[] = {
	{
		.name		= "netgear:green:wps",
		.gpio		= WNR1000V2_GPIO_WMAC_LED_WPS_GREEN,
		.active_low	= 1,
	}, {
		.name		= "netgear:blue:wlan",
		.gpio		= WNR1000V2_GPIO_WMAC_LED_WLAN_BLUE,
		.active_low	= 1,
	}
};

static struct gpio_led wpn824n_leds_gpio[] __initdata = {
	{
		.name		= "netgear:amber:wan",
		.gpio		= WPN824N_GPIO_LED_WAN_AMBER,
		.active_low	= 1,
	}, {
		.name		= "netgear:amber:status",
		.gpio		= WPN824N_GPIO_LED_STATUS_AMBER,
		.active_low	= 1,
	}, {
		.name		= "netgear:amber:lan1",
		.gpio		= WPN824N_GPIO_LED_LAN1_AMBER,
		.active_low	= 1,
	}, {
		.name		= "netgear:amber:lan2",
		.gpio		= WPN824N_GPIO_LED_LAN2_AMBER,
		.active_low	= 1,
	}, {
		.name		= "netgear:amber:lan3",
		.gpio		= WPN824N_GPIO_LED_LAN3_AMBER,
		.active_low	= 1,
	}, {
		.name		= "netgear:amber:lan4",
		.gpio		= WPN824N_GPIO_LED_LAN4_AMBER,
		.active_low	= 1,
	}, {
		.name		= "netgear:green:lan1",
		.gpio		= WPN824N_GPIO_LED_LAN1_GREEN,
		.active_low	= 1,
	}, {
		.name		= "netgear:green:lan2",
		.gpio		= WPN824N_GPIO_LED_LAN2_GREEN,
		.active_low	= 1,
	}, {
		.name		= "netgear:green:lan3",
		.gpio		= WPN824N_GPIO_LED_LAN3_GREEN,
		.active_low	= 1,
	}, {
		.name		= "netgear:green:lan4",
		.gpio		= WPN824N_GPIO_LED_LAN4_GREEN,
		.active_low	= 1,
	}, {
		.name		= "netgear:green:wan",
		.gpio		= WPN824N_GPIO_LED_WAN_GREEN,
		.active_low	= 1,
	}
};

static struct gpio_led wpn824n_wmac_leds_gpio[] = {
	{
		.name		= "netgear:green:power",
		.gpio		= WPN824N_WGPIO_LED_PWR_GREEN,
		.active_low	= 1,
	}, {
		.name		= "netgear:blue:wps1",
		.gpio		= WPN824N_WGPIO_LED_WPS1_BLUE,
		.active_low	= 1,
	}, {
		.name		= "netgear:blue:wps2",
		.gpio		= WPN824N_WGPIO_LED_WPS2_BLUE,
		.active_low	= 1,
	}, {
		.name		= "netgear:amber:test",
		.gpio		= WPN824N_WGPIO_LED_TEST_AMBER,
		.active_low	= 1,
	}, {
		.name		= "netgear:blue:wlan",
		.gpio		= WPN824N_WGPIO_LED_WLAN_BLUE,
		.active_low	= 1,
	}
};

static struct gpio_keys_button wnr2000v3_keys_gpio[] __initdata = {
	{
		.desc		= "wps",
		.type		= EV_KEY,
		.code		= KEY_WPS_BUTTON,
		.debounce_interval = WNR2000V3_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= WNR2000V3_GPIO_BTN_WPS,
		.active_low	= 1,
	}
};

static struct gpio_keys_button wnr2000v3_wmac_keys_gpio[] = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = WNR2000V3_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= WNR2000V3_GPIO_WMAC_BTN_RESET,
		.active_low	= 1,
	}, {
		.desc		= "rfkill",
		.type		= EV_KEY,
		.code		= KEY_RFKILL,
		.debounce_interval = WNR2000V3_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= WNR2000V3_GPIO_WMAC_BTN_RFKILL,
		.active_low	= 1,
	}
};

static struct gpio_keys_button wnr612v2_wmac_keys_gpio[] = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = WNR2000V3_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= WNR612V2_GPIO_WMAC_BTN_RESET,
		.active_low	= 1,
	}
};

static struct gpio_keys_button wnr1000v2_wmac_keys_gpio[] = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = WNR2000V3_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= WNR1000V2_GPIO_WMAC_BTN_RESET,
		.active_low	= 1,
	}, {
		.desc		= "rfkill",
		.type		= EV_KEY,
		.code		= KEY_RFKILL,
		.debounce_interval = WNR2000V3_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= WNR1000V2_GPIO_WMAC_BTN_RFKILL,
		.active_low	= 1,
	}, {
		.desc		= "wps",
		.type		= EV_KEY,
		.code		= KEY_WPS_BUTTON,
		.debounce_interval = WNR2000V3_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= WNR1000V2_GPIO_WMAC_BTN_WPS,
		.active_low	= 1,
	}
};

static struct gpio_keys_button wpn824n_wmac_keys_gpio[] = {
	{
		.desc			= "reset",
		.type			= EV_KEY,
		.code			= KEY_RESTART,
		.debounce_interval	= WNR2000V3_KEYS_DEBOUNCE_INTERVAL,
		.gpio			= WPN824N_WGPIO_BTN_RESET,
		.active_low		= 1,
	}, {
		.desc			= "rfkill",
		.type			= EV_KEY,
		.code			= KEY_RFKILL,
		.debounce_interval	= WNR2000V3_KEYS_DEBOUNCE_INTERVAL,
		.gpio			= WPN824N_WGPIO_BTN_WLAN,
		.active_low		= 1,
	}, {
		.desc			= "wps",
		.type			= EV_KEY,
		.code			= KEY_WPS_BUTTON,
		.debounce_interval	= WNR2000V3_KEYS_DEBOUNCE_INTERVAL,
		.gpio			= WPN824N_WGPIO_BTN_WPS,
		.active_low		= 1,
	}
};

/*
 * For WNR2000v3 ART flash area used for WLAN MAC is usually empty (0xff)
 * so ath9k driver uses random MAC instead each time module is loaded.
 * To fix that, assign permanent WLAN MAC equal to ethN's MAC plus 1,
 * so network interfaces get sequential addresses.
 * If ART wireless MAC address field has been filled by user, use it.
 */
static void __init wnr_get_wmac(u8 *wmac_gen_addr, int mac0_art_offset,
				int mac1_art_offset, int wmac_art_offset)
{
	u8 *art = (u8 *) KSEG1ADDR(0x1fff0000);
	u8 *eth0_mac_addr = (u8 *) (art + mac0_art_offset);
	u8 *eth1_mac_addr = (u8 *) (art + mac1_art_offset);
	u8 *wlan_mac_addr = (u8 *) (art + wmac_art_offset);

	/* only 0xff if all bits are set - address is invalid, empty area */
	if ((wlan_mac_addr[0] & wlan_mac_addr[1] & wlan_mac_addr[2] &
	     wlan_mac_addr[3] & wlan_mac_addr[4] & wlan_mac_addr[5]) == 0xff) {
		memcpy(wmac_gen_addr, eth0_mac_addr, 5);
		wmac_gen_addr[5] = max(eth0_mac_addr[5], eth1_mac_addr[5]) + 1;

		/* Avoid potential conflict in case max(0xff,0x00)+1==0x00 */
		if (!wmac_gen_addr[5])
			wmac_gen_addr[5] = 1;
	} else
		memcpy(wmac_gen_addr, wlan_mac_addr, 6);
}

static void __init wnr_common_setup(u8 *wmac_addr)
{
	u8 *art = (u8 *) KSEG1ADDR(0x1fff0000);

	ath79_register_mdio(0, 0x0);

	ath79_init_mac(ath79_eth0_data.mac_addr, art+WNR2000V3_MAC0_OFFSET, 0);
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RMII;
	ath79_eth0_data.speed = SPEED_100;
	ath79_eth0_data.duplex = DUPLEX_FULL;

	ath79_init_mac(ath79_eth1_data.mac_addr, art+WNR2000V3_MAC1_OFFSET, 0);
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_RMII;
	ath79_eth1_data.phy_mask = 0x10;

	ath79_register_eth(0);
	ath79_register_eth(1);

	ath79_register_m25p80(NULL);
	ap91_pci_init(art + WNR2000V3_PCIE_CALDATA_OFFSET, wmac_addr);
}

static void __init wnr2000v3_setup(void)
{
	u8 wlan_mac_addr[6];

	/*
	 * Disable JTAG to use all AR724X GPIO LEDs.
	 * Also disable CLKs and bit 20 as u-boot does.
	 * Finally, allow OS to control all link LEDs.
	 */
	ath79_gpio_function_setup(AR724X_GPIO_FUNC_JTAG_DISABLE |
				  AR724X_GPIO_FUNC_UART_EN,
				  AR724X_GPIO_FUNC_CLK_OBS1_EN |
				  AR724X_GPIO_FUNC_CLK_OBS2_EN |
				  AR724X_GPIO_FUNC_CLK_OBS3_EN |
				  AR724X_GPIO_FUNC_CLK_OBS4_EN |
				  AR724X_GPIO_FUNC_CLK_OBS5_EN |
				  AR724X_GPIO_FUNC_GE0_MII_CLK_EN |
				  AR724X_GPIO_FUNC_ETH_SWITCH_LED0_EN |
				  AR724X_GPIO_FUNC_ETH_SWITCH_LED1_EN |
				  AR724X_GPIO_FUNC_ETH_SWITCH_LED2_EN |
				  AR724X_GPIO_FUNC_ETH_SWITCH_LED3_EN |
				  AR724X_GPIO_FUNC_ETH_SWITCH_LED4_EN |
				  BIT(20));

	wnr_get_wmac(wlan_mac_addr, WNR2000V3_MAC0_OFFSET,
		     WNR2000V3_MAC1_OFFSET, WNR2000V3_WMAC_OFFSET);

	wnr_common_setup(wlan_mac_addr);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(wnr2000v3_leds_gpio),
				 wnr2000v3_leds_gpio);

	/* Do not use id=-1, we can have more GPIO key-polled devices */
	ath79_register_gpio_keys_polled(PLATFORM_DEVID_AUTO,
					WNR2000V3_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(wnr2000v3_keys_gpio),
					wnr2000v3_keys_gpio);

	ap9x_pci_setup_wmac_leds(0, wnr2000v3_wmac_leds_gpio,
				 ARRAY_SIZE(wnr2000v3_wmac_leds_gpio));

	ap9x_pci_setup_wmac_btns(0, wnr2000v3_wmac_keys_gpio,
				 ARRAY_SIZE(wnr2000v3_wmac_keys_gpio),
				 WNR2000V3_KEYS_POLL_INTERVAL);
}

MIPS_MACHINE(ATH79_MACH_WNR2000_V3, "WNR2000V3", "NETGEAR WNR2000 V3", wnr2000v3_setup);

static void __init wnr612v2_setup(void)
{
	u8 wlan_mac_addr[6];

	/*
	 * Disable JTAG and CLKs. Allow OS to control all link LEDs.
	 * Note: U-Boot for WNR612v2 sets undocumented bit 15 but
	 * we leave it for now.
	 */
	ath79_gpio_function_setup(AR724X_GPIO_FUNC_JTAG_DISABLE |
				  AR724X_GPIO_FUNC_UART_EN,
				  AR724X_GPIO_FUNC_CLK_OBS1_EN |
				  AR724X_GPIO_FUNC_CLK_OBS2_EN |
				  AR724X_GPIO_FUNC_CLK_OBS3_EN |
				  AR724X_GPIO_FUNC_CLK_OBS4_EN |
				  AR724X_GPIO_FUNC_CLK_OBS5_EN |
				  AR724X_GPIO_FUNC_GE0_MII_CLK_EN |
				  AR724X_GPIO_FUNC_ETH_SWITCH_LED0_EN |
				  AR724X_GPIO_FUNC_ETH_SWITCH_LED1_EN |
				  AR724X_GPIO_FUNC_ETH_SWITCH_LED2_EN |
				  AR724X_GPIO_FUNC_ETH_SWITCH_LED3_EN |
				  AR724X_GPIO_FUNC_ETH_SWITCH_LED4_EN);

	wnr_get_wmac(wlan_mac_addr, WNR2000V3_MAC0_OFFSET,
		     WNR2000V3_MAC1_OFFSET, WNR2000V3_WMAC_OFFSET);

	wnr_common_setup(wlan_mac_addr);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(wnr612v2_leds_gpio),
				 wnr612v2_leds_gpio);

	ap9x_pci_setup_wmac_leds(0, wnr612v2_wmac_leds_gpio,
				 ARRAY_SIZE(wnr612v2_wmac_leds_gpio));

	ap9x_pci_setup_wmac_btns(0, wnr612v2_wmac_keys_gpio,
				 ARRAY_SIZE(wnr612v2_wmac_keys_gpio),
				 WNR2000V3_KEYS_POLL_INTERVAL);
}

MIPS_MACHINE(ATH79_MACH_WNR612_V2, "WNR612V2", "NETGEAR WNR612 V2", wnr612v2_setup);

static void __init wnr1000v2_setup(void)
{
	u8 wlan_mac_addr[6];

	/*
	 * Disable JTAG and CLKs. Allow OS to control all link LEDs.
	 * Note: U-Boot for WNR1000v2 sets undocumented bit 15 but
	 * we leave it for now.
	 */
	ath79_gpio_function_setup(AR724X_GPIO_FUNC_JTAG_DISABLE |
				  AR724X_GPIO_FUNC_UART_EN,
				  AR724X_GPIO_FUNC_CLK_OBS1_EN |
				  AR724X_GPIO_FUNC_CLK_OBS2_EN |
				  AR724X_GPIO_FUNC_CLK_OBS3_EN |
				  AR724X_GPIO_FUNC_CLK_OBS4_EN |
				  AR724X_GPIO_FUNC_CLK_OBS5_EN |
				  AR724X_GPIO_FUNC_GE0_MII_CLK_EN |
				  AR724X_GPIO_FUNC_ETH_SWITCH_LED0_EN |
				  AR724X_GPIO_FUNC_ETH_SWITCH_LED1_EN |
				  AR724X_GPIO_FUNC_ETH_SWITCH_LED2_EN |
				  AR724X_GPIO_FUNC_ETH_SWITCH_LED3_EN |
				  AR724X_GPIO_FUNC_ETH_SWITCH_LED4_EN);

	wnr_get_wmac(wlan_mac_addr, WNR2000V3_MAC0_OFFSET,
		     WNR2000V3_MAC1_OFFSET, WNR2000V3_WMAC_OFFSET);

	wnr_common_setup(wlan_mac_addr);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(wnr1000v2_leds_gpio),
				 wnr1000v2_leds_gpio);

	ap9x_pci_setup_wmac_leds(0, wnr1000v2_wmac_leds_gpio,
				 ARRAY_SIZE(wnr1000v2_wmac_leds_gpio));

	/* All 3 buttons are connected to wireless chip */
	ap9x_pci_setup_wmac_btns(0, wnr1000v2_wmac_keys_gpio,
				 ARRAY_SIZE(wnr1000v2_wmac_keys_gpio),
				 WNR2000V3_KEYS_POLL_INTERVAL);
}

MIPS_MACHINE(ATH79_MACH_WNR1000_V2, "WNR1000V2", "NETGEAR WNR1000 V2", wnr1000v2_setup);

static void __init wpn824n_setup(void)
{
	ath79_gpio_function_setup(AR724X_GPIO_FUNC_JTAG_DISABLE,
				  AR724X_GPIO_FUNC_ETH_SWITCH_LED0_EN |
				  AR724X_GPIO_FUNC_ETH_SWITCH_LED1_EN |
				  AR724X_GPIO_FUNC_ETH_SWITCH_LED2_EN |
				  AR724X_GPIO_FUNC_ETH_SWITCH_LED3_EN |
				  AR724X_GPIO_FUNC_ETH_SWITCH_LED4_EN |
				  AR724X_GPIO_FUNC_CLK_OBS3_EN);

	wnr_common_setup(NULL);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(wpn824n_leds_gpio),
				 wpn824n_leds_gpio);

	ap9x_pci_setup_wmac_leds(0, wpn824n_wmac_leds_gpio,
				 ARRAY_SIZE(wpn824n_wmac_leds_gpio));
	ap9x_pci_setup_wmac_btns(0, wpn824n_wmac_keys_gpio,
				 ARRAY_SIZE(wpn824n_wmac_keys_gpio),
				 WNR2000V3_KEYS_POLL_INTERVAL);
}

MIPS_MACHINE(ATH79_MACH_WPN824N, "WPN824N", "NETGEAR WPN824N", wpn824n_setup);

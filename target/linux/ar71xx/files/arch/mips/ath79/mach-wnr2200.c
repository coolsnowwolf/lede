/*
 *  NETGEAR WNR2200 board support
 *
 *  Copyright (C) 2013 Aidan Kissane <aidankissane at googlemail.com>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/gpio.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/kernel.h> /* for max() macro */

#include <asm/mach-ath79/ath79.h>
#include <asm/mach-ath79/ar71xx_regs.h> /* needed to disable switch LEDs */
#include "common.h" /* needed to disable switch LEDs */

#include "dev-ap9x-pci.h"
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-usb.h"
#include "machtypes.h"

/* WNR2200 - connected through AR7241 */
#define WNR2200_GPIO_LED_LAN2_AMBER	0
#define WNR2200_GPIO_LED_LAN4_AMBER	1
#define WNR2200_GPIO_LED_LAN1_AMBER	6 /* AR724X_GPIO_FUNC_JTAG_DISABLE */
#define WNR2200_GPIO_LED_WPS_GREEN	7 /* AR724X_GPIO_FUNC_JTAG_DISABLE */
#define WNR2200_GPIO_LED_USB_GREEN	8 /* AR724X_GPIO_FUNC_JTAG_DISABLE */
#define WNR2200_GPIO_LED_LAN3_AMBER	11
#define WNR2200_GPIO_LED_WAN_AMBER	12
#define WNR2200_GPIO_LED_LAN1_GREEN	13 /* AR724X_..._ETH_SWITCH_LED0 */
#define WNR2200_GPIO_LED_LAN2_GREEN	14 /* AR724X_..._ETH_SWITCH_LED1 */
#define WNR2200_GPIO_LED_LAN3_GREEN	15 /* AR724X_..._ETH_SWITCH_LED2 */
#define WNR2200_GPIO_LED_LAN4_GREEN	16 /* AR724X_..._ETH_SWITCH_LED3 */
#define WNR2200_GPIO_LED_WAN_GREEN	17 /* AR724X_..._ETH_SWITCH_LED4 */

/* WNR2200 - connected through AR9287 */
#define WNR2200_GPIO_WMAC_LED_WLAN_BLUE		0
#define WNR2200_GPIO_WMAC_LED_TEST_AMBER	1
#define WNR2200_GPIO_WMAC_LED_POWER_GREEN	2
#define WNR2200_GPIO_WMAC_BTN_RFKILL		3
#define WNR2200_GPIO_WMAC_USB_5V		4
#define WNR2200_GPIO_WMAC_BTN_WPS		5
#define WNR2200_GPIO_WMAC_BTN_RESET		6

#define WNR2200_KEYS_POLL_INTERVAL	20 /* msecs */
#define WNR2200_KEYS_DEBOUNCE_INTERVAL	(3 * WNR2200_KEYS_POLL_INTERVAL)

#define WNR2200_MAC0_OFFSET		0
#define WNR2200_MAC1_OFFSET		6
#define WNR2200_PCIE_CALDATA_OFFSET	0x1000
#define WNR2200_WMAC_OFFSET		0x108c	/* wireless MAC is inside ART */

static struct gpio_led wnr2200_leds_gpio[] __initdata = {
	{
		.name		= "netgear:amber:lan1",
		.gpio		= WNR2200_GPIO_LED_LAN1_AMBER,
		.active_low	= 1,
	}, {
		.name		= "netgear:amber:lan2",
		.gpio		= WNR2200_GPIO_LED_LAN2_AMBER,
		.active_low	= 1,
	}, {
		.name		= "netgear:amber:lan3",
		.gpio		= WNR2200_GPIO_LED_LAN3_AMBER,
		.active_low	= 1,
	}, {
		.name		= "netgear:amber:lan4",
		.gpio		= WNR2200_GPIO_LED_LAN4_AMBER,
		.active_low	= 1,
	}, {
		.name		= "netgear:amber:wan",
		.gpio		= WNR2200_GPIO_LED_WAN_AMBER,
		.active_low	= 1,
	}, {
		.name		= "netgear:green:lan1",
		.gpio		= WNR2200_GPIO_LED_LAN1_GREEN,
		.active_low	= 1,
	}, {
		.name		= "netgear:green:lan2",
		.gpio		= WNR2200_GPIO_LED_LAN2_GREEN,
		.active_low	= 1,
	}, {
		.name		= "netgear:green:lan3",
		.gpio		= WNR2200_GPIO_LED_LAN3_GREEN,
		.active_low	= 1,
	}, {
		.name		= "netgear:green:lan4",
		.gpio		= WNR2200_GPIO_LED_LAN4_GREEN,
		.active_low	= 1,
	}, {
		.name		= "netgear:green:usb",
		.gpio		= WNR2200_GPIO_LED_USB_GREEN,
		.active_low	= 1,
	}, {
		.name		= "netgear:green:wan",
		.gpio		= WNR2200_GPIO_LED_WAN_GREEN,
		.active_low	= 1,
	}, {
		.name		= "netgear:green:wps",
		.gpio		= WNR2200_GPIO_LED_WPS_GREEN,
		.active_low	= 1,
	}
};

static struct gpio_led wnr2200_wmac_leds_gpio[] = {
	{
		.name		= "netgear:amber:test",
		.gpio		= WNR2200_GPIO_WMAC_LED_TEST_AMBER,
		.active_low	= 1,
	}, {
		.name		= "netgear:green:power",
		.gpio		= WNR2200_GPIO_WMAC_LED_POWER_GREEN,
		.active_low	= 1,
		.default_state	= LEDS_GPIO_DEFSTATE_ON,
	}, {
		.name		= "netgear:blue:wlan",
		.gpio		= WNR2200_GPIO_WMAC_LED_WLAN_BLUE,
		.active_low	= 1,
	}
};

static struct gpio_keys_button wnr2200_wmac_keys_gpio[] = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = WNR2200_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= WNR2200_GPIO_WMAC_BTN_RESET,
		.active_low	= 1,
	}, {
		.desc		= "rfkill",
		.type		= EV_KEY,
		.code		= KEY_RFKILL,
		.debounce_interval = WNR2200_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= WNR2200_GPIO_WMAC_BTN_RFKILL,
		.active_low	= 1,
	}, {
		.desc		= "wps",
		.type		= EV_KEY,
		.code		= KEY_WPS_BUTTON,
		.debounce_interval = WNR2200_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= WNR2200_GPIO_WMAC_BTN_WPS,
		.active_low	= 1,
	}
};

/*
 * For WNR2200 ART flash area used for WLAN MAC is usually empty (0xff)
 * so ath9k driver uses random MAC instead each time module is loaded.
 * OpenWrt's original fix was to copy eth1 address to WLAN interface.
 * New solution does not duplicate hardware addresses and is taken from
 * WNR2000v3 code. It assigns permanent WLAN MAC equal to ethN's MAC
 * plus 1, so network interfaces get sequential addresses.
 * If ART wireless MAC address field has been filled by user, use it.
 */
static void __init wnr2200_get_wmac(u8 *wmac_gen_addr, int mac0_art_offset,
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

static void __init wnr2200_setup(void)
{
	u8 *art = (u8 *) KSEG1ADDR(0x1fff0000);
	u8 wlan_mac_addr[6];

	/*
	 * Disable JTAG to use all AR724X GPIO LEDs. Disable CLKs.
	 * Allow OS to control all link LEDs.
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

	ath79_register_mdio(0, 0x0);

	ath79_init_mac(ath79_eth0_data.mac_addr, art + WNR2200_MAC0_OFFSET, 0);
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RMII;
	ath79_eth0_data.speed = SPEED_100;
	ath79_eth0_data.duplex = DUPLEX_FULL;

	ath79_init_mac(ath79_eth1_data.mac_addr, art + WNR2200_MAC1_OFFSET, 0);
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_RMII;
	ath79_eth1_data.phy_mask = 0x10;

	ath79_register_eth(0);
	ath79_register_eth(1);

	ath79_register_m25p80(NULL);

	wnr2200_get_wmac(wlan_mac_addr, WNR2200_MAC0_OFFSET,
			 WNR2200_MAC1_OFFSET, WNR2200_WMAC_OFFSET);
	ap9x_pci_setup_wmac_led_pin(0, 0);
	ap91_pci_init(art + WNR2200_PCIE_CALDATA_OFFSET, wlan_mac_addr);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(wnr2200_leds_gpio),
				 wnr2200_leds_gpio);

	ap9x_pci_setup_wmac_leds(0, wnr2200_wmac_leds_gpio,
				 ARRAY_SIZE(wnr2200_wmac_leds_gpio));

	/* All 3 buttons are connected to wireless chip */
	ap9x_pci_setup_wmac_btns(0, wnr2200_wmac_keys_gpio,
				 ARRAY_SIZE(wnr2200_wmac_keys_gpio),
				 WNR2200_KEYS_POLL_INTERVAL);

	/* enable power for the USB port */
	ap9x_pci_setup_wmac_gpio(0, BIT(WNR2200_GPIO_WMAC_USB_5V),
				 BIT(WNR2200_GPIO_WMAC_USB_5V));

	ath79_register_usb();
}

MIPS_MACHINE(ATH79_MACH_WNR2200, "WNR2200", "NETGEAR WNR2200", wnr2200_setup);

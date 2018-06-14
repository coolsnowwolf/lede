/*
 *  HiWiFi HC6361 board support
 *
 *  Copyright (C) 2012-2013 eric
 *  Copyright (C) 2014 Yousong Zhou <yszhou4tech@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/gpio.h>
#include <linux/proc_fs.h>

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

#define HIWIFI_HC6361_GPIO_LED_WLAN_2P4		0	/* 2.4G WLAN LED */
#define HIWIFI_HC6361_GPIO_LED_SYSTEM		1	/* System LED */
#define HIWIFI_HC6361_GPIO_LED_INTERNET		27	/* Internet LED */

#define HIWIFI_HC6361_GPIO_USBPOWER		20	/* USB power control */
#define HIWIFI_HC6361_GPIO_BTN_RST		11	/* Reset button */

#define HIWIFI_HC6361_KEYS_POLL_INTERVAL	20	/* msecs */
#define HIWIFI_HC6361_KEYS_DEBOUNCE_INTERVAL	\
	(3 * HIWIFI_HC6361_KEYS_POLL_INTERVAL)

static struct gpio_led hiwifi_leds_gpio[] __initdata = {
	{
		.name		= "hiwifi:blue:wlan-2p4",
		.gpio		= HIWIFI_HC6361_GPIO_LED_WLAN_2P4,
		.active_low	= 1,
	}, {
		.name		= "hiwifi:blue:system",
		.gpio		= HIWIFI_HC6361_GPIO_LED_SYSTEM,
		.active_low	= 1,
	}, {
		.name		= "hiwifi:blue:internet",
		.gpio		= HIWIFI_HC6361_GPIO_LED_INTERNET,
		.active_low	= 1,
	}
};

static struct gpio_keys_button hiwifi_gpio_keys[] __initdata = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = HIWIFI_HC6361_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= HIWIFI_HC6361_GPIO_BTN_RST,
		.active_low	= 1,
	}
};

static void __init get_mac_from_bdinfo(u8 *mac, void *bdinfo)
{
	if (sscanf(bdinfo, "fac_mac = %2hhx:%2hhx:%2hhx:%2hhx:%2hhx:%2hhx",
				&mac[0], &mac[1], &mac[2], &mac[3],
				&mac[4], &mac[5]) == 6) {
		return;
	}

	printk(KERN_WARNING "Parsing MAC address failed.\n");
	memcpy(mac, "\x00\xba\xbe\x00\x00\x00", 6);
}

static void __init hiwifi_hc6361_setup(void)
{
	u8 *ee = (u8 *) KSEG1ADDR(0x1fff1000);
	u8 mac[6];

	ath79_setup_ar933x_phy4_switch(false, false);

	ath79_register_m25p80(NULL);
	ath79_gpio_function_enable(
			AR933X_GPIO_FUNC_ETH_SWITCH_LED0_EN |
			AR933X_GPIO_FUNC_ETH_SWITCH_LED1_EN |
			AR933X_GPIO_FUNC_ETH_SWITCH_LED2_EN |
			AR933X_GPIO_FUNC_ETH_SWITCH_LED3_EN |
			AR933X_GPIO_FUNC_ETH_SWITCH_LED4_EN);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(hiwifi_leds_gpio),
			hiwifi_leds_gpio);
	ath79_register_gpio_keys_polled(-1, HIWIFI_HC6361_KEYS_POLL_INTERVAL,
			ARRAY_SIZE(hiwifi_gpio_keys),
			hiwifi_gpio_keys);
	gpio_request_one(HIWIFI_HC6361_GPIO_USBPOWER,
			GPIOF_OUT_INIT_HIGH | GPIOF_EXPORT_DIR_FIXED,
			"USB power");
	ath79_register_usb();

	get_mac_from_bdinfo(mac, (void *) KSEG1ADDR(0x1f010180));
	ath79_init_mac(ath79_eth0_data.mac_addr, mac, 1);
	ath79_init_mac(ath79_eth1_data.mac_addr, mac, 0);

	ath79_register_mdio(0, 0x0);

	ath79_register_eth(1);
	ath79_register_eth(0);

	ath79_register_wmac(ee, mac);
}

MIPS_MACHINE(ATH79_MACH_HIWIFI_HC6361, "HiWiFi-HC6361",
		"HiWiFi HC6361", hiwifi_hc6361_setup);

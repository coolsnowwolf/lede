/*
 * EnGenius EAP300 v2 board support
 *
 * Copyright (C) 2014 Gabor Juhos <juhosg@openwrt.org>
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

#define EAP300V2_GPIO_LED_POWER		0
#define EAP300V2_GPIO_LED_LAN		16
#define EAP300V2_GPIO_LED_WLAN		17

#define EAP300V2_GPIO_BTN_RESET		1

#define EAP300V2_KEYS_POLL_INTERVAL	20	/* msecs */
#define EAP300V2_KEYS_DEBOUNCE_INTERVAL	(3 * EAP300V2_KEYS_POLL_INTERVAL)

static struct gpio_led eap300v2_leds_gpio[] __initdata = {
	{
		.name		= "engenius:blue:power",
		.gpio		= EAP300V2_GPIO_LED_POWER,
		.active_low	= 1,
	}, {
		.name		= "engenius:blue:lan",
		.gpio		= EAP300V2_GPIO_LED_LAN,
		.active_low	= 1,
	}, {
		.name		= "engenius:blue:wlan",
		.gpio		= EAP300V2_GPIO_LED_WLAN,
		.active_low	= 1,
	}
};

static struct gpio_keys_button eap300v2_gpio_keys[] __initdata = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = EAP300V2_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= EAP300V2_GPIO_BTN_RESET,
		.active_low	= 1,
	}
};

#define EAP300V2_ART_MAC_OFFSET		2

#define EAP300V2_LAN_PHYMASK		BIT(0)

static void __init eap300v2_setup(void)
{
	u8 *art = (u8 *)KSEG1ADDR(0x1fff1000);

	ath79_gpio_function_enable(AR934X_GPIO_FUNC_JTAG_DISABLE);

	ath79_gpio_output_select(EAP300V2_GPIO_LED_POWER, AR934X_GPIO_OUT_GPIO);
	ath79_gpio_output_select(EAP300V2_GPIO_LED_LAN, AR934X_GPIO_OUT_GPIO);
	ath79_gpio_output_select(EAP300V2_GPIO_LED_WLAN, AR934X_GPIO_OUT_GPIO);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(eap300v2_leds_gpio),
				 eap300v2_leds_gpio);
	ath79_register_gpio_keys_polled(-1, EAP300V2_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(eap300v2_gpio_keys),
					eap300v2_gpio_keys);

	ath79_register_m25p80(NULL);
	ath79_register_wmac(art, NULL);
	ath79_register_mdio(1, 0x0);

	ath79_setup_ar934x_eth_cfg(AR934X_ETH_CFG_SW_PHY_SWAP);

	ath79_init_mac(ath79_eth0_data.mac_addr,
		       art + EAP300V2_ART_MAC_OFFSET, 0);

	ath79_switch_data.phy4_mii_en = 1;
	ath79_switch_data.phy_poll_mask = EAP300V2_LAN_PHYMASK;
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ath79_eth0_data.phy_mask = EAP300V2_LAN_PHYMASK;
	ath79_eth0_data.mii_bus_dev = &ath79_mdio1_device.dev;
	ath79_register_eth(0);
}

MIPS_MACHINE(ATH79_MACH_EAP300V2, "EAP300V2", "EnGenius EAP300 v2",
	     eap300v2_setup);

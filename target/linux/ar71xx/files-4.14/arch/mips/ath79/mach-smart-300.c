/*
 *  NC-LINK SMART-300 board support
 *
 *  Copyright (C) 2012 Gabor Juhos <juhosg@openwrt.org>
 *  Copyright (C) 2014 Imre Kaloz <kaloz@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <asm/mach-ath79/ath79.h>
#include <asm/mach-ath79/ar71xx_regs.h>
#include <asm/mach-ath79/ag71xx_platform.h>

#include "common.h"
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-wmac.h"
#include "machtypes.h"

#define SMART_300_GPIO_LED_WLAN		13
#define SMART_300_GPIO_LED_WAN		18
#define SMART_300_GPIO_LED_LAN4		19
#define SMART_300_GPIO_LED_LAN3		12
#define SMART_300_GPIO_LED_LAN2		21
#define SMART_300_GPIO_LED_LAN1		20
#define SMART_300_GPIO_LED_SYSTEM	15
#define SMART_300_GPIO_LED_POWER	14

#define SMART_300_GPIO_BTN_RESET	17
#define SMART_300_GPIO_SW_RFKILL	16

#define SMART_300_KEYS_POLL_INTERVAL	20	/* msecs */
#define SMART_300_KEYS_DEBOUNCE_INTERVAL (3 * SMART_300_KEYS_POLL_INTERVAL)

#define SMART_300_GPIO_MASK        0x007fffff

static const char *smart_300_part_probes[] = {
	"tp-link",
	NULL,
};

static struct flash_platform_data smart_300_flash_data = {
	.part_probes	= smart_300_part_probes,
};

static struct gpio_led smart_300_leds_gpio[] __initdata = {
	{
		.name		= "nc-link:green:lan1",
		.gpio		= SMART_300_GPIO_LED_LAN1,
		.active_low	= 1,
	}, {
		.name		= "nc-link:green:lan2",
		.gpio		= SMART_300_GPIO_LED_LAN2,
		.active_low	= 1,
	}, {
		.name		= "nc-link:green:lan3",
		.gpio		= SMART_300_GPIO_LED_LAN3,
		.active_low	= 1,
	}, {
		.name		= "nc-link:green:lan4",
		.gpio		= SMART_300_GPIO_LED_LAN4,
		.active_low	= 1,
	}, {
		.name		= "nc-link:green:system",
		.gpio		= SMART_300_GPIO_LED_SYSTEM,
		.active_low	= 1,
	}, {
		.name		= "nc-link:green:wan",
		.gpio		= SMART_300_GPIO_LED_WAN,
		.active_low	= 1,
	}, {
		.name		= "nc-link:green:wlan",
		.gpio		= SMART_300_GPIO_LED_WLAN,
		.active_low	= 1,
	},
};

static struct gpio_keys_button smart_300_gpio_keys[] __initdata = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = SMART_300_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= SMART_300_GPIO_BTN_RESET,
		.active_low	= 1,
	}
};

static void __init smart_300_setup(void)
{
	u8 *mac = (u8 *) KSEG1ADDR(0x1f01fc00);
	u8 *ee = (u8 *) KSEG1ADDR(0x1fff1000);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(smart_300_leds_gpio),
				 smart_300_leds_gpio);

	ath79_register_gpio_keys_polled(1, SMART_300_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(smart_300_gpio_keys),
					smart_300_gpio_keys);

	ath79_register_m25p80(&smart_300_flash_data);

	ath79_setup_ar934x_eth_cfg(AR934X_ETH_CFG_SW_ONLY_MODE);

	ath79_register_mdio(1, 0x0);

	ath79_init_mac(ath79_eth0_data.mac_addr, mac, -1);
	ath79_init_mac(ath79_eth1_data.mac_addr, mac, 1);

	/* GMAC0 is connected to the PHY0 of the internal switch */
	ath79_switch_data.phy4_mii_en = 1;
	ath79_switch_data.phy_poll_mask = BIT(4);
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ath79_eth0_data.phy_mask = BIT(4);
	ath79_eth0_data.mii_bus_dev = &ath79_mdio1_device.dev;
	ath79_register_eth(0);

	/* GMAC1 is connected to the internal switch */
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_GMII;
	ath79_register_eth(1);

	ath79_register_wmac(ee, mac);

	gpio_request(SMART_300_GPIO_LED_POWER, "power");
	gpio_direction_output(SMART_300_GPIO_LED_POWER, GPIOF_OUT_INIT_LOW);
}

MIPS_MACHINE(ATH79_MACH_SMART_300, "SMART-300", "NC-LINK SMART-300",
	     smart_300_setup);

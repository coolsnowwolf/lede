/*
 *  Qxwlan E1700AC v2 board support
 *
 *  Copyright (C) 2017 Peng Zhang <sd20@qxwlan.com>
 *  Copyright (C) 2018 Piotr Dymacz <pepe2k@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/platform_device.h>
#include <linux/ar8216_platform.h>

#include "common.h"
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-usb.h"
#include "dev-wmac.h"
#include "machtypes.h"
#include "pci.h"

#define E1700AC_V2_GPIO_LED_SYS		1
#define E1700AC_V2_GPIO_LED_USB		7
#define E1700AC_V2_GPIO_LED_WLAN2G	19

#define E1700AC_V2_GPIO_BTN_SW1		2
#define E1700AC_V2_GPIO_BTN_RESET	11

#define E1700AC_V2_KEYS_POLL_INTERVAL		20 /* msecs */
#define E1700AC_V2_KEYS_DEBOUNCE_INTERVAL	\
					(3 * E1700AC_V2_KEYS_POLL_INTERVAL)

static struct gpio_led e1700ac_v2_leds_gpio[] __initdata = {
	{
		.name		= "e1700ac-v2:green:system",
		.gpio		= E1700AC_V2_GPIO_LED_SYS,
		.active_low	= 1,
	}, {
		.name		= "e1700ac-v2:green:usb",
		.gpio		= E1700AC_V2_GPIO_LED_USB,
		.active_low	= 1,
	}, {
		.name		= "e1700ac-v2:green:wlan2g",
		.gpio		= E1700AC_V2_GPIO_LED_WLAN2G,
		.active_low	= 1,
	},
};

static struct gpio_keys_button e1700ac_v2_gpio_keys[] __initdata = {
	{
		.desc			= "reset",
		.type			= EV_KEY,
		.code			= KEY_RESTART,
		.debounce_interval	= E1700AC_V2_KEYS_DEBOUNCE_INTERVAL,
		.gpio			= E1700AC_V2_GPIO_BTN_RESET,
		.active_low		= 1,
	}, {
		.desc			= "sw1",
		.type			= EV_KEY,
		.code			= BTN_0,
		.debounce_interval	= E1700AC_V2_KEYS_DEBOUNCE_INTERVAL,
		.gpio			= E1700AC_V2_GPIO_BTN_SW1,
		.active_low		= 1,
	},
};

static const struct ar8327_led_info e1700ac_v2_leds_qca8334[] = {
	AR8327_LED_INFO(PHY1_0, HW, "e1700ac-v2:green:lan"),
	AR8327_LED_INFO(PHY2_0, HW, "e1700ac-v2:green:wan"),
};

/* Blink rate: 1 Gbps -> 8 hz, 100 Mbs -> 4 Hz, 10 Mbps -> 2 Hz */
static struct ar8327_led_cfg e1700ac_v2_qca8334_led_cfg = {
	.led_ctrl0 = 0xcf37cf37,
	.led_ctrl1 = 0xcf37cf37,
	.led_ctrl2 = 0xcf37cf37,
	.led_ctrl3 = 0x0,
	.open_drain = true,
};

static struct ar8327_pad_cfg e1700ac_v2_qca8334_pad0_cfg = {
	.mode = AR8327_PAD_MAC_SGMII,
	.sgmii_delay_en = true,
};

static struct ar8327_platform_data e1700ac_v2_qca8334_data = {
	.pad0_cfg = &e1700ac_v2_qca8334_pad0_cfg,
	.port0_cfg = {
		.force_link = 1,
		.speed = AR8327_PORT_SPEED_1000,
		.duplex = 1,
		.txpause = 1,
		.rxpause = 1,
	},
	.led_cfg = &e1700ac_v2_qca8334_led_cfg,
	.leds = e1700ac_v2_leds_qca8334,
	.num_leds = ARRAY_SIZE(e1700ac_v2_leds_qca8334),
};

static struct mdio_board_info e1700ac_v2_mdio0_info[] = {
	{
		.bus_id = "ag71xx-mdio.0",
		.mdio_addr = 0,
		.platform_data = &e1700ac_v2_qca8334_data,
	},
};

static void __init e1700ac_v2_setup(void)
{
	u8 *mac = (u8 *) KSEG1ADDR(0x1f050400);
	u8 *art = (u8 *) KSEG1ADDR(0x1f061000);

	ath79_register_m25p80(NULL);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(e1700ac_v2_leds_gpio),
				 e1700ac_v2_leds_gpio);

	ath79_register_gpio_keys_polled(-1, E1700AC_V2_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(e1700ac_v2_gpio_keys),
					e1700ac_v2_gpio_keys);

	ath79_register_mdio(0, 0x0);
	mdiobus_register_board_info(e1700ac_v2_mdio0_info,
				    ARRAY_SIZE(e1700ac_v2_mdio0_info));

	/* GMAC0 is connected to QCA8334 switch */
	ath79_eth0_data.duplex = DUPLEX_FULL;
	ath79_eth0_data.mii_bus_dev = &ath79_mdio0_device.dev;
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_SGMII;
	ath79_eth0_data.phy_mask = BIT(0);
	ath79_eth0_data.speed = SPEED_1000;
	ath79_eth0_pll_data.pll_1000 = 0x03000101;
	ath79_init_mac(ath79_eth0_data.mac_addr, mac, 0);
	ath79_register_eth(0);

	ath79_register_pci();
	ath79_register_usb();
	ath79_register_wmac(art, NULL);
}

MIPS_MACHINE(ATH79_MACH_E1700AC_V2, "E1700AC-V2", "Qxwlan E1700AC v2",
	     e1700ac_v2_setup);

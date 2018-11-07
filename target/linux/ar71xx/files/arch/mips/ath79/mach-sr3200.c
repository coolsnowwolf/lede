/*
 * Support for YunCore SR3200 and XD3200 boards
 *
 * Copyright (C) 2016 Piotr Dymacz <pepe2k@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 */

#include <linux/phy.h>
#include <linux/gpio.h>
#include <linux/ar8216_platform.h>
#include <linux/platform_device.h>

#include <asm/mach-ath79/ath79.h>
#include <asm/mach-ath79/ar71xx_regs.h>

#include "common.h"
#include "dev-ap9x-pci.h"
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-usb.h"
#include "dev-wmac.h"
#include "machtypes.h"

#define SR3200_XD3200_GPIO_LED_SYSTEM	1
#define SR3200_XD3200_GPIO_LED_WLAN2G	19

#define SR3200_XD3200_GPIO_BTN_RESET	2

#define SR3200_XD3200_KEYS_POLL_INTERVAL	20
#define SR3200_XD3200_KEYS_DEBOUNCE_INTERVAL	\
		(3 * SR3200_XD3200_KEYS_POLL_INTERVAL)

static struct gpio_led xd3200_leds_gpio[] __initdata = {
	{
		.name		= "xd3200:green:system",
		.gpio		= SR3200_XD3200_GPIO_LED_SYSTEM,
		.active_low	= 1,
	},
	{
		.name		= "xd3200:blue:wlan2g",
		.gpio		= SR3200_XD3200_GPIO_LED_WLAN2G,
		.active_low	= 1,
	},
};

static struct gpio_led sr3200_leds_gpio[] __initdata = {
	{
		.name		= "sr3200:green:system",
		.gpio		= SR3200_XD3200_GPIO_LED_SYSTEM,
		.active_low	= 1,
	},
	{
		.name		= "sr3200:green:wlan2g",
		.gpio		= SR3200_XD3200_GPIO_LED_WLAN2G,
		.active_low	= 1,
	},
};

static struct gpio_keys_button sr3200_xd3200_gpio_keys[] __initdata = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = SR3200_XD3200_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= SR3200_XD3200_GPIO_BTN_RESET,
		.active_low	= 1,
	},
};

static const struct ar8327_led_info sr3200_leds_qca833x[] = {
	AR8327_LED_INFO(PHY0_0, HW, "sr3200:green:lan1"),
	AR8327_LED_INFO(PHY1_0, HW, "sr3200:green:lan2"),
	AR8327_LED_INFO(PHY2_0, HW, "sr3200:green:lan3"),
	AR8327_LED_INFO(PHY3_0, HW, "sr3200:green:lan4"),
	AR8327_LED_INFO(PHY4_0, HW, "sr3200:green:wan"),
};

static const struct ar8327_led_info xd3200_leds_qca833x[] = {
	AR8327_LED_INFO(PHY1_0, HW, "xd3200:green:lan"),
	AR8327_LED_INFO(PHY2_0, HW, "xd3200:green:wan"),
};

/* Blink rate: 1 Gbps -> 8 hz, 100 Mbs -> 4 Hz, 10 Mbps -> 2 Hz */
static struct ar8327_led_cfg sr3200_xd3200_qca833x_led_cfg = {
	.led_ctrl0 = 0xcf37cf37,
	.led_ctrl1 = 0xcf37cf37,
	.led_ctrl2 = 0xcf37cf37,
	.led_ctrl3 = 0x0,
	.open_drain = true,
};

static struct ar8327_pad_cfg sr3200_xd3200_qca833x_pad0_cfg = {
	.mode = AR8327_PAD_MAC_SGMII,
	.sgmii_delay_en = true,
};

static struct ar8327_platform_data sr3200_xd3200_qca833x_data = {
	.pad0_cfg = &sr3200_xd3200_qca833x_pad0_cfg,
	.port0_cfg = {
		.force_link = 1,
		.speed = AR8327_PORT_SPEED_1000,
		.duplex = 1,
		.txpause = 1,
		.rxpause = 1,
	},
	.led_cfg = &sr3200_xd3200_qca833x_led_cfg,
};

static struct mdio_board_info sr3200_xd3200_mdio0_info[] = {
	{
		.bus_id = "ag71xx-mdio.0",
		.mdio_addr = 0,
		.platform_data = &sr3200_xd3200_qca833x_data,
	},
};

static void __init sr3200_xd3200_common_setup(void)
{
	u8 *mac = (u8 *) KSEG1ADDR(0x1fff0000);

	ath79_register_m25p80(NULL);

	ath79_register_mdio(0, 0x0);
	mdiobus_register_board_info(sr3200_xd3200_mdio0_info,
				    ARRAY_SIZE(sr3200_xd3200_mdio0_info));

	/* GMAC0 is connected to QCA8334/QCA8337N switch */
	ath79_eth0_data.mii_bus_dev = &ath79_mdio0_device.dev;
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_SGMII;
	ath79_eth0_data.phy_mask = BIT(0);
	ath79_eth0_data.speed = SPEED_1000;
	ath79_eth0_data.duplex = DUPLEX_FULL;

	ath79_init_mac(ath79_eth0_data.mac_addr, mac, 0);
	ath79_register_eth(0);

	ath79_register_wmac(mac + 0x1000, NULL);

	ap91_pci_init(mac + 0x5000, NULL);

	ath79_gpio_direction_select(SR3200_XD3200_GPIO_LED_SYSTEM, true);
	ath79_gpio_direction_select(SR3200_XD3200_GPIO_LED_WLAN2G, true);

	/* Mute LEDs on boot */
	gpio_set_value(SR3200_XD3200_GPIO_LED_SYSTEM, 1);
	gpio_set_value(SR3200_XD3200_GPIO_LED_WLAN2G, 1);

	ath79_gpio_output_select(SR3200_XD3200_GPIO_LED_SYSTEM, 0);
	ath79_gpio_output_select(SR3200_XD3200_GPIO_LED_WLAN2G, 0);

	ath79_register_gpio_keys_polled(-1, SR3200_XD3200_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(sr3200_xd3200_gpio_keys),
					sr3200_xd3200_gpio_keys);
}

static void __init sr3200_setup(void)
{
	sr3200_xd3200_qca833x_data.leds = sr3200_leds_qca833x;
	sr3200_xd3200_qca833x_data.num_leds = ARRAY_SIZE(sr3200_leds_qca833x);

	sr3200_xd3200_common_setup();

	ath79_register_leds_gpio(-1, ARRAY_SIZE(sr3200_leds_gpio),
				 sr3200_leds_gpio);

	ath79_register_usb();
}

MIPS_MACHINE(ATH79_MACH_SR3200, "SR3200", "YunCore SR3200", sr3200_setup);

static void __init xd3200_setup(void)
{
	sr3200_xd3200_qca833x_data.leds = xd3200_leds_qca833x;
	sr3200_xd3200_qca833x_data.num_leds = ARRAY_SIZE(xd3200_leds_qca833x);

	sr3200_xd3200_common_setup();

	ath79_register_leds_gpio(-1, ARRAY_SIZE(xd3200_leds_gpio),
				 xd3200_leds_gpio);
}

MIPS_MACHINE(ATH79_MACH_XD3200, "XD3200", "YunCore XD3200", xd3200_setup);

/*
 *  Hak5 LAN Turtle and Packet Squirrel boards support
 *
 *  Copyright (C) 2018 Sebastian Kinne <seb@hak5.org>
 *  Copyright (C) 2018 Piotr Dymacz <pepe2k@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/gpio.h>

#include <asm/mach-ath79/ath79.h>
#include <asm/mach-ath79/ar71xx_regs.h>

#include "common.h"
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-usb.h"
#include "machtypes.h"

#define LAN_TURTLE_GPIO_BTN_RESET	11
#define LAN_TURTLE_GPIO_LED_SYS		13

#define PACKET_SQUIRREL_GPIO_BTN_SW1	18
#define PACKET_SQUIRREL_GPIO_BTN_SW2	20
#define PACKET_SQUIRREL_GPIO_BTN_SW3	21
#define PACKET_SQUIRREL_GPIO_BTN_SW4	24
#define PACKET_SQUIRREL_GPIO_BTN_RESET	11
#define PACKET_SQUIRREL_GPIO_LED_B	23
#define PACKET_SQUIRREL_GPIO_LED_G	22
#define PACKET_SQUIRREL_GPIO_LED_R	19

#define HAK5_KEYS_POLL_INTERVAL		20 /* msecs */
#define HAK5_KEYS_DEBOUNCE_INTERVAL	(3 * HAK5_KEYS_POLL_INTERVAL)

static const char *hak5_part_probes[] = {
	"tp-link",
	NULL,
};

static struct flash_platform_data hak5_flash_data = {
	.part_probes = hak5_part_probes,
};

/* LAN Turtle */
static struct gpio_led lan_turtle_leds_gpio[] __initdata = {
	{
		.name		= "lan-turtle:orange:system",
		.gpio		= LAN_TURTLE_GPIO_LED_SYS,
		.active_low	= 1,
	},
};

static struct gpio_keys_button lan_turtle_gpio_keys[] __initdata = {
	{
		.desc			= "reset",
		.type			= EV_KEY,
		.code			= KEY_RESTART,
		.debounce_interval	= HAK5_KEYS_DEBOUNCE_INTERVAL,
		.gpio			= LAN_TURTLE_GPIO_BTN_RESET,
		.active_low		= 1,
	},
};

/* Packet Squirrel */
static struct gpio_led packet_squirrel_leds_gpio[] __initdata = {
	{
		.name		= "packet-squirrel:blue:system",
		.gpio		= PACKET_SQUIRREL_GPIO_LED_B,
		.active_low	= 1,
	}, {
		.name		= "packet-squirrel:green:system",
		.gpio		= PACKET_SQUIRREL_GPIO_LED_G,
		.active_low	= 1,
	}, {
		.name		= "packet-squirrel:red:system",
		.gpio		= PACKET_SQUIRREL_GPIO_LED_R,
		.active_low	= 1,
	},
};

static struct gpio_keys_button packet_squirrel_gpio_keys[] __initdata = {
	{
		.desc			= "reset",
		.type			= EV_KEY,
		.code			= KEY_RESTART,
		.debounce_interval	= HAK5_KEYS_DEBOUNCE_INTERVAL,
		.gpio			= PACKET_SQUIRREL_GPIO_BTN_RESET,
		.active_low		= 1,
	}, {
		.desc			= "sw1",
		.type			= EV_KEY,
		.code			= BTN_0,
		.debounce_interval	= HAK5_KEYS_DEBOUNCE_INTERVAL,
		.gpio			= PACKET_SQUIRREL_GPIO_BTN_SW1,
		.active_low		= 1,
	}, {
		.desc			= "sw2",
		.type			= EV_KEY,
		.code			= BTN_1,
		.debounce_interval	= HAK5_KEYS_DEBOUNCE_INTERVAL,
		.gpio			= PACKET_SQUIRREL_GPIO_BTN_SW2,
		.active_low		= 1,
	}, {
		.desc			= "sw3",
		.type			= EV_KEY,
		.code			= BTN_2,
		.debounce_interval	= HAK5_KEYS_DEBOUNCE_INTERVAL,
		.gpio			= PACKET_SQUIRREL_GPIO_BTN_SW3,
		.active_low		= 1,
	}, {
		.desc			= "sw4",
		.type			= EV_KEY,
		.code			= BTN_3,
		.debounce_interval	= HAK5_KEYS_DEBOUNCE_INTERVAL,
		.gpio			= PACKET_SQUIRREL_GPIO_BTN_SW4,
		.active_low		= 1,
	},
};

static void __init hak5_common_setup(void)
{
	u8 *mac = (u8 *) KSEG1ADDR(0x1f01fc00);

	ath79_register_m25p80(&hak5_flash_data);

	ath79_register_mdio(0, 0x0);

	ath79_switch_data.phy_poll_mask = 0xfe;

	ath79_init_mac(ath79_eth0_data.mac_addr, mac, 1);
	ath79_register_eth(0);

	ath79_init_mac(ath79_eth1_data.mac_addr, mac, -1);
	ath79_register_eth(1);

	ath79_register_usb();

	/* GPIO11/12 */
	ath79_gpio_function_disable(AR933X_GPIO_FUNC_UART_RTS_CTS_EN);
}

static void __init lan_turtle_setup(void)
{
	hak5_common_setup();

	/* GPIO13 */
	ath79_gpio_function_disable(AR933X_GPIO_FUNC_ETH_SWITCH_LED0_EN);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(lan_turtle_leds_gpio),
				 lan_turtle_leds_gpio);

	ath79_register_gpio_keys_polled(-1, HAK5_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(lan_turtle_gpio_keys),
					lan_turtle_gpio_keys);
}

static void __init packet_squirrel_setup(void)
{
	hak5_common_setup();

	ath79_register_leds_gpio(-1, ARRAY_SIZE(packet_squirrel_leds_gpio),
				 packet_squirrel_leds_gpio);

	ath79_register_gpio_keys_polled(-1, HAK5_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(packet_squirrel_gpio_keys),
					packet_squirrel_gpio_keys);
}

MIPS_MACHINE(ATH79_MACH_LAN_TURTLE, "LAN-TURTLE",
	     "Hak5 LAN Turtle", lan_turtle_setup);

MIPS_MACHINE(ATH79_MACH_PACKET_SQUIRREL, "PACKET-SQUIRREL",
	     "Hak5 Packet Squirrel", packet_squirrel_setup);

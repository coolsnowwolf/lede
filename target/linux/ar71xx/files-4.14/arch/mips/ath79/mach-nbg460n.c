/*
 *  Zyxel NBG 460N/550N/550NH board support
 *
 *  Copyright (C) 2010 Michael Kurz <michi.kurz@googlemail.com>
 *
 *  based on mach-tl-wr1043nd.c
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/i2c-algo-bit.h>
#include <linux/i2c-gpio.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/platform_device.h>
#include <linux/rtl8366.h>

#include <asm/mach-ath79/ath79.h>

#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-wmac.h"
#include "machtypes.h"

/* LEDs */
#define NBG460N_GPIO_LED_WPS		3
#define NBG460N_GPIO_LED_WAN		6
#define NBG460N_GPIO_LED_POWER		14
#define NBG460N_GPIO_LED_WLAN		15

/* Buttons */
#define NBG460N_GPIO_BTN_WPS		12
#define NBG460N_GPIO_BTN_RESET		21

#define NBG460N_KEYS_POLL_INTERVAL	20	/* msecs */
#define NBG460N_KEYS_DEBOUNCE_INTERVAL	(3 * NBG460N_KEYS_POLL_INTERVAL)

/* RTC chip PCF8563 I2C interface */
#define NBG460N_GPIO_PCF8563_SDA	8
#define NBG460N_GPIO_PCF8563_SCK	7

/* Switch configuration I2C interface */
#define NBG460N_GPIO_RTL8366_SDA	16
#define NBG460N_GPIO_RTL8366_SCK	18

static struct mtd_partition nbg460n_partitions[] = {
	{
		.name		= "Bootbase",
		.offset		= 0,
		.size		= 0x010000,
		.mask_flags	= MTD_WRITEABLE,
	}, {
		.name		= "U-Boot Config",
		.offset		= 0x010000,
		.size		= 0x030000,
	}, {
		.name		= "U-Boot",
		.offset		= 0x040000,
		.size		= 0x030000,
	}, {
		.name		= "linux",
		.offset		= 0x070000,
		.size		= 0x0e0000,
	}, {
		.name		= "rootfs",
		.offset		= 0x150000,
		.size		= 0x2a0000,
	}, {
		.name		= "CalibData",
		.offset		= 0x3f0000,
		.size		= 0x010000,
		.mask_flags	= MTD_WRITEABLE,
	}, {
		.name		= "firmware",
		.offset		= 0x070000,
		.size		= 0x380000,
	}
};

static struct flash_platform_data nbg460n_flash_data = {
	.parts		= nbg460n_partitions,
	.nr_parts       = ARRAY_SIZE(nbg460n_partitions),
};

static struct gpio_led nbg460n_leds_gpio[] __initdata = {
	{
		.name		= "nbg460n:green:power",
		.gpio		= NBG460N_GPIO_LED_POWER,
		.active_low	= 0,
		.default_trigger = "default-on",
	}, {
		.name		= "nbg460n:green:wps",
		.gpio		= NBG460N_GPIO_LED_WPS,
		.active_low	= 0,
	}, {
		.name		= "nbg460n:green:wlan",
		.gpio		= NBG460N_GPIO_LED_WLAN,
		.active_low	= 0,
	}, {
		/* Not really for controlling the LED,
		   when set low the LED blinks uncontrollable  */
		.name		= "nbg460n:green:wan",
		.gpio		= NBG460N_GPIO_LED_WAN,
		.active_low	= 0,
	}
};

static struct gpio_keys_button nbg460n_gpio_keys[] __initdata = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = NBG460N_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= NBG460N_GPIO_BTN_RESET,
		.active_low	= 1,
	}, {
		.desc		= "wps",
		.type		= EV_KEY,
		.code		= KEY_WPS_BUTTON,
		.debounce_interval = NBG460N_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= NBG460N_GPIO_BTN_WPS,
		.active_low	= 1,
	}
};

static struct i2c_gpio_platform_data nbg460n_i2c_device_platdata = {
	.sda_pin	= NBG460N_GPIO_PCF8563_SDA,
	.scl_pin	= NBG460N_GPIO_PCF8563_SCK,
	.udelay		= 10,
};

static struct platform_device nbg460n_i2c_device = {
	.name		= "i2c-gpio",
	.id		= -1,
	.num_resources	= 0,
	.resource	= NULL,
	.dev		= {
		.platform_data	= &nbg460n_i2c_device_platdata,
	},
};

static struct i2c_board_info nbg460n_i2c_devs[] __initdata = {
	{
		I2C_BOARD_INFO("pcf8563", 0x51),
	},
};

static void nbg460n_i2c_init(void)
{
	/* The gpio interface */
	platform_device_register(&nbg460n_i2c_device);
	/* I2C devices */
	i2c_register_board_info(0, nbg460n_i2c_devs,
				ARRAY_SIZE(nbg460n_i2c_devs));
}


static struct rtl8366_platform_data nbg460n_rtl8366s_data = {
	.gpio_sda	= NBG460N_GPIO_RTL8366_SDA,
	.gpio_sck	= NBG460N_GPIO_RTL8366_SCK,
};

static struct platform_device nbg460n_rtl8366s_device = {
	.name		= RTL8366S_DRIVER_NAME,
	.id		= -1,
	.dev = {
		.platform_data	= &nbg460n_rtl8366s_data,
	}
};

static void __init nbg460n_setup(void)
{
	/* end of bootloader sector contains mac address */
	u8 *mac = (u8 *) KSEG1ADDR(0x1fc0fff8);
	/* last sector contains wlan calib data */
	u8 *eeprom = (u8 *) KSEG1ADDR(0x1fff1000);

	/* LAN Port */
	ath79_init_mac(ath79_eth0_data.mac_addr, mac, 0);
	ath79_eth0_data.mii_bus_dev = &nbg460n_rtl8366s_device.dev;
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth0_data.speed = SPEED_1000;
	ath79_eth0_data.duplex = DUPLEX_FULL;

	/* WAN Port */
	ath79_init_mac(ath79_eth1_data.mac_addr, mac, 1);
	ath79_eth1_data.mii_bus_dev = &nbg460n_rtl8366s_device.dev;
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth1_data.phy_mask = 0x10;

	ath79_register_eth(0);
	ath79_register_eth(1);

	/* register the switch phy */
	platform_device_register(&nbg460n_rtl8366s_device);

	/* register flash */
	ath79_register_m25p80(&nbg460n_flash_data);

	ath79_register_wmac(eeprom, mac);

	/* register RTC chip */
	nbg460n_i2c_init();

	ath79_register_leds_gpio(-1, ARRAY_SIZE(nbg460n_leds_gpio),
				 nbg460n_leds_gpio);

	ath79_register_gpio_keys_polled(-1, NBG460N_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(nbg460n_gpio_keys),
					nbg460n_gpio_keys);
}

MIPS_MACHINE(ATH79_MACH_NBG460N, "NBG460N", "Zyxel NBG460N/550N/550NH",
	     nbg460n_setup);

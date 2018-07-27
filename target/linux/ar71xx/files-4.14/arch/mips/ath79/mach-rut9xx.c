/*
 * Teltonika RUT900 series boards support
 *
 * Copyright (C) 2018 Piotr Dymacz <pepe2k@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 */

#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/i2c-algo-bit.h>
#include <linux/i2c-gpio.h>
#include <linux/platform_data/pca953x.h>
#include <linux/platform_device.h>

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

#define RUT9XX_GPIO_LED_LAN1		14
#define RUT9XX_GPIO_LED_LAN2		13
#define RUT9XX_GPIO_LED_LAN3		22
#define RUT9XX_GPIO_LED_WAN		1

#define RUT9XX_PCA9539_GPIO_BASE	32
#define RUT9XX_PCA9539_GPIO_SIGNAL1	(0 + RUT9XX_PCA9539_GPIO_BASE)
#define RUT9XX_PCA9539_GPIO_SIGNAL2	(1 + RUT9XX_PCA9539_GPIO_BASE)
#define RUT9XX_PCA9539_GPIO_SIGNAL3	(2 + RUT9XX_PCA9539_GPIO_BASE)
#define RUT9XX_PCA9539_GPIO_SIGNAL4	(3 + RUT9XX_PCA9539_GPIO_BASE)
#define RUT9XX_PCA9539_GPIO_SIGNAL5	(4 + RUT9XX_PCA9539_GPIO_BASE)
#define RUT9XX_PCA9539_GPIO_STATUS_R	(5 + RUT9XX_PCA9539_GPIO_BASE)
#define RUT9XX_PCA9539_GPIO_STATUS_G	(6 + RUT9XX_PCA9539_GPIO_BASE)

#define RUT9XX_GPIO_BTN_RESET		15

#define RUT9XX_GPIO_I2C_SCK		16
#define RUT9XX_GPIO_I2C_SDA		17

#define RUT9XX_GPIO_EXT_LNA0		2

#define RUT9XX_WMAC_CALDATA_OFFSET	0x1000

#define RUT9XX_KEYS_POLL_INTERVAL	20
#define RUT9XX_KEYS_DEBOUNCE_INTERVAL	(3 * RUT9XX_KEYS_POLL_INTERVAL)

static struct gpio_led rut900_leds_gpio[] __initdata = {
	{
		.name		= "rut900:green:lan1",
		.gpio		= RUT9XX_GPIO_LED_LAN1,
		.active_low	= 1,
	}, {
		.name		= "rut900:green:lan2",
		.gpio		= RUT9XX_GPIO_LED_LAN2,
		.active_low	= 1,
	}, {
		.name		= "rut900:green:lan3",
		.gpio		= RUT9XX_GPIO_LED_LAN3,
		.active_low	= 1,
	}, {
		.name		= "rut900:green:signal1",
		.gpio		= RUT9XX_PCA9539_GPIO_SIGNAL1,
		.active_low	= 0,
	}, {
		.name		= "rut900:green:signal2",
		.gpio		= RUT9XX_PCA9539_GPIO_SIGNAL2,
		.active_low	= 0,
	}, {
		.name		= "rut900:green:signal3",
		.gpio		= RUT9XX_PCA9539_GPIO_SIGNAL3,
		.active_low	= 0,
	}, {
		.name		= "rut900:green:signal4",
		.gpio		= RUT9XX_PCA9539_GPIO_SIGNAL4,
		.active_low	= 0,
	}, {
		.name		= "rut900:green:signal5",
		.gpio		= RUT9XX_PCA9539_GPIO_SIGNAL5,
		.active_low	= 0,
	}, {
		.name		= "rut900:green:status",
		.gpio		= RUT9XX_PCA9539_GPIO_STATUS_G,
		.active_low	= 0,
	}, {
		.name		= "rut900:green:wan",
		.gpio		= RUT9XX_GPIO_LED_WAN,
		.active_low	= 1,
	}, {
		.name		= "rut900:red:status",
		.gpio		= RUT9XX_PCA9539_GPIO_STATUS_R,
		.active_low	= 0,
	},
};

static struct gpio_keys_button rut900_gpio_keys[] __initdata = {
	{
		.desc			= "reset",
		.type			= EV_KEY,
		.code			= KEY_RESTART,
		.debounce_interval	= RUT9XX_KEYS_DEBOUNCE_INTERVAL,
		.gpio			= RUT9XX_GPIO_BTN_RESET,
		.active_low		= 1,
	},
};

static struct i2c_gpio_platform_data rut900_i2c_gpio_data = {
	.sda_pin	= RUT9XX_GPIO_I2C_SDA,
	.scl_pin	= RUT9XX_GPIO_I2C_SCK,
	.udelay		= 10,
};

static struct platform_device rut900_i2c_device = {
	.name	= "i2c-gpio",
	.id	= 0,
	.dev	= {
		.platform_data = &rut900_i2c_gpio_data,
	},
};

static struct pca953x_platform_data rut9xx_pca9539_data = {
	.gpio_base	= RUT9XX_PCA9539_GPIO_BASE,
	.irq_base	= -1,
};

static struct i2c_board_info rut900_i2c_devs[] __initdata = {
	{
		I2C_BOARD_INFO("pca9539", 0x74),
		.platform_data = &rut9xx_pca9539_data,
	},
};

static void __init rut900_setup(void)
{
	u8 *art = (u8 *) KSEG1ADDR(0x1f030000);
	u8 *mac = (u8 *) KSEG1ADDR(0x1f020000);
	u8  wlan_mac[ETH_ALEN];

	ath79_register_m25p80(NULL);

	ath79_register_mdio(1, 0x0);

	ath79_switch_data.phy4_mii_en = 1;
	ath79_switch_data.phy_poll_mask = 0xf1;

	/* LAN */
	ath79_eth1_data.duplex = DUPLEX_FULL;
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_GMII;
	ath79_init_mac(ath79_eth1_data.mac_addr, mac, 0);
	ath79_register_eth(1);

	/* WAN */
	ath79_eth0_data.duplex = DUPLEX_FULL;
	ath79_eth0_data.mii_bus_dev = &ath79_mdio1_device.dev;
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ath79_eth0_data.phy_mask = BIT(4);
	ath79_eth0_data.speed = SPEED_100;
	ath79_init_mac(ath79_eth0_data.mac_addr, mac, 1);
	ath79_register_eth(0);

	platform_device_register(&rut900_i2c_device);
	i2c_register_board_info(0, rut900_i2c_devs,
				ARRAY_SIZE(rut900_i2c_devs));

	/* Disable JTAG (enables GPIO0-3) */
	ath79_gpio_function_enable(AR934X_GPIO_FUNC_JTAG_DISABLE);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(rut900_leds_gpio),
				 rut900_leds_gpio);

	ath79_register_gpio_keys_polled(-1, RUT9XX_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(rut900_gpio_keys),
					rut900_gpio_keys);

	ath79_wmac_set_ext_lna_gpio(0, RUT9XX_GPIO_EXT_LNA0);

	ath79_init_mac(wlan_mac, mac, 2);
	ath79_register_wmac(art + RUT9XX_WMAC_CALDATA_OFFSET, wlan_mac);

	ath79_register_usb();
}

MIPS_MACHINE(ATH79_MACH_RUT9XX, "RUT900", "Teltonika RUT900", rut900_setup);

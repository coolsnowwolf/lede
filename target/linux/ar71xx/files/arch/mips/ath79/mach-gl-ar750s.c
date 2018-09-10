/*
 *  GL.iNet GL-AR750S board support
 *
 *  Copyright (C) 2018 luochongjun <luochongjun@gl-inet.com>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/pci.h>
#include <linux/platform_device.h>
#include <linux/ath9k_platform.h>
#include <linux/etherdevice.h>

#include <linux/i2c.h>
#include <linux/i2c-gpio.h>

#include <asm/mach-ath79/ath79.h>
#include <asm/mach-ath79/irq.h>
#include <asm/mach-ath79/ar71xx_regs.h>

#include <linux/platform_data/phy-at803x.h>
#include <linux/ar8216_platform.h>

#include "common.h"
#include "dev-ap9x-pci.h"
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-spi.h"
#include "dev-m25p80.h"
#include "dev-wmac.h"
#include "dev-usb.h"
#include "machtypes.h"


#define GL_AR750S_KEYS_POLL_INTERVAL	20
#define GL_AR750S_KEYS_DEBOUNCE_INTERVAL	(3 * GL_AR750S_KEYS_POLL_INTERVAL)


#define GL_AR750S_GPIO_LED_WLAN2G       19
#define GL_AR750S_GPIO_LED_WLAN5G       20
#define GL_AR750S_GPIO_LED_POWER	1
#define GL_AR750S_GPIO_USB_POWER	7

#define GL_AR750S_GPIO_BTN_RESET	2
#define GL_AR750S_GPIO_BTN_RIGHT	8

#define GL_AR750S_MAC0_OFFSET             0x0000
#define GL_AR750S_WMAC_CALDATA_OFFSET     0x1000
#define GL_AR750S_PCI_CALDATA_OFFSET      0x5000

#define GL_AR750S_GPIO_I2C_SDA	5
#define GL_AR750S_GPIO_I2C_SCL	21



static struct spi_board_info gl_ar750s_spi_info[] = {
    {
        .bus_num    = 0,
        .chip_select    = 0,
        .max_speed_hz   = 25000000,
        .modalias   = "m25p80",
        .platform_data  = NULL,
    },
};

static struct ath79_spi_platform_data gl_ar750s_spi_data = {
    .bus_num        = 0,
    .num_chipselect     = 2,
};

static struct gpio_led gl_ar750s_leds_gpio[] __initdata = {
	{
		 .name           = "gl-ar750s:green:power",
		 .gpio           = GL_AR750S_GPIO_LED_POWER,
		.default_state  = LEDS_GPIO_DEFSTATE_KEEP,
		.active_low     = 1,
   	 },{
		 .name           = "gl-ar750s:green:usbpower",
		 .gpio           = GL_AR750S_GPIO_USB_POWER,
		.active_low     = 1,
   	 },{
		.name		= "gl-ar750s:green:wlan2g",
		.gpio		= GL_AR750S_GPIO_LED_WLAN2G,
		.active_low	= 1,
	},{
		.name		= "gl-ar750s:green:wlan5g",
		.gpio		= GL_AR750S_GPIO_LED_WLAN5G,
		.active_low	= 0,
	}
};

static struct gpio_keys_button gl_ar750s_gpio_keys[] __initdata = {
    {
        .desc                   = "reset",
        .type                   = EV_KEY,
        .code                   = KEY_RESTART,
        .debounce_interval      = GL_AR750S_KEYS_DEBOUNCE_INTERVAL,
        .gpio                   = GL_AR750S_GPIO_BTN_RESET,
        .active_low             = 1,
       }, {
        .desc                   = "right",
        .type                   = EV_KEY,
        .code                   = BTN_0,
        .debounce_interval      = GL_AR750S_KEYS_DEBOUNCE_INTERVAL,
        .gpio                   = GL_AR750S_GPIO_BTN_RIGHT,
        .active_low             = 1,
       },
};

static struct i2c_gpio_platform_data gl_ar750s_i2c_gpio_data = {
        .sda_pin        = GL_AR750S_GPIO_I2C_SDA,
        .scl_pin        = GL_AR750S_GPIO_I2C_SCL,
};

static struct platform_device gl_ar750s_i2c_gpio_device = {
        .name           = "i2c-gpio",
        .id             = 0,
        .dev = {
                .platform_data  = &gl_ar750s_i2c_gpio_data,
	}

};

static struct ar8327_pad_cfg gl_ar750s_ar8327_pad0_cfg = {
	.mode = AR8327_PAD_MAC_SGMII,
	.sgmii_delay_en = true,
};

static struct ar8327_platform_data gl_ar750s_ar8327_data = {
	.pad0_cfg = &gl_ar750s_ar8327_pad0_cfg,
	.port0_cfg = {
		.force_link = 1,
		.speed = AR8327_PORT_SPEED_1000,
		.duplex = 1,
		.txpause = 1,
		.rxpause = 1,
	},
};


static struct mdio_board_info gl_ar750s_mdio0_info[] = {
	{
		.bus_id = "ag71xx-mdio.0",
		.mdio_addr = 0,
		.platform_data = &gl_ar750s_ar8327_data,
	},
};

static void __init  gl_ar750s_setup(void)
{
	u8 *eeprom = (u8 *) KSEG1ADDR(0x1f050000);

	ath79_register_spi(&gl_ar750s_spi_data, gl_ar750s_spi_info, 1);

	ath79_init_mac(ath79_eth0_data.mac_addr,
	               eeprom + GL_AR750S_MAC0_OFFSET, 0);

	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_SGMII;
	ath79_eth0_data.mii_bus_dev = &ath79_mdio0_device.dev;
	ath79_eth0_data.phy_mask = BIT(0);

	mdiobus_register_board_info(gl_ar750s_mdio0_info,
	                            ARRAY_SIZE(gl_ar750s_mdio0_info));

	ath79_register_mdio(0, 0x00);
	ath79_register_eth(0);


	ath79_register_usb();


	ath79_register_wmac(eeprom + GL_AR750S_WMAC_CALDATA_OFFSET, NULL);


	ap91_pci_init(eeprom + GL_AR750S_PCI_CALDATA_OFFSET, NULL);

	platform_device_register(&gl_ar750s_i2c_gpio_device);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(gl_ar750s_leds_gpio),
	                         gl_ar750s_leds_gpio);

	ath79_register_gpio_keys_polled(-1, GL_AR750S_KEYS_POLL_INTERVAL,
	                                ARRAY_SIZE(gl_ar750s_gpio_keys),
	                                gl_ar750s_gpio_keys);
}


MIPS_MACHINE(ATH79_MACH_GL_AR750S, "GL-AR750S", "GL-AR750S",
         gl_ar750s_setup);

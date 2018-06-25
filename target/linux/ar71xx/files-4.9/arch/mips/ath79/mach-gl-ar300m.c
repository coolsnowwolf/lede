/*
 *  GLI AR300M(D) board support
 *
 *  Copyright (C) 2011 dongyuqi <729650915@qq.com>
 *  Copyright (C) 2011-2012 Gabor Juhos <juhosg@openwrt.org>
 *  Copyright (C) 2013 alzhao <alzhao@gmail.com>
 *  Copyright (C) 2014 Michel Stempin <michel.stempin@wanadoo.fr>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 */

#include <linux/gpio.h>
#include <linux/pci.h>

#include <asm/mach-ath79/ath79.h>
#include <asm/mach-ath79/ar71xx_regs.h>

#include "common.h"
#include "dev-ap9x-pci.h"
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-usb.h"
#include "dev-spi.h"
#include "dev-wmac.h"
#include "machtypes.h"
#include "pci.h"

#define GL_AR300M_GPIO_LED_USB		2
#define GL_AR300M_GPIO_LED_WLAN		14
#define GL_AR300M_GPIO_LED_LAN		13
#define GL_AR300M_GPIO_LED_SYSTEM	12
#define GL_AR300M_GPIO_BTN_RESET	3
#define GL_AR300M_GPIO_BTN_LEFT		0
#define GL_AR300M_GPIO_BTN_RIGHT	1

#define GL_AR300M_KEYS_POLL_INTERVAL		20	/* msecs */
#define GL_AR300M_KEYS_DEBOUNCE_INTERVAL	(3 * GL_AR300M_KEYS_POLL_INTERVAL)

#define GL_AR300M_MAC0_OFFSET	0
#define GL_AR300M_MAC1_OFFSET	6
#define GL_AR300M_WMAC_CALDATA_OFFSET	0x1000
#define GL_AR300M_PCIE_CALDATA_OFFSET	0x5000

static struct gpio_led gl_ar300m_leds_gpio[] __initdata = {
	{
		.name = "gl-ar300m:green:usb",
		.gpio = GL_AR300M_GPIO_LED_USB,
		.active_low = 0,
		.default_state = 1,
	},
	{
		.name = "gl-ar300m:green:wlan",
		.gpio = GL_AR300M_GPIO_LED_WLAN,
		.active_low = 1,
	},
	{
		.name = "gl-ar300m:green:lan",
		.gpio = GL_AR300M_GPIO_LED_LAN,
		.active_low = 1,
	},
	{
		.name = "gl-ar300m:green:system",
		.gpio = GL_AR300M_GPIO_LED_SYSTEM,
		.active_low = 1,
		.default_state = 1,
	},
};

static struct gpio_keys_button gl_ar300m_gpio_keys[] __initdata = {
	{
		.desc = "reset",
		.type = EV_KEY,
		.code = KEY_RESTART,
		.debounce_interval = GL_AR300M_KEYS_DEBOUNCE_INTERVAL,
		.gpio = GL_AR300M_GPIO_BTN_RESET,
		.active_low = 1,
	},
	{
		.desc = "button right",
		.type = EV_KEY,
		.code = BTN_0,
		.debounce_interval = GL_AR300M_KEYS_DEBOUNCE_INTERVAL,
		.gpio = GL_AR300M_GPIO_BTN_LEFT,
		.active_low = 0,
	},
	{
		.desc = "button left",
		.type = EV_KEY,
		.code = BTN_1,
		.debounce_interval = GL_AR300M_KEYS_DEBOUNCE_INTERVAL,
		.gpio = GL_AR300M_GPIO_BTN_RIGHT,
		.active_low = 0,
	},
};

static struct spi_board_info gl_ar300m_spi_info[] = {
	{
		.bus_num	= 0,
		.chip_select	= 0,
		.max_speed_hz	= 25000000,
		.modalias	= "m25p80",
		.platform_data	= NULL,
	},
	{
		.bus_num	= 0,
		.chip_select	= 1,
		.max_speed_hz	= 25000000,
		.modalias	= "ath79-spinand",
		.platform_data	= NULL,
	}
};

static struct ath79_spi_platform_data gl_ar300m_spi_data = {
	.bus_num		= 0,
	.num_chipselect		= 2,
};

static void __init gl_ar300m_setup(void)
{
	u8 *art = (u8 *) KSEG1ADDR(0x1fff0000);
	u8 tmpmac[ETH_ALEN];

	ath79_gpio_function_enable(AR934X_GPIO_FUNC_JTAG_DISABLE);
	ath79_register_spi(&gl_ar300m_spi_data, gl_ar300m_spi_info, 2);

	/* register gpio LEDs and keys */
	ath79_register_leds_gpio(-1, ARRAY_SIZE(gl_ar300m_leds_gpio),
				 gl_ar300m_leds_gpio);
	ath79_register_gpio_keys_polled(-1, GL_AR300M_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(gl_ar300m_gpio_keys),
					gl_ar300m_gpio_keys);

	ath79_register_mdio(0, 0x0);

	/* WAN */
	ath79_init_mac(ath79_eth0_data.mac_addr, art + GL_AR300M_MAC0_OFFSET, 0);
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ath79_eth0_data.speed = SPEED_100;
	ath79_eth0_data.duplex = DUPLEX_FULL;
	ath79_eth0_data.phy_mask = BIT(4);
	ath79_register_eth(0);

	/* LAN */
	ath79_init_mac(ath79_eth1_data.mac_addr, art + GL_AR300M_MAC1_OFFSET, 0);
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_GMII;
	ath79_eth1_data.speed = SPEED_1000;
	ath79_eth1_data.duplex = DUPLEX_FULL;
	ath79_switch_data.phy_poll_mask |= BIT(4);
	ath79_switch_data.phy4_mii_en = 1;
	ath79_register_eth(1);

	ath79_init_mac(tmpmac, art + GL_AR300M_WMAC_CALDATA_OFFSET + 2, 0);
	ath79_register_wmac(art + GL_AR300M_WMAC_CALDATA_OFFSET, tmpmac);

	/* enable usb */
	ath79_register_usb();
	/* enable pci */
	ath79_register_pci();
}

MIPS_MACHINE(ATH79_MACH_GL_AR300M, "GL-AR300M", "GL.iNet GL-AR300M", gl_ar300m_setup);

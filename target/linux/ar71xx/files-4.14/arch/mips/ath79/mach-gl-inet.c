/*
 *  GL-CONNECT iNet board support
 *
 *  Copyright (C) 2011 dongyuqi <729650915@qq.com>
 *  Copyright (C) 2011-2012 Gabor Juhos <juhosg@openwrt.org>
 *  Copyright (C) 2013 alzhao <alzhao@gmail.com>
 *  Copyright (C) 2014 Michel Stempin <michel.stempin@wanadoo.fr>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/gpio.h>

#include <asm/mach-ath79/ath79.h>

#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-usb.h"
#include "dev-wmac.h"
#include "machtypes.h"

#define GL_INET_GPIO_LED_WLAN		0
#define GL_INET_GPIO_LED_LAN		13
#define GL_INET_GPIO_BTN_RESET		11

#define GL_INET_KEYS_POLL_INTERVAL	20	/* msecs */
#define GL_INET_KEYS_DEBOUNCE_INTERVAL	(3 * GL_INET_KEYS_POLL_INTERVAL)

static const char * gl_inet_part_probes[] = {
	"tp-link", /* dont change, this will use tplink parser */
	NULL ,
};

static struct flash_platform_data gl_inet_flash_data = {
	.part_probes = gl_inet_part_probes,
};

static struct gpio_led gl_inet_leds_gpio[] __initdata = {
	{
		.name = "gl-inet:red:wlan",
		.gpio = GL_INET_GPIO_LED_WLAN,
		.active_low = 0,
	},
	{
		.name = "gl-inet:green:lan",
		.gpio = GL_INET_GPIO_LED_LAN,
		.active_low = 0,
		.default_state = 1,
	},
};

static struct gpio_keys_button gl_inet_gpio_keys[] __initdata = {
	{
		.desc = "reset",
		.type = EV_KEY,
		.code = KEY_RESTART,
		.debounce_interval = GL_INET_KEYS_DEBOUNCE_INTERVAL,
		.gpio = GL_INET_GPIO_BTN_RESET,
		.active_low = 0,
	}
};

static void __init gl_inet_setup(void)
{
	/* get the mac address which is stored in the 1st 64k uboot MTD */
	u8 *mac = (u8 *) KSEG1ADDR(0x1f01fc00);

	/* get the art address, which is the last 64K. By using
	   0x1fff1000, it doesn't matter it is 4M, 8M or 16M flash */
	u8 *ee = (u8 *) KSEG1ADDR(0x1fff1000);

	/* disable PHY_SWAP and PHY_ADDR_SWAP bits */
	ath79_setup_ar933x_phy4_switch(false, false);

	/* register flash. MTD will use tp-link parser to parser MTD */
	ath79_register_m25p80(&gl_inet_flash_data);

	/* register gpio LEDs and keys */
	ath79_register_leds_gpio(-1, ARRAY_SIZE(gl_inet_leds_gpio),
				 gl_inet_leds_gpio);
	ath79_register_gpio_keys_polled(-1, GL_INET_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(gl_inet_gpio_keys),
					gl_inet_gpio_keys);

	/* enable usb */
	ath79_register_usb();

	/* register eth0 as WAN, eth1 as LAN */
	ath79_init_mac(ath79_eth0_data.mac_addr, mac, 0);
	ath79_register_mdio(0, 0x0);
	ath79_register_eth(0);
	ath79_init_mac(ath79_eth1_data.mac_addr, mac, 0);
	ath79_register_eth(1);

	/* register wireless mac with cal data */
	ath79_register_wmac(ee, mac);
}

MIPS_MACHINE(ATH79_MACH_GL_INET, "GL-INET", "GL-CONNECT INET v1",
	     gl_inet_setup);

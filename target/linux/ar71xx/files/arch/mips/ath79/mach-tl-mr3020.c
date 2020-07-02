/*
 *  TP-LINK TL-MR3020 board support
 *
 *  Copyright (C) 2011 dongyuqi <729650915@qq.com>
 *  Copyright (C) 2011-2012 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/gpio.h>

#include <asm/mach-ath79/ath79.h>
#include <asm/mach-ath79/ar71xx_regs.h>

#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-usb.h"
#include "dev-wmac.h"
#include "machtypes.h"

#define TL_MR3020_GPIO_LED_3G		27
#define TL_MR3020_GPIO_LED_WLAN		0
#define TL_MR3020_GPIO_LED_LAN		17
#define TL_MR3020_GPIO_LED_WPS		26

#define TL_MR3020_GPIO_BTN_WPS		11
#define TL_MR3020_GPIO_BTN_SW1		18
#define TL_MR3020_GPIO_BTN_SW2		20

#define TL_MR3020_GPIO_USB_POWER	8

#define TL_MR3020_KEYS_POLL_INTERVAL	20	/* msecs */
#define TL_MR3020_KEYS_DEBOUNCE_INTERVAL	(3 * TL_MR3020_KEYS_POLL_INTERVAL)

static const char *tl_mr3020_part_probes[] = {
	"tp-link",
	NULL,
};

static struct flash_platform_data tl_mr3020_flash_data = {
	.part_probes	= tl_mr3020_part_probes,
};

static struct gpio_led tl_mr3020_leds_gpio[] __initdata = {
	{
		.name		= "tp-link:green:3g",
		.gpio		= TL_MR3020_GPIO_LED_3G,
		.active_low	= 1,
	},
	{
		.name		= "tp-link:green:wlan",
		.gpio		= TL_MR3020_GPIO_LED_WLAN,
		.active_low	= 0,
	},
	{
		.name		= "tp-link:green:lan",
		.gpio		= TL_MR3020_GPIO_LED_LAN,
		.active_low	= 1,
	},
	{
		.name		= "tp-link:green:wps",
		.gpio		= TL_MR3020_GPIO_LED_WPS,
		.active_low	= 1,
	},
};

static struct gpio_keys_button tl_mr3020_gpio_keys[] __initdata = {
	{
		.desc		= "wps",
		.type		= EV_KEY,
		.code		= KEY_WPS_BUTTON,
		.debounce_interval = TL_MR3020_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= TL_MR3020_GPIO_BTN_WPS,
		.active_low	= 0,
	},
	{
		.desc		= "sw1",
		.type		= EV_SW,
		.code		= BTN_0,
		.debounce_interval = TL_MR3020_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= TL_MR3020_GPIO_BTN_SW1,
		.active_low	= 0,
	},
	{
		.desc		= "sw2",
		.type		= EV_SW,
		.code		= BTN_1,
		.debounce_interval = TL_MR3020_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= TL_MR3020_GPIO_BTN_SW2,
		.active_low	= 0,
	}
};

static void __init tl_mr3020_setup(void)
{
	u8 *mac = (u8 *) KSEG1ADDR(0x1f01fc00);
	u8 *ee = (u8 *) KSEG1ADDR(0x1fff1000);

	/* disable PHY_SWAP and PHY_ADDR_SWAP bits */
	ath79_setup_ar933x_phy4_switch(false, false);

	ath79_register_m25p80(&tl_mr3020_flash_data);
	ath79_register_leds_gpio(-1, ARRAY_SIZE(tl_mr3020_leds_gpio),
				 tl_mr3020_leds_gpio);
	ath79_register_gpio_keys_polled(-1, TL_MR3020_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(tl_mr3020_gpio_keys),
					tl_mr3020_gpio_keys);

	gpio_request_one(TL_MR3020_GPIO_USB_POWER,
			 GPIOF_OUT_INIT_HIGH | GPIOF_EXPORT_DIR_FIXED,
			 "USB power");
	ath79_register_usb();

	ath79_init_mac(ath79_eth0_data.mac_addr, mac, 0);

	ath79_register_mdio(0, 0x0);
	ath79_register_eth(0);
	ath79_register_wmac(ee, mac);
}

MIPS_MACHINE(ATH79_MACH_TL_MR3020, "TL-MR3020", "TP-LINK TL-MR3020",
	     tl_mr3020_setup);

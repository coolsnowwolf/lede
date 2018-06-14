/*
 *  TP-LINK TL-MR11U/TL-MR3040 board support
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

#include "common.h"
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-usb.h"
#include "dev-wmac.h"
#include "machtypes.h"

#define TL_MR11U_GPIO_LED_3G		27
#define TL_MR11U_GPIO_LED_WLAN		26
#define TL_MR11U_GPIO_LED_LAN		17

#define TL_MR11U_GPIO_BTN_WPS		20
#define TL_MR11U_GPIO_BTN_RESET		11

#define TL_MR11U_GPIO_USB_POWER		8
#define TL_MR3040_GPIO_USB_POWER	18

#define TL_MR3040_V2_GPIO_BTN_SW1	19
#define TL_MR3040_V2_GPIO_BTN_SW2	20

#define TL_MR11U_KEYS_POLL_INTERVAL	20	/* msecs */
#define TL_MR11U_KEYS_DEBOUNCE_INTERVAL	(3 * TL_MR11U_KEYS_POLL_INTERVAL)

static const char *tl_mr11u_part_probes[] = {
	"tp-link",
	NULL,
};

static struct flash_platform_data tl_mr11u_flash_data = {
	.part_probes	= tl_mr11u_part_probes,
};

static struct gpio_led tl_mr11u_leds_gpio[] __initdata = {
	{
		.name		= "tp-link:green:3g",
		.gpio		= TL_MR11U_GPIO_LED_3G,
		.active_low	= 1,
	},
	{
		.name		= "tp-link:green:wlan",
		.gpio		= TL_MR11U_GPIO_LED_WLAN,
		.active_low	= 1,
	},
	{
		.name		= "tp-link:green:lan",
		.gpio		= TL_MR11U_GPIO_LED_LAN,
		.active_low	= 1,
	}
};

static struct gpio_keys_button tl_mr11u_gpio_keys[] __initdata = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = TL_MR11U_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= TL_MR11U_GPIO_BTN_RESET,
		.active_low	= 0,
	},
	{
		.desc		= "wps",
		.type		= EV_KEY,
		.code		= KEY_WPS_BUTTON,
		.debounce_interval = TL_MR11U_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= TL_MR11U_GPIO_BTN_WPS,
		.active_low	= 0,
	},
};

static struct gpio_keys_button tl_mr3040_v2_gpio_keys[] __initdata = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = TL_MR11U_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= TL_MR11U_GPIO_BTN_RESET,
		.active_low	= 0,
	},
	{
		.desc		= "sw1",
		.type		= EV_SW,
		.code		= BTN_0,
		.debounce_interval = TL_MR11U_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= TL_MR3040_V2_GPIO_BTN_SW1,
		.active_low	= 0,
	},
	{
		.desc		= "sw2",
		.type		= EV_SW,
		.code		= BTN_1,
		.debounce_interval = TL_MR11U_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= TL_MR3040_V2_GPIO_BTN_SW2,
		.active_low	= 0,
	}
};

static void __init common_setup(void)
{
	u8 *mac = (u8 *) KSEG1ADDR(0x1f01fc00);
	u8 *ee = (u8 *) KSEG1ADDR(0x1fff1000);

	/* Disable hardware control LAN1 and LAN2 LEDs, enabling GPIO14 and GPIO15 */
	ath79_gpio_function_disable(AR933X_GPIO_FUNC_ETH_SWITCH_LED1_EN |
				    AR933X_GPIO_FUNC_ETH_SWITCH_LED2_EN);

	/* disable PHY_SWAP and PHY_ADDR_SWAP bits */
	ath79_setup_ar933x_phy4_switch(false, false);

	ath79_register_m25p80(&tl_mr11u_flash_data);
	ath79_register_leds_gpio(-1, ARRAY_SIZE(tl_mr11u_leds_gpio),
				 tl_mr11u_leds_gpio);

	ath79_register_usb();

	ath79_init_mac(ath79_eth0_data.mac_addr, mac, 0);

	ath79_register_mdio(0, 0x0);
	ath79_register_eth(0);

	ath79_register_wmac(ee, mac);
}

static void __init tl_mr11u_setup(void)
{
	common_setup();

	ath79_register_gpio_keys_polled(-1, TL_MR11U_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(tl_mr11u_gpio_keys),
					tl_mr11u_gpio_keys);
	gpio_request_one(TL_MR11U_GPIO_USB_POWER,
			 GPIOF_OUT_INIT_HIGH | GPIOF_EXPORT_DIR_FIXED,
			 "USB power");
}

MIPS_MACHINE(ATH79_MACH_TL_MR11U, "TL-MR11U", "TP-LINK TL-MR11U",
	     tl_mr11u_setup);

static void __init tl_mr3040_setup(void)
{
	common_setup();

	ath79_register_gpio_keys_polled(-1, TL_MR11U_KEYS_POLL_INTERVAL,
					1, tl_mr11u_gpio_keys);
	gpio_request_one(TL_MR3040_GPIO_USB_POWER,
			 GPIOF_OUT_INIT_HIGH | GPIOF_EXPORT_DIR_FIXED,
			 "USB power");
}

MIPS_MACHINE(ATH79_MACH_TL_MR3040, "TL-MR3040", "TP-LINK TL-MR3040",
	     tl_mr3040_setup);

static void __init tl_mr3040_v2_setup(void)
{
	common_setup();

	ath79_register_gpio_keys_polled(-1, TL_MR11U_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(tl_mr3040_v2_gpio_keys),
					tl_mr3040_v2_gpio_keys);
	gpio_request_one(TL_MR3040_GPIO_USB_POWER,
			 GPIOF_OUT_INIT_HIGH | GPIOF_EXPORT_DIR_FIXED,
			 "USB power");
}

MIPS_MACHINE(ATH79_MACH_TL_MR3040_V2, "TL-MR3040-v2", "TP-LINK TL-MR3040 v2",
	     tl_mr3040_v2_setup);

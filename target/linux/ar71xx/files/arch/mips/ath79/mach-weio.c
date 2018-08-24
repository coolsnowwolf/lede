/**
 * WEIO Web Of Things Platform
 *
 * Copyright (C) 2013 Drasko DRASKOVIC and Uros PETREVSKI
 *
 *              ##      ## ######## ####  #######  
 *              ##  ##  ## ##        ##  ##     ## 
 *              ##  ##  ## ##        ##  ##     ## 
 *              ##  ##  ## ######    ##  ##     ## 
 *              ##  ##  ## ##        ##  ##     ## 
 *              ##  ##  ## ##        ##  ##     ## 
 *               ###  ###  ######## ####  #######
 *
 *                   Web Of Things Platform
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * Authors : 
 * Drasko DRASKOVIC <drasko.draskovic@gmail.com>
 * Uros PETREVSKI <uros@nodesign.net>
 */

#include <asm/mach-ath79/ath79.h>
#include <asm/mach-ath79/ar71xx_regs.h>
#include <linux/i2c-gpio.h>
#include <linux/platform_device.h>
#include "common.h"
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-spi.h"
#include "dev-usb.h"
#include "dev-wmac.h"
#include "machtypes.h"

#define WEIO_GPIO_LED_STA		1
#define WEIO_GPIO_LED_AP		16

#define WEIO_GPIO_BTN_AP		20
#define WEIO_GPIO_BTN_RESET		23

#define WEIO_KEYS_POLL_INTERVAL		20	/* msecs */
#define WEIO_KEYS_DEBOUNCE_INTERVAL	(3 * WEIO_KEYS_POLL_INTERVAL)

#define WEIO_MAC0_OFFSET			0x0000
#define WEIO_MAC1_OFFSET			0x0006
#define WEIO_CALDATA_OFFSET			0x1000
#define WEIO_WMAC_MAC_OFFSET		0x1002

static struct gpio_led weio_leds_gpio[] __initdata = {
	{
		.name		= "weio:green:sta",
		.gpio		= WEIO_GPIO_LED_STA,
		.active_low	= 1,
		.default_state = LEDS_GPIO_DEFSTATE_ON,
	},
	{
		.name		= "weio:green:ap",
		.gpio		= WEIO_GPIO_LED_AP,
		.active_low	= 1,
		.default_state = LEDS_GPIO_DEFSTATE_ON,
	}
};

static struct gpio_keys_button weio_gpio_keys[] __initdata = {
	{
		.desc		= "ap button",
		.type		= EV_KEY,
		.code		= BTN_0,
		.debounce_interval = WEIO_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= WEIO_GPIO_BTN_AP,
		.active_low	= 1,
	},
	{
		.desc		= "soft-reset button",
		.type		= EV_KEY,
		.code		= BTN_1,
		.debounce_interval = WEIO_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= WEIO_GPIO_BTN_RESET,
		.active_low	= 1,
	}
};

static struct i2c_gpio_platform_data weio_i2c_gpio_data = {
	.sda_pin        = 18,
	.scl_pin        = 19,
};

static struct platform_device weio_i2c_gpio = {
	.name           = "i2c-gpio",
	.id             = 0,
	.dev            = {
		.platform_data  = &weio_i2c_gpio_data,
	},
};

static void __init weio_common_setup(void)
{
	u8 *art = (u8 *) KSEG1ADDR(0x1fff0000);

	ath79_register_m25p80(NULL);
	ath79_register_wmac(art + WEIO_CALDATA_OFFSET, art + WEIO_WMAC_MAC_OFFSET);
}

static void __init weio_setup(void)
{
	weio_common_setup();

	ath79_gpio_function_disable(AR933X_GPIO_FUNC_ETH_SWITCH_LED0_EN |
				AR933X_GPIO_FUNC_ETH_SWITCH_LED1_EN |
				AR933X_GPIO_FUNC_ETH_SWITCH_LED2_EN |
				AR933X_GPIO_FUNC_ETH_SWITCH_LED3_EN |
				AR933X_GPIO_FUNC_ETH_SWITCH_LED4_EN);

	platform_device_register(&weio_i2c_gpio);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(weio_leds_gpio),
				weio_leds_gpio);

	ath79_register_gpio_keys_polled(-1, WEIO_KEYS_POLL_INTERVAL,
				ARRAY_SIZE(weio_gpio_keys),
				weio_gpio_keys);

	ath79_register_usb();
}

MIPS_MACHINE(ATH79_MACH_WEIO, "WEIO", "WeIO board", weio_setup);

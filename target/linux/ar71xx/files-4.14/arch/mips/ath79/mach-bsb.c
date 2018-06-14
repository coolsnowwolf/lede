/*
 *  Smart Electronics Black Swift board support
 *
 *  Copyright (C) 2014 Dmitriy Zherebkov dzh@black-swift.com
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <asm/mach-ath79/ath79.h>
#include <asm/mach-ath79/ar71xx_regs.h>
#include "common.h"
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-spi.h"
#include "dev-usb.h"
#include "dev-wmac.h"
#include "machtypes.h"

#define BSB_GPIO_LED_SYS		27

#define BSB_GPIO_BTN_RESET		11

#define BSB_KEYS_POLL_INTERVAL		20	/* msecs */
#define BSB_KEYS_DEBOUNCE_INTERVAL	(3 * BSB_KEYS_POLL_INTERVAL)

#define BSB_MAC_OFFSET			0x0000
#define BSB_CALDATA_OFFSET		0x1000

static struct gpio_led bsb_leds_gpio[] __initdata = {
	{
		.name		= "bsb:red:sys",
		.gpio		= BSB_GPIO_LED_SYS,
		.active_low	= 1,
	}
};

static struct gpio_keys_button bsb_gpio_keys[] __initdata = {
	{
		.desc		= "reset button",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = BSB_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= BSB_GPIO_BTN_RESET,
		.active_low	= 1,
	},
};

static void __init bsb_setup(void)
{
	u8 *art = (u8 *) KSEG1ADDR(0x1fff0000);

	/* disable PHY_SWAP and PHY_ADDR_SWAP bits */
	ath79_setup_ar933x_phy4_switch(false,false);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(bsb_leds_gpio),
				 bsb_leds_gpio);
	ath79_register_gpio_keys_polled(-1, BSB_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(bsb_gpio_keys),
					bsb_gpio_keys);

	ath79_register_usb();

	ath79_register_m25p80(NULL);

	ath79_init_mac(ath79_eth0_data.mac_addr, art + BSB_MAC_OFFSET, 1);
	ath79_init_mac(ath79_eth1_data.mac_addr, art + BSB_MAC_OFFSET, 2);

	ath79_register_mdio(0, 0x0);

	ath79_register_eth(0);
	ath79_register_eth(1);

	ath79_register_wmac(art + BSB_CALDATA_OFFSET,
			    art + BSB_MAC_OFFSET);
}

MIPS_MACHINE(ATH79_MACH_BSB, "BSB", "Smart Electronics Black Swift board",
		bsb_setup);


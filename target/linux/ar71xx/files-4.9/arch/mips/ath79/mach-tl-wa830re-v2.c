/*
 *  TP-LINK TL-WA830RE v2 board support
 *
 *  Copyright (C) 2014 Fredrik Jonson <fredrik@famjonson.se>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/gpio.h>
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

#define TL_WA830REV2_GPIO_LED_WLAN	13
#define TL_WA830REV2_GPIO_LED_QSS	15
#define TL_WA830REV2_GPIO_LED_LAN	18
#define TL_WA830REV2_GPIO_LED_SYSTEM	14

#define TL_WA830REV2_GPIO_BTN_RESET	17
#define TL_WA830REV2_GPIO_SW_RFKILL	16	/* WPS for MR3420 v2 */

#define TL_WA830REV2_GPIO_USB_POWER	4

#define TL_WA830REV2_KEYS_POLL_INTERVAL	20	/* msecs */
#define TL_WA830REV2_KEYS_DEBOUNCE_INTERVAL (3 * TL_WA830REV2_KEYS_POLL_INTERVAL)

static const char *tl_wa830re_v2_part_probes[] = {
	"tp-link",
	NULL,
};

static struct flash_platform_data tl_wa830re_v2_flash_data = {
	.part_probes	= tl_wa830re_v2_part_probes,
};

static struct gpio_led tl_wa830re_v2_leds_gpio[] __initdata = {
	{
		.name		= "tp-link:green:qss",
		.gpio		= TL_WA830REV2_GPIO_LED_QSS,
		.active_low	= 1,
	}, {
		.name		= "tp-link:green:system",
		.gpio		= TL_WA830REV2_GPIO_LED_SYSTEM,
		.active_low	= 1,
	}, {
		.name		= "tp-link:green:lan",
		.gpio		= TL_WA830REV2_GPIO_LED_LAN,
		.active_low	= 1,
	}, {
		.name		= "tp-link:green:wlan",
		.gpio		= TL_WA830REV2_GPIO_LED_WLAN,
		.active_low	= 1,
	},
};

static struct gpio_keys_button tl_wa830re_v2_gpio_keys[] __initdata = {
	{
		.desc		= "Reset button",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = TL_WA830REV2_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= TL_WA830REV2_GPIO_BTN_RESET,
		.active_low	= 1,
	}, {
		.desc		= "RFKILL switch",
		.type		= EV_SW,
		.code		= KEY_RFKILL,
		.debounce_interval = TL_WA830REV2_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= TL_WA830REV2_GPIO_SW_RFKILL,
		.active_low	= 0,
	}
};

static void __init tl_ap123_setup(void)
{
	u8 *mac = (u8 *) KSEG1ADDR(0x1f01fc00);
	u8 *ee = (u8 *) KSEG1ADDR(0x1fff1000);

	/* Disable JTAG, enabling GPIOs 0-3 */
	/* Configure OBS4 line, for GPIO 4*/
	ath79_gpio_function_setup(AR934X_GPIO_FUNC_JTAG_DISABLE,
				 AR934X_GPIO_FUNC_CLK_OBS4_EN);

	/* config gpio4 as normal gpio function */
	ath79_gpio_output_select(TL_WA830REV2_GPIO_USB_POWER,
				 AR934X_GPIO_OUT_GPIO);

	ath79_register_m25p80(&tl_wa830re_v2_flash_data);

	ath79_setup_ar934x_eth_cfg(AR934X_ETH_CFG_SW_PHY_SWAP);

	ath79_register_mdio(1, 0x0);

	ath79_init_mac(ath79_eth0_data.mac_addr, mac, 0);

	/* GMAC0 is connected to the PHY0 of the internal switch */
	ath79_switch_data.phy4_mii_en = 1;
	ath79_switch_data.phy_poll_mask = BIT(0);
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ath79_eth0_data.phy_mask = BIT(0);
	ath79_eth0_data.mii_bus_dev = &ath79_mdio1_device.dev;
	ath79_register_eth(0);

	ath79_register_wmac(ee, mac);
}

static void __init tl_wa830re_v2_setup(void)
{
	tl_ap123_setup();

	ath79_register_leds_gpio(-1, ARRAY_SIZE(tl_wa830re_v2_leds_gpio) - 1,
				 tl_wa830re_v2_leds_gpio);

	ath79_register_gpio_keys_polled(1, TL_WA830REV2_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(tl_wa830re_v2_gpio_keys),
					tl_wa830re_v2_gpio_keys);
}

MIPS_MACHINE(ATH79_MACH_TL_WA830RE_V2, "TL-WA830RE-v2", "TP-LINK TL-WA830RE v2",
	     tl_wa830re_v2_setup);

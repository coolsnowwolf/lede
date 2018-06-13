/*
 * TP-LINK TL-MR6400 board support
 *
 *  Copyright (C) 2017 Filip Moc <lede@moc6.cz>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * The name of the author may not be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY AUTHOR "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DAMAGES ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE.
 */

#include <linux/gpio.h>
#include <linux/platform_device.h>

#include <asm/mach-ath79/ar71xx_regs.h>

#include "common.h"
#include "dev-gpio-buttons.h"
#include "dev-eth.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-usb.h"
#include "dev-wmac.h"
#include "machtypes.h"

#define TL_MR6400_GPIO_LTE_POWER	4
#define TL_MR6400_GPIO_BTN_RESET	12	/* SW2 */
#define TL_MR6400_GPIO_BTN_RFKILL	14	/* SW3 */
#define TL_MR6400_GPIO_LED_WAN		0	/* D12 */
#define TL_MR6400_GPIO_LED_4G		1	/* D11 */
#define TL_MR6400_GPIO_LED_WPS		3	/* D5  */
#define TL_MR6400_GPIO_LED_WLAN		11	/* D3  */
#define TL_MR6400_GPIO_LED_POWER	13	/* D2  */
#define TL_MR6400_GPIO_LED_LAN		16	/* D4  */

#define TL_MR6400_KEYS_POLL_INTERVAL	20 /* msecs */
#define TL_MR6400_KEYS_DEBOUNCE_INTERVAL (3 * TL_MR6400_KEYS_POLL_INTERVAL)

#define TL_MR6400_WMAC_CALDATA_OFFSET	0x1000

static const char *tl_mr6400_part_probes[] = {
	"tp-link",
	NULL,
};

static struct flash_platform_data tl_mr6400_flash_data = {
	.part_probes	= tl_mr6400_part_probes,
	.type		= "w25q64",
};

static struct gpio_led tl_mr6400_leds_gpio[] __initdata = {
	{
		.name		= "tp-link:white:wan",
		.gpio		= TL_MR6400_GPIO_LED_WAN,
		.active_low	= 0,
	},
	{
		.name		= "tp-link:white:4g",
		.gpio		= TL_MR6400_GPIO_LED_4G,
		.active_low	= 0,
	},
	{
		.name		= "tp-link:white:wps",
		.gpio		= TL_MR6400_GPIO_LED_WPS,
		.active_low	= 0,
	},
	{
		.name		= "tp-link:white:wlan",
		.gpio		= TL_MR6400_GPIO_LED_WLAN,
		.active_low	= 0,
	},
	{
		.name		= "tp-link:white:power",
		.gpio		= TL_MR6400_GPIO_LED_POWER,
		.active_low	= 0,
	},
	{
		.name		= "tp-link:white:lan",
		.gpio		= TL_MR6400_GPIO_LED_LAN,
		.active_low	= 0,
	},
};

static struct gpio_keys_button tl_mr6400_gpio_keys[] __initdata = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = TL_MR6400_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= TL_MR6400_GPIO_BTN_RESET,
		.active_low	= 1,
	},
	{
		.desc		= "rfkill",
		.type		= EV_KEY,
		.code		= KEY_RFKILL,
		.debounce_interval = TL_MR6400_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= TL_MR6400_GPIO_BTN_RFKILL,
		.active_low	= 1,
	},
};

static void __init tl_mr6400_setup(void)
{
	u8 *mac = (u8 *) KSEG1ADDR(0x1f01fc00);
	u8 *art = (u8 *) KSEG1ADDR(0x1fff0000);

	ath79_register_m25p80(&tl_mr6400_flash_data);

	ath79_register_mdio(0, 0x0);

	/* LAN1, LAN2, LAN3 */
	ath79_switch_data.phy4_mii_en = 1;
	ath79_switch_data.phy_poll_mask |= BIT(0);
	ath79_eth1_data.duplex = DUPLEX_FULL;
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_GMII;
	ath79_eth1_data.speed = SPEED_1000;
	ath79_init_mac(ath79_eth1_data.mac_addr, mac, -1);
	ath79_register_eth(1);

	/* LAN4 / WAN */
	ath79_eth0_data.phy_mask = BIT(0);
	ath79_eth0_data.duplex = DUPLEX_FULL;
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ath79_eth0_data.speed = SPEED_100;
	ath79_init_mac(ath79_eth0_data.mac_addr, mac, 1);
	ath79_register_eth(0);

	ath79_register_wmac(art + TL_MR6400_WMAC_CALDATA_OFFSET, mac);

	ath79_register_leds_gpio(-1,
				 ARRAY_SIZE(tl_mr6400_leds_gpio),
				 tl_mr6400_leds_gpio);

	ath79_register_gpio_keys_polled(-1,
					TL_MR6400_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(tl_mr6400_gpio_keys),
					tl_mr6400_gpio_keys);

	gpio_request_one(TL_MR6400_GPIO_LTE_POWER,
			 GPIOF_OUT_INIT_LOW | GPIOF_EXPORT_DIR_FIXED | GPIOF_ACTIVE_LOW,
			 "LTE power");
	ath79_register_usb();
}

MIPS_MACHINE(ATH79_MACH_TL_MR6400, "TL-MR6400", "TP-LINK TL-MR6400",
	     tl_mr6400_setup);

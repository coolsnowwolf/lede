/*
 *  Support for TP-Link boards:
 *  - TL-WA750RE v1
 *  - TL-WA801ND v2
 *  - TL-WA850RE v1/v2
 *  - TL-WA855RE v1
 *  - TL-WA901ND v3
 *
 *  Copyright (C) 2013 Martijn Zilverschoon <thefriedzombie@gmail.com>
 *  Copyright (C) 2013 Jiri Pirko <jiri@resnulli.us>
 *  Copyright (C) 2017 Piotr Dymacz <pepe2k@gmail.com>
 *  Copyright (C) 2017 Federico Cappon <dududede371@gmail.com>
 *  Copyright (C) 2017 Nicolò Veronese <nicveronese@gmail.com>
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
#include "dev-wmac.h"
#include "machtypes.h"

#define TL_WAX50RE_GPIO_LED_LAN		20
#define TL_WAX50RE_GPIO_LED_WLAN	13
#define TL_WAX50RE_GPIO_LED_RE		15
#define TL_WAX50RE_GPIO_LED_SIGNAL1	0
#define TL_WAX50RE_GPIO_LED_SIGNAL2	1
#define TL_WAX50RE_GPIO_LED_SIGNAL3	2
#define TL_WAX50RE_GPIO_LED_SIGNAL4	3
#define TL_WAX50RE_GPIO_LED_SIGNAL5	4

#define TL_WA850RE_V2_GPIO_LED_LAN	14
#define TL_WA850RE_V2_GPIO_LED_RE	12
#define TL_WA850RE_V2_GPIO_LED_SIGNAL1	0
#define TL_WA850RE_V2_GPIO_LED_SIGNAL2	1
#define TL_WA850RE_V2_GPIO_LED_SIGNAL3	2
#define TL_WA850RE_V2_GPIO_LED_SIGNAL4	3
#define TL_WA850RE_V2_GPIO_LED_SIGNAL5	4
#define TL_WA850RE_V2_GPIO_LED_WLAN	13

#define TL_WA850RE_V2_GPIO_ENABLE_LEDS	15

#define TL_WA855REV1_GPIO_LED_RED	11
#define TL_WA855REV1_GPIO_LED_GREEN	12

#define TL_WA860RE_GPIO_LED_WLAN_ORANGE	0
#define TL_WA860RE_GPIO_LED_WLAN_GREEN	2
#define TL_WA860RE_GPIO_LED_POWER_ORANGE	12
#define TL_WA860RE_GPIO_LED_POWER_GREEN	14
#define TL_WA860RE_GPIO_LED_LAN		20

#define TL_WA801ND_V2_GPIO_LED_LAN	18
#define TL_WA801ND_V2_GPIO_LED_SYSTEM	14

#define TL_WAX50RE_GPIO_BTN_RESET	17
#define TL_WAX50RE_GPIO_BTN_WPS		16

#define TL_WA860RE_GPIO_BTN_RESET	17
#define TL_WA860RE_GPIO_BTN_WPS		16
#define TL_WA860RE_GPIO_BTN_ONOFF	11

#define TL_WAX50RE_KEYS_POLL_INTERVAL	20	/* msecs */
#define TL_WAX50RE_KEYS_DEBOUNCE_INTERVAL (3 * TL_WAX50RE_KEYS_POLL_INTERVAL)

static const char *tl_wax50re_part_probes[] = {
	"tp-link",
	NULL,
};

static struct flash_platform_data tl_wax50re_flash_data = {
	.part_probes	= tl_wax50re_part_probes,
};

static struct gpio_led tl_wa750re_leds_gpio[] __initdata = {
	{
		.name		= "tp-link:orange:lan",
		.gpio		= TL_WAX50RE_GPIO_LED_LAN,
		.active_low	= 1,
	}, {
		.name		= "tp-link:orange:wlan",
		.gpio		= TL_WAX50RE_GPIO_LED_WLAN,
		.active_low	= 1,
	}, {
		.name		= "tp-link:orange:re",
		.gpio		= TL_WAX50RE_GPIO_LED_RE,
		.active_low	= 1,
	}, {
		.name		= "tp-link:orange:signal1",
		.gpio		= TL_WAX50RE_GPIO_LED_SIGNAL1,
		.active_low	= 1,
	}, {
		.name		= "tp-link:orange:signal2",
		.gpio		= TL_WAX50RE_GPIO_LED_SIGNAL2,
		.active_low	= 1,
	}, {
		.name		= "tp-link:orange:signal3",
		.gpio		= TL_WAX50RE_GPIO_LED_SIGNAL3,
		.active_low	= 1,
	}, {
		.name		= "tp-link:orange:signal4",
		.gpio		= TL_WAX50RE_GPIO_LED_SIGNAL4,
		.active_low	= 1,
	}, {
		.name		= "tp-link:orange:signal5",
		.gpio		= TL_WAX50RE_GPIO_LED_SIGNAL5,
		.active_low	= 1,
	},
};

static struct gpio_led tl_wa850re_leds_gpio[] __initdata = {
	{
		.name		= "tp-link:blue:lan",
		.gpio		= TL_WAX50RE_GPIO_LED_LAN,
		.active_low	= 1,
	}, {
		.name		= "tp-link:blue:wlan",
		.gpio		= TL_WAX50RE_GPIO_LED_WLAN,
		.active_low	= 1,
	}, {
		.name		= "tp-link:blue:re",
		.gpio		= TL_WAX50RE_GPIO_LED_RE,
		.active_low	= 1,
	}, {
		.name		= "tp-link:blue:signal1",
		.gpio		= TL_WAX50RE_GPIO_LED_SIGNAL1,
		.active_low	= 1,
	}, {
		.name		= "tp-link:blue:signal2",
		.gpio		= TL_WAX50RE_GPIO_LED_SIGNAL2,
		.active_low	= 1,
	}, {
		.name		= "tp-link:blue:signal3",
		.gpio		= TL_WAX50RE_GPIO_LED_SIGNAL3,
		.active_low	= 1,
	}, {
		.name		= "tp-link:blue:signal4",
		.gpio		= TL_WAX50RE_GPIO_LED_SIGNAL4,
		.active_low	= 1,
	}, {
		.name		= "tp-link:blue:signal5",
		.gpio		= TL_WAX50RE_GPIO_LED_SIGNAL5,
		.active_low	= 1,
	},
};

static struct gpio_led tl_wa850re_v2_leds_gpio[] __initdata = {
	{
		.name		= "tp-link:blue:lan",
		.gpio		= TL_WA850RE_V2_GPIO_LED_LAN,
		.active_low	= 1,
	}, {
		.name		= "tp-link:blue:re",
		.gpio		= TL_WA850RE_V2_GPIO_LED_RE,
		.active_low	= 1,
	}, {
		.name		= "tp-link:blue:signal1",
		.gpio		= TL_WA850RE_V2_GPIO_LED_SIGNAL1,
		.active_low	= 1,
	}, {
		.name		= "tp-link:blue:signal2",
		.gpio		= TL_WA850RE_V2_GPIO_LED_SIGNAL2,
		.active_low	= 1,
	}, {
		.name		= "tp-link:blue:signal3",
		.gpio		= TL_WA850RE_V2_GPIO_LED_SIGNAL3,
		.active_low	= 1,
	}, {
		.name		= "tp-link:blue:signal4",
		.gpio		= TL_WA850RE_V2_GPIO_LED_SIGNAL4,
		.active_low	= 1,
	}, {
		.name		= "tp-link:blue:signal5",
		.gpio		= TL_WA850RE_V2_GPIO_LED_SIGNAL5,
		.active_low	= 1,
	}, {
		.name		= "tp-link:blue:wlan",
		.gpio		= TL_WA850RE_V2_GPIO_LED_WLAN,
		.active_low	= 1,
	},
};

static struct gpio_led tl_wa855re_v1_leds_gpio[] __initdata = {
	 {
		.name		= "tp-link:green:power",
		.gpio		= TL_WA855REV1_GPIO_LED_GREEN,
		.active_low	= 0,
	}, {
		.name		= "tp-link:red:power",
		.gpio		= TL_WA855REV1_GPIO_LED_RED,
		.active_low	= 0,
	},
};

static struct gpio_led tl_wa860re_leds_gpio[] __initdata = {
	{
		.name		= "tp-link:green:lan",
		.gpio		= TL_WA860RE_GPIO_LED_LAN,
		.active_low	= 1,
	}, {
		.name		= "tp-link:green:power",
		.gpio		= TL_WA860RE_GPIO_LED_POWER_GREEN,
		.active_low	= 1,
	}, {
		.name		= "tp-link:orange:power",
		.gpio		= TL_WA860RE_GPIO_LED_POWER_ORANGE,
		.active_low	= 1,
	}, {
		.name		= "tp-link:green:wlan",
		.gpio		= TL_WA860RE_GPIO_LED_WLAN_GREEN,
		.active_low	= 1,
	}, {
		.name		= "tp-link:orange:wlan",
		.gpio		= TL_WA860RE_GPIO_LED_WLAN_ORANGE,
		.active_low	= 1,
	},
};


static struct gpio_keys_button tl_wax50re_gpio_keys[] __initdata = {
	{
		.desc		   = "Reset button",
		.type		   = EV_KEY,
		.code		   = KEY_RESTART,
		.debounce_interval = TL_WAX50RE_KEYS_DEBOUNCE_INTERVAL,
		.gpio		   = TL_WAX50RE_GPIO_BTN_RESET,
		.active_low	   = 1,
	}, {
		.desc		   = "WPS",
		.type		   = EV_KEY,
		.code		   = KEY_WPS_BUTTON,
		.debounce_interval = TL_WAX50RE_KEYS_DEBOUNCE_INTERVAL,
		.gpio		   = TL_WAX50RE_GPIO_BTN_WPS,
		.active_low	   = 1,
	},
};

static struct gpio_keys_button tl_wa860re_gpio_keys[] __initdata = {
	{
		.desc		   = "Reset button",
		.type		   = EV_KEY,
		.code		   = KEY_RESTART,
		.debounce_interval = TL_WAX50RE_KEYS_DEBOUNCE_INTERVAL,
		.gpio		   = TL_WA860RE_GPIO_BTN_RESET,
		.active_low	   = 1,
	}, {
		.desc		   = "WPS",
		.type		   = EV_KEY,
		.code		   = KEY_WPS_BUTTON,
		.debounce_interval = TL_WAX50RE_KEYS_DEBOUNCE_INTERVAL,
		.gpio		   = TL_WA860RE_GPIO_BTN_WPS,
		.active_low	   = 1,
	}, {
		.desc		   = "ONOFF",
		.type		   = EV_KEY,
		.code		   = BTN_1,
		.debounce_interval = TL_WAX50RE_KEYS_DEBOUNCE_INTERVAL,
		.gpio		   = TL_WA860RE_GPIO_BTN_ONOFF,
		.active_low	   = 1,
	},
};

static struct gpio_led tl_wa801nd_v2_leds_gpio[] __initdata = {
	{
		.name		= "tp-link:green:lan",
		.gpio		= TL_WA801ND_V2_GPIO_LED_LAN,
		.active_low	= 1,
	}, {
		.name		= "tp-link:green:wlan",
		.gpio		= TL_WAX50RE_GPIO_LED_WLAN,
		.active_low	= 1,
	}, {
		.name		= "tp-link:green:qss",
		.gpio		= TL_WAX50RE_GPIO_LED_RE,
		.active_low	= 1,
	}, {
		.name		= "tp-link:green:system",
		.gpio		= TL_WA801ND_V2_GPIO_LED_SYSTEM,
		.active_low	= 1,
	},
};

static void __init tl_ap123_setup(void)
{
	u8 *mac = (u8 *) KSEG1ADDR(0x1f01fc00);
	u8 *ee = (u8 *) KSEG1ADDR(0x1fff1000);

	ath79_register_m25p80(&tl_wax50re_flash_data);

	ath79_setup_ar934x_eth_cfg(AR934X_ETH_CFG_SW_PHY_SWAP);

	ath79_register_mdio(1, 0x0);

	ath79_init_mac(ath79_eth0_data.mac_addr, mac, 0);

	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ath79_eth0_data.phy_mask = BIT(0);
	ath79_eth0_data.mii_bus_dev = &ath79_mdio1_device.dev;
	ath79_register_eth(0);

	ath79_register_wmac(ee, mac);
}

static void __init tl_ap143_setup(void)
{
	u8 *mac = (u8 *) KSEG1ADDR(0x1f3c0008);
	u8 *ee = (u8 *) KSEG1ADDR(0x1fff1000);
	u8 tmpmac[ETH_ALEN];

	ath79_register_m25p80(NULL);

	ath79_setup_ar933x_phy4_switch(false, false);

	ath79_register_mdio(0, 0x0);

	ath79_switch_data.phy4_mii_en = 1;
	ath79_eth0_data.duplex = DUPLEX_FULL;
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ath79_eth0_data.phy_mask = BIT(4);
	ath79_eth0_data.speed = SPEED_100;
	ath79_init_mac(ath79_eth0_data.mac_addr, mac, -2);
	ath79_register_eth(0);

	ath79_init_mac(tmpmac, mac, 0);
	ath79_register_wmac(ee, mac);
}

static void  __init tl_wa750re_setup(void)
{
	tl_ap123_setup();
	ath79_register_leds_gpio(-1, ARRAY_SIZE(tl_wa750re_leds_gpio),
				 tl_wa750re_leds_gpio);

	ath79_register_gpio_keys_polled(-1, TL_WAX50RE_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(tl_wax50re_gpio_keys),
					tl_wax50re_gpio_keys);
}

MIPS_MACHINE(ATH79_MACH_TL_WA750RE, "TL-WA750RE", "TP-LINK TL-WA750RE",
	     tl_wa750re_setup);

static void __init tl_wa801nd_v2_setup(void)
{
	tl_ap123_setup();
	ath79_register_leds_gpio(-1, ARRAY_SIZE(tl_wa801nd_v2_leds_gpio),
			tl_wa801nd_v2_leds_gpio);

	ath79_register_gpio_keys_polled(-1, TL_WAX50RE_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(tl_wax50re_gpio_keys),
					tl_wax50re_gpio_keys);
}

MIPS_MACHINE(ATH79_MACH_TL_WA801ND_V2, "TL-WA801ND-v2", "TP-LINK TL-WA801ND v2",
	     tl_wa801nd_v2_setup);

static void  __init tl_wa850re_setup(void)
{
	tl_ap123_setup();
	ath79_register_leds_gpio(-1, ARRAY_SIZE(tl_wa850re_leds_gpio),
				 tl_wa850re_leds_gpio);

	ath79_register_gpio_keys_polled(-1, TL_WAX50RE_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(tl_wax50re_gpio_keys),
					tl_wax50re_gpio_keys);
}

MIPS_MACHINE(ATH79_MACH_TL_WA850RE, "TL-WA850RE", "TP-LINK TL-WA850RE",
	     tl_wa850re_setup);

static void  __init tl_wa850re_v2_setup(void)
{
	tl_ap143_setup();

	/* For GPIO 0~4 */
	ath79_gpio_function_setup(AR934X_GPIO_FUNC_JTAG_DISABLE,
				  AR934X_GPIO_FUNC_CLK_OBS4_EN);

	/* Allow to enable/disable all LEDs from userspace */
	gpio_request_one(TL_WA850RE_V2_GPIO_ENABLE_LEDS,
			 GPIOF_OUT_INIT_HIGH | GPIOF_EXPORT_DIR_FIXED,
			 "LEDs enable");

	ath79_register_leds_gpio(-1, ARRAY_SIZE(tl_wa850re_v2_leds_gpio),
				 tl_wa850re_v2_leds_gpio);

	ath79_register_gpio_keys_polled(-1, TL_WAX50RE_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(tl_wax50re_gpio_keys),
					tl_wax50re_gpio_keys);
}

MIPS_MACHINE(ATH79_MACH_TL_WA850RE_V2, "TL-WA850RE-V2",
	     "TP-LINK TL-WA850RE v2", tl_wa850re_v2_setup);

static void __init tl_wa855re_v1_setup(void)
{
	tl_ap143_setup();

	ath79_register_leds_gpio(-1, ARRAY_SIZE(tl_wa855re_v1_leds_gpio),
				 tl_wa855re_v1_leds_gpio);

	ath79_register_gpio_keys_polled(-1, TL_WAX50RE_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(tl_wax50re_gpio_keys),
					tl_wax50re_gpio_keys);
}

MIPS_MACHINE(ATH79_MACH_TL_WA855RE_V1, "TL-WA855RE-v1", "TP-LINK TL-WA855RE v1",
	     tl_wa855re_v1_setup);

static void  __init tl_wa860re_setup(void)
{
	tl_ap123_setup();
	ath79_register_leds_gpio(-1, ARRAY_SIZE(tl_wa860re_leds_gpio),
				 tl_wa860re_leds_gpio);

	ath79_register_gpio_keys_polled(-1, TL_WAX50RE_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(tl_wa860re_gpio_keys),
					tl_wa860re_gpio_keys);
}

MIPS_MACHINE(ATH79_MACH_TL_WA860RE, "TL-WA860RE", "TP-LINK TL-WA860RE",
	     tl_wa860re_setup);

static void __init tl_wa901nd_v3_setup(void)
{
	tl_ap123_setup();
	ath79_register_leds_gpio(-1, ARRAY_SIZE(tl_wa801nd_v2_leds_gpio),
			tl_wa801nd_v2_leds_gpio);

	ath79_register_gpio_keys_polled(-1, TL_WAX50RE_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(tl_wax50re_gpio_keys) - 1,
					tl_wax50re_gpio_keys);
}

MIPS_MACHINE(ATH79_MACH_TL_WA901ND_V3, "TL-WA901ND-v3", "TP-LINK TL-WA901ND v3",
	     tl_wa901nd_v3_setup);

/*
 *  TP-LINK TL-WR840N v2/v3 / TL-WR841N/ND v9/v11 / TL-WR842N/ND v3
 *
 *  Copyright (C) 2014 Matthias Schiffer <mschiffer@universe-factory.net>
 *  Copyright (C) 2016 Cezary Jackiewicz <cezary@eko.one.pl>
 *  Copyright (C) 2016 Stijn Segers <francesco.borromini@gmail.com>
 *  Copyright (C) 2017 Vaclav Svoboda <svoboda@neng.cz>
 *  Copyright (C) 2017 Andrey Polischuk <androld.b@gmail.com>
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

#define TL_WR840NV2_GPIO_LED_SYSTEM	15
#define TL_WR840NV2_GPIO_LED_WLAN	13
#define TL_WR840NV2_GPIO_LED_WPS	3
#define TL_WR840NV2_GPIO_LED_WAN	4
#define TL_WR840NV2_GPIO_LED_LAN	16

#define TL_WR840NV2_GPIO_BTN_RESET	12

#define TL_WR841NV9_GPIO_LED_WLAN	13
#define TL_WR841NV9_GPIO_LED_QSS	3
#define TL_WR841NV9_GPIO_LED_WAN	4
#define TL_WR841NV9_GPIO_LED_LAN1	16
#define TL_WR841NV9_GPIO_LED_LAN2	15
#define TL_WR841NV9_GPIO_LED_LAN3	14
#define TL_WR841NV9_GPIO_LED_LAN4	11

#define TL_WR841NV9_GPIO_BTN_RESET	12
#define TL_WR841NV9_GPIO_BTN_WIFI	17

#define TL_WR841NV11_GPIO_LED_SYSTEM	1
#define TL_WR841NV11_GPIO_LED_QSS	3
#define TL_WR841NV11_GPIO_LED_WAN	4
#define TL_WR841NV11_GPIO_LED_WAN_STATUS	2
#define TL_WR841NV11_GPIO_LED_WLAN	13
#define TL_WR841NV11_GPIO_LED_LAN1	16
#define TL_WR841NV11_GPIO_LED_LAN2	15
#define TL_WR841NV11_GPIO_LED_LAN3	14
#define TL_WR841NV11_GPIO_LED_LAN4	11

#define TL_WR841NV11_GPIO_BTN_RESET	12
#define TL_WR841NV11_GPIO_BTN_WIFI	17

#define TL_WR842NV3_GPIO_LED_SYSTEM	2
#define TL_WR842NV3_GPIO_LED_WLAN	3
#define TL_WR842NV3_GPIO_LED_WAN_RED	4
#define TL_WR842NV3_GPIO_LED_WAN_GREEN	11
#define TL_WR842NV3_GPIO_LED_LAN1	12
#define TL_WR842NV3_GPIO_LED_LAN2	13
#define TL_WR842NV3_GPIO_LED_LAN3	14
#define TL_WR842NV3_GPIO_LED_LAN4	15
#define TL_WR842NV3_GPIO_LED_3G		16
#define TL_WR842NV3_GPIO_LED_WPS	17

#define TL_WR842NV3_GPIO_BTN_RESET	1
#define TL_WR842NV3_GPIO_BTN_WIFI	0

#define TL_WR740NV6_GPIO_LED_SYSTEM	1
#define TL_WR740NV6_GPIO_LED_QSS	3
#define TL_WR740NV6_GPIO_LED_WAN_ORANGE	2
#define TL_WR740NV6_GPIO_LED_WAN_GREEN	4
#define TL_WR740NV6_GPIO_LED_LAN1	16
#define TL_WR740NV6_GPIO_LED_LAN2	15
#define TL_WR740NV6_GPIO_LED_LAN3	14
#define TL_WR740NV6_GPIO_LED_LAN4	11
#define TL_WR740NV6_GPIO_LED_WLAN	13

#define TL_WR740NV6_GPIO_BTN_RESET	12
#define TL_WR740NV6_GPIO_BTN_WIFI	17

#define TL_WR841NV9_KEYS_POLL_INTERVAL	20	/* msecs */
#define TL_WR841NV9_KEYS_DEBOUNCE_INTERVAL (3 * TL_WR841NV9_KEYS_POLL_INTERVAL)

static const char *tl_wr841n_v9_part_probes[] = {
	"tp-link",
	NULL,
};

static struct flash_platform_data tl_wr841n_v9_flash_data = {
	.part_probes	= tl_wr841n_v9_part_probes,
};

static struct gpio_led tl_wr840n_v2_leds_gpio[] __initdata = {
	{
		.name		= "tp-link:green:system",
		.gpio		= TL_WR840NV2_GPIO_LED_SYSTEM,
		.active_low	= 1,
	}, {
		.name		= "tp-link:green:lan",
		.gpio		= TL_WR840NV2_GPIO_LED_LAN,
		.active_low	= 1,
	}, {
		.name		= "tp-link:green:wps",
		.gpio		= TL_WR840NV2_GPIO_LED_WPS,
		.active_low	= 1,
	}, {
		.name		= "tp-link:green:wan",
		.gpio		= TL_WR840NV2_GPIO_LED_WAN,
		.active_low	= 1,
	}, {
		.name		= "tp-link:green:wlan",
		.gpio		= TL_WR840NV2_GPIO_LED_WLAN,
		.active_low	= 1,
	},
};

static struct gpio_keys_button tl_wr840n_v2_gpio_keys[] __initdata = {
	{
		.desc		= "Reset button",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = TL_WR841NV9_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= TL_WR840NV2_GPIO_BTN_RESET,
		.active_low	= 1,
	}
};

static struct gpio_led tl_wr841n_v9_leds_gpio[] __initdata = {
	{
		.name		= "tp-link:green:lan1",
		.gpio		= TL_WR841NV9_GPIO_LED_LAN1,
		.active_low	= 1,
	}, {
		.name		= "tp-link:green:lan2",
		.gpio		= TL_WR841NV9_GPIO_LED_LAN2,
		.active_low	= 1,
	}, {
		.name		= "tp-link:green:lan3",
		.gpio		= TL_WR841NV9_GPIO_LED_LAN3,
		.active_low	= 1,
	}, {
		.name		= "tp-link:green:lan4",
		.gpio		= TL_WR841NV9_GPIO_LED_LAN4,
		.active_low	= 1,
	}, {
		.name		= "tp-link:green:qss",
		.gpio		= TL_WR841NV9_GPIO_LED_QSS,
		.active_low	= 1,
	}, {
		.name		= "tp-link:green:wan",
		.gpio		= TL_WR841NV9_GPIO_LED_WAN,
		.active_low	= 1,
	}, {
		.name		= "tp-link:green:wlan",
		.gpio		= TL_WR841NV9_GPIO_LED_WLAN,
		.active_low	= 1,
	},
};

static struct gpio_keys_button tl_wr841n_v9_gpio_keys[] __initdata = {
	{
		.desc		= "Reset button",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = TL_WR841NV9_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= TL_WR841NV9_GPIO_BTN_RESET,
		.active_low	= 1,
	}, {
		.desc		= "WIFI button",
		.type		= EV_KEY,
		.code		= KEY_RFKILL,
		.debounce_interval = TL_WR841NV9_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= TL_WR841NV9_GPIO_BTN_WIFI,
		.active_low	= 1,
	}
};

static struct gpio_led tl_wr841n_v11_leds_gpio[] __initdata = {
	{
		.name		= "tp-link:green:lan1",
		.gpio		= TL_WR841NV9_GPIO_LED_LAN1,
		.active_low	= 1,
	}, {
		.name		= "tp-link:green:lan2",
		.gpio		= TL_WR841NV9_GPIO_LED_LAN2,
		.active_low	= 1,
	}, {
		.name		= "tp-link:green:lan3",
		.gpio		= TL_WR841NV9_GPIO_LED_LAN3,
		.active_low	= 1,
	}, {
		.name		= "tp-link:green:lan4",
		.gpio		= TL_WR841NV9_GPIO_LED_LAN4,
		.active_low	= 1,
	}, {
		.name		= "tp-link:green:qss",
		.gpio		= TL_WR841NV9_GPIO_LED_QSS,
		.active_low	= 1,
	}, {
		.name		= "tp-link:green:system",
		.gpio		= TL_WR841NV11_GPIO_LED_SYSTEM,
		.active_low	= 1,
	}, {
		.name		= "tp-link:green:wan",
		.gpio		= TL_WR841NV9_GPIO_LED_WAN,
		.active_low	= 1,
	}, {
		.name		= "tp-link:green:wan_status",
		.gpio		= TL_WR841NV11_GPIO_LED_WAN_STATUS,
		.active_low	= 1,
	}, {
		.name		= "tp-link:green:wlan",
		.gpio		= TL_WR841NV9_GPIO_LED_WLAN,
		.active_low	= 1,
	},
};

static struct gpio_led tl_wr842n_v3_leds_gpio[] __initdata = {
	{
		.name		= "tp-link:green:lan1",
		.gpio		= TL_WR842NV3_GPIO_LED_LAN1,
		.active_low	= 1,
	}, {
		.name		= "tp-link:green:lan2",
		.gpio		= TL_WR842NV3_GPIO_LED_LAN2,
		.active_low	= 1,
	}, {
		.name		= "tp-link:green:lan3",
		.gpio		= TL_WR842NV3_GPIO_LED_LAN3,
		.active_low	= 1,
	}, {
		.name		= "tp-link:green:lan4",
		.gpio		= TL_WR842NV3_GPIO_LED_LAN4,
		.active_low	= 1,
	}, {
		.name		= "tp-link:green:wan",
		.gpio		= TL_WR842NV3_GPIO_LED_WAN_GREEN,
		.active_low	= 1,
	}, {
		.name		= "tp-link:red:wan",
		.gpio		= TL_WR842NV3_GPIO_LED_WAN_RED,
		.active_low	= 1,
	}, {
		.name		= "tp-link:green:wlan",
		.gpio		= TL_WR842NV3_GPIO_LED_WLAN,
		.active_low	= 1,
	}, {
		.name		= "tp-link:green:system",
		.gpio		= TL_WR842NV3_GPIO_LED_SYSTEM,
		.active_low	= 1,
	}, {
		.name		= "tp-link:green:3g",
		.gpio		= TL_WR842NV3_GPIO_LED_3G,
		.active_low	= 1,
	}, {
		.name		= "tp-link:green:wps",
		.gpio		= TL_WR842NV3_GPIO_LED_WPS,
		.active_low	= 1,
	},
};

static struct gpio_keys_button tl_wr842n_v3_gpio_keys[] __initdata = {
	{
		.desc		= "Reset button",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = TL_WR841NV9_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= TL_WR842NV3_GPIO_BTN_RESET,
		.active_low	= 1,
	}, {
		.desc		= "WIFI button",
		.type		= EV_KEY,
		.code		= KEY_RFKILL,
		.debounce_interval = TL_WR841NV9_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= TL_WR842NV3_GPIO_BTN_WIFI,
		.active_low	= 1,
	}
};

static struct gpio_led tl_wr740n_v6_leds_gpio[] __initdata = {
	{
		.name		= "tp-link:green:lan1",
		.gpio		= TL_WR740NV6_GPIO_LED_LAN1,
		.active_low	= 1,
	}, {
		.name		= "tp-link:green:lan2",
		.gpio		= TL_WR740NV6_GPIO_LED_LAN2,
		.active_low	= 1,
	}, {
		.name		= "tp-link:green:lan3",
		.gpio		= TL_WR740NV6_GPIO_LED_LAN3,
		.active_low	= 1,
	}, {
		.name		= "tp-link:green:lan4",
		.gpio		= TL_WR740NV6_GPIO_LED_LAN4,
		.active_low	= 1,
	}, {
		.name		= "tp-link:green:system",
		.gpio		= TL_WR740NV6_GPIO_LED_SYSTEM,
		.active_low	= 1,
	}, {
		.name		= "tp-link:green:qss",
		.gpio		= TL_WR740NV6_GPIO_LED_QSS,
		.active_low	= 1,
	}, {
		.name		= "tp-link:green:wan",
		.gpio		= TL_WR740NV6_GPIO_LED_WAN_GREEN,
		.active_low	= 1,
	}, {
		.name		= "tp-link:orange:wan",
		.gpio		= TL_WR740NV6_GPIO_LED_WAN_ORANGE,
		.active_low	= 1,
	}, {
		.name		= "tp-link:green:wlan",
		.gpio		= TL_WR740NV6_GPIO_LED_WLAN,
		.active_low	= 1,
	},
};

static struct gpio_keys_button tl_wr740n_v6_gpio_keys[] __initdata = {
	{
		.desc		= "Reset button",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = TL_WR841NV9_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= TL_WR740NV6_GPIO_BTN_RESET,
		.active_low	= 1,
	}, {
		.desc		= "WIFI button",
		.type		= EV_KEY,
		.code		= KEY_RFKILL,
		.debounce_interval = TL_WR841NV9_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= TL_WR740NV6_GPIO_BTN_WIFI,
		.active_low	= 1,
	}
};

static void __init tl_ap143_setup(void)
{
	u8 *mac = (u8 *) KSEG1ADDR(0x1f01fc00);
	u8 *ee = (u8 *) KSEG1ADDR(0x1fff1000);
	u8 tmpmac[ETH_ALEN];

	ath79_register_m25p80(&tl_wr841n_v9_flash_data);

	ath79_setup_ar933x_phy4_switch(false, false);

	ath79_register_mdio(0, 0x0);

	/* LAN */
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_GMII;
	ath79_eth1_data.duplex = DUPLEX_FULL;
	ath79_switch_data.phy_poll_mask |= BIT(4);
	ath79_init_mac(ath79_eth1_data.mac_addr, mac, 0);
	ath79_register_eth(1);

	/* WAN */
	ath79_switch_data.phy4_mii_en = 1;
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ath79_eth0_data.duplex = DUPLEX_FULL;
	ath79_eth0_data.speed = SPEED_100;
	ath79_eth0_data.phy_mask = BIT(4);
	ath79_init_mac(ath79_eth0_data.mac_addr, mac, 1);
	ath79_register_eth(0);

	ath79_init_mac(tmpmac, mac, 0);
	ath79_register_wmac(ee, tmpmac);
}


static void __init tl_wr840n_v2_setup(void)
{
	tl_ap143_setup();

	ath79_register_leds_gpio(-1, ARRAY_SIZE(tl_wr840n_v2_leds_gpio),
				 tl_wr840n_v2_leds_gpio);

	ath79_register_gpio_keys_polled(1, TL_WR841NV9_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(tl_wr840n_v2_gpio_keys),
					tl_wr840n_v2_gpio_keys);
}

MIPS_MACHINE(ATH79_MACH_TL_WR840N_V2, "TL-WR840N-v2", "TP-LINK TL-WR840N v2",
	     tl_wr840n_v2_setup);

MIPS_MACHINE(ATH79_MACH_TL_WR840N_V3, "TL-WR840N-v3", "TP-LINK TL-WR840N v3",
	     tl_wr840n_v2_setup);

static void __init tl_wr841n_v9_setup(void)
{
	tl_ap143_setup();

	ath79_register_leds_gpio(-1, ARRAY_SIZE(tl_wr841n_v9_leds_gpio),
				 tl_wr841n_v9_leds_gpio);

	ath79_register_gpio_keys_polled(1, TL_WR841NV9_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(tl_wr841n_v9_gpio_keys),
					tl_wr841n_v9_gpio_keys);
}

MIPS_MACHINE(ATH79_MACH_TL_WR841N_V9, "TL-WR841N-v9", "TP-LINK TL-WR841N/ND v9",
	     tl_wr841n_v9_setup);

static void __init tl_wr841n_v11_setup(void)
{
	tl_ap143_setup();

	ath79_register_leds_gpio(-1, ARRAY_SIZE(tl_wr841n_v11_leds_gpio),
				 tl_wr841n_v11_leds_gpio);

	ath79_register_gpio_keys_polled(1, TL_WR841NV9_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(tl_wr841n_v9_gpio_keys),
					tl_wr841n_v9_gpio_keys);
}

MIPS_MACHINE(ATH79_MACH_TL_WR841N_V11, "TL-WR841N-v11", "TP-LINK TL-WR841N/ND v11",
	     tl_wr841n_v11_setup);

static void __init tl_wr842n_v3_setup(void)
{
	tl_ap143_setup();

	ath79_register_leds_gpio(-1, ARRAY_SIZE(tl_wr842n_v3_leds_gpio),
				 tl_wr842n_v3_leds_gpio);

	ath79_register_gpio_keys_polled(1, TL_WR841NV9_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(tl_wr842n_v3_gpio_keys),
					tl_wr842n_v3_gpio_keys);

	ath79_register_usb();
}

MIPS_MACHINE(ATH79_MACH_TL_WR842N_V3, "TL-WR842N-v3", "TP-LINK TL-WR842N/ND v3",
	     tl_wr842n_v3_setup);

static void __init tl_wr740n_v6_setup(void)
{
	tl_ap143_setup();

	ath79_register_leds_gpio(-1, ARRAY_SIZE(tl_wr740n_v6_leds_gpio),
				 tl_wr740n_v6_leds_gpio);

	ath79_register_gpio_keys_polled(1, TL_WR841NV9_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(tl_wr740n_v6_gpio_keys),
					tl_wr740n_v6_gpio_keys);
}

MIPS_MACHINE(ATH79_MACH_TL_WR740N_V6, "TL-WR740N-v6", "TP-LINK TL-WR740N/ND v6",
	     tl_wr740n_v6_setup);

/*
 *  Support for COMFAST boards:
 *  - CF-E316N v2 (AR9341)
 *  - CF-E320N v2 (QCA9531)
 *  - CF-E355AC v1 (QCA9531 + QCA9882)
 *  - CF-E355AC v2 (QCA9531 + QCA9886)
 *  - CF-E375AC (QCA9563 + QCA9886 + QCA8337)
 *  - CF-E380AC v1/v2 (QCA9558)
 *  - CF-E385AC (QCA9558 + QCA9984 + QCA8337)
 *  - CF-E520N/CF-E530N (QCA9531)
 *
 *  Copyright (C) 2016 Piotr Dymacz <pepe2k@gmail.com>
 *  Copyright (C) 2016 Gareth Parker <gareth41@orcon.net.nz>
 *  Copyright (C) 2015 Paul Fertser <fercerpav@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/gpio.h>
#include <linux/ar8216_platform.h>
#include <linux/platform_data/phy-at803x.h>
#include <linux/platform_device.h>
#include <linux/timer.h>

#include <asm/mach-ath79/ath79.h>
#include <asm/mach-ath79/ar71xx_regs.h>

#include "common.h"
#include "dev-ap9x-pci.h"
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-wmac.h"
#include "dev-usb.h"
#include "machtypes.h"
#include "pci.h"

#define CF_EXXXN_KEYS_POLL_INTERVAL	20
#define CF_EXXXN_KEYS_DEBOUNCE_INTERVAL	(3 * CF_EXXXN_KEYS_POLL_INTERVAL)

/* CF-E316N v2 */
#define CF_E316N_V2_GPIO_LED_DIAG_B	0
#define CF_E316N_V2_GPIO_LED_DIAG_R	2
#define CF_E316N_V2_GPIO_LED_DIAG_G	3
#define CF_E316N_V2_GPIO_LED_WLAN	12
#define CF_E316N_V2_GPIO_LED_WAN	17
#define CF_E316N_V2_GPIO_LED_LAN	19

#define CF_E316N_V2_GPIO_EXT_WDT	16

#define CF_E316N_V2_GPIO_EXTERNAL_PA0	13
#define CF_E316N_V2_GPIO_EXTERNAL_PA1	14

#define CF_E316N_V2_GPIO_BTN_RESET	20

static struct gpio_led cf_e316n_v2_leds_gpio[] __initdata = {
	{
		.name		= "cf-e316n-v2:blue:diag",
		.gpio		= CF_E316N_V2_GPIO_LED_DIAG_B,
		.active_low	= 0,
	}, {
		.name		= "cf-e316n-v2:red:diag",
		.gpio		= CF_E316N_V2_GPIO_LED_DIAG_R,
		.active_low	= 0,
	}, {
		.name		= "cf-e316n-v2:green:diag",
		.gpio		= CF_E316N_V2_GPIO_LED_DIAG_G,
		.active_low	= 0,
	}, {
		.name		= "cf-e316n-v2:blue:wlan",
		.gpio		= CF_E316N_V2_GPIO_LED_WLAN,
		.active_low	= 1,
	}, {
		.name		= "cf-e316n-v2:blue:wan",
		.gpio		= CF_E316N_V2_GPIO_LED_WAN,
		.active_low	= 1,
	}, {
		.name		= "cf-e316n-v2:blue:lan",
		.gpio		= CF_E316N_V2_GPIO_LED_LAN,
		.active_low	= 1,
	},
};

static struct gpio_keys_button cf_e316n_v2_gpio_keys[] __initdata = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = CF_EXXXN_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= CF_E316N_V2_GPIO_BTN_RESET,
		.active_low	= 1,
	},
};

/* CF-E320N v2 */
#define CF_E320N_V2_GPIO_LED_WLAN	0
#define CF_E320N_V2_GPIO_LED_WAN	2
#define CF_E320N_V2_GPIO_LED_LAN	3

#define CF_E320N_V2_GPIO_HEADER_J9_1	14
#define CF_E320N_V2_GPIO_HEADER_J9_2	12
#define CF_E320N_V2_GPIO_HEADER_J9_3	11
#define CF_E320N_V2_GPIO_HEADER_J9_4	16

#define CF_E320N_V2_GPIO_EXT_WDT	13

#define CF_E320N_V2_GPIO_BTN_RESET	17

static struct gpio_led cf_e320n_v2_leds_gpio[] __initdata = {
	{
		.name		= "cf-e320n-v2:green:lan",
		.gpio		= CF_E320N_V2_GPIO_LED_LAN,
		.active_low	= 0,
	}, {
		.name		= "cf-e320n-v2:red:wan",
		.gpio		= CF_E320N_V2_GPIO_LED_WAN,
		.active_low	= 0,
	}, {
		.name		= "cf-e320n-v2:blue:wlan",
		.gpio		= CF_E320N_V2_GPIO_LED_WLAN,
		.active_low	= 0,
	},
};

static struct gpio_keys_button cf_e320n_v2_gpio_keys[] __initdata = {
	{
		.desc		= "Reset button",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = CF_EXXXN_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= CF_E320N_V2_GPIO_BTN_RESET,
		.active_low	= 1,
	},
};

/* CF-E355AC v1/v2 */
#define CF_E355AC_GPIO_LED_LAN		3
#define CF_E355AC_GPIO_LED_WLAN2G	0
#define CF_E355AC_GPIO_LED_WLAN5G	2

#define CF_E355AC_GPIO_EXT_WDT		13

#define CF_E355AC_GPIO_BTN_RESET	17

static struct gpio_led cf_e355ac_v1_leds_gpio[] __initdata = {
	{
		.name		= "cf-e355ac-v1:green:lan",
		.gpio		= CF_E355AC_GPIO_LED_LAN,
		.active_low	= 0,
	}, {
		.name		= "cf-e355ac-v1:blue:wlan2g",
		.gpio		= CF_E355AC_GPIO_LED_WLAN2G,
		.active_low	= 0,
	}, {
		.name		= "cf-e355ac-v1:red:wlan5g",
		.gpio		= CF_E355AC_GPIO_LED_WLAN5G,
		.active_low	= 0,
	},
};

static struct gpio_led cf_e355ac_v2_leds_gpio[] __initdata = {
	{
		.name		= "cf-e355ac-v2:green:lan",
		.gpio		= CF_E355AC_GPIO_LED_LAN,
		.active_low	= 0,
	}, {
		.name		= "cf-e355ac-v2:blue:wlan2g",
		.gpio		= CF_E355AC_GPIO_LED_WLAN2G,
		.active_low	= 0,
	}, {
		.name		= "cf-e355ac-v2:red:wlan5g",
		.gpio		= CF_E355AC_GPIO_LED_WLAN5G,
		.active_low	= 0,
	},
};

static struct gpio_keys_button cf_e355ac_gpio_keys[] __initdata = {
	{
		.desc		= "Reset button",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = CF_EXXXN_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= CF_E355AC_GPIO_BTN_RESET,
		.active_low	= 1,
	},
};

/* CF-E375AC */
#define CF_E375AC_GPIO_LED_LAN		17
#define CF_E375AC_GPIO_LED_WLAN2G	16
#define CF_E375AC_GPIO_LED_WLAN5G	15

#define CF_E375AC_GPIO_EXT_WDT		6

#define CF_E375AC_GPIO_BTN_RESET	2

static struct gpio_led cf_e375ac_leds_gpio[] __initdata = {
	{
		.name		= "cf-e375ac:green:lan",
		.gpio		= CF_E375AC_GPIO_LED_LAN,
		.active_low	= 0,
	}, {
		.name		= "cf-e375ac:red:wlan5g",
		.gpio		= CF_E375AC_GPIO_LED_WLAN5G,
		.active_low	= 0,
	}, {
		.name		= "cf-e375ac:blue:wlan2g",
		.gpio		= CF_E375AC_GPIO_LED_WLAN2G,
		.active_low	= 0,
	},
};

static struct gpio_keys_button cf_e375ac_gpio_keys[] __initdata = {
	{
		.desc		= "Reset button",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = CF_EXXXN_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= CF_E375AC_GPIO_BTN_RESET,
		.active_low	= 1,
	},
};

static struct ar8327_pad_cfg cf_e375ac_ar8337_pad0_cfg = {
	.mode = AR8327_PAD_MAC_SGMII,
	.sgmii_delay_en = true,
};

static struct ar8327_platform_data cf_e375ac_ar8337_data = {
	.pad0_cfg = &cf_e375ac_ar8337_pad0_cfg,
	.port0_cfg = {
		.force_link = 1,
		.speed = AR8327_PORT_SPEED_1000,
		.duplex = 1,
		.txpause = 1,
		.rxpause = 1,
	},
};

static struct mdio_board_info cf_e375ac_mdio0_info[] = {
	{
		.bus_id = "ag71xx-mdio.0",
		.phy_addr = 0,
		.platform_data = &cf_e375ac_ar8337_data,
	},
};

/* CF-E380AC v1/v2, CF-E385AC */
#define CF_E38XAC_GPIO_LED_LAN		0
#define CF_E38XAC_GPIO_LED_WLAN2G	2
#define CF_E38XAC_GPIO_LED_WLAN5G	3

#define CF_E38XAC_GPIO_EXT_WDT		17

#define CF_E38XAC_GPIO_BTN_RESET	19

static struct gpio_led cf_e380ac_v1_leds_gpio[] __initdata = {
	{
		.name		= "cf-e380ac-v1:green:lan",
		.gpio		= CF_E38XAC_GPIO_LED_LAN,
		.active_low	= 0,
	}, {
		.name		= "cf-e380ac-v1:blue:wlan2g",
		.gpio		= CF_E38XAC_GPIO_LED_WLAN2G,
		.active_low	= 0,
	}, {
		.name		= "cf-e380ac-v1:red:wlan5g",
		.gpio		= CF_E38XAC_GPIO_LED_WLAN5G,
		.active_low	= 0,
	},
};

static struct gpio_led cf_e380ac_v2_leds_gpio[] __initdata = {
	{
		.name		= "cf-e380ac-v2:green:lan",
		.gpio		= CF_E38XAC_GPIO_LED_LAN,
		.active_low	= 0,
	}, {
		.name		= "cf-e380ac-v2:blue:wlan2g",
		.gpio		= CF_E38XAC_GPIO_LED_WLAN2G,
		.active_low	= 0,
	}, {
		.name		= "cf-e380ac-v2:red:wlan5g",
		.gpio		= CF_E38XAC_GPIO_LED_WLAN5G,
		.active_low	= 0,
	},
};

static struct gpio_led cf_e385ac_leds_gpio[] __initdata = {
	{
		.name		= "cf-e385ac:green:lan",
		.gpio		= CF_E38XAC_GPIO_LED_LAN,
		.active_low	= 0,
	}, {
		.name		= "cf-e385ac:blue:wlan2g",
		.gpio		= CF_E38XAC_GPIO_LED_WLAN2G,
		.active_low	= 0,
	}, {
		.name		= "cf-e385ac:red:wlan5g",
		.gpio		= CF_E38XAC_GPIO_LED_WLAN5G,
		.active_low	= 0,
	},
};

static struct gpio_keys_button cf_e38xac_gpio_keys[] __initdata = {
	{
		.desc		= "Reset button",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = CF_EXXXN_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= CF_E38XAC_GPIO_BTN_RESET,
		.active_low	= 1,
	},
};

static struct at803x_platform_data cf_e380ac_v1v2_at803x_data = {
	.disable_smarteee = 1,
};

static struct mdio_board_info cf_e380ac_v1v2_mdio0_info[] = {
	{
		.bus_id = "ag71xx-mdio.0",
		.phy_addr = 0,
		.platform_data = &cf_e380ac_v1v2_at803x_data,
	},
};

/* CF-E520N/CF-E530N */
#define CF_E5X0N_GPIO_LED_WAN		11
#define CF_E5X0N_GPIO_BTN_RESET		17

static struct gpio_led cf_e520n_leds_gpio[] __initdata = {
	{
		.name		= "cf-e520n:blue:wan",
		.gpio		= CF_E5X0N_GPIO_LED_WAN,
		.active_low	= 1,
	}
};

static struct gpio_led cf_e530n_leds_gpio[] __initdata = {
	{
		.name		= "cf-e530n:blue:wan",
		.gpio		= CF_E5X0N_GPIO_LED_WAN,
		.active_low	= 1,
	}
};

/*
 * Some COMFAST devices include external hardware watchdog chip,
 * Pericon Technology PT7A7514, connected to a selected GPIO
 * and WiSoC RESET_L input. Watchdog time-out is ~1.6 s.
 */
#define CF_EXXXN_EXT_WDT_TIMEOUT_MS	500

static struct timer_list gpio_wdt_timer;

static void gpio_wdt_toggle(unsigned long gpio)
{
	static int state;

	state = !state;
	gpio_set_value(gpio, state);

	mod_timer(&gpio_wdt_timer,
		  jiffies + msecs_to_jiffies(CF_EXXXN_EXT_WDT_TIMEOUT_MS));
}

static void __init cf_exxxn_common_setup(unsigned long art_ofs, int gpio_wdt)
{
	u8 *art = (u8 *) KSEG1ADDR(0x1f001000 + art_ofs);

	if (gpio_wdt > -1) {
		gpio_request_one(gpio_wdt, GPIOF_OUT_INIT_HIGH,
				 "PT7A7514 watchdog");

		setup_timer(&gpio_wdt_timer, gpio_wdt_toggle, gpio_wdt);
		gpio_wdt_toggle(gpio_wdt);
	}

	ath79_register_m25p80(NULL);

	ath79_register_wmac(art, NULL);

	ath79_register_usb();
}

static void __init cf_e316n_v2_setup(void)
{
	u8 *mac = (u8 *) KSEG1ADDR(0x1f010000);

	cf_exxxn_common_setup(0x10000, CF_E316N_V2_GPIO_EXT_WDT);

	ath79_setup_ar934x_eth_cfg(AR934X_ETH_CFG_SW_PHY_SWAP);

	ath79_register_mdio(1, 0x0);

	/* GMAC0 is connected to the PHY0 of the internal switch */
	ath79_switch_data.phy4_mii_en = 1;
	ath79_switch_data.phy_poll_mask = BIT(0);
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ath79_eth0_data.phy_mask = BIT(0);
	ath79_eth0_data.mii_bus_dev = &ath79_mdio1_device.dev;
	ath79_init_mac(ath79_eth0_data.mac_addr, mac, 0);
	ath79_register_eth(0);

	/* GMAC1 is connected to the internal switch */
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_GMII;
	ath79_init_mac(ath79_eth1_data.mac_addr, mac, 2);
	ath79_register_eth(1);

	/* Enable 2x Skyworks SE2576L WLAN power amplifiers */
	gpio_request_one(CF_E316N_V2_GPIO_EXTERNAL_PA0, GPIOF_OUT_INIT_HIGH,
			 "WLAN PA0");
	gpio_request_one(CF_E316N_V2_GPIO_EXTERNAL_PA1, GPIOF_OUT_INIT_HIGH,
			 "WLAN PA1");

	ath79_register_leds_gpio(-1, ARRAY_SIZE(cf_e316n_v2_leds_gpio),
				 cf_e316n_v2_leds_gpio);

	ath79_register_gpio_keys_polled(1, CF_EXXXN_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(cf_e316n_v2_gpio_keys),
					cf_e316n_v2_gpio_keys);
}

MIPS_MACHINE(ATH79_MACH_CF_E316N_V2, "CF-E316N-V2", "COMFAST CF-E316N v2",
	     cf_e316n_v2_setup);

static void __init cf_exxxn_qca953x_eth_setup(void)
{
	u8 *mac = (u8 *) KSEG1ADDR(0x1f010000);

	ath79_setup_ar933x_phy4_switch(false, false);

	ath79_register_mdio(0, 0x0);

	ath79_switch_data.phy4_mii_en = 1;
	ath79_switch_data.phy_poll_mask |= BIT(4);

	/* LAN */
	ath79_eth1_data.duplex = DUPLEX_FULL;
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_GMII;
	ath79_eth1_data.speed = SPEED_1000;
	ath79_init_mac(ath79_eth1_data.mac_addr, mac, 2);
	ath79_register_eth(1);

	/* WAN */
	ath79_eth0_data.duplex = DUPLEX_FULL;
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ath79_eth0_data.speed = SPEED_100;
	ath79_eth0_data.phy_mask = BIT(4);
	ath79_init_mac(ath79_eth0_data.mac_addr, mac, 0);
	ath79_register_eth(0);
}

static void __init cf_e320n_v2_setup(void)
{
	cf_exxxn_common_setup(0x10000, CF_E320N_V2_GPIO_EXT_WDT);

	cf_exxxn_qca953x_eth_setup();

	/* Disable JTAG (enables GPIO0-3) */
	ath79_gpio_function_enable(AR934X_GPIO_FUNC_JTAG_DISABLE);

	ath79_gpio_direction_select(CF_E320N_V2_GPIO_LED_LAN, true);
	ath79_gpio_direction_select(CF_E320N_V2_GPIO_LED_WAN, true);
	ath79_gpio_direction_select(CF_E320N_V2_GPIO_LED_WLAN, true);

	ath79_gpio_output_select(CF_E320N_V2_GPIO_LED_LAN, 0);
	ath79_gpio_output_select(CF_E320N_V2_GPIO_LED_WAN, 0);
	ath79_gpio_output_select(CF_E320N_V2_GPIO_LED_WLAN, 0);

	/* Enable GPIO function for GPIOs in J9 header */
	ath79_gpio_output_select(CF_E320N_V2_GPIO_HEADER_J9_1, 0);
	ath79_gpio_output_select(CF_E320N_V2_GPIO_HEADER_J9_2, 0);
	ath79_gpio_output_select(CF_E320N_V2_GPIO_HEADER_J9_3, 0);
	ath79_gpio_output_select(CF_E320N_V2_GPIO_HEADER_J9_4, 0);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(cf_e320n_v2_leds_gpio),
				 cf_e320n_v2_leds_gpio);

	ath79_register_gpio_keys_polled(-1, CF_EXXXN_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(cf_e320n_v2_gpio_keys),
					cf_e320n_v2_gpio_keys);
}

MIPS_MACHINE(ATH79_MACH_CF_E320N_V2, "CF-E320N-V2", "COMFAST CF-E320N v2",
	     cf_e320n_v2_setup);

static void __init cf_e355ac_setup(void)
{
	u8 *art = (u8 *) KSEG1ADDR(0x1f010000);

	/* Disable JTAG, enabling GPIOs 0-3 */
	ath79_gpio_function_setup(AR934X_GPIO_FUNC_JTAG_DISABLE, 0);

	cf_exxxn_common_setup(0x10000, CF_E355AC_GPIO_EXT_WDT);

	cf_exxxn_qca953x_eth_setup();

	ath79_gpio_output_select(CF_E355AC_GPIO_LED_LAN, 0);
	ath79_gpio_output_select(CF_E355AC_GPIO_LED_WLAN2G, 0);
	ath79_gpio_output_select(CF_E355AC_GPIO_LED_WLAN5G, 0);

	ap91_pci_init(art + 0x5000, NULL);

	ath79_register_gpio_keys_polled(1, CF_EXXXN_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(cf_e355ac_gpio_keys),
					cf_e355ac_gpio_keys);
}

static void __init cf_e355ac_v1_setup(void)
{
	cf_e355ac_setup();

	ath79_register_leds_gpio(-1, ARRAY_SIZE(cf_e355ac_v1_leds_gpio),
				 cf_e355ac_v1_leds_gpio);
}

static void __init cf_e355ac_v2_setup(void)
{
	cf_e355ac_setup();

	ath79_register_leds_gpio(-1, ARRAY_SIZE(cf_e355ac_v2_leds_gpio),
				 cf_e355ac_v2_leds_gpio);
}

MIPS_MACHINE(ATH79_MACH_CF_E355AC, "CF-E355AC-V1", "COMFAST CF-E355AC v1",
	     cf_e355ac_v1_setup);

MIPS_MACHINE(ATH79_MACH_CF_E355AC_V2, "CF-E355AC-V2", "COMFAST CF-E355AC v2",
	     cf_e355ac_v2_setup);

static void __init cf_e375ac_setup(void)
{
	u8 *mac = (u8 *) KSEG1ADDR(0x1f040000);

	/* Disable JTAG, enabling GPIOs 0-3 */
	ath79_gpio_function_setup(AR934X_GPIO_FUNC_JTAG_DISABLE, 0);

	cf_exxxn_common_setup(0x40000, CF_E375AC_GPIO_EXT_WDT);

	ath79_gpio_output_select(CF_E375AC_GPIO_LED_LAN, 0);
	ath79_gpio_output_select(CF_E375AC_GPIO_LED_WLAN2G, 0);
	ath79_gpio_output_select(CF_E375AC_GPIO_LED_WLAN5G, 0);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(cf_e375ac_leds_gpio),
				 cf_e375ac_leds_gpio);

	ath79_register_gpio_keys_polled(-1, CF_EXXXN_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(cf_e375ac_gpio_keys),
					cf_e375ac_gpio_keys);

	platform_device_register(&ath79_mdio0_device);

	mdiobus_register_board_info(cf_e375ac_mdio0_info,
				    ARRAY_SIZE(cf_e375ac_mdio0_info));

	/* GMAC0 is connected to an AR8337 switch */
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_SGMII;
	ath79_eth0_data.speed = SPEED_1000;
	ath79_eth0_data.duplex = DUPLEX_FULL;
	ath79_eth0_data.phy_mask = BIT(0);
	ath79_eth0_data.mii_bus_dev = &ath79_mdio0_device.dev;
	ath79_init_mac(ath79_eth0_data.mac_addr, mac, 0);
	ath79_register_eth(0);

	ath79_register_pci();
}

MIPS_MACHINE(ATH79_MACH_CF_E375AC, "CF-E375AC", "COMFAST CF-E375AC",
	     cf_e375ac_setup);

static void __init cf_e38xac_common_setup(unsigned long art_ofs)
{
	cf_exxxn_common_setup(art_ofs, CF_E38XAC_GPIO_EXT_WDT);

	ath79_register_pci();

	/* Disable JTAG (enables GPIO0-3) */
	ath79_gpio_function_enable(AR934X_GPIO_FUNC_JTAG_DISABLE);

	ath79_gpio_direction_select(CF_E38XAC_GPIO_LED_LAN, true);
	ath79_gpio_direction_select(CF_E38XAC_GPIO_LED_WLAN2G, true);
	ath79_gpio_direction_select(CF_E38XAC_GPIO_LED_WLAN5G, true);

	ath79_gpio_output_select(CF_E38XAC_GPIO_LED_LAN, 0);
	ath79_gpio_output_select(CF_E38XAC_GPIO_LED_WLAN2G, 0);
	ath79_gpio_output_select(CF_E38XAC_GPIO_LED_WLAN5G, 0);

	/* For J7-4 */
	ath79_gpio_function_disable(AR934X_GPIO_FUNC_CLK_OBS4_EN);

	ath79_register_gpio_keys_polled(-1, CF_EXXXN_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(cf_e38xac_gpio_keys),
					cf_e38xac_gpio_keys);
}

static void __init cf_e380ac_v1v2_common_setup(unsigned long art_ofs)
{
	u8 *mac = (u8 *) KSEG1ADDR(0x1f000000 + art_ofs);

	cf_e38xac_common_setup(art_ofs);

	ath79_setup_qca955x_eth_cfg(QCA955X_ETH_CFG_RGMII_EN);

	ath79_register_mdio(0, 0x0);
	mdiobus_register_board_info(cf_e380ac_v1v2_mdio0_info,
				    ARRAY_SIZE(cf_e380ac_v1v2_mdio0_info));

	/* LAN */
	ath79_eth0_data.mii_bus_dev = &ath79_mdio0_device.dev;
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth0_data.phy_mask = BIT(0);
	ath79_eth0_pll_data.pll_1000 = 0xbe000000;
	ath79_eth0_pll_data.pll_100 = 0xb0000101;
	ath79_eth0_pll_data.pll_10 = 0xb0001313;
	ath79_init_mac(ath79_eth0_data.mac_addr, mac, 0);
	ath79_register_eth(0);
}

static void __init cf_e380ac_v1_setup(void)
{
	cf_e380ac_v1v2_common_setup(0x20000);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(cf_e380ac_v1_leds_gpio),
				 cf_e380ac_v1_leds_gpio);
}

MIPS_MACHINE(ATH79_MACH_CF_E380AC_V1, "CF-E380AC-V1", "COMFAST CF-E380AC v1",
	     cf_e380ac_v1_setup);

static void __init cf_e380ac_v2_setup(void)
{
	cf_e380ac_v1v2_common_setup(0x40000);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(cf_e380ac_v2_leds_gpio),
				 cf_e380ac_v2_leds_gpio);
}

MIPS_MACHINE(ATH79_MACH_CF_E380AC_V2, "CF-E380AC-V2", "COMFAST CF-E380AC v2",
	     cf_e380ac_v2_setup);

/* QCA8337 GMAC0 is connected with QCA9558 over RGMII */
static struct ar8327_pad_cfg cf_e385ac_qca8337_pad0_cfg = {
	.mode = AR8327_PAD_MAC_RGMII,
	.txclk_delay_en = true,
	.rxclk_delay_en = true,
	.txclk_delay_sel = AR8327_CLK_DELAY_SEL1,
	.rxclk_delay_sel = AR8327_CLK_DELAY_SEL1,
};

/* QCA8337 GMAC6 is connected with QCA9558 over SGMII */
static struct ar8327_pad_cfg cf_e385ac_qca8337_pad6_cfg = {
	.mode = AR8327_PAD_MAC_SGMII,
	.sgmii_delay_en = true,
	.rxclk_delay_sel = AR8327_CLK_DELAY_SEL0,
};

static struct ar8327_platform_data cf_e385ac_qca8337_data = {
	.pad0_cfg = &cf_e385ac_qca8337_pad0_cfg,
	.pad6_cfg = &cf_e385ac_qca8337_pad6_cfg,
	.port0_cfg = {
		.force_link = 1,
		.speed = AR8327_PORT_SPEED_1000,
		.duplex = 1,
		.txpause = 1,
		.rxpause = 1,
	},
	.port6_cfg = {
		.force_link = 1,
		.speed = AR8327_PORT_SPEED_1000,
		.duplex = 1,
		.txpause = 1,
		.rxpause = 1,
	},
};

static struct mdio_board_info cf_e385ac_mdio0_info[] = {
	{
		.bus_id = "ag71xx-mdio.0",
		.phy_addr = 0,
		.platform_data = &cf_e385ac_qca8337_data,
	},
};

static void __init cf_e385ac_setup(void)
{
	u8 *mac = (u8 *) KSEG1ADDR(0x1f040000);

	cf_e38xac_common_setup(0x40000);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(cf_e385ac_leds_gpio),
				 cf_e385ac_leds_gpio);

	mdiobus_register_board_info(cf_e385ac_mdio0_info,
				    ARRAY_SIZE(cf_e385ac_mdio0_info));
	ath79_register_mdio(0, 0x0);

	ath79_setup_qca955x_eth_cfg(QCA955X_ETH_CFG_RGMII_EN);

	/* QCA9558 GMAC0 is connected to RMGII interface */
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth0_data.phy_mask = BIT(0);
	ath79_eth0_data.mii_bus_dev = &ath79_mdio0_device.dev;
	ath79_eth0_pll_data.pll_1000 = 0x96000000;

	ath79_init_mac(ath79_eth0_data.mac_addr, mac, 0);
	ath79_register_eth(0);

	/* QCA9558 GMAC1 is connected to SGMII interface */
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_SGMII;
	ath79_eth1_data.speed = SPEED_1000;
	ath79_eth1_data.duplex = DUPLEX_FULL;
	ath79_eth1_pll_data.pll_1000 = 0x03000101;

	ath79_init_mac(ath79_eth1_data.mac_addr, mac, 1);
	ath79_register_eth(1);
}

MIPS_MACHINE(ATH79_MACH_CF_E385AC, "CF-E385AC", "COMFAST CF-E385AC",
	     cf_e385ac_setup);

static void __init cf_e5x0n_gpio_setup(void)
{
	ath79_gpio_direction_select(CF_E5X0N_GPIO_LED_WAN, true);

	ath79_gpio_output_select(CF_E5X0N_GPIO_LED_WAN, 0);

	ath79_register_gpio_keys_polled(-1, CF_EXXXN_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(cf_e320n_v2_gpio_keys),
					cf_e320n_v2_gpio_keys);
}

static void __init cf_e520n_setup(void)
{
	cf_exxxn_common_setup(0x10000, -1);

	cf_exxxn_qca953x_eth_setup();

	cf_e5x0n_gpio_setup();

	ath79_register_leds_gpio(-1, ARRAY_SIZE(cf_e520n_leds_gpio),
				 cf_e520n_leds_gpio);
}

MIPS_MACHINE(ATH79_MACH_CF_E520N, "CF-E520N", "COMFAST CF-E520N",
	     cf_e520n_setup);

static void __init cf_e530n_setup(void)
{
	cf_exxxn_common_setup(0x10000, -1);

	cf_exxxn_qca953x_eth_setup();

	cf_e5x0n_gpio_setup();

	ath79_register_leds_gpio(-1, ARRAY_SIZE(cf_e530n_leds_gpio),
				 cf_e530n_leds_gpio);
}

MIPS_MACHINE(ATH79_MACH_CF_E530N, "CF-E530N", "COMFAST CF-E530N",
	     cf_e530n_setup);

/*
 *  TP-LINK EAP120 board support
 *
 * Copyright (C) 2016 Henryk Heisig <hyniu@o2.pl>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 */

#include <linux/gpio.h>
#include <linux/platform_device.h>
#include <linux/platform_data/mdio-gpio.h>
#include <asm/mach-ath79/ath79.h>
#include <asm/mach-ath79/ar71xx_regs.h>
#include <linux/platform_data/phy-at803x.h>

#include "common.h"
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-wmac.h"
#include "machtypes.h"


#define EAP120_GPIO_LED_RED	12
#define EAP120_GPIO_LED_YEL	13
#define EAP120_GPIO_LED_GRN	15
#define EAP120_GPIO_BTN_RESET	4

#define EAP120_KEYS_POLL_INTERVAL	20 /* msecs */
#define EAP120_KEYS_DEBOUNCE_INTERVAL	(3 * EAP120_KEYS_POLL_INTERVAL)

#define EAP120_GPIO_SMI_MDIO		16
#define EAP120_GPIO_SMI_MDC		17

#define EAP120_LAN_PHYADDR		4

static struct gpio_led eap120_leds_gpio[] __initdata = {
	{
		.name		= "eap120:red:system",
		.gpio		= EAP120_GPIO_LED_RED,
		.active_low	= 1,
	}, {
		.name		= "eap120:yellow:system",
		.gpio		= EAP120_GPIO_LED_YEL,
		.active_low	= 1,
	}, {
		.name		= "eap120:green:system",
		.gpio		= EAP120_GPIO_LED_GRN,
		.active_low	= 1,
	},
};

static struct gpio_keys_button eap120_gpio_keys[] __initdata = {
	{
		.desc		= "Reset button",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = EAP120_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= EAP120_GPIO_BTN_RESET,
		.active_low	= 1,
	}
};

static struct mdio_gpio_platform_data eap120_mdio = {
	.mdc		= EAP120_GPIO_SMI_MDC,
	.mdio		= EAP120_GPIO_SMI_MDIO,
	.phy_mask	= ~BIT(EAP120_LAN_PHYADDR),
};

static struct at803x_platform_data eap120_ar8035_data = {
	.disable_smarteee = 0,
	.enable_rgmii_rx_delay = 1,
	.enable_rgmii_tx_delay = 0,
	.fixup_rgmii_tx_delay = 1,
};

static struct platform_device eap120_phy_device = {
	.name	= "mdio-gpio",
	.id	= 0,
	.dev	= {
		.platform_data = &eap120_mdio, &eap120_ar8035_data
	},
};

static void __init eap_setup(u8 *mac)
{
	ath79_register_leds_gpio(-1, ARRAY_SIZE(eap120_leds_gpio),
				 eap120_leds_gpio);

	ath79_register_gpio_keys_polled(1, EAP120_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(eap120_gpio_keys),
					eap120_gpio_keys);

	ath79_register_m25p80(NULL);

	/* MDIO Interface */
	platform_device_register(&eap120_phy_device);
	ath79_setup_ar934x_eth_cfg(AR934X_ETH_CFG_RGMII_GMAC0);

	/* GMAC0 is connected to the RGMII interface to an Atheros AR8035-A */
	ath79_init_mac(ath79_eth0_data.mac_addr, mac, 0);
	ath79_eth0_data.mii_bus_dev = &eap120_phy_device.dev;
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth0_data.phy_mask = BIT(EAP120_LAN_PHYADDR);
	ath79_eth0_pll_data.pll_1000 = 0x0e000000;
	ath79_eth0_pll_data.pll_100 = 0x00000101;
	ath79_eth0_pll_data.pll_10 = 0x00001313;
	ath79_register_eth(0);
}

static void __init eap120_setup(void)
{
	u8 *mac = (u8 *) KSEG1ADDR(0x1f030008);
	u8 *ee = (u8 *) KSEG1ADDR(0x1fff1000);

	eap_setup(mac);

	ath79_register_wmac(ee, mac);
}

MIPS_MACHINE(ATH79_MACH_EAP120, "EAP120", "TP-LINK EAP120",
		eap120_setup);


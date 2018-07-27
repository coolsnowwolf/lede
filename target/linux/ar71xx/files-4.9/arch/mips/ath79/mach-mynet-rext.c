/*
 *  WD My Net WI-FI Range Extender (Codename:Starfish db12x) board support
 *
 *  Copyright (C) 2013 Christian Lamparter <chunkeey@googlemail.com>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/pci.h>
#include <linux/phy.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>
#include <linux/ath9k_platform.h>
#include <linux/ar8216_platform.h>
#include <linux/platform_data/phy-at803x.h>

#include <asm/mach-ath79/ar71xx_regs.h>

#include "common.h"
#include "dev-ap9x-pci.h"
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-spi.h"
#include "dev-usb.h"
#include "dev-wmac.h"
#include "machtypes.h"
#include "nvram.h"

#define MYNET_REXT_GPIO_LED_POWER	11
#define MYNET_REXT_GPIO_LED_ETHERNET	12
#define MYNET_REXT_GPIO_LED_WIFI	19

#define MYNET_REXT_GPIO_LED_RF_QTY1	20
#define MYNET_REXT_GPIO_LED_RF_QTY2	21
#define MYNET_REXT_GPIO_LED_RF_QTY3	22

#define MYNET_REXT_GPIO_BTN_RESET	13
#define MYNET_REXT_GPIO_BTN_WPS		15
#define MYNET_REXT_GPIO_SW_RF		14

#define MYNET_REXT_GPIO_PHY_SWRST	16	/* disables Ethernet PHY */
#define MYNET_REXT_GPIO_PHY_INT		17
#define MYNET_REXT_GPIO_18		18

#define MYNET_REXT_KEYS_POLL_INTERVAL	20	/* msecs */
#define MYNET_REXT_KEYS_DEBOUNCE_INTERVAL (3 * MYNET_REXT_KEYS_POLL_INTERVAL)

#define MYNET_REXT_WMAC_CALDATA_OFFSET	0x1000

#define MYNET_REXT_NVRAM_ADDR		0x1f7e0010
#define MYNET_REXT_NVRAM_SIZE		0xfff0

#define MYNET_REXT_ART_ADDR		0x1f7f0000

static const char *mynet_rext_part_probes[] = {
	"cybertan",
	NULL,
};

static struct flash_platform_data mynet_rext_flash_data = {
	.type		= "s25fl064k",
	.part_probes	= mynet_rext_part_probes,
};

static struct gpio_led mynet_rext_leds_gpio[] __initdata = {
	{
		.name		= "wd:blue:power",
		.gpio		= MYNET_REXT_GPIO_LED_POWER,
		.active_low	= 0,
	},
	{
		.name		= "wd:blue:wireless",
		.gpio		= MYNET_REXT_GPIO_LED_WIFI,
		.active_low	= 1,
	},
	{
		.name		= "wd:blue:ethernet",
		.gpio		= MYNET_REXT_GPIO_LED_ETHERNET,
		.active_low	= 1,
	},
	{
		.name		= "wd:blue:quality1",
		.gpio		= MYNET_REXT_GPIO_LED_RF_QTY1,
		.active_low	= 1,
	},
	{
		.name		= "wd:blue:quality2",
		.gpio		= MYNET_REXT_GPIO_LED_RF_QTY2,
		.active_low	= 1,
	},
	{
		.name		= "wd:blue:quality3",
		.gpio		= MYNET_REXT_GPIO_LED_RF_QTY3,
		.active_low	= 1,
	},
};

static struct gpio_keys_button mynet_rext_gpio_keys[] __initdata = {
	{
		.desc		= "Reset button",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = MYNET_REXT_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= MYNET_REXT_GPIO_BTN_RESET,
		.active_low	= 1,
	},
	{
		.desc		= "WPS button",
		.type		= EV_KEY,
		.code		= KEY_WPS_BUTTON,
		.debounce_interval = MYNET_REXT_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= MYNET_REXT_GPIO_BTN_WPS,
		.active_low	= 1,
	},
	{
		.desc           = "RF Band switch",
		.type           = EV_SW,
		.code           = BTN_1,
		.debounce_interval = MYNET_REXT_KEYS_DEBOUNCE_INTERVAL,
		.gpio           = MYNET_REXT_GPIO_SW_RF,
	},
};

static struct at803x_platform_data mynet_rext_at803x_data = {
	.disable_smarteee = 0,
	.enable_rgmii_rx_delay = 1,
	.enable_rgmii_tx_delay = 0,
	.fixup_rgmii_tx_delay = 1,
};

static struct mdio_board_info mynet_rext_mdio0_info[] = {
        {
                .bus_id = "ag71xx-mdio.0",
                .phy_addr = 4,
                .platform_data = &mynet_rext_at803x_data,
        },
};

static void mynet_rext_get_mac(const char *name, char *mac)
{
	u8 *nvram = (u8 *) KSEG1ADDR(MYNET_REXT_NVRAM_ADDR);
	int err;

	err = ath79_nvram_parse_mac_addr(nvram, MYNET_REXT_NVRAM_SIZE,
					 name, mac);
	if (err)
		pr_err("no MAC address found for %s\n", name);
}

static void __init mynet_rext_setup(void)
{
	u8 *art = (u8 *) KSEG1ADDR(MYNET_REXT_ART_ADDR);
	u8 tmpmac[ETH_ALEN];

	ath79_register_m25p80(&mynet_rext_flash_data);

	/* GPIO configuration from drivers/char/GPIO8.c */

	ath79_gpio_output_select(MYNET_REXT_GPIO_LED_POWER,
				 AR934X_GPIO_OUT_GPIO);
	ath79_gpio_output_select(MYNET_REXT_GPIO_LED_WIFI,
				 AR934X_GPIO_OUT_GPIO);
	ath79_gpio_output_select(MYNET_REXT_GPIO_LED_RF_QTY1,
				 AR934X_GPIO_OUT_GPIO);
	ath79_gpio_output_select(MYNET_REXT_GPIO_LED_RF_QTY2,
				 AR934X_GPIO_OUT_GPIO);
	ath79_gpio_output_select(MYNET_REXT_GPIO_LED_RF_QTY3,
				 AR934X_GPIO_OUT_GPIO);
	ath79_gpio_output_select(MYNET_REXT_GPIO_LED_ETHERNET,
				 AR934X_GPIO_OUT_GPIO);
	ath79_register_leds_gpio(-1, ARRAY_SIZE(mynet_rext_leds_gpio),
				 mynet_rext_leds_gpio);

	ath79_register_gpio_keys_polled(-1, MYNET_REXT_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(mynet_rext_gpio_keys),
					mynet_rext_gpio_keys);

	ath79_setup_ar934x_eth_cfg(AR934X_ETH_CFG_RGMII_GMAC0 |
				   AR934X_ETH_CFG_RXD_DELAY |
				   AR934X_ETH_CFG_RDV_DELAY);

	ath79_register_mdio(0, 0x0);

	mdiobus_register_board_info(mynet_rext_mdio0_info,
				    ARRAY_SIZE(mynet_rext_mdio0_info));

	/* LAN */
	mynet_rext_get_mac("et0macaddr=", ath79_eth0_data.mac_addr);

	/* GMAC0 is connected to an external PHY on Port 4 */
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth0_data.phy_mask = BIT(4);
	ath79_eth0_pll_data.pll_10   = 0x00001313; /* athrs_mac.c */
	ath79_eth0_pll_data.pll_1000 = 0x0e000000; /* athrs_mac.c */
	ath79_eth0_data.mii_bus_dev = &ath79_mdio0_device.dev;
	ath79_register_eth(0);

	/* WLAN */
	mynet_rext_get_mac("wl0_hwaddr=", tmpmac);
	ap91_pci_init(art + MYNET_REXT_WMAC_CALDATA_OFFSET, tmpmac);
}

MIPS_MACHINE(ATH79_MACH_MYNET_REXT, "MYNET-REXT",
	     "WD My Net Wi-Fi Range Extender", mynet_rext_setup);

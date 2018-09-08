/*
 *  ALFA Network AP120C board support
 *
 *  Copyright (C) 2012 Gabor Juhos <juhosg@openwrt.org>
 *  Copyright (C) 2016 Luka Perkov <luka@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/bitops.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>
#include <linux/spi/spi.h>
#include <linux/ar8216_platform.h>
#include <linux/ath9k_platform.h>

#include <asm/mach-ath79/ath79.h>
#include <asm/mach-ath79/ar71xx_regs.h>

#include "common.h"
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-spi.h"
#include "dev-ap9x-pci.h"
#include "dev-wmac.h"
#include "machtypes.h"

#define ALFA_AP120C_GPIO_LED	0

#define ALFA_AP120C_GPIO_BUTTON_WIFI	16

#define ALFA_AP120C_GPIO_WATCH_DOG	20

#define ALFA_AP120C_KEYS_POLL_INTERVAL		20	/* msecs */
#define ALFA_AP120C_KEYS_DEBOUNCE_INTERVAL	(3 * ALFA_AP120C_KEYS_POLL_INTERVAL)

#define ALFA_AP120C_MAC_OFFSET		0x1002
#define ALFA_AP120C_CAL0_OFFSET		0x1000

static struct gpio_keys_button alfa_ap120c_gpio_keys[] __initdata = {
	{
		.desc		= "Wireless button",
		.type		= EV_KEY,
		.code		= KEY_RFKILL,
		.debounce_interval = ALFA_AP120C_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= ALFA_AP120C_GPIO_BUTTON_WIFI,
		.active_low	= 1,
	}
};

static struct gpio_led alfa_ap120c_leds_gpio[] __initdata = {
	{
			.name		= "ap120c:red:wlan",
			.gpio		= ALFA_AP120C_GPIO_LED,
			.active_low = 0,
	}
};

static struct ar8327_pad_cfg ap120c_ar8327_pad0_cfg = {
	.mode = AR8327_PAD_MAC_RGMII,
	.txclk_delay_en = true,
	.rxclk_delay_en = true,
	.txclk_delay_sel = AR8327_CLK_DELAY_SEL1,
	.rxclk_delay_sel = AR8327_CLK_DELAY_SEL2,
};

static struct ar8327_platform_data ap120c_ar8327_data = {
	.pad0_cfg = &ap120c_ar8327_pad0_cfg,
	.port0_cfg = {
		.force_link = 1,
		.speed = AR8327_PORT_SPEED_1000,
		.duplex = 1,
		.txpause = 1,
		.rxpause = 1,
	},
};

static struct mdio_board_info ap120c_mdio0_info[] = {
	{
		.bus_id = "ag71xx-mdio.0",
		.mdio_addr = 0,
		.platform_data = &ap120c_ar8327_data,
	},
};

static struct flash_platform_data flash __initdata = { NULL, NULL, 0 };

#define ALFA_AP120C_LAN_PHYMASK		BIT(5)
#define ALFA_AP120C_MDIO_PHYMASK	ALFA_AP120C_LAN_PHYMASK

static void __init alfa_ap120c_init(void)
{
	u8 *art = (u8 *) KSEG1ADDR(0x1fff0000);
	u8 mac[ETH_ALEN];

	struct ath9k_platform_data *pdata;

	ath79_register_leds_gpio(-1, ARRAY_SIZE(alfa_ap120c_leds_gpio),
							 alfa_ap120c_leds_gpio);
	ath79_register_gpio_keys_polled(-1, ALFA_AP120C_KEYS_POLL_INTERVAL,
									ARRAY_SIZE(alfa_ap120c_gpio_keys),
									alfa_ap120c_gpio_keys);

	ath79_gpio_function_enable(AR71XX_GPIO_FUNC_SPI_CS1_EN |
							   AR71XX_GPIO_FUNC_SPI_CS2_EN);

	ath79_register_m25p80_multi(&flash);

	ath79_init_mac(mac, art + ALFA_AP120C_MAC_OFFSET, 1);
	ath79_register_wmac(art + ALFA_AP120C_CAL0_OFFSET, mac);

	ath79_init_mac(mac, art + ALFA_AP120C_MAC_OFFSET, 2);
	ap91_pci_init(NULL, mac);
	pdata = ap9x_pci_get_wmac_data(0);
	if (!pdata) {
		pr_err("ap120c: unable to get address of wlan data\n");
		return;
	}
	pdata->use_eeprom = true;

	ath79_setup_ar934x_eth_cfg(AR934X_ETH_CFG_RGMII_GMAC0 |
							   BIT(15) | BIT(17) | BIT(19) | BIT(21));

	ath79_register_mdio(0, 0x0);

	ath79_init_mac(ath79_eth0_data.mac_addr, art + ALFA_AP120C_MAC_OFFSET, 0);

	mdiobus_register_board_info(ap120c_mdio0_info, ARRAY_SIZE(ap120c_mdio0_info));

	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth0_data.phy_mask = ALFA_AP120C_LAN_PHYMASK;

	ath79_eth0_pll_data.pll_1000 = 0x42000000;
	ath79_eth0_pll_data.pll_10 = 0x00001313;

	ath79_eth0_data.mii_bus_dev = &ath79_mdio0_device.dev;

	ath79_register_eth(0);
}

MIPS_MACHINE(ATH79_MACH_ALFA_AP120C, "ALFA-AP120C", "ALFA Network AP120C",
			 alfa_ap120c_init);

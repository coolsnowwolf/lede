/*
 *  TP-LINK TL-WDR8500_V1 board support
 *
 *  Copyright (C) 2018 mht
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/gpio.h>
#include <linux/phy.h>
#include <linux/platform_device.h>
#include <linux/proc_fs.h>
#include <linux/spi/spi_gpio.h>
#include <linux/spi/74x164.h>
#include <linux/ath9k_platform.h>
#include <linux/ar8216_platform.h>

#include <asm/mach-ath79/ath79.h>
#include <asm/mach-ath79/ar71xx_regs.h>
#include <asm/mach-ath79/irq.h>

#include "common.h"
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-spi.h"
#include "dev-wmac.h"
#include "machtypes.h"
#include "pci.h"

#define TL_WDR8500_V1_GPIO_BTN_RESET	1
#define TL_WDR8500_V1_GPIO_BTN_TURBO	2

// tx = 18 / rx = 22
#define TL_WDR8500_V1_GPIO_LED_TURBO	5
#define TL_WDR8500_V1_GPIO_LED_SYS	21

#define TL_WDR8500_V1_KEYS_POLL_INTERVAL	20	/* msecs */
#define TL_WDR8500_V1_KEYS_DEBOUNCE_INTERVAL	(3 * TL_WDR8500_V1_KEYS_POLL_INTERVAL)

#define TL_WDR8500_V1_WMAC_CALDATA_OFFSET	0x1000

#define TL_WDR8500_V1_GPIO_MDC	3
#define TL_WDR8500_V1_GPIO_MDIO	4

// u-boot 128K
static const char *tl_wdr8500_v1_part_probes[] = {
	"tp-link",
	NULL,
};

static struct flash_platform_data tl_wdr8500_v1_flash_data = {
	.part_probes	= tl_wdr8500_v1_part_probes,
};

static struct gpio_led tl_wdr8500_v1_leds_gpio[] __initdata = {
	{
		.name		= "tp-link:blue:system",
		.gpio		= TL_WDR8500_V1_GPIO_LED_SYS,
		.active_low	= 1,
	}, {
		.name		= "tp-link:green:turbo",
		.gpio		= TL_WDR8500_V1_GPIO_LED_TURBO,
		.active_low	= 1,
	},
};

static struct gpio_keys_button tl_wdr8500_v1_gpio_keys[] __initdata = {
	{
		.desc           = "RFKill button",
		.type           = EV_KEY,
		.code           = KEY_RFKILL,
		.debounce_interval = TL_WDR8500_V1_KEYS_DEBOUNCE_INTERVAL,
		.gpio           = TL_WDR8500_V1_GPIO_BTN_TURBO,
		.active_low     = 1,
	},
	{
		.desc           = "Reset button",
		.type           = EV_KEY,
		.code           = KEY_RESTART,
		.debounce_interval = TL_WDR8500_V1_KEYS_DEBOUNCE_INTERVAL,
		.gpio           = TL_WDR8500_V1_GPIO_BTN_RESET,
		.active_low     = 1,
	},
};

static struct ar8327_pad_cfg TL_WDR8500_v1_ar8337_pad0_cfg = {
	.mode = AR8327_PAD_MAC_SGMII,
	.sgmii_delay_en = true,
};

static struct ar8327_platform_data TL_WDR8500_v1_ar8337_data = {
	.pad0_cfg = &TL_WDR8500_v1_ar8337_pad0_cfg,
	.port0_cfg = {
		.force_link = 1,
		.speed = AR8327_PORT_SPEED_1000,
		.duplex = 1,
		.txpause = 1,
		.rxpause = 1,
	},
};

static struct mdio_board_info TL_WDR8500_v1_mdio0_info[] = {
	{
		.bus_id = "ag71xx-mdio.0",
		.mdio_addr = 0,
		.platform_data = &TL_WDR8500_v1_ar8337_data,
	},
};

static void __init TL_WDR8500_v1_setup(void)
{
	u8 *mac = (u8 *) KSEG1ADDR(0x1f01fc00);
	u8 *art = (u8 *) KSEG1ADDR(0x1fff0000);

	ath79_register_m25p80(&tl_wdr8500_v1_flash_data);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(tl_wdr8500_v1_leds_gpio),
				 tl_wdr8500_v1_leds_gpio);

	ath79_register_gpio_keys_polled(-1, TL_WDR8500_V1_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(tl_wdr8500_v1_gpio_keys),
					tl_wdr8500_v1_gpio_keys);

	platform_device_register(&ath79_mdio0_device);

	ath79_register_mdio(1, 0x0);

	mdiobus_register_board_info(TL_WDR8500_v1_mdio0_info,
				    ARRAY_SIZE(TL_WDR8500_v1_mdio0_info));

	ath79_register_wmac(art + TL_WDR8500_V1_WMAC_CALDATA_OFFSET, mac);
	ath79_register_pci();

	/* GMAC0 is connected to an AR8337 switch */
	ath79_init_mac(ath79_eth0_data.mac_addr, mac, -1);
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_SGMII;
	ath79_eth0_data.speed = SPEED_1000;
	ath79_eth0_data.duplex = DUPLEX_FULL;
	ath79_eth0_data.phy_mask = BIT(0);
	ath79_eth0_data.mii_bus_dev = &ath79_mdio0_device.dev;
	ath79_register_eth(0);
}

MIPS_MACHINE(ATH79_MACH_TL_WDR8500_V1, "TL-WDR8500-v1", "TP-LINK TL-WDR8500 v1",
	     TL_WDR8500_v1_setup);

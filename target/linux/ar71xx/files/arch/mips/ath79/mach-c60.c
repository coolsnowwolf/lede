/*
 *  AirTight Networks C-60 board support
 *
 *  Copyright (C) 2016 Christian Lamparter <chunkeey@googlemail.com>
 *
 *  Based on AirTight Networks C-55 board support
 *
 *  Copyright (C) 2014-2015 Chris Blake <chrisrblake93@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/version.h>
#include <linux/pci.h>
#include <linux/phy.h>
#include <linux/mtd/mtd.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0)
#include <linux/mtd/nand.h>
#else
#include <linux/mtd/rawnand.h>
#endif
#include <linux/mtd/partitions.h>
#include <linux/mtd/physmap.h>
#include <linux/platform_device.h>
#include <linux/platform/ar934x_nfc.h>
#include <linux/ar8216_platform.h>
#include <linux/ath9k_platform.h>
#include <linux/version.h>

#include <asm/mach-ath79/ar71xx_regs.h>

#include "common.h"
#include "pci.h"
#include "dev-ap9x-pci.h"
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-spi.h"
#include "dev-wmac.h"
#include "dev-usb.h"
#include "dev-nfc.h"
#include "machtypes.h"

#define C60_GPIO_LED_PWR_AMBER		11
#define C60_GPIO_LED_WLAN2_GREEN	12
#define C60_GPIO_LED_WLAN2_AMBER	13
#define C60_GPIO_LED_PWR_GREEN		16

#define C60_GPIO_BTN_RESET		17

/* GPIOs of the AR9300 PCIe chip */
#define C60_GPIO_WMAC_LED_WLAN1_AMBER	0
#define C60_GPIO_WMAC_LED_WLAN1_GREEN	3

#define C60_KEYS_POLL_INTERVAL		20	/* msecs */
#define C60_KEYS_DEBOUNCE_INTERVAL (3 * C60_KEYS_POLL_INTERVAL)

#define C60_ART_ADDR			0x1f7f0000
#define C60_ART_SIZE			0xffff
#define C60_MAC_OFFSET			0
#define C60_WMAC_CALDATA_OFFSET		0x1000
#define C60_PCIE_CALDATA_OFFSET		0x5000

static struct gpio_led c60_leds_gpio[] __initdata = {
	{
		.name		= "c-60:amber:pwr",
		.gpio		= C60_GPIO_LED_PWR_AMBER,
		.active_low	= 1,
	},
	{
		.name		= "c-60:green:pwr",
		.gpio		= C60_GPIO_LED_PWR_GREEN,
		.active_low	= 1,
	},
	{
		.name		= "c-60:green:wlan2",
		.gpio		= C60_GPIO_LED_WLAN2_GREEN,
		.active_low	= 1,
	},
	{
		.name		= "c-60:amber:wlan2",
		.gpio		= C60_GPIO_LED_WLAN2_AMBER,
		.active_low	= 1,
	},
};

static struct gpio_keys_button c60_gpio_keys[] __initdata = {
	{
		.desc		= "Reset button",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = C60_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= C60_GPIO_BTN_RESET,
		.active_low	= 1,
	},
};

static struct ar8327_pad_cfg c60_ar8327_pad0_cfg = {
	.mode = AR8327_PAD_MAC_RGMII,
	.txclk_delay_en = true,
	.rxclk_delay_en = true,
	.txclk_delay_sel = AR8327_CLK_DELAY_SEL1,
	.rxclk_delay_sel = AR8327_CLK_DELAY_SEL2,
};

static struct ar8327_platform_data c60_ar8327_data = {
	.pad0_cfg = &c60_ar8327_pad0_cfg,
	.port0_cfg = {
		.force_link = 1,
		.speed = AR8327_PORT_SPEED_1000,
		.duplex = 1,
		.txpause = 1,
		.rxpause = 1,
	}
};

static struct mdio_board_info c60_mdio0_info[] = {
	{
		.bus_id = "ag71xx-mdio.0",
		.mdio_addr = 0,
		.platform_data = &c60_ar8327_data,
	},
};

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,6,0)
static struct nand_ecclayout c60_nand_ecclayout = {
	.eccbytes       = 7,
	.eccpos         = { 4, 8, 9, 10, 13, 14, 15 },
	.oobavail       = 9,
	.oobfree        = { { 0, 3 }, { 6, 2 }, { 11, 2 }, }
};

#else

static int c60_ooblayout_ecc(struct mtd_info *mtd, int section,
			     struct mtd_oob_region *oobregion)
{
	switch (section) {
	case 0:
		oobregion->offset = 4;
		oobregion->length = 1;
		return 0;
	case 1:
		oobregion->offset = 8;
		oobregion->length = 3;
		return 0;
	case 2:
		oobregion->offset = 13;
		oobregion->length = 3;
		return 0;
	default:
		return -ERANGE;
	}
}

static int c60_ooblayout_free(struct mtd_info *mtd, int section,
			      struct mtd_oob_region *oobregion)
{
	switch (section) {
	case 0:
		oobregion->offset = 0;
		oobregion->length = 3;
		return 0;
	case 1:
		oobregion->offset = 6;
		oobregion->length = 2;
		return 0;
	case 2:
		oobregion->offset = 11;
		oobregion->length = 2;
		return 0;
	default:
		return -ERANGE;
	}
}

static const struct mtd_ooblayout_ops c60_nand_ecclayout_ops = {
	.ecc = c60_ooblayout_ecc,
	.free = c60_ooblayout_free,
};
#endif /* < 4.6 */

static int c60_nand_scan_fixup(struct mtd_info *mtd)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,6,0)
	struct nand_chip *chip = mtd->priv;
#else
	struct nand_chip *chip = mtd_to_nand(mtd);
#endif

	chip->ecc.size = 512;
	chip->ecc.strength = 4;
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,6,0)
	chip->ecc.layout = &c60_nand_ecclayout;
#else
	mtd_set_ooblayout(mtd, &c60_nand_ecclayout_ops);
#endif
	return 0;
}

static struct gpio_led c60_wmac0_leds_gpio[] = {
	{
		.name		= "c-60:amber:wlan1",
		.gpio		= C60_GPIO_WMAC_LED_WLAN1_AMBER,
		.active_low	= 1,
	},
	{
		.name		= "c-60:green:wlan1",
		.gpio		= C60_GPIO_WMAC_LED_WLAN1_GREEN,
		.active_low	= 1,
	},
};

static void __init c60_setup(void)
{
	u8 tmpmac[6];
	u8 *art = (u8 *) KSEG1ADDR(C60_ART_ADDR);

	/* NAND */
	ath79_nfc_set_ecc_mode(AR934X_NFC_ECC_SOFT_BCH);
	ath79_nfc_set_scan_fixup(c60_nand_scan_fixup);
	ath79_register_nfc();

	/* SPI Storage*/
	ath79_register_m25p80(NULL);

	/* AR8327 Switch Ethernet */

	ath79_setup_ar934x_eth_cfg(AR934X_ETH_CFG_RGMII_GMAC0);

	mdiobus_register_board_info(c60_mdio0_info,
				    ARRAY_SIZE(c60_mdio0_info));

	ath79_register_mdio(0, 0x0);

	/* GMAC0 is connected to an AR8327N switch */
	ath79_init_mac(ath79_eth0_data.mac_addr, art + C60_MAC_OFFSET, 0);
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth0_data.phy_mask = BIT(0);
	ath79_eth0_data.mii_bus_dev = &ath79_mdio0_device.dev;
	ath79_eth0_pll_data.pll_1000 = 0x06000000;
	ath79_register_eth(0);

	/* LEDs & GPIO */
	ath79_register_leds_gpio(-1, ARRAY_SIZE(c60_leds_gpio),
				 c60_leds_gpio);
	ath79_register_gpio_keys_polled(-1, C60_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(c60_gpio_keys),
					c60_gpio_keys);
	ap9x_pci_setup_wmac_leds(0, c60_wmac0_leds_gpio,
				 ARRAY_SIZE(c60_wmac0_leds_gpio));
	/* USB */
	ath79_register_usb();

	/* WiFi */
	ath79_init_mac(tmpmac, art + C60_MAC_OFFSET, 1);
	ap91_pci_init(art + C60_PCIE_CALDATA_OFFSET, tmpmac);
	ath79_init_mac(tmpmac, art + C60_MAC_OFFSET, 2);
	ath79_register_wmac(art + C60_WMAC_CALDATA_OFFSET, tmpmac);
}
MIPS_MACHINE(ATH79_MACH_C60, "C-60", "AirTight Networks C-60",
	     c60_setup);

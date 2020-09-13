/*
 *  MikroTik RouterBOARD 95X support
 *
 *  Copyright (C) 2012 Stijn Tintel <stijn@linux-ipv6.be>
 *  Copyright (C) 2012 Gabor Juhos <juhosg@openwrt.org>
 *  Copyright (C) 2013 Kamil Trzcinski <ayufan@ayufan.eu>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#define pr_fmt(fmt) "rb95x: " fmt

#include <linux/version.h>
#include <linux/phy.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/ath9k_platform.h>
#include <linux/ar8216_platform.h>
#include <linux/mtd/mtd.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0)
#include <linux/mtd/nand.h>
#else
#include <linux/mtd/rawnand.h>
#endif
#include <linux/mtd/partitions.h>
#include <linux/spi/spi.h>
#include <linux/spi/flash.h>
#include <linux/routerboot.h>
#include <linux/gpio.h>
#include <linux/version.h>

#include <asm/mach-ath79/ath79.h>
#include <asm/mach-ath79/ar71xx_regs.h>

#include "common.h"
#include "dev-eth.h"
#include "dev-m25p80.h"
#include "dev-nfc.h"
#include "dev-usb.h"
#include "dev-wmac.h"
#include "machtypes.h"
#include "routerboot.h"
#include "dev-leds-gpio.h"

#define RB95X_GPIO_NAND_NCE	14

static struct mtd_partition rb95x_nand_partitions[] = {
	{
		.name	= "booter",
		.offset	= 0,
		.size	= (256 * 1024),
		.mask_flags = MTD_WRITEABLE,
	},
	{
		.name	= "kernel",
		.offset	= (256 * 1024),
		.size	= (4 * 1024 * 1024) - (256 * 1024),
	},
	{
		.name	= "ubi",
		.offset	= MTDPART_OFS_NXTBLK,
		.size	= MTDPART_SIZ_FULL,
	},
};

static struct gpio_led rb951ui_leds_gpio[] __initdata = {
	{
		.name		= "rb:green:wlan",
		.gpio		= 11,
		.active_low	= 1,
	}, {
		.name		= "rb:green:act",
		.gpio		= 3,
		.active_low	= 1,
	}, {
		.name		= "rb:green:port1",
		.gpio		= 13,
		.active_low	= 1,
	}, {
		.name		= "rb:green:port2",
		.gpio		= 12,
		.active_low	= 1,
	}, {
		.name		= "rb:green:port3",
		.gpio		= 4,
		.active_low	= 1,
	}, {
		.name		= "rb:green:port4",
		.gpio		= 21,
		.active_low	= 1,
	}, {
		.name		= "rb:green:port5",
		.gpio		= 16,
		.active_low	= 1,
	}
};

static struct ar8327_pad_cfg rb95x_ar8327_pad0_cfg = {
	.mode = AR8327_PAD_MAC_RGMII,
	.txclk_delay_en = true,
	.rxclk_delay_en = true,
	.txclk_delay_sel = AR8327_CLK_DELAY_SEL1,
	.rxclk_delay_sel = AR8327_CLK_DELAY_SEL2,
};

static struct ar8327_platform_data rb95x_ar8327_data = {
	.pad0_cfg = &rb95x_ar8327_pad0_cfg,
	.port0_cfg = {
		.force_link = 1,
		.speed = AR8327_PORT_SPEED_1000,
		.duplex = 1,
		.txpause = 1,
		.rxpause = 1,
	}
};

static struct mdio_board_info rb95x_mdio0_info[] = {
	{
		.bus_id = "ag71xx-mdio.0",
		.mdio_addr = 0,
		.platform_data = &rb95x_ar8327_data,
	},
};

void __init rb95x_wlan_init(void)
{
	char *art_buf;
	u8 wlan_mac[ETH_ALEN];

	art_buf = rb_get_wlan_data();
	if (art_buf == NULL)
		return;

	ath79_init_mac(wlan_mac, ath79_mac_base, 11);
	ath79_register_wmac(art_buf + 0x1000, wlan_mac);

	kfree(art_buf);
}

static void rb95x_nand_select_chip(int chip_no)
{
	switch (chip_no) {
	case 0:
		gpio_set_value(RB95X_GPIO_NAND_NCE, 0);
		break;
	default:
		gpio_set_value(RB95X_GPIO_NAND_NCE, 1);
		break;
	}
	ndelay(500);
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,6,0)
static struct nand_ecclayout rb95x_nand_ecclayout = {
	.eccbytes	= 6,
	.eccpos		= { 8, 9, 10, 13, 14, 15 },
	.oobavail	= 9,
	.oobfree	= { { 0, 4 }, { 6, 2 }, { 11, 2 }, { 4, 1 } }
};

#else

static int rb95x_ooblayout_ecc(struct mtd_info *mtd, int section,
			       struct mtd_oob_region *oobregion)
{
	switch (section) {
	case 0:
		oobregion->offset = 8;
		oobregion->length = 3;
		return 0;
	case 1:
		oobregion->offset = 13;
		oobregion->length = 3;
		return 0;
	default:
		return -ERANGE;
	}
}

static int rb95x_ooblayout_free(struct mtd_info *mtd, int section,
				struct mtd_oob_region *oobregion)
{
	switch (section) {
	case 0:
		oobregion->offset = 0;
		oobregion->length = 4;
		return 0;
	case 1:
		oobregion->offset = 4;
		oobregion->length = 1;
		return 0;
	case 2:
		oobregion->offset = 6;
		oobregion->length = 2;
		return 0;
	case 3:
		oobregion->offset = 11;
		oobregion->length = 2;
		return 0;
	default:
		return -ERANGE;
	}
}

static const struct mtd_ooblayout_ops rb95x_nand_ecclayout_ops = {
	.ecc = rb95x_ooblayout_ecc,
	.free = rb95x_ooblayout_free,
};
#endif /* < 4.6 */

static int rb95x_nand_scan_fixup(struct mtd_info *mtd)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,6,0)
	struct nand_chip *chip = mtd->priv;
#else
	struct nand_chip *chip = mtd_to_nand(mtd);
#endif /* < 4.6.0 */

	if (mtd->writesize == 512) {
		/*
		 * Use the OLD Yaffs-1 OOB layout, otherwise RouterBoot
		 * will not be able to find the kernel that we load.
		 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,6,0)
		chip->ecc.layout = &rb95x_nand_ecclayout;
#else
		mtd_set_ooblayout(mtd, &rb95x_nand_ecclayout_ops);
#endif
	}

	chip->options = NAND_NO_SUBPAGE_WRITE;

	return 0;
}

void __init rb95x_nand_init(void)
{
	gpio_request_one(RB95X_GPIO_NAND_NCE, GPIOF_OUT_INIT_HIGH, "NAND nCE");

	ath79_nfc_set_scan_fixup(rb95x_nand_scan_fixup);
	ath79_nfc_set_parts(rb95x_nand_partitions,
			    ARRAY_SIZE(rb95x_nand_partitions));
	ath79_nfc_set_select_chip(rb95x_nand_select_chip);
	ath79_nfc_set_swap_dma(true);
	ath79_register_nfc();
}

static int __init rb95x_setup(void)
{
	const struct rb_info *info;

	info = rb_init_info((void *)(KSEG1ADDR(AR71XX_SPI_BASE)), 0x10000);
	if (!info)
		return -EINVAL;

	rb95x_nand_init();

	return 0;
}

static void __init rb951g_setup(void)
{
	if (rb95x_setup())
		return;

	ath79_setup_ar934x_eth_cfg(AR934X_ETH_CFG_RGMII_GMAC0 |
				   AR934X_ETH_CFG_RXD_DELAY |
				   AR934X_ETH_CFG_SW_ONLY_MODE);

	ath79_register_mdio(0, 0x0);

	mdiobus_register_board_info(rb95x_mdio0_info,
				    ARRAY_SIZE(rb95x_mdio0_info));

	ath79_init_mac(ath79_eth0_data.mac_addr, ath79_mac_base, 0);
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth0_data.phy_mask = BIT(0);
	ath79_eth0_pll_data.pll_1000 = 0x6f000000;

	ath79_register_eth(0);

	rb95x_wlan_init();
	ath79_register_usb();
}

MIPS_MACHINE(ATH79_MACH_RB_951G, "951G", "MikroTik RouterBOARD 951G-2HnD",
	     rb951g_setup);

static void __init rb951ui_setup(void)
{
	if (rb95x_setup())
		return;

	ath79_setup_ar934x_eth_cfg(AR934X_ETH_CFG_SW_ONLY_MODE);

	ath79_register_mdio(1, 0x0);

	ath79_init_mac(ath79_eth0_data.mac_addr, ath79_mac_base, 0);
	ath79_init_mac(ath79_eth1_data.mac_addr, ath79_mac_base, 1);

	ath79_switch_data.phy4_mii_en = 1;
	ath79_switch_data.phy_poll_mask = BIT(4);
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ath79_eth0_data.phy_mask = BIT(4);
	ath79_eth0_data.mii_bus_dev = &ath79_mdio1_device.dev;
	ath79_register_eth(0);

	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_GMII;
	ath79_register_eth(1);

	gpio_request_one(20, GPIOF_OUT_INIT_HIGH | GPIOF_EXPORT_DIR_FIXED,
			 "USB power");

	gpio_request_one(2, GPIOF_OUT_INIT_HIGH | GPIOF_EXPORT_DIR_FIXED,
			 "POE power");

	rb95x_wlan_init();
	ath79_register_usb();

	ath79_register_leds_gpio(-1, ARRAY_SIZE(rb951ui_leds_gpio),
				 rb951ui_leds_gpio);
}

MIPS_MACHINE(ATH79_MACH_RB_951U, "951HnD", "MikroTik RouterBOARD 951Ui-2HnD",
	     rb951ui_setup);

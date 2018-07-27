/*
 *  MikroTik RouterBOARD 2011 support
 *
 *  Copyright (C) 2012 Stijn Tintel <stijn@linux-ipv6.be>
 *  Copyright (C) 2012 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#define pr_fmt(fmt) "rb2011: " fmt

#include <linux/phy.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/ath9k_platform.h>
#include <linux/ar8216_platform.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/partitions.h>
#include <linux/spi/spi.h>
#include <linux/spi/flash.h>
#include <linux/routerboot.h>
#include <linux/gpio.h>
#include <linux/version.h>

#include <asm/prom.h>
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

#define RB2011_GPIO_NAND_NCE	14
#define RB2011_GPIO_SFP_LOS	21

#define RB_ROUTERBOOT_OFFSET	0x0000
#define RB_ROUTERBOOT_MIN_SIZE	0xb000
#define RB_HARD_CFG_SIZE	0x1000
#define RB_BIOS_OFFSET		0xd000
#define RB_BIOS_SIZE		0x1000
#define RB_SOFT_CFG_OFFSET	0xf000
#define RB_SOFT_CFG_SIZE	0x1000

#define RB_ART_SIZE		0x10000

#define RB2011_FLAG_SFP		BIT(0)
#define RB2011_FLAG_USB		BIT(1)
#define RB2011_FLAG_WLAN	BIT(2)

static struct mtd_partition rb2011_spi_partitions[] = {
	{
		.name		= "routerboot",
		.offset		= RB_ROUTERBOOT_OFFSET,
		.mask_flags	= MTD_WRITEABLE,
	}, {
		.name		= "hard_config",
		.size		= RB_HARD_CFG_SIZE,
		.mask_flags	= MTD_WRITEABLE,
	}, {
		.name		= "bios",
		.offset		= RB_BIOS_OFFSET,
		.size		= RB_BIOS_SIZE,
		.mask_flags	= MTD_WRITEABLE,
	}, {
		.name		= "soft_config",
		.size		= RB_SOFT_CFG_SIZE,
	}
};

static void __init rb2011_init_partitions(const struct rb_info *info)
{
	rb2011_spi_partitions[0].size = info->hard_cfg_offs;
	rb2011_spi_partitions[1].offset = info->hard_cfg_offs;
	rb2011_spi_partitions[3].offset = info->soft_cfg_offs;
}

static struct mtd_partition rb2011_nand_partitions[] = {
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

static struct flash_platform_data rb2011_spi_flash_data = {
	.parts		= rb2011_spi_partitions,
	.nr_parts	= ARRAY_SIZE(rb2011_spi_partitions),
};

static struct ar8327_pad_cfg rb2011_ar8327_pad0_cfg = {
	.mode = AR8327_PAD_MAC_RGMII,
	.txclk_delay_en = true,
	.rxclk_delay_en = true,
	.txclk_delay_sel = AR8327_CLK_DELAY_SEL3,
	.rxclk_delay_sel = AR8327_CLK_DELAY_SEL0,
};

static struct ar8327_pad_cfg rb2011_ar8327_pad6_cfg;
static struct ar8327_sgmii_cfg rb2011_ar8327_sgmii_cfg;

static struct ar8327_led_cfg rb2011_ar8327_led_cfg = {
	.led_ctrl0 = 0xc731c731,
	.led_ctrl1 = 0x00000000,
	.led_ctrl2 = 0x00000000,
	.led_ctrl3 = 0x0030c300,
	.open_drain = false,
};

static const struct ar8327_led_info rb2011_ar8327_leds[] = {
	AR8327_LED_INFO(PHY0_0, HW, "rb:green:eth1"),
	AR8327_LED_INFO(PHY1_0, HW, "rb:green:eth2"),
	AR8327_LED_INFO(PHY2_0, HW, "rb:green:eth3"),
	AR8327_LED_INFO(PHY3_0, HW, "rb:green:eth4"),
	AR8327_LED_INFO(PHY4_0, HW, "rb:green:eth5"),
	AR8327_LED_INFO(PHY0_1, SW, "rb:green:eth6"),
	AR8327_LED_INFO(PHY1_1, SW, "rb:green:eth7"),
	AR8327_LED_INFO(PHY2_1, SW, "rb:green:eth8"),
	AR8327_LED_INFO(PHY3_1, SW, "rb:green:eth9"),
	AR8327_LED_INFO(PHY4_1, SW, "rb:green:eth10"),
	AR8327_LED_INFO(PHY4_2, SW, "rb:green:usr"),
};

static struct ar8327_platform_data rb2011_ar8327_data = {
	.pad0_cfg = &rb2011_ar8327_pad0_cfg,
	.port0_cfg = {
		.force_link = 1,
		.speed = AR8327_PORT_SPEED_1000,
		.duplex = 1,
		.txpause = 1,
		.rxpause = 1,
	},
	.led_cfg = &rb2011_ar8327_led_cfg,
	.num_leds = ARRAY_SIZE(rb2011_ar8327_leds),
	.leds = rb2011_ar8327_leds,
};

static struct mdio_board_info rb2011_mdio0_info[] = {
	{
		.bus_id = "ag71xx-mdio.0",
		.phy_addr = 0,
		.platform_data = &rb2011_ar8327_data,
	},
};

static void __init rb2011_wlan_init(void)
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

static void rb2011_nand_select_chip(int chip_no)
{
	switch (chip_no) {
	case 0:
		gpio_set_value(RB2011_GPIO_NAND_NCE, 0);
		break;
	default:
		gpio_set_value(RB2011_GPIO_NAND_NCE, 1);
		break;
	}
	ndelay(500);
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,6,0)
static struct nand_ecclayout rb2011_nand_ecclayout = {
	.eccbytes	= 6,
	.eccpos		= { 8, 9, 10, 13, 14, 15 },
	.oobavail	= 9,
	.oobfree	= { { 0, 4 }, { 6, 2 }, { 11, 2 }, { 4, 1 } }
};

#else

static int rb2011_ooblayout_ecc(struct mtd_info *mtd, int section,
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

static int rb2011_ooblayout_free(struct mtd_info *mtd, int section,
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

static const struct mtd_ooblayout_ops rb2011_nand_ecclayout_ops = {
	.ecc = rb2011_ooblayout_ecc,
	.free = rb2011_ooblayout_free,
};
#endif /* < 4.6 */

static int rb2011_nand_scan_fixup(struct mtd_info *mtd)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,6,0)
	struct nand_chip *chip = mtd->priv;
#endif

	if (mtd->writesize == 512) {
		/*
		 * Use the OLD Yaffs-1 OOB layout, otherwise RouterBoot
		 * will not be able to find the kernel that we load.
		 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,6,0)
		chip->ecc.layout = &rb2011_nand_ecclayout;
#else
		mtd_set_ooblayout(mtd, &rb2011_nand_ecclayout_ops);
#endif
	}

	return 0;
}

static void __init rb2011_nand_init(void)
{
	gpio_request_one(RB2011_GPIO_NAND_NCE, GPIOF_OUT_INIT_HIGH, "NAND nCE");

	ath79_nfc_set_scan_fixup(rb2011_nand_scan_fixup);
	ath79_nfc_set_parts(rb2011_nand_partitions,
			    ARRAY_SIZE(rb2011_nand_partitions));
	ath79_nfc_set_select_chip(rb2011_nand_select_chip);
	ath79_nfc_set_swap_dma(true);
	ath79_register_nfc();
}

static int rb2011_get_port_link(unsigned port)
{
	if (port != 6)
		return -EINVAL;

	/* The Loss of signal line is active low */
	return !gpio_get_value(RB2011_GPIO_SFP_LOS);
}

static void __init rb2011_sfp_init(void)
{
	gpio_request_one(RB2011_GPIO_SFP_LOS, GPIOF_IN, "SFP LOS");

	rb2011_ar8327_pad6_cfg.mode = AR8327_PAD_MAC_SGMII;

	rb2011_ar8327_data.pad6_cfg = &rb2011_ar8327_pad6_cfg;

	rb2011_ar8327_sgmii_cfg.sgmii_ctrl = 0xc70167d0;
	rb2011_ar8327_sgmii_cfg.serdes_aen = true;

	rb2011_ar8327_data.sgmii_cfg = &rb2011_ar8327_sgmii_cfg;

	rb2011_ar8327_data.port6_cfg.force_link = 1;
	rb2011_ar8327_data.port6_cfg.speed = AR8327_PORT_SPEED_1000;
	rb2011_ar8327_data.port6_cfg.duplex = 1;

	rb2011_ar8327_data.get_port_link = rb2011_get_port_link;
}

static int __init rb2011_setup(u32 flags)
{
	const struct rb_info *info;
	char buf[64];

	info = rb_init_info((void *) KSEG1ADDR(0x1f000000), 0x10000);
	if (!info)
		return -ENODEV;

	scnprintf(buf, sizeof(buf), "Mikrotik RouterBOARD %s",
		  (info->board_name) ? info->board_name : "");
	mips_set_machine_name(buf);

	rb2011_init_partitions(info);

	ath79_register_m25p80(&rb2011_spi_flash_data);
	rb2011_nand_init();

	ath79_setup_ar934x_eth_cfg(AR934X_ETH_CFG_RGMII_GMAC0 |
				   AR934X_ETH_CFG_RXD_DELAY |
				   AR934X_ETH_CFG_SW_ONLY_MODE);

	ath79_register_mdio(1, 0x0);
	ath79_register_mdio(0, 0x0);

	mdiobus_register_board_info(rb2011_mdio0_info,
				    ARRAY_SIZE(rb2011_mdio0_info));

	/* GMAC0 is connected to an ar8327 switch */
	ath79_init_mac(ath79_eth0_data.mac_addr, ath79_mac_base, 0);
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth0_data.phy_mask = BIT(0);
	ath79_eth0_data.mii_bus_dev = &ath79_mdio0_device.dev;
	ath79_eth0_pll_data.pll_1000 = 0x6f000000;

	ath79_register_eth(0);

	/* GMAC1 is connected to the internal switch */
	ath79_init_mac(ath79_eth1_data.mac_addr, ath79_mac_base, 5);
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_GMII;
	ath79_eth1_data.speed = SPEED_1000;
	ath79_eth1_data.duplex = DUPLEX_FULL;

	ath79_register_eth(1);

	if (flags & RB2011_FLAG_SFP)
		rb2011_sfp_init();

	if (flags & RB2011_FLAG_WLAN)
		rb2011_wlan_init();

	if (flags & RB2011_FLAG_USB)
		ath79_register_usb();

	return 0;
}

static void __init rb2011l_setup(void)
{
	rb2011_setup(0);
}

MIPS_MACHINE_NONAME(ATH79_MACH_RB_2011L, "2011L", rb2011l_setup);

static void __init rb2011us_setup(void)
{
	rb2011_setup(RB2011_FLAG_SFP | RB2011_FLAG_USB);
}

MIPS_MACHINE_NONAME(ATH79_MACH_RB_2011US, "2011US", rb2011us_setup);

static void __init rb2011r5_setup(void)
{
	rb2011_setup(RB2011_FLAG_SFP | RB2011_FLAG_USB | RB2011_FLAG_WLAN);
}

MIPS_MACHINE_NONAME(ATH79_MACH_RB_2011R5, "2011r5", rb2011r5_setup);

static void __init rb2011g_setup(void)
{
	rb2011_setup(RB2011_FLAG_SFP |
		     RB2011_FLAG_USB |
		     RB2011_FLAG_WLAN);
}

MIPS_MACHINE_NONAME(ATH79_MACH_RB_2011G, "2011G", rb2011g_setup);

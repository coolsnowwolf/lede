/*
 *  MikroTik RouterBOARD 92X support
 *
 *  Copyright (C) 2015 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/version.h>
#include <linux/phy.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/ath9k_platform.h>
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
#include <linux/platform_data/phy-at803x.h>
#include <linux/version.h>

#include <asm/prom.h>
#include <asm/mach-ath79/ath79.h>
#include <asm/mach-ath79/ar71xx_regs.h>

#include "common.h"
#include "dev-gpio-buttons.h"
#include "dev-eth.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-nfc.h"
#include "dev-usb.h"
#include "dev-spi.h"
#include "machtypes.h"
#include "pci.h"
#include "routerboot.h"

#define RB922_GPIO_LED_USR	12
#define RB922_GPIO_USB_POWER	13
#define RB922_GPIO_FAN_CTRL	14
#define RB922_GPIO_BTN_RESET	20
#define RB922_GPIO_NAND_NCE	23

#define RB92X_FLAG_USB		BIT(0)
#define RB92X_FLAG_USB_POWER	BIT(1)
#define RB92X_FLAG_PCIE		BIT(2)

#define RB922_PHY_ADDR		4

#define RB922_KEYS_POLL_INTERVAL	20	/* msecs */
#define RB922_KEYS_DEBOUNCE_INTERVAL	(3 * RB922_KEYS_POLL_INTERVAL)

#define RB_ROUTERBOOT_OFFSET	0x0000
#define RB_ROUTERBOOT_MIN_SIZE	0xb000
#define RB_HARD_CFG_SIZE	0x1000
#define RB_BIOS_OFFSET		0xd000
#define RB_BIOS_SIZE		0x1000
#define RB_SOFT_CFG_OFFSET	0xf000
#define RB_SOFT_CFG_SIZE	0x1000

struct rb_board_info {
        const char *name;
        u32 flags;
};

static struct mtd_partition rb922gs_spi_partitions[] = {
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

static void __init rb922gs_init_partitions(const struct rb_info *info)
{
	rb922gs_spi_partitions[0].size = info->hard_cfg_offs;
	rb922gs_spi_partitions[1].offset = info->hard_cfg_offs;
	rb922gs_spi_partitions[3].offset = info->soft_cfg_offs;
}

static struct mtd_partition rb922gs_nand_partitions[] = {
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

static struct flash_platform_data rb922gs_spi_flash_data = {
	.parts		= rb922gs_spi_partitions,
	.nr_parts	= ARRAY_SIZE(rb922gs_spi_partitions),
};

static struct gpio_led rb922gs_leds[] __initdata = {
	{
		.name		= "rb:green:user",
		.gpio		= RB922_GPIO_LED_USR,
		.active_low	= 1,
	},
};

static struct gpio_keys_button rb922gs_gpio_keys[] __initdata = {
	{
		.desc		= "Reset button",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = RB922_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= RB922_GPIO_BTN_RESET,
		.active_low	= 1,
	},
};

static struct at803x_platform_data rb922gs_at803x_data = {
	.disable_smarteee = 1,
};

static struct mdio_board_info rb922gs_mdio0_info[] = {
	{
		.bus_id = "ag71xx-mdio.0",
		.mdio_addr = RB922_PHY_ADDR,
		.platform_data = &rb922gs_at803x_data,
	},
};



static void rb922gs_nand_select_chip(int chip_no)
{
	switch (chip_no) {
	case 0:
		gpio_set_value(RB922_GPIO_NAND_NCE, 0);
		break;
	default:
		gpio_set_value(RB922_GPIO_NAND_NCE, 1);
		break;
	}
	ndelay(500);
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,6,0)
static struct nand_ecclayout rb922gs_nand_ecclayout = {
	.eccbytes	= 6,
	.eccpos		= { 8, 9, 10, 13, 14, 15 },
	.oobavail	= 9,
	.oobfree	= { { 0, 4 }, { 6, 2 }, { 11, 2 }, { 4, 1 } }
};

#else

static int rb922gs_ooblayout_ecc(struct mtd_info *mtd, int section,
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

static int rb922gs_ooblayout_free(struct mtd_info *mtd, int section,
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

static const struct mtd_ooblayout_ops rb922gs_nand_ecclayout_ops = {
	.ecc = rb922gs_ooblayout_ecc,
	.free = rb922gs_ooblayout_free,
};
#endif /* < 4.6 */

static int rb922gs_nand_scan_fixup(struct mtd_info *mtd)
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
		chip->ecc.layout = &rb922gs_nand_ecclayout;
#else
		mtd_set_ooblayout(mtd, &rb922gs_nand_ecclayout_ops);
#endif
	}

	chip->options = NAND_NO_SUBPAGE_WRITE;

	return 0;
}

static void __init rb922gs_nand_init(void)
{
	gpio_request_one(RB922_GPIO_NAND_NCE, GPIOF_OUT_INIT_HIGH, "NAND nCE");

	ath79_nfc_set_scan_fixup(rb922gs_nand_scan_fixup);
	ath79_nfc_set_parts(rb922gs_nand_partitions,
			    ARRAY_SIZE(rb922gs_nand_partitions));
	ath79_nfc_set_select_chip(rb922gs_nand_select_chip);
	ath79_nfc_set_swap_dma(true);
	ath79_register_nfc();
}

#define RB_BOARD_INFO(_name, _flags)    \
        {                               \
                .name = (_name),        \
                .flags = (_flags),      \
        }

static const struct rb_board_info rb92x_boards[] __initconst = {
        RB_BOARD_INFO("921GS-5HPacD r2", RB92X_FLAG_PCIE),
        RB_BOARD_INFO("922UAGS-5HPacD", RB92X_FLAG_USB | RB92X_FLAG_USB_POWER | RB92X_FLAG_PCIE),
};

static u32 rb92x_get_flags(const struct rb_info *info)
{
        int i;

        for (i = 0; i < ARRAY_SIZE(rb92x_boards); i++) {
                const struct rb_board_info *bi;

                bi = &rb92x_boards[i];
                if (strcmp(info->board_name, bi->name) == 0)
                        return bi->flags;
        }

        return 0;
}

static void __init rb922gs_setup(void)
{
	const struct rb_info *info;
	char buf[64];
	u32 flags;

	info = rb_init_info((void *) KSEG1ADDR(0x1f000000), 0x10000);
	if (!info)
		return;

	scnprintf(buf, sizeof(buf), "MikroTik RouterBOARD %s",
		  (info->board_name) ? info->board_name : "");
	mips_set_machine_name(buf);

	rb922gs_init_partitions(info);
	ath79_register_m25p80(&rb922gs_spi_flash_data);

	rb922gs_nand_init();

	ath79_setup_qca955x_eth_cfg(QCA955X_ETH_CFG_RGMII_EN);

	ath79_register_mdio(0, 0x0);

	mdiobus_register_board_info(rb922gs_mdio0_info,
				    ARRAY_SIZE(rb922gs_mdio0_info));

	ath79_init_mac(ath79_eth0_data.mac_addr, ath79_mac_base, 0);
	ath79_eth0_data.mii_bus_dev = &ath79_mdio0_device.dev;
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth0_data.phy_mask = BIT(RB922_PHY_ADDR);
	if (strcmp(info->board_name, "921GS-5HPacD r2") == 0 ||
	    strcmp(info->board_name, "922UAGS-5HPacD") == 0)
	{
		ath79_eth0_pll_data.pll_10 = 0xa0001313;
		ath79_eth0_pll_data.pll_100 = 0xa0000101;
		ath79_eth0_pll_data.pll_1000 = 0x8f000000;
	}
	else {
		ath79_eth0_pll_data.pll_10 = 0x81001313;
		ath79_eth0_pll_data.pll_100 = 0x81000101;
		ath79_eth0_pll_data.pll_1000 = 0x8f000000;
	}

	ath79_register_eth(0);

	flags = rb92x_get_flags(info);

        if (flags & RB92X_FLAG_USB)
                ath79_register_usb();

        if (flags & RB92X_FLAG_USB_POWER)
                gpio_request_one(RB922_GPIO_USB_POWER, GPIOF_OUT_INIT_LOW |
			GPIOF_EXPORT_DIR_FIXED, "USB power");

        if (flags & RB92X_FLAG_PCIE)
                ath79_register_pci();

	ath79_register_leds_gpio(-1, ARRAY_SIZE(rb922gs_leds), rb922gs_leds);
	ath79_register_gpio_keys_polled(-1, RB922_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(rb922gs_gpio_keys),
					rb922gs_gpio_keys);

	/* NOTE:
	 * This only supports the RB911G-5HPacD board for now. For other boards
	 * more devices must be registered based on the hardware options which
	 * can be found in the hardware configuration of RouterBOOT.
	 */
}

MIPS_MACHINE_NONAME(ATH79_MACH_RB_922GS, "922gs", rb922gs_setup);

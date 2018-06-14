/*
 *  MikroTik RouterBOARD SXT Lite support
 *
 *  Copyright (C) 2012 Stijn Tintel <stijn@linux-ipv6.be>
 *  Copyright (C) 2012 Gabor Juhos <juhosg@openwrt.org>
 *  Copyright (C) 2013 Vyacheslav Adamanov <adamanov@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#define pr_fmt(fmt) "sxtlite: " fmt

#include <linux/phy.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/ath9k_platform.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/partitions.h>
#include <linux/spi/spi.h>
#include <linux/spi/flash.h>
#include <linux/rle.h>
#include <linux/routerboot.h>
#include <linux/gpio.h>
#include <linux/version.h>

#include <asm/mach-ath79/ath79.h>
#include <asm/mach-ath79/ar71xx_regs.h>
#include "common.h"
#include "dev-ap9x-pci.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-eth.h"
#include "dev-m25p80.h"
#include "dev-nfc.h"
#include "dev-wmac.h"
#include "dev-usb.h"
#include "machtypes.h"
#include "routerboot.h"
#include <linux/ar8216_platform.h>

#define SXTLITE_GPIO_NAND_NCE  14
#define SXTLITE_GPIO_LED_USER 3
#define SXTLITE_GPIO_LED_1 13
#define SXTLITE_GPIO_LED_2 12
#define SXTLITE_GPIO_LED_3 4
#define SXTLITE_GPIO_LED_4 21
#define SXTLITE_GPIO_LED_5 18
#define SXTLITE_GPIO_LED_POWER 11

#define SXTLITE_GPIO_BUZZER 19

#define SXTLITE_GPIO_BTN_RESET 15

#define SXTLITE_KEYS_POLL_INTERVAL 20
#define SXTLITE_KEYS_DEBOUNCE_INTERVAL (3 * SXTLITE_KEYS_POLL_INTERVAL)

static struct mtd_partition rbsxtlite_nand_partitions[] = {
	{
		.name   = "booter",
		.offset = 0,
		.size   = (256 * 1024),
		.mask_flags = MTD_WRITEABLE,
	},
	{
		.name   = "kernel",
		.offset = (256 * 1024),
		.size   = (4 * 1024 * 1024) - (256 * 1024),
	},
	{
		.name   = "ubi",
		.offset = MTDPART_OFS_NXTBLK,
		.size   = MTDPART_SIZ_FULL,
	},
};

static struct gpio_led rbsxtlite_leds_gpio[] __initdata = {
	{
		.name		=	"rb:green:user",
		.gpio		=	SXTLITE_GPIO_LED_USER,
		.active_low	=	1,
	},
	{
		.name		=	"rb:green:led1",
		.gpio		=	SXTLITE_GPIO_LED_1,
		.active_low	=	1,
	},
	{
		.name		=	"rb:green:led2",
		.gpio		=	SXTLITE_GPIO_LED_2,
		.active_low	=	1,
	},
	{
		.name		=	"rb:green:led3",
		.gpio		=	SXTLITE_GPIO_LED_3,
		.active_low	=	1,
	},
	{
		.name		=	"rb:green:led4",
		.gpio		=	SXTLITE_GPIO_LED_4,
		.active_low	=	1,
	},
	{
		.name		=	"rb:green:led5",
		.gpio		=	SXTLITE_GPIO_LED_5,
		.active_low	=	1,
	},
	{
		.name		=	"rb:green:power",
		.gpio		=	SXTLITE_GPIO_LED_POWER,
		.default_state	=	LEDS_GPIO_DEFSTATE_KEEP,
	},
};

static struct gpio_keys_button rbsxtlite_gpio_keys[] __initdata = {
	{
		.desc		=	"Reset button",
		.type		=	EV_KEY,
		.code		=	KEY_RESTART,
		.debounce_interval	=	SXTLITE_KEYS_DEBOUNCE_INTERVAL,
		.gpio		=	SXTLITE_GPIO_BTN_RESET,
		.active_low	=	0,
	},
};

static int __init rbsxtlite_rbinfo_init(void)
{
	const struct rb_info *info;

	info = rb_init_info((void *)(KSEG1ADDR(AR71XX_SPI_BASE)), 0x10000);
	if (!info)
		return -EINVAL;
	return 0;

}

void __init rbsxtlite_wlan_init(void)
{
	char *art_buf;
	u8 wlan_mac[ETH_ALEN];

	art_buf = rb_get_wlan_data();
	if (art_buf == NULL)
		return;

	ath79_init_mac(wlan_mac, ath79_mac_base, 1);
	ath79_register_wmac(art_buf + 0x1000, wlan_mac);

	kfree(art_buf);
}

static void rbsxtlite_nand_select_chip(int chip_no)
{
	switch (chip_no) {
	case 0:
		gpio_set_value(SXTLITE_GPIO_NAND_NCE, 0);
		break;
	default:
		gpio_set_value(SXTLITE_GPIO_NAND_NCE, 1);
		break;
	}
	ndelay(500);
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,6,0)
static struct nand_ecclayout rbsxtlite_nand_ecclayout = {
	.eccbytes	= 6,
	.eccpos	 = { 8, 9, 10, 13, 14, 15 },
	.oobavail	= 9,
	.oobfree	= { { 0, 4 }, { 6, 2 }, { 11, 2 }, { 4, 1 } }
};

#else

static int rbsxtlite_ooblayout_ecc(struct mtd_info *mtd, int section,
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

static int rbsxtlite_ooblayout_free(struct mtd_info *mtd, int section,
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

static const struct mtd_ooblayout_ops rbsxtlite_nand_ecclayout_ops = {
	.ecc = rbsxtlite_ooblayout_ecc,
	.free = rbsxtlite_ooblayout_free,
};
#endif /* < 4.6 */

static int rbsxtlite_nand_scan_fixup(struct mtd_info *mtd)
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
		chip->ecc.layout = &rbsxtlite_nand_ecclayout;
#else
		mtd_set_ooblayout(mtd, &rbsxtlite_nand_ecclayout_ops);
#endif
	}

	return 0;
}

void __init rbsxtlite_gpio_init(void)
{
	gpio_request_one(SXTLITE_GPIO_NAND_NCE, GPIOF_OUT_INIT_HIGH, "NAND nCE");
}

void __init rbsxtlite_nand_init(void)
{
	ath79_nfc_set_scan_fixup(rbsxtlite_nand_scan_fixup);
	ath79_nfc_set_parts(rbsxtlite_nand_partitions,
			   ARRAY_SIZE(rbsxtlite_nand_partitions));
	ath79_nfc_set_select_chip(rbsxtlite_nand_select_chip);
	ath79_nfc_set_swap_dma(true);
	ath79_register_nfc();
}


static void __init rbsxtlite_setup(void)
{
	if(rbsxtlite_rbinfo_init())
		return;
	rbsxtlite_nand_init();
	rbsxtlite_wlan_init();

	ath79_register_leds_gpio(-1, ARRAY_SIZE(rbsxtlite_leds_gpio),
				 rbsxtlite_leds_gpio);
	ath79_register_gpio_keys_polled(-1, SXTLITE_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(rbsxtlite_gpio_keys),
					rbsxtlite_gpio_keys);

	ath79_setup_ar934x_eth_cfg(AR934X_ETH_CFG_SW_ONLY_MODE);

	ath79_register_mdio(1, 0x0);

	/* GMAC0 is left unused */

	/* GMAC1 is connected to MAC0 on the internal switch */
	/* The ethernet port connects to PHY P0, which connects to MAC1
	   on the internal switch */
	ath79_init_mac(ath79_eth1_data.mac_addr, ath79_mac_base, 0);
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_GMII;
	ath79_register_eth(1);


}


MIPS_MACHINE(ATH79_MACH_RB_SXTLITE2ND, "sxt2n", "MikroTik RouterBOARD SXT Lite2",
	    rbsxtlite_setup);

MIPS_MACHINE(ATH79_MACH_RB_SXTLITE5ND, "sxt5n", "MikroTik RouterBOARD SXT Lite5",
	    rbsxtlite_setup);


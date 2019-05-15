/*
 *  MikroTik RouterBOARD 91X support
 *
 *  Copyright (C) 2013 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#define pr_fmt(fmt) "rb91x: " fmt

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
#include <linux/spi/74x164.h>
#include <linux/spi/flash.h>
#include <linux/routerboot.h>
#include <linux/gpio.h>
#include <linux/platform_data/gpio-latch.h>
#include <linux/platform_data/rb91x_nand.h>
#include <linux/platform_data/phy-at803x.h>

#include <asm/prom.h>
#include <asm/mach-ath79/ath79.h>
#include <asm/mach-ath79/ath79_spi_platform.h>
#include <asm/mach-ath79/ar71xx_regs.h>

#include "common.h"
#include "dev-eth.h"
#include "dev-leds-gpio.h"
#include "dev-nfc.h"
#include "dev-usb.h"
#include "dev-spi.h"
#include "dev-wmac.h"
#include "machtypes.h"
#include "pci.h"
#include "routerboot.h"

#define RB_ROUTERBOOT_OFFSET	0x0000
#define RB_ROUTERBOOT_MIN_SIZE	0xb000
#define RB_HARD_CFG_SIZE	0x1000
#define RB_BIOS_OFFSET		0xd000
#define RB_BIOS_SIZE		0x1000
#define RB_SOFT_CFG_OFFSET	0xf000
#define RB_SOFT_CFG_SIZE	0x1000

#define RB91X_FLAG_USB		BIT(0)
#define RB91X_FLAG_PCIE		BIT(1)

#define RB91X_LATCH_GPIO_BASE	32
#define RB91X_LATCH_GPIO(_x)	(RB91X_LATCH_GPIO_BASE + (_x))

#define RB91X_SSR_GPIO_BASE	(RB91X_LATCH_GPIO_BASE + AR934X_GPIO_COUNT)
#define RB91X_SSR_GPIO(_x)	(RB91X_SSR_GPIO_BASE + (_x))

#define RB91X_SSR_BIT_LED1		0
#define RB91X_SSR_BIT_LED2		1
#define RB91X_SSR_BIT_LED3		2
#define RB91X_SSR_BIT_LED4		3
#define RB91X_SSR_BIT_LED5		4
#define RB91X_SSR_BIT_5			5
#define RB91X_SSR_BIT_USB_POWER		6
#define RB91X_SSR_BIT_PCIE_POWER	7

#define RB91X_GPIO_SSR_STROBE	RB91X_LATCH_GPIO(0)
#define RB91X_GPIO_LED_POWER	RB91X_LATCH_GPIO(1)
#define RB91X_GPIO_LED_USER	RB91X_LATCH_GPIO(2)
#define RB91X_GPIO_NAND_READ	RB91X_LATCH_GPIO(3)
#define RB91X_GPIO_NAND_RDY	RB91X_LATCH_GPIO(4)
#define RB91X_GPIO_NLE		RB91X_LATCH_GPIO(11)
#define RB91X_GPIO_NAND_NRW	RB91X_LATCH_GPIO(12)
#define RB91X_GPIO_NAND_NCE	RB91X_LATCH_GPIO(13)
#define RB91X_GPIO_NAND_CLE	RB91X_LATCH_GPIO(14)
#define RB91X_GPIO_NAND_ALE	RB91X_LATCH_GPIO(15)

#define RB91X_GPIO_LED_1	RB91X_SSR_GPIO(RB91X_SSR_BIT_LED1)
#define RB91X_GPIO_LED_2	RB91X_SSR_GPIO(RB91X_SSR_BIT_LED2)
#define RB91X_GPIO_LED_3	RB91X_SSR_GPIO(RB91X_SSR_BIT_LED3)
#define RB91X_GPIO_LED_4	RB91X_SSR_GPIO(RB91X_SSR_BIT_LED4)
#define RB91X_GPIO_LED_5	RB91X_SSR_GPIO(RB91X_SSR_BIT_LED5)
#define RB91X_GPIO_USB_POWER	RB91X_SSR_GPIO(RB91X_SSR_BIT_USB_POWER)
#define RB91X_GPIO_PCIE_POWER	RB91X_SSR_GPIO(RB91X_SSR_BIT_PCIE_POWER)

struct rb_board_info {
	const char *name;
	u32 flags;
};

static struct mtd_partition rb711gr100_spi_partitions[] = {
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

static struct flash_platform_data rb711gr100_spi_flash_data = {
	.parts		= rb711gr100_spi_partitions,
	.nr_parts	= ARRAY_SIZE(rb711gr100_spi_partitions),
};

static int rb711gr100_gpio_latch_gpios[AR934X_GPIO_COUNT] __initdata = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
	12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22
};

static struct gpio_latch_platform_data rb711gr100_gpio_latch_data __initdata = {
	.base = RB91X_LATCH_GPIO_BASE,
	.num_gpios = ARRAY_SIZE(rb711gr100_gpio_latch_gpios),
	.gpios = rb711gr100_gpio_latch_gpios,
	.le_gpio_index = 11,
	.le_active_low = true,
};

static struct rb91x_nand_platform_data rb711gr100_nand_data __initdata = {
	.gpio_nce = RB91X_GPIO_NAND_NCE,
	.gpio_ale = RB91X_GPIO_NAND_ALE,
	.gpio_cle = RB91X_GPIO_NAND_CLE,
	.gpio_rdy = RB91X_GPIO_NAND_RDY,
	.gpio_read = RB91X_GPIO_NAND_READ,
	.gpio_nrw = RB91X_GPIO_NAND_NRW,
	.gpio_nle = RB91X_GPIO_NLE,
};

static u8 rb711gr100_ssr_initdata[] = {
	BIT(RB91X_SSR_BIT_PCIE_POWER) |
	BIT(RB91X_SSR_BIT_USB_POWER) |
	BIT(RB91X_SSR_BIT_5)
};

static struct gen_74x164_chip_platform_data rb711gr100_ssr_data = {
	.base = RB91X_SSR_GPIO_BASE,
	.num_registers = ARRAY_SIZE(rb711gr100_ssr_initdata),
	.init_data = rb711gr100_ssr_initdata,
};

static struct spi_board_info rb711gr100_spi_info[] = {
	{
		.bus_num	= 0,
		.chip_select	= 0,
		.max_speed_hz	= 25000000,
		.modalias	= "m25p80",
		.platform_data  = &rb711gr100_spi_flash_data,
	}, {
		.bus_num	= 0,
		.chip_select	= 1,
		.max_speed_hz	= 10000000,
		.modalias	= "74x164",
		.platform_data	= &rb711gr100_ssr_data,
	}
};

static int rb711gr100_spi_cs_gpios[2] = {
	-ENOENT,
	RB91X_GPIO_SSR_STROBE,
};

static struct ath79_spi_platform_data rb711gr100_spi_data __initdata = {
	.bus_num = 0,
	.num_chipselect = 2,
	.cs_gpios = rb711gr100_spi_cs_gpios,
};

static struct gpio_led rb711gr100_leds[] __initdata = {
	{
		.name		= "rb:green:led1",
		.gpio		= RB91X_GPIO_LED_1,
		.active_low	= 0,
	},
	{
		.name		= "rb:green:led2",
		.gpio		= RB91X_GPIO_LED_2,
		.active_low	= 0,
	},
	{
		.name		= "rb:green:led3",
		.gpio		= RB91X_GPIO_LED_3,
		.active_low	= 0,
	},
	{
		.name		= "rb:green:led4",
		.gpio		= RB91X_GPIO_LED_4,
		.active_low	= 0,
	},
	{
		.name		= "rb:green:led5",
		.gpio		= RB91X_GPIO_LED_5,
		.active_low	= 0,
	},
	{
		.name		= "rb:green:user",
		.gpio		= RB91X_GPIO_LED_USER,
		.active_low	= 0,
	},
	{
		.name		= "rb:green:power",
		.gpio		= RB91X_GPIO_LED_POWER,
		.active_low	= 0,
		.default_state	= LEDS_GPIO_DEFSTATE_KEEP,
	},
};

static struct at803x_platform_data rb91x_at803x_data = {
	.disable_smarteee = 1,
	.enable_rgmii_rx_delay = 1,
	.enable_rgmii_tx_delay = 1,
};

static struct mdio_board_info rb91x_mdio0_info[] = {
	{
		.bus_id = "ag71xx-mdio.0",
		.mdio_addr = 0,
		.platform_data = &rb91x_at803x_data,
	},
};

static void __init rb711gr100_init_partitions(const struct rb_info *info)
{
	rb711gr100_spi_partitions[0].size = info->hard_cfg_offs;
	rb711gr100_spi_partitions[1].offset = info->hard_cfg_offs;

	rb711gr100_spi_partitions[3].offset = info->soft_cfg_offs;
}

void __init rb711gr100_wlan_init(void)
{
	char *caldata;
	u8 wlan_mac[ETH_ALEN];

	caldata = rb_get_wlan_data();
	if (caldata == NULL)
		return;

	ath79_init_mac(wlan_mac, ath79_mac_base, 1);
	ath79_register_wmac(caldata + 0x1000, wlan_mac);

	kfree(caldata);
}

#define RB_BOARD_INFO(_name, _flags)	\
	{				\
		.name = (_name),	\
		.flags = (_flags),	\
	}

static const struct rb_board_info rb711gr100_boards[] __initconst = {
	RB_BOARD_INFO("911G-2HPnD", 0),
	RB_BOARD_INFO("911G-5HPnD", 0),
	RB_BOARD_INFO("912UAG-2HPnD", RB91X_FLAG_USB | RB91X_FLAG_PCIE),
	RB_BOARD_INFO("912UAG-5HPnD", RB91X_FLAG_USB | RB91X_FLAG_PCIE),
};

static u32 rb711gr100_get_flags(const struct rb_info *info)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(rb711gr100_boards); i++) {
		const struct rb_board_info *bi;

		bi = &rb711gr100_boards[i];
		if (strcmp(info->board_name, bi->name) == 0)
			return bi->flags;
	}

	return 0;
}

static void __init rb711gr100_setup(void)
{
	const struct rb_info *info;
	char buf[64];
	u32 flags;

	info = rb_init_info((void *) KSEG1ADDR(0x1f000000), 0x10000);
	if (!info)
		return;

	scnprintf(buf, sizeof(buf), "Mikrotik RouterBOARD %s",
		  (info->board_name) ? info->board_name : "");
	mips_set_machine_name(buf);

	rb711gr100_init_partitions(info);
	ath79_register_spi(&rb711gr100_spi_data, rb711gr100_spi_info,
			   ARRAY_SIZE(rb711gr100_spi_info));

	ath79_setup_ar934x_eth_cfg(AR934X_ETH_CFG_RGMII_GMAC0 |
				   AR934X_ETH_CFG_RXD_DELAY |
				   AR934X_ETH_CFG_SW_ONLY_MODE);

	ath79_register_mdio(0, 0x0);

	mdiobus_register_board_info(rb91x_mdio0_info,
				    ARRAY_SIZE(rb91x_mdio0_info));

	ath79_init_mac(ath79_eth0_data.mac_addr, ath79_mac_base, 0);
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth0_data.phy_mask = BIT(0);
	ath79_eth0_pll_data.pll_1000 = 0x02000000;

	ath79_register_eth(0);

	rb711gr100_wlan_init();

	platform_device_register_data(NULL, "rb91x-nand", -1,
				      &rb711gr100_nand_data,
				      sizeof(rb711gr100_nand_data));

	platform_device_register_data(NULL, "gpio-latch", -1,
				      &rb711gr100_gpio_latch_data,
				      sizeof(rb711gr100_gpio_latch_data));

	ath79_register_leds_gpio(-1, ARRAY_SIZE(rb711gr100_leds),
				 rb711gr100_leds);

	flags = rb711gr100_get_flags(info);

	if (flags & RB91X_FLAG_USB)
		ath79_register_usb();

	if (flags & RB91X_FLAG_PCIE)
		ath79_register_pci();

}

MIPS_MACHINE_NONAME(ATH79_MACH_RB_711GR100, "711Gr100", rb711gr100_setup);

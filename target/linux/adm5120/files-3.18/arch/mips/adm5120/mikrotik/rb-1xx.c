/*
 *  Mikrotik RouterBOARD 1xx series support
 *
 *  Copyright (C) 2007-2008 Gabor Juhos <juhosg@openwrt.org>
 *
 *  NAND initialization code was based on a driver for Linux 2.6.19+ which
 *  was derived from the driver for Linux 2.4.xx published by Mikrotik for
 *  their RouterBoard 1xx and 5xx series boards.
 *    Copyright (C) 2007 David Goodenough <david.goodenough@linkchoose.co.uk>
 *    Copyright (C) 2007 Florian Fainelli <florian@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 */

#include "rb-1xx.h"

#define RB1XX_NAND_CHIP_DELAY	25

#define RB1XX_KEYS_POLL_INTERVAL	20
#define RB1XX_KEYS_DEBOUNCE_INTERVAL	(3 * RB1XX_KEYS_POLL_INTERVAL)

static struct adm5120_pci_irq rb1xx_pci_irqs[] __initdata = {
	PCIIRQ(1, 0, 1, ADM5120_IRQ_PCI0),
	PCIIRQ(2, 0, 1, ADM5120_IRQ_PCI1),
	PCIIRQ(3, 0, 1, ADM5120_IRQ_PCI2)
};

static struct mtd_partition rb1xx_nor_parts[] = {
	{
		.name	= "booter",
		.offset	= 0,
		.size	= 64*1024,
		.mask_flags = MTD_WRITEABLE,
	} , {
		.name	= "firmware",
		.offset	= MTDPART_OFS_APPEND,
		.size	= MTDPART_SIZ_FULL,
	}
};

static struct mtd_partition rb1xx_nand_parts[] = {
	{
		.name	= "kernel",
		.offset	= 0,
		.size	= 4 * 1024 * 1024,
	} , {
		.name	= "rootfs",
		.offset	= MTDPART_OFS_NXTBLK,
		.size	= MTDPART_SIZ_FULL
	}
};

/*
 * We need to use the OLD Yaffs-1 OOB layout, otherwise the RB bootloader
 * will not be able to find the kernel that we load.  So set the oobinfo
 * when creating the partitions
 */
static struct nand_ecclayout rb1xx_nand_ecclayout = {
	.eccbytes	= 6,
	.eccpos		= { 8, 9, 10, 13, 14, 15 },
	.oobavail	= 9,
	.oobfree	= { { 0, 4 }, { 6, 2 }, { 11, 2 }, { 4, 1 } }
};

/*--------------------------------------------------------------------------*/

static int rb1xx_nand_fixup(struct mtd_info *mtd)
{
	struct nand_chip *chip = mtd->priv;

	if (mtd->writesize == 512)
		chip->ecc.layout = &rb1xx_nand_ecclayout;

	return 0;
}

struct platform_nand_data rb1xx_nand_data __initdata = {
	.chip = {
		.nr_chips	= 1,
		.nr_partitions	= ARRAY_SIZE(rb1xx_nand_parts),
		.partitions	= rb1xx_nand_parts,
		.chip_delay	= RB1XX_NAND_CHIP_DELAY,
		.chip_fixup	= rb1xx_nand_fixup,
	},
};

struct gpio_keys_button rb1xx_gpio_buttons[] __initdata = {
	{
		.desc		= "reset_button",
		.type		= EV_KEY,
		.code           = KEY_RESTART,
		.debounce_interval = RB1XX_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= ADM5120_GPIO_PIN7,
	}
};

static void __init rb1xx_mac_setup(void)
{
	if (rb_hs.mac_base != NULL && is_valid_ether_addr(rb_hs.mac_base)) {
		adm5120_setup_eth_macs(rb_hs.mac_base);
	} else {
		u8 mac[ETH_ALEN];

		random_ether_addr(mac);
		adm5120_setup_eth_macs(mac);
	}
}

void __init rb1xx_add_device_flash(void)
{
	/* setup data for flash0 device */
	adm5120_flash0_data.nr_parts = ARRAY_SIZE(rb1xx_nor_parts);
	adm5120_flash0_data.parts = rb1xx_nor_parts;
	adm5120_flash0_data.window_size = 128*1024;

	adm5120_add_device_flash(0);
}

void __init rb1xx_add_device_nand(void)
{
	/* enable NAND flash interface */
	adm5120_nand_enable();

	/* initialize NAND chip */
	adm5120_nand_set_spn(1);
	adm5120_nand_set_wpn(0);

	adm5120_add_device_nand(&rb1xx_nand_data);
}

void __init rb1xx_generic_setup(void)
{
	if (adm5120_package_bga())
		adm5120_pci_set_irq_map(ARRAY_SIZE(rb1xx_pci_irqs),
					rb1xx_pci_irqs);

	adm5120_add_device_uart(0);
	adm5120_add_device_uart(1);

	adm5120_register_gpio_buttons(-1, RB1XX_KEYS_POLL_INTERVAL,
				      ARRAY_SIZE(rb1xx_gpio_buttons),
				      rb1xx_gpio_buttons);

	rb1xx_add_device_flash();
	rb1xx_mac_setup();
}

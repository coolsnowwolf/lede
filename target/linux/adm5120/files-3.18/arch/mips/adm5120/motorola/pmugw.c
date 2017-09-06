/*
 *  Motorola Powerline MU Gateway board
 *
 *  Copyright (C) 2008 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/irq.h>
#include <linux/etherdevice.h>

#include <asm/mips_machine.h>

#include <asm/mach-adm5120/adm5120_defs.h>
#include <asm/mach-adm5120/adm5120_platform.h>
#include <asm/mach-adm5120/adm5120_info.h>

#include <prom/admboot.h>

#define PMUGW_CONFIG_OFFSET	0x10000
#define PMUGW_CONFIG_SIZE	0x1000

static struct mtd_partition pmugw_partitions[] = {
	{
		.name	= "admboot",
		.offset	= 0,
		.size	= 64*1024,
		.mask_flags = MTD_WRITEABLE,
	} , {
		.name	= "boardcfg",
		.offset	= MTDPART_OFS_APPEND,
		.size	= 64*1024,
	} , {
		.name	= "firmware",
		.offset	= MTDPART_OFS_APPEND,
		.size	= MTDPART_SIZ_FULL,
	}
};

static u8 pmugw_vlans[6] __initdata = {
	0x41, 0x42, 0x44, 0x48, 0x50, 0x00
};

static __init void pmugw_setup_mac(void)
{
	u8 mac_base[6];
	int err;

	err = admboot_get_mac_base(PMUGW_CONFIG_OFFSET,
				   PMUGW_CONFIG_SIZE, mac_base);

	if ((err) || !is_valid_ether_addr(mac_base))
		random_ether_addr(mac_base);

	adm5120_setup_eth_macs(mac_base);
}

static void switch_bank_gpio5(unsigned bank)
{
	switch (bank) {
	case 0:
		gpio_set_value(ADM5120_GPIO_PIN5, 0);
		break;
	case 1:
		gpio_set_value(ADM5120_GPIO_PIN5, 1);
		break;
	}
}

void __init pmugw_setup(void)
{
	/* setup flash A20 line */
	gpio_request(ADM5120_GPIO_PIN5, NULL);
	gpio_direction_output(ADM5120_GPIO_PIN5, 0);
	adm5120_flash0_data.switch_bank = switch_bank_gpio5;

	adm5120_flash0_data.nr_parts = ARRAY_SIZE(pmugw_partitions);
	adm5120_flash0_data.parts = pmugw_partitions;

	adm5120_add_device_uart(1); /* ttyAM0 */
	adm5120_add_device_uart(0); /* ttyAM1 */

	adm5120_add_device_flash(0);

	pmugw_setup_mac();
	adm5120_add_device_switch(5, pmugw_vlans);
}

MIPS_MACHINE(MACH_ADM5120_PMUGW, "PMUGW", "Motorola Powerline MU Gateway",
	     pmugw_setup);

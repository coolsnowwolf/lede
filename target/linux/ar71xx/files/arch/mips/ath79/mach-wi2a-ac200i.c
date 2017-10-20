/*
 * Nokia WI2A-AC200i support
 *
 * Copyright (c) 2012 Qualcomm Atheros
 * Copyright (c) 2012-2013 Gabor Juhos <juhosg@openwrt.org>
 * Copyright (c) 2017 Felix Fietkau <nbd@nbd.name>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

#include <linux/platform_device.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/platform/ar934x_nfc.h>

#include <asm/mach-ath79/ar71xx_regs.h>
#include <asm/fw/fw.h>

#include "common.h"
#include "pci.h"
#include "dev-ap9x-pci.h"
#include "dev-gpio-buttons.h"
#include "dev-eth.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-nfc.h"
#include "dev-usb.h"
#include "dev-wmac.h"
#include "machtypes.h"

#define AC200I_GPIO_BTN_RESET		17

#define AC200I_KEYS_POLL_INTERVAL	20	/* msecs */
#define AC200I_KEYS_DEBOUNCE_INTERVAL	(3 * AC200I_KEYS_POLL_INTERVAL)

#define AC200I_MAC_ADDR			0x1f040249
#define AC200I_MAC1_OFFSET		6
#define AC200I_WMAC_CALDATA_ADDR	0x1f061000

static struct gpio_led ac200i_leds_gpio[] __initdata = {
	{
		.name		= "nokia:red:wlan-2g",
		.gpio		= 0,
		.active_low	= 1,
	},
	{
		.name		= "nokia:green:power",
		.gpio		= 1,
		.active_low	= 1,
	},
	{
		.name		= "nokia:green:wlan-2g",
		.gpio		= 2,
		.active_low	= 1,
	},
	{
		.name		= "nokia:green:ctrl",
		.gpio		= 3,
		.active_low	= 1,
	},
	{
		.name		= "nokia:green:eth",
		.gpio		= 4,
		.active_low	= 1,
	},
	{
		.name		= "nokia:red:power",
		.gpio		= 13,
		.active_low	= 1,
	},
	{
		.name		= "nokia:red:eth",
		.gpio		= 14,
		.active_low	= 1,
	},
	{
		.name		= "nokia:red:wlan-5g",
		.gpio		= 18,
		.active_low	= 1,
	},
	{
		.name		= "nokia:green:wlan-5g",
		.gpio		= 19,
		.active_low	= 1,
	},
	{
		.name		= "nokia:red:ctrl",
		.gpio		= 20,
		.active_low	= 1,
	},
};

static struct gpio_keys_button ac200i_gpio_keys[] __initdata = {
	{
		.desc		= "Reset button",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = AC200I_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= AC200I_GPIO_BTN_RESET,
		.active_low	= 1,
	},
};

static struct mtd_partition ac200i_nand_partitions[] = {
	{
		.name	= "cfg",
		.offset	= 0x0100000,
		.size	= 0x1800000,
		.mask_flags = MTD_WRITEABLE,
	},
	{
		.name	= "kernel",
		.offset	= 0x2000000,
		.size	= 0x0400000,
	},
	{
		.name	= "ubi",
		.offset	= 0x2400000,
		.size	= 0x2000000,
	},
	{
		.name	= "kernel",
		.offset	= 0x5000000,
		.size	= 0x0400000,
	},
	{
		.name	= "ubi",
		.offset	= 0x5400000,
		.size	= 0x2000000,
	},
};

static const char *boot_getenv(const char *key)
{
	const char *start = (const char *) KSEG1ADDR(0x1f070000);
	const char *end = start + 0x20000;
	const char *addr;

	for (addr = start + 4;
	     *addr && *addr != 0xff && addr < end &&
	     strnlen(addr, end - addr) < end - addr;
	     addr += strnlen(addr, end - addr) + 1) {
		const char *val;

		val = strchr(addr, '=');
		if (!val)
			continue;

		if (strncmp(addr, key, val - addr))
			continue;

		return val + 1;
	}
	return NULL;
}

static void __init ac200i_setup(void)
{
	const char *img;
	u8 *wmac = (u8 *) KSEG1ADDR(AC200I_WMAC_CALDATA_ADDR);
	u8 *mac_addr = (u8 *) KSEG1ADDR(AC200I_MAC_ADDR);

	ath79_register_m25p80(NULL);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(ac200i_leds_gpio),
				 ac200i_leds_gpio);
	ath79_register_gpio_keys_polled(-1, AC200I_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(ac200i_gpio_keys),
					ac200i_gpio_keys);

	ath79_register_usb();
	ath79_nfc_set_parts(ac200i_nand_partitions,
			    ARRAY_SIZE(ac200i_nand_partitions));
	ath79_nfc_set_ecc_mode(AR934X_NFC_ECC_HW);
	ath79_register_nfc();

	ath79_register_wmac(wmac, NULL);

	ath79_register_mdio(0, 0x0);
	ath79_init_mac(ath79_eth0_data.mac_addr, mac_addr, 0);

	/* GMAC0 is connected to the SGMII interface */
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_SGMII;
	ath79_eth0_data.phy_mask = BIT(4);
	ath79_eth0_data.mii_bus_dev = &ath79_mdio0_device.dev;
	ath79_eth0_pll_data.pll_1000 = 0x03000101;
	ath79_eth0_pll_data.pll_100 = 0x80000101;
	ath79_eth0_pll_data.pll_10 = 0x80001313;

	img = boot_getenv("dualPartition");
	if (img && !strcmp(img, "imgA")) {
		ac200i_nand_partitions[3].name = "kernel_alt";
		ac200i_nand_partitions[4].name = "ubi_alt";
	} else {
		ac200i_nand_partitions[1].name = "kernel_alt";
		ac200i_nand_partitions[2].name = "ubi_alt";
	}

	ath79_register_eth(0);

	ath79_register_pci();
}

MIPS_MACHINE(ATH79_MACH_WI2A_AC200I, "WI2A-AC200i",
	     "Nokia WI2A-AC200i",
	     ac200i_setup);

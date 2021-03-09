/*
 *  8devices Rambutan board support
 *
 *  Copyright (C) 2017 Mantas Pucka <mantas@8devices.com>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/pci.h>
#include <linux/phy.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>
#include <linux/ath9k_platform.h>

#include <asm/mach-ath79/ath79.h>
#include <asm/mach-ath79/ar71xx_regs.h>
#include <linux/platform/ar934x_nfc.h>
#include <linux/ar8216_platform.h>
#include <linux/platform_data/phy-at803x.h>
#include "common.h"
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-spi.h"
#include "dev-nfc.h"
#include "dev-usb.h"
#include "dev-wmac.h"
#include "machtypes.h"
#include "pci.h"

static struct at803x_platform_data rambutan_ar8032_data = {
	.has_reset_gpio = 1,
	.reset_gpio = 17,
};

static struct mdio_board_info rambutan_mdio0_info[] = {
	{
		.bus_id = "ag71xx-mdio.0",
		.mdio_addr = 0,
		.platform_data = &rambutan_ar8032_data,
	},
};

static struct at803x_platform_data rambutan_ar8033_data = {
	.has_reset_gpio = 1,
	.override_sgmii_aneg = 1,
	.reset_gpio = 23,
};

static struct mdio_board_info rambutan_mdio1_info[] = {
	{
		.bus_id = "ag71xx-mdio.1",
		.mdio_addr = 0,
		.platform_data = &rambutan_ar8033_data,
	},
};

static void __init rambutan_setup(void)
{
	ath79_nfc_set_ecc_mode(AR934X_NFC_ECC_HW);
	ath79_register_nfc();
	ath79_register_usb();
	ath79_register_pci();
	ath79_register_wmac_simple();

	mdiobus_register_board_info(rambutan_mdio0_info,
				    ARRAY_SIZE(rambutan_mdio0_info));
	mdiobus_register_board_info(rambutan_mdio1_info,
				    ARRAY_SIZE(rambutan_mdio1_info));
	ath79_register_mdio(0, 0x0);
	ath79_register_mdio(1, 0x0);

	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ath79_eth0_data.phy_mask = BIT(0);
	ath79_eth0_data.mii_bus_dev = &ath79_mdio0_device.dev;
	ath79_register_eth(0);

	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_SGMII;
	ath79_eth1_data.phy_mask = BIT(0);
	ath79_eth1_data.mii_bus_dev = &ath79_mdio1_device.dev;
	ath79_eth1_data.enable_sgmii_fixup = 1;
	ath79_eth1_pll_data.pll_1000 = 0x17000000;
	ath79_eth1_pll_data.pll_10 = 0x1313;
	ath79_register_eth(1);
}

MIPS_MACHINE(ATH79_MACH_RAMBUTAN, "RAMBUTAN", "8devices Rambutan board",
	     rambutan_setup);


/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * MFD driver for Airoha AN8855 Switch
 */
#ifndef _LINUX_INCLUDE_MFD_AIROHA_AN8855_MFD_H
#define _LINUX_INCLUDE_MFD_AIROHA_AN8855_MFD_H

#include <linux/bitfield.h>

/* MII Registers */
#define AN8855_PHY_SELECT_PAGE		0x1f
#define   AN8855_PHY_PAGE		GENMASK(2, 0)
#define   AN8855_PHY_PAGE_STANDARD	FIELD_PREP_CONST(AN8855_PHY_PAGE, 0x0)
#define   AN8855_PHY_PAGE_EXTENDED_1	FIELD_PREP_CONST(AN8855_PHY_PAGE, 0x1)
#define   AN8855_PHY_PAGE_EXTENDED_4	FIELD_PREP_CONST(AN8855_PHY_PAGE, 0x4)

/* MII Registers Page 4 */
#define AN8855_PBUS_MODE		0x10
#define   AN8855_PBUS_MODE_ADDR_FIXED	0x0
#define AN8855_PBUS_MODE_ADDR_INCR	BIT(15)
#define AN8855_PBUS_WR_ADDR_HIGH	0x11
#define AN8855_PBUS_WR_ADDR_LOW		0x12
#define AN8855_PBUS_WR_DATA_HIGH	0x13
#define AN8855_PBUS_WR_DATA_LOW		0x14
#define AN8855_PBUS_RD_ADDR_HIGH	0x15
#define AN8855_PBUS_RD_ADDR_LOW		0x16
#define AN8855_PBUS_RD_DATA_HIGH	0x17
#define AN8855_PBUS_RD_DATA_LOW		0x18

struct an8855_mfd_priv {
	struct device *dev;
	struct mii_bus *bus;

	unsigned int switch_addr;
	u16 current_page;
};

int an8855_mii_set_page(struct an8855_mfd_priv *priv, u8 phy_id,
			u8 page);

#endif

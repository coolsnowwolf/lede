// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2018 MediaTek Inc.
 * Author: Weijie Gao <weijie.gao@mediatek.com>
 */

#include <linux/kernel.h>
#include <linux/delay.h>

#include "mt753x.h"
#include "mt753x_regs.h"

/* MT7530 registers */

/* Unique fields of PMCR for MT7530 */
#define FORCE_MODE			BIT(15)

/* Unique fields of GMACCR for MT7530 */
#define VLAN_SUPT_NO_S			14
#define VLAN_SUPT_NO_M			0x1c000
#define LATE_COL_DROP			BIT(13)

/* Unique fields of (M)HWSTRAP for MT7530 */
#define BOND_OPTION			BIT(24)
#define P5_PHY0_SEL			BIT(20)
#define CHG_TRAP			BIT(16)
#define LOOPDET_DIS			BIT(14)
#define P5_INTF_SEL_GMAC5		BIT(13)
#define SMI_ADDR_S			11
#define SMI_ADDR_M			0x1800
#define XTAL_FSEL_S			9
#define XTAL_FSEL_M			0x600
#define P6_INTF_DIS			BIT(8)
#define P5_INTF_MODE_RGMII		BIT(7)
#define P5_INTF_DIS_S			BIT(6)
#define C_MDIO_BPS_S			BIT(5)
#define EEPROM_EN_S			BIT(4)

/* PHY EEE Register bitmap of define */
#define PHY_DEV07			0x07
#define PHY_DEV07_REG_03C		0x3c

/* PHY Extend Register 0x14 bitmap of define */
#define PHY_EXT_REG_14			0x14

/* Fields of PHY_EXT_REG_14 */
#define PHY_EN_DOWN_SHFIT		BIT(4)

/* PHY Token Ring Register 0x10 bitmap of define */
#define PHY_TR_REG_10			0x10

/* PHY Token Ring Register 0x12 bitmap of define */
#define PHY_TR_REG_12			0x12

/* PHY LPI PCS/DSP Control Register bitmap of define */
#define PHY_LPI_REG_11			0x11

/* PHY DEV 0x1e Register bitmap of define */
#define PHY_DEV1E			0x1e
#define PHY_DEV1E_REG_123		0x123
#define PHY_DEV1E_REG_A6		0xa6

/* Values of XTAL_FSEL */
#define XTAL_20MHZ			1
#define XTAL_40MHZ			2
#define XTAL_25MHZ			3

#define P6ECR				0x7830
#define P6_INTF_MODE_TRGMII		BIT(0)

#define TRGMII_TXCTRL			0x7a40
#define TRAIN_TXEN			BIT(31)
#define TXC_INV				BIT(30)
#define TX_DOEO				BIT(29)
#define TX_RST				BIT(28)

#define TRGMII_TD0_CTRL			0x7a50
#define TRGMII_TD1_CTRL			0x7a58
#define TRGMII_TD2_CTRL			0x7a60
#define TRGMII_TD3_CTRL			0x7a68
#define TRGMII_TXCTL_CTRL		0x7a70
#define TRGMII_TCK_CTRL			0x7a78
#define TRGMII_TD_CTRL(n)		(0x7a50 + (n) * 8)
#define NUM_TRGMII_CTRL			6
#define TX_DMPEDRV			BIT(31)
#define TX_DM_SR			BIT(15)
#define TX_DMERODT			BIT(14)
#define TX_DMOECTL			BIT(13)
#define TX_TAP_S			8
#define TX_TAP_M			0xf00
#define TX_TRAIN_WD_S			0
#define TX_TRAIN_WD_M			0xff

#define TRGMII_TD0_ODT			0x7a54
#define TRGMII_TD1_ODT			0x7a5c
#define TRGMII_TD2_ODT			0x7a64
#define TRGMII_TD3_ODT			0x7a6c
#define TRGMII_TXCTL_ODT		0x7574
#define TRGMII_TCK_ODT			0x757c
#define TRGMII_TD_ODT(n)		(0x7a54 + (n) * 8)
#define NUM_TRGMII_ODT			6
#define TX_DM_DRVN_PRE_S		30
#define TX_DM_DRVN_PRE_M		0xc0000000
#define TX_DM_DRVP_PRE_S		28
#define TX_DM_DRVP_PRE_M		0x30000000
#define TX_DM_TDSEL_S			24
#define TX_DM_TDSEL_M			0xf000000
#define TX_ODTEN			BIT(23)
#define TX_DME_PRE			BIT(20)
#define TX_DM_DRVNT0			BIT(19)
#define TX_DM_DRVPT0			BIT(18)
#define TX_DM_DRVNTE			BIT(17)
#define TX_DM_DRVPTE			BIT(16)
#define TX_DM_ODTN_S			12
#define TX_DM_ODTN_M			0x7000
#define TX_DM_ODTP_S			8
#define TX_DM_ODTP_M			0x700
#define TX_DM_DRVN_S			4
#define TX_DM_DRVN_M			0xf0
#define TX_DM_DRVP_S			0
#define TX_DM_DRVP_M			0x0f

#define P5RGMIIRXCR			0x7b00
#define CSR_RGMII_RCTL_CFG_S		24
#define CSR_RGMII_RCTL_CFG_M		0x7000000
#define CSR_RGMII_RXD_CFG_S		16
#define CSR_RGMII_RXD_CFG_M		0x70000
#define CSR_RGMII_EDGE_ALIGN		BIT(8)
#define CSR_RGMII_RXC_90DEG_CFG_S	4
#define CSR_RGMII_RXC_90DEG_CFG_M	0xf0
#define CSR_RGMII_RXC_0DEG_CFG_S	0
#define CSR_RGMII_RXC_0DEG_CFG_M	0x0f

#define P5RGMIITXCR			0x7b04
#define CSR_RGMII_TXEN_CFG_S		16
#define CSR_RGMII_TXEN_CFG_M		0x70000
#define CSR_RGMII_TXD_CFG_S		8
#define CSR_RGMII_TXD_CFG_M		0x700
#define CSR_RGMII_TXC_CFG_S		0
#define CSR_RGMII_TXC_CFG_M		0x1f

#define CHIP_REV			0x7ffc
#define CHIP_NAME_S			16
#define CHIP_NAME_M			0xffff0000
#define CHIP_REV_S			0
#define CHIP_REV_M			0x0f

/* MMD registers */
#define CORE_PLL_GROUP2			0x401
#define RG_SYSPLL_EN_NORMAL		BIT(15)
#define RG_SYSPLL_VODEN			BIT(14)
#define RG_SYSPLL_POSDIV_S		5
#define RG_SYSPLL_POSDIV_M		0x60

#define CORE_PLL_GROUP4			0x403
#define RG_SYSPLL_DDSFBK_EN		BIT(12)
#define RG_SYSPLL_BIAS_EN		BIT(11)
#define RG_SYSPLL_BIAS_LPF_EN		BIT(10)

#define CORE_PLL_GROUP5			0x404
#define RG_LCDDS_PCW_NCPO1_S		0
#define RG_LCDDS_PCW_NCPO1_M		0xffff

#define CORE_PLL_GROUP6			0x405
#define RG_LCDDS_PCW_NCPO0_S		0
#define RG_LCDDS_PCW_NCPO0_M		0xffff

#define CORE_PLL_GROUP7			0x406
#define RG_LCDDS_PWDB			BIT(15)
#define RG_LCDDS_ISO_EN			BIT(13)
#define RG_LCCDS_C_S			4
#define RG_LCCDS_C_M			0x70
#define RG_LCDDS_PCW_NCPO_CHG		BIT(3)

#define CORE_PLL_GROUP10		0x409
#define RG_LCDDS_SSC_DELTA_S		0
#define RG_LCDDS_SSC_DELTA_M		0xfff

#define CORE_PLL_GROUP11		0x40a
#define RG_LCDDS_SSC_DELTA1_S		0
#define RG_LCDDS_SSC_DELTA1_M		0xfff

#define CORE_GSWPLL_GCR_1		0x040d
#define GSWPLL_PREDIV_S			14
#define GSWPLL_PREDIV_M			0xc000
#define GSWPLL_POSTDIV_200M_S		12
#define GSWPLL_POSTDIV_200M_M		0x3000
#define GSWPLL_EN_PRE			BIT(11)
#define GSWPLL_FBKSEL			BIT(10)
#define GSWPLL_BP			BIT(9)
#define GSWPLL_BR			BIT(8)
#define GSWPLL_FBKDIV_200M_S		0
#define GSWPLL_FBKDIV_200M_M		0xff

#define CORE_GSWPLL_GCR_2		0x040e
#define GSWPLL_POSTDIV_500M_S		8
#define GSWPLL_POSTDIV_500M_M		0x300
#define GSWPLL_FBKDIV_500M_S		0
#define GSWPLL_FBKDIV_500M_M		0xff

#define TRGMII_GSW_CLK_CG		0x0410
#define TRGMIICK_EN			BIT(1)
#define GSWCK_EN			BIT(0)

static int mt7530_mii_read(struct gsw_mt753x *gsw, int phy, int reg)
{
	if (phy < MT753X_NUM_PHYS)
		phy = (gsw->phy_base + phy) & MT753X_SMI_ADDR_MASK;

	return mdiobus_read(gsw->host_bus, phy, reg);
}

static void mt7530_mii_write(struct gsw_mt753x *gsw, int phy, int reg, u16 val)
{
	if (phy < MT753X_NUM_PHYS)
		phy = (gsw->phy_base + phy) & MT753X_SMI_ADDR_MASK;

	mdiobus_write(gsw->host_bus, phy, reg, val);
}

static int mt7530_mmd_read(struct gsw_mt753x *gsw, int addr, int devad, u16 reg)
{
	u16 val;

	if (addr < MT753X_NUM_PHYS)
		addr = (gsw->phy_base + addr) & MT753X_SMI_ADDR_MASK;

	mutex_lock(&gsw->host_bus->mdio_lock);

	gsw->host_bus->write(gsw->host_bus, addr, MII_MMD_ACC_CTL_REG,
			     (MMD_ADDR << MMD_CMD_S) |
			     ((devad << MMD_DEVAD_S) & MMD_DEVAD_M));

	gsw->host_bus->write(gsw->host_bus, addr, MII_MMD_ADDR_DATA_REG, reg);

	gsw->host_bus->write(gsw->host_bus, addr, MII_MMD_ACC_CTL_REG,
			     (MMD_DATA << MMD_CMD_S) |
			     ((devad << MMD_DEVAD_S) & MMD_DEVAD_M));

	val = gsw->host_bus->read(gsw->host_bus, addr, MII_MMD_ADDR_DATA_REG);

	mutex_unlock(&gsw->host_bus->mdio_lock);

	return val;
}

static void mt7530_mmd_write(struct gsw_mt753x *gsw, int addr, int devad,
			     u16 reg, u16 val)
{
	if (addr < MT753X_NUM_PHYS)
		addr = (gsw->phy_base + addr) & MT753X_SMI_ADDR_MASK;

	mutex_lock(&gsw->host_bus->mdio_lock);

	gsw->host_bus->write(gsw->host_bus, addr, MII_MMD_ACC_CTL_REG,
		      (MMD_ADDR << MMD_CMD_S) |
		      ((devad << MMD_DEVAD_S) & MMD_DEVAD_M));

	gsw->host_bus->write(gsw->host_bus, addr, MII_MMD_ADDR_DATA_REG, reg);

	gsw->host_bus->write(gsw->host_bus, addr, MII_MMD_ACC_CTL_REG,
		      (MMD_DATA << MMD_CMD_S) |
		      ((devad << MMD_DEVAD_S) & MMD_DEVAD_M));

	gsw->host_bus->write(gsw->host_bus, addr, MII_MMD_ADDR_DATA_REG, val);

	mutex_unlock(&gsw->host_bus->mdio_lock);
}

static void mt7530_core_reg_write(struct gsw_mt753x *gsw, u32 reg, u32 val)
{
	gsw->mmd_write(gsw, 0, 0x1f, reg, val);
}

static void mt7530_trgmii_setting(struct gsw_mt753x *gsw)
{
	u16 i;

	mt7530_core_reg_write(gsw, CORE_PLL_GROUP5, 0x0780);
	mdelay(1);
	mt7530_core_reg_write(gsw, CORE_PLL_GROUP6, 0);
	mt7530_core_reg_write(gsw, CORE_PLL_GROUP10, 0x87);
	mdelay(1);
	mt7530_core_reg_write(gsw, CORE_PLL_GROUP11, 0x87);

	/* PLL BIAS enable */
	mt7530_core_reg_write(gsw, CORE_PLL_GROUP4,
			      RG_SYSPLL_DDSFBK_EN | RG_SYSPLL_BIAS_EN);
	mdelay(1);

	/* PLL LPF enable */
	mt7530_core_reg_write(gsw, CORE_PLL_GROUP4,
			      RG_SYSPLL_DDSFBK_EN |
			      RG_SYSPLL_BIAS_EN | RG_SYSPLL_BIAS_LPF_EN);

	/* sys PLL enable */
	mt7530_core_reg_write(gsw, CORE_PLL_GROUP2,
			      RG_SYSPLL_EN_NORMAL | RG_SYSPLL_VODEN |
			      (1 << RG_SYSPLL_POSDIV_S));

	/* LCDDDS PWDS */
	mt7530_core_reg_write(gsw, CORE_PLL_GROUP7,
			      (3 << RG_LCCDS_C_S) |
			      RG_LCDDS_PWDB | RG_LCDDS_ISO_EN);
	mdelay(1);

	/* Enable MT7530 TRGMII clock */
	mt7530_core_reg_write(gsw, TRGMII_GSW_CLK_CG, GSWCK_EN | TRGMIICK_EN);

	/* lower Tx Driving */
	for (i = 0 ; i < NUM_TRGMII_ODT; i++)
		mt753x_reg_write(gsw, TRGMII_TD_ODT(i),
				 (4 << TX_DM_DRVP_S) | (4 << TX_DM_DRVN_S));
}

static void mt7530_rgmii_setting(struct gsw_mt753x *gsw)
{
	u32 val;

	mt7530_core_reg_write(gsw, CORE_PLL_GROUP5, 0x0c80);
	mdelay(1);
	mt7530_core_reg_write(gsw, CORE_PLL_GROUP6, 0);
	mt7530_core_reg_write(gsw, CORE_PLL_GROUP10, 0x87);
	mdelay(1);
	mt7530_core_reg_write(gsw, CORE_PLL_GROUP11, 0x87);

	val = mt753x_reg_read(gsw, TRGMII_TXCTRL);
	val &= ~TXC_INV;
	mt753x_reg_write(gsw, TRGMII_TXCTRL, val);

	mt753x_reg_write(gsw, TRGMII_TCK_CTRL,
			 (8 << TX_TAP_S) | (0x55 << TX_TRAIN_WD_S));
}

static int mt7530_mac_port_setup(struct gsw_mt753x *gsw)
{
	u32 hwstrap, p6ecr = 0, p5mcr, p6mcr, phyad;

	hwstrap = mt753x_reg_read(gsw, MHWSTRAP);
	hwstrap &= ~(P6_INTF_DIS | P5_INTF_MODE_RGMII | P5_INTF_DIS_S);
	hwstrap |= P5_INTF_SEL_GMAC5;
	if (!gsw->port5_cfg.enabled) {
		p5mcr = FORCE_MODE;
		hwstrap |= P5_INTF_DIS_S;
	} else {
		p5mcr = (IPG_96BIT_WITH_SHORT_IPG << IPG_CFG_S) |
			MAC_MODE | MAC_TX_EN | MAC_RX_EN |
			BKOFF_EN | BACKPR_EN;

		if (gsw->port5_cfg.force_link) {
			p5mcr |= FORCE_MODE | FORCE_LINK | FORCE_RX_FC |
				 FORCE_TX_FC;
			p5mcr |= gsw->port5_cfg.speed << FORCE_SPD_S;

			if (gsw->port5_cfg.duplex)
				p5mcr |= FORCE_DPX;
		}

		switch (gsw->port5_cfg.phy_mode) {
		case PHY_INTERFACE_MODE_MII:
		case PHY_INTERFACE_MODE_GMII:
			break;
		case PHY_INTERFACE_MODE_RGMII:
			hwstrap |= P5_INTF_MODE_RGMII;
			break;
		default:
			dev_info(gsw->dev, "%s is not supported by port5\n",
				 phy_modes(gsw->port5_cfg.phy_mode));
			p5mcr = FORCE_MODE;
			hwstrap |= P5_INTF_DIS_S;
		}

		/* Port5 to PHY direct mode */
		if (of_property_read_u32(gsw->port5_cfg.np, "phy-address",
					 &phyad))
			goto parse_p6;

		if (phyad != 0 && phyad != 4) {
			dev_info(gsw->dev,
				 "Only PHY 0/4 can be connected to Port 5\n");
			goto parse_p6;
		}

		hwstrap &= ~P5_INTF_SEL_GMAC5;
		if (phyad == 0)
			hwstrap |= P5_PHY0_SEL;
		else
			hwstrap &= ~P5_PHY0_SEL;
	}

parse_p6:
	if (!gsw->port6_cfg.enabled) {
		p6mcr = FORCE_MODE;
		hwstrap |= P6_INTF_DIS;
	} else {
		p6mcr = (IPG_96BIT_WITH_SHORT_IPG << IPG_CFG_S) |
			MAC_MODE | MAC_TX_EN | MAC_RX_EN |
			BKOFF_EN | BACKPR_EN;

		if (gsw->port6_cfg.force_link) {
			p6mcr |= FORCE_MODE | FORCE_LINK | FORCE_RX_FC |
				 FORCE_TX_FC;
			p6mcr |= gsw->port6_cfg.speed << FORCE_SPD_S;

			if (gsw->port6_cfg.duplex)
				p6mcr |= FORCE_DPX;
		}

		switch (gsw->port6_cfg.phy_mode) {
		case PHY_INTERFACE_MODE_RGMII:
			p6ecr = BIT(1);
			break;
		case PHY_INTERFACE_MODE_TRGMII:
			/* set MT7530 central align */
			p6ecr = BIT(0);
			break;
		default:
			dev_info(gsw->dev, "%s is not supported by port6\n",
				 phy_modes(gsw->port6_cfg.phy_mode));
			p6mcr = FORCE_MODE;
			hwstrap |= P6_INTF_DIS;
		}
	}

	mt753x_reg_write(gsw, MHWSTRAP, hwstrap);
	mt753x_reg_write(gsw, P6ECR, p6ecr);

	mt753x_reg_write(gsw, PMCR(5), p5mcr);
	mt753x_reg_write(gsw, PMCR(6), p6mcr);

	return 0;
}

static void mt7530_core_pll_setup(struct gsw_mt753x *gsw)
{
	u32 hwstrap;

	hwstrap = mt753x_reg_read(gsw, HWSTRAP);

	switch ((hwstrap & XTAL_FSEL_M) >> XTAL_FSEL_S) {
	case XTAL_40MHZ:
		/* Disable MT7530 core clock */
		mt7530_core_reg_write(gsw, TRGMII_GSW_CLK_CG, 0);

		/* disable MT7530 PLL */
		mt7530_core_reg_write(gsw, CORE_GSWPLL_GCR_1,
				      (2 << GSWPLL_POSTDIV_200M_S) |
				      (32 << GSWPLL_FBKDIV_200M_S));

		/* For MT7530 core clock = 500Mhz */
		mt7530_core_reg_write(gsw, CORE_GSWPLL_GCR_2,
				      (1 << GSWPLL_POSTDIV_500M_S) |
				      (25 << GSWPLL_FBKDIV_500M_S));

		/* Enable MT7530 PLL */
		mt7530_core_reg_write(gsw, CORE_GSWPLL_GCR_1,
				      (2 << GSWPLL_POSTDIV_200M_S) |
				      (32 << GSWPLL_FBKDIV_200M_S) |
				      GSWPLL_EN_PRE);

		usleep_range(20, 40);

		/* Enable MT7530 core clock */
		mt7530_core_reg_write(gsw, TRGMII_GSW_CLK_CG, GSWCK_EN);
		break;
	default:
		/* TODO: PLL settings for 20/25MHz */
		break;
	}

	hwstrap = mt753x_reg_read(gsw, HWSTRAP);
	hwstrap |= CHG_TRAP;
	if (gsw->direct_phy_access)
		hwstrap &= ~C_MDIO_BPS_S;
	else
		hwstrap |= C_MDIO_BPS_S;

	mt753x_reg_write(gsw, MHWSTRAP, hwstrap);

	if (gsw->port6_cfg.enabled &&
	    gsw->port6_cfg.phy_mode == PHY_INTERFACE_MODE_TRGMII) {
		mt7530_trgmii_setting(gsw);
	} else {
		/* RGMII */
		mt7530_rgmii_setting(gsw);
	}

	/* delay setting for 10/1000M */
	mt753x_reg_write(gsw, P5RGMIIRXCR,
			 CSR_RGMII_EDGE_ALIGN |
			 (2 << CSR_RGMII_RXC_0DEG_CFG_S));
	mt753x_reg_write(gsw, P5RGMIITXCR, 0x14 << CSR_RGMII_TXC_CFG_S);
}

static int mt7530_sw_detect(struct gsw_mt753x *gsw, struct chip_rev *crev)
{
	u32 rev;

	rev = mt753x_reg_read(gsw, CHIP_REV);

	if (((rev & CHIP_NAME_M) >> CHIP_NAME_S) == MT7530) {
		if (crev) {
			crev->rev = rev & CHIP_REV_M;
			crev->name = "MT7530";
		}

		return 0;
	}

	return -ENODEV;
}

static void mt7530_phy_setting(struct gsw_mt753x *gsw)
{
	int i;
	u32 val;

	for (i = 0; i < MT753X_NUM_PHYS; i++) {
		/* Disable EEE */
		gsw->mmd_write(gsw, i, PHY_DEV07, PHY_DEV07_REG_03C, 0);

		/* Enable HW auto downshift */
		gsw->mii_write(gsw, i, 0x1f, 0x1);
		val = gsw->mii_read(gsw, i, PHY_EXT_REG_14);
		val |= PHY_EN_DOWN_SHFIT;
		gsw->mii_write(gsw, i, PHY_EXT_REG_14, val);

		/* Increase SlvDPSready time */
		gsw->mii_write(gsw, i, 0x1f, 0x52b5);
		gsw->mii_write(gsw, i, PHY_TR_REG_10, 0xafae);
		gsw->mii_write(gsw, i, PHY_TR_REG_12, 0x2f);
		gsw->mii_write(gsw, i, PHY_TR_REG_10, 0x8fae);

		/* Increase post_update_timer */
		gsw->mii_write(gsw, i, 0x1f, 0x3);
		gsw->mii_write(gsw, i, PHY_LPI_REG_11, 0x4b);
		gsw->mii_write(gsw, i, 0x1f, 0);

		/* Adjust 100_mse_threshold */
		gsw->mmd_write(gsw, i, PHY_DEV1E, PHY_DEV1E_REG_123, 0xffff);

		/* Disable mcc */
		gsw->mmd_write(gsw, i, PHY_DEV1E, PHY_DEV1E_REG_A6, 0x300);
	}
}

static inline bool get_phy_access_mode(const struct device_node *np)
{
	return of_property_read_bool(np, "mt7530,direct-phy-access");
}

static int mt7530_sw_init(struct gsw_mt753x *gsw)
{
	int i;
	u32 val;

	gsw->direct_phy_access = get_phy_access_mode(gsw->dev->of_node);

	/* Force MT7530 to use (in)direct PHY access */
	val = mt753x_reg_read(gsw, HWSTRAP);
	val |= CHG_TRAP;
	if (gsw->direct_phy_access)
		val &= ~C_MDIO_BPS_S;
	else
		val |= C_MDIO_BPS_S;
	mt753x_reg_write(gsw, MHWSTRAP, val);

	/* Read PHY address base from HWSTRAP */
	gsw->phy_base  = (((val & SMI_ADDR_M) >> SMI_ADDR_S) << 3) + 8;
	gsw->phy_base &= MT753X_SMI_ADDR_MASK;

	if (gsw->direct_phy_access) {
		gsw->mii_read = mt7530_mii_read;
		gsw->mii_write = mt7530_mii_write;
		gsw->mmd_read = mt7530_mmd_read;
		gsw->mmd_write = mt7530_mmd_write;
	} else {
		gsw->mii_read = mt753x_mii_read;
		gsw->mii_write = mt753x_mii_write;
		gsw->mmd_read = mt753x_mmd_ind_read;
		gsw->mmd_write = mt753x_mmd_ind_write;
	}

	for (i = 0; i < MT753X_NUM_PHYS; i++) {
		val = gsw->mii_read(gsw, i, MII_BMCR);
		val |= BMCR_PDOWN;
		gsw->mii_write(gsw, i, MII_BMCR, val);
	}

	/* Force MAC link down before reset */
	mt753x_reg_write(gsw, PMCR(5), FORCE_MODE);
	mt753x_reg_write(gsw, PMCR(6), FORCE_MODE);

	/* Switch soft reset */
	/* BUG: sw reset causes gsw int flooding */
	mt753x_reg_write(gsw, SYS_CTRL, SW_PHY_RST | SW_SYS_RST | SW_REG_RST);
	usleep_range(10, 20);

	/* global mac control settings configuration */
	mt753x_reg_write(gsw, GMACCR,
			 LATE_COL_DROP | (15 << MTCC_LMT_S) |
			 (2 << MAX_RX_JUMBO_S) | RX_PKT_LEN_MAX_JUMBO);

	mt7530_core_pll_setup(gsw);
	mt7530_mac_port_setup(gsw);

	return 0;
}

static int mt7530_sw_post_init(struct gsw_mt753x *gsw)
{
	int i;
	u32 val;

	mt7530_phy_setting(gsw);

	for (i = 0; i < MT753X_NUM_PHYS; i++) {
		val = gsw->mii_read(gsw, i, MII_BMCR);
		val &= ~BMCR_PDOWN;
		gsw->mii_write(gsw, i, MII_BMCR, val);
	}

	return 0;
}

struct mt753x_sw_id mt7530_id = {
	.model = MT7530,
	.detect = mt7530_sw_detect,
	.init = mt7530_sw_init,
	.post_init = mt7530_sw_post_init
};

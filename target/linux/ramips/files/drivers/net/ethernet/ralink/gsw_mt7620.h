/*   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; version 2 of the License
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   Copyright (C) 2009-2015 John Crispin <blogic@openwrt.org>
 *   Copyright (C) 2009-2015 Felix Fietkau <nbd@nbd.name>
 *   Copyright (C) 2013-2015 Michael Lee <igvtee@gmail.com>
 */

#ifndef _RALINK_GSW_MT7620_H__
#define _RALINK_GSW_MT7620_H__

#define GSW_REG_PHY_TIMEOUT	(5 * HZ)

#ifdef CONFIG_SOC_MT7621
#define MT7620A_GSW_REG_PIAC	0x0004
#else
#define MT7620A_GSW_REG_PIAC	0x7004
#endif

#define GSW_NUM_VLANS		16
#define GSW_NUM_VIDS		4096
#define GSW_NUM_PORTS		7
#define GSW_PORT6		6

#define GSW_MDIO_ACCESS		BIT(31)
#define GSW_MDIO_READ		BIT(19)
#define GSW_MDIO_WRITE		BIT(18)
#define GSW_MDIO_START		BIT(16)
#define GSW_MDIO_ADDR_SHIFT	20
#define GSW_MDIO_REG_SHIFT	25

#define GSW_REG_MIB_CNT_EN	0x4000

#define GSW_REG_PORT_PMCR(x)	(0x3000 + (x * 0x100))
#define GSW_REG_PORT_STATUS(x)	(0x3008 + (x * 0x100))
#define GSW_REG_SMACCR0		0x3fE4
#define GSW_REG_SMACCR1		0x3fE8
#define GSW_REG_CKGCR		0x3ff0

#define GSW_REG_IMR		0x7008
#define GSW_REG_ISR		0x700c
#define GSW_REG_GPC1		0x7014
#define GSW_REG_GPC2		0x701c

#define GSW_REG_GPCx_TXDELAY	BIT(3)
#define GSW_REG_GPCx_RXDELAY	BIT(2)

#define GSW_REG_MAC_P0_MCR	0x100
#define GSW_REG_MAC_P1_MCR	0x200

// Global MAC control register
#define GSW_REG_GMACCR		0x30E0

#define SYSC_REG_CHIP_REV_ID	0x0c
#define SYSC_REG_CFG1		0x14
#define RST_CTRL_MCM		BIT(2)
#define SYSC_PAD_RGMII2_MDIO	0x58
#define SYSC_GPIO_MODE		0x60

#define PORT_IRQ_ST_CHG		0x7f

#ifdef CONFIG_SOC_MT7621
#define ESW_PHY_POLLING		0x0000
#else
#define ESW_PHY_POLLING		0x7000
#endif

#define	PMCR_IPG		BIT(18)
#define	PMCR_MAC_MODE		BIT(16)
#define	PMCR_FORCE		BIT(15)
#define	PMCR_TX_EN		BIT(14)
#define	PMCR_RX_EN		BIT(13)
#define	PMCR_BACKOFF		BIT(9)
#define	PMCR_BACKPRES		BIT(8)
#define	PMCR_RX_FC		BIT(5)
#define	PMCR_TX_FC		BIT(4)
#define	PMCR_SPEED(_x)		(_x << 2)
#define	PMCR_DUPLEX		BIT(1)
#define	PMCR_LINK		BIT(0)

#define PHY_AN_EN		BIT(31)
#define PHY_PRE_EN		BIT(30)
#define PMY_MDC_CONF(_x)	((_x & 0x3f) << 24)


enum {
	/* Global attributes. */
	GSW_ATTR_ENABLE_VLAN,
	/* Port attributes. */
	GSW_ATTR_PORT_UNTAG,
};

enum {
	PORT4_EPHY = 0,
	PORT4_EXT,
};

struct mt7620_gsw {
	struct device		*dev;
	void __iomem		*base;
	int			irq;
	int			port4;
	unsigned long int	autopoll;
	u16			ephy_base;
};

void mtk_switch_w32(struct mt7620_gsw *gsw, u32 val, unsigned reg);
u32 mtk_switch_r32(struct mt7620_gsw *gsw, unsigned reg);
int mtk_gsw_init(struct fe_priv *priv);

int mt7620_mdio_write(struct mii_bus *bus, int phy_addr, int phy_reg, u16 val);
int mt7620_mdio_read(struct mii_bus *bus, int phy_addr, int phy_reg);
void mt7620_mdio_link_adjust(struct fe_priv *priv, int port);
int mt7620_has_carrier(struct fe_priv *priv);
void mt7620_print_link_state(struct fe_priv *priv, int port, int link,
			     int speed, int duplex);

void mt7530_mdio_w32(struct mt7620_gsw *gsw, u32 reg, u32 val);
u32 mt7530_mdio_r32(struct mt7620_gsw *gsw, u32 reg);

u32 _mt7620_mii_write(struct mt7620_gsw *gsw, u32 phy_addr,
			     u32 phy_register, u32 write_data);
u32 _mt7620_mii_read(struct mt7620_gsw *gsw, int phy_addr, int phy_reg);
void mt7620_handle_carrier(struct fe_priv *priv);

#endif

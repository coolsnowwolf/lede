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

#ifndef FE_ETH_H
#define FE_ETH_H

#include <linux/mii.h>
#include <linux/interrupt.h>
#include <linux/netdevice.h>
#include <linux/dma-mapping.h>
#include <linux/phy.h>
#include <linux/ethtool.h>
#include <linux/version.h>

enum fe_reg {
	FE_REG_PDMA_GLO_CFG = 0,
	FE_REG_PDMA_RST_CFG,
	FE_REG_DLY_INT_CFG,
	FE_REG_TX_BASE_PTR0,
	FE_REG_TX_MAX_CNT0,
	FE_REG_TX_CTX_IDX0,
	FE_REG_TX_DTX_IDX0,
	FE_REG_RX_BASE_PTR0,
	FE_REG_RX_MAX_CNT0,
	FE_REG_RX_CALC_IDX0,
	FE_REG_RX_DRX_IDX0,
	FE_REG_FE_INT_ENABLE,
	FE_REG_FE_INT_STATUS,
	FE_REG_FE_DMA_VID_BASE,
	FE_REG_FE_COUNTER_BASE,
	FE_REG_FE_RST_GL,
	FE_REG_FE_INT_STATUS2,
	FE_REG_COUNT
};

enum fe_work_flag {
	FE_FLAG_RESET_PENDING,
	FE_FLAG_MAX
};

#define MTK_FE_DRV_VERSION		"0.2"

/* power of 2 to let NEXT_TX_DESP_IDX work */
#define NUM_DMA_DESC		BIT(10)
#define MAX_DMA_DESC		0xfff

#define FE_DELAY_EN_INT		0x80
#define FE_DELAY_MAX_INT	0x04
#define FE_DELAY_MAX_TOUT	0x04
#define FE_DELAY_TIME		20
#define FE_DELAY_CHAN		(((FE_DELAY_EN_INT | FE_DELAY_MAX_INT) << 8) | \
				 FE_DELAY_MAX_TOUT)
#define FE_DELAY_INIT		((FE_DELAY_CHAN << 16) | FE_DELAY_CHAN)
#define FE_PSE_FQFC_CFG_INIT	0x80504000
#define FE_PSE_FQFC_CFG_256Q	0xff908000

/* interrupt bits */
#define FE_CNT_PPE_AF		BIT(31)
#define FE_CNT_GDM_AF		BIT(29)
#define FE_PSE_P2_FC		BIT(26)
#define FE_PSE_BUF_DROP		BIT(24)
#define FE_GDM_OTHER_DROP	BIT(23)
#define FE_PSE_P1_FC		BIT(22)
#define FE_PSE_P0_FC		BIT(21)
#define FE_PSE_FQ_EMPTY		BIT(20)
#define FE_GE1_STA_CHG		BIT(18)
#define FE_TX_COHERENT		BIT(17)
#define FE_RX_COHERENT		BIT(16)
#define FE_TX_DONE_INT3		BIT(11)
#define FE_TX_DONE_INT2		BIT(10)
#define FE_TX_DONE_INT1		BIT(9)
#define FE_TX_DONE_INT0		BIT(8)
#define FE_RX_DONE_INT0		BIT(2)
#define FE_TX_DLY_INT		BIT(1)
#define FE_RX_DLY_INT		BIT(0)

#define FE_RX_DONE_INT		FE_RX_DONE_INT0
#define FE_TX_DONE_INT		(FE_TX_DONE_INT0 | FE_TX_DONE_INT1 | \
				 FE_TX_DONE_INT2 | FE_TX_DONE_INT3)

#define RT5350_RX_DLY_INT	BIT(30)
#define RT5350_TX_DLY_INT	BIT(28)
#define RT5350_RX_DONE_INT1	BIT(17)
#define RT5350_RX_DONE_INT0	BIT(16)
#define RT5350_TX_DONE_INT3	BIT(3)
#define RT5350_TX_DONE_INT2	BIT(2)
#define RT5350_TX_DONE_INT1	BIT(1)
#define RT5350_TX_DONE_INT0	BIT(0)

#define RT5350_RX_DONE_INT	(RT5350_RX_DONE_INT0 | RT5350_RX_DONE_INT1)
#define RT5350_TX_DONE_INT	(RT5350_TX_DONE_INT0 | RT5350_TX_DONE_INT1 | \
				 RT5350_TX_DONE_INT2 | RT5350_TX_DONE_INT3)

/* registers */
#define FE_FE_OFFSET		0x0000
#define FE_GDMA_OFFSET		0x0020
#define FE_PSE_OFFSET		0x0040
#define FE_GDMA2_OFFSET		0x0060
#define FE_CDMA_OFFSET		0x0080
#define FE_DMA_VID0		0x00a8
#define FE_PDMA_OFFSET		0x0100
#define FE_PPE_OFFSET		0x0200
#define FE_CMTABLE_OFFSET	0x0400
#define FE_POLICYTABLE_OFFSET	0x1000

#define RT5350_PDMA_OFFSET	0x0800
#define RT5350_SDM_OFFSET	0x0c00

#define FE_MDIO_ACCESS		(FE_FE_OFFSET + 0x00)
#define FE_MDIO_CFG		(FE_FE_OFFSET + 0x04)
#define FE_FE_GLO_CFG		(FE_FE_OFFSET + 0x08)
#define FE_FE_RST_GL		(FE_FE_OFFSET + 0x0C)
#define FE_FE_INT_STATUS	(FE_FE_OFFSET + 0x10)
#define FE_FE_INT_ENABLE	(FE_FE_OFFSET + 0x14)
#define FE_MDIO_CFG2		(FE_FE_OFFSET + 0x18)
#define FE_FOC_TS_T		(FE_FE_OFFSET + 0x1C)

#define	FE_GDMA1_FWD_CFG	(FE_GDMA_OFFSET + 0x00)
#define FE_GDMA1_SCH_CFG	(FE_GDMA_OFFSET + 0x04)
#define FE_GDMA1_SHPR_CFG	(FE_GDMA_OFFSET + 0x08)
#define FE_GDMA1_MAC_ADRL	(FE_GDMA_OFFSET + 0x0C)
#define FE_GDMA1_MAC_ADRH	(FE_GDMA_OFFSET + 0x10)

#define	FE_GDMA2_FWD_CFG	(FE_GDMA2_OFFSET + 0x00)
#define FE_GDMA2_SCH_CFG	(FE_GDMA2_OFFSET + 0x04)
#define FE_GDMA2_SHPR_CFG	(FE_GDMA2_OFFSET + 0x08)
#define FE_GDMA2_MAC_ADRL	(FE_GDMA2_OFFSET + 0x0C)
#define FE_GDMA2_MAC_ADRH	(FE_GDMA2_OFFSET + 0x10)

#define FE_PSE_FQ_CFG		(FE_PSE_OFFSET + 0x00)
#define FE_CDMA_FC_CFG		(FE_PSE_OFFSET + 0x04)
#define FE_GDMA1_FC_CFG		(FE_PSE_OFFSET + 0x08)
#define FE_GDMA2_FC_CFG		(FE_PSE_OFFSET + 0x0C)

#define FE_CDMA_CSG_CFG		(FE_CDMA_OFFSET + 0x00)
#define FE_CDMA_SCH_CFG		(FE_CDMA_OFFSET + 0x04)

#ifdef CONFIG_SOC_MT7621
#define MT7620A_GDMA_OFFSET		0x0500
#else
#define MT7620A_GDMA_OFFSET		0x0600
#endif
#define	MT7620A_GDMA1_FWD_CFG		(MT7620A_GDMA_OFFSET + 0x00)
#define MT7620A_FE_GDMA1_SCH_CFG	(MT7620A_GDMA_OFFSET + 0x04)
#define MT7620A_FE_GDMA1_SHPR_CFG	(MT7620A_GDMA_OFFSET + 0x08)
#define MT7620A_FE_GDMA1_MAC_ADRL	(MT7620A_GDMA_OFFSET + 0x0C)
#define MT7620A_FE_GDMA1_MAC_ADRH	(MT7620A_GDMA_OFFSET + 0x10)

#define MT7620A_RESET_FE	BIT(21)
#define MT7620A_RESET_ESW	BIT(23)
#define MT7620A_RESET_EPHY	BIT(24)

#define RT5350_TX_BASE_PTR0	(RT5350_PDMA_OFFSET + 0x00)
#define RT5350_TX_MAX_CNT0	(RT5350_PDMA_OFFSET + 0x04)
#define RT5350_TX_CTX_IDX0	(RT5350_PDMA_OFFSET + 0x08)
#define RT5350_TX_DTX_IDX0	(RT5350_PDMA_OFFSET + 0x0C)
#define RT5350_TX_BASE_PTR1	(RT5350_PDMA_OFFSET + 0x10)
#define RT5350_TX_MAX_CNT1	(RT5350_PDMA_OFFSET + 0x14)
#define RT5350_TX_CTX_IDX1	(RT5350_PDMA_OFFSET + 0x18)
#define RT5350_TX_DTX_IDX1	(RT5350_PDMA_OFFSET + 0x1C)
#define RT5350_TX_BASE_PTR2	(RT5350_PDMA_OFFSET + 0x20)
#define RT5350_TX_MAX_CNT2	(RT5350_PDMA_OFFSET + 0x24)
#define RT5350_TX_CTX_IDX2	(RT5350_PDMA_OFFSET + 0x28)
#define RT5350_TX_DTX_IDX2	(RT5350_PDMA_OFFSET + 0x2C)
#define RT5350_TX_BASE_PTR3	(RT5350_PDMA_OFFSET + 0x30)
#define RT5350_TX_MAX_CNT3	(RT5350_PDMA_OFFSET + 0x34)
#define RT5350_TX_CTX_IDX3	(RT5350_PDMA_OFFSET + 0x38)
#define RT5350_TX_DTX_IDX3	(RT5350_PDMA_OFFSET + 0x3C)
#define RT5350_RX_BASE_PTR0	(RT5350_PDMA_OFFSET + 0x100)
#define RT5350_RX_MAX_CNT0	(RT5350_PDMA_OFFSET + 0x104)
#define RT5350_RX_CALC_IDX0	(RT5350_PDMA_OFFSET + 0x108)
#define RT5350_RX_DRX_IDX0	(RT5350_PDMA_OFFSET + 0x10C)
#define RT5350_RX_BASE_PTR1	(RT5350_PDMA_OFFSET + 0x110)
#define RT5350_RX_MAX_CNT1	(RT5350_PDMA_OFFSET + 0x114)
#define RT5350_RX_CALC_IDX1	(RT5350_PDMA_OFFSET + 0x118)
#define RT5350_RX_DRX_IDX1	(RT5350_PDMA_OFFSET + 0x11C)
#define RT5350_PDMA_GLO_CFG	(RT5350_PDMA_OFFSET + 0x204)
#define RT5350_PDMA_RST_CFG	(RT5350_PDMA_OFFSET + 0x208)
#define RT5350_DLY_INT_CFG	(RT5350_PDMA_OFFSET + 0x20c)
#define RT5350_FE_INT_STATUS	(RT5350_PDMA_OFFSET + 0x220)
#define RT5350_FE_INT_ENABLE	(RT5350_PDMA_OFFSET + 0x228)
#define RT5350_PDMA_SCH_CFG	(RT5350_PDMA_OFFSET + 0x280)

#define FE_PDMA_GLO_CFG		(FE_PDMA_OFFSET + 0x00)
#define FE_PDMA_RST_CFG		(FE_PDMA_OFFSET + 0x04)
#define FE_PDMA_SCH_CFG		(FE_PDMA_OFFSET + 0x08)
#define FE_DLY_INT_CFG		(FE_PDMA_OFFSET + 0x0C)
#define FE_TX_BASE_PTR0		(FE_PDMA_OFFSET + 0x10)
#define FE_TX_MAX_CNT0		(FE_PDMA_OFFSET + 0x14)
#define FE_TX_CTX_IDX0		(FE_PDMA_OFFSET + 0x18)
#define FE_TX_DTX_IDX0		(FE_PDMA_OFFSET + 0x1C)
#define FE_TX_BASE_PTR1		(FE_PDMA_OFFSET + 0x20)
#define FE_TX_MAX_CNT1		(FE_PDMA_OFFSET + 0x24)
#define FE_TX_CTX_IDX1		(FE_PDMA_OFFSET + 0x28)
#define FE_TX_DTX_IDX1		(FE_PDMA_OFFSET + 0x2C)
#define FE_RX_BASE_PTR0		(FE_PDMA_OFFSET + 0x30)
#define FE_RX_MAX_CNT0		(FE_PDMA_OFFSET + 0x34)
#define FE_RX_CALC_IDX0		(FE_PDMA_OFFSET + 0x38)
#define FE_RX_DRX_IDX0		(FE_PDMA_OFFSET + 0x3C)
#define FE_TX_BASE_PTR2		(FE_PDMA_OFFSET + 0x40)
#define FE_TX_MAX_CNT2		(FE_PDMA_OFFSET + 0x44)
#define FE_TX_CTX_IDX2		(FE_PDMA_OFFSET + 0x48)
#define FE_TX_DTX_IDX2		(FE_PDMA_OFFSET + 0x4C)
#define FE_TX_BASE_PTR3		(FE_PDMA_OFFSET + 0x50)
#define FE_TX_MAX_CNT3		(FE_PDMA_OFFSET + 0x54)
#define FE_TX_CTX_IDX3		(FE_PDMA_OFFSET + 0x58)
#define FE_TX_DTX_IDX3		(FE_PDMA_OFFSET + 0x5C)
#define FE_RX_BASE_PTR1		(FE_PDMA_OFFSET + 0x60)
#define FE_RX_MAX_CNT1		(FE_PDMA_OFFSET + 0x64)
#define FE_RX_CALC_IDX1		(FE_PDMA_OFFSET + 0x68)
#define FE_RX_DRX_IDX1		(FE_PDMA_OFFSET + 0x6C)

/* Switch DMA configuration */
#define RT5350_SDM_CFG		(RT5350_SDM_OFFSET + 0x00)
#define RT5350_SDM_RRING	(RT5350_SDM_OFFSET + 0x04)
#define RT5350_SDM_TRING	(RT5350_SDM_OFFSET + 0x08)
#define RT5350_SDM_MAC_ADRL	(RT5350_SDM_OFFSET + 0x0C)
#define RT5350_SDM_MAC_ADRH	(RT5350_SDM_OFFSET + 0x10)
#define RT5350_SDM_TPCNT	(RT5350_SDM_OFFSET + 0x100)
#define RT5350_SDM_TBCNT	(RT5350_SDM_OFFSET + 0x104)
#define RT5350_SDM_RPCNT	(RT5350_SDM_OFFSET + 0x108)
#define RT5350_SDM_RBCNT	(RT5350_SDM_OFFSET + 0x10C)
#define RT5350_SDM_CS_ERR	(RT5350_SDM_OFFSET + 0x110)

#define RT5350_SDM_ICS_EN	BIT(16)
#define RT5350_SDM_TCS_EN	BIT(17)
#define RT5350_SDM_UCS_EN	BIT(18)

/* MDIO_CFG register bits */
#define FE_MDIO_CFG_AUTO_POLL_EN	BIT(29)
#define FE_MDIO_CFG_GP1_BP_EN		BIT(16)
#define FE_MDIO_CFG_GP1_FRC_EN		BIT(15)
#define FE_MDIO_CFG_GP1_SPEED_10	(0 << 13)
#define FE_MDIO_CFG_GP1_SPEED_100	(1 << 13)
#define FE_MDIO_CFG_GP1_SPEED_1000	(2 << 13)
#define FE_MDIO_CFG_GP1_DUPLEX		BIT(12)
#define FE_MDIO_CFG_GP1_FC_TX		BIT(11)
#define FE_MDIO_CFG_GP1_FC_RX		BIT(10)
#define FE_MDIO_CFG_GP1_LNK_DWN		BIT(9)
#define FE_MDIO_CFG_GP1_AN_FAIL		BIT(8)
#define FE_MDIO_CFG_MDC_CLK_DIV_1	(0 << 6)
#define FE_MDIO_CFG_MDC_CLK_DIV_2	(1 << 6)
#define FE_MDIO_CFG_MDC_CLK_DIV_4	(2 << 6)
#define FE_MDIO_CFG_MDC_CLK_DIV_8	(3 << 6)
#define FE_MDIO_CFG_TURBO_MII_FREQ	BIT(5)
#define FE_MDIO_CFG_TURBO_MII_MODE	BIT(4)
#define FE_MDIO_CFG_RX_CLK_SKEW_0	(0 << 2)
#define FE_MDIO_CFG_RX_CLK_SKEW_200	(1 << 2)
#define FE_MDIO_CFG_RX_CLK_SKEW_400	(2 << 2)
#define FE_MDIO_CFG_RX_CLK_SKEW_INV	(3 << 2)
#define FE_MDIO_CFG_TX_CLK_SKEW_0	0
#define FE_MDIO_CFG_TX_CLK_SKEW_200	1
#define FE_MDIO_CFG_TX_CLK_SKEW_400	2
#define FE_MDIO_CFG_TX_CLK_SKEW_INV	3

/* uni-cast port */
#define FE_GDM1_JMB_LEN_MASK	0xf
#define FE_GDM1_JMB_LEN_SHIFT	28
#define FE_GDM1_ICS_EN		BIT(22)
#define FE_GDM1_TCS_EN		BIT(21)
#define FE_GDM1_UCS_EN		BIT(20)
#define FE_GDM1_JMB_EN		BIT(19)
#define FE_GDM1_STRPCRC		BIT(16)
#define FE_GDM1_UFRC_P_CPU	(0 << 12)
#define FE_GDM1_UFRC_P_GDMA1	(1 << 12)
#define FE_GDM1_UFRC_P_PPE	(6 << 12)

/* checksums */
#define FE_ICS_GEN_EN		BIT(2)
#define FE_UCS_GEN_EN		BIT(1)
#define FE_TCS_GEN_EN		BIT(0)

/* dma ring */
#define FE_PST_DRX_IDX0		BIT(16)
#define FE_PST_DTX_IDX3		BIT(3)
#define FE_PST_DTX_IDX2		BIT(2)
#define FE_PST_DTX_IDX1		BIT(1)
#define FE_PST_DTX_IDX0		BIT(0)

#define FE_RX_2B_OFFSET		BIT(31)
#define FE_TX_WB_DDONE		BIT(6)
#define FE_RX_DMA_BUSY		BIT(3)
#define FE_TX_DMA_BUSY		BIT(1)
#define FE_RX_DMA_EN		BIT(2)
#define FE_TX_DMA_EN		BIT(0)

#define FE_PDMA_SIZE_4DWORDS	(0 << 4)
#define FE_PDMA_SIZE_8DWORDS	(1 << 4)
#define FE_PDMA_SIZE_16DWORDS	(2 << 4)

#define FE_US_CYC_CNT_MASK	0xff
#define FE_US_CYC_CNT_SHIFT	0x8
#define FE_US_CYC_CNT_DIVISOR	1000000

/* rxd2 */
#define RX_DMA_DONE		BIT(31)
#define RX_DMA_LSO		BIT(30)
#define RX_DMA_PLEN0(_x)	(((_x) & 0x3fff) << 16)
#define RX_DMA_GET_PLEN0(_x)	(((_x) >> 16) & 0x3fff)
#define RX_DMA_TAG		BIT(15)
/* rxd3 */
#define RX_DMA_TPID(_x)		(((_x) >> 16) & 0xffff)
#define RX_DMA_VID(_x)		((_x) & 0xffff)
/* rxd4 */
#define RX_DMA_L4VALID		BIT(30)

struct fe_rx_dma {
	unsigned int rxd1;
	unsigned int rxd2;
	unsigned int rxd3;
	unsigned int rxd4;
} __packed __aligned(4);

#define TX_DMA_BUF_LEN		0x3fff
#define TX_DMA_PLEN0_MASK	(TX_DMA_BUF_LEN << 16)
#define TX_DMA_PLEN0(_x)	(((_x) & TX_DMA_BUF_LEN) << 16)
#define TX_DMA_PLEN1(_x)	((_x) & TX_DMA_BUF_LEN)
#define TX_DMA_GET_PLEN0(_x)    (((_x) >> 16) & TX_DMA_BUF_LEN)
#define TX_DMA_GET_PLEN1(_x)    ((_x) & TX_DMA_BUF_LEN)
#define TX_DMA_LS1		BIT(14)
#define TX_DMA_LS0		BIT(30)
#define TX_DMA_DONE		BIT(31)

#define TX_DMA_INS_VLAN_MT7621	BIT(16)
#define TX_DMA_INS_VLAN		BIT(7)
#define TX_DMA_INS_PPPOE	BIT(12)
#define TX_DMA_QN(_x)		((_x) << 16)
#define TX_DMA_PN(_x)		((_x) << 24)
#define TX_DMA_QN_MASK		TX_DMA_QN(0x7)
#define TX_DMA_PN_MASK		TX_DMA_PN(0x7)
#define TX_DMA_UDF		BIT(20)
#define TX_DMA_CHKSUM		(0x7 << 29)
#define TX_DMA_TSO		BIT(28)

/* frame engine counters */
#define FE_PPE_AC_BCNT0		(FE_CMTABLE_OFFSET + 0x00)
#define FE_GDMA1_TX_GBCNT	(FE_CMTABLE_OFFSET + 0x300)
#define FE_GDMA2_TX_GBCNT	(FE_GDMA1_TX_GBCNT + 0x40)

/* phy device flags */
#define FE_PHY_FLAG_PORT	BIT(0)
#define FE_PHY_FLAG_ATTACH	BIT(1)

struct fe_tx_dma {
	unsigned int txd1;
	unsigned int txd2;
	unsigned int txd3;
	unsigned int txd4;
} __packed __aligned(4);

struct fe_priv;

struct fe_phy {
	/* make sure that phy operations are atomic */
	spinlock_t		lock;

	struct phy_device	*phy[8];
	struct device_node	*phy_node[8];
	const __be32		*phy_fixed[8];
	int			duplex[8];
	int			speed[8];
	int			tx_fc[8];
	int			rx_fc[8];
	int (*connect)(struct fe_priv *priv);
	void (*disconnect)(struct fe_priv *priv);
	void (*start)(struct fe_priv *priv);
	void (*stop)(struct fe_priv *priv);
};

struct fe_soc_data {
	const u16 *reg_table;

	void (*init_data)(struct fe_soc_data *data, struct net_device *netdev);
	void (*reset_fe)(struct fe_priv *priv);
	void (*set_mac)(struct fe_priv *priv, unsigned char *mac);
	int (*fwd_config)(struct fe_priv *priv);
	void (*tx_dma)(struct fe_tx_dma *txd);
	int (*switch_init)(struct fe_priv *priv);
	int (*switch_config)(struct fe_priv *priv);
	void (*port_init)(struct fe_priv *priv, struct device_node *port);
	int (*has_carrier)(struct fe_priv *priv);
	int (*mdio_init)(struct fe_priv *priv);
	void (*mdio_cleanup)(struct fe_priv *priv);
	int (*mdio_write)(struct mii_bus *bus, int phy_addr, int phy_reg,
			  u16 val);
	int (*mdio_read)(struct mii_bus *bus, int phy_addr, int phy_reg);
	void (*mdio_adjust_link)(struct fe_priv *priv, int port);

	void *swpriv;
	u32 pdma_glo_cfg;
	u32 rx_int;
	u32 tx_int;
	u32 status_int;
	u32 checksum_bit;
};

#define FE_FLAG_PADDING_64B		BIT(0)
#define FE_FLAG_PADDING_BUG		BIT(1)
#define FE_FLAG_JUMBO_FRAME		BIT(2)
#define FE_FLAG_RX_2B_OFFSET		BIT(3)
#define FE_FLAG_RX_SG_DMA		BIT(4)
#define FE_FLAG_NAPI_WEIGHT		BIT(6)
#define FE_FLAG_CALIBRATE_CLK		BIT(7)
#define FE_FLAG_HAS_SWITCH		BIT(8)

#define FE_STAT_REG_DECLARE		\
	_FE(tx_bytes)			\
	_FE(tx_packets)			\
	_FE(tx_skip)			\
	_FE(tx_collisions)		\
	_FE(rx_bytes)			\
	_FE(rx_packets)			\
	_FE(rx_overflow)		\
	_FE(rx_fcs_errors)		\
	_FE(rx_short_errors)		\
	_FE(rx_long_errors)		\
	_FE(rx_checksum_errors)		\
	_FE(rx_flow_control_packets)

struct fe_hw_stats {
	/* make sure that stats operations are atomic */
	spinlock_t stats_lock;

	struct u64_stats_sync syncp;
#define _FE(x) u64 x;
	FE_STAT_REG_DECLARE
#undef _FE
};

struct fe_tx_buf {
	struct sk_buff *skb;
	DEFINE_DMA_UNMAP_ADDR(dma_addr0);
	DEFINE_DMA_UNMAP_ADDR(dma_addr1);
	u16 dma_len0;
	u16 dma_len1;
};

struct fe_tx_ring {
	struct fe_tx_dma *tx_dma;
	struct fe_tx_buf *tx_buf;
	dma_addr_t tx_phys;
	u16 tx_ring_size;
	u16 tx_free_idx;
	u16 tx_next_idx;
	u16 tx_thresh;
};

struct fe_rx_ring {
	struct page_frag_cache frag_cache;
	struct fe_rx_dma *rx_dma;
	u8 **rx_data;
	dma_addr_t rx_phys;
	u16 rx_ring_size;
	u16 frag_size;
	u16 rx_buf_size;
	u16 rx_calc_idx;
};

struct fe_priv {
	/* make sure that register operations are atomic */
	spinlock_t			page_lock;

	struct fe_soc_data		*soc;
	struct net_device		*netdev;
	struct device_node		*switch_np;
	u32				msg_enable;
	u32				flags;

	struct device			*dev;
	unsigned long			sysclk;

	struct fe_rx_ring		rx_ring;
	struct napi_struct		rx_napi;

	struct fe_tx_ring               tx_ring;

	struct fe_phy			*phy;
	struct mii_bus			*mii_bus;
	struct phy_device		*phy_dev;
	u32				phy_flags;

	int				link[8];

	struct fe_hw_stats		*hw_stats;
	unsigned long			vlan_map;
	struct work_struct		pending_work;
	DECLARE_BITMAP(pending_flags, FE_FLAG_MAX);

	struct reset_control		*rst_ppe;
	struct reset_control		*rst_fe;
	struct mtk_foe_entry		*foe_table;
	dma_addr_t			foe_table_phys;
	struct flow_offload __rcu	**foe_flow_table;
};

extern const struct of_device_id of_fe_match[];

void fe_w32(u32 val, unsigned reg);
void fe_m32(struct fe_priv *priv, u32 clear, u32 set, unsigned reg);
u32 fe_r32(unsigned reg);

int fe_set_clock_cycle(struct fe_priv *priv);
void fe_csum_config(struct fe_priv *priv);
void fe_stats_update(struct fe_priv *priv);
void fe_fwd_config(struct fe_priv *priv);
void fe_reg_w32(u32 val, enum fe_reg reg);
u32 fe_reg_r32(enum fe_reg reg);

void fe_reset(u32 reset_bits);
void fe_reset_fe(struct fe_priv *priv);

static inline void *priv_netdev(struct fe_priv *priv)
{
	return (char *)priv - ALIGN(sizeof(struct net_device), NETDEV_ALIGN);
}

int ra_ppe_probe(struct fe_priv *eth);
void ra_ppe_remove(struct fe_priv *eth);
int mtk_flow_offload(struct fe_priv *eth,
		     enum flow_offload_type type,
		     struct flow_offload *flow,
		     struct flow_offload_hw_path *src,
		     struct flow_offload_hw_path *dest);
int ra_offload_check_rx(struct fe_priv *eth, struct sk_buff *skb, u32 rxd4);


#endif /* FE_ETH_H */

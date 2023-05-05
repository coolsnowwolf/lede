/*
 *   Copyright (C) 2018 MediaTek Inc.
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; version 2 of the License
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   Copyright (C) 2009-2016 John Crispin <blogic@openwrt.org>
 *   Copyright (C) 2009-2016 Felix Fietkau <nbd@openwrt.org>
 *   Copyright (C) 2013-2016 Michael Lee <igvtee@gmail.com>
 */

#ifndef MTK_ETH_DBG_H
#define MTK_ETH_DBG_H

/* Debug Purpose Register */
#define MTK_PSE_FQFC_CFG		0x100
#define MTK_FE_CDM1_FSM			0x220
#define MTK_FE_CDM2_FSM			0x224
#define MTK_FE_CDM3_FSM			0x238
#define MTK_FE_CDM4_FSM			0x298
#define MTK_FE_GDM1_FSM			0x228
#define MTK_FE_GDM2_FSM			0x22C
#define MTK_FE_PSE_FREE			0x240
#define MTK_FE_DROP_FQ			0x244
#define MTK_FE_DROP_FC			0x248
#define MTK_FE_DROP_PPE			0x24C
#define MTK_MAC_FSM(x)			(0x1010C + ((x) * 0x100))
#define MTK_SGMII_FALSE_CARRIER_CNT(x)	(0x10060028 + ((x) * 0x10000))
#define MTK_SGMII_EFUSE			0x11D008C8
#define MTK_WED_RTQM_GLO_CFG		0x15010B00

#if defined(CONFIG_MEDIATEK_NETSYS_V2) || defined(CONFIG_MEDIATEK_NETSYS_V3)
#define MTK_PSE_IQ_STA(x)		(0x180 + (x) * 0x4)
#define MTK_PSE_OQ_STA(x)		(0x1A0 + (x) * 0x4)
#else
#define MTK_PSE_IQ_STA(x)		(0x110 + (x) * 0x4)
#define MTK_PSE_OQ_STA(x)		(0x118 + (x) * 0x4)
#endif

#define MTKETH_MII_READ                  0x89F3
#define MTKETH_MII_WRITE                 0x89F4
#define MTKETH_ESW_REG_READ              0x89F1
#define MTKETH_ESW_REG_WRITE             0x89F2
#define MTKETH_MII_READ_CL45             0x89FC
#define MTKETH_MII_WRITE_CL45            0x89FD
#define REG_ESW_MAX                     0xFC

#define PROCREG_ESW_CNT			"esw_cnt"
#define PROCREG_TXRING			"tx_ring"
#define PROCREG_RXRING			"rx_ring"
#define PROCREG_DIR			"mtketh"
#define PROCREG_DBG_REGS		"dbg_regs"
#define PROCREG_HW_LRO_STATS		"hw_lro_stats"
#define PROCREG_HW_LRO_AUTO_TLB		"hw_lro_auto_tlb"

/* HW LRO flush reason */
#define MTK_HW_LRO_AGG_FLUSH		(1)
#define MTK_HW_LRO_AGE_FLUSH		(2)
#define MTK_HW_LRO_NOT_IN_SEQ_FLUSH	(3)
#define MTK_HW_LRO_TIMESTAMP_FLUSH	(4)
#define MTK_HW_LRO_NON_RULE_FLUSH	(5)

#define SET_PDMA_RXRING_MAX_AGG_CNT(eth, x, y)				\
{									\
	u32 reg_val1 = mtk_r32(eth, MTK_LRO_CTRL_DW2_CFG(x));		\
	u32 reg_val2 = mtk_r32(eth, MTK_LRO_CTRL_DW3_CFG(x));		\
	reg_val1 &= ~MTK_LRO_RING_AGG_CNT_L_MASK;			\
	reg_val2 &= ~MTK_LRO_RING_AGG_CNT_H_MASK;			\
	reg_val1 |= ((y) & 0x3f) << MTK_LRO_RING_AGG_CNT_L_OFFSET;	\
	reg_val2 |= (((y) >> 6) & 0x03) <<				\
		     MTK_LRO_RING_AGG_CNT_H_OFFSET;			\
	mtk_w32(eth, reg_val1, MTK_LRO_CTRL_DW2_CFG(x));		\
	mtk_w32(eth, reg_val2, MTK_LRO_CTRL_DW3_CFG(x));		\
}

#define SET_PDMA_RXRING_AGG_TIME(eth, x, y)				\
{									\
	u32 reg_val = mtk_r32(eth, MTK_LRO_CTRL_DW2_CFG(x));		\
	reg_val &= ~MTK_LRO_RING_AGG_TIME_MASK;				\
	reg_val |= ((y) & 0xffff) << MTK_LRO_RING_AGG_TIME_OFFSET;	\
	mtk_w32(eth, reg_val, MTK_LRO_CTRL_DW2_CFG(x));			\
}

#define SET_PDMA_RXRING_AGE_TIME(eth, x, y)				\
{									\
	u32 reg_val1 = mtk_r32(eth, MTK_LRO_CTRL_DW1_CFG(x));		\
	u32 reg_val2 = mtk_r32(eth, MTK_LRO_CTRL_DW2_CFG(x));		\
	reg_val1 &= ~MTK_LRO_RING_AGE_TIME_L_MASK;			\
	reg_val2 &= ~MTK_LRO_RING_AGE_TIME_H_MASK;			\
	reg_val1 |= ((y) & 0x3ff) << MTK_LRO_RING_AGE_TIME_L_OFFSET;	\
	reg_val2 |= (((y) >> 10) & 0x03f) <<				\
		     MTK_LRO_RING_AGE_TIME_H_OFFSET;			\
	mtk_w32(eth, reg_val1, MTK_LRO_CTRL_DW1_CFG(x));		\
	mtk_w32(eth, reg_val2, MTK_LRO_CTRL_DW2_CFG(x));		\
}

#define SET_PDMA_LRO_BW_THRESHOLD(eth, x)				\
{									\
	u32 reg_val = mtk_r32(eth, MTK_PDMA_LRO_CTRL_DW2);		\
	reg_val = (x);							\
	mtk_w32(eth, reg_val, MTK_PDMA_LRO_CTRL_DW2);			\
}

#define SET_PDMA_RXRING_VALID(eth, x, y)				\
{									\
	u32 reg_val = mtk_r32(eth, MTK_LRO_CTRL_DW2_CFG(x));		\
	reg_val &= ~(0x1 << MTK_RX_PORT_VALID_OFFSET);			\
	reg_val |= ((y) & 0x1) << MTK_RX_PORT_VALID_OFFSET;		\
	mtk_w32(eth, reg_val, MTK_LRO_CTRL_DW2_CFG(x));			\
}

struct mtk_lro_alt_v1_info0 {
	u32 dtp : 16;
	u32 stp : 16;
};

struct mtk_lro_alt_v1_info1 {
	u32 sip0 : 32;
};

struct mtk_lro_alt_v1_info2 {
	u32 sip1 : 32;
};

struct mtk_lro_alt_v1_info3 {
	u32 sip2 : 32;
};

struct mtk_lro_alt_v1_info4 {
	u32 sip3 : 32;
};

struct mtk_lro_alt_v1_info5 {
	u32 vlan_vid0 : 32;
};

struct mtk_lro_alt_v1_info6 {
	u32 vlan_vid1 : 16;
	u32 vlan_vid_vld : 4;
	u32 cnt : 12;
};

struct mtk_lro_alt_v1_info7 {
	u32 dw_len : 32;
};

struct mtk_lro_alt_v1_info8 {
	u32 dip_id : 2;
	u32 ipv6 : 1;
	u32 ipv4 : 1;
	u32 resv : 27;
	u32 valid : 1;
};

struct mtk_lro_alt_v1 {
	struct mtk_lro_alt_v1_info0 alt_info0;
	struct mtk_lro_alt_v1_info1 alt_info1;
	struct mtk_lro_alt_v1_info2 alt_info2;
	struct mtk_lro_alt_v1_info3 alt_info3;
	struct mtk_lro_alt_v1_info4 alt_info4;
	struct mtk_lro_alt_v1_info5 alt_info5;
	struct mtk_lro_alt_v1_info6 alt_info6;
	struct mtk_lro_alt_v1_info7 alt_info7;
	struct mtk_lro_alt_v1_info8 alt_info8;
};

struct mtk_lro_alt_v2_info0 {
	u32 v2_id_h:3;
	u32 v1_id:12;
	u32 v0_id:12;
	u32 v3_valid:1;
	u32 v2_valid:1;
	u32 v1_valid:1;
	u32 v0_valid:1;
	u32 valid:1;
};

struct mtk_lro_alt_v2_info1 {
	u32 sip3_h:9;
	u32 v6_valid:1;
	u32 v4_valid:1;
	u32 v3_id:12;
	u32 v2_id_l:9;
};

struct mtk_lro_alt_v2_info2 {
	u32 sip2_h:9;
	u32 sip3_l:23;
};
struct mtk_lro_alt_v2_info3 {
	u32 sip1_h:9;
	u32 sip2_l:23;
};
struct mtk_lro_alt_v2_info4 {
	u32 sip0_h:9;
	u32 sip1_l:23;
};
struct mtk_lro_alt_v2_info5 {
	u32 dip3_h:9;
	u32 sip0_l:23;
};
struct mtk_lro_alt_v2_info6 {
	u32 dip2_h:9;
	u32 dip3_l:23;
};
struct mtk_lro_alt_v2_info7 {
	u32 dip1_h:9;
	u32 dip2_l:23;
};
struct mtk_lro_alt_v2_info8 {
	u32 dip0_h:9;
	u32 dip1_l:23;
};
struct mtk_lro_alt_v2_info9 {
	u32 sp_h:9;
	u32 dip0_l:23;
};
struct mtk_lro_alt_v2_info10 {
	u32 resv:9;
	u32 dp:16;
	u32 sp_l:7;
};

struct mtk_lro_alt_v2 {
	struct mtk_lro_alt_v2_info0 alt_info0;
	struct mtk_lro_alt_v2_info1 alt_info1;
	struct mtk_lro_alt_v2_info2 alt_info2;
	struct mtk_lro_alt_v2_info3 alt_info3;
	struct mtk_lro_alt_v2_info4 alt_info4;
	struct mtk_lro_alt_v2_info5 alt_info5;
	struct mtk_lro_alt_v2_info6 alt_info6;
	struct mtk_lro_alt_v2_info7 alt_info7;
	struct mtk_lro_alt_v2_info8 alt_info8;
	struct mtk_lro_alt_v2_info9 alt_info9;
	struct mtk_lro_alt_v2_info10 alt_info10;
};

struct mtk_esw_reg {
	unsigned int off;
	unsigned int val;
};

struct mtk_mii_ioctl_data {
	u16 phy_id;
	u16 reg_num;
	unsigned int val_in;
	unsigned int val_out;
};

#if defined(CONFIG_NET_DSA_MT7530) || defined(CONFIG_MT753X_GSW)
static inline bool mt7530_exist(struct mtk_eth *eth)
{
	return true;
}
#else
static inline bool mt7530_exist(struct mtk_eth *eth)
{
	return false;
}
#endif

extern u32 _mtk_mdio_read(struct mtk_eth *eth, u16 phy_addr, u16 phy_reg);
extern u32 _mtk_mdio_write(struct mtk_eth *eth, u16 phy_addr,
		    u16 phy_register, u16 write_data);

extern u32 mtk_cl45_ind_read(struct mtk_eth *eth, u16 port, u16 devad, u16 reg, u16 *data);
extern u32 mtk_cl45_ind_write(struct mtk_eth *eth, u16 port, u16 devad, u16 reg, u16 data);

int debug_proc_init(struct mtk_eth *eth);
void debug_proc_exit(void);

int mtketh_debugfs_init(struct mtk_eth *eth);
void mtketh_debugfs_exit(struct mtk_eth *eth);
int mtk_do_priv_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd);
void hw_lro_stats_update(u32 ring_no, struct mtk_rx_dma *rxd);
void hw_lro_flush_stats_update(u32 ring_no, struct mtk_rx_dma *rxd);

#endif /* MTK_ETH_DBG_H */

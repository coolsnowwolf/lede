/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2018 MediaTek Inc.
 * Author: Weijie Gao <weijie.gao@mediatek.com>
 */

#ifndef _MT753X_REGS_H_
#define _MT753X_REGS_H_

#include <linux/bitops.h>

/* Values of Egress TAG Control */
#define ETAG_CTRL_UNTAG			0
#define ETAG_CTRL_TAG			2
#define ETAG_CTRL_SWAP			1
#define ETAG_CTRL_STACK			3

#define VTCR				0x90
#define VAWD1				0x94
#define VAWD2				0x98

/* Fields of VTCR */
#define VTCR_BUSY			BIT(31)
#define IDX_INVLD			BIT(16)
#define VTCR_FUNC_S			12
#define VTCR_FUNC_M			0xf000
#define VTCR_VID_S			0
#define VTCR_VID_M			0xfff

/* Values of VTCR_FUNC */
#define VTCR_READ_VLAN_ENTRY		0
#define VTCR_WRITE_VLAN_ENTRY		1
#define VTCR_INVD_VLAN_ENTRY		2
#define VTCR_ENABLE_VLAN_ENTRY		3
#define VTCR_READ_ACL_ENTRY		4
#define VTCR_WRITE_ACL_ENTRY		5
#define VTCR_READ_TRTCM_TABLE		6
#define VTCR_WRITE_TRTCM_TABLE		7
#define VTCR_READ_ACL_MASK_ENTRY	8
#define VTCR_WRITE_ACL_MASK_ENTRY	9
#define VTCR_READ_ACL_RULE_ENTRY	10
#define VTCR_WRITE_ACL_RULE_ENTRY	11
#define VTCR_READ_ACL_RATE_ENTRY	12
#define VTCR_WRITE_ACL_RATE_ENTRY	13

/* VLAN entry fields */
/* VAWD1 */
#define PORT_STAG			BIT(31)
#define IVL_MAC				BIT(30)
#define EG_CON				BIT(29)
#define VTAG_EN				BIT(28)
#define COPY_PRI			BIT(27)
#define USER_PRI_S			24
#define USER_PRI_M			0x7000000
#define PORT_MEM_S			16
#define PORT_MEM_M			0xff0000
#define S_TAG1_S			4
#define S_TAG1_M			0xfff0
#define FID_S				1
#define FID_M				0x0e
#define VENTRY_VALID			BIT(0)

/* VAWD2 */
#define S_TAG2_S			16
#define S_TAG2_M			0xffff0000
#define PORT_ETAG_S(p)			((p) * 2)
#define PORT_ETAG_M			0x03

#define PORT_CTRL_BASE			0x2000
#define PORT_CTRL_PORT_OFFSET		0x100
#define PORT_CTRL_REG(p, r)		(PORT_CTRL_BASE + \
					(p) * PORT_CTRL_PORT_OFFSET +  (r))
#define CKGCR(p)			PORT_CTRL_REG(p, 0x00)
#define PCR(p)				PORT_CTRL_REG(p, 0x04)
#define PIC(p)				PORT_CTRL_REG(p, 0x08)
#define PSC(p)				PORT_CTRL_REG(p, 0x0c)
#define PVC(p)				PORT_CTRL_REG(p, 0x10)
#define PPBV1(p)			PORT_CTRL_REG(p, 0x14)
#define PPBV2(p)			PORT_CTRL_REG(p, 0x18)
#define BSR(p)				PORT_CTRL_REG(p, 0x1c)
#define STAG01				PORT_CTRL_REG(p, 0x20)
#define STAG23				PORT_CTRL_REG(p, 0x24)
#define STAG45				PORT_CTRL_REG(p, 0x28)
#define STAG67				PORT_CTRL_REG(p, 0x2c)

#define PPBV(p, g)			(PPBV1(p) + ((g) / 2) * 4)

/* Fields of PCR */
#define MLDV2_EN			BIT(30)
#define EG_TAG_S			28
#define EG_TAG_M			0x30000000
#define PORT_PRI_S			24
#define PORT_PRI_M			0x7000000
#define PORT_MATRIX_S			16
#define PORT_MATRIX_M			0xff0000
#define UP2DSCP_EN			BIT(12)
#define UP2TAG_EN			BIT(11)
#define ACL_EN				BIT(10)
#define PORT_TX_MIR			BIT(9)
#define PORT_RX_MIR			BIT(8)
#define ACL_MIR				BIT(7)
#define MIS_PORT_FW_S			4
#define MIS_PORT_FW_M			0x70
#define VLAN_MIS			BIT(2)
#define PORT_VLAN_S			0
#define PORT_VLAN_M			0x03

/* Values of PORT_VLAN */
#define PORT_MATRIX_MODE		0
#define FALLBACK_MODE			1
#define CHECK_MODE			2
#define SECURITY_MODE			3

/* Fields of PVC */
#define STAG_VPID_S			16
#define STAG_VPID_M			0xffff0000
#define DIS_PVID			BIT(15)
#define FORCE_PVID			BIT(14)
#define PT_VPM				BIT(12)
#define PT_OPTION			BIT(11)
#define PVC_EG_TAG_S			8
#define PVC_EG_TAG_M			0x700
#define VLAN_ATTR_S			6
#define VLAN_ATTR_M			0xc0
#define PVC_PORT_STAG			BIT(5)
#define BC_LKYV_EN			BIT(4)
#define MC_LKYV_EN			BIT(3)
#define UC_LKYV_EN			BIT(2)
#define ACC_FRM_S			0
#define ACC_FRM_M			0x03

/* Values of VLAN_ATTR */
#define VA_USER_PORT			0
#define VA_STACK_PORT			1
#define VA_TRANSLATION_PORT		2
#define VA_TRANSPARENT_PORT		3

/* Fields of PPBV */
#define GRP_PORT_PRI_S(g)		(((g) % 2) * 16 + 13)
#define GRP_PORT_PRI_M			0x07
#define GRP_PORT_VID_S(g)		(((g) % 2) * 16)
#define GRP_PORT_VID_M			0xfff

#define PORT_MAC_CTRL_BASE		0x3000
#define PORT_MAC_CTRL_PORT_OFFSET	0x100
#define PORT_MAC_CTRL_REG(p, r)		(PORT_MAC_CTRL_BASE + \
					(p) * PORT_MAC_CTRL_PORT_OFFSET + (r))
#define PMCR(p)				PORT_MAC_CTRL_REG(p, 0x00)
#define PMEEECR(p)			PORT_MAC_CTRL_REG(p, 0x04)
#define PMSR(p)				PORT_MAC_CTRL_REG(p, 0x08)
#define PINT_EN(p)			PORT_MAC_CTRL_REG(p, 0x10)
#define PINT_STS(p)			PORT_MAC_CTRL_REG(p, 0x14)

#define GMACCR				(PORT_MAC_CTRL_BASE + 0xe0)
#define TXCRC_EN			BIT(19)
#define RXCRC_EN			BIT(18)
#define PRMBL_LMT_EN			BIT(17)
#define MTCC_LMT_S			9
#define MTCC_LMT_M			0x1e00
#define MAX_RX_JUMBO_S			2
#define MAX_RX_JUMBO_M			0x3c
#define MAX_RX_PKT_LEN_S		0
#define MAX_RX_PKT_LEN_M		0x3

/* Values of MAX_RX_PKT_LEN */
#define RX_PKT_LEN_1518			0
#define RX_PKT_LEN_1536			1
#define RX_PKT_LEN_1522			2
#define RX_PKT_LEN_MAX_JUMBO		3

/* Fields of PMCR */
#define IPG_CFG_S			18
#define IPG_CFG_M			0xc0000
#define EXT_PHY				BIT(17)
#define MAC_MODE			BIT(16)
#define MAC_TX_EN			BIT(14)
#define MAC_RX_EN			BIT(13)
#define MAC_PRE				BIT(11)
#define BKOFF_EN			BIT(9)
#define BACKPR_EN			BIT(8)
#define FORCE_EEE1G			BIT(7)
#define FORCE_EEE1000			BIT(6)
#define FORCE_RX_FC			BIT(5)
#define FORCE_TX_FC			BIT(4)
#define FORCE_SPD_S			2
#define FORCE_SPD_M			0x0c
#define FORCE_DPX			BIT(1)
#define FORCE_LINK			BIT(0)

/* Fields of PMSR */
#define EEE1G_STS			BIT(7)
#define EEE100_STS			BIT(6)
#define RX_FC_STS			BIT(5)
#define TX_FC_STS			BIT(4)
#define MAC_SPD_STS_S			2
#define MAC_SPD_STS_M			0x0c
#define MAC_DPX_STS			BIT(1)
#define MAC_LNK_STS			BIT(0)

/* Values of MAC_SPD_STS */
#define MAC_SPD_10			0
#define MAC_SPD_100			1
#define MAC_SPD_1000			2
#define MAC_SPD_2500			3

/* Values of IPG_CFG */
#define IPG_96BIT			0
#define IPG_96BIT_WITH_SHORT_IPG	1
#define IPG_64BIT			2

#define MIB_COUNTER_BASE		0x4000
#define MIB_COUNTER_PORT_OFFSET		0x100
#define MIB_COUNTER_REG(p, r)		(MIB_COUNTER_BASE + \
					(p) * MIB_COUNTER_PORT_OFFSET + (r))

#define STATS_TDPC			0x00
#define STATS_TCRC			0x04
#define STATS_TUPC			0x08
#define STATS_TMPC			0x0C
#define STATS_TBPC			0x10
#define STATS_TCEC			0x14
#define STATS_TSCEC			0x18
#define STATS_TMCEC			0x1C
#define STATS_TDEC			0x20
#define STATS_TLCEC			0x24
#define STATS_TXCEC			0x28
#define STATS_TPPC			0x2C
#define STATS_TL64PC			0x30
#define STATS_TL65PC			0x34
#define STATS_TL128PC			0x38
#define STATS_TL256PC			0x3C
#define STATS_TL512PC			0x40
#define STATS_TL1024PC			0x44
#define STATS_TOC			0x48
#define STATS_RDPC			0x60
#define STATS_RFPC			0x64
#define STATS_RUPC			0x68
#define STATS_RMPC			0x6C
#define STATS_RBPC			0x70
#define STATS_RAEPC			0x74
#define STATS_RCEPC			0x78
#define STATS_RUSPC			0x7C
#define STATS_RFEPC			0x80
#define STATS_ROSPC			0x84
#define STATS_RJEPC			0x88
#define STATS_RPPC			0x8C
#define STATS_RL64PC			0x90
#define STATS_RL65PC			0x94
#define STATS_RL128PC			0x98
#define STATS_RL256PC			0x9C
#define STATS_RL512PC			0xA0
#define STATS_RL1024PC			0xA4
#define STATS_ROC			0xA8
#define STATS_RDPC_CTRL			0xB0
#define STATS_RDPC_ING			0xB4
#define STATS_RDPC_ARL			0xB8

#define SYS_CTRL			0x7000
#define SW_PHY_RST			BIT(2)
#define SW_SYS_RST			BIT(1)
#define SW_REG_RST			BIT(0)

#define SYS_INT_EN			0x7008
#define SYS_INT_STS			0x700c
#define MAC_PC_INT			BIT(16)
#define PHY_INT(p)			BIT((p) + 8)
#define PHY_LC_INT(p)			BIT(p)

#define PHY_IAC				0x701c
#define PHY_ACS_ST			BIT(31)
#define MDIO_REG_ADDR_S			25
#define MDIO_REG_ADDR_M			0x3e000000
#define MDIO_PHY_ADDR_S			20
#define MDIO_PHY_ADDR_M			0x1f00000
#define MDIO_CMD_S			18
#define MDIO_CMD_M			0xc0000
#define MDIO_ST_S			16
#define MDIO_ST_M			0x30000
#define MDIO_RW_DATA_S			0
#define MDIO_RW_DATA_M			0xffff

/* MDIO_CMD: MDIO commands */
#define MDIO_CMD_ADDR			0
#define MDIO_CMD_WRITE			1
#define MDIO_CMD_READ			2
#define MDIO_CMD_READ_C45		3

/* MDIO_ST: MDIO start field */
#define MDIO_ST_C45			0
#define MDIO_ST_C22			1

#define HWSTRAP				0x7800
#define MHWSTRAP			0x7804

/* Internal GPHY Page Control Register */
#define PHY_CL22_PAGE_CTRL		0x1f
#define PHY_TR_PAGE			0x52b5

/* Internal GPHY Token Ring Access Registers */
#define PHY_TR_CTRL			0x10
#define PHY_TR_LOW_DATA			0x11
#define PHY_TR_HIGH_DATA		0x12

/* Fields of PHY_TR_CTRL */
#define PHY_TR_PKT_XMT_STA		BIT(15)
#define PHY_TR_WR_S			13
#define PHY_TR_CH_ADDR_S		11
#define PHY_TR_NODE_ADDR_S		7
#define PHY_TR_DATA_ADDR_S		1

enum phy_tr_wr {
	PHY_TR_WRITE = 0,
	PHY_TR_READ = 1,
};

/* Helper macro for GPHY Token Ring Access */
#define PHY_TR_LOW_VAL(x)		((x) & 0xffff)
#define PHY_TR_HIGH_VAL(x)		(((x) & 0xff0000) >> 16)

/* Token Ring Channels */
#define PMA_CH				0x1
#define DSP_CH				0x2

/* Token Ring Nodes */
#define PMA_NOD				0xf
#define DSP_NOD				0xd

/* Token Ring register range */
enum tr_pma_reg_addr {
	PMA_MIN = 0x0,
	PMA_01  = 0x1,
	PMA_17  = 0x17,
	PMA_18  = 0x18,
	PMA_MAX = 0x3d,
};

enum tr_dsp_reg_addr {
	DSP_MIN = 0x0,
	DSP_06  = 0x6,
	DSP_08  = 0x8,
	DSP_0f  = 0xf,
	DSP_10  = 0x10,
	DSP_MAX = 0x3e,
};
#endif /* _MT753X_REGS_H_ */

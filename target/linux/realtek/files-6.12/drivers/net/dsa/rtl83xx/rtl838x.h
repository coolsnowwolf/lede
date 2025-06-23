/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef _RTL838X_H
#define _RTL838X_H

#include <net/dsa.h>

/* Register definition */
#define RTL838X_MAC_PORT_CTRL(port)		(0xd560 + (((port) << 7)))
#define RTL839X_MAC_PORT_CTRL(port)		(0x8004 + (((port) << 7)))
#define RTL930X_MAC_PORT_CTRL(port)		(0x3260 + (((port) << 6)))
#define RTL931X_MAC_PORT_CTRL			(0x6004)

#define RTL930X_MAC_L2_PORT_CTRL(port)		(0x3268 + (((port) << 6)))
#define RTL931X_MAC_L2_PORT_CTRL		(0x6000)

#define RTL838X_RST_GLB_CTRL_0			(0x003c)

#define RTL838X_MAC_FORCE_MODE_CTRL		(0xa104)
#define RTL839X_MAC_FORCE_MODE_CTRL		(0x02bc)
#define RTL930X_MAC_FORCE_MODE_CTRL		(0xCA1C)
#define RTL931X_MAC_FORCE_MODE_CTRL		(0x0DCC)

#define RTL838X_DMY_REG31			(0x3b28)
#define RTL838X_SDS_MODE_SEL			(0x0028)
#define RTL838X_SDS_CFG_REG			(0x0034)
#define RTL838X_INT_MODE_CTRL			(0x005c)
#define RTL838X_CHIP_INFO			(0x00d8)
#define RTL839X_CHIP_INFO			(0x0ff4)
#define RTL838X_PORT_ISO_CTRL(port)		(0x4100 + ((port) << 2))
#define RTL839X_PORT_ISO_CTRL(port)		(0x1400 + ((port) << 3))

/* Packet statistics */
#define RTL838X_STAT_PORT_STD_MIB		(0x1200)
#define RTL839X_STAT_PORT_STD_MIB		(0xC000)
#define RTL930X_STAT_PORT_MIB_CNTR		(0x0664)
#define RTL838X_STAT_RST			(0x3100)
#define RTL839X_STAT_RST			(0xF504)
#define RTL930X_STAT_RST			(0x3240)
#define RTL931X_STAT_RST			(0x7ef4)
#define RTL838X_STAT_PORT_RST			(0x3104)
#define RTL839X_STAT_PORT_RST			(0xF508)
#define RTL930X_STAT_PORT_RST			(0x3244)
#define RTL931X_STAT_PORT_RST			(0x7ef8)
#define RTL838X_STAT_CTRL			(0x3108)
#define RTL839X_STAT_CTRL			(0x04cc)
#define RTL930X_STAT_CTRL			(0x3248)
#define RTL931X_STAT_CTRL			(0x5720)

/* Registers of the internal Serdes of the 8390 */
#define RTL8390_SDS0_1_XSG0			(0xA000)
#define RTL8390_SDS0_1_XSG1			(0xA100)
#define RTL839X_SDS12_13_XSG0			(0xB800)
#define RTL839X_SDS12_13_XSG1			(0xB900)
#define RTL839X_SDS12_13_PWR0			(0xb880)
#define RTL839X_SDS12_13_PWR1			(0xb980)

/* Registers of the internal Serdes of the 8380 */
#define RTL838X_SDS4_FIB_REG0			(0xF800)
#define RTL838X_SDS4_REG28			(0xef80)
#define RTL838X_SDS4_DUMMY0			(0xef8c)
#define RTL838X_SDS5_EXT_REG6			(0xf18c)

/* VLAN registers */
#define RTL838X_VLAN_CTRL			(0x3A74)
#define RTL838X_VLAN_PROFILE(idx)		(0x3A88 + ((idx) << 2))
#define RTL838X_VLAN_PORT_EGR_FLTR		(0x3A84)
#define RTL838X_VLAN_PORT_PB_VLAN		(0x3C00)
#define RTL838X_VLAN_PORT_IGR_FLTR		(0x3A7C)

#define RTL839X_VLAN_PROFILE(idx)		(0x25C0 + (((idx) << 3)))
#define RTL839X_VLAN_CTRL			(0x26D4)
#define RTL839X_VLAN_PORT_PB_VLAN		(0x26D8)
#define RTL839X_VLAN_PORT_IGR_FLTR		(0x27B4)
#define RTL839X_VLAN_PORT_EGR_FLTR		(0x27C4)

#define RTL930X_VLAN_PROFILE_SET(idx)		(0x9c60 + (((idx) * 20)))
#define RTL930X_VLAN_CTRL			(0x82D4)
#define RTL930X_VLAN_PORT_PB_VLAN		(0x82D8)
#define RTL930X_VLAN_PORT_IGR_FLTR		(0x83C0)
#define RTL930X_VLAN_PORT_EGR_FLTR		(0x83C8)

#define RTL931X_VLAN_PROFILE_SET(idx)		(0x9800 + (((idx) * 28)))
#define RTL931X_VLAN_CTRL			(0x94E4)
#define RTL931X_VLAN_PORT_IGR_CTRL		(0x94E8)
#define RTL931X_VLAN_PORT_IGR_FLTR		(0x96B4)
#define RTL931X_VLAN_PORT_EGR_FLTR		(0x96C4)

/* Table access registers */
#define RTL838X_TBL_ACCESS_CTRL_0		(0x6914)
#define RTL838X_TBL_ACCESS_DATA_0(idx)		(0x6918 + ((idx) << 2))
#define RTL838X_TBL_ACCESS_CTRL_1		(0xA4C8)
#define RTL838X_TBL_ACCESS_DATA_1(idx)		(0xA4CC + ((idx) << 2))

#define RTL839X_TBL_ACCESS_CTRL_0		(0x1190)
#define RTL839X_TBL_ACCESS_DATA_0(idx)		(0x1194 + ((idx) << 2))
#define RTL839X_TBL_ACCESS_CTRL_1		(0x6b80)
#define RTL839X_TBL_ACCESS_DATA_1(idx)		(0x6b84 + ((idx) << 2))
#define RTL839X_TBL_ACCESS_CTRL_2		(0x611C)
#define RTL839X_TBL_ACCESS_DATA_2(i)		(0x6120 + (((i) << 2)))

#define RTL930X_TBL_ACCESS_CTRL_0		(0xB340)
#define RTL930X_TBL_ACCESS_DATA_0(idx)		(0xB344 + ((idx) << 2))
#define RTL930X_TBL_ACCESS_CTRL_1		(0xB3A0)
#define RTL930X_TBL_ACCESS_DATA_1(idx)		(0xB3A4 + ((idx) << 2))
#define RTL930X_TBL_ACCESS_CTRL_2		(0xCE04)
#define RTL930X_TBL_ACCESS_DATA_2(i)		(0xCE08 + (((i) << 2)))

#define RTL931X_TBL_ACCESS_CTRL_0		(0x8500)
#define RTL931X_TBL_ACCESS_DATA_0(idx)		(0x8508 + ((idx) << 2))
#define RTL931X_TBL_ACCESS_CTRL_1		(0x40C0)
#define RTL931X_TBL_ACCESS_DATA_1(idx)		(0x40C4 + ((idx) << 2))
#define RTL931X_TBL_ACCESS_CTRL_2		(0x8528)
#define RTL931X_TBL_ACCESS_DATA_2(i)		(0x852C + (((i) << 2)))
#define RTL931X_TBL_ACCESS_CTRL_3		(0x0200)
#define RTL931X_TBL_ACCESS_DATA_3(i)		(0x0204 + (((i) << 2)))
#define RTL931X_TBL_ACCESS_CTRL_4		(0x20DC)
#define RTL931X_TBL_ACCESS_DATA_4(i)		(0x20E0 + (((i) << 2)))
#define RTL931X_TBL_ACCESS_CTRL_5		(0x7E1C)
#define RTL931X_TBL_ACCESS_DATA_5(i)		(0x7E20 + (((i) << 2)))

/* MAC handling */
#define RTL838X_MAC_LINK_STS			(0xa188)
#define RTL839X_MAC_LINK_STS			(0x0390)
#define RTL930X_MAC_LINK_STS			(0xCB10)
#define RTL931X_MAC_LINK_STS			(0x0EC0)
#define RTL838X_MAC_LINK_SPD_STS(p)		(0xa190 + (((p >> 4) << 2)))
#define RTL839X_MAC_LINK_SPD_STS(p)		(0x03a0 + (((p >> 4) << 2)))
#define RTL930X_MAC_LINK_SPD_STS(p)		(0xCB18 + (((p >> 3) << 2)))
#define RTL931X_MAC_LINK_SPD_STS		(0x0ED0)
#define RTL838X_MAC_LINK_DUP_STS		(0xa19c)
#define RTL839X_MAC_LINK_DUP_STS		(0x03b0)
#define RTL930X_MAC_LINK_DUP_STS		(0xCB28)
#define RTL931X_MAC_LINK_DUP_STS		(0x0EF0)
#define RTL838X_MAC_TX_PAUSE_STS		(0xa1a0)
#define RTL839X_MAC_TX_PAUSE_STS		(0x03b8)
#define RTL930X_MAC_TX_PAUSE_STS		(0xCB2C)
#define RTL931X_MAC_TX_PAUSE_STS		(0x0EF8)
#define RTL838X_MAC_RX_PAUSE_STS		(0xa1a4)
#define RTL839X_MAC_RX_PAUSE_STS		(0x03c0)
#define RTL930X_MAC_RX_PAUSE_STS		(0xCB30)
#define RTL931X_MAC_RX_PAUSE_STS		(0x0F00)
#define RTL930X_MAC_LINK_MEDIA_STS		(0xCB14)
#define RTL931X_MAC_LINK_MEDIA_STS		(0x0EC8)

/* MAC link state bits */
#define RTL_SPEED_10				0
#define RTL_SPEED_100				1
#define RTL_SPEED_1000				2
#define RTL_SPEED_2500				5
#define RTL_SPEED_5000				6
#define RTL_SPEED_10000				4

#define RTL83XX_FORCE_EN			(1 << 0)
#define RTL83XX_FORCE_LINK_EN			(1 << 1)

#define RTL838X_NWAY_EN				(1 << 2)
#define RTL838X_DUPLEX_MODE			(1 << 3)
#define RTL838X_SPEED_SHIFT			(4)
#define RTL838X_SPEED_MASK			(3 << RTL838X_SPEED_SHIFT)
#define RTL838X_TX_PAUSE_EN			(1 << 6)
#define RTL838X_RX_PAUSE_EN			(1 << 7)
#define RTL838X_MAC_FORCE_FC_EN			(1 << 8)

#define RTL839X_DUPLEX_MODE			(1 << 2)
#define RTL839X_SPEED_SHIFT			(3)
#define RTL839X_SPEED_MASK			(3 << RTL839X_SPEED_SHIFT)
#define RTL839X_TX_PAUSE_EN			(1 << 5)
#define RTL839X_RX_PAUSE_EN			(1 << 6)
#define RTL839X_MAC_FORCE_FC_EN			(1 << 7)

#define RTL930X_FORCE_EN			(1 << 0)
#define RTL930X_FORCE_LINK_EN			(1 << 1)
#define RTL930X_DUPLEX_MODE			(1 << 2)
#define RTL930X_SPEED_SHIFT			(3)
#define RTL930X_SPEED_MASK			(15 << RTL930X_SPEED_SHIFT)
#define RTL930X_TX_PAUSE_EN			(1 << 7)
#define RTL930X_RX_PAUSE_EN			(1 << 8)
#define RTL930X_MAC_FORCE_FC_EN			(1 << 9)

#define RTL931X_FORCE_EN			(1 << 9)
#define RTL931X_FORCE_LINK_EN			(1 << 0)
#define RTL931X_DUPLEX_MODE			(1 << 2)
#define RTL931X_MAC_FORCE_FC_EN			(1 << 4)
#define RTL931X_TX_PAUSE_EN			(1 << 16)
#define RTL931X_RX_PAUSE_EN			(1 << 17)

/* EEE */
#define RTL838X_MAC_EEE_ABLTY			(0xa1a8)
#define RTL838X_EEE_PORT_TX_EN			(0x014c)
#define RTL838X_EEE_PORT_RX_EN			(0x0150)
#define RTL838X_EEE_CLK_STOP_CTRL		(0x0148)
#define RTL838X_EEE_TX_TIMER_GIGA_CTRL		(0xaa04)
#define RTL838X_EEE_TX_TIMER_GELITE_CTRL	(0xaa08)

#define RTL839X_EEE_TX_TIMER_GELITE_CTRL	(0x042C)
#define RTL839X_EEE_TX_TIMER_GIGA_CTRL		(0x0430)
#define RTL839X_EEE_TX_TIMER_10G_CTRL		(0x0434)
#define RTL839X_EEE_CTRL(p)			(0x8008 + ((p) << 7))
#define RTL839X_MAC_EEE_ABLTY			(0x03C8)

#define RTL930X_MAC_EEE_ABLTY			(0xCB34)
#define RTL930X_EEE_CTRL(p)			(0x3274 + ((p) << 6))
#define RTL930X_EEEP_PORT_CTRL(p)		(0x3278 + ((p) << 6))

#define RTL931X_MAC_EEE_ABLTY			(0x0f08)

/* L2 functionality */
#define RTL838X_L2_CTRL_0			(0x3200)
#define RTL839X_L2_CTRL_0			(0x3800)
#define RTL930X_L2_CTRL				(0x8FD8)
#define RTL931X_L2_CTRL				(0xC800)
#define RTL838X_L2_CTRL_1			(0x3204)
#define RTL839X_L2_CTRL_1			(0x3804)
#define RTL930X_L2_AGE_CTRL			(0x8FDC)
#define RTL931X_L2_AGE_CTRL			(0xC804)
#define RTL838X_L2_PORT_AGING_OUT		(0x3358)
#define RTL839X_L2_PORT_AGING_OUT		(0x3b74)
#define	RTL930X_L2_PORT_AGE_CTRL		(0x8FE0)
#define	RTL931X_L2_PORT_AGE_CTRL		(0xc808)
#define RTL838X_TBL_ACCESS_L2_CTRL		(0x6900)
#define RTL839X_TBL_ACCESS_L2_CTRL		(0x1180)
#define RTL930X_TBL_ACCESS_L2_CTRL		(0xB320)
#define RTL930X_TBL_ACCESS_L2_METHOD_CTRL	(0xB324)
#define RTL838X_TBL_ACCESS_L2_DATA(idx)		(0x6908 + ((idx) << 2))
#define RTL839X_TBL_ACCESS_L2_DATA(idx)		(0x1184 + ((idx) << 2))
#define RTL930X_TBL_ACCESS_L2_DATA(idx)		(0xab08 + ((idx) << 2))

#define RTL838X_L2_TBL_FLUSH_CTRL		(0x3370)
#define RTL839X_L2_TBL_FLUSH_CTRL		(0x3ba0)
#define RTL930X_L2_TBL_FLUSH_CTRL		(0x9404)
#define RTL931X_L2_TBL_FLUSH_CTRL		(0xCD9C)

#define RTL838X_L2_LRN_CONSTRT			(0x329C)
#define RTL839X_L2_LRN_CONSTRT			(0x3910)
#define RTL930X_L2_LRN_CONSTRT_CTRL		(0x909c)
#define RTL931X_L2_LRN_CONSTRT_CTRL		(0xC964)

#define RTL838X_L2_FLD_PMSK			(0x3288)
#define RTL839X_L2_FLD_PMSK			(0x38EC)
#define RTL930X_L2_BC_FLD_PMSK			(0x9068)
#define RTL931X_L2_BC_FLD_PMSK			(0xC8FC)

#define RTL930X_L2_UNKN_UC_FLD_PMSK		(0x9064)
#define RTL931X_L2_UNKN_UC_FLD_PMSK		(0xC8F4)

#define RTL838X_L2_LRN_CONSTRT_EN		(0x3368)
#define RTL838X_L2_PORT_LRN_CONSTRT		(0x32A0)
#define RTL839X_L2_PORT_LRN_CONSTRT		(0x3914)

#define RTL838X_L2_PORT_NEW_SALRN(p)		(0x328c + (((p >> 4) << 2)))
#define RTL839X_L2_PORT_NEW_SALRN(p)		(0x38F0 + (((p >> 4) << 2)))
#define RTL930X_L2_PORT_SALRN(p)		(0x8FEC + (((p >> 4) << 2)))
#define RTL931X_L2_PORT_NEW_SALRN(p)		(0xC820 + (((p >> 4) << 2)))

#define SALRN_PORT_SHIFT(p)			((p % 16) * 2)
#define SALRN_MODE_MASK				0x3
#define SALRN_MODE_HARDWARE			0
#define SALRN_MODE_DISABLED			2

#define RTL838X_L2_PORT_NEW_SA_FWD(p)		(0x3294 + (((p >> 4) << 2)))
#define RTL839X_L2_PORT_NEW_SA_FWD(p)		(0x3900 + (((p >> 4) << 2)))
#define RTL930X_L2_PORT_NEW_SA_FWD(p)		(0x8FF4 + (((p / 10) << 2)))
#define RTL931X_L2_PORT_NEW_SA_FWD(p)		(0xC830 + (((p / 10) << 2)))

#define RTL838X_L2_PORT_MV_ACT(p)		(0x335c + (((p >> 4) << 2)))
#define RTL839X_L2_PORT_MV_ACT(p)		(0x3b80 + (((p >> 4) << 2)))

#define RTL838X_L2_PORT_STATIC_MV_ACT(p)	(0x327c + (((p >> 4) << 2)))
#define RTL839X_L2_PORT_STATIC_MV_ACT(p)	(0x38dc + (((p >> 4) << 2)))

#define MV_ACT_PORT_SHIFT(p)			((p % 16) * 2)
#define MV_ACT_MASK				0x3
#define MV_ACT_FORWARD				0
#define MV_ACT_DROP				1
#define MV_ACT_TRAP2CPU				2
#define MV_ACT_COPY2CPU				3

#define RTL930X_ST_CTRL				(0x8798)

#define RTL930X_L2_PORT_SABLK_CTRL		(0x905c)
#define RTL930X_L2_PORT_DABLK_CTRL		(0x9060)

#define RTL838X_L2_PORT_LM_ACT(p)		(0x3208 + ((p) << 2))
#define RTL838X_VLAN_PORT_FWD			(0x3A78)
#define RTL839X_VLAN_PORT_FWD			(0x27AC)
#define RTL930X_VLAN_PORT_FWD			(0x834C)
#define RTL931X_VLAN_PORT_FWD			(0x95CC)
#define RTL838X_VLAN_FID_CTRL			(0x3aa8)

/* Port Mirroring */
#define RTL838X_MIR_CTRL			(0x5D00)
#define RTL838X_MIR_DPM_CTRL			(0x5D20)
#define RTL838X_MIR_SPM_CTRL			(0x5D10)

#define RTL839X_MIR_CTRL			(0x2500)
#define RTL839X_MIR_DPM_CTRL			(0x2530)
#define RTL839X_MIR_SPM_CTRL			(0x2510)

#define RTL930X_MIR_CTRL			(0xA2A0)
#define RTL930X_MIR_DPM_CTRL			(0xA2C0)
#define RTL930X_MIR_SPM_CTRL			(0xA2B0)

#define RTL931X_MIR_CTRL			(0xAF00)
#define RTL931X_MIR_DPM_CTRL			(0xAF30)
#define RTL931X_MIR_SPM_CTRL			(0xAF10)

/* Storm/rate control and scheduling */
#define RTL838X_STORM_CTRL			(0x4700)
#define RTL839X_STORM_CTRL			(0x1800)
#define RTL838X_STORM_CTRL_LB_CTRL(p)		(0x4884 + (((p) << 2)))
#define RTL838X_STORM_CTRL_BURST_PPS_0		(0x4874)
#define RTL838X_STORM_CTRL_BURST_PPS_1		(0x4878)
#define RTL838X_STORM_CTRL_BURST_0		(0x487c)
#define RTL838X_STORM_CTRL_BURST_1		(0x4880)
#define RTL839X_STORM_CTRL_LB_TICK_TKN_CTRL_0	(0x1804)
#define RTL839X_STORM_CTRL_LB_TICK_TKN_CTRL_1	(0x1808)
#define RTL838X_SCHED_CTRL			(0xB980)
#define RTL839X_SCHED_CTRL			(0x60F4)
#define RTL838X_SCHED_LB_TICK_TKN_CTRL_0	(0xAD58)
#define RTL838X_SCHED_LB_TICK_TKN_CTRL_1	(0xAD5C)
#define RTL839X_SCHED_LB_TICK_TKN_CTRL_0	(0x1804)
#define RTL839X_SCHED_LB_TICK_TKN_CTRL_1	(0x1808)
#define RTL839X_STORM_CTRL_SPCL_LB_TICK_TKN_CTRL (0x2000)
#define RTL839X_IGR_BWCTRL_LB_TICK_TKN_CTRL_0	(0x1604)
#define RTL839X_IGR_BWCTRL_LB_TICK_TKN_CTRL_1	(0x1608)
#define RTL839X_SCHED_LB_TICK_TKN_CTRL		(0x60F8)
#define RTL839X_SCHED_LB_TICK_TKN_PPS_CTRL	(0x6200)
#define RTL838X_SCHED_LB_THR			(0xB984)
#define RTL839X_SCHED_LB_THR			(0x60FC)
#define RTL838X_SCHED_P_EGR_RATE_CTRL(p)	(0xC008 + (((p) << 7)))
#define RTL838X_SCHED_Q_EGR_RATE_CTRL(p, q)	(0xC00C + (p << 7) + (((q) << 2)))
#define RTL838X_STORM_CTRL_PORT_BC_EXCEED	(0x470C)
#define RTL838X_STORM_CTRL_PORT_MC_EXCEED	(0x4710)
#define RTL838X_STORM_CTRL_PORT_UC_EXCEED	(0x4714)
#define RTL839X_STORM_CTRL_PORT_BC_EXCEED(p)	(0x180c + (((p >> 5) << 2)))
#define RTL839X_STORM_CTRL_PORT_MC_EXCEED(p)	(0x1814 + (((p >> 5) << 2)))
#define RTL839X_STORM_CTRL_PORT_UC_EXCEED(p)	(0x181c + (((p >> 5) << 2)))
#define RTL838X_STORM_CTRL_PORT_UC(p)		(0x4718 + (((p) << 2)))
#define RTL838X_STORM_CTRL_PORT_MC(p)		(0x478c + (((p) << 2)))
#define RTL838X_STORM_CTRL_PORT_BC(p)		(0x4800 + (((p) << 2)))
#define RTL839X_STORM_CTRL_PORT_UC_0(p)		(0x185C + (((p) << 3)))
#define RTL839X_STORM_CTRL_PORT_UC_1(p)		(0x1860 + (((p) << 3)))
#define RTL839X_STORM_CTRL_PORT_MC_0(p)		(0x19FC + (((p) << 3)))
#define RTL839X_STORM_CTRL_PORT_MC_1(p)		(0x1a00 + (((p) << 3)))
#define RTL839X_STORM_CTRL_PORT_BC_0(p)		(0x1B9C + (((p) << 3)))
#define RTL839X_STORM_CTRL_PORT_BC_1(p)		(0x1BA0 + (((p) << 3)))
#define RTL839X_TBL_ACCESS_CTRL_2		(0x611C)
#define RTL839X_TBL_ACCESS_DATA_2(i)		(0x6120 + (((i) << 2)))
#define RTL839X_IGR_BWCTRL_PORT_CTRL_10G_0(p)	(0x1618 + (((p) << 3)))
#define RTL839X_IGR_BWCTRL_PORT_CTRL_10G_1(p)	(0x161C + (((p) << 3)))
#define RTL839X_IGR_BWCTRL_PORT_CTRL_0(p)	(0x1640 + (((p) << 3)))
#define RTL839X_IGR_BWCTRL_PORT_CTRL_1(p)	(0x1644 + (((p) << 3)))
#define RTL839X_IGR_BWCTRL_CTRL_LB_THR		(0x1614)

/* Link aggregation (Trunking) */
#define TRUNK_DISTRIBUTION_ALGO_SPA_BIT         0x01
#define TRUNK_DISTRIBUTION_ALGO_SMAC_BIT        0x02
#define TRUNK_DISTRIBUTION_ALGO_DMAC_BIT        0x04
#define TRUNK_DISTRIBUTION_ALGO_SIP_BIT         0x08
#define TRUNK_DISTRIBUTION_ALGO_DIP_BIT         0x10
#define TRUNK_DISTRIBUTION_ALGO_SRC_L4PORT_BIT  0x20
#define TRUNK_DISTRIBUTION_ALGO_DST_L4PORT_BIT  0x40
#define TRUNK_DISTRIBUTION_ALGO_MASKALL         0x7F

#define TRUNK_DISTRIBUTION_ALGO_L2_SPA_BIT         0x01
#define TRUNK_DISTRIBUTION_ALGO_L2_SMAC_BIT        0x02
#define TRUNK_DISTRIBUTION_ALGO_L2_DMAC_BIT        0x04
#define TRUNK_DISTRIBUTION_ALGO_L2_VLAN_BIT         0x08
#define TRUNK_DISTRIBUTION_ALGO_L2_MASKALL         0xF

#define TRUNK_DISTRIBUTION_ALGO_L3_SPA_BIT         0x01
#define TRUNK_DISTRIBUTION_ALGO_L3_SMAC_BIT        0x02
#define TRUNK_DISTRIBUTION_ALGO_L3_DMAC_BIT        0x04
#define TRUNK_DISTRIBUTION_ALGO_L3_VLAN_BIT         0x08
#define TRUNK_DISTRIBUTION_ALGO_L3_SIP_BIT         0x10
#define TRUNK_DISTRIBUTION_ALGO_L3_DIP_BIT         0x20
#define TRUNK_DISTRIBUTION_ALGO_L3_SRC_L4PORT_BIT  0x40
#define TRUNK_DISTRIBUTION_ALGO_L3_DST_L4PORT_BIT  0x80
#define TRUNK_DISTRIBUTION_ALGO_L3_PROTO_BIT  0x100
#define TRUNK_DISTRIBUTION_ALGO_L3_FLOW_LABEL_BIT  0x200
#define TRUNK_DISTRIBUTION_ALGO_L3_MASKALL         0x3FF

#define RTL838X_TRK_MBR_CTR                     (0x3E00)
#define RTL838X_TRK_HASH_IDX_CTRL               (0x3E20)
#define RTL838X_TRK_HASH_CTRL                   (0x3E24)

#define RTL839X_TRK_MBR_CTR                     (0x2200)
#define RTL839X_TRK_HASH_IDX_CTRL               (0x2280)
#define RTL839X_TRK_HASH_CTRL                   (0x2284)

#define RTL930X_TRK_MBR_CTRL                    (0xA41C)
#define RTL930X_TRK_HASH_CTRL                   (0x9F80)

#define RTL931X_TRK_MBR_CTRL                    (0xB8D0)
#define RTL931X_TRK_HASH_CTRL                   (0xBA70)

/* Attack prevention */
#define RTL838X_ATK_PRVNT_PORT_EN		(0x5B00)
#define RTL838X_ATK_PRVNT_CTRL			(0x5B04)
#define RTL838X_ATK_PRVNT_ACT			(0x5B08)
#define RTL838X_ATK_PRVNT_STS			(0x5B1C)

/* 802.1X */
#define RTL838X_RMA_BPDU_FLD_PMSK		(0x4348)
#define RTL930X_RMA_BPDU_FLD_PMSK		(0x9F18)
#define RTL931X_RMA_BPDU_FLD_PMSK		(0x8950)
#define RTL839X_RMA_BPDU_FLD_PMSK		(0x125C)

#define RTL838X_SPCL_TRAP_CTRL			(0x6980)
#define RTL838X_SPCL_TRAP_EAPOL_CTRL		(0x6988)
#define RTL838X_SPCL_TRAP_ARP_CTRL		(0x698C)
#define RTL838X_SPCL_TRAP_IGMP_CTRL		(0x6984)
#define RTL838X_SPCL_TRAP_IPV6_CTRL		(0x6994)
#define RTL838X_SPCL_TRAP_SWITCH_MAC_CTRL	(0x6998)

#define RTL839X_SPCL_TRAP_CTRL			(0x1054)
#define RTL839X_SPCL_TRAP_EAPOL_CTRL		(0x105C)
#define RTL839X_SPCL_TRAP_ARP_CTRL		(0x1060)
#define RTL839X_SPCL_TRAP_IGMP_CTRL		(0x1058)
#define RTL839X_SPCL_TRAP_IPV6_CTRL		(0x1064)
#define RTL839X_SPCL_TRAP_SWITCH_MAC_CTRL	(0x1068)
#define RTL839X_SPCL_TRAP_SWITCH_IPV4_ADDR_CTRL	(0x106C)
#define RTL839X_SPCL_TRAP_CRC_CTRL		(0x1070)
/* special port action controls */
/* values:
 *      0 = FORWARD (default)
 *      1 = DROP
 *      2 = TRAP2CPU
 *      3 = FLOOD IN ALL PORT
 *
 *      Register encoding.
 *      offset = CTRL + (port >> 4) << 2
 *      value/mask = 3 << ((port & 0xF) << 1)
 */

typedef enum {
	BPDU = 0,
	PTP,
	PTP_UDP,
	PTP_ETH2,
	LLDP,
	EAPOL,
	GRATARP,
} rma_ctrl_t;

typedef enum {
	FORWARD = 0,
	DROP,
	TRAP2CPU,
	FLOODALL,
	TRAP2MASTERCPU,
	COPY2CPU,
} action_type_t;

#define RTL838X_RMA_BPDU_CTRL			(0x4330)
#define RTL839X_RMA_BPDU_CTRL			(0x122C)
#define RTL930X_RMA_BPDU_CTRL			(0x9E7C)
#define RTL931X_RMA_BPDU_CTRL			(0x881C)

#define RTL838X_RMA_PTP_CTRL			(0x4338)
#define RTL839X_RMA_PTP_CTRL			(0x123C)
#define RTL930X_RMA_PTP_CTRL			(0x9E88)
#define RTL931X_RMA_PTP_CTRL			(0x8834)

#define RTL838X_RMA_LLDP_CTRL			(0x4340)
#define RTL839X_RMA_LLDP_CTRL			(0x124C)
#define RTL930X_RMA_LLDP_CTRL			(0x9EFC)
#define RTL931X_RMA_LLDP_CTRL			(0x8918)

#define RTL930X_RMA_EAPOL_CTRL			(0x9F08)
#define RTL931X_RMA_EAPOL_CTRL			(0x8930)
#define RTL931X_TRAP_ARP_GRAT_PORT_ACT		(0x8C04)

/* QoS */
#define RTL838X_QM_INTPRI2QID_CTRL		(0x5F00)
#define RTL839X_QM_INTPRI2QID_CTRL(q)		(0x1110 + (q << 2))
#define RTL839X_QM_PORT_QNUM(p)			(0x1130 + (((p / 10) << 2)))
#define RTL838X_PRI_SEL_PORT_PRI(p)		(0x5FB8 + (((p / 10) << 2)))
#define RTL839X_PRI_SEL_PORT_PRI(p)		(0x10A8 + (((p / 10) << 2)))
#define RTL838X_QM_PKT2CPU_INTPRI_MAP		(0x5F10)
#define RTL839X_QM_PKT2CPU_INTPRI_MAP		(0x1154)
#define RTL838X_PRI_SEL_CTRL			(0x10E0)
#define RTL839X_PRI_SEL_CTRL			(0x10E0)
#define RTL838X_PRI_SEL_TBL_CTRL(i)		(0x5FD8 + (((i) << 2)))
#define RTL839X_PRI_SEL_TBL_CTRL(i)		(0x10D0 + (((i) << 2)))
#define RTL838X_QM_PKT2CPU_INTPRI_0		(0x5F04)
#define RTL838X_QM_PKT2CPU_INTPRI_1		(0x5F08)
#define RTL838X_QM_PKT2CPU_INTPRI_2		(0x5F0C)
#define RTL839X_OAM_CTRL			(0x2100)
#define RTL839X_OAM_PORT_ACT_CTRL(p)	 	(0x2104 + (((p) << 2)))
#define RTL839X_RMK_PORT_DEI_TAG_CTRL(p)	(0x6A9C + (((p >> 5) << 2)))
#define RTL839X_PRI_SEL_IPRI_REMAP		(0x1080)
#define RTL838X_PRI_SEL_IPRI_REMAP		(0x5F8C)
#define RTL839X_PRI_SEL_DEI2DP_REMAP		(0x10EC)
#define RTL839X_PRI_SEL_DSCP2DP_REMAP_ADDR(i)	(0x10F0 + (((i >> 4) << 2)))
#define RTL839X_RMK_DEI_CTRL			(0x6AA4)
#define RTL839X_WRED_PORT_THR_CTRL(i)		(0x6084 + ((i) << 2))
#define RTL839X_WRED_QUEUE_THR_CTRL(q, i) 	(0x6090 + ((q) * 12) + ((i) << 2))
#define RTL838X_PRI_DSCP_INVLD_CTRL0		(0x5FE8)
#define RTL838X_RMK_IPRI_CTRL			(0xA460)
#define RTL838X_RMK_OPRI_CTRL			(0xA464)
#define RTL838X_SCHED_P_TYPE_CTRL(p)		(0xC04C + (((p) << 7)))
#define RTL838X_SCHED_LB_CTRL(p)		(0xC004 + (((p) << 7)))
#define RTL838X_FC_P_EGR_DROP_CTRL(p)		(0x6B1C + (((p) << 2)))

/* Debug features */
#define RTL930X_STAT_PRVTE_DROP_COUNTER0	(0xB5B8)

/* Packet Inspection Engine */
#define RTL838X_METER_GLB_CTRL			(0x4B08)
#define RTL839X_METER_GLB_CTRL			(0x1300)
#define RTL930X_METER_GLB_CTRL			(0xa0a0)
#define RTL931X_METER_GLB_CTRL			(0x411C)

#define RTL839X_ACL_CTRL			(0x1288)

#define RTL838X_ACL_BLK_LOOKUP_CTRL		(0x6100)
#define RTL839X_ACL_BLK_LOOKUP_CTRL		(0x1280)
#define RTL930X_PIE_BLK_LOOKUP_CTRL		(0xa5a0)
#define RTL931X_PIE_BLK_LOOKUP_CTRL		(0x4180)

#define RTL838X_ACL_BLK_PWR_CTRL		(0x6104)
#define RTL839X_PS_ACL_PWR_CTRL			(0x049c)

#define RTL838X_ACL_BLK_TMPLTE_CTRL(block)	(0x6108 + ((block) << 2))
#define RTL839X_ACL_BLK_TMPLTE_CTRL(block)	(0x128c + ((block) << 2))
#define RTL930X_PIE_BLK_TMPLTE_CTRL(block)	(0xa624 + ((block) << 2))
#define RTL931X_PIE_BLK_TMPLTE_CTRL(block)	(0x4214 + ((block) << 2))

#define RTL838X_ACL_BLK_GROUP_CTRL		(0x615C)
#define RTL839X_ACL_BLK_GROUP_CTRL		(0x12ec)

#define RTL838X_ACL_CLR_CTRL			(0x6168)
#define RTL839X_ACL_CLR_CTRL			(0x12fc)
#define RTL930X_PIE_CLR_CTRL			(0xa66c)
#define RTL931X_PIE_CLR_CTRL			(0x42D8)

#define RTL838X_DMY_REG27			(0x3378)

#define RTL838X_ACL_PORT_LOOKUP_CTRL(p)		(0x616C + (((p) << 2)))
#define RTL930X_ACL_PORT_LOOKUP_CTRL(p)		(0xA784 + (((p) << 2)))
#define RTL931X_ACL_PORT_LOOKUP_CTRL(p)		(0x44F8 + (((p) << 2)))

#define RTL930X_PIE_BLK_PHASE_CTRL		(0xA5A4)
#define RTL931X_PIE_BLK_PHASE_CTRL		(0x4184)

/* PIE actions */
#define PIE_ACT_COPY_TO_PORT	2
#define PIE_ACT_REDIRECT_TO_PORT 4
#define PIE_ACT_ROUTE_UC	6
#define PIE_ACT_VID_ASSIGN	0

/* L3 actions */
#define L3_FORWARD		0
#define L3_DROP			1
#define L3_TRAP2CPU		2
#define L3_COPY2CPU		3
#define L3_TRAP2MASTERCPU	4
#define L3_COPY2MASTERCPU	5
#define L3_HARDDROP		6

/* Route actions */
#define ROUTE_ACT_FORWARD	0
#define ROUTE_ACT_TRAP2CPU	1
#define ROUTE_ACT_COPY2CPU	2
#define ROUTE_ACT_DROP		3

/* L3 Routing */
#define RTL839X_ROUTING_SA_CTRL 		0x6afc
#define RTL930X_L3_HOST_TBL_CTRL		(0xAB48)
#define RTL930X_L3_IPUC_ROUTE_CTRL		(0xAB4C)
#define RTL930X_L3_IP6UC_ROUTE_CTRL		(0xAB50)
#define RTL930X_L3_IPMC_ROUTE_CTRL		(0xAB54)
#define RTL930X_L3_IP6MC_ROUTE_CTRL		(0xAB58)
#define RTL930X_L3_IP_MTU_CTRL(i)		(0xAB5C + ((i >> 1) << 2))
#define RTL930X_L3_IP6_MTU_CTRL(i)		(0xAB6C + ((i >> 1) << 2))
#define RTL930X_L3_HW_LU_KEY_CTRL		(0xAC9C)
#define RTL930X_L3_HW_LU_KEY_IP_CTRL		(0xACA0)
#define RTL930X_L3_HW_LU_CTRL			(0xACC0)
#define RTL930X_L3_IP_ROUTE_CTRL		0xab44

/* Port LED Control */
#define RTL930X_LED_PORT_NUM_CTRL(p)		(0xCC04 + (((p >> 4) << 2)))
#define RTL930X_LED_SET0_0_CTRL			(0xCC28)
#define RTL930X_LED_PORT_COPR_SET_SEL_CTRL(p)	(0xCC2C + (((p >> 4) << 2)))
#define RTL930X_LED_PORT_FIB_SET_SEL_CTRL(p)	(0xCC34 + (((p >> 4) << 2)))
#define RTL930X_LED_PORT_COPR_MASK_CTRL		(0xCC3C)
#define RTL930X_LED_PORT_FIB_MASK_CTRL		(0xCC40)
#define RTL930X_LED_PORT_COMBO_MASK_CTRL	(0xCC44)

#define RTL931X_LED_PORT_NUM_CTRL(p)		(0x0604 + (((p >> 4) << 2)))
#define RTL931X_LED_SET0_0_CTRL			(0x0630)
#define RTL931X_LED_PORT_COPR_SET_SEL_CTRL(p)	(0x0634 + (((p >> 4) << 2)))
#define RTL931X_LED_PORT_FIB_SET_SEL_CTRL(p)	(0x0644 + (((p >> 4) << 2)))
#define RTL931X_LED_PORT_COPR_MASK_CTRL		(0x0654)
#define RTL931X_LED_PORT_FIB_MASK_CTRL		(0x065c)
#define RTL931X_LED_PORT_COMBO_MASK_CTRL	(0x0664)

#define MAX_VLANS 4096
#define MAX_LAGS 16
#define MAX_PRIOS 8
#define RTL930X_PORT_IGNORE 0x3f
#define MAX_MC_GROUPS 512
#define UNKNOWN_MC_PMASK (MAX_MC_GROUPS - 1)
#define PIE_BLOCK_SIZE 128
#define MAX_PIE_ENTRIES (18 * PIE_BLOCK_SIZE)
#define N_FIXED_FIELDS 12
#define N_FIXED_FIELDS_RTL931X 14
#define MAX_COUNTERS 2048
#define MAX_ROUTES 512
#define MAX_HOST_ROUTES 1536
#define MAX_INTF_MTUS 8
#define DEFAULT_MTU 1536
#define MAX_INTERFACES 100
#define MAX_ROUTER_MACS 64
#define L3_EGRESS_DMACS 2048
#define MAX_SMACS 64

enum phy_type {
	PHY_NONE = 0,
	PHY_RTL838X_SDS = 1,
	PHY_RTL8218B_INT = 2,
	PHY_RTL8218B_EXT = 3,
	PHY_RTL8214FC = 4,
	PHY_RTL839X_SDS = 5,
	PHY_RTL930X_SDS = 6,
};

enum pbvlan_type {
	PBVLAN_TYPE_INNER = 0,
	PBVLAN_TYPE_OUTER,
};

enum pbvlan_mode {
	PBVLAN_MODE_UNTAG_AND_PRITAG = 0,
	PBVLAN_MODE_UNTAG_ONLY,
	PBVLAN_MODE_ALL_PKT,
};

struct rtl838x_port {
	bool enable;
	u64 pm;
	u16 pvid;
	bool eee_enabled;
	enum phy_type phy;
	bool phy_is_integrated;
	bool is10G;
	bool is2G5;
	int sds_num;
	int led_set;
	int leds_on_this_port;
	const struct dsa_port *dp;
};

struct rtl838x_pcs {
	struct phylink_pcs pcs;
	struct rtl838x_switch_priv *priv;
	int port;
};

struct rtl838x_vlan_info {
	u64 untagged_ports;
	u64 tagged_ports;
	u8 profile_id;
	bool hash_mc_fid;
	bool hash_uc_fid;
	u8 fid; /* AKA MSTI */

	/* The following fields are used only by the RTL931X */
	int if_id;		/* Interface (index in L3_EGR_INTF_IDX) */
	u16 multicast_grp_mask;
	int l2_tunnel_list_id;
};

enum l2_entry_type {
	L2_INVALID = 0,
	L2_UNICAST = 1,
	L2_MULTICAST = 2,
	IP4_MULTICAST = 3,
	IP6_MULTICAST = 4,
};

struct rtl838x_l2_entry {
	u8 mac[6];
	u16 vid;
	u16 rvid;
	u8 port;
	bool valid;
	enum l2_entry_type type;
	bool is_static;
	bool is_ip_mc;
	bool is_ipv6_mc;
	bool block_da;
	bool block_sa;
	bool suspended;
	bool next_hop;
	int age;
	u8 trunk;
	bool is_trunk;
	u8 stack_dev;
	u16 mc_portmask_index;
	u32 mc_gip;
	u32 mc_sip;
	u16 mc_mac_index;
	u16 nh_route_id;
	bool nh_vlan_target;  /* Only RTL83xx: VLAN used for next hop */

	/* The following is only valid on RTL931x */
	bool is_open_flow;
	bool is_pe_forward;
	bool is_local_forward;
	bool is_remote_forward;
	bool is_l2_tunnel;
	int l2_tunnel_id;
	int l2_tunnel_list_id;
};

enum fwd_rule_action {
	FWD_RULE_ACTION_NONE = 0,
	FWD_RULE_ACTION_FWD = 1,
};

enum pie_phase {
	PHASE_VACL = 0,
	PHASE_IACL = 1,
};

enum igr_filter {
	IGR_FORWARD = 0,
	IGR_DROP = 1,
	IGR_TRAP = 2,
};

enum egr_filter {
	EGR_DISABLE = 0,
	EGR_ENABLE = 1,
};

/* Intermediate representation of a  Packet Inspection Engine Rule
 * as suggested by the Kernel's tc flower offload subsystem
 * Field meaning is universal across SoC families, but data content is specific
 * to SoC family (e.g. because of different port ranges) */
struct pie_rule {
	int id;
	enum pie_phase phase;	/* Phase in which this template is applied */
	int packet_cntr;	/* ID of a packet counter assigned to this rule */
	int octet_cntr;		/* ID of a byte counter assigned to this rule */
	u32 last_packet_cnt;
	u64 last_octet_cnt;

	/* The following are requirements for the pie template */
	bool is_egress;
	bool is_ipv6;		/* This is a rule with IPv6 fields */

	/* Fixed fields that are always matched against on RTL8380 */
	u8 spmmask_fix;
	u8 spn;			/* Source port number */
	bool stacking_port;	/* Source port is stacking port */
	bool mgnt_vlan;		/* Packet arrived on management VLAN */
	bool dmac_hit_sw;	/* The packet's destination MAC matches one of the device's */
	bool content_too_deep;	/* The content of the packet cannot be parsed: too many layers */
	bool not_first_frag;	/* Not the first IP fragment */
	u8 frame_type_l4;	/* 0: UDP, 1: TCP, 2: ICMP/ICMPv6, 3: IGMP */
	u8 frame_type;		/* 0: ARP, 1: L2 only, 2: IPv4, 3: IPv6 */
	bool otag_fmt;		/* 0: outer tag packet, 1: outer priority tag or untagged */
	bool itag_fmt;		/* 0: inner tag packet, 1: inner priority tag or untagged */
	bool otag_exist;	/* packet with outer tag */
	bool itag_exist;	/* packet with inner tag */
	bool frame_type_l2;	/* 0: Ethernet, 1: LLC_SNAP, 2: LLC_Other, 3: Reserved */
	bool igr_normal_port;	/* Ingress port is not cpu or stacking port */
	u8 tid;			/* The template ID defining the what the templated fields mean */

	/* Masks for the fields that are always matched against on RTL8380 */
	u8 spmmask_fix_m;
	u8 spn_m;
	bool stacking_port_m;
	bool mgnt_vlan_m;
	bool dmac_hit_sw_m;
	bool content_too_deep_m;
	bool not_first_frag_m;
	u8 frame_type_l4_m;
	u8 frame_type_m;
	bool otag_fmt_m;
	bool itag_fmt_m;
	bool otag_exist_m;
	bool itag_exist_m;
	bool frame_type_l2_m;
	bool igr_normal_port_m;
	u8 tid_m;

	/* Logical operations between rules, special rules for rule numbers apply */
	bool valid;
	bool cond_not;		/* Matches when conditions not match */
	bool cond_and1;		/* And this rule 2n with the next rule 2n+1 in same block */
	bool cond_and2;		/* And this rule m in block 2n with rule m in block 2n+1 */
	bool ivalid;

	/* Actions to be performed */
	bool drop;		/* Drop the packet */
	bool fwd_sel;		/* Forward packet: to port, portmask, dest route, next rule, drop */
	bool ovid_sel;		/* So something to outer vlan-id: shift, re-assign */
	bool ivid_sel;		/* Do something to inner vlan-id: shift, re-assign */
	bool flt_sel;		/* Filter the packet when sending to certain ports */
	bool log_sel;		/* Log the packet in one of the LOG-table counters */
	bool rmk_sel;		/* Re-mark the packet, i.e. change the priority-tag */
	bool meter_sel;		/* Meter the packet, i.e. limit rate of this type of packet */
	bool tagst_sel;		/* Change the ergress tag */
	bool mir_sel;		/* Mirror the packet to a Link Aggregation Group */
	bool nopri_sel;		/* Change the normal priority */
	bool cpupri_sel;	/* Change the CPU priority */
	bool otpid_sel;		/* Change Outer Tag Protocol Identifier (802.1q) */
	bool itpid_sel;		/* Change Inner Tag Protocol Identifier (802.1q) */
	bool shaper_sel;	/* Apply traffic shaper */
	bool mpls_sel;		/* MPLS actions */
	bool bypass_sel;	/* Bypass actions */
	bool fwd_sa_lrn;	/* Learn the source address when forwarding */
	bool fwd_mod_to_cpu;	/* Forward the modified VLAN tag format to CPU-port */

	/* Fields used in predefined templates 0-2 on RTL8380 / 90 / 9300 */
	u64 spm;		/* Source Port Matrix */
	u16 otag;		/* Outer VLAN-ID */
	u8 smac[ETH_ALEN];	/* Source MAC address */
	u8 dmac[ETH_ALEN];	/* Destination MAC address */
	u16 ethertype;		/* Ethernet frame type field in ethernet header */
	u16 itag;		/* Inner VLAN-ID */
	u16 field_range_check;
	u32 sip;		/* Source IP */
	struct in6_addr sip6;	/* IPv6 Source IP */
	u32 dip;		/* Destination IP */
	struct in6_addr dip6;	/* IPv6 Destination IP */
	u16 tos_proto;		/* IPv4: TOS + Protocol fields, IPv6: Traffic class + next header */
	u16 sport;		/* TCP/UDP source port */
	u16 dport;		/* TCP/UDP destination port */
	u16 icmp_igmp;
	u16 tcp_info;
	u16 dsap_ssap;		/* Destination / Source Service Access Point bytes (802.3) */

	u64 spm_m;
	u16 otag_m;
	u8 smac_m[ETH_ALEN];
	u8 dmac_m[ETH_ALEN];
	u8 ethertype_m;
	u16 itag_m;
	u16 field_range_check_m;
	u32 sip_m;
	struct in6_addr sip6_m;	/* IPv6 Source IP mask */
	u32 dip_m;
	struct in6_addr dip6_m;	/* IPv6 Destination IP mask */
	u16 tos_proto_m;
	u16 sport_m;
	u16 dport_m;
	u16 icmp_igmp_m;
	u16 tcp_info_m;
	u16 dsap_ssap_m;

	/* Data associated with actions */
	u8 fwd_act;		/* Type of forwarding action */
				/* 0: permit, 1: drop, 2: copy to port id, 4: copy to portmask */
				/* 4: redirect to portid, 5: redirect to portmask */
				/* 6: route, 7: vlan leaky (only 8380) */
	u16 fwd_data;		/* Additional data for forwarding action, e.g. destination port */
	u8 ovid_act;
	u16 ovid_data;		/* Outer VLAN ID */
	u8 ivid_act;
	u16 ivid_data;		/* Inner VLAN ID */
	u16 flt_data;		/* Filtering data */
	u16 log_data;		/* ID of packet or octet counter in LOG table, on RTL93xx */
				/* unnecessary since PIE-Rule-ID == LOG-counter-ID */
	bool log_octets;
	u8 mpls_act;		/* MPLS action type */
	u16 mpls_lib_idx;	/* MPLS action data */

	u16 rmk_data;		/* Data for remarking */
	u16 meter_data;		/* ID of meter for bandwidth control */
	u16 tagst_data;
	u16 mir_data;
	u16 nopri_data;
	u16 cpupri_data;
	u16 otpid_data;
	u16 itpid_data;
	u16 shaper_data;

	/* Bypass actions, ignored on RTL8380 */
	bool bypass_all;	/* Not clear */
	bool bypass_igr_stp;	/* Bypass Ingress STP state */
	bool bypass_ibc_sc;	/* Bypass Ingress Bandwidth Control and Storm Control */
};

struct rtl838x_l3_intf {
	u16 vid;
	u8 smac_idx;
	u8 ip4_mtu_id;
	u8 ip6_mtu_id;
	u16 ip4_mtu;
	u16 ip6_mtu;
	u8 ttl_scope;
	u8 hl_scope;
	u8 ip4_icmp_redirect;
	u8 ip6_icmp_redirect;
	u8 ip4_pbr_icmp_redirect;
	u8 ip6_pbr_icmp_redirect;
};

/* An entry in the RTL93XX SoC's ROUTER_MAC tables setting up a termination point
 * for the L3 routing system. Packets arriving and matching an entry in this table
 * will be considered for routing.
 * Mask fields state whether the corresponding data fields matter for matching
 */
struct rtl93xx_rt_mac {
	bool valid;	/* Valid or not */
	bool p_type;	/* Individual (0) or trunk (1) port */
	bool p_mask;	/* Whether the port type is used */
	u8 p_id;
	u8 p_id_mask;	/* Mask for the port */
	u8 action;	/* Routing action performed: 0: FORWARD, 1: DROP, 2: TRAP2CPU */
			/*   3: COPY2CPU, 4: TRAP2MASTERCPU, 5: COPY2MASTERCPU, 6: HARDDROP */
	u16 vid;
	u16 vid_mask;
	u64 mac;	/* MAC address used as source MAC in the routed packet */
	u64 mac_mask;
};

struct rtl83xx_nexthop {
	u16 id;		/* ID: L3_NEXT_HOP table-index or route-index set in L2_NEXT_HOP */
	u32 dev_id;
	u16 port;
	u16 vid;	/* VLAN-ID for L2 table entry (saved from L2-UC entry) */
	u16 rvid;	/* Relay VID/FID for the L2 table entry */
	u64 mac;	/* The MAC address of the entry in the L2_NEXT_HOP table */
	u16 mac_id;
	u16 l2_id;	/* Index of this next hop forwarding entry in L2 FIB table */
	u64 gw;		/* The gateway MAC address packets are forwarded to */
	int if_id;	/* Interface (into L3_EGR_INTF_IDX) */
};

struct rtl838x_switch_priv;

struct rtl83xx_flow {
	unsigned long cookie;
	struct rhash_head node;
	struct rcu_head rcu_head;
	struct rtl838x_switch_priv *priv;
	struct pie_rule rule;
	u32 flags;
};

struct rtl93xx_route_attr {
	bool valid;
	bool hit;
	bool ttl_dec;
	bool ttl_check;
	bool dst_null;
	bool qos_as;
	u8 qos_prio;
	u8 type;
	u8 action;
};

struct rtl83xx_route {
	u32 gw_ip;			/* IP of the route's gateway */
	u32 dst_ip;			/* IP of the destination net */
	struct in6_addr dst_ip6;
	int prefix_len;			/* Network prefix len of the destination net */
	bool is_host_route;
	int id;				/* ID number of this route */
	struct rhlist_head linkage;
	u16 switch_mac_id;		/* Index into switch's own MACs, RTL839X only */
	struct rtl83xx_nexthop nh;
	struct pie_rule pr;
	struct rtl93xx_route_attr attr;
};

struct rtl838x_reg {
	void (*mask_port_reg_be)(u64 clear, u64 set, int reg);
	void (*set_port_reg_be)(u64 set, int reg);
	u64 (*get_port_reg_be)(int reg);
	void (*mask_port_reg_le)(u64 clear, u64 set, int reg);
	void (*set_port_reg_le)(u64 set, int reg);
	u64 (*get_port_reg_le)(int reg);
	int stat_port_rst;
	int stat_rst;
	int stat_port_std_mib;
	int (*port_iso_ctrl)(int p);
	void (*traffic_enable)(int source, int dest);
	void (*traffic_disable)(int source, int dest);
	void (*traffic_set)(int source, u64 dest_matrix);
	u64 (*traffic_get)(int source);
	int l2_ctrl_0;
	int l2_ctrl_1;
	int smi_poll_ctrl;
	u32 l2_port_aging_out;
	int l2_tbl_flush_ctrl;
	void (*exec_tbl0_cmd)(u32 cmd);
	void (*exec_tbl1_cmd)(u32 cmd);
	int (*tbl_access_data_0)(int i);
	int isr_glb_src;
	int isr_port_link_sts_chg;
	int imr_port_link_sts_chg;
	int imr_glb;
	void (*vlan_tables_read)(u32 vlan, struct rtl838x_vlan_info *info);
	void (*vlan_set_tagged)(u32 vlan, struct rtl838x_vlan_info *info);
	void (*vlan_set_untagged)(u32 vlan, u64 portmask);
	void (*vlan_profile_dump)(int index);
	void (*vlan_profile_setup)(int profile);
	void (*vlan_port_pvidmode_set)(int port, enum pbvlan_type type, enum pbvlan_mode mode);
	void (*vlan_port_pvid_set)(int port, enum pbvlan_type type, int pvid);
	void (*vlan_port_keep_tag_set)(int port, bool keep_outer, bool keep_inner);
	void (*set_vlan_igr_filter)(int port, enum igr_filter state);
	void (*set_vlan_egr_filter)(int port, enum egr_filter state);
	void (*enable_learning)(int port, bool enable);
	void (*enable_flood)(int port, bool enable);
	void (*enable_mcast_flood)(int port, bool enable);
	void (*enable_bcast_flood)(int port, bool enable);
	void (*set_static_move_action)(int port, bool forward);
	void (*stp_get)(struct rtl838x_switch_priv *priv, u16 msti, u32 port_state[]);
	void (*stp_set)(struct rtl838x_switch_priv *priv, u16 msti, u32 port_state[]);
	int  (*mac_force_mode_ctrl)(int port);
	int  (*mac_port_ctrl)(int port);
	int  (*l2_port_new_salrn)(int port);
	int  (*l2_port_new_sa_fwd)(int port);
	int (*set_ageing_time)(unsigned long msec);
	int mir_ctrl;
	int mir_dpm;
	int mir_spm;
	int mac_link_sts;
	int mac_link_dup_sts;
	int  (*mac_link_spd_sts)(int port);
	int mac_rx_pause_sts;
	int mac_tx_pause_sts;
	u64 (*read_l2_entry_using_hash)(u32 hash, u32 position, struct rtl838x_l2_entry *e);
	void (*write_l2_entry_using_hash)(u32 hash, u32 pos, struct rtl838x_l2_entry *e);
	u64 (*read_cam)(int idx, struct rtl838x_l2_entry *e);
	void (*write_cam)(int idx, struct rtl838x_l2_entry *e);
	int (*trk_mbr_ctr)(int group);
	int rma_bpdu_fld_pmask;
	int spcl_trap_eapol_ctrl;
	void (*init_eee)(struct rtl838x_switch_priv *priv, bool enable);
	void (*set_mac_eee)(struct rtl838x_switch_priv *priv, int port, bool enable);
	u64 (*l2_hash_seed)(u64 mac, u32 vid);
	u32 (*l2_hash_key)(struct rtl838x_switch_priv *priv, u64 seed);
	u64 (*read_mcast_pmask)(int idx);
	void (*write_mcast_pmask)(int idx, u64 portmask);
	void (*vlan_fwd_on_inner)(int port, bool is_set);
	void (*pie_init)(struct rtl838x_switch_priv *priv);
	int (*pie_rule_read)(struct rtl838x_switch_priv *priv, int idx, struct  pie_rule *pr);
	int (*pie_rule_write)(struct rtl838x_switch_priv *priv, int idx, struct pie_rule *pr);
	int (*pie_rule_add)(struct rtl838x_switch_priv *priv, struct pie_rule *rule);
	void (*pie_rule_rm)(struct rtl838x_switch_priv *priv, struct pie_rule *rule);
	void (*l2_learning_setup)(void);
	u32 (*packet_cntr_read)(int counter);
	void (*packet_cntr_clear)(int counter);
	void (*route_read)(int idx, struct rtl83xx_route *rt);
	void (*route_write)(int idx, struct rtl83xx_route *rt);
	void (*host_route_write)(int idx, struct rtl83xx_route *rt);
	int (*l3_setup)(struct rtl838x_switch_priv *priv);
	void (*set_l3_nexthop)(int idx, u16 dmac_id, u16 interface);
	void (*get_l3_nexthop)(int idx, u16 *dmac_id, u16 *interface);
	u64 (*get_l3_egress_mac)(u32 idx);
	void (*set_l3_egress_mac)(u32 idx, u64 mac);
	int (*find_l3_slot)(struct rtl83xx_route *rt, bool must_exist);
	int (*route_lookup_hw)(struct rtl83xx_route *rt);
	void (*get_l3_router_mac)(u32 idx, struct rtl93xx_rt_mac *m);
	void (*set_l3_router_mac)(u32 idx, struct rtl93xx_rt_mac *m);
	void (*set_l3_egress_intf)(int idx, struct rtl838x_l3_intf *intf);
	void (*set_distribution_algorithm)(int group, int algoidx, u32 algomask);
	void (*set_receive_management_action)(int port, rma_ctrl_t type, action_type_t action);
	void (*led_init)(struct rtl838x_switch_priv *priv);
};

struct rtl838x_switch_priv {
	/* Switch operation */
	struct dsa_switch *ds;
	struct device *dev;
	u16 id;
	u16 family_id;
	char version;
	struct rtl838x_port ports[57];
	struct rtl838x_pcs pcs[57];
	struct mutex reg_mutex;		/* Mutex for individual register manipulations */
	struct mutex pie_mutex;		/* Mutex for Packet Inspection Engine */
	int link_state_irq;
	int mirror_group_ports[4];
	struct mii_bus *parent_bus;
	const struct rtl838x_reg *r;
	u8 cpu_port;
	u8 port_mask;
	u8 port_width;
	u8 port_ignore;
	u64 irq_mask;
	u32 fib_entries;
	int l2_bucket_size;
	struct dentry *dbgfs_dir;
	int n_lags;
	u64 lags_port_members[MAX_LAGS];
	struct net_device *lag_devs[MAX_LAGS];
	u32 lag_primary[MAX_LAGS];
	u32 is_lagmember[57];
	u64 lagmembers;
	struct notifier_block nb;  /* TODO: change to different name */
	struct notifier_block ne_nb;
	struct notifier_block fib_nb;
	bool eee_enabled;
	unsigned long int mc_group_bm[MAX_MC_GROUPS >> 5];
	int n_pie_blocks;
	struct rhashtable tc_ht;
	unsigned long int pie_use_bm[MAX_PIE_ENTRIES >> 5];
	int n_counters;
	unsigned long int octet_cntr_use_bm[MAX_COUNTERS >> 5];
	unsigned long int packet_cntr_use_bm[MAX_COUNTERS >> 4];
	struct rhltable routes;
	unsigned long int route_use_bm[MAX_ROUTES >> 5];
	unsigned long int host_route_use_bm[MAX_HOST_ROUTES >> 5];
	struct rtl838x_l3_intf *interfaces[MAX_INTERFACES];
	u16 intf_mtus[MAX_INTF_MTUS];
	int intf_mtu_count[MAX_INTF_MTUS];
};

void rtl838x_dbgfs_init(struct rtl838x_switch_priv *priv);
void rtl930x_dbgfs_init(struct rtl838x_switch_priv *priv);

#endif /* _RTL838X_H */

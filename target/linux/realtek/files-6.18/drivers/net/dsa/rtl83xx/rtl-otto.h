/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef _RTL838X_H
#define _RTL838X_H

#include <asm/mach-rtl-otto/mach-rtl-otto.h>
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

#define RTL838X_PORT_ISO_CTRL(port)		(0x4100 + ((port) << 2))
#define RTL839X_PORT_ISO_CTRL(port)		(0x1400 + ((port) << 3))

/* Packet statistics */
#define RTL838X_STAT_PORT_STD_MIB		(0x1200)
#define RTL839X_STAT_PORT_STD_MIB		(0xC000)
#define RTL930X_STAT_PORT_MIB_CNTR		(0x0664)
#define RTL930X_STAT_PORT_PRVTE_CNTR		(0x2364)
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

/* VLAN registers */
#define RTL838X_VLAN_CTRL			(0x3A74)
#define RTL838X_VLAN_PROFILE(idx)		(0x3A88 + ((idx) << 2))
#define RTL838X_VLAN_PROFILE_MAX		7
#define RTL838X_VLAN_PORT_EGR_FLTR		(0x3A84)
#define RTL838X_VLAN_PORT_PB_VLAN		(0x3C00)
#define RTL838X_VLAN_PORT_IGR_FLTR		(0x3A7C)

#define RTL838X_VLAN_L2_LEARN_EN(i)		(i)
#define RTL838X_VLAN_L2_UNKN_MC_FLD(pmsk)	(pmsk << 1)
#define RTL838X_VLAN_IP4_UNKN_MC_FLD(pmsk)	(pmsk << 10)
#define RTL838X_VLAN_IP6_UNKN_MC_FLD(pmsk)	(pmsk << 19)

#define RTL838X_VLAN_L2_LEARN_EN_R(p)		(p & RTL838X_VLAN_L2_LEARN_EN(1))
#define RTL838X_VLAN_L2_UNKN_MC_FLD_PMSK(p)	((p >> 1) & (MAX_MC_PMASKS - 1))
#define RTL838X_VLAN_IP4_UNKN_MC_FLD_PMSK(p)	((p >> 10) & (MAX_MC_PMASKS - 1))
#define RTL838X_VLAN_IP6_UNKN_MC_FLD_PMSK(p)	((p >> 19) & (MAX_MC_PMASKS - 1))

#define RTL839X_VLAN_PROFILE(idx)		(0x25C0 + (((idx) << 3)))
#define RTL839X_VLAN_PROFILE_MAX		7
#define RTL839X_VLAN_CTRL			(0x26D4)
#define RTL839X_VLAN_PORT_PB_VLAN		(0x26D8)
#define RTL839X_VLAN_PORT_IGR_FLTR		(0x27B4)
#define RTL839X_VLAN_PORT_EGR_FLTR		(0x27C4)

#define RTL839X_VLAN_L2_LEARN_EN(i)		(i)
#define RTL839X_VLAN_L2_UNKN_MC_FLD(pmsk)	(pmsk << 1)
#define RTL839X_VLAN_IP4_UNKN_MC_FLD(pmsk)	(pmsk << 13)
#define RTL839X_VLAN_IP6_UNKN_MC_FLD(pmsk)	(pmsk)

#define RTL839X_VLAN_L2_LEARN_EN_R(p)		(p[1] & RTL839X_VLAN_L2_LEARN_EN(1))
#define RTL839X_VLAN_L2_UNKN_MC_FLD_PMSK(p)	((p[1] >> 1) & (MAX_MC_PMASKS - 1))
#define RTL839X_VLAN_IP4_UNKN_MC_FLD_PMSK(p)	((p[1] >> 13) & (MAX_MC_PMASKS - 1))
#define RTL839X_VLAN_IP6_UNKN_MC_FLD_PMSK(p)	((p[0]) & (MAX_MC_PMASKS - 1))

#define RTL930X_VLAN_PROFILE_SET(idx)		(0x9c60 + (((idx) * 20)))
#define RTL930X_VLAN_PROFILE_MAX		7
#define RTL930X_VLAN_CTRL			(0x82D4)
#define RTL930X_VLAN_PORT_PB_VLAN		(0x82D8)
#define RTL930X_VLAN_PORT_IGR_FLTR		(0x83C0)
#define RTL930X_VLAN_PORT_EGR_FLTR		(0x83C8)

#define RTL930X_VLAN_L2_UNKN_MC_FLD(pmsk)	(pmsk)
#define RTL930X_VLAN_IP4_UNKN_MC_FLD(pmsk)	(pmsk)
#define RTL930X_VLAN_IP6_UNKN_MC_FLD(pmsk)	(pmsk)

#define RTL930X_VLAN_L2_LEARN_EN_R(p)		(p[0] & (3 << 21))
#define RTL930X_VLAN_L2_UNKN_MC_FLD_PMSK(p)	(p[2] & RTL930X_MC_PMASK_ALL_PORTS)
#define RTL930X_VLAN_IP4_UNKN_MC_FLD_PMSK(p)	(p[3] & RTL930X_MC_PMASK_ALL_PORTS)
#define RTL930X_VLAN_IP6_UNKN_MC_FLD_PMSK(p)	(p[4] & RTL930X_MC_PMASK_ALL_PORTS)

#define RTL931X_VLAN_PROFILE_SET(idx)		(0x9800 + (((idx) * 28)))
#define RTL931X_VLAN_PROFILE_MAX		15
#define RTL931X_VLAN_CTRL			(0x94E4)
#define RTL931X_VLAN_PORT_IGR_CTRL		(0x94E8)
#define RTL931X_VLAN_PORT_IGR_FLTR		(0x96B4)
#define RTL931X_VLAN_PORT_EGR_FLTR		(0x96C4)

#define RTL931X_VLAN_L2_UNKN_MC_FLD_H(pmsk)	(((u64)pmsk) >> 32)
#define RTL931X_VLAN_L2_UNKN_MC_FLD_L(pmsk)	(pmsk & GENMASK_ULL(31, 0))
#define RTL931X_VLAN_IP4_UNKN_MC_FLD_H(pmsk)	(((u64)pmsk) >> 32)
#define RTL931X_VLAN_IP4_UNKN_MC_FLD_L(pmsk)	(pmsk & GENMASK_ULL(31, 0))
#define RTL931X_VLAN_IP6_UNKN_MC_FLD_H(pmsk)	(((u64)pmsk) >> 32)
#define RTL931X_VLAN_IP6_UNKN_MC_FLD_L(pmsk)	(pmsk & GENMASK_ULL(31, 0))

#define RTL931X_VLAN_L2_LEARN_EN_R(p)		(p[0] & (3 << 14))
#define RTL931X_VLAN_L2_UNKN_MC_FLD_PMSK(p)	((((u64)p[1]) << 32 | p[2]) & RTL931X_MC_PMASK_ALL_PORTS)
#define RTL931X_VLAN_IP4_UNKN_MC_FLD_PMSK(p)	((((u64)p[3]) << 32 | p[4]) & RTL931X_MC_PMASK_ALL_PORTS)
#define RTL931X_VLAN_IP6_UNKN_MC_FLD_PMSK(p)	((((u64)p[5]) << 32 | p[6]) & RTL931X_MC_PMASK_ALL_PORTS)

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

/* MAC link state bits */
#define RTL_SPEED_10				0
#define RTL_SPEED_100				1
#define RTL_SPEED_1000				2
#define RTL_SPEED_2500				5
#define RTL_SPEED_5000				6
#define RTL_SPEED_10000				4

#define RTL83XX_FORCE_EN			BIT(0)
#define RTL83XX_FORCE_LINK_EN			BIT(1)

#define RTL838X_NWAY_EN				BIT(2)
#define RTL838X_DUPLEX_MODE			BIT(3)
#define RTL838X_SPEED_SHIFT			(4)
#define RTL838X_SPEED_MASK			(3 << RTL838X_SPEED_SHIFT)
#define RTL838X_TX_PAUSE_EN			BIT(6)
#define RTL838X_RX_PAUSE_EN			BIT(7)
#define RTL838X_MAC_FORCE_FC_EN			BIT(8)

#define RTL839X_DUPLEX_MODE			BIT(2)
#define RTL839X_SPEED_SHIFT			(3)
#define RTL839X_SPEED_MASK			(3 << RTL839X_SPEED_SHIFT)
#define RTL839X_TX_PAUSE_EN			BIT(5)
#define RTL839X_RX_PAUSE_EN			BIT(6)
#define RTL839X_MAC_FORCE_FC_EN			BIT(7)

#define RTL930X_FORCE_EN			BIT(0)
#define RTL930X_FORCE_LINK_EN			BIT(1)
#define RTL930X_DUPLEX_MODE			BIT(2)
#define RTL930X_SPEED_SHIFT			(3)
#define RTL930X_SPEED_MASK			(15 << RTL930X_SPEED_SHIFT)
#define RTL930X_TX_PAUSE_EN			BIT(7)
#define RTL930X_RX_PAUSE_EN			BIT(8)
#define RTL930X_MAC_FORCE_FC_EN			BIT(9)

#define RTL931X_FORCE_EN			BIT(9)
#define RTL931X_FORCE_LINK_EN			BIT(0)
#define RTL931X_DUPLEX_MODE			BIT(2)
#define RTL931X_MAC_FORCE_FC_EN			BIT(4)
#define RTL931X_TX_PAUSE_EN			BIT(16)
#define RTL931X_RX_PAUSE_EN			BIT(17)

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

#define RTL838X_L2_BC_FLD(pmsk)			(pmsk << 9)
#define RTL838X_L2_UNKN_UC_FLD(pmsk)		(pmsk)
#define RTL839X_L2_BC_FLD(pmsk)			(pmsk << 12)
#define RTL839X_L2_UNKN_UC_FLD(pmsk)		(pmsk)

#define RTL838X_L2_LRN_CONSTRT_EN		(0x3368)
#define RTL838X_L2_PORT_LRN_CONSTRT		(0x32A0)
#define RTL839X_L2_PORT_LRN_CONSTRT		(0x3914)
#define RTL930X_L2_LRN_PORT_CONSTRT_CTRL	(0x90A4)
#define RTL931X_L2_LRN_PORT_CONSTRT_CTRL	(0xC96C)

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

#define RTL838X_VLAN_STP_CTRL			(0x3b20)
#define RTL839X_ST_CTRL				(0x27e4)
#define RTL930X_ST_CTRL				(0x8798)
#define RTL931X_ST_CTRL				(0x8000)

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
#define TRUNK_DISTRIBUTION_ALGO_SPA_BIT         BIT(0)
#define TRUNK_DISTRIBUTION_ALGO_SMAC_BIT        BIT(1)
#define TRUNK_DISTRIBUTION_ALGO_DMAC_BIT        BIT(2)

#define TRUNK_DISTRIBUTION_ALGO_SIP_BIT         BIT(3)
#define TRUNK_DISTRIBUTION_ALGO_DIP_BIT         BIT(4)
#define TRUNK_DISTRIBUTION_ALGO_SRC_L4PORT_BIT  BIT(5)
#define TRUNK_DISTRIBUTION_ALGO_DST_L4PORT_BIT  BIT(6)
#define TRUNK_DISTRIBUTION_ALGO_VLAN_BIT        BIT(7)
#define TRUNK_DISTRIBUTION_ALGO_MASKALL		GENMASK(6, 0)

#define TRUNK_DISTRIBUTION_ALGO_L2_SPA_BIT         BIT(0)
#define TRUNK_DISTRIBUTION_ALGO_L2_SMAC_BIT        BIT(1)
#define TRUNK_DISTRIBUTION_ALGO_L2_DMAC_BIT        BIT(2)
#define TRUNK_DISTRIBUTION_ALGO_L2_VLAN_BIT        BIT(3)

#define TRUNK_DISTRIBUTION_ALGO_L3_SPA_BIT         BIT(4 + 0)
#define TRUNK_DISTRIBUTION_ALGO_L3_SMAC_BIT        BIT(4 + 1)
#define TRUNK_DISTRIBUTION_ALGO_L3_DMAC_BIT        BIT(4 + 2)
#define TRUNK_DISTRIBUTION_ALGO_L3_VLAN_BIT        BIT(4 + 3)
#define TRUNK_DISTRIBUTION_ALGO_L3_SIP_BIT         BIT(4 + 4)
#define TRUNK_DISTRIBUTION_ALGO_L3_DIP_BIT         BIT(4 + 5)
#define TRUNK_DISTRIBUTION_ALGO_L3_SRC_L4PORT_BIT  BIT(4 + 6)
#define TRUNK_DISTRIBUTION_ALGO_L3_DST_L4PORT_BIT  BIT(4 + 7)
#define TRUNK_DISTRIBUTION_ALGO_L3_PROTO_BIT       BIT(4 + 8)
#define TRUNK_DISTRIBUTION_ALGO_L3_FLOW_LABEL_BIT  BIT(4 + 9)

#define RTL838X_TRK_MBR_CTR			(0x3E00)
#define RTL838X_TRK_HASH_IDX_CTRL		(0x3E20)
#define RTL838X_TRK_HASH_CTRL			(0x3E24)

#define RTL839X_TRK_MBR_CTR			(0x2200)
#define RTL839X_TRK_HASH_IDX_CTRL		(0x2280)
#define RTL839X_TRK_HASH_CTRL			(0x2284)

#define RTL930X_LOCAL_PORT_TRK_MAP		(0xD0C8)
#define RTL930X_TRK_ID_CTRL			(0xA3A8)
#define RTL930X_TRK_MBR_CTRL			(0xA41C)
#define RTL930X_TRK_HASH_CTRL			(0x9F80)
#define RTL930X_TRK_CTRL			(0x9F88)
#define RTL930X_TRK_SHIFT_CTRL			(0x9F8C)
#define RTL930X_TRK_LOCAL_TBL_REFRESH		(0x9F90)
#define RTL930X_TRK_LOCAL_TBL			(0x9F94)
#define RTL930X_TRK_STK_CTRL			(0xA07C)

#define RTL930X_TRK_ID_CTRL_TRK_VALID		BIT(6)
#define RTL930X_TRK_ID_CTRL_TRK_ID		GENMASK(5, 0)

#define RTL930X_LOCAL_PORT_TRK_MAP_IS_TRK_MBR	BIT(6)
#define RTL930X_LOCAL_PORT_TRK_MAP_TRK_ID	GENMASK(5, 0)

#define RTL930X_SRC_TRK_MAP_TRK_VALID		BIT(31)
#define RTL930X_SRC_TRK_MAP_TRK_ID		GENMASK(30, 25)

#define RTL931X_LOCAL_PORT_TRK_MAP		(0x4CAC)
#define RTL931X_TRK_ID_CTRL			(0xB800)
#define RTL931X_TRK_MBR_CTRL			(0xB8D0)
#define RTL931X_TRK_HASH_CTRL			(0xBA70)
#define RTL931X_TRK_CTRL			(0xBA78)
#define RTL931X_TRK_SHIFT_CTRL			(0xBA7C)
#define RTL931X_TRK_LOCAL_TBL_REFRESH		(0xBA80)
#define RTL931X_TRK_LOCAL_TBL			(0xBA84)
#define RTL931X_TRK_STK_CTRL			(0xBE94)

#define RLT931X_TRK_ID_CTRL_TRK_ID		GENMASK(6, 0)
#define RTL931X_TRK_ID_CTRL_TRK_VALID		BIT(7)

#define RTL931X_LOCAL_PORT_TRK_MAP_IS_TRK_MBR	BIT(7)
#define RTL931X_LOCAL_PORT_TRK_MAP_TRK_ID	GENMASK(6, 0)

#define RTL931X_SRC_TRK_MAP_TRK_ID		GENMASK(30, 24)
#define RTL931X_SRC_TRK_MAP_TRK_ID_VALID	BIT(31)

#define GENMASK_MOD(high, low)			GENMASK((high) % 32, (low) % 32)
#define BIT_MOD(bit)				BIT((bit) % 32)

/* RTL930X LAG Table offsets */
#define RTL930X_LAG_NUM_TX_CANDI		GENMASK_MOD(92, 89)
#define RTL930X_LAG_L2_HASH_MSK_IDX		BIT_MOD(88)
#define RTL930X_LAG_IP4_HASH_MSK_IDX		BIT_MOD(87)
#define RTL930X_LAG_IP6_HASH_MSK_IDX		BIT_MOD(86)
#define RTL930X_LAG_SEP_DLF_BCAST_EN		BIT_MOD(85)
#define RTL930X_LAG_SEP_KWN_MC_EN		BIT_MOD(84)
#define RTL930X_LAG_TRK_DEV7			GENMASK_MOD(83, 80)
#define RTL930X_LAG_TRK_PORT7			GENMASK_MOD(79, 74)
#define RTL930X_LAG_TRK_DEV6			GENMASK_MOD(73, 70)
#define RTL930X_LAG_TRK_PORT6			GENMASK_MOD(69, 64)

#define RTL930X_LAG_TRK_DEV5			GENMASK_MOD(61, 58)
#define RTL930X_LAG_TRK_PORT5			GENMASK_MOD(57, 52)
#define RTL930X_LAG_TRK_DEV4			GENMASK_MOD(51, 48)
#define RTL930X_LAG_TRK_PORT4			GENMASK_MOD(47, 42)
#define RTL930X_LAG_TRK_DEV3			GENMASK_MOD(41, 38)
#define RTL930X_LAG_TRK_PORT3			GENMASK_MOD(37, 32)

#define RTL930X_LAG_TRK_DEV2			GENMASK_MOD(29, 26)
#define RTL930X_LAG_TRK_PORT2			GENMASK_MOD(25, 20)
#define RTL930X_LAG_TRK_DEV1			GENMASK_MOD(19, 16)
#define RTL930X_LAG_TRK_PORT1			GENMASK_MOD(15, 10)
#define RTL930X_LAG_TRK_DEV0			GENMASK_MOD(9, 6)
#define RTL930X_LAG_TRK_PORT0			GENMASK_MOD(5, 0)

/* RTL931X LAG Table offsets */
#define RTL931X_LAG_NUM_TX_CANDI		GENMASK_MOD(92, 89)
#define RTL931X_LAG_L2_HASH_MSK_IDX		BIT_MOD(88)
#define RTL931X_LAG_IP4_HASH_MSK_IDX		BIT_MOD(87)
#define RTL931X_LAG_IP6_HASH_MSK_IDX		BIT_MOD(86)
#define RTL931X_LAG_SEP_FLOOD_EN		BIT_MOD(85)
#define RTL931X_LAG_SEP_KWN_MC_EN		BIT_MOD(84)
#define RTL931X_LAG_TRK_DEV7			GENMASK_MOD(83, 80)
#define RTL931X_LAG_TRK_PORT7			GENMASK_MOD(79, 74)
#define RTL931X_LAG_TRK_DEV6			GENMASK_MOD(73, 70)
#define RTL931X_LAG_TRK_PORT6			GENMASK_MOD(69, 64)

#define RTL931X_LAG_TRK_DEV5			GENMASK_MOD(61, 58)
#define RTL931X_LAG_TRK_PORT5			GENMASK_MOD(57, 52)
#define RTL931X_LAG_TRK_DEV4			GENMASK_MOD(51, 48)
#define RTL931X_LAG_TRK_PORT4			GENMASK_MOD(47, 42)
#define RTL931X_LAG_TRK_DEV3			GENMASK_MOD(41, 38)
#define RTL931X_LAG_TRK_PORT3			GENMASK_MOD(37, 32)

#define RTL931X_LAG_TRK_DEV2			GENMASK_MOD(29, 26)
#define RTL931X_LAG_TRK_PORT2			GENMASK_MOD(25, 20)
#define RTL931X_LAG_TRK_DEV1			GENMASK_MOD(19, 16)
#define RTL931X_LAG_TRK_PORT1			GENMASK_MOD(15, 10)
#define RTL931X_LAG_TRK_DEV0			GENMASK_MOD(9, 6)
#define RTL931X_LAG_TRK_PORT0			GENMASK_MOD(5, 0)

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

#define RTL930X_BANDWIDTH_CTRL_EGRESS(port)	(0x7660 + (port * 16))
#define RTL930X_BANDWIDTH_CTRL_INGRESS(port)	(0x8068 + (port * 4))
#define RTL930X_BANDWIDTH_CTRL_MAX_BURST	(64 * 1000)
#define RTL930X_BANDWIDTH_CTRL_INGRESS_BURST_HIGH_ON(port) \
						(0x80DC + (port * 8))
#define RTL930X_BANDWIDTH_CTRL_INGRESS_BURST_HIGH_OFF(port) \
						(0x80E0 + (port * 8))
#define RTL930X_BANDWIDTH_CTRL_INGRESS_BURST_MAX \
						GENMASK(30, 0)

#define RTL931X_BANDWIDTH_CTRL_EGRESS(port)	(0x2164 + (port * 8))
#define RTL931X_BANDWIDTH_CTRL_INGRESS(port)	(0xe008 + (port * 8))

#define RTL93XX_BANDWIDTH_CTRL_RATE_MAX		GENMASK(19, 0)
#define RTL93XX_BANDWIDTH_CTRL_ENABLE		BIT(20)
#define RTL931X_BANDWIDTH_CTRL_MAX_BURST	GENMASK(15, 0)

#define RTL930X_INGRESS_FC_CTRL(port)		(0x81CC + ((port / 29) * 4))
#define RTL930X_INGRESS_FC_CTRL_EN(port)	BIT(port % 29)

/* Switch interrupts */
#define RTL838X_IMR_GLB				(0x1100)
#define RTL838X_IMR_PORT_LINK_STS_CHG		(0x1104)
#define RTL838X_ISR_GLB_SRC			(0x1148)
#define RTL838X_ISR_PORT_LINK_STS_CHG		(0x114C)

#define RTL839X_IMR_GLB				(0x0064)
#define RTL839X_IMR_PORT_LINK_STS_CHG		(0x0068)
#define RTL839X_ISR_GLB_SRC			(0x009c)
#define RTL839X_ISR_PORT_LINK_STS_CHG		(0x00a0)

#define RTL930X_IMR_GLB				(0xC628)
#define RTL930X_IMR_PORT_LINK_STS_CHG		(0xC62C)
#define RTL930X_ISR_GLB				(0xC658)
#define RTL930X_ISR_PORT_LINK_STS_CHG		(0xC660)

/* IMR_GLB does not exit on RTL931X */
#define RTL931X_IMR_PORT_LINK_STS_CHG		(0x126C)
#define RTL931X_ISR_GLB_SRC			(0x12B4)
#define RTL931X_ISR_PORT_LINK_STS_CHG		(0x12B8)

#define RTL838X_SMI_GLB_CTRL			(0xa100) /* used by RTL838x EEE setup */

#define RTL838X_LED_GLB_CTRL			(0xA000)
#define RTL839X_LED_GLB_CTRL			(0x00E4)
#define RTL930X_LED_GLB_CTRL			(0xCC00)
#define RTL931X_LED_GLB_CTRL			(0x0600)

/* LED control by switch */
#define RTL838X_LED_MODE_SEL			(0x1004)
#define RTL838X_LED_MODE_CTRL			(0xA004)
#define RTL838X_LED_P_EN_CTRL			(0xA008)

/* LED control by software */
#define RTL838X_LED_SW_CTRL			(0xA00C)
#define RTL838X_LED0_SW_P_EN_CTRL		(0xA010)
#define RTL838X_LED1_SW_P_EN_CTRL		(0xA014)
#define RTL838X_LED2_SW_P_EN_CTRL		(0xA018)
#define RTL838X_LED_SW_P_CTRL			(0xA01C)
#define RTL838X_LED_SW_P_CTRL_PORT(p)		(RTL838X_LED_SW_P_CTRL + (((p) << 2)))

#define RTL93XX_HASH_MASK_INDEX_L2	(0)
#define RTL93XX_HASH_MASK_INDEX_L23	(1)

#define RTL93XX_TRK_CTRL_NON_TMN_TUNNEL_HASH_SEL	BIT(0)
#define RTL93XX_TRK_CTRL_SEP_PORT_SEL			BIT(1)
#define RTL93XX_TRK_CTRL_TRK_STAND_ALONE_MODE		BIT(2)
#define RTL93XX_TRK_CTRL_STK_HASH_CAL			BIT(3)
#define RTL93XX_TRK_CTRL_LOCAL_FIRST			BIT(4)
#define RTL93XX_TRK_CTRL_CONGST_AVOID			BIT(5)
#define RTL93XX_TRK_CTRL_LINK_DOWN_AVOID		BIT(6)

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

enum rtldsa_flood_type {
	RTLDSA_FLOOD_TYPE_FORWARD = 0,
	RTLDSA_FLOOD_TYPE_DROP,
	RTLDSA_FLOOD_TYPE_TRAP2CPU,
	RTLDSA_FLOOD_TYPE_COPY2CPU,
	RTLDSA_FLOOD_TYPE_TRAP2MASTER,
	RTLDSA_FLOOD_TYPE_COPY2MASTER,
};

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
#define RTL930X_SPCL_TRAP_PORT_CTRL		(0xA1A0)
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
#define RTL839X_OAM_PORT_ACT_CTRL(p)		(0x2104 + (((p) << 2)))
#define RTL839X_RMK_PORT_DEI_TAG_CTRL(p)	(0x6A9C + (((p >> 5) << 2)))
#define RTL839X_PRI_SEL_IPRI_REMAP		(0x1080)
#define RTL838X_PRI_SEL_IPRI_REMAP		(0x5F8C)
#define RTL839X_PRI_SEL_DEI2DP_REMAP		(0x10EC)
#define RTL839X_PRI_SEL_DSCP2DP_REMAP_ADDR(i)	(0x10F0 + (((i >> 4) << 2)))
#define RTL839X_RMK_DEI_CTRL			(0x6AA4)
#define RTL839X_WRED_PORT_THR_CTRL(i)		(0x6084 + ((i) << 2))
#define RTL839X_WRED_QUEUE_THR_CTRL(q, i)	(0x6090 + ((q) * 12) + ((i) << 2))
#define RTL838X_PRI_DSCP_INVLD_CTRL0		(0x5FE8)
#define RTL838X_RMK_IPRI_CTRL			(0xA460)
#define RTL838X_RMK_OPRI_CTRL			(0xA464)
#define RTL838X_SCHED_P_TYPE_CTRL(p)		(0xC04C + (((p) << 7)))
#define RTL838X_SCHED_LB_CTRL(p)		(0xC004 + (((p) << 7)))
#define RTL838X_FC_P_EGR_DROP_CTRL(p)		(0x6B1C + (((p) << 2)))

#define RTL930X_REMAP_DSCP(p)			(0x9B04 + (((p) / 10) * 4))
#define RTL931X_REMAP_DSCP(p)			(0x9034 + (((p) / 10) * 4))
#define RTL93XX_REMAP_DSCP_INTPRI_DSCP_OFFSET(p) \
						(((p) % 10) * 3)
#define RTL93XX_REMAP_DSCP_INTPRI_DSCP_MASK(index) \
						(0x7 << RTL93XX_REMAP_DSCP_INTPRI_DSCP_OFFSET(index))

#define RTL930X_PORT_TBL_IDX_CTRL(port)		(0x9B20 + (((port) / 16) * 4))
#define RTL931X_PORT_TBL_IDX_CTRL(port)		(0x9064 + (((port) / 16) * 4))
#define RTL93XX_PORT_TBL_IDX_CTRL_IDX_OFFSET(port) \
						(((port) & 0xF) << 1)
#define RTL93XX_PORT_TBL_IDX_CTRL_IDX_MASK(port) \
						(0x3 << RTL93XX_PORT_TBL_IDX_CTRL_IDX_OFFSET(port))

#define RTL93XX_PRI_SEL_GROUP_0			(0)
#define RTL93XX_PRI_SEL_GROUP_1			(1)

#define RTL930X_PRI_SEL_TBL_CTRL(group)		(0x9B28 + ((group) * 4))
#define RTL931X_PRI_SEL_TBL_CTRL(group)		(0x9074 + ((group) * 8))
#define RTL931X_PRI_SEL_TBL_CTRL_1BR_MASK	GENMASK(15, 12)
#define RTL931X_PRI_SEL_TBL_CTRL_MPLS_MASK	GENMASK(11, 8)
#define RTL931X_PRI_SEL_TBL_CTRL_11E_MASK	GENMASK(7, 4)
#define RTL931X_PRI_SEL_TBL_CTRL_TUNNEL_MASK	GENMASK(3, 0)

#define RTL93XX_PRI_SEL_TBL_CTRL_ROUT_MASK	GENMASK(31, 28)
#define RTL93XX_PRI_SEL_TBL_CTRL_PROT_VLAN_MASK	GENMASK(27, 24)
#define RTL93XX_PRI_SEL_TBL_CTRL_MAC_VLAN_MASK	GENMASK(23, 20)
#define RTL93XX_PRI_SEL_TBL_CTRL_OTAG_MASK	GENMASK(19, 16)
#define RTL93XX_PRI_SEL_TBL_CTRL_ITAG_MASK	GENMASK(15, 12)
#define RTL93XX_PRI_SEL_TBL_CTRL_DSCP_MASK	GENMASK(11, 8)
#define RTL93XX_PRI_SEL_TBL_CTRL_VACL_MASK	GENMASK(7, 4)
#define RTL93XX_PRI_SEL_TBL_CTRL_PORT_MASK	GENMASK(3, 0)

/* port: 0-23, index: 0-7 */
#define RTL930X_SCHED_PORT_Q_CTRL_SET0(port, index) \
						(0x3D48 + ((port) * 384) + ((index) * 4))
/* port: 24-27, index: 0-11 */
#define RTL930X_SCHED_PORT_Q_CTRL_SET1(port, index) \
						((0xE860 + ((port) - 24) * 48) + ((index) * 4))
/* port: 0-51, index: 0-7 */
#define RTL931X_SCHED_PORT_Q_CTRL_SET0(port, index) \
						(0x2888 + ((port) << 5) + ((index) * 4))
/* port: 52-55, index: 0-11 */
#define RTL931X_SCHED_PORT_Q_CTRL_SET1(port, index) \
						((0x2F08 + ((port) - 52) * 48) + ((index) * 4))

#define RTL930X_QM_INTPRI2QID_CTRL		(0xA320)
#define RTL931X_QM_INTPRI2QID_CTRL		(0xA9D0)

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
#define RTL839X_ROUTING_SA_CTRL			0x6afc
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

#define RTL931X_LED_GLB_ACTIVE_LOW BIT(21)

#define RTL931X_LED_SETX_0_CTRL(x) (RTL931X_LED_SET0_0_CTRL - (x * 8))
#define RTL931X_LED_SETX_1_CTRL(x) (RTL931X_LED_SETX_0_CTRL(x) - 4)

/* get register for given set and led in the set */
#define RTL931X_LED_SETX_LEDY(x, y) (RTL931X_LED_SETX_0_CTRL(x) - 4 * (y / 2))

/* get shift for given led in any set */
#define RTL931X_LED_SET_LEDX_SHIFT(x) (16 * (x % 2))

#define MAX_VLANS 4096
#define MAX_LAGS 16
#define MAX_PRIOS 8
#define RTL930X_PORT_IGNORE 0x3f
/* ToDo: MAX_MC_GROUPS could be increased
 * 838x/839x/930x/931x -> 8192/16384/16384/32768 entries (priv->fib_entries)
 * They are shared with unicast entries
 */
#define MAX_MC_GROUPS 512
/* ToDo: MAX_MC_PMASKS could be increased
 * 838x/839x/930x/931x -> 512/4096/1024/4096 entries
 */
#define MAX_MC_PMASKS 512
#define RTL838X_MC_PMASK_ALL_PORTS (GENMASK(RTL838X_CPU_PORT, 0))
#define RTL839X_MC_PMASK_ALL_PORTS (GENMASK_ULL(RTL839X_CPU_PORT, 0))
#define RTL930X_MC_PMASK_ALL_PORTS (GENMASK(RTL930X_CPU_PORT, 0))
#define RTL931X_MC_PMASK_ALL_PORTS (GENMASK_ULL(RTL931X_CPU_PORT, 0))
#define MC_PMASK_ALL_PORTS_IDX	((MAX_MC_PMASKS - 1))
#define PIE_BLOCK_SIZE 128
#define MAX_PIE_ENTRIES (18 * PIE_BLOCK_SIZE)
#define N_FIXED_FIELDS 12
#define N_FIXED_FIELDS_RTL931X 14
#define MAX_COUNTERS 2048
#define MAX_INTF_MTUS 8
#define DEFAULT_MTU 1536
#define MAX_ROUTER_MACS 64
#define L3_EGRESS_DMACS 2048
#define MAX_SMACS 64
#define DSCP_MAP_MAX 64

/* This interval needs to be short enough to prevent an undetected counter
 * overflow. The octet counters don't need to be considered for this, because
 * they are 64 bits on all platforms. Based on the possible packets per second
 * at the highest supported speeds, an interval of a minute is probably a safe
 * choice for the other counters.
 */
#define RTLDSA_COUNTERS_POLL_INTERVAL	(60 * HZ)

/* Some SoC families require table access to get the HW counters. A mutex is
 * required for this access - which will potentially cause a sleep in the
 * current context. This is not always possible with .get_stats64 because it
 * is also called in atomic contexts.
 *
 * For these SoCs, the retrieval of the current counters in .get_stats64 is
 * skipped and the counters are simply retrieved a lot more often from the HW.
 */
#define RTLDSA_COUNTERS_FAST_POLL_INTERVAL	(3 * HZ)

enum pbvlan_type {
	PBVLAN_TYPE_INNER = 0,
	PBVLAN_TYPE_OUTER,
};

enum pbvlan_mode {
	PBVLAN_MODE_UNTAG_AND_PRITAG = 0,
	PBVLAN_MODE_UNTAG_ONLY,
	PBVLAN_MODE_ALL_PKT,
};

struct rtldsa_counter {
	u64 val;
	u32 last;
};

struct rtldsa_counter_state {
	/**
	 * @lock: protect updates to members of the structure when the
	 * priv->counters_lock is not used. (see rtl931x_reg->stat_update_counters_atomically)
	 */
	spinlock_t lock;
	ktime_t last_update;

	struct rtldsa_counter symbol_errors;

	struct rtldsa_counter if_in_octets;
	struct rtldsa_counter if_out_octets;
	struct rtldsa_counter if_in_ucast_pkts;
	struct rtldsa_counter if_in_mcast_pkts;
	struct rtldsa_counter if_in_bcast_pkts;
	struct rtldsa_counter if_out_ucast_pkts;
	struct rtldsa_counter if_out_mcast_pkts;
	struct rtldsa_counter if_out_bcast_pkts;
	struct rtldsa_counter if_out_discards;
	struct rtldsa_counter single_collisions;
	struct rtldsa_counter multiple_collisions;
	struct rtldsa_counter deferred_transmissions;
	struct rtldsa_counter late_collisions;
	struct rtldsa_counter excessive_collisions;
	struct rtldsa_counter crc_align_errors;
	struct rtldsa_counter rx_pkts_over_max_octets;

	struct rtldsa_counter unsupported_opcodes;

	struct rtldsa_counter rx_undersize_pkts;
	struct rtldsa_counter rx_oversize_pkts;
	struct rtldsa_counter rx_fragments;
	struct rtldsa_counter rx_jabbers;

	struct rtldsa_counter tx_pkts[ETHTOOL_RMON_HIST_MAX];
	struct rtldsa_counter rx_pkts[ETHTOOL_RMON_HIST_MAX];

	struct rtldsa_counter drop_events;
	struct rtldsa_counter collisions;

	struct rtldsa_counter rx_pause_frames;
	struct rtldsa_counter tx_pause_frames;

	/** @link_stat_lock: Protect link_stat */
	spinlock_t link_stat_lock;

	/** @link_stat: Prepared return data for .get_stats64 which can be accessed without mutex */
	struct rtnl_link_stats64 link_stat;
};

struct rtldsa_93xx_lag_entry {
	u32 trk_port0:6;
	u32 trk_dev0:4;
	u32 trk_port1:6;
	u32 trk_dev1:4;
	u32 trk_port2:6;
	u32 trk_dev2:4;
	u32 trk_port3:6;
	u32 trk_dev3:4;
	u32 trk_port4:6;
	u32 trk_dev4:4;
	u32 trk_port5:6;
	u32 trk_dev5:4;
	u32 trk_port6:6;
	u32 trk_dev6:4;
	u32 trk_port7:6;
	u32 trk_dev7:4;
	u32 sep_kwn_mc_en:1;
	union {
		// for rtl930x
		u32 sep_dlf_bcast_en:1;
		// for rtl931x
		u32 sep_flood_en:1;
	} flood_dlf_bcast;
	u32 ip6_hash_mask_idx:1;
	u32 ip4_hash_mask_idx:1;
	u32 l2_hash_mask_idx:1;
	u32 num_tx_candi:4;
};

struct rtldsa_port {
	bool enable:1;
	bool phy:1;
	bool isolated:1;
	bool rate_police_egress:1;
	bool rate_police_ingress:1;
	unsigned long cached_flags;
	u64 pm;
	u16 pvid;
	bool eee_enabled;
	bool has_pcs;
	int led_set;
	enum rtldsa_flood_type flood_type;
	int leds_on_this_port;
	struct rtldsa_counter_state counters;
	const struct dsa_port *dp;
};

struct rtldsa_vlan_info {
	u64 untagged_ports;
	u64 member_ports;
	u8 profile_id;
	bool hash_mc_fid;
	bool hash_uc_fid;
	u8 fid; /* AKA MSTI */

	/* The following fields are used only by the RTL931X */
	int if_id;		/* Interface (index in L3_EGR_INTF_IDX) */
	u16 multicast_grp_mask;
	int l2_tunnel_list_id;
};

struct rtldsa_mst {
	/** @msti: MSTI mapped to this slot. 0 == unused */
	u16 msti;

	/** @refcount: number of vlans currently using this msti, undefined when unused */
	struct kref refcount;
};

struct rtldsa_vlan_profile {
	union {
		struct {
			u64 l2;
			u64 ip;
			u64 ip6;
		} pmsks;
		struct {
			u16 l2;
			u16 ip;
			u16 ip6;
		} pmsks_idx;
	} unkn_mc_fld;

	int l2_learn;

	u8 pmsk_is_idx:1, routing_ipuc:1, routing_ip6uc:1,
	   routing_ipmc:1, routing_ip6mc:1, bridge_ipmc:1, bridge_ip6mc:1;
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
	enum l2_entry_type type;
	bool valid:1;
	bool is_static:1;
	bool is_ip_mc:1;
	bool is_ipv6_mc:1;
	bool block_da:1;
	bool block_sa:1;
	bool suspended:1;
	bool next_hop:1;
	bool is_trunk:1;
	bool nh_vlan_target:1;  /* Only RTL83xx: VLAN used for next hop */
	int age;
	u8 trunk;
	u8 stack_dev;
	u16 mc_portmask_index;
	u32 mc_gip;
	u32 mc_sip;
	u16 mc_mac_index;
	u16 nh_route_id;

	/* The following is only valid on RTL931x */
	bool is_open_flow:1;
	bool is_pe_forward:1;
	bool is_local_forward:1;
	bool is_remote_forward:1;
	bool is_l2_tunnel:1;
	bool hash_msb:1;
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
 * to SoC family (e.g. because of different port ranges)
 */
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

struct rtl838x_switch_priv;

struct rtl83xx_flow {
	unsigned long cookie;
	struct rhash_head node;
	struct rcu_head rcu_head;
	struct rtl838x_switch_priv *priv;
	struct pie_rule rule;
	u32 flags;
};

/**
 * struct rtldsa_mirror_config - Mirror configuration for specific group and port
 */
struct rtldsa_mirror_config {
	/** @ctrl: control register for mirroring group */
	int ctrl;

	/** @spm: register for the destination port members */
	int spm;

	/** @dpm: register for the source port members */
	int dpm;

	/** @val: @ctrl register settings to enable mirroring */
	u32 val;
};

struct rtldsa_config {
	const struct dsa_switch_ops *switch_ops;
	const struct phylink_mac_ops *phylink_mac_ops;
	void (*mask_port_reg_be)(u64 clear, u64 set, int reg);
	void (*set_port_reg_be)(u64 set, int reg);
	u64 (*get_port_reg_be)(int reg);
	void (*mask_port_reg_le)(u64 clear, u64 set, int reg);
	void (*set_port_reg_le)(u64 set, int reg);
	u64 (*get_port_reg_le)(int reg);
	int stat_port_rst;
	int stat_rst;
	void (*stat_init)(struct rtl838x_switch_priv *priv);
	int stat_port_std_mib;
	int stat_port_prv_mib;
	const struct rtldsa_mib_desc *mib_desc;
	u64 (*stat_port_table_read)(int port, unsigned int mib_size, unsigned int offset, bool is_pvt);
	void (*stat_counters_lock)(struct rtl838x_switch_priv *priv, int port);
	void (*stat_counters_unlock)(struct rtl838x_switch_priv *priv, int port);

	/**
	 * @stat_update_counters_atomically: When set, the SoC family allows atomically retrieving
	 * of statistic counters using this function.  This function must not require "might_sleep"
	 * code.
	 *
	 * Any SoC family which requires stat_port_table_read must use the table
	 * rtldsa_counters_(un)lock_table helpers. They are using a mutex for locking. The counters
	 * update is therefore not atomic.
	 */
	void (*stat_update_counters_atomically)(struct rtl838x_switch_priv *priv, int port);
	unsigned long stat_counter_poll_interval;
	int (*port_iso_ctrl)(int p);
	void (*traffic_enable)(int source, int dest);
	void (*traffic_disable)(int source, int dest);
	void (*traffic_set)(int source, u64 dest_matrix);
	int l2_ctrl_0;
	int l2_ctrl_1;
	bool high_res_l2_age;
	u32 self_mac_trap_ctrl;
	u32 l2_port_aging_out;
	int l2_tbl_flush_ctrl;
	int isr_glb_src;
	int isr_port_link_sts_chg;
	int imr_port_link_sts_chg;
	int imr_glb;
	int n_counters;
	int n_pie_blocks;
	u8 num_lag_ids;
	u8 cpu_port;
	u8 port_ignore;
	u8 l2_bucket_size;
	u16 n_mst;
	u32 fib_entries;
	int trk_ctrl;
	int trk_hash_ctrl;
	int spanning_tree_ctrl;
	void (*vlan_tables_read)(u32 vlan, struct rtldsa_vlan_info *info);
	void (*vlan_set_tagged)(u32 vlan, struct rtldsa_vlan_info *info);
	void (*vlan_set_untagged)(u32 vlan, u64 portmask);
	int (*vlan_profile_get)(int index, struct rtldsa_vlan_profile *profile);
	void (*vlan_profile_dump)(struct rtl838x_switch_priv *priv, int index);
	void (*vlan_profile_setup)(int profile);
	void (*vlan_port_pvidmode_set)(int port, enum pbvlan_type type, enum pbvlan_mode mode);
	void (*vlan_port_pvid_set)(int port, enum pbvlan_type type, int pvid);
	void (*vlan_port_keep_tag_set)(int port, bool keep_outer, bool keep_inner);
	int (*fast_age)(struct rtl838x_switch_priv *priv, int port, int vid);
	void (*set_vlan_igr_filter)(int port, enum igr_filter state);
	void (*set_vlan_egr_filter)(int port, enum egr_filter state);
	void (*enable_learning)(int port, bool enable);
	void (*enable_l2_new_sa_fwd)(int port, enum rtldsa_flood_type flood_type);
	void (*enable_flood)(int port, enum rtldsa_flood_type flood_type);
	void (*enable_mcast_flood)(int port, bool enable);
	void (*enable_bcast_flood)(int port, bool enable);
	void (*set_static_move_action)(int port, bool forward);
	int (*stp_get)(struct rtl838x_switch_priv *priv, u16 msti, int port);
	void (*stp_set)(struct rtl838x_switch_priv *priv, u16 msti, int port, int state);
	int mac_link_sts;
	u32 mac_force_mode_mask;
	int  (*mac_force_mode_ctrl)(int port);
	int  (*mac_port_ctrl)(int port);
	int  (*l2_port_new_salrn)(int port);
	int  (*l2_port_new_sa_fwd)(int port);
	int (*set_ageing_time)(unsigned long msec);
	int (*get_mirror_config)(struct rtldsa_mirror_config *config, int group, int port);
	int (*port_rate_police_add)(struct dsa_switch *ds, int port,
				    const struct flow_action_entry *act, bool ingress);
	int (*port_rate_police_del)(struct dsa_switch *ds, int port, struct flow_cls_offload *cls,
				    bool ingress);
	void (*print_matrix)(void);
	u64 (*read_l2_entry_using_hash)(u32 hash, u32 position, struct rtl838x_l2_entry *e);
	void (*write_l2_entry_using_hash)(u32 hash, u32 pos, struct rtl838x_l2_entry *e);
	u64 (*read_cam)(int idx, struct rtl838x_l2_entry *e);
	void (*write_cam)(int idx, struct rtl838x_l2_entry *e);
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
	void (*set_receive_management_action)(int port, rma_ctrl_t type, action_type_t action);
	void (*led_init)(struct rtl838x_switch_priv *priv);
	u32 (*get_egress_rate)(struct rtl838x_switch_priv *priv, int port);
	int (*set_egress_rate)(struct rtl838x_switch_priv *priv, int port, u32 rate);
	void (*qos_init)(struct rtl838x_switch_priv *priv);
	int (*trk_mbr_ctr)(int group);
	void (*lag_switch_init)(struct rtl838x_switch_priv *priv);
	void (*prepare_lag_fdb)(struct rtl838x_l2_entry *e, int lag_group);
	int (*lag_set_port_members)(struct rtl838x_switch_priv *priv, int group, u64 members,
				    struct netdev_lag_upper_info *info);
	int (*lag_setup_algomask)(struct rtl838x_switch_priv *priv, int group,
				  struct netdev_lag_upper_info *info);
	int (*lag_set_distribution_algorithm)(struct rtl838x_switch_priv *priv,
					      int group, int algoidx,
					      u32 algomask);
	void (*lag_set_local_group_id)(int local_group, int global_group, bool valid);
	void (*lag_write_data)(u32 data[], struct rtldsa_93xx_lag_entry *e);
	void (*lag_fill_data)(u32 data[], struct rtldsa_93xx_lag_entry *e);
	void (*lag_set_local_port2group)(int group, int port, bool valid);
	void (*lag_set_port2group)(int group, int port, bool valid);
	struct table_reg* (*lag_table)(void);
	void (*lag_sync_tables)(void);
};

struct rtl838x_switch_priv {
	/* Switch operation */
	struct dsa_switch *ds;
	struct device *dev;
	u16 family_id;
	struct rtldsa_port ports[57];
	struct mutex reg_mutex;		/* Mutex for individual register manipulations */
	struct mutex pie_mutex;		/* Mutex for Packet Inspection Engine */
	int link_state_irq;
	int mirror_group_ports[4];
	const struct rtldsa_config *r;
	struct otto_l3_ctrl *l3_ctrl;
	u64 irq_mask;
	struct dentry *dbgfs_dir;

	/** @lags_port_members: Port (bit) is part of a specific LAG */
	u64 lags_port_members[MAX_LAGS];

	/** @lag_primary: port of a LAG is primary (repesenting) and is added to
	 * the port matrix
	 */
	u32 lag_primary[MAX_LAGS];

	/**
	 * @lag_non_primary: Port (bit) is part of any LAG but not the
	 * first/primary port which needs to be added in the port matrix
	 */
	u64 lag_non_primary;

	/** @lagmembers: Port (bit) is part of any LAG */
	u64 lagmembers;
	struct workqueue_struct *wq;
	bool eee_enabled;
	unsigned long mc_group_bm[MAX_MC_GROUPS >> 5];
	struct rhashtable tc_ht;
	unsigned long pie_use_bm[MAX_PIE_ENTRIES >> 5];
	unsigned long octet_cntr_use_bm[MAX_COUNTERS >> 5];
	unsigned long packet_cntr_use_bm[MAX_COUNTERS >> 4];
	u16 intf_mtus[MAX_INTF_MTUS];
	int intf_mtu_count[MAX_INTF_MTUS];

	struct delayed_work counters_work;

	/**
	 * @counters_lock: Protects the hardware reads happening from MIB
	 * callbacks and the workqueue which reads the data
	 * periodically.
	 */
	struct mutex counters_lock;

	/**
	 * @msts: MSTI to HW MST slot allocations. index 0 is for HW slot 1 because CIST is
	 * not stored in @msts
	 */
	struct rtldsa_mst msts[];
};

struct fdb_update_work {
	struct work_struct work;
	struct net_device *ndev;
	u64 macs[];
};

enum mib_reg {
	MIB_REG_INVALID = 0,
	MIB_REG_STD,
	MIB_REG_PRV,
	MIB_TBL_STD,
	MIB_TBL_PRV,
};

#define MIB_ITEM(_reg, _offset, _size) \
		{.reg = _reg, .offset = _offset, .size = _size}

#define MIB_LIST_ITEM(_name, _item) \
		{.name = _name, .item = _item}

struct rtldsa_mib_item {
	enum mib_reg reg;
	unsigned int offset;
	unsigned int size;
};

struct rtldsa_mib_list_item {
	const char *name;
	struct rtldsa_mib_item item;
};

struct rtldsa_mib_desc {
	struct rtldsa_mib_item symbol_errors;

	struct rtldsa_mib_item if_in_octets;
	struct rtldsa_mib_item if_out_octets;
	struct rtldsa_mib_item if_in_ucast_pkts;
	struct rtldsa_mib_item if_in_mcast_pkts;
	struct rtldsa_mib_item if_in_bcast_pkts;
	struct rtldsa_mib_item if_out_ucast_pkts;
	struct rtldsa_mib_item if_out_mcast_pkts;
	struct rtldsa_mib_item if_out_bcast_pkts;
	struct rtldsa_mib_item if_out_discards;
	struct rtldsa_mib_item single_collisions;
	struct rtldsa_mib_item multiple_collisions;
	struct rtldsa_mib_item deferred_transmissions;
	struct rtldsa_mib_item late_collisions;
	struct rtldsa_mib_item excessive_collisions;
	struct rtldsa_mib_item crc_align_errors;
	struct rtldsa_mib_item rx_pkts_over_max_octets;

	struct rtldsa_mib_item unsupported_opcodes;

	struct rtldsa_mib_item rx_undersize_pkts;
	struct rtldsa_mib_item rx_oversize_pkts;
	struct rtldsa_mib_item rx_fragments;
	struct rtldsa_mib_item rx_jabbers;

	struct rtldsa_mib_item tx_pkts[ETHTOOL_RMON_HIST_MAX];
	struct rtldsa_mib_item rx_pkts[ETHTOOL_RMON_HIST_MAX];
	struct ethtool_rmon_hist_range rmon_ranges[ETHTOOL_RMON_HIST_MAX];

	struct rtldsa_mib_item drop_events;
	struct rtldsa_mib_item collisions;

	struct rtldsa_mib_item rx_pause_frames;
	struct rtldsa_mib_item tx_pause_frames;

	size_t list_count;
	const struct rtldsa_mib_list_item *list;
};

/* API for switch table access */
struct table_reg {
	u16 addr;
	u16 data;
	u8  max_data;
	u8 c_bit;
	u8 t_bit;
	u8 rmode;
	u8 tbl;
	struct mutex lock;
};

#define TBL_DESC(_addr, _data, _max_data, _c_bit, _t_bit, _rmode) \
		{  .addr = _addr, .data = _data, .max_data = _max_data, .c_bit = _c_bit, \
		    .t_bit = _t_bit, .rmode = _rmode \
		}

typedef enum {
	RTL8380_TBL_L2 = 0,
	RTL8380_TBL_0,
	RTL8380_TBL_1,
	RTL8390_TBL_L2,
	RTL8390_TBL_0,
	RTL8390_TBL_1,
	RTL8390_TBL_2,
	RTL9300_TBL_L2,
	RTL9300_TBL_0,
	RTL9300_TBL_1,
	RTL9300_TBL_2,
	RTL9300_TBL_HSB,
	RTL9300_TBL_HSA,
	RTL9310_TBL_0,
	RTL9310_TBL_1,
	RTL9310_TBL_2,
	RTL9310_TBL_3,
	RTL9310_TBL_4,
	RTL9310_TBL_5,
	RTL_TBL_END
} rtl838x_tbl_reg_t;

void rtl_table_init(void);
struct table_reg *rtl_table_get(rtl838x_tbl_reg_t r, int t);
void rtl_table_release(struct table_reg *r);
int rtl_table_read(struct table_reg *r, int idx);
int rtl_table_write(struct table_reg *r, int idx);
inline u16 rtl_table_data(struct table_reg *r, int i);
inline u32 rtl_table_data_r(struct table_reg *r, int i);
inline void rtl_table_data_w(struct table_reg *r, u32 v, int i);

int rtldsa_83xx_lag_setup_algomask(struct rtl838x_switch_priv *priv, int group,
				   struct netdev_lag_upper_info *info);

void rtldsa_838x_qos_init(struct rtl838x_switch_priv *priv);
void rtldsa_839x_qos_init(struct rtl838x_switch_priv *priv);

void rtldsa_port_fast_age(struct dsa_switch *ds, int port);
int rtl83xx_packet_cntr_alloc(struct rtl838x_switch_priv *priv);
int rtldsa_port_get_stp_state(struct rtl838x_switch_priv *priv, int port);
int rtl83xx_port_is_under(const struct net_device *dev, struct rtl838x_switch_priv *priv);
void rtldsa_port_stp_state_set(struct dsa_switch *ds, int port, u8 state);
int rtl83xx_setup_tc(struct net_device *dev, enum tc_setup_type type, void *type_data);

/* Port register accessor functions for the RTL839x and RTL931X SoCs */
void rtl839x_mask_port_reg_be(u64 clear, u64 set, int reg);
u32 rtldsa_839x_get_egress_rate(struct rtl838x_switch_priv *priv, int port);
u64 rtl839x_get_port_reg_be(int reg);
void rtl839x_set_port_reg_be(u64 set, int reg);
void rtl839x_mask_port_reg_le(u64 clear, u64 set, int reg);
int rtldsa_839x_set_egress_rate(struct rtl838x_switch_priv *priv, int port, u32 rate);
void rtl839x_set_port_reg_le(u64 set, int reg);
u64 rtl839x_get_port_reg_le(int reg);

/* Port register accessor functions for the RTL838x and RTL930X SoCs */
void rtl838x_mask_port_reg(u64 clear, u64 set, int reg);
void rtl838x_set_port_reg(u64 set, int reg);
u32 rtldsa_838x_get_egress_rate(struct rtl838x_switch_priv *priv, int port);
u64 rtl838x_get_port_reg(int reg);
int rtldsa_838x_set_egress_rate(struct rtl838x_switch_priv *priv, int port, u32 rate);

/* RTL838x-specific */
u32 rtl838x_hash(struct rtl838x_switch_priv *priv, u64 seed);
void rtldsa_838x_print_matrix(void);

/* RTL839x-specific */
u32 rtl839x_hash(struct rtl838x_switch_priv *priv, u64 seed);
void rtl839x_exec_tbl2_cmd(u32 cmd);
void rtldsa_839x_print_matrix(void);

/* RTL930x-specific */
u32 rtl930x_hash(struct rtl838x_switch_priv *priv, u64 seed);
void rtldsa_930x_print_matrix(void);

/* RTL931x-specific */
void rtldsa_931x_print_matrix(void);

int rtl83xx_lag_add(struct dsa_switch *ds, int group, int port, struct netdev_lag_upper_info *info);
int rtl83xx_lag_del(struct dsa_switch *ds, int group, int port);

/*
 * TODO: The following functions are currently not in use. So compiler will complain if
 * they are static and not made available externally. To preserve them for future use
 * collect them in this section.
 */

void rtl839x_pie_rule_dump(struct  pie_rule *pr);
void rtl839x_set_egress_queue(int port, int queue);

void rtl9300_dump_debug(void);
void rtl930x_pie_rule_dump_raw(u32 r[]);

extern const struct dsa_switch_ops rtldsa_83xx_switch_ops;
extern const struct dsa_switch_ops rtldsa_93xx_switch_ops;

extern const struct phylink_mac_ops rtldsa_83xx_phylink_mac_ops;
extern const struct phylink_mac_ops rtldsa_93xx_phylink_mac_ops;

extern const struct rtldsa_config rtldsa_838x_cfg;
extern const struct rtldsa_config rtldsa_839x_cfg;
extern const struct rtldsa_config rtldsa_930x_cfg;
extern const struct rtldsa_config rtldsa_931x_cfg;

/* TODO actually from arch/mips/rtl838x/prom.c */
extern struct rtl83xx_soc_info soc_info;


void rtl838x_dbgfs_init(struct rtl838x_switch_priv *priv);
void rtl930x_dbgfs_init(struct rtl838x_switch_priv *priv);
void rtldsa_93xx_lag_switch_init(struct rtl838x_switch_priv *priv);
int rtldsa_93xx_lag_set_distribution_algorithm(struct rtl838x_switch_priv *priv,
					       int group, int algoidx, u32 algomsk);
int rtldsa_93xx_lag_set_port_members(struct rtl838x_switch_priv *priv, int group,
				     u64 members, struct netdev_lag_upper_info *info);

void rtldsa_93xx_prepare_lag_fdb(struct rtl838x_l2_entry *e, int lag_group);

void rtldsa_counters_lock_register(struct rtl838x_switch_priv *priv, int port)
	__acquires(&priv->ports[port].counters.lock);
void rtldsa_counters_unlock_register(struct rtl838x_switch_priv *priv, int port)
	__releases(&priv->ports[port].counters.lock);
void rtldsa_counters_lock_table(struct rtl838x_switch_priv *priv, int port)
	__acquires(&priv->counters_lock);
void rtldsa_counters_unlock_table(struct rtl838x_switch_priv *priv, int port)
	__releases(&priv->ports[port].counters.lock);

void rtldsa_update_counters_atomically(struct rtl838x_switch_priv *priv, int port);


struct otto_l3_nexthop;
int rtl83xx_l2_nexthop_add(struct rtl838x_switch_priv *priv, struct otto_l3_nexthop *nh);
int rtl83xx_l2_nexthop_rm(struct rtl838x_switch_priv *priv, struct otto_l3_nexthop *nh);


extern int rtldsa_max_available_queue[];
extern int rtldsa_default_queue_weights[];

#endif /* _RTL838X_H */

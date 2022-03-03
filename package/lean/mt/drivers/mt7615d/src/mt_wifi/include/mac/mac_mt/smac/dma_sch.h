/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology	5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering	the source code	is stricitly prohibited, unless	the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	dma_sch.h

	Abstract:
	Ralink Wireless Chip MAC related definition & structures

	Revision History:
	Who			When		  What
	--------	----------	  ----------------------------------------------
*/


#ifndef __DMA_SCH_H__
#define __DMA_SCH_H__

#define DMA_SCH_BASE		0x4000 /* equal to HIF_BASE */


#define MT_TXTIME_THD_0		(DMA_SCH_BASE + 0x500)
#define MT_TXTIME_THD_1		(DMA_SCH_BASE + 0x504)
#define MT_TXTIME_THD_2		(DMA_SCH_BASE + 0x508)
#define MT_TXTIME_THD_3		(DMA_SCH_BASE + 0x50c)
#define MT_TXTIME_THD_4		(DMA_SCH_BASE + 0x510)
#define MT_TXTIME_THD_5		(DMA_SCH_BASE + 0x514)
#define MT_TXTIME_THD_6		(DMA_SCH_BASE + 0x518)
#define MT_TXTIME_THD_7		(DMA_SCH_BASE + 0x51c)
#define MT_TXTIME_THD_8		(DMA_SCH_BASE + 0x520)
#define MT_TXTIME_THD_9		(DMA_SCH_BASE + 0x524)
#define MT_TXTIME_THD_10	(DMA_SCH_BASE + 0x528)
#define MT_TXTIME_THD_11	(DMA_SCH_BASE + 0x52c)
#define MT_TXTIME_THD_12	(DMA_SCH_BASE + 0x530)
#define MT_TXTIME_THD_13	(DMA_SCH_BASE + 0x534)
#define MT_TXTIME_THD_14	(DMA_SCH_BASE + 0x538)
#define MT_TXTIME_THD_15	(DMA_SCH_BASE + 0x53c)

#define MT_PAGE_CNT_0 (DMA_SCH_BASE + 0x540)
#define PAGE_CNT_0_MASK (0xfffff)
#define GET_PAGE_CNT_0(p) (((p) & PAGE_CNT_0_MASK))
#define MT_PAGE_CNT_1 (DMA_SCH_BASE + 0x544)
#define PAGE_CNT_1_MASK (0xfffff)
#define GET_PAGE_CNT_1(p) (((p) & PAGE_CNT_1_MASK))
#define MT_PAGE_CNT_2 (DMA_SCH_BASE + 0x548)
#define PAGE_CNT_2_MASK (0xfffff)
#define GET_PAGE_CNT_2(p) (((p) & PAGE_CNT_2_MASK))
#define MT_PAGE_CNT_3 (DMA_SCH_BASE + 0x54c)
#define PAGE_CNT_3_MASK (0xfffff)
#define GET_PAGE_CNT_3(p) (((p) & PAGE_CNT_3_MASK))
#define MT_PAGE_CNT_4 (DMA_SCH_BASE + 0x550)
#define PAGE_CNT_4_MASK (0xfffff)
#define GET_PAGE_CNT_4(p) (((p) & PAGE_CNT_4_MASK))
#define MT_PAGE_CNT_5 (DMA_SCH_BASE + 0x554)
#define PAGE_CNT_5_MASK (0xfffff)
#define GET_PAGE_CNT_5(p) (((p) & PAGE_CNT_5_MASK))
#define MT_PAGE_CNT_6 (DMA_SCH_BASE + 0x558)
#define PAGE_CNT_6_MASK (0xfffff)
#define GET_PAGE_CNT_6(p) (((p) & PAGE_CNT_6_MASK))
#define MT_PAGE_CNT_7 (DMA_SCH_BASE + 0x55c)
#define PAGE_CNT_7_MASK (0xfffff)
#define GET_PAGE_CNT_7(p) (((p) & PAGE_CNT_7_MASK))
#define MT_PAGE_CNT_8 (DMA_SCH_BASE + 0x560)
#define PAGE_CNT_8_MASK (0xfffff)
#define GET_PAGE_CNT_8(p) (((p) & PAGE_CNT_8_MASK))
#define MT_PAGE_CNT_9 (DMA_SCH_BASE + 0x564)
#define PAGE_CNT_9_MASK (0xfffff)
#define GET_PAGE_CNT_9(p) (((p) & PAGE_CNT_9_MASK))
#define MT_PAGE_CNT_10 (DMA_SCH_BASE + 0x568)
#define PAGE_CNT_10_MASK (0xfffff)
#define GET_PAGE_CNT_10(p) (((p) & PAGE_CNT_10_MASK))
#define MT_PAGE_CNT_11 (DMA_SCH_BASE + 0x56c)
#define PAGE_CNT_11_MASK (0xfffff)
#define GET_PAGE_CNT_11(p) (((p) & PAGE_CNT_11_MASK))
#define MT_PAGE_CNT_12 (DMA_SCH_BASE + 0x570)
#define PAGE_CNT_12_MASK (0xfffff)
#define GET_PAGE_CNT_12(p) (((p) & PAGE_CNT_12_MASK))
#define MT_PAGE_CNT_13 (DMA_SCH_BASE + 0x574)
#define PAGE_CNT_13_MASK (0xfffff)
#define GET_PAGE_CNT_13(p) (((p) & PAGE_CNT_13_MASK))
#define MT_PAGE_CNT_14 (DMA_SCH_BASE + 0x578)
#define PAGE_CNT_14_MASK (0xfffff)
#define GET_PAGE_CNT_14(p) (((p) & PAGE_CNT_14_MASK))
#define MT_PAGE_CNT_15 (DMA_SCH_BASE + 0x57c)
#define PAGE_CNT_15_MASK (0xfffff)
#define GET_PAGE_CNT_15(p) (((p) & PAGE_CNT_15_MASK))

#define MT_QUEUE_PRIORITY_1	(DMA_SCH_BASE + 0x580)
#define RG_QUEUE_PRIORITY_0_MASK (0xf)
#define RG_QUEUE_PRIORITY_0(p) (((p) & 0xf))
#define GET_RG_QUEUE_PRIORITY_0(p) (((p) & RG_QUEUE_PRIORITY_0_MASK))
#define RG_QUEUE_PRIORITY_1_MASK (0xf << 4)
#define RG_QUEUE_PRIORITY_1(p) (((p) & 0xf) << 4)
#define GET_RG_QUEUE_PRIORITY_1(p) (((p) & RG_QUEUE_PRIORITY_1_MASK) >> 4)
#define RG_QUEUE_PRIORITY_2_MASK (0xf << 8)
#define RG_QUEUE_PRIORITY_2(p) (((p) & 0xf) << 8)
#define GET_RG_QUEUE_PRIORITY_2(p) (((p) & RG_QUEUE_PRIORITY_2_MASK) >> 8)
#define RG_QUEUE_PRIORITY_3_MASK (0xf << 12)
#define RG_QUEUE_PRIORITY_3(p) (((p) & 0xf) << 12)
#define GET_RG_QUEUE_PRIORITY_3(p) (((p) & RG_QUEUE_PRIORITY_3_MASK) >> 12)
#define RG_QUEUE_PRIORITY_4_MASK (0xf << 16)
#define RG_QUEUE_PRIORITY_4(p) (((p) & 0xf) << 16)
#define GET_RG_QUEUE_PRIORITY_4(p) (((p) & RG_QUEUE_PRIORITY_4_MASK) >> 16)
#define RG_QUEUE_PRIORITY_5_MASK (0xf << 20)
#define RG_QUEUE_PRIORITY_5(p) (((p) & 0xf) << 20)
#define GET_RG_QUEUE_PRIORITY_5(p) (((p) & RG_QUEUE_PRIORITY_5_MASK) >> 20)
#define RG_QUEUE_PRIORITY_6_MASK (0xf << 24)
#define RG_QUEUE_PRIORITY_6(p) (((p) & 0xf) << 24)
#define GET_RG_QUEUE_PRIORITY_6(p) (((p) & RG_QUEUE_PRIORITY_6_MASK) >> 24)
#define RG_QUEUE_PRIORITY_7_MASK (0xf << 28)
#define RG_QUEUE_PRIORITY_7(p) (((p) & 0xf) << 28)
#define GET_RG_QUEUE_PRIORITY_7(p) (((p) & RG_QUEUE_PRIORITY_7_MASK) >> 28)


#define MT_QUEUE_PRIORITY_2	(DMA_SCH_BASE + 0x584)
#define RG_QUEUE_PRIORITY_8_MASK (0xf)
#define RG_QUEUE_PRIORITY_8(p) (((p) & 0xf))
#define GET_RG_QUEUE_PRIORITY_8(p) (((p) & RG_QUEUE_PRIORITY_8_MASK))
#define RG_QUEUE_PRIORITY_9_MASK (0xf << 4)
#define RG_QUEUE_PRIORITY_9(p) (((p) & 0xf) << 4)
#define GET_RG_QUEUE_PRIORITY_9(p) (((p) & RG_QUEUE_PRIORITY_9_MASK) >> 4)
#define RG_QUEUE_PRIORITY_10_MASK (0xf << 8)
#define RG_QUEUE_PRIORITY_10(p) (((p) & 0xf) << 8)
#define GET_RG_QUEUE_PRIORITY_10(p) (((p) & RG_QUEUE_PRIORITY_10_MASK) >> 8)
#define RG_QUEUE_PRIORITY_11_MASK (0xf << 12)
#define RG_QUEUE_PRIORITY_11(p) (((p) & 0xf) << 12)
#define GET_RG_QUEUE_PRIORITY_11(p) (((p) & RG_QUEUE_PRIORITY_11_MASK) >> 12)
#define RG_QUEUE_PRIORITY_12_MASK (0xf << 16)
#define RG_QUEUE_PRIORITY_12(p) (((p) & 0xf) << 16)
#define GET_RG_QUEUE_PRIORITY_12(p) (((p) & RG_QUEUE_PRIORITY_12_MASK) >> 16)
#define RG_QUEUE_PRIORITY_13_MASK (0xf << 20)
#define RG_QUEUE_PRIORITY_13(p) (((p) & 0xf) << 20)
#define GET_RG_QUEUE_PRIORITY_13(p) (((p) & RG_QUEUE_PRIORITY_13_MASK) >> 20)
#define RG_QUEUE_PRIORITY_14_MASK (0xf << 24)
#define RG_QUEUE_PRIORITY_14(p) (((p) & 0xf) << 24)
#define GET_RG_QUEUE_PRIORITY_14(p) (((p) & RG_QUEUE_PRIORITY_14_MASK) >> 24)
#define RG_QUEUE_PRIORITY_15_MASK (0xf << 28)
#define RG_QUEUE_PRIORITY_15(p) (((p) & 0xf) << 28)
#define GET_RG_QUEUE_PRIORITY_15(p) (((p) & RG_QUEUE_PRIORITY_15_MASK) >> 28)

#define MT_SCH_REG_1 (DMA_SCH_BASE + 0x588)
#define RG_FFA_THD_MASK (0xfffff)
#define RG_FFA_THD(p) (((p) & 0xfffff))
#define GET_RG_FFA_THD(p) (((p) & RG_FFA_THD_MASK))
#define RG_PAGE_SIZE_MASK (0xf << 28)
#define RG_PAGE_SIZE(p) (((p) & 0xf) << 28)
#define GET_RG_PAGE_SIZE(p) (((p) & RG_PAGE_SIZE_MASK) >> 28)

#define MT_SCH_REG_2 (DMA_SCH_BASE + 0x58c)
#define RG_MAX_PKT_SIZE_MASK (0xfffff)
#define RG_MAX_PKT_SIZE(p) (((p) & 0xfffff))
#define GET_RG_MAX_PKT_SIZE(p) (((p) & RG_MAX_PKT_SIZE_MASK))

/* TODO: shiang-usw, these two CR definition is the same for "SCH_REG4" and "MT_SCH_REG_4", need to revise */
#define SCH_REG4 (DMA_SCH_BASE + 0x0594)
#define SCH_REG4_FORCE_QID_MASK (0x0f)
#define SCH_REG4_FORCE_QID(p) (((p) & 0x0f))
#define SCH_REG4_BYPASS_MODE_MASK (0x1 << 5)
#define SCH_REG4_BYPASS_MODE(p) (((p) & 0x1) << 5)


#define MT_SCH_REG_3		(DMA_SCH_BASE + 0x590)
#define MT_SCH_REG_4		(DMA_SCH_BASE + 0x594)

#define MT_SCH_REG_4 (DMA_SCH_BASE + 0x594)
#define FORCE_QID_MASK (0Xf)
#define FORCE_QID(p) (((p) & 0xf))
#define GET_FORCE_QID(p) (((p) & FORCE_QID_MASK))
#define FORCE_MODE (1 << 4)
#define GET_FORCE_MODE(p) (((p) & FORCE_MODE) >> 4)
#define BYPASS_MODE (1 << 5)
#define GET_BYPASS_MODE(p) (((p) & BYPASS_MODE) >> 5)
#define HYBIRD_MODE (1 << 6)
#define GET_HYBIRD_MODE(p) (((p) & HYBIRD_MODE) >> 6)
#define RG_PREDICT_NO_MASK (1 << 7)
#define GET_RG_PREDICT_NO_MASK(p) (((p) & RG_PREDICT_NO_MASK) >> 7)
#define SW_MODE (1 << 10)
#define GET_SW_MODE(p) (((p) & SW_MODE) >> 10)
#define RG_RATE_MAP_MASK (0x3fff << 16)
#define RG_RATE_MAP(p) (((p) & 0x3fff) << 16)
#define GET_RG_RATE_MAP(p) (((p) & RG_RATE_MAP_MASK) >> 16)

#define MT_GROUP_THD_0 (DMA_SCH_BASE + 0x598)
#define GROUP_THD_0_MASK (0xfffff)
#define GROUP_THD_0(p) (((p) & 0xfffff))
#define GET_GROUP_THD_0(p) (((p) & GROUP_THD_0_MASK))

#define MT_GROUP_THD_1		(DMA_SCH_BASE + 0x59c)

#define GROUP_THD_1_MASK (0xfffff)
#define GROUP_THD_1(p) (((p) & 0xfffff))
#define GET_GROUP_THD_1(p) (((p) & GROUP_THD_1_MASK))

#define MT_GROUP_THD_2		(DMA_SCH_BASE + 0x5a0)
#define GROUP_THD_2_MASK (0xfffff)
#define GROUP_THD_2(p) (((p) & 0xfffff))
#define GET_GROUP_THD_2(p) (((p) & GROUP_THD_2_MASK))

#define MT_GROUP_THD_3		(DMA_SCH_BASE + 0x5a4)
#define GROUP_THD_3_MASK (0xfffff)
#define GROUP_THD_3(p) (((p) & 0xfffff))
#define GET_GROUP_THD_3(p) (((p) & GROUP_THD_3_MASK))

#define MT_GROUP_THD_4		(DMA_SCH_BASE + 0x5a8)
#define GROUP_THD_4_MASK (0xfffff)
#define GROUP_THD_4(p) (((p) & 0xfffff))
#define GET_GROUP_THD_4(p) (((p) & GROUP_THD_4_MASK))

#define MT_GROUP_THD_5		(DMA_SCH_BASE + 0x5ac)
#define GROUP_THD_5_MASK (0xfffff)
#define GROUP_THD_5(p) (((p) & 0xfffff))
#define GET_GROUP_THD_5(p) (((p) & GROUP_THD_5_MASK))

#define MT_BMAP_0 (DMA_SCH_BASE + 0x5b0)
#define RG_BMAP_0_MASK (0xffff)
#define RG_BMAP_0(p) (((p) & 0xffff))
#define GET_RG_BMAP_0(p) (((p) & RG_BMAP_0_MASK))
#define RG_BMAP_1_MASK (0xffff << 16)
#define RG_BMAP_1(p) (((p) & 0xffff) << 16)
#define GET_RG_BMAP_1(p) (((p) & RG_BMAP_1_MASK) >> 16)

#define MT_BMAP_1 (DMA_SCH_BASE + 0x5b4)
#define RG_BMAP_2_MASK (0xffff)
#define RG_BMAP_2(p) (((p) & 0xffff))
#define GET_RG_BMAP_2(p) (((p) & RG_BMAP_2_MASK))
#define RG_BMAP_3_MASK (0xffff << 16)
#define RG_BMAP_3(p) (((p) & 0xffff) << 16)
#define GET_RG_BMAP_3(p) (((p) & RG_BMAP_3_MASK) >> 16)

#define MT_BMAP_2		(DMA_SCH_BASE + 0x5b8)
#define RG_BMAP_4_MASK (0xffff)
#define RG_BMAP_4(p) (((p) & 0xffff))
#define GET_RG_BMAP_4(p) (((p) & RG_BMAP_4_MASK))
#define RG_BMAP_5_MASK (0xffff << 16)
#define RG_BMAP_5(p) (((p) & 0xffff) << 16)
#define GET_RG_BMAP_5(p) (((p) & RG_BMAP_5_MASK) >> 16)

#define MT_HIGH_PRIORITY_1	(DMA_SCH_BASE + 0x5bc)
#define RG_HIGH_PRIORITY_0_MASK (0xf)
#define RG_HIGH_PRIORITY_0(p) (((p) & 0xf))
#define GET_RG_HIGH_PRIORITY_0(p) (((p) & RG_HIGH_PRIORITY_0_MASK))
#define RG_HIGH_PRIORITY_1_MASK (0xf << 4)
#define RG_HIGH_PRIORITY_1(p) (((p) & 0xf) << 4)
#define GET_RG_HIGH_PRIORITY_1(p) (((p) & RG_HIGH_PRIORITY_1_MASK) >> 4)
#define RG_HIGH_PRIORITY_2_MASK (0xf << 8)
#define RG_HIGH_PRIORITY_2(p) (((p) & 0xf) << 8)
#define GET_RG_HIGH_PRIORITY_2(p) (((p) & RG_HIGH_PRIORITY_2_MASK) >> 8)
#define RG_HIGH_PRIORITY_3_MASK (0xf << 12)
#define RG_HIGH_PRIORITY_3(p) (((p) & 0xf) << 12)
#define GET_RG_HIGH_PRIORITY_3(p) (((p) & RG_HIGH_PRIORITY_3_MASK) >> 12)
#define RG_HIGH_PRIORITY_4_MASK (0xf << 16)
#define RG_HIGH_PRIORITY_4(p) (((p) & 0xf) << 16)
#define GET_RG_HIGH_PRIORITY_4(p) (((p) & RG_HIGH_PRIORITY_4_MASK) >> 16)
#define RG_HIGH_PRIORITY_5_MASK (0xf << 20)
#define RG_HIGH_PRIORITY_5(p) (((p) & 0xf) << 20)
#define GET_RG_HIGH_PRIORITY_5(p) (((p) & RG_HIGH_PRIORITY_5_MASK) >> 20)
#define RG_HIGH_PRIORITY_6_MASK (0xf << 24)
#define RG_HIGH_PRIORITY_6(p) (((p) & 0xf) << 24)
#define GET_RG_HIGH_PRIORITY_6(p) (((p) & RG_HIGH_PRIORITY_6_MASK) >> 24)
#define RG_HIGH_PRIORITY_7_MASK (0xf << 28)
#define RG_HIGH_PRIORITY_7(p) (((p) & 0xf) << 28)
#define GET_RG_HIGH_PRIORITY_7(p) (((p) & RG_HIGH_PRIORITY_7_MASK) >> 28)

#define MT_HIGH_PRIORITY_2	(DMA_SCH_BASE + 0x5c0)
#define RG_HIGH_PRIORITY_8_MASK (0xf)
#define RG_HIGH_PRIORITY_8(p) (((p) & 0xf))
#define GET_RG_HIGH_PRIORITY_8(p) (((p) & RG_HIGH_PRIORITY_8_MASK))
#define RG_HIGH_PRIORITY_9_MASK (0xf << 4)
#define RG_HIGH_PRIORITY_9(p) (((p) & 0xf) << 4)
#define GET_RG_HIGH_PRIORITY_9(p) (((p) & RG_HIGH_PRIORITY_9_MASK) >> 4)
#define RG_HIGH_PRIORITY_10_MASK (0xf << 8)
#define RG_HIGH_PRIORITY_10(p) (((p) & 0xf) << 8)
#define GET_RG_HIGH_PRIORITY_10(p) (((p) & RG_HIGH_PRIORITY_10_MASK) >> 8)
#define RG_HIGH_PRIORITY_11_MASK (0xf << 12)
#define RG_HIGH_PRIORITY_11(p) (((p) & 0xf) << 12)
#define GET_RG_HIGH_PRIORITY_11(p) (((p) & RG_HIGH_PRIORITY_11_MASK) >> 12)
#define RG_HIGH_PRIORITY_12_MASK (0xf << 16)
#define RG_HIGH_PRIORITY_12(p) (((p) & 0xf) << 16)
#define GET_RG_HIGH_PRIORITY_12(p) (((p) & RG_HIGH_PRIORITY_12_MASK) >> 16)
#define RG_HIGH_PRIORITY_13_MASK (0xf << 20)
#define RG_HIGH_PRIORITY_13(p) (((p) & 0xf) << 20)
#define GET_RG_HIGH_PRIORITY_13(p) (((p) & RG_HIGH_PRIORITY_13_MASK) >> 20)
#define RG_HIGH_PRIORITY_14_MASK (0xf << 24)
#define RG_HIGH_PRIORITY_14(p) (((p) & 0xf) << 24)
#define GET_RG_HIGH_PRIORITY_14(p) (((p) & RG_HIGH_PRIORITY_14_MASK) >> 24)
#define RG_HIGH_PRIORITY_15_MASK (0xf << 28)
#define RG_HIGH_PRIORITY_15(p) (((p) & 0xf) << 28)
#define GET_RG_HIGH_PRIORITY_15(p) (((p) & RG_HIGH_PRIORITY_15_MASK) >> 28)

#define MT_PRIORITY_MASK (DMA_SCH_BASE + 0x5c4)
#define RG_QUEUE_PRIORITY_MASK (0xffff)
#define RG_QUEUE_PRIORITY(p) (((p) & 0xffff))
#define GET_RG_QUEUE_PRIORITY(p) (((p) & RG_QUEUE_PRIORITY_MASK))
#define RG_HIGH_PRIORITY_MASK (0xffff << 16)
#define RG_HIGH_PRIORITY(p) (((p) & 0xffff) << 16)
#define GET_RG_HIGH_PRIORITY(p) (((p) & RG_HIGH_PRIORITY_MASK) >> 16)

#define MT_RSV_MAX_THD (DMA_SCH_BASE + 0x5c8)
#define RG_RSV_MAX_THD_MASK (0xfffff)
#define RG_RSV_MAX_THD(p) (((p) & 0xfffff))
#define GET_RG_RSV_MAX_THD(p) (((p) & RG_RSV_MAX_THD_MASK))

#define RSV_AC_CNT_0 (DMA_SCH_BASE + 0x5d0)
#define RSV_AC_CNT_0_MASK (0xfffff)
#define GET_RSV_AC_CNT_0(p) (((p) & RSV_AC_CNT_0_MASK))

#define RSV_AC_CNT_1 (DMA_SCH_BASE + 0x5d4)
#define RSV_AC_CNT_1_MASK (0xfffff)
#define GET_RSV_AC_CNT_1(p) (((p) & RSV_AC_CNT_1_MASK))

#define RSV_AC_CNT_2 (DMA_SCH_BASE + 0x5d8)
#define RSV_AC_CNT_2_MASK (0xfffff)
#define GET_RSV_AC_CNT_2(p) (((p) & RSV_AC_CNT_2_MASK))

#define RSV_AC_CNT_3 (DMA_SCH_BASE + 0x5dc)
#define RSV_AC_CNT_3_MASK (0xfffff)
#define GET_RSV_AC_CNT_3(p) (((p) & RSV_AC_CNT_3_MASK))

#define RSV_AC_CNT_4 (DMA_SCH_BASE + 0x5e0)
#define RSV_AC_CNT_4_MASK (0xfffff)
#define GET_RSV_AC_CNT_4(p) (((p) & RSV_AC_CNT_4_MASK))

#define RSV_AC_CNT_5 (DMA_SCH_BASE + 0x5e4)
#define RSV_AC_CNT_5_MASK (0xfffff)
#define GET_RSV_AC_CNT_5(p) (((p) & RSV_AC_CNT_5_MASK))

#define RSV_AC_CNT_6 (DMA_SCH_BASE + 0x5e8)
#define RSV_AC_CNT_6_MASK (0xfffff)
#define GET_RSV_AC_CNT_6(p) (((p) & RSV_AC_CNT_6_MASK))

#define RSV_AC_CNT_7 (DMA_SCH_BASE + 0x5ec)
#define RSV_AC_CNT_7_MASK (0xfffff)
#define GET_RSV_AC_CNT_7(p) (((p) & RSV_AC_CNT_7_MASK))

#define RSV_AC_CNT_8 (DMA_SCH_BASE + 0x5f0)
#define RSV_AC_CNT_8_MASK (0xfffff)
#define GET_RSV_AC_CNT_8(p) (((p) & RSV_AC_CNT_8_MASK))

#define RSV_AC_CNT_9 (DMA_SCH_BASE + 0x5f4)
#define RSV_AC_CNT_9_MASK (0xfffff)
#define GET_RSV_AC_CNT_9(p) (((p) & RSV_AC_CNT_9_MASK))

#define RSV_AC_CNT_10 (DMA_SCH_BASE + 0x5f8)
#define RSV_AC_CNT_10_MASK (0xfffff)
#define GET_RSV_AC_CNT_10(p) (((p) & RSV_AC_CNT_10_MASK))

#define RSV_AC_CNT_11 (DMA_SCH_BASE + 0x5fc)
#define RSV_AC_CNT_11_MASK (0xfffff)
#define GET_RSV_AC_CNT_11(p) (((p) & RSV_AC_CNT_11_MASK))

#define RSV_AC_CNT_12 (DMA_SCH_BASE + 0x600)
#define RSV_AC_CNT_12_MASK (0xfffff)
#define GET_RSV_AC_CNT_12(p) (((p) & RSV_AC_CNT_12_MASK))

#define RSV_AC_CNT_13 (DMA_SCH_BASE + 0x604)
#define RSV_AC_CNT_13_MASK (0xfffff)
#define GET_RSV_AC_CNT_13(p) (((p) & RSV_AC_CNT_13_MASK))

#define RSV_AC_CNT_14 (DMA_SCH_BASE + 0x608)
#define RSV_AC_CNT_14_MASK (0xfffff)
#define GET_RSV_AC_CNT_14(p) (((p) & RSV_AC_CNT_14_MASK))

#define RSV_AC_CNT_15 (DMA_SCH_BASE + 0x60c)
#define RSV_AC_CNT_15_MASK (0xfffff)
#define GET_RSV_AC_CNT_15(p) (((p) & RSV_AC_CNT_15_MASK))

#define SCH_DBG0_0 (DMA_SCH_BASE + 0x650)
#define RSV_ENOUGH_MASK (0xffff)
#define GET_RSV_ENOUGH(p) (((p) & 0xffff))
#define GROUP_ENOUGH (1 << 20)
#define GET_GROUP_ENOUGH(p) (((p) & GROUP_ENOUGH) >> 20)
#define QUEUE_ENOUGH (1 << 21)
#define GET_QUEUE_ENOUGH(p) (((p) & QUEUE_ENOUGH) >> 21)
#define PREDICT_MODE (1 << 22)
#define GET_PREDICT_MODE(p) (((p) & PREDICT_MODE) >> 22)
#define REST_QUEUE_EN (1 << 28)
#define GET_REST_QUEUE_EN(p) (((p) & REST_QUEUE_EN) >> 28)
#define ENOUGH_TXTIME (1 << 30)
#define GET_ENOUGH_TXTIME(p) (((p) & ENOUGH_TXTIME) >> 30)
#define ENOUGH_BUF (1 << 31)
#define GET_ENOUGH_BUF(p) (((p) & ENOUGH_BUF) >> 31)

#define SCH_DBG_1 (DMA_SCH_BASE + 0x654)
#define FFA_PAGE_CNT_MASK (0xfffff)
#define GET_FFA_PAGE_CNT(p) (((p) & FFA_PAGE_CNT_MASK))


#define SCH_DBG_2 (DMA_SCH_BASE + 0x658)
#define PKT_TX_TIME_MASK (0xffffffff)
#define GET_PKT_TX_TIME(p) (((p) & PKT_TX_TIME_MASK))

#define SCH_DBG_3 (DMA_SCH_BASE + 0x65c)
#define TX_TIME_PER_BYTE_MASK (0x1fff)
#define GET_TX_TIME_PER_BYTE(p) (((p) & TX_TIME_PER_BYTE_MASK))

#define SCH_DBG_4 (DMA_SCH_BASE + 0x660)
#define PSE_RSV_SPACE_MASK (0xfff)
#define GET_PSE_RSV_SPACE(p) (((p) & PSE_RSV_SPACE_MASK))
#define HIF_RSV_PCNT_MASK (0xfff << 16)
#define GET_HIF_RSV_PCNT(p) (((p) & HIF_RSV_PCNT_MASK) >> 16)
#define HIF_RSV_PCNT_UPDT_MASK (1 << 31)
#define GET_HIF_RSV_PCNT_UPDT(p) (((p) & HIF_RSV_PCNT_UPDT_MASK) >> 31)

#define SCH_DBG_5 (DMA_SCH_BASE + 0x664)
#define GROUP_EN_MASK (0xffff)
#define GET_GROUP_EN(p) (((p) & GROUP_EN_MASK))

#define SCH_DBG_6 (DMA_SCH_BASE + 0x668)
#define USED_GROUP_0_MASK (0xfffff)
#define GET_USED_GROUP_0(p) (((p) & USED_GROUP_0_MASK))

#define SCH_DBG_7 (DMA_SCH_BASE + 0x66c)
#define USED_GROUP_1_MASK (0xfffff)
#define GET_USED_GROUP_1(p) (((p) & USED_GROUP_1_MASK))

#define SCH_DBG_8 (DMA_SCH_BASE + 0x670)
#define USED_GROUP_2_MASK (0xfffff)
#define GET_USED_GROUP_2(p) (((p) & USED_GROUP_2_MASK))

#define SCH_DBG_9 (DMA_SCH_BASE + 0x674)
#define USED_GROUP_3_MASK (0xfffff)
#define GET_USED_GROUP_3(p) (((p) & USED_GROUP_3_MASK))

#define SCH_DBG_10 (DMA_SCH_BASE + 0x678)
#define USED_GROUP_4_MASK (0xfffff)
#define GET_USED_GROUP_4(p) (((p) & USED_GROUP_4_MASK))

#define SCH_DBG_11 (DMA_SCH_BASE + 0x67c)
#define USED_GROUP_5_MASK (0xfffff)
#define GET_USED_GROUP_5(p) (((p) & USED_GROUP_5_MASK))

#if defined(MT7636_FPGA) || defined(MT7637_FPGA)
#define MT_FPGA_PSE_SET_0	(0x8102008C)
#define MT_FPGA_PSE_SET_1	(0x8102009C)
#define MT_FPGA_PSE_CLIENT_CNT (0x800c006c)
#endif /* MT7636_FPGA || MT7637_FPGA */


#endif /* __DMA_SCH_H__ */


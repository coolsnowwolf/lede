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

#define DMASHDL_BASE		0x5000a000

#define MT_HIF_DMASHDL_PKT_MAX_SIZE (DMASHDL_BASE + 0x1c)

#define PLE_PKT_MAX_SIZE_MASK (0xfff << 0)
#define PLE_PKT_MAX_SIZE_NUM(p) (((p) & 0xfff) << 0)
#define GET_PLE_PKT_MAX_SIZE_NUM(p) (((p) & PLE_PKT_MAX_SIZE_MASK) >> 0)

#define PSE_PKT_MAX_SIZE_MASK (0xfff << 16)
#define PSE_PKT_MAX_SIZE_NUM(p) (((p) & 0xfff) << 16)
#define GET_PSE_PKT_MAX_SIZE_NUM(p) (((p) & PSE_PKT_MAX_SIZE_MASK) >> 16)
#define MT_HIF_DMASHDL_OPTION_CTRL	(DMASHDL_BASE + 0x08)
#define MT_HIF_DMASHDL_REFILL_CTRL	(DMASHDL_BASE + 0x10)
#define MT_HIF_DMASHDL_GROUP0_CTRL	(DMASHDL_BASE + 0x20)
#define MT_HIF_DMASHDL_GROUP1_CTRL	(DMASHDL_BASE + 0x24)
#define MT_HIF_DMASHDL_GROUP2_CTRL	(DMASHDL_BASE + 0x28)
#define MT_HIF_DMASHDL_GROUP3_CTRL	(DMASHDL_BASE + 0x2c)
#define MT_HIF_DMASHDL_GROUP4_CTRL	(DMASHDL_BASE + 0x30)
#define MT_HIF_DMASHDL_GROUP5_CTRL	(DMASHDL_BASE + 0x34)
#define MT_HIF_DMASHDL_GROUP6_CTRL	(DMASHDL_BASE + 0x38)
#define MT_HIF_DMASHDL_GROUP7_CTRL	(DMASHDL_BASE + 0x3c)
#define MT_HIF_DMASHDL_GROUP8_CTRL	(DMASHDL_BASE + 0x40)
#define MT_HIF_DMASHDL_GROUP9_CTRL	(DMASHDL_BASE + 0x44)
#define MT_HIF_DMASHDL_GROUP10_CTRL	(DMASHDL_BASE + 0x48)
#define MT_HIF_DMASHDL_GROUP11_CTRL	(DMASHDL_BASE + 0x4c)
#define MT_HIF_DMASHDL_GROUP12_CTRL	(DMASHDL_BASE + 0x50)
#define MT_HIF_DMASHDL_GROUP13_CTRL	(DMASHDL_BASE + 0x54)
#define MT_HIF_DMASHDL_GROUP14_CTRL	(DMASHDL_BASE + 0x58)
#define MT_HIF_DMASHDL_GROUP15_CTRL	(DMASHDL_BASE + 0x5c)

#define MT_HIF_DMASHDL_SHDL_SET0	(DMASHDL_BASE + 0xb0)
#define MT_HIF_DMASHDL_SHDL_SET1	(DMASHDL_BASE + 0xb4)


#define MT_HIF_DMASHDL_SLOT_SET0	(DMASHDL_BASE + 0xc4)
#define MT_HIF_DMASHDL_SLOT_SET1	(DMASHDL_BASE + 0xc8)


#define MT_HIF_DMASHDL_Q_MAP0		(DMASHDL_BASE + 0xd0)
#define MT_HIF_DMASHDL_Q_MAP1		(DMASHDL_BASE + 0xd4)
#define MT_HIF_DMASHDL_Q_MAP2		(DMASHDL_BASE + 0xd8)
#define MT_HIF_DMASHDL_Q_MAP3		(DMASHDL_BASE + 0xdc)

#define MT_HIF_DMASHDL_PAGE_SET         (DMASHDL_BASE + 0x0c)

/* User program group sequence type control */
/* 0: user program group sequence order type, 1: pre define each slot group strict order */
#define DMASHDL_GROUP_SEQ_ORDER_TYPE   BIT(16)


#define MT_HIF_DMASHDL_CTRL_SIGNAL      (DMASHDL_BASE + 0x18)
#define DMASHDL_PLE_TXD_GT_MAX_SIZE_FLAG_CLR \
	BIT(0)  /* enable to clear the flag of PLE TXD size greater than ple max. size */
#define DMASHDL_HIF_ASK_SUB_ENA        BIT(16) /* enable packet in substration action from HIF ask period */
#define DMASHDL_PLE_SUB_ENA            BIT(17) /* enable packet in substration action from PLE */
#define DMASHDL_PLE_ADD_INT_REFILL_ENA BIT(29) /* enable terminate refill period when PLE release packet to do addition */
#define DMASHDL_PDMA_ADD_INT_REFILL_ENA \
	BIT(30) /* enable terminate refill period when packet in to do addition */
#define DMASHDL_PKTIN_INT_REFILL_ENA   BIT(31) /* enable terminate refill period when packet in to do substration */

#define MT_HIF_DMASHDL_ERROR_FLAG_CTRL  (DMASHDL_BASE + 0x9c)

#define MT_HIF_DMASHDL_STATUS_RD        (DMASHDL_BASE + 0x100)
#define MT_HIF_DMASHDL_STATUS_RD_GP0    (DMASHDL_BASE + 0x140)
#define MT_HIF_DMASHDL_STATUS_RD_GP1    (DMASHDL_BASE + 0x144)
#define MT_HIF_DMASHDL_STATUS_RD_GP2    (DMASHDL_BASE + 0x148)
#define MT_HIF_DMASHDL_STATUS_RD_GP3    (DMASHDL_BASE + 0x14c)
#define MT_HIF_DMASHDL_STATUS_RD_GP4    (DMASHDL_BASE + 0x150)
#define MT_HIF_DMASHDL_STATUS_RD_GP5    (DMASHDL_BASE + 0x154)
#define MT_HIF_DMASHDL_STATUS_RD_GP6    (DMASHDL_BASE + 0x158)
#define MT_HIF_DMASHDL_STATUS_RD_GP7    (DMASHDL_BASE + 0x15c)
#define MT_HIF_DMASHDL_STATUS_RD_GP8    (DMASHDL_BASE + 0x160)
#define MT_HIF_DMASHDL_STATUS_RD_GP9    (DMASHDL_BASE + 0x164)
#define MT_HIF_DMASHDL_STATUS_RD_GP10   (DMASHDL_BASE + 0x168)
#define MT_HIF_DMASHDL_STATUS_RD_GP11   (DMASHDL_BASE + 0x16c)
#define MT_HIF_DMASHDL_STATUS_RD_GP12   (DMASHDL_BASE + 0x170)
#define MT_HIF_DMASHDL_STATUS_RD_GP13   (DMASHDL_BASE + 0x174)
#define MT_HIF_DMASHDL_STATUS_RD_GP14   (DMASHDL_BASE + 0x178)
#define MT_HIF_DMASHDL_STATUS_RD_GP15   (DMASHDL_BASE + 0x17c)
#define MT_HIF_DMASHDLRD_GP_PKT_CNT_0   (DMASHDL_BASE + 0x180)
#define MT_HIF_DMASHDLRD_GP_PKT_CNT_1   (DMASHDL_BASE + 0x184)
#define MT_HIF_DMASHDLRD_GP_PKT_CNT_2   (DMASHDL_BASE + 0x188)
#define MT_HIF_DMASHDLRD_GP_PKT_CNT_3   (DMASHDL_BASE + 0x18c)
#define MT_HIF_DMASHDLRD_GP_PKT_CNT_4   (DMASHDL_BASE + 0x190)
#define MT_HIF_DMASHDLRD_GP_PKT_CNT_5   (DMASHDL_BASE + 0x194)
#define MT_HIF_DMASHDLRD_GP_PKT_CNT_6   (DMASHDL_BASE + 0x198)
#define MT_HIF_DMASHDLRD_GP_PKT_CNT_7   (DMASHDL_BASE + 0x19c)

#define DMASHDL_RSV_CNT_MASK           (0xfff << 16)
#define DMASHDL_SRC_CNT_MASK           (0xfff << 0)
#define DMASHDL_RSV_CNT_OFFSET         16
#define DMASHDL_SRC_CNT_OFFSET         0
#define DMASHDL_FREE_PG_CNT_MASK       (0xfff << 16)
#define DMASHDL_FFA_CNT_MASK           (0xfff << 0)
#define DMASHDL_FREE_PG_CNT_OFFSET     16
#define DMASHDL_FFA_CNT_OFFSET         0
#define DMASHDL_MAX_QUOTA_MASK         (0xfff << 16)
#define DMASHDL_MIN_QUOTA_MASK         (0xfff << 0)
#define DMASHDL_MAX_QUOTA_OFFSET       16
#define DMASHDL_MIN_QUOTA_OFFSET       0

#define DMASHDL_MIN_QUOTA_NUM(p) (((p) & 0xfff) << DMASHDL_MIN_QUOTA_OFFSET)
#define GET_DMASHDL_MIN_QUOTA_NUM(p) (((p) & DMASHDL_MIN_QUOTA_MASK) >> DMASHDL_MIN_QUOTA_OFFSET)

#define DMASHDL_MAX_QUOTA_NUM(p) (((p) & 0xfff) << DMASHDL_MAX_QUOTA_OFFSET)
#define GET_DMASHDL_MAX_QUOTA_NUM(p) (((p) & DMASHDL_MAX_QUOTA_MASK) >> DMASHDL_MAX_QUOTA_OFFSET)


#define ODD_GROUP_ASK_CN_MASK (0xff << 16)
#define ODD_GROUP_ASK_CN_OFFSET 16
#define GET_ODD_GROUP_ASK_CNT(p) (((p) & ODD_GROUP_ASK_CN_MASK) >> ODD_GROUP_ASK_CN_OFFSET)
#define ODD_GROUP_PKT_IN_CN_MASK (0xff << 24)
#define ODD_GROUP_PKT_IN_CN_OFFSET 24
#define GET_ODD_GROUP_PKT_IN_CNT(p) (((p) & ODD_GROUP_PKT_IN_CN_MASK) >> ODD_GROUP_PKT_IN_CN_OFFSET)
#define EVEN_GROUP_ASK_CN_MASK (0xff << 0)
#define EVEN_GROUP_ASK_CN_OFFSET 0
#define GET_EVEN_GROUP_ASK_CNT(p) (((p) & EVEN_GROUP_ASK_CN_MASK) >> EVEN_GROUP_ASK_CN_OFFSET)
#define EVEN_GROUP_PKT_IN_CN_MASK (0xff << 8)
#define EVEN_GROUP_PKT_IN_CN_OFFSET 8
#define GET_EVEN_GROUP_PKT_IN_CNT(p) (((p) & EVEN_GROUP_PKT_IN_CN_MASK) >> EVEN_GROUP_PKT_IN_CN_OFFSET)
#define HIF_DMASHDL_IO_READ32(_A, _R, _pV) \
	do {\
		UINT32 restore_remap_addr;\
		HW_IO_READ32(_A, 0x2504, &restore_remap_addr);\
		\
		HW_IO_WRITE32(_A, 0x2504, DMASHDL_BASE);\
		HW_IO_READ32(_A, (0x80000+((UINT32)_R&0xffff)), _pV);\
		\
		HW_IO_WRITE32(_A, 0x2504, restore_remap_addr);\
	} while (0)

#define HIF_DMASHDL_IO_WRITE32(_A, _R, _pV) \
	do {\
		UINT32 restore_remap_addr;\
		\
		HW_IO_READ32(_A, 0x2504, &restore_remap_addr);\
		\
		HW_IO_WRITE32(_A, 0x2504, DMASHDL_BASE);\
		HW_IO_WRITE32(_A, (0x80000+((UINT32)_R&0xffff)), _pV);\
		\
		HW_IO_WRITE32(_A, 0x2504, restore_remap_addr);\
	} while (0)


#endif /* __DMA_SCH_H__ */


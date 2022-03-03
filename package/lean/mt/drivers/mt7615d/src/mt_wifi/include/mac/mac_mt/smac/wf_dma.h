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
	wf_dma.h

	Abstract:
	Ralink Wireless Chip MAC related definition & structures

	Revision History:
	Who			When		  What
	--------	----------	  ----------------------------------------------
*/


#ifndef __WF_DMA_H__
#define __WF_DMA_H__

#define WF_DMA_BASE		0x21C00
#define DMA_DCR0		(WF_DMA_BASE + 0x000)
#define MAX_RX_PKT_LENGTH_MASK (0x3fff << 2)
#define MAX_RX_PKT_LENGTH(p) (((p) & 0x3fff) << 2)
#define GET_MAX_PKT_LENGTH(p) (((p) & MAX_RX_PKT_LENGTH_MASK) >> 2)
#define DAMSDU_EN (1 << 16)
#define VEC_DROP_EN (1 << 17)
#define RX_HDR_TRANS_CHK_BSSID (1 << 18)
#define RX_HD_TRANS_EN (1 << 19)
#define PCP_EQ_TID (1 << 20)
#define RX_INS_VLAN (1 << 21)
#define RX_RM_VLAN (1 << 22)
#define RX_HDR_TRANS_MODE (1 << 23)
#define ETBL0_EN (1 << 24)
#define ETBL1_EN (1 << 25)
#define ETBL2_EN (1 << 26)
#define ETBL3_EN (1 << 27)
#define ETBL4_EN (1 << 28)
#define ETBL5_EN (1 << 29)
#define ETBL6_EN (1 << 30)
#define ETBL7_EN (1 << 31)

#define DMA_DCR1		(WF_DMA_BASE + 0x004)	/* 0x21c04 */
#define RXSM_GROUP1_EN	(1 << 11)
#define RXSM_GROUP2_EN	(1 << 12)
#define RXSM_GROUP3_EN	(1 << 13)


#define DMA_FQCR0        (WF_DMA_BASE + 0x008)   /* 0x21c08 */
#define DMA_FQCR0_FQ_EN                     BIT31
#define DMA_FQCR0_FQ_STS                    BIT30
#define DMA_FQCR0_FQ_MODE_OMAC              BIT29
#define DMA_FQCR0_FQ_MODE_WIDX              ~BIT29
#define DMA_FQCR0_FQ_DEST_QID_MASK          (0x1f)
#define DMA_FQCR0_FQ_DEST_QID(p)            (((p) & DMA_FQCR0_FQ_DEST_QID_MASK) << 24)
#define DMA_FQCR0_FQ_DEST_PID_MASK          (0x3)
#define DMA_FQCR0_FQ_DEST_PID(p)            (((p) & DMA_FQCR0_FQ_DEST_PID_MASK) << 22)
#define DMA_FQCR0_FQ_TARG_QID_MASK          (0x1f)
#define DMA_FQCR0_FQ_TARG_QID(p)            (((p) & DMA_FQCR0_FQ_TARG_QID_MASK) << 16)
#define DMA_FQCR0_FQ_TARG_OM_MASK           (0x3f)
#define DMA_FQCR0_FQ_TARG_OM(p)             (((p) & DMA_FQCR0_FQ_TARG_OM_MASK) << 8)
#define DMA_FQCR0_FQ_TARG_WIDX_MASK         (0xff)
#define DMA_FQCR0_FQ_TARG_WIDX(p)           (((p) & DMA_FQCR0_FQ_TARG_WIDX_MASK))

#define FQ_MODE_WIDX    0
#define FQ_MODE_OMIDX   1

#define DMA_FQCR1        (WF_DMA_BASE + 0x00c)   /* 0x21c0c */


#define DMA_RCFR0		(WF_DMA_BASE + 0x070)	/* 0x21c70 */
#define DMA_VCFR0		(WF_DMA_BASE + 0x07c)	/* 0x21c7c */

#define TXS_ME (1 << 0)
#define TXS_RE (1 << 1)
#define TXS_LE (1 << 2)
#define TXS_BE (1 << 3)
#define TXS_TXOP (1 << 4)
#define TXS_PS (1 << 5)
#define TXS_BAF (1 << 6)

#define VTR0 (WF_DMA_BASE + 0x010)
#define VTR1 (WF_DMA_BASE + 0x014)
#define VTR2 (WF_DMA_BASE + 0x018)
#define VTR3 (WF_DMA_BASE + 0x01c)
#define VTR4 (WF_DMA_BASE + 0x020)
#define VTR5 (WF_DMA_BASE + 0x024)
#define VTR6 (WF_DMA_BASE + 0x028)
#define VTR7 (WF_DMA_BASE + 0x02c)
#define VTR8 (WF_DMA_BASE + 0x030)
#define VTR9 (WF_DMA_BASE + 0x034)
#define VTR10 (WF_DMA_BASE + 0x038)
#define VTR11 (WF_DMA_BASE + 0x03c)
#define VTR12 (WF_DMA_BASE + 0x040)
#define VTR13 (WF_DMA_BASE + 0x044)
#define VTR14 (WF_DMA_BASE + 0x048)
#define VTR15 (WF_DMA_BASE + 0x04c)
#define VTR16 (WF_DMA_BASE + 0x050)
#define VTR17 (WF_DMA_BASE + 0x054)
#define BSSIDxx0_TCL_VID_MASK (0xfff)
#define BSSIDxx0_TCL_VID(p) (((p) & 0xfff))
#define GET_BSSIDxx0_TCL_VID(p) (((p) & BSSIDxx0_TCL_VID_MASK))
#define BSSIDxx0_TCL_CFI (1 << 12)
#define BSSIDxx0_TCL_PCP_MASK (0x7 << 13)
#define BSSIDxx0_TCL_PCP(p) (((p) & 0x7) << 13)
#define GET_BSSIDxx0_TCP_PCP(p) (((p) & BSSIDxx0_TCL_PCP_MASK) >> 13)
#define BSSIDxx1_TCL_VID_MASK (0xfff << 16)
#define BSSIDxx1_TCL_VID(p) (((p) & 0xfff) << 16)
#define GET_BSSIDxx1_TCL_VID(p) (((p) & BSSIDxx1_TCL_VID_MASK) >> 16)
#define BSSIDxx1_TCL_CFI (1 << 28)
#define BSSIDxx1_TCL_PCP_MASK (0x7 << 29)
#define BSSIDxx1_TCL_PCP(p) (((p) & 0x7) << 29)
#define GET_BSSIDxx1_TCP_PCP(p) (((p) & BSSIDxx1_TCL_PCP_MASK) >> 29)

enum {
	ETBL_INDEX0,
	ETBL_INDEX1,
	ETBL_INDEX2,
	ETBL_INDEX3,
	ETBL_INDEX4,
	ETBL_INDEX5,
	ETBL_INDEX6,
	ETBL_INDEX7,
};

#define ETBLR0 (WF_DMA_BASE + 0x060)
#define ETBL0_MASK (0xffff)
#define ETBL0(p) (((p) & 0xffff))
#define GET_ETBL0(p) (((p) & ETBL0_MASK))
#define ETBL1_MASK (0xffff << 16)
#define ETBL1(p) (((p) & 0xffff) << 16)
#define GET_ETBL1(p) (((p) & ETBL1_MASK) >> 16)

#define ETBLR1 (WF_DMA_BASE + 0x064)
#define ETBL2_MASK (0xffff)
#define ETBL2(p) (((p) & 0xffff))
#define GET_ETBL2(p) (((p) & ETBL2_MASK))
#define ETBL3_MASK (0xffff << 16)
#define ETBL3(p) (((p) & 0xffff) << 16)
#define GET_ETBL3(p) (((p) & ETBL3_MASK) >> 16)

#define ETBLR2 (WF_DMA_BASE + 0x068)
#define ETBL4_MASK (0xffff)
#define ETBL4(p) (((p) & 0xffff))
#define GET_ETBL4(p) (((p) & ETBL4_MASK))
#define ETBL5_MASK (0xffff << 16)
#define ETBL5(p) (((p) & 0xffff) << 16)
#define GET_ETBL5(p) (((p) & ETBL5_MASK) >> 16)

#define ETBLR3 (WF_DMA_BASE + 0x06C)
#define ETBL6_MASK (0xffff)
#define ETBL6(p) (((p) & 0xffff))
#define GET_ETBL6(p) (((p) & ETBL6_MASK))
#define ETBL7_MASK (0xffff << 16)
#define ETBL7(p) (((p) & 0xffff) << 16)
#define GET_ETBL7(p) (((p) & ETBL7_MASK) >> 16)

#define DMA_TCFR0		(WF_DMA_BASE + 0x080)	/* 0x21c80 */
#define TXS2M_BIT_MAP_MASK (0x7f)
#define TXS2M_BIT_MAP(p) (((p) & 0x7f))
#define GET_TXS2M_BIT_MAP(p) (((p) & TXS2M_BIT_MAP_MASK))
#define TXS2M_AGG_CNT_MASK (0x1f << 8)
#define TXS2M_AGG_CNT(p) (((p) & 0x1f) << 8)
#define GET_TXS2M_AGG_CNT(p) (((p) & TXS2M_AGG_CNT_MASK) >> 8)
#define TXS2M_QID_MASK (0x3 << 14)
#define TXS2M_QID(p) (((p) & 0x3) << 14)
#define GET_TXS2M_QID(p) (((p) & TXS2M_QID_MASK) >> 14)
#define TXS2M_TOUT_MASK (0xfff << 16)
#define TXS2M_TOUT(p) (((p) & 0xfff) << 16)
#define GET_TXS2M_TOUT(p) (((p) & TXS2M_TOUT_MASK) >> 16)
#define DMA_TCFR1		(WF_DMA_BASE + 0x084)	/* 0x21c84 */
#define TXS2H_BIT_MAP_MASK (0x7f)
#define TXS2H_BIT_MAP(p) (((p) & 0x7f))
#define GET_TXS2H_BIT_MAP(p) (((p) & TXS2H_BIT_MAP_MASK))
#define TXS2H_AGG_CNT_MASK (0x1f << 8)
#define TXS2H_AGG_CNT(p) (((p) & 0x1f) << 8)
#define GET_TXS2H_AGG_CNT(p) (((p) & TXS2H_AGG_CNT_MASK) >> 8)
#define TXS2H_QID (1 << 14)
#define TXS2H_TOUT_MASK (0xfff << 16)
#define TXS2H_TOUT(p) (((p) & 0xfff) << 16)
#define GET_TXS2H_TOUT(p) (((p) & TXS2H_TOUT_MASK) >> 16)
#define DMA_TMCFR0		(WF_DMA_BASE + 0x088)	/* 0x21c88 */

#endif /* __WF_DMA_H__ */


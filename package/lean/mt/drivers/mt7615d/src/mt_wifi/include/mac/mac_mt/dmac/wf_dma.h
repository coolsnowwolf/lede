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

#define WF_DMA_BASE		0x21800
#define DMA_DCR0		(WF_DMA_BASE + 0x000)
#define DMA_DCR0_RX_HDR_TRANS_CHK_BSSID	(1<<18)
#define DMA_DCR0_RX_HDR_TRANS_EN_BIT	(1<<19)
#define DMA_DCR0_RX_HDR_TRANS_MODE_BIT	(1<<23)
#define DMA_DCR0_RX_RM_VLAN_BIT			(1<<22)
#define DMA_DCR0_RX_INS_VLAN_BIT			(1<<21)

#define DMA_DCR1		(WF_DMA_BASE + 0x004)	/* 0x21804 */
#define RXSM_GROUP1_EN	(1 << 11)
#define RXSM_GROUP2_EN	(1 << 12)
#define RXSM_GROUP3_EN	(1 << 13)
#define RHTR_AMS_VLAN_EN	(1<<15)

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

#define DMA_FQCR1        (WF_DMA_BASE + 0x00c)   /* 0x21c0c */

#ifdef VLAN_SUPPORT
#define DMA_VTR0			(WF_DMA_BASE + 0x010)
#define DMA_VTR2			(WF_DMA_BASE + 0x018)
#define DMA_VTR20			(WF_DMA_BASE + 0x0A0)
#define DMA_VTR24			(WF_DMA_BASE + 0x0F0)
#define DMA_VTR_GET_ADDR(_omac) \
			((_omac < 0x10) ? (DMA_VTR0 + ((_omac >> 1) << 2)) : \
				(_omac < 0x34) ? (DMA_VTR2 + (((_omac - 0x10) >> 1) << 2)) : \
					(_omac < 0x3c) ? (DMA_VTR20 + (((_omac - 0x34) >> 1) << 2)) : \
						(DMA_VTR24 + (((_omac - 0x3c) >> 1) << 2)))
#define DMA_VTR_SET_VID(_omac, _tci, _vid) \
				((_omac % 2) ? ((_tci & 0xF000FFFF) | (_vid << 16)) : ((_tci & 0xFFFFF000) | _vid))
#define DMA_VTR_SET_PCP(_omac, _tci, _pcp) \
				((_omac % 2) ? ((_tci & 0x1FFFFFFF) | (_pcp << 29)) : ((_tci & 0xFFFF1FFF) | (_pcp << 13)))
#endif /*VLAN_SUPPORT*/

#define DMA_BN0RCFR0		(WF_DMA_BASE + 0x070)	/* 0x21870 */
#define DMA_BN0RCFR1		(WF_DMA_BASE + 0x074)	/* 0x21874 */
#define DMA_BN0VCFR0		(WF_DMA_BASE + 0x07c)	/* 0x2187c */
#define DMA_BN1RCFR0		(WF_DMA_BASE + 0x0B0)	/* 0x218b0 */
#define DMA_BN1RCFR1		(WF_DMA_BASE + 0x0B4)	/* 0x218b4 */
#define DMA_BN1VCFR0		(WF_DMA_BASE + 0x0Bc)	/* 0x218bc */


#define DMA_BN0TCFR0		(WF_DMA_BASE + 0x080)	/* 0x21880 */
#define DMA_BN1TCFR0		(WF_DMA_BASE + 0x0c0)	/* 0x218c0 */
#define TXS_ME (1 << 0)
#define TXS_RE (1 << 1)
#define TXS_LE (1 << 2)
#define TXS_BE (1 << 3)
#define TXS_TXOP (1 << 4)
#define TXS_PS (1 << 5)
#define TXS_BAF (1 << 6)
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


#define DMA_BN0TCFR1		(WF_DMA_BASE + 0x084)	/* 0x21884 */
#define DMA_BN1TCFR1		(WF_DMA_BASE + 0x0c4)	/* 0x218c4 */
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

#define DMA_BN0TMCFR0		(WF_DMA_BASE + 0x088)	/* 0x21888 */
#define DMA_BN1TMCFR0		(WF_DMA_BASE + 0x0c8)	/* 0x218c8 */

#define DMA_DBG_00			(WF_DMA_BASE + 0x0d0)	/* 0x218d0 */
#define DMA_DBG_01			(WF_DMA_BASE + 0x0d4)	/* 0x218d4 */

#endif /* __WF_DMA_H__ */


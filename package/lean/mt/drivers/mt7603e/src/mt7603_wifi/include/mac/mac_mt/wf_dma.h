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
#define DMA_DCR1		(WF_DMA_BASE + 0x004)	/* 0x21c04 */
#define RXSM_GROUP1_EN	(1 << 11)
#define RXSM_GROUP2_EN	(1 << 12)
#define RXSM_GROUP3_EN	(1 << 13)
#define DMA_RCFR0		(WF_DMA_BASE + 0x070)	/* 0x21c70 */
#define DMA_VCFR0		(WF_DMA_BASE + 0x07c)	/* 0x21c7c */

#define TXS_ME (1 << 0)
#define TXS_RE (1 << 1)
#define TXS_LE (1 << 2)
#define TXS_BE (1 << 3)
#define TXS_TXOP (1 << 4)
#define TXS_PS (1 << 5)
#define TXS_BAF (1 << 6)
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


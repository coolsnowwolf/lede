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
	wf_agg.h

	Abstract:
	Ralink Wireless Chip MAC related definition & structures

	Revision History:
	Who			When		  What
	--------	----------	  ----------------------------------------------
*/


#ifndef __WF_AGG_H__
#define __WF_AGG_H__

#define WF_AGG_BASE		0x21200

#define AGG_ARCR			(WF_AGG_BASE + 0x010)	/* 0x21210 */
#define INI_RATE1               (0x1 << 0)
#define FB_SGI_DIS              (0x1 << 1)
#define RTS_RATE_DOWN_TH_MASK	(0x1F << 8)
#define RTS_RATE_DOWN_TH(p)		(((p) & 0x1F) << 8)
#define RATE_DOWN_EXTRA_RATIO_MASK	(0x3 << 16)
#define RATE_DOWN_EXTRA_RATIO(p)	(((p) & 0x3) << 16)
#define RATE_DOWN_EXTRA_RATIO_EN	(0x1 << 19)
#define RATE_UP_EXTRA_TH_MASK	(0x07 << 20)
#define RATE_UP_EXTRA_TH(p)		(((p) & 0x7) << 20)
#define SPE_DIS_TH_MASK         (0x0F << 24)
#define SPE_DIS_TH(p)           (((p) & 0xF) << 24)


#define AGG_ARUCR		(WF_AGG_BASE + 0x014)	/* 0x21214 */
#define RATE1_UP_MPDU_LIMIT_MASK (0x7)
#define RATE1_UP_MPDU_LINIT(p) (((p) & 0x7))
#define RATE2_UP_MPDU_LIMIT_MASK (0x7 << 4)
#define RATE2_UP_MPDU_LIMIT(p) (((p) & 0x7) << 4)
#define RATE3_UP_MPDU_LIMIT_MASK (0x7 << 8)
#define RATE3_UP_MPDU_LIMIT(p) (((p) & 0x7) << 8)
#define RATE4_UP_MPDU_LIMIT_MASK (0x7 << 12)
#define RATE4_UP_MPDU_LIMIT(p) (((p) & 0x7) << 12)
#define RATE5_UP_MPDU_LIMIT_MASK (0x7 << 16)
#define RATE5_UP_MPDU_LIMIT(p) (((p) & 0x7) << 16)
#define RATE6_UP_MPDU_LIMIT_MASK (0x7 << 20)
#define RATE6_UP_MPDU_LIMIT(p) (((p) & 0x7) << 20)
#define RATE7_UP_MPDU_LIMIT_MASK (0x7 << 24)
#define RATE7_UP_MPDU_LIMIT(p) (((p) & 0x7) << 24)
#define RATE8_UP_MPDU_LIMIT_MASK (0X7 << 28)
#define RATE8_UP_MPDU_LIMIT(p) (((p) & 0x7) << 28)

#define AGG_ARDCR		(WF_AGG_BASE + 0x018)	/* 0x21218 */
#define RATE1_DOWN_MPDU_LIMIT_MASK (0x7)
#define RATE1_DOWN_MPDU_LIMIT(p) (((p) & 0x7))
#define RATE2_DOWN_MPDU_LIMIT_MASK (0x7 << 4)
#define RATE2_DOWN_MPDU_LIMIT(p) (((p) & 0x7) << 4)
#define RATE3_DOWN_MPDU_LIMIT_MASK (0x7 << 8)
#define RATE3_DOWN_MPDU_LIMIT(p) (((p) & 0x7) << 8)
#define RATE4_DOWN_MPDU_LIMIT_MASK (0x7 << 12)
#define RATE4_DOWN_MPDU_LIMIT(p) (((p) & 0x7) << 12)
#define RATE5_DOWN_MPDU_LIMIT_MASK (0x7 << 16)
#define RATE5_DOWN_MPDU_LIMIT(p) (((p) & 0x7) << 16)
#define RATE6_DOWN_MPDU_LIMIT_MASK (0x7 << 20)
#define RATE6_DOWN_MPDU_LIMIT(p) (((p) & 0x7) << 20)
#define RATE7_DOWN_MPDU_LIMIT_MASK (0x7 << 24)
#define RATE7_DOWN_MPDU_LIMIT(p) (((p) & 0x7) << 24)
#define RATE8_DOWN_MPDU_LIMIT_MASK (0x7 << 28)
#define RATE8_DOWN_MPDU_LIMIT(p) (((p) & 0x7) << 28)

#define AGG_AALCR		(WF_AGG_BASE + 0x040)	/* 0x21240 */
#define AC0_AGG_LIMIT_MASK (0x3f)
#define AC0_AGG_LIMIT(p) (((p) & 0x3f))
#define GET_AC0_AGG_LIMIT(p) (((p) & AC0_AGG_LIMIT_MASK))
#define AC1_AGG_LIMIT_MASK (0x3f << 8)
#define AC1_AGG_LIMIT(p) (((p) & 0x3f) << 8)
#define GET_AC1_AGG_LIMIT(p) (((p) & AC1_AGG_LIMIT_MASK) >> 8)
#define AC2_AGG_LIMIT_MASK (0x3f << 16)
#define AC2_AGG_LIMIT(p) (((p) & 0x3f) << 16)
#define GET_AC2_AGG_LIMIT(p) (((p) & AC2_AGG_LIMIT_MASK) >> 16)
#define AC3_AGG_LIMIT_MASK (0x3f << 24)
#define AC3_AGG_LIMIT(p) (((p) & 0x3f) << 24)
#define GET_AC3_AGG_LIMIT(p) (((p) & AC3_AGG_LIMIT_MASK) >> 24)

#define AGG_AALCR1		(WF_AGG_BASE + 0x044)	/* 0x21244 */
#define AC10_AGG_LIMIT_MASK (0x3f)
#define AC10_AGG_LIMIT(p) (((p) & 0x3f))
#define GET_AC10_AGG_LIMIT(p) (((p) & AC10_AGG_LIMIT_MASK))
#define AC11_AGG_LIMIT_MASK (0x3f << 8)
#define AC11_AGG_LIMIT(p) (((p) & 0x3f) << 8)
#define GET_AC11_AGG_LIMIT(p) (((p) & AC11_AGG_LIMIT_MASK) >> 8)
#define AC12_AGG_LIMIT_MASK (0x3f << 16)
#define AC12_AGG_LIMIT(p) (((p) & 0x3f) << 16)
#define GET_AC12_AGG_LIMIT(p) (((p) & AC12_AGG_LIMIT_MASK) >> 16)
#define AC13_AGG_LIMIT_MASK (0x3f << 24)
#define AC13_AGG_LIMIT(p) (((p) & 0x3f) << 24)
#define GET_AC13_AGG_LIMIT(p) (((p) & AC13_AGG_LIMIT_MASK) >> 24)

#define AGG_AWSCR		(WF_AGG_BASE + 0x048)	/* 0x21248 */
#define WINSIZE0_MASK (0x3f)
#define WINSIZE0(p) (((p) & 0x3f))
#define GET_WINSIZE0(p) (((p) & WINSIZE0_MASK))
#define WINSIZE1_MASK (0x3f << 8)
#define WINSIZE1(p) (((p) & 0x3f) << 8)
#define GET_WINSIZE1(p) (((p) & WINSIZE1_MASK) >> 8)
#define WINSIZE2_MASK (0x3f << 16)
#define WINSIZE2(p) (((p) & 0x3f) << 16)
#define GET_WINSIZE2(p) (((p) & WINSIZE2_MASK) >> 16)
#define WINSIZE3_MASK (0x3f << 24)
#define WINSIZE3(p) (((p) & 0x3f) << 24)
#define GET_WINSIZE3(p) (((p) & WINSIZE3_MASK) >> 24)

#define AGG_AWSCR1		(WF_AGG_BASE + 0x04c)	/* 0x2124c */
#define WINSIZE4_MASK (0x3f)
#define WINSIZE4(p) (((p) & 0x3f))
#define GET_WINSIZE4(p) (((p) & WINSIZE4_MASK))
#define WINSIZE5_MASK (0x3f << 8)
#define WINSIZE5(p) (((p) & 0x3f) << 8)
#define GET_WINSIZE5(p) (((p) & WINSIZE5_MASK) >> 8)
#define WINSIZE6_MASK (0x3f << 16)
#define WINSIZE6(p) (((p) & 0x3f) << 16)
#define GET_WINSIZE6(p) (((p) & WINSIZE6_MASK) >> 16)
#define WINSIZE7_MASK (0x3f << 24)
#define WINSIZE7(p) (((p) & 0x3f) << 24)
#define GET_WINSIZE7(p) (((p) & WINSIZE7_MASK) >> 24) 

#define AGG_PCR			(WF_AGG_BASE + 0x050)	/* 0x21250 */
#define MM_PROTECTION		(1 << 16)
#define GET_MM_PROTECTION(p) (((p) & MM_PROTECTION) >> 16)
#define GF_PROTECTION		(1 << 17)
#define GET_GF_PROTECTION(p) (((p) & GF_PROTECTION) >> 17)
#define PROTECTION_MODE 	(1 << 19)
#define GET_PROTECTION_MODE(p) (((p) & PROTECTION_MODE) >> 19)
#define BW40_PROTECTION		(1 << 20)
#define GET_BW40_PROTECTION(p) (((p) & BW40_PROTECTION) >> 20)
#define RIFS_PROTECTION		(1 << 21)
#define GET_RIFS_PROTECTION(p) (((p) & RIFS_PROTECTION) >> 21)
#define BW80_PROTECTION		(1 << 22)
#define GET_BW80_PROTECTION(p) (((p) & BW80_PROTECTION) >> 22)
#define BW160_PROTECTION	(1 << 23)
#define GET_BW160_PROTECTION(p) (((p) & BW160_PROTECTION) >> 23)
#define ERP_PROTECTION_MASK (0x1f << 24)
#define ERP_PROTECTION(p)	(((p) & 0x1f) << 24)
#define GET_ERP_PROTECTION(p) (((p) & ERP_PROTECTION_MASK) >> 24)


#define AGG_PCR1		(WF_AGG_BASE + 0x054)	/* 0x21254 */
#define RTS_THRESHOLD_MASK	(0xfffff)
#define RTS_THRESHOLD(p)	(((p) & 0xfffff))
#define GET_RTS_THRESHOLD(p) (((p) & RTS_THRESHOLD_MASK))
#define RTS_PKT_NUM_THRESHOLD_MASK	(0x7f << 25)
#define RTS_PKT_NUM_THRESHOLD(p)	(((p) & 0x7f) << 25)
#define GET_RTS_PKT_NUM_THRESHOLD(p) (((p) & RTS_PKT_NUM_THRESHOLD_MASK) >> 25)


#define AGG_ASRCR		(WF_AGG_BASE + 0x060)	/* 0x21260 */
#define AGG_BQCR		(WF_AGG_BASE + 0x064)	/* 0x21264 */
#define AGG_BCR		(WF_AGG_BASE + 0x068)	/* 0x21268 */

#define AGG_ACR		(WF_AGG_BASE + 0x070)	/* 0x21270 */
#define AMPDU_NO_BA_RULE (1 << 0)
#define GET_AMPDU_NO_BA_RULE(p) (((p) & AMPDU_NO_BA_RULE))
#define AGG_ACR_AMPDU_NO_BA_AR_RULE_MASK	(1 << 1)
#define AGG_ACR_AMPDU_NO_BA_AR_RULE(p)		(((p) & 0x1) << 1)
#define GET_AGG_ACR_AMPDU_NO_BA_AR_RULE(p) (((p) & AGG_ACR_AMPDU_NO_BA_AR_RULE_MASK) >> 1)

#define BAR_RATE_TX_RATE_MASK (0x3f << 20)
#define BAR_RATE_TX_RATE(p) (((p) & 0x3f) << 20)
#define GET_BAR_RATE_TX_RATE(p) (((p) & BAR_RATE_TX_RATE_MASK) >> 20)
#define BAR_RATE_TX_MODE_MASK (0x7 << 26)
#define BAR_RATE_TX_MODE(p) (((p) & 0x7) << 26)
#define GET_BAR_RATE_TX_MODE(p) (((p) & BAR_RATE_TX_MODE_MASK) >> 26)
#define BAR_RATE_NSTS_MASK (0x3 << 29)
#define BAR_RATE_NSTS(p) (((p) & 0x3) << 29)
#define GET_BAR_RATE_NSTS(p) (((p) & BAR_RATE_NSTS_MASK) >> 29)
#define BAR_RATE_STBC (1 << 31)
#define GET_BAR_RATE_STBC(p) (((p) & BAR_RATE_STBC) >> 31)

#define AGG_ACR1		(WF_AGG_BASE + 0x074)	/* 0x21274 */
#define AGG_BWCR		(WF_AGG_BASE + 0x0ec)	/* 0x212ec */
#define AGG_MRCR		(WF_AGG_BASE + 0x0f4)	/* 0x212f4 */
#define RTS_RTY_CNT_LIMIT_MASK (0x1f << 7)
#define RTS_RTY_CNT_LIMIT(p) (((p) & 0x1f) << 7)
#define GET_RTS_RTY_CNT_LIMIT(p) (((p) & RTS_RTY_CNT_LIMIT_MASK) >> 7) 
#define BAR_TX_CNT_LIMIT_MASK (0x0f << 12)
#define BAR_TX_CNT_LIMIT(p) (((p) & 0x0f) << 12)
#define GET_BAR_TX_CNT_LIMIT(p) (((p) & BAR_TX_CNT_LIMIT_MASK) >> 12)
#define AGG_DSCR1		(WF_AGG_BASE + 0xb4)	/* 0x212b4 */
#define AGG_TEMP		(WF_AGG_BASE + 0xd8)

#endif /* __WF_AGG_H__ */



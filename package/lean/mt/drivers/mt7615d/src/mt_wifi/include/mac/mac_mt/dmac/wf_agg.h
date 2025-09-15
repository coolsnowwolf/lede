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

#define WF_AGG_BASE		0x20a00

#define AGG_TPCR			(WF_AGG_BASE + 0x000)	/* 0x20a00 */

#define AGG_ARCR			(WF_AGG_BASE + 0x010)	/* 0x20a10 */
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
#define DUAL_BTIM_EN		(0x1 << 31)

#define AGG_ARCR1			(WF_AGG_BASE + 0x014)	/* 0x20a14 */

#define AGG_ARUCR		(WF_AGG_BASE + 0x018)	/* 0x20a18 */
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

#define AGG_ARDCR		(WF_AGG_BASE + 0x01c)	/* 0x20a1c */
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

#define AGG_FRM1RR0	(WF_AGG_BASE + 0x020)
#define FRM1_RATE0(_p)		((_p) & 0x0fff)
#define FRM1_RATE1(_p)		(((_p) & 0x0fff) << 16)

#define AGG_FRM1RR1	(WF_AGG_BASE + 0x024)
#define FRM1_RATE2(_p)		((_p) & 0x0fff)
#define FRM1_RATE3(_p)		(((_p) & 0x0fff) << 16)

#define AGG_FRM1RR2	(WF_AGG_BASE + 0x028)
#define FRM1_RATE4(_p)		((_p) & 0x0fff)
#define FRM1_RATE5(_p)		(((_p) & 0x0fff) << 16)

#define AGG_FRM1RR3	(WF_AGG_BASE + 0x02c)
#define FRM1_RATE6(_p)		((_p) & 0x0fff)
#define FRM1_RATE7(_p)		(((_p) & 0x0fff) << 16)

#define AGG_FRM1RR4	(WF_AGG_BASE + 0x030)
#define FRM1_RATE8(_p)		((_p) & 0x0fff)
#define FRM1_RATE9(_p)		(((_p) & 0x0fff) << 16)

#define AGG_FRM1RR5	(WF_AGG_BASE + 0x034)
#define FRM1_RATE10(_p)		((_p) & 0x0fff)
#define FRM1_RATE11(_p)		(((_p) & 0x0fff) << 16)

#define AGG_FRM1RR6	(WF_AGG_BASE + 0x038)
#define FRM1_RATE12(_p)		((_p) & 0x0fff)
#define FRM1_RATE13(_p)		(((_p) & 0x0fff) << 16)

#define AGG_FRM1RR7	(WF_AGG_BASE + 0x03c)
#define FRM1_RATE14(_p)		((_p) & 0x0fff)
#define FRM1_RATE15(_p)		(((_p) & 0x0fff) << 16)


#define AGG_AALCR0		(WF_AGG_BASE + 0x040)	/* 0x20a40 */
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

#define AGG_AALCR2		(WF_AGG_BASE + 0x048)	/* 0x21248 */
#define AC20_AGG_LIMIT_MASK (0x3f)
#define AC20_AGG_LIMIT(p) (((p) & 0x3f))
#define GET_AC20_AGG_LIMIT(p) (((p) & AC10_AGG_LIMIT_MASK))
#define AC21_AGG_LIMIT_MASK (0x3f << 8)
#define AC21_AGG_LIMIT(p) (((p) & 0x3f) << 8)
#define GET_AC21_AGG_LIMIT(p) (((p) & AC11_AGG_LIMIT_MASK) >> 8)
#define AC22_AGG_LIMIT_MASK (0x3f << 16)
#define AC22_AGG_LIMIT(p) (((p) & 0x3f) << 16)
#define GET_AC22_AGG_LIMIT(p) (((p) & AC12_AGG_LIMIT_MASK) >> 16)
#define AC23_AGG_LIMIT_MASK (0x3f << 24)
#define AC23_AGG_LIMIT(p) (((p) & 0x3f) << 24)
#define GET_AC23_AGG_LIMIT(p) (((p) & AC13_AGG_LIMIT_MASK) >> 24)

#define AGG_AALCR3		(WF_AGG_BASE + 0x04c)	/* 0x20a4c */
#define AC30_AGG_LIMIT_MASK (0x3f)
#define AC30_AGG_LIMIT(p) (((p) & 0x3f))
#define GET_AC30_AGG_LIMIT(p) (((p) & AC10_AGG_LIMIT_MASK))
#define AC31_AGG_LIMIT_MASK (0x3f << 8)
#define AC31_AGG_LIMIT(p) (((p) & 0x3f) << 8)
#define GET_AC31_AGG_LIMIT(p) (((p) & AC11_AGG_LIMIT_MASK) >> 8)
#define AC32_AGG_LIMIT_MASK (0x3f << 16)
#define AC32_AGG_LIMIT(p) (((p) & 0x3f) << 16)
#define GET_AC32_AGG_LIMIT(p) (((p) & AC12_AGG_LIMIT_MASK) >> 16)
#define AC33_AGG_LIMIT_MASK (0x3f << 24)
#define AC33_AGG_LIMIT(p) (((p) & 0x3f) << 24)
#define GET_AC33_AGG_LIMIT(p) (((p) & AC13_AGG_LIMIT_MASK) >> 24)


#define AGG_AWSCR0		(WF_AGG_BASE + 0x050)	/* 0x20a50 */
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

#define AGG_AWSCR1		(WF_AGG_BASE + 0x054)	/* 0x20a54 */
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


#define AGG_PCR			(WF_AGG_BASE + 0x058)	/* 0x20a50 */
#define MM_PROTECTION		(1 << 16)
#define GET_MM_PROTECTION(p) (((p) & MM_PROTECTION) >> 16)
#define GF_PROTECTION		(1 << 17)
#define GET_GF_PROTECTION(p) (((p) & GF_PROTECTION) >> 17)
#define PROTECTION_MODE	(1 << 19)
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


#define AGG_PCR1		(WF_AGG_BASE + 0x05c)
#define RTS_THRESHOLD_MASK	(0xfffff)
#define RTS_THRESHOLD(p)	(((p) & 0xfffff))
#define GET_RTS_THRESHOLD(p) (((p) & RTS_THRESHOLD_MASK))
#define RTS_PKT_NUM_THRESHOLD_MASK	(0x7f << 25)
#define RTS_PKT_NUM_THRESHOLD(p)	(((p) & 0x7f) << 25)
#define GET_RTS_PKT_NUM_THRESHOLD(p) (((p) & RTS_PKT_NUM_THRESHOLD_MASK) >> 25)

#define AGG_PCR2		(WF_AGG_BASE + 0x15c)

#define AGG_ASRCR0		(WF_AGG_BASE + 0x060)	/* 0x20a60 */
#define AGG_ASRCR1		(WF_AGG_BASE + 0x064)	/* 0x20a60 */
#define AGG_BCR		(WF_AGG_BASE + 0x068)	/* 0x20a68 */

/* TODO: shiang-MT7615, document mismatch!! */
#define AGG_ACR0		(WF_AGG_BASE + 0x070)	/* 0x20a70 */
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

#define AGG_ACR1		(WF_AGG_BASE + 0x170)	/* 0x20b70 */
#define AGG_BWCR		(WF_AGG_BASE + 0x078)	/* 0x20a78 */

#define AGG_MRCR		(WF_AGG_BASE + 0x07c)	/* 0x20a7c */
#define RTY_MODE0_MASK	(0x3)
#define RTY_MODE0(_p)	((_p) & 0x3)
#define RTY_MODE1_MASK	((0x3) << 16)
#define RTY_MODE1(_p)	(((_p) & 0x3) << 16)
#define RTS_RTY_CNT_LIMIT_MASK (0x1f << 7)
#define RTS_RTY_CNT_LIMIT(p) (((p) & 0x1f) << 7)
#define GET_RTS_RTY_CNT_LIMIT(p) (((p) & RTS_RTY_CNT_LIMIT_MASK) >> 7)
#define BAR_TX_CNT_LIMIT_MASK (0x0f << 12)
#define BAR_TX_CNT_LIMIT(p) (((p) & 0x0f) << 12)
#define GET_BAR_TX_CNT_LIMIT(p) (((p) & BAR_TX_CNT_LIMIT_MASK) >> 12)

/* the rate is used to indicate the second Tim frame rate, that is auto-gened by HW.
    per spec, it's shall be low rate one.
    low rate is same mcs or modulation as Beacon frame.
*/
#define AGG_BTIMRR0     (WF_AGG_BASE + 0x080)
#define AGG_BTIMRR1     (WF_AGG_BASE + 0x084)
#define AGG_BTIMRR2     (WF_AGG_BASE + 0x088)
#define AGG_BTIMRR3     (WF_AGG_BASE + 0x08c)
#define AGG_BTIMRR4     (WF_AGG_BASE + 0x090)
#define AGG_BTIMRR5     (WF_AGG_BASE + 0x094)
#define AGG_BTIMRR6     (WF_AGG_BASE + 0x098)
#define AGG_BTIMRR7     (WF_AGG_BASE + 0x09c)
#define AGG_BTIMRR8     (WF_AGG_BASE + 0x0a0)
#define AGG_BTIMRR9     (WF_AGG_BASE + 0x0a4)

typedef enum _AGG_TX_MODE_T {
	Legacy_CCK =  0x0,
	Legacy_OFDM,
	HT_MIXED,
	HT_GF,
	VHT
} AGG_TX_MODE_T;

typedef enum _AGG_CCK_RATE_T {
	LONG_PREAMBLE_1M = 0x0,
	LONG_PREAMBLE_2M = 0x1,
	LONG_PREAMBLE_5M = 0x2,
	LONG_PREAMBLE_11M = 0x3,
	SHORT_PREAMBLE_2M = 0x5,
	SHORT_PREAMBLE_5M = 0x6,
	SHORT_PREAMBLE_11M = 0x7
} AGG_CCK_RATE_T;

typedef enum _AGG_OFDM_RATE_T {
	TIM_OFDM_6M = 0xB,
	TIM_OFDM_9M = 0xF,
	TIM_OFDM_12M = 0xA,
	TIM_OFDM_18M = 0xE,
	TIM_OFDM_24M = 0x9,
	TIM_OFDM_36M = 0xD,
	TIM_OFDM_48M = 0x8,
	TIM_OFDM_54M = 0xC
} AGG_OFDM_RATE_T;

/* it's for the second that auto-gen by HW. */
#define AGG_BTIMRR_TX_MODE_MASK         (0x7)
#define AGG_BTIMRR_TX_MODE(p)           (((p) & AGG_BTIMRR_TX_MODE_MASK) << 6)
#define AGG_BTIMRR_TX_RATE_MASK         (0x3f)
#define AGG_BTIMRR_TX_RATE(p)           (((p) & AGG_BTIMRR_TX_RATE_MASK))

#define AGG_BTIMRR_ODD_OMAC_IDX_RATE(p)           (((p) << 16))

#define AGG_DSCR1		(WF_AGG_BASE + 0xb4)	/* 0x20ab4 */

#define AGG_MUCR            (WF_AGG_BASE + 0xe0)       /* 0x20ae0 */
#define MUCR_PRIM_BAR_BIT   (15)
#define MUCR_PRIM_BAR_MASK  (1<<15)

#define AGG_SCR			(WF_AGG_BASE + 0xfc)	/* 0x20afc */
#define NLNAV_MID_PTEC_DIS  (1<<3)
#define RTS_FAIL_CHARGE_DIS  (1<<27)

#define AGG_ATCR0               (WF_AGG_BASE + 0xf4)    /* 0x212f4 */
#define MAX_BFF_TIME_EN (1 << 30)
#define AIR_TIME_RESP_MODE  (1 << 31)
#define MAX_BFF_TIME 10000
#endif /* __WF_AGG_H__ */



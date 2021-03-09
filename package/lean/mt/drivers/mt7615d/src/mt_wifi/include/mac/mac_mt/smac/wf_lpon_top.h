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
	wf_lpon_top.h

	Abstract:
	Ralink Wireless Chip MAC related definition & structures

	Revision History:
	Who			When		  What
	--------	----------	  ----------------------------------------------
*/


#ifndef __WF_LPON_TOP_H__
#define __WF_LPON_TOP_H__

/* In range of 0x24000 ~ 0x24400 */
#define WF_LPON_TOP_BASE		0x24000
#define LPON_TT0SBOR1		(WF_LPON_TOP_BASE + 0x00)
#define LPON_TT0SBOR2		(WF_LPON_TOP_BASE + 0x04)
#define LPON_TT0SBOR3		(WF_LPON_TOP_BASE + 0x08)
#define LPON_TT0SBOR4		(WF_LPON_TOP_BASE + 0x0c)
#define LPON_TT0SBOR5		(WF_LPON_TOP_BASE + 0x54)
#define LPON_TT0SBOR6		(WF_LPON_TOP_BASE + 0x58)
#define LPON_TT0SBOR7		(WF_LPON_TOP_BASE + 0x64)
#define LPON_TT0SBOR8		(WF_LPON_TOP_BASE + 0x68)
#define LPON_TT0SBOR9		(WF_LPON_TOP_BASE + 0x6c)
#define LPON_TT0SBOR10		(WF_LPON_TOP_BASE + 0x70)
#define LPON_TT0SBOR11		(WF_LPON_TOP_BASE + 0x74)
#define LPON_TT0SBOR12		(WF_LPON_TOP_BASE + 0x78)
#define LPON_TT0SBOR13		(WF_LPON_TOP_BASE + 0x80)
#define LPON_TT0SBOR14		(WF_LPON_TOP_BASE + 0x84)
#define LPON_TT0SBOR15		(WF_LPON_TOP_BASE + 0x8c)
#define SUB_BSSID0_TTTT_OFFSET_n_MASK (0x3ffff)
#define SBSS_TTTT0_TSF0_EN  BIT29
#define TTTT0_n_INT_EN      BIT30
#define PRE_TTTT0_n_INT_EN  BIT31

#define TSF_TIMER_HW_MODE_FULL_ADHOC   (3 << 2)
#define TSF_TIMER_HW_MODE_RX_ONLY      (2 << 2)
#define TSF_TIMER_HW_MODE_TICK_ONLY    (1 << 2)
#define TSF_TIMER_HW_MODE_FULL         (0 << 2)

#define TSF_TIMER_SW_MODE_TSFTIMERVALUEREAD        3
#define TSF_TIMER_SW_MODE_TSFTIMERVALUEADJUST      2
#define TSF_TIMER_SW_MODE_TSFTIMERVALUECHANGE      1

#define LPON_T0CR        (WF_LPON_TOP_BASE + 0x10)	/* 0x24010 */
#define LPON_T1CR        (WF_LPON_TOP_BASE + 0x14)	/* 0x24014 */
#define LPON_UTTR0          (WF_LPON_TOP_BASE + 0x18)	/* 0x24018 */
#define LPON_UTTR1          (WF_LPON_TOP_BASE + 0x1c)	/* 0x2401c */
#define LPON_BTEIR          (WF_LPON_TOP_BASE + 0x20)	/* 0x24020 */
#define LPON_TIMTR          (WF_LPON_TOP_BASE + 0x24)	/* 0x24024 */
#define LPON_T0STR          (WF_LPON_TOP_BASE + 0x28)	/* 0x24028 */
#define LPON_T1STR          (WF_LPON_TOP_BASE + 0x2c)	/* 0x2402c */
#define LPON_PISR          (WF_LPON_TOP_BASE + 0x30)	/* 0x24030 */
#define DEFAULT_PRETBTT_INTERVAL_IN_MS      0x50    /* 5ms */

#define LPON_T0TPCR          (WF_LPON_TOP_BASE + 0x34)	/* 0x24034 */
#define LPON_T1TPCR          (WF_LPON_TOP_BASE + 0x38)	/* 0x24038 */
#define LPON_T2TPCR          (WF_LPON_TOP_BASE + 0x108)	/* 0x24108 */
#define LPON_T3TPCR          (WF_LPON_TOP_BASE + 0x10c)	/* 0x2410c */

#define BEACONPERIODn_MASK (0xffff)
#define BEACONPERIODn(p) (((p) & 0xffff))
#define DTIMPERIODn_MASK (0xff << 16)
#define DTIMPERIODn(p) (((p) & 0xff) << 16)
#define TBTTWAKEPERIODn_MASK (0xf << 24)
#define TBTTWAKEPERIODn(p) (((p) & 0xf) << 24)
#define DTIMWAKEPERIODn_MASK (0x7 << 28)
#define DTIMWAKEPERIODn(p) (((p) & 0x7) << 28)
#define TBTTn_CAL_EN (1 << 31)

#define LPON_TTSR          (WF_LPON_TOP_BASE + 0x3c)	/* 0x2403c */
#define LPON_TFRSR          (WF_LPON_TOP_BASE + 0x40)	/* 0x24040 */
#define LPON_TSELR	(WF_LPON_TOP_BASE + 0x44)	/* 0x24044 */
#define LPON_SPCR          (WF_LPON_TOP_BASE + 0x48)	/* 0x24048 */
#define LPON_BCNTR		(WF_LPON_TOP_BASE + 0x4c)	/* 0x2404c */
#define LPON_TCLCR		(WF_LPON_TOP_BASE + 0x50)	/* 0x24050 */


#define PRETBTT_INT_EN      (1 << 7)
#define BMC_TIMEOUT_EN      (1 << 6)
#define BCN_TIMEOUT_EN      (1 << 5)
#define PRETBTT_TIMEUP_EN   (1 << 4)
#define TBTT_TIMEUP_EN          (1 << 3)
#define PREDTIM_TRIG_EN         (1 << 2)
#define PRETBTT_TRIG_EN         (1 << 1)
#define TBTT_PERIOD_TIMER_EN (1 << 0)

#define LPON_MPTCR0		(WF_LPON_TOP_BASE + 0x5c)	/* 0x2405c */
#define LPON_MPTCR1          (WF_LPON_TOP_BASE + 0x60)	/* 0x24060 */
#define LPON_MPTCR2		(WF_LPON_TOP_BASE + 0x110)	/* 0x24110 */
#define LPON_MPTCR3          (WF_LPON_TOP_BASE + 0x114)	/* 0x24114 */

#define LPON_FRCR		(WF_LPON_TOP_BASE + 0x7c)	/* 0x2407c */
#define LPON_WLANCKCR          (WF_LPON_TOP_BASE + 0x88)	/* 0x24088 */


#define LPON_SBTOR1		(WF_LPON_TOP_BASE + 0xa0)
#define LPON_SBTOR2		(WF_LPON_TOP_BASE + 0xa4)
#define LPON_SBTOR3		(WF_LPON_TOP_BASE + 0xa8)
#define LPON_SBTOR4		(WF_LPON_TOP_BASE + 0xac)
#define LPON_SBTOR5		(WF_LPON_TOP_BASE + 0xb0)
#define LPON_SBTOR6		(WF_LPON_TOP_BASE + 0xb4)
#define LPON_SBTOR7		(WF_LPON_TOP_BASE + 0xb8)
#define LPON_SBTOR8		(WF_LPON_TOP_BASE + 0xbc)
#define LPON_SBTOR9		(WF_LPON_TOP_BASE + 0xc0)
#define LPON_SBTOR10		(WF_LPON_TOP_BASE + 0xc4)
#define LPON_SBTOR11		(WF_LPON_TOP_BASE + 0xc8)
#define LPON_SBTOR12		(WF_LPON_TOP_BASE + 0xcc)
#define LPON_SBTOR13		(WF_LPON_TOP_BASE + 0xd0)
#define LPON_SBTOR14		(WF_LPON_TOP_BASE + 0xd4)
#define LPON_SBTOR15		(WF_LPON_TOP_BASE + 0xd8)
#define SUB_BSSID0_TIME_OFFSET_n_MASK (0xfffff)
#define SUB_BSSID0_TIME_OFFSET_n(p) (((p) & 0xfffff))
#define SBSS_TBTT0_TSF0_EN  BIT29
#define TBTT0_n_INT_EN      BIT30
#define PRE_TBTT0_n_INT_EN  BIT31

#define LPON_T2CR        (WF_LPON_TOP_BASE + 0xF8)
#define LPON_T3CR        (WF_LPON_TOP_BASE + 0xFC)

#define LPON_MPTCR4		(WF_LPON_TOP_BASE + 0x128)	/* 0x24128 */
#define TTTT0_PERIODTIMER_EN (1 << 0)
#define PRETTTT0_TRIG_EN (1 << 1)
#define TTTT0_TIMEUP_EN (1 << 3)
#define PRETTTT0TIMEUP_EN (1 << 4)
#define TIM_TIMEOUT0_EN (1 << 5)
#define TIM_BMC_TIMEOUT0_EN (1 << 6)
#define PRETTTT0INT_EN (1 << 7)
#define TTTT1PERIODTIMER_EN (1 << 8)
#define PRETTTT1_TRIG_EN (1 << 9)
#define TTTT1_TIMEUP_EN (1 << 11)
#define PRETTTT1TIMEUP_EN (1 << 12)
#define TIM_TIMEOUT1_EN (1 << 13)
#define TIM_BMC_TIMEOUT1_EN (1 << 14)
#define PRETTTT1INT_EN (1 << 15)
#define TIM_PARSE_TIM0_EN (1 << 27)
#define TIM_BMC0_EN (1 << 28)
#define TIM_PARSE_TIM1_EN (1 << 29)
#define TIM_BMC1_EN (1 << 30)

#define LPON_MPTCR5		(WF_LPON_TOP_BASE + 0x12c)	/* 0x2412c */
#define LPON_PTTISR		(WF_LPON_TOP_BASE + 0xdc)	/* 0x240dc */
#define DEFAULT_PRETTTT_INTERVAL_IN_MS      0x50    /* 5ms, unit is 64usec */
#define IDX_DEFAULT_PRETTTT_INTERVAL(_p)    (DEFAULT_PRETTTT_INTERVAL_IN_MS << (8 * _p))

#define LPON_TT0TPCR        (WF_LPON_TOP_BASE + 0xe8)   /* 0x240e8 */
#define DEFAULT_TTTT_OFFSET_IN_MS      0x2800    /* make default +10ms, unit is usec*/
#define LPON_TT1TPCR		(WF_LPON_TOP_BASE + 0xec)	/* 0x240ec */
#define LPON_TT2TPCR		(WF_LPON_TOP_BASE + 0xf0)	/* 0x240f0 */
#define LPON_TT3TPCR		(WF_LPON_TOP_BASE + 0xf4)	/* 0x240f4 */
#define TTTTn_OFFSET_OF_TBTTn_MASK (0x3ffff)
#define TTTTn_OFFSET_OF_TBTTn(p) (((p) & 0x3ffff))
#define TTTTn_CAL_EN        BIT31

#define LPON_TT4TPCR		(WF_LPON_TOP_BASE + 0x1b8)	/* 0x241b8 */
#define LPON_TT5TPCR		(WF_LPON_TOP_BASE + 0x1bc)	/* 0x241bc */
#define LPON_TT6TPCR		(WF_LPON_TOP_BASE + 0x1c0)	/* 0x241c0 */
#define LPON_TT7TPCR		(WF_LPON_TOP_BASE + 0x1c4)	/* 0x241c4 */
#define TIM_PERIODn_MASK (0xffffff)
#define TIM_PERIODn(p) (((p) & 0xffffff))

#define SET_TTTT_PERIOD(_pAd, _interval)    (_interval * _pAd->CommonCfg.BeaconPeriod)

/* TSF_TIMER_SW_MODE */
#define TSF_TIMER_VALUE_CHANGE  0x1
#define TSF_TIMER_VALUE_ADJUST  0x2
#define TSF_TIMER_VALUE_READ  0x3

#define TSF_TIMER_HW_MODE_MASK  (0x3 << 2)
#define TSF_TIMER_SW_MODE_MASK  (0x3 << 0)

#endif /* __WF_LPON_TOP_H__ */


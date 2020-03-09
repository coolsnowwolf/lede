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
	wf_phy.h

	Abstract:
	Ralink Wireless Chip MAC related definition & structures

	Revision History:
	Who			When		  What
	--------	----------	  ----------------------------------------------
*/


#ifndef __WF_PHY_H__
#define __WF_PHY_H__

#define WF_PHY_BASE			0x10000

#define PHY_PHYSYS_CTRL		(WF_PHY_BASE + 0x0000) /* 0x82070000 */
#define PHY_PHYCK_CTRL			(WF_PHY_BASE + 0x0004) /* 0x82070004 */

#define PHY_BAND0_PHY_CTRL_0	(WF_PHY_BASE + 0x0200) /* 0x82070200 */
#define RO_BAND0_PHYCTRL_STS0	(WF_PHY_BASE + 0x020C) /* 0x8207020C */
#define RO_BAND0_PHYCTRL_STS1	(WF_PHY_BASE + 0x0210) /* 0x82070210 */
#define RO_BAND0_PHYCTRL_STS2	(WF_PHY_BASE + 0x0214) /* 0x82070214 */
#define RO_BAND0_PHYCTRL_STS4	(WF_PHY_BASE + 0x021C) /* 0x8207021C */
#define RO_BAND0_PHYCTRL_STS5	(WF_PHY_BASE + 0x0220) /* 0x82070220 */
#define RO_BAND0_PHYCTRL_STS	(WF_PHY_BASE + 0x0230) /* 0x82070230 */



#define PHY_BAND1_PHY_CTRL_0	(WF_PHY_BASE + 0x0400) /* 0x82070400 */
#define RO_BAND1_PHYCTRL_STS0	(WF_PHY_BASE + 0x040C) /* 0x8207040C */
#define RO_BAND1_PHYCTRL_STS1	(WF_PHY_BASE + 0x0410) /* 0x82070410 */
#define RO_BAND1_PHYCTRL_STS2	(WF_PHY_BASE + 0x0414) /* 0x82070414 */
#define RO_BAND1_PHYCTRL_STS4	(WF_PHY_BASE + 0x041C) /* 0x8207041C */
#define RO_BAND1_PHYCTRL_STS5	(WF_PHY_BASE + 0x0420) /* 0x82070420 */
#define RO_BAND1_PHYCTRL_STS	(WF_PHY_BASE + 0x0430) /* 0x82070430 */


#define PHY_BAND0_PHYMUX_5	(WF_PHY_BASE + 0x0614) /* 0x82070614 */
#define PHY_BAND0_PHYMUX_6	(WF_PHY_BASE + 0x0618) /* 0x82070618 */
#define PHY_BAND0_PHYMUX_23	(WF_PHY_BASE + 0x065c) /* 0x8207065c */
#define PHY_BAND1_PHYMUX_5	(WF_PHY_BASE + 0x0814) /* 0x82070814 */
#define PHY_BAND1_PHYMUX_6	(WF_PHY_BASE + 0x0818) /* 0x82070818 */
#define PHY_BAND1_PHYMUX_23	(WF_PHY_BASE + 0x085c) /* 0x8207085c */
#define PHY_BAND0_PHY_CCA       (WF_PHY_BASE + 0x0618) /*  0x82070618 */
#define PHY_BAND1_PHY_CCA       (WF_PHY_BASE + 0x0818) /*  0x82070818 */

#define CR_DBGSGD_MODE (WF_PHY_BASE + 0x0C04) /*  0x82070C04 */

#define PHY_TXFD_1				(WF_PHY_BASE + 0x8004) /* 0x82078004 */

#define PHY_RXTD_RXFE_01_B0	(WF_PHY_BASE + 0x2004) /* 0x82072004 */
#define PHY_RXTD_RXFE_01_B1	(WF_PHY_BASE + 0x2804) /* 0x82072804 */

#define PHY_DCRF_TRACK		(WF_PHY_BASE + 0x200C) /* 0x8207200C */

#define PHY_RXTD_0				(WF_PHY_BASE + 0x2200) /* 0x82072200 */
#define PHY_RXTD_12                        (WF_PHY_BASE + 0x2230) /* 0x82072230 */
#define PHY_MIN_PRI_PWR              (WF_PHY_BASE + 0x229C) /* 0x8207229C */
#define BAND1_PHY_MIN_PRI_PWR       (WF_PHY_BASE + 0x0084) /* 0x82070084 */
#define PHY_RXTD2_10                      (WF_PHY_BASE + 0x2a28) /* 0x82072a28 */
#define PHY_RXTD2_0				(WF_PHY_BASE + 0x2a00) /* 0x82072a00 */

#define PHY_LTFSYNC_6			(WF_PHY_BASE + 0x22f4) /* 0x820722f4 */

#define PHY_RXTD_CCKPD_3		(WF_PHY_BASE + 0x2300) /* 0x82072300 */
#define PHY_RXTD_CCKPD_4		(WF_PHY_BASE + 0x2304) /* 0x82072304 */
#define PHY_RXTD_CCKPD_6		(WF_PHY_BASE + 0x230c) /* 0x8207230c */
#define PHY_RXTD_CCKPD_7		(WF_PHY_BASE + 0x2310) /* 0x82072310 */
#define PHY_RXTD_CCKPD_8		(WF_PHY_BASE + 0x2314) /* 0x82072314 */

#define RO_BAND0_RXTD_DEBUG0		(WF_PHY_BASE + 0x227c) /* 0x8207227c */
#define RO_BAND0_RXTD_DEBUG4		(WF_PHY_BASE + 0x228c) /* 0x8207228c */
#define RO_BAND0_RXTD_DEBUG6		(WF_PHY_BASE + 0x2294) /* 0x82072294 */


#define PHY_RXTD1_0				(WF_PHY_BASE + 0x2600) /* 0x82072600 */
#define PHY_RXTD1_1				(WF_PHY_BASE + 0x2604) /* 0x82072604 */
#define PHY_RXTD1_4				(WF_PHY_BASE + 0x2610) /* 0x82072610 */

#define PHY_RXTD_43				(WF_PHY_BASE + 0x22ac) /* 0x820722ac */
#define PHY_RXTD_44				(WF_PHY_BASE + 0x22b0) /* 0x820722b0 */
#define PHY_RXTD_56				(WF_PHY_BASE + 0x2728) /* 0x82072728 */
#define PHY_RXTD_58				(WF_PHY_BASE + 0x2730) /* 0x82072730 */

#define PHY_RXTD_BAND0_AGC_23_RX0	(WF_PHY_BASE + 0x215c) /* 0x8207215c */
#define PHY_RXTD_BAND0_AGC_23_RX1	(WF_PHY_BASE + 0x255c) /* 0x8207255c */
#define RO_BAND0_AGC_DEBUG_0		(WF_PHY_BASE + 0x21a0) /* 0x820721a0 */
#define RO_BAND0_AGC_DEBUG_2		(WF_PHY_BASE + 0x21A8) /* 0x820721A8 */
#define RO_BAND0_AGC_DEBUG_4		(WF_PHY_BASE + 0x21B0) /* 0x820721B0 */
#define RO_BAND0_AGC_DEBUG_6		(WF_PHY_BASE + 0x21b8) /* 0x820721b8 */
#define RO_BAND1_AGC_DEBUG_2		(WF_PHY_BASE + 0x29A8) /* 0x820729A8 */
#define RO_BAND1_AGC_DEBUG_4		(WF_PHY_BASE + 0x29B0) /* 0x820729B0 */

#define DCRF_TRACK		(WF_PHY_BASE + 0x200C) /* 0x8207200C */

#define PHY_FSD_CTRL_1				(WF_PHY_BASE + 0x50d8) /* 0x820750d8 */

#define PHY_TX_BAND0_WF0_CR_TXFE_3	(WF_PHY_BASE + 0x8408) /* 0x82078408 */
#define PHY_TX_BAND0_WF1_CR_TXFE_3	(WF_PHY_BASE + 0x8420) /* 0x82078420 */
#define PHY_TX_BAND1_WF0_CR_TXFE_3	(WF_PHY_BASE + 0x8438) /* 0x82078438 */
#define PHY_TX_BAND1_WF1_CR_TXFE_3	(WF_PHY_BASE + 0x8450) /* 0x82078450 */

#define PHY_CTRL_TSSI_9					(WF_PHY_BASE + 0x9c24) /* 0x82079c24 */
#define PHY_CTRL_WF1_TSSI_9			(WF_PHY_BASE + 0x9d24) /* 0x82079d24 */
#define PHY_CTRL_WF2_TSSI_9			(WF_PHY_BASE + 0x9e24) /* 0x82079e24 */
#define PHY_CTRL_WF3_TSSI_9			(WF_PHY_BASE + 0x9f24) /* 0x82079f24 */



#endif /* __WF_PHY_H__ */

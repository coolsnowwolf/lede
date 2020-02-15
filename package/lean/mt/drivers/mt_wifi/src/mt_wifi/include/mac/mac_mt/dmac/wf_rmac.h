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
	wf_rmac.h

	Abstract:
	Ralink Wireless Chip MAC related definition & structures

	Revision History:
	Who			When		  What
	--------	----------	  ----------------------------------------------
*/



#ifndef __WF_RMAC_H__
#define __WF_RMAC_H__

#define WF_RMAC_BASE		0x21200
/* TODO: shiang-7603, un-finished yet! */


/*
	Current BSSID 0 Register 0/1
*/
#define RMAC_RFCR_BAND_0			(WF_RMAC_BASE + 0x000)
#define RMAC_RFCR_BAND_1			(WF_RMAC_BASE + 0x100)
#define DROP_STBC_BCN_BC_MC		(1 << 0)
#define DROP_FCS_ERROR_FRAME (1 << 1)
#define RX_PROMISCUOUS_MODE (1 << 2)
#define DROP_VERSION_NO_0 (1 << 3)
#define DROP_PROBE_REQ (1 << 4)
#define DROP_MC_FRAME (1 << 5)
#define DROP_BC_FRAME (1 << 6)
#define DROP_NOT_IN_MC_TABLE (1 << 7)
#define DROP_ADDR3_OWN_MAC (1 << 8)
#define DROP_DIFF_BSSID_A3 (1 << 9)
#define DROP_DIFF_BSSID_A2 (1 << 10)
#define DROP_DIFF_BSSID_BCN (1 << 11)
#define RM_FRAME_REPORT_EN (1 << 12)
#define DROP_CTRL_RSV (1 << 13)
#define DROP_CTS (1 << 14)
#define DROP_RTS (1 << 15)
#define DROP_DUPLICATE (1 << 16)
#define DROP_NOT_MY_BSSID (1 << 17)
#define DROP_NOT_UC2ME (1 << 18)
#define DROP_DIFF_BSSID_BTIM (1 << 19)
#define DROP_NDPA (1 << 20)
#define RX_UNWANTED_CTL_FRM (1 << 21)


#define RMAC_RFCR1				(WF_RMAC_BASE + 0x004)
#define RMAC_RFCR1_BAND_1		(WF_RMAC_BASE + 0x104)

#define RMAC_BCNF0		(WF_RMAC_BASE + 0x008)
#define RMAC_BCNF0_BAND_1		(WF_RMAC_BASE + 0x108)

#define RMAC_NAVOPT		(WF_RMAC_BASE + 0x40)

#define RMAC_BCNF1		(WF_RMAC_BASE + 0x00c)
#define RMAC_BCNF1_BAND_1		(WF_RMAC_BASE + 0x10c)


#define BSCR0_BAND_0	(WF_RMAC_BASE + 0x010)
#define BSCR1_BAND_0	(WF_RMAC_BASE + 0x014)
#define BSCR0_BAND_1	(WF_RMAC_BASE + 0x110)
#define BSCR1_BAND_1	(WF_RMAC_BASE + 0x114)
#define BA_MAC_ADDR_47_32_MASK (0xffff)
#define BA_MAC_ADDR_47_32(p) (((p) & 0xffff))
#define RST_BA_TID_MASK (0xf << 16)
#define RST_BA_TID(p) (((p) & 0xf) << 16)
#define RST_BA_SEL_MASK (0x3 << 20)
#define RST_BA_MAC_TID_MATCH 0
#define RST_BA_MAC_MATCH 1
#define RST_BA_WITHOUT_MATCH 2
#define RST_BA_SEL(p) (((p) & 0x3) << 20)
#define START_RST_BA_SB (1 << 22)

#define RMAC_RMACDR		(WF_RMAC_BASE + 0x018)	/* 0x21218 */
#define RMAC_RMACDR_BAND_1		(WF_RMAC_BASE + 0x118)	/* 0x21318 */
#define DISABLE_TSF_PROBE_RSP (1 << 0)
#define ENABLE_TSF_TIM (1 << 4)
#define RMACDR_MBSSID_MASK (0x3 << 24)
#define RMACDR_MBSSID(p) (((p) & 0x3) << 24)
#define CHK_HTC_BY_RATE (1 << 26)
#define SELECT_RXMAXLEN_20BIT (1 << 30)

/* TODO:shiang-MT7615, confirm this! */
#define RMAC_NSUR		(WF_RMAC_BASE + 0x01c)
#define RMAC_NSUR_BAND_1		(WF_RMAC_BASE + 0x11c)


#define RMAC_RMCR			(WF_RMAC_BASE + 0x020)
#define RMAC_RMCR_BAND_1	(WF_RMAC_BASE + 0x120)
#define RMAC_RMCR_SMPS_MODE_MASK (0x3 << 20)
#define RMAC_RMCR_SMPS_MODE(p) (((p) & 0x3) << 20)
#define DISABLE_SMPS_RX_BYPASS_SETTING 0
#define STATIC_SMPS 1
#define DYNAMIC_SMPS 2
#define DISABLE_SMPS_RX_BYSETTING 3
#define RMAC_RMCR_RX_STREAM_0		(1 << 22)
#define RMAC_RMCR_RX_STREAM_1		(1 << 23)
#define RMAC_RMCR_RX_STREAM_2		(1 << 24)
#define RMAC_RMCR_RX_STREAM_3		(1 << 25)
#define RMAC_RMCR_SMPS_BY_RTS		(1 << 26)

#define RMAC_MORE			(WF_RMAC_BASE + 0x024)
#define RMAC_MORE_BAND_1	(WF_RMAC_BASE + 0x124)
typedef union _RMAC_MORE_STRUC {
	struct {
		UINT32 mpdu_len_compare:14;
		UINT32 rsv_14_15:2;
		UINT32 rmac_cr_smps_slot_idle:2;
		UINT32 rsv_18_25:8;
		UINT32 drop_ndp:1;
		UINT32 drop_uc2me_bssid_unmatch:1;
		UINT32 rsv_28_29:2;
		UINT32 muar_mode_sel:2;
	} field;

	UINT32 word;
} RMAC_MORE_STRUC;

#define RMAC_CHFREQ0				(WF_RMAC_BASE + 0x030)
#define RMAC_CHFREQ1				(WF_RMAC_BASE + 0x130)

#define RMAC_MISC					(WF_RMAC_BASE + 0x034)
#define RMAC_MISC_BAND_1			(WF_RMAC_BASE + 0x134)

#define RMAC_MAXMINLEN				(WF_RMAC_BASE + 0x038)
#define RMAC_MAXMINLEN_BAND_1	(WF_RMAC_BASE + 0x138)
#define RMAC_DROP_MIN_LEN	0xe
#define RMAC_DROP_MIN_LEN_MASK	(0xff << 24)
#define RMAC_DROP_MAX_LEN	0x40000
#define RMAC_DROP_MAX_LEN_MASK	(0xfffff)

/*information from 2015-July-9 task group mm.*/
enum ENUM_HW_TMR_VER {
	TMR_VER_1_0 = 0, /*6630E3/7628/Gen3/7637/7622/7615E1*/
	TMR_VER_1_5 = 1, /* MT7615 e3 only.*/
	TMR_VER_2_0 = 2  /* for 6632 */
};

#define RMAC_TMR_PA				(WF_RMAC_BASE + 0x03c)
#define RMAC_TMR_PA_BAND_1			(WF_RMAC_BASE + 0x013c)

#define RMAC_TMR_ENABLE			BIT31
#define RMAC_TMR_ROLE			BIT30

/*below CRs are available in MT7615E3 and MT6632 E1 */
#define	RMAC_TMR_LOCK_EN		BIT29
#define	RMAC_TMR_UNLOCK_CTRL		BIT28
#define	RMAC_TMR_CATEGORY3_EN		BIT27
#define	RMAC_TMR_CATEGORY2_EN		BIT26
#define	RMAC_TMR_CATEGORY1_EN		BIT25
#define	RMAC_TMR_CATEGORY0_EN		BIT24
/*above CRs are available in MT7615E3 and MT6632 E1 */

#define TMR_PA_TYPE_SUBTYPE3_MASK	BITS(18, 23)
#define TMR_PA_TYPE_SUBTYPE3_OFST	18
#define TMR_PA_TYPE_SUBTYPE2_MASK	BITS(12, 17)
#define TMR_PA_TYPE_SUBTYPE2_OFST	12
#define TMR_PA_TYPE_SUBTYPE1_MASK	BITS(6, 11)
#define TMR_PA_TYPE_SUBTYPE1_OFST	6
#define TMR_PA_TYPE_SUBTYPE0_MASK	BITS(0, 5)
#define TMR_PA_TYPE_SUBTYPE0_OFST	0

#define TMR_PA_SUBTYPE_OFST		2

#define RMAC_PHY_DELAY				(WF_RMAC_BASE + 0x06c)
#define RMAC_PHY_DELAY_BAND_1	(WF_RMAC_BASE + 0x016c)

#ifdef AIR_MONITOR
#define RMAC_SMESH		(WF_RMAC_BASE + 0x048)
#define RMAC_SMESH_B1	(WF_RMAC_BASE + 0x148)
#endif


/*
	Current BSSID 0 Register 0/1
*/
#define RMAC_CB0R0		(WF_RMAC_BASE + 0x200)	/* 0x21400 */
#define RMAC_CB0R1		(WF_RMAC_BASE + 0x204)
#define RMAC_CB1R0		(WF_RMAC_BASE + 0x208)
#define RMAC_CB1R1		(WF_RMAC_BASE + 0x20c)
#define RMAC_CB2R0		(WF_RMAC_BASE + 0x210)
#define RMAC_CB2R1		(WF_RMAC_BASE + 0x214)
#define RMAC_CB3R0		(WF_RMAC_BASE + 0x218)
#define RMAC_CB3R1		(WF_RMAC_BASE + 0x21c)

typedef struct _RMAC_CBxR0_STRUC {
	UINT32 addr_31_0;
} RMAC_CBxR0_STRUC;

typedef union _RMAC_CBxR1_STRUC {
	struct {
		UINT32 addr_47_32:16;
		UINT32 bssid_en:1;
		UINT32 rsv_17:15;
	} field;
	UINT32 word;
} RMAC_CBxR1_STRUC;


/*
	OWN MAC Address 0 Register 0/1
*/
#define RMAC_OMA0R0	(WF_RMAC_BASE + 0x220)	/* 0x21420 */
#define RMAC_OMA0R1	(WF_RMAC_BASE + 0x224)	/* 0x21424 */
#define RMAC_OMA1R0	(WF_RMAC_BASE + 0x228)	/* 0x21428 */
#define RMAC_OMA1R1	(WF_RMAC_BASE + 0x22c)	/* 0x2142c */
#define RMAC_OMA2R0	(WF_RMAC_BASE + 0x230)	/* 0x21430 */
#define RMAC_OMA2R1	(WF_RMAC_BASE + 0x234)	/* 0x21434 */
#define RMAC_OMA3R0	(WF_RMAC_BASE + 0x238)	/* 0x21438 */
#define RMAC_OMA3R1	(WF_RMAC_BASE + 0x23c)	/* 0x2143c */
#define RMAC_OMA4R0	(WF_RMAC_BASE + 0x240)	/* 0x21440 */
#define RMAC_OMA4R1	(WF_RMAC_BASE + 0x244)	/* 0x21444 */
#define RMAC_B01AR_B1AR	(WF_RMAC_BASE + 0x250)	/* 0x21450 */
#define RMAC_B23AR_B1AR	(WF_RMAC_BASE + 0x24c)	/* 0x2144c */
#define RMAC_ACBEN		(WF_RMAC_BASE + 0x258)	/* 0x21458 */

#define RMAC_MAR0               (WF_RMAC_BASE + 0x25c)	/* 0x2145c */
#define RMAC_MAR1               (WF_RMAC_BASE + 0x260)	/* 0x21460 */
#define RMAC_RSVD0              (WF_RMAC_BASE + 0x2e0)  /* 0x214e0 */
#define RMAC_DEBUG_CR       (WF_RMAC_BASE + 0x2e4)
typedef struct _RMAC_MAR0_STRUC {
	UINT32 addr_31_0;
} RMAC_MAR0_STRUC;

typedef union _RMAC_MAR1_STRUC {
	struct {
		UINT32 addr_39_32:8;
		UINT32 addr_47_40:8;
		UINT32 access_start:1;
		UINT32 readwrite:1;
		UINT32 rsv18_23:6;
		UINT32 multicast_addr_index:6;
		UINT32 multicast_addr_group:2;
	} field;
	UINT32 word;
} RMAC_MAR1_STRUC;


#define RX_EIFS_TIME_ZERO   (1 << 21)

#define RMAC_MBSSIDEXT1_0               (WF_RMAC_BASE + 0x300)
#define RMAC_MBSSIDEXT1_1               (WF_RMAC_BASE + 0x304)
#define RMAC_MBSSIDEXT2_0               (WF_RMAC_BASE + 0x308)
#define RMAC_MBSSIDEXT2_1               (WF_RMAC_BASE + 0x30c)
#define RMAC_MBSSIDEXT3_0               (WF_RMAC_BASE + 0x310)
#define RMAC_MBSSIDEXT3_1               (WF_RMAC_BASE + 0x314)
#define RMAC_MBSSIDEXT4_0               (WF_RMAC_BASE + 0x318)
#define RMAC_MBSSIDEXT4_1               (WF_RMAC_BASE + 0x31c)
#define RMAC_MBSSIDEXT5_0               (WF_RMAC_BASE + 0x320)
#define RMAC_MBSSIDEXT5_1               (WF_RMAC_BASE + 0x324)
#define RMAC_MBSSIDEXT6_0               (WF_RMAC_BASE + 0x328)
#define RMAC_MBSSIDEXT6_1               (WF_RMAC_BASE + 0x32c)
#define RMAC_MBSSIDEXT7_0               (WF_RMAC_BASE + 0x330)
#define RMAC_MBSSIDEXT7_1               (WF_RMAC_BASE + 0x334)
#define RMAC_MBSSIDEXT8_0               (WF_RMAC_BASE + 0x338)
#define RMAC_MBSSIDEXT8_1               (WF_RMAC_BASE + 0x33c)
#define RMAC_MBSSIDEXT9_0               (WF_RMAC_BASE + 0x340)
#define RMAC_MBSSIDEXT9_1               (WF_RMAC_BASE + 0x344)
#define RMAC_MBSSIDEXT10_0               (WF_RMAC_BASE + 0x348)
#define RMAC_MBSSIDEXT10_1               (WF_RMAC_BASE + 0x34c)
#define RMAC_MBSSIDEXT11_0               (WF_RMAC_BASE + 0x350)
#define RMAC_MBSSIDEXT11_1               (WF_RMAC_BASE + 0x354)
#define RMAC_MBSSIDEXT12_0               (WF_RMAC_BASE + 0x358)
#define RMAC_MBSSIDEXT12_1               (WF_RMAC_BASE + 0x35c)
#define RMAC_MBSSIDEXT13_0               (WF_RMAC_BASE + 0x360)
#define RMAC_MBSSIDEXT13_1               (WF_RMAC_BASE + 0x364)
#define RMAC_MBSSIDEXT14_0               (WF_RMAC_BASE + 0x368)
#define RMAC_MBSSIDEXT14_1               (WF_RMAC_BASE + 0x36c)
#define RMAC_MBSSIDEXT15_0               (WF_RMAC_BASE + 0x370)
#define RMAC_MBSSIDEXT15_1               (WF_RMAC_BASE + 0x374)

#define RMAC_MIBTIME0                         (WF_RMAC_BASE + 0x3C4)
#define RMAC_MIBTIME5                         (WF_RMAC_BASE + 0x3D8)
#define RMAC_MIBTIME6                         (WF_RMAC_BASE + 0x3DC)
#define RMAC_AIRTIME0					(WF_RMAC_BASE + 0x380)
#define RX_AIRTIME_EN					BIT(30)


#endif /* __WF_RMAC_H__*/



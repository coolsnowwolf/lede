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

#define WF_RMAC_BASE		0x21800

// TODO: shiang-7603, un-finished yet!


/*
	Current BSSID 0 Register 0/1
*/
#define RMAC_RFCR		(WF_RMAC_BASE + 0x00)
#define DROP_STBC_BCN_BC_MC (1 << 0)
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

/*
	Current BSSID 0 Register 0/1
*/
#define RMAC_CB0R0		(WF_RMAC_BASE + 0x04)
#define RMAC_CB0R1		(WF_RMAC_BASE + 0x08)
#define RMAC_CB1R0		(WF_RMAC_BASE + 0x0c)
#define RMAC_CB1R1		(WF_RMAC_BASE + 0x10)
#define RMAC_CB2R0		(WF_RMAC_BASE + 0x14)
#define RMAC_CB2R1		(WF_RMAC_BASE + 0x18)
#define RMAC_CB3R0		(WF_RMAC_BASE + 0x1c)
#define RMAC_CB3R1		(WF_RMAC_BASE + 0x20)
#define RMAC_RFCR1		(WF_RMAC_BASE + 0xa4)

typedef struct _RMAC_CBxR0_STRUC {
	UINT32 addr_31_0;
}RMAC_CBxR0_STRUC;

typedef union _RMAC_CBxR1_STRUC {
	struct {
		UINT32 addr_47_32: 16;
		UINT32 bssid_en:1;
		UINT32 rsv_17: 15;
	} field;
	UINT32 word;
}RMAC_CBxR1_STRUC;


/*
	OWN MAC Address 0 Register 0/1
*/
#define RMAC_OMA0R0	(WF_RMAC_BASE + 0x024)
#define RMAC_OMA0R1	(WF_RMAC_BASE + 0x028)
#define RMAC_OMA1R0	(WF_RMAC_BASE + 0x02c)
#define RMAC_OMA1R1	(WF_RMAC_BASE + 0x030)
#define RMAC_OMA2R0	(WF_RMAC_BASE + 0x034)
#define RMAC_OMA2R1	(WF_RMAC_BASE + 0x038)
#define RMAC_OMA3R0	(WF_RMAC_BASE + 0x03c)
#define RMAC_OMA3R1	(WF_RMAC_BASE + 0x040)
#define RMAC_OMA4R0	(WF_RMAC_BASE + 0x044)
#define RMAC_OMA4R1	(WF_RMAC_BASE + 0x048)
#define RMAC_B01AR_B1AR	(WF_RMAC_BASE + 0x04c)
#define RMAC_B23AR_B1AR	(WF_RMAC_BASE + 0x050)
#define RMAC_ACBEN		(WF_RMAC_BASE + 0x054)

#define RMAC_MAR0               (WF_RMAC_BASE + 0x060)
#define RMAC_MAR1               (WF_RMAC_BASE + 0x064)
typedef struct _RMAC_MAR0_STRUC{
        UINT32 addr_31_0;
}RMAC_MAR0_STRUC;

typedef union _RMAC_MAR1_STRUC{
        struct {
                UINT32 addr_39_32:8;
                UINT32 addr_47_40:8;
                UINT32 access_start:1;
                UINT32 readwrite:1;
                UINT32 rsv18_23:6;
                UINT32 multicast_addr_index:5;
                UINT32 rsv29:1;
                UINT32 multicast_addr_group:2;
        } field;
        UINT32 word;
}RMAC_MAR1_STRUC;


#define BSCR0	(WF_RMAC_BASE + 0x068)

#define BSCR1	(WF_RMAC_BASE + 0x06c)
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

#define RMAC_RMACDR		(WF_RMAC_BASE + 0x078)
#define DISABLE_TSF_PROBE_RSP (1 << 0)
#define ENABLE_TSF_TIM (1 << 4)
#define RMACDR_MBSSID_MASK (0x3 << 24)
#define RMACDR_MBSSID(p) (((p) & 0x3) << 24)
#define CHK_HTC_BY_RATE (1 << 26)
#define SELECT_RXMAXLEN_20BIT (1 << 30)

#define RMAC_RMCR (WF_RMAC_BASE + 0x080)
#define RMAC_RMCR_SMPS_MODE_MASK (0x3 << 20)
#define RMAC_RMCR_SMPS_MODE(p) (((p) & 0x3) << 20)
#define DISABLE_SMPS_RX_BYPASS_SETTING 0
#define STATIC_SMPS 1
#define DYNAMIC_SMPS 2
#define DISABLE_SMPS_RX_BYSETTING 3
#define RMAC_RMCR_RX_STREAM_0		(1 << 22)
#define RMAC_RMCR_RX_STREAM_1		(1 << 23)
#define RMAC_RMCR_RX_STREAM_2		(1 << 24)
#define RMAC_RMCR_SMPS_BY_RTS		(1 << 25)

#define RMAC_MORE		(WF_RMAC_BASE + 0x084)
typedef union _RMAC_MORE_STRUC {
	struct {
		UINT32 mpdu_len_compare:14;
		UINT32 rsv1415:2;
		UINT32 rmac_cr_smps_slot_idle:2;
		UINT32 rsv1823:8;
		UINT32 drop_ndp:1;
		UINT32 drop_uc2me_bssid_unmatch:1;
		UINT32 rsv2829:2;
		UINT32 muar_mode_sel:2;
	} field;

	UINT32 word;
}RMAC_MORE_STRUC;

#define RMAC_CHFREQ		(WF_RMAC_BASE + 0x090)
#define RMAC_MISC		(WF_RMAC_BASE + 0x094)
#define RMAC_MAXMINLEN	(WF_RMAC_BASE + 0x098)
#define RMAC_DROP_MIN_LEN   0xe
#define RMAC_DROP_MIN_LEN_MASK  (0xff << 24)
#define RMAC_DROP_MAX_LEN   0x19000
#define RMAC_DROP_MAX_LEN_MASK  (0xfffff)
#define RMAC_PHY_DELAY	(WF_RMAC_BASE + 0x0a8)
#define RMAC_TMR_PA     (WF_RMAC_BASE + 0x0e0)
#ifdef AIR_MONITOR
#define RMAC_SMESH		(WF_RMAC_BASE + 0x114)
#endif
#endif /* __WF_RMAC_H__*/



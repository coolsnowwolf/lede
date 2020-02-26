/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	mcu.h

	Abstract:
	MCU related information

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/

#ifndef __BTCOEX_H__
#define __BTCOEX_H__

struct _RTMP_ADAPTER;
struct _MAC_TABLE_ENTRY;

/* Woody add for BT COEX*/
#define BIT0		(1 << 0)
#define BIT1		(1 << 1)
#define BIT2		(1 << 2)
#define BIT3		(1 << 3)
#define BIT4		(1 << 4)
#define BIT5		(1 << 5)
#define BIT6		(1 << 6)
#define BIT7		(1 << 7)
#define BIT8		(1 << 8)
#define BIT9		(1 << 9)
#define BIT10	(1 << 10)
#define BIT11	(1 << 11)
#define BIT12	(1 << 12)
#define BIT13	(1 << 13)
#define BIT14	(1 << 14)
#define BIT15	(1 << 15)
#define BIT16	(1 << 16)
#define BIT17	(1 << 17)
#define BIT18	(1 << 18)
#define BIT19	(1 << 19)
#define BIT20	(1 << 20)
#define BIT21	(1 << 21)
#define BIT22	(1 << 22)
#define BIT23	(1 << 23)
#define BIT24	(1 << 24)
#define BIT25	(1 << 25)
#define BIT26	(1 << 26)
#define BIT27	(1 << 27)
#define BIT28	(1 << 28)
#define BIT29	(1 << 29)
#define BIT30	(1 << 30)
#define BIT31	(1 << 31)

#define IS_MT76XXBTCOMBO(_pAd) (IS_MT7650(_pAd)  || IS_MT7630(_pAd) || IS_MT7662(_pAd))
/* Antenna mode (ANT_MODE_XXX) */
#define ANT_MODE_DUAL (0x01) /* Chip is in dual antenna mode */
#define ANT_MODE_SINGLE (0x02) /* Chip is in single antenna mode */

#define TypeTFSwitch						0x1
#define TypeProtectionFrame					0x2
#define TypeAFH							0x3
#define TypeWiFiStatus                                  0x4
#define TypeHostLoopBackTFSwitch							0xFFF1
#define TypeCoexCCUForceMode							0xFFF2

#define COEX_MODE_RESET           0
#define COEX_MODE_TDD               1
#define COEX_MODE_HYBRID          2
#define COEX_MODE_FDD               3

#define COEX_OPMODE_STA                  1
#define COEX_OPMODE_AP                   2
#define COEX_OPMODE_GC                  3
#define COEX_OPMODE_GO               4
#define COEX_OPMODE_BT                   5
#define COEX_OPMODE_AD_HOC            6


#define COEX_BARXSIZE_A2DP       8
#define COEX_BARXSIZE_SCO_ESCO   2
#define COEX_BARXSIZE_OPP        32




#ifdef COEX_SUPPORT
#define MT76xx_COEX_MODE_RESET           0
#define MT76xx_COEX_MODE_TDD               1
#define MT76xx_COEX_MODE_FDD               2


#define MT76xx_WLAN_NO_BSSID               0x0
#define MT76xx_WLAN_Device_OFF                 0x1
#define MT76xx_WLAN_Device_ON    0x2
#define MT76xx_WLAN_SCANREQEST_2G                         0x3
#define MT76xx_WLAN_SCANDONE_2G                              0x4
#define MT76xx_WLAN_SCANREQEST_5G                             0x5
#define MT76xx_WLAN_SCANDONE_5G                             0x6
#define MT76xx_WLAN_LINK_START                            0x7
#define MT76xx_WLAN_LINK_DONE                            0x8

VOID MT76xxMLMEHook(
	IN struct _RTMP_ADAPTER *pAd,
	IN UCHAR		WlanStatus,
	IN ULONG              BssHashID
);

INT Set_MT76xxCoexBeaconLimit_Proc(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_MT76xxCoexBaWinsize_Proc(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_MT76xxCoexMode_Proc(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_MT76xxCoexSupportMode_Proc(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_MT76xxCoex_Proc(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_CoexSkipFDDFix20MH_Proc(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_STRING *arg);

INT Set_MT76xxCoex_Protection_Mode_Proc(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Set_MT76xxCoex_Protection_Rate_Proc(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* COEX_SUPPORT */


#define fBTSTATUS_BT_DISABLE                  0x00000001
#define fBTSTATUS_BT_INACTIVE                0x00000002
#define fBTSTATUS_BT_SCANONLY               0x00000004
#define fBTSTATUS_BT_HOLD                       0x00000008
#define fBTSTATUS_BT_ACTIVE                    0x00000010
#define NULLFRAMESPACE                  10
#define COEXNOZEROSHIFT                1
#define CTSTOSELF                  1
#define CFEND                         2
#define POWERSAVE0               3
#define POWERSAVE1               4
#define PROTECTIONFRAMEREADY  1
#define PROTECTIONFRAMECANCEL 2

#define COEX_WIFI_LINK_UP              1
#define COEX_WIFI_LINK_DOWN         2
#define BSSID_WCID				1

typedef struct PROTECTION_FRAME_STRUCT {
	BOOLEAN     Occupied;
	ULONG         Triggernumber;
	ULONG         Valid;
	ULONG         NodeType;
	ULONG         BssHashID;
	ULONG         FrameType;
} PROTECTION_FRAME_STRUCT, *PPROTECTION_FRAME_STRUCT;

typedef struct _COEX_RX_MANUAL_AGC_GAIN {
	BOOLEAN     bEnable;
	BOOLEAN     bStopAGC;
	CHAR      LMthreshold;
	CHAR      MHthreshold;
} COEX_RX_MANUAL_AGC_GAIN, *PCOEX_RX_MANUAL_AGC_GAIN;

typedef struct _COEX_MODE_STRUCTURE {
	BOOLEAN     bForce;
	BOOLEAN     FDDRequest;
	BOOLEAN     TDDRequest;
	ULONG       TDD_Power;
	ULONG       FDD_Power;
	ULONG       DefaultMode;
	ULONG       CurrentMode;
	ULONG       UpdateMode;
	CHAR        CoexTDDRSSITreshold;
	CHAR        CoexFDDRSSITreshold;
	UCHAR       RxBAWinLimit; /* 0 = not set, use original value */
	UCHAR       TxBAWinLimit; /* 0 = not set, use origianl value */
} COEX_MODE_STRUCTURE, *PCOEX_MODE_STRUCTURE;


typedef struct _COEX_PROTECTION_FRAME_INFO {
	ULONG					CoexOperation;
	ULONG                   Triggernumber;
	ULONG                   Valid;
	ULONG                   NodeType;
	ULONG	            BssHashID;
	ULONG                   FrameType;
} COEX_PROTECTION_FRAME_INFO, *PCOEX_PROTECTION_FRAME_INFO;

typedef struct _COEX_AFH {
	ULONG					CoexOperation;
	ULONG                   LinkStatus;
	ULONG                   BW;
	ULONG                   Channel;
	ULONG	            BssHashID;
} COEX_AFH, *PCOEX_AFH;

typedef struct _COEX_TF_SWITCH {
	ULONG					CoexOperation;
	ULONG                   CoexMode;

} COEX_TF_SWITCH, *PCOEX_TF_SWITCH;


typedef struct _COEX_WLAN_STATUS {
	ULONG		     CoexOperation;
	ULONG                   WLANStatus;
	ULONG                   PrivilegeTime;
	ULONG	            BssHashID;

} COEX_WLAN_STATUS, *PCOEX_WLAN_STATUS;

#define PKT_CMD_TYPE_COEX_OP			(17)

#define BTCOEX_BB_BITWISE_WRITE(_BbReg, _BitLocation, _BitValue) \
	(_BbReg = (((_BbReg) & ~(_BitLocation)) | (_BitValue)))

#define BTCOEX_BB_BITMASK_READ(_BbReg, _BitLocation) \
	((_BbReg) & (_BitLocation))

#define MAC_BITWISE_WRITE(_MacReg, _BitLocation, _BitValue) \
	(_MacReg = (((_MacReg) & ~(_BitLocation)) | (_BitValue)))

#define MAC_BITMASK_READ(_MacReg, _BitLocation) \
	((_MacReg) & (_BitLocation))
#endif

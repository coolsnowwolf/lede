/** $Id: $
*/

/*! \file   "ra_ctrl.h"
    \brief  All Dynamic Rate Switch Related Structure & Definition
*/

/*******************************************************************************
* Copyright (c) 2014 MediaTek Inc.
*
* All rights reserved. Copying, compilation, modification, distribution
* or any other use whatsoever of this material is strictly prohibited
* except in accordance with a Software License Agreement with
* MediaTek Inc.
********************************************************************************
*/

/*******************************************************************************
* LEGAL DISCLAIMER
*
* BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND
* AGREES THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK
* SOFTWARE") RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE
* PROVIDED TO BUYER ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY
* DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT
* LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
* PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE
* ANY WARRANTY WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY
* WHICH MAY BE USED BY, INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK
* SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY
* WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE
* FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION OR TO
* CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
* BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
* LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL
* BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT
* ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY
* BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
* THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
* WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT
* OF LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING
* THEREOF AND RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN
* FRANCISCO, CA, UNDER THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE
* (ICC).
********************************************************************************
*/

/*
** $Log: ra_ctrl.h $
**
**
**
*/

#ifndef __RA_CTRL_H__
#define __RA_CTRL_H__

#ifdef WIFI_BUILD_RAM
#if (CFG_WIFI_DRIVER_OFFLOAD_RATE_CTRL == 1)
#define MT_MAC
#define DOT11_N_SUPPORT
#define MCS_LUT_SUPPORT

#if (PRODUCT_VERSION == 7636)
#define NEW_RATE_ADAPT_SUPPORT
#endif

#if (PRODUCT_VERSION == 7615 || PRODUCT_VERSION == 7637 || PRODUCT_VERSION == 7622)
#define RATE_ADAPT_AGBS_SUPPORT
#define DOT11_VHT_AC
#endif

#endif
#endif /* WIFI_BUILD_RAM */
/*******************************************************************************
*                              C O N S T A N T S
********************************************************************************
*/

/* Values of LastSecTxRateChangeAction */
#define RATE_NO_CHANGE      0               /* No change in rate */
#define RATE_UP             1               /* Trying higher rate or same rate with different BF */
#define RATE_DOWN           2               /* Trying lower rate */
#define RATE_BW_DOWN        3
#define RATE_BW_UP          4
#define RATE_BW_STATE       5

#define PER_THRD_ADJ        1

#define RA_MAX_INIT_RATE_ENTRY      10

#define RA_WAPI_PHY_RATE_UPPER_BOUND        1000

#define RA_INIT_RATE_BY_RSSI        1
#define RA_INIT_RATE_BY_LAST_RATE   2
#define RA_INIT_RATE_TRACKING       3

#define RA_DYNAMIC_BW_DISABLE       0
#define RA_DYNAMIC_BW_DOWN          1
#define RA_DYNAMIC_BW_UP            2

#define RA_LOW_TRAFFIC_TRACKING_THRD        3

#define RA_TRAIN_UP_HIGH_NSSOFFSET          5

#define AGBS_NSS_CHANGE_FLAG                0x80
#define AGBS_UPRATE_SGI_FLAG                0x40
#define AGBS_UPRATE_DOWNRATE_INVALID        0xFF

#define DRS_TX_QUALITY_WORST_BOUND  8       /* 3 */

/* Rate Adaptation timing */
#define DEF_QUICK_RA_TIME_INTERVAL  100
#define DEF_RA_TIME_INTRVAL         500
#define RA_RATE                     5       /* RA every fifth 100msec period */
#define RA_INTERVAL                 (RA_RATE*100)   /* RA Interval in msec */

/* #define RA_RMDS_THRD                12 */
#define RA_RMDS_THRD                16      /* MT7615 disable dynamic SGI by RMSD */

#define RA_MAX_SUPPORT_MCS          26

#define GET_TX_STAT_TOTAL_TX_CNT    0x00000001
#define GET_TX_STAT_LAST_TX_RATE    0x00000002
#define GET_TX_STAT_ENTRY_TX_RATE   0x00000004
#define GET_TX_STAT_ENTRY_TX_CNT    0x00000008

#define HT_LDPC                 0x01
#define VHT_LDPC                0x02

#define G_BAND_256QAM_AMPDU_FACTOR  7

#define LIMIT_MAX_PHY_RATE_THRESHOLD    50
#define MAX_PHY_RATE_3SS                1300
#define MAX_PHY_RATE_2SS                866

/* Maximum Tx Rate Table Index value */
#ifdef RATE_ADAPT_AGBS_SUPPORT
#define MAX_TX_QUALITY_INDEX    4
#else
#define MAX_TX_QUALITY_INDEX    34
#endif

#define TX_QUALITY_NSS_KEEP     0
#define TX_QUALITY_NSS_UP       1
#define TX_QUALITY_NSS_DN       2
#define TX_QUALITY_BW_UP        3

#define SGI_20                  1
#define SGI_40                  2
#define SGI_80                  4
#define SGI_160                 8

#ifdef WIFI_BUILD_RAM
#define OPMODE_STA              0
#define OPMODE_AP               1

#define RATE_1                  0
#define RATE_2                  1
#define RATE_5_5                2
#define RATE_11                 3

#define MCS_0                   0           /* 1S */
#define MCS_1                   1
#define MCS_2                   2
#define MCS_3                   3
#define MCS_4                   4
#define MCS_5                   5
#define MCS_6                   6
#define MCS_7                   7
#define MCS_8                   8           /* 2S */
#define MCS_9                   9
#define MCS_10                  10
#define MCS_11                  11
#define MCS_12                  12
#define MCS_13                  13
#define MCS_14                  14
#define MCS_15                  15
#define MCS_16                  16          /* 3S */
#define MCS_17                  17
#define MCS_18                  18
#define MCS_19                  19
#define MCS_20                  20
#define MCS_21                  21
#define MCS_22                  22
#define MCS_23                  23
#define MCS_24                  24          /* 4S */
#define MCS_25                  25
#define MCS_26                  26
#define MCS_27                  27
#define MCS_28                  28
#define MCS_29                  29
#define MCS_30                  30
#define MCS_31                  31
#define MCS_32                  32
#define MCS_AUTO                33

#define TMI_TX_RATE_BIT_STBC    11
#define TMI_TX_RATE_BIT_NSS     9
#define TMI_TX_RATE_MASK_NSS    0x3
#define TMI_TX_RATE_MASK_MCS    0x3f

#define SHORT_PREAMBLE          0
#define LONG_PREAMBLE           1

#define TMI_TX_RATE_BIT_MODE    6
#define TMI_TX_RATE_MASK_MODE   0x7
#define TMI_TX_RATE_MODE_CCK	0
#define TMI_TX_RATE_MODE_OFDM	1
#define TMI_TX_RATE_MODE_HTMIX	2
#define TMI_TX_RATE_MODE_HTGF	3
#define TMI_TX_RATE_MODE_VHT	4

#define TMI_TX_RATE_CCK_1M_LP   0
#define TMI_TX_RATE_CCK_2M_LP   1
#define TMI_TX_RATE_CCK_5M_LP   2
#define TMI_TX_RATE_CCK_11M_LP  3

#define TMI_TX_RATE_CCK_2M_SP   5
#define TMI_TX_RATE_CCK_5M_SP   6
#define TMI_TX_RATE_CCK_11M_SP  7

#define TMI_TX_RATE_OFDM_6M     11
#define TMI_TX_RATE_OFDM_9M     15
#define TMI_TX_RATE_OFDM_12M    10
#define TMI_TX_RATE_OFDM_18M    14
#define TMI_TX_RATE_OFDM_24M    9
#define TMI_TX_RATE_OFDM_36M    13
#define TMI_TX_RATE_OFDM_48M    8
#define TMI_TX_RATE_OFDM_54M    12

#define MAX_LEN_OF_CCK_RATES    4
#define MAX_LEN_OF_OFDM_RATES   8
#define MAX_LEN_OF_HT_RATES     24
#ifdef DOT11_VHT_AC
#define MAX_LEN_OF_VHT_RATES    20
#endif /* DOT11_VHT_AC */

/* BW */
#define BW_20                   0
#define BW_40                   1
#define BW_80                   2
#define BW_160                  3
#define BW_10                   4
#define BW_5                    5
#define BW_8080	                6

#define MODE_CCK                0
#define MODE_OFDM               1
#ifdef DOT11_N_SUPPORT
#define MODE_HTMIX              2
#define MODE_HTGREENFIELD       3
#endif /* DOT11_N_SUPPORT */
#define MODE_VHT                4

#define GI_400                  1           /* only support in HT/VHT mode */
#define GI_800                  0
#define GI_BOTH                 2

#define SUPPORT_CCK_MODE        1
#define SUPPORT_OFDM_MODE       2
#define SUPPORT_HT_MODE         4
#define SUPPORT_VHT_MODE        8

#ifdef DOT11_N_SUPPORT
#define HTMODE_MM               0
#define HTMODE_GF               1

#define STBC_NONE               0
#define STBC_USE                1

#define MMPS_STATIC             0
#define MMPS_DYNAMIC            1
#endif /* DOT11_N_SUPPORT */

#define fCLIENT_STATUS_SGI20_CAPABLE            0x00000010
#define fCLIENT_STATUS_SGI40_CAPABLE            0x00000020
#define fCLIENT_STATUS_VHT_RX_LDPC_CAPABLE      0x00800000
#define fCLIENT_STATUS_HT_RX_LDPC_CAPABLE       0x01000000

#ifdef DOT11_VHT_AC
#define fCLIENT_STATUS_SGI80_CAPABLE            0x00010000
#define fCLIENT_STATUS_SGI160_CAPABLE           0x00020000
#define fCLIENT_STATUS_VHT_TXSTBC_CAPABLE       0x00040000
#define fCLIENT_STATUS_VHT_RXSTBC_CAPABLE       0x00080000
#endif /* DOT11_VHT_AC */

#endif /* WIFI_BUILD_RAM */

/*******************************************************************************
*                             D A T A   T Y P E S
********************************************************************************
*/
struct _RTMP_ADAPTER;
struct _MAC_TABLE_ENTRY;

#ifndef WIFI_BUILD_RAM
typedef BOOLEAN BOOL;
typedef UINT8   UINT_8;
typedef UINT16  UINT_16;
typedef UINT32  UINT_32;
typedef CHAR    INT_8;
typedef INT16   INT_16;
typedef INT32   INT_32;
#endif /* !WIFI_BUILD_RAM */

typedef struct _RTMP_TX_RATE {
	UCHAR   mode;
	UCHAR   bw;
	UCHAR   mcs;
	UCHAR   nss;
	UCHAR   sgi;
	UCHAR   stbc;
} RTMP_TX_RATE;

typedef struct _RTMP_RA_LEGACY_TB {
	UCHAR   ItemNo;
#ifdef RT_BIG_ENDIAN
	UCHAR   Rsv2: 1;
	UCHAR   Mode: 3;
	UCHAR   BW: 2;
	UCHAR   ShortGI: 1;
	UCHAR   STBC: 1;
#else
	UCHAR   STBC: 1;
	UCHAR   ShortGI: 1;
	UCHAR   BW: 2;
	UCHAR   Mode: 3;
	UCHAR   Rsv2: 1;
#endif
	UCHAR   CurrMCS;
	UCHAR   TrainUp;
	UCHAR   TrainDown;
} RTMP_RA_LEGACY_TB;

#if defined(NEW_RATE_ADAPT_SUPPORT) || defined(RATE_ADAPT_AGBS_SUPPORT)
typedef struct _RA_PHY_CFG_T {
	UINT_8  MODE;
	UINT_8  Flags;
	UINT_8  STBC;
	UINT_8  ShortGI;
	UINT_8  BW;
	UINT_8  ldpc;
	UINT_8  MCS;
	UINT_8  VhtNss;
} RA_PHY_CFG_T;

typedef struct _RA_ENTRY_INFO_T {
	BOOL    fgRaValid;
	UINT_8  ucWcid;
	BOOL    fgAutoTxRateSwitch;

	UINT_8  ucPhyMode;                  /* wdev->PhyMode WMODE_CAP_AC */
	UINT_8  ucChannel;
	UINT_8  ucBBPCurrentBW;             /* BW_10, BW_20, BW_40, BW_80 */

	BOOL    fgDisableCCK;

	BOOL    fgHtCapMcs32;
	BOOL    fgHtCapInfoGF;
	UCHAR   aucHtCapMCSSet[4];
	UCHAR   ucMmpsMode;

	INT_8   AvgRssiSample[4];
	UINT_8  ucCERMSD;

	UINT_8  ucGband256QAMSupport;
	UINT_8  ucMaxAmpduFactor;

	UCHAR   RateLen;
	UCHAR   ucSupportRateMode;
	UINT_8  ucSupportCCKMCS;
	UINT_8  ucSupportOFDMMCS;
#ifdef DOT11_N_SUPPORT
	UINT_32 u4SupportHTMCS;
#ifdef DOT11_VHT_AC
	UINT_16 u2SupportVHTMCS1SS;
	UINT_16 u2SupportVHTMCS2SS;
	UINT_16 u2SupportVHTMCS3SS;
	UINT_16 u2SupportVHTMCS4SS;
	BOOL    force_op_mode;
	UINT_8  vhtOpModeChWidth;
	UINT_8  vhtOpModeRxNss;
	UINT_8  vhtOpModeRxNssType;
#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */

	UINT_32 ClientStatusFlags;

	RA_PHY_CFG_T MaxPhyCfg;
	RA_PHY_CFG_T TxPhyCfg;

	BOOL    fgAuthWapiMode;                 /* Security hardware limitation workaround */

#ifdef WIFI_BUILD_RAM
#if (PRODUCT_VERSION == 7615 || PRODUCT_VERSION == 7622)
	BOOL    fgRaCtrlExec;
	BOOL    fgQuickResponse;
	INT_8   ucRaTimeSlot;
	INT_8   ucRaQuickTimeSlot;
	INT_8   ucRaQuickTimeRatio;
	UINT_32 u4TxCount;
	UINT_16 u2Rate1TxCnt;
	UINT_16 u2Rate1FailCnt;
	UINT_16 u2Rate2OkCnt;
	UINT_16 u2Rate3OkCnt;
#endif
#endif /* WIFI_BUILD_RAM */
} RA_ENTRY_INFO_T, *P_RA_ENTRY_INFO_T;

typedef struct _RA_INTERNAL_INFO_T {
	BOOL    fgLastSecAccordingRSSI;
	UINT_8  ucInitialRateMode;
	UINT_8  ucLowTrafficCount;
	UINT_8  ucMcsGroup;
	UINT_8  ucGroupCnt;

	UINT_8  ucCurrTxRateIndex;
	UINT_8  ucLastRateIdx;
	UINT_8  ucLastMcsGroup;

	UINT_8  ucTxRateUpPenalty;              /* extra # of second penalty due to last unstable condition */

	/* to record the each TX rate's quality. 0 is best, the bigger the worse. */
	UINT_8  aucTxQuality[MAX_TX_QUALITY_INDEX];
	/* UINT_8  aucPER[MAX_TX_QUALITY_INDEX]; */
	UINT_8  ucDynamicSGIState;
	UINT_8  ucDynamicBWState;
	UINT_8  ucDynamicBW;

	UINT_8  ucLastSecTxRateChangeAction;    /* 0: no change, 1:rate UP, 2:rate down */
	UINT_8  ucLastTimeTxRateChangeAction;   /* Keep last time value of LastSecTxRateChangeAction */
	UINT_8  ucLastTxPER;                    /* Tx PER in last Rate Adaptation interval */
	UINT_32 u4LastTxOkCount;                /* TxSuccess count in last Rate Adaptation interval */

	UCHAR   *pucTable;                      /* Pointer to this entry's Tx Rate Table */
	UCHAR   *pucLastTable;
} RA_INTERNAL_INFO_T, *P_RA_INTERNAL_INFO_T;

typedef struct _RA_COMMON_INFO_T {
	UINT_8  OpMode;
	BOOL    fgAdHocOn;
	BOOL    fgShortPreamble;

	UCHAR   TxStream;
	UCHAR   RxStream;

	UINT_8  ucRateAlg;

	BOOL    TestbedForceShortGI;
	BOOL    TestbedForceGreenField;
#ifdef DOT11_N_SUPPORT
	UCHAR   HtMode;
	BOOL    fAnyStation20Only;              /* Check if any Station can't support GF. */
	BOOL    bRcvBSSWidthTriggerEvents;
#ifdef DOT11_VHT_AC
	UCHAR   vht_nss_cap;
#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */

	BOOL    fgThermalProtectToggle;
	BOOL    force_one_tx_stream;

	UINT_8  ucForceTxStream;

	BOOL    fgSeOff;
	UINT_8  ucAntennaIndex;

	UINT_8  TrainUpRule;                    /* QuickDRS train up criterion: 0=>Throughput, 1=>PER, 2=> Throughput & PER */
	USHORT  TrainUpHighThrd;
	SHORT   TrainUpRuleRSSI;
	USHORT  lowTrafficThrd;

#if defined(MT7615) || defined(MT7622) || defined(P18) || defined(MT7663)
	UINT_16 u2MaxPhyRate;
#endif /* defined(MT7615) || defined(MT7622) || defined(P18) || defined(MT7663) */

	UINT_32 PhyCaps;                        /* pAd->chipCap.phy_caps 0x1:2.4G 0x2:5G 0x10:ht 0x20:vht 0x100:TXBF 0x200:ldpc */

	UINT_32 u4RaInterval;
	UINT_32 u4RaFastInterval;

#ifdef WIFI_BUILD_RAM
	UINT_8  ucStaCount;
	BOOL    fgRaCfgSet;
	BOOL    fgBtCoExEnable;
	UINT_8  ucBtCoExBitMap;
	BOOL    fgPwrSavingEnable;
	BOOL    fgThroughputBurst;
	RA_PHY_CFG_T rLastTxCfg;
#endif /* WIFI_BUILD_RAM */

} RA_COMMON_INFO_T, *P_RA_COMMON_INFO_T;

typedef struct _BSSINFO_AUTO_RATE_CFG_T {
    /* Auto Rate (Group10) */
	UINT_16 u2Tag;                          /* Tag = 10 */
	UINT_16 u2Length;

	UINT_8  OpMode;
	BOOL    fgAdHocOn;
	BOOL    fgShortPreamble;

	UCHAR   TxStream;
	UCHAR   RxStream;

	UINT_8  ucRateAlg;

	BOOL    TestbedForceShortGI;
	BOOL    TestbedForceGreenField;
	UCHAR   HtMode;
	BOOL    fAnyStation20Only;              /* Check if any Station can't support GF. */
	BOOL    bRcvBSSWidthTriggerEvents;
	UCHAR   vht_nss_cap;
	UCHAR   vht_bw_signal;					/* not use */
	BOOL    vht_force_sgi;					/* not use */

	BOOL    fgSeOff;
	UINT_8  ucAntennaIndex;

	UINT_8  TrainUpRule;                    /* QuickDRS train up criterion: 0=>Throughput, 1=>PER, 2=> Throughput & PER */
	UINT_8  Resv[3];
	USHORT  TrainUpHighThrd;
	SHORT   TrainUpRuleRSSI;
	USHORT  lowTrafficThrd;

	UINT_16 u2MaxPhyRate;

	UINT_32 PhyCaps;                        /* pAd->chipCap.phy_caps 0x1:2.4G 0x2:5G 0x10:ht 0x20:vht 0x100:TXBF 0x200:ldpc */

	UINT_32 u4RaInterval;
	UINT_32 u4RaFastInterval;
} CMD_BSSINFO_AUTO_RATE_CFG_T, *P_CMD_BSSINFO_AUTO_RATE_CFG_T;

typedef struct _STAREC_AUTO_RATE_T {
	/* Auto Rate (Group1) */
	UINT_16 u2Tag;                          /* Tag = 0x01 */
	UINT_16 u2Length;
	BOOL    fgRaValid;
	BOOL    fgAutoTxRateSwitch;

	UINT_8  ucPhyMode;                      /* wdev->PhyMode WMODE_CAP_AC */
	UINT_8  ucChannel;
	UINT_8  ucBBPCurrentBW;                 /* BW_10, BW_20, BW_40, BW_80 */

	BOOL    fgDisableCCK;

	BOOL    fgHtCapMcs32;
	BOOL    fgHtCapInfoGF;
	UCHAR   aucHtCapMCSSet[4];
	UCHAR   ucMmpsMode;

	UINT_8  ucGband256QAMSupport;
	UINT_8  ucMaxAmpduFactor;

	BOOL    fgAuthWapiMode;                 /* Security hardware limitation workaround */

	UCHAR   RateLen;
	UCHAR   ucSupportRateMode;
	UINT_8  ucSupportCCKMCS;
	UINT_8  ucSupportOFDMMCS;
	UINT_32 u4SupportHTMCS;
	UINT_16 u2SupportVHTMCS1SS;
	UINT_16 u2SupportVHTMCS2SS;
	UINT_16 u2SupportVHTMCS3SS;
	UINT_16 u2SupportVHTMCS4SS;
	BOOL    force_op_mode;
	UINT_8  vhtOpModeChWidth;
	UINT_8  vhtOpModeRxNss;
	UINT_8  vhtOpModeRxNssType;

	UINT_32 ClientStatusFlags;

	RA_PHY_CFG_T MaxPhyCfg;
} CMD_STAREC_AUTO_RATE_T, *P_CMD_STAREC_AUTO_RATE_T;

typedef struct _STAREC_AUTO_RATE_CFG_T {
	/* Auto Rate (Group2) */
	UINT_16 u2Tag;                          /* Tag = 0x02 */
	UINT_16 u2Length;

	UINT_8  OpMode;
	BOOL    fgAdHocOn;
	BOOL    fgShortPreamble;

	UCHAR   TxStream;
	UCHAR   RxStream;

	UINT_8  ucRateAlg;

	BOOL    TestbedForceShortGI;
	BOOL    TestbedForceGreenField;
	UCHAR   HtMode;
	BOOL    fAnyStation20Only;              /* Check if any Station can't support GF. */
	BOOL    bRcvBSSWidthTriggerEvents;
	UCHAR   vht_nss_cap;
	UCHAR   vht_bw_signal;					/* not use */
	BOOL    vht_force_sgi;					/* not use */

	BOOL    fgSeOff;
	UINT_8  ucAntennaIndex;

	UINT_8  TrainUpRule;                    /* QuickDRS train up criterion: 0=>Throughput, 1=>PER, 2=> Throughput & PER */
	UINT_8  Resv[3];
	USHORT  TrainUpHighThrd;
	SHORT   TrainUpRuleRSSI;
	USHORT  lowTrafficThrd;

	UINT_16 u2MaxPhyRate;

	UINT_32 PhyCaps;                        /* pAd->chipCap.phy_caps 0x1:2.4G 0x2:5G 0x10:ht 0x20:vht 0x100:TXBF 0x200:ldpc */

	UINT_32 u4RaInterval;
	UINT_32 u4RaFastInterval;
} CMD_STAREC_AUTO_RATE_CFG_T, *P_CMD_STAREC_AUTO_RATE_CFG_T;

typedef struct _STAREC_AUTO_RATE_UPDATE_T {
	/* Auto Rate (Group3) */
	UINT_16 u2Tag;                          /* Tag = 0x03 */
	UINT_16 u2Length;

	UINT_32 u4Field;

	BOOL    force_op_mode;
	UINT_8  vhtOpModeChWidth;
	UINT_8  vhtOpModeRxNss;
	UINT_8  vhtOpModeRxNssType;

	RA_PHY_CFG_T FixedRateCfg;
	UINT_8  ucSpeEn;
	UINT_8  ucShortPreamble;
	BOOL    fgIs5G;

	UINT_8  ucMmpsMode;
} CMD_STAREC_AUTO_RATE_UPDATE_T, *P_CMD_STAREC_AUTO_RATE_UPDATE_T;

typedef struct _EXT_CMD_GET_TX_STATISTIC_T {
	UINT_32 u4Field;
	UINT_8 ucWlanIdx;
	UINT_8 ucBandIdx;
	UINT_8 aucReserved[6];
} EXT_CMD_GET_TX_STATISTIC_T, *P_EXT_CMD_GET_TX_STATISTIC_T;

typedef struct _CMD_SET_MAX_PHY_RATA_T {
	UINT_16 u2MaxPhyRate;
	UINT_8  aucReserve[2];
} CMD_SET_MAX_PHY_RATA, *P_CMD_SET_MAX_PHY_RATA;

typedef struct _EXT_EVENT_MAX_AMSDU_LENGTH_UPDATE_T {
	UINT_8 ucWlanIdx;
	UINT_8 ucAmsduLen;
} EXT_EVENT_MAX_AMSDU_LENGTH_UPDATE_T, *P_EXT_EVENT_MAX_AMSDU_LENGTH_UPDATE_T;

typedef struct _EXT_EVENT_TX_STATISTIC_RESULT_T {
	UINT_8 ucWlanIdx;
	UINT_8 ucBandIdx;
	UINT_8 aucResv1[2];
	UINT_32 u4Field;
	UINT_32 u4TotalTxCount;
	UINT_32 u4TotalTxFailCount;
	UINT_32 u4CurrBwTxCnt;
	UINT_32 u4CurrBwTxFailCnt;
	UINT_32 u4OtherBwTxCnt;
	UINT_32 u4OtherBwTxFailCnt;
	UINT_32 u4EntryTxCount;
	UINT_32 u4EntryTxFailCount;
	RA_PHY_CFG_T rLastTxRate;
	RA_PHY_CFG_T rEntryTxRate;
	UINT_8 aucResv2[8];
} EXT_EVENT_TX_STATISTIC_RESULT_T, *P_EXT_EVENT_TX_STATISTIC_RESULT_T;

typedef struct _EXT_EVENT_G_BAND_256QAM_PROBE_RESULT_T {
	UINT_8 ucWlanIdx;
	UINT_8 ucResult;
	UINT_8 aucReserved[2];
} EXT_EVENT_G_BAND_256QAM_PROBE_RESULT_T, *P_EXT_EVENT_G_BAND_256QAM_PROBE_RESULT_T;
#endif /* defined(NEW_RATE_ADAPT_SUPPORT) || defined(RATE_ADAPT_AGBS_SUPPORT) */

#ifdef NEW_RATE_ADAPT_SUPPORT
typedef struct  _RTMP_RA_GRP_TB {
	UCHAR   ItemNo;
#ifdef RT_BIG_ENDIAN
	UCHAR   Rsv2: 1;
	UCHAR   Mode: 3;
	UCHAR   BW: 2;
	UCHAR   ShortGI: 1;
	UCHAR   STBC: 1;
#else
	UCHAR   STBC: 1;
	UCHAR   ShortGI: 1;
	UCHAR   BW: 2;
	UCHAR   Mode: 3;
	UCHAR   Rsv2: 1;
#endif
	UCHAR   CurrMCS;
	UCHAR   TrainUp;
	UCHAR   TrainDown;
	UCHAR   downMcs;
	UCHAR   upMcs3;
	UCHAR   upMcs2;
	UCHAR   upMcs1;
	UCHAR   dataRate;
} RTMP_RA_GRP_TB;
#endif /* NEW_RATE_ADAPT_SUPPORT */

#ifdef RATE_ADAPT_AGBS_SUPPORT
typedef struct  __RA_AGBS_TABLE_ENTRY {
	UCHAR   ItemNo;
#ifdef RT_BIG_ENDIAN
	UCHAR   Rsv2: 1;
	UCHAR   Mode: 3;
	UCHAR   BW: 2;
	UCHAR   ShortGI: 1;
	UCHAR   STBC: 1;
#else
	UCHAR   STBC: 1;
	UCHAR   ShortGI: 1;
	UCHAR   BW: 2;
	UCHAR   Mode: 3;
	UCHAR   Rsv2: 1;
#endif
	UCHAR   CurrMCS;
	UCHAR   TrainUp;
	UCHAR   TrainDown;
	UCHAR   UpIdx;
	UCHAR   DownIdx;
	UCHAR   NssUpIdx;
	UCHAR   NssDnIdx;
	UCHAR   ChangeBw;
#ifdef RT_BIG_ENDIAN
	UCHAR   AmsduLenBw40: 4;
	UCHAR   AmsduLenBw20: 4;
	UCHAR   AmsduLenBw160: 4;
	UCHAR   AmsduLenBw80: 4;
#else
	UCHAR   AmsduLenBw20: 4;
	UCHAR   AmsduLenBw40: 4;
	UCHAR   AmsduLenBw80: 4;
	UCHAR   AmsduLenBw160: 4;
#endif
	UCHAR   TxCnt;
	UCHAR   CBRN;
} *P_RA_AGBS_TABLE_ENTRY, RA_AGBS_TABLE_ENTRY;

typedef struct __RA_AGBS_UPRATE_INFO {
	UINT_8 ucUpRateIdx;
	UINT_8 ucUpRateGrp;
	UINT_8 ucNssUpIdx;
	UINT_8 ucNssUpGrp;
	UINT_8 ucNssDnIdx;
	UINT_8 ucNssDnGrp;
} *P_RA_AGBS_UPRATE_INFO, RA_AGBS_UPRATE_INFO;

union RA_RATE_CODE {
	struct {
		UINT_16 mcs: 6;
		UINT_16 mode: 3;
		UINT_16 nsts: 2;
		UINT_16 stbc: 1;
		UINT_16 reserved: 4;
	} field;
	UINT_16 word;
};

typedef struct __RA_INIT_RATE_ENTRY {
	UCHAR *pucTable;
	UCHAR ucMcsGrp;
	CHAR cRateIndex;
	CHAR cRssi;
} *P_RA_INIT_RATE_ENTRY, RA_INIT_RATE_ENTRY;

typedef struct __RA_PHY_RATE_INFO {
	UINT_16 u2Bw20LGI;
	UINT_16 u2Bw20SGI;
	UINT_16 u2Bw40LGI;
	UINT_16 u2Bw40SGI;
	UINT_16 u2Bw80LGI;
	UINT_16 u2Bw80SGI;
	UINT_16 u2Bw160LGI;
	UINT_16 u2Bw160SGI;
} *P_RA_PHY_RATE_INFO, RA_PHY_RATE_INFO;

#endif /* RATE_ADAPT_AGBS_SUPPORT */


enum RATE_ADAPT_ALG {
	RATE_ALG_LEGACY = 1,
	RATE_ALG_GRP = 2,
	RATE_ALG_AGS = 3,
	RATE_ALG_AGBS = 4,
	RATE_ALG_MAX_NUM
};

typedef enum {
	RAL_OLD_DRS,
	RAL_NEW_DRS,
	RAL_QUICK_DRS
} RA_LOG_TYPE;

typedef enum {
	RA_PARAM_VHT_OPERATING_MODE = 1,
	RA_PARAM_HT_2040_COEX = 2,
	RA_PARAM_FIXED_RATE = 3,
	RA_PARAM_FIXED_RATE_FALLBACK = 4,
	RA_PARAM_MMPS_UPDATE = 5,
	RA_PARAM_HT_2040_BACK = 6,
	RA_PARAM_MAX
} AUTO_RATE_UPDATE_PARAM;

typedef enum {
	RA_DYNAMIC_SGI_INITIAL_STATE = 0,
	RA_DYNAMIC_SGI_TRYING_STATE,
	RA_DYNAMIC_SGI_TRY_SUCCESS_STATE,
	RA_DYNAMIC_SGI_TRY_FAIL_STATE
} DYNAMIC_SGI_STATE;

typedef enum {
	RA_DYNAMIC_BW_UNCHANGED_STATE = 0,
	RA_DYNAMIC_BW_DOWN_STATE,
	RA_DYNAMIC_BW_UP_FAIL_STATE
} DYNAMIC_BW_STATE;

typedef enum {
	RA_G_BAND_256QAM_DISABLE = 0,
	RA_G_BAND_256QAM_ENABLE,
	RA_G_BAND_256QAM_PROBING
} G_BAND_256QAM_CONFIG;

typedef enum {
	RA_G_BAND_256QAM_PROBE_FAIL = 0,
	RA_G_BAND_256QAM_PROBE_SUCCESS,
} G_BAND_256QAM_PROBE_RESULT;

#if defined(COMPOS_WIN) || defined(WIFI_BUILD_RAM)
enum WIFI_MODE {
	WMODE_INVALID = 0,
	WMODE_A = 1 << 0,
	WMODE_B = 1 << 1,
	WMODE_G = 1 << 2,
	WMODE_GN = 1 << 3,
	WMODE_AN = 1 << 4,
	WMODE_AC = 1 << 5,
	WMODE_COMP = 6,                         /* total types of supported wireless mode, add this value once yow add new type */
};
#endif /* COMPOS_WIN */

/*******************************************************************************
*                                 M A C R O S
********************************************************************************
*/
#define PTX_RA_LEGACY_ENTRY(pTable, idx)    ((RTMP_RA_LEGACY_TB *)&(pTable[(idx+1)*5]))
#define RATE_TABLE_SIZE(pTable)             ((pTable)[0])       /* Byte 0 is number of rate indices */
#define RATE_TABLE_INIT_INDEX(pTable)       ((pTable)[1])       /* Byte 1 is initial rate index */
#define PTX_RA_GRP_ENTRY(pTable, idx)       ((RTMP_RA_GRP_TB *)&(pTable[(idx+1)*10]))

#define RA_AGBS_ENTRY(pTable, idx)          ((idx < pTable[0]?(RA_AGBS_TABLE_ENTRY *)&(pTable[(idx+1)*14]):(RA_AGBS_TABLE_ENTRY *)&(pTable[(1)*14])))

/* normal rate switch */
#define RTMP_DRS_ALG_INIT(__pAd, __Alg)     ((__pAd)->rateAlg = __Alg)

#define VHT_INVALID_RATE(__MCS, __Nss, __Bw)    \
	(((__Bw == BW_20) && (__Nss == 1) && (__MCS == MCS_9)) || \
	 ((__Bw == BW_20) && (__Nss == 2) && (__MCS == MCS_9)) || \
	 ((__Bw == BW_20) && (__Nss == 4) && (__MCS == MCS_9)) || \
	 ((__Bw == BW_80) && (__Nss == 3) && (__MCS == MCS_6)) || \
	 ((__Bw == BW_160) && (__Nss == 3) && (__MCS == MCS_9)))

#ifdef NEW_RATE_ADAPT_SUPPORT
#ifdef DOT11_N_SUPPORT
#ifdef DOT11_VHT_AC
/* VHT */
#define ADAPT_RATE_TABLE(pTable)    ((pTable) == RateSwitchTableAdapt11B || \
									 (pTable) == RateSwitchTableAdapt11G || \
									 (pTable) == RateSwitchTableAdapt11BG || \
									 (pTable) == RateSwitchTableAdapt11N1S ||\
									 (pTable) == RateSwitchTableAdapt11N2S ||\
									 (pTable) == RateSwitchTableAdapt11N3S ||\
									 (pTable) == RateTableVht1S ||\
									 (pTable) == RateTableVht1S_MCS9 ||\
									 (pTable) == RateTableVht2S || \
									 (pTable) == RateTableVht2S_MCS7 || \
									 (pTable) == RateTableVht2S_BW20 ||\
									 (pTable) == RateTableVht2S_BW40)
#else
/* 11n */
#define ADAPT_RATE_TABLE(pTable)    ((pTable) == RateSwitchTableAdapt11B || \
									 (pTable) == RateSwitchTableAdapt11G || \
									 (pTable) == RateSwitchTableAdapt11BG || \
									 (pTable) == RateSwitchTableAdapt11N1S || \
									 (pTable) == RateSwitchTableAdapt11N2S || \
									 (pTable) == RateSwitchTableAdapt11N3S)
#endif /* DOT11_VHT_AC */
#else
/* Legacy */
#define ADAPT_RATE_TABLE(pTable)    ((pTable) == RateSwitchTableAdapt11B || \
									 (pTable) == RateSwitchTableAdapt11G || \
									 (pTable) == RateSwitchTableAdapt11BG)
#endif /* DOT11_N_SUPPORT */
#endif /* NEW_RATE_ADAPT_SUPPORT */

#ifdef RATE_ADAPT_AGBS_SUPPORT
#ifdef DOT11_N_SUPPORT
#define AGBS_HT_TABLE(pTable)       ((pTable) == RateSwitchTableAGBS11N1SS ||\
									 (pTable) == RateSwitchTableAGBS11N2SS ||\
									 (pTable) == RateSwitchTableAGBS11N3SS ||\
									 (pTable) == RateSwitchTableAGBS11N4SS)
#ifdef DOT11_VHT_AC
/* VHT */
#define AGBS_VHT_TABLE(pTable)      ((pTable) == RateSwitchTableAGBSVht1SS ||\
									 (pTable) == RateSwitchTableAGBSVht2SS || \
									 (pTable) == RateSwitchTableAGBSVht3SS || \
									 (pTable) == RateSwitchTableAGBSVht4SS)

#define RATE_TABLE_AGBS(pTable)     ((pTable) == RateSwitchTableAGBS11B || \
									 (pTable) == RateSwitchTableAGBS11G || \
									 (pTable) == RateSwitchTableAGBS11BG || \
									 (pTable) == RateSwitchTableAGBS11N1SS ||\
									 (pTable) == RateSwitchTableAGBS11N2SS ||\
									 (pTable) == RateSwitchTableAGBS11N3SS ||\
									 (pTable) == RateSwitchTableAGBS11N4SS ||\
									 (pTable) == RateSwitchTableAGBSVht1SS ||\
									 (pTable) == RateSwitchTableAGBSVht2SS || \
									 (pTable) == RateSwitchTableAGBSVht3SS || \
									 (pTable) == RateSwitchTableAGBSVht4SS)
#else
/* 11n */
#define RATE_TABLE_AGBS(pTable)     ((pTable) == RateSwitchTableAGBS11B || \
									 (pTable) == RateSwitchTableAGBS11G || \
									 (pTable) == RateSwitchTableAGBS11BG || \
									 (pTable) == RateSwitchTableAGBS11N1SS ||\
									 (pTable) == RateSwitchTableAGBS11N2SS ||\
									 (pTable) == RateSwitchTableAGBS11N3SS ||\
									 (pTable) == RateSwitchTableAGBS11N4SS)
#endif /* DOT11_VHT_AC */
#else
/* legacy */
#define RATE_TABLE_AGBS(pTable)     ((pTable) == RateSwitchTableAGBS11B || \
									 (pTable) == RateSwitchTableAGBS11G || \
									 (pTable) == RateSwitchTableAGBS11BG)
#endif /* DOT11_N_SUPPORT */
#endif /* RATE_ADAPT_AGBS_SUPPORT */

#ifdef WIFI_BUILD_RAM
#define RA_SAVE_LAST_TX_CFG(__pRaEntry)                         \
	{                                                               \
		extern RA_COMMON_INFO_T g_rRaCfg;                           \
		g_rRaCfg.rLastTxCfg.MODE = __pRaEntry->TxPhyCfg.MODE;       \
		g_rRaCfg.rLastTxCfg.MCS = __pRaEntry->TxPhyCfg.MCS;         \
		g_rRaCfg.rLastTxCfg.BW = __pRaEntry->TxPhyCfg.BW;           \
		g_rRaCfg.rLastTxCfg.ldpc = __pRaEntry->TxPhyCfg.ldpc ? 1:0; \
		g_rRaCfg.rLastTxCfg.ShortGI = __pRaEntry->TxPhyCfg.ShortGI?1:0; \
		g_rRaCfg.rLastTxCfg.STBC = __pRaEntry->TxPhyCfg.STBC;       \
	}
#else
#define RA_SAVE_LAST_TX_CFG(__pRaEntry)                         \
	{                                                               \
		HTTRANSMIT_SETTING LastTxRate;                              \
		LastTxRate.field.MODE = __pRaEntry->TxPhyCfg.MODE;          \
		LastTxRate.field.MCS = __pRaEntry->TxPhyCfg.MCS;            \
		LastTxRate.field.BW = __pRaEntry->TxPhyCfg.BW;              \
		LastTxRate.field.ldpc = __pRaEntry->TxPhyCfg.ldpc ? 1:0;    \
		LastTxRate.field.ShortGI = __pRaEntry->TxPhyCfg.ShortGI?1:0;\
		LastTxRate.field.STBC = __pRaEntry->TxPhyCfg.STBC;          \
		\
		pAd->LastTxRate = (USHORT)(LastTxRate.word);                \
	}
#endif /* WIFI_BUILD_RAM */


#if defined(COMPOS_WIN) || defined(WIFI_BUILD_RAM)
#define WMODE_EQUAL(_x, _mode)              ((_x) == (_mode))
#define WMODE_CAP_N(_x)                     (((_x) & (WMODE_GN | WMODE_AN)) != 0)
#define WMODE_CAP_AC(_x)                    (((_x) & (WMODE_AC)) != 0)
#endif /* COMPOS_WIN */

/*******************************************************************************
*                    E X T E R N A L   R E F E R E N C E S
********************************************************************************
*/


#ifdef NEW_RATE_ADAPT_SUPPORT
extern UCHAR RateSwitchTableAdapt11B[];
extern UCHAR RateSwitchTableAdapt11G[];
extern UCHAR RateSwitchTableAdapt11BG[];

#ifdef DOT11_N_SUPPORT
extern UCHAR RateSwitchTableAdapt11N1S[];
extern UCHAR RateSwitchTableAdapt11N2S[];
extern UCHAR RateSwitchTableAdapt11N3S[];

/* ADAPT_RATE_TABLE - true if pTable is one of the Adaptive Rate Switch tables */
#ifdef DOT11_VHT_AC
extern UCHAR RateTableVht1S[];
extern UCHAR RateTableVht1S_MCS9[];
extern UCHAR RateTableVht2S[];
extern UCHAR RateTableVht2S_MCS7[];
extern UCHAR RateTableVht2S_BW20[];
extern UCHAR RateTableVht2S_BW40[];
#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT*/
#endif /* NEW_RATE_ADAPT_SUPPORT */

#ifdef RATE_ADAPT_AGBS_SUPPORT
extern UCHAR RateSwitchTableAGBS11B[];
extern UCHAR RateSwitchTableAGBS11G[];
extern UCHAR RateSwitchTableAGBS11BG[];

extern UINT_16 HwFallbackTable11B[32];
extern UINT_16 HwFallbackTable11G[64];
/* extern UINT_16 HwFallbackTable11BG[48]; */
extern UINT_16 HwFallbackTable11BG[56];

extern RA_INIT_RATE_ENTRY RaInitRateTable11B[4];
extern RA_INIT_RATE_ENTRY RaInitRateTable11G[8];
extern RA_INIT_RATE_ENTRY RaInitRateTable11BG[8];

#ifdef DOT11_N_SUPPORT
extern UCHAR RateSwitchTableAGBS11N1SS[];
extern UCHAR RateSwitchTableAGBS11N2SS[];
extern UCHAR RateSwitchTableAGBS11N3SS[];
extern UCHAR RateSwitchTableAGBS11N4SS[];

extern UCHAR *AgbsHtMappingTable[];

extern UINT_16 HwFallbackTable11N1SS[80];
extern UINT_16 HwFallbackTable11N2SS[80];
extern UINT_16 HwFallbackTable11N3SS[80];
extern UINT_16 HwFallbackTable11N4SS[80];
extern UINT_16 HwFallbackTableBGN1SS[80];
extern UINT_16 HwFallbackTableBGN2SS[80];
extern UINT_16 HwFallbackTableBGN3SS[80];
extern UINT_16 HwFallbackTableBGN4SS[80];

extern RA_INIT_RATE_ENTRY RaInitRateTable11N1SS[10];
extern RA_INIT_RATE_ENTRY RaInitRateTable11N2SS[10];
extern RA_INIT_RATE_ENTRY RaInitRateTable11N3SS[10];
extern RA_INIT_RATE_ENTRY RaInitRateTable11N4SS[10];

#ifdef DOT11_VHT_AC
extern UCHAR RateSwitchTableAGBSVht1SS[];
extern UCHAR RateSwitchTableAGBSVht2SS[];
extern UCHAR RateSwitchTableAGBSVht3SS[];
extern UCHAR RateSwitchTableAGBSVht4SS[];

extern UCHAR *AgbsVhtMappingTable[];

extern UINT_16 HwFallbackTableVht1SS[80];
extern UINT_16 HwFallbackTableVht2SS[80];
extern UINT_16 HwFallbackTableVht3SS[80];
/*
extern UINT_16 HwFallbackTableVht3SSBw80[24];
*/
extern UINT_16 HwFallbackTableVht4SS[80];

extern RA_INIT_RATE_ENTRY RaInitRateTableVht1SS[11];
extern RA_INIT_RATE_ENTRY RaInitRateTableVht2SS[11];
extern RA_INIT_RATE_ENTRY RaInitRateTableVht3SS[11];
extern RA_INIT_RATE_ENTRY RaInitRateTableVht4SS[11];
#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */

extern RA_PHY_RATE_INFO RaPhyRate1SS[];
extern RA_PHY_RATE_INFO RaPhyRate2SS[];
extern RA_PHY_RATE_INFO RaPhyRate3SS[];
extern RA_PHY_RATE_INFO RaPhyRate4SS[];

#endif /* RATE_ADAPT_AGBS_SUPPORT */

/*******************************************************************************
*                   F U N C T I O N   D E C L A R A T I O N S (MT)
********************************************************************************
*/
#ifdef MT_MAC
CHAR
raMaxRssi(
	IN struct _RA_COMMON_INFO_T *pRaCfg,
	IN CHAR Rssi0,
	IN CHAR Rssi1,
	IN CHAR Rssi2
);

CHAR
raMinRssi(
	IN struct _RA_COMMON_INFO_T *pRaCfg,
	IN CHAR Rssi0,
	IN CHAR Rssi1,
	IN CHAR Rssi2
);

VOID
raWrapperEntrySet(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct _MAC_TABLE_ENTRY *pEntry,
	OUT struct _RA_ENTRY_INFO_T *pRaEntry
);

VOID
raWrapperEntryRestore(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct _MAC_TABLE_ENTRY *pEntry,
	IN struct _RA_ENTRY_INFO_T *pRaEntry
);

VOID
raWrapperConfigSet(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	OUT struct _RA_COMMON_INFO_T *pRaCfg
);

VOID
QuickResponeForRateAdaptMT(
	IN struct _RTMP_ADAPTER *pAd, UINT_8 idx
);

VOID
DynamicTxRateSwitchingAdaptMT(
	IN struct _RTMP_ADAPTER *pAd, UINT_8 idx
);

VOID
raSelectTxRateTable(
	IN struct _RA_ENTRY_INFO_T *pRaEntry,
	IN struct _RA_COMMON_INFO_T *pRaCfg,
	IN struct _RA_INTERNAL_INFO_T *pRaInternal,
	OUT UCHAR **ppTable,
	OUT UCHAR *pTableSize,
	OUT UCHAR *pInitTxRateIdx
);

VOID
RAInit(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct _MAC_TABLE_ENTRY *pEntry
);

VOID
RAParamUpdate(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct _MAC_TABLE_ENTRY *pEntry,
	IN struct _STAREC_AUTO_RATE_UPDATE_T *prParam
);

VOID RATriggerQuickResponeTimer(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct _RA_ENTRY_INFO_T *pRaEntry
);

#ifdef NEW_RATE_ADAPT_SUPPORT
VOID
raClearTxQuality(
	IN struct _RA_INTERNAL_INFO_T *pRaInternal
);

VOID
raDecTxQuality(
	IN struct _RA_INTERNAL_INFO_T *pRaInternal,
	IN UCHAR ucRateIndex
);

VOID
raSetTxQuality(
	IN struct _RA_INTERNAL_INFO_T *pRaInternal,
	IN UINT_8 ucRateIndex,
	IN UINT_8 ucQuality
);

UINT_8
raGetTxQuality(
	IN struct _RA_INTERNAL_INFO_T *pRaInternal,
	IN UINT_8 ucRateIndex
);

VOID
raRestoreLastRate(
	IN struct _RA_INTERNAL_INFO_T *pRaInternal
);

VOID
raSetMcsGroup(
	IN struct _RA_ENTRY_INFO_T *pRaEntry,
	IN struct _RA_COMMON_INFO_T *pRaCfg,
	IN struct _RA_INTERNAL_INFO_T *pRaInternal
);

UCHAR
raSelectUpRate(
	IN struct _RA_ENTRY_INFO_T *pRaEntry,
	IN struct _RA_COMMON_INFO_T *pRaCfg,
	IN struct _RA_INTERNAL_INFO_T *pRaInternal,
	IN struct _RTMP_RA_GRP_TB *pCurrTxRate
);

UCHAR
raSelectDownRate(
	IN struct _RA_ENTRY_INFO_T *pRaEntry,
	IN struct _RA_COMMON_INFO_T *pRaCfg,
	IN struct _RA_INTERNAL_INFO_T *pRaInternal,
	IN UCHAR CurrRateIdx
);

UCHAR*
raSelectTxRateTableGRP(
	IN struct _RA_ENTRY_INFO_T *pRaEntry,
	IN struct _RA_COMMON_INFO_T *pRaCfg,
	IN struct _RA_INTERNAL_INFO_T *pRaInternal
);

VOID
QuickResponeForRateAdaptMTCore(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct _RA_ENTRY_INFO_T *pRaEntry,
	IN struct _RA_COMMON_INFO_T *pRaCfg,
	IN struct _RA_INTERNAL_INFO_T *pRaInternal
);

VOID
DynamicTxRateSwitchingAdaptMtCore(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct _RA_ENTRY_INFO_T *pRaEntry,
	IN struct _RA_COMMON_INFO_T *pRaCfg,
	IN struct _RA_INTERNAL_INFO_T *pRaInternal
);

VOID
NewTxRateMtCore(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct _RA_ENTRY_INFO_T *pRaEntry,
	IN struct _RA_COMMON_INFO_T *pRaCfg,
	IN struct _RA_INTERNAL_INFO_T *pRaInternal
);
#endif /* NEW_RATE_ADAPT_SUPPORT */

#ifdef RATE_ADAPT_AGBS_SUPPORT
VOID
raClearTxQualityAGBS(
	IN struct _RA_INTERNAL_INFO_T *pRaInternal
);

VOID
raPrintRateTableNameAGBS(
	IN UCHAR *pTable
);

UINT_16
raGetPhyRate(
	IN UINT_8 ucMode,
	IN UINT_8 ucMcs,
	IN UINT_8 ucVhtNss,
	IN UINT_8 ucBw,
	IN UINT_8 ucSgi
);

VOID
raSetMcsGroupAGBS(
	IN struct _RA_ENTRY_INFO_T *pRaEntry,
	IN struct _RA_COMMON_INFO_T *pRaCfg,
	IN struct _RA_INTERNAL_INFO_T *pRaInternal
);

VOID
QuickResponeForRateAdaptAGBSMTCore(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct _RA_ENTRY_INFO_T *pRaEntry,
	IN struct _RA_COMMON_INFO_T *pRaCfg,
	IN struct _RA_INTERNAL_INFO_T *pRaInternal
);

VOID
DynamicTxRateSwitchingAGBSMtCore(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct _RA_ENTRY_INFO_T *pRaEntry,
	IN struct _RA_COMMON_INFO_T *pRaCfg,
	IN struct _RA_INTERNAL_INFO_T *pRaInternal
);

UCHAR*
raSelectVHTTxRateTableAGBS(
	IN struct _RA_ENTRY_INFO_T *pRaEntry,
	IN struct _RA_COMMON_INFO_T *pRaCfg,
	IN struct _RA_INTERNAL_INFO_T *pRaInternal
);

UCHAR*
raSelectTxRateTableAGBS(
	IN struct _RA_ENTRY_INFO_T *pRaEntry,
	IN struct _RA_COMMON_INFO_T *pRaCfg,
	IN struct _RA_INTERNAL_INFO_T *pRaInternal
);

VOID
SetTxRateMtCoreAGBS(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct _RA_ENTRY_INFO_T *pRaEntry,
	IN struct _RA_COMMON_INFO_T *pRaCfg,
	IN struct _RA_INTERNAL_INFO_T *pRaInternal
);

VOID
raMaxAmsduLenNotifyAGBS(
	IN struct _RA_ENTRY_INFO_T *pRaEntry,
	IN struct _RA_INTERNAL_INFO_T *pRaInternal
);
#endif /* RATE_ADAPT_AGBS_SUPPORT */

#ifdef MCS_LUT_SUPPORT
UINT_8
raStbcSettingCheck(
	UINT_8 ucOrigStbc,
	UINT_8 ucMode,
	UINT_8 ucMcs,
	UINT_8 ucVhtNss,
	BOOL fgBFOn,
	BOOL fgForceOneTx
);
#ifdef NEW_RATE_ADAPT_SUPPORT
VOID
MtAsicMcsLutUpdateCore(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct _RA_ENTRY_INFO_T *pRaEntry,
	IN struct _RA_COMMON_INFO_T *pRaCfg,
	IN struct _RA_INTERNAL_INFO_T *pRaInternal
);
#endif /* NEW_RATE_ADAPT_SUPPORT */

#ifdef RATE_ADAPT_AGBS_SUPPORT
VOID
MtAsicMcsLutUpdateCoreAGBS(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct _RA_ENTRY_INFO_T *pRaEntry,
	IN struct _RA_COMMON_INFO_T *pRaCfg,
	IN struct _RA_INTERNAL_INFO_T *pRaInternal
);
#endif /* RATE_ADAPT_AGBS_SUPPORT */
#endif /* MCS_LUT_SUPPORT */

#ifdef DBG
INT
Set_Fixed_Rate_Proc(
	IN struct _RTMP_ADAPTER *pAd,
	IN RTMP_STRING * arg
);

INT
Set_Fixed_Rate_With_FallBack_Proc(
	IN struct _RTMP_ADAPTER *pAd,
	IN RTMP_STRING * arg
);

INT
Set_RA_Debug_Proc(
	IN struct _RTMP_ADAPTER *pAd,
	IN RTMP_STRING * arg
);
#endif /* DBG */

#if defined(MT7615) || defined(MT7622) || defined(P18) || defined(MT7663)
INT32
BssInfoRACommCfgSet(
	IN P_RA_COMMON_INFO_T pRaCfg,
    OUT P_CMD_BSSINFO_AUTO_RATE_CFG_T pCmdBssInfoAutoRateCfg
);
#endif /* defined(MT7615) || defined(MT7622) || defined(P18) || defined(MT7663) */

#if defined(MT7636) || defined(MT7615) || defined(MT7637) || defined(MT7622) || defined(P18) || defined(MT7663)
INT32
StaRecAutoRateParamSet(
	IN struct _RA_ENTRY_INFO_T *pRaEntry,
	OUT struct _STAREC_AUTO_RATE_T *pCmdStaRecAutoRate
);

INT32
StaRecAutoRateUpdate(
	IN struct _RA_ENTRY_INFO_T *pRaEntry,
	IN struct _RA_INTERNAL_INFO_T *pRaInternal,
	IN struct _STAREC_AUTO_RATE_UPDATE_T *pRaParam,
	OUT struct _STAREC_AUTO_RATE_UPDATE_T *pCmdStaRecAutoRate
);
#endif /* defined(MT7636) || defined(MT7615) || defined(MT7637) || defined(MT7622) || defined(P18) || defined(MT7663) */


#endif /* MT_MAC */


/*******************************************************************************
*                   F U N C T I O N   D E C L A R A T I O N S (RT)
********************************************************************************
*/
#if !defined(COMPOS_WIN) && !defined(WIFI_BUILD_RAM)
VOID
RTMPSetSupportMCS(
	IN struct _RTMP_ADAPTER *pAd,
	IN UCHAR OpMode,
	IN struct _MAC_TABLE_ENTRY *pEntry,
	IN UCHAR SupRate[],
	IN UCHAR SupRateLen,
	IN UCHAR ExtRate[],
	IN UCHAR ExtRateLen,
#ifdef DOT11_VHT_AC
	IN UCHAR vht_cap_len,
	IN struct _VHT_CAP_IE *vht_cap,
#endif /* DOT11_VHT_AC */
	IN struct _HT_CAPABILITY_IE *pHtCapability,
	IN UCHAR HtCapabilityLen
);

#ifdef NEW_RATE_ADAPT_SUPPORT

INT
Set_PerThrdAdj_Proc(
	struct _RTMP_ADAPTER *pAd,
	RTMP_STRING * arg
);

INT
Set_LowTrafficThrd_Proc(
	struct _RTMP_ADAPTER *pAd,
	RTMP_STRING * arg
);

INT
Set_TrainUpRule_Proc(
	struct _RTMP_ADAPTER *pAd,
	RTMP_STRING * arg
);

INT
Set_TrainUpRuleRSSI_Proc(
	struct _RTMP_ADAPTER *pAd,
	RTMP_STRING * arg
);

INT
Set_TrainUpLowThrd_Proc(
	struct _RTMP_ADAPTER *pAd,
	RTMP_STRING * arg
);

INT
Set_TrainUpHighThrd_Proc(
	struct _RTMP_ADAPTER *pAd,
	RTMP_STRING * arg
);

#endif /* NEW_RATE_ADAPT_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
VOID
APMlmeDynamicTxRateSwitching(
	IN struct _RTMP_ADAPTER *pAd
);

VOID
APQuickResponeForRateUpExec(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3
);

#endif /* CONFIG_AP_SUPPORT */


#endif /* COMPOS_WIN */

#endif /* __RA_CTRL_H__ */


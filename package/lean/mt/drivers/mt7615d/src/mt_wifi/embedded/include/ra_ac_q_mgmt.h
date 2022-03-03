/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology	5th	Rd.
 * Science-based Industrial	Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2006, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of Ralink Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	ra_ac_q_mgmt.h
*/

#include    "rt_config.h"

#ifndef _RA_AC_Q_MGMT_H_
#define _RA_AC_Q_MGMT_H_

#if defined(RED_SUPPORT) && (defined(MT7622) || defined(P18) || defined(MT7663))
#define	RED_SUPPORT_BY_HOST
#endif

#define CFG_RED_TRUN_ON_RANDOM_DROP					(0)
#define CFG_ESTIMATION_RED_CPU_UTILIZATION			(0)

#define RED_STA_REC_NUM			MAX_LEN_OF_MAC_TABLE

#define QLEN_SCALED_BIT								(0)
#define PROB_SCALED_BIT								(0)
#define QTH_WRIGTH_BIT								(2)
#define WSCALING_BIT								(16)
#define QLEN_SCALED									(1 << QLEN_SCALED_BIT)
#define PROB_SCALED									(1 << PROB_SCALED_BIT)

#define RED_DROP_TH_LOWER_BOUND						(20)
#define RED_DROP_TH_UPPER_BOUND						(600)

#define RED_VHT_BW20_DEFAULT_THRESHOLD 				(384)
#define RED_VHT_BW40_DEFAULT_THRESHOLD 				(768)
#define RED_VHT_BW80_DEFAULT_THRESHOLD 				(1536)
#define RED_HT_BW20_DEFAULT_THRESHOLD				(192)
#define RED_HT_BW40_DEFAULT_THRESHOLD				(384)
#define RED_LEGACY_DEFAULT_THRESHOLD				(192)

#define RED_WLOG_DEFAULT							(10)
#define RED_MPDU_TIME_INIT							(200)
#define RED_MULTIPLE_NUM_DEFAULT					(30)
#define RA_AC_FREE_FOR_ALL							(3072)

#define RED_BAD_NODE_DROP_THRESHOLD 				(192)
#define RED_BAD_NODE_HT_VHT_DEFAULT_THRESHOLD		(192)
#define RED_BAD_NODE_LEGACY_DEFAULT_THRESHOLD 		(60)
#define	RED_MAX_BAD_NODE_CNT						(10)
#define	RED_MAX_GOOD_NODE_CNT						(7)
#define	RED_BAD_NODE_CNT_MASK						(0x0f)
#define	RED_GOOD_NODE_CNT_MASK						(0x70)
#define	RED_IS_BAD_NODE_MASK						(0x80)
#define	RED_GOOD_NODE_CNT_SHIFT_BIT					(4)
#define	RED_IS_BAD_NODE_SHIFT_BIT					(7)

#define VOW_FEATURE_CONFIGURE_CR					(0x82060370)
#define VOW_WATF_MASK								(0x02000000)
#define VOW_ATF_MASK								(0x20000000)
#define VOW_ATC_SHIFT_BIT							(31)
#define VOW_ATF_SHIFT_BIT							(29)
#define VOW_WATF_SHIFT_BIT							(25)

#define FORCE_RATIO_THRESHOLD						(25)

#define TX_DONE_EVENT_Q_IDX_MASK					(0xf8000000)
#define TX_DONE_EVENT_Q_IDX_SHIFT_BIT				(27)
#define TX_DONE_EVENT_WLAN_ID_MASK					(0x03ff0000)
#define TX_DONE_EVENT_WLAN_ID_SHIFT_BIT				(16)

#define RED_INUSE_BITSHIFT					5
#define RED_INUSE_BITMASK					(0x1f)

/* per AC data structure */
typedef struct _RED_AC_ElEMENT_T {
	UINT32 u2TotalDropCnt;
	UINT16 u2DropCnt;
	UINT16 u2EnqueueCnt;
	UINT16 u2DequeueCnt;
	UINT16 u2qEmptyCnt;
	UINT8 ucShiftBit;
	UINT8 ucGBCnt;
#if (CFG_RED_TRUN_ON_RANDOM_DROP == 1)
	INT8 iWlogBit;
	UINT32 u4AvgLen;
	UINT16 u2qRan;
	UINT16 u2qCount;
	UINT16 u2DropProbCnt;
	UINT16 u2DropTailCnt;
#endif
} RED_AC_ElEMENT_T, *P_RED_AC_ElEMENT_T;

/* per STA data structure */
typedef struct _RED_STA_T{
	INT32 i4MpduTime;
	INT32 tx_msdu_avg_cnt;
	INT32 tx_msdu_cnt;
	UINT16 u4Dropth;
	UINT16 u2DriverFRCnt; /* Record ForceRate counter which is from Driver. */
	UINT8 ucMultiplyNum;
	RED_AC_ElEMENT_T arRedElm[WMM_NUM_OF_AC];
#if (CFG_RED_TRUN_ON_RANDOM_DROP == 1)
	BOOLEAN ucIsBadNode;
	UINT32 u4IGMPCnt;
	UINT32 u4TxmCnt;
#endif
} RED_STA_T, *P_RED_STA_T;


VOID RedInit(PRTMP_ADAPTER pAd);

VOID RedResetSta(UINT8 ucWlanIdx, UINT_8 ucMode, UINT_8 ucBW, struct _RTMP_ADAPTER *pAd);

VOID RedBadNode(UINT8 ucWlanIdx, UINT8 ATC_WATF_Enable, struct _RTMP_ADAPTER *pAd);

bool RedMarkPktDrop(UINT8 ucWlanIdx, UINT8 ucQidx, struct _RTMP_ADAPTER *pAd);

bool red_mark_pktdrop_cr4(UINT8 ucWlanIdx, UINT8 ucQidx, struct _RTMP_ADAPTER *pAd);

VOID RedRecordCP(UINT8 ucWlanIdx, struct _RTMP_ADAPTER *pAd);

VOID red_recordcp_cr4(UINT8 ucWlanIdx, struct _RTMP_ADAPTER *pAd);

VOID RedEnqueueFail(UINT8 ucWlanIdx, UINT8 ucQidx, struct _RTMP_ADAPTER *pAd);

VOID red_enqueue_fail_cr4(UINT8 ucWlanIdx, UINT8 ucQidx, struct _RTMP_ADAPTER *pAd);

VOID RedSetTargetDelay(INT16 i2TarDelay, struct _RTMP_ADAPTER *pAd);

INT32 RedCalProbB(UINT8 ucWlanIdx, UINT8 ac);

/* VOID RedUpdatePBC(P_WIFI_CMD_T prWiFiCmd); */

VOID RedRecordForceRateFromDriver(RTMP_ADAPTER *pAd, UCHAR wcid);

VOID RedCalForceRateRatio(UINT8 ucWcid, UINT16 u2N9ARCnt, UINT16 u2N9FRCnt, struct _RTMP_ADAPTER *pAd);

VOID RedTxFreeEventNotifyHandler(RTMP_ADAPTER *pAd, UINT8 ucQidx, PNDIS_PACKET pkt);

VOID appShowRedDebugMessage(struct _RTMP_ADAPTER *pAd);

VOID UpdateWlogBit(UINT8 ucWlanIdx, UINT32 u4Mpdutime);

VOID UpdateThreshold(UINT8 ucWlanIdx, RTMP_ADAPTER *pAd);

VOID UpdateAirtimeRatio(UINT8 ucWlanIdx, UINT8 ucAirtimeRatio, UINT8 ATCEnable, struct _RTMP_ADAPTER *pAd);

VOID UpdateTargetDelay(UINT8 ATC_WATF_Enable, struct _RTMP_ADAPTER *pAd);

int uint32_log2(UINT32 n);

INT set_red_enable(PRTMP_ADAPTER pAd, RTMP_STRING *arg);

INT set_red_show_sta(PRTMP_ADAPTER pAd, RTMP_STRING *arg);

INT set_red_target_delay(PRTMP_ADAPTER pAd, RTMP_STRING *arg);

INT set_red_debug_enable(PRTMP_ADAPTER pAd, RTMP_STRING *arg);

INT show_red_info(PRTMP_ADAPTER pAd, RTMP_STRING *arg);

INT set_red_dump_reset(PRTMP_ADAPTER pAd, RTMP_STRING *arg);

#endif /* _RA_AC_Q_MGMT_H_ */



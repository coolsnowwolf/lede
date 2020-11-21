/****************************************************************************
 * Ralink Tech Inc.
 * Taiwan, R.O.C.
 *
 * (c) Copyright 2010, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************/

/****************************************************************************

	Abstract:

	All Dynamic Rate Switch Related Structure & Definition

***************************************************************************/

#ifndef __DRS_EXTR_H__
#define __DRS_EXTR_H__

#define SWIFT_TRAIN_UP 1
#define CCK_TRAIN_UP 1

struct _RTMP_ADAPTER;
struct _MAC_TABLE_ENTRY;

#define TX_RATE_INDEX_1	0
#define TX_RATE_INDEX_2	1
#define TX_RATE_INDEX_3	2
#define TX_RATE_INDEX_4	3
#define TX_RATE_INDEX_5	4
#define TX_RATE_INDEX_678	5
#define TX_COUNTER_OVERFLOW	0xFF

#define RA_PROBING_ABORT_TX_CNT             15
#define RA_PROBING_ABORT_CCK_TX_CNT         3

typedef struct _RTMP_TX_RATE {
	UCHAR mode;
	UCHAR bw;
	UCHAR mcs;
	UCHAR nss;
	UCHAR sgi;
	UCHAR stbc;
}RTMP_TX_RATE;


typedef struct _RTMP_RA_LEGACY_TB
{
	UCHAR   ItemNo;
#ifdef RT_BIG_ENDIAN
	UCHAR	Rsv2:1;
	UCHAR	Mode:3;
	UCHAR	BW:2;
	UCHAR	ShortGI:1;
	UCHAR	STBC:1;
#else
	UCHAR	STBC:1;
	UCHAR	ShortGI:1;
	UCHAR	BW:2;
	UCHAR	Mode:3;
	UCHAR	Rsv2:1;
#endif	
	UCHAR   CurrMCS;
	UCHAR   TrainUp;
	UCHAR   TrainDown;
} RTMP_RA_LEGACY_TB;

#define PTX_RA_LEGACY_ENTRY(pTable, idx)	((RTMP_RA_LEGACY_TB *)&(pTable[(idx+1)*5]))


#ifdef NEW_RATE_ADAPT_SUPPORT
typedef struct  _RTMP_RA_GRP_TB
{
	UCHAR   ItemNo;
#ifdef RT_BIG_ENDIAN
	UCHAR	Rsv2:1;
	UCHAR	Mode:3;	
	UCHAR	BW:2;
	UCHAR	ShortGI:1;
	UCHAR	STBC:1;
#else
	UCHAR	STBC:1;
	UCHAR	ShortGI:1;
	UCHAR	BW:2;
	UCHAR	Mode:3;
	UCHAR	Rsv2:1;
#endif	
	UCHAR   CurrMCS;
	UCHAR   TrainUp;
	UCHAR   TrainDown;
	UCHAR	downMcs;
	UCHAR	upMcs3;
	UCHAR	upMcs2;
	UCHAR	upMcs1;
	UCHAR	dataRate;
} RTMP_RA_GRP_TB;

#ifdef MT_MAC
typedef struct _MT_TX_COUNTER
{
	UINT32 TxCount;
	UINT32 TxFailCount;
	UINT16 Rate1TxCnt;
	UINT16 Rate1FailCnt;
	UCHAR Rate2TxCnt;
	UCHAR Rate3TxCnt;
	UCHAR Rate4TxCnt;
	UCHAR Rate5TxCnt;
	UCHAR RateIndex;
} MT_TX_COUNTER;
#endif /* MT_MAC */

#define PTX_RA_GRP_ENTRY(pTable, idx)	((RTMP_RA_GRP_TB *)&(pTable[(idx+1)*10]))
#endif /* NEW_RATE_ADAPT_SUPPORT */

#define RATE_TABLE_SIZE(pTable)			((pTable)[0])		/* Byte 0 is number of rate indices */
#define RATE_TABLE_INIT_INDEX(pTable)	((pTable)[1])		/* Byte 1 is initial rate index */


/* Values of LastSecTxRateChangeAction */
#define RATE_NO_CHANGE	0		/* No change in rate */
#define RATE_UP			1		/* Trying higher rate or same rate with different BF */
#define RATE_DOWN		2		/* Trying lower rate */

enum RATE_ADAPT_ALG{
	RATE_ALG_LEGACY = 1,
	RATE_ALG_GRP = 2,
	RATE_ALG_AGS = 3,
	RATE_ALG_MAX_NUM
};


typedef enum {
	RAL_OLD_DRS,
	RAL_NEW_DRS,
	RAL_QUICK_DRS
}RA_LOG_TYPE;

#define HIGH_TRAFFIC_THRESHOLD 15
#define RATE_ADAPT_HOLD_TIME 30

enum RATE_ADAPT_TRAFFIC_LOADING {
	RA_INIT_STATE,
	ZERO_TRAFFIC,
	LOW_TRAFFIC,
	HIGH_TRAFFIC,
};

#define RATE_ADAPT_HOLD_TX_RATE(_traffic_loading_old, _traffic_loading_new, _buffer_time) \
	((_traffic_loading_old == HIGH_TRAFFIC || _traffic_loading_old == LOW_TRAFFIC) && (_traffic_loading_new == ZERO_TRAFFIC)) || \
	((_traffic_loading_new == ZERO_TRAFFIC) && (_buffer_time > 0))

extern UCHAR RateSwitchTable11B[];
extern UCHAR RateSwitchTable11G[];
extern UCHAR RateSwitchTable11BG[];

#ifdef DOT11_N_SUPPORT
extern UCHAR RateSwitchTable11BGN1S[];
extern UCHAR RateSwitchTable11BGN2S[];
extern UCHAR RateSwitchTable11BGN2SForABand[];
extern UCHAR RateSwitchTable11N1S[];
extern UCHAR RateSwitchTable11N1SForABand[];
extern UCHAR RateSwitchTable11N2S[];
extern UCHAR RateSwitchTable11N2SForABand[];
extern UCHAR RateSwitchTable11BGN3S[];
extern UCHAR RateSwitchTable11BGN3SForABand[];

#ifdef NEW_RATE_ADAPT_SUPPORT
extern UCHAR RateSwitchTableAdapt11B[];
extern UCHAR RateSwitchTableAdapt11G[];
extern UCHAR RateSwitchTableAdapt11BG[];
extern UCHAR RateSwitchTableAdapt11N1S[];
extern UCHAR RateSwitchTableAdapt11N2S[];
#ifdef MULTI_CLIENT_SUPPORT
extern UCHAR RateSwitchTableAdapt11N1SForMultiClients[];
extern UCHAR RateSwitchTableAdapt11N2SForMultiClients[];
#endif
#ifdef INTERFERENCE_RA_SUPPORT
extern UCHAR RateSwitchTableAdapt11N1SForInterference[];
extern UCHAR RateSwitchTableAdapt11N2SForInterference[];
#endif
extern UCHAR RateSwitchTableAdapt11N3S[];

#define PER_THRD_ADJ			1

/* ADAPT_RATE_TABLE - true if pTable is one of the Adaptive Rate Switch tables */
#ifdef DOT11_VHT_AC
extern UCHAR RateTableVht1S[];
extern UCHAR RateTableVht1S_MCS9[];
extern UCHAR RateTableVht2S[];
extern UCHAR RateTableVht2S_MCS7[];
extern UCHAR RateTableVht2S_BW20[];
extern UCHAR RateTableVht2S_BW40[];

#define ADAPT_RATE_TABLE(pTable)	((pTable)==RateSwitchTableAdapt11B || \
									(pTable)==RateSwitchTableAdapt11G || \
									(pTable)==RateSwitchTableAdapt11BG || \
									(pTable)==RateSwitchTableAdapt11N1S ||\
									(pTable)==RateSwitchTableAdapt11N2S ||\
									(pTable)==RateSwitchTableAdapt11N3S ||\
									(pTable)==RateTableVht1S ||\
									(pTable)==RateTableVht1S_MCS9 ||\
									(pTable)==RateTableVht2S || \
									(pTable)==RateTableVht2S_MCS7 || \
									(pTable)==RateTableVht2S_BW20 ||\
									(pTable)==RateTableVht2S_BW40)
#else
#if defined(INTERFERENCE_RA_SUPPORT) && defined(MULTI_CLIENT_SUPPORT)
#define ADAPT_RATE_TABLE(pTable)	((pTable)==RateSwitchTableAdapt11B || \
									(pTable)==RateSwitchTableAdapt11G || \
									(pTable)==RateSwitchTableAdapt11BG || \
									(pTable)==RateSwitchTableAdapt11N1S || \
									(pTable)==RateSwitchTableAdapt11N2S || \
									(pTable)==RateSwitchTableAdapt11N1SForMultiClients ||\
									(pTable)==RateSwitchTableAdapt11N2SForMultiClients ||\
									(pTable)==RateSwitchTableAdapt11N1SForInterference ||\
									(pTable)==RateSwitchTableAdapt11N2SForInterference ||\
									(pTable)==RateSwitchTableAdapt11N3S)
#elif defined(MULTI_CLIENT_SUPPORT)
#define ADAPT_RATE_TABLE(pTable)	((pTable)==RateSwitchTableAdapt11B || \
									(pTable)==RateSwitchTableAdapt11G || \
									(pTable)==RateSwitchTableAdapt11BG || \
									(pTable)==RateSwitchTableAdapt11N1S || \
									(pTable)==RateSwitchTableAdapt11N2S || \
									(pTable)==RateSwitchTableAdapt11N1SForMultiClients ||\
									(pTable)==RateSwitchTableAdapt11N2SForMultiClients ||\
									(pTable)==RateSwitchTableAdapt11N3S)
#elif defined(INTERFERENCE_RA_SUPPORT)
#define ADAPT_RATE_TABLE(pTable)	((pTable)==RateSwitchTableAdapt11B || \
									(pTable)==RateSwitchTableAdapt11G || \
									(pTable)==RateSwitchTableAdapt11BG || \
									(pTable)==RateSwitchTableAdapt11N1S || \
									(pTable)==RateSwitchTableAdapt11N2S || \
									(pTable)==RateSwitchTableAdapt11N1SForInterference ||\
									(pTable)==RateSwitchTableAdapt11N2SForInterference ||\
									(pTable)==RateSwitchTableAdapt11N3S)
#else
#define ADAPT_RATE_TABLE(pTable)	((pTable)==RateSwitchTableAdapt11B || \
									(pTable)==RateSwitchTableAdapt11G || \
									(pTable)==RateSwitchTableAdapt11BG || \
									(pTable)==RateSwitchTableAdapt11N1S || \
									(pTable)==RateSwitchTableAdapt11N2S || \
									(pTable)==RateSwitchTableAdapt11N3S)
#endif 
#endif /* DOT11_VHT_AC */
#endif /* NEW_RATE_ADAPT_SUPPORT */
#endif /* DOT11_N_SUPPORT */


/* FUNCTION */
VOID MlmeGetSupportedMcs(
	IN struct _RTMP_ADAPTER *pAd,
	IN UCHAR *pTable,
	OUT CHAR mcs[]);

UCHAR MlmeSelectTxRate(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct _MAC_TABLE_ENTRY *pEntry,
	IN CHAR mcs[],
	IN CHAR Rssi,
	IN CHAR RssiOffset);

VOID MlmeClearTxQuality(struct _MAC_TABLE_ENTRY *pEntry);
VOID MlmeClearAllTxQuality(struct _MAC_TABLE_ENTRY *pEntry);
VOID MlmeDecTxQuality(struct _MAC_TABLE_ENTRY *pEntry, UCHAR rateIndex);
USHORT MlmeGetTxQuality(struct _MAC_TABLE_ENTRY *pEntry, UCHAR rateIndex);
VOID MlmeSetTxQuality(
	IN struct _MAC_TABLE_ENTRY *pEntry,
	IN UCHAR rateIndex,
	IN USHORT txQuality);



VOID MlmeOldRateAdapt(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct _MAC_TABLE_ENTRY *pEntry,
	IN UCHAR			CurrRateIdx,
	IN UCHAR			UpRateIdx,
	IN UCHAR			DownRateIdx,
	IN ULONG			TrainUp,
	IN ULONG			TrainDown,
	IN ULONG			TxErrorRatio);

VOID MlmeRestoreLastRate(
	IN struct _MAC_TABLE_ENTRY *pEntry);

VOID MlmeCheckRDG(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct _MAC_TABLE_ENTRY *pEntry);

VOID RTMPSetSupportMCS(
	IN struct _RTMP_ADAPTER *pAd,
	IN UCHAR OpMode,
	IN struct _MAC_TABLE_ENTRY *pEntry,
	IN UCHAR SupRate[],
	IN UCHAR SupRateLen,
	IN UCHAR ExtRate[],
	IN UCHAR ExtRateLen,
#ifdef DOT11_VHT_AC
	IN UCHAR vht_cap_len,
	IN VHT_CAP_IE *vht_cap,
#endif /* DOT11_VHT_AC */
	IN HT_CAPABILITY_IE *pHtCapability,
	IN UCHAR HtCapabilityLen);

#ifdef NEW_RATE_ADAPT_SUPPORT
VOID MlmeSetMcsGroup(struct _RTMP_ADAPTER *pAd, struct _MAC_TABLE_ENTRY *pEnt);

UCHAR MlmeSelectUpRate(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct _MAC_TABLE_ENTRY *pEntry,
	IN RTMP_RA_GRP_TB *pCurrTxRate);

UCHAR MlmeSelectDownRate(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct _MAC_TABLE_ENTRY *pEntry,
	IN UCHAR			CurrRateIdx);

VOID MlmeGetSupportedMcsAdapt(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct _MAC_TABLE_ENTRY *pEntry,
	IN UCHAR	mcs23GI,
	OUT CHAR 	mcs[]);

UCHAR MlmeSelectTxRateAdapt(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct _MAC_TABLE_ENTRY *pEntry,
	IN CHAR		mcs[],
	IN CHAR		Rssi,
	IN CHAR		RssiOffset);

BOOLEAN MlmeRAHybridRule(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct _MAC_TABLE_ENTRY *pEntry,
	IN RTMP_RA_GRP_TB *pCurrTxRate,
	IN ULONG			NewTxOkCount,
	IN ULONG			TxErrorRatio);

VOID MlmeNewRateAdapt(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct _MAC_TABLE_ENTRY *pEntry,
	IN UCHAR			UpRateIdx,
	IN UCHAR			DownRateIdx,
	IN ULONG			TrainUp,
	IN ULONG			TrainDown,
	IN ULONG			TxErrorRatio);

#ifdef MT_MAC
VOID NewRateAdaptMT(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct _MAC_TABLE_ENTRY *pEntry,
	IN UCHAR			UpRateIdx,
	IN UCHAR			DownRateIdx,
	IN UCHAR			TrainUp,
	IN UCHAR			TrainDown,
	IN UCHAR			Rate1ErrorRatio,
	IN UCHAR			HwAggRateIndex);
#endif /* MT_MAC */
#ifdef DBG
INT	Set_PerThrdAdj_Proc(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT	Set_LowTrafficThrd_Proc(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT	Set_TrainUpRule_Proc(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT	Set_TrainUpRuleRSSI_Proc(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT	Set_TrainUpLowThrd_Proc(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT	Set_TrainUpHighThrd_Proc(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_RateTable_Proc(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* DBG */
#ifdef AGS_SUPPORT
INT Show_AGS_Proc(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);


#ifdef CONFIG_AP_SUPPORT
VOID ApMlmeDynamicTxRateSwitchingAGS(
	IN RTMP_ADAPTER *pAd,
	IN INT idx);

VOID ApQuickResponeForRateUpExecAGS(
	IN RTMP_ADAPTER *pAd,
	IN INT idx);
#endif /* CONFIG_AP_SUPPORT */
#endif /* AGS_SUPPORT */

#ifdef MT_MAC
VOID DynamicTxRateSwitchingAdaptMT(struct _RTMP_ADAPTER *pAd, UINT idx);
VOID QuickResponeForRateUpExecAdaptMT(struct _RTMP_ADAPTER *pAd, UINT idx);
VOID DynamicRaInterval(struct _RTMP_ADAPTER *pAd);
#endif /* MT_MAC */

#ifdef CONFIG_AP_SUPPORT
VOID APMlmeDynamicTxRateSwitchingAdapt(struct _RTMP_ADAPTER *pAd, UINT idx);
VOID APQuickResponeForRateUpExecAdapt(struct _RTMP_ADAPTER *pAd, UINT idx);
#endif /* CONFIG_AP_SUPPORT */

#endif /* NEW_RATE_ADAPT_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
VOID APMlmeDynamicTxRateSwitching(
    IN struct _RTMP_ADAPTER *pAd);

VOID APQuickResponeForRateUpExec(
    IN PVOID SystemSpecific1, 
    IN PVOID FunctionContext, 
    IN PVOID SystemSpecific2, 
    IN PVOID SystemSpecific3);

VOID APMlmeSetTxRate(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct _MAC_TABLE_ENTRY *pEntry,
	IN RTMP_RA_LEGACY_TB *pTxRate);
#endif /* CONFIG_AP_SUPPORT */


VOID MlmeRAInit(struct _RTMP_ADAPTER *pAd, struct _MAC_TABLE_ENTRY *pEntry);
VOID MlmeNewTxRate(struct _RTMP_ADAPTER *pAd, struct _MAC_TABLE_ENTRY *pEntry);

VOID MlmeRALog(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct _MAC_TABLE_ENTRY *pEntry,
	IN RA_LOG_TYPE raLogType,
	IN ULONG TxErrorRatio,
	IN ULONG TxTotalCnt);

VOID MlmeSelectTxRateTable(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct _MAC_TABLE_ENTRY *pEntry,
	IN UCHAR **ppTable,
	IN UCHAR *pTableSize,
	IN UCHAR *pInitTxRateIdx);

void ra_swift_train_up_table_reset(struct _MAC_TABLE_ENTRY *pEntry);

/* normal rate switch */
#define RTMP_DRS_ALG_INIT(__pAd, __Alg)										\
	(__pAd)->rateAlg = __Alg;

#ifdef NEW_RATE_ADAPT_SUPPORT
UCHAR ra_get_lowest_rate(struct _RTMP_ADAPTER *pAd, struct _MAC_TABLE_ENTRY *pEntry);
#endif /* NEW_RATE_ADAPT_SUPPORT */

#endif /* __DRS_EXTR_H__ */


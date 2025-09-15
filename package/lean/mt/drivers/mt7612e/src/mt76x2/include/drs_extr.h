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

struct _RTMP_ADAPTER;
struct _MAC_TABLE_ENTRY;


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
extern UCHAR RateSwitchTableAdapt11N1S[];
extern UCHAR RateSwitchTableAdapt11N2S[];
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
extern UCHAR RateTableVht1S_2G_BW20[];
extern UCHAR RateTableVht1S_2G_BW40[];
extern UCHAR RateTableVht2S_2G_BW20[];
extern UCHAR RateTableVht2S_2G_BW40[];

#define ADAPT_RATE_TABLE(pTable)	((pTable)==RateSwitchTableAdapt11N1S ||\
									(pTable)==RateSwitchTableAdapt11N2S ||\
									(pTable)==RateSwitchTableAdapt11N3S ||\
									(pTable)==RateTableVht1S ||\
									(pTable)==RateTableVht1S_MCS9 ||\
									(pTable)==RateTableVht2S || \
									(pTable)==RateTableVht2S_MCS7 || \
									(pTable)==RateTableVht2S_BW20 ||\
									(pTable)==RateTableVht2S_BW40 ||\
									(pTable)==RateTableVht1S_2G_BW20 ||\
									(pTable)==RateTableVht1S_2G_BW40 ||\
									(pTable)==RateTableVht2S_2G_BW20 ||\
									(pTable)==RateTableVht2S_2G_BW40)
#else
#define ADAPT_RATE_TABLE(pTable)	((pTable)==RateSwitchTableAdapt11N1S || \
									(pTable)==RateSwitchTableAdapt11N2S || \
									(pTable)==RateSwitchTableAdapt11N3S)
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
VOID TriggerQuickInitMCSRate(
    IN struct _RTMP_ADAPTER *pAd,
    IN struct _MAC_TABLE_ENTRY	*pEntry,
    IN UINT32 CheckInterval);

BOOLEAN QuickInitMCSRate(
    IN struct _RTMP_ADAPTER *pAd,
    IN struct _MAC_TABLE_ENTRY	*pEntry,
    IN UINT16 PacketSucc,
	IN UINT16 PacketErro);

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

#ifdef NEW_RATE_ADAPT_SUPPORT
INT	Set_PerThrdAdj_Proc(
	IN struct _RTMP_ADAPTER *pAd,
	IN PSTRING arg);

INT	Set_LowTrafficThrd_Proc(
	IN struct _RTMP_ADAPTER *pAd,
	IN PSTRING			arg);

INT	Set_TrainUpRule_Proc(
	IN struct _RTMP_ADAPTER *pAd,
	IN PSTRING			arg);

INT	Set_TrainUpRuleRSSI_Proc(
	IN struct _RTMP_ADAPTER *pAd,
	IN PSTRING			arg);

INT	Set_TrainUpLowThrd_Proc(
	IN struct _RTMP_ADAPTER *pAd,
	IN PSTRING			arg);

INT	Set_TrainUpHighThrd_Proc(
	IN struct _RTMP_ADAPTER *pAd,
	IN PSTRING			arg);

INT Set_RateTable_Proc(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  PSTRING arg);
#endif /*NEW_RATE_ADAPT_SUPPORT*/

#ifdef AGS_SUPPORT
INT Show_AGS_Proc(
    IN  PRTMP_ADAPTER	pAd, 
    IN  PSTRING			arg);

#ifdef CONFIG_STA_SUPPORT
VOID MlmeDynamicTxRateSwitchingAGS(
	IN PRTMP_ADAPTER pAd, 
	IN PMAC_TABLE_ENTRY pEntry, 
	IN PUCHAR pTable, 
	IN UCHAR TableSize, 
	IN PAGS_STATISTICS_INFO pAGSStatisticsInfo,
	IN UCHAR InitTxRateIdx);

VOID StaQuickResponeForRateUpExecAGS(
	IN PRTMP_ADAPTER pAd, 
	IN PMAC_TABLE_ENTRY pEntry, 
	IN PUCHAR pTable, 
	IN UCHAR TableSize, 
	IN PAGS_STATISTICS_INFO pAGSStatisticsInfo,
	IN UCHAR InitTxRateIdx);
#endif /* CONFIG_STA_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
VOID ApMlmeDynamicTxRateSwitchingAGS(
	IN RTMP_ADAPTER *pAd,
	IN INT idx);

VOID ApQuickResponeForRateUpExecAGS(
	IN RTMP_ADAPTER *pAd,
	IN INT idx);
#endif /* CONFIG_AP_SUPPORT */
#endif /* AGS_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
VOID APMlmeDynamicTxRateSwitchingAdapt(struct _RTMP_ADAPTER *pAd, UINT idx);
VOID APQuickResponeForRateUpExecAdapt(struct _RTMP_ADAPTER *pAd, UINT idx);
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
VOID StaQuickResponeForRateUpExecAdapt(
	IN struct _RTMP_ADAPTER *pAd,
	IN ULONG i,
	IN CHAR Rssi);

VOID MlmeDynamicTxRateSwitchingAdapt(
	IN struct _RTMP_ADAPTER *pAd,
	IN ULONG i,
	IN ULONG TxSuccess,
	IN ULONG TxRetransmit,
	IN ULONG TxFailCount);
#endif /* CONFIG_STA_SUPPORT */
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

#ifdef CONFIG_STA_SUPPORT
VOID MlmeDynamicTxRateSwitching(
	IN struct _RTMP_ADAPTER *pAd);

VOID StaQuickResponeForRateUpExec(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3);

VOID MlmeSetTxRate(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct _MAC_TABLE_ENTRY *pEntry,
	IN RTMP_RA_LEGACY_TB *pTxRate);
#endif /* CONFIG_STA_SUPPORT */

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

/* normal rate switch */
#define RTMP_DRS_ALG_INIT(__pAd, __Alg)										\
	(__pAd)->rateAlg = __Alg;

#endif /* __DRS_EXTR_H__ */


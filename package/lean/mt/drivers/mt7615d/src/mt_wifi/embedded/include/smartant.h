/*


*/
#ifndef __SMART_ANT_H__
#define __SMART_ANT_H__

#include "rtmp_type.h"

struct _RTMP_ADAPTER;

typedef enum _SA_TRAIN_COND {
	SA_TRAIN_UPPER_MCS = 0x1,			/* 0x1: trainUpperMCS instead of train CurrentMCS */
	SA_TRAIN_GRADUAL_APPROACH = 0x2,	/* 0x2: train one antenna one time */
} SA_TRAIN_COND;


typedef struct _SA_GPIO_PAIR_ {
	UINT32 regAddr;
	UINT32 bitMask;
} SA_GPIO_PAIR;

#define RTMP_SA_WORK_ON(pAd)	((pAd->smartAntEnable == TRUE) && (pAd->pSAParam))

INT Show_SA_CfgInfo_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Show_SA_DbgInfo_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_SA_McsBound_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT	Set_McsStableCnt_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT	Set_MaxAntennaTry_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

int Set_SA_AGSP_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

int Set_SA_TrainSeq_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *args);

int Set_SA_TrainDelay_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

int Set_SA_AntCand_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

int Set_SA_TrainCond_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_SA_RssiVariance_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_SA_RssiThreshold_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_SA_SkipConfirmStage_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT	Set_SmartAnt_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT	Set_TestPeriod_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT	Set_DbgLogOn_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT	Set_SA_StaticAntPair_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT	set_SA_txNss_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT	Set_SA_Mode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_SA_Station_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_SA_StationCandRule_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT RtmpSATrainInfoUpdate(
	IN struct _RTMP_ADAPTER *pAd,
	IN RTMP_SA_TRAINING_PARAM * pTrainEntry,
	IN RXWI_STRUC * pRxWI,
	IN UCHAR txNss);

BOOLEAN sa_del_train_entry(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR * pMacAddr,
	IN BOOLEAN bForced);

RTMP_SA_TRAINING_PARAM *sa_add_train_entry(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR * pMacAddr,
	IN BOOLEAN bAddByUser);

VOID RtmpSAUpdateRxSignal(
	IN RTMP_ADAPTER *pAd);

INT RtmpSAChkAndGo(
	IN struct _RTMP_ADAPTER *pAd);

int RtmpSAStop(
	IN struct _RTMP_ADAPTER *pAd);

int RtmpSAStart(
	IN struct _RTMP_ADAPTER *pAd);

int RtmpSAInit(
	IN struct _RTMP_ADAPTER *pAd);

int RtmpSAExit(
	IN struct _RTMP_ADAPTER *pAd);

#endif /* __SMART_ANT_H__ // */


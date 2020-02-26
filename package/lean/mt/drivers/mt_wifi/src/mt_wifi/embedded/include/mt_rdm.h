/*
 ***************************************************************************
 * MediaTek Inc.
 *
 * All rights reserved. source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

    Module Name:
    mt_rdm.h//Jelly20150123
*/


#ifndef _MT_RDM_H_
#define _MT_RDM_H_

#ifdef MT_DFS_SUPPORT

#include "rt_config.h"


/* Remember add a RDM compile flag -- Shihwei 20141104 */
/*******************************************************************************
*                              C O N S T A N T S
********************************************************************************
*/

#if defined(MT7615) || defined(MT7622)
#define RDD_STOP     0
#define RDD_START    1
#define RDD_DET_MODE    2
#define RDD_DETSTOP   3
#define CAC_START    4
#define CAC_END    5
#define NORMAL_START 6
#define DISABLE_DFS_CAL 7
#define RDD_PULSEDBG 8
#define RDD_READPULSE 9
#define RDD_RESUME_BF 10
#else
typedef enum {
	RDD_STOP = 0,
	RDD_START,
	RDD_DET_MODE,
	RDD_RADAR_EMULATE,
	RDD_START_TXQ = 20,
	CAC_START = 50,
	CAC_END,
	NORMAL_START,
	DISABLE_DFS_CAL,
	RDD_PULSEDBG,
	RDD_READPULSE,
	RDD_RESUME_BF,
	Dfs_CTRL_NUM,
} DFS_CTRL_TYPE;
#endif


#define HW_RDD0      0
#define HW_RDD1      1

#define RESTRICTION_BAND_LOW	116
#define RESTRICTION_BAND_HIGH	128
#define CHAN_SWITCH_PERIOD 10
#define CHAN_NON_OCCUPANCY 1800
#define CAC_NON_WETHER_BAND 65
#define CAC_WETHER_BAND 605
#define GROUP1_LOWER 36
#define GROUP1_UPPER 48
#define GROUP2_LOWER 52
#define GROUP2_UPPER 64
#define GROUP3_LOWER 100
#define GROUP3_UPPER 112
#define GROUP4_LOWER 116
#define GROUP4_UPPER 128

#define DFS_BW_CH_QUERY_LEVEL1 1
#define DFS_BW_CH_QUERY_LEVEL2 2
#define DFS_AVAILABLE_LIST_BW_NUM 4
#define DFS_AVAILABLE_LIST_CH_NUM 30/*MAX_NUM_OF_CHANNELS*/
#define DFS_BW40_GROUP_NUM	15
#define DFS_BW80_GROUP_NUM	9
#define DFS_BW160_GROUP_NUM	4
#define DFS_BW40_PRIMCH_NUM	2
#define DFS_BW80_PRIMCH_NUM	4
#define DFS_BW160_PRIMCH_NUM	8

#define DFS_MACHINE_BASE	0
#define DFS_BEFORE_SWITCH    0
#define DFS_MAX_STATE		1
#define DFS_CAC_END 0
#define DFS_CHAN_SWITCH_TIMEOUT 1
#define DFS_MAX_MSG			2
#define DFS_FUNC_SIZE (DFS_MAX_STATE * DFS_MAX_MSG)

/* DFS zero wait */
#define ZeroWaitCacApplyDefault      0xFF  /* Apply default setting */
#define BgnScanCacUnit               60000 /* unit is 1ms */
#define DEFAULT_OFF_CHNL_CAC_TIME    (1*BgnScanCacUnit+3000) /* 6*BgnScanCacUnit //6 mins, unit is 1minute for non-weather band channel */
#define WEATHER_OFF_CHNL_CAC_TIME    (10*BgnScanCacUnit+3000) /* 60*BgnScanCacUnit //60 mins, unit is 1minute for weather band channel */
#define DYNAMIC_ZEROWAIT_ON			1
#define DYNAMIC_ZEROWAIT_OFF			0

#define IS_CH_ABAND(_ch)	\
		(_ch > 14)

#define GET_BGND_PARAM(_pAd, _param)		\
	DfsGetBgndParameter(pAd, _param)
enum {
	BW80Group1 = 1, /* CH36~48 */
	BW80Group2,     /* CH52~64 */
	BW80Group3,     /* CH100~112 */
	BW80Group4,     /* CH116~128 */
	BW80Group5,     /* CH132~144 */
	BW80Group6,     /* CH149~161 */
};

enum {
	RXSEL_0 = 0,    /*RxSel = 0*/
	RXSEL_1,	/*RxSel = 1*/
};

enum {
	REG_DEFAULT = 0,    /*No region distinguish*/
	REG_JP_53,	    /*JAP_53*/
	REG_JP_56,          /*JAP_56*/
};

enum {
	RDD_BAND0 = 0,
	RDD_BAND1,
};

enum {
	RDD_DETMODE_OFF = 0,
	RDD_DETMODE_ON,
};

/*******************************************************************************
*                                 M A C R O S
********************************************************************************
*/


/*******************************************************************************
*                             D A T A   T Y P E S
********************************************************************************
*/

typedef struct _DFS_CHANNEL_LIST {
	UCHAR Channel;
	USHORT NonOccupancy;
	UCHAR NOPClrCnt;
	UCHAR SupportBwBitMap;
	USHORT NOPSaveForClear;
	UCHAR NOPSetByBw;
} DFS_CHANNEL_LIST, *PDFS_CHANNEL_LIST;

enum DFS_SET_NEWCH_MODE {
	DFS_SET_NEWCH_DISABLED = 0x0,
	DFS_SET_NEWCH_ENABLED = 0x1,
	DFS_SET_NEWCH_INIT = 0xa,	/* Magic number 0xa stands for initial value */
};

enum {
	DFS_IDLE = 0,
	DFS_INIT_CAC,
	DFS_CAC,          /* Channel avail check state */
	DFS_OFF_CHNL_CAC_TIMEOUT, /* Off channel CAC timeout state */
	DFS_INSERV_MONI, /* In service monitor state */
	DFS_RADAR_DETECT,  /* Radar detected  state */
	DFS_MBSS_CAC,
};

enum {
	INBAND_CH = 0,
	INBAND_BW,
	OUTBAND_CH,
	OUTBAND_BW,
	ORI_INBAND_CH,
	ORI_INBAND_BW,
};

enum ZEROWAIT_ACT_CODE{
	ZERO_WAIT_DFS_ENABLE = 0,/*0*/
	INIT_AVAL_CH_LIST_UPDATE,
	MONITOR_CH_ASSIGN,
	NOP_FORCE_SET,
	PRE_ASSIGN_NEXT_TARGET,
	SHOW_TARGET_INFO,
	QUERY_AVAL_CH_LIST = 20,
	QUERY_NOP_OF_CH_LIST,

};

/*Report to Customer*/
typedef struct _DFS_REPORT_AVALABLE_CH_LIST {
	UCHAR Channel;
	UCHAR RadarHitCnt;
} DFS_REPORT_AVALABLE_CH_LIST, *PDFS_REPORT_AVALABLE_CH_LIST;

typedef struct _NOP_REPORT_CH_LIST {
	UCHAR Channel;
	UCHAR Bw;
	USHORT NonOccupancy;
} NOP_REPORT_CH_LIST, *PNOP_REPORT_CH_LIST;

union dfs_zero_wait_msg {
	struct _aval_channel_list_msg{
		UCHAR Action;
		UCHAR Bw80TotalChNum;
		UCHAR Bw40TotalChNum;
		UCHAR Bw20TotalChNum;
		DFS_REPORT_AVALABLE_CH_LIST Bw80AvalChList[DFS_AVAILABLE_LIST_CH_NUM];
		DFS_REPORT_AVALABLE_CH_LIST Bw40AvalChList[DFS_AVAILABLE_LIST_CH_NUM];
		DFS_REPORT_AVALABLE_CH_LIST Bw20AvalChList[DFS_AVAILABLE_LIST_CH_NUM];
	} aval_channel_list_msg;

	struct _nop_of_channel_list_msg{
		UCHAR Action;
		UCHAR NOPTotalChNum;
		NOP_REPORT_CH_LIST NopReportChList[DFS_AVAILABLE_LIST_CH_NUM];
	} nop_of_channel_list_msg;

	struct _set_monitored_ch_msg{
		UCHAR Action;
		UCHAR SyncNum;
		UCHAR Channel;
		UCHAR Bw;
		UCHAR doCAC;
	} set_monitored_ch_msg;

	struct _zerowait_dfs_ctrl_msg{
		UCHAR Action;
		UCHAR Enable;
	} zerowait_dfs_ctrl_msg;

	struct _nop_force_set_msg{
		UCHAR Action;
		UCHAR Channel;
		UCHAR Bw;
		USHORT NOPTime;
	} nop_force_set_msg;

	struct _assign_next_target{
		UCHAR Channel;
		UCHAR Bw;
		USHORT CacValue;
	} assign_next_target;

	struct _target_ch_show{
		UCHAR mode;
	} target_ch_show;
};

typedef struct _DFS_PARAM {
	UCHAR Band0Ch;/* smaller channel number */
	UCHAR Band1Ch;/* larger channel number */
	UCHAR PrimCh;
	UCHAR DbdcAdditionCh;
	UCHAR PrimBand;
	UCHAR Bw;
	UCHAR RDDurRegion;
	DFS_CHANNEL_LIST DfsChannelList[MAX_NUM_OF_CHANNELS];
	UCHAR ChannelListNum;
	UCHAR AvailableBwChIdx[DFS_AVAILABLE_LIST_BW_NUM][DFS_AVAILABLE_LIST_CH_NUM];
	UCHAR Bw40GroupIdx[DFS_BW40_GROUP_NUM][DFS_BW40_PRIMCH_NUM];
	UCHAR Bw80GroupIdx[DFS_BW80_GROUP_NUM][DFS_BW80_PRIMCH_NUM];
	UCHAR Bw160GroupIdx[DFS_BW160_GROUP_NUM][DFS_BW160_PRIMCH_NUM];
	DFS_REPORT_AVALABLE_CH_LIST Bw80AvailableChList[DFS_AVAILABLE_LIST_CH_NUM];
	DFS_REPORT_AVALABLE_CH_LIST Bw40AvailableChList[DFS_AVAILABLE_LIST_CH_NUM];
	DFS_REPORT_AVALABLE_CH_LIST Bw20AvailableChList[DFS_AVAILABLE_LIST_CH_NUM];
	BOOLEAN bIEEE80211H;
	BOOLEAN DfsChBand[2];
	BOOLEAN RadarDetected[2];
	struct DOT11_H Dot11_H[DBDC_BAND_NUM];
	UCHAR RegTxSettingBW;
	BOOLEAN bDfsCheck;
	BOOLEAN RadarDetectState;
	UCHAR NeedSetNewChList;
	BOOLEAN DisableDfsCal;
	BOOLEAN bNoSwitchCh;
	BOOLEAN bShowPulseInfo;
	BOOLEAN bDBDCMode;
	BOOLEAN bDfsEnable;
	BOOLEAN bNoAvailableCh;
	UCHAR RadarHitIdxRecord;
	UCHAR targetCh;
	UCHAR targetBw;
	USHORT targetCacValue;
	/* DFS zero wait */
	BOOLEAN bZeroWaitSupport;    /* Save the profile setting of DfsZeroWait */
	UCHAR   ZeroWaitDfsState;    /* for DFS zero wait state machine using */
	UCHAR   DfsZeroWaitCacTime;  /* unit is minute and Maximum Off-Channel CAC time is one hour */
	BOOLEAN bZeroWaitCacSecondHandle;
	BOOLEAN bDedicatedZeroWaitSupport;
	UCHAR	OutBandCh;
	UCHAR	OutBandBw;
	UCHAR	OrigInBandCh;
	UCHAR	OrigInBandBw;
	USHORT	DedicatedOutBandCacCount;
	BOOLEAN bOutBandAvailable;
	BOOLEAN bSetInBandCacReStart;
	BOOLEAN bDedicatedZeroWaitDefault;
	BOOLEAN bInitOutBandBranch;
	USHORT	DedicatedOutBandCacTime;
	BOOLEAN RadarHitReport;
	UCHAR	OutBandAvailableCh;
	/* MBSS DFS zero wait */
	BOOLEAN bInitMbssZeroWait;

	STATE_MACHINE_FUNC		DfsStateFunc[DFS_FUNC_SIZE];
	STATE_MACHINE			DfsStatMachine;
} DFS_PARAM, *PDFS_PARAM;

/*******************************************************************************
*                    E X T E R N A L   R E F E R E N C E S
********************************************************************************
*/


/*******************************************************************************
*                   F U N C T I O N   D E C L A R A T I O N S
********************************************************************************
*/

INT ZeroWaitDfsCmdHandler(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT * wrq
	);

INT ZeroWaitDfsQueryCmdHandler(
	RTMP_ADAPTER *pAd,
	RTMP_IOCTL_INPUT_STRUCT * wrq
	);

INT Set_RadarDetectStart_Proc(
	RTMP_ADAPTER * pAd, RTMP_STRING *arg);

INT Set_RadarDetectStop_Proc(
	RTMP_ADAPTER * pAd, RTMP_STRING *arg);

INT Set_ByPassCac_Proc(
	RTMP_ADAPTER * pAd,
	RTMP_STRING *arg);

INT Set_RDDReport_Proc(
	RTMP_ADAPTER * pAd,
	RTMP_STRING *arg);

UCHAR DfsPrimToCent(
	UCHAR Channel, UCHAR Bw);

UCHAR DfsGetBgndParameter(
	IN PRTMP_ADAPTER pAd, UCHAR QueryParam);

VOID DfsGetSysParameters(
	IN PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev,
	UCHAR vht_cent2,
	UCHAR phy_bw);

VOID DfsParamInit(/* finish */
	IN PRTMP_ADAPTER	pAd);

VOID DfsStateMachineInit(
	IN RTMP_ADAPTER * pAd,
	IN STATE_MACHINE *Sm,
	OUT STATE_MACHINE_FUNC Trans[]);

INT Set_DfsChannelShow_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg);

INT Set_DfsBwShow_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg);

INT Set_DfsRDModeShow_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg);

INT Set_DfsRDDRegionShow_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg);

INT Show_DfsNonOccupancy_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg);

INT Set_DfsNOP_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg);

/* DFS Zero Wait */
INT Set_DfsZeroWaitCacTime_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_DedicatedBwCh_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_DfsZeroWaitDynamicCtrl_Proc(
	RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_DfsZeroWaitNOP_Proc(
		RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_DfsTargetCh_Proc(
		RTMP_ADAPTER *pAd, RTMP_STRING *arg);

VOID DfsDedicatedExclude(IN PRTMP_ADAPTER pAd);

VOID DfsSetCalibration(
	IN PRTMP_ADAPTER pAd, UINT_32 DisableDfsCal);

VOID DfsSetZeroWaitCacSecond(
	IN PRTMP_ADAPTER pAd);

BOOLEAN DfsBypassRadarStateCheck(
	struct wifi_dev *wdev);

BOOLEAN DfsRadarChannelCheck(
	PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev,
	UCHAR vht_cent2,
	UCHAR phy_bw);

VOID DfsSetNewChInit(
	IN PRTMP_ADAPTER pAd);

VOID DfsCacEndUpdate(
	RTMP_ADAPTER *pAd,
	MLME_QUEUE_ELEM *Elem);

NTSTATUS DfsChannelSwitchTimeoutAction(
	PRTMP_ADAPTER pAd, PCmdQElmt CMDQelmt);

NTSTATUS DfsAPRestart(
	PRTMP_ADAPTER pAd, PCmdQElmt CMDQelmt);

VOID DfsCacNormalStart(
	PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev,
	UCHAR CompareMode);

BOOLEAN DfsCacRestrictBand(
	IN PRTMP_ADAPTER pAd, IN UCHAR Bw, IN UCHAR Ch, IN UCHAR SecCh);

VOID DfsBuildChannelList(
	IN PRTMP_ADAPTER pAd, IN struct wifi_dev *wdev);

VOID DfsBuildChannelGroupByBw(
    IN PRTMP_ADAPTER pAd);

BOOLEAN DfsCheckBwGroupAllAvailable(
    UCHAR ChechChIdx, UCHAR Bw, IN PDFS_PARAM pDfsParam);

BOOLEAN DfsSwitchCheck(/* finish */
	IN PRTMP_ADAPTER	pAd,
	UCHAR	Channel,
	UCHAR bandIdx);

BOOLEAN DfsStopWifiCheck(
	IN PRTMP_ADAPTER	pAd);

VOID DfsNonOccupancyUpdate(
	IN PRTMP_ADAPTER pAd);

VOID DfsNonOccupancyCountDown(/*RemainingTimeForUse --, finish*/
	IN PRTMP_ADAPTER pAd);

VOID WrapDfsRddReportHandle(/*handle the event of EXT_EVENT_ID_RDD_REPORT*/
	IN PRTMP_ADAPTER pAd, UCHAR ucRddIdx);

BOOLEAN DfsRddReportHandle(/*handle the event of EXT_EVENT_ID_RDD_REPORT*/
	IN PDFS_PARAM pDfsParam, UCHAR ucRddIdx);

VOID WrapDfsSetNonOccupancy(/*Set Channel non-occupancy time, finish */
	IN PRTMP_ADAPTER pAd, UCHAR bandIdx);

VOID DfsSetNonOccupancy(/*Set Channel non-occupancy time, finish*/
	IN PRTMP_ADAPTER pAd, IN PDFS_PARAM pDfsParam, UCHAR bandIdx);

VOID WrapDfsSelectChannel(/*Select new channel, finish*/
	IN PRTMP_ADAPTER pAd, UCHAR bandIdx);

VOID DfsSelectChannel(/*Select new channel, finish*/
	IN PRTMP_ADAPTER pAd, PDFS_PARAM pDfsParam);

UCHAR WrapDfsRandomSelectChannel(/*Select new channel using random selection, finish*/
	IN PRTMP_ADAPTER pAd, BOOLEAN bSkipDfsCh, UCHAR avoidCh);

UCHAR DfsRandomSelectChannel(/*Select new channel using random selection, finish*/
	IN PRTMP_ADAPTER pAd, PDFS_PARAM pDfsParam, BOOLEAN bSkipDfsCh, UCHAR avoidCh);

USHORT DfsBwChQueryByDefault(/*Query current available BW & Channel list or select default*/
	IN PRTMP_ADAPTER pAd, UCHAR Bw, PDFS_PARAM pDfsParam, UCHAR level, BOOLEAN bDefaultSelect, BOOLEAN SkipNonDfsCh);

VOID DfsBwChQueryAllList(/*Query current All available BW & Channel list*/
	IN PRTMP_ADAPTER pAd, UCHAR Bw, PDFS_PARAM pDfsParam, BOOLEAN SkipWorkingCh);

BOOLEAN DfsDedicatedCheckChBwValid(
	IN PRTMP_ADAPTER pAd, UCHAR Channel, UCHAR Bw);

VOID DfsAdjustBwSetting(
	struct wifi_dev *wdev, UCHAR CurrentBw, UCHAR NewBw);

VOID WrapDfsRadarDetectStart(/*Start Radar Detection or not, finish*/
	IN PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev);

VOID DfsRadarDetectStart(/*Start Radar Detection or not, finish*/
	IN PRTMP_ADAPTER pAd,
	PDFS_PARAM pDfsParam,
	struct wifi_dev *wdev);

VOID WrapDfsRadarDetectStop(/*Start Radar Detection or not*/
	IN PRTMP_ADAPTER pAd);

VOID DfsRadarDetectStop(/*Start Radar Detection or not, finish*/
	IN PRTMP_ADAPTER pAd, PDFS_PARAM pDfsParam);

VOID DfsDedicatedOutBandRDDStart(
	IN PRTMP_ADAPTER pAd);

VOID DfsDedicatedOutBandRDDRunning(
	IN PRTMP_ADAPTER pAd);

VOID DfsDedicatedOutBandRDDStop(
	IN PRTMP_ADAPTER pAd);

BOOLEAN DfsIsRadarHitReport(
	IN PRTMP_ADAPTER pAd);

VOID DfsRadarHitReportReset(
	IN PRTMP_ADAPTER pAd);

BOOLEAN DfsIsTargetChAvailable(
	IN PRTMP_ADAPTER pAd);

VOID DfsReportCollision(
	IN PRTMP_ADAPTER pAd);

BOOLEAN DfsIsOutBandAvailable(
	IN PRTMP_ADAPTER pAd);

VOID DfsOutBandCacReset(
	IN PRTMP_ADAPTER pAd);

VOID DfsSetCacRemainingTime(
	IN PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev);

VOID DfsOutBandCacCountUpdate(
	IN PRTMP_ADAPTER pAd);

VOID DfsDedicatedExamineSetNewCh(
	IN struct _RTMP_ADAPTER *pAd, UCHAR Channel);

UCHAR DfsGetCentCh(IN PRTMP_ADAPTER pAd, IN UCHAR Channel, IN UCHAR bw, IN struct wifi_dev *wdev);

INT mtRddControl(
	IN struct _RTMP_ADAPTER *pAd,
	IN UCHAR ucRddCtrl,
	IN UCHAR ucRddIdex,
	IN UCHAR ucRddInSel,
	IN UCHAR ucSetVal);

#ifdef BACKGROUND_SCAN_SUPPORT
/* Mbss Zero Wait */
BOOLEAN MbssZeroWaitStopValidate(PRTMP_ADAPTER pAd, UCHAR MbssCh, INT MbssIdx);
VOID ZeroWaitUpdateForMbss(PRTMP_ADAPTER pAd, BOOLEAN bZeroWaitStop, UCHAR MbssCh, INT MbssIdx);
VOID DfsDedicatedScanStart(IN PRTMP_ADAPTER pAd);
VOID DfsInitDedicatedScanStart(IN PRTMP_ADAPTER pAd);
VOID DfsSetInitDediatedScanStart(IN PRTMP_ADAPTER pAd);
VOID DfsDedicatedInBandSetChannel(IN PRTMP_ADAPTER pAd, UCHAR Channel, UCHAR Bw, BOOLEAN doCAC);
VOID DfsDedicatedOutBandSetChannel(IN PRTMP_ADAPTER pAd, UCHAR Channel, UCHAR Bw);
VOID DfsDedicatedDynamicCtrl(IN PRTMP_ADAPTER pAd, UINT_32 DfsDedicatedOnOff);
#endif /* BACKGROUND_SCAN_SUPPORT */

INT Set_ModifyChannelList_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Show_available_BwCh_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Show_NOP_Of_ChList(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT Show_Target_Ch_Info(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

VOID ZeroWait_DFS_Initialize_Candidate_List(
	IN struct _RTMP_ADAPTER *pAd,
	IN UCHAR Bw80Num, IN PDFS_REPORT_AVALABLE_CH_LIST pBw80AvailableChList,
	IN UCHAR Bw40Num, IN PDFS_REPORT_AVALABLE_CH_LIST pBw40AvailableChList,
	IN UCHAR Bw20Num, IN PDFS_REPORT_AVALABLE_CH_LIST pBw20AvailableChList);

VOID DfsProvideAvailableChList(
	IN PRTMP_ADAPTER pAd);

VOID DfsProvideNopOfChList(
	IN PRTMP_ADAPTER pAd, union dfs_zero_wait_msg *msg);

VOID ZeroWait_DFS_set_NOP_to_Channel_List(
	IN PRTMP_ADAPTER pAd, IN UCHAR Channel, UCHAR Bw, USHORT NOPTime);

VOID ZeroWait_DFS_Pre_Assign_Next_Target_Channel(
	IN PRTMP_ADAPTER pAd, IN UCHAR Channel, IN UCHAR Bw, IN USHORT CacValue);

VOID ZeroWait_DFS_Next_Target_Show(
	IN PRTMP_ADAPTER pAd, IN UCHAR mode);

VOID ZeroWait_DFS_collision_report(
	IN PRTMP_ADAPTER pAd, IN UCHAR SynNum, IN UCHAR Channel, UCHAR Bw);

VOID DfsZeroHandOffRecovery(IN struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
#endif /*MT_DFS_SUPPORT*/
#endif /*_MT_RDM_H_ */

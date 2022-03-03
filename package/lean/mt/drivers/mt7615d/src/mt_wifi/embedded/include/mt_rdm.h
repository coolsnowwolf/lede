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
#ifdef	CONFIG_RCSA_SUPPORT
#define RDD_DETECT_INFO	11
#define RDD_ALTX_CTRL	12
#endif

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
#define HW_RDD_NUM   2

#define RESTRICTION_BAND_LOW	116
#define RESTRICTION_BAND_HIGH	128
#define CHAN_SWITCH_PERIOD 10
#define CHAN_NON_OCCUPANCY 1800

#if defined(OFFCHANNEL_SCAN_FEATURE) && defined(ONDEMAND_DFS)
#define CHAN_NON_OCCUPANCY_FREE 0
#endif
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

typedef enum _ENUM_AP_DFS_STATES {
	DFS_CAC_END = 0,
	DFS_CHAN_SWITCH_TIMEOUT,
#ifdef ONDEMAND_DFS
	DFS_ONDEMAND_CAC_FINISH,
#endif
#ifdef DFS_VENDOR10_CUSTOM_FEATURE
	DFS_V10_W56_APDOWN_FINISH,
	DFS_V10_W56_APDOWN_ENBL,
	DFS_V10_ACS_CSA_UPDATE,
#endif
	DFS_MAX_MSG
} ENUM_AP_DFS_STATES;

#define DFS_FUNC_SIZE (DFS_MAX_STATE * DFS_MAX_MSG)

#ifdef ONDEMAND_DFS
#define ONDEMAND_2x2MODE    0
#define ONDEMAND_4x4MODE    1

#define IS_ONDEMAND_DFS_MODE_4x4(_pAd)	\
		(_pAd->CommonCfg.DfsParameter.bOnDemandDFSMode == TRUE)
#define SET_ONDEMAND_DFS_MODE(_pAd, _param)		\
		(_pAd->CommonCfg.DfsParameter.bOnDemandDFSMode = _param)
#endif

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


#ifdef DFS_VENDOR10_CUSTOM_FEATURE
#define V10_W52_SIZE           4
#define V10_W53_SIZE           4
#define V10_W56_VHT80_A_SIZE   4
#define V10_W56_VHT80_B_SIZE   4
#define V10_W56_VHT80_SIZE     (V10_W56_VHT80_A_SIZE + V10_W56_VHT80_A_SIZE)
#define V10_W56_VHT20_SIZE     3
#define V10_W56_SIZE          11
#define V10_TOTAL_CHANNEL_COUNT (V10_W52_SIZE + V10_W53_SIZE \
				+ V10_W56_VHT80_A_SIZE + V10_W56_VHT80_B_SIZE + V10_W56_VHT20_SIZE)
#define V10_5G_TOTAL_CHNL_COUNT (V10_TOTAL_CHANNEL_COUNT + V10_LAST_SIZE)

typedef enum _V10_NEC_GRP_LIST {
	W52 = 0,    /* CH36~48 */
	W53,        /* CH52~64 */
	W56_UA,     /* CH100~112 */
	W56_UB,     /* CH116~128 */
	W56_UC,     /* CH132~140 */
	W56_UAB,    /* VHT80 Ch 100 ~ 128*/
	W56,        /* All W56 */
	NA_GRP
} V10_NEC_GRP_LIST;

#define V10_LAST_SIZE 5

#define GROUP5_LOWER 132

#define GROUP6_LOWER 149

#define V10_WEIGH_FACTOR_W53 3
#define V10_WEIGH_FACTOR_W52 2
#define V10_WEIGH_FACTOR_W56 1

#define V10_W56_APDOWN_TIME 1805
#define V10_BGND_SCAN_TIME 20
#define V10_NORMAL_SCAN_TIME 200

#define IS_V10_W56_VHT80_SWITCHED(_pAd) \
		(_pAd->CommonCfg.DfsParameter.bV10W56SwitchVHT80 == TRUE)
#define SET_V10_W56_VHT80_SWITCH(_pAd, switch) \
			(_pAd->CommonCfg.DfsParameter.bV10W56SwitchVHT80 = switch)

#define GET_V10_OFF_CHNL_TIME(_pAd) \
	(_pAd->CommonCfg.DfsParameter.gV10OffChnlWaitTime)
#define SET_V10_OFF_CHNL_TIME(_pAd, waitTime) \
		(_pAd->CommonCfg.DfsParameter.gV10OffChnlWaitTime = waitTime)

#define IS_V10_W56_GRP_VALID(_pAd) \
	 (_pAd->CommonCfg.DfsParameter.bV10W56GrpValid == TRUE)
#define SET_V10_W56_GRP_VALID(_pAd, valid) \
	(_pAd->CommonCfg.DfsParameter.bV10W56GrpValid = valid)
#endif


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

#ifdef ONDEMAND_DFS
typedef struct _OD_CHANNEL_LIST {
	UCHAR Channel;
	BOOLEAN isConsumed;
} OD_CHANNEL_LIST, *POD_CHANNEL_LIST;
#endif

#ifdef DFS_VENDOR10_CUSTOM_FEATURE
typedef struct _V10_CHANNEL_LIST {
	UINT_32 BusyTime;
	UCHAR Channel;
	BOOLEAN isConsumed;
} V10_CHANNEL_LIST, *PV10_CHANNEL_LIST;
#endif

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

#ifdef CUSTOMISE_RDD_THRESHOLD_SUPPORT
typedef struct _DFS_PULSE_THRESHOLD_PARAM {
	UINT32 u4PulseWidthMax;			/* unit us */
	INT32 i4PulsePwrMax;			/* unit dbm */
	INT32 i4PulsePwrMin;			/* unit dbm */
	UINT32 u4PRI_MIN_STGR;			/* unit us */
	UINT32 u4PRI_MAX_STGR;			/* unit us */
	UINT32 u4PRI_MIN_CR;			/* unit us */
	UINT32 u4PRI_MAX_CR;			/* unit us */
} DFS_PULSE_THRESHOLD_PARAM, *PDFS_PULSE_THRESHOLD_PARAM;

typedef struct _DFS_RADAR_THRESHOLD_PARAM {
	DFS_PULSE_THRESHOLD_PARAM rPulseThresholdParam;
	BOOLEAN afgSupportedRT[RT_NUM];
	SW_RADAR_TYPE_T arRadarType[RT_NUM];
} DFS_RADAR_THRESHOLD_PARAM, *PDFS_RADAR_THRESHOLD_PARAM;
#endif /* CUSTOMISE_RDD_THRESHOLD_SUPPORT */

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
#ifdef ONDEMAND_DFS
	BOOLEAN bOnDemandZeroWaitSupport;		/* OnDemand DFS Support */
	BOOLEAN bOnDemandDFSMode;               /* OnDemand DFS Mode: FALSE= 2x2, TRUE= 4x4*/
	BOOLEAN bOnDemandChannelListValid;	/* Is ACS Channel List Valid */
	UCHAR   MaxGroupCount;					/* Max Group Count from ACS */
	POD_CHANNEL_LIST OnDemandChannelList;   /* ACS CHannel List */
#endif
#ifdef DFS_VENDOR10_CUSTOM_FEATURE
	ULONG   gV10OffChnlWaitTime;
	ULONG	gV10W56TrgrApDownTime;
	BOOLEAN bDFSV10Support; /* NEC DFS Support */
	BOOLEAN	bV10ChannelListValid;
	BOOLEAN	bV10BootACSValid;
	BOOLEAN bV10W56GrpValid;
	BOOLEAN bV10W56SwitchVHT80;
	BOOLEAN bV10W56APDownEnbl;
	BOOLEAN bV10APBcnUpdateEnbl;
	BOOLEAN bV10APInterfaceDownEnbl;
	UCHAR   GroupCount; /* Max Group Count from ACS */
	V10_CHANNEL_LIST DfsV10SortedACSList[V10_TOTAL_CHANNEL_COUNT];
#endif
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

#ifdef CUSTOMISE_RDD_THRESHOLD_SUPPORT
	/*Threshold params*/
	BOOLEAN fgHwRDDLogEnable;
	BOOLEAN fgSwRDDLogEnable;
	BOOLEAN fgSwRDDLogCond;
	UINT16 u2FCC_LPN_MIN;
	BOOLEAN fgRDRegionConfigured;
	DFS_RADAR_THRESHOLD_PARAM rRadarThresholdParam;
#endif /* CUSTOMISE_RDD_THRESHOLD_SUPPORT */

#ifdef RDM_FALSE_ALARM_DEBUG_SUPPORT
	BOOLEAN fgRadarEmulate;
#endif /*RDM_FALSE_ALARM_DEBUG_SUPPORT */

#ifdef CONFIG_RCSA_SUPPORT
	BOOLEAN	bRCSAEn;
	BOOLEAN fSendRCSA;
	BOOLEAN fUseCsaCfg;
	BOOLEAN	fCheckRcsaTxDone;
	UCHAR	ChSwMode;
#endif

	STATE_MACHINE_FUNC		DfsStateFunc[DFS_FUNC_SIZE];
	STATE_MACHINE			DfsStatMachine;
} DFS_PARAM, *PDFS_PARAM;

#ifdef MT_DFS_SUPPORT
typedef int (*_k_ARC_ZeroWait_DFS_CAC_Time_Meet_report_callback_fun_type)(UCHAR SyncNum, UCHAR Bw, UCHAR monitored_Ch);
#endif /* MT_DFS_SUPPORT */

/*******************************************************************************
*                    E X T E R N A L   R E F E R E N C E S
********************************************************************************
*/

#ifdef CUSTOMISE_RDD_THRESHOLD_SUPPORT
extern DFS_RADAR_THRESHOLD_PARAM g_arRadarThresholdParam[4];
#endif /* CUSTOMISE_RDD_THRESHOLD_SUPPORT */


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

#ifdef CUSTOMISE_RDD_THRESHOLD_SUPPORT
VOID DfsThresholdParamInit(
	IN PRTMP_ADAPTER	pAd);

INT Set_DfsDefaultRDDThresholdParam(
	IN PRTMP_ADAPTER pAd);
#endif /* CUSTOMISE_RDD_THRESHOLD_SUPPORT */

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

VOID DfsCacEndLoadDCOCData(
	IN PRTMP_ADAPTER pAd);

#ifdef ONDEMAND_DFS
VOID DfsOutBandCacPass(
	RTMP_ADAPTER *pAd,
	MLME_QUEUE_ELEM *Elem);
#endif

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

#ifdef ONDEMAND_DFS
USHORT DfsOnDemandSelectBestChannel(/*Select the Channel from Rank List by Bgnd Scan*/
	IN PRTMP_ADAPTER pAd, BOOLEAN bSkipDfsCh);

BOOLEAN CheckNonOccupancyOnDemandChannel(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR channel);

#ifdef OFFCHANNEL_SCAN_FEATURE
VOID DfsNonOccupancyCmpltnEvent(
	IN PRTMP_ADAPTER pAd,
	IN UINT_8 chIndex,
	IN BOOLEAN isNOPSaveForClear);
#endif
#endif

#ifdef DFS_VENDOR10_CUSTOM_FEATURE
USHORT DfsV10SelectBestChannel(/*Select the Channel from Rank List by ACS*/
	IN PRTMP_ADAPTER pAd,
	IN UCHAR oldChannel);

UCHAR DfsV10CheckChnlGrp(
	IN UCHAR Channel);

BOOLEAN DfsV10CheckW56Grp(
	IN UCHAR channel);

VOID DfsV10AddWeighingFactor(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *pwdev);

BOOLEAN DfsV10CheckGrpChnlLeft(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR		 chGrp,
	IN UCHAR		 grpWidth);

UINT_8 DfsV10W56FindMaxNopDuration(
	IN PRTMP_ADAPTER pAd);

UINT_8 DfsV10FindNonNopChannel(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR		 chGrp,
	IN UCHAR		 grpWidth);

BOOLEAN DfsV10W56APDownStart(
	IN PRTMP_ADAPTER pAd,
	IN PAUTO_CH_CTRL pAutoChCtrl,
	IN ULONG	     V10W56TrgrApDownTime);

VOID DfsV10W56APDownTimeCountDown(/*RemainingTimeForUse --*/
	IN PRTMP_ADAPTER pAd);

VOID DfsV10W56APDownPass(
	RTMP_ADAPTER *pAd,
	MLME_QUEUE_ELEM *Elem);

VOID DfsV10W56APDownEnbl(
	RTMP_ADAPTER *pAd,
	MLME_QUEUE_ELEM *Elem);

VOID DfsV10APBcnUpdate(
	RTMP_ADAPTER *pAd,
	MLME_QUEUE_ELEM *Elem);
#endif

VOID WrapDfsRddReportHandle(/*handle the event of EXT_EVENT_ID_RDD_REPORT*/
	IN PRTMP_ADAPTER pAd, UCHAR ucRddIdx);

BOOLEAN DfsRddReportHandle(/*handle the event of EXT_EVENT_ID_RDD_REPORT*/
	IN PRTMP_ADAPTER pAd, PDFS_PARAM pDfsParam, UCHAR ucRddIdx, UCHAR bandIdx);

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

#ifdef ONDEMAND_DFS
VOID DfsOnDemandInBandRDDStart(
	IN PRTMP_ADAPTER pAd);

UCHAR DfsOnDemandDynamicChannelUpdate(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev,
	IN UCHAR Channel);
#endif

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

#ifdef RDM_FALSE_ALARM_DEBUG_SUPPORT
VOID UpdateRadarInfo(
	P_EXT_EVENT_RDD_REPORT_T prRadarReport);

VOID DumpRadarHwPulsesInfo(
	IN PRTMP_ADAPTER pAd,
	IN P_EXT_EVENT_RDD_REPORT_T prRadarReport);

VOID DumpRadarSwPulsesInfo(
	IN PRTMP_ADAPTER pAd,
	IN P_EXT_EVENT_RDD_REPORT_T prRadarReport);

INT Show_DFS_Debug_Proc(
	PRTMP_ADAPTER pAd,
	RTMP_STRING *arg);
#endif /* RDM_FALSE_ALARM_DEBUG_SUPPORT */
#endif /*MT_DFS_SUPPORT*/
#endif /*_MT_RDM_H_ */

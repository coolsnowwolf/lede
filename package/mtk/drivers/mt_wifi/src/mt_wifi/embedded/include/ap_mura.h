/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

    Module Name:
    ap_mura.h

    Abstract:
    Miniport generic portion header file

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
*/
#ifndef __AP_MUMIMO_RA_H__
#define __AP_MUMIMO_RA_H__

#define MURA_COLOR_RED     "\x1b[31m"
#define MURA_COLOR_GREEN   "\x1b[32m"
#define MURA_COLOR_YELLOW  "\x1b[33m"
#define MURA_COLOR_BLUE    "\x1b[34m"
#define MURA_COLOR_MAGENTA "\x1b[35m"
#define MURA_COLOR_CYAN    "\x1b[36m"
#define MURA_COLOR_RESET   "\x1b[0m"

#define MAX_MURA_NUM            8
#define MAX_MURA_WAIT_NUM       32
#define MAX_MURA_GRP            68
#define MU_2U_NUM               28
#define MU_3U_NUM               35

#define MURA_STATE              1
#define MURA_GROUP_STAT         2
#define MURA_HWFB_STAT          3

enum {
	/* debug commands */
	MURA_PERIODIC_SND = 0,
	MURA_ALGORITHM_STAT = 1,
	MURA_ALGORITHM_GROUP_STAT,
	MURA_ALGORITHM_HWFB_STAT,
	MURA_FIXED_RATE_ALGORITHM,
	MURA_FIXED_GROUP_RATE_ALGORITHM,
	MURA_TEST_ALGORITHM,
	MURA_SOUNDING_PERIOD,
	MURA_PLATFORM_TYPE,
	MURA_DISABLE_CN3_CN4,
	MURA_PFID_STAT,
	MURA_MOBILITY_CTRL,
	MURA_MOBILITY_INTERVAL_CTRL,
	MURA_MOBILITY_SNR_CTRL,
	MURA_MOBILITY_THRESHOLD_CTRL,
	MURA_MOBILITY_SOUNDING_INTERVAL_COUNT,
	MURA_MOBILITY_MODE_CTRL,
	MURA_MOBILITY_LOG_CTRL,
	MURA_MOBILITY_TEST_CTRL,
	MURA_MAX_COMMAND,
};

enum {
	MURA_EVENT_ALGORITHM_STAT = 0,
	MURA_EVENT_ALGORITHM_GROUP_STAT,
	MURA_EVENT_ALGORITHM_HWFB_STAT,
	MURA_EVENT_PFID_STAT,
	MURA_MAX_EVENT
};

typedef struct _LINK_ENTRY_T {
	struct _LINK_ENTRY_T *prNext, *prPrev;
} LINK_ENTRY_T, *P_LINK_ENTRY_T;

typedef struct _MURA_CN_ENTRY_T {
	LINK_ENTRY_T rLinkEntry;

	UINT_8  ucBBPCalcState;
#ifdef RT_BIG_ENDIAN
	UINT_8	ucMU3RateCalcState:2;
	UINT_8	ucMU2RateCalcState:2;
	UINT_8	ucMU1RateCalcState:2;
	UINT_8	ucMU0RateCalcState:2;
#else
	UINT_8  ucMU0RateCalcState:2;
	UINT_8  ucMU1RateCalcState:2;
	UINT_8  ucMU2RateCalcState:2;
	UINT_8  ucMU3RateCalcState:2;
#endif
#ifdef RT_BIG_ENDIAN
	UINT_16 u2Reserve1:1;
	UINT_16 u2U2PreSSN:1;
	UINT_16 u2U1PreSSN:1;
	UINT_16 u2U0PreSSN:1;
	UINT_16 u2U2PreRate:4;
	UINT_16 u2U1PreRate:4;
	UINT_16 u2U0PreRate:4;
#else
	UINT_16 u2U0PreRate:4;
	UINT_16 u2U1PreRate:4;
	UINT_16 u2U2PreRate:4;
	UINT_16 u2U0PreSSN:1;
	UINT_16 u2U1PreSSN:1;
	UINT_16 u2U2PreSSN:1;
	UINT_16 u2Reserve1:1;
#endif
#ifdef RT_BIG_ENDIAN
	UINT_16 u2U3Rate:4;
	UINT_16 u2U2Rate:4;
	UINT_16 u2U1Rate:4;
	UINT_16 u2U0Rate:4;
#else
	UINT_16 u2U0Rate:4;
	UINT_16 u2U1Rate:4;
	UINT_16 u2U2Rate:4;
	UINT_16 u2U3Rate:4;
#endif
#ifdef RT_BIG_ENDIAN
	UINT_16 u2U3AutoRate:4;
	UINT_16 u2U2AutoRate:4;
	UINT_16 u2U1AutoRate:4;
	UINT_16 u2U0AutoRate:4;
#else
	UINT_16 u2U0AutoRate:4;
	UINT_16 u2U1AutoRate:4;
	UINT_16 u2U2AutoRate:4;
	UINT_16 u2U3AutoRate:4;
#endif
#ifdef RT_BIG_ENDIAN
	UINT_16 u2U3LastRate:4;
	UINT_16 u2U2LastRate:4;
	UINT_16 u2U1LastRate:4;
	UINT_16 u2U0LastRate:4;
#else
	UINT_16 u2U0LastRate:4;
	UINT_16 u2U1LastRate:4;
	UINT_16 u2U2LastRate:4;
	UINT_16 u2U3LastRate:4;
#endif
#ifdef RT_BIG_ENDIAN
	UINT_16 u2U3DeltaMCS:4;
	UINT_16 u2U2DeltaMCS:4;
	UINT_16 u2U1DeltaMCS:4;
	UINT_16 u2U0DeltaMCS:4;
#else
	UINT_16 u2U0DeltaMCS:4;
	UINT_16 u2U1DeltaMCS:4;
	UINT_16 u2U2DeltaMCS:4;
	UINT_16 u2U3DeltaMCS:4;
#endif
#ifdef RT_BIG_ENDIAN
    UINT_8  ucU3LastSSNReverse:3;
    UINT_8  ucGroupInit:1;
    UINT_8  ucU3LastSSN:1;
    UINT_8  ucU2LastSSN:1;
	UINT_8  ucU1LastSSN:1;
    UINT_8  ucU0LastSSN:1;
#else
	UINT_8  ucU0LastSSN:1;
	UINT_8  ucU1LastSSN:1;
	UINT_8  ucU2LastSSN:1;
	UINT_8  ucU3LastSSN:1;
	UINT_8  ucGroupInit:1;
	UINT_8  ucU3LastSSNReverse:3;
#endif
#ifdef RT_BIG_ENDIAN
	UINT_8	ucUserNum:2;
	UINT_8	ucGPBW:2;
	UINT_8	ucU3SSN:1;
	UINT_8	ucU2SSN:1;
	UINT_8	ucU1SSN:1;
	UINT_8	ucU0SSN:1;
#else
	UINT_8  ucU0SSN:1;
	UINT_8  ucU1SSN:1;
	UINT_8  ucU2SSN:1;
	UINT_8  ucU3SSN:1;
	UINT_8  ucGPBW:2;
	UINT_8  ucUserNum:2;
#endif
#ifdef RT_BIG_ENDIAN
	UINT_32 u4GroupValid:1;
	UINT_32 u4GroupID:11; // 0 ~ 2047 Group Idx
	UINT_32 u4U3PFID:5;
	UINT_32 u4U2PFID:5;
	UINT_32 u4U1PFID:5;
	UINT_32 u4U0PFID:5;
#else
	UINT_32 u4U0PFID:5;
	UINT_32 u4U1PFID:5;
	UINT_32 u4U2PFID:5;
	UINT_32 u4U3PFID:5;
	UINT_32 u4GroupID:11; /* 0 ~ 2047 Group Idx */
	UINT_32 u4GroupValid:1;
#endif

	UINT_8  ucU0TxPER;
	UINT_8  ucU1TxPER;
	UINT_8  ucU2TxPER;
	UINT_8  ucU3TxPER;

	/* For Future Used : Smart MU Rate Algorithm with Dynamic Programming Method */
#ifdef RT_BIG_ENDIAN
	UINT_8	ucU1TxQ_UpSS1:4;
	UINT_8  ucU0TxQ_UpSS1:4;
#else
    UINT_8  ucU0TxQ_UpSS1:4;
	UINT_8  ucU1TxQ_UpSS1:4;
#endif
#ifdef RT_BIG_ENDIAN
	UINT_8	ucU3TxQ_UpSS1:4;
	UINT_8	ucU2TxQ_UpSS1:4;
#else
	UINT_8  ucU2TxQ_UpSS1:4;
	UINT_8  ucU3TxQ_UpSS1:4;
#endif
#ifdef RT_BIG_ENDIAN
	UINT_8	ucU1TxQ_UpSS2:4;
	UINT_8	ucU0TxQ_UpSS2:4;
#else
	UINT_8  ucU0TxQ_UpSS2:4;
	UINT_8  ucU1TxQ_UpSS2:4;
#endif
#ifdef RT_BIG_ENDIAN
	UINT_8	ucU3TxQ_UpSS2:4;
	UINT_8	ucU2TxQ_UpSS2:4;
#else
	UINT_8  ucU2TxQ_UpSS2:4;
	UINT_8  ucU3TxQ_UpSS2:4;
#endif
#ifdef RT_BIG_ENDIAN
	UINT_8	ucU1TxQ_Down:4;
	UINT_8	ucU0TxQ_Down:4;
#else
	UINT_8  ucU0TxQ_Down:4;
	UINT_8  ucU1TxQ_Down:4;
#endif
#ifdef RT_BIG_ENDIAN
	UINT_8	ucU3TxQ_Down:4;
	UINT_8	ucU2TxQ_Down:4;
#else
	UINT_8  ucU2TxQ_Down:4;
	UINT_8  ucU3TxQ_Down:4;
#endif
} MURA_CN_ENTRY_INFO_T, *P_MURA_CN_ENTRY_INFO_T;


typedef struct _CMD_MURGA_SET_GROUP_TBL_ENTRY {
#ifdef RT_BIG_ENDIAN
	UINT_8		 NS3:1;
	UINT_8		 NS2:1;
	UINT_8		 NS1:1;
	UINT_8		 NS0:1;
	UINT_8		 BW:2;
	UINT_8		 numUser:2;
#else
	UINT_8		 numUser:2;
	UINT_8		 BW:2;
	UINT_8		 NS0:1;
	UINT_8		 NS1:1;
	UINT_8		 NS2:1;
	UINT_8		 NS3:1;
#endif

	UINT_8       WLIDUser0; /* WLANID0 */
	UINT_8       WLIDUser1; /* WLANID1 */
	UINT_8       WLIDUser2; /* WLANID2 */
	UINT_8       WLIDUser3; /* WLANID3 */
#ifdef RT_BIG_ENDIAN
	UINT_8		 initMcsUser1:4;
	UINT_8		 initMcsUser0:4;
#else
	UINT_8		 initMcsUser0:4;
	UINT_8		 initMcsUser1:4;
#endif

#ifdef RT_BIG_ENDIAN
	UINT_8		 initMcsUser3:4;
	UINT_8		 initMcsUser2:4;
#else
	UINT_8		 initMcsUser2:4;
	UINT_8		 initMcsUser3:4;
#endif

} CMD_MURGA_SET_GROUP_TBL_ENTRY, *P_CMD_MURGA_SET_GROUP_TBL_ENTRY;

typedef struct _CMD_SET_PERIODIC_SND {
	UINT16 u2Reserved;
} CMD_SET_PERIODIC_SND, *P_CMD_SET_PERIODIC_SND;

typedef struct _CMD_SET_SND_PARAMS {
	UINT_8       ucAllMuUser;
	UINT_8       ucWLIDUser;
	UINT_8       ucMaxSoundingPeriod;
	UINT_8       ucMinSoundingPeriod;
	UINT_8       ucSoundingPeriodStep;
} CMD_SET_SND_PARAMS, *P_CMD_SET_SND_PARAMS;

typedef struct _CMD_SET_PLATFORM_TYPE {
	UINT_8       ucPlatformType;
	UINT_8       ucReserved[3];
} CMD_SET_PLATFORM_TYPE, *P_CMD_SET_PLATFORM_TYPE;

typedef struct _CMD_SET_DISABLE_CN3_CN4 {
	UINT_8       ucDisableCn3Cn4;
	UINT_8       ucReserved[3];
} CMD_SET_DISABLE_CN3_CN4, *P_CMD_SET_DISABLE_CN3_CN4;

typedef struct _CMD_MURGA_SET_MOBILITY_TYPE {
	BOOLEAN      fgMobilityType;
	UINT_8       ucReserved[3];
} CMD_MURGA_SET_MOBILITY_TYPE, *P_CMD_MURGA_SET_MOBILITY_TYPE;

typedef struct _CMD_MURGA_SET_MOBILITY_INTERVAL {
	UINT_16      u2MobilityInteral;
	UINT_8       ucReserved[2];
} CMD_MURGA_SET_MOBILITY_INTERVAL, *P_CMD_MURGA_SET_MOBILITY_INTERVAL;

typedef struct _CMD_MURGA_SET_MOBILITY_SNR {
	UINT_8       ucMobilitySNR;
	UINT_8       ucReserved[3];
} CMD_MURGA_SET_MOBILITY_SNR, *P_CMD_MURGA_SET_MOBILITY_SNR;

typedef struct _CMD_MURGA_SET_MOBILITY_THRESHOLD {
	UINT_8       ucWlanId;
	UINT_8       ucMobilityThreshold;
	UINT_8       ucReserved[2];
} CMD_MURGA_SET_MOBILITY_THRESHOLD, *P_CMD_MURGA_SET_MOBILITY_THRESHOLD;

typedef struct _CMD_MURGA_GET_MOBILITY_SND_INTERVAL {
	BOOLEAN      fgMobilitySndIntvalCnt;
	UINT_8       ucReserved[3];
} CMD_MURGA_GET_MOBILITY_SND_INTERVAL, *P_CMD_MURGA_GET_MOBILITY_SND_INTERVAL;

typedef struct _CMD_MURGA_SET_MOBILITY_MODE {
	BOOLEAN      fgMULQPingPongEn;
    BOOLEAN      fgMULQTriggerCalEn;
    BOOLEAN      fgMobilityFlagForceEn;
	UINT_8       ucReserved;
} CMD_MURGA_SET_MOBILITY_MODE, *P_CMD_MURGA_SET_MOBILITY_MODE;

typedef struct _CMD_MURGA_SET_MOBILITY_LOG {
	BOOLEAN      fgMobilityLogEn;
	UINT_8       ucReserved[3];
} CMD_MURGA_SET_MOBILITY_LOG, *P_CMD_MURGA_SET_MOBILITY_LOG;

typedef struct _CMD_MURGA_SET_MOBILITY_TEST {
	BOOLEAN      fgMobilityTestEn;
	UINT_8       ucReserved[3];
} CMD_MURGA_SET_MOBILITY_TEST, *P_CMD_MURGA_SET_MOBILITY_TEST;

typedef struct _EVENT_SHOW_ALGORITHM_STATE {
	UINT_32                 u4EventId;

	/* MU-RGA Overall Information */
	BOOLEAN                 fgMobility[MAX_MURA_NUM];
	UINT_8                  ucMaxMuarNum;
	UINT_8                  ucSoundingNum;
	UINT_8                  ucWaitingNum;
	UINT_8                  ucMaxSndingCap;

	UINT_8                  ucSoundingPeriod[MAX_MURA_NUM];
	UINT_32                 u4TxSuccCnt[MAX_MURA_NUM];
	UINT_32                 u4TxFailCnt[MAX_MURA_NUM];
	UINT_32                 u4TxCn3SuccCnt[MAX_MURA_NUM];
	UINT_32                 u4TxCn3FailCnt[MAX_MURA_NUM];
	UINT_32                 u4TxCn4SuccCnt[MAX_MURA_NUM];
	UINT_32                 u4TxCn4FailCnt[MAX_MURA_NUM];
	UINT_8                  ucTxPER[MAX_MURA_NUM];
	UINT_8                  ucTxCn3PER[MAX_MURA_NUM];
	UINT_8                  ucTxCn4PER[MAX_MURA_NUM];

	/* PFID Sounding Information */
	UINT_8                  ucMURAWlanIdList[MAX_MURA_NUM];
	UINT_8                  ucMURAPfidList[MAX_MURA_NUM];
	BOOLEAN                 au4PfidGroupTableMap[MAX_MURA_NUM][MAX_MURA_GRP];
	UINT_8                  aucMURADeltaMCS[MAX_MURA_NUM];
	UINT_16                 au2SuccSounding[MAX_MURA_NUM];
	UINT_16                 au2FailSounding[MAX_MURA_NUM];
	UINT_8                  aucSoundingFailRate[MAX_MURA_NUM];

	/* MU-RGA Timing */
	UINT_32                 u4CalculateSoundingEnd;
	UINT_32                 u4CalculateSoundingStart;
	UINT_32                 u4CalculateGroupMcsRateEnd;
	UINT_32                 u4CalculateGroupMcsRateStart;

} EVENT_SHOW_ALGORITHM_STATE, *P_EVENT_SHOW_ALGORITHM_STATE;

typedef struct _EVENT_SHOW_PFID_STAT {
	UINT_32		u4EventId;

	/* MU-RGA Overall Information */
	BOOLEAN		fgMobility;
	UINT_8		u1MaxMuarNum;
	UINT_8		u1SoundingNum;
	UINT_8		u1WaitingNum;
	UINT_8		u1MaxSndingCap;

	UINT_8		u1SoundingPeriod;
	UINT_8		u1MinSoundingPeriod;
	UINT_8		u1MaxSoundingPeriod;
	UINT_8		u1SoundingPeriodStep;
	UINT_8		u1WlanIdx;
	UINT_32		u4TxSuccCnt;
	UINT_32		u4TxFailCnt;
	UINT_32		u4TxCn3SuccCnt;
	UINT_32		u4TxCn3FailCnt;
	UINT_32		u4TxCn4SuccCnt;
	UINT_32		u4TxCn4FailCnt;
	UINT_8		u1TxPER;
	UINT_8		u1TxCn3PER;
	UINT_8		u1TxCn4PER;

	/* PFID Sounding Information */
	UINT_16		u2SuccSounding;
	UINT_16		u2FailSounding;
	UINT_8		u1SoundingFailRate;
} EVENT_SHOW_PFID_STAT, *P_EVENT_SHOW_PFID_STAT;

typedef struct _EVENT_SHOW_ALGORITHM_GROUP_STATE {
	UINT_32                 u4EventId;

	/* Group Entry Information */
	/* MURA_CN_ENTRY_INFO_T    arMuraGroupEntry[MAX_MURA_GRP]; */
	MURA_CN_ENTRY_INFO_T    rMuraGroupEntry;

} EVENT_SHOW_ALGORITHM_GROUP_STATE, *P_EVENT_SHOW_ALGORITHM_GROUP_STATE;

typedef struct _EVENT_SHOW_ALGORITHM_HWFB_STATE {
	UINT_32                 u4EventId;

	/* HWFB Entry Information */
	BOOLEAN                 fgDownOneStep[10];
	BOOLEAN                 fgUpOneStep[10];
	UINT_8                  ucDownThreshold[10];
	UINT_8                  ucUpThreshold[10];
	UINT_8                  ucDownMCS[10];
	UINT_8                  ucUpMCS[10];
	UINT_8                  uc20BWSample[10];
	UINT_8                  uc40BWSample[10];
	UINT_8                  uc80BWSample[10];
	UINT_8                  uc20BWSampleFactor;
	UINT_8                  uc40BWSampleFactor;
	UINT_8                  uc80BWSampleFactor;

} EVENT_SHOW_ALGORITHM_HWFB_STATE, *P_EVENT_SHOW_ALGORITHM_HWFB_STATE;


INT GetMuraMonitorStateProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

/** @brief Get MU_RGA PFID statistics
 *
 *  Get MU_RGA PFID statistics for debug
 *
 *  @param pAd: pointer to adapter
 *  @param arg: pointer to argument
 *  @return Int.
 */
INT GetMuraPFIDStatProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetMuraPeriodicSndProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetMuraTestAlgorithmProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetMuraTestAlgorithmInit(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetMuraFixedRateProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetMuraFixedGroupRateProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetMuraFixedSndParamProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetMuraPlatformTypeProc(RTMP_ADAPTER *pAd);
INT SetMuraMobilityCtrlProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetMuraMobilityIntervalCtrlProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetMuraMobilitySNRCtrlProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetMuraMobilityThresholdCtrlProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetMuraMobilitySndCountProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetMuraMobilityModeCtrlProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetMuraMobilityLogCtrlProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT SetMuraMobilityTestCtrlProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#endif  /* __AP_MUMIMO_RA_H__ */

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
    radar.h

    Abstract:
     CS/DFS common functions.

    Revision History:
    Who       When            What
    --------  ----------      ----------------------------------------------
*/
#ifndef __RADAR_H__
#define __RADAR_H__

struct freq_cfg;

#define DEFAULT_CAL_BUF_TIME	60
#define DEFAULT_CAL_BUF_TIME_MAX	0x10000

/* RESTRICTION_BAND_1: 5600MHz ~ 5650MHz */
#define RESTRICTION_BAND_1(_pAd, __Channel, _BW)												\
	(_BW >= BW_40 ?						\
	 ((__Channel >= 116) && (__Channel <= 128)) :	\
	 ((__Channel >= 120) && (__Channel <= 128)))

#define RESTRICTION_BAND_KOREA(_pAd, __Channel, _BW)												\
	(_BW >= BW_80 ? 				\
	((__Channel >= 116) && (__Channel <= 128)) :	\
	(_BW >= BW_40 ?						\
	((__Channel >= 124) && (__Channel <= 128)) :	\
	(__Channel == 128)))

#define IS_DOT11_H_RADAR_STATE(_pAd, _RadarState, __Channel, _pDot11h)		\
	((__Channel > 14)	\
	 && (_pAd->CommonCfg.bIEEE80211H == 1)	\
	 && RadarChannelCheck(_pAd, __Channel)	\
	 && _pDot11h->RDMode == _RadarState)

#ifdef MT_DFS_SUPPORT
#define IS_SUPPORT_MT_DFS(_pAd) \
	(_pAd->CommonCfg.DfsParameter.bDfsEnable == TRUE)

#define UPDATE_MT_ZEROWAIT_DFS_STATE(_pAd, _State) \
	{                                         \
		_pAd->CommonCfg.DfsParameter.ZeroWaitDfsState = _State; \
	}

#define UPDATE_MT_ZEROWAIT_DFS_Support(_pAd, _Enable) \
	{                                         \
		_pAd->CommonCfg.DfsParameter.bZeroWaitSupport = _Enable; \
	}

#define IS_SUPPORT_MT_ZEROWAIT_DFS(_pAd) \
	(_pAd->CommonCfg.DfsParameter.bZeroWaitSupport == TRUE)

#define IS_SUPPORT_DEDICATED_ZEROWAIT_DFS(_pAd) \
	(_pAd->CommonCfg.DfsParameter.bDedicatedZeroWaitSupport == TRUE)

#define CHK_MT_ZEROWAIT_DFS_STATE(_pAd, __STATE) \
	((_pAd->CommonCfg.DfsParameter.ZeroWaitDfsState == __STATE))

#define GET_MT_ZEROWAIT_DFS_STATE(_pAd) \
	((_pAd->CommonCfg.DfsParameter.ZeroWaitDfsState))

#define UPDATE_MT_INIT_ZEROWAIT_MBSS(_pAd, _Enable) \
	{                                         \
		_pAd->CommonCfg.DfsParameter.bInitMbssZeroWait = _Enable; \
	}

#define GET_MT_MT_INIT_ZEROWAIT_MBSS(_pAd) \
	((_pAd->CommonCfg.DfsParameter.bInitMbssZeroWait))

#endif /* MT_DFS_SUPPORT */

/* 802.11H */
struct DOT11_H {
	/* 802.11H and DFS related params */
	UCHAR CSCount;		/*Channel switch counter */
	UCHAR CSPeriod;	/*Channel switch period (beacon count) */
	USHORT RDCount;	/*Radar detection counter, if RDCount >  ChMovingTime, start to send beacons*/
	UCHAR RDMode;		/*Radar Detection mode */
	UCHAR org_ch;
	UCHAR new_channel;
	USHORT ChMovingTime;
	USHORT DfsZeroWaitChMovingTime;
	BOOLEAN bDFSIndoor;
	ULONG InServiceMonitorCount;	/* unit: sec */
	ULONG CalBufTime;	/* A Timing buffer for befroe calibrations which generates Tx signals */
	UINT16 wdev_count;
};

BOOLEAN RadarChannelCheck(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR			Ch);

VOID RadarStateCheck(
	struct _RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev);

BOOLEAN CheckNonOccupancyChannel(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev);

ULONG JapRadarType(
	IN PRTMP_ADAPTER pAd);

UCHAR get_channel_by_reference(
	IN PRTMP_ADAPTER pAd,
	IN UINT8 mode,
	IN struct wifi_dev *wdev);

#ifdef CONFIG_AP_SUPPORT
VOID ChannelSwitchingCountDownProc(
	IN PRTMP_ADAPTER	pAd,
	struct wifi_dev *wdev);

NTSTATUS Dot11HCntDownTimeoutAction(RTMP_ADAPTER *pAd, PCmdQElmt CMDQelmt);

#endif /* CONFIG_AP_SUPPORT */

VOID RadarDetectPeriodic(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR Channel);

INT Set_CSPeriod_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_ChMovingTime_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_BlockChReset_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

/* wdev->pDot11H Initailization */
VOID UpdateDot11hForWdev(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, BOOLEAN attach);

#endif /* __RADAR_H__ */

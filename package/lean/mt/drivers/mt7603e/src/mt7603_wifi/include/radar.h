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

#define DEFAULT_CAL_BUF_TIME	60
#define DEFAULT_CAL_BUF_TIME_MAX	0x10000

/* RESTRICTION_BAND_1: 5600MHz ~ 5650MHz */
#define RESTRICTION_BAND_1(_pAd)												\
	(_pAd->CommonCfg.RegTransmitSetting.field.BW == BW_40 ? 						\
	((_pAd->CommonCfg.Channel >= 116) && (_pAd->CommonCfg.Channel <= 128)) :	\
	((_pAd->CommonCfg.Channel >= 120) && (_pAd->CommonCfg.Channel <= 128)))

#define IS_DOT11_H_RADAR_STATE(_pAd, _RadarState)		\
		( (_pAd->CommonCfg.Channel > 14)	\
		&& (_pAd->CommonCfg.bIEEE80211H == 1)	\
		&& RadarChannelCheck(_pAd, _pAd->CommonCfg.Channel)	\
		&& _pAd->Dot11_H.RDMode == _RadarState)

/* 802.11H */
typedef struct _DOT11_H {
	/* 802.11H and DFS related params */
	UCHAR CSCount;		/*Channel switch counter */
	UCHAR CSPeriod; 	/*Channel switch period (beacon count) */
	USHORT RDCount; 	/*Radar detection counter, if RDCount >  ChMovingTime, start to send beacons*/
	UCHAR RDMode;		/*Radar Detection mode */
	UCHAR org_ch;
	UCHAR new_channel;
	USHORT ChMovingTime;
	BOOLEAN bDFSIndoor;
	ULONG InServiceMonitorCount;	/* unit: sec */
	ULONG CalBufTime;	/* A Timing buffer for befroe calibrations which generates Tx signals */
} DOT11_H, *PDOT11_H;

BOOLEAN RadarChannelCheck(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR			Ch);

VOID RadarStateCheck(
	IN PRTMP_ADAPTER	pAd);

ULONG JapRadarType(
	IN PRTMP_ADAPTER pAd);

UCHAR get_channel_by_reference(
	IN PRTMP_ADAPTER pAd,
	IN UINT8 mode);

#ifdef CONFIG_AP_SUPPORT
VOID ChannelSwitchingCountDownProc(
	IN PRTMP_ADAPTER	pAd);
#endif /* CONFIG_AP_SUPPORT */

VOID RadarDetectPeriodic(
	IN PRTMP_ADAPTER	pAd);

INT Set_CSPeriod_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_ChMovingTime_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_BlockChReset_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#if defined(DFS_SUPPORT) || defined(CARRIER_DETECTION_SUPPORT)
INT	Set_RadarShow_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

VOID CckMrcStatusCtrl(
	IN PRTMP_ADAPTER pAd);

VOID RadarGLRTCompensate(
	IN PRTMP_ADAPTER pAd);

#endif /*defined(DFS_SUPPORT) || defined(CARRIER_DETECTION_SUPPORT)*/

#endif /* __RADAR_H__ */

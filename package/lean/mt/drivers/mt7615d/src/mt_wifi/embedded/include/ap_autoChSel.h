/****************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ****************************************************************************

    Abstract:


 */

#include "ap_autoChSel_cmm.h"

#ifndef __AUTOCHSELECT_H__
#define __AUTOCHSELECT_H__

ULONG AutoChBssSearchWithSSID(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR Bssid,
	IN PUCHAR pSsid,
	IN UCHAR SsidLen,
	IN UCHAR Channel,
	IN struct wifi_dev *pwdev);

VOID APAutoChannelInit(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *pwdev);

VOID UpdateChannelInfo(
	IN PRTMP_ADAPTER pAd,
	IN int ch,
	IN ChannelSel_Alg Alg,
	IN struct wifi_dev *pwdev);

#ifdef DFS_VENDOR10_CUSTOM_FEATURE
#define IS_V10_BOOTACS_INVALID(_pAd) \
	 (_pAd->CommonCfg.DfsParameter.bV10BootACSValid == TRUE)

#define SET_V10_BOOTACS_INVALID(_pAd, valid) \
	(_pAd->CommonCfg.DfsParameter.bV10BootACSValid = valid)

#define IS_V10_APINTF_DOWN(_pAd) \
		 (_pAd->CommonCfg.DfsParameter.bV10APInterfaceDownEnbl == TRUE)

#define SET_V10_APINTF_DOWN(_pAd, valid) \
		(_pAd->CommonCfg.DfsParameter.bV10APInterfaceDownEnbl = valid)

#define IS_V10_W56_AP_DOWN_ENBLE(_pAd) \
	 (_pAd->CommonCfg.DfsParameter.bV10W56APDownEnbl == TRUE)

#define SET_V10_W56_AP_DOWN(_pAd, valid) \
	(_pAd->CommonCfg.DfsParameter.bV10W56APDownEnbl = valid)

#define SET_V10_AP_BCN_UPDATE_ENBL(_pAd, enable) \
	(_pAd->CommonCfg.DfsParameter.bV10APBcnUpdateEnbl = enable)

#define IS_V10_AP_BCN_UPDATE_ENBL(_pAd) \
	 (_pAd->CommonCfg.DfsParameter.bV10APBcnUpdateEnbl == TRUE)


VOID AutoChannelSkipListAppend(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR			Ch);

VOID AutoChannelSkipChannels(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR			size,
	IN UINT16			grpStart);

VOID AutoChannelSkipListClear(
	IN PRTMP_ADAPTER	pAd);

BOOLEAN DfsV10ACSMarkChnlConsumed(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR channel);
#endif

ULONG AutoChBssInsertEntry(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pBssid,
	IN CHAR Ssid[],
	IN UCHAR SsidLen,
	IN UCHAR ChannelNo,
	IN UCHAR ExtChOffset,
	IN CHAR Rssi,
	IN struct wifi_dev *pwdev);

VOID AutoChBssTableInit(
	IN PRTMP_ADAPTER pAd);

VOID ChannelInfoInit(
	IN PRTMP_ADAPTER pAd);

VOID AutoChBssTableDestroy(
	IN PRTMP_ADAPTER pAd);

VOID ChannelInfoDestroy(
	IN PRTMP_ADAPTER pAd);

VOID CheckPhyModeIsABand(
	IN PRTMP_ADAPTER pAd,
	IN UINT8 BandIdx);

UCHAR SelectBestChannel(
	IN PRTMP_ADAPTER pAd,
	IN ChannelSel_Alg Alg,
	IN struct wifi_dev *pwdev);
UCHAR APAutoSelectChannel(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *pwdev,
	IN ChannelSel_Alg Alg,
	IN BOOLEAN IsABand);

UCHAR MTAPAutoSelectChannel(
	IN RTMP_ADAPTER * pAd,
	IN struct wifi_dev *pwdev,
	IN ChannelSel_Alg Alg,
	IN BOOLEAN IsABand);

#ifdef AP_SCAN_SUPPORT
VOID AutoChannelSelCheck(
	IN PRTMP_ADAPTER pAd);
#endif /* AP_SCAN_SUPPORT */

VOID AutoChSelBuildChannelList(
	IN RTMP_ADAPTER * pAd,
	IN BOOLEAN IsABand,
	IN struct wifi_dev *pwdev);

VOID AutoChSelBuildChannelListFor2G(
	IN RTMP_ADAPTER *pAd,
	IN struct wifi_dev *pwdev);

VOID AutoChSelBuildChannelListFor5G(
	IN RTMP_ADAPTER *pAd,
	IN struct wifi_dev *pwdev);

VOID AutoChSelUpdateChannel(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR Channel,
	IN BOOLEAN IsABand,
	IN struct wifi_dev *pwdev);

CHAR AutoChSelFindScanChIdx(
	IN RTMP_ADAPTER * pAd,
	IN struct wifi_dev *pwdev,
	IN CHAR LastScanChIdx);

VOID AutoChSelScanNextChannel(
	IN RTMP_ADAPTER * pAd,
	IN struct wifi_dev *pwdev);

VOID AutoChSelScanReqAction(
	IN RTMP_ADAPTER * pAd,
	IN MLME_QUEUE_ELEM * pElem);

VOID AutoChSelScanTimeoutAction(
	IN RTMP_ADAPTER * pAd,
	IN MLME_QUEUE_ELEM * pElem);

VOID AutoChSelScanStart(
	IN RTMP_ADAPTER * pAd,
	IN struct wifi_dev *pwdev);

VOID AutoChSelScanTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

VOID AutoChSelStateMachineInit(
	IN RTMP_ADAPTER * pAd,
	IN UCHAR BandIdx,
	IN STATE_MACHINE * Sm,
	OUT STATE_MACHINE_FUNC Trans[]);

VOID AutoChSelInit(
	IN PRTMP_ADAPTER pAd);

#ifdef DFS_VENDOR10_CUSTOM_FEATURE
UINT8 SelectBestV10Chnl_From_List(
	IN RTMP_ADAPTER *pAd);
#endif

#if defined(OFFCHANNEL_SCAN_FEATURE) || defined (ONDEMAND_DFS)
VOID ChannelInfoResetNew(
	IN PRTMP_ADAPTER pAd);
#endif
#if defined(OFFCHANNEL_SCAN_FEATURE) && defined (ONDEMAND_DFS)
UINT8 SelectBestChannel_From_List(
	IN RTMP_ADAPTER *pAd,
	IN BOOLEAN IsABand,
	IN BOOLEAN SkipDFS);
#endif

VOID AutoChSelRelease(
	IN PRTMP_ADAPTER pAd);
#endif /* __AUTOCHSELECT_H__ */


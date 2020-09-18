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

#define AP_AUTO_CH_SEL(__P, __O)	APAutoSelectChannel((__P), (__O))

ULONG AutoChBssSearchWithSSID(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR Bssid,
	IN PUCHAR pSsid,
	IN UCHAR SsidLen,
	IN UCHAR Channel);

VOID APAutoChannelInit(
	IN PRTMP_ADAPTER pAd);

VOID UpdateChannelInfo(
	IN PRTMP_ADAPTER pAd,
	IN int ch,
	IN ChannelSel_Alg Alg);

#ifdef CUSTOMER_DCC_FEATURE
VOID ChannelInfoResetNew(
	IN PRTMP_ADAPTER pAd);
#endif
ULONG AutoChBssInsertEntry(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pBssid,
	IN CHAR Ssid[],
	IN UCHAR SsidLen, 
	IN UCHAR ChannelNo,
	IN UCHAR ExtChOffset,
	IN CHAR Rssi);

VOID AutoChBssTableInit(
	IN PRTMP_ADAPTER pAd);

VOID ChannelInfoInit(
	IN PRTMP_ADAPTER pAd);

VOID AutoChBssTableDestroy(
	IN PRTMP_ADAPTER pAd);

VOID ChannelInfoDestroy(
	IN PRTMP_ADAPTER pAd);

VOID CheckPhyModeIsABand(
	IN PRTMP_ADAPTER pAd);

UCHAR SelectBestChannel(
	IN PRTMP_ADAPTER pAd,
	IN ChannelSel_Alg Alg);

UCHAR APAutoSelectChannel(
	IN PRTMP_ADAPTER pAd,
	IN ChannelSel_Alg Alg);

#ifdef AP_SCAN_SUPPORT
VOID AutoChannelSelCheck(
	IN PRTMP_ADAPTER pAd);
#endif /* AP_SCAN_SUPPORT */

#endif /* __AUTOCHSELECT_H__ */


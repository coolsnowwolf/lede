/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	chlist.c

	Abstract:

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/

#ifndef __CHLIST_H__
#define __CHLIST_H__

#include "rtmp_type.h"
#include "rtmp_def.h"

#define CH_GROUP_BAND0		(1<<0)
#define CH_GROUP_BAND1		(1<<1)
#define CH_GROUP_BAND2		(1<<2)
#define CH_GROUP_BAND3		(1<<3)

typedef struct _CH_GROUP_DESC {
	UCHAR FirstChannel;
	UCHAR NumOfCh;
} CH_GROUP_DESC, *PCH_GROUP_DESC;

typedef struct _CH_DESC {
	UCHAR FirstChannel;
	UCHAR NumOfCh;
	UCHAR ChannelProp;
} CH_DESC, *PCH_DESC;

typedef struct _COUNTRY_REGION_CH_DESC {
	UCHAR RegionIndex;
	PCH_DESC pChDesc;
} COUNTRY_REGION_CH_DESC, *PCOUNTRY_REGION_CH_DESC;

#define ODOR			0
#define IDOR			1
#define BOTH			2

typedef struct _CH_DESP {
	UCHAR FirstChannel;
	UCHAR NumOfCh;
	CHAR MaxTxPwr;			/* dBm */
	UCHAR Geography;			/* 0:out door, 1:in door, 2:both */
	BOOLEAN DfsReq;			/* Dfs require, 0: No, 1: yes. */
} CH_DESP, *PCH_DESP;

typedef struct _CH_REGION {
	UCHAR CountReg[3];
	UCHAR op_class_region;	/* 0: CE, 1: FCC, 2: JAP, 3:JAP_W53, JAP_W56  5:CHN*/
	BOOLEAN edcca_on;
	CH_DESP *pChDesp;
} CH_REGION, *PCH_REGION;

extern CH_REGION ChRegion[];

typedef struct _CH_FREQ_MAP_ {
	UINT16		channel;
	UINT16		freqKHz;
} CH_FREQ_MAP;

extern CH_FREQ_MAP CH_HZ_ID_MAP[];
extern int CH_HZ_ID_MAP_NUM;


#define     MAP_CHANNEL_ID_TO_KHZ(_ch, _khz)                 \
	RTMP_MapChannelID2KHZ(_ch, (UINT32 *)&(_khz))
#define     MAP_KHZ_TO_CHANNEL_ID(_khz, _ch)                 \
	RTMP_MapKHZ2ChannelID(_khz, (INT *)&(_ch))

/* Check if it is Japan W53(ch52,56,60,64) channel. */
#define JapanChannelCheck(_ch)  ((_ch == 52) || (_ch == 56) || (_ch == 60) || (_ch == 64))

PCH_REGION GetChRegion(
	IN PUCHAR CountryCode);

#ifdef EXT_BUILD_CHANNEL_LIST
VOID BuildChannelListEx(
	IN PRTMP_ADAPTER pAd);

VOID BuildBeaconChList(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev,
	OUT PUCHAR pBuf,
	OUT	PULONG pBufLen);
#endif /* EXT_BUILD_CHANNEL_LIST */

UCHAR GetCountryRegionFromCountryCode(UCHAR *country_code);

#ifdef DOT11_N_SUPPORT
BOOLEAN ExtChCheck(PRTMP_ADAPTER pAd, UCHAR Channel, UCHAR Direction, struct wifi_dev *wdev);
UCHAR N_SetCenCh(RTMP_ADAPTER *pAd, UCHAR channel, UCHAR ht_bw);
BOOLEAN N_ChannelGroupCheck(RTMP_ADAPTER *pAd, UCHAR channel, struct wifi_dev *wdev);
VOID ht_ext_cha_adjust(struct _RTMP_ADAPTER *pAd, UCHAR prim_ch, UCHAR *ht_bw, UCHAR *ext_cha, struct wifi_dev *wdev);

#endif /* DOT11_N_SUPPORT */

UINT8 GetCuntryMaxTxPwr(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR PhyMode,
	IN struct wifi_dev *wdev,
	IN UCHAR ht_bw);

VOID RTMP_MapChannelID2KHZ(
	IN UCHAR Ch,
	OUT UINT32 *pFreq);

VOID RTMP_MapKHZ2ChannelID(
	IN ULONG Freq,
	OUT INT *pCh);

UCHAR GetChannel_5GHZ(
	IN PCH_DESC pChDesc,
	IN UCHAR index);

UCHAR GetChannel_2GHZ(
	IN PCH_DESC pChDesc,
	IN UCHAR index);

UCHAR GetChannelFlag(
	IN PCH_DESC pChDesc,
	IN UCHAR index);

UINT16 TotalChNum(
	IN PCH_DESC pChDesc);

UCHAR get_channel_bw_cap(struct wifi_dev *wdev, UCHAR channel);

INT32 ChannelFreqToGroup(
	IN UINT32 ChannelFreq);

BOOLEAN MTChGrpValid(
	IN CHANNEL_CTRL *ChCtrl);

void MTSetChGrp(RTMP_ADAPTER *pAd, RTMP_STRING *buf);

BOOLEAN MTChGrpChannelChk(
	IN CHANNEL_CTRL *pChCtrl,
	IN UCHAR ch);
#endif /* __CHLIST_H__ */


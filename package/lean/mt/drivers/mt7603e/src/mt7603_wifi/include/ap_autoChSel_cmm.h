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


#ifndef __AUTOCHSELECT_CMM_H__
#define __AUTOCHSELECT_CMM_H__

/* Note: RSSI_TO_DBM_OFFSET has been changed to variable for new RF (2004-0720). */
/* SHould not refer to this constant anymore */
//#define RSSI_TO_DBM_OFFSET 120 /* RSSI-115 = dBm */


#ifdef ACS_CTCC_SUPPORT
typedef struct {
	UCHAR BestCtrlChannel; /* Min in group */
	UCHAR CenChannel;
	UCHAR RegulatoryDomain;
	UINT32 Max_Busy_Time; /* Max in group */
	UINT32 Min_Busy_Time; /* Min in group */
	BOOLEAN SkipGroup;
} ACS_SCAN_CH_GROUP_LIST, *PACS_SCAN_CH_GROUP_LIST;
typedef struct {
	UCHAR GroupType; /*0:BW20, 1:BW40, 2:BW80 */
	UCHAR ApCnt;
	UCHAR Channel;
	UCHAR CenChannel;
	UCHAR DfsReq;
	UCHAR RegulatoryDomain;
	BOOLEAN SkipChannel;
	UINT32 BusyTime;

#define CHANNEL_DEFAULT_PROP	0x00
#define CHANNEL_DISABLED		0x01	/* no use */
#define CHANNEL_PASSIVE_SCAN	0x02
#define CHANNEL_NO_IBSS			0x04
#define CHANNEL_RADAR			0x08
#define CHANNEL_NO_FAT_ABOVE	0x10
#define CHANNEL_NO_FAT_BELOW	0x20
#define CHANNEL_40M_CAP			0x40
#define CHANNEL_80M_CAP			0x80
	UCHAR Flags;
} ACS_SCAN_SUPP_CH_LIST, *PACS_SCAN_SUPP_CH_LIST;
#endif
typedef struct {
	ULONG dirtyness[MAX_NUM_OF_CHANNELS+1];
	ULONG ApCnt[MAX_NUM_OF_CHANNELS+1];
	UINT32 FalseCCA[MAX_NUM_OF_CHANNELS+1];
    BOOLEAN SkipList[MAX_NUM_OF_CHANNELS+1];
//#ifdef AP_QLOAD_SUPPORT
	UINT32 chanbusytime[MAX_NUM_OF_CHANNELS+1]; /* QLOAD ALARM */
//#endif /* AP_QLOAD_SUPPORT */
        BOOLEAN IsABand;
#ifdef ACS_CTCC_SUPPORT
	AUTO_CH_SEL_SCORE ChannelScore[MAX_NUM_OF_CHANNELS+1];
	UINT32 ChannelListNum;
	UINT32 GroupChListNum;
	ACS_SCAN_SUPP_CH_LIST suppChList[MAX_NUM_OF_CHANNELS+1];
	ACS_SCAN_CH_GROUP_LIST GroupChList[MAX_NUM_OF_CHANNELS+1];
#endif
} CHANNELINFO, *PCHANNELINFO;

typedef struct {
	UCHAR Bssid[MAC_ADDR_LEN];
	UCHAR SsidLen;
	CHAR Ssid[MAX_LEN_OF_SSID];
	UCHAR Channel;
	UCHAR ExtChOffset;
	CHAR Rssi;
} BSSENTRY, *PBSSENTRY;

typedef struct {
	UCHAR BssNr;
	BSSENTRY BssEntry[MAX_LEN_OF_BSS_TABLE];	
} BSSINFO, *PBSSINFO;


typedef enum ChannelSelAlg
{
	ChannelAlgRandom, /*use by Dfs */
	ChannelAlgApCnt,
	ChannelAlgCCA,
	ChannelAlgBusyTime,
	ChannelAlgCombined
} ChannelSel_Alg;

#endif /* __AUTOCHSELECT_CMM_H__ */


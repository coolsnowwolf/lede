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


typedef struct {
	ULONG dirtyness[MAX_NUM_OF_CHANNELS+1];
	ULONG ApCnt[MAX_NUM_OF_CHANNELS+1];
	UINT32 FalseCCA[MAX_NUM_OF_CHANNELS+1];
    BOOLEAN SkipList[MAX_NUM_OF_CHANNELS+1];
#ifdef CUSTOMER_DCC_FEATURE
	UINT32 chanbusytime[MAX_NUM_OF_CHANNELS+1];
	UINT32 ChannelNo;
	BOOLEAN GetChannelInfo;
#else
#ifdef AP_QLOAD_SUPPORT
	UINT32 chanbusytime[MAX_NUM_OF_CHANNELS+1]; /* QLOAD ALARM */
#endif /* AP_QLOAD_SUPPORT */
#endif /* CUSTOMER_DCC_FEATURE */
        BOOLEAN IsABand;
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
	ChannelAlgCCA
} ChannelSel_Alg;

#endif /* __AUTOCHSELECT_CMM_H__ */


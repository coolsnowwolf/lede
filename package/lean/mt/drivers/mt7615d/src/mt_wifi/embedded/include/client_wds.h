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
	client_wds.h

	Abstract:
*/

#ifndef __CLIENT_WDS_H__
#define __CLIENT_WDS_H__

#include "client_wds_cmm.h"

VOID CliWds_ProxyTabInit(
	IN PRTMP_ADAPTER pAd);

VOID CliWds_ProxyTabDestory(
	IN PRTMP_ADAPTER pAd);

PCLIWDS_PROXY_ENTRY CliWdsEntyAlloc(
	IN PRTMP_ADAPTER pAd);


VOID CliWdsEntyFree(
	IN PRTMP_ADAPTER pAd,
	IN PCLIWDS_PROXY_ENTRY pCliWdsEntry);

VOID CliWdsEnryFreeAid(
	 IN PRTMP_ADAPTER pAd,
	 IN SHORT Aid);

UCHAR *CliWds_ProxyLookup(RTMP_ADAPTER *pAd, UCHAR *pMac);


VOID CliWds_ProxyTabUpdate(
	IN PRTMP_ADAPTER pAd,
	IN SHORT Aid,
	IN PUCHAR pMac);


VOID CliWds_ProxyTabMaintain(
	IN PRTMP_ADAPTER pAd);
#ifndef WDS_SUPPORT
MAC_TABLE_ENTRY *FindWdsEntry(
	IN PRTMP_ADAPTER pAd,
	IN RX_BLK * pRxBlk);

MAC_TABLE_ENTRY *WdsTableLookupByWcid(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR wcid,
	IN PUCHAR pAddr,
	IN BOOLEAN bResetIdelCount);


MAC_TABLE_ENTRY *WdsTableLookup(RTMP_ADAPTER *pAd, UCHAR *addr, BOOLEAN bResetIdelCount);
#endif
#endif /* __CLIENT_WDS_H__ */


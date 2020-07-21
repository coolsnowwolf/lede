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

     Module Name:
     ap_ids.c

     Abstract:
     IDS definition

     Revision History:
     Who         When          What
     --------    ----------    ----------------------------------------------
 */

VOID RTMPIdsPeriodicExec(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

BOOLEAN RTMPSpoofedMgmtDetection(
	IN RTMP_ADAPTER * pAd,
	IN RX_BLK * rxblk);

VOID RTMPConflictSsidDetection(
	IN PRTMP_ADAPTER	pAd,
	IN PUCHAR			pSsid,
	IN UCHAR			SsidLen,
	IN CHAR				Rssi0,
	IN CHAR				Rssi1,
	IN CHAR				Rssi2
#if defined(CUSTOMER_DCC_FEATURE) || defined(CONFIG_MAP_SUPPORT)
	,
	IN CHAR 			Rssi3
#endif
	);

BOOLEAN RTMPReplayAttackDetection(
	IN RTMP_ADAPTER * pAd,
	IN UCHAR * pAddr2,
	IN RX_BLK * rxblk);

VOID RTMPUpdateStaMgmtCounter(
	IN PRTMP_ADAPTER	pAd,
	IN USHORT			type);

VOID RTMPClearAllIdsCounter(
	IN PRTMP_ADAPTER	pAd);

VOID RTMPIdsStart(
	IN PRTMP_ADAPTER	pAd);

VOID RTMPIdsStop(
	IN PRTMP_ADAPTER	pAd);

VOID rtmp_read_ids_from_file(
	IN  PRTMP_ADAPTER pAd,
	char *tmpbuf,
	char *buffer);


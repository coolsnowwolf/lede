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
    sta.h

    Abstract:
    Miniport generic portion header file

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
*/

#ifndef __STA_H__
#define __STA_H__


BOOLEAN RTMPCheckChannel(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR		CentralChannel,
	IN UCHAR		Channel);

VOID InitChannelRelatedValue(
	IN  PRTMP_ADAPTER   pAd);

VOID AdjustChannelRelatedValue(
	IN PRTMP_ADAPTER pAd,
	OUT UCHAR *pBwFallBack,
	IN USHORT ifIndex,
	IN BOOLEAN BandWidth,
	IN UCHAR PriCh,
	IN UCHAR ExtraCh);

VOID RTMPReportMicError(
	IN  PRTMP_ADAPTER   pAd, 
	IN  PCIPHER_KEY     pWpaKey);

INT RTMPCheckRxError(
	IN RTMP_ADAPTER *pAd, 
	IN HEADER_802_11 *pHeader,
	IN RX_BLK *pRxBlk,
	IN RXINFO_STRUC *pRxInfo);

VOID WpaMicFailureReportFrame(
	IN  PRTMP_ADAPTER    pAd, 
	IN  MLME_QUEUE_ELEM *Elem);

VOID WpaDisassocApAndBlockAssoc(
    IN  PVOID SystemSpecific1, 
    IN  PVOID FunctionContext, 
    IN  PVOID SystemSpecific2, 
    IN  PVOID SystemSpecific3);

VOID WpaStaPairwiseKeySetting(
	IN	PRTMP_ADAPTER	pAd);

VOID WpaStaGroupKeySetting(
	IN	PRTMP_ADAPTER	pAd);

VOID WpaSendEapolStart(
	IN	PRTMP_ADAPTER	pAdapter,
	IN  PUCHAR          pBssid);


VOID STAHandleRxDataFrame(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk);

VOID STARxEAPOLFrameIndicate(
	IN	PRTMP_ADAPTER	pAd,
	IN	MAC_TABLE_ENTRY	*pEntry,
	IN	RX_BLK			*pRxBlk,
	IN	UCHAR			FromWhichBSSID);

#ifdef TXBF_SUPPORT
NDIS_STATUS	STAHardTransmit(
	IN PRTMP_ADAPTER	pAd,
	IN TX_BLK			*pTxBlk,
	IN UCHAR			QueIdx,
	IN UCHAR            TxSndgTypePerEntry);

#else

NDIS_STATUS	STAHardTransmit(
	IN PRTMP_ADAPTER	pAd,
	IN TX_BLK			*pTxBlk,
	IN  UCHAR			QueIdx);
#endif

INT STASendPacket(
	IN RTMP_ADAPTER *pAd,
	IN PNDIS_PACKET pPacket);

INT STAInitialize(RTMP_ADAPTER *pAd);

#endif /* __STA_H__ */


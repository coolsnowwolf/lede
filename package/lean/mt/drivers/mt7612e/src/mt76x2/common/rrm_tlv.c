/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2006, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of Ralink Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************/

/****************************************************************************
	Abstract:

***************************************************************************/

#ifdef DOT11K_RRM_SUPPORT

#include "rt_config.h"

#define RTMP_INSERT_IE(_FRAMEBUFER, _TOTALFRAMELEN, _FRAMELEN, _FRAME) {\
	ULONG _TempLen; \
	MakeOutgoingFrame(	(_FRAMEBUFER),	&(_TempLen), \
						(_FRAMELEN),	(_FRAME), \
						END_OF_ARGS); \
	*(_TOTALFRAMELEN) += (_TempLen); \
}


VOID RRM_InsertBcnReqIE(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN PUCHAR pBcnReq)
{
	RTMP_INSERT_IE(pFrameBuf, pFrameLen, sizeof(RRM_BEACON_REQ_INFO), pBcnReq);
}

VOID RRM_InsertBcnReqSsidSubIE(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN PUCHAR pSsid,
	IN UINT8 SsidLen)
{
	ULONG TempLen;
	UINT8 SubId = RRM_BCN_REQ_SUBID_SSID;

	MakeOutgoingFrame(	pFrameBuf,		&TempLen,
						1,				&SubId,
						1,				&SsidLen,
						SsidLen,		(PUCHAR)pSsid,
						END_OF_ARGS);

	*pFrameLen = *pFrameLen + TempLen;
}

VOID RRM_InsertBcnReqRepCndSubIE(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN UINT8 RepCnd,
	IN UINT8 Threshold)
{
	ULONG TempLen;
	UINT8 Len = 2;
	UINT8 SubId = RRM_BCN_REQ_SUBID_BCN_REP_INFO;

	MakeOutgoingFrame(	pFrameBuf,		&TempLen,
						1,				&SubId,
						1,				&Len,
						1,				&RepCnd,
						1,				&Threshold,
						END_OF_ARGS);

	*pFrameLen = *pFrameLen + TempLen;
}

VOID RRM_InsertBcnReqRepDetailSubIE(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN UINT8 Detail)
{
	ULONG TempLen;
	UINT8 Len = 1;
	UINT8 SubId = RRM_BCN_REQ_SUBID_RET_DETAIL;

	MakeOutgoingFrame(	pFrameBuf,		&TempLen,
						1,				&SubId,
						1,				&Len,
						1,				&Detail,
						END_OF_ARGS);

	*pFrameLen = *pFrameLen + TempLen;
}

/*
	==========================================================================
	Description:
		Insert RRM Enable Capabilitys IE into frame.
		
	Parametrs:
		1. frame buffer pointer.
		2. frame length.
	
	Return	: None.
	==========================================================================
 */
VOID RRM_InsertRRMEnCapIE(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN INT BssIdx)
{
	ULONG TempLen;
	UINT8 Len = 5;
	UINT8 ElementID = IE_RRM_EN_CAP;
	RRM_EN_CAP_IE RrmEnCap;
	PMULTISSID_STRUCT pMBss = NULL;

	if (BssIdx < MAX_MBSSID_NUM(pAd))
		pMBss = &pAd->ApCfg.MBSSID[BssIdx];
	else
		return;

	RrmEnCap.field.LinkMeasureCap = 1;
	RrmEnCap.field.NeighborRepCap = 1;
	RrmEnCap.field.ParallelMeasureCap = 0;
	RrmEnCap.field.RepeatMeasureCap = 0;
	RrmEnCap.field.BeaconPassiveMeasureCap = 1;
	RrmEnCap.field.BeaconActiveMeasureCap = 1;
	RrmEnCap.field.BeaconTabMeasureCap = 1;
	RrmEnCap.field.BeaconMeasureReportCndCap = 1;
	RrmEnCap.field.FrameMeasureCap = 0;
	RrmEnCap.field.ChannelLoadMeasureCap = 0;
	RrmEnCap.field.NoiseHistogramMeasureCap = 0;
	RrmEnCap.field.StatisticMeasureCap = 0;
	RrmEnCap.field.LCIMeasureCap = 0;
	RrmEnCap.field.LCIAzimuthCap = 0;
	RrmEnCap.field.TransmitStreamCap = 1;
	RrmEnCap.field.TriggeredTransmitStreamCap = 1;
	RrmEnCap.field.APChannelReportCap = 1;
	RrmEnCap.field.RRMMibCap = 0;
	RrmEnCap.field.OperatingChMaxMeasureDuration = 0;
	RrmEnCap.field.NotOperatingChMaxMeasureDuration = 0;
	RrmEnCap.field.MeasurePilotCap = 0;
	RrmEnCap.field.MeasurePilotTxInfoCap = 0;
	RrmEnCap.field.NeighReportTSFOffsetCap =
		(pMBss->RrmCfg.bDot11kRRMNeighborRepTSFEnable) ? 1 : 0;
	RrmEnCap.field.RCPIMeasureCap = 0;
	RrmEnCap.field.RSNIMeasureCap = 0;
	RrmEnCap.field.BssAvgAccessDelayCap = 0;
	RrmEnCap.field.BssAvaiableAcmCap = 
		0;
	RrmEnCap.field.AntennaInfoCap = 0;

	MakeOutgoingFrame(pFrameBuf,					&TempLen,
						1,							&ElementID,
						1,							&Len,
						Len,						(PUCHAR)&RrmEnCap.word,
						END_OF_ARGS);

	*pFrameLen = *pFrameLen + TempLen;

	return;
}

VOID RRM_InsertNeighborRepIE(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN UINT8 Len,
	IN PUINT8 pBssid,
	IN RRM_BSSID_INFO BssidInfo,
	IN UINT8 RegClass,
	IN UINT8 Channel,
	IN UINT8 PhyType)
{
	ULONG TempLen;
	UINT8 IEId = IE_RRM_NEIGHBOR_REP;
	
	RRM_NEIGHBOR_REP_INFO NeighborRepInfo;

	COPY_MAC_ADDR(NeighborRepInfo.Bssid, pBssid);
	NeighborRepInfo.BssidInfo = BssidInfo.word;
	NeighborRepInfo.RegulatoryClass = RegClass;
	NeighborRepInfo.ChNum = Channel;
	NeighborRepInfo.PhyType = PhyType;

	MakeOutgoingFrame(	pFrameBuf,						&TempLen,
						1,								&IEId,
						1,								&Len,
						sizeof(RRM_NEIGHBOR_REP_INFO),	&NeighborRepInfo,
						END_OF_ARGS);

	*pFrameLen = *pFrameLen + TempLen;

	return;
}

VOID RRM_InsertNeighborTSFOffsetSubIE(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN UINT16 TSFOffset,
	IN UINT16 BcnInterval)
{
	ULONG TempLen;
	UINT8 Len = 4;
	UINT8 SubId = RRM_NEIGHBOR_REP_TSF_INFO_SUB_ID;

	MakeOutgoingFrame(	pFrameBuf,		&TempLen,
						1,				&SubId,
						1,				&Len,
						2,				&TSFOffset,
						2,				&BcnInterval,
						END_OF_ARGS);

	*pFrameLen = *pFrameLen + TempLen;
}

VOID RRM_InsertQuietIE(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN UINT8 QuietCnt,
	IN UINT8 QuietPeriod,
	IN UINT8 QuietDuration,
	IN UINT8 QuietOffset)
{
	ULONG TempLen;
	UINT8 IEId = IE_QUIET;
	QUIET_INFO QuietInfo;
	UINT8 Len;
	
	QuietInfo.QuietCnt = QuietCnt;
	QuietInfo.QuietPeriod = QuietPeriod;
	QuietInfo.QuietDuration = cpu2le16(QuietDuration);
	QuietInfo.QuietOffset = cpu2le16(QuietOffset);
	Len = 6;
	MakeOutgoingFrame(	pFrameBuf,		&TempLen,
						1,				&IEId,
						1,				&Len,
						Len,			&QuietInfo,
						END_OF_ARGS);

	*pFrameLen = *pFrameLen + TempLen;

	return;	
}

VOID RRM_InsertBssACDelayIE(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen)
{
	ULONG TempLen;
	UINT8 IEId = IE_BSS_AC_DELAY;
	RRM_BSS_AC_DELAY_INFO AcDelayInfo;
	UINT8 Len;

	Len = 4;
	AcDelayInfo.BE_ACDelay = 255;
	AcDelayInfo.BK_ACDelay = 255;
	AcDelayInfo.VI_ACDelay = 255;
	AcDelayInfo.VO_ACDelay = 255;

	MakeOutgoingFrame(	pFrameBuf,		&TempLen,
						1,				&IEId,
						1,				&Len,
						Len,			&AcDelayInfo,
						END_OF_ARGS);

	*pFrameLen = *pFrameLen + TempLen;

	return;	
}

VOID RRM_InsertBssAvailableACIE(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen)
{
	INT idx;
	ULONG TempLen;
	UINT8 IEId = IE_BSS_AVAILABLE_AC;
	RRM_BSS_AVAILABLE_AC_INFO AvailableAcInfo;
	PRRM_BSS_AVAILABLE_AC_BITMAP pAcBitMap;
	UINT8 Len;


	pAcBitMap = (PRRM_BSS_AVAILABLE_AC_BITMAP) \
					&AvailableAcInfo.AvailableAcBitMap;

	pAcBitMap->word = 0;

	pAcBitMap->word = cpu2le16(pAcBitMap->word);

	/* cacule the total length of the IE. */
	Len = 2;
	for (idx=0; idx<12; idx++)
	{
		if (pAcBitMap->word & (0x1 << idx))
			Len += 2;
	}

	MakeOutgoingFrame(	pFrameBuf,							&TempLen,
						1,									&IEId,
						1,									&Len,
						sizeof(RRM_BSS_AVAILABLE_AC_INFO),	&AvailableAcInfo,
						END_OF_ARGS);

	*pFrameLen += TempLen;
	pFrameBuf += TempLen;


	return;	
}

VOID RRM_InsertRequestIE(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen)
{
	UINT8 RequestIEs[] = {	0,		/* SSID */
							48,		/* RSN IE */
							70,		/* RRM Capabilities. */
							54,		/* Mobility Domain. */
							221};	/* Vendor Specific. */

	ULONG TempLen;
	UINT8 IEId = IE_802_11D_REQUEST;
	UINT8 Len;

	Len = 5;
	MakeOutgoingFrame(	pFrameBuf,		&TempLen,
						1,				&IEId,
						1,				&Len,
						Len,			RequestIEs,
						END_OF_ARGS);

	*pFrameLen = *pFrameLen + TempLen;

	return;	
}

VOID RRM_InsertTxStreamReqIE(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN PUCHAR pBuf)
{
	RTMP_INSERT_IE(pFrameBuf, pFrameLen,
			sizeof(RRM_TRANSMIT_MEASURE_INFO), pBuf);
}

VOID RRM_InsertTxStreamReqTriggerReportSubIE(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN PUCHAR pBuf)
{
	ULONG TempLen;
	ULONG Len = sizeof(RRM_TRANSMIT_MEASURE_TRIGGER_REPORT);
	UINT8 SubId = RRM_TX_STREAM_SUBID_TRIGGER_REPORT;

	MakeOutgoingFrame(	pFrameBuf,		&TempLen,
						1,				&SubId,
						1,				&Len,
						Len,			pBuf,
						END_OF_ARGS);

	*pFrameLen = *pFrameLen + TempLen;
}

VOID RRM_EnqueueBcnReq(
	IN PRTMP_ADAPTER pAd,
	IN UINT8 Aid,
	IN UINT8 IfIdx,
	IN PRRM_MLME_BCN_REQ_INFO pMlmeBcnReq)
{
	UINT8 MeasureReqType = RRM_MEASURE_SUBTYPE_BEACON;
	MEASURE_REQ_MODE MeasureReqMode;
	UINT8 MeasureReqToken = RandomByte(pAd);
	RRM_BEACON_REQ_INFO BcnReq;
	UINT8 ReportDetail;
	UINT8 TotalLen;

	HEADER_802_11 ActHdr;
	PUCHAR pOutBuffer = NULL;
	NDIS_STATUS NStatus;
	ULONG FrameLen;

	NStatus = MlmeAllocateMemory(pAd, (PVOID)&pOutBuffer);  /*Get an unused nonpaged memory */
	if(NStatus != NDIS_STATUS_SUCCESS)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("%s() allocate memory failed \n", __FUNCTION__));
		return;
	}

	/* build action frame header. */
	MgtMacHeaderInit(pAd, &ActHdr, SUBTYPE_ACTION, 0, pAd->MacTab.Content[Aid].Addr,
						pAd->ApCfg.MBSSID[IfIdx].wdev.if_addr,
						pAd->ApCfg.MBSSID[IfIdx].wdev.bssid);

	NdisMoveMemory(pOutBuffer, (PCHAR)&ActHdr, sizeof(HEADER_802_11));
	FrameLen = sizeof(HEADER_802_11);

	/*
		Action header has a field to indicate total length of packet
		but the total length is unknow untial whole packet completd.
		So skip the action here and fill it late.
		1. skip Catgore (1 octect), Action(1 octect).
		2. skip dailog token (1 octect).
		3. skip Num Of Repetitions field (2 octects)
		3. skip MeasureReqIE (2 + sizeof(MEASURE_REQ_INFO)).
	*/
	FrameLen += (7 + sizeof(MEASURE_REQ_INFO));

	TotalLen = sizeof(MEASURE_REQ_INFO);
	/*
		Insert Beacon Req IE.
	*/
	BcnReq.RegulatoryClass = pMlmeBcnReq->RegulatoryClass;
	BcnReq.ChNumber = pMlmeBcnReq->MeasureCh;
	BcnReq.RandomInterval = cpu2le16((UINT16)RandomByte(pAd) << 8
								| (UINT16)RandomByte(pAd));
	BcnReq.MeasureDuration = cpu2le16(pMlmeBcnReq->MeasureDuration);
	BcnReq.MeasureMode = pMlmeBcnReq->MeasureMode;
	COPY_MAC_ADDR(BcnReq.Bssid, pMlmeBcnReq->Bssid);

	RRM_InsertBcnReqIE(pAd, (pOutBuffer+FrameLen),
		&FrameLen, (PUCHAR)&BcnReq);

	TotalLen += sizeof(RRM_BEACON_REQ_INFO);

	/* inssert SSID sub field. */
	if (pMlmeBcnReq->SsidLen != 0)
	{
		RRM_InsertBcnReqSsidSubIE(pAd, (pOutBuffer+FrameLen),
			&FrameLen, (PUCHAR)pMlmeBcnReq->pSsid, pMlmeBcnReq->SsidLen);

		TotalLen += (pMlmeBcnReq->SsidLen + 2); /* SSID sub field. */
	}

	/* inssert report condition sub field. */
	if (pMlmeBcnReq->BcnReqCapFlag.field.ReportCondition == TRUE)
	{
		RRM_InsertBcnReqRepCndSubIE(pAd, (pOutBuffer+FrameLen), &FrameLen, 0, 0);
		TotalLen += (2 + 2); /* SSID sub field. */
	}

	/* inssert channel report sub field. */
	if (pMlmeBcnReq->BcnReqCapFlag.field.ChannelRep == TRUE)
	{
		ULONG idx;
		idx = 0;
		while(pMlmeBcnReq->ChRepRegulatoryClass[idx] != 0)
		{
			ULONG FramelenTmp = FrameLen;
			InsertChannelRepIE(pAd, (pOutBuffer+FrameLen), &FrameLen,
								(PSTRING)pAd->CommonCfg.CountryCode,
								pMlmeBcnReq->ChRepRegulatoryClass[idx]);
			TotalLen += (FrameLen - FramelenTmp);
			idx ++;
		}
	}


	/* inssert report detail sub field. */
	if (BcnReq.MeasureMode == 2)
		ReportDetail = 0;
	else
		ReportDetail = 1;

	if (ReportDetail == 1)
	{
		ULONG FramelenTmp = FrameLen;
		RRM_InsertBcnReqRepDetailSubIE(pAd, (pOutBuffer+FrameLen), &FrameLen, 1);
		TotalLen += (FrameLen - FramelenTmp);

		FramelenTmp = FrameLen;
		RRM_InsertRequestIE(pAd, (pOutBuffer+FrameLen), &FrameLen);
		TotalLen += (FrameLen - FramelenTmp);
	}
	else
	{
		ULONG FramelenTmp = FrameLen;
		RRM_InsertBcnReqRepDetailSubIE(pAd, (pOutBuffer+FrameLen), &FrameLen, 0);		
		TotalLen += (FrameLen - FramelenTmp);
	}


	/* Insert Action header here. */
	{
		ULONG tmpLen = sizeof(HEADER_802_11);;
		MeasureReqMode.word = 0;
		MakeMeasurementReqFrame(pAd, pOutBuffer, &tmpLen,
			TotalLen, CATEGORY_RM, RRM_MEASURE_REQ, MeasureReqToken,
			MeasureReqMode.word, MeasureReqType, 1);
	}

	MeasureReqInsert(pAd, MeasureReqToken);

	MiniportMMRequest(pAd, QID_AC_BE, pOutBuffer, FrameLen);

	if (pOutBuffer)
		MlmeFreeMemory(pAd, pOutBuffer);

	return;
}

#ifdef CONFIG_STA_SUPPORT
VOID RRM_EnqueueNeighborReq(
	IN PRTMP_ADAPTER pAd,
	IN PUINT8 pDA,
	IN PUINT8 pSsid,
	IN UINT8 SsidLen)
{
	HEADER_802_11 ActHdr;
	PUCHAR pOutBuffer = NULL;
	NDIS_STATUS NStatus;
	ULONG FrameLen;
	UINT8 DialogToken = RandomByte(pAd);

	NStatus = MlmeAllocateMemory(pAd, (PVOID)&pOutBuffer);  /*Get an unused nonpaged memory */
	if(NStatus != NDIS_STATUS_SUCCESS)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("%s() allocate memory failed \n", __FUNCTION__));
		return;
	}

	/* build action frame header. */
	MgtMacHeaderInit(pAd, &ActHdr, SUBTYPE_ACTION, 0, pDA,
						pAd->MlmeAux.Bssid);

	NdisMoveMemory(pOutBuffer, (PCHAR)&ActHdr, sizeof(HEADER_802_11));
	FrameLen = sizeof(HEADER_802_11);
	
	/* fill Dialog Token */
	InsertDialogToken(pAd, (pOutBuffer + FrameLen), &FrameLen, DialogToken);

	MiniportMMRequest(pAd, QID_AC_BE, pOutBuffer, FrameLen);

	if (pOutBuffer)
		MlmeFreeMemory(pAd, pOutBuffer);

	return;
}
#endif /* CONFIG_STA_SUPPORT */

VOID RRM_EnqueueNeighborRep(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY pEntry,
	IN UINT8 DialogToken,
	IN PCHAR pSsid,
	IN UINT8 SsidLen)
{
#define MIN(_x, _y) ((_x) > (_y) ? (_x) : (_y))
	INT loop;
	HEADER_802_11 ActHdr;
	PUCHAR pOutBuffer = NULL;
	NDIS_STATUS NStatus;
	ULONG FrameLen;
	ULONG PktLen;

	if ((pEntry == NULL) || (pEntry->apidx >= pAd->ApCfg.BssidNum))
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s: Invalid STA.\n", __FUNCTION__));
		return;
	}

	NStatus = MlmeAllocateMemory(pAd, (PVOID)&pOutBuffer);  /*Get an unused nonpaged memory */
	if(NStatus != NDIS_STATUS_SUCCESS)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("%s() allocate memory failed \n", __FUNCTION__));
		return;
	}

	/* build action frame header. */
	MgtMacHeaderInit(pAd, &ActHdr, SUBTYPE_ACTION, 0, pEntry->Addr,
						pAd->ApCfg.MBSSID[pEntry->apidx].wdev.if_addr,
						pAd->ApCfg.MBSSID[pEntry->apidx].wdev.bssid);

	NdisMoveMemory(pOutBuffer, (PCHAR)&ActHdr, sizeof(HEADER_802_11));
	FrameLen = sizeof(HEADER_802_11);

	InsertActField(pAd, (pOutBuffer + FrameLen), &FrameLen,
		CATEGORY_RM, RRM_NEIGHTBOR_RSP);
	
	/* fill Dialog Token */
	InsertDialogToken(pAd, (pOutBuffer + FrameLen), &FrameLen, DialogToken);

#ifdef AP_SCAN_SUPPORT
	/* insert NeighborRep IE. */
	for (loop = 0; loop < pAd->ScanTab.BssNr; loop++)
	{
		UINT8 BssMatch = FALSE;
		BSS_ENTRY *pBssEntry = &pAd->ScanTab.BssEntry[loop];

		/* Discards all remain Bss if the packet length exceed packet buffer size. */
		PktLen = FrameLen + sizeof(RRM_NEIGHBOR_REP_INFO)
				+ (pAd->ApCfg.MBSSID[pEntry->apidx].RrmCfg.bDot11kRRMNeighborRepTSFEnable == TRUE ? 6 : 0);
		if (PktLen >= MGMT_DMA_BUFFER_SIZE)
			break;

		if (SsidLen != 0)
			BssMatch = RTMPEqualMemory(pBssEntry->Ssid, pSsid,
								MIN(SsidLen, pBssEntry->SsidLen));
		else
			BssMatch = TRUE;

		if (BssMatch)
		{
			RRM_BSSID_INFO BssidInfo;
			BssidInfo.word = 0;
			BssidInfo.field.APReachAble = 3;
			BssidInfo.field.Security = 0; /* rrm to do. */
			BssidInfo.field.KeyScope = 0; /* "report AP has same authenticator as the AP. */
			BssidInfo.field.SepctrumMng = (pBssEntry->CapabilityInfo & (1 << 8))?1:0;
			BssidInfo.field.Qos = (pBssEntry->CapabilityInfo & (1 << 9))?1:0;
			BssidInfo.field.APSD = (pBssEntry->CapabilityInfo & (1 << 11))?1:0;
			BssidInfo.field.RRM = (pBssEntry->CapabilityInfo & RRM_CAP_BIT)?1:0;
			BssidInfo.field.DelayBlockAck = (pBssEntry->CapabilityInfo & (1 << 14))?1:0;
			BssidInfo.field.ImmediateBA = (pBssEntry->CapabilityInfo & (1 << 15))?1:0;

/* 
	reference 2012 spec.
	802.11-2012.pdf
	page#581 (0 is not euqal to no security )
	The Security bit, if 1, indicates that the AP identified by this BSSID supports the same security provisioning
	as used by the STA in its current association. If the bit is 0, it indicates either that the AP does not support
	the same security provisioning or that the security information is not available at this time.
*/

			
			BssidInfo.field.KeyScope = 0; /* "report AP has same authenticator as the AP. */
/*
	reference 2012 spec.
	802.11-2012.pdf
	page#582 (0 means information is not available  )
	The Key Scope bit, when set, indicates the AP indicated by this BSSID has the same authenticator as the AP
	sending the report. If this bit is 0, it indicates a distinct authenticator or the information is not available.
*/

			BssidInfo.field.SepctrumMng = (pBssEntry->CapabilityInfo & (1 << 8))?1:0;
			BssidInfo.field.Qos = (pBssEntry->CapabilityInfo & (1 << 9))?1:0;
			BssidInfo.field.APSD = (pBssEntry->CapabilityInfo & (1 << 11))?1:0;
			BssidInfo.field.RRM = (pBssEntry->CapabilityInfo & RRM_CAP_BIT)?1:0;
			BssidInfo.field.DelayBlockAck = (pBssEntry->CapabilityInfo & (1 << 14))?1:0;
			BssidInfo.field.ImmediateBA = (pBssEntry->CapabilityInfo & (1 << 15))?1:0;


			BssidInfo.field.MobilityDomain = (pBssEntry->bHasMDIE )?1:0;
			BssidInfo.field.HT = (pBssEntry->HtCapabilityLen != 0)?1:0;
#ifdef DOT11_VHT_AC			
			BssidInfo.field.VHT = (pBssEntry->vht_cap_len != 0)?1:0;
#endif /* DOT11_VHT_AC */

			/*
			reference spec:
			dot11FrameRprtPhyType OBJECT-TYPE
			SYNTAX INTEGER {
			fhss(1),
			dsss(2),
			irbaseband(3),
			ofdm(4),
			hrdsss(5),
			erp(6),
			ht(7),
			vht(9)
			}

			*/
			
			if (pBssEntry->Channel > 14) // 5G case
			{
				if (pBssEntry->HtCapabilityLen != 0) // HT or Higher case
				{
#ifdef DOT11_VHT_AC				
					if (pBssEntry->vht_cap_len != 0)
						pBssEntry->CondensedPhyType = 9;
					else
#endif /* DOT11_VHT_AC */
						pBssEntry->CondensedPhyType = 7;
				}
				else // OFDM case
				{
					pBssEntry->CondensedPhyType = 4;
				}
			}
			else // 2.4G case
			{

				if (pBssEntry->HtCapabilityLen != 0) //HT case
					pBssEntry->CondensedPhyType = 7;
				else if (ERP_IS_NON_ERP_PRESENT(pBssEntry->Erp)) //ERP case
					pBssEntry->CondensedPhyType = 6;
				else if (pBssEntry->SupRateLen > 4)// OFDM case (1,2,5.5,11 for CCK 4 Rates)
					pBssEntry->CondensedPhyType = 4;

				/* no CCK's definition in spec. */
			}

			RRM_InsertNeighborRepIE(pAd, (pOutBuffer + FrameLen), &FrameLen,
				sizeof(RRM_NEIGHBOR_REP_INFO), pBssEntry->Bssid,
				BssidInfo, pBssEntry->RegulatoryClass,
				pBssEntry->Channel, pBssEntry->CondensedPhyType);
		}

		/*
			shall insert Neighbor Report TSF offset
			when the MIB attribute
			dot11RRMNeighborReportTSFOffsetEnabled is true.
		*/
		if (pAd->ApCfg.MBSSID[pEntry->apidx].RrmCfg.bDot11kRRMNeighborRepTSFEnable)
		{
			UINT32 Ttfs = (UINT32)(pBssEntry->TTSF[3] << 24)
					+ (UINT32)(pBssEntry->TTSF[2] << 16)
					+ (UINT32)(pBssEntry->TTSF[1] << 8)
					+ (UINT32)(pBssEntry->TTSF[0]);

			UINT32 Ptfs = (UINT32)(pBssEntry->PTSF[3] << 24)
					+ (UINT32)(pBssEntry->PTSF[2] << 16)
					+ (UINT32)(pBssEntry->PTSF[1] << 8)
					+ (UINT32)(pBssEntry->PTSF[0]);

			RRM_InsertNeighborTSFOffsetSubIE(pAd, (pOutBuffer + FrameLen),
						&FrameLen, cpu2le16((UINT16)ABS(Ttfs, Ptfs)),
						pBssEntry->BeaconPeriod);
		}
	}
#endif /* AP_SCAN_SUPPORT */
	MiniportMMRequest(pAd, QID_AC_BE, pOutBuffer, FrameLen);

	if (pOutBuffer)
		MlmeFreeMemory(pAd, pOutBuffer);

	return;
}

VOID RRM_EnqueueLinkMeasureReq(
	IN PRTMP_ADAPTER pAd,
	IN UINT8 Aid,
	IN UINT8 apidx)
{
	UINT8 DialogToken = RandomByte(pAd);
	HEADER_802_11 ActHdr;
	PUCHAR pOutBuffer = NULL;
	NDIS_STATUS NStatus;
	ULONG FrameLen;
	PMAC_TABLE_ENTRY pEntry;

	if ((apidx >= pAd->ApCfg.BssidNum)
		|| (Aid >= MAX_LEN_OF_MAC_TABLE))
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s: Invalid STA. apidx=%d Aid=%d\n",
			__FUNCTION__, apidx, Aid));
		return;
	}

	NStatus = MlmeAllocateMemory(pAd, (PVOID)&pOutBuffer);  /* Get an unused nonpaged memory */
	if(NStatus != NDIS_STATUS_SUCCESS)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("%s() allocate memory failed \n", __FUNCTION__));
		return;
	}

	pEntry = &pAd->MacTab.Content[Aid];

	/* build action frame header. */
	MgtMacHeaderInit(pAd, &ActHdr, SUBTYPE_ACTION, 0, pEntry->Addr,
						pAd->ApCfg.MBSSID[apidx].wdev.if_addr,
						pAd->ApCfg.MBSSID[apidx].wdev.bssid);

	NdisMoveMemory(pOutBuffer, (PCHAR)&ActHdr, sizeof(HEADER_802_11));
	FrameLen = sizeof(HEADER_802_11);

	InsertActField(pAd, (pOutBuffer + FrameLen), &FrameLen,
		CATEGORY_RM, RRM_LNK_MEASURE_REQ);
	
	/* fill Dialog Token */
	InsertDialogToken(pAd, (pOutBuffer + FrameLen), &FrameLen, DialogToken);

	/* fill Tx Power Used field */
	{
		ULONG TempLen;
		UINT8 TxPwr = RTMP_GetTxPwr(pAd, pAd->CommonCfg.MlmeTransmit);
		MakeOutgoingFrame(pOutBuffer + FrameLen,	&TempLen,
						1,							&TxPwr,
						END_OF_ARGS);

		FrameLen += TempLen;
	}
	

	/* fill Max Tx Power field */
	{
		ULONG TempLen;
		UINT8 MaxTxPwr = GetCuntryMaxTxPwr(pAd, pAd->CommonCfg.Channel);
		MakeOutgoingFrame(pOutBuffer + FrameLen,	&TempLen,
						1,							&MaxTxPwr,
						END_OF_ARGS);

		FrameLen += TempLen;
	}


	MeasureReqInsert(pAd, DialogToken);

	MiniportMMRequest(pAd, QID_AC_BE, pOutBuffer, FrameLen);

	if (pOutBuffer)
		MlmeFreeMemory(pAd, pOutBuffer);

	return;
}

VOID RRM_EnqueueTxStreamMeasureReq(
	IN PRTMP_ADAPTER pAd,
	IN UINT8 Aid,
	IN UINT8 apidx,
	IN PRRM_MLME_TRANSMIT_REQ_INFO pMlmeTxMeasureReq)
{
	UINT8 MeasureReqType = RRM_MEASURE_SUBTYPE_TX_STREAM;
	MEASURE_REQ_MODE MeasureReqMode;
	UINT8 MeasureReqToken = RandomByte(pAd);
	RRM_TRANSMIT_MEASURE_INFO TxMeasureReq;
	RRM_TRANSMIT_MEASURE_TRIGGER_REPORT TriggerReport;
	UINT8 TotalLen;

	HEADER_802_11 ActHdr;
	PUCHAR pOutBuffer = NULL;
	NDIS_STATUS NStatus;
	ULONG FrameLen;

	NStatus = MlmeAllocateMemory(pAd, (PVOID)&pOutBuffer);  /*Get an unused nonpaged memory */
	if(NStatus != NDIS_STATUS_SUCCESS)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("%s() allocate memory failed \n", __FUNCTION__));
		return;
	}

	MeasureReqMode.word = 0;

	/* build action frame header. */
	MgtMacHeaderInit(pAd, &ActHdr, SUBTYPE_ACTION, 0, pAd->MacTab.Content[Aid].Addr,
						pAd->ApCfg.MBSSID[apidx].wdev.if_addr,
						pAd->ApCfg.MBSSID[apidx].wdev.bssid);

	NdisMoveMemory(pOutBuffer, (PCHAR)&ActHdr, sizeof(HEADER_802_11));
	FrameLen = sizeof(HEADER_802_11);

	/*
		Action header has a field to indicate total length of packet
		but the total length is unknow untial whole packet completd.
		So skip the action here and fill it late.
		1. skip Catgore (1 octect), Action(1 octect).
		2. skip dailog token (1 octect).
		3. skip Num Of Repetitions field (2 octects)
		3. skip MeasureReqIE (2 + sizeof(MEASURE_REQ_INFO)).
	*/
	FrameLen += (7 + sizeof(MEASURE_REQ_INFO));

	TotalLen = sizeof(MEASURE_REQ_INFO);

	/*
		Insert Tx stream Measure Req IE.
	*/

	/* according to WiFi Voice-enterprise testing req. the RandomInterval shall be zero. */
	TxMeasureReq.RandomInterval = 0;
	TxMeasureReq.MeasureDuration = cpu2le16(pMlmeTxMeasureReq->MeasureDuration);
	COPY_MAC_ADDR(TxMeasureReq.PeerStaMac, pAd->MacTab.Content[Aid].Addr);
	TxMeasureReq.TIDField.Rev = 0;
	TxMeasureReq.TIDField.TID = pMlmeTxMeasureReq->Tid;
	TxMeasureReq.Bin0Range = pMlmeTxMeasureReq->BinRange;

	RRM_InsertTxStreamReqIE(pAd, (pOutBuffer+FrameLen),
		&FrameLen, (PUCHAR)&TxMeasureReq);

	TotalLen += sizeof(RRM_TRANSMIT_MEASURE_INFO);

	/* inssert Trigger report sub field. */
	if (pMlmeTxMeasureReq->bTriggerReport == 1)
	{
		PRRM_TRANSMIT_MEASURE_TRIGGER_CONDITION pTiggerCon
			= (PRRM_TRANSMIT_MEASURE_TRIGGER_CONDITION)&TriggerReport.TriggerCondition;
	
		pTiggerCon->field.Average = pMlmeTxMeasureReq->ArvCondition;
		pTiggerCon->field.Consecutive = pMlmeTxMeasureReq->ConsecutiveCondition;
		pTiggerCon->field.Delay = pMlmeTxMeasureReq->DelayCondition;
		if (pTiggerCon->field.Average)
			TriggerReport.AvrErrorThreshold = pMlmeTxMeasureReq->AvrErrorThreshold;
		if (pTiggerCon->field.Consecutive)	
			TriggerReport.ConsecutiveErrorThreshold = pMlmeTxMeasureReq->ConsecutiveErrorThreshold;
		if (pTiggerCon->field.Delay)
			TriggerReport.DelayThreshold = pMlmeTxMeasureReq->DelayThreshold;

		TriggerReport.TriggerTimeout = pMlmeTxMeasureReq->TriggerTimeout;
		TriggerReport.MeasurementCnt = pMlmeTxMeasureReq->MeasureCnt;
		RRM_InsertTxStreamReqTriggerReportSubIE(pAd, (pOutBuffer+FrameLen),
			&FrameLen, (PUCHAR)&TriggerReport);

		TotalLen +=
			(sizeof(RRM_TRANSMIT_MEASURE_TRIGGER_REPORT) + 2);

		MeasureReqMode.field.Report = 1;
	}

	/* Insert Action header here. */
	{
		ULONG tmpLen = sizeof(HEADER_802_11);;
		MeasureReqMode.field.Enable = 1;
		MeasureReqMode.field.DurationMandatory =
			pMlmeTxMeasureReq->bDurationMandatory;
		MakeMeasurementReqFrame(pAd, pOutBuffer, &tmpLen,
			TotalLen, CATEGORY_RM, RRM_MEASURE_REQ, MeasureReqToken,
			MeasureReqMode.word, MeasureReqType, 0xffff);
	}

	MeasureReqInsert(pAd, MeasureReqToken);

	MiniportMMRequest(pAd, QID_AC_BE, pOutBuffer, FrameLen);

	if (pOutBuffer)
		MlmeFreeMemory(pAd, pOutBuffer);

	return;
}

#endif /* DOT11K_RRM_SUPPORT */


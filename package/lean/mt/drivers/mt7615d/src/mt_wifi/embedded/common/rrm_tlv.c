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
		MakeOutgoingFrame((_FRAMEBUFER),	&(_TempLen), \
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

	MakeOutgoingFrame(pFrameBuf,		&TempLen,
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

	MakeOutgoingFrame(pFrameBuf,		&TempLen,
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

	MakeOutgoingFrame(pFrameBuf,		&TempLen,
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
	PRRM_EN_CAP_IE pRrmEnCap;

	BSS_STRUCT *pMBss = NULL;

	if (BssIdx < MAX_MBSSID_NUM(pAd))
		pMBss = &pAd->ApCfg.MBSSID[BssIdx];
	else
		return;
	pRrmEnCap = (PRRM_EN_CAP_IE)&pMBss->wdev.RrmCfg.rrm_capabilities;

	MakeOutgoingFrame(pFrameBuf,					&TempLen,
					  1,							&ElementID,
					  1,							&Len,
					  Len,						(PUCHAR)&pRrmEnCap->word,
					  END_OF_ARGS);
	*pFrameLen = *pFrameLen + TempLen;
	return;
}

VOID RRM_InsertNeighborRepIE(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN UINT8 Len,
	IN RRM_PNEIGHBOR_REP_INFO pNeighborRepInfo)
{
	ULONG TempLen;
	UINT8 IEId = IE_RRM_NEIGHBOR_REP;

	MakeOutgoingFrame(pFrameBuf,						&TempLen,
						1,								&IEId,
						1,								&Len,
						sizeof(RRM_NEIGHBOR_REP_INFO),	pNeighborRepInfo,
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

	MakeOutgoingFrame(pFrameBuf,		&TempLen,
						1,				&SubId,
						1,				&Len,
						2,				&TSFOffset,
						2,				&BcnInterval,
						END_OF_ARGS);
	*pFrameLen = *pFrameLen + TempLen;
}

#ifdef FTM_SUPPORT
VOID RRM_InsertNeighborMsmtRptLocationLCISubIE(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN	PRRM_CONFIG pRrmCfg,
	IN PMSMT_RPT_SUBELEMENT pLciHdr,
	IN PUSAGE_SUBELEMENT pLciUsage,
	IN PZ_ELEMENT pLciZ,
	IN PLCI_FIELD pLci,
	IN BOOLEAN bSetZRpt
)
{
	MEASUREMENT_REPORT LciRpt;
	ULONG				FrameLen = 0;
	ULONG				TmpLen = 0;
	PMEASURE_REPORT_MODE pRptMode = NULL;

	NdisZeroMemory(&LciRpt, sizeof(MEASUREMENT_REPORT));
	LciRpt.ID = IE_MEASUREMENT_REPORT;
	LciRpt.Length = sizeof(MEASUREMENT_REPORT) - 2;
	LciRpt.Token = pRrmCfg->PeerMeasurementToken;
	pRptMode = (PMEASURE_REPORT_MODE)&LciRpt.ReportMode;
	pRptMode->field.Late = 0;
	pRptMode->field.Incapable = 0;
	pRptMode->field.Refused = 0;
	LciRpt.Type = MSMT_LCI_RPT;
	/* 0. LCI */
	LciRpt.Length += (sizeof(MSMT_RPT_SUBELEMENT) + pLciHdr->Length);

	/* 4. Z element */
	if (bSetZRpt) {
		LciRpt.Length += (pLciZ->Length + 2);
		pLciZ->Floor.field.ExpectedToMove = 0;
	}

	/* 6. Usage Rules/Policy */
	pLciUsage->SubElement = LCI_RPTID_USAGE_RULES;
	pLciUsage->RulesAndPolicy.field.RetransAllowed = 1;
	pLciUsage->RulesAndPolicy.field.RetExpiresPresent = 0;
	pLciUsage->RulesAndPolicy.field.LocationPolicy = 0;
	pLciUsage->Length = pLciUsage->RulesAndPolicy.field.RetExpiresPresent ? \
						(sizeof(USAGE_SUBELEMENT) - 2) :
						(sizeof(USAGE_SUBELEMENT) - 2 \
						 - sizeof(pLciUsage->RetExpires));
	LciRpt.Length += (pLciUsage->Length + 2);
	MakeOutgoingFrame(pFrameBuf, &FrameLen,
					  sizeof(MEASUREMENT_REPORT), &LciRpt,
					  sizeof(MSMT_RPT_SUBELEMENT), pLciHdr,
					  pLciHdr->Length, pLci,
					  END_OF_ARGS);

	if (bSetZRpt) {
		MakeOutgoingFrame(pFrameBuf + FrameLen, &TmpLen,
						  sizeof(Z_ELEMENT), pLciZ,
						  END_OF_ARGS);
		FrameLen += TmpLen;
	}

	MakeOutgoingFrame(pFrameBuf + FrameLen, &TmpLen,
					  (pLciUsage->Length + 2), pLciUsage,
					  END_OF_ARGS);
	FrameLen += TmpLen;
	*pFrameLen = *pFrameLen + FrameLen;
	return;
}
VOID RRM_InsertNeighborMsmtRptLocationCIVICSubIE(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN	PRRM_CONFIG pRrmCfg,
	IN  PMSMT_RPT_SUBELEMENT pCivicHdr,
	IN  PLOCATION_CIVIC pCivic,
	IN	UINT8 *pCA_Value
)
{
	MEASUREMENT_REPORT	CivicRpt;
	UINT8				CivicLocationType;
	ULONG				FrameLen = 0;
	ULONG				TmpLen = 0;
	PMEASURE_REPORT_MODE pRptMode = NULL;

	NdisZeroMemory(&CivicRpt, sizeof(MEASUREMENT_REPORT));
	CivicRpt.ID = IE_MEASUREMENT_REPORT;
	CivicRpt.Length = sizeof(MEASUREMENT_REPORT) - 2;
	CivicRpt.Token = pRrmCfg->PeerMeasurementToken;
	pRptMode = (PMEASURE_REPORT_MODE)&CivicRpt.ReportMode;
	pRptMode->field.Late = 0;
	pRptMode->field.Incapable = 0;
	pRptMode->field.Refused = 0;
	CivicRpt.Type = MSMT_LOCATION_CIVIC_RPT;
	/* Location Civic Report */
	CivicLocationType = CIVIC_TYPE_IETF_RFC4776_2006;
	CivicRpt.Length += sizeof(CivicLocationType);
	CivicRpt.Length += (sizeof(MSMT_RPT_SUBELEMENT) + pCivicHdr->Length);
	MakeOutgoingFrame(pFrameBuf, &FrameLen,
					  sizeof(MEASUREMENT_REPORT), &CivicRpt,
					  sizeof(CivicLocationType), &CivicLocationType,
					  sizeof(MSMT_RPT_SUBELEMENT), pCivicHdr,
					  END_OF_ARGS);

	if (pCivicHdr->Length) {
		MakeOutgoingFrame(pFrameBuf + FrameLen, &TmpLen,
						  sizeof(LOCATION_CIVIC), pCivic,
						  pCivic->CA_Length, pCA_Value,
						  END_OF_ARGS);
		FrameLen += TmpLen;
	}

	*pFrameLen = *pFrameLen + FrameLen;
	return;
}

#endif/* FTM_SUPPORT */

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
	MakeOutgoingFrame(pFrameBuf,		&TempLen,
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
	MakeOutgoingFrame(pFrameBuf,		&TempLen,
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

	/* cacule the total length of the IE. */
	Len = 2;

	for (idx = 0; idx < 12; idx++) {
		if (pAcBitMap->word & (0x1 << idx))
			Len += 2;
	}

#ifdef RT_BIG_ENDIAN
		pAcBitMap->word = cpu2le16(pAcBitMap->word);
#endif

	MakeOutgoingFrame(pFrameBuf,							&TempLen,
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
	OUT PULONG pFrameLen,
	IN  UINT8 ie_num,
	IN  PUINT8 ie_list)
{
	ULONG TempLen;
	UINT8 IEId = IE_802_11D_REQUEST;
	UINT8 Len;

	Len = ie_num;
	MakeOutgoingFrame(pFrameBuf,		&TempLen,
						1,				&IEId,
						1,				&Len,
						Len,			ie_list,
						END_OF_ARGS);
	*pFrameLen = *pFrameLen + TempLen;
	return;
}

VOID RRM_InsertRequestIE_11KV_API(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN PUCHAR pRequest,
	IN UINT8 RequestLen)
{
	ULONG TempLen = 0;
	UINT8 IEId = IE_802_11D_REQUEST;
	UINT8 Len = 0;

	Len = RequestLen;

	MakeOutgoingFrame(pFrameBuf,		&TempLen,
						1,				&IEId,
						1,				&Len,
						Len,			pRequest,
						END_OF_ARGS);

	*pFrameLen = *pFrameLen + TempLen;
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

	MakeOutgoingFrame(pFrameBuf,		&TempLen,
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

	if (NStatus != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_TRACE, ("%s() allocate memory failed\n", __func__));
		return;
	}

	/* build action frame header. */
	if (Aid) {
	MgtMacHeaderInit(pAd, &ActHdr, SUBTYPE_ACTION, 0, pAd->MacTab.Content[Aid].Addr,
					 pAd->ApCfg.MBSSID[IfIdx].wdev.if_addr,
					 pAd->ApCfg.MBSSID[IfIdx].wdev.bssid);
	} else {
		MgtMacHeaderInit(pAd, &ActHdr, SUBTYPE_ACTION, 0, pMlmeBcnReq->Addr,
			pAd->ApCfg.MBSSID[IfIdx].wdev.if_addr,
			pAd->ApCfg.MBSSID[IfIdx].wdev.bssid);
	}

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
	BcnReq.RandomInterval = cpu2le16(pMlmeBcnReq->RandInt);
	BcnReq.MeasureDuration = cpu2le16(pMlmeBcnReq->MeasureDuration);
	BcnReq.MeasureMode = pMlmeBcnReq->MeasureMode;
	COPY_MAC_ADDR(BcnReq.Bssid, pMlmeBcnReq->Bssid);
	RRM_InsertBcnReqIE(pAd, (pOutBuffer + FrameLen),
					   &FrameLen, (PUCHAR)&BcnReq);
	TotalLen += sizeof(RRM_BEACON_REQ_INFO);
	/* inssert SSID sub field. */
	/* Fix Voice Enterprise : Item V-E-4.3, case2 still need to include the SSID sub filed even SsidLen is 0 */
	/* if (pMlmeBcnReq->SsidLen != 0) */
	{
		RRM_InsertBcnReqSsidSubIE(pAd, (pOutBuffer + FrameLen),
								  &FrameLen, (PUCHAR)pMlmeBcnReq->pSsid, pMlmeBcnReq->SsidLen);
		TotalLen += (pMlmeBcnReq->SsidLen + 2); /* SSID sub field. */
	}

	/* inssert report condition sub field. */
	if (pMlmeBcnReq->BcnReqCapFlag.field.ReportCondition == TRUE) {
		RRM_InsertBcnReqRepCndSubIE(pAd, (pOutBuffer + FrameLen), &FrameLen, 0, 0);
		TotalLen += (2 + 2); /* SSID sub field. */
	}

	/* inssert channel report sub field. */
	if (pMlmeBcnReq->BcnReqCapFlag.field.ChannelRep == TRUE) {
		ULONG idx;

		idx = 0;

		while (pMlmeBcnReq->ChRepRegulatoryClass[idx] != 0) {
			ULONG FramelenTmp = FrameLen;

			InsertChannelRepIE(pAd, (pOutBuffer + FrameLen), &FrameLen,
							   (RTMP_STRING *)pAd->CommonCfg.CountryCode,
							   pMlmeBcnReq->ChRepRegulatoryClass[idx],
							   &pMlmeBcnReq->ChRepList[0],
							   pAd->ApCfg.MBSSID[IfIdx].wdev.PhyMode);
			TotalLen += (FrameLen - FramelenTmp);
			idx++;
		}
	}

	/* inssert report detail sub field. */
	ReportDetail = pMlmeBcnReq->report_detail;
	if (BcnReq.MeasureMode == RRM_BCN_REQ_MODE_BCNTAB)
		ReportDetail = 0; /*force report detail to 0 when it is table mode*/
	{   /*insert report detail sub ie*/
		ULONG FramelenTmp = FrameLen;
		RRM_InsertBcnReqRepDetailSubIE(pAd, (pOutBuffer + FrameLen), &FrameLen, ReportDetail);
		TotalLen += (FrameLen - FramelenTmp);
	}

	if (ReportDetail == 1) {
		/*if report detail is 1, insert request ie list*/
		ULONG FramelenTmp = FrameLen;
		RRM_InsertRequestIE(pAd, (pOutBuffer + FrameLen), &FrameLen, pMlmeBcnReq->request_ie_num, pMlmeBcnReq->request_ie);
		TotalLen += (FrameLen - FramelenTmp);
	}

	{
		/* Adjust TotalLen of the Measurement Req while inserting
		 * Bcn Report Indication*/
		ULONG FramelenTmp = FrameLen;
		InsertBcnReportIndicationReqIE(pAd, (pOutBuffer + FrameLen), &FrameLen, 1);
		TotalLen += (FrameLen - FramelenTmp);
	}
	/* Insert Action header here. */
	{
		ULONG tmpLen = sizeof(HEADER_802_11);

		MeasureReqMode.word = 0;
		MeasureReqMode.field.DurationMandatory = 0;
		MeasureReqMode.field.Report = 0;
		MeasureReqMode.field.Request = 0;
		MeasureReqMode.field.Enable = 0;
		MeasureReqMode.field.Parallel = 0;

		MakeMeasurementReqFrame(pAd, pOutBuffer, &tmpLen,
								TotalLen, CATEGORY_RM, RRM_MEASURE_REQ, MeasureReqToken,
								MeasureReqMode.word, MeasureReqType, 0);
	}
	MeasureReqInsert(pAd, MeasureReqToken);
	MiniportMMRequest(pAd, (MGMT_USE_QUEUE_FLAG | QID_AC_BE), pOutBuffer, FrameLen);

	if (pOutBuffer)
		MlmeFreeMemory(pOutBuffer);

	return;
}

void RRM_measurement_report_to_host(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem);

int RRM_EnqueueBcnReqAction(
	IN PRTMP_ADAPTER pAd,
	IN UINT8 IfIdx,
	IN p_bcn_req_data_t p_bcn_req_data)
{

	HEADER_802_11 ActHdr;
	PUCHAR pOutBuffer = NULL;
	NDIS_STATUS NStatus;
	ULONG FrameLen;
	UINT unicast_peer = 0, count = 0;
	MAC_TABLE_ENTRY *pEntry;
	UINT8 mode;

	mode = p_bcn_req_data->bcn_req[RRM_BCNREQ_MODE_OFFSET];
	if (mode > 0x2) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_OFF, ("Incorrect mode\n"));
		return NDIS_STATUS_FAILURE;
	}
	while (count < MAC_ADDR_LEN) {
		if (p_bcn_req_data->peer_address[count++] != 0xff) {
			unicast_peer = 1;
			break;
		}
	}

	if (unicast_peer) {
		pEntry = MacTableLookup(pAd, p_bcn_req_data->peer_address);
		if (!pEntry || (pEntry->Sst != SST_ASSOC)) {
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_OFF, ("Peer Not connected\n"));
			return NDIS_STATUS_FAILURE;
		}

		if ((pEntry->func_tb_idx == IfIdx)
			&& IS_RRM_ENABLE(pEntry->wdev)
			&& ((pEntry->RrmEnCap.word >> 0x4) & (1 << mode))) {

		} else {
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_OFF, ("Peer does not support this request\n"));
			return NDIS_STATUS_FAILURE;
		}

	}

	NStatus = MlmeAllocateMemory(pAd, (PVOID)&pOutBuffer);  /*Get an unused nonpaged memory */
	if (NStatus != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_TRACE, ("%s() allocate memory failed\n", __func__));
		return NDIS_STATUS_FAILURE;
	}

	/* build action frame header. */
	MgtMacHeaderInit(pAd, &ActHdr, SUBTYPE_ACTION, 0, p_bcn_req_data->peer_address,
		pAd->ApCfg.MBSSID[IfIdx].wdev.if_addr,
		pAd->ApCfg.MBSSID[IfIdx].wdev.bssid);

	NdisMoveMemory(pOutBuffer, (PCHAR)&ActHdr, sizeof(HEADER_802_11));
	FrameLen = sizeof(HEADER_802_11);

	InsertActField(pAd, (pOutBuffer + FrameLen), &FrameLen, MT2_PEER_RM_CATE, RRM_MEASURE_REQ);

	/* fill Dialog Token*/
	InsertDialogToken(pAd, (pOutBuffer + FrameLen), &FrameLen, p_bcn_req_data->dialog_token);
	RTMP_INSERT_IE(pOutBuffer + FrameLen, &FrameLen, p_bcn_req_data->bcn_req_len, p_bcn_req_data->bcn_req);
	NStatus = MiniportMMRequest(pAd, MGMT_USE_PS_FLAG | QID_AC_BE, pOutBuffer, FrameLen);

	if (pOutBuffer)
		MlmeFreeMemory(pOutBuffer);

	return NStatus;
}

#ifdef CONFIG_11KV_API_SUPPORT
INT rrm_send_beacon_req_param(
	IN PRTMP_ADAPTER pAd,
	IN p_bcn_req_info pBcnReq,
	IN UINT32 BcnReqLen)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	BSS_STRUCT *pMbss = NULL;
	PMEASURE_REQ_ENTRY pEntry = NULL;
	PBCN_EVENT_DATA Event = NULL;
	UINT32 Len = 0;
	NDIS_STATUS NStatus = NDIS_STATUS_SUCCESS;
	PUCHAR pBuf = NULL;
	UINT8 IfIdx = pObj->ioctl_if;
	UINT8 MeasureReqToken = 0;
	static UINT8 k;

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_TRACE,
		("%s\n", __func__));

	/* check the AP supports 11k or not */
	pMbss = &pAd->ApCfg.MBSSID[IfIdx];
	if (pMbss->wdev.RrmCfg.bDot11kRRMEnable == FALSE) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_ERROR,
			("%s() rrm off\n", __func__));
		return NDIS_STATUS_FAILURE;
	}

	if (BcnReqLen != sizeof(*pBcnReq)) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_ERROR,
			("%s, bcn req len check failed len1=%d, len2=%ld\n",
			__func__, BcnReqLen, sizeof(*pBcnReq)));
		return NDIS_STATUS_FAILURE;
	}

	/*send bcn req msg into mlme*/
	Len = sizeof(*Event) + BcnReqLen;
	os_alloc_mem(NULL, (UCHAR **)&pBuf, Len);
	if (!pBuf) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_ERROR,
			("%s, allocate event memory failed\n", __func__));
		return NDIS_STATUS_RESOURCES;
	}
	NdisZeroMemory(pBuf, Len);

	/*insert bcn req token into measure table*/
	k++;
	if (k == 0)
		k = 1;
	MeasureReqToken = k;
	pEntry = MeasureReqInsert(pAd, MeasureReqToken);
	if (!pEntry) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_ERROR,
			("%s, Fail to Insert MesureReq Token(%d)!\n",
			__func__, MeasureReqToken));
		return NDIS_STATUS_FAILURE;
	}
	pEntry->skip_time_check = TRUE;
	pEntry->CurrentState = WAIT_BCN_REQ;
	pEntry->Priv = pAd;
	pEntry->ControlIndex = IfIdx;
	COPY_MAC_ADDR(pEntry->StaMac, pBcnReq->peer_address);

	Event = (PBCN_EVENT_DATA)pBuf;
	Event->ControlIndex = IfIdx;
	Event->MeasureReqToken = MeasureReqToken;
	RTMPMoveMemory(Event->stamac, pBcnReq->peer_address, MAC_ADDR_LEN);
	Event->DataLen = BcnReqLen;
	NdisMoveMemory(Event->Data, pBcnReq, BcnReqLen);

	if (MlmeEnqueue(pAd, BCN_STATE_MACHINE, BCN_REQ, Len, pBuf, 0) == FALSE) {
		NStatus = NDIS_STATUS_FAILURE;
		MeasureReqDelete(pAd, MeasureReqToken);
	}

	/*free memory*/
	os_free_mem(pBuf);
	return NStatus;
}
#endif /* CONFIG_11KV_API_SUPPORT */


/*
	reference 2012 spec.
	802.11-2012.pdf
	page#581 (0 is not euqal to no security )
	The Security bit, if 1, indicates that the AP identified by this BSSID supports the same security provisioning
	as used by the STA in its current association. If the bit is 0, it indicates either that the AP does not support
	the same security provisioning or that the security information is not available at this time.
*/
BOOLEAN RRM_CheckBssAndStaSecurityMatch(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY pEntry,
	IN BSS_ENTRY * pBssEntry
)
{
	BOOLEAN ret = FALSE;

	if ((pBssEntry->AKMMap & pEntry->SecConfig.AKMMap)
	&& (pBssEntry->PairwiseCipher & pEntry->SecConfig.PairwiseCipher))
		ret = TRUE;
	else
		ret = FALSE;

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_TRACE,
		("%s: pBssEntry %s/%s & pSTA %s/%s = (%d/%d)  ret %d\n", __func__,
		GetAuthModeStr(pBssEntry->AKMMap), GetEncryModeStr(pBssEntry->PairwiseCipher),
		GetAuthModeStr(pEntry->SecConfig.AKMMap), GetEncryModeStr(pEntry->SecConfig.PairwiseCipher),
		(pBssEntry->AKMMap & pEntry->SecConfig.AKMMap),
		(pBssEntry->PairwiseCipher & pEntry->SecConfig.PairwiseCipher),
		ret
		));

	return ret;
}

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
	PRRM_CONFIG pRrmCfg;
	RRM_NEIGHBOR_REP_INFO NeighborRepInfo;
#ifdef FTM_SUPPORT
	UINT		idx = 0;
	PFTM_CTRL	pFtm = pAd->pFtmCtrl;
	BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[pEntry->func_tb_idx];
	RRM_BSSID_INFO BssidInfo;
	UINT8 CondensedPhyType = (pMbss->wdev.channel > 14) ? 9 : 7; /* 7:2g, 9:5g  //v0.3.0a Page 91 table 37 */
#endif /* FTM_SUPPORT */

	if ((pEntry == NULL) || (pEntry->func_tb_idx >= pAd->ApCfg.BssidNum)) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_ERROR, ("%s: Invalid STA.\n", __func__));
		return;
	}

	NStatus = MlmeAllocateMemory(pAd, (PVOID)&pOutBuffer);  /*Get an unused nonpaged memory */

	if (NStatus != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_TRACE, ("%s() allocate memory failed\n", __func__));
		return;
	}

	/* build action frame header. */
	MgtMacHeaderInit(pAd, &ActHdr, SUBTYPE_ACTION, 0, pEntry->Addr,
					 pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.if_addr,
					 pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.bssid);
	NdisMoveMemory(pOutBuffer, (PCHAR)&ActHdr, sizeof(HEADER_802_11));
	FrameLen = sizeof(HEADER_802_11);
	InsertActField(pAd, (pOutBuffer + FrameLen), &FrameLen,
				   CATEGORY_RM, RRM_NEIGHTBOR_RSP);
	/* fill Dialog Token */
	InsertDialogToken(pAd, (pOutBuffer + FrameLen), &FrameLen, DialogToken);
	pRrmCfg = &pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.RrmCfg;
#ifdef FTM_SUPPORT
	/* our own info */
	BssidInfo.word = 0;
	BssidInfo.field.APReachAble = 3;
	BssidInfo.field.Security = 1; /* security is match in our own info case */
	BssidInfo.field.KeyScope = 0; /* "report AP has same authenticator as the AP. */
	BssidInfo.field.SepctrumMng = (pMbss->CapabilityInfo & (1 << 8)) ? 1 : 0;
	BssidInfo.field.Qos = (pMbss->CapabilityInfo & (1 << 9)) ? 1 : 0;
	BssidInfo.field.APSD = (pMbss->CapabilityInfo & (1 << 11)) ? 1 : 0;
	BssidInfo.field.RRM = (pMbss->CapabilityInfo & RRM_CAP_BIT) ? 1 : 0;
	BssidInfo.field.DelayBlockAck = (pMbss->CapabilityInfo & (1 << 14)) ? 1 : 0;
	BssidInfo.field.ImmediateBA = (pMbss->CapabilityInfo & (1 << 15)) ? 1 : 0;
	BssidInfo.field.FTM = 1;
	COPY_MAC_ADDR(NeighborRepInfo.Bssid, pMbss->wdev.bssid);
	NeighborRepInfo.BssidInfo = BssidInfo.word;
	NeighborRepInfo.RegulatoryClass = get_regulatory_class(pAd, pMbss->wdev.channel, pMbss->wdev.PhyMode, &pMbss->wdev);
	NeighborRepInfo.ChNum = pMbss->wdev.channel;
	NeighborRepInfo.PhyType = CondensedPhyType;
	RRM_InsertNeighborRepIE(pAd, (pOutBuffer + FrameLen), &FrameLen,
							sizeof(RRM_NEIGHBOR_REP_INFO), &NeighborRepInfo);

	if (pRrmCfg->bPeerReqLCI) {
		RRM_InsertNeighborMsmtRptLocationLCISubIE(pAd, (pOutBuffer + FrameLen),
				&FrameLen, pRrmCfg, &pFtm->LciHdr, &pFtm->LciUsage, &pFtm->LciZ,
				&pFtm->LciField, pFtm->bSetZRpt);
	}

	if (pRrmCfg->bPeerReqCIVIC) {
		RRM_InsertNeighborMsmtRptLocationCIVICSubIE(pAd, (pOutBuffer + FrameLen),
				&FrameLen, pRrmCfg, &pFtm->CivicHdr, &pFtm->Civic, pFtm->CA_Value);
	}

	/* neighbor info */
	for (idx = 0; idx < MAX_FTM_TBL_SIZE; idx++) {
		PFTM_NEIGHBORS pFtmNeighbor = &pAd->pFtmCtrl->FtmNeighbor[idx];

		if (pFtmNeighbor->NeighborValid) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("%s() pFtmNeighbor Idx %d (%02x:%02x:%02x:%02x:%02x:%02x) is valid\n"
					  , __func__, idx, PRINT_MAC(pFtmNeighbor->NeighborBSSID)));
			BssidInfo.word = 0;
			BssidInfo.field.APReachAble = 3;
			BssidInfo.field.Security = 0; /* rrm to do. */
			BssidInfo.field.KeyScope = 0; /* "report AP has same authenticator as the AP. */
			BssidInfo.field.SepctrumMng = (pMbss->CapabilityInfo & (1 << 8)) ? 1 : 0;
			BssidInfo.field.Qos = (pMbss->CapabilityInfo & (1 << 9)) ? 1 : 0;
			BssidInfo.field.APSD = (pMbss->CapabilityInfo & (1 << 11)) ? 1 : 0;
			BssidInfo.field.RRM = (pMbss->CapabilityInfo & RRM_CAP_BIT) ? 1 : 0;
			BssidInfo.field.DelayBlockAck = (pMbss->CapabilityInfo & (1 << 14)) ? 1 : 0;
			BssidInfo.field.ImmediateBA = (pMbss->CapabilityInfo & (1 << 15)) ? 1 : 0;
			BssidInfo.field.FTM = pFtmNeighbor->NeighborFTMCap;
			COPY_MAC_ADDR(NeighborRepInfo.Bssid, pFtmNeighbor->NeighborBSSID);
			NeighborRepInfo.BssidInfo = BssidInfo.word;
			NeighborRepInfo.RegulatoryClass = pFtmNeighbor->NeighborOpClass;
			NeighborRepInfo.ChNum = pFtmNeighbor->NeighborChannel;
			NeighborRepInfo.PhyType = pFtmNeighbor->NeighborPhyType;
			RRM_InsertNeighborRepIE(pAd, (pOutBuffer + FrameLen), &FrameLen,
									sizeof(RRM_NEIGHBOR_REP_INFO), &NeighborRepInfo);

			if (pRrmCfg->bPeerReqLCI) {
				RRM_InsertNeighborMsmtRptLocationLCISubIE(pAd, (pOutBuffer + FrameLen),
						&FrameLen, pRrmCfg, &pFtmNeighbor->LciHdr, &pFtmNeighbor->LciUsage
						, &pFtmNeighbor->LciZ, &pFtmNeighbor->LciField, pFtmNeighbor->bSetNeighbotZRpt);
			}

			if (pRrmCfg->bPeerReqCIVIC) {
				RRM_InsertNeighborMsmtRptLocationCIVICSubIE(pAd, (pOutBuffer + FrameLen),
						&FrameLen, pRrmCfg, &pFtmNeighbor->CivicHdr, &pFtmNeighbor->Civic
						, pFtmNeighbor->CA_Value);
			}
		}
	}

#endif /* FTM_SUPPORT */
#ifdef AP_SCAN_SUPPORT

	/* insert NeighborRep IE. */
	for (loop = 0; loop < pAd->ScanTab.BssNr; loop++) {
		UINT8 BssMatch = FALSE;
		BSS_ENTRY *pBssEntry = &pAd->ScanTab.BssEntry[loop];
		/* Discards all remain Bss if the packet length exceed packet buffer size. */
		PktLen = FrameLen + sizeof(RRM_NEIGHBOR_REP_INFO)
				 + (pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.RrmCfg.bDot11kRRMNeighborRepTSFEnable == TRUE ? 6 : 0);

		if (PktLen >= MAX_MGMT_PKT_LEN)
			break;

		if (SsidLen != 0)
			BssMatch = RTMPEqualMemory(pBssEntry->Ssid, pSsid,
									   MIN(SsidLen, pBssEntry->SsidLen));
		else
			BssMatch = TRUE;

		if (BssMatch) {
			RRM_BSSID_INFO BssidInfo;

			BssidInfo.word = 0;
			BssidInfo.field.APReachAble = 3;
			BssidInfo.field.Security = RRM_CheckBssAndStaSecurityMatch(pAd, pEntry, pBssEntry);

			BssidInfo.field.KeyScope = 0; /* "report AP has same authenticator as the AP. */
			/*
				reference 2012 spec.
				802.11-2012.pdf
				page#582 (0 means information is not available  )
				The Key Scope bit, when set, indicates the AP indicated by this BSSID has the same authenticator as the AP
				sending the report. If this bit is 0, it indicates a distinct authenticator or the information is not available.
			*/
			BssidInfo.field.SepctrumMng = (pBssEntry->CapabilityInfo & (1 << 8)) ? 1 : 0;
			BssidInfo.field.Qos = (pBssEntry->CapabilityInfo & (1 << 9)) ? 1 : 0;
			BssidInfo.field.APSD = (pBssEntry->CapabilityInfo & (1 << 11)) ? 1 : 0;
			BssidInfo.field.RRM = (pBssEntry->CapabilityInfo & RRM_CAP_BIT) ? 1 : 0;
			BssidInfo.field.DelayBlockAck = (pBssEntry->CapabilityInfo & (1 << 14)) ? 1 : 0;
			BssidInfo.field.ImmediateBA = (pBssEntry->CapabilityInfo & (1 << 15)) ? 1 : 0;
			BssidInfo.field.MobilityDomain = (pBssEntry->bHasMDIE) ? 1 : 0;
			BssidInfo.field.HT = (pBssEntry->HtCapabilityLen != 0) ? 1 : 0;
#ifdef DOT11_VHT_AC
			BssidInfo.field.VHT = (pBssEntry->vht_cap_len != 0) ? 1 : 0;
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

			if (pBssEntry->Channel > 14) { /* 5G case */
				if (pBssEntry->HtCapabilityLen != 0) { /* HT or Higher case */
#ifdef DOT11_VHT_AC
					if (pBssEntry->vht_cap_len != 0)
						pBssEntry->CondensedPhyType = 9;
					else
#endif /* DOT11_VHT_AC */
						pBssEntry->CondensedPhyType = 7;
				} else /* OFDM case */
					pBssEntry->CondensedPhyType = 4;
			} else { /* 2.4G case */
				if (pBssEntry->HtCapabilityLen != 0) /* HT case */
					pBssEntry->CondensedPhyType = 7;
				else if (ERP_IS_NON_ERP_PRESENT(pBssEntry->Erp)) /* ERP case */
					pBssEntry->CondensedPhyType = 6;
				else if (pBssEntry->SupRateLen > 4)/* OFDM case (1,2,5.5,11 for CCK 4 Rates) */
					pBssEntry->CondensedPhyType = 4;

				/* no CCK's definition in spec. */
			}

			COPY_MAC_ADDR(NeighborRepInfo.Bssid, pBssEntry->Bssid);
			NeighborRepInfo.BssidInfo = cpu2le32(BssidInfo.word);
			NeighborRepInfo.RegulatoryClass = pBssEntry->RegulatoryClass;
			NeighborRepInfo.ChNum = pBssEntry->Channel;
			NeighborRepInfo.PhyType = pBssEntry->CondensedPhyType;
			RRM_InsertNeighborRepIE(pAd, (pOutBuffer + FrameLen), &FrameLen,
									sizeof(RRM_NEIGHBOR_REP_INFO), &NeighborRepInfo);
		}

		/*
			shall insert Neighbor Report TSF offset
			when the MIB attribute
			dot11RRMNeighborReportTSFOffsetEnabled is true.
		*/
		if (pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.RrmCfg.bDot11kRRMNeighborRepTSFEnable) {
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
									cpu2le16(pBssEntry->BeaconPeriod));
		}
	}

#endif /* AP_SCAN_SUPPORT */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s() : send Neighbor RSP\n", __func__));
	MiniportMMRequest(pAd, (MGMT_USE_QUEUE_FLAG | QID_AC_BE), pOutBuffer, FrameLen);

	if (pOutBuffer)
		MlmeFreeMemory(pOutBuffer);

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
	struct wifi_dev *wdev;

	if ((apidx >= pAd->ApCfg.BssidNum)
		|| (!(VALID_UCAST_ENTRY_WCID(pAd, Aid)))) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_ERROR, ("%s: Invalid STA. apidx=%d Aid=%d\n",
				 __func__, apidx, Aid));
		return;
	}

	NStatus = MlmeAllocateMemory(pAd, (PVOID)&pOutBuffer);  /* Get an unused nonpaged memory */

	if (NStatus != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_TRACE, ("%s() allocate memory failed\n", __func__));
		return;
	}

	pEntry = &pAd->MacTab.Content[Aid];
	wdev = pEntry->wdev;
	/* build action frame header. */
	MgtMacHeaderInit(pAd, &ActHdr, SUBTYPE_ACTION, 0, pEntry->Addr,
					 pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.if_addr,
					 pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.bssid);
	NdisMoveMemory(pOutBuffer, (PCHAR)&ActHdr, sizeof(HEADER_802_11));
	FrameLen = sizeof(HEADER_802_11);
	InsertActField(pAd, (pOutBuffer + FrameLen), &FrameLen,
				   CATEGORY_RM, RRM_LNK_MEASURE_REQ);
	/* fill Dialog Token */
	InsertDialogToken(pAd, (pOutBuffer + FrameLen), &FrameLen, DialogToken);
	/* fill Tx Power Used field */
	{
		ULONG TempLen;
		UINT8 TxPwr = RTMP_GetTxPwr(pAd, wdev->rate.MlmeTransmit, wdev->channel, wdev);

		MakeOutgoingFrame(pOutBuffer + FrameLen,	&TempLen,
						  1,							&TxPwr,
						  END_OF_ARGS);
		FrameLen += TempLen;
	}
	/* fill Max Tx Power field */
	{
		ULONG TempLen;
		UCHAR op_ht_bw = wlan_operate_get_ht_bw(wdev);
		UINT8 MaxTxPwr = GetCuntryMaxTxPwr(pAd, wdev->PhyMode, wdev, op_ht_bw);

		MakeOutgoingFrame(pOutBuffer + FrameLen,	&TempLen,
						  1,							&MaxTxPwr,
						  END_OF_ARGS);
		FrameLen += TempLen;
	}
	MeasureReqInsert(pAd, DialogToken);
	MiniportMMRequest(pAd, (MGMT_USE_QUEUE_FLAG | QID_AC_BE), pOutBuffer, FrameLen);

	if (pOutBuffer)
		MlmeFreeMemory(pOutBuffer);

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

	if (NStatus != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_TRACE, ("%s() allocate memory failed\n", __func__));
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
	RRM_InsertTxStreamReqIE(pAd, (pOutBuffer + FrameLen),
							&FrameLen, (PUCHAR)&TxMeasureReq);
	TotalLen += sizeof(RRM_TRANSMIT_MEASURE_INFO);

	/* inssert Trigger report sub field. */
	if (pMlmeTxMeasureReq->bTriggerReport == 1) {
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
		RRM_InsertTxStreamReqTriggerReportSubIE(pAd, (pOutBuffer + FrameLen),
												&FrameLen, (PUCHAR)&TriggerReport);
		TotalLen +=
			(sizeof(RRM_TRANSMIT_MEASURE_TRIGGER_REPORT) + 2);
		MeasureReqMode.field.Report = 1;
	}

	/* Insert Action header here. */
	{
		ULONG tmpLen = sizeof(HEADER_802_11);

		MeasureReqMode.field.Enable = 1;
		MeasureReqMode.field.DurationMandatory =
			pMlmeTxMeasureReq->bDurationMandatory;
		MakeMeasurementReqFrame(pAd, pOutBuffer, &tmpLen,
								TotalLen, CATEGORY_RM, RRM_MEASURE_REQ, MeasureReqToken,
								MeasureReqMode.word, MeasureReqType, 0xffff);
	}
	MeasureReqInsert(pAd, MeasureReqToken);
	MiniportMMRequest(pAd, (MGMT_USE_QUEUE_FLAG | QID_AC_BE), pOutBuffer, FrameLen);

	if (pOutBuffer)
		MlmeFreeMemory(pOutBuffer);

	return;
}

#endif /* DOT11K_RRM_SUPPORT */


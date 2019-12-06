/****************************************************************************
* Mediatek Inc.
* 5F., No.5, Taiyuan 1st St., Zhubei City,
* Hsinchu County 302, Taiwan, R.O.C.
* (c) Copyright 2014, Mediatek, Inc.
*
* All rights reserved. Mediatek's source code is an unpublished work and the
* use of a copyright notice does not imply otherwise. This source code
* contains confidential trade secret material of Mediatek. Any attemp
* or participation in deciphering, decoding, reverse engineering or in any
* way altering the source code is stricitly prohibited, unless the prior
* written consent of Mediatek, Inc. is obtained.
****************************************************************************

	Module Name:
	mbo.c

	Abstract:
	MBO (AP) implementation.

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
	kyle        2016.8.10   Initial version: MBO IE API
*/

#ifdef MBO_SUPPORT
#include "rt_config.h"
#include "mbo.h"

static MBO_ERR_CODE MboInsertAttrById(
	struct wifi_dev *wdev,
	PUINT8 pAttrTotalLen,
	PUINT8 pAttrBuf,
	UINT8  AttrId
	)
{
	P_MBO_CTRL	pMboCtrl = NULL;
	MBO_ATTR_STRUCT MboAttr;
	PUINT8 pAttrBufOffset = (pAttrBuf + *pAttrTotalLen);
	UINT16 OverflowChk = 0;

	if (wdev) {
		pMboCtrl = &wdev->MboCtrl;
	} else {
		MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("%s - Mbss is NULL !!!!!\n", __func__));
		return MBO_INVALID_ARG;
	}

	if (!VALID_MBO_ATTR_ID(AttrId)) {
		MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("%s %d - Invalid attr Id [%d] !!!!!\n", __func__, __LINE__, AttrId));
		return MBO_INVALID_ARG;
	}

	NdisZeroMemory(&MboAttr, sizeof(MBO_ATTR_STRUCT));

	switch (AttrId) {
	case MBO_ATTR_AP_CAP_INDCATION:
		MboAttr.AttrID		= MBO_ATTR_AP_CAP_INDCATION;
		MboAttr.AttrLen	= 1;
		MboAttr.AttrBody[0] = pMboCtrl->MboCapIndication;
		break;
	case MBO_ATTR_AP_ASSOC_DISALLOW:
		MboAttr.AttrID		= MBO_ATTR_AP_ASSOC_DISALLOW;
		MboAttr.AttrLen	= 1;
		MboAttr.AttrBody[0] = pMboCtrl->AssocDisallowReason;
		break;
	case MBO_ATTR_AP_CDCP:
		MboAttr.AttrID	= MBO_ATTR_AP_CDCP;
		MboAttr.AttrLen	= 1;
		MboAttr.AttrBody[0] = pMboCtrl->CellularPreference;
		break;
	case MBO_ATTR_AP_TRANS_REASON:
		MboAttr.AttrID		= MBO_ATTR_AP_TRANS_REASON;
		MboAttr.AttrLen	= 1;
		MboAttr.AttrBody[0] = pMboCtrl->TransitionReason;
		break;
	case MBO_ATTR_AP_ASSOC_RETRY_DELAY:
		MboAttr.AttrID	= MBO_ATTR_AP_ASSOC_RETRY_DELAY;
		MboAttr.AttrLen		= 2;
		NdisCopyMemory(&MboAttr.AttrBody[0], &pMboCtrl->ReAssocDelay, MboAttr.AttrLen);
		break;
	default:
		MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("%s - UNKNOWN AttrId [%d] !!!!!\n", __func__, AttrId));
	}

	OverflowChk = *pAttrTotalLen + MboAttr.AttrLen + 2;
	if (OverflowChk >= MBO_ATTR_MAX_LEN) {
		MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("%s - AttrTotalLen %d Overflow, should be below %d !!!!!\n",
		__func__, OverflowChk, MBO_ATTR_MAX_LEN));
		return MBO_UNEXP;
	} else {
		/* safe, insert the attribute */
		NdisCopyMemory(pAttrBufOffset, &MboAttr, MboAttr.AttrLen+2);
		*pAttrTotalLen += (MboAttr.AttrLen + 2);
	}

	return MBO_SUCCESS;
}


static MBO_ERR_CODE MboCollectAttribute(
	PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev,
	PUINT8 pAttrLen,
	PUINT8 pAttrBuf,
	UINT8 FrameType
	)
{
	UINT8 ErrCode = MBO_SUCCESS;
	P_MBO_CTRL	pMboCtrl = NULL;

	MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		("%s - collect attr for FrameType %d\n", __func__, FrameType));

	if (!pAd || !wdev || !pAttrLen || !pAttrBuf) {
		MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("%s - Invalid input argument !!!!!\n", __func__));
		return MBO_INVALID_ARG;
	}

	pMboCtrl = &wdev->MboCtrl;

	switch (FrameType) {
#ifdef CONFIG_AP_SUPPORT
	case MBO_FRAME_TYPE_BEACON:
	case MBO_FRAME_TYPE_ASSOC_RSP:
	case MBO_FRAME_TYPE_PROBE_RSP:
		ErrCode = MboInsertAttrById(wdev, pAttrLen, pAttrBuf, MBO_ATTR_AP_CAP_INDCATION);
		if (!MBO_AP_ALLOW_ASSOC(wdev)) {
			ErrCode = MboInsertAttrById(wdev, pAttrLen, pAttrBuf, MBO_ATTR_AP_ASSOC_DISALLOW);
		}
		break;
#endif /* CONFIG_AP_SUPPORT */


	default:
		MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("%s - UNKNOWN FrameType %d !!!!!\n", __func__, FrameType));
		return MBO_UNEXP;
	}

	return ErrCode;
}

VOID MakeMboOceIE(
	PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev,
	PUINT8 pFrameBuf,
	PULONG pFrameLen,
	UINT8 FrameType
	)
{
	ULONG	TempLen;
	UINT8	IEId = IE_MBO_ELEMENT_ID;
	UINT8	IELen = 0;
	UINT8	AttrLen = 0;
	UCHAR	MBO_OCE_OUIBYTE[4] = {0x50, 0x6f, 0x9a, 0x16};
	PUCHAR	pAttrBuf = NULL;

	if (wdev == NULL) {
		return;
	}


	MlmeAllocateMemory(pAd, &pAttrBuf);

	if (pAttrBuf == NULL)
		return;

	MboCollectAttribute(pAd, wdev, &AttrLen, pAttrBuf, FrameType);

	IELen = 4 + AttrLen;

	MakeOutgoingFrame(pFrameBuf,						&TempLen,
						1,								&IEId,
						1,								&IELen,
						4,								MBO_OCE_OUIBYTE,
						AttrLen,						pAttrBuf,
						END_OF_ARGS);

	*pFrameLen = *pFrameLen + TempLen;
	MlmeFreeMemory(pAttrBuf);
	return;
}

MBO_ERR_CODE ReadMboParameterFromFile(
    PRTMP_ADAPTER pAd,
    RTMP_STRING *tmpbuf,
    RTMP_STRING *pBuffer)
{
	INT loop;
	RTMP_STRING *macptr;

    /* MboSupport */
	if (RTMPGetKeyParameter("MboSupport", tmpbuf, MAX_PARAMETER_LEN, pBuffer, TRUE)) {
#ifdef CONFIG_AP_SUPPORT
		for (loop = 0, macptr = rstrtok(tmpbuf, ";");
				(macptr && loop < MAX_MBSSID_NUM(pAd));
					macptr = rstrtok(NULL, ";"), loop++) {
			UINT8 Enable;

			Enable = (UINT8)simple_strtol(macptr, 0, 10);
			pAd->ApCfg.MBSSID[loop].wdev.MboCtrl.bMboEnable =
				(Enable) ? TRUE : FALSE;
			pAd->ApCfg.MBSSID[loop].wdev.MboCtrl.MboCapIndication =
				(Enable) ? MBO_AP_CAP_CELLULAR_AWARE : MBO_AP_CAP_NOT_SUPPORT;

			MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s::(bMboEnable[%d]=%d, MboCapIndication = 0x%02x)\n", __func__, loop,
				pAd->ApCfg.MBSSID[loop].wdev.MboCtrl.bMboEnable,
				pAd->ApCfg.MBSSID[loop].wdev.MboCtrl.MboCapIndication));

		}
#endif /*CONFIG_AP_SUPPORT*/


	}
	return MBO_SUCCESS;
}


MBO_ERR_CODE MboInit(
	PRTMP_ADAPTER pAd)
{
	INT loop;
	P_MBO_CTRL pMboCtrl = NULL;

#ifdef CONFIG_AP_SUPPORT
	for (loop = 0; loop < MAX_MBSSID_NUM(pAd); loop++) {
		pMboCtrl = &pAd->ApCfg.MBSSID[loop].wdev.MboCtrl;
		NdisZeroMemory(pMboCtrl, sizeof(MBO_CTRL));
	}
#endif /* CONFIG_AP_SUPPORT */


	return MBO_SUCCESS;
}

#ifdef CONFIG_AP_SUPPORT
static VOID WextMboSendNeighborReportToDaemonEvent(
	PNET_DEV net_dev,
	P_DAEMON_EVENT_NR_LIST NeighborRepList,
	UINT16 report_buf_len)
{
	P_DAEMON_NR_MSG pNRMsg;
	UINT16 buflen = 0;
	char *buf;

	buflen = sizeof(DAEMON_NR_MSG);
	os_alloc_mem(NULL, (UCHAR **)&buf, buflen);
	NdisZeroMemory(buf, buflen);

	pNRMsg = (P_DAEMON_NR_MSG)buf;

	NdisCopyMemory(&pNRMsg->evt_nr_list, NeighborRepList, report_buf_len);
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s - sizeof %u report_buf_len %u buflen %u\n",
				__func__, (UINT32)sizeof(DAEMON_EVENT_NR_LIST), report_buf_len, buflen));
	RtmpOSWrielessEventSend(net_dev, RT_WLAN_EVENT_CUSTOM,
					OID_NEIGHBOR_REPORT, NULL, (PUCHAR)buf, buflen);

	os_free_mem(buf);
}

static VOID MboUpdateNRElement(
	RTMP_ADAPTER *pAd,
	struct wifi_dev *pWdev,
	BSS_ENTRY *pBssEntry,
	DAEMON_NEIGHBOR_REP_INFO *pNeighborEntry)
{
	RRM_BSSID_INFO BssidInfo;
	BOOLEAN bInsertMyOwnBss = (pBssEntry == NULL)?TRUE:FALSE;
	BSS_ENTRY myOwnBss;
	BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[pWdev->func_idx];

	if (bInsertMyOwnBss) {
		UINT8 CondensedPhyType = 0;

		COPY_MAC_ADDR(pNeighborEntry->Bssid, pWdev->bssid);
		pBssEntry = &myOwnBss;
		NdisZeroMemory(pBssEntry, sizeof(BSS_ENTRY));

		pBssEntry->CapabilityInfo = pMbss->CapabilityInfo;

		BssidInfo.word = 0;
		BssidInfo.field.APReachAble = 3;
		BssidInfo.field.Security = 1;
		BssidInfo.field.KeyScope = 0;
		BssidInfo.field.SepctrumMng = (pBssEntry->CapabilityInfo & (1 << 8)) ? 1:0;
		BssidInfo.field.Qos = (pBssEntry->CapabilityInfo & (1 << 9)) ? 1:0;
		BssidInfo.field.APSD = (pBssEntry->CapabilityInfo & (1 << 11)) ? 1:0;
		BssidInfo.field.RRM = (pBssEntry->CapabilityInfo & RRM_CAP_BIT) ? 1:0;
		BssidInfo.field.DelayBlockAck = (pBssEntry->CapabilityInfo & (1 << 14)) ? 1:0;
		BssidInfo.field.ImmediateBA = (pBssEntry->CapabilityInfo & (1 << 15)) ? 1:0;
#ifdef DOT11R_FT_SUPPORT
		BssidInfo.field.MobilityDomain = (pWdev->FtCfg.FtCapFlag.Dot11rFtEnable) ? 1:0;
#endif /* DOT11R_FT_SUPPORT */
		BssidInfo.field.HT = WMODE_CAP_N(pWdev->PhyMode) ? 1:0;
#ifdef DOT11_VHT_AC
		BssidInfo.field.VHT = WMODE_CAP_AC(pWdev->PhyMode) ? 1:0;
#endif /* DOT11_VHT_AC */

		if (pWdev->channel > 14) {
			if (BssidInfo.field.HT) {
#ifdef DOT11_VHT_AC
				if (BssidInfo.field.VHT)
					CondensedPhyType = 9;
				else
#endif /* DOT11_VHT_AC */
					CondensedPhyType = 7;
			} else {
				CondensedPhyType = 4;
			}
		} else {
			if (BssidInfo.field.HT)
				CondensedPhyType = 7;
			else if (ERP_IS_NON_ERP_PRESENT(pAd->ApCfg.ErpIeContent))
				CondensedPhyType = 6;
			else if (pBssEntry->SupRateLen > 4)
				CondensedPhyType = 4;
		}

		pNeighborEntry->BssidInfo = BssidInfo.word;
		pNeighborEntry->RegulatoryClass = get_regulatory_class(pAd, pWdev->channel, pWdev->PhyMode, pWdev);
		pNeighborEntry->ChNum = pWdev->channel;
		pNeighborEntry->PhyType = CondensedPhyType;
	} else {
		COPY_MAC_ADDR(pNeighborEntry->Bssid, pBssEntry->Bssid);
		/* update Neighbor Report Information Elements */
		BssidInfo.word = 0;
		BssidInfo.field.APReachAble = 3;
		BssidInfo.field.Security = 1; /* default value, will be updated in daemon */
		BssidInfo.field.KeyScope = 0;
		BssidInfo.field.SepctrumMng = (pBssEntry->CapabilityInfo & (1 << 8)) ? 1:0;
		BssidInfo.field.Qos = (pBssEntry->CapabilityInfo & (1 << 9)) ? 1:0;
		BssidInfo.field.APSD = (pBssEntry->CapabilityInfo & (1 << 11)) ? 1:0;
		BssidInfo.field.RRM = (pBssEntry->CapabilityInfo & RRM_CAP_BIT) ? 1:0;
		BssidInfo.field.DelayBlockAck = (pBssEntry->CapabilityInfo & (1 << 14)) ? 1:0;
		BssidInfo.field.ImmediateBA = (pBssEntry->CapabilityInfo & (1 << 15)) ? 1:0;
		BssidInfo.field.MobilityDomain = (pBssEntry->bHasMDIE) ? 1:0;
		BssidInfo.field.HT = (pBssEntry->HtCapabilityLen != 0) ? 1:0;
#ifdef DOT11_VHT_AC
		BssidInfo.field.VHT = (pBssEntry->vht_cap_len != 0) ? 1:0;
#endif /* DOT11_VHT_AC */

		if (pBssEntry->Channel > 14) {
			if (pBssEntry->HtCapabilityLen != 0) {
#ifdef DOT11_VHT_AC
				if (pBssEntry->vht_cap_len != 0)
					pBssEntry->CondensedPhyType = 9;
				else
#endif /* DOT11_VHT_AC */
					pBssEntry->CondensedPhyType = 7;
			} else {
				pBssEntry->CondensedPhyType = 4;
			}
		} else {
			if (pBssEntry->HtCapabilityLen != 0)
				pBssEntry->CondensedPhyType = 7;
			else if (ERP_IS_NON_ERP_PRESENT(pBssEntry->Erp))
				pBssEntry->CondensedPhyType = 6;
			else if (pBssEntry->SupRateLen > 4)
				pBssEntry->CondensedPhyType = 4;
		}

		pNeighborEntry->BssidInfo = BssidInfo.word;
		pNeighborEntry->RegulatoryClass = get_regulatory_class(pAd, pBssEntry->Channel,
		pWdev->PhyMode, pWdev);/* pBssEntry->RegulatoryClass; */
		pNeighborEntry->ChNum = pBssEntry->Channel;
		pNeighborEntry->PhyType = pBssEntry->CondensedPhyType;

		pNeighborEntry->akm = pBssEntry->AKMMap;
		pNeighborEntry->cipher = pBssEntry->PairwiseCipher;
	}


	/* add BSS Transition Candidate Preference subelement - [Subelement ID=3][length=1][preference=20] */
	pNeighborEntry->CandidatePrefSubID  = MBO_RRM_SUBID_BSS_TRANSITION_CANDIDATE_PREFERENCE;
	pNeighborEntry->CandidatePrefSubLen = 1;
	if (bInsertMyOwnBss && !MBO_AP_ALLOW_ASSOC(pWdev))
		pNeighborEntry->CandidatePref = 0;
#ifdef DOT11R_FT_SUPPORT
	else if (pWdev->FtCfg.FtCapFlag.Dot11rFtEnable == TRUE) {
		/* YF_NR */
		pNeighborEntry->CandidatePref = MBO_AP_DEFAULT_CAND_PREF;

		if (bInsertMyOwnBss)
			pNeighborEntry->CandidatePref = 0;
		else if (pBssEntry->bHasMDIE &&
			    (pBssEntry->AKMMap == pWdev->SecConfig.AKMMap) &&
			    (pBssEntry->PairwiseCipher == pWdev->SecConfig.PairwiseCipher) &&
			     NdisCmpMemory(pBssEntry->FT_MDIE.MdId, pWdev->FtCfg.FtMdId, FT_MDID_LEN) == 0) {
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_FT, DBG_LVL_OFF, ("AP(%02x:%02x:%02x:%02x:%02x:%02x) same in FT Domain\n",
				PRINT_MAC(pNeighborEntry->Bssid)));
			pNeighborEntry->CandidatePref = 255;
		}
	}
#endif /* DOT11R_FT_SUPPORT */
	else
		pNeighborEntry->CandidatePref = MBO_AP_DEFAULT_CAND_PREF;
}

/*	format : iwpriv [interface] set mbo_nr=[append]-[nr_entry_num]
	sample : iwpriv ra0 set mbo_nr=0-12
			==> renew list,not append,indicate 12 entries
*/
INT SetMboNRIndicateProc(
	PRTMP_ADAPTER	pAd,
	RTMP_STRING *arg)
{
	UINT8 i = 0, input = 0, ReportNum = 0;
	RTMP_STRING *macptr;
	BOOLEAN AppendMode = FALSE;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct wifi_dev *pWdev = &pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev;


	for (i = 0, macptr = rstrtok(arg, "-"); macptr; macptr = rstrtok(NULL, "-"), i++) {
		if (i == 0)
			input = (UINT8) simple_strtol(macptr, 0, 10);
		else if (i == 1)
			ReportNum = (UINT8) simple_strtol(macptr, 0, 10);
		else
			break;
	}

	AppendMode = (input)?TRUE:FALSE;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						("%s - AppendMode %d , reportNum %d\n",
						__func__, AppendMode, ReportNum));

	if (ReportNum == 0)
		ReportNum = 10;

	MboIndicateNeighborReportToDaemon(pAd, pWdev, AppendMode, ReportNum);

	return TRUE;
}

INT MboIndicateNeighborReportToDaemon(
	PRTMP_ADAPTER	pAd,
	struct wifi_dev *pWdev,
	BOOLEAN		AppendMode,
	UINT8			ReportNum)
{
#ifdef DOT11K_RRM_SUPPORT
#ifdef AP_SCAN_SUPPORT
	UINT32 loop = 0;
	DAEMON_EVENT_NR_LIST NeighborRepList;
	BOOLEAN bNewlist = !AppendMode;

	DAEMON_NEIGHBOR_REP_INFO *pNeighborEntry = NULL;
	UINT8 TotalReportNum = 0;

	NdisZeroMemory(&NeighborRepList, sizeof(DAEMON_EVENT_NR_LIST));


	if (pAd->ScanTab.BssNr > 0) {

		BssTableSortByRssi(&pAd->ScanTab, FALSE);

		TotalReportNum = ReportNum < pAd->ScanTab.BssNr ?
				 ReportNum : pAd->ScanTab.BssNr;
		NeighborRepList.TotalNum = TotalReportNum;

		/* insert our own bss info into NR list first */
		pNeighborEntry = &NeighborRepList.EvtNRInfo[NeighborRepList.CurrNum++];

		MboUpdateNRElement(pAd, pWdev, NULL, pNeighborEntry);

		for (loop = 0; loop < TotalReportNum - 1; loop++) {/* minus our own NR entry */
			BSS_ENTRY *pBssEntry = &pAd->ScanTab.BssEntry[loop];
			DAEMON_NEIGHBOR_REP_INFO *pNeighborEntry = &NeighborRepList.EvtNRInfo[NeighborRepList.CurrNum];

			NeighborRepList.CurrNum++;

			MboUpdateNRElement(pAd, pWdev, pBssEntry, pNeighborEntry);

			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("%s - append NO. %u len %u BSSID %02X:%02X:%02X:%02X:%02X:%02X Chnum %d BssidInfo %X\n"
				"PhyType %x RegulatoryClass %x Privacy %d SSID %s\n",
				__func__, loop, (UINT32)sizeof(DAEMON_NEIGHBOR_REP_INFO), PRINT_MAC(pNeighborEntry->Bssid),
				pNeighborEntry->ChNum, pNeighborEntry->BssidInfo,
				pNeighborEntry->PhyType, pNeighborEntry->RegulatoryClass,
				pBssEntry->Privacy, pBssEntry->Ssid));
			hex_dump("neighbor_entry", (UCHAR *)pNeighborEntry, sizeof(DAEMON_NEIGHBOR_REP_INFO));

			if ((NeighborRepList.CurrNum % PER_EVENT_LIST_MAX_NUM == 0)
			&& (NeighborRepList.CurrNum != 0)
			) {
				NeighborRepList.Newlist = bNewlist;
				bNewlist = FALSE;
				/* indicate sublist to daemon */
				WextMboSendNeighborReportToDaemonEvent(pAd->net_dev,
							  &NeighborRepList,
							  sizeof(DAEMON_EVENT_NR_LIST));
				printk("\033[1;32m %s, %u NeighborRepList.Newlist %d NeighborRepList.TotalNum %d NeighborRepList.CurrNum %d\033[0m\n"
					, __func__, __LINE__, NeighborRepList.Newlist, NeighborRepList.TotalNum, NeighborRepList.CurrNum);  /* Kyle Debug Print */
				NdisZeroMemory(&NeighborRepList, sizeof(DAEMON_EVENT_NR_LIST));
				NeighborRepList.TotalNum = TotalReportNum;
			}

		}

		if (NeighborRepList.CurrNum != 0) {
			NeighborRepList.Newlist = bNewlist;
			bNewlist = FALSE;
			/* indicate the last sublist to daemon */
			WextMboSendNeighborReportToDaemonEvent(pAd->net_dev,
						  &NeighborRepList,
						  sizeof(DAEMON_EVENT_NR_LIST));

			NdisZeroMemory(&NeighborRepList, sizeof(DAEMON_EVENT_NR_LIST));
		}
	} else {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s - nothing to indicate, pAd->ScanTab.BssNr = %d\n",
				__func__, pAd->ScanTab.BssNr));

		NeighborRepList.Newlist = TRUE;
		NeighborRepList.TotalNum = 0;

		/* indicate zero to daemon */
		WextMboSendNeighborReportToDaemonEvent(pAd->net_dev,
							  &NeighborRepList,
							  sizeof(DAEMON_EVENT_NR_LIST));
	}

#endif /* AP_SCAN_SUPPORT */
#endif /* DOT11K_RRM_SUPPORT */

	return TRUE;
}

INT MBO_MsgHandle(
	IN PRTMP_ADAPTER pAd,
	UINT32 Param,
	UINT32 Value)
{

	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	UCHAR APIndex = pObj->ioctl_if;
	P_MBO_CTRL pMboCtrl;

	pMboCtrl = &pAd->ApCfg.MBSSID[APIndex].wdev.MboCtrl;

	switch (Param) {
	case PARAM_MBO_AP_ASSOC_DISALLOW:
		pMboCtrl->AssocDisallowReason = Value;
		break;
	case PARAM_MBO_AP_CAP:
		pMboCtrl->MboCapIndication = Value;
		break;
	case PARAM_MBO_AP_CDCP:
		pMboCtrl->CellularPreference = Value;
		break;
	case PARAM_MBO_AP_BSS_TERM:
		MboBssTermStart(pAd, Value);
		break;
	default:
		MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Unknow Parameter:%d\n", Param));
		break;
	}
	return 0;
}

VOID MboWaitAllStaGone(PRTMP_ADAPTER pAd, INT apidx)
{
	UINT i = 0, j = 0;
	PMAC_TABLE_ENTRY pEntry;

	for (j = 0; j < 10; j++) {
		BOOLEAN bSTAIsKeep = FALSE;

		for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
			pEntry = &pAd->MacTab.Content[i];
			if (pEntry && IS_ENTRY_CLIENT(pEntry)
				&& pEntry->func_tb_idx == apidx && pEntry->IsKeep) {
				bSTAIsKeep = TRUE;
				break;
			}
		}

		if (bSTAIsKeep)
			RtmpOsMsDelay(50);
		else
			return;  /* All Sta Gone , return */
	}

	/* exceed 500 ms */
	return;
}

#endif /* CONFIG_AP_SUPPORT */

static VOID WextMboSendStaInfoToDaemonEvent(
	PNET_DEV pNetDev,
	P_MBO_STA_CH_PREF_CDC_INFO pStaInfo,
	MBO_MSG_TYPE MsgType,
	UINT16 ReportBufLen)
{
	P_MBO_MSG pMboMsg;
	UINT16 buflen = 0;
	char *buf;

	buflen = sizeof(MBO_MSG);
	os_alloc_mem(NULL, (UCHAR **)&buf, buflen);
	NdisZeroMemory(buf, buflen);

	pMboMsg = (P_MBO_MSG)buf;
	pMboMsg->ifindex = RtmpOsGetNetIfIndex(pNetDev);
	pMboMsg->MboMsgLen = ReportBufLen;
	pMboMsg->MboMsgType = MsgType;

	NdisCopyMemory(&pMboMsg->MboMsgBody.MboEvtStaInfo, pStaInfo, ReportBufLen);

	if (MsgType == MBO_MSG_CDC_UPDATE)
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("%s, indicate STA CDC %d\n",
					__func__, pMboMsg->MboMsgBody.MboEvtStaInfo.cdc));


	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s - sizeof %u report_buf_len %d buflen %u msg_type %s\n",
				__func__, (UINT32)sizeof(MBO_STA_CH_PREF_CDC_INFO), ReportBufLen, buflen, MboMsgTypeToString(MsgType)));
	RtmpOSWrielessEventSend(pNetDev, RT_WLAN_EVENT_CUSTOM,
					OID_802_11_MBO_MSG, NULL, (PUCHAR)buf, buflen);

	os_free_mem(buf);
}

INT MboIndicateStaInfoToDaemon(
	PRTMP_ADAPTER	pAd,
	P_MBO_STA_CH_PREF_CDC_INFO pStaInfo,
	MBO_MSG_TYPE MsgType)
{
	/* mac table lookup & update sta's akm/cipher here */
	PMAC_TABLE_ENTRY pEntry = MacTableLookup(pAd, pStaInfo->mac_addr);

	if (pEntry != NULL) {
		pStaInfo->akm = pEntry->SecConfig.AKMMap;
		pStaInfo->cipher = pEntry->SecConfig.PairwiseCipher;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,
			("\033[1;33m %s, %u pEntry->wcid %d pStaInfo->akm 0x%x pStaInfo->cipher 0x%x\033[0m\n"
			, __func__, __LINE__, pEntry->wcid, pStaInfo->akm, pStaInfo->cipher));  /* Kyle Debug Print (Y) */
	} else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("%s [ERROR] can't find sta entry!!\n",
					__func__));
		return FALSE;
	}

	/* send wext event */
	WextMboSendStaInfoToDaemonEvent(pAd->net_dev, pStaInfo, MsgType, sizeof(MBO_STA_CH_PREF_CDC_INFO));

	return TRUE;
}

VOID MboParseStaNPCElement(
	PRTMP_ADAPTER pAd,
	struct wifi_dev *pWdev,
	UCHAR *PktContent,
	UINT8 ElementLen,
	P_MBO_STA_CH_PREF_CDC_INFO pMboStaCHInfo,
	MBO_FRAME_TYPE MboFrameType)
{
	/* PktContent starts at Operating Class Field */
	INT8 NpclListLen = 0;
	BOOLEAN bEmptyNPC = FALSE;

	if (MboFrameType == MBO_FRAME_TYPE_WNM_REQ) {
		bEmptyNPC = (ElementLen <= 4)?TRUE:FALSE; /* contains only OUI == EmptyNPC */
		NpclListLen = (ElementLen > 7)?ElementLen - 7:0; /* OUI 4 bytes , Operating Class 1 byte, Pref 1 byte, Reason Code 1 byte */
	} else if (MboFrameType == MBO_FRAME_TYPE_ASSOC_REQ) {
		bEmptyNPC = (ElementLen == 0)?TRUE:FALSE; /* no op class == EmptyNPC */
		NpclListLen = (ElementLen > 3)?ElementLen - 3:0; /* Operating Class 1 byte, Pref 1 byte, Reason Code 1 byte */
	} else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s, UNKNOWN Frame Type %d , quit.\n", __func__, MboFrameType));
		return;
	}


	if (bEmptyNPC) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s, ElementLen %d indicates no NPC list, All OP-Class CH good.\n"
			, __func__, ElementLen));

		pMboStaCHInfo->npc_num = 255;
		return;
	} else {
		UINT8 OperatingClass = *PktContent;
		UCHAR *ChListStart = PktContent + 1;
		UINT8 Preference = *(ChListStart + NpclListLen);
		UINT8 ReasonCode = *(ChListStart + NpclListLen + 1);
		UCHAR *OpChList = get_channelset_by_reg_class(pAd, OperatingClass, pWdev->PhyMode);
		UCHAR OpChListLen = get_channel_set_num(OpChList);
		UINT8 NpcList[MAX_NOT_PREFER_CH_NUM] = {0};
		UINT8 i = 0, j = 0;

		/* to prevent NpclListLen overflow, should not happen  */
		if (NpclListLen > MAX_NOT_PREFER_CH_NUM)
			NpclListLen = MAX_NOT_PREFER_CH_NUM;

		/* to prevent pMboStaCHInfo->npc stack overflow  */
		if (pMboStaCHInfo->npc_num + OpChListLen >= MBO_NPC_MAX_LEN) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s, cur_npc_num %d, OpChListLen %d >= MBO_NPC_MAX_LEN %d, return due to overflow.\n",
			__func__, pMboStaCHInfo->npc_num, OpChListLen, MBO_NPC_MAX_LEN));
			return;
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("%s, Got NpclListLen %d, OperatingClass %d, Preference %d, ReasonCode %d OpChListLen %d pMboStaCHInfo->npc_num %d\n",
			__func__, NpclListLen, OperatingClass, Preference, ReasonCode,
			OpChListLen, pMboStaCHInfo->npc_num));

		if (OpChListLen == 0)
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s, Got OpChListLen %d can't find OperatingClass 0x%x  ERROR!!!!!!!!!!!!!!!!\n",
			__func__, OpChListLen, OperatingClass));

		for (i = 0; i < NpclListLen; i++) {
			NpcList[i] = *(ChListStart + i);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("NpcList[%d] = %d\n", i, NpcList[i]));
		}

		/* append channel list to  pMboStaCHInfo */
		for (j = 0; j < OpChListLen; j++) {
			BOOLEAN bIsNpc = FALSE;
			UINT8 offset = pMboStaCHInfo->npc_num + j;
			P_STA_CH_PREF pNpc = &pMboStaCHInfo->npc[offset];

			for (i = 0; i < NpclListLen; i++) {
				if (OpChList[j] == NpcList[i])
					bIsNpc = TRUE;
			}

			pNpc->pref = (bIsNpc)?Preference:255;
			pNpc->ch = OpChList[j];
			pNpc->reason_code = ReasonCode;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("++ final NPC struct [%d] : ch %d , pref %d\n", offset, pNpc->ch, pNpc->pref));
		}

		pMboStaCHInfo->npc_num += OpChListLen;
	}

}

VOID MboParseStaMboIE(PRTMP_ADAPTER pAd, struct wifi_dev *pWdev, struct _MAC_TABLE_ENTRY *pEntry, UCHAR *buf, UCHAR len, MBO_FRAME_TYPE MboFrameType)
{
	UCHAR *pos = NULL;
	UCHAR ParsedLen = 0;
	MBO_STA_CH_PREF_CDC_INFO *pMboStaInfoNPC = NULL;
	MBO_STA_CH_PREF_CDC_INFO *pMboStaInfoCDC = NULL;
	PEID_STRUCT eid_ptr;

	if (!pEntry) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s - pEntry is NULL!!!\n", __func__));
		return;
	}

	pMboStaInfoCDC = &pEntry->MboStaInfoCDC;
	pMboStaInfoNPC = &pEntry->MboStaInfoNPC;
	pEntry->bIndicateCDC = FALSE;
	pEntry->bIndicateNPC = FALSE;

	NdisZeroMemory(pMboStaInfoNPC, sizeof(MBO_STA_CH_PREF_CDC_INFO));
	NdisZeroMemory(pMboStaInfoCDC, sizeof(MBO_STA_CH_PREF_CDC_INFO));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s - Got MBO IE len [%d]\n", __func__, len));


	pos = buf;

	/* skip OUI 4 bytes */
	pos += 4;
	ParsedLen += 4;

	eid_ptr = (PEID_STRUCT)pos;

	/*
		in empty NPC case , MBO IE contains only OUI and NPC eid 2 len 0.
		so ParsedLen+2 == len
	*/
	while ((ParsedLen+2) <= len) {
		switch (eid_ptr->Eid) {
		case MBO_ATTR_STA_CDC:
				COPY_MAC_ADDR(pMboStaInfoCDC->mac_addr, pEntry->Addr);
				pMboStaInfoCDC->cdc = eid_ptr->Octet[0];
				pMboStaInfoCDC->npc_num = 0;
				pEntry->bIndicateCDC = TRUE;
				break;
		case MBO_ATTR_STA_NOT_PREFER_CH_REP:
				COPY_MAC_ADDR(pMboStaInfoNPC->mac_addr, pEntry->Addr);
				MboParseStaNPCElement(pAd, pWdev, &eid_ptr->Octet[0], eid_ptr->Len, pMboStaInfoNPC, MboFrameType);
				pEntry->bIndicateNPC = TRUE;
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("%s, %u npc_num %d mac_addr %02X:%02X:%02X:%02X:%02X:%02X bIndicateNPC %d\n"
					, __func__, __LINE__, pMboStaInfoNPC->npc_num, PRINT_MAC(pMboStaInfoNPC->mac_addr), pEntry->bIndicateNPC));
				break;
		default:
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("%s - ignored MBO_ATTR [%d]\n", __func__, eid_ptr->Eid));
		}

		ParsedLen += (2 + eid_ptr->Len);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, %u eid_ptr->Len %d ParsedLen %d IE len %d Break %d\n"
					, __func__, __LINE__, eid_ptr->Len, ParsedLen, len, ((ParsedLen+2) < len)));
		eid_ptr = (PEID_STRUCT)((UCHAR *)eid_ptr + 2 + eid_ptr->Len);
	}

}

VOID MboIndicateStaBssidInfo(PRTMP_ADAPTER pAd, struct wifi_dev *pWdev, UCHAR *mac_addr)
{
	MBO_STA_CH_PREF_CDC_INFO MboStaInfo;

	NdisZeroMemory(&MboStaInfo, sizeof(MBO_STA_CH_PREF_CDC_INFO));
	COPY_MAC_ADDR(&MboStaInfo.mac_addr, mac_addr);
	COPY_MAC_ADDR(&MboStaInfo.bssid, pWdev->bssid);
	MboIndicateStaInfoToDaemon(pAd, &MboStaInfo, MBO_MSG_BSSID_UPDATE);
}

static VOID WextMboSendStaDisassocToDaemonEvent(
	PNET_DEV pNetDev,
	P_MBO_EVENT_STA_DISASSOC pStaDisassocInfo,
	MBO_MSG_TYPE MsgType,
	UINT16 ReportBufLen)
{
	P_MBO_MSG pMboMsg;
	UINT16 buflen = 0;
	char *buf;

	buflen = sizeof(MBO_MSG);
	os_alloc_mem(NULL, (UCHAR **)&buf, buflen);
	NdisZeroMemory(buf, buflen);

	pMboMsg = (P_MBO_MSG)buf;
	pMboMsg->ifindex = RtmpOsGetNetIfIndex(pNetDev);
	pMboMsg->MboMsgLen = ReportBufLen;
	pMboMsg->MboMsgType = MsgType;

	if (MsgType != MBO_MSG_AP_TERMINATION)
		NdisCopyMemory(&pMboMsg->MboMsgBody.MboEvtStaDisassoc, pStaDisassocInfo, ReportBufLen);

	if (pStaDisassocInfo && pStaDisassocInfo->mac_addr) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s [%02x:%02x:%02x:%02x:%02x:%02x] sizeof %u \
					report_buf_len %d buflen %d msg_type %s\n",
				__func__, PRINT_MAC(pStaDisassocInfo->mac_addr), (UINT32)sizeof(MBO_EVENT_STA_DISASSOC), ReportBufLen, buflen, MboMsgTypeToString(MsgType)));

		RtmpOSWrielessEventSend(pNetDev, RT_WLAN_EVENT_CUSTOM,
					OID_802_11_MBO_MSG, NULL, (PUCHAR)buf, buflen);
	}
	os_free_mem(buf);
}

INT MboIndicateStaDisassocToDaemon(
	PRTMP_ADAPTER	pAd,
	P_MBO_EVENT_STA_DISASSOC pStaDisassocInfo,
	MBO_MSG_TYPE MsgType)
{
	WextMboSendStaDisassocToDaemonEvent(pAd->net_dev, pStaDisassocInfo, MsgType, sizeof(MBO_EVENT_STA_DISASSOC));

	return TRUE;
}

static VOID WextMboSendBssTermToDaemonEvent(
	PNET_DEV pNetDev,
	P_MBO_EVENT_BSS_TERM pBssTermTsf,
	MBO_MSG_TYPE MsgType,
	UINT16 ReportBufLen)
{
	P_MBO_MSG pMboMsg;
	UINT16 buflen = 0;
	char *buf;

	if (pBssTermTsf == NULL)
	return;

	buflen = sizeof(MBO_MSG);
	os_alloc_mem(NULL, (UCHAR **)&buf, buflen);
	NdisZeroMemory(buf, buflen);

	pMboMsg = (P_MBO_MSG)buf;
	pMboMsg->ifindex = RtmpOsGetNetIfIndex(pNetDev);
	pMboMsg->MboMsgLen = ReportBufLen;
	pMboMsg->MboMsgType = MsgType;

	NdisCopyMemory(&pMboMsg->MboMsgBody.MboEvtBssTermTsf, pBssTermTsf, ReportBufLen);

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s - sizeof %u report_buf_len %d buflen %d msg_type %s\n",
				__func__, (UINT32)sizeof(MBO_EVENT_BSS_TERM), ReportBufLen, buflen, MboMsgTypeToString(MsgType)));

	RtmpOSWrielessEventSend(pNetDev, RT_WLAN_EVENT_CUSTOM,
					OID_802_11_MBO_MSG, NULL, (PUCHAR)buf, buflen);
	os_free_mem(buf);
}

RTMP_STRING *MboMsgTypeToString(MBO_MSG_TYPE MsgType)
{
	if (MsgType == MBO_MSG_NEIGHBOR_REPORT)
		return "MBO_MSG_NEIGHBOR_REPORT";
	else if (MsgType == MBO_MSG_STA_PREF_UPDATE)
		return "MBO_MSG_STA_PREF_UPDATE";
	else if (MsgType == MBO_MSG_CDC_UPDATE)
		return "MBO_MSG_CDC_UPDATE";
	else if (MsgType == MBO_MSG_STA_STEERING)
		return "MBO_MSG_STA_STEERING";
	else if (MsgType == MBO_MSG_DISASSOC_STA)
		return "MBO_MSG_DISASSOC_STA";
	else if (MsgType == MBO_MSG_AP_TERMINATION)
		return "MBO_MSG_AP_TERMINATION";
	else if (MsgType == MBO_MSG_BSSID_UPDATE)
		return "MBO_MSG_BSSID_UPDATE";
	else if (MsgType == MBO_MSG_REMOVE_STA)
		return "MBO_MSG_REMOVE_STA";
	else
		return "UNKNOWN MSG TYPE";

}


RTMP_STRING *MboAttrValueToString(UINT8 AttrID, UINT8 AttrValue)
{
	switch (AttrID) {
	case MBO_ATTR_AP_CAP_INDCATION:
		if (AttrValue == MBO_AP_CAP_NOT_SUPPORT)
			return "MBO_AP_CAP_NOT_SUPPORT";
		else if (AttrValue == MBO_AP_CAP_CELLULAR_AWARE)
			return "MBO_AP_CAP_CELLULAR_AWARE";
		else
			return "UNKNOWN ATTR VALUE...";
		break;
	case MBO_ATTR_AP_ASSOC_DISALLOW:
		if (AttrValue == MBO_AP_DISALLOW_REASON_RESERVED)
			return "MBO_AP_ALLOW_ASSOC";
		else if (AttrValue == MBO_AP_DISALLOW_REASON_UNSPECIFIED)
			return "MBO_AP_DISALLOW_REASON_UNSPECIFIED";
		else if (AttrValue == MBO_AP_DISALLOW_MAX_STA_NUM_REACHED)
			return "MBO_AP_DISALLOW_MAX_STA_NUM_REACHED";
		else if (AttrValue == MBO_AP_DISALLOW_AIR_OVERLOADED)
			return "MBO_AP_DISALLOW_AIR_OVERLOADED";
		else if (AttrValue == MBO_AP_DISALLOW_AUTH_SERVER_OVERLOADED)
			return "MBO_AP_DISALLOW_AUTH_SERVER_OVERLOADED";
		else if (AttrValue == MBO_AP_DISALLOW_RSSI_TOO_LOW)
			return "MBO_AP_DISALLOW_RSSI_TOO_LOW";
		else
			return "UNKNOWN ATTR VALUE...";
		break;
	case MBO_ATTR_AP_CDCP:
		if (AttrValue == MBO_AP_CDCP_FORBID_STA_USE_CDC)
			return "MBO_AP_CDCP_FORBID_STA_USE_CDC";
		else if (AttrValue == MBO_AP_CDCP_PREFER_STA_NOT_USE_CDC)
			return "MBO_AP_CDCP_PREFER_STA_NOT_USE_CDC";
		else if (AttrValue == MBO_AP_CDCP_PREFER_STA_USE_CDC)
			return "MBO_AP_CDCP_PREFER_STA_USE_CDC";
		else
			return "UNKNOWN ATTR VALUE...";
		break;
	case MBO_ATTR_AP_TRANS_REASON:
		if (AttrValue == MBO_AP_TRANS_REASON_UNSPECIFIED)
			return "MBO_AP_TRANS_REASON_UNSPECIFIED";
		else if (AttrValue == MBO_AP_TRANS_REASON_TX_PER_TOO_HIGH)
			return "MBO_AP_TRANS_REASON_TX_PER_TOO_HIGH";
		else if (AttrValue == MBO_AP_TRANS_REASON_TRAFFIC_DELAY)
			return "MBO_AP_TRANS_REASON_TRAFFIC_DELAY";
		else if (AttrValue == MBO_AP_TRANS_REASON_INSUFFICIENT_BW)
			return "MBO_AP_TRANS_REASON_INSUFFICIENT_BW";
		else if (AttrValue == MBO_AP_TRANS_REASON_LOAD_BALACING)
			return "MBO_AP_TRANS_REASON_LOAD_BALACING";
		else if (AttrValue == MBO_AP_TRANS_REASON_RSSI_LOW)
			return "MBO_AP_TRANS_REASON_RSSI_LOW";
		else if (AttrValue == MBO_AP_TRANS_REASON_TOO_MANY_RETRY)
			return "MBO_AP_TRANS_REASON_TOO_MANY_RETRY";
		else if (AttrValue == MBO_AP_TRANS_REASON_HIGH_INTRFERENCE)
			return "MBO_AP_TRANS_REASON_HIGH_INTRFERENCE";
		else if (AttrValue == MBO_AP_TRANS_REASON_GRAY_ZONE)
			return "MBO_AP_TRANS_REASON_GRAY_ZONE";
		else if (AttrValue == MBO_AP_TRANS_REASON_TO_PREMIUM_AP)
			return "MBO_AP_TRANS_REASON_TO_PREMIUM_AP";
		else
			return "UNKNOWN ATTR VALUE...";
		break;
	default:
		return "UNKNOWN ATTR VALUE...";
	}
}


INT32 ShowMboStatProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8 loop = 0;
	P_MBO_CTRL pMboCtrl = NULL;
#ifdef CONFIG_AP_SUPPORT
	for (loop = 0; loop < pAd->ApCfg.BssidNum; loop++) {
		pMboCtrl = &pAd->ApCfg.MBSSID[loop].wdev.MboCtrl;

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("========= apidx %d ===========\n", loop));

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("bMboEnable \t\t %d\n",
			pMboCtrl->bMboEnable));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("AssocDisallowReason \t 0x%x [%s]\n",
			pMboCtrl->AssocDisallowReason, MboAttrValueToString(MBO_ATTR_AP_ASSOC_DISALLOW, pMboCtrl->AssocDisallowReason)));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("CellularPreference \t 0x%x [%s]\n",
			pMboCtrl->CellularPreference, MboAttrValueToString(MBO_ATTR_AP_CDCP, pMboCtrl->CellularPreference)));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TransitionReason \t 0x%x [%s]\n",
			pMboCtrl->TransitionReason, MboAttrValueToString(MBO_ATTR_AP_TRANS_REASON, pMboCtrl->TransitionReason)));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ReAssocDelay \t\t 0x%x [second]\n",
			pMboCtrl->ReAssocDelay));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("MboCapIndication \t\t 0x%x\n",
			pMboCtrl->MboCapIndication));
	}
#endif /* CONFIG_AP_SUPPORT */


	return TRUE;
}

VOID MboIndicateOneNRtoDaemonByBssEntry(
	PRTMP_ADAPTER pAd,
	struct wifi_dev *pWdev,
	BSS_ENTRY *pBssEntry)
{
	DAEMON_EVENT_NR_LIST NeighborRepList;
	DAEMON_NEIGHBOR_REP_INFO *pNeighborEntry = NULL;

	pNeighborEntry = &NeighborRepList.EvtNRInfo[0];
	NdisZeroMemory(pNeighborEntry, sizeof(DAEMON_NEIGHBOR_REP_INFO));
	/* Fill the NR entry */
	MboUpdateNRElement(pAd, pWdev, pBssEntry, pNeighborEntry);
	/* Fill the NR list */
	NeighborRepList.Newlist = FALSE;
    NeighborRepList.TotalNum = 1;
	NeighborRepList.CurrNum = 1;
	/* Send the list to daemon (which contains only one entry) */
	WextMboSendNeighborReportToDaemonEvent(pAd->net_dev,
						  &NeighborRepList,
						  sizeof(DAEMON_EVENT_NR_LIST));
}

VOID MboBssTermStart(
	PRTMP_ADAPTER pAd,
	UINT8 countdown)
{
	UINT32 hTsf, lTsf, tTsf;
	MBO_EVENT_BSS_TERM bss_term;

	AsicGetTsfTime(pAd, &hTsf, &lTsf, HW_BSSID_0);

	/* Calcuate target TSF */
	/* tTsf = countdown << 20; */ /* sec to usec */
	tTsf = countdown*1000000; /* sec to usec */
	tTsf = lTsf + tTsf;
	if (tTsf < lTsf)
		hTsf++;
	bss_term.TsfLowPart = tTsf;
	bss_term.TsfHighPart = hTsf;

	/* Send an event to daemon */
	WextMboSendBssTermToDaemonEvent(pAd->net_dev,
					&bss_term,
					MBO_MSG_AP_TERMINATION,
					sizeof(MBO_EVENT_BSS_TERM));

	/* Trigger BSS Termination count Down */
	pAd->MboBssTermCountDown = countdown + 1; /* plus 1 in case substract immediately */

}

VOID MboCheckBssTermination(
	PRTMP_ADAPTER pAd)
{
	struct wifi_dev *wdev = NULL;

	if (pAd->MboBssTermCountDown != 0) {
	    if ((--pAd->MboBssTermCountDown) == 0) {
		wdev = &pAd->ApCfg.MBSSID[0].wdev;
		RTMP_BSS_TERMINATION(pAd, wdev);
		}
	}
}
#endif /* MBO_SUPPORT */

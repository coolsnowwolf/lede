/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2013, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:

	Abstract:

	Revision History:
	Who		When			What
	--------	----------		----------------------------------------------
*/

#define RTMP_MODULE_OS

#ifdef RT_CFG80211_SUPPORT

#include "rt_config.h"


VOID CFG80211_Announce802_3Packet(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk, UCHAR FromWhichBSSID)
{
#ifdef CONFIG_AP_SUPPORT

	if (RX_BLK_TEST_FLAG(pRxBlk, fRX_STA))
		announce_or_forward_802_3_pkt(pAd, pRxBlk->pRxPacket, wdev_search_by_idx(pAd, FromWhichBSSID), OPMODE_AP);
	else
#endif /* CONFIG_AP_SUPPORT */
	{
		announce_or_forward_802_3_pkt(pAd, pRxBlk->pRxPacket, wdev_search_by_idx(pAd, FromWhichBSSID), OPMODE_STA);
	}
}

BOOLEAN CFG80211_CheckActionFrameType(
	IN RTMP_ADAPTER *pAd,
	IN PUCHAR preStr,
	IN PUCHAR pData,
	IN UINT32 length)
{
	BOOLEAN isP2pFrame = FALSE;
	struct ieee80211_mgmt *mgmt;

	mgmt = (struct ieee80211_mgmt *)pData;

	if (ieee80211_is_mgmt(mgmt->frame_control)) {
		if (ieee80211_is_probe_resp(mgmt->frame_control)) {
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("CFG80211_PKT: %s ProbeRsp Frame %d\n", preStr, pAd->LatchRfRegs.Channel));

			if (!mgmt->u.probe_resp.timestamp) {
				struct timespec64 tv;

				ktime_get_real_ts64(&tv);
				mgmt->u.probe_resp.timestamp = ((UINT64) tv.tv_sec * 1000000) + tv.tv_nsec;
			}
		}
#ifdef HOSTAPD_11R_SUPPORT
		else if (ieee80211_is_auth(mgmt->frame_control))
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("CFG80211_PKT: %s AUTH Frame\n", preStr));
		else if (ieee80211_is_reassoc_req(mgmt->frame_control))
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("CFG80211_PKT: %s REASSOC Req Frame\n", preStr));
		else if (ieee80211_is_reassoc_resp(mgmt->frame_control))
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("CFG80211_PKT: %s REASSOC Resp Frame\n", preStr));
#endif	 /* HOSTAPD_11R_SUPPORT */
#ifdef HOSTAPD_OWE_SUPPORT
		else if (ieee80211_is_assoc_resp(mgmt->frame_control))
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("CFG80211_PKT: %s ASSOC Resp Frame\n", preStr));
#endif
		else if (ieee80211_is_disassoc(mgmt->frame_control))
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("CFG80211_PKT: %s DISASSOC Frame\n", preStr));
		else if (ieee80211_is_deauth(mgmt->frame_control))
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("CFG80211_PKT: %s Deauth Frame on %d\n", preStr, pAd->LatchRfRegs.Channel));
		else if (ieee80211_is_action(mgmt->frame_control)) {
			PP2P_PUBLIC_FRAME pFrame = (PP2P_PUBLIC_FRAME)pData;
#ifdef HOSTAPD_11K_SUPPORT

		if (mgmt->u.action.category == CATEGORY_RM) {
			isP2pFrame = TRUE; /* CFG80211_HandleP2pMgmtFrame is being used for sending all pkts to upper layer, so isP2pFrame variable is being used */
		}
#endif
			if ((pFrame->p80211Header.FC.SubType == SUBTYPE_ACTION) &&
				(pFrame->Category == CATEGORY_PUBLIC) &&
				(pFrame->Action == ACTION_WIFI_DIRECT)) {
				isP2pFrame = TRUE;

				switch (pFrame->Subtype) {
				case GO_NEGOCIATION_REQ:
					MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("CFG80211_PKT: %s GO_NEGOCIACTION_REQ %d\n",
							 preStr, pAd->LatchRfRegs.Channel));
					break;

				case GO_NEGOCIATION_RSP:
					MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("CFG80211_PKT: %s GO_NEGOCIACTION_RSP %d\n",
							 preStr, pAd->LatchRfRegs.Channel));
					break;

				case GO_NEGOCIATION_CONFIRM:
					MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("CFG80211_PKT: %s GO_NEGOCIACTION_CONFIRM %d\n",
							 preStr,  pAd->LatchRfRegs.Channel));
					break;

				case P2P_PROVISION_REQ:
					MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("CFG80211_PKT: %s P2P_PROVISION_REQ %d\n",
							 preStr, pAd->LatchRfRegs.Channel));
					break;

				case P2P_PROVISION_RSP:
					MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("CFG80211_PKT: %s P2P_PROVISION_RSP %d\n",
							 preStr, pAd->LatchRfRegs.Channel));
					break;

				case P2P_INVITE_REQ:
					MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("CFG80211_PKT: %s P2P_INVITE_REQ %d\n",
							 preStr, pAd->LatchRfRegs.Channel));
					break;

				case P2P_INVITE_RSP:
					MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("CFG80211_PKT: %s P2P_INVITE_RSP %d\n",
							 preStr, pAd->LatchRfRegs.Channel));
					break;

				case P2P_DEV_DIS_REQ:
					MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("CFG80211_PKT: %s P2P_DEV_DIS_REQ %d\n",
							 preStr, pAd->LatchRfRegs.Channel));
					break;

				case P2P_DEV_DIS_RSP:
					MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("CFG80211_PKT: %s P2P_DEV_DIS_RSP %d\n",
							 preStr, pAd->LatchRfRegs.Channel));
					break;
				}
			} else if ((pFrame->p80211Header.FC.SubType == SUBTYPE_ACTION) &&
					   (pFrame->Category == CATEGORY_PUBLIC) &&
					   ((pFrame->Action == ACTION_GAS_INITIAL_REQ)	 ||
						(pFrame->Action == ACTION_GAS_INITIAL_RSP)	 ||
						(pFrame->Action == ACTION_GAS_COMEBACK_REQ) ||
						(pFrame->Action == ACTION_GAS_COMEBACK_RSP)))
				isP2pFrame = TRUE;

			else
				MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("CFG80211_PKT: %s ACTION Frame with Channel%d\n", preStr, pAd->LatchRfRegs.Channel));
		} else
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("CFG80211_PKT: %s UNKOWN MGMT FRAME TYPE\n", preStr));
	} else
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("CFG80211_PKT: %s UNKOWN FRAME TYPE\n", preStr));

	return isP2pFrame;
}


#ifdef CFG_TDLS_SUPPORT
BOOLEAN CFG80211_HandleTdlsDiscoverRespFrame(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk, UCHAR OpMode)
{
#ifdef MT_MAC
#else
	RXWI_STRUC *pRxWI = pRxBlk->pRxWI;
	PNET_DEV pNetDev = NULL;
#endif /* MT_MAC */
	PHEADER_802_11 pHeader = pRxBlk->pHeader;
	UINT32 freq;
	/* PNDIS_PACKET pRxPacket = pRxBlk->pRxPacket; */
	PP2P_PUBLIC_FRAME pFrame = (PP2P_PUBLIC_FRAME)pHeader;
	UINT32 MPDUtotalByteCnt = 0;
#ifdef MT_MAC
	MPDUtotalByteCnt = pRxBlk->MPDUtotalByteCnt;
#else
	MPDUtotalByteCnt = pRxWI->RXWI_N.MPDUtotalByteCnt;
#endif /* MT_MAC */

	if ((pFrame->p80211Header.FC.SubType == SUBTYPE_ACTION) &&
		(pFrame->Category == CATEGORY_PUBLIC) &&
		(pFrame->Action == ACTION_TDLS_DISCOVERY_RSP)) {
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s : GOT ACTION_TDLS_DISCOVERY_RSP ACTION: 0x%x\n", __func__, pFrame->Action));
		MAP_CHANNEL_ID_TO_KHZ(pAd->LatchRfRegs.Channel, freq);
		freq /= 1000;
		CFG80211OS_RxMgmt(pAd->net_dev, freq, (PUCHAR)pHeader, MPDUtotalByteCnt);
		return TRUE;
	}

	/* report TDLS DiscoverRespFrame to supplicant , but it does nothing , events.c line 2432 */
	return FALSE;
}
#endif /* CFG_TDLS_SUPPORT */

#if defined(HOSTAPD_11R_SUPPORT) || defined(HOSTAPD_SAE_SUPPORT)
VOID CFG80211_AuthReqHandler(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk, AUTH_FRAME_INFO *auth_info)
{
	PFRAME_802_11 Fr = (PFRAME_802_11)pRxBlk->FC;
	ULONG MsgLen = pRxBlk->DataSize;
#ifdef BAND_STEERING
	struct wifi_dev *wdev = NULL;
	HEADER_802_11 AuthReqHdr;
#endif /* BAND_STEERING */

	if (auth_info->auth_alg == AUTH_MODE_FT)
	{
		PEID_STRUCT eid_ptr;
		UCHAR *Ptr;
		UCHAR WPA2_OUI[3] = {0x00, 0x0F, 0xAC};
		PFT_INFO pFtInfo = &auth_info->FtInfo;

		NdisZeroMemory(pFtInfo, sizeof(FT_INFO));

		Ptr = &Fr->Octet[6];
		eid_ptr = (PEID_STRUCT) Ptr;

	    /* get variable fields from payload and advance the pointer */
		while (((UCHAR *)eid_ptr + eid_ptr->Len + 1) < ((UCHAR *)Fr + MsgLen)) {
			switch (eid_ptr->Eid) {
			case IE_FT_MDIE:
				FT_FillMdIeInfo(eid_ptr, &pFtInfo->MdIeInfo);
				break;

			case IE_FT_FTIE:
				FT_FillFtIeInfo(eid_ptr, &pFtInfo->FtIeInfo);
				break;

			case IE_FT_RIC_DATA:
				/* record the pointer of first RDIE. */
				if (pFtInfo->RicInfo.pRicInfo == NULL) {
					pFtInfo->RicInfo.pRicInfo = &eid_ptr->Eid;
					pFtInfo->RicInfo.Len = ((UCHAR *)Fr + MsgLen)
								- (UCHAR *)eid_ptr + 1;
				}

				if ((pFtInfo->RicInfo.RicIEsLen + eid_ptr->Len + 2) < MAX_RICIES_LEN) {
					NdisMoveMemory(&pFtInfo->RicInfo.RicIEs[pFtInfo->RicInfo.RicIEsLen],
									&eid_ptr->Eid, eid_ptr->Len + 2);
					pFtInfo->RicInfo.RicIEsLen += eid_ptr->Len + 2;
				}
				break;

			case IE_FT_RIC_DESCRIPTOR:
				if ((pFtInfo->RicInfo.RicIEsLen + eid_ptr->Len + 2) < MAX_RICIES_LEN) {
					NdisMoveMemory(&pFtInfo->RicInfo.RicIEs[pFtInfo->RicInfo.RicIEsLen],
									&eid_ptr->Eid, eid_ptr->Len + 2);
					pFtInfo->RicInfo.RicIEsLen += eid_ptr->Len + 2;
				}
				break;

			case IE_RSN:
				if (NdisEqualMemory(&eid_ptr->Octet[2], WPA2_OUI, sizeof(WPA2_OUI))) {
					NdisMoveMemory(pFtInfo->RSN_IE, eid_ptr, eid_ptr->Len + 2);
					pFtInfo->RSNIE_Len = eid_ptr->Len + 2;
				}
				break;

			default:
				break;
			}
			eid_ptr = (PEID_STRUCT)((UCHAR *)eid_ptr + 2 + eid_ptr->Len);
		}
	}

#ifdef BAND_STEERING
	NdisZeroMemory(&AuthReqHdr, sizeof(HEADER_802_11));
	NdisCopyMemory((UCHAR *)&(AuthReqHdr.FC), pRxBlk->FC, sizeof(FRAME_CONTROL));
	wdev = wdev_search_by_address(pAd, AuthReqHdr.Addr1);

	if (pAd->ApCfg.BandSteering
	) {
		BOOLEAN bBndStrgCheck = TRUE;
		ULONG MsgType = APMT2_PEER_AUTH_REQ;
		struct raw_rssi_info rssi_info;

		NdisMoveMemory((rssi_info.raw_rssi), pRxBlk->rx_signal.raw_rssi, sizeof(struct raw_rssi_info));
		bBndStrgCheck = BndStrg_CheckConnectionReq(pAd, wdev, auth_info->addr2, &rssi_info, MsgType, NULL);

		if (bBndStrgCheck == FALSE) {
			APPeerAuthSimpleRspGenAndSend(pAd, &AuthReqHdr, auth_info->auth_alg, auth_info->auth_seq + 1, MLME_UNSPECIFY_FAIL);
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("AUTH - check failed.\n"));
			return;
		}

	}
#endif /* BAND_STEERING */
}


VOID CFG80211_AssocReqHandler(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk)
{
	struct wifi_dev *wdev = NULL;
	struct dev_rate_info *rate;
	BSS_STRUCT *pMbss;
	IE_LISTS *ie_list = NULL;
	HEADER_802_11 AssocReqHdr;
	USHORT StatusCode = MLME_SUCCESS;
	UCHAR MaxSupportedRate = 0;
	UCHAR PhyMode, FlgIs11bSta;
	UCHAR i;
#if defined(TXBF_SUPPORT) && defined(VHT_TXBF_SUPPORT)
/*	UCHAR ucETxBfCap; */
#endif /* TXBF_SUPPORT && VHT_TXBF_SUPPORT */
	MAC_TABLE_ENTRY *pEntry = NULL;
	STA_TR_ENTRY *tr_entry;
#ifdef DBG
	UCHAR isReassoc = 0;
#endif /* DBG */
	BOOLEAN bACLReject = FALSE;
#ifdef WSC_AP_SUPPORT
	WSC_CTRL *wsc_ctrl;
#endif /* WSC_AP_SUPPORT */
#ifdef RT_BIG_ENDIAN
	UINT32 tmp_1;
	UINT64 tmp_2;
#endif /*RT_BIG_ENDIAN*/
	ADD_HT_INFO_IE *addht;
#ifdef GREENAP_SUPPORT
/*    struct greenap_ctrl *greenap = &pAd->ApCfg.greenap; */
#endif /* GREENAP_SUPPORT */
#ifdef MBO_SUPPORT
	BOOLEAN bMboReject = FALSE;
#endif /* MBO_SUPPORT */
#ifdef WAPP_SUPPORT
/*	UINT8 wapp_cnnct_stage = WAPP_ASSOC; */
	UINT16 wapp_assoc_fail = NOT_FAILURE;
#endif /* WAPP_SUPPORT */

	NdisZeroMemory(&AssocReqHdr, sizeof(HEADER_802_11));
	NdisCopyMemory((UCHAR *)&(AssocReqHdr.FC), pRxBlk->FC, sizeof(FRAME_CONTROL));

	/* its reassoc frame */
	if (AssocReqHdr.FC.SubType == SUBTYPE_REASSOC_REQ)
		isReassoc = 1;


	/* disallow new association */
	if (pAd->ApCfg.BANClass3Data == TRUE) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("Disallow new Association\n"));
#ifdef WAPP_SUPPORT
		wapp_assoc_fail = DISALLOW_NEW_ASSOCI;
#endif /* WAPP_SUPPORT */
	/*	goto assoc_check; */
		return;
	}

	/* allocate memory */
	os_alloc_mem(NULL, (UCHAR **)&ie_list, sizeof(IE_LISTS));

	if (ie_list == NULL) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s(): mem alloc failed\n", __func__));
#ifdef WAPP_SUPPORT
		wapp_assoc_fail = MLME_NO_RESOURCE;
#endif /* WAPP_SUPPORT */
	/*	goto assoc_check; */
		return;
	}

	NdisZeroMemory(ie_list, sizeof(IE_LISTS));

	/* fill ie_list */
	if (!PeerAssocReqCmmSanity(pAd, isReassoc, (VOID *)pRxBlk->FC, pRxBlk->DataSize, ie_list))
		goto LabelOK;

	pEntry = MacTableLookup(pAd, ie_list->Addr2);

	if (!pEntry) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("NoAuth MAC - %02x:%02x:%02x:%02x:%02x:%02x\n",
				  PRINT_MAC(ie_list->Addr2)));
		goto LabelOK;
	}

	/* attach ie_list to pEntry */
	pEntry->ie_list = ie_list;

	if (!VALID_MBSS(pAd, pEntry->func_tb_idx)) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s():pEntry bounding invalid wdev(apidx=%d)\n",
				  __func__, pEntry->func_tb_idx));
		goto LabelOK;
	}

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s():pEntry->func_tb_idx=%d\n",
			  __func__, pEntry->func_tb_idx));
	wdev = wdev_search_by_address(pAd, ie_list->Addr1);

	if (wdev == NULL) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("Wrong Addr1 - %02x:%02x:%02x:%02x:%02x:%02x\n",
				  PRINT_MAC(ie_list->Addr1)));
		goto LabelOK;
	}

	/* WPS_BandSteering Support */
#if defined(BAND_STEERING) && defined(WSC_INCLUDED)
	if (pAd->ApCfg.BandSteering) {
		PWSC_CTRL pWscControl = NULL;
		PBND_STRG_CLI_ENTRY cli_entry = NULL;
		PBND_STRG_CLI_TABLE table = NULL;
		PWPS_WHITELIST_ENTRY wps_entry = NULL;
		PBS_LIST_ENTRY bs_whitelist_entry = NULL;

		pWscControl = &pAd->ApCfg.MBSSID[wdev->func_idx].WscControl;
		table = Get_BndStrgTable(pAd, wdev->func_idx);
		if (table && table->bEnabled) {

			cli_entry = BndStrg_TableLookup(table, pEntry->Addr);
			wps_entry = FindWpsWhiteListEntry(&table->WpsWhiteList, pEntry->Addr);

			/* WPS: special WIN7 case: no wps/rsn ie in assoc */
			/* and send eapol start, consider it as wps station */
			if ((ie_list->RSNIE_Len == 0) && (IS_AKM_WPA_CAPABILITY_Entry(wdev))
				&& (pWscControl->WscConfMode != WSC_DISABLE))
				ie_list->bWscCapable = TRUE;

			/* in case probe did not have wps ie, but assoc has, create wps whitelist entry here */
			if (!wps_entry && ie_list->bWscCapable && pWscControl->bWscTrigger) {

				NdisAcquireSpinLock(&table->WpsWhiteListLock);
				AddWpsWhiteList(&table->WpsWhiteList, pEntry->Addr);
				NdisReleaseSpinLock(&table->WpsWhiteListLock);
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("channel %u: WPS Assoc req: STA %02x:%02x:%02x:%02x:%02x:%02x wps whitelisted\n",
				 table->Channel, PRINT_MAC(pEntry->Addr)));
				BND_STRG_PRINTQAMSG(table, pEntry->Addr, ("ASSOC STA %02x:%02x:%02x:%02x:%02x:%02x channel %u  added in WPS Whitelist\n",
				PRINT_MAC(pEntry->Addr), table->Channel));
			}

			bs_whitelist_entry = FindBsListEntry(&table->WhiteList, pEntry->Addr);

			/* handle case: where a client has wps ie in probe, not have bndstrg entry/bndstrg whitelist, */
			/* but doing normal assoc: dont allow */
			if ((pWscControl->bWscTrigger) && (!cli_entry) && (!ie_list->bWscCapable) && (!bs_whitelist_entry)) {
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\n%s():reject assoc:bWscTrigger:%d, cli_entry:%p,bWscCapable:%d, bs_whitelist_entry:%p\n",
				 __func__, pWscControl->bWscTrigger, cli_entry, ie_list->bWscCapable, bs_whitelist_entry));
				BND_STRG_PRINTQAMSG(table, pEntry->Addr, ("STA %02x:%02x:%02x:%02x:%02x:%02x Normal Assoc Rejected for BS unauthorized client\n",
				PRINT_MAC(pEntry->Addr)));
				goto LabelOK;
			}
		}
	}
#endif
	pMbss = &pAd->ApCfg.MBSSID[wdev->func_idx];
	tr_entry = &pAd->MacTab.tr_entry[pEntry->tr_tb_idx];

#ifdef WSC_AP_SUPPORT
	wsc_ctrl = &wdev->WscControl;
#endif /* WSC_AP_SUPPORT */
	PhyMode = wdev->PhyMode;
	rate = &wdev->rate;
	addht = wlan_operate_get_addht(wdev);

	if (!OPSTATUS_TEST_FLAG_WDEV(wdev, fOP_AP_STATUS_MEDIA_STATE_CONNECTED)) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s(): AP is not ready, disallow new Association\n", __func__));
		goto LabelOK;
	}

		FlgIs11bSta = 1;

		for (i = 0; i < ie_list->SupportedRatesLen; i++) {
			if (((ie_list->SupportedRates[i] & 0x7F) != 2) &&
				((ie_list->SupportedRates[i] & 0x7F) != 4) &&
				((ie_list->SupportedRates[i] & 0x7F) != 11) &&
				((ie_list->SupportedRates[i] & 0x7F) != 22)) {
				FlgIs11bSta = 0;
				break;
			}
		}
#ifdef CONFIG_MAP_SUPPORT
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				("%s():IS_MAP_ENABLE=%d\n", __func__, IS_MAP_ENABLE(pAd)));
		if (IS_MAP_ENABLE(pAd)) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				("%s():Elem->MsgLen=%d, ASSOC_REQ_LEN = %d\n",
					__func__, pRxBlk->DataSize, (INT)ASSOC_REQ_LEN));
		NdisMoveMemory(pEntry->assoc_req_frame, (VOID *)pRxBlk->FC, ASSOC_REQ_LEN);
	}
#endif


#ifdef MBO_SUPPORT
	if (!MBO_AP_ALLOW_ASSOC(wdev)) {
		StatusCode = MLME_ASSOC_REJ_UNABLE_HANDLE_STA;
		bMboReject = TRUE;
#ifdef WAPP_SUPPORT
		wapp_assoc_fail = MLME_UNABLE_HANDLE_STA;
#endif /* WAPP_SUPPORT */
		/* goto SendAssocResponse; */
		return;
	}
#endif /* MBO_SUPPORT */

	/* clear the previous Pairwise key table */
	if ((pEntry->Aid != 0)
#ifdef DOT11R_FT_SUPPORT
	&& (!IS_FT_STA(pEntry))
#endif /* DOT11R_FT_SUPPORT */
		&& ((!IS_AKM_OPEN(pEntry->SecConfig.AKMMap)) || (!IS_AKM_SHARED(pEntry->SecConfig.AKMMap))
#ifdef DOT1X_SUPPORT
			|| IS_IEEE8021X(&pEntry->SecConfig)
#endif /* DOT1X_SUPPORT */
		   )) {
		ASIC_SEC_INFO Info = {0};
		/* clear GTK state */
		pEntry->SecConfig.Handshake.GTKState = REKEY_NEGOTIATING;
		NdisZeroMemory(&pEntry->SecConfig.PTK, LEN_MAX_PTK);
		/* Set key material to Asic */
		os_zero_mem(&Info, sizeof(ASIC_SEC_INFO));
		Info.Operation = SEC_ASIC_REMOVE_PAIRWISE_KEY;
		Info.Wcid = pEntry->wcid;
		/* Set key material to Asic */
		HW_ADDREMOVE_KEYTABLE(pAd, &Info);
#if defined(DOT1X_SUPPORT) && !defined(RADIUS_ACCOUNTING_SUPPORT)

		/* Notify 802.1x daemon to clear this sta info */
		if (IS_AKM_1X(pEntry->SecConfig.AKMMap)
			|| IS_IEEE8021X(&pEntry->SecConfig))
			DOT1X_InternalCmdAction(pAd, pEntry, DOT1X_DISCONNECT_ENTRY);

#endif /* DOT1X_SUPPORT */
	}

#ifdef WSC_AP_SUPPORT
	/* since sta has been left, ap should receive EapolStart and EapRspId again. */
	pEntry->Receive_EapolStart_EapRspId = 0;
	pEntry->bWscCapable = ie_list->bWscCapable;
#ifdef WSC_V2_SUPPORT

	if ((wsc_ctrl->WscV2Info.bEnableWpsV2) &&
		(wsc_ctrl->WscV2Info.bWpsEnable == FALSE))
		;
	else
#endif /* WSC_V2_SUPPORT */
	{
		if (pEntry->func_tb_idx < pAd->ApCfg.BssidNum) {
			if (MAC_ADDR_EQUAL(pEntry->Addr, wsc_ctrl->EntryAddr)) {
				BOOLEAN Cancelled;

				RTMPZeroMemory(wsc_ctrl->EntryAddr, MAC_ADDR_LEN);
				RTMPCancelTimer(&wsc_ctrl->EapolTimer, &Cancelled);
				wsc_ctrl->EapolTimerRunning = FALSE;
			}
		}

		if ((ie_list->RSNIE_Len == 0) &&
			(IS_AKM_WPA_CAPABILITY_Entry(wdev)) &&
			(wsc_ctrl->WscConfMode != WSC_DISABLE))
			pEntry->bWscCapable = TRUE;
	}

#endif /* WSC_AP_SUPPORT */

		/* for hidden SSID sake, SSID in AssociateRequest should be fully verified */
		if ((ie_list->SsidLen != pMbss->SsidLen) ||
			(NdisEqualMemory(ie_list->Ssid, pMbss->Ssid, ie_list->SsidLen) == 0))
			goto LabelOK;
#ifdef WSC_V2_SUPPORT
			/* Do not check ACL when WPS V2 is enabled and ACL policy is positive. */
			if ((pEntry->bWscCapable) &&
				(wsc_ctrl->WscConfMode != WSC_DISABLE) &&
				(wsc_ctrl->WscV2Info.bEnableWpsV2) &&
				(wsc_ctrl->WscV2Info.bWpsEnable) &&
				(MAC_ADDR_EQUAL(wsc_ctrl->EntryAddr, ZERO_MAC_ADDR)))
				;
			else
#endif /* WSC_V2_SUPPORT */
				/* set a flag for sending Assoc-Fail response to unwanted STA later. */
				if (!ApCheckAccessControlList(pAd, ie_list->Addr2, pEntry->func_tb_idx))
					bACLReject = TRUE;
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("assoc - MBSS(%d), receive reassoc request from %02x:%02x:%02x:%02x:%02x:%02x\n",
				  pEntry->func_tb_idx, PRINT_MAC(ie_list->Addr2)));
		/* supported rates array may not be sorted. sort it and find the maximum rate */
		for (i = 0; i < ie_list->SupportedRatesLen; i++) {
			if (MaxSupportedRate < (ie_list->SupportedRates[i] & 0x7f))
				MaxSupportedRate = ie_list->SupportedRates[i] & 0x7f;
		}
		/*
			Assign RateLen here or we will select wrong rate table in
			APBuildAssociation() when 11N compile option is disabled.
		*/
		pEntry->RateLen = ie_list->SupportedRatesLen;
		pEntry->MaxSupportedRate = MaxSupportedRate;
		RTMPSetSupportMCS(pAd,
						  OPMODE_AP,
						  pEntry,
						  ie_list->SupportedRates,
						  ie_list->SupportedRatesLen,
						  NULL,
						  0,
#ifdef DOT11_VHT_AC
						  ie_list->vht_cap_len,
						  &ie_list->vht_cap,
#endif /* DOT11_VHT_AC */
						  &ie_list->HTCapability,
						  ie_list->ht_cap_len);


	MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s <<<<<<\n", __func__));
	return;

LabelOK:
#ifdef RT_CFG80211_SUPPORT
	if (StatusCode != MLME_SUCCESS)
		CFG80211_ApStaDelSendEvent(pAd, pEntry->Addr, pEntry->wdev->if_dev);
#endif /* RT_CFG80211_SUPPORT */
	if (ie_list != NULL)
		os_free_mem(ie_list);

	return;
}
#endif /* HOSTAPD_11R_SUPPORT */

BOOLEAN CFG80211_HandleP2pMgmtFrame(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk, UCHAR OpMode)
{
#ifndef MT_MAC
	RXWI_STRUC *pRxWI = pRxBlk->pRxWI;
#endif /* !MT_MAC */
	HEADER_802_11 Header;
	PHEADER_802_11 pHeader = &Header;
	PNET_DEV pNetDev;
#ifdef RT_CFG80211_SUPPORT
    struct wifi_dev *pWdev = WdevSearchByBssid(pAd, pRxBlk->Addr1);
#endif
	PCFG80211_CTRL pCfg80211_ctrl = &pAd->cfg80211_ctrl;
	UINT32 freq;
	UINT32 MPDUtotalByteCnt = 0;

    if (pWdev == NULL) {
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_LOUD,
			("%s return , can't find wdev for %02x:%02x:%02x:%02x:%02x:%02x\n"
			, __FUNCTION__, PRINT_MAC(pHeader->Addr2)));
		pNetDev = CFG80211_GetEventDevice(pAd);
	} else {
		pNetDev = pWdev->if_dev;
	}

	NdisZeroMemory(&Header, sizeof(HEADER_802_11));
	NdisCopyMemory((UCHAR *) &(pHeader->FC), pRxBlk->FC, sizeof(FRAME_CONTROL));
	NdisCopyMemory(pHeader->Addr1, pRxBlk->Addr1, MAC_ADDR_LEN);
	NdisCopyMemory(pHeader->Addr2, pRxBlk->Addr2, MAC_ADDR_LEN);
	NdisCopyMemory(pHeader->Addr3, pRxBlk->Addr3, MAC_ADDR_LEN);
	pHeader->Duration = pRxBlk->Duration;
#ifdef MT_MAC
	MPDUtotalByteCnt = pRxBlk->MPDUtotalByteCnt;
#else
	MPDUtotalByteCnt = pRxWI->RXWI_N.MPDUtotalByteCnt;
#endif /* MT_MAC */

	if ((pHeader->FC.SubType == SUBTYPE_PROBE_REQ) ||
#if defined(HOSTAPD_11R_SUPPORT) || defined(HOSTAPD_SAE_SUPPORT)  || defined(HOSTAPD_OWE_SUPPORT)
		 (pHeader->FC.SubType == SUBTYPE_AUTH) ||
#endif
#if defined(HOSTAPD_11R_SUPPORT) || defined(HOSTAPD_OWE_SUPPORT)
		 (pHeader->FC.SubType == SUBTYPE_REASSOC_REQ) ||
#endif
#ifdef HOSTAPD_OWE_SUPPORT
		(pHeader->FC.SubType == SUBTYPE_ASSOC_REQ) ||
#endif
		((pHeader->FC.SubType == SUBTYPE_ACTION) &&
		CFG80211_CheckActionFrameType(pAd, "RX", (PUCHAR)pRxBlk->pData, MPDUtotalByteCnt))) {
		MAP_CHANNEL_ID_TO_KHZ(pAd->LatchRfRegs.Channel, freq);
		freq /= 1000;

		if (((pHeader->FC.SubType == SUBTYPE_PROBE_REQ) &&
			 (pCfg80211_ctrl->cfg80211MainDev.Cfg80211RegisterProbeReqFrame == TRUE)) ||
			((pHeader->FC.SubType == SUBTYPE_ACTION)  /*&& ( pAd->Cfg80211RegisterActionFrame == TRUE)*/)
#if defined(HOSTAPD_11R_SUPPORT) || defined(HOSTAPD_SAE_SUPPORT) || defined(HOSTAPD_OWE_SUPPORT)
			 || (pHeader->FC.SubType == SUBTYPE_AUTH)
#endif
#if defined(HOSTAPD_11R_SUPPORT) || defined(HOSTAPD_OWE_SUPPORT)

			 || (pHeader->FC.SubType == SUBTYPE_REASSOC_REQ)
#endif
#ifdef HOSTAPD_OWE_SUPPORT

					||	(pHeader->FC.SubType == SUBTYPE_ASSOC_REQ)
#endif

				) {
			MTWF_LOG(DBG_CAT_P2P, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("MAIN STA RtmpOsCFG80211RxMgmt OK!! TYPE = %d, freq = %d, %02x:%02x:%02x:%02x:%02x:%02x\n",
					 pHeader->FC.SubType, freq, PRINT_MAC(pHeader->Addr2)));
		if (pHeader->FC.SubType == SUBTYPE_PROBE_REQ) {
#ifdef BAND_STEERING
		PEER_PROBE_REQ_PARAM ProbeReqParam = { {0} };

			if (pAd->ApCfg.BandSteering) {
				if (PeerProbeReqSanity(pAd, (VOID *)pRxBlk->pData, pRxBlk->DataSize, &ProbeReqParam) == FALSE) {
					MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("%s():PeerProbeReqSanity failed!\n", __func__));
					return TRUE; /*drop when SSID sanity check fails in ProbeReq */
				}
			}
#endif

		if (IS_BROADCAST_MAC_ADDR(pRxBlk->Addr1)) {
			int apidx;
		/*deliver broadcast frame to all virtual interface */
			for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++) {
				BSS_STRUCT *mbss = &pAd->ApCfg.MBSSID[apidx];
				struct wifi_dev *wdev = &mbss->wdev;
				if (wdev->if_dev != NULL) {
#ifdef BAND_STEERING
		if (pAd->ApCfg.BandSteering
		) {
			BOOLEAN bBndStrgCheck = TRUE;
			ULONG MsgType = APMT2_PEER_PROBE_REQ;
			struct raw_rssi_info rssi_info;

			NdisMoveMemory((rssi_info.raw_rssi), pRxBlk->rx_signal.raw_rssi, sizeof(struct raw_rssi_info));
			bBndStrgCheck = BndStrg_CheckConnectionReq(pAd, wdev, ProbeReqParam.Addr2, &rssi_info, MsgType, &ProbeReqParam);

			if (bBndStrgCheck == FALSE)
				return TRUE;
		}

#endif /* BAND_STEERING */

					CFG80211OS_RxMgmt(wdev->if_dev, freq, pRxBlk->pData, MPDUtotalByteCnt);
				}
			}
		} else {
#ifdef BAND_STEERING
		if (pAd->ApCfg.BandSteering
		) {
			BOOLEAN bBndStrgCheck = TRUE;
			ULONG MsgType = APMT2_PEER_PROBE_REQ;
			struct raw_rssi_info rssi_info;

			NdisMoveMemory((rssi_info.raw_rssi), pRxBlk->rx_signal.raw_rssi, sizeof(struct raw_rssi_info));
			bBndStrgCheck = BndStrg_CheckConnectionReq(pAd, pWdev, ProbeReqParam.Addr2, &rssi_info, MsgType, &ProbeReqParam);

			if (bBndStrgCheck == FALSE)
				return TRUE;
		}

#endif /* BAND_STEERING */
			CFG80211OS_RxMgmt(pNetDev, freq, pRxBlk->pData, MPDUtotalByteCnt);
		}
#if defined(HOSTAPD_11R_SUPPORT) || defined(HOSTAPD_SAE_SUPPORT) || defined(HOSTAPD_OWE_SUPPORT)
	} else if (pHeader->FC.SubType == SUBTYPE_AUTH) {
		AUTH_FRAME_INFO *auth_info = &(pCfg80211_ctrl->auth_info);
		PFRAME_802_11 Fr = ((PFRAME_802_11)pRxBlk->pData);

		COPY_MAC_ADDR(auth_info->addr1,  pHeader->Addr1);		/* BSSID */
		COPY_MAC_ADDR(auth_info->addr2,  pHeader->Addr2);		/* SA */
		/*  TODO: shiang-usw, how about the endian issue here?? */
		NdisMoveMemory(&auth_info->auth_alg,	&Fr->Octet[0], 2);
		NdisMoveMemory(&auth_info->auth_seq,	&Fr->Octet[2], 2);
		NdisMoveMemory(&auth_info->auth_status, &Fr->Octet[4], 2);

		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:auth req frame: alg:%d, seq:%d, status:%d\n",
					__func__, auth_info->auth_alg, auth_info->auth_seq, auth_info->auth_status));
		if ((auth_info->auth_alg == AUTH_MODE_FT) || (auth_info->auth_alg == AUTH_MODE_SAE)) {
			/* handle FT Auth or SAE Auth in hostapd */
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: calling CFG80211OS_RxMgmt: auth req frame\n", __func__));
			CFG80211_AuthReqHandler(pAd, pRxBlk, auth_info);
			CFG80211OS_RxMgmt(pNetDev, freq, pRxBlk->pData, MPDUtotalByteCnt);
		} else {
			/* handle normal auth in driver */
			if (pWdev && IS_AKM_OWE(pWdev->SecConfig.AKMMap))
				CFG80211OS_RxMgmt(pNetDev, freq, pRxBlk->pData, MPDUtotalByteCnt);

			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: handle open auth req frame in driver\n", __func__));
			return FALSE;
		}
#endif
#ifdef HOSTAPD_OWE_SUPPORT
		} else if (pHeader->FC.SubType == SUBTYPE_ASSOC_REQ) {
			MAC_TABLE_ENTRY *pEntry = NULL;

			pEntry = MacTableLookup(pAd, pHeader->Addr2);
			if (pEntry && pWdev && IS_AKM_OWE(pWdev->SecConfig.AKMMap)) {
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						("pEntry Found for assoc- %02x:%02x:%02x:%02x:%02x:%02x\n",
						PRINT_MAC(pHeader->Addr2)));

				CFG80211_AssocReqHandler(pAd, pRxBlk);
				if (pEntry->ie_list && pEntry->ie_list->ecdh_ie.length) {
					MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						("ASSOC Req sending to CFG layer for OWE mode \n"));
					CFG80211OS_RxMgmt(pNetDev, freq, pRxBlk->pData, MPDUtotalByteCnt);
				} else {
					MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						("ASSOC Req OWE ie not found. Handle in Driver \n"));
					if (pEntry->ie_list != NULL)
						os_free_mem(pEntry->ie_list);
					return FALSE;
				}
			} else {
				      return FALSE;
				}
#endif
#if defined(HOSTAPD_11R_SUPPORT) || defined(HOSTAPD_OWE_SUPPORT)
	} else if (pHeader->FC.SubType == SUBTYPE_REASSOC_REQ) {
		MAC_TABLE_ENTRY *pEntry = NULL;

		pEntry = MacTableLookup(pAd, pHeader->Addr2);
		if (pEntry) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						("pEntry Found for reassoc- %02x:%02x:%02x:%02x:%02x:%02x\n",
						PRINT_MAC(pHeader->Addr2)));

			CFG80211_AssocReqHandler(pAd, pRxBlk);
#ifdef HOSTAPD_11R_SUPPORT
			if (pEntry->ie_list && pEntry->ie_list->FtInfo.MdIeInfo.Len) {
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: calling CFG80211OS_RxMgmt:FT reassoc req frame\n", __func__));
				CFG80211OS_RxMgmt(pNetDev, freq, pRxBlk->pData, MPDUtotalByteCnt);
			} else
#endif
#ifdef HOSTAPD_OWE_SUPPORT
			if (pEntry->ie_list && pEntry->ie_list->ecdh_ie.length) {
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					("OWE Reassoc Req Handle in CFG Layer \n"));
				CFG80211OS_RxMgmt(pNetDev, freq, pRxBlk->pData, MPDUtotalByteCnt);
			} else
#endif
			{
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: not FT reassoc , handle in normal driver\n", __func__));
				if (pEntry->ie_list != NULL)
					os_free_mem(pEntry->ie_list);
				return FALSE;
			}
		} else {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: pEntry not found , handle in normal driver\n", __func__));
			return FALSE;
		}
#endif
	} else
		CFG80211OS_RxMgmt(pNetDev, freq, pRxBlk->pData, MPDUtotalByteCnt);
		if (OpMode == OPMODE_AP)
			return TRUE;
		}
	}

	return FALSE;
}


#endif /* RT_CFG80211_SUPPORT */


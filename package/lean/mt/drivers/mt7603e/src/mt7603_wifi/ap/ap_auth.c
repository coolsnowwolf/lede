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
    auth.c
 
    Abstract:
    Handle de-auth request from local MLME
 
    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    John Chang  08-04-2003    created for 11g soft-AP
 */

#include "rt_config.h"
#ifdef DOT11R_FT_SUPPORT
#include "ft.h"
#endif /* DOT11R_FT_SUPPORT */

#ifdef WH_EZ_SETUP
#ifdef DUAL_CHIP
extern NDIS_SPIN_LOCK ez_conn_perm_lock;
#endif
#endif

/*
    ==========================================================================
    Description:
        MLME message sanity check
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
 */
static BOOLEAN PeerDeauthReqSanity(
    IN PRTMP_ADAPTER pAd, 
    IN VOID *Msg, 
    IN ULONG MsgLen, 
    OUT PUCHAR pAddr2, 
    OUT	UINT16	*SeqNum, 
    OUT USHORT *Reason) 
{
    PFRAME_802_11 Fr = (PFRAME_802_11)Msg;

    COPY_MAC_ADDR(pAddr2, &Fr->Hdr.Addr2);
	*SeqNum = Fr->Hdr.Sequence;
    NdisMoveMemory(Reason, &Fr->Octet[0], 2);

    return TRUE;
}

VOID ap_mlme_broadcast_deauth_req_action(
		IN PRTMP_ADAPTER pAd,
		IN MLME_QUEUE_ELEM * Elem)
{
	MLME_BROADCAST_DEAUTH_REQ_STRUCT	*pInfo;
	HEADER_802_11			Hdr;
	PUCHAR					pOutBuffer = NULL;
	NDIS_STATUS				NStatus;
	ULONG					FrameLen = 0;
	MAC_TABLE_ENTRY			*pEntry;
	UCHAR					apidx = 0;
	struct wifi_dev *wdev;
	int wcid, startWcid;

	startWcid = 1;
	pInfo = (PMLME_BROADCAST_DEAUTH_REQ_STRUCT)Elem->Msg;
	if (!MAC_ADDR_EQUAL(pInfo->Addr, BROADCAST_ADDR))
		return;
	wdev = pInfo->wdev;
	apidx = wdev->func_idx;
	for (wcid = startWcid; VALID_UCAST_ENTRY_WCID(wcid); wcid++) {
		pEntry = &pAd->MacTab.Content[wcid];
		if (pEntry->wdev != wdev)
			continue;
		RTMPSendWirelessEvent(pAd, IW_DEAUTH_EVENT_FLAG,
			pEntry->Addr, 0, 0);
		ApLogEvent(pAd, pInfo->Addr, EVENT_DISASSOCIATED);
		MacTableDeleteEntry(pAd, wcid, pEntry->Addr);
	}
	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);
	if (NStatus != NDIS_STATUS_SUCCESS)
		return;
	MgtMacHeaderInit(pAd, &Hdr, SUBTYPE_DEAUTH, 0, pInfo->Addr,
					pAd->ApCfg.MBSSID[apidx].wdev.if_addr,
					pAd->ApCfg.MBSSID[apidx].wdev.bssid);
	MakeOutgoingFrame(pOutBuffer,				&FrameLen,
					  sizeof(HEADER_802_11), &Hdr,
					  2, &pInfo->Reason,
					  END_OF_ARGS);
	MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
	MlmeFreeMemory(pAd, pOutBuffer);
}

void ap_send_broadcast_deauth(void *ad_obj, struct wifi_dev *wdev)
{
	MLME_BROADCAST_DEAUTH_REQ_STRUCT  *pInfo = NULL;
	MLME_QUEUE_ELEM *Elem;
	RTMP_ADAPTER *pAd = ad_obj;

	MlmeAllocateMemory(pAd, (UCHAR **)&Elem);
	if (Elem == NULL)
		return;
	if (Elem) {
		pInfo = (MLME_BROADCAST_DEAUTH_REQ_STRUCT *)Elem->Msg;
		pInfo->wdev = wdev;
		Elem->Wcid = WCID_ALL;
		pInfo->Reason = MLME_UNSPECIFY_FAIL;
		NdisCopyMemory(pInfo->Addr, BROADCAST_ADDR, MAC_ADDR_LEN);
		APMlmeDeauthReqAction(pAd, Elem);
		MlmeFreeMemory(pAd, Elem);
	}
}


/*
    ==========================================================================
    Description:
        Upper Layer request to kick out a STA
    ==========================================================================
 */
VOID APMlmeDeauthReqAction(
    IN PRTMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    MLME_DEAUTH_REQ_STRUCT	*pInfo;
    HEADER_802_11			Hdr;
    PUCHAR					pOutBuffer = NULL;
    NDIS_STATUS				NStatus;
    ULONG					FrameLen = 0;
    MAC_TABLE_ENTRY			*pEntry;
	UCHAR					apidx;
#if (defined(WH_EZ_SETUP)  || defined (WH_EVENT_NOTIFIER))
	struct wifi_dev 		*wdev;
#endif


    pInfo = (MLME_DEAUTH_REQ_STRUCT *)Elem->Msg;

#ifdef WH_EZ_SETUP
	if(IS_ADPTR_EZ_SETUP_ENABLED(pAd))
		EZ_DEBUG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,(" ---> %s, wcid = %d\n", __FUNCTION__,Elem->Wcid));
#endif

    if (Elem->Wcid < MAX_LEN_OF_MAC_TABLE)
    {
#ifdef WH_EZ_SETUP
		if(IS_ADPTR_EZ_SETUP_ENABLED(pAd))
			EZ_DEBUG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("Valid unicast entry\n"));
#endif
    
		pEntry = &pAd->MacTab.Content[Elem->Wcid];
		if (!pEntry)
			return;

#if (defined(WH_EZ_SETUP)  || defined (WH_EVENT_NOTIFIER))
		wdev = pEntry->wdev;
#endif
		
#ifdef WAPI_SUPPORT
		WAPI_InternalCmdAction(pAd, 
							   pEntry->AuthMode, 
							   pEntry->func_tb_idx, 
							   pEntry->Addr, 
							   WAI_MLME_DISCONNECT);		
#endif /* WAPI_SUPPORT */
		
		/* send wireless event - for deauthentication */
		RTMPSendWirelessEvent(pAd, IW_DEAUTH_EVENT_FLAG, pInfo->Addr, 0, 0);  

#ifdef ALL_NET_EVENT
		wext_send_event(pEntry->wdev->if_dev,
			pEntry->Addr,
			pEntry->bssid,
			pAd->CommonCfg.Channel,
			RTMPAvgRssi(pAd, &pEntry->RssiSample),
			FBT_LINK_OFFLINE_NOTIFY);
#endif /* ALL_NET_EVENT */

		ApLogEvent(pAd, pInfo->Addr, EVENT_DISASSOCIATED);

		apidx = pEntry->func_tb_idx;
#ifdef WIFI_DIAG
		if (IS_ENTRY_CLIENT(pEntry))
			DiagConnError(pAd, pEntry->func_tb_idx, pEntry->Addr,
				DIAG_CONN_DEAUTH, pInfo->Reason);
#endif
#ifdef CONN_FAIL_EVENT
		if (IS_ENTRY_CLIENT(pEntry))
			ApSendConnFailMsg(pAd,
				pAd->ApCfg.MBSSID[pEntry->func_tb_idx].Ssid,
				pAd->ApCfg.MBSSID[pEntry->func_tb_idx].SsidLen,
				pEntry->Addr,
				pInfo->Reason);
#endif
        /* 1. remove this STA from MAC table */
        MacTableDeleteEntry(pAd, Elem->Wcid, pInfo->Addr);

        /* 2. send out DE-AUTH request frame */
        NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);
        if (NStatus != NDIS_STATUS_SUCCESS) 
            return;

        DBGPRINT(RT_DEBUG_TRACE,
				("AUTH - Send DE-AUTH req to %02x:%02x:%02x:%02x:%02x:%02x\n",
				PRINT_MAC(pInfo->Addr)));
           		
        MgtMacHeaderInit(pAd, &Hdr, SUBTYPE_DEAUTH, 0, pInfo->Addr,
						pAd->ApCfg.MBSSID[apidx].wdev.if_addr,
						pAd->ApCfg.MBSSID[apidx].wdev.bssid);
        MakeOutgoingFrame(pOutBuffer,				&FrameLen, 
                          sizeof(HEADER_802_11),	&Hdr, 
                          2,						&pInfo->Reason, 
                          END_OF_ARGS);
        MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);

        MlmeFreeMemory(pAd, pOutBuffer);

#ifdef WH_EVENT_NOTIFIER
		{
			EventHdlr pEventHdlrHook = NULL;
			pEventHdlrHook = GetEventNotiferHook(WHC_DRVEVNT_STA_LEAVE);
			if(pEventHdlrHook && wdev)
				pEventHdlrHook(pAd, wdev, pInfo->Addr, Elem->Channel);
		}
#endif /* WH_EVENT_NOTIFIER */
	} else {
		ap_mlme_broadcast_deauth_req_action(pAd, Elem);
	}
}


static VOID APPeerDeauthReqAction(
    IN PRTMP_ADAPTER pAd, 
    IN PMLME_QUEUE_ELEM Elem) 
{
	UCHAR Addr2[MAC_ADDR_LEN];
	UINT16 Reason, SeqNum;
	MAC_TABLE_ENTRY *pEntry;
#if (defined(WH_EZ_SETUP) || defined(WH_EVENT_NOTIFIER))
	struct wifi_dev *wdev;
#endif

#ifdef WH_EZ_SETUP
	if(IS_ADPTR_EZ_SETUP_ENABLED(pAd))
		EZ_DEBUG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,("AUTH_RSP - APPeerDeauthReqAction\n"));
#endif


	if (! PeerDeauthReqSanity(pAd, Elem->Msg, Elem->MsgLen, Addr2, &SeqNum, &Reason)) 
		return;

	pEntry = NULL;

	/*pEntry = MacTableLookup(pAd, Addr2); */
	if (Elem->Wcid < MAX_LEN_OF_MAC_TABLE)
	{
		pEntry = &pAd->MacTab.Content[Elem->Wcid];
#if (defined(WH_EZ_SETUP) || defined(WH_EVENT_NOTIFIER))
		wdev = pEntry->wdev;
#endif

		{ 
			/*
				Add Hotspot2.0 Rlease 1 Prestested Code
			*/
			BSS_STRUCT  *pMbss = &pAd->ApCfg.MBSSID[pEntry->func_tb_idx];
			PFRAME_802_11 Fr = (PFRAME_802_11)Elem->Msg;
#ifdef DBG
			unsigned char *tmp = (unsigned char *)pMbss->wdev.bssid;
			unsigned char *tmp2 = (unsigned char *)&Fr->Hdr.Addr1;
#endif /* DBG */
			if (memcmp(&Fr->Hdr.Addr1, pMbss->wdev.bssid, 6) != 0)
			{
				DBGPRINT(RT_DEBUG_INFO,
					("da not match bssid,bssid:0x%02x%02x%02x%02x%02x%02x, addr1:0x%02x%02x%02x%02x%02x%02x\n",
					*tmp, *(tmp+1), *(tmp+2), *(tmp+3), *(tmp+4), *(tmp+5), *tmp2, *(tmp2+1), *(tmp2+2), *(tmp2+3), *(tmp2+4), *(tmp2+5)));
				return;
			}
			else
				DBGPRINT(RT_DEBUG_INFO,("da match,0x%02x%02x%02x%02x%02x%02x\n", 
				*tmp, *(tmp+1), *(tmp+2), *(tmp+3), *(tmp+4), *(tmp+5)));
		}
#ifdef DOT1X_SUPPORT    
		/* Notify 802.1x daemon to clear this sta info */
		if (pEntry->AuthMode == Ndis802_11AuthModeWPA || 
			pEntry->AuthMode == Ndis802_11AuthModeWPA2 ||
			pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.IEEE8021X)
			DOT1X_InternalCmdAction(pAd, pEntry, DOT1X_DISCONNECT_ENTRY);
#endif /* DOT1X_SUPPORT */

#ifdef WAPI_SUPPORT
		WAPI_InternalCmdAction(pAd, 
								pEntry->AuthMode,
								pEntry->func_tb_idx,
								pEntry->Addr,
								WAI_MLME_DISCONNECT);
#endif /* WAPI_SUPPORT */

		/* send wireless event - for deauthentication */
		RTMPSendWirelessEvent(pAd, IW_DEAUTH_EVENT_FLAG, Addr2, 0, 0);  

#ifdef ALL_NET_EVENT
		wext_send_event(pEntry->wdev->if_dev,
			pEntry->Addr,
			pEntry->bssid,
			pAd->CommonCfg.Channel,
			RTMPAvgRssi(pAd, &pEntry->RssiSample),
			FBT_LINK_OFFLINE_NOTIFY);
#endif /* ALL_NET_EVENT */

		ApLogEvent(pAd, Addr2, EVENT_DISASSOCIATED);

		if (pEntry->CMTimerRunning == TRUE)
		{
			/*
				If one who initilized Counter Measure deauth itself,
				AP doesn't log the MICFailTime
			*/
			pAd->ApCfg.aMICFailTime = pAd->ApCfg.PrevaMICFailTime;
		}

#ifdef WIFI_DIAG
		if (pEntry && IS_ENTRY_CLIENT(pEntry))
			DiagConnError(pAd, pEntry->func_tb_idx, pEntry->Addr,
				DIAG_CONN_DEAUTH, REASON_DEAUTH_STA_LEAVING);
#endif
#ifdef CONN_FAIL_EVENT
		if (IS_ENTRY_CLIENT(pEntry))
			ApSendConnFailMsg(pAd,
				pAd->ApCfg.MBSSID[pEntry->func_tb_idx].Ssid,
				pAd->ApCfg.MBSSID[pEntry->func_tb_idx].SsidLen,
				pEntry->Addr,
				REASON_DEAUTH_STA_LEAVING);
#endif
		MacTableDeleteEntry(pAd, Elem->Wcid, Addr2);

		DBGPRINT(RT_DEBUG_TRACE,
					("AUTH - receive DE-AUTH(seq-%d) from "
					"%02x:%02x:%02x:%02x:%02x:%02x, reason=%d\n",
					SeqNum, PRINT_MAC(Addr2), Reason));

#ifdef MAC_REPEATER_SUPPORT
		if (pAd->ApCfg.bMACRepeaterEn == TRUE)
		{
			UCHAR apCliIdx, CliIdx, isLinkValid;
			REPEATER_CLIENT_ENTRY *pReptEntry = NULL;

			pReptEntry = RTMPLookupRepeaterCliEntry(pAd, TRUE, Addr2, TRUE, &isLinkValid);
			if (pReptEntry && (pReptEntry->CliConnectState != 0))
			{
				apCliIdx = pReptEntry->MatchApCliIdx;
				CliIdx = pReptEntry->MatchLinkIdx;
				MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_DISCONNECT_REQ, 0, NULL,
								(64 + MAX_EXT_MAC_ADDR_SIZE*apCliIdx + CliIdx));
				RTMP_MLME_HANDLER(pAd);
				//RTMPRemoveRepeaterEntry(pAd, apCliIdx, CliIdx);
			}
		}
#endif /* MAC_REPEATER_SUPPORT */


#ifdef WH_EVENT_NOTIFIER
		{
			EventHdlr pEventHdlrHook = NULL;
			pEventHdlrHook = GetEventNotiferHook(WHC_DRVEVNT_STA_LEAVE);
			if(pEventHdlrHook && wdev)
				pEventHdlrHook(pAd, wdev, Addr2, Elem->Channel);
		}
#endif /* WH_EVENT_NOTIFIER */

	}
}


/* 
    ==========================================================================
    Description:
        MLME message sanity check
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
 */
static BOOLEAN APPeerAuthSanity(
    IN RTMP_ADAPTER *pAd,
    IN VOID *Msg,
    IN ULONG MsgLen,
    IN AUTH_FRAME_INFO *auth_info)
{
	PFRAME_802_11 Fr = (PFRAME_802_11)Msg;

	COPY_MAC_ADDR(auth_info->addr1,  &Fr->Hdr.Addr1);		/* BSSID */
	COPY_MAC_ADDR(auth_info->addr2,  &Fr->Hdr.Addr2);		/* SA */
	// TODO: shiang-usw, how about the endian issue here??
	NdisMoveMemory(&auth_info->auth_alg,    &Fr->Octet[0], 2);
	NdisMoveMemory(&auth_info->auth_seq,    &Fr->Octet[2], 2);
	NdisMoveMemory(&auth_info->auth_status, &Fr->Octet[4], 2);

#ifdef WH_EZ_SETUP
	if(IS_ADPTR_EZ_SETUP_ENABLED(pAd))
		auth_info->auth_alg = le2cpu16(auth_info->auth_alg);
#endif

	if (auth_info->auth_alg == AUTH_MODE_OPEN) 
	{
		if (auth_info->auth_seq == 1 || auth_info->auth_seq == 2) 
			return TRUE;
		else 
		{
			DBGPRINT(RT_DEBUG_TRACE, ("%s(): fail - wrong Seg# (=%d)\n", 
						__FUNCTION__, auth_info->auth_seq));
			return FALSE;
		}
	} 
	else if (auth_info->auth_alg == AUTH_MODE_KEY) 
	{
		if (auth_info->auth_seq == 1 || auth_info->auth_seq == 4) 
			return TRUE;
		else if (auth_info->auth_seq == 2 || auth_info->auth_seq == 3) 
		{
			NdisMoveMemory(auth_info->Chtxt, &Fr->Octet[8], CIPHER_TEXT_LEN);
			return TRUE;
		}
		else
		{
			DBGPRINT(RT_DEBUG_TRACE, ("%s(): fail - wrong Seg# (=%d)\n",
						__FUNCTION__, auth_info->auth_seq));
			return FALSE;
		}
	} 
#ifdef DOT11R_FT_SUPPORT
	else if (auth_info->auth_alg == AUTH_MODE_FT)
	{
		PEID_STRUCT eid_ptr;
		UCHAR *Ptr;
		UCHAR WPA2_OUI[3]={0x00,0x0F,0xAC};
		PFT_INFO pFtInfo = &auth_info->FtInfo;

		NdisZeroMemory(pFtInfo, sizeof(FT_INFO));

		Ptr = &Fr->Octet[6];
		eid_ptr = (PEID_STRUCT) Ptr;

	    /* get variable fields from payload and advance the pointer */
		while(((UCHAR*)eid_ptr + eid_ptr->Len + 1) < ((UCHAR*)Fr + MsgLen))
		{
			switch(eid_ptr->Eid)
			{
				case IE_FT_MDIE:
					FT_FillMdIeInfo(eid_ptr, &pFtInfo->MdIeInfo);
					break;

				case IE_FT_FTIE:
					FT_FillFtIeInfo(eid_ptr, &pFtInfo->FtIeInfo);
					break;

				case IE_FT_RIC_DATA:
					/* record the pointer of first RDIE. */
					if (pFtInfo->RicInfo.pRicInfo == NULL)
					{
						pFtInfo->RicInfo.pRicInfo = &eid_ptr->Eid;
						pFtInfo->RicInfo.Len = ((UCHAR*)Fr + MsgLen)
												- (UCHAR*)eid_ptr + 1;
					}

					if ((pFtInfo->RicInfo.RicIEsLen + eid_ptr->Len + 2) < MAX_RICIES_LEN)
					{
						NdisMoveMemory(&pFtInfo->RicInfo.RicIEs[pFtInfo->RicInfo.RicIEsLen],
										&eid_ptr->Eid, eid_ptr->Len + 2);
						pFtInfo->RicInfo.RicIEsLen += eid_ptr->Len + 2;
					}
					break;

					
				case IE_FT_RIC_DESCRIPTOR:
					if ((pFtInfo->RicInfo.RicIEsLen + eid_ptr->Len + 2) < MAX_RICIES_LEN)
					{
						NdisMoveMemory(&pFtInfo->RicInfo.RicIEs[pFtInfo->RicInfo.RicIEsLen],
										&eid_ptr->Eid, eid_ptr->Len + 2);
						pFtInfo->RicInfo.RicIEsLen += eid_ptr->Len + 2;
					}
					break;

				case IE_RSN:
					if (NdisEqualMemory(&eid_ptr->Octet[2], WPA2_OUI, sizeof(WPA2_OUI)))
					{
	                    NdisMoveMemory(pFtInfo->RSN_IE, eid_ptr, eid_ptr->Len + 2);
						pFtInfo->RSNIE_Len = eid_ptr->Len + 2;
					}
					break;

				default:
					break;
			}
	        eid_ptr = (PEID_STRUCT)((UCHAR*)eid_ptr + 2 + eid_ptr->Len);        
		}
	}
#endif /* DOT11R_FT_SUPPORT */
#ifdef WH_EZ_SETUP
	else if(IS_ADPTR_EZ_SETUP_ENABLED(pAd) && (auth_info->auth_alg == AUTH_MODE_EZ)){
		EZ_DEBUG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
			("%s(): receive easy setup auth request (alg=0x%02x)\n",
			__FUNCTION__, auth_info->auth_alg));
		return TRUE;
	}
#endif /* WH_EZ_SETUP */
#ifdef DOT11_SAE_SUPPORT
	else if (auth_info->auth_alg == AUTH_MODE_SAE) {
		if (auth_info->auth_seq != SAE_COMMIT_SEQ && auth_info->auth_seq != SAE_CONFIRM_SEQ)
			return FALSE;
	}
#endif /* DOT11_SAE_SUPPORT */

    else 
    {
        DBGPRINT(RT_DEBUG_TRACE, ("%s(): fail - wrong algorithm (=%d)\n", 
					__FUNCTION__, auth_info->auth_alg));
        return FALSE;
    }

	return TRUE;
}


/*
    ==========================================================================
    Description:
        Send out a Authentication (response) frame
    ==========================================================================
*/
VOID APPeerAuthSimpleRspGenAndSend(
    IN PRTMP_ADAPTER pAd, 
    IN PHEADER_802_11 pHdr, 
    IN USHORT Alg, 
    IN USHORT Seq, 
    IN USHORT StatusCode) 
{
    HEADER_802_11     AuthHdr;
    ULONG             FrameLen = 0;
    PUCHAR            pOutBuffer = NULL;
    NDIS_STATUS       NStatus;


    NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);
    if (NStatus != NDIS_STATUS_SUCCESS) 
        return;

    if (StatusCode == MLME_SUCCESS)
    {
        DBGPRINT(RT_DEBUG_TRACE, ("AUTH_RSP - Send AUTH response (SUCCESS)...\n"));
	}
    else
    {
        /* For MAC wireless client(Macintosh), need to send AUTH_RSP with Status Code (fail reason code) to reject it. */
        DBGPRINT(RT_DEBUG_TRACE, ("AUTH_RSP - Peer AUTH fail (Status = %d)...\n", StatusCode));
    }

	MgtMacHeaderInit(pAd, &AuthHdr, SUBTYPE_AUTH, 0, pHdr->Addr2, 
						pHdr->Addr1,
						pHdr->Addr1);
	MakeOutgoingFrame(pOutBuffer,				&FrameLen,
					  sizeof(HEADER_802_11),	&AuthHdr,
					  2,						&Alg,
					  2,						&Seq,
					  2,						&StatusCode,
					  END_OF_ARGS);
	MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
	MlmeFreeMemory(pAd, pOutBuffer);
}


static VOID APPeerAuthReqAtIdleAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	INT i;
	USHORT RspReason;
	AUTH_FRAME_INFO auth_info;
	UINT32 apidx;
	PHEADER_802_11 pRcvHdr;
	HEADER_802_11 AuthHdr;
	PUCHAR pOutBuffer = NULL;
	NDIS_STATUS NStatus;
	ULONG FrameLen = 0;
	MAC_TABLE_ENTRY *pEntry;
#ifdef DOT11W_PMF_SUPPORT  
	STA_TR_ENTRY *tr_entry;
#endif /* DOT11W_PMF_SUPPORT */
	UCHAR ChTxtIe = 16, ChTxtLen = CIPHER_TEXT_LEN;
#ifdef DOT11R_FT_SUPPORT
	PFT_CFG pFtCfg;
	PFT_INFO pFtInfoBuf;
#endif /* DOT11R_FT_SUPPORT */
	BSS_STRUCT *pMbss;
	struct wifi_dev *wdev = NULL;
	UINT32 u4MaxMBSSIDSize = sizeof(pAd->ApCfg.MBSSID)/sizeof(pAd->ApCfg.MBSSID[0]);
#ifdef WAPP_SUPPORT
	UINT8 wapp_cnnct_stage = WAPP_AUTH;
	UINT16 wapp_auth_fail = NOT_FAILURE;
#endif/* WAPP_SUPPORT */

#ifdef WH_EZ_SETUP
	if(IS_ADPTR_EZ_SETUP_ENABLED(pAd))
		EZ_DEBUG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("AUTH - APPeerAuthReqAtIdleAction\n"));
#endif


	if (pAd->ApCfg.BANClass3Data == TRUE)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("Disallow new Association\n"));
		/*return;*/
#ifdef WAPP_SUPPORT
		wapp_auth_fail = DISALLOW_NEW_ASSOCI;
#endif/* WAPP_SUPPORT */
		goto auth_failure;
	}

	if (!APPeerAuthSanity(pAd, Elem->Msg, Elem->MsgLen, &auth_info)) {
		/*return;*/
#ifdef WAPP_SUPPORT
		wapp_auth_fail = PEER_REQ_SANITY_FAIL;
#endif /* WAPP_SUPPORT */
		goto auth_failure;
	}

	/* Find which MBSSID to be authenticate */
	apidx = get_apidx_by_addr(pAd, auth_info.addr1);
	if ((apidx >= pAd->ApCfg.BssidNum) || (apidx >= u4MaxMBSSIDSize))
	{
		DBGPRINT(RT_DEBUG_TRACE, ("AUTH - Bssid not found\n"));
		/*return;*/
#ifdef WAPP_SUPPORT
		wapp_auth_fail = BSSID_NOT_FOUND;
#endif/* WAPP_SUPPORT */
		goto auth_failure;
	}

	pMbss = &pAd->ApCfg.MBSSID[apidx];
	wdev = &pMbss->wdev;
	ASSERT((wdev->func_idx == apidx));

	if ((wdev->if_dev == NULL) || ((wdev->if_dev != NULL) &&
		!(RTMP_OS_NETDEV_STATE_RUNNING(wdev->if_dev))))
	{
		DBGPRINT(RT_DEBUG_TRACE, ("AUTH - Bssid IF didn't up yet.\n"));
		/*return;*/
#ifdef WAPP_SUPPORT
		wapp_auth_fail = BSSID_IF_NOT_READY;
#endif /* WAPP_SUPPORT */
		goto auth_failure;
	}

#ifdef WH_EZ_SETUP
	if (IS_EZ_SETUP_ENABLED(wdev) && (ez_is_connection_allowed(wdev) == FALSE)) {
		/*return;*/
#ifdef WAPP_SUPPORT
		wapp_auth_fail = EZ_CONNECT_DISALLOW;
#endif /* WAPP_SUPPORT */
		goto auth_failure;
	}
	if ((auth_info.auth_alg == AUTH_MODE_EZ)
		&& (!IS_EZ_SETUP_ENABLED(wdev))) {
			 //    ez_update_connection_permission(pAd,NULL,EZ_ALLOW_ALL);
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
			("AUTH - Easy setup function is disabled. Reject easy setup auth request.\n"));
		/*return;*/
#ifdef WAPP_SUPPORT
		wapp_auth_fail = EZ_SETUP_FUNC_DISABLED;
#endif/* WAPP_SUPPORT */
		goto auth_failure;
	}
#endif /* WH_EZ_SETUP */

	pEntry = MacTableLookup(pAd, auth_info.addr2);
	if (pEntry && IS_ENTRY_CLIENT(pEntry))
	{
		/* reset NoDataIdleCount to prevent unexpected STA assoc timeout and kicked by MacTableMaintenance */
		pEntry->NoDataIdleCount = 0;
#ifdef DOT11W_PMF_SUPPORT  
		tr_entry = &pAd->MacTab.tr_entry[pEntry->wcid];
		
                if ((CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_PMF_CAPABLE)) 
                        && (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED))
                        goto SendAuth;
#endif /* DOT11W_PMF_SUPPORT */
	
		if (!RTMPEqualMemory(auth_info.addr1, pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.bssid, MAC_ADDR_LEN))
		{					
			MacTableDeleteEntry(pAd, pEntry->wcid, pEntry->Addr);
			pEntry = NULL;
			DBGPRINT(RT_DEBUG_WARN, ("AUTH - Bssid does not match\n"));
#ifdef WAPP_SUPPORT
			wapp_auth_fail = BSSID_MISMATCH;
#endif/* WAPP_SUPPORT */
			goto auth_failure;

		}
		else
		{
			if (pEntry->bIAmBadAtheros == TRUE)
			{
				AsicUpdateProtect(pAd, 8, ALLN_SETPROTECT, FALSE, FALSE);
				DBGPRINT(RT_DEBUG_TRACE, ("Atheros Problem. Turn on RTS/CTS!!!\n"));
				pEntry->bIAmBadAtheros = FALSE;
			}

#ifdef DOT11_N_SUPPORT
			BASessionTearDownALL(pAd, pEntry->wcid);
#endif /* DOT11_N_SUPPORT */
			ASSERT(pEntry->Aid == Elem->Wcid);
		}
	}

#ifdef DOT11W_PMF_SUPPORT
SendAuth:
#endif /* DOT11W_PMF_SUPPORT */

	pRcvHdr = (PHEADER_802_11)(Elem->Msg);
	DBGPRINT(RT_DEBUG_TRACE,
			("AUTH - MBSS(%d), Rcv AUTH seq#%d, Alg=%d, Status=%d from "
			"[wcid=%d]%02x:%02x:%02x:%02x:%02x:%02x\n",
			apidx, auth_info.auth_seq, auth_info.auth_alg, 
			auth_info.auth_status, Elem->Wcid, 
			PRINT_MAC(auth_info.addr2)));
	if(pEntry) {
		INT32 prev = -1;
		prev = (INT32)pEntry->AuthAssocNotInProgressFlag;
		pEntry->AuthAssocNotInProgressFlag = 0;
		DBGPRINT(RT_DEBUG_TRACE,("===[%s] prev_val = %d AuthAssocNotInProgressFlag = %u \n", 
			__FUNCTION__, prev, pEntry->AuthAssocNotInProgressFlag));
	}

#ifdef WH_EZ_SETUP
	if ((auth_info.auth_alg == AUTH_MODE_EZ)
		&& IS_EZ_SETUP_ENABLED(wdev)) {
		/* 
			Do not check ACL when easy setup is enabled 
			and ACL policy is positive. 
		*/
		EZ_DEBUG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("%s: This is an easy setup device.\n", __FUNCTION__));
	}
	else
#endif /* WH_EZ_SETUP */	
#ifdef WSC_V2_SUPPORT
	/* Do not check ACL when WPS V2 is enabled and ACL policy is positive. */
	if ((pMbss->WscControl.WscConfMode != WSC_DISABLE) &&
		(pMbss->WscControl.WscV2Info.bEnableWpsV2) &&
		(pMbss->WscControl.WscV2Info.bWpsEnable) &&
		(pMbss->AccessControlList.Policy == 1))
		; 
	else
#endif /* WSC_V2_SUPPORT */
	/* fail in ACL checking => send an AUTH-Fail seq#2. */
    if (! ApCheckAccessControlList(pAd, auth_info.addr2, apidx))
    {
		ASSERT(auth_info.auth_seq == 1);
		ASSERT(pEntry == NULL);
		APPeerAuthSimpleRspGenAndSend(pAd, pRcvHdr, auth_info.auth_alg, auth_info.auth_seq + 1, MLME_UNSPECIFY_FAIL);

#ifdef WIFI_DIAG
		DiagConnError(pAd, apidx, auth_info.addr2, DIAG_CONN_ACL_BLK, 0);
#endif
#ifdef CONN_FAIL_EVENT
		ApSendConnFailMsg(pAd,
			pAd->ApCfg.MBSSID[apidx].Ssid,
			pAd->ApCfg.MBSSID[apidx].SsidLen,
			auth_info.addr2,
			REASON_DECLINED);
#endif
		/* If this STA exists, delete it. */
		if (pEntry)
			MacTableDeleteEntry(pAd, pEntry->wcid, pEntry->Addr);

		RTMPSendWirelessEvent(pAd, IW_MAC_FILTER_LIST_EVENT_FLAG, auth_info.addr2, wdev->wdev_idx, 0);

		DBGPRINT(RT_DEBUG_TRACE,
				("Failed in ACL checking => send an AUTH seq#2 with "
				"Status code = %d\n", MLME_UNSPECIFY_FAIL));

#ifdef WH_EVENT_NOTIFIER
		{
			EventHdlr pEventHdlrHook = NULL;
			pEventHdlrHook = GetEventNotiferHook(WHC_DRVEVNT_STA_AUTH_REJECT);
			if(pEventHdlrHook && wdev)
				pEventHdlrHook(pAd, wdev, auth_info.addr2, Elem);
		}
#endif /* WH_EVENT_NOTIFIER */
		
		/*return;*/
#ifdef WAPP_SUPPORT
		wapp_auth_fail = ACL_CHECK_FAIL;
#endif /* WAPP_SUPPORT */
		goto auth_failure;
    }
#ifdef BAND_STEERING
		if ((pAd->ApCfg.BandSteering)
#ifdef WH_EZ_SETUP
			&& !((wdev != NULL) && (IS_EZ_SETUP_ENABLED(wdev)) && (auth_info.auth_alg == AUTH_MODE_EZ))
#endif
		) {
			BOOLEAN bBndStrgCheck = TRUE;
			bBndStrgCheck = BndStrg_CheckConnectionReq(pAd, wdev, auth_info.addr2, Elem, NULL);
			if (bBndStrgCheck == FALSE) {
				APPeerAuthSimpleRspGenAndSend(pAd, pRcvHdr, auth_info.auth_alg, auth_info.auth_seq + 1, MLME_UNSPECIFY_FAIL);
				DBGPRINT(RT_DEBUG_TRACE, ("AUTH - BndStrg check failed.\n"));
				/*return;*/
#ifdef WAPP_SUPPORT
				wapp_auth_fail = BND_STRG_CONNECT_CHECK_FAIL;
#endif /* WAPP_SUPPORT */
#ifdef WIFI_DIAG
			DiagConnError(pAd, apidx, auth_info.addr2, DIAG_CONN_BAND_STE, 0);
#endif
#ifdef CONN_FAIL_EVENT
			ApSendConnFailMsg(pAd,
						pAd->ApCfg.MBSSID[apidx].Ssid,
						pAd->ApCfg.MBSSID[apidx].SsidLen,
						auth_info.addr2,
						REASON_UNSPECIFY);
#endif
				goto auth_failure;
			}
		}
#endif /* BAND_STEERING */
#ifdef DOT11R_FT_SUPPORT
	pFtCfg = &pMbss->FtCfg;
	if ((pFtCfg->FtCapFlag.Dot11rFtEnable)
		&& (auth_info.auth_alg == AUTH_MODE_FT))
	{
		USHORT result;

		if (!pEntry)
			pEntry = MacTableInsertEntry(pAd, auth_info.addr2, wdev, ENTRY_CLIENT, OPMODE_AP, TRUE);
		
		if (pEntry != NULL)
		{
			os_alloc_mem(pAd, (UCHAR **)&pFtInfoBuf, sizeof(FT_INFO));

			if (pFtInfoBuf)
			{
				result = FT_AuthReqHandler(pAd, pEntry, &auth_info.FtInfo, pFtInfoBuf);
				if (result == MLME_SUCCESS)
				{
					NdisMoveMemory(&pEntry->MdIeInfo, &auth_info.FtInfo.MdIeInfo, sizeof(FT_MDIE_INFO));

					pEntry->AuthState = AS_AUTH_OPEN;
					pEntry->Sst = SST_AUTH;
					} else if (result == MLME_FAIL_NO_RESOURCE) {
						MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
							("%s - give up this AUTH pkt => Query R1KH from backbone(Wcid%d, %d)\n",
							 __func__, pEntry->wcid, pEntry->FT_R1kh_CacheMiss_Times));
						os_free_mem(NULL, pFtInfoBuf);
						/*return;*/
#ifdef WAPP_SUPPORT
						wapp_auth_fail = MLME_NO_RESOURCE;
#endif /* WAPP_SUPPORT */
						goto auth_failure;
					}

				FT_EnqueueAuthReply(pAd, pRcvHdr, auth_info.auth_alg, 2, result,
							&pFtInfoBuf->MdIeInfo, &pFtInfoBuf->FtIeInfo, NULL,
							pFtInfoBuf->RSN_IE, pFtInfoBuf->RSNIE_Len);
				NdisZeroMemory(pEntry->LastTK, LEN_TK);
				os_free_mem(NULL, pFtInfoBuf);
			}
		}
		return;
	}
	else
#endif /* DOT11R_FT_SUPPORT */

#ifdef DOT11_SAE_SUPPORT
	if ((auth_info.auth_alg == AUTH_MODE_SAE) &&
		(pMbss->wdev.AuthMode == Ndis802_11AuthModeWPA3PSK ||
			pMbss->wdev.AuthMode == Ndis802_11AuthModeWPA2PSKWPA3PSK)) {
		UCHAR *pmk;
#ifdef DOT11W_PMF_SUPPORT

		if (pEntry) {
			tr_entry = &pAd->MacTab.tr_entry[pEntry->wcid];

			if ((CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_PMF_CAPABLE) == TRUE)
				&& (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)) {
				APPeerAuthSimpleRspGenAndSend(pAd, pRcvHdr,
								auth_info.auth_alg,
								auth_info.auth_seq,
								MLME_ASSOC_REJ_TEMPORARILY);
				PMF_MlmeSAQueryReq(pAd, pEntry);
				return;
			}
		}

#endif /* DOT11W_PMF_SUPPORT */

		sae_handle_auth(pAd, &pAd->SaeCfg, Elem->Msg, Elem->MsgLen,
				pMbss->PSK,
				auth_info.auth_seq, auth_info.auth_status, &pmk);

		if (pmk) {

			if (!pEntry)
				pEntry = MacTableInsertEntry(pAd, auth_info.addr2, wdev, ENTRY_CLIENT, OPMODE_AP, TRUE);

			if (pEntry) {
				UCHAR pmkid[80];

				NdisMoveMemory(pEntry->PMK, pmk, LEN_PMK);
				pEntry->AuthState = AS_AUTH_OPEN;
				pEntry->Sst = SST_AUTH; /* what if it already in SST_ASSOC ??????? */
				if (sae_get_pmk_cache(&pAd->SaeCfg, auth_info.addr1, auth_info.addr2, pmkid, NULL)) {

					RTMPAddPMKIDCache(pAd,
							apidx,
							pEntry->Addr,
							pmkid,
							pmk
#ifdef CONFIG_OWE_SUPPORT
							, LEN_PMK
#endif
							);
					MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_TRACE,
						("WPA3PSK(SAE):(%02x:%02x:%02x:%02x:%02x:%02x)"
						"Calc PMKID=%02x:%02x:%02x:%02x:%02x:%02x\n",
						PRINT_MAC(pEntry->Addr), pmkid[0], pmkid[1],
						pmkid[2], pmkid[3], pmkid[4], pmkid[5]));
				}
			}
		}
	} else
#endif /* DOT11_SAE_SUPPORT */

	if ((auth_info.auth_alg == AUTH_MODE_OPEN) && 
		(pMbss->wdev.AuthMode != Ndis802_11AuthModeShared)) 
	{
		if (!pEntry)
			pEntry = MacTableInsertEntry(pAd, auth_info.addr2, wdev, ENTRY_CLIENT, OPMODE_AP, TRUE);

		if (pEntry)
		{
#ifdef DOT11W_PMF_SUPPORT
			tr_entry = &pAd->MacTab.tr_entry[pEntry->wcid];

                if (!(CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_PMF_CAPABLE)) 
                        || (tr_entry->PortSecured != WPA_802_1X_PORT_SECURED))
#endif /* DOT11W_PMF_SUPPORT */
                        {
			pEntry->AuthState = AS_AUTH_OPEN;
			pEntry->Sst = SST_AUTH; /* what if it already in SST_ASSOC ??????? */
                        }
			APPeerAuthSimpleRspGenAndSend(pAd, pRcvHdr, auth_info.auth_alg, auth_info.auth_seq + 1, MLME_SUCCESS);

		} else {
#ifdef WIFI_DIAG
			DiagConnError(pAd, apidx, auth_info.addr2, DIAG_CONN_STA_LIM, 0);
#endif
#ifdef CONN_FAIL_EVENT
			ApSendConnFailMsg(pAd,
				pAd->ApCfg.MBSSID[apidx].Ssid,
				pAd->ApCfg.MBSSID[apidx].SsidLen,
				auth_info.addr2,
				REASON_DISASSPC_AP_UNABLE);
#endif
		}

	}
	else if ((auth_info.auth_alg == AUTH_MODE_KEY) && 
				((wdev->AuthMode == Ndis802_11AuthModeShared)
				|| (wdev->AuthMode == Ndis802_11AuthModeAutoSwitch)))
	{
		if (!pEntry)
			pEntry = MacTableInsertEntry(pAd, auth_info.addr2, wdev, ENTRY_CLIENT, OPMODE_AP, TRUE);

		if (pEntry)
		{
			pEntry->AuthState = AS_AUTHENTICATING;
			pEntry->Sst = SST_NOT_AUTH; /* what if it already in SST_ASSOC ??????? */

			/* log this STA in AuthRspAux machine, only one STA is stored. If two STAs using */
			/* SHARED_KEY authentication mingled together, then the late comer will win. */
			COPY_MAC_ADDR(&pAd->ApMlmeAux.Addr, auth_info.addr2);
			for(i=0; i<CIPHER_TEXT_LEN; i++) 
				pAd->ApMlmeAux.Challenge[i] = RandomByte(pAd);

			RspReason = 0;
			auth_info.auth_seq++;
  
			NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);
			if (NStatus != NDIS_STATUS_SUCCESS) {
				/*return;*/  /* if no memory, can't do anything */
#ifdef WAPP_SUPPORT
				wapp_auth_fail = MLME_NO_RESOURCE;
#endif /* WAPP_SUPPORT */
				goto auth_failure; /* if no memory, can't do anything */
			}
			DBGPRINT(RT_DEBUG_TRACE, ("AUTH - Send AUTH seq#2 (Challenge)\n"));

			MgtMacHeaderInit(pAd, &AuthHdr, SUBTYPE_AUTH, 0, 	auth_info.addr2, 
								wdev->if_addr,
								wdev->bssid);
			MakeOutgoingFrame(pOutBuffer,            &FrameLen,
								sizeof(HEADER_802_11), &AuthHdr,
								2,                     &auth_info.auth_alg,
								2,                     &auth_info.auth_seq,
								2,                     &RspReason,
								1,                     &ChTxtIe,
								1,                     &ChTxtLen,
								CIPHER_TEXT_LEN,       pAd->ApMlmeAux.Challenge,
								END_OF_ARGS);
			MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
			MlmeFreeMemory(pAd, pOutBuffer);
		} else {
#ifdef WIFI_DIAG
			DiagConnError(pAd, apidx, auth_info.addr2, DIAG_CONN_STA_LIM, 0);
#endif
#ifdef CONN_FAIL_EVENT
			ApSendConnFailMsg(pAd,
				pAd->ApCfg.MBSSID[apidx].Ssid,
				pAd->ApCfg.MBSSID[apidx].SsidLen,
				auth_info.addr2,
				REASON_DISASSPC_AP_UNABLE);
#endif
		}

	}
#ifdef WH_EZ_SETUP  /* Move to ez_cmm.c */
	else if ((auth_info.auth_alg == AUTH_MODE_EZ)
		&& IS_EZ_SETUP_ENABLED(wdev)) {
		if (!pEntry)
			pEntry = MacTableInsertEntry(pAd, auth_info.addr2, wdev, ENTRY_CLIENT, OPMODE_AP, TRUE);

		if (pEntry) {
			if (ez_process_auth_request(pAd, wdev, &auth_info, Elem->Msg, Elem->MsgLen)) {
				pEntry->AuthState = AS_AUTH_OPEN;
				pEntry->Sst = SST_AUTH;
			}
			else 
			{
				ez_update_connection(pAd, wdev);
			MacTableDeleteEntry(pAd, pEntry->wcid, pEntry->Addr);
			}
			
		}
	}
#endif /* WH_EZ_SETUP */	
	else
	{
		/* wrong algorithm */
		APPeerAuthSimpleRspGenAndSend(pAd, pRcvHdr, auth_info.auth_alg, auth_info.auth_seq + 1, MLME_ALG_NOT_SUPPORT);
#ifdef WIFI_DIAG
		DiagConnError(pAd, apidx, auth_info.addr2,
			DIAG_CONN_AUTH_FAIL, REASON_AUTH_WRONG_ALGORITHM);
#endif
#ifdef CONN_FAIL_EVENT
				ApSendConnFailMsg(pAd,
					pAd->ApCfg.MBSSID[apidx].Ssid,
					pAd->ApCfg.MBSSID[apidx].SsidLen,
					auth_info.addr2,
					REASON_AKMP_NOT_VALID);
#endif
		/* If this STA exists, delete it. */
		if (pEntry)
			MacTableDeleteEntry(pAd, pEntry->wcid, pEntry->Addr);

		DBGPRINT(RT_DEBUG_TRACE, ("AUTH - Alg=%d, Seq=%d, AuthMode=%d\n",
				auth_info.auth_alg, auth_info.auth_seq, pAd->ApCfg.MBSSID[apidx].wdev.AuthMode));
	}
	return;
auth_failure:
#ifdef WAPP_SUPPORT
	wapp_send_sta_connect_rejected(pAd, wdev, auth_info.addr2, auth_info.addr1, wapp_cnnct_stage, wapp_auth_fail);
#endif/* WAPP_SUPPORT */
	return;
}


static VOID APPeerAuthConfirmAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	AUTH_FRAME_INFO auth_info;
	PHEADER_802_11  pRcvHdr;
	MAC_TABLE_ENTRY *pEntry;
	UINT32 apidx, u4MaxMBSSIDSize;
#ifdef DOT11R_FT_SUPPORT
	PFT_CFG pFtCfg;
	PFT_INFO pFtInfoBuf;
#endif /* DOT11R_FT_SUPPORT */



	if (pAd == NULL)
	{	
		DBGPRINT(RT_DEBUG_ERROR, ("%s: pAd is NULL\n",__FUNCTION__));	
		return;
	}
	

	if (!APPeerAuthSanity(pAd, Elem->Msg, Elem->MsgLen, &auth_info))
		return;

	u4MaxMBSSIDSize = sizeof(pAd->ApCfg.MBSSID)/sizeof(pAd->ApCfg.MBSSID[0]);
	apidx = get_apidx_by_addr(pAd, auth_info.addr1);
	if ((apidx >= pAd->ApCfg.BssidNum) || (apidx >= u4MaxMBSSIDSize))
	{	
		DBGPRINT(RT_DEBUG_TRACE, ("AUTH - Bssid not found\n"));
		return;
	}

	if ((pAd->ApCfg.MBSSID[apidx].wdev.if_dev != NULL) &&
		!(RTMP_OS_NETDEV_STATE_RUNNING(pAd->ApCfg.MBSSID[apidx].wdev.if_dev)))
	{
    	DBGPRINT(RT_DEBUG_TRACE, ("AUTH - Bssid IF didn't up yet.\n"));
	   	return;
	} /* End of if */

	if (Elem->Wcid >= MAX_LEN_OF_MAC_TABLE)
	{
    	DBGPRINT(RT_DEBUG_ERROR, ("AUTH - Invalid wcid (%d).\n", Elem->Wcid));		
		return; 
	}

	pEntry = &pAd->MacTab.Content[Elem->Wcid];
	if (pEntry && IS_ENTRY_CLIENT(pEntry))
	{
		if (!RTMPEqualMemory(auth_info.addr1, pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.bssid, MAC_ADDR_LEN))
		{
			MacTableDeleteEntry(pAd, pEntry->wcid, pEntry->Addr);
			pEntry = NULL;
			DBGPRINT(RT_DEBUG_WARN, ("AUTH - Bssid does not match\n"));
		}
		else
		{
			if (pEntry->bIAmBadAtheros == TRUE)
			{
				AsicUpdateProtect(pAd, 8, ALLN_SETPROTECT, FALSE, FALSE);
				DBGPRINT(RT_DEBUG_TRACE, ("Atheros Problem. Turn on RTS/CTS!!!\n"));
				pEntry->bIAmBadAtheros = FALSE;
			}

			ASSERT(pEntry->Aid == Elem->Wcid);

#ifdef DOT11_N_SUPPORT
			BASessionTearDownALL(pAd, pEntry->wcid);
#endif /* DOT11_N_SUPPORT */
		}
	}

    pRcvHdr = (PHEADER_802_11)(Elem->Msg);

	DBGPRINT(RT_DEBUG_TRACE,
			("AUTH - MBSS(%d), Rcv AUTH seq#%d, Alg=%d, Status=%d from "
			"[wcid=%d]%02x:%02x:%02x:%02x:%02x:%02x\n",
			apidx, auth_info.auth_seq, auth_info.auth_alg, 
			auth_info.auth_status, Elem->Wcid, 
			PRINT_MAC(auth_info.addr2)));
	if(pEntry) {
		INT32 prev = -1;
		prev = (INT32)pEntry->AuthAssocNotInProgressFlag;
		pEntry->AuthAssocNotInProgressFlag = 0;
		DBGPRINT(RT_DEBUG_TRACE,("===[%s]..2 prev_val = %d AuthAssocNotInProgressFlag = %u \n", 
			__FUNCTION__, prev, pEntry->AuthAssocNotInProgressFlag));
	}
	if (pEntry && MAC_ADDR_EQUAL(auth_info.addr2, pAd->ApMlmeAux.Addr)) 
	{
#ifdef DOT11R_FT_SUPPORT
		pFtCfg = &pAd->ApCfg.MBSSID[apidx].FtCfg;
		if ((pFtCfg->FtCapFlag.Dot11rFtEnable) && (auth_info.auth_alg == AUTH_MODE_FT))
		{
			USHORT result;

			os_alloc_mem(pAd, (UCHAR **)&pFtInfoBuf, sizeof(FT_INFO));
			if (pFtInfoBuf)
			{
				os_alloc_mem(pAd, (UCHAR **)&(pFtInfoBuf->RicInfo.pRicInfo), 512);
				if (pFtInfoBuf->RicInfo.pRicInfo != NULL)
				{
					result = FT_AuthConfirmHandler(pAd, pEntry, &auth_info.FtInfo, pFtInfoBuf);

					FT_EnqueueAuthReply(pAd, pRcvHdr, auth_info.auth_alg, 4, result,
										&pFtInfoBuf->MdIeInfo, &pFtInfoBuf->FtIeInfo,
										&pFtInfoBuf->RicInfo, pFtInfoBuf->RSN_IE, pFtInfoBuf->RSNIE_Len);

					os_free_mem(NULL, pFtInfoBuf->RicInfo.pRicInfo);
				}
				os_free_mem(NULL, pFtInfoBuf);
			}
		}
		else
#endif /* DOT11R_FT_SUPPORT */
		if ((pRcvHdr->FC.Wep == 1) &&
			NdisEqualMemory(auth_info.Chtxt, pAd->ApMlmeAux.Challenge, CIPHER_TEXT_LEN)) 
		{
			/* Successful */
			APPeerAuthSimpleRspGenAndSend(pAd, pRcvHdr, auth_info.auth_alg, auth_info.auth_seq + 1, MLME_SUCCESS);
			pEntry->AuthState = AS_AUTH_KEY;
			pEntry->Sst = SST_AUTH;
		}
		else 
		{
	
			/* send wireless event - Authentication rejected because of challenge failure */
			RTMPSendWirelessEvent(pAd, IW_AUTH_REJECT_CHALLENGE_FAILURE, pEntry->Addr, 0, 0);  

			/* fail - wep bit is not set or challenge text is not equal */
			APPeerAuthSimpleRspGenAndSend(pAd, pRcvHdr, auth_info.auth_alg, 
											auth_info.auth_seq + 1, 
											MLME_REJ_CHALLENGE_FAILURE);
#ifdef WIFI_DIAG
			if (IS_ENTRY_CLIENT(pEntry))
				DiagConnError(pAd, pEntry->func_tb_idx, pEntry->Addr,
					DIAG_CONN_AUTH_FAIL, REASON_CHALLENGE_FAIL);
#endif
#ifdef CONN_FAIL_EVENT
			if (IS_ENTRY_CLIENT(pEntry))
				ApSendConnFailMsg(pAd,
					pAd->ApCfg.MBSSID[pEntry->func_tb_idx].Ssid,
					pAd->ApCfg.MBSSID[pEntry->func_tb_idx].SsidLen,
					pEntry->Addr,
					REASON_MIC_FAILURE);
#endif
			MacTableDeleteEntry(pAd, pEntry->wcid, pEntry->Addr);

			/*Chtxt[127]='\0'; */
			/*pAd->ApMlmeAux.Challenge[127]='\0'; */
			DBGPRINT(RT_DEBUG_TRACE, ("%s\n", 
						((pRcvHdr->FC.Wep == 1) ? "challenge text is not equal" : "wep bit is not set")));
			/*DBGPRINT(RT_DEBUG_TRACE, ("Sent Challenge = %s\n",&pAd->ApMlmeAux.Challenge[100])); */
			/*DBGPRINT(RT_DEBUG_TRACE, ("Rcv Challenge = %s\n",&Chtxt[100])); */
		}
	} 
	else 
	{
		/* fail for unknown reason. most likely is AuthRspAux machine be overwritten by another */
		/* STA also using SHARED_KEY authentication */
		APPeerAuthSimpleRspGenAndSend(pAd, pRcvHdr, auth_info.auth_alg, auth_info.auth_seq + 1, MLME_UNSPECIFY_FAIL);

		/* If this STA exists, delete it. */
		if (pEntry) {
#ifdef WIFI_DIAG
			if (IS_ENTRY_CLIENT(pEntry))
				DiagConnError(pAd, pEntry->func_tb_idx, pEntry->Addr,
					DIAG_CONN_AUTH_FAIL, REASON_UNKNOWN);
#endif
#ifdef CONN_FAIL_EVENT
			if (IS_ENTRY_CLIENT(pEntry))
				ApSendConnFailMsg(pAd,
					pAd->ApCfg.MBSSID[pEntry->func_tb_idx].Ssid,
					pAd->ApCfg.MBSSID[pEntry->func_tb_idx].SsidLen,
					pEntry->Addr,
					REASON_MIC_FAILURE);
#endif
			MacTableDeleteEntry(pAd, pEntry->wcid, pEntry->Addr);
		}
	}
}


/*
    ==========================================================================
    Description:
        Some STA/AP
    Note:
        This action should never trigger AUTH state transition, therefore we
        separate it from AUTH state machine, and make it as a standalone service
    ==========================================================================
 */
VOID APCls2errAction(
	IN RTMP_ADAPTER *pAd,
	IN ULONG Wcid,
	IN HEADER_802_11 *pHeader) 
{
	HEADER_802_11 Hdr;
	UCHAR *pOutBuffer = NULL;
	NDIS_STATUS NStatus;
	ULONG FrameLen = 0;
	USHORT Reason = REASON_CLS2ERR;
	MAC_TABLE_ENTRY *pEntry = NULL;
	UCHAR apidx;

	if (Wcid < MAX_LEN_OF_MAC_TABLE)
		pEntry = &(pAd->MacTab.Content[Wcid]);

	if (pEntry && IS_ENTRY_CLIENT(pEntry)) {
#ifdef WIFI_DIAG
		if (IS_ENTRY_CLIENT(pEntry))
			DiagConnError(pAd, pEntry->func_tb_idx, pEntry->Addr,
				DIAG_CONN_DEAUTH, Reason);
#endif
#ifdef CONN_FAIL_EVENT
		if (IS_ENTRY_CLIENT(pEntry))
			ApSendConnFailMsg(pAd,
				pAd->ApCfg.MBSSID[pEntry->func_tb_idx].Ssid,
				pAd->ApCfg.MBSSID[pEntry->func_tb_idx].SsidLen,
				pEntry->Addr,
				REASON_CLS2ERR);
#endif
		/*ApLogEvent(pAd, pAddr, EVENT_DISASSOCIATED); */
		MacTableDeleteEntry(pAd, pEntry->wcid, pHeader->Addr2);
	}
	else
	{
		apidx = get_apidx_by_addr(pAd, pHeader->Addr1);         
		if (apidx >= pAd->ApCfg.BssidNum)
		{
			DBGPRINT(RT_DEBUG_TRACE,("AUTH - Class 2 error but not my bssid %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(pHeader->Addr1))); 
			return;
		}
	}

	/* send out DEAUTH frame */
	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);
	if (NStatus != NDIS_STATUS_SUCCESS) 
		return;

	DBGPRINT(RT_DEBUG_TRACE,
			("AUTH - Class 2 error, Send DEAUTH frame to "
			"%02x:%02x:%02x:%02x:%02x:%02x\n",
			PRINT_MAC(pHeader->Addr2)));

	MgtMacHeaderInit(pAd, &Hdr, SUBTYPE_DEAUTH, 0, pHeader->Addr2, 
						pHeader->Addr1,
						pHeader->Addr1);
	MakeOutgoingFrame(pOutBuffer, &FrameLen, 
					  sizeof(HEADER_802_11), &Hdr, 
					  2, &Reason, 
					  END_OF_ARGS);
	MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
	MlmeFreeMemory(pAd, pOutBuffer);
}


/*
    ==========================================================================
    Description:
        authenticate state machine init, including state transition and timer init
    Parameters:
        Sm - pointer to the auth state machine
    Note:
        The state machine looks like this
        
                                    AP_AUTH_REQ_IDLE           
        APMT2_MLME_DEAUTH_REQ     mlme_deauth_req_action  
    ==========================================================================
 */
void APAuthStateMachineInit(
    IN PRTMP_ADAPTER pAd, 
    IN STATE_MACHINE *Sm, 
    OUT STATE_MACHINE_FUNC Trans[]) 
{
	StateMachineInit(Sm, (STATE_MACHINE_FUNC *)Trans, AP_MAX_AUTH_STATE,
					AP_MAX_AUTH_MSG, (STATE_MACHINE_FUNC)Drop,
					AP_AUTH_REQ_IDLE, AP_AUTH_MACHINE_BASE);
	 
	/* the first column */
	StateMachineSetAction(Sm, AP_AUTH_REQ_IDLE, APMT2_MLME_DEAUTH_REQ,
						(STATE_MACHINE_FUNC)APMlmeDeauthReqAction);
	StateMachineSetAction(Sm, AP_AUTH_REQ_IDLE, APMT2_PEER_DEAUTH,
						(STATE_MACHINE_FUNC)APPeerDeauthReqAction);
	StateMachineSetAction(Sm, AP_AUTH_REQ_IDLE, APMT2_PEER_AUTH_REQ,
						(STATE_MACHINE_FUNC)APPeerAuthReqAtIdleAction);
	StateMachineSetAction(Sm, AP_AUTH_REQ_IDLE, APMT2_PEER_AUTH_CONFIRM,
						(STATE_MACHINE_FUNC)APPeerAuthConfirmAction);
}


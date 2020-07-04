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
 ***************************************************************************


	Module Name:
	apcli_assoc.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Fonchi		2006-6-23		modified for rt61-APClinent
*/

#ifdef APCLI_SUPPORT

#include "rt_config.h"

static VOID ApCliAssocTimeout(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3);

#ifdef MAC_REPEATER_SUPPORT
static VOID ApCliAssocTimeoutExt(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3);
#endif /* MAC_REPEATER_SUPPORT */

static VOID ApCliMlmeAssocReqAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);

static VOID ApCliMlmeDisassocReqAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);

static VOID ApCliPeerAssocRspAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);

static VOID ApCliPeerDisassocAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);

static VOID ApCliAssocTimeoutAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);

static VOID ApCliInvalidStateWhenAssoc(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);

static VOID ApCliInvalidStateWhenDisassociate(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);

static VOID ApCliAssocPostProc(
	IN PRTMP_ADAPTER pAd, 
	IN PUCHAR pAddr2, 
	IN USHORT CapabilityInfo, 
	IN USHORT IfIndex, 
	IN UCHAR SupRate[], 
	IN UCHAR SupRateLen,
	IN UCHAR ExtRate[],
	IN UCHAR ExtRateLen,
	IN PEDCA_PARM pEdcaParm,
	IN HT_CAPABILITY_IE *pHtCapability,
	IN UCHAR HtCapabilityLen, 
	IN ADD_HT_INFO_IE *pAddHtInfo);

DECLARE_TIMER_FUNCTION(ApCliAssocTimeout);
BUILD_TIMER_FUNCTION(ApCliAssocTimeout);
#ifdef MAC_REPEATER_SUPPORT
DECLARE_TIMER_FUNCTION(ApCliAssocTimeoutExt);
BUILD_TIMER_FUNCTION(ApCliAssocTimeoutExt);
#endif /* MAC_REPEATER_SUPPORT */

/*  
    ==========================================================================
    Description: 
        association state machine init, including state transition and timer init
    Parameters: 
        S - pointer to the association state machine
    Note:
        The state machine looks like the following 
    ==========================================================================
 */
VOID ApCliAssocStateMachineInit(
	IN	PRTMP_ADAPTER	pAd, 
	IN  STATE_MACHINE *S, 
	OUT STATE_MACHINE_FUNC Trans[]) 
{
	UCHAR i;
#ifdef MAC_REPEATER_SUPPORT
	UCHAR j;
#endif /* MAC_REPEATER_SUPPORT */

	StateMachineInit(S, (STATE_MACHINE_FUNC*)Trans,
		APCLI_MAX_ASSOC_STATE, APCLI_MAX_ASSOC_MSG,
		(STATE_MACHINE_FUNC)Drop, APCLI_ASSOC_IDLE,
		APCLI_ASSOC_MACHINE_BASE);

	/* first column */
	StateMachineSetAction(S, APCLI_ASSOC_IDLE, APCLI_MT2_MLME_ASSOC_REQ, (STATE_MACHINE_FUNC)ApCliMlmeAssocReqAction);
	StateMachineSetAction(S, APCLI_ASSOC_IDLE, APCLI_MT2_MLME_DISASSOC_REQ, (STATE_MACHINE_FUNC)ApCliMlmeDisassocReqAction);
	StateMachineSetAction(S, APCLI_ASSOC_IDLE, APCLI_MT2_PEER_DISASSOC_REQ, (STATE_MACHINE_FUNC)ApCliPeerDisassocAction);
   
	/* second column */
	StateMachineSetAction(S, APCLI_ASSOC_WAIT_RSP, APCLI_MT2_MLME_ASSOC_REQ, (STATE_MACHINE_FUNC)ApCliInvalidStateWhenAssoc);
	StateMachineSetAction(S, APCLI_ASSOC_WAIT_RSP, APCLI_MT2_MLME_DISASSOC_REQ, (STATE_MACHINE_FUNC)ApCliInvalidStateWhenDisassociate);
	StateMachineSetAction(S, APCLI_ASSOC_WAIT_RSP, APCLI_MT2_PEER_DISASSOC_REQ, (STATE_MACHINE_FUNC)ApCliPeerDisassocAction);
	StateMachineSetAction(S, APCLI_ASSOC_WAIT_RSP, APCLI_MT2_PEER_ASSOC_RSP, (STATE_MACHINE_FUNC)ApCliPeerAssocRspAction);
	StateMachineSetAction(S, APCLI_ASSOC_WAIT_RSP, APCLI_MT2_ASSOC_TIMEOUT, (STATE_MACHINE_FUNC)ApCliAssocTimeoutAction);

	for (i=0; i < MAX_APCLI_NUM; i++)
	{
		pAd->ApCfg.ApCliTab[i].AssocCurrState = APCLI_ASSOC_IDLE;

		/* timer init */
		RTMPInitTimer(pAd, &pAd->ApCfg.ApCliTab[i].MlmeAux.ApCliAssocTimer,
						GET_TIMER_FUNCTION(ApCliAssocTimeout), pAd, FALSE);

#ifdef MAC_REPEATER_SUPPORT
		for (j = 0; j < MAX_EXT_MAC_ADDR_SIZE; j++)
		{
			pAd->ApCfg.ApCliTab[i].RepeaterCli[j].pAd = pAd;
			pAd->ApCfg.ApCliTab[i].RepeaterCli[j].MatchApCliIdx = i;
			pAd->ApCfg.ApCliTab[i].RepeaterCli[j].MatchLinkIdx = j;
			pAd->ApCfg.ApCliTab[i].RepeaterCli[j].AssocCurrState = APCLI_ASSOC_IDLE;

			/* timer init */
			RTMPInitTimer(pAd, &pAd->ApCfg.ApCliTab[i].RepeaterCli[j].ApCliAssocTimer,
								GET_TIMER_FUNCTION(ApCliAssocTimeoutExt), &pAd->ApCfg.ApCliTab[i].RepeaterCli[j], FALSE);
		}
#endif /* MAC_REPEATER_SUPPORT */
	}

	return;
}

/*
    ==========================================================================
    Description:
        Association timeout procedure. After association timeout, this function 
        will be called and it will put a message into the MLME queue
    Parameters:
        Standard timer parameters
    ==========================================================================
 */
static VOID ApCliAssocTimeout(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)FunctionContext;

	DBGPRINT(RT_DEBUG_TRACE, ("APCLI_ASSOC - enqueue APCLI_MT2_ASSOC_TIMEOUT \n"));

	MlmeEnqueue(pAd, APCLI_ASSOC_STATE_MACHINE, APCLI_MT2_ASSOC_TIMEOUT, 0, NULL, 0);
	RTMP_MLME_HANDLER(pAd);

	return;
}

#ifdef MAC_REPEATER_SUPPORT
/*
    ==========================================================================
    Description:
        Association timeout procedure. After association timeout, this function 
        will be called and it will put a message into the MLME queue
    Parameters:
        Standard timer parameters
    ==========================================================================
 */
static VOID ApCliAssocTimeoutExt(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3)
{
	PREPEATER_CLIENT_ENTRY pRepeaterCliEntry = (PREPEATER_CLIENT_ENTRY)FunctionContext;
	PRTMP_ADAPTER pAd;
	USHORT ifIndex = 0;

	DBGPRINT(RT_DEBUG_TRACE, ("Repeater Cli ASSOC - enqueue APCLI_MT2_ASSOC_TIMEOUT\n"));

	pAd = pRepeaterCliEntry->pAd;
	ifIndex = (64 + (16*pRepeaterCliEntry->MatchApCliIdx) + pRepeaterCliEntry->MatchLinkIdx);

	DBGPRINT(RT_DEBUG_ERROR, (" (%s) ifIndex = %d, CliIdx = %d !!!\n",
					__FUNCTION__, pRepeaterCliEntry->MatchApCliIdx, pRepeaterCliEntry->MatchLinkIdx));

	MlmeEnqueue(pAd, APCLI_ASSOC_STATE_MACHINE, APCLI_MT2_ASSOC_TIMEOUT, 0, NULL, ifIndex);
	RTMP_MLME_HANDLER(pAd);

	return;
}
#endif /* MAC_REPEATER_SUPPORT */

/*
    ==========================================================================
    Description:
        mlme assoc req handling procedure
    Parameters:
        Adapter - Adapter pointer
        Elem - MLME Queue Element
    Pre:
        the station has been authenticated and the following information is stored in the config
            -# SSID
            -# supported rates and their length
    Post  :
        -# An association request frame is generated and sent to the air
        -# Association timer starts
        -# Association state -> ASSOC_WAIT_RSP
        
    ==========================================================================
 */
static VOID ApCliMlmeAssocReqAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem) 
{
	NDIS_STATUS		 NStatus;
	BOOLEAN          Cancelled;
	UCHAR            ApAddr[6];
	HEADER_802_11    AssocHdr;
	UCHAR            WmeIe[9] = {IE_VENDOR_SPECIFIC, 0x07, 0x00, 0x50, 0xf2, 0x02, 0x00, 0x01, 0x00};
	USHORT           ListenIntv;
	ULONG            Timeout;
	USHORT           CapabilityInfo;
	PUCHAR           pOutBuffer = NULL;
	ULONG            FrameLen = 0;
	ULONG            tmp;
	UCHAR            SsidIe    = IE_SSID;
	UCHAR            SupRateIe = IE_SUPP_RATES;
	UCHAR            ExtRateIe = IE_EXT_SUPP_RATES;
	APCLI_CTRL_MSG_STRUCT ApCliCtrlMsg;
	USHORT ifIndex = (USHORT)(Elem->Priv);
	PULONG pCurrState = NULL;
	UCHAR RSNIe = IE_WPA;
	APCLI_STRUCT *apcli_entry;
	struct wifi_dev *wdev;
#ifdef MAC_REPEATER_SUPPORT
	UCHAR CliIdx = 0xFF;
#if defined(APCLI_SAE_SUPPORT) || defined(APCLI_OWE_SUPPORT)
	REPEATER_CLIENT_ENTRY *pReptEntry = NULL;
#endif
#endif /* MAC_REPEATER_SUPPORT */
#if defined(APCLI_SAE_SUPPORT) || defined(APCLI_OWE_SUPPORT)
	MAC_TABLE_ENTRY *pentry = (MAC_TABLE_ENTRY *)NULL;
#endif
#ifdef APCLI_OWE_SUPPORT
	OWE_INFO *owe = NULL;
	UCHAR *curr_group = NULL;
#endif


#ifdef WH_EZ_SETUP
		if(IS_ADPTR_EZ_SETUP_ENABLED(pAd))
			EZ_DEBUG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ApCliMlmeAssocReqAction()\n"));
#endif
	if ((ifIndex >= MAX_APCLI_NUM)
#ifdef MAC_REPEATER_SUPPORT
		&& (ifIndex < 64)
#endif /* MAC_REPEATER_SUPPORT */
		)
		return;

#ifdef MAC_REPEATER_SUPPORT
	if (ifIndex >= 64)
	{
		CliIdx = ((ifIndex - 64) % 16);
		ifIndex = ((ifIndex - 64) / 16);
		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].AssocCurrState;
	}
	else
#endif /* MAC_REPEATER_SUPPORT */
		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].AssocCurrState;

	apcli_entry = &pAd->ApCfg.ApCliTab[ifIndex];
	wdev = &apcli_entry->wdev;

#if defined(APCLI_SAE_SUPPORT) || defined(APCLI_OWE_SUPPORT)
	pentry = &apcli_entry->pre_mac_entry;
#endif
	/* Block all authentication request durning WPA block period */
	if (apcli_entry->bBlockAssoc == TRUE)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("APCLI_ASSOC - Block Auth request durning WPA block period!\n"));
		*pCurrState = APCLI_ASSOC_IDLE;
		ApCliCtrlMsg.Status = MLME_STATE_MACHINE_REJECT;
		MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_ASSOC_RSP,
			sizeof(APCLI_CTRL_MSG_STRUCT), &ApCliCtrlMsg, ifIndex);
	}
	else if(MlmeAssocReqSanity(pAd, Elem->Msg, Elem->MsgLen, ApAddr, &CapabilityInfo, &Timeout, &ListenIntv))
	{
		//RTMPCancelTimer(&apcli_entry->MlmeAux.ApCliAssocTimer, &Cancelled);
#ifdef MAC_REPEATER_SUPPORT
		if (CliIdx != 0xFF)
			RTMPCancelTimer(&apcli_entry->RepeaterCli[CliIdx].ApCliAssocTimer, &Cancelled);
		else
#endif /* MAC_REPEATER_SUPPORT */
		RTMPCancelTimer(&apcli_entry->MlmeAux.ApCliAssocTimer, &Cancelled);

		/* allocate and send out AssocRsp frame */
		NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  /*Get an unused nonpaged memory */
		if (NStatus != NDIS_STATUS_SUCCESS)
		{
			DBGPRINT(RT_DEBUG_TRACE, ("APCLI_ASSOC - ApCliMlmeAssocReqAction() allocate memory failed \n"));
			*pCurrState = APCLI_ASSOC_IDLE;

			ApCliCtrlMsg.Status = MLME_FAIL_NO_RESOURCE;
			MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_ASSOC_RSP,
				sizeof(APCLI_CTRL_MSG_STRUCT), &ApCliCtrlMsg, ifIndex);

			return;
		}


		DBGPRINT(RT_DEBUG_TRACE, ("APCLI_ASSOC - Send ASSOC request...\n"));
		ApCliMgtMacHeaderInit(pAd, &AssocHdr, SUBTYPE_ASSOC_REQ, 0, ApAddr, ApAddr, ifIndex);

#ifdef MAC_REPEATER_SUPPORT
		if (CliIdx != 0xFF)
			COPY_MAC_ADDR(AssocHdr.Addr2, apcli_entry->RepeaterCli[CliIdx].CurrentAddress);
#endif /* MAC_REPEATER_SUPPORT */

		/* Build basic frame first */
		MakeOutgoingFrame(pOutBuffer,               &FrameLen,
			sizeof(HEADER_802_11),    &AssocHdr,
			2,                        &CapabilityInfo,
			2,                        &ListenIntv,
			1,                        &SsidIe,
			1,                        &apcli_entry->MlmeAux.SsidLen, 
			apcli_entry->MlmeAux.SsidLen,     apcli_entry->MlmeAux.Ssid,
			1,                        &SupRateIe,
			1,                        &apcli_entry->MlmeAux.SupRateLen,
			apcli_entry->MlmeAux.SupRateLen,  apcli_entry->MlmeAux.SupRate,
			END_OF_ARGS);

		if(apcli_entry->MlmeAux.ExtRateLen != 0)
		{
			MakeOutgoingFrame(pOutBuffer + FrameLen,    &tmp,
				1,                        &ExtRateIe,
				1,                        &apcli_entry->MlmeAux.ExtRateLen,
				apcli_entry->MlmeAux.ExtRateLen,  apcli_entry->MlmeAux.ExtRate,
				END_OF_ARGS);
			FrameLen += tmp;
		}

#ifdef DOT11_N_SUPPORT
		/*	
			WFA recommend to restrict the encryption type in 11n-HT mode.
			So, the WEP and TKIP are not allowed in HT rate.
		*/
		if (pAd->CommonCfg.HT_DisallowTKIP &&
			IS_INVALID_HT_SECURITY(wdev->WepStatus))
		{
			/* Force to None-HT mode due to WiFi 11n policy */
			apcli_entry->MlmeAux.HtCapabilityLen = 0;
#ifdef DOT11_VHT_AC
			apcli_entry->MlmeAux.vht_cap_len = 0;
#endif /* DOT11_VHT_AC */
			DBGPRINT(RT_DEBUG_TRACE, ("%s : Force AP-client as Non-HT mode\n", __FUNCTION__));
		}

		/* HT */
		if ((apcli_entry->MlmeAux.HtCapabilityLen > 0) && 
			WMODE_CAP_N(pAd->CommonCfg.PhyMode))
		{
			ULONG TmpLen;
			HT_CAPABILITY_IE HtCapabilityTmp;

			NdisZeroMemory(&HtCapabilityTmp, sizeof(HT_CAPABILITY_IE));
			NdisMoveMemory(&HtCapabilityTmp, &apcli_entry->MlmeAux.HtCapability, apcli_entry->MlmeAux.HtCapabilityLen);
#ifdef DOT11N_SS3_SUPPORT
			HtCapabilityTmp.MCSSet[2] = (apcli_entry->MlmeAux.HtCapability.MCSSet[2] & apcli_entry->RxMcsSet[2]);
#endif /* DOT11N_SS3_SUPPORT */

#ifdef RT_BIG_ENDIAN
        		*(USHORT *)(&HtCapabilityTmp.HtCapInfo) = SWAP16(*(USHORT *)(&HtCapabilityTmp.HtCapInfo));
        		*(USHORT *)(&HtCapabilityTmp.ExtHtCapInfo) = SWAP16(*(USHORT *)(&HtCapabilityTmp.ExtHtCapInfo));
#endif /* RT_BIG_ENDINA */
        		MakeOutgoingFrame(pOutBuffer + FrameLen,         &TmpLen,
        							1,                           &HtCapIe,
        							1,                           &apcli_entry->MlmeAux.HtCapabilityLen,
        							apcli_entry->MlmeAux.HtCapabilityLen, &HtCapabilityTmp, 
        							END_OF_ARGS);
			FrameLen += TmpLen;

#ifdef DOT11_VHT_AC
			if (WMODE_CAP_AC(pAd->CommonCfg.PhyMode) &&
				(pAd->CommonCfg.Channel > 14) &&
				(apcli_entry->MlmeAux.vht_cap_len))
			{
				FrameLen += build_vht_ies(pAd, (UCHAR *)(pOutBuffer + FrameLen), SUBTYPE_ASSOC_REQ);
			}
#endif /* DOT11_VHT_AC */
		}
#endif /* DOT11_N_SUPPORT */

#ifdef WH_EZ_SETUP
		/*
			To prevent old device has trouble to parse MTK vendor IE,
			insert easy setup IE first.
		*/
		if (IS_EZ_SETUP_ENABLED(wdev)
#ifdef MAC_REPEATER_SUPPORT
			&& (CliIdx == 0xFF)
#endif /* MAC_REPEATER_SUPPORT */
			&& apcli_entry->MlmeAux.support_easy_setup) {
			FrameLen += ez_build_assoc_request_ie(pAd,wdev, ApAddr, pOutBuffer+FrameLen, FrameLen);
		}
		//Arvind : Required to aad some info in beacon
        //FrameLen += build_vendor_ie(pAd, wdev, pOutBuffer+FrameLen, SUBTYPE_ASSOC_REQ);		
#endif /* WH_EZ_SETUP */
#if defined(MWDS) || defined(WH_EZ_SETUP) || defined(STA_FORCE_ROAM_SUPPORT)
		FrameLen += build_vendor_ie(pAd, wdev, pOutBuffer+FrameLen
#ifdef CONFIG_OWE_SUPPORT
		, VIE_ASSOC_REQ
#endif
#ifdef WH_EZ_SETUP
		, SUBTYPE_ASSOC_REQ
#endif
		);
#else

#ifdef AGGREGATION_SUPPORT
		/*
			add Ralink proprietary IE to inform AP this STA is going to use AGGREGATION or PIGGY-BACK+AGGREGATION
			Case I: (Aggregation + Piggy-Back)
				1. user enable aggregation, AND
				2. Mac support piggy-back
				3. AP annouces it's PIGGY-BACK+AGGREGATION-capable in BEACON
			Case II: (Aggregation)
				1. user enable aggregation, AND
				2. AP annouces it's AGGREGATION-capable in BEACON
		*/
		if (pAd->CommonCfg.bAggregationCapable)
		{
#ifdef PIGGYBACK_SUPPORT
			if ((pAd->CommonCfg.bPiggyBackCapable) && ((apcli_entry->MlmeAux.APRalinkIe & 0x00000003) == 3))
			{
				ULONG TmpLen;
				UCHAR RalinkIe[9] = {IE_VENDOR_SPECIFIC, 7, 0x00, 0x0c, 0x43, 0x03, 0x00, 0x00, 0x00}; 
				MakeOutgoingFrame(pOutBuffer+FrameLen,           &TmpLen,
								  9,                             RalinkIe,
								  END_OF_ARGS);
				FrameLen += TmpLen;
			} else
#endif /* PIGGYBACK_SUPPORT */
			if (apcli_entry->MlmeAux.APRalinkIe & 0x00000001)
			{
				ULONG TmpLen;
				UCHAR RalinkIe[9] = {IE_VENDOR_SPECIFIC, 7, 0x00, 0x0c, 0x43, 0x01, 0x00, 0x00, 0x00}; 
				MakeOutgoingFrame(pOutBuffer+FrameLen,           &TmpLen,
								  9,                             RalinkIe,
								  END_OF_ARGS);
				FrameLen += TmpLen;
			}
		}
		else
		{
			ULONG TmpLen;
			UCHAR RalinkIe[9] = {IE_VENDOR_SPECIFIC, 7, 0x00, 0x0c, 0x43, 0x06, 0x00, 0x00, 0x00}; 
			MakeOutgoingFrame(pOutBuffer+FrameLen,		 &TmpLen,
							  9,						 RalinkIe,
							  END_OF_ARGS);
			FrameLen += TmpLen;
		}
#endif  /* AGGREGATION_SUPPORT */
#endif
		if (apcli_entry->MlmeAux.APEdcaParm.bValid)
		{
			if (apcli_entry->wdev.UapsdInfo.bAPSDCapable &&
				apcli_entry->MlmeAux.APEdcaParm.bAPSDCapable)
			{
				QBSS_STA_INFO_PARM QosInfo;

				NdisZeroMemory(&QosInfo, sizeof(QBSS_STA_INFO_PARM));
				QosInfo.UAPSD_AC_BE = pAd->CommonCfg.bAPSDAC_BE;
				QosInfo.UAPSD_AC_BK = pAd->CommonCfg.bAPSDAC_BK;
				QosInfo.UAPSD_AC_VI = pAd->CommonCfg.bAPSDAC_VI;
				QosInfo.UAPSD_AC_VO = pAd->CommonCfg.bAPSDAC_VO;
				QosInfo.MaxSPLength = pAd->CommonCfg.MaxSPLength;
				WmeIe[8] |= *(PUCHAR)&QosInfo;
			}
			else
			{
                /* The Parameter Set Count is set to ¡§0¡¨ in the association request frames */
                /* WmeIe[8] |= (pAd->MlmeAux.APEdcaParm.EdcaUpdateCount & 0x0f); */
			}

			MakeOutgoingFrame(pOutBuffer + FrameLen,    &tmp,
							  9,                        &WmeIe[0],
							  END_OF_ARGS);
			FrameLen += tmp;
		}

		/* Append RSN_IE when WPAPSK OR WPA2PSK, */
		if (((wdev->AuthMode == Ndis802_11AuthModeWPAPSK) || 
			(wdev->AuthMode == Ndis802_11AuthModeWPA2PSK)
#ifdef APCLI_SAE_SUPPORT
			|| (pentry->AuthMode == Ndis802_11AuthModeWPA3PSK)
			|| (pentry->AuthMode == Ndis802_11AuthModeWPA2PSK)
#endif
#ifdef APCLI_OWE_SUPPORT
			|| (pentry->AuthMode == Ndis802_11AuthModeOWE)
#endif
			)
#ifdef WSC_AP_SUPPORT
			&& ((apcli_entry->WscControl.WscConfMode == WSC_DISABLE)
			|| ((apcli_entry->WscControl.WscConfMode != WSC_DISABLE) 
			&& !(apcli_entry->WscControl.bWscTrigger)))
#endif /* WSC_AP_SUPPORT */
            )
		{
			RSNIe = IE_WPA;
			
			if ((wdev->AuthMode == Ndis802_11AuthModeWPA2PSK)
#ifdef APCLI_SAE_SUPPORT
				|| (pentry->AuthMode == Ndis802_11AuthModeWPA3PSK)
				|| (pentry->AuthMode == Ndis802_11AuthModeWPA2PSK)
#endif
#ifdef APCLI_OWE_SUPPORT
				|| (pentry->AuthMode == Ndis802_11AuthModeOWE)
#endif
				)
				RSNIe = IE_WPA2;


#if defined(APCLI_SAE_SUPPORT) || defined(APCLI_OWE_SUPPORT)

			if ((pentry->AuthMode == Ndis802_11AuthModeWPA3PSK)
				|| (pentry->AuthMode == Ndis802_11AuthModeOWE)
				|| (pentry->AuthMode == Ndis802_11AuthModeWPA2PSK
				&& wdev->AuthMode == Ndis802_11AuthModeWPA3PSK)
				) {
				INT idx;
				PBSSID_INFO psaved_pmk = NULL;
				VOID *psaved_pmk_lock = NULL;

#ifdef MAC_REPEATER_SUPPORT
				pReptEntry = &pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx];

				if ((pAd->ApCfg.bMACRepeaterEn) &&
					(CliIdx != 0xFF)) {

					psaved_pmk = &pReptEntry->SavedPMK[0];
					psaved_pmk_lock = (void *)&pReptEntry->SavedPMK_lock;
				} else
#endif /* MAC_REPEATER_SUPPORT */
				{

					psaved_pmk = &apcli_entry->SavedPMK[0];
					psaved_pmk_lock = (VOID *)&apcli_entry->SavedPMK_lock;

				}
				pentry = &apcli_entry->pre_mac_entry; /* similar to MlmeAux*/

				idx = apcli_search_pmkid_cache(pAd, ApAddr, ifIndex
#ifdef MAC_REPEATER_SUPPORT
					, CliIdx
#endif
					);

				if (idx != INVALID_PMKID_IDX) {

					if (psaved_pmk_lock)
						NdisAcquireSpinLock(psaved_pmk_lock);

				/*Update the pentry->pmkcache from the Saved PMK cache */
					pentry->pmkid = psaved_pmk[idx].PMKID;
					pentry->pmk_cache = psaved_pmk[idx].PMK;


					if (psaved_pmk_lock)
						NdisReleaseSpinLock(psaved_pmk_lock);
				}
				WPAMakeEntryRSNIE(pAd, pentry->AuthMode, pentry->WepStatus, pentry);
			}
#endif /* defined(APCLI_SAE_SUPPORT) || defined(APCLI_OWE_SUPPORT) */

			MakeOutgoingFrame(pOutBuffer + FrameLen,    				&tmp,
			              	1,                                      	&RSNIe,
	                        1,                                      	&apcli_entry->RSNIE_Len,
	                        apcli_entry->RSNIE_Len,		apcli_entry->RSN_IE,
	                        END_OF_ARGS);
			
			FrameLen += tmp;
		}	

#ifdef WSC_AP_SUPPORT
		/* Add WSC IE if we are connecting to WSC AP */
		if ((pAd->ApCfg.ApCliTab[ifIndex].WscControl.WscConfMode != WSC_DISABLE) &&
			(pAd->ApCfg.ApCliTab[ifIndex].WscControl.bWscTrigger)) 
			{
			UCHAR *pWscBuf = NULL, WscIeLen = 0;
			ULONG WscTmpLen = 0;

			os_alloc_mem(pAd, (UCHAR **) &pWscBuf, 512);
/*			if( (pWscBuf = kmalloc(512, GFP_ATOMIC)) != NULL) */
			if (pWscBuf != NULL) {
				NdisZeroMemory(pWscBuf, 512);
				WscBuildAssocReqIE(&pAd->ApCfg.ApCliTab[ifIndex].WscControl, pWscBuf, &WscIeLen);

				MakeOutgoingFrame(pOutBuffer + FrameLen,
						  &WscTmpLen, WscIeLen, pWscBuf,
						  END_OF_ARGS);

				FrameLen += WscTmpLen;
/*				kfree(pWscBuf); */
				os_free_mem(NULL, pWscBuf);
			} else
				DBGPRINT(RT_DEBUG_WARN,
					 ("%s:: WscBuf Allocate failed!\n",
					  __FUNCTION__));
		}
#endif /* WSC_AP_SUPPORT */

#ifdef APCLI_OWE_SUPPORT

#ifdef MAC_REPEATER_SUPPORT

			if ((pAd->ApCfg.bMACRepeaterEn) &&
					(CliIdx != 0xFF))
				curr_group = &pReptEntry->curr_owe_group;
			else
#endif /* MAC_REPEATER_SUPPORT */
				curr_group = &apcli_entry->curr_owe_group;

			pentry = &apcli_entry->pre_mac_entry;

			owe = &pentry->owe;

			if (pentry->AuthMode == Ndis802_11AuthModeOWE) {

				/* OWE use initialize group and add owe dh key ie to assoc request */
				if (init_owe_group(owe, *curr_group) == 0) {
					MTWF_LOG(DBG_CAT_SEC, CATSEC_OWE, DBG_LVL_ERROR,
						("==> %s(), init_owe_group failed. shall not happen!\n", __func__));
					*pCurrState = APCLI_ASSOC_IDLE;
					ApCliCtrlMsg.Status = MLME_UNSPECIFY_FAIL;
					MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_ASSOC_RSP,
						sizeof(APCLI_CTRL_MSG_STRUCT), &ApCliCtrlMsg, ifIndex);

					return;
				}

				if (*curr_group == ECDH_GROUP_384)
					pentry->key_deri_alg = SEC_KEY_DERI_SHA384;
				else
					pentry->key_deri_alg = SEC_KEY_DERI_SHA256;

				FrameLen += build_owe_dh_ie(pAd, pentry, (UCHAR *)(pOutBuffer + FrameLen), *curr_group);

			}

#endif /*APCLI_OWE_SUPPORT*/


		MiniportMMRequest(pAd, QID_AC_BE, pOutBuffer, FrameLen);
		MlmeFreeMemory(pAd, pOutBuffer);

#ifdef MAC_REPEATER_SUPPORT
		if (CliIdx != 0xFF)
			RTMPSetTimer(&apcli_entry->RepeaterCli[CliIdx].ApCliAssocTimer, Timeout);
		else
#endif /* MAC_REPEATER_SUPPORT */
			RTMPSetTimer(&apcli_entry->MlmeAux.ApCliAssocTimer, Timeout);
		*pCurrState = APCLI_ASSOC_WAIT_RSP;
	} 
	else
	{
		DBGPRINT(RT_DEBUG_TRACE, ("APCLI_ASSOC - ApCliMlmeAssocReqAction() sanity check failed. BUG!!!!!! \n"));
		*pCurrState = APCLI_ASSOC_IDLE;

		ApCliCtrlMsg.Status = MLME_INVALID_FORMAT;
		MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_ASSOC_RSP,
			sizeof(APCLI_CTRL_MSG_STRUCT), &ApCliCtrlMsg, ifIndex);
	}

	return;
}

/*
    ==========================================================================
    Description:
        Upper layer issues disassoc request
    Parameters:
        Elem -
    ==========================================================================
 */
static VOID ApCliMlmeDisassocReqAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem) 
{
	PMLME_DISASSOC_REQ_STRUCT pDisassocReq;
	HEADER_802_11         DisassocHdr;
	PUCHAR                 pOutBuffer = NULL;
	ULONG                 FrameLen = 0;
	NDIS_STATUS           NStatus;
	APCLI_CTRL_MSG_STRUCT ApCliCtrlMsg;
	USHORT ifIndex = (USHORT)(Elem->Priv);
	PULONG pCurrState = NULL;
#ifdef MAC_REPEATER_SUPPORT
	UCHAR CliIdx = 0xFF;
#endif /* MAC_REPEATER_SUPPORT */

#ifdef WH_EZ_SETUP
		if(IS_ADPTR_EZ_SETUP_ENABLED(pAd))
			EZ_DEBUG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ApCliMlmeDisassocReqAction() \n"));
#endif

	if ((ifIndex >= MAX_APCLI_NUM)
#ifdef MAC_REPEATER_SUPPORT
			&& (ifIndex < 64)
#endif /* MAC_REPEATER_SUPPORT */
	)
		return;

#ifdef MAC_REPEATER_SUPPORT
	if (ifIndex >= 64)
	{
		CliIdx = ((ifIndex - 64) % 16);
		ifIndex = ((ifIndex - 64) / 16);
		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].AssocCurrState;
	}
	else
#endif /* MAC_REPEATER_SUPPORT */
		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].AssocCurrState;

	/* skip sanity check */
	pDisassocReq = (PMLME_DISASSOC_REQ_STRUCT)(Elem->Msg);

	/* allocate and send out DeassocReq frame */
	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  /*Get an unused nonpaged memory */
	if (NStatus != NDIS_STATUS_SUCCESS) 
	{
		DBGPRINT(RT_DEBUG_TRACE, ("APCLI_ASSOC - ApCliMlmeDisassocReqAction() allocate memory failed\n"));
		*pCurrState = APCLI_ASSOC_IDLE;

		ApCliCtrlMsg.Status = MLME_FAIL_NO_RESOURCE;
#ifdef MAC_REPEATER_SUPPORT
		ApCliCtrlMsg.BssIdx = ifIndex;
		ApCliCtrlMsg.CliIdx = CliIdx;
		ifIndex = (USHORT)(Elem->Priv);
#endif /* MAC_REPEATER_SUPPORT */
		MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_DEASSOC_RSP,
			sizeof(APCLI_CTRL_MSG_STRUCT), &ApCliCtrlMsg, ifIndex);
		return;
	}

	DBGPRINT(RT_DEBUG_TRACE, ("APCLI_ASSOC - Send DISASSOC request [BSSID::%02x:%02x:%02x:%02x:%02x:%02x] \n", 
				pDisassocReq->Addr[0], pDisassocReq->Addr[1], pDisassocReq->Addr[2],
				pDisassocReq->Addr[3], pDisassocReq->Addr[4], pDisassocReq->Addr[5]));
	ApCliMgtMacHeaderInit(pAd, &DisassocHdr, SUBTYPE_DISASSOC, 0, pDisassocReq->Addr, pDisassocReq->Addr, ifIndex);

#ifdef MAC_REPEATER_SUPPORT
	if (CliIdx != 0xFF)
		COPY_MAC_ADDR(DisassocHdr.Addr2, pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].CurrentAddress);
#endif /* MAC_REPEATER_SUPPORT */

	MakeOutgoingFrame(pOutBuffer,				&FrameLen, 
						sizeof(HEADER_802_11),	&DisassocHdr, 
						2,						&pDisassocReq->Reason, 
						END_OF_ARGS);
	MiniportMMRequest(pAd, QID_AC_BE, pOutBuffer, FrameLen);
	MlmeFreeMemory(pAd, pOutBuffer);

    *pCurrState = APCLI_ASSOC_IDLE;

	ApCliCtrlMsg.Status = MLME_SUCCESS;

#ifdef MAC_REPEATER_SUPPORT
	ApCliCtrlMsg.BssIdx = ifIndex;
	ApCliCtrlMsg.CliIdx = CliIdx;
	ifIndex = (USHORT)(Elem->Priv);
#endif /* MAC_REPEATER_SUPPORT */

	MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_DEASSOC_RSP,
		sizeof(APCLI_CTRL_MSG_STRUCT), &ApCliCtrlMsg, ifIndex);



	return;
}


/*
    ==========================================================================
    Description:
        peer sends assoc rsp back
    Parameters:
        Elme - MLME message containing the received frame
    ==========================================================================
 */
static VOID ApCliPeerAssocRspAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem) 
{
	BOOLEAN				Cancelled;
	USHORT				CapabilityInfo, Status, Aid;
	UCHAR				SupRate[MAX_LEN_OF_SUPPORTED_RATES], SupRateLen;
	UCHAR				ExtRate[MAX_LEN_OF_SUPPORTED_RATES], ExtRateLen;
	UCHAR				Addr2[MAC_ADDR_LEN];
	EDCA_PARM			EdcaParm;
	UCHAR				CkipFlag;
	APCLI_CTRL_MSG_STRUCT	ApCliCtrlMsg;
	HT_CAPABILITY_IE	HtCapability;
	ADD_HT_INFO_IE		AddHtInfo;	/* AP might use this additional ht info IE */
	UCHAR				HtCapabilityLen;
	UCHAR				AddHtInfoLen;
	UCHAR				NewExtChannelOffset = 0xff;
	USHORT ifIndex = (USHORT)(Elem->Priv);
	ULONG *pCurrState = NULL;
#if defined(DOT11_VHT_AC) || defined(WH_EZ_SETUP) || defined(MAP_SUPPORT)
	PAPCLI_STRUCT pApCliEntry = NULL;
#endif /* DOT11_VHT_AC */
#ifdef APCLI_OWE_SUPPORT
	PMAC_TABLE_ENTRY pEntry = NULL;
#endif
#ifdef MAC_REPEATER_SUPPORT
	UCHAR CliIdx = 0xFF;
#endif /* MAC_REPEATER_SUPPORT */
	IE_LISTS *ie_list = NULL;

#ifdef WH_EZ_SETUP
		if(IS_ADPTR_EZ_SETUP_ENABLED(pAd))
			EZ_DEBUG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s()\n", __FUNCTION__));
#endif

	if ((ifIndex >= MAX_APCLI_NUM)
#ifdef MAC_REPEATER_SUPPORT
			&& (ifIndex < 64)
#endif /* MAC_REPEATER_SUPPORT */
		)
		return;

#ifdef MAC_REPEATER_SUPPORT
	if (ifIndex >= 64)
	{
		CliIdx = ((ifIndex - 64) % 16);
		ifIndex = ((ifIndex - 64) / 16);
		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].AssocCurrState;
	}
	else
#endif /* MAC_REPEATER_SUPPORT */
		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].AssocCurrState;

#if defined(DOT11_VHT_AC) || defined(WH_EZ_SETUP) || defined(MAP_SUPPORT)
	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];
#endif /* DOT11_VHT_AC */
	os_alloc_mem(pAd, (UCHAR **)&ie_list, sizeof(IE_LISTS));
	if (ie_list == NULL) {
		DBGPRINT(RT_DEBUG_OFF, ("%s():mem alloc failed!\n", __FUNCTION__));
		return;
	}
			
	NdisZeroMemory((UCHAR *)ie_list, sizeof(IE_LISTS));
	
	if (ApCliPeerAssocRspSanity(pAd, Elem->Msg, Elem->MsgLen, Addr2, &CapabilityInfo, &Status, &Aid, SupRate, &SupRateLen, ExtRate, &ExtRateLen, 
		&HtCapability, &AddHtInfo, &HtCapabilityLen,&AddHtInfoLen,&NewExtChannelOffset, &EdcaParm, &CkipFlag, ie_list))
	{
		/* The frame is for me ? */
		if(MAC_ADDR_EQUAL(Addr2, pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.Bssid))
		{
			DBGPRINT(RT_DEBUG_TRACE, ("APCLI_ASSOC - receive ASSOC_RSP to me (status=%d)\n", Status));

#ifdef MAC_REPEATER_SUPPORT
			if (CliIdx != 0xFF)
				RTMPCancelTimer(&pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].ApCliAssocTimer, &Cancelled);
			else
#endif /* MAC_REPEATER_SUPPORT */
			RTMPCancelTimer(&pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.ApCliAssocTimer, &Cancelled);

#ifdef WH_EZ_SETUP
			if ((Status == MLME_SUCCESS)
				&& IS_EZ_SETUP_ENABLED(&pApCliEntry->wdev)
#ifdef MAC_REPEATER_SUPPORT
				&& (CliIdx == 0xFF)
#endif /* MAC_REPEATER_SUPPORT */
				&& pApCliEntry->MlmeAux.support_easy_setup) {
				Status = ez_process_assoc_response(&pApCliEntry->wdev, Addr2, Elem->Msg, Elem->MsgLen);
			} else if (Status == MLME_EZ_CONNECTION_LOOP)
			{
			}
#endif /* WH_EZ_SETUP */

			if(Status == MLME_SUCCESS) 
			{
				/* go to procedure listed on page 376 */
#ifdef MAC_REPEATER_SUPPORT
				if (CliIdx == 0xFF)
#endif /* MAC_REPEATER_SUPPORT */
				{
					ApCliAssocPostProc(pAd, Addr2, CapabilityInfo, ifIndex, SupRate, SupRateLen,
						ExtRate, ExtRateLen, &EdcaParm, &HtCapability, HtCapabilityLen, &AddHtInfo);  	
#ifdef DOT11_VHT_AC 
					RTMPZeroMemory(&pApCliEntry->MlmeAux.vht_cap, sizeof(VHT_CAP_IE));
					RTMPZeroMemory(&pApCliEntry->MlmeAux.vht_op, sizeof(VHT_OP_IE));
					pApCliEntry->MlmeAux.vht_cap_len = 0;
					pApCliEntry->MlmeAux.vht_op_len = 0;
					if (WMODE_CAP_AC(pAd->CommonCfg.PhyMode) && ie_list->vht_cap_len && ie_list->vht_op_len)
					{
						DBGPRINT(RT_DEBUG_TRACE, ("There is vht le at Assoc Rsp ifIndex=%d vht_cap_len=%d\n", ifIndex,ie_list->vht_cap_len));
						NdisMoveMemory(&pApCliEntry->MlmeAux.vht_cap, &(ie_list->vht_cap), ie_list->vht_cap_len);
						pApCliEntry->MlmeAux.vht_cap_len = ie_list->vht_cap_len;
						NdisMoveMemory(&pApCliEntry->MlmeAux.vht_op, &(ie_list->vht_op), ie_list->vht_op_len);
						pApCliEntry->MlmeAux.vht_op_len = ie_list->vht_op_len; 
					}	
#endif /* DOT11_VHT_AC */						
				}

#ifdef APCLI_OWE_SUPPORT
				pEntry = &pAd->ApCfg.ApCliTab[ifIndex].pre_mac_entry;

				if (pEntry->AuthMode == Ndis802_11AuthModeOWE) {
					UINT8 *pmkid = NULL;
					UINT8 pmkid_count = 0;
					INT idx = 0;

					pmkid = WPA_ExtractSuiteFromRSNIE(ie_list->RSN_IE,
								ie_list->RSNIE_Len,
								PMKID_LIST,
								&pmkid_count);

					if (pmkid != NULL) {
						idx = apcli_search_pmkid_cache(pAd, Addr2, ifIndex
#ifdef MAC_REPEATER_SUPPORT
							, CliIdx
#endif
							);

						if ((idx == INVALID_PMKID_IDX) ||
							((pEntry->pmkid) &&
							((RTMPEqualMemory(pmkid,
								pEntry->pmkid,
								LEN_PMKID)) == 0))) {
							MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
								("%s: no OWE PMKID, do normal ECDH procedure\n",
								__func__));
							ApCliCtrlMsg.Status = MLME_UNSPECIFY_FAIL;
							MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_ASSOC_RSP,
								sizeof(APCLI_CTRL_MSG_STRUCT),
								&ApCliCtrlMsg, ifIndex);
							RTMP_MLME_HANDLER(pAd);
						}
					} else {
						pEntry->need_process_ecdh_ie = TRUE;
						/*Process ECDH IE as no pmkid in assoc rsp rsnie,*/
						/*PMK id  sent might have been invalid at AP,*/
						/*need to clear seconfig pmk cache*/
						idx = apcli_search_pmkid_cache(pAd, Addr2, ifIndex
#ifdef MAC_REPEATER_SUPPORT
							, CliIdx
#endif
							);
						if ((idx != INVALID_PMKID_IDX) &&
							(is_pmkid_cache_in_entry(pEntry))) {
							apcli_delete_pmkid_cache(pAd, Addr2, ifIndex
#ifdef MAC_REPEATER_SUPPORT
								, CliIdx
#endif
								);
							pEntry->pmkid = NULL;
							pEntry->pmk_cache = NULL;
						}
					}
					if ((pEntry->need_process_ecdh_ie == TRUE) && (ie_list->ecdh_ie.length > 0))
						NdisMoveMemory(&pEntry->ecdh_ie,
							&ie_list->ecdh_ie,
							sizeof(EXT_ECDH_PARAMETER_IE));
				}
#endif /*CONFIG_OWE_SUPPORT*/


				ApCliCtrlMsg.Status = MLME_SUCCESS;
#ifdef MAC_REPEATER_SUPPORT
				ApCliCtrlMsg.BssIdx = ifIndex;
				ApCliCtrlMsg.CliIdx = CliIdx;
				ifIndex = (USHORT)(Elem->Priv);
#endif /* MAC_REPEATER_SUPPORT */

				MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_ASSOC_RSP,
							sizeof(APCLI_CTRL_MSG_STRUCT), &ApCliCtrlMsg, ifIndex);
			}
			else
			{
				ApCliCtrlMsg.Status = Status;
				MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_ASSOC_RSP,
							sizeof(APCLI_CTRL_MSG_STRUCT), &ApCliCtrlMsg, ifIndex);
			}

			*pCurrState = APCLI_ASSOC_IDLE;
		}
	}
	else
	{
		DBGPRINT(RT_DEBUG_TRACE, ("APCLI_ASSOC - ApCliPeerAssocRspAction() sanity check fail\n"));
	}

	if (ie_list)
		os_free_mem(pAd, ie_list);

	return;
}

/*
    ==========================================================================
    Description:
        left part of IEEE 802.11/1999 p.374 
    Parameters:
        Elem - MLME message containing the received frame
    ==========================================================================
 */
static VOID ApCliPeerDisassocAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem) 
{
	UCHAR         Addr2[MAC_ADDR_LEN];
	USHORT        Reason;
	USHORT ifIndex = (USHORT)(Elem->Priv);
	ULONG *pCurrState = NULL;
#ifdef MAC_REPEATER_SUPPORT
	UCHAR CliIdx = 0xFF;
#endif /* MAC_REPEATER_SUPPORT */

#ifdef WH_EZ_SETUP
	if(IS_ADPTR_EZ_SETUP_ENABLED(pAd))
		EZ_DEBUG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ApCliPeerDisassocAction()\n"));
#endif

	if ((ifIndex >= MAX_APCLI_NUM)
#ifdef MAC_REPEATER_SUPPORT
		&& (ifIndex < 64)
#endif /* MAC_REPEATER_SUPPORT */
		)
		return;

#ifdef MAC_REPEATER_SUPPORT
	if (ifIndex >= 64)
	{
		CliIdx = ((ifIndex - 64) % 16);
		ifIndex = ((ifIndex - 64) / 16);
		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].AssocCurrState;
	}
	else
#endif /* MAC_REPEATER_SUPPORT */
		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].AssocCurrState;

	if(PeerDisassocSanity(pAd, Elem->Msg, Elem->MsgLen, Addr2, &Reason))
	{
		if (MAC_ADDR_EQUAL(pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.Bssid, Addr2))
		{
			*pCurrState = APCLI_ASSOC_IDLE;

#ifdef MAC_REPEATER_SUPPORT
			ifIndex = (USHORT)(Elem->Priv);
#endif /* MAC_REPEATER_SUPPORT */
			MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_PEER_DISCONNECT_REQ, 0, NULL, ifIndex);
#ifdef MAC_REPEATER_SUPPORT
			if ((pAd->ApCfg.bMACRepeaterEn == TRUE) && (ifIndex >= 64))
			{
				RTMP_MLME_HANDLER(pAd);
				ifIndex = ((ifIndex - 64) / 16);
				//RTMPRemoveRepeaterEntry(pAd, ifIndex, CliIdx);
			}
#endif /* MAC_REPEATER_SUPPORT */
        }
    }
    else
    {
        DBGPRINT(RT_DEBUG_TRACE, ("APCLI_ASSOC - ApCliPeerDisassocAction() sanity check fail\n"));
    }
	
	return;
}

/*
    ==========================================================================
    Description:
        what the state machine will do after assoc timeout
    ==========================================================================
 */
static VOID ApCliAssocTimeoutAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem) 
{
	USHORT ifIndex = (USHORT)(Elem->Priv);
	PULONG pCurrState = NULL;
#ifdef MAC_REPEATER_SUPPORT
	UCHAR CliIdx = 0xFF;
#endif /* MAC_REPEATER_SUPPORT */

	DBGPRINT(RT_DEBUG_TRACE, ("APCLI_ASSOC - ApCliAssocTimeoutAction\n"));

	if ((ifIndex >= MAX_APCLI_NUM)
#ifdef MAC_REPEATER_SUPPORT
		&& (ifIndex < 64)
#endif /* MAC_REPEATER_SUPPORT */
		)
	return;

#ifdef MAC_REPEATER_SUPPORT
	if (ifIndex >= 64)
	{
		CliIdx = ((ifIndex - 64) % 16);
		ifIndex = ((ifIndex - 64) / 16);
		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].AssocCurrState;
	}
	else
#endif /* MAC_REPEATER_SUPPORT */
		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].AssocCurrState;

	*pCurrState = APCLI_ASSOC_IDLE;

#ifdef MAC_REPEATER_SUPPORT
	ifIndex = (USHORT)(Elem->Priv);
#endif /* MAC_REPEATER_SUPPORT */

	MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_ASSOC_REQ_TIMEOUT, 0, NULL, ifIndex);

	return;
}

static VOID ApCliInvalidStateWhenAssoc(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem) 
{
	APCLI_CTRL_MSG_STRUCT ApCliCtrlMsg;
	USHORT ifIndex = (USHORT)(Elem->Priv);
	PULONG pCurrState = NULL;
#ifdef MAC_REPEATER_SUPPORT
	UCHAR CliIdx = 0xFF;
#endif /* MAC_REPEATER_SUPPORT */

	if ((ifIndex >= MAX_APCLI_NUM)
#ifdef MAC_REPEATER_SUPPORT
		&& (ifIndex < 64)
#endif /* MAC_REPEATER_SUPPORT */
		)
		return;
	
#ifdef MAC_REPEATER_SUPPORT
	if (ifIndex >= 64)
	{
		CliIdx = ((ifIndex - 64) % 16);
		ifIndex = ((ifIndex - 64) / 16);
		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].AssocCurrState;
	}
	else
#endif /* MAC_REPEATER_SUPPORT */
		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].AssocCurrState;

	DBGPRINT(RT_DEBUG_TRACE, ("APCLI_ASSOC - ApCliInvalidStateWhenAssoc(state=%ld), reset APCLI_ASSOC state machine\n", *pCurrState));
	*pCurrState = APCLI_ASSOC_IDLE;

	ApCliCtrlMsg.Status = MLME_STATE_MACHINE_REJECT;

#ifdef MAC_REPEATER_SUPPORT
	ifIndex = (USHORT)(Elem->Priv);
#endif /* MAC_REPEATER_SUPPORT */

	MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_ASSOC_RSP,
		sizeof(APCLI_CTRL_MSG_STRUCT), &ApCliCtrlMsg, ifIndex);

	return;
}

static VOID ApCliInvalidStateWhenDisassociate(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem) 
{
	APCLI_CTRL_MSG_STRUCT ApCliCtrlMsg;
	USHORT ifIndex = (USHORT)(Elem->Priv);
	ULONG *pCurrState = NULL;
#ifdef MAC_REPEATER_SUPPORT
	UCHAR CliIdx = 0xFF;
#endif /* MAC_REPEATER_SUPPORT */

	if ((ifIndex >= MAX_APCLI_NUM)
#ifdef MAC_REPEATER_SUPPORT
		&& (ifIndex < 64)
#endif /* MAC_REPEATER_SUPPORT */
		)
		return;

#ifdef MAC_REPEATER_SUPPORT
	if (ifIndex >= 64)
	{
		CliIdx = ((ifIndex - 64) % 16);
		ifIndex = ((ifIndex - 64) / 16);
		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].AssocCurrState;
	}
	else
#endif /* MAC_REPEATER_SUPPORT */
		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].AssocCurrState;

	DBGPRINT(RT_DEBUG_TRACE, ("APCLI_ASSOC - InvalidStateWhenApCliDisassoc(state=%ld), reset APCLI_ASSOC state machine\n", *pCurrState));
	*pCurrState = APCLI_ASSOC_IDLE;

	ApCliCtrlMsg.Status = MLME_STATE_MACHINE_REJECT;

#ifdef MAC_REPEATER_SUPPORT
	ApCliCtrlMsg.BssIdx = ifIndex;
	ApCliCtrlMsg.CliIdx = CliIdx;
	ifIndex = (USHORT)(Elem->Priv);
#endif /* MAC_REPEATER_SUPPORT */

	MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_DEASSOC_RSP,
		sizeof(APCLI_CTRL_MSG_STRUCT), &ApCliCtrlMsg, ifIndex);

	return;
}

/*
    ==========================================================================
    Description:
        procedures on IEEE 802.11/1999 p.376 
    Parametrs:
    ==========================================================================
 */
static VOID ApCliAssocPostProc(
	IN PRTMP_ADAPTER pAd, 
	IN PUCHAR pAddr2, 
	IN USHORT CapabilityInfo, 
	IN USHORT IfIndex, 
	IN UCHAR SupRate[], 
	IN UCHAR SupRateLen,
	IN UCHAR ExtRate[],
	IN UCHAR ExtRateLen,
	IN PEDCA_PARM pEdcaParm,
	IN HT_CAPABILITY_IE *pHtCapability,
	IN UCHAR HtCapabilityLen, 
	IN ADD_HT_INFO_IE *pAddHtInfo)
{
	APCLI_STRUCT *pApCliEntry = NULL;

	if (IfIndex >= MAX_APCLI_NUM)
		return;

	pApCliEntry = &pAd->ApCfg.ApCliTab[IfIndex];

	pApCliEntry->MlmeAux.BssType = BSS_INFRA;	
	pApCliEntry->MlmeAux.CapabilityInfo = CapabilityInfo & SUPPORTED_CAPABILITY_INFO;
	NdisMoveMemory(&pApCliEntry->MlmeAux.APEdcaParm, pEdcaParm, sizeof(EDCA_PARM));

	/* filter out un-supported rates */
	pApCliEntry->MlmeAux.SupRateLen = SupRateLen;
	NdisMoveMemory(pApCliEntry->MlmeAux.SupRate, SupRate, SupRateLen);
    RTMPCheckRates(pAd, pApCliEntry->MlmeAux.SupRate, &(pApCliEntry->MlmeAux.SupRateLen));

	/* filter out un-supported rates */
	pApCliEntry->MlmeAux.ExtRateLen = ExtRateLen;
	NdisMoveMemory(pApCliEntry->MlmeAux.ExtRate, ExtRate, ExtRateLen);
    RTMPCheckRates(pAd, pApCliEntry->MlmeAux.ExtRate, &(pApCliEntry->MlmeAux.ExtRateLen));

	DBGPRINT(RT_DEBUG_TRACE, (HtCapabilityLen ? "%s===> 11n HT STA\n" : "%s===> legacy STA\n", __FUNCTION__));

#ifdef DOT11_N_SUPPORT
	if (HtCapabilityLen > 0 && WMODE_CAP_N(pAd->CommonCfg.PhyMode))
		ApCliCheckHt(pAd, IfIndex, pHtCapability, pAddHtInfo);
#endif /* DOT11_N_SUPPORT */

}

#endif /* APCLI_SUPPORT */


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
	sta_sync.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Fonchi		2006-06-23      modified for rt61-APClinent
*/

#ifdef APCLI_SUPPORT

#include "rt_config.h"

static VOID ApCliProbeTimeout(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3);

static VOID ApCliMlmeProbeReqAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem);

static VOID ApCliPeerProbeRspAtJoinAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);

static VOID ApCliProbeTimeoutAtJoinAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem);

static VOID ApCliInvalidStateWhenJoin(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);

static VOID ApCliEnqueueProbeRequest(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR SsidLen,
	IN PCHAR Ssid,
	IN PCHAR Bssid,
	IN USHORT ifIndex);

static VOID ApCliMlmeScanReqAction(
	IN RTMP_ADAPTER *pAd,
	IN MLME_QUEUE_ELEM *Elem);

#ifdef SMART_MESH_MONITOR
static VOID ApCliPeerProbeRspAtScanReqAction(
	IN RTMP_ADAPTER *pAd,
	IN MLME_QUEUE_ELEM *Elem);
#endif /* SMART_MESH_MONITOR */

DECLARE_TIMER_FUNCTION(ApCliProbeTimeout);
BUILD_TIMER_FUNCTION(ApCliProbeTimeout);

/*
    ==========================================================================
    Description:
        The sync state machine, 
    Parameters:
        Sm - pointer to the state machine
    Note:
        the state machine looks like the following
    ==========================================================================
 */
VOID ApCliSyncStateMachineInit(
	IN PRTMP_ADAPTER pAd,
	IN STATE_MACHINE *Sm,
	OUT STATE_MACHINE_FUNC Trans[])
{
	UCHAR i;
#ifdef APCLI_CONNECTION_TRIAL		
	PAPCLI_STRUCT	pApCliEntry;
#endif /*APCLI_CONNECTION_TRIAL*/

	StateMachineInit(Sm, (STATE_MACHINE_FUNC*)Trans,
		APCLI_MAX_SYNC_STATE, APCLI_MAX_SYNC_MSG,
		(STATE_MACHINE_FUNC)Drop, APCLI_SYNC_IDLE,
		APCLI_SYNC_MACHINE_BASE);

	/* column 1 */
	StateMachineSetAction(Sm, APCLI_SYNC_IDLE, APCLI_MT2_MLME_PROBE_REQ, (STATE_MACHINE_FUNC)ApCliMlmeProbeReqAction);
	StateMachineSetAction(Sm, APCLI_SYNC_IDLE, APCLI_MT2_MLME_SCAN_REQ, (STATE_MACHINE_FUNC)ApCliMlmeScanReqAction);
#ifdef SMART_MESH_MONITOR
	StateMachineSetAction(Sm, APCLI_SYNC_IDLE, APCLI_MT2_PEER_PROBE_RSP, (STATE_MACHINE_FUNC)ApCliPeerProbeRspAtScanReqAction);
#endif /* SMART_MESH_MONITOR */

	/*column 2 */
	StateMachineSetAction(Sm, APCLI_JOIN_WAIT_PROBE_RSP, APCLI_MT2_MLME_PROBE_REQ, (STATE_MACHINE_FUNC)ApCliInvalidStateWhenJoin);
	StateMachineSetAction(Sm, APCLI_JOIN_WAIT_PROBE_RSP, APCLI_MT2_PEER_PROBE_RSP, (STATE_MACHINE_FUNC)ApCliPeerProbeRspAtJoinAction);
	StateMachineSetAction(Sm, APCLI_JOIN_WAIT_PROBE_RSP, APCLI_MT2_PROBE_TIMEOUT, (STATE_MACHINE_FUNC)ApCliProbeTimeoutAtJoinAction);
	StateMachineSetAction(Sm, APCLI_JOIN_WAIT_PROBE_RSP, APCLI_MT2_PEER_BEACON, (STATE_MACHINE_FUNC)ApCliPeerProbeRspAtJoinAction);


	for (i = 0; i < MAX_APCLI_NUM; i++)
	{
		/* timer init */
#ifdef APCLI_CONNECTION_TRIAL		
		pApCliEntry = &pAd->ApCfg.ApCliTab[i];
#endif /*APCLI_CONNECTION_TRIAL*/

#ifdef APCLI_CONNECTION_TRIAL	
		RTMPInitTimer(pAd, &pAd->ApCfg.ApCliTab[i].MlmeAux.ProbeTimer, GET_TIMER_FUNCTION(ApCliProbeTimeout), (PVOID)pApCliEntry, FALSE);
#else
		RTMPInitTimer(pAd, &pAd->ApCfg.ApCliTab[i].MlmeAux.ProbeTimer, GET_TIMER_FUNCTION(ApCliProbeTimeout), pAd, FALSE);
#endif /* APCLI_CONNECTION_TRIAL */
		pAd->ApCfg.ApCliTab[i].SyncCurrState = APCLI_SYNC_IDLE;
	}

	return;
}

/* 
    ==========================================================================
    Description:
        Becaon timeout handler, executed in timer thread
    ==========================================================================
 */
static VOID ApCliProbeTimeout(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3)
{
#ifdef APCLI_CONNECTION_TRIAL
	PAPCLI_STRUCT pApCliEntry = (APCLI_STRUCT *)FunctionContext;
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pApCliEntry->pAd;
#else
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)FunctionContext;
#endif /*APCLI_CONNECTION_TRIAL*/

	DBGPRINT(RT_DEBUG_TRACE, ("ApCli_SYNC - ProbeReqTimeout\n"));
#ifndef APCLI_CONNECTION_TRIAL
	MlmeEnqueue(pAd, APCLI_SYNC_STATE_MACHINE, APCLI_MT2_PROBE_TIMEOUT, 0, NULL, 0);
#else
	MlmeEnqueue(pAd, APCLI_SYNC_STATE_MACHINE, APCLI_MT2_PROBE_TIMEOUT, 0, NULL, pApCliEntry->ifIndex);
#endif /* APCLI_CONNECTION_TRIAL */
	RTMP_MLME_HANDLER(pAd);

	return;
}

/* 
    ==========================================================================
    Description:
        MLME PROBE req state machine procedure
    ==========================================================================
 */
static VOID ApCliMlmeProbeReqAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	BOOLEAN Cancelled;
	APCLI_MLME_JOIN_REQ_STRUCT *Info = (APCLI_MLME_JOIN_REQ_STRUCT *)(Elem->Msg);
	USHORT ifIndex = (USHORT)(Elem->Priv);
	PULONG pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].SyncCurrState;
	APCLI_STRUCT *pApCliEntry = NULL;
	struct wifi_dev *wdev;
	ULONG bss_idx = BSS_NOT_FOUND;

	DBGPRINT(RT_DEBUG_TRACE, ("ApCli SYNC - ApCliMlmeProbeReqAction(Ssid %s)\n", Info->Ssid));

	if (ifIndex >= MAX_APCLI_NUM)
		return;

	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];
	wdev = &pApCliEntry->wdev;

	/* reset all the timers */
	RTMPCancelTimer(&(pApCliEntry->MlmeAux.ProbeTimer), &Cancelled);

	pApCliEntry->MlmeAux.Rssi = -9999;
	bss_idx = BssSsidTableSearchBySSID(&pAd->ScanTab, (PCHAR)Info->Ssid, Info->SsidLen);
	if (bss_idx == BSS_NOT_FOUND)
	{
#ifdef APCLI_CONNECTION_TRIAL
		if (pApCliEntry->TrialCh ==0)
			pApCliEntry->MlmeAux.Channel = pAd->CommonCfg.Channel;
		else
			pApCliEntry->MlmeAux.Channel = pApCliEntry->TrialCh;
#else
		pApCliEntry->MlmeAux.Channel = pAd->CommonCfg.Channel;
#endif /* APCLI_CONNECTION_TRIAL */
	}
	else
	{
#ifdef APCLI_CONNECTION_TRIAL
		if (pApCliEntry->TrialCh ==0)
			pApCliEntry->MlmeAux.Channel = pAd->CommonCfg.Channel;
		else
			pApCliEntry->MlmeAux.Channel = pApCliEntry->TrialCh;
#else
		DBGPRINT(RT_DEBUG_TRACE, ("%s, Found %s in scanTable , goto channel %d\n", 
				__FUNCTION__, pAd->ScanTab.BssEntry[bss_idx].Ssid,  
				pAd->ScanTab.BssEntry[bss_idx].Channel));

		pApCliEntry->MlmeAux.Channel = pAd->ScanTab.BssEntry[bss_idx].Channel;
#endif /* APCLI_CONNECTION_TRIAL */
	}

#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE
	pApCliEntry->MlmeAux.SupRateLen = pAd->cfg80211_ctrl.P2pSupRateLen;
	NdisMoveMemory(pApCliEntry->MlmeAux.SupRate, pAd->cfg80211_ctrl.P2pSupRate, pAd->cfg80211_ctrl.P2pSupRateLen);
	
	pApCliEntry->MlmeAux.ExtRateLen = pAd->cfg80211_ctrl.P2pExtRateLen;
	NdisMoveMemory(pApCliEntry->MlmeAux.ExtRate, pAd->cfg80211_ctrl.P2pExtRate, pAd->cfg80211_ctrl.P2pExtRateLen);
#else
#ifdef APCLI_AUTO_BW_SUPPORT
        pApCliEntry->MlmeAux.SupRateLen = wdev->SupRateLen;
        NdisMoveMemory(pApCliEntry->MlmeAux.SupRate, wdev->SupRate, wdev->SupRateLen);

        /* Prepare the default value for extended rate */
        pApCliEntry->MlmeAux.ExtRateLen = wdev->ExtRateLen;
        NdisMoveMemory(pApCliEntry->MlmeAux.ExtRate, wdev->ExtRate, wdev->ExtRateLen);
#else
	pApCliEntry->MlmeAux.SupRateLen = pAd->CommonCfg.SupRateLen;
	NdisMoveMemory(pApCliEntry->MlmeAux.SupRate, pAd->CommonCfg.SupRate, pAd->CommonCfg.SupRateLen);

	/* Prepare the default value for extended rate */
	pApCliEntry->MlmeAux.ExtRateLen = pAd->CommonCfg.ExtRateLen;
	NdisMoveMemory(pApCliEntry->MlmeAux.ExtRate, pAd->CommonCfg.ExtRate, pAd->CommonCfg.ExtRateLen);
#endif /* APCLI_AUTO_BW_SUPPORT */
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE */

	RTMPSetTimer(&(pApCliEntry->MlmeAux.ProbeTimer), PROBE_TIMEOUT);

#ifdef APCLI_CONNECTION_TRIAL
	NdisZeroMemory(pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.Bssid, MAC_ADDR_LEN);
	NdisZeroMemory(pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.Ssid, MAX_LEN_OF_SSID);
	NdisCopyMemory(pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.Bssid, Info->Bssid, MAC_ADDR_LEN);
	NdisCopyMemory(pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.Ssid, Info->Ssid, Info->SsidLen);
#endif /* APCLI_CONNECTION_TRIAL */

	ApCliEnqueueProbeRequest(pAd, Info->SsidLen, (PCHAR)Info->Ssid, (PCHAR)Info->Bssid, ifIndex);

	DBGPRINT(RT_DEBUG_TRACE, ("ApCli SYNC - Start Probe the SSID %s on channel =%d\n", Info->Ssid, pApCliEntry->MlmeAux.Channel));

	*pCurrState = APCLI_JOIN_WAIT_PROBE_RSP;

	return;
}


/* 
    ==========================================================================
    Description:
        When waiting joining the (I)BSS, beacon received from external
    ==========================================================================
 */
static VOID ApCliPeerProbeRspAtJoinAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem) 
{
	USHORT LenVIE;
	UCHAR *VarIE = NULL;
	NDIS_802_11_VARIABLE_IEs *pVIE = NULL;
	APCLI_CTRL_MSG_STRUCT ApCliCtrlMsg;
	PAPCLI_STRUCT pApCliEntry = NULL;
	struct wifi_dev *wdev;
#ifdef DOT11_N_SUPPORT
	UCHAR CentralChannel;
#endif /* DOT11_N_SUPPORT */
	USHORT ifIndex = (USHORT)(Elem->Priv);
	ULONG *pCurrState;
	BCN_IE_LIST *ie_list = NULL;
	UCHAR PhyMode = pAd->CommonCfg.PhyMode;
	PFRAME_802_11 pFrame = NULL;

	if (ifIndex >= MAX_APCLI_NUM)
		return;

	/* Init Variable IE structure */
	os_alloc_mem(NULL, (UCHAR **)&VarIE, MAX_VIE_LEN);
	if (VarIE == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s: Allocate memory fail!!!\n", __FUNCTION__));
		goto LabelErr;
	}
	pVIE = (PNDIS_802_11_VARIABLE_IEs) VarIE;
	pVIE->Length = 0;
	
	os_alloc_mem(NULL, (UCHAR **)&ie_list, sizeof(BCN_IE_LIST));
	if (ie_list == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s: Allocate ie_list fail!!!\n", __FUNCTION__));
		goto LabelErr;
	}
	NdisZeroMemory(ie_list, sizeof(BCN_IE_LIST));
	pFrame = (PFRAME_802_11)Elem->Msg;

	pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].SyncCurrState;
	if (PeerBeaconAndProbeRspSanity(pAd, 
								Elem->Msg, 
								Elem->MsgLen, 
								Elem->Channel,
								ie_list,
								&LenVIE,
								pVIE))
	{
		/*
			BEACON from desired BSS/IBSS found. We should be able to decide most
			BSS parameters here.
			Q. But what happen if this JOIN doesn't conclude a successful ASSOCIATEION?
				Do we need to receover back all parameters belonging to previous BSS?
			A. Should be not. There's no back-door recover to previous AP. It still need
				a new JOIN-AUTH-ASSOC sequence.
		*/
		INT ssidEqualFlag = FALSE;
		INT ssidEmptyFlag = FALSE;
		INT bssidEqualFlag = FALSE;
		INT bssidEmptyFlag = FALSE;
		INT matchFlag = FALSE;

		ULONG   Bssidx;
        LONG    RealRssi = -127;
#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE
		RealRssi = (LONG)(RTMPMaxRssi(pAd, ConvertToRssi(pAd, Elem->Rssi0, RSSI_0), 
						   ConvertToRssi(pAd, Elem->Rssi1, RSSI_1), 
						   ConvertToRssi(pAd, Elem->Rssi2, RSSI_2)));		
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE */

		/* Update ScanTab */
		Bssidx = BssTableSearch(&pAd->ScanTab, ie_list->Bssid, ie_list->Channel);
		if (Bssidx == BSS_NOT_FOUND)
		{
			/* discover new AP of this network, create BSS entry */
#ifdef CUSTOMER_DCC_FEATURE
			Bssidx = BssTableSetEntry(pAd, &pAd->ScanTab, ie_list, -127, LenVIE, pVIE, Elem->Snr0, Elem->Snr1);
#else
			Bssidx = BssTableSetEntry(pAd, &pAd->ScanTab, ie_list, -127, LenVIE, pVIE);
#endif
			if (Bssidx == BSS_NOT_FOUND) /* return if BSS table full */
			{
				DBGPRINT(RT_DEBUG_ERROR, ("ERROR: Driver ScanTable Full In Apcli ProbeRsp Join\n"));
				goto LabelErr;
			}

			NdisMoveMemory(pAd->ScanTab.BssEntry[Bssidx].PTSF, &Elem->Msg[24], 4);
			NdisMoveMemory(&pAd->ScanTab.BssEntry[Bssidx].TTSF[0], &Elem->TimeStamp.u.LowPart, 4);
			NdisMoveMemory(&pAd->ScanTab.BssEntry[Bssidx].TTSF[4], &Elem->TimeStamp.u.LowPart, 4);
			pAd->ScanTab.BssEntry[Bssidx].MinSNR = Elem->Signal % 10;
			if (pAd->ScanTab.BssEntry[Bssidx].MinSNR == 0)
				pAd->ScanTab.BssEntry[Bssidx].MinSNR = -5;
			
			NdisMoveMemory(pAd->ScanTab.BssEntry[Bssidx].MacAddr, ie_list->Addr2, MAC_ADDR_LEN);
		}

#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE
                DBGPRINT(RT_DEBUG_TRACE, ("Info: Update the SSID %s in Kernel Table\n", ie_list->Ssid));
                RT_CFG80211_SCANNING_INFORM(pAd, Bssidx, ie_list->Channel, (UCHAR *)Elem->Msg, Elem->MsgLen, RealRssi);
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE */


		pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];
		wdev = &pApCliEntry->wdev;

		/* Check the Probe-Rsp's Ssid. */
#ifdef WSC_AP_SUPPORT
        if ((pApCliEntry->WscControl.WscConfMode != WSC_DISABLE) &&
            (pApCliEntry->WscControl.bWscTrigger == TRUE))
        {
#ifdef SMART_MESH
            if((pApCliEntry->WscControl.WscMode == 2) && 
                (pApCliEntry->WscControl.bWscPBCAddrMode == TRUE))
                bssidEqualFlag = MAC_ADDR_EQUAL(pApCliEntry->WscControl.WscPBCAddr, ie_list->Bssid);
            else
#endif /* SMART_MESH */
            {
                if(!MAC_ADDR_EQUAL(pApCliEntry->CfgApCliBssid, ZERO_MAC_ADDR))
    			    bssidEqualFlag = MAC_ADDR_EQUAL(pApCliEntry->CfgApCliBssid, ie_list->Bssid);
    		    else
    			    bssidEmptyFlag = TRUE;
            }
            
            if(pApCliEntry->WscControl.WscSsid.SsidLength != 0)
			    ssidEqualFlag = SSID_EQUAL(pApCliEntry->WscControl.WscSsid.Ssid,
			                             pApCliEntry->WscControl.WscSsid.SsidLength,ie_list->Ssid, ie_list->SsidLen);
		    else
			    ssidEmptyFlag = TRUE;
        }
        else
#endif /* WSC_AP_SUPPORT */
        {
		/* Check the Probe-Rsp's Bssid. */
		if(!MAC_ADDR_EQUAL(pApCliEntry->CfgApCliBssid, ZERO_MAC_ADDR))
			bssidEqualFlag = MAC_ADDR_EQUAL(pApCliEntry->CfgApCliBssid, ie_list->Bssid);
		else
			bssidEmptyFlag = TRUE;

		if(pApCliEntry->CfgSsidLen != 0)
			ssidEqualFlag = SSID_EQUAL(pApCliEntry->CfgSsid, pApCliEntry->CfgSsidLen, ie_list->Ssid, ie_list->SsidLen);
		else
			ssidEmptyFlag = TRUE;
        }

		/* bssid and ssid, Both match. */
		if (bssidEqualFlag && ssidEqualFlag)
			matchFlag = TRUE;

		/* ssid match but bssid doesn't be indicate. */
		else if(ssidEqualFlag && bssidEmptyFlag)
			matchFlag = TRUE;

		/* user doesn't indicate any bssid or ssid. AP-Clinet will auto pick a AP to join by most strong siganl strength. */
		else if (bssidEmptyFlag && ssidEmptyFlag)
		{      
			matchFlag = TRUE;
#ifdef SMART_MESH
			matchFlag = FALSE;
#endif /* SMART_MESH */
		}

		DBGPRINT(RT_DEBUG_TRACE, ("SYNC - bssidEqualFlag=%d, ssidEqualFlag=%d, matchFlag=%d\n",
					bssidEqualFlag, ssidEqualFlag, matchFlag));
		if (matchFlag)
		{
			/* Validate RSN IE if necessary, then copy store this information */
			if ((LenVIE > 0) 
#ifdef WSC_AP_SUPPORT
                		&& ((pApCliEntry->WscControl.WscConfMode == WSC_DISABLE) || 
                		(pApCliEntry->WscControl.bWscTrigger == FALSE))
#endif /* WSC_AP_SUPPORT */
#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE
				/* When using CFG80211 and trigger WPS, do not check security. */
				&& ! (pApCliEntry->wpa_supplicant_info.WpaSupplicantUP & WPA_SUPPLICANT_ENABLE_WPS)
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE */
                	)
			{
				if (ApCliValidateRSNIE(pAd, (PEID_STRUCT)pVIE, LenVIE, ifIndex))
				{
					pApCliEntry->MlmeAux.VarIELen = LenVIE;
					NdisMoveMemory(pApCliEntry->MlmeAux.VarIEs, pVIE, pApCliEntry->MlmeAux.VarIELen);
				}
				else
				{
					/* ignore this response */
					pApCliEntry->MlmeAux.VarIELen = 0;
					DBGPRINT(RT_DEBUG_ERROR, ("ERROR: The RSN IE of this received Probe-resp is dis-match !!!!!!!!!! \n"));
					goto LabelErr;
				}
			}
			else
			{
				if (pApCliEntry->wdev.AuthMode >= Ndis802_11AuthModeWPA
#ifdef WSC_AP_SUPPORT
					&& ((pApCliEntry->WscControl.WscConfMode == WSC_DISABLE) || 
                			(pApCliEntry->WscControl.bWscTrigger == FALSE))
#endif /* WSC_AP_SUPPORT */
                    )
				{
					/* ignore this response */
					DBGPRINT(RT_DEBUG_ERROR, ("ERROR: The received Probe-resp has empty RSN IE !!!!!!!!!! \n"));
					goto LabelErr;
				}	
				
				pApCliEntry->MlmeAux.VarIELen = 0;
			}

#ifdef SMART_MESH
			PSMART_MESH_CFG pSmartMeshCfg = NULL;
			pSmartMeshCfg = &pApCliEntry->SmartMeshCfg;

			if(((pSmartMeshCfg->HiFiFlagMask != 0) && (pSmartMeshCfg->HiFiFlagValue != 0)) &&
				((ie_list->VIEFlag & pSmartMeshCfg->HiFiFlagMask) == pSmartMeshCfg->HiFiFlagValue))
			{
				pAd->ScanTab.BssEntry[Bssidx].bHyperFiPeer = TRUE;
				pApCliEntry->MlmeAux.bHyperFiPeer = TRUE;
			}
			else
			{
				pAd->ScanTab.BssEntry[Bssidx].bHyperFiPeer = FALSE;
				pApCliEntry->MlmeAux.bHyperFiPeer = FALSE;
			}
			
			if((pSmartMeshCfg->bHiFiPeerFilter == TRUE) &&
			   (pAd->ScanTab.BssEntry[Bssidx].bHyperFiPeer == FALSE))
			{
				DBGPRINT(RT_DEBUG_OFF, ("Reject this PROBE_RSP due to not desired Hyper-Fi peer(%02X:%02X:%02X:%02X:%02X:%02X).\n",PRINT_MAC(pAd->ScanTab.BssEntry[Bssidx].Bssid)));
				goto LabelErr;
			}

			if(pAd->ScanTab.BssEntry[Bssidx].bSupportSmartMesh != ie_list->bSupportSmartMesh)
				pAd->ScanTab.BssEntry[Bssidx].bSupportSmartMesh = ie_list->bSupportSmartMesh;
			
			if(pAd->ScanTab.BssEntry[Bssidx].bSupportSmartMesh)
			{
				pApCliEntry->MlmeAux.bSupportSmartMesh = TRUE;
				DBGPRINT(RT_DEBUG_OFF, ("AP supports SMART MESH\n"));
			}
			else
				pApCliEntry->MlmeAux.bSupportSmartMesh = FALSE;

			if(pAd->ScanTab.BssEntry[Bssidx].bHyperFiPeer)
				DBGPRINT(RT_DEBUG_OFF, ("AP is Hyper-Fi device\n"));
#endif /* SMART_MESH */
#ifdef MWDS
			if(pAd->ScanTab.BssEntry[Bssidx].bSupportMWDS != ie_list->bSupportMWDS)
				pAd->ScanTab.BssEntry[Bssidx].bSupportMWDS = ie_list->bSupportMWDS;
			
			if(pAd->ScanTab.BssEntry[Bssidx].bSupportMWDS)
			{
				pApCliEntry->MlmeAux.bSupportMWDS = TRUE;
				DBGPRINT(RT_DEBUG_OFF, ("AP supports MWDS\n"));
			}
			else
				pApCliEntry->MlmeAux.bSupportMWDS = FALSE;
#endif /* MWDS */

#ifdef WSC_AP_SUPPORT
#ifdef SMART_MESH_HIDDEN_WPS
            if(pAd->ScanTab.BssEntry[Bssidx].bSupportHiddenWPS != ie_list->bSupportHiddenWPS)
			   pAd->ScanTab.BssEntry[Bssidx].bSupportHiddenWPS = ie_list->bSupportHiddenWPS;
            
            if(pAd->ScanTab.BssEntry[Bssidx].bSupportHiddenWPS)
				DBGPRINT(RT_DEBUG_OFF, ("AP supports HiddenWPS\n"));

            if(pAd->ScanTab.BssEntry[Bssidx].bRunningHiddenWPS != ie_list->bRunningHiddenWPS)
                pAd->ScanTab.BssEntry[Bssidx].bRunningHiddenWPS = ie_list->bRunningHiddenWPS;

            if(pAd->ScanTab.BssEntry[Bssidx].bRunningHiddenWPS)
                DBGPRINT(RT_DEBUG_OFF, ("AP is running HiddenWPS\n"));
#endif /* SMART_MESH_HIDDEN_WPS */
#endif /* WSC_AP_SUPPORT */
			DBGPRINT(RT_DEBUG_TRACE, ("SYNC - receive desired PROBE_RSP at JoinWaitProbeRsp... Channel = %d\n",
							ie_list->Channel));

			/* if the Bssid doesn't be indicated then you need to decide which AP to connect by most strong Rssi signal strength. */
			if (bssidEqualFlag == FALSE)
			{
				/* caculate real rssi value. */
				CHAR Rssi0 = ConvertToRssi(pAd, Elem->Rssi0, RSSI_0);
				CHAR Rssi1 = ConvertToRssi(pAd, Elem->Rssi1, RSSI_1);
				CHAR Rssi2 = ConvertToRssi(pAd, Elem->Rssi2, RSSI_2);
				LONG RealMaxRssi = (LONG)(RTMPMaxRssi(pAd, Rssi0, Rssi1, Rssi2));

				DBGPRINT(RT_DEBUG_TRACE, ("SYNC - previous Rssi = %ld current Rssi=%ld\n", pApCliEntry->MlmeAux.Rssi, (LONG)RealMaxRssi));
				if (pApCliEntry->MlmeAux.Rssi > (LONG)RealMaxRssi)
					goto LabelErr;
				else
					pApCliEntry->MlmeAux.Rssi = RealMaxRssi;
			}
			else
			{
				BOOLEAN Cancelled;
				RTMPCancelTimer(&pApCliEntry->MlmeAux.ProbeTimer, &Cancelled);
			}

			NdisMoveMemory(pApCliEntry->MlmeAux.Ssid, ie_list->Ssid, ie_list->SsidLen);
			pApCliEntry->MlmeAux.SsidLen = ie_list->SsidLen;

			NdisMoveMemory(pApCliEntry->MlmeAux.Bssid, ie_list->Bssid, MAC_ADDR_LEN);
			pApCliEntry->MlmeAux.CapabilityInfo = ie_list->CapabilityInfo & SUPPORTED_CAPABILITY_INFO;
			pApCliEntry->MlmeAux.BssType = ie_list->BssType;
			pApCliEntry->MlmeAux.BeaconPeriod = ie_list->BeaconPeriod;
			pApCliEntry->MlmeAux.Channel = ie_list->Channel;
			pApCliEntry->MlmeAux.CentralChannel = ie_list->Channel; /* by default */
			pApCliEntry->MlmeAux.AtimWin = ie_list->AtimWin;
			pApCliEntry->MlmeAux.CfpPeriod = ie_list->CfParm.CfpPeriod;
			pApCliEntry->MlmeAux.CfpMaxDuration = ie_list->CfParm.CfpMaxDuration;
			pApCliEntry->MlmeAux.APRalinkIe = ie_list->RalinkIe;

			/* Copy AP's supported rate to MlmeAux for creating assoication request */
			/* Also filter out not supported rate */
			pApCliEntry->MlmeAux.SupRateLen = ie_list->SupRateLen;
			NdisMoveMemory(pApCliEntry->MlmeAux.SupRate, ie_list->SupRate, ie_list->SupRateLen);
			RTMPCheckRates(pAd, pApCliEntry->MlmeAux.SupRate, &pApCliEntry->MlmeAux.SupRateLen);
			pApCliEntry->MlmeAux.ExtRateLen = ie_list->ExtRateLen;
			NdisMoveMemory(pApCliEntry->MlmeAux.ExtRate, ie_list->ExtRate, ie_list->ExtRateLen);
			RTMPCheckRates(pAd, pApCliEntry->MlmeAux.ExtRate, &pApCliEntry->MlmeAux.ExtRateLen);
#ifdef APCLI_CERT_SUPPORT
			/*  Get the ext capability info element */
			if (pAd->bApCliCertTest == TRUE 
#ifdef DOT11N_DRAFT3				
				&& pAd->CommonCfg.bBssCoexEnable == TRUE
#endif /* DOT11N_DRAFT3 */			
				)
			{
				NdisMoveMemory(&pApCliEntry->MlmeAux.ExtCapInfo, &ie_list->ExtCapInfo,sizeof(ie_list->ExtCapInfo));
#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
				DBGPRINT(RT_DEBUG_TRACE, ("\x1b[31m ApCliMlmeAux.ExtCapInfo=%d \x1b[m\n", pApCliEntry->MlmeAux.ExtCapInfo.BssCoexistMgmtSupport)); //zero debug 210121122
					pAd->CommonCfg.ExtCapIE.BssCoexistMgmtSupport = 1;
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */
			}
#endif /* APCLI_CERT_SUPPORT */
#ifdef DOT11_N_SUPPORT
			NdisZeroMemory(pApCliEntry->RxMcsSet,sizeof(pApCliEntry->RxMcsSet));
		
#ifdef APCLI_AUTO_BW_SUPPORT
	        	PhyMode = pApCliEntry->wdev.PhyMode;
			DBGPRINT(RT_DEBUG_OFF, ("%s: check HT Rule --> %d %d %d %d\n", __FUNCTION__, (ie_list->HtCapabilityLen > 0),  
				(pApCliEntry->wdev.DesiredHtPhyInfo.bHtEnable), WMODE_CAP_N(pApCliEntry->wdev.PhyMode), 
				!(pAd->CommonCfg.HT_DisallowTKIP && IS_INVALID_HT_SECURITY(wdev->WepStatus))));	
#endif /* APCLI_AUTO_BW_SUPPORT */

			/* filter out un-supported ht rates */
			if ((ie_list->HtCapabilityLen > 0) && 
				(pApCliEntry->wdev.DesiredHtPhyInfo.bHtEnable) &&
				WMODE_CAP_N(PhyMode) &&
				/* For Dissallow TKIP rule on STA */
				!(pAd->CommonCfg.HT_DisallowTKIP && IS_INVALID_HT_SECURITY(wdev->WepStatus)))
			{
				RTMPZeroMemory(&pApCliEntry->MlmeAux.HtCapability, SIZE_HT_CAP_IE);
				pApCliEntry->MlmeAux.NewExtChannelOffset = ie_list->NewExtChannelOffset;
				pApCliEntry->MlmeAux.HtCapabilityLen = ie_list->HtCapabilityLen;
				ApCliCheckHt(pAd, ifIndex, &ie_list->HtCapability, &ie_list->AddHtInfo);

				if (ie_list->AddHtInfoLen > 0)
				{
					CentralChannel = ie_list->AddHtInfo.ControlChan;
		 			/* Check again the Bandwidth capability of this AP. */
					CentralChannel = get_cent_ch_by_htinfo(pAd, &ie_list->AddHtInfo,
														&ie_list->HtCapability);
					pApCliEntry->MlmeAux.CentralChannel = CentralChannel;									
		 			DBGPRINT(RT_DEBUG_TRACE, ("PeerBeaconAtJoinAction HT===>CentralCh = %d, ControlCh = %d\n",
									CentralChannel, ie_list->AddHtInfo.ControlChan));
				}
			}
			else
#endif /* DOT11_N_SUPPORT */
			{
				RTMPZeroMemory(&pApCliEntry->MlmeAux.HtCapability, SIZE_HT_CAP_IE);
				RTMPZeroMemory(&pApCliEntry->MlmeAux.AddHtInfo, SIZE_ADD_HT_INFO_IE);
				pApCliEntry->MlmeAux.HtCapabilityLen = 0;
			}
			ApCliUpdateMlmeRate(pAd, ifIndex);

#ifdef DOT11_N_SUPPORT
			/* copy QOS related information */
			if (WMODE_CAP_N(PhyMode))
			{
				NdisMoveMemory(&pApCliEntry->MlmeAux.APEdcaParm, &ie_list->EdcaParm, sizeof(EDCA_PARM));
				NdisMoveMemory(&pApCliEntry->MlmeAux.APQbssLoad, &ie_list->QbssLoad, sizeof(QBSS_LOAD_PARM));
				NdisMoveMemory(&pApCliEntry->MlmeAux.APQosCapability, &ie_list->QosCapability, sizeof(QOS_CAPABILITY_PARM));
			}
			else
#endif /* DOT11_N_SUPPORT */
			{
				NdisZeroMemory(&pApCliEntry->MlmeAux.APEdcaParm, sizeof(EDCA_PARM));
				NdisZeroMemory(&pApCliEntry->MlmeAux.APQbssLoad, sizeof(QBSS_LOAD_PARM));
				NdisZeroMemory(&pApCliEntry->MlmeAux.APQosCapability, sizeof(QOS_CAPABILITY_PARM));
			}

			DBGPRINT(RT_DEBUG_TRACE, ("APCLI SYNC - after JOIN, SupRateLen=%d, ExtRateLen=%d\n", 
				pApCliEntry->MlmeAux.SupRateLen, pApCliEntry->MlmeAux.ExtRateLen));

			if (ie_list->AironetCellPowerLimit != 0xFF)
			{
				/* We need to change our TxPower for CCX 2.0 AP Control of Client Transmit Power */
				ChangeToCellPowerLimit(pAd, ie_list->AironetCellPowerLimit);
			}
			else  /* Used the default TX Power Percentage. */
				pAd->CommonCfg.TxPowerPercentage = pAd->CommonCfg.TxPowerDefault;

#ifdef APCLI_AUTO_BW_SUPPORT
			if ((ie_list->HtCapabilityLen > 0) &&
			    (ie_list->HtCapability.HtCapInfo.ChannelWidth == BW_40))
			{
				ApCliAutoBwAction(pAd, ifIndex);
			}
#endif /* APCLI_AUTO_BW_SUPPORT */	
			
			if(bssidEqualFlag == TRUE)
			{
				*pCurrState = APCLI_SYNC_IDLE;

				ApCliCtrlMsg.Status = MLME_SUCCESS;
#ifdef MAC_REPEATER_SUPPORT
				ApCliCtrlMsg.BssIdx = ifIndex;
				ApCliCtrlMsg.CliIdx = 0xFF;
#endif /* MAC_REPEATER_SUPPORT */

				MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_PROBE_RSP,
					sizeof(APCLI_CTRL_MSG_STRUCT), &ApCliCtrlMsg, ifIndex);
			}

#ifdef SMART_MESH_MONITOR
			if(pFrame && (pFrame->Hdr.FC.SubType == SUBTYPE_PROBE_RSP))
			{
				struct nsmpif_drvevnt_buf drvevnt;
				
				drvevnt.data.proberesp.type = NSMPIF_DRVEVNT_AP_PROBE_RESP;
				drvevnt.data.proberesp.channel = ie_list->Channel;
				COPY_MAC_ADDR(drvevnt.data.proberesp.ap_mac, ie_list->Addr2);
				drvevnt.data.proberesp.is_ucast = 1;
				drvevnt.data.proberesp.rate = pAd->LastMgmtRxRate;
				drvevnt.data.proberesp.rssi = RealRssi;
				drvevnt.data.proberesp.snr = ConvertToSnr(pAd, Elem->Signal);
#ifdef RTMP_MAC
				if (pAd->chipCap.hif_type == HIF_RTMP)
				{
					if (IS_RT6352(pAd))
					{
						if ((42 - drvevnt.data.proberesp.snr) >= 0)
							drvevnt.data.proberesp.snr = (42 - drvevnt.data.proberesp.snr);
						else
						    drvevnt.data.proberesp.snr = 0;
					}
				}
#endif /* RTMP_MAC */

				NdisZeroMemory(drvevnt.data.proberesp.ssid,sizeof(drvevnt.data.proberesp.ssid));
				NdisCopyMemory(drvevnt.data.proberesp.ssid,ie_list->Ssid,ie_list->SsidLen);
				drvevnt.data.proberesp.cap = 0;
#ifdef DOT11_N_SUPPORT
				if(ie_list->HtCapabilityLen > 0)
					drvevnt.data.proberesp.cap |= NSMP_WLCAP_80211_N;
#endif /* DOT11_N_SUPPORT */
#ifdef DOT11_VHT_AC
				if (ie_list->vht_cap_len > 0)
					drvevnt.data.proberesp.cap |= NSMP_WLCAP_80211_AC;
#endif /* DOT11_VHT_AC */
				// Bandwdith
				if (ie_list->HtCapability.HtCapInfo.ChannelWidth == BW_40)
				{
#ifdef DOT11_VHT_AC
					if(ie_list->vht_op_len > 0 && ie_list->vht_op_ie.vht_op_info.ch_width >= 1)
						drvevnt.data.proberesp.cap |= NSMP_WLCAP_HT80;
					else
#endif /* DOT11_VHT_AC */				
						drvevnt.data.proberesp.cap |= NSMP_WLCAP_HT40;
				}
				// RX/TX STREAM
				drvevnt.data.proberesp.cap |= \
				(ie_list->HtCapability.MCSSet[3] != 0x00) ? (NSMP_WLCAP_RX_4_STREAMS|NSMP_WLCAP_TX_4_STREAMS) :\
				(ie_list->HtCapability.MCSSet[2] != 0x00) ? (NSMP_WLCAP_RX_3_STREAMS|NSMP_WLCAP_TX_3_STREAMS) :\
				(ie_list->HtCapability.MCSSet[1] != 0x00) ? (NSMP_WLCAP_RX_2_STREAMS|NSMP_WLCAP_TX_2_STREAMS) : 0;
	
				/* Vendor information element */
				drvevnt.data.proberesp.ntgr_vie_len = ie_list->vendor_ie_len;
				NdisZeroMemory(drvevnt.data.proberesp.ntgr_vie,sizeof(drvevnt.data.proberesp.ntgr_vie));
				if(ie_list->vendor_ie_len > 0)
					NdisCopyMemory(drvevnt.data.proberesp.ntgr_vie,ie_list->vendor_ie,ie_list->vendor_ie_len);
				
				RtmpOSWrielessEventSend(pAd->net_dev, RT_WLAN_EVENT_CUSTOM,NSMPIF_DRVEVNT_AP_PROBE_RESP,
										NULL, (PUCHAR)&drvevnt.data.proberesp, sizeof(drvevnt.data.proberesp));
			}
#endif /* SMART_MESH_MONITOR */
		}
	}

LabelErr:
	if (VarIE != NULL)
		os_free_mem(NULL, VarIE);
	if (ie_list != NULL)
		os_free_mem(NULL, ie_list);

	return;
}

static VOID ApCliProbeTimeoutAtJoinAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem) 
{
	APCLI_CTRL_MSG_STRUCT ApCliCtrlMsg;
	USHORT ifIndex = (USHORT)(Elem->Priv);
	PULONG pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].SyncCurrState;
	APCLI_STRUCT *pApCliEntry = NULL;

	DBGPRINT(RT_DEBUG_TRACE, ("APCLI_SYNC - ProbeTimeoutAtJoinAction\n"));

	if (ifIndex >= MAX_APCLI_NUM)
		return;

	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];
	*pCurrState = SYNC_IDLE;

#ifdef APCLI_CONNECTION_TRIAL
	if (ifIndex == 1)
		*pCurrState = APCLI_CTRL_DISCONNECTED;
#endif /* APCLI_CONNECTION_TRIAL */

	DBGPRINT(RT_DEBUG_TRACE, ("APCLI_SYNC - MlmeAux.Bssid=%02x:%02x:%02x:%02x:%02x:%02x\n",
								PRINT_MAC(pApCliEntry->MlmeAux.Bssid)));

	if(!MAC_ADDR_EQUAL(pApCliEntry->MlmeAux.Bssid, ZERO_MAC_ADDR))
	{
		ApCliCtrlMsg.Status = MLME_SUCCESS;
#ifdef MAC_REPEATER_SUPPORT
		ApCliCtrlMsg.BssIdx = ifIndex;
		ApCliCtrlMsg.CliIdx = 0xFF;
#endif /* MAC_REPEATER_SUPPORT */

		MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_PROBE_RSP,
			sizeof(APCLI_CTRL_MSG_STRUCT), &ApCliCtrlMsg, ifIndex);
	} else
	{
		MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_JOIN_REQ_TIMEOUT, 0, NULL, ifIndex);
	}

	return;
}

/* 
    ==========================================================================
    Description:
    ==========================================================================
 */
static VOID ApCliInvalidStateWhenJoin(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem) 
{
	APCLI_CTRL_MSG_STRUCT ApCliCtrlMsg;
	USHORT ifIndex = (USHORT)(Elem->Priv);
	PULONG pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].SyncCurrState;

	*pCurrState = APCLI_SYNC_IDLE;
	ApCliCtrlMsg.Status = MLME_STATE_MACHINE_REJECT;
#ifdef MAC_REPEATER_SUPPORT
	ApCliCtrlMsg.BssIdx = ifIndex;
	ApCliCtrlMsg.CliIdx = 0xFF;
#endif /* MAC_REPEATER_SUPPORT */

	MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_PROBE_RSP,
		sizeof(APCLI_CTRL_MSG_STRUCT), &ApCliCtrlMsg, ifIndex);

	DBGPRINT(RT_DEBUG_TRACE, ("APCLI_AYNC - ApCliInvalidStateWhenJoin(state=%ld). Reset SYNC machine\n", *pCurrState));

	return;
}

/* 
	==========================================================================
	Description:
	==========================================================================
 */
static VOID ApCliEnqueueProbeRequest(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR SsidLen,
	IN PCHAR Ssid,
	IN PCHAR Bssid,
	IN USHORT ifIndex)
{
	NDIS_STATUS     NState;
	PUCHAR          pOutBuffer;
	ULONG           FrameLen = 0;
	HEADER_802_11   Hdr80211;
	UCHAR           SsidIe    = IE_SSID;
	UCHAR           SupRateIe = IE_SUPP_RATES;
	UCHAR ssidLen;
	CHAR ssid[MAX_LEN_OF_SSID];
	APCLI_STRUCT *pApCliEntry = NULL;
	BOOLEAN bHasWscIe = FALSE;
	UCHAR   PhyMode = pAd->CommonCfg.PhyMode;
	
	DBGPRINT(RT_DEBUG_TRACE, ("force out a ProbeRequest ...\n"));

	if (ifIndex >= MAX_APCLI_NUM)
		return;

	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];
#ifdef APCLI_AUTO_BW_SUPPORT
    PhyMode = pApCliEntry->wdev.PhyMode;
#endif /* APCLI_AUTO_BW_SUPPORT */	
	
	NState = MlmeAllocateMemory(pAd, &pOutBuffer);  /* Get an unused nonpaged memory */
	if(NState != NDIS_STATUS_SUCCESS)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("EnqueueProbeRequest() allocate memory fail\n"));
		return;
	}
	else
	{
		if(MAC_ADDR_EQUAL(Bssid, ZERO_MAC_ADDR))
			ApCliMgtMacHeaderInit(pAd, &Hdr80211, SUBTYPE_PROBE_REQ, 0,
				BROADCAST_ADDR, BROADCAST_ADDR, ifIndex);
		else
			ApCliMgtMacHeaderInit(pAd, &Hdr80211, SUBTYPE_PROBE_REQ, 0,
				Bssid, Bssid, ifIndex);

		ssidLen = SsidLen;
		NdisZeroMemory(ssid, MAX_LEN_OF_SSID);
		NdisMoveMemory(ssid, Ssid, ssidLen);

		/* this ProbeRequest explicitly specify SSID to reduce unwanted ProbeResponse */
		MakeOutgoingFrame(pOutBuffer,		&FrameLen,
			sizeof(HEADER_802_11),			&Hdr80211,
			1,								&SsidIe,
			1,								&ssidLen,
			ssidLen,						ssid,
			1,								&SupRateIe,
			1,								&pApCliEntry->MlmeAux.SupRateLen,
			pApCliEntry->MlmeAux.SupRateLen,		pApCliEntry->MlmeAux.SupRate,
			END_OF_ARGS);

		/* Add the extended rate IE */
		if (pApCliEntry->MlmeAux.ExtRateLen != 0)
		{
			ULONG            tmp;
		
			MakeOutgoingFrame(pOutBuffer + FrameLen,    &tmp,
				1,                        &ExtRateIe,
				1,                        &pApCliEntry->MlmeAux.ExtRateLen,
				pApCliEntry->MlmeAux.ExtRateLen,  pApCliEntry->MlmeAux.ExtRate,
				END_OF_ARGS);
			FrameLen += tmp;
		}

#ifdef SMART_MESH
		SMART_MESH_INSERT_IE(pApCliEntry->SmartMeshCfg,
							pOutBuffer,
							FrameLen,
							SM_IE_PROBE_REQ);
#endif /* SMART_MESH */

#ifdef DOT11_VHT_AC
		if (WMODE_CAP_AC(PhyMode) &&
			(pAd->CommonCfg.Channel > 14))
		{

			build_vht_cap_ie(pAd, (UCHAR *)&pApCliEntry->MlmeAux.vht_cap);
			pApCliEntry->MlmeAux.vht_cap_len = sizeof(VHT_CAP_IE);

            FrameLen += build_vht_ies(pAd, (UCHAR *)(pOutBuffer + FrameLen), SUBTYPE_PROBE_REQ);
		}
#endif /* DOT11_VHT_AC */
#ifdef WSC_AP_SUPPORT
/* Append WSC information in probe request if WSC state is running */
		if ((pAd->ApCfg.ApCliTab[ifIndex].WscControl.WscConfMode != WSC_DISABLE) &&
			(pAd->ApCfg.ApCliTab[ifIndex].WscControl.bWscTrigger))
		{
			bHasWscIe = TRUE;
		}
#ifdef WSC_V2_SUPPORT
		else if (pAd->ApCfg.ApCliTab[ifIndex].WscControl.WscV2Info.bEnableWpsV2)
		{
			bHasWscIe = TRUE;	
		}
#endif /* WSC_V2_SUPPORT */

#ifdef WSC_AP_SUPPORT
#ifdef SMART_MESH_HIDDEN_WPS
        if(pAd->ApCfg.ApCliTab[ifIndex].SmartMeshCfg.bSupportHiddenWPS)
            bHasWscIe = FALSE;
#endif /* SMART_MESH_HIDDEN_WPS */
#endif /* WSC_AP_SUPPORT */

		if (bHasWscIe)
		{
			UCHAR		/* WscBuf[256], */ WscIeLen = 0;
			UCHAR		*WscBuf = NULL;
			ULONG 		WscTmpLen = 0;
			/* allocate memory */
			os_alloc_mem(NULL, (UCHAR **)&WscBuf, 512);
			if (WscBuf != NULL)
			{
				NdisZeroMemory(WscBuf, 512);
				WscBuildProbeReqIE(pAd, STA_MODE, WscBuf, &WscIeLen);
				MakeOutgoingFrame(pOutBuffer + FrameLen,              &WscTmpLen,
								WscIeLen,                             WscBuf,
								END_OF_ARGS);
				FrameLen += WscTmpLen;
				os_free_mem(NULL, WscBuf);
			}
			else
				DBGPRINT(RT_DEBUG_ERROR, ("%s: Allocate memory fail!!!\n", __FUNCTION__));
		}
#endif /*WSC_AP_SUPPORT*/
#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE
		if ((pAd->StaCfg.wpa_supplicant_info.WpaSupplicantUP != WPA_SUPPLICANT_DISABLE) &&
			(pAd->cfg80211_ctrl.ExtraIeLen != 0))
		{
				ULONG ExtraIeTmpLen = 0;
		
				MakeOutgoingFrame(pOutBuffer + FrameLen,			  &ExtraIeTmpLen,
								  pAd->cfg80211_ctrl.ExtraIeLen,   pAd->cfg80211_ctrl.pExtraIe,
								  END_OF_ARGS);
		
				FrameLen += ExtraIeTmpLen;
		}
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE*/		

		MiniportMMRequest(pAd, QID_AC_BE, pOutBuffer, FrameLen);
		MlmeFreeMemory(pAd, pOutBuffer);
	}

	return;
}

VOID ApCliSiteSurvey(
	IN	RTMP_ADAPTER  		*pAd,
	IN	UCHAR			ifIndex,
	IN	NDIS_802_11_SSID	*pSsid,
	IN	UCHAR				ScanType,
	IN	BOOLEAN				ChannelSel)
{
	MLME_SCAN_REQ_STRUCT    ScanReq;

	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))
	{
		/*	
		* Still scanning, ignore this scan.
		*/
		DBGPRINT(RT_DEBUG_TRACE, ("ApCliSiteSurvey:: scanning now\n"));
		return;
	}

	if(ifIndex >= MAX_APCLI_NUM)
		return;

#ifdef WSC_AP_SUPPORT
    if ((pAd->ApCfg.ApCliTab[ifIndex].WscControl.WscConfMode != WSC_DISABLE) &&
    	(pAd->ApCfg.ApCliTab[ifIndex].WscControl.bWscTrigger == TRUE))
    {
       DBGPRINT(RT_DEBUG_TRACE, ("%s(): Stop scanning due to WPS triggered!\n",__FUNCTION__));
       return;
    }
#endif /* WSC_AP_SUPPORT */

	AsicDisableSync(pAd);

#ifdef AP_PARTIAL_SCAN_SUPPORT
	if (((pAd->ApCfg.bPartialScanning == TRUE) && (pAd->ApCfg.LastPartialScanChannel == 0)) ||
		(pAd->ApCfg.bPartialScanning == FALSE))
#endif /* AP_PARTIAL_SCAN_SUPPORT */
	{
		BssTableInit(&pAd->ScanTab);
	}
	pAd->Mlme.ApSyncMachine.CurrState = AP_SYNC_IDLE;
	pAd->ApCfg.ApCliTab[ifIndex].SyncCurrState = APCLI_SYNC_IDLE;

	RTMPZeroMemory(ScanReq.Ssid, MAX_LEN_OF_SSID);
	ScanReq.SsidLen = 0;
	if (pSsid)
	{
	    ScanReq.SsidLen = pSsid->SsidLength;
	    NdisMoveMemory(ScanReq.Ssid, pSsid->Ssid, pSsid->SsidLength);
	}
    ScanReq.BssType = BSS_ANY;
    ScanReq.ScanType = ScanType;
    pAd->ApCfg.bAutoChannelAtBootup = ChannelSel;
    
    MlmeEnqueue(pAd, APCLI_SYNC_STATE_MACHINE, APCLI_MT2_MLME_SCAN_REQ, sizeof(MLME_SCAN_REQ_STRUCT), &ScanReq, 0);
    RTMP_MLME_HANDLER(pAd);
}

static VOID ApCliMlmeScanReqAction(
	IN RTMP_ADAPTER *pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	BOOLEAN Cancelled;
	UCHAR Ssid[MAX_LEN_OF_SSID], SsidLen, ScanType, BssType;

	/* Suspend MSDU transmission here */
	RTMPSuspendMsduTransmission(pAd);

	/* first check the parameter sanity */
	if (MlmeScanReqSanity(pAd, Elem->Msg, Elem->MsgLen, &BssType, (PCHAR)Ssid, &SsidLen, &ScanType))
	{
		DBGPRINT(RT_DEBUG_TRACE, ("%s(): mlme scan req action!\n", __FUNCTION__));
		NdisGetSystemUpTime(&pAd->ApCfg.LastScanTime);

		RTMPCancelTimer(&pAd->MlmeAux.APScanTimer, &Cancelled);

		/* record desired BSS parameters */
		pAd->MlmeAux.BssType = BssType;
		pAd->MlmeAux.ScanType = ScanType;
		pAd->MlmeAux.SsidLen = SsidLen;
		NdisMoveMemory(pAd->MlmeAux.Ssid, Ssid, SsidLen);

		/* start from the first channel */
#ifdef AP_PARTIAL_SCAN_SUPPORT
		pAd->MlmeAux.Channel = RTMPFindScanChannel(pAd, 0);
#else
		pAd->MlmeAux.Channel = FirstChannel(pAd);
#endif /* AP_PARTIAL_SCAN_SUPPORT */

		/* Let BBP register at 20MHz to do scan */
		bbp_set_bw(pAd, BW_20);
		DBGPRINT(RT_DEBUG_TRACE, ("SYNC - BBP R4 to 20MHz.l\n"));

#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			if (pAd->ApCfg.bAutoChannelAtBootup == TRUE)/* iwpriv set auto channel selection */
			{
				APAutoChannelInit(pAd);	
				pAd->ApCfg.AutoChannel_Channel = pAd->ChannelList[0].Channel;
			}
#ifdef SMART_MESH
			Set_Scan_False_CCA(pAd, 0, CCA_RESET);
#endif /* SMART_MESH */		
		}
#endif /* CONFIG_AP_SUPPORT */
		ScanNextChannel(pAd, OPMODE_AP, INT_APCLI);
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s(): sanity check fail. BUG!!!!\n", __FUNCTION__));
		pAd->Mlme.ApSyncMachine.CurrState = AP_SYNC_IDLE;
	}
}

#ifdef SMART_MESH_MONITOR
static VOID ApCliPeerProbeRspAtScanReqAction(
	IN RTMP_ADAPTER *pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	USHORT LenVIE;
	UCHAR *VarIE = NULL;
	PFRAME_802_11 pFrame = NULL;
	PAPCLI_STRUCT pApCliEntry = NULL;
	NDIS_802_11_VARIABLE_IEs *pVIE = NULL;
	USHORT ifIndex = (USHORT)(Elem->Priv);
	BCN_IE_LIST *ie_list = NULL;
	
	if (ifIndex >= MAX_APCLI_NUM)
		return;

	/* Init Variable IE structure */
	os_alloc_mem(NULL, (UCHAR **)&VarIE, MAX_VIE_LEN);
	if (VarIE == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s: Allocate memory fail!!!\n", __FUNCTION__));
		goto lblErr;
	}
	pVIE = (PNDIS_802_11_VARIABLE_IEs) VarIE;
	pVIE->Length = 0;
	
	os_alloc_mem(NULL, (UCHAR **)&ie_list, sizeof(BCN_IE_LIST));
	if (ie_list == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s: Allocate ie_list fail!!!\n", __FUNCTION__));
		goto lblErr;
	}
	NdisZeroMemory(ie_list, sizeof(BCN_IE_LIST));
	pFrame = (PFRAME_802_11)Elem->Msg;
	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];

	if (PeerBeaconAndProbeRspSanity(pAd, 
								Elem->Msg, 
								Elem->MsgLen, 
								Elem->Channel,
								ie_list,
								&LenVIE,
								pVIE))
	{
		CHAR RealRssi = -127;

		RealRssi = (LONG)(RTMPMaxRssi(pAd, ConvertToRssi(pAd, Elem->Rssi0, RSSI_0), 
						   ConvertToRssi(pAd, Elem->Rssi1, RSSI_1), 
						   ConvertToRssi(pAd, Elem->Rssi2, RSSI_2)));
		if(pFrame && (pFrame->Hdr.FC.SubType == SUBTYPE_PROBE_RSP))
		{
			struct nsmpif_drvevnt_buf drvevnt;
			
			drvevnt.data.proberesp.type = NSMPIF_DRVEVNT_AP_PROBE_RESP;
			drvevnt.data.proberesp.channel = ie_list->Channel;
			COPY_MAC_ADDR(drvevnt.data.proberesp.ap_mac, ie_list->Addr2);
			drvevnt.data.proberesp.is_ucast = 1;
			drvevnt.data.proberesp.rate = pAd->LastMgmtRxRate;
			drvevnt.data.proberesp.rssi = RealRssi;
			drvevnt.data.proberesp.snr = ConvertToSnr(pAd, Elem->Signal);
#ifdef RTMP_MAC
			if (pAd->chipCap.hif_type == HIF_RTMP)
			{
				if (IS_RT6352(pAd))
				{
					if ((42 - drvevnt.data.proberesp.snr) >= 0)
						drvevnt.data.proberesp.snr = (42 - drvevnt.data.proberesp.snr);
					else
						drvevnt.data.proberesp.snr = 0;
				}
			}
#endif /* RTMP_MAC */
			NdisZeroMemory(drvevnt.data.proberesp.ssid,sizeof(drvevnt.data.proberesp.ssid));
			NdisCopyMemory(drvevnt.data.proberesp.ssid,ie_list->Ssid,ie_list->SsidLen);
			drvevnt.data.proberesp.cap = 0;
#ifdef DOT11_N_SUPPORT
			if(ie_list->HtCapabilityLen > 0)
				drvevnt.data.proberesp.cap |= NSMP_WLCAP_80211_N;
#endif /* DOT11_N_SUPPORT */
#ifdef DOT11_VHT_AC
			if (ie_list->vht_cap_len > 0)
				drvevnt.data.proberesp.cap |= NSMP_WLCAP_80211_AC;
#endif /* DOT11_VHT_AC */
			// Bandwdith
			if (ie_list->HtCapability.HtCapInfo.ChannelWidth == BW_40)
			{
#ifdef DOT11_VHT_AC
				if(ie_list->vht_op_len > 0 && ie_list->vht_op_ie.vht_op_info.ch_width >= 1)
					drvevnt.data.proberesp.cap |= NSMP_WLCAP_HT80;
				else
#endif /* DOT11_VHT_AC */				
					drvevnt.data.proberesp.cap |= NSMP_WLCAP_HT40;
			}
			// RX/TX STREAM
			drvevnt.data.proberesp.cap |= \
			(ie_list->HtCapability.MCSSet[3] != 0x00) ? (NSMP_WLCAP_RX_4_STREAMS|NSMP_WLCAP_TX_4_STREAMS) :\
			(ie_list->HtCapability.MCSSet[2] != 0x00) ? (NSMP_WLCAP_RX_3_STREAMS|NSMP_WLCAP_TX_3_STREAMS) :\
			(ie_list->HtCapability.MCSSet[1] != 0x00) ? (NSMP_WLCAP_RX_2_STREAMS|NSMP_WLCAP_TX_2_STREAMS) : 0;

			/* Vendor information element */
			drvevnt.data.proberesp.ntgr_vie_len = ie_list->vendor_ie_len;
			NdisZeroMemory(drvevnt.data.proberesp.ntgr_vie,sizeof(drvevnt.data.proberesp.ntgr_vie));
			if(ie_list->vendor_ie_len > 0)
				NdisCopyMemory(drvevnt.data.proberesp.ntgr_vie,ie_list->vendor_ie,ie_list->vendor_ie_len);

			RtmpOSWrielessEventSend(pAd->net_dev, RT_WLAN_EVENT_CUSTOM,NSMPIF_DRVEVNT_AP_PROBE_RESP,
									NULL, (PUCHAR)&drvevnt.data.proberesp, sizeof(drvevnt.data.proberesp));
		}
	}

lblErr:
	if (VarIE != NULL)
		os_free_mem(NULL, VarIE);
	if (ie_list != NULL)
		os_free_mem(NULL, ie_list);

	return;
}
#endif /* SMART_MESH_MONITOR */
#endif /* APCLI_SUPPORT */


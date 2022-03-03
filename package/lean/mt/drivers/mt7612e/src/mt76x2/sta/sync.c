/*
 ***************************************************************************
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
 ***************************************************************************

	Module Name:
	sync.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	John Chang	2004-09-01      modified for rt2561/2661
	Jan Lee		2006-08-01      modified for rt2860 for 802.11n
*/
#include "rt_config.h"

#ifdef DOT11R_FT_SUPPORT
#include "ft.h"
#endif /* DOT11R_FT_SUPPORT */

#define ADHOC_ENTRY_BEACON_LOST_TIME	(2*OS_HZ)	/* 2 sec */

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
VOID SyncStateMachineInit(
	IN PRTMP_ADAPTER pAd, 
	IN STATE_MACHINE *Sm, 
	OUT STATE_MACHINE_FUNC Trans[]) 
{
	StateMachineInit(Sm, Trans, MAX_SYNC_STATE, MAX_SYNC_MSG, (STATE_MACHINE_FUNC)Drop, SYNC_IDLE, SYNC_MACHINE_BASE);

	/* column 1 */
	StateMachineSetAction(Sm, SYNC_IDLE, MT2_MLME_SCAN_REQ, (STATE_MACHINE_FUNC)MlmeScanReqAction);
	StateMachineSetAction(Sm, SYNC_IDLE, MT2_MLME_FORCE_SCAN_REQ, (STATE_MACHINE_FUNC)MlmeForceScanReqAction);
	StateMachineSetAction(Sm, SYNC_IDLE, MT2_MLME_JOIN_REQ, (STATE_MACHINE_FUNC)MlmeJoinReqAction);
	StateMachineSetAction(Sm, SYNC_IDLE, MT2_MLME_FORCE_JOIN_REQ, (STATE_MACHINE_FUNC)MlmeForceJoinReqAction);
	StateMachineSetAction(Sm, SYNC_IDLE, MT2_MLME_START_REQ, (STATE_MACHINE_FUNC)MlmeStartReqAction);
	StateMachineSetAction(Sm, SYNC_IDLE, MT2_PEER_BEACON, (STATE_MACHINE_FUNC)PeerBeacon);
	StateMachineSetAction(Sm, SYNC_IDLE, MT2_PEER_PROBE_REQ, (STATE_MACHINE_FUNC)PeerProbeReqAction); 

	/* column 2 */
	StateMachineSetAction(Sm, JOIN_WAIT_BEACON, MT2_MLME_JOIN_REQ, (STATE_MACHINE_FUNC)MlmeJoinReqAction);
	StateMachineSetAction(Sm, JOIN_WAIT_BEACON, MT2_MLME_START_REQ, (STATE_MACHINE_FUNC)InvalidStateWhenStart);
	StateMachineSetAction(Sm, JOIN_WAIT_BEACON, MT2_PEER_BEACON, (STATE_MACHINE_FUNC)PeerBeaconAtJoinAction);
	StateMachineSetAction(Sm, JOIN_WAIT_BEACON, MT2_BEACON_TIMEOUT, (STATE_MACHINE_FUNC)BeaconTimeoutAtJoinAction);
	StateMachineSetAction(Sm, JOIN_WAIT_BEACON, MT2_PEER_PROBE_RSP, (STATE_MACHINE_FUNC)PeerBeaconAtScanAction);

	/* column 3 */
	StateMachineSetAction(Sm, SCAN_LISTEN, MT2_MLME_JOIN_REQ, (STATE_MACHINE_FUNC)MlmeJoinReqAction);
	StateMachineSetAction(Sm, SCAN_LISTEN, MT2_MLME_START_REQ, (STATE_MACHINE_FUNC)InvalidStateWhenStart);
	StateMachineSetAction(Sm, SCAN_LISTEN, MT2_PEER_BEACON, (STATE_MACHINE_FUNC)PeerBeaconAtScanAction);
	StateMachineSetAction(Sm, SCAN_LISTEN, MT2_PEER_PROBE_RSP, (STATE_MACHINE_FUNC)PeerBeaconAtScanAction);
	StateMachineSetAction(Sm, SCAN_LISTEN, MT2_SCAN_TIMEOUT, (STATE_MACHINE_FUNC)ScanTimeoutAction);
	/* StateMachineSetAction(Sm, SCAN_LISTEN, MT2_MLME_SCAN_CNCL, (STATE_MACHINE_FUNC)ScanCnclAction); */

	/* resume scanning for fast-roaming */
	StateMachineSetAction(Sm, SCAN_PENDING, MT2_MLME_SCAN_REQ, (STATE_MACHINE_FUNC)MlmeScanReqAction);
       StateMachineSetAction(Sm, SCAN_PENDING, MT2_PEER_BEACON, (STATE_MACHINE_FUNC)PeerBeacon);

	/* timer init */
	RTMPInitTimer(pAd, &pAd->MlmeAux.BeaconTimer, GET_TIMER_FUNCTION(BeaconTimeout), pAd, FALSE);
	RTMPInitTimer(pAd, &pAd->MlmeAux.ScanTimer, GET_TIMER_FUNCTION(ScanTimeout), pAd, FALSE);
}

/* 
	==========================================================================
	Description:
		Beacon timeout handler, executed in timer thread

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
VOID BeaconTimeout(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3) 
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)FunctionContext;

	DBGPRINT(RT_DEBUG_TRACE,("SYNC - BeaconTimeout\n"));
	
	/*
	    Do nothing if the driver is starting halt state.
	    This might happen when timer already been fired before cancel timer with mlmehalt
	*/
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS))
		return;
	
#ifdef DOT11_N_SUPPORT
	if ((pAd->CommonCfg.BBPCurrentBW == BW_40)
	)
	{
		bbp_set_bw(pAd, BW_40);

		AsicSwitchChannel(pAd, pAd->CommonCfg.CentralChannel, FALSE);
		AsicLockChannel(pAd, pAd->CommonCfg.CentralChannel);
		DBGPRINT(RT_DEBUG_TRACE, ("SYNC - End of SCAN, restore to 40MHz channel %d, Total BSS[%02d]\n",
									pAd->CommonCfg.CentralChannel, pAd->ScanTab.BssNr));
	}
#endif /* DOT11_N_SUPPORT */

	MlmeEnqueue(pAd, SYNC_STATE_MACHINE, MT2_BEACON_TIMEOUT, 0, NULL, 0);
	RTMP_MLME_HANDLER(pAd);
}

/* 
	==========================================================================
	Description:
		Scan timeout handler, executed in timer thread

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
VOID ScanTimeout(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3) 
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)FunctionContext;

	
	/* 
	    Do nothing if the driver is starting halt state.
	    This might happen when timer already been fired before cancel timer with mlmehalt
	*/
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS))
		return;
	
	if (MlmeEnqueue(pAd, SYNC_STATE_MACHINE, MT2_SCAN_TIMEOUT, 0, NULL, 0))
	{
	RTMP_MLME_HANDLER(pAd);
}
	else
	{
		/* To prevent SyncMachine.CurrState is SCAN_LISTEN forever. */
		pAd->MlmeAux.Channel = 0;
		ScanNextChannel(pAd, OPMODE_STA, pAd->MlmeAux.ScanInfType);
			RTMPSendWirelessEvent(pAd, IW_SCAN_ENQUEUE_FAIL_EVENT_FLAG, NULL, BSS0, 0); 
	}
}


VOID MlmeForceJoinReqAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem) 
{
	BOOLEAN        TimerCancelled;
	HEADER_802_11 Hdr80211;	
	NDIS_STATUS   NStatus;
	ULONG         FrameLen = 0;	
	PUCHAR        pOutBuffer = NULL;
	PUCHAR        pSupRate = NULL;
	UCHAR         SupRateLen;
	PUCHAR        pExtRate = NULL;
	UCHAR         ExtRateLen;	
	UCHAR         ASupRate[] = {0x8C, 0x12, 0x98, 0x24, 0xb0, 0x48, 0x60, 0x6C};
	UCHAR         ASupRateLen = sizeof(ASupRate)/sizeof(UCHAR);	
	MLME_JOIN_REQ_STRUCT *pInfo = (MLME_JOIN_REQ_STRUCT *)(Elem->Msg);

#ifdef CONFIG_PM
#ifdef USB_SUPPORT_SELECTIVE_SUSPEND
	POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
#endif /* USB_SUPPORT_SELECTIVE_SUSPEND */
#endif /* CONFIG_PM */


	DBGPRINT(RT_DEBUG_TRACE, ("SYNC - MlmeForeJoinReqAction(BSS #%ld)\n", pInfo->BssIdx));

#ifdef CONFIG_PM
#ifdef USB_SUPPORT_SELECTIVE_SUSPEND

	if ( (RTMP_Usb_AutoPM_Get_Interface(pObj->pUsb_Dev,pObj->intf)) == 1)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("MlmeJoinReqAction: autopm_resume success\n"));
		RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_SUSPEND);
	}
	else if ((RTMP_Usb_AutoPM_Get_Interface(pObj->pUsb_Dev,pObj->intf)) == (-1))
	{
		DBGPRINT(RT_DEBUG_ERROR, ("MlmeJoinReqAction autopm_resume fail ------\n"));
		RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_SUSPEND);
		return;
	}
	else
		DBGPRINT(RT_DEBUG_TRACE, ("MlmeJoinReqAction: autopm_resume do nothing \n"));


#endif /* USB_SUPPORT_SELECTIVE_SUSPEND */
#endif /* CONFIG_PM */

#ifdef PCIE_PS_SUPPORT
    if ((OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_ADVANCE_POWER_SAVE_PCIE_DEVICE)) && 
        (IDLE_ON(pAd)) &&
		(pAd->StaCfg.bRadio == TRUE) &&
		(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF)))
	{
			RT28xxPciAsicRadioOn(pAd, GUI_IDLE_POWER_SAVE);
	}
#endif /* PCIE_PS_SUPPORT */

	/* reset all the timers */
	RTMPCancelTimer(&pAd->MlmeAux.ScanTimer, &TimerCancelled);
	RTMPCancelTimer(&pAd->MlmeAux.BeaconTimer, &TimerCancelled);

	{
		RTMPZeroMemory(pAd->MlmeAux.Ssid, MAX_LEN_OF_SSID);
		NdisMoveMemory(pAd->MlmeAux.Ssid, pAd->StaCfg.ConnectinfoSsid, pAd->StaCfg.ConnectinfoSsidLen);	
		pAd->MlmeAux.SsidLen = pAd->StaCfg.ConnectinfoSsidLen;
	}

	pAd->MlmeAux.BssType = pAd->StaCfg.ConnectinfoBssType;
	pAd->MlmeAux.Channel = pAd->StaCfg.ConnectinfoChannel;


	/* Let BBP register at 20MHz to do scan */
	AsicSetChannel(pAd, pAd->MlmeAux.Channel, BW_20, EXTCHA_NONE, FALSE);
	DBGPRINT(RT_DEBUG_TRACE, ("SYNC - BBP R4 to 20MHz.l\n"));

	RTMPSetTimer(&pAd->MlmeAux.BeaconTimer, JOIN_TIMEOUT);

    do
	{
		/*
	    send probe request
	*/
	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);
	if (NStatus == NDIS_STATUS_SUCCESS)
	{
		if (pAd->MlmeAux.Channel <= 14)
		{
			pSupRate = pAd->CommonCfg.SupRate;
			SupRateLen = pAd->CommonCfg.SupRateLen;
			pExtRate = pAd->CommonCfg.ExtRate;
			ExtRateLen = pAd->CommonCfg.ExtRateLen;
		}
		else
		{
			/*
		           Overwrite Support Rate, CCK rate are not allowed
		*/
			pSupRate = ASupRate;
			SupRateLen = ASupRateLen;
			ExtRateLen = 0;
		}

		if ((pAd->MlmeAux.BssType == BSS_INFRA)  && (!MAC_ADDR_EQUAL(ZERO_MAC_ADDR, pAd->StaCfg.ConnectinfoBssid)))
		{
			COPY_MAC_ADDR(pAd->MlmeAux.Bssid, pAd->StaCfg.ConnectinfoBssid);
			MgtMacHeaderInit(pAd, &Hdr80211, SUBTYPE_PROBE_REQ, 0, pAd->MlmeAux.Bssid,
                                                                pAd->CurrentAddress,
								pAd->MlmeAux.Bssid);
		}
		else
			MgtMacHeaderInit(pAd, &Hdr80211, SUBTYPE_PROBE_REQ, 0, BROADCAST_ADDR,
                                                                pAd->CurrentAddress,
								BROADCAST_ADDR);

		MakeOutgoingFrame(pOutBuffer,               &FrameLen,
						  sizeof(HEADER_802_11),    &Hdr80211,
						  1,                        &SsidIe,
						  1,                        &pAd->MlmeAux.SsidLen,
						  pAd->MlmeAux.SsidLen,	    pAd->MlmeAux.Ssid,
						  1,                        &SupRateIe,
						  1,                        &SupRateLen,
						  SupRateLen,               pSupRate,
						  END_OF_ARGS);

		if (ExtRateLen)
		{
			ULONG Tmp;
			MakeOutgoingFrame(pOutBuffer + FrameLen,            &Tmp,
							  1,                                &ExtRateIe,
							  1,                                &ExtRateLen,
							  ExtRateLen,                       pExtRate, 
							  END_OF_ARGS);
			FrameLen += Tmp;
	} 
		
#ifdef WPA_SUPPLICANT_SUPPORT
		if ((pAd->OpMode == OPMODE_STA) &&
			(pAd->StaCfg.wpa_supplicant_info.WpaSupplicantUP != WPA_SUPPLICANT_DISABLE) &&
			(pAd->StaCfg.wpa_supplicant_info.WpsProbeReqIeLen != 0))
	{
			ULONG 		WpsTmpLen = 0;
			
			MakeOutgoingFrame(pOutBuffer + FrameLen,              &WpsTmpLen,
							pAd->StaCfg.wpa_supplicant_info.WpsProbeReqIeLen,
							pAd->StaCfg.wpa_supplicant_info.pWpsProbeReqIe,
							END_OF_ARGS);

			FrameLen += WpsTmpLen;
		}

#ifdef RT_CFG80211_SUPPORT
        	if ((pAd->OpMode == OPMODE_STA) &&
                    (pAd->StaCfg.wpa_supplicant_info.WpaSupplicantUP != WPA_SUPPLICANT_DISABLE) &&
                    (pAd->cfg80211_ctrl.ExtraIeLen != 0))
        	{
                	ULONG ExtraIeTmpLen = 0;

                	MakeOutgoingFrame(pOutBuffer + FrameLen,              &ExtraIeTmpLen,
                                          pAd->cfg80211_ctrl.ExtraIeLen,   pAd->cfg80211_ctrl.pExtraIe,
                                          END_OF_ARGS);

                	FrameLen += ExtraIeTmpLen;
        	}
#endif /* RT_CFG80211_SUPPORT */

#endif /* WPA_SUPPLICANT_SUPPORT */
#ifdef WFD_SUPPORT
#ifdef RT_CFG80211_SUPPORT
		if (pAd->StaCfg.WfdCfg.bSuppInsertWfdIe)
		{
			ULONG	WfdIeLen, WfdIeBitmap;
			PUCHAR	ptr;
			
			ptr = pOutBuffer + FrameLen;
			WfdIeBitmap = (0x1 << SUBID_WFD_DEVICE_INFO) | (0x1 << SUBID_WFD_ASSOCIATED_BSSID) |
				(0x1 << SUBID_WFD_COUPLED_SINK_INFO);
			WfdMakeWfdIE(pAd, WfdIeBitmap, ptr, &WfdIeLen);
			FrameLen += WfdIeLen;
		}
#endif /* RT_CFG80211_SUPPORT */
#endif /* WFD_SUPPORT */

		MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
		MlmeFreeMemory(pAd, pOutBuffer);
	}
    } while (FALSE);

	DBGPRINT(0, ("FORCE JOIN SYNC - Switch to ch %d, Wait BEACON from %02x:%02x:%02x:%02x:%02x:%02x\n", 
		pAd->StaCfg.ConnectinfoChannel, PRINT_MAC(pAd->StaCfg.ConnectinfoBssid)));

	pAd->Mlme.SyncMachine.CurrState = JOIN_WAIT_BEACON;
}


VOID MlmeForceScanReqAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem)
{
	UCHAR          Ssid[MAX_LEN_OF_SSID], SsidLen, ScanType, BssType;
	BOOLEAN        TimerCancelled;
	ULONG		   Now;
	USHORT         Status;

       /*
	    Check the total scan tries for one single OID command
	    If this is the CCX 2.0 Case, skip that!
	*/
	if ( !RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP))
	{
		DBGPRINT(RT_DEBUG_TRACE, ("SYNC - MlmeForceScanReqAction before Startup\n"));
		return;
	}


	/* first check the parameter sanity */
	if (MlmeScanReqSanity(pAd, 
						  Elem->Msg, 
						  Elem->MsgLen, 
						  &BssType, 
						  (PCHAR)Ssid, 
						  &SsidLen, 
						  &ScanType)) 
	{
#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE                                    
                if (pAd->ApCfg.ApCliTab[MAIN_MBSSID].Valid && RTMP_CFG80211_VIF_P2P_CLI_ON(pAd))
                {
                        DBGPRINT(RT_DEBUG_TRACE, ("CFG80211_NULL: PWR_SAVE IN ForceScanStart\n"));
                        RT_CFG80211_P2P_CLI_SEND_NULL_FRAME(pAd, PWR_SAVE);
                }
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE */
		
		/*
		    To prevent data lost.	
		    Send an NULL data with turned PSM bit on to current associated AP before SCAN progress.
		    And should send an NULL data with turned PSM bit off to AP, when scan progress done 
		*/
		if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED) && (INFRA_ON(pAd)))
		{
			RTMPSendNullFrame(pAd, 
							  pAd->CommonCfg.TxRate, 
							  (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_WMM_INUSED) ? TRUE:FALSE),
							  PWR_SAVE);

			
			DBGPRINT(RT_DEBUG_TRACE, ("MlmeForceScanReqAction -- Send PSM Data frame for off channel RM, SCAN_IN_PROGRESS=%d!\n",
											RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)));
				OS_WAIT(20);
		}
		
		/* 
                     Check for channel load and noise hist request
                     Suspend MSDU only at scan request, not the last two mentioned
                     Suspend MSDU transmission here
		     YF: Move this line after SendNullFrame due to ScanInProg Flag	
                */
                RTMPSuspendMsduTransmission(pAd);

		RTMPSendWirelessEvent(pAd, IW_SCANNING_EVENT_FLAG, NULL, BSS0, 0);

		NdisGetSystemUpTime(&Now);
		pAd->StaCfg.LastScanTime = Now;
		/* reset all the timers */
		RTMPCancelTimer(&pAd->MlmeAux.BeaconTimer, &TimerCancelled);
		RTMPCancelTimer(&pAd->MlmeAux.ScanTimer, &TimerCancelled);

		/* record desired BSS parameters */
		pAd->MlmeAux.BssType = BssType;
		pAd->MlmeAux.ScanType = ScanType;
		pAd->MlmeAux.SsidLen = SsidLen;
       	 NdisZeroMemory(pAd->MlmeAux.Ssid, MAX_LEN_OF_SSID);
		NdisMoveMemory(pAd->MlmeAux.Ssid, Ssid, SsidLen);

		/*
			Scanning was pending (for fast scanning)
		*/
		if ((pAd->StaCfg.bImprovedScan) && (pAd->Mlme.SyncMachine.CurrState == SCAN_PENDING))
		{
			pAd->MlmeAux.Channel = pAd->StaCfg.LastScanChannel;
		}
		else
		{
			if (pAd->StaCfg.bFastConnect && (pAd->CommonCfg.Channel != 0) && !pAd->StaCfg.bNotFirstScan)
			{
		pAd->MlmeAux.Channel = pAd->CommonCfg.Channel;
			}
			else
				/* start from the first channel */
				pAd->MlmeAux.Channel = FirstChannel(pAd);
		}

		/* Let BBP register at 20MHz to do scan */
		bbp_set_bw(pAd, BW_20);
		DBGPRINT(RT_DEBUG_TRACE, ("SYNC - BBP R4 to 20MHz.l\n"));
#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
		/* Before scan, reset trigger event table. */
		TriEventInit(pAd);
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */

		ScanNextChannel(pAd, OPMODE_STA, INT_MAIN);
		if(pAd->StaCfg.ConnectinfoChannel != 0)
			pAd->MlmeAux.Channel = 0;
		pAd->Mlme.CntlMachine.CurrState = CNTL_WAIT_SCAN_FOR_CONNECT;
	} 
	else 
	{
		DBGPRINT_ERR(("SYNC - MlmeForceScanReqAction() sanity check fail\n"));
		pAd->Mlme.SyncMachine.CurrState = SYNC_IDLE;
		Status = MLME_INVALID_FORMAT;
		MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_SCAN_CONF, 2, &Status, 0);
	}
}



/* 
	==========================================================================
	Description:
		MLME SCAN req state machine procedure
	==========================================================================
 */
VOID MlmeScanReqAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem) 
{
	UCHAR          Ssid[MAX_LEN_OF_SSID], SsidLen, ScanType, BssType;
	BOOLEAN        TimerCancelled;
	ULONG		   Now;
	USHORT         Status;

       /*
	    Check the total scan tries for one single OID command
	    If this is the CCX 2.0 Case, skip that!
	*/
	if ( !RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP))
	{
		DBGPRINT(RT_DEBUG_TRACE, ("SYNC - MlmeScanReqAction before Startup\n"));
		return;
	}

#ifdef PCIE_PS_SUPPORT
    if ((OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_ADVANCE_POWER_SAVE_PCIE_DEVICE)) && 
        (IDLE_ON(pAd)) &&
		(pAd->StaCfg.bRadio == TRUE) &&
		(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF)))
	{
	    if (pAd->StaCfg.PSControl.field.EnableNewPS == FALSE)
		{
			AsicSendCmdToMcuAndWait(pAd, 0x31, PowerWakeCID, 0x00, 0x02, FALSE);   
			RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF);
			DBGPRINT(RT_DEBUG_TRACE, ("PSM - Issue Wake up command \n"));
		}
		else
		{
			RT28xxPciAsicRadioOn(pAd, GUI_IDLE_POWER_SAVE);
		}
	}
#endif /* PCIE_PS_SUPPORT */

	/* first check the parameter sanity */
	if (MlmeScanReqSanity(pAd, 
						  Elem->Msg, 
						  Elem->MsgLen, 
						  &BssType, 
						  (PCHAR)Ssid, 
						  &SsidLen, 
						  &ScanType)) 
	{
#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE                                
                if (pAd->ApCfg.ApCliTab[MAIN_MBSSID].Valid && RTMP_CFG80211_VIF_P2P_CLI_ON(pAd))
                {
                        DBGPRINT(RT_DEBUG_TRACE, ("CFG80211_NULL: PWR_SAVE IN ScanStart\n"));
                        RT_CFG80211_P2P_CLI_SEND_NULL_FRAME(pAd, PWR_SAVE);
                }

#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE */
		
		/*
		    To prevent data lost.	
		    Send an NULL data with turned PSM bit on to current associated AP before SCAN progress.
		    And should send an NULL data with turned PSM bit off to AP, when scan progress done 
		*/
		if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED) && (INFRA_ON(pAd)))
		{
			RTMPSendNullFrame(pAd, 
							  pAd->CommonCfg.TxRate, 
							  (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_WMM_INUSED) ? TRUE:FALSE),
							  PWR_SAVE);
			DBGPRINT(RT_DEBUG_TRACE, ("MlmeScanReqAction -- Send PSM Data frame for off channel RM, SCAN_IN_PROGRESS=%d!\n",
											RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)));
			OS_WAIT(20);
		}
		
		/* 
                     Check for channel load and noise hist request
                     Suspend MSDU only at scan request, not the last two mentioned
                     Suspend MSDU transmission here
                */
                RTMPSuspendMsduTransmission(pAd);

		RTMPSendWirelessEvent(pAd, IW_SCANNING_EVENT_FLAG, NULL, BSS0, 0);

		NdisGetSystemUpTime(&Now);
		pAd->StaCfg.LastScanTime = Now;
		/* reset all the timers */
		RTMPCancelTimer(&pAd->MlmeAux.BeaconTimer, &TimerCancelled);
		RTMPCancelTimer(&pAd->MlmeAux.ScanTimer, &TimerCancelled);

		/* record desired BSS parameters */
		pAd->MlmeAux.BssType = BssType;
		pAd->MlmeAux.ScanType = ScanType;
		pAd->MlmeAux.SsidLen = SsidLen;
		NdisZeroMemory(pAd->MlmeAux.Ssid, MAX_LEN_OF_SSID);
		NdisMoveMemory(pAd->MlmeAux.Ssid, Ssid, SsidLen);

		/*
			Scanning was pending (for fast scanning)
		*/
		if ((pAd->StaCfg.bImprovedScan) && (pAd->Mlme.SyncMachine.CurrState == SCAN_PENDING))
		{
			pAd->MlmeAux.Channel = pAd->StaCfg.LastScanChannel;
		}
		else
		{
			if (pAd->StaCfg.bFastConnect && (pAd->CommonCfg.Channel != 0) && !pAd->StaCfg.bNotFirstScan)
			{
				pAd->MlmeAux.Channel = pAd->CommonCfg.Channel;
			}
			else
			{
				{
					/* start from the first channel */
					pAd->MlmeAux.Channel = FirstChannel(pAd);
				}
			}
		}

		/* Let BBP register at 20MHz to do scan */
		bbp_set_bw(pAd, BW_20);
		DBGPRINT(RT_DEBUG_TRACE, ("SYNC - BBP R4 to 20MHz.l\n"));

#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
		/* Before scan, reset trigger event table. */
		TriEventInit(pAd);
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */

		ScanNextChannel(pAd, OPMODE_STA, INT_MAIN);
	} 
	else 
	{
		DBGPRINT_ERR(("SYNC - MlmeScanReqAction() sanity check fail\n"));
		pAd->Mlme.SyncMachine.CurrState = SYNC_IDLE;
		Status = MLME_INVALID_FORMAT;
		MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_SCAN_CONF, 2, &Status, 0);
	}
}

/* 
	==========================================================================
	Description:
		MLME JOIN req state machine procedure
	==========================================================================
 */
VOID MlmeJoinReqAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem) 
{
	BSS_ENTRY    *pBss;
	BOOLEAN       TimerCancelled;
	HEADER_802_11 Hdr80211;	
	NDIS_STATUS   NStatus;
	ULONG         FrameLen = 0;	
	PUCHAR        pOutBuffer = NULL;
	PUCHAR        pSupRate = NULL;
	UCHAR         SupRateLen;
	PUCHAR        pExtRate = NULL;
	UCHAR         ExtRateLen;	
	UCHAR         ASupRate[] = {0x8C, 0x12, 0x98, 0x24, 0xb0, 0x48, 0x60, 0x6C};
	UCHAR         ASupRateLen = sizeof(ASupRate)/sizeof(UCHAR);	
	MLME_JOIN_REQ_STRUCT *pInfo = (MLME_JOIN_REQ_STRUCT *)(Elem->Msg);
#ifdef WSC_STA_SUPPORT
	BOOLEAN bHasWscIe = FALSE;
#endif /* WSC_STA_SUPPORT */

#ifdef CONFIG_PM
#ifdef USB_SUPPORT_SELECTIVE_SUSPEND
	POS_COOKIE  pObj = (POS_COOKIE) pAd->OS_Cookie;
#endif /* USB_SUPPORT_SELECTIVE_SUSPEND */
#endif /* CONFIG_PM */
	BOOLEAN       bChangeInitBW = FALSE;

	DBGPRINT(RT_DEBUG_TRACE, ("SYNC - MlmeJoinReqAction(BSS #%ld)\n", pInfo->BssIdx));
#ifdef CONFIG_PM
#ifdef USB_SUPPORT_SELECTIVE_SUSPEND

	if ( (RTMP_Usb_AutoPM_Get_Interface(pObj->pUsb_Dev,pObj->intf)) == 1)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("MlmeJoinReqAction: autopm_resume success\n"));
		RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_SUSPEND);
	}
	else if ((RTMP_Usb_AutoPM_Get_Interface(pObj->pUsb_Dev,pObj->intf)) == (-1))
	{
		DBGPRINT(RT_DEBUG_ERROR, ("MlmeJoinReqAction autopm_resume fail ------\n"));
		RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_SUSPEND);
		return;
	}
	else
		DBGPRINT(RT_DEBUG_TRACE, ("MlmeJoinReqAction: autopm_resume do nothing \n"));


#endif /* USB_SUPPORT_SELECTIVE_SUSPEND */
#endif /* CONFIG_PM */



#ifdef PCIE_PS_SUPPORT
    if ((OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_ADVANCE_POWER_SAVE_PCIE_DEVICE)) && 
        (IDLE_ON(pAd)) &&
		(pAd->StaCfg.bRadio == TRUE) &&
		(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF)))
	{
		RT28xxPciAsicRadioOn(pAd, GUI_IDLE_POWER_SAVE);
	}
#endif /* PCIE_PS_SUPPORT */

	/* reset all the timers */
	RTMPCancelTimer(&pAd->MlmeAux.ScanTimer, &TimerCancelled);
	RTMPCancelTimer(&pAd->MlmeAux.BeaconTimer, &TimerCancelled);

	pBss = &pAd->MlmeAux.SsidBssTab.BssEntry[pInfo->BssIdx];

	/* record the desired SSID & BSSID we're waiting for */
	COPY_MAC_ADDR(pAd->MlmeAux.Bssid, pBss->Bssid);
	
	/* If AP's SSID is not hidden, it is OK for updating ssid to MlmeAux again. */
	if (pBss->Hidden == 0)
	{
		RTMPZeroMemory(pAd->MlmeAux.Ssid, MAX_LEN_OF_SSID);
		NdisMoveMemory(pAd->MlmeAux.Ssid, pBss->Ssid, pBss->SsidLen);	
		pAd->MlmeAux.SsidLen = pBss->SsidLen;
	}
	
	pAd->MlmeAux.BssType = pBss->BssType;
	pAd->MlmeAux.Channel = pBss->Channel;
	pAd->MlmeAux.CentralChannel = pBss->CentralChannel;
	
#ifdef EXT_BUILD_CHANNEL_LIST
	/* Country IE of the AP will be evaluated and will be used. */
	if ((pAd->StaCfg.IEEE80211dClientMode != Rt802_11_D_None) &&
		(pBss->bHasCountryIE == TRUE))
	{
		NdisMoveMemory(&pAd->CommonCfg.CountryCode[0], &pBss->CountryString[0], 2);
		if (pBss->CountryString[2] == 'I')
			pAd->CommonCfg.Geography = IDOR;
		else if (pBss->CountryString[2] == 'O')
			pAd->CommonCfg.Geography = ODOR;
		else
			pAd->CommonCfg.Geography = BOTH;
		BuildChannelListEx(pAd);
	}
#endif /* EXT_BUILD_CHANNEL_LIST */
	
	{
		bChangeInitBW = TRUE;
	}


	if (bChangeInitBW == TRUE)
	{
		/* Let BBP register at 20MHz to do scan */
		bbp_set_bw(pAd, BW_20);
		DBGPRINT(RT_DEBUG_TRACE, ("%s(): Set BBP BW=20MHz\n", __FUNCTION__));

		/* switch channel and waiting for beacon timer */
		AsicSwitchChannel(pAd, pAd->MlmeAux.Channel, FALSE);
		AsicLockChannel(pAd, pAd->MlmeAux.Channel);
	}

#ifdef WSC_STA_SUPPORT
#ifdef WSC_LED_SUPPORT
	/* LED indication. */
	if (pAd->MlmeAux.BssType == BSS_INFRA)
	{
		LEDConnectionStart(pAd);
		LEDConnectionCompletion(pAd, TRUE);
	}
#endif /* WSC_LED_SUPPORT */
#endif /* WSC_STA_SUPPORT */

	RTMPSetTimer(&pAd->MlmeAux.BeaconTimer, JOIN_TIMEOUT);

	do
	{
		if (((pAd->CommonCfg.bIEEE80211H == 1) && 
			(pAd->MlmeAux.Channel > 14) && 
			RadarChannelCheck(pAd, pAd->MlmeAux.Channel))
#ifdef CARRIER_DETECTION_SUPPORT /* Roger sync Carrier */             
			|| (pAd->CommonCfg.CarrierDetect.Enable == TRUE)
#endif /* CARRIER_DETECTION_SUPPORT */
		)
		{
			/* We can't send any Probe request frame to meet 802.11h. */
			if (pBss->Hidden == 0)
				break;
		}
		    
		/*
		    send probe request
		*/
		NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);
		if (NStatus == NDIS_STATUS_SUCCESS)
		{
			if (pAd->MlmeAux.Channel <= 14)
			{
				pSupRate = pAd->CommonCfg.SupRate;
				SupRateLen = pAd->CommonCfg.SupRateLen;
				pExtRate = pAd->CommonCfg.ExtRate;
				ExtRateLen = pAd->CommonCfg.ExtRateLen;
			}
			else
			{
				/* Overwrite Support Rate, CCK rate are not allowed */
				pSupRate = ASupRate;
				SupRateLen = ASupRateLen;
				ExtRateLen = 0;
			}

			if (pAd->MlmeAux.BssType == BSS_INFRA)
				MgtMacHeaderInit(pAd, &Hdr80211, SUBTYPE_PROBE_REQ, 0, pAd->MlmeAux.Bssid,
									pAd->CurrentAddress,
									pAd->MlmeAux.Bssid);
			else
				MgtMacHeaderInit(pAd, &Hdr80211, SUBTYPE_PROBE_REQ, 0, BROADCAST_ADDR,
									pAd->CurrentAddress,
									BROADCAST_ADDR);

			MakeOutgoingFrame(pOutBuffer, &FrameLen,
							  sizeof(HEADER_802_11), &Hdr80211,
							  1,                        &SsidIe,
							  1,                        &pAd->MlmeAux.SsidLen,
							  pAd->MlmeAux.SsidLen,	    pAd->MlmeAux.Ssid,
							  1,                        &SupRateIe,
							  1,                        &SupRateLen,
							  SupRateLen,               pSupRate,
							  END_OF_ARGS);

			if (ExtRateLen)
			{
				ULONG Tmp;
				MakeOutgoingFrame(pOutBuffer + FrameLen,            &Tmp,
								  1,                                &ExtRateIe,
								  1,                                &ExtRateLen,
								  ExtRateLen,                       pExtRate, 
								  END_OF_ARGS);
				FrameLen += Tmp;
			}
			
#ifdef WSC_STA_SUPPORT
			/* Append WSC information in probe request if WSC state is running */
			if ((pAd->StaCfg.WscControl.WscEnProbeReqIE) && 
				(pAd->StaCfg.WscControl.WscConfMode != WSC_DISABLE) &&
				(pAd->StaCfg.WscControl.bWscTrigger
				))
				bHasWscIe = TRUE;
#ifdef WSC_V2_SUPPORT
			else if ((pAd->StaCfg.WscControl.WscEnProbeReqIE) && 
				(pAd->StaCfg.WscControl.WscV2Info.bEnableWpsV2))
				bHasWscIe = TRUE;
#endif /* WSC_V2_SUPPORT */

			if (bHasWscIe)
			{
				UCHAR WscIeLen = 0;
				UCHAR *WscBuf = NULL;
				ULONG WscTmpLen = 0;

				/* allocate memory */
				os_alloc_mem(NULL, (UCHAR **)&WscBuf, 256);
				if (WscBuf != NULL)
				{
					NdisZeroMemory(WscBuf, 256);
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
#endif /* WSC_STA_SUPPORT */


#ifdef WPA_SUPPLICANT_SUPPORT
			if ((pAd->OpMode == OPMODE_STA) &&
				(pAd->StaCfg.wpa_supplicant_info.WpaSupplicantUP != WPA_SUPPLICANT_DISABLE) &&
				(pAd->StaCfg.wpa_supplicant_info.WpsProbeReqIeLen != 0))
			{
				ULONG 		WpsTmpLen = 0;
				
				MakeOutgoingFrame(pOutBuffer + FrameLen,              &WpsTmpLen,
								pAd->StaCfg.wpa_supplicant_info.WpsProbeReqIeLen,
								pAd->StaCfg.wpa_supplicant_info.pWpsProbeReqIe,
								END_OF_ARGS);

				FrameLen += WpsTmpLen;
			}
#endif /* WPA_SUPPLICANT_SUPPORT */

#ifdef RT_CFG80211_SUPPORT
	                if ((pAd->OpMode == OPMODE_STA) &&
        	            (pAd->StaCfg.wpa_supplicant_info.WpaSupplicantUP != WPA_SUPPLICANT_DISABLE) &&
                	    (pAd->cfg80211_ctrl.ExtraIeLen != 0))
                	{
                        	ULONG ExtraIeTmpLen = 0;

                        	MakeOutgoingFrame(pOutBuffer + FrameLen,              &ExtraIeTmpLen,
                                	          pAd->cfg80211_ctrl.ExtraIeLen,   pAd->cfg80211_ctrl.pExtraIe,
                                        	  END_OF_ARGS);

                        	FrameLen += ExtraIeTmpLen;
                	}
#endif /* RT_CFG80211_SUPPORT */

#ifdef WFD_SUPPORT
#ifdef RT_CFG80211_SUPPORT
		if (pAd->StaCfg.WfdCfg.bSuppInsertWfdIe)
		{
			ULONG	WfdIeLen, WfdIeBitmap;
			PUCHAR	ptr;
			
			ptr = pOutBuffer + FrameLen;
			WfdIeBitmap = (0x1 << SUBID_WFD_DEVICE_INFO) | (0x1 << SUBID_WFD_ASSOCIATED_BSSID) |
				(0x1 << SUBID_WFD_COUPLED_SINK_INFO);
			WfdMakeWfdIE(pAd, WfdIeBitmap, ptr, &WfdIeLen);
			FrameLen += WfdIeLen;
		}
#endif /* RT_CFG80211_SUPPORT */
#endif /* WFD_SUPPORT */

			MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
			MlmeFreeMemory(pAd, pOutBuffer);
		}
	} while (FALSE);

	DBGPRINT(RT_DEBUG_TRACE, ("SYNC - Switch to ch %d, Wait BEACON from %02x:%02x:%02x:%02x:%02x:%02x\n", 
		pBss->Channel, pBss->Bssid[0], pBss->Bssid[1], pBss->Bssid[2], pBss->Bssid[3], pBss->Bssid[4], pBss->Bssid[5]));

	pAd->Mlme.SyncMachine.CurrState = JOIN_WAIT_BEACON;
}

/* 
	==========================================================================
	Description:
		MLME START Request state machine procedure, starting an IBSS
	==========================================================================
 */
VOID MlmeStartReqAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem) 
{
	UCHAR Ssid[MAX_LEN_OF_SSID], SsidLen; 
	BOOLEAN TimerCancelled;
	UCHAR *VarIE = NULL;		/* New for WPA security suites */
	NDIS_802_11_VARIABLE_IEs *pVIE = NULL;
	LARGE_INTEGER TimeStamp;
	BOOLEAN Privacy;
	USHORT Status;


	/* allocate memory */
	os_alloc_mem(NULL, (UCHAR **)&VarIE, MAX_VIE_LEN);
	if (VarIE == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s: Allocate memory fail!!!\n", __FUNCTION__));
		return;
	}

	/* Init Variable IE structure */
	pVIE = (PNDIS_802_11_VARIABLE_IEs) VarIE;
	pVIE->Length = 0;
	TimeStamp.u.LowPart  = 0;
	TimeStamp.u.HighPart = 0;

	if ((MlmeStartReqSanity(pAd, Elem->Msg, Elem->MsgLen, (PCHAR)Ssid, &SsidLen)) &&
		(CHAN_PropertyCheck(pAd, pAd->MlmeAux.Channel, CHANNEL_NO_IBSS) == FALSE))
	{
		struct wifi_dev *wdev = &pAd->StaCfg.wdev;

		/* reset all the timers */
		RTMPCancelTimer(&pAd->MlmeAux.ScanTimer, &TimerCancelled);
		RTMPCancelTimer(&pAd->MlmeAux.BeaconTimer, &TimerCancelled);

		/* Start a new IBSS. All IBSS parameters are decided now */
		DBGPRINT(RT_DEBUG_TRACE, ("MlmeStartReqAction - Start a new IBSS. All IBSS parameters are decided now.... \n"));
		pAd->MlmeAux.BssType = BSS_ADHOC;
		NdisMoveMemory(pAd->MlmeAux.Ssid, Ssid, SsidLen); 
		pAd->MlmeAux.SsidLen = SsidLen;

#ifdef IWSC_SUPPORT
		if (pAd->StaCfg.IWscInfo.bDoNotChangeBSSID)
			pAd->StaCfg.IWscInfo.bDoNotChangeBSSID = FALSE;
		else
#endif /* IWSC_SUPPORT */
		{
			/* generate a radom number as BSSID */
			MacAddrRandomBssid(pAd, pAd->MlmeAux.Bssid);
			DBGPRINT(RT_DEBUG_TRACE, ("MlmeStartReqAction - generate a radom number as BSSID \n"));
		}

		Privacy = (wdev->WepStatus == Ndis802_11WEPEnabled) || 
				  (wdev->WepStatus == Ndis802_11TKIPEnable) || 
				  (wdev->WepStatus == Ndis802_11AESEnable);
		pAd->MlmeAux.CapabilityInfo = CAP_GENERATE(0,1,Privacy, (pAd->CommonCfg.TxPreamble == Rt802_11PreambleShort), 1, 0);
		pAd->MlmeAux.BeaconPeriod = pAd->CommonCfg.BeaconPeriod;
		pAd->MlmeAux.AtimWin = pAd->StaCfg.AtimWin;
		pAd->MlmeAux.Channel = pAd->CommonCfg.Channel;

		pAd->CommonCfg.CentralChannel = pAd->CommonCfg.Channel;
		pAd->MlmeAux.CentralChannel = pAd->CommonCfg.CentralChannel;

		pAd->MlmeAux.SupRateLen= pAd->CommonCfg.SupRateLen;
		NdisMoveMemory(pAd->MlmeAux.SupRate, pAd->CommonCfg.SupRate, MAX_LEN_OF_SUPPORTED_RATES);
		RTMPCheckRates(pAd, pAd->MlmeAux.SupRate, &pAd->MlmeAux.SupRateLen);
		pAd->MlmeAux.ExtRateLen = pAd->CommonCfg.ExtRateLen;
		NdisMoveMemory(pAd->MlmeAux.ExtRate, pAd->CommonCfg.ExtRate, MAX_LEN_OF_SUPPORTED_RATES);
		RTMPCheckRates(pAd, pAd->MlmeAux.ExtRate, &pAd->MlmeAux.ExtRateLen);
#ifdef DOT11_N_SUPPORT
		if (WMODE_CAP_N(pAd->CommonCfg.PhyMode) && (pAd->StaCfg.bAdhocN == TRUE))
		{
			RTMPUpdateHTIE(&pAd->CommonCfg.DesiredHtPhy, &wdev->DesiredHtPhyInfo.MCSSet[0], &pAd->MlmeAux.HtCapability, &pAd->MlmeAux.AddHtInfo);
			pAd->MlmeAux.HtCapabilityLen = sizeof(HT_CAPABILITY_IE);
			/* Not turn pAd->StaActive.SupportedHtPhy.bHtEnable = TRUE here. */
			DBGPRINT(RT_DEBUG_TRACE, ("SYNC -pAd->StaActive.SupportedHtPhy.bHtEnable = TRUE\n"));
#ifdef DOT11_VHT_AC
			if (WMODE_CAP_AC(pAd->CommonCfg.PhyMode) &&
				(pAd->MlmeAux.Channel > 14))
			{	
				build_vht_cap_ie(pAd, (UCHAR *)&pAd->MlmeAux.vht_cap);
				pAd->MlmeAux.vht_cap_len = sizeof(VHT_CAP_IE);
			}
#endif /* DOT11_VHT_AC */
		}
		else
#endif /* DOT11_N_SUPPORT */
		{
			pAd->MlmeAux.HtCapabilityLen = 0;
			pAd->StaActive.SupportedPhyInfo.bHtEnable = FALSE;
			NdisZeroMemory(&pAd->StaActive.SupportedPhyInfo.MCSSet[0], 16);
		}
		/* temporarily not support QOS in IBSS */
		NdisZeroMemory(&pAd->MlmeAux.APEdcaParm, sizeof(EDCA_PARM));
		NdisZeroMemory(&pAd->MlmeAux.APQbssLoad, sizeof(QBSS_LOAD_PARM));
		NdisZeroMemory(&pAd->MlmeAux.APQosCapability, sizeof(QOS_CAPABILITY_PARM));

		AsicSwitchChannel(pAd, pAd->MlmeAux.Channel, FALSE);
		AsicLockChannel(pAd, pAd->MlmeAux.Channel);

		DBGPRINT(RT_DEBUG_TRACE, ("SYNC - MlmeStartReqAction(ch= %d,sup rates= %d, ext rates=%d)\n",
			pAd->MlmeAux.Channel, pAd->MlmeAux.SupRateLen, pAd->MlmeAux.ExtRateLen));

		pAd->Mlme.SyncMachine.CurrState = SYNC_IDLE;
		Status = MLME_SUCCESS;
		MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_START_CONF, 2, &Status, 0);
	} 
	else 
	{
		DBGPRINT_ERR(("SYNC - MlmeStartReqAction() sanity check fail.\n"));
		pAd->Mlme.SyncMachine.CurrState = SYNC_IDLE;
		Status = MLME_INVALID_FORMAT;
		MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_START_CONF, 2, &Status, 0);
	}

	if (VarIE != NULL)
		os_free_mem(NULL, VarIE);
}


//+++Add by shiang to check correctness of new sanity function
VOID rtmp_dbg_sanity_diff(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	/* Parameters used for old sanity function */
	UCHAR Bssid[MAC_ADDR_LEN], Addr2[MAC_ADDR_LEN];
	UCHAR *Ssid = NULL;
	UCHAR SsidLen=0, DtimCount, DtimPeriod, BcastFlag, MessageToMe, NewChannel, Channel = 0, BssType;
	CF_PARM CfParm = {0};
	USHORT BeaconPeriod, AtimWin, CapabilityInfo;
	LARGE_INTEGER TimeStamp;
	UCHAR SupRate[MAX_LEN_OF_SUPPORTED_RATES], ExtRate[MAX_LEN_OF_SUPPORTED_RATES];
	UCHAR CkipFlag;
	EDCA_PARM EdcaParm = {0};
	UCHAR AironetCellPowerLimit;
	UCHAR SupRateLen, ExtRateLen;
	QBSS_LOAD_PARM QbssLoad;
	QOS_CAPABILITY_PARM QosCapability = {0};
	ULONG RalinkIe;
	UCHAR			AddHtInfoLen;
	EXT_CAP_INFO_ELEMENT	ExtCapInfo;
	HT_CAPABILITY_IE		*pHtCapability = NULL;
	ADD_HT_INFO_IE		*pAddHtInfo = NULL;	/* AP might use this additional ht info IE */
	UCHAR			HtCapabilityLen = 0, PreNHtCapabilityLen = 0;
	UCHAR Erp;
	UCHAR			NewExtChannelOffset = 0xff;
	USHORT LenVIE;
	UCHAR *VarIE = NULL;
	NDIS_802_11_VARIABLE_IEs *pVIE = NULL;
#ifdef CONFIG_STA_SUPPORT
#ifdef NATIVE_WPA_SUPPLICANT_SUPPORT
	UCHAR SelReg = 0;
#endif /* NATIVE_WPA_SUPPLICANT_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */



	BCN_IE_LIST *ie_list = NULL;
	BOOLEAN sanity_new, sanity_old;

	/* allocate memory */
	os_alloc_mem(NULL, (UCHAR **)&Ssid, MAX_LEN_OF_SSID);
	if (Ssid == NULL)
		goto LabelErr;
	os_alloc_mem(NULL, (UCHAR **)&pHtCapability, sizeof(HT_CAPABILITY_IE));
	if (pHtCapability == NULL)
		goto LabelErr;
	os_alloc_mem(NULL, (UCHAR **)&pAddHtInfo, sizeof(ADD_HT_INFO_IE));
	if (pAddHtInfo == NULL)
		goto LabelErr;

	
	NdisZeroMemory(&QbssLoad, sizeof(QBSS_LOAD_PARM)); /* woody */
#ifdef DOT11_N_SUPPORT
    RTMPZeroMemory(pHtCapability, sizeof(HT_CAPABILITY_IE));
	RTMPZeroMemory(pAddHtInfo, sizeof(ADD_HT_INFO_IE));
#endif /* DOT11_N_SUPPORT */

	NdisZeroMemory(Ssid, MAX_LEN_OF_SSID);

	os_alloc_mem(NULL, (UCHAR **)&ie_list, sizeof(BCN_IE_LIST));
	if (ie_list == NULL)
		goto LabelErr;
	NdisZeroMemory(ie_list, sizeof(BCN_IE_LIST));


	sanity_new = PeerBeaconAndProbeRspSanity(pAd,
						&Elem->Msg[0], Elem->MsgLen,
						Elem->Channel,
						ie_list, &LenVIE, pVIE);
						
	sanity_old = PeerBeaconAndProbeRspSanity_Old(pAd, 
								Elem->Msg, 
								Elem->MsgLen, 
								Elem->Channel,
								Addr2, 
								Bssid, 
								(PCHAR)Ssid, 
								&SsidLen, 
								&BssType, 
								&BeaconPeriod, 
								&Channel, 
								&NewChannel,
								&TimeStamp, 
								&CfParm, 
								&AtimWin, 
								&CapabilityInfo, 
								&Erp,
								&DtimCount, 
								&DtimPeriod, 
								&BcastFlag, 
								&MessageToMe, 
								SupRate,
								&SupRateLen,
								ExtRate,
								&ExtRateLen,
								&CkipFlag,
								&AironetCellPowerLimit,
								&EdcaParm,
								&QbssLoad,
								&QosCapability,
								&RalinkIe,
								&HtCapabilityLen,
#ifdef CONFIG_STA_SUPPORT
								&PreNHtCapabilityLen,
#ifdef NATIVE_WPA_SUPPLICANT_SUPPORT
								&SelReg,
#endif /* NATIVE_WPA_SUPPLICANT_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */
								pHtCapability,
								&ExtCapInfo,
								&AddHtInfoLen,
								pAddHtInfo,
								&NewExtChannelOffset,
								&LenVIE,
								pVIE);

		if (sanity_old != sanity_new)
		{
			DBGPRINT(RT_DEBUG_ERROR, ("sanity mismatch, old=%d, new=%d\n", sanity_old, sanity_new));
		}
		else
		{
			if (NdisCmpMemory(&ie_list->Addr2[0], &Addr2[0], MAC_ADDR_LEN) != 0)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("Add2 mismatch!Old=%02x:%02x:%02x:%02x:%02x:%02x!New=%02x:%02x:%02x:%02x:%02x:%02x!\n",
									PRINT_MAC(Addr2), PRINT_MAC(ie_list->Addr2)));
			}

			if (NdisCmpMemory(&ie_list->Bssid[0], &Bssid[0], MAC_ADDR_LEN) != 0)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("Bssid mismatch!Old=%02x:%02x:%02x:%02x:%02x:%02x!New=%02x:%02x:%02x:%02x:%02x:%02x!\n",
									PRINT_MAC(Bssid), PRINT_MAC(ie_list->Bssid)));
			}

			if (SsidLen != ie_list->SsidLen)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("SsidLen mismatch!Old=%d, New=%d\n", SsidLen, ie_list->SsidLen));
			}

			if (NdisCmpMemory(&ie_list->Ssid[0], &Ssid[0], SsidLen) != 0)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("Ssid mismatch!Old=%s, New=%s\n", Ssid, ie_list->Ssid));
			}
			
			if (BssType != ie_list->BssType)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("BssType mismatch!Old=%d, New=%d\n", BssType, ie_list->BssType));
			}

			if (BeaconPeriod != ie_list->BeaconPeriod)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("BeaconPeriod mismatch!Old=%d, New=%d\n", BeaconPeriod, ie_list->BeaconPeriod));
			}

			if (Channel != ie_list->Channel)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("Channel mismatch!Old=%d, New=%d\n", Channel, ie_list->Channel));
			}

			if (NewChannel != ie_list->NewChannel)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("NewChannel mismatch!Old=%d, New=%d\n", NewChannel, ie_list->NewChannel));
			}

			if (NdisCmpMemory(&ie_list->TimeStamp, &TimeStamp, sizeof(LARGE_INTEGER)) != 0)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("TimeStamp mismatch!Old=%d - %d, New=%d - %d\n",
							TimeStamp.u.LowPart, TimeStamp.u.HighPart,
							ie_list->TimeStamp.u.LowPart, ie_list->TimeStamp.u.HighPart));
			}

			if (NdisCmpMemory(&ie_list->CfParm, &CfParm, sizeof(CF_PARM)) != 0)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("CFParam mismatch!\n"));
				hex_dump("Old CFParam", (UCHAR *)&CfParm, sizeof(CF_PARM));
				hex_dump("New CFParam", (UCHAR *)&ie_list->CfParm, sizeof(CF_PARM));
			}

			if (AtimWin != ie_list->AtimWin)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("AtimWin mismatch!Old=%d, New=%d\n", AtimWin, ie_list->AtimWin));
			}


			if (CapabilityInfo != ie_list->CapabilityInfo)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("CapabilityInfo mismatch!Old=%d, New=%d\n", CapabilityInfo, ie_list->CapabilityInfo));
			}

			if (Erp != ie_list->Erp)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("Erp mismatch!Old=%d, New=%d\n", Erp, ie_list->Erp));
			}

			if (DtimCount != ie_list->DtimCount)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("DtimCount mismatch!Old=%d, New=%d\n", DtimCount, ie_list->DtimCount));
			}

			if (DtimPeriod != ie_list->DtimPeriod)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("DtimPeriod mismatch!Old=%d, New=%d\n", DtimPeriod, ie_list->DtimPeriod));
			}

			if (BcastFlag != ie_list->BcastFlag)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("BcastFlag mismatch!Old=%d, New=%d\n", BcastFlag, ie_list->BcastFlag));
			}

			if (MessageToMe != ie_list->MessageToMe)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("MessageToMe mismatch!Old=%d, New=%d\n", MessageToMe, ie_list->MessageToMe));
			}

			if (SupRateLen != ie_list->SupRateLen)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("SupRateLen mismatch!Old=%d, New=%d\n", SupRateLen, ie_list->SupRateLen));
			}

			if (NdisCmpMemory(&ie_list->SupRate[0], &SupRate, ie_list->SupRateLen) != 0)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("SupRate mismatch!\n"));
				hex_dump("Old SupRate", (UCHAR *)&SupRate, ie_list->SupRateLen);
				hex_dump("New SupRate", (UCHAR *)&ie_list->SupRate, ie_list->SupRateLen);
			}


			if (ExtRateLen != ie_list->ExtRateLen)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("ExtRateLen mismatch!Old=%d, New=%d\n", ExtRateLen, ie_list->ExtRateLen));
			}
			if (NdisCmpMemory(&ie_list->ExtRate[0], &ExtRate, ie_list->ExtRateLen) != 0)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("ExtRate mismatch!\n"));
				hex_dump("Old ExtRate", (UCHAR *)&ExtRate, ie_list->ExtRateLen);
				hex_dump("New ExtRate", (UCHAR *)&ie_list->ExtRate, ie_list->ExtRateLen);
			}


			if (CkipFlag != ie_list->CkipFlag)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("CkipFlag mismatch!Old=%d, New=%d\n", CkipFlag, ie_list->CkipFlag));
			}

			if (AironetCellPowerLimit != ie_list->AironetCellPowerLimit)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("AironetCellPowerLimit mismatch!Old=%d, New=%d\n", AironetCellPowerLimit, ie_list->AironetCellPowerLimit));
			}
			if (NdisCmpMemory(&ie_list->EdcaParm, &EdcaParm, sizeof(EDCA_PARM)) != 0)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("EdcaParm mismatch!\n"));
				hex_dump("Old EdcaParm", (UCHAR *)&EdcaParm, sizeof(EDCA_PARM));
				hex_dump("New EdcaParm", (UCHAR *)&ie_list->EdcaParm, sizeof(EDCA_PARM));
			}

			if (NdisCmpMemory(&ie_list->QbssLoad, &QbssLoad, sizeof(QBSS_LOAD_PARM)) != 0)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("QbssLoad mismatch!\n"));
				hex_dump("Old QbssLoad", (UCHAR *)&QbssLoad, sizeof(QBSS_LOAD_PARM));
				hex_dump("New QbssLoad", (UCHAR *)&ie_list->QbssLoad, sizeof(QBSS_LOAD_PARM));
			}

			if (NdisCmpMemory(&ie_list->QosCapability, &QosCapability, sizeof(QOS_CAPABILITY_PARM)) != 0)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("QosCapability mismatch!\n"));
				hex_dump("Old QosCapability", (UCHAR *)&QosCapability, sizeof(QOS_CAPABILITY_PARM));
				hex_dump("New QosCapability", (UCHAR *)&ie_list->QosCapability, sizeof(QOS_CAPABILITY_PARM));
			}

			if (RalinkIe != ie_list->RalinkIe)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("RalinkIe mismatch!Old=%lx, New=%lx\n", RalinkIe, ie_list->RalinkIe));
			}

			if (HtCapabilityLen != ie_list->HtCapabilityLen)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("HtCapabilityLen mismatch!Old=%d, New=%d\n", HtCapabilityLen, ie_list->HtCapabilityLen));
			}

#ifdef CONFIG_STA_SUPPORT
			if (PreNHtCapabilityLen != ie_list->PreNHtCapabilityLen)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("PreNHtCapabilityLen mismatch!Old=%d, New=%d\n", PreNHtCapabilityLen, ie_list->PreNHtCapabilityLen));
			}
#endif /* CONFIG_STA_SUPPORT */
			if (NdisCmpMemory(&ie_list->HtCapability, pHtCapability, sizeof(HT_CAPABILITY_IE)) != 0)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("pHtCapability mismatch!\n"));
				hex_dump("Old HtCapability", (UCHAR *)pHtCapability, sizeof(HT_CAPABILITY_IE));
				hex_dump("New HtCapability", (UCHAR *)&ie_list->HtCapability, sizeof(HT_CAPABILITY_IE));
			}

			if (NdisCmpMemory(&ie_list->ExtCapInfo, &ExtCapInfo, sizeof(EXT_CAP_INFO_ELEMENT)) != 0)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("ExtCapInfo mismatch!\n"));
				hex_dump("Old ExtCapInfo", (UCHAR *)&ExtCapInfo, sizeof(EXT_CAP_INFO_ELEMENT));
				hex_dump("New ExtCapInfo", (UCHAR *)&ie_list->ExtCapInfo, sizeof(EXT_CAP_INFO_ELEMENT));
			}

			if (AddHtInfoLen != ie_list->AddHtInfoLen)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("AddHtInfoLen mismatch!Old=%d, New=%d\n", AddHtInfoLen, ie_list->AddHtInfoLen));
			}

			if (NdisCmpMemory(&ie_list->AddHtInfo, pAddHtInfo, sizeof(ADD_HT_INFO_IE)) != 0)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("AddHtInfo mismatch!\n"));
				hex_dump("Old AddHtInfo", (UCHAR *)pAddHtInfo, sizeof(ADD_HT_INFO_IE));
				hex_dump("New AddHtInfo", (UCHAR *)&ie_list->AddHtInfo, sizeof(ADD_HT_INFO_IE));
			}
			
			if (NewExtChannelOffset != ie_list->NewExtChannelOffset)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("AddHtInfoLen mismatch!Old=%d, New=%d\n", NewExtChannelOffset, ie_list->NewExtChannelOffset));
			}
		}
		goto LabelOK;
			
LabelErr:
	DBGPRINT(RT_DEBUG_ERROR, ("%s: Allocate memory fail!!!\n", __FUNCTION__));

LabelOK:
	if (Ssid != NULL)
		os_free_mem(NULL, Ssid);
	if (VarIE != NULL)
		os_free_mem(NULL, VarIE);
	if (pHtCapability != NULL)
		os_free_mem(NULL, pHtCapability);
	if (pAddHtInfo != NULL)
		os_free_mem(NULL, pAddHtInfo);

	if (ie_list != NULL)
		os_free_mem(NULL, ie_list);
		
}
//---Add by shiang to check correctness of new sanity function


/* 
	==========================================================================
	Description:
		peer sends beacon back when scanning
	==========================================================================
 */
VOID PeerBeaconAtScanAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem) 
{
	PFRAME_802_11 pFrame;
	USHORT LenVIE;
	UCHAR *VarIE = NULL;
	NDIS_802_11_VARIABLE_IEs *pVIE = NULL;
	BCN_IE_LIST *ie_list = NULL;


	os_alloc_mem(NULL, (UCHAR **)&ie_list, sizeof(BCN_IE_LIST));
	if (!ie_list) {
		DBGPRINT(RT_DEBUG_ERROR, ("%s():Alloc ie_list failed!\n", __FUNCTION__));
		return;
	}
	NdisZeroMemory((UCHAR *)ie_list, sizeof(BCN_IE_LIST));

	/* Init Variable IE structure */	
	os_alloc_mem(NULL, (UCHAR **)&VarIE, MAX_VIE_LEN);
	if (VarIE == NULL)
		goto LabelErr;
	pVIE = (PNDIS_802_11_VARIABLE_IEs) VarIE;
	pVIE->Length = 0;


	pFrame = (PFRAME_802_11) Elem->Msg;


	if (PeerBeaconAndProbeRspSanity(pAd,
						&Elem->Msg[0], Elem->MsgLen,
						Elem->Channel,
						ie_list, &LenVIE, pVIE))
	{
		ULONG Idx = 0;
		CHAR Rssi = 0;
		
		Idx = BssTableSearch(&pAd->ScanTab, &ie_list->Bssid[0], ie_list->Channel);
		if (Idx != BSS_NOT_FOUND) 
			Rssi = pAd->ScanTab.BssEntry[Idx].Rssi;

		Rssi = RTMPMaxRssi(pAd, ConvertToRssi(pAd, Elem->Rssi0, RSSI_0),
							ConvertToRssi(pAd, Elem->Rssi1, RSSI_1),
							ConvertToRssi(pAd, Elem->Rssi2, RSSI_2));


#ifdef DOT11_N_SUPPORT
		if ((ie_list->HtCapabilityLen > 0) || (ie_list->PreNHtCapabilityLen > 0))
			ie_list->HtCapabilityLen = SIZE_HT_CAP_IE;
#endif /* DOT11_N_SUPPORT */

		Idx = BssTableSetEntry(pAd, &pAd->ScanTab, ie_list, Rssi, LenVIE, pVIE);
#ifdef DOT11_N_SUPPORT
		/* TODO: Check for things need to do when enable "DOT11V_WNM_SUPPORT" */
#ifdef DOT11N_DRAFT3
		/* Check if this scan channel is the effeced channel */
		if (INFRA_ON(pAd) 
			&& (pAd->CommonCfg.bBssCoexEnable == TRUE) 
			&& ((ie_list->Channel > 0) && (ie_list->Channel <= 14)))
		{
			int chListIdx;

			/* find the channel list idx by the channel number */
			for (chListIdx = 0; chListIdx < pAd->ChannelListNum; chListIdx++)
			{
				if (ie_list->Channel == pAd->ChannelList[chListIdx].Channel)
					break;
			}

			if (chListIdx < pAd->ChannelListNum)
			{
				/* 
					If this channel is effected channel for the 20/40 coex operation. Check the related IEs.
				*/
				if (pAd->ChannelList[chListIdx].bEffectedChannel == TRUE)
				{
					UCHAR RegClass;
					OVERLAP_BSS_SCAN_IE	BssScan;

					/* Read Beacon's Reg Class IE if any. */
					PeerBeaconAndProbeRspSanity2(pAd, Elem->Msg, Elem->MsgLen, &BssScan, &RegClass);
					TriEventTableSetEntry(pAd, &pAd->CommonCfg.TriggerEventTab, &ie_list->Bssid[0], 
										&ie_list->HtCapability, ie_list->HtCapabilityLen,
										RegClass, ie_list->Channel);
				}
			}
		}
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */
		if (Idx != BSS_NOT_FOUND)
		{
			BSS_ENTRY *pBssEntry = &pAd->ScanTab.BssEntry[Idx];
			NdisMoveMemory(pBssEntry->PTSF, &Elem->Msg[24], 4);
			NdisMoveMemory(&pBssEntry->TTSF[0], &Elem->TimeStamp.u.LowPart, 4);
			NdisMoveMemory(&pBssEntry->TTSF[4], &Elem->TimeStamp.u.LowPart, 4);

			pBssEntry->MinSNR = Elem->Signal % 10;
			if (pBssEntry->MinSNR == 0)
				pBssEntry->MinSNR = -5;

			NdisMoveMemory(pBssEntry->MacAddr, &ie_list->Addr2[0], MAC_ADDR_LEN);

			if ((pFrame->Hdr.FC.SubType == SUBTYPE_PROBE_RSP) && (LenVIE != 0))
			{
				pBssEntry->VarIeFromProbeRspLen = 0;
				if (pBssEntry->pVarIeFromProbRsp)
				{
					pBssEntry->VarIeFromProbeRspLen = LenVIE;
					RTMPZeroMemory(pBssEntry->pVarIeFromProbRsp, MAX_VIE_LEN);
					RTMPMoveMemory(pBssEntry->pVarIeFromProbRsp, pVIE, LenVIE);
				}
			}
		}

#ifdef LINUX
#ifdef RT_CFG80211_SUPPORT
		if (RTMPEqualMemory(ie_list->Ssid, "DIRECT-", 7))
			DBGPRINT(RT_DEBUG_OFF, ("%s P2P_SCANNING: %s [%d]\n", __FUNCTION__, ie_list->Ssid, Idx));

		RT_CFG80211_SCANNING_INFORM(pAd, Idx, Elem->Channel, (UCHAR *)pFrame,
									Elem->MsgLen, Rssi);
#endif /* RT_CFG80211_SUPPORT */
#endif /* LINUX */
	}
	/* sanity check fail, ignored */
	goto LabelOK;

LabelErr:
	DBGPRINT(RT_DEBUG_ERROR, ("%s: Allocate memory fail!!!\n", __FUNCTION__));

LabelOK:
	if (VarIE != NULL)
		os_free_mem(NULL, VarIE);
	if (ie_list != NULL)
		os_free_mem(NULL, ie_list);
	return;
}


/* 
	==========================================================================
	Description:
		When waiting joining the (I)BSS, beacon received from external
	==========================================================================
 */
VOID PeerBeaconAtJoinAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem) 
{
	BOOLEAN TimerCancelled;
	USHORT LenVIE;
	USHORT Status;
	UCHAR *VarIE = NULL;
	NDIS_802_11_VARIABLE_IEs *pVIE = NULL;
	ULONG Idx = 0;
	CHAR Rssi = 0;

#ifdef DOT11_N_SUPPORT
	UCHAR CentralChannel;
	BOOLEAN bAllowNrate = FALSE;
#endif /* DOT11_N_SUPPORT */
	BCN_IE_LIST *ie_list = NULL;
#ifdef NATIVE_WPA_SUPPLICANT_SUPPORT
	UCHAR SelReg = 0;
#endif /* NATIVE_WPA_SUPPLICANT_SUPPORT */


	/* allocate memory */	
	os_alloc_mem(NULL, (UCHAR **)&ie_list, sizeof(BCN_IE_LIST));
	if (ie_list == NULL)
		goto LabelErr;
	NdisZeroMemory(ie_list, sizeof(BCN_IE_LIST));
	
	os_alloc_mem(NULL, (UCHAR **)&VarIE, MAX_VIE_LEN);
	if (VarIE == NULL)
		goto LabelErr;
	/* Init Variable IE structure */
	pVIE = (PNDIS_802_11_VARIABLE_IEs) VarIE;
	pVIE->Length = 0;
	

	if (PeerBeaconAndProbeRspSanity(pAd, 
								Elem->Msg, 
								Elem->MsgLen, 
								Elem->Channel,
								ie_list,
								&LenVIE,
								pVIE))
	{
		/* Disqualify 11b only adhoc when we are in 11g only adhoc mode */
		if ((ie_list->BssType == BSS_ADHOC) &&
			WMODE_EQUAL(pAd->CommonCfg.PhyMode, WMODE_G) &&
			((ie_list->SupRateLen+ie_list->ExtRateLen)< 12))
			goto LabelOK;


		/*
		    BEACON from desired BSS/IBSS found. We should be able to decide most
		    BSS parameters here.
		    Q. But what happen if this JOIN doesn't conclude a successful ASSOCIATEION?
		        Do we need to receover back all parameters belonging to previous BSS?
		    A. Should be not. There's no back-door recover to previous AP. It still need
		        a new JOIN-AUTH-ASSOC sequence.
		*/
		if (MAC_ADDR_EQUAL(pAd->MlmeAux.Bssid, &ie_list->Bssid[0])) 
		{
			struct wifi_dev *wdev = &pAd->StaCfg.wdev;

			DBGPRINT(RT_DEBUG_TRACE, ("%s():receive desired BEACON,Channel=%d\n",
								__FUNCTION__, ie_list->Channel));
			RTMPCancelTimer(&pAd->MlmeAux.BeaconTimer, &TimerCancelled);

			/* Update RSSI to prevent No signal display when cards first initialized */
			pAd->StaCfg.RssiSample.LastRssi0 = ConvertToRssi(pAd, Elem->Rssi0, RSSI_0);
			pAd->StaCfg.RssiSample.LastRssi1 = ConvertToRssi(pAd, Elem->Rssi1, RSSI_1);
			pAd->StaCfg.RssiSample.LastRssi2 = ConvertToRssi(pAd, Elem->Rssi2, RSSI_2);
			pAd->StaCfg.RssiSample.AvgRssi0 = pAd->StaCfg.RssiSample.LastRssi0;
			pAd->StaCfg.RssiSample.AvgRssi0X8	= pAd->StaCfg.RssiSample.AvgRssi0 << 3;
			pAd->StaCfg.RssiSample.AvgRssi1 = pAd->StaCfg.RssiSample.LastRssi1;
			pAd->StaCfg.RssiSample.AvgRssi1X8	= pAd->StaCfg.RssiSample.AvgRssi1 << 3;
			pAd->StaCfg.RssiSample.AvgRssi2 = pAd->StaCfg.RssiSample.LastRssi2;
			pAd->StaCfg.RssiSample.AvgRssi2X8	= pAd->StaCfg.RssiSample.AvgRssi2 << 3;

			/*
			  We need to check if SSID only set to any, then we can record the current SSID.
			  Otherwise will cause hidden SSID association failed. 
			*/
			if (pAd->MlmeAux.SsidLen == 0)
			{
				NdisMoveMemory(pAd->MlmeAux.Ssid, ie_list->Ssid, ie_list->SsidLen);
				pAd->MlmeAux.SsidLen = ie_list->SsidLen;
			}
			else
			{
				Idx = BssSsidTableSearch(&pAd->ScanTab, ie_list->Bssid,
										pAd->MlmeAux.Ssid, pAd->MlmeAux.SsidLen,
										ie_list->Channel);

				if (Idx == BSS_NOT_FOUND)				
				{
					Rssi = RTMPMaxRssi(pAd, ConvertToRssi(pAd, Elem->Rssi0, RSSI_0),
											ConvertToRssi(pAd, Elem->Rssi1, RSSI_1),
											ConvertToRssi(pAd, Elem->Rssi2, RSSI_2));
					Idx = BssTableSetEntry(pAd, &pAd->ScanTab, ie_list, Rssi, LenVIE, pVIE);
					if (Idx != BSS_NOT_FOUND)
					{
						NdisMoveMemory(pAd->ScanTab.BssEntry[Idx].PTSF, &Elem->Msg[24], 4);
						NdisMoveMemory(&pAd->ScanTab.BssEntry[Idx].TTSF[0], &Elem->TimeStamp.u.LowPart, 4);
						NdisMoveMemory(&pAd->ScanTab.BssEntry[Idx].TTSF[4], &Elem->TimeStamp.u.LowPart, 4);
						ie_list->CapabilityInfo = pAd->ScanTab.BssEntry[Idx].CapabilityInfo;

						pAd->ScanTab.BssEntry[Idx].MinSNR = Elem->Signal % 10;
						if (pAd->ScanTab.BssEntry[Idx].MinSNR == 0)
							pAd->ScanTab.BssEntry[Idx].MinSNR = -5;

						NdisMoveMemory(pAd->ScanTab.BssEntry[Idx].MacAddr, ie_list->Addr2, MAC_ADDR_LEN);
					}
				}
				else
				{
#ifdef WPA_SUPPLICANT_SUPPORT
					if (pAd->StaCfg.wpa_supplicant_info.WpaSupplicantUP & WPA_SUPPLICANT_ENABLE_WPS)
						;
					else
#endif /* WPA_SUPPLICANT_SUPPORT */
#ifdef WSC_STA_SUPPORT
					if ((pAd->StaCfg.WscControl.WscState != WSC_STATE_OFF)
					)
						;
					else
#endif /* WSC_STA_SUPPORT */
					{

						/*
						    Check if AP privacy is different Staion, if yes, 
						    start a new scan and ignore the frame 
						    (often happen during AP change privacy at short time)
						*/
						if ((((wdev->WepStatus != Ndis802_11WEPDisabled) << 4) ^ ie_list->CapabilityInfo) & 0x0010)
						{	
#ifdef NATIVE_WPA_SUPPLICANT_SUPPORT
							/* When using -Dwext and trigger WPS, do not check security. */
							if ( SelReg == 0 )
#endif /* NATIVE_WPA_SUPPLICANT_SUPPORT */							
							{
							MLME_SCAN_REQ_STRUCT ScanReq;
							DBGPRINT(RT_DEBUG_TRACE, ("%s:AP privacy %d is differenct from STA privacy%d\n", 
										__FUNCTION__, (ie_list->CapabilityInfo & 0x0010) >> 4 ,
										wdev->WepStatus != Ndis802_11WEPDisabled));
							ScanParmFill(pAd, &ScanReq, (PSTRING) pAd->MlmeAux.Ssid, pAd->MlmeAux.SsidLen, BSS_ANY, SCAN_ACTIVE);
							MlmeEnqueue(pAd, SYNC_STATE_MACHINE, MT2_MLME_SCAN_REQ, sizeof(MLME_SCAN_REQ_STRUCT), &ScanReq, 0);
							pAd->Mlme.CntlMachine.CurrState = CNTL_WAIT_OID_LIST_SCAN;
							NdisGetSystemUpTime(&pAd->StaCfg.LastScanTime);
							pAd->Mlme.SyncMachine.CurrState = SYNC_IDLE;
							RTMP_MLME_HANDLER(pAd);
							goto LabelOK;
						}
					}
					}

					/* Multiple SSID case, used correct CapabilityInfo */
					ie_list->CapabilityInfo = pAd->ScanTab.BssEntry[Idx].CapabilityInfo;
				}
			}
			pAd->MlmeAux.CapabilityInfo = ie_list->CapabilityInfo & SUPPORTED_CAPABILITY_INFO;
			pAd->MlmeAux.BssType = ie_list->BssType;
			pAd->MlmeAux.BeaconPeriod = ie_list->BeaconPeriod;

			/*
				Some AP may carrys wrong beacon interval (ex. 0) in Beacon IE.
				We need to check here for preventing divided by 0 error.
			*/
			if (pAd->MlmeAux.BeaconPeriod == 0)
				pAd->MlmeAux.BeaconPeriod = 100;
			
			pAd->MlmeAux.Channel = ie_list->Channel;
			pAd->MlmeAux.AtimWin = ie_list->AtimWin;
			pAd->MlmeAux.CfpPeriod = ie_list->CfParm.CfpPeriod;
			pAd->MlmeAux.CfpMaxDuration = ie_list->CfParm.CfpMaxDuration;
			pAd->MlmeAux.APRalinkIe = ie_list->RalinkIe;

			/*
			    Copy AP's supported rate to MlmeAux for creating assoication request
			    Also filter out not supported rate
			*/
			pAd->MlmeAux.SupRateLen = ie_list->SupRateLen;
			NdisMoveMemory(pAd->MlmeAux.SupRate, ie_list->SupRate, ie_list->SupRateLen);
			RTMPCheckRates(pAd, pAd->MlmeAux.SupRate, &pAd->MlmeAux.SupRateLen);
			pAd->MlmeAux.ExtRateLen = ie_list->ExtRateLen;
			NdisMoveMemory(pAd->MlmeAux.ExtRate, ie_list->ExtRate, ie_list->ExtRateLen);
			RTMPCheckRates(pAd, pAd->MlmeAux.ExtRate, &pAd->MlmeAux.ExtRateLen);

			NdisZeroMemory(pAd->StaActive.SupportedPhyInfo.MCSSet, 16);

#ifdef DOT11R_FT_SUPPORT
			if (pAd->StaCfg.Dot11RCommInfo.bFtSupport &&
				FT_GetMDIE(pVIE, LenVIE, &pAd->MlmeAux.MdIeInfo))
			{
				DBGPRINT(RT_DEBUG_TRACE, ("PeerBeaconAtJoinAction! (MdId=%x%x, FtOverDs=%d, RsrReqCap=%d)\n", 
					pAd->MlmeAux.MdIeInfo.MdId[0], 
					pAd->MlmeAux.MdIeInfo.MdId[1], 
					pAd->MlmeAux.MdIeInfo.FtCapPlc.field.FtOverDs, 
					pAd->MlmeAux.MdIeInfo.FtCapPlc.field.RsrReqCap));
			}
#endif /* DOT11R_FT_SUPPORT */

			/*  Get the ext capability info element */
			NdisMoveMemory(&pAd->MlmeAux.ExtCapInfo, &ie_list->ExtCapInfo,sizeof(ie_list->ExtCapInfo));

#ifdef DOT11_VHT_AC
			pAd->StaActive.SupportedPhyInfo.bVhtEnable = FALSE;
			pAd->StaActive.SupportedPhyInfo.vht_bw = VHT_BW_2040;
#endif /* DOT11_VHT_AC */
			
#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
			DBGPRINT(RT_DEBUG_TRACE, ("MlmeAux.ExtCapInfo=%d\n", pAd->MlmeAux.ExtCapInfo.BssCoexistMgmtSupport));
			if (pAd->CommonCfg.bBssCoexEnable == TRUE)
				pAd->CommonCfg.ExtCapIE.BssCoexistMgmtSupport = 1;
#endif /* DOT11N_DRAFT3 */

			if (((wdev->WepStatus != Ndis802_11WEPEnabled) && (wdev->WepStatus != Ndis802_11TKIPEnable))
				|| (pAd->CommonCfg.HT_DisallowTKIP == FALSE))
			{
				if ((pAd->StaCfg.BssType == BSS_INFRA) || 
					((pAd->StaCfg.BssType == BSS_ADHOC) && (pAd->StaCfg.bAdhocN == TRUE)))
				bAllowNrate = TRUE;			
			}
			
			pAd->MlmeAux.NewExtChannelOffset = ie_list->NewExtChannelOffset;
			pAd->MlmeAux.HtCapabilityLen = ie_list->HtCapabilityLen;

			CentralChannel = ie_list->Channel;
			
			RTMPZeroMemory(&pAd->MlmeAux.HtCapability, SIZE_HT_CAP_IE);
			/* filter out un-supported ht rates */
			if (((ie_list->HtCapabilityLen > 0) || (ie_list->PreNHtCapabilityLen > 0)) && 
				(wdev->DesiredHtPhyInfo.bHtEnable) &&
				(WMODE_CAP_N(pAd->CommonCfg.PhyMode) && bAllowNrate))
			{
   				RTMPMoveMemory(&pAd->MlmeAux.AddHtInfo, &ie_list->AddHtInfo, SIZE_ADD_HT_INFO_IE);
				
                		/* StaActive.SupportedHtPhy.MCSSet stores Peer AP's 11n Rx capability */
				NdisMoveMemory(pAd->StaActive.SupportedPhyInfo.MCSSet, ie_list->HtCapability.MCSSet, 16);
				pAd->MlmeAux.NewExtChannelOffset = ie_list->NewExtChannelOffset;
				pAd->MlmeAux.HtCapabilityLen = SIZE_HT_CAP_IE;
				pAd->StaActive.SupportedPhyInfo.bHtEnable = TRUE;
				if (ie_list->PreNHtCapabilityLen > 0)
					pAd->StaActive.SupportedPhyInfo.bPreNHt = TRUE;
				RTMPCheckHt(pAd, BSSID_WCID, &ie_list->HtCapability, &ie_list->AddHtInfo);
				/* Copy AP Parameter to StaActive.  This is also in LinkUp. */
				DBGPRINT(RT_DEBUG_TRACE, ("%s():(MpduDensity=%d, MaxRAmpduFactor=%d, BW=%d)\n", 
							__FUNCTION__, pAd->StaActive.SupportedHtPhy.MpduDensity,
							pAd->StaActive.SupportedHtPhy.MaxRAmpduFactor,
							ie_list->HtCapability.HtCapInfo.ChannelWidth));

				if (ie_list->AddHtInfoLen > 0)
				{
		 			/* Check again the Bandwidth capability of this AP. */
					CentralChannel = get_cent_ch_by_htinfo(pAd,
													&ie_list->AddHtInfo,
													&ie_list->HtCapability);
                    
		 			DBGPRINT(RT_DEBUG_OFF, ("%s(): HT-CtrlChannel=%d, CentralChannel=>%d\n",
		 						__FUNCTION__, ie_list->AddHtInfo.ControlChan, CentralChannel));
				}

#ifdef DOT11_VHT_AC
				if (WMODE_CAP_AC(pAd->CommonCfg.PhyMode) &&
					(pAd->MlmeAux.Channel > 14) &&
					(ie_list->vht_cap_len) && (ie_list->vht_op_len))
				{
					VHT_OP_INFO *vht_op = &ie_list->vht_op_ie.vht_op_info;

					RTMPZeroMemory(&pAd->MlmeAux.vht_cap, sizeof(VHT_CAP_IE));
					NdisMoveMemory(&pAd->MlmeAux.vht_cap, &ie_list->vht_cap_ie, ie_list->vht_cap_len);
					pAd->MlmeAux.vht_cap_len = ie_list->vht_cap_len;
					
					RTMPZeroMemory(&pAd->MlmeAux.vht_op, sizeof(VHT_OP_IE));
					NdisMoveMemory(&pAd->MlmeAux.vht_op, &ie_list->vht_op_ie, ie_list->vht_op_len);
					pAd->MlmeAux.vht_op_len = ie_list->vht_op_len;
					
					pAd->StaActive.SupportedPhyInfo.bVhtEnable = TRUE;

					if (vht_op->ch_width == 0) {
						pAd->StaActive.SupportedPhyInfo.vht_bw = VHT_BW_2040;
					} else if (vht_op->ch_width == 1) {
						CentralChannel = vht_op->center_freq_1;
						pAd->StaActive.SupportedPhyInfo.vht_bw = VHT_BW_80;
					}

					DBGPRINT(RT_DEBUG_OFF, ("%s(): CentralChannel=>%d\n", 
												__FUNCTION__, CentralChannel));
				}
#endif /* DOT11_VHT_AC */
			}
			else
#endif /* DOT11_N_SUPPORT */
			{
   				/* To prevent error, let legacy AP must have same CentralChannel and Channel. */
				if ((ie_list->HtCapabilityLen == 0) && (ie_list->PreNHtCapabilityLen == 0))
					pAd->MlmeAux.CentralChannel = pAd->MlmeAux.Channel;

				pAd->StaActive.SupportedPhyInfo.bHtEnable = FALSE;
#ifdef DOT11_VHT_AC
				pAd->StaActive.SupportedPhyInfo.bVhtEnable = FALSE;
				pAd->StaActive.SupportedPhyInfo.vht_bw = VHT_BW_2040;
#endif /* DOT11_VHT_AC */
				pAd->MlmeAux.NewExtChannelOffset = 0xff;
				RTMPZeroMemory(&pAd->MlmeAux.HtCapability, SIZE_HT_CAP_IE);
				pAd->MlmeAux.HtCapabilityLen = 0;
				RTMPZeroMemory(&pAd->MlmeAux.AddHtInfo, SIZE_ADD_HT_INFO_IE);
			}

			pAd->hw_cfg.cent_ch = CentralChannel;

			pAd->MlmeAux.CentralChannel = pAd->hw_cfg.cent_ch;

			DBGPRINT(RT_DEBUG_OFF, ("%s(): Set CentralChannel=%d\n", __FUNCTION__, pAd->MlmeAux.CentralChannel));

			RTMPUpdateMlmeRate(pAd);
	
			/* copy QOS related information */
			if ((pAd->CommonCfg.bWmmCapable)
#ifdef DOT11_N_SUPPORT
				 || WMODE_CAP_N(pAd->CommonCfg.PhyMode)
#endif /* DOT11_N_SUPPORT */
				)
			{
				NdisMoveMemory(&pAd->MlmeAux.APEdcaParm, &ie_list->EdcaParm, sizeof(EDCA_PARM));
				NdisMoveMemory(&pAd->MlmeAux.APQbssLoad, &ie_list->QbssLoad, sizeof(QBSS_LOAD_PARM));
				NdisMoveMemory(&pAd->MlmeAux.APQosCapability, &ie_list->QosCapability, sizeof(QOS_CAPABILITY_PARM));
			}
			else
			{
				NdisZeroMemory(&pAd->MlmeAux.APEdcaParm, sizeof(EDCA_PARM));
				NdisZeroMemory(&pAd->MlmeAux.APQbssLoad, sizeof(QBSS_LOAD_PARM));
				NdisZeroMemory(&pAd->MlmeAux.APQosCapability, sizeof(QOS_CAPABILITY_PARM));
			}
			
			DBGPRINT(RT_DEBUG_TRACE, ("%s(): - after JOIN, SupRateLen=%d, ExtRateLen=%d\n", 
								__FUNCTION__, pAd->MlmeAux.SupRateLen,
								pAd->MlmeAux.ExtRateLen));

			if (ie_list->AironetCellPowerLimit != 0xFF)
			{
				/* We need to change our TxPower for CCX 2.0 AP Control of Client Transmit Power */
				ChangeToCellPowerLimit(pAd, ie_list->AironetCellPowerLimit);
			}
			else  /* Used the default TX Power Percentage. */
				pAd->CommonCfg.TxPowerPercentage = pAd->CommonCfg.TxPowerDefault;

			if (pAd->StaCfg.BssType == BSS_INFRA)
			{
				BOOLEAN InfraAP_BW;
				UCHAR BwFallBack = 0;

				if (pAd->MlmeAux.HtCapability.HtCapInfo.ChannelWidth == BW_40)
					InfraAP_BW = TRUE;
				else
					InfraAP_BW = FALSE;

				AdjustChannelRelatedValue(pAd,
											&BwFallBack,
											BSS0,
											InfraAP_BW,
											pAd->MlmeAux.Channel,
											pAd->MlmeAux.CentralChannel);
			}

			pAd->Mlme.SyncMachine.CurrState = SYNC_IDLE;
			Status = MLME_SUCCESS;
			MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_JOIN_CONF, 2, &Status, 0);

#ifdef LINUX
#ifdef RT_CFG80211_SUPPORT
			if (RTMPEqualMemory(ie_list->Ssid, "DIRECT-", 7))
                        	DBGPRINT(RT_DEBUG_OFF, ("%s P2P_SCANNING: %s [%d]\n", __FUNCTION__, ie_list->Ssid, Idx));

			RT_CFG80211_SCANNING_INFORM(pAd, Idx, Elem->Channel, Elem->Msg,
										Elem->MsgLen, Rssi);
#endif /* RT_CFG80211_SUPPORT */
#endif /* LINUX */
		}
		/* not to me BEACON, ignored */
	} 
	/* sanity check fail, ignore this frame */

	goto LabelOK;

LabelErr:
	DBGPRINT(RT_DEBUG_ERROR, ("%s: Allocate memory fail!!!\n", __FUNCTION__));

LabelOK:
	if (ie_list != NULL)
		os_free_mem(NULL, ie_list);
	if (VarIE != NULL)
		os_free_mem(NULL, VarIE);

	return;
}

/* 
	==========================================================================
	Description:
		receive BEACON from peer

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
VOID PeerBeacon(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem) 
{
	UCHAR index=0;
	USHORT TbttNumToNextWakeUp;
	USHORT LenVIE;
	UCHAR *VarIE = NULL;		/* Total VIE length = MAX_VIE_LEN - -5 */
	NDIS_802_11_VARIABLE_IEs *pVIE = NULL;
	struct wifi_dev *wdev = &pAd->StaCfg.wdev;

	BCN_IE_LIST *bcn_ie_list = NULL;
#ifdef NATIVE_WPA_SUPPLICANT_SUPPORT
	UCHAR SelReg = 0;
#endif /* NATIVE_WPA_SUPPLICANT_SUPPORT */

#ifdef RALINK_ATE
    if (ATE_ON(pAd))
		return;
#endif /* RALINK_ATE */

	if (!(INFRA_ON(pAd) || ADHOC_ON(pAd)
		))
		return;

	/* allocate memory */
	os_alloc_mem(NULL, (UCHAR **)&bcn_ie_list, sizeof(BCN_IE_LIST));
	if (bcn_ie_list == NULL)
		goto LabelErr;
	NdisZeroMemory(bcn_ie_list, sizeof(BCN_IE_LIST));
	
	os_alloc_mem(NULL, (UCHAR **)&VarIE, MAX_VIE_LEN);
	if (VarIE == NULL)
		goto LabelErr;
	/* Init Variable IE structure */
	pVIE = (PNDIS_802_11_VARIABLE_IEs) VarIE;
	pVIE->Length = 0;

	if (PeerBeaconAndProbeRspSanity(pAd, 
								Elem->Msg, 
								Elem->MsgLen, 
								Elem->Channel,
								bcn_ie_list,
								&LenVIE,
								pVIE)) 
	{
		BOOLEAN is_my_bssid, is_my_ssid;
		ULONG Bssidx, Now;
		BSS_ENTRY *pBss;
		CHAR RealRssi = RTMPMaxRssi(pAd, ConvertToRssi(pAd, Elem->Rssi0, RSSI_0),
									ConvertToRssi(pAd, Elem->Rssi1, RSSI_1),
									ConvertToRssi(pAd, Elem->Rssi2, RSSI_2));

		is_my_bssid = MAC_ADDR_EQUAL(bcn_ie_list->Bssid, pAd->CommonCfg.Bssid)? TRUE : FALSE;
		is_my_ssid = SSID_EQUAL(bcn_ie_list->Ssid, bcn_ie_list->SsidLen, pAd->CommonCfg.Ssid, pAd->CommonCfg.SsidLen)? TRUE:FALSE;




		/* ignore BEACON not for my SSID */
		if ((!is_my_ssid) && (!is_my_bssid))
			goto LabelOK;

		/* It means STA waits disassoc completely from this AP, ignores this beacon. */
		if (pAd->Mlme.CntlMachine.CurrState == CNTL_WAIT_DISASSOC)
			goto LabelOK;
		
#ifdef DOT11_N_SUPPORT
		/* Copy Control channel for this BSSID. */	
		if (bcn_ie_list->AddHtInfoLen != 0)
			bcn_ie_list->Channel = bcn_ie_list->AddHtInfo.ControlChan;

		if ((bcn_ie_list->HtCapabilityLen > 0) || (bcn_ie_list->PreNHtCapabilityLen > 0))
			bcn_ie_list->HtCapabilityLen = SIZE_HT_CAP_IE;
#endif /* DOT11_N_SUPPORT */

		/* Housekeeping "SsidBssTab" table for later-on ROAMing usage. */
		Bssidx = BssTableSearchWithSSID(&pAd->MlmeAux.SsidBssTab, bcn_ie_list->Bssid, bcn_ie_list->Ssid, bcn_ie_list->SsidLen, bcn_ie_list->Channel);
		if (Bssidx == BSS_NOT_FOUND)
		{			
			/* discover new AP of this network, create BSS entry */
			Bssidx = BssTableSetEntry(pAd, &pAd->MlmeAux.SsidBssTab, bcn_ie_list, RealRssi, LenVIE, pVIE);
			if (Bssidx == BSS_NOT_FOUND)
				;
			else
			{
				BSS_ENTRY *pBssEntry = &pAd->MlmeAux.SsidBssTab.BssEntry[Bssidx];
				NdisMoveMemory(&pBssEntry->PTSF[0], &Elem->Msg[24], 4);
				NdisMoveMemory(&pBssEntry->TTSF[0], &Elem->TimeStamp.u.LowPart, 4);
				NdisMoveMemory(&pBssEntry->TTSF[4], &Elem->TimeStamp.u.LowPart, 4);
				pBssEntry->Rssi = RealRssi;

				NdisMoveMemory(pBssEntry->MacAddr, bcn_ie_list->Addr2, MAC_ADDR_LEN);
				

#ifdef LINUX
#ifdef RT_CFG80211_SUPPORT
//                if (RTMPEqualMemory(ie_list->Ssid, "DIRECT-", 7))
                        DBGPRINT(RT_DEBUG_INFO, ("%s PASSIVE SCANNING: %s [%d]\n", __FUNCTION__, bcn_ie_list->Ssid, Bssidx));

                  RT_CFG80211_SCANNING_INFORM(pAd, Bssidx, Elem->Channel, Elem->Msg,
                                                                        Elem->MsgLen, RealRssi);
#endif /* RT_CFG80211_SUPPORT */
#endif /* LINUX */
		
			}
		}			
		
		/* Update ScanTab */
		Bssidx = BssTableSearch(&pAd->ScanTab, bcn_ie_list->Bssid, bcn_ie_list->Channel);
		if (Bssidx == BSS_NOT_FOUND)
		{
			/* discover new AP of this network, create BSS entry */
			Bssidx = BssTableSetEntry(pAd, &pAd->ScanTab, bcn_ie_list, RealRssi, LenVIE, pVIE);
			if (Bssidx == BSS_NOT_FOUND) /* return if BSS table full */
				goto LabelOK;  
			
			NdisMoveMemory(pAd->ScanTab.BssEntry[Bssidx].PTSF, &Elem->Msg[24], 4);
			NdisMoveMemory(&pAd->ScanTab.BssEntry[Bssidx].TTSF[0], &Elem->TimeStamp.u.LowPart, 4);
			NdisMoveMemory(&pAd->ScanTab.BssEntry[Bssidx].TTSF[4], &Elem->TimeStamp.u.LowPart, 4);
			pAd->ScanTab.BssEntry[Bssidx].MinSNR = Elem->Signal % 10;
			if (pAd->ScanTab.BssEntry[Bssidx].MinSNR == 0)
				pAd->ScanTab.BssEntry[Bssidx].MinSNR = -5;
			
			NdisMoveMemory(pAd->ScanTab.BssEntry[Bssidx].MacAddr, bcn_ie_list->Addr2, MAC_ADDR_LEN);
			
			
			
		}

		/* 
		    if the ssid matched & bssid unmatched, we should select the bssid with large value.
		    This might happened when two STA start at the same time
		*/
		if ((! is_my_bssid) && ADHOC_ON(pAd))
		{
			INT	i;
#ifdef IWSC_SUPPORT
			if ((pAd->StaCfg.WscControl.WscConfMode != WSC_DISABLE) &&
				(pAd->StaCfg.WscControl.bWscTrigger == TRUE))
			{
				;
			}
			else
#endif /* IWSC_SUPPORT */
			/* Add the safeguard against the mismatch of adhoc wep status */
			if ((wdev->WepStatus != pAd->ScanTab.BssEntry[Bssidx].WepStatus) ||
				(wdev->AuthMode != pAd->ScanTab.BssEntry[Bssidx].AuthMode))
			{
				goto LabelOK;
			}
			/* collapse into the ADHOC network which has bigger BSSID value. */
			for (i = 0; i < 6; i++)
			{
				if (bcn_ie_list->Bssid[i] > pAd->CommonCfg.Bssid[i])
				{
					DBGPRINT(RT_DEBUG_TRACE, ("SYNC - merge to the IBSS with bigger BSSID=%02x:%02x:%02x:%02x:%02x:%02x\n", 
						PRINT_MAC(bcn_ie_list->Bssid)));
					AsicDisableSync(pAd);
					COPY_MAC_ADDR(pAd->CommonCfg.Bssid, bcn_ie_list->Bssid);
					AsicSetBssid(pAd, pAd->CommonCfg.Bssid); 
					MakeIbssBeacon(pAd);        /* re-build BEACON frame */
					AsicEnableIbssSync(pAd);    /* copy BEACON frame to on-chip memory */
					is_my_bssid = TRUE;
					break;
				}
				else if (bcn_ie_list->Bssid[i] < pAd->CommonCfg.Bssid[i])
					break;
			}
		}


		NdisGetSystemUpTime(&Now);
		pBss = &pAd->ScanTab.BssEntry[Bssidx];
		pBss->Rssi = RealRssi;       /* lastest RSSI */
		pBss->LastBeaconRxTime = Now;   /* last RX timestamp */

		/*
		   BEACON from my BSSID - either IBSS or INFRA network
		*/ 
		if (is_my_bssid)
		{
			RXWI_STRUC	RxWI;
			UINT8 RXWISize = pAd->chipCap.RXWISize;

#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
			OVERLAP_BSS_SCAN_IE	BssScan;
			UCHAR					RegClass;
			BOOLEAN					brc;

			/* Read Beacon's Reg Class IE if any. */
			brc = PeerBeaconAndProbeRspSanity2(pAd, Elem->Msg, Elem->MsgLen, &BssScan, &RegClass);
			if (brc == TRUE)
			{
				UpdateBssScanParm(pAd, BssScan);
				pAd->StaCfg.RegClass = RegClass;
			}
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */

			pAd->StaCfg.DtimCount = bcn_ie_list->DtimCount;
			pAd->StaCfg.DtimPeriod = bcn_ie_list->DtimPeriod;
			pAd->StaCfg.LastBeaconRxTime = Now;

			NdisZeroMemory(&RxWI, RXWISize);
#ifdef RLT_MAC
			if (pAd->chipCap.hif_type == HIF_RLT)
			{
				{
					RxWI.RXWI_N.rssi[0]= Elem->Rssi0;
					RxWI.RXWI_N.rssi[1] = Elem->Rssi1;
					RxWI.RXWI_N.rssi[2] = Elem->Rssi2;
					RxWI.RXWI_N.phy_mode = 0; /* Prevent SNR calculate error. */
				}
			}
#endif /* RLT_MAC */
#ifdef RTMP_MAC
			if (pAd->chipCap.hif_type == HIF_RTMP)
			{
				RxWI.RXWI_O.RSSI0= Elem->Rssi0;
				RxWI.RXWI_O.RSSI1 = Elem->Rssi1;
				RxWI.RXWI_O.RSSI2 = Elem->Rssi2;
				RxWI.RXWI_O.phy_mode = 0; /* Prevent SNR calculate error. */
			}
#endif /* RTMP_MAC */
			if (INFRA_ON(pAd)) {
				MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.Content[BSSID_WCID];
				if (pEntry)
				{
					Update_Rssi_Sample(pAd, &pEntry->RssiSample, &RxWI);
#ifdef RT_CFG80211_P2P_SINGLE_DEVICE
					if (CFG_P2PCLI_ON(pAd))
					{
						CFG80211_PeerP2pBeacon(pAd, bcn_ie_list->Addr2, Elem, bcn_ie_list->TimeStamp);
					}
#endif /* RT_CFG80211_P2P_SINGLE_DEVICE */
				}
			}

			Update_Rssi_Sample(pAd, &pAd->StaCfg.RssiSample, &RxWI);

			if ((pAd->CommonCfg.bIEEE80211H == 1) &&
				(bcn_ie_list->NewChannel != 0) &&
				(bcn_ie_list->Channel != bcn_ie_list->NewChannel))
			{
				/* Switching to channel 1 can prevent from rescanning the current channel immediately (by auto reconnection). */
				/* In addition, clear the MLME queue and the scan table to discard the RX packets and previous scanning results. */
				AsicSwitchChannel(pAd, 1, FALSE);
				AsicLockChannel(pAd, 1);
			    LinkDown(pAd, FALSE);
				MlmeQueueInit(pAd, &pAd->Mlme.Queue);
				BssTableInit(&pAd->ScanTab);
			    RtmpusecDelay(1000000);		/* use delay to prevent STA do reassoc */
						
				/* channel sanity check */
				for (index = 0 ; index < pAd->ChannelListNum; index++)
				{
					if (pAd->ChannelList[index].Channel == bcn_ie_list->NewChannel)
					{
						pAd->ScanTab.BssEntry[Bssidx].Channel = bcn_ie_list->NewChannel;
						pAd->CommonCfg.Channel = bcn_ie_list->NewChannel;
#ifdef RT_CFG80211_SUPPORT
						//CFG_TODO
#else
						AsicSwitchChannel(pAd, pAd->CommonCfg.Channel, FALSE);
						AsicLockChannel(pAd, pAd->CommonCfg.Channel);
#endif /* RT_CFG80211_SUPPORT */						
						DBGPRINT(RT_DEBUG_TRACE, ("PeerBeacon - STA receive channel switch announcement IE (New Channel =%d)\n", bcn_ie_list->NewChannel));
						break;
					}
				}

				if (index >= pAd->ChannelListNum)
				{
					DBGPRINT_ERR(("PeerBeacon(can not find New Channel=%d in ChannelList[%d]\n", pAd->CommonCfg.Channel, pAd->ChannelListNum));
				}
			}

#ifdef WPA_SUPPLICANT_SUPPORT
			if (pAd->StaCfg.wpa_supplicant_info.WpaSupplicantUP & WPA_SUPPLICANT_ENABLE_WPS)
				;
			else
#endif /* WPA_SUPPLICANT_SUPPORT */
#ifdef WSC_STA_SUPPORT
			if (pAd->StaCfg.WscControl.WscState == WSC_STATE_OFF)
#endif /* WSC_STA_SUPPORT */
			{
				if ((((wdev->WepStatus != Ndis802_11WEPDisabled) << 4) ^ bcn_ie_list->CapabilityInfo) & 0x0010)
				{

#ifdef NATIVE_WPA_SUPPLICANT_SUPPORT
					/* When using -Dwext and trigger WPS, do not check security. */
					if ( SelReg == 0 )
#endif /* NATIVE_WPA_SUPPLICANT_SUPPORT */
					{
					/*
						To prevent STA connect to OPEN/WEP AP when STA is OPEN/NONE or 
						STA connect to OPEN/NONE AP when STA is OPEN/WEP AP.
					*/
					DBGPRINT(RT_DEBUG_TRACE, ("%s:AP privacy:%x is differenct from STA privacy:%x\n",
								__FUNCTION__, (bcn_ie_list->CapabilityInfo & 0x0010) >> 4 , 
								wdev->WepStatus != Ndis802_11WEPDisabled));
					if (INFRA_ON(pAd))
					{
						LinkDown(pAd,FALSE);
						BssTableInit(&pAd->ScanTab);
					}
					goto LabelOK;
				}
			}
			}

#ifdef LINUX
#ifdef RT_CFG80211_SUPPORT
/*			CFG80211_BeaconCountryRegionParse(pAd, pVIE, LenVIE); */
#endif /* RT_CFG80211_SUPPORT */
#endif /* LINUX */

			if (bcn_ie_list->AironetCellPowerLimit != 0xFF)
			{
				/*
				   We get the Cisco (ccx) "TxPower Limit" required
				   Changed to appropriate TxPower Limit for Ciso Compatible Extensions
				*/
				ChangeToCellPowerLimit(pAd, bcn_ie_list->AironetCellPowerLimit);
			}
			else
			{
				/*
				   AironetCellPowerLimit equal to 0xFF means the Cisco (ccx) "TxPower Limit" not exist.
				   Used the default TX Power Percentage, that set from UI.	
				*/
				pAd->CommonCfg.TxPowerPercentage = pAd->CommonCfg.TxPowerDefault;	
			}

			if (ADHOC_ON(pAd) && (CAP_IS_IBSS_ON(bcn_ie_list->CapabilityInfo)))   
			{
				UCHAR			MaxSupportedRateIn500Kbps = 0;
				UCHAR			idx;
				MAC_TABLE_ENTRY *pEntry;
#ifdef IWSC_SUPPORT
				PWSC_CTRL pWpsCtrl = &pAd->StaCfg.WscControl;
#endif /* IWSC_SUPPORT */
	
				MaxSupportedRateIn500Kbps = dot11_max_sup_rate(bcn_ie_list->SupRateLen, &bcn_ie_list->SupRate[0], 
																bcn_ie_list->ExtRateLen, &bcn_ie_list->ExtRate[0]);

				/* look up the existing table */
				pEntry = MacTableLookup(pAd, bcn_ie_list->Addr2);

				/*
				   Ad-hoc mode is using MAC address as BA session. So we need to continuously find newly joined adhoc station by receiving beacon.
				   To prevent always check this, we use wcid == RESERVED_WCID to recognize it as newly joined adhoc station.
				*/
				if ((ADHOC_ON(pAd) && ((!pEntry) || (pEntry && IS_ENTRY_NONE(pEntry)))) ||
					(pEntry && RTMP_TIME_AFTER(Now, pEntry->LastBeaconRxTime + ADHOC_ENTRY_BEACON_LOST_TIME)))
				{
					/* Another adhoc joining, add to our MAC table. */
					if (pEntry == NULL)
						pEntry = MacTableInsertEntry(pAd, bcn_ie_list->Addr2, wdev, BSS0, OPMODE_STA, FALSE);

					if (pEntry == NULL)
						goto LabelOK;

#ifdef IWSC_SUPPORT
					hex_dump("Another adhoc joining - Addr2", bcn_ie_list->Addr2, 6);
					hex_dump("Another adhoc joining - WscPeerMAC", pAd->StaCfg.WscControl.WscPeerMAC, 6);
					if ((NdisEqualMemory(bcn_ie_list->Addr2, pAd->StaCfg.WscControl.WscPeerMAC, MAC_ADDR_LEN)) &&
						(pAd->StaCfg.IWscInfo.bSendEapolStart == FALSE) &&
						(pWpsCtrl->bWscTrigger == TRUE))
					{
						pAd->StaCfg.IWscInfo.bSendEapolStart = TRUE;
					}
#endif /* IWSC_SUPPORT */

#ifdef DOT11_VHT_AC
{
					BOOLEAN result;
					IE_LISTS *ielist;

					os_alloc_mem(NULL, (UCHAR **)&ielist, sizeof(IE_LISTS));
					if (!ielist)
						goto LabelOK;
					NdisZeroMemory((UCHAR *)ielist, sizeof(IE_LISTS));

					if (bcn_ie_list->vht_cap_len && bcn_ie_list->vht_op_len)
					{
						NdisMoveMemory(&ielist->vht_cap, &bcn_ie_list->vht_cap_ie, sizeof(VHT_CAP_IE));
						NdisMoveMemory(&ielist->vht_op, &bcn_ie_list->vht_op_ie, sizeof(VHT_OP_IE));
						ielist->vht_cap_len = bcn_ie_list->vht_cap_len;
						ielist->vht_op_len = bcn_ie_list->vht_op_len;
					}
					result = StaUpdateMacTableEntry(pAd, 
											pEntry, 
											MaxSupportedRateIn500Kbps, 
											&bcn_ie_list->HtCapability, 
											bcn_ie_list->HtCapabilityLen, 
											&bcn_ie_list->AddHtInfo,
											bcn_ie_list->AddHtInfoLen,
											ielist,
											bcn_ie_list->CapabilityInfo);

					os_free_mem(NULL, ielist);
					if ( result== FALSE)
					{
						DBGPRINT(RT_DEBUG_TRACE, ("ADHOC - Add Entry failed.\n"));
						goto LabelOK;
					}
#ifdef IWSC_SUPPORT
					else
						pEntry->bUpdateInfoFromPeerBeacon = TRUE;
#endif /* IWSC_SUPPORT */
}
#else
					if (StaUpdateMacTableEntry(pAd, 
											pEntry, 
											MaxSupportedRateIn500Kbps, 
											&bcn_ie_list->HtCapability, 
											bcn_ie_list->HtCapabilityLen, 
											&bcn_ie_list->AddHtInfo,
											bcn_ie_list->AddHtInfoLen,
											NULL,
											bcn_ie_list->CapabilityInfo) == FALSE)
					{
						DBGPRINT(RT_DEBUG_TRACE, ("ADHOC - Add Entry failed.\n"));
						goto LabelOK;
					}
#ifdef IWSC_SUPPORT
					else
						pEntry->bUpdateInfoFromPeerBeacon = TRUE;
#endif /* IWSC_SUPPORT */
#endif /* DOT11_VHT_AC */

					if (ADHOC_ON(pAd) && pEntry)
					{
						RTMPSetSupportMCS(pAd,
										OPMODE_STA,
										pEntry,
										bcn_ie_list->SupRate,
										bcn_ie_list->SupRateLen,
										bcn_ie_list->ExtRate,
										bcn_ie_list->ExtRateLen,
#ifdef DOT11_VHT_AC
										bcn_ie_list->vht_cap_len,
										&bcn_ie_list->vht_cap_ie,
#endif /* DOT11_VHT_AC */
										&bcn_ie_list->HtCapability,
										bcn_ie_list->HtCapabilityLen);
					}

					pEntry->LastBeaconRxTime = 0;

#ifdef ADHOC_WPA2PSK_SUPPORT
                    /* Adhoc support WPA2PSK by Eddy */
                    if ((wdev->AuthMode == Ndis802_11AuthModeWPA2PSK) 
				&& (pEntry->WPA_Authenticator.WpaState < AS_INITPSK)
#ifdef IWSC_SUPPORT
				&& ((pAd->StaCfg.WscControl.WscConfMode == WSC_DISABLE) ||
					(pAd->StaCfg.WscControl.bWscTrigger == FALSE) ||
					(NdisEqualMemory(pEntry->Addr, pAd->StaCfg.WscControl.WscPeerMAC, MAC_ADDR_LEN) == FALSE))
#ifdef IWSC_TEST_SUPPORT
				&& (pAd->StaCfg.IWscInfo.bBlockConnection == FALSE)
#endif /* IWSC_TEST_SUPPORT */
#endif // IWSC_SUPPORT //
			)                      
                    {
        				INT len, i;
        				PEID_STRUCT         pEid;
                    	NDIS_802_11_VARIABLE_IEs    *pVIE2 = NULL;
						BOOLEAN bHigherMAC = FALSE;

                        pVIE2 = pVIE;
        				len	 = LenVIE;
        				while (len > 0)
        				{
        					pEid = (PEID_STRUCT) pVIE;	
                            if ((pEid->Eid == IE_RSN) && (NdisEqualMemory(pEid->Octet + 2, RSN_OUI, 3)))
        					{
        						NdisMoveMemory(pEntry->RSN_IE, pVIE, (pEid->Len + 2));
        						pEntry->RSNIE_Len = (pEid->Len + 2);	
        					}
        					pVIE2 += (pEid->Len + 2);
        					len  -= (pEid->Len + 2);
        				}
                		pEntry->PortSecured = WPA_802_1X_PORT_NOT_SECURED;
                        NdisZeroMemory(&pEntry->WPA_Supplicant.ReplayCounter, LEN_KEY_DESC_REPLAY);                        
                        NdisZeroMemory(&pEntry->WPA_Authenticator.ReplayCounter, LEN_KEY_DESC_REPLAY);                        
                        pEntry->WPA_Authenticator.WpaState = AS_INITPSK;
                        pEntry->WPA_Supplicant.WpaState = AS_INITPSK;
                		pEntry->EnqueueEapolStartTimerRunning = EAPOL_START_PSK;

						for (i = 0; i < 6; i++)
						{
							if (bcn_ie_list->Addr2[i] > pAd->CurrentAddress[i])
							{
								bHigherMAC = TRUE;
								break;
							}
							else if (bcn_ie_list->Addr2[i] < pAd->CurrentAddress[i])
								break;
						}
						hex_dump("PeerBeacon:: Addr2", bcn_ie_list->Addr2, MAC_ADDR_LEN);
						hex_dump("PeerBeacon:: CurrentAddress", pAd->CurrentAddress, MAC_ADDR_LEN);
						pEntry->bPeerHigherMAC = bHigherMAC;
						if (pEntry->bPeerHigherMAC == FALSE)
						{
							/*
								My MAC address is higher than peer's MAC address.
							*/
							DBGPRINT(RT_DEBUG_TRACE, ("ADHOC - EnqueueStartForPSKTimer.\n"));
                			RTMPSetTimer(&pEntry->EnqueueStartForPSKTimer, ENQUEUE_EAPOL_START_TIMER);                        
                    	}
                    }
					else 
					{
                		pEntry->PortSecured = WPA_802_1X_PORT_SECURED;
                    }
#endif /* ADHOC_WPA2PSK_SUPPORT */

					if (pEntry && (Elem->Wcid == RESERVED_WCID))
					{
						idx = wdev->DefaultKeyId;
							RTMP_SET_WCID_SEC_INFO(pAd, BSS0, idx, 
												   pAd->SharedKey[BSS0][idx].CipherAlg,
												   pEntry->wcid,
												   SHAREDKEYTABLE);
					}
				}

				if (pEntry && IS_ENTRY_CLIENT(pEntry))
				{
					pEntry->LastBeaconRxTime = Now;
#ifdef IWSC_SUPPORT
					if (pEntry->bUpdateInfoFromPeerBeacon == FALSE)
					{
						if (StaUpdateMacTableEntry(pAd, 
												pEntry, 
												MaxSupportedRateIn500Kbps, 
												&bcn_ie_list->HtCapability, 
												bcn_ie_list->HtCapabilityLen, 
												&bcn_ie_list->AddHtInfo,
												bcn_ie_list->AddHtInfoLen,
												NULL,
												bcn_ie_list->CapabilityInfo) == FALSE)
						{
							DBGPRINT(RT_DEBUG_TRACE, ("ADHOC 2 - Add Entry failed.\n"));
							return;
						}

						if (ADHOC_ON(pAd) && pEntry)
						{
							RTMPSetSupportMCS(pAd,
											OPMODE_STA,
											pEntry,
											bcn_ie_list->SupRate,
											bcn_ie_list->SupRateLen,
											bcn_ie_list->ExtRate,
											bcn_ie_list->ExtRateLen,
#ifdef DOT11_VHT_AC
											bcn_ie_list->vht_cap_len,
											&bcn_ie_list->vht_cap_ie,
#endif /* DOT11_VHT_AC */
											&bcn_ie_list->HtCapability,
											bcn_ie_list->HtCapabilityLen);
						}

						pEntry->bUpdateInfoFromPeerBeacon = TRUE;
					}
#endif /* IWSC_SUPPORT */
				}

				/* At least another peer in this IBSS, declare MediaState as CONNECTED */
				if (!OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED))
				{
					OPSTATUS_SET_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED); 
					RTMP_IndicateMediaState(pAd, NdisMediaStateConnected);
	                pAd->ExtraInfo = GENERAL_LINK_UP;
					DBGPRINT(RT_DEBUG_TRACE, ("ADHOC  fOP_STATUS_MEDIA_STATE_CONNECTED.\n"));
				}	
#ifdef IWSC_SUPPORT
				if (pAd->StaCfg.IWscInfo.bSendEapolStart &&
					(pAd->Mlme.IWscMachine.CurrState != IWSC_WAIT_PIN) &&
					(pAd->StaCfg.WscControl.WscConfMode == WSC_ENROLLEE))
				{
					pAd->StaCfg.IWscInfo.bSendEapolStart = FALSE;
					pWpsCtrl->WscState = WSC_STATE_LINK_UP;
					pWpsCtrl->WscStatus = STATUS_WSC_LINK_UP;
					NdisMoveMemory(pWpsCtrl->EntryAddr, pWpsCtrl->WscPeerMAC, MAC_ADDR_LEN);
					WscSendEapolStart(pAd, pWpsCtrl->WscPeerMAC, STA_MODE);
				}
#endif // IWSC_SUPPORT 
			}

			if (INFRA_ON(pAd))
			{
				BOOLEAN bUseShortSlot, bUseBGProtection;

				/*
				   decide to use/change to - 
				      1. long slot (20 us) or short slot (9 us) time
				      2. turn on/off RTS/CTS and/or CTS-to-self protection
				      3. short preamble
				*/

					
				/* bUseShortSlot = pAd->CommonCfg.bUseShortSlotTime && CAP_IS_SHORT_SLOT(CapabilityInfo); */
				bUseShortSlot = CAP_IS_SHORT_SLOT(bcn_ie_list->CapabilityInfo);
				if (bUseShortSlot != OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SHORT_SLOT_INUSED))
					AsicSetSlotTime(pAd, bUseShortSlot);

				bUseBGProtection = (pAd->CommonCfg.UseBGProtection == 1) ||    /* always use */
								   ((pAd->CommonCfg.UseBGProtection == 0) && ERP_IS_USE_PROTECTION(bcn_ie_list->Erp));

				if (pAd->CommonCfg.Channel > 14)  /* always no BG protection in A-band. falsely happened when switching A/G band to a dual-band AP */
					bUseBGProtection = FALSE;

				if (bUseBGProtection != OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_BG_PROTECTION_INUSED))
				{
					if (bUseBGProtection)
					{
						OPSTATUS_SET_FLAG(pAd, fOP_STATUS_BG_PROTECTION_INUSED);
						AsicUpdateProtect(pAd, pAd->MlmeAux.AddHtInfo.AddHtInfo2.OperaionMode,
											(OFDMSETPROTECT|CCKSETPROTECT|ALLN_SETPROTECT),
											FALSE,(pAd->MlmeAux.AddHtInfo.AddHtInfo2.NonGfPresent == 1));
					}
					else
					{
						OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_BG_PROTECTION_INUSED);
						AsicUpdateProtect(pAd, pAd->MlmeAux.AddHtInfo.AddHtInfo2.OperaionMode,
											(OFDMSETPROTECT|CCKSETPROTECT|ALLN_SETPROTECT),TRUE,
											(pAd->MlmeAux.AddHtInfo.AddHtInfo2.NonGfPresent == 1));
					}
					
					DBGPRINT(RT_DEBUG_WARN, ("SYNC - AP changed B/G protection to %d\n", bUseBGProtection));
				}
				
#ifdef DOT11_N_SUPPORT
				/* check Ht protection mode. and adhere to the Non-GF device indication by AP. */
				if ((bcn_ie_list->AddHtInfoLen != 0) && 
					((bcn_ie_list->AddHtInfo.AddHtInfo2.OperaionMode != pAd->MlmeAux.AddHtInfo.AddHtInfo2.OperaionMode) ||
					(bcn_ie_list->AddHtInfo.AddHtInfo2.NonGfPresent != pAd->MlmeAux.AddHtInfo.AddHtInfo2.NonGfPresent)))
				{
					pAd->MlmeAux.AddHtInfo.AddHtInfo2.NonGfPresent = bcn_ie_list->AddHtInfo.AddHtInfo2.NonGfPresent;
					pAd->MlmeAux.AddHtInfo.AddHtInfo2.OperaionMode = bcn_ie_list->AddHtInfo.AddHtInfo2.OperaionMode;
					if (pAd->MlmeAux.AddHtInfo.AddHtInfo2.NonGfPresent == 1)
				{
						AsicUpdateProtect(pAd, pAd->MlmeAux.AddHtInfo.AddHtInfo2.OperaionMode, 
									ALLN_SETPROTECT, FALSE, TRUE);
					}
					else
						AsicUpdateProtect(pAd, pAd->MlmeAux.AddHtInfo.AddHtInfo2.OperaionMode, 
									ALLN_SETPROTECT, FALSE, FALSE);

					DBGPRINT(RT_DEBUG_TRACE, ("SYNC - AP changed N OperaionMode to %d\n", 
						pAd->MlmeAux.AddHtInfo.AddHtInfo2.OperaionMode));
				}
#endif /* DOT11_N_SUPPORT */
				
				if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED) && 
					ERP_IS_USE_BARKER_PREAMBLE(bcn_ie_list->Erp))
				{
					MlmeSetTxPreamble(pAd, Rt802_11PreambleLong);
					DBGPRINT(RT_DEBUG_TRACE, ("SYNC - AP forced to use LONG preamble\n"));
				}

				if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_WMM_INUSED)    &&
					(bcn_ie_list->EdcaParm.bValid == TRUE)                          &&
					(bcn_ie_list->EdcaParm.EdcaUpdateCount != pAd->CommonCfg.APEdcaParm.EdcaUpdateCount))
				{
					DBGPRINT(RT_DEBUG_TRACE, ("SYNC - AP change EDCA parameters(from %d to %d)\n", 
						pAd->CommonCfg.APEdcaParm.EdcaUpdateCount,
						bcn_ie_list->EdcaParm.EdcaUpdateCount));
					AsicSetEdcaParm(pAd, &bcn_ie_list->EdcaParm);
				}

				/* copy QOS related information */
				NdisMoveMemory(&pAd->CommonCfg.APQbssLoad, &bcn_ie_list->QbssLoad, sizeof(QBSS_LOAD_PARM));
				NdisMoveMemory(&pAd->CommonCfg.APQosCapability, &bcn_ie_list->QosCapability, sizeof(QOS_CAPABILITY_PARM));
#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
				/* 
				   2009: PF#1: 20/40 Coexistence in 2.4 GHz Band
				   When AP changes "STA Channel Width" and "Secondary Channel Offset" fields of HT Operation Element in the Beacon to 0
				*/
				if ((bcn_ie_list->AddHtInfoLen != 0) && INFRA_ON(pAd) && pAd->CommonCfg.Channel <= 14)
				{
					BOOLEAN bChangeBW = FALSE;

					/*
					     1) HT Information
					     2) Secondary Channel Offset Element
					
					     40 -> 20 case
					*/
					if (pAd->CommonCfg.BBPCurrentBW == BW_40)
					{
						if (((bcn_ie_list->AddHtInfo.AddHtInfo.ExtChanOffset == EXTCHA_NONE) &&
							(bcn_ie_list->AddHtInfo.AddHtInfo.RecomWidth == 0)) 
							||(bcn_ie_list->NewExtChannelOffset==0x0)
						)
						{
							pAd->StaActive.SupportedHtPhy.ChannelWidth = BW_20;
							pAd->MacTab.Content[BSSID_WCID].HTPhyMode.field.BW = 0;

							{
								bChangeBW = TRUE;
								pAd->CommonCfg.CentralChannel = pAd->CommonCfg.Channel;
								DBGPRINT(RT_DEBUG_TRACE, ("FallBack from 40MHz to 20MHz(CtrlCh=%d, CentralCh=%d)\n", 
															pAd->CommonCfg.Channel, pAd->CommonCfg.CentralChannel));
								CntlChannelWidth(pAd, pAd->CommonCfg.Channel, pAd->CommonCfg.CentralChannel, BW_20, 0);
							}
						}
					}

					/*
					    20 -> 40 case
					    1.) Supported Channel Width Set Field of the HT Capabilities element of both STAs is set to a non-zero
					    2.) Secondary Channel Offset field is SCA or SCB
					    3.) 40MHzRegulatoryClass is TRUE (not implement it)
					*/
					else if (((pAd->CommonCfg.BBPCurrentBW == BW_20) ||(bcn_ie_list->NewExtChannelOffset!=0x0)) &&
							(pAd->CommonCfg.DesiredHtPhy.ChannelWidth != BW_20)
						)
					{
						if ((bcn_ie_list->AddHtInfo.AddHtInfo.ExtChanOffset != EXTCHA_NONE) &&
							(bcn_ie_list->AddHtInfo.AddHtInfo.RecomWidth == 1) &&
							(bcn_ie_list->HtCapabilityLen>0) && (bcn_ie_list->HtCapability.HtCapInfo.ChannelWidth == 1)
						)
						{
							{
								pAd->CommonCfg.CentralChannel = get_cent_ch_by_htinfo(pAd, 
																		&bcn_ie_list->AddHtInfo,
																		&bcn_ie_list->HtCapability);
								if (pAd->CommonCfg.CentralChannel != bcn_ie_list->AddHtInfo.ControlChan)
									bChangeBW = TRUE;
								
								if (bChangeBW)
								{
									pAd->CommonCfg.Channel = bcn_ie_list->AddHtInfo.ControlChan;
										pAd->StaActive.SupportedHtPhy.ChannelWidth = BW_40;
									DBGPRINT(RT_DEBUG_TRACE, ("FallBack from 20MHz to 40MHz(CtrlCh=%d, CentralCh=%d)\n", 
																pAd->CommonCfg.Channel, pAd->CommonCfg.CentralChannel));
									CntlChannelWidth(pAd, pAd->CommonCfg.Channel, pAd->CommonCfg.CentralChannel, BW_40, bcn_ie_list->AddHtInfo.AddHtInfo.ExtChanOffset);
									pAd->MacTab.Content[BSSID_WCID].HTPhyMode.field.BW = 1;
								}
							}
						}
					}

					if (bChangeBW)
					{
						pAd->CommonCfg.BSSCoexist2040.word = 0;
						TriEventInit(pAd);
						BuildEffectedChannelList(pAd);
					}
				}
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */
			}

			/* only INFRASTRUCTURE mode support power-saving feature */
			if ((INFRA_ON(pAd) && (RtmpPktPmBitCheck(pAd) == TRUE)) || (pAd->CommonCfg.bAPSDForcePowerSave))
			{
				UCHAR FreeNumber;
				/*
				     1. AP has backlogged unicast-to-me frame, stay AWAKE, send PSPOLL
				     2. AP has backlogged broadcast/multicast frame and we want those frames, stay AWAKE
				     3. we have outgoing frames in TxRing or MgmtRing, better stay AWAKE
				     4. Psm change to PWR_SAVE, but AP not been informed yet, we better stay AWAKE
				     5. otherwise, put PHY back to sleep to save battery.
				*/
				if (bcn_ie_list->MessageToMe)
				{
#ifdef PCIE_PS_SUPPORT
					if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_ADVANCE_POWER_SAVE_PCIE_DEVICE))
					{
						/* Restore to correct BBP R3 value */
						if (pAd->Antenna.field.RxPath > 1)
							RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, pAd->StaCfg.BBPR3);
						/* Turn clk to 80Mhz. */
					}
#endif /* PCIE_PS_SUPPORT */
#ifdef UAPSD_SUPPORT
					if (pAd->StaCfg.UapsdInfo.bAPSDCapable &&
						pAd->CommonCfg.APEdcaParm.bAPSDCapable &&
						pAd->CommonCfg.bAPSDAC_BE &&
						pAd->CommonCfg.bAPSDAC_BK &&
						pAd->CommonCfg.bAPSDAC_VI &&
						pAd->CommonCfg.bAPSDAC_VO)
					{
						pAd->CommonCfg.bNeedSendTriggerFrame = TRUE;
#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
						RTMPSendNullFrame(pAd, 
				  						pAd->CommonCfg.TxRate, 
				  						TRUE,
				  						pAd->CommonCfg.bAPSDForcePowerSave ? PWR_SAVE : pAd->StaCfg.Psm);
#endif /* defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) */
					}
					else
#endif /* UAPSD_SUPPORT */
					{
						if (pAd->StaCfg.WindowsBatteryPowerMode == Ndis802_11PowerModeFast_PSP)
						{
							/* wake up and send a NULL frame with PM = 0 to the AP */
							RTMP_SET_PSM_BIT(pAd, PWR_ACTIVE);
							RTMPSendNullFrame(pAd, 
											  pAd->CommonCfg.TxRate, 
											  (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_WMM_INUSED) ? TRUE:FALSE),
											  PWR_ACTIVE);
						}
						else
						{
							/* use PS-Poll to get any buffered packet */
							RTMP_PS_POLL_ENQUEUE(pAd);
						}
					}
				}
				else if (bcn_ie_list->BcastFlag && (bcn_ie_list->DtimCount == 0) && OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_RECEIVE_DTIM))
				{
#ifdef PCIE_PS_SUPPORT
					if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_ADVANCE_POWER_SAVE_PCIE_DEVICE))
					{
						if (pAd->Antenna.field.RxPath > 1)
							RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, pAd->StaCfg.BBPR3);
					}
#endif /* PCIE_PS_SUPPORT */
				} 
				else if ((pAd->TxSwQueue[QID_AC_BK].Number != 0) ||
						(pAd->TxSwQueue[QID_AC_BE].Number != 0) ||
						(pAd->TxSwQueue[QID_AC_VI].Number != 0) ||
						(pAd->TxSwQueue[QID_AC_VO].Number != 0) ||
						(RTMPFreeTXDRequest(pAd, QID_AC_BK, TX_RING_SIZE - 1, &FreeNumber) != NDIS_STATUS_SUCCESS) ||
						(RTMPFreeTXDRequest(pAd, QID_AC_BE, TX_RING_SIZE - 1, &FreeNumber) != NDIS_STATUS_SUCCESS) ||
						(RTMPFreeTXDRequest(pAd, QID_AC_VI, TX_RING_SIZE - 1, &FreeNumber) != NDIS_STATUS_SUCCESS) ||
						(RTMPFreeTXDRequest(pAd, QID_AC_VO, TX_RING_SIZE - 1, &FreeNumber) != NDIS_STATUS_SUCCESS) ||
						(RTMPFreeTXDRequest(pAd, QID_MGMT, MGMT_RING_SIZE - 1, &FreeNumber) != NDIS_STATUS_SUCCESS)) 
				{
					/* TODO: consider scheduled HCCA. might not be proper to use traditional DTIM-based power-saving scheme */
					/* can we cheat here (i.e. just check MGMT & AC_BE) for better performance? */
#ifdef PCIE_PS_SUPPORT
					if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_ADVANCE_POWER_SAVE_PCIE_DEVICE))
					{
						if (pAd->Antenna.field.RxPath > 1)
							RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, pAd->StaCfg.BBPR3);
					}
#endif /* PCIE_PS_SUPPORT */
				}
				else 
				{
					if ((pAd->CommonCfg.bACMAPSDTr[QID_AC_VO]) ||
						(pAd->CommonCfg.bACMAPSDTr[QID_AC_VI]) ||
						(pAd->CommonCfg.bACMAPSDTr[QID_AC_BK]) ||
						(pAd->CommonCfg.bACMAPSDTr[QID_AC_BE])
#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
						|| (pAd->StaCfg.FlgPsmCanNotSleep == TRUE)
						|| (RtmpPktPmBitCheck(pAd) == FALSE)
#endif /* defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) */
						)
					{
					}
					else
					{
						USHORT NextDtim = bcn_ie_list->DtimCount;

						if (NextDtim == 0) 
							NextDtim = bcn_ie_list->DtimPeriod;

						TbttNumToNextWakeUp = pAd->StaCfg.DefaultListenCount;
						if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_RECEIVE_DTIM) && (TbttNumToNextWakeUp > NextDtim))
							TbttNumToNextWakeUp = NextDtim;

						if (!OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE))
						{
							/* Set a flag to go to sleep . Then after parse this RxDoneInterrupt, will go to sleep mode. */
							pAd->StaCfg.ThisTbttNumToNextWakeUp = TbttNumToNextWakeUp;
							AsicSleepThenAutoWakeup(pAd, pAd->StaCfg.ThisTbttNumToNextWakeUp);
						}
					}
				}
			}
			
		}
		/* not my BSSID, ignore it */
	}

	/* sanity check fail, ignore this frame */
	goto LabelOK;

LabelErr:
	DBGPRINT(RT_DEBUG_ERROR, ("%s: Allocate memory fail!!!\n", __FUNCTION__));

LabelOK:
	if (VarIE != NULL)
		os_free_mem(NULL, VarIE);
	if (bcn_ie_list != NULL)
		os_free_mem(NULL, bcn_ie_list);

	return;
}

/* 
	==========================================================================
	Description:
		Receive PROBE REQ from remote peer when operating in IBSS mode
	==========================================================================
 */
VOID PeerProbeReqAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem) 
{
	PEER_PROBE_REQ_PARAM ProbeReqParam;
#ifdef DOT11_N_SUPPORT
	UCHAR		  HtLen, AddHtLen, NewExtLen;
#endif /* DOT11_N_SUPPORT */
	HEADER_802_11 ProbeRspHdr;
	NDIS_STATUS   NStatus;
	PUCHAR        pOutBuffer = NULL;
	ULONG         FrameLen = 0;
	LARGE_INTEGER FakeTimestamp;
	UCHAR         DsLen = 1, IbssLen = 2;
	UCHAR         LocalErpIe[3] = {IE_ERP, 1, 0};
	BOOLEAN       Privacy;
	USHORT        CapabilityInfo;


	if (! ADHOC_ON(pAd))
		return;

	if (PeerProbeReqSanity(pAd, Elem->Msg, Elem->MsgLen, &ProbeReqParam))
	{
		struct wifi_dev *wdev = &pAd->StaCfg.wdev;

		if ((ProbeReqParam.SsidLen == 0) ||
			SSID_EQUAL(ProbeReqParam.Ssid, ProbeReqParam.SsidLen, pAd->CommonCfg.Ssid, pAd->CommonCfg.SsidLen))
		{			
			/* allocate and send out ProbeRsp frame */
			NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  /* Get an unused nonpaged memory */
			if (NStatus != NDIS_STATUS_SUCCESS)
				return;

			MgtMacHeaderInit(pAd, &ProbeRspHdr, SUBTYPE_PROBE_RSP,
								0, ProbeReqParam.Addr2,
								pAd->CurrentAddress,
								pAd->CommonCfg.Bssid);

			Privacy = (wdev->WepStatus == Ndis802_11WEPEnabled) || 
					  (wdev->WepStatus == Ndis802_11TKIPEnable) || 
					  (wdev->WepStatus == Ndis802_11AESEnable);
			CapabilityInfo = CAP_GENERATE(0, 1, Privacy, (pAd->CommonCfg.TxPreamble == Rt802_11PreambleShort), 0, 0);

			MakeOutgoingFrame(pOutBuffer,                   &FrameLen, 
							  sizeof(HEADER_802_11),        &ProbeRspHdr, 
							  TIMESTAMP_LEN,                &FakeTimestamp,
							  2,                            &pAd->CommonCfg.BeaconPeriod,
							  2,                            &CapabilityInfo,
							  1,                            &SsidIe, 
							  1,                            &pAd->CommonCfg.SsidLen, 
							  pAd->CommonCfg.SsidLen,       pAd->CommonCfg.Ssid,
							  1,                            &SupRateIe, 
							  1,                            &pAd->StaActive.SupRateLen,
							  pAd->StaActive.SupRateLen,    pAd->StaActive.SupRate, 
							  1,                            &DsIe, 
							  1,                            &DsLen, 
							  1,                            &pAd->CommonCfg.Channel,
							  1,                            &IbssIe, 
							  1,                            &IbssLen, 
							  2,                            &pAd->StaActive.AtimWin,
							  END_OF_ARGS);

			if (pAd->StaActive.ExtRateLen)
			{
				ULONG tmp;
				MakeOutgoingFrame(pOutBuffer + FrameLen,        &tmp, 
								  3,                            LocalErpIe,
								  1,                            &ExtRateIe,
								  1,                            &pAd->StaActive.ExtRateLen,
								  pAd->StaActive.ExtRateLen,    &pAd->StaActive.ExtRate,
								  END_OF_ARGS);
				FrameLen += tmp;
			}

        	/* Modify by Eddy, support WPA2PSK in Adhoc mode */
        	if ((wdev->AuthMode == Ndis802_11AuthModeWPANone)
#ifdef ADHOC_WPA2PSK_SUPPORT
                || (wdev->AuthMode == Ndis802_11AuthModeWPA2PSK)
#endif /* ADHOC_WPA2PSK_SUPPORT */
                )
        	{
        		ULONG   tmp;
       	        UCHAR   RSNIe = IE_WPA;

#ifdef ADHOC_WPA2PSK_SUPPORT
                RTMPMakeRSNIE(pAd, wdev->AuthMode, wdev->WepStatus, BSS0);
            	if (wdev->AuthMode == Ndis802_11AuthModeWPA2PSK)
                    RSNIe = IE_RSN;
#endif /* ADHOC_WPA2PSK_SUPPORT */
                
        		MakeOutgoingFrame(pOutBuffer + FrameLen,        	&tmp,
        						  1,                              	&RSNIe,
        						  1,                            	&pAd->StaCfg.RSNIE_Len,
        						  pAd->StaCfg.RSNIE_Len,      		pAd->StaCfg.RSN_IE,
        						  END_OF_ARGS);
        		FrameLen += tmp;	
        	}

#ifdef DOT11_N_SUPPORT
			if (WMODE_CAP_N(pAd->CommonCfg.PhyMode))
			{
				ULONG TmpLen;
				USHORT  epigram_ie_len;
				UCHAR	BROADCOM[4] = {0x0, 0x90, 0x4c, 0x33};
				HtLen = sizeof(pAd->CommonCfg.HtCapability);
				AddHtLen = sizeof(pAd->CommonCfg.AddHTInfo);
				NewExtLen = 1;
				/* New extension channel offset IE is included in Beacon, Probe Rsp or channel Switch Announcement Frame */
				if (pAd->bBroadComHT == TRUE)
				{
					epigram_ie_len = pAd->MlmeAux.HtCapabilityLen + 4;
					MakeOutgoingFrame(pOutBuffer + FrameLen,            &TmpLen,
								  1,                                &WpaIe,
								  1,          						&epigram_ie_len,
								  4,                                &BROADCOM[0],
								 pAd->MlmeAux.HtCapabilityLen,          &pAd->MlmeAux.HtCapability, 
								  END_OF_ARGS);
				}
				else				
				{
				MakeOutgoingFrame(pOutBuffer + FrameLen,            &TmpLen,
								  1,                                &HtCapIe,
								  1,                                &HtLen,
								 sizeof(HT_CAPABILITY_IE),          &pAd->CommonCfg.HtCapability, 
								  1,                                &AddHtInfoIe,
								  1,                                &AddHtLen,
								 sizeof(ADD_HT_INFO_IE),          &pAd->CommonCfg.AddHTInfo, 
								  END_OF_ARGS);
				}
				FrameLen += TmpLen;
			}
#endif /* DOT11_N_SUPPORT */

#ifdef WSC_STA_SUPPORT
		    /* add Simple Config Information Element */
		    if (pAd->StaCfg.WpsIEProbeResp.ValueLen != 0)
		    {
				ULONG WscTmpLen = 0;
		        
				MakeOutgoingFrame(pOutBuffer + FrameLen,					&WscTmpLen,
								  pAd->StaCfg.WpsIEProbeResp.ValueLen,		pAd->StaCfg.WpsIEProbeResp.Value,
								  END_OF_ARGS);
				FrameLen += WscTmpLen;
		    }
#endif /* WSC_STA_SUPPORT */


			MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
			MlmeFreeMemory(pAd, pOutBuffer);
		}
	}
}

VOID BeaconTimeoutAtJoinAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem) 
{
	USHORT Status;
	DBGPRINT(RT_DEBUG_TRACE, ("SYNC - BeaconTimeoutAtJoinAction\n"));
	pAd->Mlme.SyncMachine.CurrState = SYNC_IDLE;
	Status = MLME_REJ_TIMEOUT;
	MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_JOIN_CONF, 2, &Status, 0);
}

/* 
	==========================================================================
	Description:
		Scan timeout procedure. basically add channel index by 1 and rescan
	==========================================================================
 */
VOID ScanTimeoutAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem) 
{


	if (pAd->StaCfg.bFastConnect && !pAd->StaCfg.bNotFirstScan)
	{
		pAd->MlmeAux.Channel = 0;
		pAd->StaCfg.bNotFirstScan = TRUE;
	}
	else
	{
		pAd->MlmeAux.Channel = NextChannel(pAd, pAd->MlmeAux.Channel);
	}

	/* Only one channel scanned for CISCO beacon request */
	if ((pAd->MlmeAux.ScanType == SCAN_CISCO_ACTIVE) || 
		(pAd->MlmeAux.ScanType == SCAN_CISCO_PASSIVE) ||
		(pAd->MlmeAux.ScanType == SCAN_CISCO_NOISE) ||
		(pAd->MlmeAux.ScanType == SCAN_CISCO_CHANNEL_LOAD))
		pAd->MlmeAux.Channel = 0;

	/* this routine will stop if pAd->MlmeAux.Channel == 0 */
	ScanNextChannel(pAd, OPMODE_STA, pAd->MlmeAux.ScanInfType); 
}

/* 
	==========================================================================
	Description:
	==========================================================================
 */
VOID InvalidStateWhenScan(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem) 
{
	USHORT Status;

	if (Elem->MsgType != MT2_MLME_SCAN_REQ)
		DBGPRINT(RT_DEBUG_TRACE, ("AYNC - InvalidStateWhenScan(state=%ld). Reset SYNC machine\n", pAd->Mlme.SyncMachine.CurrState));
	else
		DBGPRINT(RT_DEBUG_TRACE, ("AYNC - Already in scanning, do nothing here.(state=%ld). \n", pAd->Mlme.SyncMachine.CurrState));
	
	if (Elem->MsgType != MT2_MLME_SCAN_REQ)
	{
		pAd->Mlme.SyncMachine.CurrState = SYNC_IDLE;
		Status = MLME_STATE_MACHINE_REJECT;
		MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_SCAN_CONF, 2, &Status, 0);
	}
}

/* 
	==========================================================================
	Description:
	==========================================================================
 */
VOID InvalidStateWhenJoin(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem) 
{
	USHORT Status;
	DBGPRINT(RT_DEBUG_TRACE, ("InvalidStateWhenJoin(state=%ld, msg=%ld). Reset SYNC machine\n", 
								pAd->Mlme.SyncMachine.CurrState,
								Elem->MsgType));
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))
	{
		RTMPResumeMsduTransmission(pAd);
	}
	pAd->Mlme.SyncMachine.CurrState = SYNC_IDLE;
	Status = MLME_STATE_MACHINE_REJECT;
	MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_JOIN_CONF, 2, &Status, 0);
}

/* 
	==========================================================================
	Description:
	==========================================================================
 */
VOID InvalidStateWhenStart(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem) 
{
	USHORT Status;
	DBGPRINT(RT_DEBUG_TRACE, ("InvalidStateWhenStart(state=%ld). Reset SYNC machine\n", pAd->Mlme.SyncMachine.CurrState));
	pAd->Mlme.SyncMachine.CurrState = SYNC_IDLE;
	Status = MLME_STATE_MACHINE_REJECT;
	MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_START_CONF, 2, &Status, 0);
}

/* 
	==========================================================================
	Description:

	IRQL = DISPATCH_LEVEL
	
	==========================================================================
 */
VOID EnqueuePsPoll(RTMP_ADAPTER *pAd)
{
#ifdef RALINK_ATE
    if (ATE_ON(pAd))
		return;
#endif /* RALINK_ATE */

	
	if (pAd->StaCfg.WindowsPowerMode == Ndis802_11PowerModeLegacy_PSP)
    	pAd->PsPollFrame.FC.PwrMgmt = PWR_SAVE;
	MiniportMMRequest(pAd, QID_AC_BE, (PUCHAR)&pAd->PsPollFrame, sizeof(PSPOLL_FRAME));
	
	/* Keep Waking up */
	if (pAd->CountDowntoPsm <= 0)
		pAd->CountDowntoPsm = 2;	/* 100 ms; stay awake 200ms at most, average will be 1xx ms */
}


/* 
	==========================================================================
	Description:
	==========================================================================
 */
VOID EnqueueProbeRequest(RTMP_ADAPTER *pAd) 
{
	NDIS_STATUS     NState;
	PUCHAR          pOutBuffer;
	ULONG           FrameLen = 0;
	HEADER_802_11   Hdr80211;

	DBGPRINT(RT_DEBUG_TRACE, ("force out a ProbeRequest ...\n"));

	NState = MlmeAllocateMemory(pAd, &pOutBuffer);  /* Get an unused nonpaged memory */
	if (NState == NDIS_STATUS_SUCCESS) 
	{
		MgtMacHeaderInit(pAd, &Hdr80211, SUBTYPE_PROBE_REQ, 0, BROADCAST_ADDR,
							pAd->CurrentAddress,
							BROADCAST_ADDR);

		/* this ProbeRequest explicitly specify SSID to reduce unwanted ProbeResponse */
		MakeOutgoingFrame(pOutBuffer,                     &FrameLen,
						  sizeof(HEADER_802_11),          &Hdr80211,
						  1,                              &SsidIe,
						  1,                              &pAd->CommonCfg.SsidLen,
						  pAd->CommonCfg.SsidLen,		  pAd->CommonCfg.Ssid,
						  1,                              &SupRateIe,
						  1,                              &pAd->StaActive.SupRateLen,
						  pAd->StaActive.SupRateLen,      pAd->StaActive.SupRate, 
						  END_OF_ARGS);
		MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
		MlmeFreeMemory(pAd, pOutBuffer);
	}

}


#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3

/*
	========================================================================
	
	Routine Description:
		Control Primary&Central Channel, ChannelWidth and Second Channel Offset

	Arguments:
		pAd						Pointer to our adapter
		PrimaryChannel			Primary Channel
		CentralChannel			Central Channel
		ChannelWidth				BW_20 or BW_40
		SecondaryChannelOffset	EXTCHA_NONE, EXTCHA_ABOVE and EXTCHA_BELOW
		
	Return Value:
		None
		
	Note:
		
	========================================================================
*/
VOID CntlChannelWidth(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR prim_ch,
	IN UCHAR cent_ch,	
	IN UCHAR ch_bw,
	IN UCHAR sec_ch_offset) 
{
	UCHAR rf_channel = 0;
	UINT8 rf_bw, ext_ch;


	DBGPRINT(RT_DEBUG_TRACE, ("%s: PrimaryChannel[%d] \n",__FUNCTION__,prim_ch));
	DBGPRINT(RT_DEBUG_TRACE, ("%s: CentralChannel[%d] \n",__FUNCTION__,cent_ch));
	DBGPRINT(RT_DEBUG_TRACE, ("%s: ChannelWidth[%d] \n",__FUNCTION__,ch_bw));
	DBGPRINT(RT_DEBUG_TRACE, ("%s: SecondaryChannelOffset[%d] \n",__FUNCTION__,sec_ch_offset));

#ifdef DOT11_N_SUPPORT
	/*Change to AP channel */
	if (ch_bw == BW_40)
	{
		if(sec_ch_offset == EXTCHA_ABOVE)
		{	
			rf_bw = BW_40;
			ext_ch = EXTCHA_ABOVE;
			rf_channel = cent_ch;
		}
		else if (sec_ch_offset == EXTCHA_BELOW)
		{	
			rf_bw = BW_40;
			ext_ch = EXTCHA_BELOW;
			rf_channel = cent_ch;
		}
	}
	else
#endif /* DOT11_N_SUPPORT */
	{
		rf_bw = BW_20;
		ext_ch = EXTCHA_NONE;
		rf_channel = prim_ch;
	}

	if (rf_channel != 0) {
		bbp_set_bw(pAd, rf_bw);

		/* Tx/ RX : control channel setting */
		bbp_set_ctrlch(pAd, ext_ch);
		rtmp_mac_set_ctrlch(pAd, ext_ch);

		AsicSwitchChannel(pAd, rf_channel, FALSE);
		AsicLockChannel(pAd, rf_channel);

#ifdef RT28xx
		RT28xx_ch_tunning(pAd, rf_bw);
#endif /* RT28xx */

		DBGPRINT(RT_DEBUG_TRACE, ("!!!40MHz Lower !!! Control Channel at Below. Central = %d \n", pAd->CommonCfg.CentralChannel ));

		bbp_get_agc(pAd, &pAd->BbpTuning.R66CurrentValue, RX_CHAIN_0);
	}
}
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */

/*
    ==========================================================================
    Description:
        MLME Cancel the SCAN req state machine procedure
    ==========================================================================
 */
VOID ScanCnclAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	BOOLEAN Cancelled;

	RTMPCancelTimer(&pAd->MlmeAux.ScanTimer, &Cancelled);
	pAd->MlmeAux.Channel = 0;
	ScanNextChannel(pAd, OPMODE_STA, INT_MAIN);

	return;
}


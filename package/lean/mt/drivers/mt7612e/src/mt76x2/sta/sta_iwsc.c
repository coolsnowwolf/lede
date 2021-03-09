/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 5F., No.36, Taiyuan St., Jhubei City,
 * Hsinchu County 302,
 * Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2010, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	sta_iwsc

	Abstract:	
	Handle IWSC for IBSS
*/

#ifdef IWSC_SUPPORT

#include "rt_config.h"

#define IWSC_SEL_REG_START_NOTITY		0
#define IWSC_SEL_REG_FINISH_NOTITY	1
#define IWSC_DEV_QUERY_REQUEST		2
#define IWSC_DEV_QUERY_RESPONSE		3

#define IWSC_ENTRY_TIME_OUT			15000

extern UCHAR IWSC_OUI[];
extern UCHAR ZERO_MAC_ADDR[];
extern UCHAR IWSC_ACTION_OUI[];

VOID IWSC_MlmeStartAction(
    IN  PRTMP_ADAPTER		pAd, 
    IN  PMLME_QUEUE_ELEM 	pElem);

VOID	IWSC_MlmeStopAction(
    IN  PRTMP_ADAPTER		pAd, 
    IN  PMLME_QUEUE_ELEM 	pElem);

VOID 	IWSC_InvalidState(
    IN  PRTMP_ADAPTER 		pAd, 
    IN  PMLME_QUEUE_ELEM 	pElem);

VOID	IWSC_ScanDoneAction(
    IN  PRTMP_ADAPTER		pAd, 
    IN  PMLME_QUEUE_ELEM 	pElem);

VOID 	IWSC_ReConnectAction(
    IN  PRTMP_ADAPTER 		pAd, 
    IN  PMLME_QUEUE_ELEM 	pElem);

VOID	IWSC_SendActionFrame(
	IN  PRTMP_ADAPTER		pAd,
	IN  UCHAR				FrameType);

VOID	IWSC_PeerAction(
    IN  PRTMP_ADAPTER		pAd, 
    IN  PMLME_QUEUE_ELEM 	pElem);

VOID	IWSC_RoleAction(
    IN  PRTMP_ADAPTER	pAd,
    IN  INT				WscConfMode);

ULONG 	IWSC_SearchWpsApByPinMethod(
	IN PRTMP_ADAPTER	pAd);

VOID	IWSC_GetConfigMethod(
	IN  PRTMP_ADAPTER	pAd,
	IN	PUCHAR			pVIE,
	IN	INT				VIELen,
	OUT PUSHORT			pConfigMethod);

VOID	IWSC_PeerProbeRequest(
    IN  PRTMP_ADAPTER		pAd,
    IN  PMLME_QUEUE_ELEM 	pElem);

VOID	IWSC_PeerProbeResponse(
    IN  PRTMP_ADAPTER		pAd,
    IN  PMLME_QUEUE_ELEM 	pElem);

VOID	IWSC_PeerPIN(
    IN  PRTMP_ADAPTER		pAd,
    IN  PMLME_QUEUE_ELEM 	pElem);

VOID	IWSC_BuildDevQueryFrame(
	IN  PRTMP_ADAPTER	pAd,
	OUT	PUCHAR			pOutBuf,
	OUT	PUSHORT			pIeLen);

VOID 	IWSC_ResetIpContent(
	IN  PRTMP_ADAPTER	pAd);


/*
    ==========================================================================
    Description:
        IWSC state machine init, including state transition
    Parameters:
        Sm - pointer to the auth state machine
    Note:
        The state machine looks like this        
    ==========================================================================
 */
void	IWSC_StateMachineInit(
    IN  PRTMP_ADAPTER pAd, 
    IN  STATE_MACHINE *Sm, 
    OUT STATE_MACHINE_FUNC Trans[])
{
    StateMachineInit(Sm, 
    				 Trans, 
    				 MAX_IWSC_STATE, 
    				 MAX_IWSC_MSG, 
    				 (STATE_MACHINE_FUNC)Drop, 
    				 IWSC_IDLE, 
    				 IWSC_MACHINE_BASE);

	StateMachineSetAction(Sm, IWSC_IDLE, IWSC_MT2_MLME_START, (STATE_MACHINE_FUNC)IWSC_MlmeStartAction);
	StateMachineSetAction(Sm, IWSC_IDLE, IWSC_MT2_PEER_ACTION_FRAME, (STATE_MACHINE_FUNC)IWSC_PeerAction);
	StateMachineSetAction(Sm, IWSC_IDLE, IWSC_MT2_MLME_SCAN_DONE, (STATE_MACHINE_FUNC)IWSC_ScanDoneAction);
	StateMachineSetAction(Sm, IWSC_IDLE, IWSC_MT2_MLME_RECONNECT, (STATE_MACHINE_FUNC)IWSC_InvalidState);

	StateMachineSetAction(Sm, IWSC_START, IWSC_MT2_MLME_START, (STATE_MACHINE_FUNC)IWSC_MlmeStartAction);
	StateMachineSetAction(Sm, IWSC_START, IWSC_MT2_MLME_STOP, (STATE_MACHINE_FUNC)IWSC_MlmeStopAction);
	StateMachineSetAction(Sm, IWSC_START, IWSC_MT2_PEER_PROBE_REQ, (STATE_MACHINE_FUNC)IWSC_PeerProbeRequest);
	StateMachineSetAction(Sm, IWSC_START, IWSC_MT2_MLME_SCAN_DONE, (STATE_MACHINE_FUNC)IWSC_ScanDoneAction);
	StateMachineSetAction(Sm, IWSC_START, IWSC_MT2_MLME_RECONNECT, (STATE_MACHINE_FUNC)IWSC_ReConnectAction);

	StateMachineSetAction(Sm, IWSC_SCAN, IWSC_MT2_MLME_START, (STATE_MACHINE_FUNC)IWSC_MlmeStartAction);
	StateMachineSetAction(Sm, IWSC_SCAN, IWSC_MT2_MLME_STOP, (STATE_MACHINE_FUNC)IWSC_MlmeStopAction);
	StateMachineSetAction(Sm, IWSC_SCAN, IWSC_MT2_PEER_PROBE_RSP, (STATE_MACHINE_FUNC)IWSC_PeerProbeResponse);
	StateMachineSetAction(Sm, IWSC_SCAN, IWSC_MT2_MLME_SCAN_DONE, (STATE_MACHINE_FUNC)IWSC_ScanDoneAction);
	StateMachineSetAction(Sm, IWSC_SCAN, IWSC_MT2_MLME_RECONNECT, (STATE_MACHINE_FUNC)IWSC_InvalidState);

	StateMachineSetAction(Sm, IWSC_WAIT_PIN, IWSC_MT2_PEER_PIN, (STATE_MACHINE_FUNC)IWSC_PeerPIN);
	StateMachineSetAction(Sm, IWSC_WAIT_PIN, IWSC_MT2_MLME_STOP, (STATE_MACHINE_FUNC)IWSC_MlmeStopAction);

	StateMachineSetAction(Sm, IWSC_WAIT_JOIN, IWSC_MT2_MLME_STOP, (STATE_MACHINE_FUNC)IWSC_MlmeStopAction);
	
}


VOID IWSC_InvalidState(RTMP_ADAPTER *pAd, PMLME_QUEUE_ELEM pElem)
{
    DBGPRINT(RT_DEBUG_TRACE, ("IWSC - InvalidState (state=%ld, msg_type = %ld)\n", 
								pAd->Mlme.IWscMachine.CurrState, pElem->MsgType));
}


VOID IWSC_Init(RTMP_ADAPTER *pAd)
{
	PIWSC_INFO	pIWscInfo = &pAd->StaCfg.IWscInfo;

	RTMPInitTimer(pAd, 
				  &pIWscInfo->IWscT1Timer,
				  GET_TIMER_FUNCTION(IWSC_T1TimerAction), 
				  pAd, 
				  FALSE);

	RTMPInitTimer(pAd, 
				  &pIWscInfo->IWscT2Timer,
				  GET_TIMER_FUNCTION(IWSC_T2TimerAction), 
				  pAd, 
				  FALSE);
	
	RTMPInitTimer(pAd, 
				  &pIWscInfo->IWscEntryTimer,
				  GET_TIMER_FUNCTION(IWSC_EntryTimerAction), 
				  pAd, 
				  FALSE);

	RTMPInitTimer(pAd, 
				  &pIWscInfo->IWscDevQueryTimer,
				  GET_TIMER_FUNCTION(IWSC_DevQueryAction), 
				  pAd, 
				  FALSE);

	pIWscInfo->bIWscT1TimerRunning = FALSE;
	pIWscInfo->bIWscT2TimerRunning = FALSE;
	pIWscInfo->bIWscEntryTimerRunning = FALSE;
	pIWscInfo->bIWscDevQueryReqTimerRunning = FALSE;
	pIWscInfo->bIWscDevQueryRspTimerRunning = FALSE;

	pIWscInfo->DialogToken = 0;
	pIWscInfo->PeerDialogToken = 0;
	pIWscInfo->bSelRegStart = FALSE;
	pIWscInfo->bReStart = FALSE;
	pIWscInfo->IWscSmpbcAcceptCount = 0;
	pIWscInfo->bLimitedUI = FALSE;
	pIWscInfo->bSinglePIN = FALSE;
	pIWscInfo->bDoNotChangeBSSID = FALSE;
	pIWscInfo->bSendEapolStart = FALSE;
	pIWscInfo->IpConfMethod = (IWSC_IPV4_ASSIGNMENT | IWSC_DHCP_IPV4 | IWSC_STATIC_IPV4);
	pIWscInfo->IpMethod = IWSC_IPV4_ASSIGNMENT;
	pIWscInfo->SelfIpv4Addr = IWSC_DEFAULT_REG_IPV4_ADDR;
	pIWscInfo->PeerIpv4Addr = 0;
	pIWscInfo->RegIpv4Addr = 0;
	pIWscInfo->Ipv4SubMask = IWSC_DEFAULT_IPV4_SUBMASK;
	pIWscInfo->CurrentIpRange = IWSC_DEFAULT_IPV4_RANGE;
	pIWscInfo->RegDepth = 0;
	pIWscInfo->IpDevCount = 0;
	pIWscInfo->AvaSubMaskListCount = IWSC_MAX_SUB_MASK_LIST_COUNT;
	pIWscInfo->AvaSubMaskList[0] = IWSC_IPV4_RANGE1;
	pIWscInfo->AvaSubMaskList[1] = IWSC_IPV4_RANGE2;
	pIWscInfo->AvaSubMaskList[2] = IWSC_IPV4_RANGE3;
	pIWscInfo->bAssignWscIPv4 = TRUE;
	pIWscInfo->bDoNotStop = FALSE;
	pIWscInfo->SmpbcEnrolleeCount = 0;
	RTMPZeroMemory(pIWscInfo->RegMacAddr, MAC_ADDR_LEN);
	RTMPZeroMemory(pIWscInfo->IWscDevQueryReqMacAddr, MAC_ADDR_LEN);

	initList(&pAd->StaCfg.WscControl.WscConfiguredPeerList);
	NdisAllocateSpinLock(pAd, &pAd->StaCfg.WscControl.WscConfiguredPeerListSemLock);
	
#ifdef IWSC_TEST_SUPPORT
	pIWscInfo->IWscDefaultSecurity = 3;
	pIWscInfo->bIwscSmpbcScanningOnly = FALSE;
	pIWscInfo->bEmptySubmaskList = FALSE;
#endif // IWSC_TEST_SUPPORT //
}


VOID IWSC_Stop(RTMP_ADAPTER *pAd, BOOLEAN bSendNotification)
{
	PWSC_CTRL	pWpsCtrl = &pAd->StaCfg.WscControl;
	PIWSC_INFO	pIWscInfo = &pAd->StaCfg.IWscInfo;
	BOOLEAN		bCancelled;

	DBGPRINT(RT_DEBUG_TRACE, ("<----- IWSC_Stop\n"));

	if (pIWscInfo->bIWscT1TimerRunning)
	{
		pIWscInfo->bIWscT1TimerRunning = FALSE;
		RTMPCancelTimer(&pIWscInfo->IWscT1Timer, &bCancelled);
	}
	
	if (pIWscInfo->bIWscT2TimerRunning)
	{
		pIWscInfo->bIWscT2TimerRunning = FALSE;
		RTMPCancelTimer(&pIWscInfo->IWscT2Timer, &bCancelled);
	}

	if (pIWscInfo->bIWscEntryTimerRunning)
	{
		pIWscInfo->bIWscEntryTimerRunning = FALSE;
		RTMPCancelTimer(&pIWscInfo->IWscEntryTimer, &bCancelled);		
	}

	if (pWpsCtrl->WscPBCTimerRunning)
	{
		pWpsCtrl->WscPBCTimerRunning = FALSE;
		RTMPCancelTimer(&pWpsCtrl->WscPBCTimer, &bCancelled);
	}
	pIWscInfo->IWscSmpbcAcceptCount = 0;

	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_REMOVE_IN_PROGRESS) == FALSE)
	{
		AsicDisableSync(pAd);
		pAd->StaCfg.WpsIEBeacon.ValueLen = 0;
		pAd->StaCfg.WpsIEProbeResp.ValueLen = 0;
		MakeIbssBeacon(pAd);        /* re-build BEACON frame */
		AsicEnableIbssSync(pAd);    /* copy BEACON frame to on-chip memory */

		WscStop(pAd,
#ifdef CONFIG_AP_SUPPORT
				FALSE,
#endif /* CONFIG_AP_SUPPORT */
				pWpsCtrl);
	}
	
	if (bSendNotification)
	{
		/*
			Send Selected Registrar Finish Notification (multicast action frame)
		*/	
		IWSC_SendActionFrame(pAd, IWSC_SEL_REG_FINISH_NOTITY);
	}
	pAd->Mlme.IWscMachine.CurrState = IWSC_IDLE;

	DBGPRINT(RT_DEBUG_TRACE, ("-----> IWSC_Stop\n"));
}


/**************************************************** IWSC Timer Function Begin ****************************************************/
VOID IWSC_T1TimerAction(
    IN  PVOID SystemSpecific1,
    IN  PVOID FunctionContext,
    IN  PVOID SystemSpecific2,
    IN  PVOID SystemSpecific3)
{
    RTMP_ADAPTER    *pAd = (PRTMP_ADAPTER)FunctionContext;
	PWSC_CTRL		pWpsCtrl = NULL;

	DBGPRINT(RT_DEBUG_TRACE, ("!!! IWSC_T1TimerAction !!!\n"));

	if (pAd)
	{
		pWpsCtrl = &pAd->StaCfg.WscControl;
		if (pWpsCtrl)
		{
			pAd->StaCfg.IWscInfo.bDoNotStop = FALSE;
			if ((pWpsCtrl->WscConfMode == WSC_REGISTRAR) && OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED))
			{
				/*
					Send Selected Registrar Finish Notification (multicast action frame)
				*/
				IWSC_Stop(pAd, TRUE);
			}
			else
				IWSC_Stop(pAd, FALSE);
			
			pWpsCtrl->WscStatus = STATUS_WSC_IDLE;
#ifdef IWSC_TEST_SUPPORT
			pAd->StaCfg.IWscInfo.IWscConfMode = WSC_DISABLE;
#endif // IWSC_TEST_SUPPORT //			
			RTMPSendWirelessEvent(pAd, IW_IWSC_T1_TIMER_TIMEOUT, NULL, pWpsCtrl->EntryIfIdx, 0);
		}
		else
			DBGPRINT(RT_DEBUG_ERROR, ("!!! IWSC_T1TimerAction - pWpsCtrl is null !!!\n"));
	}
	else
		DBGPRINT(RT_DEBUG_ERROR, ("!!! IWSC_T1TimerAction - pAd is null !!!\n"));
}


VOID IWSC_T2TimerAction(
    IN  PVOID SystemSpecific1,
    IN  PVOID FunctionContext,
    IN  PVOID SystemSpecific2,
    IN  PVOID SystemSpecific3)
{
    RTMP_ADAPTER    *pAd = (PRTMP_ADAPTER)FunctionContext;
	PWSC_CTRL		pWpsCtrl = NULL;

	DBGPRINT(RT_DEBUG_TRACE, ("!!! IWSC_T2TimerAction - Become Enrollee !!!\n"));

	if (pAd)
	{
		pWpsCtrl = &pAd->StaCfg.WscControl;
		if (pWpsCtrl)
		{
			RTMPSendWirelessEvent(pAd, IW_IWSC_T2_TIMER_TIMEOUT, NULL, pWpsCtrl->EntryIfIdx, 0);
			IWSC_RoleAction(pAd, WSC_ENROLLEE);
			pAd->Mlme.IWscMachine.CurrState = IWSC_SCAN;			
		}
		else
			DBGPRINT(RT_DEBUG_ERROR, ("!!! IWSC_T2TimerAction - pWpsCtrl is null !!!\n"));		
	}
	else
		DBGPRINT(RT_DEBUG_ERROR, ("!!! IWSC_T2TimerAction - pAd is null !!!\n"));
}


VOID IWSC_EntryTimerAction(
    IN  PVOID SystemSpecific1,
    IN  PVOID FunctionContext,
    IN  PVOID SystemSpecific2,
    IN  PVOID SystemSpecific3)
{
	RTMP_ADAPTER    *pAd = (PRTMP_ADAPTER)FunctionContext;
	PIWSC_INFO		pIWscInfo = NULL;
	
	DBGPRINT(RT_DEBUG_TRACE, ("!!! IWSC_EntryTimerAction !!!\n"));
	if (pAd)
	{
		pIWscInfo = &pAd->StaCfg.IWscInfo;
		if (pIWscInfo)
		{
			pIWscInfo->bIWscEntryTimerRunning = FALSE;
			WscBuildProbeRespIE(pAd, 
								WSC_MSGTYPE_REGISTRAR, 
								pAd->StaCfg.WscControl.WscConfStatus, 
								TRUE, 
								DEV_PASS_ID_SMPBC, 
								pAd->StaCfg.WscControl.WscConfigMethods, 
								BSS0, 
								NULL, 
								0,
								STA_MODE);
			RTMPSendWirelessEvent(pAd, IW_IWSC_ENTRY_TIMER_TIMEOUT, NULL, pAd->StaCfg.WscControl.EntryIfIdx, 0);
		}
		else
			DBGPRINT(RT_DEBUG_ERROR, ("!!! IWSC_EntryTimerAction - pIWscInfo is null !!!\n"));
	}
	else
		DBGPRINT(RT_DEBUG_ERROR, ("!!! IWSC_EntryTimerAction - pAd is null !!!\n"));
}


VOID IWSC_DevQueryAction(
    IN  PVOID SystemSpecific1,
    IN  PVOID FunctionContext,
    IN  PVOID SystemSpecific2,
    IN  PVOID SystemSpecific3)
{
	RTMP_ADAPTER    *pAd = (PRTMP_ADAPTER)FunctionContext;
	PIWSC_INFO		pIWscInfo = NULL;
	
	DBGPRINT(RT_DEBUG_TRACE, ("!!! IWSC_DevQueryAction !!!\n"));
	if (pAd)
	{
		pIWscInfo = &pAd->StaCfg.IWscInfo;		
		if (pIWscInfo)
		{
			if (pIWscInfo->bIWscDevQueryReqTimerRunning)
			{
				pIWscInfo->bIWscDevQueryReqTimerRunning = FALSE;
				IWSC_SendActionFrame(pAd, IWSC_DEV_QUERY_REQUEST);
				RtmpusecDelay(200000); // 200 ms
				IWSC_SendActionFrame(pAd, IWSC_DEV_QUERY_REQUEST);
				RtmpusecDelay(200000); // 200 ms
				IWSC_SendActionFrame(pAd, IWSC_DEV_QUERY_REQUEST);
			}
			if (pIWscInfo->bIWscDevQueryRspTimerRunning)
			{
				pIWscInfo->bIWscDevQueryRspTimerRunning = FALSE;
				IWSC_SendActionFrame(pAd, IWSC_DEV_QUERY_RESPONSE);
			}
		}
		else
			DBGPRINT(RT_DEBUG_ERROR, ("!!! IWSC_DevQueryAction - pIWscInfo is null !!!\n"));
	}
	else
		DBGPRINT(RT_DEBUG_ERROR, ("!!! IWSC_DevQueryAction - pAd is null !!!\n"));
}
/**************************************************** IWSC Timer Function End ****************************************************/


VOID IWSC_MlmeStartAction(RTMP_ADAPTER *pAd, PMLME_QUEUE_ELEM pElem) 
{
	PWSC_CTRL	pWpsCtrl = &pAd->StaCfg.WscControl;
	PIWSC_INFO	pIWscInfo = &pAd->StaCfg.IWscInfo;
	
	/*
		Check IWSC State
	*/
	if (pAd->Mlme.IWscMachine.CurrState != IWSC_IDLE)
	{
		/*
			Stop IWSC
		*/
		IWSC_Stop(pAd, FALSE);
	}	

	RTMPSetTimer(&pIWscInfo->IWscT1Timer, WSC_TWO_MINS_TIME_OUT);
	pIWscInfo->bIWscT1TimerRunning = TRUE;
	WscInitRegistrarPair(pAd, pWpsCtrl, BSS0);
	WscGetRegDataPIN(pAd, pWpsCtrl->WscPinCode, pWpsCtrl);	
	pWpsCtrl->RegData.ReComputePke = 1;
	if (!OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED))
	{
		IWSC_ResetIpContent(pAd);
	}
	pIWscInfo->SmpbcEnrolleeCount = 0;
	pIWscInfo->IWscSmpbcAcceptCount = 0;
	NdisZeroMemory(pWpsCtrl->EntryAddr, MAC_ADDR_LEN);

	RTMP_SEM_LOCK(&pWpsCtrl->WscPeerListSemLock);
	WscClearPeerList(&pWpsCtrl->WscPeerList);
	RTMP_SEM_UNLOCK(&pWpsCtrl->WscPeerListSemLock);
	
	RTMP_SEM_LOCK(&pWpsCtrl->WscConfiguredPeerListSemLock);
	WscClearPeerList(&pWpsCtrl->WscConfiguredPeerList);
	RTMP_SEM_UNLOCK(&pWpsCtrl->WscConfiguredPeerListSemLock);

	if (pWpsCtrl->WscMode == WSC_SMPBC_MODE)
	{				
		if (pWpsCtrl->WscConfMode == WSC_REGISTRAR)
		{
			RTMPSetTimer(&pIWscInfo->IWscEntryTimer, IWSC_ENTRY_TIME_OUT);
			pIWscInfo->bIWscEntryTimerRunning = TRUE;
			IWSC_RoleAction(pAd, WSC_REGISTRAR);
			IWSC_SendActionFrame(pAd, IWSC_SEL_REG_START_NOTITY);
			pAd->Mlme.IWscMachine.CurrState = IWSC_START;
		}
		else
		{
			IWSC_RoleAction(pAd, WSC_ENROLLEE);
			pAd->Mlme.IWscMachine.CurrState = IWSC_SCAN;
		}
	}
	else
	{
#ifdef IWSC_TEST_SUPPORT
		if (pIWscInfo->IWscConfMode == WSC_ENROLLEE)
		{
			IWSC_RoleAction(pAd, WSC_ENROLLEE);
			pAd->Mlme.IWscMachine.CurrState = IWSC_SCAN;
		}
		else
#endif // IWSC_TEST_SUPPORT //
		{
			IWSC_RoleAction(pAd, WSC_REGISTRAR);
			if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED))
			{
				/*
					[A member of IBSS] - Send Selected Registrar Start Notification (multicast action frame)
				*/
				IWSC_SendActionFrame(pAd, IWSC_SEL_REG_START_NOTITY);
			}
			else
			{
				if ((pAd->StaCfg.IWscInfo.bSinglePIN == FALSE)
#ifdef IWSC_TEST_SUPPORT
					&& (pIWscInfo->IWscConfMode == WSC_DISABLE)
#endif // IWSC_TEST_SUPPORT //					
					)
				{
					/*
						[!A member of IBSS]
					*/
					UCHAR	RandomTime;

					RandomTime = RandomByte(pAd) & 0x3C;
					if (RandomTime > 60)
						RandomTime = 60;
					else if (RandomTime < 20)
						RandomTime = 20;
					DBGPRINT(RT_DEBUG_TRACE, ("RandomTime = %d\n", RandomTime));
					RTMPSetTimer(&pIWscInfo->IWscT2Timer, (RandomTime*100));
					pIWscInfo->bIWscT2TimerRunning = TRUE;
				}
			}
			pAd->Mlme.IWscMachine.CurrState = IWSC_START;
		}
	}	
	pWpsCtrl->bWscTrigger = TRUE;
}


VOID IWSC_MlmeStopAction(RTMP_ADAPTER *pAd, PMLME_QUEUE_ELEM pElem) 
{
	PWSC_CTRL pWpsCtrl = &pAd->StaCfg.WscControl;

	if (pAd->StaCfg.IWscInfo.bSinglePIN &&
		pAd->StaCfg.IWscInfo.bDoNotStop)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("SinglePIN registrar now, keep going!!\n"));
		pWpsCtrl->WscConfMode = WSC_REGISTRAR;
		pWpsCtrl->RegData.ReComputePke = 1;
		pWpsCtrl->bWscTrigger = TRUE;
		pWpsCtrl->WscState = WSC_STATE_LINK_UP;
		pWpsCtrl->WscStatus = STATUS_WSC_LINK_UP;
		return;
	}

#ifdef IWSC_TEST_SUPPORT
	pAd->StaCfg.IWscInfo.IWscConfMode = WSC_DISABLE;
#endif // IWSC_TEST_SUPPORT //

	/*
		Check IWSC State
	*/
	if (pAd->Mlme.IWscMachine.CurrState != IWSC_IDLE)
	{
		/*
			Stop IWSC
		*/
		if ((pWpsCtrl->WscConfMode == WSC_REGISTRAR) && OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED))
		{
			/*
				Send Selected Registrar Finish Notification (multicast action frame)
			*/
			IWSC_Stop(pAd, TRUE);
		}
		else
			IWSC_Stop(pAd, FALSE);		
	}

}


VOID IWSC_ScanDoneAction(RTMP_ADAPTER *pAd, PMLME_QUEUE_ELEM pElem) 
{
	PWSC_CTRL	pWpsCtrl = &pAd->StaCfg.WscControl;
	UCHAR		RandomTime;

	if (pAd->Mlme.IWscMachine.CurrState != IWSC_SCAN)
	{
		if (pAd->Mlme.IWscMachine.CurrState != IWSC_IDLE)
			DBGPRINT(RT_DEBUG_TRACE, ("IWSC_ScanDoneAction:: CurrState = %ld\n", pAd->Mlme.IWscMachine.CurrState));
		return;
	}

	DBGPRINT(RT_DEBUG_TRACE, ("-----> IWSC_ScanDoneAction\n"));	
	
	if (pWpsCtrl->WscMode == WSC_PBC_MODE)
	{		
		if (WscPBCExec(pAd, FALSE, pWpsCtrl) == FALSE)
		{
			if (pWpsCtrl->WscPBCBssCount > 1)
			{
				IWSC_Stop(pAd, FALSE);
			}
			else
			{
#ifdef IWSC_TEST_SUPPORT
				if (pAd->StaCfg.IWscInfo.IWscConfMode != WSC_ENROLLEE)
#endif /* IWSC_TEST_SUPPORT */
				{
					IWSC_RoleAction(pAd, WSC_REGISTRAR);
					RandomTime = RandomByte(pAd) & 0x3C;
					if (RandomTime > 60)
						RandomTime = 60;
					else if (RandomTime < 20)
						RandomTime = 20;
					DBGPRINT(RT_DEBUG_TRACE, ("RandomTime = %d\n", RandomTime));
					RTMPSetTimer(&pAd->StaCfg.IWscInfo.IWscT2Timer, (RandomTime*100));
					pAd->StaCfg.IWscInfo.bIWscT2TimerRunning = TRUE;
					pAd->Mlme.IWscMachine.CurrState = IWSC_START;
				}
#ifdef IWSC_TEST_SUPPORT
				else
				{
					RTMPSetTimer(&pWpsCtrl->WscScanTimer, 1000);
		        	pWpsCtrl->WscScanTimerRunning = TRUE;
				}
#endif /* IWSC_TEST_SUPPORT */
			}
		}
		else
		{
			BOOLEAN bCancel;
			if (pWpsCtrl->WscPBCTimerRunning)			
				RTMPCancelTimer(&pWpsCtrl->WscPBCTimer, &bCancel);
			pAd->Mlme.IWscMachine.CurrState = IWSC_START;
			pAd->StaCfg.IWscInfo.bSendEapolStart = TRUE;
		}
	}
	else if (pWpsCtrl->WscMode == WSC_SMPBC_MODE)
	{
		WscPBCExec(pAd, FALSE, pWpsCtrl);
#ifdef IWSC_TEST_SUPPORT
		if (pAd->StaCfg.IWscInfo.bIwscSmpbcScanningOnly)
		{
			RTMPSetTimer(&pWpsCtrl->WscScanTimer, 1000);
        	pWpsCtrl->WscScanTimerRunning = TRUE;
			DBGPRINT(RT_DEBUG_TRACE, ("<----- IWSC_ScanDoneAction\n"));
			return;
		}
#endif /* IWSC_TEST_SUPPORT */
		if (pWpsCtrl->WscPBCBssCount == 1)
		{
			pWpsCtrl->WscStatus = WSC_STATE_START;
			pAd->Mlme.IWscMachine.CurrState = IWSC_START;
			pAd->StaCfg.IWscInfo.bSendEapolStart = TRUE;
		}
		else if (pWpsCtrl->WscPBCBssCount == 0)
		{
			RTMPSetTimer(&pWpsCtrl->WscScanTimer, 1000);
        	pWpsCtrl->WscScanTimerRunning = TRUE;
		}
		else
			IWSC_Stop(pAd, FALSE);
	}
	else
	{
		{
#ifdef IWSC_TEST_SUPPORT
			if (pAd->StaCfg.IWscInfo.IWscConfMode == WSC_ENROLLEE)
			{
				WscScanExec(pAd);
			}
			else
#endif // IWSC_TEST_SUPPORT //
			{
				IWSC_RoleAction(pAd, WSC_REGISTRAR);		
				RandomTime = RandomByte(pAd) & 0x3C;
				if (RandomTime > 60)
					RandomTime = 60;
				else if (RandomTime < 20)
					RandomTime = 20;
				DBGPRINT(RT_DEBUG_TRACE, ("RandomTime = %d\n", RandomTime));
				RTMPSetTimer(&pAd->StaCfg.IWscInfo.IWscT2Timer, (RandomTime*100));
				pAd->StaCfg.IWscInfo.bIWscT2TimerRunning = TRUE;
				pAd->Mlme.IWscMachine.CurrState = IWSC_START;
			}
		}
	}

	DBGPRINT(RT_DEBUG_TRACE, ("<----- IWSC_ScanDoneAction\n"));

}


VOID IWSC_ReConnectAction(RTMP_ADAPTER *pAd, PMLME_QUEUE_ELEM pElem) 
{
	PWSC_CTRL	pWpsCtrl = &pAd->StaCfg.WscControl;
	
    DBGPRINT(RT_DEBUG_TRACE, ("-----> %s\n", __FUNCTION__));
	MlmeEnqueue(pAd,
				MLME_CNTL_STATE_MACHINE,
				OID_802_11_SSID,
				sizeof(NDIS_802_11_SSID),
				(VOID *)&pWpsCtrl->WscSsid, 0);
	RTMP_MLME_HANDLER(pAd);
	DBGPRINT(RT_DEBUG_TRACE, ("<----- %s\n", __FUNCTION__));
}


VOID IWSC_BuildSelRegStartNotification(RTMP_ADAPTER *pAd, UCHAR *pOutBuf, USHORT *pIeLen)
{
	UCHAR *Data = NULL, *pData;
	INT Len = 0, templen = 0;
	WSC_CTRL *pWpsCtrl = &pAd->StaCfg.WscControl;
	WSC_REG_DATA *pReg = (PWSC_REG_DATA) &pWpsCtrl->RegData;
	USHORT tempVal = 0, ConfigError = htons(0);
	WSC_IE_HEADER ieHdr;

	/* allocate memory */
	os_alloc_mem(NULL, (UCHAR **)&Data, 512);
	if (Data == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s: Allocate memory fail!!!\n", __FUNCTION__));
		return;
	}
	
	DBGPRINT(RT_DEBUG_TRACE, ("-----> IWSC_BuildSelRegStartNotification\n"));

	// WSC IE HEader
	ieHdr.elemId = 221;
	ieHdr.length = 4;
	ieHdr.oui[0] = 0x00; 
	ieHdr.oui[1] = 0x50; 
	ieHdr.oui[2] = 0xF2; 
	ieHdr.oui[3] = 0x10;

	pData = (PUCHAR) &Data[0];
	Len = 0;
	
	/* Version */
	templen = AppendWSCTLV(WSC_ID_VERSION, pData, &pReg->SelfInfo.Version, 0);
	pData += templen;
	Len   += templen;

	/* Request Type */
	tempVal = WSC_MSGTYPE_REGISTRAR;
    templen = AppendWSCTLV(WSC_ID_REQ_TYPE, pData, (UINT8 *)&tempVal, 0);
	pData += templen;
	Len   += templen;

	/* Config method */
	tempVal = htons(0x008c);
	templen = AppendWSCTLV(WSC_ID_CONFIG_METHODS, pData, (UINT8 *)&tempVal, 0);
	pData += templen;
	Len   += templen;

	/* UUID */
	templen = AppendWSCTLV(WSC_ID_UUID_E, pData, pReg->SelfInfo.Uuid, 0);
	pData += templen;
	Len   += templen;

	/* Primary device type */
	templen = AppendWSCTLV(WSC_ID_PRIM_DEV_TYPE, pData, pReg->SelfInfo.PriDeviceType, 0);
	pData += templen;
	Len   += templen;

	/* RF band, shall change based on current channel */
    templen = AppendWSCTLV(WSC_ID_RF_BAND, pData, &pReg->SelfInfo.RfBand, 0);
	pData += templen;
	Len   += templen;

	/* Config error */
	templen = AppendWSCTLV(WSC_ID_CONFIG_ERROR, pData, (UINT8 *)&ConfigError, 0);
	pData += templen;
	Len   += templen;

	/* Device Password ID */
	if (pWpsCtrl->WscMode == WSC_PIN_MODE)
		tempVal = DEV_PASS_ID_PIN;//cpu2be16(DEV_PASS_ID_PIN);
	else
		tempVal = cpu2be16(DEV_PASS_ID_PBC);
	templen = AppendWSCTLV(WSC_ID_DEVICE_PWD_ID, pData, (UINT8 *)&tempVal, 0);
	pData += templen;
	Len   += templen;

	/* MAC address */
	templen = AppendWSCTLV(WSC_ID_MAC_ADDR, pData, pReg->SelfInfo.MacAddr, 0);
	pData += templen;
	Len   += templen;

	ieHdr.length = ieHdr.length + Len;

	RTMPMoveMemory(pOutBuf, &ieHdr, sizeof(WSC_IE_HEADER));
	RTMPMoveMemory(pOutBuf + sizeof(WSC_IE_HEADER), Data, Len);
	*pIeLen = (USHORT)(sizeof(WSC_IE_HEADER) + Len);

	if (Data != NULL)
		os_free_mem(NULL, Data);
	
	DBGPRINT(RT_DEBUG_TRACE, ("<----- IWSC_BuildSelRegStartNotification\n"));
}


VOID	IWSC_BuildSelRegFinishNotification(
	IN  PRTMP_ADAPTER	pAd,
	OUT	PUCHAR			pOutBuf,
	OUT	PUSHORT			pIeLen)
{
//	UCHAR 			Data[512];
	UCHAR 			*Data = NULL;
	PUCHAR			pData;
	INT				Len = 0, templen = 0;
	PWSC_CTRL		pWpsCtrl = &pAd->StaCfg.WscControl;
	PWSC_REG_DATA	pReg = (PWSC_REG_DATA) &pWpsCtrl->RegData;
	WSC_IE_HEADER 	ieHdr;
	
	/* allocate memory */
	os_alloc_mem(NULL, (UCHAR **)&Data, 512);
	if (Data == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s: Allocate memory fail!!!\n", __FUNCTION__));
		return;
	}
	
	DBGPRINT(RT_DEBUG_TRACE, ("-----> IWSC_BuildSelRegFinishNotification\n"));

	// WSC IE HEader
	ieHdr.elemId = 221;
	ieHdr.length = 4;
	ieHdr.oui[0] = 0x00; 
	ieHdr.oui[1] = 0x50; 
	ieHdr.oui[2] = 0xF2; 
	ieHdr.oui[3] = 0x10;
	
	pData = (PUCHAR) &Data[0];
	Len = 0;
	
	/* Version */
	templen = AppendWSCTLV(WSC_ID_VERSION, pData, &pReg->SelfInfo.Version, 0);
	pData += templen;
	Len   += templen;

	/* Simple Config State */
	templen = AppendWSCTLV(WSC_ID_SC_STATE, pData, (UINT8 *)&pWpsCtrl->WscConfStatus, 0);
	pData += templen;
	Len   += templen;

	templen = AppendWSCTLV(WSC_ID_MAC_ADDR, pData, &pAd->CurrentAddress[0], 0);
	pData += templen;
	Len   += templen;

	ieHdr.length = ieHdr.length + Len;

	RTMPMoveMemory(pOutBuf, &ieHdr, sizeof(WSC_IE_HEADER));
	RTMPMoveMemory(pOutBuf + sizeof(WSC_IE_HEADER), Data, Len);
	*pIeLen = sizeof(WSC_IE_HEADER) + Len;

	if (Data != NULL)
		os_free_mem(NULL, Data);
	
	DBGPRINT(RT_DEBUG_TRACE, ("<----- IWSC_BuildSelRegFinishNotification\n"));
}


/*
	IWSC Public Action Frame Format
	-----------------------------------------------------------------------------------------------
	Field			Size		Value
	-----------------------------------------------------------------------------------------------
	Category		1 Octet		0x04
	-----------------------------------------------------------------------------------------------
	Action field	1 Octet		0xDD
	-----------------------------------------------------------------------------------------------
	OUI				3 Octets	0x50 0x6F 0x9A
	-----------------------------------------------------------------------------------------------
	OUI type		1 Octet		0x10
	-----------------------------------------------------------------------------------------------
	OUI Subtype		1 Octet		0 - Selected Registrar Start Notification
								1 - Selected Registrar Finish Notification
								2 - Device Query Request
								3 - Device Query Response
								4 ~ 255 - Reserved
	-----------------------------------------------------------------------------------------------
	Dialog Token	1 Octet		nonzero value (to identify the request/response transaction
	-----------------------------------------------------------------------------------------------
	Elements		variable	IWSC IE 
								(It has the Element ID(0xDD), Length, IWSC OUI(0x00 0x50 0xF2 0x10)
	-----------------------------------------------------------------------------------------------
*/
VOID IWSC_SendActionFrame(RTMP_ADAPTER *pAd, UCHAR FrameType)
{
	HEADER_802_11	ActHdr;
	PUCHAR			pOutBuffer = NULL;
	ULONG			FrameLen = 0;
	UCHAR			Category = CATEGORY_PUBLIC;
	UCHAR			Action = 9;
	PUCHAR			pWscBuf = NULL;
	USHORT			WscIeLen = 0;
	ULONG 			WscTmpLen = 0;
	PIWSC_INFO		pIWscInfo = &pAd->StaCfg.IWscInfo;
	NDIS_STATUS		NStatus;

	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  
	if (NStatus != NDIS_STATUS_SUCCESS) 
	{
		DBGPRINT(RT_DEBUG_TRACE,("IWSC_SendStartNotification:: allocate memory failed \n"));
		return;
	}

	if (FrameType == IWSC_DEV_QUERY_RESPONSE)
		ActHeaderInit(pAd, &ActHdr, pIWscInfo->IWscDevQueryReqMacAddr, pAd->CurrentAddress, pAd->CommonCfg.Bssid);
	else
		ActHeaderInit(pAd, &ActHdr, BROADCAST_ADDR, pAd->CurrentAddress, pAd->CommonCfg.Bssid);

	if (pIWscInfo->DialogToken == 0)
		pIWscInfo->DialogToken = 1;
	else
		pIWscInfo->DialogToken++;
	
	MakeOutgoingFrame(pOutBuffer,				&FrameLen,
					sizeof(HEADER_802_11),		&ActHdr,
					1,							&Category,
					1,							&Action,
					4,							IWSC_ACTION_OUI,
					1,							&FrameType,
					1,							&pIWscInfo->DialogToken,
					END_OF_ARGS);

	os_alloc_mem(NULL, &pWscBuf, 512);
	if( pWscBuf != NULL)
	{
		NdisZeroMemory(pWscBuf, 512);
		if (FrameType == IWSC_SEL_REG_START_NOTITY)
			IWSC_BuildSelRegStartNotification(pAd, pWscBuf, &WscIeLen);
		else if (FrameType == IWSC_SEL_REG_FINISH_NOTITY)
			IWSC_BuildSelRegFinishNotification(pAd, pWscBuf, &WscIeLen);
		else if (FrameType == IWSC_DEV_QUERY_REQUEST)
			IWSC_BuildDevQueryFrame(pAd, pWscBuf, &WscIeLen);
		else if (FrameType == IWSC_DEV_QUERY_RESPONSE)
			IWSC_BuildDevQueryFrame(pAd, pWscBuf, &WscIeLen);

		MakeOutgoingFrame(pOutBuffer + FrameLen, &WscTmpLen,
						  WscIeLen,				 pWscBuf,
						  END_OF_ARGS);

		FrameLen += WscTmpLen;
		os_free_mem(NULL, pWscBuf);
	}
	else
		DBGPRINT(RT_DEBUG_WARN, ("%s:: WscBuf Allocate failed!\n", __FUNCTION__));

	MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
	MlmeFreeMemory(pAd, pOutBuffer);
}


VOID IWSC_PeerAction(RTMP_ADAPTER *pAd, PMLME_QUEUE_ELEM pElem) 
{	
	UCHAR	Token = pElem->Msg[LENGTH_802_11+7];
	UCHAR	FrameType = pElem->Msg[LENGTH_802_11+6];
	PUCHAR	pData = pElem->Msg+(LENGTH_802_11+8);
	PIWSC_INFO	pIWscInfo = &pAd->StaCfg.IWscInfo;
	INT		Len = 0;
	INT		TotalLen = (INT)pElem->MsgLen - (LENGTH_802_11+8);
	USHORT	RegDPID;
	USHORT	RegCfgMethods;
	UINT8	ReqType = 0;
	UCHAR	PeerMAC[MAC_ADDR_LEN];

	DBGPRINT(RT_DEBUG_TRACE, ("-----> IWSC_PeerAction\n"));

	/*
		Marvell STA always send same non-zero Token value, thus we don't need to check Token value here.
	*/
	{
		/*
			IWSC Element may not be first one element in Action frame.
		*/
		pIWscInfo->PeerDialogToken = Token;
		for (;;)
		{
			Len = (INT)*(pData + 1);
			/*hex_dump("sn - pData", pData, Len+2);*/
			pData = pData + 2;
			if (NdisEqualMemory(pData, IWSC_OUI, 4))
			{
				Len -= 4; // OUI Length
				pData = pData + 4;
				break;
			}

			TotalLen -= (Len + 2); // 2: ID + Len
			if (TotalLen <= 0)
			{
				DBGPRINT(RT_DEBUG_TRACE, ("No IWSC IE!\n<----- IWSC_PeerAction\n"));
				return;
			}
			pData = pData + Len;
		}

		hex_dump("pData", pData, Len);
		while (Len > 0)
		{
			WSC_IE	WscIE;
			PWSC_IE	pWscIE;
			NdisMoveMemory(&WscIE, pData, sizeof(WSC_IE));
			/* Check for WSC IEs */
			pWscIE = &WscIE;

			if (be2cpu16(pWscIE->Type) == WSC_ID_REQ_TYPE)
			{
				ReqType = *(pData + 4);
			}

			if (be2cpu16(pWscIE->Type) == WSC_ID_SEL_REG_CFG_METHODS)
			{
				NdisMoveMemory(&RegCfgMethods, pData + 4, sizeof(USHORT));
				RegCfgMethods = be2cpu16(RegCfgMethods);
			}

			if (be2cpu16(pWscIE->Type) == WSC_ID_DEVICE_PWD_ID)
			{
				NdisMoveMemory(&RegDPID, pData + 4, sizeof(USHORT));
				RegDPID = be2cpu16(RegDPID);
			}
			
			if (be2cpu16(pWscIE->Type) == WSC_ID_MAC_ADDR)
			{
				RTMPMoveMemory(PeerMAC, pData + 4, MAC_ADDR_LEN);
			}

			/*
				Set the offset
				Since Type and Length are both short type, we need to offset 4, not 2
			*/
			pData += (be2cpu16(pWscIE->Length) + 4);
			Len   -= (be2cpu16(pWscIE->Length) + 4);
		}		

		if (FrameType == IWSC_SEL_REG_START_NOTITY)
		{
			DBGPRINT(RT_DEBUG_TRACE, ("Receive SelRegStartNotification from %02X:%02X:%02X:%02X:%02X:%02X!\n",
						PeerMAC[0], PeerMAC[1], PeerMAC[2], PeerMAC[3], PeerMAC[4], PeerMAC[5]));
			pIWscInfo->bSelRegStart = TRUE;
			RTMPMoveMemory(pIWscInfo->RegMacAddr, PeerMAC, MAC_ADDR_LEN);
			WscBuildProbeRespIE(pAd, 
								WSC_MSGTYPE_IWSC_NOTIFIER, 
								pAd->StaCfg.WscControl.WscConfStatus, 
								TRUE, 
								RegDPID, 
								RegCfgMethods, 
								0,
								NULL,
								0,
								STA_MODE);
		}
		else if (FrameType == IWSC_SEL_REG_FINISH_NOTITY)
		{
			DBGPRINT(RT_DEBUG_TRACE, ("Receive SelRegFinishNotification from %02X:%02X:%02X:%02X:%02X:%02X!\n",
						PeerMAC[0], PeerMAC[1], PeerMAC[2], PeerMAC[3], PeerMAC[4], PeerMAC[5]));
			if (NdisEqualMemory(pIWscInfo->RegMacAddr, PeerMAC, MAC_ADDR_LEN))
			{
				pIWscInfo->bSelRegStart = FALSE;
				NdisZeroMemory(pIWscInfo->RegMacAddr, MAC_ADDR_LEN);
			}
			if (pAd->StaCfg.WscControl.bWscTrigger == FALSE)
				pAd->StaCfg.WpsIEProbeResp.ValueLen = 0;
		}
		else if (FrameType == IWSC_DEV_QUERY_REQUEST)
		{			
			UCHAR RandomVal = RandomByte(pAd);
			MAC_TABLE_ENTRY *pEntry;
			
			DBGPRINT(RT_DEBUG_TRACE, ("Receive Device Query Request from %02X:%02X:%02X:%02X:%02X:%02X!\n",
						PeerMAC[0], PeerMAC[1], PeerMAC[2], PeerMAC[3], PeerMAC[4], PeerMAC[5]));			
			if (RandomVal < 20)
				RandomVal = 20;
			if (RandomVal > 50)
				RandomVal = 50;
			NdisMoveMemory(pIWscInfo->IWscDevQueryReqMacAddr, PeerMAC, MAC_ADDR_LEN);
			RTMPSetTimer(&pIWscInfo->IWscDevQueryTimer, RandomVal*10);
			pIWscInfo->bIWscDevQueryRspTimerRunning = TRUE;

#ifdef IWSC_TEST_SUPPORT
			if (pAd->StaCfg.IWscInfo.bBlockConnection)
				return;
#endif /* IWSC_TEST_SUPPORT */

			pEntry = MacTableLookup(pAd, PeerMAC);

			if (pEntry == NULL)
			{
				// Another adhoc joining, add to our MAC table. 
				pEntry = MacTableInsertEntry(pAd, PeerMAC, &pAd->StaCfg.wdev, BSS0, OPMODE_STA, FALSE);
			}

			if (pEntry)
			{
#ifdef ADHOC_WPA2PSK_SUPPORT
                //Adhoc support WPA2PSK by Eddy
                if ((pAd->StaCfg.wdev.AuthMode == Ndis802_11AuthModeWPA2PSK) 
                    && (pEntry->WPA_Authenticator.WpaState < AS_INITPSK))                      
                {
    				INT len, i;
					BOOLEAN bHigherMAC = FALSE;

            		pEntry->PortSecured = WPA_802_1X_PORT_NOT_SECURED;
                    NdisZeroMemory(&pEntry->WPA_Supplicant.ReplayCounter, LEN_KEY_DESC_REPLAY);                        
                    NdisZeroMemory(&pEntry->WPA_Authenticator.ReplayCounter, LEN_KEY_DESC_REPLAY);                        
                    pEntry->WPA_Authenticator.WpaState = AS_INITPSK;
                    pEntry->WPA_Supplicant.WpaState = AS_INITPSK;
            		pEntry->EnqueueEapolStartTimerRunning = EAPOL_START_PSK;

					// collapse into the ADHOC network which has bigger BSSID value.
					for (i = 0; i < 6; i++)
					{
						if (PeerMAC[i] > pAd->CurrentAddress[i])
						{
							bHigherMAC = TRUE;
							break;
						}
						else if (PeerMAC[i] < pAd->CurrentAddress[i])
							break;
					}

					hex_dump("IWSC_PeerAction:: PeerMAC", PeerMAC, MAC_ADDR_LEN);
					hex_dump("IWSC_PeerAction:: pAd->CurrentAddress", pAd->CurrentAddress, MAC_ADDR_LEN);
					pEntry->bPeerHigherMAC = bHigherMAC;
					if (pEntry->bPeerHigherMAC == FALSE)
					{
						/*
							My MAC address is higher than peer's MAC address.
						*/
						DBGPRINT(RT_DEBUG_TRACE, ("%s - EnqueueStartForPSKTimer.\n", __FUNCTION__));
            			RTMPSetTimer(&pEntry->EnqueueStartForPSKTimer, ENQUEUE_EAPOL_START_TIMER);
						pEntry->bPeerHigherMAC = FALSE;
					}
                }
				else 
#endif // ADHOC_WPA2PSK_SUPPORT //
				{
					if (pAd->StaCfg.wdev.AuthMode < Ndis802_11AuthModeWPA)
						pEntry->PortSecured = WPA_802_1X_PORT_SECURED;
                }
			}
		}
		else if (FrameType == IWSC_DEV_QUERY_RESPONSE)
		{
			MAC_TABLE_ENTRY *pEntry;
			
			DBGPRINT(RT_DEBUG_TRACE, ("Receive Device Query Response from %02X:%02X:%02X:%02X:%02X:%02X!\n",
						PeerMAC[0], PeerMAC[1], PeerMAC[2], PeerMAC[3], PeerMAC[4], PeerMAC[5]));


#ifdef IWSC_TEST_SUPPORT
			if (pAd->StaCfg.IWscInfo.bBlockConnection)
				return;
#endif /* IWSC_TEST_SUPPORT */

			pEntry = MacTableLookup(pAd, PeerMAC);

			if (pEntry == NULL)
			{
				// Another adhoc joining, add to our MAC table. 
				pEntry = MacTableInsertEntry(pAd, PeerMAC, &pAd->StaCfg.wdev, BSS0, OPMODE_STA, FALSE);
			}

			if (pEntry)
			{
#ifdef ADHOC_WPA2PSK_SUPPORT
                //Adhoc support WPA2PSK by Eddy
                if ((pAd->StaCfg.wdev.AuthMode == Ndis802_11AuthModeWPA2PSK) 
                    && (pEntry->WPA_Authenticator.WpaState < AS_INITPSK))                      
                {
    				INT               len, i;
					BOOLEAN bHigherMAC = FALSE;

            		pEntry->PortSecured = WPA_802_1X_PORT_NOT_SECURED;
                    NdisZeroMemory(&pEntry->WPA_Supplicant.ReplayCounter, LEN_KEY_DESC_REPLAY);                        
                    NdisZeroMemory(&pEntry->WPA_Authenticator.ReplayCounter, LEN_KEY_DESC_REPLAY);                        
                    pEntry->WPA_Authenticator.WpaState = AS_INITPSK;
                    pEntry->WPA_Supplicant.WpaState = AS_INITPSK;
            		pEntry->EnqueueEapolStartTimerRunning = EAPOL_START_PSK;

					// collapse into the ADHOC network which has bigger BSSID value.
					for (i = 0; i < 6; i++)
					{
						if (PeerMAC[i] > pAd->CurrentAddress[i])
						{
							bHigherMAC = TRUE;
							break;
						}
						else if (PeerMAC[i] < pAd->CurrentAddress[i])
							break;
					}

					hex_dump("IWSC_PeerAction:: PeerMAC", PeerMAC, MAC_ADDR_LEN);
					hex_dump("IWSC_PeerAction:: pAd->CurrentAddress", pAd->CurrentAddress, MAC_ADDR_LEN);
					pEntry->bPeerHigherMAC = bHigherMAC;
					if (pEntry->bPeerHigherMAC == FALSE)
					{
						/*
							My MAC address is higher than peer's MAC address.
						*/
						DBGPRINT(RT_DEBUG_TRACE, ("%s - EnqueueStartForPSKTimer.\n", __FUNCTION__));
            			RTMPSetTimer(&pEntry->EnqueueStartForPSKTimer, ENQUEUE_EAPOL_START_TIMER);
						pEntry->bPeerHigherMAC = FALSE;
					}
                }
				else 
#endif // ADHOC_WPA2PSK_SUPPORT //
				{
					if (pAd->StaCfg.wdev.AuthMode < Ndis802_11AuthModeWPA)
						pEntry->PortSecured = WPA_802_1X_PORT_SECURED;
                }
			}
		}
		else
		{
			DBGPRINT(RT_DEBUG_TRACE, ("Unknow Frame Type!\n"));
		}
	}
	DBGPRINT(RT_DEBUG_TRACE, ("<----- IWSC_PeerAction\n"));
}


VOID IWSC_PeerProbeRequest(RTMP_ADAPTER *pAd, PMLME_QUEUE_ELEM pElem)
{
	PWSC_CTRL	pWscCtrl = &pAd->StaCfg.WscControl;
	PIWSC_INFO	pIWscInfo = &pAd->StaCfg.IWscInfo;
	USHORT 		PeerConfigMethod = 0;
	USHORT 		ConfigMethod = 0;
	BOOLEAN		Cancelled;
	USHORT		DPID = 0;

	if (pWscCtrl->WscConfMode != WSC_REGISTRAR)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("IWSC_PeerProbeRequest:: Not Registrar now(WscConfMode = %d)\n"
							, pWscCtrl->WscConfMode));
		return;
	}

	DBGPRINT(RT_DEBUG_TRACE, ("<----- IWSC_PeerProbeRequest\n"));
	
	NdisMoveMemory(&PeerConfigMethod, pElem->Msg, pElem->MsgLen);

	if (pIWscInfo->bIWscT2TimerRunning)
	{
		RTMPCancelTimer(&pIWscInfo->IWscT2Timer, &Cancelled);
		pIWscInfo->bIWscT2TimerRunning = FALSE;
	}
	
	if (pWscCtrl->WscMode == WSC_PIN_MODE)
	{
		if (PeerConfigMethod & WPS_CONFIG_METHODS_KEYPAD)
		{
			if (pIWscInfo->bSinglePIN == FALSE)
			{
				pWscCtrl->WscEnrolleePinCode = WscRandomGeneratePinCode(pAd, BSS0);
				pWscCtrl->WscEnrolleePinCodeLen = 8;
				pWscCtrl->WscPinCodeLen = 8;
				pWscCtrl->WscStatus = STATUS_WSC_IBSS_NEW_RANDOM_PIN;
			}
			else
				pWscCtrl->WscStatus = STATUS_WSC_IBSS_FIXED_PIN;
			
			WscGetRegDataPIN(pAd, pWscCtrl->WscEnrolleePinCode, pWscCtrl);
			DBGPRINT(RT_DEBUG_TRACE, ("IWSC_PeerProbeRequest:: WscPinCode = %08d\n", pWscCtrl->WscPinCode));
		}
		else 
		{
			if (pAd->StaCfg.IWscInfo.bLimitedUI)
			{
				DBGPRINT(RT_DEBUG_ERROR, 
					("IWSC_PeerProbeRequest:: Both devices are limited UI. Shall we change to use PBC mode?\n"));
				IWSC_Stop(pAd, FALSE);
				return;
			}
			else
			{
				pWscCtrl->WscPinCode = 0;
				pWscCtrl->WscStatus = STATUS_WSC_WAIT_PIN_CODE;
				DBGPRINT(RT_DEBUG_TRACE, ("IWSC_PeerProbeRequest:: Please input Peer's PIN\n"));
			}
		}
		if (PeerConfigMethod & WPS_CONFIG_METHODS_KEYPAD)
		{
			DPID = DEV_PASS_ID_REG;
			ConfigMethod = WPS_CONFIG_METHODS_DISPLAY;
		}
		else
		{
			DPID = DEV_PASS_ID_PIN;
			ConfigMethod = (WPS_CONFIG_METHODS_DISPLAY | WPS_CONFIG_METHODS_KEYPAD);
		}
		
		AsicDisableSync(pAd);
		WscBuildBeaconIE(pAd, 
						pWscCtrl->WscConfStatus, 
						TRUE, 
						DPID,
						ConfigMethod, 
						BSS0, 
						NULL, 
						0,
						STA_MODE);
		MakeIbssBeacon(pAd);        /* re-build BEACON frame */
		AsicEnableIbssSync(pAd);    /* copy BEACON frame to on-chip memory */
		WscBuildProbeRespIE(pAd, 
						WSC_MSGTYPE_REGISTRAR, 
						pWscCtrl->WscConfStatus, 
						TRUE, 
						DPID, 
						ConfigMethod, 
						BSS0, 
						NULL, 
						0,
						STA_MODE);
	}
	pAd->Mlme.IWscMachine.CurrState = IWSC_WAIT_JOIN;

	DBGPRINT(RT_DEBUG_TRACE, ("-----> IWSC_PeerProbeRequest\n"));
	return;
}


VOID IWSC_PeerProbeResponse(RTMP_ADAPTER *pAd, PMLME_QUEUE_ELEM pElem)
{
	PWSC_CTRL	pWscCtrl = &pAd->StaCfg.WscControl;
	PIWSC_INFO	pIWscInfo = &pAd->StaCfg.IWscInfo;
	USHORT 		PeerConfigMethod = 0;
	BOOLEAN		Cancelled;

	if (pWscCtrl->WscConfMode != WSC_ENROLLEE)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("IWSC_PeerProbeResponse:: Not Enrollee now(WscConfMode = %d)\n"
							, pWscCtrl->WscConfMode));
		return;
	}

	DBGPRINT(RT_DEBUG_TRACE, ("<----- IWSC_PeerProbeResponse\n"));

	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))
	{
		RTMPCancelTimer(&pAd->MlmeAux.ScanTimer,	   &Cancelled);
		pAd->MlmeAux.Channel = 0;
		// Change back to original channel in case of doing scan
		AsicSwitchChannel(pAd, pAd->CommonCfg.Channel, FALSE);
		AsicLockChannel(pAd, pAd->CommonCfg.Channel);

		// Resume MSDU which is turned off durning scan
		RTMPResumeMsduTransmission(pAd);

		pAd->Mlme.CntlMachine.CurrState    = CNTL_IDLE;
		pAd->Mlme.SyncMachine.CurrState    = SYNC_IDLE;
		DBGPRINT(RT_DEBUG_TRACE, ("Cancel Scan Timer\n"));
	}
	
	NdisMoveMemory(&PeerConfigMethod, pElem->Msg, pElem->MsgLen);

	if (pAd->StaCfg.IWscInfo.bLimitedUI)
	{
		if (PeerConfigMethod & WPS_CONFIG_METHODS_KEYPAD)
		{
			if (pIWscInfo->bSinglePIN == FALSE)
			{
				pWscCtrl->WscEnrolleePinCode = WscRandomGeneratePinCode(pAd, BSS0);
				pWscCtrl->WscEnrolleePinCodeLen = 8;
				pWscCtrl->WscPinCodeLen = 8;			
				DBGPRINT(RT_DEBUG_TRACE, ("IWSC_PeerProbeResponse:: WscPinCode = %d\n", pWscCtrl->WscPinCode));
				pWscCtrl->WscStatus = STATUS_WSC_IBSS_NEW_RANDOM_PIN;
			}
			else
				pWscCtrl->WscStatus = STATUS_WSC_IBSS_FIXED_PIN;

			WscGetRegDataPIN(pAd, pWscCtrl->WscEnrolleePinCode, pWscCtrl);
			pAd->Mlme.IWscMachine.CurrState = IWSC_START;
			pAd->StaCfg.IWscInfo.bSendEapolStart = TRUE;
		}
		else
		{
			DBGPRINT(RT_DEBUG_ERROR, 
				("IWSC_PeerProbeRequest:: Both devices are limited UI. Shall we change to use PBC mode?\n"));
			IWSC_Stop(pAd, FALSE);
			return;
		}
	}
	else
	{
		DBGPRINT(RT_DEBUG_TRACE, ("IWSC_PeerProbeResponse:: Please input Peer's PIN\n"));
		pAd->Mlme.IWscMachine.CurrState = IWSC_WAIT_PIN;
		pWscCtrl->WscStatus = STATUS_WSC_WAIT_PIN_CODE;
	}
	
	MlmeEnqueue(pAd,
		MLME_CNTL_STATE_MACHINE,
		OID_802_11_SSID,
		sizeof(NDIS_802_11_SSID),
		(VOID *)&pWscCtrl->WscSsid, 0);
	RTMP_MLME_HANDLER(pAd);
		
	DBGPRINT(RT_DEBUG_TRACE, ("-----> IWSC_PeerProbeResponse\n"));
	return;
}


VOID IWSC_PeerPIN(RTMP_ADAPTER *pAd, PMLME_QUEUE_ELEM pElem)
{
	PWSC_CTRL	pWscCtrl = &pAd->StaCfg.WscControl;

	DBGPRINT(RT_DEBUG_TRACE, ("-----> IWSC_PeerPIN\n"));
	if (ADHOC_ON(pAd))
	{
		NdisZeroMemory(pWscCtrl->EntryAddr, MAC_ADDR_LEN);
		NdisMoveMemory(pWscCtrl->EntryAddr, pWscCtrl->WscPeerMAC, MAC_ADDR_LEN);
		pWscCtrl->WscState = WSC_STATE_LINK_UP;
		pWscCtrl->WscStatus = STATUS_WSC_LINK_UP;
		WscSendEapolStart(pAd, pWscCtrl->WscPeerMAC, STA_MODE);
	}
	else
	{
		hex_dump("WscPeerMAC", pWscCtrl->WscPeerMAC, MAC_ADDR_LEN);
		MlmeEnqueue(pAd,
				MLME_CNTL_STATE_MACHINE,
				OID_802_11_SSID,
				sizeof(NDIS_802_11_SSID),
				(VOID *)&pWscCtrl->WscSsid, 0);
		RTMP_MLME_HANDLER(pAd);
		pAd->StaCfg.IWscInfo.bSendEapolStart = TRUE;
	}
	pAd->Mlme.IWscMachine.CurrState = IWSC_START;

	DBGPRINT(RT_DEBUG_TRACE, ("<----- IWSC_PeerPIN\n"));
}


BOOLEAN	IWSC_PeerEapolStart(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry)
{
	PWSC_CTRL	pWpsCtrl = &pAd->StaCfg.WscControl;
	PIWSC_INFO	pIWscInfo = &pAd->StaCfg.IWscInfo;
	BOOLEAN		Cancelled;

	if (pWpsCtrl->WscConfMode != WSC_REGISTRAR)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("IWSC_PeerEapolStart:: Not Registrar now(WscConfMode = %d)\n", pWpsCtrl->WscConfMode));
		return FALSE;
	}

	DBGPRINT(RT_DEBUG_TRACE, ("<----- IWSC_PeerEapolStart\n"));

	// Receive enrollee identity from EAP
	if (pWpsCtrl->WscMode == WSC_PBC_MODE)
	{
		/*
			Some WPS PBC Station select AP from UI directly; doesn't do PBC scan.
			Need to check DPID from STA again here.
		*/
		WscPBC_DPID_FromSTA(pAd, pEntry->Addr);
		WscPBCSessionOverlapCheck(pAd);
		if ((pAd->CommonCfg.WscStaPbcProbeInfo.WscPBCStaProbeCount == 1) &&
			!NdisEqualMemory(pAd->CommonCfg.WscStaPbcProbeInfo.StaMacAddr[0], &ZERO_MAC_ADDR[0], MAC_ADDR_LEN) &&
			(NdisEqualMemory(pAd->CommonCfg.WscStaPbcProbeInfo.StaMacAddr[0], &pEntry->Addr[0], 6) == FALSE))
		{
			pAd->CommonCfg.WscPBCOverlap = TRUE;
		}
		//if (pAd->CommonCfg.WscPBCOverlap)
		{
			hex_dump("EntryAddr", pWpsCtrl->EntryAddr, 6);
			hex_dump("StaMacAddr0", pAd->CommonCfg.WscStaPbcProbeInfo.StaMacAddr[0], 6);
			hex_dump("StaMacAddr1", pAd->CommonCfg.WscStaPbcProbeInfo.StaMacAddr[1], 6);
			hex_dump("StaMacAddr2", pAd->CommonCfg.WscStaPbcProbeInfo.StaMacAddr[2], 6);
			hex_dump("StaMacAddr3", pAd->CommonCfg.WscStaPbcProbeInfo.StaMacAddr[3], 6);
		}
		if (pAd->CommonCfg.WscPBCOverlap == TRUE)
		{
			// PBC session overlap
			pWpsCtrl->WscStatus = STATUS_WSC_PBC_SESSION_OVERLAP;
			RTMPSendWirelessEvent(pAd, IW_WSC_PBC_SESSION_OVERLAP, NULL, pWpsCtrl->EntryIfIdx, 0); 
			DBGPRINT(RT_DEBUG_TRACE, ("IWSC_PeerEapolStart: PBC Session Overlap!\n"));
			IWSC_MlmeStopAction(pAd, NULL);
			return FALSE;
		}
	}
	else if (pWpsCtrl->WscMode == WSC_SMPBC_MODE)
	{
		if (pEntry->bIWscSmpbcAccept == FALSE)
		{
			DBGPRINT(RT_DEBUG_TRACE, ("IWSC_PeerEapolStart:: pEntry->bIWscSmpbcAccept == FALSE\n"));
			return FALSE;
		}

		if (pWpsCtrl->EapMsgRunning == TRUE)
		{
			DBGPRINT(RT_DEBUG_TRACE, ("IWSC_PeerEapolStart:: Busy now\n"));
			return FALSE;
		}

		if (pIWscInfo->bIWscT1TimerRunning)
		{
			RTMPCancelTimer(&pIWscInfo->IWscT1Timer, &Cancelled);
			pIWscInfo->bIWscT1TimerRunning = FALSE;
		}
	}

	if (pIWscInfo->bIWscT2TimerRunning)
	{
		RTMPCancelTimer(&pIWscInfo->IWscT2Timer, &Cancelled);
		pIWscInfo->bIWscT2TimerRunning = FALSE;
	}
	
	if (MAC_ADDR_EQUAL(pWpsCtrl->EntryAddr, ZERO_MAC_ADDR))
	{
		NdisMoveMemory(pWpsCtrl->EntryAddr, pEntry->Addr, MAC_ADDR_LEN);
	}

	DBGPRINT(RT_DEBUG_TRACE, ("-----> IWSC_PeerEapolStart\n"));
	return TRUE;
}


VOID IWSC_RoleAction(RTMP_ADAPTER *pAd, INT WscConfMode)
{
	PWSC_CTRL	pWpsCtrl = &pAd->StaCfg.WscControl;
	USHORT		WscMode;
	USHORT		ConfigMethod = pWpsCtrl->WscConfigMethods;
	
	if (WscConfMode == WSC_REGISTRAR)
	{
		pWpsCtrl->WscConfMode = WSC_REGISTRAR;		
		AsicDisableSync(pAd);

		if (pWpsCtrl->WscMode == WSC_PBC_MODE)
			WscMode = DEV_PASS_ID_PBC;
		else if (pWpsCtrl->WscMode == WSC_SMPBC_MODE)
			WscMode = DEV_PASS_ID_SMPBC;
		else
		{
			WscMode = DEV_PASS_ID_REG;
		}
		
		if (!pAd->StaCfg.IWscInfo.bLimitedUI)
			ConfigMethod |= WPS_CONFIG_METHODS_KEYPAD;
		else
			ConfigMethod &= (~WPS_CONFIG_METHODS_KEYPAD);
		WscBuildBeaconIE(pAd, pWpsCtrl->WscConfStatus, TRUE, WscMode, ConfigMethod, BSS0, NULL, 0, STA_MODE);
		WscBuildProbeRespIE(pAd, WSC_MSGTYPE_REGISTRAR, pWpsCtrl->WscConfStatus, TRUE, WscMode, ConfigMethod, BSS0, NULL, 0, STA_MODE);
		MakeIbssBeacon(pAd);        /* re-build BEACON frame */
		AsicEnableIbssSync(pAd);    /* copy BEACON frame to on-chip memory */
		pWpsCtrl->WscState = WSC_STATE_LINK_UP;
		pWpsCtrl->WscStatus = STATUS_WSC_LINK_UP;
		RTMPSendWirelessEvent(pAd, IW_IWSC_BECOME_REGISTRAR, NULL, pWpsCtrl->EntryIfIdx, 0);
	}
	else
	{
		pWpsCtrl->WscRejectSamePinFromEnrollee = FALSE;
		pWpsCtrl->WscConfMode = WSC_ENROLLEE;
			
		/*
			Set the AutoReconnectSsid to prevent it reconnect to old SSID
		*/
		pAd->MlmeAux.AutoReconnectSsidLen= 32;
		NdisZeroMemory(pAd->MlmeAux.AutoReconnectSsid, pAd->MlmeAux.AutoReconnectSsidLen);
		
		AsicDisableSync(pAd);
		pAd->StaCfg.WpsIEBeacon.ValueLen = 0;
		pAd->StaCfg.WpsIEProbeResp.ValueLen = 0;
		MakeIbssBeacon(pAd);        /* re-build BEACON frame */
		AsicEnableIbssSync(pAd);    /* copy BEACON frame to on-chip memory */

		pWpsCtrl->WscState = WSC_STATE_START;
		pWpsCtrl->WscStatus = STATUS_WSC_SCAN_AP;
		NdisZeroMemory(&pWpsCtrl->WscSsid, sizeof(NDIS_802_11_SSID));
		RTMPSendWirelessEvent(pAd, IW_IWSC_BECOME_ENROLLEE, NULL, pWpsCtrl->EntryIfIdx, 0);
		// For PBC, the PIN is all '0'
		WscGetRegDataPIN(pAd, pWpsCtrl->WscPinCode, pWpsCtrl);
		WscScanExec(pAd, pWpsCtrl);
	}
}


VOID IWSC_AddSmpbcEnrollee(RTMP_ADAPTER *pAd, UCHAR *pPeerAddr)
{
	PWSC_CTRL pWpsCtrl = &pAd->StaCfg.WscControl;
	PIWSC_INFO	pIWscInfo = &pAd->StaCfg.IWscInfo;
	PMAC_TABLE_ENTRY pEntry = NULL;
	PWSC_PEER_ENTRY pWscPeerEntry = NULL;

	DBGPRINT(RT_DEBUG_TRACE, ("<----- IWSC_AddSmpbcEnrollee (WscConfMode = %d, WscStatus = %d)\n", 
					pWpsCtrl->WscConfMode, pWpsCtrl->WscStatus));

	pWscPeerEntry = WscFindPeerEntry(&pWpsCtrl->WscPeerList, pPeerAddr);
	if (pWscPeerEntry && 
		pWscPeerEntry->bIWscSmpbcAccept &&
		!pIWscInfo->bIWscEntryTimerRunning)
	{
		pEntry = MacTableLookup(pAd, pPeerAddr);
		if (pEntry == NULL)
			pEntry = MacTableInsertEntry(pAd, pPeerAddr, &pAd->StaCfg.wdev, BSS0, OPMODE_STA, FALSE);
		if (pEntry)
		{
			pEntry->bIWscSmpbcAccept = TRUE;
			if ((pWpsCtrl->WscConfMode == WSC_DISABLE) &&
				(pWpsCtrl->WscStatus == STATUS_WSC_CONFIGURED || pWpsCtrl->WscStatus == STATUS_WSC_IBSS_WAIT_NEXT_SMPBC_ENROLLEE))
			{
				pWpsCtrl->WscConfMode = WSC_REGISTRAR;
				pWpsCtrl->WscMode = WSC_SMPBC_MODE;
				pWpsCtrl->WscState = WSC_STATE_LINK_UP;
				pWpsCtrl->WscStatus = STATUS_WSC_LINK_UP;
				pWpsCtrl->bWscTrigger = TRUE;
				WscBuildProbeRespIE(pAd, 
								WSC_MSGTYPE_REGISTRAR, 
								pAd->StaCfg.WscControl.WscConfStatus, 
								TRUE, 
								DEV_PASS_ID_SMPBC, 
								pAd->StaCfg.WscControl.WscConfigMethods, 
								BSS0, 
								NULL, 
								0,
								STA_MODE);
			}
			DBGPRINT(RT_DEBUG_TRACE, ("SMPBC Enrollee - %02x:%02x:%02x:%02x:%02x:%02x\n",
										PRINT_MAC(pEntry->Addr)));
		}
	}
	else if (pIWscInfo->bIWscEntryTimerRunning)
	{
		pEntry = MacTableLookup(pAd, pPeerAddr);
		if (pEntry == NULL)
		{			
			pEntry = MacTableInsertEntry(pAd, pPeerAddr, &pAd->StaCfg.wdev, BSS0, OPMODE_STA, FALSE);
			pWscPeerEntry = WscFindPeerEntry(&pWpsCtrl->WscPeerList, pPeerAddr);
			if (pWscPeerEntry == NULL)
				pIWscInfo->IWscSmpbcAcceptCount++;
		}

		if (pEntry && !pEntry->bIWscSmpbcAccept)
		{
			pEntry->bIWscSmpbcAccept = TRUE;
			DBGPRINT(RT_DEBUG_TRACE, ("Accept this peer enrollee - %02x:%02x:%02x:%02x:%02x:%02x\n",
										PRINT_MAC(pEntry->Addr)));

			WscInsertPeerEntryByMAC(&pWpsCtrl->WscPeerList, pPeerAddr);
			pWscPeerEntry = WscFindPeerEntry(&pWpsCtrl->WscPeerList, pPeerAddr);

			if (pWscPeerEntry)
				pWscPeerEntry->bIWscSmpbcAccept = TRUE;
			
			if ((pWpsCtrl->WscConfMode == WSC_DISABLE) &&
				(pWpsCtrl->WscStatus == STATUS_WSC_CONFIGURED || pWpsCtrl->WscStatus == STATUS_WSC_IBSS_WAIT_NEXT_SMPBC_ENROLLEE))
			{
				pWpsCtrl->WscConfMode = WSC_REGISTRAR;
				pWpsCtrl->WscMode = WSC_SMPBC_MODE;
				pWpsCtrl->WscState = WSC_STATE_LINK_UP;
				pWpsCtrl->WscStatus = STATUS_WSC_LINK_UP;
				pWpsCtrl->bWscTrigger = TRUE;
			}
			WscBuildProbeRespIE(pAd, 
								WSC_MSGTYPE_REGISTRAR, 
								pAd->StaCfg.WscControl.WscConfStatus, 
								TRUE, 
								DEV_PASS_ID_SMPBC, 
								pAd->StaCfg.WscControl.WscConfigMethods, 
								BSS0, 
								NULL, 
								0,
								STA_MODE);
		}
	}

	DBGPRINT(RT_DEBUG_TRACE, ("-----> IWSC_AddSmpbcEnrollee\n"));
	return;
}


ULONG IWSC_SearchWpsApByPinMethod(RTMP_ADAPTER *pAd)
{
	UCHAR 		i;
	BSS_ENTRY 	*pBss;

	for (i = 0; i < pAd->ScanTab.BssNr; i++) 
	{
		pBss = &pAd->ScanTab.BssEntry[i];
		if ((pBss->WpsAP) &&
			(pBss->BssType == BSS_ADHOC) &&
			((pBss->WscDPIDFromWpsAP == DEV_PASS_ID_PIN) || (pBss->WscDPIDFromWpsAP == DEV_PASS_ID_REG)))
		{
			return (ULONG)i;
		}
	}
	return (ULONG)BSS_NOT_FOUND;
}


VOID IWSC_GetConfigMethod(RTMP_ADAPTER *pAd, UCHAR *pVIE, INT VIELen, USHORT *pConfigMethod)
{
	PUCHAR	pData = pVIE;
	INT		Len = VIELen;
	PBEACON_EID_STRUCT	pEid;
	USHORT	Type = 0, TLV_Len = 0;
	
	while (Len > 0)
	{
		pEid = (PBEACON_EID_STRUCT) pData;
		
		// No match, skip the Eid and move forward, IE_WFA_WSC = 0xdd
		if (pEid->Eid != IE_WFA_WSC)
		{
			// Set the offset and look for next IE
			pData += (pEid->Len + 2);
			Len   -= (pEid->Len + 2);
			continue;
		}
		else
		{
			// Found IE with 0xdd
			// check for WSC OUI -- 00 50 f2 04
			if (NdisEqualMemory(pEid->Octet, IWSC_OUI, 4) == FALSE)
			{
				// Set the offset and look for next IE
				pData += (pEid->Len + 2);
				Len   -= (pEid->Len + 2);
				continue;
			}				
		}

		// 3. Found	AP with WSC IE in beacons, skip 6 bytes = 1 + 1 + 4
		pData += 6;
		Len   -= 6;

		// 4. Start to look the PBC type within WSC VarIE
		while (Len > 0)
		{
			// Check for WSC IEs
			NdisMoveMemory(&Type, pData, 2);
			NdisMoveMemory(&TLV_Len, pData+2, 2);

			// Check for config method
			if (be2cpu16(Type) == WSC_ID_SEL_REG_CFG_METHODS)
			{
				NdisMoveMemory(pConfigMethod, pData+4, 2);
				*pConfigMethod = be2cpu16(*pConfigMethod);
			}

			// Set the offset and look for PBC information
			// Since Type and Length are both short type, we need to offset 4, not 2
			pData += (be2cpu16(TLV_Len) + 4);
			Len   -= (be2cpu16(TLV_Len) + 4);
		}
	}
}


VOID IWSC_BuildDevQueryFrame(RTMP_ADAPTER *pAd, UCHAR *pOutBuf, USHORT *pIeLen)
{
	UCHAR 			*Data = NULL;
	PUCHAR			pData;
	INT				Len = 0, templen = 0;
	PWSC_CTRL		pWpsCtrl = &pAd->StaCfg.WscControl;
	PWSC_REG_DATA	pReg = (PWSC_REG_DATA) &pWpsCtrl->RegData;
	USHORT			tempVal = 0;
	WSC_IE_HEADER 	ieHdr;

	/* allocate memory */
	os_alloc_mem(NULL, (UCHAR **)&Data, 512);
	if (Data == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s: Allocate memory fail!!!\n", __FUNCTION__));
		return;
	}
	
	DBGPRINT(RT_DEBUG_TRACE, ("-----> IWSC_BuildDevQueryFrame\n"));

	// WSC IE HEader
	ieHdr.elemId = 221;
	ieHdr.length = 4;
	ieHdr.oui[0] = 0x00; 
	ieHdr.oui[1] = 0x50; 
	ieHdr.oui[2] = 0xF2; 
	ieHdr.oui[3] = 0x10;

	pData = (PUCHAR) &Data[0];
	Len = 0;
	
	/* Version */
	templen = AppendWSCTLV(WSC_ID_VERSION, pData, &pReg->SelfInfo.Version, 0);
	pData += templen;
	Len   += templen;

	/* MAC address */
	templen = AppendWSCTLV(WSC_ID_MAC_ADDR, pData, pReg->SelfInfo.MacAddr, 0);
	pData += templen;
	Len   += templen;

	/* Manufacture */
	templen = AppendWSCTLV(WSC_ID_MANUFACTURER, pData, pReg->SelfInfo.Manufacturer, strlen((PSTRING) pReg->SelfInfo.Manufacturer));
	pData += templen;
	Len   += templen;

	/* Model Name */
	templen = AppendWSCTLV(WSC_ID_MODEL_NAME, pData, pReg->SelfInfo.ModelName, strlen((PSTRING) pReg->SelfInfo.ModelName));
	pData += templen;
	Len   += templen;
	
	/* Model Number */
	templen = AppendWSCTLV(WSC_ID_MODEL_NUMBER, pData, pReg->SelfInfo.ModelNumber, strlen((PSTRING) pReg->SelfInfo.ModelNumber));
	pData += templen;
	Len   += templen;

	/* Device Name */
	templen = AppendWSCTLV(WSC_ID_DEVICE_NAME, pData, pReg->SelfInfo.DeviceName, strlen((PSTRING) pReg->SelfInfo.DeviceName));
	pData += templen;
	Len   += templen;

	ieHdr.length = ieHdr.length + Len;

	RTMPMoveMemory(pOutBuf, &ieHdr, sizeof(WSC_IE_HEADER));
	RTMPMoveMemory(pOutBuf + sizeof(WSC_IE_HEADER), Data, Len);
	*pIeLen = (USHORT)(sizeof(WSC_IE_HEADER) + Len);

	if (Data != NULL)
		os_free_mem(NULL, Data);
	
	DBGPRINT(RT_DEBUG_TRACE, ("<----- IWSC_BuildDevQueryFrame\n"));
}


VOID IWSC_ResetIpContent(RTMP_ADAPTER *pAd)
{
	PIWSC_INFO	pIWscInfo = &pAd->StaCfg.IWscInfo;

	DBGPRINT(RT_DEBUG_TRACE, ("<----- %s\n", __FUNCTION__));
	pIWscInfo->IpDevCount = 0;
	pIWscInfo->RegDepth = 0;
	pIWscInfo->AvaSubMaskListCount = IWSC_MAX_SUB_MASK_LIST_COUNT;
	pIWscInfo->bAssignWscIPv4 = TRUE;
	pIWscInfo->AvaSubMaskList[0] = IWSC_IPV4_RANGE1;
	pIWscInfo->AvaSubMaskList[1] = IWSC_IPV4_RANGE2;
	pIWscInfo->AvaSubMaskList[2] = IWSC_IPV4_RANGE3;
	DBGPRINT(RT_DEBUG_TRACE, ("<----- %s\n", __FUNCTION__));
}


BOOLEAN IWSC_IpContentForCredential(RTMP_ADAPTER *pAd)
{
	PIWSC_INFO	pIWscInfo = &pAd->StaCfg.IWscInfo;

	if (pIWscInfo->IpDevCount >= IWSC_MAX_IP_DEV_COUNT)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("pIWscInfo->IpDevCount = %d\n", 
			pIWscInfo->IpDevCount));
		return FALSE;
	}

	if (pIWscInfo->bAssignWscIPv4 == FALSE)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("pIWscInfo->bAssignWscIPv4 = %d\n", 
			pIWscInfo->bAssignWscIPv4));
		return FALSE;
	}
	if (pIWscInfo->RegDepth == 0)
	{
		pIWscInfo->IpDevCount++;
		pIWscInfo->SelfIpv4Addr = IWSC_DEFAULT_REG_IPV4_ADDR;
		pIWscInfo->PeerIpv4Addr = pIWscInfo->SelfIpv4Addr + pIWscInfo->IpDevCount;
		DBGPRINT(RT_DEBUG_TRACE, ("1) SelfIpv4Addr = 0x%08x, PeerIpv4Addr = 0x%08x\n", 
			pIWscInfo->SelfIpv4Addr,
			pIWscInfo->PeerIpv4Addr));
		return TRUE;
	}
	else
	{
		UINT32 shift_offset = 0;
		UINT32 dev_count = (UINT32)(++pIWscInfo->IpDevCount);
		if (pIWscInfo->SelfIpv4Addr == 0)
		{
			DBGPRINT(RT_DEBUG_TRACE, ("IMPOSSIBLE!! pIWscInfo->SelfIpv4Addr = %d\n", pIWscInfo->SelfIpv4Addr));
			return FALSE;
		}

		if (pIWscInfo->CurrentIpRange == IWSC_IPV4_RANGE0)
		{
			shift_offset = 0;
		}
		else if (pIWscInfo->CurrentIpRange == IWSC_IPV4_RANGE1)
		{
			shift_offset = 6;
		}
		else if (pIWscInfo->CurrentIpRange == IWSC_IPV4_RANGE2)
		{
			shift_offset = 12;
		}
		else if (pIWscInfo->CurrentIpRange == IWSC_IPV4_RANGE3)
		{
			shift_offset = 18;
		}

		pIWscInfo->PeerIpv4Addr = (pIWscInfo->SelfIpv4Addr & pIWscInfo->CurrentIpRange) + (dev_count << shift_offset);
		DBGPRINT(RT_DEBUG_TRACE, ("2) SelfIpv4Addr = 0x%08x, PeerIpv4Addr = 0x%08x\n", 
			pIWscInfo->SelfIpv4Addr,
			pIWscInfo->PeerIpv4Addr));
		return TRUE;
	}
}
#endif /* IWSC_SUPPORT */


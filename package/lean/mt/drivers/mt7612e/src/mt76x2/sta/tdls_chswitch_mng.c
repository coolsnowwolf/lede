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
    tdls.h
 
    Abstract:
 
    Revision History:
    Who        When          What
    ---------  ----------    ----------------------------------------------
    Arvin Tai  17-04-2009    created for 802.11z
 */

#ifdef DOT11Z_TDLS_SUPPORT

#include <linux/timer.h>
#include "rt_config.h"

/*
    ==========================================================================
    Description:
        tdls state machine init, including state transition and timer init
    Parameters:
        Sm - pointer to the dls state machine
    Note:
        The state machine looks like this
        
                            TDLS_IDLE
        
	IRQL = PASSIVE_LEVEL

    ==========================================================================
 */
VOID TDLS_ChSwStateMachineInit(
	IN PRTMP_ADAPTER pAd, 
	IN STATE_MACHINE *Sm, 
	OUT STATE_MACHINE_FUNC Trans[]) 
{
	UCHAR i;
	PRT_802_11_TDLS	pTDLS = NULL;

	StateMachineInit(Sm, (STATE_MACHINE_FUNC*)Trans, (ULONG)MAX_TDLS_STATE,
		(ULONG)MAX_TDLS_CHSW_MSG, (STATE_MACHINE_FUNC)Drop, TDLS_IDLE, TDLS_CHSW_MACHINE_BASE);
     
	/* the first column */
	StateMachineSetAction(Sm, TDLS_IDLE, MT2_MLME_TDLS_CH_SWITCH_REQ, (STATE_MACHINE_FUNC)TDLS_MlmeChannelSwitchAction);
	StateMachineSetAction(Sm, TDLS_IDLE, MT2_MLME_TDLS_CH_SWITCH_RSP, (STATE_MACHINE_FUNC)TDLS_MlmeChannelSwitchRspAction);
	StateMachineSetAction(Sm, TDLS_IDLE, MT2_PEER_TDLS_CH_SWITCH_REQ, (STATE_MACHINE_FUNC)TDLS_PeerChannelSwitchReqAction);
	StateMachineSetAction(Sm, TDLS_IDLE, MT2_PEER_TDLS_CH_SWITCH_RSP, (STATE_MACHINE_FUNC)TDLS_PeerChannelSwitchRspAction);

	for (i = 0; i < MAX_NUMBER_OF_DLS_ENTRY; i++)
	{
		pAd->StaCfg.TdlsInfo.TDLSEntry[i].pAd = pAd;
		pTDLS = &pAd->StaCfg.TdlsInfo.TDLSEntry[i];

		RTMPInitTimer(pAd, &pTDLS->ChannelSwitchTimer,
					GET_TIMER_FUNCTION(TDLS_ChannelSwitchTimeAction), pTDLS, FALSE);
		RTMPInitTimer(pAd, &pTDLS->ChannelSwitchTimeoutTimer,
					GET_TIMER_FUNCTION(TDLS_ChannelSwitchTimeOutAction), pTDLS, FALSE);
	}

	RTMPInitTimer(pAd, &pAd->StaCfg.TdlsInfo.TdlsDisableChannelSwitchTimer,
				GET_TIMER_FUNCTION(TDLS_DisablePeriodChannelSwitchAction), pAd, FALSE);
	RTMPInitTimer(pAd, &pAd->StaCfg.TdlsInfo.TdlsPeriodGoOffChTimer,
				GET_TIMER_FUNCTION(TDLS_BaseChExpired), pAd, FALSE);
	RTMPInitTimer(pAd, &pAd->StaCfg.TdlsInfo.TdlsPeriodGoBackBaseChTimer,
				GET_TIMER_FUNCTION(TDLS_OffChExpired), pAd, FALSE);
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID
TDLS_BuildChannelSwitchRequest(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN PUCHAR pPeerAddr,
	IN USHORT ChSwitchTime,
	IN USHORT ChSwitchTimeOut,
	IN UCHAR TargetChannel,
	IN UCHAR TargetChannelBW)
{
	PTDLS_STRUCT pTdlsControl = &pAd->StaCfg.TdlsInfo;
	PRT_802_11_TDLS	pTDLS = NULL;
	INT LinkId = 0xff;

	/* fill action code */
	TDLS_InsertActField(pAd, (pFrameBuf + *pFrameLen), pFrameLen,
							CATEGORY_TDLS, TDLS_ACTION_CODE_CHANNEL_SWITCH_REQUEST);

	/* Target Channel */
	TDLS_InsertTargetChannel(pAd, (pFrameBuf + *pFrameLen), pFrameLen, TargetChannel);

	/* Regulatory Class */
	TDLS_InsertRegulatoryClass(pAd, (pFrameBuf + *pFrameLen), pFrameLen, TargetChannel, TargetChannelBW);

	/* Secondary Channel Offset */
	if(TargetChannelBW != EXTCHA_NONE)
	{
		if (TargetChannel > 14)
		{
			if ((TargetChannel == 36) || (TargetChannel == 44) || (TargetChannel == 52) ||
				(TargetChannel == 60) || (TargetChannel == 100) || (TargetChannel == 108) ||
				(TargetChannel == 116) || (TargetChannel == 124) || (TargetChannel == 132) ||
				(TargetChannel == 149) || (TargetChannel == 157))
			{
				TDLS_InsertSecondaryChOffsetIE(pAd, (pFrameBuf + *pFrameLen), pFrameLen, EXTCHA_ABOVE);
				pTdlsControl->TdlsDesireChannelBW = EXTCHA_ABOVE;
				
			}
			else if ((TargetChannel == 40) || (TargetChannel == 48) || (TargetChannel == 56) |
					(TargetChannel == 64) || (TargetChannel == 104) || (TargetChannel == 112) ||
					(TargetChannel == 120) || (TargetChannel == 128) || (TargetChannel == 136) ||
					(TargetChannel == 153) || (TargetChannel == 161))
			{
				TDLS_InsertSecondaryChOffsetIE(pAd, (pFrameBuf + *pFrameLen), pFrameLen, EXTCHA_BELOW);
				pTdlsControl->TdlsDesireChannelBW = EXTCHA_BELOW;
			}
		}
		else
		{
			UCHAR ExtCh;
			UCHAR Dir = TargetChannelBW;

			ExtCh = TDLS_GetExtCh(TargetChannel, Dir);

			if (TDLS_IsValidChannel(pAd, ExtCh))
			{
				TDLS_InsertSecondaryChOffsetIE(pAd, (pFrameBuf + *pFrameLen), pFrameLen, Dir);
				pTdlsControl->TdlsDesireChannelBW = Dir;
			}
			else
			{
				Dir = (Dir == EXTCHA_ABOVE) ? EXTCHA_BELOW : EXTCHA_ABOVE;
				ExtCh = TDLS_GetExtCh(TargetChannel, Dir);

				if (TDLS_IsValidChannel(pAd, ExtCh))
				{
					TDLS_InsertSecondaryChOffsetIE(pAd, (pFrameBuf + *pFrameLen), pFrameLen, Dir);
					pTdlsControl->TdlsDesireChannelBW = Dir;
				}
			}
		}
	}
	else
	{
		pTdlsControl->TdlsDesireChannelBW = EXTCHA_NONE;
	}

	/* fill link identifier */
	LinkId = TDLS_SearchLinkId(pAd, pPeerAddr);
	if (LinkId == -1 || LinkId == MAX_NUM_OF_TDLS_ENTRY)
	{
		DBGPRINT(RT_DEBUG_ERROR,("%s - can not find the LinkId!\n", __FUNCTION__));
		return;
	}
	pTDLS = (PRT_802_11_TDLS)&pAd->StaCfg.TdlsInfo.TDLSEntry[LinkId];

	if (pTDLS->bInitiator)
		TDLS_InsertLinkIdentifierIE(pAd, (pFrameBuf + *pFrameLen), pFrameLen, pPeerAddr, pAd->CurrentAddress);
	else
		TDLS_InsertLinkIdentifierIE(pAd, (pFrameBuf + *pFrameLen), pFrameLen, pAd->CurrentAddress, pPeerAddr);

	/* Channel Switch Timing */
	TDLS_InsertChannelSwitchTimingIE(pAd, (pFrameBuf + *pFrameLen), pFrameLen, ChSwitchTime, ChSwitchTimeOut);
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID
TDLS_BuildChannelSwitchResponse(
	IN	PRTMP_ADAPTER	pAd,
	OUT PUCHAR	pFrameBuf,
	OUT PULONG	pFrameLen,
	IN	PRT_802_11_TDLS	pTDLS,
	IN	USHORT	ChSwitchTime,
	IN	USHORT	ChSwitchTimeOut,
	IN	UINT16	ReasonCode)
{
	/* fill action code */
	TDLS_InsertActField(pAd, (pFrameBuf + *pFrameLen), pFrameLen,
						CATEGORY_TDLS, TDLS_ACTION_CODE_CHANNEL_SWITCH_RESPONSE);

	/* fill reason code */
	TDLS_InsertReasonCode(pAd, (pFrameBuf + *pFrameLen), pFrameLen, ReasonCode);

	/* fill link identifier */
	if (pTDLS->bInitiator)
		TDLS_InsertLinkIdentifierIE(pAd, (pFrameBuf + *pFrameLen), pFrameLen, pTDLS->MacAddr, pAd->CurrentAddress);
	else
		TDLS_InsertLinkIdentifierIE(pAd, (pFrameBuf + *pFrameLen), pFrameLen, pAd->CurrentAddress, pTDLS->MacAddr);

	/* Channel Switch Timing */
	TDLS_InsertChannelSwitchTimingIE(pAd, (pFrameBuf + *pFrameLen), pFrameLen, ChSwitchTime, ChSwitchTimeOut);
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
NDIS_STATUS
TDLS_ChannelSwitchReqAction(
	IN PRTMP_ADAPTER	pAd,
	IN PRT_802_11_TDLS pTDLS,
	IN PUCHAR pPeerAddr,
	IN UCHAR TargetChannel,
	IN UCHAR TargetChannelBW)
{
	UCHAR TDLS_ETHERTYPE[] = {0x89, 0x0d};
	UCHAR Header802_3[14];
	PUCHAR pOutBuffer = NULL;
	ULONG FrameLen = 0;
	ULONG TempLen;
	UCHAR RemoteFrameType = PROTO_NAME_TDLS;
	NDIS_STATUS	NStatus = NDIS_STATUS_SUCCESS;
	MAC_TABLE_ENTRY *pEntry = NULL;

	DBGPRINT(RT_DEBUG_TRACE, ("===> %s()\n", __FUNCTION__));

	MAKE_802_3_HEADER(Header802_3, pPeerAddr, pAd->CurrentAddress, TDLS_ETHERTYPE);

	/* Allocate buffer for transmitting message */
	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);
	if (NStatus	!= NDIS_STATUS_SUCCESS)	
		return NStatus;

	MakeOutgoingFrame(pOutBuffer,		&TempLen,
						1,				&RemoteFrameType,
						END_OF_ARGS);

	FrameLen = FrameLen + TempLen;

	TDLS_BuildChannelSwitchRequest(pAd,
									pOutBuffer,
									&FrameLen,
									pPeerAddr,
									TDLS_CHANNEL_SWITCH_TIME,
									TDLS_CHANNEL_SWITCH_TIMEOUT,
									TargetChannel,
									TargetChannelBW);

	pEntry = MacTableLookup(pAd, pPeerAddr);

	if (pEntry && IS_ENTRY_TDLS(pEntry))
	{
		pTDLS->ChannelSwitchCurrentState = TDLS_CHANNEL_SWITCH_WAIT_RSP;

		TDLS_SendChannelSwitchActionFrame(pAd,
											pEntry,
											Header802_3,
											LENGTH_802_3,
											pOutBuffer,
											(UINT)FrameLen,
											FALSE);
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Can't find TDLS entry on Mac TABLE !!!!\n"));
	}

	hex_dump("TDLS switch channel request send pack", pOutBuffer, FrameLen);

	MlmeFreeMemory(pAd, pOutBuffer);

	DBGPRINT(RT_DEBUG_TRACE, ("<=== %s()\n", __FUNCTION__));

	return NStatus;
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
NDIS_STATUS
TDLS_ChannelSwitchRspAction(
	IN	PRTMP_ADAPTER	pAd,
	IN	PRT_802_11_TDLS	pTDLS,
	IN	USHORT	ChSwitchTime,
	IN	USHORT	ChSwitchTimeOut,
	IN	UINT16	StatusCode,
	IN	UCHAR	FrameType)
{
	UCHAR	TDLS_ETHERTYPE[] = {0x89, 0x0d};
	UCHAR	Header802_3[14];
	PUCHAR	pOutBuffer = NULL;
	ULONG	FrameLen = 0;
	ULONG	TempLen;
	UCHAR	RemoteFrameType = PROTO_NAME_TDLS;
	NDIS_STATUS	NStatus = NDIS_STATUS_SUCCESS;

	DBGPRINT(RT_DEBUG_TRACE, ("===> %s()\n", __FUNCTION__));

	MAKE_802_3_HEADER(Header802_3, pTDLS->MacAddr, pAd->CurrentAddress, TDLS_ETHERTYPE);

	/* Allocate buffer for transmitting message */
	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);

	if (NStatus	!= NDIS_STATUS_SUCCESS)
	{
		DBGPRINT(RT_DEBUG_ERROR,("ACT - %s() allocate memory failed \n", __FUNCTION__));
		return NStatus;
	}

	MakeOutgoingFrame(pOutBuffer,		&TempLen,
						1,				&RemoteFrameType,
						END_OF_ARGS);

	FrameLen = FrameLen + TempLen;

	TDLS_BuildChannelSwitchResponse(pAd,
									pOutBuffer,
									&FrameLen,
									pTDLS,
									ChSwitchTime,
									ChSwitchTimeOut,
									StatusCode);

	TDLS_SendChannelSwitchActionFrame(pAd,
									&pAd->MacTab.Content[pTDLS->MacTabMatchWCID],
									Header802_3,
									LENGTH_802_3,
									pOutBuffer,
									(UINT)FrameLen,
									FrameType);

	hex_dump("TDLS send channel switch response pack", pOutBuffer, FrameLen);

	MlmeFreeMemory(pAd, pOutBuffer);

	DBGPRINT(RT_DEBUG_TRACE, ("<=== %s()\n", __FUNCTION__));

	return NStatus;
}

VOID
TDLS_TriggerChannelSwitchAction(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR DtimCount)
{
	PTDLS_STRUCT pTdlsControl = &pAd->StaCfg.TdlsInfo;
	PRT_802_11_TDLS	pTdlsEntry = NULL;
	BOOLEAN bCanDoChSwitch = FALSE;
	NDIS_STATUS	NStatus = NDIS_STATUS_SUCCESS;
	INT	LinkId = 0xff;

	RTMP_SEM_LOCK(&pTdlsControl->TdlsChSwLock);
	if (INFRA_ON(pAd) &&
		(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)) &&
		(pTdlsControl->bDoingPeriodChannelSwitch) &&
		(pTdlsControl->bChannelSwitchInitiator))
		bCanDoChSwitch = TRUE;
	RTMP_SEM_UNLOCK(&pTdlsControl->TdlsChSwLock);

	if (pAd->StaActive.ExtCapInfo.TDLSChSwitchProhibited == TRUE)
	{
		DBGPRINT(RT_DEBUG_ERROR,("%s - AP Prohibite TDLS Channel Switch !!!\n", __FUNCTION__));
		return;
	}

	if (bCanDoChSwitch)
	{
		if (DtimCount == 0)
		{
			BOOLEAN TimerCancelled;
			ULONG Now, Now1, Now2;
			
			DBGPRINT(RT_DEBUG_ERROR, ("101. %ld !!!\n", (jiffies * 1000) / OS_HZ));
			
			NdisGetSystemUpTime(&Now);
						
			if (pTdlsControl->TdlsGoBackStartTime == 0)
				Now2 = (pTdlsControl->TdlsActiveSwitchTime / 1000);
			else
			{
				if (pTdlsControl->TdlsGoBackStartTime > Now)
				{
					Now2 = (pTdlsControl->TdlsActiveSwitchTime / 1000);
				}
				else
				{
					Now1 = Now - pTdlsControl->TdlsGoBackStartTime;
					Now2 = ((Now1 * 1000) / OS_HZ);
				}
			}
			
			/* Drop not within my TDLS Table that created before ! */
			LinkId = TDLS_SearchLinkId(pAd, pTdlsControl->TdlsDesireChSwMacAddr);
			if (LinkId == -1 || LinkId == MAX_NUM_OF_TDLS_ENTRY)
			{
				RTMPCancelTimer(&pTdlsControl->TdlsPeriodGoBackBaseChTimer, &TimerCancelled);
				DBGPRINT(RT_DEBUG_ERROR, ("%s() - can not find the LinkId!\n", __FUNCTION__));
				return;
			}

			/* Point to the current Link ID */
			pTdlsEntry = &pAd->StaCfg.TdlsInfo.TDLSEntry[LinkId];

			if (pTdlsEntry->Valid && pTdlsEntry->bDoingPeriodChannelSwitch)
			{
				pTdlsControl->TdlsForcePowerSaveWithAP = TRUE;


				if (Now2 >= (pTdlsControl->TdlsActiveSwitchTime / 1000))
					RtmpOsMsDelay(9);
				else
					RtmpOsMsDelay(((pTdlsControl->TdlsActiveSwitchTime / 1000) - Now2) + 9);

				TDLS_KickOutHwNullFrame(pAd, PWR_SAVE, 0);
				
				RTMPCancelTimer(&pTdlsControl->TdlsDisableChannelSwitchTimer, &TimerCancelled);
				RTMPSetTimer(&pTdlsControl->TdlsDisableChannelSwitchTimer,
							((pAd->StaCfg.DtimPeriod * pAd->CommonCfg.BeaconPeriod) - 5));

				/* Build TDLS channel switch Request Frame */
				NStatus = TDLS_ChannelSwitchReqAction(pAd,
													pTdlsEntry,
													pTdlsEntry->MacAddr,
													pTdlsControl->TdlsDesireChannel,
													pTdlsControl->TdlsDesireChannelBW);
			
				if (NStatus != NDIS_STATUS_SUCCESS) 
					DBGPRINT(RT_DEBUG_ERROR,("%s() - Build Channel Switch Request Fail !!!\n", __FUNCTION__));
				else
					pTdlsControl->TdlsChannelSwitchPairCount++;
			}
		}
	}
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID
TDLS_MlmeChannelSwitchAction(
    IN PRTMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem)
{
	PMLME_TDLS_CH_SWITCH_STRUCT pChSwReq = NULL; 
	PTDLS_STRUCT pTdlsControl = &pAd->StaCfg.TdlsInfo;
	PRT_802_11_TDLS pTDLS = NULL;
	NDIS_STATUS	NStatus = NDIS_STATUS_SUCCESS;
	INT	LinkId = 0xff;
	BOOLEAN TimerCancelled;

	DBGPRINT(RT_DEBUG_TRACE, ("===> %s()\n", __FUNCTION__));

	pChSwReq = (PMLME_TDLS_CH_SWITCH_STRUCT)Elem->Msg;

	if (pAd->StaActive.ExtCapInfo.TDLSChSwitchProhibited == TRUE)
	{
		DBGPRINT(RT_DEBUG_ERROR,("%s - AP Prohibite TDLS Channel Switch !!!\n", __FUNCTION__));
		return;
	}

	if (INFRA_ON(pAd))
	{
		/* Drop not within my TDLS Table that created before ! */
		LinkId = TDLS_SearchLinkId(pAd, pChSwReq->PeerMacAddr);
		if (LinkId == -1 || LinkId == MAX_NUM_OF_TDLS_ENTRY)
		{
			DBGPRINT(RT_DEBUG_ERROR,("%s() - can not find the LinkId!\n", __FUNCTION__));
			return;
		}

		/* Point to the current Link ID */
		pTDLS = &pAd->StaCfg.TdlsInfo.TDLSEntry[LinkId];

		pTdlsControl->TdlsForcePowerSaveWithAP = TRUE;

		TDLS_KickOutHwNullFrame(pAd, PWR_SAVE, 0);

		RTMPCancelTimer(&pTdlsControl->TdlsDisableChannelSwitchTimer, &TimerCancelled);
		RTMPSetTimer(&pTdlsControl->TdlsDisableChannelSwitchTimer,
					((pAd->StaCfg.DtimPeriod * pAd->CommonCfg.BeaconPeriod) - 5));

		/* Build TDLS channel switch Request Frame */
		NStatus = TDLS_ChannelSwitchReqAction(pAd,
											pTDLS,
											pChSwReq->PeerMacAddr,
											pChSwReq->TargetChannel,
											pChSwReq->TargetChannelBW);

		if (NStatus	!= NDIS_STATUS_SUCCESS)	
			DBGPRINT(RT_DEBUG_ERROR,("%s() - Build Channel Switch Request Fail !!!\n", __FUNCTION__));
		else
			pTdlsControl->TdlsChannelSwitchPairCount++;
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR,("%s - TDLS only support infra mode !!!\n", __FUNCTION__));
	}

	DBGPRINT(RT_DEBUG_TRACE, ("<=== %s()\n", __FUNCTION__));

	return;
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID
TDLS_MlmeChannelSwitchRspAction(
    IN PRTMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem)
{
	PMLME_TDLS_CH_SWITCH_STRUCT pMlmeChSwitchRsp = NULL; 
	NDIS_STATUS	NStatus = NDIS_STATUS_SUCCESS;
	PRT_802_11_TDLS	pTdls = NULL;
	INT LinkId = 0xff;

	DBGPRINT(RT_DEBUG_TRACE, ("===> %s()\n", __FUNCTION__));

	pMlmeChSwitchRsp = (PMLME_TDLS_CH_SWITCH_STRUCT)Elem->Msg;

	if (INFRA_ON(pAd))
	{
		// Drop not within my TDLS Table that created before !
		LinkId = TDLS_SearchLinkId(pAd, pMlmeChSwitchRsp->PeerMacAddr);

		if (LinkId == -1 || LinkId == MAX_NUM_OF_TDLS_ENTRY)
		{
			DBGPRINT(RT_DEBUG_ERROR, ("%s() - can not find the LinkId!\n", __FUNCTION__));
			return;
		}

		/* Point to the current Link ID */
		pTdls = &pAd->StaCfg.TdlsInfo.TDLSEntry[LinkId];

		/* Build TDLS channel switch Request Frame */
		NStatus = TDLS_ChannelSwitchRspAction(pAd,
											pTdls,
											pTdls->ChSwitchTime,
											pTdls->ChSwitchTimeout,
											MLME_SUCCESS,
											RTMP_TDLS_SPECIFIC_NOACK);

		if (NStatus != NDIS_STATUS_SUCCESS)	
		{
			DBGPRINT(RT_DEBUG_ERROR, ("%s() - Build Channel Switch Response Fail !!!\n", __FUNCTION__));
		}
		else
		{
			RtmpusecDelay(300);
			NdisGetSystemUpTime(&pAd->StaCfg.TdlsInfo.TdlsGoBackStartTime);

			RTMP_OS_NETDEV_STOP_QUEUE(pAd->net_dev);
			RTMP_SET_MORE_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DEQUEUE);
			TDLS_DisablePktChannel(pAd, FIFO_HCCA);

			if (pAd->CommonCfg.CentralChannel > pAd->CommonCfg.Channel)
				TDLS_InitChannelRelatedValue(pAd, TRUE, pAd->CommonCfg.Channel, EXTCHA_ABOVE);
			else if (pAd->CommonCfg.CentralChannel < pAd->CommonCfg.Channel)
				TDLS_InitChannelRelatedValue(pAd, TRUE, pAd->CommonCfg.Channel, EXTCHA_BELOW);
			else
				TDLS_InitChannelRelatedValue(pAd, TRUE, pAd->CommonCfg.Channel, EXTCHA_NONE);

			TDLS_EnablePktChannel(pAd, FIFO_HCCA | FIFO_EDCA);
			RTMP_CLEAR_MORE_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DEQUEUE);
			RTMP_OS_NETDEV_WAKE_QUEUE(pAd->net_dev);
		}
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR,("%s - TDLS only support infra mode !!!\n", __FUNCTION__));
	}

	DBGPRINT(RT_DEBUG_TRACE, ("<=== %s()\n", __FUNCTION__));

	return;
}


/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID
TDLS_PeerChannelSwitchReqAction(
    IN PRTMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem)
{
	PRT_802_11_TDLS	pTDLS = NULL;
	INT		LinkId = 0xff;
	UCHAR	PeerAddr[MAC_ADDR_LEN];
	BOOLEAN	IsInitator;
	UCHAR	TargetChannel;
	UCHAR	RegulatoryClass;
	UCHAR	NewExtChannelOffset = 0xff;
	USHORT	PeerChSwitchTime;
	USHORT	PeerChSwitchTimeOut;
	NDIS_STATUS	NStatus = NDIS_STATUS_SUCCESS;
	USHORT	StatusCode = MLME_SUCCESS;
	PTDLS_STRUCT pTdlsControl = &pAd->StaCfg.TdlsInfo;

	DBGPRINT(RT_DEBUG_TRACE, ("===> %s()\n", __FUNCTION__));

	/* Not TDLS Capable, ignore it */
	if (!IS_TDLS_SUPPORT(pAd))
		return;

	if (!INFRA_ON(pAd))
		return;

	if (pAd->StaActive.ExtCapInfo.TDLSChSwitchProhibited == TRUE)
	{
		DBGPRINT(RT_DEBUG_OFF,("%s - AP Prohibite TDLS Channel Switch !!!\n", __FUNCTION__));
		return;
	}

	hex_dump("TDLS peer channel switch request receive pack", Elem->Msg, Elem->MsgLen);
	
	if (!PeerTdlsChannelSwitchReqSanity(pAd,
								Elem->Msg,
								Elem->MsgLen,
								PeerAddr,
								&IsInitator,
								&TargetChannel,
								&RegulatoryClass,
								&NewExtChannelOffset,
								&PeerChSwitchTime,
								&PeerChSwitchTimeOut))
	{
		DBGPRINT(RT_DEBUG_ERROR,
						("%s():  from %02x:%02x:%02x:%02x:%02x:%02x Sanity Check Fail !!!\n", 
						__FUNCTION__, PRINT_MAC(PeerAddr)));
		return;
	}

	if (RtmpPktPmBitCheck(pAd))
	{
		RTMP_SET_PSM_BIT(pAd, PWR_ACTIVE);
		TDLS_SendNullFrame(pAd, pAd->CommonCfg.TxRate, TRUE);
	}

	DBGPRINT(RT_DEBUG_WARN, ("%s():  from %02x:%02x:%02x:%02x:%02x:%02x !!!\n",
						__FUNCTION__, PRINT_MAC(PeerAddr)));

	DBGPRINT(RT_DEBUG_ERROR, ("300. %ld !!!\n", (jiffies * 1000) / OS_HZ));

	/* Drop not within my TDLS Table that created before ! */
	LinkId = TDLS_SearchLinkId(pAd, PeerAddr);
	if (LinkId == -1 || LinkId == MAX_NUM_OF_TDLS_ENTRY)
	{
		DBGPRINT(RT_DEBUG_ERROR,
						("%s() - can not find from %02x:%02x:%02x:%02x:%02x:%02x on TDLS entry !!!\n",
						__FUNCTION__, PRINT_MAC(PeerAddr)));
		return;
	}

	if (pTdlsControl->bChannelSwitchInitiator == FALSE)
	{
		pTdlsControl->TdlsForcePowerSaveWithAP = TRUE;
		TDLS_KickOutHwNullFrame(pAd, PWR_SAVE, 0);
	}

	/* Point to the current Link ID */
	pTDLS = &pAd->StaCfg.TdlsInfo.TDLSEntry[LinkId];

	if (TDLS_CHANNEL_SWITCH_TIME >= PeerChSwitchTime)
		PeerChSwitchTime = TDLS_CHANNEL_SWITCH_TIME;

	if (TDLS_CHANNEL_SWITCH_TIMEOUT >= PeerChSwitchTimeOut)
		PeerChSwitchTimeOut = TDLS_CHANNEL_SWITCH_TIMEOUT;

	{
		UINT32 macCfg, TxCount;
		UINT32 MTxCycle;
		UCHAR  TdlsFrameType = 0;

		RTMP_IO_READ32(pAd, TX_REPORT_CNT, &macCfg);

		
		if  (TargetChannel != pAd->CommonCfg.Channel)
			TdlsFrameType = RTMP_TDLS_SPECIFIC_WAIT_ACK;
		else
			TdlsFrameType = (RTMP_TDLS_SPECIFIC_WAIT_ACK + RTMP_TDLS_SPECIFIC_PKTQ_HCCA);

		NStatus = TDLS_ChannelSwitchRspAction(pAd,
												pTDLS,
												PeerChSwitchTime,
												PeerChSwitchTimeOut,
												StatusCode,
												TdlsFrameType);

		for (MTxCycle = 0; MTxCycle < 50; MTxCycle++)
		{
			RTMP_IO_READ32(pAd, TX_REPORT_CNT, &macCfg);
			TxCount = macCfg & 0x0000ffff;
			if (TxCount > 0)
			{
				DBGPRINT(RT_DEBUG_WARN, ("MTxCycle = %d, %ld !!!\n", MTxCycle, (jiffies * 1000) / OS_HZ));
				break;
			}
			else
				RtmpusecDelay(1000);
		}

		if (MTxCycle >= 50)
		{
			TDLS_KickOutHwNullFrame(pAd, PWR_ACTIVE, 0);

			NStatus = NDIS_STATUS_FAILURE;
			DBGPRINT(RT_DEBUG_ERROR, ("%s() - TDLS Transmit Channel Switch Response Fail !!!\n", __FUNCTION__));
		}
	}

	if (NStatus == NDIS_STATUS_SUCCESS)
	{
		if  (TargetChannel != pAd->CommonCfg.Channel)
		{
			BOOLEAN TimerCancelled;

			pTdlsControl->TdlsDesireChannel = TargetChannel;

			if (NewExtChannelOffset != 0)
				pTdlsControl->TdlsDesireChannelBW = NewExtChannelOffset;
			else
				pTdlsControl->TdlsDesireChannelBW = EXTCHA_NONE;

			pTDLS->ChSwitchTime = PeerChSwitchTime;
			pTdlsControl->TdlsActiveSwitchTime = PeerChSwitchTime;
			pTDLS->ChSwitchTimeout = PeerChSwitchTimeOut;
			pTdlsControl->TdlsActiveSwitchTimeOut = PeerChSwitchTimeOut;

			RTMP_OS_NETDEV_STOP_QUEUE(pAd->net_dev);
			RTMP_SET_MORE_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DEQUEUE);

			/* Cancel the timer since the received packet to me. */
			RTMPCancelTimer(&pTDLS->ChannelSwitchTimeoutTimer, &TimerCancelled);
			RTMPSetTimer(&pTDLS->ChannelSwitchTimeoutTimer, (PeerChSwitchTimeOut / 1000));

			RTMPCancelTimer(&pTdlsControl->TdlsDisableChannelSwitchTimer, &TimerCancelled);
			pTDLS->bDoingPeriodChannelSwitch = TRUE;
			pTdlsControl->bChannelSwitchWaitSuccess = TRUE;
			pTdlsControl->bDoingPeriodChannelSwitch = TRUE;

			if (RTDebugLevel < RT_DEBUG_WARN)
				RtmpusecDelay(300);
			else
				DBGPRINT(RT_DEBUG_WARN, ("1041. %ld !!!\n", (jiffies * 1000) / OS_HZ));

			TDLS_DisablePktChannel(pAd, FIFO_EDCA);

			TDLS_InitChannelRelatedValue(pAd, FALSE, TargetChannel, NewExtChannelOffset);

		}
		else
		{
			pTDLS->bDoingPeriodChannelSwitch = FALSE;
			pTdlsControl->bDoingPeriodChannelSwitch = FALSE;
			pTdlsControl->TdlsForcePowerSaveWithAP = FALSE;

			RTMP_OS_NETDEV_STOP_QUEUE(pAd->net_dev);
			RTMP_SET_MORE_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DEQUEUE);
			RtmpusecDelay(300);

			TDLS_DisablePktChannel(pAd, FIFO_HCCA);

			if (pAd->CommonCfg.CentralChannel > pAd->CommonCfg.Channel)
				TDLS_InitChannelRelatedValue(pAd, TRUE, pAd->CommonCfg.Channel, EXTCHA_ABOVE);
			else if (pAd->CommonCfg.CentralChannel < pAd->CommonCfg.Channel)
				TDLS_InitChannelRelatedValue(pAd, TRUE, pAd->CommonCfg.Channel, EXTCHA_BELOW);
			else
				TDLS_InitChannelRelatedValue(pAd, TRUE, pAd->CommonCfg.Channel, EXTCHA_NONE);

			TDLS_EnablePktChannel(pAd, FIFO_HCCA | FIFO_EDCA);

			TDLS_KickOutHwNullFrame(pAd, PWR_ACTIVE, 0);
			RTMP_CLEAR_MORE_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DEQUEUE);
			RTMP_OS_NETDEV_WAKE_QUEUE(pAd->net_dev);
		}
	}

	DBGPRINT(RT_DEBUG_TRACE, ("<=== %s()\n", __FUNCTION__));

	return;
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID
TDLS_PeerChannelSwitchRspAction(
    IN PRTMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem)
{
	PRT_802_11_TDLS pTDLS = NULL;
	INT LinkId = 0xff;
	UCHAR PeerAddr[MAC_ADDR_LEN];
	BOOLEAN TimerCancelled;
	USHORT PeerChSwitchTime;
	USHORT PeerChSwitchTimeOut;
	USHORT StatusCode = MLME_SUCCESS;
	PTDLS_STRUCT pTdlsControl = &pAd->StaCfg.TdlsInfo;

	DBGPRINT(RT_DEBUG_TRACE, ("===> %s()\n", __FUNCTION__));

	if (!INFRA_ON(pAd))
		return;

	/* Not TDLS Capable, ignore it */
	if (!IS_TDLS_SUPPORT(pAd))
		return;

	DBGPRINT(RT_DEBUG_TRACE, ("%s() - from %02x:%02x:%02x:%02x:%02x:%02x !!!\n",
					__FUNCTION__, PRINT_MAC(PeerAddr)));

	hex_dump("TDLS peer channel switch response receive pack", Elem->Msg, Elem->MsgLen);

	if (!PeerTdlsChannelSwitchRspSanity(pAd,
								Elem->Msg,
								Elem->MsgLen,
								PeerAddr,
								&StatusCode,
								&PeerChSwitchTime,
								&PeerChSwitchTimeOut))
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s() - Sanity Check Fail !!!\n", __FUNCTION__));
		return;
	}

	/* Drop not within my TDLS Table that created before ! */
	LinkId = TDLS_SearchLinkId(pAd, PeerAddr);
	if (LinkId == -1 || LinkId == MAX_NUM_OF_TDLS_ENTRY)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s() - can not find the MAC on TDLS Table !!!\n", __FUNCTION__));
		return;
	}

	/* Point to the current Link ID */
	pTDLS = &pAd->StaCfg.TdlsInfo.TDLSEntry[LinkId];

	if ((pTDLS->ChannelSwitchCurrentState == TDLS_CHANNEL_SWITCH_NONE) &&
		(StatusCode == MLME_REQUEST_DECLINED))
	{
		DBGPRINT(RT_DEBUG_ERROR,
						("%s() - received a failed StatusCode = %d on Unsolicited response !!!\n",
						__FUNCTION__, StatusCode));
		return;
	}

	if (StatusCode == MLME_REQUEST_DECLINED)
	{
		if ((pTdlsControl->TdlsChannelSwitchRetryCount > 0) &&
			(pTDLS->bDoingPeriodChannelSwitch) &&
			(pTdlsControl->bDoingPeriodChannelSwitch))
		{
			pTdlsControl->TdlsChannelSwitchRetryCount--;

			DBGPRINT(RT_DEBUG_ERROR,
							("%s() - received a failed StatusCode = %d  re-try again !!!\n",
							__FUNCTION__, StatusCode));
		}
		else
		{
			pTDLS->bDoingPeriodChannelSwitch = FALSE;
			pTdlsControl->bDoingPeriodChannelSwitch = FALSE;
			pTdlsControl->TdlsForcePowerSaveWithAP = FALSE;
		}

		TDLS_KickOutHwNullFrame(pAd, PWR_ACTIVE, 0);

		DBGPRINT(RT_DEBUG_ERROR,("%s() - received a failed StatusCode = %d !!!\n",
						__FUNCTION__, StatusCode ));
		return;
	}

	if (StatusCode == MLME_SUCCESS)
	{
		if (pTDLS->ChannelSwitchCurrentState == TDLS_CHANNEL_SWITCH_NONE)
		{
			if (pTdlsControl->bChannelSwitchInitiator)
			{
				DBGPRINT(RT_DEBUG_TRACE,("%s() - i am Initiator !!!\n", __FUNCTION__));
			}
			else
			{
				DBGPRINT(RT_DEBUG_TRACE,("%s() - i am responder!!!\n",  __FUNCTION__));
			}

			//if (pAd->LatchRfRegs.Channel != pAd->CommonCfg.CentralChannel)
			if (pAd->StaCfg.TdlsInfo.TdlsAsicOperateChannel != pAd->CommonCfg.Channel)
			{
				DBGPRINT(RT_DEBUG_WARN, ("106. %ld !!!\n", (jiffies * 1000) / OS_HZ));

				NdisGetSystemUpTime(&pTdlsControl->TdlsGoBackStartTime);

				RTMP_OS_NETDEV_STOP_QUEUE(pAd->net_dev);
				RTMP_SET_MORE_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DEQUEUE);
				TDLS_DisablePktChannel(pAd, FIFO_HCCA);

				if (pAd->CommonCfg.CentralChannel > pAd->CommonCfg.Channel)
					TDLS_InitChannelRelatedValue(pAd, TRUE, pAd->CommonCfg.Channel, EXTCHA_ABOVE);
				else if (pAd->CommonCfg.CentralChannel < pAd->CommonCfg.Channel)
					TDLS_InitChannelRelatedValue(pAd, TRUE, pAd->CommonCfg.Channel, EXTCHA_BELOW);
				else
					TDLS_InitChannelRelatedValue(pAd, TRUE, pAd->CommonCfg.Channel, EXTCHA_NONE);

				TDLS_EnablePktChannel(pAd, FIFO_HCCA | FIFO_EDCA);

				TDLS_KickOutHwNullFrame(pAd, PWR_ACTIVE, 0);
				RTMP_CLEAR_MORE_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DEQUEUE);
				RTMP_OS_NETDEV_WAKE_QUEUE(pAd->net_dev);
			}
		}
		else
		{
			if (pTdlsControl->TdlsDesireChannel != pAd->CommonCfg.Channel)
			{
				if (pTdlsControl->bChannelSwitchInitiator)
				{
					pTdlsControl->TdlsChannelSwitchPairCount--;
					pTdlsControl->TdlsChannelSwitchRetryCount = 10;

					if (TDLS_CHANNEL_SWITCH_TIME >= PeerChSwitchTime)
						PeerChSwitchTime = TDLS_CHANNEL_SWITCH_TIME;

					if (TDLS_CHANNEL_SWITCH_TIMEOUT >= PeerChSwitchTimeOut)
						PeerChSwitchTimeOut = TDLS_CHANNEL_SWITCH_TIMEOUT;

					pTDLS->ChSwitchTime = PeerChSwitchTime;
					pTdlsControl->TdlsActiveSwitchTime = PeerChSwitchTime;
					pTDLS->ChSwitchTimeout = PeerChSwitchTimeOut;
					pTdlsControl->TdlsActiveSwitchTimeOut = PeerChSwitchTimeOut;
					pTDLS->ChannelSwitchCurrentState = TDLS_CHANNEL_SWITCH_NONE;

					RTMP_OS_NETDEV_STOP_QUEUE(pAd->net_dev);
					RTMP_SET_MORE_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DEQUEUE);

					RTMPCancelTimer(&pTdlsControl->TdlsDisableChannelSwitchTimer, &TimerCancelled);
					//Cancel the timer since the received packet to me.
					RTMPCancelTimer(&pTDLS->ChannelSwitchTimer, &TimerCancelled);
					RTMPSetTimer(&pTDLS->ChannelSwitchTimer, (PeerChSwitchTime / 1000));
					NdisGetSystemUpTime(&pTdlsControl->TdlsChSwSilenceTime);
					if (RTDebugLevel < RT_DEBUG_WARN)
						RtmpusecDelay(300);
					else
						DBGPRINT(RT_DEBUG_ERROR, ("104. %ld !!!\n", (jiffies * 1000) / OS_HZ));
					TDLS_DisablePktChannel(pAd, FIFO_EDCA);

					TDLS_InitChannelRelatedValue(pAd, FALSE, pTdlsControl->TdlsDesireChannel, pTdlsControl->TdlsDesireChannelBW);
				}
			}
			else
			{
				pTDLS->bDoingPeriodChannelSwitch = FALSE;
				pTdlsControl->bDoingPeriodChannelSwitch = FALSE;
				pTdlsControl->TdlsForcePowerSaveWithAP = FALSE;

				if (pTdlsControl->bChannelSwitchInitiator)
				{
					pTdlsControl->bChannelSwitchInitiator = FALSE;
					DBGPRINT(RT_DEBUG_WARN, ("%s() - i am channel switch Initiator !!!\n", __FUNCTION__));
				}
				else
				{
					DBGPRINT(RT_DEBUG_WARN, ("%s() - i am channel switch responder!!!\n",  __FUNCTION__));
				}

				RTMPCancelTimer(&pTdlsControl->TdlsDisableChannelSwitchTimer, &TimerCancelled);

				//if (pAd->LatchRfRegs.Channel != pAd->CommonCfg.CentralChannel)
				if (pAd->StaCfg.TdlsInfo.TdlsAsicOperateChannel != pAd->CommonCfg.Channel)
				{
					RTMP_OS_NETDEV_STOP_QUEUE(pAd->net_dev);
					RTMP_SET_MORE_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DEQUEUE);
					TDLS_DisablePktChannel(pAd, FIFO_HCCA);

					if (pAd->CommonCfg.CentralChannel > pAd->CommonCfg.Channel)
						TDLS_InitChannelRelatedValue(pAd, TRUE, pAd->CommonCfg.Channel, EXTCHA_ABOVE);
					else if (pAd->CommonCfg.CentralChannel < pAd->CommonCfg.Channel)
						TDLS_InitChannelRelatedValue(pAd, TRUE, pAd->CommonCfg.Channel, EXTCHA_BELOW);
					else
						TDLS_InitChannelRelatedValue(pAd, TRUE, pAd->CommonCfg.Channel, EXTCHA_NONE);

					TDLS_EnablePktChannel(pAd, FIFO_HCCA | FIFO_EDCA);

					RTMP_CLEAR_MORE_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DEQUEUE);
					RTMP_OS_NETDEV_WAKE_QUEUE(pAd->net_dev);
				}

				TDLS_KickOutHwNullFrame(pAd, PWR_ACTIVE, 0);
			}
		}
	}

	DBGPRINT(RT_DEBUG_TRACE, ("<=== %s()\n", __FUNCTION__));

	return;
}

VOID
TDLS_ForceSendChannelSwitchResponse(
    IN PRTMP_ADAPTER pAd, 
    IN PRT_802_11_TDLS pTDLS)
{
	DBGPRINT(RT_DEBUG_TRACE,("===> %s() \n", __FUNCTION__));

	if (INFRA_ON(pAd))
	{
		/* Build TDLS channel switch Response Frame */
		UCHAR TDLS_ETHERTYPE[] = {0x89, 0x0d};
		UCHAR Header802_3[14];
		PUCHAR pOutBuffer = NULL;
		ULONG FrameLen = 0;
		ULONG TempLen;
		UCHAR RemoteFrameType = PROTO_NAME_TDLS;
		NDIS_STATUS	NStatus = NDIS_STATUS_SUCCESS;

		MAKE_802_3_HEADER(Header802_3, pTDLS->MacAddr, pAd->CurrentAddress, TDLS_ETHERTYPE);

		/* Allocate buffer for transmitting message */
		NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);

		if (NStatus != NDIS_STATUS_SUCCESS)
		{
			DBGPRINT(RT_DEBUG_ERROR,("ACT - %s() allocate memory failed \n", __FUNCTION__));
			return;
		}

		MakeOutgoingFrame(pOutBuffer,		&TempLen,
							1,				&RemoteFrameType,
							END_OF_ARGS);

		FrameLen = FrameLen + TempLen;

		TDLS_BuildChannelSwitchResponse(pAd,
										pOutBuffer,
										&FrameLen,
										pTDLS,
										pTDLS->ChSwitchTime,
										pTDLS->ChSwitchTimeout,
										MLME_SUCCESS);
		TDLS_SendChannelSwitchActionFrame(pAd,
										&pAd->MacTab.Content[pTDLS->MacTabMatchWCID],
										Header802_3,
										LENGTH_802_3,
										pOutBuffer,
										(UINT)FrameLen,
										RTMP_TDLS_SPECIFIC_NOACK);
		if (NStatus != NDIS_STATUS_SUCCESS)	
			DBGPRINT(RT_DEBUG_ERROR,("%s - Build Channel Switch Response Fail !!!\n", __FUNCTION__));
		else
			hex_dump("TDLS send channel switch response pack", pOutBuffer, FrameLen);

		MlmeFreeMemory(pAd, pOutBuffer);
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR,("%s - TDLS only support infra mode !!!\n", __FUNCTION__));
	}

	DBGPRINT(RT_DEBUG_TRACE,("<=== %s() \n", __FUNCTION__));

	return;
}

VOID
TDLS_SendChannelSwitchActionFrame(
    IN  PRTMP_ADAPTER pAd,
    IN  PMAC_TABLE_ENTRY pEntry,
    IN  PUCHAR pHeader802_3,
    IN  UINT HdrLen,
    IN  PUCHAR pData,
    IN  UINT DataLen,
    IN  UCHAR FrameType)
{
	PNDIS_PACKET    pPacket;
	NDIS_STATUS     Status;

	DBGPRINT(RT_DEBUG_TRACE,("===> %s() \n", __FUNCTION__));

	if ((!pEntry) || (!IS_ENTRY_TDLS(pEntry)))
		return;

	do {
		/* build a NDIS packet*/
		Status = RTMPAllocateNdisPacket(pAd, &pPacket, pHeader802_3, HdrLen, pData, DataLen);
		if (Status != NDIS_STATUS_SUCCESS)
			break;


		RTMP_SET_PACKET_NET_DEVICE_MBSSID(pPacket, MAIN_MBSSID);	/* set a default value*/
		if(pEntry->apidx != 0)
			RTMP_SET_PACKET_NET_DEVICE_MBSSID(pPacket, pEntry->apidx);

		RTMP_SET_PACKET_WCID(pPacket, (UCHAR)pEntry->Aid);
		RTMP_SET_PACKET_MOREDATA(pPacket, FALSE);

		if (FrameType > 0)
			RTMP_SET_TDLS_SPECIFIC_PACKET(pPacket, FrameType);
		else
			RTMP_SET_TDLS_SPECIFIC_PACKET(pPacket, 0);

		/* send out the packet*/
		Status = STASendPacket(pAd, pPacket);
		if (Status == NDIS_STATUS_SUCCESS)
		{
			UCHAR   Index;

			/* Dequeue one frame from TxSwQueue0..3 queue and process it*/
			/* There are three place calling dequeue for TX ring.*/
			/* 1. Here, right after queueing the frame.*/
			/* 2. At the end of TxRingTxDone service routine.*/
			/* 3. Upon NDIS call RTMPSendPackets*/
			if((!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)) && 
				(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS)))
			{
				for(Index = 0; Index < 5; Index ++)
					if(pAd->TxSwQueue[Index].Number > 0)
						RTMPDeQueuePacket(pAd, FALSE, Index, MAX_TX_PROCESS);
			}
		}
	} while (FALSE);

	DBGPRINT(RT_DEBUG_TRACE,("<=== %s() \n", __FUNCTION__));

	return;
}

VOID TDLS_OffChExpired(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3) 
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)FunctionContext;
	PTDLS_STRUCT pTdlsControl = &pAd->StaCfg.TdlsInfo;
	UCHAR idx = 0;
	ULONG Now, GapJiffies, BPtoJiffies, CurTime;

	if ((pAd != NULL) && INFRA_ON(pAd))
	{
		if (pAd->StaCfg.DtimPeriod > 1)
		{
			NdisGetSystemUpTime(&Now);
			BPtoJiffies = (((pAd->CommonCfg.BeaconPeriod * 1024 / 1000) * OS_HZ) / 1000);

			if (RTMP_TIME_AFTER(pAd->StaCfg.LastBeaconRxTime + (BPtoJiffies * pAd->StaCfg.DtimPeriod), Now))
			{
				GapJiffies = (pAd->StaCfg.LastBeaconRxTime + (pAd->StaCfg.DtimPeriod* BPtoJiffies) - Now);
				CurTime = (GapJiffies * 1000) / OS_HZ;

				if (CurTime >= (pAd->CommonCfg.BeaconPeriod + 20))
				{
					RTMPModTimer(&pTdlsControl->TdlsPeriodGoBackBaseChTimer, pAd->CommonCfg.BeaconPeriod);
					goto done;
				}
				else
				{
					if (CurTime > 20)
					{
						RTMPModTimer(&pTdlsControl->TdlsPeriodGoBackBaseChTimer, CurTime - 20);
						goto done;
					}
				}
			}
		}

		//if (pAd->LatchRfRegs.Channel != pAd->CommonCfg.CentralChannel)
		if (pAd->StaCfg.TdlsInfo.TdlsAsicOperateChannel != pAd->CommonCfg.Channel)
		{
			BOOLEAN TimerCancelled;

			RTMP_OS_NETDEV_STOP_QUEUE(pAd->net_dev);

			if (pTdlsControl->bChannelSwitchInitiator)
				DBGPRINT(RT_DEBUG_WARN, ("200. Initiator %ld !!!\n", (jiffies * 1000) / OS_HZ));
			else
				DBGPRINT(RT_DEBUG_WARN, ("201. Responder %ld !!!\n", (jiffies * 1000) / OS_HZ));
		
			RTMPCancelTimer(&pTdlsControl->TdlsDisableChannelSwitchTimer, &TimerCancelled);
			RTMPSetTimer(&pTdlsControl->TdlsDisableChannelSwitchTimer, ((pAd->StaCfg.DtimPeriod * pAd->CommonCfg.BeaconPeriod) - 11));
		
			for (idx = 0; idx < MAX_NUM_OF_TDLS_ENTRY; idx++)
			{
				PRT_802_11_TDLS pTdlsEntry = &pAd->StaCfg.TdlsInfo.TDLSEntry[idx];
		
				if (pTdlsEntry->Valid)
				{
					TDLS_ForceSendChannelSwitchResponse(pAd, pTdlsEntry);
				}
			}

			RtmpusecDelay(4000);
			NdisGetSystemUpTime(&pTdlsControl->TdlsGoBackStartTime);
		
			RTMP_SET_MORE_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DEQUEUE);

			TDLS_DisablePktChannel(pAd, FIFO_HCCA);

			if (pAd->CommonCfg.CentralChannel > pAd->CommonCfg.Channel)
				TDLS_InitChannelRelatedValue(pAd, TRUE, pAd->CommonCfg.Channel, EXTCHA_ABOVE);
			else if (pAd->CommonCfg.CentralChannel < pAd->CommonCfg.Channel)
				TDLS_InitChannelRelatedValue(pAd, TRUE, pAd->CommonCfg.Channel, EXTCHA_BELOW);
			else
				TDLS_InitChannelRelatedValue(pAd, TRUE, pAd->CommonCfg.Channel, EXTCHA_NONE);

			TDLS_EnablePktChannel(pAd, FIFO_HCCA | FIFO_EDCA);
		
			TDLS_KickOutHwNullFrame(pAd, PWR_ACTIVE, 0);
			RTMP_CLEAR_MORE_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DEQUEUE);
			RTMP_OS_NETDEV_WAKE_QUEUE(pAd->net_dev);
		}
		else
		{
			TDLS_EnablePktChannel(pAd, FIFO_HCCA | FIFO_EDCA);
			DBGPRINT(RT_DEBUG_INFO, ("I am lready on Base Channel :(%d) !!!\n", pAd->CommonCfg.Channel));
		}
	}

done:

	return;
}

VOID TDLS_BaseChExpired(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3) 
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)FunctionContext;
	PTDLS_STRUCT pTdlsControl = &pAd->StaCfg.TdlsInfo;
	PRT_802_11_TDLS	pTdlsEntry = NULL;

	if ((pAd != NULL) && INFRA_ON(pAd))
	{
		UCHAR	idx = 0;
		MLME_TDLS_CH_SWITCH_STRUCT	TdlsChSwitch;

		if (pTdlsControl->bChannelSwitchInitiator == FALSE)
			return;

		for (idx = 0; idx < MAX_NUM_OF_TDLS_ENTRY; idx++)
		{
			pTdlsEntry = &pAd->StaCfg.TdlsInfo.TDLSEntry[idx];
	
			if (pTdlsEntry->Valid && pTdlsEntry->bDoingPeriodChannelSwitch)
			{
				NdisZeroMemory(&TdlsChSwitch, sizeof(MLME_TDLS_CH_SWITCH_STRUCT));
				COPY_MAC_ADDR(TdlsChSwitch.PeerMacAddr, pTdlsEntry->MacAddr);
				TdlsChSwitch.TargetChannel = pTdlsControl->TdlsDesireChannel;
				TdlsChSwitch.TargetChannelBW = pTdlsControl->TdlsDesireChannelBW;

				DBGPRINT(RT_DEBUG_WARN, ("102. %d,%d,%ld !!!\n",
							pTdlsControl->TdlsDesireChannel, pTdlsControl->TdlsDesireChannelBW, (jiffies * 1000) / OS_HZ));

				MlmeEnqueue(pAd, 
							TDLS_CHSW_STATE_MACHINE, 
							MT2_MLME_TDLS_CH_SWITCH_REQ, 
							sizeof(MLME_TDLS_CH_SWITCH_STRUCT), 
							&TdlsChSwitch,
							0);

				RTMP_MLME_HANDLER(pAd);
			}
		}
	}
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID
TDLS_ChannelSwitchTimeAction(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3) 
{
	PRT_802_11_TDLS	pTDLS = (PRT_802_11_TDLS)FunctionContext;
	PRTMP_ADAPTER pAd = pTDLS->pAd;
		UINT32 macCfg, TxCount;
		UINT32 MTxCycle;
		UINT16 MaxWaitingTime;
	ULONG Now, RealSwitchTime;

	DBGPRINT(RT_DEBUG_TRACE, ("Channel switch procedure for (%02x:%02x:%02x:%02x:%02x:%02x)\n",
				PRINT_MAC(pTDLS->MacAddr)));

	NdisGetSystemUpTime(&Now);

	RealSwitchTime = ((Now - pAd->StaCfg.TdlsInfo.TdlsChSwSilenceTime) * 1000) / OS_HZ;
	
	if (RealSwitchTime < (pTDLS->ChSwitchTime / 1000))
		RtmpOsMsDelay((pTDLS->ChSwitchTime / 1000) - RealSwitchTime);

	TDLS_EnableMacTx(pAd);
	RTMP_IO_READ32(pAd, TX_REPORT_CNT, &macCfg);
	RtmpusecDelay(2000);

	DBGPRINT(RT_DEBUG_ERROR, ("105. %ld !!!\n", (jiffies * 1000) / OS_HZ));

	TDLS_SendOutNullFrame(pAd, &pAd->MacTab.Content[pTDLS->MacTabMatchWCID], TRUE, TRUE);

	MaxWaitingTime = (pTDLS->ChSwitchTimeout - pTDLS->ChSwitchTime);
	for (MTxCycle = 0; MTxCycle < MaxWaitingTime; MTxCycle+= 1000)
	{
		RTMP_IO_READ32(pAd, TX_REPORT_CNT, &macCfg);
		TxCount = macCfg & 0x0000ffff;

		if (TxCount > 0)
			break;
		else
			RtmpusecDelay(1000);
	}

	if (MTxCycle == MaxWaitingTime)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("24. %ld @!!!\n", (jiffies * 1000) / OS_HZ));

		NdisGetSystemUpTime(&pAd->StaCfg.TdlsInfo.TdlsGoBackStartTime);

		if (pAd->CommonCfg.CentralChannel > pAd->CommonCfg.Channel)
			TDLS_InitChannelRelatedValue(pAd, TRUE, pAd->CommonCfg.Channel, EXTCHA_ABOVE);
		else if (pAd->CommonCfg.CentralChannel < pAd->CommonCfg.Channel)
			TDLS_InitChannelRelatedValue(pAd, TRUE, pAd->CommonCfg.Channel, EXTCHA_BELOW);
		else
			TDLS_InitChannelRelatedValue(pAd, TRUE, pAd->CommonCfg.Channel, EXTCHA_NONE);

		TDLS_KickOutHwNullFrame(pAd, PWR_ACTIVE, 0);
		TDLS_EnablePktChannel(pAd, FIFO_HCCA | FIFO_EDCA);
	}
	else
		TDLS_EnablePktChannel(pAd, FIFO_HCCA);

	RTMP_CLEAR_MORE_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DEQUEUE);
	RTMP_OS_NETDEV_WAKE_QUEUE(pAd->net_dev);

	return;
}

/*
==========================================================================
	Description:
	    
	IRQL = PASSIVE_LEVEL
==========================================================================
*/
VOID
TDLS_ChannelSwitchTimeOutAction(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3) 
{
	PRT_802_11_TDLS	pTDLS = (PRT_802_11_TDLS)FunctionContext;
	PRTMP_ADAPTER pAd = pTDLS->pAd;
	PTDLS_STRUCT pTdlsControl = &pAd->StaCfg.TdlsInfo;

	DBGPRINT(RT_DEBUG_WARN,
				("Channel switch timeout , terminate the channel switch procedure (%02x:%02x:%02x:%02x:%02x:%02x)\n",
				PRINT_MAC(pTDLS->MacAddr)));

	DBGPRINT(RT_DEBUG_ERROR, ("Channel switch timeout\n"));

	pTdlsControl->bChannelSwitchWaitSuccess = FALSE;
	pTdlsControl->bDoingPeriodChannelSwitch = FALSE;

	if (pAd->CommonCfg.CentralChannel > pAd->CommonCfg.Channel)
		TDLS_InitChannelRelatedValue(pAd, TRUE, pAd->CommonCfg.Channel, EXTCHA_ABOVE);
	else if (pAd->CommonCfg.CentralChannel < pAd->CommonCfg.Channel)
		TDLS_InitChannelRelatedValue(pAd, TRUE, pAd->CommonCfg.Channel, EXTCHA_BELOW);
	else
		TDLS_InitChannelRelatedValue(pAd, TRUE, pAd->CommonCfg.Channel, EXTCHA_NONE);

	TDLS_KickOutHwNullFrame(pAd, PWR_ACTIVE, 0);
	TDLS_EnablePktChannel(pAd, FIFO_HCCA | FIFO_EDCA);
	RTMP_CLEAR_MORE_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DEQUEUE);
	RTMP_OS_NETDEV_WAKE_QUEUE(pAd->net_dev);

	return;
}

VOID
TDLS_DisablePeriodChannelSwitchAction(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3) 
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)FunctionContext;
	PTDLS_STRUCT pTdlsControl = &pAd->StaCfg.TdlsInfo;

	if (!(pAd->StaCfg.TdlsInfo.bChannelSwitchInitiator))
	{
		pTdlsControl->bChannelSwitchInitiator = FALSE;
		pTdlsControl->bDoingPeriodChannelSwitch = FALSE;
	}

	pTdlsControl->TdlsForcePowerSaveWithAP = FALSE;

	if (pAd->StaCfg.TdlsInfo.TdlsAsicOperateChannel != pAd->CommonCfg.Channel)
	{
		RTMP_OS_NETDEV_STOP_QUEUE(pAd->net_dev);
		RTMP_SET_MORE_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DEQUEUE);
		TDLS_DisablePktChannel(pAd, FIFO_HCCA);

		if (pAd->CommonCfg.CentralChannel > pAd->CommonCfg.Channel)
			TDLS_InitChannelRelatedValue(pAd, TRUE, pAd->CommonCfg.Channel, EXTCHA_ABOVE);
		else if (pAd->CommonCfg.CentralChannel < pAd->CommonCfg.Channel)
			TDLS_InitChannelRelatedValue(pAd, TRUE, pAd->CommonCfg.Channel, EXTCHA_BELOW);
		else
			TDLS_InitChannelRelatedValue(pAd, TRUE, pAd->CommonCfg.Channel, EXTCHA_NONE);
	}

#ifdef RTMP_MAC_PCI
	RTMP_SEM_LOCK(&pAd->StaCfg.TdlsInfo.TdlsInitChannelLock);
#endif /* RTMP_MAC_PCI */
	pAd->StaCfg.TdlsInfo.TdlsCurrentTargetChannel = pAd->CommonCfg.Channel;
#ifdef RTMP_MAC_PCI
	RTMP_SEM_UNLOCK(&pAd->StaCfg.TdlsInfo.TdlsInitChannelLock);
#endif /* RTMP_MAC_PCI */

	TDLS_EnableMacTxRx(pAd);
	TDLS_KickOutHwNullFrame(pAd, PWR_ACTIVE, 0);
	TDLS_EnablePktChannel(pAd, FIFO_HCCA | FIFO_EDCA);
	RTMP_CLEAR_MORE_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DEQUEUE);
	RTMP_OS_NETDEV_WAKE_QUEUE(pAd->net_dev);

	return;
}
#endif /* DOT11Z_TDLS_SUPPORT */


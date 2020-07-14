/*
 ***************************************************************************
 * Ralink Tech Inc. 
 * 5F., No.36, Taiyuan St., Jhubei City, 
 * Hsinchu County 302, 
 * Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2008, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	ft_auth.c

	Abstract:
	Handle State Machine for 802.11r Fast BSS Transition Over-The-Air auth frame.
	
*/
#ifdef DOT11R_FT_SUPPORT

#include "rt_config.h"
#include "ft.h"

BOOLEAN PeerFtAuthRspSanity(
    IN  PRTMP_ADAPTER 	pAd,
    IN  PUCHAR		 	ApAddr,
    IN  VOID 			*pMsg, 
    IN  ULONG 			MsgLen,
    OUT MAC_TABLE_ENTRY *pEntry) 
{
	PEID_STRUCT     pEid;
	USHORT          Length = 0;
	UCHAR			Sanity = 0;
	PFRAME_802_11 	pFrame = (PFRAME_802_11)pMsg;
	FT_MDIE			MdIe;
	UINT8			PMKR0Name[16];

	pEntry = &pAd->MacTab.Content[MCAST_WCID];

	/*
		Alg: 2 bytes
		Seq: 2 bytes
		Status: 2 bytes
	*/
    pEid = (PEID_STRUCT) &pFrame->Octet[6];
	while ((Length + 2 + (USHORT)pEid->Len) <= (MsgLen - LENGTH_802_11 - 6))
	{
		switch(pEid->Eid)
		{
			case IE_FT_MDIE:
				if (pEid->Len == sizeof(FT_MDIE))
				{
					NdisZeroMemory(&MdIe, sizeof(FT_MDIE));
					NdisMoveMemory(&MdIe, &pEid->Octet[0], pEid->Len);
					FT_SET_MDID(pEntry->MdIeInfo.MdId, MdIe.MdId);
					pEntry->MdIeInfo.FtCapPlc.word = MdIe.FtCapPlc.word;
					Sanity |= 0x01;
				}
				break;
			case IE_FT_FTIE:
				if (pEid->Len)
				{
					NdisMoveMemory(&pAd->MlmeAux.FtIeInfo.ANonce[0], &pEid->Octet[18], 32);
					FT_FTIeParse(pEid->Len, 
								 (PFT_FTIE)pEid/*(PFT_FTIE)&pEid->Octet[0]*/, 
								 &pEntry->FtIeInfo.R1khId[0], 
								 &pEntry->FtIeInfo.GtkLen, 
								 &pEntry->FtIeInfo.GtkSubIE[0], 
								 &pEntry->FtIeInfo.R0khIdLen, 
								 &pEntry->FtIeInfo.R0khId[0]);
					Sanity |= 0x02;
				}
				break;

			case IE_RSN:
				if (pEid->Len > LEN_PMK_NAME)
				{
					/* Check PMKR0Name */
					NdisMoveMemory(&PMKR0Name[0], &pEid->Octet[pEid->Len - LEN_PMK_NAME], LEN_PMK_NAME);
					if (NdisEqualMemory(&PMKR0Name[0], &pAd->StaCfg.Dot11RCommInfo.PMKR0Name[0], LEN_PMK_NAME))
						Sanity |= 0x04;
				}
				break;
        }
		Length = Length + 2 + (USHORT)pEid->Len;  /* Eid[1] + Len[1]+ content[Len] */
		pEid = (PEID_STRUCT)((UCHAR*)pEid + 2 + pEid->Len);        
	}

	if ((Sanity & 0x01) == 0)
	{
		DBGPRINT(RT_DEBUG_WARN, ("PeerFtAuthRspSanity - missing field, Sanity=0x%02x\n", Sanity));
		return FALSE;
	}

	if (pAd->StaCfg.AuthMode >= Ndis802_11AuthModeWPA)
	{
		if ((Sanity & 0x07) == 0)
		{
			DBGPRINT(RT_DEBUG_WARN, ("PeerFtAuthRspSanity - missing field, Sanity=0x%02x\n", Sanity));
			return FALSE;
		}
		else
		{
			return TRUE;
		}
	}

    return TRUE;
}

/*
    ==========================================================================
    Description:
        authenticate state machine init, including state transition and timer init
    Parameters:
        Sm - pointer to the auth state machine
    Note:
        The state machine looks like this
        
								FT_OTA_AUTH_REQ_IDLE		FT_OTA_AUTH_WAIT_RESP			FT_OTA_AUTH_WAIT_ACK
	FT_OTA_MT2_MLME_AUTH_REQ	FT_OTA_MT2_MLME_AUTH_REQ	invalid_state_when_auth			invalid_state_when_auth
	FT_OTA_MT2_PEER_AUTH_EVEN	Drop						peer_auth_even_at_seq2_action	peer_auth_even_at_seq4_action
	FT_OTA_MT2_AUTH_TIMEOUT		Drop						auth_timeout_action				auth_timeout_action
        
	IRQL = PASSIVE_LEVEL

    ==========================================================================
 */

void FT_OTA_AuthStateMachineInit(
    IN PRTMP_ADAPTER pAd, 
    IN STATE_MACHINE *Sm, 
    OUT STATE_MACHINE_FUNC Trans[]) 
{
    StateMachineInit(Sm, Trans, FT_OTA_MAX_AUTH_STATE, FT_OTA_MAX_AUTH_MSG, (STATE_MACHINE_FUNC)Drop, FT_OTA_AUTH_REQ_IDLE, FT_OTA_AUTH_MACHINE_BASE);
     
    /* the first column */
    StateMachineSetAction(Sm, FT_OTA_AUTH_REQ_IDLE, FT_OTA_MT2_MLME_AUTH_REQ, (STATE_MACHINE_FUNC)FT_OTA_MlmeAuthReqAction);

    /* the second column */
    StateMachineSetAction(Sm, FT_OTA_AUTH_WAIT_RESP, FT_OTA_MT2_MLME_AUTH_REQ, (STATE_MACHINE_FUNC)FT_OTA_InvalidStateWhenAuth);
    StateMachineSetAction(Sm, FT_OTA_AUTH_WAIT_RESP, FT_OTA_MT2_PEER_AUTH_EVEN, (STATE_MACHINE_FUNC)FT_OTA_PeerAuthRspAction);
    
    /* the third column */
    StateMachineSetAction(Sm, FT_OTA_AUTH_WAIT_ACK, FT_OTA_MT2_MLME_AUTH_REQ, (STATE_MACHINE_FUNC)FT_OTA_InvalidStateWhenAuth);
    StateMachineSetAction(Sm, FT_OTA_AUTH_WAIT_ACK, FT_OTA_MT2_PEER_AUTH_EVEN, (STATE_MACHINE_FUNC)FT_OTA_PeerAuthAckAction);
    
	RTMPInitTimer(pAd, &pAd->MlmeAux.FtOtaAuthTimer, GET_TIMER_FUNCTION(FT_OTA_AuthTimeout), pAd, FALSE);
}

/*
    ==========================================================================
    Description:
        function to be executed at timer thread when auth timer expires
        
	IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
VOID FT_OTA_AuthTimeout(
    IN PVOID SystemSpecific1, 
    IN PVOID FunctionContext, 
    IN PVOID SystemSpecific2, 
    IN PVOID SystemSpecific3) 
{
    RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)FunctionContext;
	USHORT Status;
    
    DBGPRINT(RT_DEBUG_TRACE,("FT_OTA_AUTH - FT_OTA_AuthTimeout\n"));
	
	/* Do nothing if the driver is starting halt state. */
	/* This might happen when timer already been fired before cancel timer with mlmehalt */
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST))
		return;

	pAd->StaCfg.Dot11RCommInfo.FtRspSuccess = FT_OTA_RESPONSE;
    pAd->Mlme.FtOtaAuthMachine.CurrState = FT_OTA_AUTH_REQ_IDLE;
    Status = MLME_REJ_TIMEOUT;
    MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_AUTH_CONF, 2, &Status, 0);
    RTMP_MLME_HANDLER(pAd);
}


/*
    ==========================================================================
    Description:
        
	IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
VOID FT_OTA_MlmeAuthReqAction(
    IN PRTMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
	USHORT			Status;
	NDIS_STATUS		NStatus;
	PUCHAR			pOutBuffer = NULL;
	ULONG			FrameLen = 0;
	PMLME_FT_OTA_AUTH_REQ_STRUCT	pFtOtaAuthReq = (MLME_FT_OTA_AUTH_REQ_STRUCT *)Elem->Msg;
    
	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  /*Get an unused nonpaged memory */
    if(NStatus != NDIS_STATUS_SUCCESS) 
    {
        DBGPRINT(RT_DEBUG_TRACE, ("FT_OTA_AUTH - FT_OTA_MlmeAuthReqAction allocate memory failed\n"));
        pAd->Mlme.FtOtaAuthMachine.CurrState = FT_OTA_AUTH_REQ_IDLE;
        Status = MLME_FAIL_NO_RESOURCE;
        MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_AUTH_CONF, 2, &Status, 0);
        return;
    }
	
	/* MDIE */
	FT_InsertMdIE(pAd, pOutBuffer, &FrameLen, pFtOtaAuthReq->MdIe.MdId, pFtOtaAuthReq->MdIe.FtCapPlc);

	/* work in an RSN */
	if (pAd->StaCfg.AuthMode >= Ndis802_11AuthModeWPA)
	{
		FT_ConstructAuthReqInRsn(pAd, pOutBuffer, &FrameLen);
	}

	if (AUTH_ReqSend(pAd, Elem, &pAd->MlmeAux.FtOtaAuthTimer, "FT_OTA_AUTH", 1, pOutBuffer, FrameLen))
		pAd->Mlme.FtOtaAuthMachine.CurrState = FT_OTA_AUTH_WAIT_RESP;

	MlmeFreeMemory(pAd, pOutBuffer);	
}

/*
    ==========================================================================
    Description:
        
	IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
VOID FT_OTA_PeerAuthRspAction(
    IN PRTMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    UCHAR			Addr2[MAC_ADDR_LEN];
    USHORT			Seq, Status, Alg;
    BOOLEAN			TimerCancelled;
    PUCHAR			pOutBuffer = NULL;
    NDIS_STATUS		NStatus;
    ULONG			FrameLen = 0;
	UINT8			ptk_len;
	UCHAR			EleID;
	MAC_TABLE_ENTRY *pEntry = NULL;

    if (PeerAuthSanity(pAd, Elem->Msg, Elem->MsgLen, Addr2, &Alg, &Seq, &Status, NULL))
    {
        if (MAC_ADDR_EQUAL(pAd->MlmeAux.Bssid, Addr2) && 
			(Alg == AUTH_MODE_FT) &&
			(Seq == 2))
        {
            DBGPRINT(RT_DEBUG_TRACE, ("FT_OTA_AUTH - Receive FT_OTA_AUTH_RSP to me (Alg=%d, Status=%d)\n", Alg, Status));
            RTMPCancelTimer(&pAd->MlmeAux.FtOtaAuthTimer, &TimerCancelled);
            pAd->StaCfg.Dot11RCommInfo.FtRspSuccess = FT_OTA_RESPONSE;
            if ((Status == MLME_SUCCESS) &&
				(PeerFtAuthRspSanity(pAd, Addr2, Elem->Msg, Elem->MsgLen, pEntry) == TRUE)) 
            {
            	DBGPRINT(RT_DEBUG_TRACE, ("%s : Ready to derive PMK \n", __FUNCTION__));

				pEntry = &pAd->MacTab.Content[MCAST_WCID];
				NdisMoveMemory(pEntry->SNonce, pAd->MlmeAux.FtIeInfo.SNonce, 32);

				/* Get ANonce from authentication-response */
				NdisMoveMemory(pEntry->ANonce, pAd->MlmeAux.FtIeInfo.ANonce, 32);

				FT_DerivePMKR1(pAd->StaCfg.Dot11RCommInfo.PMKR0, 
							 pAd->StaCfg.Dot11RCommInfo.PMKR0Name, 
							 pAd->MlmeAux.Bssid, 
							 pAd->CurrentAddress, 
							 pEntry->FT_PMK_R1, 
							 pEntry->FT_PMK_R1_NAME);

				if (pEntry->WepStatus == Ndis802_11TKIPEnable)
					ptk_len = 32+32;
				else
					ptk_len = 32+16;

				/* Derive FT PTK and PTK-NAME */
				FT_DerivePTK(pEntry->FT_PMK_R1, 
						   	pEntry->FT_PMK_R1_NAME, 
						   	pEntry->ANonce, 
						   	pEntry->SNonce, 
					   		pAd->MlmeAux.Bssid, 	
					   		pAd->CurrentAddress, 							
						   	ptk_len, 			 
						   	pEntry->PTK, 
						   	pEntry->PTK_NAME);
			
                if ((pAd->MlmeAux.MdIeInfo.FtCapPlc.field.RsrReqCap == FALSE) ||
					(pAd->StaCfg.Dot11RCommInfo.bSupportResource == FALSE))
                {
                	/* 
                		AP doesn't support resource request or
                		Station doesn't want to do resource request
					*/
                    pAd->Mlme.FtOtaAuthMachine.CurrState = FT_OTA_AUTH_REQ_IDLE;
                    MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_AUTH_CONF, 2, &Status, 0);
                } 
                else 
                {
                	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  /*Get an unused nonpaged memory */
				    if(NStatus != NDIS_STATUS_SUCCESS) 
				    {
				        DBGPRINT(RT_DEBUG_TRACE, ("FT_OTA_AUTH - FT_OTA_MlmeAuthReqAction allocate memory failed\n"));
				        pAd->Mlme.FtOtaAuthMachine.CurrState = FT_OTA_AUTH_REQ_IDLE;
				        Status = MLME_FAIL_NO_RESOURCE;
				        MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_AUTH_CONF, 2, &Status, 0);
				        return;
				    }

					/* Send Auth Confirm */
					/* RSNIE */
					if (pAd->StaCfg.AuthMode >= Ndis802_11AuthModeWPA)
					{
						FT_ConstructAuthReqInRsn(pAd, pOutBuffer, &FrameLen);
					}
					
					/* MDIE */
					FT_InsertMdIE(pAd, 
								  pOutBuffer, 
								  &FrameLen, 
								  pAd->MlmeAux.MdIeInfo.MdId, 
								  pAd->MlmeAux.MdIeInfo.FtCapPlc);

					/* RIC-Request */
					EleID = IE_FT_RIC_DATA;
					
					AUTH_ReqSend(pAd, Elem, &pAd->MlmeAux.FtOtaAuthTimer, "FT_OTA_AUTH", 3, pOutBuffer, FrameLen);
					MlmeFreeMemory(pAd, pOutBuffer);
					pAd->Mlme.FtOtaAuthMachine.CurrState = FT_OTA_AUTH_WAIT_ACK;
                }
            } 
            else 
            {
                pAd->StaCfg.AuthFailReason = Status;
                COPY_MAC_ADDR(pAd->StaCfg.AuthFailSta, Addr2);
                pAd->Mlme.FtOtaAuthMachine.CurrState = FT_OTA_AUTH_REQ_IDLE;
                MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_AUTH_CONF, 2, &Status, 0);
            }
        }
    }
    else
    {
        DBGPRINT(RT_DEBUG_TRACE, ("AUTH - PeerAuthSanity() sanity check fail\n"));
    }
}

/*
    ==========================================================================
    Description:
        
	IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
VOID FT_OTA_PeerAuthAckAction(
    IN PRTMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
    UCHAR         Addr2[MAC_ADDR_LEN];
    USHORT        Alg, Seq, Status;
    BOOLEAN       TimerCancelled;

    if (PeerAuthSanity(pAd, Elem->Msg, Elem->MsgLen, Addr2, &Alg, &Seq, &Status, NULL)) 
    {
        if (MAC_ADDR_EQUAL(pAd->MlmeAux.Bssid, Addr2) && 
			(Alg == AUTH_MODE_FT) &&
			(Seq == 4))
        {
            DBGPRINT(RT_DEBUG_TRACE, ("FT_OTA_AUTH - Receive FT_OTA_AUTH_ACK to me\n"));
            RTMPCancelTimer(&pAd->MlmeAux.FtOtaAuthTimer, &TimerCancelled);
            pAd->StaCfg.Dot11RCommInfo.FtRspSuccess = FT_OTA_RESPONSE;
            if (Status == MLME_SUCCESS) 
            {
                /* Retrieve Reassociation Deadline */
				
				/* Check RIC-Response */
            }
			else
			{				
				pAd->StaCfg.AuthFailReason = Status;
                COPY_MAC_ADDR(pAd->StaCfg.AuthFailSta, Addr2);
			}

            pAd->Mlme.FtOtaAuthMachine.CurrState = FT_OTA_AUTH_REQ_IDLE;
            MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_AUTH_CONF, 2, &Status, 0);
        }
    }
    else
    {
        DBGPRINT(RT_DEBUG_TRACE, ("FT_OTA_AUTH - FT_OTA_PeerAuthAckAction() sanity check fail\n"));
    }
}

/*
    ==========================================================================
    Description:
        
	IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
VOID FT_OTA_InvalidStateWhenAuth(
    IN PRTMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem) 
{
	DBGPRINT(RT_DEBUG_TRACE, ("FT_OTA_AUTH - FT_OTA_InvalidStateWhenAuth (state=%ld), reset FT_OTA_AUTH state machine\n", pAd->Mlme.FtOtaAuthMachine.CurrState));
}

VOID FT_OTA_AuthParmFill(
	IN PRTMP_ADAPTER pAd, 
	IN OUT MLME_FT_OTA_AUTH_REQ_STRUCT *pFtOtaAuthReq, 
	IN PUCHAR 	pAddr, 
	IN USHORT 	Alg,
	IN PDOT11R_CMN_STRUC	pCmmDot11rCfg)
{		
	COPY_MAC_ADDR(pFtOtaAuthReq->Addr, pAddr);
	pFtOtaAuthReq->Alg = Alg;
	pFtOtaAuthReq->Timeout = (AUTH_TIMEOUT * 2);

	/* MDIE */
	pFtOtaAuthReq->MdIe.FtCapPlc.field.RsrReqCap = 0;
	pFtOtaAuthReq->MdIe.FtCapPlc.field.FtOverDs = 0;
	if (pCmmDot11rCfg->bSupportResource && pAd->MlmeAux.MdIeInfo.FtCapPlc.field.RsrReqCap)
		pFtOtaAuthReq->MdIe.FtCapPlc.field.RsrReqCap = 1;
	pFtOtaAuthReq->MdIe.FtCapPlc.field.FtOverDs = pAd->MlmeAux.MdIeInfo.FtCapPlc.field.FtOverDs;
	FT_SET_MDID(pFtOtaAuthReq->MdIe.MdId, pCmmDot11rCfg->MdIeInfo.MdId);
}
#endif /* DOT11R_FT_SUPPORT */


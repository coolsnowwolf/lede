/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 5F, No. 36 TaiYuan St.  Juebei City.
 * Hsin-chu County, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2009, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	action_ft.c

	Abstract:
	Handle State Machine for 802.11r Fast BSS Transition Over-The-DS action frame.

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Jan			2008-12-17		Fast BSS Transition Over-The-DS State Machine.
*/

#ifdef DOT11R_FT_SUPPORT

#include "rt_config.h"
#include "ft.h"

BOOLEAN MlmeFtReqSanity(
	IN PRTMP_ADAPTER pAd,
	IN VOID *Msg,
	IN ULONG MsgLen,
	OUT PUCHAR pAddr,
	OUT PULONG pTimeout,
	OUT FT_MDIE *pMdIe,
	OUT UCHAR *pSnonce,
	OUT UCHAR *pR0KhIdLen,
	OUT UCHAR *pR0KhId,
	OUT USHORT *LengthVIE,
	OUT PNDIS_802_11_VARIABLE_IEs pRSNIE)
{
	MLME_FT_REQ_STRUCT *pInfo;

	pInfo = (MLME_FT_REQ_STRUCT *) Msg;
	*LengthVIE = 0;
	*pR0KhIdLen = 0;
	*pTimeout = 0;

	COPY_MAC_ADDR(pAddr, pInfo->TargetAddr);
	RTMPMoveMemory(pMdIe, &pInfo->MdIe, sizeof (FT_MDIE));
	RTMPMoveMemory(pR0KhId, pInfo->R0khId, pInfo->R0khIdLen);
	*pR0KhIdLen = pInfo->R0khIdLen;
	RTMPMoveMemory(pSnonce, pInfo->SNonce, 32);
	*LengthVIE = (USHORT) pInfo->VIeLen;
	RTMPMoveMemory(pRSNIE, pInfo->VIe, *LengthVIE);
	*pTimeout = pInfo->Timeout;

	COPY_MAC_ADDR(pAddr, pInfo->TargetAddr);

	return TRUE;
}

BOOLEAN PeerFtRspSanity(
	IN PRTMP_ADAPTER pAd,
	IN VOID *Msg,
	IN ULONG MsgLen,
	OUT PUCHAR pAddr,
	OUT FT_FTIE *pFtIe,
	OUT FT_MDIE *pMdIe,
	OUT USHORT *pStatus)
{
	FT_OVER_DS_ACTION_RSP_ACK *pFtRspFrame = NULL;	/*(FT_OVER_DS_ACTION_RSP_ACK)Msg; */
	UCHAR Category;
	UCHAR Action;
	PEID_STRUCT pEid;
	CHAR *Ptr;
	ULONG Length = 0;

	pFtRspFrame = (FT_OVER_DS_ACTION_RSP_ACK *) (Msg + LENGTH_802_11);
	Ptr = (PCHAR) pFtRspFrame->InfoElm;
	COPY_MAC_ADDR(pAddr, pFtRspFrame->TargetAP_Addr);
	Action = pFtRspFrame->Action;
	Category = pFtRspFrame->Category;
	*pStatus = pFtRspFrame->StatusCode;

	if (Action != FT_ACTION_BT_RSP) {
		DBGPRINT(RT_DEBUG_TRACE,
			 ("PeerFtRspSanity fail - wrong Action [%x]. \n",
			  Action));
		return FALSE;
	}

	pEid = (PEID_STRUCT) Ptr;
	Length = MsgLen - sizeof (FT_OVER_DS_ACTION_RSP_ACK) - LENGTH_802_11;
	/* get variable fields from payload and advance the pointer */
	/*while ((Length + 2 + pEid->Len) <= MsgLen) */
	while ((Length > 0) && (Length >= (2 + pEid->Len))) {
		switch (pEid->Eid) {
		case IE_RSN:
			break;
		case IE_FT_MDIE:
			if (pEid->Len == 3) {
				NdisMoveMemory(pMdIe, pEid->Octet, pEid->Len);
			}
			DBGPRINT(RT_DEBUG_TRACE,
				 ("PeerFtRspSanity IE_MOBILITY_DOMAIN Mdid = [%x%x]. \n",
				  pMdIe->MdId[0], pMdIe->MdId[1]));
			break;
		case IE_FT_FTIE:
			if (pEid->Len >= sizeof (FT_FTIE)) {
				NdisMoveMemory(pFtIe, pEid->Octet,
					       sizeof (FT_FTIE));
			}
			DBGPRINT(RT_DEBUG_TRACE,
				 ("PeerFtRspSanity IE_FAST_BSS_TRANSITION . \n"));
			break;
		}
		Length -= (2 + pEid->Len);	/* Eid[1] + Len[1]+ content[Len] */
		pEid = (PEID_STRUCT) ((UCHAR *) pEid + 2 + pEid->Len);
	}

	return TRUE;
}

/*
    ==========================================================================
    Description:
        ft state machine init, including state transition and timer init
    Parameters:
        Sm - pointer to the dls state machine
    Note:
        The state machine looks like this
        
                            FT_IDLE
    MT2_MLME_FT_REQUEST   MlmeDlsReqAction
    MT2_PEER_DLS_REQUEST   PeerDlsReqAction
    MT2_PEER_DLS_RESPONSE  PeerDlsRspAction
    MT2_MLME_DLS_TEARDOWN  MlmeTearDownAction
    MT2_PEER_DLS_TEARDOWN  PeerTearDownAction
        
	IRQL = PASSIVE_LEVEL

    ==========================================================================
 */
void FT_OTD_StateMachineInit(
	IN PRTMP_ADAPTER pAd,
	IN STATE_MACHINE *Sm,
	OUT STATE_MACHINE_FUNC Trans[])
{
	StateMachineInit(Sm, Trans, FT_OTD_MAX_STATE, FT_OTD_MAX_MSG,
			 (STATE_MACHINE_FUNC) Drop, FT_OTD_IDLE,
			 FT_OTD_MACHINE_BASE);

	/* the first column */
	StateMachineSetAction(Sm, FT_OTD_IDLE, FT_OTD_MT2_MLME_REQ,
			      (STATE_MACHINE_FUNC) FT_OTD_ReqAction);

	StateMachineSetAction(Sm, FT_OTD_WAIT_SEQ2, FT_OTD_MT2_MLME_REQ,
			      (STATE_MACHINE_FUNC) FT_OTD_InvalidStateWhenFt);
	StateMachineSetAction(Sm, FT_OTD_WAIT_SEQ2, FT_OTD_MT2_PEER_EVEN,
			      (STATE_MACHINE_FUNC) FT_OTD_PeerRspAtSeq2Action);
	StateMachineSetAction(Sm, FT_OTD_WAIT_SEQ2, FT_OTD_MT2_TIMEOUT,
			      (STATE_MACHINE_FUNC) FT_OTD_TimeoutAction);

	StateMachineSetAction(Sm, FT_OTD_WAIT_SEQ4, FT_OTD_MT2_MLME_REQ,
			      (STATE_MACHINE_FUNC) FT_OTD_InvalidStateWhenFt);
	StateMachineSetAction(Sm, FT_OTD_WAIT_SEQ4, FT_OTD_MT2_PEER_EVEN,
			      (STATE_MACHINE_FUNC) FT_OTD_PeerAckAtSeq4Action);
	StateMachineSetAction(Sm, FT_OTD_WAIT_SEQ4, FT_OTD_MT2_TIMEOUT,
			      (STATE_MACHINE_FUNC) FT_OTD_TimeoutAction);

	RTMPInitTimer(pAd, &pAd->MlmeAux.FtOtdActTimer,
		      GET_TIMER_FUNCTION(FT_OTD_TimeoutAction), pAd, FALSE);

}

/*
    ==========================================================================
    Description:
        
	IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
VOID FT_OTD_ReqAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	PUCHAR pOutBuffer = NULL;
	NDIS_STATUS NStatus;
	ULONG FrameLen = 0;
	HEADER_802_11 FtReqHdr;
	FT_MDIE MdIe;
	UCHAR Snonce[32];
	UCHAR R0KhIdLen;
	UCHAR R0KhId[FT_ROKH_ID_LEN + 1];
	UCHAR Category = FT_CATEGORY_BSS_TRANSITION;
	UCHAR Action = FT_ACTION_BT_REQ;
	ULONG Timeout = 0;
	USHORT Status;
	UCHAR TargetAddr[6];
	NDIS_802_11_VARIABLE_IEs *pRsnIE = NULL;
	USHORT LenRsnIE;

	if (!MlmeFtReqSanity
	    (pAd, Elem->Msg, Elem->MsgLen, TargetAddr, &Timeout, &MdIe, Snonce,
	     &R0KhIdLen, R0KhId, &LenRsnIE, pRsnIE)) {
		DBGPRINT_ERR(("FT_OTD_ACTION - FT_OTD_ReqAction() sanity check failed\n"));
		pAd->Mlme.AuthMachine.CurrState = FT_OTD_IDLE;
		Status = MLME_INVALID_FORMAT;
		MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_AUTH_CONF, 2,
			    &Status, 0);
		return;
	}

	DBGPRINT(RT_DEBUG_TRACE, ("FT_OTD_ACTION :FT_OTD_ReqAction() \n"));

	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);	/*Get an unused nonpaged memory */
	if (NStatus != NDIS_STATUS_SUCCESS) {
		DBGPRINT(RT_DEBUG_ERROR,
			 ("FT_OTD_ACTION :FT_OTD_ReqAction() allocate memory failed \n"));
		return;
	}

	ActHeaderInit(pAd, &FtReqHdr, pAd->CommonCfg.Bssid, pAd->CurrentAddress,
		      pAd->CommonCfg.Bssid);

	/* Build basic frame first */
	MakeOutgoingFrame(pOutBuffer, &FrameLen,
			  sizeof (HEADER_802_11), &FtReqHdr,
			  1, &Category,
			  1, &Action,
			  6, pAd->CurrentAddress, 6, TargetAddr, END_OF_ARGS);

	/* MDIE */
	FT_InsertMdIE(pAd, pOutBuffer + FrameLen, &FrameLen, MdIe.MdId,
		      MdIe.FtCapPlc);

	/* Process with RSN */
	if (pAd->StaCfg.AuthMode >= Ndis802_11AuthModeWPA) {
		FT_ConstructAuthReqInRsn(pAd, pOutBuffer, &FrameLen);
	}

	MiniportMMRequest(pAd, QID_AC_BE, pOutBuffer, FrameLen);
	MlmeFreeMemory(pAd, pOutBuffer);

	RTMPSetTimer(&pAd->MlmeAux.FtOtdActTimer, Timeout);
	pAd->Mlme.FtOtdActMachine.CurrState = FT_OTD_WAIT_SEQ2;

}

VOID FT_OTD_PeerRspAtSeq2Action(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	UCHAR TargetAddr[MAC_ADDR_LEN];
	USHORT Status;
	BOOLEAN TimerCancelled;
	ULONG BssIdx = 0;
	FT_FTIE FtIe;
	FT_MDIE MdIe;
	PFRAME_802_11 pFrame = (PFRAME_802_11) Elem->Msg;
	MLME_ASSOC_REQ_STRUCT AssocReq;
	UCHAR BBPValue = 0;

	DBGPRINT(RT_DEBUG_TRACE,
		 ("FT_OTD_ACTION - PeerFtRspAtSeq2Action MlmeAux.Bssid = %02x:%02x:%02x:%02x:%02x:%02x\n",
			PRINT_MAC(pAd->MlmeAux.Bssid)));

	if (PeerFtRspSanity
	    (pAd, Elem->Msg, Elem->MsgLen, TargetAddr, &FtIe, &MdIe, &Status)) {
		if (MAC_ADDR_EQUAL(pAd->CommonCfg.Bssid, pFrame->Hdr.Addr2)) {
			DBGPRINT(RT_DEBUG_TRACE,
				 ("FT_OTD_ACTION - Receive FT_RSP seq#2 to me ( Status=%d)\n",
				  Status));
			RTMPCancelTimer(&pAd->MlmeAux.FtOtdActTimer,
					&TimerCancelled);

			if (Status == MLME_SUCCESS) {
				UINT8 ptk_len;
				PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[MCAST_WCID];

				NdisMoveMemory(pEntry->SNonce, FtIe.SNonce, 32);

				/* Get ANonce from authentication-response */
				NdisMoveMemory(pEntry->ANonce, FtIe.ANonce, 32);

				hex_dump("anonce", pEntry->ANonce, 32);
				hex_dump("snonce", pEntry->SNonce, 32);

				FT_DerivePMKR1(pAd->StaCfg.Dot11RCommInfo.PMKR0, pAd->StaCfg.Dot11RCommInfo.PMKR0Name, TargetAddr,	/*pAd->MlmeAux.Bssid, */
					       pAd->CurrentAddress,
					       pEntry->FT_PMK_R1,
					       pEntry->FT_PMK_R1_NAME);

				if (pEntry->WepStatus == Ndis802_11TKIPEnable)
					ptk_len = 32 + 32;
				else
					ptk_len = 32 + 16;

				/* Derive FT PTK and PTK-NAME */
				FT_DerivePTK(pEntry->FT_PMK_R1, pEntry->FT_PMK_R1_NAME, pEntry->ANonce, pEntry->SNonce, TargetAddr,	/*pAd->MlmeAux.Bssid, */
					     pAd->CurrentAddress,
					     ptk_len,
					     pEntry->PTK, pEntry->PTK_NAME);

				/*
				   How to know there is resource request session now ????????
				 */
				if ((pAd->StaCfg.Dot11RCommInfo.bSupportResource)
				    && (pAd->MlmeAux.MdIeInfo.FtCapPlc.field.RsrReqCap)) {
					/* Prepare to send FT Confirm packet. */
					DBGPRINT(RT_DEBUG_TRACE,
						 ("FT_OTD_ACTION - Receive FT_RSP seq#2 to me, Prepare to send FT Confirm. \n"));
					pAd->Mlme.FtOtdActMachine.CurrState =
					    FT_OTD_WAIT_SEQ4;

				} else {
					BSS_ENTRY *pBss = NULL;
					/*
					   Doesn't need to send FT Confirm packet. 
					 */
					DBGPRINT(RT_DEBUG_TRACE,
						 ("FT_OTD_ACTION - Receive FT_RSP seq#2 to me, Prepare to send Reassoc. \n"));
					pAd->StaCfg.Dot11RCommInfo.FtRspSuccess = FT_OTD_RESPONSE;
					pAd->Mlme.FtOtdActMachine.CurrState = FT_OTD_IDLE;
					RTMPMoveMemory(pAd->MlmeAux.Bssid, TargetAddr, MAC_ADDR_LEN);

					/*
					   find the desired BSS in the latest SCAN result table
					   search 2.4G band first
					 */
					BssIdx = BssTableSearch(&pAd->ScanTab, TargetAddr, 1);
					/*
					   search 5G band, if AP does not exist in 2.4G band
					 */
					if (BssIdx == BSS_NOT_FOUND)
						BssIdx = BssTableSearch(&pAd->ScanTab, TargetAddr, 36);
					if (BssIdx == BSS_NOT_FOUND) {
						DBGPRINT(RT_DEBUG_TRACE,
							 ("FT_OTD_ACTION - BSSID not found. reply NDIS_STATUS_NOT_ACCEPTED\n"));
						pAd->Mlme.CntlMachine.CurrState = CNTL_IDLE;
						return;
					}

					pBss = &pAd->ScanTab.BssEntry[BssIdx];
					pAd->MlmeAux.Channel = pBss->Channel;
					pAd->MlmeAux.CentralChannel = pBss->CentralChannel;
					RTMPZeroMemory(pAd->MlmeAux.ExtRate,
						       MAX_LEN_OF_SUPPORTED_RATES);
					RTMPZeroMemory(pAd->MlmeAux.SupRate,
						       MAX_LEN_OF_SUPPORTED_RATES);
					pAd->MlmeAux.ExtRateLen = pBss->ExtRateLen;
					RTMPMoveMemory(pAd->MlmeAux.ExtRate,
						       pBss->ExtRate,
						       pBss->ExtRateLen);
					pAd->MlmeAux.SupRateLen = pBss->SupRateLen;
					RTMPMoveMemory(pAd->MlmeAux.SupRate,
						       pBss->SupRate,
						       pBss->SupRateLen);

					RTMPZeroMemory(pAd->MlmeAux.Ssid,
						       MAX_LEN_OF_SSID);
					pAd->MlmeAux.SsidLen = pBss->SsidLen;
					RTMPMoveMemory(pAd->MlmeAux.Ssid,
						       pBss->Ssid,
						       pBss->SsidLen);

					/* 
					   StaActive.SupportedHtPhy.MCSSet stores Peer AP's 11n Rx capability 
					 */
					if (pBss->HtCapabilityLen) {
						RTMPMoveMemory(pAd->StaActive.SupportedPhyInfo.MCSSet,
							       pBss->HtCapability.MCSSet, 16);
					} else {
						NdisZeroMemory(pAd->StaActive.SupportedPhyInfo.MCSSet, 16);
					}

					bbp_set_bw(pAd, BW_20);

					AsicSwitchChannel(pAd, pAd->MlmeAux.Channel, FALSE);
					AsicLockChannel(pAd, pAd->MlmeAux.Channel);

					RTMPUpdateMlmeRate(pAd);

					AssocParmFill(pAd, &AssocReq,
						      pAd->MlmeAux.Bssid,
						      pAd->MlmeAux.CapabilityInfo,
						      ASSOC_TIMEOUT,
						      pAd->StaCfg.DefaultListenCount);

					MlmeEnqueue(pAd, ASSOC_STATE_MACHINE,
						    MT2_MLME_REASSOC_REQ,
						    sizeof
						    (MLME_ASSOC_REQ_STRUCT),
						    &AssocReq, 0);

					pAd->Mlme.CntlMachine.CurrState = CNTL_WAIT_REASSOC;
				}

			} else {
				pAd->StaCfg.AuthFailReason = Status;
				COPY_MAC_ADDR(pAd->StaCfg.AuthFailSta, pFrame->Hdr.Addr2);
				pAd->Mlme.FtOtdActMachine.CurrState = FT_OTD_IDLE;
				MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE,
					    MT2_FT_OTD_CONF, 2, &Status, 0);
			}
		}
	} else {
		DBGPRINT(RT_DEBUG_TRACE,
			 ("FT_OTD_ACTION - PeerFtRspSanity() sanity check fail\n"));
	}
}

VOID FT_OTD_PeerAckAtSeq4Action(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	pAd->StaCfg.Dot11RCommInfo.FtRspSuccess = FT_OTD_RESPONSE;
	pAd->Mlme.FtOtdActMachine.CurrState = FT_OTD_IDLE;
}

/*
	==========================================================================
	Description:
    
	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
VOID FT_OTD_TimeoutAction(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *) FunctionContext;
	USHORT Status;

	if (pAd != NULL) {
		pAd->Mlme.FtOtdActMachine.CurrState = FT_OTD_IDLE;
		Status = MLME_REJ_TIMEOUT;
		/*MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_FT_OTD_CONF, 2, &Status, 0); */
	}
	DBGPRINT(RT_DEBUG_TRACE, ("FT_OTD_ACTION - FtTimeoutAction\n"));
}

/*
	==========================================================================
	Description:
    
	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
VOID FT_OTD_InvalidStateWhenFt(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem)
{
	USHORT Status;
	DBGPRINT(RT_DEBUG_TRACE,
		 ("FT_OTD_ACTION - InvalidStateWhenFt (state=%lu), reset FT state machine\n",
		  pAd->Mlme.FtOtdActMachine.CurrState));
	pAd->Mlme.FtOtdActMachine.CurrState = FT_OTD_IDLE;
	Status = MLME_STATE_MACHINE_REJECT;
	MlmeEnqueue(pAd, MLME_CNTL_STATE_MACHINE, MT2_FT_OTD_CONF, 2, &Status,
		    0);
}

VOID FT_OTD_ActParmFill(
	IN PRTMP_ADAPTER pAd,
	IN OUT MLME_FT_REQ_STRUCT *FtReq,
	IN PUCHAR pAddr,
	IN NDIS_802_11_AUTHENTICATION_MODE AuthMode,
	IN PFT_MDIE_INFO FtMdieInfo,
	IN PFT_FTIE_INFO FtFtieInfo,
	IN UCHAR VarIeLen,
	IN PUCHAR pVarIe)
{
	RTMPZeroMemory(FtReq, sizeof (MLME_FT_REQ_STRUCT));
	COPY_MAC_ADDR(FtReq->TargetAddr, pAddr);
	if (AuthMode >= Ndis802_11AuthModeWPA)
		FtReq->HaveRSN = 1;
	FtReq->Timeout = FT_ACT_TIMEOUT;
	FT_SET_MDID(FtReq->MdIe.MdId, FtMdieInfo->MdId);
	FtReq->MdIe.FtCapPlc.word = FtMdieInfo->FtCapPlc.word;
	RTMPMoveMemory(&FtReq->SNonce[0], FtFtieInfo->SNonce, 32);
	RTMPMoveMemory(&FtReq->R0khId[0], FtFtieInfo->R0khId,
		       FtFtieInfo->R0khIdLen);
	FtReq->R0khIdLen = FtFtieInfo->R0khIdLen;
	RTMPMoveMemory(&FtReq->VIe[0], pVarIe, VarIeLen);
}
#endif /* DOT11R_FT_SUPPORT */

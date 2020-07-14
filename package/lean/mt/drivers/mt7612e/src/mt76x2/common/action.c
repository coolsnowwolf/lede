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
	action.c
 
    Abstract:
    Handle association related requests either from WSTA or from local MLME
 
    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
	Jan Lee		2006	  	created for rt2860
 */

#include "rt_config.h"
#include "action.h"

extern UCHAR  ZeroSsid[32];
#ifdef IWSC_SUPPORT
extern UCHAR  IWSC_ACTION_OUI[];
#endif // IWSC_SUPPORT //

static VOID ReservedAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);



/*
    ==========================================================================
    Description: 
        association state machine init, including state transition and timer init
    Parameters: 
        S - pointer to the association state machine
    Note:
        The state machine looks like the following 
        
                                    ASSOC_IDLE             
        MT2_MLME_DISASSOC_REQ    mlme_disassoc_req_action 
        MT2_PEER_DISASSOC_REQ    peer_disassoc_action     
        MT2_PEER_ASSOC_REQ       drop                     
        MT2_PEER_REASSOC_REQ     drop                     
        MT2_CLS3ERR              cls3err_action           
    ==========================================================================
 */
VOID ActionStateMachineInit(
    IN RTMP_ADAPTER *pAd, 
    IN STATE_MACHINE *S, 
    OUT STATE_MACHINE_FUNC Trans[]) 
{
	StateMachineInit(S, (STATE_MACHINE_FUNC *)Trans, MAX_ACT_STATE, MAX_ACT_MSG, (STATE_MACHINE_FUNC)Drop, ACT_IDLE, ACT_MACHINE_BASE);

	StateMachineSetAction(S, ACT_IDLE, MT2_PEER_SPECTRUM_CATE, (STATE_MACHINE_FUNC)PeerSpectrumAction);
	StateMachineSetAction(S, ACT_IDLE, MT2_PEER_QOS_CATE, (STATE_MACHINE_FUNC)PeerQOSAction);

	StateMachineSetAction(S, ACT_IDLE, MT2_PEER_DLS_CATE, (STATE_MACHINE_FUNC)ReservedAction);
#ifdef QOS_DLS_SUPPORT
		StateMachineSetAction(S, ACT_IDLE, MT2_PEER_DLS_CATE, (STATE_MACHINE_FUNC)PeerDLSAction);
#endif /* QOS_DLS_SUPPORT */

#ifdef DOT11_N_SUPPORT
	StateMachineSetAction(S, ACT_IDLE, MT2_PEER_BA_CATE, (STATE_MACHINE_FUNC)PeerBAAction);
	StateMachineSetAction(S, ACT_IDLE, MT2_PEER_HT_CATE, (STATE_MACHINE_FUNC)PeerHTAction);
#ifdef DOT11_VHT_AC
	StateMachineSetAction(S, ACT_IDLE, MT2_PEER_VHT_CATE, (STATE_MACHINE_FUNC)PeerVHTAction);
#endif /* DOT11_VHT_AC */
	StateMachineSetAction(S, ACT_IDLE, MT2_MLME_ADD_BA_CATE, (STATE_MACHINE_FUNC)MlmeADDBAAction);
	StateMachineSetAction(S, ACT_IDLE, MT2_MLME_ORI_DELBA_CATE, (STATE_MACHINE_FUNC)MlmeDELBAAction);
	StateMachineSetAction(S, ACT_IDLE, MT2_MLME_REC_DELBA_CATE, (STATE_MACHINE_FUNC)MlmeDELBAAction);
#endif /* DOT11_N_SUPPORT */

	StateMachineSetAction(S, ACT_IDLE, MT2_PEER_PUBLIC_CATE, (STATE_MACHINE_FUNC)PeerPublicAction);
	StateMachineSetAction(S, ACT_IDLE, MT2_PEER_RM_CATE, (STATE_MACHINE_FUNC)PeerRMAction);
	
	StateMachineSetAction(S, ACT_IDLE, MT2_MLME_QOS_CATE, (STATE_MACHINE_FUNC)MlmeQOSAction);
	StateMachineSetAction(S, ACT_IDLE, MT2_MLME_DLS_CATE, (STATE_MACHINE_FUNC)MlmeDLSAction);
	StateMachineSetAction(S, ACT_IDLE, MT2_ACT_INVALID, (STATE_MACHINE_FUNC)MlmeInvalidAction);


#ifdef CONFIG_AP_SUPPORT
#ifdef DOT11R_FT_SUPPORT
	StateMachineSetAction(S, ACT_IDLE, FT_CATEGORY_BSS_TRANSITION, (STATE_MACHINE_FUNC)FT_FtAction);
#endif /* DOT11R_FT_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

#ifdef DOT11W_PMF_SUPPORT
	StateMachineSetAction(S, ACT_IDLE, MT2_PEER_PMF_CATE, (STATE_MACHINE_FUNC)PMF_PeerAction);
#endif /* DOT11W_PMF_SUPPORT */

#ifdef DOT11V_WNM_SUPPORT	
	StateMachineSetAction(S, ACT_IDLE, WNM_CATEGORY_BSS_TRANSITION, (STATE_MACHINE_FUNC)WNM_Action);
#endif /* DOT11V_WNM_SUPPORT */

}


#ifdef DOT11_N_SUPPORT
VOID MlmeADDBAAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem) 
{
	MLME_ADDBA_REQ_STRUCT *pInfo;
	UCHAR Addr[6];
	PUCHAR pOutBuffer = NULL;
	NDIS_STATUS NStatus;
	ULONG Idx;
	FRAME_ADDBA_REQ Frame;
	ULONG FrameLen;
	/*BA_ORI_ENTRY *pBAEntry = NULL;*/
	MAC_TABLE_ENTRY *pEntry = NULL;
	struct wifi_dev *wdev;

	pInfo = (MLME_ADDBA_REQ_STRUCT *)Elem->Msg;
	NdisZeroMemory(&Frame, sizeof(FRAME_ADDBA_REQ));
	
	if(MlmeAddBAReqSanity(pAd, Elem->Msg, Elem->MsgLen, Addr) &&
		VALID_WCID(pInfo->Wcid)) 
	{
		NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  /* Get an unused nonpaged memory*/
		if(NStatus != NDIS_STATUS_SUCCESS) 
		{
			DBGPRINT(RT_DEBUG_TRACE,("BA - MlmeADDBAAction() allocate memory failed \n"));
			return;
		}
		/* 1. find entry */
		pEntry = &pAd->MacTab.Content[pInfo->Wcid];
		ASSERT((pEntry->wdev != NULL));
		wdev = pEntry->wdev;
		
		Idx = pEntry->BAOriWcidArray[pInfo->TID];
		if (Idx == 0)
		{
			MlmeFreeMemory(pAd, pOutBuffer);
			DBGPRINT(RT_DEBUG_ERROR,("BA - MlmeADDBAAction() can't find BAOriEntry \n"));
			return;
		} 
		else
		{
			/*pBAEntry =&pAd->BATable.BAOriEntry[Idx];*/
		}

#ifdef APCLI_SUPPORT
#ifdef MAC_REPEATER_SUPPORT
		if (IS_ENTRY_APCLI(pEntry) && (pEntry->bReptCli == TRUE)) {
			INT idx = pAd->MacTab.Content[pInfo->Wcid].wdev_idx;
			ActHeaderInit(pAd, &Frame.Hdr, pInfo->pAddr,
							pAd->ApCfg.ApCliTab[idx].RepeaterCli[pEntry->MatchReptCliIdx].CurrentAddress,
							pInfo->pAddr);
		}
		else
#endif /* MAC_REPEATER_SUPPORT */
#endif /* APCLI_SUPPORT */
			ActHeaderInit(pAd, &Frame.Hdr, pInfo->pAddr, wdev->if_addr, wdev->bssid);

		Frame.Category = CATEGORY_BA;
		Frame.Action = ADDBA_REQ;
		Frame.BaParm.AMSDUSupported = 0;
#ifdef WFA_VHT_PF
		if (pAd->CommonCfg.DesiredHtPhy.AmsduEnable)
			Frame.BaParm.AMSDUSupported = 1;
#endif /* WFA_VHT_PF */
		Frame.BaParm.BAPolicy = IMMED_BA;
		Frame.BaParm.TID = pInfo->TID;
		Frame.BaParm.BufSize = pInfo->BaBufSize;
		Frame.Token = pInfo->Token;
		Frame.TimeOutValue = pInfo->TimeOutValue;
		Frame.BaStartSeq.field.FragNum = 0;
		Frame.BaStartSeq.field.StartSeq = pEntry->TxSeq[pInfo->TID];

#ifdef UNALIGNMENT_SUPPORT
		{
			BA_PARM tmpBaParm;

			NdisMoveMemory((PUCHAR)(&tmpBaParm), (PUCHAR)(&Frame.BaParm), sizeof(BA_PARM));
			*(USHORT *)(&tmpBaParm) = cpu2le16(*(USHORT *)(&tmpBaParm));
			NdisMoveMemory((PUCHAR)(&Frame.BaParm), (PUCHAR)(&tmpBaParm), sizeof(BA_PARM));
		}
#else
		*(USHORT *)(&(Frame.BaParm)) = cpu2le16((*(USHORT *)(&(Frame.BaParm))));
#endif /* UNALIGNMENT_SUPPORT */

		Frame.TimeOutValue = cpu2le16(Frame.TimeOutValue);
		Frame.BaStartSeq.word = cpu2le16(Frame.BaStartSeq.word);

		MakeOutgoingFrame(pOutBuffer, &FrameLen,
		              sizeof(FRAME_ADDBA_REQ), &Frame,
		              END_OF_ARGS);

		MiniportMMRequest(pAd, (MGMT_USE_QUEUE_FLAG | WMM_UP2AC_MAP[pInfo->TID]), pOutBuffer, FrameLen);

		MlmeFreeMemory(pAd, pOutBuffer);
		
		DBGPRINT(RT_DEBUG_TRACE,
					("BA - Send ADDBA request. StartSeq = %x,  FrameLen = %ld. BufSize = %d\n",
					Frame.BaStartSeq.field.StartSeq, FrameLen, Frame.BaParm.BufSize));
    }
}


/*
    ==========================================================================
    Description:
        send DELBA and delete BaEntry if any
    Parametrs:
        Elem - MLME message MLME_DELBA_REQ_STRUCT
        
	IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
VOID MlmeDELBAAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem) 
{
	MLME_DELBA_REQ_STRUCT *pInfo;
	PUCHAR pOutBuffer = NULL, pOutBuffer2 = NULL;
	FRAME_DELBA_REQ Frame;
	ULONG FrameLen;
	FRAME_BAR FrameBar;
	MAC_TABLE_ENTRY *pEntry = NULL;
	struct wifi_dev *wdev;


	pInfo = (MLME_DELBA_REQ_STRUCT *)Elem->Msg;	
	/* must send back DELBA */
	NdisZeroMemory(&Frame, sizeof(FRAME_DELBA_REQ));
	DBGPRINT(RT_DEBUG_TRACE, ("==> MlmeDELBAAction(), Initiator(%d) \n", pInfo->Initiator));

	if(MlmeDelBAReqSanity(pAd, Elem->Msg, Elem->MsgLen) &&
		VALID_WCID(pInfo->Wcid))
	{
		if(MlmeAllocateMemory(pAd, &pOutBuffer) != NDIS_STATUS_SUCCESS)
		{
			DBGPRINT(RT_DEBUG_ERROR,("BA - MlmeDELBAAction() allocate memory failed 1. \n"));
			return;
		}

		if(MlmeAllocateMemory(pAd, &pOutBuffer2) != NDIS_STATUS_SUCCESS)
		{
			MlmeFreeMemory(pAd, pOutBuffer);
			DBGPRINT(RT_DEBUG_ERROR, ("BA - MlmeDELBAAction() allocate memory failed 2. \n"));
			return;
		}

		/* SEND BAR (Send BAR to refresh peer reordering buffer.) */
		pEntry = &pAd->MacTab.Content[pInfo->Wcid];
		if (!pEntry->wdev)
		{
			DBGPRINT(RT_DEBUG_ERROR, ("%s():No binding wdev for wcid(%d)\n", __FUNCTION__, pInfo->Wcid));
			MlmeFreeMemory(pAd, pOutBuffer);
			MlmeFreeMemory(pAd, pOutBuffer2);
			return;
		}

		wdev = pEntry->wdev;
#ifdef APCLI_SUPPORT
#ifdef MAC_REPEATER_SUPPORT
		if (IS_ENTRY_APCLI(pEntry) && pEntry->bReptCli)
		{
			UINT apidx = pAd->MacTab.Content[pInfo->Wcid].wdev_idx;
			BarHeaderInit(pAd, &FrameBar, pEntry->Addr,
						pAd->ApCfg.ApCliTab[apidx].RepeaterCli[pEntry->MatchReptCliIdx].CurrentAddress);
		}
		else
#endif /* MAC_REPEATER_SUPPORT */
#endif /* APCLI_SUPPORT */
		BarHeaderInit(pAd, &FrameBar, pEntry->Addr, wdev->if_addr);

		FrameBar.StartingSeq.field.FragNum = 0; /* make sure sequence not clear in DEL funciton.*/
		FrameBar.StartingSeq.field.StartSeq = pAd->MacTab.Content[pInfo->Wcid].TxSeq[pInfo->TID]; /* make sure sequence not clear in DEL funciton.*/
		FrameBar.BarControl.TID = pInfo->TID; /* make sure sequence not clear in DEL funciton.*/
		FrameBar.BarControl.ACKPolicy = IMMED_BA; /* make sure sequence not clear in DEL funciton.*/
		FrameBar.BarControl.Compressed = 1; /* make sure sequence not clear in DEL funciton.*/
		FrameBar.BarControl.MTID = 0; /* make sure sequence not clear in DEL funciton.*/

		MakeOutgoingFrame(pOutBuffer2, &FrameLen,
					  sizeof(FRAME_BAR), &FrameBar,
					  END_OF_ARGS);
		MiniportMMRequest(pAd, QID_AC_BE, pOutBuffer2, FrameLen);
		MlmeFreeMemory(pAd, pOutBuffer2);
		DBGPRINT(RT_DEBUG_TRACE,("BA - MlmeDELBAAction() . Send BAR to refresh peer reordering buffer \n"));

		/* SEND DELBA FRAME*/
		FrameLen = 0;
//CFG_TODO		
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
#ifdef APCLI_SUPPORT
#ifdef MAC_REPEATER_SUPPORT
			if (IS_ENTRY_APCLI(pEntry) && pEntry->bReptCli)
			{
				UINT apidx = pEntry->wdev_idx;
				ActHeaderInit(pAd, &Frame.Hdr, pEntry->Addr,
							pAd->ApCfg.ApCliTab[apidx].RepeaterCli[pEntry->MatchReptCliIdx].CurrentAddress, pAd->MacTab.Content[pInfo->Wcid].Addr);
			}
			else
#endif /* MAC_REPEATER_SUPPORT */
#endif /* APCLI_SUPPORT */
			ActHeaderInit(pAd, &Frame.Hdr,
							pEntry->Addr,
							pEntry->wdev->if_addr,
							pEntry->wdev->bssid);
		}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{
			if (ADHOC_ON(pAd)
#ifdef QOS_DLS_SUPPORT
				|| (IS_ENTRY_DLS(pEntry))
#endif /* QOS_DLS_SUPPORT */
#ifdef DOT11Z_TDLS_SUPPORT
				|| (IS_ENTRY_TDLS(pEntry))
#endif /* DOT11Z_TDLS_SUPPORT */
				)
				ActHeaderInit(pAd, &Frame.Hdr, pEntry->Addr, pAd->StaCfg.wdev.if_addr, pAd->StaCfg.wdev.bssid);
			else
				ActHeaderInit(pAd, &Frame.Hdr, pEntry->Addr, pAd->StaCfg.wdev.if_addr, pAd->StaCfg.wdev.bssid);
		}
#endif /* CONFIG_STA_SUPPORT */

		Frame.Category = CATEGORY_BA;
		Frame.Action = DELBA;
		Frame.DelbaParm.Initiator = pInfo->Initiator;
		Frame.DelbaParm.TID = pInfo->TID;
		Frame.ReasonCode = 39; /* Time Out*/
		*(USHORT *)(&Frame.DelbaParm) = cpu2le16(*(USHORT *)(&Frame.DelbaParm));
		Frame.ReasonCode = cpu2le16(Frame.ReasonCode);
		
		MakeOutgoingFrame(pOutBuffer, &FrameLen,
		              sizeof(FRAME_DELBA_REQ), &Frame,
		              END_OF_ARGS);
		MiniportMMRequest(pAd, QID_AC_BE, pOutBuffer, FrameLen);
		MlmeFreeMemory(pAd, pOutBuffer);		
		{
			int tid=0;
			MAC_TABLE_ENTRY *pEntry;
			pEntry = &pAd->MacTab.Content[pInfo->Wcid];
			for (tid=0; tid<NUM_OF_TID; tid++)
			    	pEntry->TxBarSeq[tid] = -1;
		}
		DBGPRINT(RT_DEBUG_TRACE, ("BA - MlmeDELBAAction() . 3 DELBA sent. Initiator(%d)\n", pInfo->Initiator));
    	}
}
#endif /* DOT11_N_SUPPORT */


VOID MlmeQOSAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem) 
{
#if defined(CONFIG_AP_SUPPORT) && defined(CONFIG_HOTSPOT_R2)
	MLME_QOS_ACTION_STRUCT *pInfo;
	PUCHAR			pOutBuffer = NULL;
	ULONG			FrameLen = 0;
	NDIS_STATUS 	NStatus;
	FRAME_ACTION_HDR	Frame;
	pInfo = (MLME_QOS_ACTION_STRUCT *)Elem->Msg;

	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  /*Get an unused nonpaged memory*/
	if(NStatus != NDIS_STATUS_SUCCESS) 
	{
		DBGPRINT(RT_DEBUG_ERROR,("ACT - MlmeQOSAction() allocate memory failed \n"));
		return;
	}

	if (pInfo->ActionField == ACTION_QOSMAP_CONFIG)
	{
		UCHAR OosMapIE = IE_QOS_MAP_SET;
		QOSMAP_SET *pQosMapBuf = &pInfo->QOSMap;
		MULTISSID_STRUCT *mbss = &pAd->ApCfg.MBSSID[pInfo->apidx];
		UCHAR	ielen = 0;
		
		ActHeaderInit(pAd, &Frame.Hdr, pInfo->Addr, pAd->ApCfg.MBSSID[pInfo->apidx].wdev.bssid, pAd->ApCfg.MBSSID[pInfo->apidx].wdev.bssid);				   

		Frame.Category = CATEGORY_QOS;
		Frame.Action = ACTION_QOSMAP_CONFIG;

		ielen = pQosMapBuf->DSCP_Field_Len;
						
		MakeOutgoingFrame(pOutBuffer, &FrameLen,
					sizeof(FRAME_ACTION_HDR),	  &Frame,	
					1,			&OosMapIE,
					1,			&ielen,
					ielen,		pQosMapBuf->DSCP_Field,
					END_OF_ARGS);
					
		MiniportMMRequest(pAd, QID_AC_BE, pOutBuffer, FrameLen);
	}

	MlmeFreeMemory(pAd, pOutBuffer);		
#endif /* defined(CONFIG_AP_SUPPORT) && defined(CONFIG_HOTSPOT_R2) */

}


VOID MlmeDLSAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem) 
{
}


VOID MlmeInvalidAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem) 
{
	/*PUCHAR		   pOutBuffer = NULL;*/
	/*Return the receiving frame except the MSB of category filed set to 1.  7.3.1.11*/
}


VOID PeerQOSAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem) 
{
}


#ifdef QOS_DLS_SUPPORT
VOID PeerDLSAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem) 
{
	UCHAR Action = Elem->Msg[LENGTH_802_11+1];

	switch(Action)
	{
		case ACTION_DLS_REQUEST:
#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
				APPeerDlsReqAction(pAd, Elem);
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
			PeerDlsReqAction(pAd, Elem);
#endif /* CONFIG_STA_SUPPORT */
			break;

		case ACTION_DLS_RESPONSE:
#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
				APPeerDlsRspAction(pAd, Elem);
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
			PeerDlsRspAction(pAd, Elem);
#endif /* CONFIG_STA_SUPPORT */
			break;

		case ACTION_DLS_TEARDOWN:
#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
				APPeerDlsTearDownAction(pAd, Elem);
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
			PeerDlsTearDownAction(pAd, Elem);
#endif /* CONFIG_STA_SUPPORT */
			break;
	}
}
#endif /* QOS_DLS_SUPPORT */




#ifdef DOT11_N_SUPPORT
VOID PeerBAAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem) 
{
	UCHAR Action = Elem->Msg[LENGTH_802_11+1];
	
	switch(Action)
	{
		case ADDBA_REQ:
			PeerAddBAReqAction(pAd,Elem);
			break;
		case ADDBA_RESP:
			PeerAddBARspAction(pAd,Elem);
			break;
		case DELBA:
			PeerDelBAAction(pAd,Elem);
			break;
	}
}


#ifdef DOT11N_DRAFT3
#ifdef CONFIG_AP_SUPPORT
extern UCHAR get_regulatory_class(IN PRTMP_ADAPTER pAd);

VOID SendBSS2040CoexistMgmtAction(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR Wcid,
	IN UCHAR apidx,
	IN UCHAR InfoReq)
{
	UCHAR *pOutBuffer = NULL;
	NDIS_STATUS NStatus;
	FRAME_ACTION_HDR Frame;
	ULONG FrameLen;
	BSS_2040_COEXIST_ELEMENT BssCoexistInfo;
	BSS_2040_INTOLERANT_CH_REPORT BssIntolerantInfo;
	UCHAR *pAddr1;
	struct wifi_dev *wdev;

	DBGPRINT(RT_DEBUG_TRACE, ("SendBSS2040CoexistMgmtAction(): Wcid=%d, apidx=%d, InfoReq=%d!\n", Wcid, apidx, InfoReq));
	
	NdisZeroMemory((PUCHAR)&BssCoexistInfo, sizeof(BSS_2040_COEXIST_ELEMENT));
	NdisZeroMemory((PUCHAR)&BssIntolerantInfo, sizeof(BSS_2040_INTOLERANT_CH_REPORT));
	
	BssCoexistInfo.ElementID = IE_2040_BSS_COEXIST;
	BssCoexistInfo.Len = 1;
	BssCoexistInfo.BssCoexistIe.word = pAd->CommonCfg.LastBSSCoexist2040.word;
	BssCoexistInfo.BssCoexistIe.field.InfoReq = InfoReq;
	BssIntolerantInfo.ElementID = IE_2040_BSS_INTOLERANT_REPORT;
	BssIntolerantInfo.Len = 1;
	BssIntolerantInfo.RegulatoryClass = get_regulatory_class(pAd);

	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  /*Get an unused nonpaged memory*/
	if(NStatus != NDIS_STATUS_SUCCESS) 
	{
		DBGPRINT(RT_DEBUG_ERROR,("ACT - SendBSS2040CoexistMgmtAction() allocate memory failed \n"));
		return;
	}

	if (Wcid == MCAST_WCID)
		pAddr1 = &BROADCAST_ADDR[0];
	else
		pAddr1 = pAd->MacTab.Content[Wcid].Addr;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	ActHeaderInit(pAd, &Frame.Hdr, pAddr1, wdev->if_addr, wdev->bssid);
	
	Frame.Category = CATEGORY_PUBLIC;
	Frame.Action = ACTION_BSS_2040_COEXIST;
	
	MakeOutgoingFrame(pOutBuffer, &FrameLen,
					sizeof(FRAME_ACTION_HDR), &Frame,
					sizeof(BSS_2040_COEXIST_ELEMENT), &BssCoexistInfo,
					sizeof(BSS_2040_INTOLERANT_CH_REPORT), &BssIntolerantInfo,
				  	END_OF_ARGS);
	
	MiniportMMRequest(pAd, QID_AC_BE, pOutBuffer, FrameLen);
	MlmeFreeMemory(pAd, pOutBuffer);
	
	DBGPRINT(RT_DEBUG_ERROR,("ACT - SendBSS2040CoexistMgmtAction(BSSCoexist2040=0x%x)\n", BssCoexistInfo.BssCoexistIe.word));
	
}
#endif /* CONFIG_AP_SUPPORT */


#ifdef CONFIG_STA_SUPPORT
VOID StaPublicAction(RTMP_ADAPTER *pAd, BSS_2040_COEXIST_IE *pBssCoexIE)
{
	MLME_SCAN_REQ_STRUCT ScanReq;

	DBGPRINT(RT_DEBUG_TRACE,("ACTION - StaPeerPublicAction  Bss2040Coexist = %x\n", *((PUCHAR)pBssCoexIE)));

	/* AP asks Station to return a 20/40 BSS Coexistence mgmt frame.  So we first starts a scan, then send back 20/40 BSS Coexistence mgmt frame */
	if ((pBssCoexIE->field.InfoReq == 1) && (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SCAN_2040)))
	{
		/* Clear record first.  After scan , will update those bit and send back to transmiter.*/
		pAd->CommonCfg.BSSCoexist2040.field.InfoReq = 1;
		pAd->CommonCfg.BSSCoexist2040.field.Intolerant40 = 0;
		pAd->CommonCfg.BSSCoexist2040.field.BSS20WidthReq = 0;
		/* Clear Trigger event table*/
		TriEventInit(pAd);
		/* Fill out stuff for scan request  and kick to scan*/
		ScanParmFill(pAd, &ScanReq, ZeroSsid, 0, BSS_ANY, SCAN_2040_BSS_COEXIST);
		MlmeEnqueue(pAd, SYNC_STATE_MACHINE, MT2_MLME_SCAN_REQ, sizeof(MLME_SCAN_REQ_STRUCT), &ScanReq, 0);
		pAd->Mlme.CntlMachine.CurrState = CNTL_WAIT_OID_LIST_SCAN;
		RTMP_MLME_HANDLER(pAd);
	}
}

VOID UpdateBssScanParm(
	IN	PRTMP_ADAPTER	pAd,
	IN	OVERLAP_BSS_SCAN_IE	APBssScan) 
{									 
	pAd->CommonCfg.Dot11BssWidthChanTranDelayFactor = le2cpu16(APBssScan.DelayFactor); /*APBssScan.DelayFactor[1] * 256 + APBssScan.DelayFactor[0];*/
	/* out of range defined in MIB... So fall back to default value.*/
	if ((pAd->CommonCfg.Dot11BssWidthChanTranDelayFactor <5) || (pAd->CommonCfg.Dot11BssWidthChanTranDelayFactor > 100))
	{
		/*DBGPRINT(RT_DEBUG_ERROR,("ACT - UpdateBssScanParm( Dot11BssWidthChanTranDelayFactor out of range !!!!)  \n"));*/
		pAd->CommonCfg.Dot11BssWidthChanTranDelayFactor = 5;
	}

	pAd->CommonCfg.Dot11BssWidthTriggerScanInt = le2cpu16(APBssScan.TriggerScanInt); /*APBssScan.TriggerScanInt[1] * 256 + APBssScan.TriggerScanInt[0];*/
	/* out of range defined in MIB... So fall back to default value.*/
	if ((pAd->CommonCfg.Dot11BssWidthTriggerScanInt < 10) ||(pAd->CommonCfg.Dot11BssWidthTriggerScanInt > 900))
	{
		/*DBGPRINT(RT_DEBUG_ERROR,("ACT - UpdateBssScanParm( Dot11BssWidthTriggerScanInt out of range !!!!)  \n"));*/
		pAd->CommonCfg.Dot11BssWidthTriggerScanInt = 900;
	}
		
	pAd->CommonCfg.Dot11OBssScanPassiveDwell = le2cpu16(APBssScan.ScanPassiveDwell); /*APBssScan.ScanPassiveDwell[1] * 256 + APBssScan.ScanPassiveDwell[0];*/
	/* out of range defined in MIB... So fall back to default value.*/
	if ((pAd->CommonCfg.Dot11OBssScanPassiveDwell < 5) ||(pAd->CommonCfg.Dot11OBssScanPassiveDwell > 1000))
	{
		/*DBGPRINT(RT_DEBUG_ERROR,("ACT - UpdateBssScanParm( Dot11OBssScanPassiveDwell out of range !!!!)  \n"));*/
		pAd->CommonCfg.Dot11OBssScanPassiveDwell = 20;
	}
	
	pAd->CommonCfg.Dot11OBssScanActiveDwell = le2cpu16(APBssScan.ScanActiveDwell); /*APBssScan.ScanActiveDwell[1] * 256 + APBssScan.ScanActiveDwell[0];*/
	/* out of range defined in MIB... So fall back to default value.*/
	if ((pAd->CommonCfg.Dot11OBssScanActiveDwell < 10) ||(pAd->CommonCfg.Dot11OBssScanActiveDwell > 1000))
	{
		/*DBGPRINT(RT_DEBUG_ERROR,("ACT - UpdateBssScanParm( Dot11OBssScanActiveDwell out of range !!!!)  \n"));*/
		pAd->CommonCfg.Dot11OBssScanActiveDwell = 10;
	}

	pAd->CommonCfg.Dot11OBssScanPassiveTotalPerChannel = le2cpu16(APBssScan.PassiveTalPerChannel); /*APBssScan.PassiveTalPerChannel[1] * 256 + APBssScan.PassiveTalPerChannel[0];*/
	/* out of range defined in MIB... So fall back to default value.*/
	if ((pAd->CommonCfg.Dot11OBssScanPassiveTotalPerChannel < 200) ||(pAd->CommonCfg.Dot11OBssScanPassiveTotalPerChannel > 10000))
	{
		/*DBGPRINT(RT_DEBUG_ERROR,("ACT - UpdateBssScanParm( Dot11OBssScanPassiveTotalPerChannel out of range !!!!)  \n"));*/
		pAd->CommonCfg.Dot11OBssScanPassiveTotalPerChannel = 200;
	}
	
	pAd->CommonCfg.Dot11OBssScanActiveTotalPerChannel = le2cpu16(APBssScan.ActiveTalPerChannel); /*APBssScan.ActiveTalPerChannel[1] * 256 + APBssScan.ActiveTalPerChannel[0];*/
	/* out of range defined in MIB... So fall back to default value.*/
	if ((pAd->CommonCfg.Dot11OBssScanActiveTotalPerChannel < 20) ||(pAd->CommonCfg.Dot11OBssScanActiveTotalPerChannel > 10000))
	{
		/*DBGPRINT(RT_DEBUG_ERROR,("ACT - UpdateBssScanParm( Dot11OBssScanActiveTotalPerChannel out of range !!!!)  \n"));*/
		pAd->CommonCfg.Dot11OBssScanActiveTotalPerChannel = 20;
	}

	pAd->CommonCfg.Dot11OBssScanActivityThre = le2cpu16(APBssScan.ScanActThre); /*APBssScan.ScanActThre[1] * 256 + APBssScan.ScanActThre[0];*/
	/* out of range defined in MIB... So fall back to default value.*/
	if (pAd->CommonCfg.Dot11OBssScanActivityThre > 100)
	{
		/*DBGPRINT(RT_DEBUG_ERROR,("ACT - UpdateBssScanParm( Dot11OBssScanActivityThre out of range !!!!)  \n"));*/
		pAd->CommonCfg.Dot11OBssScanActivityThre = 25;
	}

	pAd->CommonCfg.Dot11BssWidthChanTranDelay = (pAd->CommonCfg.Dot11BssWidthTriggerScanInt * pAd->CommonCfg.Dot11BssWidthChanTranDelayFactor);
	/*DBGPRINT(RT_DEBUG_LOUD,("ACT - UpdateBssScanParm( Dot11BssWidthTriggerScanInt = %d )  \n", pAd->CommonCfg.Dot11BssWidthTriggerScanInt));*/
}

#endif /* CONFIG_STA_SUPPORT */

#if defined(CONFIG_STA_SUPPORT) || defined(APCLI_SUPPORT)

/*
Description : Build Intolerant Channel Rerpot from Trigger event table.
return : how many bytes copied. 
*/
ULONG BuildIntolerantChannelRep(RTMP_ADAPTER *pAd, UCHAR *pDest) 
{
	ULONG			FrameLen = 0;
	ULONG			ReadOffset = 0;
	UCHAR			i, j, k, idx = 0;
	/*UCHAR			LastRegClass = 0xff;*/
	UCHAR			ChannelList[MAX_TRIGGER_EVENT];
	UCHAR			TmpRegClass;
	UCHAR			RegClassArray[7] = {0, 11,12, 32, 33, 54,55}; /* Those regulatory class has channel in 2.4GHz. See Annex J.*/


	RTMPZeroMemory(ChannelList, MAX_TRIGGER_EVENT);

	/* Find every regulatory class*/
	for ( k = 0;k < 7;k++)
	{
		TmpRegClass = RegClassArray[k];
		
		idx = 0;
		/* Find Channel report with the same regulatory class in 2.4GHz.*/
		for ( i = 0;i < pAd->CommonCfg.TriggerEventTab.EventANo;i++)
		{
			if (pAd->CommonCfg.TriggerEventTab.EventA[i].bValid == TRUE)
			{
				if (pAd->CommonCfg.TriggerEventTab.EventA[i].RegClass == TmpRegClass)
				{				
					for (j = 0;j < idx;j++)
					{
						if (ChannelList[j] == (UCHAR)pAd->CommonCfg.TriggerEventTab.EventA[i].Channel)
							break;
					}
					if ((j == idx))
					{
						ChannelList[idx] = (UCHAR)pAd->CommonCfg.TriggerEventTab.EventA[i].Channel;
						idx++;
					} 
					pAd->CommonCfg.TriggerEventTab.EventA[i].bValid = FALSE;
				}
				DBGPRINT(RT_DEBUG_ERROR,("ACT - BuildIntolerantChannelRep , Total Channel number = %d \n", idx));
			}
		}

		/* idx > 0 means this regulatory class has some channel report and need to copy to the pDest.*/
		if (idx > 0)
		{
			/* For each regaulatory IE report, contains all channels that has the same regulatory class.*/
			*(pDest + ReadOffset) = IE_2040_BSS_INTOLERANT_REPORT;  /* IE*/
			*(pDest + ReadOffset + 1) = 1+ idx;	/* Len = RegClass byte + channel byte.*/
			*(pDest + ReadOffset + 2) = TmpRegClass;	/* Len = RegClass byte + channel byte.*/
			RTMPMoveMemory(pDest + ReadOffset + 3, ChannelList, idx);

			FrameLen += (3 + idx);
			ReadOffset += (3 + idx);
		}
		
	}

	DBGPRINT(RT_DEBUG_ERROR,("ACT-BuildIntolerantChannelRep(Size=%ld)\n", FrameLen));
	hex_dump("ACT-pDestMsg", pDest, FrameLen);

	return FrameLen;
}


/*	
	==========================================================================
	Description: 
	After scan, Update 20/40 BSS Coexistence IE and send out.
	According to 802.11n D3.03 11.14.10
		
	Parameters: 
	==========================================================================
 */
VOID Update2040CoexistFrameAndNotify(
	IN	PRTMP_ADAPTER	pAd,
	IN    UCHAR  Wcid,
	IN	BOOLEAN	bAddIntolerantCha) 
{
	/*BSS_2040_COEXIST_IE		OldValue;*/

	DBGPRINT(RT_DEBUG_ERROR,("%s(): ACT -BSSCoexist2040 = %x. EventANo = %d. \n",
				__FUNCTION__, pAd->CommonCfg.BSSCoexist2040.word,
				pAd->CommonCfg.TriggerEventTab.EventANo));

	/*OldValue.word = pAd->CommonCfg.BSSCoexist2040.word;*/
	/* Reset value.*/
	pAd->CommonCfg.BSSCoexist2040.word = 0;

	if (pAd->CommonCfg.TriggerEventTab.EventBCountDown > 0)
		pAd->CommonCfg.BSSCoexist2040.field.BSS20WidthReq = 1;

	/* Need to check !!!!*/
	/* How STA will set Intolerant40 if implementation dependent. Now we don't set this bit first!!!!!*/
	/* So Only check BSS20WidthReq change.*/
	/*if (OldValue.field.BSS20WidthReq != pAd->CommonCfg.BSSCoexist2040.field.BSS20WidthReq)*/
	{
		Send2040CoexistAction(pAd, Wcid, bAddIntolerantCha);
	}
}


/*
Description : Send 20/40 BSS Coexistence Action frame If one trigger event is triggered.
*/
VOID Send2040CoexistAction(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR Wcid,
	IN BOOLEAN bAddIntolerantCha) 
{
	UCHAR *pOutBuffer = NULL;
	NDIS_STATUS NStatus;
	FRAME_ACTION_HDR Frame;
	ULONG FrameLen;
	UINT32 IntolerantChaRepLen;
	UCHAR HtLen = 1;
#ifdef APCLI_SUPPORT
        UCHAR apidx;
#endif /* APCLI_SUPPORT */ 
	IntolerantChaRepLen = 0;
	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  /*Get an unused nonpaged memory*/
	if(NStatus != NDIS_STATUS_SUCCESS) 
	{
		DBGPRINT(RT_DEBUG_ERROR,("ACT - Send2040CoexistAction() allocate memory failed \n"));
		return;
	}

#ifdef DOT11V_WNM_SUPPORT
	/* Not complete yet. Ignore for compliing successfully.*/
#else
#ifdef APCLI_SUPPORT   
        if (IS_ENTRY_APCLI(&pAd->MacTab.Content[Wcid]))
        {               
                apidx = pAd->MacTab.Content[Wcid].apidx;
                ActHeaderInit(pAd, &Frame.Hdr, pAd->MacTab.Content[Wcid].Addr, pAd->ApCfg.ApCliTab[apidx].wdev.if_addr, pAd->MacTab.Content[Wcid].Addr);            
                DBGPRINT(RT_DEBUG_OFF, ("\x1b[31m\n%02x:%02x:%02x:%02x:%02x:%02x \x1b[m\n", 
                                                               pAd->MacTab.Content[Wcid].Addr[0], pAd->MacTab.Content[Wcid].Addr[1], pAd->MacTab.Content[Wcid].Addr[2],
                                                               pAd->MacTab.Content[Wcid].Addr[3], pAd->MacTab.Content[Wcid].Addr[4], pAd->MacTab.Content[Wcid].Addr[5]));
        }
        else
#endif /* APCLI_SUPPORT */   
	ActHeaderInit(pAd, &Frame.Hdr, pAd->MacTab.Content[Wcid].Addr, pAd->CurrentAddress, pAd->CommonCfg.Bssid);	
#endif /* DOT11V_WNM_SUPPORT */

	Frame.Category = CATEGORY_PUBLIC;
	Frame.Action = ACTION_BSS_2040_COEXIST; /*COEXIST_2040_ACTION;*/
	
	MakeOutgoingFrame(pOutBuffer,				&FrameLen,
				  sizeof(FRAME_ACTION_HDR),	  &Frame,
				  1,                                &BssCoexistIe,
				  1,                                &HtLen,
				  1,                                &pAd->CommonCfg.BSSCoexist2040.word,
				  END_OF_ARGS);
	
	if (bAddIntolerantCha == TRUE)
		IntolerantChaRepLen = BuildIntolerantChannelRep(pAd, pOutBuffer + FrameLen);

	/*2009 PF#3: IOT issue with Motorola AP. It will not check the field of BSSCoexist2040.*/
	/*11.14.12 Switching between 40 MHz and 20 MHz*/
	DBGPRINT(RT_DEBUG_TRACE, ("IntolerantChaRepLen=%d, BSSCoexist2040=0x%x!\n", 
								IntolerantChaRepLen, pAd->CommonCfg.BSSCoexist2040.word));
	if (!((IntolerantChaRepLen == 0) && (pAd->CommonCfg.BSSCoexist2040.word == 0)))
		MiniportMMRequest(pAd, QID_AC_BE, pOutBuffer, FrameLen + IntolerantChaRepLen);
		
	MlmeFreeMemory(pAd, pOutBuffer);
	
	DBGPRINT(RT_DEBUG_TRACE,("ACT - Send2040CoexistAction( BSSCoexist2040 = 0x%x )  \n", pAd->CommonCfg.BSSCoexist2040.word));
}



	
#endif /* defined(CONFIG_STA_SUPPORT) || defined(APCLI_SUPPORT) */


BOOLEAN ChannelSwitchSanityCheck(
	IN	PRTMP_ADAPTER	pAd,
	IN    UCHAR  Wcid,
	IN    UCHAR  NewChannel,
	IN    UCHAR  Secondary) 
{
	UCHAR		i;
	
	if (Wcid >= MAX_LEN_OF_MAC_TABLE)
		return FALSE;

	if ((NewChannel > 7) && (Secondary == 1))
		return FALSE;

	if ((NewChannel < 5) && (Secondary == 3))
		return FALSE;

	/* 0. Check if new channel is in the channellist.*/
	for (i = 0;i < pAd->ChannelListNum;i++)
	{
		if (pAd->ChannelList[i].Channel == NewChannel)
		{
			break;
		}
	}

	if (i == pAd->ChannelListNum)
		return FALSE;
	
	return TRUE;
}


VOID ChannelSwitchAction(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR Wcid,
	IN UCHAR NewChannel,
	IN UCHAR Secondary)
{
	UCHAR rf_channel = 0, rf_bw;

	DBGPRINT(RT_DEBUG_TRACE,("%s(): NewChannel=%d, Secondary=%d\n", 
				__FUNCTION__, NewChannel, Secondary));

	if (ChannelSwitchSanityCheck(pAd, Wcid, NewChannel, Secondary) == FALSE)
		return;

	pAd->CommonCfg.Channel = NewChannel;
	if (Secondary == EXTCHA_NONE)
	{
		pAd->CommonCfg.CentralChannel = pAd->CommonCfg.Channel;
		pAd->MacTab.Content[Wcid].HTPhyMode.field.BW = 0;
		pAd->CommonCfg.AddHTInfo.AddHtInfo.RecomWidth = 0;
		pAd->CommonCfg.AddHTInfo.AddHtInfo.ExtChanOffset = 0;		

		rf_bw = BW_20;
		rf_channel = pAd->CommonCfg.Channel;
	}
	/* 1.  Switches to BW = 40 And Station supports BW = 40.*/
	else if (((Secondary == EXTCHA_ABOVE) || (Secondary == EXTCHA_BELOW)) &&
			(pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth == 1)
	)
	{
		rf_bw = BW_40;
#ifdef GREENAP_SUPPORT
		if (pAd->ApCfg.bGreenAPActive == 1)
		{
			rf_bw = BW_20;
			pAd->CommonCfg.CentralChannel = pAd->CommonCfg.Channel;
		}
		else
#endif /* GREENAP_SUPPORT */
		if (Secondary == EXTCHA_ABOVE)
			pAd->CommonCfg.CentralChannel = pAd->CommonCfg.Channel + 2;
		else
			pAd->CommonCfg.CentralChannel = pAd->CommonCfg.Channel - 2;

		rf_channel = pAd->CommonCfg.CentralChannel;
		pAd->MacTab.Content[Wcid].HTPhyMode.field.BW = 1;
	}

	if (rf_channel != 0) {
		AsicSetChannel(pAd, rf_channel, rf_bw, Secondary, FALSE);
		
		DBGPRINT(RT_DEBUG_TRACE, ("%s(): %dMHz LINK UP, CtrlChannel=%d,  CentralChannel= %d \n",
					__FUNCTION__, (rf_bw == BW_40 ? 40 : 20),
					pAd->CommonCfg.Channel, 
					pAd->CommonCfg.CentralChannel));
#ifdef SMART_MESH_MONITOR
		{
			struct nsmpif_drvevnt_buf drvevnt;
			drvevnt.data.channel_change.type = NSMPIF_DRVEVNT_CHANNEL_CHANGE;
			drvevnt.data.channel_change.channel = pAd->CommonCfg.Channel;
			NdisZeroMemory(drvevnt.data.channel_change.op_channels,sizeof(drvevnt.data.channel_change.op_channels));
			drvevnt.data.channel_change.op_channels[0] = pAd->CommonCfg.Channel;
#ifdef DOT11_N_SUPPORT
			if(rf_bw == BW_40)
				drvevnt.data.channel_change.op_channels[1] = N_GetSecondaryChannel(pAd);
#endif /* DOT11_N_SUPPORT */
			RtmpOSWrielessEventSend(pAd->net_dev, RT_WLAN_EVENT_CUSTOM,NSMPIF_DRVEVNT_CHANNEL_CHANGE,
									NULL, (PUCHAR)&drvevnt.data.channel_change, sizeof(drvevnt.data.channel_change));
		}
#endif /* SMART_MESH_MONITOR */
	}
}
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */


VOID PeerPublicAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem) 
{
	UCHAR Action = Elem->Msg[LENGTH_802_11+1];

#if defined(CONFIG_HOTSPOT) && defined(CONFIG_AP_SUPPORT)
	if (!HotSpotEnable(pAd, Elem, ACTION_STATE_MESSAGES))
#endif
	if ((Elem->Wcid >= MAX_LEN_OF_MAC_TABLE)
#ifdef DOT11Z_TDLS_SUPPORT
		&& (Action != ACTION_TDLS_DISCOVERY_RSP)
#endif // DOT11Z_TDLS_SUPPORT //
		)
		return;


	switch(Action)
	{
#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
		case ACTION_BSS_2040_COEXIST:	/* Format defined in IEEE 7.4.7a.1 in 11n Draf3.03*/
			{
				/*UCHAR	BssCoexist;*/
				BSS_2040_COEXIST_ELEMENT		*pCoexistInfo;
				BSS_2040_COEXIST_IE 			*pBssCoexistIe;
				BSS_2040_INTOLERANT_CH_REPORT	*pIntolerantReport = NULL;
				
				if (Elem->MsgLen <= (LENGTH_802_11 + sizeof(BSS_2040_COEXIST_ELEMENT)) )
				{
					DBGPRINT(RT_DEBUG_ERROR, ("ACTION - 20/40 BSS Coexistence Management Frame length too short! len = %ld!\n", Elem->MsgLen));
					break;
				}			
				DBGPRINT(RT_DEBUG_TRACE, ("ACTION - 20/40 BSS Coexistence Management action----> \n"));
				hex_dump("CoexistenceMgmtFrame", Elem->Msg, Elem->MsgLen);

				
				pCoexistInfo = (BSS_2040_COEXIST_ELEMENT *) &Elem->Msg[LENGTH_802_11+2];
				/*hex_dump("CoexistInfo", (PUCHAR)pCoexistInfo, sizeof(BSS_2040_COEXIST_ELEMENT));*/
				if (Elem->MsgLen >= (LENGTH_802_11 + sizeof(BSS_2040_COEXIST_ELEMENT) + sizeof(BSS_2040_INTOLERANT_CH_REPORT)))
				{
					pIntolerantReport = (BSS_2040_INTOLERANT_CH_REPORT *)((PUCHAR)pCoexistInfo + sizeof(BSS_2040_COEXIST_ELEMENT));
				}
				/*hex_dump("IntolerantReport ", (PUCHAR)pIntolerantReport, sizeof(BSS_2040_INTOLERANT_CH_REPORT));*/
				
				if(pAd->CommonCfg.bBssCoexEnable == FALSE || (pAd->CommonCfg.bForty_Mhz_Intolerant == TRUE))
				{
					DBGPRINT(RT_DEBUG_TRACE, ("20/40 BSS CoexMgmt=%d, bForty_Mhz_Intolerant=%d, ignore this action!!\n", 
												pAd->CommonCfg.bBssCoexEnable,
												pAd->CommonCfg.bForty_Mhz_Intolerant));
					break;
				}

				pBssCoexistIe = (BSS_2040_COEXIST_IE *)(&pCoexistInfo->BssCoexistIe);
#ifdef CONFIG_AP_SUPPORT
				IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
				{
#ifdef APCLI_SUPPORT
#ifdef APCLI_CERT_SUPPORT
					if (!IS_ENTRY_APCLI(&pAd->MacTab.Content[Elem->Wcid]))
					{
#endif /* APCLI_CERT_SUPPORT */
#endif /* APCLI_SUPPORT */				
					BOOLEAN		bNeedFallBack = FALSE;
									
					/*ApPublicAction(pAd, Elem);*/
					if ((pBssCoexistIe->field.BSS20WidthReq ==1) || (pBssCoexistIe->field.Intolerant40 == 1))
					{	
						bNeedFallBack = TRUE;

						DBGPRINT(RT_DEBUG_TRACE, ("BSS_2040_COEXIST: BSS20WidthReq=%d, Intolerant40=%d!\n", pBssCoexistIe->field.BSS20WidthReq, pBssCoexistIe->field.Intolerant40));
					}
					else if ((pIntolerantReport) && (pIntolerantReport->Len > 1)
							/*&& (pIntolerantReport->RegulatoryClass == get_regulatory_class(pAd))*/)
					{
						int i;
						UCHAR *ptr;
						INT retVal;
						BSS_COEX_CH_RANGE coexChRange;

						ptr = pIntolerantReport->ChList;
						bNeedFallBack = TRUE;
						
						DBGPRINT(RT_DEBUG_TRACE, ("The pIntolerantReport len = %d, chlist=", pIntolerantReport->Len));
						for(i =0 ; i < (pIntolerantReport->Len -1); i++, ptr++)
						{
							DBGPRINT(RT_DEBUG_TRACE, ("%d,", *ptr));
						}
						DBGPRINT(RT_DEBUG_TRACE, ("\n"));

						retVal = GetBssCoexEffectedChRange(pAd, &coexChRange);
						if (retVal == TRUE)
						{
							ptr = pIntolerantReport->ChList;
							bNeedFallBack = FALSE;
							DBGPRINT(RT_DEBUG_TRACE, ("Check IntolerantReport Channel List in our effectedChList(%d~%d)\n",
													pAd->ChannelList[coexChRange.effectChStart].Channel,
													pAd->ChannelList[coexChRange.effectChEnd].Channel));
							for(i =0 ; i < (pIntolerantReport->Len -1); i++, ptr++)
							{
								UCHAR chEntry;

								chEntry = *ptr;
								if (chEntry >= pAd->ChannelList[coexChRange.effectChStart].Channel && 
									chEntry <= pAd->ChannelList[coexChRange.effectChEnd].Channel)
								{
									DBGPRINT(RT_DEBUG_TRACE, ("Found Intolerant channel in effect range=%d!\n", *ptr));
									bNeedFallBack = TRUE;
									break;
								}
							}
							DBGPRINT(RT_DEBUG_TRACE, ("After CoexChRange Check, bNeedFallBack=%d!\n", bNeedFallBack));
						}
						
						if (bNeedFallBack)
						{
							pBssCoexistIe->field.Intolerant40 = 1;
							pBssCoexistIe->field.BSS20WidthReq = 1;
						}
					}

					if (bNeedFallBack)
					{
						int apidx;
						
						NdisMoveMemory((PUCHAR)&pAd->CommonCfg.LastBSSCoexist2040, (PUCHAR)pBssCoexistIe, sizeof(BSS_2040_COEXIST_IE));
						pAd->CommonCfg.Bss2040CoexistFlag |= BSS_2040_COEXIST_INFO_SYNC;

						if (!(pAd->CommonCfg.Bss2040CoexistFlag & BSS_2040_COEXIST_TIMER_FIRED))
						{	
							DBGPRINT(RT_DEBUG_TRACE, ("Fire the Bss2040CoexistTimer with timeout=%ld!\n", 
									pAd->CommonCfg.Dot11BssWidthChanTranDelay));

							pAd->CommonCfg.Bss2040CoexistFlag |= BSS_2040_COEXIST_TIMER_FIRED;
							/* More 5 sec for the scan report of STAs.*/
							RTMPSetTimer(&pAd->CommonCfg.Bss2040CoexistTimer,  (pAd->CommonCfg.Dot11BssWidthChanTranDelay + 5) * 1000);

						}
						else
						{
							DBGPRINT(RT_DEBUG_TRACE, ("Already fallback to 20MHz, Extend the timeout of Bss2040CoexistTimer!\n"));
							/* More 5 sec for the scan report of STAs.*/
							RTMPModTimer(&pAd->CommonCfg.Bss2040CoexistTimer, (pAd->CommonCfg.Dot11BssWidthChanTranDelay + 5) * 1000);
						}

						apidx = pAd->MacTab.Content[Elem->Wcid].apidx;
						for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++)
							SendBSS2040CoexistMgmtAction(pAd, MCAST_WCID, apidx, 0);
					}
#ifdef APCLI_SUPPORT	
#ifdef APCLI_CERT_SUPPORT
					}	
#endif /* APCLI_CERT_SUPPORT */					
#endif /* APCLI_SUPPORT */						
				}
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
				IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
				{
					if (INFRA_ON(pAd))
					{
						StaPublicAction(pAd, pBssCoexistIe);
					}
				}
#endif /* CONFIG_STA_SUPPORT */
			}
			break;
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */

#if defined(CONFIG_HOTSPOT) && defined(CONFIG_AP_SUPPORT)
		case ACTION_GAS_INIT_REQ:
			if (HotSpotEnable(pAd, Elem, ACTION_STATE_MESSAGES))
			{
				ReceiveGASInitReq(pAd, Elem);
}
			break;
		case ACTION_GAS_CB_REQ:
			if (HotSpotEnable(pAd, Elem, ACTION_STATE_MESSAGES))
				ReceiveGASCBReq(pAd, Elem);
			break;
#endif
		case ACTION_WIFI_DIRECT:
#ifdef IWSC_SUPPORT
			if ((pAd->StaCfg.BssType == BSS_ADHOC) &&
				NdisEqualMemory(&(Elem->Msg[LENGTH_802_11+2]), IWSC_ACTION_OUI, 4)) 
			{
				MlmeEnqueue(pAd, IWSC_STATE_MACHINE, IWSC_MT2_PEER_ACTION_FRAME, Elem->MsgLen, Elem->Msg, 0);
				RTMP_MLME_HANDLER(pAd);
				break;
			}
#endif // IWSC_SUPPORT //

			break;

#ifdef DOT11Z_TDLS_SUPPORT
		case ACTION_TDLS_DISCOVERY_RSP:
			IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
			{
				if (INFRA_ON(pAd))
				{
					TDLS_DiscoveryRspPublicAction(pAd, Elem, Elem->Msg, Elem->MsgLen);
				}
			}
			break;
#endif // DOT11Z_TDLS_SUPPORT //

		default:
			break;
	}

}	


static VOID ReservedAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	UCHAR Category;

	if (Elem->MsgLen <= LENGTH_802_11)
	{
		return;
	}

	Category = Elem->Msg[LENGTH_802_11];
	DBGPRINT(RT_DEBUG_TRACE,("Rcv reserved category(%d) Action Frame\n", Category));
	hex_dump("Reserved Action Frame", &Elem->Msg[0], Elem->MsgLen);
}


VOID PeerRMAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem) 
{
#ifdef CONFIG_AP_SUPPORT
#ifdef DOT11K_RRM_SUPPORT
	UCHAR Action = Elem->Msg[LENGTH_802_11+1];
	MAC_TABLE_ENTRY *pEntry = NULL;

	if (VALID_WCID(Elem->Wcid))
		pEntry = &pAd->MacTab.Content[Elem->Wcid];
	else
		return;

	if ((pEntry->apidx < pAd->ApCfg.BssidNum) &&
		!IS_RRM_ENABLE(pAd, pEntry->apidx))
		return;

	switch(Action)
	{
		case RRM_MEASURE_REP:
			DBGPRINT(RT_DEBUG_TRACE, ("%s: Get RRM Measure report.\n",
				__FUNCTION__));

			RRM_PeerMeasureRepAction(pAd, Elem);
			break;

		case RRM_NEIGHTBOR_REQ:
			RRM_PeerNeighborReqAction(pAd, Elem);
			break;
	}
#endif /* DOT11K_RRM_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
	return;
}


#ifdef DOT11_N_SUPPORT

#ifdef CONFIG_AP_SUPPORT
#ifdef DOT11N_DRAFT3
VOID SendNotifyBWActionFrame(RTMP_ADAPTER *pAd, UCHAR Wcid, UCHAR apidx)
{
	UCHAR *pOutBuffer = NULL, *pAddr1;
	NDIS_STATUS NStatus;
	FRAME_ACTION_HDR Frame;
	ULONG FrameLen;
	struct wifi_dev *wdev;


	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  /* Get an unused nonpaged memory */
	if(NStatus != NDIS_STATUS_SUCCESS) 
	{
		DBGPRINT(RT_DEBUG_ERROR,("ACT - SendNotifyBWAction() allocate memory failed \n"));
		return;
	}

	if (Wcid == MCAST_WCID)
		pAddr1 = &BROADCAST_ADDR[0];
	else
		pAddr1 = pAd->MacTab.Content[Wcid].Addr;

	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
	ActHeaderInit(pAd, &Frame.Hdr, pAddr1, wdev->if_addr, wdev->bssid);
	Frame.Category = CATEGORY_HT;
	Frame.Action = NOTIFY_BW_ACTION;
	MakeOutgoingFrame(pOutBuffer, &FrameLen,
				  sizeof(FRAME_ACTION_HDR), &Frame,
				  END_OF_ARGS);

	*(pOutBuffer + FrameLen) = pAd->CommonCfg.AddHTInfo.AddHtInfo.RecomWidth;
	FrameLen++;

	MiniportMMRequest(pAd, QID_AC_BE, pOutBuffer, FrameLen);
	MlmeFreeMemory(pAd, pOutBuffer);
	DBGPRINT(RT_DEBUG_TRACE,("ACT - SendNotifyBWAction(NotifyBW= %d)!\n", pAd->CommonCfg.AddHTInfo.AddHtInfo.RecomWidth));

}
#endif /* DOT11N_DRAFT3 */
#endif /* CONFIG_AP_SUPPORT */


VOID PeerHTAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem) 
{
	UCHAR Action = Elem->Msg[LENGTH_802_11+1];
	MAC_TABLE_ENTRY *pEntry;
	
	if (Elem->Wcid >= MAX_LEN_OF_MAC_TABLE)
		return;

	pEntry = &pAd->MacTab.Content[Elem->Wcid];

	switch(Action)
	{
		case NOTIFY_BW_ACTION:
			DBGPRINT(RT_DEBUG_TRACE,("ACTION - HT Notify Channel bandwidth action----> \n"));
#ifdef CONFIG_STA_SUPPORT
			if(pAd->StaActive.SupportedPhyInfo.bHtEnable == FALSE)
			{
				/* Note, this is to patch DIR-1353 AP. When the AP set to Wep, it will use legacy mode. But AP still keeps */
				/* sending BW_Notify Action frame, and cause us to linkup and linkdown. */
				/* In legacy mode, don't need to parse HT action frame.*/
				DBGPRINT(RT_DEBUG_TRACE,("ACTION -Ignore HT Notify Channel BW when link as legacy mode. BW = %d---> \n", 
								Elem->Msg[LENGTH_802_11+2] ));
				break;
			}
#endif /* CONFIG_STA_SUPPORT */

			if (Elem->Msg[LENGTH_802_11+2] == 0)	/* 7.4.8.2. if value is 1, keep the same as supported channel bandwidth. */
				pEntry->HTPhyMode.field.BW = 0;
			else 
			{
				pEntry->HTPhyMode.field.BW = pEntry->MaxHTPhyMode.field.BW &
											pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth;
			}
			break;

		case SMPS_ACTION:
			/* 7.3.1.25*/
 			DBGPRINT(RT_DEBUG_TRACE,("ACTION - SMPS action----> \n"));
			if (((Elem->Msg[LENGTH_802_11+2] & 0x1) == 0))
				pEntry->MmpsMode = MMPS_DISABLE;
			else if (((Elem->Msg[LENGTH_802_11+2] & 0x2) == 0))
				pEntry->MmpsMode = MMPS_STATIC;
			else
				pEntry->MmpsMode = MMPS_DYNAMIC;

			DBGPRINT(RT_DEBUG_TRACE,("Wcid(%d) MIMO PS = %d\n", Elem->Wcid, pEntry->MmpsMode));
			/* rt2860c : add something for smps change.*/
			break;
 
		case SETPCO_ACTION:
			break;
			
		case MIMO_CHA_MEASURE_ACTION:
			break;
			
		default:
			DBGPRINT(RT_DEBUG_WARN,("%s(): Unknown HT Action:%d\n", __FUNCTION__, Action));
	    		break;
	}
}


#ifdef DOT11_VHT_AC
VOID PeerVHTAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	UCHAR Action = Elem->Msg[LENGTH_802_11+1];
	
	if (Elem->Wcid >= MAX_LEN_OF_MAC_TABLE)
		return;

	switch(Action)
	{
		case ACT_VHT_OPMODE_NOTIFY:
			{
				OPERATING_MODE *op_mode = (OPERATING_MODE *)&Elem->Msg[LENGTH_802_11+2];
				MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.Content[Elem->Wcid];

				DBGPRINT(RT_DEBUG_TRACE,("ACTION - Operating Mode Notification action---->\n"));
				hex_dump("OperatingModeNotify", &Elem->Msg[0], Elem->MsgLen);
				DBGPRINT(RT_DEBUG_TRACE, ("\t RxNssType=%d, RxNss=%d, ChBW=%d\n",
							op_mode->rx_nss_type, op_mode->rx_nss, op_mode->ch_width));

				if (op_mode->rx_nss_type == 0) {
					pEntry->force_op_mode = TRUE;
					NdisMoveMemory(&pEntry->operating_mode, op_mode, 1);
				}
			}
			break;
#ifdef VHT_TXBF_SUPPORT
		case ACT_VHT_COMPRESS_BF:
			{
				//DBGPRINT(RT_DEBUG_OFF,("ACTION - VHT Compressed Beamforming action---->\n"));
				//hex_dump("VHT Compressed BF", &Elem->Msg[0], Elem->MsgLen);
				break;
			}
#endif /* VHT_TXBF_SUPPORT */
		default:
			break;
	}
}
#endif /* DOT11_VHT_AC */




VOID SendRefreshBAR(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry) 
{
	FRAME_BAR FrameBar;
	ULONG FrameLen;
	NDIS_STATUS NStatus;
	UCHAR *pOutBuffer = NULL, i, TID;
	USHORT Sequence, idx;
	BA_ORI_ENTRY *pBAEntry;

	for (i = 0; i <NUM_OF_TID; i++)
	{
		idx = pEntry->BAOriWcidArray[i];
		if (idx == 0)
			continue;

		pBAEntry = &pAd->BATable.BAOriEntry[idx];

		if  (pBAEntry->ORI_BA_Status == Originator_Done)
		{
			TID = pBAEntry->TID;
			
			if (pEntry->TxBarSeq[TID] == pEntry->TxSeq[TID])
			{
				continue;
			}	

			ASSERT(pBAEntry->Wcid < MAX_LEN_OF_MAC_TABLE);

			NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  /*Get an unused nonpaged memory*/
			if(NStatus != NDIS_STATUS_SUCCESS) 
			{
				DBGPRINT(RT_DEBUG_ERROR,("BA - MlmeADDBAAction() allocate memory failed \n"));
				return;
			}

			Sequence = pEntry->TxSeq[TID];

			pEntry->TxBarSeq[TID] = pEntry->TxSeq[TID];

#ifdef APCLI_SUPPORT
#ifdef MAC_REPEATER_SUPPORT
			if (IS_ENTRY_APCLI(pEntry) && pEntry->bReptCli)
				BarHeaderInit(pAd, &FrameBar, pEntry->Addr, 
							pAd->ApCfg.ApCliTab[pEntry->wdev_idx].RepeaterCli[pEntry->MatchReptCliIdx].CurrentAddress);
			else
#endif /* MAC_REPEATER_SUPPORT */
#endif /* APCLI_SUPPORT */
			BarHeaderInit(pAd, &FrameBar, pEntry->Addr, pEntry->wdev->if_addr);

			FrameBar.StartingSeq.field.FragNum = 0; /* make sure sequence not clear in DEL function.*/
			FrameBar.StartingSeq.field.StartSeq = Sequence; /* make sure sequence not clear in DEL funciton.*/
			FrameBar.BarControl.TID = TID; /* make sure sequence not clear in DEL funciton.*/

			MakeOutgoingFrame(pOutBuffer,		&FrameLen,
							  sizeof(FRAME_BAR),	&FrameBar,
							  END_OF_ARGS);
			MiniportMMRequest(pAd, (MGMT_USE_QUEUE_FLAG | WMM_UP2AC_MAP[TID]), pOutBuffer, FrameLen);

			MlmeFreeMemory(pAd, pOutBuffer);
		}
	}
}
#endif /* DOT11_N_SUPPORT */


VOID ActHeaderInit(
    IN RTMP_ADAPTER *pAd,
    IN OUT PHEADER_802_11 pHdr80211,
    IN UCHAR *da,
    IN UCHAR *sa,
    IN UCHAR *bssid)
{
    NdisZeroMemory(pHdr80211, sizeof(HEADER_802_11));
	pHdr80211->FC.Type = FC_TYPE_MGMT;
    pHdr80211->FC.SubType = SUBTYPE_ACTION;

	COPY_MAC_ADDR(pHdr80211->Addr1, da);
	COPY_MAC_ADDR(pHdr80211->Addr2, sa);
	COPY_MAC_ADDR(pHdr80211->Addr3, bssid);
}


VOID BarHeaderInit(
	IN	PRTMP_ADAPTER	pAd, 
	IN OUT PFRAME_BAR pCntlBar, 
	IN PUCHAR pDA,
	IN PUCHAR pSA) 
{
/*	USHORT	Duration;*/

	NdisZeroMemory(pCntlBar, sizeof(FRAME_BAR));
	pCntlBar->FC.Type = FC_TYPE_CNTL;
	pCntlBar->FC.SubType = SUBTYPE_BLOCK_ACK_REQ;
   	pCntlBar->BarControl.MTID = 0;
	pCntlBar->BarControl.Compressed = 1;
	pCntlBar->BarControl.ACKPolicy = 0;


	pCntlBar->Duration = 16 + RTMPCalcDuration(pAd, RATE_1, sizeof(FRAME_BA));

	COPY_MAC_ADDR(pCntlBar->Addr1, pDA);
	COPY_MAC_ADDR(pCntlBar->Addr2, pSA);
}


/*
	==========================================================================
	Description:
		Insert Category and action code into the action frame.
		
	Parametrs:
		1. frame buffer pointer.
		2. frame length.
		3. category code of the frame.
		4. action code of the frame.
	
	Return	: None.
	==========================================================================
 */
VOID InsertActField(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN UINT8 Category,
	IN UINT8 ActCode)
{
	ULONG TempLen;

	MakeOutgoingFrame(	pFrameBuf,		&TempLen,
						1,				&Category,
						1,				&ActCode,
						END_OF_ARGS);

	*pFrameLen = *pFrameLen + TempLen;

	return;
}


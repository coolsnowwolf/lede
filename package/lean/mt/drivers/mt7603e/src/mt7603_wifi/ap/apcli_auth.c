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
	apcli_auth.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		-------------------------------------------
*/

#ifdef APCLI_SUPPORT

#include "rt_config.h"

static VOID ApCliAuthTimeout(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3);

DECLARE_TIMER_FUNCTION(ApCliAuthTimeout);
BUILD_TIMER_FUNCTION(ApCliAuthTimeout);

#ifdef MAC_REPEATER_SUPPORT
static VOID ApCliAuthTimeoutExt(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3);

DECLARE_TIMER_FUNCTION(ApCliAuthTimeoutExt);
BUILD_TIMER_FUNCTION(ApCliAuthTimeoutExt);
#endif /* MAC_REPEATER_SUPPORT */



/*
	==========================================================================
	Description:
		function to be executed at timer thread when auth timer expires
	==========================================================================
 */
static VOID ApCliAuthTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)FunctionContext;

	DBGPRINT(RT_DEBUG_TRACE, ("%s():AuthTimeout\n", __FUNCTION__));

	MlmeEnqueue(pAd, APCLI_AUTH_STATE_MACHINE, APCLI_MT2_AUTH_TIMEOUT, 0, NULL, 0);
	RTMP_MLME_HANDLER(pAd);

	return;
}

#ifdef MAC_REPEATER_SUPPORT
static VOID ApCliAuthTimeoutExt(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3)
{
	PREPEATER_CLIENT_ENTRY pRepeaterCliEntry = (PREPEATER_CLIENT_ENTRY)FunctionContext;
	PRTMP_ADAPTER pAd;
	USHORT ifIndex = 0;

	DBGPRINT(RT_DEBUG_TRACE, ("Repeater Cli AUTH - AuthTimeout\n"));

	pAd = pRepeaterCliEntry->pAd;
	ifIndex = (64 + (16*pRepeaterCliEntry->MatchApCliIdx) + pRepeaterCliEntry->MatchLinkIdx);

	DBGPRINT(RT_DEBUG_ERROR, (" (%s) ifIndex = %d, CliIdx = %d !!!\n",
				__FUNCTION__, pRepeaterCliEntry->MatchApCliIdx, pRepeaterCliEntry->MatchLinkIdx));


	MlmeEnqueue(pAd, APCLI_AUTH_STATE_MACHINE, APCLI_MT2_AUTH_TIMEOUT, 0, NULL, ifIndex);
	RTMP_MLME_HANDLER(pAd);

	return;
}
#endif /* MAC_REPEATER_SUPPORT */

/*
	==========================================================================
	Description:
	==========================================================================
 */
static VOID ApCliMlmeAuthReqAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem)
{
	BOOLEAN             Cancelled;
	NDIS_STATUS         NState;
	UCHAR               Addr[MAC_ADDR_LEN];
	USHORT              Alg, Seq, Status;
	ULONG               Timeout;
	HEADER_802_11       AuthHdr; 
	PUCHAR              pOutBuffer = NULL;
	ULONG               FrameLen = 0;
	APCLI_CTRL_MSG_STRUCT ApCliCtrlMsg;
	USHORT ifIndex = (USHORT)(Elem->Priv);
	PULONG pCurrState = NULL;
#ifdef MAC_REPEATER_SUPPORT
	UCHAR CliIdx = 0xFF;
#endif /* MAC_REPEATER_SUPPORT */

#ifdef WH_EZ_SETUP
		struct wifi_dev *wdev;
	
		if(IS_ADPTR_EZ_SETUP_ENABLED(pAd))
			EZ_DEBUG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ApCliMlmeAuthReqAction()\n"));
#endif /* WH_EZ_SETUP */

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
		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].AuthCurrState;
	}
	else
#endif /* MAC_REPEATER_SUPPORT */
		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].AuthCurrState;

	/* Block all authentication request durning WPA block period */
	if (pAd->ApCfg.ApCliTab[ifIndex].bBlockAssoc == TRUE)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("APCLI AUTH - Block Auth request durning WPA block period!\n"));
		*pCurrState = APCLI_AUTH_REQ_IDLE;
		ApCliCtrlMsg.Status = MLME_STATE_MACHINE_REJECT;
#ifdef MAC_REPEATER_SUPPORT
		ApCliCtrlMsg.BssIdx = ifIndex;
		ApCliCtrlMsg.CliIdx = CliIdx;
		ifIndex = (USHORT)(Elem->Priv);
#endif /* MAC_REPEATER_SUPPORT */
		MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_AUTH_RSP,
			sizeof(APCLI_CTRL_MSG_STRUCT), &ApCliCtrlMsg, ifIndex);
	}
	else if(MlmeAuthReqSanity(pAd, Elem->Msg, Elem->MsgLen, Addr, &Timeout, &Alg))
	{
#ifdef MAC_REPEATER_SUPPORT
		/* reset timer */
		if (CliIdx != 0xFF)
			RTMPCancelTimer(&pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].ApCliAuthTimer, &Cancelled);
		else
#endif /* MAC_REPEATER_SUPPORT */
		RTMPCancelTimer(&pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.ApCliAuthTimer, &Cancelled);

		pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.Alg  = Alg;

		Seq = 1;
		Status = MLME_SUCCESS;

#ifdef WH_EZ_SETUP
		wdev = &pAd->ApCfg.ApCliTab[ifIndex].wdev;
		if (IS_EZ_SETUP_ENABLED(wdev)
			&& (Alg == AUTH_MODE_EZ)) {
			Alg = cpu2le16(Alg);
		}
#endif /* WH_EZ_SETUP */

		/* allocate and send out AuthReq frame */
		NState = MlmeAllocateMemory(pAd, &pOutBuffer);
		if(NState != NDIS_STATUS_SUCCESS)
		{
			DBGPRINT(RT_DEBUG_ERROR, ("APCLI AUTH - MlmeAuthReqAction() allocate memory failed\n"));
			*pCurrState = APCLI_AUTH_REQ_IDLE;

			ApCliCtrlMsg.Status = MLME_FAIL_NO_RESOURCE;
#ifdef MAC_REPEATER_SUPPORT
			ApCliCtrlMsg.BssIdx = ifIndex;
			ApCliCtrlMsg.CliIdx = CliIdx;
			ifIndex = (USHORT)(Elem->Priv);
#endif /* MAC_REPEATER_SUPPORT */
			MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_AUTH_RSP,
				sizeof(APCLI_CTRL_MSG_STRUCT), &ApCliCtrlMsg, ifIndex);
			return;
		}

		DBGPRINT(RT_DEBUG_TRACE, ("APCLI AUTH - Send AUTH request seq#1 (Alg=%d)...\n", Alg));
		ApCliMgtMacHeaderInit(pAd, &AuthHdr, SUBTYPE_AUTH, 0, Addr, pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.Bssid, ifIndex);
#ifdef MAC_REPEATER_SUPPORT
		if (CliIdx != 0xFF)
			COPY_MAC_ADDR(AuthHdr.Addr2, pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].CurrentAddress);
#endif /* MAC_REPEATER_SUPPORT */

		MakeOutgoingFrame(pOutBuffer,           &FrameLen, 
						  sizeof(HEADER_802_11),&AuthHdr, 
						  2,                    &Alg, 
						  2,                    &Seq, 
						  2,                    &Status, 
						  END_OF_ARGS);

#ifdef WH_EZ_SETUP
		if (IS_EZ_SETUP_ENABLED(wdev)
			&& (pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.support_easy_setup)) {
			FrameLen += ez_build_auth_request_ie(wdev, 
				pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.Bssid, pOutBuffer+FrameLen);
		}
#endif /* WH_EZ_SETUP */

		MiniportMMRequest(pAd, QID_AC_BE, pOutBuffer, FrameLen);
		MlmeFreeMemory(pAd, pOutBuffer);

#ifdef MAC_REPEATER_SUPPORT
		if (CliIdx != 0xFF)
			RTMPSetTimer(&pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].ApCliAuthTimer, AUTH_TIMEOUT);
		else
#endif /* MAC_REPEATER_SUPPORT */
		RTMPSetTimer(&pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.ApCliAuthTimer, AUTH_TIMEOUT);
		*pCurrState = APCLI_AUTH_WAIT_SEQ2;
	} 
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("APCLI AUTH - MlmeAuthReqAction() sanity check failed. BUG!!!!!\n"));
		*pCurrState = APCLI_AUTH_REQ_IDLE;
	}

	return;
}

/*
	==========================================================================
	Description:
	==========================================================================
 */
static VOID ApCliPeerAuthRspAtSeq2Action(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM * Elem)
{
	BOOLEAN         Cancelled;
	UCHAR           Addr2[MAC_ADDR_LEN];
	USHORT          Seq, Status, Alg;
	USHORT          RemoteStatus;
	UCHAR			iv_hdr[LEN_WEP_IV_HDR];
/*	UCHAR           ChlgText[CIPHER_TEXT_LEN]; */
	UCHAR           *ChlgText = NULL;
	UCHAR           CyperChlgText[CIPHER_TEXT_LEN + 8 + 8];
	ULONG			c_len = 0;	
	HEADER_802_11   AuthHdr;
	NDIS_STATUS     NState;
	PUCHAR          pOutBuffer = NULL;
	ULONG           FrameLen = 0;
	APCLI_CTRL_MSG_STRUCT ApCliCtrlMsg;
	UCHAR		  	ChallengeIe = IE_CHALLENGE_TEXT;
	UCHAR		  	len_challengeText = CIPHER_TEXT_LEN;
	USHORT ifIndex = (USHORT)(Elem->Priv);
	PULONG pCurrState = NULL;
	APCLI_STRUCT *apcli_entry;
#ifdef MAC_REPEATER_SUPPORT
	UCHAR CliIdx = 0xFF;
#endif /* MAC_REPEATER_SUPPORT */

#ifdef WH_EZ_SETUP
		if(IS_ADPTR_EZ_SETUP_ENABLED(pAd))
			EZ_DEBUG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_OFF, ("ApCliPeerAuthRspAtSeq2Action\n"));
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
		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].AuthCurrState;
	}
	else
#endif /* MAC_REPEATER_SUPPORT */
		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].AuthCurrState;

	apcli_entry = &pAd->ApCfg.ApCliTab[ifIndex];

	/* allocate memory */
	os_alloc_mem(NULL, (UCHAR **)&ChlgText, CIPHER_TEXT_LEN);
	if (ChlgText == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s: Allocate memory fail!!!\n", __FUNCTION__));
		return;
	}

	if(PeerAuthSanity(pAd, Elem->Msg, Elem->MsgLen, Addr2, &Alg, &Seq, &Status, (CHAR *) ChlgText))
	{
		if(MAC_ADDR_EQUAL(apcli_entry->MlmeAux.Bssid, Addr2) && Seq == 2)
		{
#ifdef MAC_REPEATER_SUPPORT
			if (CliIdx != 0xFF)
			{
				DBGPRINT(RT_DEBUG_TRACE, ("AUTH - Repeater Cli Receive AUTH_RSP seq#2 to me (Alg=%d, Status=%d)\n", Alg, Status));
				RTMPCancelTimer(&apcli_entry->RepeaterCli[CliIdx].ApCliAuthTimer, &Cancelled);
			}
			else
#endif /* MAC_REPEATER_SUPPORT */
			{
				DBGPRINT(RT_DEBUG_TRACE, ("APCLI AUTH - Receive AUTH_RSP seq#2 to me (Alg=%d, Status=%d)\n", Alg, Status));
				RTMPCancelTimer(&apcli_entry->MlmeAux.ApCliAuthTimer, &Cancelled);
			}

			if(Status == MLME_SUCCESS)
			{
#ifdef WH_EZ_SETUP			
				if (IS_EZ_SETUP_ENABLED(&apcli_entry->wdev)
					&& (Alg == AUTH_MODE_EZ)) {
					ez_process_auth_response(pAd, 
							&apcli_entry->wdev,
							Addr2,
							pCurrState, 
							Elem->Msg, 
							Elem->MsgLen);
					os_free_mem(NULL, ChlgText);
					return;
				}
#endif /* WH_EZ_SETUP */
			
				if(apcli_entry->MlmeAux.Alg == Ndis802_11AuthModeOpen)
				{
					*pCurrState = APCLI_AUTH_REQ_IDLE;

					ApCliCtrlMsg.Status= MLME_SUCCESS;
#ifdef MAC_REPEATER_SUPPORT
					ApCliCtrlMsg.CliIdx = CliIdx;
					ApCliCtrlMsg.BssIdx = ifIndex;
					ifIndex = (USHORT)(Elem->Priv);
#endif /* MAC_REPEATER_SUPPORT */
					MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_AUTH_RSP,
						sizeof(APCLI_CTRL_MSG_STRUCT), &ApCliCtrlMsg, ifIndex);
				} 
				else
				{
					PCIPHER_KEY  pKey;	
					UINT default_key = apcli_entry->wdev.DefaultKeyId;

					pKey = &apcli_entry->SharedKey[default_key];

					/* 2. shared key, need to be challenged */
					Seq++;
					RemoteStatus = MLME_SUCCESS;
					/* allocate and send out AuthRsp frame */
					NState = MlmeAllocateMemory(pAd, &pOutBuffer); 					
					if(NState != NDIS_STATUS_SUCCESS)
					{
						DBGPRINT(RT_DEBUG_TRACE, ("AUTH - ApCliPeerAuthRspAtSeq2Action allocate memory fail\n"));
						*pCurrState = APCLI_AUTH_REQ_IDLE;

						ApCliCtrlMsg.Status= MLME_FAIL_NO_RESOURCE;
#ifdef MAC_REPEATER_SUPPORT
						ApCliCtrlMsg.CliIdx = CliIdx;
						ApCliCtrlMsg.BssIdx = ifIndex;
						ifIndex = (USHORT)(Elem->Priv);
#endif /* MAC_REPEATER_SUPPORT */
						MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_AUTH_RSP,
							sizeof(APCLI_CTRL_MSG_STRUCT), &ApCliCtrlMsg, ifIndex);
						goto LabelOK;
					}

#ifdef MAC_REPEATER_SUPPORT
					if (CliIdx != 0xFF)
						DBGPRINT(RT_DEBUG_TRACE, ("AUTH - Repeater Cli Send AUTH request seq#3...\n"));
					else
#endif /* MAC_REPEATER_SUPPORT */
					DBGPRINT(RT_DEBUG_TRACE, ("AUTH - Send AUTH request seq#3...\n"));
					ApCliMgtMacHeaderInit(pAd, &AuthHdr, SUBTYPE_AUTH, 0, Addr2, apcli_entry->MlmeAux.Bssid, ifIndex);
					AuthHdr.FC.Wep = 1;
#ifdef MAC_REPEATER_SUPPORT
					if (CliIdx != 0xFF)
						COPY_MAC_ADDR(AuthHdr.Addr2, apcli_entry->RepeaterCli[CliIdx].CurrentAddress);
#endif /* MAC_REPEATER_SUPPORT */
					
					/* Encrypt challenge text & auth information */
					/* TSC increment */ 
					INC_TX_TSC(pKey->TxTsc, LEN_WEP_TSC);

					/* Construct the 4-bytes WEP IV header */
					RTMPConstructWEPIVHdr(default_key, pKey->TxTsc, iv_hdr);
									 
					Alg = cpu2le16(*(USHORT *)&Alg);
					Seq = cpu2le16(*(USHORT *)&Seq);
					RemoteStatus= cpu2le16(*(USHORT *)&RemoteStatus);                    				

					/* Construct message text */
					MakeOutgoingFrame(CyperChlgText,        &c_len, 
							          2,                    &Alg, 
							          2,                    &Seq,
							          2,                    &RemoteStatus,  
							          1,					&ChallengeIe, 
							          1,					&len_challengeText,
							          len_challengeText,	ChlgText,
							          END_OF_ARGS);

					if (RTMPSoftEncryptWEP(pAd, 
										   iv_hdr, 
										   pKey,
										   CyperChlgText, 
										   c_len) == FALSE)
					{
						DBGPRINT(RT_DEBUG_TRACE, ("AUTH - ApCliPeerAuthRspAtSeq2Action allocate memory fail\n"));
						*pCurrState = APCLI_AUTH_REQ_IDLE;

						ApCliCtrlMsg.Status= MLME_FAIL_NO_RESOURCE;
#ifdef MAC_REPEATER_SUPPORT
						ApCliCtrlMsg.BssIdx = ifIndex;
						ApCliCtrlMsg.CliIdx = CliIdx;
						ifIndex = (USHORT)(Elem->Priv);
#endif /* MAC_REPEATER_SUPPORT */
						MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_AUTH_RSP,
									sizeof(APCLI_CTRL_MSG_STRUCT), &ApCliCtrlMsg, ifIndex);
						goto LabelOK;
					}

					/* Update the total length for 4-bytes ICV */
					c_len += LEN_ICV;

					MakeOutgoingFrame(pOutBuffer,               &FrameLen, 
									  sizeof(HEADER_802_11),    &AuthHdr,  
							          LEN_WEP_IV_HDR,			iv_hdr,								          
							          c_len,     				CyperChlgText, 
									  END_OF_ARGS);

					MiniportMMRequest(pAd, QID_AC_BE, pOutBuffer, FrameLen);
				
#ifdef MAC_REPEATER_SUPPORT
					if (CliIdx != 0xFF)
						RTMPSetTimer(&apcli_entry->RepeaterCli[CliIdx].ApCliAuthTimer, AUTH_TIMEOUT);
					else
#endif /* MAC_REPEATER_SUPPORT */
					RTMPSetTimer(&apcli_entry->MlmeAux.ApCliAuthTimer, AUTH_TIMEOUT);
					*pCurrState = APCLI_AUTH_WAIT_SEQ4;
				}
			} 
			else
			{
				*pCurrState = APCLI_AUTH_REQ_IDLE;

#ifdef MAC_REPEATER_SUPPORT
				ApCliCtrlMsg.CliIdx = CliIdx;
				ApCliCtrlMsg.BssIdx = ifIndex;
				ifIndex = (USHORT)(Elem->Priv);
#endif /* MAC_REPEATER_SUPPORT */
				ApCliCtrlMsg.Status= Status;
				MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_AUTH_RSP,
					sizeof(APCLI_CTRL_MSG_STRUCT), &ApCliCtrlMsg, ifIndex);
			}
		}
	} 
	else
	{
		DBGPRINT(RT_DEBUG_TRACE, ("APCLI AUTH - PeerAuthSanity() sanity check fail\n"));
	}

LabelOK:
	if (pOutBuffer != NULL)
		MlmeFreeMemory(pAd, pOutBuffer);
	if (ChlgText != NULL)
		os_free_mem(NULL, ChlgText);
	return;
}

/*
	==========================================================================
	Description:
	==========================================================================
 */
static VOID ApCliPeerAuthRspAtSeq4Action(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	BOOLEAN     Cancelled;
	UCHAR       Addr2[MAC_ADDR_LEN];
	USHORT      Alg, Seq, Status;
	CHAR        ChlgText[CIPHER_TEXT_LEN];
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
		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].AuthCurrState;
	}
	else
#endif /* MAC_REPEATER_SUPPORT */
		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].AuthCurrState;

	if(PeerAuthSanity(pAd, Elem->Msg, Elem->MsgLen, Addr2, &Alg, &Seq, &Status, ChlgText))
	{
		if(MAC_ADDR_EQUAL(pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.Bssid, Addr2) && Seq == 4)
		{
#ifdef MAC_REPEATER_SUPPORT
			if (CliIdx != 0xFF)
			{
				DBGPRINT(RT_DEBUG_TRACE, ("AUTH - Repeater Cli Receive AUTH_RSP seq#4 to me\n"));
				RTMPCancelTimer(&pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].ApCliAuthTimer, &Cancelled);
			}
			else
#endif /* MAC_REPEATER_SUPPORT */
			{
				DBGPRINT(RT_DEBUG_TRACE, ("APCLI AUTH - Receive AUTH_RSP seq#4 to me\n"));
				RTMPCancelTimer(&pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.ApCliAuthTimer, &Cancelled);
			}

			ApCliCtrlMsg.Status = MLME_SUCCESS;

			if(Status != MLME_SUCCESS)
			{
				ApCliCtrlMsg.Status = Status;
			}

#ifdef MAC_REPEATER_SUPPORT
			ApCliCtrlMsg.CliIdx = CliIdx;
			ApCliCtrlMsg.BssIdx = ifIndex;
			ifIndex = (USHORT)(Elem->Priv);
#endif /* MAC_REPEATER_SUPPORT */

			*pCurrState = APCLI_AUTH_REQ_IDLE;
			MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_AUTH_RSP,
			sizeof(APCLI_CTRL_MSG_STRUCT), &ApCliCtrlMsg, ifIndex);
		}
	} else
	{
		DBGPRINT(RT_DEBUG_TRACE, ("APCLI - PeerAuthRspAtSeq4Action() sanity check fail\n"));
	}

	return;
}

/*
    ==========================================================================
    Description:
    ==========================================================================
*/
static VOID ApCliPeerDeauthAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	UCHAR       Addr1[MAC_ADDR_LEN];
	UCHAR       Addr2[MAC_ADDR_LEN];
	UCHAR       Addr3[MAC_ADDR_LEN];
	USHORT      Reason;
	USHORT ifIndex = (USHORT)(Elem->Priv);
	PULONG pCurrState = NULL;
#ifdef MAC_REPEATER_SUPPORT
	UCHAR CliIdx = 0xFF;
#endif /* MAC_REPEATER_SUPPORT */
#ifdef APCLI_SAE_SUPPORT
	PULONG pCtrlCurrState = NULL;
#endif

#ifdef WH_EZ_SETUP
		if(IS_ADPTR_EZ_SETUP_ENABLED(pAd))
			EZ_DEBUG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_OFF, ("ApCliPeerDeauthAction\n"));
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
		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].AuthCurrState;
#ifdef APCLI_SAE_SUPPORT
		pCtrlCurrState = &pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].CtrlCurrState;
#endif

	} else
#endif /* MAC_REPEATER_SUPPORT */
	{
		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].AuthCurrState;
#ifdef APCLI_SAE_SUPPORT
		pCtrlCurrState = &pAd->ApCfg.ApCliTab[ifIndex].CtrlCurrState;
#endif
	}


	if (PeerDeauthSanity(pAd, Elem->Msg, Elem->MsgLen, Addr1, Addr2, Addr3, &Reason))
	{
		DBGPRINT(RT_DEBUG_TRACE, ("APCLI AUTH_RSP - receive DE-AUTH from our AP\n"));
		*pCurrState = APCLI_AUTH_REQ_IDLE;


#ifdef APCLI_OWE_SUPPORT
		apcli_reset_owe_parameters(pAd, ifIndex);
#endif


#if defined(APCLI_SAE_SUPPORT) || defined(APCLI_OWE_SUPPORT)
	if ((*pCtrlCurrState == APCLI_CTRL_ASSOC) || (*pCtrlCurrState == APCLI_CTRL_CONNECTED)) {
		UCHAR if_addr[6];
		INT CachedIdx;
		UCHAR pmkid[80];
		UCHAR pmk[LEN_PMK];

#ifdef MAC_REPEATER_SUPPORT
		if (CliIdx != 0xFF)
			NdisCopyMemory(if_addr,
				&pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].CurrentAddress,
				MAC_ADDR_LEN);
		else
#endif /* MAC_REPEATER_SUPPORT */
			NdisCopyMemory(if_addr, &pAd->ApCfg.ApCliTab[ifIndex].wdev.if_addr, MAC_ADDR_LEN);

		/*Received PMK invalid status from AP delete entry from SavedPMK and delete SAE instance*/
		if (
#ifdef APCLI_SAE_SUPPORT
			(pAd->ApCfg.ApCliTab[ifIndex].wdev.AuthMode == Ndis802_11AuthModeWPA3PSK
			&& sae_get_pmk_cache(&pAd->SaeCfg, if_addr,
			pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.Bssid, pmkid, pmk))
#endif

#ifdef APCLI_OWE_SUPPORT
			|| pAd->ApCfg.ApCliTab[ifIndex].wdev.AuthMode == Ndis802_11AuthModeOWE
#endif
			) {

			CachedIdx = apcli_search_pmkid_cache(pAd,
				pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.Bssid, ifIndex
#ifdef MAC_REPEATER_SUPPORT
				, CliIdx
#endif
				);

			if (CachedIdx != INVALID_PMKID_IDX) {
#ifdef APCLI_SAE_SUPPORT
				SAE_INSTANCE *pSaeIns =
					search_sae_instance(&pAd->SaeCfg, if_addr,
						pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.Bssid);

				MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_OFF,
					("%s:Reconnection falied with pmkid,delete cache entry and sae instance\n",
					__func__));
				if (pSaeIns != NULL)
					delete_sae_instance(pSaeIns);
#endif
				apcli_delete_pmkid_cache(pAd,
					pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.Bssid, ifIndex
#ifdef MAC_REPEATER_SUPPORT
					, CliIdx
#endif
					);
			}
		}

		}
#endif

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
	else
	{
		DBGPRINT(RT_DEBUG_TRACE, ("APCLI AUTH_RSP - ApCliPeerDeauthAction() sanity check fail\n"));
	}

	return;
}

/*
	==========================================================================
	Description:
	==========================================================================
 */
static VOID ApCliAuthTimeoutAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
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
		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].AuthCurrState;
	}
	else
#endif /* MAC_REPEATER_SUPPORT */
		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].AuthCurrState;

	DBGPRINT(RT_DEBUG_TRACE, ("APCLI AUTH - AuthTimeoutAction\n"));

	*pCurrState = APCLI_AUTH_REQ_IDLE;

#ifdef MAC_REPEATER_SUPPORT
	ifIndex = (USHORT)(Elem->Priv);
#endif /* MAC_REPEATER_SUPPORT */

	MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_AUTH_REQ_TIMEOUT, 0, NULL, ifIndex);

	return;
}

/*
	==========================================================================
	Description:
	==========================================================================
 */
static VOID ApCliInvalidStateWhenAuth(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
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
		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].AuthCurrState;
	}
	else
#endif /* MAC_REPEATER_SUPPORT */
		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].AuthCurrState;

	DBGPRINT(RT_DEBUG_TRACE, ("APCLI AUTH - InvalidStateWhenAuth (state=%ld), reset AUTH state machine\n", *pCurrState));

	*pCurrState= APCLI_AUTH_REQ_IDLE;

#ifdef MAC_REPEATER_SUPPORT
	ApCliCtrlMsg.BssIdx = ifIndex;
	ApCliCtrlMsg.CliIdx = CliIdx;
	ifIndex = (USHORT)(Elem->Priv);
#endif /* MAC_REPEATER_SUPPORT */
	ApCliCtrlMsg.Status = MLME_STATE_MACHINE_REJECT;
	MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_AUTH_RSP,
		sizeof(APCLI_CTRL_MSG_STRUCT), &ApCliCtrlMsg, ifIndex);

	return;
}

/*
	==========================================================================
	Description:
	==========================================================================
 */
#ifndef WH_EZ_SETUP
	static
#endif
VOID ApCliMlmeDeauthReqAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem)
{
	PMLME_DEAUTH_REQ_STRUCT pDeauthReq;
	HEADER_802_11 DeauthHdr;
	PUCHAR pOutBuffer = NULL;
	ULONG FrameLen = 0;
	NDIS_STATUS NStatus;
	USHORT ifIndex = (USHORT)(Elem->Priv);
	PULONG pCurrState = NULL;
#ifdef MAC_REPEATER_SUPPORT
	UCHAR CliIdx = 0xFF;
#endif /* MAC_REPEATER_SUPPORT */

	DBGPRINT(RT_DEBUG_TRACE, ("APCLI AUTH - ApCliMlmeAuthReqAction (state=%ld), reset AUTH state machine\n",
		pAd->Mlme.ApCliAuthMachine.CurrState));

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
		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].AuthCurrState;
	}
	else
#endif /* MAC_REPEATER_SUPPORT */
		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].AuthCurrState;

	pDeauthReq = (PMLME_DEAUTH_REQ_STRUCT)(Elem->Msg);

	*pCurrState= APCLI_AUTH_REQ_IDLE;

	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);  /*Get an unused nonpaged memory */
	if (NStatus != NDIS_STATUS_SUCCESS)
		return;
	
	DBGPRINT(RT_DEBUG_TRACE, ("AUTH - Send DE-AUTH request (Reason=%d)...\n", pDeauthReq->Reason));

	ApCliMgtMacHeaderInit(pAd, &DeauthHdr, SUBTYPE_DEAUTH, 0, pDeauthReq->Addr, pDeauthReq->Addr, ifIndex);
#ifdef MAC_REPEATER_SUPPORT
	if (CliIdx != 0xFF)
		COPY_MAC_ADDR(DeauthHdr.Addr2, pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].CurrentAddress);
#endif /* MAC_REPEATER_SUPPORT */

	MakeOutgoingFrame(pOutBuffer,           &FrameLen,
		sizeof(HEADER_802_11),&DeauthHdr,
		2,                    &pDeauthReq->Reason,
		END_OF_ARGS);
	MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
	MlmeFreeMemory(pAd, pOutBuffer);

	return;
}

#ifdef APCLI_SAE_SUPPORT

/*	==========================================================================*/
/*	Description:																						*/
/*																									*/
/*	IRQL = DISPATCH_LEVEL																			*/
/*																									*/
/*	==========================================================================*/

VOID ApCliMlmeSaeAuthReqAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	USHORT ifIndex = (USHORT)(Elem->Priv);
#ifdef MAC_REPEATER_SUPPORT
	UCHAR CliIdx = 0xFF;
	REPEATER_CLIENT_ENTRY *pReptEntry = NULL;
#endif /* MAC_REPEATER_SUPPORT */
	/* SAE_MLME_AUTH_REQ_STRUCT *AuthReq = (SAE_MLME_AUTH_REQ_STRUCT *)Elem->Msg; */
	MLME_AUTH_REQ_STRUCT *AuthReq = (MLME_AUTH_REQ_STRUCT *)Elem->Msg;
	PULONG pCurrState = NULL;
	UCHAR if_addr[MAC_ADDR_LEN];
	APCLI_STRUCT *apcli_entry = NULL;
	UCHAR *pSae_cfg_group = NULL;
	SAE_CFG *pSaeCfg = NULL;
	APCLI_CTRL_MSG_STRUCT ApCliCtrlMsg;


	pSaeCfg = &pAd->SaeCfg;

		if ((ifIndex >= MAX_APCLI_NUM)
#ifdef MAC_REPEATER_SUPPORT
			&& (ifIndex < REPT_MLME_START_IDX)
#endif /* MAC_REPEATER_SUPPORT */
		   )
		return;

	NdisZeroMemory(if_addr, MAC_ADDR_LEN);
#ifdef MAC_REPEATER_SUPPORT
		if (ifIndex >= REPT_MLME_START_IDX) {
			CliIdx = ((ifIndex - REPT_MLME_START_IDX) % 16);
			ifIndex = ((ifIndex - REPT_MLME_START_IDX) / 16);
			pReptEntry = &pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx];
			pSae_cfg_group = &pReptEntry->sae_cfg_group;
			ifIndex = pReptEntry->wdev->func_idx;
			apcli_entry = &pAd->ApCfg.ApCliTab[ifIndex];
			pCurrState = &pReptEntry->AuthCurrState;
			COPY_MAC_ADDR(if_addr, pReptEntry->CurrentAddress);
		} else
#endif /* MAC_REPEATER_SUPPORT */
		{
			apcli_entry = &pAd->ApCfg.ApCliTab[ifIndex];
			pSae_cfg_group = &apcli_entry->sae_cfg_group;
			pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].AuthCurrState;
			COPY_MAC_ADDR(if_addr, pAd->ApCfg.ApCliTab[ifIndex].wdev.if_addr);
		}
	MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_OFF, ("==>%s()\n", __func__));

	if (sae_auth_init(pAd, &pAd->SaeCfg, if_addr, AuthReq->Addr,
			apcli_entry->MlmeAux.Bssid, apcli_entry->PSK, *pSae_cfg_group))
		*pCurrState = APCLI_AUTH_WAIT_SAE;
	else {
		*pCurrState = APCLI_AUTH_REQ_IDLE;
		ApCliCtrlMsg.Status = MLME_UNSPECIFY_FAIL;
#ifdef MAC_REPEATER_SUPPORT
		ApCliCtrlMsg.BssIdx = ifIndex;
		ApCliCtrlMsg.CliIdx = CliIdx;
		ifIndex = (USHORT)(Elem->Priv);
#endif /* MAC_REPEATER_SUPPORT */
		MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_AUTH_RSP,
			sizeof(APCLI_CTRL_MSG_STRUCT), &ApCliCtrlMsg, ifIndex);
		RTMP_MLME_HANDLER(pAd);

	}
}



/*==========================================================================*/
/*	Description:																						*/
/*																									*/
/*	IRQL = DISPATCH_LEVEL																			*/
/*																									*/
/*==========================================================================*/

VOID ApCliMlmeSaeAuthRspAction(RTMP_ADAPTER *pAd, MLME_QUEUE_ELEM *Elem)
{
	FRAME_802_11 *Fr = (FRAME_802_11 *)Elem->Msg;
	USHORT seq;
	USHORT status;
	UCHAR *pmk;
	USHORT ifIndex = (USHORT)(Elem->Priv);
	PULONG pAuthCurrState = NULL;
	APCLI_CTRL_MSG_STRUCT ApCliCtrlMsg;
	APCLI_STRUCT *apcli_entry;
#ifdef MAC_REPEATER_SUPPORT
	REPEATER_CLIENT_ENTRY *pReptEntry = NULL;
#endif
#if defined(MAC_REPEATER_SUPPORT)
	UCHAR CliIdx = 0xFF;
#endif /* MAC_REPEATER_SUPPORT */


	if ((ifIndex >= MAX_APCLI_NUM)
#ifdef MAC_REPEATER_SUPPORT
		&& (ifIndex < REPT_MLME_START_IDX)
#endif /* MAC_REPEATER_SUPPORT */
		)
		return;
#ifdef MAC_REPEATER_SUPPORT
	if (ifIndex >= REPT_MLME_START_IDX) {
		CliIdx = ((ifIndex - REPT_MLME_START_IDX) % 16);
		ifIndex = ((ifIndex - REPT_MLME_START_IDX) / 16);
		pReptEntry = &pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx];
		ifIndex = pReptEntry->wdev->func_idx;
		pAuthCurrState = &pReptEntry->AuthCurrState;
	} else
#endif /* MAC_REPEATER_SUPPORT */
	{
		pAuthCurrState = &pAd->ApCfg.ApCliTab[ifIndex].AuthCurrState;
	}

	apcli_entry = &pAd->ApCfg.ApCliTab[ifIndex];

	NdisMoveMemory(&seq, &Fr->Octet[2], 2);
	NdisMoveMemory(&status, &Fr->Octet[4], 2);


	if (sae_handle_auth(pAd, &pAd->SaeCfg, Elem->Msg, Elem->MsgLen,
			apcli_entry->PSK,
			seq, status, &pmk) == FALSE){

		*pAuthCurrState = APCLI_AUTH_REQ_IDLE;
	/*If SAE instance has been deleted*/
		ApCliCtrlMsg.Status = MLME_UNSPECIFY_FAIL;
#ifdef MAC_REPEATER_SUPPORT
		ApCliCtrlMsg.CliIdx = CliIdx;
		ApCliCtrlMsg.BssIdx = ifIndex;
		ifIndex = (USHORT)(Elem->Priv);
#endif /* MAC_REPEATER_SUPPORT */

		MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_AUTH_RSP,
					sizeof(APCLI_CTRL_MSG_STRUCT), &ApCliCtrlMsg, ifIndex);
		RTMP_MLME_HANDLER(pAd);

		} else if (pmk != NULL) {
		USHORT Status;
		MAC_TABLE_ENTRY *pEntry = NULL;

		pEntry = &apcli_entry->pre_mac_entry;

		DebugLevel = DBG_LVL_TRACE;
		hex_dump("pmk:", (char *)pmk, LEN_PMK);
		DebugLevel = DBG_LVL_ERROR;

		if (pEntry) {
			NdisMoveMemory(pEntry->PMK, pmk, LEN_PMK);
			Status = MLME_SUCCESS;
			MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_OFF,
				("%s(): Security AKM = 0x%x, PairwiseCipher = 0x%x, GroupCipher = 0x%x\n",
				__func__, pEntry->AuthMode, pEntry->WepStatus, pEntry->GroupKeyWepStatus));
		} else
			Status = MLME_UNSPECIFY_FAIL;
		*pAuthCurrState = APCLI_AUTH_REQ_IDLE;
		ApCliCtrlMsg.Status = Status;
#ifdef MAC_REPEATER_SUPPORT
		ApCliCtrlMsg.CliIdx = CliIdx;
		ApCliCtrlMsg.BssIdx = ifIndex;
		ifIndex = (USHORT)(Elem->Priv);
#endif /* MAC_REPEATER_SUPPORT */
		MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_AUTH_RSP,
					sizeof(APCLI_CTRL_MSG_STRUCT), &ApCliCtrlMsg, ifIndex);
		RTMP_MLME_HANDLER(pAd);
	}
}
#endif /* APCLI_SAE_SUPPORT */



/*
	==========================================================================
	Description:
		authenticate state machine init, including state transition and timer init
	Parameters:
		Sm - pointer to the auth state machine
	Note:
		The state machine looks like this
	==========================================================================
 */
VOID ApCliAuthStateMachineInit(
	IN RTMP_ADAPTER *pAd,
	IN STATE_MACHINE *Sm,
	OUT STATE_MACHINE_FUNC Trans[])
{
	UCHAR i;
#ifdef MAC_REPEATER_SUPPORT
	UCHAR j;
#endif /* MAC_REPEATER_SUPPORT */

	StateMachineInit(Sm, (STATE_MACHINE_FUNC*)Trans,
		APCLI_MAX_AUTH_STATE, APCLI_MAX_AUTH_MSG,
		(STATE_MACHINE_FUNC)Drop, APCLI_AUTH_REQ_IDLE,
		APCLI_AUTH_MACHINE_BASE);

	/* the first column */
	StateMachineSetAction(Sm, APCLI_AUTH_REQ_IDLE, APCLI_MT2_MLME_AUTH_REQ, (STATE_MACHINE_FUNC)ApCliMlmeAuthReqAction);
	StateMachineSetAction(Sm, APCLI_AUTH_REQ_IDLE, APCLI_MT2_PEER_DEAUTH, (STATE_MACHINE_FUNC)ApCliPeerDeauthAction);
	StateMachineSetAction(Sm, APCLI_AUTH_REQ_IDLE, APCLI_MT2_MLME_DEAUTH_REQ, (STATE_MACHINE_FUNC)ApCliMlmeDeauthReqAction);

	/* the second column */
	StateMachineSetAction(Sm, APCLI_AUTH_WAIT_SEQ2, APCLI_MT2_MLME_AUTH_REQ, (STATE_MACHINE_FUNC)ApCliInvalidStateWhenAuth);
	StateMachineSetAction(Sm, APCLI_AUTH_WAIT_SEQ2, APCLI_MT2_PEER_AUTH_EVEN, (STATE_MACHINE_FUNC)ApCliPeerAuthRspAtSeq2Action);
	StateMachineSetAction(Sm, APCLI_AUTH_WAIT_SEQ2, APCLI_MT2_PEER_DEAUTH, (STATE_MACHINE_FUNC)ApCliPeerDeauthAction);
	StateMachineSetAction(Sm, APCLI_AUTH_WAIT_SEQ2, APCLI_MT2_AUTH_TIMEOUT, (STATE_MACHINE_FUNC)ApCliAuthTimeoutAction);
	StateMachineSetAction(Sm, APCLI_AUTH_WAIT_SEQ2, APCLI_MT2_MLME_DEAUTH_REQ, (STATE_MACHINE_FUNC)ApCliMlmeDeauthReqAction);

	/* the third column */
	StateMachineSetAction(Sm, APCLI_AUTH_WAIT_SEQ4, APCLI_MT2_MLME_AUTH_REQ, (STATE_MACHINE_FUNC)ApCliInvalidStateWhenAuth);
	StateMachineSetAction(Sm, APCLI_AUTH_WAIT_SEQ4, APCLI_MT2_PEER_AUTH_EVEN, (STATE_MACHINE_FUNC)ApCliPeerAuthRspAtSeq4Action);
	StateMachineSetAction(Sm, APCLI_AUTH_WAIT_SEQ4, APCLI_MT2_PEER_DEAUTH, (STATE_MACHINE_FUNC)ApCliPeerDeauthAction);
	StateMachineSetAction(Sm, APCLI_AUTH_WAIT_SEQ4, APCLI_MT2_AUTH_TIMEOUT, (STATE_MACHINE_FUNC)ApCliAuthTimeoutAction);
	StateMachineSetAction(Sm, APCLI_AUTH_WAIT_SEQ4, APCLI_MT2_MLME_DEAUTH_REQ, (STATE_MACHINE_FUNC)ApCliMlmeDeauthReqAction);

#ifdef APCLI_SAE_SUPPORT
	StateMachineSetAction(Sm, APCLI_AUTH_REQ_IDLE, APCLI_MT2_MLME_SAE_AUTH_REQ,
						  (STATE_MACHINE_FUNC) ApCliMlmeSaeAuthReqAction);
	StateMachineSetAction(Sm, APCLI_AUTH_WAIT_SAE, APCLI_MT2_MLME_SAE_AUTH_COMMIT,
						  (STATE_MACHINE_FUNC) ApCliMlmeSaeAuthRspAction);
	StateMachineSetAction(Sm, APCLI_AUTH_WAIT_SAE, APCLI_MT2_MLME_SAE_AUTH_CONFIRM,
						  (STATE_MACHINE_FUNC) ApCliMlmeSaeAuthRspAction);
#endif /*APCLI_SAE_SUPPORT */


	for (i=0; i < MAX_APCLI_NUM; i++)
	{
		pAd->ApCfg.ApCliTab[i].AuthCurrState = APCLI_AUTH_REQ_IDLE;

		/* timer init */
		RTMPInitTimer(pAd, &pAd->ApCfg.ApCliTab[i].MlmeAux.ApCliAuthTimer, GET_TIMER_FUNCTION(ApCliAuthTimeout), pAd, FALSE);

#ifdef MAC_REPEATER_SUPPORT
		for (j = 0; j < MAX_EXT_MAC_ADDR_SIZE; j++)
		{
			pAd->ApCfg.ApCliTab[i].RepeaterCli[j].pAd = pAd;
			pAd->ApCfg.ApCliTab[i].RepeaterCli[j].MatchApCliIdx = i;
			pAd->ApCfg.ApCliTab[i].RepeaterCli[j].MatchLinkIdx = j;
			pAd->ApCfg.ApCliTab[i].RepeaterCli[j].AuthCurrState = APCLI_AUTH_REQ_IDLE;

			/* timer init */
			RTMPInitTimer(pAd, &pAd->ApCfg.ApCliTab[i].RepeaterCli[j].ApCliAuthTimer,
							GET_TIMER_FUNCTION(ApCliAuthTimeoutExt), &pAd->ApCfg.ApCliTab[i].RepeaterCli[j], FALSE);

		}
#endif /* MAC_REPEATER_SUPPORT */

	}

	return;
}

#endif /* APCLI_SUPPORT */


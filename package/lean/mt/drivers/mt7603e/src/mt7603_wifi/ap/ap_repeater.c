/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2012, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

    Module Name:
    ap_repeater.c

    Abstract:
    Support MAC Repeater function.

    Revision History:
    Who             		When              What
    --------------  ----------      ----------------------------------------------
    Arvin				11-16-2012      created
*/

#ifdef MAC_REPEATER_SUPPORT

#include "rt_config.h"


REPEATER_CLIENT_ENTRY *RTMPLookupRepeaterCliEntry(
	IN PVOID pData,
	IN BOOLEAN bRealMAC,
	IN PUCHAR pAddr,
	IN BOOLEAN bIsPad,
	OUT PUCHAR pIsLinkValid)
{
	ULONG HashIdx;
	UCHAR tempMAC[6];
	REPEATER_CLIENT_ENTRY *pEntry = NULL;
	REPEATER_CLIENT_ENTRY_MAP *pMapEntry = NULL;
	*pIsLinkValid  = TRUE;

	if (bIsPad == TRUE) {
		NdisAcquireSpinLock(&((PRTMP_ADAPTER)pData)->ApCfg.ReptCliEntryLock);
	} else {
		NdisAcquireSpinLock(((REPEATER_ADAPTER_DATA_TABLE *)pData)->EntryLock);
	}
	
	COPY_MAC_ADDR(tempMAC, pAddr);
	HashIdx = MAC_ADDR_HASH_INDEX(tempMAC);

	if (bRealMAC == TRUE)
	{
		if (bIsPad == TRUE) {
			pMapEntry = ((PRTMP_ADAPTER)pData)->ApCfg.ReptMapHash[HashIdx];
		} else
			pMapEntry = (REPEATER_CLIENT_ENTRY_MAP *)(*((((REPEATER_ADAPTER_DATA_TABLE *)pData)->MapHash)
					+ HashIdx));

		while (pMapEntry)
		{
			pEntry = pMapEntry->pReptCliEntry;

			if (pEntry && MAC_ADDR_EQUAL(pEntry->OriginalAddress, tempMAC))
			{
				if (pEntry->CliValid == FALSE) {
					*pIsLinkValid = FALSE;
					pEntry = NULL;
				}
				
				break;			
			}
			else
			{
				pEntry = NULL;
				pMapEntry = pMapEntry->pNext;
			}
		}
	}
	else
	{
		if (bIsPad == TRUE) {
			pEntry = ((PRTMP_ADAPTER)pData)->ApCfg.ReptCliHash[HashIdx];
		} else {
			pEntry = (REPEATER_CLIENT_ENTRY *)(*((((REPEATER_ADAPTER_DATA_TABLE*)pData)->CliHash)
					+ HashIdx));
		}


		while (pEntry)
		{
			if (MAC_ADDR_EQUAL(pEntry->CurrentAddress, tempMAC))
			{
				if (pEntry->CliValid == FALSE) {
					*pIsLinkValid = FALSE;
					pEntry = NULL;
				}
				break;
			}
			else
				pEntry = pEntry->pNext;
		}
	}

	if (bIsPad == TRUE) {
		NdisReleaseSpinLock(&((PRTMP_ADAPTER)pData)->ApCfg.ReptCliEntryLock);
	} else {
		NdisReleaseSpinLock(((REPEATER_ADAPTER_DATA_TABLE *)pData)->EntryLock);
	}

	return pEntry;
}

BOOLEAN RTMPQueryLookupRepeaterCliEntryMT(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pAddr)
{
	UCHAR isLinkValid;

	DBGPRINT(RT_DEBUG_INFO, ("%s:: %02x:%02x:%02x:%02x:%02x:%02x\n", 
							__FUNCTION__,
							pAddr[0],
							pAddr[1],
							pAddr[2], 
							pAddr[3],
							pAddr[4],
							pAddr[5]));

	if (RTMPLookupRepeaterCliEntry(pAd, FALSE, pAddr, TRUE, &isLinkValid) == NULL) {
		DBGPRINT(RT_DEBUG_INFO, ("%s:: not the repeater client\n", __FUNCTION__));
		return FALSE;
	} else {
		DBGPRINT(RT_DEBUG_INFO, ("%s:: is the repeater client\n", __FUNCTION__));
		return TRUE;
	}
}

#if defined (CONFIG_WIFI_PKT_FWD)
EXPORT_SYMBOL(RTMPQueryLookupRepeaterCliEntryMT);
#endif /* CONFIG_WIFI_PKT_FWD */


VOID RTMPInsertRepeaterEntry(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR apidx,
	IN PUCHAR pAddr)
{
	INT CliIdx, idx, validIdx;
	UCHAR HashIdx;
	BOOLEAN Cancelled;
	UCHAR tempMAC[MAC_ADDR_LEN];
	APCLI_CTRL_MSG_STRUCT ApCliCtrlMsg;
	PREPEATER_CLIENT_ENTRY pReptCliEntry = NULL, pCurrEntry = NULL;
	PREPEATER_CLIENT_ENTRY_MAP pReptCliMap;
	UCHAR SPEC_ADDR[6][3] = {{0x02, 0x0F, 0xB5}, {0x02, 0x09, 0x5B},
								{0x02, 0x14, 0x6C}, {0x02, 0x18, 0x4D},
								{0x02, 0x1B, 0x2F}, {0x02, 0x1E, 0x2A}};

 	MAC_TABLE_ENTRY *pMacEntry = NULL; 	
	STA_TR_ENTRY *tr_entry = NULL;

	DBGPRINT(RT_DEBUG_TRACE, (" %s.\n", __FUNCTION__));


	pMacEntry = MacTableLookup(pAd, pAddr);

 	if (pMacEntry && IS_ENTRY_CLIENT(pMacEntry)) 	
	{

		tr_entry = &pAd->MacTab.tr_entry[pMacEntry->wcid];
		if (tr_entry && tr_entry->PortSecured == WPA_802_1X_PORT_NOT_SECURED)
		{
			DBGPRINT(RT_DEBUG_ERROR, (" wireless client is not ready !!!\n"));
			return ;
		} 	
	}
	
	NdisAcquireSpinLock(&pAd->ApCfg.ReptCliEntryLock);

	if (pAd->ApCfg.RepeaterCliSize >= MAX_EXT_MAC_ADDR_SIZE)
	{
		DBGPRINT(RT_DEBUG_ERROR, (" Repeater Client Full !!!\n"));
		NdisReleaseSpinLock(&pAd->ApCfg.ReptCliEntryLock);
		return ;
	}

	if (pAd->ApCfg.ApCliTab[apidx].Enable == FALSE)
	{
		DBGPRINT(RT_DEBUG_ERROR, (" ApCli Interface is Down !!!\n"));
		NdisReleaseSpinLock(&pAd->ApCfg.ReptCliEntryLock);
		return ;
	}

	validIdx = -1;
	for (CliIdx = 0; CliIdx < MAX_EXT_MAC_ADDR_SIZE; CliIdx++)
	{
		pReptCliEntry = &pAd->ApCfg.ApCliTab[apidx].RepeaterCli[CliIdx];

		if ((pReptCliEntry->CliEnable) && 
			(MAC_ADDR_EQUAL(pReptCliEntry->OriginalAddress, pAddr) || MAC_ADDR_EQUAL(pReptCliEntry->CurrentAddress, pAddr)))
		{
			DBGPRINT(RT_DEBUG_WARN, ("\n  receive mac :%02x:%02x:%02x:%02x:%02x:%02x !!!\n",
						pAddr[0], pAddr[1], pAddr[2], pAddr[3], pAddr[4], pAddr[5]));
			DBGPRINT(RT_DEBUG_WARN, (" duplicate Insert !!!\n"));
			NdisReleaseSpinLock(&pAd->ApCfg.ReptCliEntryLock);
			return ;
		}

		if (pReptCliEntry->CliEnable == FALSE && validIdx == -1)
			validIdx = CliIdx;
	}

	if (validIdx == -1) {
		DBGPRINT(RT_DEBUG_ERROR, (" Repeater Client Full !!!\n"));
		NdisReleaseSpinLock(&pAd->ApCfg.ReptCliEntryLock);
		return ;
	}

	pReptCliEntry = &pAd->ApCfg.ApCliTab[apidx].RepeaterCli[validIdx];
	pReptCliMap = &pAd->ApCfg.ApCliTab[apidx].RepeaterCliMap[validIdx];

	/* ENTRY PREEMPTION: initialize the entry */
	RTMPCancelTimer(&pReptCliEntry->ApCliAuthTimer, &Cancelled);
	RTMPCancelTimer(&pReptCliEntry->ApCliAssocTimer, &Cancelled);
	pReptCliEntry->CtrlCurrState = APCLI_CTRL_DISCONNECTED;
	pReptCliEntry->AuthCurrState = APCLI_AUTH_REQ_IDLE;
	pReptCliEntry->AssocCurrState = APCLI_ASSOC_IDLE;
	pReptCliEntry->CliConnectState = 0;
	pReptCliEntry->CliValid = FALSE;
	pReptCliEntry->bEthCli = FALSE;
	pReptCliEntry->MacTabWCID = 0xFF;
	pReptCliEntry->AuthReqCnt = 0;
	pReptCliEntry->AssocReqCnt = 0;
	pReptCliEntry->CliTriggerTime = 0;
	pReptCliEntry->pNext = NULL;
	pReptCliEntry->wdev = &pAd->ApCfg.ApCliTab[apidx].wdev;/*sync from 7615*/
	pReptCliMap->pReptCliEntry = pReptCliEntry;

#ifdef APCLI_SAE_SUPPORT
	pReptCliEntry->sae_cfg_group = pAd->ApCfg.ApCliTab[pReptCliEntry->MatchApCliIdx].sae_cfg_group;
#endif
#ifdef APCLI_OWE_SUPPORT
	pReptCliEntry->curr_owe_group = pAd->ApCfg.ApCliTab[pReptCliEntry->MatchApCliIdx].curr_owe_group;
#endif

#if defined(APCLI_SAE_SUPPORT) || defined(APCLI_OWE_SUPPORT)
	NdisAllocateSpinLock(pAd, &pReptCliEntry->SavedPMK_lock);
#endif

	pReptCliMap->pNext = NULL;

	COPY_MAC_ADDR(pReptCliEntry->OriginalAddress, pAddr);
	COPY_MAC_ADDR(tempMAC, pAddr);

	if (pAd->ApCfg.MACRepeaterOuiMode == 1)
	{
		DBGPRINT(RT_DEBUG_ERROR, (" todo !!!\n"));
	}
	else if (pAd->ApCfg.MACRepeaterOuiMode == 2)
	{
		INT IdxToUse;
		
		for (idx = 0; idx < 6; idx++)
		{
			if (RTMPEqualMemory(SPEC_ADDR[idx], pAddr, 3))
				break;
		}

		/* If there is a matched one, use the next one; otherwise, use the first one. */
		if (idx >= 0  && idx < 5)
			IdxToUse = idx + 1;
		else 
			IdxToUse = 0;
		NdisCopyMemory(tempMAC, SPEC_ADDR[IdxToUse], 3);
	}
	// replace to all real mac
	else if (pAd->ApCfg.MACRepeaterOuiMode == 3)
	{
		INT IdxToUse;
		
		for (idx = 0; idx < pAd->ApCfg.ReptMacList.Num; idx++)
		{
			if ((pAd->ApCfg.ReptMacList.Entry[idx].bSet) && (!pAd->ApCfg.ReptMacList.Entry[idx].bUsed))
				break;
		}

		/* If there is a matched one, use the next one; otherwise, use the first one. */
		if (idx >= 0  && idx < pAd->ApCfg.ReptMacList.Num)
		{
			IdxToUse = idx;
		}
		else
		{
			/* actually the real mac shouldn't be duplicate, this need the system to take care about this! */
			IdxToUse = 0;
		}

		pAd->ApCfg.ReptMacList.Entry[IdxToUse].bUsed = 1;
		
		NdisCopyMemory(tempMAC, &pAd->ApCfg.ReptMacList.Entry[IdxToUse].Addr[0], MAC_ADDR_LEN);
	}
	else
	{
		NdisCopyMemory(tempMAC, pAd->ApCfg.ApCliTab[apidx].wdev.if_addr, 3);
	}

	COPY_MAC_ADDR(pReptCliEntry->CurrentAddress, tempMAC);
	pReptCliEntry->CliEnable = TRUE;
	pReptCliEntry->CliConnectState = 1;
	pReptCliEntry->pNext = NULL;
	NdisGetSystemUpTime(&pReptCliEntry->CliTriggerTime);

	RTMPInsertRepeaterAsicEntry(pAd, validIdx, tempMAC);
		
	HashIdx = MAC_ADDR_HASH_INDEX(tempMAC);
	if (pAd->ApCfg.ReptCliHash[HashIdx] == NULL)
	{
		pAd->ApCfg.ReptCliHash[HashIdx] = pReptCliEntry;
	}
	else
	{
		pCurrEntry = pAd->ApCfg.ReptCliHash[HashIdx];
		while (pCurrEntry->pNext != NULL)
			pCurrEntry = pCurrEntry->pNext;
		pCurrEntry->pNext = pReptCliEntry;
	}

	HashIdx = MAC_ADDR_HASH_INDEX(pReptCliEntry->OriginalAddress);
	if (pAd->ApCfg.ReptMapHash[HashIdx] == NULL)
		pAd->ApCfg.ReptMapHash[HashIdx] = pReptCliMap;
	else
	{
		PREPEATER_CLIENT_ENTRY_MAP pCurrMapEntry;
	
		pCurrMapEntry = pAd->ApCfg.ReptMapHash[HashIdx];

		while (pCurrMapEntry->pNext != NULL)
			pCurrMapEntry = pCurrMapEntry->pNext;
		if (pCurrMapEntry == pReptCliMap)
			DBGPRINT(RT_DEBUG_ERROR, 
				("Wrong!!! pCurrMapEntry == pReptCliMap(%p) in %s %d\n",
				pReptCliMap,__FUNCTION__,__LINE__));		
		else		
			pCurrMapEntry->pNext = pReptCliMap;
	}

	pAd->ApCfg.RepeaterCliSize++;
	NdisReleaseSpinLock(&pAd->ApCfg.ReptCliEntryLock);

	NdisZeroMemory(&ApCliCtrlMsg, sizeof(APCLI_CTRL_MSG_STRUCT));
	ApCliCtrlMsg.Status = MLME_SUCCESS;
	COPY_MAC_ADDR(&ApCliCtrlMsg.SrcAddr[0], tempMAC);
	ApCliCtrlMsg.BssIdx = apidx;
	ApCliCtrlMsg.CliIdx = validIdx;

	MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_MT2_AUTH_REQ,
			sizeof(APCLI_CTRL_MSG_STRUCT), &ApCliCtrlMsg, apidx);
#ifdef MTFWD
	DBGPRINT(RT_DEBUG_OFF, ("Insert MacRep Sta:%pM\n", tempMAC));
	RtmpOSWrielessEventSend(pAd->net_dev,
				RT_WLAN_EVENT_CUSTOM,
				FWD_CMD_ADD_TX_SRC,
				NULL,
				tempMAC,
				MAC_ADDR_LEN);
#endif

}

VOID RTMPRemoveRepeaterEntry(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR func_tb_idx,
	IN UCHAR CliIdx)
{
	USHORT HashIdx;
	REPEATER_CLIENT_ENTRY *pEntry, *pPrevEntry, *pProbeEntry;
	REPEATER_CLIENT_ENTRY_MAP *pMapEntry, *pPrevMapEntry, *pProbeMapEntry;
	BOOLEAN bVaild;

	DBGPRINT(RT_DEBUG_ERROR, (" %s.\n", __FUNCTION__));

	RTMPRemoveRepeaterAsicEntry(pAd, CliIdx);

	NdisAcquireSpinLock(&pAd->ApCfg.ReptCliEntryLock);
	pEntry = &pAd->ApCfg.ApCliTab[func_tb_idx].RepeaterCli[CliIdx];

#if defined(APCLI_SAE_SUPPORT) || defined(APCLI_OWE_SUPPORT)
	NdisFreeSpinLock(&pEntry->SavedPMK_lock);
#endif

	bVaild = TRUE;

	HashIdx = MAC_ADDR_HASH_INDEX(pEntry->CurrentAddress);

	pPrevEntry = NULL;
	pProbeEntry = pAd->ApCfg.ReptCliHash[HashIdx];

	ASSERT(pProbeEntry);

	if (pProbeEntry == NULL)
	{
		bVaild = FALSE;
		goto done;
	}

	if (pProbeEntry != NULL)
	{
		/* update Hash list*/
		do
		{
			if (pProbeEntry == pEntry)
			{
				if (pPrevEntry == NULL)
				{
					pAd->ApCfg.ReptCliHash[HashIdx] = pEntry->pNext;
				}
				else
				{
					pPrevEntry->pNext = pEntry->pNext;
				}
				break;
			}

			pPrevEntry = pProbeEntry;
			pProbeEntry = pProbeEntry->pNext;
		} while (pProbeEntry);
	}

	/* not found !!!*/
	ASSERT(pProbeEntry != NULL);

	if (pProbeEntry == NULL)
	{
		bVaild = FALSE;
		goto done;
	}

	pMapEntry = &pAd->ApCfg.ApCliTab[func_tb_idx].RepeaterCliMap[CliIdx];

	HashIdx = MAC_ADDR_HASH_INDEX(pEntry->OriginalAddress);

	pPrevMapEntry = NULL;
	pProbeMapEntry = pAd->ApCfg.ReptMapHash[HashIdx];
	ASSERT(pProbeMapEntry);
	if (pProbeMapEntry != NULL)
	{
		/* update Hash list*/
		do
		{
			if (pProbeMapEntry == pMapEntry)
			{
				if (pPrevMapEntry == NULL)
				{
					pAd->ApCfg.ReptMapHash[HashIdx] = pMapEntry->pNext;
				}
				else
				{
					pPrevMapEntry->pNext = pMapEntry->pNext;
				}
				pMapEntry->pNext=NULL;
				pMapEntry->pReptCliEntry=NULL;
				break;
			}

			pPrevMapEntry = pProbeMapEntry;
			if (pProbeMapEntry == pProbeMapEntry->pNext)
				DBGPRINT(RT_DEBUG_ERROR,("Wrong!!! pProbeMapEntry == pProbeMapEntry->pNext(%p) in %s %d\n",
					pProbeMapEntry,__FUNCTION__,__LINE__));
			pProbeMapEntry = pProbeMapEntry->pNext;
		} while (pProbeMapEntry);
	}
	/* not found !!!*/
	ASSERT(pProbeMapEntry != NULL);

done:

	pAd->ApCfg.ApCliTab[func_tb_idx].RepeaterCli[CliIdx].CliConnectState = 0;
	NdisZeroMemory(pAd->ApCfg.ApCliTab[func_tb_idx].RepeaterCli[CliIdx].OriginalAddress, MAC_ADDR_LEN);

	if ((bVaild == TRUE) && (pAd->ApCfg.RepeaterCliSize > 0))
	pAd->ApCfg.RepeaterCliSize--;

	/* set the apcli interface be invalid. */
	pAd->ApCfg.ApCliTab[func_tb_idx].RepeaterCli[CliIdx].CliValid = FALSE;
	pAd->ApCfg.ApCliTab[func_tb_idx].RepeaterCli[CliIdx].CliEnable = FALSE;


	NdisReleaseSpinLock(&pAd->ApCfg.ReptCliEntryLock);

#ifdef MTFWD
	DBGPRINT(RT_DEBUG_OFF, ("Remove MacRep Sta:%pM\n", pEntry->CurrentAddress));
	RtmpOSWrielessEventSend(pEntry->wdev->if_dev,
				RT_WLAN_EVENT_CUSTOM,
				FWD_CMD_DEL_TX_SRC,
				NULL,
				pEntry->CurrentAddress,
				MAC_ADDR_LEN);
#endif

	return;
}

MAC_TABLE_ENTRY *RTMPInsertRepeaterMacEntry(
	IN  RTMP_ADAPTER *pAd,
	IN  UCHAR *pAddr,
	IN struct wifi_dev *wdev,
	IN  UCHAR apIdx,
	IN  UCHAR cliIdx,
	IN BOOLEAN CleanAll)
{
	UCHAR HashIdx;
	int i;
	MAC_TABLE_ENTRY *pEntry = NULL, *pCurrEntry;
	BOOLEAN Cancelled;
#if defined(APCLI_SAE_SUPPORT) || defined(APCLI_OWE_SUPPORT)
	MAC_TABLE_ENTRY *pre_mac_entry;
#endif
	if (pAd->MacTab.Size >= MAX_LEN_OF_MAC_TABLE)
		return NULL;

	/* allocate one MAC entry*/
	NdisAcquireSpinLock(&pAd->MacTabLock);

	i = (MAX_NUMBER_OF_MAC + ((MAX_EXT_MAC_ADDR_SIZE + 1) * (apIdx - MIN_NET_DEVICE_FOR_APCLI)));

	if (cliIdx != 0xFF)
		i  = i + cliIdx + 1;

	/* pick up the first available vacancy*/
	if (IS_ENTRY_NONE(&pAd->MacTab.Content[i]))
	{
		pEntry = &pAd->MacTab.Content[i];

		/* ENTRY PREEMPTION: initialize the entry */
		if (pEntry->RetryTimer.Valid)
			RTMPCancelTimer(&pEntry->RetryTimer, &Cancelled);
		if (pEntry->EnqueueStartForPSKTimer.Valid)
			RTMPCancelTimer(&pEntry->EnqueueStartForPSKTimer, &Cancelled);
#ifdef DOT11W_PMF_SUPPORT
		RTMPCancelTimer(&pEntry->SAQueryTimer, &Cancelled);
		RTMPCancelTimer(&pEntry->SAQueryConfirmTimer, &Cancelled);
#endif /* DOT11W_PMF_SUPPORT */

		NdisZeroMemory(pEntry, sizeof(MAC_TABLE_ENTRY));

		if (CleanAll == TRUE)
		{
			pEntry->MaxSupportedRate = RATE_11;
			pEntry->CurrTxRate = RATE_11;
			NdisZeroMemory(pEntry, sizeof(MAC_TABLE_ENTRY));
			pEntry->PairwiseKey.KeyLen = 0;
			pEntry->PairwiseKey.CipherAlg = CIPHER_NONE;
		}

#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
		if (apIdx >= MIN_NET_DEVICE_FOR_APCLI)
		{
			SET_ENTRY_APCLI(pEntry);
		}
#endif /* APCLI_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

		pEntry->wdev = wdev;
		pEntry->wcid = i;
		//SET_ENTRY_AP(pEntry);//Carter, why set Apcli Entry then set to AP entry?
		pAd->MacTab.tr_entry[i].isCached = FALSE;
		//tr_entry->isCached = FALSE;
		pEntry->bIAmBadAtheros = FALSE;

		RTMPInitTimer(pAd, &pEntry->EnqueueStartForPSKTimer, GET_TIMER_FUNCTION(EnqueueStartForPSKExec), pEntry, FALSE);

#ifdef APCLI_SUPPORT
		if (IS_ENTRY_APCLI(pEntry))
		{
			RTMPInitTimer(pAd, &pEntry->RetryTimer, GET_TIMER_FUNCTION(WPARetryExec), pEntry, FALSE);
		}
#endif /* APCLI_SUPPORT */
		


		pEntry->pAd = pAd;
		pEntry->CMTimerRunning = FALSE;
		pEntry->EnqueueEapolStartTimerRunning = EAPOL_START_DISABLE;
		pEntry->RSNIE_Len = 0;
		NdisZeroMemory(pEntry->R_Counter, sizeof(pEntry->R_Counter));
		pEntry->ReTryCounter = PEER_MSG1_RETRY_TIMER_CTR;
		pEntry->func_tb_idx = (apIdx - MIN_NET_DEVICE_FOR_APCLI);

		if (IS_ENTRY_APCLI(pEntry))
			pEntry->func_tb_idx = (apIdx - MIN_NET_DEVICE_FOR_APCLI);

		pEntry->pMbss = NULL;

#ifdef APCLI_SUPPORT
		if (IS_ENTRY_APCLI(pEntry))
		{

#if defined(APCLI_SAE_SUPPORT) || defined(APCLI_OWE_SUPPORT)
			pre_mac_entry =
				&pAd->ApCfg.ApCliTab[pEntry->func_tb_idx].pre_mac_entry;

			if (pAd->ApCfg.ApCliTab[pEntry->func_tb_idx].wdev.AuthMode == Ndis802_11AuthModeOWE ||
				pAd->ApCfg.ApCliTab[pEntry->func_tb_idx].wdev.AuthMode == Ndis802_11AuthModeWPA3PSK) {
				pEntry->AuthMode = pre_mac_entry->AuthMode;
				pEntry->WepStatus = pre_mac_entry->WepStatus;
#ifdef APCLI_SAE_SUPPORT
				if (pAd->ApCfg.ApCliTab[pEntry->func_tb_idx].wdev.AuthMode
					== Ndis802_11AuthModeWPA3PSK)
					NdisMoveMemory(pEntry->PMK, pre_mac_entry->PMK, LEN_PMK);
#endif
				pEntry->pmk_cache = pre_mac_entry->pmk_cache;
				pEntry->pmkid = pre_mac_entry->pmk_cache;
#ifdef APCLI_OWE_SUPPORT
				pEntry->need_process_ecdh_ie = pre_mac_entry->need_process_ecdh_ie;
				pEntry->key_deri_alg = pre_mac_entry->key_deri_alg;
				NdisMoveMemory(&pEntry->owe,
					&pre_mac_entry->owe,
					sizeof(OWE_INFO));
				NdisMoveMemory(&pEntry->ecdh_ie,
					&pre_mac_entry->ecdh_ie,
					sizeof(EXT_ECDH_PARAMETER_IE));
#endif
			} else
#endif
			{
				pEntry->AuthMode = pAd->ApCfg.ApCliTab[pEntry->func_tb_idx].wdev.AuthMode;
				pEntry->WepStatus = pAd->ApCfg.ApCliTab[pEntry->func_tb_idx].wdev.WepStatus;
			}

			if (pEntry->AuthMode < Ndis802_11AuthModeWPA)
			{
				pEntry->WpaState = AS_NOTUSE;
				pEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;
			}
			else
			{
				pEntry->WpaState = AS_PTKSTART;
				pEntry->PrivacyFilter = Ndis802_11PrivFilter8021xWEP;
			}
		}
#endif /* APCLI_SUPPORT */	

		pEntry->GTKState = REKEY_NEGOTIATING;
		pEntry->PairwiseKey.KeyLen = 0;
		pEntry->PairwiseKey.CipherAlg = CIPHER_NONE;
		pAd->MacTab.tr_entry[i].PortSecured = WPA_802_1X_PORT_NOT_SECURED;
		//pEntry->PortSecured = WPA_802_1X_PORT_NOT_SECURED;

		pEntry->PMKID_CacheIdx = ENTRY_NOT_FOUND;
		COPY_MAC_ADDR(pEntry->Addr, pAddr);

#ifdef APCLI_SUPPORT
		if (IS_ENTRY_APCLI(pEntry))
		{
			COPY_MAC_ADDR(pEntry->bssid, pAddr);
		}
#endif // APCLI_SUPPORT //

		pEntry->Sst = SST_NOT_AUTH;
		pEntry->AuthState = AS_NOT_AUTH;
		pEntry->Aid = (USHORT)i;
		pEntry->CapabilityInfo = 0;
		pEntry->PsMode = PWR_ACTIVE;
		pAd->MacTab.tr_entry[i].PsQIdleCount = 0;
		pAd->MacTab.tr_entry[i].PsTokenFlag = 0;
		//pEntry->PsQIdleCount = 0;
		pEntry->NoDataIdleCount = 0;
		pEntry->AssocDeadLine = MAC_TABLE_ASSOC_TIMEOUT;
		pEntry->ContinueTxFailCnt = 0;
		pEntry->TimeStamp_toTxRing = 0;
		// TODO: shiang-usw,  remove upper setting becasue we need to migrate to tr_entry!
		pAd->MacTab.tr_entry[i].PsMode = PWR_ACTIVE;
		pAd->MacTab.tr_entry[i].NoDataIdleCount = 0;
		pAd->MacTab.tr_entry[i].ContinueTxFailCnt = 0;
		pAd->MacTab.tr_entry[i].LockEntryTx = FALSE;
		pAd->MacTab.tr_entry[i].TimeStamp_toTxRing = 0;

		pAd->MacTab.Size ++;

		/* Set the security mode of this entry as OPEN-NONE in ASIC */
		RTMP_REMOVE_PAIRWISE_KEY_ENTRY(pAd, (UCHAR)i);

		/* Add this entry into ASIC RX WCID search table */
		RTMP_STA_ENTRY_ADD(pAd, pEntry);


		DBGPRINT(RT_DEBUG_TRACE, ("%s - allocate entry #%d, Aid = %d, Total= %d\n",__FUNCTION__, i, pEntry->Aid, pAd->MacTab.Size));

	}
	else
	{
		if (pEntry)
			DBGPRINT(RT_DEBUG_ERROR, ("%s - exist entry #%d, Aid = %d, Total= %d\n",
					__func__, i, pEntry->Aid, pAd->MacTab.Size));
		NdisReleaseSpinLock(&pAd->MacTabLock);
		return pEntry;
	}

	/* add this MAC entry into HASH table */
	if (pEntry)
	{
		HashIdx = MAC_ADDR_HASH_INDEX(pAddr);
		if (pAd->MacTab.Hash[HashIdx] == NULL)
		{
			pAd->MacTab.Hash[HashIdx] = pEntry;
		}
		else
		{
			pCurrEntry = pAd->MacTab.Hash[HashIdx];
			while (pCurrEntry->pNext != NULL)
				pCurrEntry = pCurrEntry->pNext;
			pCurrEntry->pNext = pEntry;
		}

	}

	NdisReleaseSpinLock(&pAd->MacTabLock);
	/*update tx burst, must after unlock pAd->MacTabLock*/
	rtmp_tx_burst_set(pAd);
	
	return pEntry;
}

VOID RTMPRepeaterReconnectionCheck(
	IN PRTMP_ADAPTER pAd)
{
#ifdef APCLI_AUTO_CONNECT_SUPPORT
	INT i;
	PCHAR	pApCliSsid, pApCliCfgSsid;
	UCHAR	CfgSsidLen;
	NDIS_802_11_SSID Ssid;
	
	if ((pAd->ApCfg.bMACRepeaterEn &&
		pAd->ApCfg.MACRepeaterOuiMode == 2 &&
		pAd->ApCfg.ApCliAutoConnectRunning == FALSE)
#ifdef AP_PARTIAL_SCAN_SUPPORT
		&& (pAd->ApCfg.bPartialScanning == FALSE)
#endif /* AP_PARTIAL_SCAN_SUPPORT */		
		)		
	{
		for (i = 0; i < MAX_APCLI_NUM; i++)
		{
			pApCliSsid = pAd->ApCfg.ApCliTab[i].Ssid;
			pApCliCfgSsid = pAd->ApCfg.ApCliTab[i].CfgSsid;
			CfgSsidLen = pAd->ApCfg.ApCliTab[i].CfgSsidLen;

			if (pAd->ApCfg.ApCliTab[i].AutoConnectFlag == FALSE)
				continue;

			if ((pAd->ApCfg.ApCliTab[i].CtrlCurrState < APCLI_CTRL_AUTH ||
				!NdisEqualMemory(pApCliSsid, pApCliCfgSsid, CfgSsidLen)) &&
				pAd->ApCfg.ApCliTab[i].CfgSsidLen > 0 && 
				pAd->Mlme.OneSecPeriodicRound % 23 == 0)
			{
				DBGPRINT(RT_DEBUG_TRACE, (" %s(): Scan channels for AP (%s)\n", 
							__FUNCTION__, pApCliCfgSsid));
				pAd->ApCfg.ApCliAutoConnectRunning = TRUE;
#ifdef AP_PARTIAL_SCAN_SUPPORT
				if(pAd->ApCfg.bPartialScanEnable)
				pAd->ApCfg.bPartialScanning = TRUE;
#endif /* AP_PARTIAL_SCAN_SUPPORT */
				Ssid.SsidLength = CfgSsidLen;
				NdisCopyMemory(Ssid.Ssid, pApCliCfgSsid, CfgSsidLen);
				ApSiteSurvey(pAd, &Ssid, SCAN_ACTIVE, FALSE, &pAd->ApCfg.ApCliTab[i].wdev);
			}	
		}
	}
#endif /* APCLI_AUTO_CONNECT_SUPPORT */
}

BOOLEAN RTMPRepeaterVaildMacEntry(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pAddr)
{
	INVAILD_TRIGGER_MAC_ENTRY *pEntry = NULL;

	if (pAd->ApCfg.RepeaterCliSize >= MAX_EXT_MAC_ADDR_SIZE)
		return FALSE;

	if(IS_MULTICAST_MAC_ADDR(pAddr))
		return FALSE;

	if(IS_BROADCAST_MAC_ADDR(pAddr))
		return FALSE;

	pEntry = RepeaterInvaildMacLookup(pAd, pAddr);

	if (pEntry)
		return FALSE;
	else
		return TRUE;
}

INVAILD_TRIGGER_MAC_ENTRY *RepeaterInvaildMacLookup(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pAddr)
{
	ULONG HashIdx;
	INVAILD_TRIGGER_MAC_ENTRY *pEntry = NULL;
	
	HashIdx = MAC_ADDR_HASH_INDEX(pAddr);
	pEntry = pAd->ApCfg.ReptControl.ReptInvaildHash[HashIdx];

	while (pEntry)
	{
		if (MAC_ADDR_EQUAL(pEntry->MacAddr, pAddr))
		{
			break;
		}
		else
			pEntry = pEntry->pNext;
	}

	if (pEntry && pEntry->bInsert)
		return pEntry;
	else
		return NULL;
}

VOID RTMPRepeaterInsertInvaildMacEntry(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pAddr)
{
	UCHAR HashIdx, idx = 0;
	INVAILD_TRIGGER_MAC_ENTRY *pEntry = NULL;
	INVAILD_TRIGGER_MAC_ENTRY *pCurrEntry = NULL;

	if (pAd->ApCfg.ReptControl.ReptInVaildMacSize >= 32)
		return;

	if (MAC_ADDR_EQUAL(pAddr, ZERO_MAC_ADDR))
		return;

	NdisAcquireSpinLock(&pAd->ApCfg.ReptCliEntryLock);
	for (idx = 0; idx< 32; idx++)
	{
		pEntry = &pAd->ApCfg.ReptControl.RepeaterInvaildEntry[idx];

		if (MAC_ADDR_EQUAL(pEntry->MacAddr, pAddr))
		{
			if (pEntry->bInsert)
			{
				NdisReleaseSpinLock(&pAd->ApCfg.ReptCliEntryLock);
				return;
			}
		}

		/* pick up the first available vacancy*/
		if (pEntry->bInsert == FALSE)
		{
			NdisZeroMemory(pEntry->MacAddr, MAC_ADDR_LEN);
			COPY_MAC_ADDR(pEntry->MacAddr, pAddr);
			pEntry->bInsert = TRUE;
			break;
		}
	}

	/* add this entry into HASH table */
	if (pEntry)
	{
		HashIdx = MAC_ADDR_HASH_INDEX(pAddr);
		pEntry->pNext = NULL;
		if (pAd->ApCfg.ReptControl.ReptInvaildHash[HashIdx] == NULL)
		{
			pAd->ApCfg.ReptControl.ReptInvaildHash[HashIdx] = pEntry;
		}
		else
		{
			pCurrEntry = pAd->ApCfg.ReptControl.ReptInvaildHash[HashIdx];
			while (pCurrEntry->pNext != NULL)
				pCurrEntry = pCurrEntry->pNext;
			pCurrEntry->pNext = pEntry;
		}
	}

	DBGPRINT(RT_DEBUG_ERROR, (" Store Invaild MacAddr = %02x:%02x:%02x:%02x:%02x:%02x. !!!\n",
				PRINT_MAC(pEntry->MacAddr)));

	pAd->ApCfg.ReptControl.ReptInVaildMacSize++;
	NdisReleaseSpinLock(&pAd->ApCfg.ReptCliEntryLock);

	return;
}

BOOLEAN RTMPRepeaterRemoveInvaildMacEntry(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR idx,
	IN PUCHAR pAddr)
{
	USHORT HashIdx;
	INVAILD_TRIGGER_MAC_ENTRY *pEntry = NULL;
	INVAILD_TRIGGER_MAC_ENTRY *pPrevEntry, *pProbeEntry;

	NdisAcquireSpinLock(&pAd->ApCfg.ReptCliEntryLock);

	HashIdx = MAC_ADDR_HASH_INDEX(pAddr);
	pEntry = &pAd->ApCfg.ReptControl.RepeaterInvaildEntry[idx];

	if (pEntry && pEntry->bInsert)
	{
		pPrevEntry = NULL;
		pProbeEntry = pAd->ApCfg.ReptControl.ReptInvaildHash[HashIdx];
		ASSERT(pProbeEntry);
		if (pProbeEntry != NULL)
		{
			/* update Hash list*/
			do
			{
				if (pProbeEntry == pEntry)
				{
					if (pPrevEntry == NULL)
					{
						pAd->ApCfg.ReptControl.ReptInvaildHash[HashIdx] = pEntry->pNext;
					}
					else
					{
						pPrevEntry->pNext = pEntry->pNext;
					}
					break;
				}
		
				pPrevEntry = pProbeEntry;
				pProbeEntry = pProbeEntry->pNext;
			} while (pProbeEntry);
		}
		/* not found !!!*/
		ASSERT(pProbeEntry != NULL);

		pAd->ApCfg.ReptControl.ReptInVaildMacSize--;
	}

	NdisZeroMemory(pEntry->MacAddr, MAC_ADDR_LEN);
	pEntry->bInsert = FALSE;

	NdisReleaseSpinLock(&pAd->ApCfg.ReptCliEntryLock);

	return TRUE;
}

INT Show_Repeater_Cli_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT i;
    //UINT32 RegValue;
	ULONG DataRate=0;

	if (!pAd->ApCfg.bMACRepeaterEn)
		return TRUE;

	printk("\n");

#ifdef DOT11_N_SUPPORT
	printk("HT Operating Mode : %d\n", pAd->CommonCfg.AddHTInfo.AddHtInfo2.OperaionMode);
	printk("\n");
#endif /* DOT11_N_SUPPORT */
	
	printk("\n%-19s%-4s%-4s%-4s%-4s%-8s%-7s%-7s%-7s%-10s%-6s%-6s%-6s%-6s%-7s%-7s\n",
		   "MAC", "AID", "BSS", "PSM", "WMM", "MIMOPS", "RSSI0", "RSSI1", 
		   "RSSI2", "PhMd", "BW", "MCS", "SGI", "STBC", "Idle", "Rate");

	for (i = 0; i < MAX_LEN_OF_MAC_TABLE; i++)
	{
		PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[i];
		if (pEntry && IS_ENTRY_APCLI(pEntry)&& (pEntry->Sst == SST_ASSOC) && (pEntry->bReptCli))
		{
			DataRate=0;
			getRate(pEntry->HTPhyMode, &DataRate);

			printk("%02X:%02X:%02X:%02X:%02X:%02X  ",
					pEntry->ReptCliAddr[0], pEntry->ReptCliAddr[1], pEntry->ReptCliAddr[2],
					pEntry->ReptCliAddr[3], pEntry->ReptCliAddr[4], pEntry->ReptCliAddr[5]);

			printk("%-4d", (int)pEntry->Aid);
			printk("%-4d", (int)pEntry->func_tb_idx);
			printk("%-4d", (int)pEntry->PsMode);
			printk("%-4d", (int)CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE));
#ifdef DOT11_N_SUPPORT
			printk("%-8d", (int)pEntry->MmpsMode);
#endif /* DOT11_N_SUPPORT */
			printk("%-7d", pEntry->RssiSample.AvgRssi[0]);
			printk("%-7d", pEntry->RssiSample.AvgRssi[1]);
			printk("%-7d", pEntry->RssiSample.AvgRssi[2]);
			printk("%-10s", get_phymode_str(pEntry->HTPhyMode.field.MODE));
			printk("%-6s", get_bw_str(pEntry->HTPhyMode.field.BW));
			printk("%-6d", pEntry->HTPhyMode.field.MCS);
			printk("%-6d", pEntry->HTPhyMode.field.ShortGI);
			printk("%-6d", pEntry->HTPhyMode.field.STBC);
			printk("%-7d", (int)(pEntry->StaIdleTimeout - pEntry->NoDataIdleCount));
			printk("%-7d", (int)DataRate);
			printk("%-10d, %d, %d%%\n", pEntry->DebugFIFOCount, pEntry->DebugTxCount, 
						(pEntry->DebugTxCount) ? ((pEntry->DebugTxCount-pEntry->DebugFIFOCount)*100/pEntry->DebugTxCount) : 0);
			printk("\n");
		}
	} 

	return TRUE;
}
REPEATER_CLIENT_ENTRY *lookup_rept_entry(RTMP_ADAPTER *pAd, PUCHAR address)
{
	REPEATER_CLIENT_ENTRY *rept_entry = NULL;
	UCHAR isLinkValid;

	rept_entry = RTMPLookupRepeaterCliEntry(
					 pAd,
					 FALSE,
					 address,
					 TRUE, &isLinkValid);

	if (!rept_entry)
		rept_entry = RTMPLookupRepeaterCliEntry(
						 pAd,
						 TRUE,
						 address,
						 TRUE, &isLinkValid);

	if (rept_entry)
		return rept_entry;

	return NULL;
}
#endif /* MAC_REPEATER_SUPPORT */


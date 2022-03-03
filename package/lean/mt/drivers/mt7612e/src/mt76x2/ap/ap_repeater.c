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

#define IS_MULTICAST_MAC_ADDR(Addr)			((((Addr[0]) & 0x01) == 0x01) && ((Addr[0]) != 0xff))
#define IS_BROADCAST_MAC_ADDR(Addr)			((((Addr[0]) & 0xff) == 0xff))

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

	if (bIsPad == TRUE) {
		NdisAcquireSpinLock(&((PRTMP_ADAPTER)pData)->ApCfg.ReptCliEntryLock);	
	} else {
		NdisAcquireSpinLock(((REPEATER_ADAPTER_DATA_TABLE *)pData)->EntryLock);
	}
	
	COPY_MAC_ADDR(tempMAC, pAddr);
	HashIdx = MAC_ADDR_HASH_INDEX(tempMAC);
    *pIsLinkValid = TRUE;

	if (bRealMAC == TRUE)
	{
		if (bIsPad == TRUE) {
			pMapEntry = ((PRTMP_ADAPTER)pData)->ApCfg.ReptMapHash[HashIdx];
		} else
			pMapEntry = *((((REPEATER_ADAPTER_DATA_TABLE *)pData)->MapHash) + HashIdx) ;

		
		while (pMapEntry)
		{
			pEntry = pMapEntry->pReptCliEntry;

			if (MAC_ADDR_EQUAL(pEntry->OriginalAddress, tempMAC))
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
			pEntry = *((((REPEATER_ADAPTER_DATA_TABLE *)pData)->CliHash) + HashIdx) ;
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

BOOLEAN RTMPQueryLookupRepeaterCliEntry(
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
EXPORT_SYMBOL(RTMPQueryLookupRepeaterCliEntry);
#endif /* CONFIG_WIFI_PKT_FWD */

VOID RTMPInsertRepeaterAsicEntry(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR CliIdx,
	IN PUCHAR pAddr)
{
	ULONG offset, Addr;
	UCHAR tempMAC[MAC_ADDR_LEN];

	DBGPRINT(RT_DEBUG_WARN, (" %s.\n", __FUNCTION__));

	COPY_MAC_ADDR(tempMAC, pAddr);
	
	offset = 0x1480 + (HW_WCID_ENTRY_SIZE * CliIdx);	
	Addr = tempMAC[0] + (tempMAC[1] << 8) +(tempMAC[2] << 16) +(tempMAC[3] << 24);
	RTMP_IO_WRITE32(pAd, offset, Addr);
	Addr = tempMAC[4] + (tempMAC[5] << 8);
	RTMP_IO_WRITE32(pAd, offset + 4, Addr); 

	DBGPRINT(RT_DEBUG_ERROR, ("\n%02x:%02x:%02x:%02x:%02x:%02x-%02x\n", 
							tempMAC[0],
							tempMAC[1],
							tempMAC[2], 
							tempMAC[3],
							tempMAC[4],
							tempMAC[5],
							CliIdx));

}

VOID RTMPRemoveRepeaterAsicEntry(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR CliIdx)
{
	ULONG offset, Addr;

	DBGPRINT(RT_DEBUG_WARN, (" %s.\n", __FUNCTION__));

	offset = 0x1480 + (HW_WCID_ENTRY_SIZE * CliIdx);
	Addr = 0;
	RTMP_IO_WRITE32(pAd, offset, Addr);
	RTMP_IO_WRITE32(pAd, offset + 4, Addr);
}

VOID RTMPInsertRepeaterEntry(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR apidx,
	IN PUCHAR pAddr)
{
	INT CliIdx, idx;
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

	DBGPRINT(RT_DEBUG_TRACE, (" %s.\n", __FUNCTION__));

       pMacEntry = MacTableLookup(pAd, pAddr);
       if (pMacEntry && IS_ENTRY_CLIENT(pMacEntry))
       {
               if (pMacEntry->PortSecured == WPA_802_1X_PORT_NOT_SECURED)
               {
                       DBGPRINT(RT_DEBUG_ERROR, (" wireless client is not ready !!!\n"));
                       return;
               }
       }

	NdisAcquireSpinLock(&pAd->ApCfg.ReptCliEntryLock);

	if (pAd->ApCfg.RepeaterCliSize >= MAX_EXT_MAC_ADDR_SIZE)
	{
		DBGPRINT(RT_DEBUG_ERROR, (" Repeater Client Full !!!\n"));
		NdisReleaseSpinLock(&pAd->ApCfg.ReptCliEntryLock);
		return ;
	}

	for (CliIdx = 0; CliIdx < MAX_EXT_MAC_ADDR_SIZE; CliIdx++)
	{
		pReptCliEntry = &pAd->ApCfg.ApCliTab[apidx].RepeaterCli[CliIdx];

		if ((pReptCliEntry->CliEnable) && 
			(MAC_ADDR_EQUAL(pReptCliEntry->OriginalAddress, pAddr) || MAC_ADDR_EQUAL(pReptCliEntry->CurrentAddress, pAddr)))
		{
			DBGPRINT(RT_DEBUG_ERROR, ("\n  receive mac :%02x:%02x:%02x:%02x:%02x:%02x !!!\n", 
						pAddr[0], pAddr[1], pAddr[2], pAddr[3], pAddr[4], pAddr[5]));
			DBGPRINT(RT_DEBUG_ERROR, (" duplicate Insert !!!\n"));
			NdisReleaseSpinLock(&pAd->ApCfg.ReptCliEntryLock);
			return ;
		}

		if (pReptCliEntry->CliEnable == FALSE)
			break;
	}

	if (CliIdx >= MAX_EXT_MAC_ADDR_SIZE)
	{
		DBGPRINT(RT_DEBUG_ERROR, (" Repeater Client Full !!!\n"));
		NdisReleaseSpinLock(&pAd->ApCfg.ReptCliEntryLock);
		return ;
	}

	pReptCliEntry = &pAd->ApCfg.ApCliTab[apidx].RepeaterCli[CliIdx];
	pReptCliMap = &pAd->ApCfg.ApCliTab[apidx].RepeaterCliMap[CliIdx];

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
	pReptCliMap->pReptCliEntry = pReptCliEntry;
	pReptCliMap->pNext = NULL;

	COPY_MAC_ADDR(pReptCliEntry->OriginalAddress, pAddr);
	COPY_MAC_ADDR(tempMAC, pAddr);
#ifdef SMART_MESH
	NdisZeroMemory(pAd->vMacAddrPrefix,sizeof(pAd->vMacAddrPrefix));
#endif /* SMART_MESH */

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
#ifdef SMART_MESH
		INT vMacIdx;
		if (IdxToUse >= 0  && IdxToUse < 5)
			vMacIdx = IdxToUse + 1;
		else
			vMacIdx = 0;
		
		NdisCopyMemory(pAd->vMacAddrPrefix, SPEC_ADDR[vMacIdx], sizeof(pAd->vMacAddrPrefix));
#endif /* SMART_MESH */
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

	RTMPInsertRepeaterAsicEntry(pAd, CliIdx, tempMAC);
		
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
		pCurrMapEntry->pNext = pReptCliMap;
	}

	pAd->ApCfg.RepeaterCliSize++;
	NdisReleaseSpinLock(&pAd->ApCfg.ReptCliEntryLock);

	NdisZeroMemory(&ApCliCtrlMsg, sizeof(APCLI_CTRL_MSG_STRUCT));
	ApCliCtrlMsg.Status = MLME_SUCCESS;
	COPY_MAC_ADDR(&ApCliCtrlMsg.SrcAddr[0], tempMAC);
	ApCliCtrlMsg.BssIdx = apidx;
	ApCliCtrlMsg.CliIdx = CliIdx;

	MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_MT2_AUTH_REQ,
			sizeof(APCLI_CTRL_MSG_STRUCT), &ApCliCtrlMsg, apidx);

}

VOID RTMPRemoveRepeaterEntry(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR apIdx,
	IN UCHAR CliIdx)
{
	USHORT HashIdx;
	REPEATER_CLIENT_ENTRY *pEntry, *pPrevEntry, *pProbeEntry;
	REPEATER_CLIENT_ENTRY_MAP *pMapEntry, *pPrevMapEntry, *pProbeMapEntry;
	BOOLEAN bVaild;

	DBGPRINT(RT_DEBUG_OFF, (" %s. apIdx=%d CliIdx=%d\n", __FUNCTION__,apIdx,CliIdx));

	RTMPRemoveRepeaterAsicEntry(pAd, CliIdx);

	NdisAcquireSpinLock(&pAd->ApCfg.ReptCliEntryLock);
	pEntry = &pAd->ApCfg.ApCliTab[apIdx].RepeaterCli[CliIdx];

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

	pMapEntry = &pAd->ApCfg.ApCliTab[apIdx].RepeaterCliMap[CliIdx];

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
				break;
			}

			pPrevMapEntry = pProbeMapEntry;
			pProbeMapEntry = pProbeMapEntry->pNext;
		} while (pProbeMapEntry);
	}
	/* not found !!!*/
	ASSERT(pProbeMapEntry != NULL);

done:

	pAd->ApCfg.ApCliTab[apIdx].RepeaterCli[CliIdx].CliConnectState = 0;
	NdisZeroMemory(pAd->ApCfg.ApCliTab[apIdx].RepeaterCli[CliIdx].OriginalAddress, MAC_ADDR_LEN);

	if ((bVaild == TRUE) && (pAd->ApCfg.RepeaterCliSize > 0))
	pAd->ApCfg.RepeaterCliSize--;

	/* set the apcli interface be invalid. */
	pAd->ApCfg.ApCliTab[apIdx].RepeaterCli[CliIdx].CliValid = FALSE;
	pAd->ApCfg.ApCliTab[apIdx].RepeaterCli[CliIdx].CliEnable = FALSE;

	NdisReleaseSpinLock(&pAd->ApCfg.ReptCliEntryLock);

	return;
}

MAC_TABLE_ENTRY *RTMPInsertRepeaterMacEntry(
	IN  RTMP_ADAPTER *pAd,
	IN  UCHAR *pAddr,
	IN  struct wifi_dev *wdev,
	IN  UCHAR apIdx,
	IN  UCHAR cliIdx,
	IN BOOLEAN CleanAll)
{
	UCHAR HashIdx;
	int i;
	MAC_TABLE_ENTRY *pEntry = NULL, *pCurrEntry;
	BOOLEAN Cancelled;

	/* if FULL, return*/
	if (pAd->MacTab.Size >= MAX_LEN_OF_MAC_TABLE)
		return NULL;

	/* allocate one MAC entry*/
	NdisAcquireSpinLock(&pAd->MacTabLock);

	i = (MAX_NUMBER_OF_MAC + ((MAX_EXT_MAC_ADDR_SIZE + 1) * (apIdx - MIN_NET_DEVICE_FOR_APCLI)));

	if (cliIdx != 0xFF)
		i  = i + cliIdx + 1;

	/* pick up the first available vacancy*/
	pEntry = &pAd->MacTab.Content[i];

	if (pEntry == NULL)
	{
		printk("###### %s pEntry == NULL, i = %d\n", __func__, i);
	}

	if (pEntry && IS_ENTRY_NONE(pEntry))
	{
		/* ENTRY PREEMPTION: initialize the entry */
		if (pEntry->RetryTimer.Valid)
			RTMPCancelTimer(&pEntry->RetryTimer, &Cancelled);
		if (pEntry->EnqueueStartForPSKTimer.Valid)
			RTMPCancelTimer(&pEntry->EnqueueStartForPSKTimer, &Cancelled);

		NdisZeroMemory(pEntry, sizeof(MAC_TABLE_ENTRY));

		if (CleanAll == TRUE)
		{
			pEntry->MaxSupportedRate = RATE_11;
			pEntry->CurrTxRate = RATE_11;
			NdisZeroMemory(pEntry, sizeof(MAC_TABLE_ENTRY));
			pEntry->PairwiseKey.KeyLen = 0;
			pEntry->PairwiseKey.CipherAlg = CIPHER_NONE;
		}

		SET_ENTRY_APCLI(pEntry);
		pEntry->wdev = wdev;
		pEntry->wcid = i;
		pEntry->isCached = FALSE;
		pEntry->bIAmBadAtheros = FALSE;

		RTMPInitTimer(pAd, &pEntry->EnqueueStartForPSKTimer, GET_TIMER_FUNCTION(EnqueueStartForPSKExec), pEntry, FALSE);
		RTMPInitTimer(pAd, &pEntry->RetryTimer, GET_TIMER_FUNCTION(WPARetryExec), pEntry, FALSE);

#ifdef TXBF_SUPPORT
		if (pAd->chipCap.FlgHwTxBfCap)
			RTMPInitTimer(pAd, &pEntry->eTxBfProbeTimer, GET_TIMER_FUNCTION(eTxBfProbeTimerExec), pEntry, FALSE);
#endif /* TXBF_SUPPORT */

		pEntry->pAd = pAd;
		pEntry->CMTimerRunning = FALSE;
		pEntry->EnqueueEapolStartTimerRunning = EAPOL_START_DISABLE;
		pEntry->RSNIE_Len = 0;
		NdisZeroMemory(pEntry->R_Counter, sizeof(pEntry->R_Counter));
		pEntry->ReTryCounter = PEER_MSG1_RETRY_TIMER_CTR;
		pEntry->apidx = (apIdx - MIN_NET_DEVICE_FOR_APCLI);
		pEntry->pMbss = &pAd->ApCfg.MBSSID[pEntry->apidx];

		pEntry->AuthMode = pAd->ApCfg.ApCliTab[pEntry->apidx].wdev.AuthMode;
		pEntry->WepStatus = pAd->ApCfg.ApCliTab[pEntry->apidx].wdev.WepStatus;
		pEntry->wdev_idx = pEntry->apidx;

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

		pEntry->GTKState = REKEY_NEGOTIATING;
		pEntry->PairwiseKey.KeyLen = 0;
		pEntry->PairwiseKey.CipherAlg = CIPHER_NONE;
		pEntry->PortSecured = WPA_802_1X_PORT_NOT_SECURED;

		pEntry->PMKID_CacheIdx = ENTRY_NOT_FOUND;
		COPY_MAC_ADDR(pEntry->Addr, pAddr);
		//COPY_MAC_ADDR(pEntry->HdrAddr1, pAddr);
		//COPY_MAC_ADDR(pEntry->HdrAddr2, pAd->ApCfg.ApCliTab[pEntry->apidx].CurrentAddress);
		//COPY_MAC_ADDR(pEntry->HdrAddr3, pAddr);
		COPY_MAC_ADDR(pEntry->bssid, pAddr);

		pEntry->Sst = SST_NOT_AUTH;
		pEntry->AuthState = AS_NOT_AUTH;
		pEntry->Aid = (USHORT)i;  /*0;*/
		pEntry->CapabilityInfo = 0;
		pEntry->PsMode = PWR_ACTIVE;
		pEntry->PsQIdleCount = 0;
		pEntry->NoDataIdleCount = 0;
		pEntry->AssocDeadLine = MAC_TABLE_ASSOC_TIMEOUT;
		pEntry->ContinueTxFailCnt = 0;
		pEntry->TimeStamp_toTxRing = 0;
		InitializeQueueHeader(&pEntry->PsQueue);

#ifdef PS_ENTRY_MAITENANCE
		pEntry->continuous_ps_count = 0;
#endif /* PS_ENTRY_MAITENANCE */

		pAd->MacTab.Size ++;

		/* Set the security mode of this entry as OPEN-NONE in ASIC */
		RTMP_REMOVE_PAIRWISE_KEY_ENTRY(pAd, (UCHAR)i);

		/* Add this entry into ASIC RX WCID search table */
		RTMP_STA_ENTRY_ADD(pAd, pEntry);

#ifdef WSC_AP_SUPPORT
		pEntry->bWscCapable = FALSE;
		pEntry->Receive_EapolStart_EapRspId = 0;
#endif /* WSC_AP_SUPPORT */

#ifdef TXBF_SUPPORT
		if (pAd->chipCap.FlgHwTxBfCap)
			NdisAllocateSpinLock(pAd, &pEntry->TxSndgLock);
#endif /* TXBF_SUPPORT */

#ifdef PEER_DELBA_TX_ADAPT
		Peer_DelBA_Tx_Adapt_Init(pAd, pEntry);
#endif /* PEER_DELBA_TX_ADAPT */
#ifdef DROP_MASK_SUPPORT
		drop_mask_init_per_client(pAd, pEntry);
#endif /* DROP_MASK_SUPPORT */

#ifdef FIFO_EXT_SUPPORT
		if (pAd->chipCap.FlgHwFifoExtCap)
		{
			UCHAR tblIdx;

			if ((cliIdx != 0xFF) && IsFifoExtTblAvailable(pAd, &tblIdx))
				FifoExtTblUpdateEntry(pAd, tblIdx, i);
		}
#endif
		DBGPRINT(RT_DEBUG_TRACE, ("%s - allocate entry #%d, Aid = %d, Wcid = %d Addr(%02x:%02x:%02x:%02x:%02x:%02x) Total= %d\n",__FUNCTION__, i, 
		pEntry->Aid, pEntry->wcid, PRINT_MAC(pEntry->Addr), pAd->MacTab.Size));
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s - exist entry #%d, Aid = %d, Total= %d\n", __FUNCTION__, i, pEntry->Aid, pAd->MacTab.Size));
		NdisReleaseSpinLock(&pAd->MacTabLock);
		return pEntry;
	}

	/* add this MAC entry into HASH table */
	if (pEntry)
	{
		pEntry->pNext = NULL;

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
    	USHORT SiteSurveyPeriod;
	
	if ((pAd->ApCfg.ApCliAutoConnectRunning == FALSE)
#ifdef AP_PARTIAL_SCAN_SUPPORT
		&& (pAd->ApCfg.bPartialScanning == FALSE)
#endif /* AP_PARTIAL_SCAN_SUPPORT */
		)
	{
		for (i = 0; i < MAX_APCLI_NUM; i++)
		{
			if (!APCLI_IF_UP_CHECK(pAd, i) ||
				(pAd->ApCfg.ApCliTab[i].Enable == FALSE))
				continue;
			
			pApCliSsid = pAd->ApCfg.ApCliTab[i].Ssid;
			pApCliCfgSsid = pAd->ApCfg.ApCliTab[i].CfgSsid;
			CfgSsidLen = pAd->ApCfg.ApCliTab[i].CfgSsidLen;
			SiteSurveyPeriod = pAd->ApCfg.ApCliTab[i].ApCliSiteSurveyPeriod;
            
			if ((pAd->ApCfg.ApCliTab[i].CtrlCurrState < APCLI_CTRL_AUTH ||
				!NdisEqualMemory(pApCliSsid, pApCliCfgSsid, CfgSsidLen)) &&
				(pAd->ApCfg.ApCliTab[i].CfgSsidLen > 0) && 
				(pAd->Mlme.OneSecPeriodicRound % SiteSurveyPeriod == 0))
			{
				DBGPRINT(RT_DEBUG_TRACE, (" %s(): Scan channels for AP (%s)\n", 
							__FUNCTION__, pApCliCfgSsid));
				pAd->ApCfg.ApCliAutoConnectRunning = TRUE;
#ifdef AP_PARTIAL_SCAN_SUPPORT
				pAd->ApCfg.bPartialScanning = TRUE;
#endif /* AP_PARTIAL_SCAN_SUPPORT */
				Ssid.SsidLength = CfgSsidLen;
				NdisCopyMemory(Ssid.Ssid, pApCliCfgSsid, CfgSsidLen);
				ApSiteSurvey(pAd, &Ssid, SCAN_ACTIVE, FALSE);
			}	
		}
	}
#endif /* APCLI_AUTO_CONNECT_SUPPORT */
}

VOID RTMPRemoveRepeaterDisconnectEntry(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR apIdx,
	IN UCHAR CliIdx)
{
	PAPCLI_STRUCT pApCliEntry;
	USHORT ifIndex = apIdx;
	PULONG pCurrState = NULL;
	BOOLEAN bValid = FALSE;
	MLME_DISASSOC_REQ_STRUCT DisassocReq;
	MLME_DEAUTH_REQ_STRUCT	DeAuthFrame;
	BOOLEAN Cancelled;

	DBGPRINT(RT_DEBUG_OFF, ("(%s) disconnect apIdx=%d CliIdx=%d.\n", __FUNCTION__,apIdx, CliIdx));

	if (ifIndex >= MAX_APCLI_NUM)
		return;

	pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].CtrlCurrState;
	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];
	bValid = pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].CliValid;

	RTMPCancelTimer(&pApCliEntry->RepeaterCli[CliIdx].ApCliAssocTimer, &Cancelled);
	RTMPCancelTimer(&pApCliEntry->RepeaterCli[CliIdx].ApCliAuthTimer, &Cancelled);

	if (*pCurrState == APCLI_CTRL_ASSOC)
	{
		*pCurrState = APCLI_CTRL_DEASSOC;

		DisassocParmFill(pAd, &DisassocReq, pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.Bssid, REASON_DISASSOC_STA_LEAVING);

		MlmeEnqueue(pAd,
						APCLI_ASSOC_STATE_MACHINE,
						APCLI_MT2_MLME_DISASSOC_REQ,
						sizeof(MLME_DISASSOC_REQ_STRUCT),
						&DisassocReq,
						(64 + (MAX_EXT_MAC_ADDR_SIZE*ifIndex) + CliIdx));

		RTMP_MLME_HANDLER(pAd);
	}
	else if (*pCurrState == APCLI_CTRL_CONNECTED)
	{
		DeAuthFrame.Reason = (USHORT)REASON_DEAUTH_STA_LEAVING;
		COPY_MAC_ADDR(DeAuthFrame.Addr, pAd->ApCfg.ApCliTab[ifIndex].MlmeAux.Bssid);

		MlmeEnqueue(pAd, 
					  APCLI_AUTH_STATE_MACHINE, 
					  APCLI_MT2_MLME_DEAUTH_REQ, 
					  sizeof(MLME_DEAUTH_REQ_STRUCT),
					  &DeAuthFrame, 
					  (64 + (MAX_EXT_MAC_ADDR_SIZE*ifIndex) + CliIdx));

		RTMP_MLME_HANDLER(pAd);

		if (bValid)
			ApCliLinkDown(pAd, (64 + (MAX_EXT_MAC_ADDR_SIZE*ifIndex) + CliIdx));

		*pCurrState = APCLI_CTRL_DISCONNECTED;
	}
	else
	{
		if (bValid)
			ApCliLinkDown(pAd, (64 + (MAX_EXT_MAC_ADDR_SIZE*ifIndex) + CliIdx));

		*pCurrState = APCLI_CTRL_DISCONNECTED;
	}


	return;
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
			else
			{
				pEntry->bInsert = TRUE;
				break;
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

INT	Show_Repeater_Cli_Proc(
	IN PRTMP_ADAPTER pAd, 
	IN PSTRING arg)
{
	INT i;
    UINT32 RegValue;
	UINT32 DataRate=0;

	if (!pAd->ApCfg.bMACRepeaterEn)
		return TRUE;

	printk("\n");
	RTMP_IO_READ32(pAd, BKOFF_SLOT_CFG, &RegValue);
	printk("BackOff Slot      : %s slot time, BKOFF_SLOT_CFG(0x1104) = 0x%08x\n", 
			OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SHORT_SLOT_INUSED) ? "short" : "long",
 			RegValue);

#ifdef DOT11_N_SUPPORT
	printk("HT Operating Mode : %d\n", pAd->CommonCfg.AddHTInfo.AddHtInfo2.OperaionMode);
	printk("\n");
#endif /* DOT11_N_SUPPORT */
	
	printk("\n%-19s%-4s%-4s%-4s%-4s%-8s%-7s%-7s%-7s%-10s%-6s%-6s%-6s%-6s%-7s%-7s\n",
		   "MAC", "AID", "BSS", "PSM", "WMM", "MIMOPS", "RSSI0", "RSSI1", 
		   "RSSI2", "PhMd", "BW", "MCS", "SGI", "STBC", "Idle", "Rate");
	
	for (i = MAX_NUMBER_OF_MAC; i < MAX_NUMBER_OF_MAC + ((MAX_EXT_MAC_ADDR_SIZE + 1) * MAX_APCLI_NUM); i++)
	{
		PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[i];
		if (pEntry && IS_ENTRY_APCLI(pEntry)&& (pEntry->Sst == SST_ASSOC) && (pEntry->bReptCli))
		{
			DataRate=0;
			RtmpDrvRateGet(pAd, pEntry->HTPhyMode.field.MODE, pEntry->HTPhyMode.field.ShortGI,
				 pEntry->HTPhyMode.field.BW,pEntry->HTPhyMode.field.MCS,
				 newRateGetAntenna(pEntry->HTPhyMode.field.MCS, pEntry->HTPhyMode.field.MODE),&DataRate);
			DataRate /= 500000;
			DataRate /= 2;

			printk("%02X:%02X:%02X:%02X:%02X:%02X  ",
					pEntry->ReptCliAddr[0], pEntry->ReptCliAddr[1], pEntry->ReptCliAddr[2],
					pEntry->ReptCliAddr[3], pEntry->ReptCliAddr[4], pEntry->ReptCliAddr[5]);

			printk("%-4d", (int)pEntry->Aid);
			printk("%-4d", (int)pEntry->apidx);
			printk("%-4d", (int)pEntry->PsMode);
			printk("%-4d", (int)CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE));
#ifdef DOT11_N_SUPPORT
			printk("%-8d", (int)pEntry->MmpsMode);
#endif /* DOT11_N_SUPPORT */
			printk("%-7d", pEntry->RssiSample.AvgRssi0);
			printk("%-7d", pEntry->RssiSample.AvgRssi1);
			printk("%-7d", pEntry->RssiSample.AvgRssi2);
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

INT	Show_Repeater_Cli_Dump_Proc(
	IN PRTMP_ADAPTER pAd, 
	IN PSTRING arg)
{
	INT CliIdx;
	
	printk("\n%-19s%-19s%-12s%-12s%-12s%-12s%-12s\n",
		   "C_MAC","O_MAC", "CliEnable", "CliValid", "bEthCli", "MacTabWCID","MatchLinkIdx");

	
	for (CliIdx = 0; CliIdx < MAX_EXT_MAC_ADDR_SIZE; CliIdx++)
	{
			REPEATER_CLIENT_ENTRY *pReptEntry = NULL;
			pReptEntry = &pAd->ApCfg.ApCliTab[0].RepeaterCli[CliIdx];
			
			{

				printk("%02X:%02X:%02X:%02X:%02X:%02X  ",
						pReptEntry->CurrentAddress[0], pReptEntry->CurrentAddress[1], pReptEntry->CurrentAddress[2],
						pReptEntry->CurrentAddress[3], pReptEntry->CurrentAddress[4], pReptEntry->CurrentAddress[5]);
				printk("%02X:%02X:%02X:%02X:%02X:%02X  ",
						pReptEntry->OriginalAddress[0], pReptEntry->OriginalAddress[1], pReptEntry->OriginalAddress[2],
						pReptEntry->OriginalAddress[3], pReptEntry->OriginalAddress[4], pReptEntry->OriginalAddress[5]);
				
				printk("%-12d", (int)pReptEntry->CliEnable);
				printk("%-12d", (int)pReptEntry->CliValid);
				printk("%-12d", (int)pReptEntry->bEthCli);
				printk("%-12d", (int)pReptEntry->MacTabWCID);
				printk("%-12d", (int)pReptEntry->MatchLinkIdx);
				printk("\n");
			}
		}	
}
#endif /* MAC_REPEATER_SUPPORT */


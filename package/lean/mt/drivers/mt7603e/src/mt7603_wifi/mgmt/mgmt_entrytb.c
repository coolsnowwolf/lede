/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:

	Abstract:

	Revision History:
	Who 		When			What
	--------	----------		----------------------------------------------
*/

#include <rt_config.h>
#ifdef ANDLINK_FEATURE_SUPPORT
#include <linux/time.h>
#endif

#ifdef DBG
VOID dump_tr_entry(RTMP_ADAPTER *pAd, INT tr_idx, RTMP_STRING *caller, INT line)
{
	STA_TR_ENTRY *tr_entry;
	INT qidx;

	ASSERT(tr_idx < MAX_LEN_OF_TR_TABLE);
	if (tr_idx >= MAX_LEN_OF_TR_TABLE)
		return;

	DBGPRINT(RT_DEBUG_OFF, ("Dump TR_ENTRY called by function %s(%d)\n", caller, line));
	tr_entry = &pAd->MacTab.tr_entry[tr_idx];
	DBGPRINT(RT_DEBUG_OFF, ("TR_ENTRY[%d]\n", tr_idx));
	DBGPRINT(RT_DEBUG_OFF, ("\tEntryType=%x\n", tr_entry->EntryType));
	DBGPRINT(RT_DEBUG_OFF, ("\twdev=%p\n", tr_entry->wdev));
	DBGPRINT(RT_DEBUG_OFF, ("\twcid=%d\n", tr_entry->wcid));
	DBGPRINT(RT_DEBUG_OFF, ("\tfunc_tb_idx=%d\n", tr_entry->func_tb_idx));
	DBGPRINT(RT_DEBUG_OFF, ("\tAddr=%02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(tr_entry->Addr)));
	DBGPRINT(RT_DEBUG_OFF, ("\tBSSID=%02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(tr_entry->bssid)));

	DBGPRINT(RT_DEBUG_OFF, ("\tFlags\n"));
	DBGPRINT(RT_DEBUG_OFF, ("\t\tbIAmBadAtheros=%d, isCached=%d, PortSecured=%d, PsMode=%d, LockEntryTx=%d\n", 
				tr_entry->bIAmBadAtheros, tr_entry->isCached, tr_entry->PortSecured, tr_entry->PsMode, tr_entry->LockEntryTx));

	DBGPRINT(RT_DEBUG_OFF, ("\tTxRx Characters\n"));
	DBGPRINT(RT_DEBUG_OFF, ("\t\tNonQosDataSeq=%d\n", tr_entry->NonQosDataSeq));
	DBGPRINT(RT_DEBUG_OFF, ("\t\tTxSeq[0]=%d, TxSeq[1]=%d, TxSeq[2]=%d, TxSeq[3]=%d\n", 
				tr_entry->TxSeq[0], tr_entry->TxSeq[1], tr_entry->TxSeq[2], tr_entry->TxSeq[3]));
	DBGPRINT(RT_DEBUG_OFF, ("\tCurrTxRate=%x\n", tr_entry->CurrTxRate));
	
	DBGPRINT(RT_DEBUG_OFF, ("\tQueuing Info\n"));
	DBGPRINT(RT_DEBUG_OFF, ("\t\tenq_cap=%d, deq_cap=%d\n", tr_entry->enq_cap, tr_entry->deq_cap));
	DBGPRINT(RT_DEBUG_OFF, ("\t\tQueuedPkt: TxQ[0]=%d, TxQ[1]=%d, TxQ[2]=%d, TxQ[3]=%d, PSQ=%d\n", 
							tr_entry->tx_queue[0].Number, tr_entry->tx_queue[1].Number, 
							tr_entry->tx_queue[2].Number, tr_entry->tx_queue[3].Number,
							tr_entry->ps_queue.Number));
	DBGPRINT(RT_DEBUG_OFF, ("\t\tdeq_cnt=%d, deq_bytes=%d\n", tr_entry->deq_cnt, tr_entry->deq_bytes));

	for (qidx = 0 ; qidx < 4; qidx++) {
		rtmp_sta_txq_dump(pAd, tr_entry, qidx);
		rtmp_tx_swq_dump(pAd, qidx);
	}
}
#endif /* DBG */

VOID tr_tb_reset_entry(RTMP_ADAPTER *pAd, UCHAR tr_tb_idx)
{
	struct _STA_TR_ENTRY *tr_entry;
	INT qidx;
	//PNDIS_PACKET pPacket;

	if (tr_tb_idx >= MAX_LEN_OF_TR_TABLE)
		return;

	tr_entry = &pAd->MacTab.tr_entry[tr_tb_idx];
	if (IS_ENTRY_NONE(tr_entry))
		return;

	tr_entry->enq_cap = FALSE;
	tr_entry->deq_cap = FALSE;

#ifdef DATA_QUEUE_RESERVE
	tr_entry->high_pkt_cnt = 0;
	tr_entry->high_pkt_drop_cnt = 0;
#endif /* DATA_QUEUE_RESERVE */
	
	rtmp_tx_swq_exit(pAd, tr_tb_idx);

	SET_ENTRY_NONE(tr_entry);

	for (qidx = 0; qidx < WMM_QUE_NUM; qidx++)
		NdisFreeSpinLock(&tr_entry->txq_lock[qidx]);
	NdisFreeSpinLock(&tr_entry->ps_queue_lock);

	return;
}


VOID tr_tb_set_entry(RTMP_ADAPTER *pAd, UCHAR tr_tb_idx, MAC_TABLE_ENTRY *pEntry)
{
	struct _STA_TR_ENTRY *tr_entry;
	INT qidx, tid,upId;
    //MAC_TABLE_ENTRY *mac_entry;
    struct wtbl_entry tb_entry;
    //struct wtbl_2_struc *wtbl_2;

	if (tr_tb_idx < MAX_LEN_OF_TR_TABLE)
	{
		tr_entry = &pAd->MacTab.tr_entry[tr_tb_idx];

		tr_entry->EntryType = pEntry->EntryType;
		tr_entry->wdev = pEntry->wdev;
		tr_entry->func_tb_idx = pEntry->func_tb_idx;

		tr_entry->wcid = pEntry->wcid;
		NdisMoveMemory(tr_entry->Addr, pEntry->Addr, MAC_ADDR_LEN);

		tr_entry->NonQosDataSeq = 0;
		for (tid = 0; tid < NUM_OF_TID; tid++)
			tr_entry->TxSeq[tid] = 0;

		for(upId = 0 ; upId < NUM_OF_UP ; upId ++)
		{		
			tr_entry->cacheSn[upId] = -1;
		}

        /* Reset BA SSN & Score Board Bitmap, for BA Receiptor */
        NdisZeroMemory(&tb_entry, sizeof(tb_entry));
        if (mt_wtbl_get_entry234(pAd, tr_entry->wcid, &tb_entry) == FALSE) {
            DBGPRINT(RT_DEBUG_ERROR, ("%s(): Not found WTBL2/3/4 for tr_entry->wcid(%d), !ERROR!\n",
                        __FUNCTION__, tr_entry->wcid));
            return;          
        }

        //mac_entry = &pAd->MacTab.Content[tr_entry->wcid];

		tr_entry->PsMode = PWR_ACTIVE;
		tr_entry->isCached = FALSE;
		tr_entry->PortSecured = WPA_802_1X_PORT_NOT_SECURED;
		tr_entry->CurrTxRate = pEntry->CurrTxRate;

		for (qidx = 0 ; qidx < WMM_QUE_NUM; qidx++) {
			InitializeQueueHeader(&tr_entry->tx_queue[qidx]);
			NdisAllocateSpinLock(pAd, &tr_entry->txq_lock[qidx]);
		}
		InitializeQueueHeader(&tr_entry->ps_queue);
		NdisAllocateSpinLock(pAd, &tr_entry->ps_queue_lock);

		tr_entry->deq_cnt = 0;
		tr_entry->deq_bytes = 0;
		tr_entry->PsQIdleCount = 0;
		tr_entry->enq_cap = TRUE;
		tr_entry->deq_cap = TRUE;
		tr_entry->PsTokenFlag = 0;
		NdisMoveMemory(tr_entry->bssid, pEntry->wdev->bssid, MAC_ADDR_LEN);
	}

}


VOID tr_tb_set_mcast_entry(RTMP_ADAPTER *pAd, UCHAR tr_tb_idx, struct wifi_dev *wdev)
{
	struct _STA_TR_ENTRY *tr_entry = &pAd->MacTab.tr_entry[tr_tb_idx];
	INT qidx, tid;

	NdisZeroMemory(tr_entry, sizeof(struct _STA_TR_ENTRY));

	tr_entry->EntryType = ENTRY_CAT_MCAST;
	tr_entry->wdev = wdev;
	tr_entry->func_tb_idx = wdev->func_idx;
	tr_entry->PsMode = PWR_ACTIVE;
	tr_entry->isCached = FALSE;
	tr_entry->PortSecured = WPA_802_1X_PORT_SECURED;
	tr_entry->CurrTxRate = pAd->CommonCfg.MlmeRate;
	NdisMoveMemory(tr_entry->Addr, &BROADCAST_ADDR[0], MAC_ADDR_LEN);

	// TODO: shiang-usw, for following fields, need better assignment!
	tr_entry->wcid = tr_tb_idx;
	tr_entry->NonQosDataSeq = 0;
	for (tid = 0; tid < NUM_OF_TID; tid++)
		tr_entry->TxSeq[tid] = 0;

	for (qidx = 0 ; qidx < WMM_QUE_NUM; qidx++) {
		InitializeQueueHeader(&tr_entry->tx_queue[qidx]);
		NdisAllocateSpinLock(pAd, &tr_entry->txq_lock[qidx]);
	}
	InitializeQueueHeader(&tr_entry->ps_queue);
	NdisAllocateSpinLock(pAd, &tr_entry->ps_queue_lock);
	tr_entry->deq_cnt = 0;
	tr_entry->deq_bytes = 0;
	tr_entry->PsQIdleCount = 0;
	tr_entry->enq_cap = TRUE;
	tr_entry->deq_cap = TRUE;
	tr_entry->PsTokenFlag = 0;
#ifdef DATA_QUEUE_RESERVE
	tr_entry->high_pkt_cnt = 0;
	tr_entry->high_pkt_drop_cnt = 0;
#endif /* DATA_QUEUE_RESERVE */

	NdisMoveMemory(tr_entry->bssid, wdev->bssid, MAC_ADDR_LEN);
	
}


// TODO: this function not finish yet!!
VOID mgmt_tb_set_mcast_entry(RTMP_ADAPTER *pAd, UCHAR wcid)
{
	MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.Content[MCAST_WCID];

	pEntry->EntryType = ENTRY_CAT_MCAST;
	pEntry->Sst = SST_ASSOC;
	pEntry->Aid = MCAST_WCID;	/* Softap supports 1 BSSID and use WCID=0 as multicast Wcid index*/
	pEntry->wcid = MCAST_WCID;
	pEntry->PsMode = PWR_ACTIVE;
	pEntry->CurrTxRate = pAd->CommonCfg.MlmeRate;

	pEntry->Addr[0] = 0x01;
	pEntry->HTPhyMode.field.MODE = MODE_OFDM;
	pEntry->HTPhyMode.field.MCS = 3;

	NdisMoveMemory(pEntry->Addr, &BROADCAST_ADDR[0], MAC_ADDR_LEN);
	
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd){
		pEntry->wdev = &pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev;
	}
#endif /* CONFIG_AP_SUPPORT */
}


VOID set_entry_phy_cfg(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry)
{

	if (pEntry->MaxSupportedRate < RATE_FIRST_OFDM_RATE)
	{
		pEntry->MaxHTPhyMode.field.MODE = MODE_CCK;
		pEntry->MaxHTPhyMode.field.MCS = pEntry->MaxSupportedRate;
		pEntry->MinHTPhyMode.field.MODE = MODE_CCK;
		pEntry->MinHTPhyMode.field.MCS = pEntry->MaxSupportedRate;
		pEntry->HTPhyMode.field.MODE = MODE_CCK;
		pEntry->HTPhyMode.field.MCS = pEntry->MaxSupportedRate;
	}
	else
	{
		pEntry->MaxHTPhyMode.field.MODE = MODE_OFDM;
		pEntry->MaxHTPhyMode.field.MCS = OfdmRateToRxwiMCS[pEntry->MaxSupportedRate];
		pEntry->MinHTPhyMode.field.MODE = MODE_OFDM;
		pEntry->MinHTPhyMode.field.MCS = OfdmRateToRxwiMCS[pEntry->MaxSupportedRate];
		pEntry->HTPhyMode.field.MODE = MODE_OFDM;
		pEntry->HTPhyMode.field.MCS = OfdmRateToRxwiMCS[pEntry->MaxSupportedRate];
	}
}


VOID set_sta_ra_cap(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *ent, ULONG ra_ie)
{
	CLIENT_CAP_CLEAR_FLAG(ent, fCLIENT_STATUS_RALINK_CHIPSET);
	CLIENT_CAP_CLEAR_FLAG(ent, fCLIENT_STATUS_AGGREGATION_CAPABLE);
	CLIENT_CAP_CLEAR_FLAG(ent, fCLIENT_STATUS_PIGGYBACK_CAPABLE);

	CLIENT_STATUS_CLEAR_FLAG(ent, fCLIENT_STATUS_RALINK_CHIPSET);
	CLIENT_STATUS_CLEAR_FLAG(ent, fCLIENT_STATUS_AGGREGATION_CAPABLE);
	CLIENT_STATUS_CLEAR_FLAG(ent, fCLIENT_STATUS_PIGGYBACK_CAPABLE);
	
	/* Set cap flags */
	if (ra_ie != 0x0) {
		CLIENT_CAP_SET_FLAG(ent, fCLIENT_STATUS_RALINK_CHIPSET);

#ifdef AGGREGATION_SUPPORT
		if (ra_ie & 0x00000001)
			CLIENT_CAP_SET_FLAG(ent, fCLIENT_STATUS_AGGREGATION_CAPABLE);		
#endif /* AGGREGATION_SUPPORT */

#ifdef PIGGYBACK_SUPPORT
		if (ra_ie & 0x00000002)
			CLIENT_CAP_SET_FLAG(ent, fCLIENT_STATUS_PIGGYBACK_CAPABLE);
#endif /* PIGGYBACK_SUPPORT */
	}

	/* Set operation status */
	if (ra_ie != 0x0)
		CLIENT_STATUS_SET_FLAG(ent, fCLIENT_STATUS_RALINK_CHIPSET);
	
#ifdef AGGREGATION_SUPPORT
	if ((pAd->CommonCfg.bAggregationCapable) && (ra_ie & 0x00000001))
	{
		CLIENT_STATUS_SET_FLAG(ent, fCLIENT_STATUS_AGGREGATION_CAPABLE);
		DBGPRINT(RT_DEBUG_TRACE, ("RaAggregate= 1\n"));
	}
#endif /* AGGREGATION_SUPPORT */

#ifdef PIGGYBACK_SUPPORT
	if ((pAd->CommonCfg.bPiggyBackCapable) && (ra_ie & 0x00000002))
	{
		CLIENT_STATUS_SET_FLAG(ent, fCLIENT_STATUS_PIGGYBACK_CAPABLE);
		DBGPRINT(RT_DEBUG_TRACE, ("PiggyBack= 1\n"));
	}
#endif /* PIGGYBACK_SUPPORT */

}


/*
	==========================================================================
	Description:
		Look up the MAC address in the MAC table. Return NULL if not found.
	Return:
		pEntry - pointer to the MAC entry; NULL is not found
	==========================================================================
*/
MAC_TABLE_ENTRY *MacTableLookup(RTMP_ADAPTER *pAd, UCHAR *pAddr)
{
	ULONG HashIdx;
	MAC_TABLE_ENTRY *pEntry = NULL;
	
	HashIdx = MAC_ADDR_HASH_INDEX(pAddr);
	pEntry = pAd->MacTab.Hash[HashIdx];

	while (pEntry && !IS_ENTRY_NONE(pEntry))
	{
		if (MAC_ADDR_EQUAL(pEntry->Addr, pAddr))
			break;
		else
			pEntry = pEntry->pNext;
	}

	return pEntry;
}

#ifdef WH_EZ_SETUP
MAC_TABLE_ENTRY *MacTableLookup2(RTMP_ADAPTER *pAd, UCHAR *pAddr, struct wifi_dev *wdev)
{
	ULONG HashIdx;
	MAC_TABLE_ENTRY *pEntry = NULL;

	HashIdx = MAC_ADDR_HASH_INDEX(pAddr);
	pEntry = pAd->MacTab.Hash[HashIdx];

    if (wdev){
    	while (pEntry && !IS_ENTRY_NONE(pEntry))
    	{
    		if (MAC_ADDR_EQUAL(pEntry->Addr, pAddr) && (pEntry->wdev == wdev))
    			break;
    		else
    			pEntry = pEntry->pNext;
    	}
    }
    else{
    	while (pEntry && !IS_ENTRY_NONE(pEntry))
    	{
    		if (MAC_ADDR_EQUAL(pEntry->Addr, pAddr))
    			break;
    		else
    			pEntry = pEntry->pNext;
    	}
    }
    
	return pEntry;
}
#endif



static INT mac_entry_reset(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry, BOOLEAN clean)
{
	BOOLEAN Cancelled;

	RTMPCancelTimer(&pEntry->RetryTimer, &Cancelled);
	RTMPCancelTimer(&pEntry->EnqueueStartForPSKTimer, &Cancelled);
#ifdef DOT11W_PMF_SUPPORT
	RTMPCancelTimer(&pEntry->SAQueryTimer, &Cancelled);
	RTMPCancelTimer(&pEntry->SAQueryConfirmTimer, &Cancelled);
#endif /* DOT11W_PMF_SUPPORT */
	NdisZeroMemory(pEntry, sizeof(MAC_TABLE_ENTRY));

	if (clean == TRUE)
	{
		pEntry->MaxSupportedRate = RATE_11;
		pEntry->CurrTxRate = RATE_11;
		NdisZeroMemory(pEntry, sizeof(MAC_TABLE_ENTRY));
		pEntry->PairwiseKey.KeyLen = 0;
		pEntry->PairwiseKey.CipherAlg = CIPHER_NONE;
	}

	return 0;
}


MAC_TABLE_ENTRY *MacTableInsertEntry(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR *pAddr,
	IN struct wifi_dev *wdev,
	IN UINT32 ent_type,
	IN UCHAR OpMode,
	IN BOOLEAN CleanAll)
{
	UCHAR HashIdx;
	int i, FirstWcid;
	MAC_TABLE_ENTRY *pEntry = NULL, *pCurrEntry;
	STA_TR_ENTRY *tr_entry = NULL;
#if defined(APCLI_SAE_SUPPORT) || defined(APCLI_OWE_SUPPORT)
	MAC_TABLE_ENTRY *pre_mac_entry = NULL;
#endif
	if (pAd->MacTab.Size >= MAX_LEN_OF_MAC_TABLE)
		return NULL;

	FirstWcid = 1;


	/* allocate one MAC entry*/
	NdisAcquireSpinLock(&pAd->MacTabLock);
	for (i = FirstWcid; i< MAX_LEN_OF_MAC_TABLE; i++)   /* skip entry#0 so that "entry index == AID" for fast lookup*/
	{
		/* pick up the first available vacancy*/
		if (IS_ENTRY_NONE(&pAd->MacTab.Content[i]))
		{
			pEntry = &pAd->MacTab.Content[i];
			tr_entry = &pAd->MacTab.tr_entry[i];

			mac_entry_reset(pAd, pEntry, CleanAll);
			
			/* ENTRY PREEMPTION: initialize the entry */
			pEntry->wdev = wdev;
			pEntry->wcid = i;
			pEntry->func_tb_idx = wdev->func_idx;
			pEntry->bIAmBadAtheros = FALSE;
			pEntry->pAd = pAd;
			pEntry->CMTimerRunning = FALSE;
#ifdef ANDLINK_FEATURE_SUPPORT
			ANDLINK_GET_CURRENT_SEC(&pEntry->upTime);
			DBGPRINT(RT_DEBUG_OFF, ("%s: Set entry UpTime %llu(s)<-------\n\n", __FUNCTION__, 
				(ULONGLONG)pEntry->upTime));
#endif
#ifdef FAST_DETECT_STA_OFF
			pEntry->detect_deauth = FALSE;
#endif
			COPY_MAC_ADDR(pEntry->Addr, pAddr);
			pEntry->Sst = SST_NOT_AUTH;
			pEntry->AuthState = AS_NOT_AUTH;
			pEntry->Aid = (USHORT)i;
			pEntry->CapabilityInfo = 0;
			pEntry->AssocDeadLine = MAC_TABLE_ASSOC_TIMEOUT;
			
			pEntry->PsMode = PWR_ACTIVE;
			pEntry->NoDataIdleCount = 0;
			pEntry->ContinueTxFailCnt = 0;
#ifdef WDS_SUPPORT
			pEntry->LockEntryTx = FALSE;
#endif /* WDS_SUPPORT */
			pEntry->TimeStamp_toTxRing = 0;
			// TODO: shiang-usw,  remove upper setting becasue we need to migrate to tr_entry!
			pAd->MacTab.tr_entry[i].PsMode = PWR_ACTIVE;
			pAd->MacTab.tr_entry[i].NoDataIdleCount = 0;
			pAd->MacTab.tr_entry[i].ContinueTxFailCnt = 0;
			pAd->MacTab.tr_entry[i].LockEntryTx = FALSE;
			pAd->MacTab.tr_entry[i].TimeStamp_toTxRing = 0;
			pAd->MacTab.tr_entry[i].PsDeQWaitCnt = 0;

			pEntry->EnqueueEapolStartTimerRunning = EAPOL_START_DISABLE;
			pEntry->GTKState = REKEY_NEGOTIATING;
			pEntry->PairwiseKey.KeyLen = 0;
			pEntry->PairwiseKey.CipherAlg = CIPHER_NONE;
			pEntry->PMKID_CacheIdx = ENTRY_NOT_FOUND;
			pEntry->RSNIE_Len = 0;
			NdisZeroMemory(pEntry->R_Counter, sizeof(pEntry->R_Counter));
			pEntry->ReTryCounter = PEER_MSG1_RETRY_TIMER_CTR;
			tr_entry->PortSecured = WPA_802_1X_PORT_NOT_SECURED;

			do
			{

#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
				if (ent_type == ENTRY_APCLI)
				{
					SET_ENTRY_APCLI(pEntry);
					//SET_ENTRY_AP(pEntry);//Carter, why set entry to APCLI then set to AP????
					COPY_MAC_ADDR(pEntry->bssid, pAddr);

#if defined(APCLI_SAE_SUPPORT) || defined(APCLI_OWE_SUPPORT)
					pre_mac_entry =
						&pAd->ApCfg.ApCliTab[pEntry->func_tb_idx].pre_mac_entry;

					if (pAd->ApCfg.ApCliTab[pEntry->func_tb_idx].wdev.AuthMode
						== Ndis802_11AuthModeOWE ||
						pAd->ApCfg.ApCliTab[pEntry->func_tb_idx].wdev.AuthMode
						== Ndis802_11AuthModeWPA3PSK) {
						pEntry->AuthMode = pre_mac_entry->AuthMode;
						pEntry->WepStatus = pre_mac_entry->WepStatus;
#ifdef APCLI_SAE_SUPPORT
						if (pAd->ApCfg.ApCliTab[pEntry->func_tb_idx].wdev.AuthMode
							== Ndis802_11AuthModeWPA3PSK)
							NdisMoveMemory(pEntry->PMK, pre_mac_entry->PMK, LEN_PMK);
#endif
						pEntry->pmk_cache = pre_mac_entry->pmk_cache;
						pEntry->pmkid = pre_mac_entry->pmkid;
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
						pEntry->AuthMode =
							pAd->ApCfg.ApCliTab[pEntry->func_tb_idx].wdev.AuthMode;
						pEntry->WepStatus =
							pAd->ApCfg.ApCliTab[pEntry->func_tb_idx].wdev.WepStatus;
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
#ifdef ROAMING_ENHANCE_SUPPORT
#ifdef APCLI_SUPPORT
						pEntry->bRoamingRefreshDone = FALSE;
#endif /* APCLI_SUPPORT */
#endif /* ROAMING_ENHANCE_SUPPORT */
					break;
				}
#endif /* APCLI_SUPPORT */
#ifdef WDS_SUPPORT
				if (ent_type == ENTRY_WDS)
				{
					SET_ENTRY_WDS(pEntry);
					COPY_MAC_ADDR(pEntry->bssid, pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev.bssid);
					pEntry->AuthMode = Ndis802_11AuthModeOpen;
					pEntry->WepStatus = Ndis802_11EncryptionDisabled;
					break;
				}
#endif /* WDS_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
				if (ent_type == ENTRY_CLIENT)
				{	/* be a regular-entry*/
					if ((pEntry->func_tb_idx < pAd->ApCfg.BssidNum) &&
						(pEntry->func_tb_idx < MAX_MBSSID_NUM(pAd)) &&
						((pEntry->func_tb_idx < HW_BEACON_MAX_NUM)) &&
						(pAd->ApCfg.MBSSID[pEntry->func_tb_idx].MaxStaNum != 0) &&
						(pAd->ApCfg.MBSSID[pEntry->func_tb_idx].StaCount >= pAd->ApCfg.MBSSID[pEntry->func_tb_idx].MaxStaNum))
					{
						DBGPRINT(RT_DEBUG_WARN, ("%s: The connection table is full in ra%d.\n", __FUNCTION__, pEntry->func_tb_idx));
						NdisReleaseSpinLock(&pAd->MacTabLock);
						return NULL;
					}
					

					if (pEntry->func_tb_idx >= HW_BEACON_MAX_NUM)
						return NULL;

					ASSERT((wdev == &pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev));

					SET_ENTRY_CLIENT(pEntry);
					pEntry->pMbss = &pAd->ApCfg.MBSSID[pEntry->func_tb_idx];

					MBSS_MR_APIDX_SANITY_CHECK(pAd, pEntry->func_tb_idx);
					COPY_MAC_ADDR(pEntry->bssid, wdev->bssid);
					pEntry->AuthMode = wdev->AuthMode;
					pEntry->WepStatus = wdev->WepStatus;
					pEntry->GroupKeyWepStatus = wdev->GroupKeyWepStatus;

					if (pEntry->AuthMode < Ndis802_11AuthModeWPA)
						pEntry->WpaState = AS_NOTUSE;
					else
						pEntry->WpaState = AS_INITIALIZE;

					pEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;
					pEntry->StaIdleTimeout = pAd->ApCfg.StaIdleTimeout;
					pAd->ApCfg.MBSSID[pEntry->func_tb_idx].StaCount++;
					pAd->ApCfg.EntryClientCount++;

#ifdef WH_EVENT_NOTIFIER
					pEntry->tx_state.CurrentState = WHC_STA_STATE_ACTIVE;
					pEntry->rx_state.CurrentState = WHC_STA_STATE_ACTIVE;
#endif /* WH_EVENT_NOTIFIER */

#if (defined(WH_EZ_SETUP) && defined(EZ_DUAL_BAND_SUPPORT))
					pEntry->link_duplicate = FALSE;
#endif
					break;
				}
#ifdef AIR_MONITOR
				else if (ent_type == ENTRY_MONITOR)
				{
					SET_ENTRY_MONITOR(pEntry);
					break;
				}
#endif /* AIR_MONITOR */	
				
#endif /* CONFIG_AP_SUPPORT */

			} while (FALSE);

			tr_tb_set_entry(pAd, i, pEntry);
#ifdef DATA_QUEUE_RESERVE
			tr_entry->high_pkt_cnt = 0;
			tr_entry->high_pkt_drop_cnt = 0;
#endif /* DATA_QUEUE_RESERVE */
			
			RTMPInitTimer(pAd, &pEntry->EnqueueStartForPSKTimer, GET_TIMER_FUNCTION(EnqueueStartForPSKExec), pEntry, FALSE);


#ifdef CONFIG_AP_SUPPORT
			{
				if (IS_ENTRY_CLIENT(pEntry)) /* Only Client entry need the retry timer.*/
				{
					RTMPInitTimer(pAd, &pEntry->RetryTimer, GET_TIMER_FUNCTION(WPARetryExec), pEntry, FALSE);
#ifdef DOT11W_PMF_SUPPORT
					RTMPInitTimer(pAd, &pEntry->SAQueryTimer, GET_TIMER_FUNCTION(PMF_SAQueryTimeOut), pEntry, FALSE);
					RTMPInitTimer(pAd, &pEntry->SAQueryConfirmTimer, GET_TIMER_FUNCTION(PMF_SAQueryConfirmTimeOut), pEntry, FALSE);
#endif /* DOT11W_PMF_SUPPORT */

#if defined(MT_MAC) && defined(WSC_INCLUDED) && defined(CONFIG_AP_SUPPORT)
					RTMPInitTimer(pAd, &pEntry->EapReqIdRetryTimer, GET_TIMER_FUNCTION(WscEapReqIdRetryTimeout), pEntry, TRUE);
					pEntry->bEapReqIdRetryTimerRunning = FALSE;
#endif /* defined(MT_MAC) && defined(WSC_INCLUDED) && defined(CONFIG_AP_SUPPORT) */

				}

#ifdef APCLI_SUPPORT
				if (IS_ENTRY_APCLI(pEntry)) {
					RTMPInitTimer(pAd, &pEntry->RetryTimer, GET_TIMER_FUNCTION(WPARetryExec), pEntry, FALSE);
#ifdef APCLI_DOT11W_PMF_SUPPORT
#ifdef DOT11W_PMF_SUPPORT
					RTMPInitTimer(pAd, &pEntry->SAQueryTimer,
						GET_TIMER_FUNCTION(PMF_SAQueryTimeOut), pEntry, FALSE);
					RTMPInitTimer(pAd, &pEntry->SAQueryConfirmTimer,
						GET_TIMER_FUNCTION(PMF_SAQueryConfirmTimeOut), pEntry, FALSE);
#endif /* DOT11W_PMF_SUPPORT */
#endif /* APCLI_DOT11W_PMF_SUPPORT */
				}
#endif /* APCLI_SUPPORT */
			}
#endif /* CONFIG_AP_SUPPORT */



#ifdef STREAM_MODE_SUPPORT
			/* Enable Stream mode for first three entries in MAC table */

#endif /* STREAM_MODE_SUPPORT */

#ifdef UAPSD_SUPPORT
			/* Ralink WDS doesn't support any power saving.*/
			if (IS_ENTRY_CLIENT(pEntry)
#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
				|| IS_ENTRY_TDLS(pEntry)
#endif /* defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) */
			)
			{
				/* init U-APSD enhancement related parameters */
			DBGPRINT(RT_DEBUG_TRACE, ("%s(): INIT UAPSD MR ENTRY",__FUNCTION__));
				UAPSD_MR_ENTRY_INIT(pEntry);
			}
#endif /* UAPSD_SUPPORT */

			pAd->MacTab.Size ++;

			/* Set the security mode of this entry as OPEN-NONE in ASIC */
			RTMP_REMOVE_PAIRWISE_KEY_ENTRY(pAd, (UCHAR)i);
#ifdef MT_MAC										
			if (pAd->chipCap.hif_type == HIF_MT)
				MT_ADDREMOVE_KEY(pAd, 1, pEntry->func_tb_idx, 0, pEntry->wcid, PAIRWISEKEYTABLE, &pEntry->PairwiseKey, pEntry->Addr);
#endif	

			/* Add this entry into ASIC RX WCID search table */
			RTMP_STA_ENTRY_ADD(pAd, pEntry);

#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
			{
#ifdef WSC_AP_SUPPORT
				pEntry->bWscCapable = FALSE;
				pEntry->Receive_EapolStart_EapRspId = 0;
#endif /* WSC_AP_SUPPORT */
			}
#endif /* CONFIG_AP_SUPPORT */

			DBGPRINT(RT_DEBUG_TRACE, ("%s(): alloc entry #%d, Total= %d\n",
						__FUNCTION__, i, pAd->MacTab.Size));
			break;
		}
	}

	/* add this MAC entry into HASH table */
	if (pEntry)
	{
		HashIdx = MAC_ADDR_HASH_INDEX(pAddr);
		if (pAd->MacTab.Hash[HashIdx] == NULL)
			pAd->MacTab.Hash[HashIdx] = pEntry;
		else
		{
			pCurrEntry = pAd->MacTab.Hash[HashIdx];
			while (pCurrEntry->pNext != NULL)
				pCurrEntry = pCurrEntry->pNext;
			pCurrEntry->pNext = pEntry;
		}

#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
#ifdef DOT11V_WNM_SUPPORT
			pEntry->Beclone = FALSE;
			pEntry->bBSSMantSTASupport = FALSE;
			pEntry->bDMSSTASupport = FALSE;
#endif /* DOT11V_WNM_SUPPORT */

#ifdef WSC_AP_SUPPORT
			if (IS_ENTRY_CLIENT(pEntry) &&
				(pEntry->func_tb_idx < pAd->ApCfg.BssidNum) &&
				MAC_ADDR_EQUAL(pEntry->Addr, pAd->ApCfg.MBSSID[pEntry->func_tb_idx].WscControl.EntryAddr))
			{
				NdisZeroMemory(pAd->ApCfg.MBSSID[pEntry->func_tb_idx].WscControl.EntryAddr, MAC_ADDR_LEN);
			}
#endif /* WSC_AP_SUPPORT */

#if (MT7615_MT7603_COMBO_FORWARDING == 1)
#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)
		if(IS_ENTRY_CLIENT(pEntry) && 
		   wf_fwd_add_entry_inform_hook)
		{
		   wf_fwd_add_entry_inform_hook(pEntry->Addr);
		}
#endif /* CONFIG_WIFI_PKT_FWD */
		
#endif /* CONFIG_AP_SUPPORT */

#ifdef MTFWD
		if (IS_ENTRY_CLIENT(pEntry)) {
			DBGPRINT(RT_DEBUG_TRACE, ("New Sta:%pM\n", pEntry->Addr));
			RtmpOSWrielessEventSend(pEntry->wdev->if_dev,
						RT_WLAN_EVENT_CUSTOM,
						FWD_CMD_ADD_TX_SRC,
						NULL,
						(PUCHAR)pEntry->Addr,
						MAC_ADDR_LEN);
		}
#endif
	}
#endif

	}

	NdisReleaseSpinLock(&pAd->MacTabLock);
	
	/*update tx burst, must after unlock pAd->MacTabLock*/
	rtmp_tx_burst_set(pAd);
	return pEntry;
}


INT mac_tb_del_from_hash(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry)
{
	USHORT HashIdx;
	MAC_TABLE_ENTRY  *pPrevEntry, *pProbeEntry;

	HashIdx = MAC_ADDR_HASH_INDEX(pEntry->Addr);
	pPrevEntry = NULL;
	pProbeEntry = pAd->MacTab.Hash[HashIdx];
	ASSERT(pProbeEntry);

	/* update Hash list*/
	while(pProbeEntry)
	{
		if (pProbeEntry == pEntry)
		{
			if (pPrevEntry == NULL)
				pAd->MacTab.Hash[HashIdx] = pEntry->pNext;
			else
				pPrevEntry->pNext = pEntry->pNext;
			break;
		}

		pPrevEntry = pProbeEntry;
		pProbeEntry = pProbeEntry->pNext;
	};

	ASSERT(pProbeEntry != NULL);
			
	return TRUE;
}


/*
	==========================================================================
	Description:
		Delete a specified client from MAC table
	==========================================================================
 */
BOOLEAN MacTableDeleteEntry(RTMP_ADAPTER *pAd, USHORT wcid, UCHAR *pAddr)
{
	MAC_TABLE_ENTRY *pEntry;
	STA_TR_ENTRY *tr_entry;
	BOOLEAN Cancelled;
#if defined(MBO_SUPPORT)
	UCHAR TmpAddrForIndicate[MAC_ADDR_LEN] = {0};
	BOOLEAN bIndicateSendEvent = FALSE;
#endif/* defined(MBO_SUPPORT) */
	struct wifi_dev *wdev = NULL;

	if (wcid >= MAX_LEN_OF_MAC_TABLE)
		return FALSE;

	NdisAcquireSpinLock(&pAd->MacTabLock);

	pEntry = &pAd->MacTab.Content[wcid];
	tr_entry = &pAd->MacTab.tr_entry[wcid];

	if (pEntry && !IS_ENTRY_NONE(pEntry))
	{
#ifdef BAND_STEERING
		if((pAd->ApCfg.BandSteering) && IS_ENTRY_CLIENT(pEntry) && IS_VALID_MAC(pEntry->Addr))
			BndStrg_UpdateEntry(pAd, pEntry, NULL, FALSE);
#endif
#ifdef CONFIG_WIFI_PKT_FWD
		if(wf_fwd_delete_entry_inform_hook)
			wf_fwd_delete_entry_inform_hook(pEntry->Addr);
#endif /* CONFIG_WIFI_PKT_FWD */

#ifdef MTFWD
		DBGPRINT(RT_DEBUG_TRACE, ("Del Sta:%pM\n", pEntry->Addr));
		RtmpOSWrielessEventSend(pEntry->wdev->if_dev,
					RT_WLAN_EVENT_CUSTOM,
					FWD_CMD_DEL_TX_SRC,
					NULL,
					(PUCHAR)pEntry->Addr,
					MAC_ADDR_LEN);
#endif

#ifdef MWDS
		MWDSAPPeerDisable(pAd, pEntry);
#endif /* CONFIG_WIFI_PKT_FWD */

#ifdef MT_PS
		MtPsRedirectDisableCheck(pAd, wcid);
		tr_entry->ps_state = APPS_RETRIEVE_IDLE;
		OS_WAIT(2); /* Wait FW command arriving at MCU. */
#endif /* MT_PS */
#if defined(MAP_SUPPORT) && defined(A4_CONN)
		map_a4_peer_disable(pAd, pEntry, TRUE);
#endif
		/*get wdev*/
		wdev = pEntry->wdev;

#ifdef CONFIG_AP_SUPPORT
#ifdef MT_MAC
#ifdef WSC_INCLUDED
/* Try to always cancel all timer inside MacTableDeleteEntry.
 * Required to prevent re-init for this timer when already
 * being in active state. 
 */
/*		if (pEntry->bEapReqIdRetryTimerRunning)
		{
*/
		RTMPCancelTimer(&pEntry->EapReqIdRetryTimer, &Cancelled);
			pEntry->bEapReqIdRetryTimerRunning = FALSE;
/*
		}
*/
		#endif /* WSC_INCLUDED */
#endif /* MT_MAC */
#ifdef STA_FORCE_ROAM_SUPPORT

		if(((PRTMP_ADAPTER)(pEntry->wdev->sys_handle))->en_force_roam_supp && IS_ENTRY_CLIENT(pEntry)
#ifdef WH_EZ_SETUP
			&& (IS_EZ_SETUP_ENABLED(pEntry->wdev) && !pEntry->easy_setup_enabled)
#endif		
		){
			if(pEntry->low_rssi_notified){
				DBGPRINT(RT_DEBUG_OFF, 
					("MacTableDeleteEntry: Notify to ForceRoam App \n"));
				froam_notify_sta_disconnect(pAd, pEntry);
			}
		}
#endif

#ifdef WH_EVENT_NOTIFIER
		{
			EventHdlr pEventHdlrHook = NULL;
			pEventHdlrHook = GetEventNotiferHook(WHC_DRVEVNT_STA_LEAVE);
			if(pEventHdlrHook && pEntry->wdev)
				pEventHdlrHook(pAd, pEntry->wdev, pAddr, pAd->CommonCfg.Channel);
		}
#endif /* WH_EVENT_NOTIFIER */

		WLAN_MR_TIM_BIT_CLEAR(pAd, pEntry->func_tb_idx, pEntry->Aid);
#endif /* CONFIG_AP_SUPPORT */

		/* ENTRY PREEMPTION: Cancel all timers */
		RTMPCancelTimer(&pEntry->RetryTimer, &Cancelled);
		RTMPCancelTimer(&pEntry->EnqueueStartForPSKTimer, &Cancelled);
#ifdef DOT11W_PMF_SUPPORT
		RTMPCancelTimer(&pEntry->SAQueryTimer, &Cancelled);
		RTMPCancelTimer(&pEntry->SAQueryConfirmTimer, &Cancelled);
#endif /* DOT11W_PMF_SUPPORT */

		if (MAC_ADDR_EQUAL(pEntry->Addr, pAddr))
		{

#ifdef WH_EZ_SETUP
			if (IS_EZ_SETUP_ENABLED(pEntry->wdev)) {
				pEntry->is_apcli = FALSE;

#if defined (NEW_CONNECTION_ALGO) || defined (EZ_NETWORK_MERGE_SUPPORT)
				ez_handle_peer_disconnection(pEntry->wdev, pEntry->Addr);		
#endif
				pEntry->easy_setup_enabled = FALSE;
			}
#endif /* WH_EZ_SETUP */
		

#if defined(CONFIG_AP_SUPPORT) && defined(CONFIG_DOT11V_WNM)
			if (pAd->ApCfg.MBSSID[pEntry->func_tb_idx].WNMCtrl.ProxyARPEnable)
			{
				RemoveIPv4ProxyARPEntry(pAd, &pAd->ApCfg.MBSSID[pEntry->func_tb_idx], pEntry->Addr);
				RemoveIPv6ProxyARPEntry(pAd, &pAd->ApCfg.MBSSID[pEntry->func_tb_idx], pEntry->Addr);
			}
#ifdef CONFIG_HOTSPOT_R2
			pEntry->IsKeep = 0;
#endif /* CONFIG_HOTSPOT_R2 */
#endif
#ifdef DOT11V_WNM_SUPPORT
#ifdef CONFIG_AP_SUPPORT
			DeleteDMSEntry(pAd, pEntry);
#endif /* CONFIG_AP_SUPPORT */
#endif /* DOT11V_WNM_SUPPORT */

#ifdef DOT11_N_SUPPORT
			/* free resources of BA*/
			BASessionTearDownALL(pAd, pEntry->wcid);
#endif /* DOT11_N_SUPPORT */

			/* Delete this entry from ASIC on-chip WCID Table*/
			RTMP_STA_ENTRY_MAC_RESET(pAd, wcid);


#ifdef STREAM_MODE_SUPPORT
			/* Clear Stream Mode register for this client */
			if (pEntry->StreamModeMACReg != 0)
				RTMP_IO_WRITE32(pAd, pEntry->StreamModeMACReg+4, 0);
#endif // STREAM_MODE_SUPPORT //

#if defined(MT_MAC) && defined(WSC_INCLUDED) && defined(CONFIG_AP_SUPPORT)
			RTMPReleaseTimer(&pEntry->EapReqIdRetryTimer, &Cancelled);
#endif /* defined(MT_MAC) && defined(WSC_INCLUDED) && defined(CONFIG_AP_SUPPORT) */

#ifdef DOT11W_PMF_SUPPORT
			RTMPReleaseTimer(&pEntry->SAQueryTimer, &Cancelled);
			RTMPReleaseTimer(&pEntry->SAQueryConfirmTimer, &Cancelled);
#endif /* DOT11W_PMF_SUPPORT */


#ifdef CONFIG_AP_SUPPORT
			if (IS_ENTRY_CLIENT(pEntry)
			)
			{
#ifdef DOT1X_SUPPORT 
				INT PmkCacheIdx = -1;
#endif /* DOT1X_SUPPORT */

				RTMPReleaseTimer(&pEntry->RetryTimer, &Cancelled);

#ifdef DOT1X_SUPPORT    
				/* Notify 802.1x daemon to clear this sta info*/
				if (pEntry->AuthMode == Ndis802_11AuthModeWPA || 
					pEntry->AuthMode == Ndis802_11AuthModeWPA2 ||
					pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.IEEE8021X)
					DOT1X_InternalCmdAction(pAd, pEntry, DOT1X_DISCONNECT_ENTRY);

				/* Delete the PMK cache for this entry if it exists.*/
				PmkCacheIdx = RTMPSearchPMKIDCache(pAd, pEntry->func_tb_idx, pEntry->Addr);
				if (PmkCacheIdx != -1
#ifdef DOT11_SAE_SUPPORT
					&& pEntry->AuthMode != Ndis802_11AuthModeWPA3PSK
#endif
#ifdef CONFIG_OWE_SUPPORT
					&& pEntry->AuthMode != Ndis802_11AuthModeOWE
#endif
					)
				{
					RTMPDeletePMKIDCache(pAd, pEntry->func_tb_idx, PmkCacheIdx);
				}
#endif /* DOT1X_SUPPORT */

#ifdef WAPI_SUPPORT
				RTMPCancelWapiRekeyTimerAction(pAd, pEntry);
#endif /* WAPI_SUPPORT */

#ifdef IGMP_SNOOP_SUPPORT
				IgmpGroupDelMembers(pAd, (PUCHAR)pEntry->Addr, pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.if_dev);
#endif /* IGMP_SNOOP_SUPPORT */
				pAd->ApCfg.MBSSID[pEntry->func_tb_idx].StaCount--;
				pAd->ApCfg.EntryClientCount--;

#ifdef HOSTAPD_SUPPORT
				if(pEntry && pAd->ApCfg.MBSSID[pEntry->func_tb_idx].Hostapd == Hostapd_EXT )
				{
					RtmpOSWrielessEventSendExt(pAd->net_dev, RT_WLAN_EVENT_EXPIRED, -1, pEntry->Addr,
												NULL, 0,pEntry->func_tb_idx);
				}
#endif /* HOSTAPD_SUPPORT */

			}
#ifdef APCLI_SUPPORT
			else if (IS_ENTRY_APCLI(pEntry))
			{
				RTMPReleaseTimer(&pEntry->RetryTimer, &Cancelled);
			}
#endif /* APCLI_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

			mac_tb_del_from_hash(pAd, pEntry);

#ifdef CONFIG_AP_SUPPORT
			APCleanupPsQueue(pAd, &tr_entry->ps_queue); /* return all NDIS packet in PSQ*/
#endif /* CONFIG_AP_SUPPORT */

			tr_tb_reset_entry(pAd, wcid);

			/*RTMP_REMOVE_PAIRWISE_KEY_ENTRY(pAd, wcid);*/

#ifdef UAPSD_SUPPORT
#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
			hex_dump("mac=", pEntry->Addr, 6);
			UAPSD_MR_ENTRY_RESET(pAd, pEntry);
#else
#ifdef CONFIG_AP_SUPPORT
            UAPSD_MR_ENTRY_RESET(pAd, pEntry);
#endif /* CONFIG_AP_SUPPORT */
#endif /* defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) */
#endif /* UAPSD_SUPPORT */

			if (pEntry->EnqueueEapolStartTimerRunning != EAPOL_START_DISABLE)
			{
				RTMPCancelTimer(&pEntry->EnqueueStartForPSKTimer, &Cancelled);
				pEntry->EnqueueEapolStartTimerRunning = EAPOL_START_DISABLE;
			}
			RTMPReleaseTimer(&pEntry->EnqueueStartForPSKTimer, &Cancelled);


#ifdef CONFIG_AP_SUPPORT
#ifdef WSC_AP_SUPPORT
			if (IS_ENTRY_CLIENT(pEntry))
			{
				PWSC_CTRL	pWscControl = &pAd->ApCfg.MBSSID[pEntry->func_tb_idx].WscControl;
				if (MAC_ADDR_EQUAL(pEntry->Addr, pWscControl->EntryAddr))
				{
					/*
					Some WPS Client will send dis-assoc close to WSC_DONE. 
					If AP misses WSC_DONE, WPS Client still sends dis-assoc to AP.
					Do not cancel timer if WscState is WSC_STATE_WAIT_DONE.
					*/
					if ((pWscControl->EapolTimerRunning == TRUE) &&
						(pWscControl->WscState != WSC_STATE_WAIT_DONE))
					{
						RTMPCancelTimer(&pWscControl->EapolTimer, &Cancelled);
						pWscControl->EapolTimerRunning = FALSE;
						pWscControl->EapMsgRunning = FALSE;
						NdisZeroMemory(&(pWscControl->EntryAddr[0]), MAC_ADDR_LEN);
					}
				}
				pEntry->Receive_EapolStart_EapRspId = 0;
				pEntry->bWscCapable = FALSE;
#ifdef WH_EVENT_NOTIFIER
				pEntry->tx_state.CurrentState = WHC_STA_STATE_IDLE;
				pEntry->rx_state.CurrentState = WHC_STA_STATE_IDLE;
#endif /* WH_EVENT_NOTIFIER */
			}
#endif /* WSC_AP_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

#if defined(MBO_SUPPORT)
			COPY_MAC_ADDR(TmpAddrForIndicate, pEntry->Addr);
			bIndicateSendEvent = TRUE;
#endif/* defined(RT_CFG80211_SUPPORT) || defined(MBO_SUPPORT) */
#ifdef WH_EZ_SETUP	// Fix to avoid transmitting frames with an all-zero MAC address
			if (!IS_EZ_SETUP_ENABLED(pEntry->wdev))
			{
#endif
				//	NdisZeroMemory(pEntry, sizeof(MAC_TABLE_ENTRY));
				NdisZeroMemory(pEntry->Addr, MAC_ADDR_LEN);
#ifdef WH_EZ_SETUP
			}
#endif
#ifdef FAST_DETECT_STA_OFF
			NdisZeroMemory(&pEntry->ConCounters, sizeof(struct _COUNTER_CON));
#endif
#ifdef WAPP_SUPPORT
			if ((wdev != NULL) && IS_ENTRY_CLIENT(pEntry))
				wapp_send_cli_leave_event(pAd, RtmpOsGetNetIfIndex(wdev->if_dev), pAddr);
#endif/* WAPP_SUPPORT */

			/* invalidate the entry */
			tr_entry->PortSecured = WPA_802_1X_PORT_NOT_SECURED;			
			SET_ENTRY_NONE(pEntry);
			

			pAd->MacTab.Size--;

			DBGPRINT(RT_DEBUG_TRACE, ("MacTableDeleteEntry1 - Total= %d\n", pAd->MacTab.Size));
#ifdef R1KH_HARD_RETRY
				RTMP_OS_EXIT_COMPLETION(&pEntry->ack_r1kh);
#endif /* R1KH_HARD_RETRY */
		}
		else
		{
			DBGPRINT(RT_DEBUG_OFF, ("\n%s: Impossible Wcid = %d !!!!!\n", __FUNCTION__, wcid));
#if defined(MBO_SUPPORT)
			bIndicateSendEvent = FALSE;
#endif/*defined(MBO_SUPPORT) */
		}
	}

	NdisReleaseSpinLock(&pAd->MacTabLock);
	/*Update TX burst, must after unlock pAd->MacTabLock*/
	rtmp_tx_burst_set(pAd);

	/*Reset operating mode when no Sta.*/
	if (pAd->MacTab.Size == 0)
	{
#ifdef DOT11_N_SUPPORT
		pAd->CommonCfg.AddHTInfo.AddHtInfo2.OperaionMode = 0;
#endif /* DOT11_N_SUPPORT */
		RTMP_UPDATE_PROTECT(pAd, 0, ALLN_SETPROTECT, TRUE, 0);
	}
#ifdef MBO_SUPPORT
	/* mbo - indicate daemon to remve this sta */
	{
		MBO_EVENT_STA_DISASSOC evt_sta_disassoc;

		COPY_MAC_ADDR(evt_sta_disassoc.mac_addr, TmpAddrForIndicate);
		MboIndicateStaDisassocToDaemon(pAd, &evt_sta_disassoc, MBO_MSG_REMOVE_STA);
	}
#endif/* MBO_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
	/*APUpdateCapabilityAndErpIe(pAd);*/
	RTMP_AP_UPDATE_CAPABILITY_AND_ERPIE(pAd);  /* edit by johnli, fix "in_interrupt" error when call "MacTableDeleteEntry" in Rx tasklet*/
#endif /* CONFIG_AP_SUPPORT */

	return TRUE;
}


/*
	==========================================================================
	Description:
		This routine reset the entire MAC table. All packets pending in
		the power-saving queues are freed here.
	==========================================================================
 */
VOID MacTableReset(RTMP_ADAPTER *pAd, INT startWcid)
{
	int i;
	BOOLEAN Cancelled;    
#ifdef CONFIG_AP_SUPPORT
#ifdef RTMP_MAC_PCI
	unsigned long	IrqFlags=0;
#endif /* RTMP_MAC_PCI */
	UCHAR *pOutBuffer = NULL;
	NDIS_STATUS NStatus;
	ULONG FrameLen = 0;
	HEADER_802_11 DeAuthHdr;
	USHORT Reason;
	UCHAR apidx = MAIN_MBSSID;
#endif /* CONFIG_AP_SUPPORT */
	MAC_TABLE_ENTRY *pMacEntry;

	DBGPRINT(RT_DEBUG_TRACE, ("MacTableReset\n"));
	/*NdisAcquireSpinLock(&pAd->MacTabLock);*/

	
	if (startWcid <= 0)
		startWcid = 1;

	for (i=startWcid; i < MAX_LEN_OF_MAC_TABLE; i++)
	{
		pMacEntry = &pAd->MacTab.Content[i];
		if (IS_ENTRY_CLIENT(pMacEntry))
		{
			RTMPReleaseTimer(&pMacEntry->EnqueueStartForPSKTimer, &Cancelled);
			pMacEntry->EnqueueEapolStartTimerRunning = EAPOL_START_DISABLE;

#ifdef CONFIG_AP_SUPPORT
			//IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
			{
				/* Before reset MacTable, send disassociation packet to client.*/
				if (pMacEntry->Sst == SST_ASSOC)
				{
					/*  send out a De-authentication request frame*/
					NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);
					if (NStatus != NDIS_STATUS_SUCCESS)
					{
						DBGPRINT(RT_DEBUG_TRACE, (" MlmeAllocateMemory fail  ..\n"));
						/*NdisReleaseSpinLock(&pAd->MacTabLock);*/
						return;
					}

					Reason = REASON_NO_LONGER_VALID;
#ifdef WIFI_DIAG
					if (IS_ENTRY_CLIENT(pMacEntry))
						DiagConnError(pAd, pMacEntry->func_tb_idx, pMacEntry->Addr,
							DIAG_CONN_DEAUTH, Reason);
#endif
#ifdef CONN_FAIL_EVENT
					if (IS_ENTRY_CLIENT(pMacEntry))
						ApSendConnFailMsg(pAd,
							pAd->ApCfg.MBSSID[pMacEntry->func_tb_idx].Ssid,
							pAd->ApCfg.MBSSID[pMacEntry->func_tb_idx].SsidLen,
							pMacEntry->Addr,
							Reason);
#endif
					DBGPRINT(RT_DEBUG_OFF, ("Send DeAuth (Reason=%d) to %02x:%02x:%02x:%02x:%02x:%02x\n",
								Reason, PRINT_MAC(pMacEntry->Addr)));
					MgtMacHeaderInit(pAd, &DeAuthHdr, SUBTYPE_DEAUTH, 0, pMacEntry->Addr, 
										pAd->ApCfg.MBSSID[pMacEntry->func_tb_idx].wdev.if_addr,
										pAd->ApCfg.MBSSID[pMacEntry->func_tb_idx].wdev.bssid);
					MakeOutgoingFrame(pOutBuffer, &FrameLen,
										sizeof(HEADER_802_11), &DeAuthHdr,
										2, &Reason,
										END_OF_ARGS);

					MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
					MlmeFreeMemory(pAd, pOutBuffer);
					RtmpusecDelay(5000);
				}
			}
#endif /* CONFIG_AP_SUPPORT */
		}

		/* Delete a entry via WCID */
		MacTableDeleteEntry(pAd, i, pMacEntry->Addr);
	}

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		for (apidx = MAIN_MBSSID; apidx < pAd->ApCfg.BssidNum; apidx++)
		{
#ifdef WSC_AP_SUPPORT
			BOOLEAN Cancelled;
			
			RTMPCancelTimer(&pAd->ApCfg.MBSSID[apidx].WscControl.EapolTimer, &Cancelled);
			pAd->ApCfg.MBSSID[apidx].WscControl.EapolTimerRunning = FALSE;
			NdisZeroMemory(pAd->ApCfg.MBSSID[apidx].WscControl.EntryAddr, MAC_ADDR_LEN);
			pAd->ApCfg.MBSSID[apidx].WscControl.EapMsgRunning = FALSE;
#endif /* WSC_AP_SUPPORT */
			pAd->ApCfg.MBSSID[apidx].StaCount = 0; 
		}
#ifdef RTMP_MAC_PCI
		RTMP_IRQ_LOCK(&pAd->irq_lock, IrqFlags);
#endif /* RTMP_MAC_PCI */
		DBGPRINT(RT_DEBUG_TRACE, ("McastPsQueue.Number %u...\n", pAd->MacTab.McastPsQueue.Number));
		if (pAd->MacTab.McastPsQueue.Number > 0)
			APCleanupPsQueue(pAd, &pAd->MacTab.McastPsQueue);
		DBGPRINT(RT_DEBUG_TRACE, ("2McastPsQueue.Number %u...\n", pAd->MacTab.McastPsQueue.Number));

		/* ENTRY PREEMPTION: Zero Mac Table but entry's content */
/*		NdisZeroMemory(&pAd->MacTab, sizeof(MAC_TABLE));*/
		NdisZeroMemory(&pAd->MacTab.Size,
							sizeof(MAC_TABLE)-
							Offsetof(MAC_TABLE, Size));

		InitializeQueueHeader(&pAd->MacTab.McastPsQueue);
		/*NdisReleaseSpinLock(&pAd->MacTabLock);*/
#ifdef RTMP_MAC_PCI
		RTMP_IRQ_UNLOCK(&pAd->irq_lock, IrqFlags);
#endif /* RTMP_MAC_PCI */
	}
#endif /* CONFIG_AP_SUPPORT */
	return;
}

VOID MacTableResetWdev(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	int i;
	BOOLEAN Cancelled;
#ifdef CONFIG_AP_SUPPORT
	UCHAR *pOutBuffer = NULL;
	NDIS_STATUS NStatus;
	ULONG FrameLen = 0;
	HEADER_802_11 DeAuthHdr;
	USHORT Reason;
#endif /* CONFIG_AP_SUPPORT */
	MAC_TABLE_ENTRY *pMacEntry;

	DBGPRINT(RT_DEBUG_TRACE, ("MacTableReset\n"));
	/*NdisAcquireSpinLock(&pAd->MacTabLock);*/

	for (i = 1; i < MAX_LEN_OF_MAC_TABLE; i++) {
		pMacEntry = &pAd->MacTab.Content[i];
		if (!pMacEntry)
			continue;
		if (pMacEntry->wdev != wdev)
			continue;
		if (IS_ENTRY_CLIENT(pMacEntry)) {
			RTMPReleaseTimer(&pMacEntry->EnqueueStartForPSKTimer, &Cancelled);
			pMacEntry->EnqueueEapolStartTimerRunning = EAPOL_START_DISABLE;

#ifdef CONFIG_AP_SUPPORT
			{
				/* Before reset MacTable, send disassociation packet to client.*/
				if (pMacEntry->Sst == SST_ASSOC) {
					/*  send out a De-authentication request frame*/
					NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);
					if (NStatus != NDIS_STATUS_SUCCESS) {
						DBGPRINT(RT_DEBUG_TRACE, (" MlmeAllocateMemory fail  ..\n"));
						/*NdisReleaseSpinLock(&pAd->MacTabLock);*/
						return;
					}

					Reason = REASON_NO_LONGER_VALID;
					DBGPRINT(RT_DEBUG_OFF,
						("Send DeAuth (Reason=%d) to %02x:%02x:%02x:%02x:%02x:%02x\n",
								Reason, PRINT_MAC(pMacEntry->Addr)));
					MgtMacHeaderInit(pAd, &DeAuthHdr, SUBTYPE_DEAUTH, 0, pMacEntry->Addr,
										wdev->if_addr,
										wdev->bssid);
					MakeOutgoingFrame(pOutBuffer, &FrameLen,
										sizeof(HEADER_802_11), &DeAuthHdr,
										2, &Reason,
										END_OF_ARGS);

					MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
					MlmeFreeMemory(pAd, pOutBuffer);
					RtmpusecDelay(5000);
				}
			}
#endif /* CONFIG_AP_SUPPORT */
		}

		if (IS_ENTRY_APCLI(pMacEntry))
			continue;

		MacTableDeleteEntry(pAd, i, pMacEntry->Addr);
	}

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		{
#ifdef WSC_AP_SUPPORT
			BOOLEAN Cancelled;

			RTMPCancelTimer(&pAd->ApCfg.MBSSID[wdev->func_idx].WscControl.EapolTimer, &Cancelled);
			pAd->ApCfg.MBSSID[wdev->func_idx].WscControl.EapolTimerRunning = FALSE;
			NdisZeroMemory(pAd->ApCfg.MBSSID[wdev->func_idx].WscControl.EntryAddr, MAC_ADDR_LEN);
			pAd->ApCfg.MBSSID[wdev->func_idx].WscControl.EapMsgRunning = FALSE;
#endif /* WSC_AP_SUPPORT */
			pAd->ApCfg.MBSSID[wdev->func_idx].StaCount = 0;
		}
	}
#endif /* CONFIG_AP_SUPPORT */
}


#ifdef MAC_REPEATER_SUPPORT
MAC_TABLE_ENTRY *InsertMacRepeaterEntry(RTMP_ADAPTER *pAd, UCHAR *pAddr, UCHAR IfIdx)
{
	MAC_TABLE_ENTRY *pEntry = NULL;
	PAPCLI_STRUCT pApCliEntry = NULL;

	os_alloc_mem(NULL, (UCHAR **)&pEntry, sizeof(MAC_TABLE_ENTRY));

	if (pEntry)
	{
		pApCliEntry = &pAd->ApCfg.ApCliTab[IfIdx];
		pEntry->Aid = pApCliEntry->MacTabWCID + 1; // TODO: We need to record count of STAs
		COPY_MAC_ADDR(pEntry->Addr, pApCliEntry->MlmeAux.Bssid);
		printk("sn - InsertMacRepeaterEntry: Aid = %d\n", pEntry->Aid);
		hex_dump("sn - InsertMacRepeaterEntry pEntry->Addr", pEntry->Addr, 6);
		/* Add this entry into ASIC RX WCID search table */
		RTMP_STA_ENTRY_ADD(pAd, pEntry);
		os_free_mem(NULL, pEntry);
	}

	return NULL;
}

#endif /* MAC_REPEATER_SUPPORT */


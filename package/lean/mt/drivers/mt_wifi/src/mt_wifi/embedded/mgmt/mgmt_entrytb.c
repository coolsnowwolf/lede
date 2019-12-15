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
	Who		When			What
	--------	----------		----------------------------------------------
*/

#include <rt_config.h>
#ifdef VOW_SUPPORT
#include <ap_vow.h>
#endif /* VOW_SUPPORT */


VOID TRTableEntryDump(RTMP_ADAPTER *pAd, INT tr_idx, const RTMP_STRING *caller, INT line)
{
	STA_TR_ENTRY *tr_entry;
	INT qidx;
	struct qm_ops *ops = pAd->qm_ops;

	ASSERT(tr_idx < MAX_LEN_OF_TR_TABLE);

	if (tr_idx >= MAX_LEN_OF_TR_TABLE)
		return;

	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Dump TR_ENTRY called by function %s(%d)\n", caller, line));
	tr_entry = &pAd->MacTab.tr_entry[tr_idx];
	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TR_ENTRY[%d]\n", tr_idx));
	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tEntryType=%x\n", tr_entry->EntryType));
	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\twdev=%p\n", tr_entry->wdev));
	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\twcid=%d\n", tr_entry->wcid));
	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tfunc_tb_idx=%d\n", tr_entry->func_tb_idx));
	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tAddr=%02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(tr_entry->Addr)));
	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tBSSID=%02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(tr_entry->bssid)));
	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tFlags\n"));
	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tbIAmBadAtheros=%d, isCached=%d, PortSecured=%d, PsMode=%d, LockEntryTx=%d\n",
			 tr_entry->bIAmBadAtheros, tr_entry->isCached, tr_entry->PortSecured, tr_entry->PsMode, tr_entry->LockEntryTx));
	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tTxRx Characters\n"));
	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tNonQosDataSeq=%d\n", tr_entry->NonQosDataSeq));
	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tTxSeq[0]=%d, TxSeq[1]=%d, TxSeq[2]=%d, TxSeq[3]=%d\n",
			 tr_entry->TxSeq[0], tr_entry->TxSeq[1], tr_entry->TxSeq[2], tr_entry->TxSeq[3]));
	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tCurrTxRate=%x\n", tr_entry->CurrTxRate));
	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tQueuing Info\n"));
	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tenq_cap=%d, deq_cap=%d\n", tr_entry->enq_cap, tr_entry->deq_cap));
	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tQueuedPkt: TxQ[0]=%d, TxQ[1]=%d, TxQ[2]=%d, TxQ[3]=%d, PSQ=%d\n",
			 tr_entry->tx_queue[0].Number, tr_entry->tx_queue[1].Number,
			 tr_entry->tx_queue[2].Number, tr_entry->tx_queue[3].Number,
			 tr_entry->ps_queue.Number));
	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\tdeq_cnt=%d, deq_bytes=%d\n", tr_entry->deq_cnt, tr_entry->deq_bytes));

	for (qidx = 0; qidx < 4; qidx++) {
		if (ops->sta_dump_queue)
			ops->sta_dump_queue(pAd, tr_entry->wcid, TX_DATA, qidx);

		ge_tx_swq_dump(pAd, qidx);
	}
}

VOID TRTableResetEntry(RTMP_ADAPTER *pAd, UCHAR tr_tb_idx)
{
	struct _STA_TR_ENTRY *tr_entry;
	INT qidx;
	struct qm_ops *ops = pAd->qm_ops;

	if (tr_tb_idx >= MAX_LEN_OF_TR_TABLE)
		return;

	tr_entry = &pAd->MacTab.tr_entry[tr_tb_idx];

	if (IS_ENTRY_NONE(tr_entry))
		return;

	tr_entry->enq_cap = FALSE;
	tr_entry->deq_cap = FALSE;

	if (ops->sta_clean_queue)
		ops->sta_clean_queue(pAd, tr_entry->wcid);

	SET_ENTRY_NONE(tr_entry);

	for (qidx = 0; qidx < WMM_QUE_NUM; qidx++)
		NdisFreeSpinLock(&tr_entry->txq_lock[qidx]);

	NdisFreeSpinLock(&tr_entry->ps_queue_lock);
	NdisFreeSpinLock(&tr_entry->ps_sync_lock);
	return;
}


VOID TRTableInsertEntry(RTMP_ADAPTER *pAd, UCHAR tr_tb_idx, MAC_TABLE_ENTRY *pEntry)
{
	struct _STA_TR_ENTRY *tr_entry;
	INT qidx, tid, upId;
#ifdef FQ_SCH_SUPPORT
	struct fq_stainfo_type *pfq_sta = NULL;
#endif
	if (tr_tb_idx < MAX_LEN_OF_TR_TABLE) {
		tr_entry = &pAd->MacTab.tr_entry[tr_tb_idx];
		pEntry->tr_tb_idx = tr_tb_idx;
		tr_entry->EntryType = pEntry->EntryType;
		tr_entry->wdev = pEntry->wdev;
		tr_entry->func_tb_idx = pEntry->func_tb_idx;
		tr_entry->wcid = pEntry->wcid;
		NdisMoveMemory(tr_entry->Addr, pEntry->Addr, MAC_ADDR_LEN);
		tr_entry->NonQosDataSeq = 0;

		for (tid = 0; tid < NUM_OF_TID; tid++)
			tr_entry->TxSeq[tid] = 0;

		for (upId = 0; upId < NUM_OF_UP; upId++) {
			tr_entry->cacheSn[upId] = -1;
			tr_entry->previous_sn[upId] = -1;
			tr_entry->previous_amsdu_state[upId] = MSDU_FORMAT;
		}

#ifdef MT_MAC

		if (IS_HIF_TYPE(pAd, HIF_MT)) {
			if (tr_entry->wdev)
				tr_entry->OmacIdx = pEntry->wdev->OmacIdx;
			else
				MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: wdev == NULL\n",
						 __func__));
		}

#endif /* MT_MAC */
		tr_entry->PsMode = PWR_ACTIVE;
		tr_entry->isCached = FALSE;
		tr_entry->PortSecured = WPA_802_1X_PORT_NOT_SECURED;
		tr_entry->CurrTxRate = pEntry->CurrTxRate;

		for (qidx = 0; qidx < WMM_QUE_NUM; qidx++) {
			InitializeQueueHeader(&tr_entry->tx_queue[qidx]);
#ifdef MT_SDIO_ADAPTIVE_TC_RESOURCE_CTRL
#if TC_PAGE_BASED_DEMAND
			tr_entry->TotalPageCount[qidx] = 0;
#endif
#endif
			NdisAllocateSpinLock(pAd, &tr_entry->txq_lock[qidx]);
		}

		InitializeQueueHeader(&tr_entry->ps_queue);
		NdisAllocateSpinLock(pAd, &tr_entry->ps_queue_lock);
		NdisAllocateSpinLock(pAd, &tr_entry->ps_sync_lock);
		tr_entry->deq_cnt = 0;
		tr_entry->deq_bytes = 0;
		tr_entry->PsQIdleCount = 0;
		tr_entry->enq_cap = TRUE;
		tr_entry->deq_cap = TRUE;
		NdisMoveMemory(tr_entry->bssid, pEntry->wdev->bssid, MAC_ADDR_LEN);
#ifdef FQ_SCH_SUPPORT
		if (pAd->fq_ctrl.enable & FQ_READY) {
			pfq_sta = &tr_entry->fq_sta_rec;
			for (qidx = 0; qidx < WMM_QUE_NUM; qidx++)
				pfq_sta->status[qidx] = FQ_UN_CLEAN_STA;
			fq_clean_list(pAd, WMM_NUM_OF_AC);
		}
#endif
	}
}


VOID TRTableInsertMcastEntry(RTMP_ADAPTER *pAd, UCHAR tr_tb_idx, struct wifi_dev *wdev)
{
	struct _STA_TR_ENTRY *tr_entry = NULL;
	INT qidx, tid;

	if (tr_tb_idx > MAX_LEN_OF_TR_TABLE)
		return;

	tr_entry = &pAd->MacTab.tr_entry[tr_tb_idx];

	if (!tr_entry)
		return;

	tr_entry->EntryType = ENTRY_CAT_MCAST;
	tr_entry->wdev = wdev;
	tr_entry->func_tb_idx = wdev->func_idx;
	tr_entry->PsMode = PWR_ACTIVE;
	tr_entry->isCached = FALSE;
	tr_entry->PortSecured = WPA_802_1X_PORT_SECURED;
	tr_entry->CurrTxRate = pAd->CommonCfg.MlmeRate;
	NdisMoveMemory(tr_entry->Addr, &BROADCAST_ADDR[0], MAC_ADDR_LEN);
	/* TODO: shiang-usw, for following fields, need better assignment! */
	tr_entry->wcid = tr_tb_idx;
	tr_entry->NonQosDataSeq = 0;

	for (tid = 0; tid < NUM_OF_TID; tid++)
		tr_entry->TxSeq[tid] = 0;

	for (qidx = 0; qidx < WMM_QUE_NUM; qidx++) {
		InitializeQueueHeader(&tr_entry->tx_queue[qidx]);
		NdisAllocateSpinLock(pAd, &tr_entry->txq_lock[qidx]);
	}

	InitializeQueueHeader(&tr_entry->ps_queue);
	NdisAllocateSpinLock(pAd, &tr_entry->ps_queue_lock);
	NdisAllocateSpinLock(pAd, &tr_entry->ps_sync_lock);
	tr_entry->deq_cnt = 0;
	tr_entry->deq_bytes = 0;
	tr_entry->PsQIdleCount = 0;
	tr_entry->enq_cap = TRUE;
	tr_entry->deq_cap = TRUE;
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT)) {
		if (tr_entry->wdev)
			tr_entry->OmacIdx = wdev->OmacIdx;
		else
			MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: wdev == NULL\n",
					 __func__));
	}

#endif /* MT_MAC */
	/*
	    Carter check,
	    if Mcast pkt will reference the bssid field for do something?
	    if so, need to check flow.
	*/
	NdisMoveMemory(tr_entry->bssid, wdev->bssid, MAC_ADDR_LEN);
}

/* TODO: this function not finish yet!! */
VOID MgmtTableSetMcastEntry(RTMP_ADAPTER *pAd, UCHAR wcid)
{
	MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.Content[wcid];

	pEntry->EntryType = ENTRY_CAT_MCAST;
	pEntry->Sst = SST_ASSOC;
	pEntry->Aid = wcid;
	pEntry->wcid = wcid;
	pEntry->PsMode = PWR_ACTIVE;
	pEntry->CurrTxRate = pAd->CommonCfg.MlmeRate;
	pEntry->Addr[0] = 0x01;
	pEntry->HTPhyMode.field.MODE = MODE_OFDM;
	pEntry->HTPhyMode.field.MCS = 3;
	NdisMoveMemory(pEntry->Addr, &BROADCAST_ADDR[0], MAC_ADDR_LEN);
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		pEntry->wdev = &pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev;
	}
#endif /* CONFIG_AP_SUPPORT */
}


VOID MacTableSetEntryPhyCfg(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry)
{
	if (pEntry->MaxSupportedRate < RATE_FIRST_OFDM_RATE) {
		pEntry->MaxHTPhyMode.field.MODE = MODE_CCK;
		pEntry->MaxHTPhyMode.field.MCS = pEntry->MaxSupportedRate;
		pEntry->MinHTPhyMode.field.MODE = MODE_CCK;
		pEntry->MinHTPhyMode.field.MCS = pEntry->MaxSupportedRate;
		pEntry->HTPhyMode.field.MODE = MODE_CCK;
		pEntry->HTPhyMode.field.MCS = pEntry->MaxSupportedRate;
	} else {
		pEntry->MaxHTPhyMode.field.MODE = MODE_OFDM;
		pEntry->MaxHTPhyMode.field.MCS = OfdmRateToRxwiMCS[pEntry->MaxSupportedRate];
		pEntry->MinHTPhyMode.field.MODE = MODE_OFDM;
		pEntry->MinHTPhyMode.field.MCS = OfdmRateToRxwiMCS[pEntry->MaxSupportedRate];
		pEntry->HTPhyMode.field.MODE = MODE_OFDM;
		pEntry->HTPhyMode.field.MCS = OfdmRateToRxwiMCS[pEntry->MaxSupportedRate];
	}
}


VOID MacTableSetEntryRaCap(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *ent, struct _vendor_ie_cap *vendor_ie)
{
	ULONG ra_ie = vendor_ie->ra_cap;
	ULONG mtk_ie = vendor_ie->mtk_cap;
#ifdef DOT11_VHT_AC
	ULONG brcm_ie = vendor_ie->brcm_cap;
#endif /*DOT11_VHT_AC*/
	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("vendor_ie_cap for ra_cap=%08x, mtk_cap=%08x\n", (UINT32)ra_ie, (UINT32)mtk_ie));
	NdisCopyMemory(&ent->vendor_ie, vendor_ie, sizeof(struct _vendor_ie_cap));
	CLIENT_CAP_CLEAR_FLAG(ent, fCLIENT_STATUS_RALINK_CHIPSET);
	CLIENT_CAP_CLEAR_FLAG(ent, fCLIENT_STATUS_AGGREGATION_CAPABLE);
	CLIENT_CAP_CLEAR_FLAG(ent, fCLIENT_STATUS_PIGGYBACK_CAPABLE);
	CLIENT_CAP_CLEAR_FLAG(ent, fCLIENT_STATUS_RDG_CAPABLE);
	CLIENT_STATUS_CLEAR_FLAG(ent, fCLIENT_STATUS_RALINK_CHIPSET);
	CLIENT_STATUS_CLEAR_FLAG(ent, fCLIENT_STATUS_AGGREGATION_CAPABLE);
	CLIENT_STATUS_CLEAR_FLAG(ent, fCLIENT_STATUS_PIGGYBACK_CAPABLE);
	CLIENT_STATUS_CLEAR_FLAG(ent, fCLIENT_STATUS_RDG_CAPABLE);

	/* TODO: need MTK CAP ? */

	/* Set cap flags */
	if (vendor_ie->is_rlt == TRUE) {
		CLIENT_CAP_SET_FLAG(ent, fCLIENT_STATUS_RALINK_CHIPSET);
		CLIENT_STATUS_SET_FLAG(ent, fCLIENT_STATUS_RALINK_CHIPSET);
#ifdef AGGREGATION_SUPPORT

		if (ra_ie & RALINK_AGG_CAP) {
			CLIENT_CAP_SET_FLAG(ent, fCLIENT_STATUS_AGGREGATION_CAPABLE);

			if (pAd->CommonCfg.bAggregationCapable) {
				CLIENT_STATUS_SET_FLAG(ent, fCLIENT_STATUS_AGGREGATION_CAPABLE);
				MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL,
						 DBG_LVL_INFO, ("RaAggregate= 1\n"));
			}
		}

#endif /* AGGREGATION_SUPPORT */
#ifdef PIGGYBACK_SUPPORT

		if (ra_ie & RALINK_PIGGY_CAP) {
			CLIENT_CAP_SET_FLAG(ent, fCLIENT_STATUS_PIGGYBACK_CAPABLE);

			if (pAd->CommonCfg.bPiggyBackCapable) {
				CLIENT_STATUS_SET_FLAG(ent, fCLIENT_STATUS_PIGGYBACK_CAPABLE);
				MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL,
						 DBG_LVL_INFO, ("PiggyBack= 1\n"));
			}
		}

#endif /* PIGGYBACK_SUPPORT */

		if (ra_ie & RALINK_RDG_CAP) {
			CLIENT_CAP_SET_FLAG(ent, fCLIENT_STATUS_RDG_CAPABLE);

			if (pAd->CommonCfg.bRdg) {
				CLIENT_STATUS_SET_FLAG(ent, fCLIENT_STATUS_RDG_CAPABLE);
				MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL,
						 DBG_LVL_INFO, ("Rdg = 1\n"));
			}
		}

#ifdef DOT11_VHT_AC

		if ((ra_ie & RALINK_256QAM_CAP)
			&& (pAd->CommonCfg.g_band_256_qam)) {
			MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 ("RALINK_256QAM_CAP for 2.4G\n"));
			ent->fgGband256QAMSupport = TRUE;
		}

#endif /*DOT11_VHT_AC*/
	}

#ifdef DOT11_VHT_AC
	else if ((mtk_ie & MEDIATEK_256QAM_CAP)
			 && (pAd->CommonCfg.g_band_256_qam)) {
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 ("MEDIATEK_256QAM_CAP for 2.4G\n"));
		ent->fgGband256QAMSupport = TRUE;
	} else if ((brcm_ie & BROADCOM_256QAM_CAP)
			   && (pAd->CommonCfg.g_band_256_qam)) {
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 ("BROADCOM_256QAM_CAP for 2.4G\n"));
		ent->fgGband256QAMSupport = TRUE;
	}

#endif /*DOT11_VHT_AC*/
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

	while (pEntry && !IS_ENTRY_NONE(pEntry)) {
		if (MAC_ADDR_EQUAL(pEntry->Addr, pAddr))
			break;
		else
			pEntry = pEntry->pNext;
	}

	return pEntry;
}

MAC_TABLE_ENTRY *MacTableLookup2(RTMP_ADAPTER *pAd, UCHAR *pAddr, struct wifi_dev *wdev)
{
	ULONG HashIdx;
	MAC_TABLE_ENTRY *pEntry = NULL;

	HashIdx = MAC_ADDR_HASH_INDEX(pAddr);
	pEntry = pAd->MacTab.Hash[HashIdx];

	if (wdev) {
		while (pEntry && !IS_ENTRY_NONE(pEntry)) {
			if (MAC_ADDR_EQUAL(pEntry->Addr, pAddr) && (pEntry->wdev == wdev))
				break;
			else
				pEntry = pEntry->pNext;
		}
	} else {

		while (pEntry && !IS_ENTRY_NONE(pEntry)) {
			if (MAC_ADDR_EQUAL(pEntry->Addr, pAddr))
				break;
			else
				pEntry = pEntry->pNext;
		}
	}

	return pEntry;
}



static INT MacTableResetEntry(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry, BOOLEAN clean)
{
	BOOLEAN Cancelled;
	struct _SECURITY_CONFIG *pSecConfig = &pEntry->SecConfig;

	RTMPCancelTimer(&pSecConfig->StartFor4WayTimer, &Cancelled);
	RTMPCancelTimer(&pSecConfig->StartFor2WayTimer, &Cancelled);
	RTMPCancelTimer(&pSecConfig->Handshake.MsgRetryTimer, &Cancelled);
#ifdef DOT11W_PMF_SUPPORT
	RTMPCancelTimer(&pSecConfig->PmfCfg.SAQueryTimer, &Cancelled);
	RTMPCancelTimer(&pSecConfig->PmfCfg.SAQueryConfirmTimer, &Cancelled);
#endif /* DOT11W_PMF_SUPPORT */
	NdisZeroMemory(pEntry, sizeof(MAC_TABLE_ENTRY));

	if (clean == TRUE) {
		pEntry->MaxSupportedRate = RATE_11;
		pEntry->CurrTxRate = RATE_11;
		NdisZeroMemory(pEntry, sizeof(MAC_TABLE_ENTRY));
	}

	return 0;
}


#ifdef OUI_CHECK_SUPPORT
static VOID oui_mgroup_update(MAC_TABLE *mtb, UCHAR *addr, UCHAR act)
{
	UCHAR tmp_cnt = 0;
	UCHAR i = 0;
	MAC_TABLE_ENTRY *entry;

	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s(): update mgroup before num:%d. act=%d\n", __func__, mtb->oui_mgroup_cnt, act));

	for (i = 0; i <= mtb->Size; i++) {
		entry = &mtb->Content[i];

		if (MAC_OUI_EQUAL(entry->Addr, addr) && !MAC_ADDR_EQUAL(entry->Addr, addr)) {
			tmp_cnt++;

			if (tmp_cnt > 2)
				break;
		}
	}

	/*only 1  oui equal means new match group*/
	if (tmp_cnt == 1) {
		mtb->oui_mgroup_cnt = (act == OUI_MGROUP_ACT_JOIN) ?
							  (mtb->oui_mgroup_cnt + 1) : (mtb->oui_mgroup_cnt - 1);
	}

	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s(): update mgroup after num:%d\n", __func__, mtb->oui_mgroup_cnt));
}
#endif /*OUI_CHECK_SUPPORT*/

MAC_TABLE_ENTRY *MacTableInsertEntry(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR *pAddr,
	IN struct wifi_dev *wdev,
	IN UINT32 ent_type,
	IN UCHAR OpMode,
	IN BOOLEAN CleanAll)
{
	UCHAR HashIdx;
	int i;
	MAC_TABLE_ENTRY *pEntry = NULL, *pCurrEntry;
	/* ASIC_SEC_INFO Info = {0}; */
	struct _RTMP_CHIP_CAP *cap;

	if (pAd->MacTab.Size >= GET_MAX_UCAST_NUM(pAd))
		return NULL;

	cap = hc_get_chip_cap(pAd->hdev_ctrl);

	/* allocate one MAC entry*/
	NdisAcquireSpinLock(&pAd->MacTabLock);
	/* TODO:Carter, check why start from 1 */
	i = HcAcquireUcastWcid(pAd, wdev);

	if (i == INVAILD_WCID) {
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("%s(): Entry full!\n",
		 __func__));
		NdisReleaseSpinLock(&pAd->MacTabLock);
		return NULL;
	}

	/* pick up the first available vacancy*/
	if (IS_ENTRY_NONE(&pAd->MacTab.Content[i])) {
		pEntry = &pAd->MacTab.Content[i];
		MacTableResetEntry(pAd, pEntry, CleanAll);
		/* ENTRY PREEMPTION: initialize the entry */
		pEntry->wdev = wdev;
		pEntry->wcid = i;
		pEntry->func_tb_idx = wdev->func_idx;
		pEntry->bIAmBadAtheros = FALSE;
		pEntry->pAd = pAd;
		pEntry->CMTimerRunning = FALSE;
		COPY_MAC_ADDR(pEntry->Addr, pAddr);
		{
			struct _SECURITY_CONFIG *pSecConfig = &pEntry->SecConfig;

			RTMPInitTimer(pAd, &pSecConfig->StartFor4WayTimer, GET_TIMER_FUNCTION(WPAStartFor4WayExec), pEntry, FALSE);
			RTMPInitTimer(pAd, &pSecConfig->StartFor2WayTimer, GET_TIMER_FUNCTION(WPAStartFor2WayExec), pEntry, FALSE);
			RTMPInitTimer(pAd, &pSecConfig->Handshake.MsgRetryTimer, GET_TIMER_FUNCTION(WPAHandshakeMsgRetryExec), pEntry, FALSE);
#ifdef DOT11W_PMF_SUPPORT
			RTMPInitTimer(pAd, &pSecConfig->PmfCfg.SAQueryTimer, GET_TIMER_FUNCTION(PMF_SAQueryTimeOut), pEntry, FALSE);
			RTMPInitTimer(pAd, &pSecConfig->PmfCfg.SAQueryConfirmTimer, GET_TIMER_FUNCTION(PMF_SAQueryConfirmTimeOut), pEntry, FALSE);
#endif /* DOT11W_PMF_SUPPORT */
		}
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
		/* TODO: shiang-usw,  remove upper setting becasue we need to migrate to tr_entry! */
		pAd->MacTab.tr_entry[i].PsMode = PWR_ACTIVE;
		pAd->MacTab.tr_entry[i].NoDataIdleCount = 0;
		pAd->MacTab.tr_entry[i].ContinueTxFailCnt = 0;
		pAd->MacTab.tr_entry[i].LockEntryTx = FALSE;
		pAd->MacTab.tr_entry[i].TimeStamp_toTxRing = 0;
		pAd->MacTab.tr_entry[i].PsDeQWaitCnt = 0;
		pEntry->SecConfig.pmkid = NULL;
		pEntry->SecConfig.pmk_cache = NULL;
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
		pEntry->TotalTxSuccessCnt = 0;
		pEntry->TxStatRspCnt = 0;
#endif

		do {
#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT

			if (ent_type == ENTRY_APCLI) {
				SET_ENTRY_APCLI(pEntry);
				/* SET_ENTRY_AP(pEntry);//Carter, why set entry to APCLI then set to AP???? */
				COPY_MAC_ADDR(pEntry->bssid, pAddr);
				pEntry->pApCli = &pAd->ApCfg.ApCliTab[pEntry->func_tb_idx];
				pEntry->SecConfig.AKMMap = pAd->ApCfg.ApCliTab[pEntry->func_tb_idx].MlmeAux.AKMMap;
				pEntry->SecConfig.PairwiseCipher = pAd->ApCfg.ApCliTab[pEntry->func_tb_idx].MlmeAux.PairwiseCipher;
				pEntry->SecConfig.GroupCipher = pAd->ApCfg.ApCliTab[pEntry->func_tb_idx].MlmeAux.GroupCipher;
				SET_CONNECTION_TYPE(pEntry, CONNECTION_INFRA_AP);/*the peer type related to APCLI is AP role.*/

				if ((IS_AKM_OPEN(pEntry->SecConfig.AKMMap))
					|| (IS_AKM_SHARED(pEntry->SecConfig.AKMMap))
					|| (IS_AKM_AUTOSWITCH(pEntry->SecConfig.AKMMap))) {
					pEntry->SecConfig.Handshake.WpaState = AS_NOTUSE;
					pEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;
				} else {
					pEntry->SecConfig.Handshake.WpaState = AS_INITIALIZE;
					pEntry->PrivacyFilter = Ndis802_11PrivFilter8021xWEP;
				}

				break;
			}

#ifdef MAC_REPEATER_SUPPORT
			else if (ent_type == ENTRY_REPEATER) {
				PAPCLI_STRUCT pApCliEntry = &pAd->ApCfg.ApCliTab[pEntry->func_tb_idx];
				MAC_TABLE_ENTRY *pRootApEntry = &pAd->MacTab.Content[pApCliEntry->MacTabWCID];

				SET_ENTRY_REPEATER(pEntry);
				COPY_MAC_ADDR(pEntry->bssid, pAddr);
				pEntry->pApCli = &pAd->ApCfg.ApCliTab[pEntry->func_tb_idx];
				pEntry->SecConfig.AKMMap = pRootApEntry->SecConfig.AKMMap;
				pEntry->SecConfig.PairwiseCipher = pRootApEntry->SecConfig.PairwiseCipher;
				pEntry->SecConfig.GroupCipher = pRootApEntry->SecConfig.GroupCipher;
				SET_CONNECTION_TYPE(pEntry, CONNECTION_INFRA_AP);/*the peer type related to Rept is AP role.*/

				if ((IS_AKM_OPEN(pEntry->SecConfig.AKMMap))
					|| (IS_AKM_SHARED(pEntry->SecConfig.AKMMap))
					|| (IS_AKM_AUTOSWITCH(pEntry->SecConfig.AKMMap))) {
					pEntry->SecConfig.Handshake.WpaState = AS_NOTUSE;
					pEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;
				} else {
					pEntry->SecConfig.Handshake.WpaState = AS_INITIALIZE;
					pEntry->PrivacyFilter = Ndis802_11PrivFilter8021xWEP;
				}

				MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						 ("Repeater Security wcid=%d, AKMMap=0x%x, PairwiseCipher=0x%x, GroupCipher=0x%x\n",
						  pEntry->wcid, pEntry->SecConfig.AKMMap,
						  pEntry->SecConfig.PairwiseCipher, pEntry->SecConfig.GroupCipher));
				break;
			}

#endif /* MAC_REPEATER_SUPPORT */
#endif /* APCLI_SUPPORT */
#ifdef WDS_SUPPORT

			if (ent_type == ENTRY_WDS) {
				SET_ENTRY_WDS(pEntry);
				SET_CONNECTION_TYPE(pEntry, CONNECTION_WDS);
				COPY_MAC_ADDR(pEntry->bssid, pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev.bssid);
				break;
			}

#endif /* WDS_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_AP_SUPPORT

			if (ent_type == ENTRY_CLIENT) {
				/* be a regular-entry*/
				if (pAd->ApCfg.EntryClientCount >= GET_MAX_UCAST_NUM(pAd)) {
					MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_OFF,
							 ("%s: The station number is over MaxUcastEntryNum = %d\n",
							  __func__, GET_MAX_UCAST_NUM(pAd)));
					NdisReleaseSpinLock(&pAd->MacTabLock);
					return NULL;
				}

				if ((pEntry->func_tb_idx < pAd->ApCfg.BssidNum) &&
					(pEntry->func_tb_idx < MAX_MBSSID_NUM(pAd)) &&
					((pEntry->func_tb_idx < HW_BEACON_MAX_NUM)) &&
					(pAd->ApCfg.MBSSID[pEntry->func_tb_idx].MaxStaNum != 0) &&
					(pAd->ApCfg.MBSSID[pEntry->func_tb_idx].StaCount >= pAd->ApCfg.MBSSID[pEntry->func_tb_idx].MaxStaNum)) {
					MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("%s: The connection table is full in ra%d.\n", __func__, pEntry->func_tb_idx));
					NdisReleaseSpinLock(&pAd->MacTabLock);
					return NULL;
				}

				ASSERT((wdev == &pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev));
				SET_ENTRY_CLIENT(pEntry);
				SET_CONNECTION_TYPE(pEntry, CONNECTION_INFRA_STA);
				pEntry->pMbss = &pAd->ApCfg.MBSSID[pEntry->func_tb_idx];
				MBSS_MR_APIDX_SANITY_CHECK(pAd, pEntry->func_tb_idx);
				COPY_MAC_ADDR(pEntry->bssid, wdev->bssid);

				if (IS_SECURITY_OPEN_WEP(&wdev->SecConfig)) {
					/* OPEN WEP */
					pEntry->SecConfig.AKMMap = wdev->SecConfig.AKMMap;
					pEntry->SecConfig.PairwiseCipher = wdev->SecConfig.PairwiseCipher;
					pEntry->SecConfig.PairwiseKeyId = wdev->SecConfig.PairwiseKeyId;
					pEntry->SecConfig.GroupCipher = wdev->SecConfig.GroupCipher;
					pEntry->SecConfig.GroupKeyId = wdev->SecConfig.GroupKeyId;
					os_move_mem(pEntry->SecConfig.WepKey, wdev->SecConfig.WepKey, sizeof(SEC_KEY_INFO)*SEC_KEY_NUM);
					pEntry->SecConfig.GroupKeyId = wdev->SecConfig.GroupKeyId;
				}

				if ((IS_AKM_OPEN(wdev->SecConfig.AKMMap))
					|| (IS_SECURITY_OPEN_WEP(&wdev->SecConfig))
					|| (IS_AKM_SHARED(wdev->SecConfig.AKMMap)))
					pEntry->SecConfig.Handshake.WpaState = AS_NOTUSE;
				else
					pEntry->SecConfig.Handshake.WpaState = AS_INITIALIZE;

				pEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;
				pEntry->StaIdleTimeout = pAd->ApCfg.StaIdleTimeout;
				pAd->ApCfg.MBSSID[pEntry->func_tb_idx].StaCount++;
				pAd->ApCfg.EntryClientCount++;
#ifdef VOW_SUPPORT

				/* vow_set_client(pAd, pEntry->func_tb_idx, pEntry->wcid); */
				if (VOW_IS_ENABLED(pAd)) {
					if (vow_watf_is_enabled(pAd))
						set_vow_watf_sta_dwrr(pAd, &pEntry->Addr[0], pEntry->wcid);

					RTMP_SET_STA_DWRR(pAd, pEntry);
				}

#endif /* VOW_SUPPORT */

#ifdef ROAMING_ENHANCE_SUPPORT
#ifdef APCLI_SUPPORT
				pEntry->bRoamingRefreshDone = FALSE;
#endif /* APCLI_SUPPORT */
#endif /* ROAMING_ENHANCE_SUPPORT */
				break;
			}
#ifdef AIR_MONITOR
			else if (ent_type == ENTRY_MONITOR) {
				SET_ENTRY_MONITOR(pEntry);
				break;
			}

#endif /* AIR_MONITOR */
#endif /* CONFIG_AP_SUPPORT */
		} while (FALSE);

		RTMP_SET_TR_ENTRY(pAd, pEntry);
#ifdef TXBF_SUPPORT
#ifndef MT_MAC

		if (cap->FlgHwTxBfCap) {
			RTMPInitTimer(pAd, &pEntry->eTxBfProbeTimer, GET_TIMER_FUNCTION(eTxBfProbeTimerExec), pEntry, FALSE);
			NdisAllocateSpinLock(pAd, &pEntry->TxSndgLock);
		}

#endif /* MT_MAC */
#endif /* TXBF_SUPPORT */
#ifdef STREAM_MODE_SUPPORT
		/* Enable Stream mode for first three entries in MAC table */
#endif /* STREAM_MODE_SUPPORT */
#ifdef UAPSD_SUPPORT

		/* Ralink WDS doesn't support any power saving.*/
		if (IS_ENTRY_CLIENT(pEntry)
#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
			|| IS_ENTRY_TDLS(pEntry)
#endif /* defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) */
		   ) {
			/* init U-APSD enhancement related parameters */
			MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): INIT UAPSD MR ENTRY", __func__));
			UAPSD_MR_ENTRY_INIT(pEntry);
		}

#endif /* UAPSD_SUPPORT */
		pAd->MacTab.Size++;
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
#ifdef WSC_AP_SUPPORT
			pEntry->bWscCapable = FALSE;
			pEntry->Receive_EapolStart_EapRspId = 0;
#endif /* WSC_AP_SUPPORT */
		}

#endif /* CONFIG_AP_SUPPORT */
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): alloc entry #%d, Total= %d\n",
				 __func__, i, pAd->MacTab.Size));
	}

	/* add this MAC entry into HASH table */
	if (pEntry) {
		HashIdx = MAC_ADDR_HASH_INDEX(pAddr);

		if (pAd->MacTab.Hash[HashIdx] == NULL)
			pAd->MacTab.Hash[HashIdx] = pEntry;
		else {
			pCurrEntry = pAd->MacTab.Hash[HashIdx];

			while (pCurrEntry->pNext != NULL)
				pCurrEntry = pCurrEntry->pNext;

			pCurrEntry->pNext = pEntry;
		}

#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
#ifdef WSC_AP_SUPPORT

			if (IS_ENTRY_CLIENT(pEntry) &&
				(pEntry->func_tb_idx < pAd->ApCfg.BssidNum) &&
				MAC_ADDR_EQUAL(pEntry->Addr, pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.WscControl.EntryAddr))
				NdisZeroMemory(pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.WscControl.EntryAddr, MAC_ADDR_LEN);

#endif /* WSC_AP_SUPPORT */
#ifdef SMART_ANTENNA
			pEntry->mcsInUse = -1;
#endif /* SMART_ANTENNA */
#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)

			if (IS_ENTRY_CLIENT(pEntry) &&
				wf_drv_tbl.wf_fwd_add_entry_inform_hook)
				wf_drv_tbl.wf_fwd_add_entry_inform_hook(pEntry->Addr);

#endif /* CONFIG_WIFI_PKT_FWD */
		}

#endif /* CONFIG_AP_SUPPORT */
	}

#ifdef OUI_CHECK_SUPPORT
	oui_mgroup_update(&pAd->MacTab, pAddr, OUI_MGROUP_ACT_JOIN);
#endif
	NdisReleaseSpinLock(&pAd->MacTabLock);

	/*update tx burst, must after unlock pAd->MacTabLock*/
	/* rtmp_tx_burst_set(pAd); */
	return pEntry;
}


static INT32 MacTableDelEntryFromHash(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry)
{
	USHORT HashIdx;
	MAC_TABLE_ENTRY  *pPrevEntry, *pProbeEntry;

	HashIdx = MAC_ADDR_HASH_INDEX(pEntry->Addr);
	pPrevEntry = NULL;
	pProbeEntry = pAd->MacTab.Hash[HashIdx];
	ASSERT(pProbeEntry);

	/* update Hash list*/
	while (pProbeEntry) {
		if (pProbeEntry == pEntry) {
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
static VOID mac_entry_disconn_act(struct _RTMP_ADAPTER *pAd, struct _MAC_TABLE_ENTRY *pEntry)
{
	STA_TR_ENTRY *tr_entry = NULL;
	struct wifi_dev *wdev = pEntry->wdev;

	if (!pEntry || IS_ENTRY_NONE(pEntry))
		return;

	if (!wdev)
		return;

	/* Set port secure to NOT_SECURED here to avoid race condition with ba_ori_session_start */
	tr_entry = &pAd->MacTab.tr_entry[pEntry->tr_tb_idx];
	tr_entry->PortSecured = WPA_802_1X_PORT_NOT_SECURED;

	ba_session_tear_down_all(pAd, pEntry->wcid);
	/* RTMP_STA_ENTRY_MAC_RESET--> AsicDelWcidTab() should be integrated to below function*/
	/*in the future */
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		/* peer link up needs pEntry type information to decide txop disable or not*/
		/* so invalid pEntry type later */
		if (wdev_do_disconn_act(pEntry->wdev, pEntry) != TRUE)
			MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): disconnection fail!\n", __func__));
	}
}

BOOLEAN MacTableDeleteEntry(RTMP_ADAPTER *pAd, USHORT wcid, UCHAR *pAddr)
{
	MAC_TABLE_ENTRY *pEntry;
	STA_TR_ENTRY *tr_entry;
	BOOLEAN Cancelled;
	BOOLEAN	bDeleteEntry = FALSE;
#ifdef SMART_ANTENNA
	unsigned long irqflags;
#endif /* SMART_ANTENNA */
#ifdef CONFIG_AP_SUPPORT
	BSS_STRUCT *mbss = NULL;
#endif /*CONFIG_AP_SUPPORT*/
	struct wifi_dev *wdev;
	ADD_HT_INFO_IE *addht;
	UCHAR i;
#ifdef GREENAP_SUPPORT
	struct greenap_ctrl *greenap = &pAd->ApCfg.greenap;
#endif /* GREENAP_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
#if defined(RT_CFG80211_SUPPORT) || defined(MBO_SUPPORT)
	UCHAR TmpAddrForIndicate[MAC_ADDR_LEN] = {0};
	BOOLEAN bIndicateSendEvent = FALSE;
#endif /* defined(RT_CFG80211_SUPPORT) || defined(MBO_SUPPORT) */
#endif /* CONFIG_AP_SUPPORT */
	struct _RTMP_CHIP_CAP *cap;

	if (!pAd)
		return FALSE;

	if (!(VALID_UCAST_ENTRY_WCID(pAd, wcid)))
		return FALSE;

	cap = hc_get_chip_cap(pAd->hdev_ctrl);

	pEntry = &pAd->MacTab.Content[wcid];
	tr_entry = &pAd->MacTab.tr_entry[wcid];

	/*disconnect first*/
	mac_entry_disconn_act(pAd, pEntry);

	NdisAcquireSpinLock(&pAd->MacTabLock);

	if (pEntry && !IS_ENTRY_NONE(pEntry)) {
#ifdef CONFIG_AP_SUPPORT
		mbss = &pAd->ApCfg.MBSSID[pEntry->func_tb_idx];
#endif /*CONFIG_AP_SUPPORT*/
#ifdef BAND_STEERING
		if ((pAd->ApCfg.BandSteering) && IS_ENTRY_CLIENT(pEntry) && IS_VALID_MAC(pEntry->Addr))
			BndStrg_UpdateEntry(pAd, pEntry, NULL, FALSE);
#endif
		/*get wdev*/
		wdev = pEntry->wdev;
#ifdef CONFIG_AP_SUPPORT
		WLAN_MR_TIM_BIT_CLEAR(pAd, pEntry->func_tb_idx, pEntry->Aid);
#endif /* CONFIG_AP_SUPPORT */
#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)

		if (wf_drv_tbl.wf_fwd_delete_entry_inform_hook)
			wf_drv_tbl.wf_fwd_delete_entry_inform_hook(pEntry->Addr);

#endif /* CONFIG_WIFI_PKT_FWD */

		if (MAC_ADDR_EQUAL(pEntry->Addr, pAddr)) {


#if defined(CONFIG_AP_SUPPORT) && defined(CONFIG_DOT11V_WNM)

			if (pAd->ApCfg.MBSSID[pEntry->apidx].WNMCtrl.ProxyARPEnable) {
				RemoveIPv4ProxyARPEntry(pAd, mbss, pEntry->Addr);
				RemoveIPv6ProxyARPEntry(pAd, mbss, pEntry->Addr);
			}

#if defined(CONFIG_HOTSPOT_R2) || defined(CONFIG_DOT11V_WNM)
			pEntry->IsKeep = 0;
#endif /* CONFIG_HOTSPOT_R2 */
#endif
			bDeleteEntry = TRUE;
#ifdef TXBF_SUPPORT
#ifndef MT_MAC

			if (cap->FlgHwTxBfCap)
				RTMPReleaseTimer(&pEntry->eTxBfProbeTimer, &Cancelled);

#endif /* MT_MAC */
#endif /* TXBF_SUPPORT */
#ifdef STREAM_MODE_SUPPORT

			/* Clear Stream Mode register for this client */
			if (pEntry->StreamModeMACReg != 0)
				RTMP_IO_WRITE32(pAd, pEntry->StreamModeMACReg + 4, 0);

#endif /* STREAM_MODE_SUPPORT // */
#ifdef CONFIG_AP_SUPPORT

			if (IS_ENTRY_CLIENT(pEntry)
			   ) {
				INT32 aid = pEntry->wcid;

				RESET_FLAG_CONN_IN_PROG(aid, pAd->ConInPrgress);
#ifdef DOT1X_SUPPORT

				/* Notify 802.1x daemon to clear this sta info*/
				if (IS_AKM_1X_Entry(pEntry)
					|| IS_IEEE8021X_Entry(wdev)
#ifdef RADIUS_ACCOUNTING_SUPPORT
					|| IS_AKM_WPA_CAPABILITY_Entry(pEntry)
#endif /*RADIUS_ACCOUNTING_SUPPORT*/
				   )
					DOT1X_InternalCmdAction(pAd, pEntry, DOT1X_DISCONNECT_ENTRY);

#endif /* DOT1X_SUPPORT */
#ifdef IGMP_SNOOP_SUPPORT
				IgmpGroupDelMembers(pAd, (PUCHAR)pEntry->Addr, wdev, pEntry->wcid);
#endif /* IGMP_SNOOP_SUPPORT */
				pAd->ApCfg.MBSSID[pEntry->func_tb_idx].StaCount--;
				pAd->ApCfg.EntryClientCount--;
#ifdef HOSTAPD_SUPPORT

				if (pEntry && mbss->Hostapd == Hostapd_EXT) {
					RtmpOSWrielessEventSendExt(pAd->net_dev, RT_WLAN_EVENT_EXPIRED, -1, pEntry->Addr,
											   NULL, 0, pEntry->func_tb_idx);
				}

#endif /* HOSTAPD_SUPPORT */
			}

#endif /* CONFIG_AP_SUPPORT */
			MacTableDelEntryFromHash(pAd, pEntry);
#ifdef CONFIG_AP_SUPPORT
			APCleanupPsQueue(pAd, &tr_entry->ps_queue); /* return all NDIS packet in PSQ*/
#endif /* CONFIG_AP_SUPPORT */
			TRTableResetEntry(pAd, wcid);
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
			{
				struct _SECURITY_CONFIG *pSecConfig = &pEntry->SecConfig;

				RTMPCancelTimer(&pSecConfig->StartFor4WayTimer, &Cancelled);
				RTMPCancelTimer(&pSecConfig->StartFor2WayTimer, &Cancelled);
				RTMPCancelTimer(&pSecConfig->Handshake.MsgRetryTimer, &Cancelled);
				RTMPReleaseTimer(&pSecConfig->StartFor4WayTimer, &Cancelled);
				RTMPReleaseTimer(&pSecConfig->StartFor2WayTimer, &Cancelled);
				RTMPReleaseTimer(&pSecConfig->Handshake.MsgRetryTimer, &Cancelled);
#ifdef DOT11W_PMF_SUPPORT
				RTMPCancelTimer(&pSecConfig->PmfCfg.SAQueryTimer, &Cancelled);
				RTMPCancelTimer(&pSecConfig->PmfCfg.SAQueryConfirmTimer, &Cancelled);
				RTMPReleaseTimer(&pSecConfig->PmfCfg.SAQueryTimer, &Cancelled);
				RTMPReleaseTimer(&pSecConfig->PmfCfg.SAQueryConfirmTimer, &Cancelled);
#endif /* DOT11W_PMF_SUPPORT */
			}
#ifdef CONFIG_AP_SUPPORT
#ifdef WSC_AP_SUPPORT

			if (IS_ENTRY_CLIENT(pEntry)) {
				PWSC_CTRL	pWscControl = &mbss->wdev.WscControl;

				if (MAC_ADDR_EQUAL(pEntry->Addr, pWscControl->EntryAddr)) {
					/*
					Some WPS Client will send dis-assoc close to WSC_DONE.
					If AP misses WSC_DONE, WPS Client still sends dis-assoc to AP.
					Do not cancel timer if WscState is WSC_STATE_WAIT_DONE.
					*/
					if ((pWscControl->EapolTimerRunning == TRUE) &&
						(pWscControl->WscState != WSC_STATE_WAIT_DONE)) {
						RTMPCancelTimer(&pWscControl->EapolTimer, &Cancelled);
						pWscControl->EapolTimerRunning = FALSE;
						pWscControl->EapMsgRunning = FALSE;
						NdisZeroMemory(&(pWscControl->EntryAddr[0]), MAC_ADDR_LEN);
					}
				}

				pEntry->Receive_EapolStart_EapRspId = 0;
				pEntry->bWscCapable = FALSE;
			}

#endif /* WSC_AP_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
#ifdef SMART_ANTENNA
			RTMP_IRQ_LOCK(&pAd->smartAntLock, irqflags);

			if (RTMP_SA_WORK_ON(pAd)) {
				RTMP_SA_TRAINING_PARAM *pTrainEntry = pEntry->pTrainEntry;

				if (pTrainEntry) {
					sa_del_train_entry(pAd, pEntry->Addr, FALSE);
					pEntry->pTrainEntry = NULL;
				}

				pAd->pSAParam->bStaChange = TRUE;
			}

			RTMP_IRQ_UNLOCK(&pAd->smartAntLock, irqflags);
#endif /* SMART_ANTENNA */
#ifdef CONFIG_AP_SUPPORT
#if defined(RT_CFG80211_SUPPORT) || defined(MBO_SUPPORT)
			COPY_MAC_ADDR(TmpAddrForIndicate, pEntry->Addr);
			bIndicateSendEvent = TRUE;
#endif /* defined(RT_CFG80211_SUPPORT) || defined(MBO_SUPPORT) */
#endif /* CONFIG_AP_SUPPORT */

				/* NdisZeroMemory(pEntry, sizeof(MAC_TABLE_ENTRY)); */
				NdisZeroMemory(pEntry->Addr, MAC_ADDR_LEN);
			pAd->MacTab.Size--;
#ifdef TXBF_SUPPORT
#ifndef MT_MAC

			if (cap->FlgHwTxBfCap)
				NdisFreeSpinLock(&pEntry->TxSndgLock);

#endif /* MT_MAC */
#endif /* TXBF_SUPPORT */
			MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("MacTableDeleteEntry1 - Total= %d\n", pAd->MacTab.Size));
		} else {
			MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n%s: Impossible Wcid = %d !!!!!\n", __func__, wcid));
#ifdef CONFIG_AP_SUPPORT
#if defined(RT_CFG80211_SUPPORT) || defined(MBO_SUPPORT)
			bIndicateSendEvent = FALSE;
#endif /* defined(RT_CFG80211_SUPPORT) || defined(MBO_SUPPORT) */
#endif /* CONFIG_AP_SUPPORT */
		}

#ifdef CONFIG_AP_SUPPORT
		ApUpdateCapabilityAndErpIe(pAd, mbss);
#endif /* CONFIG_AP_SUPPORT */
	}

#ifdef OUI_CHECK_SUPPORT
	oui_mgroup_update(&pAd->MacTab, pAddr, OUI_MGROUP_ACT_LEAVE);
#endif

	/*update tx burst, must after unlock pAd->MacTabLock*/
	/* rtmp_tx_burst_set(pAd); */

	if (bDeleteEntry) {

		/*release ucast wcid*/
		HcReleaseUcastWcid(pAd, pEntry->wdev, wcid);
#ifdef CONFIG_AP_SUPPORT
		/*
		* move CFG80211_ApStaDelSendEvent here after the entry & hash are deleted ,
		* to prevent removing the same hash twice
		*/
#ifdef RT_CFG80211_SUPPORT


		if (bCfgSendEvent && pEntry && !IS_ENTRY_NONE(pEntry) && IS_ENTRY_CLIENT(pEntry)) {
			if (RTMP_CFG80211_HOSTAPD_ON(pAd)
			   )
				CFG80211_ApStaDelSendEvent(pAd, TmpAddrForIndicate, pEntry->wdev->if_dev);
		}

#endif /* RT_CFG80211_SUPPORT */
#ifdef MBO_SUPPORT
		/* mbo - indicate daemon to remve this sta */
		{
			MBO_EVENT_STA_DISASSOC evt_sta_disassoc;

			COPY_MAC_ADDR(evt_sta_disassoc.mac_addr, TmpAddrForIndicate);
			MboIndicateStaDisassocToDaemon(pAd, &evt_sta_disassoc, MBO_MSG_REMOVE_STA);
		}
#endif /* MBO_SUPPORT */
#ifdef WAPP_SUPPORT
		if (IS_ENTRY_CLIENT(pEntry))
			wapp_send_cli_leave_event(pAd, RtmpOsGetNetIfIndex(wdev->if_dev), pAddr);
#endif /* WAPP_SUPPORT */
		if (IS_ENTRY_CLIENT(pEntry))
			nonerp_sta_num(pEntry, PEER_LEAVE);
#endif /* CONFIG_AP_SUPPORT */
		/* invalidate the entry */
		SET_ENTRY_NONE(pEntry);
#ifdef GREENAP_SUPPORT

		if (greenap_get_capability(greenap) && greenap_get_allow_status(greenap))
			greenap_check_peer_connection_at_link_up_down(pAd, wdev, greenap);

#endif /* GREENAP_SUPPORT */
#ifdef R1KH_HARD_RETRY
		RTMP_OS_EXIT_COMPLETION(&pEntry->ack_r1kh);
#endif /* R1KH_HARD_RETRY */
	}
	NdisReleaseSpinLock(&pAd->MacTabLock);
	/*Reset operating mode when no Sta.*/
	if (pAd->MacTab.Size == 0) {
#ifdef DOT11_N_SUPPORT

		for (i = 0; i < WDEV_NUM_MAX; i++) {
			wdev = pAd->wdev_list[i];

			if (!wdev)
				continue;

			if (wdev->wdev_type != WDEV_TYPE_AP)
				continue;

			addht = wlan_operate_get_addht(wdev);
			addht->AddHtInfo2.OperaionMode = 0;
		}

#endif /* DOT11_N_SUPPORT */
	}

	return TRUE;
}


/*
	==========================================================================
	Description:
		This routine reset the entire MAC table. All packets pending in
		the power-saving queues are freed here.
	==========================================================================
 */
VOID MacTableResetWdev(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	int i;
#ifdef CONFIG_AP_SUPPORT
	UCHAR *pOutBuffer = NULL;
	NDIS_STATUS NStatus;
	ULONG FrameLen = 0;
	HEADER_802_11 DeAuthHdr;
	USHORT Reason;
	struct _BSS_STRUCT *mbss;
#endif /* CONFIG_AP_SUPPORT */
	MAC_TABLE_ENTRY *pMacEntry;

	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("MacTableResetWdev\n"));

	/* TODO:Carter, check why start from 1 */
	for (i = 1; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
		pMacEntry = &pAd->MacTab.Content[i];

		if (pMacEntry->wdev != wdev)
			continue;

		if (IS_ENTRY_CLIENT(pMacEntry)) {
			pMacEntry->EnqueueEapolStartTimerRunning = EAPOL_START_DISABLE;
#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
				/* Before reset MacTable, send disassociation packet to client.*/
				if (pMacEntry->Sst == SST_ASSOC) {
					/*	send out a De-authentication request frame*/
					NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);

					if (NStatus != NDIS_STATUS_SUCCESS) {
						MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (" MlmeAllocateMemory fail  ..\n"));
						/*NdisReleaseSpinLock(&pAd->MacTabLock);*/
						return;
					}

					Reason = REASON_NO_LONGER_VALID;
					MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("Send DeAuth (Reason=%d) to %02x:%02x:%02x:%02x:%02x:%02x\n",
							 Reason, PRINT_MAC(pMacEntry->Addr)));
					MgtMacHeaderInit(pAd, &DeAuthHdr, SUBTYPE_DEAUTH, 0, pMacEntry->Addr,
									 wdev->if_addr,
									 wdev->bssid);
					MakeOutgoingFrame(pOutBuffer, &FrameLen,
									  sizeof(HEADER_802_11), &DeAuthHdr,
									  2, &Reason,
									  END_OF_ARGS);
					MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
					MlmeFreeMemory(pOutBuffer);
					RtmpusecDelay(5000);
				}
			}
#endif /* CONFIG_AP_SUPPORT */
		}

		/* Delete a entry via WCID */
		MacTableDeleteEntry(pAd, i, pMacEntry->Addr);
	}

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		mbss = &pAd->ApCfg.MBSSID[wdev->func_idx];
#ifdef WSC_AP_SUPPORT
		{
			BOOLEAN Cancelled;

			RTMPCancelTimer(&mbss->wdev.WscControl.EapolTimer, &Cancelled);
			mbss->wdev.WscControl.EapolTimerRunning = FALSE;
			NdisZeroMemory(mbss->wdev.WscControl.EntryAddr, MAC_ADDR_LEN);
			mbss->wdev.WscControl.EapMsgRunning = FALSE;
		}
#endif /* WSC_AP_SUPPORT */
		mbss->StaCount = 0;
	}
#endif /* CONFIG_AP_SUPPORT */
	return;
}


VOID MacTableReset(RTMP_ADAPTER *pAd)
{
	int i;
#ifdef CONFIG_AP_SUPPORT
#ifdef RTMP_MAC_PCI
	unsigned long	IrqFlags = 0;
#endif /* RTMP_MAC_PCI */
	UCHAR *pOutBuffer = NULL;
	NDIS_STATUS NStatus;
	ULONG FrameLen = 0;
	HEADER_802_11 DeAuthHdr;
	USHORT Reason;
	UCHAR apidx = MAIN_MBSSID;
#endif /* CONFIG_AP_SUPPORT */
	MAC_TABLE_ENTRY *pMacEntry;
	/*
	MAC_TABLE_ENTRY *Hash[HASH_TABLE_SIZE];
	    MAC_TABLE_ENTRY Content[MAX_LEN_OF_MAC_TABLE];
	NdisZeroMemory(Hash, sizeof(Hash));
	NdisZeroMemory(Content, sizeof(Content));
	*/
	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("MacTableReset\n"));

	/*NdisAcquireSpinLock(&pAd->MacTabLock);*/

	/* TODO:Carter, check why start from 1 */
	for (i = 1; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
		pMacEntry = &pAd->MacTab.Content[i];

		if (IS_ENTRY_CLIENT(pMacEntry)) {
			pMacEntry->EnqueueEapolStartTimerRunning = EAPOL_START_DISABLE;
#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
				/* Before reset MacTable, send disassociation packet to client.*/
				if (pMacEntry->Sst == SST_ASSOC) {
					/*  send out a De-authentication request frame*/
					NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);

					if (NStatus != NDIS_STATUS_SUCCESS) {
						MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (" MlmeAllocateMemory fail  ..\n"));
						/*NdisReleaseSpinLock(&pAd->MacTabLock);*/
						return;
					}

					Reason = REASON_NO_LONGER_VALID;
					MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("Send DeAuth (Reason=%d) to %02x:%02x:%02x:%02x:%02x:%02x\n",
							 Reason, PRINT_MAC(pMacEntry->Addr)));
					MgtMacHeaderInit(pAd, &DeAuthHdr, SUBTYPE_DEAUTH, 0, pMacEntry->Addr,
									 pAd->ApCfg.MBSSID[pMacEntry->func_tb_idx].wdev.if_addr,
									 pAd->ApCfg.MBSSID[pMacEntry->func_tb_idx].wdev.bssid);
					MakeOutgoingFrame(pOutBuffer, &FrameLen,
									  sizeof(HEADER_802_11), &DeAuthHdr,
									  2, &Reason,
									  END_OF_ARGS);
					MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
					MlmeFreeMemory(pOutBuffer);
					RtmpusecDelay(5000);
				}
			}
#endif /* CONFIG_AP_SUPPORT */
		}

		/* Delete a entry via WCID */
		MacTableDeleteEntry(pAd, i, pMacEntry->Addr);
	}

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		MAC_TABLE_ENTRY **Hash = NULL;
		MAC_TABLE_ENTRY *Content = NULL;
		STA_TR_ENTRY *tr_entry = NULL;

		Hash = RtmpOsVmalloc(sizeof(struct _MAC_TABLE_ENTRY *) * HASH_TABLE_SIZE);
		Content = RtmpOsVmalloc(sizeof(struct _MAC_TABLE_ENTRY) * GET_MAX_UCAST_NUM(pAd));
		tr_entry = RtmpOsVmalloc(sizeof(STA_TR_ENTRY)*MAX_LEN_OF_TR_TABLE);

		if (!Hash || !Content || !tr_entry) {
			MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, (" MACTABLE AllocateMemory fail  ..(%p/%p/%p)\n", Hash, Content, tr_entry));
			ASSERT(0);
		}

		if (GET_MAX_UCAST_NUM(pAd) > MAX_LEN_OF_MAC_TABLE) {
			ASSERT(0);
			return;
		}

		NdisZeroMemory(&Hash[0], sizeof(struct _MAC_TABLE_ENTRY *) * HASH_TABLE_SIZE);
		NdisZeroMemory(&Content[0], sizeof(struct _MAC_TABLE_ENTRY) * GET_MAX_UCAST_NUM(pAd));
		NdisZeroMemory(&tr_entry[0], sizeof(STA_TR_ENTRY) * MAX_LEN_OF_TR_TABLE);

		/* MAC_TABLE_ENTRY Content[MAX_LEN_OF_MAC_TABLE]; */

		for (apidx = MAIN_MBSSID; apidx < pAd->ApCfg.BssidNum; apidx++) {
#ifdef WSC_AP_SUPPORT
			BOOLEAN Cancelled;

			RTMPCancelTimer(&pAd->ApCfg.MBSSID[apidx].wdev.WscControl.EapolTimer, &Cancelled);
			pAd->ApCfg.MBSSID[apidx].wdev.WscControl.EapolTimerRunning = FALSE;
			NdisZeroMemory(pAd->ApCfg.MBSSID[apidx].wdev.WscControl.EntryAddr, MAC_ADDR_LEN);
			pAd->ApCfg.MBSSID[apidx].wdev.WscControl.EapMsgRunning = FALSE;
#endif /* WSC_AP_SUPPORT */
			pAd->ApCfg.MBSSID[apidx].StaCount = 0;
		}

#ifdef RTMP_MAC_PCI
		RTMP_IRQ_LOCK(&pAd->irq_lock, IrqFlags);
#endif /* RTMP_MAC_PCI */
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("McastPsQueue.Number %d...\n", pAd->MacTab.McastPsQueue.Number));

		if (pAd->MacTab.McastPsQueue.Number > 0)
			APCleanupPsQueue(pAd, &pAd->MacTab.McastPsQueue);

		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("2McastPsQueue.Number %d...\n", pAd->MacTab.McastPsQueue.Number));
		/* ENTRY PREEMPTION: Zero Mac Table but entry's content */
		/* NdisZeroMemory(&pAd->MacTab.Size, sizeof(MAC_TABLE)-offsetof(MAC_TABLE, Size)); */
		NdisCopyMemory(&Hash[0], pAd->MacTab.Hash, sizeof(struct _MAC_TABLE_ENTRY *) * HASH_TABLE_SIZE);
		NdisCopyMemory(&Content[0], pAd->MacTab.Content, sizeof(struct _MAC_TABLE_ENTRY) * GET_MAX_UCAST_NUM(pAd));
		NdisCopyMemory(&tr_entry[0], pAd->MacTab.tr_entry, sizeof(STA_TR_ENTRY) * MAX_LEN_OF_TR_TABLE);
		NdisZeroMemory(&pAd->MacTab, sizeof(MAC_TABLE));
		NdisCopyMemory(pAd->MacTab.Hash, &Hash[0], sizeof(struct _MAC_TABLE_ENTRY *) * HASH_TABLE_SIZE);
		NdisCopyMemory(pAd->MacTab.Content, &Content[0], sizeof(struct _MAC_TABLE_ENTRY) * GET_MAX_UCAST_NUM(pAd));
		NdisCopyMemory(pAd->MacTab.tr_entry, &tr_entry[0], sizeof(STA_TR_ENTRY) * MAX_LEN_OF_TR_TABLE);
		RtmpOsVfree(Hash);
		RtmpOsVfree(Content);
		RtmpOsVfree(tr_entry);
		InitializeQueueHeader(&pAd->MacTab.McastPsQueue);
		/*NdisReleaseSpinLock(&pAd->MacTabLock);*/
#ifdef RTMP_MAC_PCI
		RTMP_IRQ_UNLOCK(&pAd->irq_lock, IrqFlags);
#endif /* RTMP_MAC_PCI */
	}
#endif /* CONFIG_AP_SUPPORT */
	return;
}

static VOID SetHtVhtForWdev(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
#ifdef DOT11_N_SUPPORT
	SetCommonHT(pAd, wdev);
#ifdef DOT11_VHT_AC

	if (WMODE_CAP_AC(wdev->PhyMode))
		SetCommonVHT(pAd, wdev);

#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */
}

INT	SetCommonHtVht(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	if (wdev)
		SetHtVhtForWdev(pAd, wdev);
	else
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Can't update HT/VHT due to wdev is null!\n", __func__));

	return TRUE;
}


VOID mac_entry_lookup(RTMP_ADAPTER *pAd, UCHAR *pAddr, struct wifi_dev *wdev, MAC_TABLE_ENTRY **entry)
{
	ULONG HashIdx;
	MAC_TABLE_ENTRY *pEntry = NULL;

	HashIdx = MAC_ADDR_HASH_INDEX(pAddr);
	pEntry = pAd->MacTab.Hash[HashIdx];

	while (pEntry && !IS_ENTRY_NONE(pEntry)) {
		if (MAC_ADDR_EQUAL(pEntry->Addr, pAddr))
			break;
		else
			pEntry = pEntry->pNext;
	}

	*entry = pEntry;
}

/*
* Delete MacTableEntry and equeue to cmd thread
*/
VOID mac_entry_delete(struct _RTMP_ADAPTER	*ad, struct _MAC_TABLE_ENTRY *entry)
{
	RTEnqueueInternalCmd(ad, CMDTHRED_MAC_TABLE_DEL, (VOID *) entry, sizeof(struct _MAC_TABLE_ENTRY));
}

INT test_flag_con_in_prog(ULONG *ConInPrgress)
{
	UCHAR i = 0, ConInProgInx;
	INT ret = 0;
#if	(MAX_LEN_OF_MAC_TABLE == 0)
	ConInProgInx =  1;
#elif	((MAX_LEN_OF_MAC_TABLE % NUM_BITS_IN_ULONG) == 0)
	ConInProgInx = (MAX_LEN_OF_MAC_TABLE)/(sizeof(ULONG) * NUM_OF_BITS_IN_BYTE);
#else
	ConInProgInx = (MAX_LEN_OF_MAC_TABLE)/(sizeof(ULONG) * NUM_OF_BITS_IN_BYTE) + 1;
#endif
	while (i < ConInProgInx) {
		if (ConInPrgress[i]) {
			ret = 1;
			break;
		}
		i++;
	}
	return ret;
}

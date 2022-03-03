/***************************************************************************
 * MediaTek Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 1997-2012, MediaTek, Inc.
 *
 * All rights reserved. MediaTek source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek Technology, Inc. is obtained.
 ***************************************************************************

*/
#include "rt_config.h"
#ifdef DBG_AMSDU
DECLARE_TIMER_FUNCTION(amsdu_history_exec);

VOID amsdu_history_exec(PVOID SystemSpecific1, PVOID FunctionContext,
			PVOID SystemSpecific2, PVOID SystemSpecific3)
{
	UINT32 i;
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)FunctionContext;
	STA_TR_ENTRY *tr_entry = NULL;

	for (i = 0; i < MAX_LEN_OF_TR_TABLE; i++) {
		tr_entry = &pAd->MacTab.tr_entry[i];

		if (!IS_ENTRY_NONE(tr_entry)) {
			tr_entry->amsdu_1_rec[pAd->dbg_time_slot] = tr_entry->amsdu_1;
			tr_entry->amsdu_1 = 0;
			tr_entry->amsdu_2_rec[pAd->dbg_time_slot] = tr_entry->amsdu_2;
			tr_entry->amsdu_2 = 0;
			tr_entry->amsdu_3_rec[pAd->dbg_time_slot] = tr_entry->amsdu_3;
			tr_entry->amsdu_3 = 0;
			tr_entry->amsdu_4_rec[pAd->dbg_time_slot] = tr_entry->amsdu_4;
			tr_entry->amsdu_4 = 0;

		}
	}
	pAd->dbg_time_slot++;
	pAd->dbg_time_slot = pAd->dbg_time_slot % TIME_SLOT_NUMS;

}
BUILD_TIMER_FUNCTION(amsdu_history_exec);
#endif

static INT ge_sta_clean_queue(RTMP_ADAPTER *pAd, UCHAR wcid)
{
	UCHAR idx, wcid_start, wcid_end;
	STA_TR_ENTRY *tr_entry;
	ULONG IrqFlags;
	PNDIS_PACKET pPacket;
	QUEUE_ENTRY *pEntry;
	QUEUE_HEADER *pQueue;
#ifdef FQ_SCH_SUPPORT
	INT frame_count = 0;
	struct fq_stainfo_type *pfq_sta = NULL;
	INT need_clean = 0;
#endif

	if (wcid == WCID_ALL) {
		wcid_start = 0;
		wcid_end = MAX_LEN_OF_TR_TABLE - 1;
	} else {
		if (wcid < MAX_LEN_OF_TR_TABLE)
			wcid_start = wcid_end = wcid;
		else {
			MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s():Invalid WCID[%d]\n",
					 __func__, wcid));
			return FALSE;
		}
	}

	for (wcid = wcid_start; wcid <= wcid_end; wcid++) {
		tr_entry = &pAd->MacTab.tr_entry[wcid];

		if (IS_ENTRY_NONE(tr_entry))
			continue;

		for (idx = 0; idx < WMM_QUE_NUM; idx++) {
			RTMP_IRQ_LOCK(&tr_entry->txq_lock[idx], IrqFlags);
			pQueue = &tr_entry->tx_queue[idx];

			while (pQueue->Head) {
				pEntry = RemoveHeadQueue(pQueue);
				TR_ENQ_COUNT_DEC(tr_entry);
				pPacket = QUEUE_ENTRY_TO_PACKET(pEntry);
#ifdef FQ_SCH_SUPPORT
				frame_count++;
#endif
				if (pPacket)
					RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
			}
#ifdef FQ_SCH_SUPPORT
			if ((frame_count > 0) && (pAd->fq_ctrl.enable & FQ_READY)) {
				pfq_sta = &tr_entry->fq_sta_rec;
				RTMP_SEM_LOCK(&pfq_sta->lock[idx]);
				if ((tr_entry->tx_queue[idx].Number == 0) &&
					(pfq_sta->status[idx] == FQ_IN_LIST_STA)) {
					pfq_sta->status[idx] = FQ_UN_CLEAN_STA;
					need_clean = 1;
				}
				RTMP_SEM_UNLOCK(&pfq_sta->lock[idx]);
				RTMP_SPIN_LOCK(&pAd->tx_swq_lock[idx]);
				pAd->fq_ctrl.frm_cnt[idx] -= frame_count;
				frame_count = 0;
				RTMP_SPIN_UNLOCK(&pAd->tx_swq_lock[idx]);
			}
#endif
			RTMP_IRQ_UNLOCK(&tr_entry->txq_lock[idx], IrqFlags);
		}

		RTMP_IRQ_LOCK(&tr_entry->ps_queue_lock, IrqFlags);
		pQueue = &tr_entry->ps_queue;

		while (pQueue->Head) {
			pEntry = RemoveHeadQueue(pQueue);
			pPacket = QUEUE_ENTRY_TO_PACKET(pEntry);

			if (pPacket)
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
		}

		RTMP_IRQ_UNLOCK(&tr_entry->ps_queue_lock, IrqFlags);
	}
#ifdef FQ_SCH_SUPPORT
	if (need_clean == 1)
		fq_clean_list(pAd, WMM_NUM_OF_AC);
#endif
	return NDIS_STATUS_SUCCESS;
}

static INT ge_qm_exit(RTMP_ADAPTER *pAd)
{
	INT ret;
	QUEUE_HEADER *que;
	QUEUE_ENTRY *entry;
	NDIS_PACKET *pkt;
	UCHAR i;
#ifdef DBG_AMSDU
	BOOLEAN cancelled;
#endif

#ifdef CONFIG_TX_DELAY
	BOOLEAN que_agg_timer_cancelled;
#endif

#ifdef DBG_AMSDU
	RTMPReleaseTimer(&pAd->amsdu_history_timer, &cancelled);
#endif

#ifdef CONFIG_TX_DELAY
	RTMPReleaseTimer(&pAd->tr_ctl.tx_delay_ctl.que_agg_timer, &que_agg_timer_cancelled);
#endif

	RTMP_SEM_LOCK(&pAd->mgmt_que_lock);
	que = &pAd->mgmt_que;

	while (que->Head) {
		entry = RemoveHeadQueue(que);
		pkt = QUEUE_ENTRY_TO_PACKET(entry);

		if (pkt)
			RELEASE_NDIS_PACKET(pAd, pkt, NDIS_STATUS_FAILURE);
	}

	RTMP_SEM_UNLOCK(&pAd->mgmt_que_lock);
	NdisFreeSpinLock(&pAd->mgmt_que_lock);

	RTMP_SEM_LOCK(&pAd->high_prio_que_lock);
	que = &pAd->high_prio_que;

	while (que->Head) {
		entry = RemoveHeadQueue(que);
		pkt = QUEUE_ENTRY_TO_PACKET(entry);

		if (pkt)
			RELEASE_NDIS_PACKET(pAd, pkt, NDIS_STATUS_FAILURE);
	}

	RTMP_SEM_UNLOCK(&pAd->high_prio_que_lock);
	NdisFreeSpinLock(&pAd->high_prio_que_lock);

	ret = ge_sta_clean_queue(pAd, WCID_ALL);

	for (i = 0; i < WMM_NUM_OF_AC; i++)
		NdisFreeSpinLock(&pAd->tx_swq_lock[i]);

#ifdef FQ_SCH_SUPPORT
	fq_exit(pAd);
#endif
	return ret;
}

static INT ge_enq_mgmtq_pkt(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PNDIS_PACKET pkt)
{
	struct qm_ops *qm_ops = pAd->qm_ops;

	if (wlan_operate_get_state(wdev) != WLAN_OPER_STATE_VALID) {
		RELEASE_NDIS_PACKET(pAd, pkt, NDIS_STATUS_FAILURE);
		return NDIS_STATUS_FAILURE;
	}

	if (pAd->mgmt_que.Number >= MGMT_QUE_MAX_NUMS)
		goto error;

	RTMP_SEM_LOCK(&pAd->mgmt_que_lock);
	InsertTailQueue(&pAd->mgmt_que, PACKET_TO_QUEUE_ENTRY(pkt));
	RTMP_SEM_UNLOCK(&pAd->mgmt_que_lock);

	qm_ops->schedule_tx_que(pAd);

	return NDIS_STATUS_SUCCESS;

error:
	RELEASE_NDIS_PACKET(pAd, pkt, NDIS_STATUS_FAILURE);
	return NDIS_STATUS_FAILURE;
}

static INT ge_enq_dataq_pkt(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PNDIS_PACKET pkt, UCHAR q_idx)
{
	UCHAR wcid = RTMP_GET_PACKET_WCID(pkt);
	STA_TR_ENTRY *tr_entry = &pAd->MacTab.tr_entry[wcid];
	struct qm_ops *qm_ops = pAd->qm_ops;
	RTMP_ARCH_OP *arch_ops = &pAd->archOps;
	BOOL drop_packet = FALSE, ret = TRUE;

	if (wlan_operate_get_state(wdev) != WLAN_OPER_STATE_VALID) {
		RELEASE_NDIS_PACKET(pAd, pkt, NDIS_STATUS_FAILURE);
		return NDIS_STATUS_FAILURE;
	}

	if (!RTMP_GET_PACKET_HIGH_PRIO(pkt)) {
		if ((arch_ops->archRedMarkPktDrop) && (pAd->red_en))
			drop_packet = arch_ops->archRedMarkPktDrop(wcid, q_idx, pAd);
		if (drop_packet == FALSE) {
			if (pAd->TxSwQueue[q_idx].Number >= pAd->TxSwQMaxLen) {
				RELEASE_NDIS_PACKET(pAd, pkt, NDIS_STATUS_FAILURE);
				if ((arch_ops->archRedEnqueueFail) && (pAd->red_en))
					arch_ops->archRedEnqueueFail(wcid, q_idx, pAd);
				qm_ops->schedule_tx_que(pAd);

				return NDIS_STATUS_FAILURE;
			} else {
#ifdef FQ_SCH_SUPPORT
				if (pAd->fq_ctrl.enable & FQ_READY)
					ret = fq_enq_req(pAd, pkt, q_idx, tr_entry, NULL);
				else
#endif
					ret = ge_enq_req(pAd, pkt, q_idx, tr_entry, NULL);
				if (ret != TRUE) {
					RELEASE_NDIS_PACKET(pAd, pkt, NDIS_STATUS_FAILURE);
					if (arch_ops->archRedEnqueueFail)
						arch_ops->archRedEnqueueFail(wcid, q_idx, pAd);
					qm_ops->schedule_tx_que(pAd);

					return NDIS_STATUS_FAILURE;
				}
			}
		} else {
			RELEASE_NDIS_PACKET(pAd, pkt, NDIS_STATUS_FAILURE);
			return NDIS_STATUS_FAILURE;
		}
	} else {
		RTMP_SEM_LOCK(&pAd->high_prio_que_lock);

		if (pAd->high_prio_que.Number >= HIGH_PRIO_QUE_MAX_NUMS) {
			RELEASE_NDIS_PACKET(pAd, pkt, NDIS_STATUS_FAILURE);
			RTMP_SEM_UNLOCK(&pAd->high_prio_que_lock);
			return NDIS_STATUS_FAILURE;

		}

		InsertTailQueue(&pAd->high_prio_que, PACKET_TO_QUEUE_ENTRY(pkt));
		RTMP_SEM_UNLOCK(&pAd->high_prio_que_lock);
		if ((arch_ops->archRedRecordCP) && (pAd->red_en))
			arch_ops->archRedRecordCP(wcid, pAd);
	}

	qm_ops->schedule_tx_que(pAd);

	return NDIS_STATUS_SUCCESS;
}

static INT ge_enq_psq_pkt(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, STA_TR_ENTRY *tr_entry, PNDIS_PACKET pkt)
{
	QUEUE_HEADER *pkt_que = &tr_entry->ps_queue;

	RTMP_SEM_LOCK(&tr_entry->ps_queue_lock);
	InsertTailQueue(pkt_que, PACKET_TO_QUEUE_ENTRY(pkt));
	RTMP_SEM_UNLOCK(&tr_entry->ps_queue_lock);

	return NDIS_STATUS_SUCCESS;
}

static INT ge_schedule_tx_que(RTMP_ADAPTER *pAd)
{
	struct tm_ops *tm_ops = pAd->tm_qm_ops;
	UINT i;
	UCHAR deq_wcid = 0;
	struct tx_swq_fifo *fifo_swq;

	for (i = 0; i < WMM_NUM_OF_AC; i++) {
		fifo_swq = &pAd->tx_swq[i];
		deq_wcid = fifo_swq->swq[fifo_swq->deqIdx];

		if (deq_wcid != 0)
			break;
	}

	if ((deq_wcid != 0) || (pAd->mgmt_que.Number > 0) ||
		(pAd->high_prio_que.Number > 0)) {
		tm_ops->schedule_task(pAd, TX_DEQ_TASK);
	}

	return NDIS_STATUS_SUCCESS;
}

static BOOLEAN is_amsdu_frame(RTMP_ADAPTER *pAd, NDIS_PACKET *pkt, TX_BLK *pTxBlk)
{
	int minLen = LENGTH_802_3;
	int wcid = RTMP_GET_PACKET_WCID(pkt);

	if (IS_ENTRY_MCAST(&pAd->MacTab.Content[wcid]))
		return FALSE;

	if (RTMP_GET_PACKET_DHCP(pkt) ||
		RTMP_GET_PACKET_ARP(pkt) ||
		RTMP_GET_PACKET_EAPOL(pkt) ||
		RTMP_GET_PACKET_PING(pkt) ||
		RTMP_GET_PACKET_WAI(pkt) ||
		RTMP_GET_PACKET_TDLS_MMPDU(pkt)
	   )
		return FALSE;

	/* Make sure the first packet has non-zero-length data payload */
	if (RTMP_GET_PACKET_VLAN(pkt))
		minLen += LENGTH_802_1Q; /* VLAN tag */
	else if (RTMP_GET_PACKET_LLCSNAP(pkt))
		minLen += 8; /* SNAP hdr Len*/

	if (minLen >= GET_OS_PKT_LEN(pkt))
		return FALSE;

	return TRUE;
}

static VOID ge_sta_dump_queue(RTMP_ADAPTER *pAd, UCHAR wcid, enum PACKET_TYPE pkt_type, UCHAR qidx)
{
	unsigned long IrqFlags;
	QUEUE_ENTRY *entry;
	INT cnt = 0;
	STA_TR_ENTRY *tr_entry = &pAd->MacTab.tr_entry[wcid];

	if (tr_entry == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("%s():Invalid entry(%p) or qidx(%d)\n",
				  __func__, tr_entry, qidx));
		return;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\nDump TxQ[%d] of TR_ENTRY(ID:%d,\
				MAC:%02x:%02x:%02x:%02x:%02x:%02x),\
				enq_cap = %d, ps_state = %s\n",
				qidx, tr_entry->wcid, PRINT_MAC(tr_entry->Addr),
				tr_entry->enq_cap,
				tr_entry->ps_state == PWR_ACTIVE ? "PWR_ACTIVE" : "PWR_SAVE"));

	if (pkt_type == TX_DATA) {
		switch (qidx) {
		case QID_AC_BK:
		case QID_AC_BE:
		case QID_AC_VI:
		case QID_AC_VO:
			RTMP_IRQ_LOCK(&tr_entry->txq_lock[qidx], IrqFlags);
			entry = tr_entry->tx_queue[qidx].Head;

			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\nDump Entry %s\n",
					entry == NULL ? "Empty" : "HasEntry"));

			while (entry != NULL) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" 0x%p ", entry));
				cnt++;
				entry = entry->Next;

				if (entry == NULL)
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

				if (cnt > tr_entry->tx_queue[qidx].Number) {
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						 ("%s():Buggy here? Queue[%d] entry number(%d) not equal!\n",
						  __func__, qidx, tr_entry->tx_queue[qidx].Number));
				}
			};

			RTMP_IRQ_UNLOCK(&tr_entry->txq_lock[qidx], IrqFlags);
			break;
		default:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("unknown q_idx = %d\n", qidx));
			break;
		}
	} else if (pkt_type == TX_DATA_PS) {
		RTMP_SEM_LOCK(&tr_entry->ps_queue_lock);

		entry = tr_entry->ps_queue.Head;

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\nDump Entry %s\n",
				entry == NULL ? "Empty" : "HasEntry"));

		while (entry != NULL) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" 0x%p ", entry));
			cnt++;
			entry = entry->Next;

			if (entry == NULL)
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

			if (cnt > tr_entry->ps_queue.Number) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						 ("%s():Buggy here? Queue[%d] entry number(%d) not equal!\n",
						  __func__, qidx, tr_entry->ps_queue.Number));
			}
		};

		RTMP_SEM_UNLOCK(&tr_entry->ps_queue_lock);
	} else if (pkt_type == TX_DATA_HIGH_PRIO) {

		RTMP_SEM_LOCK(&pAd->high_prio_que_lock);

		entry = pAd->high_prio_que.Head;

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\nDump Entry %s\n",
				entry == NULL ? "Empty" : "HasEntry"));

		while (entry != NULL) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" 0x%p ", entry));
			cnt++;
			entry = entry->Next;

			if (entry == NULL)
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

			if (cnt > pAd->high_prio_que.Number) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						 ("%s():Buggy here? Queue[%d] entry number(%d) not equal!\n",
						  __func__, qidx, pAd->high_prio_que.Number));
			}
		};

		RTMP_SEM_UNLOCK(&pAd->high_prio_que_lock);

	} else if (pkt_type == TX_MGMT) {

		RTMP_SEM_LOCK(&pAd->mgmt_que_lock);
		entry = pAd->mgmt_que.Head;

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\nDump Entry %s\n",
				entry == NULL ? "Empty" : "HasEntry"));

		while (entry != NULL) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" 0x%p ", entry));
			cnt++;
			entry = entry->Next;

			if (entry == NULL)
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));

			if (cnt > pAd->mgmt_que.Number) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						 ("%s():Buggy here? Queue[%d] entry number(%d) not equal!\n",
						  __func__, qidx, pAd->mgmt_que.Number));
			}
		};

		RTMP_SEM_UNLOCK(&pAd->mgmt_que_lock);
	}
}

/*
 * management queue
 * power saving queue
 * data queue
 * high priority queue
 */
static INT32 ge_dump_all_sw_queue(RTMP_ADAPTER *pAd)
{

	QUEUE_ENTRY *entry;
	INT cnt = 0;
	INT i, j;
	STA_TR_ENTRY *tr_entry = NULL;

	/* management sw queue */
	RTMP_SEM_LOCK(&pAd->mgmt_que_lock);
	entry = pAd->mgmt_que.Head;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\nDump management queue Entry %s\n",
			entry == NULL ? "Empty" : "HasEntry"));

	while (entry != NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (" 0x%p ", entry));
		cnt++;
		entry = entry->Next;

		if (entry == NULL)
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\n"));

		if (cnt > pAd->mgmt_que.Number) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s():Buggy here? entry number(%d) not equal!\n",
					  __func__, pAd->mgmt_que.Number));
		}
	};

	RTMP_SEM_UNLOCK(&pAd->mgmt_que_lock);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Count of management Entry = %d\n", cnt));

	/* high prority queue */
	cnt = 0;
	RTMP_SEM_LOCK(&pAd->high_prio_que_lock);

	entry = pAd->high_prio_que.Head;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\nDump high prority queue Entry %s\n",
			entry == NULL ? "Empty" : "HasEntry"));

	while (entry != NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (" 0x%p ", entry));
		cnt++;
		entry = entry->Next;

		if (entry == NULL)
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\n"));

		if (cnt > pAd->high_prio_que.Number) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s():Buggy here? entry number(%d) not equal!\n",
					  __func__, pAd->high_prio_que.Number));
		}
	};

	RTMP_SEM_UNLOCK(&pAd->high_prio_que_lock);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Count of high prority queue Entry = %d\n", cnt));

	/* per sta queue */
	for (i = 0; VALID_WCID(i); i++) {
		PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[i];

		/* dump all MacTable entries */
		if (pEntry->EntryType == ENTRY_NONE)
			continue;

		tr_entry = &pAd->MacTab.tr_entry[i];

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\nDump TR_ENTRY(ID:%d,\
				MAC:%02x:%02x:%02x:%02x:%02x:%02x),\
				enq_cap = %d, ps_state = %s\n",
				tr_entry->wcid, PRINT_MAC(tr_entry->Addr),
				tr_entry->enq_cap,
				tr_entry->ps_state == PWR_ACTIVE ? "PWR_ACTIVE" : "PWR_SAVE"));

		cnt = 0;
		RTMP_SEM_LOCK(&tr_entry->ps_queue_lock);

		entry = tr_entry->ps_queue.Head;

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\nDump wcid(%d) power saving queue Entry %s\n",
				i, entry == NULL ? "Empty" : "HasEntry"));

		while (entry != NULL) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (" 0x%p ", entry));
			cnt++;
			entry = entry->Next;

			if (entry == NULL)
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\n"));

			if (cnt > tr_entry->ps_queue.Number) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s():Buggy here? entry number(%d) not equal!\n",
						  __func__, tr_entry->ps_queue.Number));
			}
		};

		RTMP_SEM_UNLOCK(&tr_entry->ps_queue_lock);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Count of wcid(%d) power saving Entry = %d\n", i, cnt));


		for (j = 0; j < WMM_QUE_NUM; j++) {
			cnt = 0;
			RTMP_SEM_LOCK(&tr_entry->txq_lock[j]);
			entry = tr_entry->tx_queue[j].Head;

			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\nDump wcid(%d), qidx(%d) data queue Entry %s\n",
					i, j, entry == NULL ? "Empty" : "HasEntry"));

			while (entry != NULL) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (" 0x%p ", entry));
				cnt++;
				entry = entry->Next;

				if (entry == NULL)
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\n"));

				if (cnt > tr_entry->tx_queue[j].Number) {
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s():Buggy here? entry number(%d) not equal!\n",
						  __func__, tr_entry->tx_queue[j].Number));
				}
			};

			RTMP_SEM_UNLOCK(&tr_entry->txq_lock[j]);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Count of wcid(%d), qidx(%d) data Entry = %d\n", i, j, cnt));

		}

	}

	return NDIS_STATUS_SUCCESS;
}

VOID ge_tx_swq_dump(RTMP_ADAPTER *pAd, INT qidx)
{
	ULONG IrqFlags;
	UINT deq_id, enq_id, cnt = 0;

	RTMP_IRQ_LOCK(&pAd->tx_swq_lock[qidx], IrqFlags);
	deq_id = pAd->tx_swq[qidx].deqIdx;
	enq_id = pAd->tx_swq[qidx].enqIdx;
	MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\nDump TxSwQ[%d]: DeqIdx=%d, EnqIdx=%d, %s\n",
			  qidx, deq_id, enq_id,
			  (pAd->tx_swq[qidx].swq[deq_id] == 0 ? "Empty" : "HasEntry")));

	for (; deq_id != enq_id; (deq_id =  (deq_id == (TX_SWQ_FIFO_LEN - 1) ? 0 : deq_id + 1))) {
		MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" %d ", pAd->tx_swq[qidx].swq[deq_id]));
		cnt++;

		if (cnt > TX_SWQ_FIFO_LEN) {
			MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 ("%s(): Buggy here? force break! deq_id=%d, enq_id=%d\n",
					  __func__, deq_id, enq_id));
		}

	}

	RTMP_IRQ_UNLOCK(&pAd->tx_swq_lock[qidx], IrqFlags);
	MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
}

inline BOOLEAN ge_get_swq_state(RTMP_ADAPTER *pAd, UINT8 q_idx)
{
	return pAd->tx_swq[q_idx].q_state;
}

inline INT ge_set_swq_state(RTMP_ADAPTER *pAd, UINT8 q_idx, BOOLEAN state)
{
	pAd->tx_swq[q_idx].q_state = state;
	return NDIS_STATUS_SUCCESS;
}

inline UINT32 ge_get_swq_free_num(RTMP_ADAPTER *pAd, UINT8 q_idx)
{
	UINT cap_cnt = 0;
	struct tx_swq_fifo *fifo_swq = &pAd->tx_swq[q_idx];
	INT enq_idx = 0, deq_idx = 0;

	enq_idx = fifo_swq->enqIdx;
	deq_idx = fifo_swq->deqIdx;

	cap_cnt = (enq_idx >= deq_idx) ? (TX_SWQ_FIFO_LEN - enq_idx + deq_idx)
			: (deq_idx - enq_idx);

	return cap_cnt;
}

UINT32 ge_check_swq_state(RTMP_ADAPTER *pAd, UINT8 q_idx)
{
	BOOLEAN swq_state = ge_get_swq_state(pAd, q_idx);
	UINT swq_free_num = ge_get_swq_free_num(pAd, q_idx);

	if ((swq_state == TX_QUE_HIGH) &&
		(swq_free_num >= pAd->tx_swq[q_idx].low_water_mark)) {
		return TX_QUE_HIGH_TO_HIGH;
	} else if ((swq_state == TX_QUE_HIGH) &&
		(swq_free_num < pAd->tx_swq[q_idx].low_water_mark)) {
		return TX_QUE_HIGH_TO_LOW;
	} else if ((swq_state == TX_QUE_LOW) &&
		(swq_free_num > pAd->tx_swq[q_idx].high_water_mark)) {
		return TX_QUE_LOW_TO_HIGH;
	} else if ((swq_state == TX_QUE_LOW) &&
		(swq_free_num <= pAd->tx_swq[q_idx].high_water_mark)) {
		return TX_QUE_LOW_TO_LOW;
	} else {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: unknow state %d, q number = %d",
		__func__, swq_state, swq_free_num));
		return TX_RING_UNKNOW_CHANGE;
	}
}

INT ge_enq_req(RTMP_ADAPTER *pAd, PNDIS_PACKET pkt, UCHAR qidx,
				 STA_TR_ENTRY *tr_entry, QUEUE_HEADER *pPktQueue)
{
	unsigned long IrqFlags = 0;
	BOOLEAN enq_done = FALSE;
	INT enq_idx = 0;
	struct tx_swq_fifo *fifo_swq;
	UCHAR occupied_wcid = 0;

	ASSERT(qidx < WMM_QUE_NUM);
	ASSERT((tr_entry->wcid != 0));
	fifo_swq = &pAd->tx_swq[qidx];
	RTMP_IRQ_LOCK(&pAd->tx_swq_lock[qidx], IrqFlags);
	if ((tr_entry->enqCount > SQ_ENQ_NORMAL_MAX)
		&& (tr_entry->tx_queue[qidx].Number > SQ_ENQ_RESERVE_PERAC)) {
		occupied_wcid = fifo_swq->swq[enq_idx];
		enq_done = FALSE;
		pAd->tr_ctl.tx_sw_q_drop++;
		goto enq_end;
	}

	enq_idx = fifo_swq->enqIdx;

	if ((fifo_swq->swq[enq_idx] == 0) && (tr_entry->enq_cap)) {
		RTMP_IRQ_LOCK(&tr_entry->txq_lock[qidx], IrqFlags);
		InsertTailQueueAc(pAd, tr_entry, &tr_entry->tx_queue[qidx],
						  PACKET_TO_QUEUE_ENTRY(pkt));

		RTMP_IRQ_UNLOCK(&tr_entry->txq_lock[qidx], IrqFlags);
#ifdef MT_SDIO_ADAPTIVE_TC_RESOURCE_CTRL
#if TC_PAGE_BASED_DEMAND
		tr_entry->TotalPageCount[qidx] += (INT16)(MTSDIOTxGetPageCount(GET_OS_PKT_LEN(pkt), FALSE));
#endif /* TC_PAGE_BASED_DEMAND */
#if DEBUG_ADAPTIVE_QUOTA
		MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: wcid %d q %d pkt len %d TotalPageCount %d\n",
				 __func__, tr_entry->wcid, qidx, GET_OS_PKT_LEN(pkt),
				 tr_entry->TotalPageCount[qidx]));
#endif /* DEBUG_ADAPTIVE_QUOTA */
#endif /* MT_SDIO_ADAPTIVE_TC_RESOURCE_CTRL */

		fifo_swq->swq[enq_idx] = tr_entry->wcid;
		INC_RING_INDEX(fifo_swq->enqIdx, TX_SWQ_FIFO_LEN);
		TR_ENQ_COUNT_INC(tr_entry);
		enq_done = TRUE;
	} else {
		occupied_wcid = fifo_swq->swq[enq_idx];
		enq_done = FALSE;
		pAd->tr_ctl.tx_sw_q_drop++;

		/* Stop device first to avoid drop lots of packets, not execute on WMM case */
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_DYNAMIC_BE_TXOP_ACTIVE) &&
			!tx_flow_check_state(pAd, NO_ENOUGH_SWQ_SPACE, qidx))
			tx_flow_set_state_block(pAd, NULL, NO_ENOUGH_SWQ_SPACE, true, qidx);

		goto enq_end;
	}

enq_end:
	RTMP_IRQ_UNLOCK(&pAd->tx_swq_lock[qidx], IrqFlags);
	MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s():EnqPkt(%p) for WCID(%d) to tx_swq[%d].swq[%d] %s\n",
			  __func__, pkt, tr_entry->wcid, qidx, enq_idx,
			  (enq_done ? "success" : "fail")));

	if (enq_done == FALSE) {
#ifdef DBG_DIAGNOSE
#ifdef DBG_TXQ_DEPTH
		if ((pAd->DiagStruct.inited) && (pAd->DiagStruct.wcid == tr_entry->wcid))
			pAd->DiagStruct.diag_info[pAd->DiagStruct.ArrayCurIdx].enq_fall_cnt[qidx]++;
#endif

#endif /* DBG_DIAGNOSE */
		MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("\t FailedCause =>OccupiedWCID:%d,EnqCap:%d\n",
				  occupied_wcid, tr_entry->enq_cap));
	}
	return enq_done;
}


static INT ge_deq_req(RTMP_ADAPTER *pAd, INT cnt, struct dequeue_info *info)
{
	CHAR deq_qid = 0, start_q, end_q;
	UCHAR deq_wcid;
	struct tx_swq_fifo *fifo_swq;
	STA_TR_ENTRY *tr_entry = NULL;
	unsigned long IrqFlags = 0;
	unsigned int quota = 0;

	if (!info->inited) {
		if (info->target_que < WMM_QUE_NUM) {
			info->start_q = info->target_que;
			info->end_q = info->target_que;
		} else {
			info->start_q = (WMM_QUE_NUM - 1);
			info->end_q = 0;
		}

		info->cur_q = info->start_q;

		/*
		 * a. for specific wcid, quota number "cnt" stored in info->pkt_cnt and shared by 4 ac queue
		 * b. for all wcid, quota stored in info->pkt_cnt and info->q_max_cnt[ac_index] and each ac has quota number "cnt"
		 *    shared by all wcid
		 */
		if (info->target_wcid < MAX_LEN_OF_TR_TABLE) {
			info->pkt_cnt = cnt;
			info->full_qid[0] = FALSE;
			info->full_qid[1] = FALSE;
			info->full_qid[2] = FALSE;
			info->full_qid[3] = FALSE;
		} else {
			info->q_max_cnt[0] = cnt;
			info->q_max_cnt[1] = cnt;
			info->q_max_cnt[2] = cnt;
			info->q_max_cnt[3] = cnt;
		}

		info->inited = 1;
	}

	start_q = info->cur_q;
	end_q = info->end_q;

	/*
	 * decide cur_wcid and cur_que under info->pkt_cnt > 0 condition for specific wcid
	 * cur_wcid = info->target_wcid
	 * cur_que = deq_qid
	 * deq_que has two value, one come from info->target_que for specific ac queue,
	 * another go to check if tr_entry[deq_qid].number > 0 from highest priority
	 * to lowest priority ac queue for all ac queue
	 */
	if (info->target_wcid < MAX_LEN_OF_TR_TABLE) {
		if (info->pkt_cnt <= 0) {
			info->status = NDIS_STATUS_FAILURE;
			goto done;
		}

		deq_wcid = info->target_wcid;

		if (info->target_que >= WMM_QUE_NUM) {
			tr_entry = &pAd->MacTab.tr_entry[deq_wcid];

			for (deq_qid = start_q; deq_qid >= end_q; deq_qid--) {
				if (info->full_qid[deq_qid] == FALSE && tr_entry->tx_queue[deq_qid].Number)
					break;
			}
		} else if (info->full_qid[info->target_que] == FALSE)
			deq_qid = info->target_que;
		else {
			info->status = NDIS_STATUS_FAILURE;
			goto done;
		}

		if (deq_qid >= 0) {
			info->cur_q = deq_qid;
			info->cur_wcid = deq_wcid;
		} else
			info->status = NDIS_STATUS_FAILURE;

		goto done;
	}

	/*
	 * decide cur_wcid and cur_que for all wcid
	 * cur_wcid = deq_wcid
	 * deq_wcid need to check tx_swq_fifo from highest priority to lowest priority ac queues
	 * and come from tx_swq_fifo.swq[tx_deq_fifo.deqIdx]
	 * cur_que = deq_qid upon found a wcid
	 */
	for (deq_qid = start_q; deq_qid >= end_q; deq_qid--) {
		RTMP_IRQ_LOCK(&pAd->tx_swq_lock[deq_qid], IrqFlags);
#ifdef FQ_SCH_SUPPORT
		if (pAd->fq_ctrl.enable & FQ_READY)
			deq_wcid = fq_del_list(pAd, info, deq_qid, &quota);
		else
#endif
		{
		fifo_swq = &pAd->tx_swq[deq_qid];
		deq_wcid = fifo_swq->swq[fifo_swq->deqIdx];
			quota = info->q_max_cnt[deq_qid];
		}

		RTMP_IRQ_UNLOCK(&pAd->tx_swq_lock[deq_qid], IrqFlags);

		if (deq_wcid == 0) {
			MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_LOUD,
					 ("%s():tx_swq[%d] emtpy!\n", __func__, deq_qid));
			info->q_max_cnt[deq_qid] = 0;
			continue;
		}

		if (info->q_max_cnt[deq_qid] > 0) {
			info->cur_q = deq_qid;
			info->cur_wcid = deq_wcid;
			info->pkt_cnt = quota;
			break;
		}
	}

	if (deq_qid < end_q) {
		info->cur_q = deq_qid;
		info->status = NDIS_STATUS_FAILURE;
	}

done:
	MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s(): DeqReq %s, Start/End/Cur Queue=%d/%d/%d\n",
			  __func__,
			  (info->status == NDIS_STATUS_SUCCESS ? "success" : "fail"),
			  info->start_q, info->end_q, info->cur_q));

	if (info->status == NDIS_STATUS_SUCCESS) {
		tr_entry = &pAd->MacTab.tr_entry[info->cur_wcid];
		MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 ("\tdeq_info=>wcid:%d, qidx:%d, pkt_cnt:%d, q_max_cnt=%d, QueuedNum=%d\n",
				  info->cur_wcid, info->cur_q, info->pkt_cnt, info->q_max_cnt[deq_qid],
				  tr_entry->tx_queue[info->cur_q].Number));
	} else {
		info->status = NDIS_STATUS_FAILURE;
		MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 ("\tdeq_info=>wcid:%d, qidx:%d, pkt_cnt:%d\n",
				  info->cur_wcid, info->cur_q, info->pkt_cnt));
	}

	return TRUE;
}


static INT ge_deq_report(RTMP_ADAPTER *pAd, struct dequeue_info *info)
{
	UINT tx_cnt = info->deq_pkt_cnt, qidx = info->cur_q;
	struct tx_swq_fifo *fifo_swq;
	unsigned long IrqFlags = 0;

	MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s():Success DeQ(QId=%d) for WCID(%d), PktCnt=%d, TxSWQDeQ/EnQ ID=%d/%d\n",
			  __func__, info->cur_q, info->cur_wcid, info->deq_pkt_cnt,
			  pAd->tx_swq[qidx].deqIdx, pAd->tx_swq[qidx].enqIdx));

	if ((qidx < 4) && (tx_cnt > 0)) {
		RTMP_IRQ_LOCK(&pAd->tx_swq_lock[qidx], IrqFlags);
		fifo_swq = &pAd->tx_swq[qidx];

		do {
			if (fifo_swq->swq[fifo_swq->deqIdx]  == info->cur_wcid) {
				fifo_swq->swq[fifo_swq->deqIdx] = 0;
				INC_RING_INDEX(fifo_swq->deqIdx, TX_SWQ_FIFO_LEN);
				tx_cnt--;
			} else
				break;
		} while (tx_cnt != 0);

		if (tx_flow_check_state(pAd, NO_ENOUGH_SWQ_SPACE, qidx) &&
			ge_get_swq_free_num(pAd, qidx) > pAd->tx_swq[qidx].high_water_mark)
			tx_flow_set_state_block(pAd, NULL, NO_ENOUGH_SWQ_SPACE, FALSE, qidx);

		RTMP_IRQ_UNLOCK(&pAd->tx_swq_lock[qidx], IrqFlags);

		if (info->q_max_cnt[qidx] > 0)
			info->q_max_cnt[qidx] -= info->deq_pkt_cnt;

		if (info->target_wcid < MAX_LEN_OF_TR_TABLE)
			info->pkt_cnt -= info->deq_pkt_cnt;

		/* ge_tx_swq_dump(pAd, qidx); */
		/* rtmp_sta_txq_dump(pAd, &pAd->MacTab.tr_entry[info->wcid], qidx); */
	}

	if (qidx < WMM_NUM_OF_AC)
		MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 ("After DeqReport, tx_swq D/EQIdx=%d/%d, deq_info.q_max_cnt/pkt_cnt=%d/%d\n",
				  pAd->tx_swq[qidx].deqIdx, pAd->tx_swq[qidx].enqIdx,
				  info->q_max_cnt[qidx], info->pkt_cnt));
	return TRUE;
}

#define NUM_OF_MSDU_ID_IN_TXD 4
static BOOLEAN check_amsdu_limit(RTMP_ADAPTER *pAd, TX_BLK *tx_blk, PNDIS_PACKET pkt)
{
	MAC_TABLE_ENTRY *pEntry = tx_blk->pMacEntry;

	/*
	 * limitation rule:
	 * a. limit by A-MSDU size
	 * b. limit by A-MSDU number if amsdu_fix turn on
	 */
	if (tx_blk->TotalFrameNum < pAd->amsdu_max_num) {

		if (pAd->amsdu_fix) {
			if (tx_blk->TotalFrameNum < pAd->amsdu_fix_num)
				return TRUE;
		} else {

			MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				("%s: current total frame len = %d, pkt_len = %d, amsdu_limit_len_adjust = %d\n",
				 __func__, tx_blk->TotalFrameLen, GET_OS_PKT_LEN(pkt), pEntry->amsdu_limit_len_adjust));

			if ((tx_blk->TotalFrameLen + GET_OS_PKT_LEN(pkt))
				<= pEntry->amsdu_limit_len_adjust)
				return TRUE;
		}
	}

	return FALSE;
}

#ifdef RANDOM_PKT_GEN
static UINT32 randomvalueforqidx;
static VOID random_write_resource_idx(RTMP_ADAPTER *pAd, TX_BLK *pTxBlk)
{
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (RandomTxCtrl != 0) {
		pTxBlk->lmac_qidx = randomvalueforqidx % (cap->WmmHwNum * 4);
		randomvalueforqidx += 7;
		if (pTxBlk->lmac_qidx < cap->WmmHwNum * 4)
			pTxBlk->resource_idx = Qidmapping[pTxBlk->lmac_qidx];
	}
}
#endif

#ifdef CONFIG_TX_DELAY
DECLARE_TIMER_FUNCTION(que_agg_timeout);

VOID que_agg_timeout(PVOID SystemSpecific1, PVOID FunctionContext, PVOID SystemSpecific2, PVOID SystemSpecific3)
{
	struct _RTMP_ADAPTER *pAd = (struct _RTMP_ADAPTER *)FunctionContext;
	struct tx_delay_control *tx_delay_ctl = &pAd->tr_ctl.tx_delay_ctl;
	struct qm_ops *qm_ops = pAd->qm_ops;

	if (pAd->tx_dequeue_scheduable) {
		qm_ops->schedule_tx_que(pAd);
		tx_delay_ctl->force_deq = TRUE;
		tx_delay_ctl->que_agg_timer_running = FALSE;
	}
}
BUILD_TIMER_FUNCTION(que_agg_timeout);

static BOOLEAN ge_tx_deq_delay(RTMP_ADAPTER *pAd, STA_TR_ENTRY *tr_entry, UCHAR q_idx)
{
	NDIS_PACKET *pkt = NULL;
	unsigned long flags = 0;
	PQUEUE_HEADER que;
	struct tx_delay_control *tx_delay_ctl = &pAd->tr_ctl.tx_delay_ctl;

	RTMP_IRQ_LOCK(&tr_entry->txq_lock[q_idx], flags);
	que = &tr_entry->tx_queue[q_idx];

	if (que->Head)
		pkt = QUEUE_ENTRY_TO_PACKET(que->Head);

	RTMP_IRQ_UNLOCK(&tr_entry->txq_lock[q_idx], flags);

	if ((tx_delay_ctl->que_agg_en) &&
		(que->Number > 0) &&
		(que->Number < tx_delay_ctl->tx_process_batch_cnt) &&
		(pkt) &&
		(GET_OS_PKT_LEN(pkt) >= tx_delay_ctl->min_pkt_len) &&
		(GET_OS_PKT_LEN(pkt) <= tx_delay_ctl->max_pkt_len) &&
		(!tx_delay_ctl->force_deq)) {

		if (!is_udp_packet(pAd, pkt)) {
			if (!tx_delay_ctl->que_agg_timer_running) {
				RTMPSetTimer(&tx_delay_ctl->que_agg_timer, tx_delay_ctl->que_agg_timeout_value / 1000);
				tx_delay_ctl->que_agg_timer_running = TRUE;
			}

			return TRUE;
		}
	}

	return FALSE;
}
#endif

INT deq_packet_gatter(RTMP_ADAPTER *pAd, struct dequeue_info *deq_info, TX_BLK *pTxBlk)
{
	STA_TR_ENTRY *tr_entry;
	PQUEUE_ENTRY qEntry = NULL;
	PNDIS_PACKET pPacket;
	PQUEUE_HEADER pQueue;
	UCHAR q_idx = deq_info->cur_q;
	UCHAR wcid = deq_info->cur_wcid;
	struct wifi_dev *wdev = NULL;
	RTMP_ARCH_OP *arch_ops = &pAd->archOps;
	unsigned long IrqFlags = 0;
	INT ret = 0;

	tr_entry = &pAd->MacTab.tr_entry[wcid];

	MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("-->%s(): deq_info->wcid=%d, qidx=%d!\n",
			 __func__, wcid, q_idx));

#ifdef CONFIG_TX_DELAY
	if (ge_tx_deq_delay(pAd, tr_entry, q_idx))
		return NDIS_STATUS_FAILURE;
#endif

	deq_info->deq_pkt_cnt = 0;

	RTMP_IRQ_LOCK(&tr_entry->txq_lock[q_idx], IrqFlags);

	do {
		pQueue = &tr_entry->tx_queue[q_idx];
dequeue:
		qEntry = pQueue->Head;

		if (qEntry != NULL) {
			qEntry = RemoveHeadQueue(pQueue);
			TR_ENQ_COUNT_DEC(tr_entry);
			pPacket = QUEUE_ENTRY_TO_PACKET(qEntry);
			ASSERT(RTMP_GET_PACKET_WCID(pPacket) == wcid);

			if (pTxBlk->TotalFrameNum == 0) {
				wdev = wdev_search_by_pkt(pAd, pPacket);
				pTxBlk->resource_idx = arch_ops->get_resource_idx(pAd, wdev, TX_DATA, q_idx);

#if defined(VOW_SUPPORT) && defined(VOW_DVT)
				if (wcid <= (MAX_LEN_OF_MAC_TABLE - HW_BEACON_MAX_NUM)) {
					if (pAd->vow_sta_ac[wcid] != QID_AC_VO)
						pTxBlk->resource_idx = pAd->vow_sta_ac[wcid]; /* BK/BE/VI */
					else
						pTxBlk->resource_idx = HIF_TX_IDX4; /* VO */
				}
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\x1b[31m%s: wcid %d, ac %d, resource_idx %d\x1b[m\n",
							__func__, wcid, q_idx, pTxBlk->resource_idx));
#endif /* defined(VOW_SUPPORT) && defined(VOW_DVT) */

#ifdef RANDOM_PKT_GEN
				random_write_resource_idx(pAd, pTxBlk);
#endif
			}


			MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 ("-->%s(): GetPacket, wcid=%d, deq_pkt_cnt=%d, TotalFrameNum=%d, TotalFrameLen = %d\n",
					  __func__, wcid, deq_info->deq_pkt_cnt, pTxBlk->TotalFrameNum, pTxBlk->TotalFrameLen));

			pTxBlk->TxFrameType = tx_pkt_classification(pAd, pPacket, pTxBlk);

			if (pTxBlk->TxFrameType & TX_AMSDU_FRAME) {
				if (pTxBlk->TotalFrameNum > 0) {
					if ((!is_amsdu_frame(pAd, pPacket, pTxBlk))
						|| !check_amsdu_limit(pAd, pTxBlk, pPacket)) {
						InsertHeadQueue(pQueue, PACKET_TO_QUEUE_ENTRY(pPacket));
						TR_ENQ_COUNT_INC(tr_entry);
						goto start_kick;
					}
				}
			}
#ifdef HW_TX_AMSDU_SUPPORT
			else if (TX_BLK_TEST_FLAG(pTxBlk, fTX_HW_AMSDU)) {
				/*
				* Need to check if the packet is proper for HW AMSDU
				*/
				if (!is_amsdu_frame(pAd, pPacket, pTxBlk))
					TX_BLK_CLEAR_FLAG(pTxBlk, fTX_HW_AMSDU);
			}
#endif /* HW_TX_AMSDU_SUPPORT */

			pTxBlk->QueIdx = q_idx;

			ret = arch_ops->check_hw_resource(pAd, wdev, pTxBlk->resource_idx);
			if (ret == NDIS_STATUS_RESOURCES) {
				/* if tx ring is full, re-check pdma status */
				mtd_free_txd(pAd, pTxBlk->resource_idx);
				ret = arch_ops->check_hw_resource(pAd, wdev, pTxBlk->resource_idx);
				if (!ret)
					arch_ops->set_resource_state(pAd, pTxBlk->resource_idx, TX_RING_HIGH);

				pci_dec_resource_full_cnt(pAd, pTxBlk->resource_idx);
			}

			if (ret) {
				InsertHeadQueue(pQueue, PACKET_TO_QUEUE_ENTRY(pPacket));
				TR_ENQ_COUNT_INC(tr_entry);

				/*
				 * Because of tx resource is not enough for this q_idx,
				 * set deque quota of this q_idx to 0 to let deq request can
				 * service next q_idx which may have tx resource
				 */
				if (deq_info->target_wcid < MAX_LEN_OF_TR_TABLE)
					deq_info->full_qid[q_idx] = TRUE;
				else
					deq_info->q_max_cnt[q_idx] = 0;

#ifdef DBG_DIAGNOSE

				if (pAd->DiagStruct.inited && pAd->DiagStruct.wcid == pTxBlk->Wcid) {
					struct dbg_diag_info *diag_info;

					diag_info = &pAd->DiagStruct.diag_info[pAd->DiagStruct.ArrayCurIdx];
#ifdef DBG_TXQ_DEPTH
					diag_info->deq_fail_no_resource_cnt[QueIdx]++;
#endif
				}

#endif

				goto start_kick;
			}

			pTxBlk->TotalFrameNum++;
			/* The real fragment number maybe vary */
			pTxBlk->TotalFragNum += RTMP_GET_PACKET_FRAGMENTS(pPacket);
			pTxBlk->TotalFrameLen += GET_OS_PKT_LEN(pPacket);

			if (pTxBlk->TotalFrameNum == 1) {
				pTxBlk->pPacket = pPacket;
				pTxBlk->wdev = wdev;
				pTxBlk->tr_entry = tr_entry;
				pTxBlk->HeaderBuf = arch_ops->get_hif_buf(pAd, pTxBlk,
						pTxBlk->resource_idx, pTxBlk->TxFrameType);
#if defined(P2P_SUPPORT) || defined(RT_CFG80211_P2P_SUPPORT) || defined(CFG80211_MULTI_STA)
				pTxBlk->OpMode = RTMP_GET_PACKET_OPMODE(pPacket);
#endif /* P2P_SUPPORT || RT_CFG80211_P2P_SUPPORT */
			}

			InsertTailQueue(&pTxBlk->TxPacketList, PACKET_TO_QUEUE_ENTRY(pPacket));
		} else {

			/*
			 * use to clear wcid of fifo_swq->swq[fifo_swq->deqIdx] to 0,
			 * that may happen when previos de-queue more than one packet
			 */
			if (pTxBlk->TxPacketList.Number == 0) {
#ifdef FQ_SCH_SUPPORT
				if (!(pAd->fq_ctrl.enable & FQ_READY))
#endif
				deq_info->deq_pkt_cnt++;
				MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						("<--%s():Try deQ a empty Q. pTxBlk.TxPktList.Num=%d, deq_info.pkt_cnt=%d\n",
						 __func__, pTxBlk->TxPacketList.Number, deq_info->pkt_cnt));
				break;
			}
		}

		if ((pTxBlk->TxFrameType & TX_AMSDU_FRAME) &&
			pQueue->Head) {
			goto dequeue;
		}

start_kick:

		if (pTxBlk->TxFrameType & TX_AMSDU_FRAME) {
			if (pTxBlk->TxPacketList.Number == 1)
				pTxBlk->TxFrameType = TX_LEGACY_FRAME;
#ifdef DBG_AMSDU
			if (pTxBlk->TxPacketList.Number == 1)
				tr_entry->amsdu_1++;
			else if (pTxBlk->TxPacketList.Number == 2)
				tr_entry->amsdu_2++;
			else if (pTxBlk->TxPacketList.Number == 3)
				tr_entry->amsdu_3++;
			else if (pTxBlk->TxPacketList.Number == 4)
				tr_entry->amsdu_4++;
#endif
		}

		MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("<--%s():pTxBlk.TxPktList.Num=%d, deq_info.pkt_cnt=%d\n",
				 __func__, pTxBlk->TxPacketList.Number, deq_info->pkt_cnt));
		break;
	} while (pTxBlk->TxPacketList.Number < deq_info->pkt_cnt);

	RTMP_IRQ_UNLOCK(&tr_entry->txq_lock[q_idx], IrqFlags);

	if (pTxBlk->TxPacketList.Number > 0)
		deq_info->deq_pkt_cnt += pTxBlk->TxPacketList.Number;

	return NDIS_STATUS_SUCCESS;

}

static NDIS_PACKET *get_high_prio_pkt(RTMP_ADAPTER *pAd)
{
	PQUEUE_ENTRY q_entry;

	RTMP_SEM_LOCK(&pAd->high_prio_que_lock);
	q_entry = RemoveHeadQueue(&pAd->high_prio_que);
	RTMP_SEM_UNLOCK(&pAd->high_prio_que_lock);

	if (q_entry)
		return QUEUE_ENTRY_TO_PACKET(q_entry);
	else
		return NULL;
}

static NDIS_PACKET *first_high_prio_pkt(RTMP_ADAPTER *pAd)
{
	PQUEUE_ENTRY q_entry;

	RTMP_SEM_LOCK(&pAd->high_prio_que_lock);
	q_entry = pAd->high_prio_que.Head;
	RTMP_SEM_UNLOCK(&pAd->high_prio_que_lock);

	if (q_entry)
		return QUEUE_ENTRY_TO_PACKET(q_entry);
	else
		return NULL;
}

static INT32 ge_deq_high_prio_pkt(RTMP_ADAPTER *pAd, TX_BLK *tx_blk)
{
	struct wifi_dev *wdev;
	struct wifi_dev_ops *wdev_ops;
	NDIS_PACKET *pkt = NULL;
	INT32 ret = 0;
	UCHAR wcid = 0;
	STA_TR_ENTRY *tr_entry = NULL;
	RTMP_ARCH_OP *arch_ops = &pAd->archOps;

	do {
		pkt = first_high_prio_pkt(pAd);

		if (!pkt) {
			return NDIS_STATUS_FAILURE;
		}

		wdev = wdev_search_by_pkt(pAd, pkt);
		wdev_ops = wdev->wdev_ops;
#if defined(VOW_SUPPORT) && defined(VOW_DVT)
		wcid = RTMP_GET_PACKET_WCID(pkt);
		tx_blk->QueIdx = RTMP_GET_PACKET_QUEIDX(pkt);

		if (wcid <= (MAX_LEN_OF_MAC_TABLE - HW_BEACON_MAX_NUM)) {
			if (pAd->vow_sta_ac[wcid] != QID_AC_VO)
				tx_blk->resource_idx = pAd->vow_sta_ac[wcid]; /* BK/BE/VI */
			else
				tx_blk->resource_idx = HIF_TX_IDX4; /* VO */
		} else
			tx_blk->resource_idx = arch_ops->get_resource_idx(pAd, wdev, TX_DATA_HIGH_PRIO, tx_blk->QueIdx);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\x1b[31m%s: QueIdx %d, resource_idx %d\x1b[m\n",
			__func__, tx_blk->QueIdx, tx_blk->resource_idx));
#else
		tx_blk->QueIdx = RTMP_GET_PACKET_QUEIDX(pkt);
		tx_blk->resource_idx = arch_ops->get_resource_idx(pAd, wdev, TX_DATA_HIGH_PRIO, tx_blk->QueIdx);
#endif /* defined(VOW_SUPPORT) && defined(VOW_DVT) */
#ifdef RANDOM_PKT_GEN
		random_write_resource_idx(pAd, tx_blk);
#endif
		ret = arch_ops->check_hw_resource(pAd, wdev, tx_blk->resource_idx);

		if (ret) {
			return NDIS_STATUS_FAILURE;
		}

		pkt = get_high_prio_pkt(pAd);
		wcid = RTMP_GET_PACKET_WCID(pkt);

		if (wcid >= MAX_LEN_OF_MAC_TABLE) {
			MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 ("%s(): WCID is invalid\n", __func__));
			RELEASE_NDIS_PACKET(pAd, pkt, NDIS_STATUS_FAILURE);
			continue;
		}

		tx_blk->wdev = wdev;

		tr_entry = &pAd->MacTab.tr_entry[wcid];

		TX_BLK_SET_FLAG(tx_blk, fTX_HIGH_PRIO);
		tx_blk->TotalFrameNum = 1;
		tx_blk->TotalFragNum = 1;
		tx_blk->tr_entry = tr_entry;
#if defined(VOW_SUPPORT) && defined(VOW_DVT)
		if (wcid <= (MAX_LEN_OF_MAC_TABLE - HW_BEACON_MAX_NUM))
			tx_blk->QueIdx = pAd->vow_sta_ac[wcid];
		else
			tx_blk->QueIdx = RTMP_GET_PACKET_QUEIDX(pkt);
#else
		tx_blk->QueIdx = RTMP_GET_PACKET_QUEIDX(pkt);
#endif /*#if defined(VOW_SUPPORT) && defined(VOW_DVT) */
		tx_blk->TotalFrameLen = GET_OS_PKT_LEN(pkt);
		tx_blk->pPacket = pkt;
		tx_blk->TxFrameType = tx_pkt_classification(pAd, tx_blk->pPacket, tx_blk);
		tx_blk->HeaderBuf = arch_ops->get_hif_buf(pAd, tx_blk, tx_blk->resource_idx, tx_blk->TxFrameType);
#ifdef HW_TX_AMSDU_SUPPORT
		if (TX_BLK_TEST_FLAG(tx_blk, fTX_HW_AMSDU)) {
				if (!is_amsdu_frame(pAd, tx_blk->pPacket, tx_blk))
					TX_BLK_CLEAR_FLAG(tx_blk, fTX_HW_AMSDU);
		}
#endif /* HW_TX_AMSDU_SUPPORT */
		InsertTailQueue(&tx_blk->TxPacketList, PACKET_TO_QUEUE_ENTRY(pkt));

		break;
	} while (1);

	if (tx_blk->TxFrameType & TX_AMSDU_FRAME) {
		if (tx_blk->TxPacketList.Number == 1)
			tx_blk->TxFrameType = TX_LEGACY_FRAME;
	}

	return NDIS_STATUS_SUCCESS;
}

static NDIS_PACKET *ge_get_psq_pkt(RTMP_ADAPTER *pAd, struct _STA_TR_ENTRY *tr_entry)
{
	PQUEUE_ENTRY q_entry;

	RTMP_SEM_LOCK(&tr_entry->ps_queue_lock);
	q_entry = RemoveHeadQueue(&tr_entry->ps_queue);
	RTMP_SEM_UNLOCK(&tr_entry->ps_queue_lock);

	if (q_entry)
		return QUEUE_ENTRY_TO_PACKET(q_entry);
	else
		return NULL;
}

static NDIS_PACKET *get_mgmt_pkt(RTMP_ADAPTER *pAd)
{
	PQUEUE_ENTRY q_entry;

	RTMP_SEM_LOCK(&pAd->mgmt_que_lock);
	q_entry = RemoveHeadQueue(&pAd->mgmt_que);
	RTMP_SEM_UNLOCK(&pAd->mgmt_que_lock);

	if (q_entry)
		return QUEUE_ENTRY_TO_PACKET(q_entry);
	else
		return NULL;
}

static NDIS_PACKET *first_mgmt_pkt(RTMP_ADAPTER *pAd)
{
	PQUEUE_ENTRY q_entry;

	RTMP_SEM_LOCK(&pAd->mgmt_que_lock);
	q_entry = pAd->mgmt_que.Head;
	RTMP_SEM_UNLOCK(&pAd->mgmt_que_lock);

	if (q_entry)
		return QUEUE_ENTRY_TO_PACKET(q_entry);
	else
		return NULL;
}

static INT32 ge_deq_mgmt_pkt(RTMP_ADAPTER *pAd, TX_BLK *tx_blk)
{
	struct wifi_dev *wdev;
	struct wifi_dev_ops *wdev_ops;
	NDIS_PACKET *pkt = NULL;
	INT32 ret = 0;
	UCHAR wcid = 0;
	STA_TR_ENTRY *tr_entry = NULL;
	RTMP_ARCH_OP *arch_ops = &pAd->archOps;

	do {
		pkt = first_mgmt_pkt(pAd);

		if (!pkt) {
			return NDIS_STATUS_FAILURE;
		}

		wdev = wdev_search_by_pkt(pAd, pkt);
		wdev_ops = wdev->wdev_ops;
		tx_blk->resource_idx = arch_ops->get_resource_idx(pAd, wdev, RTMP_GET_PACKET_TYPE(pkt), RTMP_GET_PACKET_QUEIDX(pkt));
		ret = arch_ops->check_hw_resource(pAd, wdev, tx_blk->resource_idx);

		if (ret) {
			return NDIS_STATUS_FAILURE;
		}

		pkt = get_mgmt_pkt(pAd);
		wcid = RTMP_GET_PACKET_WCID(pkt);

		if (wcid >= MAX_LEN_OF_MAC_TABLE) {
			MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 ("%s(): WCID is invalid\n", __func__));
			RELEASE_NDIS_PACKET(pAd, pkt, NDIS_STATUS_FAILURE);
			continue;
		}

		tx_blk->wdev = wdev;

		tr_entry = &pAd->MacTab.tr_entry[wcid];

		tx_blk->TotalFrameNum = 1;
		tx_blk->TotalFragNum = 1;
		tx_blk->tr_entry = tr_entry;
		tx_blk->QueIdx = RTMP_GET_PACKET_QUEIDX(pkt);
		tx_blk->TotalFrameLen = GET_OS_PKT_LEN(pkt);
		tx_blk->pPacket = pkt;
		tx_blk->TxFrameType = tx_pkt_classification(pAd, tx_blk->pPacket, tx_blk);
		tx_blk->HeaderBuf = arch_ops->get_hif_buf(pAd, tx_blk, tx_blk->resource_idx, tx_blk->TxFrameType);
#ifdef HW_TX_AMSDU_SUPPORT
		if (TX_BLK_TEST_FLAG(tx_blk, fTX_HW_AMSDU)) {
				if (!is_amsdu_frame(pAd, tx_blk->pPacket, tx_blk))
					TX_BLK_CLEAR_FLAG(tx_blk, fTX_HW_AMSDU);
		}
#endif /* HW_TX_AMSDU_SUPPORT */
		InsertTailQueue(&tx_blk->TxPacketList, PACKET_TO_QUEUE_ENTRY(pkt));

		break;
	} while (1);

	return NDIS_STATUS_SUCCESS;
}

static INT32 ge_deq_data_pkt(RTMP_ADAPTER *pAd, TX_BLK *tx_blk, INT32 max_cnt, struct dequeue_info *info)
{
	INT ret = NDIS_STATUS_SUCCESS;

	ge_deq_req(pAd, max_cnt, info);

	/* wait for next schedule period to service de-queue pkt */
	if (info->status == NDIS_STATUS_FAILURE)
		return NDIS_STATUS_FAILURE;

	MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(): deq_info:cur_wcid=%d, cur_qidx=%d, pkt_cnt=%d, pkt_bytes=%d\n", __func__, info->cur_wcid, info->cur_q, info->pkt_cnt, info->pkt_bytes));

	ret = deq_packet_gatter(pAd, info, tx_blk);

	if (!ret) {
#ifdef FQ_SCH_SUPPORT
		if (pAd->fq_ctrl.enable & FQ_READY)
			fq_del_report(pAd, info);
		else
#endif
		ge_deq_report(pAd, info);
	}
	return ret;
}

VOID ge_tx_pkt_deq_func(RTMP_ADAPTER *pAd)
{
	BOOLEAN need_schedule = (pAd->tx_dequeue_scheduable ? TRUE : FALSE);
	INT Count = 0;
	TX_BLK TxBlk, *pTxBlk = &TxBlk;
	UINT32 KickRingBitMap = 0;
	UINT32 idx = 0;
	struct wifi_dev *wdev;
	struct wifi_dev_ops *wdev_ops;
	RTMP_ARCH_OP *arch_ops = &pAd->archOps;
	struct dequeue_info deq_info = {0};
	INT32 max_cnt = MAX_TX_PROCESS;
	PKT_TOKEN_CB *pktTokenCb = (PKT_TOKEN_CB *)pAd->PktTokenCb;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 num_of_tx_ring = GET_NUM_OF_TX_RING(cap);
	struct qm_ops *qm_ops = pAd->qm_ops;

#ifdef CONFIG_TP_DBG
	struct tp_debug *tp_dbg = &pAd->tr_ctl.tp_dbg;
#endif

#ifdef CONFIG_TX_DELAY
	struct tx_delay_control *tx_delay_ctl = &pAd->tr_ctl.tx_delay_ctl;
#endif

	if (!pktTokenCb->tx_id_list.token_inited)
		return;

	if (RTMP_TEST_FLAG(pAd, TX_FLAG_STOP_DEQUEUE))
		return;

#ifdef ERR_RECOVERY
	if (IsStopingPdma(&pAd->ErrRecoveryCtl))
		return;
#endif /* ERR_RECOVERY */

	deq_info.target_wcid = WCID_ALL;
	deq_info.target_que = WMM_NUM_OF_AC;

	while (need_schedule) {
		NdisZeroMemory((UCHAR *)pTxBlk, sizeof(TX_BLK));

		if (!ge_deq_mgmt_pkt(pAd, pTxBlk))
			goto pkt_handle;

		if (!ge_deq_high_prio_pkt(pAd, pTxBlk))
			goto pkt_handle;

		if (qm_ops->deq_data_pkt(pAd, pTxBlk, max_cnt, &deq_info)) {
			need_schedule = FALSE;
			break;
		}

pkt_handle:
		if (pTxBlk->TotalFrameNum) {
			ASSERT(pTxBlk->wdev);
			wdev = pTxBlk->wdev;
			wdev_ops = wdev->wdev_ops;
			wdev_ops->tx_pkt_handle(pAd, wdev, pTxBlk);
#if defined(VOW_SUPPORT) && defined(VOW_DVT)
			if (pTxBlk->TxFrameType == TX_LEGACY_FRAME)
				KickRingBitMap |= vow_clone_legacy_frame(pAd, pTxBlk);
#endif /* defined(VOW_SUPPORT) && defined(VOW_DVT) */
			Count += pTxBlk->TotalFrameNum;
			KickRingBitMap |= (1 << pTxBlk->resource_idx);
		}
	}

#ifdef DBG_DEQUE
	MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("--->%s():DeQueueRule:WCID[%d], Que[%d]\n",
			  __func__, deq_info.target_wcid, deq_info.target_que));

	if (pAd->DiagStruct.inited) {
		struct dbg_diag_info *diag_info;

		diag_info = &pAd->DiagStruct.diag_info[pAd->DiagStruct.ArrayCurIdx];
		diag_info->deq_called++;
		diag_info->deq_round += round;

		if (Count < 8)
			diag_info->deq_cnt[Count]++;
		else
			diag_info->deq_cnt[8]++;
	}
#endif /* DBG_DEQUE */

	while (KickRingBitMap != 0 && idx < num_of_tx_ring) {
		if (KickRingBitMap & 0x1) {
			arch_ops->kickout_data_tx(pAd, pTxBlk, idx);
#ifdef CONFIG_TP_DBG
			tp_dbg->IoWriteTx++;
#endif
		}

		KickRingBitMap >>= 1;
		idx++;
	}

#ifdef CONFIG_TX_DELAY
	tx_delay_ctl->force_deq = FALSE;
#endif
}

static INT ge_bss_clean_queue(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev)
{
	INT sta_idx;
	INT qidx;
	struct _STA_TR_ENTRY *tr_entry;
#ifdef FQ_SCH_SUPPORT
	struct fq_stainfo_type *pfq_sta = NULL;
	INT frame_count = 0;
#endif
	/*TODO: add check de-queue task idle*/
	qm_leave_queue_pkt(wdev, &ad->mgmt_que, &ad->mgmt_que_lock);
	/*leave per sta/ac queue*/
	for (sta_idx = 0 ; sta_idx < MAX_LEN_OF_TR_TABLE ; sta_idx++) {

		tr_entry = &ad->MacTab.tr_entry[sta_idx];

		if (tr_entry->wdev != wdev)
			continue;

		for (qidx = 0; qidx < WMM_QUE_NUM ; qidx++) {
#ifdef FQ_SCH_SUPPORT
			frame_count = tr_entry->tx_queue[qidx].Number;
			qm_leave_queue_pkt(wdev, &tr_entry->tx_queue[qidx], &tr_entry->txq_lock[qidx]);
			pfq_sta = &tr_entry->fq_sta_rec;
			frame_count = frame_count - tr_entry->tx_queue[qidx].Number;
			if (pfq_sta->status[qidx] == FQ_IN_LIST_STA)
				if (tr_entry->tx_queue[qidx].Number == 0)
					pfq_sta->status[qidx] = FQ_UN_CLEAN_STA;
				ad->fq_ctrl.frm_cnt[qidx] -= frame_count;
#else
			qm_leave_queue_pkt(wdev, &tr_entry->tx_queue[qidx], &tr_entry->txq_lock[qidx]);
#endif
	}
	}
#ifdef FQ_SCH_SUPPORT
	fq_clean_list(ad, WMM_NUM_OF_AC);
#endif
	return NDIS_STATUS_SUCCESS;
}

static INT ge_qm_init(RTMP_ADAPTER *pAd)
{
	UCHAR i;

	NdisAllocateSpinLock(pAd, &pAd->mgmt_que_lock);
	InitializeQueueHeader(&pAd->mgmt_que);
	NdisAllocateSpinLock(pAd, &pAd->high_prio_que_lock);
	InitializeQueueHeader(&pAd->high_prio_que);

	pAd->TxSwQMaxLen = MAX_PACKETS_IN_QUEUE;

	for (i = 0; i < WMM_NUM_OF_AC; i++) {
		NdisAllocateSpinLock(pAd, &pAd->tx_swq_lock[i]);
	}

	NdisZeroMemory(pAd->tx_swq, sizeof(pAd->tx_swq));

	for (i = 0; i < WMM_NUM_OF_AC; i++) {
		pAd->tx_swq[i].low_water_mark = 5;
		pAd->tx_swq[i].high_water_mark = TX_SWQ_FIFO_LEN>>1;
	}

#ifdef DBG_AMSDU
	pAd->dbg_time_slot = 0;
	RTMPInitTimer(pAd, &pAd->amsdu_history_timer, GET_TIMER_FUNCTION(amsdu_history_exec), pAd, TRUE);
	RTMPSetTimer(&pAd->amsdu_history_timer, 1000);
#endif
	pAd->amsdu_fix_num = NUM_OF_MSDU_ID_IN_TXD;
	pAd->amsdu_fix = FALSE;
	pAd->amsdu_max_num = NUM_OF_MSDU_ID_IN_TXD;

#ifdef FQ_SCH_SUPPORT
	fq_init(pAd);
#endif
	return NDIS_STATUS_SUCCESS;
}

VOID RTMPDeQueuePacket(
	IN RTMP_ADAPTER *pAd,
	IN BOOLEAN in_hwIRQ,
	IN UCHAR QIdx,
	IN INT wcid,
	IN INT max_cnt)
{
}

struct qm_ops ge_qm_ops = {
	.init = ge_qm_init,
	.exit = ge_qm_exit,
	.enq_mgmtq_pkt = ge_enq_mgmtq_pkt,
	.enq_dataq_pkt = ge_enq_dataq_pkt,
	.get_psq_pkt = ge_get_psq_pkt,
	.enq_psq_pkt = ge_enq_psq_pkt,
	.schedule_tx_que = ge_schedule_tx_que,
	.sta_clean_queue = ge_sta_clean_queue,
	.sta_dump_queue = ge_sta_dump_queue,
	.bss_clean_queue = ge_bss_clean_queue,
	.dump_all_sw_queue = ge_dump_all_sw_queue,
	.deq_data_pkt = ge_deq_data_pkt,
};

extern struct qm_ops fp_qm_ops;
extern struct qm_ops ge_fair_qm_ops;
extern struct qm_ops fp_fair_qm_ops;

VOID qm_leave_queue_pkt(struct wifi_dev *wdev, struct _QUEUE_HEADER *queue, NDIS_SPIN_LOCK *lock)
{
	struct _RTMP_ADAPTER *ad = wdev->sys_handle;
	struct _QUEUE_HEADER local_q;
	struct _QUEUE_ENTRY *q_entry;
	NDIS_PACKET *pkt;

	InitializeQueueHeader(&local_q);
	RTMP_SEM_LOCK(lock);

	/*remove entry owned by wdev*/
	do {
		q_entry = RemoveHeadQueue(queue);

		if (!q_entry)
			break;

		pkt = QUEUE_ENTRY_TO_PACKET(q_entry);

		if (RTMP_GET_PACKET_WDEV(pkt) == wdev->wdev_idx)
			RELEASE_NDIS_PACKET(ad, pkt, NDIS_STATUS_SUCCESS);
		else
			InsertTailQueue(&local_q, q_entry);
	} while (1);

	/*re-enqueue other entries*/
	do {
		q_entry = RemoveHeadQueue(&local_q);

		if (!q_entry)
			break;

		InsertTailQueue(queue, q_entry);
	} while (1);

	RTMP_SEM_UNLOCK(lock);
}

static INT qm_for_wsys_notify_handle(struct notify_entry *ne, INT event_id, VOID *data)
{
	INT ret = NOTIFY_STAT_OK;
	struct wsys_notify_info *info = data;
	struct _RTMP_ADAPTER *ad = ne->priv;
	struct wifi_dev *wdev = info->wdev;
	struct qm_ops *qm = ad->qm_ops;

	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		("%s(): event_id: %d, wdev=%d\n", __func__, event_id, info->wdev->wdev_idx));

	switch (event_id) {
	case WSYS_NOTIFY_CLOSE:
		if (qm->bss_clean_queue)
			qm->bss_clean_queue(ad, wdev);
		break;
	case WSYS_NOTIFY_OPEN:
	case WSYS_NOTIFY_CONNT_ACT:
	case WSYS_NOTIFY_DISCONNT_ACT:
	case WSYS_NOTIFY_LINKUP:
	case WSYS_NOTIFY_LINKDOWN:
	case WSYS_NOTIFY_STA_UPDATE:
	default:
		break;
	}
	return ret;
}

static INT qm_notify_register(struct _RTMP_ADAPTER *ad)
{
	INT ret;
	struct notify_entry *ne = &ad->qm_wsys_ne;

	/*fill notify entry for wifi system chain*/
	ne->notify_call = qm_for_wsys_notify_handle;
	ne->priority = WSYS_NOTIFY_PRIORITY_QM;
	ne->priv = ad;
	/*register wifi system notify chain*/
	ret = register_wsys_notifier(&ad->WifiSysInfo, ne);
	return ret;
}

static INT qm_notify_unregister(struct _RTMP_ADAPTER *ad)
{
	INT ret;
	struct notify_entry *ne = &ad->qm_wsys_ne;

	/*register wifi system notify chain*/
	ret = unregister_wsys_notifier(&ad->WifiSysInfo, ne);
	return ret;
}


INT qm_init(RTMP_ADAPTER *pAd)
{
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	struct qm_ops **qm_ops = &pAd->qm_ops;
#ifdef CONFIG_TX_DELAY
	struct tx_delay_control *tx_delay_ctl = &pAd->tr_ctl.tx_delay_ctl;
#endif
	INT ret;

	if (cap->qm == GENERIC_QM) {
		*qm_ops = &ge_qm_ops;
	} else if (cap->qm == FAST_PATH_QM) {
		*qm_ops = &fp_qm_ops;
	} else if (cap->qm == GENERIC_FAIR_QM) {
		*qm_ops = &ge_fair_qm_ops;
	} else if (cap->qm == FAST_PATH_FAIR_QM) {
		*qm_ops = &fp_fair_qm_ops;
	}

	ret = (*qm_ops)->init(pAd);

	pAd->fp_que_max_size = DATA_QUE_MAX_NUMS;
#ifdef CONFIG_TX_DELAY
	if (cap->tx_delay_support) {
		tx_delay_ctl->force_deq = FALSE;
		tx_delay_ctl->que_agg_en = FALSE;
		tx_delay_ctl->que_agg_timeout_value = QUE_AGG_TIMEOUT;
		tx_delay_ctl->min_pkt_len = MIN_AGG_PKT_LEN;
		tx_delay_ctl->max_pkt_len = MAX_AGG_PKT_LEN;
		tx_delay_ctl->que_agg_timer_running = FALSE;

		if (IS_ASIC_CAP(pAd, fASIC_CAP_MCU_OFFLOAD)) {
			tx_delay_ctl->tx_process_batch_cnt = TX_BATCH_CNT;

			MtCmdCr4Set(pAd, CR4_SET_ID_CONFIG_TX_DELAY_MODE,
					TX_DELAY_MODE_ARG1_TX_BATCH_CNT, tx_delay_ctl->tx_process_batch_cnt);

			MtCmdCr4Set(pAd, CR4_SET_ID_CONFIG_TX_DELAY_MODE,
					TX_DELAY_MODE_ARG1_TX_DELAY_TIMEOUT_US, tx_delay_ctl->que_agg_timeout_value);

			MtCmdCr4Set(pAd, CR4_SET_ID_CONFIG_TX_DELAY_MODE,
					TX_DELAY_MODE_ARG1_PKT_LENGTHS, tx_delay_ctl->min_pkt_len);
		} else {
			tx_delay_ctl->tx_process_batch_cnt = TX_BATCH_CNT;
			RTMPInitTimer(pAd, &tx_delay_ctl->que_agg_timer, GET_TIMER_FUNCTION(que_agg_timeout), pAd, FALSE);
		}
	} else {
		tx_delay_ctl->que_agg_en = FALSE;
	}
#endif
	/*register qm related notify chain*/
	qm_notify_register(pAd);
	return ret;
}

INT qm_exit(RTMP_ADAPTER *pAd)
{
	struct qm_ops *ops = pAd->qm_ops;
	INT ret;

	/*unregister qm related notify chain*/
	qm_notify_unregister(pAd);
	ret = ops->exit(pAd);
	return ret;
}

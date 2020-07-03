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
#include "mcu/mt_cmd.h"

#ifdef FQ_SCH_SUPPORT
static INT fq_reset_list_entry(RTMP_ADAPTER *pAd, UCHAR qidx, UCHAR wcid);
static INT fq_add_list(RTMP_ADAPTER *pAd, UCHAR qidx, STA_TR_ENTRY *tr_entry);
static INT fq_schedule_tx_que(RTMP_ADAPTER *pAd);
static INT fq_del_report_v2(RTMP_ADAPTER *pAd, struct dequeue_info *info);
static UINT16 fq_del_list_v2(RTMP_ADAPTER *pAd, struct dequeue_info *info, CHAR deq_qid, UINT32 *tx_quota);

extern INT32 fp_fair_enq_dataq_pkt(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PNDIS_PACKET pkt, UCHAR q_idx);
extern INT32 fp_enq_dataq_pkt(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PNDIS_PACKET pkt, UCHAR q_idx);
extern struct qm_ops ge_qm_ops;
extern struct qm_ops fp_qm_ops;
extern struct qm_ops fp_fair_qm_ops;
struct qm_ops ge_fair_qm_ops;

static inline UINT32 fq_get_swq_free_num(RTMP_ADAPTER *pAd, UINT8 q_idx)
{
	return (FQ_PER_AC_LIMIT - pAd->fq_ctrl.frm_cnt[q_idx]);
}

INT fq_init(RTMP_ADAPTER *pAd)
{
	INT i, j;
	STA_TR_ENTRY *tr_entry;
	struct fq_stainfo_type *pfq_sta = NULL;
	UINT32 fq_en = 0, factor = 0;
	ULONG IrqFlags = 0;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	struct qm_ops **qm_ops = &pAd->qm_ops;

#ifdef FQ_SCH_DBG_SUPPORT
	for (i = 0; i < WMM_NUM_OF_AC; i++) {
		pAd->fq_ctrl.prev_wcid[i] = -1;
		pAd->fq_ctrl.prev_kick_cnt[i] = 0;
	}
	pAd->fq_ctrl.prev_qidx = -1;
#endif

	if ((pAd->fq_ctrl.enable & FQ_EN) == 0)
		return 0;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("Fair Queueing Scheduler Initialization...\n"));
	fq_en = pAd->fq_ctrl.enable;
	factor = pAd->fq_ctrl.factor;
	os_zero_mem(&pAd->fq_ctrl, sizeof(struct fq_ctrl_type));
	pAd->fq_ctrl.enable = fq_en;
	pAd->fq_ctrl.factor = factor;


	for (i = 0; i < WMM_NUM_OF_AC; i++) {
		RTMP_IRQ_LOCK(&pAd->tx_swq_lock[i], IrqFlags);
		InitializeQueueHeader(&pAd->fq_ctrl.fq[i]);
		for (j = 0; j < FQ_BITMAP_DWORD; j++) {
			pAd->fq_ctrl.list_map[i][j] = 0;
			pAd->fq_ctrl.no_packet_chk_map[i][j] = 0;
		}
		pAd->fq_ctrl.frm_cnt[i] = 0;
		pAd->fq_ctrl.drop_cnt[i] = 0;
		RTMP_IRQ_UNLOCK(&pAd->tx_swq_lock[i], IrqFlags);
	}
	for (j = 0; j < MAX_LEN_OF_MAC_TABLE ; j++) {
		tr_entry = &pAd->MacTab.tr_entry[j];
		pfq_sta = &tr_entry->fq_sta_rec;
		for (i = 0; i < WMM_NUM_OF_AC; i++) {
			os_zero_mem(pfq_sta, sizeof(struct fq_stainfo_type));
			NdisAllocateSpinLock(pAd, &pfq_sta->lock[i]);
		}
		fq_reset_list_entry(pAd, WMM_NUM_OF_AC, j);
	}

	for (i = 0; i < WMM_NUM_OF_AC; i++) {
		RTMP_IRQ_LOCK(&pAd->tx_swq_lock[i], IrqFlags);
		for (j = 0; j < MAX_LEN_OF_MAC_TABLE ; j++) {
			tr_entry = &pAd->MacTab.tr_entry[j];
			if (tr_entry) {
				RTMP_SPIN_LOCK(&tr_entry->txq_lock[i]);
				if (tr_entry->tx_queue[i].Number > 0)
					fq_add_list(pAd, i, tr_entry);
				RTMP_SPIN_UNLOCK(&tr_entry->txq_lock[i]);
			}
		}
		RTMP_IRQ_UNLOCK(&pAd->tx_swq_lock[i], IrqFlags);
	}

	for (i = 0; i < WMM_NUM_OF_AC; i++)
		pAd->tx_swq[i].high_water_mark = FQ_PER_AC_LIMIT>>1;

	if (cap->qm == FAST_PATH_FAIR_QM) {
		(*qm_ops)->enq_dataq_pkt = fp_fair_enq_dataq_pkt;
		(*qm_ops)->schedule_tx_que = fq_schedule_tx_que;

		pAd->TxSwQMaxLen = MAX_PACKETS_IN_QUEUE;

		for (i = 0; i < WMM_NUM_OF_AC; i++)
			NdisAllocateSpinLock(pAd, &pAd->tx_swq_lock[i]);

		os_zero_mem(pAd->tx_swq, sizeof(pAd->tx_swq));
	} else
		(*qm_ops)->schedule_tx_que = fq_schedule_tx_que;

	pAd->fq_ctrl.msdu_threshold = FQ_PLE_SIZE + cap->tx_ring_size;

	for (i = 0; i < WMM_NUM_OF_AC; i++)
		pAd->fq_ctrl.srch_pos[i] = 1;

	pAd->fq_ctrl.enable |= FQ_READY;

	return NDIS_STATUS_SUCCESS;

}

INT fq_deinit(RTMP_ADAPTER *pAd)
{
	STA_TR_ENTRY *tr_entry = NULL;
	struct fq_stainfo_type *pfq_sta = NULL;
	INT i, j;
	UINT32 prev_enable;

	pAd->fq_ctrl.enable &= ~FQ_READY;
	prev_enable = pAd->fq_ctrl.enable & (FQ_EN | FQ_NEED_ON);

	for (j = 0; j < MAX_LEN_OF_MAC_TABLE ; j++) {
		tr_entry = &pAd->MacTab.tr_entry[j];
		pfq_sta = &tr_entry->fq_sta_rec;
		for (i = 0; i < WMM_NUM_OF_AC; i++) {
			NdisFreeSpinLock(&pfq_sta->lock[i]);
			os_zero_mem(pfq_sta, sizeof(struct fq_stainfo_type));
		}
		fq_reset_list_entry(pAd, WMM_NUM_OF_AC, j);
	}

	os_zero_mem(&pAd->fq_ctrl, sizeof(struct fq_ctrl_type));

	pAd->fq_ctrl.enable = prev_enable | FQ_NO_PKT_STA_KEEP_IN_LIST | FQ_ARRAY_SCH;
	pAd->fq_ctrl.factor = 2;

	return 0;
}

INT fq_exit(RTMP_ADAPTER *pAd)
{
	if (pAd->fq_ctrl.enable & FQ_READY)
		fq_deinit(pAd);

	return NDIS_STATUS_SUCCESS;
}

static INT fq_add_list(RTMP_ADAPTER *pAd, UCHAR qidx, STA_TR_ENTRY *tr_entry)
{
	struct fq_stainfo_type *pfq_sta = NULL;
	UINT32 IsHit = 0;
	INT ret = 1;

	pfq_sta = &tr_entry->fq_sta_rec;
	IsHit = pAd->fq_ctrl.list_map[qidx][tr_entry->wcid>>FQ_BITMAP_SHIFT] &
					(1<<(tr_entry->wcid & FQ_BITMAP_MASK));

	if (pfq_sta && ((IsHit == 0) || (pAd->fq_ctrl.fq[qidx].Head == NULL))) {
		if (pAd->fq_ctrl.fq[qidx].Head == NULL) {
			pAd->fq_ctrl.fq[qidx].Number = 0;
			pAd->fq_ctrl.frm_cnt[qidx] = 0;
		}
		pAd->fq_ctrl.frm_cnt[qidx] += tr_entry->tx_queue[qidx].Number;
		InsertTailQueue(&pAd->fq_ctrl.fq[qidx], &(pfq_sta->Entry[qidx]));
		pAd->fq_ctrl.list_map[qidx][tr_entry->wcid>>FQ_BITMAP_SHIFT]
			|= (1<<(tr_entry->wcid & FQ_BITMAP_MASK));
		pfq_sta->kickPktCnt[qidx] = 0;
		RTMP_SEM_LOCK(&pfq_sta->lock[qidx]);
		pfq_sta->status[qidx] = FQ_IN_LIST_STA;
		pfq_sta->wcid = tr_entry->wcid;
		RTMP_SEM_UNLOCK(&pfq_sta->lock[qidx]);
		ret = 0;
	}

	return ret;
}

UINT16 fq_del_list(RTMP_ADAPTER *pAd, struct dequeue_info *info, CHAR deq_qid, UINT32 *tx_quota)
{
	INT tot_sta;
	UINT16 deq_wcid = 0, wcid = 0, j;
	QUEUE_ENTRY *pEntry = NULL, *pPrevEntry = NULL, *pTmpEntry = NULL;
	STA_TR_ENTRY *tr_entry = NULL;
	struct fq_stainfo_type *pfq_sta = NULL;
	INT32 quota = 0;
	UINT32 list_bitmap[FQ_BITMAP_DWORD] = {0};

	if (pAd->fq_ctrl.enable & FQ_ARRAY_SCH)
		return fq_del_list_v2(pAd, info, deq_qid, tx_quota);

	if (info->q_max_cnt[deq_qid] == 0)
		return 0;

	for (j = 0; j < FQ_BITMAP_DWORD; j++)
		if (pAd->fq_ctrl.list_map[deq_qid][j] != 0)
				break;
	if (j == FQ_BITMAP_DWORD)
		return 0;

#if FQ_SCH_DBG_SUPPORT
	if (pAd->fq_ctrl.frm_max_cnt[deq_qid] < pAd->fq_ctrl.frm_cnt[deq_qid])
		pAd->fq_ctrl.frm_max_cnt[deq_qid] = pAd->fq_ctrl.frm_cnt[deq_qid];
#endif
	tot_sta = pAd->fq_ctrl.fq[deq_qid].Number;

	pEntry = pAd->fq_ctrl.fq[deq_qid].Head;
	pPrevEntry = pEntry;
	while (tot_sta > 0) {
		tot_sta--;
		if (pEntry) {
			pfq_sta = container_of(pEntry, struct fq_stainfo_type, Entry[deq_qid]);
			wcid = pfq_sta->wcid;
			tr_entry = &pAd->MacTab.tr_entry[wcid];
			list_bitmap[tr_entry->wcid >> FQ_BITMAP_SHIFT] |= 1<<(tr_entry->wcid & FQ_BITMAP_MASK);
			if (IS_ENTRY_NONE(tr_entry)) {
				if (pEntry == pAd->fq_ctrl.fq[deq_qid].Head) {
					pTmpEntry = RemoveHeadQueue(&pAd->fq_ctrl.fq[deq_qid]);
					pPrevEntry = pAd->fq_ctrl.fq[deq_qid].Head;
					pEntry = pPrevEntry;
				} else {
					pTmpEntry = pEntry;
					PickFromQueue(&pAd->fq_ctrl.fq[deq_qid], pPrevEntry, pTmpEntry);
					pEntry = pPrevEntry->Next;
				}

				pAd->fq_ctrl.list_map[deq_qid][wcid>>FQ_BITMAP_SHIFT]
					&= ~(1<<(wcid & FQ_BITMAP_MASK));
				pAd->fq_ctrl.no_packet_chk_map[deq_qid][wcid>>FQ_BITMAP_SHIFT]
					&= ~(1<<(wcid & FQ_BITMAP_MASK));

				fq_reset_list_entry(pAd, deq_qid, pfq_sta->wcid);
				continue;
			}
			if (tr_entry->PsMode == PWR_SAVE) {
				pPrevEntry = pEntry;
				pEntry = pEntry->Next;
				continue;
			}
			if (tr_entry->tx_queue[deq_qid].Number <= 0) {
				if (pEntry == pAd->fq_ctrl.fq[deq_qid].Head) {
					pTmpEntry = RemoveHeadQueue(&pAd->fq_ctrl.fq[deq_qid]);
					pPrevEntry = pAd->fq_ctrl.fq[deq_qid].Head;
					pEntry = pPrevEntry;
				} else {
					pTmpEntry = pEntry;
					PickFromQueue(&pAd->fq_ctrl.fq[deq_qid], pPrevEntry, pTmpEntry);
					pEntry = pPrevEntry->Next;
				}
				if (!(pAd->fq_ctrl.enable & FQ_NO_PKT_STA_KEEP_IN_LIST)) {
					pAd->fq_ctrl.list_map[deq_qid][wcid>>FQ_BITMAP_SHIFT]
						&= ~(1<<(wcid & FQ_BITMAP_MASK));
					fq_reset_list_entry(pAd, deq_qid, pfq_sta->wcid);
				} else
					InsertTailQueue(&pAd->fq_ctrl.fq[deq_qid], pTmpEntry);
				continue;
			}

			if ((!(pAd->fq_ctrl.enable & FQ_SKIP_SINGLE_STA_CASE)) && (pAd->fq_ctrl.nactive == 1)) {
				*tx_quota = (tr_entry->tx_queue[deq_qid].Number <
								MAX_TX_PROCESS) ?
								tr_entry->tx_queue[deq_qid].Number :
								MAX_TX_PROCESS;
			} else {
				quota = (pfq_sta->thMax[deq_qid]<<pAd->fq_ctrl.factor) -
					(INT16)(pfq_sta->macInQLen[deq_qid]-pfq_sta->macOutQLen[deq_qid]);
				if (quota <= 0)
					*tx_quota = MIN_HT_THMAX;
				else if (quota >= MAX_FQ_VHT_AMPDU_NUM)
					*tx_quota = MAX_FQ_VHT_AMPDU_NUM;
				else
					*tx_quota = quota;
			}
			deq_wcid = wcid;
			pAd->fq_ctrl.pPrevEntry[deq_qid] = pPrevEntry;
#ifdef FQ_SCH_DBG_SUPPORT
			pAd->fq_ctrl.sta_in_head[deq_qid][deq_wcid]++;
			if (pfq_sta->qlen_max_cnt[deq_qid] < tr_entry->tx_queue[deq_qid].Number)
				pfq_sta->qlen_max_cnt[deq_qid] = tr_entry->tx_queue[deq_qid].Number;
#endif
			break;
		}
		deq_wcid = 0;
		break;
	} /* while */

	if (deq_wcid == 0) {
		for (j = 0; j < MAX_LEN_OF_MAC_TABLE; j++) {
			if ((pAd->fq_ctrl.list_map[deq_qid][j >> FQ_BITMAP_SHIFT] & (1 << (j & FQ_BITMAP_MASK)))
				&& (!(list_bitmap[j >> FQ_BITMAP_SHIFT] & (1 << (j & FQ_BITMAP_MASK))))) {
				/* add to list */
				tr_entry = &pAd->MacTab.tr_entry[j];
				pAd->fq_ctrl.list_map[deq_qid][j >> FQ_BITMAP_SHIFT] &=
					~(1 << (j & FQ_BITMAP_MASK));
				RTMP_SPIN_LOCK(&tr_entry->txq_lock[deq_qid]);
				fq_add_list(pAd, deq_qid, tr_entry);
				RTMP_SPIN_UNLOCK(&tr_entry->txq_lock[deq_qid]);
			}
		}
		pAd->fq_ctrl.pPrevEntry[deq_qid] = NULL;
	}

	return deq_wcid;
}

INT fq_del_report(RTMP_ADAPTER *pAd, struct dequeue_info *info)
{
	ULONG IrqFlags = 0;
	UCHAR qidx = info->cur_q;
	UCHAR wcid = info->cur_wcid;
	struct fq_stainfo_type *pfq_sta = NULL;
	QUEUE_ENTRY *pEntry = NULL, *pTmpEntry = NULL;
	STA_TR_ENTRY *tr_entry = NULL;
	INT prev_wcid_tmp;
	INT ret = NDIS_STATUS_SUCCESS;

	if (pAd->fq_ctrl.enable & FQ_ARRAY_SCH)
		return fq_del_report_v2(pAd, info);

	RTMP_IRQ_LOCK(&pAd->tx_swq_lock[qidx], IrqFlags);

	if (info->q_max_cnt[info->cur_q] > 0)
		info->q_max_cnt[info->cur_q] -= info->deq_pkt_cnt;

	if (info->target_wcid < MAX_LEN_OF_TR_TABLE)
		info->pkt_cnt -= info->deq_pkt_cnt;

#ifdef FQ_SCH_DBG_SUPPORT
	if (pAd->fq_ctrl.dbg_en&FQ_DBG_DUMP_STA_LOG) {
		if (pAd->fq_ctrl.prev_qidx >= 0) {
			UCHAR the_prev_qidx = pAd->fq_ctrl.prev_qidx;

			if (the_prev_qidx == qidx)
				prev_wcid_tmp = pAd->fq_ctrl.prev_wcid[qidx];
			else
				prev_wcid_tmp = pAd->fq_ctrl.prev_wcid[the_prev_qidx];

			if ((prev_wcid_tmp > 0) && (prev_wcid_tmp < MAX_LEN_OF_MAC_TABLE)) {
				if ((pAd->fq_ctrl.prev_wcid[qidx] == wcid) &&
					((pAd->fq_ctrl.fq[qidx].Number >= 1) || (!pAd->fq_ctrl.enable)
					|| (pAd->fq_ctrl.nactive == 1))) {
					tr_entry = &pAd->MacTab.tr_entry[wcid];
					pfq_sta = &tr_entry->fq_sta_rec;
					pAd->fq_ctrl.prev_kick_cnt[qidx] += info->deq_pkt_cnt;
					RTMP_SEM_LOCK(&pfq_sta->lock[qidx]);
					if (pfq_sta->KMAX < pAd->fq_ctrl.prev_kick_cnt[qidx])
						pfq_sta->KMAX = pAd->fq_ctrl.prev_kick_cnt[qidx];
					RTMP_SEM_UNLOCK(&pfq_sta->lock[qidx]);
				} else {
					tr_entry = &pAd->MacTab.tr_entry[pAd->fq_ctrl.prev_wcid[qidx]];
					pfq_sta = &tr_entry->fq_sta_rec;
					RTMP_SEM_LOCK(&pfq_sta->lock[qidx]);
					if (pfq_sta->KMAX < pAd->fq_ctrl.prev_kick_cnt[the_prev_qidx])
						pfq_sta->KMAX = pAd->fq_ctrl.prev_kick_cnt[the_prev_qidx];
					pAd->fq_ctrl.prev_kick_cnt[qidx] = info->deq_pkt_cnt;
					RTMP_SEM_UNLOCK(&pfq_sta->lock[qidx]);
				}
			} else
				pAd->fq_ctrl.prev_kick_cnt[qidx] = info->deq_pkt_cnt;
		} else
			pAd->fq_ctrl.prev_kick_cnt[qidx] = info->deq_pkt_cnt;
		pAd->fq_ctrl.prev_wcid[qidx] = wcid;
		pAd->fq_ctrl.prev_qidx = qidx;
	}
#endif

	tr_entry = &pAd->MacTab.tr_entry[wcid];
	pfq_sta = &tr_entry->fq_sta_rec;
	pEntry = &pfq_sta->Entry[qidx];

	if (info->deq_pkt_cnt > 0) {
		RTMP_SEM_LOCK(&pfq_sta->lock[qidx]);
		pfq_sta->kickPktCnt[qidx] += info->deq_pkt_cnt;
		pfq_sta->macInQLen[qidx] += info->deq_pkt_cnt;
		RTMP_SEM_UNLOCK(&pfq_sta->lock[qidx]);
		if (pAd->fq_ctrl.frm_cnt[qidx] >= info->deq_pkt_cnt)
			pAd->fq_ctrl.frm_cnt[qidx] -= info->deq_pkt_cnt;
		else
			pAd->fq_ctrl.frm_cnt[qidx] = 0;
	}

	if (tx_flow_check_state(pAd, NO_ENOUGH_SWQ_SPACE, qidx) &&
	    fq_get_swq_free_num(pAd, qidx) > pAd->tx_swq[qidx].high_water_mark)
		tx_flow_set_state_block(pAd, NULL, NO_ENOUGH_SWQ_SPACE, FALSE, qidx);

	RTMP_SPIN_LOCK(&tr_entry->txq_lock[qidx]);
	if (tr_entry->tx_queue[info->cur_q].Number > 0) {
		if ((pAd->fq_ctrl.enable & FQ_SKIP_SINGLE_STA_CASE) || (pAd->fq_ctrl.nactive > 1))
		if ((pfq_sta->thMax[qidx]<<pAd->fq_ctrl.factor) <=
			(INT16)(pfq_sta->macInQLen[qidx] - pfq_sta->macOutQLen[qidx])) {
			pfq_sta->kickPktCnt[qidx] = 0;
			if (pAd->fq_ctrl.fq[qidx].Number > 1) {
					if (pEntry == pAd->fq_ctrl.fq[qidx].Head) {
						pTmpEntry = RemoveHeadQueue(&pAd->fq_ctrl.fq[qidx]);
					} else {
						if (pAd->fq_ctrl.pPrevEntry[qidx] == NULL) {
							MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
								("-->%s(%d): STA%d[%d],pRrevEntry is NULL\n",
								__func__, __LINE__, wcid, qidx));
							ret = NDIS_STATUS_FAILURE;
							goto EXIT;
						}

						pTmpEntry = pEntry;
						PickFromQueue(&pAd->fq_ctrl.fq[qidx],
								pAd->fq_ctrl.pPrevEntry[qidx], pTmpEntry);
					}
					InsertTailQueue(&pAd->fq_ctrl.fq[qidx], pTmpEntry);
			}
		}
	} else {
		if (pAd->fq_ctrl.enable & FQ_NO_PKT_STA_KEEP_IN_LIST) {
			if (pAd->fq_ctrl.fq[qidx].Number > 1) {
				if (pEntry == pAd->fq_ctrl.fq[qidx].Head) {
					pTmpEntry = RemoveHeadQueue(&pAd->fq_ctrl.fq[qidx]);
				} else {
					if (pAd->fq_ctrl.pPrevEntry[qidx] == NULL) {
						MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
							("-->%s(%d): STA%d[%d],pRrevEntry is NULL\n",
							__func__, __LINE__, wcid, qidx));
						ret = NDIS_STATUS_FAILURE;
						goto EXIT;
					}
					pTmpEntry = pEntry;
					PickFromQueue(&pAd->fq_ctrl.fq[qidx], pAd->fq_ctrl.pPrevEntry[qidx],
							pTmpEntry);
				}
				InsertTailQueue(&pAd->fq_ctrl.fq[qidx], pTmpEntry);
			}
				pfq_sta->kickPktCnt[qidx] = 0;
		} else {
			if (pEntry == pAd->fq_ctrl.fq[qidx].Head) {
				pTmpEntry = RemoveHeadQueue(&pAd->fq_ctrl.fq[qidx]);
			} else {
				if (pAd->fq_ctrl.pPrevEntry[qidx] == NULL) {
					MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						("-->%s(%d): STA%d[%d],pRrevEntry is NULL\n",
						__func__, __LINE__, wcid, qidx));
					ret = NDIS_STATUS_FAILURE;
					goto EXIT;
				}
				pTmpEntry = pEntry;
				PickFromQueue(&pAd->fq_ctrl.fq[qidx], pAd->fq_ctrl.pPrevEntry[qidx], pTmpEntry);
			}
			pAd->fq_ctrl.list_map[qidx][wcid>>FQ_BITMAP_SHIFT] &= ~(1<<(wcid & FQ_BITMAP_MASK));
			pfq_sta = container_of(pEntry, struct fq_stainfo_type, Entry[qidx]);
			RTMP_SEM_LOCK(&pfq_sta->lock[qidx]);
			fq_reset_list_entry(pAd, qidx, wcid);
			RTMP_SEM_UNLOCK(&pfq_sta->lock[qidx]);
		}
	}
EXIT:
	RTMP_SPIN_UNLOCK(&tr_entry->txq_lock[qidx]);
	RTMP_IRQ_UNLOCK(&pAd->tx_swq_lock[qidx], IrqFlags);

	if (info->deq_pkt_cnt <= 0)
		ret = NDIS_STATUS_FAILURE;

	return ret;
}

static INT fq_reset_list_entry(RTMP_ADAPTER *pAd, UCHAR qidx, UCHAR wcid)
{
	int i;
	struct fq_stainfo_type *pfq_sta = NULL;
	STA_TR_ENTRY *tr_entry = NULL;

	if (wcid > MAX_LEN_OF_TR_TABLE)
		return NDIS_STATUS_FAILURE;
	if (qidx > WMM_NUM_OF_AC)
		return NDIS_STATUS_FAILURE;

	tr_entry = &pAd->MacTab.tr_entry[wcid];
	pfq_sta = &tr_entry->fq_sta_rec;

	if (qidx < WMM_NUM_OF_AC) {
		pfq_sta->thMax[qidx] = DEFAULT_THMAX;
		pfq_sta->Entry[qidx].Next = NULL;
		pfq_sta->macInQLen[qidx] = 0;
		pfq_sta->macOutQLen[qidx] = 0;
		pfq_sta->kickPktCnt[qidx] = 0;
		pfq_sta->status[qidx] = FQ_EMPTY_STA;
		pfq_sta->wcid = wcid;
	} else {
		for (i = 0; i < WMM_NUM_OF_AC; i++) {
			pfq_sta->thMax[i] = DEFAULT_THMAX;
			pfq_sta->Entry[i].Next = NULL;
			pfq_sta->macInQLen[i] = 0;
			pfq_sta->macOutQLen[i] = 0;
			pfq_sta->kickPktCnt[i] = 0;
			pfq_sta->status[i] = FQ_EMPTY_STA;
		}
		pfq_sta->wcid = wcid;
		pfq_sta->mpduTime =  RED_MPDU_TIME_INIT;
		pfq_sta->KMAX = 0;
	}

	return NDIS_STATUS_SUCCESS;
}

INT fq_clean_list(RTMP_ADAPTER *pAd, UCHAR qidx)
{
	UCHAR qidx_c, qidx_s, qidx_e;
	struct fq_stainfo_type *pfq_sta = NULL;
	QUEUE_ENTRY *pEntry = NULL, *pPrevEntry = NULL, *pTmpEntry = NULL, *pPrevTmpEntry = NULL;
	STA_TR_ENTRY *tr_entry = NULL;
	UINT8	sta_num = 0, wcid;

	if (pAd->fq_ctrl.enable & FQ_ARRAY_SCH)
		return NDIS_STATUS_SUCCESS;

	if (qidx > WMM_NUM_OF_AC)
		return NDIS_STATUS_FAILURE;

	if (qidx == WMM_NUM_OF_AC) {
		qidx_s = 0;
		qidx_e = WMM_NUM_OF_AC-1;
	} else {
		qidx_s = qidx;
		qidx_e = qidx;
	}

	for (qidx_c = qidx_s; qidx_c <= qidx_e; qidx_c++) {
		RTMP_SEM_LOCK(&pAd->tx_swq_lock[qidx_c]);
		pEntry = pAd->fq_ctrl.fq[qidx_c].Head;
		sta_num = pAd->fq_ctrl.fq[qidx_c].Number;
		pPrevEntry = pEntry;
		while (pEntry) {
			pfq_sta = container_of(pEntry, struct fq_stainfo_type, Entry[qidx_c]);
			tr_entry = &pAd->MacTab.tr_entry[pfq_sta->wcid];
			RTMP_SPIN_LOCK(&pfq_sta->lock[qidx_c]);
			if ((pfq_sta->status[qidx_c] == FQ_UN_CLEAN_STA) && (tr_entry->tx_queue[qidx_c].Number == 0)) {
				if (pEntry == pAd->fq_ctrl.fq[qidx_c].Head) {
					pTmpEntry = RemoveHeadQueue(&pAd->fq_ctrl.fq[qidx_c]);
					pPrevTmpEntry = pAd->fq_ctrl.fq[qidx_c].Head;
					if (pPrevTmpEntry)
						pTmpEntry = pPrevTmpEntry;
				} else {
				PickFromQueue(&pAd->fq_ctrl.fq[qidx_c], pPrevEntry, pEntry);
					pTmpEntry = pPrevEntry->Next;
					pPrevTmpEntry = pPrevEntry;
				}
				pAd->fq_ctrl.list_map[qidx_c][pfq_sta->wcid>>FQ_BITMAP_SHIFT]
						&= ~(1<<(pfq_sta->wcid & FQ_BITMAP_MASK));
				pAd->fq_ctrl.no_packet_chk_map[qidx_c][pfq_sta->wcid>>FQ_BITMAP_SHIFT]
						&= ~(1<<(pfq_sta->wcid & FQ_BITMAP_MASK));
				if (fq_reset_list_entry(pAd, qidx_c, pfq_sta->wcid)) {
					RTMP_SPIN_UNLOCK(&pfq_sta->lock[qidx_c]);
					break;
				}
			} else {
				if (pfq_sta->status[qidx_c] == FQ_EMPTY_STA)
					MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("-->%s(%d): STA%d[%d] Empty but in list\n",
					  __func__, __LINE__, pfq_sta->wcid, qidx_c));
				pfq_sta->status[qidx_c] = FQ_IN_LIST_STA;
				if (tr_entry->tx_queue[qidx_c].Number > 0)
					pAd->fq_ctrl.no_packet_chk_map[qidx_c][pfq_sta->wcid>>FQ_BITMAP_SHIFT] &=
						~(1<<(pfq_sta->wcid & FQ_BITMAP_MASK));

				pPrevTmpEntry = pEntry;
				pTmpEntry = pEntry->Next;
			}
			RTMP_SPIN_UNLOCK(&pfq_sta->lock[qidx_c]);
			pPrevEntry = pPrevTmpEntry;
			pEntry = pTmpEntry;
			if (pAd->fq_ctrl.fq[qidx_c].Number == 0)
				break;
			sta_num--;
			if (sta_num <= 0)
				break;
		}
		RTMP_SEM_UNLOCK(&pAd->tx_swq_lock[qidx_c]);
	}

	if (qidx == WMM_NUM_OF_AC) {
		for (qidx_c = qidx_s; qidx_c <= qidx_e; qidx_c++) {
			RTMP_SEM_LOCK(&pAd->tx_swq_lock[qidx_c]);
			for (wcid = 0; wcid < MAX_LEN_OF_MAC_TABLE; wcid++) {
				if (pAd->fq_ctrl.list_map[qidx_c][wcid>>FQ_BITMAP_SHIFT] &
					(1<<(wcid & FQ_BITMAP_MASK))) {
					tr_entry = &pAd->MacTab.tr_entry[wcid];
					pfq_sta = &tr_entry->fq_sta_rec;
					RTMP_SPIN_LOCK(&pfq_sta->lock[qidx_c]);
					if ((pfq_sta->status[qidx_c] == FQ_UN_CLEAN_STA) &&
						(tr_entry->tx_queue[qidx_c].Number == 0)) {
						pAd->fq_ctrl.list_map[qidx_c][wcid>>FQ_BITMAP_SHIFT]
							&= ~(1<<(wcid & FQ_BITMAP_MASK));
						pAd->fq_ctrl.no_packet_chk_map[qidx_c][wcid>>FQ_BITMAP_SHIFT]
							&= ~(1<<(wcid & FQ_BITMAP_MASK));
						fq_reset_list_entry(pAd, qidx_c, wcid);
					} else {
						if ((pfq_sta->status[qidx_c] != FQ_IN_LIST_STA) &&
							(tr_entry->tx_queue[qidx_c].Number > 0))
							MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
								("-->%s(%d): STA%d[%d]  txqnum:%d ,st:%d\n",
								__func__, __LINE__, pfq_sta->wcid, qidx_c,
								tr_entry->tx_queue[qidx_c].Number,
								pfq_sta->status[qidx_c]));


					}
					RTMP_SPIN_UNLOCK(&pfq_sta->lock[qidx_c]);
				}
			}
			RTMP_SEM_UNLOCK(&pAd->tx_swq_lock[qidx_c]);
		}
	}

	return NDIS_STATUS_SUCCESS;
}

INT fq_update_thMax(RTMP_ADAPTER *pAd, STA_TR_ENTRY *tr_entry, UINT8 wcid,
			INT32 mpduTime, UINT32 dwrr_quantum, UINT32 *Value)
{
	UINT32  max_thMax = 0, max_amptu_len = 0, max_ampdu_num = 0;
	UINT32 txop, txop_usec, thMax, dwrr_idx, dwrr_time;
	INT i, not_active[WMM_NUM_OF_AC];
	struct fq_stainfo_type *pfq_sta = NULL;
	PMAC_TABLE_ENTRY pEntry = NULL;

	if (pAd->fq_ctrl.enable & FQ_READY) {
		pfq_sta = &tr_entry->fq_sta_rec;
		if (mpduTime > 0)
			pfq_sta->mpduTime = mpduTime;
		else {
			if (tr_entry->enqCount <= 0)
				pfq_sta->mpduTime = MAX_FQ_PPDU_TIME;
		}
		pfq_sta->tx_msdu_cnt = 0;

		for (i = 0; i < WMM_NUM_OF_AC; i++) {
			not_active[i] = 0;
			RTMP_SEM_LOCK(&pfq_sta->lock[i]);
			if ((pfq_sta->macInQLen[i]-pfq_sta->macOutQLen[i]) < 0) {
				pfq_sta->macInQLen[i] = 0;
				pfq_sta->macOutQLen[i] = 0;
			}
			if ((tr_entry->tx_queue[i].Number == 0) &&
				((pfq_sta->macInQLen[i]-pfq_sta->macOutQLen[i] <= 0) || (mpduTime < 0))) {
				not_active[i] = 1;
				if (pAd->fq_ctrl.enable & FQ_NO_PKT_STA_KEEP_IN_LIST) {
					if ((pAd->fq_ctrl.no_packet_chk_map[i][wcid>>FQ_BITMAP_SHIFT] &
						(1<<(wcid & FQ_BITMAP_MASK))) &&
						(pAd->fq_ctrl.list_map[i][wcid>>FQ_BITMAP_SHIFT] &
						(1<<(wcid & FQ_BITMAP_MASK)))) {
						pfq_sta->status[i] = FQ_UN_CLEAN_STA;
						pAd->fq_ctrl.no_packet_chk_map[i][wcid>>FQ_BITMAP_SHIFT] &=
							~(1<<(wcid & FQ_BITMAP_MASK));
					} else if ((pAd->fq_ctrl.list_map[i][wcid>>FQ_BITMAP_SHIFT] &
						(1<<(wcid & FQ_BITMAP_MASK))))
						pAd->fq_ctrl.no_packet_chk_map[i][wcid>>FQ_BITMAP_SHIFT] |=
							(1<<(wcid & FQ_BITMAP_MASK));
				}
				RTMP_SEM_UNLOCK(&pfq_sta->lock[i]);
				continue;
			} else
				pAd->fq_ctrl.no_packet_chk_map[i][wcid>>FQ_BITMAP_SHIFT] &= ~(1<<(wcid & FQ_BITMAP_MASK));

			if (i&0x1)
				txop = GET_AC1LIMIT(Value[i]);
			else
				txop = GET_AC0LIMIT(Value[i]);

			txop_usec = txop<<5;
			if ((pAd->vow_watf_en) || (txop_usec == 0)) {
				dwrr_idx = pAd->vow_sta_cfg[wcid].dwrr_quantum[i];
				dwrr_time = ((dwrr_quantum >> (dwrr_idx * UMAX_AIRTIME_QUANTUM_OFFSET))
						& UMAX_AIRTIME_QUANTUM_MASK) << 8;
				thMax = (dwrr_time > 0) ? (dwrr_time/pfq_sta->mpduTime) :
					(MAX_FQ_PPDU_TIME/pfq_sta->mpduTime);
			} else
				thMax = txop_usec / pfq_sta->mpduTime;

			/* TODO: should use STA's link to compute */
			pEntry = &pAd->MacTab.Content[wcid];
			if (pEntry->HTPhyMode.field.MODE == MODE_VHT) {
				max_ampdu_num = MAX_FQ_VHT_AMPDU_NUM;
				max_amptu_len = MAX_FQ_VHT_AMPDU_LEN;
			} else if ((pEntry->HTPhyMode.field.MODE == MODE_HTMIX) ||
					(pEntry->HTPhyMode.field.MODE == MODE_HTGREENFIELD)) {
				max_ampdu_num = MAX_FQ_HT_AMPDU_NUM;
				max_amptu_len = MAX_FQ_HT_AMPDU_LEN;
			} else {
				max_ampdu_num = MIN_HT_THMAX;
				max_amptu_len = MIN_HT_THMAX*1024;
			}

			if (pfq_sta->macQPktLen[i] > 0)
				max_thMax = max_amptu_len/pfq_sta->macQPktLen[i];
			else
				max_thMax = pfq_sta->thMax[i];
			max_thMax = (max_thMax > max_ampdu_num) ? max_ampdu_num : max_thMax;
			if ((mpduTime < 0) && (tr_entry->tx_queue[i].Number > 0))
				thMax = pfq_sta->thMax[i];
			if (thMax > max_thMax)
				pfq_sta->thMax[i] = max_thMax;
			else if (thMax < MIN_HT_THMAX)
				pfq_sta->thMax[i] = MIN_HT_THMAX;
			else
				pfq_sta->thMax[i] = thMax;

			RTMP_SEM_UNLOCK(&pfq_sta->lock[i]);
		}

		if ((not_active[0] == 0) || (not_active[1] == 0) || (not_active[2] == 0) || (not_active[3] == 0))
			return NDIS_STATUS_SUCCESS;
		else {
			return NDIS_STATUS_FAILURE;
		}
	}
	return NDIS_STATUS_SUCCESS;
}

static INT fq_schedule_tx_que(RTMP_ADAPTER *pAd)
{
	struct tm_ops *tm_ops = pAd->tm_qm_ops;
	UINT i, j;
	UCHAR need_schedule = 0;

	for (i = 0; i < WMM_NUM_OF_AC; i++)
		if (pAd->fq_ctrl.frm_cnt[i] != 0) {
			need_schedule = 1;
			break;
		}
	if (need_schedule == 0) {
		for (i = 0; i < WMM_NUM_OF_AC; i++)
			for (j = 0; j < FQ_BITMAP_DWORD; j++)
				if (pAd->fq_ctrl.list_map[i][j] != 0) {
					need_schedule = 1;
					break;
			}
	}
	if ((need_schedule != 0) || (pAd->mgmt_que.Number > 0) ||
		(pAd->high_prio_que.Number > 0)) {
		tm_ops->schedule_task(pAd, TX_DEQ_TASK);
	}

	return NDIS_STATUS_SUCCESS;
}

BOOLEAN fq_queue_limit_check(RTMP_ADAPTER *pAd, NDIS_PACKET *pkt, UCHAR qidx, STA_TR_ENTRY *tr_entry)
{
	int i;
	UCHAR wcid;
	int qmax = 0;
	int qidx_tmp = 0;
	STA_TR_ENTRY *tr_entry_tmp;
	QUEUE_ENTRY *pEntry;
	RTMP_ARCH_OP *arch_ops = &pAd->archOps;

	if ((pAd->fq_ctrl.frm_cnt[qidx] >= (FQ_PER_AC_LIMIT)) && !(pAd->fq_ctrl.enable & FQ_LONGEST_DROP)) {
		pAd->fq_ctrl.drop_cnt[qidx]++;
		pAd->tr_ctl.tx_sw_q_drop++;
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_DYNAMIC_BE_TXOP_ACTIVE) &&
			!tx_flow_check_state(pAd, NO_ENOUGH_SWQ_SPACE, qidx))
			tx_flow_set_state_block(pAd, NULL, NO_ENOUGH_SWQ_SPACE, TRUE, qidx);
		return FALSE;
	}

	InsertTailQueueAc(pAd, tr_entry, &tr_entry->tx_queue[qidx], PACKET_TO_QUEUE_ENTRY(pkt));
	TR_ENQ_COUNT_INC(tr_entry);
	pAd->fq_ctrl.frm_cnt[qidx]++;

	if (!(pAd->fq_ctrl.enable & FQ_LONGEST_DROP))
		return TRUE;

	if (pAd->fq_ctrl.frm_cnt[qidx] >= (FQ_PER_AC_LIMIT)) {
		for (i = 0; i < WMM_NUM_OF_AC ; i++)
			if (pAd->fq_ctrl.frm_cnt[i] > qmax) {
				qidx_tmp = i;
				qmax = pAd->fq_ctrl.frm_cnt[i];
			}

		if (qmax == 0)
			qidx_tmp = qidx;
		qmax = 0;
		wcid = 0;
		for (i = 0; i < MAX_LEN_OF_MAC_TABLE; i++) {
			if ((pAd->fq_ctrl.list_map[qidx_tmp][i>>FQ_BITMAP_SHIFT] &
					(1 << (i & FQ_BITMAP_MASK)))) {
				tr_entry_tmp = &pAd->MacTab.tr_entry[i];
				if (tr_entry_tmp->tx_queue[qidx_tmp].Number >= qmax) {
					qmax = tr_entry_tmp->tx_queue[qidx_tmp].Number;
					wcid = tr_entry_tmp->wcid;
			}
		}
		}
		if (pAd->fq_ctrl.frm_cnt[qidx_tmp] <= 0) {
			qidx_tmp = qidx;
			wcid = tr_entry->wcid;
		}

		tr_entry_tmp = &pAd->MacTab.tr_entry[wcid];
		if (qidx_tmp != qidx)
			RTMP_SPIN_LOCK(&pAd->tx_swq_lock[qidx_tmp]);
		if (wcid != tr_entry->wcid)
			RTMP_SPIN_LOCK(&tr_entry_tmp->txq_lock[qidx_tmp]);
		pEntry = RemoveHeadQueue(&tr_entry_tmp->tx_queue[qidx_tmp]);
		pAd->fq_ctrl.drop_cnt[qidx_tmp]++;
		pAd->fq_ctrl.frm_cnt[qidx_tmp]--;
		TR_ENQ_COUNT_DEC(tr_entry_tmp);
		if ((arch_ops->archRedEnqueueFail) && (!(pAd->fq_ctrl.enable & FQ_SKIP_RED)))
			arch_ops->archRedEnqueueFail(wcid, qidx_tmp, pAd);
		if (wcid != tr_entry->wcid)
			RTMP_SPIN_UNLOCK(&tr_entry_tmp->txq_lock[qidx_tmp]);
		RELEASE_NDIS_PACKET(pAd, QUEUE_ENTRY_TO_PACKET(pEntry), NDIS_STATUS_FAILURE);
		/* Stop device first to avoid drop lots of packets, not execute on WMM case */

		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_DYNAMIC_BE_TXOP_ACTIVE) &&
			!tx_flow_check_state(pAd, NO_ENOUGH_SWQ_SPACE, qidx_tmp))
			tx_flow_set_state_block(pAd, NULL, NO_ENOUGH_SWQ_SPACE, TRUE, qidx_tmp);

		if (qidx_tmp != qidx)
			RTMP_SPIN_UNLOCK(&pAd->tx_swq_lock[qidx_tmp]);
	}

	return TRUE;
}

INT fq_enq_req(RTMP_ADAPTER *pAd, NDIS_PACKET *pkt, UCHAR qidx,
		STA_TR_ENTRY *tr_entry, QUEUE_HEADER *pPktQueue)
{
	BOOLEAN enq_done = FALSE;
	INT enq_idx = 0;
	struct tx_swq_fifo *fifo_swq;
	UCHAR occupied_wcid = 0;
	QUEUE_ENTRY *pEntry;
	struct fq_stainfo_type *pfq_sta = NULL;
	UINT capCount = 0;
	NDIS_PACKET *tmpPkt;
	ULONG IrqFlags = 0;

	ASSERT(qidx < WMM_QUE_NUM);
	ASSERT((tr_entry->wcid != 0));
	fifo_swq = &pAd->tx_swq[qidx];

	RTMP_IRQ_LOCK(&pAd->tx_swq_lock[qidx], IrqFlags);
	if ((tr_entry->enqCount > SQ_ENQ_NORMAL_MAX)
		&& (tr_entry->tx_queue[qidx].Number > SQ_ENQ_RESERVE_PERAC)) {
		enq_done = FALSE;
		pAd->fq_ctrl.drop_cnt[qidx]++;
		pAd->tr_ctl.tx_sw_q_drop++;
		goto enq_end;
	}

	enq_idx = fifo_swq->enqIdx;

	if (tr_entry->enq_cap) {
		RTMP_SPIN_LOCK(&tr_entry->txq_lock[qidx]);
		RTMP_SET_PACKET_QUEIDX(pkt, qidx);

		pAd->fq_ctrl.no_packet_chk_map[qidx][tr_entry->wcid>>FQ_BITMAP_SHIFT] &=
						~(1<<(tr_entry->wcid & FQ_BITMAP_MASK));

		if (!(pAd->fq_ctrl.enable & FQ_ARRAY_SCH)) {
		if ((tr_entry->tx_queue[qidx].Number == 0) || (pAd->fq_ctrl.fq[qidx].Number == 0) ||
			(pAd->fq_ctrl.fq[qidx].Head == NULL))
			fq_add_list(pAd, qidx, tr_entry);
		} else
			pAd->fq_ctrl.list_map[qidx][tr_entry->wcid>>FQ_BITMAP_SHIFT] |=
				(1<<(tr_entry->wcid & FQ_BITMAP_MASK));
		pfq_sta = &tr_entry->fq_sta_rec;
		pfq_sta->macQPktLen[qidx] = (pfq_sta->macQPktLen[qidx] <= 0) ? GET_OS_PKT_LEN(pkt)
				: ((((pfq_sta->macQPktLen[qidx]<<6) - pfq_sta->macQPktLen[qidx])>>6)
				+ ((GET_OS_PKT_LEN(pkt))>>6));

		enq_done = fq_queue_limit_check(pAd, pkt, qidx, tr_entry);

		RTMP_SPIN_UNLOCK(&tr_entry->txq_lock[qidx]);
#ifdef MT_SDIO_ADAPTIVE_TC_RESOURCE_CTRL
#if TC_PAGE_BASED_DEMAND
		if (enq_done)
		tr_entry->TotalPageCount[qidx] += (INT16)(MTSDIOTxGetPageCount(GET_OS_PKT_LEN(pkt), FALSE));
#endif /* TC_PAGE_BASED_DEMAND */
#if DEBUG_ADAPTIVE_QUOTA
		MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s: wcid %d q %d, pkt len %d, TotalPageCount %d\n",
				__func__, tr_entry->wcid, qidx, GET_OS_PKT_LEN(pkt),
				tr_entry->TotalPageCount[qidx]));
#endif /* DEBUG_ADAPTIVE_QUOTA */
#endif /* MT_SDIO_ADAPTIVE_TC_RESOURCE_CTRL */
	} else {
		enq_done = FALSE;
		goto enq_end;
	}

	capCount = SQ_ENQ_RESERVE_PERAC - tr_entry->tx_queue[qidx].Number;

	if (pPktQueue && pPktQueue->Number <= capCount) {
		while (pPktQueue->Head) {
			pEntry = RemoveHeadQueue(pPktQueue);
			tmpPkt =  QUEUE_ENTRY_TO_PACKET(pEntry);
			enq_idx = fifo_swq->enqIdx;
			if ((pAd->fq_ctrl.enable & FQ_READY) && (tr_entry->enq_cap)) {
				if (tr_entry->enqCount >= SQ_ENQ_NORMAL_MAX) {
					occupied_wcid = fifo_swq->swq[enq_idx];
					enq_done = FALSE;
					goto enq_end;
				}
				RTMP_SPIN_LOCK(&tr_entry->txq_lock[qidx]);
				pAd->fq_ctrl.no_packet_chk_map[qidx][tr_entry->wcid>>FQ_BITMAP_SHIFT]
							&= ~(1<<(tr_entry->wcid & FQ_BITMAP_MASK));
				if ((tr_entry->tx_queue[qidx].Number == 0) || (pAd->fq_ctrl.fq[qidx].Number == 0) ||
					(pAd->fq_ctrl.fq[qidx].Head == NULL))
					fq_add_list(pAd, qidx, tr_entry);

				pfq_sta = &tr_entry->fq_sta_rec;
				pfq_sta->macQPktLen[qidx] = (pfq_sta->macQPktLen[qidx] <= 0) ? GET_OS_PKT_LEN(tmpPkt)
						: ((((pfq_sta->macQPktLen[qidx]<<6) - pfq_sta->macQPktLen[qidx])>>6)
						+ ((GET_OS_PKT_LEN(tmpPkt))>>6));

				enq_done = fq_queue_limit_check(pAd, tmpPkt, qidx, tr_entry);

				RTMP_SPIN_UNLOCK(&tr_entry->txq_lock[qidx]);
#ifdef MT_SDIO_ADAPTIVE_TC_RESOURCE_CTRL
#if TC_PAGE_BASED_DEMAND
				if (enq_done)
				tr_entry->TotalPageCount[qidx] +=
					(INT16)(MTSDIOTxGetPageCount(GET_OS_PKT_LEN(tmpPkt), FALSE));
#endif /* TC_PAGE_BASED_DEMAND */
#if DEBUG_ADAPTIVE_QUOTA
				MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("%s: wcid %d q %d, pkt len %d TotalPageCount %d\n",
					__func__, tr_entry->wcid, qidx, GET_OS_PKT_LEN(tmpPkt),
					tr_entry->TotalPageCount[qidx]));
#endif /* DEBUG_ADAPTIVE_QUOTA */
#endif /* MT_SDIO_ADAPTIVE_TC_RESOURCE_CTRL */
			} else {
				enq_done = FALSE;
				goto enq_end;
			}
		}
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
	}

#ifdef MT_SDIO_ADAPTIVE_TC_RESOURCE_CTRL
	else {
		/* Soumik: may need to move the to _RTMPDeQueuePacket()
		 * for AP mode to help account for the demand due to
		 * delivery of PS buffered frame.
		 */
		MTAdaptiveResourceCheckFastAdjustment(pAd, tr_entry->wcid, qidx);
		MTAdaptiveResourceAllocation(pAd, WCID_ALL, GET_NUM_OF_TX_RING(pAd->chipCap));
	}

#endif
	return enq_done;
}

/* UserPriority To AccessCategory mapping */
void fq_tx_free_per_packet(RTMP_ADAPTER *pAd, UINT8 ucAC, UINT8 ucWlanIdx, NDIS_PACKET *pkt)
{
	struct fq_stainfo_type *pfq_sta = NULL;
	STA_TR_ENTRY *tr_entry = NULL;

	tr_entry = &pAd->MacTab.tr_entry[ucWlanIdx];
	pfq_sta = &tr_entry->fq_sta_rec;
	pfq_sta->macOutQLen[ucAC]++;
	pfq_sta->tx_msdu_cnt++;
	pAd->fq_ctrl.msdu_out_hw++;

	return;
}

INT set_fq_enable(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
	UINT32 en, rv, factor;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	struct qm_ops **qm_ops = &pAd->qm_ops;
	int prev_qm = 0;

	if (arg) {
		rv = sscanf(arg, "%d-%d", &en, &factor);
		if ((rv > 0) && (en <= FQ_EN_MASK)) {
			if (((pAd->fq_ctrl.enable & FQ_READY) == 0) && (en & FQ_EN)) {
				tm_exit(pAd);
				qm_exit(pAd);
				pAd->fq_ctrl.enable = en;
				prev_qm = cap->qm;

				if (cap->qm == FAST_PATH_QM) {
					cap->qm = FAST_PATH_FAIR_QM;
					*qm_ops = &fp_fair_qm_ops;
					os_move_mem(*qm_ops, &fp_qm_ops, sizeof(struct qm_ops));
				} else {
					cap->qm = GENERIC_FAIR_QM;
					*qm_ops = &ge_fair_qm_ops;
					os_move_mem(*qm_ops, &ge_qm_ops, sizeof(struct qm_ops));
				}
				qm_init(pAd);
				tm_init(pAd);
				pAd->fq_ctrl.prev_qm = prev_qm;
			} else if ((pAd->fq_ctrl.enable & FQ_READY) && ((en & FQ_EN) == 0)) {
				prev_qm = pAd->fq_ctrl.prev_qm;
				tm_exit(pAd);
				qm_exit(pAd);
				if (pAd->fq_ctrl.prev_qm == FAST_PATH_QM) {
					cap->qm = FAST_PATH_QM;
					*qm_ops = &fp_qm_ops;
				} else {
					cap->qm = GENERIC_QM;
					*qm_ops = &ge_qm_ops;
				}
				pAd->fq_ctrl.enable = en;
				qm_init(pAd);
				tm_init(pAd);
				pAd->fq_ctrl.prev_qm = prev_qm;
			} else
				pAd->fq_ctrl.enable = en | ((pAd->fq_ctrl.enable & FQ_READY) ? FQ_READY : 0);

			if ((factor <= 4) && (factor >= 0))
				pAd->fq_ctrl.factor = factor;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s: set Host FQ Enable to %x (%d).(factor to %d)\n", __func__,
				pAd->fq_ctrl.enable, en, pAd->fq_ctrl.factor));
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT show_fq_info(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT i, j;
	STA_TR_ENTRY *tr_entry = NULL;
	struct fq_stainfo_type *pfq_sta = NULL;
	QUEUE_ENTRY *pEntry = NULL;
	char buf[MAX_LEN_OF_MAC_TABLE*2];
	INT32 pos = 0;
	UINT8 qidx = 0, idx = 0, nqid = 0;
	ULONG IrqFlags = 0;
	INT s_size = MAX_LEN_OF_MAC_TABLE*2;

	for (i = 0; i < MAX_LEN_OF_MAC_TABLE; i++) {
		pos = 0;
		os_zero_mem(buf, s_size);
		pos += snprintf(buf + pos, s_size-pos, "wcid%d:", i);
		tr_entry = &pAd->MacTab.tr_entry[i];
		pfq_sta = &tr_entry->fq_sta_rec;
		nqid = 0;
		for (qidx = 0; qidx < WMM_NUM_OF_AC; qidx++) {
			if ((pfq_sta->macInQLen[qidx] == pfq_sta->macOutQLen[qidx]) &&
				(pfq_sta->macInQLen[qidx] == 0))
				continue;
			nqid++;
			pos += snprintf(buf + pos, s_size-pos, "[AC%d :macQL:%d;qmax:%d,dp:%d,txqn:%d,st:%d]",
					qidx, (INT16)(pfq_sta->macInQLen[qidx]-pfq_sta->macOutQLen[qidx]), pfq_sta->qlen_max_cnt[qidx],
					pfq_sta->drop_cnt[qidx], tr_entry->tx_queue[qidx].Number,
					pfq_sta->status[qidx]
					);
			pfq_sta->qlen_max_cnt[qidx] = 0;
			pfq_sta->drop_cnt[qidx] = 0;
		}
		if (nqid > 0) {
			snprintf(buf + pos, s_size-pos, "KMAX=%d\n", pfq_sta->KMAX);
			pfq_sta->KMAX = 0;
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s", buf));
		}
	}

	for (i = 0; i < WMM_NUM_OF_AC; i++) {
		pAd->fq_ctrl.prev_wcid[i] = -1;
		pAd->fq_ctrl.prev_kick_cnt[i] = 0;
	}

	pAd->fq_ctrl.prev_qidx = -1;

	if ((pAd->fq_ctrl.enable & FQ_EN) == 0) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("FQ was is Disabled (qm=%d)\n", cap->qm));
		return TRUE;
	}
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("FQ was Enabled [0x%x] qm:%d (nSTA:%d,%d[RED]) bcmc:%d ps:%d \n",
			pAd->fq_ctrl.enable, cap->qm, pAd->fq_ctrl.nactive,
			pAd->red_in_use_sta, pAd->fq_ctrl.nbcmc_active, pAd->fq_ctrl.npow_save));

	for (i = 0; i < WMM_NUM_OF_AC; i++) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("FQAC%d's frm_cnt=%d.\n", i, pAd->fq_ctrl.frm_cnt[i]));
	}
	for (i = 0; i < WMM_NUM_OF_AC; i++) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("FQAC%d's frm_max_cnt=%d.\n", i, pAd->fq_ctrl.frm_max_cnt[i]));
		pAd->fq_ctrl.frm_max_cnt[i] = 0;
	}
	for (i = 0; i < WMM_NUM_OF_AC; i++) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("FQAC%d's drop_cnt=%d.\n", i, pAd->fq_ctrl.drop_cnt[i]));
		pAd->fq_ctrl.drop_cnt[i] = 0;
	}

	for (i = 0; i < WMM_NUM_OF_AC; i++) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("FQAC%d's num=%d.\n", i, pAd->fq_ctrl.fq[i].Number));
	}

	for (i = 0; i < WMM_NUM_OF_AC; i++) {
		for (j = 0; j < FQ_BITMAP_DWORD; j++)
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("no_packet_chkmap[%d][%d]=0x%08X.\n", i, j,
				pAd->fq_ctrl.no_packet_chk_map[i][j]));
	}

	for (j = 0; j < FQ_BITMAP_DWORD; j++)
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("staInUseBitmap[%d]=0x%08X.\n", j, pAd->fq_ctrl.staInUseBitmap[j]));

	for (i = 0; i < WMM_NUM_OF_AC; i++) {
		for (j = 0; j < FQ_BITMAP_DWORD; j++)
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("list_map[%d][%d]=0x%08X.\n", i, j, pAd->fq_ctrl.list_map[i][j]));
	}


	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("sta_in_head:\n"));
	for (i = 0; i < WMM_NUM_OF_AC; i++) {
		RTMP_IRQ_LOCK(&pAd->tx_swq_lock[i], IrqFlags);
		for (j = 0; j < MAX_LEN_OF_MAC_TABLE; j++)
			if (pAd->fq_ctrl.sta_in_head[i][j] > 0) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("[%d][%d]=%d\n", i, j, pAd->fq_ctrl.sta_in_head[i][j]));
				pAd->fq_ctrl.sta_in_head[i][j] = 0;
			}
		RTMP_IRQ_UNLOCK(&pAd->tx_swq_lock[i], IrqFlags);
	}


	for (i = 0; i < WMM_NUM_OF_AC; i++) {
		RTMP_IRQ_LOCK(&pAd->tx_swq_lock[i], IrqFlags);
		os_zero_mem(buf, MAX_LEN_OF_MAC_TABLE*2);
		pEntry = pAd->fq_ctrl.fq[i].Head;
		pos = 0;
		pos = snprintf(buf+pos, s_size-pos, "FQ[%d]={", i);
		idx = 0;
		while (pAd->fq_ctrl.fq[i].Number > 0) {
			PMAC_TABLE_ENTRY pMACEntry;

			if (pEntry == NULL) {
				pos += snprintf(buf+pos, s_size-pos, "Empty");
				break;
			}
			pfq_sta = container_of(pEntry, struct fq_stainfo_type, Entry[i]);
			pMACEntry = &pAd->MacTab.Content[pfq_sta->wcid];
			tr_entry = &pAd->MacTab.tr_entry[pfq_sta->wcid];
			if ((idx % 16) == 15) {
				snprintf(buf+pos, s_size-pos, "%d[%d](%d)(%d)\n", pfq_sta->wcid,
						tr_entry->tx_queue[i].Number,
						pMACEntry->tr_tb_idx, tr_entry->wcid);
						MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
							("%s", buf));
				pos = 0;
				os_zero_mem(buf, s_size);
			} else
				pos += snprintf(buf+pos, s_size-pos, "%d[%d](%d)(%d),", pfq_sta->wcid,
						tr_entry->tx_queue[i].Number,
						pMACEntry->tr_tb_idx, tr_entry->wcid);
			idx++;
			if (pEntry == pEntry->Next) {
				pos += snprintf(buf+pos, s_size-pos, "++");
				break;
			}
			pEntry = pEntry->Next;
			if (pEntry == NULL)
				break;
		}
		if ((idx >= 0) && (pos >= 0))
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s}\n", buf));
		RTMP_IRQ_UNLOCK(&pAd->tx_swq_lock[i], IrqFlags);
	}

	for (i = 0; i < WMM_NUM_OF_AC; i++) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("srchpos[%d]=%d\n", i, pAd->fq_ctrl.srch_pos[i]));
	}

	return TRUE;
}

INT set_fq_debug_enable(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
	UINT32 en, rv;

	if (arg) {
		rv = sscanf(arg, "%d", &en);

		if ((rv > 0) && (en <= FQ_DBG_MASK)) {
			pAd->fq_ctrl.dbg_en = en;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s: set FQ Debug Message Enable to %d.\n", __func__, pAd->fq_ctrl.dbg_en));
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_fq_dbg_listmap(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
	UINT32 en, rv, wcid, qidx;

	if (arg) {
		rv = sscanf(arg, "%d-%d-%d", &qidx, &wcid, &en);

		if ((rv > 0) && (wcid <= MAX_LEN_OF_MAC_TABLE) && (qidx <= WMM_AC_VO)) {
			if (en == 0)
				pAd->fq_ctrl.list_map[qidx][wcid>>FQ_BITMAP_SHIFT]
						&= ~(1<<(wcid & FQ_BITMAP_MASK));
			else
				pAd->fq_ctrl.list_map[qidx][wcid>>FQ_BITMAP_SHIFT]
						|= (1<<(wcid & FQ_BITMAP_MASK));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s: set STA%d[%d] to %d.\n", __func__, wcid, qidx, en));
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

INT set_fq_dbg_linklist(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
	UINT32 en = 0, rv, wcid = 0, qidx = 0, sta_num = 0;
	struct fq_stainfo_type *pfq_sta = NULL;
	QUEUE_ENTRY *pEntry = NULL, *pPrevEntry = NULL, *pTmpEntry = NULL, *pPrevTmpEntry = NULL;
	STA_TR_ENTRY *tr_entry = NULL;

	if (arg) {
		rv = sscanf(arg, "%d-%d-%d", &qidx, &wcid, &en);
		if ((rv > 0) && (wcid < MAX_LEN_OF_MAC_TABLE) && (qidx <= WMM_AC_VO)) {
			if (en == 0) {
				RTMP_SEM_LOCK(&pAd->tx_swq_lock[qidx]);
				pEntry = pAd->fq_ctrl.fq[qidx].Head;
				sta_num = 0;
				pPrevEntry = pEntry;
				while (pEntry) {
					pfq_sta = container_of(pEntry, struct fq_stainfo_type, Entry[qidx]);
					RTMP_SPIN_LOCK(&pfq_sta->lock[qidx]);
					if (wcid == pfq_sta->wcid) {
						tr_entry = &pAd->MacTab.tr_entry[wcid];
						if (pEntry == pAd->fq_ctrl.fq[qidx].Head) {
							pTmpEntry = RemoveHeadQueue(&pAd->fq_ctrl.fq[qidx]);
							pPrevTmpEntry = pAd->fq_ctrl.fq[qidx].Head;
							if (pPrevTmpEntry)
								pTmpEntry = pPrevTmpEntry;
						} else {
							pTmpEntry = pEntry;
							PickFromQueue(&pAd->fq_ctrl.fq[qidx], pPrevEntry, pTmpEntry);
							if (pPrevEntry->Next)
								pTmpEntry = pPrevEntry->Next;
						}
						pAd->fq_ctrl.frm_cnt[qidx] -= tr_entry->tx_queue[qidx].Number;
					} else {
						pPrevTmpEntry = pEntry;
						pTmpEntry = pEntry->Next;
					}
					RTMP_SPIN_UNLOCK(&pfq_sta->lock[qidx]);
					if (pPrevTmpEntry)
						pPrevEntry = pPrevTmpEntry;
					pEntry = pTmpEntry;
					sta_num++;
					if (sta_num >= MAX_LEN_OF_MAC_TABLE)
						break;
				}
				RTMP_SEM_UNLOCK(&pAd->tx_swq_lock[qidx]);
			} else {
				RTMP_SEM_LOCK(&pAd->tx_swq_lock[qidx]);
				tr_entry = &pAd->MacTab.tr_entry[wcid];
				pfq_sta = &tr_entry->fq_sta_rec;
				RTMP_SPIN_LOCK(&pfq_sta->lock[qidx]);
				pfq_sta->kickPktCnt[qidx] = 0;
				pfq_sta->status[qidx] = FQ_IN_LIST_STA;
				pfq_sta->wcid = tr_entry->wcid;
				InsertTailQueue(&pAd->fq_ctrl.fq[qidx], &(pfq_sta->Entry[qidx]));
				pAd->fq_ctrl.frm_cnt[qidx] += tr_entry->tx_queue[qidx].Number;
				RTMP_SPIN_UNLOCK(&pfq_sta->lock[qidx]);
				RTMP_SEM_UNLOCK(&pAd->tx_swq_lock[qidx]);
			}
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s: set STA%d[%d] %s list .\n", __func__, wcid, qidx, (en == 0) ? "from" : "to"));
		} else
			return FALSE;
	} else
		return FALSE;

	return TRUE;
}

static UINT16 fq_del_list_v2(RTMP_ADAPTER *pAd, struct dequeue_info *info, CHAR deq_qid, UINT32 *tx_quota)
{
	UINT16 wcid = 0;
	STA_TR_ENTRY *tr_entry = NULL;
	struct fq_stainfo_type *pfq_sta = NULL;
	INT32 quota = 0, n;

	if (info->q_max_cnt[deq_qid] == 0)
		return 0;

#if FQ_SCH_DBG_SUPPORT
	if (pAd->fq_ctrl.frm_max_cnt[deq_qid] < pAd->fq_ctrl.frm_cnt[deq_qid])
		pAd->fq_ctrl.frm_max_cnt[deq_qid] = pAd->fq_ctrl.frm_cnt[deq_qid];
#endif
	n = 0;
	do {
		UINT16 srch_pos;

		srch_pos = (pAd->fq_ctrl.srch_pos[deq_qid]+1);
		if (srch_pos == MAX_LEN_OF_MAC_TABLE)
			srch_pos = 1;
		wcid = srch_pos;
		if ((pAd->fq_ctrl.list_map[deq_qid][wcid>>FQ_BITMAP_SHIFT] & (1 << (wcid & FQ_BITMAP_MASK))) == 0)
			goto LOOP_END;

		tr_entry = &pAd->MacTab.tr_entry[wcid];
		if (tr_entry->tx_queue[deq_qid].Number == 0)
			goto LOOP_END;

		if (!VALID_UCAST_ENTRY_WCID(pAd, wcid)) {
			*tx_quota = MAX_TX_PROCESS;
			pAd->fq_ctrl.srch_pos[deq_qid] = srch_pos;
			pAd->fq_ctrl.sta_in_head[deq_qid][wcid]++;
			return wcid;
		}

		pfq_sta = &tr_entry->fq_sta_rec;
		pAd->fq_ctrl.srch_pos[deq_qid] = srch_pos;

		if ((!(pAd->fq_ctrl.enable & FQ_SKIP_SINGLE_STA_CASE)) && (pAd->fq_ctrl.nactive == 1)) {
			*tx_quota = (tr_entry->tx_queue[deq_qid].Number <
							MAX_TX_PROCESS) ?
							tr_entry->tx_queue[deq_qid].Number :
							MAX_TX_PROCESS;
		} else {
			if ((INT16)(pAd->fq_ctrl.msdu_in_hw - pAd->fq_ctrl.msdu_out_hw) <
				(pAd->fq_ctrl.msdu_threshold - pAd->fq_ctrl.nactive * MAX_FQ_VHT_AMPDU_NUM)) {
				*tx_quota = (tr_entry->tx_queue[deq_qid].Number <
								MAX_FQ_VHT_AMPDU_NUM) ?
								tr_entry->tx_queue[deq_qid].Number :
								MAX_FQ_VHT_AMPDU_NUM;
			} else {
				quota = (pfq_sta->thMax[deq_qid]<<pAd->fq_ctrl.factor) -
					(INT16)(pfq_sta->macInQLen[deq_qid]-pfq_sta->macOutQLen[deq_qid]);
				if (quota <= 0)
					*tx_quota = MIN_HT_THMAX;
				else if (quota >= MAX_FQ_VHT_AMPDU_NUM)
					*tx_quota = MAX_FQ_VHT_AMPDU_NUM;
				else
					*tx_quota = quota;
			}
		}

		pAd->fq_ctrl.sta_in_head[deq_qid][wcid]++;
		return wcid;

LOOP_END:
		n++;
		pAd->fq_ctrl.srch_pos[deq_qid] = srch_pos;
	} while (n < MAX_LEN_OF_MAC_TABLE);
	*tx_quota = 0;

	return 0;

}

static INT fq_del_report_v2(RTMP_ADAPTER *pAd, struct dequeue_info *info)
{
	ULONG IrqFlags = 0;
	UCHAR qidx = info->cur_q;
	UCHAR wcid = info->cur_wcid;
	struct fq_stainfo_type *pfq_sta = NULL;
	STA_TR_ENTRY *tr_entry = NULL;
	INT prev_wcid_tmp;
	INT ret = NDIS_STATUS_SUCCESS;

	ASSERT(qidx < WMM_QUE_NUM);
	ASSERT(wcid < MAX_LEN_OF_TR_TABLE);

	RTMP_IRQ_LOCK(&pAd->tx_swq_lock[qidx], IrqFlags);

	if (info->q_max_cnt[info->cur_q] > 0)
		info->q_max_cnt[info->cur_q] -= info->deq_pkt_cnt;

#ifdef FQ_SCH_DBG_SUPPORT
	if (pAd->fq_ctrl.dbg_en&FQ_DBG_DUMP_STA_LOG) {
		if (pAd->fq_ctrl.prev_qidx >= 0) {
			UCHAR the_prev_qidx = pAd->fq_ctrl.prev_qidx;

			if (the_prev_qidx == qidx)
				prev_wcid_tmp = pAd->fq_ctrl.prev_wcid[qidx];
			else
				prev_wcid_tmp = pAd->fq_ctrl.prev_wcid[the_prev_qidx];

			if ((prev_wcid_tmp > 0) && (prev_wcid_tmp < MAX_LEN_OF_MAC_TABLE)) {
				if ((pAd->fq_ctrl.prev_wcid[qidx] == wcid) &&
					((pAd->fq_ctrl.fq[qidx].Number >= 1) || (!pAd->fq_ctrl.enable)
					|| (pAd->fq_ctrl.nactive == 1))) {
					tr_entry = &pAd->MacTab.tr_entry[wcid];
					pfq_sta = &tr_entry->fq_sta_rec;
					pAd->fq_ctrl.prev_kick_cnt[qidx] += info->deq_pkt_cnt;
					RTMP_SEM_LOCK(&pfq_sta->lock[qidx]);
					if (pfq_sta->KMAX < pAd->fq_ctrl.prev_kick_cnt[qidx])
						pfq_sta->KMAX = pAd->fq_ctrl.prev_kick_cnt[qidx];
					RTMP_SEM_UNLOCK(&pfq_sta->lock[qidx]);
				} else {
					tr_entry = &pAd->MacTab.tr_entry[pAd->fq_ctrl.prev_wcid[qidx]];
					pfq_sta = &tr_entry->fq_sta_rec;
					RTMP_SEM_LOCK(&pfq_sta->lock[qidx]);
					if (pfq_sta->KMAX < pAd->fq_ctrl.prev_kick_cnt[the_prev_qidx])
						pfq_sta->KMAX = pAd->fq_ctrl.prev_kick_cnt[the_prev_qidx];
					pAd->fq_ctrl.prev_kick_cnt[qidx] = info->deq_pkt_cnt;
					RTMP_SEM_UNLOCK(&pfq_sta->lock[qidx]);
				}
			} else
				pAd->fq_ctrl.prev_kick_cnt[qidx] = info->deq_pkt_cnt;
		} else
			pAd->fq_ctrl.prev_kick_cnt[qidx] = info->deq_pkt_cnt;
		pAd->fq_ctrl.prev_wcid[qidx] = wcid;
		pAd->fq_ctrl.prev_qidx = qidx;
	}
#endif
	tr_entry = &pAd->MacTab.tr_entry[wcid];
	pfq_sta = &tr_entry->fq_sta_rec;
	pfq_sta->KMAX = info->deq_pkt_cnt;

	if (info->deq_pkt_cnt > 0) {
		RTMP_SEM_LOCK(&pfq_sta->lock[qidx]);
		if (info->pkt_cnt >= info->deq_pkt_cnt)
			info->pkt_cnt -= info->deq_pkt_cnt;
		else
			info->pkt_cnt = 0;
		pfq_sta->kickPktCnt[qidx] += info->deq_pkt_cnt;
		pfq_sta->macInQLen[qidx] += info->deq_pkt_cnt;
		pAd->fq_ctrl.msdu_in_hw += info->deq_pkt_cnt;
		RTMP_SEM_UNLOCK(&pfq_sta->lock[qidx]);

		if (pAd->fq_ctrl.frm_cnt[qidx] >= info->deq_pkt_cnt)
			pAd->fq_ctrl.frm_cnt[qidx] -= info->deq_pkt_cnt;
		else {
			if (pAd->fq_ctrl.frm_cnt[qidx] > 0)
				pAd->fq_ctrl.frm_cnt[qidx] = 0;
		}
		RTMP_SPIN_LOCK(&tr_entry->txq_lock[qidx]);
		if (tr_entry->tx_queue[qidx].Number == 0)
			pAd->fq_ctrl.list_map[qidx][tr_entry->wcid>>FQ_BITMAP_SHIFT]
					&= ~(1<<(tr_entry->wcid & FQ_BITMAP_MASK));
		RTMP_SPIN_UNLOCK(&tr_entry->txq_lock[qidx]);

		if (tx_flow_check_state(pAd, NO_ENOUGH_SWQ_SPACE, qidx) &&
			fq_get_swq_free_num(pAd, qidx) > pAd->tx_swq[qidx].high_water_mark)
			tx_flow_set_state_block(pAd, NULL, NO_ENOUGH_SWQ_SPACE, FALSE, qidx);
	}

	RTMP_IRQ_UNLOCK(&pAd->tx_swq_lock[qidx], IrqFlags);

	return ret;
}
VOID app_show_fq_dbgmsg(RTMP_ADAPTER *pAd)
{
	UINT_8 wcid, ac_idx, idx;
	INT pos = 0;
	char buf[MAX_LEN_OF_MAC_TABLE*2];
	ULONG IrqFlags = 0;
	STA_TR_ENTRY *tr_entry = NULL;
	struct fq_stainfo_type *pfq_sta = NULL;
	QUEUE_ENTRY *pEntry = NULL;
	P_RED_STA_T prRedSta;
	INT32 s_size = MAX_LEN_OF_MAC_TABLE*2;

	/* Show Debug Message by cmd */
	if (pAd->fq_ctrl.dbg_en&FQ_DBG_DUMP_FQLIST) {
		for (ac_idx = WMM_AC_BK; ac_idx <= WMM_AC_VO; ac_idx++) {
			if (pAd->fq_ctrl.fq[ac_idx].Number == 0)
				continue;
			RTMP_IRQ_LOCK(&pAd->tx_swq_lock[ac_idx], IrqFlags);
			os_zero_mem(buf, MAX_LEN_OF_MAC_TABLE*2);
			pEntry = pAd->fq_ctrl.fq[ac_idx].Head;
			pos = snprintf(buf, s_size, "FQ[%d]={", ac_idx);
			idx = 0;
			while (pAd->fq_ctrl.fq[ac_idx].Number > 0) {
				if (pEntry == NULL) {
					pos += snprintf(buf+pos, s_size-pos, "Empty");
					break;
				}
				pfq_sta = container_of(pEntry, struct fq_stainfo_type, Entry[ac_idx]);

				if (idx == 0)
					pos += snprintf(buf+pos, s_size-pos, "%d", pfq_sta->wcid);
				else if ((idx % 16) == 0) {
					snprintf(buf+pos, s_size-pos, ",%d\n", pfq_sta->wcid);
					MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
							("%s", buf));
					pos = 0;
					os_zero_mem(buf, MAX_LEN_OF_MAC_TABLE*2);
				} else
					pos += snprintf(buf+pos, s_size-pos, ",%d", pfq_sta->wcid);
				idx++;
				if (pEntry == pEntry->Next) {
					pos += snprintf(buf+pos, s_size-pos, "++");
					break;
				}

				pEntry = pEntry->Next;
				if (pEntry == NULL)
					break;
			}

			if ((idx > 0) && (pos >= 0))
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("%s},frm:%d\n", buf, pAd->fq_ctrl.frm_cnt[ac_idx]));

			RTMP_IRQ_UNLOCK(&pAd->tx_swq_lock[ac_idx], IrqFlags);
		}
	}

	for (wcid = 1; wcid < MAX_LEN_OF_MAC_TABLE; wcid++) {
		tr_entry = &pAd->MacTab.tr_entry[wcid];
		pfq_sta = &tr_entry->fq_sta_rec;
		prRedSta = &pAd->red_sta[wcid];
		for (ac_idx = WMM_AC_BK; ac_idx <= WMM_AC_VO; ac_idx++) {
			if ((pAd->fq_ctrl.dbg_en&FQ_DBG_DUMP_STA_LOG) &&
					(pAd->fq_ctrl.list_map[ac_idx][wcid>>FQ_BITMAP_SHIFT] &
					(1 << (wcid & FQ_BITMAP_MASK)))) {
				if (pAd->fq_ctrl.enable&FQ_READY) {
					os_zero_mem(buf, MAX_LEN_OF_MAC_TABLE*2);
					sprintf(buf, "STA%d[AC%d]th:%d macQLen:%d(%d,%d),PktL:%d,kPkt:%d,"
						"KMax:%d,tqn:%d,mT:%d,S=%d,"
						"[0x%08X] msdu_cnt=%d,avg=%d tr_st=%d",
						wcid, ac_idx, pfq_sta->thMax[ac_idx],
						(INT16)(pfq_sta->macInQLen[ac_idx]-pfq_sta->macOutQLen[ac_idx]),
						pfq_sta->macInQLen[ac_idx], pfq_sta->macOutQLen[ac_idx],
						pfq_sta->macQPktLen[ac_idx], pfq_sta->kickPktCnt[ac_idx],
						pfq_sta->KMAX, tr_entry->tx_queue[ac_idx].Number,
						pfq_sta->mpduTime, pfq_sta->status[ac_idx],
						pAd->fq_ctrl.list_map[ac_idx][wcid>>5],
						pfq_sta->tx_msdu_cnt, prRedSta->tx_msdu_avg_cnt,
						tr_entry->StaRec.ConnectionState);
					MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s\n", buf));
					if (pfq_sta->KMAX > 0)
						pfq_sta->KMAX = 0;
				}
			}
		}
	}
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("frmcnt:[%d, %d, %d, %d]\n", pAd->fq_ctrl.frm_cnt[0],
		pAd->fq_ctrl.frm_cnt[1], pAd->fq_ctrl.frm_cnt[2], pAd->fq_ctrl.frm_cnt[3]));

}

struct qm_ops ge_fair_qm_ops = { 0 };
#else
struct qm_ops ge_fair_qm_ops = {
	.init = NULL,
	.exit = NULL,
};
#endif /* FQ_SCH_SUPPORT */

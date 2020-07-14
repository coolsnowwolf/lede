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

#ifdef FQ_SCH_SUPPORT

extern INT32 fq_deq_data_pkt(RTMP_ADAPTER *pAd, TX_BLK *tx_blk, INT32 max_cnt, struct dequeue_info *info);
extern INT fq_del_report(RTMP_ADAPTER *pAd, struct dequeue_info *info);

extern INT fq_enq_req(RTMP_ADAPTER *pAd, NDIS_PACKET *pkt, UCHAR qidx,
			STA_TR_ENTRY *tr_entry, QUEUE_HEADER *pPktQueue);

extern UCHAR WMM_UP2AC_MAP[8];

static INT fp_fair_deq_req(RTMP_ADAPTER *pAd, INT cnt, struct dequeue_info *info);

INT32 fp_fair_enq_dataq_pkt(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PNDIS_PACKET pkt, UCHAR q_idx)
{
	struct qm_ops *qm_ops = pAd->qm_ops;
	UCHAR wcid, the_q_idx;
	STA_TR_ENTRY *tr_entry = NULL;

	if (wlan_operate_get_state(wdev) != WLAN_OPER_STATE_VALID)
		goto error;

	the_q_idx = WMM_UP2AC_MAP[q_idx];
	wcid = RTMP_GET_PACKET_WCID(pkt);
	tr_entry = &pAd->MacTab.tr_entry[wcid];

	if (!fq_enq_req(pAd, pkt, the_q_idx, tr_entry, NULL))
		goto error;

	qm_ops->schedule_tx_que(pAd);

	return NDIS_STATUS_SUCCESS;
error:
	RELEASE_NDIS_PACKET(pAd, pkt, NDIS_STATUS_FAILURE);
	return NDIS_STATUS_FAILURE;
}

static INT fp_fair_deq_req(RTMP_ADAPTER *pAd, INT cnt, struct dequeue_info *info)
{
	CHAR deq_qid = 0, start_q = 0, end_q = 0;
	UCHAR deq_wcid = 0;
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

		deq_wcid = fq_del_list(pAd, info, deq_qid, &quota);

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
	return TRUE;
}
static NDIS_PACKET *fp_fair_get_data_tx_element(RTMP_ADAPTER *pAd, struct dequeue_info
						*deq_info, STA_TR_ENTRY *tr_entry)
{
	PQUEUE_ENTRY q_entry;
	UCHAR q_idx = deq_info->cur_q;

	q_entry = RemoveHeadQueue(&tr_entry->tx_queue[q_idx]);
	TR_ENQ_COUNT_DEC(tr_entry);
	if (q_entry)
		return QUEUE_ENTRY_TO_PACKET(q_entry);

	return NULL;
}

static NDIS_PACKET *fp_fair_first_data_tx_element(RTMP_ADAPTER *pAd, struct dequeue_info
						*deq_info, STA_TR_ENTRY *tr_entry)
{
	PQUEUE_ENTRY q_entry;
	UCHAR q_idx = deq_info->cur_q;

	q_entry = tr_entry->tx_queue[q_idx].Head;
	if (q_entry)
		return QUEUE_ENTRY_TO_PACKET(q_entry);

	return NULL;
}
static NDIS_PACKET *fp_fair_get_mgmt_tx_element(RTMP_ADAPTER *pAd)
{
	PQUEUE_ENTRY q_entry;

	RTMP_SEM_LOCK(&pAd->mgmt_que_lock);

	q_entry = RemoveHeadQueue(&pAd->mgmt_que);
	RTMP_SEM_UNLOCK(&pAd->mgmt_que_lock);

	if (q_entry)
		return QUEUE_ENTRY_TO_PACKET(q_entry);

	return NULL;
}

static NDIS_PACKET *fp_fair_first_mgmt_tx_element(RTMP_ADAPTER *pAd)
{
	PQUEUE_ENTRY q_entry;

	RTMP_SEM_LOCK(&pAd->mgmt_que_lock);
	q_entry = pAd->mgmt_que.Head;
	RTMP_SEM_UNLOCK(&pAd->mgmt_que_lock);

	if (q_entry)
		return QUEUE_ENTRY_TO_PACKET(q_entry);

	return NULL;
}

VOID fp_fair_tx_pkt_deq_func(RTMP_ADAPTER *pAd)
{
	TX_BLK tx_blk, *pTxBlk;
	NDIS_PACKET *pkt = NULL;
	UCHAR Wcid = 0;
	UCHAR q_idx = 0;
	INT ret = 0;
	UINT32 KickRingBitMap = 0;
	PKT_TOKEN_CB *pktTokenCb = (PKT_TOKEN_CB *)pAd->PktTokenCb;
	UINT32 idx = 0;
	struct wifi_dev *wdev = NULL;
	STA_TR_ENTRY *tr_entry = NULL;
	struct wifi_dev_ops *wdev_ops;
	RTMP_ARCH_OP *arch_ops = &pAd->archOps;
	BOOLEAN need_schedule = (pAd->tx_dequeue_scheduable ? TRUE : FALSE);
	UCHAR user_prio = 0;
	BOOLEAN data_turn = FALSE;
	struct dequeue_info deq_info = {0};
#ifdef CONFIG_TP_DBG
	struct tp_debug *tp_dbg = &pAd->tr_ctl.tp_dbg;
#endif
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	INT32 max_cnt = GET_TX_RING_SIZE(cap);

	deq_info.target_wcid = WCID_ALL;
	deq_info.target_que = WMM_NUM_OF_AC;

	while (need_schedule) {
		if (!pktTokenCb->tx_id_list.token_inited)
			break;
		if (data_turn == FALSE)	{
			pkt = fp_fair_first_mgmt_tx_element(pAd);
			if (!pkt) {
				os_zero_mem(&deq_info, sizeof(struct dequeue_info));
				deq_info.status = NDIS_STATUS_SUCCESS;
				deq_info.target_wcid = WCID_ALL;
				deq_info.target_que = WMM_NUM_OF_AC;
				deq_info.inited = 0;
				fp_fair_deq_req(pAd, max_cnt, &deq_info);
				if (deq_info.status == NDIS_STATUS_FAILURE)
					break;

				data_turn = TRUE;
				q_idx = deq_info.cur_q;
				Wcid = deq_info.cur_wcid;

				deq_info.deq_pkt_cnt = 0;
				tr_entry = &pAd->MacTab.tr_entry[Wcid];
				RTMP_SEM_LOCK(&tr_entry->txq_lock[q_idx]);
				pkt = fp_fair_first_data_tx_element(pAd, &deq_info, tr_entry);
				if (!pkt)
					goto DATA_EXIT;
			}
		} else {
			pkt = fp_fair_first_data_tx_element(pAd, &deq_info, tr_entry);
			if (!pkt)
				goto DATA_EXIT;
		}

		NdisZeroMemory((UCHAR *)&tx_blk, sizeof(TX_BLK));
		pTxBlk = &tx_blk;

		wdev = wdev_search_by_pkt(pAd, pkt);
		if (!wdev)
			continue;

		wdev_ops = wdev->wdev_ops;
		pTxBlk->resource_idx = arch_ops->get_resource_idx(pAd, wdev, 0, 0);
		if (arch_ops->check_hw_resource(pAd, wdev, pTxBlk->resource_idx)) {
			if (data_turn == FALSE)
				break;
			if (deq_info.target_wcid < MAX_LEN_OF_TR_TABLE)
				deq_info.full_qid[q_idx] = TRUE;
			else
				deq_info.q_max_cnt[q_idx] = 0;
			goto DATA_EXIT;
		}

		if (data_turn == FALSE)	{
			pkt = fp_fair_get_mgmt_tx_element(pAd);
			if (!pkt)
				continue;
			Wcid = RTMP_GET_PACKET_WCID(pkt);
		} else {
			pkt = fp_fair_get_data_tx_element(pAd, &deq_info, tr_entry);
			deq_info.deq_pkt_cnt++;
		}

		if (data_turn == FALSE) {
			/*if wcid is out of MAC table size, free it*/
			if (Wcid >= MAX_LEN_OF_MAC_TABLE) {
				MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						 ("%s(): WCID is invalid\n", __func__));
				RELEASE_NDIS_PACKET(pAd, pkt, NDIS_STATUS_FAILURE);
				continue;
			}
			tr_entry = &pAd->MacTab.tr_entry[Wcid];
		}
		if (wdev)
			pTxBlk->wdev = wdev;
		else {
			RELEASE_NDIS_PACKET(pAd, pkt, NDIS_STATUS_FAILURE);
			continue;
		}
		pTxBlk->TotalFrameNum = 1;
		pTxBlk->TotalFragNum = 1;
		pTxBlk->tr_entry = tr_entry;
		user_prio = RTMP_GET_PACKET_UP(pkt);
		pTxBlk->QueIdx = RTMP_GET_PACKET_QUEIDX(pkt);
		pTxBlk->TotalFrameLen = GET_OS_PKT_LEN(pkt);
		pTxBlk->pPacket = pkt;
		pTxBlk->TxFrameType = tx_pkt_classification(pAd, pTxBlk->pPacket, pTxBlk);
		pTxBlk->HeaderBuf = arch_ops->get_hif_buf(pAd, pTxBlk, pTxBlk->resource_idx, pTxBlk->TxFrameType);
		InsertTailQueue(&pTxBlk->TxPacketList, PACKET_TO_QUEUE_ENTRY(pkt));

		ret = wdev_ops->tx_pkt_handle(pAd, wdev, pTxBlk);

		if (!ret)
			KickRingBitMap |= (1 << pTxBlk->resource_idx);

		if (data_turn == TRUE)	{
			if (deq_info.q_max_cnt[q_idx] > 0)
				deq_info.q_max_cnt[q_idx]--;
			if (deq_info.target_wcid < MAX_LEN_OF_TR_TABLE)
				deq_info.pkt_cnt--;
			max_cnt -= deq_info.deq_pkt_cnt;
			if (max_cnt <= 0)
				goto DATA_EXIT;
			if ((deq_info.q_max_cnt[q_idx] <= 0) ||
				(deq_info.pkt_cnt <= deq_info.deq_pkt_cnt)) {
				data_turn = FALSE;
				RTMP_SEM_UNLOCK(&tr_entry->txq_lock[q_idx]);
				fq_del_report(pAd, &deq_info);
			}
		}
	}
DATA_EXIT:
	if (data_turn == TRUE) {
		RTMP_SEM_UNLOCK(&tr_entry->txq_lock[q_idx]);
		fq_del_report(pAd, &deq_info);
	}

	while (KickRingBitMap != 0 && idx < GET_NUM_OF_TX_RING(cap)) {
		if (KickRingBitMap & 0x1) {
			arch_ops->kickout_data_tx(pAd, pTxBlk, idx);
#ifdef CONFIG_TP_DBG
			tp_dbg->IoWriteTx++;
#endif
		}

		KickRingBitMap >>= 1;
		idx++;
	}
}


struct qm_ops fp_fair_qm_ops = {0};
#else
struct qm_ops fp_fair_qm_ops = {
	.init = NULL,
	.exit = NULL,
};
VOID fp_fair_tx_pkt_deq_func(RTMP_ADAPTER *pAd)
{
	return;
}
#endif /* FQ_SCH_SUPPORT */



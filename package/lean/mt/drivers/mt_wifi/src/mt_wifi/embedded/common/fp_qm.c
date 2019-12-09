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

static NDIS_PACKET *fp_get_tx_element(RTMP_ADAPTER *pAd)
{
	PQUEUE_ENTRY q_entry;

	RTMP_SEM_LOCK(&pAd->mgmt_que_lock);
	q_entry = RemoveHeadQueue(&pAd->mgmt_que);
	RTMP_SEM_UNLOCK(&pAd->mgmt_que_lock);

	if (q_entry)
		return QUEUE_ENTRY_TO_PACKET(q_entry);

	RTMP_SEM_LOCK(&pAd->fp_que_lock);
	q_entry = RemoveHeadQueue(&pAd->fp_que);
	RTMP_SEM_UNLOCK(&pAd->fp_que_lock);

	if (q_entry)
		return QUEUE_ENTRY_TO_PACKET(q_entry);

	return NULL;
}

static NDIS_PACKET *fp_first_tx_element(RTMP_ADAPTER *pAd)
{
	PQUEUE_ENTRY q_entry;

	RTMP_SEM_LOCK(&pAd->mgmt_que_lock);
	q_entry = pAd->mgmt_que.Head;
	RTMP_SEM_UNLOCK(&pAd->mgmt_que_lock);

	if (q_entry)
		return QUEUE_ENTRY_TO_PACKET(q_entry);

	RTMP_SEM_LOCK(&pAd->fp_que_lock);
	q_entry = pAd->fp_que.Head;
	RTMP_SEM_UNLOCK(&pAd->fp_que_lock);

	if (q_entry)
		return QUEUE_ENTRY_TO_PACKET(q_entry);

	return NULL;
}

#ifdef DSCP_PRI_SUPPORT
INT8	GetDscpMappedPriority(
		IN  PRTMP_ADAPTER pAd,
		IN  PNDIS_PACKET  pPkt)
{
	INT8 pri = -1;
	UINT8 dscpVal = 0;
	PUCHAR	pPktHdr = NULL;
	UINT16 protoType;

	pPktHdr = GET_OS_PKT_DATAPTR(pPkt);
	if (!pPktHdr)
		return pri;
	protoType = OS_NTOHS(get_unaligned((PUINT16)(pPktHdr + 12)));
	switch (protoType) {
	case 0x0800:
		dscpVal = ((pPktHdr[15] & 0xfc) >> 2);
		break;
	case 0x86DD:
		dscpVal = (((pPktHdr[14] & 0x0f) << 2) | ((pPktHdr[15] & 0xc0) >> 6));
		break;
	default:
		return pri;
	}
	if (dscpVal <= 63) {
		UCHAR wdev_idx = RTMP_GET_PACKET_WDEV(pPkt);

		if (wdev_idx < WDEV_NUM_MAX) {
			if (pAd->wdev_list[wdev_idx]->channel <= 14) {
				pri = pAd->dscp_pri_map[DSCP_PRI_2G_MAP][dscpVal];
			} else
				pri = pAd->dscp_pri_map[DSCP_PRI_5G_MAP][dscpVal];
		}
	}
	return pri;
}
#endif

VOID fp_tx_pkt_deq_func(RTMP_ADAPTER *pAd)
{
	TX_BLK tx_blk, *pTxBlk;
	NDIS_PACKET *pkt = NULL;
	UCHAR Wcid = 0;
	INT ret = 0;
	UINT32 KickRingBitMap = 0;
	PKT_TOKEN_CB *pktTokenCb = (PKT_TOKEN_CB *)pAd->PktTokenCb;
	UINT32 idx = 0;
	struct wifi_dev *wdev = NULL;
	STA_TR_ENTRY *tr_entry;
	struct wifi_dev_ops *wdev_ops;
	RTMP_ARCH_OP *arch_ops = &pAd->archOps;
	BOOLEAN need_schedule = (pAd->tx_dequeue_scheduable ? TRUE : FALSE);
	UCHAR user_prio;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 num_of_tx_ring = GET_NUM_OF_TX_RING(cap);

#ifdef CONFIG_TP_DBG
	struct tp_debug *tp_dbg = &pAd->tr_ctl.tp_dbg;
#endif

	if (RTMP_TEST_FLAG(pAd, TX_FLAG_STOP_DEQUEUE))
		return;

	while (need_schedule) {
		if (!pktTokenCb->tx_id_list.token_inited)
			break;

		NdisZeroMemory((UCHAR *)&tx_blk, sizeof(TX_BLK));
		pTxBlk = &tx_blk;
		pkt = fp_first_tx_element(pAd);

		if (!pkt) {
			break;
		}

		wdev = wdev_search_by_pkt(pAd, pkt);
		wdev_ops = wdev->wdev_ops;
		pTxBlk->resource_idx = arch_ops->get_resource_idx(pAd, wdev, 0, 0);
		ret = arch_ops->check_hw_resource(pAd, wdev, pTxBlk->resource_idx);

		if (ret) {
			if (ret & NDIS_STATUS_RESOURCES) {
				mtd_free_txd(pAd, pTxBlk->resource_idx);
				ret = arch_ops->check_hw_resource(pAd, wdev, pTxBlk->resource_idx);
				if (ret)
					break;
			} else
				break;
		}

		pkt = fp_get_tx_element(pAd);
		Wcid = RTMP_GET_PACKET_WCID(pkt);

		/*if wcid is out of MAC table size, free it*/
		if (Wcid >= MAX_LEN_OF_MAC_TABLE) {
			MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 ("%s(): WCID is invalid\n", __func__));
			pAd->tr_ctl.tx_sw_q_drop++;
			RELEASE_NDIS_PACKET(pAd, pkt, NDIS_STATUS_FAILURE);
			continue;
		}

		pTxBlk->wdev = wdev;

		tr_entry = &pAd->MacTab.tr_entry[Wcid];

		pTxBlk->TotalFrameNum = 1;
		pTxBlk->TotalFragNum = 1;
		pTxBlk->tr_entry = tr_entry;
		user_prio = RTMP_GET_PACKET_UP(pkt);
		pTxBlk->QueIdx = RTMP_GET_PACKET_QUEIDX(pkt);
		pTxBlk->TotalFrameLen = GET_OS_PKT_LEN(pkt);
		pTxBlk->pPacket = pkt;
		pTxBlk->TxFrameType = tx_pkt_classification(pAd, pTxBlk->pPacket, pTxBlk);
		pTxBlk->HeaderBuf = arch_ops->get_hif_buf(pAd, pTxBlk, pTxBlk->resource_idx, pTxBlk->TxFrameType);
#ifdef DSCP_PRI_SUPPORT
	/*Get the Dscp value of the packet and if there is any mapping defined set the DscpMappedPri value */
	pTxBlk->DscpMappedPri =  GetDscpMappedPriority(pAd, pkt);
#endif
		InsertTailQueue(&pTxBlk->TxPacketList, PACKET_TO_QUEUE_ENTRY(pkt));

		ret = wdev_ops->tx_pkt_handle(pAd, wdev, pTxBlk);

		if (!ret)
			KickRingBitMap |= (1 << pTxBlk->resource_idx);
	}

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
}

static INT fp_qm_init(RTMP_ADAPTER *pAd)
{
	NdisAllocateSpinLock(pAd, &pAd->fp_que_lock);
	InitializeQueueHeader(&pAd->fp_que);
	NdisAllocateSpinLock(pAd, &pAd->mgmt_que_lock);
	InitializeQueueHeader(&pAd->mgmt_que);
#ifdef FQ_SCH_SUPPORT
	fq_init(pAd);
#endif
	return NDIS_STATUS_SUCCESS;
}


static INT fp_qm_exit(RTMP_ADAPTER *pAd)
{
	PQUEUE_ENTRY q_entry;

	do {
		RTMP_SEM_LOCK(&pAd->mgmt_que_lock);
		q_entry = RemoveHeadQueue(&pAd->mgmt_que);

		if (!q_entry) {
			RTMP_SEM_UNLOCK(&pAd->mgmt_que_lock);
			break;
		}

		RELEASE_NDIS_PACKET(pAd, QUEUE_ENTRY_TO_PACKET(q_entry), NDIS_STATUS_SUCCESS);
		RTMP_SEM_UNLOCK(&pAd->mgmt_que_lock);
	} while (1);

	NdisFreeSpinLock(&pAd->mgmt_que_lock);

	do {
		RTMP_SEM_LOCK(&pAd->fp_que_lock);
		q_entry = RemoveHeadQueue(&pAd->fp_que);

		if (!q_entry) {
			RTMP_SEM_UNLOCK(&pAd->fp_que_lock);
			break;
		}

		RELEASE_NDIS_PACKET(pAd, QUEUE_ENTRY_TO_PACKET(q_entry), NDIS_STATUS_SUCCESS);
		RTMP_SEM_UNLOCK(&pAd->fp_que_lock);
	} while (1);

	NdisFreeSpinLock(&pAd->fp_que_lock);
#ifdef FQ_SCH_SUPPORT
	fq_exit(pAd);
#endif
	return NDIS_STATUS_SUCCESS;
}

static INT fp_enq_mgmtq_pkt(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PNDIS_PACKET pkt)
{
	struct tm_ops *tm_ops = pAd->tm_qm_ops;

	if (wlan_operate_get_state(wdev) != WLAN_OPER_STATE_VALID)
		goto error;

	if (pAd->mgmt_que.Number >= MGMT_QUE_MAX_NUMS)
		goto error;

	RTMP_SEM_LOCK(&pAd->mgmt_que_lock);
	InsertTailQueue(&pAd->mgmt_que, PACKET_TO_QUEUE_ENTRY(pkt));
	RTMP_SEM_UNLOCK(&pAd->mgmt_que_lock);

	tm_ops->schedule_task(pAd, TX_DEQ_TASK);

	return NDIS_STATUS_SUCCESS;
error:
	RELEASE_NDIS_PACKET(pAd, pkt, NDIS_STATUS_FAILURE);
	return NDIS_STATUS_FAILURE;
}

INT32 fp_enq_dataq_pkt(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PNDIS_PACKET pkt, UCHAR q_idx)
{
	struct tm_ops *tm_ops = pAd->tm_qm_ops;

	if (wlan_operate_get_state(wdev) != WLAN_OPER_STATE_VALID)
		goto error;

	if (pAd->fp_que.Number >= pAd->fp_que_max_size)
		goto error;

	RTMP_SEM_LOCK(&pAd->fp_que_lock);
	InsertTailQueue(&pAd->fp_que, PACKET_TO_QUEUE_ENTRY(pkt));
	RTMP_SEM_UNLOCK(&pAd->fp_que_lock);

	tm_ops->schedule_task(pAd, TX_DEQ_TASK);

	return NDIS_STATUS_SUCCESS;
error:
	pAd->tr_ctl.tx_sw_q_drop++;
	RELEASE_NDIS_PACKET(pAd, pkt, NDIS_STATUS_FAILURE);
	return NDIS_STATUS_FAILURE;
}

static INT fp_schedule_tx_que(RTMP_ADAPTER *pAd)
{
	struct tm_ops *tm_ops = pAd->tm_qm_ops;

	if ((pAd->fp_que.Number > 0) || (pAd->mgmt_que.Number > 0))
		tm_ops->schedule_task(pAd, TX_DEQ_TASK);

	return NDIS_STATUS_SUCCESS;
}

static INT fp_bss_clean_queue(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev)
{
	/*TODO: add check de-queue task idle*/
	qm_leave_queue_pkt(wdev, &ad->mgmt_que, &ad->mgmt_que_lock);
	qm_leave_queue_pkt(wdev, &ad->fp_que, &ad->fp_que_lock);
	return NDIS_STATUS_SUCCESS;
}

struct qm_ops fp_qm_ops = {
	.init = fp_qm_init,
	.exit = fp_qm_exit,
	.enq_mgmtq_pkt = fp_enq_mgmtq_pkt,
	.enq_dataq_pkt = fp_enq_dataq_pkt,
	.schedule_tx_que = fp_schedule_tx_que,
	.bss_clean_queue = fp_bss_clean_queue,
};

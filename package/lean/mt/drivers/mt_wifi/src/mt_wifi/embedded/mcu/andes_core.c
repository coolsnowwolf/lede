/*
 ***************************************************************************
 * MediaTek Inc.
 *
 * All rights reserved. source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

	Module Name:
	andes_core.c
*/
#include "rt_config.h"

#ifdef DBG_STARVATION
static void andes_starv_timeout_handle(struct starv_dbg *starv, struct starv_log_entry *entry)
{
	struct cmd_msg *cmd = container_of(starv, struct cmd_msg, starv);
	struct MCU_CTRL *ctrl = starv->block->priv;
	struct starv_log_basic *log = NULL;

	os_alloc_mem(NULL, (UCHAR **) &log, sizeof(struct starv_log_basic));
	if (!log)
		return;

	log->qsize = DlListLen(&ctrl->txq);
	log->id = cmd->attr.ext_type;
	entry->log = log;
}

static void andes_starv_block_init(struct starv_log *ctrl, struct MCU_CTRL *mcu_ctrl)
{
	struct starv_dbg_block *block = &mcu_ctrl->block;

	strncpy(block->name, "andes", sizeof(block->name));
	block->priv = mcu_ctrl;
	block->ctrl = ctrl;
	block->timeout = 1000;
	block->timeout_fn = andes_starv_timeout_handle;
	block->log_fn = starv_timeout_log_basic;
	register_starv_block(block);
}
#endif /*DBG_STARVATION*/

struct cmd_msg *AndesAllocCmdMsg(RTMP_ADAPTER *ad, unsigned int length)
{
	struct cmd_msg *msg = NULL;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(ad->hdev_ctrl);
	struct MCU_CTRL *ctl = &ad->MCUCtrl;
	PNDIS_PACKET net_pkt = NULL;
	INT32 AllocateSize = cap->cmd_header_len + length + cap->cmd_padding_len;
	net_pkt = RTMP_AllocateFragPacketBuffer(ad, AllocateSize);

	if (!net_pkt) {
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("can not allocate net_pkt\n"));
		goto error0;
	}

	OS_PKT_RESERVE(net_pkt, cap->cmd_header_len);
	os_alloc_mem(NULL, (PUCHAR *)&msg, sizeof(*msg));

	if (!msg) {
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("can not allocate cmd msg\n"));
		goto error1;
	}

	CMD_MSG_CB(net_pkt)->msg = msg;
	os_zero_mem(msg, sizeof(*msg));
	msg->priv = (void *)ad;
	msg->net_pkt = net_pkt;
	ctl->alloc_cmd_msg++;
	return msg;

error1:
	RTMPFreeNdisPacket(ad, net_pkt);
error0:
	return NULL;
}

VOID AndesInitCmdMsg(struct cmd_msg *msg, CMD_ATTRIBUTE attr)
{
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s:: mcu_dest(%d), cmd_type(0x%x), ExtCmdType(0x%x)\n",
			  __func__, attr.mcu_dest, attr.type, attr.ext_type));
	SET_CMD_MSG_PORT_QUEUE_ID(msg, GetRealPortQueueID(msg, attr.type));
	SET_CMD_MSG_MCU_DEST(msg, attr.mcu_dest);
	SET_CMD_MSG_TYPE(msg, attr.type);
	SET_CMD_MSG_CTRL_FLAGS(msg, attr.ctrl.flags);
	SET_CMD_MSG_EXT_TYPE(msg, attr.ext_type);
	SET_CMD_MSG_CTRL_RSP_WAIT_MS_TIME(msg, attr.ctrl.wait_ms_time);

	if (IS_CMD_MSG_NEED_SYNC_WITH_FW_FLAG_SET(msg))
		RTMP_OS_INIT_COMPLETION(&msg->ack_done);

	SET_CMD_MSG_RETRY_TIMES(msg, 0);
	SET_CMD_MSG_CTRL_RSP_EXPECT_SIZE(msg, attr.ctrl.expect_size);
	SET_CMD_MSG_RSP_WB_BUF_IN_CALBK(msg, attr.rsp.wb_buf_in_calbk);
	SET_CMD_MSG_RSP_HANDLER(msg, attr.rsp.handler);
}

VOID AndesAppendCmdMsg(struct cmd_msg *msg, char *data, unsigned int len)
{
	PNDIS_PACKET net_pkt;

	if (!msg)
		return;

	net_pkt = msg->net_pkt;

	if (data)
		memcpy(OS_PKT_TAIL_BUF_EXTEND(net_pkt, len), data, len);
}


VOID AndesAppendHeadCmdMsg(struct cmd_msg *msg, char *data, unsigned int len)
{
	PNDIS_PACKET net_pkt = msg->net_pkt;

	if (data)
		memcpy(OS_PKT_HEAD_BUF_EXTEND(net_pkt, len), data, len);
}

VOID AndesFreeCmdMsg(struct cmd_msg *msg)
{
	RTMP_ADAPTER *ad = NULL;
	ULONG flags = 0;
	struct MCU_CTRL *ctl = NULL;

	if (!msg)
		return;

	ad = (RTMP_ADAPTER *)(msg->priv);

	if (!ad) {
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: pAd is null\n", __func__));
		goto free_memory;
	}

	ctl = &ad->MCUCtrl;

	if (!ctl) {
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: MCUCtrl is null\n", __func__));
		goto free_memory;
	}

	if (IS_CMD_MSG_NEED_SYNC_WITH_FW_FLAG_SET(msg))
		RTMP_OS_EXIT_COMPLETION(&msg->ack_done);

	OS_SPIN_LOCK_IRQSAVE(&ctl->msg_lock, &flags);
	ctl->free_cmd_msg++;
	OS_SPIN_UNLOCK_IRQRESTORE(&ctl->msg_lock, &flags);
free_memory:
	os_free_mem(msg);
}

VOID AndesForceFreeCmdMsg(struct cmd_msg *msg)
{
	RTMP_ADAPTER *ad = NULL;
	struct MCU_CTRL *ctl = NULL;
	ULONG flags = 0;

	if (!msg)
		return;

	ad = (RTMP_ADAPTER *)(msg->priv);

	if (!ad) {
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: pAd is null\n", __func__));
		goto free_memory;
	}

	ctl = &ad->MCUCtrl;

	if (!ctl) {
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: MCUCtrl is null\n", __func__));
		goto free_memory;
	}

	if (IS_CMD_MSG_NEED_SYNC_WITH_FW_FLAG_SET(msg))
		RTMP_OS_EXIT_COMPLETION(&msg->ack_done);

	OS_SPIN_LOCK_IRQSAVE(&ctl->msg_lock, &flags);
	ctl->free_cmd_msg++;
	OS_SPIN_UNLOCK_IRQRESTORE(&ctl->msg_lock, &flags);
free_memory:

	if (ad && msg->net_pkt)
		RTMPFreeNdisPacket(ad, msg->net_pkt);

	os_free_mem(msg);
}


BOOLEAN IsInbandCmdProcessing(RTMP_ADAPTER *ad)
{
	BOOLEAN ret = 0;
	return ret;
}


UCHAR GetCmdRspNum(RTMP_ADAPTER *ad)
{
	UCHAR Num = 0;
	return Num;
}


VOID AndesIncErrorCount(struct MCU_CTRL *ctl, enum cmd_msg_error_type type)
{
	if (OS_TEST_BIT(MCU_INIT, &ctl->flags)) {
		switch (type) {
		case error_tx_kickout_fail:
			ctl->tx_kickout_fail_count++;
			break;

		case error_tx_timeout_fail:
			ctl->tx_timeout_fail_count++;
			break;

		case error_rx_receive_fail:
			ctl->rx_receive_fail_count++;
			break;

		default:
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s:unknown cmd_msg_error_type(%d)\n",
					  __func__, type));
		}
	}
}


static NDIS_SPIN_LOCK *AndesGetSpinLock(struct MCU_CTRL *ctl, DL_LIST *list)
{
	NDIS_SPIN_LOCK *lock = NULL;

	if (list == &ctl->txq)
		lock = &ctl->txq_lock;
	else if (list == &ctl->rxq)
		lock = &ctl->rxq_lock;
	else if (list == &ctl->ackq)
		lock = &ctl->ackq_lock;
	else if (list == &ctl->kickq)
		lock = &ctl->kickq_lock;
	else if (list == &ctl->tx_doneq)
		lock = &ctl->tx_doneq_lock;
	else if (list == &ctl->rx_doneq)
		lock = &ctl->rx_doneq_lock;

	else {
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("list pointer = %p\n", list));
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("txq = %p, rxq = %p, ackq = %p, kickq = %p, tx_doneq = %p, rx_doneq = %p\n",
				  &ctl->txq, &ctl->rxq, &ctl->ackq, &ctl->kickq,
				  &ctl->tx_doneq, &ctl->rx_doneq));
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s:illegal list\n", __func__));
	}

	return lock;
}


static inline UCHAR AndesGetCmdMsgSeq(RTMP_ADAPTER *ad)
{
	struct MCU_CTRL *ctl = &ad->MCUCtrl;
	struct cmd_msg *msg;
	unsigned long flags;

	RTMP_SPIN_LOCK_IRQSAVE(&ctl->ackq_lock, &flags);
get_seq:
	ctl->cmd_seq >= 0xf ? ctl->cmd_seq = 1 : ctl->cmd_seq++;
	DlListForEach(msg, &ctl->ackq, struct cmd_msg, list) {
		if (msg->seq == ctl->cmd_seq) {
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("command(seq: %d) is still running\n", ctl->cmd_seq));
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("command response nums = %d\n", GetCmdRspNum(ad)));
			goto get_seq;
		}
	}
	RTMP_SPIN_UNLOCK_IRQRESTORE(&ctl->ackq_lock, &flags);
	return ctl->cmd_seq;
}


static VOID _AndesQueueTailCmdMsg(DL_LIST *list, struct cmd_msg *msg, enum cmd_msg_state state)
{
	msg->state = state;
	DlListAddTail(list, &msg->list);
}


VOID AndesQueueTailCmdMsg(DL_LIST *list, struct cmd_msg *msg, enum cmd_msg_state state)
{
	unsigned long flags;
	NDIS_SPIN_LOCK *lock;
	RTMP_ADAPTER *ad = (RTMP_ADAPTER *)msg->priv;
	struct MCU_CTRL *ctl = &ad->MCUCtrl;

	lock = AndesGetSpinLock(ctl, list);

	if (lock) {
		RTMP_SPIN_LOCK_IRQSAVE(lock, &flags);
		_AndesQueueTailCmdMsg(list, msg, state);
		RTMP_SPIN_UNLOCK_IRQRESTORE(lock, &flags);
	}
}


static VOID _AndesQueueHeadCmdMsg(DL_LIST *list, struct cmd_msg *msg, enum cmd_msg_state state)
{
	msg->state = state;
	DlListAdd(list, &msg->list);
}


VOID AndesQueueHeadCmdMsg(DL_LIST *list, struct cmd_msg *msg, enum cmd_msg_state state)
{
	unsigned long flags;
	NDIS_SPIN_LOCK *lock;
	RTMP_ADAPTER *ad = (RTMP_ADAPTER *)msg->priv;
	struct MCU_CTRL *ctl = &ad->MCUCtrl;

	lock = AndesGetSpinLock(ctl, list);

	if (lock) {
		RTMP_SPIN_LOCK_IRQSAVE(lock, &flags);
		_AndesQueueHeadCmdMsg(list, msg, state);
		RTMP_SPIN_UNLOCK_IRQRESTORE(lock, &flags);
	}
}

UINT32 AndesQueueLen(struct MCU_CTRL *ctl, DL_LIST *list)
{
	UINT32 qlen = 0;
	unsigned long flags;
	NDIS_SPIN_LOCK *lock;

	lock = AndesGetSpinLock(ctl, list);

	if (lock) {
		RTMP_SPIN_LOCK_IRQSAVE(lock, &flags);
		qlen = DlListLen(list);
		RTMP_SPIN_UNLOCK_IRQRESTORE(lock, &flags);
	}

	return qlen;
}

/*AndesQueueEmpty is uselsess, disable it*/

static VOID AndesQueueInit(struct MCU_CTRL *ctl, DL_LIST *list)
{
	unsigned long flags;
	NDIS_SPIN_LOCK *lock;

	lock = AndesGetSpinLock(ctl, list);

	if (lock) {
		RTMP_SPIN_LOCK_IRQSAVE(lock, &flags);
		DlListInit(list);
		RTMP_SPIN_UNLOCK_IRQRESTORE(lock, &flags);
	}
}


VOID _AndesUnlinkCmdMsg(struct cmd_msg *msg, DL_LIST *list)
{
	if (!msg)
		return;

	DlListDel(&msg->list);
}


VOID AndesUnlinkCmdMsg(struct cmd_msg *msg, DL_LIST *list)
{
	unsigned long flags;
	NDIS_SPIN_LOCK *lock;
	RTMP_ADAPTER *ad = (RTMP_ADAPTER *)msg->priv;
	struct MCU_CTRL *ctl = &ad->MCUCtrl;

	lock = AndesGetSpinLock(ctl, list);

	if (lock) {
		RTMP_SPIN_LOCK_IRQSAVE(lock, &flags);
		_AndesUnlinkCmdMsg(msg, list);
		RTMP_SPIN_UNLOCK_IRQRESTORE(lock, &flags);
	}
}


static struct cmd_msg *_AndesDequeueCmdMsg(DL_LIST *list)
{
	struct cmd_msg *msg;

	msg = DlListFirst(list, struct cmd_msg, list);
	_AndesUnlinkCmdMsg(msg, list);
	return msg;
}


struct cmd_msg *AndesDequeueCmdMsg(struct MCU_CTRL *ctl, DL_LIST *list)
{
	unsigned long flags;
	struct cmd_msg *msg = NULL;
	NDIS_SPIN_LOCK *lock;

	lock = AndesGetSpinLock(ctl, list);

	if (lock) {
		RTMP_SPIN_LOCK_IRQSAVE(lock, &flags);
		msg = _AndesDequeueCmdMsg(list);
		RTMP_SPIN_UNLOCK_IRQRESTORE(lock, &flags);
	}

	return msg;
}



#ifdef RTMP_PCI_SUPPORT
VOID PciKickOutCmdMsgComplete(PNDIS_PACKET net_pkt)
{
	struct cmd_msg *msg = CMD_MSG_CB(net_pkt)->msg;
	RTMP_ADAPTER *ad = (RTMP_ADAPTER *)msg->priv;
	struct MCU_CTRL *ctl = &ad->MCUCtrl;

	if (!OS_TEST_BIT(MCU_INIT, &ctl->flags))
		return;

	if (!IS_CMD_MSG_NEED_SYNC_WITH_FW_FLAG_SET(msg)) {
		AndesUnlinkCmdMsg(msg, &ctl->kickq);
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 ("%s: msg state = %d\n", __func__, msg->state));
		AndesQueueTailCmdMsg(&ctl->tx_doneq, msg, tx_done);
	} else  {
		if (msg->state != tx_done)
			msg->state = wait_ack;
	}

	AndesBhSchedule(ad);
}
#endif /* RTMP_PCI_SUPPORT */




VOID AndesRxProcessCmdMsg(RTMP_ADAPTER *ad, struct cmd_msg *rx_msg)
{
	RX_BLK RxBlk;

	mt_rx_pkt_process(ad, HIF_RX_IDX1, &RxBlk, rx_msg->net_pkt);
}


VOID AndesCmdMsgBh(unsigned long param)
{
	RTMP_ADAPTER *ad = (RTMP_ADAPTER *)param;
	struct MCU_CTRL *ctl = &ad->MCUCtrl;
	struct cmd_msg *msg = NULL;

	while ((msg = AndesDequeueCmdMsg(ctl, &ctl->rx_doneq))) {
		switch (msg->state) {
		case rx_done:
			AndesRxProcessCmdMsg(ad, msg);
			AndesFreeCmdMsg(msg);
			break;

		case rx_receive_fail:
			AndesFreeCmdMsg(msg);
			break;

		default:
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("unknow msg state(%d)\n", msg->state));
			AndesFreeCmdMsg(msg);
			break;
		}
	}

	while ((msg = AndesDequeueCmdMsg(ctl, &ctl->tx_doneq))) {
		switch (msg->state) {
		case tx_done:
		case tx_kickout_fail:
		case tx_timeout_fail:
#ifdef DBG_STARVATION
			starv_dbg_put(&msg->starv);
#endif /*DBG_STARVATION*/
			AndesFreeCmdMsg(msg);
			break;

		default:
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("unknow msg state(%d)\n", msg->state));
			AndesFreeCmdMsg(msg);
			break;
		}
	}

	if (OS_TEST_BIT(MCU_INIT, &ctl->flags)) {
		AndesBhSchedule(ad);
	}
}




VOID AndesBhSchedule(RTMP_ADAPTER *ad)
{
	struct MCU_CTRL *ctl = &ad->MCUCtrl;

	if (!OS_TEST_BIT(MCU_INIT, &ctl->flags))
		return;

	if (((AndesQueueLen(ctl, &ctl->rx_doneq) > 0)
		 || (AndesQueueLen(ctl, &ctl->tx_doneq) > 0))
		&& OS_TEST_BIT(MCU_INIT, &ctl->flags)) {
#ifndef WORKQUEUE_BH
		RTMP_NET_TASK_DATA_ASSIGN(&ctl->cmd_msg_task, (unsigned long)(ad));
		RTMP_OS_TASKLET_SCHE(&ctl->cmd_msg_task);
#else
		tasklet_hi_schedule(&ctl->cmd_msg_task);
#endif
	}
}


VOID AndesCleanupCmdMsg(RTMP_ADAPTER *ad, DL_LIST *list)
{
	unsigned long flags;
	struct cmd_msg *msg, *msg_tmp;
	NDIS_SPIN_LOCK *lock;
	struct MCU_CTRL *ctl = &ad->MCUCtrl;

	lock = AndesGetSpinLock(ctl, list);
	if (lock)
		RTMP_SPIN_LOCK_IRQSAVE(lock, &flags);
	DlListForEachSafe(msg, msg_tmp, list, struct cmd_msg, list) {
		_AndesUnlinkCmdMsg(msg, list);

		/*If need wait, clean up need to trigger complete for andes send cmd to free msg*/
		if (IS_CMD_MSG_NEED_SYNC_WITH_FW_FLAG_SET(msg))
			RTMP_OS_EXIT_COMPLETION(&msg->ack_done);
		else
			AndesFreeCmdMsg(msg);
	}
	DlListInit(list);
	if (lock)
		RTMP_SPIN_UNLOCK_IRQRESTORE(lock, &flags);
}


#if defined(RTMP_PCI_SUPPORT) || defined(RTMP_RBUS_SUPPORT)
static VOID AndesCtrlPciInit(RTMP_ADAPTER *ad)
{
	struct MCU_CTRL *ctl = &ad->MCUCtrl;

	RTMP_CLEAR_FLAG(ad, fRTMP_ADAPTER_MCU_SEND_IN_BAND_CMD);
	ctl->cmd_seq = 0;
#ifndef WORKQUEUE_BH
	RTMP_OS_TASKLET_INIT(ad, &ctl->cmd_msg_task, AndesCmdMsgBh, (unsigned long)ad);
#else
	tasklet_init(&ctl->cmd_msg_task, AndesCmdMsgBh, (unsigned long)ad);
#endif
	NdisAllocateSpinLock(ad, &ctl->txq_lock);
	AndesQueueInit(ctl, &ctl->txq);
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ctl->txq = %p\n", &ctl->txq));
	NdisAllocateSpinLock(ad, &ctl->rxq_lock);
	AndesQueueInit(ctl, &ctl->rxq);
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ctl->rxq = %p\n", &ctl->rxq));
	NdisAllocateSpinLock(ad, &ctl->ackq_lock);
	AndesQueueInit(ctl, &ctl->ackq);
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ctl->ackq = %p\n", &ctl->ackq));
	NdisAllocateSpinLock(ad, &ctl->kickq_lock);
	AndesQueueInit(ctl, &ctl->kickq);
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ctl->kickq = %p\n", &ctl->kickq));
	NdisAllocateSpinLock(ad, &ctl->tx_doneq_lock);
	AndesQueueInit(ctl, &ctl->tx_doneq);
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ctl->tx_doneq = %p\n", &ctl->tx_doneq));
	NdisAllocateSpinLock(ad, &ctl->rx_doneq_lock);
	AndesQueueInit(ctl, &ctl->rx_doneq);
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ctl->rx_doneq = %p\n", &ctl->rx_doneq));
	ctl->tx_kickout_fail_count = 0;
	ctl->tx_timeout_fail_count = 0;
	ctl->rx_receive_fail_count = 0;
	ctl->alloc_cmd_msg = 0;
	ctl->free_cmd_msg = 0;
	OS_SET_BIT(MCU_INIT, &ctl->flags);
	ctl->ad = ad;
}
#endif






VOID AndesCtrlInit(RTMP_ADAPTER *pAd)
{
	struct MCU_CTRL *ctl = &pAd->MCUCtrl;

	if (!OS_TEST_BIT(MCU_INIT, &ctl->flags)) {
#ifdef RTMP_MAC_PCI

		if (IS_PCI_INF(pAd) || IS_RBUS_INF(pAd))
			AndesCtrlPciInit(pAd);

#endif
	}

	ctl->power_on = FALSE;
	ctl->dpd_on = FALSE;
	ctl->RxStream0 = 0;
	ctl->RxStream1 = 0;
	NdisAllocateSpinLock(pAd, &ctl->msg_lock);
#ifdef DBG_STARVATION
	andes_starv_block_init(&pAd->starv_log_ctrl, ctl);
#endif /*DBG_STARVATION*/
}




#ifdef RTMP_PCI_SUPPORT
static VOID AndesCtrlPciExit(RTMP_ADAPTER *ad)
{
	struct MCU_CTRL *ctl = &ad->MCUCtrl;

	RTMP_CLEAR_FLAG(ad, fRTMP_ADAPTER_MCU_SEND_IN_BAND_CMD);
	/*clear andes queue by tasklet*/
	AndesBhSchedule(ad);
	OS_CLEAR_BIT(MCU_INIT, &ctl->flags);
	RTMP_OS_TASKLET_KILL(&ctl->cmd_msg_task);
	AndesCleanupCmdMsg(ad, &ctl->txq);
	NdisFreeSpinLock(&ctl->txq_lock);
	AndesCleanupCmdMsg(ad, &ctl->ackq);
	NdisFreeSpinLock(&ctl->ackq_lock);
	AndesCleanupCmdMsg(ad, &ctl->rxq);
	NdisFreeSpinLock(&ctl->rxq_lock);
	AndesCleanupCmdMsg(ad, &ctl->kickq);
	NdisFreeSpinLock(&ctl->kickq_lock);
	AndesCleanupCmdMsg(ad, &ctl->tx_doneq);
	NdisFreeSpinLock(&ctl->tx_doneq_lock);
	AndesCleanupCmdMsg(ad, &ctl->rx_doneq);
	NdisFreeSpinLock(&ctl->rx_doneq_lock);
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("tx_kickout_fail_count = %ld\n", ctl->tx_kickout_fail_count));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("tx_timeout_fail_count = %ld\n", ctl->tx_timeout_fail_count));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("rx_receive_fail_count = %ld\n", ctl->rx_receive_fail_count));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("alloc_cmd_msg = %ld\n", ctl->alloc_cmd_msg));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("free_cmd_msg = %ld\n", ctl->free_cmd_msg));
}
#endif



VOID AndesCtrlExit(RTMP_ADAPTER *pAd)
{
	struct MCU_CTRL *ctl = &pAd->MCUCtrl;

	if (OS_TEST_BIT(MCU_INIT, &ctl->flags)) {
#ifdef DBG_STARVATION
		unregister_starv_block(&ctl->block);
#endif /*DBG_STARVATION*/
#ifdef RTMP_PCI_SUPPORT

		if (IS_PCI_INF(pAd) || IS_RBUS_INF(pAd))
			AndesCtrlPciExit(pAd);

#endif
	}

	ctl->power_on = FALSE;
	ctl->dpd_on = FALSE;
}


static INT32 AndesDequeueAndKickOutCmdMsgs(RTMP_ADAPTER *ad)
{
	struct cmd_msg *msg = NULL;
	VOID *net_pkt = NULL;
	struct MCU_CTRL *ctl = &ad->MCUCtrl;
	int ret = NDIS_STATUS_SUCCESS;
	struct _RTMP_CHIP_OP *chip_ops = hc_get_chip_ops(ad->hdev_ctrl);

	while ((msg = AndesDequeueCmdMsg(ctl, &ctl->txq)) != NULL) {
		if (!RTMP_TEST_FLAG(ad, fRTMP_ADAPTER_MCU_SEND_IN_BAND_CMD)     ||
			RTMP_TEST_FLAG(ad, fRTMP_ADAPTER_NIC_NOT_EXIST)         ||
			RTMP_TEST_FLAG(ad, fRTMP_ADAPTER_SUSPEND)) {
			if (!IS_CMD_MSG_NEED_SYNC_WITH_FW_FLAG_SET(msg))
				AndesForceFreeCmdMsg(msg);

			continue;
		}

		net_pkt = (VOID *)msg->net_pkt;

		if (msg->state != tx_retransmit) {
			if (IS_CMD_MSG_NEED_SYNC_WITH_FW_FLAG_SET(msg))
				msg->seq = AndesGetCmdMsgSeq(ad);
			else
				msg->seq = 0;

			if (chip_ops->andes_fill_cmd_header != NULL)
				chip_ops->andes_fill_cmd_header(msg, net_pkt);
		}

#if defined(RTMP_PCI_SUPPORT) || defined(RTMP_RBUS_SUPPORT)

		if (chip_ops->pci_kick_out_cmd_msg != NULL)
			ret = chip_ops->pci_kick_out_cmd_msg(ad, msg);

#endif

		if (ret) {
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("kick out msg fail\n"));

			if (ret == NDIS_STATUS_FAILURE)
				AndesForceFreeCmdMsg(msg);

			break;
		}
	}

	AndesBhSchedule(ad);
	return ret;
}



static INT32 AndesWaitForCompleteTimeout(struct cmd_msg *msg, ULONG timeout)
{
	int ret = 0;
	ULONG expire = timeout ?
				   RTMPMsecsToJiffies(timeout) : RTMPMsecsToJiffies(CMD_MSG_TIMEOUT);
	ret = RTMP_OS_WAIT_FOR_COMPLETION_TIMEOUT(&msg->ack_done, expire);
	return ret;
}


INT32 AndesSendCmdMsg(PRTMP_ADAPTER ad, struct cmd_msg *msg)
{
	struct MCU_CTRL *ctl = &ad->MCUCtrl;
	BOOLEAN is_cmd_need_wait = IS_CMD_MSG_NEED_SYNC_WITH_FW_FLAG_SET(msg);
	int ret = NDIS_STATUS_SUCCESS;

	if (in_interrupt() && IS_CMD_MSG_NEED_SYNC_WITH_FW_FLAG_SET(msg)) {
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("BUG: %s is called from invalid context\n",
				  __func__));
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: Command type = %x, Extension command type = %x\n",
				  __func__, msg->attr.type, msg->attr.ext_type));
		AndesForceFreeCmdMsg(msg);
		return NDIS_STATUS_FAILURE;
	}

	if (!RTMP_TEST_FLAG(ad, fRTMP_ADAPTER_MCU_SEND_IN_BAND_CMD)     ||
		RTMP_TEST_FLAG(ad, fRTMP_ADAPTER_NIC_NOT_EXIST)         ||
		RTMP_TEST_FLAG(ad, fRTMP_ADAPTER_SUSPEND)) {
		if (!RTMP_TEST_FLAG(ad, fRTMP_ADAPTER_MCU_SEND_IN_BAND_CMD)) {
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: Could not send in band command due to diablefRTMP_ADAPTER_MCU_SEND_IN_BAND_CMD\n",
					  __func__));
		} else if (RTMP_TEST_FLAG(ad, fRTMP_ADAPTER_NIC_NOT_EXIST)) {
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: Could not send in band command due to fRTMP_ADAPTER_NIC_NOT_EXIST\n",
					  __func__));
		} else if (RTMP_TEST_FLAG(ad, fRTMP_ADAPTER_SUSPEND)) {
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: Could not send in band command due to fRTMP_ADAPTER_SUSPEND\n",
					  __func__));
		}

		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: Command type = %x, Extension command type = %x\n",
				  __func__, msg->attr.type, msg->attr.ext_type));

		AndesForceFreeCmdMsg(msg);
		return NDIS_STATUS_FAILURE;
	}

#ifdef DBG_STARVATION
	starv_dbg_init(&ctl->block, &msg->starv);
	starv_dbg_get(&msg->starv);
#endif /*DBG_STARVATION*/
	AndesQueueTailCmdMsg(&ctl->txq, msg, tx_start);
retransmit:

	if (AndesDequeueAndKickOutCmdMsgs(ad) != NDIS_STATUS_SUCCESS)
		goto bailout;

	/* Wait for response */
	if (is_cmd_need_wait) {
		enum cmd_msg_state state = 0;
		ULONG IsComplete;

		IsComplete = AndesWaitForCompleteTimeout(msg, msg->attr.ctrl.wait_ms_time);

		if (!OS_TEST_BIT(MCU_INIT, &ctl->flags)) {
			/*If need wait, clean up will trigger complete for here to free msg*/
			AndesFreeCmdMsg(msg);
			goto bailout;
		}

		if (!IsComplete) {
			ret = NDIS_STATUS_FAILURE;
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("FWCmdTimeout: command (%x), ext_cmd_type (%x), seq(%d), timeout(%dms)\n",
					  msg->attr.type, msg->attr.ext_type,
					  msg->seq,
					  (msg->attr.ctrl.wait_ms_time == 0) ?
					  CMD_MSG_TIMEOUT : msg->attr.ctrl.wait_ms_time));
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("pAd->Flags  = 0x%.8lx\n", ad->Flags));
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("txq qlen = %d\n", AndesQueueLen(ctl, &ctl->txq)));
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("rxq qlen = %d\n", AndesQueueLen(ctl, &ctl->rxq)));
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("kickq qlen = %d\n", AndesQueueLen(ctl, &ctl->kickq)));
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("ackq qlen = %d\n", AndesQueueLen(ctl, &ctl->ackq)));
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("tx_doneq.qlen = %d\n", AndesQueueLen(ctl, &ctl->tx_doneq)));
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("rx_done qlen = %d\n", AndesQueueLen(ctl, &ctl->rx_doneq)));

			if (msg->state == wait_cmd_out_and_ack) {
				AndesUnlinkCmdMsg(msg, &ctl->ackq);
			} else if (msg->state == wait_ack)
				AndesUnlinkCmdMsg(msg, &ctl->ackq);

			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: msg state = %d\n", __func__, msg->state));
			AndesIncErrorCount(ctl, error_tx_timeout_fail);
			state = tx_timeout_fail;

			if (msg->retry_times > 0)
				msg->retry_times--;

			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("msg->retry_times = %d\n", msg->retry_times));
			ASSERT(FALSE);
		} else {
			if (msg->state == tx_kickout_fail) {
				state = tx_kickout_fail;
				msg->retry_times--;
			} else {
				if (msg->state == wait_cmd_out_and_ack) {
					AndesUnlinkCmdMsg(msg, &ctl->ackq);
				} else if (msg->state == wait_ack)
					AndesUnlinkCmdMsg(msg, &ctl->ackq);

				state = tx_done;
				msg->retry_times = 0;
			}
		}

		if (is_cmd_need_wait && (msg->retry_times > 0)) {
			RTMP_OS_EXIT_COMPLETION(&msg->ack_done);
			RTMP_OS_INIT_COMPLETION(&msg->ack_done);
			state = tx_retransmit;
			AndesQueueHeadCmdMsg(&ctl->txq, msg, state);
			goto retransmit;
		} else {
			if (msg->attr.ext_type == EXT_CMD_STAREC_UPDATE) {
				/* Only StaRec update command read FW's response to minimize the impact.
				 FW's response will become the final return value.
				*/
				ret = msg->cmd_return_status;
			}

			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 ("%s: msg state = %d\n", __func__, state));
			/* msg will be free after enqueuing to tx_doneq. So msg is not able to pass FW's response to caller. */
			AndesQueueTailCmdMsg(&ctl->tx_doneq, msg, state);
		}
	}
bailout:
	return ret;
}

INT32 MtCmdSendMsg(PRTMP_ADAPTER ad, struct cmd_msg *msg)
{
	INT32 ret = 0;

	ret = AndesSendCmdMsg(ad, msg);
	return ret;
}


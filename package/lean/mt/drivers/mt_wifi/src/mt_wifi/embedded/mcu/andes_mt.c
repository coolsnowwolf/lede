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
	andes_mt.c
*/

#include	"rt_config.h"

#if defined (RLM_CAL_CACHE_SUPPORT) || defined(PRE_CAL_TRX_SET2_SUPPORT)
#include    "phy/rlm_cal_cache.h"
#endif /* defined(RLM_CAL_CACHE_SUPPORT) || defined(PRE_CAL_TRX_SET2_SUPPORT) */

#ifdef UNIFY_FW_CMD
/* static decaration */
static VOID AndesMTFillTxDHeader(struct cmd_msg *msg, PNDIS_PACKET net_pkt);
#endif /* UNIFY_FW_CMD */



#if defined(RTMP_PCI_SUPPORT) || defined(RTMP_RBUS_SUPPORT)
INT32 AndesMTPciKickOutCmdMsg(PRTMP_ADAPTER pAd, struct cmd_msg *msg)
{
	int ret = NDIS_STATUS_SUCCESS;
	unsigned long flags = 0;
	ULONG FreeNum;
	PNDIS_PACKET net_pkt = msg->net_pkt;
	UINT32 SwIdx = 0;
	UCHAR *pSrcBufVA;
	UINT SrcBufLen = 0;
	PACKET_INFO PacketInfo;
	TXD_STRUC *pTxD;
	struct MCU_CTRL *ctl = &pAd->MCUCtrl;
#ifdef RT_BIG_ENDIAN
	TXD_STRUC *pDestTxD;
	UCHAR tx_hw_info[TXD_SIZE];
#endif
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);
	RTMP_CTRL_RING *ring = &hif->CtrlRing;
	NDIS_SPIN_LOCK *lock = &hif->CtrlRingLock;

	if (!OS_TEST_BIT(MCU_INIT, &ctl->flags))
		return NDIS_STATUS_FAILURE;

	FreeNum = GET_CTRLRING_FREENO(pAd);

	if (FreeNum == 0) {
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_WARN,
				 ("%s FreeNum == 0 (TxCpuIdx = %d, TxDmaIdx = %d, TxSwFreeIdx = %d)\n",
				  __func__, ring->TxCpuIdx,
				  ring->TxDmaIdx, ring->TxSwFreeIdx));
		return NDIS_STATUS_FAILURE;
	}

	RTMP_SPIN_LOCK_IRQSAVE(lock, &flags);
	RTMP_QueryPacketInfo(net_pkt, &PacketInfo, &pSrcBufVA, &SrcBufLen);

	if (pSrcBufVA == NULL) {
		RTMP_SPIN_UNLOCK_IRQRESTORE(lock, &flags);
		return NDIS_STATUS_FAILURE;
	}

	SwIdx = ring->TxCpuIdx;
#ifdef RT_BIG_ENDIAN
	pDestTxD  = (TXD_STRUC *)ring->Cell[SwIdx].AllocVa;
	NdisMoveMemory(&tx_hw_info[0], (UCHAR *)pDestTxD, TXD_SIZE);
	pTxD = (TXD_STRUC *)&tx_hw_info[0];
#else
	pTxD  = (TXD_STRUC *)ring->Cell[SwIdx].AllocVa;
#endif
#ifdef RT_BIG_ENDIAN
	RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#endif /* RT_BIG_ENDIAN */
	ring->Cell[SwIdx].pNdisPacket = net_pkt;
	ring->Cell[SwIdx].pNextNdisPacket = NULL;
	ring->Cell[SwIdx].PacketPa = PCI_MAP_SINGLE(pAd, (pSrcBufVA),
			(SrcBufLen), 0, RTMP_PCI_DMA_TODEVICE);
	pTxD->LastSec0 = 1;
	pTxD->LastSec1 = 0;
	pTxD->SDLen0 = SrcBufLen;
	pTxD->SDLen1 = 0;
	pTxD->SDPtr0 = ring->Cell[SwIdx].PacketPa;
	pTxD->Burst = 0;
	pTxD->DMADONE = 0;
#ifdef RT_BIG_ENDIAN
	RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
	WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, FALSE, TYPE_TXD);
#endif
	/* flush dcache if no consistent memory is supported */
	RTMP_DCACHE_FLUSH(SrcBufPA, SrcBufLen);
	RTMP_DCACHE_FLUSH(ring->Cell[SwIdx].AllocPa, TXD_SIZE);
	/* Increase TX_CTX_IDX, but write to register later.*/
	INC_RING_INDEX(ring->TxCpuIdx, CTL_RING_SIZE);

	if (IS_CMD_MSG_NEED_SYNC_WITH_FW_FLAG_SET(msg)) {
		AndesQueueTailCmdMsg(&ctl->ackq, msg, wait_ack);
		msg->sending_time_in_jiffies = jiffies;
	} else
		AndesQueueTailCmdMsg(&ctl->tx_doneq, msg, tx_done);

	if (!OS_TEST_BIT(MCU_INIT, &ctl->flags)) {
		RTMP_SPIN_UNLOCK_IRQRESTORE(lock, &flags);
		return -1;
	}

	HIF_IO_WRITE32(pAd, ring->hw_cidx_addr, ring->TxCpuIdx);
	RTMP_SPIN_UNLOCK_IRQRESTORE(lock, &flags);
	return ret;
}

#if defined(MT7615) || defined(MT7622) || defined(P18) || defined(MT7663)
INT32 AndesMTPciKickOutCmdMsgFwDlRing(PRTMP_ADAPTER pAd, struct cmd_msg *msg)
{
	int ret = NDIS_STATUS_SUCCESS;
	unsigned long flags = 0;
	ULONG FreeNum;
	PNDIS_PACKET net_pkt = msg->net_pkt;
	UINT32 SwIdx = 0;
	UCHAR *pSrcBufVA;
	UINT SrcBufLen = 0;
	PACKET_INFO PacketInfo;
	TXD_STRUC *pTxD;
	struct MCU_CTRL *ctl = &pAd->MCUCtrl;
	RTMP_RING *pRing;
#ifdef RT_BIG_ENDIAN
	TXD_STRUC *pDestTxD;
	UCHAR tx_hw_info[TXD_SIZE];
#endif
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);

	if (!OS_TEST_BIT(MCU_INIT, &ctl->flags))
		return -1;

	pRing = (RTMP_RING *)(&(hif->FwDwloRing));
	FreeNum = GET_FWDWLORING_FREENO(pRing);

	if (FreeNum == 0) {
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_WARN,
				 ("%s FreeNum == 0 (TxCpuIdx = %d, TxDmaIdx = %d, TxSwFreeIdx = %d)\n",
				  __func__, pRing->TxCpuIdx, pRing->TxDmaIdx, pRing->TxSwFreeIdx));
		return NDIS_STATUS_FAILURE;
	}

	RTMP_SPIN_LOCK_IRQSAVE(&pRing->RingLock, &flags);
	RTMP_QueryPacketInfo(net_pkt, &PacketInfo, &pSrcBufVA, &SrcBufLen);

	if (pSrcBufVA == NULL) {
		RTMP_SPIN_UNLOCK_IRQRESTORE(&pRing->RingLock, &flags);
		return NDIS_STATUS_FAILURE;
	}

	SwIdx = pRing->TxCpuIdx;
#ifdef RT_BIG_ENDIAN
	pDestTxD  = (TXD_STRUC *)pRing->Cell[SwIdx].AllocVa;
	NdisMoveMemory(&tx_hw_info[0], (UCHAR *)pDestTxD, TXD_SIZE);
	pTxD = (TXD_STRUC *)&tx_hw_info[0];
#else
	pTxD  = (TXD_STRUC *)pRing->Cell[SwIdx].AllocVa;
#endif
#ifdef RT_BIG_ENDIAN
	/* RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD); */
#endif /* RT_BIG_ENDIAN */
	pRing->Cell[SwIdx].pNdisPacket = net_pkt;
	pRing->Cell[SwIdx].pNextNdisPacket = NULL;
	pRing->Cell[SwIdx].PacketPa = PCI_MAP_SINGLE(pAd, (pSrcBufVA),
								  (SrcBufLen), 0, RTMP_PCI_DMA_TODEVICE);
	pTxD->LastSec0 = 1;
	pTxD->LastSec1 = 0;
	pTxD->SDLen0 = SrcBufLen;
	pTxD->SDLen1 = 0;
	pTxD->SDPtr0 = pRing->Cell[SwIdx].PacketPa;
	pTxD->Burst = 0;
	pTxD->DMADONE = 0;
#ifdef RT_BIG_ENDIAN
	RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
	WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, FALSE, TYPE_TXD);
#endif
	/* flush dcache if no consistent memory is supported */
	RTMP_DCACHE_FLUSH(pRing->Cell[SwIdx].PacketPa, SrcBufLen);
	RTMP_DCACHE_FLUSH(pRing->Cell[SwIdx].AllocPa, TXD_SIZE);
	/* Increase TX_CTX_IDX, but write to register later.*/
	INC_RING_INDEX(pRing->TxCpuIdx, CTL_RING_SIZE);

	if (IS_CMD_MSG_NEED_SYNC_WITH_FW_FLAG_SET(msg))
		AndesQueueTailCmdMsg(&ctl->ackq, msg, wait_ack);
	else
		AndesQueueTailCmdMsg(&ctl->tx_doneq, msg, tx_done);

	if (!OS_TEST_BIT(MCU_INIT, &ctl->flags)) {
		RTMP_SPIN_UNLOCK_IRQRESTORE(&pRing->RingLock, &flags);
		return -1;
	}

	HIF_IO_WRITE32(pAd, pRing->hw_cidx_addr, pRing->TxCpuIdx);
	RTMP_SPIN_UNLOCK_IRQRESTORE(&pRing->RingLock, &flags);
	return ret;
}
#endif /* defined(MT7615) || defined(MT7622) || defined(P18) || defined(MT7663) */
#endif /* defined(RTMP_PCI_SUPPORT) || defined(RTMP_RBUS_SUPPORT) */

static VOID EventExtCmdResult(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	struct _EVENT_EXT_CMD_RESULT_T *EventExtCmdResult =
		(struct _EVENT_EXT_CMD_RESULT_T *)Data;
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("%s: EventExtCmdResult.ucExTenCID = 0x%x\n",
			  __func__, EventExtCmdResult->ucExTenCID));
	EventExtCmdResult->u4Status = le2cpu32(EventExtCmdResult->u4Status);
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("%s: EventExtCmdResult.u4Status = 0x%x\n",
			  __func__, EventExtCmdResult->u4Status));
}


#if defined(MT7636) && defined(TXBF_SUPPORT)
INT32 CmdETxBfSoundingPeriodicTriggerCtrl(
	RTMP_ADAPTER *pAd,
	UCHAR			SndgEn,
	UCHAR			SndgBW,
	UCHAR			NDPAMcs,
	UINT32          u4SNDPeriod,
	UINT32          wlanidx)
{
	struct cmd_msg *msg;
	struct _EXT_CMD_ETXBf_SND_PERIODIC_TRIGGER_CTRL_T ETxBfSdPeriodicTriggerCtrl;
	INT32 ret = 0;
	struct _CMD_ATTRIBUTE attr = {0};
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("%s: Sounding trigger enable = %d\n", __func__, SndgEn));
	msg = AndesAllocCmdMsg(pAd, sizeof(ETxBfSdPeriodicTriggerCtrl));

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	os_zero_mem(&ETxBfSdPeriodicTriggerCtrl, sizeof(ETxBfSdPeriodicTriggerCtrl));
	ETxBfSdPeriodicTriggerCtrl.ucWlanIdx        = (wlanidx);
	ETxBfSdPeriodicTriggerCtrl.ucWMMIdx         = (1);
	ETxBfSdPeriodicTriggerCtrl.ucBW             = (SndgBW);
	ETxBfSdPeriodicTriggerCtrl.u2NDPARateCode   = cpu2le16(NDPAMcs);
	ETxBfSdPeriodicTriggerCtrl.u2NDPRateCode    = cpu2le16(8);      /* MCS8 */
	ETxBfSdPeriodicTriggerCtrl.u4SoundingInterval = cpu2le32(u4SNDPeriod);
	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr,
						  ((SndgEn) ? EXT_CMD_BF_SOUNDING_START : EXT_CMD_BF_SOUNDING_STOP));
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET_AND_RSP);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 8);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, EventExtCmdResult);
	AndesInitCmdMsg(msg, attr);
	AndesAppendCmdMsg(msg, (char *)&ETxBfSdPeriodicTriggerCtrl,
					  sizeof(ETxBfSdPeriodicTriggerCtrl));
	ret = AndesSendCmdMsg(pAd, msg);
error:
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("%s:(ret = %d)\n", __func__, ret));
	return ret;
}
#endif /* MT7636 && TXBF_SUPPORT */


#ifdef UNIFY_FW_CMD
static VOID AndesMTFillTxDHeader(struct cmd_msg *msg, PNDIS_PACKET net_pkt)
{
	TMAC_TXD_L *txd;
	UCHAR *tmac_info;
	tmac_info = (UCHAR *)OS_PKT_HEAD_BUF_EXTEND(net_pkt, sizeof(TMAC_TXD_L));
	txd = (TMAC_TXD_L *)tmac_info;
	NdisZeroMemory(txd, sizeof(TMAC_TXD_L));
	txd->TxD0.TxByteCount = GET_OS_PKT_LEN(net_pkt);
	txd->TxD0.p_idx = (msg->pq_id & 0x8000) >> 15;
	txd->TxD0.q_idx = (msg->pq_id & 0x7c00) >> 10;
	txd->TxD1.ft = 0x1;
	txd->TxD1.hdr_format = 0x1;

	if (msg->attr.type == MT_FW_SCATTER)
		txd->TxD1.pkt_ft = TMI_PKT_FT_HIF_FW;
	else
		txd->TxD1.pkt_ft = TMI_PKT_FT_HIF_CMD;

#ifdef RT_BIG_ENDIAN
	MTMacInfoEndianChange(NULL, (UCHAR *)tmac_info, TYPE_TMACINFO, sizeof(TMAC_TXD_L));
#endif
	return;
}
#endif /* UNIFY_FW_CMD */

/* old chip (before mt7615) use this compact format for cmd header */
VOID AndesMTFillCmdHeader(struct cmd_msg *msg, VOID *pkt)
{
	FW_TXD *fw_txd = NULL;
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)msg->priv;
	struct MCU_CTRL *Ctl = &pAd->MCUCtrl;
	PNDIS_PACKET net_pkt = (PNDIS_PACKET) pkt;

	if (Ctl->fwdl_ctrl.stage == FWDL_STAGE_FW_RUNNING)
		fw_txd = (FW_TXD *)OS_PKT_HEAD_BUF_EXTEND(net_pkt, sizeof(*fw_txd));
	else
		fw_txd = (FW_TXD *)OS_PKT_HEAD_BUF_EXTEND(net_pkt, 12);

	fw_txd->fw_txd_0.field.length = GET_OS_PKT_LEN(net_pkt);
	fw_txd->fw_txd_0.field.pq_id = msg->pq_id;
	fw_txd->fw_txd_1.field.cid = msg->attr.type;
	fw_txd->fw_txd_1.field.pkt_type_id = PKT_ID_CMD;
	fw_txd->fw_txd_1.field.set_query = IS_CMD_MSG_NA_FLAG_SET(msg) ?
									   CMD_NA : IS_CMD_MSG_SET_QUERY_FLAG_SET(msg);
	fw_txd->fw_txd_1.field.seq_num = msg->seq;
	fw_txd->fw_txd_2.field.ext_cid = msg->attr.ext_type;

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s: fw_txd: 0x%x 0x%x 0x%x, Length=%d\n", __func__,
			  fw_txd->fw_txd_0.word, fw_txd->fw_txd_1.word,
			  fw_txd->fw_txd_2.word, fw_txd->fw_txd_0.field.length));

	if ((IS_EXT_CMD_AND_SET_NEED_RSP(msg)) && !(IS_CMD_MSG_NA_FLAG_SET(msg)))
		fw_txd->fw_txd_2.field.ext_cid_option = EXT_CID_OPTION_NEED_ACK;
	else
		fw_txd->fw_txd_2.field.ext_cid_option = EXT_CID_OPTION_NO_NEED_ACK;

	fw_txd->fw_txd_0.word = cpu2le32(fw_txd->fw_txd_0.word);
	fw_txd->fw_txd_1.word = cpu2le32(fw_txd->fw_txd_1.word);
	fw_txd->fw_txd_2.word = cpu2le32(fw_txd->fw_txd_2.word);
#ifdef CONFIG_TRACE_SUPPORT
	TRACE_MCU_CMD_INFO(fw_txd->fw_txd_0.field.length,
					   fw_txd->fw_txd_0.field.pq_id, fw_txd->fw_txd_1.field.cid,
					   fw_txd->fw_txd_1.field.pkt_type_id, fw_txd->fw_txd_1.field.set_query,
					   fw_txd->fw_txd_1.field.seq_num, fw_txd->fw_txd_2.field.ext_cid,
					   fw_txd->fw_txd_2.field.ext_cid_option,
					   (char *)(GET_OS_PKT_DATAPTR(net_pkt)), GET_OS_PKT_LEN(net_pkt));
#endif /* CONFIG_TRACE_SUPPORT */
}

/* unify cmd header since mt7615 */
VOID AndesMTFillCmdHeaderWithTXD(struct cmd_msg *msg, VOID *pkt)
{
	FW_TXD *fw_txd = NULL;
	PNDIS_PACKET net_pkt = (PNDIS_PACKET) pkt;

	fw_txd = (FW_TXD *)OS_PKT_HEAD_BUF_EXTEND(net_pkt, sizeof(FW_TXD));
	AndesMTFillTxDHeader(msg, net_pkt);
	NdisZeroMemory(fw_txd, sizeof(FW_TXD));

	fw_txd->fw_txd_0.field.length =	GET_OS_PKT_LEN(net_pkt) - sizeof(TMAC_TXD_L);
	fw_txd->fw_txd_0.field.pq_id = msg->pq_id;
	fw_txd->fw_txd_1.field.cid = msg->attr.type;
	fw_txd->fw_txd_1.field.pkt_type_id = PKT_ID_CMD;
	fw_txd->fw_txd_1.field.set_query = IS_CMD_MSG_NA_FLAG_SET(msg) ?
									   CMD_NA : IS_CMD_MSG_SET_QUERY_FLAG_SET(msg);
	fw_txd->fw_txd_1.field.seq_num = msg->seq;
	fw_txd->fw_txd_2.field.ext_cid = msg->attr.ext_type;

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s: mcu_dest(%d):%s\n", __func__, msg->attr.mcu_dest,
			  (msg->attr.mcu_dest == HOST2N9) ? "HOST2N9" : "HOST2CR4"));

	if (msg->attr.mcu_dest == HOST2N9)
		fw_txd->fw_txd_2.field.ucS2DIndex = HOST2N9;
	else
		fw_txd->fw_txd_2.field.ucS2DIndex = HOST2CR4;

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s: fw_txd: 0x%x 0x%x 0x%x, Length=%d\n", __func__,
			  fw_txd->fw_txd_0.word, fw_txd->fw_txd_1.word,
			  fw_txd->fw_txd_2.word, fw_txd->fw_txd_0.field.length));

	if ((IS_EXT_CMD_AND_SET_NEED_RSP(msg)) && !(IS_CMD_MSG_NA_FLAG_SET(msg)))
		fw_txd->fw_txd_2.field.ext_cid_option = EXT_CID_OPTION_NEED_ACK;
	else
		fw_txd->fw_txd_2.field.ext_cid_option = EXT_CID_OPTION_NO_NEED_ACK;

	fw_txd->fw_txd_0.word = cpu2le32(fw_txd->fw_txd_0.word);
	fw_txd->fw_txd_1.word = cpu2le32(fw_txd->fw_txd_1.word);
	fw_txd->fw_txd_2.word = cpu2le32(fw_txd->fw_txd_2.word);
#ifdef CONFIG_TRACE_SUPPORT
	TRACE_MCU_CMD_INFO(fw_txd->fw_txd_0.field.length,
					   fw_txd->fw_txd_0.field.pq_id, fw_txd->fw_txd_1.field.cid,
					   fw_txd->fw_txd_1.field.pkt_type_id, fw_txd->fw_txd_1.field.set_query,
					   fw_txd->fw_txd_1.field.seq_num, fw_txd->fw_txd_2.field.ext_cid,
					   fw_txd->fw_txd_2.field.ext_cid_option,
					   (char *)(GET_OS_PKT_DATAPTR(net_pkt)), GET_OS_PKT_LEN(net_pkt));
#endif /* CONFIG_TRACE_SUPPORT */
}

static VOID EventChPrivilegeHandler(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	struct MCU_CTRL *ctl = &pAd->MCUCtrl;
	UINT32 Value;

	if (IS_MT7603(pAd) || IS_MT7628(pAd)  || IS_MT76x6(pAd) || IS_MT7637(pAd)) {
		RTMP_IO_READ32(pAd, RMAC_RMCR, &Value);

		if (ctl->RxStream0 == 1)
			Value |= RMAC_RMCR_RX_STREAM_0;
		else
			Value &= ~RMAC_RMCR_RX_STREAM_0;

		if (ctl->RxStream1 == 1)
			Value |= RMAC_RMCR_RX_STREAM_1;
		else
			Value &= ~RMAC_RMCR_RX_STREAM_1;

		RTMP_IO_WRITE32(pAd, RMAC_RMCR, Value);
	}

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s\n", __func__));
}


static VOID ExtEventBeaconLostHandler(RTMP_ADAPTER *pAd,
									  UINT8 *Data, UINT32 Length)
{
	struct _EXT_EVENT_BEACON_LOSS_T *pExtEventBeaconLoss =
		(struct _EXT_EVENT_BEACON_LOSS_T *)Data;
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s::FW LOG, Beacon lost (%02x:%02x:%02x:%02x:%02x:%02x), Reason 0x%x\n", __func__,
			  pExtEventBeaconLoss->aucBssid[0],
			  pExtEventBeaconLoss->aucBssid[1],
			  pExtEventBeaconLoss->aucBssid[2],
			  pExtEventBeaconLoss->aucBssid[3],
			  pExtEventBeaconLoss->aucBssid[4],
			  pExtEventBeaconLoss->aucBssid[5],
			  pExtEventBeaconLoss->ucReason));

	switch (pExtEventBeaconLoss->ucReason) {
#ifdef CONFIG_AP_SUPPORT

	case ENUM_BCN_LOSS_AP_DISABLE:
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("  Beacon lost - AP disabled!!!\n"));
		break;

	case ENUM_BCN_LOSS_AP_SER_TRIGGER:
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("  Beacon lost - SER happened!!!\n"));
		break;

	case ENUM_BCN_LOSS_AP_ERROR:
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("  Beacon lost - Error!!! Re-issue BCN_OFFLOAD cmd\n"));
		/* update Beacon again if operating in AP mode. */
		UpdateBeaconHandler(pAd, NULL, BCN_UPDATE_AP_RENEW);
		break;
#endif

	default:
		break;
	}
}

#ifdef MT_DFS_SUPPORT /* Jelly20140123 */
static VOID ExtEventRddReportHandler(RTMP_ADAPTER *pAd,
									 UINT8 *Data, UINT32 Length)
{
	struct _EXT_EVENT_RDD_REPORT_T *pExtEventRddReport =
		(struct _EXT_EVENT_RDD_REPORT_T *)Data;
	UCHAR ucRddIdx = pExtEventRddReport->ucRddIdx;
	WrapDfsRddReportHandle(pAd, ucRddIdx);
}

#endif

#ifdef WIFI_SPECTRUM_SUPPORT
/*
	==========================================================================
	Description:
	Unsolicited extend event handler of wifi-spectrum.
	Return:
	==========================================================================
*/
static VOID ExtEventWifiSpectrumHandler(
	IN RTMP_ADAPTER *pAd,
	IN UINT8 *pData,
	IN UINT32 Length)
{
	EXT_EVENT_SPECTRUM_RESULT_T *pResult = (EXT_EVENT_SPECTRUM_RESULT_T *)pData;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("%s----------------->\n", __func__));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s: FuncIndex = %d\n", __func__, pResult->u4FuncIndex));

	switch (pResult->u4FuncIndex) {
	case SPECTRUM_CTRL_FUNCID_DUMP_RAW_DATA:
		RTEnqueueInternalCmd(pAd, CMDTHRED_WIFISPECTRUM_DUMP_RAW_DATA, (VOID *)pData, Length);
		break;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("%s<-----------------\n", __func__));

	return;
}

/*
	==========================================================================
	Description:
	Cmd queue raw data handler of wifi-spectrum.
	Return:
	==========================================================================
*/
NTSTATUS WifiSpectrumRawDataHandler(
	IN RTMP_ADAPTER *pAd,
	IN PCmdQElmt CMDQelmt)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (ops->SpectrumEventRawDataHandler != NULL) {
		ops->SpectrumEventRawDataHandler(pAd, CMDQelmt->buffer, CMDQelmt->bufferlength);
		return NDIS_STATUS_SUCCESS;
	} else
		return NDIS_STATUS_FAILURE;
}

/*
	==========================================================================
	Description:
	Extend event raw data handler of wifi-spectrum.
	Return:
	==========================================================================
*/
VOID ExtEventWifiSpectrumRawDataHandler(
	IN RTMP_ADAPTER *pAd,
	IN UINT8 *pData,
	IN UINT32 Length)
{
	UINT32 i, CapNode, Data;
	UINT8 msg_IQ[CAP_FILE_MSG_LEN], msg_Gain[CAP_FILE_MSG_LEN];
	INT32 retval, Status;
	INT16 I_0, Q_0, LNA, LPF;
	RTMP_OS_FS_INFO osFSInfo;
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 BankNum = pChipCap->SpectrumBankNum;
	RBIST_DESC_T *pSpectrumDesc = &pChipCap->pSpectrumDesc[0];
	EXT_EVENT_RBIST_DUMP_RAW_DATA_T *pSpectrumEvent = NULL;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("%s----------------->\n", __func__));

	/* Update pSpectrumEvent */
	pSpectrumEvent = (EXT_EVENT_RBIST_DUMP_RAW_DATA_T *)pData;
	pSpectrumEvent->u4FuncIndex = le2cpu32(pSpectrumEvent->u4FuncIndex);
	pSpectrumEvent->u4PktNum = le2cpu32(pSpectrumEvent->u4PktNum);

	/* If file is closed, we need to drop this packet. */
	if ((pAd->pSrcf_IQ == NULL) || (pAd->pSrcf_Gain == NULL)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("\x1b[31m%s: File is already closed!!\x1b[m\n", __func__));
		return;
	}

	/* If we receive the packet which is delivered from last time data-capure, we need to drop it.*/
	if (pSpectrumEvent->u4PktNum > pAd->SpectrumEventCnt) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("\x1b[31m%s: Packet out of order: Pkt num %d, EventCnt %d\x1b[m\n",
				__func__, pSpectrumEvent->u4PktNum, pAd->SpectrumEventCnt));
		return;
	}

	/* Change limits of authority in order to read/write file */
	RtmpOSFSInfoChange(&osFSInfo, TRUE);

	/* Dump I/Q/LNA/LPF data to file */
	for (i = 0; i < SPECTRUM_EVENT_DATA_SAMPLE; i++) {
		Data = le2cpu32(pSpectrumEvent->u4Data[i]);
		os_zero_mem(msg_IQ, CAP_FILE_MSG_LEN);
		os_zero_mem(msg_Gain, CAP_FILE_MSG_LEN);
		/* Parse I/Q/LNA/LPF data and dump these data to file */
		CapNode = Get_System_CapNode_Info(pAd);
		if ((CapNode == pChipCap->SpectrumWF0ADC) || (CapNode == pChipCap->SpectrumWF1ADC)
			|| (CapNode == pChipCap->SpectrumWF2ADC) || (CapNode == pChipCap->SpectrumWF3ADC)) { /* Dump 1-way RXADC */
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					("%s : Dump 1-way RXADC\n", __func__));

			if (pSpectrumDesc->ucADCRes == 10) {
				/* Parse and dump I/Q data */
				Q_0 = (Data & 0x3FF);
				I_0 = ((Data & (0x3FF << 10)) >> 10);

				if (Q_0 >= 512)
					Q_0 -= 1024;

				if (I_0 >= 512)
					I_0 -= 1024;

				sprintf(msg_IQ, "%+04d\t%+04d\n", I_0, Q_0);
				retval = RtmpOSFileWrite(pAd->pSrcf_IQ, (RTMP_STRING *)msg_IQ, strlen(msg_IQ));
				/* Parse and dump LNA/LPF data */
				LNA = ((Data & (0x3 << 28)) >> 28);
				LPF = ((Data & (0xF << 24)) >> 24);
				sprintf(msg_Gain, "%+04d\t%+04d\n", LNA, LPF);
				retval = RtmpOSFileWrite(pAd->pSrcf_Gain, (RTMP_STRING *)msg_Gain, strlen(msg_Gain));
			}
		} else if ((CapNode == pChipCap->SpectrumWF0FIIQ) || (CapNode == pChipCap->SpectrumWF1FIIQ)
				   || (CapNode == pChipCap->SpectrumWF2FIIQ) || (CapNode == pChipCap->SpectrumWF3FIIQ)
				   || (CapNode == pChipCap->SpectrumWF0FDIQ) || (CapNode == pChipCap->SpectrumWF1FDIQ)
				   || (CapNode == pChipCap->SpectrumWF2FDIQ) || (CapNode == pChipCap->SpectrumWF3FDIQ)) { /* Dump 1-way RXIQC */
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					("%s : Dump 1-way RXIQC\n", __func__));

			if (pSpectrumDesc->ucIQCRes == 12) {
				/* Parse and dump I/Q data */
				Q_0 = (Data & 0xFFF);
				I_0 = ((Data & (0xFFF << 12)) >> 12);

				if (Q_0 >= 2048)
					Q_0 -= 4096;

				if (I_0 >= 2048)
					I_0 -= 4096;

				sprintf(msg_IQ, "%+05d\t%+05d\n", I_0, Q_0);
				retval = RtmpOSFileWrite(pAd->pSrcf_IQ, (RTMP_STRING *)msg_IQ, strlen(msg_IQ));
				/* Parse and dump LNA/LPF data */
				LNA = ((Data & (0x3 << 28)) >> 28);
				LPF = ((Data & (0xF << 24)) >> 24);
				sprintf(msg_Gain, "%+04d\t%+04d\n", LNA, LPF);
				retval = RtmpOSFileWrite(pAd->pSrcf_Gain, (RTMP_STRING *)msg_Gain, strlen(msg_Gain));
			}
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("%s : 0x%08x\n", __func__, Data));
	}

	/* Change limits of authority in order to read/write file */
	RtmpOSFSInfoChange(&osFSInfo, FALSE);
	/* Update SpectrumEventCnt */
	pAd->SpectrumEventCnt++;

	if (IS_MT7615(pAd)) {
		/* Check whether is the last FW event of whole data query or not */
		if (pAd->SpectrumEventCnt == MT7615_SPECTRUM_TOTAL_SIZE) {
			/* Reset SpectrumEventCnt */
			pAd->SpectrumEventCnt = 0;
			/* Update SpectrumStatus */
			pAd->SpectrumStatus = CAP_SUCCESS;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("\x1b[31m%s: Dump 128K done !! \x1b[m\n", __func__));
			RTMP_OS_COMPLETE(&pAd->SpectrumDumpDataDone);
		}
	} else {
		/* Update pSpectrumDesc */
		pSpectrumDesc = &pChipCap->pSpectrumDesc[pAd->SpectrumIdx];

		/* Check whether is the last FW event of data query in the same bank */
		if (pAd->SpectrumEventCnt == pSpectrumDesc->u4BankSize) {
			/* Check whether is the last bank or not */
			if ((pAd->SpectrumIdx + 1) == BankNum) {
				/* Print log to console to indicate data process done */
				{
					UINT32 TotalSize = 0;

					for (i = 0; i < BankNum; i++) {
						/* Update pSpectrumDesc */
						pSpectrumDesc = &pChipCap->pSpectrumDesc[i];
						/* Calculate total size  */
						TotalSize = TotalSize + pSpectrumDesc->u4BankSize;
					}

					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
							("\x1b[31m%s: Dump %d K done !! \x1b[m\n", __func__, TotalSize));
				}
				/* Update status */
				pAd->SpectrumStatus = CAP_SUCCESS;
			}

			/* Reset SpectrumEventCnt */
			pAd->SpectrumEventCnt = 0;
			/* OS wait for completion done */
			RTMP_OS_COMPLETE(&pAd->SpectrumDumpDataDone);
		}
	}

	/* Update status */
	Status = pAd->SpectrumStatus;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s:(Status = %d)\n", __func__, Status));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("%s<-----------------\n", __func__));

	return;
}
#endif /* WIFI_SPECTRUM_SUPPORT */

#ifdef INTERNAL_CAPTURE_SUPPORT
/*
	==========================================================================
	Description:
	Cmd queue raw data handler of ICAP.
	Return:
	==========================================================================
*/
NTSTATUS ICapRawDataHandler(
	IN RTMP_ADAPTER *pAd,
	IN PCmdQElmt CMDQelmt)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (ops->ICapEventRawDataHandler != NULL) {
		ops->ICapEventRawDataHandler(pAd, CMDQelmt->buffer, CMDQelmt->bufferlength);
		return NDIS_STATUS_SUCCESS;
	} else
		return NDIS_STATUS_FAILURE;
}

/*
	==========================================================================
	Description:
	Extend event of 96-bit raw data parser of ICAP.
	Return:
	==========================================================================
*/
VOID ExtEventICap96BitDataParser(
	IN RTMP_ADAPTER *pAd)
{
	INT32 retval, Status;
	UINT32 i, j, StopPoint, CapNode;
	BOOLEAN Wrap;
	PUINT32 pTemp_L32Bit = NULL, pTemp_M32Bit = NULL, pTemp_H32Bit = NULL;
	RTMP_REG_PAIR RegStartAddr, RegStopAddr, RegWrap;
	P_RBIST_IQ_DATA_T pIQ_Array = NULL;
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);
	RBIST_DESC_T *pICapDesc = &pChipCap->pICapDesc[0];
	UINT8 BankNum = pChipCap->ICapBankNum;
	UINT32 BankSmplCnt = pChipCap->ICapBankSmplCnt;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("%s----------------->\n", __func__));

	/* Get RBIST start address */
	RegStartAddr.Register = RBISTCR2;
	MtCmdMultipleMacRegAccessRead(pAd, &RegStartAddr, 1);
	/* Get RBIST stop address */
	RegStopAddr.Register = RBISTCR9;
	MtCmdMultipleMacRegAccessRead(pAd, &RegStopAddr, 1);
	/* Calculate stop point */
	StopPoint = (RegStopAddr.Value - RegStartAddr.Value) / 4;
	/* Get RBIST wrapper */
	RegWrap.Register = RBISTCR0;
	MtCmdMultipleMacRegAccessRead(pAd, &RegWrap, 1);
	Wrap = ((RegWrap.Value & BIT(ICAP_WRAP)) >> ICAP_WRAP);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("\x1b[42m RBISTCR2: 0x%08x, RBISTCR9: 0x%08x, Wrap: %d \x1b[m\n",
			RegStartAddr.Value, RegStopAddr.Value, Wrap));

	/* Re-arrange each buffer by stop address and wrapper */
	if (!Wrap) {
		UINT32 Len, Offset;

		Len = ((BankNum / 3) * BankSmplCnt - StopPoint - 1) * sizeof(UINT32);
		Offset = StopPoint + 1;
		/* Set the rest of redundant data to zero */
		os_zero_mem((pAd->pL32Bit + Offset), Len);
		os_zero_mem((pAd->pM32Bit + Offset), Len);
		os_zero_mem((pAd->pH32Bit + Offset), Len);
	} else {
		UINT32 Len, Offset;

		/* Dynamic allocate memory for pTemp_L32Bit */
		Len = (BankNum / 3) * BankSmplCnt * sizeof(UINT32);
		retval = os_alloc_mem(pAd, (UCHAR **)&pTemp_L32Bit, Len);
		if (retval != NDIS_STATUS_SUCCESS) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("%s : Not enough memory for dynamic allocating !!\n", __func__));
			goto error;
		}

		/* Dynamic allocate memory for pTemp_M32Bit */
		retval = os_alloc_mem(pAd, (UCHAR **)&pTemp_M32Bit, Len);
		if (retval != NDIS_STATUS_SUCCESS) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("%s : Not enough memory for dynamic allocating !!\n", __func__));
			goto error;
		}

		/* Dynamic allocate memory for pTemp_H32Bit */
		retval = os_alloc_mem(pAd, (UCHAR **)&pTemp_H32Bit, Len);
		if (retval != NDIS_STATUS_SUCCESS) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("%s : Not enough memory for dynamic allocating !!\n", __func__));
			goto error;
		}

		/* Initialization of data buffer of Temp_L32Bit/Temp_M32Bit/Temp_H32Bit */
		os_zero_mem(pTemp_L32Bit, Len);
		os_zero_mem(pTemp_M32Bit, Len);
		os_zero_mem(pTemp_H32Bit, Len);
		os_move_mem(pTemp_L32Bit, pAd->pL32Bit, Len);
		os_move_mem(pTemp_M32Bit, pAd->pM32Bit, Len);
		os_move_mem(pTemp_H32Bit, pAd->pH32Bit, Len);

		for (i = 0; i < (Len / sizeof(UINT32)); i++) {
			/* Re-arrange data buffer of L32Bit/M32Bit/H32Bit */
			Offset = (StopPoint + 1 + i) % (Len / sizeof(UINT32));
			*(pAd->pL32Bit + i) = *(pTemp_L32Bit + Offset);
			*(pAd->pM32Bit + i) = *(pTemp_M32Bit + Offset);
			*(pAd->pH32Bit + i) = *(pTemp_H32Bit + Offset);
		}
	}

	/* Reset IQ_Array */
	pIQ_Array = pAd->pIQ_Array;
	os_zero_mem(pIQ_Array, MAX_ICAP_IQ_DATA_CNT * sizeof(RBIST_IQ_DATA_T));

	/* Parse I/Q data and store these data to buffer */
	CapNode = Get_System_CapNode_Info(pAd);
	if (CapNode == pChipCap->ICapFourWayADC) { /* 4-way ADC */
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\x1b[42m4-Way RXADC ------> \x1b[m\n"));

		for (i = 0; i < (pChipCap->ICapADCIQCnt / 3); i++) {
			if (pICapDesc->ucADCRes == 4) {
				/* Parse I/Q data */
				pIQ_Array[3 * i].IQ_Array[CAP_WF0][CAP_Q_TYPE] = ((*(pAd->pL32Bit + i) & (0xF << 8)) >> 8);         /* Parsing Q0 */
				pIQ_Array[3 * i + 1].IQ_Array[CAP_WF0][CAP_Q_TYPE] = ((*(pAd->pL32Bit + i) & (0xF << 4)) >> 4);     /* Parsing Q0 */
				pIQ_Array[3 * i + 2].IQ_Array[CAP_WF0][CAP_Q_TYPE] = (*(pAd->pL32Bit + i) & 0xF);                   /* Parsing Q0 */
				pIQ_Array[3 * i].IQ_Array[CAP_WF0][CAP_I_TYPE] = ((*(pAd->pL32Bit + i) & (0xF << 20)) >> 20);       /* Parsing I0 */
				pIQ_Array[3 * i + 1].IQ_Array[CAP_WF0][CAP_I_TYPE] = ((*(pAd->pL32Bit + i) & (0xF << 16)) >> 16);   /* Parsing I0 */
				pIQ_Array[3 * i + 2].IQ_Array[CAP_WF0][CAP_I_TYPE] = ((*(pAd->pL32Bit + i) & (0xF << 12)) >> 12);   /* Parsing I0 */
				pIQ_Array[3 * i].IQ_Array[CAP_WF1][CAP_Q_TYPE] = (*(pAd->pM32Bit + i) & 0xF);                       /* Parsing Q1 */
				pIQ_Array[3 * i + 1].IQ_Array[CAP_WF1][CAP_Q_TYPE] = ((*(pAd->pL32Bit + i) & (0xF << 28)) >> 28);   /* Parsing Q1 */
				pIQ_Array[3 * i + 2].IQ_Array[CAP_WF1][CAP_Q_TYPE] = ((*(pAd->pL32Bit + i) & (0xF << 24)) >> 24);   /* Parsing Q1 */
				pIQ_Array[3 * i].IQ_Array[CAP_WF1][CAP_I_TYPE] = ((*(pAd->pM32Bit + i) & (0xF << 12)) >> 12);       /* Parsing I1 */
				pIQ_Array[3 * i + 1].IQ_Array[CAP_WF1][CAP_I_TYPE] = ((*(pAd->pM32Bit + i) & (0xF << 8)) >> 8);     /* Parsing I1 */
				pIQ_Array[3 * i + 2].IQ_Array[CAP_WF1][CAP_I_TYPE] = ((*(pAd->pM32Bit + i) & (0xF << 4)) >> 4);     /* Parsing I1 */
				pIQ_Array[3 * i].IQ_Array[CAP_WF2][CAP_Q_TYPE] = ((*(pAd->pM32Bit + i) & (0xF << 24)) >> 24);       /* Parsing Q2 */
				pIQ_Array[3 * i + 1].IQ_Array[CAP_WF2][CAP_Q_TYPE] = ((*(pAd->pM32Bit + i) & (0xF << 20)) >> 20);   /* Parsing Q2 */
				pIQ_Array[3 * i + 2].IQ_Array[CAP_WF2][CAP_Q_TYPE] = ((*(pAd->pM32Bit + i) & (0xF << 16)) >> 16);   /* Parsing Q2 */
				pIQ_Array[3 * i].IQ_Array[CAP_WF2][CAP_I_TYPE] = ((*(pAd->pH32Bit + i) & (0xF << 4)) >> 4);         /* Parsing I2 */
				pIQ_Array[3 * i + 1].IQ_Array[CAP_WF2][CAP_I_TYPE] = (*(pAd->pH32Bit + i) & 0xF);                   /* Parsing I2 */
				pIQ_Array[3 * i + 2].IQ_Array[CAP_WF2][CAP_I_TYPE] = ((*(pAd->pM32Bit + i) & (0xF << 28)) >> 28);   /* Parsing I2 */
				pIQ_Array[3 * i].IQ_Array[CAP_WF3][CAP_Q_TYPE] = ((*(pAd->pH32Bit + i) & (0xF << 16)) >> 16);       /* Parsing Q3 */
				pIQ_Array[3 * i + 1].IQ_Array[CAP_WF3][CAP_Q_TYPE] = ((*(pAd->pH32Bit + i) & (0xF << 12)) >> 12);   /* Parsing Q3 */
				pIQ_Array[3 * i + 2].IQ_Array[CAP_WF3][CAP_Q_TYPE] = ((*(pAd->pH32Bit + i) & (0xF << 8)) >> 8);     /* Parsing Q3 */
				pIQ_Array[3 * i].IQ_Array[CAP_WF3][CAP_I_TYPE] = ((*(pAd->pH32Bit + i) & (0xF << 28)) >> 28);       /* Parsing I3 */
				pIQ_Array[3 * i + 1].IQ_Array[CAP_WF3][CAP_I_TYPE] = ((*(pAd->pH32Bit + i) & (0xF << 24)) >> 24);   /* Parsing I3 */
				pIQ_Array[3 * i + 2].IQ_Array[CAP_WF3][CAP_I_TYPE] = ((*(pAd->pH32Bit + i) & (0xF << 20)) >> 20);   /* Parsing I3 */

				/* Calculation of two-complement to decimal */
				for (j = 0; j < 3; j++) {
					pIQ_Array[3 * i + j].IQ_Array[CAP_WF0][CAP_Q_TYPE] -= 8;
					pIQ_Array[3 * i + j].IQ_Array[CAP_WF0][CAP_I_TYPE] -= 8;
					pIQ_Array[3 * i + j].IQ_Array[CAP_WF1][CAP_Q_TYPE] -= 8;
					pIQ_Array[3 * i + j].IQ_Array[CAP_WF1][CAP_I_TYPE] -= 8;
					pIQ_Array[3 * i + j].IQ_Array[CAP_WF2][CAP_Q_TYPE] -= 8;
					pIQ_Array[3 * i + j].IQ_Array[CAP_WF2][CAP_I_TYPE] -= 8;
					pIQ_Array[3 * i + j].IQ_Array[CAP_WF3][CAP_Q_TYPE] -= 8;
					pIQ_Array[3 * i + j].IQ_Array[CAP_WF3][CAP_I_TYPE] -= 8;
				}
			}
		}
	} else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\x1b[42m 4-Way IQC ------>  \x1b[m\n"));

		for (i = 0; i < pChipCap->ICapIQCIQCnt; i++) {
			if (pICapDesc->ucIQCRes == 12) {
				/* Parse I/Q data */
				pIQ_Array[i].IQ_Array[CAP_WF0][CAP_Q_TYPE] = (*(pAd->pL32Bit + i) & 0xFFF);                  /* Parsing Q0 */
				pIQ_Array[i].IQ_Array[CAP_WF0][CAP_I_TYPE] = ((*(pAd->pL32Bit + i) & (0xFFF << 12)) >> 12);  /* Parsing I0 */
				pIQ_Array[i].IQ_Array[CAP_WF1][CAP_Q_TYPE] = ((*(pAd->pL32Bit + i) & (0xFF << 24)) >> 24);   /* Parsing Q1 */
				pIQ_Array[i].IQ_Array[CAP_WF1][CAP_Q_TYPE] |= ((*(pAd->pM32Bit + i) & 0xF) << 8);		     /* Parsing Q1 */
				pIQ_Array[i].IQ_Array[CAP_WF1][CAP_I_TYPE] = ((*(pAd->pM32Bit + i) & (0xFFF << 4)) >> 4);    /* Parsing I1 */
				pIQ_Array[i].IQ_Array[CAP_WF2][CAP_Q_TYPE] = ((*(pAd->pM32Bit + i) & (0xFFF << 16)) >> 16);  /* Parsing Q2 */
				pIQ_Array[i].IQ_Array[CAP_WF2][CAP_I_TYPE] = ((*(pAd->pM32Bit + i) & (0xF << 28)) >> 28);    /* Parsing I2 */
				pIQ_Array[i].IQ_Array[CAP_WF2][CAP_I_TYPE] |= ((*(pAd->pH32Bit + i) & 0xFF) << 4);           /* Parsing I2 */
				pIQ_Array[i].IQ_Array[CAP_WF3][CAP_Q_TYPE] = ((*(pAd->pH32Bit + i) & (0xFFF << 8)) >> 8);    /* Parsing Q3 */
				pIQ_Array[i].IQ_Array[CAP_WF3][CAP_I_TYPE] = ((*(pAd->pH32Bit + i) & (0xFFF << 20)) >> 20);  /* Parsing I3 */

				/* Calculation of two-complement to decimal */
				if (pIQ_Array[i].IQ_Array[CAP_WF0][CAP_Q_TYPE] >= 2048)
					pIQ_Array[i].IQ_Array[CAP_WF0][CAP_Q_TYPE] -= 4096;

				if (pIQ_Array[i].IQ_Array[CAP_WF0][CAP_I_TYPE] >= 2048)
					pIQ_Array[i].IQ_Array[CAP_WF0][CAP_I_TYPE] -= 4096;

				if (pIQ_Array[i].IQ_Array[CAP_WF1][CAP_Q_TYPE] >= 2048)
					pIQ_Array[i].IQ_Array[CAP_WF1][CAP_Q_TYPE] -= 4096;

				if (pIQ_Array[i].IQ_Array[CAP_WF1][CAP_I_TYPE] >= 2048)
					pIQ_Array[i].IQ_Array[CAP_WF1][CAP_I_TYPE] -= 4096;

				if (pIQ_Array[i].IQ_Array[CAP_WF2][CAP_Q_TYPE] >= 2048)
					pIQ_Array[i].IQ_Array[CAP_WF2][CAP_Q_TYPE] -= 4096;

				if (pIQ_Array[i].IQ_Array[CAP_WF2][CAP_I_TYPE] >= 2048)
					pIQ_Array[i].IQ_Array[CAP_WF2][CAP_I_TYPE] -= 4096;

				if (pIQ_Array[i].IQ_Array[CAP_WF3][CAP_Q_TYPE] >= 2048)
					pIQ_Array[i].IQ_Array[CAP_WF3][CAP_Q_TYPE] -= 4096;

				if (pIQ_Array[i].IQ_Array[CAP_WF3][CAP_I_TYPE] >= 2048)
					pIQ_Array[i].IQ_Array[CAP_WF3][CAP_I_TYPE] -= 4096;
			}
		}
	}

	/* Print log to console to indicate data process done */
	{
		UINT32 TotalSize = 0;

		for (i = 0; i < BankNum; i++) {
			/* Update pICapDesc */
			pICapDesc = &pChipCap->pICapDesc[i];
			/* Calculate total size */
			TotalSize = TotalSize + pICapDesc->u4BankSize;
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\x1b[31m%s: Dump %d K done !! \x1b[m\n", __func__, TotalSize));
	}
	/* Update ICap overall status */
	pAd->ICapStatus = CAP_SUCCESS;

error:
	if (pTemp_L32Bit != NULL)
		os_free_mem(pTemp_L32Bit);

	if (pTemp_M32Bit != NULL)
		os_free_mem(pTemp_M32Bit);

	if (pTemp_H32Bit != NULL)
		os_free_mem(pTemp_H32Bit);

	/* Update status */
	Status = pAd->ICapStatus;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s:(Status = %d)\n", __func__, Status));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("%s<-----------------\n", __func__));

	return;
}

/*
	==========================================================================
	Description:
	Extend event 96-bit raw data handler of ICAP.
	Return:
	==========================================================================
*/
VOID ExtEventICap96BitRawDataHandler(
	IN RTMP_ADAPTER *pAd,
	IN UINT8 *pData,
	IN UINT32 Length)
{
	INT32 retval, Status;
	UINT32 i, j;
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);
	RBIST_DESC_T *pICapDesc = &pChipCap->pICapDesc[0];
	UINT8 BankNum = pChipCap->ICapBankNum;
	UINT32 BankSmplCnt = pChipCap->ICapBankSmplCnt;
	EXT_EVENT_RBIST_DUMP_RAW_DATA_T *pICapEvent = NULL;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("%s----------------->\n", __func__));

	/* Update pICapEvent */
	pICapEvent = (EXT_EVENT_RBIST_DUMP_RAW_DATA_T *)pData;
	pICapEvent->u4FuncIndex = le2cpu32(pICapEvent->u4FuncIndex);
	pICapEvent->u4PktNum = le2cpu32(pICapEvent->u4PktNum);
	pICapEvent->u4Bank = le2cpu32(pICapEvent->u4Bank);

	/* If we receive the packet which is delivered from last time data-capure, we need to drop it. */
	if (pICapEvent->u4PktNum > pAd->ICapEventCnt) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("\x1b[31m%s: Packet out of order: Pkt num %d, EventCnt %d\x1b[m\n",
				__func__, pICapEvent->u4PktNum, pAd->ICapEventCnt));
		return;
	}

	/* Dynamic allocate memory for L32Bit/M32Bit/H32Bit buffer */
	{
		UINT32 Len;

		Len = (BankNum / 3) * BankSmplCnt * sizeof(UINT32);
		if (pAd->pL32Bit == NULL) {
			retval = os_alloc_mem(pAd, (UCHAR **)&pAd->pL32Bit, Len);
			if (retval != NDIS_STATUS_SUCCESS) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						("%s : Not enough memory for dynamic allocating !!\n", __func__));
				goto error;
			}
		}

		if (pAd->pM32Bit == NULL) {
			retval = os_alloc_mem(pAd, (UCHAR **)&pAd->pM32Bit, Len);
			if (retval != NDIS_STATUS_SUCCESS) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						("%s : Not enough memory for dynamic allocating !!\n", __func__));
				goto error;
			}
		}

		if (pAd->pH32Bit == NULL) {
			retval = os_alloc_mem(pAd, (UCHAR **)&pAd->pH32Bit, Len);
			if (retval != NDIS_STATUS_SUCCESS) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						("%s : Not enough memory for dynamic allocating !!\n", __func__));
				goto error;
			}
		}

		/* Initialization of data buffer of L32Bit/M32Bit/H32Bit */
		if ((pAd->ICapL32Cnt == 0) && (pAd->ICapM32Cnt == 0)
			&& (pAd->ICapH32Cnt == 0)) {
			os_zero_mem(pAd->pL32Bit, Len);
			os_zero_mem(pAd->pM32Bit, Len);
			os_zero_mem(pAd->pH32Bit, Len);
		}
	}

	/* Store L32Bit, M32Bit, H32Bit data to each buffer */
	for (j = 0; j < (BankNum / 3); j++) {
		if (pICapEvent->u4Bank == pICapDesc->pLBank[j]) {
			for (i = 0; i < ICAP_EVENT_DATA_SAMPLE; i++) {
				pAd->pL32Bit[pAd->ICapL32Cnt] = le2cpu32(pICapEvent->u4Data[i]);
				pAd->ICapL32Cnt++;
			}
		} else if (pICapEvent->u4Bank == pICapDesc->pMBank[j]) {
			for (i = 0; i < ICAP_EVENT_DATA_SAMPLE; i++) {
				pAd->pM32Bit[pAd->ICapM32Cnt] = le2cpu32(pICapEvent->u4Data[i]);
				pAd->ICapM32Cnt++;
			}
		} else if (pICapEvent->u4Bank == pICapDesc->pHBank[j]) {
			for (i = 0; i < ICAP_EVENT_DATA_SAMPLE; i++) {
				pAd->pH32Bit[pAd->ICapH32Cnt] = le2cpu32(pICapEvent->u4Data[i]);
				pAd->ICapH32Cnt++;
			}
		}
	}

	/* Print ICap data to console for debugging purpose */
	for (i = 0; i < ICAP_EVENT_DATA_SAMPLE; i++) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("\x1b[42m 0x%08x\x1b[m\n", le2cpu32(pICapEvent->u4Data[i])));
	}

	/* Update ICapEventCnt */
	pAd->ICapEventCnt++;
	/* Update pICapDesc */
	pICapDesc = &pChipCap->pICapDesc[pAd->ICapIdx];

	/* Check whether is the last FW event or not */
	if (pAd->ICapEventCnt == pICapDesc->u4BankSize) {
		/* Check whether is the last bank or not */
		if ((pAd->ICapIdx + 1) == BankNum)
			ExtEventICap96BitDataParser(pAd);

		/* Reset ICapEventCnt */
		pAd->ICapEventCnt = 0;
		/* OS wait for completion done */
		RTMP_OS_COMPLETE(&pAd->ICapDumpDone);
	}

error:
	/* Update status */
	Status = pAd->ICapStatus;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s:(Status = %d)\n", __func__, Status));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("%s<-----------------\n", __func__));

	return;
}

/*
	==========================================================================
	Description:
	Extend event of querying data-captured status handler of ICAP.
	Return:
	==========================================================================
*/
VOID ExtEventICapStatusHandler(
	IN RTMP_ADAPTER *pAd,
	IN UINT8 *pData,
	IN UINT32 Length)
{
	EXT_EVENT_RBIST_ADDR_T *prICapGetEvent = (EXT_EVENT_RBIST_ADDR_T *)pData;
	/* save iCap result */
	/* send iCap result to QA tool */
#ifdef CONFIG_ATE
	NdisMoveMemory(&(pAd->ATECtrl.icap_info), prICapGetEvent, sizeof(EXT_EVENT_RBIST_ADDR_T));
#endif /* CONFIG_ATE */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: prICapGetEvent->"
			"u4StartAddr1 = 0x%x\n", __func__, prICapGetEvent->u4StartAddr1));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: prICapGetEvent->"
			"u4StartAddr2 = 0x%x\n", __func__, prICapGetEvent->u4StartAddr2));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: prICapGetEvent->"
			"u4StartAddr3 = 0x%x\n", __func__, prICapGetEvent->u4StartAddr3));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: prICapGetEvent->"
			"u4EndAddr = 0x%x\n", __func__, prICapGetEvent->u4EndAddr));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: prICapGetEvent->"
			"u4StopAddr = 0x%x\n", __func__, prICapGetEvent->u4StopAddr));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: prICapGetEvent->"
			"u4Wrap = 0x%x\n", __func__, prICapGetEvent->u4Wrap));
#ifdef CONFIG_ATE
	RTMP_OS_COMPLETE(&(pAd->ATECtrl.cmd_done));
#endif /* CONFIG_ATE */
}
#endif /* INTERNAL_CAPTURE_SUPPORT */

#ifdef RACTRL_FW_OFFLOAD_SUPPORT
static VOID ExtEventThroughputBurst(RTMP_ADAPTER *pAd,
									UINT8 *Data, UINT32 Length)
{
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	struct wifi_dev *wdev;
	BOOLEAN fgEnable = FALSE;
	UCHAR pkt_num = 0;
	UINT32 length = 0;
#ifdef CONFIG_AP_SUPPORT
	wdev = &pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev;
#endif

	if (!Data || !wdev) {
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s:: Data is NULL\n", __func__));
		return;
	}

	fgEnable = (BOOLEAN)(*Data);
	pAd->bDisableRtsProtect = fgEnable;

	if (pAd->bDisableRtsProtect) {
		pkt_num = MAX_RTS_PKT_THRESHOLD;
		length = MAX_RTS_THRESHOLD;
	} else {
		pkt_num = wlan_operate_get_rts_pkt_thld(wdev);
		length = wlan_operate_get_rts_len_thld(wdev);
	}

	HW_SET_RTS_THLD(pAd, wdev, pkt_num, length);
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("%s::%d\n", __func__, fgEnable));
}


static VOID ExtEventGBand256QamProbeResule(RTMP_ADAPTER *pAd,
		UINT8 *Data, UINT32 Length)
{
	P_EXT_EVENT_G_BAND_256QAM_PROBE_RESULT_T pResult;
	MAC_TABLE_ENTRY *pEntry = NULL;

	if (Data != NULL) {
		pResult = (P_EXT_EVENT_G_BAND_256QAM_PROBE_RESULT_T)(Data);
		pEntry = &pAd->MacTab.Content[pResult->ucWlanIdx];

		if (IS_ENTRY_NONE(pEntry)) {
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s:: pEntry is NONE\n", __func__));
			return;
		}

		if (pResult->ucResult == RA_G_BAND_256QAM_PROBE_SUCCESS)
			pEntry->fgGband256QAMSupport = TRUE;

		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("%s::Gband256QAMSupport = %d\n", __func__, pResult->ucResult));
	} else {
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s:: Data is NULL\n", __func__));
	}
}
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */

UINT max_line = 130;

static VOID ExtEventAssertDumpHandler(RTMP_ADAPTER *pAd, UINT8 *Data,
									  UINT32 Length, EVENT_RXD *event_rxd)
{
	struct _EXT_EVENT_ASSERT_DUMP_T *pExtEventAssertDump =
		(struct _EXT_EVENT_ASSERT_DUMP_T *)Data;

	if (max_line) {
		if (max_line == 130)
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("**************************************************\n\n"));

		max_line--;
		pExtEventAssertDump->aucBuffer[Length] = 0;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s\n", pExtEventAssertDump->aucBuffer));
	}

#ifdef FW_DUMP_SUPPORT

	if (!pAd->fw_dump_buffer) {
		os_alloc_mem(pAd, &pAd->fw_dump_buffer, pAd->fw_dump_max_size);
		pAd->fw_dump_size = 0;
		pAd->fw_dump_read = 0;

		if (pAd->fw_dump_buffer) {
			if (event_rxd->fw_rxd_2.field.s2d_index == N92HOST)
				RTMP_OS_FWDUMP_PROCCREATE(pAd, "_N9");
			else if (event_rxd->fw_rxd_2.field.s2d_index == CR42HOST)
				RTMP_OS_FWDUMP_PROCCREATE(pAd, "_CR4");
			else
				RTMP_OS_FWDUMP_PROCCREATE(pAd, "\0");
		} else {
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: cannot alloc mem for FW dump\n", __func__));
		}
	}

	if (pAd->fw_dump_buffer) {
		if ((pAd->fw_dump_size + Length) <= pAd->fw_dump_max_size) {
			os_move_mem(pAd->fw_dump_buffer + pAd->fw_dump_size, Data, Length);
			pAd->fw_dump_size += Length;
		} else {
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: FW dump size too big\n", __func__));
		}
	}

#endif
}

static VOID ExtEventPsSyncHandler(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	struct _EXT_EVENT_PS_SYNC_T *pExtEventPsSync =
		(struct _EXT_EVENT_PS_SYNC_T *)Data;

	struct _STA_TR_ENTRY *tr_entry = NULL;
	UCHAR wcid = pExtEventPsSync->ucWtblIndex;
	struct qm_ctl *qm_ctl = &pAd->qm_ctl;
	struct qm_ops *qm_ops = pAd->qm_ops;
	NDIS_PACKET *pkt = NULL;

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s: PsSync Event from FW APPS WIdx=%d PSBit=%d len=%d\n", __func__,
			wcid, pExtEventPsSync->ucPsBit, Length));

	if (VALID_TR_WCID(wcid)) {
		tr_entry = &pAd->MacTab.tr_entry[wcid];

		RTMP_SEM_LOCK(&tr_entry->ps_sync_lock);

		tr_entry->ps_state = (pExtEventPsSync->ucPsBit == 0) ? FALSE : TRUE;

		if (tr_entry->ps_state == PWR_ACTIVE) {
			do {
				pkt = qm_ops->get_psq_pkt(pAd, tr_entry);

				if (pkt) {
					UCHAR q_idx = RTMP_GET_PACKET_QUEIDX(pkt);
					UCHAR wdev_idx = RTMP_GET_PACKET_WDEV(pkt);
					struct wifi_dev *wdev = pAd->wdev_list[wdev_idx];

					qm_ctl->total_psq_cnt--;
					qm_ops->enq_dataq_pkt(pAd, wdev, pkt, q_idx);
				}
			} while (pkt);
		}

		RTMP_SEM_UNLOCK(&tr_entry->ps_sync_lock);

	} else {

		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: wtbl index(%d) is invalid\n", __func__, wcid));
	}
}

#if defined(MT_MAC) && (!defined(MT7636)) && defined(TXBF_SUPPORT)
VOID ExtEventBfStatusRead(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	struct _EXT_EVENT_BF_STATUS_T *pExtEventBfStatus = (struct _EXT_EVENT_BF_STATUS_T *)Data;
	struct _EXT_EVENT_IBF_STATUS_T *pExtEventIBfStatus = (struct _EXT_EVENT_IBF_STATUS_T *)Data;
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

#if defined(CONFIG_ATE) && defined(CONFIG_QA)

	if (ATE_ON(pAd))
		HQA_BF_INFO_CB(pAd, Data, Length);

#endif

	switch (pExtEventBfStatus->ucBfDataFormatID) {
	case BF_PFMU_TAG:
		TxBfProfileTagPrint(pAd,
							pExtEventBfStatus->fgBFer,
							pExtEventBfStatus->aucBuffer);
		break;

	case BF_PFMU_DATA:
		TxBfProfileDataPrint(pAd,
							 le2cpu16(pExtEventBfStatus->u2subCarrIdx),
							 pExtEventBfStatus->aucBuffer);
		break;

	case BF_PFMU_PN:
		TxBfProfilePnPrint(pExtEventBfStatus->ucBw,
						   pExtEventBfStatus->aucBuffer);
		break;

	case BF_PFMU_MEM_ALLOC_MAP:
		TxBfProfileMemAllocMap(pExtEventBfStatus->aucBuffer);
		break;

	case BF_STAREC:
		StaRecBfRead(pAd, pExtEventBfStatus->aucBuffer);
		break;

	case BF_CAL_PHASE:
		ops->iBFPhaseCalReport(pAd,
									   pExtEventIBfStatus->ucGroup_L_M_H,
									   pExtEventIBfStatus->ucGroup,
									   pExtEventIBfStatus->fgSX2,
									   pExtEventIBfStatus->ucStatus,
									   pExtEventIBfStatus->ucPhaseCalType,
									   pExtEventIBfStatus->aucBuffer);
		break;

	default:
		break;
	}
}
#endif /* MT_MAC && TXBF_SUPPORT */

#define NET_DEV_NAME_MAX_LENGTH	16
static VOID ExtEventFwLog2HostHandler(RTMP_ADAPTER *pAd, UINT8 *Data,
									  UINT32 Length, EVENT_RXD *event_rxd)
{
	UCHAR *dev_name = NULL;
	UCHAR empty_name[] = " ";
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s: s2d_index = 0x%x\n", __func__,
			  event_rxd->fw_rxd_2.field.s2d_index));
	dev_name = RtmpOsGetNetDevName(pAd->net_dev);

	if ((dev_name == NULL) || strlen(dev_name) >= NET_DEV_NAME_MAX_LENGTH)
		dev_name = &empty_name[0];

	if (event_rxd->fw_rxd_2.field.s2d_index == N92HOST) {
#ifdef PRE_CAL_TRX_SET1_SUPPORT

		if (pAd->KtoFlashDebug)
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 ("(%s): %s", dev_name, Data));
		else
#endif /*PRE_CAL_TRX_SET1_SUPPORT*/
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 ("N9 LOG(%s): %s\n", dev_name, Data));
	} else if (event_rxd->fw_rxd_2.field.s2d_index == CR42HOST) {
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("CR4 LOG(%s): %s\n", dev_name, Data));
	} else {
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("unknow MCU LOG(%s): %s", dev_name, Data));
	}
}

#ifdef COEX_SUPPORT
static VOID ExtEventBTCoexHandler(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{

	UINT8 SubOpCode;
	MAC_TABLE_ENTRY *pEntry;
	struct _EVENT_EXT_COEXISTENCE_T *coext_event_t =
		(struct _EVENT_EXT_COEXISTENCE_T *)Data;
	SubOpCode = coext_event_t->ucSubOpCode;
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("SubOpCode: 0x%x\n", coext_event_t->ucSubOpCode));
	hex_dump("Coex Event payload ", coext_event_t->aucBuffer, Length);

	if (SubOpCode == 0x01) {
		struct _EVENT_COEX_CMD_RESPONSE_T *CoexResp =
			(struct _EVENT_COEX_CMD_RESPONSE_T *)coext_event_t->aucBuffer;
		CoexResp->u4Status = le2cpu32(CoexResp->u4Status);
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("--->Cmd_Resp=0x%x\n", CoexResp->u4Status));
	} else if (SubOpCode == 0x02) {
		struct _EVENT_COEX_REPORT_COEX_MODE_T *CoexReportMode =
			(struct _EVENT_COEX_REPORT_COEX_MODE_T *)coext_event_t->aucBuffer;
		CoexReportMode->u4SupportCoexMode = le2cpu32(CoexReportMode->u4SupportCoexMode);
		CoexReportMode->u4CurrentCoexMode = le2cpu32(CoexReportMode->u4CurrentCoexMode);
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("--->SupportCoexMode=0x%x\n", CoexReportMode->u4SupportCoexMode));
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("--->CurrentCoexMode=0x%x\n", CoexReportMode->u4CurrentCoexMode));
		pAd->BtCoexSupportMode = ((CoexReportMode->u4SupportCoexMode) & 0x3);
		pAd->BtCoexMode = ((CoexReportMode->u4CurrentCoexMode) & 0x3);
	} else if (SubOpCode == 0x03) {
		struct _EVENT_COEX_MASK_OFF_TX_RATE_T *CoexMaskTxRate =
			(struct _EVENT_COEX_MASK_OFF_TX_RATE_T *)coext_event_t->aucBuffer;
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("--->MASK_OFF_TX_RATE=0x%x\n", CoexMaskTxRate->ucOn));
	} else if (SubOpCode == 0x04) {
		struct _EVENT_COEX_CHANGE_RX_BA_SIZE_T *CoexChgBaSize =
			(struct _EVENT_COEX_CHANGE_RX_BA_SIZE_T *)coext_event_t->aucBuffer;
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("--->Change_BA_Size ucOn=%d\n", CoexChgBaSize->ucOn));
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("--->Change_BA_Size=0x%x\n", CoexChgBaSize->ucRXBASize));
		pEntry = &pAd->MacTab.Content[BSSID_WCID];

		if (CoexChgBaSize->ucOn == 1) {
			BA_REC_ENTRY *pBAEntry = NULL;
			UCHAR Idx;

			Idx = pEntry->BARecWcidArray[0];
			pBAEntry = &pAd->BATable.BARecEntry[Idx];
			pAd->BtCoexBASize = CoexChgBaSize->ucRXBASize;

			if (pBAEntry->BAWinSize == 0) {
				pBAEntry->BAWinSize =
					pAd->CommonCfg.REGBACapability.field.RxBAWinLimit;
			}

			if (pAd->BtCoexBASize != 0 &&
				((pAd->BtCoexBASize < pAd->CommonCfg.REGBACapability.field.RxBAWinLimit) ||
				(pAd->BtCoexBASize < pAd->CommonCfg.REGBACapability.field.TxBAWinLimit)
				)
				) {

				if (pAd->BtCoexBASize < pAd->CommonCfg.REGBACapability.field.RxBAWinLimit)
					pAd->CommonCfg.BACapability.field.RxBAWinLimit = pAd->BtCoexBASize;
				if (pAd->BtCoexBASize < pAd->CommonCfg.REGBACapability.field.TxBAWinLimit)
					pAd->CommonCfg.BACapability.field.TxBAWinLimit = pAd->BtCoexBASize;

				ba_ori_session_tear_down(pAd, BSSID_WCID, 0, FALSE, FALSE);
				ba_rec_session_tear_down(pAd, BSSID_WCID, 0, FALSE);
				MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						 ("COEX: TDD mode: Set RxBASize to %d\n", pAd->BtCoexBASize));
			}
		} else {
			pAd->CommonCfg.BACapability.field.RxBAWinLimit =
				pAd->CommonCfg.REGBACapability.field.RxBAWinLimit;
			pAd->CommonCfg.BACapability.field.TxBAWinLimit =
				pAd->CommonCfg.REGBACapability.field.TxBAWinLimit;


			ba_ori_session_tear_down(pAd, BSSID_WCID, 0, FALSE, FALSE);
			ba_rec_session_tear_down(pAd, BSSID_WCID, 0, FALSE);

			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("COEX: TDD mode: Set RxBASize to %d\n", pAd->BtCoexOriBASize));
		}
	} else if (SubOpCode == 0x05) {
		struct _EVENT_COEX_LIMIT_BEACON_SIZE_T *CoexLimitBeacon =
			(struct _EVENT_COEX_LIMIT_BEACON_SIZE_T *)coext_event_t->aucBuffer;
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("--->COEX_LIMIT_BEACON_SIZE ucOn =%d\n", CoexLimitBeacon->ucOn));
		pAd->BtCoexBeaconLimit = CoexLimitBeacon->ucOn;
	} else if (SubOpCode == 0x06) {
		struct _EVENT_COEX_EXTEND_BTO_ROAMING_T *CoexExtendBTORoam =
			(struct _EVENT_COEX_EXTEND_BTO_ROAMING_T *)coext_event_t->aucBuffer;
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("--->EVENT_COEX_EXTEND_BNCTO_ROAMING ucOn =%d\n",
				  CoexExtendBTORoam->ucOn));
	}
}
#endif /* COEX_SUPPORT */

#ifdef BCN_OFFLOAD_SUPPORT
BOOLEAN MtUpdateBcnAndTimToMcu(
	IN RTMP_ADAPTER *pAd,
	VOID *wdev_void,
	IN UINT16 FrameLen,
	IN UCHAR UpdatePktType)
{
	BCN_BUF_STRUC *bcn_buf = NULL;
#ifdef CONFIG_AP_SUPPORT
	TIM_BUF_STRUC *tim_buf = NULL;
#endif
	UCHAR *buf;
	INT len;
	PNDIS_PACKET *pkt = NULL;
	CMD_BCN_OFFLOAD_T bcn_offload;
	struct wifi_dev *wdev = (struct wifi_dev *)wdev_void;
	BOOLEAN bSntReq = FALSE;
	UCHAR TimIELocation = 0, CsaIELocation = 0;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 tx_hw_hdr_len = cap->tx_hw_hdr_len;

	NdisZeroMemory(&bcn_offload, sizeof(CMD_BCN_OFFLOAD_T));

	if (!wdev) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s(): wdev is NULL!\n", __func__));
		return FALSE;
	}

	bcn_buf = &wdev->bcn_buf;

	if (!bcn_buf) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s(): bcn_buf is NULL!\n", __func__));
		return FALSE;
	}

	if (UpdatePktType == PKT_BCN) {
		pkt = bcn_buf->BeaconPkt;
		bSntReq = bcn_buf->bBcnSntReq;
		TimIELocation = bcn_buf->TimIELocationInBeacon;
		CsaIELocation = bcn_buf->CsaIELocationInBeacon;
	}

#ifdef CONFIG_AP_SUPPORT
	else { /* tim pkt case in AP mode. */
		if (pAd->OpMode == OPMODE_AP)
			tim_buf = &wdev->bcn_buf.tim_buf;

		if (!tim_buf) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s(): tim_buf is NULL!\n", __func__));
			return FALSE;
		}

		pkt = tim_buf->TimPkt;
		bSntReq = tim_buf->bTimSntReq;
		TimIELocation = bcn_buf->TimIELocationInTim;
		CsaIELocation = bcn_buf->CsaIELocationInBeacon;
	}

#endif /* CONFIG_AP_SUPPORT */

	if (pkt) {
		buf = (UCHAR *)GET_OS_PKT_DATAPTR(pkt);
		len = FrameLen + tx_hw_hdr_len;/* TXD & pkt content. */
		bcn_offload.ucEnable = bSntReq;
		bcn_offload.ucWlanIdx = 0;/* hardcode at present */
		bcn_offload.ucOwnMacIdx = wdev->OmacIdx;
		bcn_offload.ucBandIdx = HcGetBandByWdev(wdev);
		bcn_offload.u2PktLength = len;
		bcn_offload.ucPktType = UpdatePktType;
		bcn_offload.fgNeedPretbttIntEvent = cap->fgIsNeedPretbttIntEvent;
#ifdef CONFIG_AP_SUPPORT
		bcn_offload.u2TimIePos = TimIELocation + tx_hw_hdr_len;
		bcn_offload.u2CsaIePos = CsaIELocation + tx_hw_hdr_len;
		bcn_offload.ucCsaCount = wdev->csa_count;
#endif
		NdisCopyMemory(bcn_offload.acPktContent, buf, len);
		MtCmdBcnOffloadSet(pAd, bcn_offload);
	} else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s(): BeaconPkt is NULL!\n", __func__));
		return FALSE;
	}

	return TRUE;
}
#endif /*BCN_OFFLOAD*/

#if defined(PRETBTT_INT_EVENT_SUPPORT) || defined(BCN_OFFLOAD_SUPPORT)
static VOID ExtEventPretbttIntHandler(RTMP_ADAPTER *pAd,
									  UINT8 *Data, UINT32 Length)
{
	if ((RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET))      ||
		(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS))   ||
		(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)))
		return;

	RTMP_HANDLE_PRETBTT_INT_EVENT(pAd);
}
#endif

#ifdef THERMAL_PROTECT_SUPPORT
static VOID EventThermalProtect(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	EXT_EVENT_THERMAL_PROTECT_T *EvtThermalProtect;
	UINT8 HLType;
	UINT8 Reason;
	struct wifi_dev *wdev;
	POS_COOKIE pObj;
	pObj = (POS_COOKIE) pAd->OS_Cookie;
#ifdef CONFIG_AP_SUPPORT
	wdev = &pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev;
#endif
	EvtThermalProtect = (EXT_EVENT_THERMAL_PROTECT_T *)Data;
	HLType = EvtThermalProtect->ucHLType;
	Reason = EvtThermalProtect->ucReason;
	pAd->last_thermal_pro_temp = EvtThermalProtect->cCurrentTemp;
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s: HLType = %d, CurrentTemp = %d, Reason = %d\n",
			  __func__, HLType, pAd->last_thermal_pro_temp, Reason));

	if (Reason == THERAML_PROTECTION_REASON_RADIO) {
		pAd->force_radio_off = TRUE;
		/* AsicRadioOnOffCtrl(pAd, HcGetBandByWdev(wdev), WIFI_RADIO_OFF); */
		/* Set Radio off Process*/
		/* Set_RadioOn_Proc(pAd, "0"); */
		RTMP_SET_THERMAL_RADIO_OFF(pAd);
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Radio Off due to too high temperature.\n"));
	}

}
#endif
#ifdef CFG_TDLS_SUPPORT
static VOID ExtEventTdlsBackToBaseHandler(RTMP_ADAPTER *pAd,
		UINT8 *Data, UINT32 Length)
{
	P_EXT_EVENT_TDLS_STATUS_T prEventExtCmdResult =
		(P_EXT_EVENT_TDLS_STATUS_T)Data;
	INT chsw_fw_resp = 0;
	chsw_fw_resp = prEventExtCmdResult->ucResultId;

	switch (chsw_fw_resp) {
	case 1:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,
				 ("RX RSP_EVT_TYPE_TDLS_STAY_TIME_OUT\n"));
		PCFG_TDLS_STRUCT pCfgTdls =
			&pAd->StaCfg[0].wpa_supplicant_info.CFG_Tdls_info;
		cfg_tdls_chsw_resp(pAd, pCfgTdls->CHSWPeerMacAddr,
						   pCfgTdls->ChSwitchTime, pCfgTdls->ChSwitchTimeout, 0);
		break;

	case 2:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,
				 ("RX RSP_EVT_TYPE_TDLS_BACK_TO_BASE_CHANNEL\n"));
		pAd->StaCfg[0].wpa_supplicant_info.CFG_Tdls_info.IamInOffChannel = FALSE;
		break;

	default:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,
				 ("%s : unknown event type %d\n", __func__, chsw_fw_resp));
		break;
	}
}
#endif /* CFG_TDLS_SUPPORT */

#define MAX_MSDU_SIZE 1544
static VOID ExtEventMaxAmsduLengthUpdate(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	P_EXT_EVENT_MAX_AMSDU_LENGTH_UPDATE_T len_update =
		(P_EXT_EVENT_MAX_AMSDU_LENGTH_UPDATE_T)Data;
	MAC_TABLE_ENTRY *mac_entry = NULL;
	UINT8 wcid = len_update->ucWlanIdx;
	UINT8 amsdu_len = len_update->ucAmsduLen;

	/* this is a temporary workaround to fix no amsdu at HT20 high rate */
	if (amsdu_len == 0)
		amsdu_len = 1;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("ExtEventMaxAmsduLengthUpdate: wlan_idx = %d,\
			  amsdu_len = %d\n",
			  wcid, amsdu_len));

	if (!VALID_UCAST_ENTRY_WCID(pAd, wcid))
		return;

	mac_entry = &pAd->MacTab.Content[wcid];

	if (mac_entry->amsdu_limit_len != 0) {
		mac_entry->amsdu_limit_len_adjust = (mac_entry->amsdu_limit_len < (MAX_MSDU_SIZE * amsdu_len)
						? mac_entry->amsdu_limit_len : (MAX_MSDU_SIZE * amsdu_len));
	}
}

static VOID ExtEventBaTriggerHandler(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	P_CMD_BA_TRIGGER_EVENT_T prEventExtBaTrigger =
		(P_CMD_BA_TRIGGER_EVENT_T)Data;
	STA_TR_ENTRY *tr_entry = NULL;
	MAC_TABLE_ENTRY *mac_entry = NULL;
	UINT8 wcid = 0;
	UINT8 tid = 0;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("RX P_CMD_BA_TRIGGER_EVENT_T: Wcid=%d, Tid=%d\n",
			  prEventExtBaTrigger->ucWlanIdx, prEventExtBaTrigger->ucTid));
	wcid = prEventExtBaTrigger->ucWlanIdx;

	if (!VALID_UCAST_ENTRY_WCID(pAd, wcid))
		return;

	tid = prEventExtBaTrigger->ucTid;
	tr_entry = &pAd->MacTab.tr_entry[wcid];
	mac_entry = &pAd->MacTab.Content[tr_entry->wcid];
	ba_ori_session_setup(pAd, mac_entry, tid, 0, 10, FALSE);
}

static VOID ExtEventTmrCalcuInfoHandler(
	RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	P_EXT_EVENT_TMR_CALCU_INFO_T ptmr_calcu_info;
	TMR_FRM_STRUC *p_tmr_frm;
	ptmr_calcu_info = (P_EXT_EVENT_TMR_CALCU_INFO_T)Data;
	p_tmr_frm = (TMR_FRM_STRUC *)ptmr_calcu_info->aucTmrFrm;
#ifdef RT_BIG_ENDIAN
	RTMPEndianChange(p_tmr_frm, sizeof(TMR_FRM_STRUC));
#endif
	/*Tmr pkt comes to FW event, fw already take cares of the whole calculation.*/
	TmrReportParser(pAd, p_tmr_frm, TRUE, le2cpu32(ptmr_calcu_info->u4TOAECalibrationResult));
}

static VOID ExtEventCswNotifyHandler(
	RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	P_EXT_EVENT_CSA_NOTIFY_T csa_notify_event = (P_EXT_EVENT_CSA_NOTIFY_T)Data;
	struct wifi_dev *wdev;
	struct DOT11_H *pDot11h = NULL;
	UCHAR Index;
	struct wifi_dev *wdevEach = NULL;

	wdev = wdev_search_by_omac_idx(pAd, csa_notify_event->ucOwnMacIdx);

	if (!wdev)
		return;

	wdev->csa_count = csa_notify_event->ucChannelSwitchCount;
	pDot11h = wdev->pDot11_H;
	if (pDot11h == NULL)
		return;
	for (Index = 0; Index < WDEV_NUM_MAX; Index++) {
		wdevEach = pAd->wdev_list[Index];
		if (wdevEach == NULL)
			continue;
		if (wdevEach->pHObj == NULL)
			continue;
		if (HcGetBandByWdev(wdevEach) == HcGetBandByWdev(wdev)) {
			wdevEach->csa_count = wdev->csa_count;
		}
	}

	if ((HcIsRfSupport(pAd, RFIC_5GHZ))
		&& (pAd->CommonCfg.bIEEE80211H == 1)
		&& (pDot11h->RDMode == RD_SWITCHING_MODE)) {
#ifdef CONFIG_AP_SUPPORT
		pDot11h->CSCount = pDot11h->CSPeriod;
		ChannelSwitchingCountDownProc(pAd, wdev);
#endif /*CONFIG_AP_SUPPORT*/
	}
}

static VOID ExtEventBssAcQPktNumHandler(RTMP_ADAPTER *pAd,
										UINT8 *Data, UINT32 Length)
{
	P_EVENT_BSS_ACQ_PKT_NUM_T prEventBssAcQPktNum =
		(P_EVENT_BSS_ACQ_PKT_NUM_T)Data;
	UINT8 i = 0;
	UINT32 sum = 0;
	P_EVENT_PER_BSS_ACQ_PKT_NUM_T prPerBssInfo = NULL;
#ifdef RT_BIG_ENDIAN
	prEventBssAcQPktNum->u4BssMap = le2cpu32(prEventBssAcQPktNum->u4BssMap);
#endif
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("RX ExtEventBssAcQPktNumHandler: u4BssMap=0x%08X\n",
			  prEventBssAcQPktNum->u4BssMap));

	for (i = 0; (i < CR4_CFG_BSS_NUM) && (prEventBssAcQPktNum->u4BssMap & (1 << i)) ; i++) {
		prPerBssInfo = &prEventBssAcQPktNum->bssPktInfo[i];
#ifdef RT_BIG_ENDIAN
		prPerBssInfo->au4AcqPktCnt[WMM_AC_BK] = le2cpu32(prPerBssInfo->au4AcqPktCnt[WMM_AC_BK]);
		prPerBssInfo->au4AcqPktCnt[WMM_AC_BE] = le2cpu32(prPerBssInfo->au4AcqPktCnt[WMM_AC_BE]);
		prPerBssInfo->au4AcqPktCnt[WMM_AC_VI] = le2cpu32(prPerBssInfo->au4AcqPktCnt[WMM_AC_VI]);
		prPerBssInfo->au4AcqPktCnt[WMM_AC_VO] = le2cpu32(prPerBssInfo->au4AcqPktCnt[WMM_AC_VO]);
#endif
		sum =  prPerBssInfo->au4AcqPktCnt[WMM_AC_BK]
			+ prPerBssInfo->au4AcqPktCnt[WMM_AC_VI]
			+ prPerBssInfo->au4AcqPktCnt[WMM_AC_VO];

		if (sum) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 ("BSS[%d], AC_BK = %d, AC_BE = %d, AC_VI = %d, AC_VO = %d\n",
					  i,
					  prPerBssInfo->au4AcqPktCnt[WMM_AC_BK],
					  prPerBssInfo->au4AcqPktCnt[WMM_AC_BE],
					  prPerBssInfo->au4AcqPktCnt[WMM_AC_VI],
					  prPerBssInfo->au4AcqPktCnt[WMM_AC_VO]));
			pAd->OneSecondnonBEpackets += sum;
		}
	}

	mt_dynamic_wmm_be_tx_op(pAd, ONE_SECOND_NON_BE_PACKETS_THRESHOLD);
}

#ifdef CONFIG_HOTSPOT_R2
static VOID ExtEventReprocessPktHandler(RTMP_ADAPTER *pAd,
										UINT8 *Data, UINT32 Length)
{
	P_CMD_PKT_REPROCESS_EVENT_T prReprocessPktEvt =
		(P_CMD_PKT_REPROCESS_EVENT_T)Data;
	PNDIS_PACKET pPacket = NULL;
	UINT8 Type = 0;
	UINT16 reprocessToken = 0;
	PKT_TOKEN_CB *pktTokenCb = pAd->PktTokenCb;
	PKT_TOKEN_QUEUE *listq = &pktTokenCb->tx_id_list;

	prReprocessPktEvt->u2MsduToken = le2cpu16(prReprocessPktEvt->u2MsduToken);
	reprocessToken = prReprocessPktEvt->u2MsduToken;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,
			 ("%s - Reprocess Token ID = %d\n", __func__, reprocessToken));
	pPacket = listq->list->pkt_token[reprocessToken].pkt_buf;

	if (pPacket == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s - Unexpected Reprocessing TOKEN ID %d, pkt_buf NULL!!!!!!!\n",
				 __func__, reprocessToken));
		return;
	}

	listq->list->pkt_token[reprocessToken].Reprocessed = TRUE;
	cut_through_tx_deq(pAd->PktTokenCb, reprocessToken, &Type);

	if (hotspot_check_dhcp_arp(pAd, pPacket) == TRUE) {
		USHORT Wcid = RTMP_GET_PACKET_WCID(pPacket);
		MAC_TABLE_ENTRY *pMacEntry = &pAd->MacTab.Content[Wcid];
		struct wifi_dev *wdev = pMacEntry->wdev;
		RTMP_SET_PACKET_DIRECT_TX(pPacket, 1);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s - Driver Direct TX\n", __func__));
		send_data_pkt(pAd, wdev, pPacket);
	} else {
		if (Type == TOKEN_TX_DATA)
			RELEASE_NDIS_PACKET_IRQ(pAd, pPacket, NDIS_STATUS_SUCCESS);
		else {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s - Unexpected Reprocessing Mgmt!!\n", __func__));
			RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
		}
	}
}

static VOID ExtEventGetHotspotCapabilityHandler(RTMP_ADAPTER *pAd,
		UINT8 *Data, UINT32 Length)
{
	P_CMD_GET_CR4_HOTSPOT_CAPABILITY_T prGetCapaEvt =
		(P_CMD_GET_CR4_HOTSPOT_CAPABILITY_T)Data;
	UINT8 i = 0;

	for (i = 0; i < 2; i++) {
		PHOTSPOT_CTRL pHSCtrl =  &pAd->ApCfg.MBSSID[i].HotSpotCtrl;
		PWNM_CTRL pWNMCtrl = &pAd->ApCfg.MBSSID[i].WNMCtrl;
		MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("======== BSSID %d  CR4 ======\n", i));
		hotspot_bssflag_dump(prGetCapaEvt->ucHotspotBssFlags[i]);
		MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("======== BSSID %d  DRIVER ======\n", i));
		MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("pHSCtrl->HotSpotEnable = %d\n"
				  "pHSCtrl->ProxyARPEnable = %d\n"
				  "pHSCtrl->ASANEnable = %d\n"
				  "pHSCtrl->DGAFDisable = %d\n"
				  "pHSCtrl->QosMapEnable = %d\n"
				  , pHSCtrl->HotSpotEnable
				  , pWNMCtrl->ProxyARPEnable
				  , pHSCtrl->bASANEnable
				  , pHSCtrl->DGAFDisable
				  , pHSCtrl->QosMapEnable
				 ));
		MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("======== BSSID %d END======\n", i));
	}
}
#endif /* CONFIG_HOTSPOT_R2 */

static VOID ext_event_get_cr4_tx_statistics(RTMP_ADAPTER *pAd, UINT8 *data, UINT32 len)
{
	P_EXT_EVENT_GET_CR4_TX_STATISTICS_T tx_statistics =
		(P_EXT_EVENT_GET_CR4_TX_STATISTICS_T)data;
	MAC_TABLE *table = &pAd->MacTab;
	MAC_TABLE_ENTRY *entry = &table->Content[tx_statistics->wlan_index];
	entry->OneSecTxBytes = tx_statistics->one_sec_tx_bytes;
	entry->one_sec_tx_pkts = tx_statistics->one_sec_tx_cnts;
	entry->AvgTxBytes = (entry->AvgTxBytes == 0) ?
						entry->OneSecTxBytes :
						((entry->AvgTxBytes + entry->OneSecTxBytes) >> 1);
	entry->avg_tx_pkts = (entry->avg_tx_pkts == 0) ? \
						 entry->one_sec_tx_pkts : \
						 ((entry->avg_tx_pkts + entry->one_sec_tx_pkts) >> 1);
}

static BOOLEAN IsUnsolicitedEvent(EVENT_RXD *event_rxd)
{
	if ((GET_EVENT_FW_RXD_SEQ_NUM(event_rxd) == 0)                          ||
		(GET_EVENT_FW_RXD_EXT_EID(event_rxd) == EXT_EVENT_FW_LOG_2_HOST)	||
		(GET_EVENT_FW_RXD_EXT_EID(event_rxd) == EXT_EVENT_THERMAL_PROTECT)  ||
		(GET_EVENT_FW_RXD_EXT_EID(event_rxd) == EXT_EVENT_ID_ASSERT_DUMP) ||
		(GET_EVENT_FW_RXD_EXT_EID(event_rxd) == EXT_EVENT_ID_PS_SYNC))
		return TRUE;

	return FALSE;
}

#if defined(RLM_CAL_CACHE_SUPPORT) || defined(PRE_CAL_TRX_SET2_SUPPORT)
static INT ExtEventPreCalStoreProc(RTMP_ADAPTER *pAd, UINT32 EventId, UINT8 *Data)
{
	UINT32 Offset = 0, IDOffset = 0, LenOffset = 0, HeaderSize = 0, CalDataSize = 0, BitMap = 0;
	UINT32 i, Length, TotalSize, ChGroupId = 0;
	UINT16 DoPreCal = 0;
	static int rf_count;
	int rf_countMax;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("\x1b[41m %s ------------> \x1b[m\n", __FUNCTION__));
	rf_countMax = pAd->ChGrpMap == 0x1FF ? 9 : 6;

	switch (EventId) {
	case PRECAL_TXLPF: {
		TXLPF_CAL_INFO_T *prTxLPFGetEvent = (TXLPF_CAL_INFO_T *)Data;
		/* Store header */
		HeaderSize = (UINT32)(uintptr_t)&((TXLPF_CAL_INFO_T *)NULL)->au4Data[0];
		/* Update Cal data size */
		CalDataSize = TXLPF_PER_GROUP_DATA_SIZE;
		/* Update channel group BitMap */
		prTxLPFGetEvent->u2BitMap = le2cpu16(prTxLPFGetEvent->u2BitMap);
		BitMap = prTxLPFGetEvent->u2BitMap;
		for (i = 0; i < CHANNEL_GROUP_NUM*SCN_NUM; i++)
			prTxLPFGetEvent->au4Data[i] = le2cpu32(prTxLPFGetEvent->au4Data[i]);
	}
	break;

	case PRECAL_TXIQ: {
		TXIQ_CAL_INFO_T *prTxIQGetEvent = (TXIQ_CAL_INFO_T *)Data;
		/* Store header */
		HeaderSize = (UINT32)(uintptr_t)&((TXIQ_CAL_INFO_T *)NULL)->au4Data[0];
		/* Update Cal data size */
		CalDataSize = TXIQ_PER_GROUP_DATA_SIZE;
		/* Update channel group BitMap */
		prTxIQGetEvent->u2BitMap = le2cpu16(prTxIQGetEvent->u2BitMap);
		BitMap = prTxIQGetEvent->u2BitMap;
		for (i = 0; i < CHANNEL_GROUP_NUM*SCN_NUM*6; i++)
			prTxIQGetEvent->au4Data[i] = le2cpu32(prTxIQGetEvent->au4Data[i]);
	}
	break;

	case PRECAL_TXDC: {
		TXDC_CAL_INFO_T *prTxDCGetEvent = (TXDC_CAL_INFO_T *)Data;
		/* Store header */
		HeaderSize = (UINT32)(uintptr_t)&((TXDC_CAL_INFO_T *)NULL)->au4Data[0];
		/* Update Cal data size */
		CalDataSize = TXDC_PER_GROUP_DATA_SIZE;
		/* Update channel group BitMap */
		prTxDCGetEvent->u2BitMap = le2cpu16(prTxDCGetEvent->u2BitMap);
		BitMap = prTxDCGetEvent->u2BitMap;
		for (i = 0; i < CHANNEL_GROUP_NUM*SCN_NUM*6; i++)
			prTxDCGetEvent->au4Data[i] = le2cpu32(prTxDCGetEvent->au4Data[i]);
	}
	break;

	case PRECAL_RXFI: {
		RXFI_CAL_INFO_T *prRxFIGetEvent = (RXFI_CAL_INFO_T *)Data;
		/* Store header */
		HeaderSize = (UINT32)(uintptr_t)&((RXFI_CAL_INFO_T *)NULL)->au4Data[0];
		/* Update Cal data size */
		CalDataSize = RXFI_PER_GROUP_DATA_SIZE;
		/* Update channel group BitMap */
		prRxFIGetEvent->u2BitMap = le2cpu16(prRxFIGetEvent->u2BitMap);
		BitMap = prRxFIGetEvent->u2BitMap;
		for (i = 0; i < CHANNEL_GROUP_NUM*SCN_NUM*4; i++)
			prRxFIGetEvent->au4Data[i] = le2cpu32(prRxFIGetEvent->au4Data[i]);
	}
	break;

	case PRECAL_RXFD: {
		RXFD_CAL_INFO_T *prRxFDGetEvent = (RXFD_CAL_INFO_T *)Data;
		/* Store header */
		HeaderSize = (UINT32)(uintptr_t)&((RXFD_CAL_INFO_T *)NULL)->au4Data[0];
		/* Update Cal data size */
		CalDataSize = RXFD_PER_GROUP_DATA_SIZE;
		/* Update channel group BitMap */
		prRxFDGetEvent->u2BitMap = le2cpu16(prRxFDGetEvent->u2BitMap);
		BitMap = prRxFDGetEvent->u2BitMap;
		/* Update group ID */
		prRxFDGetEvent->u4ChGroupId = le2cpu32(prRxFDGetEvent->u4ChGroupId);
		ChGroupId = prRxFDGetEvent->u4ChGroupId;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("\x1b[33m%s: RXFD ChGroupId %d\x1b[m\n", __FUNCTION__, ChGroupId));
		for (i = 0;
		i < (SCN_NUM*RX_SWAGC_LNA_NUM)+(SCN_NUM*RX_FDIQ_LPF_GAIN_NUM*RX_FDIQ_TABLE_SIZE*3);
		i++)
			prRxFDGetEvent->au4Data[i] = le2cpu32(prRxFDGetEvent->au4Data[i]);
	}
	break;

	default:
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("\x1b[41m %s: Not support this calibration item !!!! \x1b[m\n", __FUNCTION__));
		break;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("\x1b[33m%s: EventID = %d, Bitmp = %x\x1b[m\n", __FUNCTION__, EventId, BitMap));
	/* Update offset parameter */
	IDOffset = LenOffset = Offset = pAd->PreCalWriteOffSet;
	LenOffset += 4; /*Skip ID field*/
	Offset += 8; /*Skip (ID + Len) field*/

	if (pAd->PreCalStoreBuffer != NULL) {
		/* Store ID */
		os_move_mem(pAd->PreCalStoreBuffer + IDOffset, &EventId, sizeof(EventId));
		/* Store header */
		os_move_mem(pAd->PreCalStoreBuffer + Offset, Data, (ULONG)HeaderSize);
		Offset += HeaderSize; /*Skip header*/
		Data += HeaderSize; /*Skip header*/

		/* Store pre-cal data */
		if (EventId == PRECAL_RXFD) {
			if ((rf_count < rf_countMax) && (BitMap & (1 << ChGroupId))) {
				rf_count++;
				os_move_mem(pAd->PreCalStoreBuffer + Offset, Data, CalDataSize);
				Offset += CalDataSize;
			}
		} else {
			int count = 0;
			int countMax = pAd->ChGrpMap == 0x1FF ? 8 : 5;

			for (i = 0; i < CHANNEL_GROUP_NUM; i++) {
				if (count > countMax)
					break;

				if (BitMap & (1 << i)) {
					count++;
					os_move_mem(pAd->PreCalStoreBuffer + Offset, Data + i * CalDataSize, CalDataSize);
					Offset += CalDataSize;
				}
			}
		}

		/* Update current temp buffer write-offset */
		pAd->PreCalWriteOffSet = Offset;
		/* Calculate buffer size (len + header + data) for each calibration item */
		Length = Offset - LenOffset;
		/* Store buffer size for each calibration item */
		os_move_mem(pAd->PreCalStoreBuffer + LenOffset, &Length, sizeof(Length));

		/* The last event ID - update calibration data to flash */
		if (EventId == PRECAL_RXFI) {
			TotalSize = pAd->PreCalWriteOffSet;
#ifdef RTMP_FLASH_SUPPORT
			/* Write pre-cal data to flash */
			if (pAd->E2pAccessMode == E2P_FLASH_MODE)
				RtmpFlashWrite(pAd->hdev_ctrl, pAd->PreCalStoreBuffer,
					get_dev_eeprom_offset(pAd) + PRECALPART_OFFSET, TotalSize);
#endif/* RTMP_FLASH_SUPPORT */
			if (pAd->E2pAccessMode == E2P_BIN_MODE)
				rtmp_cal_write_to_bin(pAd, pAd->PreCalStoreBuffer, PRECALPART_OFFSET, TotalSize);

			/* Raise DoPreCal bits */
			if (pAd->E2pAccessMode == E2P_FLASH_MODE || pAd->E2pAccessMode == E2P_BIN_MODE) {
				RT28xx_EEPROM_READ16(pAd, 0x52, DoPreCal);
				DoPreCal |= (1 << 2);
				RT28xx_EEPROM_WRITE16(pAd, 0x52, DoPreCal);
			}

			/* Reset parameter */
			pAd->PreCalWriteOffSet = 0;
			rf_count = 0;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 ("\x1b[41m %s: Pre-calibration done !! \x1b[m\n", __FUNCTION__));
		}
	} else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("\x1b[41m %s: PreCalStoreBuffer is NULL !! \x1b[m\n", __FUNCTION__));
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("\x1b[41m %s <------------ \x1b[m\n", __FUNCTION__));
	return TRUE;
}

NTSTATUS PreCalTxLPFStoreProcHandler(PRTMP_ADAPTER pAd, PCmdQElmt CMDQelmt)
{
	ExtEventPreCalStoreProc(pAd, PRECAL_TXLPF, CMDQelmt->buffer);
	return 0;
}

NTSTATUS PreCalTxIQStoreProcHandler(PRTMP_ADAPTER pAd, PCmdQElmt CMDQelmt)
{
	ExtEventPreCalStoreProc(pAd, PRECAL_TXIQ, CMDQelmt->buffer);
	return 0;
}

NTSTATUS PreCalTxDCStoreProcHandler(PRTMP_ADAPTER pAd, PCmdQElmt CMDQelmt)
{
	ExtEventPreCalStoreProc(pAd, PRECAL_TXDC, CMDQelmt->buffer);
	return 0;
}

NTSTATUS PreCalRxFIStoreProcHandler(PRTMP_ADAPTER pAd, PCmdQElmt CMDQelmt)
{
	ExtEventPreCalStoreProc(pAd, PRECAL_RXFI, CMDQelmt->buffer);
	return 0;
}

NTSTATUS PreCalRxFDStoreProcHandler(PRTMP_ADAPTER pAd, PCmdQElmt CMDQelmt)
{
	ExtEventPreCalStoreProc(pAd, PRECAL_RXFD, CMDQelmt->buffer);
	return 0;
}
static VOID ExtEventTxLPFCalInfoHandler(
	RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	hex_dump("P_TXLPF_CAL_INFO", Data, Length);

	if (RLM_PRECAL_TXLPF_TO_FLASH_CHECK(Data)) {
		/* Store pre-cal data to flash */
		RTEnqueueInternalCmd(pAd, CMDTHRED_PRECAL_TXLPF, (VOID *)Data, Length);
	} else {
		RlmCalCacheTxLpfInfo(pAd->rlmCalCache, Data, Length);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("\x1b[42m%s: Not store to flash !! \x1b[m\n", __FUNCTION__));
	}

	return;
};

static VOID ExtEventTxIQCalInfoHandler(
	RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	hex_dump("P_TXIQ_CAL_INFO", Data, Length);

	if (RLM_PRECAL_TXIQ_TO_FLASH_CHECK(Data)) {
		/* Store pre-cal data to flash */
		RTEnqueueInternalCmd(pAd, CMDTHRED_PRECAL_TXIQ, (VOID *)Data, Length);
	} else {
		RlmCalCacheTxIqInfo(pAd->rlmCalCache, Data, Length);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("\x1b[42m%s: Not store to flash !! \x1b[m\n", __FUNCTION__));
	}

	return;
};

static VOID ExtEventTxDCCalInfoHandler(
	RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	hex_dump("P_TXDC_CAL_INFO", Data, Length);

	if (RLM_PRECAL_TXDC_TO_FLASH_CHECK(Data)) {
		/* Store pre-cal data to flash */
		RTEnqueueInternalCmd(pAd, CMDTHRED_PRECAL_TXDC, (VOID *)Data, Length);
	} else {
		RlmCalCacheTxDcInfo(pAd->rlmCalCache, Data, Length);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("\x1b[42m%s: Not store to flash !! \x1b[m\n", __FUNCTION__));
	}

	return;
};

static VOID ExtEventRxFICalInfoHandler(
	RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	hex_dump("P_RXFI_CAL_INFO", Data, Length);

	if (RLM_PRECAL_RXFI_TO_FLASH_CHECK(Data)) {
		/* Store pre-cal data to flash */
		RTEnqueueInternalCmd(pAd, CMDTHRED_PRECAL_RXFI, (VOID *)Data, Length);
	} else {
		RlmCalCacheRxFiInfo(pAd->rlmCalCache, Data, Length);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("\x1b[42m%s: Not store to flash !! \x1b[m\n", __FUNCTION__));
	}

	return;
};

static VOID ExtEventRxFDCalInfoHandler(
	RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	hex_dump("P_RXFD_CAL_INFO", Data, Length);

	if (RLM_PRECAL_RXFD_TO_FLASH_CHECK(Data)) {
		/* Store pre-cal data to flash */
		RTEnqueueInternalCmd(pAd, CMDTHRED_PRECAL_RXFD, (VOID *)Data, Length);
	} else {
		RlmCalCacheRxFdInfo(pAd->rlmCalCache, Data, Length);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("\x1b[42m%s: Not store to flash !! \x1b[m\n", __FUNCTION__));
	}

	return;
};
#endif  /* defined(RLM_CAL_CACHE_SUPPORT) || defined(PRE_CAL_TRX_SET2_SUPPORT) */

#ifdef RACTRL_FW_OFFLOAD_SUPPORT
static VOID event_get_tx_statistic_handle(struct _RTMP_ADAPTER *pAd, UINT8 *data, UINT32 Length)
{
	struct _EXT_EVENT_TX_STATISTIC_RESULT_T *event =
		(struct _EXT_EVENT_TX_STATISTIC_RESULT_T *)data;
	UCHAR wcid = event->ucWlanIdx;
	struct _MAC_TABLE_ENTRY *entry;
	struct _STA_TR_ENTRY *tr_entry;
	UINT32 tx_success;

	if (!VALID_UCAST_ENTRY_WCID(pAd, wcid))
		return;

	entry = &pAd->MacTab.Content[wcid];

	if (IS_ENTRY_NONE(entry))
		return;

	tr_entry = &pAd->MacTab.tr_entry[entry->tr_tb_idx];

	if (tr_entry->StaRec.ConnectionState != STATE_PORT_SECURE)
		return;

	if (entry->TxStatRspCnt) {
		tx_success = event->u4EntryTxCount - event->u4EntryTxFailCount;
		entry->TotalTxSuccessCnt += tx_success;
		entry->one_sec_tx_succ_pkts = tx_success;
	}

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		("%s(): wcid(%d), TotalTxCnt(%u) - TotalTxFail(%u) = %u (%s)\n",
		__func__,
		wcid,
		event->u4EntryTxCount,
		event->u4EntryTxFailCount,
		entry->TotalTxSuccessCnt,
		(entry->TxStatRspCnt) ? "Valid" : "Invalid"));
	entry->TxStatRspCnt++;
}
#endif /*RACTRL_FW_OFFLOAD_SUPPORT*/

static VOID EventExtEventHandler(RTMP_ADAPTER *pAd, UINT8 ExtEID, UINT8 *Data,
								 UINT32 Length, EVENT_RXD *event_rxd)
{
	switch (ExtEID) {
	case EXT_EVENT_CMD_RESULT:
		EventExtCmdResult(NULL, Data, Length);
		break;

	case EXT_EVENT_ID_PS_SYNC:
		ExtEventPsSyncHandler(pAd, Data, Length);
		break;

	case EXT_EVENT_FW_LOG_2_HOST:
		ExtEventFwLog2HostHandler(pAd, Data, Length, event_rxd);
		break;
#ifdef COEX_SUPPORT

	case EXT_EVENT_BT_COEX:
		ExtEventBTCoexHandler(pAd, Data, Length);
		break;
#endif /* COEX_SUPPORT */
#ifdef THERMAL_PROTECT_SUPPORT

	case EXT_EVENT_THERMAL_PROTECT:
		EventThermalProtect(pAd, Data, Length);
		break;
#endif
#if defined(PRETBTT_INT_EVENT_SUPPORT) || defined(BCN_OFFLOAD_SUPPORT)

	case EXT_EVENT_PRETBTT_INT:
		ExtEventPretbttIntHandler(pAd, Data, Length);
		break;
#endif /*PRETBTT_INT_EVENT_SUPPORT*/

	case EXT_EVENT_BEACON_LOSS:
		ExtEventBeaconLostHandler(pAd, Data, Length);
		break;
#ifdef RACTRL_FW_OFFLOAD_SUPPORT

	case EXT_EVENT_RA_THROUGHPUT_BURST:
		ExtEventThroughputBurst(pAd, Data, Length);
		break;

	case EXT_EVENT_G_BAND_256QAM_PROBE_RESULT:
		ExtEventGBand256QamProbeResule(pAd, Data, Length);
		break;
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */

	case EXT_EVENT_ID_ASSERT_DUMP:
		ExtEventAssertDumpHandler(pAd, Data, Length, event_rxd);
		break;
#ifdef CFG_TDLS_SUPPORT

	case EXT_EVENT_TDLS_STATUS:
		ExtEventTdlsBackToBaseHandler(pAd, Data, Length);
		break;
#endif
#if defined(MT_MAC) && defined(TXBF_SUPPORT)

	case EXT_EVENT_ID_BF_STATUS_READ:
		/* ExtEventFwLog2HostHandler(pAd, Data, Length); */
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("%s: EXT_EVENT_ID_BF_STATUS_READ\n", __func__));
		ExtEventBfStatusRead(pAd, Data, Length);
		break;
#endif /* MT_MAC && TXBF_SUPPORT */
#ifdef CONFIG_ATE

	case EXT_EVENT_ID_RF_TEST:
		MT_ATERFTestCB(pAd, Data, Length);
		break;
#endif
#ifdef MT_DFS_SUPPORT

	case EXT_EVENT_ID_RDD_REPORT:
		ExtEventRddReportHandler(pAd, Data, Length);
		break;
#endif

	case EXT_EVENT_ID_MAX_AMSDU_LENGTH_UPDATE:
		ExtEventMaxAmsduLengthUpdate(pAd, Data, Length);
		break;

	case EXT_EVENT_ID_BA_TRIGGER:
		ExtEventBaTriggerHandler(pAd, Data, Length);
		break;

#ifdef WIFI_SPECTRUM_SUPPORT
	case EXT_EVENT_ID_WIFI_SPECTRUM:
		ExtEventWifiSpectrumHandler(pAd, Data, Length);
		break;
#endif /* WIFI_SPECTRUM_SUPPORT */

	case EXT_EVENT_CSA_NOTIFY:
		ExtEventCswNotifyHandler(pAd, Data, Length);
		break;

	case EXT_EVENT_TMR_CALCU_INFO:
		ExtEventTmrCalcuInfoHandler(pAd, Data, Length);
		break;

	case EXT_EVENT_ID_BSS_ACQ_PKT_NUM:
		ExtEventBssAcQPktNumHandler(pAd, Data, Length);
		break;
#if defined (RLM_CAL_CACHE_SUPPORT) || defined(PRE_CAL_TRX_SET2_SUPPORT)

	case EXT_EVENT_ID_TXLPF_CAL_INFO:
		ExtEventTxLPFCalInfoHandler(pAd, Data, Length);
		break;

	case EXT_EVENT_ID_TXIQ_CAL_INFO:
		ExtEventTxIQCalInfoHandler(pAd, Data, Length);
		break;

	case EXT_EVENT_ID_TXDC_CAL_INFO:
		ExtEventTxDCCalInfoHandler(pAd, Data, Length);
		break;

	case EXT_EVENT_ID_RXFI_CAL_INFO:
		ExtEventRxFICalInfoHandler(pAd, Data, Length);
		break;

	case EXT_EVENT_ID_RXFD_CAL_INFO:
		ExtEventRxFDCalInfoHandler(pAd, Data, Length);
		break;
#endif /* defined(RLM_CAL_CACHE_SUPPORT) || defined(PRE_CAL_TRX_SET2_SUPPORT) */

	case EXT_EVENT_ID_TX_POWER_FEATURE_CTRL:
		EventTxPowerHandler(pAd, Data, Length);
		break;
#ifdef CONFIG_HOTSPOT_R2

	case EXT_EVENT_ID_INFORM_HOST_REPROCESS_PKT:
		ExtEventReprocessPktHandler(pAd, Data, Length);
		break;

	case EXT_EVENT_ID_GET_CR4_HOTSPOT_CAPABILITY:
		ExtEventGetHotspotCapabilityHandler(pAd, Data, Length);
		break;
#endif /* CONFIG_HOTSPOT_R2 */
#if defined(RED_SUPPORT) && (defined(MT7622) || defined(P18)	\
	|| defined(MT7663) || defined(MT7615))

	case EXT_EVENT_ID_MPDU_TIME_UPDATE:
		ExtEventMpduTimeHandler(pAd, Data, Length);
		break;
#endif /* defined(RED_SUPPORT) && (defined(MT7622) || defined(P18) || defined(MT7663)) */

	case EXT_EVENT_GET_CR4_TX_STATISTICS:
		ext_event_get_cr4_tx_statistics(pAd, Data, Length);
		break;
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
	case EXT_EVENT_GET_TX_STATISTIC:
		event_get_tx_statistic_handle(pAd, Data, Length);
		break;
#endif /*RACTRL_FW_OFFLOAD_SUPPORT*/
	default:
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("%s: Unknown Ext Event(%x)\n", __func__, ExtEID));
		break;
	}
}

static VOID EventExtGenericEventHandler(UINT8 *Data)
{
	struct _EVENT_EXT_CMD_RESULT_T *EventExtCmdResult =
		(struct _EVENT_EXT_CMD_RESULT_T *)Data;
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s: EventExtCmdResult.ucExTenCID = 0x%x\n",
			  __func__, EventExtCmdResult->ucExTenCID));
	EventExtCmdResult->u4Status = le2cpu32(EventExtCmdResult->u4Status);

	if (EventExtCmdResult->u4Status == CMD_RESULT_SUCCESS) {
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 ("%s: CMD Success\n", __func__));
	} else if (EventExtCmdResult->u4Status == CMD_RESULT_NONSUPPORT) {
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 ("%s: CMD Non-Support\n", __func__));
	} else {
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 ("%s: CMD Fail!, EventExtCmdResult.u4Status = 0x%x\n",
				  __func__, EventExtCmdResult->u4Status));
	}
}

static VOID EventGenericEventHandler(UINT8 *Data)
{
	struct _INIT_EVENT_CMD_RESULT *EventCmdResult =
		(struct _INIT_EVENT_CMD_RESULT *)Data;
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s: EventCmdResult.ucCID = 0x%x\n",
			  __func__, EventCmdResult->ucCID));

	if (EventCmdResult->ucStatus == CMD_RESULT_SUCCESS) {
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("%s: CMD Success\n", __func__));
	} else if (EventCmdResult->ucStatus == CMD_RESULT_NONSUPPORT) {
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("%s: CMD Non-Support\n", __func__));
	} else {
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("%s: CMD Fail!, EventCmdResult.ucStatus = 0x%x\n",
				  __func__, EventCmdResult->ucStatus));
	}
}


static VOID GenericEventHandler(UINT8 EID, UINT8 ExtEID, UINT8 *Data)
{
	switch (EID) {
	case EXT_EVENT:
		EventExtGenericEventHandler(Data);
		break;

	case GENERIC_EVENT:
		EventGenericEventHandler(Data);
		break;

	default:
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("%s: Unknown Event(%x)\n", __func__, EID));
		break;
	}
}

static VOID UnsolicitedEventHandler(RTMP_ADAPTER *pAd, UINT8 EID, UINT8 ExtEID,
									UINT8 *Data, UINT32 Length, EVENT_RXD *event_rxd)
{
	switch (EID) {
	case EVENT_CH_PRIVILEGE:
		EventChPrivilegeHandler(pAd, Data, Length);
		break;

	case EXT_EVENT:
		EventExtEventHandler(pAd, ExtEID, Data, Length, event_rxd);
		break;

	default:
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("%s: Unknown Event(%x)\n", __func__, EID));
		break;
	}
}


static BOOLEAN IsRspLenVariableAndMatchSpecificMinLen(EVENT_RXD *event_rxd,
		struct cmd_msg *msg)
{
	if ((msg->attr.ctrl.expect_size <= GET_EVENT_HDR_ADD_PAYLOAD_TOTAL_LEN(event_rxd))
		&& (msg->attr.ctrl.expect_size != 0) && IS_CMD_MSG_LEN_VAR_FLAG_SET(msg))
		return TRUE;
	else
		return FALSE;
}



static BOOLEAN IsRspLenNonZeroAndMatchExpected(EVENT_RXD *event_rxd,
		struct cmd_msg *msg)
{
	if ((msg->attr.ctrl.expect_size == GET_EVENT_HDR_ADD_PAYLOAD_TOTAL_LEN(event_rxd))
		&& (msg->attr.ctrl.expect_size != 0))
		return TRUE;
	else
		return FALSE;
}

static VOID HandlSeq0AndOtherUnsolicitedEvents(RTMP_ADAPTER *pAd,
		EVENT_RXD *event_rxd, PNDIS_PACKET net_pkt)
{
	UnsolicitedEventHandler(pAd,
							GET_EVENT_FW_RXD_EID(event_rxd),
							GET_EVENT_FW_RXD_EXT_EID(event_rxd),
							GET_EVENT_HDR_ADDR(net_pkt),
							GET_EVENT_HDR_ADD_PAYLOAD_TOTAL_LEN(event_rxd), event_rxd);
}


static void CompleteWaitCmdMsgOrFreeCmdMsg(struct cmd_msg *msg)
{
	if (IS_CMD_MSG_NEED_SYNC_WITH_FW_FLAG_SET(msg))
		RTMP_OS_COMPLETE(&msg->ack_done);
	else {
		DlListDel(&msg->list);
		AndesFreeCmdMsg(msg);
	}
}

static void FillRspPayloadLenAndDumpExpectLenAndRspLenInfo(
	EVENT_RXD *event_rxd, struct cmd_msg *msg)
{
	/* Error occurs!!! dump info for debugging */
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("expect response len(%d), command response len(%zd) invalid\n",
			  msg->attr.ctrl.expect_size, GET_EVENT_HDR_ADD_PAYLOAD_TOTAL_LEN(event_rxd)));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s:cmd_type = 0x%x, ext_cmd_type = 0x%x, FW_RXD_EXT_EID = 0x%x\n",
			  __func__, msg->attr.type, msg->attr.ext_type,
			  GET_EVENT_FW_RXD_EXT_EID(event_rxd)));
	msg->attr.ctrl.expect_size = GET_EVENT_HDR_ADD_PAYLOAD_TOTAL_LEN(event_rxd);
}


static VOID HandleLayer1GenericEvent(UINT8 EID, UINT8 ExtEID, UINT8 *Data)
{
	GenericEventHandler(EID, ExtEID, Data);
}

static void CallEventHookHandlerOrDumpErrorMsg(EVENT_RXD *event_rxd,
		struct cmd_msg *msg, PNDIS_PACKET net_pkt)
{
	if (msg->attr.rsp.handler == NULL) {
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s(): rsp_handler is NULL!!!!(cmd_type = 0x%x, ext_cmd_type = 0x%x, FW_RXD_EXT_EID = 0x%x)\n",
				  __func__, msg->attr.type, msg->attr.ext_type,
				  GET_EVENT_FW_RXD_EXT_EID(event_rxd)));

		if (GET_EVENT_FW_RXD_EXT_EID(event_rxd) == 0) {
			HandleLayer1GenericEvent(GET_EVENT_FW_RXD_EID(event_rxd),
									 GET_EVENT_FW_RXD_EXT_EID(event_rxd),
									 GET_EVENT_HDR_ADDR(net_pkt));
		}
	} else {
		msg->attr.rsp.handler(msg, GET_EVENT_HDR_ADDR(net_pkt),
							  GET_EVENT_HDR_ADD_PAYLOAD_TOTAL_LEN(event_rxd));
	}
}

static void FwDebugPurposeHandler(EVENT_RXD *event_rxd,
								  struct cmd_msg *msg, PNDIS_PACKET net_pkt)
{
	/* hanle FW debug purpose only */
	CallEventHookHandlerOrDumpErrorMsg(event_rxd, msg, net_pkt);
}

static VOID HandleNormalLayer1Events(EVENT_RXD *event_rxd,
									 struct cmd_msg *msg, PNDIS_PACKET net_pkt)
{
	CallEventHookHandlerOrDumpErrorMsg(event_rxd, msg, net_pkt);
}

static void EventLenVariableHandler(EVENT_RXD *event_rxd,
									struct cmd_msg *msg, PNDIS_PACKET net_pkt)
{
	/* hanle event len variable */
	HandleNormalLayer1Events(event_rxd, msg, net_pkt);
}


static void HandleLayer1Events(EVENT_RXD *event_rxd,
							   struct cmd_msg *msg, PNDIS_PACKET net_pkt)
{
	/* handler normal layer 1 event */
	if (IsRspLenNonZeroAndMatchExpected(event_rxd, msg))
		HandleNormalLayer1Events(event_rxd, msg, net_pkt);
	else if (IsRspLenVariableAndMatchSpecificMinLen(event_rxd, msg)) {
		/* hanle event len variable */
		EventLenVariableHandler(event_rxd, msg, net_pkt);
	} else if (IS_IGNORE_RSP_PAYLOAD_LEN_CHECK(msg)) {
		/* hanle FW debug purpose only */
		FwDebugPurposeHandler(event_rxd, msg, net_pkt);
	} else
		FillRspPayloadLenAndDumpExpectLenAndRspLenInfo(event_rxd, msg);
}

static VOID HandleLayer0GenericEvent(UINT8 EID, UINT8 ExtEID, UINT8 *Data)
{
	GenericEventHandler(EID, ExtEID, Data);
}

static BOOLEAN IsNormalLayer0Events(EVENT_RXD *event_rxd)
{
	if ((GET_EVENT_FW_RXD_EID(event_rxd) == MT_FW_START_RSP)            ||
		(GET_EVENT_FW_RXD_EID(event_rxd) == MT_RESTART_DL_RSP)          ||
		(GET_EVENT_FW_RXD_EID(event_rxd) == MT_TARGET_ADDRESS_LEN_RSP)  ||
		(GET_EVENT_FW_RXD_EID(event_rxd) == MT_PATCH_SEM_RSP)           ||
		(GET_EVENT_FW_RXD_EID(event_rxd) == EVENT_ACCESS_REG))
		return TRUE;
	else
		return FALSE;
}
static void HandleLayer0Events(EVENT_RXD *event_rxd,
							   struct cmd_msg *msg, PNDIS_PACKET net_pkt)
{
	/* handle layer0 generic event */
	if (GET_EVENT_FW_RXD_EID(event_rxd) == GENERIC_EVENT) {
		HandleLayer0GenericEvent(GET_EVENT_FW_RXD_EID(event_rxd),
								 GET_EVENT_FW_RXD_EXT_EID(event_rxd),
								 GET_EVENT_HDR_ADDR(net_pkt) - 4);
	} else {
		/* handle normal layer0 event */
		if (IsNormalLayer0Events(event_rxd)) {
#ifdef RT_BIG_ENDIAN
			event_rxd->fw_rxd_2.word = cpu2le32(event_rxd->fw_rxd_2.word);
#endif
			msg->attr.rsp.handler(msg, GET_EVENT_HDR_ADDR(net_pkt) - 4,
								  GET_EVENT_HDR_ADD_PAYLOAD_TOTAL_LEN(event_rxd) + 4);
		} else if (IsRspLenVariableAndMatchSpecificMinLen(event_rxd, msg)) {
			/* hanle event len is variable */
			EventLenVariableHandler(event_rxd, msg, net_pkt);
		} else if (IS_IGNORE_RSP_PAYLOAD_LEN_CHECK(msg)) {
			/* hanle FW debug purpose only */
			FwDebugPurposeHandler(event_rxd, msg, net_pkt);
		} else
			FillRspPayloadLenAndDumpExpectLenAndRspLenInfo(event_rxd, msg);
	}
}

static VOID GetMCUCtrlAckQueueSpinLock(struct MCU_CTRL **ctl,
									   unsigned long *flags)
{
#if defined(RTMP_PCI_SUPPORT) || defined(RTMP_RBUS_SUPPORT)
	RTMP_SPIN_LOCK_IRQSAVE(&((*ctl)->ackq_lock), flags);
#endif
}

static VOID ReleaseMCUCtrlAckQueueSpinLock(struct MCU_CTRL **ctl,
		unsigned long *flags)
{
#if defined(RTMP_PCI_SUPPORT) || defined(RTMP_RBUS_SUPPORT)
	RTMP_SPIN_UNLOCK_IRQRESTORE(&((*ctl)->ackq_lock), flags);
#endif
}

static UINT8 GetEventFwRxdSequenceNumber(EVENT_RXD *event_rxd)
{
	return (UINT8)(GET_EVENT_FW_RXD_SEQ_NUM(event_rxd));
}
static VOID HandleSeqNonZeroNormalEvents(RTMP_ADAPTER *pAd,
		EVENT_RXD *event_rxd, PNDIS_PACKET net_pkt)
{
	UINT8 peerSeq;
	struct cmd_msg *msg, *msg_tmp;
	struct MCU_CTRL *ctl = &pAd->MCUCtrl;
	unsigned long flags = 0;
	GetMCUCtrlAckQueueSpinLock(&ctl, &flags);
	DlListForEachSafe(msg, msg_tmp, &ctl->ackq, struct cmd_msg, list) {
		peerSeq = GetEventFwRxdSequenceNumber(event_rxd);
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				 ("%s: msg->seq=%x, field.seq_num=%x, msg->attr.ctrl.expect_size=%d\n",
				  __func__, msg->seq, peerSeq, msg->attr.ctrl.expect_size));

		if (msg->seq == peerSeq) {
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 ("%s (seq=%d)\n", __func__, msg->seq));
			ReleaseMCUCtrlAckQueueSpinLock(&ctl, &flags);
			msg->receive_time_in_jiffies = jiffies;
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 ("%s: CMD_ID(0x%x 0x%x),total spent %ld ms\n", __func__,
					  msg->attr.type, msg->attr.ext_type, ((msg->receive_time_in_jiffies - msg->sending_time_in_jiffies) * 1000 / OS_HZ)));

			if (GET_EVENT_FW_RXD_EID(event_rxd) == EXT_EVENT)
				HandleLayer1Events(event_rxd, msg, net_pkt);
			else
				HandleLayer0Events(event_rxd, msg, net_pkt);

			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					 ("%s: need_wait=%d\n", __func__,
					  IS_CMD_MSG_NEED_SYNC_WITH_FW_FLAG_SET(msg)));
			CompleteWaitCmdMsgOrFreeCmdMsg(msg);
			GetMCUCtrlAckQueueSpinLock(&ctl, &flags);
			break;
		}
	}
	ReleaseMCUCtrlAckQueueSpinLock(&ctl, &flags);
}

static VOID AndesMTRxProcessEvent(RTMP_ADAPTER *pAd, struct cmd_msg *rx_msg)
{
	PNDIS_PACKET net_pkt = rx_msg->net_pkt;
	EVENT_RXD *event_rxd = (EVENT_RXD *)GET_OS_PKT_DATAPTR(net_pkt);
#ifdef CONFIG_TRACE_SUPPORT
	TRACE_MCU_EVENT_INFO(GET_EVENT_FW_RXD_LENGTH(event_rxd),
						 GET_EVENT_FW_RXD_PKT_TYPE_ID(event_rxd),
						 GET_EVENT_FW_RXD_EID(event_rxd),
						 GET_EVENT_FW_RXD_SEQ_NUM(event_rxd),
						 GET_EVENT_FW_RXD_EXT_EID(event_rxd),
						 GET_EVENT_HDR_ADDR(net_pkt),
						 GET_EVENT_HDR_ADD_PAYLOAD_TOTAL_LEN(event_rxd));
#endif
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s: seq_num=%d, ext_eid=%x\n", __func__,
			  GET_EVENT_FW_RXD_SEQ_NUM(event_rxd),
			  GET_EVENT_FW_RXD_EXT_EID(event_rxd)));

	if (IsUnsolicitedEvent(event_rxd))
		HandlSeq0AndOtherUnsolicitedEvents(pAd, event_rxd, net_pkt);
	else
		HandleSeqNonZeroNormalEvents(pAd, event_rxd, net_pkt);
}


VOID AndesMTRxEventHandler(RTMP_ADAPTER *pAd, UCHAR *data)
{
	struct cmd_msg *msg;
	struct MCU_CTRL *ctl = &pAd->MCUCtrl;
	EVENT_RXD *event_rxd = (EVENT_RXD *)data;

	if (!OS_TEST_BIT(MCU_INIT, &ctl->flags))
		return;

#ifdef RT_BIG_ENDIAN
	event_rxd->fw_rxd_0.word = le2cpu32(event_rxd->fw_rxd_0.word);
	event_rxd->fw_rxd_1.word = le2cpu32(event_rxd->fw_rxd_1.word);
	event_rxd->fw_rxd_2.word = le2cpu32(event_rxd->fw_rxd_2.word);
#endif
	msg = AndesAllocCmdMsg(pAd, GET_EVENT_FW_RXD_LENGTH(event_rxd));

	if (!msg)
		return;

	AndesAppendCmdMsg(msg, (char *)data, GET_EVENT_FW_RXD_LENGTH(event_rxd));
	AndesMTRxProcessEvent(pAd, msg);
#if defined(RTMP_PCI_SUPPORT) || defined(RTMP_RBUS_SUPPORT)

	if (msg->net_pkt)
		RTMPFreeNdisPacket(pAd, msg->net_pkt);

#endif
	AndesFreeCmdMsg(msg);
}


#if defined(RTMP_PCI_SUPPORT) || defined(RTMP_RBUS_SUPPORT)
VOID AndesMTPciFwInit(RTMP_ADAPTER *pAd)
{
	struct MCU_CTRL *Ctl = &pAd->MCUCtrl;
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __func__));
	Ctl->fwdl_ctrl.stage = FWDL_STAGE_FW_NOT_DL;
#if defined(MT7615)

	if (MTK_REV_GTE(pAd, MT7615, MT7615E3))
		EnhancedPDMAInit(pAd);

#endif
	/* Enable Interrupt*/
	RTMP_IRQ_ENABLE(pAd);
	RT28XXDMAEnable(pAd);
	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_START_UP);
	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_MCU_SEND_IN_BAND_CMD);
}


VOID AndesMTPciFwExit(RTMP_ADAPTER *pAd)
{
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __func__));
	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_START_UP);
	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_MCU_SEND_IN_BAND_CMD);
	RT28XXDMADisable(pAd);
	RTMP_ASIC_INTERRUPT_DISABLE(pAd);
}
#endif /* defined(RTMP_PCI_SUPPORT) || defined(RTMP_RBUS_SUPPORT) */





VOID EventTxPowerHandler(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	UINT8  ucEventCategoryID;

	/* Get Event Category ID */
	ucEventCategoryID = *Data;

	/* Event Handle for different Category ID */
	switch (ucEventCategoryID) {
	case TXPOWER_EVENT_SHOW_INFO:
		EventTxPowerShowInfo(pAd, Data, Length);
		break;

	case TXPOWER_EVENT_UPDATE_COMPENSATE_TABLE:
		EventTxPowerCompTable(pAd, Data, Length);
		break;

	case TXPOWER_EVENT_UPDATE_EPA_STATUS:
		EventTxPowerEPAInfo(pAd, Data, Length);
		break;

	case TXPOWER_EVENT_THERMAL_SENSOR_SHOW_INFO:
		EventThermalStateShowInfo(pAd, Data, Length);
		break;

	case TXPOWER_EVENT_POWER_BACKUP_TABLE_SHOW_INFO:
		EventPowerTableShowInfo(pAd, Data, Length);
		break;

	case TXPOWER_EVENT_THERMAL_COMPENSATE_TABLE_SHOW_INFO:
		EventThermalCompTableShowInfo(pAd, Data, Length);
		break;

	case TXPOWER_EVENT_TXV_BBP_POWER_SHOW_INFO:
		EventTxvBbpPowerInfo(pAd, Data, Length);
		break;

	default:
		break;
	}
}

VOID EventTxPowerShowInfo(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	P_EXT_EVENT_TXPOWER_INFO_T prEventTxPowerInfo;
	UINT8    TMACIdx, LoopIdx, RatePowerTableIdx, ColumnIdx, BFBackoffIdx, BBPCRIdx;
	UINT8    Value;
	UINT8    ucBandIdx;
	BOOLEAN  fg2GEPA;
	BOOLEAN  fg5GEPA;
	BOOLEAN  fgSKUEnable;
	BOOLEAN  fgPERCENTAGEEnable;
	BOOLEAN  fgBFBACKOFFEnable;
	BOOLEAN  fgThermalCompEnable;
	INT8     cSKUTable[SKU_TABLE_SIZE_ALL];
	INT8     cThermalCompValue;
	INT8     cPowerDrop;
	UINT32   u4RatePowerCRValue[RATE_POWER_TMAC_SIZE];
	INT8     cTxPwrBFBackoffValue[BF_BACKOFF_MODE][BF_BACKOFF_CASE];
	UINT32   u4BackoffCRValue[6];
	UINT32   u4PowerBoundCRValue;
	UINT32   u4BBPAddress;
	UINT8    ucPwrBoundIdx;
	UINT8    ucChannelBandIdx;
	TX_POWER_BOUND_TABLE_T TxPowerBoundTable[4] = {
		{ MAX_POWER_BAND0, 0, 0},
		{ MIN_POWER_BAND0, 0, 0},
		{ MAX_POWER_BAND1, 0, 0},
		{ MIN_POWER_BAND1, 0, 0}
	};
	TX_RATE_POWER_TABLE_T  TxRatePowerTable[30] = {
		{ OFDM_48M, 0, 0},
		{ OFDM_24M_36M, 0, 0},
		{ OFDM_12M_18M, 0, 0},
		{ OFDM_6M_9M, 0, 0},
		{ HT20_MCS5, 0, 0},
		{ HT20_MCS3_4, 0, 0},
		{ HT20_MCS1_2, 0, 0},
		{ HT20_MCS0, 0, 0},
		{ HT40_MCS5, 0, 0},
		{ HT40_MCS3_4, 0, 0},
		{ HT40_MCS1_2, 0, 0},
		{ HT40_MCS0, 0, 0},
		{ HT40_MCS32, 0, 0},
		{ CCK_5M11M, 0, 0},
		{ OFDM_54M, 0, 0},
		{ CCK_1M2M, 0, 0},
		{ HT40_MCS7, 0, 0},
		{ HT40_MCS6, 0, 0},
		{ HT20_MCS7, 0, 0},
		{ HT20_MCS6, 0, 0},
		{ VHT20_MCS5_6, 0, 0},
		{ VHT20_MCS3_4, 0, 0},
		{ VHT20_MCS1_2, 0, 0},
		{ VHT20_MCS0, 0, 0},
		{ VHT20_MCS9, 0, 0},
		{ VHT20_MCS8, 0, 0},
		{ VHT20_MCS7, 0, 0},
		{ VHT160, 0, 0},
		{ VHT80, 0, 0},
		{ VHT40, 0, 0}
	};

	prEventTxPowerInfo = (P_EXT_EVENT_TXPOWER_INFO_T)Data;
	for (TMACIdx = 0; TMACIdx < RATE_POWER_TMAC_SIZE; TMACIdx++)
		prEventTxPowerInfo->u4RatePowerCRValue[TMACIdx] = le2cpu32(prEventTxPowerInfo->u4RatePowerCRValue[TMACIdx]);

	prEventTxPowerInfo->u4PowerBoundCRValue	= le2cpu32(prEventTxPowerInfo->u4PowerBoundCRValue);

	/* get DBDC Band Info */
	ucBandIdx = prEventTxPowerInfo->ucBandIdx;

	/* get Channel Band Info */
	ucChannelBandIdx = prEventTxPowerInfo->ucChannelBandIdx;

	/* EPPROME Info */
	fg2GEPA = prEventTxPowerInfo->fg2GEPA;
	fg5GEPA = prEventTxPowerInfo->fg5GEPA;

	/* SKU enable/disable */
	fgSKUEnable = prEventTxPowerInfo->fgSKUEnable;

	/* Power Percentage enable/disable */
	fgPERCENTAGEEnable = prEventTxPowerInfo->fgPERCENTAGEEnable;
	cPowerDrop = prEventTxPowerInfo->cPowerDrop;

	/* BF Backoff enable/disable */
	fgBFBACKOFFEnable = prEventTxPowerInfo->fgBFBACKOFFEnable;

	/* SKU Info */
	os_move_mem(cSKUTable, prEventTxPowerInfo->cSKUTable, SKU_TABLE_SIZE_ALL);

	/* TMAC Info */
	os_move_mem(u4RatePowerCRValue, prEventTxPowerInfo->u4RatePowerCRValue, RATE_POWER_TMAC_SIZE * 4);

	/* Per Rate Tx Power for TMAC CR (Band0) */
	for (TMACIdx = 0, LoopIdx = 0, RatePowerTableIdx = 0; TMACIdx < RATE_POWER_TMAC_SIZE; TMACIdx++) {
		for (ColumnIdx = 0; ColumnIdx < CR_COLUMN_SIZE; ColumnIdx++, LoopIdx++) {
			Value = (UINT8)((u4RatePowerCRValue[TMACIdx] & BITS((3 - ColumnIdx) * 8, (3 - ColumnIdx) * 8 + 7)) >> ((3 - ColumnIdx) * 8));

			/* Update Power DAC value and Power Decimal value for valid CR value in TMAC column */
			if ((LoopIdx != 24) && (LoopIdx != 28)) {
				TxRatePowerTable[RatePowerTableIdx].CRValue = Value;

				if (TxRatePowerTable[RatePowerTableIdx].CRValue > 63)
					TxRatePowerTable[RatePowerTableIdx].PowerDecimal = TxRatePowerTable[RatePowerTableIdx].CRValue - 128;
				else
					TxRatePowerTable[RatePowerTableIdx].PowerDecimal = TxRatePowerTable[RatePowerTableIdx].CRValue;

				RatePowerTableIdx++;
			}
		}
	}

	/* BF Backoff Info */
	u4BBPAddress = (0 == ucBandIdx) ? (0x8207067C) : (0x8207087C);
	os_move_mem(cTxPwrBFBackoffValue, prEventTxPowerInfo->cTxPwrBFBackoffValue, BF_BACKOFF_MODE * BF_BACKOFF_CASE);
	os_move_mem(u4BackoffCRValue, prEventTxPowerInfo->u4BackoffCRValue, BFBACKOFF_BBPCR_SIZE * 4);

	/* Power Upper Bound Info */
	ucPwrBoundIdx = ucBandIdx << 1;
	u4PowerBoundCRValue = prEventTxPowerInfo->u4PowerBoundCRValue;       /* TMAC: 0x820F4080 */

	for (ColumnIdx = 0; ColumnIdx < CR_COLUMN_SIZE; ColumnIdx++) {
		Value = (UINT8)((u4PowerBoundCRValue & BITS((3 - ColumnIdx) * 8, (3 - ColumnIdx) * 8 + 7)) >> ((3 - ColumnIdx) * 8));
		TxPowerBoundTable[ColumnIdx].CRValue = Value;

		if (TxPowerBoundTable[ColumnIdx].CRValue > 63)
			TxPowerBoundTable[ColumnIdx].PowerDecimal = (INT8)TxPowerBoundTable[ColumnIdx].CRValue - 128;
		else
			TxPowerBoundTable[ColumnIdx].PowerDecimal = (INT8)TxPowerBoundTable[ColumnIdx].CRValue;
	}

	/* Thermal Compensation Info */
	fgThermalCompEnable = prEventTxPowerInfo->fgThermalCompEnable;
	cThermalCompValue = prEventTxPowerInfo->cThermalCompValue;
	/* Show Info in Debug Log */
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("============================================================================= \n"));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("                             EPPROME Info                                     \n"));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("============================================================================= \n"));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("  EPA_%s = %d                                                                 \n", (ucChannelBandIdx == 1) ? "5G" : "2G", (ucChannelBandIdx == 1) ? fg5GEPA : fg2GEPA));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("============================================================================= \n"));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("                                SKU Info                                      \n"));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("============================================================================= \n"));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("  SKU Enable status:  (Band%d): %d                                            \n", ucBandIdx, fgSKUEnable));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("----------------------------------------------------------------------------- \n"));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("         Band%d                                                               \n", ucBandIdx));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("         CCK_1M2M    = 0x%2x                                                 \n", cSKUTable[CCK1M2M]));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("         CCK_5M11M   = 0x%2x                                                 \n", cSKUTable[CCK5M11M]));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("         OFDM_6M9M   = 0x%2x                                                 \n", cSKUTable[OFDM6M9M]));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("         OFDM_12M18M = 0x%2x                                                 \n", cSKUTable[OFDM12M18M]));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("         OFDM_24M36M = 0x%2x                                                 \n", cSKUTable[OFDM24M36M]));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("         OFDM_48M    = 0x%2x                                                 \n", cSKUTable[OFDM48M]));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("         OFDM_54M    = 0x%2x                                                 \n", cSKUTable[OFDM54M]));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("         HT20_M0     = 0x%2x                                                 \n", cSKUTable[HT20M0]));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("         HT20_M32    = 0x%2x                                                 \n", cSKUTable[HT20M32]));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("         HT20_M1M2   = 0x%2x                                                 \n", cSKUTable[HT20M1M2]));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("         HT20_M3M4   = 0x%2x                                                 \n", cSKUTable[HT20M3M4]));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("         HT20_M5     = 0x%2x                                                 \n", cSKUTable[HT20M5]));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("         HT20_M6     = 0x%2x                                                 \n", cSKUTable[HT20M6]));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("         HT20_M7     = 0x%2x                                                 \n", cSKUTable[HT20M7]));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("         HT40_M0     = 0x%2x                                                 \n", cSKUTable[HT40M0]));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("         HT40_M32    = 0x%2x                                                 \n", cSKUTable[HT40M32]));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("         HT40_M1M2   = 0x%2x                                                 \n", cSKUTable[HT40M1M2]));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("         HT40_M3M4   = 0x%2x                                                 \n", cSKUTable[HT40M3M4]));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("         HT40_M5     = 0x%2x                                                 \n", cSKUTable[HT40M5]));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("         HT40_M6     = 0x%2x                                                 \n", cSKUTable[HT40M6]));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("         HT40_M7     = 0x%2x                                                 \n", cSKUTable[HT40M7]));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("         VHT20_M0    = 0x%2x                                                 \n", cSKUTable[VHT20M0]));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("         VHT20_M1M2  = 0x%2x                                                 \n", cSKUTable[VHT20M1M2]));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("         VHT20_M3M4  = 0x%2x                                                 \n", cSKUTable[VHT20M3M4]));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("         VHT20_M5M6  = 0x%2x                                                 \n", cSKUTable[VHT20M5M6]));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("         VHT20_M7    = 0x%2x                                                 \n", cSKUTable[VHT20M7]));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("         VHT20_M8    = 0x%2x                                                 \n", cSKUTable[VHT20M8]));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("         VHT20_M9    = 0x%2x                                                 \n", cSKUTable[VHT20M9]));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("         VHT40_M0    = 0x%2x                                                 \n", cSKUTable[VHT40M0]));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("         VHT40_M1M2  = 0x%2x                                                 \n", cSKUTable[VHT40M1M2]));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("         VHT40_M3M4  = 0x%2x                                                 \n", cSKUTable[VHT40M3M4]));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("         VHT40_M5M6  = 0x%2x                                                 \n", cSKUTable[VHT40M5M6]));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("         VHT40_M7    = 0x%2x                                                 \n", cSKUTable[VHT40M7]));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("         VHT40_M8    = 0x%2x                                                 \n", cSKUTable[VHT40M8]));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("         VHT40_M9    = 0x%2x                                                 \n", cSKUTable[VHT40M9]));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("         VHT80_M0    = 0x%2x                                                 \n", cSKUTable[VHT80M0]));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("         VHT80_M1M2  = 0x%2x                                                 \n", cSKUTable[VHT80M1M2]));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("         VHT80_M3M4  = 0x%2x                                                 \n", cSKUTable[VHT80M3M4]));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("         VHT80_M5M6  = 0x%2x                                                 \n", cSKUTable[VHT80M5M6]));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("         VHT80_M7    = 0x%2x                                                 \n", cSKUTable[VHT80M7]));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("         VHT80_M8    = 0x%2x                                                 \n", cSKUTable[VHT80M8]));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("         VHT80_M9    = 0x%2x                                                 \n", cSKUTable[VHT80M9]));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("         VHT160_M0   = 0x%2x                                                 \n", cSKUTable[VHT160M0]));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("         VHT160_M1M2 = 0x%2x                                                 \n", cSKUTable[VHT160M1M2]));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("         VHT160_M3M4 = 0x%2x                                                 \n", cSKUTable[VHT160M3M4]));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("         VHT160_M5M6 = 0x%2x                                                 \n", cSKUTable[VHT160M5M6]));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("         VHT160_M7   = 0x%2x                                                 \n", cSKUTable[VHT160M7]));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("         VHT160_M8   = 0x%2x                                                 \n", cSKUTable[VHT160M8]));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("         VHT160_M9   = 0x%2x                                                 \n", cSKUTable[VHT160M9]));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("         1SS_OFFSET  = 0x%2x                                                 \n", cSKUTable[TXPOWER_1SS_OFFSET]));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("         2SS_OFFSET  = 0x%2x                                                 \n", cSKUTable[TXPOWER_2SS_OFFSET]));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("         3SS_OFFSET  = 0x%2x                                                 \n", cSKUTable[TXPOWER_3SS_OFFSET]));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("         4SS_OFFSET  = 0x%2x                                                 \n", cSKUTable[TXPOWER_4SS_OFFSET]));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("============================================================================= \n"));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("                          Power Percentage Info                               \n"));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("============================================================================= \n"));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("  Power Percentage Enable status :  (Band%d): %d                              \n", ucBandIdx, fgPERCENTAGEEnable));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("  PowerDrop status               :  (Band%d): %d                              \n", ucBandIdx, cPowerDrop));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("============================================================================= \n"));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("                             BF Backoff Info                                  \n"));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("============================================================================= \n"));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("  BF BACKOFF Enable status:  (Band%d): %d                                     \n", ucBandIdx, fgBFBACKOFFEnable));

	for (BFBackoffIdx = 0; BFBackoffIdx < BFBACKOFF_TABLE_SIZE; BFBackoffIdx++)
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("  BFBACKOFFTableOn[%d]  = 0x%02x                                          \n", BFBackoffIdx,
				  cTxPwrBFBackoffValue[BF_BACKOFF_ON_MODE][BFBackoffIdx]));

	for (BFBackoffIdx = 0; BFBackoffIdx < BFBACKOFF_TABLE_SIZE; BFBackoffIdx++)
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("  BFBACKOFFTableOff[%d] = 0x%02x                                          \n", BFBackoffIdx,
				  cTxPwrBFBackoffValue[BF_BACKOFF_OFF_MODE][BFBackoffIdx]));

	for (BBPCRIdx = 0; BBPCRIdx < BFBACKOFF_BBPCR_SIZE; BBPCRIdx++)
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("  BackoffCRValue[%d] 0x%x = 0x%08x                                        \n", BBPCRIdx, u4BBPAddress + 4 * BBPCRIdx,
				  u4BackoffCRValue[BBPCRIdx]));

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("============================================================================= \n"));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("                          Power Upper Bound Info                              \n"));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("============================================================================= \n"));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("  Power Bound CRValue (0x820F4080) = 0x%08x                                   \n", u4PowerBoundCRValue));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("============================================================================= \n"));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("                         Thermal Compensation Info                            \n"));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("============================================================================= \n"));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("  Thermal Compensation Enable: %d                                             \n", fgThermalCompEnable));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("  Thermal Compensation Value : %d                                             \n", cThermalCompValue));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("============================================================================= \n"));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("                            TMAC Info (Band%d)                                \n", ucBandIdx));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("============================================================================= \n"));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 (" [CCK_1M2M]       Power DAC value: 0x%2x,   Power Decimal value: %d        \n", TxRatePowerTable[CCK_1M2M].CRValue,
			  TxRatePowerTable[CCK_1M2M].PowerDecimal));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 (" [CCK_5M11M]      Power DAC value: 0x%2x,   Power Decimal value: %d        \n", TxRatePowerTable[CCK_5M11M].CRValue,
			  TxRatePowerTable[CCK_5M11M].PowerDecimal));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 (" [OFDM_6M_9M]     Power DAC value: 0x%2x,   Power Decimal value: %d        \n", TxRatePowerTable[OFDM_6M_9M].CRValue,
			  TxRatePowerTable[OFDM_6M_9M].PowerDecimal));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 (" [OFDM_12M_18M]   Power DAC value: 0x%2x,   Power Decimal value: %d        \n",
			  TxRatePowerTable[OFDM_12M_18M].CRValue, TxRatePowerTable[OFDM_12M_18M].PowerDecimal));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 (" [OFDM_24M_36M]   Power DAC value: 0x%2x,   Power Decimal value: %d        \n",
			  TxRatePowerTable[OFDM_24M_36M].CRValue, TxRatePowerTable[OFDM_24M_36M].PowerDecimal));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 (" [OFDM_48M]       Power DAC value: 0x%2x,   Power Decimal value: %d        \n", TxRatePowerTable[OFDM_48M].CRValue,
			  TxRatePowerTable[OFDM_48M].PowerDecimal));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 (" [OFDM_54M]       Power DAC value: 0x%2x,   Power Decimal value: %d        \n", TxRatePowerTable[OFDM_54M].CRValue,
			  TxRatePowerTable[OFDM_54M].PowerDecimal));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 (" [HT20_MCS0]      Power DAC value: 0x%2x,   Power Decimal value: %d        \n", TxRatePowerTable[HT20_MCS0].CRValue,
			  TxRatePowerTable[HT20_MCS0].PowerDecimal));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 (" [HT20_MCS1_2]    Power DAC value: 0x%2x,   Power Decimal value: %d        \n", TxRatePowerTable[HT20_MCS1_2].CRValue,
			  TxRatePowerTable[HT20_MCS1_2].PowerDecimal));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 (" [HT20_MCS3_4]    Power DAC value: 0x%2x,   Power Decimal value: %d        \n", TxRatePowerTable[HT20_MCS3_4].CRValue,
			  TxRatePowerTable[HT20_MCS3_4].PowerDecimal));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 (" [HT20_MCS5]      Power DAC value: 0x%2x,   Power Decimal value: %d        \n", TxRatePowerTable[HT20_MCS5].CRValue,
			  TxRatePowerTable[HT20_MCS5].PowerDecimal));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 (" [HT20_MCS6]      Power DAC value: 0x%2x,   Power Decimal value: %d        \n", TxRatePowerTable[HT20_MCS6].CRValue,
			  TxRatePowerTable[HT20_MCS6].PowerDecimal));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 (" [HT20_MCS7]      Power DAC value: 0x%2x,   Power Decimal value: %d        \n", TxRatePowerTable[HT20_MCS7].CRValue,
			  TxRatePowerTable[HT20_MCS7].PowerDecimal));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 (" [HT40_MCS32]     Power DAC value: 0x%2x,   Power Decimal value: %d        \n", TxRatePowerTable[HT40_MCS32].CRValue,
			  TxRatePowerTable[HT40_MCS32].PowerDecimal));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 (" [HT40_MCS0]      Power DAC value: 0x%2x,   Power Decimal value: %d        \n", TxRatePowerTable[HT40_MCS0].CRValue,
			  TxRatePowerTable[HT40_MCS0].PowerDecimal));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 (" [HT40_MCS1_2]    Power DAC value: 0x%2x,   Power Decimal value: %d        \n", TxRatePowerTable[HT40_MCS1_2].CRValue,
			  TxRatePowerTable[HT40_MCS1_2].PowerDecimal));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 (" [HT40_MCS3_4]    Power DAC value: 0x%2x,   Power Decimal value: %d        \n", TxRatePowerTable[HT40_MCS3_4].CRValue,
			  TxRatePowerTable[HT40_MCS3_4].PowerDecimal));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 (" [HT40_MCS5]      Power DAC value: 0x%2x,   Power Decimal value: %d        \n", TxRatePowerTable[HT40_MCS5].CRValue,
			  TxRatePowerTable[HT40_MCS5].PowerDecimal));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 (" [HT40_MCS6]      Power DAC value: 0x%2x,   Power Decimal value: %d        \n", TxRatePowerTable[HT40_MCS6].CRValue,
			  TxRatePowerTable[HT40_MCS6].PowerDecimal));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 (" [HT40_MCS7]      Power DAC value: 0x%2x,   Power Decimal value: %d        \n", TxRatePowerTable[HT40_MCS7].CRValue,
			  TxRatePowerTable[HT40_MCS7].PowerDecimal));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 (" [VHT20_MCS0]     Power DAC value: 0x%2x,   Power Decimal value: %d        \n", TxRatePowerTable[VHT20_MCS0].CRValue,
			  TxRatePowerTable[VHT20_MCS0].PowerDecimal));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 (" [VHT20_MCS1_2]   Power DAC value: 0x%2x,   Power Decimal value: %d        \n",
			  TxRatePowerTable[VHT20_MCS1_2].CRValue, TxRatePowerTable[VHT20_MCS1_2].PowerDecimal));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 (" [VHT20_MCS3_4]   Power DAC value: 0x%2x,   Power Decimal value: %d        \n",
			  TxRatePowerTable[VHT20_MCS3_4].CRValue, TxRatePowerTable[VHT20_MCS3_4].PowerDecimal));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 (" [VHT20_MCS5_6]   Power DAC value: 0x%2x,   Power Decimal value: %d        \n",
			  TxRatePowerTable[VHT20_MCS5_6].CRValue, TxRatePowerTable[VHT20_MCS5_6].PowerDecimal));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 (" [VHT20_MCS7]     Power DAC value: 0x%2x,   Power Decimal value: %d        \n", TxRatePowerTable[VHT20_MCS7].CRValue,
			  TxRatePowerTable[VHT20_MCS7].PowerDecimal));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 (" [VHT20_MCS8]     Power DAC value: 0x%2x,   Power Decimal value: %d        \n", TxRatePowerTable[VHT20_MCS8].CRValue,
			  TxRatePowerTable[VHT20_MCS8].PowerDecimal));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 (" [VHT20_MCS9]     Power DAC value: 0x%2x,   Power Decimal value: %d        \n", TxRatePowerTable[VHT20_MCS9].CRValue,
			  TxRatePowerTable[VHT20_MCS9].PowerDecimal));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("----------------------------------------------------------------------------- \n"));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 (" [VHT40][Offset]  Power DAC value: 0x%2x,   Power Decimal value: %d        \n", TxRatePowerTable[VHT40].CRValue,
			  TxRatePowerTable[VHT40].PowerDecimal));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 (" [VHT80][Offset]  Power DAC value: 0x%2x,   Power Decimal value: %d        \n", TxRatePowerTable[VHT80].CRValue,
			  TxRatePowerTable[VHT80].PowerDecimal));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 (" [VHT160][Offset] Power DAC value: 0x%2x,   Power Decimal value: %d        \n", TxRatePowerTable[VHT160].CRValue,
			  TxRatePowerTable[VHT160].PowerDecimal));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("----------------------------------------------------------------------------- \n"));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 (" [MAX][Bound]     Power DAC value: 0x%2x,   Power Decimal value: %d        \n",
			  TxPowerBoundTable[ucPwrBoundIdx].CRValue, TxPowerBoundTable[ucPwrBoundIdx].PowerDecimal));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 (" [MIN][Bound]     Power DAC value: 0x%2x,   Power Decimal value: %d        \n",
			  TxPowerBoundTable[ucPwrBoundIdx + 1].CRValue, TxPowerBoundTable[ucPwrBoundIdx + 1].PowerDecimal));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("============================================================================= \n"));
}

VOID EventTxPowerEPAInfo(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	P_EXT_EVENT_EPA_STATUS_T  prEventTxPowerEPAInfo;
	prEventTxPowerEPAInfo = (P_EXT_EVENT_EPA_STATUS_T)Data;
	/* update EPA status */
	pAd->fgEPA = prEventTxPowerEPAInfo->fgEPA;
}


NTSTATUS EventTxvBbpPowerInfo(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	P_EXT_EVENT_TXV_BBP_POWER_INFO_T prEventTxvBbpPowerInfo;
	UINT8 ucBandIdx = 0;
	UINT8 ucAntIdx = 0;
	UINT8 ucAbsoTxvPower = 0;
	PUINT8 pu1AbsoBbpPower = NULL;
	UINT8 ucNegTxv = 0;
	PUINT8 pu1NegBbp = NULL;
	UINT8 ucWfNum = 0;

	/* get event info buffer contents */
	prEventTxvBbpPowerInfo = (P_EXT_EVENT_TXV_BBP_POWER_INFO_T)Data;

	if (Length != sizeof(EXT_EVENT_TXV_BBP_POWER_INFO_T))
		return STATUS_UNSUCCESSFUL;

	/*WF Path*/
	ucWfNum = prEventTxvBbpPowerInfo->ucWfNum;

	/* allocate memory for buffer power limit value */
	os_alloc_mem(pAd, (UINT8 **)&pu1AbsoBbpPower, ucWfNum);

	/*Check allocated memory*/
	if (!pu1AbsoBbpPower)
		return STATUS_UNSUCCESSFUL;

	os_alloc_mem(pAd, (UINT8 **)&pu1NegBbp, ucWfNum);

	/*Check allocated memory and Free memory*/
	if (!pu1NegBbp) {
		os_free_mem(pu1AbsoBbpPower);
		return STATUS_UNSUCCESSFUL;
	}

	/* initinal memory */
	os_zero_mem(pu1AbsoBbpPower, ucWfNum);
	os_zero_mem(pu1NegBbp, ucWfNum);

	if (prEventTxvBbpPowerInfo->cTxvPower < 0) {
		ucAbsoTxvPower = ~prEventTxvBbpPowerInfo->cTxvPower + 1;
		ucNegTxv = 1;
	} else
		ucAbsoTxvPower = prEventTxvBbpPowerInfo->cTxvPower;

	for (ucAntIdx = 0; ucAntIdx < ucWfNum; ucAntIdx++) {
		if (prEventTxvBbpPowerInfo->cBbpPower[ucAntIdx] < 0) {
			*(pu1AbsoBbpPower + ucAntIdx) = ~prEventTxvBbpPowerInfo->cBbpPower[ucAntIdx] + 1;
			*(pu1NegBbp + ucAntIdx) = 1;
		} else
			*(pu1AbsoBbpPower + ucAntIdx)  = prEventTxvBbpPowerInfo->cBbpPower[ucAntIdx];
	}

	ucBandIdx = prEventTxvBbpPowerInfo->ucBandIdx;

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("=============================================================================\n"));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("   Target TXV and BBP POWER INFO (per packet)             \n"));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("=============================================================================\n"));

	/* get cTxvPower */
	if (prEventTxvBbpPowerInfo->cTxvPower % 2) {
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("[%s]  TXV Power  (0x%x [%02d:%02d]): 0x%02x     (%s%02d.5 dBm)\n", (ucBandIdx == 1) ? "BAND1" : "BAND0",
		(prEventTxvBbpPowerInfo->u2TxvPowerCR), (prEventTxvBbpPowerInfo->ucTxvPowerMaskEnd), (prEventTxvBbpPowerInfo->ucTxvPowerMaskBegin),
		(prEventTxvBbpPowerInfo->cTxvPowerDac),
		(ucNegTxv == 1) ? "-" : " ", (ucAbsoTxvPower>>1)));
	} else {
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("[%s]  TXV Power  (0x%x [%02d:%02d]): 0x%02x     (%s%02d dBm)\n", (ucBandIdx == 1) ? "BAND1" : "BAND0",
		(prEventTxvBbpPowerInfo->u2TxvPowerCR), (prEventTxvBbpPowerInfo->ucTxvPowerMaskEnd), (prEventTxvBbpPowerInfo->ucTxvPowerMaskBegin),
		(prEventTxvBbpPowerInfo->cTxvPowerDac),
		(ucNegTxv == 1) ? "-" : " ", (ucAbsoTxvPower>>1)));
	}

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("-----------------------------------------------------------------------------\n"));

	/* BBP POWER INFO */
	for (ucAntIdx = 0; ucAntIdx < ucWfNum; ucAntIdx++) {
		if (prEventTxvBbpPowerInfo->cBbpPower[ucAntIdx] % 2) {
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[WF%01d]  BBP Power  (0x%x [%02d:%02d]): 0x%02x     (%s%02d.5 dBm)\n",
			ucAntIdx, prEventTxvBbpPowerInfo->u2BbpPowerCR[ucAntIdx],
			prEventTxvBbpPowerInfo->ucBbpPowerMaskEnd, prEventTxvBbpPowerInfo->ucBbpPowerMaskBegin,
			(prEventTxvBbpPowerInfo->cBbpPowerDac[ucAntIdx]), (*(pu1NegBbp + ucAntIdx)  == 1) ? "-" : " ", (*(pu1AbsoBbpPower + ucAntIdx) >> 1)));
		} else {
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[WF%01d]  BBP Power  (0x%x [%02d:%02d]): 0x%02x     (%s%02d dBm)\n",
			ucAntIdx, prEventTxvBbpPowerInfo->u2BbpPowerCR[ucAntIdx],
			prEventTxvBbpPowerInfo->ucBbpPowerMaskEnd, prEventTxvBbpPowerInfo->ucBbpPowerMaskBegin,
			(prEventTxvBbpPowerInfo->cBbpPowerDac[ucAntIdx]), (*(pu1NegBbp + ucAntIdx)  == 1) ? "-" : " ", (*(pu1AbsoBbpPower + ucAntIdx) >> 1)));

		}
	}

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("-----------------------------------------------------------------------------\n"));

	/* free allocated memory */
	os_free_mem(pu1AbsoBbpPower);
	os_free_mem(pu1NegBbp);

	return STATUS_SUCCESS;
}



 VOID EventThermalStateShowInfo(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	P_EXT_EVENT_THERMAL_STATE_INFO_T  prEventThermalStateInfo;
	UINT8  ucThermalItemIdx;
	CHAR   cThermalItem[THERMO_ITEM_NUM][18] = {"DPD_CAL          ", /*  0 */
												"OVERHEAT         ", /*  1 */
												"BB_HI            ", /*  2 */
												"BB_LO            ", /*  3 */
												"NTX_PROTECT_HI   ", /*  4 */
												"NTX_PROTECT_LO   ", /*  5 */
												"ADM_PROTECT_HI   ", /*  6 */
												"ADM_PROTECT_LO   ", /*  7 */
												"RF_PROTECT_HI    ", /*  8 */
												"_TSSI_COMP       ", /*  9 */
												"TEMP_COMP_N7_2G4 ", /* 10 */
												"TEMP_COMP_N6_2G4 ", /* 11 */
												"TEMP_COMP_N5_2G4 ", /* 12 */
												"TEMP_COMP_N4_2G4 ", /* 13 */
												"TEMP_COMP_N3_2G4 ", /* 14 */
												"TEMP_COMP_N2_2G4 ", /* 15 */
												"TEMP_COMP_N1_2G4 ", /* 16 */
												"TEMP_COMP_N0_2G4 ", /* 17 */
												"TEMP_COMP_P1_2G4 ", /* 18 */
												"TEMP_COMP_P2_2G4 ", /* 19 */
												"TEMP_COMP_P3_2G4 ", /* 20 */
												"TEMP_COMP_P4_2G4 ", /* 21 */
												"TEMP_COMP_P5_2G4 ", /* 22 */
												"TEMP_COMP_P6_2G4 ", /* 23 */
												"TEMP_COMP_P7_2G4 ", /* 24 */
												"TEMP_COMP_N7_5G  ", /* 25 */
												"TEMP_COMP_N6_5G  ", /* 26 */
												"TEMP_COMP_N5_5G  ", /* 27 */
												"TEMP_COMP_N4_5G  ", /* 28 */
												"TEMP_COMP_N3_5G  ", /* 29 */
												"TEMP_COMP_N2_5G  ", /* 30 */
												"TEMP_COMP_N1_5G  ", /* 31 */
												"TEMP_COMP_N0_5G  ", /* 32 */
												"TEMP_COMP_P1_5G  ", /* 33 */
												"TEMP_COMP_P2_5G  ", /* 34 */
												"TEMP_COMP_P3_5G  ", /* 35 */
												"TEMP_COMP_P4_5G  ", /* 36 */
												"TEMP_COMP_P5_5G  ", /* 37 */
												"TEMP_COMP_P6_5G  ", /* 38 */
												"TEMP_COMP_P7_5G  ", /* 39 */
												"DYNAMIC_G0       "	 /* 40 */
												};

	/* Get pointer of Event Info Structure */
	prEventThermalStateInfo = (P_EXT_EVENT_THERMAL_STATE_INFO_T)Data;

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Total Thermo Item Num: %d\n\n", prEventThermalStateInfo->ucThermoItemsNum));

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("================================================================================== \n"));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("        Item            Type       LowEn       HighEn      LowerBnd       UpperBnd \n"));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("================================================================================== \n"));

	/* Thermal State Info Table */
	for (ucThermalItemIdx = 0; ucThermalItemIdx < prEventThermalStateInfo->ucThermoItemsNum; ucThermalItemIdx++) {

		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s     %3d        %3d         %3d          %3d            %3d\n",
															cThermalItem[prEventThermalStateInfo->arThermoItems[ucThermalItemIdx].ucThermoItem],
															prEventThermalStateInfo->arThermoItems[ucThermalItemIdx].ucThermoType,
															prEventThermalStateInfo->arThermoItems[ucThermalItemIdx].fgLowerEn,
															prEventThermalStateInfo->arThermoItems[ucThermalItemIdx].fgUpperEn,
															prEventThermalStateInfo->arThermoItems[ucThermalItemIdx].cLowerBound,
															prEventThermalStateInfo->arThermoItems[ucThermalItemIdx].cUpperBound
															));
	}
}

VOID EventPowerTableShowInfo(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	P_EXT_EVENT_TX_POWER_BACKUP_TABLE_INFO_T  prEventPowerTableInfo;
	UINT8  ucPowerTblIdx;
	CHAR  cPowerItem[SKU_TABLE_SIZE][13] = {"CCK_1M2M    ",
											"CCK_5M11M   ",
											"OFDM_6M9M   ",
											"OFDM_12M18M ",
											"OFDM_24M36M ",
											"OFDM_48M    ",
											"OFDM_54M    ",
											"HT20_MCS0   ",
											"HT20_MCS32  ",
											"HT20_MCS12  ",
											"HT20_MCS34  ",
											"HT20_MCS5   ",
											"HT20_MCS6   ",
											"HT20_MCS7   ",
											"HT40_MCS0   ",
											"HT40_MCS32  ",
											"HT40_MCS12  ",
											"HT40_MCS34  ",
											"HT40_MCS5   ",
											"HT40_MCS6   ",
											"HT40_MCS7   ",
											"VHT20_MCS0  ",
											"VHT20_MCS12 ",
											"VHT20_MCS34 ",
											"VHT20_MCS56 ",
											"VHT20_MCS7  ",
											"VHT20_MCS8  ",
											"VHT20_MCS9  ",
											"VHT40_MCS0  ",
											"VHT40_MCS12 ",
											"VHT40_MCS34 ",
											"VHT40_MCS56 ",
											"VHT40_MCS7  ",
											"VHT40_MCS8  ",
											"VHT40_MCS9  ",
											"VHT80_MCS0  ",
											"VHT80_MCS12 ",
											"VHT80_MCS34 ",
											"VHT80_MCS56 ",
											"VHT80_MCS7  ",
											"VHT80_MCS8  ",
											"VHT80_MCS9  ",
											"VHT160_MCS0 ",
											"VHT160_MCS12",
											"VHT160_MCS34",
											"VHT160_MCS56",
											"VHT160_MCS7 ",
											"VHT160_MCS8 ",
											"VHT160_MCS9 "
											};

	/* Get pointer of Event Info Structure */
	prEventPowerTableInfo = (P_EXT_EVENT_TX_POWER_BACKUP_TABLE_INFO_T)Data;

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("============================================================================= \n"));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("   Phy Rate             1SS        2SS        3SS        4SS                  \n"));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("============================================================================= \n"));

	for (ucPowerTblIdx = 0; ucPowerTblIdx < SKU_TABLE_SIZE; ucPowerTblIdx++) {

		/* Neglect Invalid rate HT20 MCS32 */
		if (HT20M32 == ucPowerTblIdx)
			continue;

		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s             %2d        %2d        %2d        %2d\n",
															cPowerItem[ucPowerTblIdx],
															prEventPowerTableInfo->cTxPowerBackup[ucPowerTblIdx][SKU_TX_SPATIAL_STREAM_1SS],
															prEventPowerTableInfo->cTxPowerBackup[ucPowerTblIdx][SKU_TX_SPATIAL_STREAM_2SS],
															prEventPowerTableInfo->cTxPowerBackup[ucPowerTblIdx][SKU_TX_SPATIAL_STREAM_3SS],
															prEventPowerTableInfo->cTxPowerBackup[ucPowerTblIdx][SKU_TX_SPATIAL_STREAM_4SS]
															));
	}
}


VOID EventThermalCompTableShowInfo(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	P_EXT_EVENT_THERMAL_COMPENSATION_TABLE_INFO_T prEventThermalCompTableInfo = NULL;
	UINT8 ucIdx = 0;
	UINT8 ucChBand = 0;
	UINT8 ucBandIdx = 0;
	CHAR  cPowerItem[THERMAL_TABLE_SIZE][20] = {"-7_Step_Number  ",
												"-6_Step_Number  ",
												"-5_Step_Number  ",
												"-4_Step_Number  ",
												"-3_Step_Number  ",
												"-2_Step_Number  ",
												"-1_Step_Number  ",
												" 0_Step_Number  ",
												" 1_Step_Number  ",
												" 2_Step_Number  ",
												" 3_Step_Number  ",
												" 4_Step_Number  ",
												" 5_Step_Number  ",
												" 6_Step_Number  ",
												" 7_Step_Number  "
											};
	if (Data == NULL)
		return;

	/* Get pointer of Event Info Structure */
	prEventThermalCompTableInfo = (P_EXT_EVENT_THERMAL_COMPENSATION_TABLE_INFO_T)Data;

	/* get Band Info */
	ucChBand = prEventThermalCompTableInfo->ucBand;

	/* get Band Index */
	ucBandIdx = prEventThermalCompTableInfo->ucBandIdx;

	/* Show Thermal Compensation Table */
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("=========================================\n"));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("       Thermal Compensation Table        \n"));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("=========================================\n"));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("  Band Index: %d,  Channel Band: %s\n",
		 prEventThermalCompTableInfo->ucBandIdx, (prEventThermalCompTableInfo->ucBand) ? ("2G") : ("5G")));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("-----------------------------------------\n"));

	for (ucIdx = 0; ucIdx < THERMAL_TABLE_SIZE; ucIdx++)
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s    = 0x%x \n", cPowerItem[ucIdx], prEventThermalCompTableInfo->cThermalComp[ucIdx]));

MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("------------------------------------------\n"));

}


VOID EventTxPowerCompTable(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	P_EXT_EVENT_TXPOWER_BACKUP_T  prEventTxPowerCompTable;
	prEventTxPowerCompTable = (P_EXT_EVENT_TXPOWER_BACKUP_T)Data;

	/* update power compensation value table */
	os_move_mem(pAd->CommonCfg.cTxPowerCompBackup[prEventTxPowerCompTable->ucBandIdx], prEventTxPowerCompTable->cTxPowerCompBackup, sizeof(INT8) * SKU_TABLE_SIZE * SKU_TX_SPATIAL_STREAM_NUM);
}

#ifdef LED_CONTROL_SUPPORT
#if defined(MT7615) || defined(MT7622)
INT AndesLedEnhanceOP(
	RTMP_ADAPTER *pAd,
	UCHAR led_idx,
	UCHAR tx_over_blink,
	UCHAR reverse_polarity,
	UCHAR band,
	UCHAR blink_mode,
	UCHAR off_time,
	UCHAR on_time,
	UCHAR led_control_mode
)
#else
INT AndesLedEnhanceOP(
	RTMP_ADAPTER *pAd,
	UCHAR led_idx,
	UCHAR tx_over_blink,
	UCHAR reverse_polarity,
	UCHAR blink_mode,
	UCHAR off_time,
	UCHAR on_time,
	UCHAR led_control_mode
)
#endif
{
	struct cmd_msg *msg;
	CHAR *pos, *buf;
	UINT32 len;
	UINT32 arg0;
	INT32 ret;
	LED_ENHANCE led_enhance;
	struct _CMD_ATTRIBUTE attr = {0};
	len = sizeof(LED_ENHANCE) + sizeof(arg0);
	msg = AndesAllocCmdMsg(pAd, len);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	SET_CMD_ATTR_MCU_DEST(attr, HOST2N9);
	SET_CMD_ATTR_TYPE(attr, EXT_CID);
	SET_CMD_ATTR_EXT_TYPE(attr, EXT_CMD_ID_LED);
	SET_CMD_ATTR_CTRL_FLAGS(attr, INIT_CMD_SET);
	SET_CMD_ATTR_RSP_WAIT_MS_TIME(attr, 0);
	SET_CMD_ATTR_RSP_EXPECT_SIZE(attr, 0);
	SET_CMD_ATTR_RSP_WB_BUF_IN_CALBK(attr, NULL);
	SET_CMD_ATTR_RSP_HANDLER(attr, NULL);
	AndesInitCmdMsg(msg, attr);
	/* Led ID and Parameter */
	arg0 = led_idx;
	led_enhance.word = 0;
	led_enhance.field.on_time = on_time;
	led_enhance.field.off_time = off_time;
	led_enhance.field.tx_blink = blink_mode;
#if defined(MT7615) || defined(MT7622)
	if (IS_MT7622(pAd))
		led_enhance.field.band_select = BAND0;
	else if (IS_MT7615(pAd))
	led_enhance.field.band_select = band;
#endif
	led_enhance.field.reverse_polarity = reverse_polarity;
	led_enhance.field.tx_over_blink = tx_over_blink;
	/*
		if (pAd->LedCntl.LedMethod == 1)
		{
			led_enhance.field.tx_blink=2;
			led_enhance.field.reverse_polarity=1;
			if (led_control_mode == 1 || led_control_mode == 0)
				led_control_mode = ~(led_control_mode) & 0x1;
		}
	*/
	led_enhance.field.idx = led_control_mode;
	os_alloc_mem(pAd, (UCHAR **)&buf, len);

	if (buf == NULL)
		return NDIS_STATUS_RESOURCES;

	NdisZeroMemory(buf, len);
	pos = buf;
	/* Parameter */
#ifdef RT_BIG_ENDIAN
	arg0 = cpu2le32(arg0);
	led_enhance.word = cpu2le32(led_enhance.word);
#endif
	NdisMoveMemory(pos, &arg0, 4);
	NdisMoveMemory(pos + 4, &led_enhance, sizeof(led_enhance));
	pos += 4;
	hex_dump("AndesLedOPEnhance: ", buf, len);
	AndesAppendCmdMsg(msg, (char *)buf, len);
	ret = AndesSendCmdMsg(pAd, msg);
	os_free_mem(buf);
error:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s:(ret = %d)\n", __func__, ret));
	return ret;
}
#endif

#ifdef FQ_SCH_SUPPORT
VOID ExtEventMpduTimeHandler_fp(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	P_EXT_EVENT_MPDU_TIME_FQ_UPDATE_T prMpduTimeFQEvt = (P_EXT_EVENT_MPDU_TIME_FQ_UPDATE_T)Data;
	PMAC_TABLE_ENTRY pEntry;
	STA_TR_ENTRY *tr_entry;
	UINT8 i;
	UINT8 ucInUseSta = 0;
	UINT8 not_active = 0;
	UINT32 MpduTime = 0;
	UINT32 Value[WMM_NUM_OF_AC];
	UINT32 dwrr_quantum;

	RTMP_IO_READ32(pAd, UMAC_AIRTIME_QUANTUM_SETTING0, &dwrr_quantum);
	RTMP_IO_READ32(pAd, TMAC_ACTXOPLR1, &Value[0]);
	RTMP_IO_READ32(pAd, TMAC_ACTXOPLR1, &Value[1]);
	RTMP_IO_READ32(pAd, TMAC_ACTXOPLR0, &Value[2]);
	RTMP_IO_READ32(pAd, TMAC_ACTXOPLR0, &Value[3]);

	if (prMpduTimeFQEvt->ucfgValid) {
		for (i = 0; i < RED_STA_REC_NUM; i++) {

			pEntry = &pAd->MacTab.Content[i];
			if (IS_ENTRY_NONE(pEntry))
				continue;

			if (!VALID_UCAST_ENTRY_WCID(pAd, pEntry->wcid))
				continue;

			tr_entry = &pAd->MacTab.tr_entry[pEntry->tr_tb_idx];
			if (tr_entry->StaRec.ConnectionState == STATE_PORT_SECURE) {
				/*Calcualte in used station number */
				if ((i > 0) && (i < (MAX_LEN_OF_MAC_TABLE - HW_BEACON_MAX_NUM)) &&
					(prMpduTimeFQEvt->arMpduTime[i] > 0))
					ucInUseSta++;
			}

			if (pAd->fq_ctrl.enable & FQ_READY) {
				MpduTime = prMpduTimeFQEvt->arMpduTime[i];
				if (fq_update_thMax(pAd, tr_entry, i, MpduTime, dwrr_quantum, Value)
					!= NDIS_STATUS_SUCCESS)
					not_active++;
			}
		}

		/*update in used station number */
		pAd->red_in_use_sta = ucInUseSta;
		if (pAd->fq_ctrl.enable & FQ_READY)
			pAd->fq_ctrl.nactive = pAd->red_in_use_sta - not_active;
	}
}
#endif /* defined(FQ_SCH_SUPPORT) */

#ifdef RED_SUPPORT
VOID ExtEventMpduTimeHandler_avg(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	P_EXT_EVENT_MPDU_TIME_UPDATE_T prMpduTimeEvt = (P_EXT_EVENT_MPDU_TIME_UPDATE_T)Data;
	P_MPDU_SHORT_AVG_TIME_UPDATE_T prMpduShortTimeUpdate  = NULL;
	/* P_STA_RECORD_T prStaRec = cnmGetStaRecByIndex(0); */
	PMAC_TABLE_ENTRY pEntry;
	STA_TR_ENTRY *tr_entry;
	P_RED_STA_T prRedSta = &pAd->red_sta[0];
	UINT8 i;
	UINT8 ucInUseSta = 0;
	UINT8 ucPhyMode = 0;
	UINT8 ucBW = 0;
	UINT8 fgATCEnable = pAd->vow_cfg.en_bw_ctrl;
	UINT8 fgATFEnable = pAd->vow_cfg.en_airtime_fairness;
	UINT8 fgWATFEnable = pAd->vow_watf_en;
	UINT8 fgATCorWATFEnable = fgATCEnable || (fgATFEnable && fgWATFEnable);
#ifdef FQ_SCH_SUPPORT
	UINT8 j;
	UINT8 active = 0, bcmc_active = 0, pow_save = 0;
	struct fq_stainfo_type *pfq_sta = NULL;
	STA_TR_ENTRY *tr_entry_tmp;
	UINT32 Value[WMM_NUM_OF_AC];
	UINT32 dwrr_quantum;
#endif
	prMpduShortTimeUpdate = (P_MPDU_SHORT_AVG_TIME_UPDATE_T)(Data +
				sizeof (EXT_EVENT_MPDU_TIME_UPDATE_T));
#ifdef FQ_SCH_SUPPORT
	RTMP_IO_READ32(pAd, UMAC_AIRTIME_QUANTUM_SETTING0, &dwrr_quantum);
	RTMP_IO_READ32(pAd, TMAC_ACTXOPLR1, &Value[0]);
	RTMP_IO_READ32(pAd, TMAC_ACTXOPLR1, &Value[1]);
	RTMP_IO_READ32(pAd, TMAC_ACTXOPLR0, &Value[2]);
	RTMP_IO_READ32(pAd, TMAC_ACTXOPLR0, &Value[3]);
#endif
	for (i = 0; i < RED_STA_REC_NUM; i++) {
		if ((prMpduTimeEvt->staInUseBitmap[i>>RED_INUSE_BITSHIFT] &
					(1<<(i&RED_INUSE_BITMASK))) == 0) {
#ifdef FQ_SCH_SUPPORT
			tr_entry_tmp = &pAd->MacTab.tr_entry[i];
			pfq_sta = &tr_entry_tmp->fq_sta_rec;

			for (j = 0; j < WMM_NUM_OF_AC; j++) {
				RTMP_SEM_LOCK(&pfq_sta->lock[j]);
				if (pAd->fq_ctrl.list_map[j][i>>FQ_BITMAP_SHIFT] & (1<<(i & FQ_BITMAP_MASK)))
					pfq_sta->status[j] = FQ_UN_CLEAN_STA;
				RTMP_SEM_UNLOCK(&pfq_sta->lock[j]);
			}
#endif
			continue;
		}
		pEntry = &pAd->MacTab.Content[i];
		if (IS_ENTRY_NONE(pEntry)) {
#ifdef FQ_SCH_SUPPORT
			tr_entry_tmp = &pAd->MacTab.tr_entry[i];
			pfq_sta = &tr_entry_tmp->fq_sta_rec;
			for (j = 0; j < WMM_NUM_OF_AC; j++) {
				RTMP_SEM_LOCK(&pfq_sta->lock[j]);
				if (pAd->fq_ctrl.list_map[j][i>>FQ_BITMAP_SHIFT] & (1<<(i & FQ_BITMAP_MASK)))
					pfq_sta->status[j] = FQ_UN_CLEAN_STA;
				RTMP_SEM_UNLOCK(&pfq_sta->lock[j]);
			}
#endif
			prMpduShortTimeUpdate++;
			continue;
		}
		tr_entry = &pAd->MacTab.tr_entry[pEntry->tr_tb_idx];
		prRedSta = &pAd->red_sta[i];

		if (!VALID_UCAST_ENTRY_WCID(pAd, pEntry->wcid)) {
#ifdef FQ_SCH_SUPPORT
			tr_entry_tmp = &pAd->MacTab.tr_entry[i];
			prRedSta = &pAd->red_sta[i];
			if (pAd->fq_ctrl.enable & FQ_READY) {
				prRedSta->tx_msdu_avg_cnt = ((prRedSta->tx_msdu_avg_cnt)>>1) +
							(prRedSta->tx_msdu_cnt>>1);
				prRedSta->i4MpduTime = (prMpduShortTimeUpdate->arMpduTime < 0) ?
					(prMpduShortTimeUpdate->arMpduTime) :
					((prRedSta->tx_msdu_avg_cnt > 0) ?
					(prMpduShortTimeUpdate->arMpduTime/
					prRedSta->tx_msdu_avg_cnt) :
					prRedSta->i4MpduTime);
				prRedSta->tx_msdu_cnt = 0;
				if (fq_update_thMax(pAd, tr_entry_tmp, i, prRedSta->i4MpduTime,
						dwrr_quantum, Value)
						== NDIS_STATUS_SUCCESS) {
					active++;
					bcmc_active++;
				}
			}
#endif
			prMpduShortTimeUpdate++;
			continue;
		}

		if (tr_entry->StaRec.ConnectionState == STATE_PORT_SECURE) {
			prRedSta->tx_msdu_avg_cnt = ((prRedSta->tx_msdu_avg_cnt)>>1) +
							(prRedSta->tx_msdu_cnt>>1);
			prRedSta->i4MpduTime = (prMpduShortTimeUpdate->arMpduTime < 0) ?
					prMpduShortTimeUpdate->arMpduTime :
					((prRedSta->tx_msdu_avg_cnt > 0) ?
					(prMpduShortTimeUpdate->arMpduTime/
					prRedSta->tx_msdu_avg_cnt) :
					prRedSta->i4MpduTime);

			prRedSta->tx_msdu_cnt = 0;
			ucPhyMode = prMpduShortTimeUpdate->arPhymodeBW >> 4;
			ucBW = prMpduShortTimeUpdate->arPhymodeBW & 0x0F;
			/*If TxForceCnt/TxCnt > 25% and not badnode, then reset to default.*/
			RedCalForceRateRatio(i,
						prMpduShortTimeUpdate->arN9TxARCnt,
						prMpduShortTimeUpdate->arN9TxFRCnt,
						 pAd);

			/*Calcualte in used station number */
			if ((i > 0) && (i < MAX_LEN_OF_MAC_TABLE) &&
					(prMpduShortTimeUpdate->arMpduTime > 0))
				ucInUseSta++;

			if (prRedSta->i4MpduTime < 0)
				RedResetSta(i, ucPhyMode, ucBW, pAd);
			else
				UpdateThreshold(i, pAd);

			UpdateAirtimeRatio(i, prMpduShortTimeUpdate->arAirtimeRatio
						, fgATCorWATFEnable, pAd);
#ifdef FQ_SCH_SUPPORT
			if (pAd->fq_ctrl.enable & FQ_READY) {
				tr_entry_tmp = &pAd->MacTab.tr_entry[i];
				if (fq_update_thMax(pAd, tr_entry_tmp, i, prRedSta->i4MpduTime,
						dwrr_quantum, Value)
						== NDIS_STATUS_SUCCESS)
					active++;
			}
#endif
		} else {
			/*fgIsInUse == FALSE, don't care PHY mode. */
			RedResetSta(i, MODE_CCK, BW_20, pAd);
			prRedSta->i4MpduTime = RED_MPDU_TIME_INIT;
			prRedSta->tx_msdu_cnt = 0;
#ifdef FQ_SCH_SUPPORT
			tr_entry_tmp = &pAd->MacTab.tr_entry[i];
			pfq_sta = &tr_entry_tmp->fq_sta_rec;

			for (j = 0; j < WMM_NUM_OF_AC; j++) {
				RTMP_SEM_LOCK(&pfq_sta->lock[j]);
				if (pAd->fq_ctrl.list_map[j][i>>FQ_BITMAP_SHIFT] & (1<<(i & FQ_BITMAP_MASK)))
					pfq_sta->status[j] = FQ_UN_CLEAN_STA;
				RTMP_SEM_UNLOCK(&pfq_sta->lock[j]);
		}
#endif
		}
#ifdef FQ_SCH_SUPPORT
		if (tr_entry->PsMode == PWR_SAVE)
			pow_save++;
#endif
		prMpduShortTimeUpdate++;
	}

	/*update in used station number */
	pAd->red_in_use_sta = ucInUseSta;
#ifdef FQ_SCH_SUPPORT
	if (pAd->fq_ctrl.enable & FQ_READY) {
		pAd->fq_ctrl.nactive = active;
		pAd->fq_ctrl.nbcmc_active = bcmc_active;
		pAd->fq_ctrl.npow_save = pow_save;
		fq_clean_list(pAd, WMM_NUM_OF_AC);
	}
#endif
}
#endif

#ifdef RED_SUPPORT
VOID ExtEventMpduTimeHandler(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	P_EXT_EVENT_MPDU_TIME_UPDATE_T prMpduTimeEvt = (P_EXT_EVENT_MPDU_TIME_UPDATE_T)Data;
	P_MPDU_SHORT_TIME_UPDATE_T prMpduShortTimeUpdate  = NULL;
	/* P_STA_RECORD_T prStaRec = cnmGetStaRecByIndex(0); */
	PMAC_TABLE_ENTRY pEntry;
	STA_TR_ENTRY *tr_entry;
	P_RED_STA_T prRedSta = &pAd->red_sta[0];
	UINT8 i;
	UINT8 ucInUseSta = 0;
	UINT8 ucPhyMode = 0;
	UINT8 ucBW = 0;
	UINT8 fgATCEnable = pAd->vow_cfg.en_bw_ctrl;
	UINT8 fgATFEnable = pAd->vow_cfg.en_airtime_fairness;
	UINT8 fgWATFEnable = pAd->vow_watf_en;
	UINT8 fgATCorWATFEnable = fgATCEnable || (fgATFEnable && fgWATFEnable);
#ifdef FQ_SCH_SUPPORT
	UINT8 j;
	UINT8 active = 0, bcmc_active = 0, pow_save = 0;
	struct fq_stainfo_type *pfq_sta = NULL;
	STA_TR_ENTRY *tr_entry_tmp;
	UINT32 Value[WMM_NUM_OF_AC];
	UINT32 dwrr_quantum;

	for (j = 0; j < FQ_BITMAP_DWORD; j++)
		pAd->fq_ctrl.staInUseBitmap[j] = prMpduTimeEvt->staInUseBitmap[j];
#endif
	if (prMpduTimeEvt->ucfgValid == 2)
		return ExtEventMpduTimeHandler_avg(pAd, Data, Length);

#ifdef FQ_SCH_SUPPORT
	if (IS_MT7615(pAd))
		return ExtEventMpduTimeHandler_fp(pAd, Data, Length);
#endif
#ifdef FQ_SCH_SUPPORT
	RTMP_IO_READ32(pAd, UMAC_AIRTIME_QUANTUM_SETTING0, &dwrr_quantum);
	RTMP_IO_READ32(pAd, TMAC_ACTXOPLR1, &Value[0]);
	RTMP_IO_READ32(pAd, TMAC_ACTXOPLR1, &Value[1]);
	RTMP_IO_READ32(pAd, TMAC_ACTXOPLR0, &Value[2]);
	RTMP_IO_READ32(pAd, TMAC_ACTXOPLR0, &Value[3]);
#endif
	prMpduShortTimeUpdate = (P_MPDU_SHORT_TIME_UPDATE_T)(Data +
				sizeof (EXT_EVENT_MPDU_TIME_UPDATE_T));
	for (i = 0; i < RED_STA_REC_NUM; i++) {
		if ((prMpduTimeEvt->staInUseBitmap[i>>RED_INUSE_BITSHIFT]&
						(1<<(i&RED_INUSE_BITMASK))) == 0) {
#ifdef FQ_SCH_SUPPORT
			tr_entry_tmp = &pAd->MacTab.tr_entry[i];
			pfq_sta = &tr_entry_tmp->fq_sta_rec;

			for (j = 0; j < WMM_NUM_OF_AC; j++) {
				RTMP_SEM_LOCK(&pfq_sta->lock[j]);
				if (pAd->fq_ctrl.list_map[j][i>>FQ_BITMAP_SHIFT] & (1<<(i & FQ_BITMAP_MASK)))
					pfq_sta->status[j] = FQ_UN_CLEAN_STA;
				RTMP_SEM_UNLOCK(&pfq_sta->lock[j]);
			}
#endif
			continue;
		}
		pEntry = &pAd->MacTab.Content[i];
		if (IS_ENTRY_NONE(pEntry)) {
#ifdef FQ_SCH_SUPPORT
			tr_entry_tmp = &pAd->MacTab.tr_entry[i];
			pfq_sta = &tr_entry_tmp->fq_sta_rec;
			for (j = 0; j < WMM_NUM_OF_AC; j++) {
				RTMP_SEM_LOCK(&pfq_sta->lock[j]);
				if (pAd->fq_ctrl.list_map[j][i>>FQ_BITMAP_SHIFT] & (1<<(i & FQ_BITMAP_MASK)))
					pfq_sta->status[j] = FQ_UN_CLEAN_STA;
				RTMP_SEM_UNLOCK(&pfq_sta->lock[j]);
			}
#endif
			prMpduShortTimeUpdate++;
				continue;
		}

		if (!VALID_UCAST_ENTRY_WCID(pAd, pEntry->wcid)) {
#ifdef FQ_SCH_SUPPORT
			tr_entry_tmp = &pAd->MacTab.tr_entry[i];
			prRedSta = &pAd->red_sta[i];
			if (pAd->fq_ctrl.enable & FQ_READY) {
				prRedSta->tx_msdu_avg_cnt = ((prRedSta->tx_msdu_avg_cnt)>>1) +
							(prRedSta->tx_msdu_cnt>>1);
				prRedSta->i4MpduTime = (prMpduShortTimeUpdate->arMpduTime < 0) ?
					(prMpduShortTimeUpdate->arMpduTime) :
					((prRedSta->tx_msdu_avg_cnt > 0) ?
					(prMpduShortTimeUpdate->arMpduTime/
					prRedSta->tx_msdu_avg_cnt) :
					prRedSta->i4MpduTime);
				prRedSta->tx_msdu_cnt = 0;
				if (fq_update_thMax(pAd, tr_entry_tmp, i, prRedSta->i4MpduTime,
						dwrr_quantum, Value)
						== NDIS_STATUS_SUCCESS) {
					active++;
					bcmc_active++;
				}
			}
#endif
			prMpduShortTimeUpdate++;
				continue;
		}

		tr_entry = &pAd->MacTab.tr_entry[pEntry->tr_tb_idx];
		prRedSta = &pAd->red_sta[i];
		if (tr_entry->StaRec.ConnectionState == STATE_PORT_SECURE) {
			prRedSta->tx_msdu_avg_cnt = ((prRedSta->tx_msdu_avg_cnt)>>1) +
							(prRedSta->tx_msdu_cnt>>1);

			prRedSta->i4MpduTime = (prMpduShortTimeUpdate->arMpduTime < 0) ?
						prMpduShortTimeUpdate->arMpduTime :
						((prRedSta->tx_msdu_avg_cnt > 0) ?
						(prMpduShortTimeUpdate->arMpduTime/
						prRedSta->tx_msdu_avg_cnt) :
						prRedSta->i4MpduTime);

			prRedSta->tx_msdu_cnt = 0;

			ucPhyMode = prMpduShortTimeUpdate->arPhymodeBW >> 4;
			ucBW = prMpduShortTimeUpdate->arPhymodeBW & 0x0F;
			/*If TxForceCnt/TxCnt > 25% and not badnode, then reset to default.*/
			RedCalForceRateRatio(i,
					prMpduShortTimeUpdate->arN9TxARCnt,
					prMpduShortTimeUpdate->arN9TxFRCnt,
					 pAd);

			/*Calcualte in used station number */
			if ((i > 0) && (i < MAX_LEN_OF_MAC_TABLE) &&
				(prMpduShortTimeUpdate->arMpduTime > 0))
					ucInUseSta++;

			if (prRedSta->i4MpduTime < 0)
				RedResetSta(i, ucPhyMode, ucBW, pAd);
			else
				UpdateThreshold(i, pAd);

			UpdateAirtimeRatio(i, prMpduShortTimeUpdate->arAirtimeRatio
						, fgATCorWATFEnable, pAd);
#ifdef FQ_SCH_SUPPORT
			if (pAd->fq_ctrl.enable & FQ_READY) {
				tr_entry_tmp = &pAd->MacTab.tr_entry[i];
				if (fq_update_thMax(pAd, tr_entry_tmp, i, prRedSta->i4MpduTime,
						dwrr_quantum, Value)
						== NDIS_STATUS_SUCCESS)
					active++;
			}
#endif
		} else {
			/*fgIsInUse == FALSE, don't care PHY mode. */
			RedResetSta(i, MODE_CCK, BW_20, pAd);
			prRedSta->i4MpduTime = RED_MPDU_TIME_INIT;
			prRedSta->tx_msdu_cnt = 0;
#ifdef FQ_SCH_SUPPORT
			tr_entry_tmp = &pAd->MacTab.tr_entry[i];
			pfq_sta = &tr_entry_tmp->fq_sta_rec;

			for (j = 0; j < WMM_NUM_OF_AC; j++) {
				RTMP_SEM_LOCK(&pfq_sta->lock[j]);
				if (pAd->fq_ctrl.list_map[j][i>>FQ_BITMAP_SHIFT] & (1<<(i & FQ_BITMAP_MASK)))
					pfq_sta->status[j] = FQ_UN_CLEAN_STA;
				RTMP_SEM_UNLOCK(&pfq_sta->lock[j]);
		}
#endif
		}
#ifdef FQ_SCH_SUPPORT
		if (tr_entry->PsMode == PWR_SAVE)
			pow_save++;
#endif
	prMpduShortTimeUpdate++;
	}

	/*update in used station number */
	pAd->red_in_use_sta = ucInUseSta;
#ifdef FQ_SCH_SUPPORT
	if (pAd->fq_ctrl.enable & FQ_READY) {
		pAd->fq_ctrl.nactive = active;
		pAd->fq_ctrl.nbcmc_active = bcmc_active;
		pAd->fq_ctrl.npow_save = pow_save;
		fq_clean_list(pAd, WMM_NUM_OF_AC);
	}
#endif
}
#endif /* defined(RED_SUPPORT) */

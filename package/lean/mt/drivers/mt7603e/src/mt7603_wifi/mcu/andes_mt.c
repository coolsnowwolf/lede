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

#ifdef RTMP_SDIO_SUPPORT
INT32 AndesMTSendCmdMsgToSdio(PRTMP_ADAPTER pAd)
{
	INT32 Ret = 0;
	struct MCU_CTRL *ctl = &pAd->MCUCtrl;
	PNDIS_PACKET NetPkt = NULL;
	struct cmd_msg *msg = NULL;

	while ((msg = AndesDequeueCmdMsg(ctl, &ctl->txq_sdio)) != NULL) {
		if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_MCU_SEND_IN_BAND_CMD)
				|| RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)
				|| RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_SUSPEND)) {
			if (!msg->need_wait)
				AndesFreeCmdMsg(msg);
			continue;
		}

		if (AndesQueueLen(ctl, &ctl->ackq) > 0) {
			AndesQueueHeadCmdMsg(&ctl->txq_sdio, msg, msg->state);
			Ret = NDIS_STATUS_FAILURE;
			continue;
		}

		NetPkt = msg->net_pkt;

		/* send to SDIO host */
		Ret = MTSDIOCmdTx(pAd, GET_OS_PKT_DATAPTR(NetPkt), 
											GET_OS_PKT_LEN(NetPkt));

		if (Ret)
		{
			msg->state = tx_sdio_fail;
			DBGPRINT(RT_DEBUG_ERROR, ("kick out cmd to sdio host fail\n"));
			RTMP_OS_COMPLETE(&msg->tx_sdio_done);
			break;
		}
		else
		{
			msg->state = tx_sdio_ok;
			DBGPRINT(RT_DEBUG_TRACE, ("kick out cmd to sdio host ok\n"));
		}

		RTMP_OS_COMPLETE(&msg->tx_sdio_done);
	}

	return Ret;
}


static INT32 AndesMTSdioSubmitCmdMsgToSDIOWorker(PRTMP_ADAPTER pAd, struct cmd_msg *msg)
{
	INT32 Ret = 0;
	struct MCU_CTRL *ctl = &pAd->MCUCtrl;
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;

	AndesQueueTailCmdMsg(&ctl->txq_sdio, msg, msg->state);

	MTSDIOAddWorkerTaskList(pAd);
	queue_work(pObj->SdioWq, &pObj->SdioWork); 
	
	/* Wait for SDIO tx timeout */
	if (!AndesWaitForSdioCompleteTimeout(msg, msg->tx_sdio_timeout)) {
		DBGPRINT(RT_DEBUG_ERROR, ("%s: Wait for SDIO tx timeout(%d)\n", __FUNCTION__,
											msg->tx_sdio_timeout));

		AndesUnlinkCmdMsg(msg, &ctl->txq_sdio);
		Ret = -1;
	}
	else
	{
		if (msg->state == tx_sdio_ok)
		{
			Ret = 0;
		}
		else
		{
			Ret = -1;
		}
	}

	return Ret;
}


INT32 AndesMTSdioKickOutCmdMsg(PRTMP_ADAPTER pAd, struct cmd_msg *msg)
{
	struct MCU_CTRL *ctl = &pAd->MCUCtrl;
	INT32 Ret = 0;
	PNDIS_PACKET net_pkt = msg->net_pkt;

	if (!OS_TEST_BIT(MCU_INIT, &ctl->flags))
		return -1;

	if (msg->state != tx_retransmit) {
		/* 
 		 * append to meet block unit and zero four bytes padding 
 		 * when using sdio block mode 
 		 */
		memset(OS_PKT_TAIL_BUF_EXTEND(net_pkt, ctl->SDIOPadSize), 0x00, SDIO_END_PADDING);
	}

	Ret = AndesMTSdioSubmitCmdMsgToSDIOWorker(pAd, msg);

	if (msg->need_wait)
		AndesQueueTailCmdMsg(&ctl->ackq, msg, wait_cmd_out_and_ack);
	else
		AndesQueueTailCmdMsg(&ctl->kickq, msg, wait_cmd_out);

	if (Ret) {
		if (!msg->need_wait) {
			AndesUnlinkCmdMsg(msg, &ctl->kickq);
			AndesQueueTailCmdMsg(&ctl->tx_doneq, msg, tx_kickout_fail);
			AndesIncErrorCount(ctl, error_tx_kickout_fail);
		} else {
			AndesUnlinkCmdMsg(msg, &ctl->ackq);
			msg->state = tx_kickout_fail;
			AndesIncErrorCount(ctl, error_tx_kickout_fail);
			RTMP_OS_COMPLETE(&msg->ack_done);
		}

		DBGPRINT(RT_DEBUG_ERROR, ("%s:submit cmd msg to SDIO host fail(%d)\n", __FUNCTION__, Ret));
	}

	return Ret;
}


INT32 AndesMTSdioChkCrc(RTMP_ADAPTER *pAd, UINT32 checksum_len)
{


}


UINT16 AndesMTSdioGetCrc(RTMP_ADAPTER *pAd)
{




}
#endif



#ifdef RTMP_PCI_SUPPORT
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

	if (!OS_TEST_BIT(MCU_INIT, &ctl->flags))
		return -1;

	FreeNum = GET_CTRLRING_FREENO(pAd);

	if (FreeNum == 0) {
		DBGPRINT(RT_DEBUG_WARN, ("%s FreeNum == 0 (TxCpuIdx = %d, TxDmaIdx = %d, TxSwFreeIdx = %d)\n",
		__FUNCTION__, pAd->CtrlRing.TxCpuIdx, pAd->CtrlRing.TxDmaIdx, pAd->CtrlRing.TxSwFreeIdx));
		return NDIS_STATUS_FAILURE;
	}

	RTMP_SPIN_LOCK_IRQSAVE(&pAd->CtrlRingLock, &flags);

	RTMP_QueryPacketInfo(net_pkt, &PacketInfo, &pSrcBufVA, &SrcBufLen);

	if (pSrcBufVA == NULL) {
		RTMP_SPIN_UNLOCK_IRQRESTORE(&pAd->CtrlRingLock, &flags);
		return NDIS_STATUS_FAILURE;
	}

	SwIdx = pAd->CtrlRing.TxCpuIdx;

#ifdef RT_BIG_ENDIAN
	pDestTxD  = (TXD_STRUC *)pAd->CtrlRing.Cell[SwIdx].AllocVa;
	NdisMoveMemory(&tx_hw_info[0], (UCHAR *)pDestTxD, TXD_SIZE);
 	pTxD = (TXD_STRUC *)&tx_hw_info[0];
#else
	pTxD  = (TXD_STRUC *)pAd->CtrlRing.Cell[SwIdx].AllocVa;
#endif

#ifdef RT_BIG_ENDIAN
	RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#endif /* RT_BIG_ENDIAN */

	pAd->CtrlRing.Cell[SwIdx].pNdisPacket = net_pkt;
	pAd->CtrlRing.Cell[SwIdx].pNextNdisPacket = NULL;

	pAd->CtrlRing.Cell[SwIdx].PacketPa = PCI_MAP_SINGLE(pAd, (pSrcBufVA) , (SrcBufLen), 0, RTMP_PCI_DMA_TODEVICE);

	pTxD->LastSec0 = 1;
	pTxD->LastSec1 = 0;
	pTxD->SDLen0 = SrcBufLen;
	pTxD->SDLen1 = 0;
	pTxD->SDPtr0 = pAd->CtrlRing.Cell[SwIdx].PacketPa;
	pTxD->Burst = 0;
	pTxD->DMADONE = 0;

#ifdef RT_BIG_ENDIAN
	RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
	WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, FALSE, TYPE_TXD);
#endif

	/* flush dcache if no consistent memory is supported */
	RTMP_DCACHE_FLUSH(SrcBufPA, SrcBufLen);
	RTMP_DCACHE_FLUSH(pAd->CtrlRing.Cell[SwIdx].AllocPa, TXD_SIZE);

   	/* Increase TX_CTX_IDX, but write to register later.*/
	INC_RING_INDEX(pAd->CtrlRing.TxCpuIdx, MGMT_RING_SIZE);

	if (msg->need_wait)
		AndesQueueTailCmdMsg(&ctl->ackq, msg, wait_ack);
	else
		AndesQueueTailCmdMsg(&ctl->tx_doneq, msg, tx_done);

	if (!OS_TEST_BIT(MCU_INIT, &ctl->flags)) {
		RTMP_SPIN_UNLOCK_IRQRESTORE(&pAd->CtrlRingLock, &flags);
		return -1;
	}

	RTMP_IO_WRITE32(pAd, pAd->CtrlRing.hw_cidx_addr, pAd->CtrlRing.TxCpuIdx);

	RTMP_SPIN_UNLOCK_IRQRESTORE(&pAd->CtrlRingLock, &flags);

	return ret;
}
#endif /* RTMP_PCI_SUPPORT */

VOID AndesMTFillCmdHeader(struct cmd_msg *msg, PNDIS_PACKET net_pkt)
{
	FW_TXD *fw_txd;
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)msg->priv;
	struct MCU_CTRL *Ctl = &pAd->MCUCtrl;

	if ((Ctl->Stage == FW_NO_INIT) || (Ctl->Stage == FW_DOWNLOAD) || (Ctl->Stage == ROM_PATCH_DOWNLOAD))
		fw_txd = (FW_TXD *)OS_PKT_HEAD_BUF_EXTEND(net_pkt, 12);
	else if (Ctl->Stage == FW_RUN_TIME)
		fw_txd = (FW_TXD *)OS_PKT_HEAD_BUF_EXTEND(net_pkt, sizeof(*fw_txd));
	else
	{
		DBGPRINT(RT_DEBUG_OFF, ("%s: Unknown Control Stage(%d)\n", __FUNCTION__,
								Ctl->Stage));
		return;
	}

	fw_txd->fw_txd_0.field.length = GET_OS_PKT_LEN(net_pkt);
	fw_txd->fw_txd_0.field.pq_id = msg->pq_id;
	fw_txd->fw_txd_1.field.cid = msg->cmd_type;
	fw_txd->fw_txd_1.field.pkt_type_id = PKT_ID_CMD;
	fw_txd->fw_txd_1.field.set_query = msg->set_query;
	fw_txd->fw_txd_1.field.seq_num = msg->seq;
	fw_txd->fw_txd_2.field.ext_cid =  msg->ext_cmd_type;

	if ((msg->cmd_type == EXT_CID) && ((msg->set_query == CMD_SET) || (msg->set_query == CMD_QUERY))
									&& (msg->need_rsp == TRUE))
	{
		fw_txd->fw_txd_2.field.ext_cid_option = EXT_CID_OPTION_NEED_ACK;
	}
	else
	{
		fw_txd->fw_txd_2.field.ext_cid_option = EXT_CID_OPTION_NO_NEED_ACK;
	}

	fw_txd->fw_txd_0.word = cpu2le32(fw_txd->fw_txd_0.word);
	fw_txd->fw_txd_1.word = cpu2le32(fw_txd->fw_txd_1.word);
	fw_txd->fw_txd_2.word = cpu2le32(fw_txd->fw_txd_2.word);

#ifdef CONFIG_TRACE_SUPPORT
	TRACE_MCU_CMD_INFO(fw_txd->fw_txd_0.field.length, fw_txd->fw_txd_0.field.pq_id,
						fw_txd->fw_txd_1.field.cid, fw_txd->fw_txd_1.field.pkt_type_id,
						fw_txd->fw_txd_1.field.set_query, fw_txd->fw_txd_1.field.seq_num,
						fw_txd->fw_txd_2.field.ext_cid, fw_txd->fw_txd_2.field.ext_cid_option,
						(char *)(GET_OS_PKT_DATAPTR(net_pkt)), GET_OS_PKT_LEN(net_pkt));
#endif /* CONFIG_TRACE_SUPPORT */
}


INT32 CmdInitAccessRegWrite(RTMP_ADAPTER *ad, UINT32 address, UINT32 data)
{
	struct cmd_msg *msg;
	struct _INIT_CMD_ACCESS_REG access_reg;
	INT32 ret = 0;

	DBGPRINT(RT_DEBUG_INFO, ("%s: address = %x, data = %x\n", __FUNCTION__, address, data));

	msg = AndesAllocCmdMsg(ad, sizeof(struct _INIT_CMD_ACCESS_REG));

	if (!msg)
	{
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	AndesInitCmdMsg(msg, P1_Q0, INIT_CMD_ACCESS_REG, CMD_SET, EXT_CMD_NA, FALSE, 0, FALSE, FALSE, 0, NULL, NULL);

	memset(&access_reg, 0x00, sizeof(access_reg));

	access_reg.ucSetQuery = 1;
	access_reg.u4Address = cpu2le32(address);
	access_reg.u4Data = cpu2le32(data);

	AndesAppendCmdMsg(msg, (char *)&access_reg, sizeof(access_reg));

	ret = AndesSendCmdMsg(ad, msg);

error:
	DBGPRINT(RT_DEBUG_INFO, ("%s:(ret = %d)\n", __FUNCTION__, ret));
	return ret;
}


static VOID CmdInitAccessRegReadCb(struct cmd_msg *msg, char *data, UINT16 len)
{
	struct _INIT_EVENT_ACCESS_REG *access_reg = (struct _INIT_EVENT_ACCESS_REG *)data;

	NdisMoveMemory(msg->rsp_payload, &access_reg->u4Data, len - 4);
	*((UINT32 *)(msg->rsp_payload)) = le2cpu32(*((UINT32 *)msg->rsp_payload));
}


INT32 CmdInitAccessRegRead(RTMP_ADAPTER *pAd, UINT32 address, UINT32 *data)
{
	struct cmd_msg *msg;
	struct _INIT_CMD_ACCESS_REG access_reg;
	INT32 ret = 0;

	DBGPRINT(RT_DEBUG_INFO, ("%s: address = %x\n", __FUNCTION__, address));

	msg = AndesAllocCmdMsg(pAd, sizeof(struct _INIT_CMD_ACCESS_REG));

	if (!msg)
	{
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	AndesInitCmdMsg(msg, P1_Q0, INIT_CMD_ACCESS_REG, CMD_QUERY, EXT_CMD_NA, TRUE, 0,
							TRUE, TRUE, 8, (CHAR *)data, CmdInitAccessRegReadCb);

	memset(&access_reg, 0x00, sizeof(access_reg));

	access_reg.ucSetQuery = 0;
	access_reg.u4Address = cpu2le32(address);

	AndesAppendCmdMsg(msg, (char *)&access_reg, sizeof(access_reg));

	ret = AndesSendCmdMsg(pAd, msg);

error:
	DBGPRINT(RT_DEBUG_INFO, ("%s:(ret = %d)\n", __FUNCTION__, ret));
	return ret;
}


static VOID CmdReStartDLRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	FW_RXD_2 Status;
	Status = *(FW_RXD_2 *)Data;

	
	switch (Status.field.ext_eid)
	{
		case WIFI_FW_DOWNLOAD_SUCCESS:
			DBGPRINT(RT_DEBUG_OFF, ("%s: WiFI FW Download Success\n", __FUNCTION__));
			break;
		case WIFI_FW_DOWNLOAD_INVALID_PARAM:
			DBGPRINT(RT_DEBUG_OFF, ("%s: WiFi FW Download Invalid Parameter\n", __FUNCTION__));
			break;
		case WIFI_FW_DOWNLOAD_INVALID_CRC:
			DBGPRINT(RT_DEBUG_OFF, ("%s: WiFi FW Download Invalid CRC\n", __FUNCTION__));
			break;
		case WIFI_FW_DOWNLOAD_DECRYPTION_FAIL:
			DBGPRINT(RT_DEBUG_OFF, ("%s: WiFi FW Download Decryption Fail\n", __FUNCTION__));
			break;
		case WIFI_FW_DOWNLOAD_UNKNOWN_CMD:
			DBGPRINT(RT_DEBUG_OFF, ("%s: WiFi FW Download Unknown CMD\n", __FUNCTION__));
			break;
		case WIFI_FW_DOWNLOAD_TIMEOUT:
			DBGPRINT(RT_DEBUG_OFF, ("%s: WiFi FW Download Timeout\n", __FUNCTION__));
			break;
		default:
			DBGPRINT(RT_DEBUG_OFF, ("%s: Unknow Status(%u)\n", __FUNCTION__, Status.field.ext_eid));
			break;
	}
}


static VOID CmdSecKeyRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	P_EVENT_SEC_ADDREMOVE_STRUC_T EvtSecKey;
	UINT32 Status;
	UINT32 WlanIndex;

	EvtSecKey = (struct _EVENT_SEC_ADDREMOVE_STRUC_T *)Data;

	Status = le2cpu32(EvtSecKey->u4Status);
	WlanIndex = le2cpu32(EvtSecKey->u4WlanIdx);

	if (Status != 0) {
		DBGPRINT(RT_DEBUG_ERROR, ("%s, error set key, wlan idx(%d), status: 0x%x\n", __FUNCTION__, WlanIndex, Status));
	} else {
		DBGPRINT(RT_DEBUG_TRACE, ("%s, wlan idx(%d), status: 0x%x\n", __FUNCTION__, WlanIndex, Status));
	}
}

static VOID CmdPsRetrieveRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{

	P_EXT_EVENT_AP_PS_RETRIEVE_T EvtPsCapatibility;
	UINT32 Status;

    EvtPsCapatibility = (P_EXT_EVENT_AP_PS_RETRIEVE_T)Data;
        Status = le2cpu32(EvtPsCapatibility->u4Param1);

   DBGPRINT(RT_DEBUG_ERROR, ("%s Disable FW PS Supportstatus:%x !!!!!\n",__FUNCTION__,Status));

}

#ifdef MT_PS
#if defined(MT7603) && defined(RTMP_PCI_SUPPORT)
static VOID CmdPsRetrieveStartRspFromCR(RTMP_ADAPTER *pAd, char *Data, UINT16 Len)
{
	MAC_TABLE_ENTRY *pEntry;
	P_EXT_EVENT_AP_PS_RETRIEVE_T EvtPsRetrieveStart;
	UINT32 WlanIdx;
	STA_TR_ENTRY *tr_entry;
	NDIS_STATUS token_status;
	unsigned char q_idx;
	struct tx_swq_fifo *ps_fifo_swq;
	UINT deq_qid;
	//unsigned long IrqFlags;

   EvtPsRetrieveStart = (P_EXT_EVENT_AP_PS_RETRIEVE_T)Data;
   WlanIdx = le2cpu32(EvtPsRetrieveStart->u4Param1);

   if (!(VALID_TR_WCID(WlanIdx))) {
		DBGPRINT(RT_DEBUG_ERROR | DBG_FUNC_PS, ("---->%s INVALID_TR_WCID(WlanIndex)\n", __FUNCTION__));
		goto NEXT;
   }

	if (WlanIdx >= MAX_LEN_OF_MAC_TABLE) {
		DBGPRINT(RT_DEBUG_ERROR, ("Invalid value of WlanIdx"));
		return;
	}


   pEntry = &pAd->MacTab.Content[WlanIdx];
	tr_entry = &pAd->MacTab.tr_entry[WlanIdx];	
	if (IS_ENTRY_NONE(pEntry))
	{
		tr_entry->ps_state = APPS_RETRIEVE_WAIT_EVENT;
		RTEnqueueInternalCmd(pAd, CMDTHREAD_PS_CLEAR, (VOID *)&WlanIdx, sizeof(UINT32));
		DBGPRINT(RT_DEBUG_TRACE | DBG_FUNC_PS,("---->%s Entry(wcid=%d) left.\n", __func__, WlanIdx));
		goto NEXT;
	}
   
	DBGPRINT(RT_DEBUG_INFO | DBG_FUNC_PS, ("---->%s: Start to send TOKEN frames, WlanIdx=%d\n", __FUNCTION__, WlanIdx));

	if (tr_entry->ps_state != APPS_RETRIEVE_START_PS)
	{
		DBGPRINT(RT_DEBUG_ERROR | DBG_FUNC_PS, ("---->%s Entry(wcid=%d) ps state(%d) is not APPS_RETRIEVE_START_PS\n", __FUNCTION__, WlanIdx, tr_entry->ps_state));
		goto NEXT;
	}

	tr_entry->ps_state = APPS_RETRIEVE_GOING;
	CheckSkipTX(pAd, pEntry);
	tr_entry->ps_qbitmap = 0;

	for (q_idx = 0; q_idx < NUM_OF_TX_RING; q_idx++)
	{
		UINT16 IsEmpty = IS_TXRING_EMPTY(pAd, q_idx);

		if (!IsEmpty)
		{
			token_status = RtmpEnqueueTokenFrame(pAd, &(pEntry->Addr[0]), 0, WlanIdx, 0, q_idx);
			if (!token_status)
				tr_entry->ps_qbitmap |= (1 << q_idx);
			else
				DBGPRINT(RT_DEBUG_ERROR, ("%s(%d) Fail:	Send TOKEN Frame, AC=%d\n", __FUNCTION__, __LINE__, q_idx));
		}
	}

	if (tr_entry->ps_qbitmap == 0)
	{
		q_idx = QID_AC_VO;

		token_status = RtmpEnqueueTokenFrame(pAd, &(pEntry->Addr[0]), 0, WlanIdx, 0, q_idx);
		
		if (!token_status)
			tr_entry->ps_qbitmap |= (1 << q_idx);
	}

	if (tr_entry->ps_qbitmap == 0)
	{
		tr_entry->ps_state = APPS_RETRIEVE_WAIT_EVENT;
		tr_entry->token_enq_all_fail = TRUE;
		RTEnqueueInternalCmd(pAd, CMDTHREAD_PS_CLEAR, (VOID *)&WlanIdx, sizeof(UINT32));
		DBGPRINT(RT_DEBUG_WARN | DBG_FUNC_PS, ("%s(%d): (ps_state = %d) token_enq_all_fail!! ==> send CMDTHREAD_PS_CLEAR cmd.\n", 
			__FUNCTION__, __LINE__, tr_entry->ps_state));
	}
	else
	{
		tr_entry->token_enq_all_fail = FALSE;
	}
   
NEXT:
	ps_fifo_swq = &pAd->apps_cr_q;
	deq_qid = ps_fifo_swq->deqIdx;
	while (ps_fifo_swq->swq[deq_qid] != 0) {
		WlanIdx = ps_fifo_swq->swq[deq_qid];
		pEntry = &pAd->MacTab.Content[WlanIdx];
   tr_entry = &pAd->MacTab.tr_entry[WlanIdx];

		if (pEntry->PsMode == PWR_ACTIVE) {
			ps_fifo_swq->swq[deq_qid]  = 0;
			INC_RING_INDEX(ps_fifo_swq->deqIdx, TX_SWQ_FIFO_LEN);	
			tr_entry->ps_state = APPS_RETRIEVE_IDLE;
			MtHandleRxPsPoll(pAd, &pEntry->Addr[0], WlanIdx, TRUE);
			deq_qid = ps_fifo_swq->deqIdx;
		} else {
			if (MtStartPSRetrieve(pAd, ps_fifo_swq->swq[deq_qid]) == TRUE) {
				ps_fifo_swq->swq[deq_qid]  = 0;
				INC_RING_INDEX(ps_fifo_swq->deqIdx, TX_SWQ_FIFO_LEN);	
				tr_entry->ps_state = APPS_RETRIEVE_START_PS;
				tr_entry->ps_start_time = jiffies;
			}
			break;
		}
	}
}
#endif /* MT7603 && RTMP_PCI_SUPPORT  */

VOID AndesPsRetrieveStartRsp(RTMP_ADAPTER *pAd, char *Data, UINT16 Len)
	{
	MAC_TABLE_ENTRY *pEntry;
	P_EXT_EVENT_AP_PS_RETRIEVE_T EvtPsRetrieveStart;
	UINT32 WlanIdx;
	STA_TR_ENTRY *tr_entry;
	NDIS_STATUS token_status;
	unsigned char q_idx;

	EvtPsRetrieveStart = (P_EXT_EVENT_AP_PS_RETRIEVE_T)Data;
	WlanIdx = le2cpu32(EvtPsRetrieveStart->u4Param1);

	if (!(VALID_TR_WCID(WlanIdx))) {
		DBGPRINT(RT_DEBUG_ERROR | DBG_FUNC_PS, ("---->%s INVALID_TR_WCID(WlanIndex)\n", __FUNCTION__));
      return;
   }

	if (WlanIdx >= MAX_LEN_OF_MAC_TABLE) {
		DBGPRINT(RT_DEBUG_ERROR, ("Invalid value of WlanIdx"));
		return;
	}

	pEntry = &pAd->MacTab.Content[WlanIdx];
	tr_entry = &pAd->MacTab.tr_entry[WlanIdx];
	if (IS_ENTRY_NONE(pEntry))
	{
		MtPsRedirectDisableCheck(pAd, WlanIdx);
		DBGPRINT(RT_DEBUG_TRACE | DBG_FUNC_PS, ("---->%s Entry(wcid=%d) left.\n", __FUNCTION__, WlanIdx));
		return;
	}

	DBGPRINT(RT_DEBUG_INFO | DBG_FUNC_PS, ("---->%s: Start to send TOKEN frames, WlanIdx=%d\n", __FUNCTION__, WlanIdx));

   tr_entry->ps_state = APPS_RETRIEVE_GOING;
	tr_entry->ps_qbitmap = 0;

	for (q_idx = 0; q_idx < NUM_OF_TX_RING; q_idx++)
	   {
		   //UINT16 IsEmpty = IS_TXRING_EMPTY(pAd, q_idx);
		   UINT16 IsEmpty = (pAd->TxRing[q_idx].TxDmaIdx == pAd->TxRing[q_idx].TxCpuIdx) ? 1: 0;

		   if (!IsEmpty)
		   {
			   token_status = RtmpEnqueueTokenFrame(pAd, &(pEntry->Addr[0]), 0, WlanIdx, 0, q_idx);

   if (!token_status)
			   {
				   tr_entry->ps_qbitmap |= (1 << q_idx);
			   }
   else
			   {
				   DBGPRINT(RT_DEBUG_ERROR, ("%s(%d) Fail: Send TOKEN Frame, AC=%d\n", __FUNCTION__, __LINE__, q_idx));
			   }
		   }
	   }

	   if (tr_entry->ps_qbitmap == 0)
	   {
		   q_idx = QID_AC_VO;

		   token_status = RtmpEnqueueTokenFrame(pAd, &(pEntry->Addr[0]), 0, WlanIdx, 0, q_idx); 	   

   if (!token_status)
			   tr_entry->ps_qbitmap |= (1 << q_idx);
	   }

	   if (tr_entry->ps_qbitmap == 0)
	   {
		   tr_entry->ps_state = APPS_RETRIEVE_WAIT_EVENT;
		   tr_entry->token_enq_all_fail = TRUE;
		   RTEnqueueInternalCmd(pAd, CMDTHREAD_PS_CLEAR, (VOID *)&WlanIdx, sizeof(UINT32));
		   DBGPRINT(RT_DEBUG_WARN | DBG_FUNC_PS, ("%s(%d): (ps_state = %d) token_enq_all_fail!! ==> send CMDTHREAD_PS_CLEAR cmd.\n", 
			   __FUNCTION__, __LINE__, tr_entry->ps_state)); 
	   }
   else
	   {
		   tr_entry->token_enq_all_fail = FALSE;
	   }
}

static VOID CmdPsRetrieveStartRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	RTMP_ADAPTER *ad = NULL;

	ad = (RTMP_ADAPTER *)msg->priv;

	DBGPRINT(RT_DEBUG_ERROR, ("------> %s(%d)\n", __FUNCTION__, __LINE__));
#ifdef RTMP_PCI_SUPPORT
	AndesPsRetrieveStartRsp(ad, Data, Len);
#endif /* RTMP_PCI_SUPPORT */
	
#if defined(RTMP_USB_SUPPORT) || defined(RTMP_SDIO_SUPPORT)
	/*
		We need to move AndesPsRetrieveStartRsp to cmd thread for CR read/write.
	*/
	RTEnqueueInternalCmd(ad, CMDTHREAD_PS_RETRIEVE_RSP, Data, Len);
#endif /* defined(RTMP_USB_SUPPORT) || defined(RTMP_SDIO_SUPPORT) */
	DBGPRINT(RT_DEBUG_ERROR, ("<------ %s(%d)\n", __FUNCTION__, __LINE__));
}


static VOID CmdPsClearRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	//UINT8 Status;
    MAC_TABLE_ENTRY *pEntry;
	//QUEUE_ENTRY *pQEntry;
	P_CMD_AP_PS_CLEAR_STRUC_T EvtPsClear;
	RTMP_ADAPTER *ad = (RTMP_ADAPTER *)msg->priv;
	STA_TR_ENTRY *tr_entry;
	//unsigned long IrqFlags;
	//struct wtbl_entry tb_entry;
	UINT32 WlanIndex;
	//union WTBL_1_DW3 *dw3 = (union WTBL_1_DW3 *)&tb_entry.wtbl_1.wtbl_1_d3.word;
	unsigned char q_idx = 0;

	EvtPsClear = (struct _CMD_AP_PS_CLEAR_STRUC_T *)Data;
	WlanIndex = le2cpu32(EvtPsClear->u4WlanIdx);
        pEntry = &ad->MacTab.Content[WlanIndex];
	tr_entry = &ad->MacTab.tr_entry[WlanIndex];


	if (ad->MacTab.tr_entry[WlanIndex].PsMode == PWR_ACTIVE)
      tr_entry->ps_state = APPS_RETRIEVE_IDLE;
   else
      tr_entry->ps_state = APPS_RETRIEVE_DONE;

   DBGPRINT(RT_DEBUG_INFO | DBG_FUNC_PS, ("wcid=%d, Receive Event of CmdPsClear tr_entry->ps_state=%d\n", WlanIndex,tr_entry->ps_state));

	if(tr_entry->token_enq_all_fail)
	{
		tr_entry->token_enq_all_fail = FALSE;

		if (tr_entry->ps_queue.Number) 
			MtEnqTxSwqFromPsQueue(ad, q_idx, tr_entry);

		for (q_idx = 0; q_idx < NUM_OF_TX_RING; q_idx++)
		{
			if (q_idx < WMM_QUE_NUM)
				tr_entry->TokenCount[q_idx] = tr_entry->tx_queue[q_idx].Number;
			else
				break;
		}
	}
	if(IS_ENTRY_NONE(pEntry)) {
		DBGPRINT(RT_DEBUG_TRACE | DBG_FUNC_PS, ("wcid=%d, pEntry none when CmdPsClearRsp\n", WlanIndex));
		return;
	}

#ifdef RTMP_MAC_PCI
#ifdef DOT11_N_SUPPORT
   SendRefreshBAR(ad, pEntry);
#endif /* DOT11_N_SUPPORT */
#endif /* RTMP_MAC_PCI */
   if (tr_entry->ps_state == APPS_RETRIEVE_IDLE)
      MtHandleRxPsPoll(ad, &pEntry->Addr[0], WlanIndex, TRUE);
#ifdef UAPSD_SUPPORT
	else
	{
		if (tr_entry->bEospNullSnd)
		{
			//UINT32	AcQueId;

			tr_entry->bEospNullSnd = FALSE;
			/* sanity Check for UAPSD condition */
			if (tr_entry->EospNullUp >= 8)
				tr_entry->EospNullUp = 1; /* shout not be here */

			/* get the AC ID of incoming packet */
			//AcQueId = WMM_UP2AC_MAP[tr_entry->EospNullUp];
			
			/* bQosNull = bEOSP = TRUE = 1 */

			/*
				Use management queue to tx QoS Null frame to avoid delay so
				us_of_frame is not used.
			*/
#ifdef CONFIG_AP_SUPPORT
#ifdef MT_PS			
			if (pEntry->i_psm == I_PSM_DISABLE)
			{
				MT_SET_IGNORE_PSM(ad, pEntry, I_PSM_ENABLE);
			}
#endif /* MT_PS */
#endif /* CONFIG_AP_SUPPORT */

			RtmpEnqueueNullFrame(ad, pEntry->Addr, pEntry->CurrTxRate,
								pEntry->Aid, pEntry->func_tb_idx, TRUE, TRUE, tr_entry->EospNullUp);

#ifdef UAPSD_DEBUG
			DBGPRINT(RT_DEBUG_ERROR, ("%s: send a EOSP QoS Null frame!\n", __FUNCTION__));
#endif /* UAPSD_DEBUG */
		}
		else if (pEntry->UAPSDTxNum != 0)
		{
			RTMPDeQueuePacket(ad, TRUE, NUM_OF_TX_RING, pEntry->wcid, pEntry->UAPSDTxNum);
		}
	}
#endif /* UAPSD_SUPPORT */
}
#endif /* MT_PS */

static VOID CmdStartDLRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	UINT8 Status;

	Status = *Data;

	switch (Status)
	{
		case WIFI_FW_DOWNLOAD_SUCCESS:
			DBGPRINT(RT_DEBUG_OFF, ("%s: WiFI FW Download Success\n", __FUNCTION__));
			break;
		case WIFI_FW_DOWNLOAD_INVALID_PARAM:
			DBGPRINT(RT_DEBUG_OFF, ("%s: WiFi FW Download Invalid Parameter\n", __FUNCTION__));
			break;
		case WIFI_FW_DOWNLOAD_INVALID_CRC:
			DBGPRINT(RT_DEBUG_OFF, ("%s: WiFi FW Download Invalid CRC\n", __FUNCTION__));
			break;
		case WIFI_FW_DOWNLOAD_DECRYPTION_FAIL:
			DBGPRINT(RT_DEBUG_OFF, ("%s: WiFi FW Download Decryption Fail\n", __FUNCTION__));
			break;
		case WIFI_FW_DOWNLOAD_UNKNOWN_CMD:
			DBGPRINT(RT_DEBUG_OFF, ("%s: WiFi FW Download Unknown CMD\n", __FUNCTION__));
			break;
		case WIFI_FW_DOWNLOAD_TIMEOUT:
			DBGPRINT(RT_DEBUG_OFF, ("%s: WiFi FW Download Timeout\n", __FUNCTION__));
			break;
		default:
			DBGPRINT(RT_DEBUG_OFF, ("%s: Unknow Status(%d)\n", __FUNCTION__, Status));
			break;
	}
}

INT32 CmdSecKeyReq(RTMP_ADAPTER *ad, UINT8 AddRemove, UINT8 Keytype, UINT8 *pAddr, UINT8 Alg, UINT8 KeyID, UINT8 KeyLen, UINT8 WlanIdx, UINT8 *KeyMaterial)
{
	struct cmd_msg *msg;
	struct _CMD_SEC_ADDREMOVE_KEY_STRUC_T CmdSecKey;
	int ret = 0;

	msg = AndesAllocCmdMsg(ad, sizeof(struct _CMD_SEC_ADDREMOVE_KEY_STRUC_T));

	if (!msg)
	{
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	AndesInitCmdMsg(msg, P1_Q0, EXT_CID, CMD_SET, EXT_CMD_SEC_ADDREMOVE_KEY, FALSE, 0, FALSE, TRUE, sizeof(struct _EVENT_SEC_ADDREMOVE_STRUC_T), NULL, CmdSecKeyRsp);

	memset(&CmdSecKey, 0x00, sizeof(CmdSecKey));

	CmdSecKey.ucAddRemove = AddRemove;
	CmdSecKey.ucKeyType = Keytype;
	memcpy(CmdSecKey.aucPeerAddr, pAddr, 6);
	CmdSecKey.ucAlgorithmId = Alg;
	CmdSecKey.ucKeyId = KeyID;
	CmdSecKey.ucKeyLen = KeyLen;
	memcpy(CmdSecKey.aucKeyMaterial, KeyMaterial, KeyLen);
	CmdSecKey.ucWlanIndex = WlanIdx;
	AndesAppendCmdMsg(msg, (char *)&CmdSecKey, sizeof(CmdSecKey));

	ret = AndesSendCmdMsg(ad, msg);

error:
	DBGPRINT(RT_DEBUG_INFO, ("%s:(ret = %d)\n", __FUNCTION__, ret));
	return ret;
}

INT32 CmdPsRetrieveReq(RTMP_ADAPTER *ad, UINT32 enable)
{
   	struct cmd_msg *msg;
      struct _CMD_AP_PS_RETRIEVE_T CmdPsCapatibility;

      int ret = 0;

      msg = AndesAllocCmdMsg(ad, sizeof(struct _CMD_AP_PS_RETRIEVE_T));

      if (!msg)
      {
         ret = NDIS_STATUS_RESOURCES;
         goto error;
      }

      AndesInitCmdMsg(msg, P1_Q0, EXT_CID, CMD_SET, EXT_CMD_AP_PWR_SAVING_CAPABILITY, TRUE, 0, TRUE, TRUE, sizeof(struct _CMD_AP_PS_RETRIEVE_T), NULL, CmdPsRetrieveRsp);

      NdisZeroMemory(&CmdPsCapatibility, sizeof(CmdPsCapatibility));

      CmdPsCapatibility.u4Option=  cpu2le32(0);
      CmdPsCapatibility.u4Param1=  cpu2le32(enable);

      AndesAppendCmdMsg(msg, (char *)&CmdPsCapatibility, sizeof(CmdPsCapatibility));
      ret = AndesSendCmdMsg(ad, msg);

error:
      DBGPRINT(RT_DEBUG_INFO, ("%s:(ret = %d)\n", __FUNCTION__, ret));
   	return ret;
}

#ifdef MT_PS
INT32 CmdPsRetrieveStartReq(RTMP_ADAPTER *ad, UINT32 WlanIdx)
{
   struct cmd_msg *msg;
   struct _EXT_CMD_AP_PWS_START_T CmdApPwsStart;

   int ret = 0;

   /*how to handle memory allocate failure? */
   msg = AndesAllocCmdMsg(ad, sizeof(struct _EXT_CMD_AP_PWS_START_T));
   if (!msg)
   {
      ret = NDIS_STATUS_RESOURCES;
	DBGPRINT(RT_DEBUG_ERROR | DBG_FUNC_PS, ("%s:(ret = %d)\n", __FUNCTION__, ret));
      goto error;
   }

   DBGPRINT(RT_DEBUG_INFO | DBG_FUNC_PS, ("%s(%d): RTEnqueueInternalCmd comming!! WlanIdx: %x\n",__FUNCTION__, __LINE__,WlanIdx));

   AndesInitCmdMsg(msg, P1_Q0, EXT_CID, CMD_SET, EXT_CMD_PS_RETRIEVE_START, TRUE, 0, TRUE, TRUE, sizeof(struct _EXT_CMD_AP_PWS_START_T), NULL, CmdPsRetrieveStartRsp);

   NdisZeroMemory(&CmdApPwsStart, sizeof(CmdApPwsStart));

   CmdApPwsStart.u4WlanIdx= cpu2le32(WlanIdx);

   AndesAppendCmdMsg(msg, (char *)&CmdApPwsStart, sizeof(CmdApPwsStart));
   msg->wcid = WlanIdx;
   ret = AndesSendCmdMsg(ad, msg);

error:
   	return ret;
}



INT32 CmdPsClearReq(RTMP_ADAPTER *ad, UINT32 wlanidx, BOOLEAN p_wait)
{
	struct cmd_msg *msg;
	struct _CMD_AP_PS_CLEAR_STRUC_T CmdPsClear;
	int ret = 0;

	msg = AndesAllocCmdMsg(ad, sizeof(struct _CMD_AP_PS_CLEAR_STRUC_T));

	if (!msg)
	{
		ret = NDIS_STATUS_RESOURCES;
	DBGPRINT(RT_DEBUG_ERROR | DBG_FUNC_PS, ("%s:(ret = %d)\n", __FUNCTION__, ret));
		goto error;
	}

	DBGPRINT(RT_DEBUG_INFO | DBG_FUNC_PS, ("%s(%d): RTEnqueueInternalCmd comming!! WlanIdx: %x\n",__FUNCTION__, __LINE__,wlanidx));

	AndesInitCmdMsg(msg, P1_Q0, EXT_CID, CMD_QUERY, EXT_CMD_PWR_SAVING, TRUE, 0, TRUE, TRUE, sizeof(struct _CMD_AP_PS_CLEAR_STRUC_T), NULL, CmdPsClearRsp);

	NdisZeroMemory(&CmdPsClear, sizeof(CmdPsClear));

	CmdPsClear.u4WlanIdx = cpu2le32(wlanidx);
	CmdPsClear.u4Status = cpu2le32(0);

	AndesAppendCmdMsg(msg, (char *)&CmdPsClear, sizeof(CmdPsClear));
	msg->wcid = wlanidx;
	ret = AndesSendCmdMsg(ad, msg);

error:
	return ret;
}
#endif /* MT_PS */

static INT32 CmdRestartDLReq(RTMP_ADAPTER *ad)
{
	struct cmd_msg *msg;
	int ret = 0;

	msg = AndesAllocCmdMsg(ad, 0);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	AndesInitCmdMsg(msg, P1_Q0, MT_RESTART_DL_REQ, CMD_NA, EXT_CMD_NA, TRUE, 0, TRUE, TRUE, 0, NULL, CmdReStartDLRsp);

	ret = AndesSendCmdMsg(ad, msg);

error:
	DBGPRINT(RT_DEBUG_INFO, ("%s:(ret = %d)\n", __FUNCTION__, ret));
	return ret;
}

/*Nobody uses it currently*/

static VOID CmdAddrellLenRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	UINT8 Status;

	Status = *Data;

	switch (Status)
	{
		case TARGET_ADDRESS_LEN_SUCCESS:
			DBGPRINT(RT_DEBUG_INFO, ("%s: Request target address and length success\n", __FUNCTION__));
			break;
		default:
			DBGPRINT(RT_DEBUG_ERROR, ("%s: Unknow Status(%d)\n", __FUNCTION__, Status));
			break;
	}
}


static INT32 CmdAddressLenReq(RTMP_ADAPTER *ad, UINT32 address, UINT32 len, UINT32 data_mode)
{
	struct cmd_msg *msg;
	int ret = 0;
	UINT32 value;

	DBGPRINT(RT_DEBUG_TRACE, ("Start address = %x, DL length = %d, Data mode = %x\n",
									address, len, data_mode));
	msg = AndesAllocCmdMsg(ad, 12);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	if (address == ROM_PATCH_START_ADDRESS)
		AndesInitCmdMsg(msg, P1_Q0, MT_PATCH_START_REQ, CMD_NA, EXT_CMD_NA, TRUE, 0, TRUE, TRUE, 0, NULL, CmdAddrellLenRsp);
	else
		AndesInitCmdMsg(msg, P1_Q0, MT_TARGET_ADDRESS_LEN_REQ, CMD_NA, EXT_CMD_NA, TRUE, 0, TRUE, TRUE, 0, NULL, CmdAddrellLenRsp);

	/* start address */
	value = cpu2le32(address);
	AndesAppendCmdMsg(msg, (char *)&value, 4);

	/* dl length */
	value = cpu2le32(len);
	AndesAppendCmdMsg(msg, (char *)&value, 4);

	/* data mode */
	value = cpu2le32(data_mode);
	AndesAppendCmdMsg(msg, (char *)&value, 4);

	ret = AndesSendCmdMsg(ad, msg);

error:
	DBGPRINT(RT_DEBUG_OFF, ("%s:(ret = %d)\n", __FUNCTION__, ret));
	return ret;
}

static INT32 CmdFwScatter(RTMP_ADAPTER *ad, UINT8 *dl_payload, UINT32 dl_len, UINT32 count)
{
	struct cmd_msg *msg;
	int ret = 0;

	msg = AndesAllocCmdMsg(ad, dl_len);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	AndesInitCmdMsg(msg, 0xC000, MT_FW_SCATTER, CMD_NA, EXT_CMD_NA, FALSE, 0, FALSE, FALSE, 0, NULL, NULL);

	AndesAppendCmdMsg(msg, (char *)dl_payload, dl_len);

	ret = AndesSendCmdMsg(ad, msg);

error:
	DBGPRINT(RT_DEBUG_INFO, ("%s:(scatter = %d, ret = %d)\n", __FUNCTION__, count, ret));
	return ret;
}

static INT32 CmdFwScatters(RTMP_ADAPTER *ad, UINT8 *image, UINT32 image_len)
{
	INT32 sent_len;
	UINT32 cur_len = 0, count = 0;
	RTMP_CHIP_CAP *cap = &ad->chipCap;
	int ret = 0;

	while (1)
	{
		INT32 sent_len_max = MT_UPLOAD_FW_UNIT - cap->cmd_header_len;
		sent_len = (image_len - cur_len) >=  sent_len_max ? sent_len_max : (image_len - cur_len);

		if (sent_len > 0) {
			ret = CmdFwScatter(ad, image + cur_len, sent_len, count);
			count++;
			if (ret)
				goto error;
			cur_len += sent_len;
		} else {
			break;
		}
	}

error:
	DBGPRINT(RT_DEBUG_INFO, ("%s:(ret = %d)\n", __FUNCTION__, ret));
	return ret;
}


static VOID CmdPatchFinishRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	UINT8 Status;

	Status = *Data;

	switch (Status)
	{
		case WIFI_FW_DOWNLOAD_SUCCESS:
			DBGPRINT(RT_DEBUG_OFF, ("%s: WiFI ROM Patch Download Success\n", __FUNCTION__));
			break;
		default:
			DBGPRINT(RT_DEBUG_OFF, ("%s: WiFi ROM Patch Fail (%d)\n", __FUNCTION__, Status));
			break;
	}
}


static INT32 CmdPatchFinishReq(RTMP_ADAPTER *ad)
{
	struct cmd_msg *msg;
	int ret = 0;

	DBGPRINT(RT_DEBUG_OFF, ("%s\n", __FUNCTION__));

	msg = AndesAllocCmdMsg(ad, 0);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	AndesInitCmdMsg(msg, P1_Q0, MT_PATCH_FINISH_REQ, CMD_NA, EXT_CMD_NA, TRUE, 0, TRUE, TRUE, 0, NULL, CmdPatchFinishRsp);

	ret = AndesSendCmdMsg(ad, msg);

error:
	DBGPRINT(RT_DEBUG_INFO, ("%s:(ret = %d)\n", __FUNCTION__, ret));
	return ret;
}


static INT32 CmdFwStartReq(RTMP_ADAPTER *ad, UINT32 override, UINT32 address)
{
	struct cmd_msg *msg;
	int ret = 0;
	UINT32 value;

	DBGPRINT(RT_DEBUG_OFF, ("%s: override = %d, address = %d\n", __FUNCTION__, override, address));

	msg = AndesAllocCmdMsg(ad, 8);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	AndesInitCmdMsg(msg, P1_Q0, MT_FW_START_REQ, CMD_NA, EXT_CMD_NA, TRUE, 0, TRUE, TRUE, 0, NULL, CmdStartDLRsp);

	/* override */
	value = cpu2le32(override);
	AndesAppendCmdMsg(msg, (char *)&value, 4);

	/* entry point address */
	value = cpu2le32(address);

	AndesAppendCmdMsg(msg, (char *)&value, 4);

	ret = AndesSendCmdMsg(ad, msg);

error:
	DBGPRINT(RT_DEBUG_INFO, ("%s:(ret = %d)\n", __FUNCTION__, ret));
	return ret;
}


INT32 CmdChPrivilege(RTMP_ADAPTER *ad, UINT8 Action, UINT8 control_chl, UINT8 central_chl,
							UINT8 BW, UINT8 TXStream, UINT8 RXStream)
{
	struct cmd_msg *msg;
	struct _CMD_CH_PRIVILEGE_T ch_privilege;
	INT32 ret = 0;
	struct MCU_CTRL *Ctl = &ad->MCUCtrl;

	if (central_chl == 0)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s: central channel = 0 is invalid\n", __FUNCTION__));
		return -1;
	}

	DBGPRINT(RT_DEBUG_INFO, ("%s: control_chl = %d, central_chl = %d, BW = %d,	\
								TXStream = %d, RXStream = %d\n", __FUNCTION__,	\
							control_chl, central_chl, BW, TXStream, RXStream));

	msg = AndesAllocCmdMsg(ad, sizeof(struct _CMD_CH_PRIVILEGE_T));

	if (!msg)
	{
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	AndesInitCmdMsg(msg, P1_Q0, CMD_CH_PRIVILEGE, CMD_SET, EXT_CMD_NA, FALSE, 0, FALSE, FALSE, 0, NULL, NULL);

	memset(&ch_privilege, 0x00, sizeof(ch_privilege));

	ch_privilege.ucAction = Action;
	ch_privilege.ucPrimaryChannel = control_chl;

	if (BW == BAND_WIDTH_20)
	{
		ch_privilege.ucRfSco = CMD_CH_PRIV_SCO_SCN;
	}
	else if (BW == BAND_WIDTH_40)
	{
		if (control_chl < central_chl)
			ch_privilege.ucRfSco = CMD_CH_PRIV_SCO_SCA;
		else
			ch_privilege.ucRfSco = CMD_CH_PRIV_SCO_SCB;
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("unknown bandwidth = %d\n", BW));
	}

	if (central_chl > 14)
		ch_privilege.ucRfBand =  CMD_CH_PRIV_BAND_A;
	else
		ch_privilege.ucRfBand = CMD_CH_PRIV_BAND_G;

	ch_privilege.ucRfChannelWidth = CMD_CH_PRIV_CH_WIDTH_20_40;

	ch_privilege.ucReqType = CMD_CH_PRIV_REQ_JOIN;

	AndesAppendCmdMsg(msg, (char *)&ch_privilege, sizeof(ch_privilege));

	if (IS_MT7603(ad) || IS_MT7628(ad))
	{
		UINT32 Value;
		RTMP_IO_READ32(ad, RMAC_RMCR, &Value);

		if (Value & RMAC_RMCR_RX_STREAM_0)
			Ctl->RxStream0 = 1;

		if (Value & RMAC_RMCR_RX_STREAM_1)
			Ctl->RxStream1 = 1;

		Value |= RMAC_RMCR_RX_STREAM_0;
		Value |= RMAC_RMCR_RX_STREAM_1;
		RTMP_IO_WRITE32(ad, RMAC_RMCR, Value);
	}

	ret = AndesSendCmdMsg(ad, msg);

error:
	DBGPRINT(RT_DEBUG_INFO, ("%s:(ret = %d)\n", __FUNCTION__, ret));
	return ret;
}


static VOID CmdMultipleMacRegAccessWriteCb(struct cmd_msg *msg, 
											char *data, UINT16 len)
{
	EXT_EVENT_MULTI_CR_ACCESS_WR_T *EventMultiCRAccessWR
								= (EXT_EVENT_MULTI_CR_ACCESS_WR_T *)(data + 20);

	EventMultiCRAccessWR->u4Status = le2cpu32(EventMultiCRAccessWR->u4Status);

	if (EventMultiCRAccessWR->u4Status)
		DBGPRINT(RT_DEBUG_ERROR, ("%s: fail\n", __FUNCTION__));
}


INT32 CmdMultipleMacRegAccessWrite(RTMP_ADAPTER *pAd, RTMP_REG_PAIR *RegPair, 
														UINT32 Num)
{
	struct cmd_msg *msg;
	CMD_MULTI_CR_ACCESS_T MultiCR;
	INT32 Ret;
	UINT32 Index;

	msg = AndesAllocCmdMsg(pAd, sizeof(CMD_MULTI_CR_ACCESS_T) * Num);

	if (!msg)
	{
		Ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	AndesInitCmdMsg(msg, P1_Q0, EXT_CID, CMD_SET, EXT_CMD_MULTIPLE_REG_ACCESS, 
				TRUE, 0, TRUE, TRUE, 32, NULL, CmdMultipleMacRegAccessWriteCb);

	for (Index = 0; Index < Num; Index++)
	{
		memset(&MultiCR, 0x00, sizeof(MultiCR));
		MultiCR.u4Type = cpu2le32(MAC_CR);
		MultiCR.u4Addr = cpu2le32(RegPair[Index].Register);
		MultiCR.u4Data = cpu2le32(RegPair[Index].Value);
//		DBGPRINT(RT_DEBUG_TRACE, ("%s: offset: = %x\n", __FUNCTION__,MultiCR.u4Addr));	
	
		AndesAppendCmdMsg(msg, (char *)&MultiCR, sizeof(MultiCR));
	}

	Ret = AndesSendCmdMsg(pAd, msg);

error:
	DBGPRINT(RT_DEBUG_INFO, ("%s:(ret = %d)\n", __FUNCTION__, Ret));
	return Ret;
}


static VOID CmdMultipleRfRegAccessWriteCb(struct cmd_msg *msg, 
											char *data, UINT16 len)
{
	EXT_EVENT_MULTI_CR_ACCESS_WR_T *EventMultiCRAccessWR
								= (EXT_EVENT_MULTI_CR_ACCESS_WR_T *)(data + 20);

	EventMultiCRAccessWR->u4Status = le2cpu32(EventMultiCRAccessWR->u4Status);

	if (EventMultiCRAccessWR->u4Status)
		DBGPRINT(RT_DEBUG_ERROR, ("%s: fail\n", __FUNCTION__));
}


INT32 CmdMultipleRfRegAccessWrite(RTMP_ADAPTER *pAd, MT_RF_REG_PAIR *RegPair, 
														UINT32 Num)
{
	struct cmd_msg *msg;
	CMD_MULTI_CR_ACCESS_T MultiCR;
	INT32 Ret;
	UINT32 Index;

	msg = AndesAllocCmdMsg(pAd, sizeof(CMD_MULTI_CR_ACCESS_T) * Num);

	if (!msg)
	{
		Ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	AndesInitCmdMsg(msg, P1_Q0, EXT_CID, CMD_SET, EXT_CMD_MULTIPLE_REG_ACCESS, 
				TRUE, 0, TRUE, TRUE, 32, NULL, CmdMultipleRfRegAccessWriteCb);

	for (Index = 0; Index < Num; Index++)
	{
		memset(&MultiCR, 0x00, sizeof(MultiCR));
		MultiCR.u4Type = cpu2le32((RF_CR & 0xff) | 
							((RegPair->WiFiStream & 0xffffff) << 8));
		MultiCR.u4Addr = cpu2le32(RegPair[Index].Register);
		MultiCR.u4Data = cpu2le32(RegPair[Index].Value);
	
		AndesAppendCmdMsg(msg, (char *)&MultiCR, sizeof(MultiCR));
	}

	Ret = AndesSendCmdMsg(pAd, msg);

error:
	DBGPRINT(RT_DEBUG_INFO, ("%s:(ret = %d)\n", __FUNCTION__, Ret));
	return Ret;
}


static VOID CmdMultipleRfRegAccessReadCb(struct cmd_msg *msg, 
											char *data, UINT16 len)
{
	UINT32 Index;
	UINT32 Num = (len -20) / sizeof(EXT_EVENT_MULTI_CR_ACCESS_RD_T);
	EXT_EVENT_MULTI_CR_ACCESS_RD_T *EventMultiCRAccessRD
								= (EXT_EVENT_MULTI_CR_ACCESS_RD_T *)(data + 20);
	MT_RF_REG_PAIR *RegPair = (MT_RF_REG_PAIR *)msg->rsp_payload;

	for (Index = 0; Index < Num; Index++)
	{
		RegPair->WiFiStream = (le2cpu32(EventMultiCRAccessRD->u4Type) 
								& (0xffffff << 8)) >> 8;
		RegPair->Register = le2cpu32(EventMultiCRAccessRD->u4Addr);
		RegPair->Value = le2cpu32(EventMultiCRAccessRD->u4Data);

		EventMultiCRAccessRD++;
		RegPair++;
	}
}


INT32 CmdMultiPleRfRegAccessRead(RTMP_ADAPTER *pAd, MT_RF_REG_PAIR *RegPair, 
														UINT32 Num)
{
	struct cmd_msg *msg;
	CMD_MULTI_CR_ACCESS_T MultiCR;
	INT32 Ret;
	UINT32 Index;

	msg = AndesAllocCmdMsg(pAd, sizeof(CMD_MULTI_CR_ACCESS_T) * Num);

	if (!msg)
	{
		Ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	AndesInitCmdMsg(msg, P1_Q0, EXT_CID, CMD_QUERY, EXT_CMD_MULTIPLE_REG_ACCESS, 
				TRUE, 0, TRUE, TRUE, (12 * Num) + 20, (char *)RegPair, 
				CmdMultipleRfRegAccessReadCb);

	for (Index = 0; Index < Num; Index++)
	{
		memset(&MultiCR, 0x00, sizeof(MultiCR));
		MultiCR.u4Type = cpu2le32((RF_CR & 0xff) | 
							((RegPair->WiFiStream & 0xffffff) << 8));
		MultiCR.u4Addr = cpu2le32(RegPair[Index].Register);
	
		AndesAppendCmdMsg(msg, (char *)&MultiCR, sizeof(MultiCR));
	}

	Ret = AndesSendCmdMsg(pAd, msg);

error:
	DBGPRINT(RT_DEBUG_INFO, ("%s:(ret = %d)\n", __FUNCTION__, Ret));
	return Ret;
}


static VOID CmdMultipleMacRegAccessReadCb(struct cmd_msg *msg, 
											char *data, UINT16 len)
{
	UINT32 Index;
	UINT32 Num = (len - 20) / sizeof(EXT_EVENT_MULTI_CR_ACCESS_RD_T);
	EXT_EVENT_MULTI_CR_ACCESS_RD_T *EventMultiCRAccessRD
								= (EXT_EVENT_MULTI_CR_ACCESS_RD_T *)(data + 20);
	RTMP_REG_PAIR *RegPair = (RTMP_REG_PAIR *)msg->rsp_payload;

	for (Index = 0; Index < Num; Index++)
	{
		RegPair->Register = le2cpu32(EventMultiCRAccessRD->u4Addr);
		RegPair->Value = le2cpu32(EventMultiCRAccessRD->u4Data);

		EventMultiCRAccessRD++;
		RegPair++;
	}
}


INT32 CmdMultiPleMacRegAccessRead(RTMP_ADAPTER *pAd, RTMP_REG_PAIR *RegPair, 
														UINT32 Num)
{
	struct cmd_msg *msg;
	CMD_MULTI_CR_ACCESS_T MultiCR;
	INT32 Ret;
	UINT32 Index;

	msg = AndesAllocCmdMsg(pAd, sizeof(CMD_MULTI_CR_ACCESS_T) * Num);

	if (!msg)
	{
		Ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	AndesInitCmdMsg(msg, P1_Q0, EXT_CID, CMD_QUERY, EXT_CMD_MULTIPLE_REG_ACCESS, 
				TRUE, 0, TRUE, TRUE, (12 * Num) + 20, (char *)RegPair, 
				CmdMultipleMacRegAccessReadCb);

	for (Index = 0; Index < Num; Index++)
	{
		memset(&MultiCR, 0x00, sizeof(MultiCR));
		MultiCR.u4Type = cpu2le32(MAC_CR);
		MultiCR.u4Addr = cpu2le32(RegPair[Index].Register);
	
		AndesAppendCmdMsg(msg, (char *)&MultiCR, sizeof(MultiCR));
	}

	Ret = AndesSendCmdMsg(pAd, msg);

error:
	DBGPRINT(RT_DEBUG_INFO, ("%s:(ret = %d)\n", __FUNCTION__, Ret));
	return Ret;
}


INT32 CmdAccessRegWrite(RTMP_ADAPTER *ad, UINT32 address, UINT32 data)
{
	struct cmd_msg *msg;
	struct _CMD_ACCESS_REG_T access_reg;
	INT32 ret = 0;

	DBGPRINT(RT_DEBUG_INFO, ("%s: address = %x, data = %x\n", __FUNCTION__, address, data));

	msg = AndesAllocCmdMsg(ad, sizeof(struct _CMD_ACCESS_REG_T));

	if (!msg)
	{
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	AndesInitCmdMsg(msg, P1_Q0, CMD_ACCESS_REG, CMD_SET, EXT_CMD_NA, FALSE, 0, FALSE, FALSE, 0, NULL, NULL);

	memset(&access_reg, 0x00, sizeof(access_reg));

	access_reg.u4Address = cpu2le32(address);
	access_reg.u4Data = cpu2le32(data);

	AndesAppendCmdMsg(msg, (char *)&access_reg, sizeof(access_reg));

	ret = AndesSendCmdMsg(ad, msg);

error:
	DBGPRINT(RT_DEBUG_INFO, ("%s:(ret = %d)\n", __FUNCTION__, ret));
	return ret;
}


static VOID CmdAccessRegReadCb(struct cmd_msg *msg, char *data, UINT16 len)
{
	struct _CMD_ACCESS_REG_T *access_reg = (struct _CMD_ACCESS_REG_T *)data;

	DBGPRINT(RT_DEBUG_INFO, ("%s\n", __FUNCTION__));
	
	NdisMoveMemory(msg->rsp_payload, &access_reg->u4Data, len - 4);
	*((UINT32 *)(msg->rsp_payload)) = le2cpu32(*((UINT32 *)msg->rsp_payload));
}


INT32 CmdAccessRegRead(RTMP_ADAPTER *pAd, UINT32 address, UINT32 *data)
{
	struct cmd_msg *msg;
	struct _CMD_ACCESS_REG_T access_reg;
	INT32 ret = 0;

	msg = AndesAllocCmdMsg(pAd, sizeof(struct _CMD_ACCESS_REG_T));

	if (!msg)
	{
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	AndesInitCmdMsg(msg, P1_Q0, CMD_ACCESS_REG, CMD_QUERY, EXT_CMD_NA, TRUE, 0,
							TRUE, TRUE, 8, (CHAR *)data, CmdAccessRegReadCb);

	memset(&access_reg, 0x00, sizeof(access_reg));

	access_reg.u4Address = cpu2le32(address);

	AndesAppendCmdMsg(msg, (char *)&access_reg, sizeof(access_reg));

	ret = AndesSendCmdMsg(pAd, msg);

	DBGPRINT(RT_DEBUG_INFO, ("%s: address = %x, value = %x\n", __FUNCTION__, address, *data));
error:
	DBGPRINT(RT_DEBUG_INFO, ("%s:(ret = %d)\n", __FUNCTION__, ret));
	return ret;
}


INT32 CmdRFRegAccessWrite(RTMP_ADAPTER *pAd, UINT32 RFIdx, UINT32 Offset, UINT32 Value)
{
	struct cmd_msg *msg;
	struct _CMD_RF_REG_ACCESS_T RFRegAccess;
	INT32 ret = 0;

	DBGPRINT(RT_DEBUG_INFO, ("%s: RFIdx = %d, Offset = %x, Value = %x\n", __FUNCTION__,\
										RFIdx, Offset, Value));

	msg = AndesAllocCmdMsg(pAd, sizeof(struct _CMD_RF_REG_ACCESS_T));

	if (!msg)
	{
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	AndesInitCmdMsg(msg, P1_Q0, EXT_CID, CMD_SET, EXT_CMD_RF_REG_ACCESS, FALSE, 0, FALSE, FALSE, 0, NULL, NULL);

	memset(&RFRegAccess, 0x00, sizeof(RFRegAccess));

	RFRegAccess.WiFiStream = cpu2le32(RFIdx);
	RFRegAccess.Address = cpu2le32(Offset);
	RFRegAccess.Data = cpu2le32(Value);

	AndesAppendCmdMsg(msg, (char *)&RFRegAccess, sizeof(RFRegAccess));

	ret = AndesSendCmdMsg(pAd, msg);

error:
	DBGPRINT(RT_DEBUG_INFO, ("%s:(ret = %d)\n", __FUNCTION__, ret));
	return ret;
}


static VOID CmdRFRegAccessReadCb(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	struct _CMD_RF_REG_ACCESS_T *RFRegAccess = (struct _CMD_RF_REG_ACCESS_T *)Data;

	NdisMoveMemory(msg->rsp_payload, &RFRegAccess->Data, Len - 8);
	*msg->rsp_payload = le2cpu32(*msg->rsp_payload);
}


INT32 CmdRFRegAccessRead(RTMP_ADAPTER *pAd, UINT32 RFIdx, UINT32 Offset, UINT32 *Value)
{
	struct cmd_msg *msg;
	struct _CMD_RF_REG_ACCESS_T RFRegAccess;
	INT32 ret = 0;

	DBGPRINT(RT_DEBUG_INFO, ("%s: RFIdx = %d, Offset = %x\n", __FUNCTION__, RFIdx, Offset));

	msg = AndesAllocCmdMsg(pAd, sizeof(struct _CMD_RF_REG_ACCESS_T));

	if (!msg)
	{
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	AndesInitCmdMsg(msg, P1_Q0, EXT_CID, CMD_QUERY, EXT_CMD_RF_REG_ACCESS, TRUE, 0,
							TRUE, TRUE, 12, (CHAR *)Value, CmdRFRegAccessReadCb);

	memset(&RFRegAccess, 0x00, sizeof(RFRegAccess));

	RFRegAccess.WiFiStream = cpu2le32(RFIdx);
	RFRegAccess.Address = cpu2le32(Offset);

	AndesAppendCmdMsg(msg, (char *)&RFRegAccess, sizeof(RFRegAccess));

	ret = AndesSendCmdMsg(pAd, msg);

error:
	DBGPRINT(RT_DEBUG_INFO, ("%s:(ret = %d)\n", __FUNCTION__, ret));
	return ret;
}


VOID CmdIOWrite32(RTMP_ADAPTER *pAd, UINT32 Offset, UINT32 Value)
{
	struct MCU_CTRL *Ctl = &pAd->MCUCtrl;
	RTMP_REG_PAIR RegPair;

	if (Ctl->Stage == FW_RUN_TIME)
	{
		RegPair.Register = Offset;
		RegPair.Value = Value;
		CmdMultipleMacRegAccessWrite(pAd, &RegPair, 1);
	}
#ifdef E2P_WITHOUT_FW_SUPPORT	
        else if (Ctl->Stage == FW_NO_INIT)
        {
		RTMP_IO_WRITE32(pAd, Offset, Value);
        }
#endif /* E2P_WITHOUT_FW_SUPPORT */		
	else
	{
		CmdInitAccessRegWrite(pAd, Offset, Value);
	}
} 


VOID CmdIORead32(struct _RTMP_ADAPTER *pAd, UINT32 Offset, UINT32 *Value)
{

	struct MCU_CTRL *Ctl = &pAd->MCUCtrl;
	RTMP_REG_PAIR RegPair;

	if (Ctl->Stage == FW_RUN_TIME)
	{
		RegPair.Register = Offset;
		CmdMultiPleMacRegAccessRead(pAd, &RegPair, 1);
		*Value = RegPair.Value;
	}
#ifdef E2P_WITHOUT_FW_SUPPORT	
	else if (Ctl->Stage == FW_NO_INIT)
	{
		RTMP_IO_READ32(pAd, Offset, Value);
	}
#endif /* E2P_WITHOUT_FW_SUPPORT */	
	else
	{
		CmdInitAccessRegRead(pAd, Offset, Value); 
	}
}

static VOID EventExtCmdResult(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	struct _EVENT_EXT_CMD_RESULT_T *EventExtCmdResult = (struct _EVENT_EXT_CMD_RESULT_T *)Data;
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)msg->priv;

	DBGPRINT(RT_DEBUG_INFO, ("%s: EventExtCmdResult.ucExTenCID = 0x%x\n",
									__FUNCTION__, EventExtCmdResult->ucExTenCID));

	EventExtCmdResult->u4Status = le2cpu32(EventExtCmdResult->u4Status);

	DBGPRINT(RT_DEBUG_INFO, ("%s: EventExtCmdResult.u4Status = 0x%x\n",
									__FUNCTION__, EventExtCmdResult->u4Status));

	RTMP_OS_TXRXHOOK_CALL(WLAN_CALIB_TEST_RSP,NULL,EventExtCmdResult->u4Status,pAd);
}


VOID EventExtCmdResultHandler(RTMP_ADAPTER *pAd, char *Data, UINT16 Len)
{
	struct _EVENT_EXT_CMD_RESULT_T *EventExtCmdResult = (struct _EVENT_EXT_CMD_RESULT_T *)Data;
	//RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)msg->priv;

	DBGPRINT(RT_DEBUG_INFO, ("%s: EventExtCmdResult.ucExTenCID = 0x%x\n",
									__FUNCTION__, EventExtCmdResult->ucExTenCID));

	EventExtCmdResult->u4Status = le2cpu32(EventExtCmdResult->u4Status);

	DBGPRINT(RT_DEBUG_INFO, ("%s: EventExtCmdResult.u4Status = 0x%x\n",
									__FUNCTION__, EventExtCmdResult->u4Status));

	RTMP_OS_TXRXHOOK_CALL(WLAN_CALIB_TEST_RSP,NULL,EventExtCmdResult->u4Status,pAd);
}


INT32 CmdIcapOverLap(RTMP_ADAPTER *pAd, UINT32 IcapLen)
{
    struct cmd_msg *msg;
    struct _CMD_TEST_CTRL_T TestCtrl;
    INT32 ret = 0;

    DBGPRINT(RT_DEBUG_INFO, ("%s: IcapLen = %d\n", __FUNCTION__, IcapLen));

    msg = AndesAllocCmdMsg(pAd, sizeof(TestCtrl));

    if (!msg) {
        ret = NDIS_STATUS_RESOURCES;
        goto error;
    }

	AndesInitCmdMsg(msg, P1_Q0, EXT_CID, CMD_SET, EXT_CMD_RF_TEST, TRUE, 0,
							TRUE, TRUE, 8, NULL, EventExtCmdResult);

    memset(&TestCtrl, 0x00, sizeof(TestCtrl));

    TestCtrl.ucAction = 0;

    TestCtrl.ucIcapLen = IcapLen;
    
	TestCtrl.u.u4OpMode =cpu2le32( OPERATION_ICAP_OVERLAP);

    AndesAppendCmdMsg(msg, (char *)&TestCtrl, sizeof(TestCtrl));

    ret = AndesSendCmdMsg(pAd, msg);

error:
	DBGPRINT(RT_DEBUG_INFO, ("%s:(ret = %d)\n", __FUNCTION__, ret));
	return ret;
}


INT32 CmdRfTest(RTMP_ADAPTER *pAd, UINT8 Action, UINT8 Mode, UINT8 CalItem)
{
    struct cmd_msg *msg;
    struct _CMD_TEST_CTRL_T TestCtrl;
    INT32 ret = 0;

    DBGPRINT(RT_DEBUG_INFO, ("%s: Action = %d Mode = %d CalItem = %d\n", __FUNCTION__, Action, Mode, CalItem));

    msg = AndesAllocCmdMsg(pAd, sizeof(TestCtrl));

    if (!msg) {
        ret = NDIS_STATUS_RESOURCES;
        goto error;
    }

	AndesInitCmdMsg(msg, P1_Q0, EXT_CID, CMD_SET, EXT_CMD_RF_TEST, TRUE, 0,
							TRUE, TRUE, 8, NULL, EventExtCmdResult);

    memset(&TestCtrl, 0x00, sizeof(TestCtrl));

    TestCtrl.ucAction = Action;
    TestCtrl.u.u4OpMode = cpu2le32((UINT32)Mode);

    if (Action == ACTION_IN_RFTEST) {
        /* set Cal Items */
        TestCtrl.u.rRfATInfo.u4FuncIndex = cpu2le32(1);
        TestCtrl.u.rRfATInfo.u4FuncData = cpu2le32((UINT32)CalItem);
    }

    AndesAppendCmdMsg(msg, (char *)&TestCtrl, sizeof(TestCtrl));

    ret = AndesSendCmdMsg(pAd, msg);

error:
	DBGPRINT(RT_DEBUG_INFO, ("%s:(ret = %d)\n", __FUNCTION__, ret));
	return ret;

}

INT32 CmdRadioOnOffCtrl(RTMP_ADAPTER *pAd, UINT8 On)
{
	struct cmd_msg *msg;
	struct _EXT_CMD_RADIO_ON_OFF_CTRL_T RadioOnOffCtrl;
	INT32 ret = 0;

	DBGPRINT(RT_DEBUG_INFO, ("%s: On = %d\n", __FUNCTION__, On));

	msg = AndesAllocCmdMsg(pAd, sizeof(RadioOnOffCtrl));

	if (!msg)
	{
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	AndesInitCmdMsg(msg, P1_Q0, EXT_CID, CMD_SET, EXT_CMD_RADIO_ON_OFF_CTRL, TRUE, 0,
							TRUE, TRUE, 8, NULL, EventExtCmdResult);

	memset(&RadioOnOffCtrl, 0x00, sizeof(RadioOnOffCtrl));

	if (On == WIFI_RADIO_ON)
		RadioOnOffCtrl.ucWiFiRadioCtrl = WIFI_RADIO_ON;
	else if (On == WIFI_RADIO_OFF)
		RadioOnOffCtrl.ucWiFiRadioCtrl = WIFI_RADIO_OFF;
	else
		DBGPRINT(RT_DEBUG_ERROR, ("%s: Unknown state On = %d\n", __FUNCTION__, On));

	AndesAppendCmdMsg(msg, (char *)&RadioOnOffCtrl, sizeof(RadioOnOffCtrl));

	ret = AndesSendCmdMsg(pAd, msg);

error:
	DBGPRINT(RT_DEBUG_INFO, ("%s:(ret = %d)\n", __FUNCTION__, ret));
	return ret;
}


INT32 CmdWiFiRxDisable(RTMP_ADAPTER *pAd, UINT RxDisable)
{
	struct cmd_msg *msg;
	struct _EXT_CMD_WIFI_RX_DISABLE_T WiFiRxDisable;
	INT32 ret = 0;

	DBGPRINT(RT_DEBUG_INFO, ("%s: WiFiRxDisable = %d\n", __FUNCTION__, RxDisable));

	if (RxDisable != WIFI_RX_DISABLE)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Error: %s: RxDisable = %d\n", __FUNCTION__, RxDisable));
		return ret;
	}

	msg = AndesAllocCmdMsg(pAd, sizeof(WiFiRxDisable));

	if (!msg)
	{
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	AndesInitCmdMsg(msg, P1_Q0, EXT_CID, CMD_SET, EXT_CMD_WIFI_RX_DISABLE, TRUE, 0,
							TRUE, TRUE, 8, NULL, EventExtCmdResult);

	memset(&WiFiRxDisable, 0x00, sizeof(WiFiRxDisable));

	WiFiRxDisable.ucWiFiRxDisableCtrl = RxDisable;

	AndesAppendCmdMsg(msg, (char *)&WiFiRxDisable, sizeof(WiFiRxDisable));

	ret = AndesSendCmdMsg(pAd, msg);

error:
	DBGPRINT(RT_DEBUG_INFO, ("%s:(ret = %d)\n", __FUNCTION__, ret));
	return ret;
}


//NoA
INT32 CmdP2pNoaOffloadCtrl(RTMP_ADAPTER *ad, UINT8 enable)
{
      struct cmd_msg *msg;
      struct _EXT_CMD_NOA_CTRL_T extCmdNoaCtrl;		
      int ret = 0;

      msg = AndesAllocCmdMsg(ad, sizeof(struct _EXT_CMD_NOA_CTRL_T));

      if (!msg)
      {
         ret = NDIS_STATUS_RESOURCES;
         goto error;
      }

      AndesInitCmdMsg(msg, P1_Q0, EXT_CID, CMD_SET, EXT_CMD_ID_NOA_OFFLOAD_CTRL, 
			TRUE, 0, TRUE, TRUE, 
			sizeof(struct _EXT_CMD_NOA_CTRL_T), NULL, EventExtCmdResult);

      NdisZeroMemory(&extCmdNoaCtrl, sizeof(extCmdNoaCtrl));

      extCmdNoaCtrl.ucMode1 = enable;
      //extCmdNoaCtrl.ucMode0 = enable;

      AndesAppendCmdMsg(msg, (char *)&extCmdNoaCtrl, sizeof(extCmdNoaCtrl));
      ret = AndesSendCmdMsg(ad, msg);

error:
      DBGPRINT(RT_DEBUG_INFO, ("%s:(ret = %d)\n", __FUNCTION__, ret));
        return ret;
}

#ifdef SINGLE_SKU_V2
enum {
	SKU_CCK_1_2=0,
	SKU_CCK_55_11,
	SKU_OFDM_6_9,
	SKU_OFDM_12_18,
	SKU_OFDM_24_36,
	SKU_OFDM_48,
	SKU_OFDM_54,
	SKU_HT20_0_8,
	SKU_HT20_32,
	SKU_HT20_1_2_9_10,
	SKU_HT20_3_4_11_12,
	SKU_HT20_5_13,
	SKU_HT20_6_14,
	SKU_HT20_7_15,	
	SKU_HT40_0_8,
	SKU_HT40_32,
	SKU_HT40_1_2_9_10,
	SKU_HT40_3_4_11_12,
	SKU_HT40_5_13,
	SKU_HT40_6_14,
	SKU_HT40_7_15,
};
static VOID mt_FillSkuParameter(RTMP_ADAPTER *pAd,UINT8 channel,UINT8 *txPowerSku)
{
	CH_POWER *ch, *ch_temp;
	UCHAR start_ch;
	//UCHAR base_pwr = pAd->DefaultTargetPwr;
	UINT8 j;
	
	DlListForEachSafe(ch, ch_temp, &pAd->SingleSkuPwrList, CH_POWER, List)
	{
		start_ch = ch->StartChannel;
		if ( channel >= start_ch )
		{
			for ( j = 0; j < ch->num; j++ )
			{
				if ( channel == ch->Channel[j] )
				{

					txPowerSku[SKU_CCK_1_2] 	= ch->PwrCCK[0] ? ch->PwrCCK[0] : 0xff;
					txPowerSku[SKU_CCK_55_11] 	= ch->PwrCCK[2] ? ch->PwrCCK[2] : 0xff;
					txPowerSku[SKU_OFDM_6_9] 	= ch->PwrOFDM[0] ? ch->PwrOFDM[0] : 0xff;
					txPowerSku[SKU_OFDM_12_18]= ch->PwrOFDM[2] ? ch->PwrOFDM[2] : 0xff;
					txPowerSku[SKU_OFDM_24_36]= ch->PwrOFDM[4] ? ch->PwrOFDM[4] : 0xff;
					txPowerSku[SKU_OFDM_48] 	= ch->PwrOFDM[6] ? ch->PwrOFDM[6] : 0xff;
					txPowerSku[SKU_OFDM_54] 	= ch->PwrOFDM[7] ? ch->PwrOFDM[7] : 0xff;
					txPowerSku[SKU_HT20_0_8] 	= ch->PwrHT20[0] ? ch->PwrHT20[0] : 0xff;
					txPowerSku[SKU_HT20_32] 	= 0xff;
					txPowerSku[SKU_HT20_1_2_9_10] = ch->PwrHT20[1] ? ch->PwrHT20[1] : 0xff;
					txPowerSku[SKU_HT20_3_4_11_12] = ch->PwrHT20[3] ? ch->PwrHT20[3] : 0xff;
					txPowerSku[SKU_HT20_5_13] 	= ch->PwrHT20[5] ? ch->PwrHT20[5]  : 0xff;
					txPowerSku[SKU_HT20_6_14] 	= ch->PwrHT20[6] ? ch->PwrHT20[6]  : 0xff;
					txPowerSku[SKU_HT20_7_15] 	= ch->PwrHT20[7] ? ch->PwrHT20[7]  : 0xff;
					txPowerSku[SKU_HT40_0_8] 	= ch->PwrHT40[0] ? ch->PwrHT40[0]  : 0xff;
					txPowerSku[SKU_HT40_32] 	= 0xff;
					txPowerSku[SKU_HT40_1_2_9_10] = ch->PwrHT40[1] ?  ch->PwrHT40[1] : 0xff;
					txPowerSku[SKU_HT40_3_4_11_12] = ch->PwrHT40[3] ?  ch->PwrHT40[3] : 0xff;
					txPowerSku[SKU_HT40_5_13] 	= ch->PwrHT40[5] ?  ch->PwrHT40[5] : 0xff;
					txPowerSku[SKU_HT40_6_14] 	= ch->PwrHT40[6] ?  ch->PwrHT40[6] : 0xff;
					txPowerSku[SKU_HT40_7_15] 	= ch->PwrHT40[7] ?  ch->PwrHT40[7] : 0xff;
					break;
				}
			}
		}
	}
}
#endif

INT32 CmdChannelSwitch(RTMP_ADAPTER *pAd, UINT8 control_chl, UINT8 central_chl,
							UINT8 BW, UINT8 TXStream, UINT8 RXStream)
{
	struct cmd_msg *msg;
	struct _EXT_CMD_CHAN_SWITCH_T CmdChanSwitch;
	INT32 ret = 0,i=0;
#ifdef RTMP_SDIO_SUPPORT 
	return 0;
#endif /*leonardo for SDIO FPGA pass*/

	if (central_chl == 0)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s: central channel = 0 is invalid\n", __FUNCTION__));
		return -1;
	}

#ifdef WH_EZ_SETUP
		if(IS_ADPTR_EZ_SETUP_ENABLED(pAd)){
			EZ_DEBUG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("control_chl = %d\n", control_chl));
		}
		else
#endif
		{
			DBGPRINT(RT_DEBUG_INFO, ("%s: control_chl = %d, central_chl = %d, BW = %d,	\
									TXStream = %d, RXStream = %d\n", __FUNCTION__,	\
										control_chl, central_chl, BW, TXStream, RXStream));
		}
	msg = AndesAllocCmdMsg(pAd, sizeof(CmdChanSwitch));

	if (!msg)
	{
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	
	AndesInitCmdMsg(msg, P1_Q0, EXT_CID, CMD_SET, EXT_CMD_CHANNEL_SWITCH, TRUE, 0,
							TRUE, TRUE, 8, NULL, EventExtCmdResult);

	memset(&CmdChanSwitch, 0x00, sizeof(CmdChanSwitch));

	CmdChanSwitch.ucCtrlCh = control_chl;
	CmdChanSwitch.ucCentralCh = central_chl;
	CmdChanSwitch.ucBW = BW;
	CmdChanSwitch.ucTxStreamNum = TXStream;
	CmdChanSwitch.ucRxStreamNum = RXStream;

	for(i=0;i<SKU_SIZE;i++)
	{
		CmdChanSwitch.aucTxPowerSKU[i]=0xff;
	}
 
#ifdef SINGLE_SKU_V2
	if  (pAd->SKUEn)
	mt_FillSkuParameter(pAd,central_chl,CmdChanSwitch.aucTxPowerSKU);
#endif	
	AndesAppendCmdMsg(msg, (char *)&CmdChanSwitch, sizeof(CmdChanSwitch));

	ret = AndesSendCmdMsg(pAd, msg);

error:
	DBGPRINT(RT_DEBUG_INFO, ("%s:(ret = %d)\n", __FUNCTION__, ret));
	return ret;
}


INT32 CmdFlushFrameByWlanIdx(RTMP_ADAPTER *pAd, UINT8 WlanIdx)
{
	struct cmd_msg *msg;
	EXT_CMD_FLUSH_FRAME_BY_WCID_INFO_T CmdFlushFrameByWcid;
	INT32 ret=0;

	msg = AndesAllocCmdMsg(pAd, sizeof(CmdFlushFrameByWcid));

	if (!msg)
	{
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

    	AndesInitCmdMsg(msg, P1_Q0, EXT_CID, CMD_QUERY, EXT_CMD_FLUSH_FRAME_WCID,
			FALSE, 0, FALSE, FALSE, 0, NULL, NULL);

	memset(&CmdFlushFrameByWcid, 0x00, sizeof(CmdFlushFrameByWcid));
	CmdFlushFrameByWcid.ucWlanIdx = WlanIdx;
	
    	AndesAppendCmdMsg(msg, (char *)&CmdFlushFrameByWcid, sizeof(CmdFlushFrameByWcid));

    	ret = AndesSendCmdMsg(pAd, msg);

error:
	DBGPRINT(RT_DEBUG_OFF, ("%s:(ret = %d)\n", __FUNCTION__, ret));
	return ret;
}

static VOID EventExtNicCapability(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	struct _EXT_EVENT_NIC_CAPABILITY_T *ExtEventNicCapability = (EXT_EVENT_NIC_CAPABILITY *)Data;
	UINT32 Loop;

	DBGPRINT(RT_DEBUG_OFF, ("The data code of firmware:"));

	for (Loop = 0; Loop < 16; Loop++)
	{
		DBGPRINT(RT_DEBUG_OFF, ("%c", ExtEventNicCapability->aucDateCode[Loop]));
	}

	DBGPRINT(RT_DEBUG_OFF, ("\nThe version code of firmware:"));

	for (Loop = 0; Loop < 12; Loop++)
	{
		DBGPRINT(RT_DEBUG_OFF, ("%c", ExtEventNicCapability->aucVersionCode[Loop]));
	}
}


INT32 CmdNicCapability(RTMP_ADAPTER *pAd)
{
	struct cmd_msg *msg;
	INT32 ret = 0;

	msg = AndesAllocCmdMsg(pAd, 0);

	if (!msg)
	{
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	AndesInitCmdMsg(msg, P1_Q0, EXT_CID, CMD_QUERY, EXT_CMD_NIC_CAPABILITY, TRUE, 0,
							TRUE, TRUE, 28, NULL, EventExtNicCapability);

	ret = AndesSendCmdMsg(pAd, msg);

error:
	DBGPRINT(RT_DEBUG_INFO, ("%s:(ret = %d)\n", __FUNCTION__, ret));
	return ret;
}


INT32 CmdFwLog2Host(RTMP_ADAPTER *pAd, UINT8 FWLog2HostCtrl)
{

	struct cmd_msg *msg;
	INT32 Ret = 0;
	EXT_CMD_FW_LOG_2_HOST_CTRL_T CmdFwLog2HostCtrl;

	msg = AndesAllocCmdMsg(pAd, sizeof(CmdFwLog2HostCtrl));

	if (!msg)
	{
		Ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	AndesInitCmdMsg(msg, P1_Q0, EXT_CID, CMD_QUERY, EXT_CMD_FW_LOG_2_HOST, 
						FALSE, 0, FALSE, FALSE, 0, NULL, NULL);
	
	memset(&CmdFwLog2HostCtrl, 0x00, sizeof(CmdFwLog2HostCtrl));

	CmdFwLog2HostCtrl.ucFwLog2HostCtrl = FWLog2HostCtrl;

	AndesAppendCmdMsg(msg, (char *)&CmdFwLog2HostCtrl, 
									sizeof(CmdFwLog2HostCtrl));

	Ret = AndesSendCmdMsg(pAd, msg);

error:
	DBGPRINT(RT_DEBUG_INFO, ("%s:(ret = %d)\n", __FUNCTION__, Ret));
	return Ret;
}

#ifdef BCN_OFFLOAD_SUPPORT
static VOID ExtEventBcnUpdateHandler(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
    if ((RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS)) ||
        (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BULKOUT_RESET)) ||
        (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)) ||
        (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)))
    {
        return;
    }

    RTMP_UPDATE_BCN(pAd);
}
#endif

#ifdef THERMAL_PROTECT_SUPPORT
static VOID EventThermalProtect(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	EXT_EVENT_THERMAL_PROTECT_T *EvtThermalProtect;
	UINT8 HLType;
    UINT32 ret;

	EvtThermalProtect = (EXT_EVENT_THERMAL_PROTECT_T *)Data;

	HLType = EvtThermalProtect->ucHLType;
	pAd->last_thermal_pro_temp = EvtThermalProtect->cCurrentTemp;

	DBGPRINT(RT_DEBUG_OFF, ("%s: HLType = %d, CurrentTemp = %d\n", __FUNCTION__, HLType, pAd->last_thermal_pro_temp));

    RTMP_SEM_EVENT_WAIT(&pAd->AutoRateLock, ret);
	if (HLType == HIGH_TEMP_THD)
	{
		pAd->force_one_tx_stream = TRUE;
		DBGPRINT(RT_DEBUG_OFF, ("Switch TX to 1 stram\n"));
	}
	else
	{
		DBGPRINT(RT_DEBUG_OFF, ("Switch TX to 2 stram\n"));
		pAd->force_one_tx_stream = FALSE;
	}

    pAd->fgThermalProtectToggle = TRUE;

    RTMP_SEM_EVENT_UP(&pAd->AutoRateLock);
}


INT32 CmdThermalProtect(RTMP_ADAPTER *ad, UINT8 HighEn, CHAR HighTempTh, UINT8 LowEn, CHAR LowTempTh)
{
	struct cmd_msg *msg;
	INT32 ret = 0;
	EXT_CMD_THERMAL_PROTECT_T ThermalProtect;

	msg = AndesAllocCmdMsg(ad, sizeof(EXT_CMD_THERMAL_PROTECT_T));

	if (!msg)
	{
         ret = NDIS_STATUS_RESOURCES;
         goto error;
	}

	AndesInitCmdMsg(msg, P1_Q0, EXT_CID, CMD_SET, EXT_CMD_THERMAL_PROTECT, TRUE, 0, TRUE, TRUE, 8, NULL, EventExtCmdResult);

	NdisZeroMemory(&ThermalProtect, sizeof(ThermalProtect));

	ThermalProtect.ucHighEnable = HighEn;
	ThermalProtect.cHighTempThreshold = HighTempTh;
	ThermalProtect.ucLowEnable = LowEn;
	ThermalProtect.cLowTempThreshold = LowTempTh;

	ad->thermal_pro_high_criteria = HighTempTh;
	ad->thermal_pro_high_en = HighEn;
	ad->thermal_pro_low_criteria = LowTempTh;
	ad->thermal_pro_low_en = LowEn;

	AndesAppendCmdMsg(msg, (char *)&ThermalProtect, sizeof(ThermalProtect));
	ret = AndesSendCmdMsg(ad, msg);

error:
      DBGPRINT(RT_DEBUG_INFO, ("%s:(ret = %d)\n", __FUNCTION__, ret));
   	return ret;
}
#endif

#ifdef BCN_OFFLOAD_SUPPORT
/*****************************************
    ExT_CID = 0x33
*****************************************/
INT32 CmdBcnUpdateSet(RTMP_ADAPTER *pAd, CMD_BCN_UPDATE_T bcn_update)
{
    struct cmd_msg *msg;
    INT32 ret=0,size=0;

    size = sizeof(CMD_BCN_UPDATE_T);

    msg = AndesAllocCmdMsg(pAd, size);

    if (!msg)
    {
        ret = NDIS_STATUS_RESOURCES;
        goto error;
    }

    AndesInitCmdMsg(msg, P1_Q0, EXT_CID, CMD_SET, EXT_CMD_ID_BCN_UPDATE, FALSE, 0, FALSE, FALSE, 0, NULL, NULL);

    AndesAppendCmdMsg(msg, (char *)&bcn_update, size);

    ret = AndesSendCmdMsg(pAd, msg);

error:
    DBGPRINT(RT_DEBUG_OFF, ("%s:(ret = %d)\n", __FUNCTION__, ret));
    return ret;
}
#endif

#define TOP_MISC2 0x1134
#define SW_SYN0 0x81021250
static NDIS_STATUS AndesMTLoadFwMethod1(RTMP_ADAPTER *ad)
{
	UINT32 value, loop, dl_len;
	UINT32 ret = 0;
	RTMP_CHIP_CAP *cap = &ad->chipCap;
#ifdef RTMP_PCI_SUPPORT
	POS_COOKIE obj = (POS_COOKIE)ad->OS_Cookie;
#endif /* RTMP_PCI_SUPPORT */
	struct MCU_CTRL *Ctl = &ad->MCUCtrl;
#ifdef RTMP_PCI_SUPPORT
	UINT32 RemapBase, RemapOffset;
	UINT32 RestoreValue;
#endif

	if (cap->load_code_method == BIN_FILE_METHOD) {
		DBGPRINT(RT_DEBUG_OFF, ("load fw image from /lib/firmware/%s\n", cap->fw_bin_file_name));
#ifdef RTMP_PCI_SUPPORT
		OS_LOAD_CODE_FROM_BIN(&cap->FWImageName, cap->fw_bin_file_name, obj->pci_dev, &cap->fw_len);
#endif
	} else {
		cap->FWImageName = cap->fw_header_image;
	}

	if (!cap->FWImageName) {
		if (cap->load_code_method == BIN_FILE_METHOD) {
			DBGPRINT(RT_DEBUG_ERROR, ("%s:Please assign a fw image(/lib/firmware/%s), load_method(%d)\n", __FUNCTION__, cap->fw_bin_file_name, cap->load_code_method));
		} else {
			DBGPRINT(RT_DEBUG_ERROR, ("%s:Please assign a fw image, load_method(%d)\n",
				__FUNCTION__, cap->load_code_method));
		}
		ret = NDIS_STATUS_FAILURE;
		goto done;
	}

	Ctl->Stage = FW_DOWNLOAD;

	DBGPRINT(RT_DEBUG_OFF, ("FW Version:"));
	for (loop = 0; loop < 10; loop++)
		DBGPRINT(RT_DEBUG_OFF, ("%c", *(cap->FWImageName + cap->fw_len - 29 + loop)));
	DBGPRINT(RT_DEBUG_OFF, ("\n"));

	DBGPRINT(RT_DEBUG_OFF, ("FW Build Date:"));
	for (loop = 0; loop < 15; loop++)
		DBGPRINT(RT_DEBUG_OFF, ("%c", *(cap->FWImageName + cap->fw_len - 19 + loop)));
	DBGPRINT(RT_DEBUG_OFF, ("\n"));

	dl_len = (*(cap->FWImageName + cap->fw_len - 1) << 24) |
				(*(cap->FWImageName + cap->fw_len - 2) << 16) |
				(*(cap->FWImageName + cap->fw_len -3) << 8) |
				*(cap->FWImageName + cap->fw_len - 4);

	dl_len += 4; /* including crc value */

	DBGPRINT(RT_DEBUG_INFO, ("\ndownload len = %d\n", dl_len));

#ifdef RTMP_PCI_SUPPORT
	if (IS_MT7603(ad))
	{
		RTMP_IO_READ32(ad, MCU_PCIE_REMAP_1, &RestoreValue);
		RemapBase = GET_REMAP_1_BASE(0x50012498) << 18;
		RemapOffset = GET_REMAP_1_OFFSET(0x50012498);
		RTMP_IO_WRITE32(ad, MCU_PCIE_REMAP_1, RemapBase);
		RTMP_IO_WRITE32(ad, 0x40000 + RemapOffset, 0x5);
		RTMP_IO_WRITE32(ad, 0x40000 + RemapOffset, 0x5);
		RtmpusecDelay(1);
		RTMP_IO_WRITE32(ad, MCU_PCIE_REMAP_1, RestoreValue);
	}
#endif

#if defined(RTMP_PCI_SUPPORT) || defined(RTMP_USB_SUPPORT)
	/* switch to bypass mode */
	RTMP_IO_READ32(ad, SCH_REG4, &value);
	value &= ~SCH_REG4_BYPASS_MODE_MASK;
	value |= SCH_REG4_BYPASS_MODE(1);

#ifdef RTMP_PCI_SUPPORT
	value &= ~SCH_REG4_FORCE_QID_MASK;
	value |= SCH_REG4_FORCE_QID(5);
#endif


	RTMP_IO_WRITE32(ad, SCH_REG4, value);
#endif

	/* optional CMD procedure */
	/* CMD restart download flow request */
	RTMP_IO_READ32(ad, TOP_MISC2, &value);
	DBGPRINT(RT_DEBUG_INFO, ("TOP_MSIC = %x\n", value));

	/* check ram code if running, if it is, need to do optional cmd procedure */
	if ((value & 0x02) == 0x02) {

#ifdef LOAD_FW_ONE_TIME
		ret = NDIS_STATUS_SUCCESS;
		Ctl->Stage = FW_RUN_TIME;
		ad->FWLoad = 1;
		goto done;
#else /* LOAD_FW_ONE_TIME */
		ret = CmdRestartDLReq(ad);

		if (ret)
			goto done;
#endif /* !LOAD_FW_ONE_TIME */
	}

	/* check rom code if ready */
	loop = 0;

	do
	{
		RTMP_IO_READ32(ad, TOP_MISC2, &value);
		if((value & 0x01) == 0x01 && !(value & 0x02))
			break;
		RtmpOsMsDelay(1);
		loop++;
	} while (loop <= 500);

	if (loop > 500) {
		DBGPRINT(RT_DEBUG_ERROR, ("%s: rom code is not ready(TOP_MISC2 = %d)\n", __FUNCTION__, value));
		goto done;
	}

	/* standard CMD procedure */
	/* 1. Config PDA phase */
	ret = CmdAddressLenReq(ad, FW_CODE_START_ADDRESS1, dl_len, TARGET_ADDR_LEN_NEED_RSP);

	if (ret)
		goto done;

	/* 2. Loading firmware image phase */
	ret = CmdFwScatters(ad, cap->FWImageName, dl_len);

	if (ret)
		goto done;

	/* 3. Firmware start negotiation phase */
	ret = CmdFwStartReq(ad, 1, FW_CODE_START_ADDRESS1);

	/* 4. check Firmware running */
	for (loop = 0; loop < 500; loop++)
	{
		RTMP_IO_READ32(ad, TOP_MISC2, &value);
		DBGPRINT(RT_DEBUG_INFO, ("TOP_MSIC = %x\n", value));
		if ((value & 0x02) == 0x02)
			break;

		RtmpOsMsDelay(1);
	}

	if (loop == 500)
	{
		ret = NDIS_STATUS_FAILURE;
		DBGPRINT(RT_DEBUG_OFF, ("firmware loading failure\n"));
		Ctl->Stage = FW_NO_INIT;
	}
	else
	{
		Ctl->Stage = FW_RUN_TIME;
	}

done:

#if defined(RTMP_PCI_SUPPORT) || defined(RTMP_USB_SUPPORT)
	/* Switch to normal mode */
	RTMP_IO_READ32(ad, SCH_REG4, &value);
	value &= ~SCH_REG4_BYPASS_MODE_MASK;
	value |= SCH_REG4_BYPASS_MODE(0);
	value &= ~SCH_REG4_FORCE_QID_MASK;
	value |= SCH_REG4_FORCE_QID(0);
	RTMP_IO_WRITE32(ad, SCH_REG4, value);

	RTMP_IO_READ32(ad, SCH_REG4, &value);
	value |= (1 << 8);
	RTMP_IO_WRITE32(ad, SCH_REG4, value);
	RTMP_IO_READ32(ad, SCH_REG4, &value);
	value &= ~(1 << 8);
	RTMP_IO_WRITE32(ad, SCH_REG4, value);
#endif

	return ret;
}


static NDIS_STATUS AndesMTLoadFwMethod2(RTMP_ADAPTER *ad)
{
	UINT32 value, loop, ilm_dl_len, dlm_dl_len;
	UINT8 ilm_feature_set, dlm_feature_set;
	UINT8 ilm_chip_info, dlm_chip_info;
	UINT32 ilm_target_addr, dlm_target_addr;
	UINT32 ret = 0;
	RTMP_CHIP_CAP *cap = &ad->chipCap;
#ifdef RTMP_PCI_SUPPORT
	POS_COOKIE obj = (POS_COOKIE)ad->OS_Cookie;
#endif /* RTMP_PCI_SUPPORT */
	struct MCU_CTRL *Ctl = &ad->MCUCtrl;

	if (cap->load_code_method == BIN_FILE_METHOD) {
		DBGPRINT(RT_DEBUG_OFF, ("load fw image from /lib/firmware/%s\n", cap->fw_bin_file_name));
#ifdef RTMP_PCI_SUPPORT
		OS_LOAD_CODE_FROM_BIN(&cap->FWImageName, cap->fw_bin_file_name, obj->pci_dev, &cap->fw_len);
#endif
	} else {
		cap->FWImageName = cap->fw_header_image;
	}

	if (!cap->FWImageName) {
		if (cap->load_code_method == BIN_FILE_METHOD) {
			DBGPRINT(RT_DEBUG_ERROR, ("%s:Please assign a fw image(/lib/firmware/%s), load_method(%d)\n", __FUNCTION__, cap->fw_bin_file_name, cap->load_code_method));
		} else {
			DBGPRINT(RT_DEBUG_ERROR, ("%s:Please assign a fw image, load_method(%d)\n",
				__FUNCTION__, cap->load_code_method));
		}
		ret = NDIS_STATUS_FAILURE;
		goto done;
	}

	Ctl->Stage = FW_DOWNLOAD;

	ilm_target_addr = (*(cap->FWImageName + cap->fw_len - (33 + 36)) << 24) |
				(*(cap->FWImageName + cap->fw_len - (34 + 36)) << 16) |
				(*(cap->FWImageName + cap->fw_len -(35 + 36)) << 8) |
				*(cap->FWImageName + cap->fw_len - (36 + 36));

	DBGPRINT(RT_DEBUG_TRACE, ("ILM target address = %x\n", ilm_target_addr));

	ilm_chip_info = *(cap->FWImageName + cap->fw_len - (32 + 36));
	DBGPRINT(RT_DEBUG_TRACE, ("\nILM chip information = %x\n", ilm_chip_info));

	ilm_feature_set = *(cap->FWImageName + cap->fw_len - (31 + 36));
	DBGPRINT(RT_DEBUG_TRACE, ("\nILM feature set = %x\n", ilm_feature_set));

	DBGPRINT(RT_DEBUG_TRACE, ("\nILM Build Date:"));

	for (loop = 0; loop < 8; loop++)
		DBGPRINT(RT_DEBUG_OFF, ("%c", *(cap->FWImageName + cap->fw_len - (20 + 36) + loop)));

	ilm_dl_len = (*(cap->FWImageName + cap->fw_len - (1 + 36)) << 24) |
				(*(cap->FWImageName + cap->fw_len - (2 + 36)) << 16) |
				(*(cap->FWImageName + cap->fw_len -(3 + 36)) << 8) |
				*(cap->FWImageName + cap->fw_len - (4 + 36));

	ilm_dl_len += 4; /* including crc value */

	DBGPRINT(RT_DEBUG_TRACE, ("\nILM download len = %d\n", ilm_dl_len));

	dlm_target_addr = (*(cap->FWImageName + cap->fw_len - 33) << 24) |
				(*(cap->FWImageName + cap->fw_len - 34) << 16) |
				(*(cap->FWImageName + cap->fw_len - 35) << 8) |
				*(cap->FWImageName + cap->fw_len - 36);

	DBGPRINT(RT_DEBUG_TRACE, ("DLM target address = %x\n", dlm_target_addr));

	dlm_chip_info = *(cap->FWImageName + cap->fw_len - 32);
	DBGPRINT(RT_DEBUG_TRACE, ("\nDLM chip information = %x\n", dlm_chip_info));

	dlm_feature_set = *(cap->FWImageName + cap->fw_len - 31);
	DBGPRINT(RT_DEBUG_TRACE, ("\nDLM feature set = %x\n", dlm_feature_set));

	DBGPRINT(RT_DEBUG_TRACE, ("DLM Build Date:"));

	for (loop = 0; loop < 8; loop++)
		DBGPRINT(RT_DEBUG_OFF, ("%c", *(cap->FWImageName + cap->fw_len - 20 + loop)));

	dlm_dl_len = (*(cap->FWImageName + cap->fw_len - 1) << 24) |
				(*(cap->FWImageName + cap->fw_len - 2) << 16) |
				(*(cap->FWImageName + cap->fw_len - 3) << 8) |
				*(cap->FWImageName + cap->fw_len - 4);

	dlm_dl_len += 4; /* including crc value */

	DBGPRINT(RT_DEBUG_TRACE, ("\nDLM download len = %d\n", dlm_dl_len));

#if defined(RTMP_PCI_SUPPORT) || defined(RTMP_USB_SUPPORT)
	/* switch to bypass mode */
	RTMP_IO_READ32(ad, SCH_REG4, &value);
	value &= ~SCH_REG4_BYPASS_MODE_MASK;
	value |= SCH_REG4_BYPASS_MODE(1);

#ifdef RTMP_PCI_SUPPORT
	value &= ~SCH_REG4_FORCE_QID_MASK;
	value |= SCH_REG4_FORCE_QID(5);
#endif


	RTMP_IO_WRITE32(ad, SCH_REG4, value);
#endif

	/* optional CMD procedure */
	/* CMD restart download flow request */
#if defined(RTMP_PCI_SUPPORT) || defined(RTMP_USB_SUPPORT)
	RTMP_IO_READ32(ad, SW_SYN0, &value);
	DBGPRINT(RT_DEBUG_TRACE, ("SW_SYN0 = %x\n", value));

	/* check ram code if running, if it is, need to do optional cmd procedure */
	if ((value & 0x03) == 0x03) {
		ret = CmdRestartDLReq(ad);

		if (ret)
			goto done;
	}

	/* check rom code if ready */
	loop = 0;

	do
	{
		RTMP_IO_READ32(ad, SW_SYN0, &value);
		if ((value & 0x01) == 0x01)
			break;
		RtmpOsMsDelay(1);
		loop++;
	} while (loop <= 500);

	if (loop > 500) {
		DBGPRINT(RT_DEBUG_ERROR, ("%s: rom code is not ready(SW_SYN0 = %d)\n", __FUNCTION__, value));
		goto done;
	}
#endif

	/*  ILM */
	/* standard CMD procedure */
	/* 1. Config PDA phase */
	ret = CmdAddressLenReq(ad, ilm_target_addr, ilm_dl_len,
				(ilm_feature_set & FW_FEATURE_SET_ENCRY) |
				(FW_FEATURE_SET_KEY(GET_FEATURE_SET_KEY(ilm_feature_set))) |
				((ilm_feature_set & FW_FEATURE_SET_ENCRY) ? FW_FEATURE_RESET_IV: 0) |
				TARGET_ADDR_LEN_NEED_RSP);

	if (ret)
		goto done;


	/* 2. Loading ilm firmware image phase */
	ret = CmdFwScatters(ad, cap->FWImageName, ilm_dl_len);

	if (ret)
		goto done;

	/*  DLM */
	/* standard CMD procedure */
	/* 1. Config PDA phase */
	ret = CmdAddressLenReq(ad, dlm_target_addr, dlm_dl_len,
				(dlm_feature_set & FW_FEATURE_SET_ENCRY) |
				(FW_FEATURE_SET_KEY(GET_FEATURE_SET_KEY(dlm_feature_set))) |
				((dlm_feature_set & FW_FEATURE_SET_ENCRY) ? FW_FEATURE_RESET_IV: 0) |
				TARGET_ADDR_LEN_NEED_RSP);

	if (ret)
		goto done;

	/* 2. Loading dlm firmware image phase */
	ret = CmdFwScatters(ad, cap->FWImageName + ilm_dl_len, dlm_dl_len);

	if (ret)
		goto done;

	/* 3. Firmware start negotiation phase */
	ret = CmdFwStartReq(ad, 0, 0);


#if defined(RTMP_PCI_SUPPORT) || defined(RTMP_USB_SUPPORT)
	/* 4. check Firmware running */
	for (loop = 0; loop < 500; loop++)
	{
		RTMP_IO_READ32(ad, SW_SYN0, &value);
		DBGPRINT(RT_DEBUG_TRACE, ("SW_SYN0 = %x\n", value));
		if ((value & 0x03) == 0x03)
			break;

		RtmpOsMsDelay(1);
	}

	if (loop == 500)
	{
		ret = NDIS_STATUS_FAILURE;
		DBGPRINT(RT_DEBUG_OFF, ("firmware loading failure\n"));
		Ctl->Stage = FW_NO_INIT;
	}
	else
	{

		Ctl->Stage = FW_RUN_TIME;
	}
#endif
#ifdef RTMP_SDIO_SUPPORT 

	RtmpOsMsDelay(1000);
	Ctl->Stage = FW_RUN_TIME;
	
	printk("%s: &pAd->MCUCtrl=%p\n",__FUNCTION__,Ctl); 
	printk("%s(): findish download fw ya! \n", __FUNCTION__); 
#endif


done:

#if defined(RTMP_PCI_SUPPORT) || defined(RTMP_USB_SUPPORT)
	/* Switch to normal mode */
	RTMP_IO_READ32(ad, SCH_REG4, &value);
	value &= ~SCH_REG4_BYPASS_MODE_MASK;
	value |= SCH_REG4_BYPASS_MODE(0);
	value &= ~SCH_REG4_FORCE_QID_MASK;
	value |= SCH_REG4_FORCE_QID(0);
	RTMP_IO_WRITE32(ad, SCH_REG4, value);

	RTMP_IO_READ32(ad, SCH_REG4, &value);
	value |= (1 << 8);
	RTMP_IO_WRITE32(ad, SCH_REG4, value);
	RTMP_IO_READ32(ad, SCH_REG4, &value);
	value &= ~(1 << 8);
	RTMP_IO_WRITE32(ad, SCH_REG4, value);
#endif

	return ret;
}


NDIS_STATUS AndesMTLoadFw(RTMP_ADAPTER *pAd)
{
	UINT32 Ret;
	RTMP_CHIP_CAP *Cap = &pAd->chipCap;

	if (Cap->DownLoadType == DownLoadTypeA)
	{
		Ret = AndesMTLoadFwMethod1(pAd);
	}
	else if (Cap->DownLoadType == DownLoadTypeB)
	{
		Ret = AndesMTLoadFwMethod2(pAd);
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s: Unknow Download type(%d)\n", __FUNCTION__, Cap->DownLoadType));
		Ret = -1;
	}

	return Ret;
}


NDIS_STATUS AndesMTLoadRomPatch(RTMP_ADAPTER *ad)
{
	UINT32 value, loop;
	UINT32 ret;
	RTMP_CHIP_CAP *cap = &ad->chipCap;
	RTMP_CHIP_OP *pChipOps = &ad->chipOps;
	//POS_COOKIE obj = (POS_COOKIE)ad->OS_Cookie;
	struct MCU_CTRL *Ctl = &ad->MCUCtrl;
	UINT32 patch_len = 0, total_checksum = 0;

	if (cap->load_code_method == BIN_FILE_METHOD) {
		DBGPRINT(RT_DEBUG_OFF, ("load fw image from /lib/firmware/%s\n", cap->fw_bin_file_name));
		DBGPRINT(RT_DEBUG_ERROR, ("%s:Not support now(/lib/firmware/%s), load_method(%d)\n", __FUNCTION__, cap->rom_patch_bin_file_name, cap->load_code_method));
	} else {
		cap->rom_patch = cap->rom_patch_header_image;
	}

	if (!cap->rom_patch) {
		if (cap->load_code_method == BIN_FILE_METHOD) {
			DBGPRINT(RT_DEBUG_ERROR, ("%s:Please assign a fw image(/lib/firmware/%s), load_method(%d)\n", __FUNCTION__, cap->fw_bin_file_name, cap->load_code_method));
		} else {
			DBGPRINT(RT_DEBUG_ERROR, ("%s:Please assign a fw image, load_method(%d)\n",
				__FUNCTION__, cap->load_code_method));
		}
		ret = NDIS_STATUS_FAILURE;
		goto done;
	}

	Ctl->Stage = ROM_PATCH_DOWNLOAD;

	DBGPRINT(RT_DEBUG_TRACE, ("Build Date:"));

	for (loop = 0; loop < 16; loop++)
		DBGPRINT(RT_DEBUG_OFF, ("%c", *(cap->rom_patch + loop)));

	DBGPRINT(RT_DEBUG_OFF, ("\n"));

	DBGPRINT(RT_DEBUG_OFF, ("platform = \n"));

	for (loop = 0; loop < 4; loop++)
		DBGPRINT(RT_DEBUG_OFF, ("%c", *(cap->rom_patch + 16 + loop)));

	DBGPRINT(RT_DEBUG_OFF, ("\n"));

	DBGPRINT(RT_DEBUG_OFF, ("hw/sw version = \n"));

	for (loop = 0; loop < 4; loop++)
		DBGPRINT(RT_DEBUG_OFF, ("%02x", *(cap->rom_patch + 20 + loop)));

	DBGPRINT(RT_DEBUG_OFF, ("\n"));

	DBGPRINT(RT_DEBUG_OFF, ("patch version = \n"));

	for (loop = 0; loop < 4; loop++)
		DBGPRINT(RT_DEBUG_OFF, ("%02x", *(cap->rom_patch + 24 + loop)));

	DBGPRINT(RT_DEBUG_OFF, ("\n"));

	total_checksum = *(cap->rom_patch + 28) | (*(cap->rom_patch + 29) << 8);

	patch_len = cap->rom_patch_len - PATCH_INFO_SIZE;

	DBGPRINT(RT_DEBUG_INFO, ("\ndownload len = %d\n", patch_len));

#if defined(RTMP_PCI_SUPPORT) || defined(RTMP_USB_SUPPORT)
	/* switch to bypass mode */
	RTMP_IO_READ32(ad, SCH_REG4, &value);
	value &= ~SCH_REG4_BYPASS_MODE_MASK;
	value |= SCH_REG4_BYPASS_MODE(1);
	RTMP_IO_WRITE32(ad, SCH_REG4, value);
#endif


	/* standard CMD procedure */
	/* 1. Config PDA phase */
	ret = CmdAddressLenReq(ad, ROM_PATCH_START_ADDRESS, patch_len, TARGET_ADDR_LEN_NEED_RSP);

	if (ret)
		goto done;

	/* 2. Loading rom patch image phase */
	ret = CmdFwScatters(ad, cap->rom_patch + PATCH_INFO_SIZE, patch_len);

	if (ret)
		goto done;

	/* 3. ROM patch start negotiation phase */
	ret = CmdPatchFinishReq(ad);

	DBGPRINT(RT_DEBUG_OFF, ("Send checksum req..\n"));

	pChipOps->AndesMTChkCrc(ad, patch_len);

	RtmpOsMsDelay(20);

	if (total_checksum != pChipOps->AndesMTGetCrc(ad)) {
		DBGPRINT(RT_DEBUG_OFF, ("checksum fail!, local(0x%x) <> fw(0x%x)\n", total_checksum,
										pChipOps->AndesMTGetCrc(ad)));

		ret = NDIS_STATUS_FAILURE;
	}

	DBGPRINT(RT_DEBUG_OFF, ("checksum=0x%x\n", pChipOps->AndesMTGetCrc(ad)));


done:

#if defined(RTMP_PCI_SUPPORT) || defined(RTMP_USB_SUPPORT)
	/* Switch to normal mode */
	RTMP_IO_READ32(ad, SCH_REG4, &value);
	value &= ~SCH_REG4_BYPASS_MODE_MASK;
	value |= SCH_REG4_BYPASS_MODE(0);
	value &= ~SCH_REG4_FORCE_QID_MASK;
	value |= SCH_REG4_FORCE_QID(0);
	RTMP_IO_WRITE32(ad, SCH_REG4, value);

	RTMP_IO_READ32(ad, SCH_REG4, &value);
	value |= (1 << 8);
	RTMP_IO_WRITE32(ad, SCH_REG4, value);
	RTMP_IO_READ32(ad, SCH_REG4, &value);
	value &= ~(1 << 8);
	RTMP_IO_WRITE32(ad, SCH_REG4, value);
#endif

	return ret;
}


INT32 AndesMTEraseFw(RTMP_ADAPTER *pAd)
{
	RTMP_CHIP_CAP *cap = &pAd->chipCap;

	if (cap->load_code_method == BIN_FILE_METHOD) {

		if (cap->FWImageName) {
			os_free_mem(NULL, cap->FWImageName);
			cap->FWImageName = NULL;
		}
	}

	return 0;
}


static VOID EventChPrivilegeHandler(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	//struct cmd_msg *msg, *msg_tmp;
	struct MCU_CTRL *ctl = &pAd->MCUCtrl;
	UINT32 Value;

	if (IS_MT7603(pAd) || IS_MT7628(pAd))
	{
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

	DBGPRINT(RT_DEBUG_INFO, ("%s\n", __FUNCTION__));
}


static VOID ExtEventFwLog2HostHandler(RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length)
{
	DBGPRINT(RT_DEBUG_TRACE, ("FW LOG: %s\n", Data));
}


static VOID EventExtEventHandler(RTMP_ADAPTER *pAd, UINT8 ExtEID, UINT8 *Data, UINT32 Length)
{
	switch (ExtEID)
	{
		case EXT_EVENT_CMD_RESULT:
			EventExtCmdResultHandler(pAd, Data, Length);			
			break;
		case EXT_EVENT_FW_LOG_2_HOST:
			ExtEventFwLog2HostHandler(pAd, Data, Length);
			break;
#ifdef MT_PS
#if defined(MT7603) && defined(RTMP_PCI_SUPPORT)
		case EXT_CMD_PS_RETRIEVE_START:
			CmdPsRetrieveStartRspFromCR(pAd, Data, Length);
			break; 
#endif /* MT7603 */
#endif /* MT_PS */
#ifdef THERMAL_PROTECT_SUPPORT
		case EXT_EVENT_THERMAL_PROTECT:
			EventThermalProtect(pAd, Data, Length);
			break;
#endif
#ifdef BCN_OFFLOAD_SUPPORT
        case EXT_EVENT_BCN_UPDATE:
            ExtEventBcnUpdateHandler(pAd, Data, Length);
            break;
#endif

		default:
			DBGPRINT(RT_DEBUG_OFF, ("%s: Unknown Ext Event(%x)\n", __FUNCTION__,
										ExtEID));
			break;
	}


}


static VOID UnsolicitedEventHandler(RTMP_ADAPTER *pAd, UINT8 EID, UINT8 ExtEID, UINT8 *Data, UINT32 Length)
{
	switch (EID)
	{
		case EVENT_CH_PRIVILEGE:
			EventChPrivilegeHandler(pAd, Data, Length);
			break;
		case EXT_EVENT:
			EventExtEventHandler(pAd, ExtEID, Data, Length);
			break;
		default:
			DBGPRINT(RT_DEBUG_OFF, ("%s: Unknown Event(%x)\n", __FUNCTION__, EID));
			break;
	}
}


static VOID AndesMTRxProcessEvent(RTMP_ADAPTER *pAd, struct cmd_msg *rx_msg)
{
	PNDIS_PACKET net_pkt = rx_msg->net_pkt;
	struct cmd_msg *msg, *msg_tmp;
	EVENT_RXD *event_rxd = (EVENT_RXD *)GET_OS_PKT_DATAPTR(net_pkt);
	struct MCU_CTRL *ctl = &pAd->MCUCtrl;
#ifdef RTMP_PCI_SUPPORT
	unsigned long flags;
#endif /* RTMP_PCI_SUPPORT */
	//event_rxd->fw_rxd_0.word = le2cpu32(event_rxd->fw_rxd_0.word);
	event_rxd->fw_rxd_1.word = le2cpu32(event_rxd->fw_rxd_1.word);
	event_rxd->fw_rxd_2.word = le2cpu32(event_rxd->fw_rxd_2.word);

#ifdef CONFIG_TRACE_SUPPORT
	TRACE_MCU_EVENT_INFO(event_rxd->fw_rxd_0.field.length, event_rxd->fw_rxd_0.field.pkt_type_id,
								event_rxd->fw_rxd_1.field.eid, event_rxd->fw_rxd_1.field.seq_num,
							event_rxd->fw_rxd_2.field.ext_eid, GET_OS_PKT_DATAPTR(net_pkt) + sizeof(*event_rxd), event_rxd->fw_rxd_0.field.length - sizeof(*event_rxd));
#endif



	if ((event_rxd->fw_rxd_1.field.seq_num == 0) ||
			(event_rxd->fw_rxd_2.field.ext_eid == EXT_EVENT_FW_LOG_2_HOST) ||
			(event_rxd->fw_rxd_2.field.ext_eid == EXT_EVENT_THERMAL_PROTECT)) {
		/* if have callback function */
		UnsolicitedEventHandler(pAd, event_rxd->fw_rxd_1.field.eid, event_rxd->fw_rxd_2.field.ext_eid, GET_OS_PKT_DATAPTR(net_pkt) + sizeof(*event_rxd),
												event_rxd->fw_rxd_0.field.length - sizeof(*event_rxd));
	}
	else
	{
#if defined(RTMP_USB_SUPPORT) || defined(RTMP_SDIO_SUPPORT)
		RTMP_SPIN_LOCK_IRQ(&ctl->ackq_lock);
#endif

#ifdef RTMP_PCI_SUPPORT
		RTMP_SPIN_LOCK_IRQSAVE(&ctl->ackq_lock, &flags);
#endif
		DlListForEachSafe(msg, msg_tmp, &ctl->ackq, struct cmd_msg, list) {
			if (msg->seq == event_rxd->fw_rxd_1.field.seq_num)
			{
			#if defined(RTMP_USB_SUPPORT) || defined(RTMP_SDIO_SUPPORT)
				RTMP_SPIN_UNLOCK_IRQ(&ctl->ackq_lock);
#endif

#ifdef RTMP_PCI_SUPPORT
				RTMP_SPIN_UNLOCK_IRQRESTORE(&ctl->ackq_lock, &flags);
#endif

				if ((event_rxd->fw_rxd_1.field.eid == MT_FW_START_RSP)
						|| (event_rxd->fw_rxd_1.field.eid == MT_RESTART_DL_RSP)
						|| (event_rxd->fw_rxd_1.field.eid == MT_TARGET_ADDRESS_LEN_RSP)
						|| (event_rxd->fw_rxd_1.field.eid == MT_PATCH_SEM_RSP)) {
					msg->rsp_handler(msg, (char*)(GET_OS_PKT_DATAPTR(net_pkt) + sizeof(*event_rxd) - 4),
											event_rxd->fw_rxd_0.field.length - sizeof(*event_rxd) + 4);

				}
				else if ((msg->rsp_payload_len == event_rxd->fw_rxd_0.field.length - sizeof(*event_rxd))
						&& (msg->rsp_payload_len != 0))
				{
					if (msg->rsp_handler == NULL)
					{
						DBGPRINT(RT_DEBUG_ERROR, ("%s(): rsp_handler is NULL!!!! (cmd_type = 0x%x, ext_cmd_type = 0x%x)\n", 
							__FUNCTION__, msg->cmd_type, msg->ext_cmd_type));
					}
					else
					{
						msg->rsp_handler(msg, GET_OS_PKT_DATAPTR(net_pkt) + sizeof(*event_rxd), 
												event_rxd->fw_rxd_0.field.length - sizeof(*event_rxd));
					}
				}
				else
				{
					DBGPRINT(RT_DEBUG_ERROR, ("expect response len(%d), command response len(%d) invalid\n", msg->rsp_payload_len, event_rxd->fw_rxd_0.field.length - sizeof(*event_rxd)));
					msg->rsp_payload_len = event_rxd->fw_rxd_0.field.length - sizeof(*event_rxd);
				}

				if (msg->need_wait) {
					RTMP_OS_COMPLETE(&msg->ack_done);
				} else {
					AndesFreeCmdMsg(msg);
				}
#if defined(RTMP_USB_SUPPORT) || defined(RTMP_SDIO_SUPPORT)
				RTMP_SPIN_LOCK_IRQ(&ctl->ackq_lock);
#endif

#ifdef RTMP_PCI_SUPPORT
				RTMP_SPIN_LOCK_IRQSAVE(&ctl->ackq_lock, &flags);
#endif

				break;
			}
		}

#if defined(RTMP_USB_SUPPORT) || defined(RTMP_SDIO_SUPPORT)
		RTMP_SPIN_UNLOCK_IRQ(&ctl->ackq_lock);
#endif

#ifdef RTMP_PCI_SUPPORT
		RTMP_SPIN_UNLOCK_IRQRESTORE(&ctl->ackq_lock, &flags);
#endif
	}
}


VOID AndesMTRxEventHandler(RTMP_ADAPTER *pAd, UCHAR *data)
{
	struct cmd_msg *msg;
	struct MCU_CTRL *ctl = &pAd->MCUCtrl;
	EVENT_RXD *event_rxd = (EVENT_RXD *)data;

	if (!OS_TEST_BIT(MCU_INIT, &ctl->flags)) {
		return;
	}

	DBGPRINT(RT_DEBUG_INFO, ("%s\n", __FUNCTION__));

	msg = AndesAllocCmdMsg(pAd, event_rxd->fw_rxd_0.field.length);

	if (!msg || !msg->net_pkt)
		return;

	AndesAppendCmdMsg(msg, (char *)data, event_rxd->fw_rxd_0.field.length);

	AndesMTRxProcessEvent(pAd, msg);

#ifdef RTMP_PCI_SUPPORT
	if (msg->net_pkt)
		RTMPFreeNdisPacket(pAd, msg->net_pkt);
#endif
		
	AndesFreeCmdMsg(msg);
}


#ifdef RTMP_PCI_SUPPORT
VOID AndesMTPciFwInit(RTMP_ADAPTER *pAd)
{
	struct MCU_CTRL *Ctl = &pAd->MCUCtrl;

	DBGPRINT(RT_DEBUG_TRACE, ("%s\n", __FUNCTION__));
	Ctl->Stage = FW_NO_INIT;
	/* Enable Interrupt*/
	RTMP_IRQ_ENABLE(pAd);
	RT28XXDMAEnable(pAd);
	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_START_UP);
	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_MCU_SEND_IN_BAND_CMD);
}


VOID AndesMTPciFwExit(RTMP_ADAPTER *pAd)
{
	DBGPRINT(RT_DEBUG_TRACE, ("%s\n", __FUNCTION__));
	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_START_UP);
	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_MCU_SEND_IN_BAND_CMD);
	RT28XXDMADisable(pAd);
	RTMP_ASIC_INTERRUPT_DISABLE(pAd);
}
#endif /* RTMP_PCI_SUPPORT */




#ifdef RTMP_SDIO_SUPPORT
VOID AndesMTSdioFwInit(RTMP_ADAPTER *pAd)
{
	struct MCU_CTRL *Ctl = &pAd->MCUCtrl;
	UINT32 value;

	DBGPRINT(RT_DEBUG_TRACE, ("%s\n", __FUNCTION__));
	Ctl->Stage = FW_NO_INIT;
	/* Enable Interrupt*/
	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_START_UP);
	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_MCU_SEND_IN_BAND_CMD);

	RTMP_SDIO_WRITE32(pAd, WHIER, (RX0_DONE_INT_EN |RX1_DONE_INT_EN));
	RTMP_SDIO_READ32(pAd, WHIER,&value);
	printk("%s(): MCR_WHIER: value:%x\n", __FUNCTION__,value);
	RTMP_SDIO_READ32(pAd, WHISR,&value);
	printk("%s(): MCR_WHISR1: value:%x\n", __FUNCTION__,value); 
	RTMP_SDIO_WRITE32(pAd, WHLPCR, W_INT_EN_SET);

}


VOID AndesMTSdioFwExit(RTMP_ADAPTER *pAd)
{
	DBGPRINT(RT_DEBUG_TRACE, ("%s\n", __FUNCTION__));
	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_START_UP);
	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_MCU_SEND_IN_BAND_CMD);
}
#endif


static VOID CmdEfuseBufferModeRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	struct _EVENT_EXT_CMD_RESULT_T *EventExtCmdResult = (struct _EVENT_EXT_CMD_RESULT_T *)Data;
	//RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)msg->priv;

	DBGPRINT(RT_DEBUG_INFO, ("%s: EventExtCmdResult.ucExTenCID = 0x%x\n",__FUNCTION__, EventExtCmdResult->ucExTenCID));
	EventExtCmdResult->u4Status = le2cpu32(EventExtCmdResult->u4Status);
	DBGPRINT(RT_DEBUG_INFO, ("%s: EventExtCmdResult.u4Status = 0x%x\n",__FUNCTION__, EventExtCmdResult->u4Status));
}

static VOID CmdThemalSensorRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	struct _EXT_EVENT_GET_SENSOR_RESULT_T *EventExtCmdResult = (struct _EXT_EVENT_GET_SENSOR_RESULT_T *)Data;
	//RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)msg->priv;

	EventExtCmdResult->u4SensorResult = le2cpu32(EventExtCmdResult->u4SensorResult);
	DBGPRINT(RT_DEBUG_OFF, ("ThemalSensor = 0x%x\n", EventExtCmdResult->u4SensorResult));
}


static inline VOID bufferModeFieldSet(RTMP_ADAPTER *pAd,EXT_CMD_EFUSE_BUFFER_MODE_T *pCmd,UINT16 addr)
{
	UINT32 i = pCmd->ucCount;
	pCmd->aBinContent[i].u2Addr = cpu2le16(addr);
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[addr] ;
	pCmd->ucCount++;	
}

static VOID CmdExtPmMgtBitRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	struct _EVENT_EXT_CMD_RESULT_T *EventExtCmdResult = (struct _EVENT_EXT_CMD_RESULT_T *)Data;
	//RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)msg->priv;

	DBGPRINT(RT_DEBUG_OFF, ("%s: EventExtCmdResult.ucExTenCID = 0x%x\n",__FUNCTION__, EventExtCmdResult->ucExTenCID));
	EventExtCmdResult->u4Status = le2cpu32(EventExtCmdResult->u4Status);
	DBGPRINT(RT_DEBUG_OFF, ("%s: EventExtCmdResult.u4Status = 0x%x\n",__FUNCTION__, EventExtCmdResult->u4Status));
}

static VOID CmdExtPmStateCtrlRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	struct _EVENT_EXT_CMD_RESULT_T *EventExtCmdResult = (struct _EVENT_EXT_CMD_RESULT_T *)Data;
	//RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)msg->priv;

	DBGPRINT(RT_DEBUG_OFF, ("%s: EventExtCmdResult.ucExTenCID = 0x%x\n",__FUNCTION__, EventExtCmdResult->ucExTenCID));
	EventExtCmdResult->u4Status = le2cpu32(EventExtCmdResult->u4Status);
	DBGPRINT(RT_DEBUG_OFF, ("%s: EventExtCmdResult.u4Status = 0x%x\n",__FUNCTION__, EventExtCmdResult->u4Status));
}

static VOID CmdFillEeprom(RTMP_ADAPTER *pAd,EXT_CMD_EFUSE_BUFFER_MODE_T *pCmd)
{
	int i=0;
	pCmd->aBinContent[i].u2Addr = NIC_CONFIGURE_0_TOP;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[NIC_CONFIGURE_0_TOP] ;
	i++;
	pCmd->aBinContent[i].u2Addr = NIC_CONFIGURE_1;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[NIC_CONFIGURE_1];
	i++;
	pCmd->aBinContent[i].u2Addr = NIC_CONFIGURE_1_TOP;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[NIC_CONFIGURE_1_TOP];
	i++;
	pCmd->aBinContent[i].u2Addr = WIFI_RF_SETTING;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[WIFI_RF_SETTING];
	i++;
	pCmd->aBinContent[i].u2Addr = G_BAND_20_40_BW_PWR_DELTA;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[G_BAND_20_40_BW_PWR_DELTA];
	i++;
	pCmd->aBinContent[i].u2Addr = A_BAND_20_80_BW_PWR_DELTA_ANALOG;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[A_BAND_20_80_BW_PWR_DELTA_ANALOG];
	i++;
	pCmd->aBinContent[i].u2Addr = A_BAND_EXT_PA_SETTING;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[A_BAND_EXT_PA_SETTING];
	i++;
	pCmd->aBinContent[i].u2Addr = TEMP_SENSOR_CAL;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[TEMP_SENSOR_CAL];
	i++;
	pCmd->aBinContent[i].u2Addr = TX0_G_BAND_TSSI_SLOPE;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[TX0_G_BAND_TSSI_SLOPE];
	i++;
	pCmd->aBinContent[i].u2Addr = TX0_G_BAND_TSSI_SLOPE_TOP;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[TX0_G_BAND_TSSI_SLOPE_TOP];
	i++;
	pCmd->aBinContent[i].u2Addr = TX0_G_BAND_TARGET_PWR;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[TX0_G_BAND_TARGET_PWR];
	i++;
	pCmd->aBinContent[i].u2Addr = TX0_G_BAND_OFFSET_LOW;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[TX0_G_BAND_OFFSET_LOW];
	i++;
	pCmd->aBinContent[i].u2Addr = TX0_G_BAND_CHL_PWR_DELTA_MID;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[TX0_G_BAND_CHL_PWR_DELTA_MID];
	i++;
	pCmd->aBinContent[i].u2Addr = TX0_G_BAND_OFFSET_HIGH;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[TX0_G_BAND_OFFSET_HIGH];
	i++;
	pCmd->aBinContent[i].u2Addr = TX1_G_BAND_TSSI_SLOPE;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[TX1_G_BAND_TSSI_SLOPE];
	i++;
	pCmd->aBinContent[i].u2Addr = TX1_G_BAND_TSSI_SLOPE_TOP;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[TX1_G_BAND_TSSI_SLOPE_TOP];
	i++;
	pCmd->aBinContent[i].u2Addr = TX1_G_BAND_TARGET_PWR;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[TX1_G_BAND_TARGET_PWR];
	i++;
	pCmd->aBinContent[i].u2Addr = TX1_G_BAND_CHL_PWR_DELATE_LOW;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[TX1_G_BAND_CHL_PWR_DELATE_LOW];
	i++;
	pCmd->aBinContent[i].u2Addr = TX1_G_BAND_CHL_PWR_DELTA_MID;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[TX1_G_BAND_CHL_PWR_DELTA_MID];
	i++;
	pCmd->aBinContent[i].u2Addr = TX1_G_BAND_CHL_PWR_DELTA_HIGH;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[TX1_G_BAND_CHL_PWR_DELTA_HIGH];
	i++;
	pCmd->aBinContent[i].u2Addr = TX_PWR_CCK_1_2M;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[TX_PWR_CCK_1_2M];
	i++;
	pCmd->aBinContent[i].u2Addr = TX_PWR_CCK_5_11M;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[TX_PWR_CCK_5_11M];
	i++;
	pCmd->aBinContent[i].u2Addr = TX_PWR_G_BAND_OFDM_6_9M;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[TX_PWR_G_BAND_OFDM_6_9M];
	i++;
	pCmd->aBinContent[i].u2Addr = TX_PWR_OFDM_12_18M;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[TX_PWR_OFDM_12_18M];
	i++;
	pCmd->aBinContent[i].u2Addr = TX_PWR_G_BAND_OFDM_24_36M;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[TX_PWR_G_BAND_OFDM_24_36M];
	i++;	
	pCmd->aBinContent[i].u2Addr = TX_PWR_G_BNAD_OFDM_48;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[TX_PWR_G_BNAD_OFDM_48];
	i++;
	pCmd->aBinContent[i].u2Addr = TX_PWR_G_BAND_OFDM_54M;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[TX_PWR_G_BAND_OFDM_54M];
	i++;
	pCmd->aBinContent[i].u2Addr = TX_PWR_HT_BPSK_MCS_0_8;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[TX_PWR_HT_BPSK_MCS_0_8];
	i++;
	pCmd->aBinContent[i].u2Addr = TX_PWR_HT_BPSK_MCS_32;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[TX_PWR_HT_BPSK_MCS_32];
	i++;
	pCmd->aBinContent[i].u2Addr = TX_PWR_HT_QPSK_MCS_1_2_9_10;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[TX_PWR_HT_QPSK_MCS_1_2_9_10];
	i++;
	pCmd->aBinContent[i].u2Addr = TX_PWR_HT_16QAM_MCS_3_4_11_12;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[TX_PWR_HT_16QAM_MCS_3_4_11_12];
	i++;
	pCmd->aBinContent[i].u2Addr = TX_PWR_HT_64QAM_MCS_5_13;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[TX_PWR_HT_64QAM_MCS_5_13];
	i++;
	pCmd->aBinContent[i].u2Addr = TX_PWR_HT_64QAM_MCS_6_14;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[TX_PWR_HT_64QAM_MCS_6_14];
	i++;
	pCmd->aBinContent[i].u2Addr = TX_PWR_HT_64QAM_MCS_7_15;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[TX_PWR_HT_64QAM_MCS_7_15];
	i++;
	pCmd->aBinContent[i].u2Addr = ELAN_RX_MODE_GAIN;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[ELAN_RX_MODE_GAIN];
	i++;
	pCmd->aBinContent[i].u2Addr = ELAN_RX_MODE_NF;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[ELAN_RX_MODE_NF];
	i++;
	pCmd->aBinContent[i].u2Addr = ELAN_RX_MODE_P1DB;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[ELAN_RX_MODE_P1DB];
	i++;
	pCmd->aBinContent[i].u2Addr = ELAN_BYPASS_MODE_GAIN;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[ELAN_BYPASS_MODE_GAIN];
	i++;
	pCmd->aBinContent[i].u2Addr = ELAN_BYPASS_MODE_NF;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[ELAN_BYPASS_MODE_NF];
	i++;
	pCmd->aBinContent[i].u2Addr = ELAN_BYPASS_MODE_P1DB;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[ELAN_BYPASS_MODE_P1DB];
	i++;
	pCmd->aBinContent[i].u2Addr = STEP_NUM_NEG_7;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[STEP_NUM_NEG_7];
	i++;
	pCmd->aBinContent[i].u2Addr = STEP_NUM_NEG_6;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[STEP_NUM_NEG_6];
	i++;
	pCmd->aBinContent[i].u2Addr = STEP_NUM_NEG_5;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[STEP_NUM_NEG_5];
	i++;
	pCmd->aBinContent[i].u2Addr = STEP_NUM_NEG_4;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[STEP_NUM_NEG_4];
	i++;
	pCmd->aBinContent[i].u2Addr = STEP_NUM_NEG_3;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[STEP_NUM_NEG_3];
	i++;
	pCmd->aBinContent[i].u2Addr = STEP_NUM_NEG_2;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[STEP_NUM_NEG_2];
	i++;
	pCmd->aBinContent[i].u2Addr = STEP_NUM_NEG_1;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[STEP_NUM_NEG_1];
	i++;
	pCmd->aBinContent[i].u2Addr = STEP_NUM_NEG_0;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[STEP_NUM_NEG_0];
	i++;
	pCmd->aBinContent[i].u2Addr = REF_STEP_24G;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[REF_STEP_24G];
	i++;
	pCmd->aBinContent[i].u2Addr = REF_TEMP_24G;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[REF_TEMP_24G];
	i++;
	pCmd->aBinContent[i].u2Addr = STEP_NUM_PLUS_1;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[STEP_NUM_PLUS_1];
	i++;
	pCmd->aBinContent[i].u2Addr = STEP_NUM_PLUS_2;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[STEP_NUM_PLUS_2];
	i++;
	pCmd->aBinContent[i].u2Addr = STEP_NUM_PLUS_3;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[STEP_NUM_PLUS_3];
	i++;
	pCmd->aBinContent[i].u2Addr = STEP_NUM_PLUS_4;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[STEP_NUM_PLUS_4];
	i++;
	pCmd->aBinContent[i].u2Addr = STEP_NUM_PLUS_5;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[STEP_NUM_PLUS_5];
	i++;
	pCmd->aBinContent[i].u2Addr = STEP_NUM_PLUS_6;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[STEP_NUM_PLUS_6];
	i++;
	pCmd->aBinContent[i].u2Addr = STEP_NUM_PLUS_7;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[STEP_NUM_PLUS_7];
	i++;
	pCmd->aBinContent[i].u2Addr = XTAL_CALIB_FREQ_OFFSET;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[XTAL_CALIB_FREQ_OFFSET];
	i++;
	pCmd->aBinContent[i].u2Addr = XTAL_TRIM_2_COMP;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[XTAL_TRIM_2_COMP];
	i++;
	pCmd->aBinContent[i].u2Addr = XTAL_TRIM_3_COMP;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[XTAL_TRIM_3_COMP];
	i++;
	pCmd->aBinContent[i].u2Addr = WF_RCAL;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[WF_RCAL];
	i++;

	pCmd->ucCount = i;
	/*check endian and transfer it is needed*/
	for(i=0;i<pCmd->ucCount ;i++)
	{
		pCmd->aBinContent[i].u2Addr = cpu2le16(pCmd->aBinContent[i].u2Addr);
	}

	/*extend for function requset, need backward compatible*/
	bufferModeFieldSet(pAd,pCmd,0x24);	
	bufferModeFieldSet(pAd,pCmd,0x25);
	bufferModeFieldSet(pAd,pCmd,0x34);
	bufferModeFieldSet(pAd,pCmd,0x39);
	bufferModeFieldSet(pAd,pCmd,0x3b);
	bufferModeFieldSet(pAd,pCmd,0x42);
	bufferModeFieldSet(pAd,pCmd,0x43);
	bufferModeFieldSet(pAd,pCmd,0x9e);
	bufferModeFieldSet(pAd,pCmd,0x9f);
	bufferModeFieldSet(pAd,pCmd,0xf2);
	bufferModeFieldSet(pAd,pCmd,0xf8);
	bufferModeFieldSet(pAd,pCmd,0xf9);
	bufferModeFieldSet(pAd,pCmd,0xfa);
	bufferModeFieldSet(pAd,pCmd,0x12e);	

	for(i=0;i<=0xf;i++)
	{
		bufferModeFieldSet(pAd,pCmd,0x130+i);
	}
	
	/*need minus 1 for add one more time*/
	pCmd->ucCount--;
	
}

VOID CmdEfusBufferModeSet(RTMP_ADAPTER *pAd)
{

	struct cmd_msg *msg;
	EXT_CMD_EFUSE_BUFFER_MODE_T CmdEfuseBufferMode;
	int ret = 0;
	msg = AndesAllocCmdMsg(pAd, sizeof(EXT_CMD_EFUSE_BUFFER_MODE_T));

	if (!msg)
	{
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	AndesInitCmdMsg(msg, P1_Q0, EXT_CID, CMD_SET, EXT_CMD_EFUSE_BUFFER_MODE, TRUE, 0,TRUE, TRUE, 8, NULL, CmdEfuseBufferModeRsp);
	memset(&CmdEfuseBufferMode, 0x00, sizeof(CmdEfuseBufferMode));
	switch(pAd->eeprom_type){
	case EEPROM_EFUSE:
	{
		CmdEfuseBufferMode.ucSourceMode = EEPROM_MODE_EFUSE;
		CmdEfuseBufferMode.ucCount = 0;
	}
	break;
	case EEPROM_FLASH:
	{
		CmdEfuseBufferMode.ucSourceMode = EEPROM_MODE_BUFFER;
		CmdFillEeprom(pAd,&CmdEfuseBufferMode);
	}
	break;
	default:
		goto error;
	break;
	}
	
	AndesAppendCmdMsg(msg, (char *)&CmdEfuseBufferMode, sizeof(CmdEfuseBufferMode));
	ret = AndesSendCmdMsg(pAd, msg);

error:
	DBGPRINT(RT_DEBUG_INFO, ("%s:(ret = %d)\n", __FUNCTION__, ret));
	return ;

}

VOID CmdSetTxPowerCtrl(RTMP_ADAPTER *pAd, UINT8 central_chl)
{
	struct cmd_msg *msg;
	EXT_CMD_TX_POWER_CTRL_T CmdTxPwrCtrl;
	int ret = 0;
	int i, j;
	UINT8 PwrPercentageDelta = 0;
	USHORT Value;
	struct MT_TX_PWR_CAP *cap = &pAd->chipCap.MTTxPwrCap;

	msg = AndesAllocCmdMsg(pAd, sizeof(EXT_CMD_TX_POWER_CTRL_T));

	if (!msg)
	{
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	AndesInitCmdMsg(msg, P1_Q0, EXT_CID, CMD_SET, EXT_CMD_SET_TX_POWER_CTRL, TRUE, 0,
		TRUE, TRUE, 8, NULL, EventExtCmdResult);
	memset(&CmdTxPwrCtrl, 0x00, sizeof(CmdTxPwrCtrl));

	CmdTxPwrCtrl.ucCenterChannel = central_chl;
	
	if (pAd->eeprom_type == EEPROM_EFUSE)
	{
		
		RT28xx_EEPROM_READ16(pAd, NIC_CONFIGURE_1, Value);
		CmdTxPwrCtrl.ucTSSIEnable = ((Value & 0xff00) >> 8);

		RT28xx_EEPROM_READ16(pAd, NIC_CONFIGURE_1, Value);
		CmdTxPwrCtrl.ucTempCompEnable = (Value & 0xff);
	
		RT28xx_EEPROM_READ16(pAd, TX0_G_BAND_TARGET_PWR, Value);
		CmdTxPwrCtrl.aucTargetPower[0] = Value & 0xff;
		RT28xx_EEPROM_READ16(pAd, TX1_G_BAND_TARGET_PWR, Value);
		CmdTxPwrCtrl.aucTargetPower[1] = Value & 0xff;

		j = 0;

		for (i = 0; i < 7; i++)
		{
			RT28xx_EEPROM_READ16(pAd, TX_PWR_CCK_1_2M + (i * 2), Value);
			CmdTxPwrCtrl.aucRatePowerDelta[j] = Value & 0xff;
			j++;
			CmdTxPwrCtrl.aucRatePowerDelta[j] = ((Value & 0xff00) >> 8);
			j++;
		}

		RT28xx_EEPROM_READ16(pAd, G_BAND_20_40_BW_PWR_DELTA, Value);
		CmdTxPwrCtrl.ucBWPowerDelta = Value & 0xff;

		RT28xx_EEPROM_READ16(pAd, TX0_G_BAND_TARGET_PWR, Value);
		CmdTxPwrCtrl.aucCHPowerDelta[0] = ((Value & 0xff00) >> 8);
		
		RT28xx_EEPROM_READ16(pAd, TX0_G_BAND_CHL_PWR_DELTA_MID, Value);
		CmdTxPwrCtrl.aucCHPowerDelta[1] = Value & 0xff;
		
		CmdTxPwrCtrl.aucCHPowerDelta[2] = ((Value & 0xff00) >> 8);
		
		RT28xx_EEPROM_READ16(pAd, TX1_G_BAND_TARGET_PWR, Value);
		CmdTxPwrCtrl.aucCHPowerDelta[3] = ((Value & 0xff00) >> 8);
		
		RT28xx_EEPROM_READ16(pAd, TX1_G_BAND_CHL_PWR_DELTA_MID, Value);
		CmdTxPwrCtrl.aucCHPowerDelta[4] = Value & 0xff;
		
		CmdTxPwrCtrl.aucCHPowerDelta[5] = ((Value & 0xff00) >> 8);
		
		j = 0;

		for (i = 0; i < 9; i++)
		{
			RT28xx_EEPROM_READ16(pAd, STEP_NUM_NEG_7 + (i * 2), Value);
			CmdTxPwrCtrl.aucTempCompPower[j] = Value & 0xff;
			j++;

			if (i != 8)
			{
				CmdTxPwrCtrl.aucTempCompPower[j] = ((Value & 0xff00) >> 8);
				j++;
			}
		}
	}
	else
	{
		CmdTxPwrCtrl.ucTSSIEnable = pAd->EEPROMImage[NIC_CONFIGURE_1_TOP];
		CmdTxPwrCtrl.ucTempCompEnable = pAd->EEPROMImage[NIC_CONFIGURE_1];

		CmdTxPwrCtrl.aucTargetPower[0] = pAd->EEPROMImage[TX0_G_BAND_TARGET_PWR];
		CmdTxPwrCtrl.aucTargetPower[1] = pAd->EEPROMImage[TX1_G_BAND_TARGET_PWR];

#ifdef CONFIG_ATE
        /* Replace Target Power from QA Tool manual setting*/
		if (ATE_ON(pAd)) {
			DBGPRINT(RT_DEBUG_TRACE, ("%s,ATE set tx power\n", __FUNCTION__));
			CmdTxPwrCtrl.aucTargetPower[0] = pAd->ATECtrl.TxPower0;
			CmdTxPwrCtrl.aucTargetPower[1] = pAd->ATECtrl.TxPower1;
		}
#endif

		NdisCopyMemory(&CmdTxPwrCtrl.aucRatePowerDelta[0], &(pAd->EEPROMImage[TX_PWR_CCK_1_2M]), sizeof(CmdTxPwrCtrl.aucRatePowerDelta));
		CmdTxPwrCtrl.ucBWPowerDelta = pAd->EEPROMImage[G_BAND_20_40_BW_PWR_DELTA];
		NdisCopyMemory(&CmdTxPwrCtrl.aucCHPowerDelta[0], &(pAd->EEPROMImage[TX0_G_BAND_OFFSET_LOW]), sizeof(CmdTxPwrCtrl.aucCHPowerDelta[0]));
		NdisCopyMemory(&CmdTxPwrCtrl.aucCHPowerDelta[1], &(pAd->EEPROMImage[TX0_G_BAND_CHL_PWR_DELTA_MID]), sizeof(CmdTxPwrCtrl.aucCHPowerDelta[1]));
		NdisCopyMemory(&CmdTxPwrCtrl.aucCHPowerDelta[2], &(pAd->EEPROMImage[TX0_G_BAND_OFFSET_HIGH]), sizeof(CmdTxPwrCtrl.aucCHPowerDelta[2]));
		NdisCopyMemory(&CmdTxPwrCtrl.aucCHPowerDelta[3], &(pAd->EEPROMImage[TX1_G_BAND_CHL_PWR_DELATE_LOW]), sizeof(CmdTxPwrCtrl.aucCHPowerDelta[3]));

		NdisCopyMemory(&CmdTxPwrCtrl.aucCHPowerDelta[4], &(pAd->EEPROMImage[TX1_G_BAND_CHL_PWR_DELTA_MID]), sizeof(CmdTxPwrCtrl.aucCHPowerDelta[4]));

		NdisCopyMemory(&CmdTxPwrCtrl.aucCHPowerDelta[5], &(pAd->EEPROMImage[TX1_G_BAND_CHL_PWR_DELTA_HIGH]), sizeof(CmdTxPwrCtrl.aucCHPowerDelta[5]));

		NdisCopyMemory(&CmdTxPwrCtrl.aucTempCompPower[0], &(pAd->EEPROMImage[STEP_NUM_NEG_7]), sizeof(CmdTxPwrCtrl.aucTempCompPower));
	}

	DBGPRINT(RT_DEBUG_INFO, ("PA type = %d\n", cap->pa_type));

//	if (!(cap->pa_type & (1 << 1)))
	if (1)
	{

		if (pAd->CommonCfg.TxPowerPercentage > 90)
		{
				PwrPercentageDelta = 0;
			}
		else if (pAd->CommonCfg.TxPowerPercentage > 60)	/* reduce Pwr for 1 dB. */
		{
			PwrPercentageDelta = 1;
		}
		else if (pAd->CommonCfg.TxPowerPercentage > 30)	/* reduce Pwr for 3 dB. */
		{
			PwrPercentageDelta = 3;
		}
		else if (pAd->CommonCfg.TxPowerPercentage > 15)	/* reduce Pwr for 6 dB. */
		{
			PwrPercentageDelta = 6;
		}
		else if (pAd->CommonCfg.TxPowerPercentage > 9)	/* reduce Pwr for 9 dB. */
		{
			PwrPercentageDelta = 9;
		}
		else /* reduce Pwr for 12 dB. */
		{
			PwrPercentageDelta = 12;
		}

		DBGPRINT(RT_DEBUG_INFO, ("Before apply tx pwr percentage, CmdTxPwrCtrl.aucTargetPower[0] = 0x%x\n", CmdTxPwrCtrl.aucTargetPower[0]));
		
		DBGPRINT(RT_DEBUG_INFO, ("Before apply tx pwr percentage, CmdTxPwrCtrl.aucTargetPower[1] = 0x%x\n", CmdTxPwrCtrl.aucTargetPower[1]));
		DBGPRINT(RT_DEBUG_INFO, ("Percentage = 0x%x\n", PwrPercentageDelta));

		CmdTxPwrCtrl.ucReserved = PwrPercentageDelta;
	}
	else
	{
#ifdef CONFIG_ATE
		if (!ATE_ON(pAd))
#endif
		{
			DBGPRINT(RT_DEBUG_TRACE, ("EPA, do not need to apply tx power percentage\n"));
			goto error;
		}
	}

	DBGPRINT(RT_DEBUG_INFO, ("CmdTxPwrCtrl.ucCenterChannel=%x\n", CmdTxPwrCtrl.ucCenterChannel));
	DBGPRINT(RT_DEBUG_INFO, ("CmdTxPwrCtrl.ucTSSIEnable=%x \n", CmdTxPwrCtrl.ucTSSIEnable));
	DBGPRINT(RT_DEBUG_INFO, ("CmdTxPwrCtrl.ucTempCompEnable=%x\n", CmdTxPwrCtrl.ucTempCompEnable));
	DBGPRINT(RT_DEBUG_INFO, ("CmdTxPwrCtrl.aucTargetPower[0]=%x\n", CmdTxPwrCtrl.aucTargetPower[0]));
	DBGPRINT(RT_DEBUG_INFO, ("CmdTxPwrCtrl.aucTargetPower[1]=%x\n", CmdTxPwrCtrl.aucTargetPower[1]));
	for(i=0; i<14;i++)
		DBGPRINT(RT_DEBUG_INFO, ("CmdTxPwrCtrl.aucRatePowerDelta[%d]=%x\n", i, CmdTxPwrCtrl.aucRatePowerDelta[i]));
	DBGPRINT(RT_DEBUG_INFO, ("CmdTxPwrCtrl.ucBWPowerDelta=%x \n",CmdTxPwrCtrl.ucBWPowerDelta));
	for(i=0;i<6;i++)
		DBGPRINT(RT_DEBUG_INFO, ("CmdTxPwrCtrl.aucCHPowerDelta[%d]=%x\n", i, CmdTxPwrCtrl.aucCHPowerDelta[i]));
	for(i=0;i<17;i++)
		DBGPRINT(RT_DEBUG_INFO, ("CmdTxPwrCtrl.aucTempCompPower[%d]=%x\n", i, CmdTxPwrCtrl.aucTempCompPower[i]));

	AndesAppendCmdMsg(msg, (char *)&CmdTxPwrCtrl, sizeof(CmdTxPwrCtrl));
	ret = AndesSendCmdMsg(pAd, msg);

error:
	DBGPRINT(RT_DEBUG_INFO, ("%s:(ret = %d)\n", __FUNCTION__, ret));
	return;
}

VOID CmdGetThemalSensorResult(struct _RTMP_ADAPTER *pAd, UINT8 option)
{
	struct cmd_msg *msg;
	EXT_CMD_GET_SENSOR_RESULT_T CmdSensorResult;
	INT32 ret;
#ifdef CONFIG_QA
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
#endif /* CONFIG_QA */
	ret = 0;
		
	msg = AndesAllocCmdMsg(pAd, sizeof(EXT_CMD_GET_SENSOR_RESULT_T));
	
	if (!msg)
	{
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}
#ifdef CONFIG_QA	
	//ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	if(ATECtrl->bQAEnabled){
		DBGPRINT(RT_DEBUG_OFF, ("%s: Call to qa_agent\n", __FUNCTION__));
		AndesInitCmdMsg(msg, P1_Q0, EXT_CID, CMD_SET, EXT_CMD_GET_THEMAL_SENSOR, TRUE, 0, TRUE, TRUE, 8, NULL, HQA_GetThermalValue_CB);
	} else
#endif
	AndesInitCmdMsg(msg, P1_Q0, EXT_CID, CMD_SET, EXT_CMD_GET_THEMAL_SENSOR, TRUE, 0, TRUE, TRUE, 8, NULL, CmdThemalSensorRsp);

	CmdSensorResult.ucActionIdx = option;

	AndesAppendCmdMsg(msg, (char *)&CmdSensorResult, sizeof(CmdSensorResult));
	ret = AndesSendCmdMsg(pAd, msg);
	
error:
	DBGPRINT(RT_DEBUG_INFO, ("%s:(ret = %d)\n", __FUNCTION__, ret));
	return ; 
}

#ifdef LED_CONTROL_SUPPORT
INT AndesLedOP(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR LedIdx,
	IN UCHAR LinkStatus)
{
	struct cmd_msg *msg;
	CHAR *Pos, *pBuf;
	UINT32 VarLen;
	UINT32 arg0, arg1;
	INT32 ret;
	UINT32 blinkSel = 2; // 2 : data only, 0 : all Tx frames, 1 : Exclude Tx Beacon and TIM broadcast frames

	ret = 0;
	
	msg = AndesAllocCmdMsg(pAd, sizeof(LED_NMAC_CMD));

	if (!msg)
	{
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	AndesInitCmdMsg(msg, P1_Q0, EXT_CID, CMD_SET, EXT_CMD_LED_CTRL, FALSE, 0, FALSE, FALSE, 0, NULL, NULL);
	//memset(&CmdEfuseBufferMode, 0x00, sizeof(CmdEfuseBufferMode));

	
	/* Calibration ID and Parameter */
	VarLen = 8;
	arg0 = cpu2le32(LedIdx);
	arg1 = cpu2le32(LinkStatus | (blinkSel << 24));
	os_alloc_mem(pAd, (UCHAR **)&pBuf, VarLen);
	if (pBuf == NULL)
	{
		return NDIS_STATUS_RESOURCES;
	}

	NdisZeroMemory(pBuf, VarLen);
	
	Pos = pBuf;
	/* Parameter */
	
	NdisMoveMemory(Pos, &arg0, 4);
	NdisMoveMemory(Pos+4, &arg1, 4);

	Pos += 4;

	hex_dump("AndesLedOP: ", pBuf, VarLen);
	AndesAppendCmdMsg(msg, (char *)pBuf, VarLen);
	

	ret = AndesSendCmdMsg(pAd, msg);

	os_free_mem(NULL, pBuf);

error:
	DBGPRINT(RT_DEBUG_INFO, ("%s:(ret = %d)\n", __FUNCTION__, ret));
	return ret;	
}

INT AndesLedEnhanceOP(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR LedIdx,
	IN UCHAR on_time,
	IN UCHAR off_time,
	IN UCHAR Led_Parameter
	)
{
	struct cmd_msg *msg;
	CHAR *Pos, *pBuf;
	UINT32 VarLen;
	UINT32 arg0/*, arg1*/;
	INT32 ret = 0;
	//UCHAR parameter[4]={0};
	
	LED_ENHANCE led_enhance;
	msg = AndesAllocCmdMsg(pAd, sizeof(LED_NMAC_CMD));

	if (!msg)
	{
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	AndesInitCmdMsg(msg, P1_Q0, EXT_CID, CMD_SET, EXT_CMD_LED_CTRL, FALSE, 0, FALSE, FALSE, 0, NULL, NULL);
	//memset(&CmdEfuseBufferMode, 0x00, sizeof(CmdEfuseBufferMode));

	
	/* Calibration ID and Parameter */
	VarLen = 8;
	arg0 = LedIdx;
	//arg1 = LinkStatus;
	//parameter[1]=on_time;
	//parameter[2]=off_time;
	//parameter[3]=31;
	led_enhance.word = 0;
	led_enhance.field.on_time=on_time;
	led_enhance.field.off_time=off_time;
	led_enhance.field.idx = Led_Parameter;
	os_alloc_mem(pAd, (UCHAR **)&pBuf, VarLen);
	if (pBuf == NULL)
	{
		return NDIS_STATUS_RESOURCES;
	}

	NdisZeroMemory(pBuf, VarLen);
	
	Pos = pBuf;
	/* Parameter */
	
	NdisMoveMemory(Pos, &arg0, 4);
	//NdisMoveMemory(Pos+4, &arg1, 4);
	NdisMoveMemory(Pos+4, &led_enhance, sizeof(led_enhance));
	

	Pos += 4;

	hex_dump("AndesLedOPEnhance: ", pBuf, VarLen);
	AndesAppendCmdMsg(msg, (char *)pBuf, VarLen);
	

	ret = AndesSendCmdMsg(pAd, msg);

	os_free_mem(NULL, pBuf);

error:
	DBGPRINT(RT_DEBUG_INFO, ("%s:(ret = %d)\n", __FUNCTION__, ret));
	return ret;	
}
#endif /*LED_CONTROL_SUPPORT*/

NTSTATUS MtCmdWtblTxpsUpdate(RTMP_ADAPTER *pAd, PCmdQElmt CMDQelmt)
{
	INT32 Ret = NDIS_STATUS_FAILURE;
	struct wtbl_entry tb_entry;
	union WTBL_1_DW2 *dw2 = (union WTBL_1_DW2 *)&tb_entry.wtbl_1.wtbl_1_d2.word;
	PWTBL_TX_PS_UPDATE pWblTxpsUpdate = (PWTBL_TX_PS_UPDATE)(CMDQelmt->buffer);
	UINT8	ucWlanIdx = pWblTxpsUpdate->ucWlanIdx;
	UINT8	ucPwrMgtBit = pWblTxpsUpdate->ucPwrMgtBit;

	NdisZeroMemory(&tb_entry, sizeof(tb_entry));

	if (mt_wtbl_get_entry234(pAd, ucWlanIdx, &tb_entry) == FALSE)
	{
		Ret = NDIS_STATUS_FAILURE;
		DBGPRINT(RT_DEBUG_ERROR, ("%s():Cannot found WTBL2/3/4 for WCID(%d)\n", __FUNCTION__, ucWlanIdx));
		return Ret;
	}

	RTMP_IO_READ32(pAd, tb_entry.wtbl_addr[0] + (2 * 4), &dw2->word);
	dw2->field.tx_ps = ucPwrMgtBit;
	DBGPRINT(RT_DEBUG_ERROR, ("%s(): Set WCID(%d).TX_PS(%d)\n", __FUNCTION__, ucWlanIdx, dw2->field.tx_ps));
	RTMP_IO_WRITE32(pAd, tb_entry.wtbl_addr[0] + (2 * 4), dw2->word);

	return Ret;
}

INT32 CmdExtPwrMgtBitWifi(RTMP_ADAPTER *pAd, UINT8 ucWlanIdx, UINT8 ucPwrMgtBit, BOOLEAN ucDirectCall)
{
	if (IS_MT7603(pAd))
	{
		INT32 Ret = NDIS_STATUS_FAILURE;
		struct wtbl_entry tb_entry;
		union WTBL_1_DW2 *dw2 = (union WTBL_1_DW2 *)&tb_entry.wtbl_1.wtbl_1_d2.word;
		
		WTBL_TX_PS_UPDATE	rWblTxpsUpdate = {0};

		if (ucDirectCall)
		{
			NdisZeroMemory(&tb_entry, sizeof(tb_entry));

			if (mt_wtbl_get_entry234(pAd, ucWlanIdx, &tb_entry) == FALSE)
			{
				Ret = NDIS_STATUS_FAILURE;
				DBGPRINT(RT_DEBUG_ERROR, ("%s():Cannot found WTBL2/3/4 for WCID(%d)\n", __FUNCTION__, ucWlanIdx));
				return Ret;
			}

			RTMP_IO_READ32(pAd, tb_entry.wtbl_addr[0] + (2 * 4), &dw2->word);
			dw2->field.tx_ps = ucPwrMgtBit;
			DBGPRINT(RT_DEBUG_ERROR, ("%s(): Set WCID(%d).TX_PS(%d)\n", __FUNCTION__, ucWlanIdx, dw2->field.tx_ps));
			RTMP_IO_WRITE32(pAd, tb_entry.wtbl_addr[0] + (2 * 4), dw2->word);

			return Ret;
		}
		else
		{
			rWblTxpsUpdate.ucWlanIdx = ucWlanIdx;
			rWblTxpsUpdate.ucPwrMgtBit = ucPwrMgtBit;
				
			return RTEnqueueInternalCmd(pAd, CMDTHREAD_PWR_MGT_BIT, &rWblTxpsUpdate, sizeof(rWblTxpsUpdate));
		}
	}
	else
	{
		INT32 Ret = 0;
		struct cmd_msg *msg;
		EXT_CMD_PWR_MGT_BIT_T PwrMgtBitWifi = {0};


		msg = AndesAllocCmdMsg(pAd, sizeof(EXT_CMD_PWR_MGT_BIT_T));

		if (!msg)
		{
			Ret = NDIS_STATUS_RESOURCES;
			goto error;
		}

		PwrMgtBitWifi.ucWlanIdx = ucWlanIdx;
		PwrMgtBitWifi.ucPwrMgtBit = ucPwrMgtBit;

		DBGPRINT(RT_DEBUG_OFF, ("%s:ucWlanIdx(%d), ucPwrMgtBit(%d)\n", __FUNCTION__, ucWlanIdx, ucPwrMgtBit));

		AndesInitCmdMsg(msg, P1_Q0, EXT_CID, CMD_SET, EXT_CMD_ID_PWR_MGT_BIT_WIFI, TRUE, 0,TRUE, TRUE, 8, NULL, CmdExtPmMgtBitRsp);

		AndesAppendCmdMsg(msg, (char *)&PwrMgtBitWifi, sizeof(PwrMgtBitWifi));

		Ret = AndesSendCmdMsg(pAd, msg);

error:
		DBGPRINT(RT_DEBUG_OFF, ("%s:(Ret = %d)\n", __FUNCTION__, Ret));
		return Ret;
	}

}

 /*1: enter, 2: exit specific PM state*/
INT32 CmdExtPmStateCtrl(RTMP_ADAPTER *pAd, UINT8 ucWlanIdx, UINT8 ucPmNumber, UINT8 ucPmState)
{
	struct cmd_msg *msg = NULL;
	EXT_CMD_PM_STATE_CTRL_T CmdPmStateCtrl = {0};
	INT32 Ret = 0;
	msg = AndesAllocCmdMsg(pAd, sizeof(EXT_CMD_PM_STATE_CTRL_T));

	if (!msg)
	{
		Ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	/* Fill parameter here*/
	CmdPmStateCtrl.ucWlanIdx = ucWlanIdx;
	CmdPmStateCtrl.ucPmNumber = ucPmNumber;
	CmdPmStateCtrl.ucPmState = ucPmState;


	AndesInitCmdMsg(msg, P1_Q0, EXT_CID, CMD_SET, EXT_CMD_PM_STATE_CTRL, TRUE, 0,TRUE, TRUE, 8, NULL, CmdExtPmStateCtrlRsp);

	AndesAppendCmdMsg(msg, (char *)&CmdPmStateCtrl, sizeof(CmdPmStateCtrl));

	Ret = AndesSendCmdMsg(pAd, msg);

error:
	DBGPRINT(RT_DEBUG_OFF, ("%s:(Ret = %d)\n", __FUNCTION__, Ret));
	return Ret;
}

INT32 CmdEdcaParameterSet(RTMP_ADAPTER *pAd, CMD_EDCA_SET_T EdcaParam)
{
	struct cmd_msg *msg;
	INT32 ret=0,size=0;

	size = 4+sizeof(TX_AC_PARAM_T)*EdcaParam.ucTotalNum;

	msg = AndesAllocCmdMsg(pAd, size);

	if (!msg)
	{
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	AndesInitCmdMsg(msg, P1_Q0, EXT_CID, CMD_SET, EXT_CMD_ID_EDCA_SET, FALSE, 0, FALSE, FALSE, 0, NULL, NULL);

	AndesAppendCmdMsg(msg, (char *)&EdcaParam,size);


	ret = AndesSendCmdMsg(pAd, msg);


error:
	DBGPRINT(RT_DEBUG_INFO, ("%s:(ret = %d)\n", __FUNCTION__, ret));
	return ret;
}



INT32 CmdSlotTimeSet(RTMP_ADAPTER *pAd, UINT8 SlotTime,UINT8 SifsTime,UINT8 RifsTime,UINT16 EifsTime)
{
	struct cmd_msg *msg;
	INT32 ret=0;

	CMD_SLOT_TIME_SET_T cmdSlotTime;

	NdisZeroMemory(&cmdSlotTime,sizeof(CMD_SLOT_TIME_SET_T));

	msg = AndesAllocCmdMsg(pAd, sizeof(CMD_SLOT_TIME_SET_T));

	if (!msg)
	{
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	AndesInitCmdMsg(msg, P1_Q0, EXT_CID, CMD_SET, EXT_CMD_ID_SLOT_TIME_SET, FALSE, 0, FALSE, FALSE, 0, NULL, NULL);

	cmdSlotTime.u2Eifs = cpu2le16(EifsTime);
	cmdSlotTime.ucRifs = RifsTime;
	cmdSlotTime.ucSifs = SifsTime;
	cmdSlotTime.ucSlotTime = SlotTime;

	AndesAppendCmdMsg(msg, (char *)&cmdSlotTime,sizeof(CMD_SLOT_TIME_SET_T));


	ret = AndesSendCmdMsg(pAd, msg);

error:
	DBGPRINT(RT_DEBUG_INFO, ("%s:(ret = %d)\n", __FUNCTION__, ret));
	return ret;
}


INT32 CmdIdConfigInternalSetting(RTMP_ADAPTER *pAd, UINT8 SubOpCode, UINT8 Param)
{
	struct cmd_msg *msg;
	INT32 ret=0;
	CMD_CONFIG_INTERNAL_SETTING_T CmdConfigInternalSetting;

	NdisZeroMemory(&CmdConfigInternalSetting, sizeof(CMD_CONFIG_INTERNAL_SETTING_T));

	msg = AndesAllocCmdMsg(pAd, sizeof(CMD_CONFIG_INTERNAL_SETTING_T));

	if (!msg)
	{
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	AndesInitCmdMsg(msg, P1_Q0, EXT_CID, CMD_SET, EXT_CMD_ID_CONFIG_INTERNAL_SETTING, FALSE, 0, FALSE, FALSE, 0, NULL, NULL);

	CmdConfigInternalSetting.ucSubOpcode = SubOpCode;
	CmdConfigInternalSetting.aucPara[0] = Param;

	AndesAppendCmdMsg(msg, (char *)&CmdConfigInternalSetting, sizeof(CMD_CONFIG_INTERNAL_SETTING_T));

	ret = AndesSendCmdMsg(pAd, msg);

error:
	DBGPRINT(RT_DEBUG_INFO, ("%s:(ret = %d)\n", __FUNCTION__, ret));
	return ret;
}

#ifdef BCN_OFFLOAD_SUPPORT
VOID RT28xx_UpdateBeaconToMcu(
    IN RTMP_ADAPTER *pAd,
    IN INT apidx,
    IN UCHAR HWBssidIdx,
    IN BOOLEAN Enable,
    IN ULONG FrameLen,
    IN ULONG UpdatePos)
{
    CMD_BCN_UPDATE_T bcn_update;

    NdisZeroMemory(&bcn_update, sizeof(CMD_BCN_UPDATE_T));

    bcn_update.ucHwBssidIdx = HWBssidIdx;
    if (HWBssidIdx > 0) //HWBssid > 0 case, no extendable bssid.
        bcn_update.ucExtBssidIdx = 0;
    else
        bcn_update.ucExtBssidIdx = apidx;

    bcn_update.ucEnable = Enable;
    //bcn_update.ucWlanIdx = 0;//hardcode at present
    bcn_update.u2BcnPeriod = pAd->CommonCfg.BeaconPeriod;

    CmdBcnUpdateSet(pAd, bcn_update);
}
#endif /*BCN_OFFLOAD_SUPPORT*/

#ifdef MT_WOW_SUPPORT
static VOID EventExtCmdPacketFilterRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	P_EXT_EVENT_PF_GENERAL_T pPFRsp = (P_EXT_EVENT_PF_GENERAL_T)Data;
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)msg->priv;

	DBGPRINT(RT_DEBUG_OFF, ("%s: u4PfCmdType = 0x%x u4Status = 0x%x\n",
									__FUNCTION__, le2cpu32(pPFRsp->u4PfCmdType), le2cpu32(pPFRsp->u4Status)));
}

static VOID EventExtCmdWakeupOptionRsp(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	P_EXT_EVENT_WAKEUP_OPTION_T pWakeOptRsp = (P_EXT_EVENT_WAKEUP_OPTION_T)Data;
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)msg->priv;

	DBGPRINT(RT_DEBUG_OFF, ("%s: u4PfCmdType = 0x%x u4Status = 0x%x\n",
									__FUNCTION__, le2cpu32(pWakeOptRsp->u4PfCmdType), le2cpu32(pWakeOptRsp->u4Status)));

}

VOID MT76xxAndesWOWEnable(
	IN PRTMP_ADAPTER pAd)
{
	
	//hw-enable cmd
	//1. magic, parameter=enable
	//2. eapol , param = enable
	//3. bssid , param = bssid[3:0]
	//4. mode, parm = white
	//5. PF, param = enable
	//wakeup command param = choose usb. others dont' care

	struct wifi_dev *wdev = &pAd->StaCfg.wdev;
	MAC_TABLE_ENTRY *pEntry = NULL;

	struct cmd_msg *msg;
	INT32 ret = NDIS_STATUS_SUCCESS;


	//CMD_PACKET_FILTER_MAGIC_PACKET_T CmdMagicPacket;
	CMD_PACKET_FILTER_GLOBAL_T CmdPFGlobal;
	CMD_PACKET_FILTER_GTK_T CmdGTK;
	CMD_PACKET_FILTER_WAKEUP_OPTION_T CmdWakeupOption;
	CMD_PACKET_FILTER_ARPNS_T   CmdARPNS;
	
	DBGPRINT(RT_DEBUG_OFF, ("%s:\n", __FUNCTION__));

	/* security configuration */
	NdisZeroMemory(&CmdGTK, sizeof(CmdGTK));

	msg = AndesAllocCmdMsg(pAd, sizeof(CMD_PACKET_FILTER_GTK_T));
	
	if (!msg)
	{
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}
	
	AndesInitCmdMsg(msg, P1_Q0, EXT_CID, CMD_SET, EXT_CMD_ID_PACKET_FILTER, TRUE, 0,
							TRUE, TRUE, sizeof(EXT_EVENT_PF_GENERAL_T), NULL, EventExtCmdPacketFilterRsp);
	
	CmdGTK.PFType = cpu2le32(_ENUM_TYPE_GTK_REKEY);

	if (wdev->AuthMode == Ndis802_11AuthModeWPAPSK)
		CmdGTK.WPAVersion = cpu2le32(0);
	else
		CmdGTK.WPAVersion = cpu2le32(1);
		
	pEntry = &pAd->MacTab.Content[BSSID_WCID];

#ifdef RT_BIG_ENDIAN
	{
		int index = 0;
		UINT32 *pKey = (UINT32 *)pEntry->PTK;
		for (index=0; index < 64; index += 4)
		{
			*pKey = SWAP32(*pKey);
			pKey++;
		}
	}
#else	
	NdisCopyMemory(CmdGTK.PTK, pEntry->PTK, 64);
#endif

	CmdGTK.BssidIndex = cpu2le32(0);
	CmdGTK.OwnMacIndex = cpu2le32(0x0);
	CmdGTK.WmmIndex = cpu2le32(0);
	
	if (wdev->AuthMode >= Ndis802_11AuthModeWPAPSK){
		NdisCopyMemory(CmdGTK.ReplayCounter, pEntry->R_Counter, LEN_KEY_DESC_REPLAY);
		CmdGTK.GroupKeyIndex = cpu2le32(MCAST_WCID);
		CmdGTK.PairKeyIndex = cpu2le32(pEntry->wcid);
	}
	
	DBGPRINT(RT_DEBUG_OFF, ("%s: BssidIndex %d, GroupKeyIndex %d, OwnMacIndex %d, PairKeyIndex %d, WmmIndex %d\n", 
			__FUNCTION__, CmdGTK.BssidIndex, CmdGTK.GroupKeyIndex, CmdGTK.OwnMacIndex, CmdGTK.PairKeyIndex, CmdGTK.WmmIndex));
	
	AndesAppendCmdMsg(msg, (char *)&CmdGTK, sizeof(CMD_PACKET_FILTER_GTK_T));
	
	ret = AndesSendCmdMsg(pAd, msg);

	// Wakeup option
	NdisZeroMemory(&CmdWakeupOption, sizeof(CMD_PACKET_FILTER_WAKEUP_OPTION_T));

	msg = AndesAllocCmdMsg(pAd, sizeof(CMD_PACKET_FILTER_WAKEUP_OPTION_T));

	if (!msg)
	{
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}
	
	AndesInitCmdMsg(msg, P1_Q0, EXT_CID, CMD_SET, EXT_CMD_ID_WAKEUP_OPTION, TRUE, 0,
							TRUE, TRUE, sizeof(EXT_EVENT_WAKEUP_OPTION_T), NULL, EventExtCmdWakeupOptionRsp);

	CmdWakeupOption.WakeupInterface = cpu2le32(pAd->WOW_Cfg.nWakeupInterface); 
	CmdWakeupOption.GPIONumber = cpu2le32(pAd->WOW_Cfg.nSelectedGPIO);
	CmdWakeupOption.GPIOTimer = cpu2le32(pAd->WOW_Cfg.nHoldTime); // unit is us
	if (pAd->WOW_Cfg.nWakeupInterface == WOW_WAKEUP_BY_GPIO)
		CmdWakeupOption.GpioParameter |= 0x1;
	if (pAd->WOW_Cfg.bGPIOHighLow == WOW_GPIO_LOW_TO_HIGH)
		CmdWakeupOption.GpioParameter |= 0x4;
	else
		CmdWakeupOption.GpioParameter |= 0x2;
	
	DBGPRINT(RT_DEBUG_OFF, ("%s:(WakeupInterface=%d, GPIONumber=%d, GPIOTimer=%d, GpioParameter=0x%x)\n", __FUNCTION__, CmdWakeupOption.WakeupInterface, 
						CmdWakeupOption.GPIONumber, CmdWakeupOption.GPIOTimer, CmdWakeupOption.GpioParameter));

	AndesAppendCmdMsg(msg, (char *)&CmdWakeupOption, sizeof(CMD_PACKET_FILTER_WAKEUP_OPTION_T));
	
	ret = AndesSendCmdMsg(pAd, msg);

	
	// WOW enable
	NdisZeroMemory(&CmdPFGlobal, sizeof(CMD_PACKET_FILTER_GLOBAL_T));

	msg = AndesAllocCmdMsg(pAd, sizeof(CMD_PACKET_FILTER_GLOBAL_T));

	if (!msg)
	{
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}
	
	AndesInitCmdMsg(msg, P1_Q0, EXT_CID, CMD_SET, EXT_CMD_ID_PACKET_FILTER, TRUE, 0,
							TRUE, TRUE, sizeof(EXT_EVENT_PF_GENERAL_T), NULL, EventExtCmdPacketFilterRsp);

	CmdPFGlobal.PFType = cpu2le32(_ENUM_TYPE_GLOBAL_EN);
	CmdPFGlobal.FunctionSelect = cpu2le32(_ENUM_GLOBAL_WOW_EN);
	CmdPFGlobal.Enable = cpu2le32(1); //bit0=1 mean BSS0 for staion mode

	AndesAppendCmdMsg(msg, (char *)&CmdPFGlobal, sizeof(CMD_PACKET_FILTER_GLOBAL_T));
	
	ret = AndesSendCmdMsg(pAd, msg);
	
error:
	DBGPRINT(RT_DEBUG_OFF, ("%s:(ret = %d)\n", __FUNCTION__, ret));

	return;

}

VOID MT76xxAndesWOWDisable(
    IN PRTMP_ADAPTER pAd)
{
	CMD_PACKET_FILTER_GLOBAL_T CmdPFGlobal;

	struct cmd_msg *msg;
	INT32 ret = NDIS_STATUS_SUCCESS;
	
	DBGPRINT(RT_DEBUG_OFF, ("%s:\n", __FUNCTION__));

	// PF disable
	NdisZeroMemory(&CmdPFGlobal, sizeof(CMD_PACKET_FILTER_GLOBAL_T));

	msg = AndesAllocCmdMsg(pAd, sizeof(CMD_PACKET_FILTER_GLOBAL_T));

	if (!msg)
	{
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	AndesInitCmdMsg(msg, P1_Q0, EXT_CID, CMD_SET, EXT_CMD_ID_PACKET_FILTER, TRUE, 0,
							TRUE, TRUE, sizeof(EXT_EVENT_PF_GENERAL_T), NULL, EventExtCmdPacketFilterRsp);

	CmdPFGlobal.PFType = cpu2le32(_ENUM_TYPE_GLOBAL_EN);
	CmdPFGlobal.FunctionSelect = cpu2le32(_ENUM_GLOBAL_WOW_EN);
	CmdPFGlobal.Enable = cpu2le32(0);

	AndesAppendCmdMsg(msg, (char *)&CmdPFGlobal, sizeof(CMD_PACKET_FILTER_GLOBAL_T));

	ret = AndesSendCmdMsg(pAd, msg);

	/* traffic to Host */
	NdisZeroMemory(&CmdPFGlobal, sizeof(CMD_PACKET_FILTER_GLOBAL_T));

	msg = AndesAllocCmdMsg(pAd, sizeof(CMD_PACKET_FILTER_GLOBAL_T));

	if (!msg)
	{
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	AndesInitCmdMsg(msg, P1_Q0, EXT_CID, CMD_SET, EXT_CMD_ID_PACKET_FILTER, TRUE, 0,
							TRUE, TRUE, sizeof(EXT_EVENT_PF_GENERAL_T), NULL, EventExtCmdPacketFilterRsp);

	CmdPFGlobal.PFType = cpu2le32(_ENUM_TYPE_GLOBAL_EN);
	CmdPFGlobal.FunctionSelect = cpu2le32(_ENUM_GLOBAL_ALL_TOMCU);
	CmdPFGlobal.Enable = cpu2le32(0);

	AndesAppendCmdMsg(msg, (char *)&CmdPFGlobal, sizeof(CMD_PACKET_FILTER_GLOBAL_T));

	ret = AndesSendCmdMsg(pAd, msg);


	/* Restore MAC TX/RX */
	//AsicSetMacTxRx(pAd, ASIC_MAC_TXRX, TRUE);


error:
	DBGPRINT(RT_DEBUG_OFF, ("%s:(ret = %d)\n", __FUNCTION__, ret));

	return;


}

VOID MT76xxAndesWOWInit(
	IN PRTMP_ADAPTER pAd)
{
	struct wifi_dev *wdev = &pAd->StaCfg.wdev;
	MAC_TABLE_ENTRY *pEntry = NULL;

	struct cmd_msg *msg;
	INT32 ret = NDIS_STATUS_SUCCESS;

	CMD_PACKET_FILTER_MAGIC_PACKET_T CmdMagicPacket;
	CMD_PACKET_FILTER_GLOBAL_T CmdPFGlobal;
	CMD_PACKET_FILTER_GTK_T CmdGTK;
	CMD_PACKET_FILTER_WAKEUP_OPTION_T CmdWakeupOption;
	
	if (!pAd->WOW_Cfg.bEnable)
		return;
	
	DBGPRINT(RT_DEBUG_OFF, ("%s:\n", __FUNCTION__));
		
		
	if (pAd->WOW_Cfg.nWakeupInterface == WOW_WAKEUP_BY_GPIO){	
			
		// Wakeup option
		NdisZeroMemory(&CmdWakeupOption, sizeof(CMD_PACKET_FILTER_WAKEUP_OPTION_T));

		msg = AndesAllocCmdMsg(pAd, sizeof(CMD_PACKET_FILTER_WAKEUP_OPTION_T));

		if (!msg)
		{
			ret = NDIS_STATUS_RESOURCES;
			goto error;
		}
		
		AndesInitCmdMsg(msg, P1_Q0, EXT_CID, CMD_SET, EXT_CMD_ID_WAKEUP_OPTION, TRUE, 0,
								TRUE, TRUE, sizeof(EXT_EVENT_WAKEUP_OPTION_T), NULL, EventExtCmdWakeupOptionRsp);

		CmdWakeupOption.WakeupInterface = cpu2le32(pAd->WOW_Cfg.nWakeupInterface); 
		CmdWakeupOption.GPIONumber = cpu2le32(pAd->WOW_Cfg.nSelectedGPIO);
		CmdWakeupOption.GPIOTimer = cpu2le32(pAd->WOW_Cfg.nHoldTime); // unit is us
		if (pAd->WOW_Cfg.nWakeupInterface == WOW_WAKEUP_BY_GPIO)
			CmdWakeupOption.GpioParameter |= 0x1;
		if (pAd->WOW_Cfg.bGPIOHighLow == WOW_GPIO_LOW_TO_HIGH)
			CmdWakeupOption.GpioParameter |= 0x4;
		else
			CmdWakeupOption.GpioParameter |= 0x2;
		
		DBGPRINT(RT_DEBUG_OFF, ("%s:(WakeupInterface=%d, GPIONumber=%d, GPIOTimer=%d, GpioParameter=0x%x)\n", __FUNCTION__, CmdWakeupOption.WakeupInterface, 
							CmdWakeupOption.GPIONumber, CmdWakeupOption.GPIOTimer, CmdWakeupOption.GpioParameter));

		AndesAppendCmdMsg(msg, (char *)&CmdWakeupOption, sizeof(CMD_PACKET_FILTER_WAKEUP_OPTION_T));
		
		ret = AndesSendCmdMsg(pAd, msg);
	}
	
error:
	DBGPRINT(RT_DEBUG_OFF, ("%s:(ret = %d)\n", __FUNCTION__, ret));

	return;

}

#endif

INT32 CmdACQueue_Control
( IN struct _RTMP_ADAPTER *ad,
  IN UINT8 ucation,  // 0: stop; 1: flush; 2: start
  IN UINT8 BssidIdx,
  IN UINT32 u4AcQueueMap)
{
	struct cmd_msg *msg;
	EXT_CMD_AC_QUEUE_CONTROL_T  ac_queue_control;
	int ret;

	DBGPRINT(RT_DEBUG_TRACE, ("CmdACQueue_Control  ucation %d BssidIdx %d u4AcQueueMap %d\n",ucation,BssidIdx,u4AcQueueMap));

	msg = AndesAllocCmdMsg(ad, sizeof(EXT_CMD_AC_QUEUE_CONTROL_T));
	if (!msg)
	{
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	AndesInitCmdMsg(msg, P1_Q0, EXT_CID, CMD_SET, EXT_CMD_ID_AC_QUEUE_CONTROL, FALSE, 0,
		FALSE, FALSE, 0, NULL, NULL);

    	NdisZeroMemory(&ac_queue_control, sizeof(EXT_CMD_AC_QUEUE_CONTROL_T));
	ac_queue_control.ucAction = ucation;
	ac_queue_control.ucBssidIdx =BssidIdx;
	ac_queue_control.u4AcQueueMap =cpu2le32(u4AcQueueMap);

	AndesAppendCmdMsg(msg, (char *)&ac_queue_control, sizeof(EXT_CMD_AC_QUEUE_CONTROL_T));
	ret = AndesSendCmdMsg(ad, msg);
error:
	DBGPRINT(RT_DEBUG_OFF, ("%s:(ret = %d)\n", __FUNCTION__, ret));

	return ret;


}



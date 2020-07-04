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
	txs.c
*/

/**
 * @addtogroup tx_rx_path Wi-Fi
 * @{
 * @name TxS Control API
 * @{
 */

#include	"rt_config.h"

/**** TxS Call Back Functions ****/
#ifdef CFG_TDLS_SUPPORT
INT32 TdlsTxSHandler(RTMP_ADAPTER *pAd, CHAR *Data, UINT32 Priv)
{
	MAC_TABLE_ENTRY *pEntry = NULL;
	TXS_STRUC *txs_entry = (TXS_STRUC *)Data;
	TXS_D_0 *TxSD0 = &txs_entry->TxSD0;
	TXS_D_1 *TxSD1 = &txs_entry->TxSD1;
	TXS_D_2 *TxSD2 = &txs_entry->TxSD2;
	TXS_D_3 *TxSD3 = &txs_entry->TxSD3;
	TXS_D_4 *TxSD4 = &txs_entry->TxSD4;

	pEntry = &pAd->MacTab.Content[TxSD3->TxS_WlanIdx];
	MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():txs d0 me : %d\n", __func__, TxSD0->ME));

	if (TxSD0->ME == 0)
		pEntry->TdlsTxFailCount = 0;
	else
		pEntry->TdlsTxFailCount++;

	if (pEntry->TdlsTxFailCount > 15) {
		MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): TdlsTxFailCount > 15!!  teardown link with (%02X:%02X:%02X:%02X:%02X:%02X)!!\n"
				 , __func__, PRINT_MAC(pEntry->Addr)));
		pEntry->TdlsTxFailCount = 0;
		cfg_tdls_auto_teardown(pAd, pEntry);
	}
}
#endif /*CFG_TDLS_SUPPORT*/


INT32 ActionTxSHandler(RTMP_ADAPTER *pAd, CHAR *Data, UINT32 Priv)
{
	/* TODO: shiang-MT7615, fix me! */
	return 0;
}

INT32 BcnTxSHandler(RTMP_ADAPTER *pAd, CHAR *Data, UINT32 Priv)
{
	/* TODO: shiang-MT7615, fix me! */
	return 0;
}

#ifdef STA_LP_PHASE_1_SUPPORT
INT32 NullFramePM1TxSHandler(RTMP_ADAPTER *pAd, CHAR *Data, UINT32 Priv)
{
	return 0;
}

INT32 NullFramePM0TxSHandler(RTMP_ADAPTER *pAd, CHAR *Data, UINT32 Priv)
{
	return 0;
}
#endif /*STA_LP_PHASE_1_SUPPORT */

INT32 PsDataTxSHandler(RTMP_ADAPTER *pAd, CHAR *Data, UINT32 Priv)
{
	/* TODO: shiang-MT7615, fix me! */
	return 0;
}


/**** End of TxS Call Back Functions ****/

#ifdef FTM_SUPPORT
INT32 FtmTXSHandler(RTMP_ADAPTER *pAd, CHAR *Data)
{
	TXS_STRUC *txs_entry = (TXS_STRUC *)Data;
	TXS_D_0 *txs_d0 = &txs_entry->TxSD0;
	PFTM_PEER_ENTRY	pEntry = NULL;
	BOOLEAN TxError = (txs_d0->ME || txs_d0->RE || txs_d0->LE || txs_d0->BE || txs_d0->TxOp || txs_d0->PSBit || txs_d0->BAFail);
	BOOLEAN TimerCancelled;
	MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_WARN,
			 ("   - FTM_TxS PID:0x%02X Err:%d\n", txs_d0->TxS_PId, TxError));
	pEntry = FtmGetPidPendingNode(pAd, txs_d0->TxS_PId);

	if (!pEntry)
		return 0;

	/* Flag clear */
	pEntry->bTxSCallbackCheck = 0;
	RTMPCancelTimer(&pEntry->FtmTxTimer, &TimerCancelled);
	/* Record bTxOK */
	pEntry->bTxOK = !TxError;

	switch (pEntry->State) {
	case FTMPEER_NEGO: {
		if (pEntry->bTxOK)
			FtmEntryNegoDoneAction(pAd, pEntry);
		else
			RTMPSetTimer(&pEntry->FtmTxTimer, 1);	/* Nego should be completed within 10 ms */
	}
	break;

	default: {
		BOOLEAN bTxFTM = TRUE;

		if (pEntry->bTxOK)
			bTxFTM = FtmEntryCntDownAction(pAd, pEntry);

		if (bTxFTM)
			RTMPSetTimer(&pEntry->FtmTxTimer, FtmMinDeltaToMS(pEntry->VerdictParm.min_delta_ftm));
	}
	break;
	}

	return 0;
}
#endif /* FTM_SUPPORT */

INT32 InitTxSTypeTable(RTMP_ADAPTER *pAd)
{
	UINT32 Index, Index1;
	ULONG Flags;
	TXS_CTL *TxSCtl = &pAd->TxSCtl;

	MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s %d\n", __func__, __LINE__));

	/* Per Pkt */
	for (Index = 0; Index < TOTAL_PID_HASH_NUMS; Index++) {
		NdisAllocateSpinLock(pAd, &TxSCtl->TxSTypePerPktLock[Index]);
		RTMP_SPIN_LOCK_IRQSAVE(&TxSCtl->TxSTypePerPktLock[Index], &Flags);
		DlListInit(&TxSCtl->TxSTypePerPkt[Index]);
		RTMP_SPIN_UNLOCK_IRQRESTORE(&TxSCtl->TxSTypePerPktLock[Index], &Flags);
	}

	/* Per Pkt Type */
	for (Index = 0; Index < 3; Index++) {
		for (Index1 = 0; Index1 < TOTAL_PID_HASH_NUMS_PER_PKT_TYPE; Index1++) {
			NdisAllocateSpinLock(pAd, &TxSCtl->TxSTypePerPktTypeLock[Index][Index1]);
			RTMP_SPIN_LOCK_IRQSAVE(&TxSCtl->TxSTypePerPktTypeLock[Index][Index1], &Flags);
			DlListInit(&TxSCtl->TxSTypePerPktType[Index][Index1]);
			RTMP_SPIN_UNLOCK_IRQRESTORE(&TxSCtl->TxSTypePerPktTypeLock[Index][Index1], &Flags);
		}
	}

	for (Index = 0; Index < TXS_STATUS_NUM; Index++)
		NdisZeroMemory(&TxSCtl->TxSStatus[Index], sizeof(TXS_STATUS));

	return 0;
}

/*7636 psm*/
INT32 NullFrameTxSHandler(RTMP_ADAPTER *pAd, CHAR *Data, UINT32 Priv)
{
	return 0;
}


INT32 InitTxSCommonCallBack(RTMP_ADAPTER *pAd)
{
	/* TODO: shiang-MT7615, fix me! */
	return 0;
}


INT32 ExitTxSTypeTable(RTMP_ADAPTER *pAd)
{
	UINT32 Index, Index1;
	ULONG Flags;
	TXS_CTL *TxSCtl = &pAd->TxSCtl;
	TXS_TYPE *TxSType = NULL, *TmpTxSType = NULL;

	for (Index = 0; Index < TOTAL_PID_HASH_NUMS; Index++) {
		RTMP_SPIN_LOCK_IRQSAVE(&TxSCtl->TxSTypePerPktLock[Index], &Flags);
		DlListForEachSafe(TxSType, TmpTxSType, &TxSCtl->TxSTypePerPkt[Index],
						  TXS_TYPE, List) {
			DlListDel(&TxSType->List);
			os_free_mem(TxSType);
		}
		DlListInit(&TxSCtl->TxSTypePerPkt[Index]);
		RTMP_SPIN_UNLOCK_IRQRESTORE(&TxSCtl->TxSTypePerPktLock[Index], &Flags);
		NdisFreeSpinLock(&TxSCtl->TxSTypePerPktLock[Index]);
	}

	for (Index = 0; Index < 3; Index++) {
		for (Index1 = 0; Index1 < TOTAL_PID_HASH_NUMS_PER_PKT_TYPE; Index1++) {
			RTMP_SPIN_LOCK_IRQSAVE(&TxSCtl->TxSTypePerPktTypeLock[Index][Index1], &Flags);
			DlListForEachSafe(TxSType, TmpTxSType, &TxSCtl->TxSTypePerPktType[Index][Index1],
							  TXS_TYPE, List) {
				DlListDel(&TxSType->List);
				os_free_mem(TxSType);
			}
			DlListInit(&TxSCtl->TxSTypePerPktType[Index][Index1]);
			RTMP_SPIN_UNLOCK_IRQRESTORE(&TxSCtl->TxSTypePerPktTypeLock[Index][Index1], &Flags);
			NdisFreeSpinLock(&TxSCtl->TxSTypePerPktTypeLock[Index][Index1]);
		}
	}

	return 0;
}


INT32 AddTxSTypePerPkt(RTMP_ADAPTER *pAd, UINT32 PktPid, UINT8 Format,
					   TXS_HANDLER TxSHandler)
{
	/* TODO: shiang-MT7615, fix me! */
	return 0;
}


INT32 RemoveTxSTypePerPkt(RTMP_ADAPTER *pAd, UINT32 PktPid, UINT8 Format)
{
	ULONG Flags;
	TXS_CTL *TxSCtl = &pAd->TxSCtl;
	TXS_TYPE *TxSType = NULL, *TmpTxSType = NULL;

	RTMP_SPIN_LOCK_IRQSAVE(&TxSCtl->TxSTypePerPktLock[PktPid % TOTAL_PID_HASH_NUMS], &Flags);
	DlListForEachSafe(TxSType, TmpTxSType, &TxSCtl->TxSTypePerPkt[PktPid % TOTAL_PID_HASH_NUMS],
					  TXS_TYPE, List) {
		if ((TxSType->PktPid == PktPid) && (TxSType->Format == Format)) {
			DlListDel(&TxSType->List);
			os_free_mem(TxSType);
			RTMP_SPIN_UNLOCK_IRQRESTORE(&TxSCtl->TxSTypePerPktLock[PktPid % TOTAL_PID_HASH_NUMS],
										&Flags);
			return 0;
		}
	}
	RTMP_SPIN_UNLOCK_IRQRESTORE(&TxSCtl->TxSTypePerPktLock[PktPid % TOTAL_PID_HASH_NUMS], &Flags);
	return -1;
}


INT32 TxSTypeCtlPerPkt(RTMP_ADAPTER *pAd, UINT32 PktPid, UINT8 Format, BOOLEAN TxS2Mcu,
					   BOOLEAN TxS2Host, BOOLEAN DumpTxSReport, ULONG DumpTxSReportTimes)
{
	ULONG Flags;
	TXS_CTL *TxSCtl = &pAd->TxSCtl;
	TXS_TYPE *TxSType = NULL;

	RTMP_SPIN_LOCK_IRQSAVE(&TxSCtl->TxSTypePerPktLock[PktPid % TOTAL_PID_HASH_NUMS], &Flags);
	DlListForEach(TxSType, &TxSCtl->TxSTypePerPkt[PktPid % TOTAL_PID_HASH_NUMS], TXS_TYPE, List) {
		if ((TxSType->PktPid == PktPid) && (TxSType->Format == Format)) {
			if (TxS2Mcu)
				TxSCtl->TxS2McUStatusPerPkt |= (1 << PktPid);
			else
				TxSCtl->TxS2McUStatusPerPkt &= ~(1 << PktPid);

			if (TxS2Host)
				TxSCtl->TxS2HostStatusPerPkt |= (1 << PktPid);
			else
				TxSCtl->TxS2HostStatusPerPkt &= ~(1 << PktPid);

			if (Format == TXS_FORMAT1)
				TxSCtl->TxSFormatPerPkt |= (1 << PktPid);
			else
				TxSCtl->TxSFormatPerPkt &= ~(1 << PktPid);

			TxSType->DumpTxSReport = DumpTxSReport;
			TxSType->DumpTxSReportTimes = DumpTxSReportTimes;
			RTMP_SPIN_UNLOCK_IRQRESTORE(&TxSCtl->TxSTypePerPktLock[PktPid % TOTAL_PID_HASH_NUMS],
										&Flags);
			return 0;
		}
	}
	RTMP_SPIN_UNLOCK_IRQRESTORE(&TxSCtl->TxSTypePerPktLock[PktPid % TOTAL_PID_HASH_NUMS], &Flags);
	MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s: can not find TxSType(PktPID = %d, Format = %d)\n",
			  __func__, PktPid, Format));
	return -1;
}


INT32 AddTxSTypePerPktType(RTMP_ADAPTER *pAd, UINT8 PktType, UINT8 PktSubType,
						   UINT8 Format, TXS_HANDLER TxSHandler)
{
	ULONG Flags;
	TXS_CTL *TxSCtl = &pAd->TxSCtl;
	TXS_TYPE *TxSType = NULL, *SearchTxSType = NULL;

	os_alloc_mem(NULL, (PUCHAR *)&TxSType, sizeof(*TxSType));

	if (!TxSType) {
		MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("can not allocate TxS Type\n"));
		return -1;
	}

	RTMP_SPIN_LOCK_IRQSAVE(&TxSCtl->TxSTypePerPktTypeLock[PktType][PktSubType % TOTAL_PID_HASH_NUMS_PER_PKT_TYPE], &Flags);
	DlListForEach(SearchTxSType, &TxSCtl->TxSTypePerPktType[PktType][PktSubType % TOTAL_PID_HASH_NUMS_PER_PKT_TYPE], TXS_TYPE, List) {
		if ((SearchTxSType->PktType == PktType) && (SearchTxSType->PktSubType == PktSubType)
			&& (SearchTxSType->Format == Format)) {
			MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: already registered TxSType (PktType = %d, PktSubType = %d, Format = %d\n", __func__, PktType, PktSubType, Format));
			RTMP_SPIN_UNLOCK_IRQRESTORE(&TxSCtl->TxSTypePerPktTypeLock[PktType][PktSubType % TOTAL_PID_HASH_NUMS_PER_PKT_TYPE], &Flags);
			os_free_mem(TxSType);
			return -1;
		}
	}
	TxSType->Type = TXS_TYPE1;
	TxSType->PktType = PktType;
	TxSType->PktSubType = PktSubType;
	TxSType->Format = Format;
	TxSType->TxSHandler = TxSHandler;
	DlListAddTail(&TxSCtl->TxSTypePerPktType[PktType][PktSubType % TOTAL_PID_HASH_NUMS_PER_PKT_TYPE], &TxSType->List);
	RTMP_SPIN_UNLOCK_IRQRESTORE(&TxSCtl->TxSTypePerPktTypeLock[PktType][PktSubType % TOTAL_PID_HASH_NUMS_PER_PKT_TYPE], &Flags);
	return 0;
}


INT32 RemoveTxSTypePerPktType(RTMP_ADAPTER *pAd, UINT8 PktType, UINT8 PktSubType,
							  UINT8 Format)
{
	ULONG Flags;
	TXS_CTL *TxSCtl = &pAd->TxSCtl;
	TXS_TYPE *TxSType = NULL, *TmpTxSType = NULL;

	RTMP_SPIN_LOCK_IRQSAVE(&TxSCtl->TxSTypePerPktTypeLock[PktType][PktSubType % TOTAL_PID_HASH_NUMS_PER_PKT_TYPE], &Flags);
	DlListForEachSafe(TxSType, TmpTxSType, &TxSCtl->TxSTypePerPktType[PktType][PktSubType % TOTAL_PID_HASH_NUMS_PER_PKT_TYPE], TXS_TYPE, List) {
		if ((TxSType->PktType == PktType) && (TxSType->PktSubType == PktSubType)
			&& (TxSType->Format == Format)) {
			DlListDel(&TxSType->List);
			os_free_mem(TxSType);
			RTMP_SPIN_UNLOCK_IRQRESTORE(&TxSCtl->TxSTypePerPktTypeLock[PktType][PktSubType % TOTAL_PID_HASH_NUMS_PER_PKT_TYPE], &Flags);
			return 0;
		}
	}
	RTMP_SPIN_UNLOCK_IRQRESTORE(&TxSCtl->TxSTypePerPktTypeLock[PktType][PktSubType % TOTAL_PID_HASH_NUMS_PER_PKT_TYPE], &Flags);
	return -1;
}


INT32 TxSTypeCtlPerPktType(RTMP_ADAPTER *pAd, UINT8 PktType, UINT8 PktSubType, UINT16 WlanIdx,
						   UINT8 Format, BOOLEAN TxS2Mcu, BOOLEAN TxS2Host, BOOLEAN DumpTxSReport,
						   ULONG DumpTxSReportTimes)
{
	ULONG Flags;
	TXS_CTL *TxSCtl = &pAd->TxSCtl;
	TXS_TYPE *TxSType = NULL;

	RTMP_SPIN_LOCK_IRQSAVE(&TxSCtl->TxSTypePerPktTypeLock[PktType][PktSubType % TOTAL_PID_HASH_NUMS_PER_PKT_TYPE], &Flags);
	DlListForEach(TxSType, &TxSCtl->TxSTypePerPktType[PktType][PktSubType % TOTAL_PID_HASH_NUMS_PER_PKT_TYPE], TXS_TYPE, List) {
		if ((TxSType->PktType == PktType) && (TxSType->PktSubType == PktSubType)
			&& (TxSType->Format == Format)) {
			/*register the TYPE/SUB_TYPE need to report to MCU.*/
			if (TxS2Mcu)
				TxSCtl->TxS2McUStatusPerPktType[PktType] |= (1 << PktSubType);
			else
				TxSCtl->TxS2McUStatusPerPktType[PktType] &= ~(1 << PktSubType);

			/*register the TYPE/SUB_TYPE need to report to HOST.*/
			if (TxS2Host)
				TxSCtl->TxS2HostStatusPerPktType[PktType] |= (1 << PktSubType);
			else
				TxSCtl->TxS2HostStatusPerPktType[PktType] &= ~(1 << PktSubType);

			/*register the TXS report type*/
			if (Format == TXS_FORMAT1)
				TxSCtl->TxSFormatPerPktType[PktType] |= (1 << PktSubType);
			else
				TxSCtl->TxSFormatPerPktType[PktType] &= ~(1 << PktSubType);

			/*indicate which widx might be used for send the kinw of type/subtype pkt.*/
			if (WlanIdx < 64)
				TxSCtl->TxSStatusPerWlanIdx[0] |= (1 << (UINT64)WlanIdx);
			else if (WlanIdx >= 64 && WlanIdx < 128)
				TxSCtl->TxSStatusPerWlanIdx[1] |= (1 << (UINT64)WlanIdx);
			else {
				TxSCtl->TxSStatusPerWlanIdx[0] = 0xffffffffffffffff;
				TxSCtl->TxSStatusPerWlanIdx[1] = 0xffffffffffffffff;
			}

			TxSType->DumpTxSReport = DumpTxSReport;
			TxSType->DumpTxSReportTimes = DumpTxSReportTimes;
			RTMP_SPIN_UNLOCK_IRQRESTORE(&TxSCtl->TxSTypePerPktTypeLock[PktType][PktSubType % TOTAL_PID_HASH_NUMS_PER_PKT_TYPE], &Flags);
			return 0;
		}
	}
	RTMP_SPIN_UNLOCK_IRQRESTORE(&TxSCtl->TxSTypePerPktTypeLock[PktType][PktSubType % TOTAL_PID_HASH_NUMS_PER_PKT_TYPE], &Flags);
	MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s: can not find TxSType(PktType = %d, PktSubType = %d, Format = %d)\n",
			  __func__, PktType, PktSubType, Format));
	return -1;
}


INT32 ParseTxSPacket_v2(RTMP_ADAPTER *pAd, UINT32 Pid, UINT8 Format, CHAR *Data)
{
	TXS_STRUC *txs_entry = (TXS_STRUC *)Data;
	TXS_D_0 *TxSD0 = &txs_entry->TxSD0;

	if (Format == TXS_FORMAT0) {
#ifdef FTM_SUPPORT

		if ((Pid >= PID_FTM_MIN) && (Pid <= PID_FTM_MAX))
			FtmTXSHandler(pAd, Data);

#endif /* FTM_SUPPORT */

		if (TxSD0->ME || TxSD0->RE || TxSD0->LE || TxSD0->BE || TxSD0->TxOp || TxSD0->PSBit || TxSD0->BAFail) {
			DumpTxSFormat(pAd, Format, Data);
			return -1;
		}
	}

	return 0;
}


UINT8 AddTxSStatus(RTMP_ADAPTER *pAd, UINT8 Type, UINT8 PktPid, UINT8 PktType,
				   UINT8 PktSubType, UINT16 TxRate, UINT32 Priv)
{
	TXS_CTL *TxSCtl = &pAd->TxSCtl;
	INT idx;

	for (idx = 0; idx < TXS_STATUS_NUM; idx++) {
		if (TxSCtl->TxSStatus[idx].State == TXS_UNUSED) {
			TxSCtl->TxSPid = idx;
			TxSCtl->TxSStatus[idx].TxSPid = TxSCtl->TxSPid;
			TxSCtl->TxSStatus[idx].State = TXS_USED;
			TxSCtl->TxSStatus[idx].Type = Type;
			TxSCtl->TxSStatus[idx].PktPid = PktPid;
			TxSCtl->TxSStatus[idx].PktType = PktType;
			TxSCtl->TxSStatus[idx].PktSubType = PktSubType;
			TxSCtl->TxSStatus[idx].TxRate = TxRate;
			TxSCtl->TxSStatus[idx].Priv = Priv;
			break;
		}
	}

	if (idx >= TXS_STATUS_NUM) {
		TxSCtl->TxSFailCount++;
		idx = TXS_STATUS_NUM - 1;
		MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("%s():Cannot get empty TxSPid, use default(%d)\n",
				  __func__, idx));
	}

	return idx;
}


INT32 RemoveTxSStatus(RTMP_ADAPTER *pAd, UINT8 TxSPid, UINT8 *Type, UINT8 *PktPid,
					  UINT8 *PktType, UINT8 *PktSubType, UINT16 *TxRate, UINT32 *TxSPriv)
{
	TXS_CTL *TxSCtl = &pAd->TxSCtl;
	*Type = TxSCtl->TxSStatus[TxSPid].Type;
	*PktPid = TxSCtl->TxSStatus[TxSPid].PktPid;
	*PktType = TxSCtl->TxSStatus[TxSPid].PktType;
	*PktSubType = TxSCtl->TxSStatus[TxSPid].PktSubType;
	*TxRate = TxSCtl->TxSStatus[TxSPid].TxRate;
	*TxSPriv = TxSCtl->TxSStatus[TxSPid].Priv;
	TxSCtl->TxSStatus[TxSPid].State = TXS_UNUSED;
	return 0;
}

/** @} */
/** @} */

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

#include "rt_config.h"


VOID dump_rxinfo(RTMP_ADAPTER *pAd, RXINFO_STRUC *pRxInfo)
{
	hex_dump("RxInfo Raw Data", (UCHAR *)pRxInfo, sizeof(RXINFO_STRUC));

	DBGPRINT(RT_DEBUG_OFF, ("RxInfo Fields:\n"));

	DBGPRINT(RT_DEBUG_OFF, ("\tBA=%d\n", pRxInfo->BA));
	DBGPRINT(RT_DEBUG_OFF, ("\tDATA=%d\n", pRxInfo->DATA));
	DBGPRINT(RT_DEBUG_OFF, ("\tNULLDATA=%d\n", pRxInfo->NULLDATA));
	DBGPRINT(RT_DEBUG_OFF, ("\tFRAG=%d\n", pRxInfo->FRAG));
	DBGPRINT(RT_DEBUG_OFF, ("\tU2M=%d\n", pRxInfo->U2M));
	DBGPRINT(RT_DEBUG_OFF, ("\tMcast=%d\n", pRxInfo->Mcast));
	DBGPRINT(RT_DEBUG_OFF, ("\tBcast=%d\n", pRxInfo->Bcast));
	DBGPRINT(RT_DEBUG_OFF, ("\tMyBss=%d\n", pRxInfo->MyBss));
	DBGPRINT(RT_DEBUG_OFF, ("\tCrc=%d\n", pRxInfo->Crc));
	DBGPRINT(RT_DEBUG_OFF, ("\tCipherErr=%d\n", pRxInfo->CipherErr));
	DBGPRINT(RT_DEBUG_OFF, ("\tAMSDU=%d\n", pRxInfo->AMSDU));
	DBGPRINT(RT_DEBUG_OFF, ("\tHTC=%d\n", pRxInfo->HTC));
	DBGPRINT(RT_DEBUG_OFF, ("\tRSSI=%d\n", pRxInfo->RSSI));
	DBGPRINT(RT_DEBUG_OFF, ("\tL2PAD=%d\n", pRxInfo->L2PAD));
	DBGPRINT(RT_DEBUG_OFF, ("\tAMPDU=%d\n", pRxInfo->AMPDU));
	DBGPRINT(RT_DEBUG_OFF, ("\tDecrypted=%d\n", pRxInfo->Decrypted));
	DBGPRINT(RT_DEBUG_OFF, ("\tBssIdx3=%d\n", pRxInfo->BssIdx3));
	DBGPRINT(RT_DEBUG_OFF, ("\twapi_kidx=%d\n", pRxInfo->wapi_kidx));
	DBGPRINT(RT_DEBUG_OFF, ("\tpn_len=%d\n", pRxInfo->pn_len));
	DBGPRINT(RT_DEBUG_OFF, ("\tsw_fc_type0=%d\n", pRxInfo->sw_fc_type0));
	DBGPRINT(RT_DEBUG_OFF, ("\tsw_fc_type1=%d\n", pRxInfo->sw_fc_type1));
}


VOID dump_txinfo(RTMP_ADAPTER *pAd, TXINFO_STRUC *pTxInfo)
{
	hex_dump("TxInfo Raw Data: ", (UCHAR *)pTxInfo, sizeof(TXINFO_STRUC));
	
	DBGPRINT(RT_DEBUG_OFF, ("TxInfo Fields:\n"));

#ifdef RLT_MAC
	if (pAd->chipCap.hif_type == HIF_RLT)
		dump_rlt_txinfo(pAd, pTxInfo);
#endif /* RLT_MAC */

#ifdef RTMP_MAC
	if (pAd->chipCap.hif_type == HIF_RTMP)
		DBGPRINT(RT_DEBUG_OFF, ("\n"));
#endif /* RTMP_MAC */
}


VOID dump_tmac_info(RTMP_ADAPTER *pAd, UCHAR *tmac_info)
{
	TXWI_STRUC *pTxWI = (TXWI_STRUC *)tmac_info;
	hex_dump("TxWI Raw Data: ", (UCHAR *)pTxWI, pAd->chipCap.TXWISize);

	DBGPRINT(RT_DEBUG_OFF, ("TxWI Fields:\n"));
#ifdef RLT_MAC
	if (pAd->chipCap.hif_type == HIF_RLT)
		dump_rlt_txwi(pAd, pTxWI);
#endif /* RLT_MAC */

#ifdef RTMP_MAC
	if (pAd->chipCap.hif_type == HIF_RTMP)
		dump_rtmp_txwi(pAd, pTxWI);
#endif /* RTMP_MAC */
}


VOID dump_rmac_info(RTMP_ADAPTER *pAd, UCHAR *rmac_info)
{
	RXWI_STRUC *pRxWI = (RXWI_STRUC *)rmac_info;

	hex_dump("RxWI Raw Data", (UCHAR *)pRxWI, pAd->chipCap.RXWISize);

	DBGPRINT(RT_DEBUG_OFF, ("RxWI Fields:\n"));
#ifdef RLT_MAC
	if (pAd->chipCap.hif_type == HIF_RLT)
		dump_rlt_rxwi(pAd, pRxWI);	
#endif /* RLT_MAC */

#ifdef RTMP_MAC
	if (pAd->chipCap.hif_type == HIF_RTMP)
		dump_rtmp_rxwi(pAd, pRxWI);
#endif /* RTMP_MAC */
}


#ifdef CONFIG_AP_SUPPORT
#ifdef RTMP_MAC_PCI
/*
	========================================================================
	
	Routine Description:
		In the case, Client may be silent left without sending DeAuth or DeAssoc.
		AP'll continue retry packets for the client since AP doesn't know the STA
		is gone. To Minimum affection of exist traffic is disable retransmition for
		all those packet relative to the STA.
		So decide to change ack required setting of all packet in TX ring
		to "no ACK" requirement for specific Client.

	Arguments:
		Adapter						Pointer to our adapter

	Return Value:
		None

	IRQL = DISPATCH_LEVEL
	
	========================================================================
*/
static VOID ClearTxRingClientAck(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry)
{
	UINT8 TXWISize;

	if (!pAd || !pEntry)
		return;

	TXWISize = pAd->chipCap.TXWISize;

#ifdef RLT_MAC
	if (pAd->chipCap.hif_type == HIF_RLT) {
		DBGPRINT(RT_DEBUG_OFF, ("%s(): TBD for this function!\n", __FUNCTION__));
	}
#endif /* RLT_MAC */

#ifdef RTMP_MAC
	if (pAd->chipCap.hif_type == HIF_RTMP) {
		INT index;
		USHORT TxIdx;
		RTMP_TX_RING *pTxRing;
		TXD_STRUC *pTxD;
		TXWI_STRUC *pTxWI;
#ifdef RT_BIG_ENDIAN
		TXD_STRUC TxD, *pDestTxD;
		TXWI_STRUC TxWI, *pDestTxWI;
#endif /* RT_BIG_ENDIAN */

		for (index = 3; index >= 0; index--)
		{
			pTxRing = &pAd->TxRing[index];
			for (TxIdx = 0; TxIdx < TX_RING_SIZE; TxIdx++)
			{
#ifdef RT_BIG_ENDIAN
				pDestTxD = (TXD_STRUC *) pTxRing->Cell[TxIdx].AllocVa;
				TxD = *pDestTxD;
				pTxD = &TxD;
				RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#else
				pTxD = (TXD_STRUC *) pTxRing->Cell[TxIdx].AllocVa;
#endif /* RT_BIG_ENDIAN */

				if (!pTxD->DMADONE)
				{
#ifdef RT_BIG_ENDIAN
					pDestTxWI = (TXWI_STRUC *) pTxRing->Cell[TxIdx].DmaBuf.AllocVa;
					NdisMoveMemory((PUCHAR)&TxWI, (PUCHAR)pDestTxWI, TXWISize);
					pTxWI = &TxWI;
					RTMPWIEndianChange(pAd, (PUCHAR)pTxWI, TYPE_TXWI);
#else
					 pTxWI = (TXWI_STRUC *)pTxRing->Cell[TxIdx].DmaBuf.AllocVa;
#endif /* RT_BIG_ENDIAN */

					if (pTxWI->TXWI_O.wcid == pEntry->wcid)
						pTxWI->TXWI_O.ACK = FALSE;

#ifdef RT_BIG_ENDIAN
					RTMPWIEndianChange(pAd, (PUCHAR)pTxWI, TYPE_TXWI);
					NdisMoveMemory((PUCHAR)pDestTxWI, (PUCHAR)pTxWI, TXWISize);
#endif /* RT_BIG_ENDIAN */
				}
			}
		}
	}
#endif /* RTMP_MAC */
}
#endif /* RTMP_MAC_PCI */
#endif /* CONFIG_AP_SUPPORT */


INT rtmp_mac_fifo_stat_update(RTMP_ADAPTER *pAd)
{
	TX_STA_FIFO_STRUC	StaFifo;
	TX_STA_FIFO_EXT_STRUC StaFifoExt;
	MAC_TABLE_ENTRY		*pEntry = NULL;
	UINT32				i = 0;
	UCHAR				pid = 0, wcid = 0;
	INT32				reTry;
	UCHAR				succMCS;



	do
	{
#ifdef FIFO_EXT_SUPPORT
		if (IS_RT65XX(pAd)) {
			RTMP_IO_READ32(pAd, TX_STA_FIFO_EXT, &StaFifoExt.word);
		}
#endif /* FIFO_EXT_SUPPORT */
		RTMP_IO_READ32(pAd, TX_STA_FIFO, &StaFifo.word);

		if (StaFifo.field.bValid == 0)
			break;
	
		wcid = (UCHAR)StaFifo.field.wcid;

#ifdef DBG_CTRL_SUPPORT
#ifdef INCLUDE_DEBUG_QUEUE
	if (pAd->CommonCfg.DebugFlags & DBF_DBQ_TXFIFO) {
		dbQueueEnqueue(0x73, (UCHAR *)(&StaFifo.word));
	}
#endif /* INCLUDE_DEBUG_QUEUE */
#endif /* DBG_CTRL_SUPPORT */

	/* ignore NoACK and MGMT frame use 0xFF as WCID */
		if ((StaFifo.field.TxAckRequired == 0) || (wcid >= MAX_LEN_OF_MAC_TABLE))
		{
			i++;
			continue;
		}

		/* PID store Tx MCS Rate */
#ifdef FIFO_EXT_SUPPORT
		if (IS_RT65XX(pAd))
			pid = (UCHAR)StaFifoExt.field.pkt_id_65xx;
		else
#endif /* FIFO_EXT_SUPPORT */
			pid = (UCHAR)StaFifo.field.PidType;

		pEntry = &pAd->MacTab.Content[wcid];

		if (pEntry & pAd->MacTab.tr_entry[wcid].PsDeQWaitCnt)
			pAd->MacTab.tr_entry[wcid].PsDeQWaitCnt = 0;

		pEntry->DebugFIFOCount++;


#ifdef DOT11_N_SUPPORT
#endif /* DOT11_N_SUPPORT */

#ifdef UAPSD_SUPPORT
		UAPSD_SP_AUE_Handle(pAd, pEntry, StaFifo.field.TxSuccess);
#endif /* UAPSD_SUPPORT */


		if (!StaFifo.field.TxSuccess)
		{
			pEntry->FIFOCount++;
			pEntry->OneSecTxFailCount++;
								
			if (pEntry->FIFOCount >= 1)
			{			
				DBGPRINT(RT_DEBUG_TRACE, ("#"));
#ifdef DOT11_N_SUPPORT
				pEntry->NoBADataCountDown = 64;
#endif /* DOT11_N_SUPPORT */


				/* Update the continuous transmission counter.*/
				pEntry->ContinueTxFailCnt++;
				// TODO: shiang-usw, remove upper setting because we need to mirgate to tr_entry!
				pAd->MacTab.tr_entry[pEntry->wcid].ContinueTxFailCnt++;

				if(pEntry->PsMode == PWR_ACTIVE)
				{
#ifdef DOT11_N_SUPPORT					
					int tid;
					for (tid=0; tid<NUM_OF_TID; tid++)
						BAOriSessionTearDown(pAd, pEntry->wcid,  tid, FALSE, FALSE);
#endif /* DOT11_N_SUPPORT */

#ifdef WDS_SUPPORT
					/* fix WDS Jam issue*/
					if(IS_ENTRY_WDS(pEntry)
						&& (pEntry->LockEntryTx == FALSE)
						&& (pEntry->ContinueTxFailCnt >= pAd->ApCfg.EntryLifeCheck))
					{ 
						DBGPRINT(RT_DEBUG_TRACE, ("Entry %02x:%02x:%02x:%02x:%02x:%02x Blocked!! (Fail Cnt = %d)\n",
							PRINT_MAC(pEntry->Addr), pEntry->ContinueTxFailCnt ));

						pEntry->LockEntryTx = TRUE;
						// TODO: shiang-usw, remove upper setting because we need to mirgate to tr_entry!
						pAd->MacTab.tr_entry[pEntry->wcid].LockEntryTx = TRUE;

					}
#endif /* WDS_SUPPORT */
				}
			}
#ifdef CONFIG_AP_SUPPORT
#ifdef RTMP_MAC_PCI
		/* if Tx fail >= 20, then clear TXWI ack in Tx Ring*/
		if (pEntry->ContinueTxFailCnt >= pAd->ApCfg.EntryLifeCheck)
			ClearTxRingClientAck(pAd, pEntry);	
#endif /* RTMP_MAC_PCI */				
#endif /* CONFIG_AP_SUPPORT */
		}
		else
		{
#ifdef DOT11_N_SUPPORT
			if ((pEntry->PsMode != PWR_SAVE) && (pEntry->NoBADataCountDown > 0))
			{
				pEntry->NoBADataCountDown--;
				if (pEntry->NoBADataCountDown==0)
				{
					DBGPRINT(RT_DEBUG_TRACE, ("@\n"));
				}
			}
#endif /* DOT11_N_SUPPORT */
			pEntry->FIFOCount = 0;
			pEntry->OneSecTxNoRetryOkCount++;


			/* update NoDataIdleCount when sucessful send packet to STA.*/
			pEntry->NoDataIdleCount = 0;
			pEntry->ContinueTxFailCnt = 0;
#ifdef WDS_SUPPORT
			pEntry->LockEntryTx = FALSE;
#endif /* WDS_SUPPORT */
			// TODO: shiang-usw, remove upper setting because we need to mirgate to tr_entry!
			pAd->MacTab.tr_entry[pEntry->wcid].NoDataIdleCount = 0;
			pAd->MacTab.tr_entry[pEntry->wcid].ContinueTxFailCnt = 0;
			pAd->MacTab.tr_entry[pEntry->wcid].LockEntryTx = FALSE;

		}

		succMCS = StaFifo.field.SuccessRate & 0x7F;
#ifdef DOT11N_SS3_SUPPORT
		if (pEntry->HTCapability.MCSSet[2] == 0xff)
		{
			if (succMCS > pid)
				pid = pid + 16;
		}
#endif /* DOT11N_SS3_SUPPORT */

		if (StaFifo.field.TxSuccess)
		{
			pEntry->TXMCSExpected[pid]++;
			if (pid == succMCS)
				pEntry->TXMCSSuccessful[pid]++;
			else 
				pEntry->TXMCSAutoFallBack[pid][succMCS]++;
		}
		else
		{
			pEntry->TXMCSFailed[pid]++;
		}

#ifdef DOT11N_SS3_SUPPORT
		if (pid >= 16 && succMCS <= 8)
			succMCS += (2 - (succMCS >> 3)) * 7;
#endif /* DOT11N_SS3_SUPPORT */

		reTry = pid - succMCS;

		if (reTry > 0)
		{
			/* MCS8 falls back to 0 */
			if (pid>=8 && succMCS==0)
				reTry -= 7;
			else if ((pid >= 12) && succMCS <=7)
				reTry -= 4;

			pEntry->OneSecTxRetryOkCount += reTry;
		}

		i++;	/* ASIC store 16 stack*/
	} while ( i < (TX_RING_SIZE<<1) );

}


/*
	========================================================================
	
	Routine Description:
		Read Tx statistic raw counters from hardware registers and record to
		related software variables for later on query

	Arguments:
		pAd					Pointer to our adapter
		pStaTxCnt0			Pointer to record "TX_STA_CNT0" (0x170c)
		pStaTxCnt1			Pointer to record "TX_STA_CNT1" (0x1710)

	Return Value:
		None

	========================================================================
*/
VOID NicGetTxRawCounters(
	IN RTMP_ADAPTER *pAd,
	IN TX_STA_CNT0_STRUC *pStaTxCnt0,
	IN TX_STA_CNT1_STRUC *pStaTxCnt1)
{
	// TODO: shiang-7603
	if (pAd->chipCap.hif_type == HIF_MT) {
		DBGPRINT(RT_DEBUG_OFF, ("%s(%d): Not support for HIF_MT yet!\n",
							__FUNCTION__, __LINE__));
		return;
	}


	// TODO: shiang-7603, fix this!
	if (pAd->chipCap.hif_type != HIF_MT) {
		RTMP_IO_READ32(pAd, TX_STA_CNT0, &pStaTxCnt0->word);
		RTMP_IO_READ32(pAd, TX_STA_CNT1, &pStaTxCnt1->word);
	}
	pAd->bUpdateBcnCntDone = TRUE;	/* not appear in Rory's code */
	pAd->RalinkCounters.OneSecBeaconSentCnt += pStaTxCnt0->field.TxBeaconCount;
	pAd->RalinkCounters.OneSecTxRetryOkCount += pStaTxCnt1->field.TxRetransmit;
	pAd->RalinkCounters.OneSecTxNoRetryOkCount += pStaTxCnt1->field.TxSuccess;
	pAd->RalinkCounters.OneSecTxFailCount += pStaTxCnt0->field.TxFailCount;

#ifdef STATS_COUNT_SUPPORT
	pAd->WlanCounters.TransmittedFragmentCount.u.LowPart += pStaTxCnt1->field.TxSuccess;
	pAd->WlanCounters.RetryCount.u.LowPart += pStaTxCnt1->field.TxRetransmit;
	pAd->WlanCounters.FailedCount.u.LowPart += pStaTxCnt0->field.TxFailCount;
#endif /* STATS_COUNT_SUPPORT */

}


/*
	========================================================================
	
	Routine Description:
		Read statistical counters from hardware registers and record them
		in software variables for later on query

	Arguments:
		pAd					Pointer to our adapter

	Return Value:
		None

	IRQL = DISPATCH_LEVEL
	
	========================================================================
*/
VOID NICUpdateRawCounters(RTMP_ADAPTER *pAd)
{
	UINT32	OldValue;/*, Value2;*/
	/*ULONG	PageSum, OneSecTransmitCount;*/
	/*ULONG	TxErrorRatio, Retry, Fail;*/
	RX_STA_CNT0_STRUC	 RxStaCnt0;
	RX_STA_CNT1_STRUC   RxStaCnt1;
	RX_STA_CNT2_STRUC   RxStaCnt2;
	TX_STA_CNT0_STRUC 	 TxStaCnt0;
	TX_STA_CNT1_STRUC	 StaTx1;
	TX_STA_CNT2_STRUC	 StaTx2;
#ifdef STATS_COUNT_SUPPORT
	TX_NAG_AGG_CNT_STRUC	TxAggCnt;
	TX_AGG_CNT0_STRUC	TxAggCnt0;
	TX_AGG_CNT1_STRUC	TxAggCnt1;
	TX_AGG_CNT2_STRUC	TxAggCnt2;
	TX_AGG_CNT3_STRUC	TxAggCnt3;
	TX_AGG_CNT4_STRUC	TxAggCnt4;
	TX_AGG_CNT5_STRUC	TxAggCnt5;
	TX_AGG_CNT6_STRUC	TxAggCnt6;
	TX_AGG_CNT7_STRUC	TxAggCnt7;
#endif /* STATS_COUNT_SUPPORT */
	COUNTER_RALINK		*pRalinkCounters;


	// TODO: shiang-7603
	if (pAd->chipCap.hif_type == HIF_MT) {
		DBGPRINT(RT_DEBUG_OFF, ("%s(%d): Not support for HIF_MT yet!\n",
							__FUNCTION__, __LINE__));
		return;
	}


	pRalinkCounters = &pAd->RalinkCounters;


	// TODO: shiang-7603, fix this!
	if (pAd->chipCap.hif_type != HIF_MT) {
		RTMP_IO_READ32(pAd, RX_STA_CNT0, &RxStaCnt0.word);
		RTMP_IO_READ32(pAd, RX_STA_CNT2, &RxStaCnt2.word);
	}
	
	pAd->RalinkCounters.PhyErrCnt += RxStaCnt0.field.PhyErr;
#ifdef CONFIG_AP_SUPPORT
#ifdef CARRIER_DETECTION_SUPPORT
	if ((pAd->CommonCfg.CarrierDetect.Enable == FALSE) || (pAd->OpMode == OPMODE_STA))
#endif /* CARRIER_DETECTION_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
	{
		// TODO: shiang-7603, fix this!
		if (pAd->chipCap.hif_type != HIF_MT) {
			RTMP_IO_READ32(pAd, RX_STA_CNT1, &RxStaCnt1.word);
		}

		/* Update RX PLCP error counter*/
		pAd->RalinkCounters.PlcpErrCnt += RxStaCnt1.field.PlcpErr;
		/* Update False CCA counter*/
		pAd->RalinkCounters.OneSecFalseCCACnt = RxStaCnt1.field.FalseCca;
		pAd->RalinkCounters.FalseCCACnt += RxStaCnt1.field.FalseCca;
	}

#ifdef STATS_COUNT_SUPPORT
	/* Update FCS counters*/
	OldValue= pAd->WlanCounters.FCSErrorCount.u.LowPart;
	pAd->WlanCounters.FCSErrorCount.u.LowPart += (RxStaCnt0.field.CrcErr); /* >> 7);*/
	if (pAd->WlanCounters.FCSErrorCount.u.LowPart < OldValue)
		pAd->WlanCounters.FCSErrorCount.u.HighPart++;
#endif /* STATS_COUNT_SUPPORT */

	/* Add FCS error count to private counters*/
	pRalinkCounters->OneSecRxFcsErrCnt += RxStaCnt0.field.CrcErr;
	OldValue = pRalinkCounters->RealFcsErrCount.u.LowPart;
	pRalinkCounters->RealFcsErrCount.u.LowPart += RxStaCnt0.field.CrcErr;
	if (pRalinkCounters->RealFcsErrCount.u.LowPart < OldValue)
		pRalinkCounters->RealFcsErrCount.u.HighPart++;

	/* Update Duplicate Rcv check*/
	pRalinkCounters->DuplicateRcv += RxStaCnt2.field.RxDupliCount;
#ifdef STATS_COUNT_SUPPORT
	pAd->WlanCounters.FrameDuplicateCount.u.LowPart += RxStaCnt2.field.RxDupliCount;
#endif /* STATS_COUNT_SUPPORT */
	/* Update RX Overflow counter*/
	pAd->Counters8023.RxNoBuffer += (RxStaCnt2.field.RxFifoOverflowCount);
	
	/*pAd->RalinkCounters.RxCount = 0;*/

	
	/*if (!OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_TX_RATE_SWITCH_ENABLED) || */
	/*	(OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_TX_RATE_SWITCH_ENABLED) && (pAd->MacTab.Size != 1)))*/
	if (!pAd->bUpdateBcnCntDone)
	{
		/* Update BEACON sent count*/
		NicGetTxRawCounters(pAd, &TxStaCnt0, &StaTx1);
		// TODO: shiang-7603, fix this!
		if (pAd->chipCap.hif_type != HIF_MT) {
			RTMP_IO_READ32(pAd, TX_STA_CNT2, &StaTx2.word);
		}
	}


	/*if (pAd->bStaFifoTest == TRUE)*/
#ifdef STATS_COUNT_SUPPORT
	{
			// TODO: shiang-7603, fix this!
		if (pAd->chipCap.hif_type != HIF_MT) {
			RTMP_IO_READ32(pAd, TX_AGG_CNT, &TxAggCnt.word);
			RTMP_IO_READ32(pAd, TX_AGG_CNT0, &TxAggCnt0.word);
			RTMP_IO_READ32(pAd, TX_AGG_CNT1, &TxAggCnt1.word);
			RTMP_IO_READ32(pAd, TX_AGG_CNT2, &TxAggCnt2.word);
			RTMP_IO_READ32(pAd, TX_AGG_CNT3, &TxAggCnt3.word);
			RTMP_IO_READ32(pAd, TX_AGG_CNT4, &TxAggCnt4.word);
			RTMP_IO_READ32(pAd, TX_AGG_CNT5, &TxAggCnt5.word);
			RTMP_IO_READ32(pAd, TX_AGG_CNT6, &TxAggCnt6.word);
			RTMP_IO_READ32(pAd, TX_AGG_CNT7, &TxAggCnt7.word);
		}

		pRalinkCounters->TxAggCount += TxAggCnt.field.AggTxCount;
		pRalinkCounters->TxNonAggCount += TxAggCnt.field.NonAggTxCount;
		pRalinkCounters->TxAgg1MPDUCount += TxAggCnt0.field.AggSize1Count;
		pRalinkCounters->TxAgg2MPDUCount += TxAggCnt0.field.AggSize2Count;
		
		pRalinkCounters->TxAgg3MPDUCount += TxAggCnt1.field.AggSize3Count;
		pRalinkCounters->TxAgg4MPDUCount += TxAggCnt1.field.AggSize4Count;
		pRalinkCounters->TxAgg5MPDUCount += TxAggCnt2.field.AggSize5Count;
		pRalinkCounters->TxAgg6MPDUCount += TxAggCnt2.field.AggSize6Count;
	
		pRalinkCounters->TxAgg7MPDUCount += TxAggCnt3.field.AggSize7Count;
		pRalinkCounters->TxAgg8MPDUCount += TxAggCnt3.field.AggSize8Count;
		pRalinkCounters->TxAgg9MPDUCount += TxAggCnt4.field.AggSize9Count;
		pRalinkCounters->TxAgg10MPDUCount += TxAggCnt4.field.AggSize10Count;

		pRalinkCounters->TxAgg11MPDUCount += TxAggCnt5.field.AggSize11Count;
		pRalinkCounters->TxAgg12MPDUCount += TxAggCnt5.field.AggSize12Count;
		pRalinkCounters->TxAgg13MPDUCount += TxAggCnt6.field.AggSize13Count;
		pRalinkCounters->TxAgg14MPDUCount += TxAggCnt6.field.AggSize14Count;

		pRalinkCounters->TxAgg15MPDUCount += TxAggCnt7.field.AggSize15Count;
		pRalinkCounters->TxAgg16MPDUCount += TxAggCnt7.field.AggSize16Count;

		/* Calculate the transmitted A-MPDU count*/
		pRalinkCounters->TransmittedAMPDUCount.u.LowPart += TxAggCnt0.field.AggSize1Count;
		pRalinkCounters->TransmittedAMPDUCount.u.LowPart += (TxAggCnt0.field.AggSize2Count >> 1);

		pRalinkCounters->TransmittedAMPDUCount.u.LowPart += (TxAggCnt1.field.AggSize3Count / 3);
		pRalinkCounters->TransmittedAMPDUCount.u.LowPart += (TxAggCnt1.field.AggSize4Count >> 2);

		pRalinkCounters->TransmittedAMPDUCount.u.LowPart += (TxAggCnt2.field.AggSize5Count / 5);
		pRalinkCounters->TransmittedAMPDUCount.u.LowPart += (TxAggCnt2.field.AggSize6Count / 6);

		pRalinkCounters->TransmittedAMPDUCount.u.LowPart += (TxAggCnt3.field.AggSize7Count / 7);
		pRalinkCounters->TransmittedAMPDUCount.u.LowPart += (TxAggCnt3.field.AggSize8Count >> 3);

		pRalinkCounters->TransmittedAMPDUCount.u.LowPart += (TxAggCnt4.field.AggSize9Count / 9);
		pRalinkCounters->TransmittedAMPDUCount.u.LowPart += (TxAggCnt4.field.AggSize10Count / 10);

		pRalinkCounters->TransmittedAMPDUCount.u.LowPart += (TxAggCnt5.field.AggSize11Count / 11);
		pRalinkCounters->TransmittedAMPDUCount.u.LowPart += (TxAggCnt5.field.AggSize12Count / 12);

		pRalinkCounters->TransmittedAMPDUCount.u.LowPart += (TxAggCnt6.field.AggSize13Count / 13);
		pRalinkCounters->TransmittedAMPDUCount.u.LowPart += (TxAggCnt6.field.AggSize14Count / 14);

		pRalinkCounters->TransmittedAMPDUCount.u.LowPart += (TxAggCnt7.field.AggSize15Count / 15);
		pRalinkCounters->TransmittedAMPDUCount.u.LowPart += (TxAggCnt7.field.AggSize16Count >> 4);	
	}
#endif /* STATS_COUNT_SUPPORT */			

#ifdef DBG_DIAGNOSE
	{
		RtmpDiagStruct *pDiag;
		UCHAR ArrayCurIdx, i;
		struct dbg_diag_info *diag_info;
		
		pDiag = &pAd->DiagStruct;
		ArrayCurIdx = pDiag->ArrayCurIdx;
		
		if (pDiag->inited == 0)
		{
			NdisZeroMemory(pDiag, sizeof(struct _RtmpDiagStrcut_));
			pDiag->ArrayStartIdx = pDiag->ArrayCurIdx = 0;
			pDiag->inited = 1;
		}
		else
		{
			diag_info = &pDiag->diag_info[ArrayCurIdx];
					
			/* Tx*/
			diag_info->TxFailCnt = TxStaCnt0.field.TxFailCount;
#ifdef DBG_TX_AGG_CNT
			diag_info->TxAggCnt = TxAggCnt.field.AggTxCount;
			diag_info->TxNonAggCnt = TxAggCnt.field.NonAggTxCount;

			diag_info->TxAMPDUCnt[0] = TxAggCnt0.field.AggSize1Count;
			diag_info->TxAMPDUCnt[1] = TxAggCnt0.field.AggSize2Count;
			diag_info->TxAMPDUCnt[2] = TxAggCnt1.field.AggSize3Count;
			diag_info->TxAMPDUCnt[3] = TxAggCnt1.field.AggSize4Count;
			diag_info->TxAMPDUCnt[4] = TxAggCnt2.field.AggSize5Count;
			diag_info->TxAMPDUCnt[5] = TxAggCnt2.field.AggSize6Count;
			diag_info->TxAMPDUCnt[6] = TxAggCnt3.field.AggSize7Count;
			diag_info->TxAMPDUCnt[7] = TxAggCnt3.field.AggSize8Count;
			diag_info->TxAMPDUCnt[8] = TxAggCnt4.field.AggSize9Count;
			diag_info->TxAMPDUCnt[9] = TxAggCnt4.field.AggSize10Count;
			diag_info->TxAMPDUCnt[10] = TxAggCnt5.field.AggSize11Count;
			diag_info->TxAMPDUCnt[11] = TxAggCnt5.field.AggSize12Count;
			diag_info->TxAMPDUCnt[12] = TxAggCnt6.field.AggSize13Count;
			diag_info->TxAMPDUCnt[13] = TxAggCnt6.field.AggSize14Count;
			diag_info->TxAMPDUCnt[14] = TxAggCnt7.field.AggSize15Count;
			diag_info->TxAMPDUCnt[15] = TxAggCnt7.field.AggSize16Count;
#endif /* DBG_TX_AGG_CNT */

			diag_info->RxCrcErrCnt = RxStaCnt0.field.CrcErr;
			
			INC_RING_INDEX(pDiag->ArrayCurIdx,  DIAGNOSE_TIME);
			ArrayCurIdx = pDiag->ArrayCurIdx;

			NdisZeroMemory(&pDiag->diag_info[ArrayCurIdx], sizeof(pDiag->diag_info[ArrayCurIdx]));

			if (pDiag->ArrayCurIdx == pDiag->ArrayStartIdx)
				INC_RING_INDEX(pDiag->ArrayStartIdx,  DIAGNOSE_TIME);
		}
	}
#endif /* DBG_DIAGNOSE */
}


/*
	========================================================================
	
	Routine Description:
		Clean all Tx/Rx statistic raw counters from hardware registers

	Arguments:
		pAd					Pointer to our adapter

	Return Value:
		None

	========================================================================
*/
VOID NicResetRawCounters(RTMP_ADAPTER *pAd)
{
	UINT32 val;
	
	RTMP_IO_READ32(pAd, RX_STA_CNT0, &val);
	RTMP_IO_READ32(pAd, RX_STA_CNT1, &val);
	RTMP_IO_READ32(pAd, RX_STA_CNT2, &val);
	RTMP_IO_READ32(pAd, TX_STA_CNT0, &val);
	RTMP_IO_READ32(pAd, TX_STA_CNT1, &val);
	RTMP_IO_READ32(pAd, TX_STA_CNT2, &val);
}


/*
	========================================================================
	
	Routine Description:
		Calculates the duration which is required to transmit out frames
	with given size and specified rate.
					  
	Arguments:
		pTxWI		Pointer to head of each MPDU to HW.
		Ack 		Setting for Ack requirement bit
		Fragment	Setting for Fragment bit
		RetryMode	Setting for retry mode
		Ifs 		Setting for IFS gap
		Rate		Setting for transmit rate
		Service 	Setting for service
		Length		Frame length
		TxPreamble	Short or Long preamble when using CCK rates
		QueIdx - 0-3, according to 802.11e/d4.4 June/2003
		
	Return Value:
		None

	IN RTMP_ADAPTER *pAd, 
	UCHAR *tmac_info,
	BOOLEAN FRAG,
	BOOLEAN CFACK,
	BOOLEAN InsTimestamp,
	BOOLEAN AMPDU,
	BOOLEAN Ack, 
	BOOLEAN NSeq, 
	UCHAR BASize,
	UCHAR WCID, 
	ULONG Length, 
	UCHAR PID,
	UCHAR TID, 
	UCHAR TxRate, 
	UCHAR Txopmode,
	HTTRANSMIT_SETTING *pTransmit
	
	See also : BASmartHardTransmit()    !!!
	
	========================================================================
*/
VOID write_tmac_info(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR *tmac_info,
	IN MAC_TX_INFO *info,
	IN HTTRANSMIT_SETTING *pTransmit)
{
	PMAC_TABLE_ENTRY pMac = NULL;
	UINT8 TXWISize = pAd->chipCap.TXWISize;
	TXWI_STRUC TxWI, *pTxWI, *pOutTxWI = (TXWI_STRUC *)tmac_info;
	UINT TxEAPId_Cal = 0;
	UCHAR lut_en = FALSE, eTxBf, iTxBf, sounding, ndp_rate, stbc, bw, mcs, sgi, phy_mode, mpdu_density = 0, mimops = 0, ldpc = 0;
	
	if (info->WCID < MAX_LEN_OF_MAC_TABLE)
		pMac = &pAd->MacTab.Content[info->WCID];

	/* 
		Always use Long preamble before verifiation short preamble functionality works well.
		Todo: remove the following line if short preamble functionality works
	*/
	OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED);
	NdisZeroMemory(&TxWI, TXWISize);
	pTxWI = &TxWI;

#ifdef DOT11_N_SUPPORT
	info->BASize = 0;
	stbc = pTransmit->field.STBC;
#endif /* DOT11_N_SUPPORT */
	
	/* If CCK or OFDM, BW must be 20*/
	bw = (pTransmit->field.MODE <= MODE_OFDM) ? (BW_20) : (pTransmit->field.BW);
#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
	if (bw)
		bw = (pAd->CommonCfg.AddHTInfo.AddHtInfo.RecomWidth == 0) ? (BW_20) : (pTransmit->field.BW);
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */

	ldpc = pTransmit->field.ldpc;
	mcs = pTransmit->field.MCS;
	phy_mode = pTransmit->field.MODE;
	sgi = pTransmit->field.ShortGI;
	
#ifdef DOT11_N_SUPPORT
	if (pMac)
	{
		if (pAd->CommonCfg.bMIMOPSEnable)
		{
			if ((pMac->MmpsMode == MMPS_DYNAMIC) && (pTransmit->field.MCS > 7))
			{
				/* Dynamic MIMO Power Save Mode*/
				mimops = 1;
			}
			else if (pMac->MmpsMode == MMPS_STATIC)
			{
				/* Static MIMO Power Save Mode*/
				if (pTransmit->field.MODE >= MODE_HTMIX && pTransmit->field.MCS > 7)
				{
					mcs = 7;
					mimops = 0;
				}
			}
		}

		mpdu_density = pMac->MpduDensity;
	}
#endif /* DOT11_N_SUPPORT */

#ifdef DOT11K_RRM_SUPPORT
	if (pAd->CommonCfg.VoPwrConstraintTest == TRUE)
	{
		info->AMPDU = 0;
		mcs = 0;
		ldpc = 0;
		bw = 0;
		sgi = 0;
		stbc = 0;
		phy_mode = MODE_OFDM;
	}
#endif /* DOT11K_RRM_SUPPORT */


#ifdef RLT_MAC
	if (pAd->chipCap.hif_type == HIF_RLT) {
		struct _TXWI_NMAC *txwi_n = (struct _TXWI_NMAC *)pTxWI;

		txwi_n->lut_en = lut_en;
		txwi_n->FRAG = info->FRAG;
		txwi_n->CFACK= info->CFACK;
		txwi_n->TS = info->InsTimestamp;
		txwi_n->AMPDU = info->AMPDU;
		txwi_n->ACK = info->Ack;
		txwi_n->txop = info->Txopmode;
		txwi_n->NSEQ = info->NSeq;
		txwi_n->BAWinSize = info->BASize;
		txwi_n->ShortGI = sgi;
		txwi_n->STBC = stbc;
		txwi_n->LDPC = ldpc;
		txwi_n->MCS= mcs;
		txwi_n->BW = bw;
		txwi_n->PHYMODE= phy_mode;
		txwi_n->MpduDensity = mpdu_density;
		txwi_n->MIMOps = mimops;
		txwi_n->wcid = info->WCID;
		txwi_n->MPDUtotalByteCnt = info->Length;
		txwi_n->TxPktId = mcs; // PID is not used now!
		txwi_n->GroupID = TRUE; // Group for AP --> Hauze
		txwi_n->TxEAPId = TxEAPId_Cal;
		
	}
#endif /* RLT_MAC */

#ifdef RTMP_MAC
	if (pAd->chipCap.hif_type == HIF_RTMP) {
		struct _TXWI_OMAC *txwi_o = (struct _TXWI_OMAC *)pTxWI;
		txwi_o->FRAG = info->FRAG;
		txwi_o->CFACK = info->CFACK;
		txwi_o->TS = info->InsTimestamp;
		txwi_o->AMPDU = info->AMPDU;
		txwi_o->ACK = info->Ack;
		txwi_o->txop = info->Txopmode;
		txwi_o->NSEQ = info->NSeq;
		txwi_o->BAWinSize = info->BASize;
		txwi_o->ShortGI = sgi;
		txwi_o->STBC = stbc;
		txwi_o->MCS = mcs;
		txwi_o->BW = bw;
		txwi_o->PHYMODE = phy_mode;
		txwi_o->MpduDensity = mpdu_density;
		txwi_o->MIMOps = mimops;
		txwi_o->wcid = info->WCID;
		txwi_o->MPDUtotalByteCnt = info->Length;
		txwi_o->PacketId = mcs; // PID is not used now!
	}
#endif /* RTMP_MAC */

	NdisMoveMemory(pOutTxWI, &TxWI, TXWISize);
//+++Add by shiang for debug
if (0){
	hex_dump("TxWI", (UCHAR *)pOutTxWI, TXWISize);
	dump_tmac_info(pAd, (UCHAR *)pOutTxWI);
}
//---Add by shiang for debug
}


VOID write_tmac_info_Data(RTMP_ADAPTER *pAd, UCHAR *buf, TX_BLK *pTxBlk)
{
	HTTRANSMIT_SETTING *pTransmit;
	MAC_TABLE_ENTRY *pMacEntry;
	UINT8 TXWISize = pAd->chipCap.TXWISize;
	UCHAR wcid, pkt_id;
	UCHAR sgi, mcs, bw, stbc, phy_mode, ldpc;
#ifdef DOT11_N_SUPPORT
	UCHAR basize, ampdu, mimops = 0, mpdu_density = 0;
#endif /* DOT11_N_SUPPORT */
#ifdef MCS_LUT_SUPPORT
	BOOLEAN lut_enable = 0;
#endif /* MCS_LUT_SUPPORT */
	TXWI_STRUC *pTxWI = NULL;

	pTransmit = pTxBlk->pTransmit;
	pMacEntry = pTxBlk->pMacEntry;

	/* Reserve space for TXINFO_SIZE */
	pTxBlk->hw_rsv_len = TXINFO_SIZE;
	pTxWI = (TXWI_STRUC *)(buf + TXINFO_SIZE);

	/*
		Always use Long preamble before verifiation short preamble functionality works well.
		Todo: remove the following line if short preamble functionality works
	*/
	OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED);
	NdisZeroMemory(pTxWI, TXWISize);

		wcid = pTxBlk->Wcid;

	sgi = pTransmit->field.ShortGI;
	stbc = pTransmit->field.STBC;
	ldpc = pTransmit->field.ldpc;
	mcs = pTransmit->field.MCS;
	phy_mode = pTransmit->field.MODE;
	/* If CCK or OFDM, BW must be 20 */
	bw = (pTransmit->field.MODE <= MODE_OFDM) ? (BW_20) : (pTransmit->field.BW);

#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
	if (bw)
		bw = (pAd->CommonCfg.AddHTInfo.AddHtInfo.RecomWidth == 0) ? (BW_20) : (pTransmit->field.BW);
#endif /* DOT11N_DRAFT3 */

	ampdu = ((pTxBlk->TxFrameType == TX_AMPDU_FRAME) ? TRUE : FALSE);
	basize = pAd->CommonCfg.TxBASize;
	if(ampdu && pMacEntry)
	{
		/*
 		 * Under HT20, 2x2 chipset, OPEN, and with some atero chipsets
 		 * reduce BASize to 7 to add one bulk A-MPDU during one TXOP
 		 * to improve throughput
 		 */
		if ((pAd->CommonCfg.BBPCurrentBW == BW_20) && (pAd->Antenna.field.TxPath == 2)
			&& (pMacEntry->bIAmBadAtheros) && (pMacEntry->WepStatus == Ndis802_11EncryptionDisabled))
		{
			basize = 7;
		}
		else 
		{
			UCHAR RABAOriIdx = pTxBlk->pMacEntry->BAOriWcidArray[pTxBlk->UserPriority];
			basize = pAd->BATable.BAOriEntry[RABAOriIdx].BAWinSize;
		}
	}


#endif /* DOT11_N_SUPPORT */
	
	

#ifdef DOT11_N_SUPPORT
	if (pMacEntry)
	{
		if ((pMacEntry->MmpsMode == MMPS_DYNAMIC) && (mcs > 7))
			mimops = 1;
		else if (pMacEntry->MmpsMode == MMPS_STATIC)
		{
			if ((pTransmit->field.MODE == MODE_HTMIX || pTransmit->field.MODE == MODE_HTGREENFIELD) && 
				(mcs > 7))
			{
				mcs = 7;
				mimops = 0;
			}
		}

		if ((pAd->CommonCfg.BBPCurrentBW == BW_20) && (pMacEntry->bIAmBadAtheros))
			mpdu_density = 7;
		else
			mpdu_density = pMacEntry->MpduDensity;
	}
#endif /* DOT11_N_SUPPORT */

#ifdef DBG_DIAGNOSE
	if (pTxBlk->QueIdx== 0)
	{
		pAd->DiagStruct.diag_info[pAd->DiagStruct.ArrayCurIdx].TxDataCnt++;
#ifdef DBG_TX_MCS
		if (pTransmit->field.MODE == MODE_HTMIX || pTransmit->field.MODE == MODE_HTGREENFIELD) {
			if (mcs < MAX_MCS_SET)
				pAd->DiagStruct.diag_info[pAd->DiagStruct.ArrayCurIdx].TxMcsCnt_HT[mcs]++;
		}
#ifdef DOT11_VHT_AC
		else if (pTransmit->field.MODE == MODE_VHT) {
			INT mcs_idx = ((mcs >> 4) * 10) +  (mcs & 0xf);
			if (mcs_idx < MAX_VHT_MCS_SET)
				pAd->DiagStruct.diag_info[pAd->DiagStruct.ArrayCurIdx].TxMcsCnt_VHT[mcs_idx]++;
		}
#endif /* DOT11_VHT_AC */
#endif /* DBG_TX_MCS */
	}
#endif /* DBG_DIAGNOSE */

	/* for rate adapation*/
	pkt_id = mcs;

#ifdef INF_AMAZON_SE
	/*Iverson patch for WMM A5-T07 ,WirelessStaToWirelessSta do not bulk out aggregate */
	if( RTMP_GET_PACKET_NOBULKOUT(pTxBlk->pPacket))
	{
		if(phy_mode == MODE_CCK)
			pkt_id = 6;
	}	
#endif /* INF_AMAZON_SE */

#ifdef DOT11K_RRM_SUPPORT
	if (pAd->CommonCfg.VoPwrConstraintTest == TRUE)
	{
		ampdu = 0;
		mcs = 0;
		ldpc = 0;
		bw = 0;
		sgi = 0;
		stbc = 0;
		phy_mode = MODE_OFDM;
	}
#endif /* DOT11K_RRM_SUPPORT */

#ifdef CONFIG_FPGA_MODE
	if (pAd->fpga_ctl.fpga_on & 0x6)
	{
		phy_mode = pAd->fpga_ctl.tx_data_phy;
		mcs = pAd->fpga_ctl.tx_data_mcs;
		ldpc = pAd->fpga_ctl.tx_data_ldpc;
		bw = pAd->fpga_ctl.tx_data_bw;
		sgi = pAd->fpga_ctl.tx_data_gi;
		if (pAd->fpga_ctl.data_basize)
			basize = pAd->fpga_ctl.data_basize;
	}
#endif /* CONFIG_FPGA_MODE */

#ifdef MCS_LUT_SUPPORT
	if ((RTMP_TEST_MORE_FLAG(pAd, fASIC_CAP_MCS_LUT)) && 
		(wcid < 128) && 
		(pMacEntry && pMacEntry->bAutoTxRateSwitch == TRUE))
	{
		HTTRANSMIT_SETTING rate_ctrl;

		rate_ctrl.field.MODE = phy_mode;
		rate_ctrl.field.STBC = stbc;
		rate_ctrl.field.ShortGI = sgi;
		rate_ctrl.field.BW = bw;
		rate_ctrl.field.ldpc = ldpc;
		rate_ctrl.field.MCS = mcs; 
		if (rate_ctrl.word == pTransmit->word)
			lut_enable = 1;
	}
#ifdef PEER_DELBA_TX_ADAPT
	if (RTMP_GET_PACKET_LOWRATE(pTxBlk->pPacket) || (wcid == MCAST_WCID))
		lut_enable = 0;
	else
		lut_enable = 1;
#endif /* PEER_DELBA_TX_ADAPT */
#endif /* MCS_LUT_SUPPORT */

#ifdef RLT_MAC
	if (pAd->chipCap.hif_type == HIF_RLT) {
		struct _TXWI_NMAC *txwi_n = (struct _TXWI_NMAC *)pTxWI;

		txwi_n->FRAG = TX_BLK_TEST_FLAG(pTxBlk, fTX_bAllowFrag);
		txwi_n->ACK = TX_BLK_TEST_FLAG(pTxBlk, fTX_bAckRequired);
#ifdef WFA_VHT_PF
		if (pAd->force_noack == TRUE)
			txwi_n->ACK = 0;
#endif /* WFA_VHT_PF */
		txwi_n->txop = pTxBlk->FrameGap;
		txwi_n->wcid = wcid;
		txwi_n->MPDUtotalByteCnt = pTxBlk->MpduHeaderLen + pTxBlk->SrcBufLen;
		txwi_n->CFACK = TX_BLK_TEST_FLAG(pTxBlk, fTX_bPiggyBack);
		txwi_n->ShortGI = sgi;
		txwi_n->STBC = stbc;
		txwi_n->LDPC = ldpc;

		
#ifdef MCS_LUT_SUPPORT
		if (lut_enable)
			txwi_n->MCS = 0;
		else
#endif
		txwi_n->MCS = mcs;
		
		txwi_n->PHYMODE = phy_mode;
		txwi_n->BW = bw;
		txwi_n->TxPktId = pkt_id;
			
#ifdef DOT11_N_SUPPORT
		txwi_n->AMPDU = ampdu;
		txwi_n->BAWinSize = basize;
		txwi_n->MIMOps = mimops;
		txwi_n->MpduDensity = mpdu_density;
#endif /* DOT11_N_SUPPORT */


#ifdef MCS_LUT_SUPPORT
		txwi_n->lut_en = lut_enable;
#endif /* MCS_LUT_SUPPORT */
	}
#endif /* RLT_MAC */

#ifdef RTMP_MAC
	if (pAd->chipCap.hif_type == HIF_RTMP) {
		struct _TXWI_OMAC *txwi_o = (struct _TXWI_OMAC *)pTxWI;
		
		txwi_o->FRAG = TX_BLK_TEST_FLAG(pTxBlk, fTX_bAllowFrag);
		txwi_o->ACK = TX_BLK_TEST_FLAG(pTxBlk, fTX_bAckRequired);
#ifdef WFA_VHT_PF
		if (pAd->force_noack == TRUE)
			txwi_o->ACK = 0;
#endif /* WFA_VHT_PF */
		txwi_o->txop = pTxBlk->FrameGap;
		txwi_o->wcid = wcid;
		txwi_o->MPDUtotalByteCnt = pTxBlk->MpduHeaderLen + pTxBlk->SrcBufLen;
		txwi_o->CFACK = TX_BLK_TEST_FLAG(pTxBlk, fTX_bPiggyBack);
		txwi_o->ShortGI = sgi;
		txwi_o->STBC = stbc;
		txwi_o->MCS = mcs;
		txwi_o->PHYMODE = phy_mode;
		txwi_o->BW = bw;
		txwi_o->PacketId = pkt_id;
			
#ifdef DOT11_N_SUPPORT
		txwi_o->AMPDU = ampdu;
		txwi_o->BAWinSize = basize;
		txwi_o->MIMOps = mimops;
		txwi_o->MpduDensity= mpdu_density;
#endif /* DOT11_N_SUPPORT */

	}
#endif /* RTMP_MAC */
}


VOID write_tmac_info_Cache(RTMP_ADAPTER *pAd, UCHAR *buf, TX_BLK *pTxBlk)
{
	HTTRANSMIT_SETTING *pTransmit = pTxBlk->pTransmit;
	HTTRANSMIT_SETTING tmpTransmit;
	MAC_TABLE_ENTRY *pMacEntry = pTxBlk->pMacEntry;
	STA_TR_ENTRY *tr_entry = pTxBlk->tr_entry;
	UCHAR pkt_id;
	UCHAR bw, mcs, stbc, phy_mode, sgi, ldpc;
#ifdef DOT11_N_SUPPORT
	UCHAR ampdu, basize = 0, mimops, mpdu_density = 0;
#endif /* DOT11_N_SUPPORT */
#ifdef MCS_LUT_SUPPORT
	BOOLEAN lut_enable;
#endif /* MCS_LUT_SUPPORT */
	TXWI_STRUC *pTxWI;

	/* Reserve space for TXINFO_SIZE */
	pTxBlk->hw_rsv_len = TXINFO_SIZE;
	pTxWI = (TXWI_STRUC *)(buf + TXINFO_SIZE);

	/* If CCK or OFDM, BW must be 20*/
	bw = (pTransmit->field.MODE <= MODE_OFDM) ? (BW_20) : (pTransmit->field.BW);
	sgi = pTransmit->field.ShortGI;
	stbc = pTransmit->field.STBC;
	ldpc = pTransmit->field.ldpc;
	mcs = pTransmit->field.MCS;
	phy_mode = pTransmit->field.MODE;
	pMacEntry->LastTxRate = pTransmit->word;

#ifdef DOT11_N_SUPPORT
	ampdu = ((pMacEntry->NoBADataCountDown == 0) ? TRUE: FALSE);

#ifdef DOT11N_DRAFT3
	if (bw)
		bw = (pAd->CommonCfg.AddHTInfo.AddHtInfo.RecomWidth == 0) ? (BW_20) : (pTransmit->field.BW);
#endif /* DOT11N_DRAFT3 */

	mimops = 0;
	if (pAd->CommonCfg.bMIMOPSEnable)
	{
		/* MIMO Power Save Mode*/
		if ((pMacEntry->MmpsMode == MMPS_DYNAMIC) && (pTransmit->field.MCS > 7))
			mimops = 1;
		else if (pMacEntry->MmpsMode == MMPS_STATIC)
		{
			if ((pTransmit->field.MODE >= MODE_HTMIX) && (pTransmit->field.MCS > 7))
			{
				mcs = 7;
				mimops = 0;
			}
		}
	}

	if(ampdu)
	{
		/*
 		 * Under HT20, 2x2 chipset, OPEN, and with some atero chipsets
 		 * reduce BASize to 7 to add one bulk A-MPDU during one TXOP
 		 * to improve throughput
 		 */
		if ((pAd->CommonCfg.BBPCurrentBW == BW_20) && (pMacEntry->bIAmBadAtheros))
		{
			mpdu_density = 7;
			if ((pAd->Antenna.field.TxPath == 2) &&
				(pMacEntry->WepStatus == Ndis802_11EncryptionDisabled))
			{
				basize = 7;
			}
		}

	}
#endif /* DOT11_N_SUPPORT */

#ifdef DBG_DIAGNOSE
	if (pTxBlk->QueIdx== 0)
	{
		pAd->DiagStruct.diag_info[pAd->DiagStruct.ArrayCurIdx].TxDataCnt++;
#ifdef DBG_TX_MCS
		if (pTransmit->field.MODE == MODE_HTMIX || pTransmit->field.MODE == MODE_HTGREENFIELD) {
			if (mcs < MAX_MCS_SET)
				pAd->DiagStruct.diag_info[pAd->DiagStruct.ArrayCurIdx].TxMcsCnt_HT[mcs]++;
		}
#ifdef DOT11_VHT_AC
		else if (pTransmit->field.MODE == MODE_VHT) {
			INT mcs_idx = ((mcs >> 4) * 10) +  (mcs & 0xf);
			if (mcs_idx < MAX_VHT_MCS_SET)
				pAd->DiagStruct.diag_info[pAd->DiagStruct.ArrayCurIdx].TxMcsCnt_VHT[mcs_idx]++;
		}
#endif /* DOT11_VHT_AC */
#endif /* DBG_TX_MCS */
	}
#endif /* DBG_DIAGNOSE */


#ifdef DOT11K_RRM_SUPPORT
	if (pAd->CommonCfg.VoPwrConstraintTest == TRUE)
	{
		ampdu = 0;
		mcs = 0;
		ldpc = 0;
		bw = 0;
		sgi = 0;
		stbc = 0;
		phy_mode = MODE_OFDM;
	}
#endif /* DOT11K_RRM_SUPPORT */

#ifdef CONFIG_FPGA_MODE
	if (pAd->fpga_ctl.fpga_on & 0x6)
	{
		phy_mode = pAd->fpga_ctl.tx_data_phy;
		mcs = pAd->fpga_ctl.tx_data_mcs;
		ldpc = pAd->fpga_ctl.tx_data_ldpc;
		bw = pAd->fpga_ctl.tx_data_bw;
		sgi = pAd->fpga_ctl.tx_data_gi;
		stbc = pAd->fpga_ctl.tx_data_stbc;
		if (pAd->fpga_ctl.data_basize)
			basize = pAd->fpga_ctl.data_basize;
	
		tmpTransmit.field.BW = bw; 
		tmpTransmit.field.ShortGI = sgi;
		tmpTransmit.field.STBC = stbc;
		tmpTransmit.field.ldpc = ldpc;
		tmpTransmit.field.MCS = mcs;
		tmpTransmit.field.MODE = phy_mode;

		pMacEntry->LastTxRate = tmpTransmit.word;
	}
#endif /* CONFIG_FPGA_MODE */

#ifdef MCS_LUT_SUPPORT
	lut_enable = 0;
	if (RTMP_TEST_MORE_FLAG(pAd, fASIC_CAP_MCS_LUT) && 
		(pTxBlk->Wcid < 128) && 
		(pMacEntry->bAutoTxRateSwitch == TRUE))
	{
		HTTRANSMIT_SETTING rate_ctrl;
		
		rate_ctrl.field.MODE = phy_mode;
		rate_ctrl.field.STBC = stbc;
		rate_ctrl.field.ShortGI = sgi;
		rate_ctrl.field.BW = bw;
		rate_ctrl.field.ldpc = ldpc;
		rate_ctrl.field.MCS = mcs; 
		if (rate_ctrl.word == pTransmit->word)
			lut_enable = 1;
	}
#endif /* MCS_LUT_SUPPORT */

		/* set PID for TxRateSwitching*/
		pkt_id = mcs;

#ifdef RLT_MAC
	if (pAd->chipCap.hif_type == HIF_RLT) {
		struct _TXWI_NMAC *txwi_n = (struct _TXWI_NMAC *)pTxWI;

		txwi_n->txop = IFS_HTTXOP;
		txwi_n->BW = bw;
		txwi_n->ShortGI = sgi;
		txwi_n->STBC = stbc;
		txwi_n->LDPC = ldpc;

#ifdef MCS_LUT_SUPPORT
		if (lut_enable)
			txwi_n->MCS = 0;
		else
#endif
		txwi_n->MCS = mcs;

		txwi_n->PHYMODE = phy_mode;
		txwi_n->TxPktId = pkt_id;
		txwi_n->MPDUtotalByteCnt = pTxBlk->MpduHeaderLen + pTxBlk->SrcBufLen;
		txwi_n->ACK = TX_BLK_TEST_FLAG(pTxBlk, fTX_bAckRequired);
#ifdef WFA_VHT_PF
		if (pAd->force_noack == TRUE)
			txwi_n->ACK = 0;
#endif /* WFA_VHT_PF */

#ifdef DOT11_N_SUPPORT
		txwi_n->AMPDU = ampdu;
		if (basize)
			txwi_n->BAWinSize = basize;
		txwi_n->MIMOps = mimops;
		if (mpdu_density)
			txwi_n->MpduDensity = mpdu_density;
#endif /* DOT11_N_SUPPORT */


#ifdef MCS_LUT_SUPPORT
		txwi_n->lut_en = lut_enable;
#endif /* MCS_LUT_SUPPORT */
	}
#endif /* RLT_MAC */

#ifdef RTMP_MAC
	if (pAd->chipCap.hif_type == HIF_RTMP) {
		struct _TXWI_OMAC *txwi_o = (struct _TXWI_OMAC *)pTxWI;
		
		txwi_o->txop = IFS_HTTXOP;
		txwi_o->BW = bw;
		txwi_o->ShortGI = sgi;
		txwi_o->STBC = stbc;
		txwi_o->MCS = mcs;
		txwi_o->PHYMODE = phy_mode;
		txwi_o->PacketId = pkt_id;
		txwi_o->MPDUtotalByteCnt = pTxBlk->MpduHeaderLen + pTxBlk->SrcBufLen;
		txwi_o->ACK = TX_BLK_TEST_FLAG(pTxBlk, fTX_bAckRequired);
#ifdef WFA_VHT_PF
		if (pAd->force_noack == TRUE)
			txwi_o->ACK = 0;
#endif /* WFA_VHT_PF */

#ifdef DOT11_N_SUPPORT
		txwi_o->AMPDU = ampdu;
		if (basize)
			txwi_o->BAWinSize = basize;
		txwi_o->MIMOps = mimops;
		if (mpdu_density)
			txwi_o->MpduDensity = mpdu_density;
#endif /* DOT11_N_SUPPORT */

	}
#endif /* RTMP_MAC */
}


INT rtmp_mac_set_band(RTMP_ADAPTER *pAd, int  band)
{
	UINT32 val, band_cfg;


	RTMP_IO_READ32(pAd, TX_BAND_CFG, &band_cfg);
	val = band_cfg & (~0x6);
	switch (band)
	{
		case BAND_5G:
			val |= 0x02;
			break;
		case BAND_24G:
		default:
			val |= 0x4;
			break;
	}

	if (val != band_cfg)
		RTMP_IO_WRITE32(pAd, TX_BAND_CFG, val);

	return TRUE;
}


INT rtmp_mac_set_ctrlch(RTMP_ADAPTER *pAd, UINT8 extch)
{
	UINT32 val, band_cfg;

	RTMP_IO_READ32(pAd, TX_BAND_CFG, &band_cfg);
	val = band_cfg & (~0x1);
	switch (extch)
	{
		case EXTCHA_ABOVE:
			val &= (~0x1);
			break;
		case EXTCHA_BELOW:
			val |= (0x1);
			break;
		case EXTCHA_NONE:
			val &= (~0x1);
			break;
	}

	if (val != band_cfg)
		RTMP_IO_WRITE32(pAd, TX_BAND_CFG, val);
	
	return TRUE;
}


INT rtmp_mac_set_mmps(RTMP_ADAPTER *pAd, INT ReduceCorePower)
{
	UINT32 mac_val, org_val;

	RTMP_IO_READ32(pAd, 0x1210, &org_val);
	mac_val = org_val;
	if (ReduceCorePower)
		mac_val |= 0x09;
	else
		mac_val &= ~0x09;

	if (mac_val != org_val)
		RTMP_IO_WRITE32(pAd, 0x1210, mac_val);

	return TRUE;
}


#define BCN_TBTT_OFFSET		64	/*defer 64 us*/
VOID ReSyncBeaconTime(RTMP_ADAPTER *pAd)
{
	UINT32  Offset;
	BCN_TIME_CFG_STRUC csr;
		
	Offset = (pAd->TbttTickCount) % (BCN_TBTT_OFFSET);
	pAd->TbttTickCount++;

	/*
		The updated BeaconInterval Value will affect Beacon Interval after two TBTT
		beacasue the original BeaconInterval had been loaded into next TBTT_TIMER
	*/
	if (Offset == (BCN_TBTT_OFFSET-2))
	{
		RTMP_IO_READ32(pAd, BCN_TIME_CFG, &csr.word);

		/* ASIC register in units of 1/16 TU = 64us*/
		csr.field.BeaconInterval = (pAd->CommonCfg.BeaconPeriod << 4) - 1 ;
		RTMP_IO_WRITE32(pAd, BCN_TIME_CFG, csr.word);
	}
	else if (Offset == (BCN_TBTT_OFFSET-1))
	{
		RTMP_IO_READ32(pAd, BCN_TIME_CFG, &csr.word);
		csr.field.BeaconInterval = (pAd->CommonCfg.BeaconPeriod) << 4;
		RTMP_IO_WRITE32(pAd, BCN_TIME_CFG, csr.word);
	}
}


VOID rtmp_mac_bcn_buf_init(IN RTMP_ADAPTER *pAd, BOOLEAN bHardReset)
{
	int idx, tb_size;
	RTMP_CHIP_CAP *pChipCap = &pAd->chipCap;
	
	
	for (idx = 0; idx < pChipCap->BcnMaxHwNum; idx++)
		pAd->BeaconOffset[idx] = pChipCap->BcnBase[idx];

	DBGPRINT(RT_DEBUG_TRACE, ("< Beacon Information: >\n"));
	DBGPRINT(RT_DEBUG_TRACE, ("\tFlgIsSupSpecBcnBuf = %s\n", pChipCap->FlgIsSupSpecBcnBuf ? "TRUE" : "FALSE"));
	DBGPRINT(RT_DEBUG_TRACE, ("\tBcnMaxHwNum = %d\n", pChipCap->BcnMaxHwNum));
	DBGPRINT(RT_DEBUG_TRACE, ("\tBcnMaxNum = %d\n", pChipCap->BcnMaxNum));
	DBGPRINT(RT_DEBUG_TRACE, ("\tBcnMaxHwSize = 0x%x\n", pChipCap->BcnMaxHwSize));
	DBGPRINT(RT_DEBUG_TRACE, ("\tWcidHwRsvNum = %d\n", pChipCap->WcidHwRsvNum));
	for (idx = 0; idx < pChipCap->BcnMaxHwNum; idx++) {
		DBGPRINT(RT_DEBUG_TRACE, ("\t\tBcnBase[%d] = 0x%x, pAd->BeaconOffset[%d]=0x%x\n",
					idx, pChipCap->BcnBase[idx], idx, pAd->BeaconOffset[idx]));
	}

#ifdef RTMP_MAC
	if (pAd->chipCap.hif_type == HIF_RTMP)
	{
		{
			RTMP_REG_PAIR bcn_legacy_reg_tb[] = {
#if defined(HW_BEACON_OFFSET) && (HW_BEACON_OFFSET == 0x200)
				{BCN_OFFSET0,			0xf8f0e8e0}, 
				{BCN_OFFSET1,			0x6f77d0c8},
#elif defined(HW_BEACON_OFFSET) && (HW_BEACON_OFFSET == 0x100)
				{BCN_OFFSET0,			0xece8e4e0}, /* 0x3800, 0x3A00, 0x3C00, 0x3E00, 512B for each beacon */
				{BCN_OFFSET1,			0xfcf8f4f0}, /* 0x3800, 0x3A00, 0x3C00, 0x3E00, 512B for each beacon */
#endif /* HW_BEACON_OFFSET */
			};
			
			tb_size = (sizeof(bcn_legacy_reg_tb) / sizeof(RTMP_REG_PAIR));
			for (idx = 0; idx < tb_size; idx++)
			{
				RTMP_IO_WRITE32(pAd, (USHORT)bcn_legacy_reg_tb[idx].Register, 
										bcn_legacy_reg_tb[idx].Value);
			}
		}
	}
#endif /* RTMP_MAC */

#ifdef RLT_MAC
	if (pAd->chipCap.hif_type == HIF_RLT)
	{
		RTMP_REG_PAIR bcn_mac_reg_tb[] = {
			{RLT_BCN_OFFSET0, 0x18100800},
			{RLT_BCN_OFFSET1, 0x38302820},
			{RLT_BCN_OFFSET2, 0x58504840},
			{RLT_BCN_OFFSET3, 0x78706860},
		};

		tb_size = (sizeof(bcn_mac_reg_tb) / sizeof(RTMP_REG_PAIR));
		for (idx = 0; idx < tb_size; idx ++)
		{
			RTMP_IO_WRITE32(pAd, (USHORT)bcn_mac_reg_tb[idx].Register, 
									bcn_mac_reg_tb[idx].Value);
		}
	}
#endif /* RLT_MAC */

	/* It isn't necessary to clear this space when not hard reset. */
	if (bHardReset == TRUE)
	{
		/* clear all on-chip BEACON frame space */
#ifdef CONFIG_AP_SUPPORT
		INT	i, apidx;
		for (apidx = 0; apidx < HW_BEACON_MAX_COUNT(pAd); apidx++)
		{
			if (pAd->BeaconOffset[apidx] > 0) {
				// TODO: shiang-6590, if we didn't define MBSS_SUPPORT, the pAd->BeaconOffset[x] may set as 0 when chipCap.BcnMaxHwNum != HW_BEACON_MAX_COUNT
				for (i = 0; i < HW_BEACON_OFFSET; i+=4)
					RTMP_CHIP_UPDATE_BEACON(pAd, pAd->BeaconOffset[apidx] + i, 0x00, 4); 

			}
		}
#endif /* CONFIG_AP_SUPPORT */
	}
	
}


INT rtmp_mac_pbf_init(RTMP_ADAPTER *pAd)
{
	INT idx, tb_size = 0;
	RTMP_REG_PAIR *pbf_regs = NULL;
#ifdef RLT_MAC
	RTMP_REG_PAIR rlt_pbf_regs[]={
		{TX_MAX_PCNT,		0xefef3f1f},
		{RX_MAX_PCNT,		0xfebf},
	};
#endif /* RLT_MAC */

#ifdef RTMP_MAC
	RTMP_REG_PAIR rtmp_pbf_regs[]={
#ifdef INF_AMAZON_SE
		/*
			iverson modify for usb issue, 2008/09/19
			6F + 6F < total page count FE
			so that RX doesn't occupy TX's buffer space when WMM congestion.
		*/
		{PBF_MAX_PCNT,			0x1F3F6F6F}, 
#else
		{PBF_MAX_PCNT,			0x1F3FBF9F}, 	/* 0x1F3f7f9f},		Jan, 2006/04/20 */
#endif /* INF_AMAZON_SE */
	};
#endif /* RTMP_MAC */

#ifdef RLT_MAC
	if (pAd->chipCap.hif_type == HIF_RLT)
	{
		pbf_regs = &rlt_pbf_regs[0];
		tb_size = (sizeof(rlt_pbf_regs) / sizeof(RTMP_REG_PAIR));
	}
#endif /* RLT_MAC */

#ifdef RTMP_MAC
	if (pAd->chipCap.hif_type == HIF_RTMP) {
		pbf_regs = &rtmp_pbf_regs[0];
		tb_size = (sizeof(rtmp_pbf_regs) / sizeof(RTMP_REG_PAIR));
	}
#endif /* RTMP_MAC */

	if ((pbf_regs != NULL) && (tb_size > 0))
	{
		for (idx = 0; idx < tb_size; idx++) {
			RTMP_IO_WRITE32(pAd, pbf_regs->Register, pbf_regs->Value);
			pbf_regs++;
		}
	}
	return TRUE;
}


INT rtmp_hw_tb_init(RTMP_ADAPTER *pAd, BOOLEAN bHardReset)
{

	/*
		ASIC will keep garbage value after boot
		Clear all shared key table when initial
		This routine can be ignored in radio-ON/OFF operation.
	*/
	if (bHardReset)
	{
		USHORT KeyIdx;
		UINT32 wcid_attr_base = 0, wcid_attr_size = 0, share_key_mode_base = 0;

#ifdef RLT_MAC
		if (pAd->chipCap.hif_type == HIF_RLT) {
			wcid_attr_base = RLT_MAC_WCID_ATTRIBUTE_BASE;
			wcid_attr_size = RLT_HW_WCID_ATTRI_SIZE;
			share_key_mode_base = RLT_SHARED_KEY_MODE_BASE;
		}
#endif /* RLT_MAC */
#ifdef RTMP_MAC
		if (pAd->chipCap.hif_type == HIF_RTMP) {
			wcid_attr_base = MAC_WCID_ATTRIBUTE_BASE;
			wcid_attr_size = HW_WCID_ATTRI_SIZE;
			share_key_mode_base = SHARED_KEY_MODE_BASE;
		}
#endif /* RTMP_MAC */

		for (KeyIdx = 0; KeyIdx < 4; KeyIdx++)
		{
			RTMP_IO_WRITE32(pAd, share_key_mode_base + 4*KeyIdx, 0);
		}

		/* Clear all pairwise key table when initial*/
		for (KeyIdx = 0; KeyIdx < 256; KeyIdx++)
		{
			RTMP_IO_WRITE32(pAd, wcid_attr_base + (KeyIdx * wcid_attr_size), 1);
		}
	}

	return TRUE;
}


INT rtmp_mac_sys_reset(RTMP_ADAPTER *pAd, BOOLEAN bHardReset)
{
	UINT32 mac_val;


	DBGPRINT(RT_DEBUG_TRACE, ("%s():MACVersion[Ver:Rev=0x%08x]\n",
			__FUNCTION__, pAd->MACVersion));


#ifdef RTMP_MAC_PCI
	if (bHardReset == TRUE)
	{
		mac_val = 0x3;
	}
	else
		mac_val = 0x1;
#endif /* RTMP_MAC_PCI */

	// TODO: shiang-usw, shall we add some delay here??

	return TRUE;
}


/* 
	ASIC register initialization sets
*/
RTMP_REG_PAIR MACRegTable[] = {
	{LEGACY_BASIC_RATE,		0x0000013f}, /*  Basic rate set bitmap*/
	{HT_BASIC_RATE,		0x00008003}, /* Basic HT rate set , 20M, MCS=3, MM. Format is the same as in TXWI.*/
	{MAC_SYS_CTRL,		0x00}, /* 0x1004, , default Disable RX*/
	{RX_FILTR_CFG,		0x17f97}, /*0x1400  , RX filter control,  */
	{BKOFF_SLOT_CFG,	0x209}, /* default set short slot time, CC_DELAY_TIME should be 2	 */
	{TX_SW_CFG1,		0x80606}, /* Gary,2006-08-23 */
	{TX_LINK_CFG,		0x1020},		/* Gary,2006-08-23 */
	/*{TX_TIMEOUT_CFG,	0x00182090},	 CCK has some problem. So increase timieout value. 2006-10-09 MArvek RT*/
	{TX_TIMEOUT_CFG,	0x000a2090},	/* CCK has some problem. So increase timieout value. 2006-10-09 MArvek RT , Modify for 2860E ,2007-08-01*/

	// TODO: shiang-usw, why MT7601 don't need to set this register??
	{LED_CFG,		0x7f031e46}, /* Gary, 2006-08-23*/

	/*{TX_RTY_CFG,			0x6bb80408},	 Jan, 2006/11/16*/
/* WMM_ACM_SUPPORT*/
/*	{TX_RTY_CFG,			0x6bb80101},	 sample*/
	{TX_RTY_CFG,			0x47d01f0f},	/* Jan, 2006/11/16, Set TxWI->ACK =0 in Probe Rsp Modify for 2860E ,2007-08-03*/
	
	{AUTO_RSP_CFG,			0x00000013},	/* Initial Auto_Responder, because QA will turn off Auto-Responder*/
	{CCK_PROT_CFG,			0x05740003 /*0x01740003*/},	/* Initial Auto_Responder, because QA will turn off Auto-Responder. And RTS threshold is enabled. */
	{OFDM_PROT_CFG,		0x05740003 /*0x01740003*/},	/* Initial Auto_Responder, because QA will turn off Auto-Responder. And RTS threshold is enabled. */


	{GF20_PROT_CFG,			0x01744004},    /* set 19:18 --> Short NAV for MIMO PS*/
	{GF40_PROT_CFG,			0x03F44084},    
	{MM20_PROT_CFG,		0x01744004},    

#ifdef RTMP_MAC_PCI
	{MM40_PROT_CFG,		0x03F54084},	
#endif /* RTMP_MAC_PCI */

	{TXOP_CTRL_CFG,			0x0000583f, /*0x0000243f*/ /*0x000024bf*/},	/*Extension channel backoff.*/
	{TX_RTS_CFG,			0x00092b20},	

	{EXP_ACK_TIME,			0x002400ca},	/* default value */
	{TXOP_HLDR_ET, 			0x00000002},

	/* Jerry comments 2008/01/16: we use SIFS = 10us in CCK defaultly, but it seems that 10us
		is too small for INTEL 2200bg card, so in MBSS mode, the delta time between beacon0
		and beacon1 is SIFS (10us), so if INTEL 2200bg card connects to BSS0, the ping
		will always lost. So we change the SIFS of CCK from 10us to 16us. */
	{XIFS_TIME_CFG,			0x33a41010},
};

#ifdef RTMP_MAC
RTMP_REG_PAIR MACRegTable_RTMP[] = {
	{MAX_LEN_CFG,		MAX_AGGREGATION_SIZE | 0x00001000},	/* 0x3018, MAX frame length. Max PSDU = 16kbytes.*/
	{PWR_PIN_CFG,		0x3},	/* patch for 2880-E*/
	/*{TX_SW_CFG0,		0x40a06},  Gary,2006-08-23 */
	{TX_SW_CFG0,		0x0}, 	/* Gary,2008-05-21 for CWC test */
};
#endif /* RTMP_MAC */

#ifdef CONFIG_AP_SUPPORT
RTMP_REG_PAIR APMACRegTable[] = {
	{WMM_AIFSN_CFG,		0x00001173},
	{WMM_CWMIN_CFG,	0x00002344},
	{WMM_CWMAX_CFG,	0x000034a6},
	{WMM_TXOP0_CFG,		0x00100020},
	{WMM_TXOP1_CFG,		0x002F0038},
	{TBTT_SYNC_CFG,		0x00012000},
#ifdef STREAM_MODE_SUPPORT
	{TX_CHAIN_ADDR0_L,	0xFFFFFFFF},	/* Broadcast frames are in stream mode*/
	{TX_CHAIN_ADDR0_H,	0x3FFFF},
#endif /* STREAM_MODE_SUPPORT */
};
#endif /* CONFIG_AP_SUPPORT */



#define NUM_MAC_REG_PARMS			(sizeof(MACRegTable) / sizeof(RTMP_REG_PAIR))
#ifdef CONFIG_AP_SUPPORT
#define NUM_AP_MAC_REG_PARMS		(sizeof(APMACRegTable) / sizeof(RTMP_REG_PAIR))
#endif /* CONFIG_AP_SUPPORT */
#ifdef RTMP_MAC
#define NUM_RTMP_MAC_REG_PARAMS	(sizeof(MACRegTable_RTMP)/ sizeof(RTMP_REG_PAIR))
#endif /* RTMP_MAC */

INT rtmp_mac_init(RTMP_ADAPTER *pAd)
{
	INT idx;

	for (idx = 0; idx < NUM_MAC_REG_PARMS; idx++)
	{
		RTMP_IO_WRITE32(pAd, MACRegTable[idx].Register, MACRegTable[idx].Value);
	}

#ifdef RTMP_MAC
	if (pAd->chipCap.hif_type == HIF_RTMP)
	{
		UINT32 reg, val;
	
		for (idx = 0; idx < NUM_RTMP_MAC_REG_PARAMS; idx++){
			reg = MACRegTable_RTMP[idx].Register;
			val = MACRegTable_RTMP[idx].Value;
			RTMP_IO_WRITE32(pAd, reg, val);
		}
	}
#endif /* RTMP_MAC */

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		for (idx = 0; idx < NUM_AP_MAC_REG_PARMS; idx++)
		{
			RTMP_IO_WRITE32(pAd,
				APMACRegTable[idx].Register,
				APMACRegTable[idx].Value);
		}
	}
#endif /* CONFIG_AP_SUPPORT */


	rtmp_mac_pbf_init(pAd);

	/* re-set specific MAC registers for individual chip */
	if (pAd->chipOps.AsicMacInit != NULL)
		pAd->chipOps.AsicMacInit(pAd);

	/* auto-fall back settings */
	AsicAutoFallbackInit(pAd);

	AsicSetMacMaxLen(pAd);


	return TRUE;
}


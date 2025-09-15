/****************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ****************************************************************************

	Module Name:
		cmm_data_pci.c
 
	Abstract:

	Note:
		All functions in this file must be PCI-depended, or you should move 
		your functions to other files.

	Revision History:
	Who          When          What
	---------    ----------    ----------------------------------------------
*/

 
#include	"rt_config.h"


#ifdef RT_SECURE_DMA
# ifdef NO_CONSISTENT_MEM_SUPPORT
#  error NO_CONSISTENT_MEM_SUPPORT and RT_SECURE_DMA is not supported
# endif
# ifdef WLAN_SKB_RECYCLE
#  error WLAN_SKB_RECYCLE and RT_SECURE_DMA is not supported
# endif
#else
//# error No  RT_SECURE_DMA
#endif
#ifdef DBG
VOID dump_txd(RTMP_ADAPTER *pAd, TXD_STRUC *pTxD)
{
	DBGPRINT(RT_DEBUG_OFF, ("TxD:\n"));
	
	DBGPRINT(RT_DEBUG_OFF, ("\tSDPtr0=0x%x\n", pTxD->SDPtr0));
	DBGPRINT(RT_DEBUG_OFF, ("\tSDLen0=0x%x\n", pTxD->SDLen0));
	DBGPRINT(RT_DEBUG_OFF, ("\tLastSec0=0x%x\n", pTxD->LastSec0));
	DBGPRINT(RT_DEBUG_OFF, ("\tSDPtr1=0x%x\n", pTxD->SDPtr1));
	DBGPRINT(RT_DEBUG_OFF, ("\tSDLen1=0x%x\n", pTxD->SDLen1));
	DBGPRINT(RT_DEBUG_OFF, ("\tLastSec1=0x%x\n", pTxD->LastSec1));
	DBGPRINT(RT_DEBUG_OFF, ("\tDMADONE=0x%x\n", pTxD->DMADONE));
	DBGPRINT(RT_DEBUG_OFF, ("\tBurst=0x%x\n", pTxD->Burst));
}


VOID dump_rxd(RTMP_ADAPTER *pAd, RXD_STRUC *pRxD)
{
	DBGPRINT(RT_DEBUG_OFF, ("RxD:\n"));
	
	DBGPRINT(RT_DEBUG_OFF, ("\tSDPtr0=0x%x\n", pRxD->SDP0));
	DBGPRINT(RT_DEBUG_OFF, ("\tSDLen0=0x%x\n", pRxD->SDL0));
	DBGPRINT(RT_DEBUG_OFF, ("\tLastSec0=0x%x\n", pRxD->LS0));
	DBGPRINT(RT_DEBUG_OFF, ("\tSDPtr1=0x%x\n", pRxD->SDP1));
	DBGPRINT(RT_DEBUG_OFF, ("\tSDLen1=0x%x\n", pRxD->SDL1));
	DBGPRINT(RT_DEBUG_OFF, ("\tLastSec1=0x%x\n", pRxD->LS1));
	DBGPRINT(RT_DEBUG_OFF, ("\tDDONE=0x%x\n", pRxD->DDONE));
}


VOID dumpTxRing(RTMP_ADAPTER *pAd, INT ring_idx)
{
	RTMP_TX_RING *pTxRing;
	TXD_STRUC *pTxD;
	TXINFO_STRUC *pTxInfo;
	
	int index;
	
	ASSERT(ring_idx < NUM_OF_TX_RING);
	
	pTxRing = &pAd->TxRing[ring_idx];
	for (index = 0; index < TX_RING_SIZE; index++)
	{
		pTxD = (TXD_STRUC *)pTxRing->Cell[index].AllocVa;
		hex_dump("Dump TxDesc", (UCHAR *)pTxD, sizeof(TXD_STRUC));
		dump_txd(pAd, pTxD);
		pTxInfo = (TXINFO_STRUC *)(pTxRing->Cell[index].AllocVa + sizeof(TXD_STRUC));
		hex_dump("Dump TxInfo", (UCHAR *)pTxInfo, sizeof(TXINFO_STRUC));
		dump_txinfo(pAd, pTxInfo);
	}				
}


VOID dumpRxRing(RTMP_ADAPTER *pAd, INT ring_idx)
{
	RTMP_RX_RING *pRxRing;
	RXD_STRUC *pRxD;
	int index;
	int RxRingSize = (ring_idx == 0) ? RX_RING_SIZE : RX1_RING_SIZE;

	pRxRing = &pAd->RxRing[ring_idx];
	for (index = 0; index < RxRingSize; index++)
	{
		pRxD = (RXD_STRUC *)pRxRing->Cell[index].AllocVa;
		hex_dump("Dump RxDesc", (UCHAR *)pRxD, sizeof(TXD_STRUC));
		dump_rxd(pAd, pRxD);
	}				
}
#endif /* DBG */

#ifdef DMA_BUSY_RESET
BOOLEAN MonitorTxRing(RTMP_ADAPTER *pAd)
{
	UINT32 Value;
	WPDMA_GLO_CFG_STRUC GloCfg;

	if (pAd->TxDMACheckTimes < 10)
	{
		/* Check if TX DMA busy */
		RTMP_IO_READ32(pAd, WPDMA_GLO_CFG, &GloCfg.word);

		if (GloCfg.field.TxDMABusy)
		{
			if (pAd->PDMAWatchDogDbg)
			{
				DBGPRINT(RT_DEBUG_OFF, ("%s: check point 0: value = %x\n", __FUNCTION__, GloCfg.word));
			}

			pAd->TxDMACheckTimes++; // the counter will be clean by any of Tx INT
			return FALSE;
		}
		else
		{
			pAd->TxDMACheckTimes = 0;
			return FALSE;
		}
	}
	else // reach the trigger threshold
	{
		pAd->TxDMACheckTimes = 0;
		return TRUE;
	}
}


BOOLEAN MonitorRxRing(RTMP_ADAPTER *pAd)
{
	UINT32 Value;
	WPDMA_GLO_CFG_STRUC GloCfg;

	if (pAd->RxDMACheckTimes < 10)
	{
		/* Check if RX DMA busy */
		RTMP_IO_READ32(pAd, WPDMA_GLO_CFG, &GloCfg.word);

		if (GloCfg.field.RxDMABusy)
		{
			if (pAd->PDMAWatchDogDbg)
			{
				DBGPRINT(RT_DEBUG_OFF, ("%s: check point 0, Value = %x\n", __FUNCTION__, GloCfg.word));
			}

			pAd->RxDMACheckTimes++;  // the counter will be clean by any of Rx INT
			return FALSE;
		}
		else
		{
			pAd->RxDMACheckTimes = 0;
			return FALSE;
		}
	}
	else
	{
		pAd->RxDMACheckTimes = 0;
		return TRUE;
	}
}
#endif /* DMA_BUSY_RESET */

static VOID ral_write_txinfo(
	IN RTMP_ADAPTER *pAd,
	IN TXINFO_STRUC *pTxInfo,
	IN BOOLEAN bWiv,
	IN UCHAR QueueSel)
{
#ifdef RLT_MAC
	if (pAd->chipCap.hif_type == HIF_RLT) {
		struct _TXINFO_NMAC_PKT *nmac_info = (struct _TXINFO_NMAC_PKT *)pTxInfo;

#ifdef HDR_TRANS_SUPPORT
		if (pTxBlk && pTxBlk->NeedTrans)
			nmac_info->pkt_80211 = 0;	/* 802.3 MAC header */
		else
#endif /* HDR_TRANS_SUPPORT */
			nmac_info->pkt_80211 = 1;
		nmac_info->info_type = 0;
		nmac_info->d_port = 0;
		nmac_info->cso = 0;
		nmac_info->tso = 0;
		nmac_info->wiv = (bWiv) ? 1: 0;
		nmac_info->QSEL = (pAd->bGenOneHCCA == TRUE) ? FIFO_HCCA : QueueSel;

	}
#endif /* RLT_MAC */


#ifdef RTMP_MAC
	if (pAd->chipCap.hif_type == HIF_RTMP) {
		struct _TXINFO_OMAC *omac_info = (struct _TXINFO_OMAC *)pTxInfo;

		omac_info->WIV = (bWiv) ? 1: 0;
		omac_info->QSEL = (pAd->bGenOneHCCA == TRUE) ? FIFO_HCCA : QueueSel;

	}
#endif /* RTMP_MAC */
}



/*
	========================================================================
	
	Routine Description:
		Calculates the duration which is required to transmit out frames
	with given size and specified rate.
		
	Arguments:
		pTxD		Pointer to transmit descriptor
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
		
	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL
	
	========================================================================
*/
VOID ral_write_txd(
	IN RTMP_ADAPTER *pAd,
	IN TXD_STRUC *pTxD,
	IN TXINFO_STRUC *pTxInfo,
	IN BOOLEAN bWIV,
	IN UCHAR QueueSEL)
{
	/* Always use Long preamble before verifiation short preamble functionality works well.*/
	/* Todo: remove the following line if short preamble functionality works*/
	
	//OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED);

	ral_write_txinfo(pAd, pTxInfo, bWIV, QueueSEL);
	
	pTxD->DMADONE = 0;
}

#ifdef RLT_MAC
#endif /* RLT_MAC */


#ifdef CONFIG_STA_SUPPORT
VOID ComposePsPoll(
	IN PRTMP_ADAPTER pAd)
{
	NdisZeroMemory(&pAd->PsPollFrame, sizeof (PSPOLL_FRAME));
	pAd->PsPollFrame.FC.Type = FC_TYPE_CNTL;
	pAd->PsPollFrame.FC.SubType = SUBTYPE_PS_POLL;
	pAd->PsPollFrame.Aid = pAd->StaActive.Aid | 0xC000;
	COPY_MAC_ADDR(pAd->PsPollFrame.Bssid, pAd->CommonCfg.Bssid);
	COPY_MAC_ADDR(pAd->PsPollFrame.Ta, pAd->CurrentAddress);
}
#endif /* CONFIG_STA_SUPPORT */


/* IRQL = DISPATCH_LEVEL */
VOID ComposeNullFrame(
	IN PRTMP_ADAPTER pAd)
{
	NdisZeroMemory(&pAd->NullFrame, sizeof (HEADER_802_11));
	pAd->NullFrame.FC.Type = FC_TYPE_DATA;
	pAd->NullFrame.FC.SubType = SUBTYPE_DATA_NULL;
	pAd->NullFrame.FC.ToDs = 1;
	COPY_MAC_ADDR(pAd->NullFrame.Addr1, pAd->CommonCfg.Bssid);
	COPY_MAC_ADDR(pAd->NullFrame.Addr2, pAd->CurrentAddress);
	COPY_MAC_ADDR(pAd->NullFrame.Addr3, pAd->CommonCfg.Bssid);
}


USHORT RtmpPCI_WriteSingleTxResource(
	IN RTMP_ADAPTER *pAd,
	IN TX_BLK *pTxBlk,
	IN BOOLEAN bIsLast,
	OUT USHORT *FreeNumber)
{
	UCHAR *pDMAHeaderBufVA;
	USHORT TxIdx, RetTxIdx;
	TXD_STRUC *pTxD;
	TXINFO_STRUC *pTxInfo;
	/*TXWI_STRUC *pTxWI;*/
#ifdef RT_BIG_ENDIAN
	TXD_STRUC *pDestTxD;
	UCHAR tx_hw_info[TXD_SIZE];
	UINT8 *temp;
#endif
	UINT32 BufBasePaLow;
	RTMP_TX_RING *pTxRing;
	USHORT hwHeaderLen, inf_hdr_len;
	UINT8 TXWISize = pAd->chipCap.TXWISize;


	/* get Tx Ring Resource*/
	pTxRing = &pAd->TxRing[pTxBlk->QueIdx];
	TxIdx = pAd->TxRing[pTxBlk->QueIdx].TxCpuIdx;

	pDMAHeaderBufVA = (PUCHAR) pTxRing->Cell[TxIdx].DmaBuf.AllocVa;
	BufBasePaLow = RTMP_GetPhysicalAddressLow(pTxRing->Cell[TxIdx].DmaBuf.AllocPa);

	/* copy TXINFO + TXWI + WLAN Header + LLC into DMA Header Buffer*/
	/*hwHeaderLen = ROUND_UP(pTxBlk->MpduHeaderLen, 4);*/
	hwHeaderLen = pTxBlk->MpduHeaderLen + pTxBlk->HdrPadLen + TSO_SIZE;
	inf_hdr_len = TXWISize + hwHeaderLen;
	NdisMoveMemory(pDMAHeaderBufVA,
					(UCHAR *)(pTxBlk->HeaderBuf + TXINFO_SIZE),
					inf_hdr_len);
	/*pTxWI = (TXWI_STRUC *)pDMAHeaderBufVA;*/

	pTxRing->Cell[TxIdx].pNdisPacket = pTxBlk->pPacket;
	pTxRing->Cell[TxIdx].pNextNdisPacket = NULL;

	/* build Tx Descriptor*/
#ifndef RT_BIG_ENDIAN
	pTxD = (PTXD_STRUC) pTxRing->Cell[TxIdx].AllocVa;
	pTxInfo = (TXINFO_STRUC *)(pTxRing->Cell[TxIdx].AllocVa + sizeof(TXD_STRUC));
#else
	pDestTxD = (PTXD_STRUC) pTxRing->Cell[TxIdx].AllocVa;
	NdisMoveMemory(&tx_hw_info[0], (UCHAR *)pDestTxD, TXD_SIZE);
	pTxD = (TXD_STRUC *)&tx_hw_info[0];
	pTxInfo = (TXINFO_STRUC *)(&tx_hw_info[0] + sizeof(TXD_STRUC));
#endif
	//pTxInfo = (TXINFO_STRUC *)(pTxRing->Cell[TxIdx].AllocVa + sizeof(TXD_STRUC));

	pTxD->SDPtr0 = BufBasePaLow;
	pTxD->SDLen0 = inf_hdr_len;	/* include padding*/
#ifndef RT_SECURE_DMA
	pTxD->SDPtr1 = PCI_MAP_SINGLE(pAd, pTxBlk, 0, 1, RTMP_PCI_DMA_TODEVICE);
#else
	NdisMoveMemory(pAd->TxSecureDMA[pTxBlk->QueIdx].AllocVa + (TxIdx * 4096), pTxBlk->pSrcBufData, pTxBlk->SrcBufLen);
	pTxD->SDPtr1 = pAd->TxSecureDMA[pTxBlk->QueIdx].AllocPa + (TxIdx * 4096);
#endif
	pTxD->SDLen1 = pTxBlk->SrcBufLen;
	pTxD->LastSec0 = !(pTxD->SDLen1);
	pTxD->LastSec1 = (bIsLast && pTxD->SDLen1) ? 1 : 0;
	pTxD->Burst = 0;


	if ((pAd->CommonCfg.bMcastTest == TRUE) && 
		(pTxBlk->TxFrameType == TX_MCAST_FRAME))
		ral_write_txd(pAd, pTxD, pTxInfo, FALSE, FIFO_HCCA);
	else
	        ral_write_txd(pAd, pTxD, pTxInfo, FALSE, FIFO_EDCA);

//+++Add by shiang for debug
//---Add by shiang for debug

#ifdef RT_BIG_ENDIAN
	RTMPWIEndianChange(pAd, (PUCHAR)(pDMAHeaderBufVA), TYPE_TXWI);
	RTMPFrameEndianChange(pAd, (PUCHAR)(pDMAHeaderBufVA + TXWISize), DIR_WRITE, FALSE);
	RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
	WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, FALSE, TYPE_TXD);
#endif /* RT_BIG_ENDIAN */

	RetTxIdx = TxIdx;

	/* flush dcache if no consistent memory is supported */
	RTMP_DCACHE_FLUSH(pTxRing->Cell[TxIdx].DmaBuf.AllocPa, pTxD->SDLen0);
#ifndef RT_SECURE_DMA
	RTMP_DCACHE_FLUSH(pTxBlk->pSrcBufData, pTxBlk->SrcBufLen);
#else
    RTMP_DCACHE_FLUSH(pTxD->SDPtr1, pTxBlk->SrcBufLen);
#endif
	RTMP_DCACHE_FLUSH(pTxRing->Cell[TxIdx].AllocPa, RXD_SIZE);

	/* Update Tx index*/
	INC_RING_INDEX(TxIdx, TX_RING_SIZE);
	pTxRing->TxCpuIdx = TxIdx;

	*FreeNumber -= 1;

	return RetTxIdx;
}


USHORT RtmpPCI_WriteMultiTxResource(
	IN RTMP_ADAPTER *pAd,
	IN TX_BLK *pTxBlk,
	IN UCHAR frameNum,
	OUT	USHORT *FreeNumber)
{
	BOOLEAN bIsLast;
	UCHAR *pDMAHeaderBufVA;
	USHORT TxIdx, RetTxIdx;
	TXD_STRUC *pTxD;
	TXINFO_STRUC *pTxInfo;
#ifdef RT_BIG_ENDIAN
	TXD_STRUC *pDestTxD;
	UCHAR tx_hw_info[TXD_SIZE];
	//TXD_STRUC TxD;
#endif
	UINT32 BufBasePaLow;
	RTMP_TX_RING *pTxRing;	
	USHORT hwHdrLen;
	UINT32 firstDMALen;
	UINT8 TXWISize = pAd->chipCap.TXWISize;

	bIsLast = ((frameNum == (pTxBlk->TotalFrameNum - 1)) ? 1 : 0);		

	
	/* get Tx Ring Resource*/
	pTxRing = &pAd->TxRing[pTxBlk->QueIdx];
	TxIdx = pAd->TxRing[pTxBlk->QueIdx].TxCpuIdx;
	pDMAHeaderBufVA = (PUCHAR) pTxRing->Cell[TxIdx].DmaBuf.AllocVa;
	BufBasePaLow = RTMP_GetPhysicalAddressLow(pTxRing->Cell[TxIdx].DmaBuf.AllocPa);

	if (frameNum == 0)
	{
		/* copy TXINFO + TXWI + WLAN Header + LLC into DMA Header Buffer*/
		if (pTxBlk->TxFrameType == TX_AMSDU_FRAME)
			/*hwHdrLen = ROUND_UP(pTxBlk->MpduHeaderLen-AMSDU_SUBHEAD_LEN, 4)+AMSDU_SUBHEAD_LEN;*/
			hwHdrLen = pTxBlk->MpduHeaderLen - AMSDU_SUBHEAD_LEN + pTxBlk->HdrPadLen + AMSDU_SUBHEAD_LEN;
		else if (pTxBlk->TxFrameType == TX_RALINK_FRAME)
			/*hwHdrLen = ROUND_UP(pTxBlk->MpduHeaderLen-ARALINK_HEADER_LEN, 4)+ARALINK_HEADER_LEN;*/
			hwHdrLen = pTxBlk->MpduHeaderLen - ARALINK_HEADER_LEN + pTxBlk->HdrPadLen + ARALINK_HEADER_LEN;

		/*hwHdrLen = ROUND_UP(pTxBlk->MpduHeaderLen, 4);*/
		hwHdrLen = pTxBlk->MpduHeaderLen + pTxBlk->HdrPadLen;

		firstDMALen = TXWISize + hwHdrLen;
	}
	else
	{
		firstDMALen = pTxBlk->MpduHeaderLen;
	}

	NdisMoveMemory(pDMAHeaderBufVA, pTxBlk->HeaderBuf + TXINFO_SIZE, firstDMALen); 
		
	pTxRing->Cell[TxIdx].pNdisPacket = pTxBlk->pPacket;
	pTxRing->Cell[TxIdx].pNextNdisPacket = NULL;
	
	/* build Tx Descriptor*/
#ifndef RT_BIG_ENDIAN
	pTxD = (PTXD_STRUC) pTxRing->Cell[TxIdx].AllocVa;
	pTxInfo = (TXINFO_STRUC *)(pTxRing->Cell[TxIdx].AllocVa + sizeof(TXD_STRUC));
#else
	pDestTxD = (PTXD_STRUC) pTxRing->Cell[TxIdx].AllocVa;
	NdisMoveMemory(&tx_hw_info[0], (UCHAR *)pDestTxD, TXD_SIZE);
	pTxD = (TXD_STRUC *)&tx_hw_info[0];
	pTxInfo = (TXINFO_STRUC *)(&tx_hw_info[0] + sizeof(TXD_STRUC));
#endif

	pTxD->SDPtr0 = BufBasePaLow;
	pTxD->SDLen0 = firstDMALen; /* include padding*/
#ifndef RT_SECURE_DMA
	pTxD->SDPtr1 = PCI_MAP_SINGLE(pAd, pTxBlk, 0, 1, RTMP_PCI_DMA_TODEVICE);
#else
	NdisMoveMemory(pAd->TxSecureDMA[pTxBlk->QueIdx].AllocVa + (TxIdx * 4096), pTxBlk->pSrcBufData, pTxBlk->SrcBufLen);
	pTxD->SDPtr1 = pAd->TxSecureDMA[pTxBlk->QueIdx].AllocPa + (TxIdx * 4096);
#endif
	pTxD->SDLen1 = pTxBlk->SrcBufLen;
	pTxD->LastSec0 = !(pTxD->SDLen1);
	pTxD->LastSec1 = (bIsLast && pTxD->SDLen1) ? 1 : 0;
	pTxD->Burst = 0;

	if ((pAd->CommonCfg.bMcastTest == TRUE) && 
		(pTxBlk->TxFrameType == TX_MCAST_FRAME))
		ral_write_txd(pAd, pTxD, pTxInfo, FALSE, FIFO_HCCA);
	else
	        ral_write_txd(pAd, pTxD, pTxInfo, FALSE, FIFO_EDCA);

#ifdef RT_BIG_ENDIAN
	if (frameNum == 0)
		RTMPFrameEndianChange(pAd, (PUCHAR)(pDMAHeaderBufVA + TXWISize), DIR_WRITE, FALSE);
	
	if (frameNum != 0)
		RTMPWIEndianChange(pAd, (PUCHAR)pDMAHeaderBufVA, TYPE_TXWI);
	
	RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
	WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, FALSE, TYPE_TXD);
#endif /* RT_BIG_ENDIAN */
	
	RetTxIdx = TxIdx;
	
	/* flush dcache if no consistent memory is supported */
	RTMP_DCACHE_FLUSH(pTxRing->Cell[TxIdx].DmaBuf.AllocPa, pTxD->SDLen0);
	RTMP_DCACHE_FLUSH(pTxBlk->pSrcBufData, pTxBlk->SrcBufLen);
	RTMP_DCACHE_FLUSH(pTxRing->Cell[TxIdx].AllocPa, RXD_SIZE);

	/* Update Tx index*/
	INC_RING_INDEX(TxIdx, TX_RING_SIZE);
	pTxRing->TxCpuIdx = TxIdx;

	*FreeNumber -= 1;

	return RetTxIdx;
}


VOID RtmpPCI_FinalWriteTxResource(
	IN RTMP_ADAPTER *pAd,
	IN TX_BLK *pTxBlk,
	IN USHORT totalMPDUSize,
	IN USHORT FirstTxIdx)
{
	TXWI_STRUC *pTxWI;
	RTMP_TX_RING *pTxRing;

	/* get Tx Ring Resource*/
	pTxRing = &pAd->TxRing[pTxBlk->QueIdx];
	pTxWI = (TXWI_STRUC *) pTxRing->Cell[FirstTxIdx].DmaBuf.AllocVa;
#ifdef RLT_MAC
	if (pAd->chipCap.hif_type == HIF_RLT)
		pTxWI->TXWI_N.MPDUtotalByteCnt = totalMPDUSize;
#endif /* RLT_MAC */
#ifdef RTMP_MAC
	if (pAd->chipCap.hif_type == HIF_RTMP)
		pTxWI->TXWI_O.MPDUtotalByteCnt = totalMPDUSize;
#endif /* RTMP_MAC */
#ifdef RT_BIG_ENDIAN
	RTMPWIEndianChange(pAd, (PUCHAR)pTxWI, TYPE_TXWI);
#endif /* RT_BIG_ENDIAN */

}


USHORT	RtmpPCI_WriteFragTxResource(
	IN RTMP_ADAPTER *pAd,
	IN TX_BLK *pTxBlk,
	IN UCHAR fragNum,
	OUT	USHORT *FreeNumber)
{
	UCHAR *pDMAHeaderBufVA;
	USHORT TxIdx, RetTxIdx;
	TXD_STRUC *pTxD;
	TXINFO_STRUC *pTxInfo;
#ifdef RT_BIG_ENDIAN
	TXD_STRUC *pDestTxD;
	UCHAR tx_hw_info[TXD_SIZE];
	//TXD_STRUC TxD;
#endif
	UINT32 BufBasePaLow;
	RTMP_TX_RING *pTxRing;
	USHORT hwHeaderLen;
	UINT32 firstDMALen;
	UINT8 TXWISize = pAd->chipCap.TXWISize;


	/* Get Tx Ring Resource*/
	pTxRing = &pAd->TxRing[pTxBlk->QueIdx];
	TxIdx = pAd->TxRing[pTxBlk->QueIdx].TxCpuIdx;
	pDMAHeaderBufVA = (PUCHAR) pTxRing->Cell[TxIdx].DmaBuf.AllocVa;
	BufBasePaLow = RTMP_GetPhysicalAddressLow(pTxRing->Cell[TxIdx].DmaBuf.AllocPa);

	
	/* Copy TXINFO + TXWI + WLAN Header + LLC into DMA Header Buffer*/
	
	/*hwHeaderLen = ROUND_UP(pTxBlk->MpduHeaderLen, 4);*/
	hwHeaderLen = pTxBlk->MpduHeaderLen + pTxBlk->HdrPadLen;

	firstDMALen = TXWISize + hwHeaderLen;
	NdisMoveMemory(pDMAHeaderBufVA, (UCHAR *)(pTxBlk->HeaderBuf + TXINFO_SIZE), firstDMALen); 
		
	/* Build Tx Descriptor*/
#ifndef RT_BIG_ENDIAN
	pTxD = (PTXD_STRUC) pTxRing->Cell[TxIdx].AllocVa;
	pTxInfo = (TXINFO_STRUC *)(pTxRing->Cell[TxIdx].AllocVa + sizeof(TXD_STRUC));
#else
	pDestTxD = (PTXD_STRUC) pTxRing->Cell[TxIdx].AllocVa;
	NdisMoveMemory(&tx_hw_info[0], (UCHAR *)pDestTxD, TXD_SIZE);
	pTxD = (TXD_STRUC *)&tx_hw_info[0];
	pTxInfo = (TXINFO_STRUC *)(&tx_hw_info[0] + sizeof(TXD_STRUC));
#endif
	
	if (fragNum == pTxBlk->TotalFragNum)
	{
		pTxRing->Cell[TxIdx].pNdisPacket = pTxBlk->pPacket;
		pTxRing->Cell[TxIdx].pNextNdisPacket = NULL;
	}
	
	pTxD->SDPtr0 = BufBasePaLow;
	pTxD->SDLen0 = firstDMALen; /* include padding*/
#ifndef RT_SECURE_DMA
	pTxD->SDPtr1 = PCI_MAP_SINGLE(pAd, pTxBlk, 0, 1, RTMP_PCI_DMA_TODEVICE);
#else
	NdisMoveMemory(pAd->TxSecureDMA[pTxBlk->QueIdx].AllocVa + (TxIdx * 4096), pTxBlk->pSrcBufData, pTxBlk->SrcBufLen);
	pTxD->SDPtr1 = pAd->TxSecureDMA[pTxBlk->QueIdx].AllocPa + (TxIdx * 4096);
#endif
	pTxD->SDLen1 = pTxBlk->SrcBufLen;
	if (pTxD->SDLen1 > 0)
	{
		pTxD->LastSec0 = 0;
		pTxD->LastSec1 = 1;
	}
	else
	{
		pTxD->LastSec0 = 1;
		pTxD->LastSec1 = 0;
	}
	pTxD->Burst = 0;
	ral_write_txd(pAd, pTxD, pTxInfo, FALSE, FIFO_EDCA);

#ifdef RT_BIG_ENDIAN
	RTMPWIEndianChange(pAd, pDMAHeaderBufVA, TYPE_TXWI);
	RTMPFrameEndianChange(pAd, (PUCHAR)(pDMAHeaderBufVA + TXWISize), DIR_WRITE, FALSE);
	RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
	WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, FALSE, TYPE_TXD);	
#endif /* RT_BIG_ENDIAN */

	RetTxIdx = TxIdx;
	pTxBlk->Priv += pTxBlk->SrcBufLen;
	
	/* flush dcache if no consistent memory is supported */
	RTMP_DCACHE_FLUSH(pTxRing->Cell[TxIdx].DmaBuf.AllocPa, pTxD->SDLen0);
	RTMP_DCACHE_FLUSH(pTxBlk->pSrcBufData, pTxBlk->SrcBufLen);
	RTMP_DCACHE_FLUSH(pTxRing->Cell[TxIdx].AllocPa, RXD_SIZE);

	/* Update Tx index*/
	INC_RING_INDEX(TxIdx, TX_RING_SIZE);
	pTxRing->TxCpuIdx = TxIdx;

	*FreeNumber -= 1;

	return RetTxIdx;
}


/*
	Must be run in Interrupt context
	This function handle PCI specific TxDesc and cpu index update and kick the packet out.
 */
int RtmpPCIMgmtKickOut(
	IN RTMP_ADAPTER *pAd, 
	IN UCHAR QueIdx,
	IN PNDIS_PACKET pPacket,
	IN UCHAR *pSrcBufVA,
	IN UINT SrcBufLen)
{
	TXD_STRUC *pTxD;
#ifdef RT_BIG_ENDIAN
	UCHAR tx_hw_info[TXD_SIZE];
	TXD_STRUC *pDestTxD;
#endif
	TXINFO_STRUC *pTxInfo;
	ULONG SwIdx = pAd->MgmtRing.TxCpuIdx;
	INT pkt_len = SrcBufLen - TXINFO_SIZE;
		
#ifdef RT_BIG_ENDIAN
	pDestTxD = (PTXD_STRUC)pAd->MgmtRing.Cell[SwIdx].AllocVa;
	NdisMoveMemory(&tx_hw_info[0], (UCHAR *)pDestTxD, TXD_SIZE);
	pTxD = (TXD_STRUC *)&tx_hw_info[0];
	RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#else
	pTxD = (TXD_STRUC *)pAd->MgmtRing.Cell[SwIdx].AllocVa;
#endif
	pTxInfo = (TXINFO_STRUC *)(((UCHAR *)pTxD) + sizeof(TXD_STRUC));

	pAd->MgmtRing.Cell[SwIdx].pNdisPacket = pPacket;
	pAd->MgmtRing.Cell[SwIdx].pNextNdisPacket = NULL;

	pTxD->LastSec0 = 1;
	pTxD->LastSec1 = 0;
	pTxD->SDLen1 = 0;
#ifndef RT_SECURE_DMA
	pTxD->SDPtr0 = PCI_MAP_SINGLE(pAd, (pSrcBufVA + TXINFO_SIZE), pkt_len, 0, RTMP_PCI_DMA_TODEVICE);
#else
	NdisMoveMemory(pAd->MgmtSecureDMA.AllocVa + (SwIdx * 4096), pSrcBufVA + TXINFO_SIZE, pkt_len);
	pTxD->SDPtr0 = pAd->MgmtSecureDMA.AllocPa + (SwIdx * 4096);
#endif
	pTxD->SDLen0 = pkt_len;
	pTxD->Burst = 0;

	ral_write_txd(pAd, pTxD, pTxInfo, TRUE, FIFO_MGMT);

#ifdef RT_BIG_ENDIAN
	RTMPDescriptorEndianChange((UCHAR *)pTxD, TYPE_TXD);
	WriteBackToDescriptor((UCHAR *)pDestTxD, (UCHAR *)pTxD, FALSE, TYPE_TXD);
#endif

//+++Add by shiang for debug
//---Add by shiang for debug


	pAd->RalinkCounters.KickTxCount++;
	pAd->RalinkCounters.OneSecTxDoneCount++;

	/* flush dcache if no consistent memory is supported */
	RTMP_DCACHE_FLUSH(pSrcBufVA, SrcBufLen);
	RTMP_DCACHE_FLUSH(pAd->MgmtRing.Cell[SwIdx].AllocPa, TXD_SIZE);

	/* Increase TX_CTX_IDX, but write to register later.*/
	INC_RING_INDEX(pAd->MgmtRing.TxCpuIdx, MGMT_RING_SIZE);

	RTMP_IO_WRITE32(pAd, pAd->MgmtRing.hw_cidx_addr,  pAd->MgmtRing.TxCpuIdx);
	return 0;
}


BOOLEAN RTMPFreeTXDUponTxDmaDone(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR QueIdx)
{
	RTMP_TX_RING *pTxRing;
	TXD_STRUC *pTxD;
#ifdef RT_BIG_ENDIAN
	TXD_STRUC *pDestTxD;
	UCHAR tx_hw_info[TXD_SIZE];
#endif
	PNDIS_PACKET pPacket;
	UCHAR FREE = 0;
	BOOLEAN bReschedule = FALSE;
	UINT8 TXWISize = pAd->chipCap.TXWISize;
#ifdef RALINK_ATE
#ifdef TXBF_SUPPORT
	ULONG stTimeChk;
        PATE_INFO pATEInfo = &(pAd->ate);
#endif /* TXBF_SUPPORT */
#endif /* RALINK_ATE */

	ASSERT(QueIdx < NUM_OF_TX_RING);
	if (QueIdx >= NUM_OF_TX_RING)
		return FALSE;

	pTxRing = &pAd->TxRing[QueIdx];
	RTMP_IO_READ32(pAd, pTxRing->hw_didx_addr, &pTxRing->TxDmaIdx);
#ifdef RALINK_ATE
#ifdef TXBF_SUPPORT
    if (pAd->ceBfCertificationFlg == TRUE)
    {
	    pATEInfo->TxDoneCount = 0;
	
	    /* ATE special mode: Every 500ms, insert one sounding packet into TX ring. 
	    After the packet is sent out, revert the packet back to normal packet */

	    NdisGetSystemUpTime(&stTimeChk);   // get system time

        //if TxDmaIdx overflow happens, reset sounding packet 
        if ((pTxRing->TxDmaIdx < pAd->txDmaIdx_backup) &&
            (pAd->timeout_flg == FALSE) && (pAd->ceBfCertificationFlg == TRUE))   
        {
	
	        if ( pTxRing->TxDmaIdx > 30)
       	    {			
	            pAd->timeout_flg = TRUE;
			    pATEInfo->TxLength = pAd->txLengthBackup;////2050;
                pATEInfo->txSoundingMode =0; 
       	    }
        }
		
	    if ((((stTimeChk - pAd->sounding_periodic_count)*1000/OS_HZ) >= 500) &&   //500ms period  
              (pTxRing->TxDmaIdx >= (TX_RING_SIZE - 10)))
	    {
		    pAd->timeout_flg = FALSE;
		    NdisGetSystemUpTime(&pAd->sounding_periodic_count);    //get and save system time
		
		    pATEInfo->TxLength = 258;
		    pATEInfo->txSoundingMode = pAd->soundingMode;

            //if ( pAd->soundingPacketDone == 0)
            {
                pAd->soundingPacketDone = 1;
                /* Abort Tx, Rx DMA. */
		        RtmpSoundingDmaEnable(pAd, 0);
                
                if (pATEInfo->TxWI.TXWI_N.PHYMODE == MODE_VHT && pATEInfo->bTxBF == TRUE && pATEInfo->txSoundingMode != 0)
		        {           
			        if (ATESetUpNDPAFrame(pAd, TX_RING_SIZE-1) != 0)
				        return NDIS_STATUS_FAILURE;

                    if (ATESetUpNDPAFrame(pAd, 100) != 0)
				        return NDIS_STATUS_FAILURE;
		        }
		        else
		        {
			        if (ATESetUpFrame(pAd, TX_RING_SIZE-1) != 0)
				        return NDIS_STATUS_FAILURE;

                    if (ATESetUpFrame(pAd, 100) != 0)
				        return NDIS_STATUS_FAILURE;
		        } 
               
		        /* Start Tx, Rx DMA. */
		        RtmpSoundingDmaEnable(pAd, 1);
            }                

            pAd->txDmaIdx_backup = pTxRing->TxDmaIdx;

            RTMP_IO_READ32(pAd, pTxRing->hw_didx_addr, &pTxRing->TxDmaIdx);
		    //DBGPRINT(RT_DEBUG_OFF, ("****[7]TxDmaIdx =%d,TxCpuIdx =%d, Put Sounding Packet[%d]\n", pTxRing->TxDmaIdx, pTxRing->TxCpuIdx,TX_RING_SIZE-1));
	    }
	}
#endif
#endif
	while (pTxRing->TxSwFreeIdx != pTxRing->TxDmaIdx)
	{
		RTMP_DMACB *dma_cb = &pTxRing->Cell[pTxRing->TxSwFreeIdx];

		pAd->tx_packet_counter++;
#ifdef RALINK_ATE
#ifdef RALINK_QA
		if ((ATE_ON(pAd)) && (pAd->ate.bQATxStart == TRUE) && (pAd->ate.QID == QueIdx))
		{
			HEADER_802_11 *pHeader80211;
			
			pAd->ate.TxDoneCount++;
			pAd->RalinkCounters.KickTxCount++;

			/* always use QID_AC_BE and FIFO_EDCA */
			ASSERT(pAd->ate.QID == 0);
			pAd->ate.TxAc0++;

			FREE++;
#ifndef RT_BIG_ENDIAN
			pTxD = (PTXD_STRUC) (dma_cb->AllocVa);
#else
			pDestTxD = (PTXD_STRUC) (dma_cb->AllocVa);
			NdisMoveMemory(&tx_hw_info[0], pDestTxD, TXD_SIZE);
			pTxD = (PTXD_STRUC)&tx_hw_info[0];
			RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#endif
/*			pTxD->DMADONE = 0; */

			pHeader80211 = (PHEADER_802_11)((UCHAR *)(dma_cb->DmaBuf.AllocVa) + TXWISize);
#ifdef RT_BIG_ENDIAN
			RTMPFrameEndianChange(pAd, (PUCHAR)pHeader80211, DIR_READ, FALSE);
#endif
			pHeader80211->Sequence = ++pAd->ate.seq;
#ifdef RT_BIG_ENDIAN
			RTMPFrameEndianChange(pAd, (PUCHAR)pHeader80211, DIR_WRITE, FALSE);
#endif

			if  ((pAd->ate.bQATxStart == TRUE) && (pAd->ate.Mode & ATE_TXFRAME) && (pAd->ate.TxDoneCount < pAd->ate.TxCount))
			{
				pAd->RalinkCounters.TransmittedByteCount +=  (pTxD->SDLen1 + pTxD->SDLen0);
				pAd->RalinkCounters.OneSecTransmittedByteCount += (pTxD->SDLen1 + pTxD->SDLen0);
				pAd->RalinkCounters.OneSecDmaDoneCount[QueIdx] ++;

				/* flush dcache if no consistent memory is supported */
				RTMP_DCACHE_FLUSH(dma_cb->AllocPa, RXD_SIZE);

				INC_RING_INDEX(pTxRing->TxSwFreeIdx, TX_RING_SIZE);

				/* get TX_DTX_IDX again */
				RTMP_IO_READ32(pAd, pTxRing->hw_didx_addr,  &pTxRing->TxDmaIdx);
				goto kick_out;
			}
			else if ((pAd->ate.TxStatus == 1)/* or (pAd->ate.bQATxStart == TRUE) ??? */ && (pAd->ate.TxDoneCount == pAd->ate.TxCount))
			{
				DBGPRINT(RT_DEBUG_TRACE,("all Tx is done\n"));

				/* Tx status enters idle mode.*/
				pAd->ate.TxStatus = 0;
			}
			else if (!(pAd->ate.Mode & ATE_TXFRAME))
			{
				/* not complete sending yet, but someone press the Stop TX botton */
				DBGPRINT(RT_DEBUG_INFO,("not complete sending yet, but someone pressed the Stop TX bottom\n"));
				DBGPRINT(RT_DEBUG_INFO,("pAd->ate.Mode = 0x%02x\n", pAd->ate.Mode));
			}
			else
			{
				DBGPRINT(RT_DEBUG_OFF,("pTxRing->TxSwFreeIdx = %d\n", pTxRing->TxSwFreeIdx));
			}

#ifdef RT_BIG_ENDIAN
			RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
			NdisMoveMemory(pDestTxD, pTxD, TXD_SIZE);
#endif /* RT_BIG_ENDIAN */

			/* flush dcache if no consistent memory is supported */
			RTMP_DCACHE_FLUSH(dma_cb->AllocPa, RXD_SIZE);

			INC_RING_INDEX(pTxRing->TxSwFreeIdx, TX_RING_SIZE);
			continue;
		}
#endif /* RALINK_QA */
#endif /* RALINK_ATE */

		/*
			Note:

			Can not take off the NICUpdateFifoStaCounters(); Or the
			FIFO overflow rate will be high, i.e. > 3%
			(see the rate by "iwpriv ra0 show stainfo")

			Based on different platform, try to find the best value to
			replace '4' here (overflow rate target is about 0%).
		*/
		if (++pAd->FifoUpdateRx >= FIFO_STAT_READ_PERIOD)
		{
			NICUpdateFifoStaCounters(pAd);
			pAd->FifoUpdateRx = 0;
		}

		/* Note : If (pAd->ate.bQATxStart == TRUE), we will never reach here. */
		FREE++;
#ifndef RT_BIG_ENDIAN
		pTxD = (TXD_STRUC *) (dma_cb->AllocVa);
#else
		pDestTxD = (PTXD_STRUC) (dma_cb->AllocVa);
		NdisMoveMemory(&tx_hw_info[0], pDestTxD, TXD_SIZE);
		pTxD = (PTXD_STRUC)&tx_hw_info[0];
		RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#endif
/*		pTxD->DMADONE = 0; */

#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
#ifdef UAPSD_SUPPORT
		UAPSD_SP_PacketCheck(pAd,
				dma_cb->pNdisPacket,
				((UCHAR *)dma_cb->DmaBuf.AllocVa) + TXWISize);
#endif /* UAPSD_SUPPORT */
#else
#ifdef CONFIG_AP_SUPPORT
#ifdef UAPSD_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			UAPSD_SP_PacketCheck(pAd,
				dma_cb->pNdisPacket,
				((UCHAR *)dma_cb->DmaBuf.AllocVa) + TXWISize);
		}
#endif /* UAPSD_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
#endif /* defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) */

#ifdef RALINK_ATE
		/* Execution of this block is not allowed when ATE is running. */
		if (!(ATE_ON(pAd)))
#endif /* RALINK_ATE */
		{
			pPacket = dma_cb->pNdisPacket;
			if (pPacket)
			{
				dma_cb->pNdisPacket = NULL;

#ifdef CONFIG_5VT_ENHANCE
				if (RTMP_GET_PACKET_5VT(pPacket))
					PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr1, 16, RTMP_PCI_DMA_TODEVICE);
				else
#endif /* CONFIG_5VT_ENHANCE */
					PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr1, pTxD->SDLen1, RTMP_PCI_DMA_TODEVICE);

#ifdef WLAN_SKB_RECYCLE
				if (skb_queue_len(&pAd->rx0_recycle) < NUM_RX_DESC &&
					skb_recycle_check(RTPKT_TO_OSPKT(pPacket), RX_BUFFER_NORMSIZE))
					__skb_queue_head(&pAd->rx0_recycle, RTPKT_TO_OSPKT(pPacket));
				else
#endif /* WLAN_SKB_RECYCLE */
					RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
			}

			pPacket = dma_cb->pNextNdisPacket;
			if (pPacket)
			{
				dma_cb->pNextNdisPacket = NULL;
#ifdef CONFIG_5VT_ENHANCE
				if (RTMP_GET_PACKET_5VT(pPacket))
					PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr1, 16, RTMP_PCI_DMA_TODEVICE);
				else
#endif /* CONFIG_5VT_ENHANCE */
					PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr1, pTxD->SDLen1, RTMP_PCI_DMA_TODEVICE);

#ifdef WLAN_SKB_RECYCLE
				if (skb_queue_len(&pAd->rx0_recycle) < NUM_RX_DESC &&
					skb_recycle_check(RTPKT_TO_OSPKT(pPacket), RX_BUFFER_NORMSIZE ))
					__skb_queue_head(&pAd->rx0_recycle, RTPKT_TO_OSPKT(pPacket));
				else
#endif /* WLAN_SKB_RECYCLE */
					RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
			}
		}

		/* flush dcache if no consistent memory is supported */
		RTMP_DCACHE_FLUSH(dma_cb->AllocPa, TXD_SIZE);

		pAd->RalinkCounters.TransmittedByteCount +=  (pTxD->SDLen1 + pTxD->SDLen0);
		pAd->RalinkCounters.OneSecTransmittedByteCount += (pTxD->SDLen1 + pTxD->SDLen0);
		pAd->RalinkCounters.OneSecDmaDoneCount[QueIdx] ++;
		INC_RING_INDEX(pTxRing->TxSwFreeIdx, TX_RING_SIZE);

		/* get tx_tdx_idx again */
		//if (pTxRing->TxSwFreeIdx == pTxRing->TxDmaIdx)
		//	RTMP_IO_READ32(pAd, pTxRing->hw_didx_addr,  &pTxRing->TxDmaIdx);


#ifdef RALINK_ATE
#ifdef RALINK_QA
kick_out:
#endif /* RALINK_QA */

		/*
			ATE_TXCONT mode also need to send some normal frames, so let it in.
			ATE_STOP must be changed not to be 0xff
			to prevent it from running into this block.
		*/
		if ((pAd->ate.Mode & ATE_TXFRAME) && (pAd->ate.QID == QueIdx))
		{
			/* TxDoneCount++ has been done if QA is used.*/
			if (pAd->ate.bQATxStart == FALSE)
			{
				pAd->ate.TxDoneCount++;
			}
			if (((pAd->ate.TxCount - pAd->ate.TxDoneCount + 1) >= TX_RING_SIZE))
			{
				/* Note : We increase TxCpuIdx here, not TxSwFreeIdx ! */

				/* flush dcache if no consistent memory is supported */
				RTMP_DCACHE_FLUSH(pTxRing->Cell[pTxRing->TxCpuIdx].AllocPa, RXD_SIZE);

				INC_RING_INDEX(pAd->TxRing[QueIdx].TxCpuIdx, TX_RING_SIZE);
#ifndef RT_BIG_ENDIAN
				pTxD = (PTXD_STRUC) (pTxRing->Cell[pAd->TxRing[QueIdx].TxCpuIdx].AllocVa);
#else
				pDestTxD = (PTXD_STRUC) (pTxRing->Cell[pAd->TxRing[QueIdx].TxCpuIdx].AllocVa);
				NdisMoveMemory(&tx_hw_info[0], pDestTxD, TXD_SIZE);
				pTxD = (PTXD_STRUC)&tx_hw_info[0];
				RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#endif
				pTxD->DMADONE = 0;
#ifdef RT_BIG_ENDIAN
				RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
				NdisMoveMemory(pDestTxD, pTxD, TXD_SIZE);
#endif

				/* flush dcache if no consistent memory is supported */
				RTMP_DCACHE_FLUSH(pTxRing->Cell[pTxRing->TxCpuIdx].AllocPa, RXD_SIZE);

				/* kick Tx-Ring*/
				RTMP_IO_WRITE32(pAd, pTxRing->hw_cidx_addr, pTxRing->TxCpuIdx);

				pAd->RalinkCounters.KickTxCount++;
			}
		}
#endif /* RALINK_ATE */
	}


	return  bReschedule;

}


/*
	========================================================================

	Routine Description:
		Process TX Rings DMA Done interrupt, running in DPC level

	Arguments:
		Adapter 	Pointer to our adapter

	Return Value:
		None

	IRQL = DISPATCH_LEVEL
	
	========================================================================
*/
BOOLEAN	RTMPHandleTxRingDmaDoneInterrupt(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_TX_DONE_MASK tx_mask)
{
	BOOLEAN bReschedule = FALSE;


	/* Dequeue outgoing frames from TxSwQueue[] and process it*/
	RTMPDeQueuePacket(pAd, FALSE, NUM_OF_TX_RING, MAX_TX_PROCESS);

	return  bReschedule;
}


/*
	========================================================================

	Routine Description:
		Process MGMT ring DMA done interrupt, running in DPC level

	Arguments:
		pAd 	Pointer to our adapter

	Return Value:
		None

	IRQL = DISPATCH_LEVEL
	
	Note:

	========================================================================
*/
VOID RTMPHandleMgmtRingDmaDoneInterrupt(RTMP_ADAPTER *pAd)
{
	PTXD_STRUC	 pTxD;
#ifdef RT_BIG_ENDIAN
    PTXD_STRUC      pDestTxD;
	UCHAR tx_hw_info[TXD_SIZE];
#endif
	PNDIS_PACKET pPacket;
/*	int 		 i;*/
	UCHAR	FREE = 0;
	PRTMP_MGMT_RING pMgmtRing = &pAd->MgmtRing;
	UINT8 TXWISize = pAd->chipCap.TXWISize;

	NdisAcquireSpinLock(&pAd->MgmtRingLock);
	
	RTMP_IO_READ32(pAd, pMgmtRing->hw_didx_addr, &pMgmtRing->TxDmaIdx);
	while (pMgmtRing->TxSwFreeIdx!= pMgmtRing->TxDmaIdx)
	{
		RTMP_DMACB *dma_cb = &pMgmtRing->Cell[pMgmtRing->TxSwFreeIdx];
		
		FREE++;
#ifdef RT_BIG_ENDIAN
		pDestTxD = (PTXD_STRUC) (dma_cb->AllocVa);
		NdisMoveMemory(&tx_hw_info[0], pDestTxD, TXD_SIZE);
		pTxD = (PTXD_STRUC)&tx_hw_info[0];
		RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#else
		pTxD = (PTXD_STRUC) (dma_cb->AllocVa);
#endif

		//pTxD->DMADONE = 0;
		pPacket = dma_cb->pNdisPacket;
		if (pPacket == NULL)
		{
			INC_RING_INDEX(pMgmtRing->TxSwFreeIdx, MGMT_RING_SIZE);
			continue;
		}

#define LMR_FRAME_GET()	(GET_OS_PKT_DATAPTR(pPacket) + TXWISize)

#ifdef UAPSD_SUPPORT
#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
		UAPSD_QoSNullTxMgmtTxDoneHandle(pAd,
					pPacket,
					LMR_FRAME_GET());
#else
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			UAPSD_QoSNullTxMgmtTxDoneHandle(pAd,
					pPacket, LMR_FRAME_GET());
		}
#endif /* CONFIG_AP_SUPPORT */
#endif /* defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) */
#endif /* UAPSD_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */

#ifdef RT_CFG80211_SUPPORT
		if (pPacket)
		{	
			HEADER_802_11  *pHeader;
			pHeader = (HEADER_802_11 *)(GET_OS_PKT_DATAPTR(pPacket)+ TXINFO_SIZE + pAd->chipCap.TXWISize);
			CFG80211_SendMgmtFrameDone(pAd, pHeader->Sequence); 
		}	
#endif /* RT_CFG80211_SUPPORT */

		if (pPacket)
		{
			dma_cb->pNdisPacket = NULL;
			PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr0, pTxD->SDLen0, RTMP_PCI_DMA_TODEVICE);
			RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
		}

		pPacket = dma_cb->pNextNdisPacket;
		if (pPacket)
		{
			dma_cb->pNextNdisPacket = NULL;
			PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr1, pTxD->SDLen1, RTMP_PCI_DMA_TODEVICE);
			RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
		}

		/* flush dcache if no consistent memory is supported */
		RTMP_DCACHE_FLUSH(dma_cb->AllocPa, TXD_SIZE);

		INC_RING_INDEX(pMgmtRing->TxSwFreeIdx, MGMT_RING_SIZE);

#ifdef RT_BIG_ENDIAN
		/* TxD no change. */
		//RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
		//WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, TRUE, TYPE_TXD);
#endif
	}
	NdisReleaseSpinLock(&pAd->MgmtRingLock);

}


/*
	========================================================================

	Routine Description:
	Arguments:
		Adapter 	Pointer to our adapter. Dequeue all power safe delayed braodcast frames after beacon.

	IRQL = DISPATCH_LEVEL
	
	========================================================================
*/
VOID RTMPHandleTBTTInterrupt(RTMP_ADAPTER *pAd)
{
#ifdef CONFIG_AP_SUPPORT
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;

	if (pAd->OpMode == OPMODE_AP)
	{
		ReSyncBeaconTime(pAd);

		RTMP_OS_TASKLET_SCHE(&pObj->tbtt_task);
#ifdef CUSTOMER_DCC_FEATURE
		if(pAd->CommonCfg.channelSwitch.CHSWMode == CHANNEL_SWITCHING_MODE)
		{
			DBGPRINT(RT_DEBUG_TRACE, ("RTMPHandlePreTBTTInterrupt::Channel Switching...(%d/%d)\n", pAd->CommonCfg.channelSwitch.CHSWCount, pAd->CommonCfg.channelSwitch.CHSWPeriod));

			pAd->CommonCfg.channelSwitch.CHSWCount++;
			if(pAd->CommonCfg.channelSwitch.CHSWCount >= pAd->CommonCfg.channelSwitch.CHSWPeriod)
			{
				APStop(pAd);
				APStartUp(pAd);
			}
		}
#endif

		if ((pAd->CommonCfg.Channel > 14)
			&& (pAd->CommonCfg.bIEEE80211H == 1)
			&& (pAd->Dot11_H.RDMode == RD_SWITCHING_MODE))
		{
			ChannelSwitchingCountDownProc(pAd);
		}
	}
	else
#endif /* CONFIG_AP_SUPPORT */
	{
		if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE))
		{
		}
	}
}


/*
	========================================================================

	Routine Description:
	Arguments:
		pAd 		Pointer to our adapter. Rewrite beacon content before next send-out.

	IRQL = DISPATCH_LEVEL
	
	========================================================================
*/
VOID RTMPHandlePreTBTTInterrupt(RTMP_ADAPTER *pAd)
{
#ifdef CONFIG_AP_SUPPORT
	if (pAd->OpMode == OPMODE_AP  
#ifdef RT_CFG80211_SUPPORT
		&& (pAd->ApCfg.MBSSID[0].wdev.Hostapd == Hostapd_CFG)
#endif /*RT_CFG80211_SUPPORT*/
	)
	{
		POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;


#ifdef RT_CFG80211_SUPPORT
			RT_CFG80211_BEACON_TIM_UPDATE(pAd);
#else
			APUpdateAllBeaconFrame(pAd);
#endif /* RT_CFG80211_SUPPORT  */

		if (pAd->CommonCfg.bMcastTest == TRUE) {
#ifdef WORKQUEUE_BH
			RTMP_OS_TASKLET_SCHE(&pObj->pretbtt_work);
#else
			RTMP_OS_TASKLET_SCHE(&pObj->pretbtt_task);
#endif /* WORKQUEUE_BH */
		}
	}
	else
#endif /* CONFIG_AP_SUPPORT */
	{
		if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE))
		{
			DBGPRINT(RT_DEBUG_TRACE, ("%s()...\n", __FUNCTION__));
		}
	}

}


VOID RTMPHandleRxCoherentInterrupt(RTMP_ADAPTER *pAd)
{
	WPDMA_GLO_CFG_STRUC GloCfg;

	if (pAd == NULL)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("====> pAd is NULL, return.\n"));
		return;
	}
	
	DBGPRINT(RT_DEBUG_TRACE, ("==> RTMPHandleRxCoherentInterrupt \n"));
	
	RTMP_IO_READ32(pAd, WPDMA_GLO_CFG , &GloCfg.word);
	GloCfg.field.EnTXWriteBackDDONE = 0;
	GloCfg.field.EnableRxDMA = 0;
	GloCfg.field.EnableTxDMA = 0;
	RTMP_IO_WRITE32(pAd, WPDMA_GLO_CFG, GloCfg.word);

	RTMPRingCleanUp(pAd, QID_AC_BE);
	RTMPRingCleanUp(pAd, QID_AC_BK);
	RTMPRingCleanUp(pAd, QID_AC_VI);
	RTMPRingCleanUp(pAd, QID_AC_VO);
	RTMPRingCleanUp(pAd, QID_HCCA);
	RTMPRingCleanUp(pAd, QID_MGMT);
	RTMPRingCleanUp(pAd, QID_RX);

	RTMPEnableRxTx(pAd);
	
	DBGPRINT(RT_DEBUG_TRACE, ("<== RTMPHandleRxCoherentInterrupt \n"));
}


#ifdef CONFIG_AP_SUPPORT
VOID RTMPHandleMcuInterrupt(RTMP_ADAPTER *pAd)
{
	UINT32 McuIntSrc = 0;
	RTMP_IO_READ32(pAd, 0x7024, &McuIntSrc);

	/* check mac 0x7024 */
#ifdef CARRIER_DETECTION_SUPPORT
	if (pAd->infType == RTMP_DEV_INF_PCIE &&
		(McuIntSrc & (1<<1)) && /*bit_1: carr_status interrupt */
		(pAd->CommonCfg.CarrierDetect.Enable == TRUE))
	{
		RTMPHandleRadarInterrupt(pAd);
	}
#endif /* CARRIER_DETECTION_SUPPORT */

	/* clear MCU Int source register.*/
	RTMP_IO_WRITE32(pAd, 0x7024, 0);

}
#endif /* CONFIG_AP_SUPPORT */


PNDIS_PACKET GetPacketFromRxRing(
	IN RTMP_ADAPTER *pAd,
	OUT RX_BLK *pRxBlk,
	OUT BOOLEAN *pbReschedule,
	INOUT UINT32 *pRxPending,
	BOOLEAN *bCmdRspPacket,
	UCHAR RxRingNo)
{
	RXD_STRUC *pRxD;
#ifdef RT_BIG_ENDIAN
	RXD_STRUC *pDestRxD;
	UCHAR rx_hw_info[RXD_SIZE];
	RXINFO_STRUC *pRxInfo;	
	RXINFO_STRUC RxInfo;
#endif
	RTMP_RX_RING *pRxRing;
	NDIS_SPIN_LOCK *pRxRingLock;
	PNDIS_PACKET pRxPacket = NULL, pNewPacket;
	VOID *AllocVa;
	NDIS_PHYSICAL_ADDRESS AllocPa;
	BOOLEAN bReschedule = FALSE;
	RTMP_DMACB *pRxCell;
	UINT8 RXWISize = pAd->chipCap.RXWISize;
	UINT16 RxRingSize = (RxRingNo == 0) ? RX_RING_SIZE : RX1_RING_SIZE;

	pRxRing = &pAd->RxRing[RxRingNo];
	pRxRingLock = &pAd->RxRingLock[RxRingNo];
	RTMP_SEM_LOCK(pRxRingLock);

	if (*pRxPending == 0)
	{
		/* Get how may packets had been received*/
		RTMP_IO_READ32(pAd, pRxRing->hw_didx_addr, &pRxRing->RxDmaIdx);
		
		if (pRxRing->RxSwReadIdx == pRxRing->RxDmaIdx)
		{
			bReschedule = FALSE;
			goto done;
		}

		/* get rx pending count*/
		if (pRxRing->RxDmaIdx > pRxRing->RxSwReadIdx)
			*pRxPending = pRxRing->RxDmaIdx - pRxRing->RxSwReadIdx;
		else
			*pRxPending = pRxRing->RxDmaIdx + RxRingSize - pRxRing->RxSwReadIdx;
	}

	pRxCell = &pRxRing->Cell[pRxRing->RxSwReadIdx];

	/* flush dcache if no consistent memory is supported */
	RTMP_DCACHE_FLUSH(pRxCell->AllocPa, RXD_SIZE);

#ifdef RT_BIG_ENDIAN
	pDestRxD = (RXD_STRUC *)pRxCell->AllocVa;
	/* RxD = *pDestRxD; */
	NdisMoveMemory(&rx_hw_info[0], pDestRxD, RXD_SIZE);
	pRxD = (RXD_STRUC *)&rx_hw_info[0];
	RTMPDescriptorEndianChange((PUCHAR)pRxD, TYPE_RXD);
#else
	/* Point to Rx indexed rx ring descriptor*/
	pRxD = (PRXD_STRUC) pRxCell->AllocVa;
#endif

	if (pRxD->DDONE == 0)
	{
		*pRxPending = 0;
		DBGPRINT(RT_DEBUG_INFO, ("DDONE=0!\n"));
		/* DMAIndx had done but DDONE bit not ready*/
		bReschedule = TRUE;
		goto done;
	}

	/* return rx descriptor*/
	NdisMoveMemory(&pRxBlk->hw_rx_info[0], pRxD, RXD_SIZE);

#ifdef WLAN_SKB_RECYCLE
	{
		struct sk_buff *skb = __skb_dequeue_tail(&pAd->rx0_recycle);

		if (unlikely(skb==NULL))
			pNewPacket = RTMP_AllocateRxPacketBuffer(pAd, ((POS_COOKIE)(pAd->OS_Cookie))->pci_dev, RX_BUFFER_AGGRESIZE, FALSE, &AllocVa, &AllocPa);
		else
		{
			pNewPacket = OSPKT_TO_RTPKT(skb);
			AllocVa = GET_OS_PKT_DATAPTR(pNewPacket);
			AllocPa = PCI_MAP_SINGLE_DEV(((POS_COOKIE)(pAd->OS_Cookie))->pci_dev, AllocVa, RX_BUFFER_AGGRESIZE,  -1, RTMP_PCI_DMA_FROMDEVICE);
		}
	}
#else
	pNewPacket = RTMP_AllocateRxPacketBuffer(pAd, ((POS_COOKIE)(pAd->OS_Cookie))->pci_dev, RX_BUFFER_AGGRESIZE, FALSE, &AllocVa, &AllocPa);
#endif /* WLAN_SKB_RECYCLE */

	if (pNewPacket)
	{
		/* unmap the rx buffer*/
		PCI_UNMAP_SINGLE(pAd, pRxCell->DmaBuf.AllocPa,
					 pRxCell->DmaBuf.AllocSize, RTMP_PCI_DMA_FROMDEVICE);
		/* flush dcache if no consistent memory is supported */
		RTMP_DCACHE_FLUSH(pRxCell->DmaBuf.AllocPa, pRxCell->DmaBuf.AllocSize);

		pRxPacket = pRxCell->pNdisPacket;
#ifdef RT_SECURE_DMA
		/* Copy the header from secure buffer to packet */
		pRxBlk->pRxInfo = (RXINFO_STRUC *)GET_OS_PKT_DATAPTR(pRxPacket);
		NdisMoveMemory(GET_OS_PKT_DATAPTR(pRxPacket),
					   pAd->RxSecureDMA[RxRingNo].AllocVa + (pRxRing->RxSwReadIdx * 4096),
					   RXINFO_SIZE + sizeof(struct _RXWI_NMAC));
#endif
#ifdef RLT_MAC
		/* get rx descriptor and data buffer */
		if (pAd->chipCap.hif_type == HIF_RLT)
		{
			struct _RXWI_NMAC *rxwi_n;

			pRxBlk->pRxFceInfo = (RXFCE_INFO *)&pRxBlk->hw_rx_info[RXINFO_OFFSET];
			pRxBlk->pRxInfo = (RXINFO_STRUC *)GET_OS_PKT_DATAPTR(pRxPacket);
#ifdef RT_BIG_ENDIAN
			NdisMoveMemory(&RxInfo, pRxBlk->pRxInfo, RXINFO_SIZE);
			pRxInfo = &RxInfo;
			(*(UINT32*)pRxInfo) = le2cpu32(*(UINT32*)pRxInfo);
			NdisMoveMemory(pRxBlk->pRxInfo, pRxInfo, RXINFO_SIZE);
#endif /* RT_BIG_ENDIAN */

			SET_OS_PKT_DATAPTR(pRxPacket, GET_OS_PKT_DATAPTR(pRxPacket) + RXINFO_SIZE);
			SET_OS_PKT_LEN(pRxPacket, GET_OS_PKT_LEN(pRxPacket) - RXINFO_SIZE);
			rxwi_n = (struct _RXWI_NMAC *)(GET_OS_PKT_DATAPTR(pRxPacket));
			pRxBlk->pRxWI = (RXWI_STRUC *)(GET_OS_PKT_DATAPTR(pRxPacket));
#ifdef RT_BIG_ENDIAN		
			RTMPWIEndianChange(pAd , (PUCHAR)rxwi_n, TYPE_RXWI);
#endif			
			pRxBlk->MPDUtotalByteCnt = rxwi_n->MPDUtotalByteCnt;
			pRxBlk->wcid = rxwi_n->wcid;
			pRxBlk->key_idx = rxwi_n->key_idx;
			pRxBlk->bss_idx = rxwi_n->bss_idx;
			pRxBlk->TID = rxwi_n->tid;
			pRxBlk->DataSize = rxwi_n->MPDUtotalByteCnt;
			
			pRxBlk->rx_rate.field.MODE = rxwi_n->phy_mode;
			pRxBlk->rx_rate.field.MCS = rxwi_n->mcs;
			pRxBlk->rx_rate.field.ldpc = rxwi_n->ldpc;
			pRxBlk->rx_rate.field.BW = rxwi_n->bw;
			pRxBlk->rx_rate.field.STBC = rxwi_n->stbc;
			pRxBlk->rx_rate.field.ShortGI = rxwi_n->sgi;
			pRxBlk->rssi[0] = rxwi_n->rssi[0];
			pRxBlk->rssi[1] = rxwi_n->rssi[1];
			pRxBlk->rssi[2] = rxwi_n->rssi[2];
			pRxBlk->snr[0] = rxwi_n->bbp_rxinfo[0];
			pRxBlk->snr[1] = rxwi_n->bbp_rxinfo[1];
			pRxBlk->snr[2] = rxwi_n->bbp_rxinfo[2];
			pRxBlk->freq_offset = rxwi_n->bbp_rxinfo[4];
			pRxBlk->ldpc_ex_sym = rxwi_n->ldpc_ex_sym;
		}
#endif /* RLT_MAC */
#ifdef RTMP_MAC
		if (pAd->chipCap.hif_type == HIF_RTMP) 
		{
			struct _RXWI_OMAC *rxwi_o = (struct _RXWI_OMAC *)(GET_OS_PKT_DATAPTR(pRxPacket));

			pRxBlk->pRxInfo = (RXINFO_STRUC *)(&pRxBlk->hw_rx_info[RXINFO_OFFSET]);
			pRxBlk->pRxWI = (RXWI_STRUC *)rxwi_o;

			pRxBlk->MPDUtotalByteCnt = rxwi_o->MPDUtotalByteCnt;
			pRxBlk->wcid = rxwi_o->wcid;
			pRxBlk->key_idx = rxwi_o->key_idx;
			pRxBlk->bss_idx = rxwi_o->bss_idx;
			pRxBlk->TID = rxwi_o->tid;
			pRxBlk->DataSize = rxwi_o->MPDUtotalByteCnt;
			
			pRxBlk->rx_rate.field.MODE = rxwi_o->phy_mode;
			pRxBlk->rx_rate.field.MCS = rxwi_o->mcs;
			pRxBlk->rx_rate.field.ldpc = 0;
			pRxBlk->rx_rate.field.BW = rxwi_o->bw;
			pRxBlk->rx_rate.field.STBC = rxwi_o->stbc;
			pRxBlk->rx_rate.field.ShortGI = rxwi_o->sgi;
			pRxBlk->rssi[0] = rxwi_o->RSSI0;
			pRxBlk->rssi[1] = rxwi_o->RSSI1;
			pRxBlk->rssi[2] = rxwi_o->RSSI2;
			pRxBlk->snr[0] = rxwi_o->SNR0;
			pRxBlk->snr[1] = rxwi_o->SNR1;
			pRxBlk->snr[2] = rxwi_o->SNR2;
			pRxBlk->freq_offset = rxwi_o->FOFFSET;
		}
#endif /* RTMP_MAC */

#ifdef RT_SECURE_DMA
		/* Copy rest of packet data - Data ptr has been shifted by 4 -> rxinfo size. Length needs 4 added, or data missing */
		NdisMoveMemory(GET_OS_PKT_DATAPTR(pRxPacket),
					   pAd->RxSecureDMA[RxRingNo].AllocVa + (pRxRing->RxSwReadIdx * 4096) + RXINFO_SIZE,
					   sizeof(struct _RXWI_NMAC) + pRxBlk->DataSize + 4);
#endif
		pRxBlk->pRxPacket = pRxPacket;
		pRxBlk->pData = (UCHAR *)GET_OS_PKT_DATAPTR(pRxPacket);
		pRxBlk->pHeader = (HEADER_802_11 *)(pRxBlk->pData + RXWISize);
		pRxBlk->Flags = 0;

		pRxCell->DmaBuf.AllocSize = RX_BUFFER_AGGRESIZE;
		pRxCell->pNdisPacket = (PNDIS_PACKET) pNewPacket;
		pRxCell->DmaBuf.AllocVa = AllocVa;
		pRxCell->DmaBuf.AllocPa = AllocPa;

		/* flush dcache if no consistent memory is supported */
		RTMP_DCACHE_FLUSH(pRxCell->DmaBuf.AllocPa, pRxCell->DmaBuf.AllocSize);

		/* update SDP0 to new buffer of rx packet */
#ifndef RT_SECURE_DMA
		pRxD->SDP0 = AllocPa;
#else
		pRxD->SDP0 = pAd->RxSecureDMA[RxRingNo].AllocPa + (pRxRing->RxSwReadIdx * 4096);
#endif
		pRxD->SDL0 = RX_BUFFER_AGGRESIZE;
	}
	else 
	{
		pRxPacket = NULL;
		bReschedule = TRUE;
	}

	*pRxPending = *pRxPending - 1;	

#ifndef CACHE_LINE_32B
	pRxD->DDONE = 0;

	/* update rx descriptor and kick rx */
#ifdef RT_BIG_ENDIAN
	RTMPDescriptorEndianChange((PUCHAR)pRxD, TYPE_RXD);
	WriteBackToDescriptor((PUCHAR)pDestRxD, (PUCHAR)pRxD, FALSE, TYPE_RXD);
#endif

	INC_RING_INDEX(pRxRing->RxSwReadIdx, RxRingSize);

	pRxRing->RxCpuIdx = (pRxRing->RxSwReadIdx == 0) ? (RxRingSize-1) : (pRxRing->RxSwReadIdx-1);
	
	RTMP_IO_WRITE32(pAd, pRxRing->hw_cidx_addr, pRxRing->RxCpuIdx);

#else /* CACHE_LINE_32B */

	/*
		Because our RXD_SIZE is 16B, but if the cache line size is 32B, we
		will suffer a problem as below:

		1. We flush RXD 0, start address of RXD 0 is 32B-align.
			Nothing occurs.
		2. We flush RXD 1, start address of RXD 1 is 16B-align.
			Because cache line size is 32B, cache must flush 32B, cannot flush
			16B only, so RXD0 and RXD1 will be flushed.
			But when traffic is busy, maybe RXD0 is updated by MAC, i.e.
			DDONE bit is 1, so when the cache flushs RXD0, the DDONE bit will
			be cleared to 0.
		3. Then when we handle RXD0 in the future, we will find the DDONE bit
			is 0 and we will wait for MAC to set it to 1 forever.
	*/
	if (pRxRing->RxSwReadIdx & 0x01)
	{
		RTMP_DMACB *pRxCellLast;
#ifdef RT_BIG_ENDIAN
		PRXD_STRUC pDestRxDLast;
#endif
		/* 16B-align */

		/* update last BD 32B-align, DMA Done bit = 0 */
		pRxCell->LastBDInfo.DDONE = 0;
#ifdef RT_BIG_ENDIAN
		pRxCellLast = &pRxRing->Cell[pRxRing->RxSwReadIdx - 1];
		pDestRxDLast = (PRXD_STRUC) pRxCellLast->AllocVa;
		RTMPDescriptorEndianChange((PUCHAR)&pRxCell->LastBDInfo, TYPE_RXD);
		WriteBackToDescriptor((UCHAR *)pDestRxDLast, (UCHAR *)&pRxCell->LastBDInfo, FALSE, TYPE_RXD);
#endif

		/* update current BD 16B-align, DMA Done bit = 0 */
		pRxD->DDONE = 0;
#ifdef RT_BIG_ENDIAN
		RTMPDescriptorEndianChange((PUCHAR)pRxD, TYPE_RXD);
		WriteBackToDescriptor((PUCHAR)pDestRxD, (PUCHAR)pRxD, FALSE, TYPE_RXD);
#endif

		/* flush cache from last BD */
		RTMP_DCACHE_FLUSH(pRxCellLast->AllocPa, 32); /* use RXD_SIZE should be OK */

		/* update SW read and CPU index */
		INC_RING_INDEX(pRxRing->RxSwReadIdx, RxRingSize);
		pRxRing->RxCpuIdx = (pRxRing->RxSwReadIdx == 0) ? (RxRingSize-1) : (pRxRing->RxSwReadIdx-1);
		RTMP_IO_WRITE32(pAd, pRxRing->hw_cidx_addr, pRxRing->RxCpuIdx);
	}
	else
	{
		/* 32B-align */
		/* do not set DDONE bit and backup it */
		if (pRxRing->RxSwReadIdx >= (RxRingSize-1))
		{
			DBGPRINT(RT_DEBUG_TRACE,
					("Please change RX_RING_SIZE to mutiple of 2!\n"));

			/* flush cache from current BD */
			RTMP_DCACHE_FLUSH(pRxCell->AllocPa, RXD_SIZE);

			/* update SW read and CPU index */
			INC_RING_INDEX(pRxRing->RxSwReadIdx, RxRingSize);
			pRxRing->RxCpuIdx = (pRxRing->RxSwReadIdx == 0) ? (RxRingSize-1) : (pRxRing->RxSwReadIdx-1);
			RTMP_IO_WRITE32(pAd, pRxRing->hw_cidx_addr, pRxRing->RxCpuIdx);
		}
		else
		{
			/* backup current BD */
			pRxCell = &pRxRing->Cell[pRxRing->RxSwReadIdx + 1];
			pRxCell->LastBDInfo = *pRxD;

			/* update CPU index */
			INC_RING_INDEX(pRxRing->RxSwReadIdx, RxRingSize);
		}
	}
#endif /* CACHE_LINE_32B */

done:
	RTMP_SEM_UNLOCK(pRxRingLock);
	*pbReschedule = bReschedule;
	return pRxPacket;
}


NDIS_STATUS MlmeHardTransmitTxRing(RTMP_ADAPTER *pAd, UCHAR QueIdx, PNDIS_PACKET pPacket)
{
	PACKET_INFO PacketInfo;
	UCHAR *pSrcBufVA;
	UINT SrcBufLen;
	TXD_STRUC *pTxD;
	TXINFO_STRUC *pTxInfo;
#ifdef RT_BIG_ENDIAN
	TXD_STRUC *pDestTxD;
	UCHAR hw_hdr_info[TXD_SIZE];
#endif
	PHEADER_802_11 pHeader_802_11;
	BOOLEAN bAckRequired, bInsertTimestamp;
	ULONG SrcBufPA;
	UCHAR MlmeRate, wcid, tx_rate;
	UINT32 SwIdx;
	TXWI_STRUC *pFirstTxWI;
	ULONG FreeNum;
	MAC_TABLE_ENTRY *pMacEntry = NULL;
	UINT8 TXWISize = pAd->chipCap.TXWISize;
	HTTRANSMIT_SETTING *transmit;
#ifdef CONFIG_AP_SUPPORT
#ifdef SPECIFIC_TX_POWER_SUPPORT
	UCHAR TxPwrAdj = 0;
#endif /* SPECIFIC_TX_POWER_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

	RTMP_QueryPacketInfo(pPacket, &PacketInfo, &pSrcBufVA, &SrcBufLen);
	if (pSrcBufVA == NULL)
		return NDIS_STATUS_FAILURE;

	FreeNum = GET_TXRING_FREENO(pAd, QueIdx);
	if (FreeNum == 0)
		return NDIS_STATUS_FAILURE;

	SwIdx = pAd->TxRing[QueIdx].TxCpuIdx;
#ifdef RT_BIG_ENDIAN
	pDestTxD  = (PTXD_STRUC)pAd->TxRing[QueIdx].Cell[SwIdx].AllocVa;
	NdisMoveMemory(&hw_hdr_info[0], pDestTxD, TXD_SIZE);
	pTxD = (TXD_STRUC *)&hw_hdr_info[0];
	RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#else
	pTxD  = (PTXD_STRUC) pAd->TxRing[QueIdx].Cell[SwIdx].AllocVa;
#endif
	pTxInfo = (TXINFO_STRUC *)((UCHAR *)pTxD + sizeof(TXD_STRUC));

	if (pAd->TxRing[QueIdx].Cell[SwIdx].pNdisPacket)
	{
		DBGPRINT(RT_DEBUG_OFF, ("MlmeHardTransmit Error\n"));
		return NDIS_STATUS_FAILURE;
	}


#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		/* outgoing frame always wakeup PHY to prevent frame lost*/
		/* if (pAd->StaCfg.Psm == PWR_SAVE)*/
		if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE))
			AsicForceWakeup(pAd, TRUE);
	}
#endif /* CONFIG_STA_SUPPORT */
	
	pFirstTxWI =(TXWI_STRUC *)(pSrcBufVA + TXINFO_SIZE);
	pHeader_802_11 = (HEADER_802_11 *)(pSrcBufVA + TXINFO_SIZE + TXWISize + TSO_SIZE);
	if (pHeader_802_11->Addr1[0] & 0x01)
		MlmeRate = pAd->CommonCfg.BasicMlmeRate;
	else
		MlmeRate = pAd->CommonCfg.MlmeRate;
	
	if ((pHeader_802_11->FC.Type == FC_TYPE_DATA) &&
		(pHeader_802_11->FC.SubType == SUBTYPE_QOS_NULL))
	{
		pMacEntry = MacTableLookup(pAd, pHeader_802_11->Addr1);
	}

	/* Verify Mlme rate for a/g bands.*/
	if ((pAd->LatchRfRegs.Channel > 14) && (MlmeRate < RATE_6)) /* 11A band*/
		MlmeRate = RATE_6;

	/*
		Should not be hard code to set PwrMgmt to 0 (PWR_ACTIVE)
		Snice it's been set to 0 while on MgtMacHeaderInit
		By the way this will cause frame to be send on PWR_SAVE failed.
	*/
	
	/* In WMM-UAPSD, mlme frame should be set psm as power saving but probe request frame */
#ifdef CONFIG_STA_SUPPORT
	/* Data-Null packets alse pass through MMRequest in RT2860, however, we hope control the psm bit to pass APSD*/
	if (pHeader_802_11->FC.Type != FC_TYPE_DATA)
	{
		if ((pHeader_802_11->FC.SubType == SUBTYPE_PROBE_REQ) ||
			!(pAd->StaCfg.UapsdInfo.bAPSDCapable && pAd->CommonCfg.APEdcaParm.bAPSDCapable))
			pHeader_802_11->FC.PwrMgmt = PWR_ACTIVE;
		else
			pHeader_802_11->FC.PwrMgmt = pAd->CommonCfg.bAPSDForcePowerSave;
	}
#endif /* CONFIG_STA_SUPPORT */
	
	bInsertTimestamp = FALSE;
	if (pHeader_802_11->FC.Type == FC_TYPE_CNTL) /* must be PS-POLL */
	{
		bAckRequired = FALSE;
#ifdef VHT_TXBF_SUPPORT
		if (pHeader_802_11->FC.SubType == SUBTYPE_VHT_NDPA)
			pHeader_802_11->Duration = RTMPCalcDuration(pAd, MlmeRate, (SrcBufLen - TXINFO_SIZE - TXWISize - TSO_SIZE));
#endif /* VHT_TXBF_SUPPORT*/
	}
	else /* FC_TYPE_MGMT or FC_TYPE_DATA(must be NULL frame)*/
	{
		if (pHeader_802_11->Addr1[0] & 0x01) /* MULTICAST, BROADCAST */
		{
			bAckRequired = FALSE;
			pHeader_802_11->Duration = 0;
		}
		else
		{
			bAckRequired = TRUE;
			pHeader_802_11->Duration = RTMPCalcDuration(pAd, MlmeRate, 14);
			if (pHeader_802_11->FC.SubType == SUBTYPE_PROBE_RSP)
			{
				bInsertTimestamp = TRUE;
#ifdef CONFIG_AP_SUPPORT
#ifdef SPECIFIC_TX_POWER_SUPPORT
				{
					/* Find which MBSSID to be send this probeRsp */
					UINT32 apidx = get_apidx_by_addr(pAd, pHeader_802_11->Addr2);

					if (!(apidx >= pAd->ApCfg.BssidNum) &&
					     (pAd->ApCfg.MBSSID[apidx].TxPwrAdj != -1) /* &&
					     (pAd->CommonCfg.MlmeTransmit.field.MODE == MODE_CCK) &&
					     (pAd->CommonCfg.MlmeTransmit.field.MCS == RATE_1)*/)
					{
						TxPwrAdj = pAd->ApCfg.MBSSID[apidx].TxPwrAdj;
					}
				}
#endif /* SPECIFIC_TX_POWER_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
			}
		}
	}

	pHeader_802_11->Sequence = pAd->Sequence++;
	if (pAd->Sequence > 0xfff)
		pAd->Sequence = 0;

	/* Before radar detection done, mgmt frame can not be sent but probe req*/
	/* Because we need to use probe req to trigger driver to send probe req in passive scan*/
	if ((pHeader_802_11->FC.SubType != SUBTYPE_PROBE_REQ)
		&& (pAd->CommonCfg.bIEEE80211H == 1)
		&& (pAd->Dot11_H.RDMode != RD_NORMAL_MODE))
	{
		DBGPRINT(RT_DEBUG_ERROR,("MlmeHardTransmit --> radar detect not in normal mode !!!\n"));
		return (NDIS_STATUS_FAILURE);
	}

#ifdef RT_BIG_ENDIAN
	RTMPFrameEndianChange(pAd, (PUCHAR)pHeader_802_11, DIR_WRITE, FALSE);
#endif
	
	/*
		Fill scatter-and-gather buffer list into TXD. Internally created NDIS PACKET
		should always has only one ohysical buffer, and the whole frame size equals
		to the first scatter buffer size
	*/
	

	/*
		Initialize TX Descriptor
		For inter-frame gap, the number is for this frame and next frame
		For MLME rate, we will fix as 2Mb to match other vendor's implement
	*/
/*	pAd->CommonCfg.MlmeTransmit.field.MODE = 1;*/
	
/* management frame doesn't need encryption. so use RESERVED_WCID no matter u are sending to specific wcid or not */
	/* Only beacon use Nseq=TRUE. So here we use Nseq=FALSE.*/
	if (pMacEntry == NULL)
	{
		wcid = RESERVED_WCID;
		tx_rate = (UCHAR)pAd->CommonCfg.MlmeTransmit.field.MCS;
		transmit = &pAd->CommonCfg.MlmeTransmit;
	}
	else
	{
		wcid = pMacEntry->Aid;
		tx_rate = (UCHAR)pMacEntry->MaxHTPhyMode.field.MCS;
		transmit = &pMacEntry->MaxHTPhyMode;
	}

	RTMPWriteTxWI(pAd, pFirstTxWI, FALSE, FALSE, bInsertTimestamp, FALSE, bAckRequired, FALSE,
					0, wcid, (SrcBufLen - TXINFO_SIZE - TXWISize - TSO_SIZE), PID_MGMT, 0,
					tx_rate, IFS_BACKOFF, transmit);

#ifdef SPECIFIC_TX_POWER_SUPPORT
#ifdef RTMP_MAC
		if ((IS_RT6352(pAd) || IS_MT76x2(pAd)) &&
			(pAd->chipCap.hif_type == HIF_RTMP) && (pMacEntry == NULL))
        	pFirstTxWI->TXWI_O.TxPwrAdj = TxPwrAdj;
#endif /* RTMP_MAC */
#ifdef RLT_MAC
		if ((IS_RT6352(pAd) || IS_MT76x2(pAd)) &&
			(pAd->chipCap.hif_type == HIF_RLT) && (pMacEntry == NULL))
			pFirstTxWI->TXWI_N.TxPwrAdj = TxPwrAdj;
#endif /* RLT_MAC */	
#endif /* SPECIFIC_TX_POWER_SUPPORT */

	pAd->TxRing[QueIdx].Cell[SwIdx].pNdisPacket = pPacket;
	pAd->TxRing[QueIdx].Cell[SwIdx].pNextNdisPacket = NULL;
#ifdef RT_BIG_ENDIAN
	RTMPWIEndianChange(pAd, (PUCHAR)pFirstTxWI, TYPE_TXWI);
#endif
#ifndef RT_SECURE_DMA
	SrcBufPA = PCI_MAP_SINGLE(pAd, (pSrcBufVA + TXINFO_SIZE), (SrcBufLen - TXINFO_SIZE), 0, RTMP_PCI_DMA_TODEVICE);
#else
	NdisMoveMemory(pAd->TxSecureDMA[QueIdx].AllocVa + (SwIdx * 4096), pSrcBufVA + TXINFO_SIZE, SrcBufLen);
	SrcBufPA = pAd->TxSecureDMA[QueIdx].AllocPa + (SwIdx * 4096);
#endif
#ifdef CUSTOMER_DCC_FEATURE
	if(!(ApScanRunning(pAd)))
	{
		UINT32 Index, Length;
		HTTRANSMIT_SETTING HTSetting;
		MULTISSID_STRUCT *pMbss = NULL;

		NdisZeroMemory(&HTSetting, sizeof(HTTRANSMIT_SETTING));
		HTSetting.field.MODE = transmit->field.MODE;
		HTSetting.field.BW = transmit->field.BW;
		HTSetting.field.ShortGI = transmit->field.ShortGI;
		HTSetting.field.MCS = transmit->field.MCS;
		Length = SrcBufLen - TXWISize;
		if (pMacEntry != NULL && (pMacEntry->apidx < pAd->ApCfg.BssidNum))
			pMbss = &pAd->ApCfg.MBSSID[pMacEntry->apidx];
		else
		{
			
			UCHAR apidx;
			for(apidx=0; apidx<pAd->ApCfg.BssidNum; apidx++)
			{
				if ((pAd->ApCfg.MBSSID[apidx].wdev.if_dev == NULL) || ((pAd->ApCfg.MBSSID[apidx].wdev.if_dev != NULL) &&
					!(RTMP_OS_NETDEV_STATE_RUNNING(pAd->ApCfg.MBSSID[apidx].wdev.if_dev))))
				{
					/* the interface is down */
					continue;
				}
				if(RTMPEqualMemory(pAd->ApCfg.MBSSID[apidx].wdev.bssid, pHeader_802_11->Addr2,MAC_ADDR_LEN))
				{
					pMbss = &pAd->ApCfg.MBSSID[apidx];
				}
			}
		}
		GetMultShiftFactorIndex(HTSetting, &Index);
		RTMPCalculateAPTxRxActivityTime(pAd, Index, Length, pMbss, pMacEntry);
	}
#endif

	pTxD->LastSec0 = 1;
	pTxD->LastSec1 = 1;
	pTxD->SDLen0 = (SrcBufLen - TXINFO_SIZE);
	pTxD->SDLen1 = 0;
	pTxD->SDPtr0 = SrcBufPA;
	//pTxD->DMADONE = 0;
	ral_write_txd(pAd, pTxD, pTxInfo, TRUE, FIFO_EDCA);

#ifdef VHT_TXBF_SUPPORT
#ifdef DBG
	if (pHeader_802_11->FC.Type == FC_TYPE_CNTL && pHeader_802_11->FC.SubType == SUBTYPE_VHT_NDPA)
	{
		DBGPRINT(RT_DEBUG_OFF, ("%s(): Send VhtNDPA to peer(wcid=%d, pMacEntry=%p) with dataRate(PhyMode:%s, BW:%sHz, %dSS, MCS%d)\n",
					__FUNCTION__, wcid, pMacEntry, get_phymode_str(transmit->field.MODE),
					get_bw_str(transmit->field.BW),
					(transmit->field.MCS>>4) + 1, (transmit->field.MCS & 0xf)));

		hex_dump("VHT NDPA frame raw data", pSrcBufVA, SrcBufLen);
		dump_txwi(pAd, pFirstTxWI);

		DBGPRINT(RT_DEBUG_OFF, ("%s():pTxD->SDPtr0=0x%x, SrcBufPtr=0x%p, TxRing=%d, SwIdx=%d\n",
								__FUNCTION__, pTxD->SDPtr0,
								(UCHAR *)(pSrcBufVA + TXINFO_SIZE), QueIdx, SwIdx));

		hex_dump("MgmtTxDInfo", pAd->TxRing[QueIdx].Cell[SwIdx].AllocVa, TXD_SIZE);
		dump_txd(pAd, (TXD_STRUC *)pAd->TxRing[QueIdx].Cell[SwIdx].AllocVa);
		dump_txinfo(pAd, (TXINFO_STRUC *)(pAd->TxRing[QueIdx].Cell[SwIdx].AllocVa + sizeof(TXD_STRUC)));
		hex_dump("MgmtPktInfo", (pSrcBufVA + TXINFO_SIZE), pTxD->SDLen0);
	}
#endif
#endif /* VHT_TXBF_SUPPORT */

//+++Add by shiang for debug
//---Add by shiang for debug

#ifdef RT_BIG_ENDIAN
	RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
	WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, FALSE, TYPE_TXD);
#endif

	pAd->RalinkCounters.KickTxCount++;
	pAd->RalinkCounters.OneSecTxDoneCount++;

	/* flush dcache if no consistent memory is supported */
	RTMP_DCACHE_FLUSH(SrcBufPA, SrcBufLen);
	RTMP_DCACHE_FLUSH(pAd->TxRing[QueIdx].Cell[SwIdx].AllocPa, TXD_SIZE);

   	/* Increase TX_CTX_IDX, but write to register later.*/
	INC_RING_INDEX(pAd->TxRing[QueIdx].TxCpuIdx, TX_RING_SIZE);

	RTMP_IO_WRITE32(pAd, pAd->TxRing[QueIdx].hw_cidx_addr,  pAd->TxRing[QueIdx].TxCpuIdx);

	return NDIS_STATUS_SUCCESS;
}


#ifdef CONFIG_ANDES_SUPPORT
BOOLEAN RxRing1DoneInterruptHandle(RTMP_ADAPTER *pAd) 
{
	UINT32 RxProcessed, RxPending;
	BOOLEAN bReschedule = FALSE;
	RXINFO_STRUC *pRxInfo = NULL;
	PNDIS_PACKET pRxPacket = NULL;
	RX_BLK rxblk, *pRxBlk = NULL;
	RXFCE_INFO *pFceInfo;
	BOOLEAN bCmdRspPacket = FALSE;

	RxProcessed = RxPending = 0;

	/* process whole rx ring */
	while (1)
	{

		if (RTMP_TEST_FLAG(pAd, (fRTMP_ADAPTER_RADIO_OFF |
								fRTMP_ADAPTER_RESET_IN_PROGRESS |
									fRTMP_ADAPTER_HALT_IN_PROGRESS)) || 
			!RTMP_TEST_FLAG(pAd,fRTMP_ADAPTER_START_UP))
		{
			break;
		}

#ifdef RTMP_MAC_PCI
		if (RxProcessed++ > 32)
		{
			bReschedule = TRUE;
			break;
		}
#endif /* RTMP_MAC_PCI */

		/* 
			1. allocate a new data packet into rx ring to replace received packet
				then processing the received packet
			2. the callee must take charge of release of packet
			3. As far as driver is concerned, the rx packet must
				a. be indicated to upper layer or
				b. be released if it is discarded
		*/

		pRxBlk = &rxblk;

		pRxPacket = GetPacketFromRxRing(pAd, pRxBlk, &bReschedule, &RxPending, &bCmdRspPacket, 1);
		if ((pRxPacket == NULL))
			break;

		/* get rx descriptor and data buffer */
		pFceInfo = rxblk.pRxFceInfo;
		pRxInfo = rxblk.pRxInfo;

		if (pFceInfo->info_type == CMD_PACKET)
		{
			DBGPRINT(RT_DEBUG_INFO, ("%s: Receive command packet.\n", __FUNCTION__));
			pci_rx_cmd_msg_complete(pAd, pFceInfo, (PUCHAR) pRxInfo);
			RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_SUCCESS);
			continue;
		} else {
			DBGPRINT(RT_DEBUG_ERROR,("%s: Receive non-command packet.\n", __FUNCTION__));
		}
		
	}
	return bReschedule;
}


VOID RTMPHandleTxRing8DmaDoneInterrupt(RTMP_ADAPTER *pAd)
{
	PTXD_STRUC	 pTxD;
#ifdef RT_BIG_ENDIAN
    PTXD_STRUC      pDestTxD;
	UCHAR hw_hdr_info[TXD_SIZE];
#endif
	PNDIS_PACKET pPacket;
/*	int 		 i;*/
	UCHAR	FREE = 0;
	RTMP_CTRL_RING *pCtrlRing = &pAd->CtrlRing;

	NdisAcquireSpinLock(&pAd->CtrlRingLock);	

	RTMP_IO_READ32(pAd, pCtrlRing->hw_didx_addr, &pCtrlRing->TxDmaIdx);
	while (pCtrlRing->TxSwFreeIdx!= pCtrlRing->TxDmaIdx)
	{
		RTMP_DMACB *dma_cb = &pCtrlRing->Cell[pCtrlRing->TxSwFreeIdx];

		FREE++;
#ifdef RT_BIG_ENDIAN
		pDestTxD = (PTXD_STRUC) (dma_cb->AllocVa);
		NdisMoveMemory(&hw_hdr_info[0], pDestTxD, TXD_SIZE);
		pTxD = (TXD_STRUC *)&hw_hdr_info[0];
		RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#else
		pTxD = (PTXD_STRUC) (dma_cb->AllocVa);
#endif


		pPacket = dma_cb->pNdisPacket;
		if (pPacket == NULL)
		{
			INC_RING_INDEX(pCtrlRing->TxSwFreeIdx, MGMT_RING_SIZE);
			continue;
		}

		if (pPacket)
			PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr0, pTxD->SDLen0, RTMP_PCI_DMA_TODEVICE);
		
		dma_cb->pNdisPacket = NULL;

		/* flush dcache if no consistent memory is supported */
		RTMP_DCACHE_FLUSH(dma_cb->AllocPa, TXD_SIZE);

		INC_RING_INDEX(pCtrlRing->TxSwFreeIdx, MGMT_RING_SIZE);

#ifdef RT_BIG_ENDIAN
		RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
		WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, TRUE, TYPE_TXD);
#endif
		pci_kick_out_cmd_msg_complete(pPacket);
	}
	NdisReleaseSpinLock(&pAd->CtrlRingLock);
}

#endif /* CONFIG_ANDES_SUPPORT */


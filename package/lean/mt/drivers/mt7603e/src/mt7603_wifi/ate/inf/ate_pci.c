/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology	5th	Rd.
 * Science-based Industrial	Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2006, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of Ralink Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

 	Module Name:
	ate_pci.c

	Abstract:

	Revision History:
	Who			When	    What
	--------	----------  ----------------------------------------------
	Name		Date	    Modification logs
*/

#ifdef RTMP_MAC_PCI

#include "rt_config.h"

/* 802.11 MAC Header, Type:Data, Length:24bytes + 6 bytes QOS/HTC + 2 bytes padding */
extern UCHAR TemplateFrame[32];

INT TxDmaBusy(
	IN PRTMP_ADAPTER pAd)
{
	INT result;
	WPDMA_GLO_CFG_STRUC GloCfg;

#if defined(RTMP_MAC) || defined(RLT_MAC)
	RTMP_IO_READ32(pAd, WPDMA_GLO_CFG, &GloCfg.word);	/* disable DMA */
#elif defined(MT_MAC)
	RTMP_IO_READ32(pAd, MT_WPDMA_GLO_CFG, &GloCfg.word);	/* disable DMA */
#endif
	result = (GloCfg.field.TxDMABusy) ? TRUE : FALSE;

	return result;
}


INT RxDmaBusy(
	IN PRTMP_ADAPTER pAd)
{
	INT result;
	WPDMA_GLO_CFG_STRUC GloCfg;

#if defined(RTMP_MAC) || defined(RLT_MAC)
	RTMP_IO_READ32(pAd, WPDMA_GLO_CFG, &GloCfg.word);	/* disable DMA */
#elif defined(MT_MAC)
	RTMP_IO_READ32(pAd, MT_WPDMA_GLO_CFG, &GloCfg.word);	/* disable DMA */
#endif
	result = (GloCfg.field.RxDMABusy) ? TRUE : FALSE;

	return result;
}


VOID RtmpDmaEnable(
	IN PRTMP_ADAPTER pAd,
	IN INT Enable)
{
	ULONG WaitCnt = 0;

	/* check if DMA is in busy mode or not. */
	while (TxDmaBusy(pAd) || RxDmaBusy(pAd))
	{
		RtmpusecDelay(10);
		if (WaitCnt++ > 100)
			break;
	}

	AsicSetWPDMA(pAd, PDMA_TX_RX, (Enable > 0 ? TRUE : FALSE));

	RtmpOsMsDelay(5);

	return;
}


/*
========================================================================
	Routine Description:
		Write TxWI for ATE mode.
		
	Return Value:
		None
========================================================================
*/
VOID ATEWriteTxWI(
	IN	PRTMP_ADAPTER	pAd,
	IN	TXWI_STRUC *pOutTxWI,	
	IN	BOOLEAN			FRAG,	
	IN	BOOLEAN			CFACK,
	IN	BOOLEAN			InsTimestamp,
	IN	BOOLEAN 		AMPDU,
	IN	BOOLEAN 		Ack,
	IN	BOOLEAN 		NSeq,		/* HW new a sequence. */
	IN	UCHAR			BASize,
	IN	UCHAR			WCID,
	IN	ULONG			Length,
	IN	UCHAR 			PID,
	IN	UCHAR			TID,
	IN	UCHAR			TxRate,
	IN	UCHAR			Txopmode,	
	IN	BOOLEAN			CfAck,	
	IN	HTTRANSMIT_SETTING	*pTransmit)
{
	TXWI_STRUC TxWI, *pTxWI;
	UINT8 TXWISize = pAd->chipCap.TXWISize;
	UCHAR bw;

	/* If CCK or OFDM, BW must be 20 */
	bw = (pTransmit->field.MODE <= MODE_OFDM) ? (BW_20) : (pTransmit->field.BW);
	
	OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED);
	NdisZeroMemory(&TxWI, TXWISize);
	pTxWI = &TxWI;

#ifdef RLT_MAC
	if (pAd->chipCap.hif_type == HIF_RLT) {
		struct  _TXWI_NMAC *txwi_n = (struct  _TXWI_NMAC *)pTxWI;

		txwi_n->FRAG= FRAG;
		txwi_n->CFACK = CFACK;
		txwi_n->TS= InsTimestamp;
		txwi_n->AMPDU = AMPDU;
		txwi_n->ACK = Ack;
		txwi_n->txop = Txopmode;
		txwi_n->NSEQ= NSeq;
		txwi_n->BAWinSize = BASize;	
		txwi_n->wcid = WCID;
		txwi_n->MPDUtotalByteCnt = Length; 		
		txwi_n->BW = bw;
		txwi_n->ShortGI = pTransmit->field.ShortGI;
		txwi_n->STBC = pTransmit->field.STBC;
		txwi_n->MCS = pTransmit->field.MCS;
		txwi_n->PHYMODE = pTransmit->field.MODE;
		txwi_n->CFACK = CfAck;
		txwi_n->MIMOps = 0;
		txwi_n->MpduDensity = 0;
		txwi_n->TxPktId = txwi_n->MCS;
	}
#endif /* RLT_MAC */

#ifdef RTMP_MAC
	if (pAd->chipCap.hif_type == HIF_RTMP) {
		struct  _TXWI_OMAC *txwi_o = (struct  _TXWI_OMAC *)pTxWI;

		txwi_o->FRAG= FRAG;
		txwi_o->CFACK = CFACK;
		txwi_o->TS= InsTimestamp;
		txwi_o->AMPDU = AMPDU;
		txwi_o->ACK = Ack;
		txwi_o->txop = Txopmode;
		txwi_o->NSEQ = NSeq;
		txwi_o->BAWinSize = BASize;	
		txwi_o->wcid = WCID;
		txwi_o->MPDUtotalByteCnt = Length; 
		txwi_o->BW = bw;
		txwi_o->ShortGI = pTransmit->field.ShortGI;
		txwi_o->STBC = pTransmit->field.STBC;
		txwi_o->MCS = pTransmit->field.MCS;
		txwi_o->PHYMODE = pTransmit->field.MODE;
		txwi_o->CFACK = CfAck;
		txwi_o->MIMOps = 0;
		txwi_o->MpduDensity = 0;
		txwi_o->PacketId = txwi_o->MCS;
	}
#endif /* RTMP_MAC */

	NdisMoveMemory(pOutTxWI, &TxWI, TXWISize);

    return;
}


/* 
==========================================================================
	Description:
		Allocate Frame payload buffer.
	NOTE:
		This routine should only be used in ATE mode.
==========================================================================
*/
NDIS_STATUS ATEPayloadAlloc(
	IN PRTMP_ADAPTER pAd,
	IN UINT32 idx)
{
	ATE_INFO *pATEInfo = &(pAd->ate);
	NDIS_PHYSICAL_ADDRESS AllocPa;
	
	/* alloc buffer for payload */
#ifdef CONFIG_QA
	if ((pATEInfo->bQAEnabled == TRUE) && (pATEInfo->DLen != 0)) 
	{
		pATEInfo->pAtePacket[idx] = RTMP_AllocateRxPacketBuffer(pAd, ((POS_COOKIE)(pAd->OS_Cookie))->pci_dev,
			pATEInfo->DLen + 0x100, FALSE, &pATEInfo->AteAllocVa[idx], &AllocPa);
	}
	else
#endif /* CONFIG_QA */
	{
		pATEInfo->pAtePacket[idx] = RTMP_AllocateRxPacketBuffer(pAd, ((POS_COOKIE)(pAd->OS_Cookie))->pci_dev,
			pATEInfo->TxLength, FALSE, &pATEInfo->AteAllocVa[idx], &AllocPa);
	}

	if (pATEInfo->pAtePacket[idx] == NULL)
	{
		pATEInfo->TxCount = 0;
		DBGPRINT_ERR(("%s : fail to alloc packet space.\n", __FUNCTION__));
		return (NDIS_STATUS_RESOURCES);
	}

	return (NDIS_STATUS_SUCCESS);
}


/* 
==========================================================================
	Description:
		Initialize Frame payload.
	NOTE:
		This routine should only be used in ATE mode.
==========================================================================
*/
PNDIS_PACKET ATEPayloadInit(
	IN PRTMP_ADAPTER pAd,
	IN UINT32 idx)
{
	ATE_INFO *pATEInfo = &(pAd->ate);
	RTMP_TX_RING *pTxRing = &pAd->TxRing[QID_AC_BE];
	PNDIS_PACKET pPacket = pTxRing->Cell[idx].pNdisPacket;
	PUCHAR pDest = (PUCHAR)pATEInfo->AteAllocVa[idx];
	UINT pos = 0;

#ifdef CONFIG_QA
	if ((pATEInfo->bQATxStart == TRUE) && (pATEInfo->DLen != 0)) 
	{
		GET_OS_PKT_LEN(pPacket) = pATEInfo->DLen;
#ifndef LINUX
		GET_OS_PKT_TOTAL_LEN(pPacket) = pATEInfo->DLen;
#endif /* LIMUX */
	}
	else
#endif /* CONFIG_QA */
	{
		GET_OS_PKT_LEN(pPacket) = pATEInfo->TxLength - pATEInfo->HLen;
#ifndef LINUX
		GET_OS_PKT_TOTAL_LEN(pPacket) = pATEInfo->TxLength - pATEInfo->HLen;
#endif /* LINUX */
	}

	/* prepare frame payload */
#ifdef CONFIG_QA
	if ((pATEInfo->bQATxStart == TRUE) && (pATEInfo->DLen != 0))
	{
		/* copy pattern to payload */
		if ((pATEInfo->PLen != 0))
		{
			for (pos = 0; pos < pATEInfo->DLen; pos += pATEInfo->PLen)
			{
				memcpy(GET_OS_PKT_DATAPTR(pPacket) + pos, pATEInfo->Pattern, pATEInfo->PLen);
			}
		}
	}
	else
#endif /* CONFIG_QA */
	{
		for (pos = 0; pos < GET_OS_PKT_LEN(pPacket); pos++)
		{
			if ( pATEInfo->bFixedPayload )
			{
				/* default payload is 0xA5 */
				pDest[pos] = pATEInfo->Payload;
			} 
			else
			{
				pDest[pos] = RandomByte(pAd);
			}
		}
	}

	return pPacket;
}


/* 
==========================================================================
	Description:
		Setup Frame format.
	NOTE:
		This routine should only be used in ATE mode.
==========================================================================
*/
INT ATESetUpFrame(
	IN PRTMP_ADAPTER pAd,
	IN UINT32 TxIdx)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	TXINFO_STRUC *pTxInfo;
	TXD_STRUC *pTxD;
#ifdef RT_BIG_ENDIAN
	TXD_STRUC *pDestTxD;
	UCHAR tx_hw_info[TXD_SIZE];
#endif /* RT_BIG_ENDIAN */
	PNDIS_PACKET pPacket=NULL;
	PUCHAR pDest=NULL;
	HTTRANSMIT_SETTING	TxHTPhyMode;

	RTMP_TX_RING *pTxRing = &pAd->TxRing[QID_AC_BE];
	TXWI_STRUC *pTxWI = (TXWI_STRUC *)pTxRing->Cell[TxIdx].DmaBuf.AllocVa;
	PUCHAR pDMAHeaderBufVA = (PUCHAR) pTxRing->Cell[TxIdx].DmaBuf.AllocVa;
	UINT8 TXWISize = pAd->chipCap.TXWISize;
#ifdef CONFIG_QA
	PHEADER_802_11	pHeader80211;
#endif /* CONFIG_QA */
	UCHAR bw, sgi, stbc, mcs, phy_mode, frag, cfack, ts, ampdu, ack, nseq, bawinsize, pkt_id, txop;
	USHORT byte_cnt;

	bw = sgi = stbc = mcs = phy_mode = frag = cfack = ts =0;
	ampdu = ack = nseq = bawinsize = pkt_id = txop = 0;
	byte_cnt = 0;
#ifdef RLT_MAC
	if (pAd->chipCap.hif_type == HIF_RLT)
	{
		bw = pATEInfo->TxWI.TXWI_N.BW;
		sgi = pATEInfo->TxWI.TXWI_N.ShortGI;
		stbc = pATEInfo->TxWI.TXWI_N.STBC;
		mcs = pATEInfo->TxWI.TXWI_N.MCS;
		phy_mode = pATEInfo->TxWI.TXWI_N.PHYMODE;
			
		frag = pATEInfo->TxWI.TXWI_N.FRAG;
		cfack = pATEInfo->TxWI.TXWI_N.CFACK,
		ts = pATEInfo->TxWI.TXWI_N.TS;
		ampdu = pATEInfo->TxWI.TXWI_N.AMPDU;
		ack = pATEInfo->TxWI.TXWI_N.ACK;
		nseq = pATEInfo->TxWI.TXWI_N.NSEQ;
		bawinsize =pATEInfo->TxWI.TXWI_N.BAWinSize;
		byte_cnt = pATEInfo->TxWI.TXWI_N.MPDUtotalByteCnt;
		pkt_id = pATEInfo->TxWI.TXWI_N.TxPktId;
		txop = pATEInfo->TxWI.TXWI_N.txop;
		cfack = pATEInfo->TxWI.TXWI_N.CFACK;
	}
#endif /* RLT_MAC */
#ifdef RTMP_MAC
	if (pAd->chipCap.hif_type == HIF_RTMP)
	{
		bw = pATEInfo->TxWI.TXWI_O.BW;
		sgi = pATEInfo->TxWI.TXWI_O.ShortGI;
		stbc = pATEInfo->TxWI.TXWI_O.STBC;
		mcs = pATEInfo->TxWI.TXWI_O.MCS;
		phy_mode = pATEInfo->TxWI.TXWI_O.PHYMODE;
			
		frag = pATEInfo->TxWI.TXWI_O.FRAG;
		cfack = pATEInfo->TxWI.TXWI_O.CFACK,
		ts = pATEInfo->TxWI.TXWI_O.TS;
		ampdu = pATEInfo->TxWI.TXWI_O.AMPDU;
		ack = pATEInfo->TxWI.TXWI_O.ACK;
		nseq = pATEInfo->TxWI.TXWI_O.NSEQ;
		bawinsize =pATEInfo->TxWI.TXWI_O.BAWinSize;
		byte_cnt = pATEInfo->TxWI.TXWI_O.MPDUtotalByteCnt;
		pkt_id = pATEInfo->TxWI.TXWI_O.PacketId;
		txop = pATEInfo->TxWI.TXWI_O.txop;
		cfack = pATEInfo->TxWI.TXWI_O.CFACK;
	}
#endif /* RTMP_MAC */

	/* fill TxWI */
	TxHTPhyMode.field.BW = bw;
	TxHTPhyMode.field.ShortGI = sgi;
	TxHTPhyMode.field.STBC = stbc;
	TxHTPhyMode.field.MCS = mcs;
	TxHTPhyMode.field.MODE = phy_mode;

	if (pATEInfo->bQATxStart == TRUE) 
	{
		/* always use QID_AC_BE and FIFO_EDCA */
		ATEWriteTxWI(pAd, pTxWI, frag, cfack,
			ts, ampdu, ack,
			nseq, bawinsize, 0,
			byte_cnt, pkt_id, 0, 0,
			txop, cfack,
			&TxHTPhyMode);

	}
	else
	{
		ATEWriteTxWI(pAd, pTxWI, FALSE, FALSE, FALSE,  FALSE, FALSE, FALSE, 
			4, 0, pATEInfo->TxLength, 0, 0, 0, IFS_HTTXOP, FALSE, &TxHTPhyMode);

	}
	
	/* fill 802.11 header */
#ifdef CONFIG_QA
	if (pATEInfo->bQATxStart == TRUE) 
	{
		NdisMoveMemory(pDMAHeaderBufVA + TXWISize, pATEInfo->Header, pATEInfo->HLen);
	}
	else
#endif /* CONFIG_QA */
	{
		pATEInfo->HLen = LENGTH_802_11;
		NdisMoveMemory(pDMAHeaderBufVA + TXWISize, TemplateFrame, pATEInfo->HLen);
		NdisMoveMemory(pDMAHeaderBufVA + TXWISize + 4, pATEInfo->Addr1, MAC_ADDR_LEN);
		NdisMoveMemory(pDMAHeaderBufVA + TXWISize + 10, pATEInfo->Addr2, MAC_ADDR_LEN);
		NdisMoveMemory(pDMAHeaderBufVA + TXWISize + 16, pATEInfo->Addr3, MAC_ADDR_LEN);
	}

#ifdef RT_BIG_ENDIAN
	RTMPFrameEndianChange(pAd, (((PUCHAR)pDMAHeaderBufVA) + TXWISize), DIR_READ, FALSE);
#endif /* RT_BIG_ENDIAN */

	pPacket = ATEPayloadInit(pAd, TxIdx);
	/* error check */
	if (pPacket == NULL)
	{
		pATEInfo->TxCount = 0;
		DBGPRINT_ERR(("%s : fail to init frame payload.\n", __FUNCTION__));
		return -1;
	}
	pTxRing->Cell[TxIdx].pNdisPacket = pPacket;

	/* build Tx descriptor */
#ifndef RT_BIG_ENDIAN
	pTxD = (TXD_STRUC *) pTxRing->Cell[TxIdx].AllocVa;
	pTxInfo = (TXINFO_STRUC *)(pTxRing->Cell[TxIdx].AllocVa + sizeof(TXD_STRUC));
#else
    pDestTxD  = (TXD_STRUC *)pTxRing->Cell[TxIdx].AllocVa;
	NdisMoveMemory(&tx_hw_info[0], (UCHAR *)pDestTxD, TXD_SIZE);
	pTxD = (TXD_STRUC *)&tx_hw_info[0];
	pTxInfo = (TXINFO_STRUC *)(&tx_hw_info[0] + sizeof(TXD_STRUC));
#endif /* !RT_BIG_ENDIAN */
	{
		/* prepare TxD */
		TX_BLK txblk;
		txblk.SrcBufLen = GET_OS_PKT_LEN(pATEInfo->pAtePacket[TxIdx]);
		txblk.pSrcBufData = (PUCHAR)pATEInfo->AteAllocVa[TxIdx];
		NdisZeroMemory(pTxD, TXD_SIZE);
		/* build Tx descriptor */
		pTxD->SDPtr0 = RTMP_GetPhysicalAddressLow (pTxRing->Cell[TxIdx].DmaBuf.AllocPa);
		pTxD->SDLen0 = TXWISize + pATEInfo->HLen;
		pTxD->LastSec0 = 0;
		pTxD->SDPtr1 = PCI_MAP_SINGLE(pAd, &txblk, 0, 1, RTMP_PCI_DMA_TODEVICE);
		pTxD->SDLen1 = GET_OS_PKT_LEN(pATEInfo->pAtePacket[TxIdx]);
		pTxD->LastSec1 = 1;
		pTxD->Burst = 0;
#if defined(RTMP_MAC) || defined(RLT_MAC)
		ral_write_txd(pAd, pTxD, pTxInfo, FALSE, FIFO_EDCA);
#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */
#ifdef MT_MAC
		mt_write_txd(pAd, pTxD);
#endif /* MT_MAC */
	}
#ifdef CONFIG_QA
	if (pATEInfo->bQATxStart == TRUE)
	{
		pDest = (PUCHAR)pTxWI;
		pDest += TXWISize;
		pHeader80211 = (PHEADER_802_11)pDest;
		
		/* modify sequence number... */
		if (pATEInfo->TxDoneCount == 0)
			pATEInfo->seq = pHeader80211->Sequence;
		else
			pHeader80211->Sequence = ++pATEInfo->seq;
	}
#endif /* CONFIG_QA */

#ifdef RT_BIG_ENDIAN
	RTMPWIEndianChange(pAd, (PUCHAR)pTxWI, TYPE_TXWI);
	RTMPFrameEndianChange(pAd, (((PUCHAR)pDMAHeaderBufVA) + TXWISize), DIR_WRITE, FALSE);
	RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
	WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, FALSE, TYPE_TXD);
#endif /* RT_BIG_ENDIAN */

	return 0;
}


/* 
==========================================================================
	Description:
		Tx ring skb hook and ring index setting.
	NOTE:
		This routine should only be used in ATE mode.
==========================================================================
*/
NDIS_STATUS TxRingGetReady(
	IN PRTMP_ADAPTER pAd)
{
	ATE_INFO *pATEInfo = &(pAd->ate);
	RTMP_TX_RING *pTxRing = &pAd->TxRing[QID_AC_BE];
	UINT32 TxIdx = 0;
	UINT32 ring_idx = 0;
	RTMP_IO_READ32(pAd, pTxRing->hw_didx_addr, &pTxRing->TxDmaIdx);
	pTxRing->TxSwFreeIdx = pTxRing->TxDmaIdx;
	pTxRing->TxCpuIdx = pTxRing->TxDmaIdx;
	RTMP_IO_WRITE32(pAd, pTxRing->hw_cidx_addr, pTxRing->TxCpuIdx);

	for (ring_idx = 0; (ring_idx < TX_RING_SIZE) && (ring_idx < pATEInfo->TxCount); ring_idx++)
	{
		TxIdx = pTxRing->TxCpuIdx;

		if (ATESetUpFrame(pAd, TxIdx) != 0)
		{
			return (NDIS_STATUS_FAILURE);
		}

		if (((ring_idx + 1) < TX_RING_SIZE) && (ring_idx < pATEInfo->TxCount))
		{
			INC_RING_INDEX(pTxRing->TxCpuIdx, TX_RING_SIZE);
		}
	}

	return (NDIS_STATUS_SUCCESS);
}

#endif /* RTMP_MAC_PCI */


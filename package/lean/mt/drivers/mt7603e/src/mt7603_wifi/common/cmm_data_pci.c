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

	DBGPRINT(RT_DEBUG_OFF, ("\tSDPtr0/SDLen0/LastSec0=0x%x/0x%x/0x%x\n",
				pRxD->SDP0, pRxD->SDL0, pRxD->LS0));
	DBGPRINT(RT_DEBUG_OFF, ("\tSDPtr1/SDLen1/LastSec1=0x%x/0x%x/0x%x\n",
				pRxD->SDP1, pRxD->SDL1, pRxD->LS1));
	DBGPRINT(RT_DEBUG_OFF, ("\tDDONE=0x%x\n", pRxD->DDONE));
}


VOID dumpTxRing(RTMP_ADAPTER *pAd, INT ring_idx)
{
	//RTMP_DMABUF *pDescRing;
	RTMP_TX_RING *pTxRing;
	TXD_STRUC *pTxD;
	int index;

	ASSERT(ring_idx < NUM_OF_TX_RING);
	//pDescRing = (RTMP_DMABUF *)pAd->TxDescRing[ring_idx].AllocVa;

	pTxRing = &pAd->TxRing[ring_idx];
	for (index = 0; index < TX_RING_SIZE; index++)
	{
		pTxD = (TXD_STRUC *)pTxRing->Cell[index].AllocVa;
		hex_dump("Dump TxDesc", (UCHAR *)pTxD, sizeof(TXD_STRUC));
		dump_txd(pAd, pTxD);
#if defined(RTMP_MAC) || defined(RLT_MAC)
		if (pAd->chipCap.hif_type == HIF_RTMP || pAd->chipCap.hif_type == HIF_RLT) {
			TXINFO_STRUC *pTxInfo;

			pTxInfo = (TXINFO_STRUC *)(pTxRing->Cell[index].AllocVa + sizeof(TXD_STRUC));
			hex_dump("Dump TxInfo", (UCHAR *)pTxInfo, sizeof(TXINFO_STRUC));
			dump_txinfo(pAd, pTxInfo);
		}
#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */
	}
}


BOOLEAN MonitorTxRing(RTMP_ADAPTER *pAd)
{
	UINT32 Value;

	if (pAd->TxDMACheckTimes < 10)
	{
		/* Check if TX DMA busy */
		RTMP_IO_READ32(pAd, MT_WPDMA_GLO_CFG, &Value);
	
		if ((Value & TX_DMA_BUSY) == TX_DMA_BUSY)
		{
			/* Check TX FIFO if have space */
			RTMP_IO_WRITE32(pAd, 0x4244, 0x98000000);
			RTMP_IO_READ32(pAd, 0x4244, &Value);
			if ((Value & (1 << 8)) == 0)
			{
				pAd->TxDMACheckTimes = 0;
				return FALSE;
			}
			else
			{
				pAd->TxDMACheckTimes++;
				return FALSE;
			}
		}
		else
		{
			pAd->TxDMACheckTimes = 0;
			return FALSE;
		}
	}
	else
	{
		pAd->TxDMACheckTimes = 0;
		return TRUE;
	}
}


BOOLEAN MonitorRxRing(RTMP_ADAPTER *pAd)
{
	UINT32 Value;

	if (pAd->RxDMACheckTimes < 10)
	{
		/* Check if RX DMA busy */
		RTMP_IO_READ32(pAd, MT_WPDMA_GLO_CFG, &Value);
	
		if ((Value & RX_DMA_BUSY) == RX_DMA_BUSY)
		{
			/* Check RX FIFO if have data */
			RTMP_IO_WRITE32(pAd, 0x4244, 0x28000000);
			RTMP_IO_READ32(pAd, 0x4244, &Value);
			if ((Value & (1 << 8)) == 0)
			{
				pAd->RxDMACheckTimes = 0;
				return FALSE;
			}
			else
			{
				pAd->RxDMACheckTimes++;
				return FALSE;
			}
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


BOOLEAN MonitorRxPse(RTMP_ADAPTER *pAd)
{
	UINT32 RemapBase, RemapOffset;
	UINT32 Value;
	UINT32 RestoreValue;

	if (pAd->RxPseCheckTimes < 10)
	{
		/* Check RX FIFO if not ready */
		RTMP_IO_WRITE32(pAd, 0x4244, 0x28000000);
		RTMP_IO_READ32(pAd, 0x4244, &Value);
		if ((Value & (1 << 8)) != 0)
		{
			pAd->RxPseCheckTimes = 0;
			return FALSE;
		}
		else
		{
			RTMP_IO_READ32(pAd, MCU_PCIE_REMAP_2, &RestoreValue);
			RemapBase = GET_REMAP_2_BASE(0x800c006c) << 19;
			RemapOffset = GET_REMAP_2_OFFSET(0x800c006c);
			RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RemapBase);
	
			RTMP_IO_WRITE32(pAd, 0x80000 + RemapOffset, 3);

			RTMP_IO_READ32(pAd, 0x80000 + RemapOffset, &Value);
			
			if(((Value & (0x8001 << 16)) == (0x8001 << 16)) ||
					((Value & (0xe001 << 16)) == (0xe001 << 16)))
			{
				pAd->RxPseCheckTimes++;
				RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RestoreValue);
				return FALSE;
			}
			else
			{
				pAd->RxPseCheckTimes = 0;
				RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RestoreValue);
				return FALSE;
			}
		}
	}
	else
	{
		pAd->RxPseCheckTimes = 0;
		return TRUE;
	}
}


VOID dumpRxRing(RTMP_ADAPTER *pAd, INT ring_idx)
{
	//RTMP_DMABUF *pDescRing;
	RTMP_RX_RING *pRxRing;
	RXD_STRUC *pRxD;
	int index;


	//pDescRing = (RTMP_DMABUF *)pAd->RxDescRing[0].AllocVa;

	pRxRing = &pAd->RxRing[0];
	for (index = 0; index < RX_RING_SIZE; index++)
	{
		pRxD = (RXD_STRUC *)pRxRing->Cell[index].AllocVa;
		hex_dump("Dump RxDesc", (UCHAR *)pRxD, sizeof(RXD_STRUC));
		dump_rxd(pAd, pRxD);
	}
}


#if defined(RTMP_MAC) || defined(RLT_MAC)
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


#ifdef RLT_MAC
static VOID rlt_update_txinfo(
	IN RTMP_ADAPTER *pAd,
	IN TXINFO_STRUC *pTxInfo,
	IN TX_BLK *pTxBlk)
{
}
#endif /* RLT_MAC */


VOID ral_write_txd(
	IN RTMP_ADAPTER *pAd,
	IN TXD_STRUC *pTxD,
	IN TX_BLK *txblk,
	IN BOOLEAN bWIV,
	IN UCHAR QueueSEL)
{
	TXINFO_STRUC *pTxInfo = (TXINFO_STRUC *)(pTxD + 1);


	/* Always use Long preamble before verifiation short preamble functionality works well.*/
	/* Todo: remove the following line if short preamble functionality works*/

	OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED);

	ral_write_txinfo(pAd, pTxInfo, bWIV, QueueSEL);

	pTxD->DMADONE = 0;
}

#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */


#ifdef MT_MAC
VOID mt_write_txd(RTMP_ADAPTER *pAd, TXD_STRUC *pTxD)
{

	pTxD->DMADONE = 0;
}
#endif /* MT_MAC */





/* IRQL = DISPATCH_LEVEL */
VOID ComposeNullFrame(RTMP_ADAPTER *pAd)
{
	NdisZeroMemory(&pAd->NullFrame, sizeof (HEADER_802_11));
	pAd->NullFrame.FC.Type = FC_TYPE_DATA;
	pAd->NullFrame.FC.SubType = SUBTYPE_DATA_NULL;
	pAd->NullFrame.FC.ToDs = 1;
	COPY_MAC_ADDR(pAd->NullFrame.Addr1, pAd->CommonCfg.Bssid);
	COPY_MAC_ADDR(pAd->NullFrame.Addr2, pAd->CurrentAddress);
	COPY_MAC_ADDR(pAd->NullFrame.Addr3, pAd->CommonCfg.Bssid);
}


USHORT write_first_buf(RTMP_ADAPTER *pAd, TX_BLK *txblk, UCHAR *dma_buf)
{
	UINT tx_hw_hdr_len = pAd->chipCap.tx_hw_hdr_len; // TXWISize + TSO_SIZE
	USHORT first_buf_len;

	first_buf_len =  tx_hw_hdr_len + txblk->MpduHeaderLen + txblk->HdrPadLen - txblk->hw_rsv_len;


	NdisMoveMemory(dma_buf,
					(UCHAR *)(txblk->HeaderBuf + txblk->hw_rsv_len),
					first_buf_len);

	return first_buf_len;
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
#ifdef RT_BIG_ENDIAN
	TXD_STRUC *pDestTxD;
	UCHAR tx_hw_info[TXD_SIZE];
	UINT8 *temp;
#if defined(RTMP_MAC) || defined(RLT_MAC)
	UINT tx_hw_hdr_len = pAd->chipCap.tx_hw_hdr_len; // TXWISize + TSO_SIZE
#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */
#endif
	UINT32 BufBasePaLow;
	RTMP_TX_RING *pTxRing;
#ifdef MT_MAC
#ifdef USE_BMC
	if (pTxBlk->QueIdx == QID_BMC)
	{
		pTxRing = &pAd->TxBmcRing;
		TxIdx = pAd->TxBmcRing.TxCpuIdx;
	}
	else
#endif /* USE_BMC */
#endif
	{
		/* get Tx Ring Resource*/
		pTxRing = &pAd->TxRing[pTxBlk->QueIdx];
		TxIdx = pAd->TxRing[pTxBlk->QueIdx].TxCpuIdx;
	}

	pDMAHeaderBufVA = (UCHAR *)pTxRing->Cell[TxIdx].DmaBuf.AllocVa;
	BufBasePaLow = RTMP_GetPhysicalAddressLow(pTxRing->Cell[TxIdx].DmaBuf.AllocPa);

	pTxRing->Cell[TxIdx].pNdisPacket = pTxBlk->pPacket;
	pTxRing->Cell[TxIdx].pNextNdisPacket = NULL;
	pTxRing->Cell[TxIdx].PacketPa = PCI_MAP_SINGLE(pAd, pTxBlk, 0, 1, RTMP_PCI_DMA_TODEVICE);

	/* build Tx Descriptor*/
#ifndef RT_BIG_ENDIAN
	pTxD = (TXD_STRUC *)pTxRing->Cell[TxIdx].AllocVa;
#else
	pDestTxD = (TXD_STRUC *)pTxRing->Cell[TxIdx].AllocVa;
	NdisMoveMemory(&tx_hw_info[0], (UCHAR *)pDestTxD, TXD_SIZE);
	pTxD = (TXD_STRUC *)&tx_hw_info[0];
#endif

	pTxD->SDPtr0 = BufBasePaLow;
	pTxD->SDLen0 = write_first_buf(pAd, pTxBlk, pDMAHeaderBufVA);
	pTxD->SDPtr1 = pTxRing->Cell[TxIdx].PacketPa;
	pTxD->SDLen1 = pTxBlk->SrcBufLen;
	pTxD->LastSec0 = !(pTxD->SDLen1);
	pTxD->LastSec1 = (bIsLast && pTxD->SDLen1) ? 1 : 0;
	pTxD->Burst = 0;

#if defined(RTMP_MAC) || defined(RLT_MAC)
	ral_write_txd(pAd, pTxD, pTxBlk, FALSE, FIFO_EDCA);
#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */
#ifdef MT_MAC
	mt_write_txd(pAd, pTxD);
#endif /* MT_MAC */

#ifdef RT_BIG_ENDIAN
#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
	{
		UINT16 TmacLen = (pAd->chipCap.tx_hw_hdr_len - pTxBlk->hw_rsv_len);
		MTMacInfoEndianChange(pAd,  (PUCHAR)(pDMAHeaderBufVA), TYPE_TMACINFO, TmacLen);
		RTMPFrameEndianChange(pAd, (PUCHAR)(pDMAHeaderBufVA + TmacLen), DIR_WRITE, FALSE);
	}

#endif
#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP || pAd->chipCap.hif_type == HIF_RLT)
	{
		RTMPWIEndianChange(pAd, (PUCHAR)(pDMAHeaderBufVA), TYPE_TXWI);
		RTMPFrameEndianChange(pAd, (PUCHAR)(pDMAHeaderBufVA + tx_hw_hdr_len), DIR_WRITE, FALSE);
	}
#endif
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

#ifdef CONFIG_WIFI_TEST
	pTxRing->Cell[TxIdx].DataOut = 1;
	pTxRing->Cell[TxIdx].TimePeriod = 0;
#endif	

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
#ifdef RT_BIG_ENDIAN
	TXD_STRUC *pDestTxD;
	UCHAR tx_hw_info[TXD_SIZE];
	//TXD_STRUC TxD;
#endif
	UINT32 BufBasePaLow;
	RTMP_TX_RING *pTxRing;
#if !defined(MT7603) && !defined(MT7628)
	USHORT hwHdrLen;
#endif /* !defined(MT7603) && !defined(MT7628) */
	UINT32 firstDMALen = 0;
	//UINT8 TXWISize = pAd->chipCap.TXWISize;
	UINT tx_hw_hdr_len = pAd->chipCap.tx_hw_hdr_len; // TXWISize + TSO_SIZE

	ASSERT((pTxBlk->TxFrameType == TX_AMSDU_FRAME || pTxBlk->TxFrameType == TX_RALINK_FRAME));
	bIsLast = ((frameNum == (pTxBlk->TotalFrameNum - 1)) ? 1 : 0);

	/* get Tx Ring Resource */
	pTxRing = &pAd->TxRing[pTxBlk->QueIdx];
	TxIdx = pAd->TxRing[pTxBlk->QueIdx].TxCpuIdx;
	pDMAHeaderBufVA = (PUCHAR) pTxRing->Cell[TxIdx].DmaBuf.AllocVa;
	BufBasePaLow = RTMP_GetPhysicalAddressLow(pTxRing->Cell[TxIdx].DmaBuf.AllocPa);

#if defined(MT7603) || defined(MT7628)
	if (frameNum == 0)
	{
		/* copy TXINFO + TXWI + WLAN Header + LLC into DMA Header Buffer */
		firstDMALen =  tx_hw_hdr_len - pTxBlk->hw_rsv_len + pTxBlk->MpduHeaderLen + pTxBlk->HdrPadLen;

		NdisMoveMemory(pDMAHeaderBufVA, pTxBlk->HeaderBuf + pTxBlk->hw_rsv_len, firstDMALen);
	}
	else
	{
		firstDMALen = pTxBlk->MpduHeaderLen;
		NdisMoveMemory(pDMAHeaderBufVA, pTxBlk->HeaderBuf, firstDMALen);
	}

#else

	if (frameNum == 0)
	{
		/* copy TXINFO + TXWI + WLAN Header + LLC into DMA Header Buffer */
		if (pTxBlk->TxFrameType == TX_AMSDU_FRAME)
			hwHdrLen = pTxBlk->MpduHeaderLen - AMSDU_SUBHEAD_LEN + pTxBlk->HdrPadLen + AMSDU_SUBHEAD_LEN;
		else if (pTxBlk->TxFrameType == TX_RALINK_FRAME)
			hwHdrLen = pTxBlk->MpduHeaderLen - ARALINK_HEADER_LEN + pTxBlk->HdrPadLen + ARALINK_HEADER_LEN;

		hwHdrLen = pTxBlk->MpduHeaderLen + pTxBlk->HdrPadLen;

		firstDMALen = pAd->chipCap.TXWISize + hwHdrLen;
	}
	else
	{
		firstDMALen = pTxBlk->MpduHeaderLen;
	}

	NdisMoveMemory(pDMAHeaderBufVA, pTxBlk->HeaderBuf + TXINFO_SIZE, firstDMALen);
#endif /* defined(MT7603) || defined(MT7628) */

	pTxRing->Cell[TxIdx].pNdisPacket = pTxBlk->pPacket;
	pTxRing->Cell[TxIdx].pNextNdisPacket = NULL;
	pTxRing->Cell[TxIdx].PacketPa = PCI_MAP_SINGLE(pAd, pTxBlk, 0, 1, RTMP_PCI_DMA_TODEVICE);

	/* build Tx Descriptor */
#ifndef RT_BIG_ENDIAN
	pTxD = (TXD_STRUC *) pTxRing->Cell[TxIdx].AllocVa;
#else
	pDestTxD = (TXD_STRUC *) pTxRing->Cell[TxIdx].AllocVa;
	NdisMoveMemory(&tx_hw_info[0], (UCHAR *)pDestTxD, TXD_SIZE);
	pTxD = (TXD_STRUC *)&tx_hw_info[0];
#endif

	pTxD->SDPtr0 = BufBasePaLow;
	pTxD->SDLen0 = firstDMALen; /* include padding*/
	pTxD->SDPtr1 = pTxRing->Cell[TxIdx].PacketPa;
	pTxD->SDLen1 = pTxBlk->SrcBufLen;
	pTxD->LastSec0 = !(pTxD->SDLen1);
	pTxD->LastSec1 = (bIsLast && pTxD->SDLen1) ? 1 : 0;
	pTxD->Burst = 0;

#if defined(RTMP_MAC) || defined(RLT_MAC)
	ral_write_txd(pAd, pTxD, pTxBlk, FALSE, FIFO_EDCA);
#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */
#ifdef MT_MAC
	mt_write_txd(pAd, pTxD);
#endif /* MT_MAC */

#ifdef RT_BIG_ENDIAN
#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
	{
		UINT16  TmacLen = (pAd->chipCap.tx_hw_hdr_len - pTxBlk->hw_rsv_len);
		if (frameNum == 0)
			RTMPFrameEndianChange(pAd, (PUCHAR)(pDMAHeaderBufVA + TmacLen), DIR_WRITE, FALSE);
		if (frameNum != 0)
			MTMacInfoEndianChange(pAd,  (PUCHAR)(pDMAHeaderBufVA), TYPE_TMACINFO, TmacLen);
	}
#endif
#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP || pAd->chipCap.hif_type == HIF_RLT)
	{
	if (frameNum == 0)
		RTMPFrameEndianChange(pAd, (PUCHAR)(pDMAHeaderBufVA + pAd->chipCap.TXWISize), DIR_WRITE, FALSE);
	if (frameNum != 0)
		RTMPWIEndianChange(pAd, (PUCHAR)pDMAHeaderBufVA, TYPE_TXWI);
	}
#endif
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
	RTMP_TX_RING *pTxRing;
	UCHAR *tmac_info;

	/* get Tx Ring Resource*/
	pTxRing = &pAd->TxRing[pTxBlk->QueIdx];
	tmac_info = (UCHAR *)pTxRing->Cell[FirstTxIdx].DmaBuf.AllocVa;
#ifdef RLT_MAC
	if (pAd->chipCap.hif_type == HIF_RLT) {
		TXWI_STRUC *pTxWI = (TXWI_STRUC *)tmac_info;

		pTxWI->TXWI_N.MPDUtotalByteCnt = totalMPDUSize;
	}
#endif /* RLT_MAC */
#ifdef RTMP_MAC
	if (pAd->chipCap.hif_type == HIF_RTMP) {
		TXWI_STRUC *pTxWI = (TXWI_STRUC *)tmac_info;

		pTxWI->TXWI_O.MPDUtotalByteCnt = totalMPDUSize;
	}
#endif /* RTMP_MAC */

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT) {
		TMAC_TXD_S *txd_s = (TMAC_TXD_S *)tmac_info;

		txd_s->txd_0.tx_byte_cnt = totalMPDUSize;
	}
#endif /* MT_MAC */

#ifdef RT_BIG_ENDIAN
#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
	{
		UINT16  TmacLen = (pAd->chipCap.tx_hw_hdr_len - pTxBlk->hw_rsv_len);
		MTMacInfoEndianChange(pAd, tmac_info, TYPE_TMACINFO, TmacLen);
	}
#endif
#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP || pAd->chipCap.hif_type == HIF_RLT)
	{
	RTMPWIEndianChange(pAd, tmac_info, TYPE_TXWI);
	}
#endif
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
#ifdef RT_BIG_ENDIAN
	TXD_STRUC *pDestTxD;
	UCHAR tx_hw_info[TXD_SIZE];
	//TXD_STRUC TxD;
#endif
	UINT32 BufBasePaLow;
	RTMP_TX_RING *pTxRing;
	//UINT8 TXWISize = pAd->chipCap.TXWISize;

	/* Get Tx Ring Resource*/
	pTxRing = &pAd->TxRing[pTxBlk->QueIdx];
	TxIdx = pAd->TxRing[pTxBlk->QueIdx].TxCpuIdx;
	pDMAHeaderBufVA = (PUCHAR) pTxRing->Cell[TxIdx].DmaBuf.AllocVa;
	BufBasePaLow = RTMP_GetPhysicalAddressLow(pTxRing->Cell[TxIdx].DmaBuf.AllocPa);

	/* Build Tx Descriptor*/
#ifndef RT_BIG_ENDIAN
	pTxD = (TXD_STRUC *) pTxRing->Cell[TxIdx].AllocVa;
#else
	pDestTxD = (TXD_STRUC *) pTxRing->Cell[TxIdx].AllocVa;
	NdisMoveMemory(&tx_hw_info[0], (UCHAR *)pDestTxD, TXD_SIZE);
	pTxD = (TXD_STRUC *)&tx_hw_info[0];
#endif

	if (fragNum == pTxBlk->TotalFragNum)
	{
		pTxRing->Cell[TxIdx].pNdisPacket = pTxBlk->pPacket;
		pTxRing->Cell[TxIdx].pNextNdisPacket = NULL;
	}

	pTxD->SDPtr0 = BufBasePaLow;
	/* Copy TXINFO + TXWI + WLAN Header + LLC into DMA Header Buffer, including padding */
	pTxD->SDLen0 = write_first_buf(pAd, pTxBlk, pDMAHeaderBufVA);
	pTxD->SDPtr1 = PCI_MAP_SINGLE(pAd, pTxBlk, 0, 1, RTMP_PCI_DMA_TODEVICE);
	pTxD->SDLen1 = pTxBlk->SrcBufLen;
	pTxD->LastSec0 = !(pTxD->SDLen1);
	pTxD->LastSec1 = (pTxD->SDLen1 ? 1 : 0);
	pTxD->Burst = 0;

#if defined(RTMP_MAC) || defined(RLT_MAC)
	ral_write_txd(pAd, pTxD, pTxBlk, FALSE, FIFO_EDCA);
#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */
#ifdef MT_MAC
	mt_write_txd(pAd, pTxD);
#endif /* MT_MAC */

#ifdef RT_BIG_ENDIAN
#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
	{
		UINT16  TmacLen = (pAd->chipCap.tx_hw_hdr_len - pTxBlk->hw_rsv_len);
		MTMacInfoEndianChange(pAd,  (PUCHAR)(pDMAHeaderBufVA), TYPE_TMACINFO, TmacLen);
		RTMPFrameEndianChange(pAd, (PUCHAR)(pDMAHeaderBufVA + TmacLen), DIR_WRITE, FALSE);
	}

#endif
#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP || pAd->chipCap.hif_type == HIF_RLT)
	{
	RTMPWIEndianChange(pAd, pDMAHeaderBufVA, TYPE_TXWI);
	RTMPFrameEndianChange(pAd, (PUCHAR)(pDMAHeaderBufVA + pAd->chipCap.TXWISize), DIR_WRITE, FALSE);
	}
#endif
	RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
	WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, FALSE, TYPE_TXD);
#endif /* RT_BIG_ENDIAN */

	RetTxIdx = TxIdx;
	pTxBlk->Priv += pTxBlk->SrcBufLen;

	/* flush dcache if no consistent memory is supported */
	RTMP_DCACHE_FLUSH(pTxRing->Cell[TxIdx].DmaBuf.AllocPa, pTxD->SDLen0);
	RTMP_DCACHE_FLUSH(pTxBlk->pSrcBufData, pTxBlk->SrcBufLen);
	RTMP_DCACHE_FLUSH(pTxRing->Cell[TxIdx].AllocPa, RXD_SIZE);

	/* Update Tx index */
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
	UCHAR *frm_buf;
	UINT32 SwIdx;
	INT pkt_len;
	RTMP_DMACB *dma_buf;

#if defined(MT7603) || defined(MT7628)
	if (IS_MT7603(pAd) || (IS_MT7628(pAd)))
	{
		frm_buf = pSrcBufVA;
		pkt_len = SrcBufLen;
		if (QueIdx == Q_IDX_BCN) {
			SwIdx = pAd->BcnRing.TxCpuIdx;
			dma_buf = &pAd->BcnRing.Cell[SwIdx];
		} else {
			SwIdx = pAd->MgmtRing.TxCpuIdx;
			dma_buf = &pAd->MgmtRing.Cell[SwIdx];
		}
	}
	else
#endif /* MT7603 */
	{
		frm_buf = pSrcBufVA + TXINFO_SIZE;
		pkt_len = SrcBufLen - TXINFO_SIZE;
		SwIdx = pAd->MgmtRing.TxCpuIdx;
		dma_buf = &pAd->MgmtRing.Cell[SwIdx];
	}

#ifdef RT_BIG_ENDIAN
	pDestTxD = (TXD_STRUC *)dma_buf->AllocVa;
	NdisMoveMemory(&tx_hw_info[0], (UCHAR *)pDestTxD, TXD_SIZE);
	pTxD = (TXD_STRUC *)&tx_hw_info[0];
	RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#else
	pTxD = (TXD_STRUC *)dma_buf->AllocVa;
#endif

	dma_buf->pNdisPacket = pPacket;
	dma_buf->pNextNdisPacket = NULL;
	dma_buf->PacketPa = PCI_MAP_SINGLE(pAd, frm_buf, pkt_len, 0, RTMP_PCI_DMA_TODEVICE);

	pTxD->SDPtr0 = dma_buf->PacketPa;
	pTxD->SDLen0 = pkt_len;
	pTxD->LastSec0 = 1;
	pTxD->SDPtr1 = 0;
	pTxD->LastSec1 = 0;
	pTxD->SDLen1 = 0;
	pTxD->Burst = 0;

#if defined(RTMP_MAC) || defined(RLT_MAC)
	ral_write_txd(pAd, pTxD, NULL, TRUE, FIFO_MGMT);
#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */
#ifdef MT_MAC
	mt_write_txd(pAd, pTxD);
#endif /* MT_MAC */

#ifdef RT_BIG_ENDIAN
	RTMPDescriptorEndianChange((UCHAR *)pTxD, TYPE_TXD);
	WriteBackToDescriptor((UCHAR *)pDestTxD, (UCHAR *)pTxD, FALSE, TYPE_TXD);
#endif

	pAd->RalinkCounters.KickTxCount++;
	pAd->RalinkCounters.OneSecTxDoneCount++;

	/* flush dcache if no consistent memory is supported */
	RTMP_DCACHE_FLUSH(pSrcBufVA, SrcBufLen);
	RTMP_DCACHE_FLUSH(dma_buf->AllocPa, TXD_SIZE);

#if defined(MT7603) || defined(MT7628)
	if ((IS_MT7603(pAd) || (IS_MT7628(pAd))) && (QueIdx == Q_IDX_BCN))
	{
		INC_RING_INDEX(pAd->BcnRing.TxCpuIdx, BCN_RING_SIZE);
		RTMP_IO_WRITE32(pAd, pAd->BcnRing.hw_cidx_addr,  pAd->BcnRing.TxCpuIdx);
	}
	else
#endif /* MT7603 */
	{
		/* Increase TX_CTX_IDX, but write to register later.*/
		INC_RING_INDEX(pAd->MgmtRing.TxCpuIdx, MGMT_RING_SIZE);
		RTMP_IO_WRITE32(pAd, pAd->MgmtRing.hw_cidx_addr,  pAd->MgmtRing.TxCpuIdx);
	}

	return 0;
}

#ifdef CONFIG_AP_SUPPORT

static VOID trPsTokenUpdate(RTMP_ADAPTER *pAd,UINT QueIdx,PNDIS_PACKET pPacket)
{
	struct tx_swq_fifo *fifo_swq;
	STA_TR_ENTRY *tr_entry;
	INT enq_idx;					
	UCHAR wcid;

	wcid = RTMP_GET_PACKET_WCID(pPacket); 

	if(wcid < MAX_LEN_OF_TR_TABLE)
	{
	
		tr_entry = &pAd->MacTab.tr_entry[wcid];
		
		if (tr_entry->tx_queue[QueIdx].Number == 0) 
		{
			tr_entry->TokenCount[QueIdx] = 0;
		} else
		if ((tr_entry->ps_state == APPS_RETRIEVE_IDLE) && (tr_entry->TokenCount[QueIdx] > 0)) 
		{
			fifo_swq = &pAd->tx_swq[QueIdx];
			enq_idx = fifo_swq->enqIdx;

			if ((fifo_swq->swq[enq_idx] == 0) && (tr_entry->enq_cap)) 
			{
				fifo_swq->swq[enq_idx] = tr_entry->wcid;
				INC_RING_INDEX(fifo_swq->enqIdx, TX_SWQ_FIFO_LEN);
				TR_TOKEN_COUNT_DEC(tr_entry, QueIdx);
			}				
		}
	} 
}

#endif

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
	TXD_STRUC TxD, *pOriTxD;
	BOOLEAN bReschedule = FALSE;
	//UINT8 TXWISize = pAd->chipCap.TXWISize;

#ifdef MT_MAC
	if (QueIdx == QID_BMC)
		pTxRing = &pAd->TxBmcRing;
	else
#endif /* MT_MAC */
	{
	ASSERT(QueIdx < NUM_OF_TX_RING);
	if (QueIdx >= NUM_OF_TX_RING)
		return FALSE;

	pTxRing = &pAd->TxRing[QueIdx];
	}

	RTMP_IO_READ32(pAd, pTxRing->hw_didx_addr, &pTxRing->TxDmaIdx);
	while (pTxRing->TxSwFreeIdx != pTxRing->TxDmaIdx)
	{
#ifdef CONFIG_ATE
#ifdef CONFIG_QA
		if ((ATE_ON(pAd)) && (pAd->ATECtrl.bQATxStart == TRUE) && (pAd->ATECtrl.QID == QueIdx))
		{
			HEADER_802_11 *pHeader80211;

			pAd->ATECtrl.TxDoneCount++;
			pAd->RalinkCounters.KickTxCount++;

			/* always use QID_AC_BE and FIFO_EDCA */
			ASSERT(pAd->ATECtrl.QID == 0);
			pAd->ATECtrl.TxAc0++;

			FREE++;
#ifndef RT_BIG_ENDIAN
			pTxD = (TXD_STRUC *) (pTxRing->Cell[pTxRing->TxSwFreeIdx].AllocVa);
			pOriTxD = pTxD;
			NdisMoveMemory(&TxD, pTxD, sizeof(TXD_STRUC));
			pTxD = &TxD;
#else
			pDestTxD = (TXD_STRUC *) (pTxRing->Cell[pTxRing->TxSwFreeIdx].AllocVa);
			pOriTxD = pDestTxD ;
			//TxD = *pDestTxD;
			//pTxD = &TxD;
			NdisMoveMemory(&tx_hw_info[0], pDestTxD, TXD_SIZE);
			pTxD = (TXD_STRUC *)&tx_hw_info[0];
			RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#endif
/*			pTxD->DMADONE = 0; */

			pHeader80211 = (PHEADER_802_11)((UCHAR *)(pTxRing->Cell[pTxRing->TxSwFreeIdx].DmaBuf.AllocVa) + pAd->chipCap.TXWISize);
#ifdef RT_BIG_ENDIAN
			RTMPFrameEndianChange(pAd, (PUCHAR)pHeader80211, DIR_READ, FALSE);
#endif
			pHeader80211->Sequence = ++pAd->ATECtrl.seq;
#ifdef RT_BIG_ENDIAN
			RTMPFrameEndianChange(pAd, (PUCHAR)pHeader80211, DIR_WRITE, FALSE);
#endif

			if  ((pAd->ATECtrl.bQATxStart == TRUE) && (pAd->ATECtrl.Mode & ATE_TXFRAME) && (pAd->ATECtrl.TxDoneCount < pAd->ATECtrl.TxCount))
			{
				pAd->RalinkCounters.TransmittedByteCount +=  (pTxD->SDLen1 + pTxD->SDLen0);
				pAd->RalinkCounters.OneSecTransmittedByteCount += (pTxD->SDLen1 + pTxD->SDLen0);
				pAd->RalinkCounters.OneSecDmaDoneCount[QueIdx] ++;

				/* flush dcache if no consistent memory is supported */
				RTMP_DCACHE_FLUSH(pTxRing->Cell[pTxRing->TxSwFreeIdx].AllocPa, RXD_SIZE);

				INC_RING_INDEX(pTxRing->TxSwFreeIdx, TX_RING_SIZE);

				/* get TX_DTX_IDX again */
				RTMP_IO_READ32(pAd, pTxRing->hw_didx_addr,  &pTxRing->TxDmaIdx);
				goto kick_out;
			}
			else if ((pAd->ATECtrl.TxStatus == 1)/* or (pAd->ate.bQATxStart == TRUE) ??? */ && (pAd->ATECtrl.TxDoneCount == pAd->ATECtrl.TxCount))
			{
				DBGPRINT(RT_DEBUG_TRACE,("all Tx is done\n"));

				/* Tx status enters idle mode.*/
				pAd->ATECtrl.TxStatus = 0;
			}
			else if (!(pAd->ATECtrl.Mode & ATE_TXFRAME))
			{
				/* not complete sending yet, but someone press the Stop TX botton */
				DBGPRINT(RT_DEBUG_INFO,("not complete sending yet, but someone pressed the Stop TX bottom\n"));
				DBGPRINT(RT_DEBUG_INFO,("pAd->ate.Mode = 0x%02x\n", pAd->ATECtrl.Mode));
			}
			else
			{
				DBGPRINT(RT_DEBUG_OFF,("pTxRing->TxSwFreeIdx = %d\n", pTxRing->TxSwFreeIdx));
			}

#ifndef RT_BIG_ENDIAN
			NdisMoveMemory(pOriTxD, pTxD, sizeof(TXD_STRUC));
#else
			RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
			//*pDestTxD = TxD;
			NdisMoveMemory(pDestTxD, pTxD, TXD_SIZE);
#endif /* RT_BIG_ENDIAN */

			/* flush dcache if no consistent memory is supported */
			RTMP_DCACHE_FLUSH(pTxRing->Cell[pTxRing->TxSwFreeIdx].AllocPa, RXD_SIZE);

			INC_RING_INDEX(pTxRing->TxSwFreeIdx, TX_RING_SIZE);
			continue;
		}
#endif /* CONFIG_QA */
#endif /* CONFIG_ATE */

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
		pTxD = (TXD_STRUC *) (pTxRing->Cell[pTxRing->TxSwFreeIdx].AllocVa);
		pOriTxD = pTxD;
		NdisMoveMemory(&TxD, pTxD, sizeof(TXD_STRUC));
		pTxD = &TxD;
#else
		pDestTxD = (TXD_STRUC *) (pTxRing->Cell[pTxRing->TxSwFreeIdx].AllocVa);
		pOriTxD = pDestTxD ;
		//TxD = *pDestTxD;
		//pTxD = &TxD;
		NdisMoveMemory(&tx_hw_info[0], pDestTxD, TXD_SIZE);
		pTxD = (TXD_STRUC *)&tx_hw_info[0];
		RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#endif
/*		pTxD->DMADONE = 0; */

#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
#ifdef UAPSD_SUPPORT
		UAPSD_SP_PacketCheck(pAd,
				pTxRing->Cell[pTxRing->TxSwFreeIdx].pNdisPacket,
				((UCHAR *)pTxRing->Cell[\
				pTxRing->TxSwFreeIdx].DmaBuf.AllocVa) + pAd->chipCap.TXWISize);
#endif /* UAPSD_SUPPORT */
#else
#ifdef CONFIG_AP_SUPPORT
#ifdef UAPSD_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			UAPSD_SP_PacketCheck(pAd,
				pTxRing->Cell[pTxRing->TxSwFreeIdx].pNdisPacket,
				((UCHAR *)pTxRing->Cell[pTxRing->TxSwFreeIdx].DmaBuf.AllocVa) + pAd->chipCap.TXWISize);
		}
#endif /* UAPSD_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
#endif /* defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) */

#ifdef CONFIG_ATE
		/* Execution of this block is not allowed when ATE is running. */
		if (!(ATE_ON(pAd)))
#endif /* CONFIG_ATE */
		{
			pPacket = pTxRing->Cell[pTxRing->TxSwFreeIdx].pNdisPacket;
			if (pPacket)
			{
#ifndef DMA_CACHE_OPTIMIZE
#ifdef CONFIG_5VT_ENHANCE
				if (RTMP_GET_PACKET_5VT(pPacket))
					PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr1, 16, RTMP_PCI_DMA_TODEVICE);
				else
#endif /* CONFIG_5VT_ENHANCE */
					PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr1, pTxD->SDLen1, RTMP_PCI_DMA_TODEVICE);
#endif /* DMA_CACHE_OPTIMIZE */

#ifdef CONFIG_AP_SUPPORT
				if (QueIdx < WMM_QUE_NUM)
				{
					trPsTokenUpdate(pAd,QueIdx,pPacket);
				}
#endif /* CONFIG_AP_SUPPORT */



#ifdef WLAN_SKB_RECYCLE
				if (skb_queue_len(&pAd->rx0_recycle) < NUM_RX_DESC &&
					skb_recycle_check(RTPKT_TO_OSPKT(pPacket), RX_BUFFER_NORMSIZE))
					__skb_queue_head(&pAd->rx0_recycle, RTPKT_TO_OSPKT(pPacket));
				else
#endif /* WLAN_SKB_RECYCLE */
					RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);

				pTxRing->Cell[pTxRing->TxSwFreeIdx].pNdisPacket = NULL;
			}

			pPacket = pTxRing->Cell[pTxRing->TxSwFreeIdx].pNextNdisPacket;
			if (pPacket)
			{
#ifndef DMA_CACHE_OPTIMIZE
#ifdef CONFIG_5VT_ENHANCE
				if (RTMP_GET_PACKET_5VT(pPacket))
					PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr1, 16, RTMP_PCI_DMA_TODEVICE);
				else
#endif /* CONFIG_5VT_ENHANCE */
					PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr1, pTxD->SDLen1, RTMP_PCI_DMA_TODEVICE);
#endif /* DMA_CACHE_OPTIMIZE */

#ifdef WLAN_SKB_RECYCLE
				if (skb_queue_len(&pAd->rx0_recycle) < NUM_RX_DESC &&
					skb_recycle_check(RTPKT_TO_OSPKT(pPacket), RX_BUFFER_NORMSIZE ))
					__skb_queue_head(&pAd->rx0_recycle, RTPKT_TO_OSPKT(pPacket));
				else
#endif /* WLAN_SKB_RECYCLE */
					RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);

				pTxRing->Cell[pTxRing->TxSwFreeIdx].pNextNdisPacket = NULL;
			}
		}

		/* flush dcache if no consistent memory is supported */
		RTMP_DCACHE_FLUSH(pTxRing->Cell[pTxRing->TxSwFreeIdx].AllocPa, TXD_SIZE);

		pAd->RalinkCounters.TransmittedByteCount +=  (pTxD->SDLen1 + pTxD->SDLen0);
		pAd->RalinkCounters.OneSecTransmittedByteCount += (pTxD->SDLen1 + pTxD->SDLen0);

#ifdef MT_MAC
		if (QueIdx == QID_BMC)
			pAd->RalinkCounters.OneSecDmaDoneCount[QID_AC_BE]++;
		else
#endif /* MT_MAC */
			pAd->RalinkCounters.OneSecDmaDoneCount[QueIdx]++;

		INC_RING_INDEX(pTxRing->TxSwFreeIdx, TX_RING_SIZE);

#ifdef CONFIG_WIFI_TEST
		pTxRing->Cell[pTxRing->TxSwFreeIdx].DataOut = 0;
		pTxRing->Cell[pTxRing->TxSwFreeIdx].TimePeriod = 0;
#endif

		/* get tx_tdx_idx again */
		RTMP_IO_READ32(pAd, pTxRing->hw_didx_addr,  &pTxRing->TxDmaIdx);

#ifdef RT_BIG_ENDIAN
		RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
		//*pDestTxD = TxD;
		NdisMoveMemory(pDestTxD, pTxD, TXD_SIZE);
#else
		NdisMoveMemory(pOriTxD, pTxD, sizeof(TXD_STRUC));
#endif

#ifdef CONFIG_ATE
#ifdef CONFIG_QA
kick_out:
#endif /* CONFIG_QA */

		/*
			ATE_TXCONT mode also need to send some normal frames, so let it in.
			ATE_STOP must be changed not to be 0xff
			to prevent it from running into this block.
		*/
		if ((pAd->ATECtrl.Mode & ATE_TXFRAME) && (QueIdx == pAd->ATECtrl.QID) &&
			(QueIdx < NUM_OF_TX_RING)) {
			/* TxDoneCount++ has been done if QA is used.*/
			if (pAd->ATECtrl.bQATxStart == FALSE)
				pAd->ATECtrl.TxDoneCount++;
			
			if (((pAd->ATECtrl.TxCount - pAd->ATECtrl.TxDoneCount + 1) >= TX_RING_SIZE)) {
				/* Note : We increase TxCpuIdx here, not TxSwFreeIdx ! */

				/* flush dcache if no consistent memory is supported */
				RTMP_DCACHE_FLUSH(pTxRing->Cell[pTxRing->TxCpuIdx].AllocPa, RXD_SIZE);

				INC_RING_INDEX(pAd->TxRing[QueIdx].TxCpuIdx, TX_RING_SIZE);
#ifndef RT_BIG_ENDIAN
				pTxD = (TXD_STRUC *) (pTxRing->Cell[pAd->TxRing[QueIdx].TxCpuIdx].AllocVa);
				pOriTxD = pTxD;
		        NdisMoveMemory(&TxD, pTxD, sizeof(TXD_STRUC));
				pTxD = &TxD;
#else
		        pDestTxD = (TXD_STRUC *) (pTxRing->Cell[pAd->TxRing[QueIdx].TxCpuIdx].AllocVa);
		        pOriTxD = pDestTxD ;
				NdisMoveMemory(&tx_hw_info[0], pDestTxD, TXD_SIZE);
				pTxD = (TXD_STRUC *)&tx_hw_info[0];
		        RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#endif
				pTxD->DMADONE = 0;
#ifndef RT_BIG_ENDIAN
        		NdisMoveMemory(pOriTxD, pTxD, sizeof(TXD_STRUC));
#else
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
#endif /* CONFIG_ATE */
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
	ULONG IrqFlags = 0;
	BOOLEAN bReschedule = FALSE;


	/* Make sure Tx ring resource won't be used by other threads*/
	RTMP_IRQ_LOCK(&pAd->irq_lock, IrqFlags);

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT) {
		if (tx_mask & TX_AC0_DONE)
			bReschedule = RTMPFreeTXDUponTxDmaDone(pAd, QID_AC_BK);

		if (tx_mask & TX_AC1_DONE)
			bReschedule |= RTMPFreeTXDUponTxDmaDone(pAd, QID_AC_BE);
	}
#endif /* MT_MAC */

#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP || pAd->chipCap.hif_type == HIF_RLT)
	{
		if (tx_mask & TX_AC0_DONE)
			bReschedule = RTMPFreeTXDUponTxDmaDone(pAd, QID_AC_BE);

		if (tx_mask & TX_AC1_DONE)
			bReschedule |= RTMPFreeTXDUponTxDmaDone(pAd, QID_AC_BK);
	}
#endif /* #if defined(RTMP_MAC) || defined(RLT_MAC) */

	if (tx_mask & TX_HCCA_DONE)
		bReschedule |= RTMPFreeTXDUponTxDmaDone(pAd, QID_HCCA);

	if (tx_mask & TX_AC3_DONE)
		bReschedule |= RTMPFreeTXDUponTxDmaDone(pAd, QID_AC_VO);

	if (tx_mask & TX_AC2_DONE)
		bReschedule |= RTMPFreeTXDUponTxDmaDone(pAd, QID_AC_VI);
#ifdef MT_MAC
	if (tx_mask & TX_BMC_DONE)
		bReschedule |= RTMPFreeTXDUponTxDmaDone(pAd, QID_BMC);
#endif /* MT_MAC */

	/* Make sure to release Tx ring resource*/
	RTMP_IRQ_UNLOCK(&pAd->irq_lock, IrqFlags);

	RTMPDeQueuePacket(pAd, FALSE, NUM_OF_TX_RING, WCID_ALL, MAX_TX_PROCESS);

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
	TXD_STRUC *pTxD;
#ifdef RT_BIG_ENDIAN
    TXD_STRUC *pDestTxD;
	UCHAR tx_hw_info[TXD_SIZE];
#endif
	PNDIS_PACKET pPacket;
/*	int 		 i;*/
	UCHAR	FREE = 0;
	RTMP_MGMT_RING *pMgmtRing = &pAd->MgmtRing;
	//UINT8 TXWISize = pAd->chipCap.TXWISize;


	NdisAcquireSpinLock(&pAd->MgmtRingLock);

	RTMP_IO_READ32(pAd, pMgmtRing->hw_didx_addr, &pMgmtRing->TxDmaIdx);
	while (pMgmtRing->TxSwFreeIdx!= pMgmtRing->TxDmaIdx)
	{
		FREE++;
#ifdef RT_BIG_ENDIAN
        pDestTxD = (TXD_STRUC *) (pMgmtRing->Cell[pAd->MgmtRing.TxSwFreeIdx].AllocVa);
        //TxD = *pDestTxD;
        //pTxD = &TxD;
		NdisMoveMemory(&tx_hw_info[0], pDestTxD, TXD_SIZE);
		pTxD = (TXD_STRUC *)&tx_hw_info[0];
		RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#else
		pTxD = (TXD_STRUC *) (pMgmtRing->Cell[pAd->MgmtRing.TxSwFreeIdx].AllocVa);
#endif

		pTxD->DMADONE = 0;
		pPacket = pMgmtRing->Cell[pMgmtRing->TxSwFreeIdx].pNdisPacket;

		if (pPacket == NULL)
		{
			INC_RING_INDEX(pMgmtRing->TxSwFreeIdx, MGMT_RING_SIZE);
			continue;
		}

#define LMR_FRAME_GET()	(GET_OS_PKT_DATAPTR(pPacket) + pAd->chipCap.TXWISize)

#ifdef UAPSD_SUPPORT
#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
		UAPSD_QoSNullTxMgmtTxDoneHandle(pAd,
					pPacket,
					LMR_FRAME_GET(), FALSE);
#else
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			UAPSD_QoSNullTxMgmtTxDoneHandle(pAd,
					pPacket, LMR_FRAME_GET(), FALSE);
		}
#endif /* CONFIG_AP_SUPPORT */
#endif /* defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) */
#endif /* UAPSD_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */


		if (pPacket)
		{
			PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr0, pTxD->SDLen0, RTMP_PCI_DMA_TODEVICE);
			RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
		}
		pMgmtRing->Cell[pMgmtRing->TxSwFreeIdx].pNdisPacket = NULL;

		pPacket = pMgmtRing->Cell[pMgmtRing->TxSwFreeIdx].pNextNdisPacket;
		if (pPacket)
		{
			PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr1, pTxD->SDLen1, RTMP_PCI_DMA_TODEVICE);
			RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
		}
		pMgmtRing->Cell[pMgmtRing->TxSwFreeIdx].pNextNdisPacket = NULL;

		/* flush dcache if no consistent memory is supported */
		RTMP_DCACHE_FLUSH(pMgmtRing->Cell[pAd->MgmtRing.TxSwFreeIdx].AllocPa, TXD_SIZE);

		INC_RING_INDEX(pMgmtRing->TxSwFreeIdx, MGMT_RING_SIZE);

#ifdef RT_BIG_ENDIAN
		RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
		WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, TRUE, TYPE_TXD);
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
//+++Add by Carter
#ifdef MT_MAC
	volatile UINT32 en_cr, stat_cr;

	stat_cr = 0x00000000L;
	en_cr = 0x00000000L;

	printk("%s\n", __func__);

	RTMP_IO_READ32(pAd, HWISR3, &stat_cr);
	RTMP_IO_READ32(pAd, HWIER3, &en_cr);

	/* disable the interrupt source */
	RTMP_IO_WRITE32(pAd, HWIER3, (~stat_cr & en_cr));

	/* write 1 to clear */
	RTMP_IO_WRITE32(pAd, HWISR3, stat_cr);
	RTMP_IO_WRITE32(pAd, HWIER3, en_cr);
#endif /* MT_MAC */
//---Add by Carter

#ifdef CONFIG_AP_SUPPORT
	if (pAd->OpMode == OPMODE_AP)
	{
		POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;

#if defined(RTMP_MAC) || defined(RLT_MAC)
		if (pAd->chipCap.hif_type != HIF_MT)
			ReSyncBeaconTime(pAd);
#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */

#ifdef MT_MAC
		if (pAd->chipCap.hif_type == HIF_MT) {
			/* disable mask */

		}
#endif /*MT_MAC */

		RTMP_OS_TASKLET_SCHE(&pObj->tbtt_task);

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


VOID RTMPHandleTTTTInterrupt(RTMP_ADAPTER *pAd)
{
//+++Add by Carter
#ifdef MT_MAC
	//volatile UINT32 en_cr, stat_cr;
	printk("%s\n", __func__);

	//stat_cr = 0x00000000L;
	//en_cr = 0x00000000L;
#endif
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
	)
	{
			APUpdateAllBeaconFrame(pAd);
	}
	else
#endif /* CONFIG_AP_SUPPORT */
	{
		if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE))
		{
			DBGPRINT(RT_DEBUG_TRACE, ("%s()...\n", __FUNCTION__));
		}
	}

#ifdef MT_MAC
	/* use pretbtt to check DTIM and dequeue bmc packets */
//#ifdef USE_BMC
	if (pAd->chipCap.hif_type == HIF_MT) {
		POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
		RTMP_OS_TASKLET_SCHE(&pObj->tbtt_task);
	}
//#endif
#endif /*MT_MAC */

}

#if defined (MT_MAC) && defined (CONFIG_AP_SUPPORT)
VOID RTMPHandlePreTTTTInterrupt(RTMP_ADAPTER *pAd)
{
	printk("%s\n", __func__);
	/* only AP MODE support TTTT now, p2p go don't take care of this. */
	if (pAd->OpMode == OPMODE_AP)
	{
		APMakeAllTimFrame(pAd);
	}
}
#endif

VOID RTMPHandleRxCoherentInterrupt(RTMP_ADAPTER *pAd)
{
	if (pAd == NULL)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("====> pAd is NULL, return.\n"));
		return;
	}

	DBGPRINT(RT_DEBUG_TRACE, ("==> RTMPHandleRxCoherentInterrupt \n"));

#if defined(MT7603) || defined(MT7628)
	// TODO: shiang-MT7603, fix me for upper crash case!!
	if (IS_MT7603(pAd) || IS_MT7628(pAd)) {
		return;
	}
#endif /* MT7603*/

	AsicSetWPDMA(pAd, PDMA_TX_RX, FALSE);

	RTMPRingCleanUp(pAd, QID_AC_BE);
	RTMPRingCleanUp(pAd, QID_AC_BK);
	RTMPRingCleanUp(pAd, QID_AC_VI);
	RTMPRingCleanUp(pAd, QID_AC_VO);
	RTMPRingCleanUp(pAd, QID_CTRL);
	RTMPRingCleanUp(pAd, QID_MGMT);
	RTMPRingCleanUp(pAd, QID_RX);

	RTMPEnableRxTx(pAd);

	DBGPRINT(RT_DEBUG_TRACE, ("<== RTMPHandleRxCoherentInterrupt \n"));
}


#ifdef CONFIG_AP_SUPPORT
VOID RTMPHandleMcuInterrupt(RTMP_ADAPTER *pAd)
{
	UINT32 McuIntSrc = 0;

	// TODO: shiang-7603
	if (pAd->chipCap.hif_type == HIF_MT) {
		DBGPRINT(RT_DEBUG_OFF, ("%s(): Not support for HIF_MT yet!\n",
					__FUNCTION__));
		return;
	}

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


#ifdef RLT_MAC


static inline INT rlt_rx_info_2_blk(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk, PNDIS_PACKET pRxPacket)
{
	struct _RXWI_NMAC *rxwi_n;
	UCHAR *buf_ptr;
#ifdef RT_BIG_ENDIAN
	RXINFO_STRUC *pRxInfo;
	RXINFO_STRUC RxInfo;
#endif /* RT_BIG_ENDIAN */

	pRxBlk->pRxFceInfo = (RXFCE_INFO *)&pRxBlk->hw_rx_info[RXINFO_OFFSET];
	pRxBlk->pRxInfo = (RXINFO_STRUC *)GET_OS_PKT_DATAPTR(pRxPacket);
#ifdef RT_BIG_ENDIAN
	NdisMoveMemory(&RxInfo, pRxBlk->pRxInfo, RAL_RXINFO_SIZE);
	pRxInfo = &RxInfo;
	(*(UINT32*)pRxInfo) = le2cpu32(*(UINT32*)pRxInfo);
	NdisMoveMemory(pRxBlk->pRxInfo, pRxInfo, RAL_RXINFO_SIZE);
#endif /* RT_BIG_ENDIAN */

	rxwi_n = (struct _RXWI_NMAC *)(GET_OS_PKT_DATAPTR(pRxPacket) + RAL_RXINFO_SIZE);
	pRxBlk->pRxWI = (RXWI_STRUC *)(rxwi_n);
#ifdef RT_BIG_ENDIAN
	RTMPWIEndianChange(pAd , (UCHAR *)pRxBlk->pRxWI, TYPE_RXWI);
#endif /* RT_BIG_ENDIAN */

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
	pRxBlk->rx_signal.raw_rssi[0] = rxwi_n->rssi[0];
	pRxBlk->rx_signal.raw_rssi[1] = rxwi_n->rssi[1];
	pRxBlk->rx_signal.raw_rssi[2] = rxwi_n->rssi[2];
	{
		pRxBlk->rx_signal.raw_snr[0] = rxwi_n->bbp_rxinfo[0];
		pRxBlk->rx_signal.raw_snr[1] = rxwi_n->bbp_rxinfo[1];
		pRxBlk->rx_signal.raw_snr[2] = rxwi_n->bbp_rxinfo[2];
	}
	pRxBlk->rx_signal.freq_offset = rxwi_n->bbp_rxinfo[4];
	pRxBlk->ldpc_ex_sym = rxwi_n->ldpc_ex_sym;

	SET_OS_PKT_DATAPTR(pRxPacket, GET_OS_PKT_DATAPTR(pRxPacket) + RAL_RXINFO_SIZE + pAd->chipCap.RXWISize);
	SET_OS_PKT_LEN(pRxPacket, GET_OS_PKT_LEN(pRxPacket) - RAL_RXINFO_SIZE - pAd->chipCap.RXWISize);

	return TRUE;
}
#endif /* RLT_MAC */


#ifdef RTMP_MAC
static inline INT rtmp_rx_info_2_blk(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk, PNDIS_PACKET pRxPacket)
{
	struct _RXWI_OMAC *rxwi_o = (struct _RXWI_OMAC *)(GET_OS_PKT_DATAPTR(pRxPacket));

	pRxBlk->pRxInfo = (RXINFO_STRUC *)(&pRxBlk->hw_rx_info[RXINFO_OFFSET]);
	pRxBlk->pRxWI = (RXWI_STRUC *)rxwi_o;
#ifdef RT_BIG_ENDIAN
	RTMPWIEndianChange(pAd , (UCHAR *)pRxBlk->pRxWI, TYPE_RXWI);
#endif /* RT_BIG_ENDIAN */

	pRxBlk->MPDUtotalByteCnt = rxwi_o->MPDUtotalByteCnt;
	pRxBlk->wcid = rxwi_o->wcid;
	pRxBlk->key_idx = rxwi_o->key_idx;
	pRxBlk->bss_idx = rxwi_o->bss_idx;
	pRxBlk->TID = rxwi_o->tid;
	pRxBlk->DataSize = rxwi_o->MPDUtotalByteCnt;

	pRxBlk->rx_rate.field.MODE = rxwi_o->phy_mode;
	pRxBlk->rx_rate.field.MCS = rxwi_o->mcs;
	pRxBlk->rx_rate.field.BW = rxwi_o->bw;
	pRxBlk->rx_rate.field.STBC = rxwi_o->stbc;
	pRxBlk->rx_rate.field.ShortGI = rxwi_o->sgi;
	pRxBlk->rx_signal.raw_rssi[0] = rxwi_o->RSSI0;
	pRxBlk->rx_signal.raw_rssi[1] = rxwi_o->RSSI1;
	pRxBlk->rx_signal.raw_rssi[2] = rxwi_o->RSSI2;
	pRxBlk->rx_signal.raw_snr[0] = rxwi_o->SNR0;
	pRxBlk->rx_signal.raw_snr[1] = rxwi_o->SNR1;
	pRxBlk->rx_signal.raw_snr[2] = rxwi_o->SNR2;
	pRxBlk->rx_signal.freq_offset = rxwi_o->FOFFSET;

	SET_OS_PKT_DATAPTR(pRxPacket, GET_OS_PKT_DATAPTR(pRxPacket) + pAd->chipCap.RXWISize);
	SET_OS_PKT_LEN(pRxPacket, GET_OS_PKT_LEN(pRxPacket) - pAd->chipCap.RXWISize);

	return TRUE;
}
#endif /* RTMP_MAC */


PNDIS_PACKET GetPacketFromRxRing(
	IN RTMP_ADAPTER *pAd,
	OUT RX_BLK *pRxBlk,
	OUT BOOLEAN *pbReschedule,
	INOUT UINT32 *pRxPending,
	UCHAR RxRingNo)
{
	RXD_STRUC *pRxD;
#ifdef RT_BIG_ENDIAN
	RXD_STRUC *pDestRxD;
	UCHAR rx_hw_info[RXD_SIZE];
#endif
	RTMP_RX_RING *pRxRing;
	NDIS_SPIN_LOCK *pRxRingLock;
	PNDIS_PACKET pRxPacket = NULL, pNewPacket;
	VOID *AllocVa;
	NDIS_PHYSICAL_ADDRESS AllocPa;
	BOOLEAN bReschedule = FALSE;
	RTMP_DMACB *pRxCell;
	//UINT8 RXWISize = pAd->chipCap.RXWISize;
	UINT8 rx_hw_hdr_len = pAd->chipCap.RXWISize;

	pRxRing = &pAd->RxRing[RxRingNo];
	pRxRingLock = &pAd->RxRingLock[RxRingNo];
	RTMP_SEM_LOCK(pRxRingLock);

	if (*pRxPending == 0)
	{
		/* Get how may packets had been received */
		RTMP_IO_READ32(pAd, pRxRing->hw_didx_addr, &pRxRing->RxDmaIdx);
		if (pRxRing->RxSwReadIdx == pRxRing->RxDmaIdx)
		{
			bReschedule = FALSE;
			goto done;
		}

		/* get rx pending count */
		if (pRxRing->RxDmaIdx > pRxRing->RxSwReadIdx)
			*pRxPending = pRxRing->RxDmaIdx - pRxRing->RxSwReadIdx;
		else
			*pRxPending = pRxRing->RxDmaIdx + RX_RING_SIZE - pRxRing->RxSwReadIdx;
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
	/* Point to Rx indexed rx ring descriptor */
	pRxD = (RXD_STRUC *) pRxCell->AllocVa;
#endif

	if (pRxD->DDONE == 0)
	{
		*pRxPending = 0;
		DBGPRINT(RT_DEBUG_INFO, ("DDONE=0!\n"));
		/* DMAIndx had done but DDONE bit not ready */
		bReschedule = TRUE;
		goto done;
	}

	if ((pRxD->LS0 == 0) || ((pRxD->LS0 == 1) && (pAd->DropInvalidPacket >= 1)))
	{
		if (pRxD->LS0 == 0)
			pAd->DropInvalidPacket++;
		else
			pAd->DropInvalidPacket = 0;

		if (pAd->DropInvalidPacket >= 1)
		{
			/* unmap the rx buffer*/
			PCI_UNMAP_SINGLE(pAd, pRxCell->DmaBuf.AllocPa,
			pRxCell->DmaBuf.AllocSize, RTMP_PCI_DMA_FROMDEVICE);
			/* flush dcache if no consistent memory is supported */
			RTMP_DCACHE_FLUSH(pRxCell->DmaBuf.AllocPa, pRxCell->DmaBuf.AllocSize);

			pRxPacket = pRxCell->pNdisPacket;

			DBGPRINT(RT_DEBUG_OFF, ("Ring No = %d\n", RxRingNo));
			hex_dump("Invalid Pkt content", GET_OS_PKT_DATAPTR(pRxPacket), 32);
		}

		pRxD->SDL0 = RX_BUFFER_AGGRESIZE;
		pRxD->LS0 = 0;
		pRxD->DDONE = 0;
		bReschedule = TRUE;
		*pRxPending = 0;
		pRxPacket = NULL;

		/* update rx descriptor and kick rx */
#ifdef RT_BIG_ENDIAN
		RTMPDescriptorEndianChange((PUCHAR)pRxD, TYPE_RXD);
		WriteBackToDescriptor((PUCHAR)pDestRxD, (PUCHAR)pRxD, FALSE, TYPE_RXD);
#endif

		INC_RING_INDEX(pRxRing->RxSwReadIdx, RX_RING_SIZE);

		pRxRing->RxCpuIdx = (pRxRing->RxSwReadIdx == 0) ? (RX_RING_SIZE-1) : (pRxRing->RxSwReadIdx-1);
		RTMP_IO_WRITE32(pAd, pRxRing->hw_cidx_addr, pRxRing->RxCpuIdx);

		goto done;
	}

#ifndef RT_BIG_ENDIAN
	/* return rx descriptor */
	NdisMoveMemory(&pRxBlk->hw_rx_info[0], pRxD, RXD_SIZE);
#endif /* RT_BIG_ENDIAN */

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

	if (pNewPacket && !pAd->RxReset)
	{
#ifndef DMA_CACHE_OPTIMIZE
		/* unmap the rx buffer*/
		PCI_UNMAP_SINGLE(pAd, pRxCell->DmaBuf.AllocPa,
					 pRxCell->DmaBuf.AllocSize, RTMP_PCI_DMA_FROMDEVICE);
#endif /* DMA_CACHE_OPTIMIZE */
		/* flush dcache if no consistent memory is supported */
		RTMP_DCACHE_FLUSH(pRxCell->DmaBuf.AllocPa, pRxCell->DmaBuf.AllocSize);

		pRxPacket = pRxCell->pNdisPacket;

		pRxBlk->Flags = 0;
		pRxBlk->PDMALen = pRxD->SDL0;
#ifdef MT_MAC
		if (pAd->chipCap.hif_type == HIF_MT) {
			rx_hw_hdr_len = parse_rx_packet_type(pAd, pRxBlk, pRxPacket);
			if (rx_hw_hdr_len == 0)
			{pRxBlk->DataSize = 0;}
		}
#endif /* MT_MAC */

#ifdef RLT_MAC
		if (pAd->chipCap.hif_type == HIF_RLT) {
				rlt_rx_info_2_blk(pAd, pRxBlk, pRxPacket);
		}
#endif /* RLT_MAC */

#ifdef RTMP_MAC
		if (pAd->chipCap.hif_type == HIF_RTMP)
			rtmp_rx_info_2_blk(pAd, pRxBlk, pRxPacket);
#endif /* RTMP_MAC */

		if (pRxPacket)
		{
			pRxBlk->pRxPacket = pRxPacket;
			pRxBlk->pData = (UCHAR *)GET_OS_PKT_DATAPTR(pRxPacket);
			pRxBlk->pHeader = (HEADER_802_11 *)(pRxBlk->pData);
		}

		pRxCell->DmaBuf.AllocSize = RX_BUFFER_AGGRESIZE;
		pRxCell->pNdisPacket = (PNDIS_PACKET)pNewPacket;
		pRxCell->DmaBuf.AllocVa = AllocVa;
		pRxCell->DmaBuf.AllocPa = AllocPa;

		/* flush dcache if no consistent memory is supported */
		RTMP_DCACHE_FLUSH(pRxCell->DmaBuf.AllocPa, pRxCell->DmaBuf.AllocSize);

		/* update SDP0 to new buffer of rx packet */
		pRxD->SDP0 = AllocPa;
		pRxD->SDL0 = RX_BUFFER_AGGRESIZE;
	}
	else
	{
		if (pNewPacket)
		{
			RELEASE_NDIS_PACKET(pAd, pNewPacket, NDIS_STATUS_SUCCESS);
#ifdef NEW_IXIA_METHOD
			/*RX Drop*/
			pAd->tr_ststic.rx[DROP_NO_BUF]++;
#endif
		}

		pAd->RxResetDropCount++;

		if (pAd->RxResetDropCount > 10)
		{
			pAd->RxReset = 0;
			pAd->RxResetDropCount = 0;
#ifdef MT_PS
			MtPsRecovery(pAd);
#endif /* MT_PS */
		}

		pRxD->SDL0 = RX_BUFFER_AGGRESIZE;
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

	INC_RING_INDEX(pRxRing->RxSwReadIdx, RX_RING_SIZE);

	pRxRing->RxCpuIdx = (pRxRing->RxSwReadIdx == 0) ? (RX_RING_SIZE-1) : (pRxRing->RxSwReadIdx-1);
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
		RXD_STRUC *pDestRxDLast;
#endif
		/* 16B-align */

		/* update last BD 32B-align, DMA Done bit = 0 */
		pRxCell->LastBDInfo.DDONE = 0;
#ifdef RT_BIG_ENDIAN
		pRxCellLast = &pRxRing->Cell[pRxRing->RxSwReadIdx - 1];
		pDestRxDLast = (RXD_STRUC *) pRxCellLast->AllocVa;
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
		INC_RING_INDEX(pRxRing->RxSwReadIdx, RX_RING_SIZE);
		pRxRing->RxCpuIdx = (pRxRing->RxSwReadIdx == 0) ? (RX_RING_SIZE-1) : (pRxRing->RxSwReadIdx-1);
		RTMP_IO_WRITE32(pAd, pRxRing->hw_cidx_addr, pRxRing->RxCpuIdx);
	}
	else
	{
		/* 32B-align */
		/* do not set DDONE bit and backup it */
		if (pRxRing->RxSwReadIdx >= (RX_RING_SIZE-1))
		{
			DBGPRINT(RT_DEBUG_ERROR,
					("Please change RX_RING_SIZE to mutiple of 2!\n"));

			/* flush cache from current BD */
			RTMP_DCACHE_FLUSH(pRxCell->AllocPa, RXD_SIZE);

			/* update SW read and CPU index */
			INC_RING_INDEX(pRxRing->RxSwReadIdx, RX_RING_SIZE);
			pRxRing->RxCpuIdx = (pRxRing->RxSwReadIdx == 0) ? (RX_RING_SIZE-1) : (pRxRing->RxSwReadIdx-1);
			RTMP_IO_WRITE32(pAd, pRxRing->hw_cidx_addr, pRxRing->RxCpuIdx);
		}
		else
		{
			/* backup current BD */
			pRxCell = &pRxRing->Cell[pRxRing->RxSwReadIdx + 1];
			pRxCell->LastBDInfo = *pRxD;

			/* update CPU index */
			INC_RING_INDEX(pRxRing->RxSwReadIdx, RX_RING_SIZE);
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
	UCHAR *pSrcBufVA, *tmac_info, *frm_buf;
	UINT SrcBufLen, frm_len;
	TXD_STRUC *pTxD;
#ifdef RT_BIG_ENDIAN
	TXD_STRUC *pDestTxD;
	UCHAR hw_hdr_info[TXD_SIZE];
#endif
	PHEADER_802_11 pHeader_802_11;
	PFRAME_BAR pBar = NULL;
	BOOLEAN bAckRequired, bInsertTimestamp;
	#ifdef GN_ONLY_AP_SUPPORT
	BOOLEAN BMode = TRUE, bExcludeBRate=FALSE;
#endif
	UCHAR MlmeRate, wcid, tx_rate;
	UINT32 SwIdx;
	ULONG FreeNum;
	MAC_TABLE_ENTRY *pMacEntry = NULL;
#if defined(VHT_TXBF_SUPPORT) || (!defined(MT7603) && !defined(MT7628))
	UINT8 TXWISize = pAd->chipCap.TXWISize;
#endif /* defined(VHT_TXBF_SUPPORT) || (!defined(MT7603) && !defined(MT7628)) */
	UINT8 tx_hw_hdr_len = pAd->chipCap.tx_hw_hdr_len;
	HTTRANSMIT_SETTING *transmit, TransmitSetting;
	MAC_TX_INFO mac_info;
#ifdef CONFIG_AP_SUPPORT
#ifdef SPECIFIC_TX_POWER_SUPPORT
	UCHAR TxPwrAdj = 0;
#endif /* SPECIFIC_TX_POWER_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
	UCHAR prot = 0;
	UCHAR apidx = 0;
	
	RTMP_QueryPacketInfo(pPacket, &PacketInfo, &pSrcBufVA, &SrcBufLen);
	if (pSrcBufVA == NULL)
		return NDIS_STATUS_FAILURE;

	FreeNum = GET_TXRING_FREENO(pAd, QueIdx);
	if (FreeNum == 0)
		return NDIS_STATUS_FAILURE;

	SwIdx = pAd->TxRing[QueIdx].TxCpuIdx;
#ifdef RT_BIG_ENDIAN
	pDestTxD  = (TXD_STRUC *)pAd->TxRing[QueIdx].Cell[SwIdx].AllocVa;
	NdisMoveMemory(&hw_hdr_info[0], pDestTxD, TXD_SIZE);
	pTxD = (TXD_STRUC *)&hw_hdr_info[0];
	RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#else
	pTxD  = (TXD_STRUC *) pAd->TxRing[QueIdx].Cell[SwIdx].AllocVa;
#endif

	if (pAd->TxRing[QueIdx].Cell[SwIdx].pNdisPacket)
	{
		DBGPRINT(RT_DEBUG_OFF, ("MlmeHardTransmit Error\n"));
		return NDIS_STATUS_FAILURE;
	}


#if defined(MT7603) || defined(MT7628)
	// TODO: shiang-7603
	frm_buf = pSrcBufVA;
	frm_len = SrcBufLen;
	tmac_info = pSrcBufVA;
//DBGPRINT(RT_DEBUG_OFF, ("%s(): pSrcBufVA=0x%p, pHeader_802_11=0x%p, tmac_info=%p, tx_hw_hdr_len=%d\n",
//			__FUNCTION__, pSrcBufVA, pHeader_802_11, tmac_info, tx_hw_hdr_len));
#else
	frm_buf = (UCHAR *)(pSrcBufVA + TXINFO_SIZE);
	frm_len = SrcBufLen - TXINFO_SIZE;
	tmac_info =(UCHAR *)(pSrcBufVA + TXINFO_SIZE);
#endif /* MT7603 */
	pHeader_802_11 = (HEADER_802_11 *)(pSrcBufVA + tx_hw_hdr_len);
#ifdef GN_ONLY_AP_SUPPORT
	if((pAd->OpMode == OPMODE_STA) || (pAd->OpMode == OPMODE_AP)) {
        UCHAR BssIdx,i,CCK_rate;
	    BSS_ENTRY *pEntry01;
	    bExcludeBRate = pAd->CommonCfg.bExcludeBRate;
	    
	    if(pHeader_802_11->FC.Type == FC_TYPE_MGMT) {
		    BssIdx = BssTableSearch(&pAd->ScanTab, pHeader_802_11->Addr3, pAd->LatchRfRegs.Channel);
		    pEntry01 = &pAd->ScanTab.BssEntry[BssIdx];
		    if(BssIdx < 0xFF) {
		        BMode = FALSE;
		        for(i=0 ; i < MAX_LEN_OF_SUPPORTED_RATES;i++) {
		            CCK_rate = pEntry01->SupRate[i] & 0x7f;
			        if (CCK_rate == 2 || CCK_rate == 4 || CCK_rate == 11 || CCK_rate == 22) {
			            BMode = TRUE;
			            break;
			        }
		        }
	       	}
	    }	
	    if(!BMode || bExcludeBRate){
		    pAd->CommonCfg.MlmeTransmit.field.MODE = MODE_OFDM;
		    pAd->CommonCfg.MlmeTransmit.field.MCS = MCS_RATE_6;
	    }
	}
#endif
	if (pHeader_802_11->Addr1[0] & 0x01)
		MlmeRate = pAd->CommonCfg.BasicMlmeRate;
	else
		MlmeRate = pAd->CommonCfg.MlmeRate;

	if (((pHeader_802_11->FC.Type == FC_TYPE_DATA) &&
		(pHeader_802_11->FC.SubType == SUBTYPE_QOS_NULL)) || 
		((pHeader_802_11->FC.Type == FC_TYPE_CNTL) &&
		(pHeader_802_11->FC.SubType == SUBTYPE_BLOCK_ACK_REQ)))
	{
		pMacEntry = MacTableLookup(pAd, pHeader_802_11->Addr1);
#ifdef GN_ONLY_AP_SUPPORT
        if((pAd->OpMode == OPMODE_STA) || (pAd->OpMode == OPMODE_AP)) {
            if(!(pMacEntry->SupportRateMode & 0x01))
                BMode=FALSE;
        }
#endif		
#ifdef MAC_REPEATER_SUPPORT
		if (pMacEntry != NULL &&  pAd->ApCfg.bMACRepeaterEn && IS_ENTRY_APCLI(pMacEntry)) 
		{
			REPEATER_CLIENT_ENTRY *pReptEntry = NULL;
			UCHAR MacTabWCID=0;
			UCHAR isLinkValid;

			pReptEntry = RTMPLookupRepeaterCliEntry(pAd, FALSE, pHeader_802_11->Addr2, TRUE, &isLinkValid);
			if (pReptEntry && pReptEntry->CliValid)
			{
				MacTabWCID = pReptEntry->MacTabWCID;
				pMacEntry = &pAd->MacTab.Content[MacTabWCID];
			}

		}
#endif
	}

#ifdef DOT11W_PMF_SUPPORT
	if (pAd->chipCap.hif_type == HIF_MT)
	{
		UINT32 ret = 0;
		MAC_TABLE_ENTRY *pEntry = NULL;

		pEntry = MacTableLookup(pAd, pHeader_802_11->Addr1);
#ifdef MAC_REPEATER_SUPPORT
		if (pEntry != NULL &&  pAd->ApCfg.bMACRepeaterEn && IS_ENTRY_APCLI(pEntry)) 
		{
			REPEATER_CLIENT_ENTRY *pReptEntry = NULL;
			UCHAR MacTabWCID=0;
			UCHAR isLinkValid;

			pReptEntry = RTMPLookupRepeaterCliEntry(pAd, FALSE, pHeader_802_11->Addr2, TRUE, &isLinkValid);
			if (pReptEntry && pReptEntry->CliValid)
			{
				MacTabWCID = pReptEntry->MacTabWCID;
				pEntry = &pAd->MacTab.Content[MacTabWCID];
			}
		}
#endif
		ret = PMF_RobustFrameClassify(
					(PHEADER_802_11)pHeader_802_11,
					(PUCHAR)(((PUCHAR)pHeader_802_11)+LENGTH_802_11),
					(SrcBufLen - LENGTH_802_11 - tx_hw_hdr_len),
					(PUCHAR) pEntry,
					FALSE);

		if (pEntry)
			apidx = pEntry->func_tb_idx;

		if (ret == UNICAST_ROBUST_FRAME)
		{
			prot = 1;
			pHeader_802_11->FC.Wep = 1;
		}
		else if (ret == GROUP_ROBUST_FRAME)
		{
			ret = PMF_EncapBIPAction(pAd,
						(PUCHAR)pHeader_802_11,
						(SrcBufLen - tx_hw_hdr_len));
			if (ret == PMF_STATUS_SUCCESS)
				prot = 2;
			else
				DBGPRINT(RT_DEBUG_OFF, ("%s, PMF GROUP ROBUST Encap fail, ret=%d\n",
					__FUNCTION__, ret));
		}
	}
#endif

	/* Verify Mlme rate for a/g bands.*/
	if ((pAd->LatchRfRegs.Channel > 14) && (MlmeRate < RATE_6)) /* 11A band*/
		MlmeRate = RATE_6;
#ifdef GN_ONLY_AP_SUPPORT
        if((pAd->OpMode == OPMODE_STA) || (pAd->OpMode == OPMODE_AP)) {
            if(!BMode)
                MlmeRate= RATE_6;
        }
#endif
	/*
		Should not be hard code to set PwrMgmt to 0 (PWR_ACTIVE)
		Snice it's been set to 0 while on MgtMacHeaderInit
		By the way this will cause frame to be send on PWR_SAVE failed.
	*/

	/* In WMM-UAPSD, mlme frame should be set psm as power saving but probe request frame */

	bInsertTimestamp = FALSE;

	if (pHeader_802_11->FC.Type == FC_TYPE_CNTL)
	{
		if (pHeader_802_11->FC.SubType == SUBTYPE_BLOCK_ACK_REQ)
		{
			pBar = (PFRAME_BAR)(pSrcBufVA + tx_hw_hdr_len);
			bAckRequired = TRUE;
		}
		else
		{
			bAckRequired = FALSE;
		}
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
				/* Find which MBSSID to be send this probeRsp */
				UINT32 apidx = get_apidx_by_addr(pAd, pHeader_802_11->Addr2);

				if (!(apidx >= pAd->ApCfg.BssidNum) &&
				     (pAd->ApCfg.MBSSID[apidx].TxPwrAdj != -1) &&
				     (pAd->CommonCfg.MlmeTransmit.field.MODE == MODE_CCK) &&
				     (pAd->CommonCfg.MlmeTransmit.field.MCS == RATE_1))
				{
					TxPwrAdj = pAd->ApCfg.MBSSID[apidx].TxPwrAdj;
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

	/*
		Fill scatter-and-gather buffer list into TXD. Internally created NDIS PACKET
		should always has only one ohysical buffer, and the whole frame size equals
		to the first scatter buffer size

		Initialize TX Descriptor
		For inter-frame gap, the number is for this frame and next frame
		For MLME rate, we will fix as 2Mb to match other vendor's implement
	*/

/* management frame doesn't need encryption. so use RESERVED_WCID no matter u are sending to specific wcid or not */
	/* Only beacon use Nseq=TRUE. So here we use Nseq=FALSE.*/
	if (pMacEntry == NULL)
	{
		wcid = RESERVED_WCID;
		if (IS_MT7603(pAd) || IS_MT7628(pAd))
		{
			wcid = 0;
			if(prot)
			{
				MAC_TABLE_ENTRY *pEntry = NULL;
				
				pEntry = MacTableLookup(pAd, pHeader_802_11->Addr1);
#ifdef MAC_REPEATER_SUPPORT
				if (pEntry != NULL &&  pAd->ApCfg.bMACRepeaterEn && IS_ENTRY_APCLI(pEntry)) 
				{
					REPEATER_CLIENT_ENTRY *pReptEntry = NULL;
					UCHAR MacTabWCID=0;
					UCHAR isLinkValid;

					pReptEntry = RTMPLookupRepeaterCliEntry(pAd, FALSE, pHeader_802_11->Addr2, TRUE, &isLinkValid);
					if (pReptEntry && pReptEntry->CliValid)
					{
						MacTabWCID = pReptEntry->MacTabWCID;
						pEntry = &pAd->MacTab.Content[MacTabWCID];
					}

				}
#endif

				if (pEntry)
					wcid = pEntry->Aid;
			}
		}
		tx_rate = (UCHAR)pAd->CommonCfg.MlmeTransmit.field.MCS;
		transmit = &pAd->CommonCfg.MlmeTransmit;
	}
	else
	{
		wcid = pMacEntry->Aid;
		tx_rate = (UCHAR)pMacEntry->MaxHTPhyMode.field.MCS;
		transmit = &pMacEntry->MaxHTPhyMode;
	}

	NdisZeroMemory((UCHAR *)&mac_info, sizeof(mac_info));

	if(prot)
		mac_info.prot = prot;

	if (prot == 2)
		mac_info.bss_idx = apidx;

	mac_info.FRAG = FALSE;

	mac_info.CFACK = FALSE;
	mac_info.InsTimestamp = bInsertTimestamp;
	mac_info.AMPDU = FALSE;

	mac_info.BM = IS_BM_MAC_ADDR(pHeader_802_11->Addr1);
	mac_info.Ack = bAckRequired;
	mac_info.NSeq = FALSE;
	mac_info.BASize = 0;
	mac_info.WCID = wcid;
	mac_info.TID = 0;
#ifdef MT_MAC
	mac_info.q_idx = QueIdx;
#endif /* MT_MAC */

#if defined(MT7603) || defined(MT7628)
	mac_info.Length = (SrcBufLen - tx_hw_hdr_len);
	if (pHeader_802_11->FC.Type == FC_TYPE_MGMT) {
		mac_info.hdr_len = 24;
		if (pHeader_802_11->FC.Order == 1)
			mac_info.hdr_len += 4;
		if (pHeader_802_11->FC.SubType == SUBTYPE_BEACON)
			mac_info.q_idx = Q_IDX_BCN;
		mac_info.PID = PID_MGMT;
	} else if (pHeader_802_11->FC.Type == FC_TYPE_DATA) {
		switch (pHeader_802_11->FC.SubType) {
			case SUBTYPE_DATA_NULL:
				mac_info.hdr_len = 24;
				tx_rate = (UCHAR)pAd->CommonCfg.MlmeTransmit.field.MCS;
				transmit = &pAd->CommonCfg.MlmeTransmit;
				break;
			case SUBTYPE_QOS_NULL:
				mac_info.hdr_len = 26;
				tx_rate = (UCHAR)pAd->CommonCfg.MlmeTransmit.field.MCS;
				transmit = &pAd->CommonCfg.MlmeTransmit;
				break;
			default:
				DBGPRINT(RT_DEBUG_ERROR, ("%s(): FIXME!!!Unexpected frame(Type=%d, SubType=%d) send to MgmtRing, need to assign the length!\n",
						__FUNCTION__, pHeader_802_11->FC.Type, pHeader_802_11->FC.SubType));
				hex_dump("DataFrame", frm_buf, frm_len);
				break;
		}
		mac_info.WCID = wcid;
		if (pMacEntry && pAd->MacTab.tr_entry[wcid].PsDeQWaitCnt)
			mac_info.PID = PID_PS_DATA;
		else
			mac_info.PID = PID_MGMT;
	} else if (pHeader_802_11->FC.Type == FC_TYPE_CNTL) {
		switch (pHeader_802_11->FC.SubType) {
			case SUBTYPE_BLOCK_ACK_REQ:
				mac_info.PID = PID_CTL_BAR;
				mac_info.hdr_len = 16;
				mac_info.SpeEn = 0;
				mac_info.TID = pBar->BarControl.TID; 
				if (pAd->CommonCfg.Channel > 14)
				{ /* 2.4G */
					TransmitSetting.field.MODE = MODE_OFDM;
				}
				else
				{ /* 5G */
#ifdef GN_ONLY_AP_SUPPORT
					if((pAd->OpMode == OPMODE_STA) || (pAd->OpMode == OPMODE_AP)) {
                        if(BMode || !bExcludeBRate)
                            TransmitSetting.field.MODE = MODE_CCK;
                        else
                            TransmitSetting.field.MODE = MODE_OFDM;
                    }
                    else
#endif			
					TransmitSetting.field.MODE = MODE_CCK;
				}
				TransmitSetting.field.BW = BW_20;
				TransmitSetting.field.STBC = 0;
				TransmitSetting.field.ShortGI = 0;
				TransmitSetting.field.MCS = 0;
				TransmitSetting.field.ldpc = 0;
				transmit = &TransmitSetting;
				break;
			default:
				DBGPRINT(RT_DEBUG_ERROR, ("%s(): FIXME!!!Unexpected frame(Type=%d, SubType=%d) send to MgmtRing, need to assign the length!\n",
						__FUNCTION__, pHeader_802_11->FC.Type, pHeader_802_11->FC.SubType));
				hex_dump("Control Frame", frm_buf, frm_len);
				break;
		}
	}
#else
	mac_info.Length = (SrcBufLen - TXINFO_SIZE - TXWISize - TSO_SIZE);
	mac_info.PID = PID_MGMT;
#endif /* defined(MT7603) || defined(MT7628) */

	mac_info.TxRate = tx_rate;
	mac_info.Txopmode = IFS_BACKOFF;
	mac_info.SpeEn = 1;
	mac_info.Preamble = LONG_PREAMBLE;

	/* PCI use Miniport to send NULL frame and need to add NULL frame TxS control here to enter PSM */
	if ((pHeader_802_11->FC.Type == FC_TYPE_MGMT) && 
	    ((pHeader_802_11->FC.SubType == SUBTYPE_ASSOC_RSP) || (pHeader_802_11->FC.SubType == SUBTYPE_REASSOC_RSP))) {
	    MAC_TABLE_ENTRY *pEntry = NULL;
	    mac_info.PID = PID_MGMT_ASSOC_RSP;
        DBGPRINT(RT_DEBUG_TRACE,("===[%s] ASSOC_RSP(pci):addr1 %02x:%02x:%02x:%02x:%02x:%02x addr2 %02x:%02x:%02x:%02x:%02x:%02x\n", 
			__FUNCTION__, PRINT_MAC(pHeader_802_11->Addr1), PRINT_MAC(pHeader_802_11->Addr2)));
        pEntry = MacTableLookup(pAd, pHeader_802_11->Addr1);
        if(pEntry) {
	        DBGPRINT(RT_DEBUG_TRACE,("===[%s] pEntry->wcid = %d \n", __FUNCTION__, pEntry->wcid));
	        mac_info.WCID = pEntry->wcid;
	    } else {
		    DBGPRINT(RT_DEBUG_TRACE,("===[%s] ERROR: pEntry == NULL \n", __FUNCTION__));
	    }
	}
	
	write_tmac_info(pAd, tmac_info, &mac_info, transmit);

#ifdef SPECIFIC_TX_POWER_SUPPORT
	if (IS_RT6352(pAd) && (pMacEntry == NULL)) {
		TXWI_STRUC *pFirstTxWI = (TXWI_STRUC *)tmac_info;

		pFirstTxWI->TxPwrAdj = TxPwrAdj;
	}
#endif /* SPECIFIC_TX_POWER_SUPPORT */

	pAd->TxRing[QueIdx].Cell[SwIdx].pNdisPacket = pPacket;
	pAd->TxRing[QueIdx].Cell[SwIdx].pNextNdisPacket = NULL;

#ifdef RT_BIG_ENDIAN
#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
		MTMacInfoEndianChange(pAd, tmac_info, TYPE_TMACINFO, sizeof(TMAC_TXD_L));
#endif

#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (pAd->chipCap.hif_type == HIF_RTMP || pAd->chipCap.hif_type == HIF_RLT)
		RTMPWIEndianChange(pAd, tmac_info, TYPE_TXWI);
#endif
#endif
	
#ifdef VHT_TXBF_SUPPORT
	if (pHeader_802_11->FC.Type == FC_TYPE_CNTL && pHeader_802_11->FC.SubType == SUBTYPE_VHT_NDPA)
	{
		DBGPRINT(RT_DEBUG_OFF, ("%s(): Send VhtNDPA to peer(wcid=%d, pMacEntry=%p) with dataRate(PhyMode:%s, BW:%sHz, %dSS, MCS%d)\n",
					__FUNCTION__, wcid, pMacEntry, get_phymode_str(transmit->field.MODE),
					get_bw_str(transmit->field.BW),
					(transmit->field.MCS>>4) + 1, (transmit->field.MCS & 0xf)));

		hex_dump("VHT NDPA frame raw data", pSrcBufVA, SrcBufLen);
	}
#endif /* VHT_TXBF_SUPPORT */

#ifdef RT_BIG_ENDIAN
	RTMPFrameEndianChange(pAd, (PUCHAR)pHeader_802_11, DIR_WRITE, FALSE);
#endif

	pAd->TxRing[QueIdx].Cell[SwIdx].PacketPa =  PCI_MAP_SINGLE(pAd, frm_buf, frm_len, 0, RTMP_PCI_DMA_TODEVICE);

	pTxD->LastSec0 = 1;
	pTxD->LastSec1 = 0;
	pTxD->SDLen0 = frm_len;
	pTxD->SDLen1 = 0;
	pTxD->SDPtr0 = pAd->TxRing[QueIdx].Cell[SwIdx].PacketPa;
	pTxD->Burst = 0;

#if defined(RTMP_MAC) || defined(RLT_MAC)
	ral_write_txd(pAd, pTxD, NULL, TRUE, FIFO_EDCA);
#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */

#ifdef MT_MAC
	mt_write_txd(pAd, pTxD);
#endif /* MT_MAC */

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
	//RXD_STRUC *pRxD = NULL;
	//RXINFO_STRUC *pRxInfo = NULL;
	//UCHAR *pData = NULL;
	PNDIS_PACKET pRxPacket = NULL;
	//PHEADER_802_11 pHeader = NULL;
	RX_BLK rxblk, *pRxBlk = NULL;
	//UINT8 RXWISize = pAd->chipCap.RXWISize;
#ifdef RLT_MAC
	RXFCE_INFO *pFceInfo;
#endif /* RLT_MAC */

	RxProcessed = RxPending = 0;

	/* process whole rx ring */
	while (1)
	{

		if (RTMP_TEST_FLAG(pAd, (fRTMP_ADAPTER_RADIO_OFF |
								fRTMP_ADAPTER_RESET_IN_PROGRESS
									//| fRTMP_ADAPTER_HALT_IN_PROGRESS)//Carter commented. otherwise TXS might be droped.
            )) ||
			!RTMP_TEST_FLAG(pAd,fRTMP_ADAPTER_START_UP))
		{
			break;
		}

#ifdef RTMP_MAC_PCI
		//if (RxProcessed++ > MAX_RX_PROCESS_CNT)
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

		pRxPacket = GetPacketFromRxRing(pAd, pRxBlk, &bReschedule, &RxPending, 1);
		if (pRxPacket == NULL)
			break;

		/* get rx descriptor and data buffer */
		//pRxD = (RXD_STRUC *)&pRxBlk->hw_rx_info[0];
		//pRxInfo = rxblk.pRxInfo;
		//pData = GET_OS_PKT_DATAPTR(pRxPacket);
		//pHeader = (PHEADER_802_11)(pData + RXWISize);

#ifdef RLT_MAC
		pFceInfo = rxblk.pRxFceInfo;
		if (pFceInfo->info_type == CMD_PACKET)
		{
			DBGPRINT(RT_DEBUG_INFO, ("%s: Receive command packet.\n", __FUNCTION__));
			pci_rx_cmd_msg_complete(pAd, pFceInfo);
			RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_SUCCESS);
			continue;
		}
#endif /* RLT_MAC */

#ifdef MT_MAC
		if (pAd->chipCap.hif_type == HIF_MT) {
                   if ((rxblk.DataSize == 0) && (pRxPacket)) {
				RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_SUCCESS);
				continue;
            }
		}
#endif /* MT_MAC */
	}
	return bReschedule;
}


VOID RTMPHandleTxRing8DmaDoneInterrupt(RTMP_ADAPTER *pAd)
{
	TXD_STRUC *pTxD;
#ifdef RT_BIG_ENDIAN
    TXD_STRUC *pDestTxD;
	UCHAR hw_hdr_info[TXD_SIZE];
#endif
	PNDIS_PACKET pPacket;
/*	int 		 i;*/
	UCHAR	FREE = 0;
	RTMP_CTRL_RING *pCtrlRing = &pAd->CtrlRing;
	//UINT8 TXWISize = pAd->chipCap.TXWISize;

	NdisAcquireSpinLock(&pAd->CtrlRingLock);

	RTMP_IO_READ32(pAd, pCtrlRing->hw_didx_addr, &pCtrlRing->TxDmaIdx);
	while (pCtrlRing->TxSwFreeIdx!= pCtrlRing->TxDmaIdx)
	{
		FREE++;
#ifdef RT_BIG_ENDIAN
        pDestTxD = (TXD_STRUC *) (pCtrlRing->Cell[pCtrlRing->TxSwFreeIdx].AllocVa);
		NdisMoveMemory(&hw_hdr_info[0], pDestTxD, TXD_SIZE);
		pTxD = (TXD_STRUC *)&hw_hdr_info[0];
		RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#else
		pTxD = (TXD_STRUC *) (pCtrlRing->Cell[pCtrlRing->TxSwFreeIdx].AllocVa);
#endif

		pPacket = pCtrlRing->Cell[pCtrlRing->TxSwFreeIdx].pNdisPacket;

		if (pPacket == NULL)
		{
			INC_RING_INDEX(pCtrlRing->TxSwFreeIdx, MGMT_RING_SIZE);
			continue;
		}

		if (pPacket)
		{
			PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr0, pTxD->SDLen0, RTMP_PCI_DMA_TODEVICE);
			RTMPFreeNdisPacket(pAd, pPacket);
		}

		pCtrlRing->Cell[pCtrlRing->TxSwFreeIdx].pNdisPacket = NULL;

		/* flush dcache if no consistent memory is supported */
		RTMP_DCACHE_FLUSH(pCtrlRing->Cell[pCtrlRing->TxSwFreeIdx].AllocPa, TXD_SIZE);

		INC_RING_INDEX(pCtrlRing->TxSwFreeIdx, MGMT_RING_SIZE);

#ifdef RT_BIG_ENDIAN
		RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
		WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, TRUE, TYPE_TXD);
#endif
	}

	NdisReleaseSpinLock(&pAd->CtrlRingLock);
}
#endif /* CONFIG_ANDES_SUPPORT */


#ifdef MT_MAC
VOID RTMPHandleBcnDmaDoneInterrupt(RTMP_ADAPTER *pAd)
{
	TXD_STRUC *pTxD;
#ifdef RT_BIG_ENDIAN
    TXD_STRUC *pDestTxD;
	UCHAR hw_hdr_info[TXD_SIZE];
#endif
	PNDIS_PACKET pPacket;
	RTMP_BCN_RING *pBcnRing = &pAd->BcnRing;
	UCHAR *tmac_info = NULL;

#ifdef CONFIG_AP_SUPPORT
    BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[0];
    UCHAR bss_idx = 0;
    UINT32   Lowpart, Highpart;
DBGPRINT(RT_DEBUG_FPGA, ("-->%s():bcn_state=%d\n", __FUNCTION__, pAd->ApCfg.MBSSID[0].bcn_buf.bcn_state));
#endif /* CONFIG_AP_SUPPORT */

	RTMP_SEM_LOCK(&pAd->BcnRingLock);

	RTMP_IO_READ32(pAd, pBcnRing->hw_didx_addr, &pBcnRing->TxDmaIdx);
	while (pBcnRing->TxSwFreeIdx!= pBcnRing->TxDmaIdx)
	{
#ifdef RT_BIG_ENDIAN
        pDestTxD = (TXD_STRUC *) (pBcnRing->Cell[pBcnRing->TxSwFreeIdx].AllocVa);
		NdisMoveMemory(&hw_hdr_info[0], pDestTxD, TXD_SIZE);
		pTxD = (TXD_STRUC *)&hw_hdr_info[0];
		RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#else
		pTxD = (TXD_STRUC *) (pBcnRing->Cell[pBcnRing->TxSwFreeIdx].AllocVa);
#endif

		pPacket = pBcnRing->Cell[pBcnRing->TxSwFreeIdx].pNdisPacket;
		if (pPacket == NULL)
		{
			INC_RING_INDEX(pBcnRing->TxSwFreeIdx, BCN_RING_SIZE);
			continue;
		}

		PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr0, pTxD->SDLen0, RTMP_PCI_DMA_TODEVICE);
		tmac_info = GET_OS_PKT_DATAPTR(pPacket);
		pBcnRing->Cell[pBcnRing->TxSwFreeIdx].pNdisPacket = NULL;

		/* flush dcache if no consistent memory is supported */
		RTMP_DCACHE_FLUSH(pBcnRing->Cell[pBcnRing->TxSwFreeIdx].AllocPa, TXD_SIZE);

#ifdef CONFIG_AP_SUPPORT//Carter commented for update beacon by TxS, instead of dma_done
		// TODO: shiang-MT7603, big endian!!
		if (tmac_info) {
//+++Add by Carter for MT7603
			TMAC_TXD_1 *txd_1;
#ifdef RT_BIG_ENDIAN
			TMAC_TXD_1 temp_txd1;
#endif

//---Add by Carter for MT7603
			//BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[0];

//+++Add by Carter for MT7603
			txd_1 = (TMAC_TXD_1 *)(tmac_info + sizeof(TMAC_TXD_0));
#ifdef RT_BIG_ENDIAN
		NdisCopyMemory(&temp_txd1, txd_1, sizeof(TMAC_TXD_1));
		*(((UINT32 *)&temp_txd1)) = SWAP32(*(((UINT32 *)&temp_txd1)));
		txd_1 = &temp_txd1;
#endif /* RT_BIG_ENDIAN */				

			if (txd_1->own_mac > 0x10 && txd_1->own_mac <= 0x1f)
				bss_idx = txd_1->own_mac & 0x0f;
			else if (txd_1->own_mac == 0)
				bss_idx = txd_1->own_mac;

			pMbss = &pAd->ApCfg.MBSSID[bss_idx];
//---Add by Carter for MT7603

            if (pMbss->bcn_buf.bcn_state == BCN_TX_WRITE_TO_DMA)
                pMbss->bcn_buf.bcn_state = BCN_TX_DMA_DONE;

            AsicGetTsfTime(pAd, &Highpart, &Lowpart);
            pMbss->BcnDmaDoneTime[pMbss->timer_loop] = Lowpart;//update TSF time to corresponding field.

			DBGPRINT(RT_DEBUG_INFO, ("%s():change state as idle\n", __FUNCTION__));
		}
		else
		{
			DBGPRINT(RT_DEBUG_ERROR, ("%s():Err, cannot found tmac_info!\n", __FUNCTION__));
		}
#endif /* CONFIG_AP_SUPPORT */

		INC_RING_INDEX(pBcnRing->TxSwFreeIdx, BCN_RING_SIZE);

#ifdef RT_BIG_ENDIAN
		RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
		WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, TRUE, TYPE_TXD);
#endif

	}

	RTMP_SEM_UNLOCK(&pAd->BcnRingLock);


}
#endif /* MT_MAC */


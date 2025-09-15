/****************************************************************************
 * Ralink Tech Inc.
 * Taiwan, R.O.C.
 *
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************/



#ifdef RTMP_MAC_PCI
#include	"rt_config.h"


static INT desc_ring_alloc(RTMP_ADAPTER *pAd, RTMP_DMABUF *pDescRing, INT size)
{
	VOID *pci_dev = ((POS_COOKIE)(pAd->OS_Cookie))->pci_dev;

	pDescRing->AllocSize = size;
	RtmpAllocDescBuf(pci_dev,
				0,
				pDescRing->AllocSize,
				FALSE,
				&pDescRing->AllocVa,
				&pDescRing->AllocPa);

	if (pDescRing->AllocVa == NULL)
	{
		DBGPRINT_ERR(("Failed to allocate a big buffer\n"));
		return ERRLOG_OUT_OF_SHARED_MEMORY;
	}

	/* Zero init this memory block*/
	NdisZeroMemory(pDescRing->AllocVa, size);

	return 0;
}


static INT desc_ring_free(RTMP_ADAPTER *pAd, RTMP_DMABUF *pDescRing)
{
	VOID *pci_dev = ((POS_COOKIE)(pAd->OS_Cookie))->pci_dev;
	
	if (pDescRing->AllocVa)
	{
		RtmpFreeDescBuf(pci_dev, 
						pDescRing->AllocSize,
						pDescRing->AllocVa,
						pDescRing->AllocPa);
	}
	NdisZeroMemory(pDescRing, sizeof(RTMP_DMABUF));
	
	return TRUE;
}


#ifdef RESOURCE_PRE_ALLOC
VOID RTMPResetTxRxRingMemory(RTMP_ADAPTER *pAd)
{
	int index, j;
	RTMP_TX_RING *pTxRing;
	TXD_STRUC *pTxD;
	RTMP_DMACB *dma_cb;
#ifdef RT_BIG_ENDIAN
	TXD_STRUC *pDestTxD;
	UCHAR tx_hw_info[TXD_SIZE];
#endif /* RT_BIG_ENDIAN */
	PNDIS_PACKET pPacket;
	ULONG IrqFlags;

	RTMP_IRQ_LOCK(&pAd->irq_lock, IrqFlags);

	/* Free TxSwQueue Packet*/
	for (index=0; index <NUM_OF_TX_RING; index++)
	{
		QUEUE_ENTRY *pEntry;
		QUEUE_HEADER *pQueue;

		pQueue = &pAd->TxSwQueue[index];
		while (pQueue->Head)
		{
			pEntry = RemoveHeadQueue(pQueue);
			pPacket = QUEUE_ENTRY_TO_PACKET(pEntry);
			if (pPacket)
			RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
		}
	}

	/* Free Tx Ring Packet*/
	for (index=0;index< NUM_OF_TX_RING;index++)
	{
		pTxRing = &pAd->TxRing[index];
		for (j=0; j< TX_RING_SIZE; j++)
		{
			dma_cb = &pTxRing->Cell[j];
#ifdef RT_BIG_ENDIAN
			pDestTxD = (PTXD_STRUC)(dma_cb->AllocVa);
			NdisMoveMemory(&tx_hw_info[0], (UCHAR *)pDestTxD, TXD_SIZE);
			pTxD = (TXD_STRUC *)&tx_hw_info[0];
			RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#else
			pTxD = (PTXD_STRUC) (dma_cb->AllocVa);
#endif /* RT_BIG_ENDIAN */
			pPacket = dma_cb->pNdisPacket;

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

#ifdef RT_BIG_ENDIAN
			RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
			WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, FALSE, TYPE_TXD);
#endif /* RT_BIG_ENDIAN */
		}
	}	

	RTMP_IRQ_UNLOCK(&pAd->irq_lock, IrqFlags);

	for (index=0;index< NUM_OF_RX_RING;index++)
	{
		UINT16 RxRingSize = (index == 0) ? RX_RING_SIZE : RX1_RING_SIZE;
		
		RTMP_IRQ_LOCK(&pAd->RxRingLock[index], IrqFlags);
		
		for (j = RxRingSize - 1 ; j >= 0; j--)
		{
			dma_cb = &pAd->RxRing[index].Cell[j];
			if ((dma_cb->DmaBuf.AllocVa) && (dma_cb->pNdisPacket))
			{
				PCI_UNMAP_SINGLE(pAd, dma_cb->DmaBuf.AllocPa, dma_cb->DmaBuf.AllocSize, RTMP_PCI_DMA_FROMDEVICE);
				RELEASE_NDIS_PACKET(pAd, dma_cb->pNdisPacket, NDIS_STATUS_SUCCESS);
			}
		}
		NdisZeroMemory(pAd->RxRing[index].Cell, RX_RING_SIZE * sizeof(RTMP_DMACB));
		
		RTMP_IRQ_UNLOCK(&pAd->RxRingLock[index], IrqFlags);
	}

	if (pAd->FragFrame.pFragPacket)
	{
		RELEASE_NDIS_PACKET(pAd, pAd->FragFrame.pFragPacket, NDIS_STATUS_SUCCESS);
		pAd->FragFrame.pFragPacket = NULL;
	}

	NdisFreeSpinLock(&pAd->CmdQLock);
}


VOID RTMPFreeTxRxRingMemory(RTMP_ADAPTER *pAd)
{
	INT num;
	VOID *pci_dev = ((POS_COOKIE)(pAd->OS_Cookie))->pci_dev;

	DBGPRINT(RT_DEBUG_TRACE, ("--> RTMPFreeTxRxRingMemory\n"));


	/* Free Rx/Mgmt Desc buffer*/
	for (num = 0; num < NUM_OF_RX_RING; num++)
	{
#ifdef RT_SECURE_DMA
		if (pAd->RxSecureDMA[num].AllocVa)
		{

			NdisZeroMemory(pAd->RxSecureDMA[num].AllocVa, pAd->RxSecureDMA[num].AllocSize);
			RTMP_FreeFirstTxBuffer(pci_dev,
								   pAd->RxSecureDMA[num].AllocSize,
								   FALSE, pAd->RxSecureDMA[num].AllocVa,
								   pAd->RxSecureDMA[num].AllocPa);
		}
#endif
		desc_ring_free(pAd, &pAd->RxDescRing[num]);
	}


#ifdef RT_SECURE_DMA
	if (pAd->MgmtSecureDMA.AllocVa)
	{

		NdisZeroMemory(pAd->MgmtSecureDMA.AllocVa, pAd->MgmtSecureDMA.AllocSize);
		RTMP_FreeFirstTxBuffer(pci_dev,
							   pAd->MgmtSecureDMA.AllocSize,
							   FALSE, pAd->MgmtSecureDMA.AllocVa,
							   pAd->MgmtSecureDMA.AllocPa);
	}
#endif
	desc_ring_free(pAd, &pAd->MgmtDescRing);
#ifdef CONFIG_ANDES_SUPPORT
#ifdef RT_SECURE_DMA
	if (pAd->CtrlSecureDMA.AllocVa)
	{

		NdisZeroMemory(pAd->CtrlSecureDMA.AllocVa, pAd->CtrlSecureDMA.AllocSize);
		RTMP_FreeFirstTxBuffer(pci_dev,
							   pAd->CtrlSecureDMA.AllocSize,
							   FALSE, pAd->CtrlSecureDMA.AllocVa,
							   pAd->CtrlSecureDMA.AllocPa);
	}
#endif
	desc_ring_free(pAd, &pAd->CtrlDescRing);
#endif /* CONFIG_ANDES_SUPPORT */

	/* Free 1st TxBufSpace and TxDesc buffer*/
	for (num = 0; num < NUM_OF_TX_RING; num++)
	{
		if (pAd->TxBufSpace[num].AllocVa)
		{
			RTMP_FreeFirstTxBuffer(pci_dev, 
									pAd->TxBufSpace[num].AllocSize, 
									FALSE, pAd->TxBufSpace[num].AllocVa, 
									pAd->TxBufSpace[num].AllocPa);
		}
		NdisZeroMemory(&pAd->TxBufSpace[num], sizeof(RTMP_DMABUF));

#ifdef RT_SECURE_DMA
		if (pAd->TxSecureDMA[num].AllocVa)
		{

			NdisZeroMemory(pAd->TxSecureDMA[num].AllocVa, pAd->TxSecureDMA[num].AllocSize);
			RTMP_FreeFirstTxBuffer(pci_dev,
									pAd->TxSecureDMA[num].AllocSize,
									FALSE, pAd->TxSecureDMA[num].AllocVa,
									pAd->TxSecureDMA[num].AllocPa);
		}
#endif
		desc_ring_free(pAd, &pAd->TxDescRing[num]);
	}

	DBGPRINT(RT_DEBUG_TRACE, ("<-- RTMPFreeTxRxRingMemory\n"));
}


NDIS_STATUS RTMPInitTxRxRingMemory(RTMP_ADAPTER *pAd)
{
	INT num, index;
	ULONG RingBasePaHigh, RingBasePaLow;
	VOID *RingBaseVa;
	RTMP_TX_RING *pTxRing;
	RTMP_RX_RING *pRxRing;
	RTMP_DMABUF *pDmaBuf, *pDescRing;
	RTMP_DMACB *dma_cb;
	PNDIS_PACKET pPacket;
	TXD_STRUC *pTxD;
	RXD_STRUC *pRxD;
	ULONG ErrorValue = 0;
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;


	/* Init the CmdQ and CmdQLock*/
	NdisAllocateSpinLock(pAd, &pAd->CmdQLock);	
	NdisAcquireSpinLock(&pAd->CmdQLock);
	RTInitializeCmdQ(&pAd->CmdQ);
	NdisReleaseSpinLock(&pAd->CmdQLock);

	/* Initialize All Tx Ring Descriptors and associated buffer memory*/
	/* (5 TX rings = 4 ACs + 1 HCCA)*/
	for (num = 0; num < NUM_OF_TX_RING; num++)
	{
		ULONG BufBasePaHigh, BufBasePaLow;
		VOID *BufBaseVa;
		
		/* memory zero the  Tx ring descriptor's memory */
		pDescRing = &pAd->TxDescRing[num];
		NdisZeroMemory(pDescRing->AllocVa, pDescRing->AllocSize);
		/* Save PA & VA for further operation*/
		RingBasePaHigh = RTMP_GetPhysicalAddressHigh(pDescRing->AllocPa);
		RingBasePaLow = RTMP_GetPhysicalAddressLow (pDescRing->AllocPa);
		RingBaseVa = pDescRing->AllocVa;

		/* Zero init all 1st TXBuf's memory for this TxRing*/
		NdisZeroMemory(pAd->TxBufSpace[num].AllocVa, pAd->TxBufSpace[num].AllocSize);
		/* Save PA & VA for further operation */
		BufBasePaHigh = RTMP_GetPhysicalAddressHigh(pAd->TxBufSpace[num].AllocPa);
		BufBasePaLow = RTMP_GetPhysicalAddressLow (pAd->TxBufSpace[num].AllocPa);
		BufBaseVa = pAd->TxBufSpace[num].AllocVa;

		if (BufBasePaHigh == 0)
		{/* empty address high */}

		/* linking Tx Ring Descriptor and associated buffer memory */
		pTxRing = &pAd->TxRing[num];
		for (index = 0; index < TX_RING_SIZE; index++)
		{
			dma_cb = &pTxRing->Cell[index];
			dma_cb->pNdisPacket = NULL;
			dma_cb->pNextNdisPacket = NULL;
			/* Init Tx Ring Size, Va, Pa variables*/
			dma_cb->AllocSize = TXD_SIZE;
			dma_cb->AllocVa = RingBaseVa;
			RTMP_SetPhysicalAddressHigh(dma_cb->AllocPa, RingBasePaHigh);
			RTMP_SetPhysicalAddressLow (dma_cb->AllocPa, RingBasePaLow);

			/* Setup Tx Buffer size & address. only 802.11 header will store in this space */
			pDmaBuf = &dma_cb->DmaBuf;
			pDmaBuf->AllocSize = TX_DMA_1ST_BUFFER_SIZE;
			pDmaBuf->AllocVa = BufBaseVa;
			RTMP_SetPhysicalAddressHigh(pDmaBuf->AllocPa, BufBasePaHigh);
			RTMP_SetPhysicalAddressLow(pDmaBuf->AllocPa, BufBasePaLow);

			/* link the pre-allocated TxBuf to TXD */
			pTxD = (PTXD_STRUC)dma_cb->AllocVa;
			pTxD->SDPtr0 = BufBasePaLow;
			/* advance to next ring descriptor address */
			pTxD->DMADONE = 1;
#ifdef RT_BIG_ENDIAN
			RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#endif

			/* flush dcache if no consistent memory is supported */
			RTMP_DCACHE_FLUSH(pTxD, dma_cb->AllocSize);

			RingBasePaLow += TXD_SIZE;
			RingBaseVa = (PUCHAR) RingBaseVa + TXD_SIZE;

			/* advance to next TxBuf address */
			BufBasePaLow += TX_DMA_1ST_BUFFER_SIZE;
			BufBaseVa = (PUCHAR) BufBaseVa + TX_DMA_1ST_BUFFER_SIZE;
		}
		DBGPRINT(RT_DEBUG_TRACE, ("TxRing[%d]: total %d entry initialized\n", num, index));
	}

	/* Initialize MGMT Ring and associated buffer memory */
	pDescRing = &pAd->MgmtDescRing;
	RingBasePaHigh = RTMP_GetPhysicalAddressHigh(pDescRing->AllocPa);
	RingBasePaLow = RTMP_GetPhysicalAddressLow (pDescRing->AllocPa);
	RingBaseVa = pDescRing->AllocVa;
	NdisZeroMemory(pDescRing->AllocVa, pDescRing->AllocSize);

	if (RingBasePaHigh == 0)
	{/* empty address high */}

	for (index = 0; index < MGMT_RING_SIZE; index++)
	{
		dma_cb = &pAd->MgmtRing.Cell[index];
		dma_cb->pNdisPacket = NULL;
		dma_cb->pNextNdisPacket = NULL;
		/* Init MGMT Ring Size, Va, Pa variables */
		dma_cb->AllocSize = TXD_SIZE;
		dma_cb->AllocVa = RingBaseVa;
		RTMP_SetPhysicalAddressHigh(dma_cb->AllocPa, RingBasePaHigh);
		RTMP_SetPhysicalAddressLow (dma_cb->AllocPa, RingBasePaLow);

		/* Offset to next ring descriptor address */
		RingBasePaLow += TXD_SIZE;
		RingBaseVa = (PUCHAR) RingBaseVa + TXD_SIZE;

		/* link the pre-allocated TxBuf to TXD */
		pTxD = (PTXD_STRUC)dma_cb->AllocVa;
		pTxD->DMADONE = 1;
#ifdef RT_BIG_ENDIAN
		RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#endif

		/* flush dcache if no consistent memory is supported */
		RTMP_DCACHE_FLUSH(pTxD, dma_cb->AllocSize);

		/* no pre-allocated buffer required in MgmtRing for scatter-gather case */
	}

#ifdef CONFIG_ANDES_SUPPORT
	/* Initialize CTRL Ring and associated buffer memory */
	pDescRing = &pAd->CtrlDescRing;
	RingBasePaHigh = RTMP_GetPhysicalAddressHigh(pDescRing->AllocPa);
	RingBasePaLow = RTMP_GetPhysicalAddressLow (pDescRing->AllocPa);
	RingBaseVa = pDescRing->AllocVa;
	NdisZeroMemory(pDescRing->AllocVa, pDescRing->AllocSize);
	for (index = 0; index < MGMT_RING_SIZE; index++)
	{
		dma_cb = &pAd->CtrlRing.Cell[index];
		dma_cb->pNdisPacket = NULL;
		dma_cb->pNextNdisPacket = NULL;
		/* Init Ctrl Ring Size, Va, Pa variables */
		dma_cb->AllocSize = TXD_SIZE;
		dma_cb->AllocVa = RingBaseVa;
		RTMP_SetPhysicalAddressHigh(dma_cb->AllocPa, RingBasePaHigh);
		RTMP_SetPhysicalAddressLow (dma_cb->AllocPa, RingBasePaLow);

		/* Offset to next ring descriptor address */
		RingBasePaLow += TXD_SIZE;
		RingBaseVa = (PUCHAR) RingBaseVa + TXD_SIZE;

		/* link the pre-allocated TxBuf to TXD */
		pTxD = (PTXD_STRUC)dma_cb->AllocVa;
		pTxD->DMADONE = 1;

#ifdef RT_BIG_ENDIAN
		RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#endif

		/* flush dcache if no consistent memory is supported */
		RTMP_DCACHE_FLUSH(pTxD, dma_cb->AllocSize);

		/* no pre-allocated buffer required in CtrlRing for scatter-gather case */
	}
#endif /* CONFIG_ANDES_SUPPORT */

	/* Initialize Rx Ring and associated buffer memory */
	for (num = 0; num < NUM_OF_RX_RING; num++)
	{
		UINT16 RxRingSize = (num == 0) ? RX_RING_SIZE : RX1_RING_SIZE;
		
		pDescRing = &pAd->RxDescRing[num];
		pRxRing = &pAd->RxRing[num];

		NdisZeroMemory(pDescRing->AllocVa, pDescRing->AllocSize);

#ifdef RTMP_MAC
		if (pAd->chipCap.hif_type == HIF_RTMP && num > 0)
		{
			/* RTMP has one RX ring, skip skb allocation for second ring (save RAM) */
			NdisZeroMemory(pRxRing->Cell, RX_RING_SIZE * sizeof(RTMP_DMACB));
			continue;
		}
#endif /* RTMP_MAC */

		DBGPRINT(RT_DEBUG_OFF,  ("RX[%d] DESC %p size = %ld\n", 
					num, pDescRing->AllocVa, pDescRing->AllocSize));

		/* Save PA & VA for further operation */
		RingBasePaHigh = RTMP_GetPhysicalAddressHigh(pDescRing->AllocPa);
		RingBasePaLow = RTMP_GetPhysicalAddressLow (pDescRing->AllocPa);
		RingBaseVa = pDescRing->AllocVa;
	
		/* Linking Rx Ring and associated buffer memory */
		for (index = 0; index < RxRingSize; index++)
		{
			dma_cb = &pRxRing->Cell[index];
			/* Init RX Ring Size, Va, Pa variables*/
			dma_cb->AllocSize = RXD_SIZE;
			dma_cb->AllocVa = RingBaseVa;
			RTMP_SetPhysicalAddressHigh(dma_cb->AllocPa, RingBasePaHigh);
			RTMP_SetPhysicalAddressLow (dma_cb->AllocPa, RingBasePaLow);

			/* Offset to next ring descriptor address */
			RingBasePaLow += RXD_SIZE;
			RingBaseVa = (PUCHAR) RingBaseVa + RXD_SIZE;

			/* Setup Rx associated Buffer size & allocate share memory */
			pDmaBuf = &dma_cb->DmaBuf;
			pDmaBuf->AllocSize = RX_BUFFER_AGGRESIZE;
			pPacket = RTMP_AllocateRxPacketBuffer(
				pAd,
				((POS_COOKIE)(pAd->OS_Cookie))->pci_dev,
				pDmaBuf->AllocSize,
				FALSE,
				&pDmaBuf->AllocVa,
				&pDmaBuf->AllocPa);
			
			/* keep allocated rx packet */
			dma_cb->pNdisPacket = pPacket;

			/* Error handling*/
			if (pDmaBuf->AllocVa == NULL)
			{
				ErrorValue = ERRLOG_OUT_OF_SHARED_MEMORY;
				DBGPRINT_ERR(("Failed to allocate RxRing's 1st buffer, ErrorValue = %lu\n", ErrorValue));
				Status = NDIS_STATUS_RESOURCES;
				break;
			}

			/* Zero init this memory block */
			//NdisZeroMemory(pDmaBuf->AllocVa, pDmaBuf->AllocSize);

			/* Write RxD buffer address & allocated buffer length */
			pRxD = (PRXD_STRUC)dma_cb->AllocVa;
#ifndef RT_SECURE_DMA
			pRxD->SDP0 = RTMP_GetPhysicalAddressLow(pDmaBuf->AllocPa);
#else
			pRxD->SDP0 = pAd->RxSecureDMA[num].AllocPa + (index * 4096);
#endif
			pRxD->DDONE = 0;
			pRxD->SDL0 = pDmaBuf->AllocSize;

#ifdef RT_BIG_ENDIAN
			RTMPDescriptorEndianChange((PUCHAR)pRxD, TYPE_RXD);
#endif

			/* flush dcache if no consistent memory is supported */
			RTMP_DCACHE_FLUSH(pRxD, dma_cb->AllocSize);
		}
	}

	NdisZeroMemory(&pAd->FragFrame, sizeof(FRAGMENT_FRAME));
	pAd->FragFrame.pFragPacket =  RTMP_AllocateFragPacketBuffer(pAd, RX_BUFFER_NORMSIZE);
	if (pAd->FragFrame.pFragPacket == NULL)
		Status = NDIS_STATUS_RESOURCES;

	/* Initialize all transmit related software queues */
	for(index = 0; index < NUM_OF_TX_RING; index++)
	{
		InitializeQueueHeader(&pAd->TxSwQueue[index]);
		/* Init TX rings index pointer */
		pAd->TxRing[index].TxSwFreeIdx = 0;
		pAd->TxRing[index].TxCpuIdx = 0;
	}

	/* Init RX Ring index pointer */
	for (index = 0; index < NUM_OF_RX_RING; index++) {
		UINT16 RxRingSize = (index == 0) ? RX_RING_SIZE : RX1_RING_SIZE;
		
		pAd->RxRing[index].RxSwReadIdx = 0;
		pAd->RxRing[index].RxCpuIdx = RxRingSize - 1;
	}
	
	/* init MGMT ring index pointer */
	pAd->MgmtRing.TxSwFreeIdx = 0;
	pAd->MgmtRing.TxCpuIdx = 0;

#ifdef CONFIG_ANDES_SUPPORT
	/* init CTRL ring index pointer */
	pAd->CtrlRing.TxSwFreeIdx = 0;
	pAd->CtrlRing.TxCpuIdx = 0;
#endif /* CONFIG_ANDES_SUPPORT */

	pAd->PrivateInfo.TxRingFullCnt = 0;
	/* Init timer to flush completed packets from TX queues */
	RTMPInitTimer(pAd, &pAd->TxDoneCleanupTimer, GET_TIMER_FUNCTION(TxDoneCleanupExec), pAd, FALSE);

	return Status;

}


/*
	========================================================================
	
	Routine Description:
		Allocate DMA memory blocks for send, receive

	Arguments:
		Adapter		Pointer to our adapter

	Return Value:
		NDIS_STATUS_SUCCESS
		NDIS_STATUS_FAILURE
		NDIS_STATUS_RESOURCES

	IRQL = PASSIVE_LEVEL

	Note:
	
	========================================================================
*/
NDIS_STATUS	RTMPAllocTxRxRingMemory(RTMP_ADAPTER *pAd)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	INT num;
	ULONG ErrorValue = 0;
	VOID *pci_dev = ((POS_COOKIE)(pAd->OS_Cookie))->pci_dev;
	
	DBGPRINT(RT_DEBUG_TRACE, ("-->RTMPAllocTxRxRingMemory\n"));
	do
	{
		/*
			Allocate all ring descriptors, include TxD, RxD, MgmtD.
			Although each size is different, to prevent cacheline and alignment
			issue, I intentional set them all to 64 bytes.
		*/
		for (num = 0; num < NUM_OF_TX_RING; num++)
		{
			/* Allocate Tx ring descriptor's memory (5 TX rings = 4 ACs + 1 HCCA)*/
			desc_ring_alloc(pAd, &pAd->TxDescRing[num],
							TX_RING_SIZE * TXD_SIZE);
			if (pAd->TxDescRing[num].AllocVa == NULL) {
				Status = NDIS_STATUS_RESOURCES;
				break;
			}
			DBGPRINT(RT_DEBUG_TRACE, ("TxRing[%d]: total %d bytes allocated\n",
						num, (INT)pAd->TxDescRing[num].AllocSize));
			
			/* Allocate all 1st TXBuf's memory for this TxRing */
			pAd->TxBufSpace[num].AllocSize = TX_RING_SIZE * TX_DMA_1ST_BUFFER_SIZE;
			RTMP_AllocateFirstTxBuffer(
				pci_dev,
				num,
				pAd->TxBufSpace[num].AllocSize,
				FALSE,
				&pAd->TxBufSpace[num].AllocVa,
				&pAd->TxBufSpace[num].AllocPa);

			if (pAd->TxBufSpace[num].AllocVa == NULL)
			{
				ErrorValue = ERRLOG_OUT_OF_SHARED_MEMORY;
				DBGPRINT_ERR(("Failed to allocate a big buffer, ErrorValue = %lu\n", ErrorValue));
				Status = NDIS_STATUS_RESOURCES;
				break;
			}
#ifdef RT_SECURE_DMA
			/* Get DMA buffers, to be put into desriptors later */
			pAd->TxSecureDMA[num].AllocSize = TX_RING_SIZE * 4096;
			RTMP_AllocateFirstTxBuffer(
				pci_dev,
				num,
				pAd->TxSecureDMA[num].AllocSize,
				FALSE,
				&pAd->TxSecureDMA[num].AllocVa,
				&pAd->TxSecureDMA[num].AllocPa);
			if (pAd->TxSecureDMA[num].AllocVa == NULL)
			{
				ErrorValue = ERRLOG_OUT_OF_SHARED_MEMORY;
				DBGPRINT_ERR(("Failed to allocate a big buffer, ErrorValue = %lu\n", ErrorValue));
				Status = NDIS_STATUS_RESOURCES;
				break;
			}
#endif
		}
		if (Status == NDIS_STATUS_RESOURCES)
			break;

		
		/* Alloc MGMT ring desc buffer except Tx ring allocated eariler */
		desc_ring_alloc(pAd, &pAd->MgmtDescRing,
							MGMT_RING_SIZE * TXD_SIZE);
		if (pAd->MgmtDescRing.AllocVa == NULL) {
			Status = NDIS_STATUS_RESOURCES;
			break;
		}
		DBGPRINT(RT_DEBUG_TRACE, ("MGMT Ring: total %d bytes allocated\n",
					(INT)pAd->MgmtDescRing.AllocSize));

#ifdef RT_SECURE_DMA
			/* Get DMA buffers, to be put into desriptors later */
			pAd->MgmtSecureDMA.AllocSize = MGMT_RING_SIZE * 4096;
			RTMP_AllocateFirstTxBuffer(
				pci_dev,
				num,
				pAd->MgmtSecureDMA.AllocSize,
				FALSE,
				&pAd->MgmtSecureDMA.AllocVa,
				&pAd->MgmtSecureDMA.AllocPa);
			if (pAd->MgmtSecureDMA.AllocVa == NULL)
			{
				ErrorValue = ERRLOG_OUT_OF_SHARED_MEMORY;
				DBGPRINT_ERR(("Failed to allocate a big buffer, ErrorValue = %lu\n", ErrorValue));
				Status = NDIS_STATUS_RESOURCES;
				break;
			}
#endif
#ifdef CONFIG_ANDES_SUPPORT
		/* Alloc CTRL ring desc buffer except Tx ring allocated eariler */
		desc_ring_alloc(pAd, &pAd->CtrlDescRing,
							MGMT_RING_SIZE * TXD_SIZE);
		if (pAd->CtrlDescRing.AllocVa == NULL) {
			Status = NDIS_STATUS_RESOURCES;
			break;
		}
		DBGPRINT(RT_DEBUG_TRACE, ("CTRL Ring: total %d bytes allocated\n",
					(INT)pAd->CtrlDescRing.AllocSize));
#ifdef RT_SECURE_DMA
		/* Get DMA buffers, to be put into desriptors later */
		pAd->CtrlSecureDMA.AllocSize = MGMT_RING_SIZE * 4096;
		RTMP_AllocateFirstTxBuffer(
			pci_dev,
			num,
			pAd->CtrlSecureDMA.AllocSize,
			FALSE,
			&pAd->CtrlSecureDMA.AllocVa,
			&pAd->CtrlSecureDMA.AllocPa);
		if (pAd->CtrlSecureDMA.AllocVa == NULL)
		{
			ErrorValue = ERRLOG_OUT_OF_SHARED_MEMORY;
			DBGPRINT_ERR(("Failed to allocate a big buffer, ErrorValue = %lu\n", ErrorValue));
			Status = NDIS_STATUS_RESOURCES;
			break;
		}
#endif
#endif /* CONFIG_ANDES_SUPPORT */

		/* Alloc RX ring desc memory except Tx ring allocated eariler */
		for (num = 0; num < NUM_OF_RX_RING; num++) {
			UINT16 RxRingSize = (num == 0) ? RX_RING_SIZE : RX1_RING_SIZE;
			
			desc_ring_alloc(pAd, &pAd->RxDescRing[num], RxRingSize * RXD_SIZE);
			if (pAd->RxDescRing[num].AllocVa == NULL) {
				Status = NDIS_STATUS_RESOURCES;
				break;
			}
			DBGPRINT(RT_DEBUG_TRACE, ("Rx[%d] Ring: total %d bytes allocated\n",
						num, (INT)pAd->RxDescRing[num].AllocSize));
#ifdef RT_SECURE_DMA
			/* Get DMA buffers, to be put into desriptors later */
			pAd->RxSecureDMA[num].AllocSize = RX_RING_SIZE * 4096;
			RTMP_AllocateFirstTxBuffer(
				pci_dev,
				num,
				pAd->RxSecureDMA[num].AllocSize,
				FALSE,
				&pAd->RxSecureDMA[num].AllocVa,
				&pAd->RxSecureDMA[num].AllocPa);
			if (pAd->RxSecureDMA[num].AllocVa == NULL)
			{
				ErrorValue = ERRLOG_OUT_OF_SHARED_MEMORY;
				DBGPRINT_ERR(("Failed to allocate a big buffer, ErrorValue = %lu\n", ErrorValue));
				Status = NDIS_STATUS_RESOURCES;
				break;
			}
#endif
		}
	}	while (FALSE);


	if (Status != NDIS_STATUS_SUCCESS)
	{
		/* Log error inforamtion*/
		NdisWriteErrorLogEntry(
			pAd->AdapterHandle,
			NDIS_ERROR_CODE_OUT_OF_RESOURCES,
			1,
			ErrorValue);
	}

	DBGPRINT_S(Status, ("<-- RTMPAllocTxRxRingMemory, Status=%x\n", Status));
	return Status;
}

#else
/*
	========================================================================
	
	Routine Description:
		Allocate DMA memory blocks for send, receive

	Arguments:
		Adapter		Pointer to our adapter

	Return Value:
		NDIS_STATUS_SUCCESS
		NDIS_STATUS_FAILURE
		NDIS_STATUS_RESOURCES

	IRQL = PASSIVE_LEVEL

	Note:
	
	========================================================================
*/
NDIS_STATUS	RTMPAllocTxRxRingMemory(RTMP_ADAPTER *pAd)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	ULONG RingBasePaHigh, RingBasePaLow;
	PVOID RingBaseVa;
	INT index, num;
	TXD_STRUC *pTxD;
	RXD_STRUC *pRxD;
	ULONG ErrorValue = 0;
	RTMP_TX_RING *pTxRing;
	RTMP_DMABUF *pDmaBuf;
	RTMP_DMACB *dma_cb;
	PNDIS_PACKET pPacket;
	

	DBGPRINT(RT_DEBUG_TRACE, ("--> RTMPAllocTxRxRingMemory\n"));

	/* Init the CmdQ and CmdQLock*/
	NdisAllocateSpinLock(pAd, &pAd->CmdQLock);	
	NdisAcquireSpinLock(&pAd->CmdQLock);
	RTInitializeCmdQ(&pAd->CmdQ);
	NdisReleaseSpinLock(&pAd->CmdQLock);

	do
	{
		/* 
			Allocate all ring descriptors, include TxD, RxD, MgmtD.
			Although each size is different, to prevent cacheline and alignment
			issue, I intentional set them all to 64 bytes
		*/
		for (num=0; num<NUM_OF_TX_RING; num++)
		{
			ULONG  BufBasePaHigh;
			ULONG  BufBasePaLow;
			PVOID  BufBaseVa;

			/* 
				Allocate Tx ring descriptor's memory (5 TX rings = 4 ACs + 1 HCCA)
			*/
			desc_ring_alloc(pAd, &pAd->TxDescRing[num], TX_RING_SIZE * TXD_SIZE);
			if (pAd->TxDescRing[num].AllocVa == NULL) {
				Status = NDIS_STATUS_RESOURCES;
				break;
			}

			pDmaBuf = &pAd->TxDescRing[num];
			DBGPRINT(RT_DEBUG_TRACE, ("TxDescRing[%p]: total %d bytes allocated\n",
					pDmaBuf->AllocVa, (INT)pDmaBuf->AllocSize));

			/* Save PA & VA for further operation*/
			RingBasePaHigh = RTMP_GetPhysicalAddressHigh(pDmaBuf->AllocPa);
			RingBasePaLow = RTMP_GetPhysicalAddressLow(pDmaBuf->AllocPa);
			RingBaseVa = pDmaBuf->AllocVa;

			/*
				Allocate all 1st TXBuf's memory for this TxRing
			*/
			pAd->TxBufSpace[num].AllocSize = TX_RING_SIZE * TX_DMA_1ST_BUFFER_SIZE;
			RTMP_AllocateFirstTxBuffer(
				((POS_COOKIE)(pAd->OS_Cookie))->pci_dev,
				num,
				pAd->TxBufSpace[num].AllocSize,
				FALSE,
				&pAd->TxBufSpace[num].AllocVa,
				&pAd->TxBufSpace[num].AllocPa);

			if (pAd->TxBufSpace[num].AllocVa == NULL)
			{
				ErrorValue = ERRLOG_OUT_OF_SHARED_MEMORY;
				DBGPRINT_ERR(("Failed to allocate a big buffer, ErrorValue = %lu\n", ErrorValue));
				Status = NDIS_STATUS_RESOURCES;
				break;
			}

			/* Zero init this memory block*/
			NdisZeroMemory(pAd->TxBufSpace[num].AllocVa, pAd->TxBufSpace[num].AllocSize);

			/* Save PA & VA for further operation*/
			BufBasePaHigh = RTMP_GetPhysicalAddressHigh(pAd->TxBufSpace[num].AllocPa);
			BufBasePaLow = RTMP_GetPhysicalAddressLow (pAd->TxBufSpace[num].AllocPa);
			BufBaseVa = pAd->TxBufSpace[num].AllocVa;

			/*
				Initialize Tx Ring Descriptor and associated buffer memory
			*/
			pTxRing = &pAd->TxRing[num];
			for (index = 0; index < TX_RING_SIZE; index++)
			{
				dma_cb = &pTxRing->Cell[index];
				dma_cb->pNdisPacket = NULL;
				dma_cb->pNextNdisPacket = NULL;
				/* Init Tx Ring Size, Va, Pa variables */
				dma_cb->AllocSize = TXD_SIZE;
				dma_cb->AllocVa = RingBaseVa;
				RTMP_SetPhysicalAddressHigh(dma_cb->AllocPa, RingBasePaHigh);
				RTMP_SetPhysicalAddressLow (dma_cb->AllocPa, RingBasePaLow);

				/* Setup Tx Buffer size & address. only 802.11 header will store in this space*/
				pDmaBuf = &dma_cb->DmaBuf;
				pDmaBuf->AllocSize = TX_DMA_1ST_BUFFER_SIZE;
				pDmaBuf->AllocVa = BufBaseVa;
				RTMP_SetPhysicalAddressHigh(pDmaBuf->AllocPa, BufBasePaHigh);
				RTMP_SetPhysicalAddressLow(pDmaBuf->AllocPa, BufBasePaLow);

				/* link the pre-allocated TxBuf to TXD */
				pTxD = (PTXD_STRUC)dma_cb->AllocVa;
				pTxD->SDPtr0 = BufBasePaLow;
				/* advance to next ring descriptor address */
				pTxD->DMADONE = 1;
#ifdef RT_BIG_ENDIAN
				RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#endif

				/* flush dcache if no consistent memory is supported */
				RTMP_DCACHE_FLUSH(pTxD, dma_cb->AllocSize);

				RingBasePaLow += TXD_SIZE;
				RingBaseVa = (PUCHAR) RingBaseVa + TXD_SIZE;

				/* advance to next TxBuf address */
				BufBasePaLow += TX_DMA_1ST_BUFFER_SIZE;
				BufBaseVa = (PUCHAR) BufBaseVa + TX_DMA_1ST_BUFFER_SIZE;
			}
			DBGPRINT(RT_DEBUG_TRACE, ("TxRing[%d]: total %d entry allocated\n", num, index));
		}
		if (Status == NDIS_STATUS_RESOURCES)
			break;

		/*
			Allocate MGMT ring descriptor's memory except Tx ring which allocated eariler
		*/
		desc_ring_alloc(pAd, &pAd->MgmtDescRing, MGMT_RING_SIZE * TXD_SIZE);
		if (pAd->MgmtDescRing.AllocVa == NULL) {
			Status = NDIS_STATUS_RESOURCES;
			break;
		}
		DBGPRINT(RT_DEBUG_TRACE, ("MgmtDescRing[%p]: total %d bytes allocated\n",
				pAd->MgmtDescRing.AllocVa, (INT)pAd->MgmtDescRing.AllocSize));

		/* Save PA & VA for further operation*/
		RingBasePaHigh = RTMP_GetPhysicalAddressHigh(pAd->MgmtDescRing.AllocPa);
		RingBasePaLow = RTMP_GetPhysicalAddressLow(pAd->MgmtDescRing.AllocPa);
		RingBaseVa = pAd->MgmtDescRing.AllocVa;

		/*
			Initialize MGMT Ring and associated buffer memory
		*/
		for (index = 0; index < MGMT_RING_SIZE; index++)
		{
			dma_cb = &pAd->MgmtRing.Cell[index];
			dma_cb->pNdisPacket = NULL;
			dma_cb->pNextNdisPacket = NULL;
			/* Init MGMT Ring Size, Va, Pa variables*/
			dma_cb->AllocSize = TXD_SIZE;
			dma_cb->AllocVa = RingBaseVa;
			RTMP_SetPhysicalAddressHigh(dma_cb->AllocPa, RingBasePaHigh);
			RTMP_SetPhysicalAddressLow (dma_cb->AllocPa, RingBasePaLow);

			/* Offset to next ring descriptor address*/
			RingBasePaLow += TXD_SIZE;
			RingBaseVa = (PUCHAR) RingBaseVa + TXD_SIZE;

			/* link the pre-allocated TxBuf to TXD*/
			pTxD = (PTXD_STRUC)dma_cb->AllocVa;
			pTxD->DMADONE = 1;

#ifdef RT_BIG_ENDIAN
			RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#endif

			/* flush dcache if no consistent memory is supported */
			RTMP_DCACHE_FLUSH(pTxD, dma_cb->AllocSize);

			/* no pre-allocated buffer required in MgmtRing for scatter-gather case*/
		}
		DBGPRINT(RT_DEBUG_TRACE, ("MGMT Ring: total %d entry allocated\n", index));

#ifdef CONFIG_ANDES_SUPPORT
		/*
			Allocate CTRL ring descriptor's memory except Tx ring which allocated eariler
		*/
		desc_ring_alloc(pAd, &pAd->CtrlDescRing, MGMT_RING_SIZE * TXD_SIZE);
		if (pAd->CtrlDescRing.AllocVa == NULL) {
			Status = NDIS_STATUS_RESOURCES;
			break;
		}
		DBGPRINT(RT_DEBUG_TRACE, ("CtrlDescRing[%p]: total %d bytes allocated\n",
				pAd->CtrlDescRing.AllocVa, (INT)pAd->CtrlDescRing.AllocSize));

		/* Save PA & VA for further operation*/
		RingBasePaHigh = RTMP_GetPhysicalAddressHigh(pAd->CtrlDescRing.AllocPa);
		RingBasePaLow = RTMP_GetPhysicalAddressLow (pAd->CtrlDescRing.AllocPa);
		RingBaseVa = pAd->CtrlDescRing.AllocVa;
		
		/*
			Initialize CTRL Ring and associated buffer memory
		*/
		for (index = 0; index < MGMT_RING_SIZE; index++)
		{
			dma_cb = &pAd->CtrlRing.Cell[index];
			dma_cb->pNdisPacket = NULL;
			dma_cb->pNextNdisPacket = NULL;
			/* Init CTRL Ring Size, Va, Pa variables*/
			dma_cb->AllocSize = TXD_SIZE;
			dma_cb->AllocVa = RingBaseVa;
			RTMP_SetPhysicalAddressHigh(dma_cb->AllocPa, RingBasePaHigh);
			RTMP_SetPhysicalAddressLow(dma_cb->AllocPa, RingBasePaLow);

			/* Offset to next ring descriptor address*/
			RingBasePaLow += TXD_SIZE;
			RingBaseVa = (PUCHAR) RingBaseVa + TXD_SIZE;

			/* link the pre-allocated TxBuf to TXD*/
			pTxD = (PTXD_STRUC)dma_cb->AllocVa;
			pTxD->DMADONE = 1;

#ifdef RT_BIG_ENDIAN
			RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#endif

			/* flush dcache if no consistent memory is supported */
			RTMP_DCACHE_FLUSH(pTxD, dma_cb->AllocSize);

			/* no pre-allocated buffer required in CtrlRing for scatter-gather case*/
		}
		DBGPRINT(RT_DEBUG_TRACE, ("CTRL Ring: total %d entry allocated\n", index));
#endif /* CONFIG_ANDES_SUPPORT */


		for (num = 0; num < NUM_OF_RX_RING; num++)
		{
			UINT16 RxRingSize = (num == 0) ? RX_RING_SIZE : RX1_RING_SIZE;
			
			/* Alloc RxRingDesc memory except Tx ring allocated eariler */
			desc_ring_alloc(pAd, &pAd->RxDescRing[num], RxRingSize * RXD_SIZE);
			if (pAd->RxDescRing[num].AllocVa == NULL) {
				Status = NDIS_STATUS_RESOURCES;
				break;
			}
#ifdef RTMP_MAC
			if (pAd->chipCap.hif_type == HIF_RTMP && num > 0)
			{
				/* RTMP has one RX ring, skip skb allocation for second ring (save RAM) */
				NdisZeroMemory(pAd->RxRing[num].Cell, RX_RING_SIZE * sizeof(RTMP_DMACB));
				continue;
			}
#endif /* RTMP_MAC */
			DBGPRINT(RT_DEBUG_OFF, ("RxDescRing[%p]: total %d bytes allocated\n",
						pAd->RxDescRing[num].AllocVa, (INT)pAd->RxDescRing[num].AllocSize));
		
			/* Initialize Rx Ring and associated buffer memory */
			RingBasePaHigh = RTMP_GetPhysicalAddressHigh(pAd->RxDescRing[num].AllocPa);
			RingBasePaLow = RTMP_GetPhysicalAddressLow (pAd->RxDescRing[num].AllocPa);
			RingBaseVa = pAd->RxDescRing[num].AllocVa;
			for (index = 0; index < RxRingSize; index++)
			{
				dma_cb = &pAd->RxRing[num].Cell[index];
				/* Init RX Ring Size, Va, Pa variables*/
				dma_cb->AllocSize = RXD_SIZE;
				dma_cb->AllocVa = RingBaseVa;
				RTMP_SetPhysicalAddressHigh(dma_cb->AllocPa, RingBasePaHigh);
				RTMP_SetPhysicalAddressLow (dma_cb->AllocPa, RingBasePaLow);

				/* Offset to next ring descriptor address */
				RingBasePaLow += RXD_SIZE;
				RingBaseVa = (PUCHAR) RingBaseVa + RXD_SIZE;

				/* Setup Rx associated Buffer size & allocate share memory*/
				pDmaBuf = &dma_cb->DmaBuf;
				pDmaBuf->AllocSize = RX_BUFFER_AGGRESIZE;
				pPacket = RTMP_AllocateRxPacketBuffer(
					pAd,
					((POS_COOKIE)(pAd->OS_Cookie))->pci_dev,
					pDmaBuf->AllocSize,
					FALSE,
					&pDmaBuf->AllocVa,
					&pDmaBuf->AllocPa);
				
				/* keep allocated rx packet */
				dma_cb->pNdisPacket = pPacket;
				if (pDmaBuf->AllocVa == NULL)
				{
					ErrorValue = ERRLOG_OUT_OF_SHARED_MEMORY;
					DBGPRINT_ERR(("Failed to allocate RxRing's 1st buffer\n"));
					Status = NDIS_STATUS_RESOURCES;
					break;
				}

				/* Zero init this memory block*/
				NdisZeroMemory(pDmaBuf->AllocVa, pDmaBuf->AllocSize);

				/* Write RxD buffer address & allocated buffer length*/
				pRxD = (PRXD_STRUC)dma_cb->AllocVa;
				pRxD->SDP0 = RTMP_GetPhysicalAddressLow(pDmaBuf->AllocPa);
				pRxD->SDL0 = pDmaBuf->AllocSize;
				pRxD->DDONE = 0;
#ifdef RT_BIG_ENDIAN
				RTMPDescriptorEndianChange((PUCHAR)pRxD, TYPE_RXD);
#endif
				/* flush dcache if no consistent memory is supported */
				RTMP_DCACHE_FLUSH(pRxD, dma_cb->AllocSize);
			}

			DBGPRINT(RT_DEBUG_TRACE, ("Rx[%d] Ring: total %d entry allocated\n", num, index));
		}
	}	while (FALSE);

	NdisZeroMemory(&pAd->FragFrame, sizeof(FRAGMENT_FRAME));
	pAd->FragFrame.pFragPacket = RTMP_AllocateFragPacketBuffer(pAd, RX_BUFFER_NORMSIZE);
	if (pAd->FragFrame.pFragPacket == NULL)
		Status = NDIS_STATUS_RESOURCES;

	if (Status != NDIS_STATUS_SUCCESS)
	{
		/* Log error inforamtion*/
		NdisWriteErrorLogEntry(
			pAd->AdapterHandle,
			NDIS_ERROR_CODE_OUT_OF_RESOURCES,
			1,
			ErrorValue);
	}

	/*
		Initialize all transmit related software queues
	*/

	/* Init TX rings index pointer*/
	for(index = 0; index < NUM_OF_TX_RING; index++)
	{
		InitializeQueueHeader(&pAd->TxSwQueue[index]);
		pAd->TxRing[index].TxSwFreeIdx = 0;
		pAd->TxRing[index].TxCpuIdx = 0;
	}

	/* init MGMT ring index pointer*/
	pAd->MgmtRing.TxSwFreeIdx = 0;
	pAd->MgmtRing.TxCpuIdx = 0;

#ifdef CONFIG_ANDES_SUPPORT
	/* init CTRL ring index pointer*/
	pAd->CtrlRing.TxSwFreeIdx = 0;
	pAd->CtrlRing.TxCpuIdx = 0;
#endif /* CONFIG_ANDES_SUPPORT */

	/* Init RX Ring index pointer*/
	for(index = 0; index < NUM_OF_RX_RING; index++) {
		UINT16 RxRingSize = (index == 0) ? RX_RING_SIZE : RX1_RING_SIZE;
		
		pAd->RxRing[index].RxSwReadIdx = 0;
		pAd->RxRing[index].RxCpuIdx = RxRingSize - 1;
	}

	pAd->PrivateInfo.TxRingFullCnt = 0;

	DBGPRINT_S(Status, ("<-- RTMPAllocTxRxRingMemory, Status=%x\n", Status));
	return Status;
}


VOID RTMPFreeTxRxRingMemory(RTMP_ADAPTER *pAd)
{
	int index, num , j;
	RTMP_TX_RING *pTxRing;
	TXD_STRUC *pTxD;
#ifdef RT_BIG_ENDIAN
	TXD_STRUC *pDestTxD;
	UCHAR tx_hw_info[TXD_SIZE];
#endif /* RT_BIG_ENDIAN */
	PNDIS_PACKET pPacket;
	ULONG IrqFlags;
	RTMP_DMACB *dma_cb;

	DBGPRINT(RT_DEBUG_TRACE, ("--> RTMPFreeTxRxRingMemory\n"));

	RTMP_IRQ_LOCK(&pAd->irq_lock, IrqFlags);

	/* Free TxSwQueue Packet*/
	for (index=0; index <NUM_OF_TX_RING; index++)
	{
		PQUEUE_ENTRY pEntry;
		PQUEUE_HEADER pQueue;

		pQueue = &pAd->TxSwQueue[index];
		while (pQueue->Head)
		{
			pEntry = RemoveHeadQueue(pQueue);
			pPacket = QUEUE_ENTRY_TO_PACKET(pEntry);
			if (pPacket)
			RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
		}
	}

	/* Free Tx Ring Packet*/
	for (index=0;index< NUM_OF_TX_RING;index++)
	{
		pTxRing = &pAd->TxRing[index];
		
		for (j=0; j< TX_RING_SIZE; j++)
		{	
#ifdef RT_BIG_ENDIAN
			pDestTxD  = (PTXD_STRUC) (pTxRing->Cell[j].AllocVa);
			NdisMoveMemory(&tx_hw_info[0], (UCHAR *)pDestTxD, TXD_SIZE);
			pTxD = (TXD_STRUC *)&tx_hw_info[0];
			RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#else
			pTxD = (PTXD_STRUC) (pTxRing->Cell[j].AllocVa);
#endif /* RT_BIG_ENDIAN */

			pPacket = pTxRing->Cell[j].pNdisPacket;
			if (pPacket)
			{
				pTxRing->Cell[j].pNdisPacket = NULL;
				PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr0, pTxD->SDLen0, RTMP_PCI_DMA_TODEVICE);
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
			}		

			pPacket = pTxRing->Cell[j].pNextNdisPacket;
			if (pPacket)
			{
				pTxRing->Cell[j].pNextNdisPacket = NULL;
				PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr1, pTxD->SDLen1, RTMP_PCI_DMA_TODEVICE);
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
			}

#ifdef RT_BIG_ENDIAN
			RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
			WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, FALSE, TYPE_TXD);
#endif /* RT_BIG_ENDIAN */
		}
	}	

	RTMP_IRQ_UNLOCK(&pAd->irq_lock, IrqFlags);

	{
	RTMP_MGMT_RING *pMgmtRing = &pAd->MgmtRing;
	NdisAcquireSpinLock(&pAd->MgmtRingLock);

	RTMP_IO_READ32(pAd, pMgmtRing->hw_didx_addr, &pMgmtRing->TxDmaIdx);
	while (pMgmtRing->TxSwFreeIdx!= pMgmtRing->TxDmaIdx)
	{
		RTMP_DMACB *dma_cb = &pMgmtRing->Cell[pAd->MgmtRing.TxSwFreeIdx];

#ifdef RT_BIG_ENDIAN
		pDestTxD = (PTXD_STRUC) (dma_cb->AllocVa);
		NdisMoveMemory(&tx_hw_info[0], (UCHAR *)pDestTxD, TXD_SIZE);
		pTxD = (TXD_STRUC *)&tx_hw_info[0];
		RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#else
		pTxD = (PTXD_STRUC) (dma_cb->AllocVa);
#endif
		pTxD->DMADONE = 0;
		pPacket = dma_cb->pNdisPacket;
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

		INC_RING_INDEX(pMgmtRing->TxSwFreeIdx, MGMT_RING_SIZE);

#ifdef RT_BIG_ENDIAN
        RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
        WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, TRUE, TYPE_TXD);
#endif
	}
	NdisReleaseSpinLock(&pAd->MgmtRingLock);
	}

#ifdef CONFIG_ANDES_SUPPORT
	{
		PRTMP_MGMT_RING pCtrlRing = &pAd->CtrlRing;
		
		RTMP_IO_READ32(pAd, TX_CTRL_DIDX, &pCtrlRing->TxDmaIdx);

		while (pCtrlRing->TxSwFreeIdx!= pCtrlRing->TxDmaIdx)
		{
			RTMP_DMACB *dma_cb = &pCtrlRing->Cell[pCtrlRing->TxSwFreeIdx];
#ifdef RT_BIG_ENDIAN
			pDestTxD = (PTXD_STRUC) (dma_cb->AllocVa);
			NdisMoveMemory(&tx_hw_info[0], (UCHAR *)pDestTxD, TXD_SIZE);
			pTxD = (TXD_STRUC *)&tx_hw_info[0];
			RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#else
			pTxD = (PTXD_STRUC) (dma_cb->AllocVa);
#endif /* RT_BIG_ENDIAN */
			pTxD->DMADONE = 0;

			pPacket = dma_cb->pNdisPacket;
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

			INC_RING_INDEX(pCtrlRing->TxSwFreeIdx, MGMT_RING_SIZE);

#ifdef RT_BIG_ENDIAN
	        RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
	        WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, TRUE, TYPE_TXD);
#endif /* RT_BIG_ENDIAN */
		}
		NdisReleaseSpinLock(&pAd->CtrlRingLock);
	}
#endif /* CONFIG_ANDES_SUPPORT */

	for (j = 0; j < NUM_OF_RX_RING; j++)
	{
		UINT16 RxRingSize = (j == 0) ? RX_RING_SIZE : RX1_RING_SIZE;
		
		for (index = RxRingSize - 1 ; index >= 0; index--)
		{
			dma_cb = &pAd->RxRing[j].Cell[index];
			if ((dma_cb->DmaBuf.AllocVa) && (dma_cb->pNdisPacket))
			{
				PCI_UNMAP_SINGLE(pAd, dma_cb->DmaBuf.AllocPa,
									dma_cb->DmaBuf.AllocSize,
									RTMP_PCI_DMA_FROMDEVICE);
				RELEASE_NDIS_PACKET(pAd, dma_cb->pNdisPacket, NDIS_STATUS_SUCCESS);
			}
		}
		NdisZeroMemory(pAd->RxRing[j].Cell, RX_RING_SIZE * sizeof(RTMP_DMACB));
		
		if (pAd->RxDescRing[j].AllocVa)
			RtmpFreeDescBuf(((POS_COOKIE)(pAd->OS_Cookie))->pci_dev,
								pAd->RxDescRing[j].AllocSize,
								pAd->RxDescRing[j].AllocVa,
								pAd->RxDescRing[j].AllocPa);
		
		NdisZeroMemory(&pAd->RxDescRing[j], sizeof(RTMP_DMABUF));
	}

	if (pAd->MgmtDescRing.AllocVa)
	{
		RtmpFreeDescBuf(((POS_COOKIE)(pAd->OS_Cookie))->pci_dev, pAd->MgmtDescRing.AllocSize, pAd->MgmtDescRing.AllocVa, pAd->MgmtDescRing.AllocPa);
	}
	NdisZeroMemory(&pAd->MgmtDescRing, sizeof(RTMP_DMABUF));

#ifdef CONFIG_ANDES_SUPPORT
	if (pAd->CtrlDescRing.AllocVa)
	{
		RtmpFreeDescBuf(((POS_COOKIE)(pAd->OS_Cookie))->pci_dev, pAd->CtrlDescRing.AllocSize, pAd->CtrlDescRing.AllocVa, pAd->CtrlDescRing.AllocPa);
	}
	NdisZeroMemory(&pAd->CtrlDescRing, sizeof(RTMP_DMABUF));
#endif /* CONFIG_ANDES_SUPPORT */

	for (num = 0; num < NUM_OF_TX_RING; num++)
	{
    	if (pAd->TxBufSpace[num].AllocVa)
		{
			RTMP_FreeFirstTxBuffer(((POS_COOKIE)(pAd->OS_Cookie))->pci_dev, pAd->TxBufSpace[num].AllocSize, FALSE, pAd->TxBufSpace[num].AllocVa, pAd->TxBufSpace[num].AllocPa);
	    }
	    NdisZeroMemory(&pAd->TxBufSpace[num], sizeof(RTMP_DMABUF));
	    
    	if (pAd->TxDescRing[num].AllocVa)
		{
			RtmpFreeDescBuf(((POS_COOKIE)(pAd->OS_Cookie))->pci_dev, pAd->TxDescRing[num].AllocSize, pAd->TxDescRing[num].AllocVa, pAd->TxDescRing[num].AllocPa);
	    }
	    NdisZeroMemory(&pAd->TxDescRing[num], sizeof(RTMP_DMABUF));
	}

	if (pAd->FragFrame.pFragPacket)
	{
		RELEASE_NDIS_PACKET(pAd, pAd->FragFrame.pFragPacket, NDIS_STATUS_SUCCESS);
		pAd->FragFrame.pFragPacket = NULL;
	}

	NdisFreeSpinLock(&pAd->CmdQLock);

	DBGPRINT(RT_DEBUG_TRACE, ("<-- RTMPFreeTxRxRingMemory\n"));
}

#endif /* RESOURCE_PRE_ALLOC */


VOID AsicInitTxRxRing(RTMP_ADAPTER *pAd)
{
#ifdef RLT_MAC
	if (pAd->chipCap.hif_type == HIF_RLT)
		rlt_asic_init_txrx_ring(pAd);
#endif /* RLT_MAC */

#ifdef RTMP_MAC
	if (pAd->chipCap.hif_type == HIF_RTMP)
		rtmp_asic_init_txrx_ring(pAd);
#endif /* RTMP_MAC */
}


#ifdef DMA_BUSY_RESET
//reference MT7603E's watchdog body
VOID PDMAWatchDog(RTMP_ADAPTER *pAd)
{
	BOOLEAN NoDataOut = FALSE, NoDataIn = FALSE;

	/* Tx DMA unchaged detect */
	NoDataOut = MonitorTxRing(pAd);
		
	if (NoDataOut)
	{
		DBGPRINT(RT_DEBUG_OFF, ("TXDMA Reset\n"));
		pAd->TxDMAResetCount++;
		goto reset;
	}


	return;

reset:

	WlanResetB(pAd);
}
#endif /* DMA_BUSY_RESET */

/*
	========================================================================
	
	Routine Description:
		Reset NIC Asics. Call after rest DMA. So reset TX_CTX_IDX to zero.

	Arguments:
		Adapter						Pointer to our adapter

	Return Value:
		None

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL
	
	Note:
		Reset NIC to initial state AS IS system boot up time.
		
	========================================================================
*/
VOID RTMPRingCleanUp(RTMP_ADAPTER *pAd, UCHAR RingType)
{
	TXD_STRUC *pTxD;
	RXD_STRUC *pRxD;
#ifdef RT_BIG_ENDIAN
	TXD_STRUC *pDestTxD, TxD;
	RXD_STRUC *pDestRxD, RxD;
#endif /* RT_BIG_ENDIAN */
	QUEUE_ENTRY *pEntry;
	PNDIS_PACKET pPacket;
	RTMP_TX_RING *pTxRing;
	ULONG IrqFlags;
	int i, ring_id;

	/*
		We have to clean all descriptors in case some error happened with reset
	*/
	DBGPRINT(RT_DEBUG_TRACE,("RTMPRingCleanUp(RingIdx=%d, Pending-NDIS=%ld)\n", RingType, pAd->RalinkCounters.PendingNdisPacketCount));
	switch (RingType)
	{
		case QID_AC_BK:
		case QID_AC_BE:
		case QID_AC_VI:
		case QID_AC_VO:
		case QID_HCCA:
			
			pTxRing = &pAd->TxRing[RingType];
			
			RTMP_IRQ_LOCK(&pAd->irq_lock, IrqFlags);
			for (i=0; i<TX_RING_SIZE; i++) /* We have to scan all TX ring*/
			{
				pTxD  = (PTXD_STRUC) pTxRing->Cell[i].AllocVa;

				pPacket = (PNDIS_PACKET) pTxRing->Cell[i].pNdisPacket;
				/* release scatter-and-gather NDIS_PACKET*/
				if (pPacket)
				{
					pTxRing->Cell[i].pNdisPacket = NULL;
					RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
				}

				pPacket = (PNDIS_PACKET) pTxRing->Cell[i].pNextNdisPacket;
				/* release scatter-and-gather NDIS_PACKET*/
				if (pPacket)
				{
					pTxRing->Cell[i].pNextNdisPacket = NULL;
					RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
				}
			}

			RTMP_IO_READ32(pAd, pTxRing->hw_didx_addr, &pTxRing->TxDmaIdx);
			pTxRing->TxSwFreeIdx = pTxRing->TxDmaIdx;
			pTxRing->TxCpuIdx = pTxRing->TxDmaIdx;
			RTMP_IO_WRITE32(pAd, pTxRing->hw_cidx_addr, pTxRing->TxCpuIdx);

			while (pAd->TxSwQueue[RingType].Head != NULL)
			{
				pEntry = RemoveHeadQueue(&pAd->TxSwQueue[RingType]);
				pPacket = QUEUE_ENTRY_TO_PACKET(pEntry);
				if (pPacket)
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
				DBGPRINT(RT_DEBUG_TRACE,("Release 1 NDIS packet from s/w backlog queue\n"));
			}
			RTMP_IRQ_UNLOCK(&pAd->irq_lock, IrqFlags);
			break;

		case QID_MGMT:
			RTMP_IRQ_LOCK(&pAd->MgmtRingLock, IrqFlags);
			for (i=0; i<MGMT_RING_SIZE; i++)
			{
#ifdef RT_BIG_ENDIAN
				pDestTxD  = (PTXD_STRUC) pAd->MgmtRing.Cell[i].AllocVa;
				TxD = *pDestTxD;
				pTxD = &TxD;
				RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#else
				pTxD  = (PTXD_STRUC) pAd->MgmtRing.Cell[i].AllocVa;
#endif /* RT_BIG_ENDIAN */

				pPacket = (PNDIS_PACKET) pAd->MgmtRing.Cell[i].pNdisPacket;
				/* rlease scatter-and-gather NDIS_PACKET*/
				if (pPacket)
				{
	    				pAd->MgmtRing.Cell[i].pNdisPacket = NULL;
					PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr0, pTxD->SDLen0, RTMP_PCI_DMA_TODEVICE);
					RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
				}

				pPacket = (PNDIS_PACKET) pAd->MgmtRing.Cell[i].pNextNdisPacket;
				/* release scatter-and-gather NDIS_PACKET*/
				if (pPacket)
				{
					pAd->MgmtRing.Cell[i].pNextNdisPacket = NULL;
					PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr1, pTxD->SDLen1, RTMP_PCI_DMA_TODEVICE);			
					RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
				}

#ifdef RT_BIG_ENDIAN
				RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
				WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, FALSE, TYPE_TXD);
#endif /* RT_BIG_ENDIAN */
			}

 			RTMP_IO_READ32(pAd, pAd->MgmtRing.hw_didx_addr, &pAd->MgmtRing.TxDmaIdx);
			pAd->MgmtRing.TxSwFreeIdx = pAd->MgmtRing.TxDmaIdx;
			pAd->MgmtRing.TxCpuIdx = pAd->MgmtRing.TxDmaIdx;
			RTMP_IO_WRITE32(pAd, pAd->MgmtRing.hw_cidx_addr, pAd->MgmtRing.TxCpuIdx);

 			RTMP_IRQ_UNLOCK(&pAd->MgmtRingLock, IrqFlags);
			pAd->RalinkCounters.MgmtRingFullCount = 0;
			break;
			
		case QID_RX:
			for (ring_id =0; ring_id < NUM_OF_RX_RING; ring_id++)
			{
				UINT16 RxRingSize = (ring_id == 0) ? RX_RING_SIZE : RX1_RING_SIZE;
				
				RTMP_RX_RING *pRxRing;
				NDIS_SPIN_LOCK *lock;

				pRxRing = &pAd->RxRing[ring_id];
				lock = &pAd->RxRingLock[ring_id];
				
				RTMP_IRQ_LOCK(lock, IrqFlags);
				for (i=0; i<RxRingSize; i++)
				{
#ifdef RT_BIG_ENDIAN
					pDestRxD = (PRXD_STRUC)pRxRing->Cell[i].AllocVa;
					RxD = *pDestRxD;
					pRxD = &RxD;
					RTMPDescriptorEndianChange((PUCHAR)pRxD, TYPE_RXD);
#else
					/* Point to Rx indexed rx ring descriptor*/
					pRxD  = (PRXD_STRUC)pRxRing->Cell[i].AllocVa;
#endif /* RT_BIG_ENDIAN */

					pRxD->DDONE = 0;

#ifdef RT_BIG_ENDIAN
					RTMPDescriptorEndianChange((PUCHAR)pRxD, TYPE_RXD);
					WriteBackToDescriptor((PUCHAR)pDestRxD, (PUCHAR)pRxD, FALSE, TYPE_RXD);
#endif /* RT_BIG_ENDIAN */
				}

				RTMP_IO_READ32(pAd, pRxRing->hw_didx_addr, &pRxRing->RxDmaIdx);
				pRxRing->RxSwReadIdx = pRxRing->RxDmaIdx;
				pRxRing->RxCpuIdx = ((pRxRing->RxDmaIdx == 0) ? (RxRingSize-1) : (pRxRing->RxDmaIdx-1));
				RTMP_IO_WRITE32(pAd, pRxRing->hw_cidx_addr, pRxRing->RxCpuIdx);

				RTMP_IRQ_UNLOCK(lock, IrqFlags);
			}
			break;
			
#ifdef CONFIG_ANDES_SUPPORT
		case QID_CTRL:
			RTMP_IRQ_LOCK(&pAd->CtrlRingLock, IrqFlags);
			
			for (i=0; i<MGMT_RING_SIZE; i++)
			{
#ifdef RT_BIG_ENDIAN
				pDestTxD  = (PTXD_STRUC) pAd->CtrlRing.Cell[i].AllocVa;
				TxD = *pDestTxD;
				pTxD = &TxD;
				RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#else
				pTxD  = (PTXD_STRUC) pAd->CtrlRing.Cell[i].AllocVa;
#endif /* RT_BIG_ENDIAN */

				pPacket = (PNDIS_PACKET) pAd->CtrlRing.Cell[i].pNdisPacket;
				/* rlease scatter-and-gather NDIS_PACKET*/
				if (pPacket)
				{
					pAd->CtrlRing.Cell[i].pNdisPacket = NULL;
					PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr0, pTxD->SDLen0, RTMP_PCI_DMA_TODEVICE);
					RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
				}

				pPacket = (PNDIS_PACKET) pAd->CtrlRing.Cell[i].pNextNdisPacket;
				/* release scatter-and-gather NDIS_PACKET*/
				if (pPacket)
				{
					pAd->CtrlRing.Cell[i].pNextNdisPacket = NULL;
					PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr1, pTxD->SDLen1, RTMP_PCI_DMA_TODEVICE);			
					RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
				}

#ifdef RT_BIG_ENDIAN
				RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
				WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, FALSE, TYPE_TXD);
#endif /* RT_BIG_ENDIAN */
			}

			RTMP_IO_READ32(pAd, pAd->CtrlRing.hw_didx_addr, &pAd->CtrlRing.TxDmaIdx);
			pAd->CtrlRing.TxSwFreeIdx = pAd->CtrlRing.TxDmaIdx;
			pAd->CtrlRing.TxCpuIdx = pAd->CtrlRing.TxDmaIdx;
			RTMP_IO_WRITE32(pAd, pAd->CtrlRing.hw_cidx_addr, pAd->CtrlRing.TxCpuIdx);

			RTMP_IRQ_UNLOCK(&pAd->CtrlRingLock, IrqFlags);
			break;
#endif /* CONFIG_ANDES_SUPPORT */
		default:
			break;
	}
}


/***************************************************************************
  *
  *	register related procedures.
  *
  **************************************************************************/
/*
========================================================================
Routine Description:
    Disable DMA.

Arguments:
	*pAd				the raxx interface data pointer

Return Value:
	None

Note:
========================================================================
*/
VOID RT28XXDMADisable(RTMP_ADAPTER *pAd)
{
	WPDMA_GLO_CFG_STRUC GloCfg;

	RTMP_IO_READ32(pAd, WPDMA_GLO_CFG, &GloCfg.word);
	GloCfg.word &= 0xff0;
	GloCfg.field.EnTXWriteBackDDONE =1;
	RTMP_IO_WRITE32(pAd, WPDMA_GLO_CFG, GloCfg.word);
}


/*
========================================================================
Routine Description:
    Enable DMA.

Arguments:
	*pAd				the raxx interface data pointer

Return Value:
	None

Note:
========================================================================
*/
VOID RT28XXDMAEnable(RTMP_ADAPTER *pAd)
{
	WPDMA_GLO_CFG_STRUC GloCfg;

	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, 0x4);
	AsicWaitPDMAIdle(pAd, 200, 1000);

	RtmpusecDelay(50);

	RTMP_IO_READ32(pAd, WPDMA_GLO_CFG, &GloCfg.word);
	GloCfg.field.EnTXWriteBackDDONE = 1;
	GloCfg.field.WPDMABurstSIZE = pAd->chipCap.WPDMABurstSIZE;
	GloCfg.field.EnableRxDMA = 1;
	GloCfg.field.EnableTxDMA = 1;
	
	RTMP_IO_WRITE32(pAd, WPDMA_GLO_CFG, GloCfg.word);
	DBGPRINT(RT_DEBUG_TRACE, ("<== WRITE DMA offset 0x208 = 0x%x, WPDMABurstSIZE = %d\n", GloCfg.word, pAd->chipCap.WPDMABurstSIZE));
}


BOOLEAN AsicCheckCommanOk(RTMP_ADAPTER *pAd, UCHAR Command)
{
	UINT32	CmdStatus = 0, CID = 0, i;
	UINT32	ThisCIDMask = 0;
#ifdef SPECIFIC_BCN_BUF_SUPPORT
	ULONG	IrqFlags = 0;
#endif /* SPECIFIC_BCN_BUF_SUPPORT */

#ifdef RT65xx
	if (IS_RT65XX(pAd))
		return TRUE;
#endif /* RT65xx */


#ifdef SPECIFIC_BCN_BUF_SUPPORT
	RTMP_MAC_SHR_MSEL_PROTECT_LOCK(pAd, IrqFlags);
#endif /* SPECIFIC_BCN_BUF_SUPPORT */
	i = 0;
	do
	{
		RTMP_IO_READ32(pAd, H2M_MAILBOX_CID, &CID);
		/* Find where the command is. Because this is randomly specified by firmware.*/
		if ((CID & CID0MASK) == Command)
		{
			ThisCIDMask = CID0MASK;
			break;
		}
		else if ((((CID & CID1MASK)>>8) & 0xff) == Command)
		{
			ThisCIDMask = CID1MASK;
			break;
		}
		else if ((((CID & CID2MASK)>>16) & 0xff) == Command)
		{
			ThisCIDMask = CID2MASK;
			break;
		}
		else if ((((CID & CID3MASK)>>24) & 0xff) == Command)
		{
			ThisCIDMask = CID3MASK;
			break;
		}

		RtmpusecDelay(100);
		i++;
	}while (i < 200);

	/* Get CommandStatus Value*/
	RTMP_IO_READ32(pAd, H2M_MAILBOX_STATUS, &CmdStatus);
	
	/* This command's status is at the same position as command. So AND command position's bitmask to read status.	*/
	if (i < 200)
	{
		/* If Status is 1, the comamnd is success.*/
		if (((CmdStatus & ThisCIDMask) == 0x1) || ((CmdStatus & ThisCIDMask) == 0x100) 
			|| ((CmdStatus & ThisCIDMask) == 0x10000) || ((CmdStatus & ThisCIDMask) == 0x1000000))
		{
			DBGPRINT(RT_DEBUG_TRACE, ("--> AsicCheckCommanOk CID = 0x%x, CmdStatus= 0x%x \n", CID, CmdStatus));
			RTMP_IO_WRITE32(pAd, H2M_MAILBOX_STATUS, 0xffffffff);
			RTMP_IO_WRITE32(pAd, H2M_MAILBOX_CID, 0xffffffff);
#ifdef SPECIFIC_BCN_BUF_SUPPORT
			RTMP_MAC_SHR_MSEL_PROTECT_UNLOCK(pAd, IrqFlags);
#endif /* SPECIFIC_BCN_BUF_SUPPORT */
			return TRUE;
		}
		DBGPRINT(RT_DEBUG_TRACE, ("--> AsicCheckCommanFail1 CID = 0x%x, CmdStatus= 0x%x \n", CID, CmdStatus));
	}
	else
	{
		DBGPRINT(RT_DEBUG_TRACE, ("--> AsicCheckCommanFail2 Timeout Command = %d, CmdStatus= 0x%x \n", Command, CmdStatus));
	}
	/* Clear Command and Status.*/
	RTMP_IO_WRITE32(pAd, H2M_MAILBOX_STATUS, 0xffffffff);
	RTMP_IO_WRITE32(pAd, H2M_MAILBOX_CID, 0xffffffff);
#ifdef SPECIFIC_BCN_BUF_SUPPORT
   	RTMP_MAC_SHR_MSEL_PROTECT_UNLOCK(pAd, IrqFlags);
#endif /* SPECIFIC_BCN_BUF_SUPPORT */
	return FALSE;
}


/*
========================================================================
Routine Description:
    Write Beacon buffer to Asic.

Arguments:
	*pAd				the raxx interface data pointer

Return Value:
	None

Note:
========================================================================
*/
VOID RT28xx_UpdateBeaconToAsic(
	IN RTMP_ADAPTER		*pAd,
	IN INT				apidx,
	IN ULONG			FrameLen, 
	IN ULONG			UpdatePos)
{
	ULONG CapInfoPos = 0;
	UCHAR *ptr, *ptr_update, *ptr_capinfo;
	UINT i;
	BOOLEAN bBcnReq = FALSE;
	UCHAR bcn_idx = 0;
	UINT8 TXWISize = pAd->chipCap.TXWISize;
#ifdef CONFIG_AP_SUPPORT
	MULTISSID_STRUCT *pMbss;
#endif
	INT wr_bytes = 1;

	if (IS_MT76x2(pAd))
		wr_bytes = 4;

#ifdef CONFIG_AP_SUPPORT
	if (apidx < pAd->ApCfg.BssidNum &&
		apidx < HW_BEACON_MAX_NUM &&
	       (pAd->OpMode == OPMODE_AP)
		)
	{
		//MULTISSID_STRUCT *pMbss;

		pMbss = &pAd->ApCfg.MBSSID[apidx];
		bcn_idx = pMbss->BcnBufIdx;
		CapInfoPos = pMbss->CapabilityInfoLocationInBeacon;
		bBcnReq = BeaconTransmitRequired(pAd, apidx, pMbss);

		ptr_capinfo = (PUCHAR)&pMbss->BeaconBuf[CapInfoPos];
		ptr_update  = (PUCHAR)&pMbss->BeaconBuf[UpdatePos];
	}
	else
#endif /* CONFIG_AP_SUPPORT */
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s() : No valid Interface be found.\n", __FUNCTION__));
		return;
	}

	if (bBcnReq == FALSE)
	{
		/* when the ra interface is down, do not send its beacon frame */
		/* clear all zero */
		for(i=0; i < TXWISize; i+=4)
			RTMP_CHIP_UPDATE_BEACON(pAd, pAd->BeaconOffset[bcn_idx] + i, 0x00, 4);
	}
	else
	{
		ptr = (PUCHAR)&pAd->BeaconTxWI;

#ifdef RT_BIG_ENDIAN
		RTMPWIEndianChange(pAd, ptr, TYPE_TXWI);
#endif
		for (i=0; i < TXWISize; i+=4)  /* 16-byte TXWI field*/
		{
			UINT32 longptr =  *ptr + (*(ptr+1)<<8) + (*(ptr+2)<<16) + (*(ptr+3)<<24);
			RTMP_CHIP_UPDATE_BEACON(pAd, pAd->BeaconOffset[bcn_idx] + i, longptr, 4);
			ptr += 4;
		}

		/* Update CapabilityInfo in Beacon*/
#ifdef RT6352
		if (IS_RT6352(pAd))
		{
			/* Update CapabilityInfo in Beacon*/
			for (i = CapInfoPos; i < (CapInfoPos+2); i++)
			{
				RTMP_CHIP_UPDATE_BEACON(pAd, pAd->BeaconOffset[bcn_idx] + TXWISize + i, *ptr_capinfo, 1);
				ptr_capinfo ++;
			}
			
			if (FrameLen > UpdatePos)
			{
				for (i= UpdatePos; i< (FrameLen); i++)
				{
					RTMP_CHIP_UPDATE_BEACON(pAd, pAd->BeaconOffset[bcn_idx] + TXWISize + i, *ptr_update, 1);
					ptr_update ++;
				}
			}
		}
		else
#endif
		{
			if (wr_bytes > 1)
				CapInfoPos = (CapInfoPos & (~(wr_bytes - 1)));
#ifdef CONFIG_AP_SUPPORT
			ptr_capinfo = (PUCHAR)&pMbss->BeaconBuf[CapInfoPos];
#endif /* CONFIG_AP_SUPPORT */
			for (i = CapInfoPos; i < (CapInfoPos+2); i += wr_bytes)
			{
#ifdef RT_BIG_ENDIAN		
				UINT32 longptr =  *ptr_capinfo + (*(ptr_capinfo+1)<<8) + (*(ptr_capinfo+2)<<16) + (*(ptr_capinfo+3)<<24);
				RTMP_CHIP_UPDATE_BEACON(pAd, pAd->BeaconOffset[bcn_idx] + TXWISize + i, longptr, wr_bytes);
#else
				RTMP_CHIP_UPDATE_BEACON(pAd, pAd->BeaconOffset[bcn_idx] + TXWISize + i, *((UINT32 *)ptr_capinfo), wr_bytes);
#endif /* RT_BIG_ENDIAN */
				ptr_capinfo += wr_bytes;
			}

			if (FrameLen > UpdatePos)
			{
				if (wr_bytes > 1)
					UpdatePos = (UpdatePos & (~(wr_bytes - 1)));
#ifdef CONFIG_AP_SUPPORT
				ptr_update  = (PUCHAR)&pMbss->BeaconBuf[UpdatePos];
#endif /* CONFIG_AP_SUPPORT */
				for (i = UpdatePos; i < (FrameLen); i += wr_bytes)
				{
					UINT32 longptr =  *ptr_update + (*(ptr_update+1)<<8) + (*(ptr_update+2)<<16) + (*(ptr_update+3)<<24);
					RTMP_IO_WRITE32(pAd, pAd->BeaconOffset[bcn_idx] + TXWISize + i, longptr);
					ptr_update += wr_bytes;
				}
			}
		}
	}
}


#ifdef CONFIG_STA_SUPPORT
VOID RT28xxPciStaAsicForceWakeup(RTMP_ADAPTER *pAd, BOOLEAN bFromTx)
{
    AUTO_WAKEUP_STRUC AutoWakeupCfg;

    if (!OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE) ||
		OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_WAKEUP_NOW))
        return;

    OPSTATUS_SET_FLAG(pAd, fOP_STATUS_WAKEUP_NOW);

   /* RTMP_CLEAR_PSFLAG(pAd, fRTMP_PS_GO_TO_SLEEP_NOW);*/
#ifdef PCIE_PS_SUPPORT
    if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_ADVANCE_POWER_SAVE_PCIE_DEVICE)
		&&pAd->StaCfg.PSControl.field.EnableNewPS == TRUE)
    {
        /* Support PCIe Advance Power Save*/
    	if (bFromTx == TRUE
			&&(pAd->Mlme.bPsPollTimerRunning == TRUE))
    	{
            pAd->Mlme.bPsPollTimerRunning = FALSE;
    		RTMPPCIeLinkCtrlValueRestore(pAd, RESTORE_WAKEUP);
    		RtmpusecDelay(3000);
            DBGPRINT(RT_DEBUG_TRACE, ("=======AsicForceWakeup===bFromTx\n"));
    	}

		AutoWakeupCfg.word = 0;
		RTMP_IO_WRITE32(pAd, AUTO_WAKEUP_CFG, AutoWakeupCfg.word);
	
        if (RT28xxPciAsicRadioOn(pAd, DOT11POWERSAVE))
        {
			RTMP_CHIP_OP *pChipOps = &pAd->chipOps;
				
			if (pChipOps->AsicReverseRfFromSleepMode)
				pChipOps->AsicReverseRfFromSleepMode(pAd, FALSE);
			else
			{
				UCHAR rf_channel;

				/* In Radio Off, we turn off RF clk, So now need to call ASICSwitchChannel again.*/
				if (INFRA_ON(pAd) && (pAd->CommonCfg.CentralChannel != pAd->CommonCfg.Channel) 
					&& (pAd->MlmeAux.HtCapability.HtCapInfo.ChannelWidth == BW_40))
					rf_channel = pAd->CommonCfg.CentralChannel;
				else
					rf_channel = pAd->CommonCfg.Channel;
				AsicSwitchChannel(pAd, rf_channel, FALSE);
				AsicLockChannel(pAd, rf_channel);

			} 
        }
    }
    else
#endif /* PCIE_PS_SUPPORT */
    {
    	BOOLEAN	Canceled;
	
        /* PCI, 2860-PCIe*/
         DBGPRINT(RT_DEBUG_TRACE, ("<==RT28xxPciStaAsicForceWakeup::Original PCI Power Saving\n"));
#ifdef MT76x2
		if (IS_MT76x2(pAd))
		{
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_MCU_SEND_IN_BAND_CMD);
			PWR_SAVING_OP(pAd, RADIO_ON, 0, 0, 0, 0, 0);

			if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_ACTIVE))
        			RTMP_ASIC_INTERRUPT_ENABLE(pAd);
				
			if (pAd->Mlme.bPsPollTimerRunning)
			{
				RTMPCancelTimer(&pAd->Mlme.PsPollTimer, &Canceled);
				pAd->Mlme.bPsPollTimerRunning = FALSE;
			}
		}
		else
#endif /* MT76x2 */
		{
        	AsicSendCommandToMcu(pAd, 0x31, 0xff, 0x00, 0x02, FALSE);
        	AutoWakeupCfg.word = 0;
	    	RTMP_IO_WRITE32(pAd, AUTO_WAKEUP_CFG, AutoWakeupCfg.word);
		}
    }
    
    OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_DOZE);
    OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_WAKEUP_NOW);
    DBGPRINT(RT_DEBUG_TRACE, ("<=======RT28xxPciStaAsicForceWakeup\n"));
}


VOID RT28xxPciStaAsicSleepThenAutoWakeup(
	IN RTMP_ADAPTER *pAd,
	IN USHORT TbttNumToNextWakeUp)
{
#ifdef PCIE_PS_SUPPORT
	BOOLEAN brc;
#endif /* PCIE_PS_SUPPORT */
	
	if ((pAd->StaCfg.bRadio == FALSE) ||
		(OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_WAKEUP_NOW)) ||
		(pAd->CountDowntoPsm > 0))
	{
		OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_DOZE);
		return;
	}
	
#ifdef PCIE_PS_SUPPORT
	if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_ADVANCE_POWER_SAVE_PCIE_DEVICE)
		&&pAd->StaCfg.PSControl.field.EnableNewPS == TRUE)
	{
		ULONG	Now = 0;

		NdisGetSystemUpTime(&Now);
		/* If last send NULL fram time is too close to this receiving beacon (within 8ms), don't go to sleep for this DTM.*/
		/* Because Some AP can't queuing outgoing frames immediately.*/
		if (((pAd->Mlme.LastSendNULLpsmTime + 8) >= Now) && (pAd->Mlme.LastSendNULLpsmTime <= Now))		
		{
			DBGPRINT(RT_DEBUG_TRACE, ("Now = %lu, LastSendNULLpsmTime=%lu :  RxCountSinceLastNULL = %lu. \n", Now, pAd->Mlme.LastSendNULLpsmTime, pAd->RalinkCounters.RxCountSinceLastNULL));
			return;
		}
		else if ((pAd->RalinkCounters.RxCountSinceLastNULL > 0) && ((pAd->Mlme.LastSendNULLpsmTime + pAd->CommonCfg.BeaconPeriod) >= Now))		
		{
			DBGPRINT(RT_DEBUG_TRACE, ("Now = %lu, LastSendNULLpsmTime=%lu: RxCountSinceLastNULL = %lu > 0 \n", Now, pAd->Mlme.LastSendNULLpsmTime,  pAd->RalinkCounters.RxCountSinceLastNULL));			
			return;
		}
		
		brc = RT28xxPciAsicRadioOff(pAd, DOT11POWERSAVE, TbttNumToNextWakeUp);
		if (brc==TRUE)
			OPSTATUS_SET_FLAG(pAd, fOP_STATUS_DOZE);
	}
	else
#endif /* PCIE_PS_SUPPORT */
	{
#ifdef MT76x2
		if (IS_MT76x2(pAd))
		{
			UINT32 WakeUpTime = AUTO_WAKEUP_TIMEOUT; /*unit: msec */
			UINT32 LeadTimeRatio = 30; /* % */
			UINT32 LeadTime = 0; /* unit: msec */
			UINT32 pwr_level = 5;

		 	if(TbttNumToNextWakeUp == 0)
		 	{
		 		DBGPRINT(RT_DEBUG_TRACE, ("give up this sleep. \n"));
	            OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_DOZE);
				return;
		 	}

	    	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_ACTIVE))
    			RTMP_ASIC_INTERRUPT_DISABLE(pAd);
			
	    	if(!pAd->Mlme.bPsPollTimerRunning)
			{
				WakeUpTime = (TbttNumToNextWakeUp*pAd->CommonCfg.BeaconPeriod);
				LeadTime = (pAd->CommonCfg.BeaconPeriod*LeadTimeRatio)/100;
				if(WakeUpTime >= LeadTime) 
					WakeUpTime -= LeadTime;
				
				RTMPSetTimer(&pAd->Mlme.PsPollTimer, WakeUpTime);
				pAd->Mlme.bPsPollTimerRunning = TRUE;
			}
			
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_MCU_SEND_IN_BAND_CMD);
			PWR_SAVING_OP(pAd, RADIO_OFF, pwr_level, 0, 0, 0, 0);
		}
		else
#endif /* MT76x2 */
		{
			AUTO_WAKEUP_STRUC	AutoWakeupCfg;	
			/* we have decided to SLEEP, so at least do it for a BEACON period.	*/
			if (TbttNumToNextWakeUp == 0)
				TbttNumToNextWakeUp = 1; 

			AutoWakeupCfg.word = 0; 
			RTMP_IO_WRITE32(pAd, AUTO_WAKEUP_CFG, AutoWakeupCfg.word);  
			AutoWakeupCfg.field.NumofSleepingTbtt = TbttNumToNextWakeUp - 1;    
			AutoWakeupCfg.field.EnableAutoWakeup = 1;   
			AutoWakeupCfg.field.AutoLeadTime = 5;   
			RTMP_IO_WRITE32(pAd, AUTO_WAKEUP_CFG, AutoWakeupCfg.word);  
			AsicSendCommandToMcu(pAd, 0x30, 0xff, 0xff, 0x00, FALSE);   /* send POWER-SAVE command to MCU. Timeout 40us.*/
		}
		OPSTATUS_SET_FLAG(pAd, fOP_STATUS_DOZE);
		DBGPRINT(RT_DEBUG_TRACE, ("<-- %s, TbttNumToNextWakeUp=%d \n", __FUNCTION__, TbttNumToNextWakeUp));
	}

}

VOID PsPollWakeExec(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3) 
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)FunctionContext;
	unsigned long flags;

    DBGPRINT(RT_DEBUG_TRACE,("-->PsPollWakeExec \n"));

	RTMP_INT_LOCK(&pAd->irq_lock, flags);
#ifdef PCIE_PS_SUPPORT
    if (pAd->Mlme.bPsPollTimerRunning)
    {
	    RTMPPCIeLinkCtrlValueRestore(pAd, RESTORE_WAKEUP);
		pAd->Mlme.bPsPollTimerRunning = FALSE;
    }
#endif /* PCIE_PS_SUPPORT */
	RTMP_INT_UNLOCK(&pAd->irq_lock, flags);

#ifdef MT76x2
	if (IS_MT76x2(pAd))
		AsicForceWakeup(pAd,FALSE);
#endif /* MT76x2 */
}

#ifdef PCIE_PS_SUPPORT
VOID  RadioOnExec(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3) 
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)FunctionContext;
	RTMP_CHIP_OP *pChipOps = &pAd->chipOps;
	WPDMA_GLO_CFG_STRUC	DmaCfg;
	BOOLEAN				Cancelled;

	if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE))
	{
		DBGPRINT(RT_DEBUG_TRACE,("-->RadioOnExec() return on fOP_STATUS_DOZE == TRUE; \n"));
		if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_ADVANCE_POWER_SAVE_PCIE_DEVICE)
			&&pAd->StaCfg.PSControl.field.EnableNewPS == TRUE)
		RTMPSetTimer(&pAd->Mlme.RadioOnOffTimer, 10);
		return;
	}
	
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))
	{
		DBGPRINT(RT_DEBUG_TRACE,("-->RadioOnExec() return on SCAN_IN_PROGRESS; \n"));


		if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_ADVANCE_POWER_SAVE_PCIE_DEVICE)
				&&pAd->StaCfg.PSControl.field.EnableNewPS == TRUE)
		RTMPSetTimer(&pAd->Mlme.RadioOnOffTimer, 10);
		return;
	}
	
if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_ADVANCE_POWER_SAVE_PCIE_DEVICE)
	&&pAd->StaCfg.PSControl.field.EnableNewPS == TRUE)
	{
	pAd->Mlme.bPsPollTimerRunning = FALSE;
	RTMPCancelTimer(&pAd->Mlme.PsPollTimer,	&Cancelled);
	}


	if (pAd->StaCfg.bRadio == TRUE)
	{
		UCHAR rf_channel;

		pAd->bPCIclkOff = FALSE;
		RTMPRingCleanUp(pAd, QID_AC_BK);
		RTMPRingCleanUp(pAd, QID_AC_BE);
		RTMPRingCleanUp(pAd, QID_AC_VI);
		RTMPRingCleanUp(pAd, QID_AC_VO);
		RTMPRingCleanUp(pAd, QID_HCCA);
		RTMPRingCleanUp(pAd, QID_MGMT);
		RTMPRingCleanUp(pAd, QID_RX);

		/* 2. Send wake up command and wait for command done. */
		AsicSendCmdToMcuAndWait(pAd, 0x31, PowerWakeCID, 0x00, 0x02, FALSE);
        
		/* When PCI clock is off, don't want to service interrupt. So when back to clock on, enable interrupt.*/
		RTMP_ASIC_INTERRUPT_ENABLE(pAd);
		
		/* 3. Enable Tx DMA.*/
		RTMP_IO_READ32(pAd, WPDMA_GLO_CFG, &DmaCfg.word);
		DmaCfg.field.EnableTxDMA = 1;
		RTMP_IO_WRITE32(pAd, WPDMA_GLO_CFG, DmaCfg.word);
		
		/* In Radio Off, we turn off RF clk, So now need to call ASICSwitchChannel again.*/
		if (INFRA_ON(pAd) && (pAd->CommonCfg.CentralChannel != pAd->CommonCfg.Channel) 
			&& (pAd->MlmeAux.HtCapability.HtCapInfo.ChannelWidth == BW_40))
			rf_channel = pAd->CommonCfg.CentralChannel;
		else
			rf_channel = pAd->CommonCfg.Channel;
		AsicSwitchChannel(pAd, rf_channel, FALSE);
		AsicLockChannel(pAd, rf_channel);
			
		if (pChipOps->AsicReverseRfFromSleepMode)
			pChipOps->AsicReverseRfFromSleepMode(pAd, FALSE);

		/* Clear Radio off flag*/
		RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF);

#ifdef LED_CONTROL_SUPPORT
		/* Set LED*/
		RTMPSetLED(pAd, LED_RADIO_ON);
#endif /* LED_CONTROL_SUPPORT */

		if (RtmpPktPmBitCheck(pAd) == FALSE)
		{
#ifdef RTMP_BBP
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, pAd->StaCfg.BBPR3);
#endif /* RTMP_BBP */
		}
	}
	else
	{
		RT28xxPciAsicRadioOff(pAd, GUIRADIO_OFF, 0);
	}
}
#endif /* PCIE_PS_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */


/*
	==========================================================================
	Description:
		This routine sends command to firmware and turn our chip to wake up mode from power save mode.
		Both RadioOn and .11 power save function needs to call this routine.
	Input:
		Level = GUIRADIO_OFF : call this function is from Radio Off to Radio On.  Need to restore PCI host value.
		Level = other value : normal wake up function.

	==========================================================================
 */
BOOLEAN RT28xxPciAsicRadioOn(RTMP_ADAPTER *pAd, UCHAR Level)
{
#ifdef CONFIG_STA_SUPPORT    
#ifdef PCIE_PS_SUPPORT
	BOOLEAN Cancelled;   
#endif /* PCIE_PS_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */

	if (pAd->OpMode == OPMODE_AP && Level==DOT11POWERSAVE)
		return FALSE;

#ifdef CONFIG_STA_SUPPORT
#ifdef PCIE_PS_SUPPORT
	if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_ADVANCE_POWER_SAVE_PCIE_DEVICE))
	{
		if (pAd->StaCfg.PSControl.field.EnableNewPS == TRUE)
		{
	    	pAd->Mlme.bPsPollTimerRunning = FALSE;
			RTMPCancelTimer(&pAd->Mlme.PsPollTimer,	&Cancelled);
		}
		if ((pAd->StaCfg.PSControl.field.EnableNewPS == TRUE)&&
			(((Level == GUIRADIO_OFF) || (Level == GUI_IDLE_POWER_SAVE))
			||(RTMP_TEST_PSFLAG(pAd, fRTMP_PS_SET_PCI_CLK_OFF_COMMAND))))
		{
			/* Some chips don't need to delay 6ms, so copy RTMPPCIePowerLinkCtrlRestore*/
			/* return condition here.*/
			/*
			if (((pAd->MACVersion&0xffff0000) != 0x28600000)
				&& ((pAd->DeviceID == NIC2860_PCIe_DEVICE_ID)
				||(pAd->DeviceID == NIC2790_PCIe_DEVICE_ID)))
			*/
			{
				DBGPRINT(RT_DEBUG_TRACE, ("RT28xxPciAsicRadioOn ()\n"));
				/* 1. Set PCI Link Control in Configuration Space.*/
				RTMPPCIeLinkCtrlValueRestore(pAd, RESTORE_WAKEUP);
				RtmpusecDelay(6000);
			}
		}
	}
    
	{	
    		pAd->bPCIclkOff = FALSE;
		DBGPRINT(RT_DEBUG_TRACE, ("PSM :309xbPCIclkOff == %d\n", pAd->bPCIclkOff));
	}
#endif /* PCIE_PS_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */

	/* 2. Send wake up command.*/
	AsicSendCommandToMcu(pAd, 0x31, PowerWakeCID, 0x00, 0x02, FALSE);
	pAd->bPCIclkOff = FALSE;
	/* 2-1. wait command ok.*/
	AsicCheckCommanOk(pAd, PowerWakeCID);
    	RTMP_ASIC_INTERRUPT_ENABLE(pAd);
        

    	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF);
    	if (Level == GUI_IDLE_POWER_SAVE)
    	{
 /*2009/06/09: AP and stations need call the following function*/
 #ifndef RTMP_RBUS_SUPPORT
			/* add by johnli, RF power sequence setup, load RF normal operation-mode setup*/

		RTMP_CHIP_OP *pChipOps = &pAd->chipOps;

		if (pChipOps->AsicReverseRfFromSleepMode)
		{
			pChipOps->AsicReverseRfFromSleepMode(pAd, FALSE);
#ifdef CONFIG_STA_SUPPORT
#ifdef PCIE_PS_SUPPORT
			/* 3090 MCU Wakeup command needs more time to be stable.*/
			/* Before stable, don't issue other MCU command to prevent from firmware error.	*/

			if ( pAd->chipCap.HW_PCIE_PS_L3_ENABLE==TRUE)
			{
				RTMP_SEM_LOCK(&pAd->McuCmdLock);
				pAd->brt30xxBanMcuCmd = FALSE;
				RTMP_SEM_UNLOCK(&pAd->McuCmdLock);
			}
#endif /* PCIE_PS_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */

		}
		else
#endif /* RTMP_RBUS_SUPPORT */
		{
	    		/* In Radio Off, we turn off RF clk, So now need to call ASICSwitchChannel again.*/
#ifdef CONFIG_STA_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
			{
				UCHAR rf_channel;

	    		if (INFRA_ON(pAd) && (pAd->CommonCfg.CentralChannel != pAd->CommonCfg.Channel) 
	    			&& (pAd->MlmeAux.HtCapability.HtCapInfo.ChannelWidth == BW_40))
					rf_channel = pAd->CommonCfg.CentralChannel;
	    		else
					rf_channel = pAd->CommonCfg.Channel;
    			AsicSwitchChannel(pAd, rf_channel, FALSE);
    			AsicLockChannel(pAd, rf_channel);
			}
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
			{
				AsicSwitchChannel(pAd, pAd->CommonCfg.CentralChannel, FALSE);
				AsicLockChannel(pAd, pAd->CommonCfg.CentralChannel);
			}
#endif /* CONFIG_AP_SUPPORT */
	}
    	}
        return TRUE;

}


/*
	==========================================================================
	Description:
		This routine sends command to firmware and turn our chip to power save mode.
		Both RadioOff and .11 power save function needs to call this routine.
	Input:
		Level = GUIRADIO_OFF  : GUI Radio Off mode
		Level = DOT11POWERSAVE  : 802.11 power save mode 
		Level = RTMP_HALT  : When Disable device. 
		
	==========================================================================
 */
BOOLEAN RT28xxPciAsicRadioOff(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR Level, 
	IN USHORT TbttNumToNextWakeUp) 
{
#ifdef CONFIG_STA_SUPPORT
#ifdef RTMP_BBP
	UCHAR tempBBP_R3 = 0;
#endif /* RTMP_BBP */
#ifdef PCIE_PS_SUPPORT
	UCHAR		i;
    ULONG		BeaconPeriodTime;
	UINT32		PsPollTime = 0/*, MACValue*/;
	UINT32		TbTTTime = 0;
	BOOLEAN		Cancelled;
#endif /* PCIE_PS_SUPPORT */	
#endif /* CONFIG_STA_SUPPORT */
#if (defined(CONFIG_STA_SUPPORT) && defined(PCIE_PS_SUPPORT)) || defined(RT2860)
	BOOLEAN		brc = FALSE;
#endif /*(defined(CONFIG_STA_SUPPORT) && defined(PCIE_PS_SUPPORT)) || defined(RT2860) */


    UINT32 RxDmaIdx, RxCpuIdx;
	DBGPRINT(RT_DEBUG_TRACE, ("%s ===> Lv= %d, TxCpuIdx = %d, TxDmaIdx = %d. RxCpuIdx = %d, RxDmaIdx = %d.\n", 
								__FUNCTION__, Level,pAd->TxRing[0].TxCpuIdx, pAd->TxRing[0].TxDmaIdx, 
								pAd->RxRing[0].RxCpuIdx, pAd->RxRing[0].RxDmaIdx));

	if (pAd->OpMode == OPMODE_AP && Level==DOT11POWERSAVE)
		return FALSE;

	if (Level == DOT11POWERSAVE)
	{
		/* Check Rx DMA busy status, if more than half is occupied, give up this radio off.*/
		RTMP_IO_READ32(pAd, pAd->RxRing[0].hw_didx_addr, &RxDmaIdx);
		RTMP_IO_READ32(pAd, pAd->RxRing[0].hw_cidx_addr, &RxCpuIdx);
		if ((RxDmaIdx > RxCpuIdx) && ((RxDmaIdx - RxCpuIdx) > RX_RING_SIZE/3))
		{
			DBGPRINT(RT_DEBUG_TRACE, ("%s(): return1. RxDmaIdx=%d, RxCpuIdx=%d\n",
						__FUNCTION__, RxDmaIdx, RxCpuIdx));
			return FALSE;
		}
		else if ((RxCpuIdx >= RxDmaIdx) && ((RxCpuIdx - RxDmaIdx) < RX_RING_SIZE/3))
		{
			DBGPRINT(RT_DEBUG_TRACE, ("%s(): return2. RxDmaIdx=%d, RxCpuIdx=%d\n",
						__FUNCTION__, RxDmaIdx, RxCpuIdx));
			return FALSE;
		}
	}


    /* Once go into this function, disable tx because don't want too many packets in queue to prevent HW stops.*/
	/*pAd->bPCIclkOffDisableTx = TRUE;*/
#ifdef CONFIG_STA_SUPPORT
#ifdef PCIE_PS_SUPPORT
	RTMP_SET_PSFLAG(pAd, fRTMP_PS_DISABLE_TX);
	if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_ADVANCE_POWER_SAVE_PCIE_DEVICE) 
		&& pAd->OpMode == OPMODE_STA
		&&pAd->StaCfg.PSControl.field.EnableNewPS == TRUE
		)
	{

	    RTMPCancelTimer(&pAd->Mlme.RadioOnOffTimer,	&Cancelled);
	    RTMPCancelTimer(&pAd->Mlme.PsPollTimer,	&Cancelled);

	    if (Level == DOT11POWERSAVE)
		{
			RTMP_IO_READ32(pAd, TBTT_TIMER, &TbTTTime);
			TbTTTime &= 0x1ffff;
			/* 00. check if need to do sleep in this DTIM period.   If next beacon will arrive within 30ms , ...doesn't necessarily sleep.*/
			/* TbTTTime uint = 64us, LEAD_TIME unit = 1024us, PsPollTime unit = 1ms*/
	        if  (((64*TbTTTime) <((LEAD_TIME*1024) + 40000)) && (TbttNumToNextWakeUp == 0))
			{
				DBGPRINT(RT_DEBUG_TRACE, ("TbTTTime = 0x%x , give up this sleep. \n", TbTTTime));
	            OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_DOZE);
	            		/*pAd->bPCIclkOffDisableTx = FALSE;*/
	            		RTMP_CLEAR_PSFLAG(pAd, fRTMP_PS_DISABLE_TX);
				return FALSE;
			}
			else
			{
				PsPollTime = (64*TbTTTime- LEAD_TIME*1024)/1000;
				PsPollTime -= 3;

	            BeaconPeriodTime = pAd->CommonCfg.BeaconPeriod*102/100;
				if (TbttNumToNextWakeUp > 0)
					PsPollTime += ((TbttNumToNextWakeUp -1) * BeaconPeriodTime);
	            
	            pAd->Mlme.bPsPollTimerRunning = TRUE;
				RTMPSetTimer(&pAd->Mlme.PsPollTimer, PsPollTime);
				}
			}
		}
    	else
    	{
		DBGPRINT(RT_DEBUG_TRACE, ("RT28xxPciAsicRadioOff::Level!=DOT11POWERSAVE \n"));
    	}
#endif /* PCIE_PS_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */
	/*pAd->bPCIclkOffDisableTx = FALSE;*/
    RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF);
    
#ifdef CONFIG_STA_SUPPORT
    /* Set to 1R.*/
	if (pAd->Antenna.field.RxPath > 1 && pAd->OpMode == OPMODE_STA)
	{
#ifdef RTMP_BBP
		if (pAd->chipCap.hif_type == HIF_RTMP) {
			tempBBP_R3 = (pAd->StaCfg.BBPR3 & 0xE7);
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, tempBBP_R3);
		}
#endif /* RTMP_BBP */
		// TODO: shiang, how about RLT_BBP for this??
	}
#endif /* CONFIG_STA_SUPPORT */
    
	/* In Radio Off, we turn off RF clk, So now need to call ASICSwitchChannel again.*/
	if ((INFRA_ON(pAd) || pAd->OpMode == OPMODE_AP) && (pAd->CommonCfg.CentralChannel != pAd->CommonCfg.Channel) 
		&& (pAd->MlmeAux.HtCapability.HtCapInfo.ChannelWidth == BW_40))
		AsicTurnOffRFClk(pAd, pAd->CommonCfg.CentralChannel);
	else
		AsicTurnOffRFClk(pAd, pAd->CommonCfg.Channel);

	if (Level != RTMP_HALT)
	{
		UINT32 AutoWakeupInt = 0;
#ifdef RLT_MAC
		if (pAd->chipCap.hif_type == HIF_RLT)
			AutoWakeupInt = RLT_AutoWakeupInt;
#endif /* RLT_MAC*/
#ifdef RTMP_MAC
		if (pAd->chipCap.hif_type == HIF_RTMP)
			AutoWakeupInt = RTMP_AutoWakeupInt;
#endif /* RTMP_MAC */
		/*
			Change Interrupt bitmask.
			When PCI clock is off, don't want to service interrupt.
		*/
		RTMP_IO_WRITE32(pAd, INT_MASK_CSR, AutoWakeupInt);
	}
	else
	{
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_ACTIVE))
			RTMP_ASIC_INTERRUPT_DISABLE(pAd);
	}

	RTMP_IO_WRITE32(pAd, pAd->RxRing[0].hw_cidx_addr, pAd->RxRing[0].RxCpuIdx);
	/*  2. Send Sleep command */
	RTMP_IO_WRITE32(pAd, H2M_MAILBOX_STATUS, 0xffffffff);
	RTMP_IO_WRITE32(pAd, H2M_MAILBOX_CID, 0xffffffff);    
	/* send POWER-SAVE command to MCU. high-byte = 1 save power as much as possible. high byte = 0 save less power*/
	AsicSendCommandToMcu(pAd, SLEEP_MCU_CMD, PowerSafeCID, 0xff, 0x1, FALSE);   

    
#ifdef CONFIG_STA_SUPPORT
	/* 1. Wait DMA not busy*/
	AsicWaitPDMAIdle(pAd, 50, 20);
#endif /* CONFIG_STA_SUPPORT */

/* Disable for stability. If PCIE Link Control is modified for advance power save, re-covery this code segment.*/
/*RTMP_IO_WRITE32(pAd, PBF_SYS_CTRL, 0x1280);*/
/*OPSTATUS_SET_FLAG(pAd, fOP_STATUS_CLKSELECT_40MHZ);*/

#ifdef CONFIG_STA_SUPPORT
#ifdef PCIE_PS_SUPPORT
#endif /* PCIE_PS_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */
	if (Level == DOT11POWERSAVE)
	{
		AUTO_WAKEUP_STRUC	AutoWakeupCfg;
		/*RTMPSetTimer(&pAd->Mlme.PsPollTimer, 90);*/
			
		/* we have decided to SLEEP, so at least do it for a BEACON period.*/
		if (TbttNumToNextWakeUp == 0)
			TbttNumToNextWakeUp = 1;

		AutoWakeupCfg.word = 0;
		RTMP_IO_WRITE32(pAd, AUTO_WAKEUP_CFG, AutoWakeupCfg.word);

		/* 1. Set auto wake up timer.*/
		AutoWakeupCfg.field.NumofSleepingTbtt = TbttNumToNextWakeUp - 1;
		AutoWakeupCfg.field.EnableAutoWakeup = 1;
		AutoWakeupCfg.field.AutoLeadTime = LEAD_TIME;
		RTMP_IO_WRITE32(pAd, AUTO_WAKEUP_CFG, AutoWakeupCfg.word);
	}
	
#ifdef CONFIG_STA_SUPPORT
#ifdef PCIE_PS_SUPPORT

	/*  4-1. If it's to disable our device. Need to restore PCI Configuration Space to its original value.*/
	if (Level == RTMP_HALT && pAd->OpMode == OPMODE_STA)
	{
		if ((brc == TRUE) && (i < 50))
			RTMPPCIeLinkCtrlSetting(pAd, 1);
	}
	/*  4. Set PCI configuration Space Link Comtrol fields.  Only Radio Off needs to call this function*/
	else if (pAd->OpMode == OPMODE_STA)
	{
		if ((brc == TRUE) && (i < 50))
			RTMPPCIeLinkCtrlSetting(pAd, 3);
	}
	RTMP_CLEAR_PSFLAG(pAd, fRTMP_PS_DISABLE_TX);
#endif /* PCIE_PS_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */

    	/*pAd->bPCIclkOffDisableTx = FALSE;*/
    	
	return TRUE;
}


VOID RT28xxPciMlmeRadioOn(RTMP_ADAPTER *pAd)
{    
	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF))
   		return;
    
    DBGPRINT(RT_DEBUG_TRACE,("%s===>\n", __FUNCTION__));   

#if defined(MT76x0) || defined(MT76x2)
	if (IS_MT76x0(pAd) || IS_MT76x2(pAd))
	{
		MT76xx_PciMlmeRadioOn(pAd);
		return;
	}
#endif /* defined(MT76x0) || defined(MT76x2) */
 
    if ((pAd->OpMode == OPMODE_AP) ||
        ((pAd->OpMode == OPMODE_STA) 
#ifdef CONFIG_STA_SUPPORT
#ifdef PCIE_PS_SUPPORT
        && (!OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_ADVANCE_POWER_SAVE_PCIE_DEVICE)
        ||pAd->StaCfg.PSControl.field.EnableNewPS == FALSE)
 #endif /* PCIE_PS_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */
        ))
    {
    	RTMPRingCleanUp(pAd, QID_AC_BK);
    	RTMPRingCleanUp(pAd, QID_AC_BE);
    	RTMPRingCleanUp(pAd, QID_AC_VI);
    	RTMPRingCleanUp(pAd, QID_AC_VO);
    	RTMPRingCleanUp(pAd, QID_HCCA);
    	RTMPRingCleanUp(pAd, QID_MGMT);
    	RTMPRingCleanUp(pAd, QID_RX);

	/* Clear Radio off flag (need before call AsicSwitchChannel()) */
	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF);

#ifdef RTMP_RBUS_SUPPORT
		if (pAd->infType == RTMP_DEV_INF_RBUS)
			NICResetFromError(pAd);
#endif /* RTMP_RBUS_SUPPORT */
#ifdef RTMP_PCI_SUPPORT
#ifdef RT65xx
		if (IS_RT8592(pAd))
		{
#ifdef RT8592
			if (IS_RT8592(pAd)) {
				RTMP_ASIC_INTERRUPT_ENABLE(pAd);
			}
#endif /* RT8592 */
#ifdef DOT11_VHT_AC
			if (pAd->CommonCfg.BBPCurrentBW == BW_80)
				pAd->hw_cfg.cent_ch = pAd->CommonCfg.vht_cent_ch;
			else
#endif /* DOT11_VHT_AC */
				pAd->hw_cfg.cent_ch = pAd->CommonCfg.CentralChannel;
			AsicSwitchChannel(pAd, pAd->hw_cfg.cent_ch, FALSE);
			AsicLockChannel(pAd, pAd->hw_cfg.cent_ch);
		}
		else
#endif /* RT65xx */
		{
			if (pAd->infType == RTMP_DEV_INF_PCI || pAd->infType == RTMP_DEV_INF_PCIE)
				RT28xxPciAsicRadioOn(pAd, GUI_IDLE_POWER_SAVE);
		}
#endif /* RTMP_PCI_SUPPORT */

    	/* Enable Tx/Rx*/
    	RTMPEnableRxTx(pAd);
    	
    	/* Clear Radio off flag*/
    	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF);
	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF);

#ifdef LED_CONTROL_SUPPORT
	    /* Set LED*/
	    RTMPSetLED(pAd, LED_RADIO_ON);
#ifdef CONFIG_AP_SUPPORT
		/* The LEN_RADIO_ON indicates "Radio on but link down", 
		   so AP shall set LED LINK_UP status */
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd){
	    	RTMPSetLED(pAd, LED_LINK_UP);
		}
#endif /* CONFIG_AP_SUPPORT */
#endif /* LED_CONTROL_SUPPORT */
    }

#ifdef CONFIG_STA_SUPPORT
#ifdef PCIE_PS_SUPPORT

    if ((pAd->OpMode == OPMODE_STA) &&
        (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_ADVANCE_POWER_SAVE_PCIE_DEVICE))
        &&(pAd->StaCfg.PSControl.field.EnableNewPS == TRUE))
    {
        BOOLEAN Cancelled;
    	
    	RTMPPCIeLinkCtrlValueRestore(pAd, RESTORE_WAKEUP);

        pAd->Mlme.bPsPollTimerRunning = FALSE;
    	RTMPCancelTimer(&pAd->Mlme.PsPollTimer,	&Cancelled);
    	RTMPCancelTimer(&pAd->Mlme.RadioOnOffTimer,	&Cancelled);
    	RTMPSetTimer(&pAd->Mlme.RadioOnOffTimer, 40);
    }
#endif /* PCIE_PS_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */
}


VOID RT28xxPciMlmeRadioOFF(RTMP_ADAPTER *pAd)
{
    if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF))
    	return;
    
    DBGPRINT(RT_DEBUG_TRACE,("%s===>\n", __FUNCTION__));

#if defined(MT76x0) || defined(MT76x2)
	if (IS_MT76x0(pAd) || IS_MT76x2(pAd))
	{
		MT76xx_PciMlmeRadioOFF(pAd);
		return;
	}
#endif /* MT76x0 */

	/* Set Radio off flag*/
	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF);
#ifdef BB_SOC
	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_POLL_IDLE);
#endif /* BB_SOC */

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
    {
    	BOOLEAN Cancelled;
    	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))
    	{
			RTMPCancelTimer(&pAd->MlmeAux.ScanTimer, &Cancelled);
			RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS);
    	}

#ifdef RTMP_PCI_SUPPORT
#ifdef PCIE_PS_SUPPORT
		/* If during power safe mode. */
		if ((pAd->StaCfg.bRadio == TRUE)
			&& (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_ADVANCE_POWER_SAVE_PCIE_DEVICE)&&pAd->StaCfg.PSControl.field.EnableNewPS == TRUE))
		{
			DBGPRINT(RT_DEBUG_TRACE,("-->MlmeRadioOff() return on bRadio == TRUE; \n"));
			return;
		}
#endif /* PCIE_PS_SUPPORT */

		/* Always radio on since the NIC needs to set the MCU command (LED_RADIO_OFF).*/
		if ((pAd->infType == RTMP_DEV_INF_PCI)  || 
			(pAd->infType == RTMP_DEV_INF_PCIE))
		{
			if (IDLE_ON(pAd) &&
				(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF)))
			{
				RT28xxPciAsicRadioOn(pAd, GUI_IDLE_POWER_SAVE);
			}
		}

#ifdef PCIE_PS_SUPPORT
		if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_ADVANCE_POWER_SAVE_PCIE_DEVICE)&&pAd->StaCfg.PSControl.field.EnableNewPS == TRUE)
        {
            BOOLEAN Cancelled;
            pAd->Mlme.bPsPollTimerRunning = FALSE;
            RTMPCancelTimer(&pAd->Mlme.PsPollTimer,	&Cancelled);
	        RTMPCancelTimer(&pAd->Mlme.RadioOnOffTimer,	&Cancelled);
        }
#endif /* PCIE_PS_SUPPORT */
#endif /* RTMP_PCI_SUPPORT */

        /* Link down first if any association exists	*/
        if (INFRA_ON(pAd) || ADHOC_ON(pAd))		
		LinkDown(pAd, FALSE);   

        RtmpusecDelay(10000);

        /*==========================================    */
        /* Clean up old bss table   */
        BssTableInit(&pAd->ScanTab);
    }
#endif /* CONFIG_STA_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
#ifdef AP_SCAN_SUPPORT
	{
		BOOLEAN Cancelled;
		RTMPCancelTimer(&pAd->MlmeAux.APScanTimer, &Cancelled);
	}
#endif /* AP_SCAN_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

#ifdef LED_CONTROL_SUPPORT
	RTMPSetLED(pAd, LED_RADIO_OFF);
#endif /* LED_CONTROL_SUPPORT */

#ifdef RTMP_PCI_SUPPORT
#ifdef CONFIG_STA_SUPPORT
#ifdef PCIE_PS_SUPPORT
/*Q:Does all PCIe devices need to use timer to execute radio off function? or only if the device is PCIe and EnableNewPS is true ?*/
/*A:It is right, because only when the PCIe and EnableNewPs is true, we need to delay the RadioOffTimer*/
/*to avoid the deadlock with PCIe Power saving function. */
	if (pAd->OpMode == OPMODE_STA&&
		OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_ADVANCE_POWER_SAVE_PCIE_DEVICE)&&
		pAd->StaCfg.PSControl.field.EnableNewPS == TRUE)
	{
		RTMPSetTimer(&pAd->Mlme.RadioOnOffTimer, 10); 
	} 
	else
#endif /* PCIE_PS_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */
	if (pAd->infType == RTMP_DEV_INF_PCI || pAd->infType == RTMP_DEV_INF_PCIE)
	{
#ifdef RT8592
		if (IS_RT8592(pAd))
		{
			DISABLE_TX_RX(pAd, GUIRADIO_OFF);
			if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_ACTIVE))
			{
				RTMP_ASIC_INTERRUPT_DISABLE(pAd);
			}
		}
		else
#endif /* RT8592 */
		{
			if (RT28xxPciAsicRadioOff(pAd, GUIRADIO_OFF, 0) ==FALSE)
			{
				DBGPRINT(RT_DEBUG_ERROR,("%s call RT28xxPciAsicRadioOff fail !!\n", __FUNCTION__)); 
			}
		}
	}
#endif /* RTMP_PCI_SUPPORT */

#ifdef RTMP_RBUS_SUPPORT
	if (pAd->infType == RTMP_DEV_INF_RBUS)
	{
		WPDMA_GLO_CFG_STRUC GloCfg;

		/* Disable Tx/Rx DMA*/
		RTMP_IO_READ32(pAd, WPDMA_GLO_CFG, &GloCfg.word);	   /* disable DMA */
		GloCfg.field.EnableTxDMA = 0;
		GloCfg.field.EnableRxDMA = 0;
		RTMP_IO_WRITE32(pAd, WPDMA_GLO_CFG, GloCfg.word);	   /* abort all TX rings*/

		
		/* MAC_SYS_CTRL => value = 0x0 => 40mA*/
		RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, 0);
		
		/* PWR_PIN_CFG => value = 0x0 => 40mA*/
		RTMP_IO_WRITE32(pAd, PWR_PIN_CFG, 0);
		
		/* TX_PIN_CFG => value = 0x0 => 20mA*/
		RTMP_IO_WRITE32(pAd, TX_PIN_CFG, 0x00010000);

		// TODO: shiang-6590, here we need to make sure the CentralChannel is the same as Channel.
		if (pAd->CommonCfg.CentralChannel)
			AsicTurnOffRFClk(pAd, pAd->CommonCfg.CentralChannel);
		else
			AsicTurnOffRFClk(pAd, pAd->CommonCfg.Channel);

		/* Waiting for DMA idle*/
		AsicWaitPDMAIdle(pAd, 100, 1000);
	}
#endif /* RTMP_RBUS_SUPPORT */
}


/*
========================================================================
Routine Description:
	Get a pci map buffer.

Arguments:
	pAd				- WLAN control block pointer
	*ptr			- Virtual address or TX control block
	size			- buffer size
	sd_idx			- 1: the ptr is TX control block
	direction		- RTMP_PCI_DMA_TODEVICE or RTMP_PCI_DMA_FROMDEVICE

Return Value:
	the PCI map buffer

Note:
========================================================================
*/
ra_dma_addr_t RtmpDrvPciMapSingle(
	IN RTMP_ADAPTER *pAd,
	IN VOID *ptr,
	IN size_t size,
	IN INT sd_idx,
	IN INT direction)
{
	if (sd_idx == 1)
	{
		TX_BLK *pTxBlk = (TX_BLK *)(ptr);
		return (pTxBlk->SrcBufLen) ? linux_pci_map_single(((POS_COOKIE)(pAd->OS_Cookie))->pci_dev,
					pTxBlk->pSrcBufData, pTxBlk->SrcBufLen, 0, direction):(ra_dma_addr_t)NULL;
	}
	else
		return linux_pci_map_single(((POS_COOKIE)(pAd->OS_Cookie))->pci_dev,
					ptr, size, 0, direction);
}


int write_reg(RTMP_ADAPTER *ad, UINT32 base, UINT16 offset, UINT32 value)
{
	if (base == 0x40)
		RTMP_IO_WRITE32(ad, 0x10000 + offset, value);
	else if (base == 0x41)
		RTMP_IO_WRITE32(ad, offset, value);
	else
		DBGPRINT(RT_DEBUG_OFF, ("illegal base = %x\n", base));

	return 0;
}


int read_reg(RTMP_ADAPTER *ad, UINT32 base, UINT16 offset, UINT32 *value)
{
	if (base == 0x40) {
		RTMP_IO_READ32(ad, 0x10000 + offset, value);
	} else if (base == 0x41) {
		RTMP_IO_READ32(ad, offset, value);
	} else {
		DBGPRINT(RT_DEBUG_OFF, ("illegal base = %x\n", base));
	}
	return 0;
}

INT rtmp_irq_init(RTMP_ADAPTER *pAd)
{
	unsigned long _irqFlags;
	UINT32 reg_mask = 0;

#ifdef RLT_MAC
	if (pAd->chipCap.hif_type == HIF_RLT) {
		if(IS_MT76x2(pAd))
			reg_mask = (RLT_DELAYINTMASK) |(RLT_RxINT|RLT_76x2TxDataInt|RLT_TxMgmtInt);
		else
			reg_mask = (RLT_DELAYINTMASK) |(RLT_RxINT|RLT_TxDataInt|RLT_TxMgmtInt);
	}
#endif /* RLT_MAC */

#ifdef RTMP_MAC
	if (pAd->chipCap.hif_type == HIF_RTMP)
		reg_mask = ((RTMP_DELAYINTMASK) |(RTMP_RxINT|RTMP_TxDataInt|RTMP_TxMgmtInt)) & ~(0x03);
#endif /* RTMP_MAC */

	RTMP_INT_LOCK(&pAd->irq_lock, _irqFlags);
	pAd->int_enable_reg = reg_mask;
	pAd->int_disable_mask = 0;
	pAd->int_pending = 0;
	RTMP_INT_UNLOCK(&pAd->irq_lock, _irqFlags);

	return 0;
}

#endif /* RTMP_MAC_PCI */


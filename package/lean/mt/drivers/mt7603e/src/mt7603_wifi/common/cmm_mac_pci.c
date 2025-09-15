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

	rtmp_tx_swq_exit(pAd, WCID_ALL);

	/* Free Tx Ring Packet*/
	for (index=0;index< NUM_OF_TX_RING;index++)
	{
		pTxRing = &pAd->TxRing[index];
		for (j=0; j< TX_RING_SIZE; j++)
		{
			dma_cb = &pTxRing->Cell[j];
#ifdef RT_BIG_ENDIAN
			pDestTxD = (TXD_STRUC *)(dma_cb->AllocVa);
			NdisMoveMemory(&tx_hw_info[0], (UCHAR *)pDestTxD, TXD_SIZE);
			pTxD = (TXD_STRUC *)&tx_hw_info[0];
			RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#else
			pTxD = (TXD_STRUC *) (dma_cb->AllocVa);
#endif /* RT_BIG_ENDIAN */
			pPacket = dma_cb->pNdisPacket;

			if (pPacket)
			{
				PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr0, pTxD->SDLen0, RTMP_PCI_DMA_TODEVICE);
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
			}
			/*Always assign pNdisPacket as NULL after clear*/
			dma_cb->pNdisPacket = NULL;
					
			pPacket = dma_cb->pNextNdisPacket;
			if (pPacket)
			{
				PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr1, pTxD->SDLen1, RTMP_PCI_DMA_TODEVICE);
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
			}
			/*Always assign pNextNdisPacket as NULL after clear*/
			dma_cb->pNextNdisPacket = NULL;

#ifdef RT_BIG_ENDIAN
			RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
			WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, FALSE, TYPE_TXD);
#endif /* RT_BIG_ENDIAN */
		}
	}

#ifdef CONFIG_ANDES_SUPPORT
	{
		RTMP_CTRL_RING *pCtrlRing = &pAd->CtrlRing;
		
		RTMP_IO_READ32(pAd, pCtrlRing->hw_didx_addr, &pCtrlRing->TxDmaIdx);

		while (pCtrlRing->TxSwFreeIdx!= pCtrlRing->TxCpuIdx)
		{
#ifdef RT_BIG_ENDIAN
	        pDestTxD = (TXD_STRUC *) (pCtrlRing->Cell[pCtrlRing->TxSwFreeIdx].AllocVa);
			NdisMoveMemory(&tx_hw_info[0], (UCHAR *)pDestTxD, TXD_SIZE);
			pTxD = (TXD_STRUC *)&tx_hw_info[0];
			RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#else
			pTxD = (TXD_STRUC *) (pCtrlRing->Cell[pCtrlRing->TxSwFreeIdx].AllocVa);
#endif
			pTxD->DMADONE = 0;
			pPacket = pCtrlRing->Cell[pCtrlRing->TxSwFreeIdx].pNdisPacket;

			if (pPacket == NULL)
			{
				INC_RING_INDEX(pCtrlRing->TxSwFreeIdx, MGMT_RING_SIZE);
				continue;
			}

			if (pPacket)
			{
				PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr0, pTxD->SDLen0, RTMP_PCI_DMA_TODEVICE);
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
			}
			pCtrlRing->Cell[pCtrlRing->TxSwFreeIdx].pNdisPacket = NULL;

			pPacket = pCtrlRing->Cell[pCtrlRing->TxSwFreeIdx].pNextNdisPacket;
			if (pPacket)
			{
				PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr1, pTxD->SDLen1, RTMP_PCI_DMA_TODEVICE);
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
			}
			pCtrlRing->Cell[pCtrlRing->TxSwFreeIdx].pNextNdisPacket = NULL;
			INC_RING_INDEX(pCtrlRing->TxSwFreeIdx, MGMT_RING_SIZE);

#ifdef RT_BIG_ENDIAN
	        RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
	        WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, TRUE, TYPE_TXD);
#endif
		}
	}
#endif /* CONFIG_ANDES_SUPPORT */

#ifdef MT_MAC
	if (1 /*pAd->chipCap.hif_type == HIF_MT*/)
	{
		RTMP_BCN_RING *pBcnRing = &pAd->BcnRing;
		
		RTMP_IO_READ32(pAd, pBcnRing->hw_didx_addr, &pBcnRing->TxDmaIdx);

		while (pBcnRing->TxSwFreeIdx!= pBcnRing->TxDmaIdx)
		{
#ifdef RT_BIG_ENDIAN
			pDestTxD = (TXD_STRUC *) (pBcnRing->Cell[pBcnRing->TxSwFreeIdx].AllocVa);
			NdisMoveMemory(&tx_hw_info[0], (UCHAR *)pDestTxD, TXD_SIZE);
			pTxD = (TXD_STRUC *)&tx_hw_info[0];
			RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#else
			pTxD = (TXD_STRUC *) (pBcnRing->Cell[pBcnRing->TxSwFreeIdx].AllocVa);
#endif
			pTxD->DMADONE = 0;
			pPacket = pBcnRing->Cell[pBcnRing->TxSwFreeIdx].pNdisPacket;

			if (pPacket == NULL)
			{
				INC_RING_INDEX(pBcnRing->TxSwFreeIdx, BCN_RING_SIZE);
				continue;
			}

			if (pPacket)
			{
				PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr0, pTxD->SDLen0, RTMP_PCI_DMA_TODEVICE);
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
			}
			pBcnRing->Cell[pBcnRing->TxSwFreeIdx].pNdisPacket = NULL;

			pPacket = pBcnRing->Cell[pBcnRing->TxSwFreeIdx].pNextNdisPacket;
			if (pPacket)
			{
				PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr1, pTxD->SDLen1, RTMP_PCI_DMA_TODEVICE);
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
			}
			pBcnRing->Cell[pBcnRing->TxSwFreeIdx].pNextNdisPacket = NULL;
			INC_RING_INDEX(pBcnRing->TxSwFreeIdx, BCN_RING_SIZE);

#ifdef RT_BIG_ENDIAN
	        RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
	        WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, TRUE, TYPE_TXD);
#endif
		}
	}
#endif /* MT_MAC */

	for (index=0;index< NUM_OF_RX_RING;index++)
	{
		for (j = RX_RING_SIZE - 1 ; j >= 0; j--)
		{
			dma_cb = &pAd->RxRing[index].Cell[j];
			if ((dma_cb->DmaBuf.AllocVa) && (dma_cb->pNdisPacket))
			{
				PCI_UNMAP_SINGLE(pAd, dma_cb->DmaBuf.AllocPa, dma_cb->DmaBuf.AllocSize, RTMP_PCI_DMA_FROMDEVICE);
				RELEASE_NDIS_PACKET(pAd, dma_cb->pNdisPacket, NDIS_STATUS_SUCCESS);
			}
		}
		NdisZeroMemory(pAd->RxRing[index].Cell, RX_RING_SIZE * sizeof(RTMP_DMACB));
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
		desc_ring_free(pAd, &pAd->RxDescRing[num]);

	desc_ring_free(pAd, &pAd->MgmtDescRing);
#ifdef CONFIG_ANDES_SUPPORT
	desc_ring_free(pAd, &pAd->CtrlDescRing);
#endif /* CONFIG_ANDES_SUPPORT */

#ifdef MT_MAC
	desc_ring_free(pAd, &pAd->BcnDescRing);
	
	if (pAd->TxBmcBufSpace.AllocVa)
	{
		RTMP_FreeFirstTxBuffer(pci_dev, 
								pAd->TxBmcBufSpace.AllocSize, 
								FALSE, pAd->TxBmcBufSpace.AllocVa, 
								pAd->TxBmcBufSpace.AllocPa);
	}
	NdisZeroMemory(&pAd->TxBmcBufSpace, sizeof(RTMP_DMABUF));

	desc_ring_free(pAd, &pAd->TxBmcDescRing);
#endif /* MT_MAC */

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

		desc_ring_free(pAd, &pAd->TxDescRing[num]);
	}

	DBGPRINT(RT_DEBUG_TRACE, ("<-- RTMPFreeTxRxRingMemory\n"));
}


NDIS_STATUS RTMPInitTxRxRingMemory(RTMP_ADAPTER *pAd)
{
	INT num, index;
	ULONG /*RingBasePaHigh,*/ RingBasePaLow;
	VOID *RingBaseVa;
	RTMP_TX_RING *pTxRing;
	RTMP_RX_RING *pRxRing;
	RTMP_DMABUF *pDmaBuf, *pDescRing;
	RTMP_DMACB *dma_cb;
	PNDIS_PACKET pPacket;
	TXD_STRUC *pTxD;
	RXD_STRUC *pRxD;
	//ULONG ErrorValue = 0;
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
		ULONG /*BufBasePaHigh,*/ BufBasePaLow;
		VOID *BufBaseVa;
		
		/* memory zero the  Tx ring descriptor's memory */
		pDescRing = &pAd->TxDescRing[num];
		NdisZeroMemory(pDescRing->AllocVa, pDescRing->AllocSize);
		/* Save PA & VA for further operation*/
		//RingBasePaHigh = RTMP_GetPhysicalAddressHigh(pDescRing->AllocPa);
		RingBasePaLow = RTMP_GetPhysicalAddressLow (pDescRing->AllocPa);
		RingBaseVa = pDescRing->AllocVa;

		/* Zero init all 1st TXBuf's memory for this TxRing*/
		NdisZeroMemory(pAd->TxBufSpace[num].AllocVa, pAd->TxBufSpace[num].AllocSize);
		/* Save PA & VA for further operation */
		//BufBasePaHigh = RTMP_GetPhysicalAddressHigh(pAd->TxBufSpace[num].AllocPa);
		BufBasePaLow = RTMP_GetPhysicalAddressLow (pAd->TxBufSpace[num].AllocPa);
		BufBaseVa = pAd->TxBufSpace[num].AllocVa;

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
			RTMP_SetPhysicalAddressHigh(dma_cb->AllocPa, 0/*RingBasePaHigh*/);
			RTMP_SetPhysicalAddressLow (dma_cb->AllocPa, RingBasePaLow);

			/* Setup Tx Buffer size & address. only 802.11 header will store in this space */
			pDmaBuf = &dma_cb->DmaBuf;
			pDmaBuf->AllocSize = TX_DMA_1ST_BUFFER_SIZE;
			pDmaBuf->AllocVa = BufBaseVa;
			RTMP_SetPhysicalAddressHigh(pDmaBuf->AllocPa,0 /*BufBasePaHigh*/);
			RTMP_SetPhysicalAddressLow(pDmaBuf->AllocPa, BufBasePaLow);

			/* link the pre-allocated TxBuf to TXD */
			pTxD = (TXD_STRUC *)dma_cb->AllocVa;
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
	//RingBasePaHigh = RTMP_GetPhysicalAddressHigh(pDescRing->AllocPa);
	RingBasePaLow = RTMP_GetPhysicalAddressLow (pDescRing->AllocPa);
	RingBaseVa = pDescRing->AllocVa;
	NdisZeroMemory(pDescRing->AllocVa, pDescRing->AllocSize);
	for (index = 0; index < MGMT_RING_SIZE; index++)
	{
		dma_cb = &pAd->MgmtRing.Cell[index];
		dma_cb->pNdisPacket = NULL;
		dma_cb->pNextNdisPacket = NULL;
		/* Init MGMT Ring Size, Va, Pa variables */
		dma_cb->AllocSize = TXD_SIZE;
		dma_cb->AllocVa = RingBaseVa;
		RTMP_SetPhysicalAddressHigh(dma_cb->AllocPa, 0/*RingBasePaHigh*/);
		RTMP_SetPhysicalAddressLow (dma_cb->AllocPa, RingBasePaLow);

		/* Offset to next ring descriptor address */
		RingBasePaLow += TXD_SIZE;
		RingBaseVa = (PUCHAR) RingBaseVa + TXD_SIZE;

		/* link the pre-allocated TxBuf to TXD */
		pTxD = (TXD_STRUC *)dma_cb->AllocVa;
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
	//RingBasePaHigh = RTMP_GetPhysicalAddressHigh(pDescRing->AllocPa);
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
		RTMP_SetPhysicalAddressHigh(dma_cb->AllocPa, 0/*RingBasePaHigh*/);
		RTMP_SetPhysicalAddressLow (dma_cb->AllocPa, RingBasePaLow);

		/* Offset to next ring descriptor address */
		RingBasePaLow += TXD_SIZE;
		RingBaseVa = (PUCHAR) RingBaseVa + TXD_SIZE;

		/* link the pre-allocated TxBuf to TXD */
		pTxD = (TXD_STRUC *)dma_cb->AllocVa;
		pTxD->DMADONE = 1;

#ifdef RT_BIG_ENDIAN
		RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#endif

		/* flush dcache if no consistent memory is supported */
		RTMP_DCACHE_FLUSH(pTxD, dma_cb->AllocSize);

		/* no pre-allocated buffer required in CtrlRing for scatter-gather case */
	}
#endif /* CONFIG_ANDES_SUPPORT */


#ifdef MT_MAC
	if (1 /*pAd->chipCap.hif_type == HIF_MT*/) {
		/* Initialize CTRL Ring and associated buffer memory */
		ULONG /*BufBasePaHigh,*/ BufBasePaLow;
		VOID *BufBaseVa;

		pDescRing = &pAd->BcnDescRing;
		//RingBasePaHigh = RTMP_GetPhysicalAddressHigh(pDescRing->AllocPa);
		RingBasePaLow = RTMP_GetPhysicalAddressLow (pDescRing->AllocPa);
		RingBaseVa = pDescRing->AllocVa;
		NdisZeroMemory(pDescRing->AllocVa, pDescRing->AllocSize);
		DBGPRINT(RT_DEBUG_OFF,  ("TX_BCN DESC %p size = %ld\n", 
					pDescRing->AllocVa, pDescRing->AllocSize));
		for (index = 0; index < BCN_RING_SIZE; index++)
		{
			dma_cb = &pAd->BcnRing.Cell[index];
			dma_cb->pNdisPacket = NULL;
			dma_cb->pNextNdisPacket = NULL;
			/* Init Ctrl Ring Size, Va, Pa variables */
			dma_cb->AllocSize = TXD_SIZE;
			dma_cb->AllocVa = RingBaseVa;
			RTMP_SetPhysicalAddressHigh(dma_cb->AllocPa, 0/*RingBasePaHigh*/);
			RTMP_SetPhysicalAddressLow (dma_cb->AllocPa, RingBasePaLow);

			/* Offset to next ring descriptor address */
			RingBasePaLow += TXD_SIZE;
			RingBaseVa = (PUCHAR) RingBaseVa + TXD_SIZE;

			/* link the pre-allocated TxBuf to TXD */
			pTxD = (TXD_STRUC *)dma_cb->AllocVa;
			pTxD->DMADONE = 1;

#ifdef RT_BIG_ENDIAN
			RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#endif

			/* flush dcache if no consistent memory is supported */
			RTMP_DCACHE_FLUSH(pTxD, dma_cb->AllocSize);

			/* no pre-allocated buffer required in CtrlRing for scatter-gather case */
		}
		
		/* memory zero the  Tx BMC ring descriptor's memory */
		pDescRing = &pAd->TxBmcDescRing;
		NdisZeroMemory(pDescRing->AllocVa, pDescRing->AllocSize);
		/* Save PA & VA for further operation*/
		//RingBasePaHigh = RTMP_GetPhysicalAddressHigh(pDescRing->AllocPa);
		RingBasePaLow = RTMP_GetPhysicalAddressLow (pDescRing->AllocPa);
		RingBaseVa = pDescRing->AllocVa;

		/* Zero init all 1st TXBuf's memory for this TxRing*/
		NdisZeroMemory(pAd->TxBmcBufSpace.AllocVa, pAd->TxBmcBufSpace.AllocSize);
		/* Save PA & VA for further operation */
		//BufBasePaHigh = RTMP_GetPhysicalAddressHigh(pAd->TxBmcBufSpace.AllocPa);
		BufBasePaLow = RTMP_GetPhysicalAddressLow (pAd->TxBmcBufSpace.AllocPa);
		BufBaseVa = pAd->TxBmcBufSpace.AllocVa;

		/* linking Tx Ring Descriptor and associated buffer memory */
		pTxRing = &pAd->TxBmcRing;
		for (index = 0; index < TX_RING_SIZE; index++)
		{
			dma_cb = &pTxRing->Cell[index];
			dma_cb->pNdisPacket = NULL;
			dma_cb->pNextNdisPacket = NULL;
			/* Init Tx Ring Size, Va, Pa variables*/
			dma_cb->AllocSize = TXD_SIZE;
			dma_cb->AllocVa = RingBaseVa;
			RTMP_SetPhysicalAddressHigh(dma_cb->AllocPa, 0/*RingBasePaHigh*/);
			RTMP_SetPhysicalAddressLow (dma_cb->AllocPa, RingBasePaLow);

			/* Setup Tx Buffer size & address. only 802.11 header will store in this space */
			pDmaBuf = &dma_cb->DmaBuf;
			pDmaBuf->AllocSize = TX_DMA_1ST_BUFFER_SIZE;
			pDmaBuf->AllocVa = BufBaseVa;
			RTMP_SetPhysicalAddressHigh(pDmaBuf->AllocPa,0 /*BufBasePaHigh*/);
			RTMP_SetPhysicalAddressLow(pDmaBuf->AllocPa, BufBasePaLow);

			/* link the pre-allocated TxBuf to TXD */
			pTxD = (TXD_STRUC *)dma_cb->AllocVa;
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
	}
#endif /* MT_MAC */

	/* Initialize Rx Ring and associated buffer memory */
	for (num = 0; num < NUM_OF_RX_RING; num++)
	{
		pDescRing = &pAd->RxDescRing[num];
		pRxRing = &pAd->RxRing[num];

		NdisZeroMemory(pDescRing->AllocVa, pDescRing->AllocSize);
		DBGPRINT(RT_DEBUG_OFF,  ("RX[%d] DESC %p size = %ld\n", 
					num, pDescRing->AllocVa, pDescRing->AllocSize));

		/* Save PA & VA for further operation */
		//RingBasePaHigh = RTMP_GetPhysicalAddressHigh(pDescRing->AllocPa);
		RingBasePaLow = RTMP_GetPhysicalAddressLow (pDescRing->AllocPa);
		RingBaseVa = pDescRing->AllocVa;
	
		/* Linking Rx Ring and associated buffer memory */
		for (index = 0; index < RX_RING_SIZE; index++)
		{
			dma_cb = &pRxRing->Cell[index];
			/* Init RX Ring Size, Va, Pa variables*/
			dma_cb->AllocSize = RXD_SIZE;
			dma_cb->AllocVa = RingBaseVa;
			RTMP_SetPhysicalAddressHigh(dma_cb->AllocPa, 0/*RingBasePaHigh*/);
			RTMP_SetPhysicalAddressLow (dma_cb->AllocPa, RingBasePaLow);;

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
				//ErrorValue = ERRLOG_OUT_OF_SHARED_MEMORY;
				DBGPRINT_ERR(("Failed to allocate RxRing's 1st buffer\n"));
				Status = NDIS_STATUS_RESOURCES;
				break;
			}

			/* Zero init this memory block */
			//NdisZeroMemory(pDmaBuf->AllocVa, pDmaBuf->AllocSize);

			/* Write RxD buffer address & allocated buffer length */
			pRxD = (RXD_STRUC *)dma_cb->AllocVa;
			pRxD->SDP0 = RTMP_GetPhysicalAddressLow(pDmaBuf->AllocPa);
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
	rtmp_tx_swq_init(pAd);
	for(index = 0; index < NUM_OF_TX_RING; index++)
	{
		/* Init TX rings index pointer */
		pAd->TxRing[index].TxSwFreeIdx = 0;
		pAd->TxRing[index].TxCpuIdx = 0;
	}

	/* Init RX Ring index pointer */
	for (index = 0; index < NUM_OF_RX_RING; index++) {
		pAd->RxRing[index].RxSwReadIdx = 0;
		pAd->RxRing[index].RxCpuIdx = RX_RING_SIZE - 1;
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
			desc_ring_alloc(pAd, &pAd->TxDescRing[num], TX_RING_SIZE * TXD_SIZE);
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
				DBGPRINT_ERR(("Failed to allocate a big buffer\n"));
				Status = NDIS_STATUS_RESOURCES;
				break;
			}
		}
		if (Status == NDIS_STATUS_RESOURCES)
			break;

		
		/* Alloc MGMT ring desc buffer except Tx ring allocated eariler */
		desc_ring_alloc(pAd, &pAd->MgmtDescRing, MGMT_RING_SIZE * TXD_SIZE);
		if (pAd->MgmtDescRing.AllocVa == NULL) {
			Status = NDIS_STATUS_RESOURCES;
			break;
		}
		DBGPRINT(RT_DEBUG_TRACE, ("MGMT Ring: total %d bytes allocated\n",
					(INT)pAd->MgmtDescRing.AllocSize));

#ifdef CONFIG_ANDES_SUPPORT
		/* Alloc CTRL ring desc buffer except Tx ring allocated eariler */
		desc_ring_alloc(pAd, &pAd->CtrlDescRing, MGMT_RING_SIZE * TXD_SIZE);
		if (pAd->CtrlDescRing.AllocVa == NULL) {
			Status = NDIS_STATUS_RESOURCES;
			break;
		}
		DBGPRINT(RT_DEBUG_TRACE, ("CTRL Ring: total %d bytes allocated\n",
					(INT)pAd->CtrlDescRing.AllocSize));
#endif /* CONFIG_ANDES_SUPPORT */

#ifdef MT_MAC
		if (1 /*pAd->chipCap.hif_type == HIF_MT*/) {
			/* Alloc Beacon ring desc buffer */
			desc_ring_alloc(pAd, &pAd->BcnDescRing, BCN_RING_SIZE * TXD_SIZE);
			if (pAd->BcnDescRing.AllocVa == NULL) {
				Status = NDIS_STATUS_RESOURCES;
				break;
			}
			DBGPRINT(RT_DEBUG_TRACE, ("Beacon Ring: total %d bytes allocated\n",
						(INT)pAd->BcnDescRing.AllocSize));
						
			/* Allocate Tx ring descriptor's memory (BMC)*/
			desc_ring_alloc(pAd, &pAd->TxBmcDescRing, TX_RING_SIZE * TXD_SIZE);
			if (pAd->TxBmcDescRing.AllocVa == NULL) {
				Status = NDIS_STATUS_RESOURCES;
				break;
			}
			DBGPRINT(RT_DEBUG_TRACE, ("TxBmcRing: total %d bytes allocated\n",
						(INT)pAd->TxBmcDescRing.AllocSize));
			
			/* Allocate all 1st TXBuf's memory for this TxRing */
			pAd->TxBmcBufSpace.AllocSize = TX_RING_SIZE * TX_DMA_1ST_BUFFER_SIZE;
			RTMP_AllocateFirstTxBuffer(
				pci_dev,
				0,
				pAd->TxBmcBufSpace.AllocSize,
				FALSE,
				&pAd->TxBmcBufSpace.AllocVa,
				&pAd->TxBmcBufSpace.AllocPa);

			if (pAd->TxBmcBufSpace.AllocVa == NULL)
			{
				ErrorValue = ERRLOG_OUT_OF_SHARED_MEMORY;
				DBGPRINT_ERR(("Failed to allocate a big buffer\n"));
				Status = NDIS_STATUS_RESOURCES;
				break;
			}
		
			if (Status == NDIS_STATUS_RESOURCES)
				break;			
		}
#endif /* MT_MAC */

		/* Alloc RX ring desc memory except Tx ring allocated eariler */
		for (num = 0; num < NUM_OF_RX_RING; num++) {
			desc_ring_alloc(pAd, &pAd->RxDescRing[num],
								RX_RING_SIZE * RXD_SIZE);
			if (pAd->RxDescRing[num].AllocVa == NULL) {
				Status = NDIS_STATUS_RESOURCES;
				break;
			}
			DBGPRINT(RT_DEBUG_TRACE, ("Rx[%d] Ring: total %d bytes allocated\n",
						num, (INT)pAd->RxDescRing[num].AllocSize));
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

	DBGPRINT_S(("<-- RTMPAllocTxRxRingMemory, Status=%x, ErrorValue=%lux\n", Status,ErrorValue));

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
				DBGPRINT_ERR(("Failed to allocate a big buffer\n"));
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
				pTxD = (TXD_STRUC *)dma_cb->AllocVa;
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
			pTxD = (TXD_STRUC *)dma_cb->AllocVa;
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
			pTxD = (TXD_STRUC *)dma_cb->AllocVa;
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


#ifdef MT_MAC
		if (1/* pAd->chipCap.hif_type == HIF_MT */) {
			/* Allocate Beacon ring descriptor's memory */
			ULONG  BufBasePaHigh;
			ULONG  BufBasePaLow;
			PVOID  BufBaseVa;
			
			desc_ring_alloc(pAd, &pAd->BcnDescRing, BCN_RING_SIZE * TXD_SIZE);
			if (pAd->BcnDescRing.AllocVa == NULL) {
				Status = NDIS_STATUS_RESOURCES;
				break;
			}
			DBGPRINT(RT_DEBUG_TRACE, ("BcnDescRing[%p]: total %d bytes allocated\n",
					pAd->BcnDescRing.AllocVa, (INT)pAd->BcnDescRing.AllocSize));

			/* Save PA & VA for further operation*/
			RingBasePaHigh = RTMP_GetPhysicalAddressHigh(pAd->BcnDescRing.AllocPa);
			RingBasePaLow = RTMP_GetPhysicalAddressLow (pAd->BcnDescRing.AllocPa);
			RingBaseVa = pAd->BcnDescRing.AllocVa;
			
			/* Initialize Beacon Ring and associated buffer memory */
			for (index = 0; index < BCN_RING_SIZE; index++)
			{
				dma_cb = &pAd->BcnRing.Cell[index];
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
				pTxD = (TXD_STRUC *)dma_cb->AllocVa;
				pTxD->DMADONE = 1;

#ifdef RT_BIG_ENDIAN
				RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#endif

				/* flush dcache if no consistent memory is supported */
				RTMP_DCACHE_FLUSH(pTxD, dma_cb->AllocSize);

				/* no pre-allocated buffer required in CtrlRing for scatter-gather case*/
			}
			DBGPRINT(RT_DEBUG_TRACE, ("Bcn Ring: total %d entry allocated\n", index));
			
			/* 
				Allocate Tx ring descriptor's memory (BMC)
			*/
			desc_ring_alloc(pAd, &pAd->TxBmcDescRing, TX_RING_SIZE * TXD_SIZE);
			if (pAd->TxBmcDescRing.AllocVa == NULL) {
				Status = NDIS_STATUS_RESOURCES;
				break;
			}

			pDmaBuf = &pAd->TxBmcDescRing;
			DBGPRINT(RT_DEBUG_TRACE, ("TxBmcDescRing[%p]: total %d bytes allocated\n",
					pDmaBuf->AllocVa, (INT)pDmaBuf->AllocSize));

			/* Save PA & VA for further operation*/
			RingBasePaHigh = RTMP_GetPhysicalAddressHigh(pDmaBuf->AllocPa);
			RingBasePaLow = RTMP_GetPhysicalAddressLow(pDmaBuf->AllocPa);
			RingBaseVa = pDmaBuf->AllocVa;

			/*
				Allocate all 1st TXBuf's memory for this TxRing
			*/
			pAd->TxBmcBufSpace.AllocSize = TX_RING_SIZE * TX_DMA_1ST_BUFFER_SIZE;
			RTMP_AllocateFirstTxBuffer(
				((POS_COOKIE)(pAd->OS_Cookie))->pci_dev,
				0,
				pAd->TxBmcBufSpace.AllocSize,
				FALSE,
				&pAd->TxBmcBufSpace.AllocVa,
				&pAd->TxBmcBufSpace.AllocPa);

			if (pAd->TxBmcBufSpace.AllocVa == NULL)
			{
				ErrorValue = ERRLOG_OUT_OF_SHARED_MEMORY;
				DBGPRINT_ERR(("Failed to allocate a big buffer\n"));
				Status = NDIS_STATUS_RESOURCES;
				break;
			}

			/* Zero init this memory block*/
			NdisZeroMemory(pAd->TxBmcBufSpace.AllocVa, pAd->TxBmcBufSpace.AllocSize);

			/* Save PA & VA for further operation*/
			BufBasePaHigh = RTMP_GetPhysicalAddressHigh(pAd->TxBmcBufSpace.AllocPa);
			BufBasePaLow = RTMP_GetPhysicalAddressLow (pAd->TxBmcBufSpace.AllocPa);
			BufBaseVa = pAd->TxBmcBufSpace.AllocVa;

			/*
				Initialize Tx Ring Descriptor and associated buffer memory
			*/
			pTxRing = &pAd->TxBmcRing;
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
				pTxD = (TXD_STRUC *)dma_cb->AllocVa;
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
			
			if (Status == NDIS_STATUS_RESOURCES)
				break;
		}
#endif /* MT_MAC */

		for (num = 0; num < NUM_OF_RX_RING; num++)
		{
			/* Alloc RxRingDesc memory except Tx ring allocated eariler */
			desc_ring_alloc(pAd, &pAd->RxDescRing[num], RX_RING_SIZE * RXD_SIZE);
			if (pAd->RxDescRing[num].AllocVa == NULL) {
				Status = NDIS_STATUS_RESOURCES;
				break;
			}
			DBGPRINT(RT_DEBUG_OFF, ("RxDescRing[%p]: total %d bytes allocated\n",
						pAd->RxDescRing[num].AllocVa, (INT)pAd->RxDescRing[num].AllocSize));
		
			/* Initialize Rx Ring and associated buffer memory */
			RingBasePaHigh = RTMP_GetPhysicalAddressHigh(pAd->RxDescRing[num].AllocPa);
			RingBasePaLow = RTMP_GetPhysicalAddressLow (pAd->RxDescRing[num].AllocPa);
			RingBaseVa = pAd->RxDescRing[num].AllocVa;
			for (index = 0; index < RX_RING_SIZE; index++)
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
				pRxD = (RXD_STRUC *)dma_cb->AllocVa;
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
	rtmp_tx_swq_init(pAd);
	for(index = 0; index < NUM_OF_TX_RING; index++)
	{
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
		pAd->RxRing[index].RxSwReadIdx = 0;
		pAd->RxRing[index].RxCpuIdx = RX_RING_SIZE - 1;
	}

	pAd->PrivateInfo.TxRingFullCnt = 0;

	DBGPRINT_S(("<-- RTMPAllocTxRxRingMemory, Status=%x\n", Status));
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
	RTMP_DMACB *dma_cb;

	DBGPRINT(RT_DEBUG_TRACE, ("--> RTMPFreeTxRxRingMemory\n"));

	rtmp_tx_swq_exit(pAd, WCID_ALL);

	/* Free Tx Ring Packet*/
	for (index=0;index< NUM_OF_TX_RING;index++)
	{
		pTxRing = &pAd->TxRing[index];
		
		for (j=0; j< TX_RING_SIZE; j++)
		{	
#ifdef RT_BIG_ENDIAN
			pDestTxD  = (TXD_STRUC *) (pTxRing->Cell[j].AllocVa);
			NdisMoveMemory(&tx_hw_info[0], (UCHAR *)pDestTxD, TXD_SIZE);
			pTxD = (TXD_STRUC *)&tx_hw_info[0];
			RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#else
			pTxD = (TXD_STRUC *) (pTxRing->Cell[j].AllocVa);
#endif /* RT_BIG_ENDIAN */
			pPacket = pTxRing->Cell[j].pNdisPacket;

			if (pPacket)
			{
				PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr0, pTxD->SDLen0, RTMP_PCI_DMA_TODEVICE);
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
			}		
			/*Always assign pNdisPacket as NULL after clear*/
			pTxRing->Cell[j].pNdisPacket = NULL;
			pPacket = pTxRing->Cell[j].pNextNdisPacket;
			if (pPacket)
			{
				PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr1, pTxD->SDLen1, RTMP_PCI_DMA_TODEVICE);
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
			}
			/*Always assign pNextNdisPacket as NULL after clear*/
			pTxRing->Cell[pTxRing->TxSwFreeIdx].pNextNdisPacket = NULL;
#ifdef RT_BIG_ENDIAN
			RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
			WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, FALSE, TYPE_TXD);
#endif /* RT_BIG_ENDIAN */
		}
	}	

	{
		RTMP_MGMT_RING *pMgmtRing = &pAd->MgmtRing;
		NdisAcquireSpinLock(&pAd->MgmtRingLock);

		RTMP_IO_READ32(pAd, pMgmtRing->hw_didx_addr, &pMgmtRing->TxDmaIdx);
		while (pMgmtRing->TxSwFreeIdx!= pMgmtRing->TxDmaIdx)
		{
#ifdef RT_BIG_ENDIAN
	        pDestTxD = (TXD_STRUC *) (pMgmtRing->Cell[pAd->MgmtRing.TxSwFreeIdx].AllocVa);
			NdisMoveMemory(&tx_hw_info[0], (UCHAR *)pDestTxD, TXD_SIZE);
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
		RTMP_CTRL_RING *pCtrlRing = &pAd->CtrlRing;

		NdisAcquireSpinLock(&pAd->CtrlRingLock);
		RTMP_IO_READ32(pAd, pCtrlRing->hw_didx_addr, &pCtrlRing->TxDmaIdx);

		while (pCtrlRing->TxSwFreeIdx!= pCtrlRing->TxDmaIdx)
		{
#ifdef RT_BIG_ENDIAN
	        pDestTxD = (TXD_STRUC *) (pCtrlRing->Cell[pCtrlRing->TxSwFreeIdx].AllocVa);
			NdisMoveMemory(&tx_hw_info[0], (UCHAR *)pDestTxD, TXD_SIZE);
			pTxD = (TXD_STRUC *)&tx_hw_info[0];
			RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#else
			pTxD = (TXD_STRUC *) (pCtrlRing->Cell[pCtrlRing->TxSwFreeIdx].AllocVa);
#endif
			pTxD->DMADONE = 0;
			pPacket = pCtrlRing->Cell[pCtrlRing->TxSwFreeIdx].pNdisPacket;

			if (pPacket == NULL)
			{
				INC_RING_INDEX(pCtrlRing->TxSwFreeIdx, MGMT_RING_SIZE);
				continue;
			}

			if (pPacket)
			{
				PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr0, pTxD->SDLen0, RTMP_PCI_DMA_TODEVICE);
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
			}
			pCtrlRing->Cell[pCtrlRing->TxSwFreeIdx].pNdisPacket = NULL;

			pPacket = pCtrlRing->Cell[pCtrlRing->TxSwFreeIdx].pNextNdisPacket;
			if (pPacket)
			{
				PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr1, pTxD->SDLen1, RTMP_PCI_DMA_TODEVICE);
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
			}
			pCtrlRing->Cell[pCtrlRing->TxSwFreeIdx].pNextNdisPacket = NULL;
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
	if (1 /*pAd->chipCap.hif_type == HIF_MT*/)
	{
		RTMP_BCN_RING *pBcnRing = &pAd->BcnRing;
		RTMP_DMACB *dma_cell = &pAd->BcnRing.Cell[0];

        RTMP_SEM_LOCK(&pAd->BcnRingLock);
		//NdisAcquireSpinLock(&pAd->BcnRingLock);
		RTMP_IO_READ32(pAd, pBcnRing->hw_didx_addr, &pBcnRing->TxDmaIdx);

		while (pBcnRing->TxSwFreeIdx!= pBcnRing->TxDmaIdx)
		{
#ifdef RT_BIG_ENDIAN
			pDestTxD = (TXD_STRUC *) (dma_cell[pBcnRing->TxSwFreeIdx].AllocVa);
			NdisMoveMemory(&tx_hw_info[0], (UCHAR *)pDestTxD, TXD_SIZE);
			pTxD = (TXD_STRUC *)&tx_hw_info[0];
			RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#else
			pTxD = (TXD_STRUC *) (dma_cell[pBcnRing->TxSwFreeIdx].AllocVa);
#endif
			pTxD->DMADONE = 0;
			pPacket = dma_cell[pBcnRing->TxSwFreeIdx].pNdisPacket;

			if (pPacket == NULL)
			{
				INC_RING_INDEX(pBcnRing->TxSwFreeIdx, BCN_RING_SIZE);
				continue;
			}

			if (pPacket)
			{
				PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr0, pTxD->SDLen0, RTMP_PCI_DMA_TODEVICE);
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
			}
			dma_cell[pBcnRing->TxSwFreeIdx].pNdisPacket = NULL;

			pPacket = dma_cell[pBcnRing->TxSwFreeIdx].pNextNdisPacket;
			if (pPacket)
			{
				PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr1, pTxD->SDLen1, RTMP_PCI_DMA_TODEVICE);
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
			}
			dma_cell[pBcnRing->TxSwFreeIdx].pNextNdisPacket = NULL;
			INC_RING_INDEX(pBcnRing->TxSwFreeIdx, BCN_RING_SIZE);

#ifdef RT_BIG_ENDIAN
	        	RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
			WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, TRUE, TYPE_TXD);
#endif
		}
        RTMP_SEM_UNLOCK(&pAd->BcnRingLock);
		//NdisReleaseSpinLock(&pAd->BcnRingLock);
		
		/* Free Tx BMC Ring Packet*/
		pTxRing = &pAd->TxBmcRing;
		
		for (j=0; j< TX_RING_SIZE; j++)
		{	
#ifdef RT_BIG_ENDIAN
			pDestTxD  = (TXD_STRUC *) (pTxRing->Cell[j].AllocVa);
			NdisMoveMemory(&tx_hw_info[0], (UCHAR *)pDestTxD, TXD_SIZE);
			pTxD = (TXD_STRUC *)&tx_hw_info[0];
			RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#else
			pTxD = (TXD_STRUC *) (pTxRing->Cell[j].AllocVa);
#endif /* RT_BIG_ENDIAN */
			pPacket = pTxRing->Cell[j].pNdisPacket;

			if (pPacket)
			{
				PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr0, pTxD->SDLen0, RTMP_PCI_DMA_TODEVICE);
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
			}			
			/*Always assign pNdisPacket as NULL after clear*/
			pTxRing->Cell[j].pNdisPacket = NULL;
			pPacket = pTxRing->Cell[j].pNextNdisPacket;
			if (pPacket)
			{
				PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr1, pTxD->SDLen1, RTMP_PCI_DMA_TODEVICE);
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
			}
			/*Always assign pNextNdisPacket as NULL after clear*/
			pTxRing->Cell[pTxRing->TxSwFreeIdx].pNextNdisPacket = NULL;
#ifdef RT_BIG_ENDIAN
			RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
			WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, FALSE, TYPE_TXD);
#endif /* RT_BIG_ENDIAN */
		}	
	
    	if (pAd->TxBmcBufSpace.AllocVa)
		{
			RTMP_FreeFirstTxBuffer(((POS_COOKIE)(pAd->OS_Cookie))->pci_dev, pAd->TxBmcBufSpace.AllocSize, FALSE, pAd->TxBmcBufSpace.AllocVa, pAd->TxBmcBufSpace.AllocPa);
	    }
	    NdisZeroMemory(&pAd->TxBmcBufSpace, sizeof(RTMP_DMABUF));
	    
    	if (pAd->TxBmcDescRing.AllocVa)
		{
			RtmpFreeDescBuf(((POS_COOKIE)(pAd->OS_Cookie))->pci_dev, pAd->TxBmcDescRing.AllocSize, pAd->TxBmcDescRing.AllocVa, pAd->TxBmcDescRing.AllocPa);
	    }
	    NdisZeroMemory(&pAd->TxBmcDescRing, sizeof(RTMP_DMABUF));
	}
#endif /* MT_MAC */

	for (j = 0; j < NUM_OF_RX_RING; j++)
	{
		for (index = RX_RING_SIZE - 1 ; index >= 0; index--)
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

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
		mt_asic_init_txrx_ring(pAd);
#endif /* MT_MAC */
}


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
	//QUEUE_ENTRY *pEntry;
	PNDIS_PACKET pPacket;
	RTMP_TX_RING *pTxRing;
	ULONG IrqFlags = 0;
	int i, ring_id;


	/*
		We have to clean all descriptors in case some error happened with reset
	*/
	DBGPRINT(RT_DEBUG_OFF,("RTMPRingCleanUp(RingIdx=%d, Pending-NDIS=%ld)\n", RingType, pAd->RalinkCounters.PendingNdisPacketCount));
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
				pTxD  = (TXD_STRUC *) pTxRing->Cell[i].AllocVa;

				pPacket = (PNDIS_PACKET) pTxRing->Cell[i].pNdisPacket;
				/* release scatter-and-gather NDIS_PACKET*/
				if (pPacket)
				{
					RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
					pTxRing->Cell[i].pNdisPacket = NULL;
				}

				pPacket = (PNDIS_PACKET) pTxRing->Cell[i].pNextNdisPacket;
				/* release scatter-and-gather NDIS_PACKET*/
				if (pPacket)
				{
					RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
					pTxRing->Cell[i].pNextNdisPacket = NULL;
				}
			}

			RTMP_IO_READ32(pAd, pTxRing->hw_didx_addr, &pTxRing->TxDmaIdx);
			pTxRing->TxSwFreeIdx = pTxRing->TxDmaIdx;
			pTxRing->TxCpuIdx = pTxRing->TxDmaIdx;
			RTMP_IO_WRITE32(pAd, pTxRing->hw_cidx_addr, pTxRing->TxCpuIdx);

			RTMP_IRQ_UNLOCK(&pAd->irq_lock, IrqFlags);

			//rtmp_tx_swq_exit(pAd, WCID_ALL);
			break;

		case QID_MGMT:
			RTMP_IRQ_LOCK(&pAd->MgmtRingLock, IrqFlags);
			for (i=0; i<MGMT_RING_SIZE; i++)
			{
#ifdef RT_BIG_ENDIAN
				pDestTxD  = (TXD_STRUC *) pAd->MgmtRing.Cell[i].AllocVa;
				TxD = *pDestTxD;
				pTxD = &TxD;
				RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#else
				pTxD  = (TXD_STRUC *) pAd->MgmtRing.Cell[i].AllocVa;
#endif /* RT_BIG_ENDIAN */

				pPacket = (PNDIS_PACKET) pAd->MgmtRing.Cell[i].pNdisPacket;
				/* rlease scatter-and-gather NDIS_PACKET*/
				if (pPacket)
				{
					PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr0, pTxD->SDLen0, RTMP_PCI_DMA_TODEVICE);
					RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
				}
				pAd->MgmtRing.Cell[i].pNdisPacket = NULL;

				pPacket = (PNDIS_PACKET) pAd->MgmtRing.Cell[i].pNextNdisPacket;
				/* release scatter-and-gather NDIS_PACKET*/
				if (pPacket)
				{
					PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr1, pTxD->SDLen1, RTMP_PCI_DMA_TODEVICE);			
					RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
			}
				pAd->MgmtRing.Cell[i].pNextNdisPacket = NULL;

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
				RTMP_RX_RING *pRxRing;
				NDIS_SPIN_LOCK *lock;

				pRxRing = &pAd->RxRing[ring_id];
				lock = &pAd->RxRingLock[ring_id];
				
				RTMP_IRQ_LOCK(lock, IrqFlags);
				for (i=0; i<RX_RING_SIZE; i++)
				{
#ifdef RT_BIG_ENDIAN
					pDestRxD = (RXD_STRUC *)pRxRing->Cell[i].AllocVa;
					RxD = *pDestRxD;
					pRxD = &RxD;
					RTMPDescriptorEndianChange((PUCHAR)pRxD, TYPE_RXD);
#else
					/* Point to Rx indexed rx ring descriptor*/
					pRxD  = (RXD_STRUC *)pRxRing->Cell[i].AllocVa;
#endif /* RT_BIG_ENDIAN */

					pRxD->DDONE = 0;
					pRxD->SDL0 = RX_BUFFER_AGGRESIZE;

#ifdef RT_BIG_ENDIAN
					RTMPDescriptorEndianChange((PUCHAR)pRxD, TYPE_RXD);
					WriteBackToDescriptor((PUCHAR)pDestRxD, (PUCHAR)pRxD, FALSE, TYPE_RXD);
#endif /* RT_BIG_ENDIAN */
				}

				RTMP_IO_READ32(pAd, pRxRing->hw_didx_addr, &pRxRing->RxDmaIdx);
				pRxRing->RxSwReadIdx = pRxRing->RxDmaIdx;
				pRxRing->RxCpuIdx = ((pRxRing->RxDmaIdx == 0) ? (RX_RING_SIZE-1) : (pRxRing->RxDmaIdx-1));
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
				pDestTxD  = (TXD_STRUC *) pAd->CtrlRing.Cell[i].AllocVa;
				TxD = *pDestTxD;
				pTxD = &TxD;
				RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#else
				pTxD  = (TXD_STRUC *) pAd->CtrlRing.Cell[i].AllocVa;
#endif /* RT_BIG_ENDIAN */

				pPacket = (PNDIS_PACKET) pAd->CtrlRing.Cell[i].pNdisPacket;
				/* rlease scatter-and-gather NDIS_PACKET*/
				if (pPacket)
				{
					PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr0, pTxD->SDLen0, RTMP_PCI_DMA_TODEVICE);
					RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
				}
				pAd->CtrlRing.Cell[i].pNdisPacket = NULL;

				pPacket = (PNDIS_PACKET) pAd->CtrlRing.Cell[i].pNextNdisPacket;
				/* release scatter-and-gather NDIS_PACKET*/
				if (pPacket)
				{
					PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr1, pTxD->SDLen1, RTMP_PCI_DMA_TODEVICE);			
					RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
				}
				pAd->CtrlRing.Cell[i].pNextNdisPacket = NULL;

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

#ifdef MT_MAC
		case QID_BCN:
			RTMP_IRQ_LOCK(&pAd->BcnRingLock, IrqFlags);
			
			for (i = 0; i < BCN_RING_SIZE; i++)
			{
#ifdef RT_BIG_ENDIAN
				pDestTxD  = (TXD_STRUC *)pAd->BcnRing.Cell[i].AllocVa;
				TxD = *pDestTxD;
				pTxD = &TxD;
				RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#else
				pTxD  = (TXD_STRUC *) pAd->BcnRing.Cell[i].AllocVa;
#endif /* RT_BIG_ENDIAN */

				pPacket = (PNDIS_PACKET) pAd->BcnRing.Cell[i].pNdisPacket;
				if (pPacket)
				{
					PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr0, pTxD->SDLen0, RTMP_PCI_DMA_TODEVICE);
					//RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
				}
				pAd->BcnRing.Cell[i].pNdisPacket = NULL;

				pPacket = (PNDIS_PACKET) pAd->BcnRing.Cell[i].pNextNdisPacket;
				if (pPacket)
				{
					PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr1, pTxD->SDLen1, RTMP_PCI_DMA_TODEVICE);			
					//RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
				}
				pAd->BcnRing.Cell[i].pNextNdisPacket = NULL;

#ifdef RT_BIG_ENDIAN
				RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
				WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, FALSE, TYPE_TXD);
#endif /* RT_BIG_ENDIAN */
			}

			RTMP_IO_READ32(pAd, pAd->BcnRing.hw_didx_addr, &pAd->BcnRing.TxDmaIdx);
			pAd->BcnRing.TxSwFreeIdx = pAd->BcnRing.TxDmaIdx;
			pAd->BcnRing.TxCpuIdx = pAd->BcnRing.TxDmaIdx;
			RTMP_IO_WRITE32(pAd, pAd->BcnRing.hw_cidx_addr, pAd->BcnRing.TxCpuIdx);

			RTMP_IRQ_UNLOCK(&pAd->BcnRingLock, IrqFlags);

			break;
		case QID_BMC:
			
			for (i = 0; i < TX_RING_SIZE; i++)
			{
#ifdef RT_BIG_ENDIAN
				pDestTxD  = (TXD_STRUC *)pAd->TxBmcRing.Cell[i].AllocVa;
				TxD = *pDestTxD;
				pTxD = &TxD;
				RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#else
				pTxD  = (TXD_STRUC *) pAd->TxBmcRing.Cell[i].AllocVa;
#endif /* RT_BIG_ENDIAN */

				pPacket = (PNDIS_PACKET) pAd->TxBmcRing.Cell[i].pNdisPacket;
				if (pPacket)
				{
					PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr0, pTxD->SDLen0, RTMP_PCI_DMA_TODEVICE);
					RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
				}
				pAd->TxBmcRing.Cell[i].pNdisPacket = NULL;

				pPacket = (PNDIS_PACKET) pAd->TxBmcRing.Cell[i].pNextNdisPacket;
				if (pPacket)
				{
					PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr1, pTxD->SDLen1, RTMP_PCI_DMA_TODEVICE);			
					RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
				}
				pAd->TxBmcRing.Cell[i].pNextNdisPacket = NULL;

#ifdef RT_BIG_ENDIAN
				RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
				WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, FALSE, TYPE_TXD);
#endif /* RT_BIG_ENDIAN */
			}

			RTMP_IO_READ32(pAd, pAd->TxBmcRing.hw_didx_addr, &pAd->TxBmcRing.TxDmaIdx);
			pAd->TxBmcRing.TxSwFreeIdx = pAd->TxBmcRing.TxDmaIdx;
			pAd->TxBmcRing.TxCpuIdx = pAd->TxBmcRing.TxDmaIdx;
			RTMP_IO_WRITE32(pAd, pAd->TxBmcRing.hw_cidx_addr, pAd->TxBmcRing.TxCpuIdx);

			break;
#endif /* MT_MAC */

		default:
			break;
	}
}


VOID PDMAResetAndRecovery(RTMP_ADAPTER *pAd)
{
	UINT32 RemapBase, RemapOffset;
	UINT32 Value;
	UINT32 RestoreValue;
	UINT32 Loop = 0;
	ULONG IrqFlags = 0;

	/* Stop SW Dequeue */
	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DEQUEUEPACKET);

	RTMP_ASIC_INTERRUPT_DISABLE(pAd);
	
	/* Disable PDMA */
	RT28XXDMADisable(pAd);

	RtmpOsMsDelay(1);

	pAd->RxReset = 1;

	/* Assert csr_force_tx_eof */
	RTMP_IO_READ32(pAd, MT_WPDMA_GLO_CFG , &Value);
	Value |= FORCE_TX_EOF;
	RTMP_IO_WRITE32(pAd, MT_WPDMA_GLO_CFG, Value); 

	/* Infor PSE client of TX abort */
	RTMP_IO_READ32(pAd, MCU_PCIE_REMAP_2, &RestoreValue);
	RemapBase = GET_REMAP_2_BASE(RST) << 19;
	RemapOffset = GET_REMAP_2_OFFSET(RST);
	RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RemapBase);
	
	RTMP_IO_READ32(pAd, 0x80000 + RemapOffset, &Value);
	Value |= TX_R_E_1;
	RTMP_IO_WRITE32(pAd, 0x80000 + RemapOffset, Value);

	do
	{
		RTMP_IO_READ32(pAd, 0x80000 + RemapOffset, &Value);
		
		if((Value & TX_R_E_1_S) == TX_R_E_1_S)	
			break;
		RtmpOsMsDelay(1);
		Loop++;
	} while (Loop <= 500);

	if (Loop > 500) 
	{		
		DBGPRINT(RT_DEBUG_ERROR, ("%s: Tx state is not idle(CLIET RST = %x)\n", __FUNCTION__, Value));
		pAd->PDMAResetFailCount++;
	}

	RTMP_IO_READ32(pAd, 0x80000 + RemapOffset, &Value);
	Value |= TX_R_E_2;
	RTMP_IO_WRITE32(pAd, 0x80000 + RemapOffset, Value);

	/* Reset PDMA */
	RTMP_IO_READ32(pAd, MT_WPDMA_GLO_CFG , &Value);
	Value |= SW_RST;
	RTMP_IO_WRITE32(pAd, MT_WPDMA_GLO_CFG, Value); 

	Loop = 0;
	/* Polling for PSE client to clear TX FIFO */
	do
	{
		RTMP_IO_READ32(pAd, 0x80000 + RemapOffset, &Value);
		
		if((Value & TX_R_E_2_S) == TX_R_E_2_S)	
			break;
		RtmpOsMsDelay(1);
		Loop++;
	} while (Loop <= 500);

	if (Loop > 500) {
		DBGPRINT(RT_DEBUG_ERROR, ("%s: Tx FIFO is not empty(CLIET RST = %x)\n", __FUNCTION__, Value));
		pAd->PDMAResetFailCount++;
	}

	/* De-assert PSE client TX abort */
	RTMP_IO_READ32(pAd, 0x80000 + RemapOffset, &Value);
	Value &= ~TX_R_E_1;
	Value &= ~TX_R_E_2;
	RTMP_IO_WRITE32(pAd, 0x80000 + RemapOffset, Value);

	RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RestoreValue);

	AsicDisableSync(pAd);

	RTMP_IRQ_LOCK(&pAd->BcnRingLock, IrqFlags);

#ifdef CONFIG_AP_SUPPORT
	if (pAd->OpMode == OPMODE_AP)
	{
	    UCHAR idx;
	    for (idx = 0; idx < pAd->ApCfg.BssidNum; idx++)
	    {
	        BSS_STRUCT *pMbss;
	        pMbss = &pAd->ApCfg.MBSSID[idx];
	        ASSERT(pMbss);
	        if (pMbss) 
	        {
	            pMbss->bcn_buf.bcn_state = BCN_TX_IDLE;
	        } 
	        else 
	        {
	            DBGPRINT(RT_DEBUG_ERROR, ("%s():func_dev is NULL!\n", __FUNCTION__));
	            RTMP_IRQ_UNLOCK(&pAd->BcnRingLock, IrqFlags);
	            return ;
	        }
	    }
	}
#endif


	RTMP_IRQ_UNLOCK(&pAd->BcnRingLock, IrqFlags);
	
	RTMPRingCleanUp(pAd, QID_AC_BE);
	RTMPRingCleanUp(pAd, QID_AC_BK);
	RTMPRingCleanUp(pAd, QID_AC_VI);
	RTMPRingCleanUp(pAd, QID_AC_VO);
	RTMPRingCleanUp(pAd, QID_MGMT);
	RTMPRingCleanUp(pAd, QID_CTRL);
	RTMPRingCleanUp(pAd, QID_BCN);
	RTMPRingCleanUp(pAd, QID_BMC);
	RTMPRingCleanUp(pAd, QID_RX);

	AsicEnableBssSync(pAd, pAd->CommonCfg.BeaconPeriod);

	/* Enable PDMA */
	RT28XXDMAEnable(pAd);

	RTMP_ASIC_INTERRUPT_ENABLE(pAd);
	
	/* Enable SW Dequeue */	
	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DEQUEUEPACKET);
}


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

	/* Rx DMA unchanged detect */
	NoDataIn = MonitorRxRing(pAd);
			
	if (NoDataIn)
	{
		DBGPRINT(RT_DEBUG_OFF, ("RXDMA Reset\n"));
		pAd->RxDMAResetCount++;
		goto reset;
	}

	return;

reset:

#ifdef DMA_RESET_SUPPORT
	//replace with PSE reset instead of PMDA reset.
	pAd->pse_reset_flag=TRUE;
#else /* DMA_RESET_SUPPORT */
	PDMAResetAndRecovery(pAd);
#endif /* !DMA_RESET_SUPPORT */

}

VOID DumpPseInfo(RTMP_ADAPTER *pAd)
{
	UINT32 RemapBase, RemapOffset;
	UINT32 Value;
	UINT32 RestoreValue;
	UINT32 Index;

	RTMP_IO_READ32(pAd, MCU_PCIE_REMAP_2, &RestoreValue);

	/* PSE Infomation */
	for (Index = 0; Index < 30720; Index++)
	{	
		RemapBase = GET_REMAP_2_BASE(0xa5000000 + Index * 4) << 19;
		RemapOffset = GET_REMAP_2_OFFSET(0xa5000000 + Index * 4);
		RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RemapBase);
	
		RTMP_IO_READ32(pAd, 0x80000 + RemapOffset, &Value);

		DBGPRINT(RT_DEBUG_OFF, ("Offset[0x%x] = 0x%x\n", 0xa5000000 + Index * 4, Value));
	}

	/* Frame linker */
	for (Index  = 0; Index < 1280; Index++)
	{
		RemapBase = GET_REMAP_2_BASE(0xa00001b0) << 19;
		RemapOffset = GET_REMAP_2_OFFSET(0xa00001b0);
		RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RemapBase);

		RTMP_IO_READ32(pAd, 0x80000 + RemapOffset, &Value);
		Value &= ~0xfff;
		Value |= (Index & 0xfff);
		RTMP_IO_WRITE32(pAd, 0x80000 + RemapOffset, Value);

		RemapBase = GET_REMAP_2_BASE(0xa00001b4) << 19;
		RemapOffset = GET_REMAP_2_OFFSET(0xa00001b4);
		RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RemapBase);
		RTMP_IO_READ32(pAd, 0x80000 + RemapOffset, &Value);

		DBGPRINT(RT_DEBUG_OFF, ("Frame Linker(0x%x) = 0x%x\n", Index, Value ));

	}

	/* Page linker */
	for (Index = 0; Index < 1280; Index++)
	{
		RemapBase = GET_REMAP_2_BASE(0xa00001b8) << 19;
		RemapOffset = GET_REMAP_2_OFFSET(0xa00001b8);
		RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RemapBase);

		RTMP_IO_READ32(pAd, 0x80000 + RemapOffset, &Value);
		Value &= ~(0xfff << 16);
		Value |= ((Index & 0xfff) << 16);
		RTMP_IO_WRITE32(pAd, 0x80000 + RemapOffset, Value);
		
		RemapBase = GET_REMAP_2_BASE(0xa00001b8) << 19;
		RemapOffset = GET_REMAP_2_OFFSET(0xa00001b8);
		RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RemapBase);
		RTMP_IO_READ32(pAd, 0x80000 + RemapOffset, &Value);

		DBGPRINT(RT_DEBUG_OFF, ("Page Linker(0x%x) = 0x%x\n", Index, (Value & 0xfff)));
	}
	
	RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RestoreValue);
}


VOID PSEResetAndRecovery(RTMP_ADAPTER *pAd)
{
	UINT32 Loop = 0;
	UINT32 Value;


#ifdef RTMP_PCI_SUPPORT
	NdisAcquireSpinLock(&pAd->IndirectUpdateLock);
#endif

	RTMP_IO_READ32(pAd, 0x816c, &Value);
	if((Value & (1 << 1)) == (1 << 1))
	{
		Value &= ~(1 << 1);
		DBGPRINT(RT_DEBUG_ERROR, ("@@@ %s: This Reset Result may be old! force clean result first! \n", __FUNCTION__));
		RTMP_IO_WRITE32(pAd, 0x816c, Value);
	}

	RTMP_IO_READ32(pAd, 0x816c, &Value);
	Value |= (1 << 0);
	RTMP_IO_WRITE32(pAd, 0x816c, Value);

	do
	{
		RTMP_IO_READ32(pAd, 0x816c, &Value);
		
		if((Value & (1 << 1)) == (1 << 1))
		{
			Value &= ~(1 << 1);
			RTMP_IO_WRITE32(pAd, 0x816c, Value);
			break;
		}
		RtmpOsMsDelay(1);
		Loop++;
	} while (Loop <= 500);

	if (Loop > 500) 
	{		
		DBGPRINT(RT_DEBUG_ERROR, ("%s: PSE Reset Fail(%x)\n", __FUNCTION__, Value));

#ifdef RTMP_PCI_SUPPORT
		/*Dump the PSE CRs*/
		{
			UINT32 RemapBase, RemapOffset;
			UINT32 Value;
			UINT32 RestoreValue;

			{
				DBGPRINT(RT_DEBUG_ERROR, ("%s: START ============== PSE CR dump ===================\n", __FUNCTION__));

				RTMP_IO_READ32(pAd, 0x816c, &Value);
				DBGPRINT(RT_DEBUG_ERROR, ("mac [0x816c] = 0x%08x\n", Value));
				RTMP_IO_READ32(pAd, 0x8170, &Value);				
				DBGPRINT(RT_DEBUG_ERROR, ("mac [0x8170] = 0x%08x\n", Value));				

				RTMP_IO_READ32(pAd, MCU_PCIE_REMAP_2, &RestoreValue);
				RemapBase = GET_REMAP_2_BASE(0x800c0070) << 19;
				RemapOffset = GET_REMAP_2_OFFSET(0x800c0070);
				RTMP_IO_READ32(pAd, 0x80000 + RemapOffset, &Value);
				DBGPRINT(RT_DEBUG_ERROR, ("rw 3 : mac [0x800c0070] = 0x%08x\n", Value));

				//RTMP_IO_READ32(pAd, MCU_PCIE_REMAP_2, &RestoreValue);
				RemapBase = GET_REMAP_2_BASE(0x800c006c) << 19;
				RemapOffset = GET_REMAP_2_OFFSET(0x800c006c);
				RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RemapBase);

				// write 3
				RTMP_IO_WRITE32(pAd, 0x80000 + RemapOffset, 3);
				RTMP_IO_READ32(pAd, 0x80000 + RemapOffset, &Value);
				DBGPRINT(RT_DEBUG_ERROR, ("rw 3 : mac [0x800c006c] = 0x%08x\n", Value));
				
				// write 4
				RTMP_IO_WRITE32(pAd, 0x80000 + RemapOffset, 4);
				RTMP_IO_READ32(pAd, 0x80000 + RemapOffset, &Value);
				DBGPRINT(RT_DEBUG_ERROR, ("rw 4 : mac [0x800c006c] = 0x%08x\n", Value));

				// write 5
				RTMP_IO_WRITE32(pAd, 0x80000 + RemapOffset, 5);
				RTMP_IO_READ32(pAd, 0x80000 + RemapOffset, &Value);
				DBGPRINT(RT_DEBUG_ERROR, ("rw 5 : mac [0x800c006c] = 0x%08x\n", Value));

				// write 6
				RTMP_IO_WRITE32(pAd, 0x80000 + RemapOffset, 6);
				RTMP_IO_READ32(pAd, 0x80000 + RemapOffset, &Value);
				DBGPRINT(RT_DEBUG_ERROR, ("rw 6 :mac [0x800c006c] = 0x%08x\n", Value));

				// write 7
				RTMP_IO_WRITE32(pAd, 0x80000 + RemapOffset, 7);
				RTMP_IO_READ32(pAd, 0x80000 + RemapOffset, &Value);
				DBGPRINT(RT_DEBUG_ERROR, ("rw 7 :mac [0x800c006c] = 0x%08x\n", Value));

				// write 8
				RTMP_IO_WRITE32(pAd, 0x80000 + RemapOffset, 8);
				RTMP_IO_READ32(pAd, 0x80000 + RemapOffset, &Value);
				DBGPRINT(RT_DEBUG_ERROR, ("rw 8 :mac [0x800c006c] = 0x%08x\n", Value));

				// write 9
				RTMP_IO_WRITE32(pAd, 0x80000 + RemapOffset, 9);
				RTMP_IO_READ32(pAd, 0x80000 + RemapOffset, &Value);
				DBGPRINT(RT_DEBUG_ERROR, ("rw 9 :mac [0x800c006c] = 0x%08x\n", Value));

				// write a
				RTMP_IO_WRITE32(pAd, 0x80000 + RemapOffset, 0xa);
				RTMP_IO_READ32(pAd, 0x80000 + RemapOffset, &Value);
				DBGPRINT(RT_DEBUG_ERROR, ("rw a :mac [0x800c006c] = 0x%08x\n", Value));

				// write b
				RTMP_IO_WRITE32(pAd, 0x80000 + RemapOffset, 0xb);
				RTMP_IO_READ32(pAd, 0x80000 + RemapOffset, &Value);
				DBGPRINT(RT_DEBUG_ERROR, ("rw b :mac [0x800c006c] = 0x%08x\n", Value));

				// write c
				RTMP_IO_WRITE32(pAd, 0x80000 + RemapOffset, 0xc);
				RTMP_IO_READ32(pAd, 0x80000 + RemapOffset, &Value);
				DBGPRINT(RT_DEBUG_ERROR, ("rw c :mac [0x800c006c] = 0x%08x\n", Value));

				RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RestoreValue);
				DBGPRINT(RT_DEBUG_ERROR, ("%s: END ============== PSE CR dump ===================\n", __FUNCTION__));
				
			}
		}
#endif /* RTMP_PCI_SUPPORT */

#ifdef DMA_RESET_SUPPORT

		RTMP_IO_READ32(pAd, 0x816c, &Value);
		if ((Value & (1 << 0)) == (1 << 0))
		{
			DBGPRINT(RT_DEBUG_ERROR, ("@@@ %s: Host Poll Fail !! Try to abort this PSE reset to F/W!! \n", __FUNCTION__));
			Value &= ~(1 << 0);
			RTMP_IO_WRITE32(pAd, 0x816c, Value);
		}	

		if (pAd->PSEResetFailRecover == FALSE)
		{
			pAd->PSEResetFailRecover = TRUE;
			pAd->PSEResetFailRetryQuota = 3; // start to count down
		}
		else
		{
			if (pAd->PSEResetFailRetryQuota)
				pAd->PSEResetFailRetryQuota --;

			if (pAd->PSEResetFailRetryQuota == 0) // reach the quota
			{
				pAd->PSEResetFailRecover = FALSE;
				DBGPRINT(RT_DEBUG_ERROR, ("%s: PSE Reset Retry Reach Quota!!\n", __FUNCTION__));
			}
		}
#endif /* DMA_RESET_SUPPORT */

		pAd->PSEResetFailCount++;


#ifdef RTMP_PCI_SUPPORT
	NdisReleaseSpinLock(&pAd->IndirectUpdateLock);
#endif

	return;

	}
#ifdef DMA_RESET_SUPPORT	
	else //Reset Success
	{
		if (pAd->PSEResetFailRecover)
		{
			DBGPRINT(RT_DEBUG_ERROR, ("%s: PSE Reset Recover Back!!\n", __FUNCTION__));
		}
		
		pAd->PSEResetFailRecover = FALSE;
		pAd->PSEResetFailRetryQuota = 0;
	}

	//clear the AC / Mgmt Hit event.
	RTMP_IO_READ32(pAd, 0x816c, &Value);
	if((Value & (1 << 2)) == (1 << 2))
	{
		Value &= ~(1 << 2);
		RTMP_IO_WRITE32(pAd, 0x816c, Value);
	}

	if((Value & (1 << 3)) == (1 << 3))
	{
		Value &= ~(1 << 3);
		RTMP_IO_WRITE32(pAd, 0x816c, Value);
	}
		if((Value & (1 << 4)) == (1 << 4))
	{
		Value &= ~(1 << 4);
		RTMP_IO_WRITE32(pAd, 0x816c, Value);
	}

	if((Value & (1 << 5)) == (1 << 5))
	{
		Value &= ~(1 << 5);
		RTMP_IO_WRITE32(pAd, 0x816c, Value);
	}

	if((Value & (1 << 6)) == (1 << 6))
	{
		Value &= ~(1 << 6);
		RTMP_IO_WRITE32(pAd, 0x816c, Value);
	}	
#endif /* DMA_RESET_SUPPORT */


#ifdef RTMP_PCI_SUPPORT
	NdisReleaseSpinLock(&pAd->IndirectUpdateLock);
#endif

	PDMAResetAndRecovery(pAd);
}



#ifdef DMA_RESET_SUPPORT
VOID PSEACStuckWatchDog(RTMP_ADAPTER *pAd)
{
	UINT32 Value;

	RTMP_IO_READ32(pAd, 0x816c, &Value);

	//AC
	if((Value & (1 << 2)) == (1 << 2))
	{
		//clear after reset
		//Value &= ~(1 << 2);
		//RTMP_IO_WRITE32(pAd, 0x816c, Value);
		if (
#ifdef MAX_CONTINOUS_TX_CNT
			pAd->ContinousTxCnt != 1 ||
#endif
			pAd->ed_on == TRUE) {
			pAd->pse_reset_flag = FALSE;
			Value &= ~(1 << 2);
			RTMP_IO_WRITE32(pAd, 0x816c, Value);
			DBGPRINT(RT_DEBUG_OFF, ("Ignore HIT AC0 ! pAd->ed_on = %d\n", pAd->ed_on));
#ifdef MAX_CONTINOUS_TX_CNT
			DBGPRINT(RT_DEBUG_OFF, ("pAd->ContinousTxCnt = %d\n", pAd->ContinousTxCnt));
#endif
		} else {
			DBGPRINT(RT_DEBUG_OFF, ("HIT AC0 !\n"));

			pAd->AC0HitCount++;
			pAd->pse_reset_flag = TRUE;
		}
	}


	if((Value & (1 << 3)) == (1 << 3))
	{
		//clear after reset	
		//Value &= ~(1 << 3);
		//RTMP_IO_WRITE32(pAd, 0x816c, Value);
		if (
#ifdef MAX_CONTINOUS_TX_CNT
			pAd->ContinousTxCnt != 1 ||
#endif
			pAd->ed_on == TRUE) {
			pAd->pse_reset_flag = FALSE;
			Value &= ~(1 << 3);
			RTMP_IO_WRITE32(pAd, 0x816c, Value);
			DBGPRINT(RT_DEBUG_OFF, ("Ignore HIT AC1 ! pAd->ed_on = %d\n", pAd->ed_on));
#ifdef MAX_CONTINOUS_TX_CNT
			DBGPRINT(RT_DEBUG_OFF, ("pAd->ContinousTxCnt = %d\n", pAd->ContinousTxCnt));
#endif
		} else {
			DBGPRINT(RT_DEBUG_OFF, ("HIT AC1 !\n"));

			pAd->AC1HitCount++;
			pAd->pse_reset_flag = TRUE;
		}
	}

	if((Value & (1 << 4)) == (1 << 4))
	{
		//clear after reset	
		//Value &= ~(1 << 4);
		//RTMP_IO_WRITE32(pAd, 0x816c, Value);
		if (
#ifdef MAX_CONTINOUS_TX_CNT
			pAd->ContinousTxCnt != 1 ||
#endif
			pAd->ed_on == TRUE) {
			pAd->pse_reset_flag = FALSE;
			Value &= ~(1 << 4);
			RTMP_IO_WRITE32(pAd, 0x816c, Value);
			DBGPRINT(RT_DEBUG_OFF, ("Ignore HIT AC2 ! pAd->ed_on = %d\n", pAd->ed_on));
#ifdef MAX_CONTINOUS_TX_CNT
			DBGPRINT(RT_DEBUG_OFF, ("pAd->ContinousTxCnt = %d\n", pAd->ContinousTxCnt));
#endif
		} else {
			DBGPRINT(RT_DEBUG_OFF, ("HIT AC2 !\n"));

			pAd->AC2HitCount++;
			pAd->pse_reset_flag = TRUE;
		}
	}
	

	if((Value & (1 << 5)) == (1 << 5))
	{
		//clear after reset
		//Value &= ~(1 << 5);
		//RTMP_IO_WRITE32(pAd, 0x816c, Value);
		if (
#ifdef MAX_CONTINOUS_TX_CNT
			pAd->ContinousTxCnt != 1 ||
#endif
			pAd->ed_on == TRUE) {
			pAd->pse_reset_flag = FALSE;
			Value &= ~(1 << 5);
			RTMP_IO_WRITE32(pAd, 0x816c, Value);
			DBGPRINT(RT_DEBUG_OFF, ("Ignore HIT AC3 ! pAd->ed_on = %d\n", pAd->ed_on));
#ifdef MAX_CONTINOUS_TX_CNT
			DBGPRINT(RT_DEBUG_OFF, ("pAd->ContinousTxCnt = %d\n", pAd->ContinousTxCnt));
#endif
		} else {
			DBGPRINT(RT_DEBUG_OFF, ("HIT AC3 !\n"));

			pAd->AC3HitCount++;
			pAd->pse_reset_flag = TRUE;
		}
		//return TRUE;
	}


	if((Value & (1 << 6)) == (1 << 6))
	{
		//clear after reset
		//Value &= ~(1 << 6);
		//RTMP_IO_WRITE32(pAd, 0x816c, Value);

		if (RTDebugLevel >= RT_DEBUG_OFF)
			printk("HIT MGMT !\n");

		pAd->MgtHitCount ++;
		pAd->pse_reset_flag = TRUE;
		//return TRUE;
	}
	
}
#endif /* DMA_RESET_SUPPORT */

VOID PSEWatchDog(RTMP_ADAPTER *pAd)
{
	BOOLEAN NoDataIn = FALSE;

	NoDataIn = MonitorRxPse(pAd);
			
	if (((NoDataIn)
#ifdef DMA_RESET_SUPPORT		
		|| ((pAd->bcn_reset_en) && (pAd->pse_reset_flag))
		|| ((pAd->PSEResetFailRecover) && (pAd->PSEResetFailRetryQuota))
#endif		
		)
		&& (pAd->pse_reset_exclude_flag == FALSE))
	{
		DBGPRINT(RT_DEBUG_OFF, ("PSE Reset: MonitorRxPse\n"));
		DBGPRINT(RT_DEBUG_OFF, ("NoDataIn = %d, bcn_reset_en = %d, pse_reset_flag = %d\n",
			NoDataIn, pAd->bcn_reset_en, pAd->pse_reset_flag));
		DBGPRINT(RT_DEBUG_OFF, ("PSEResetFailRecover = %d, PSEResetFailRetryQuota = %ld\n",
			pAd->PSEResetFailRecover, pAd->PSEResetFailRetryQuota));
		pAd->PSEResetCount++;
		goto reset;
	}

/*MT7628's case may not apply to MT7603 */

	return;

reset:
#ifdef DMA_RESET_SUPPORT	
	pAd->pse_reset_flag=TRUE;
#endif	
	pAd->pse_reset_exclude_flag = TRUE;
	PSEResetAndRecovery(pAd);
	pAd->pse_reset_exclude_flag = FALSE;

#ifdef DMA_RESET_SUPPORT	
	pAd->pse_reset_flag=FALSE;
#endif
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
	AsicSetWPDMA(pAd, PDMA_TX_RX, FALSE);
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
	AsicSetMacTxRx(pAd, ASIC_MAC_TX, TRUE);
	AsicWaitPDMAIdle(pAd, 200, 1000);

	RtmpusecDelay(50);

	AsicSetWPDMA(pAd, PDMA_TX_RX, TRUE);

	DBGPRINT(RT_DEBUG_TRACE, ("<== %s(): WPDMABurstSIZE = %d\n", __FUNCTION__, pAd->chipCap.WPDMABurstSIZE));
}


BOOLEAN AsicCheckCommanOk(RTMP_ADAPTER *pAd, UCHAR Command)
{
	BOOLEAN status = FALSE;

	// TODO: shiang-7603
	if (pAd->chipCap.hif_type == HIF_MT) {
		DBGPRINT(RT_DEBUG_OFF, ("%s(%d): Not support for HIF_MT yet!\n",
							__FUNCTION__, __LINE__));
		return TRUE;
	}
	



	return status;
}


#ifdef MT_MAC
VOID RT28xx_UpdateBeaconToAsic(
	IN RTMP_ADAPTER *pAd,
	IN INT apidx,
	IN ULONG FrameLen,
	IN ULONG UpdatePos)
{
	BCN_BUF_STRUC *bcn_buf = NULL;
	UCHAR *buf/*, *hdr*/;
	INT len;
	PNDIS_PACKET *pkt = NULL;

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		bcn_buf = &pAd->ApCfg.MBSSID[apidx].bcn_buf;
	}
#endif /* CONFIG_AP_SUPPORT */


	if (!bcn_buf) {
		DBGPRINT(RT_DEBUG_ERROR, ("%s(): bcn_buf is NULL!\n", __FUNCTION__));
		return;
	}

	pkt = bcn_buf->BeaconPkt;
	if (pkt) {
		buf = (UCHAR *)GET_OS_PKT_DATAPTR(pkt);
		len = FrameLen + pAd->chipCap.tx_hw_hdr_len;
		SET_OS_PKT_LEN(pkt, len);
#ifdef RT_BIG_ENDIAN		
		MTMacInfoEndianChange(pAd, buf, TYPE_TMACINFO, sizeof(TMAC_TXD_L));
#endif /* RT_BIG_ENDIAN */
		/* Now do hardware-depened kick out.*/
		RTMP_SEM_LOCK(&pAd->BcnRingLock);
		HAL_KickOutMgmtTx(pAd, Q_IDX_BCN, pkt, buf, len);
		bcn_buf->bcn_state = BCN_TX_WRITE_TO_DMA;
		RTMP_SEM_UNLOCK(&pAd->BcnRingLock);
	} else {
		DBGPRINT(RT_DEBUG_ERROR, ("%s(): BeaconPkt is NULL!\n", __FUNCTION__));
	}
}
#endif /* MT_MAC */


#if defined(RTMP_MAC) || defined(RLT_MAC)
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
	INT wr_bytes = 1;
	UCHAR *pBeaconFrame, *tmac_info;
	UCHAR tx_hw_hdr_len = pAd->chipCap.tx_hw_hdr_len;

	if (IS_MT76x2(pAd))
		wr_bytes = 4;

#ifdef CONFIG_AP_SUPPORT
	if (apidx < pAd->ApCfg.BssidNum &&
		apidx < HW_BEACON_MAX_NUM &&
	       (pAd->OpMode == OPMODE_AP)
		)
	{
		BSS_STRUCT *pMbss;

		pMbss = &pAd->ApCfg.MBSSID[apidx];
		bcn_idx = pMbss->bcn_buf.BcnBufIdx;
		CapInfoPos = pMbss->bcn_buf.cap_ie_pos;
		bBcnReq = BeaconTransmitRequired(pAd, apidx, pMbss);

		if (wr_bytes > 1) {
			CapInfoPos = (CapInfoPos & (~(wr_bytes - 1)));
			UpdatePos = (UpdatePos & (~(wr_bytes - 1)));
		}

		tmac_info = (UCHAR *)GET_OS_PKT_DATAPTR(pMbss->bcn_buf.BeaconPkt);
		pBeaconFrame = (UCHAR *)(tmac_info + TXWISize);
		
		ptr_capinfo = (PUCHAR)(pBeaconFrame + CapInfoPos);
		ptr_update  = (PUCHAR)(pBeaconFrame + UpdatePos);
	}
	else
#endif /* CONFIG_AP_SUPPORT */
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s():Invalid Interface\n", __FUNCTION__));
		return;
	}

	if (pAd->BeaconOffset[bcn_idx] == 0) {
		DBGPRINT(RT_DEBUG_ERROR, ("%s():Invalid BcnOffset[%d]\n",
					__FUNCTION__, bcn_idx));
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
		ptr = tmac_info;

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
		for (i = CapInfoPos; i < (CapInfoPos+2); i += wr_bytes)
		{
			RTMP_CHIP_UPDATE_BEACON(pAd, pAd->BeaconOffset[bcn_idx] + TXWISize + i, *((UINT32 *)ptr_capinfo), wr_bytes);
			ptr_capinfo += wr_bytes;
		}

		if (FrameLen > UpdatePos)
		{
			for (i = UpdatePos; i < (FrameLen); i += wr_bytes)
			{
				UINT32 longptr =  *ptr_update + (*(ptr_update+1)<<8) + (*(ptr_update+2)<<16) + (*(ptr_update+3)<<24);
				RTMP_CHIP_UPDATE_BEACON(pAd, pAd->BeaconOffset[bcn_idx] + TXWISize + i, longptr, wr_bytes);
				ptr_update += wr_bytes;
			}
		}
	}
}
#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */




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

	if (pAd->OpMode == OPMODE_AP && Level==DOT11POWERSAVE)
		return FALSE;


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
 			/* add by johnli, RF power sequence setup, load RF normal operation-mode setup*/

		RTMP_CHIP_OP *pChipOps = &pAd->chipOps;

		if (pChipOps->AsicReverseRfFromSleepMode)
		{
			pChipOps->AsicReverseRfFromSleepMode(pAd, FALSE);

		}
		else
		{
	    		/* In Radio Off, we turn off RF clk, So now need to call ASICSwitchChannel again.*/
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
#if defined(RTMP_MAC) || defined(RLT_MAC)
    UINT32 RxDmaIdx, RxCpuIdx;
#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */

	// TODO: shiang-7603
	if (pAd->chipCap.hif_type == HIF_MT) {
		DBGPRINT(RT_DEBUG_OFF, ("%s(): Not support for HIF_MT yet!\n",
					__FUNCTION__));
		return FALSE;
	}

#if defined(RTMP_MAC) || defined(RLT_MAC)
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
	/*pAd->bPCIclkOffDisableTx = FALSE;*/
    RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF);
    
    
	/* In Radio Off, we turn off RF clk, So now need to call ASICSwitchChannel again.*/
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

    

/* Disable for stability. If PCIE Link Control is modified for advance power save, re-covery this code segment.*/
/*RTMP_IO_WRITE32(pAd, PBF_SYS_CTRL, 0x1280);*/
/*OPSTATUS_SET_FLAG(pAd, fOP_STATUS_CLKSELECT_40MHZ);*/

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
	

    	/*pAd->bPCIclkOffDisableTx = FALSE;*/

#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */

	return TRUE;
}


VOID PciMlmeRadioOn(RTMP_ADAPTER *pAd)
{
#ifdef LOAD_FW_ONE_TIME
        {
                UINT32 value;
                RTMP_IO_READ32(pAd, AGG_TEMP, &value);
                value &= 0x0000ffff;
                RTMP_IO_WRITE32(pAd, AGG_TEMP, value);
        }
#endif /* LOAD_FW_ONE_TIME */
    
	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF))
		return;

	DBGPRINT(RT_DEBUG_TRACE,("%s===>\n", __FUNCTION__));   

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT) {
		MTPciMlmeRadioOn(pAd);
		return;
	}
#endif /* MT_MAC */

#if defined(RTMP_MAC) || defined(RLT_MAC)


	if ((pAd->OpMode == OPMODE_AP) ||
		((pAd->OpMode == OPMODE_STA) 
		))
	{
		RTMPRingCleanUp(pAd, QID_AC_BK);
		RTMPRingCleanUp(pAd, QID_AC_BE);
		RTMPRingCleanUp(pAd, QID_AC_VI);
		RTMPRingCleanUp(pAd, QID_AC_VO);
		RTMPRingCleanUp(pAd, QID_HCCA);
		RTMPRingCleanUp(pAd, QID_MGMT);
		RTMPRingCleanUp(pAd, QID_RX);

#ifdef RTMP_PCI_SUPPORT
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
		RTMPSetLED(pAd, LED_RADIO_ON);
#ifdef CONFIG_AP_SUPPORT
		/* The LEN_RADIO_ON indicates "Radio on but link down", 
		so AP shall set LED LINK_UP status */
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			RTMPSetLED(pAd, LED_LINK_UP);
		}
#endif /* CONFIG_AP_SUPPORT */
#endif /* LED_CONTROL_SUPPORT */
	}


#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */
}


VOID PciMlmeRadioOFF(RTMP_ADAPTER *pAd)
{
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF))
		return;

	DBGPRINT(RT_DEBUG_TRACE,("%s===>\n", __FUNCTION__));

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT) {
		MTPciMlmeRadioOff(pAd);
		return;
	}
#endif /* MT_MAC */

#if defined(RTMP_MAC) || defined(RLT_MAC)


	/* Set Radio off flag*/
	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF);


#ifdef CONFIG_AP_SUPPORT
#ifdef AP_SCAN_SUPPORT
	{
		BOOLEAN Cancelled;
		RTMPCancelTimer(&pAd->ScanCtrl.APScanTimer, &Cancelled);
	}
#endif /* AP_SCAN_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

#ifdef LED_CONTROL_SUPPORT
	RTMPSetLED(pAd, LED_RADIO_OFF);
#endif /* LED_CONTROL_SUPPORT */

#ifdef RTMP_PCI_SUPPORT
	if (pAd->infType == RTMP_DEV_INF_PCI || pAd->infType == RTMP_DEV_INF_PCIE)
	{
		{
			if (RT28xxPciAsicRadioOff(pAd, GUIRADIO_OFF, 0) ==FALSE)
			{
				DBGPRINT(RT_DEBUG_ERROR,("%s call RT28xxPciAsicRadioOff fail !!\n", __FUNCTION__)); 
			}
		}
	}
#endif /* RTMP_PCI_SUPPORT */


#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */

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
	ra_dma_addr_t SrcBufPA;
	
	if (sd_idx == 1)
	{
		TX_BLK *pTxBlk = (TX_BLK *)(ptr);

		if (pTxBlk->SrcBufLen)
		{
			SrcBufPA = linux_pci_map_single(((POS_COOKIE)(pAd->OS_Cookie))->pci_dev,pTxBlk->pSrcBufData, pTxBlk->SrcBufLen, 0, direction);
		}
		else
		{
			return 0;
		}

	}
	else
	{
		SrcBufPA = linux_pci_map_single(((POS_COOKIE)(pAd->OS_Cookie))->pci_dev,
					ptr, size, 0, direction);
	}


	if (dma_mapping_error(&((POS_COOKIE)(pAd->OS_Cookie))->pci_dev->dev, SrcBufPA))
	{
		DBGPRINT(RT_DEBUG_OFF, ("%s: dma mapping error\n", __FUNCTION__));
		return 0;
	}
	 else
	{
		return SrcBufPA;
	}
}


int write_reg(RTMP_ADAPTER *ad, UINT32 base, UINT16 offset, UINT32 value)
{
	// TODO: shiang-7603
	if (ad->chipCap.hif_type == HIF_MT) {
		DBGPRINT(RT_DEBUG_OFF, ("%s(): Not support for HIF_MT yet!\n",
					__FUNCTION__));
		return FALSE;
	}
	
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
	// TODO: shiang-7603
	if (ad->chipCap.hif_type == HIF_MT) {
		DBGPRINT(RT_DEBUG_OFF, ("%s(): Not support for HIF_MT yet!\n",
					__FUNCTION__));
		return FALSE;
	}

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
	unsigned long irqFlags;
	UINT32 reg_mask = 0;

#ifdef RLT_MAC
	if (pAd->chipCap.hif_type == HIF_RLT)
		reg_mask = (RLT_DELAYINTMASK) |(RLT_RxINT|RLT_TxDataInt|RLT_TxMgmtInt);
#endif /* RLT_MAC */

#ifdef RTMP_MAC
	if (pAd->chipCap.hif_type == HIF_RTMP)
		reg_mask = ((RTMP_DELAYINTMASK) |(RTMP_RxINT|RTMP_TxDataInt|RTMP_TxMgmtInt)) & ~(0x03);
#endif /* RTMP_MAC */

#ifdef MT_MAC
	// TODO: shiang-7603
	if (pAd->chipCap.hif_type == HIF_MT)
		reg_mask = ((MT_DELAYINTMASK) |(MT_RxINT|MT_TxDataInt|MT_TxMgmtInt)|MT_INT_BMC_DLY);
#endif /* MT_MAC */

	RTMP_INT_LOCK(&pAd->irq_lock, irqFlags);
	pAd->int_enable_reg = reg_mask;
	pAd->int_disable_mask = 0;
	pAd->int_pending = 0;
	RTMP_INT_UNLOCK(&pAd->irq_lock, irqFlags);

	return 0;
}

#endif /* RTMP_MAC_PCI */


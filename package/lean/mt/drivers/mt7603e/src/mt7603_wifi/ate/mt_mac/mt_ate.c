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
	mt_ate.c
*/

#include "rt_config.h"

static INT32 MT_ATESetTxPower0(RTMP_ADAPTER *pAd, CHAR Value)
{
	ATE_CTRL *ATECtrl = &pAd->ATECtrl;
    //struct MT_TX_PWR_CAP *cap = &pAd->chipCap.MTTxPwrCap;
	INT32 Ret = 0;

	DBGPRINT(RT_DEBUG_OFF, ("%s\n", __FUNCTION__));
	ATECtrl->TxPower0 = Value;
	ATECtrl->TxPower1 = pAd->EEPROMImage[TX1_G_BAND_TARGET_PWR];
	CmdSetTxPowerCtrl(pAd, ATECtrl->Channel);
	return Ret;
}

static INT32 MT_ATESetTxPower1(RTMP_ADAPTER *pAd, CHAR Value)
{
	ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	INT32 Ret = 0;
	// For TxTone Power	
	ATECtrl->TxPower1 = Value;
	DBGPRINT(RT_DEBUG_OFF, ("%s, power1:0x%x\n", __FUNCTION__,ATECtrl->TxPower1));
	/* Same as Power0 */
    if ( ATECtrl->TxPower0 != ATECtrl->TxPower1)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s: power1 do not same as power0\n", __FUNCTION__));
        Ret = -1;
	}

	return Ret;
}


static INT32 MT_ATEStart(RTMP_ADAPTER *pAd)
{
	ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	INT32 Ret = 0;

#ifdef RTMP_PCI_SUPPORT
	UINT32 Index;
	TXD_STRUC *pTxD = NULL;
	RTMP_TX_RING *pTxRing = &pAd->TxRing[QID_AC_BE];
#endif

#ifdef RT_BIG_ENDIAN
	TXD_STRUC *pDestTxD = NULL;
	UCHAR TxHwInfo[TXD_SIZE];
#endif

#ifdef CONFIG_AP_SUPPORT
	INT32 IdBss, MaxNumBss = pAd->ApCfg.BssidNum;
#endif
	DBGPRINT(RT_DEBUG_OFF, ("%s\n", __FUNCTION__));

	/*   Stop send TX packets */
	RTMP_OS_NETDEV_STOP_QUEUE(pAd->net_dev);
#ifdef SINGLE_SKU_V2
	pAd->SKUEn = 0;
#endif
    /* Reset ATE TX/RX Counter */
    ATECtrl->TxDoneCount = 0;
    ATECtrl->RxTotalCnt = 0;
	ATECtrl->cmd_expire = RTMPMsecsToJiffies(3000);
	RTMP_OS_INIT_COMPLETION(&ATECtrl->cmd_done);
#ifdef CONFIG_ATE
	ATECtrl->TxPower0 = pAd->EEPROMImage[TX0_G_BAND_TARGET_PWR];
	ATECtrl->TxPower1 = pAd->EEPROMImage[TX1_G_BAND_TARGET_PWR];
	DBGPRINT(RT_DEBUG_INFO, ("%s, Init Txpower, Tx0:%x, Tx1:%x\n", __FUNCTION__, ATECtrl->TxPower0, ATECtrl->TxPower1));
#ifdef CONFIG_QA
	AsicGetRxStat(pAd, HQA_RX_RESET_PHY_COUNT);
	ATECtrl->RxMacMdrdyCount = 0;
	ATECtrl->RxMacFCSErrCount = 0;
#endif /* CONFIG_QA */
#endif /* CONFIG_ATE */

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		if (MaxNumBss > MAX_MBSSID_NUM(pAd))
			MaxNumBss = MAX_MBSSID_NUM(pAd);

		/*  first IdBss must not be 0 (BSS0), must be 1 (BSS1) */
		for (IdBss = FIRST_MBSSID; IdBss < MAX_MBSSID_NUM(pAd); IdBss++)
		{
			if (pAd->ApCfg.MBSSID[IdBss].wdev.if_dev)
				RTMP_OS_NETDEV_STOP_QUEUE(pAd->ApCfg.MBSSID[IdBss].wdev.if_dev);
		}
	}
#endif

	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DEQUEUEPACKET);

	AsicSetMacTxRx(pAd, ASIC_MAC_RX, FALSE);
	
	/*  Disable TX PDMA */
	AsicSetWPDMA(pAd, PDMA_TX, 0);

#ifdef RTMP_MAC_PCI
	/* Polling TX/RX path until packets empty */
	MTPciPollTxRxEmpty(pAd);


	for (Index = 0; Index < TX_RING_SIZE; Index++)
	{
		PNDIS_PACKET  pPacket;
#ifndef RT_BIG_ENDIAN
		pTxD = (TXD_STRUC *)pAd->TxRing[QID_AC_BE].Cell[Index].AllocVa;
#else
		pDestTxD = (TXD_STRUC *)pAd->TxRing[QID_AC_BE].Cell[Index].AllocVa;
		NdisMoveMemory(&TxHwInfo[0], (UCHAR *)pDestTxD, TXD_SIZE);
		pTxD = (TXD_STRUC *)&TxHwInfo[0];
		RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#endif
		pTxD->DMADONE = 0;
		pPacket = pTxRing->Cell[Index].pNdisPacket;

		if (pPacket)
		{
			PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr0, pTxD->SDLen0, RTMP_PCI_DMA_TODEVICE);
			RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
			pTxRing->Cell[Index].pNdisPacket = NULL;
		}

		/* prepare TX resource for ATE mode */
		pPacket = pTxRing->Cell[Index].pNextNdisPacket;

		if (pPacket)
		{
			PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr1, pTxD->SDLen1, RTMP_PCI_DMA_TODEVICE);
			RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
			pTxRing->Cell[Index].pNextNdisPacket = NULL;
		}

#ifdef RT_BIG_ENDIAN
		RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
		WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, FALSE, TYPE_TXD);
#endif
	}
#endif /* RTMP_MAC_PCI */






#ifdef CONFIG_AP_SUPPORT
#ifdef RTMP_MAC_PCI
	APStop(pAd);
#endif /* RTMP_MAC_PCI */
#endif /* CONFIG_AP_SUPPORT */

	AsicSetWPDMA(pAd, PDMA_TX_RX, 1);
#ifdef RTMP_MAC_PCI
	RTMP_ASIC_INTERRUPT_ENABLE(pAd);
#endif /* RTMP_MAC_PCI */

	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);

	if(ATECtrl->Mode & fATE_TXCONT_ENABLE){
		DBGPRINT(RT_DEBUG_OFF, ("%s ,Stop Continuous Tx\n",__FUNCTION__));	
		AsicStopContinousTx(pAd);
	}

	if((ATECtrl->Mode & fATE_TXCARRSUPP_ENABLE)||
	(ATECtrl->Mode & fATE_TXCARR_ENABLE)){
		DBGPRINT(RT_DEBUG_OFF, ("%s ,Stop Tx Carrier Test\n",__FUNCTION__));
    	AsicSetTxToneTest(pAd, 0, 0);
	}
	ATECtrl->Mode = ATE_START;
	{
		UINT32 Value = 0;
		ATECtrl->rmac_pcr1 = 0;
		RTMP_IO_READ32(pAd, AGG_PCR1, &ATECtrl->rmac_pcr1);
		Value = ATECtrl->rmac_pcr1;
		Value &= 0x0FFFFFFF;
		Value |= 0x10000000;
		RTMP_IO_WRITE32(pAd, AGG_PCR1, Value);
	}

        {
                UINT32 Value = 0;
                ATECtrl->rmac_pcr = 0;
                RTMP_IO_READ32(pAd, AGG_PCR, &ATECtrl->rmac_pcr);
                RTMP_IO_WRITE32(pAd, AGG_PCR, Value);
        }

	return Ret;
}


static INT32 MT_ATEStop(RTMP_ADAPTER *pAd)
{
	ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	INT32 Ret = 0;
#ifdef CONFIG_AP_SUPPORT
	INT32 IdBss, MaxNumBss = pAd->ApCfg.BssidNum;
#endif
#ifdef RTMP_MAC_PCI
	UINT32 Index, RingNum;
	RXD_STRUC *pRxD = NULL;
#endif /* RTMP_MAC_PCI */
#ifdef RT_BIG_ENDIAN
	RXD_STRUC *pDestRxD;
	UCHAR RxHwInfo[RXD_SIZE];
#endif



	DBGPRINT(RT_DEBUG_OFF, ("%s\n", __FUNCTION__));
#ifdef SINGLE_SKU_V2
	pAd->SKUEn = 1;
#endif
#ifdef RTMP_MAC_PCI
	/* Polling TX/RX path until packets empty */
	MTPciPollTxRxEmpty(pAd);
#endif

	AsicSetMacTxRx(pAd, ASIC_MAC_RXV, FALSE);

	NICInitializeAdapter(pAd, TRUE);

#ifdef RTMP_MAC_PCI

	for (RingNum = 0; RingNum < NUM_OF_RX_RING; RingNum++)
	{
		for (Index = 0; Index < RX_RING_SIZE; Index++)
		{
#ifdef RT_BIG_ENDIAN
			pDestRxD = (RXD_STRUC *)pAd->RxRing[0].Cell[Index].AllocVa;
			NdisMoveMemory(&RxHwInfo[0], pDestRxD, RXD_SIZE);
			pRxD = (RXD_STRUC *)&RxHwInfo[0];
			RTMPDescriptorEndianChange((PUCHAR)pRxD, TYPE_RXD);
#else
			/* Point to Rx indexed rx ring descriptor */
			pRxD = (RXD_STRUC *)pAd->RxRing[0].Cell[Index].AllocVa;
#endif
			pRxD->DDONE = 0;

#ifdef RT_BIG_ENDIAN
			RTMPDescriptorEndianChange((PUCHAR)pRxD, TYPE_RXD);
			WriteBackToDescriptor((PUCHAR)pDestRxD, (PUCHAR)pRxD, FALSE, TYPE_RXD);
#endif
		}
	}

#endif /* RTMP_MAC_PCI */

/* if usb  call this two function , FW will hang~~ */
#ifdef RTMP_MAC_PCI
	NICReadEEPROMParameters(pAd, NULL);
	NICInitAsicFromEEPROM(pAd);
#endif /* RTMP_MAC_PCI */

	AsicSetRxFilter(pAd);


#ifdef RTMP_MAC_PCI
	RTMP_IRQ_ENABLE(pAd);
#endif /* RTMP_MAC_PCI */
	RTMPEnableRxTx(pAd);

	AsicSwitchChannel(pAd, pAd->CommonCfg.Channel, FALSE);

#ifdef CONFIG_AP_SUPPORT
#ifdef RTMP_MAC_PCI
	APStartUp(pAd);
#endif /* RTMP_MAC_PCI */
#endif /* CONFIG_AP_SUPPORT */
	RTMP_OS_NETDEV_START_QUEUE(pAd->net_dev);

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		if (MaxNumBss > MAX_MBSSID_NUM(pAd))
			MaxNumBss = MAX_MBSSID_NUM(pAd);

		/*  first IdBss must not be 0 (BSS0), must be 1 (BSS1) */
		for (IdBss = FIRST_MBSSID; IdBss < MAX_MBSSID_NUM(pAd); IdBss++)
		{
			if (pAd->ApCfg.MBSSID[IdBss].wdev.if_dev)
				RTMP_OS_NETDEV_START_QUEUE(pAd->ApCfg.MBSSID[IdBss].wdev.if_dev);
		}
	}
#endif

	RTMP_OS_EXIT_COMPLETION(&ATECtrl->cmd_done);
	ATECtrl->Mode = ATE_STOP;

	if ((MTK_REV_GTE(pAd, MT7603, MT7603E1)) ||
		(MTK_REV_GTE(pAd, MT7628, MT7628E1)))
	{
		UINT32 Value;
		RTMP_IO_READ32(pAd, CR_RFINTF_00, &Value);
		Value &= ~CR_RFINTF_CAL_NSS_MASK;
		Value |= CR_RFINTF_CAL_NSS(0x0);
		RTMP_IO_WRITE32(pAd, CR_RFINTF_00, Value);  
	}	

	RTMP_IO_WRITE32(pAd, AGG_PCR1, ATECtrl->rmac_pcr1);
	RTMP_IO_WRITE32(pAd, AGG_PCR, ATECtrl->rmac_pcr);
	return Ret;
}


#ifdef RTMP_PCI_SUPPORT
static INT32 MT_ATESetupFrame(RTMP_ADAPTER *pAd, UINT32 TxIdx)
{
	ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	RTMP_TX_RING *pTxRing = &pAd->TxRing[QID_AC_BE];
	PUCHAR pDMAHeaderBufVA = (PUCHAR)pTxRing->Cell[TxIdx].DmaBuf.AllocVa;
	TXD_STRUC *pTxD;
	MAC_TX_INFO Info;
	UINT8 TXWISize = pAd->chipCap.TXWISize;
	PNDIS_PACKET pPacket = NULL;
	HTTRANSMIT_SETTING Transmit;
	TX_BLK TxBlk;
	INT32 Ret = 0;
#ifdef RT_BIG_ENDIAN
	TXD_STRUC *pDestTxD;
	UCHAR TxHwInfo[TXD_SIZE];
#endif /* RT_BIG_ENDIAN */

	ATECtrl->HLen = LENGTH_802_11;

	NdisZeroMemory(&Transmit, sizeof(Transmit));
	NdisZeroMemory(&TxBlk, sizeof(TxBlk));
	
	/* Fill Mac Tx info */
	NdisZeroMemory(&Info, sizeof(Info));

	/* LMAC queue index (AC0) */
 	Info.q_idx = 1;

	Info.WCID = 0;
	Info.hdr_len = ATECtrl->HLen;
	Info.hdr_pad = 0;

	Info.BM = IS_BM_MAC_ADDR(ATECtrl->Addr1);
	
	/* no ack */
	Info.Ack = 0;

	Info.bss_idx = 0;

	/*  no frag */
	Info.FRAG = 0;

	/* no protection */
	Info.prot = 0;

	Info.Length = ATECtrl->TxLength;

    /* TX Path setting */
    Info.AntPri = 0;
    Info.SpeEn = 0;
    switch (ATECtrl->TxAntennaSel) {
        case 0: /* Both */
           Info.AntPri = 0;
           Info.SpeEn = 1;
           break;
        case 1: /* TX0 */
           Info.AntPri = 0;
           Info.SpeEn = 0;
           break;
        case 2: /* TX1 */
           Info.AntPri = 2; //b'010
           Info.SpeEn = 0;
           break;
    }

	/* Fill Transmit setting */
	Transmit.field.MCS = ATECtrl->Mcs;
	Transmit.field.BW = ATECtrl->BW;
	Transmit.field.ShortGI = ATECtrl->Sgi;
	Transmit.field.STBC = ATECtrl->Stbc;
	Transmit.field.MODE = ATECtrl->PhyMode;

	if (ATECtrl->PhyMode == MODE_CCK)
	{
		Info.Preamble = LONG_PREAMBLE;

		/*
			Short preamble mapping
			UCHAR tmi_rate_map_cck_sp[]={
				TMI_TX_RATE_CCK_2M_SP,
				TMI_TX_RATE_CCK_2M_SP,
				TMI_TX_RATE_CCK_5M_SP,
				TMI_TX_RATE_CCK_11M_SP,
			};
		*/
		if (ATECtrl->Mcs == 9)
		{
			Transmit.field.MCS = 1;
			Info.Preamble = SHORT_PREAMBLE;	
		}
		else if (ATECtrl->Mcs == 10)
		{
			Transmit.field.MCS = 2;
			Info.Preamble = SHORT_PREAMBLE;	
		}
		else if (ATECtrl->Mcs == 11)
		{
			Transmit.field.MCS = 3;
			Info.Preamble = SHORT_PREAMBLE;	
		}
	} 

	write_tmac_info(pAd, pDMAHeaderBufVA, &Info, &Transmit);

	NdisMoveMemory(pDMAHeaderBufVA + TXWISize, ATECtrl->TemplateFrame, ATECtrl->HLen);
	NdisMoveMemory(pDMAHeaderBufVA + TXWISize + 4, ATECtrl->Addr1, MAC_ADDR_LEN);
	NdisMoveMemory(pDMAHeaderBufVA + TXWISize + 10, ATECtrl->Addr2, MAC_ADDR_LEN);
	NdisMoveMemory(pDMAHeaderBufVA + TXWISize + 16, ATECtrl->Addr3, MAC_ADDR_LEN);

#ifdef RT_BIG_ENDIAN
	RTMPFrameEndianChange(pAd, (((PUCHAR)pDMAHeaderBufVA) + TXWISize), DIR_READ, FALSE);
#endif /* RT_BIG_ENDIAN */

	pPacket = ATEPayloadInit(pAd, TxIdx);

	if (pPacket == NULL)
	{
		ATECtrl->TxCount = 0;
		DBGPRINT_ERR(("%s : fail to init frame payload.\n", __FUNCTION__));
		return -1;
	}

	pTxRing->Cell[TxIdx].pNdisPacket = pPacket;

	pTxD = (TXD_STRUC *)pTxRing->Cell[TxIdx].AllocVa;
#ifndef RT_BIG_ENDIAN
	pTxD = (TXD_STRUC *)pTxRing->Cell[TxIdx].AllocVa;
#else
    pDestTxD  = (TXD_STRUC *)pTxRing->Cell[TxIdx].AllocVa;
	NdisMoveMemory(&TxHwInfo[0], (UCHAR *)pDestTxD, TXD_SIZE);
	pTxD = (TXD_STRUC *)&TxHwInfo[0];
#endif
	TxBlk.SrcBufLen = GET_OS_PKT_LEN(ATECtrl->pAtePacket[TxIdx]);
	TxBlk.pSrcBufData = (PUCHAR)ATECtrl->AteAllocVa[TxIdx];

	NdisZeroMemory(pTxD, TXD_SIZE);
	/* build Tx descriptor */
	pTxD->SDPtr0 = RTMP_GetPhysicalAddressLow(pTxRing->Cell[TxIdx].DmaBuf.AllocPa);
	pTxD->SDLen0 = TXWISize + ATECtrl->HLen;
	pTxD->LastSec0 = 0;
	pTxD->SDPtr1 = PCI_MAP_SINGLE(pAd, &TxBlk, 0, 1, RTMP_PCI_DMA_TODEVICE);
	pTxD->SDLen1 = GET_OS_PKT_LEN(ATECtrl->pAtePacket[TxIdx]);
	pTxD->LastSec1 = 1;
	pTxD->DMADONE = 0;

#ifdef RT_BIG_ENDIAN
	MTMacInfoEndianChange(pAd, pDMAHeaderBufVA, TYPE_TMACINFO, sizeof(TMAC_TXD_L));
	RTMPFrameEndianChange(pAd, (((PUCHAR)pDMAHeaderBufVA) + TXWISize), DIR_WRITE, FALSE);
	RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
	WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, FALSE, TYPE_TXD);
#endif

	return Ret;
}
#endif






static INT32 MT_ATEStartTx(RTMP_ADAPTER *pAd)
{
	ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	UINT32 Value = 0;
	INT32 Ret = 0;
#ifdef RTMP_PCI_SUPPORT
	UINT32 Index;
	UINT32 TxIdx = 0;
	RTMP_TX_RING *pTxRing = &pAd->TxRing[QID_AC_BE];
#endif /* RTMP_MAC_PCI */
#ifdef CONFIG_AP_SUPPORT
	INT32 IdBss, MaxNumBss = pAd->ApCfg.BssidNum;
#endif


	DBGPRINT(RT_DEBUG_OFF, ("%s\n", __FUNCTION__));
	/* TxRx switch workaround */
	AsicSetRxPath(pAd, 0);
	AsicSetMacTxRx(pAd, ASIC_MAC_RX, TRUE);
	RtmpusecDelay(10000);
	AsicSetMacTxRx(pAd, ASIC_MAC_RX, FALSE);

	CmdChannelSwitch(pAd, ATECtrl->ControlChl, ATECtrl->Channel, ATECtrl->BW, 
							pAd->CommonCfg.TxStream, pAd->CommonCfg.RxStream);




	AsicSetMacTxRx(pAd, ASIC_MAC_RX_RXV, FALSE);

	/*   Stop send TX packets */
	RTMP_OS_NETDEV_STOP_QUEUE(pAd->net_dev);

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		if (MaxNumBss > MAX_MBSSID_NUM(pAd))
			MaxNumBss = MAX_MBSSID_NUM(pAd);

		/*  first IdBss must not be 0 (BSS0), must be 1 (BSS1) */
		for (IdBss = FIRST_MBSSID; IdBss < MAX_MBSSID_NUM(pAd); IdBss++)
		{
			if (pAd->ApCfg.MBSSID[IdBss].wdev.if_dev)
				RTMP_OS_NETDEV_STOP_QUEUE(pAd->ApCfg.MBSSID[IdBss].wdev.if_dev);
		}
	}
#endif

	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DEQUEUEPACKET);
	
	/*  Disable PDMA */
	AsicSetWPDMA(pAd, PDMA_TX, 0);

	/* Polling TX/RX path until packets empty */
#ifdef RTMP_PCI_SUPPORT
	MTPciPollTxRxEmpty(pAd);
#endif /* RTMP_MAC_PCI */

	/* Turn on RX again if set before */
	if (ATECtrl->Mode & ATE_RXFRAME)
		AsicSetMacTxRx(pAd, ASIC_MAC_RX_RXV, TRUE);

	RTMP_OS_NETDEV_START_QUEUE(pAd->net_dev);


#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		if (MaxNumBss > MAX_MBSSID_NUM(pAd))
			MaxNumBss = MAX_MBSSID_NUM(pAd);

		/*  first IdBss must not be 0 (BSS0), must be 1 (BSS1) */
		for (IdBss = FIRST_MBSSID; IdBss < MAX_MBSSID_NUM(pAd); IdBss++)
		{
			if (pAd->ApCfg.MBSSID[IdBss].wdev.if_dev)
				RTMP_OS_NETDEV_START_QUEUE(pAd->ApCfg.MBSSID[IdBss].wdev.if_dev);
		}
	}
#endif
	
	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DEQUEUEPACKET);


#ifdef RTMP_PCI_SUPPORT
	RTMP_IO_READ32(pAd, pTxRing->hw_didx_addr, &pTxRing->TxDmaIdx);
	pTxRing->TxSwFreeIdx = pTxRing->TxDmaIdx;
	pTxRing->TxCpuIdx = pTxRing->TxDmaIdx;
	RTMP_IO_WRITE32(pAd, pTxRing->hw_cidx_addr, pTxRing->TxCpuIdx);

    if (ATECtrl->bQAEnabled != TRUE) /* reset in start tx when iwpriv */
	    ATECtrl->TxDoneCount = 0;

	for (Index = 0; Index < TX_RING_SIZE; Index++)
	{
		if (ATEPayloadAlloc(pAd, Index) != (NDIS_STATUS_SUCCESS))
		{
			return (NDIS_STATUS_FAILURE);
		}
		
		pTxRing->Cell[Index].pNdisPacket = ATECtrl->pAtePacket[Index];
	}

	for (Index = 0; (Index < TX_RING_SIZE) && (Index < ATECtrl->TxCount); Index++)
	{
		DBGPRINT(RT_DEBUG_OFF, ("Index = %d, ATECtrl->TxCount = %d\n", Index, ATECtrl->TxCount));
		TxIdx = pTxRing->TxCpuIdx;

		if (MT_ATESetupFrame(pAd, TxIdx) != 0)
		{
			return (NDIS_STATUS_FAILURE);
		}

		if (((Index + 1) < TX_RING_SIZE) && (Index < ATECtrl->TxCount))
		{
			INC_RING_INDEX(pTxRing->TxCpuIdx, TX_RING_SIZE);
		}
	}
#endif /* RTMP_MAC_PCI */
	/* Enable PDMA */
	AsicSetWPDMA(pAd, PDMA_TX_RX, 1);

	ATECtrl->Mode |= ATE_TXFRAME;
#ifdef RTMP_PCI_SUPPORT
	RTMP_IO_WRITE32(pAd, pTxRing->hw_cidx_addr, pTxRing->TxCpuIdx);
#endif /* RTMP_MAC_PCI */




	/* Low temperature high rate EVM degrade Patch v2 */	
	if ((MTK_REV_GTE(pAd, MT7603, MT7603E1)) ||
		(MTK_REV_GTE(pAd, MT7628, MT7628E1)))
	{
		if (ATECtrl->TxAntennaSel == 0)
		{
			RTMP_IO_READ32(pAd, CR_RFINTF_00, &Value);
			Value &= ~CR_RFINTF_CAL_NSS_MASK;
			Value |= CR_RFINTF_CAL_NSS(0x0);
			RTMP_IO_WRITE32(pAd, CR_RFINTF_00, Value);  
		}
		else if (ATECtrl->TxAntennaSel == 1)
		{
			RTMP_IO_READ32(pAd, CR_RFINTF_00, &Value);
			Value &= ~CR_RFINTF_CAL_NSS_MASK;
			Value |= CR_RFINTF_CAL_NSS(0x0);
			RTMP_IO_WRITE32(pAd, CR_RFINTF_00, Value);  
		}
		else if (ATECtrl->TxAntennaSel == 2)
		{
			RTMP_IO_READ32(pAd, CR_RFINTF_00, &Value);
			Value &= ~CR_RFINTF_CAL_NSS_MASK;
			Value |= CR_RFINTF_CAL_NSS(0x1);
			RTMP_IO_WRITE32(pAd, CR_RFINTF_00, Value);  
		}
	}

	return Ret;
}


static INT32 MT_ATEStartRx(RTMP_ADAPTER *pAd)
{
	ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	INT32 Ret = 0, Value;
#ifdef CONFIG_AP_SUPPORT
	INT32 IdBss, MaxNumBss = pAd->ApCfg.BssidNum;
#endif

	DBGPRINT(RT_DEBUG_OFF, ("%s\n", __FUNCTION__));


#ifdef RTMP_MAC_PCI
	AsicSetMacTxRx(pAd, ASIC_MAC_RX_RXV, FALSE);
#endif /* RTMP_MAC_PCI */



	/*   Stop send TX packets */
	RTMP_OS_NETDEV_STOP_QUEUE(pAd->net_dev);

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		if (MaxNumBss > MAX_MBSSID_NUM(pAd))
			MaxNumBss = MAX_MBSSID_NUM(pAd);

		/*  first IdBss must not be 0 (BSS0), must be 1 (BSS1) */
		for (IdBss = FIRST_MBSSID; IdBss < MAX_MBSSID_NUM(pAd); IdBss++)
		{
			if (pAd->ApCfg.MBSSID[IdBss].wdev.if_dev)
				RTMP_OS_NETDEV_STOP_QUEUE(pAd->ApCfg.MBSSID[IdBss].wdev.if_dev);
		}
	}
#endif

	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DEQUEUEPACKET);

	AsicSetWPDMA(pAd, PDMA_TX, 0);

#ifdef RTMP_MAC_PCI
	/* Polling TX/RX path until packets empty */
	MTPciPollTxRxEmpty(pAd);
#endif
	
	RTMP_OS_NETDEV_START_QUEUE(pAd->net_dev);

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		if (MaxNumBss > MAX_MBSSID_NUM(pAd))
			MaxNumBss = MAX_MBSSID_NUM(pAd);

		/*  first IdBss must not be 0 (BSS0), must be 1 (BSS1) */
		for (IdBss = FIRST_MBSSID; IdBss < MAX_MBSSID_NUM(pAd); IdBss++)
		{
			if (pAd->ApCfg.MBSSID[IdBss].wdev.if_dev)
				RTMP_OS_NETDEV_START_QUEUE(pAd->ApCfg.MBSSID[IdBss].wdev.if_dev);
		}
	}
#endif
	
	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DEQUEUEPACKET);


	/* Turn on TX again if set before */
	if (ATECtrl->Mode & ATE_TXFRAME)
	{
		AsicSetMacTxRx(pAd, ASIC_MAC_TX, TRUE);
	}
    /* reset counter when iwpriv only */
    if (ATECtrl->bQAEnabled != TRUE)
    {
        ATECtrl->RxTotalCnt = 0;
    }
	pAd->WlanCounters.FCSErrorCount.u.LowPart = 0;

	RTMP_IO_READ32(pAd, RMAC_RFCR, &Value);
	Value |= RM_FRAME_REPORT_EN;
	RTMP_IO_WRITE32(pAd, RMAC_RFCR, Value);

#ifdef RTMP_MAC_PCI
	AsicSetMacTxRx(pAd, ASIC_MAC_RX_RXV, TRUE);
#endif /* RTMP_MAC_PCI */



	/* Enable PDMA */
	AsicSetWPDMA(pAd, PDMA_TX_RX, 1);


	ATECtrl->Mode |= ATE_RXFRAME;


printk("<-----iverson MT_ATEStartRx  \n ");

	return Ret;
}


static INT32 MT_ATEStopTx(RTMP_ADAPTER *pAd, UINT32 Mode)
{
	ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	INT32 Ret = 0;
#ifdef RTMP_MAC_PCI
	UINT32 Index;
	TXD_STRUC *pTxD = NULL;
	RTMP_TX_RING *pTxRing = &pAd->TxRing[QID_AC_BE];
#ifdef RT_BIG_ENDIAN
    TXD_STRUC *pDestTxD = NULL;
	UCHAR tx_hw_info[TXD_SIZE];
#endif /* RT_BIG_ENDIAN */
#endif /* RTMP_MAC_PCI */
	DBGPRINT(RT_DEBUG_OFF, ("%s\n", __FUNCTION__));

	if (Mode == ATE_TXCARR)
	{



	}
	else if (Mode == ATE_TXCARRSUPP)
	{


	}
	else if ((Mode & ATE_TXFRAME) || (Mode == ATE_STOP))
	{
		if (Mode == ATE_TXCONT)
		{


		}

		/*  Disable PDMA */
		AsicSetWPDMA(pAd, PDMA_TX_RX, 0);
#ifdef RTMP_MAC_PCI

		for (Index = 0; Index < TX_RING_SIZE; Index++)
		{
			PNDIS_PACKET pPacket;

#ifndef RT_BIG_ENDIAN
		    pTxD = (TXD_STRUC *)pAd->TxRing[QID_AC_BE].Cell[Index].AllocVa;
#else
    		pDestTxD = (TXD_STRUC *)pAd->TxRing[QID_AC_BE].Cell[Index].AllocVa;
			NdisMoveMemory(&tx_hw_info[0], (UCHAR *)pDestTxD, TXD_SIZE);
			pTxD = (TXD_STRUC *)&tx_hw_info[0];
    		RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#endif /* !RT_BIG_ENDIAN */
			pTxD->DMADONE = 0;
			pPacket = pTxRing->Cell[Index].pNdisPacket;

			if (pPacket)
			{
				PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr0, pTxD->SDLen0, RTMP_PCI_DMA_TODEVICE);
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
			}

			/* Always assign pNdisPacket as NULL after clear */
			pTxRing->Cell[Index].pNdisPacket = NULL;

			pPacket = pTxRing->Cell[Index].pNextNdisPacket;

			if (pPacket)
			{
				PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr1, pTxD->SDLen1, RTMP_PCI_DMA_TODEVICE);
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
			}

			/* Always assign pNextNdisPacket as NULL after clear */
			pTxRing->Cell[Index].pNextNdisPacket = NULL;
#ifdef RT_BIG_ENDIAN
			RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
			WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, FALSE, TYPE_TXD);
#endif /* RT_BIG_ENDIAN */
		}
    
#endif /* RTMP_MAC_PCI */


		ATECtrl->Mode &= ~ATE_TXFRAME;

		/* Enable PDMA */
		AsicSetWPDMA(pAd, PDMA_TX_RX, 1);
	}

	return Ret;
}


static INT32 MT_ATEStopRx(RTMP_ADAPTER *pAd)
{
	ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	INT32 Ret = 0;
	DBGPRINT(RT_DEBUG_OFF, ("%s\n", __FUNCTION__));

	Ret = AsicSetMacTxRx(pAd, ASIC_MAC_RX_RXV, FALSE);

    ATECtrl->Mode &= ~ATE_RXFRAME;


	return Ret;
}


static INT32 MT_ATESetTxAntenna(RTMP_ADAPTER *pAd, CHAR Ant)
{

	ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	INT32 Ret = 0;
	UINT32 Value;

	DBGPRINT(RT_DEBUG_OFF, ("%s\n", __FUNCTION__));
    /* 0: All 1:TX0 2:TX1 */
    ATECtrl->TxAntennaSel = Ant;

	if ((MTK_REV_GTE(pAd, MT7603, MT7603E1)) ||
		(MTK_REV_GTE(pAd, MT7628, MT7628E1))||
		(MTK_REV_GTE(pAd, MT7628, MT7603E2)))
	{
		if (ATECtrl->TxAntennaSel == 0)
		{
			RTMP_IO_READ32(pAd, CR_RFINTF_00, &Value);
			Value &= ~CR_RFINTF_CAL_NSS_MASK;
			Value |= CR_RFINTF_CAL_NSS(0x0);
			RTMP_IO_WRITE32(pAd, CR_RFINTF_00, Value);
			/* Tx both patch, ePA/eLNA/, iPA/eLNA, iPA/iLNA */
			{

				UINT32 RemapBase, RemapOffset;
				UINT32 RestoreValue;
				UINT32 value;
				RTMP_IO_READ32(pAd, MCU_PCIE_REMAP_2, &RestoreValue);
				RemapBase = GET_REMAP_2_BASE(0x81060008) << 19;
				RemapOffset = GET_REMAP_2_OFFSET(0x81060008);
				RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RemapBase);
				if ((MTK_REV_GTE(pAd, MT7603, MT7603E1))||
					(MTK_REV_GTE(pAd, MT7603, MT7603E2))){
					value = 0x04852390;
					RTMP_IO_WRITE32(pAd, 0x80000 + RemapOffset, value);
				}else if(MTK_REV_GTE(pAd, MT7628, MT7628E1)){
					value = 0x00489523;
					RTMP_IO_WRITE32(pAd, 0x80000 + RemapOffset, value);
				}
				RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RestoreValue);

			}
		}
		else if (ATECtrl->TxAntennaSel == 1)
		{
			RTMP_IO_READ32(pAd, CR_RFINTF_00, &Value);
			Value &= ~CR_RFINTF_CAL_NSS_MASK;
			Value |= CR_RFINTF_CAL_NSS(0x0);
			RTMP_IO_WRITE32(pAd, CR_RFINTF_00, Value);

			/* Tx0 patch, ePA/eLNA/, iPA/eLNA, iPA/iLNA */
			{
				UINT32 RemapBase, RemapOffset;
				UINT32 RestoreValue;
				UINT32 value;
				RTMP_IO_READ32(pAd, MCU_PCIE_REMAP_2, &RestoreValue);
				RemapBase = GET_REMAP_2_BASE(0x81060008) << 19;
				RemapOffset = GET_REMAP_2_OFFSET(0x81060008);
				RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RemapBase);
				if ((MTK_REV_GTE(pAd, MT7603, MT7603E1))||
					(MTK_REV_GTE(pAd, MT7603, MT7603E2))){
					value = 0x04852390;
					RTMP_IO_WRITE32(pAd, 0x80000 + RemapOffset, value);
				}else if(MTK_REV_GTE(pAd, MT7628, MT7628E1)){
					value = 0x00489523;
					RTMP_IO_WRITE32(pAd, 0x80000 + RemapOffset, value);
				}
				RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RestoreValue);
			}
		}
		else if (ATECtrl->TxAntennaSel == 2)
		{
			RTMP_IO_READ32(pAd, CR_RFINTF_00, &Value);
			Value &= ~CR_RFINTF_CAL_NSS_MASK;
			Value |= CR_RFINTF_CAL_NSS(0x1);
			RTMP_IO_WRITE32(pAd, CR_RFINTF_00, Value);
			/* Tx1 patch, ePA/eLNA/, iPA/eLNA, iPA/iLNA */
			{
				UINT32 RemapBase, RemapOffset;
				UINT32 RestoreValue;
				UINT32 value;
				RTMP_IO_READ32(pAd, MCU_PCIE_REMAP_2, &RestoreValue);
				RemapBase = GET_REMAP_2_BASE(0x81060008) << 19;
				RemapOffset = GET_REMAP_2_OFFSET(0x81060008);
				RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RemapBase);
				if ((MTK_REV_GTE(pAd, MT7603, MT7603E1))||
					(MTK_REV_GTE(pAd, MT7603, MT7603E2))){
					value = 0x04856790;
					RTMP_IO_WRITE32(pAd, 0x80000 + RemapOffset, value);
				}else if(MTK_REV_GTE(pAd, MT7628, MT7628E1)){
					value = 0x00489567;
					RTMP_IO_WRITE32(pAd, 0x80000 + RemapOffset, value);
				}	
				RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RestoreValue);
			}
		}
	}

	return Ret;
}


static INT32 MT_ATESetRxAntenna(RTMP_ADAPTER *pAd, CHAR Ant)
{
	ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	INT32 Ret = 0;

	DBGPRINT(RT_DEBUG_OFF, ("%s\n", __FUNCTION__));
    ATECtrl->RxAntennaSel = Ant;
    /* set RX path */
    AsicSetRxPath(pAd, (UINT32)ATECtrl->RxAntennaSel);
	return Ret;
}


static INT32 MT_ATESetTxFreqOffset(RTMP_ADAPTER *pAd, UINT32 FreqOffset)
{
	ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	INT32 Ret = 0;
    ATECtrl->RFFreqOffset = FreqOffset;
	DBGPRINT(RT_DEBUG_OFF, ("%s\n", __FUNCTION__));

    AsicSetRfFreqOffset(pAd, ATECtrl->RFFreqOffset);

	return Ret;
}

static INT32 MT_ATESetChannel(RTMP_ADAPTER *pAd, INT16 Value)
{
	ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	INT32 Ret = 0;

	DBGPRINT(RT_DEBUG_OFF, ("%s\n", __FUNCTION__));

	CmdChannelSwitch(pAd, ATECtrl->ControlChl, ATECtrl->Channel, ATECtrl->BW, 
							pAd->CommonCfg.TxStream, pAd->CommonCfg.RxStream);	

	return Ret;
}


static INT32 MT_ATESetBW(RTMP_ADAPTER *pAd, INT16 Value)
{
	//ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	INT32 Ret = 0;
    UINT32 val = 0;

	DBGPRINT(RT_DEBUG_OFF, ("%s\n", __FUNCTION__));

	RTMP_IO_READ32(pAd, AGG_BWCR, &val);
	val &= (~0x0c);
	switch (Value)
	{
		case BW_20:
			val |= (0);
			break;
		case BW_40:
			val |= (0x1 << 2);
			break;
		case BW_80:
			val |= (0x2 << 2);
			break;
	}
	RTMP_IO_WRITE32(pAd, AGG_BWCR, val);
    /* TODO: check CMD_CH_PRIV_ACTION_BW_REQ */
	//CmdChPrivilege(pAd, CMD_CH_PRIV_ACTION_BW_REQ, ATECtrl->ControlChl, ATECtrl->Channel,
	//					ATECtrl->BW, pAd->CommonCfg.TxStream, pAd->CommonCfg.RxStream);


	return Ret;
}


static INT32 MT_ATESampleRssi(RTMP_ADAPTER *pAd, RX_BLK *RxBlk)
{
	ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	INT32 Ret = 0;

	if (RxBlk->rx_signal.raw_rssi[0] != 0)
	{
		ATECtrl->LastRssi0	= ConvertToRssi(pAd,
			(struct raw_rssi_info *)(&RxBlk->rx_signal.raw_rssi[0]), RSSI_IDX_0);

		ATECtrl->AvgRssi0X8 = (ATECtrl->AvgRssi0X8 - ATECtrl->AvgRssi0)
									+ ATECtrl->LastRssi0;
		ATECtrl->AvgRssi0 = ATECtrl->AvgRssi0X8 >> 3;
	}

	if (RxBlk->rx_signal.raw_rssi[1] != 0)
	{
		ATECtrl->LastRssi1 = ConvertToRssi(pAd,
			(struct raw_rssi_info *)(&RxBlk->rx_signal.raw_rssi[0]), RSSI_IDX_1);

		ATECtrl->AvgRssi1X8 = (ATECtrl->AvgRssi1X8 - ATECtrl->AvgRssi1)
									+ ATECtrl->LastRssi1;
		ATECtrl->AvgRssi1 = ATECtrl->AvgRssi1X8 >> 3;
	}

	ATECtrl->LastSNR0 = RxBlk->rx_signal.raw_snr[0];;
	ATECtrl->LastSNR1 = RxBlk->rx_signal.raw_snr[1];

	ATECtrl->NumOfAvgRssiSample++;

	return Ret;
}

static INT32 MT_ATESetAIFS(RTMP_ADAPTER *pAd, CHAR Value)
{
	//ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	INT32 Ret = 0;
    /* Test mode use AC0 for TX */
    AsicSetWmmParam(pAd, WMM_PARAM_AC_0, WMM_PARAM_AIFSN, Value);
    return Ret;
}

static INT32 MT_ATESetTSSI(RTMP_ADAPTER *pAd, CHAR WFSel, CHAR Setting)
{
	//ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	INT32 Ret = 0;
    Ret = AsicSetTSSI(pAd, Setting, WFSel);
	return Ret;
}

static INT32 MT_ATESetDPD(RTMP_ADAPTER *pAd, CHAR WFSel, CHAR Setting)
{
    /* !!TEST MODE ONLY!! Normal Mode control by FW and Never disable */
    /* WF0 = 0, WF1 = 1, WF ALL = 2 */
	//ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	INT32 Ret = 0;
    Ret = AsicSetDPD(pAd, Setting, WFSel);
    return Ret;
}

static INT32 MT_ATEStartTxTone(RTMP_ADAPTER *pAd, UINT32 Mode)
{
	//ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	INT32 Ret = 0;
    DBGPRINT(RT_DEBUG_OFF, ("%s\n", __FUNCTION__));
	AsicSetTxToneTest(pAd, 1, Mode);
	return Ret;
}
static INT32 MT_ATESetTxTonePower(RTMP_ADAPTER *pAd, INT32 pwr1, INT32 pwr2)
{
	//ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	INT32 Ret = 0;
    DBGPRINT(RT_DEBUG_OFF, ("%s, pwr1:%d, pwr2:%d\n", __FUNCTION__, pwr1, pwr2));
	AsicSetTxTonePower(pAd, pwr1, pwr2);
	return Ret;
}

static INT32 MT_ATEStopTxTone(RTMP_ADAPTER *pAd)
{
	//ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	INT32 Ret = 0;
    DBGPRINT(RT_DEBUG_OFF, ("%s\n", __FUNCTION__));
	AsicSetTxToneTest(pAd, 0, 0);
    return Ret;
}

static INT32 MT_ATEStartContinousTx(RTMP_ADAPTER *pAd, CHAR WFSel)
{
	ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	INT32 Ret = 0;
    DBGPRINT(RT_DEBUG_OFF, ("%s\n", __FUNCTION__));
    AsicStartContinousTx(pAd, ATECtrl->PhyMode, ATECtrl->BW, ATECtrl->ControlChl, ATECtrl->Mcs, WFSel);
    return Ret;
}


static INT32 MT_RfRegWrite(RTMP_ADAPTER *pAd, UINT32 WFSel, UINT32 Offset, UINT32 Value)
{
	INT32 Ret = 0;

	Ret = CmdRFRegAccessWrite(pAd, WFSel, Offset, Value);

	return Ret;
}


static INT32 MT_RfRegRead(RTMP_ADAPTER *pAd, UINT32 WFSel, UINT32 Offset, UINT32 *Value)
{
	INT32 Ret = 0;

	Ret = CmdRFRegAccessRead(pAd, WFSel, Offset, Value);

	return Ret;
}


static INT32 MT_GetFWInfo(RTMP_ADAPTER *pAd, UCHAR *FWInfo)
{

	RTMP_CHIP_CAP *cap = &pAd->chipCap;

	memcpy(FWInfo, cap->FWImageName + cap->fw_len - 36, 36);

	return 0;
}


static ATE_OPERATION MT_ATEOp = {
	.ATEStart = MT_ATEStart,
	.ATEStop = MT_ATEStop,
	.StartTx = MT_ATEStartTx,
	.StartRx = MT_ATEStartRx,
	.StopTx = MT_ATEStopTx,
	.StopRx = MT_ATEStopRx,
	.SetTxPower0 = MT_ATESetTxPower0,
	.SetTxPower1 = MT_ATESetTxPower1,
	.SetTxAntenna = MT_ATESetTxAntenna,
	.SetRxAntenna = MT_ATESetRxAntenna,
	.SetTxFreqOffset = MT_ATESetTxFreqOffset,
	.SetChannel = MT_ATESetChannel,
	.SetBW = MT_ATESetBW,
	.SampleRssi = MT_ATESampleRssi,
	.SetAIFS = MT_ATESetAIFS,
	.SetTSSI = MT_ATESetTSSI,
	.SetDPD = MT_ATESetDPD,
	.StartTxTone = MT_ATEStartTxTone,
	.SetTxTonePower = MT_ATESetTxTonePower,
	.StopTxTone = MT_ATEStopTxTone,
	.StartContinousTx = MT_ATEStartContinousTx,
	.RfRegWrite = MT_RfRegWrite,
	.RfRegRead = MT_RfRegRead,
	.GetFWInfo = MT_GetFWInfo,
};


INT32 MT_ATEInit(RTMP_ADAPTER *pAd)
{
	ATE_CTRL *ATECtrl = &pAd->ATECtrl;

	ATECtrl->ATEOp = &MT_ATEOp;

	return 0;
}

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
static VOID MtATEWTBL2Update(RTMP_ADAPTER *pAd, UCHAR wcid)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	union  WTBL_2_DW9 wtbl_2_d9 = {.word = 0};
	UINT32 rate[8]; /* reg_val; */
	UCHAR stbc, bw, nss, preamble; /* , wait_cnt = 0; */

	bw = 2;
	wtbl_2_d9.field.fcap = bw;
	wtbl_2_d9.field.ccbw_sel = bw;
	wtbl_2_d9.field.cbrn = 7; /* change bw as (fcap/2) if rate_idx > 7, temporary code */

	if (ATECtrl->Sgi) {
		wtbl_2_d9.field.g2 = 1;
		wtbl_2_d9.field.g4 = 1;
		wtbl_2_d9.field.g8 = 1;
		wtbl_2_d9.field.g16 = 1;
	} else {
		wtbl_2_d9.field.g2 = 0;
		wtbl_2_d9.field.g4 = 0;
		wtbl_2_d9.field.g8 = 0;
		wtbl_2_d9.field.g16 = 0;
	}

	wtbl_2_d9.field.rate_idx = 0;

	if (ATECtrl->PhyMode == MODE_CCK)
		preamble = SHORT_PREAMBLE;
	else
		preamble = LONG_PREAMBLE;

	stbc = ATECtrl->Stbc;
	nss = 1;
	rate[0] = tx_rate_to_tmi_rate(ATECtrl->PhyMode,
								  ATECtrl->Mcs,
								  nss,
								  stbc,
								  preamble);
	rate[0] &= 0xfff;
	rate[1] = rate[2] = rate[3] = rate[4] = rate[5] = rate[6] = rate[7] = rate[0];
	/* Wtbl2RateTableUpdate(pAd, wcid, wtbl_2_d9.word, rate); */
}
static INT32 MT_ATERestoreInit(RTMP_ADAPTER *pAd)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	INT32 Ret = 0;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: Rx receive for 10000us workaround\n", __func__));
	/* Workaround CR Restore */
	/* Tx/Rx Antenna Setting Restore */
	MtAsicSetRxPath(pAd, 0);
	/* TxRx switch workaround */
	AsicSetMacTxRx(pAd, ASIC_MAC_RX, TRUE);
	RtmpusecDelay(10000);
	AsicSetMacTxRx(pAd, ASIC_MAC_RX, FALSE);
	/* Flag Resotre */
	ATECtrl->did_tx = 0;
	ATECtrl->did_rx = 0;

	return Ret;
}

static INT32 MT_ATESetTxPower0(RTMP_ADAPTER *pAd, CHAR Value)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	INT32 Ret = 0;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __func__));
	ATECtrl->TxPower0 = Value;
	ATECtrl->TxPower1 = Value; /* pAd->EEPROMImage[TX1_G_BAND_TARGET_PWR]; */
	CmdSetTxPowerCtrl(pAd, ATECtrl->Channel);
	return Ret;
}

static INT32 MT_ATESetTxPower1(RTMP_ADAPTER *pAd, CHAR Value)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	INT32 Ret = 0;

	ATECtrl->TxPower1 = Value;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __func__));

	/* Same as Power0 */
	if (ATECtrl->TxPower0 != ATECtrl->TxPower1) {
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: power1 do not same as power0\n", __func__));
		Ret = -1;
	}

	return Ret;
}


static INT32 MT_ATEStart(RTMP_ADAPTER *pAd)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	struct _ATE_IF_OPERATION *if_ops = ATECtrl->ATEIfOps;
	struct _ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	INT32 Ret = 0;
	BOOLEAN bCalFree = 0;
#ifdef CONFIG_AP_SUPPORT
	INT32 IdBss, MaxNumBss = pAd->ApCfg.BssidNum;
	BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[MAIN_MBSSID];
#endif
	INT32 i;

	i = 0;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __func__));
	/*   Stop send TX packets */
	RTMP_OS_NETDEV_STOP_QUEUE(pAd->net_dev);
	/* Reset ATE TX/RX Counter */
	ATECtrl->TxDoneCount = 0;
	ATECtrl->RxTotalCnt = 0;
	ATECtrl->TxLength = 1024;
	ATECtrl->LastRssi0	= 0;
	ATECtrl->AvgRssi0X8 = 0;
	ATECtrl->AvgRssi0 = 0;
	ATECtrl->MaxRssi0 = 0xff;
	ATECtrl->MinRssi0 = 0;
	ATECtrl->LastRssi1	= 0;
	ATECtrl->AvgRssi1X8 = 0;
	ATECtrl->AvgRssi1 = 0;
	ATECtrl->MaxRssi1 = 0xff;
	ATECtrl->MinRssi1 = 0;
	ATECtrl->LastRssi2	= 0;
	ATECtrl->AvgRssi2X8 = 0;
	ATECtrl->AvgRssi2 = 0;
	ATECtrl->MaxRssi2 = 0xff;
	ATECtrl->MinRssi2 = 0;
	ATECtrl->QID = QID_AC_BE;
	ATECtrl->tx_coherent = 0;
	ATECtrl->cmd_expire = RTMPMsecsToJiffies(3000);
	RTMP_OS_INIT_COMPLETION(&ATECtrl->cmd_done);
	ATECtrl->TxPower0 = pAd->EEPROMImage[TX0_G_BAND_TARGET_PWR];
	ATECtrl->TxPower1 = pAd->EEPROMImage[TX1_G_BAND_TARGET_PWR];
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: Init Txpower, Tx0:%x, Tx1:%x\n", __func__, ATECtrl->TxPower0, ATECtrl->TxPower1));
#ifdef CONFIG_QA
	MtAsicGetRxStat(pAd, HQA_RX_RESET_PHY_COUNT);
	ATECtrl->RxMacMdrdyCount = 0;
	ATECtrl->RxMacFCSErrCount = 0;
#endif /* CONFIG_QA */
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		if (MaxNumBss > MAX_MBSSID_NUM(pAd))
			MaxNumBss = MAX_MBSSID_NUM(pAd);

		/*  first IdBss must not be 0 (BSS0), must be 1 (BSS1) */
		for (IdBss = FIRST_MBSSID; IdBss < MAX_MBSSID_NUM(pAd); IdBss++) {
			if (pAd->ApCfg.MBSSID[IdBss].wdev.if_dev)
				RTMP_OS_NETDEV_STOP_QUEUE(pAd->ApCfg.MBSSID[IdBss].wdev.if_dev);
		}
	}
#endif
	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DEQUEUEPACKET);
	AsicSetMacTxRx(pAd, ASIC_MAC_RX, FALSE);
	/*  Disable TX PDMA */
	AsicSetWPDMA(pAd, PDMA_TX, 0);
#ifdef CONFIG_AP_SUPPORT
	APStop(pAd, pMbss, AP_BSS_OPER_ALL);
#endif /* CONFIG_AP_SUPPORT */
#ifdef RTMP_MAC_PCI
	if_ops->init(pAd);
	if_ops->clean_trx_q(pAd);
#endif /* RTMP_MAC_PCI */
	AsicSetWPDMA(pAd, PDMA_TX_RX, 1);
	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);

	/* MT7636 Test Mode Freqency offset restore*/
	if (ATECtrl->en_man_set_freq == 1) {
		if (IS_MT76x6(pAd) || IS_MT7637(pAd)) {
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("MT76x6 Manual Set Frequency Restore\n"));
			RTMP_IO_WRITE32(pAd, FREQ_OFFSET_MANUAL_ENABLE, ATECtrl->en_man_freq_restore);
			RTMP_IO_WRITE32(pAd, FREQ_OFFSET_MANUAL_VALUE, ATECtrl->normal_freq_restore);
		}

		ATECtrl->normal_freq_restore = 0;
		ATECtrl->en_man_freq_restore = 0;
		ATECtrl->en_man_set_freq = 0;
	}

	if (ATECtrl->Mode & fATE_TXCONT_ENABLE) {
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s ,Stop Continuous Tx\n", __func__));
		ATEOp->StopContinousTx(pAd);
	}

	if (ATECtrl->Mode & fATE_TXCARRSUPP_ENABLE) {
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s ,Stop Carrier Suppression Test\n", __func__));
		ATEOp->StopTxTone(pAd);
	}

	ATECtrl->Mode = ATE_START;
	ATECtrl->Mode &= ATE_RXSTOP;
	ATECtrl->en_man_set_freq = 0;
	ATECtrl->normal_freq_restore = 0;
	ATECtrl->en_man_freq_restore = 0;
	ATECtrl->TxDoneCount = 0;
	ATECtrl->RxTotalCnt = 0;
#ifdef CONFIG_QA
	MtAsicGetRxStat(pAd, HQA_RX_RESET_PHY_COUNT);
	MtAsicGetRxStat(pAd, HQA_RX_RESET_MAC_COUNT);
	ATECtrl->RxMacFCSErrCount = 0;
	ATECtrl->RxMacMdrdyCount = 0;
#endif
	MtATEWTBL2Update(pAd, 0);
	return Ret;
}


static INT32 MT_ATEStop(RTMP_ADAPTER *pAd)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	struct _ATE_IF_OPERATION *if_ops = ATECtrl->ATEIfOps;
	INT32 Ret = 0;
#ifdef CONFIG_AP_SUPPORT
	INT32 IdBss, MaxNumBss = pAd->ApCfg.BssidNum;
	BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[MAIN_MBSSID];
#endif
	UCHAR Channel = HcGetRadioChannel(pAd);

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __func__));
	AsicSetMacTxRx(pAd, ASIC_MAC_RXV, FALSE);
	NICInitializeAdapter(pAd);
#ifdef RTMP_MAC_PCI

	if (if_ops->clean_test_rx_frame)
		if_ops->clean_test_rx_frame(pAd);

#endif /* RTMP_MAC_PCI */
	/* if usb  call this two function , FW will hang~~ */

	if (if_ops->clean_trx_q)
		if_ops->clean_trx_q(pAd);

	if (if_ops->ate_leave)
		if_ops->ate_leave(pAd);

	AsicSetRxFilter(pAd);
	RTMPEnableRxTx(pAd);
	hc_reset_radio(pAd);
#ifdef CONFIG_AP_SUPPORT
	APStartUp(pAd, pMbss, AP_BSS_OPER_ALL);
#endif /* CONFIG_AP_SUPPROT  */
	RTMP_OS_NETDEV_START_QUEUE(pAd->net_dev);
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		if (MaxNumBss > MAX_MBSSID_NUM(pAd))
			MaxNumBss = MAX_MBSSID_NUM(pAd);

		/*  first IdBss must not be 0 (BSS0), must be 1 (BSS1) */
		for (IdBss = FIRST_MBSSID; IdBss < MAX_MBSSID_NUM(pAd); IdBss++) {
			if (pAd->ApCfg.MBSSID[IdBss].wdev.if_dev)
				RTMP_OS_NETDEV_START_QUEUE(pAd->ApCfg.MBSSID[IdBss].wdev.if_dev);
		}
	}
#endif

	/* MT7636 Test Mode Freqency offset restore*/
	if (ATECtrl->en_man_set_freq == 1) {
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("MT76x6 Manual Set Frequency Restore\n"));
		RTMP_IO_WRITE32(pAd, FREQ_OFFSET_MANUAL_ENABLE, ATECtrl->en_man_freq_restore);
		RTMP_IO_WRITE32(pAd, FREQ_OFFSET_MANUAL_VALUE, ATECtrl->normal_freq_restore);
		ATECtrl->normal_freq_restore = 0;
		ATECtrl->en_man_freq_restore = 0;
		ATECtrl->en_man_set_freq = 0;
	}

	RTMP_OS_EXIT_COMPLETION(&ATECtrl->cmd_done);
	ATECtrl->Mode = ATE_STOP;

	if ((MTK_REV_GTE(pAd, MT7603, MT7603E1)) ||
		(MTK_REV_GTE(pAd, MT7628, MT7628E1))) {
		UINT32 Value;

		RTMP_IO_READ32(pAd, CR_RFINTF_00, &Value);
		Value &= ~CR_RFINTF_CAL_NSS_MASK;
		Value |= CR_RFINTF_CAL_NSS(0x0);
		RTMP_IO_WRITE32(pAd, CR_RFINTF_00, Value);
	}

	return Ret;
}


#ifdef RTMP_PCI_SUPPORT
static INT32 MT_ATESetupFrame(RTMP_ADAPTER *pAd, UINT32 TxIdx)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	RTMP_TX_RING *pTxRing = &pAd->PciHif.TxRing[QID_AC_BE];
	PUCHAR pDMAHeaderBufVA = (PUCHAR)pTxRing->Cell[TxIdx].DmaBuf.AllocVa;
	TXD_STRUC *pTxD;
	MAC_TX_INFO Info;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 TXWISize = cap->TXWISize;
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
	Info.q_idx = 0;
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
		Info.AntPri = 2; /* b'010 */
		Info.SpeEn = 0;
		break;
	}

	/* Fill Transmit setting */
	Transmit.field.MCS = ATECtrl->Mcs;
	Transmit.field.BW = ATECtrl->BW;
	Transmit.field.ShortGI = ATECtrl->Sgi;
	Transmit.field.STBC = ATECtrl->Stbc;
	Transmit.field.MODE = ATECtrl->PhyMode;

	if (ATECtrl->PhyMode == MODE_CCK) {
		Info.Preamble = LONG_PREAMBLE;

		if (ATECtrl->Mcs == 9) {
			Transmit.field.MCS = 0;
			Info.Preamble = SHORT_PREAMBLE;
		} else if (ATECtrl->Mcs == 10) {
			Transmit.field.MCS = 1;
			Info.Preamble = SHORT_PREAMBLE;
		} else if (ATECtrl->Mcs == 11) {
			Transmit.field.MCS = 2;
			Info.Preamble = SHORT_PREAMBLE;
		}
	}

	Info.IsAutoRate = FALSE;
	pAd->archOps.write_tmac_info_fixed_rate(pAd, pDMAHeaderBufVA, &Info, &Transmit);
	NdisMoveMemory(pDMAHeaderBufVA + TXWISize, ATECtrl->TemplateFrame, ATECtrl->HLen);
	NdisMoveMemory(pDMAHeaderBufVA + TXWISize + 4, ATECtrl->Addr1, MAC_ADDR_LEN);
	NdisMoveMemory(pDMAHeaderBufVA + TXWISize + 10, ATECtrl->Addr2, MAC_ADDR_LEN);
	NdisMoveMemory(pDMAHeaderBufVA + TXWISize + 16, ATECtrl->Addr3, MAC_ADDR_LEN);
#ifdef RT_BIG_ENDIAN
	RTMPFrameEndianChange(pAd, (((PUCHAR)pDMAHeaderBufVA) + TXWISize), DIR_READ, FALSE);
#endif /* RT_BIG_ENDIAN */
	pPacket = ATEPayloadInit(pAd, TxIdx);

	if (pPacket == NULL) {
		ATECtrl->TxCount = 0;
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: fail to init frame payload.\n", __func__));
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
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	struct _ATE_IF_OPERATION *if_ops = ATECtrl->ATEIfOps;
	UINT32 Value = 0;
	INT32 Ret = 0;
#ifdef CONFIG_AP_SUPPORT
	INT32 IdBss, MaxNumBss = pAd->ApCfg.BssidNum;
#endif

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __func__));

	/* TxRx switch workaround */
	if (ATECtrl->did_rx == 1) {
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: DID Rx Before\n", __func__));
		MT_ATERestoreInit(pAd);
	}

	MtCmdChannelSwitch(pAd, ATECtrl->ControlChl, ATECtrl->Channel, ATECtrl->BW,
					   pAd->Antenna.field.TxPath, pAd->Antenna.field.RxPath, FALSE);
	AsicSetMacTxRx(pAd, ASIC_MAC_RX_RXV, FALSE);
	/*   Stop send TX packets */
	RTMP_OS_NETDEV_STOP_QUEUE(pAd->net_dev);
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		if (MaxNumBss > MAX_MBSSID_NUM(pAd))
			MaxNumBss = MAX_MBSSID_NUM(pAd);

		/*  first IdBss must not be 0 (BSS0), must be 1 (BSS1) */
		for (IdBss = FIRST_MBSSID; IdBss < MAX_MBSSID_NUM(pAd); IdBss++) {
			if (pAd->ApCfg.MBSSID[IdBss].wdev.if_dev)
				RTMP_OS_NETDEV_STOP_QUEUE(pAd->ApCfg.MBSSID[IdBss].wdev.if_dev);
		}
	}
#endif
	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DEQUEUEPACKET);
	/*  Disable PDMA */
	AsicSetWPDMA(pAd, PDMA_TX, 0);
	/* Polling TX/RX path until packets empty */

	if (if_ops->clean_trx_q)
		if_ops->clean_trx_q(pAd);


	/* Turn on RX again if set before */
	if (ATECtrl->Mode & ATE_RXFRAME)
		AsicSetMacTxRx(pAd, ASIC_MAC_RX_RXV, TRUE);

	RTMP_OS_NETDEV_START_QUEUE(pAd->net_dev);
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		if (MaxNumBss > MAX_MBSSID_NUM(pAd))
			MaxNumBss = MAX_MBSSID_NUM(pAd);

		/*  first IdBss must not be 0 (BSS0), must be 1 (BSS1) */
		for (IdBss = FIRST_MBSSID; IdBss < MAX_MBSSID_NUM(pAd); IdBss++) {
			if (pAd->ApCfg.MBSSID[IdBss].wdev.if_dev)
				RTMP_OS_NETDEV_START_QUEUE(pAd->ApCfg.MBSSID[IdBss].wdev.if_dev);
		}
	}
#endif
	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DEQUEUEPACKET);

	if (if_ops->setup_frame)
		if_ops->setup_frame(pAd, QID_AC_BE);

	if (if_ops->test_frame_tx) {
		if_ops->test_frame_tx(pAd);

		if (ATECtrl->TxCount != 0xFFFFFFFF)
			ATECtrl->TxCount += ATECtrl->TxDoneCount;

		ATECtrl->Mode |= ATE_TXFRAME;
	} else
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: no tx test frame callback function\n", __func__));


	/* Low temperature high rate EVM degrade Patch v2 */
	if ((MTK_REV_GTE(pAd, MT7603, MT7603E1)) ||
		(MTK_REV_GTE(pAd, MT7628, MT7628E1))) {
		if (ATECtrl->TxAntennaSel == 0) {
			RTMP_IO_READ32(pAd, CR_RFINTF_00, &Value);
			Value &= ~CR_RFINTF_CAL_NSS_MASK;
			Value |= CR_RFINTF_CAL_NSS(0x0);
			RTMP_IO_WRITE32(pAd, CR_RFINTF_00, Value);
		} else if (ATECtrl->TxAntennaSel == 1) {
			RTMP_IO_READ32(pAd, CR_RFINTF_00, &Value);
			Value &= ~CR_RFINTF_CAL_NSS_MASK;
			Value |= CR_RFINTF_CAL_NSS(0x0);
			RTMP_IO_WRITE32(pAd, CR_RFINTF_00, Value);
		} else if (ATECtrl->TxAntennaSel == 2) {
			RTMP_IO_READ32(pAd, CR_RFINTF_00, &Value);
			Value &= ~CR_RFINTF_CAL_NSS_MASK;
			Value |= CR_RFINTF_CAL_NSS(0x1);
			RTMP_IO_WRITE32(pAd, CR_RFINTF_00, Value);
		}
	}

	ATECtrl->did_tx = 1;
	return Ret;
}


static INT32 MT_ATEStartRx(RTMP_ADAPTER *pAd)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	struct _ATE_IF_OPERATION *if_ops = ATECtrl->ATEIfOps;
	INT32 Ret = 0, Value;
	UINT32 reg;
#ifdef CONFIG_AP_SUPPORT
	INT32 IdBss, MaxNumBss = pAd->ApCfg.BssidNum;
#endif

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __func__));
	RTMP_IO_READ32(pAd, ARB_SCR, &reg);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: ARB_SCR:%x\n", __func__, reg));
	reg &= ~MT_ARB_SCR_RXDIS;
	RTMP_IO_WRITE32(pAd, ARB_SCR, reg);
	AsicSetMacTxRx(pAd, ASIC_MAC_RX_RXV, FALSE);
	/*   Stop send TX packets */
	RTMP_OS_NETDEV_STOP_QUEUE(pAd->net_dev);
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		if (MaxNumBss > MAX_MBSSID_NUM(pAd))
			MaxNumBss = MAX_MBSSID_NUM(pAd);

		/*  first IdBss must not be 0 (BSS0), must be 1 (BSS1) */
		for (IdBss = FIRST_MBSSID; IdBss < MAX_MBSSID_NUM(pAd); IdBss++) {
			if (pAd->ApCfg.MBSSID[IdBss].wdev.if_dev)
				RTMP_OS_NETDEV_STOP_QUEUE(pAd->ApCfg.MBSSID[IdBss].wdev.if_dev);
		}
	}
#endif
	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DEQUEUEPACKET);
	AsicSetWPDMA(pAd, PDMA_TX, 0);

	if (if_ops->clean_trx_q)
		if_ops->clean_trx_q(pAd);

	RTMP_OS_NETDEV_START_QUEUE(pAd->net_dev);
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		if (MaxNumBss > MAX_MBSSID_NUM(pAd))
			MaxNumBss = MAX_MBSSID_NUM(pAd);

		/*  first IdBss must not be 0 (BSS0), must be 1 (BSS1) */
		for (IdBss = FIRST_MBSSID; IdBss < MAX_MBSSID_NUM(pAd); IdBss++) {
			if (pAd->ApCfg.MBSSID[IdBss].wdev.if_dev)
				RTMP_OS_NETDEV_START_QUEUE(pAd->ApCfg.MBSSID[IdBss].wdev.if_dev);
		}
	}
#endif
	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DEQUEUEPACKET);

	/* Turn on TX again if set before */
	if (ATECtrl->Mode & ATE_TXFRAME)
		AsicSetMacTxRx(pAd, ASIC_MAC_TX, TRUE);

	/* reset counter when iwpriv only */
	if (ATECtrl->bQAEnabled != TRUE)
		ATECtrl->RxTotalCnt = 0;

	pAd->WlanCounters[0].FCSErrorCount.u.LowPart = 0;
	RTMP_IO_READ32(pAd, RMAC_RFCR, &Value);
	Value |= RM_FRAME_REPORT_EN;
	RTMP_IO_WRITE32(pAd, RMAC_RFCR, Value);
	AsicSetMacTxRx(pAd, ASIC_MAC_RX_RXV, TRUE);
	/* Enable PDMA */
	AsicSetWPDMA(pAd, PDMA_TX_RX, 1);
	ATECtrl->Mode |= ATE_RXFRAME;
	ATECtrl->did_rx = 1;
	return Ret;
}


static INT32 MT_ATEStopTx(RTMP_ADAPTER *pAd, UINT32 Mode)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	struct _ATE_IF_OPERATION *if_ops = ATECtrl->ATEIfOps;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 num_of_tx_ring = GET_NUM_OF_TX_RING(cap);
	INT32 Ret = 0;
	INT32 acidx;

	acidx = 0;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __func__));

	if ((Mode & ATE_TXFRAME) || (Mode == ATE_STOP)) {
		/*  Disable PDMA */
		AsicSetWPDMA(pAd, PDMA_TX_RX, 0);

		if (if_ops->clean_trx_q)
			if_ops->clean_trx_q(pAd);

		ATECtrl->Mode &= ~ATE_TXFRAME;
		/* Enable PDMA */
		AsicSetWPDMA(pAd, PDMA_TX_RX, 1);
	}

	return Ret;
}


static INT32 MT_ATEStopRx(RTMP_ADAPTER *pAd)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	INT32 Ret = 0;
	INT32 i;
	UINT32 reg;

	i = 0;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __func__));
	Ret = AsicSetMacTxRx(pAd, ASIC_MAC_RX_RXV, FALSE);
	RTMP_IO_READ32(pAd, ARB_SCR, &reg);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: ARB_SCR:%x\n", __func__, reg));
	reg |= MT_ARB_SCR_RXDIS;
	RTMP_IO_WRITE32(pAd, ARB_SCR, reg);
	ATECtrl->Mode &= ~ATE_RXFRAME;
	return Ret;
}


static INT32 MT_ATESetTxAntenna(RTMP_ADAPTER *pAd, CHAR Ant)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	INT32 Ret = 0;
	UINT32 Value;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __func__));
	/* 0: All 1:TX0 2:TX1 */
	ATECtrl->TxAntennaSel = Ant;

	if ((MTK_REV_GTE(pAd, MT7603, MT7603E1)) ||
		(MTK_REV_GTE(pAd, MT7628, MT7628E1))) {
		UCHAR ePA;
#ifdef RTMP_EFUSE_SUPPORT

		if (pAd->E2pAccessMode == E2P_NONE) {
			UINT16 v_efuse = 0;

			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: E2P_NONE\n", __func__));
			rtmp_ee_efuse_read16(pAd, 0x35, &v_efuse);

			if (v_efuse == 0)
				ePA = pAd->EEPROMImage[0x35];
			else
				ePA = (v_efuse >> 8) & 0x00FF;

			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("E2P_NONE, EFUSE Value == 0x%x, ePA:0x%x\n", v_efuse, ePA));
		} else if (pAd->E2pAccessMode == E2P_EFUSE_MODE) {
			UINT16 v_efuse = 0;

			rtmp_ee_efuse_read16(pAd, 0x35, &v_efuse);
			ePA = (v_efuse >> 8) & 0x00FF;
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("EFUSE_MODE, EFUSE Value == 0x%x, ePA:0x%x\n", v_efuse, ePA));
		} else
#endif /* RTMP_EFUSE_SUPPORT */
		{
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: BIN/FLASH/EEPROM_MODE(0x%x), get value in buffer\n", __func__, pAd->E2pAccessMode));
			ePA = pAd->EEPROMImage[0x35];
		}

		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: ePA:0x%x, E2pAccessMode:0x%x\n", __func__, ePA, pAd->E2pAccessMode));

		if (ATECtrl->TxAntennaSel == 0) {
			RTMP_IO_READ32(pAd, CR_RFINTF_00, &Value);
			Value &= ~CR_RFINTF_CAL_NSS_MASK;
			Value |= CR_RFINTF_CAL_NSS(0x0);
			RTMP_IO_WRITE32(pAd, CR_RFINTF_00, Value);

			/* ePA Tx1 patch, bit[1]: 2.4G ePA Enable, same as Tx0 */
			if (ePA & 0x02) {
				UINT32 RemapBase, RemapOffset;
				UINT32 RestoreValue;
				UINT32 value;

				RTMP_IO_READ32(pAd, MCU_PCIE_REMAP_2, &RestoreValue);
				RemapBase = GET_REMAP_2_BASE(0x81060008) << 19;
				RemapOffset = GET_REMAP_2_OFFSET(0x81060008);
				RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RemapBase);

				if ((MTK_REV_GTE(pAd, MT7603, MT7603E1)) ||
					(MTK_REV_GTE(pAd, MT7603, MT7603E2))) {
					value = 0x04852390;
					RTMP_IO_WRITE32(pAd, 0x80000 + RemapOffset, value);
				} else if (MTK_REV_GTE(pAd, MT7628, MT7628E1)) {
					value = 0x00489523;
					RTMP_IO_WRITE32(pAd, 0x80000 + RemapOffset, value);
				}

				RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RestoreValue);
			}
		} else if (ATECtrl->TxAntennaSel == 1) {
			RTMP_IO_READ32(pAd, CR_RFINTF_00, &Value);
			Value &= ~CR_RFINTF_CAL_NSS_MASK;
			Value |= CR_RFINTF_CAL_NSS(0x0);
			RTMP_IO_WRITE32(pAd, CR_RFINTF_00, Value);

			/* ePA Tx1 patch, bit[1]: 2.4G ePA Enable */
			if (ePA & 0x02) {
				UINT32 RemapBase, RemapOffset;
				UINT32 RestoreValue;
				UINT32 value;

				RTMP_IO_READ32(pAd, MCU_PCIE_REMAP_2, &RestoreValue);
				RemapBase = GET_REMAP_2_BASE(0x81060008) << 19;
				RemapOffset = GET_REMAP_2_OFFSET(0x81060008);
				RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RemapBase);

				if ((MTK_REV_GTE(pAd, MT7603, MT7603E1)) ||
					(MTK_REV_GTE(pAd, MT7603, MT7603E2))) {
					value = 0x04852390;
					RTMP_IO_WRITE32(pAd, 0x80000 + RemapOffset, value);
				} else if (MTK_REV_GTE(pAd, MT7628, MT7628E1)) {
					value = 0x00489523;
					RTMP_IO_WRITE32(pAd, 0x80000 + RemapOffset, value);
				}

				RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RestoreValue);
			}
		} else if (ATECtrl->TxAntennaSel == 2) {
			RTMP_IO_READ32(pAd, CR_RFINTF_00, &Value);
			Value &= ~CR_RFINTF_CAL_NSS_MASK;
			Value |= CR_RFINTF_CAL_NSS(0x1);
			RTMP_IO_WRITE32(pAd, CR_RFINTF_00, Value);

			/* ePA Tx1 patch, bit[1]: 2.4G ePA Enable */
			if (ePA & 0x02) {
				UINT32 RemapBase, RemapOffset;
				UINT32 RestoreValue;
				UINT32 value;

				RTMP_IO_READ32(pAd, MCU_PCIE_REMAP_2, &RestoreValue);
				RemapBase = GET_REMAP_2_BASE(0x81060008) << 19;
				RemapOffset = GET_REMAP_2_OFFSET(0x81060008);
				RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RemapBase);

				if ((MTK_REV_GTE(pAd, MT7603, MT7603E1)) ||
					(MTK_REV_GTE(pAd, MT7603, MT7603E2))) {
					value = 0x04856790;
					RTMP_IO_WRITE32(pAd, 0x80000 + RemapOffset, value);
				} else if (MTK_REV_GTE(pAd, MT7628, MT7628E1)) {
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
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	INT32 Ret = 0;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __func__));
	ATECtrl->RxAntennaSel = Ant;
	/* set RX path */
	MtAsicSetRxPath(pAd, (UINT32)ATECtrl->RxAntennaSel, 0);
	return Ret;
}


static INT32 MT_ATESetTxFreqOffset(RTMP_ADAPTER *pAd, UINT32 FreqOffset)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	INT32 Ret = 0;
	UINT32 reg = 0;

	ATECtrl->RFFreqOffset = FreqOffset;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __func__));

	if (IS_MT76x6(pAd) || IS_MT7637(pAd)) {
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("MT76x6 Manual Set Frequency\n"));
		RTMP_IO_READ32(pAd, FREQ_OFFSET_MANUAL_ENABLE, &reg);

		if (ATECtrl->en_man_freq_restore == 0) {
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("MT76x6 Normal Set Frequency BK\n"));
			ATECtrl->en_man_freq_restore = reg;
		}

		reg = (reg & 0xFFFF80FF) | (0x7F << 8);
		RTMP_IO_WRITE32(pAd, FREQ_OFFSET_MANUAL_ENABLE, reg);
		ATECtrl->en_man_set_freq = 1;
	}

	if (ATECtrl->en_man_set_freq == 1) {
		RTMP_IO_READ32(pAd, FREQ_OFFSET_MANUAL_VALUE, &reg);

		if (ATECtrl->normal_freq_restore == 0) {
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("MT76x6 Normal Frequency BK\n"));
			ATECtrl->normal_freq_restore = reg;
		}

		reg = (reg & 0xFFFF80FF) | (ATECtrl->RFFreqOffset << 8);
		RTMP_IO_WRITE32(pAd, FREQ_OFFSET_MANUAL_VALUE, reg);
	} else
		MtAsicSetRfFreqOffset(pAd, ATECtrl->RFFreqOffset);

	return Ret;
}

static INT32 MT_ATESetChannel(RTMP_ADAPTER *pAd, INT16 Value)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	INT32 Ret = 0;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __func__));
	MtCmdChannelSwitch(pAd, ATECtrl->ControlChl, ATECtrl->Channel, ATECtrl->BW,
					   pAd->Antenna.field.TxPath, pAd->Antenna.field.RxPath, FALSE);
	return Ret;
}


static INT32 MT_ATESetBW(RTMP_ADAPTER *pAd, INT16 Value)
{
	INT32 Ret = 0;
	UINT32 val = 0;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __func__));
	RTMP_IO_READ32(pAd, AGG_BWCR, &val);
	val &= (~0x0c);

	switch (Value) {
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
	/* CmdChPrivilege(pAd, CMD_CH_PRIV_ACTION_BW_REQ, ATECtrl->ControlChl, ATECtrl->Channel, */
	/* ATECtrl->BW, pAd->Antenna.field.TxPath, pAd->Antenna.field.RxPath); */
	return Ret;
}


static INT32 MT_ATESampleRssi(RTMP_ADAPTER *pAd, RX_BLK *RxBlk)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	INT32 Ret = 0;

	if (RxBlk->rx_signal.raw_rssi[0] != 0) {
		ATECtrl->LastRssi0 = ConvertToRssi(pAd,
										   (struct raw_rssi_info *)(&RxBlk->rx_signal.raw_rssi[0]), RSSI_IDX_0);

		if (ATECtrl->MaxRssi0 <  ATECtrl->LastRssi0)
			ATECtrl->MaxRssi0 = ATECtrl->LastRssi0;

		if (ATECtrl->MinRssi0 >  ATECtrl->LastRssi0)
			ATECtrl->MinRssi0 =  ATECtrl->LastRssi0;

		ATECtrl->AvgRssi0X8 = (ATECtrl->AvgRssi0X8 - ATECtrl->AvgRssi0)
							  + ATECtrl->LastRssi0;
		ATECtrl->AvgRssi0 = ATECtrl->AvgRssi0X8 >> 3;
	}

	if (RxBlk->rx_signal.raw_rssi[1] != 0) {
		ATECtrl->LastRssi1 = ConvertToRssi(pAd,
										   (struct raw_rssi_info *)(&RxBlk->rx_signal.raw_rssi[0]), RSSI_IDX_1);

		if (ATECtrl->MaxRssi1 <  ATECtrl->LastRssi1)
			ATECtrl->MaxRssi1 = ATECtrl->LastRssi1;

		if (ATECtrl->MinRssi1 >  ATECtrl->LastRssi1)
			ATECtrl->MinRssi1 =  ATECtrl->LastRssi1;

		ATECtrl->AvgRssi1X8 = (ATECtrl->AvgRssi1X8 - ATECtrl->AvgRssi1)
							  + ATECtrl->LastRssi1;
		ATECtrl->AvgRssi1 = ATECtrl->AvgRssi1X8 >> 3;
	}

	ATECtrl->LastSNR0 = RxBlk->rx_signal.raw_snr[0];
	ATECtrl->LastSNR1 = RxBlk->rx_signal.raw_snr[1];
	ATECtrl->NumOfAvgRssiSample++;
	return Ret;
}

static INT32 MT_ATESetAIFS(RTMP_ADAPTER *pAd, CHAR Value)
{
	INT32 Ret = 0;
	UINT val = Value & 0x000000ff;

	/* Test mode use AC0 for TX */
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: Value:%x\n", __func__, val));
	AsicSetWmmParam(pAd, 0, WMM_PARAM_AC_0, WMM_PARAM_AIFSN, val);
	return Ret;
}

static INT32 MT_ATESetTSSI(RTMP_ADAPTER *pAd, CHAR WFSel, CHAR Setting)
{
	INT32 Ret = 0;

	Ret = MtAsicSetTSSI(pAd, Setting, WFSel);
	return Ret;
}

static INT32 MT_ATESetDPD(RTMP_ADAPTER *pAd, CHAR WFSel, CHAR Setting)
{
	/* !!TEST MODE ONLY!! Normal Mode control by FW and Never disable */
	/* WF0 = 0, WF1 = 1, WF ALL = 2 */
	INT32 Ret = 0;

	Ret = MtAsicSetDPD(pAd, Setting, WFSel);
	return Ret;
}

static INT32 MT_ATEStartTxTone(RTMP_ADAPTER *pAd, UINT32 Mode)
{
	INT32 Ret = 0;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __func__));
	MtAsicSetTxToneTest(pAd, 1, Mode);
	return Ret;
}

static INT32 MT_ATESetTxTonePower(RTMP_ADAPTER *pAd, INT32 pwr1, INT32 pwr2)
{
	INT32 Ret = 0;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: pwr1:%d, pwr2:%d\n", __func__, pwr1, pwr2));
	MtAsicSetTxTonePower(pAd, pwr1, pwr2);
	return Ret;
}

static INT32 MT_ATEStopTxTone(RTMP_ADAPTER *pAd)
{
	INT32 Ret = 0;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __func__));
	MtAsicSetTxToneTest(pAd, 0, 0);
	return Ret;
}

static INT32 MT_ATEStartContinousTx(RTMP_ADAPTER *pAd, CHAR WFSel)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	INT32 Ret = 0;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __func__));
	CmdTxContinous(pAd, ATECtrl->PhyMode, ATECtrl->BW, ATECtrl->ControlChl, ATECtrl->Mcs, ATECtrl->TxAntennaSel, 1);
	return Ret;
}

static INT32 MT_ATEStopContinousTx(RTMP_ADAPTER *pAd)
{
	INT32 Ret = 0;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __func__));
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;

	CmdTxContinous(pAd, ATECtrl->PhyMode, ATECtrl->BW, ATECtrl->ControlChl, ATECtrl->Mcs, ATECtrl->TxAntennaSel, 0);
	return Ret;
}

static INT32 MT_RfRegWrite(RTMP_ADAPTER *pAd, UINT32 WFSel, UINT32 Offset, UINT32 Value)
{
	INT32 Ret = 0;

	Ret = MtCmdRFRegAccessWrite(pAd, WFSel, Offset, Value);
	return Ret;
}


static INT32 MT_RfRegRead(RTMP_ADAPTER *pAd, UINT32 WFSel, UINT32 Offset, UINT32 *Value)
{
	INT32 Ret = 0;

	Ret = MtCmdRFRegAccessRead(pAd, WFSel, Offset, Value);
	return Ret;
}


static INT32 MT_GetFWInfo(RTMP_ADAPTER *pAd, UCHAR *FWInfo)
{
	struct fwdl_ctrl *ctrl = &pAd->MCUCtrl.fwdl_ctrl;

	memcpy(FWInfo, ctrl->fw_profile[WM_CPU].source.img_ptr + ctrl->fw_profile[WM_CPU].source.img_len - 36, 36);
	return 0;
}


#ifdef TXBF_SUPPORT
/*
 *==========================================================================
 *   Description:
 *	Set ATE Tx Beamforming mode
 *
 *	Return:
 *		TRUE if all parameters are OK, FALSE otherwise
 *==========================================================================
*/
INT	MT_SetATETxBfProc(RTMP_ADAPTER *pAd, UCHAR TxBfEnFlg)
{
	/* PATE_INFO pATEInfo = &(pAd->ate); */
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;

	switch (TxBfEnFlg) {
	case 0:
		/* no BF */
		ATECtrl->iTxBf = FALSE;
		ATECtrl->eTxBf = FALSE;
		break;

	case 1:
		/* ETxBF */
		ATECtrl->iTxBf = FALSE;
		ATECtrl->eTxBf = TRUE;
		break;

	case 2:
		/* ITxBF */
		ATECtrl->iTxBf = TRUE;
		ATECtrl->eTxBf = FALSE;
		break;

	case 3:
		/* Enable TXBF support */
		ATECtrl->iTxBf = TRUE;
		ATECtrl->eTxBf = TRUE;
		break;

	default:
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Set_ATE_TXBF_Proc: Invalid parameter %d\n", TxBfEnFlg));
		break;
	}

#ifdef MT_MAC

	if (ATECtrl->eTxBf == TRUE) {
		/* Enable sounding trigger in FW */
		AsicBfSoundingPeriodicTriggerCtrl(pAd, BSSID_WCID, TRUE);
	} else {
		/* Disable sounding trigger in FW */
		AsicBfSoundingPeriodicTriggerCtrl(pAd, BSSID_WCID, FALSE);
	}

#endif
	return TRUE;
}


#ifdef MT_MAC
/*
 *==========================================================================
 *Description:
 *	Enable sounding trigger
 *
 *	Return:
 *		TRUE if all parameters are OK, FALSE otherwise
 *==========================================================================
*/
INT	MT_SetATESoundingProc(RTMP_ADAPTER *pAd, UCHAR SDEnFlg)
{
	/* Enable sounding trigger in FW */
	return AsicBfSoundingPeriodicTriggerCtrl(pAd, BSSID_WCID, SDEnFlg);
}
#endif /* MT_MAC */
#endif /* TXBF_SUPPORT */



#ifdef RTMP_MAC_PCI
static INT32 pci_ate_init(RTMP_ADAPTER *pAd)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	UINT32 Index;
	RTMP_TX_RING *pTxRing = &pAd->PciHif.TxRing[QID_AC_BE];

	MTWF_LOG(DBG_CAT_TEST,  DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __func__));
	HIF_IO_READ32(pAd, pTxRing->hw_didx_addr, &pTxRing->TxDmaIdx);
	pTxRing->TxSwFreeIdx = pTxRing->TxDmaIdx;
	pTxRing->TxCpuIdx = pTxRing->TxDmaIdx;
	HIF_IO_WRITE32(pAd, pTxRing->hw_cidx_addr, pTxRing->TxCpuIdx);

	for (Index = 0; Index < TX_RING_SIZE; Index++) {
		if (ATEPayloadAlloc(pAd, Index) != (NDIS_STATUS_SUCCESS)) {
			ATECtrl->allocated = 0;
			goto pci_ate_init_err;
		}
	}

	ATECtrl->allocated = 1;
	RTMP_ASIC_INTERRUPT_ENABLE(pAd);
	return NDIS_STATUS_SUCCESS;
pci_ate_init_err:
	MTWF_LOG(DBG_CAT_TEST,  DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Allocate test packet fail at pakcet%d\n", __func__, Index));
	return NDIS_STATUS_FAILURE;
}

static INT32 pci_clean_q(RTMP_ADAPTER *pAd)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	UINT32 Index;
	TXD_STRUC *pTxD = NULL;
	RTMP_TX_RING *pTxRing = &pAd->PciHif.TxRing[QID_AC_BE];
#ifdef RT_BIG_ENDIAN
	TXD_STRUC *pDestTxD = NULL;
	UCHAR TxHwInfo[TXD_SIZE];
#endif

	MTWF_LOG(DBG_CAT_TEST,  DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s -->\n", __func__));

	/* Polling TX/RX path until packets empty */
	if (ATECtrl->tx_coherent == 0)
		MTPciPollTxRxEmpty(pAd);

	for (Index = 0; Index < TX_RING_SIZE; Index++) {
		PNDIS_PACKET  pPacket;

		pPacket = pTxRing->Cell[Index].pNextNdisPacket;

		if (pPacket)
			PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr1, pTxD->SDLen1, RTMP_PCI_DMA_TODEVICE);

#ifdef RT_BIG_ENDIAN
		RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
		WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, FALSE, TYPE_TXD);
#endif
	}

	return NDIS_STATUS_SUCCESS;
}

static INT32 pci_setup_frame(RTMP_ADAPTER *pAd, UINT32 q_idx)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	UINT32 Index;
	UINT32 TxIdx = 0;
	RTMP_TX_RING *pTxRing = &pAd->PciHif.TxRing[QID_AC_BE];

	MTWF_LOG(DBG_CAT_TEST,  DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s -->,\n", __func__));

	if (ATECtrl->allocated == 0)
		goto pci_setup_frame_err;

	HIF_IO_READ32(pAd, pTxRing->hw_didx_addr, &pTxRing->TxDmaIdx);
	pTxRing->TxSwFreeIdx = pTxRing->TxDmaIdx;
	pTxRing->TxCpuIdx = pTxRing->TxDmaIdx;
	HIF_IO_WRITE32(pAd, pTxRing->hw_cidx_addr, pTxRing->TxCpuIdx);

	if (ATECtrl->bQAEnabled != TRUE) /* reset in start tx when iwpriv */
		ATECtrl->TxDoneCount = 0;

	for (Index = 0; Index < TX_RING_SIZE; Index++)
		pTxRing->Cell[Index].pNdisPacket = ATECtrl->pAtePacket[Index];

	for (Index = 0; (Index < TX_RING_SIZE) && (Index < ATECtrl->TxCount); Index++) {
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Index = %d, ATECtrl->TxCount = %u\n", Index, ATECtrl->TxCount));
		TxIdx = pTxRing->TxCpuIdx;

		if (MT_ATESetupFrame(pAd, TxIdx) != 0)
			return NDIS_STATUS_FAILURE;

		if (((Index + 1) < TX_RING_SIZE) && (Index < ATECtrl->TxCount))
			INC_RING_INDEX(pTxRing->TxCpuIdx, TX_RING_SIZE);
	}

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: TxCpuIdx: %u, TxDmaIdx: %u\n", __func__, pTxRing->TxCpuIdx, pTxRing->TxDmaIdx));
	return NDIS_STATUS_SUCCESS;
pci_setup_frame_err:
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Setup frame fail\n", __func__));
	return NDIS_STATUS_FAILURE;
}

static INT32 pci_test_frame_tx(RTMP_ADAPTER *pAd)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	RTMP_TX_RING *pTxRing = &pAd->PciHif.TxRing[QID_AC_BE];

	if (ATECtrl->allocated == 0)
		goto pci_tx_frame_err;

	/* Enable PDMA */
	AsicSetWPDMA(pAd, PDMA_TX_RX, 1);
	ATECtrl->Mode |= ATE_TXFRAME;
	HIF_IO_WRITE32(pAd, pTxRing->hw_cidx_addr, pTxRing->TxCpuIdx);
	return NDIS_STATUS_SUCCESS;
pci_tx_frame_err:
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: tx frame fail\n", __func__));
	return NDIS_STATUS_FAILURE;
}

static INT32 pci_clean_test_rx_frame(RTMP_ADAPTER *pAd)
{
	UINT32 Index, RingNum;
	RXD_STRUC *pRxD = NULL;
#ifdef RT_BIG_ENDIAN
	RXD_STRUC *pDestRxD;
	UCHAR RxHwInfo[RXD_SIZE];
#endif
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 num_of_rx_ring = GET_NUM_OF_RX_RING(cap);

	for (RingNum = 0; RingNum < num_of_rx_ring; RingNum++) {
		for (Index = 0; Index < RX_RING_SIZE; Index++) {
#ifdef RT_BIG_ENDIAN
			pDestRxD = (RXD_STRUC *)pAd->PciHif.RxRing[0].Cell[Index].AllocVa;
			NdisMoveMemory(&RxHwInfo[0], pDestRxD, RXD_SIZE);
			pRxD = (RXD_STRUC *)&RxHwInfo[0];
			RTMPDescriptorEndianChange((PUCHAR)pRxD, TYPE_RXD);
#else
			/* Point to Rx indexed rx ring descriptor */
			pRxD = (RXD_STRUC *)pAd->PciHif.RxRing[0].Cell[Index].AllocVa;
#endif
			pRxD->DDONE = 0;
#ifdef RT_BIG_ENDIAN
			RTMPDescriptorEndianChange((PUCHAR)pRxD, TYPE_RXD);
			WriteBackToDescriptor((PUCHAR)pDestRxD, (PUCHAR)pRxD, FALSE, TYPE_RXD);
#endif
		}
	}

	return NDIS_STATUS_SUCCESS;
}

static INT32 pci_ate_leave(RTMP_ADAPTER *pAd)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	UINT32 Index;
	TXD_STRUC *pTxD = NULL;
	RTMP_TX_RING *pTxRing = &pAd->PciHif.TxRing[QID_AC_BE];
#ifdef RT_BIG_ENDIAN
	TXD_STRUC *pDestTxD = NULL;
	UCHAR tx_hw_info[TXD_SIZE];
#endif /* RT_BIG_ENDIAN */

	MTWF_LOG(DBG_CAT_TEST,  DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s -->,\n", __func__));
	NICReadEEPROMParameters(pAd, NULL);
	NICInitAsicFromEEPROM(pAd);

	if (ATECtrl->allocated == 1) {
		/*  Disable PDMA */
		AsicSetWPDMA(pAd, PDMA_TX_RX, 0);

		for (Index = 0; Index < TX_RING_SIZE; Index++) {
			PNDIS_PACKET pPacket;
#ifndef RT_BIG_ENDIAN
			pTxD = (TXD_STRUC *)pAd->PciHif.TxRing[QID_AC_BE].Cell[Index].AllocVa;
#else
			pDestTxD = (TXD_STRUC *)pAd->TxRing[QID_AC_BE].Cell[Index].AllocVa;
			NdisMoveMemory(&tx_hw_info[0], (UCHAR *)pDestTxD, TXD_SIZE);
			pTxD = (TXD_STRUC *)&tx_hw_info[0];
			RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#endif /* !RT_BIG_ENDIAN */
			pTxD->DMADONE = 0;
			pPacket = pTxRing->Cell[Index].pNdisPacket;

			if (pPacket) {
				PCI_UNMAP_SINGLE(pAd, pTxD->SDPtr0, pTxD->SDLen0, RTMP_PCI_DMA_TODEVICE);
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
			}

			/* Always assign pNdisPacket as NULL after clear */
			pTxRing->Cell[Index].pNdisPacket = NULL;
			pPacket = pTxRing->Cell[Index].pNextNdisPacket;

			if (pPacket) {
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

		ATECtrl->allocated = 0;
	}

	return NDIS_STATUS_SUCCESS;
}
#endif
static struct _ATE_OPERATION MT_ATEOp = {
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
#if !defined(COMPOS_TESTMODE_WIN)/* 1       todo RX_BLK */
	.SampleRssi = MT_ATESampleRssi,
#endif
	.SetAIFS = MT_ATESetAIFS,
	.SetTSSI = MT_ATESetTSSI,
	.SetDPD = MT_ATESetDPD,
	.StartTxTone = MT_ATEStartTxTone,
	.SetTxTonePower = MT_ATESetTxTonePower,
	.StopTxTone = MT_ATEStopTxTone,
	.StartContinousTx = MT_ATEStartContinousTx,
	.StopContinousTx = MT_ATEStopContinousTx,
	.RfRegWrite = MT_RfRegWrite,
	.RfRegRead = MT_RfRegRead,
	.GetFWInfo = MT_GetFWInfo,
#if defined(TXBF_SUPPORT) && defined(MT_MAC)
	.SetATETxBfProc = MT_SetATETxBfProc,
	.SetATETxSoundingProc = MT_SetATESoundingProc,
#endif /* MT_MAC */
};

#ifdef RTMP_MAC_PCI
static struct _ATE_IF_OPERATION ate_if_ops = {
	.init = pci_ate_init,
	.clean_trx_q = pci_clean_q,
	.clean_test_rx_frame = pci_clean_test_rx_frame,
	.setup_frame = pci_setup_frame,
	.test_frame_tx = pci_test_frame_tx,
	.ate_leave = pci_ate_leave,
};
#else
static struct _ATE_IF_OPERATION ate_if_ops = {
	.init = NULL,
	.clean_trx_q = NULL,
	.setup_frame = NULL,
	.test_frame_tx = NULL,
	.ate_leave = NULL,
};
#endif /* RTMP_MAC_PCI */

INT32 MT_ATEInit(RTMP_ADAPTER *pAd)
{
	struct _ATE_CTRL *ATECtrl = &pAd->ATECtrl;

	ATECtrl->ATEOp = &MT_ATEOp;
	ATECtrl->ATEIfOps = &ate_if_ops;
	return 0;
}

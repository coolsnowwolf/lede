/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology	5th	Rd.
 * Science-based Industrial	Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of Ralink Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	cmm_asic.c

	Abstract:
	Functions used to communicate with ASIC

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
*/


#include "rt_config.h"

#define BSSID_WCID_TO_REMOVE 1 /* Pat:TODO */

#ifdef CONFIG_AP_SUPPORT
VOID APCheckBcnQHandler(RTMP_ADAPTER *pAd, INT apidx, BOOLEAN *is_pretbtt_int)
{
	/* no implementation after mt7615 */
	return;
}
#endif /* CONFIG_AP_SUPPORT */

VOID MTPollTxRxEmpty(RTMP_ADAPTER *pAd)
{
#ifdef RTMP_MAC_PCI
	MTPciPollTxRxEmpty(pAd);
#endif /* RTMP_MAC_PCI */
}

VOID MTHifPolling(RTMP_ADAPTER *pAd, UINT8 ucDbdcIdx)
{
#ifdef RTMP_MAC_PCI
	UINT32 Loop, RxPending = 0;
	PNDIS_PACKET pRxPacket = NULL;
	RX_BLK RxBlk, *pRxBlk;
	BOOLEAN bReschedule = FALSE;
	EVENT_EXT_CMD_RESULT_T	rResult = {0};

	for (Loop = 0; Loop < 10; Loop++) {
		while (1) {
			pRxBlk = &RxBlk;
			pRxPacket = pAd->archOps.get_pkt_from_rx_resource(pAd, &bReschedule, &RxPending, 0);

			if ((RxPending == 0) && (bReschedule == FALSE))
				break;
			RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_SUCCESS);

			msleep(20);
		}
	}

	for (Loop = 0; Loop < 10; Loop++) {
		AsicExtWifiHifCtrl(pAd, ucDbdcIdx, HIF_CTRL_ID_HIF_USB_TX_RX_IDLE, &rResult);

		if (rResult.u4Status == 0)
			break;

		while (1) {
			pRxBlk = &RxBlk;
			pRxPacket = pAd->archOps.get_pkt_from_rx_resource(pAd, &bReschedule, &RxPending, 0);

			if ((RxPending == 0) && (bReschedule == FALSE))
				break;
			RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_SUCCESS);
		}

		if (Loop == 1) {
			/* Above scenario should pass at 1st time or assert */
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Failed to poll RX path empry.\n", __func__));
			ASSERT(0);
		}
	}

#endif /* RTMP_MAC_PCI */
}

VOID MTRadioOn(PRTMP_ADAPTER pAd, struct wifi_dev *wdev)
{
	/* Driver Own */
	if (IS_MT7622(pAd))
		DriverOwn(pAd);

	/*  Send radio on command and wait for ack */
	if (IS_MT7603(pAd))
		MtCmdRadioOnOffCtrl(pAd, WIFI_RADIO_ON);
	else
		RTMP_RADIO_ON_OFF_CTRL(pAd, HcGetBandByChannel(pAd, wdev->channel), WIFI_RADIO_ON);

	/* Send Led on command */

	/* Enable RX */
	if (IS_MT7603(pAd))
		AsicSetMacTxRx(pAd, ASIC_MAC_RX, TRUE);

	HcSetRadioCurStatByChannel(pAd, wdev->channel, PHY_INUSE);
}

VOID MTRadioOff(PRTMP_ADAPTER pAd, struct wifi_dev *wdev)
{
	/*  Disable RX */
	if (IS_MT7603(pAd))
		AsicSetMacTxRx(pAd, ASIC_MAC_RX, FALSE);

	/*  Polling TX/RX path until packets empty */
	if (IS_MT7603(pAd))
		MTPollTxRxEmpty(pAd);

	/* Set Radio off flag*/
	HcSetRadioCurStatByChannel(pAd, wdev->channel, PHY_RADIOOFF);

	/* Delay for CR access, review it again. Pat: */
	/* msleep(1000); */

	/*  Send radio off command and wait for ack */
	if (IS_MT7603(pAd))
		MtCmdRadioOnOffCtrl(pAd, WIFI_RADIO_OFF);
	else
		RTMP_RADIO_ON_OFF_CTRL(pAd, HcGetBandByChannel(pAd, wdev->channel), WIFI_RADIO_OFF);

	if (IS_MT7622(pAd)) {
		/* Polling RX path until packets empty when all bands are in radio off */
		if (IsHcAllSupportedBandsRadioOff(pAd)) {
			MTHifPolling(pAd, HcGetBandByWdev(wdev));

			/* FW own */
			FwOwn(pAd);
		}
	}
}

#ifdef RTMP_MAC_PCI
VOID MTMlmeLpExit(PRTMP_ADAPTER pAd, struct wifi_dev *wdev)
{
#ifdef CONFIG_AP_SUPPORT
	INT32 IdBss, MaxNumBss = pAd->ApCfg.BssidNum;
#endif
#ifdef CONFIG_FWOWN_SUPPORT
	DriverOwn(pAd);
#endif /* CONFIG_FWOWN_SUPPORT */
#ifdef RTMP_MAC_PCI
	/*  Enable PDMA */
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(%d)::PDMA\n", __func__, __LINE__));
	AsicSetWPDMA(pAd, PDMA_TX_RX, 1);
	/* Make sure get clear FW own interrupt */
	RtmpOsMsDelay(100);
#endif /* RTMP_MAC_PCI */
#ifdef CONFIG_FWOWN_SUPPORT
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(%d)::bDrvOwn(%d)\n", __func__, __LINE__, pAd->bDrvOwn));
#endif /* CONFIG_FWOWN_SUPPORT */
	MCU_CTRL_INIT(pAd);
	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_MCU_SEND_IN_BAND_CMD);
	/*  Send radio on command and wait for ack */
	RTMP_RADIO_ON_OFF_CTRL(pAd, DBDC_BAND_NUM, WIFI_RADIO_ON);
	/* Send Led on command */
	/* Enable RX */
	/* Offlaod below task to AsicExtPmStateCtrl() */
	/* AsicSetMacTxRx(pAd, ASIC_MAC_RX, TRUE); */
	HcSetAllSupportedBandsRadioOn(pAd);
	/*  Resume sending TX packet */
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
}

VOID MTMlmeLpEnter(PRTMP_ADAPTER pAd, struct wifi_dev *wdev)
{
#ifdef CONFIG_AP_SUPPORT
	INT32 IdBss, MaxNumBss = pAd->ApCfg.BssidNum;
	BSS_STRUCT *pMbss = NULL;
#endif /* CONFIG_AP_SUPPORT */
	/*  Stop send TX packets */
	RTMP_OS_NETDEV_STOP_QUEUE(pAd->net_dev);
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		if (MaxNumBss > MAX_MBSSID_NUM(pAd))
			MaxNumBss = MAX_MBSSID_NUM(pAd);

		/* first IdBss must not be 0 (BSS0), must be 1 (BSS1) */
		for (IdBss = FIRST_MBSSID; IdBss < MAX_MBSSID_NUM(pAd); IdBss++) {
			pMbss = &pAd->ApCfg.MBSSID[IdBss];
			if (pMbss->wdev.if_dev)
				RTMP_OS_NETDEV_STOP_QUEUE(pMbss->wdev.if_dev);
		}
		APStop(pAd, pMbss, AP_BSS_OPER_BY_RF);
	}
#endif /* CONFIG_AP_SUPPORT */
	/*  Disable RX */
	/* Below function is offloaded to AsicExtPmStateCtrl() */
	/* AsicSetMacTxRx(pAd, ASIC_MAC_RX, FALSE); */
	/* Set Radio off flag*/
	HcSetAllSupportedBandsRadioOff(pAd);
	/* Delay for CR access */
	msleep(1000);
	/*  Send Led off command */
	/*  Send radio off command and wait for ack */
	RTMP_RADIO_ON_OFF_CTRL(pAd, DBDC_BAND_NUM, WIFI_RADIO_OFF);
	/*  Polling TX/RX path until packets empty */
	MTHifPolling(pAd, HcGetBandByWdev(wdev));
#ifdef RTMP_MAC_PCI
	/*  Disable PDMA */
	AsicSetWPDMA(pAd, PDMA_TX_RX, 0);
#endif /* RTMP_MAC_PCI */
#ifdef CONFIG_FWOWN_SUPPORT
	FwOwn(pAd);
#endif /* CONFIG_FWOWN_SUPPORT */
}


VOID MTPciPollTxRxEmpty(RTMP_ADAPTER *pAd)
{
	UINT32 Loop, Value;
	UINT32 IdleTimes = 0;

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s\n", __func__));
	RtmpOsMsDelay(100);
	/* Fix Rx Ring FULL lead DMA Busy, when DUT is in reset stage */
	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_POLL_IDLE);

	/* Poll Tx until empty */
	for (Loop = 0; Loop < 20000; Loop++) {
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
			return;

		HIF_IO_READ32(pAd, MT_WPDMA_GLO_CFG, &Value);

		if ((Value & TX_DMA_BUSY) == 0x00) {
			IdleTimes++;
			RtmpusecDelay(50);
		}

		if (IdleTimes > 5000)
			break;
	}

	if (Loop >= 20000) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: TX DMA Busy!! WPDMA_GLO_CFG_STRUC = %d\n",
				 __func__, Value));
	}

	IdleTimes = 0;

	/*  Poll Rx to empty */
	for (Loop = 0; Loop < 20000; Loop++) {
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
			return;

		HIF_IO_READ32(pAd, MT_WPDMA_GLO_CFG, &Value);

		if ((Value & RX_DMA_BUSY) == 0x00) {
			IdleTimes++;
			RtmpusecDelay(50);
		}

		if (IdleTimes > 5000)
			break;
	}

	if (Loop >= 20000) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: RX DMA Busy!! WPDMA_GLO_CFG_STRUC = %d\n",
				 __func__, Value));
	}

	/* Fix Rx Ring FULL lead DMA Busy, when DUT is in reset stage */
	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_POLL_IDLE);
}
#endif /* RTMP_MAC_PCI */






#if defined(RTMP_PCI_SUPPORT) || defined(RTMP_RBUS_SUPPORT)
VOID PDMAResetAndRecovery(RTMP_ADAPTER *pAd)
{
	UINT32 RemapBase, RemapOffset;
	UINT32 Value;
	UINT32 RestoreValue;
	UINT32 Loop = 0;
	ULONG IrqFlags;
	struct _PCI_HIF_T *hif = hc_get_hif_ctrl(pAd->hdev_ctrl);

	/* Stop SW Dequeue */
	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DEQUEUEPACKET);
	RTMP_ASIC_INTERRUPT_DISABLE(pAd);
	/* Disable PDMA */
	RT28XXDMADisable(pAd);
	RtmpOsMsDelay(1);
	pAd->RxRest = 1;
	/* Assert csr_force_tx_eof */
	HIF_IO_READ32(pAd, MT_WPDMA_GLO_CFG, &Value);
	Value |= FORCE_TX_EOF;
	HIF_IO_WRITE32(pAd, MT_WPDMA_GLO_CFG, Value);
	/* Infor PSE client of TX abort */
	MAC_IO_READ32(pAd, MCU_PCIE_REMAP_2, &RestoreValue);
	RemapBase = GET_REMAP_2_BASE(RST) << 19;
	RemapOffset = GET_REMAP_2_OFFSET(RST);
	MAC_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RemapBase);
	MAC_IO_READ32(pAd, 0x80000 + RemapOffset, &Value);
	Value |= TX_R_E_1;
	MAC_IO_WRITE32(pAd, 0x80000 + RemapOffset, Value);

	do {
		MAC_IO_READ32(pAd, 0x80000 + RemapOffset, &Value);

		if ((Value & TX_R_E_1_S) == TX_R_E_1_S)
			break;

		RtmpOsMsDelay(1);
		Loop++;
	} while (Loop <= 500);

	if (Loop > 500) {
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR, ("%s: Tx state is not idle(CLIET RST = %x)\n", __func__, Value));
		pAd->PDMAResetFailCount++;
	}

	MAC_IO_READ32(pAd, 0x80000 + RemapOffset, &Value);
	Value |= TX_R_E_2;
	MAC_IO_WRITE32(pAd, 0x80000 + RemapOffset, Value);
	/* Reset PDMA */
	HIF_IO_READ32(pAd, MT_WPDMA_GLO_CFG, &Value);
	Value |= SW_RST;
	HIF_IO_WRITE32(pAd, MT_WPDMA_GLO_CFG, Value);
	Loop = 0;

	/* Polling for PSE client to clear TX FIFO */
	do {
		MAC_IO_READ32(pAd, 0x80000 + RemapOffset, &Value);

		if ((Value & TX_R_E_2_S) == TX_R_E_2_S)
			break;

		RtmpOsMsDelay(1);
		Loop++;
	} while (Loop <= 500);

	if (Loop > 500) {
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR, ("%s: Tx FIFO is not empty(CLIET RST = %x)\n", __func__, Value));
		pAd->PDMAResetFailCount++;
	}

	/* De-assert PSE client TX abort */
	MAC_IO_READ32(pAd, 0x80000 + RemapOffset, &Value);
	Value &= ~TX_R_E_1;
	Value &= ~TX_R_E_2;
	MAC_IO_WRITE32(pAd, 0x80000 + RemapOffset, Value);
	MAC_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RestoreValue);
	/*TODO: Carter, HWBssid idx might not 0 case, check this.*/
	AsicDisableSync(pAd, HW_BSSID_0);
	RTMP_IRQ_LOCK(&hif->BcnRingLock, IrqFlags);
#ifdef CONFIG_AP_SUPPORT

	if (pAd->OpMode == OPMODE_AP) {
		BSS_STRUCT *pMbss;

		pMbss = &pAd->ApCfg.MBSSID[MAIN_MBSSID];
		ASSERT(pMbss);

		if (pMbss)
			pMbss->wdev.bcn_buf.bcn_state = BCN_TX_IDLE;
		else {
			MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR, ("%s():func_dev is NULL!\n", __func__));
			RTMP_IRQ_UNLOCK(&hif->BcnRingLock, IrqFlags);
			return;
		}
	}

#endif
	RTMP_IRQ_UNLOCK(&hif->BcnRingLock, IrqFlags);
	RTMPRingCleanUp(pAd, QID_AC_BE);
	RTMPRingCleanUp(pAd, QID_AC_BK);
	RTMPRingCleanUp(pAd, QID_AC_VI);
	RTMPRingCleanUp(pAd, QID_AC_VO);
	RTMPRingCleanUp(pAd, QID_MGMT);
	RTMPRingCleanUp(pAd, QID_CTRL);
	RTMPRingCleanUp(pAd, QID_BCN);
	RTMPRingCleanUp(pAd, QID_BMC);
	RTMPRingCleanUp(pAd, QID_RX);
#ifdef VENDOR_FEATURE7_SUPPORT
/* Start - Triggered from PDMAWatchDog. Log and try to recover. */
	if (pAd->TxDMAResetCount >= 3) {
		UCHAR event_msg[100] = {0};

		ARRISMOD_CALL(arris_event_send_hook, ATOM_HOST, WLAN_EVENT, ASIC_RELOAD_EVENT,
			"MT7615", strlen("MT7615"));
		snprintf(event_msg, sizeof(event_msg),
			"MT7615 - Hardware error detected - recovery was attempted.\n");
		ARRISMOD_CALL(arris_event_send_hook, ATOM_HOST, WLAN_LOG_SAVE_CONSOLE,
			0, event_msg, strlen(event_msg));
		return;
	}
#endif
	/* AsicEnableBssSync(pAd, pAd->CommonCfg.BeaconPeriod);//Carter check this. */
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

	if (NoDataOut) {
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("TXDMA Reset\n"));
		pAd->TxDMAResetCount++;
		goto reset;
	}

	/* Rx DMA unchanged detect */
	NoDataIn = MonitorRxRing(pAd);

	if (NoDataIn) {
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("RXDMA Reset\n"));
		pAd->RxDMAResetCount++;
		goto reset;
	}

	return;
reset:
	PDMAResetAndRecovery(pAd);
}
#endif

VOID PSEResetAndRecovery(RTMP_ADAPTER *pAd)
{
	UINT32 Loop = 0;
	UINT32 Value;
#ifdef RTMP_PCI_SUPPORT

	if (IS_PCI_INF(pAd))
		NdisAcquireSpinLock(&pAd->IndirectUpdateLock);

#endif
	RTMP_IO_READ32(pAd, 0x816c, &Value);
	Value |= (1 << 0);
	RTMP_IO_WRITE32(pAd, 0x816c, Value);

	do {
		RTMP_IO_READ32(pAd, 0x816c, &Value);

		if ((Value & (1 << 1)) == (1 << 1)) {
			Value &= ~(1 << 1);
			RTMP_IO_WRITE32(pAd, 0x816c, Value);
			break;
		}

		RtmpOsMsDelay(1);
		Loop++;
	} while (Loop <= 500);

	if (Loop > 500) {
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR, ("%s: PSE Reset Fail(%x)\n", __func__, Value));
		pAd->PSEResetFailCount++;
	}

#ifdef RTMP_PCI_SUPPORT

	if (IS_PCI_INF(pAd))
		NdisReleaseSpinLock(&pAd->IndirectUpdateLock);

#endif
#ifdef RTMP_PCI_SUPPORT

	if (IS_PCI_INF(pAd))
		PDMAResetAndRecovery(pAd);

#endif
}


#ifdef RTMP_PCI_SUPPORT
BOOLEAN PciMonitorRxPse(RTMP_ADAPTER *pAd)
{
	UINT32 RemapBase, RemapOffset;
	UINT32 Value;
	UINT32 RestoreValue;

	if (pAd->RxPseCheckTimes < 10) {
		/* Check RX FIFO if not ready */
		MAC_IO_WRITE32(pAd, 0x4244, 0x98000000);
		MAC_IO_READ32(pAd, 0x4244, &Value);

		if ((Value & (1 << 9)) != 0) {
			pAd->RxPseCheckTimes = 0;
			return FALSE;
		}

		MAC_IO_READ32(pAd, MCU_PCIE_REMAP_2, &RestoreValue);
		RemapBase = GET_REMAP_2_BASE(0x800c006c) << 19;
		RemapOffset = GET_REMAP_2_OFFSET(0x800c006c);
		MAC_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RemapBase);
		MAC_IO_WRITE32(pAd, 0x80000 + RemapOffset, 3);
		MAC_IO_READ32(pAd, 0x80000 + RemapOffset, &Value);

		if (((Value & (0x8001 << 16)) == (0x8001 << 16)) ||
			((Value & (0xe001 << 16)) == (0xe001 << 16)) ||
			((Value & (0x4001 << 16)) == (0x4001 << 16)) ||
			((Value & (0x8002 << 16)) == (0x8002 << 16)) ||
			((Value & (0xe002 << 16)) == (0xe002 << 16)) ||
			((Value & (0x4002 << 16)) == (0x4002 << 16))) {
			if (((Value & (0x8001 << 16)) == (0x8001 << 16)) ||
				((Value & (0xe001 << 16)) == (0xe001 << 16)) ||
				((Value & (0x8002 << 16)) == (0x8002 << 16)) ||
				((Value & (0xe002 << 16)) == (0xe002 << 16)))
				pAd->PSETriggerType1Count++;

			if (((Value & (0x4001 << 16)) == (0x4001 << 16)) ||
				((Value & (0x4002 << 16)) == (0x4002 << 16)))
				pAd->PSETriggerType2Count++;

			pAd->RxPseCheckTimes++;
			MAC_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RestoreValue);
			return FALSE;
		}

		pAd->RxPseCheckTimes = 0;
		MAC_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RestoreValue);
		return FALSE;
	}

	pAd->RxPseCheckTimes = 0;
	return TRUE;
}
#endif




#ifdef RTMP_PCI_SUPPORT
#endif /* RTMP_PCI_SUPPORT */


VOID PSEWatchDog(RTMP_ADAPTER *pAd)
{
	BOOLEAN NoDataIn = FALSE;
#ifdef RTMP_PCI_SUPPORT

	if (IS_PCI_INF(pAd))
		NoDataIn = PciMonitorRxPse(pAd);

#endif

	if (NoDataIn) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("PSE Reset:MonitorRxPse\n"));
		pAd->PSEResetCount++;
		goto reset;
	}

#ifdef RTMP_PCI_SUPPORT
#endif /* RTMP_PCI_SUPPORT */
	return;
reset:
	;
	PSEResetAndRecovery(pAd);
}

VOID mt_bcn_buf_init(RTMP_ADAPTER *pAd)
{
	struct _RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	pChipCap->FlgIsSupSpecBcnBuf = FALSE;
	pChipCap->BcnMaxHwNum = 16;
	pChipCap->BcnMaxNum = 16;
#if defined(MT7603_FPGA) || defined(MT7628_FPGA) || defined(MT7636_FPGA) || defined(MT7637_FPGA)
	pChipCap->WcidHwRsvNum = 20;
#else
	pChipCap->WcidHwRsvNum = 127;
#endif /* MT7603_FPGA */
	pChipCap->BcnMaxHwSize = 0x2000;  /* useless!! */
	pChipCap->BcnBase[0] = 0;
	pChipCap->BcnBase[1] = 0;
	pChipCap->BcnBase[2] = 0;
	pChipCap->BcnBase[3] = 0;
	pChipCap->BcnBase[4] = 0;
	pChipCap->BcnBase[5] = 0;
	pChipCap->BcnBase[6] = 0;
	pChipCap->BcnBase[7] = 0;
	pChipCap->BcnBase[8] = 0;
	pChipCap->BcnBase[9] = 0;
	pChipCap->BcnBase[10] = 0;
	pChipCap->BcnBase[11] = 0;
	pChipCap->BcnBase[12] = 0;
	pChipCap->BcnBase[13] = 0;
	pChipCap->BcnBase[14] = 0;
	pChipCap->BcnBase[15] = 0;
	ops->BeaconUpdate = NULL;

	/* TODO: shiang-7603 */
	if (pChipCap->hif_type == HIF_MT) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): Not support for HIF_MT yet!\n",
				 __func__, __LINE__));
	}
}


VOID mt_chip_bcn_parameter_init(RTMP_ADAPTER *pAd)
{
	struct _RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	pChipCap->FlgIsSupSpecBcnBuf = FALSE;
	pChipCap->BcnMaxHwNum = 16;
	pChipCap->BcnMaxNum = 16;
#if defined(MT7603_FPGA) || defined(MT7628_FPGA) || defined(MT7636_FPGA)
	pChipCap->WcidHwRsvNum = 20;
#else
	pChipCap->WcidHwRsvNum = 127;
#endif /* MT7603_FPGA */
	ops->BeaconUpdate = NULL;
}

#ifdef CONFIG_AP_SUPPORT
/*
 * NOTE:
 * this function is for MT7628/MT7603/MT7636 only.
 *
 * MT7636 has no MBSS function.
 * but below to MT MBSS gen1 mac address assignment rule
 */
VOID MtAsicSetMbssWdevIfAddrGen1(struct _RTMP_ADAPTER *pAd, INT idx, UCHAR *if_addr, INT opmode)
{
	UINT32 Value = 0;
	UCHAR MacByte = 0, MacMask = 0;

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s-%s\n", __FILE__, __func__));

	if (opmode == OPMODE_AP) {
		COPY_MAC_ADDR(if_addr, pAd->CurrentAddress);
		/* read BTEIR bit[31:29] for determine to choose which byte to extend BSSID mac address.*/
		MAC_IO_READ32(pAd, LPON_BTEIR, &Value);
		/* Note: Carter, make default will use byte4 bit[31:28] to extend Mac Address */
		Value = Value | (0x2 << 29);
		MAC_IO_WRITE32(pAd, LPON_BTEIR, Value);
		MacByte = Value >> 29;
		MAC_IO_READ32(pAd, RMAC_RMACDR, &Value);
		Value = Value & ~RMACDR_MBSSID_MASK;

		if (pAd->ApCfg.BssidNum <= 2) {
			Value |= RMACDR_MBSSID(0x0);
			MacMask = 0xef;
		} else if (pAd->ApCfg.BssidNum <= 4) {
			Value |= RMACDR_MBSSID(0x1);
			MacMask = 0xcf;
		} else if (pAd->ApCfg.BssidNum <= 8) {
			Value |= RMACDR_MBSSID(0x2);
			MacMask = 0x8f;
		} else if (pAd->ApCfg.BssidNum <= 16) {
			Value |= RMACDR_MBSSID(0x3);
			MacMask = 0x0f;
		} else {
			Value |= RMACDR_MBSSID(0x3);
			MacMask = 0x0f;
		}

		MAC_IO_WRITE32(pAd, RMAC_RMACDR, Value);

		if (idx > 0) {
			/* MT7603, bit1 in byte0 shall always be b'1 for Multiple BSSID */
			if_addr[0] |= 0x2;

			switch (MacByte) {
			case 0x1: /* choose bit[23:20]*/
				if_addr[2] = if_addr[2] & MacMask;/* clear high 4 bits, */
				if_addr[2] = (if_addr[2] | (idx << 4));
				break;

			case 0x2: /* choose bit[31:28]*/
				if_addr[3] = if_addr[3] & MacMask;/* clear high 4 bits, */
				if_addr[3] = (if_addr[3] | (idx << 4));
				break;

			case 0x3: /* choose bit[39:36]*/
				if_addr[4] = if_addr[4] & MacMask;/* clear high 4 bits, */
				if_addr[4] = (if_addr[4] | (idx << 4));
				break;

			case 0x4: /* choose bit [47:44]*/
				if_addr[5] = if_addr[5] & MacMask;/* clear high 4 bits, */
				if_addr[5] = (if_addr[5] | (idx << 4));
				break;

			default: /* choose bit[15:12]*/
				if_addr[1] = if_addr[1] & MacMask;/* clear high 4 bits, */
				if_addr[1] = (if_addr[1] | (idx << 4));
				break;
			}
		}
	}
}

/*
 * NOTE: 2015-April-2.
 * this function is for MT7637/MT7615 and afterward chips
 */
VOID MtAsicSetMbssWdevIfAddrGen2(struct _RTMP_ADAPTER *pAd, INT idx, UCHAR *if_addr, INT opmode)
{
	UCHAR zeroMac[6] = {0};
	UCHAR MacMask = 0;

	if (pAd->ApCfg.BssidNum <= 2)
		MacMask = 0xef;
	else if (pAd->ApCfg.BssidNum <= 4)
		MacMask = 0xcf;
	else if (pAd->ApCfg.BssidNum <= 8)
		MacMask = 0x8f;
	else if (pAd->ApCfg.BssidNum <= 16)
		MacMask = 0x0f;
	else
		MacMask = 0x0f;

	if (idx > 0) {
		if (NdisEqualMemory(zeroMac, pAd->ExtendMBssAddr[idx - 1], MAC_ADDR_LEN)) {
			COPY_MAC_ADDR(if_addr, pAd->CurrentAddress);
			if_addr[0] |= 0x2;
			/* default choose bit[31:28], if there is no assigned mac from profile. */
			if_addr[3] = if_addr[3] & MacMask;/* clear high 4 bits, */
			if_addr[3] = (if_addr[3] | (idx << 4));
			COPY_MAC_ADDR(pAd->ExtendMBssAddr[idx - 1], if_addr);
		} else
			COPY_MAC_ADDR(if_addr, pAd->ExtendMBssAddr[idx - 1]);

		/*
		 * MT7615 and the chip afterward,
		 * the mac addr write to CR shall offload to BssInfoUpdate by EXT_MBSS tag
		 */
		/* MtAsicSetExtMbssMacByDriver(pAd, idx, wdev); */
	} else
		COPY_MAC_ADDR(if_addr, pAd->CurrentAddress);

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s-%s mbss_idx = %d, if_addr = %x %x %x %x %x %x\n",
			  __FILE__, __func__, idx,
			  if_addr[0], if_addr[1], if_addr[2],
			  if_addr[3], if_addr[4], if_addr[5]));
}
#endif /*CONFIG_AP_SUPPORT*/

#ifdef PCIE_ASPM_DYM_CTRL_SUPPORT
VOID mt_asic_pcie_aspm_dym_ctrl(RTMP_ADAPTER *pAd, UINT8 ucDbdcIdx, BOOLEAN fgL1Enable, BOOLEAN fgL0sEnable)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (!get_pcie_aspm_dym_ctrl_cap(pAd)) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s::retun since cap=0\n", __func__));
		return;
	}

	if (ops->pcie_aspm_dym_ctrl)
		ops->pcie_aspm_dym_ctrl(pAd, ucDbdcIdx, fgL1Enable, fgL0sEnable);
	else
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("For this chip, no specified dynamic aspm ctrl function!\n"));
}
#endif /* #ifdef PCIE_ASPM_DYM_CTRL_SUPPORT */

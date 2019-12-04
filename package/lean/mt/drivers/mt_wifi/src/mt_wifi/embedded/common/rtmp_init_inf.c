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
	rtmp_init_inf.c

	Abstract:
	Miniport generic portion header file

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/
#include	"rt_config.h"
#ifdef DOT11R_FT_SUPPORT
#include	"ft.h"
#endif /* DOT11R_FT_SUPPORT */

#define PROBE2LOAD_L1PROFILE /* Capable to be turned off if not required */


#ifdef MULTI_PROFILE
VOID multi_profile_exit(struct _RTMP_ADAPTER *ad);
#endif /*MULTI_PROFILE*/

#ifdef LINUX
#ifdef OS_ABL_FUNC_SUPPORT
/* Utilities provided from NET module */
RTMP_NET_ABL_OPS RtmpDrvNetOps, *pRtmpDrvNetOps = &RtmpDrvNetOps;
RTMP_PCI_CONFIG RtmpPciConfig, *pRtmpPciConfig = &RtmpPciConfig;
RTMP_USB_CONFIG RtmpUsbConfig, *pRtmpUsbConfig = &RtmpUsbConfig;

VOID RtmpDrvOpsInit(
	OUT VOID *pDrvOpsOrg,
	INOUT VOID *pDrvNetOpsOrg,
	IN RTMP_PCI_CONFIG *pPciConfig,
	IN RTMP_USB_CONFIG *pUsbConfig)
{
	RTMP_DRV_ABL_OPS *pDrvOps = (RTMP_DRV_ABL_OPS *)pDrvOpsOrg;

	/* init PCI/USB configuration in different OS */
	if (pPciConfig != NULL)
		RtmpPciConfig = *pPciConfig;

	if (pUsbConfig != NULL)
		RtmpUsbConfig = *pUsbConfig;

	/* init operators provided from us (DRIVER module) */
	pDrvOps->RTMPAllocAdapterBlock = RTMPAllocAdapterBlock;
	pDrvOps->RTMPFreeAdapter = RTMPFreeAdapter;
	pDrvOps->RtmpRaDevCtrlExit = RtmpRaDevCtrlExit;
	pDrvOps->RtmpRaDevCtrlInit = RtmpRaDevCtrlInit;
#ifdef RTMP_MAC_PCI
	pDrvOps->RTMPHandleInterrupt = mtd_isr;
#endif /* RTMP_MAC_PCI */
	pDrvOps->RTMPSendPackets = RTMPSendPackets;
	pDrvOps->RTMP_COM_IoctlHandle = RTMP_COM_IoctlHandle;
#ifdef CONFIG_AP_SUPPORT
	pDrvOps->RTMP_AP_IoctlHandle = RTMP_AP_IoctlHandle;
#endif /* CONFIG_AP_SUPPORT */
	pDrvOps->RTMPDrvOpen = RTMPDrvOpen;
	pDrvOps->RTMPDrvClose = RTMPDrvClose;
	pDrvOps->mt_wifi_init = mt_wifi_init;
	/* init operators provided from us and netif module */
}

RTMP_BUILD_DRV_OPS_FUNCTION_BODY

#endif /* OS_ABL_FUNC_SUPPORT */
#endif /* LINUX */


INT rtmp_cfg_exit(RTMP_ADAPTER *pAd)
{
	UserCfgExit(pAd);
	return TRUE;
}


INT rtmp_cfg_init(RTMP_ADAPTER *pAd, RTMP_STRING *pHostName)
{
	NDIS_STATUS status;
	UserCfgInit(pAd);
#ifdef FTM_SUPPORT
	FtmMgmtInit(pAd);
#endif /* FTM_SUPPORT */
#ifdef MBO_SUPPORT
	MboInit(pAd);
#endif /* MBO_SUPPORT */
	CfgInitHook(pAd);

	/*
		WiFi system operation mode setting base on following partitions:
		1. Parameters from config file
		2. Hardware cap from EEPROM
		3. Chip capabilities in code
	*/
	if (pAd->RfIcType == 0) {
		/* RfIcType not assigned, should not happened! */
		pAd->RfIcType = RFIC_UNKNOWN;
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): Invalid RfIcType, reset it first\n",
				 __func__));
	}

	status = RTMPReadParametersHook(pAd);

	if (status != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("RTMPReadParametersHook failed, Status[=0x%08x]\n", status));
		return FALSE;
	}

	/*check all enabled function, decide the max unicast wtbl idx will use.*/
	/*After RTMPReadParameterHook to get MBSSNum & MSTANum*/
	HcSetMaxStaNum(pAd);
	return TRUE;
}


INT rtmp_mgmt_init(RTMP_ADAPTER *pAd)
{
	return TRUE;
}


static INT rtmp_sys_exit(RTMP_ADAPTER *pAd)
{
#ifdef SMART_ANTENNA
	RtmpSAExit(pAd);
#endif /* SMART_ANTENNA */
	MeasureReqTabExit(pAd);
	TpcReqTabExit(pAd);
#ifdef DOT11_N_SUPPORT

	if (pAd->mpdu_blk_pool.mem) {
		os_free_mem(pAd->mpdu_blk_pool.mem); /* free BA pool*/
		pAd->mpdu_blk_pool.mem = NULL;
	}

#endif /* DOT11_N_SUPPORT */
	rtmp_cfg_exit(pAd);
	HwCtrlExit(pAd);
	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_REGISTER_TO_OS);
	RtmpMgmtTaskExit(pAd);
#ifdef RTMP_TIMER_TASK_SUPPORT
	NdisFreeSpinLock(&pAd->TimerQLock);
#endif
	return TRUE;
}


static INT rtmp_sys_init(RTMP_ADAPTER *pAd, RTMP_STRING *pHostName)
{
	NDIS_STATUS status;

	wifi_sys_reset(&pAd->WifiSysInfo);
#ifdef DBG_STARVATION
	starv_log_init(&pAd->starv_log_ctrl);
#endif /*DBG_STARVATION*/

	status = RtmpMgmtTaskInit(pAd);

	if (status != NDIS_STATUS_SUCCESS)
		goto err0;

	status = HwCtrlInit(pAd);

	if (status != NDIS_STATUS_SUCCESS)
		goto err1;

	/* Initialize pAd->StaCfg[], pAd->ApCfg, pAd->CommonCfg to manufacture default*/
	if (rtmp_cfg_init(pAd, pHostName) != TRUE)
		goto err2;

#ifdef DOT11_N_SUPPORT

	/* Allocate BA Reordering memory*/
	if (ba_reordering_resource_init(pAd, MAX_REORDERING_MPDU_NUM) != TRUE)
		goto err2;

#endif /* DOT11_N_SUPPORT */
#ifdef BLOCK_NET_IF
	initblockQueueTab(pAd);
#endif /* BLOCK_NET_IF */
	status = MeasureReqTabInit(pAd);

	if (status != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("MeasureReqTabInit failed, Status[=0x%08x]\n", status));
		goto err2;
	}

	status = TpcReqTabInit(pAd);

	if (status != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("TpcReqTabInit failed, Status[=0x%08x]\n", status));
		goto err2;
	}

#ifdef SMART_ANTENNA
	RtmpSAInit(pAd);
#endif /* SMART_ANTENNA */
#ifdef MT_MAC
	/* TxS Setting */
	InitTxSTypeTable(pAd);

	if (IS_HIF_TYPE(pAd, HIF_MT))
		InitTxSCommonCallBack(pAd);

#endif

	status = tr_ctl_init(pAd);

	/* QM init */
	status = qm_init(pAd);

	/* TM init */
	status = tm_init(pAd);

	if (status)
		goto err2;

#ifdef FQ_SCH_SUPPORT
	if (pAd->fq_ctrl.enable & FQ_NEED_ON) {
		if (set_fq_enable(pAd, "4105-2") == FALSE)
			goto err2;
	}
#endif


	return TRUE;
err2:
	rtmp_cfg_exit(pAd);
err1:
	HwCtrlExit(pAd);
err0:
	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_REGISTER_TO_OS);
	RtmpMgmtTaskExit(pAd);
	return FALSE;
}

#ifdef NF_SUPPORT
VOID enable_nf_support(VOID *pAdSrc)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pAdSrc;
	int Value;

	pAd->Avg_NF = 0;
	pAd->Avg_NFx16 = 0;
	HW_IO_READ32(pAd, PHY_RXTD_12, &Value);
	Value |= (1 << B0IrpiSwCtrlResetOffset);
	Value |= (1 << B0IrpiSwCtrlOnlyOffset);
	HW_IO_WRITE32(pAd, PHY_RXTD_12, Value);
	HW_IO_WRITE32(pAd, PHY_RXTD_12, Value);
	/* Enable badn0 IPI control */
	HW_IO_READ32(pAd, PHY_BAND0_PHYMUX_5, &Value);
	Value |= (B0IpiEnableCtrlValue << B0IpiEnableCtrlOffset);
	HW_IO_WRITE32(pAd, PHY_BAND0_PHYMUX_5, Value);
}
#endif

/*
*
*/
static void mt_sys_ready(struct _RTMP_ADAPTER *ad)
{
	/* TODO: shiang-MT7615, why we need do this and check if chip is MT7603/28/36/MT7615 here?? */
	if (IS_HIF_TYPE(ad, HIF_MT)) {
		/* Now Enable RxTx*/
		RTMP_IRQ_ENABLE(ad);
		RTMPEnableRxTx(ad);
	}

	RTMP_SET_FLAG(ad, fRTMP_ADAPTER_START_UP);
}

/*rename from rt28xx_init*/
int mt_wifi_init(VOID *pAdSrc, RTMP_STRING *pDefaultMac, RTMP_STRING *pHostName)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pAdSrc;
	NDIS_STATUS Status;
	/* UCHAR EDCCACtrl; */
	UCHAR ucBandIdx = 0;
	struct _RTMP_CHIP_CAP *cap = NULL;
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	struct wifi_dev *wdev = NULL;
	UCHAR BandIdx;
	CHANNEL_CTRL *pChCtrl;
	CHANNEL_CTRL *pChCtrl_hwband1;

	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\x1b[1;33m [mt_wifi_init] Test - pObj->ioctl_if = %d, pObj->ioctl_if_type = %d \x1b[m \n", pObj->ioctl_if, pObj->ioctl_if_type));
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		wdev = &pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev;
	}
#endif
	if (!wdev) {
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[mt_wifi_init] wdev == NULL\n"));
		return FALSE;
	}
	BandIdx = HcGetBandByWdev(wdev);
	pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);
	pChCtrl_hwband1 = hc_get_channel_ctrl(pAd->hdev_ctrl, 1);

	cap = hc_get_chip_cap(pAd->hdev_ctrl);

#ifdef CONFIG_FWOWN_SUPPORT
	DriverOwn(pAd);
#endif
	mt_chip_info_show(pAd);

	/* TODO: shiang-usw, need to check this for RTMP_MAC */
	/* Disable interrupts here which is as soon as possible*/
	/* This statement should never be true. We might consider to remove it later*/
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_ACTIVE))
		RTMP_ASIC_INTERRUPT_DISABLE(pAd);

	/* reset Adapter flags */
	RTMP_CLEAR_FLAGS(pAd);
	rtmp_hif_data_init(pAd);

	/*for software system initialize*/
	if (rtmp_sys_init(pAd, pHostName) != TRUE)
		goto err2;

	Status = WfInit(pAd);

	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("WfInit faild!!, ret=%d, cap=%p\n", Status, cap));
		goto err2;
	}

	/* initialize MLME*/
	Status = MlmeInit(pAd);

	if (Status != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("MlmeInit failed, Status[=0x%08x]\n", Status));
		goto err3;
	}

	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_REGISTER_TO_OS);
#ifdef CCK_LQ_SUPPORT
	if ((IS_MT7615(pAd)) && (pAd->CommonCfg.cfg_wmode & WMODE_B)) {
		/* Configure HW to send RXV frame to driver */
		MtAsicSetMacTxRx(pAd, ASIC_MAC_TXRX_RXV, TRUE, 0);
		MtAsicSetMacTxRx(pAd, ASIC_MAC_TXRX_RXV, TRUE, 1);
	}
#endif
#ifdef NF_SUPPORT
	/* Enable Noise Histrogram */
	if (IS_MT7615(pAd)) {
		MAC_IO_WRITE32(pAd, 0x12234, 0x07000000);
		MAC_IO_WRITE32(pAd, PHY_BAND0_PHYMUX_5, 0x50DC10);
	}
#endif
	NICInitializeAsic(pAd);
#ifdef LED_CONTROL_SUPPORT
	/* Send LED Setting to MCU */
	RTMPInitLEDMode(pAd);
#endif /* LED_CONTROL_SUPPORT */
	tx_pwr_comp_init(pAd);
#ifdef WIN_NDIS

	/* Patch cardbus controller if EEPROM said so. */
	if (pAd->bTest1 == FALSE)
		RTMPPatchCardBus(pAd);

#endif /* WIN_NDIS */
#ifdef IKANOS_VX_1X0
	VR_IKANOS_FP_Init(pAd->ApCfg.BssidNum, pAd->PermanentAddress);
#endif /* IKANOS_VX_1X0 */
#ifdef CONFIG_ATE
	rtmp_ate_init(pAd);
#endif /*CONFIG_ATE*/
	/* Microsoft HCT require driver send a disconnect event after driver initialization.*/
	/* STA_STATUS_CLEAR_FLAG(pStaCfg, fSTA_STATUS_MEDIA_STATE_CONNECTED); */
	OPSTATUS_CLEAR_FLAG(pAd, fOP_AP_STATUS_MEDIA_STATE_CONNECTED);
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("NDIS_STATUS_MEDIA_DISCONNECT Event B!\n"));
#ifdef SMART_CARRIER_SENSE_SUPPORT
	SCS_init(pAd);
#endif /* SMART_CARRIER_SENSE_SUPPORT */
	RTMPIoctlRvRDebug_Init(pAd);
#ifdef MAC_INIT_OFFLOAD
	AsicSetMacTxRx(pAd, ASIC_MAC_TXRX, TRUE);
#endif /*MAC_INIT_OFFLOAD*/
#ifdef CONFIG_AP_SUPPORT
	RT_CONFIG_IF_OPMODE_ON_AP(pAd->OpMode) {
		rtmp_ap_init(pAd);
	}
#endif
	/*SW prepare done, enable system ready*/
	mt_sys_ready(pAd);
#ifdef DYNAMIC_VGA_SUPPORT

	if (pAd->CommonCfg.lna_vga_ctl.bDyncVgaEnable)
		dynamic_vga_enable(pAd);

#endif /* DYNAMIC_VGA_SUPPORT */

	/* Set PHY to appropriate mode and will update the ChannelListNum in this function */

	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\x1b[1;33m [mt_wifi_init] Test - BandIdx = %d, pChCtrl->ChListNum = %d, pChCtrl_hwband1->ChListNum = %d \x1b[m \n", BandIdx, pChCtrl->ChListNum, pChCtrl_hwband1->ChListNum));

	/* TBD: if (pChCtrl->ChListNum == 0) { */
	if (0) {
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("Wrong configuration. No valid channel found. Check \"ContryCode\" and \"ChannelGeography\" setting.\n"));
		goto err3;
	}

#ifdef UAPSD_SUPPORT
	UAPSD_Init(pAd);
#endif /* UAPSD_SUPPORT */
	/* assign function pointers*/
#ifdef MAT_SUPPORT
	/* init function pointers, used in OS_ABL */
	RTMP_MATOpsInit(pAd);
#endif /* MAT_SUPPORT */
#ifdef STREAM_MODE_SUPPORT
	AsicStreamModeInit(pAd);
#endif /* STREAM_MODE_SUPPORT */
#ifdef MT_WOW_SUPPORT
	ASIC_WOW_INIT(pAd);
#endif
#ifdef USB_IOT_WORKAROUND2
	pAd->bUSBIOTReady = TRUE;
#endif
#ifdef CONFIG_AP_SUPPORT
	AutoChSelInit(pAd);
#endif /* CONFIG_AP_SUPPORT */
#ifdef REDUCE_TCP_ACK_SUPPORT
	ReduceAckInit(pAd);
#endif

	/* Trigger MIB counter update */
	for (ucBandIdx = 0; ucBandIdx < DBDC_BAND_NUM; ucBandIdx++)
		pAd->OneSecMibBucket.Enabled[ucBandIdx] = TRUE;

	pAd->MsMibBucket.Enabled = TRUE;
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("<==== mt_wifi_init, Status=%x\n", Status));
#if defined(TXBF_SUPPORT) && defined(MT_MAC) && (!defined(MT7636))
	TxBfModuleEnCtrl(pAd);

	mt_Trigger_Sounding_Packet(pAd,
							   TRUE,
							   0,
							   BF_PROCESSING,
							   0,
							   NULL);
	AsicTxBfHwEnStatusUpdate(pAd,
							 pAd->CommonCfg.ETxBfEnCond,
							 pAd->CommonCfg.RegTransmitSetting.field.ITxBfEn);
#ifdef TXBF_DYNAMIC_DISABLE
	pAd->CommonCfg.ucAutoSoundingCtrl = 0;/* After interface down up, BF disable will be cancelled */
#endif /* TXBF_DYNAMIC_DISABLE */
#endif /* TXBF_SUPPORT */

	/* EDCCA support */
	/* EDCCACtrl = GetEDCCASupport(pAd); */

	for (ucBandIdx = 0; ucBandIdx < DBDC_BAND_NUM; ucBandIdx++) {
		/* pAd->CommonCfg.ucEDCCACtrl[BandIdx] = EDCCACtrl; */
		EDCCACtrlCmd(pAd, ucBandIdx, pAd->CommonCfg.ucEDCCACtrl[ucBandIdx]);
	}

#ifdef FTM_SUPPORT
	TmrCtrl(pAd, TMR_INITIATOR, cap->TmrHwVer);
#endif
	return TRUE;
err3:
	MlmeHalt(pAd);
	RTMP_AllTimerListRelease(pAd);
err2:
	rtmp_sys_exit(pAd);
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("!!! mt_wifi_init  fail !!!\n"));
	return FALSE;
}


VOID RTMPDrvOpen(VOID *pAdSrc)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pAdSrc;
	RTMP_CLEAR_PSFLAG(pAd, fRTMP_PS_MCU_SLEEP);
	/*check all enabled function, decide the max unicast wtbl idx will use.*/
	HcSetMaxStaNum(pAd);
	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_START_UP);
#ifdef CONFIG_AP_SUPPORT
#ifdef BG_FT_SUPPORT
	BG_FTPH_Init();
#endif /* BG_FT_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
#ifdef WSC_INCLUDED
#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */
	/* WSC hardware push button function 0811 */
	WSC_HDR_BTN_Init(pAd);
#endif /* WSC_INCLUDED */
#ifdef COEX_SUPPORT
	/* SendAndesWLANStatus(pAd,WLAN_Device_ON,0); */
	if (IS_MT76x6(pAd) || IS_MT7637(pAd) || IS_MT7622(pAd))
		MT76xxMLMEHook(pAd, MT76xx_WLAN_Device_ON, 0);
#endif /* COEX_SUPPORT */
#ifdef MT_WOW_SUPPORT
	pAd->WOW_Cfg.bWoWRunning = FALSE;
#endif
#ifdef CONFIG_AP_SUPPORT
#ifdef VOW_SUPPORT

	if (IS_MT7615(pAd) || IS_MT7622(pAd) || IS_P18(pAd) || IS_MT7663(pAd))
		vow_init(pAd);

#else
	vow_atf_off_init(pAd);
#endif /* VOW_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
#ifdef RED_SUPPORT

	if (pAd->OpMode == OPMODE_AP)
		RedInit(pAd);

#endif /* RED_SUPPORT */
	cp_support_is_enabled(pAd);

#if defined(MT_DFS_SUPPORT) && defined(BACKGROUND_SCAN_SUPPORT)

	if (IS_MT7615(pAd)) {
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("Trigger DFS Zero wait procedure Support=%d, DfsZeroWaitChannel=%d", pAd->BgndScanCtrl.DfsZeroWaitSupport,
				  pAd->BgndScanCtrl.DfsZeroWaitChannel));

#ifdef BACKGROUND_SCAN_SUPPORT
		/*DfsDedicatedScanStart(pAd);*/
		DfsSetInitDediatedScanStart(pAd);

#endif
	}

#endif /* defined(MT_DFS_SUPPORT) && defined(BACKGROUND_SCAN_SUPPORT) */
#ifdef BAND_STEERING
#ifdef CONFIG_AP_SUPPORT
    if (pAd->ApCfg.BandSteering) {
	PBND_STRG_CLI_TABLE table;

	table = Get_BndStrgTable(pAd, BSS0);
	if (table) {
	    /* Inform daemon interface ready */
	    struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[BSS0].wdev;

	    BndStrg_SetInfFlags(pAd, wdev, table, TRUE);
	}
    }
#endif /* CONFIG_AP_SUPPORT */
#endif /* BAND_STEERING */
}


VOID RTMPDrvClose(VOID *pAdSrc, VOID *net_dev)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pAdSrc;
	UINT32 i = 0;
	struct MCU_CTRL *prCtl = NULL;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 num_of_tx_ring = GET_NUM_OF_TX_RING(cap);
	prCtl = &pAd->MCUCtrl;
#ifdef CONFIG_AP_SUPPORT
#ifdef BG_FT_SUPPORT
	BG_FTPH_Remove();
#endif /* BG_FT_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_POLL_IDLE);
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT))
#endif /* MT_MAC */
		RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);

#ifdef EXT_BUILD_CHANNEL_LIST

	if (pAd->CommonCfg.pChDesp != NULL)
		os_free_mem(pAd->CommonCfg.pChDesp);

	pAd->CommonCfg.pChDesp = NULL;
	pAd->CommonCfg.DfsType = MAX_RD_REGION;
	pAd->CommonCfg.bCountryFlag = 0;
#endif /* EXT_BUILD_CHANNEL_LIST */
	pAd->CommonCfg.bCountryFlag = FALSE;

	for (i = 0; i < num_of_tx_ring; i++) {
		while (pAd->DeQueueRunning[i] == TRUE) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Waiting for TxQueue[%d] done..........\n", i));
			RtmpusecDelay(1000);
		}
	}

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		BOOLEAN Cancelled = FALSE;
#ifdef DOT11N_DRAFT3

		if (pAd->CommonCfg.Bss2040CoexistFlag & BSS_2040_COEXIST_TIMER_FIRED) {
			RTMPCancelTimer(&pAd->CommonCfg.Bss2040CoexistTimer, &Cancelled);
			pAd->CommonCfg.Bss2040CoexistFlag  = 0;
		}

#endif /* DOT11N_DRAFT3 */
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
#ifdef MAT_SUPPORT
		MATEngineExit(pAd);
#endif /* MAT_SUPPORT */
#ifdef CLIENT_WDS
		CliWds_ProxyTabDestory(pAd);
#endif /* CLIENT_WDS */
		/* Shutdown Access Point function, release all related resources */
		APShutdown(pAd);
		/*#ifdef AUTO_CH_SELECT_ENHANCE*/
		/* Free BssTab & ChannelInfo tabbles.*/
		/*		AutoChBssTableDestroy(pAd); */
		/*		ChannelInfoDestroy(pAd); */
		/*#endif  AUTO_CH_SELECT_ENHANCE */
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_ATE
	ATEExit(pAd);
#endif /*CONFIG_ATE*/
	/* Stop Mlme state machine*/
	MlmeHalt(pAd);
	/* Close net tasklets*/
		NICRestartFirmware(pAd);


	/*  Disable Interrupt */
	if (IS_MT7637(pAd)) { /* workaround for MtCmdRestartDLReq */
#ifdef RTMP_PCI_SUPPORT
		/*  Polling TX/RX path until packets empty */
		MTPciPollTxRxEmpty(pAd);
		/*  Disable PDMA */
		AsicSetWPDMA(pAd, PDMA_TX_RX, 0);
		/*  Polling TX/RX path until packets empty */
		MTPciPollTxRxEmpty(pAd);
#endif
		HcSetAllSupportedBandsRadioOff(pAd);

		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_ACTIVE))
			RTMP_ASIC_INTERRUPT_DISABLE(pAd);
	}

	MeasureReqTabExit(pAd);
	TpcReqTabExit(pAd);
#ifdef LED_CONTROL_SUPPORT
	RTMPExitLEDMode(pAd);
#endif /* LED_CONTROL_SUPPORT */
	/* Close kernel threads*/
	RtmpMgmtTaskExit(pAd);
#ifdef RTMP_MAC_PCI
	{
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_ACTIVE)) {
			DISABLE_TX_RX(pAd, RTMP_HALT);
			RTMP_ASIC_INTERRUPT_DISABLE(pAd);
		}

		/* Receive packets to clear DMA index after disable interrupt. */
		/*RTMPHandleRxDoneInterrupt(pAd);*/
		/* put to radio off to save power when driver unload.  After radiooff, can't write /read register.  So need to finish all */
		/* register access before Radio off.*/
#ifdef RTMP_PCI_SUPPORT

		if (pAd->infType == RTMP_DEV_INF_PCI || pAd->infType == RTMP_DEV_INF_PCIE) {
			BOOLEAN brc = TRUE;
			brc = RT28xxPciAsicRadioOff(pAd, RTMP_HALT, 0);

			if (brc == FALSE)
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s call RT28xxPciAsicRadioOff fail !!\n", __func__));
		}

#endif /* RTMP_PCI_SUPPORT */
	}
#endif /* RTMP_MAC_PCI */

	/* Free IRQ*/
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_REGISTER_TO_OS)) {
#ifdef RTMP_MAC_PCI
		/* Deregister interrupt function*/
		RTMP_OS_IRQ_RELEASE(pAd, net_dev);
#endif /* RTMP_MAC_PCI */
		RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_REGISTER_TO_OS);
	}

#ifdef SINGLE_SKU_V2
    RTMPResetSkuParam(pAd);
    RTMPResetBackOffParam(pAd);
#endif
	tr_ctl_exit(pAd);

	/* tm exit */
	tm_exit(pAd);

	/* qm exit */
	qm_exit(pAd);
	/*remove hw related system info*/
	WfSysPosExit(pAd);
	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);
#ifdef WLAN_SKB_RECYCLE
	skb_queue_purge(&pAd->rx0_recycle);
#endif /* WLAN_SKB_RECYCLE */
	/* Free BA reorder resource*/
	ba_reordering_resource_release(pAd);
	UserCfgExit(pAd); /* must after ba_reordering_resource_release */
#ifdef MT_MAC

	if (IS_HIF_TYPE(pAd, HIF_MT))
		ExitTxSTypeTable(pAd);

#endif
#ifdef BACKGROUND_SCAN_SUPPORT
	BackgroundScanDeInit(pAd);
#endif /* BACKGROUND_SCAN_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
	AutoChSelRelease(pAd);
#endif/* CONFIG_AP_SUPPORT */
	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_START_UP);
	/*+++Modify by woody to solve the bulk fail+++*/
	/* clear MAC table */
	/* TODO: do not clear spin lock, such as fLastChangeAccordingMfbLock */
	NdisZeroMemory(&pAd->MacTab, sizeof(MAC_TABLE));
	/* release all timers */
	RtmpusecDelay(2000);
	RTMP_AllTimerListRelease(pAd);
	/* WCNCR00034259: moved from RTMP{Reset, free}TxRxRingMemory() */
	NdisFreeSpinLock(&pAd->CmdQLock);
#ifdef RTMP_TIMER_TASK_SUPPORT
	NdisFreeSpinLock(&pAd->TimerQLock);
#endif /* RTMP_TIMER_TASK_SUPPORT */
#ifdef CONFIG_FWOWN_SUPPORT
	FwOwn(pAd);
#endif /* CONFIG_FWOWN_SUPPORT */
	/* Close Hw ctrl*/
	HwCtrlExit(pAd);
#ifdef REDUCE_TCP_ACK_SUPPORT
	ReduceAckExit(pAd);
#endif
#ifdef PRE_CAL_TRX_SET1_SUPPORT

	if (pAd->E2pAccessMode == E2P_BIN_MODE) {
		if (pAd->CalDCOCImage != NULL)
			os_free_mem(pAd->CalDCOCImage);

		if (pAd->CalDPDAPart1Image != NULL)
			os_free_mem(pAd->CalDPDAPart1Image);

		if (pAd->CalDPDAPart2Image != NULL)
			os_free_mem(pAd->CalDPDAPart2Image);
	}

#endif /* PRE_CAL_TRX_SET1_SUPPORT */
#ifdef PRE_CAL_TRX_SET2_SUPPORT

	if (pAd->E2pAccessMode != E2P_FLASH_MODE && pAd->PreCalStoreBuffer != NULL) {
		os_free_mem(pAd->PreCalStoreBuffer);
		pAd->PreCalStoreBuffer = NULL;
	}

	if (pAd->PreCalReStoreBuffer != NULL) {
		os_free_mem(pAd->PreCalReStoreBuffer);
		pAd->PreCalReStoreBuffer = NULL;
	}

#endif/* PRE_CAL_TRX_SET2_SUPPORT */
	os_free_mem(pAd->EEPROMImage);
	pAd->EEPROMImage = NULL;

#if defined(INTERNAL_CAPTURE_SUPPORT) || defined(WIFI_SPECTRUM_SUPPORT)
	if (pAd->pIQ_Array != NULL)
		os_free_mem(pAd->pIQ_Array);

	if (pAd->pL32Bit != NULL)
		os_free_mem(pAd->pL32Bit);

	if (pAd->pM32Bit != NULL)
		os_free_mem(pAd->pM32Bit);

	if (pAd->pH32Bit != NULL)
		os_free_mem(pAd->pH32Bit);
#endif /* defined(INTERNAL_CAPTURE_SUPPORT) || defined(WIFI_SPECTRUM_SUPPORT) */
	/*multi profile release*/
#ifdef MULTI_PROFILE
	multi_profile_exit(pAd);
#endif /*MULTI_PROFILE*/
#ifdef CCK_LQ_SUPPORT
	pAd->Avg_LQ = 0;
	pAd->Avg_LQx16 = 0;
#endif
#ifdef NF_SUPPORT
	pAd->Avg_NF = 0;
	pAd->Avg_NFx16 = 0;
#endif
#ifdef LTF_SNR_SUPPORT
	pAd->Avg_LTFSNR = 0;
	pAd->Avg_LTFSNRx16 = 0;
#endif

}

PNET_DEV RtmpPhyNetDevMainCreate(VOID *pAdSrc)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pAdSrc;
	PNET_DEV pDevNew;
	UINT32 MC_RowID = 0, IoctlIF = 0;
	char *dev_name;
#ifdef MULTIPLE_CARD_SUPPORT
	MC_RowID = pAd->MC_RowID;
#endif /* MULTIPLE_CARD_SUPPORT */
#ifdef HOSTAPD_SUPPORT
	IoctlIF = pAd->IoctlIF;
#endif /* HOSTAPD_SUPPORT */
#if defined(MT_WIFI_MODULE) && defined(PROBE2LOAD_L1PROFILE)
	if (load_dev_l1profile(pAd) == NDIS_STATUS_SUCCESS)
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("load l1profile succeed!\n"));
	else
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("load l1profile failed!\n"));
#endif
	dev_name = get_dev_name_prefix(pAd, INT_MAIN);
	pDevNew = RtmpOSNetDevCreate((INT32)MC_RowID, (UINT32 *)&IoctlIF,
					 INT_MAIN, 0, sizeof(struct mt_dev_priv), dev_name, FALSE);
#ifdef HOSTAPD_SUPPORT
	pAd->IoctlIF = IoctlIF;
#endif /* HOSTAPD_SUPPORT */
	return pDevNew;
}



#ifdef ERR_RECOVERY
INT	Set_ErrDetectOn_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg)
{
	UINT32 Enable;
	Enable = os_str_tol(arg, 0, 10);
	CmdExtGeneralTestOn(pAd, (Enable == 0) ? (FALSE) : (TRUE));
	return TRUE;
}

INT	Set_ErrDetectMode_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg)
{
	UINT8 mode = 0;
	UINT8 sub_mode = 0;
	PCHAR seg_str;
	seg_str = strsep((char **)&arg, "_");

	if (seg_str != NULL)
		mode = (BOOLEAN) os_str_tol(seg_str, 0, 10);

	seg_str = strsep((char **)&arg, "_");

	if (seg_str != NULL)
		sub_mode = (BOOLEAN) os_str_tol(seg_str, 0, 10);

	CmdExtGeneralTestMode(pAd, mode, sub_mode);
	return TRUE;
}
#endif /* ERR_RECOVERY */


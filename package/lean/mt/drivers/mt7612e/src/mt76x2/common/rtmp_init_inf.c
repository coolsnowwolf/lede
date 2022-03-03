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



#ifdef CONFIG_STA_SUPPORT
#ifdef PROFILE_STORE
NDIS_STATUS WriteDatThread(RTMP_ADAPTER *pAd);
#endif /* PROFILE_STORE */
#endif /* CONFIG_STA_SUPPORT */

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
	pDrvOps->RTMPHandleInterrupt = RTMPHandleInterrupt;
#endif /* RTMP_MAC_PCI */
	pDrvOps->RTMPSendPackets = RTMPSendPackets;

	pDrvOps->RTMP_COM_IoctlHandle = RTMP_COM_IoctlHandle;
#ifdef CONFIG_AP_SUPPORT
	pDrvOps->RTMP_AP_IoctlHandle = RTMP_AP_IoctlHandle;
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	pDrvOps->RTMP_STA_IoctlHandle = RTMP_STA_IoctlHandle;
#endif /* CONFIG_STA_SUPPORT */

	pDrvOps->RTMPDrvOpen = RTMPDrvOpen;
	pDrvOps->RTMPDrvClose = RTMPDrvClose;
	pDrvOps->RTMPInfClose = RTMPInfClose;
	pDrvOps->rt28xx_init = rt28xx_init;

	/* init operators provided from us and netif module */
}

RTMP_BUILD_DRV_OPS_FUNCTION_BODY

#endif /* OS_ABL_FUNC_SUPPORT */
#endif /* LINUX */


int rt28xx_init(VOID *pAdSrc, PSTRING pDefaultMac, PSTRING pHostName)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pAdSrc;
	NDIS_STATUS Status;
	
	if (!pAd)
		return FALSE;

	if (rtmp_asic_top_init(pAd) != TRUE)
		goto err1;

	DBGPRINT(RT_DEBUG_TRACE, ("MAC[Ver:Rev=0x%08x : 0x%08x]\n",
				pAd->MACVersion, pAd->ChipID));

	RT28XXDMADisable(pAd);

	if (mcu_sys_init(pAd) != TRUE)
		goto err1;

#ifdef RTMP_MAC_PCI
	// TODO: shiang-usw, need to check this for RTMP_MAC
	/* Disable interrupts here which is as soon as possible*/
	/* This statement should never be true. We might consider to remove it later*/
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_ACTIVE))
	{
		RTMP_ASIC_INTERRUPT_DISABLE(pAd);
	}
#endif /* RTMP_MAC_PCI */

	/* reset Adapter flags*/
	RTMP_CLEAR_FLAGS(pAd);

	if (MAX_LEN_OF_MAC_TABLE > MAX_AVAILABLE_CLIENT_WCID(pAd))
	{
		DBGPRINT(RT_DEBUG_ERROR, ("MAX_LEN_OF_MAC_TABLE can not be larger than MAX_AVAILABLE_CLIENT_WCID!!!!\n"));
		goto err1;
	}

#ifdef CONFIG_AP_SUPPORT
	/* Init BssTab & ChannelInfo tabbles for auto channel select.*/
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		AutoChBssTableInit(pAd);
		ChannelInfoInit(pAd);
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CUSTOMER_DCC_FEATURE
	/* init rate multiplication and shift factor table */
	InitRateMultiplicationAndShiftFactor(pAd);
#endif

#ifdef DOT11_N_SUPPORT
	/* Allocate BA Reordering memory*/
	if (ba_reordering_resource_init(pAd, MAX_REORDERING_MPDU_NUM) != TRUE)		
		goto err1;
#endif /* DOT11_N_SUPPORT */

#ifdef RESOURCE_PRE_ALLOC
	Status = RTMPInitTxRxRingMemory(pAd);
#else
	Status = RTMPAllocTxRxRingMemory(pAd);
#endif /* RESOURCE_PRE_ALLOC */
	if (Status != NDIS_STATUS_SUCCESS)
	{
		DBGPRINT_ERR(("RTMPAllocTxRxMemory failed, Status[=0x%08x]\n", Status));
		goto err2;
	}


#ifdef WLAN_SKB_RECYCLE
	skb_queue_head_init(&pAd->rx0_recycle);
#endif /* WLAN_SKB_RECYCLE */

	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE);

	/* initialize MLME*/
#ifdef RT6352
	if (IS_RT6352(pAd))
		pAd->bCalibrationDone = FALSE;
#endif /* RT6352 */
	
	Status = RtmpMgmtTaskInit(pAd);
	if (Status != NDIS_STATUS_SUCCESS)
		goto err3;

	Status = MlmeInit(pAd);
	if (Status != NDIS_STATUS_SUCCESS)
	{
		DBGPRINT_ERR(("MlmeInit failed, Status[=0x%08x]\n", Status));
		goto err4;
	}

	/* Initialize pAd->StaCfg, pAd->ApCfg, pAd->CommonCfg to manufacture default*/
	UserCfgInit(pAd);


	Status = RtmpNetTaskInit(pAd);
	if (Status != NDIS_STATUS_SUCCESS)
		goto err5;

	CfgInitHook(pAd);

#ifdef BLOCK_NET_IF
	initblockQueueTab(pAd);
#endif /* BLOCK_NET_IF */

	Status = MeasureReqTabInit(pAd);
	if (Status != NDIS_STATUS_SUCCESS)
	{
		DBGPRINT_ERR(("MeasureReqTabInit failed, Status[=0x%08x]\n",Status));
		goto err6;
	}
	Status = TpcReqTabInit(pAd);
	if (Status != NDIS_STATUS_SUCCESS)
	{
		DBGPRINT_ERR(("TpcReqTabInit failed, Status[=0x%08x]\n",Status));
		goto err6;
	}

	/*
		WiFi system operation mode setting base on following partitions:
		1. Parameters from config file
		2. Hardware cap from EEPROM
		3. Chip capabilities in code
	*/
	pAd->RfIcType = RFIC_UNKNOWN;
	Status = RTMPReadParametersHook(pAd);
	if (Status != NDIS_STATUS_SUCCESS)
	{
		DBGPRINT_ERR(("RTMPReadParametersHook failed, Status[=0x%08x]\n",Status));
		goto err6;
	}
	DBGPRINT(RT_DEBUG_OFF, ("1. Phy Mode = %d\n", pAd->CommonCfg.PhyMode));

	if (pAd->CommonCfg.bMcastTest == TRUE)
	{
		UINT32 macValue;
#ifdef RLT_MAC
		if (pAd->chipCap.hif_type == HIF_RLT) 
		{
			RTMP_IO_READ32(pAd, RLT_PBF_CFG, &macValue);
			macValue = (macValue | 0x2);
			RTMP_IO_WRITE32(pAd, RLT_PBF_CFG, macValue);
		}
		else
		{
#endif /* RLT_MAC */
#ifdef RTMP_MAC
			if (pAd->chipCap.hif_type == HIF_RTMP)
			{
				RTMP_IO_READ32(pAd, PBF_CFG, &macValue);
				macValue = (macValue | 0x8);
				RTMP_IO_WRITE32(pAd, PBF_CFG, macValue);
			}
#endif /* RTMP_MAC */
#ifdef RLT_MAC
		}
#endif /* RLT_MAC */
	}

	/* We should read EEPROM for all cases */
	NICReadEEPROMParameters(pAd, (PSTRING)pDefaultMac);
#ifdef CONFIG_STA_SUPPORT
#ifdef ETH_CONVERT_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		extern UCHAR ZERO_MAC_ADDR[MAC_ADDR_LEN];

		if ( NdisEqualMemory(&pAd->EthConvert.EthCloneMac[0], &ZERO_MAC_ADDR[0], MAC_ADDR_LEN))
		{
			NdisMoveMemory(&pAd->EthConvert.EthCloneMac[0], &pAd->CurrentAddress[0], MAC_ADDR_LEN);
			DBGPRINT(RT_DEBUG_OFF, ("Read EEPROM, EthCloneMac is %02x:%02x:%02x:%02x:%02x:%02x!\n",
					PRINT_MAC(pAd->EthConvert.EthCloneMac)));
		}
	}
#endif /* ETH_CONVERT_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */
	DBGPRINT(RT_DEBUG_OFF, ("2. Phy Mode = %d\n", pAd->CommonCfg.PhyMode));

	/* After operation mode is finialized, init the AP or STA mode */
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		APInitialize(pAd);

#ifdef DOT11K_RRM_SUPPORT
                RRM_CfgInit(pAd);
#endif /* DOT11K_RRM_SUPPORT */
	}
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
		STAInitialize(pAd);

#ifdef CREDENTIAL_STORE
		RecoverConnectInfo(pAd);
#endif /* CREDENTIAL_STORE */
	}
#endif /* CONFIG_STA_SUPPORT */


#ifdef DOT11_N_SUPPORT
   	/*Init Ba Capability parameters.*/
	pAd->CommonCfg.DesiredHtPhy.MpduDensity = (UCHAR)pAd->CommonCfg.BACapability.field.MpduDensity;
	pAd->CommonCfg.DesiredHtPhy.AmsduEnable = (USHORT)pAd->CommonCfg.BACapability.field.AmsduEnable;
	pAd->CommonCfg.DesiredHtPhy.AmsduSize = (USHORT)pAd->CommonCfg.BACapability.field.AmsduSize;
	pAd->CommonCfg.DesiredHtPhy.MimoPs = (USHORT)pAd->CommonCfg.BACapability.field.MMPSmode;
	/* UPdata to HT IE*/
	pAd->CommonCfg.HtCapability.HtCapInfo.MimoPs = (USHORT)pAd->CommonCfg.BACapability.field.MMPSmode;
	pAd->CommonCfg.HtCapability.HtCapInfo.AMsduSize = (USHORT)pAd->CommonCfg.BACapability.field.AmsduSize;
	pAd->CommonCfg.HtCapability.HtCapParm.MpduDensity = (UCHAR)pAd->CommonCfg.BACapability.field.MpduDensity;
#endif /* DOT11_N_SUPPORT */

	/* after reading Registry, we now know if in AP mode or STA mode */
	DBGPRINT(RT_DEBUG_OFF, ("3. Phy Mode = %d\n", pAd->CommonCfg.PhyMode));


	/*
		All settle down, now it's time to init asic related parameters
	*/
	/* Init the hardware, we need to init asic before read registry, otherwise mac register will be reset */
	Status = NICInitializeAdapter(pAd, TRUE);
	if (Status != NDIS_STATUS_SUCCESS)
	{
		DBGPRINT_ERR(("NICInitializeAdapter failed, Status[=0x%08x]\n", Status));
		if (Status != NDIS_STATUS_SUCCESS)
		goto err6;
	}

#ifdef CONFIG_FPGA_MODE
#ifdef CAPTURE_MODE
	cap_mode_init(pAd);
#endif /* CAPTURE_MODE */
#endif /* CONFIG_FPGA_MODE */

	NICInitAsicFromEEPROM(pAd);

#ifdef LED_CONTROL_SUPPORT
	/* Send LED Setting to MCU */
	RTMPInitLEDMode(pAd);	
#endif /* LED_CONTROL_SUPPORT */


	/*
		Do necessary calibration after ASIC initialized
		this's chip variant and may different for different chips
	*/
#ifdef RT6352
	if (IS_RT6352(pAd))
	{
		RtmpKickOutHwNullFrame(pAd, TRUE, FALSE);

#if defined(RT6352_EP_SUPPORT) || defined(RT6352_EL_SUPPORT)
		{
			ULONG SysRegValue;

			RTMP_SYS_IO_READ32(0xb0000060, &SysRegValue);
			if ((SysRegValue & 0x100000) == 0x0)
			{
				SysRegValue |= 0x100000;
				RTMP_SYS_IO_WRITE32(0xb0000060, SysRegValue);
				DBGPRINT(RT_DEBUG_ERROR,("Change as GPIO Mode(0x%x)\n", SysRegValue));
			}
		}
#endif /* defined(RT6352_EP_SUPPORT) || defined(RT6352_EL_SUPPORT) */

		/* Do R-Calibration */
		R_Calibration(pAd);

#ifdef RTMP_TEMPERATURE_CALIBRATION
		/* Temperature Init */
		RT6352_Temperature_Init(pAd);		
		RT6352_TemperatureCalibration(pAd);
#endif /* RTMP_TEMPERATURE_CALIBRATION */

#ifdef RTMP_TEMPERATURE_COMPENSATION
		/*
			read out tempature reference value (0x80 ~ 0x7F)
			TssiPlusBoundaryG [7] [6] [5] [4] [3] [2] [1] [0] (smaller) +
			TssiMinusBoundaryG[0] [1] [2] [3] [4] [5] [6] [7] (larger)
		*/
		RT6352_EEPROM_TSSI_24G_READ(pAd);
		/* 
			pAd->TssiCalibratedOffset: 
			reference temperature(e2p[D1h])
		*/				
		/* adjust the boundary table by pAd->TssiCalibratedOffset */
		RT6352_TssiTableAdjust(pAd);

		/* ATE temperature(e2p[77h]) */
		RT6352_TssiMpAdjust(pAd);

		DBGPRINT(RT_DEBUG_OFF,("E2PROM: G Tssi[-7 .. +7] = %d %d %d %d %d %d %d - %d - %d %d %d %d %d %d %d, offset=%d, tuning=%d\n",
			pAd->TssiMinusBoundaryG[7], pAd->TssiMinusBoundaryG[6], pAd->TssiMinusBoundaryG[5],
			pAd->TssiMinusBoundaryG[4], pAd->TssiMinusBoundaryG[3], pAd->TssiMinusBoundaryG[2], pAd->TssiMinusBoundaryG[1],
			pAd->TssiRefG,
			pAd->TssiPlusBoundaryG[1], pAd->TssiPlusBoundaryG[2], pAd->TssiPlusBoundaryG[3], pAd->TssiPlusBoundaryG[4],
			pAd->TssiPlusBoundaryG[5], pAd->TssiPlusBoundaryG[6], pAd->TssiPlusBoundaryG[7],
			pAd->TssiCalibratedOffset, pAd->bAutoTxAgcG));
#endif /* RTMP_TEMPERATURE_COMPENSATION */

		AsicSwitchChannel(pAd, pAd->CommonCfg.Channel, TRUE);
		AsicLockChannel(pAd, pAd->CommonCfg.Channel);

		/* RF Self TX DC Calibration */
		RF_SELF_TXDC_CAL(pAd);

		/* Rx DCOC Calibration */
		RxDCOC_Calibration(pAd);

		/* BandWidth Filter Calibration */
		BW_Filter_Calibration(pAd,TRUE);
		BW_Filter_Calibration(pAd,FALSE);

		/* Do LOFT and IQ Calibration */
		LOFT_IQ_Calibration(pAd);

		/* DPD_Calibration */
#ifdef RT6352_EP_SUPPORT
		if (pAd->bExtPA == FALSE)
#endif /* RT6352_EP_SUPPORT */
		{
			DoDPDCalibration(pAd);
			pAd->DoDPDCurrTemperature = 0x7FFFFFFF;
		}

		/* Rx DCOC Calibration */
		RxDCOC_Calibration(pAd);

		/* Do RXIQ Calibration */
		RXIQ_Calibration(pAd);

#if defined(RT6352_EP_SUPPORT) || defined(RT6352_EL_SUPPORT)
		RT6352_Init_ExtPA_ExtLNA(pAd, FALSE);
#endif /* defined(RT6352_EP_SUPPORT) || defined(RT6352_EL_SUPPORT) */
	}
#endif /* RT6352 */

#ifdef RALINK_ATE
	if (ATEInit(pAd) != NDIS_STATUS_SUCCESS)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s(): ATE initialization failed !\n", __FUNCTION__));
		goto err6;
	}
#endif /* RALINK_ATE */


#ifdef RTMP_INTERNAL_TX_ALC
	/* Initialize the desired TSSI table*/
	RTMP_CHIP_ASIC_TSSI_TABLE_INIT(pAd);
#endif /* RTMP_INTERNAL_TX_ALC */

#ifdef RT6352
	if (IS_RT6352(pAd))
		InitRfPaModeTable(pAd);
#endif /* RT6352 */

#ifdef RT8592
	// TODO: shiang-6590, actually, this operation shall be move to bbp_init
	if (IS_RT8592(pAd))
		bw_filter_cal(pAd);
#endif /* RT8592 */

#ifdef RTMP_TEMPERATURE_COMPENSATION
	/* Temperature compensation, initialize the lookup table */
	DBGPRINT(RT_DEBUG_OFF, ("bAutoTxAgcG = %d\n", pAd->bAutoTxAgcG));

	if (pAd->chipCap.bTempCompTxALC && pAd->bAutoTxAgcG)
		InitLookupTable(pAd);
#endif /* RTMP_TEMPERATURE_COMPENSATION */


	/* Set PHY to appropriate mode and will update the ChannelListNum in this function */
	RTMPSetPhyMode(pAd, pAd->CommonCfg.PhyMode);
	if (pAd->ChannelListNum == 0)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Wrong configuration. No valid channel found. Check \"ContryCode\" and \"ChannelGeography\" setting.\n"));
		goto err6;
	}

#ifdef DOT11_N_SUPPORT
	DBGPRINT(RT_DEBUG_OFF, ("MCS Set = %02x %02x %02x %02x %02x\n", pAd->CommonCfg.HtCapability.MCSSet[0],
           pAd->CommonCfg.HtCapability.MCSSet[1], pAd->CommonCfg.HtCapability.MCSSet[2],
           pAd->CommonCfg.HtCapability.MCSSet[3], pAd->CommonCfg.HtCapability.MCSSet[4]));
#endif /* DOT11_N_SUPPORT */

#ifdef WIN_NDIS
	/* Patch cardbus controller if EEPROM said so. */
	if (pAd->bTest1 == FALSE)
		RTMPPatchCardBus(pAd);
#endif /* WIN_NDIS */

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
#ifdef AP_QLOAD_SUPPORT
		QBSS_LoadInit(pAd);
#endif /* AP_QLOAD_SUPPORT */
	}
#endif /* CONFIG_AP_SUPPORT */

	/* APInitialize(pAd);*/

#ifdef IKANOS_VX_1X0
	VR_IKANOS_FP_Init(pAd->ApCfg.BssidNum, pAd->PermanentAddress);
#endif /* IKANOS_VX_1X0 */


#ifdef RALINK_ATE
#endif /* RALINK_ATE */


#ifdef RTMP_INTERNAL_TX_ALC
#ifdef RT6352
	if (IS_RT6352(pAd) && (pAd->TxPowerCtrl.bInternalTxALC == TRUE))
	{
		RT635xTssiDcCalibration(pAd);
	}
#endif /* RT6352 */
#endif /* RTMP_INTERNAL_TX_ALC */

	/*
		Some modules init must be called before APStartUp().
		Or APStartUp() will make up beacon content and call
		other modules API to get some information to fill.
	*/



	if (pAd && (Status != NDIS_STATUS_SUCCESS))
	{
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE))
		{
			RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE);
		}
	}
	else if (pAd)
	{
		/* Microsoft HCT require driver send a disconnect event after driver initialization.*/
		OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED);
		OPSTATUS_CLEAR_FLAG(pAd, fOP_AP_STATUS_MEDIA_STATE_CONNECTED);

		DBGPRINT(RT_DEBUG_TRACE, ("NDIS_STATUS_MEDIA_DISCONNECT Event B!\n"));

#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			if (pAd->ApCfg.bAutoChannelAtBootup || (pAd->CommonCfg.Channel == 0))
			{
				/* Enable Interrupt first due to we need to scan channel to receive beacons.*/
#ifdef RTMP_MAC_PCI
				RTMP_IRQ_ENABLE(pAd);
#endif /* RTMP_MAC_PCI */

				/* Now Enable RxTx*/
				RTMPEnableRxTx(pAd);
#ifdef MT762x
				// TODO: shiang-usw, check why MT76x2 don't need to set this flag here!
				if (!IS_MT762x(pAd))
#endif /* MT762x */
					RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_START_UP);

				/* Let BBP register at 20MHz to do scan */
				bbp_set_bw(pAd, BW_20);

				/* Now we can receive the beacon and do the listen beacon*/
				/* use default BW to select channel*/
				pAd->CommonCfg.Channel = AP_AUTO_CH_SEL(pAd, pAd->ApCfg.AutoChannelAlg);
				pAd->ApCfg.bAutoChannelAtBootup = FALSE;
			}

#ifdef DOT11_N_SUPPORT
			/* If WMODE_CAP_N(phymode) and BW=40 check extension channel, after select channel  */
			N_ChannelCheck(pAd);

#ifdef DOT11N_DRAFT3
        		/* 
         			We only do this Overlapping BSS Scan when system up, for the 
				other situation of channel changing, we depends on station's 
				report to adjust ourself.
			*/
			if (pAd->CommonCfg.bForty_Mhz_Intolerant == TRUE)
			{
				DBGPRINT(RT_DEBUG_TRACE, ("Disable 20/40 BSSCoex Channel Scan(BssCoex=%d, 40MHzIntolerant=%d)\n", 
											pAd->CommonCfg.bBssCoexEnable, 
											pAd->CommonCfg.bForty_Mhz_Intolerant));
			}
			else if(pAd->CommonCfg.bBssCoexEnable == TRUE)
			{	
				DBGPRINT(RT_DEBUG_TRACE, ("Enable 20/40 BSSCoex Channel Scan(BssCoex=%d)\n", 
							pAd->CommonCfg.bBssCoexEnable));
				APOverlappingBSSScan(pAd);
			}

			RTMP_11N_D3_TimerInit(pAd);
/*			RTMPInitTimer(pAd, &pAd->CommonCfg.Bss2040CoexistTimer, GET_TIMER_FUNCTION(Bss2040CoexistTimeOut), pAd, FALSE);*/
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */


			APStartUp(pAd);
			DBGPRINT(RT_DEBUG_OFF, ("Main bssid = %02x:%02x:%02x:%02x:%02x:%02x\n", 
						PRINT_MAC(pAd->ApCfg.MBSSID[BSS0].wdev.bssid)));

#ifdef MT76x2
			if (IS_MT76x2(pAd)) {
				mt76x2_reinit_agc_gain(pAd, pAd->hw_cfg.cent_ch);
				mt76x2_reinit_hi_lna_gain(pAd, pAd->hw_cfg.cent_ch);
				mt76x2_get_agc_gain(pAd, TRUE);
			}
#endif
		}
#endif /* CONFIG_AP_SUPPORT */

#ifdef RT6352
		if (IS_RT6352(pAd))
		{
#ifdef DYNAMIC_VGA_SUPPORT
			if (pAd->CommonCfg.lna_vga_ctl.bDyncVgaEnable == TRUE)
			{
				rt6352_dynamic_vga_enable(pAd);
			}
#endif /* DYNAMIC_VGA_SUPPORT */

			pAd->bCalibrationDone = TRUE;
		}
#endif /* RT6352 */

	}

	/* Set up the Mac address*/
#ifdef CONFIG_AP_SUPPORT
	RtmpOSNetDevAddrSet(pAd->OpMode, pAd->net_dev, &pAd->CurrentAddress[0], NULL);
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	NdisMoveMemory(&pAd->StaCfg.wdev.if_addr[0], &pAd->CurrentAddress[0], MAC_ADDR_LEN);
	RtmpOSNetDevAddrSet(pAd->OpMode, pAd->net_dev, &pAd->CurrentAddress[0], (PUCHAR)(pAd->StaCfg.dev_name));
	NdisMoveMemory(&pAd->StaCfg.wdev.if_addr[0], &pAd->CurrentAddress[0], MAC_ADDR_LEN);
#endif /* CONFIG_STA_SUPPORT */

#ifdef UAPSD_SUPPORT
        UAPSD_Init(pAd);
#endif /* UAPSD_SUPPORT */

	/* assign function pointers*/
#ifdef MAT_SUPPORT
	/* init function pointers, used in OS_ABL */
	RTMP_MATOpsInit(pAd);
#endif /* MAT_SUPPORT */

#ifdef RTMP_RBUS_SUPPORT
	if (pAd->infType == RTMP_DEV_INF_RBUS)
	{
#ifdef VIDEO_TURBINE_SUPPORT
		VideoTurbineDynamicTune(pAd);
#endif /* VIDEO_TURBINE_SUPPORT */

#ifdef RT3XXX_ANTENNA_DIVERSITY_SUPPORT
		RT3XXX_AntDiversity_Init(pAd);
#endif /* RT3XXX_ANTENNA_DIVERSITY_SUPPORT */
	}
#endif /* RTMP_RBUS_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
#ifdef MAT_SUPPORT
		MATEngineInit(pAd);
#endif /* MAT_SUPPORT */

#ifdef CLIENT_WDS
		CliWds_ProxyTabInit(pAd);
#endif /* CLIENT_WDS */
	}
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
#ifdef DOT11Z_TDLS_SUPPORT
		TDLS_Table_Init(pAd);
#endif /* DOT11Z_TDLS_SUPPORT */

#ifdef WPA_SUPPLICANT_SUPPORT
#ifndef NATIVE_WPA_SUPPLICANT_SUPPORT
		/* send wireless event to wpa_supplicant for infroming interface up.*/
		RtmpOSWrielessEventSend(pAd->net_dev, RT_WLAN_EVENT_CUSTOM, RT_INTERFACE_UP, NULL, NULL, 0);
#endif /* NATIVE_WPA_SUPPLICANT_SUPPORT */
#endif /* WPA_SUPPLICANT_SUPPORT */

#ifdef ETH_CONVERT_SUPPORT
		MATEngineInit(pAd);
#endif /* ETH_CONVERT_SUPPORT */
	}
#endif /* CONFIG_STA_SUPPORT */

	/* auto-fall back settings */
#ifdef RT6352
	if (IS_RT6352(pAd))
		RTMP_IO_WRITE32(pAd, HT_FBK_CFG1, 0xedcba980);
#endif /* RT6352 */
#ifdef DOT11N_SS3_SUPPORT
	if (pAd->CommonCfg.TxStream >= 3)
	{
		RTMP_IO_WRITE32(pAd, TX_FBK_CFG_3S_0, 0x12111008);
		RTMP_IO_WRITE32(pAd, TX_FBK_CFG_3S_1, 0x16151413);
	}
#endif /* DOT11N_SS3_SUPPORT */

#ifdef STREAM_MODE_SUPPORT
	RtmpStreamModeInit(pAd);
#endif /* STREAM_MODE_SUPPORT */


#ifdef DOT11_N_SUPPORT
#ifdef TXBF_SUPPORT
#ifndef MT76x2
	if (pAd->CommonCfg.ITxBfTimeout)
	{
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R179, 0x02);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R180, 0);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R182, pAd->CommonCfg.ITxBfTimeout & 0xFF);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R180, 1);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R182, (pAd->CommonCfg.ITxBfTimeout>>8) & 0xFF);
	}
#endif	

	if (pAd->CommonCfg.ETxBfTimeout)
	{
		RTMP_IO_WRITE32(pAd, TX_TXBF_CFG_3, pAd->CommonCfg.ETxBfTimeout);
	}
#endif /* TXBF_SUPPORT */
#endif /* DOT11_N_SUPPORT */



#ifdef CUSTOMER_DCC_FEATURE
	pAd->ChannelInfo.GetChannelInfo = FALSE;
#endif

#ifdef REDUCE_TCP_ACK_SUPPORT
	ReduceAckInit(pAd);
#endif /* REDUCE_TCP_ACK_SUPPORT */
	
	DBGPRINT_S(Status, ("<==== rt28xx_init, Status=%x\n", Status));

	return TRUE;

err6:

#ifdef IGMP_SNOOP_SUPPORT
	MultiCastFilterTableReset(pAd, &pAd->pMulticastFilterTable);
#endif /* IGMP_SNOOP_SUPPORT */

	MeasureReqTabExit(pAd);
	TpcReqTabExit(pAd);
err5:	
	RtmpNetTaskExit(pAd);
	UserCfgExit(pAd);
err4:	
	MlmeHalt(pAd);
	RTMP_AllTimerListRelease(pAd);
err3:	
	RtmpMgmtTaskExit(pAd);
#ifdef RTMP_TIMER_TASK_SUPPORT
	NdisFreeSpinLock(&pAd->TimerQLock);
#endif /* RTMP_TIMER_TASK_SUPPORT */
err2:
#ifdef RESOURCE_PRE_ALLOC
	RTMPResetTxRxRingMemory(pAd);
#else
	RTMPFreeTxRxRingMemory(pAd);
#endif /* RESOURCE_PRE_ALLOC */

err1:

	MCU_CTRL_EXIT(pAd);

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
	/* Free BssTab & ChannelInfo tabbles.*/
	AutoChBssTableDestroy(pAd);
	ChannelInfoDestroy(pAd);
	}
#endif /* CONFIG_AP_SUPPORT */


#ifdef DOT11_N_SUPPORT
	if(pAd->mpdu_blk_pool.mem)
		os_free_mem(pAd, pAd->mpdu_blk_pool.mem); /* free BA pool*/
#endif /* DOT11_N_SUPPORT */

#ifdef INF_AMAZON_SE
err0:
#endif /* INF_AMAZON_SE */
#ifdef ST
err0:
#endif /* ST */

	DBGPRINT(RT_DEBUG_ERROR, ("!!! rt28xx init fail !!!\n"));
	return FALSE;
}


VOID RTMPDrvOpen(VOID *pAdSrc)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pAdSrc;
#ifdef BTCOEX_CONCURRENT
	BT_COEX_VAL btcoex_val;
	UINT32 val;
	USHORT ee_tmp;
#endif
	RTMP_CLEAR_PSFLAG(pAd, fRTMP_PS_MCU_SLEEP);
#ifdef CONFIG_STA_SUPPORT
#ifdef DOT11R_FT_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		FT_RIC_Init(pAd);
	}
#endif /* DOT11R_FT_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */

#ifdef RTMP_MAC
	// TODO: shiang-usw, check this for RMTP_MAC
	if (pAd->chipCap.hif_type == HIF_RTMP) {
#ifdef RTMP_MAC_PCI
		/* Enable Interrupt*/
		RTMP_IRQ_ENABLE(pAd);
#endif /* RTMP_MAC_PCI */

		/* Now Enable RxTx*/
		RTMPEnableRxTx(pAd);
	}
#endif /* RTMP_MAC */

	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_START_UP);




#ifdef CONFIG_STA_SUPPORT
#ifdef PCIE_PS_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
        RTMPInitPCIeLinkCtrlValue(pAd);
#endif /* PCIE_PS_SUPPORT */

#endif /* CONFIG_STA_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
#ifdef BG_FT_SUPPORT
	BG_FTPH_Init();
#endif /* BG_FT_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */


//+++Add by shiang for debug
	DBGPRINT(RT_DEBUG_OFF, ("%s(1):Check if PDMA is idle!\n", __FUNCTION__));
	AsicWaitPDMAIdle(pAd, 5, 10);
//---Add by shiang for debug

#ifdef CONFIG_STA_SUPPORT
	/*
		To reduce connection time, 
		do auto reconnect here instead of waiting STAMlmePeriodicExec to do auto reconnect.
	*/
	if (pAd->OpMode == OPMODE_STA)
		MlmeAutoReconnectLastSSID(pAd);
#endif /* CONFIG_STA_SUPPORT */
//+++Add by shiang for debug
	DBGPRINT(RT_DEBUG_OFF, ("%s(2):Check if PDMA is idle!\n", __FUNCTION__));
	AsicWaitPDMAIdle(pAd, 5, 10);
//---Add by shiang for debug

#ifdef CONFIG_STA_SUPPORT
#ifdef DOT11W_PMF_SUPPORT
	if (pAd->OpMode == OPMODE_STA)
	{
		pAd->StaCfg.PmfCfg.MFPC = FALSE;
		pAd->StaCfg.PmfCfg.MFPR = FALSE;
		pAd->StaCfg.PmfCfg.PMFSHA256 = FALSE;
		if ((pAd->StaCfg.wdev.AuthMode == Ndis802_11AuthModeWPA2 || pAd->StaCfg.wdev.AuthMode == Ndis802_11AuthModeWPA2PSK)
			&& (pAd->StaCfg.wdev.WepStatus == Ndis802_11AESEnable))
		{
			pAd->StaCfg.PmfCfg.PMFSHA256 = pAd->StaCfg.PmfCfg.Desired_PMFSHA256;
			if (pAd->StaCfg.PmfCfg.Desired_MFPC)
			{
				pAd->StaCfg.PmfCfg.MFPC = TRUE;
				pAd->StaCfg.PmfCfg.MFPR = pAd->StaCfg.PmfCfg.Desired_MFPR;

				if (pAd->StaCfg.PmfCfg.MFPR)
					pAd->StaCfg.PmfCfg.PMFSHA256 = TRUE;
			}
		} else if (pAd->StaCfg.PmfCfg.Desired_MFPC) {
			DBGPRINT(RT_DEBUG_ERROR, ("[PMF]%s:: Security is not WPA2/WPA2PSK AES\n", __FUNCTION__));
		}

		DBGPRINT(RT_DEBUG_ERROR, ("[PMF]%s:: MFPC=%d, MFPR=%d, SHA256=%d\n",
					__FUNCTION__, pAd->StaCfg.PmfCfg.MFPC, pAd->StaCfg.PmfCfg.MFPR,
					pAd->StaCfg.PmfCfg.PMFSHA256));
	}
#endif /* DOT11W_PMF_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */

#ifdef WSC_INCLUDED
#ifdef CONFIG_AP_SUPPORT
	if ((pAd->OpMode == OPMODE_AP)
		)
	{
		INT index;
		for (index = 0; index < pAd->ApCfg.BssidNum; index++)
		{
#ifdef HOSTAPD_SUPPORT
			if (pAd->ApCfg.MBSSID[index].Hostapd == Hostapd_EXT)
			{
				DBGPRINT(RT_DEBUG_TRACE, ("WPS is control by hostapd now.\n"));
			}
			else
#endif /*HOSTAPD_SUPPORT*/
			{
				PWSC_CTRL pWscControl;
				UCHAR zeros16[16]= {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
				
				pWscControl = &pAd->ApCfg.MBSSID[index].WscControl;
				DBGPRINT(RT_DEBUG_TRACE, ("Generate UUID for apidx(%d)\n", index));
				if (NdisEqualMemory(&pWscControl->Wsc_Uuid_E[0], zeros16, UUID_LEN_HEX))
					WscGenerateUUID(pAd, &pWscControl->Wsc_Uuid_E[0], &pWscControl->Wsc_Uuid_Str[0], index, FALSE);
				WscInit(pAd, FALSE, index);
			}
		}

#ifdef APCLI_SUPPORT
		for(index = 0; index < MAX_APCLI_NUM; index++)
		{
			PWSC_CTRL pWpsCtrl = &pAd->ApCfg.ApCliTab[index].WscControl;
			
			pWpsCtrl->pAd = pAd;        
			NdisZeroMemory(pWpsCtrl->EntryAddr, MAC_ADDR_LEN);
#ifdef WSC_V2_SUPPORT
			pWpsCtrl->WscConfigMethods= 0x238C;
#else			
			pWpsCtrl->WscConfigMethods= 0x018C;
#endif /*WSC_V2_SUPPORT*/
			RTMP_AP_IoctlHandle(pAd, NULL, CMD_RTPRIV_IOCTL_WSC_INIT, 0, (VOID *)&pAd->ApCfg.ApCliTab[index], index);
		}
#endif /* APCLI_SUPPORT */
	}
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		PWSC_CTRL pWscControl = &pAd->StaCfg.WscControl;
		
		WscGenerateUUID(pAd, &pWscControl->Wsc_Uuid_E[0], &pWscControl->Wsc_Uuid_Str[0], 0, FALSE);
		WscInit(pAd, FALSE, BSS0);
#ifdef WSC_V2_SUPPORT
		WscInitRegistrarPair(pAd, &pAd->StaCfg.WscControl, BSS0);
#endif /* WSC_V2_SUPPORT */
	}
#endif /* CONFIG_STA_SUPPORT */

	/* WSC hardware push button function 0811 */
	WSC_HDR_BTN_Init(pAd);
#endif /* WSC_INCLUDED */

#ifdef BTCOEX_CONCURRENT
	RT28xx_EEPROM_READ16(pAd, 0x22, ee_tmp);
	btcoex_val.eeprom23=(ee_tmp & 0xFF00)>>8;
	
	RT28xx_EEPROM_READ16(pAd, 0x24, ee_tmp);
	btcoex_val.eeprom24=(ee_tmp & 0x00FF);	
	btcoex_val.eeprom25=(ee_tmp & 0xFF00)>>8;

	NdisCopyMemory(&val, &btcoex_val, sizeof(UINT32));
	printk("val = 0x%x\n",val);
	andes_fun_set(pAd, 11, val);
#endif
#ifdef DMA_BUSY_RESET
#ifdef RTMP_PCI_SUPPORT
	pAd->PDMAWatchDogEn = 1;
#endif /* RTMP_PCI_SUPPORT */
#endif /* DMA_BUSY_RESET */
#ifdef ED_MONITOR
{
	BOOLEAN bEdcca = FALSE;

	bEdcca = GetEDCCASupport(pAd);

	if (bEdcca)
	{
		ed_monitor_init(pAd);
	}
}
#endif /* ED_MONITOR */
}


VOID RTMPDrvClose(VOID *pAdSrc, VOID *net_dev)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pAdSrc;
	UINT32 i = 0;

#ifdef DMA_BUSY_RESET
#ifdef RTMP_PCI_SUPPORT
	pAd->PDMAWatchDogEn = 0;
#endif /* RTMP_PCI_SUPPORT */
#endif /* DMA_BUSY_RESET */


#ifdef LED_CONTROL_SUPPORT
	RTMPExitLEDMode(pAd);
#endif // LED_CONTROL_SUPPORT

#ifdef RT_CFG80211_SUPPORT
#ifdef CONFIG_AP_SUPPORT
		if (pAd->cfg80211_ctrl.isCfgInApMode == RT_CMD_80211_IFTYPE_AP && RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP))
		{
			CFG80211DRV_DisableApInterface(pAd);
			pAd->cfg80211_ctrl.isCfgInApMode = RT_CMD_80211_IFTYPE_STATION;			
		}
#endif /* CONFIG_AP_SUPPORT */	
#endif/*RT_CFG80211_SUPPORT*/

	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_MCU_SEND_IN_BAND_CMD);
#ifdef BB_SOC
	 if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF))
	 {
	 	DBGPRINT(RT_DEBUG_TRACE, ("Radio_ON first....\n"));
    		MlmeRadioOn(pAd);
	 }
#endif /* BB_SOC */

#ifdef CONFIG_STA_SUPPORT
#ifdef CREDENTIAL_STORE
		if (pAd->IndicateMediaState == NdisMediaStateConnected)
			StoreConnectInfo(pAd);
		else
		{
			RTMP_SEM_LOCK(&pAd->StaCtIf.Lock);
			pAd->StaCtIf.Changeable = FALSE;
			RTMP_SEM_UNLOCK(&pAd->StaCtIf.Lock);
		}
#endif /* CREDENTIAL_STORE */
#endif /* CONFIG_STA_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
#ifdef BG_FT_SUPPORT
	BG_FTPH_Remove();
#endif /* BG_FT_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

#ifdef RTMP_RBUS_SUPPORT
#ifdef RT3XXX_ANTENNA_DIVERSITY_SUPPORT
	if (pAd->infType == RTMP_DEV_INF_RBUS)
	RT3XXX_AntDiversity_Fini(pAd);
#endif /* RT3XXX_ANTENNA_DIVERSITY_SUPPORT */
#endif /* RTMP_RBUS_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
#ifdef PCIE_PS_SUPPORT
		RTMPPCIeLinkCtrlValueRestore(pAd, RESTORE_CLOSE);
#endif /* PCIE_PS_SUPPORT */

		/* If dirver doesn't wake up firmware here,*/
		/* NICLoadFirmware will hang forever when interface is up again.*/
		if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE))
        {      
		    AsicForceWakeup(pAd, TRUE);
        }


#ifdef RTMP_MAC_PCI
		pAd->bPCIclkOff = FALSE;    
#endif /* RTMP_MAC_PCI */
	}
#endif /* CONFIG_STA_SUPPORT */

#if ((defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT)) && defined(WOW_IFDOWN_SUPPORT)
	if (pAd->WOW_Cfg.bEnable == FALSE)
#endif /* ((defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT)) && defined(WOW_IFDOWN_SUPPORT) */
	{
		RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_MCU_SEND_IN_BAND_CMD);
		RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);
	}

#ifdef EXT_BUILD_CHANNEL_LIST
	if (pAd->CommonCfg.pChDesp != NULL)
		os_free_mem(NULL, pAd->CommonCfg.pChDesp);
	pAd->CommonCfg.pChDesp = NULL;
	pAd->CommonCfg.DfsType = MAX_RD_REGION;
	pAd->CommonCfg.bCountryFlag = 0;
#endif /* EXT_BUILD_CHANNEL_LIST */
	pAd->CommonCfg.bCountryFlag = FALSE;



#ifdef WDS_SUPPORT
	WdsDown(pAd);
#endif /* WDS_SUPPORT */

	RtmpOsMsDelay(20); /* wait for disconnect requests transmitted */

	for (i = 0 ; i < NUM_OF_TX_RING; i++)
	{
		while (pAd->DeQueueRunning[i] == TRUE)
		{
			DBGPRINT(RT_DEBUG_TRACE, ("Waiting for TxQueue[%d] done..........\n", i));
			RtmpusecDelay(1000);
		}
	}
	
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
#ifdef DOT11N_DRAFT3
		BOOLEAN Cancelled = FALSE;

		if (pAd->CommonCfg.Bss2040CoexistFlag & BSS_2040_COEXIST_TIMER_FIRED)
		{
			RTMPCancelTimer(&pAd->CommonCfg.Bss2040CoexistTimer, &Cancelled);
			pAd->CommonCfg.Bss2040CoexistFlag  = 0;
		}
#endif /* DOT11N_DRAFT3 */

		/* PeriodicTimer already been canceled by MlmeHalt() API.*/
		/*RTMPCancelTimer(&pAd->PeriodicTimer,	&Cancelled);*/
	}
#endif /* CONFIG_AP_SUPPORT */

	/* Stop Mlme state machine*/
	MlmeHalt(pAd);
	
	/* Close net tasklets*/
	RtmpNetTaskExit(pAd);


#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		MacTableReset(pAd);
#ifdef MAT_SUPPORT
		MATEngineExit(pAd);
#endif /* MAT_SUPPORT */
#if ((defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT)) && defined(WOW_IFDOWN_SUPPORT)
		if (pAd->WOW_Cfg.bEnable == TRUE)
			ASIC_WOW_ENABLE(pAd);
		else
#endif /* ((defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT)) && defined(WOW_IFDOWN_SUPPORT) */
			MlmeRadioOff(pAd);
	}
#endif /* CONFIG_STA_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
#ifdef MAT_SUPPORT
		MATEngineExit(pAd);
#endif /* MAT_SUPPORT */

#ifdef CLIENT_WDS
		CliWds_ProxyTabDestory(pAd);
#endif /* CLIENT_WDS */

#ifdef MWDS
		MWDSAPDown(pAd);
#endif /* MWDS */

		/* Shutdown Access Point function, release all related resources */
		APShutdown(pAd);
	}
#endif /* CONFIG_AP_SUPPORT */

	MeasureReqTabExit(pAd);
	TpcReqTabExit(pAd);



	/* Close kernel threads*/
	RtmpMgmtTaskExit(pAd);
	
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		/* must after RtmpMgmtTaskExit(); Or pAd->pChannelInfo will be NULL */
		/* Free BssTab & ChannelInfo tabbles.*/
		AutoChBssTableDestroy(pAd);
		ChannelInfoDestroy(pAd);
	}
#endif /* CONFIG_AP_SUPPORT */

#ifdef RTMP_MAC_PCI
	{
#if defined(MT76x0) || defined(MT76x2)
		if (IS_MT76x0(pAd) || IS_MT76x2(pAd)) {
			DISABLE_TX_RX(pAd, RTMP_HALT);
		}
		else
#endif /* MT76x0 */
		{
			if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_ACTIVE))
			{
				DISABLE_TX_RX(pAd, RTMP_HALT);
				RTMP_ASIC_INTERRUPT_DISABLE(pAd);
			}
		}

		/* Receive packets to clear DMA index after disable interrupt. */
		/*RTMPHandleRxDoneInterrupt(pAd);*/
		/* put to radio off to save power when driver unload.  After radiooff, can't write /read register.  So need to finish all */
		/* register access before Radio off.*/

#ifdef RTMP_PCI_SUPPORT
		if (pAd->infType == RTMP_DEV_INF_PCI || pAd->infType == RTMP_DEV_INF_PCIE)
		{
			BOOLEAN brc = TRUE;

#if defined(MT76x0) || defined(MT76x2)
			// TODO: shiang, how about RadioOff for 65xx??
			if (!(IS_MT76x0(pAd) || IS_MT76x2(pAd)))
#endif /* defined(MT76x0) || defined(MT76x2) */
			brc = RT28xxPciAsicRadioOff(pAd, RTMP_HALT, 0);

/*In  solution 3 of 3090F, the bPCIclkOff will be set to TRUE after calling RT28xxPciAsicRadioOff*/
#ifdef PCIE_PS_SUPPORT
			pAd->bPCIclkOff = FALSE;
#endif /* PCIE_PS_SUPPORT */

			if (brc==FALSE)
			{
				DBGPRINT(RT_DEBUG_ERROR,("%s call RT28xxPciAsicRadioOff fail !!\n", __FUNCTION__)); 
			}
		}
#endif /* RTMP_PCI_SUPPORT */
	}

#endif /* RTMP_MAC_PCI */

	/* Free IRQ*/
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE))
	{
#ifdef RTMP_MAC_PCI
		/* Deregister interrupt function*/
		RTMP_OS_IRQ_RELEASE(pAd, net_dev);
#endif /* RTMP_MAC_PCI */
		RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE);
	}

#ifdef SINGLE_SKU_V2
	{
		CH_POWER *ch, *ch_temp;
		DlListForEachSafe(ch, ch_temp, &pAd->SingleSkuPwrList, CH_POWER, List)
		{
			DlListDel(&ch->List);
			os_free_mem(NULL, ch);
		}
	}
#endif /* SINGLE_SKU_V2 */

	/* Free Ring or USB buffers*/
#ifdef RESOURCE_PRE_ALLOC
	RTMPResetTxRxRingMemory(pAd);
#else
	/* Free Ring or USB buffers*/
	RTMPFreeTxRxRingMemory(pAd);
#endif /* RESOURCE_PRE_ALLOC */

	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);

#ifdef WLAN_SKB_RECYCLE
	skb_queue_purge(&pAd->rx0_recycle);
#endif /* WLAN_SKB_RECYCLE */

#ifdef DOT11_N_SUPPORT
	/* Free BA reorder resource*/
	ba_reordering_resource_release(pAd);
#endif /* DOT11_N_SUPPORT */

	UserCfgExit(pAd); /* must after ba_reordering_resource_release */

#ifdef CONFIG_STA_SUPPORT
#ifdef DOT11R_FT_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		FT_RIC_Release(pAd);
	}
#endif /* DOT11R_FT_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */

	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_START_UP);

/*+++Modify by woody to solve the bulk fail+++*/
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
#ifdef DOT11Z_TDLS_SUPPORT
		TDLS_Table_Destory(pAd);
#ifdef TDLS_AUTOLINK_SUPPORT
		TDLS_ClearEntryList(&pAd->StaCfg.TdlsInfo.TdlsDiscovPeerList);
		NdisFreeSpinLock(&pAd->StaCfg.TdlsInfo.TdlsDiscovPeerListSemLock);
		TDLS_ClearEntryList(&pAd->StaCfg.TdlsInfo.TdlsBlackList);
		NdisFreeSpinLock(&pAd->StaCfg.TdlsInfo.TdlsBlackListSemLock);
#endif /* TDLS_AUTOLINK_SUPPORT */
#endif /* DOT11Z_TDLS_SUPPORT */
	}
#endif /* CONFIG_STA_SUPPORT */

	/* clear MAC table */
	/* TODO: do not clear spin lock, such as fLastChangeAccordingMfbLock */
	NdisZeroMemory(&pAd->MacTab, sizeof(MAC_TABLE));

	/* release all timers */
	RtmpusecDelay(2000);
	RTMP_AllTimerListRelease(pAd);

#ifdef ED_MONITOR
	if (pAd->ed_chk)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("@@@ %s: go to ed_monitor_exit()!!\n", __FUNCTION__));		
		ed_monitor_exit(pAd);
	}
#endif /* ED_MONITOR */

#ifdef RTMP_TIMER_TASK_SUPPORT
	NdisFreeSpinLock(&pAd->TimerQLock);
#endif /* RTMP_TIMER_TASK_SUPPORT */

#ifdef CONFIG_FPGA_MODE
#ifdef CAPTURE_MODE
	cap_mode_deinit(pAd);
#endif /* CAPTURE_MODE */
#endif /* CONFIG_FPGA_MODE */

#ifdef REDUCE_TCP_ACK_SUPPORT
	ReduceAckExit(pAd);
#endif /* REDUCE_TCP_ACK_SUPPORT */
}


VOID RTMPInfClose(VOID *pAdSrc)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pAdSrc;

#ifdef CONFIG_AP_SUPPORT
	pAd->ApCfg.MBSSID[MAIN_MBSSID].bBcnSntReq = FALSE;

	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		/* kick out all STAs behind the bss.*/
		MbssKickOutStas(pAd, MAIN_MBSSID, REASON_DISASSOC_INACTIVE);
	}

	APMakeAllBssBeacon(pAd);
	APUpdateAllBeaconFrame(pAd);
#endif /* CONFIG_AP_SUPPORT */



#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
#ifdef PROFILE_STORE
		WriteDatThread(pAd);
		RtmpusecDelay(1000);
#endif /* PROFILE_STORE */
#ifdef QOS_DLS_SUPPORT
		/* send DLS-TEAR_DOWN message, */
		if (pAd->CommonCfg.bDLSCapable)
		{
			UCHAR i;

			/* tear down local dls table entry*/
			for (i=0; i<MAX_NUM_OF_INIT_DLS_ENTRY; i++)
			{
				if (pAd->StaCfg.DLSEntry[i].Valid && (pAd->StaCfg.DLSEntry[i].Status == DLS_FINISH))
				{
					RTMPSendDLSTearDownFrame(pAd, pAd->StaCfg.DLSEntry[i].MacAddr);
					pAd->StaCfg.DLSEntry[i].Status	= DLS_NONE;
					pAd->StaCfg.DLSEntry[i].Valid	= FALSE;
				}
			}

			/* tear down peer dls table entry*/
			for (i=MAX_NUM_OF_INIT_DLS_ENTRY; i<MAX_NUM_OF_DLS_ENTRY; i++)
			{
				if (pAd->StaCfg.DLSEntry[i].Valid && (pAd->StaCfg.DLSEntry[i].Status == DLS_FINISH))
				{
					RTMPSendDLSTearDownFrame(pAd, pAd->StaCfg.DLSEntry[i].MacAddr);
					pAd->StaCfg.DLSEntry[i].Status = DLS_NONE;
					pAd->StaCfg.DLSEntry[i].Valid	= FALSE;
				}
			}
			RTMP_MLME_HANDLER(pAd);
		}
#endif /* QOS_DLS_SUPPORT */

		if (INFRA_ON(pAd) &&
#if ((defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT)) && defined(WOW_IFDOWN_SUPPORT)
	/* In WOW state, can't issue disassociation reqeust */
			pAd->WOW_Cfg.bEnable == FALSE &&
#endif /* ((defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT)) && defined(WOW_IFDOWN_SUPPORT) */
			(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST)))
		{
			MLME_DISASSOC_REQ_STRUCT	DisReq;
			MLME_QUEUE_ELEM *MsgElem;
    
			os_alloc_mem(NULL, (UCHAR **)&MsgElem, sizeof(MLME_QUEUE_ELEM));
			if (MsgElem)
			{
			COPY_MAC_ADDR(DisReq.Addr, pAd->CommonCfg.Bssid);
			DisReq.Reason =  REASON_DEAUTH_STA_LEAVING;

			MsgElem->Machine = ASSOC_STATE_MACHINE;
			MsgElem->MsgType = MT2_MLME_DISASSOC_REQ;
			MsgElem->MsgLen = sizeof(MLME_DISASSOC_REQ_STRUCT);
			NdisMoveMemory(MsgElem->Msg, &DisReq, sizeof(MLME_DISASSOC_REQ_STRUCT));

			/* Prevent to connect AP again in STAMlmePeriodicExec*/
			pAd->MlmeAux.AutoReconnectSsidLen= 32;
			NdisZeroMemory(pAd->MlmeAux.AutoReconnectSsid, pAd->MlmeAux.AutoReconnectSsidLen);

			pAd->Mlme.CntlMachine.CurrState = CNTL_WAIT_OID_DISASSOC;
			MlmeDisassocReqAction(pAd, MsgElem);
			os_free_mem(NULL, MsgElem);
			}
			
			RtmpusecDelay(1000);
		}

#ifdef WPA_SUPPLICANT_SUPPORT
#ifndef NATIVE_WPA_SUPPLICANT_SUPPORT
		/* send wireless event to wpa_supplicant for infroming interface down.*/
		RtmpOSWrielessEventSend(pAd->net_dev, RT_WLAN_EVENT_CUSTOM, RT_INTERFACE_DOWN, NULL, NULL, 0);
#endif /* NATIVE_WPA_SUPPLICANT_SUPPORT */

		if (pAd->StaCfg.wpa_supplicant_info.pWpsProbeReqIe)
		{
			os_free_mem(NULL, pAd->StaCfg.wpa_supplicant_info.pWpsProbeReqIe);
			pAd->StaCfg.wpa_supplicant_info.pWpsProbeReqIe = NULL;
			pAd->StaCfg.wpa_supplicant_info.WpsProbeReqIeLen = 0;
		}

		if (pAd->StaCfg.wpa_supplicant_info.pWpaAssocIe)
		{
			os_free_mem(NULL, pAd->StaCfg.wpa_supplicant_info.pWpaAssocIe);
			pAd->StaCfg.wpa_supplicant_info.pWpaAssocIe = NULL;
			pAd->StaCfg.wpa_supplicant_info.WpaAssocIeLen = 0;
		}
#endif /* WPA_SUPPLICANT_SUPPORT */


	}
#endif /* CONFIG_STA_SUPPORT */
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

	dev_name = get_dev_name_prefix(pAd, INT_MAIN);
	pDevNew = RtmpOSNetDevCreate((INT32)MC_RowID, (UINT32 *)&IoctlIF,
					INT_MAIN, 0, sizeof(struct mt_dev_priv), dev_name);

#ifdef HOSTAPD_SUPPORT
	pAd->IoctlIF = IoctlIF;
#endif /* HOSTAPD_SUPPORT */

	return pDevNew;
}


#ifdef CONFIG_STA_SUPPORT
#ifdef PROFILE_STORE
static void WriteConfToDatFile(RTMP_ADAPTER *pAd)
{
	char	*cfgData = 0, *offset = 0;
	PSTRING fileName = NULL, pTempStr = NULL;
	RTMP_OS_FD file_r, file_w;
	RTMP_OS_FS_INFO osFSInfo;
	LONG rv, fileLen = 0;


	DBGPRINT(RT_DEBUG_TRACE, ("-----> WriteConfToDatFile\n"));

#ifdef RTMP_RBUS_SUPPORT
	if (pAd->infType == RTMP_DEV_INF_RBUS)
		fileName = STA_PROFILE_PATH_RBUS;
	else
#endif /* RTMP_RBUS_SUPPORT */
		fileName = STA_PROFILE_PATH;

	RtmpOSFSInfoChange(&osFSInfo, TRUE);

	file_r = RtmpOSFileOpen(fileName, O_RDONLY, 0);
	if (IS_FILE_OPEN_ERR(file_r)) 
	{
		DBGPRINT(RT_DEBUG_TRACE, ("-->1) %s: Error opening file %s\n", __FUNCTION__, fileName));
		return;
	}
	else 
	{
		char tempStr[64] = {0};
		while((rv = RtmpOSFileRead(file_r, tempStr, 64)) > 0)
		{
			fileLen += rv;
		}
		os_alloc_mem(NULL, (UCHAR **)&cfgData, fileLen);
		if (cfgData == NULL)
		{
			RtmpOSFileClose(file_r);
			DBGPRINT(RT_DEBUG_TRACE, ("CfgData mem alloc fail. (fileLen = %ld)\n", fileLen));
			goto out;
		}
		NdisZeroMemory(cfgData, fileLen);
		RtmpOSFileSeek(file_r, 0);
		rv = RtmpOSFileRead(file_r, (PSTRING)cfgData, fileLen);
		RtmpOSFileClose(file_r);
		if (rv != fileLen)
		{
			DBGPRINT(RT_DEBUG_TRACE, ("CfgData mem alloc fail, fileLen = %ld\n", fileLen));
			goto ReadErr;
		}
	}

	file_w = RtmpOSFileOpen(fileName, O_WRONLY|O_TRUNC, 0);
	if (IS_FILE_OPEN_ERR(file_w)) 
	{
		goto WriteFileOpenErr;
	}
	else 
	{
		offset = (PCHAR) rtstrstr((PSTRING) cfgData, "Default\n");
		offset += strlen("Default\n");
		RtmpOSFileWrite(file_w, (PSTRING)cfgData, (int)(offset-cfgData));
		os_alloc_mem(NULL, (UCHAR **)&pTempStr, 512);
		if (!pTempStr)
		{
			DBGPRINT(RT_DEBUG_TRACE, ("pTempStr mem alloc fail. (512)\n"));
			RtmpOSFileClose(file_w);
			goto WriteErr;
		}
			
		for (;;)
		{
			int i = 0;
			PSTRING ptr;

			NdisZeroMemory(pTempStr, 512);
			ptr = (PSTRING) offset;
			while(*ptr && *ptr != '\n')
			{
				pTempStr[i++] = *ptr++;
			}
			pTempStr[i] = 0x00;
			if ((size_t)(offset - cfgData) < fileLen)
			{
				offset += strlen(pTempStr) + 1;
				if (strncmp(pTempStr, "SSID=", strlen("SSID=")) == 0)
				{
					NdisZeroMemory(pTempStr, 512);
					NdisMoveMemory(pTempStr, "SSID=", strlen("SSID="));
					NdisMoveMemory(pTempStr + 5, pAd->CommonCfg.Ssid, pAd->CommonCfg.SsidLen);
				}
				else if (strncmp(pTempStr, "AuthMode=", strlen("AuthMode=")) == 0)
				{
					NdisZeroMemory(pTempStr, 512);
					if (pAd->StaCfg.AuthMode == Ndis802_11AuthModeOpen)
						sprintf(pTempStr, "AuthMode=OPEN");
					else if (pAd->StaCfg.AuthMode == Ndis802_11AuthModeShared)
						sprintf(pTempStr, "AuthMode=SHARED");
					else if (pAd->StaCfg.AuthMode == Ndis802_11AuthModeAutoSwitch)
						sprintf(pTempStr, "AuthMode=WEPAUTO");
					else if (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPAPSK)
						sprintf(pTempStr, "AuthMode=WPAPSK");
					else if (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2PSK)
						sprintf(pTempStr, "AuthMode=WPA2PSK");
					else if (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA)
						sprintf(pTempStr, "AuthMode=WPA");
					else if (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPA2)
						sprintf(pTempStr, "AuthMode=WPA2");
					else if (pAd->StaCfg.AuthMode == Ndis802_11AuthModeWPANone)
						sprintf(pTempStr, "AuthMode=WPANONE");
				}
				else if (strncmp(pTempStr, "EncrypType=", strlen("EncrypType=")) == 0)
				{
					NdisZeroMemory(pTempStr, 512);
					if (pAd->StaCfg.WepStatus == Ndis802_11WEPDisabled)
						sprintf(pTempStr, "EncrypType=NONE");
					else if (pAd->StaCfg.WepStatus == Ndis802_11WEPEnabled)
						sprintf(pTempStr, "EncrypType=WEP");
					else if (pAd->StaCfg.WepStatus == Ndis802_11TKIPEnable)
						sprintf(pTempStr, "EncrypType=TKIP");
					else if (pAd->StaCfg.WepStatus == Ndis802_11AESEnable)
						sprintf(pTempStr, "EncrypType=AES");
				}
				RtmpOSFileWrite(file_w, pTempStr, strlen(pTempStr));
				RtmpOSFileWrite(file_w, "\n", 1);
			}
			else
			{
				break;
			}
		}
		RtmpOSFileClose(file_w);
	}

WriteErr:   
	if (pTempStr)
		os_free_mem(NULL, pTempStr);
ReadErr:
WriteFileOpenErr:    
	if (cfgData)
		os_free_mem(NULL, cfgData);
out:
	RtmpOSFSInfoChange(&osFSInfo, FALSE);


	DBGPRINT(RT_DEBUG_TRACE, ("<----- WriteConfToDatFile\n"));
	return;
}


INT write_dat_file_thread (
    IN ULONG Context)
{
	RTMP_OS_TASK *pTask;
	RTMP_ADAPTER *pAd;
	//int 	Status = 0;

	pTask = (RTMP_OS_TASK *)Context;

	if (pTask == NULL)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("%s: pTask is NULL\n", __FUNCTION__));
		return 0;
	}
	
	pAd = (PRTMP_ADAPTER)RTMP_OS_TASK_DATA_GET(pTask);

	if (pAd == NULL)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd is NULL\n", __FUNCTION__));
		return 0;
	}

	RtmpOSTaskCustomize(pTask);

	/* Update ssid, auth mode and encr type to DAT file */
	WriteConfToDatFile(pAd);
	
		RtmpOSTaskNotifyToExit(pTask);
	
	return 0;
}

NDIS_STATUS WriteDatThread(
	IN  RTMP_ADAPTER *pAd)
{
	NDIS_STATUS status = NDIS_STATUS_FAILURE;
	RTMP_OS_TASK *pTask;

	if (pAd->bWriteDat == FALSE)
		return 0;

	DBGPRINT(RT_DEBUG_TRACE, ("-->WriteDatThreadInit()\n"));

	pTask = &pAd->WriteDatTask;

	RTMP_OS_TASK_INIT(pTask, "RtmpWriteDatTask", pAd);
	status = RtmpOSTaskAttach(pTask, write_dat_file_thread, (ULONG)&pAd->WriteDatTask);
	DBGPRINT(RT_DEBUG_TRACE, ("<--WriteDatThreadInit(), status=%d!\n", status));

	return status;
}
#endif /* PROFILE_STORE */
#endif /* CONFIG_STA_SUPPORT */


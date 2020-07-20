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
	rt65xx.c

	Abstract:
	Specific funcitons and configurations for RT65xx

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/

#ifdef RT65xx

#include	"rt_config.h"




VOID RT65xxDisableTxRx(
	RTMP_ADAPTER *pAd,
	UCHAR Level)
{
	UINT32 MacReg = 0;
	UINT32 MTxCycle;
	BOOLEAN bResetWLAN = FALSE;
	BOOLEAN bFree = TRUE;
	UINT8 CheckFreeTimes = 0;

	if (!IS_RT65XX(pAd))
		return;

	DBGPRINT(RT_DEBUG_TRACE, ("----> %s\n", __FUNCTION__));

	if (Level == RTMP_HALT)
	{
		RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_ACTIVE);
	}

	DBGPRINT(RT_DEBUG_TRACE, ("%s Tx success = %ld\n", 
		__FUNCTION__, (ULONG)pAd->WlanCounters.TransmittedFragmentCount.u.LowPart));
	DBGPRINT(RT_DEBUG_TRACE, ("%s Tx success = %ld\n", 
		__FUNCTION__, (ULONG)pAd->WlanCounters.ReceivedFragmentCount.QuadPart));

	StopDmaTx(pAd, Level);

	/*
		Check page count in TxQ,
	*/
	for (MTxCycle = 0; MTxCycle < 2000; MTxCycle++)
	{
		BOOLEAN bFree = TRUE;
		RTMP_IO_READ32(pAd, 0x438, &MacReg);
		if (MacReg != 0)
			bFree = FALSE;
		RTMP_IO_READ32(pAd, 0xa30, &MacReg);
		if (MacReg & 0x000000FF)
			bFree = FALSE;
		RTMP_IO_READ32(pAd, 0xa34, &MacReg);
		if (MacReg & 0xFF00FF00)
			bFree = FALSE;
		if (bFree)
			break;
		if (MacReg == 0xFFFFFFFF)
		{
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST);
			return;
		}
	}

	if (MTxCycle >= 2000)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Check TxQ page count max\n"));
		RTMP_IO_READ32(pAd, 0x0a30, &MacReg);
		DBGPRINT(RT_DEBUG_TRACE, ("0x0a30 = 0x%08x\n", MacReg));

		RTMP_IO_READ32(pAd, 0x0a34, &MacReg);
		DBGPRINT(RT_DEBUG_TRACE, ("0x0a34 = 0x%08x\n", MacReg));

		RTMP_IO_READ32(pAd, 0x438, &MacReg);
		DBGPRINT(RT_DEBUG_TRACE, ("0x438 = 0x%08x\n", MacReg));
		bResetWLAN = TRUE;
	}

	/*
		Check MAC Tx idle
	*/
	for (MTxCycle = 0; MTxCycle < 2000; MTxCycle++)
	{
		RTMP_IO_READ32(pAd, MAC_STATUS_CFG, &MacReg);
		if (MacReg & 0x1)
			RtmpusecDelay(50);
		else
			break;

		if (MacReg == 0xFFFFFFFF)
		{
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST);
			return;
		}
	}

	if (MTxCycle >= 2000)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Check MAC Tx idle max(0x%08x)\n", MacReg));
		bResetWLAN = TRUE;
	}
	
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST) == FALSE)
	{
		if (Level == RTMP_HALT)
		{
			/*
				Disable MAC TX/RX
			*/
			RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &MacReg);
			MacReg &= ~(0x0000000c);
			RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, MacReg);
		}
		else
		{
			/*
				Disable MAC RX
			*/
			RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &MacReg);
			MacReg &= ~(0x00000008);
			RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, MacReg);
		}
	}

	/*
		Check page count in RxQ,
	*/
	for (MTxCycle = 0; MTxCycle < 2000; MTxCycle++)
	{
		bFree = TRUE;
		RTMP_IO_READ32(pAd, 0x430, &MacReg);
		
		if (MacReg & (0x00FF0000))
			bFree = FALSE;
		
		RTMP_IO_READ32(pAd, 0xa30, &MacReg);
		
		if (MacReg != 0)
			bFree = FALSE;
		
		RTMP_IO_READ32(pAd, 0xa34, &MacReg);
		
		if (MacReg != 0)
			bFree = FALSE;
		
		if (bFree && (CheckFreeTimes > 20) && (!is_inband_cmd_processing(pAd)))
			break;
		
		if (bFree)
			CheckFreeTimes++;
		
		if (MacReg == 0xFFFFFFFF)
		{
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST);
			return;
		}
	}
	
	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_POLL_IDLE);
	
	if (MTxCycle >= 2000)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Check RxQ page count max\n"));
		
		RTMP_IO_READ32(pAd, 0x0a30, &MacReg);
		DBGPRINT(RT_DEBUG_TRACE, ("0x0a30 = 0x%08x\n", MacReg));

		RTMP_IO_READ32(pAd, 0x0a34, &MacReg);
		DBGPRINT(RT_DEBUG_TRACE, ("0x0a34 = 0x%08x\n", MacReg));

		RTMP_IO_READ32(pAd, 0x0430, &MacReg);
		DBGPRINT(RT_DEBUG_TRACE, ("0x0430 = 0x%08x\n", MacReg));
		bResetWLAN = TRUE;
	}

	/*
		Check MAC Rx idle
	*/
	for (MTxCycle = 0; MTxCycle < 2000; MTxCycle++)
	{
		RTMP_IO_READ32(pAd, MAC_STATUS_CFG, &MacReg);
		if (MacReg & 0x2)
			RtmpusecDelay(50);
		else
			break;
		if (MacReg == 0xFFFFFFFF)
		{
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST);
			return;
		}
	}
	
	if (MTxCycle >= 2000)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Check MAC Rx idle max(0x%08x)\n", MacReg));
		bResetWLAN = TRUE;
	}

	StopDmaRx(pAd, Level);

	if ((Level == RTMP_HALT) &&
		(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST) == FALSE))
	{
		if (!pAd->chipCap.ram_code_protect)
			NICEraseFirmware(pAd);
		
		/*
 		 * Disable RF/MAC and do not do reset WLAN under below cases
 		 * 1. Combo card
 		 * 2. suspend including wow application
 		 * 3. radion off command
 		 */
		if ((pAd->chipCap.IsComboChip) || RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_SUSPEND)
				|| RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_CMD_RADIO_OFF))
			bResetWLAN = 0;

		rlt_wlan_chip_onoff(pAd, FALSE, bResetWLAN);
	}
	
	DBGPRINT(RT_DEBUG_TRACE, ("<---- %s\n", __FUNCTION__));
}


#ifdef RTMP_PCI_SUPPORT
void MT76xx_PciMlmeRadioOn(RTMP_ADAPTER *pAd)
{
#ifdef CONFIG_AP_SUPPORT
	INT32 IdBss, MaxNumBss = pAd->ApCfg.BssidNum;
#endif /* CONFIG_AP_SUPPORT */
	UINT32 mac_val = 0;


	MCU_CTRL_INIT(pAd);

	/* Clear Radio off flag*/
	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF);
	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_MCU_SEND_IN_BAND_CMD);

	RTMPRingCleanUp(pAd, QID_AC_BK);
   	RTMPRingCleanUp(pAd, QID_AC_BE);
   	RTMPRingCleanUp(pAd, QID_AC_VI);
   	RTMPRingCleanUp(pAd, QID_AC_VO);
   	RTMPRingCleanUp(pAd, QID_HCCA);
   	RTMPRingCleanUp(pAd, QID_MGMT);
   	RTMPRingCleanUp(pAd, QID_RX);
#ifdef CONFIG_ANDES_SUPPORT
	RTMPRingCleanUp(pAd, QID_CTRL); 
#endif /* CONFIG_ANDES_SUPPORT */
	
	RTMP_ASIC_INTERRUPT_ENABLE(pAd);
	

	PWR_SAVING_OP(pAd, RADIO_ON, 0, 0, 0, 0, 0);

	andes_load_cr(pAd, RF_BBP_CR, 0, 0);
	
	RtmpusecDelay(50);

#ifdef DOT11_VHT_AC
	if (pAd->CommonCfg.BBPCurrentBW == BW_80)
		pAd->hw_cfg.cent_ch = pAd->CommonCfg.vht_cent_ch;
	else
#endif /* DOT11_VHT_AC */
		pAd->hw_cfg.cent_ch = pAd->CommonCfg.CentralChannel;

	AsicSwitchChannel(pAd, pAd->hw_cfg.cent_ch, FALSE);
	AsicLockChannel(pAd, pAd->hw_cfg.cent_ch);

#ifdef MT76x2
	if (IS_MT76x2(pAd)) {
		/* 
 		 * add 504, 50c value per ben kao suggestion for rx receivce packet.
	 	 */
		RTMP_IO_WRITE32(pAd, 0x50c, 0x0);
		RTMP_IO_WRITE32(pAd, 0x504, 0x06000000);
		RtmpOsMsDelay(5);
		RTMP_IO_WRITE32(pAd, 0x504, 0x0);
	}
#endif


	/* Enable Tx/Rx*/
	RTMPEnableRxTx(pAd);
	
	if (pAd->chipCap.ed_cca_enable == TRUE) {
		RTMP_IO_READ32(pAd, TXOP_CTRL_CFG, &mac_val);
		mac_val |= (1 << 20);
		RTMP_IO_WRITE32(pAd, TXOP_CTRL_CFG, mac_val);
	}

	RTMP_IO_READ32(pAd, TXOP_HLDR_ET, &mac_val);
	mac_val |= 2;
	RTMP_IO_WRITE32(pAd, TXOP_HLDR_ET, mac_val);
	

	/* Restore RTS retry count */
	RTMP_IO_WRITE32(pAd, 0x1344, pAd->rts_tx_retry_num);		
	
	/* Clear Radio off flag*/
	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF);
	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_IDLE_RADIO_OFF);

#ifdef LED_CONTROL_SUPPORT
#ifdef CONFIG_AP_SUPPORT
	/* The LEN_RADIO_ON indicates "Radio on but link down", 
	   so AP shall set LED LINK_UP status */
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
    	RTMPSetLED(pAd, LED_LINK_UP);
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
	    /* Set LED*/
	    RTMPSetLED(pAd, LED_RADIO_ON);
	}
#endif /* CONFIG_STA_SUPPORT */
#endif /* LED_CONTROL_SUPPORT */

	RTMP_OS_NETDEV_START_QUEUE(pAd->net_dev);
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		if (MaxNumBss > MAX_MBSSID_NUM(pAd))
			MaxNumBss = MAX_MBSSID_NUM(pAd);
		
		/* first IdBss must not be 0 (BSS0), must be 1 (BSS1) */
		for (IdBss = FIRST_MBSSID; IdBss < MAX_MBSSID_NUM(pAd); IdBss++)
		{
			if (pAd->ApCfg.MBSSID[IdBss].MSSIDDev)
				RTMP_OS_NETDEV_START_QUEUE(pAd->ApCfg.MBSSID[IdBss].MSSIDDev);
		}
#ifdef A_BAND_SUPPORT
		if ( (pAd->CommonCfg.Channel > 14)
			&& (pAd->CommonCfg.bIEEE80211H == 1)
			&& RadarChannelCheck(pAd, pAd->CommonCfg.Channel))
		{
#ifdef DFS_SUPPORT		
			NewRadarDetectionStart(pAd);
#endif /* DFS_SUPPORT */		
		}
#endif /* A_BAND_SUPPORT */	
	}
#endif /* CONFIG_AP_SUPPORT */

	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DEQUEUEPACKET);
}

void MT76xx_PciMlmeRadioOFF(RTMP_ADAPTER *pAd)
{
	
	UINT32 pwr_level = 5, mac_val = 0, bbp_val = 0, loop = 0;
#ifdef CONFIG_AP_SUPPORT
	INT32 IdBss, MaxNumBss = pAd->ApCfg.BssidNum;
#endif /* CONFIG_AP_SUPPORT */

	RTMP_OS_NETDEV_STOP_QUEUE(pAd->net_dev);

	/* disable prim/second EDCCA before disable mac TX/RX */
	RTMP_IO_READ32(pAd, TXOP_CTRL_CFG, &mac_val);
	if ((mac_val & 0x100000) == 0x100000) {
		DBGPRINT(RT_DEBUG_OFF,("%s:: ED CCA has been enabled\n", __FUNCTION__));
		pAd->chipCap.ed_cca_enable = TRUE;
		mac_val &= ~(1 << 20);
		RTMP_IO_WRITE32(pAd, TXOP_CTRL_CFG, mac_val);

		RTMP_IO_READ32(pAd, TXOP_HLDR_ET, &mac_val);
		mac_val &= ~2;
		RTMP_IO_WRITE32(pAd, TXOP_HLDR_ET, mac_val);
	}

	RTMP_IO_READ32(pAd, 0x1004, &mac_val);
	mac_val &= ~0xC;
	RTMP_IO_WRITE32(pAd, 0x1004, mac_val);

	/* Backup the original RTS retry count and then set to 0 */
	RTMP_IO_READ32(pAd, 0x1344, &pAd->rts_tx_retry_num);
	RTMP_IO_WRITE32(pAd, 0x1344, 0x00092B00);
	
	do {
		RTMP_IO_READ32(pAd, 0x1200, &mac_val);
		mac_val &= 0x1;
		RTMP_BBP_IO_READ32(pAd, 0x2130, &bbp_val);
		DBGPRINT(RT_DEBUG_OFF,("%s:: Wait until MAC 0x1200 bit0 and BBP 0x2130 become 0\n", __FUNCTION__));
		RtmpusecDelay(1);
		loop++;
	} while (((mac_val != 0) || (bbp_val != 0)) && (loop < 300));

	if (loop >= 300)
		DBGPRINT(RT_DEBUG_OFF, ("%s:: Wait until MAC 0x1200 bit0 and BBP 0x2130 become 0 > 300 times\n", __FUNCTION__));
	
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		if (MaxNumBss > MAX_MBSSID_NUM(pAd))
			MaxNumBss = MAX_MBSSID_NUM(pAd);
		
		/* first IdBss must not be 0 (BSS0), must be 1 (BSS1) */
		for (IdBss = FIRST_MBSSID; IdBss < MAX_MBSSID_NUM(pAd); IdBss++)
		{
			if (pAd->ApCfg.MBSSID[IdBss].MSSIDDev)
				RTMP_OS_NETDEV_STOP_QUEUE(pAd->ApCfg.MBSSID[IdBss].MSSIDDev);
		}
	}
#endif /* CONFIG_AP_SUPPORT */

	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DEQUEUEPACKET);
	
	DBGPRINT(RT_DEBUG_TRACE, ("\npwr_level = %d\n", pwr_level));

#ifdef AP_SCAN_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		BOOLEAN		Cancelled;

		RTMPCancelTimer(&pAd->MlmeAux.APScanTimer, &Cancelled);
	}
#endif /* AP_SCAN_SUPPORT */

#ifdef LED_CONTROL_SUPPORT
	RTMPSetLED(pAd, LED_RADIO_OFF);
#endif /* LED_CONTROL_SUPPORT */
	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_MCU_SEND_IN_BAND_CMD);
	
	PWR_SAVING_OP(pAd, RADIO_OFF, pwr_level, 0, 0, 0, 0);	
	
	RT65xxDisableTxRx(pAd, GUIRADIO_OFF);
	
	MCU_CTRL_EXIT(pAd);

	/*
		Wait for Andes firmware receiving this in-band command packet
	*/			
	RtmpusecDelay(50);
	
	/* Set Radio off flag*/
	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF);

	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_ACTIVE))
	{
		RTMP_ASIC_INTERRUPT_DISABLE(pAd);
	}
}
#endif


VOID dump_bw_info(RTMP_ADAPTER *pAd)
{
#ifdef DBG
		UINT32 core_r1, agc_r0, be_r0, band_cfg;
		static UCHAR *bw_str[]={"20", "10", "40", "80"};
		UCHAR bw, prim_ch_idx, decode_cap;
		static UCHAR *decode_str[] = {"0", "20", "40", "20/40",
									"80", "20/80", "40/80", "20/40/80"};
		UCHAR tx_prim;


		RTMP_BBP_IO_READ32(pAd, CORE_R1, &core_r1);
		RTMP_BBP_IO_READ32(pAd, AGC1_R0, &agc_r0);
		RTMP_BBP_IO_READ32(pAd, TXBE_R0, &be_r0);
		RTMP_IO_READ32(pAd, TX_BAND_CFG, &band_cfg);
		
		/*  Tx/RX : control channel setting */
		DBGPRINT(RT_DEBUG_OFF, ("\n%s():RegisterSetting: TX_BAND_CFG=0x%x, CORE_R1=0x%x, AGC1_R0=0x%x, TXBE_R0=0x%x\n",
				__FUNCTION__, band_cfg, core_r1, agc_r0, be_r0));
		bw = ((core_r1 & 0x18) >> 3) & 0xff;
		DBGPRINT(RT_DEBUG_OFF, ("[CORE_R1]\n"));
		DBGPRINT(RT_DEBUG_OFF, ("\tTx/Rx BandwidthCtrl(CORE_R1[4:3])=%d(%s MHz)\n", 
					bw, bw_str[bw]));

		DBGPRINT(RT_DEBUG_OFF, ("[AGC_R0]\n"));
		prim_ch_idx = ((agc_r0 & 0x300) >> 8) & 0xff;
		DBGPRINT(RT_DEBUG_OFF, ("\tPrimary Channel Idx(AGC_R0[9:8])=%d\n", prim_ch_idx));
		decode_cap = ((agc_r0 & 0x7000) >> 12);
		DBGPRINT(RT_DEBUG_OFF, ("\tDecodeBWCap(AGC_R0[14:12])=%d(%s MHz Data)\n",
					decode_cap, decode_str[decode_cap]));

		DBGPRINT(RT_DEBUG_OFF, ("[TXBE_R0 - PPM]\n"));
		tx_prim = (be_r0 & 0x3);
		DBGPRINT(RT_DEBUG_OFF, ("\tTxPrimary(TXBE_R0[1:0])=%d\n", tx_prim));
#endif
}

#endif /* RT65xx */


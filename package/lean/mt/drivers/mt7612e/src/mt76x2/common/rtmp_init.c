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
	rtmp_init.c

	Abstract:
	Miniport generic portion header file

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/
#include	"rt_config.h"

#ifdef OS_ABL_FUNC_SUPPORT
/* Os utility link: printk, scanf */
RTMP_OS_ABL_OPS RaOsOps, *pRaOsOps = &RaOsOps;
#endif /* OS_ABL_FUNC_SUPPORT */

UCHAR NUM_BIT8[] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};
#ifdef DBG
char *CipherName[] = {"none","wep64","wep128","TKIP","AES","CKIP64","CKIP128","CKIP152","SMS4"};
#endif



/*
	Use the global variable is not a good solution.
	But we can not put it to pAd and use the lock in pAd of RALINK_TIMER_STRUCT;
	Or when the structure is cleared, we maybe get NULL for pAd and can not lock.
	Maybe we can put pAd in RTMPSetTimer/ RTMPModTimer/ RTMPCancelTimer.
*/
NDIS_SPIN_LOCK TimerSemLock;


/*
	========================================================================

	Routine Description:
		Allocate RTMP_ADAPTER data block and do some initialization

	Arguments:
		Adapter		Pointer to our adapter

	Return Value:
		NDIS_STATUS_SUCCESS
		NDIS_STATUS_FAILURE

	IRQL = PASSIVE_LEVEL

	Note:

	========================================================================
*/
NDIS_STATUS RTMPAllocAdapterBlock(VOID *handle, VOID **ppAdapter)
{
	RTMP_ADAPTER *pAd = NULL;
	NDIS_STATUS	 Status;
	INT index;
	UCHAR *pBeaconBuf = NULL;


#ifdef OS_ABL_FUNC_SUPPORT
	/* must put the function before any print message */
	/* init OS utilities provided from UTIL module */
	RtmpOsOpsInit(&RaOsOps);
#endif /* OS_ABL_FUNC_SUPPORT */

	DBGPRINT(RT_DEBUG_TRACE, ("--> RTMPAllocAdapterBlock\n"));

	/* init UTIL module */
	RtmpUtilInit();

	*ppAdapter = NULL;

	do
	{
		os_alloc_mem(NULL, (UCHAR **)&pBeaconBuf, MAX_BEACON_SIZE);
		if (pBeaconBuf == NULL)
		{
			Status = NDIS_STATUS_FAILURE;
			DBGPRINT_ERR(("Failed to allocate memory - BeaconBuf!\n"));
			break;
		}
		NdisZeroMemory(pBeaconBuf, MAX_BEACON_SIZE);

		/* Allocate RTMP_ADAPTER memory block*/
		Status = AdapterBlockAllocateMemory(handle, (PVOID *)&pAd, sizeof(RTMP_ADAPTER));
		if (Status != NDIS_STATUS_SUCCESS)
		{
			DBGPRINT_ERR(("Failed to allocate memory - ADAPTER\n"));
			break;
		}
		else
		{
			/* init resource list (must be after pAd allocation) */
			initList(&pAd->RscTimerMemList);
			initList(&pAd->RscTaskMemList);
			initList(&pAd->RscLockMemList);
			initList(&pAd->RscTaskletMemList);
			initList(&pAd->RscSemMemList);
			initList(&pAd->RscAtomicMemList);

			initList(&pAd->RscTimerCreateList);

			pAd->OS_Cookie = handle;
#ifdef WORKQUEUE_BH
			((POS_COOKIE)(handle))->pAd_va = (UINT32)pAd;
#endif /* WORKQUEUE_BH */
		}
		pAd->BeaconBuf = pBeaconBuf;
		DBGPRINT(RT_DEBUG_OFF, ("\n\n=== pAd = %p, size = %d ===\n\n", pAd, sizeof(RTMP_ADAPTER)));

		if (RtmpOsStatsAlloc(&pAd->iw_stats) == FALSE)
		{
			Status = NDIS_STATUS_FAILURE;
			break;
		}

		/* Init spin locks*/
		NdisAllocateSpinLock(pAd, &pAd->MgmtRingLock);
#ifdef RTMP_MAC_PCI
		for (index = 0; index < NUM_OF_RX_RING; index++)
			NdisAllocateSpinLock(pAd, &pAd->RxRingLock[index]);
#ifdef CONFIG_ANDES_SUPPORT
		NdisAllocateSpinLock(pAd, &pAd->CtrlRingLock);
		NdisAllocateSpinLock(pAd, &pAd->mcu_atomic);
#endif /* CONFIG_ANDES_SUPPORT */
		NdisAllocateSpinLock(pAd, &pAd->tssi_lock);
#endif /* RTMP_MAC_PCI */

#if defined(RT3290) || defined(RLT_MAC)
		NdisAllocateSpinLock(pAd, &pAd->WlanEnLock);
#endif /* defined(RT3290) || defined(RLT_MAC) */

		for (index =0 ; index < NUM_OF_TX_RING; index++)
		{
			NdisAllocateSpinLock(pAd, &pAd->TxSwQueueLock[index]);
			NdisAllocateSpinLock(pAd, &pAd->DeQueueLock[index]);
			pAd->DeQueueRunning[index] = FALSE;
		}

#ifdef RESOURCE_PRE_ALLOC
		/*
			move this function from rt28xx_init() to here. now this function only allocate memory and
			leave the initialization job to RTMPInitTxRxRingMemory() which called in rt28xx_init().
		*/
		Status = RTMPAllocTxRxRingMemory(pAd);
		if (Status != NDIS_STATUS_SUCCESS)
		{
			DBGPRINT_ERR(("Failed to allocate memory - TxRxRing\n"));
			break;
		}
#endif /* RESOURCE_PRE_ALLOC */

		NdisAllocateSpinLock(pAd, &pAd->irq_lock);

#ifdef RTMP_MAC_PCI
		NdisAllocateSpinLock(pAd, &pAd->LockInterrupt);
#endif /* RTMP_MAC_PCI */

		NdisAllocateSpinLock(pAd, &TimerSemLock);

#ifdef SPECIFIC_BCN_BUF_SUPPORT
#ifdef RTMP_MAC_PCI
		NdisAllocateSpinLock(pAd, &pAd->ShrMemLock);
#endif /* RTMP_MAC_PCI */
#endif /* SPECIFIC_BCN_BUF_SUPPORT */


#ifdef RALINK_ATE
#endif /* RALINK_ATE */

		*ppAdapter = (VOID *)pAd;
	} while (FALSE);

	if (Status != NDIS_STATUS_SUCCESS)
	{
		if (pBeaconBuf)
			os_free_mem(NULL, pBeaconBuf);

		if (pAd != NULL)
		{
			if (pAd->iw_stats != NULL)
				os_free_mem(NULL, pAd->iw_stats);

			os_free_mem(NULL, pAd);
		}

		return Status;
	}

	/* Init ProbeRespIE Table */
	for (index = 0; index < MAX_LEN_OF_BSS_TABLE; index++)
	{
		if (os_alloc_mem(pAd,&pAd->ProbeRespIE[index].pIe, MAX_VIE_LEN) == NDIS_STATUS_SUCCESS)
			RTMPZeroMemory(pAd->ProbeRespIE[index].pIe, MAX_VIE_LEN);
		else
			pAd->ProbeRespIE[index].pIe = NULL;
	}

	DBGPRINT_S(Status, ("<-- RTMPAllocAdapterBlock, Status=%x\n", Status));
	return Status;
}


static UINT8 NICGetBandSupported(RTMP_ADAPTER *pAd)
{
	if (BOARD_IS_5G_ONLY(pAd))
	{
		return RFIC_5GHZ;
	}
	else if (BOARD_IS_2G_ONLY(pAd))
	{
		return RFIC_24GHZ;
	}
	else if (RFIC_IS_5G_BAND(pAd))
	{
		return RFIC_DUAL_BAND;
	}
	else
		return RFIC_24GHZ;
}


BOOLEAN RTMPCheckPhyMode(RTMP_ADAPTER *pAd, UINT8 band_cap, UCHAR *pPhyMode)
{
	BOOLEAN RetVal = TRUE;

	if (band_cap == RFIC_24GHZ)
	{
		if (!WMODE_2G_ONLY(*pPhyMode))
		{
			DBGPRINT(RT_DEBUG_TRACE,
					("%s(): Warning! The board type is 2.4G only!\n",
					__FUNCTION__));
			RetVal =  FALSE;
		}
	}
	else if (band_cap == RFIC_5GHZ)
	{
		if (!WMODE_5G_ONLY(*pPhyMode))
		{
			DBGPRINT(RT_DEBUG_TRACE,
					("%s(): Warning! The board type is 5G only!\n",
					__FUNCTION__));
			RetVal =  FALSE;
		}
	}
	else if (band_cap == RFIC_DUAL_BAND)
	{
		RetVal = TRUE;
	}
	else
	{
		DBGPRINT(RT_DEBUG_TRACE,
				("%s(): Unknown supported band (%u), assume dual band used.\n",
				__FUNCTION__, band_cap));

		RetVal = TRUE;
	}

	if (RetVal == FALSE)
	{
#ifdef DOT11_N_SUPPORT
		if (band_cap == RFIC_5GHZ) /*5G ony: change to A/N mode */
			*pPhyMode = PHY_11AN_MIXED;
		else /* 2.4G only or Unknown supported band: change to B/G/N mode */
			*pPhyMode = PHY_11BGN_MIXED;
#else
		if (band_cap == RFIC_5GHZ) /*5G ony: change to A mode */
			*pPhyMode = PHY_11A;
		else /* 2.4G only or Unknown supported band: change to B/G mode */
			*pPhyMode = PHY_11BG_MIXED;
#endif /* !DOT11_N_SUPPORT */

		DBGPRINT(RT_DEBUG_TRACE,
				("%s(): Changed PhyMode to %u\n",
				__FUNCTION__, *pPhyMode));
	}

	return RetVal;

}


/*
	========================================================================

	Routine Description:
		Read initial parameters from EEPROM

	Arguments:
		Adapter						Pointer to our adapter

	Return Value:
		None

	IRQL = PASSIVE_LEVEL

	Note:

	========================================================================
*/
VOID NICReadEEPROMParameters(RTMP_ADAPTER *pAd, PSTRING mac_addr)
{
	USHORT i, value, value2;
	EEPROM_TX_PWR_STRUC Power;
	EEPROM_VERSION_STRUC Version;
	EEPROM_ANTENNA_STRUC Antenna;
	EEPROM_NIC_CONFIG2_STRUC NicConfig2;
#ifdef READ_MAC_FROM_EEPROM
	USHORT  Addr01,Addr23,Addr45 ;
#endif
	MAC_DW0_STRUC csr2;
	MAC_DW1_STRUC csr3;

	DBGPRINT(RT_DEBUG_TRACE, ("--> NICReadEEPROMParameters\n"));

	if (pAd->chipOps.eeinit)
	{
#ifndef MULTIPLE_CARD_SUPPORT
		/* If we are run in Multicard mode, the eeinit shall execute in RTMP_CardInfoRead() */
		pAd->chipOps.eeinit(pAd);
#endif /* MULTIPLE_CARD_SUPPORT */
	}

#ifdef READ_MAC_FROM_EEPROM
	/* Read MAC setting from EEPROM and record as permanent MAC address */
	DBGPRINT(RT_DEBUG_ERROR, ("Initialize MAC Address from E2PROM \n"));

	RT28xx_EEPROM_READ16(pAd, 0x04, Addr01);
	RT28xx_EEPROM_READ16(pAd, 0x06, Addr23);
	RT28xx_EEPROM_READ16(pAd, 0x08, Addr45);

	pAd->PermanentAddress[0] = (UCHAR)(Addr01 & 0xff);
	pAd->PermanentAddress[1] = (UCHAR)(Addr01 >> 8);
	pAd->PermanentAddress[2] = (UCHAR)(Addr23 & 0xff);
	pAd->PermanentAddress[3] = (UCHAR)(Addr23 >> 8);
	pAd->PermanentAddress[4] = (UCHAR)(Addr45 & 0xff);
	pAd->PermanentAddress[5] = (UCHAR)(Addr45 >> 8);

	/*more conveninet to test mbssid, so ap's bssid &0xf1*/
	if (pAd->PermanentAddress[0] == 0xff)
		pAd->PermanentAddress[0] = RandomByte(pAd)&0xf8;

	DBGPRINT(RT_DEBUG_ERROR, ("E2PROM MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
								PRINT_MAC(pAd->PermanentAddress)));

	/* Assign the actually working MAC Address */
	if (pAd->bLocalAdminMAC)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("Use the MAC address what is assigned from Configuration file(.dat). \n"));
#if defined(BB_SOC)&&!defined(NEW_MBSSID_MODE)
		BBUPrepareMAC(pAd, pAd->CurrentAddress);
		COPY_MAC_ADDR(pAd->PermanentAddress, pAd->CurrentAddress);
		printk("now bb MainSsid mac %02x:%02x:%02x:%02x:%02x:%02x\n",PRINT_MAC(pAd->CurrentAddress));
#endif
	}
	else
#endif
	if (mac_addr && strlen((PSTRING)mac_addr) == 17 &&
			 (strcmp(mac_addr, "00:00:00:00:00:00") != 0))
	{
		INT j;
		PSTRING	macptr;

		macptr = (PSTRING) mac_addr;
		for (j=0; j<MAC_ADDR_LEN; j++)
		{
			AtoH(macptr, &pAd->CurrentAddress[j], 1);
			macptr=macptr+3;
		}

		DBGPRINT(RT_DEBUG_ERROR, ("Use the MAC address what is assigned from Moudle Parameter. \n"));
	}
#ifdef READ_MAC_FROM_EEPROM
	else
	{
		COPY_MAC_ADDR(pAd->CurrentAddress, pAd->PermanentAddress);
		DBGPRINT(RT_DEBUG_ERROR, ("Use the MAC address what is assigned from EEPROM. \n"));
	}
#endif

#if !defined(NEW_MBSSID_MODE)
#if defined(MBSS_SUPPORT)
	/* Test MAC[5] for 4 MAC support */
	if ((pAd->CurrentAddress[5] % 4) != 0)
	{
		pAd->CurrentAddress[5] &= 0xfc; // Max 4 MBSSID
	}
#elif defined(APCLI_SUPPORT)
	/* Test MAC[5] for 2 MAC support */
	if ((pAd->CurrentAddress[5] % 2) != 0)
	{
		pAd->CurrentAddress[5] &= 0xfe; // Max 2 MBSSID
	}
#endif
#endif

	/* Set the current MAC to ASIC */
	csr2.field.Byte0 = pAd->CurrentAddress[0];
	csr2.field.Byte1 = pAd->CurrentAddress[1];
	csr2.field.Byte2 = pAd->CurrentAddress[2];
	csr2.field.Byte3 = pAd->CurrentAddress[3];
	RTMP_IO_WRITE32(pAd, MAC_ADDR_DW0, csr2.word);
	csr3.word = 0;
	csr3.field.Byte4 = pAd->CurrentAddress[4];
	{
		csr3.field.Byte5 = pAd->CurrentAddress[5];
		csr3.field.U2MeMask = 0xff;
	}
	RTMP_IO_WRITE32(pAd, MAC_ADDR_DW1, csr3.word);

#ifdef HDR_TRANS_SUPPORT
	RTMP_IO_WRITE32(pAd, HT_MAC_ADDR_DW0, csr2.word);
	csr3.word &= 0xff00ffff;
	csr3.word |= 0x00410000;				// HW test code
	RTMP_IO_WRITE32(pAd, HT_MAC_ADDR_DW1, csr3.word);
#endif /* HDR_TRANS_SUPPORT */

	DBGPRINT_RAW(RT_DEBUG_ERROR,("Current MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
					PRINT_MAC(pAd->CurrentAddress)));

	/* if not return early. cause fail at emulation.*/
	/* Init the channel number for TX channel power*/
#ifdef RT8592
	if (IS_RT8592(pAd))
		RT85592_ReadChannelPwr(pAd);
	else
#endif /* RT8592 */
#ifdef MT76x2
	if (IS_MT76x2(pAd))
		mt76x2_read_chl_pwr(pAd);
	else
#endif
		RTMPReadChannelPwr(pAd);

	/* if E2PROM version mismatch with driver's expectation, then skip*/
	/* all subsequent E2RPOM retieval and set a system error bit to notify GUI*/
	RT28xx_EEPROM_READ16(pAd, EEPROM_VERSION_OFFSET, Version.word);
	pAd->EepromVersion = Version.field.Version + Version.field.FaeReleaseNumber * 256;
	DBGPRINT(RT_DEBUG_TRACE, ("E2PROM: Version = %d, FAE release #%d\n", Version.field.Version, Version.field.FaeReleaseNumber));

	/* Read BBP default value from EEPROM and store to array(EEPROMDefaultValue) in pAd */
	RT28xx_EEPROM_READ16(pAd, EEPROM_NIC1_OFFSET, value);
	pAd->EEPROMDefaultValue[EEPROM_NIC_CFG1_OFFSET] = value;

	/* EEPROM offset 0x36 - NIC Configuration 1 */
	RT28xx_EEPROM_READ16(pAd, EEPROM_NIC2_OFFSET, value);
	pAd->EEPROMDefaultValue[EEPROM_NIC_CFG2_OFFSET] = value;
	NicConfig2.word = pAd->EEPROMDefaultValue[EEPROM_NIC_CFG2_OFFSET];

#if defined(BT_COEXISTENCE_SUPPORT) || defined(RT3290) || defined(RT8592) || defined(MT76x2)
	RT28xx_EEPROM_READ16(pAd, EEPROM_NIC3_OFFSET, value);
#ifdef RT8592
	if (value == 0xffff)
		value = 0;
#endif /* RT8592 */

	pAd->EEPROMDefaultValue[EEPROM_NIC_CFG3_OFFSET] = value;
	pAd->NicConfig3.word = pAd->EEPROMDefaultValue[EEPROM_NIC_CFG3_OFFSET];
#endif /* defined(BT_COEXISTENCE_SUPPORT) || defined(RT3290) || defined(RT8592) || defined(MT76x2) */

	{
		RT28xx_EEPROM_READ16(pAd, EEPROM_COUNTRY_REGION, value);	/* Country Region*/
		pAd->EEPROMDefaultValue[EEPROM_COUNTRY_REG_OFFSET] = value;
		DBGPRINT(RT_DEBUG_OFF, ("Country Region from e2p = %x\n", value));
	}

	for(i = 0; i < 8; i++)
	{
#if defined(RT65xx) || defined(MT7601) /* MT7650 EEPROM doesn't have those BBP setting @20121001 */
		if (IS_RT65XX(pAd) || IS_MT7601(pAd))
			break;
#endif /* defined(RT65xx) || defined(MT7601)*/

		RT28xx_EEPROM_READ16(pAd, EEPROM_BBP_BASE_OFFSET + i*2, value);
		pAd->EEPROMDefaultValue[i+EEPROM_BBP_ARRAY_OFFSET] = value;
	}

	/* We have to parse NIC configuration 0 at here.*/
	/* If TSSI did not have preloaded value, it should reset the TxAutoAgc to false*/
	/* Therefore, we have to read TxAutoAgc control beforehand.*/
	/* Read Tx AGC control bit*/
		Antenna.word = pAd->EEPROMDefaultValue[EEPROM_NIC_CFG1_OFFSET];

#ifdef MT76x2
	if (IS_MT76x2(pAd))
		mt76x2_antenna_sel_ctl(pAd);
#endif /* MT76x2 */

#ifdef RT8592
	if (IS_RT8592(pAd)) {
		DBGPRINT(RT_DEBUG_OFF, ("RT85592: EEPROM(NicConfig1=0x%04x) - Antenna.RfIcType=%d, TxPath=%d, RxPath=%d\n",
					Antenna.word, Antenna.field.RfIcType, Antenna.field.TxPath, Antenna.field.RxPath));
		// TODO: fix me!!
		Antenna.word = 0;
		Antenna.field.BoardType = 0;
		Antenna.field.RfIcType = 0xf;
		Antenna.field.TxPath = 2;
		Antenna.field.RxPath = 2;
	}
#endif /* RT8592 */



	// TODO: shiang, why we only check oxff00??
	if (((Antenna.word & 0xFF00) == 0xFF00) || IS_MT76x2(pAd))
/*	if (Antenna.word == 0xFFFF)*/
		RTMP_CHIP_ANTENNA_INFO_DEFAULT_RESET(pAd, &Antenna);

	/* Choose the desired Tx&Rx stream.*/
	if ((pAd->CommonCfg.TxStream == 0) || (pAd->CommonCfg.TxStream > Antenna.field.TxPath))
		pAd->CommonCfg.TxStream = Antenna.field.TxPath;

	if ((pAd->CommonCfg.RxStream == 0) || (pAd->CommonCfg.RxStream > Antenna.field.RxPath))
	{
		pAd->CommonCfg.RxStream = Antenna.field.RxPath;

		if ((pAd->MACVersion != RALINK_3883_VERSION) &&
			(pAd->MACVersion != RALINK_2883_VERSION) &&
			(pAd->CommonCfg.RxStream > 2))
		{
			/* only 2 Rx streams for RT2860 series*/
			pAd->CommonCfg.RxStream = 2;
		}
	}

	DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s(): AfterAdjust, RxPath = %d, TxPath = %d\n",
					__FUNCTION__, Antenna.field.RxPath, Antenna.field.TxPath));

#ifdef WSC_INCLUDED
	/* WSC hardware push button function 0811 */
	if ((pAd->MACVersion == 0x28600100) || (pAd->MACVersion == 0x28700100))
		WSC_HDR_BTN_MR_HDR_SUPPORT_SET(pAd, NicConfig2.field.EnableWPSPBC);
#endif /* WSC_INCLUDED */

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		if (NicConfig2.word == 0xffff)
			NicConfig2.word = 0;
	}
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		if ((NicConfig2.word & 0x00ff) == 0xff)
			NicConfig2.word &= 0xff00;

		if ((NicConfig2.word >> 8) == 0xff)
			NicConfig2.word &= 0x00ff;
	}
#endif /* CONFIG_STA_SUPPORT */

	if (NicConfig2.field.DynamicTxAgcControl == 1) {
		pAd->bAutoTxAgcA = pAd->bAutoTxAgcG = TRUE;
#ifdef RT8592
		if (IS_RT8592(pAd))
		{
			pAd->chipCap.bTempCompTxALC = TRUE;
			pAd->chipCap.rx_temp_comp = pAd->NicConfig3.field.rx_temp_comp;
		}
#endif /* RT8592 */
	}
	else
		pAd->bAutoTxAgcA = pAd->bAutoTxAgcG = FALSE;

	/* Save value for future using */
	pAd->NicConfig2.word = NicConfig2.word;

	DBGPRINT_RAW(RT_DEBUG_TRACE, ("NICReadEEPROMParameters: RxPath = %d, TxPath = %d, RfIcType = %d\n",
		Antenna.field.RxPath, Antenna.field.TxPath, Antenna.field.RfIcType));

	/* Save the antenna for future use*/
	pAd->Antenna.word = Antenna.word;

	/* Set the RfICType here, then we can initialize RFIC related operation callbacks*/
	pAd->Mlme.RealRxPath = (UCHAR) Antenna.field.RxPath;
	pAd->RfIcType = (UCHAR) Antenna.field.RfIcType;

#ifdef RT8592
	// TODO: shiang-6590, currently we don't have eeprom value, so directly force to set it as 0xff
	if (IS_RT8592(pAd)) {
		pAd->Mlme.RealRxPath = 2;
		pAd->RfIcType = RFIC_UNKNOWN;
	}
#endif /* RT8592 */


#ifdef MT76x2
	if (IS_MT7662(pAd))
		pAd->RfIcType = RFIC_7662;

	if (IS_MT7612(pAd))
		pAd->RfIcType = RFIC_7612;

	if (IS_MT7602(pAd))
		pAd->RfIcType = RFIC_7602;
#endif

	pAd->phy_ctrl.rf_band_cap = NICGetBandSupported(pAd);

	/* check if the chip supports 5G band */
	if (WMODE_CAP_5G(pAd->CommonCfg.PhyMode))
	{
		if (!RFIC_IS_5G_BAND(pAd))
		{
			DBGPRINT_RAW(RT_DEBUG_ERROR,
						("%s():Err! chip not support 5G band %d!\n",
						__FUNCTION__, pAd->RfIcType));
#ifdef DOT11_N_SUPPORT
			/* change to bgn mode */
			Set_WirelessMode_Proc(pAd, "9");
#else
			/* change to bg mode */
			Set_WirelessMode_Proc(pAd, "0");
#endif /* DOT11_N_SUPPORT */
			pAd->phy_ctrl.rf_band_cap = RFIC_24GHZ;
		}
			pAd->phy_ctrl.rf_band_cap = RFIC_24GHZ | RFIC_5GHZ;
	}
	else
	{
			pAd->phy_ctrl.rf_band_cap = RFIC_24GHZ;
	}


	RTMP_NET_DEV_NICKNAME_INIT(pAd);

#ifdef MT76x2
	if (IS_MT76x2(pAd))
	{
		mt76x2_read_temp_info_from_eeprom(pAd);
#ifdef RTMP_TEMPERATURE_TX_ALC
		mt76x2_read_tx_alc_info_from_eeprom(pAd);
#endif /* RTMP_TEMPERATURE_TX_ALC */
#ifdef SINGLE_SKU_V2
		mt76x2_read_single_sku_info_from_eeprom(pAd);
#endif /* SINGLE_SKU_V2 */
	}
#endif /* MT76x2 */

	/* Read TSSI reference and TSSI boundary for temperature compensation. This is ugly */
	/* 0. 11b/g*/
#ifdef RT65xx
	if (IS_MT76x0(pAd) || IS_MT76x2(pAd))
	{
		;
	}
	else
#endif /* RT65xx */
	{
		/* these are tempature reference value (0x00 ~ 0xFE)
		   ex: 0x00 0x15 0x25 0x45 0x88 0xA0 0xB5 0xD0 0xF0
		   TssiPlusBoundaryG [4] [3] [2] [1] [0] (smaller) +
		   TssiMinusBoundaryG[0] [1] [2] [3] [4] (larger) */
		{
			RT28xx_EEPROM_READ16(pAd, EEPROM_G_TSSI_BOUND1, Power.word);
			pAd->TssiMinusBoundaryG[4] = Power.field.Byte0;
			pAd->TssiMinusBoundaryG[3] = Power.field.Byte1;
			RT28xx_EEPROM_READ16(pAd, EEPROM_G_TSSI_BOUND2, Power.word);
			pAd->TssiMinusBoundaryG[2] = Power.field.Byte0;
			pAd->TssiMinusBoundaryG[1] = Power.field.Byte1;
			RT28xx_EEPROM_READ16(pAd, EEPROM_G_TSSI_BOUND3, Power.word);
			pAd->TssiRefG   = Power.field.Byte0; /* reference value [0] */
			pAd->TssiPlusBoundaryG[1] = Power.field.Byte1;
			RT28xx_EEPROM_READ16(pAd, EEPROM_G_TSSI_BOUND4, Power.word);
			pAd->TssiPlusBoundaryG[2] = Power.field.Byte0;
			pAd->TssiPlusBoundaryG[3] = Power.field.Byte1;
			RT28xx_EEPROM_READ16(pAd, EEPROM_G_TSSI_BOUND5, Power.word);
			pAd->TssiPlusBoundaryG[4] = Power.field.Byte0;
			pAd->TxAgcStepG = Power.field.Byte1;
			pAd->TxAgcCompensateG = 0;
			pAd->TssiMinusBoundaryG[0] = pAd->TssiRefG;
			pAd->TssiPlusBoundaryG[0]  = pAd->TssiRefG;

			/* Disable TxAgc if the based value is not right*/
			if (pAd->TssiRefG == 0xff)
				pAd->bAutoTxAgcG = FALSE;
		}

		DBGPRINT(RT_DEBUG_TRACE,("E2PROM: G Tssi[-4 .. +4] = %d %d %d %d - %d -%d %d %d %d, step=%d, tuning=%d\n",
			pAd->TssiMinusBoundaryG[4], pAd->TssiMinusBoundaryG[3], pAd->TssiMinusBoundaryG[2], pAd->TssiMinusBoundaryG[1],
			pAd->TssiRefG,
			pAd->TssiPlusBoundaryG[1], pAd->TssiPlusBoundaryG[2], pAd->TssiPlusBoundaryG[3], pAd->TssiPlusBoundaryG[4],
			pAd->TxAgcStepG, pAd->bAutoTxAgcG));
	}

	/* 1. 11a*/
#ifdef RT65xx
	if (IS_MT76x0(pAd) || IS_MT76x2(pAd))
	{
		;
	}
	else
#endif /* RT65xx */
	{
		{
			RT28xx_EEPROM_READ16(pAd, EEPROM_A_TSSI_BOUND1, Power.word);
			pAd->TssiMinusBoundaryA[0][4] = Power.field.Byte0;
			pAd->TssiMinusBoundaryA[0][3] = Power.field.Byte1;
			RT28xx_EEPROM_READ16(pAd, EEPROM_A_TSSI_BOUND2, Power.word);
			pAd->TssiMinusBoundaryA[0][2] = Power.field.Byte0;
			pAd->TssiMinusBoundaryA[0][1] = Power.field.Byte1;
			RT28xx_EEPROM_READ16(pAd, EEPROM_A_TSSI_BOUND3, Power.word);
			pAd->TssiRefA = Power.field.Byte0;
			pAd->TssiPlusBoundaryA[0][1] = Power.field.Byte1;
			RT28xx_EEPROM_READ16(pAd, EEPROM_A_TSSI_BOUND4, Power.word);
			pAd->TssiPlusBoundaryA[0][2] = Power.field.Byte0;
			pAd->TssiPlusBoundaryA[0][3] = Power.field.Byte1;
			RT28xx_EEPROM_READ16(pAd, EEPROM_A_TSSI_BOUND5, Power.word);
			pAd->TssiPlusBoundaryA[0][4] = Power.field.Byte0;
			pAd->TxAgcStepA = Power.field.Byte1;    
			pAd->TxAgcCompensateA = 0;
			pAd->TssiMinusBoundaryA[0][0] = pAd->TssiRefA;
			pAd->TssiPlusBoundaryA[0][0]  = pAd->TssiRefA;

			/* Disable TxAgc if the based value is not right*/
			if (pAd->TssiRefA == 0xff)
				pAd->bAutoTxAgcA = FALSE;
		}

		DBGPRINT(RT_DEBUG_TRACE,("E2PROM: A Tssi[-4 .. +4] = %d %d %d %d - %d -%d %d %d %d, step=%d, tuning=%d\n",
			pAd->TssiMinusBoundaryA[0][4], pAd->TssiMinusBoundaryA[0][3], pAd->TssiMinusBoundaryA[0][2], pAd->TssiMinusBoundaryA[0][1],
			pAd->TssiRefA,
			pAd->TssiPlusBoundaryA[0][1], pAd->TssiPlusBoundaryA[0][2], pAd->TssiPlusBoundaryA[0][3], pAd->TssiPlusBoundaryA[0][4],
			pAd->TxAgcStepA, pAd->bAutoTxAgcA));
	}
	pAd->BbpRssiToDbmDelta = 0x0;

	/* Read frequency offset setting for RF*/
		RT28xx_EEPROM_READ16(pAd, EEPROM_FREQ_OFFSET, value);

#ifdef RT6352
	if (IS_RT6352(pAd))
	{
		pAd->RfFreqOffset = (ULONG)(value & 0x00FF);
	}
	else
#endif /* RT6352 */
	{
		if ((value & 0x00FF) != 0x00FF)
			pAd->RfFreqOffset = (ULONG) (value & 0x00FF);
		else
			pAd->RfFreqOffset = 0;
	}

#ifdef RTMP_RBUS_SUPPORT
	if (pAd->infType == RTMP_DEV_INF_RBUS)
	{
		if (pAd->RfFreqDelta & 0x10)
		{
			pAd->RfFreqOffset = (pAd->RfFreqOffset >= pAd->RfFreqDelta)? (pAd->RfFreqOffset - (pAd->RfFreqDelta & 0xf)) : 0;
		}
		else
		{
#ifdef RT6352
			if (IS_RT6352(pAd))
				pAd->RfFreqOffset = ((pAd->RfFreqOffset + pAd->RfFreqDelta) < 0xFF)? (pAd->RfFreqOffset + (pAd->RfFreqDelta & 0xf)) : 0xFF;
			else
#endif /* RT6352 */
				pAd->RfFreqOffset = ((pAd->RfFreqOffset + pAd->RfFreqDelta) < 0x40)? (pAd->RfFreqOffset + (pAd->RfFreqDelta & 0xf)) : 0x3f;
		}
	}
#endif /* RTMP_RBUS_SUPPORT */

	DBGPRINT(RT_DEBUG_TRACE, ("E2PROM: RF FreqOffset=0x%x \n", pAd->RfFreqOffset));

#if !defined(EEPROM_COUNTRY_UNLOCK)
	/*CountryRegion byte offset (38h)*/
	{
		value = pAd->EEPROMDefaultValue[EEPROM_COUNTRY_REG_OFFSET] >> 8;		/* 2.4G band*/
		value2 = pAd->EEPROMDefaultValue[EEPROM_COUNTRY_REG_OFFSET] & 0x00FF;	/* 5G band*/
	}

	if ((value <= REGION_MAXIMUM_BG_BAND) || (value == REGION_31_BG_BAND) || (value == REGION_32_BG_BAND) || (value == REGION_33_BG_BAND) )
		pAd->CommonCfg.CountryRegion = ((UCHAR) value) | EEPROM_IS_PROGRAMMED;

	if (value2 <= REGION_MAXIMUM_A_BAND)
		pAd->CommonCfg.CountryRegionForABand = ((UCHAR) value2) | EEPROM_IS_PROGRAMMED;
#endif

	/* Get RSSI Offset on EEPROM 0x9Ah & 0x9Ch.*/
	/* The valid value are (-10 ~ 10) */
	/* */
#ifdef MT76x2
	if (IS_MT76x2(pAd)) {
		RT28xx_EEPROM_READ16(pAd, EEPROM_RSSI_BG_OFFSET, value);

		if (((value & 0xff) == 0x00) || ((value & 0xff) == 0xff)) {
			pAd->BGRssiOffset[0] = 0;
		} else {
			if (value & RSSI0_OFFSET_G_BAND_EN) {
				if (value & RSSI0_OFFSET_G_BAND_SIGN)
					pAd->BGRssiOffset[0] = (value & RSSI0_OFFSET_G_BAND_MASK);
				else
					pAd->BGRssiOffset[0] = -(value & RSSI0_OFFSET_G_BAND_MASK);
			} else {
				pAd->BGRssiOffset[0] = 0;
			}
		}

		if (((value & 0xff00) == 0x0000) || ((value & 0xff00) == 0xff00)) {
			pAd->BGRssiOffset[1] = 0;
		} else {
			if (value & RSSI1_OFFSET_G_BAND_EN) {
				if (value & RSSI1_OFFSET_G_BAND_SIGN)
					pAd->BGRssiOffset[1] = ((value & RSSI1_OFFSET_G_BAND_MASK) >> 8);
				else
					pAd->BGRssiOffset[1] = -((value & RSSI1_OFFSET_G_BAND_MASK) >> 8);
			} else {
				pAd->BGRssiOffset[1] = 0;
			}
		}
	}
	else
#endif /* MT76x2 */
	{
		RT28xx_EEPROM_READ16(pAd, EEPROM_RSSI_BG_OFFSET, value);
		pAd->BGRssiOffset[0] = value & 0x00ff;
		pAd->BGRssiOffset[1] = (value >> 8);
	}

	{
		RT28xx_EEPROM_READ16(pAd, EEPROM_RSSI_BG_OFFSET+2, value);
		{
/*			if (IS_RT2860(pAd))  RT2860 supports 3 Rx and the 2.4 GHz RSSI #2 offset is in the EEPROM 0x48*/
				pAd->BGRssiOffset[2] = value & 0x00ff;
			pAd->ALNAGain1 = (value >> 8);
		}
	}

	{
		RT28xx_EEPROM_READ16(pAd, EEPROM_LNA_OFFSET, value);
		pAd->BLNAGain = value & 0x00ff;
		/* External LNA gain for 5GHz Band(CH36~CH64) */
		pAd->ALNAGain0 = (value >> 8);
	}


#ifdef MT76x2
	if (IS_MT76x2(pAd)) {
		RT28xx_EEPROM_READ16(pAd, EEPROM_RSSI_A_OFFSET, value);

		if (((value & 0xff) == 0x00) || ((value & 0xff) == 0xff)) {
			pAd->ARssiOffset[0] = 0;
		} else {
			if (value & RSSI0_OFFSET_A_BANE_EN) {
				if (value & RSSI0_OFFSET_A_BAND_SIGN)
					pAd->ARssiOffset[0] = (value & RSSI0_OFFSET_A_BAND_MASK);
				else
					pAd->ARssiOffset[0] = -(value & RSSI0_OFFSET_A_BAND_MASK);
			} else {
				pAd->ARssiOffset[0] = 0;
			}
		}

		if (((value & 0xff00) == 0x0000) || ((value & 0xff00) == 0xff00)) {
			pAd->ARssiOffset[1] = 0;
		} else {
			if (value & RSSI1_OFFSET_A_BAND_EN) {
				if (value & RSSI1_OFFSET_A_BAND_SIGN)
					pAd->ARssiOffset[1] = ((value & RSSI1_OFFSET_A_BAND_MASK) >> 8);
				else
					pAd->ARssiOffset[1] = -((value & RSSI1_OFFSET_A_BAND_MASK) >> 8);
			} else {
				pAd->ARssiOffset[1] = 0;
			}
		}
	}
	else
#endif /* MT76x2 */
	{
		RT28xx_EEPROM_READ16(pAd, EEPROM_RSSI_A_OFFSET, value);
		pAd->ARssiOffset[0] = value & 0x00ff;
		pAd->ARssiOffset[1] = (value >> 8);
	}
	{
		RT28xx_EEPROM_READ16(pAd, (EEPROM_RSSI_A_OFFSET+2), value);
		{
			pAd->ARssiOffset[2] = value & 0x00ff;
			pAd->ALNAGain2 = (value >> 8);
		}
	}


	if (((UCHAR)pAd->ALNAGain1 == 0xFF) || (pAd->ALNAGain1 == 0x00))
		pAd->ALNAGain1 = pAd->ALNAGain0;
	if (((UCHAR)pAd->ALNAGain2 == 0xFF) || (pAd->ALNAGain2 == 0x00))
		pAd->ALNAGain2 = pAd->ALNAGain0;

	DBGPRINT(RT_DEBUG_TRACE, ("ALNAGain0 = %d, ALNAGain1 = %d, ALNAGain2 = %d\n",
					pAd->ALNAGain0, pAd->ALNAGain1, pAd->ALNAGain2));

	/* Validate 11a/b/g RSSI 0/1/2 offset.*/
	for (i =0 ; i < 3; i++)
	{
		if ((pAd->BGRssiOffset[i] < -10) || (pAd->BGRssiOffset[i] > 10))
			pAd->BGRssiOffset[i] = 0;

		if ((pAd->ARssiOffset[i] < -10) || (pAd->ARssiOffset[i] > 10))
			pAd->ARssiOffset[i] = 0;
	}

	/*
		Get TX mixer gain setting
		0xff are invalid value
		Note:
			RT30xx default value is 0x00 and will program to RF_R17
				only when this value is not zero
			RT359x default value is 0x02
	*/
	if (IS_RT30xx(pAd) || IS_RT3572(pAd)  || IS_RT3593(pAd)
		|| IS_RT5390(pAd) || IS_RT5392(pAd) || IS_RT5592(pAd)
		|| IS_RT3290(pAd) || IS_RT65XX(pAd) || IS_MT7601(pAd))
	{
		RT28xx_EEPROM_READ16(pAd, EEPROM_TXMIXER_GAIN_2_4G, value);
		pAd->TxMixerGain24G = 0;
		value &= 0x00ff;
		if (value != 0xff)
		{
			value &= 0x07;
			pAd->TxMixerGain24G = (UCHAR)value;
		}

	}


#ifdef LED_CONTROL_SUPPORT
	/* LED Setting */
	RTMPGetLEDSetting(pAd);
#endif /* LED_CONTROL_SUPPORT */

	RTMPReadTxPwrPerRate(pAd);

#ifdef MT76x2
	if (IS_MT76x2(pAd))
		mt76x2_get_external_lna_gain(pAd);
#endif /* MT76x2 */

#ifdef RT6352
	if (IS_RT6352(pAd))
	{
		/* init base power by e2p target power */
		RT28xx_EEPROM_READ16(pAd, 0xD0, pAd->E2p_D0_Value);
		DBGPRINT(RT_DEBUG_ERROR, ("E2PROM: D0 target power=0x%x \n", pAd->E2p_D0_Value));

#ifdef RTMP_TEMPERATURE_CALIBRATION
		RT28xx_EEPROM_READ16(pAd, EEPROM_NIC3_OFFSET, value);
		if (value & 0x0800)
		{
			pAd->bRef25CVaild = FALSE;
		}
		else
		{
			pAd->bRef25CVaild = TRUE;
			pAd->TemperatureRef25C = (pAd->E2p_D0_Value >> 8) & 0xFF;
			DBGPRINT(RT_DEBUG_ERROR, (" pAd->TemperatureRef25C = 0x%x\n", pAd->TemperatureRef25C));
		}
#endif /* RTMP_TEMPERATURE_CALIBRATION */

		/* Get 40 MW Power Delta */
	 	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_DELTA, value);
	 	pAd->BW_Power_Delta = 0;
	 	if ((value & 0xff) != 0xff)
	 	{
	  		if ((value  & 0x80))
	   			pAd->BW_Power_Delta = (value &0xf);

	 		if ((value  & 0x40) == 0)
	   			pAd->BW_Power_Delta = -1* pAd->BW_Power_Delta;
	 	}
		DBGPRINT(RT_DEBUG_ERROR, ("E2PROM: 40 MW Power Delta= %d \n", pAd->BW_Power_Delta));

#ifdef RT6352_EP_SUPPORT
		if ((NicConfig2.word != 0) && (pAd->EEPROMDefaultValue[EEPROM_NIC_CFG2_OFFSET] & 0xC000))
			pAd->bExtPA = TRUE;
		else
#endif /* RT6352_EP_SUPPORT */
			pAd->bExtPA = FALSE;
	}
#endif /* RT6352 */

#ifdef SINGLE_SKU
	{
		RT28xx_EEPROM_READ16(pAd, EEPROM_DEFINE_MAX_TXPWR, pAd->CommonCfg.DefineMaxTxPwr);
	}

	/*
		Some dongle has old EEPROM value, use ModuleTxpower for saving correct value fo DefineMaxTxPwr.
		ModuleTxpower will override DefineMaxTxPwr (value from EEPROM) if ModuleTxpower is not zero.
	*/
	if (pAd->CommonCfg.ModuleTxpower > 0)
		pAd->CommonCfg.DefineMaxTxPwr = pAd->CommonCfg.ModuleTxpower;

	DBGPRINT(RT_DEBUG_TRACE, ("TX Power set for SINGLE SKU MODE is : 0x%04x \n", pAd->CommonCfg.DefineMaxTxPwr));

	pAd->CommonCfg.bSKUMode = FALSE;
	if ((pAd->CommonCfg.DefineMaxTxPwr & 0xFF) <= 0x50)
	{
		if (IS_RT3883(pAd))
			pAd->CommonCfg.bSKUMode = TRUE;
		else if ((pAd->CommonCfg.AntGain > 0) && (pAd->CommonCfg.BandedgeDelta >= 0))
			pAd->CommonCfg.bSKUMode = TRUE;
	}
	DBGPRINT(RT_DEBUG_TRACE, ("Single SKU Mode is %s\n",
				pAd->CommonCfg.bSKUMode ? "Enable" : "Disable"));
#endif /* SINGLE_SKU */

#ifdef SINGLE_SKU_V2
#ifdef RT6352
	if (IS_RT6352(pAd))
		RT6352_InitSkuRateDiffTable(pAd);
#endif /* RT6352 */
#endif /* SINGLE_SKU_V2 */

#ifdef RTMP_EFUSE_SUPPORT
	RtmpEfuseSupportCheck(pAd);
#endif /* RTMP_EFUSE_SUPPORT */

#ifdef RTMP_INTERNAL_TX_ALC
#ifdef RT65xx
	if (IS_MT76x0(pAd) || IS_MT76x2(pAd))
	{
		; // TODO: wait TC6008 EEPROM format
	}
	else
#endif /* RT65xx */
	{
		/*
		    Internal Tx ALC support is starting from RT3370 / RT3390, which combine PA / LNA in single chip.
    		The old chipset don't have this, add new feature flag RTMP_INTERNAL_TX_ALC.
 		*/
		value = pAd->EEPROMDefaultValue[EEPROM_NIC_CFG2_OFFSET];
		if (value == 0xFFFF) /*EEPROM is empty*/
	    	pAd->TxPowerCtrl.bInternalTxALC = FALSE;
		else if (value & 1<<13)
	    	pAd->TxPowerCtrl.bInternalTxALC = TRUE;
		else
	    	pAd->TxPowerCtrl.bInternalTxALC = FALSE;
	}
	DBGPRINT(RT_DEBUG_TRACE, ("TXALC> bInternalTxALC = %d\n", pAd->TxPowerCtrl.bInternalTxALC));
#endif /* RTMP_INTERNAL_TX_ALC */




	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->Antenna.field.BoardType = %d, IS_MINI_CARD(pAd) = %d, IS_RT5390U(pAd) = %d\n",
		__FUNCTION__,
		pAd->Antenna.field.BoardType,
		IS_MINI_CARD(pAd),
		IS_RT5390U(pAd)));


	DBGPRINT(RT_DEBUG_TRACE, ("<-- NICReadEEPROMParameters\n"));
}


/*
	========================================================================

	Routine Description:
		Set default value from EEPROM

	Arguments:
		Adapter						Pointer to our adapter

	Return Value:
		None

	IRQL = PASSIVE_LEVEL

	Note:

	========================================================================
*/
VOID NICInitAsicFromEEPROM(RTMP_ADAPTER *pAd)
{
#ifdef CONFIG_STA_SUPPORT
	UINT32 data = 0;
#endif /* CONFIG_STA_SUPPORT */
#if defined(RT30xx) || defined(RTMP_BBP)
	USHORT i;
#endif /* defined(RT30xx) || defined(RTMP_BBP) */
#ifdef RALINK_ATE
	USHORT value;
#endif /* RALINK_ATE */
	EEPROM_NIC_CONFIG2_STRUC NicConfig2;

	DBGPRINT(RT_DEBUG_TRACE, ("--> NICInitAsicFromEEPROM\n"));
#ifdef RTMP_BBP
	// TODO: shiang, fix this for some chips which has side-effect (ex: 5572/3572, etc.)
	if (pAd->chipCap.hif_type == HIF_RTMP)
	{
		for (i = EEPROM_BBP_ARRAY_OFFSET; i < NUM_EEPROM_BBP_PARMS; i++)
		{
			UCHAR BbpRegIdx, BbpValue;

			if ((pAd->EEPROMDefaultValue[i] != 0xFFFF) && (pAd->EEPROMDefaultValue[i] != 0))
			{
				BbpRegIdx = (UCHAR)(pAd->EEPROMDefaultValue[i] >> 8);
				BbpValue  = (UCHAR)(pAd->EEPROMDefaultValue[i] & 0xff);
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BbpRegIdx, BbpValue);
			}
		}
	}
#endif /* RTMP_BBP */

	NicConfig2.word = pAd->NicConfig2.word;

	/* finally set primary ant */
	AntCfgInit(pAd);

	RTMP_CHIP_ASIC_INIT_TEMPERATURE_COMPENSATION(pAd);

#ifdef RTMP_RF_RW_SUPPORT
	/*Init RFRegisters after read RFIC type from EEPROM*/
	InitRFRegisters(pAd);
#endif /* RTMP_RF_RW_SUPPORT */


#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		/* Read Hardware controlled Radio state enable bit*/
		if (NicConfig2.field.HardwareRadioControl == 1)
		{
			BOOLEAN radioOff = FALSE;
			pAd->StaCfg.bHardwareRadio = TRUE;

			{
				/* Read GPIO pin2 as Hardware controlled radio state*/
				RTMP_IO_READ32(pAd, GPIO_CTRL_CFG, &data);
				if ((data & 0x04) == 0)
					radioOff = TRUE;
			}

			if (radioOff)
			{
				pAd->StaCfg.bHwRadio = FALSE;
				pAd->StaCfg.bRadio = FALSE;
				/* RTMP_IO_WRITE32(pAd, PWR_PIN_CFG, 0x00001818); */
				RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF);
			}
		}
		else
			pAd->StaCfg.bHardwareRadio = FALSE;

#ifdef LED_CONTROL_SUPPORT
		RTMPSetLED(pAd, pAd->StaCfg.bRadio == FALSE ? LED_RADIO_OFF : LED_RADIO_ON);
#endif /* LED_CONTROL_SUPPORT */

#ifdef RTMP_MAC_PCI
		if (pAd->StaCfg.bRadio == TRUE)
		{
			AsicSendCmdToMcuAndWait(pAd, 0x30, PowerRadioOffCID, 0xff, 0x02, FALSE);

			/*AsicSendCommandToMcu(pAd, 0x30, 0xff, 0xff, 0x02, FALSE);*/
			AsicSendCmdToMcuAndWait(pAd, 0x31, PowerWakeCID, 0x00, 0x00, FALSE);
		}
#endif /* RTMP_MAC_PCI */
	}
#ifdef PCIE_PS_SUPPORT
#endif /* PCIE_PS_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */

#ifdef WIN_NDIS
	/* Turn off patching for cardbus controller */
	/*
	if (NicConfig2.field.CardbusAcceleration == 1)
		pAd->bTest1 = TRUE;
	*/
#endif /* WIN_NDIS */

	if (NicConfig2.field.DynamicTxAgcControl == 1)
		pAd->bAutoTxAgcA = pAd->bAutoTxAgcG = TRUE;
	else
		pAd->bAutoTxAgcA = pAd->bAutoTxAgcG = FALSE;

#ifdef RTMP_INTERNAL_TX_ALC
	/*
	    Internal Tx ALC support is starting from RT3370 / RT3390, which combine PA / LNA in single chip.
	    The old chipset don't have this, add new feature flag RTMP_INTERNAL_TX_ALC.
	 */

	/* Internal Tx ALC */
	if (((NicConfig2.field.DynamicTxAgcControl == 1) &&
            (NicConfig2.field.bInternalTxALC == 1)) ||
            ((!IS_RT3390(pAd)) && (!IS_RT3350(pAd)) &&
            (!IS_RT3352(pAd)) && (!IS_RT5350(pAd)) &&
            (!IS_RT5390(pAd)) && (!IS_RT3290(pAd)) && (!IS_RT6352(pAd)) && (!IS_MT7601(pAd))))
	{
		/*
			If both DynamicTxAgcControl and bInternalTxALC are enabled,
			it is a wrong configuration.
			If the chipset does not support Internal TX ALC, we shall disable it.
		*/
		pAd->TxPowerCtrl.bInternalTxALC = FALSE;
	}
	else
	{
		if (NicConfig2.field.bInternalTxALC == 1)
			pAd->TxPowerCtrl.bInternalTxALC = TRUE;
		else
			pAd->TxPowerCtrl.bInternalTxALC = FALSE;
	}


	/* Old 5390 NIC always disables the internal ALC */
	if ((pAd->MACVersion == 0x53900501)
#ifdef RT6352
		&& !IS_RBUS_INF(pAd)
#endif /* RT6352 */
	)
		pAd->TxPowerCtrl.bInternalTxALC = FALSE;

	DBGPRINT(RT_DEBUG_TRACE, ("%s: pAd->TxPowerCtrl.bInternalTxALC = %d\n",
		__FUNCTION__,
		pAd->TxPowerCtrl.bInternalTxALC));
#endif /* RTMP_INTERNAL_TX_ALC */

#ifdef RALINK_ATE
	RT28xx_EEPROM_READ16(pAd, EEPROM_TSSI_GAIN_AND_ATTENUATION, value);
	value = (value & 0x00FF);

	if (IS_RT5390(pAd))
		pAd->TssiGain = 0x02;	 /* RT5390 uses 2 as TSSI gain/attenuation default value */
	else
		pAd->TssiGain = 0x03; /* RT5392 uses 3 as TSSI gain/attenuation default value */

	if ((value != 0x00) && (value != 0xFF))
		pAd->TssiGain =  (UCHAR) (value & 0x000F);

	DBGPRINT(RT_DEBUG_TRACE, ("%s: EEPROM_TSSI_GAIN_AND_ATTENUATION = 0x%X, pAd->TssiGain=0x%x\n",
				__FUNCTION__,
				value,
				pAd->TssiGain));
#endif /* RALINK_ATE */

	bbp_set_rxpath(pAd, pAd->Antenna.field.RxPath);

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		INT dac = 0;
		/* Handle the difference when 1T*/
		{
			if(pAd->Antenna.field.TxPath == 1)
				dac = 0;
			else
				dac = 2;
		}
		bbp_set_txdac(pAd, dac);
		DBGPRINT(RT_DEBUG_TRACE, ("Use Hw Radio Control Pin=%d; if used Pin=%d;\n",
					pAd->StaCfg.bHardwareRadio, pAd->StaCfg.bHardwareRadio));
	}
#endif /* CONFIG_STA_SUPPORT */

	RTMP_EEPROM_ASIC_INIT(pAd);



#ifdef CONFIG_STA_SUPPORT
#endif /* CONFIG_STA_SUPPORT */
	DBGPRINT(RT_DEBUG_TRACE, ("TxPath = %d, RxPath = %d, RFIC=%d\n",
				pAd->Antenna.field.TxPath, pAd->Antenna.field.RxPath, pAd->RfIcType));
	DBGPRINT(RT_DEBUG_TRACE, ("<-- NICInitAsicFromEEPROM\n"));
}


/*
	========================================================================

	Routine Description:
		Initialize NIC hardware

	Arguments:
		Adapter						Pointer to our adapter

	Return Value:
		None

	IRQL = PASSIVE_LEVEL

	Note:

	========================================================================
*/
NDIS_STATUS	NICInitializeAdapter(RTMP_ADAPTER *pAd, BOOLEAN bHardReset)
{
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	UINT rty_cnt = 0;
#ifdef RTMP_MAC_PCI
	WPDMA_GLO_CFG_STRUC GloCfg;
#endif /* RTMP_MAC_PCI */

	DBGPRINT(RT_DEBUG_TRACE, ("--> NICInitializeAdapter\n"));

	/* Set DMA global configuration except TX_DMA_EN and RX_DMA_EN bits */
retry:

	if (AsicWaitPDMAIdle(pAd, 100, 1000) != TRUE) {
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
			return NDIS_STATUS_FAILURE;
	}

#ifdef RTMP_MAC_PCI
	RTMP_IO_READ32(pAd, WPDMA_GLO_CFG, &GloCfg.word);
	GloCfg.word &= 0xff0;
	GloCfg.field.EnTXWriteBackDDONE = 1;
	RTMP_IO_WRITE32(pAd, WPDMA_GLO_CFG, GloCfg.word);
	DBGPRINT(RT_DEBUG_TRACE, ("<== DMA offset 0x208 = 0x%x\n", GloCfg.word));

	/* pbf hardware reset, asic simulation sequence put this ahead before loading firmware */
#ifdef MT76x2
	// TODO: shiang-usw, check why MT762x don't need to do this!
	if (!IS_MT76x2(pAd))
#endif /* MT76x2 */
		RTMP_IO_WRITE32(pAd, WPDMA_RST_IDX, 0xffffffff /*0x1003f*/);

#ifdef RT8592
	// TODO: shiang-single driver, sync with windows, does 765x need this??
	if (!IS_RT65XX(pAd))
#endif /* RT8592 */
	{
		RTMP_IO_WRITE32(pAd, PBF_SYS_CTRL, 0xe1f);
		RTMP_IO_WRITE32(pAd, PBF_SYS_CTRL, 0xe00);
	}
#endif /* RTMP_MAC_PCI */

#ifdef RTMP_MAC_PCI
	AsicInitTxRxRing(pAd);
#endif

	/* Initialze ASIC for TX & Rx operation*/
	if (NICInitializeAsic(pAd , bHardReset) != NDIS_STATUS_SUCCESS)
	{
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
			return NDIS_STATUS_FAILURE;

		if (pAd->chipOps.loadFirmware)
		{
			if (rty_cnt++ == 0)
			{
				NICLoadFirmware(pAd);
				goto retry;
			}
		}
		return NDIS_STATUS_FAILURE;
	}


	DBGPRINT(RT_DEBUG_TRACE, ("<-- NICInitializeAdapter\n"));
	return Status;
}

/*
	========================================================================

	Routine Description:
		Initialize ASIC

	Arguments:
		Adapter						Pointer to our adapter

	Return Value:
		None

	IRQL = PASSIVE_LEVEL

	Note:

	========================================================================
*/
NDIS_STATUS	NICInitializeAsic(RTMP_ADAPTER *pAd, BOOLEAN bHardReset)
{
	ULONG Index = 0;
	UINT32 mac_val = 0;
	USHORT KeyIdx;

	DBGPRINT(RT_DEBUG_TRACE, ("--> NICInitializeAsic\n"));

	/*@!Release
		For MT76x0 series,
		PWR_PIN_CFG[2:0]: obsolete, no function
		Don't need to change PWR_PIN_CFG here.
	*/
#ifdef RTMP_MAC_PCI
#ifdef RT8592
	// TODO:shiang-6590, why we don't need to change PWR_PIN_CFG as 1 here? Windows did it!!
	if (IS_RT8592(pAd))
		mac_val = 0x1;
	else
#endif /* RT8592 */
#if defined(MT76x0) || defined(MT76x2) || defined(MT7601)
	if (IS_MT76x0(pAd) || IS_MT76x2(pAd) || IS_MT7601(pAd)) {
		/*
			PWR_PIN_CFG[2:0]: obsolete, no function
			Don't need to change PWR_PIN_CFG here.
		*/
		mac_val = 0x0;
	}
	else
#endif /* defined(MT76x0) || defined(MT76x2) || defined(MT7601) */
		mac_val = 0x3;	/* To fix driver disable/enable hang issue when radio off*/
	RTMP_IO_WRITE32(pAd, PWR_PIN_CFG, mac_val);

	if (bHardReset == TRUE)
	{
		mac_val = 0x3;
#ifdef RT6352
		if (IS_RT6352(pAd))
			mac_val = 0x1;
#endif /* RT6352 */
	}
	else
		mac_val = 0x1;
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, mac_val);

#ifdef RT6352
	if (IS_RT6352(pAd) && (bHardReset == TRUE))
	{
		UCHAR BbpReg;
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R21, &BbpReg);
		BbpReg |= 0x1;
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R21, BbpReg);
		RtmpOsMsDelay(1);
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R21, &BbpReg);
		BbpReg &= (~0x1);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R21, BbpReg);
	}
#endif /* RT6352 */
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, 0x0);
#endif /* RTMP_MAC_PCI */

#ifdef RTMP_PCI_SUPPORT
	pAd->CommonCfg.bPCIeBus = FALSE; /* PCI bus*/
#ifdef RTMP_MAC
	if ((pAd->chipCap.hif_type == HIF_RTMP) && IS_PCI_INF(pAd))
	{
		{
			/* PCI and PCIe have different value in US_CYC_CNT*/
			RTMP_IO_READ32(pAd, PCI_CFG, &mac_val);
			if ((mac_val & 0x10000) == 0)
			{
				US_CYC_CNT_STRUC USCycCnt;

				RTMP_IO_READ32(pAd, US_CYC_CNT, &mac_val);
				USCycCnt.word = mac_val;
				USCycCnt.field.UsCycCnt = 0x7D;
				RTMP_IO_WRITE32(pAd, US_CYC_CNT, USCycCnt.word);

				pAd->CommonCfg.bPCIeBus = TRUE;
			}
		}
	}
#endif /* RTMP_MAC */
	DBGPRINT(RT_DEBUG_TRACE, ("%s():device act as PCI%s driver\n",
					__FUNCTION__, (pAd->CommonCfg.bPCIeBus ? "-E" : "")));
#endif /* RTMP_PCI_SUPPORT */


#ifdef CONFIG_ANDES_SUPPORT
	if (pAd->chipOps.fw_init)
		pAd->chipOps.fw_init(pAd);
#endif /* CONFIG_ANDES_SUPPORT */
	rtmp_mac_init(pAd);

	rtmp_mac_bcn_buf_init(pAd);

#ifdef RTMP_MAC
	if (pAd->chipCap.hif_type == HIF_RTMP) {
		/* Before program BBP, we need to wait BBP/RF get wake up.*/
		Index = 0;
		do
		{
			if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
				return NDIS_STATUS_FAILURE;

			RTMP_IO_READ32(pAd, MAC_STATUS_CFG, &mac_val);
			if ((mac_val & 0x03) == 0)	/* if BB.RF is stable*/
				break;

			DBGPRINT(RT_DEBUG_TRACE, ("Check if MAC_STATUS_CFG is busy(=%x)\n", mac_val));
			RtmpusecDelay(1000);
		} while (Index++ < 100);
	}
#endif /* RTMP_MAC */

	NICInitBBP(pAd);

#ifdef RTMP_MAC_PCI
	/* TODO: check MACVersion, currently, rbus-based chip use this.*/
	if (pAd->MACVersion == 0x28720200)
	{
		UINT32 value2;

		/*Maximum PSDU length from 16K to 32K bytes	*/
		RTMP_IO_READ32(pAd, MAX_LEN_CFG, &value2);
		value2 &= ~(0x3<<12);
		value2 |= (0x2<<12);
		RTMP_IO_WRITE32(pAd, MAX_LEN_CFG, value2);
	}
#endif /* RTMP_MAC_PCI */

	if ((IS_RT3883(pAd)) || IS_RT65XX(pAd) ||
#ifdef RT6352
		(IS_RT6352(pAd)) ||
#endif /* RT6352 */
		((pAd->MACVersion >= RALINK_2880E_VERSION) &&
		(pAd->MACVersion < RALINK_3070_VERSION))) /* 3*3*/
	{
		UINT32 csr;
		RTMP_IO_READ32(pAd, MAX_LEN_CFG, &csr);
#if defined(RT2883) || defined(RT3883) || defined(RT3593) || defined(RT65xx) || defined(MT7601)
		if (IS_RT2883(pAd) || IS_RT3883(pAd) || IS_RT3593(pAd) || IS_RT65XX(pAd) || IS_MT7601(pAd))
			csr |= 0x3fff;
		else
#endif /* defined(RT2883) || defined(RT3883) || defined(RT3593) || defined(RT65xx) || defined(MT7601) */
#ifdef RT6352
		if (IS_RT6352(pAd))
		{
			csr &= 0xFFF;
			csr |= 0x3000;
		}
		else
#endif /* RT6352 */
		{
			csr &= 0xFFF;
			csr |= 0x2000;
		}
		RTMP_IO_WRITE32(pAd, MAX_LEN_CFG, csr);
	}


#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		/* Add radio off control*/
		if (pAd->StaCfg.bRadio == FALSE)
		{
			/* RTMP_IO_WRITE32(pAd, PWR_PIN_CFG, 0x00001818);*/
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF);
			DBGPRINT(RT_DEBUG_TRACE, ("Set Radio Off\n"));
		}
	}
#endif /* CONFIG_STA_SUPPORT */

	/* Clear raw counters*/
	NicResetRawCounters(pAd);

	/*
		ASIC will keep garbage value after boot
		Clear all shared key table when initial
		This routine can be ignored in radio-ON/OFF operation.
	*/
	if (bHardReset)
	{
		UINT32 wcid_attr_base = 0, wcid_attr_size = 0, share_key_mode_base = 0;
#ifdef RLT_MAC
		if (pAd->chipCap.hif_type == HIF_RLT) {
			wcid_attr_base = RLT_MAC_WCID_ATTRIBUTE_BASE;
			wcid_attr_size = RLT_HW_WCID_ATTRI_SIZE;
			share_key_mode_base = RLT_SHARED_KEY_MODE_BASE;
		}
#endif /* RLT_MAC */
#ifdef RTMP_MAC
		if (pAd->chipCap.hif_type == HIF_RTMP) {
			wcid_attr_base = MAC_WCID_ATTRIBUTE_BASE;
			wcid_attr_size = HW_WCID_ATTRI_SIZE;
			share_key_mode_base = SHARED_KEY_MODE_BASE;
		}
#endif /* RTMP_MAC */

		for (KeyIdx = 0; KeyIdx < 4; KeyIdx++)
		{
			RTMP_IO_WRITE32(pAd, share_key_mode_base + 4*KeyIdx, 0);
		}

		/* Clear all pairwise key table when initial*/
		for (KeyIdx = 0; KeyIdx < 256; KeyIdx++)
		{
			RTMP_IO_WRITE32(pAd, wcid_attr_base + (KeyIdx * wcid_attr_size), 1);
		}
	}

	/* It isn't necessary to clear this space when not hard reset. */
	if (bHardReset == TRUE)
	{
		/* clear all on-chip BEACON frame space */
#ifdef CONFIG_AP_SUPPORT
		INT	i, apidx;
		for (apidx = 0; apidx < HW_BEACON_MAX_COUNT(pAd); apidx++)
		{
			if (pAd->BeaconOffset[apidx] > 0) {
				// TODO: shiang-6590, if we didn't define MBSS_SUPPORT, the pAd->BeaconOffset[x] may set as 0 when chipCap.BcnMaxHwNum != HW_BEACON_MAX_COUNT
				for (i = 0; i < HW_BEACON_OFFSET; i+=4)
					RTMP_CHIP_UPDATE_BEACON(pAd, pAd->BeaconOffset[apidx] + i, 0x00, 4);

			}
		}
#endif /* CONFIG_AP_SUPPORT */
	}




	DBGPRINT(RT_DEBUG_TRACE, ("<-- NICInitializeAsic\n"));
	return NDIS_STATUS_SUCCESS;
}


#ifdef RTMP_RBUS_SUPPORT
/*
	========================================================================

	Routine Description:
		Reset NIC from error

	Arguments:
		Adapter						Pointer to our adapter

	Return Value:
		None

	IRQL = PASSIVE_LEVEL

	Note:
		Reset NIC from error state

	========================================================================
*/
VOID NICResetFromError(RTMP_ADAPTER *pAd)
{
	UCHAR rf_channel;

	/* Reset BBP (according to alex, reset ASIC will force reset BBP*/
	/* Therefore, skip the reset BBP*/
	/* RTMP_IO_WRITE32(pAd, MAC_CSR1, 0x2);*/

	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, 0x1);
	/* Remove ASIC from reset state*/
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, 0x0);

	NICInitializeAdapter(pAd, FALSE);
	NICInitAsicFromEEPROM(pAd);

	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		AsicBBPAdjust(pAd);
	}

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		AsicStaBbpTuning(pAd);
	}
#endif /* CONFIG_STA_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		if (INFRA_ON(pAd) && (pAd->CommonCfg.CentralChannel != pAd->CommonCfg.Channel)
			&& (pAd->MlmeAux.HtCapability.HtCapInfo.ChannelWidth == BW_40))
			rf_channel = pAd->CommonCfg.CentralChannel;
		else
			rf_channel = pAd->CommonCfg.Channel;
	}
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		rf_channel = pAd->CommonCfg.CentralChannel;
	}
#endif /* CONFIG_AP_SUPPORT */

#if defined(CONFIG_AP_SUPPORT) || defined(CONFIG_STA_SUPPORT)
	AsicSwitchChannel(pAd, rf_channel, FALSE);
	AsicLockChannel(pAd, rf_channel);
#endif /* defined(CONFIG_AP_SUPPORT) || defined(CONFIG_STA_SUPPORT) */
}
#endif /* RTMP_RBUS_SUPPORT */


#ifdef CONFIG_AP_SUPPORT
#ifdef RTMP_MAC_PCI
/*
	========================================================================

	Routine Description:
		In the case, Client may be silent left without sending DeAuth or DeAssoc.
		AP'll continue retry packets for the client since AP doesn't know the STA
		is gone. To Minimum affection of exist traffic is disable retransmition for
		all those packet relative to the STA.
		So decide to change ack required setting of all packet in TX ring
		to "no ACK" requirement for specific Client.

	Arguments:
		Adapter						Pointer to our adapter

	Return Value:
		None

	IRQL = DISPATCH_LEVEL

	========================================================================
*/
VOID ClearTxRingClientAck(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry)
{
	INT index;
	USHORT TxIdx;
	RTMP_TX_RING *pTxRing;
	TXD_STRUC *pTxD;
	TXWI_STRUC *pTxWI;
#ifdef RT_BIG_ENDIAN
	PTXD_STRUC	pDestTxD;
	TXD_STRUC	TxD;
	TXWI_STRUC *pDestTxWI;
	TXWI_STRUC	TxWI;
	UINT8 TXWISize;
#endif /* RT_BIG_ENDIAN */

	if (!pAd || !pEntry)
		return;

#ifdef RT_BIG_ENDIAN
	TXWISize = pAd->chipCap.TXWISize;
#endif /* RT_BIG_ENDIAN */

		for (index = 3; index >= 0; index--)
		{
			pTxRing = &pAd->TxRing[index];
			for (TxIdx = 0; TxIdx < TX_RING_SIZE; TxIdx++)
			{
#ifdef RT_BIG_ENDIAN
				pDestTxD = (PTXD_STRUC) pTxRing->Cell[TxIdx].AllocVa;
				TxD = *pDestTxD;
				pTxD = &TxD;
				RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#else
				pTxD = (PTXD_STRUC) pTxRing->Cell[TxIdx].AllocVa;
#endif /* RT_BIG_ENDIAN */

				if (!pTxD->DMADONE)
				{
#ifdef RT_BIG_ENDIAN
					pDestTxWI = (TXWI_STRUC *) pTxRing->Cell[TxIdx].DmaBuf.AllocVa;
					NdisMoveMemory((PUCHAR)&TxWI, (PUCHAR)pDestTxWI, TXWISize);
					pTxWI = &TxWI;
					RTMPWIEndianChange(pAd, (PUCHAR)pTxWI, TYPE_TXWI);
#else
					 pTxWI = (TXWI_STRUC *)pTxRing->Cell[TxIdx].DmaBuf.AllocVa;
#endif /* RT_BIG_ENDIAN */

#ifdef RTMP_MAC
					if (pAd->chipCap.hif_type == HIF_RTMP && pTxWI->TXWI_O.wcid == pEntry->wcid)
						pTxWI->TXWI_O.ACK = FALSE;
#endif /* RTMP_MAC */
#ifdef RLT_MAC
					if (pAd->chipCap.hif_type == HIF_RLT && pTxWI->TXWI_N.wcid == pEntry->wcid)
						pTxWI->TXWI_N.ACK = FALSE;
#endif /* RTL_MAC */

#ifdef RT_BIG_ENDIAN
					RTMPWIEndianChange(pAd, (PUCHAR)pTxWI, TYPE_TXWI);
					NdisMoveMemory((PUCHAR)pDestTxWI, (PUCHAR)pTxWI, TXWISize);
#endif /* RT_BIG_ENDIAN */
				}
			}
		}
}
#endif /* RTMP_MAC_PCI */

VOID ApTxFailCntUpdate(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry, ULONG TxSuccess, ULONG TxRetransmit)
{
#ifdef RT65xx
	if (IS_RT65XX(pAd)) {
		if (pAd->MacTab.Size <= 8) {
#ifdef UAPSD_SUPPORT
			/* check the EOSP packet by RateCtrl's way */
			UAPSD_SP_AUE_Handle(pAd, pEntry, TRUE);
#endif /* UAPSD_SUPPORT */
			
			if ((TxSuccess == 0) && (TxRetransmit > 0))
			{
				/* prevent fast drop long range clients */
				if (TxRetransmit > MAC_ENTRY_LIFE_CHECK_CNT / 4)
					TxRetransmit = MAC_ENTRY_LIFE_CHECK_CNT / 4;
				
				/* No TxPkt ok in this period as continue tx fail */
				pEntry->ContinueTxFailCnt += TxRetransmit;
			}
			else
			{
				pEntry->ContinueTxFailCnt = 0;
			}

			if (TxSuccess > 0)
				pEntry->NoDataIdleCount = 0;

			DBGPRINT(RT_DEBUG_INFO, ("%s:(OK:%ld, FAIL:%ld, ConFail:%d) \n",__FUNCTION__,
					TxSuccess, TxRetransmit, pEntry->ContinueTxFailCnt));
		}
	}
	else
#endif /* RT65xx */
	{
		return;
	}
}
#endif /* CONFIG_AP_SUPPORT */


VOID NICUpdateFifoStaCounters(RTMP_ADAPTER *pAd)
{
#ifdef FIFO_EXT_SUPPORT
	TX_STA_FIFO_EXT_STRUC StaFifoExt;
#endif /* FIFO_EXT_SUPPORT */
	TX_STA_FIFO_STRUC	StaFifo;
	MAC_TABLE_ENTRY		*pEntry = NULL;
	UINT32				i = 0;
	UCHAR				pid = 0, wcid = 0;
	INT32				reTry;
	UCHAR				succMCS, PhyMode;

#ifdef RALINK_ATE
	/* Nothing to do in ATE mode */
	if (ATE_ON(pAd))
		return;
#endif /* RALINK_ATE */


#ifdef CONFIG_AP_SUPPORT
#ifdef RT65xx
	if (pAd->MacTab.Size <= 8) {
		/* mark by UAPSD Accurate Issue */
		if (IS_RT65XX(pAd))
			return;
	}
#endif /* RT65xx */
#endif /* CONFIG_AP_SUPPORT */


	do
	{
#ifdef FIFO_EXT_SUPPORT
		RTMP_IO_READ32(pAd, TX_STA_FIFO_EXT, &StaFifoExt.word);
#endif /* FIFO_EXT_SUPPORT */
		RTMP_IO_READ32(pAd, TX_STA_FIFO, &StaFifo.word);

		if (StaFifo.field.bValid == 0)
			break;

		wcid = (UCHAR)StaFifo.field.wcid;

#ifdef DBG_CTRL_SUPPORT
#ifdef INCLUDE_DEBUG_QUEUE
		if (pAd->CommonCfg.DebugFlags & DBF_DBQ_TXFIFO) {
			dbQueueEnqueue(0x73, (UCHAR *)(&StaFifo.word));
		}
#endif /* INCLUDE_DEBUG_QUEUE */
#endif /* DBG_CTRL_SUPPORT */

		/* ignore NoACK and MGMT frame use 0xFF as WCID */
		if ((StaFifo.field.TxAckRequired == 0) || (wcid >= MAX_LEN_OF_MAC_TABLE))
		{
			i++;
			continue;
		}

		/* PID store Tx MCS Rate */
#ifdef FIFO_EXT_SUPPORT
#ifdef RT65xx
		if (IS_MT76x2(pAd))
		{
			PhyMode = StaFifo.field.PhyMode;
			if((PhyMode == 2) || (PhyMode == 3))
			{
 				pid = (UCHAR)StaFifoExt.field.PidType & 0xF;
			}
			else if(PhyMode == 4)
			{
				pid = (UCHAR)StaFifoExt.field.PidType & 0xF;
				pid += (((UCHAR)StaFifoExt.field.PidType & 0x10) ? 10 : 0);
			}
		}
		else
#endif /* RT65xx */
#endif /* FIFO_EXT_SUPPORT */

		pid = (UCHAR)StaFifo.field.PidType;

		pEntry = &pAd->MacTab.Content[wcid];


#ifdef CONFIG_STA_SUPPORT
#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
		if (!StaFifo.field.TxSuccess)
		{
			IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
			{
				if(IS_ENTRY_TDLS(pEntry))
                    {
                        DBGPRINT(RT_DEBUG_ERROR,(" pEntry->TdlsTxFailCount : %d\n", pEntry->TdlsTxFailCount));
                        pEntry->TdlsTxFailCount++;
                        if (pEntry->TdlsTxFailCount >= 15 && pAd->StaCfg.wpa_supplicant_info.CFG_Tdls_info.bDoingPeriodChannelSwitch == FALSE)
                        {
                                DBGPRINT(RT_DEBUG_OFF, ("TDLS: TxFail >= 15 LinkTearDown !!!\n"));
                                                        TDLS_TearDownPeerLink(pAd, pEntry->Addr, FALSE);
                        }
			if(pAd->StaCfg.wpa_supplicant_info.CFG_Tdls_info.bDoingPeriodChannelSwitch == TRUE)
				pEntry->TdlsTxFailCount=0;
				}
			}
		}
		else
		{
			pEntry->NoDataIdleCount = 0;
			pEntry->TdlsTxFailCount=0;
		}
#endif /* defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) */
#endif /* CONFIG_STA_SUPPORT */


		pEntry->DebugFIFOCount++;


#ifdef DOT11_N_SUPPORT
#ifdef TXBF_SUPPORT
		/* Update BF statistics*/
		if (pAd->chipCap.FlgHwTxBfCap)
		{
			int succMCS = (StaFifo.field.SuccessRate & 0x7F);
			int origMCS = pid;

			if (succMCS==32)
				origMCS = 32;
#ifdef DOT11N_SS3_SUPPORT
			if (succMCS>origMCS && pEntry->HTCapability.MCSSet[2]==0xff)
				origMCS += 16;
#endif /* DOT11N_SS3_SUPPORT */

			if (succMCS>origMCS)
				origMCS = succMCS+1;

			/* MCS16 falls back to MCS8*/
			if (origMCS>=16 && succMCS<=8)
				succMCS += 8;

			/* MCS8 falls back to 0 */
			if (origMCS >= 8 && succMCS == 0)
				succMCS += 7;

			reTry = origMCS-succMCS;

			if (StaFifo.field.eTxBF) {
				if (StaFifo.field.TxSuccess)
					pEntry->TxBFCounters.ETxSuccessCount++;
				else
					pEntry->TxBFCounters.ETxFailCount++;
					pEntry->TxBFCounters.ETxRetryCount += reTry;
			}
			else if (StaFifo.field.iTxBF) {
				if (StaFifo.field.TxSuccess)
					pEntry->TxBFCounters.ITxSuccessCount++;
				else
					pEntry->TxBFCounters.ITxFailCount++;
				pEntry->TxBFCounters.ITxRetryCount += reTry;
			}
			else {
			if (StaFifo.field.TxSuccess)
				pEntry->TxBFCounters.TxSuccessCount++;
			else
				pEntry->TxBFCounters.TxFailCount++;
			pEntry->TxBFCounters.TxRetryCount += reTry;
		}
	}
#endif /* TXBF_SUPPORT */
#endif /* DOT11_N_SUPPORT */

#ifdef UAPSD_SUPPORT
	UAPSD_SP_AUE_Handle(pAd, pEntry, StaFifo.field.TxSuccess);
#endif /* UAPSD_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))
		continue;
#endif /* CONFIG_STA_SUPPORT */

	if (!StaFifo.field.TxSuccess)
	{
		pEntry->FIFOCount++;
		pEntry->OneSecTxFailCount++;
		pEntry->TotalTxFailCount++;
#ifdef CONFIG_AP_SUPPORT
		pEntry->StatTxFailCount += pEntry->OneSecTxFailCount;
		pAd->ApCfg.MBSSID[pEntry->apidx].StatTxFailCount += pEntry->StatTxFailCount;
#endif /* CONFIG_AP_SUPPORT */

#ifdef DYNAMIC_VGA_SUPPORT
		pEntry->DyncVgaOneSecTxCount++;
#endif /*DYNAMIC_VGA_SUPPORT*/

		if (pEntry->FIFOCount >= 1)
		{
			DBGPRINT(RT_DEBUG_TRACE, ("#"));
#ifdef DOT11_N_SUPPORT
			//pEntry->NoBADataCountDown = 64;
#endif /* DOT11_N_SUPPORT */

			/* Update the continuous transmission counter.*/
#ifdef FIFO_EXT_SUPPORT
			if (StaFifoExt.field.txRtyCnt > 0)
				pEntry->ContinueTxFailCnt += StaFifoExt.field.txRtyCnt;
			else
#endif
			pEntry->ContinueTxFailCnt++;

			if(pEntry->PsMode == PWR_ACTIVE)
			{
#ifdef DOT11_N_SUPPORT
#ifdef CONFIG_AP_SUPPORT
#ifdef NOISE_TEST_ADJUST
				if ((pAd->ApCfg.EntryClientCount > 2) &&
					(pEntry->HTPhyMode.field.MODE >= MODE_HTMIX) &&
					(pEntry->lowTrafficCount >= 4 /* 2 sec */))
					pEntry->NoBADataCountDown = 10;
#endif /* NOISE_TEST_ADJUST */
#endif /* CONFIG_AP_SUPPORT */

#endif /* DOT11_N_SUPPORT */

#ifdef WDS_SUPPORT
				/* fix WDS Jam issue*/
				if(IS_ENTRY_WDS(pEntry)
					&& (pEntry->LockEntryTx == FALSE)
					&& (pEntry->ContinueTxFailCnt >= pAd->ApCfg.EntryLifeCheck))
				{
					DBGPRINT(RT_DEBUG_TRACE, ("Entry %02x:%02x:%02x:%02x:%02x:%02x Blocked!! (Fail Cnt = %d)\n",
							PRINT_MAC(pEntry->Addr), pEntry->ContinueTxFailCnt ));

					pEntry->LockEntryTx = TRUE;
				}
#endif /* WDS_SUPPORT */
			}
		}
#ifdef CONFIG_AP_SUPPORT
#ifdef RTMP_MAC_PCI
		/* if Tx fail >= 20, then clear TXWI ack in Tx Ring*/
		if (pEntry->ContinueTxFailCnt >= pAd->ApCfg.EntryLifeCheck)
			ClearTxRingClientAck(pAd, pEntry);
#endif /* RTMP_MAC_PCI */
#endif /* CONFIG_AP_SUPPORT */
	}
	else
	{
#ifdef DOT11_N_SUPPORT
		if ((pEntry->PsMode != PWR_SAVE) && (pEntry->NoBADataCountDown > 0))
		{
			pEntry->NoBADataCountDown--;
			if (pEntry->NoBADataCountDown==0)
			{
				DBGPRINT(RT_DEBUG_TRACE, ("@\n"));
			}
		}
#endif /* DOT11_N_SUPPORT */
		pEntry->FIFOCount = 0;
		pEntry->OneSecTxNoRetryOkCount++;


		/* update NoDataIdleCount when sucessful send packet to STA.*/
		pEntry->NoDataIdleCount = 0;
		pEntry->ContinueTxFailCnt = 0;
#ifdef WDS_SUPPORT
		pEntry->LockEntryTx = FALSE;
#endif /* WDS_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
#if defined (DOT11Z_TDLS_SUPPORT) || defined (CFG_TDLS_SUPPORT)
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{
			if(IS_ENTRY_TDLS(pEntry))
				pEntry->TdlsTxFailCount = 0;
		}
#endif /* DOT11Z_TDLS_SUPPORT || CFG_TDLS_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */
	}

#ifdef RT65xx
	if (IS_MT76x2(pAd))
	{
		PhyMode = StaFifo.field.PhyMode;
		if((PhyMode == 2) || (PhyMode == 3))
		{
  	    		succMCS = StaFifo.field.SuccessRate & 0xF;

#ifdef DOT11N_SS3_SUPPORT
			if (pEntry->HTCapability.MCSSet[2] == 0xff)
			{
				if (succMCS > pid)
					pid = pid + 16;
			}
#endif /* DOT11N_SS3_SUPPORT */

			if (StaFifo.field.TxSuccess)
			{
				pEntry->TXMCSExpected[pid]++;
				if (pid == succMCS)
					pEntry->TXMCSSuccessful[pid]++;
				else {
#ifdef CUSTOMER_DCC_FEATURE
#ifdef MBSS_802_11_STATISTICS
					pAd->ApCfg.MBSSID[pEntry->apidx].TxRetriedPktCount++;
					pEntry->TxRetriedPktCount++;
					pAd->RadioStatsCounter.TxRetriedPktCount++;
#endif
#endif
					pEntry->TXMCSAutoFallBack[pid][succMCS]++;
				}
			}
			else
			{
#ifdef CUSTOMER_DCC_FEATURE
#ifdef MBSS_802_11_STATISTICS
				pAd->ApCfg.MBSSID[pEntry->apidx].TxRetriedPktCount++;
				pEntry->TxRetriedPktCount++;
				pAd->RadioStatsCounter.TxRetriedPktCount++;
#endif
#endif
				pEntry->TXMCSFailed[pid]++;
			}

#ifdef DOT11N_SS3_SUPPORT
			if (pid >= 16 && succMCS <= 8)
				succMCS += (2 - (succMCS >> 3)) * 7;
#endif /* DOT11N_SS3_SUPPORT */

			reTry = pid - succMCS;

			if (reTry > 0)
			{
				/* MCS8 falls back to 0 */
				if (pid>=8 && succMCS==0)
					reTry -= 7;
		    		//else if ((pid >= 12) && succMCS <=7)
			    	//	reTry -= 4;

				pEntry->OneSecTxRetryOkCount += reTry;

#ifdef CONFIG_AP_SUPPORT
				pEntry->StatTxRetryOkCount += pEntry->OneSecTxRetryOkCount;
				pAd->ApCfg.MBSSID[pEntry->apidx].StatTxRetryOkCount += pEntry->StatTxRetryOkCount;
#endif /* CONFIG_AP_SUPPORT */
				
#ifdef DYNAMIC_VGA_SUPPORT
				pEntry->DyncVgaOneSecTxCount += reTry;
#endif /*DYNAMIC_VGA_SUPPORT*/
			}
		}
		else if(PhyMode == 4)
		{
  	    		succMCS = StaFifo.field.SuccessRate & 0xF;
			succMCS += ((StaFifo.field.SuccessRate & 0x10) ? 10 : 0);
			//DBGPRINT(0, ("%s()Succ MCS :TxMCS(%d):PHYMode(%d)\n", __FUNCTION__, pid, PhyMode));
		    	if (StaFifo.field.TxSuccess)
		    	{
		    		pEntry->TXMCSExpected[pid]++;

			    	if (pid == succMCS)
			    		pEntry->TXMCSSuccessful[pid]++;
			    	else {
#ifdef CUSTOMER_DCC_FEATURE
#ifdef MBSS_802_11_STATISTICS
						pAd->ApCfg.MBSSID[pEntry->apidx].TxRetriedPktCount++;
						pEntry->TxRetriedPktCount++;
						pAd->RadioStatsCounter.TxRetriedPktCount++;
#endif
#endif
			    		pEntry->TXMCSAutoFallBack[pid][succMCS]++;
			    	}
		    	}
		    	else
			{
#ifdef CUSTOMER_DCC_FEATURE
#ifdef MBSS_802_11_STATISTICS
					pAd->ApCfg.MBSSID[pEntry->apidx].TxRetriedPktCount++;
					pEntry->TxRetriedPktCount++;
					pAd->RadioStatsCounter.TxRetriedPktCount++;
#endif
#endif
				pEntry->TXMCSFailed[pid]++;
			}

			reTry = pid - succMCS;

			if (reTry > 0)
			{
				/* MCS10 falls back to 0 */
				if (pid >= 10 && succMCS == 0)
					reTry -= 9;

				pEntry->OneSecTxRetryOkCount += reTry;
#ifdef DYNAMIC_VGA_SUPPORT
				pEntry->DyncVgaOneSecTxCount += reTry;
#endif /*DYNAMIC_VGA_SUPPORT*/
			}

			if(reTry <= 0)
				pEntry->DownTxMCSRate[0]++;
			else if(reTry > (NUM_OF_SWFB-1))
				pEntry->DownTxMCSRate[NUM_OF_SWFB-1]++;
			else
				pEntry->DownTxMCSRate[reTry]++;
			}
		}
		else
#endif /* RT65xx */
		{
			succMCS = StaFifo.field.SuccessRate & 0x7F;

#ifdef DOT11N_SS3_SUPPORT
			if (pEntry->HTCapability.MCSSet[2] == 0xff)
			{
				if (succMCS > pid)
					pid = pid + 16;
			}
#endif /* DOT11N_SS3_SUPPORT */

			if (StaFifo.field.TxSuccess)
			{
				pEntry->TXMCSExpected[pid]++;
				if (pid == succMCS)
				{
					pEntry->TXMCSSuccessful[pid]++;
				}
				else
				{
#ifdef CUSTOMER_DCC_FEATURE
#ifdef MBSS_802_11_STATISTICS
					pAd->ApCfg.MBSSID[pEntry->apidx].TxRetriedPktCount++;
					pEntry->TxRetriedPktCount++;
					pAd->RadioStatsCounter.TxRetriedPktCount++;
#endif
#endif
					pEntry->TXMCSAutoFallBack[pid][succMCS]++;
				}
			}
			else
			{
#ifdef CUSTOMER_DCC_FEATURE
#ifdef MBSS_802_11_STATISTICS
				pAd->ApCfg.MBSSID[pEntry->apidx].TxRetriedPktCount++;
				pEntry->TxRetriedPktCount++;
				pAd->RadioStatsCounter.TxRetriedPktCount++;
#endif
#endif
				pEntry->TXMCSFailed[pid]++;
			}

#ifdef DOT11N_SS3_SUPPORT
			if (pid >= 16 && succMCS <= 8)
				succMCS += (2 - (succMCS >> 3)) * 7;
#endif /* DOT11N_SS3_SUPPORT */

			reTry = pid - succMCS;

			if (reTry > 0)
			{
				/* MCS8 falls back to 0 */
				if (pid>=8 && succMCS==0)
					reTry -= 7;
				else if ((pid >= 12) && succMCS <=7)
				{
					reTry -= 4;
				}

				pEntry->OneSecTxRetryOkCount += reTry;
#ifdef DYNAMIC_VGA_SUPPORT
				pEntry->DyncVgaOneSecTxCount += reTry;
#endif /*DYNAMIC_VGA_SUPPORT*/
			}
		}

		i++;	/* ASIC store 16 stack*/
	} while ( i < (TX_RING_SIZE<<1) );

}


#ifdef FIFO_EXT_SUPPORT
BOOLEAN NicGetMacFifoTxCnt(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry)
{
	if (pEntry->bUseHwFifoExt)
	{
		WCID_TX_CNT_STRUC wcidTxCnt;

		RTMP_IO_READ32(pAd, pAd->FifoExtTbl[pEntry->hwFifoExtIdx].hwTxCntCROffset, &wcidTxCnt.word);

		pEntry->fifoTxSucCnt += wcidTxCnt.field.succCnt;
		pEntry->fifoTxRtyCnt += wcidTxCnt.field.reTryCnt;

		return TRUE;
	}

	return FALSE;
}


/*
	The 8 entries will be dynamically replaced to apcli-repeater entry, starting from tail of table, at most replace 4 entries:
		{ CLI, CLI, CLI, CLI, CLI, CLI, CLI, CLI}			still 4 vacancy entries
	--> { CLI, CLI, CLI, CLI, CLI, CLI, CLI, APCLI}			still 3 vacancy entries
	--> { CLI, CLI, CLI, CLI, CLI, CLI, APCLI, APCLI}		still 2 vacancy entries
	--> { CLI, CLI, CLI, CLI, CLI, APCLI, APCLI, APCLI}		still 1 vacancy entry
	--> { CLI, CLI, CLI, CLI, APCLI, APCLI, APCLI, APCLI}	no vacancy entry
*/
BOOLEAN IsFifoExtTblAvailable(IN RTMP_ADAPTER *pAd, IN UCHAR *pTblIdx)
{
	MAC_TABLE_ENTRY *pEntry;
	int vacancy = -1;	/* initialized by a invalid value */
	int i;

	for (i = (FIFO_EXT_HW_SIZE - 1); i >= 4; i --)
	{
		pEntry = &pAd->MacTab.Content[pAd->FifoExtTbl[i].wcid];

		if (IS_ENTRY_APCLI(pEntry))
			continue;

		/* The first non-ApCli entry (starting from tail) */
		vacancy = i;
		break;
	}

	if (vacancy == -1)
		return FALSE;

	DBGPRINT(RT_DEBUG_TRACE, ("FIFO_EXT: find vacancy entry %d\n", vacancy));
	*pTblIdx = (UCHAR)vacancy;
	return TRUE;
}


VOID FifoExtTblUpdateEntry(IN RTMP_ADAPTER *pAd, IN UCHAR tblIdx, IN UCHAR wcid)
{
	MAC_TABLE_ENTRY *pNewEntry, *pOldEntry;
	UCHAR oldWcid;

	if (tblIdx >= FIFO_EXT_HW_SIZE)
		return;

	oldWcid = pAd->FifoExtTbl[tblIdx].wcid;
	if (oldWcid == wcid)
		return;

	pOldEntry = &pAd->MacTab.Content[oldWcid];	
	pNewEntry = &pAd->MacTab.Content[wcid];

	/* Clear the old one */
	pOldEntry->bUseHwFifoExt = FALSE;

	/* Set the new one */
	pNewEntry->hwFifoExtIdx = tblIdx;
	pNewEntry->bUseHwFifoExt = TRUE;

	/* Update global setting */
	pAd->FifoExtTbl[tblIdx].wcid = wcid;
	RTMP_IO_WRITE8(pAd, WCID_MAPPING_0 + tblIdx, wcid);
}


const UCHAR PreDefWcid[] = {1, 2, 3, 4, 5, 6, 7, 8};

VOID FifoExtTblRmReptEntry(IN RTMP_ADAPTER *pAd, IN UCHAR wcid)
{ 
	MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.Content[wcid];
	UCHAR tblIdx = pEntry->hwFifoExtIdx;

	FifoExtTblUpdateEntry(pAd, tblIdx, PreDefWcid[tblIdx]);
}

VOID FifoExtTableInit(IN RTMP_ADAPTER *pAd)
{
	if (pAd->chipCap.FlgHwFifoExtCap)
	{
		UCHAR i, wcid;

		for (i = 0; i < FIFO_EXT_HW_SIZE; i ++)
		{
			wcid = PreDefWcid[i];
			pAd->FifoExtTbl[i].wcid = wcid;
			pAd->FifoExtTbl[i].hwTxCntCROffset = WCID_TX_CNT_0 + (i * 4);

			/* set pEntry */
			pAd->MacTab.Content[wcid].hwFifoExtIdx = i;
			pAd->MacTab.Content[wcid].bUseHwFifoExt = TRUE;
		}
	}
}


VOID AsicFifoExtSet(IN RTMP_ADAPTER *pAd)
{
	if (pAd->chipCap.FlgHwFifoExtCap)
	{
		UCHAR i;
		UINT32 WcidSet;

		WcidSet = 0;
		for (i = 0; i < 4; i ++)
			WcidSet |= (pAd->FifoExtTbl[i].wcid << (i * 8));

		RTMP_IO_WRITE32(pAd, WCID_MAPPING_0, WcidSet);

		WcidSet = 0;
		for (i = 0; i < 4; i ++)
			WcidSet |= (pAd->FifoExtTbl[i + 4].wcid << (i * 8));
 
		RTMP_IO_WRITE32(pAd, WCID_MAPPING_1, WcidSet);
	}
}


VOID AsicFifoExtEntryClean(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry)
{
	WCID_TX_CNT_STRUC wcidTxCnt;
			
	if (pAd->chipCap.FlgHwFifoExtCap)
	{
		/* We clean the fifo info when entry Tx Counter is controlled by hw */
		if (pEntry->bUseHwFifoExt)
			RTMP_IO_READ32(pAd, pAd->FifoExtTbl[pEntry->hwFifoExtIdx].hwTxCntCROffset, &wcidTxCnt.word);
	}
}
#endif /* FIFO_EXT_SUPPORT */


/*
	========================================================================

	Routine Description:
		Read Tx statistic raw counters from hardware registers and record to
		related software variables for later on query

	Arguments:
		pAd					Pointer to our adapter
		pStaTxCnt0			Pointer to record "TX_STA_CNT0" (0x170c)
		pStaTxCnt1			Pointer to record "TX_STA_CNT1" (0x1710)

	Return Value:
		None

	========================================================================
*/
VOID NicGetTxRawCounters(
	IN RTMP_ADAPTER *pAd,
	IN TX_STA_CNT0_STRUC *pStaTxCnt0,
	IN TX_STA_CNT1_STRUC *pStaTxCnt1)
{

	RTMP_IO_READ32(pAd, TX_STA_CNT0, &pStaTxCnt0->word);
	RTMP_IO_READ32(pAd, TX_STA_CNT1, &pStaTxCnt1->word);

	pAd->bUpdateBcnCntDone = TRUE;	/* not appear in Rory's code */
	pAd->RalinkCounters.OneSecBeaconSentCnt += pStaTxCnt0->field.TxBeaconCount;
	pAd->RalinkCounters.OneSecTxRetryOkCount += pStaTxCnt1->field.TxRetransmit;
	pAd->RalinkCounters.OneSecTxNoRetryOkCount += pStaTxCnt1->field.TxSuccess;
	pAd->RalinkCounters.OneSecTxFailCount += pStaTxCnt0->field.TxFailCount;

#ifdef STATS_COUNT_SUPPORT
	pAd->WlanCounters.TransmittedFragmentCount.u.LowPart += pStaTxCnt1->field.TxSuccess;
	pAd->WlanCounters.RetryCount.u.LowPart += pStaTxCnt1->field.TxRetransmit;
	pAd->WlanCounters.FailedCount.u.LowPart += pStaTxCnt0->field.TxFailCount;
#endif /* STATS_COUNT_SUPPORT */


}

/*
	========================================================================
	
	Routine Description:
		Read Tx RTS counters from hardware registers and record to
		related software variables for later on query

	Arguments:
		pAd					Pointer to our adapter
		pRtsTxCnt			Pointer to record "RTS_TX_CNT" (0x1744)

	Return Value:
		None

	========================================================================
*/
VOID NicGetTxRTSCounters(
	IN RTMP_ADAPTER *pAd,
	IN RTS_TX_CNT_STRUC *pRtsTxCnt)
{

	RTMP_IO_READ32(pAd, RTS_TX_CNT, &pRtsTxCnt->word);

#ifdef STATS_COUNT_SUPPORT
	pAd->WlanCounters.RTSSuccessCount.u.LowPart += pRtsTxCnt->field.RtsTxOkCount;
	pAd->WlanCounters.RTSFailureCount.u.LowPart += pRtsTxCnt->field.RtsTxFailCount;
#endif /* STATS_COUNT_SUPPORT */

}

/*
	========================================================================

	Routine Description:
		Clean all Tx/Rx statistic raw counters from hardware registers

	Arguments:
		pAd					Pointer to our adapter

	Return Value:
		None

	========================================================================
*/
VOID NicResetRawCounters(RTMP_ADAPTER *pAd)
{
	UINT32 Counter;

	RTMP_IO_READ32(pAd, RX_STA_CNT0, &Counter);
	RTMP_IO_READ32(pAd, RX_STA_CNT1, &Counter);
	RTMP_IO_READ32(pAd, RX_STA_CNT2, &Counter);
	RTMP_IO_READ32(pAd, TX_STA_CNT0, &Counter);
	RTMP_IO_READ32(pAd, TX_STA_CNT1, &Counter);
	RTMP_IO_READ32(pAd, TX_STA_CNT2, &Counter);
}


/*
	========================================================================

	Routine Description:
		Read statistical counters from hardware registers and record them
		in software variables for later on query

	Arguments:
		pAd					Pointer to our adapter

	Return Value:
		None

	IRQL = DISPATCH_LEVEL

	========================================================================
*/
VOID NICUpdateRawCounters(RTMP_ADAPTER *pAd)
{
	UINT32	OldValue;/*, Value2;*/
	/*ULONG	PageSum, OneSecTransmitCount;*/
	/*ULONG	TxErrorRatio, Retry, Fail;*/
	RX_STA_CNT0_STRUC	 RxStaCnt0;
	RX_STA_CNT1_STRUC   RxStaCnt1;
	RX_STA_CNT2_STRUC   RxStaCnt2;
	TX_STA_CNT0_STRUC 	 TxStaCnt0;
	TX_STA_CNT1_STRUC	 StaTx1;
	TX_STA_CNT2_STRUC	 StaTx2;
#ifdef STATS_COUNT_SUPPORT
	TX_NAG_AGG_CNT_STRUC	TxAggCnt;
	TX_AGG_CNT0_STRUC	TxAggCnt0;
	TX_AGG_CNT1_STRUC	TxAggCnt1;
	TX_AGG_CNT2_STRUC	TxAggCnt2;
	TX_AGG_CNT3_STRUC	TxAggCnt3;
	TX_AGG_CNT4_STRUC	TxAggCnt4;
	TX_AGG_CNT5_STRUC	TxAggCnt5;
	TX_AGG_CNT6_STRUC	TxAggCnt6;
	TX_AGG_CNT7_STRUC	TxAggCnt7;
#endif /* STATS_COUNT_SUPPORT */
	COUNTER_RALINK		*pRalinkCounters;


	pRalinkCounters = &pAd->RalinkCounters;



	RTMP_IO_READ32(pAd, RX_STA_CNT0, &RxStaCnt0.word);
	RTMP_IO_READ32(pAd, RX_STA_CNT2, &RxStaCnt2.word);

	pAd->RalinkCounters.PhyErrCnt += RxStaCnt0.field.PhyErr;

#ifdef CUSTOMER_DCC_FEATURE
	if(!pAd->EnableChannelStatsCheck && !(ApScanRunning(pAd)))
#endif
	{
		RTMP_IO_READ32(pAd, RX_STA_CNT1, &RxStaCnt1.word);
		/* Update RX PLCP error counter*/
		pAd->RalinkCounters.PlcpErrCnt += RxStaCnt1.field.PlcpErr;
		/* Update False CCA counter*/
		pAd->RalinkCounters.OneSecFalseCCACnt += RxStaCnt1.field.FalseCca;
#ifdef ED_MONITOR
		if (pAd->ed_chk /*&& pAd->ed_timer_inited == TRUE*/) //no timer now, and the data may not correct before.
			pAd->false_cca_stat[pAd->ed_stat_lidx] += RxStaCnt1.field.FalseCca;
#endif /* ED_MONITOR */
		pAd->RalinkCounters.FalseCCACnt += RxStaCnt1.field.FalseCca;
#ifdef SMART_MESH

		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))
		{
			/*Still scanning*/
			pAd->ChannelList[pAd->ApCfg.scan_channel_index].FalseCCA += RxStaCnt1.field.FalseCca;
		}
#endif /* SMART_MESH */		
	}

#ifdef STATS_COUNT_SUPPORT
	/* Update FCS counters*/
	OldValue= pAd->WlanCounters.FCSErrorCount.u.LowPart;
	pAd->WlanCounters.FCSErrorCount.u.LowPart += (RxStaCnt0.field.CrcErr); /* >> 7);*/
	if (pAd->WlanCounters.FCSErrorCount.u.LowPart < OldValue)
		pAd->WlanCounters.FCSErrorCount.u.HighPart++;
#endif /* STATS_COUNT_SUPPORT */

	/* Add FCS error count to private counters*/
	pRalinkCounters->OneSecRxFcsErrCnt += RxStaCnt0.field.CrcErr;
	OldValue = pRalinkCounters->RealFcsErrCount.u.LowPart;
	pRalinkCounters->RealFcsErrCount.u.LowPart += RxStaCnt0.field.CrcErr;
	if (pRalinkCounters->RealFcsErrCount.u.LowPart < OldValue)
		pRalinkCounters->RealFcsErrCount.u.HighPart++;

	/* Update Duplicate Rcv check*/
	pRalinkCounters->DuplicateRcv += RxStaCnt2.field.RxDupliCount;
#ifdef STATS_COUNT_SUPPORT
	pAd->WlanCounters.FrameDuplicateCount.u.LowPart += RxStaCnt2.field.RxDupliCount;
#endif /* STATS_COUNT_SUPPORT */
	/* Update RX Overflow counter*/
	pAd->Counters8023.RxNoBuffer += (RxStaCnt2.field.RxFifoOverflowCount);

	/*pAd->RalinkCounters.RxCount = 0;*/


	/*if (!OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_TX_RATE_SWITCH_ENABLED) || */
	/*	(OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_TX_RATE_SWITCH_ENABLED) && (pAd->MacTab.Size != 1)))*/
	if (!pAd->bUpdateBcnCntDone)
	{
		/* Update BEACON sent count*/
		NicGetTxRawCounters(pAd, &TxStaCnt0, &StaTx1);
		RTMP_IO_READ32(pAd, TX_STA_CNT2, &StaTx2.word);
#ifdef CUSTOMER_DCC_FEATURE
		pAd->RadioStatsCounter.TotalBeaconSentCount += TxStaCnt0.field.TxBeaconCount;
		pAd->RadioStatsCounter.TotalTxCount += (StaTx1.field.TxSuccess + TxStaCnt0.field.TxFailCount + StaTx1.field.TxRetransmit + TxStaCnt0.field.TxBeaconCount);
		pAd->RadioStatsCounter.TxRetryCount += StaTx1.field.TxRetransmit;
#endif
	}


	/*if (pAd->bStaFifoTest == TRUE)*/
#ifdef STATS_COUNT_SUPPORT
	{
		RTMP_IO_READ32(pAd, TX_AGG_CNT, &TxAggCnt.word);
	RTMP_IO_READ32(pAd, TX_AGG_CNT0, &TxAggCnt0.word);
	RTMP_IO_READ32(pAd, TX_AGG_CNT1, &TxAggCnt1.word);
	RTMP_IO_READ32(pAd, TX_AGG_CNT2, &TxAggCnt2.word);
	RTMP_IO_READ32(pAd, TX_AGG_CNT3, &TxAggCnt3.word);
		RTMP_IO_READ32(pAd, TX_AGG_CNT4, &TxAggCnt4.word);
		RTMP_IO_READ32(pAd, TX_AGG_CNT5, &TxAggCnt5.word);
		RTMP_IO_READ32(pAd, TX_AGG_CNT6, &TxAggCnt6.word);
		RTMP_IO_READ32(pAd, TX_AGG_CNT7, &TxAggCnt7.word);
		pRalinkCounters->TxAggCount += TxAggCnt.field.AggTxCount;
		pRalinkCounters->TxNonAggCount += TxAggCnt.field.NonAggTxCount;
		pRalinkCounters->TxAgg1MPDUCount += TxAggCnt0.field.AggSize1Count;
		pRalinkCounters->TxAgg2MPDUCount += TxAggCnt0.field.AggSize2Count;

		pRalinkCounters->TxAgg3MPDUCount += TxAggCnt1.field.AggSize3Count;
		pRalinkCounters->TxAgg4MPDUCount += TxAggCnt1.field.AggSize4Count;
		pRalinkCounters->TxAgg5MPDUCount += TxAggCnt2.field.AggSize5Count;
		pRalinkCounters->TxAgg6MPDUCount += TxAggCnt2.field.AggSize6Count;

		pRalinkCounters->TxAgg7MPDUCount += TxAggCnt3.field.AggSize7Count;
		pRalinkCounters->TxAgg8MPDUCount += TxAggCnt3.field.AggSize8Count;
		pRalinkCounters->TxAgg9MPDUCount += TxAggCnt4.field.AggSize9Count;
		pRalinkCounters->TxAgg10MPDUCount += TxAggCnt4.field.AggSize10Count;

		pRalinkCounters->TxAgg11MPDUCount += TxAggCnt5.field.AggSize11Count;
		pRalinkCounters->TxAgg12MPDUCount += TxAggCnt5.field.AggSize12Count;
		pRalinkCounters->TxAgg13MPDUCount += TxAggCnt6.field.AggSize13Count;
		pRalinkCounters->TxAgg14MPDUCount += TxAggCnt6.field.AggSize14Count;

		pRalinkCounters->TxAgg15MPDUCount += TxAggCnt7.field.AggSize15Count;
		pRalinkCounters->TxAgg16MPDUCount += TxAggCnt7.field.AggSize16Count;

		/* Calculate the transmitted A-MPDU count*/
		pRalinkCounters->TransmittedAMPDUCount.u.LowPart += TxAggCnt0.field.AggSize1Count;
		pRalinkCounters->TransmittedAMPDUCount.u.LowPart += (TxAggCnt0.field.AggSize2Count >> 1);

		pRalinkCounters->TransmittedAMPDUCount.u.LowPart += (TxAggCnt1.field.AggSize3Count / 3);
		pRalinkCounters->TransmittedAMPDUCount.u.LowPart += (TxAggCnt1.field.AggSize4Count >> 2);

		pRalinkCounters->TransmittedAMPDUCount.u.LowPart += (TxAggCnt2.field.AggSize5Count / 5);
		pRalinkCounters->TransmittedAMPDUCount.u.LowPart += (TxAggCnt2.field.AggSize6Count / 6);

		pRalinkCounters->TransmittedAMPDUCount.u.LowPart += (TxAggCnt3.field.AggSize7Count / 7);
		pRalinkCounters->TransmittedAMPDUCount.u.LowPart += (TxAggCnt3.field.AggSize8Count >> 3);

		pRalinkCounters->TransmittedAMPDUCount.u.LowPart += (TxAggCnt4.field.AggSize9Count / 9);
		pRalinkCounters->TransmittedAMPDUCount.u.LowPart += (TxAggCnt4.field.AggSize10Count / 10);

		pRalinkCounters->TransmittedAMPDUCount.u.LowPart += (TxAggCnt5.field.AggSize11Count / 11);
		pRalinkCounters->TransmittedAMPDUCount.u.LowPart += (TxAggCnt5.field.AggSize12Count / 12);

		pRalinkCounters->TransmittedAMPDUCount.u.LowPart += (TxAggCnt6.field.AggSize13Count / 13);
		pRalinkCounters->TransmittedAMPDUCount.u.LowPart += (TxAggCnt6.field.AggSize14Count / 14);

		pRalinkCounters->TransmittedAMPDUCount.u.LowPart += (TxAggCnt7.field.AggSize15Count / 15);
		pRalinkCounters->TransmittedAMPDUCount.u.LowPart += (TxAggCnt7.field.AggSize16Count >> 4);
	}
#endif /* STATS_COUNT_SUPPORT */

#ifdef DBG_DIAGNOSE
	{
		RtmpDiagStruct *pDiag;
		UCHAR ArrayCurIdx, i;
		struct dbg_diag_info *diag_info;

		pDiag = &pAd->DiagStruct;
		ArrayCurIdx = pDiag->ArrayCurIdx;

		if (pDiag->inited == 0)
		{
			NdisZeroMemory(pDiag, sizeof(struct _RtmpDiagStrcut_));
			pDiag->ArrayStartIdx = pDiag->ArrayCurIdx = 0;
			pDiag->inited = 1;
		}
		else
		{
			diag_info = &pDiag->diag_info[ArrayCurIdx];

			/* Tx*/
			diag_info->TxFailCnt = TxStaCnt0.field.TxFailCount;
#ifdef DBG_TX_AGG_CNT
			diag_info->TxAggCnt = TxAggCnt.field.AggTxCount;
			diag_info->TxNonAggCnt = TxAggCnt.field.NonAggTxCount;

			diag_info->TxAMPDUCnt[0] = TxAggCnt0.field.AggSize1Count;
			diag_info->TxAMPDUCnt[1] = TxAggCnt0.field.AggSize2Count;
			diag_info->TxAMPDUCnt[2] = TxAggCnt1.field.AggSize3Count;
			diag_info->TxAMPDUCnt[3] = TxAggCnt1.field.AggSize4Count;
			diag_info->TxAMPDUCnt[4] = TxAggCnt2.field.AggSize5Count;
			diag_info->TxAMPDUCnt[5] = TxAggCnt2.field.AggSize6Count;
			diag_info->TxAMPDUCnt[6] = TxAggCnt3.field.AggSize7Count;
			diag_info->TxAMPDUCnt[7] = TxAggCnt3.field.AggSize8Count;
			diag_info->TxAMPDUCnt[8] = TxAggCnt4.field.AggSize9Count;
			diag_info->TxAMPDUCnt[9] = TxAggCnt4.field.AggSize10Count;
			diag_info->TxAMPDUCnt[10] = TxAggCnt5.field.AggSize11Count;
			diag_info->TxAMPDUCnt[11] = TxAggCnt5.field.AggSize12Count;
			diag_info->TxAMPDUCnt[12] = TxAggCnt6.field.AggSize13Count;
			diag_info->TxAMPDUCnt[13] = TxAggCnt6.field.AggSize14Count;
			diag_info->TxAMPDUCnt[14] = TxAggCnt7.field.AggSize15Count;
			diag_info->TxAMPDUCnt[15] = TxAggCnt7.field.AggSize16Count;
#endif /* DBG_TX_AGG_CNT */

			diag_info->RxCrcErrCnt = RxStaCnt0.field.CrcErr;

			INC_RING_INDEX(pDiag->ArrayCurIdx,  DIAGNOSE_TIME);
			ArrayCurIdx = pDiag->ArrayCurIdx;

			NdisZeroMemory(&pDiag->diag_info[ArrayCurIdx], sizeof(pDiag->diag_info[ArrayCurIdx]));

			if (pDiag->ArrayCurIdx == pDiag->ArrayStartIdx)
				INC_RING_INDEX(pDiag->ArrayStartIdx,  DIAGNOSE_TIME);
		}
	}
#endif /* DBG_DIAGNOSE */


}


int load_patch(RTMP_ADAPTER *ad)
{
	int ret = NDIS_STATUS_SUCCESS;
	ULONG Old, New, Diff;

	if (ad->chipOps.load_rom_patch) {
		RTMP_GetCurrentSystemTick(&Old);
		ret = ad->chipOps.load_rom_patch(ad);
		RTMP_GetCurrentSystemTick(&New);
		Diff = (New - Old) * 1000 / OS_HZ;
		DBGPRINT(RT_DEBUG_TRACE, ("load rom patch spent %ldms\n", Diff));
	}

	return ret;
}

VOID erase_patch(RTMP_ADAPTER *ad)
{
	if (ad->chipOps.erase_rom_patch)
		ad->chipOps.erase_rom_patch(ad);
}

int NICLoadFirmware(RTMP_ADAPTER *ad)
{
	int ret	= NDIS_STATUS_SUCCESS;
	ULONG Old, New, Diff;

	if (ad->chipOps.loadFirmware) {
		RTMP_GetCurrentSystemTick(&Old);
		ret = ad->chipOps.loadFirmware(ad);
		RTMP_GetCurrentSystemTick(&New);
		Diff = (New - Old) * 1000 / OS_HZ;
		DBGPRINT(RT_DEBUG_TRACE, ("load fw spent %ldms\n", Diff));
	}

	return ret;
}


VOID NICEraseFirmware(RTMP_ADAPTER *pAd)
{
	if (pAd->chipOps.eraseFirmware)
		pAd->chipOps.eraseFirmware(pAd);
}


/*
	========================================================================

	Routine Description:
		Compare two memory block

	Arguments:
		pSrc1		Pointer to first memory address
		pSrc2		Pointer to second memory address

	Return Value:
		0:			memory is equal
		1:			pSrc1 memory is larger
		2:			pSrc2 memory is larger

	IRQL = DISPATCH_LEVEL

	Note:

	========================================================================
*/
ULONG RTMPCompareMemory(VOID *pSrc1, VOID *pSrc2, ULONG Length)
{
	PUCHAR	pMem1;
	PUCHAR	pMem2;
	ULONG	Index = 0;

	pMem1 = (PUCHAR) pSrc1;
	pMem2 = (PUCHAR) pSrc2;

	for (Index = 0; Index < Length; Index++)
	{
		if (pMem1[Index] > pMem2[Index])
			return (1);
		else if (pMem1[Index] < pMem2[Index])
			return (2);
	}

	/* Equal*/
	return (0);
}


/*
	========================================================================

	Routine Description:
		Zero out memory block

	Arguments:
		pSrc1		Pointer to memory address
		Length		Size

	Return Value:
		None

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL

	Note:

	========================================================================
*/
VOID RTMPZeroMemory(VOID *pSrc, ULONG Length)
{
	PUCHAR	pMem;
	ULONG	Index = 0;

	pMem = (PUCHAR) pSrc;

	for (Index = 0; Index < Length; Index++)
	{
		pMem[Index] = 0x00;
	}
}


/*
	========================================================================

	Routine Description:
		Copy data from memory block 1 to memory block 2

	Arguments:
		pDest		Pointer to destination memory address
		pSrc		Pointer to source memory address
		Length		Copy size

	Return Value:
		None

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL

	Note:

	========================================================================
*/
VOID RTMPMoveMemory(VOID *pDest, VOID *pSrc, ULONG Length)
{
	PUCHAR	pMem1;
	PUCHAR	pMem2;
	UINT	Index;

	ASSERT((Length==0) || (pDest && pSrc));

	pMem1 = (PUCHAR) pDest;
	pMem2 = (PUCHAR) pSrc;

	for (Index = 0; Index < Length; Index++)
	{
		pMem1[Index] = pMem2[Index];
	}
}


VOID UserCfgExit(RTMP_ADAPTER *pAd)
{
#ifdef RT_CFG80211_SUPPORT
	/* Reset the CFG80211 Internal Flag */
	RTMP_DRIVER_80211_RESET(pAd);
#endif /* RT_CFG80211_SUPPORT */

#ifdef DOT11_N_SUPPORT
	BATableExit(pAd);
#endif /* DOT11_N_SUPPORT */


	NdisFreeSpinLock(&pAd->MacTabLock);

#ifdef MAC_REPEATER_SUPPORT
	NdisFreeSpinLock(&pAd->ApCfg.ReptCliEntryLock);
#endif /* MAC_REPEATER_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
#ifdef BAND_STEERING
		BndStrg_Release(pAd);
#endif /* BAND_STEERING */
	}
#endif /* CONFIG_AP_SUPPORT */
}


/*
	========================================================================

	Routine Description:
		Initialize port configuration structure

	Arguments:
		Adapter						Pointer to our adapter

	Return Value:
		None

	IRQL = PASSIVE_LEVEL

	Note:

	========================================================================
*/
VOID UserCfgInit(RTMP_ADAPTER *pAd)
{
	UINT i;
#ifdef CONFIG_AP_SUPPORT
	UINT j;
#endif /* CONFIG_AP_SUPPORT */
	UINT key_index, bss_index;
	UINT8 TXWISize = pAd->chipCap.TXWISize;

	DBGPRINT(RT_DEBUG_TRACE, ("--> UserCfgInit\n"));

	pAd->IndicateMediaState = NdisMediaStateDisconnected;

	/* part I. intialize common configuration */
	pAd->CommonCfg.BasicRateBitmap = 0xF;
	pAd->CommonCfg.BasicRateBitmapOld = 0xF;


	for(key_index=0; key_index<SHARE_KEY_NUM; key_index++)
	{
		for(bss_index = 0; bss_index < MAX_MBSSID_NUM(pAd) + MAX_P2P_NUM; bss_index++)
		{
			pAd->SharedKey[bss_index][key_index].KeyLen = 0;
			pAd->SharedKey[bss_index][key_index].CipherAlg = CIPHER_NONE;
		}
	}

	pAd->bLocalAdminMAC = FALSE;
	pAd->EepromAccess = FALSE;

	pAd->Antenna.word = 0;
	pAd->CommonCfg.BBPCurrentBW = BW_20;

#ifdef RTMP_MAC_PCI
#ifdef LED_CONTROL_SUPPORT
	pAd->LedCntl.LedIndicatorStrength = 0;
#endif /* LED_CONTROL_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	pAd->RLnkCtrlOffset = 0;
	pAd->HostLnkCtrlOffset = 0;
#endif /* CONFIG_STA_SUPPORT */

	pAd->int_cpu_analysis = 0;
	pAd->fake_int_counter = 0;
	pAd->rx_coherent_counter = 0;
	pAd->tx_coherent_counter = 0;
	pAd->fifo_int_counter = 0;
	pAd->hcca_int_counter = 0;
	pAd->tx_ac3_int_counter = 0;
	pAd->tx_ac2_int_counter = 0;
	pAd->tx_ac1_int_counter = 0;

	/* tx ac0 tasklet related */
	pAd->tx_tasklet_counter = 0;
	pAd->tx_int_counter = 0;
	pAd->tx_packet_counter = 0;

	/* rx tasklet related */
	pAd->rx_tasklet_counter = 0;
	pAd->rx_int_counter = 0;
	pAd->rx_packet_counter = 0;
	pAd->rx_tasklet_resche_counter = 0;
	pAd->rx_packet_1_counter = 0;
	pAd->rx_packet_2_counter = 0;
	pAd->rx_packet_3_counter = 0;
	pAd->rx_packet_4_counter = 0;
	pAd->rx_packet_5_counter = 0;
	pAd->rx_packet_6_10_counter = 0;
	pAd->rx_packet_11_15_counter = 0;
	pAd->rx_packet_16_20_counter = 0;
	pAd->rx_packet_21_25_counter = 0;
	pAd->rx_packet_26_31_counter = 0;
	pAd->rx_packet_32_max_counter = 0;
#endif /* RTMP_MAC_PCI */

	pAd->bAutoTxAgcA = FALSE;			/* Default is OFF*/
	pAd->bAutoTxAgcG = FALSE;			/* Default is OFF*/

#ifdef SINGLE_SKU_V2
	pAd->sku_init_done = FALSE;
	pAd->tc_init_val = 0;
#endif /* SINGLE_SKU_V2 */

#if defined(RTMP_INTERNAL_TX_ALC) || defined(RTMP_TEMPERATURE_COMPENSATION)
	pAd->TxPowerCtrl.bInternalTxALC = FALSE; /* Off by default */
	pAd->TxPowerCtrl.idxTxPowerTable = 0;
	pAd->TxPowerCtrl.idxTxPowerTable2 = 0;
#ifdef RTMP_TEMPERATURE_COMPENSATION
	pAd->TxPowerCtrl.LookupTableIndex = 0;
#endif /* RTMP_TEMPERATURE_COMPENSATION */
#endif /* RTMP_INTERNAL_TX_ALC || RTMP_TEMPERATURE_COMPENSATION */

#ifdef THERMAL_PROTECT_SUPPORT
	pAd->force_one_tx_stream = FALSE;
	pAd->last_thermal_pro_temp = 0;
	pAd->thermal_pro_criteria = 80;
#endif /* THERMAL_PROTECT_SUPPORT */
#if defined(RTMP_INTERNAL_TX_ALC) || defined(RTMP_TEMPERATURE_COMPENSATION)
	pAd->TxPowerCtrl.bInternalTxALC = FALSE; /* Off by default */
	pAd->TxPowerCtrl.idxTxPowerTable = 0;
	pAd->TxPowerCtrl.idxTxPowerTable2 = 0;
#ifdef RTMP_TEMPERATURE_COMPENSATION
	pAd->TxPowerCtrl.LookupTableIndex = 0;
#endif /* RTMP_TEMPERATURE_COMPENSATION */
#endif /* RTMP_INTERNAL_TX_ALC || RTMP_TEMPERATURE_COMPENSATION */

	pAd->RfIcType = RFIC_2820;

	/* Init timer for reset complete event*/
	pAd->CommonCfg.CentralChannel = 1;
	pAd->bForcePrintTX = FALSE;
	pAd->bForcePrintRX = FALSE;
	pAd->bStaFifoTest = FALSE;
	pAd->bProtectionTest = FALSE;
	pAd->bHCCATest = FALSE;
	pAd->bGenOneHCCA = FALSE;
	pAd->CommonCfg.Dsifs = 10;      /* in units of usec */
	pAd->CommonCfg.TxPower = 100; /* mW*/
	pAd->CommonCfg.TxPowerPercentage = 0xffffffff; /* AUTO*/
	pAd->CommonCfg.TxPowerDefault = 0xffffffff; /* AUTO*/
	pAd->CommonCfg.TxPreamble = Rt802_11PreambleAuto; /* use Long preamble on TX by defaut*/
	pAd->CommonCfg.bUseZeroToDisableFragment = FALSE;
	pAd->CommonCfg.RtsThreshold = 2347;
	pAd->CommonCfg.FragmentThreshold = 2346;
	pAd->CommonCfg.UseBGProtection = 0;    /* 0: AUTO*/
	pAd->CommonCfg.bEnableTxBurst = TRUE; /* 0;    	*/
	pAd->CommonCfg.PhyMode = 0xff;     /* unknown*/
	pAd->CommonCfg.SavedPhyMode = pAd->CommonCfg.PhyMode;
	pAd->CommonCfg.BandState = UNKNOWN_BAND;

	pAd->wmm_cw_min = 4;
	switch (pAd->OpMode)
	{
		case OPMODE_AP:
			pAd->wmm_cw_max = 6;
			break;
		case OPMODE_STA:
			pAd->wmm_cw_max = 10;
			break;
	}


#ifdef CONFIG_AP_SUPPORT
#ifdef AP_SCAN_SUPPORT
	pAd->ApCfg.ACSCheckTime = 0;
#endif /* AP_SCAN_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

#ifdef CARRIER_DETECTION_SUPPORT
	pAd->CommonCfg.CarrierDetect.delta = CARRIER_DETECT_DELTA;
	pAd->CommonCfg.CarrierDetect.div_flag = CARRIER_DETECT_DIV_FLAG;
	pAd->CommonCfg.CarrierDetect.criteria = CARRIER_DETECT_CRITIRIA;
	pAd->CommonCfg.CarrierDetect.threshold = CARRIER_DETECT_THRESHOLD;
	pAd->CommonCfg.CarrierDetect.recheck1 = CARRIER_DETECT_RECHECK_TIME;
	pAd->CommonCfg.CarrierDetect.CarrierGoneThreshold = CARRIER_GONE_TRESHOLD;
	pAd->CommonCfg.CarrierDetect.VGA_Mask = CARRIER_DETECT_DEFAULT_MASK;
	pAd->CommonCfg.CarrierDetect.Packet_End_Mask = CARRIER_DETECT_DEFAULT_MASK;
	pAd->CommonCfg.CarrierDetect.Rx_PE_Mask = CARRIER_DETECT_DEFAULT_MASK;
#endif /* CARRIER_DETECTION_SUPPORT */

#ifdef DFS_SUPPORT
	pAd->CommonCfg.RadarDetect.bDfsInit = FALSE;
#endif /* DFS_SUPPORT */


#ifdef UAPSD_SUPPORT
#ifdef CONFIG_AP_SUPPORT
{
	UINT32 IdMbss;

	for(IdMbss=0; IdMbss<HW_BEACON_MAX_NUM; IdMbss++)
		UAPSD_INFO_INIT(&pAd->ApCfg.MBSSID[IdMbss].UapsdInfo);
}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	pAd->StaCfg.UapsdInfo.bAPSDCapable = FALSE;
#endif /* CONFIG_STA_SUPPORT */
#endif /* UAPSD_SUPPORT */
	pAd->CommonCfg.bNeedSendTriggerFrame = FALSE;
	pAd->CommonCfg.TriggerTimerCount = 0;
	pAd->CommonCfg.bAPSDForcePowerSave = FALSE;
	/*pAd->CommonCfg.bCountryFlag = FALSE;*/
	pAd->CommonCfg.TxStream = 0;
	pAd->CommonCfg.RxStream = 0;

	NdisZeroMemory(&pAd->BeaconTxWI, TXWISize);

#ifdef DOT11_VHT_AC
	if (IS_MT76x2(pAd))
		pAd->CommonCfg.b256QAM_2G = TRUE;
	else
		pAd->CommonCfg.b256QAM_2G = FALSE;
#endif /* DOT11_VHT_AC */

#ifdef DOT11_N_SUPPORT
	NdisZeroMemory(&pAd->CommonCfg.HtCapability, sizeof(pAd->CommonCfg.HtCapability));
	pAd->HTCEnable = FALSE;
	pAd->bBroadComHT = FALSE;
	pAd->CommonCfg.bRdg = FALSE;

#ifdef DOT11N_DRAFT3
	pAd->CommonCfg.Dot11OBssScanPassiveDwell = dot11OBSSScanPassiveDwell;	/* Unit : TU. 5~1000*/
	pAd->CommonCfg.Dot11OBssScanActiveDwell = dot11OBSSScanActiveDwell;	/* Unit : TU. 10~1000*/
	pAd->CommonCfg.Dot11BssWidthTriggerScanInt = dot11BSSWidthTriggerScanInterval;	/* Unit : Second	*/
	pAd->CommonCfg.Dot11OBssScanPassiveTotalPerChannel = dot11OBSSScanPassiveTotalPerChannel;	/* Unit : TU. 200~10000*/
	pAd->CommonCfg.Dot11OBssScanActiveTotalPerChannel = dot11OBSSScanActiveTotalPerChannel;	/* Unit : TU. 20~10000*/
	pAd->CommonCfg.Dot11BssWidthChanTranDelayFactor = dot11BSSWidthChannelTransactionDelayFactor;
	pAd->CommonCfg.Dot11OBssScanActivityThre = dot11BSSScanActivityThreshold;	/* Unit : percentage*/
	pAd->CommonCfg.Dot11BssWidthChanTranDelay = (pAd->CommonCfg.Dot11BssWidthTriggerScanInt * pAd->CommonCfg.Dot11BssWidthChanTranDelayFactor);

	pAd->CommonCfg.bBssCoexEnable = TRUE; /* by default, we enable this feature, you can disable it via the profile or ioctl command*/
	pAd->CommonCfg.BssCoexApCntThr = 0;
	pAd->CommonCfg.Bss2040NeedFallBack = 0;
#endif  /* DOT11N_DRAFT3 */

	pAd->CommonCfg.bRcvBSSWidthTriggerEvents = FALSE;

	NdisZeroMemory(&pAd->CommonCfg.AddHTInfo, sizeof(pAd->CommonCfg.AddHTInfo));
	pAd->CommonCfg.BACapability.field.MMPSmode = MMPS_DISABLE;
	pAd->CommonCfg.BACapability.field.MpduDensity = 0;
	pAd->CommonCfg.BACapability.field.Policy = IMMED_BA;
	pAd->CommonCfg.BACapability.field.RxBAWinLimit = 64; /*32;*/
	pAd->CommonCfg.BACapability.field.TxBAWinLimit = 64; /*32;*/
	DBGPRINT(RT_DEBUG_TRACE, ("--> UserCfgInit. BACapability = 0x%x\n", pAd->CommonCfg.BACapability.word));

	pAd->CommonCfg.BACapability.field.AutoBA = FALSE;
	BATableInit(pAd, &pAd->BATable);

	pAd->CommonCfg.bExtChannelSwitchAnnouncement = 1;
	pAd->CommonCfg.bHTProtect = 1;
	pAd->CommonCfg.bMIMOPSEnable = TRUE;
#ifdef GREENAP_SUPPORT
	pAd->ApCfg.bGreenAPEnable=FALSE;
	pAd->ApCfg.bGreenAPActive = FALSE;
	pAd->ApCfg.GreenAPLevel= GREENAP_WITHOUT_ANY_STAS_CONNECT;
#endif /* GREENAP_SUPPORT */
	pAd->CommonCfg.bBADecline = FALSE;
	pAd->CommonCfg.bDisableReordering = FALSE;

	if (pAd->MACVersion == 0x28720200)
		pAd->CommonCfg.TxBASize = 13; /*by Jerry recommend*/
	else
		pAd->CommonCfg.TxBASize = 7;

	pAd->CommonCfg.REGBACapability.word = pAd->CommonCfg.BACapability.word;
#endif /* DOT11_N_SUPPORT */

	pAd->CommonCfg.TxRate = RATE_6;

	pAd->CommonCfg.MlmeTransmit.field.MCS = MCS_RATE_6;
	pAd->CommonCfg.MlmeTransmit.field.BW = BW_20;
	pAd->CommonCfg.MlmeTransmit.field.MODE = MODE_OFDM;

	pAd->CommonCfg.BeaconPeriod = 100;     /* in mSec*/

#ifdef DELAYED_TCP_ACK
	pAd->CommonCfg.ACKQEN = 0;
	pAd->CommonCfg.AckWaitTime = 40;
	pAd->CommonCfg.Acklen = 16;
#endif /* DELAYED_TCP_ACK */

#ifdef STREAM_MODE_SUPPORT
	if (pAd->chipCap.FlgHwStreamMode)
	{
		pAd->CommonCfg.StreamMode = 3;
		pAd->CommonCfg.StreamModeMCS = 0x0B0B;
		NdisMoveMemory(&pAd->CommonCfg.StreamModeMac[0][0],
				BROADCAST_ADDR, MAC_ADDR_LEN);
	}
#endif /* STREAM_MODE_SUPPORT */

#ifdef TXBF_SUPPORT
	pAd->CommonCfg.ETxBfNoncompress = 0;
	pAd->CommonCfg.ETxBfIncapable = 0;
#endif /* TXBF_SUPPORT */

#ifdef NEW_RATE_ADAPT_SUPPORT
	pAd->CommonCfg.lowTrafficThrd = 2;
	pAd->CommonCfg.TrainUpRule = 2; // 1;
	pAd->CommonCfg.TrainUpRuleRSSI = -70; // 0;
	pAd->CommonCfg.TrainUpLowThrd = 90;
	pAd->CommonCfg.TrainUpHighThrd = 110;
#endif /* NEW_RATE_ADAPT_SUPPORT */

#ifdef PS_ENTRY_MAITENANCE
	pAd->ps_timeout = 32;
#endif /* PS_ENTRY_MAITENANCE */



#ifdef CFO_TRACK
#endif /* CFO_TRACK */

#ifdef DBG_CTRL_SUPPORT
	pAd->CommonCfg.DebugFlags = 0;
#endif /* DBG_CTRL_SUPPORT */

#ifdef WAPI_SUPPORT
	pAd->CommonCfg.wapi_usk_rekey_method = REKEY_METHOD_DISABLE;
	pAd->CommonCfg.wapi_msk_rekey_method = REKEY_METHOD_DISABLE;
	pAd->CommonCfg.wapi_msk_rekey_cnt = 0;
#endif /* WAPI_SUPPORT */

#ifdef MCAST_RATE_SPECIFIC
	pAd->CommonCfg.MCastPhyMode.word = pAd->MacTab.Content[MCAST_WCID].HTPhyMode.word;
#endif /* MCAST_RATE_SPECIFIC */

	/* WFA policy - disallow TH rate in WEP or TKIP cipher */
	pAd->CommonCfg.HT_DisallowTKIP = TRUE;

	/* Frequency for rate adaptation */
	pAd->ra_interval = DEF_RA_TIME_INTRVAL;
	pAd->ra_fast_interval = DEF_QUICK_RA_TIME_INTERVAL;

#ifdef AGS_SUPPORT
	if (pAd->rateAlg == RATE_ALG_AGS)
		pAd->ra_fast_interval = AGS_QUICK_RA_TIME_INTERVAL;
#endif /* AGS_SUPPORT */

	/* Tx Sw queue length setting */
	pAd->TxSwQMaxLen = MAX_PACKETS_IN_QUEUE;

#ifdef DATA_QUEUE_RESERVE
	pAd->TxRsvLen = FIFO_RSV_FOR_HIGH_PRIORITY;
#endif /* DATA_QUEUE_RESERVE */

	pAd->CommonCfg.bRalinkBurstMode = FALSE;

#ifdef CONFIG_STA_SUPPORT
	/* part II. intialize STA specific configuration*/
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		RX_FILTER_SET_FLAG(pAd, fRX_FILTER_ACCEPT_DIRECT);
		RX_FILTER_CLEAR_FLAG(pAd, fRX_FILTER_ACCEPT_MULTICAST);
		RX_FILTER_SET_FLAG(pAd, fRX_FILTER_ACCEPT_BROADCAST);
		RX_FILTER_SET_FLAG(pAd, fRX_FILTER_ACCEPT_ALL_MULTICAST);

		pAd->StaCfg.Psm = PWR_ACTIVE;

		pAd->StaCfg.PairCipher = Ndis802_11EncryptionDisabled;
		pAd->StaCfg.GroupCipher = Ndis802_11EncryptionDisabled;
		pAd->StaCfg.bMixCipher = FALSE;
		pAd->StaCfg.wdev.DefaultKeyId = 0;

		/* 802.1x port control*/
		pAd->StaCfg.PrivacyFilter = Ndis802_11PrivFilter8021xWEP;
		pAd->StaCfg.wdev.PortSecured = WPA_802_1X_PORT_NOT_SECURED;
		pAd->StaCfg.LastMicErrorTime = 0;
		pAd->StaCfg.MicErrCnt        = 0;
		pAd->StaCfg.bBlockAssoc      = FALSE;
		pAd->StaCfg.WpaState         = SS_NOTUSE;

		pAd->CommonCfg.NdisRadioStateOff = FALSE;		/* New to support microsoft disable radio with OID command*/

		pAd->StaCfg.RssiTrigger = 0;
		NdisZeroMemory(&pAd->StaCfg.RssiSample, sizeof(RSSI_SAMPLE));
		pAd->StaCfg.RssiTriggerMode = RSSI_TRIGGERED_UPON_BELOW_THRESHOLD;
		pAd->StaCfg.AtimWin = 0;
		pAd->StaCfg.DefaultListenCount = 3;/*default listen count;*/
#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
		pAd->StaCfg.DefaultListenCount = 1;
#endif /* defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) */
		pAd->StaCfg.BssType = BSS_INFRA;  /* BSS_INFRA or BSS_ADHOC or BSS_MONITOR*/
		pAd->StaCfg.bSkipAutoScanConn = FALSE;
		OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_DOZE);
		OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_WAKEUP_NOW);

		pAd->StaCfg.wdev.bAutoTxRateSwitch = TRUE;
		pAd->StaCfg.wdev.DesiredTransmitSetting.field.MCS = MCS_AUTO;
		pAd->StaCfg.bAutoConnectIfNoSSID = FALSE;
	}

#ifdef EXT_BUILD_CHANNEL_LIST
	pAd->StaCfg.IEEE80211dClientMode = Rt802_11_D_None;
#endif /* EXT_BUILD_CHANNEL_LIST */

#ifdef RTMP_MAC_PCI
	pAd->brt30xxBanMcuCmd = FALSE;
	pAd->StaCfg.PSControl.field.EnableNewPS=FALSE;

#ifdef PCIE_PS_SUPPORT
	pAd->StaCfg.PSControl.field.EnableNewPS=TRUE;
	pAd->b3090ESpecialChip = FALSE;
	/*The value of PowerMode could be 1 or 3. Level 3 could save more power than Level 1. */
	pAd->StaCfg.PSControl.field.rt30xxPowerMode=3;
	pAd->StaCfg.PSControl.field.rt30xxForceASPMTest=0;
	pAd->StaCfg.PSControl.field.rt30xxFollowHostASPM=1;

	if (IS_SUPPORT_PCIE_PS_L3(pAd))
	{
		pAd->chipCap.HW_PCIE_PS_L3_ENABLE=TRUE;
		DBGPRINT(RT_DEBUG_TRACE, ("Support PCIe PS3 \n"));
	}
#endif /* PCIE_PS_SUPPORT */
#endif /* RTMP_MAC_PCI */
#endif /* CONFIG_STA_SUPPORT */

	/* global variables mXXXX used in MAC protocol state machines*/
	OPSTATUS_SET_FLAG(pAd, fOP_STATUS_RECEIVE_DTIM);
	OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_ADHOC_ON);
	OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_INFRA_ON);

	/* PHY specification*/
	pAd->CommonCfg.PhyMode = (WMODE_B | WMODE_G);		/* default PHY mode*/
	OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED);  /* CCK use LONG preamble*/

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		/* user desired power mode*/
		pAd->StaCfg.WindowsPowerMode = Ndis802_11PowerModeCAM;
		pAd->StaCfg.WindowsBatteryPowerMode = Ndis802_11PowerModeCAM;
		pAd->StaCfg.bWindowsACCAMEnable = FALSE;

		pAd->StaCfg.bHwRadio = TRUE; /* Default Hardware Radio status is On*/
		pAd->StaCfg.bSwRadio = TRUE; /* Default Software Radio status is On*/
		pAd->StaCfg.bRadio = TRUE; /* bHwRadio && bSwRadio*/
		pAd->StaCfg.bHardwareRadio = FALSE;		/* Default is OFF*/
		pAd->StaCfg.bShowHiddenSSID = FALSE;		/* Default no show*/

		/* Nitro mode control*/
#if defined(NATIVE_WPA_SUPPLICANT_SUPPORT) || defined(RT_CFG80211_SUPPORT)
		pAd->StaCfg.bAutoReconnect = FALSE;
#else
		pAd->StaCfg.bAutoReconnect = TRUE;
#endif /* NATIVE_WPA_SUPPLICANT_SUPPORT || RT_CFG80211_SUPPORT*/

		/* Save the init time as last scan time, the system should do scan after 2 seconds.*/
		/* This patch is for driver wake up from standby mode, system will do scan right away.*/
		NdisGetSystemUpTime(&pAd->StaCfg.LastScanTime);
		if (pAd->StaCfg.LastScanTime > 10 * OS_HZ)
			pAd->StaCfg.LastScanTime -= (10 * OS_HZ);

		NdisZeroMemory(pAd->nickname, IW_ESSID_MAX_SIZE+1);
#ifdef PROFILE_STORE
		pAd->bWriteDat = FALSE;
#endif /* PROFILE_STORE */
#ifdef WPA_SUPPLICANT_SUPPORT
		pAd->StaCfg.wdev.IEEE8021X = FALSE;
		pAd->StaCfg.wpa_supplicant_info.IEEE8021x_required_keys = FALSE;
		pAd->StaCfg.wpa_supplicant_info.WpaSupplicantUP = WPA_SUPPLICANT_DISABLE;
		pAd->StaCfg.wpa_supplicant_info.bRSN_IE_FromWpaSupplicant = FALSE;

#if defined(NATIVE_WPA_SUPPLICANT_SUPPORT) || defined(RT_CFG80211_SUPPORT)
		pAd->StaCfg.wpa_supplicant_info.WpaSupplicantUP = WPA_SUPPLICANT_ENABLE;
#ifdef PROFILE_STORE
		pAd->bWriteDat = TRUE;
#endif /* PROFILE_STORE */
#endif /* NATIVE_WPA_SUPPLICANT_SUPPORT || RT_CFG80211_SUPPORT */

		pAd->StaCfg.wpa_supplicant_info.bLostAp = FALSE;
		pAd->StaCfg.wpa_supplicant_info.pWpsProbeReqIe = NULL;
		pAd->StaCfg.wpa_supplicant_info.WpsProbeReqIeLen = 0;
		pAd->StaCfg.wpa_supplicant_info.pWpaAssocIe = NULL;
		pAd->StaCfg.wpa_supplicant_info.WpaAssocIeLen = 0;
		pAd->StaCfg.wpa_supplicant_info.WpaSupplicantScanCount = 0;
#endif /* WPA_SUPPLICANT_SUPPORT */


#ifdef WSC_STA_SUPPORT
		{
			INT					idx;
			PWSC_CTRL 			pWscControl;
#ifdef WSC_V2_SUPPORT
			PWSC_V2_INFO	pWscV2Info;
#endif /* WSC_V2_SUPPORT */

			/*
				WscControl cannot be zero here, because WscControl timers are initial in MLME Initialize
				and MLME Initialize is called before UserCfgInit.
			*/
			pWscControl = &pAd->StaCfg.WscControl;
			pWscControl->WscConfMode = WSC_DISABLE;
			pWscControl->WscMode = WSC_PIN_MODE;
			pWscControl->WscConfStatus = WSC_SCSTATE_UNCONFIGURED;
#ifdef WSC_V2_SUPPORT
			pWscControl->WscConfigMethods= 0x238C;
#else
			pWscControl->WscConfigMethods= 0x008C;
#endif /* WSC_V2_SUPPORT */
			pWscControl->WscState = WSC_STATE_OFF;
			pWscControl->WscStatus = STATUS_WSC_NOTUSED;
			pWscControl->WscPinCode = 0;
			pWscControl->WscLastPinFromEnrollee = 0;
			pWscControl->WscEnrollee4digitPinCode = FALSE;
			pWscControl->WscEnrolleePinCode = 0;
			pWscControl->WscSelReg = 0;
			NdisZeroMemory(&pAd->StaCfg.WscControl.RegData, sizeof(WSC_REG_DATA));
			NdisZeroMemory(&pWscControl->WscProfile, sizeof(WSC_PROFILE));
			pWscControl->WscUseUPnP = 0;
			pWscControl->WscEnAssociateIE = TRUE;
			pWscControl->WscEnProbeReqIE = TRUE;
			pWscControl->RegData.ReComputePke = 1;
			pWscControl->lastId = 1;
			pWscControl->EntryIfIdx = BSS0;
			pWscControl->pAd = pAd;
			pWscControl->WscDriverAutoConnect = 0x02;
			pAd->WriteWscCfgToDatFile = 0xFF;
			pWscControl->WscRejectSamePinFromEnrollee = FALSE;
			pWscControl->WpsApBand = PREFERRED_WPS_AP_PHY_TYPE_AUTO_SELECTION;
			pWscControl->bCheckMultiByte = FALSE;
			pWscControl->bWscAutoTigeer = FALSE;
			/* Enrollee Nonce, first generate and save to Wsc Control Block*/
			for (idx = 0; idx < 16; idx++)
			{
				pWscControl->RegData.SelfNonce[idx] = RandomByte(pAd);
			}
			pWscControl->WscRxBufLen = 0;
			pWscControl->pWscRxBuf = NULL;
			os_alloc_mem(pAd, &pWscControl->pWscRxBuf, MGMT_DMA_BUFFER_SIZE);
			if (pWscControl->pWscRxBuf)
				NdisZeroMemory(pWscControl->pWscRxBuf, MGMT_DMA_BUFFER_SIZE);
			pWscControl->WscTxBufLen = 0;
			pWscControl->pWscTxBuf = NULL;
			os_alloc_mem(pAd, &pWscControl->pWscTxBuf, MGMT_DMA_BUFFER_SIZE);
			if (pWscControl->pWscTxBuf)
				NdisZeroMemory(pWscControl->pWscTxBuf, MGMT_DMA_BUFFER_SIZE);
			pWscControl->bWscFragment = FALSE;
			pWscControl->WscFragSize = 128;
			initList(&pWscControl->WscPeerList);
			NdisAllocateSpinLock(pAd, &pWscControl->WscPeerListSemLock);

#ifdef WSC_V2_SUPPORT
			pWscV2Info = &pWscControl->WscV2Info;
			pWscV2Info->bWpsEnable = TRUE;
			pWscV2Info->ExtraTlv.TlvLen = 0;
			pWscV2Info->ExtraTlv.TlvTag = 0;
			pWscV2Info->ExtraTlv.pTlvData = NULL;
			pWscV2Info->ExtraTlv.TlvType = TLV_ASCII;
			pWscV2Info->bEnableWpsV2 = TRUE;
			pWscV2Info->bForceSetAP = FALSE;
#endif /* WSC_V2_SUPPORT */

		}
#ifdef IWSC_SUPPORT
		IWSC_Init(pAd);
#endif /* IWSC_SUPPORT */
#endif /* WSC_STA_SUPPORT */
		NdisZeroMemory(pAd->StaCfg.ReplayCounter, 8);

#ifdef DOT11R_FT_SUPPORT
		NdisZeroMemory(&pAd->StaCfg.Dot11RCommInfo, sizeof(DOT11R_CMN_STRUC));
#endif /* DOT11R_FT_SUPPORT */

		pAd->StaCfg.bAutoConnectByBssid = FALSE;
		pAd->StaCfg.BeaconLostTime = BEACON_LOST_TIME;
		NdisZeroMemory(pAd->StaCfg.WpaPassPhrase, 64);
		pAd->StaCfg.WpaPassPhraseLen = 0;
		pAd->StaCfg.bAutoRoaming = FALSE;
		pAd->StaCfg.bForceTxBurst = FALSE;
		pAd->StaCfg.bNotFirstScan = FALSE;
		pAd->StaCfg.bImprovedScan = FALSE;
#ifdef DOT11_N_SUPPORT
		pAd->StaCfg.bAdhocN = TRUE;
#endif /* DOT11_N_SUPPORT */
		pAd->StaCfg.bFastConnect = FALSE;
		pAd->StaCfg.bAdhocCreator = FALSE;
	}
#endif /* CONFIG_STA_SUPPORT */

	/* Default for extra information is not valid*/
	pAd->ExtraInfo = EXTRA_INFO_CLEAR;

	/* Default Config change flag*/
	pAd->bConfigChanged = FALSE;

	/*
		part III. AP configurations
	*/
#ifdef CONFIG_AP_SUPPORT
#if defined(P2P_APCLI_SUPPORT) || defined(RT_CFG80211_P2P_SUPPORT)
#else
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
#endif /* P2P_APCLI_SUPPORT || RT_CFG80211_P2P_SUPPORT */
	{
		/* Set MBSS Default Configurations*/
		pAd->ApCfg.BssidNum = MAX_MBSSID_NUM(pAd);
		for(j = BSS0; j < pAd->ApCfg.BssidNum; j++)
		{
			MULTISSID_STRUCT *mbss = &pAd->ApCfg.MBSSID[j];
			struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[j].wdev;

			mbss->AssocReqFailRssiThreshold = 0;
			mbss->AssocReqNoRspRssiThreshold = 0;
			mbss->AuthFailRssiThreshold = 0;
			mbss->AuthNoRspRssiThreshold = 0;
			mbss->RssiLowForStaKickOut = 0;
                        mbss->ProbeRspRssiThreshold = 0;

			mbss->StatTxRetryOkCount = 0;
			mbss->StatTxFailCount = 0;

			wdev->AuthMode = Ndis802_11AuthModeOpen;
			wdev->WepStatus = Ndis802_11EncryptionDisabled;
			wdev->GroupKeyWepStatus = Ndis802_11EncryptionDisabled;
			wdev->DefaultKeyId = 0;
			wdev->WpaMixPairCipher = MIX_CIPHER_NOTUSE;
			mbss->RekeyCountDown = 0;	/* it's used for WPA rekey */

			mbss->ProbeRspTimes = 3;
#ifdef SPECIFIC_TX_POWER_SUPPORT
			if (IS_RT6352(pAd) || IS_MT76x2(pAd))
				mbss->TxPwrAdj = -1;
#endif /* SPECIFIC_TX_POWER_SUPPORT */

#ifdef DOT1X_SUPPORT
			mbss->wdev.IEEE8021X = FALSE;
			mbss->PreAuth = FALSE;

			/* PMK cache setting*/
			mbss->PMKCachePeriod = (10 * 60 * OS_HZ); /* unit : tick(default: 10 minute)*/
			NdisZeroMemory(&mbss->PMKIDCache, sizeof(NDIS_AP_802_11_PMKID));

			/* dot1x related per BSS */
			mbss->radius_srv_num = 0;
			mbss->NasIdLen = 0;
#endif /* DOT1X_SUPPORT */

			/* VLAN related */
			mbss->wdev.VLAN_VID = 0;

			/* Default MCS as AUTO*/
			wdev->bAutoTxRateSwitch = TRUE;
			wdev->DesiredTransmitSetting.field.MCS = MCS_AUTO;

			/* Default is zero. It means no limit.*/
			mbss->MaxStaNum = 0;
			mbss->StaCount = 0;

#ifdef WSC_AP_SUPPORT
			mbss->WscSecurityMode = 0xff;
			{
				PWSC_CTRL pWscControl;
				INT idx;
#ifdef WSC_V2_SUPPORT
				PWSC_V2_INFO	pWscV2Info;
#endif /* WSC_V2_SUPPORT */
				/*
					WscControl cannot be zero here, because WscControl timers are initial in MLME Initialize
					and MLME Initialize is called before UserCfgInit.
				*/
				pWscControl = &mbss->WscControl;
				NdisZeroMemory(&pWscControl->RegData, sizeof(WSC_REG_DATA));
				NdisZeroMemory(&pAd->CommonCfg.WscStaPbcProbeInfo, sizeof(WSC_STA_PBC_PROBE_INFO));
				pWscControl->WscMode = 1;
				pWscControl->WscConfStatus = 1;
#ifdef WSC_V2_SUPPORT
				pWscControl->WscConfigMethods= 0x238C;
#else
				pWscControl->WscConfigMethods= 0x0084;
#endif /* WSC_V2_SUPPORT */
#ifdef WSC_NFC_SUPPORT
				pWscControl->WscConfigMethods |= WPS_CONFIG_METHODS_ENT;
#endif /* WSC_NFC_SUPPORT */
				pWscControl->RegData.ReComputePke = 1;
				pWscControl->lastId = 1;
				/* pWscControl->EntryIfIdx = (MIN_NET_DEVICE_FOR_MBSSID | j); */
				pWscControl->pAd = pAd;
				pWscControl->WscRejectSamePinFromEnrollee = FALSE;
				pAd->CommonCfg.WscPBCOverlap = FALSE;
				pWscControl->WscConfMode = 0;
				pWscControl->WscStatus = 0;
				pWscControl->WscState = 0;
				pWscControl->WscPinCode = 0;
				pWscControl->WscLastPinFromEnrollee = 0;
				pWscControl->WscEnrollee4digitPinCode = FALSE;
				pWscControl->WscEnrolleePinCode = 0;
				pWscControl->WscSelReg = 0;
				pWscControl->WscUseUPnP = 0;
				pWscControl->bWCNTest = FALSE;
				pWscControl->WscKeyASCII = 0; /* default, 0 (64 Hex) */

				/*
					Enrollee 192 random bytes for DH key generation
				*/
				for (idx = 0; idx < 192; idx++)
					pWscControl->RegData.EnrolleeRandom[idx] = RandomByte(pAd);

				/* Enrollee Nonce, first generate and save to Wsc Control Block*/
				for (idx = 0; idx < 16; idx++)
					pWscControl->RegData.SelfNonce[idx] = RandomByte(pAd);

				NdisZeroMemory(&pWscControl->WscDefaultSsid, sizeof(NDIS_802_11_SSID));
				NdisZeroMemory(&pWscControl->Wsc_Uuid_Str[0], UUID_LEN_STR);
				NdisZeroMemory(&pWscControl->Wsc_Uuid_E[0], UUID_LEN_HEX);
				pWscControl->bCheckMultiByte = FALSE;
				pWscControl->bWscAutoTigeer = FALSE;
				pWscControl->bWscFragment = FALSE;
				pWscControl->WscFragSize = 128;
				pWscControl->WscRxBufLen = 0;
				pWscControl->pWscRxBuf = NULL;
				os_alloc_mem(pAd, &pWscControl->pWscRxBuf, MGMT_DMA_BUFFER_SIZE);
				if (pWscControl->pWscRxBuf)
					NdisZeroMemory(pWscControl->pWscRxBuf, MGMT_DMA_BUFFER_SIZE);
				pWscControl->WscTxBufLen = 0;
				pWscControl->pWscTxBuf = NULL;
				os_alloc_mem(pAd, &pWscControl->pWscTxBuf, MGMT_DMA_BUFFER_SIZE);
				if (pWscControl->pWscTxBuf)
					NdisZeroMemory(pWscControl->pWscTxBuf, MGMT_DMA_BUFFER_SIZE);
				initList(&pWscControl->WscPeerList);
				NdisAllocateSpinLock(pAd, &pWscControl->WscPeerListSemLock);
				pWscControl->PinAttackCount = 0;
				pWscControl->bSetupLock = FALSE;
				pWscControl->SetupLockTime = WSC_WPS_AP_SETUP_LOCK_TIME;
				pWscControl->MaxPinAttack = WSC_WPS_AP_MAX_PIN_ATTACK;
#ifdef WSC_V2_SUPPORT
				pWscV2Info = &pWscControl->WscV2Info;
				pWscV2Info->bWpsEnable = TRUE;
				pWscV2Info->ExtraTlv.TlvLen = 0;
				pWscV2Info->ExtraTlv.TlvTag = 0;
				pWscV2Info->ExtraTlv.pTlvData = NULL;
				pWscV2Info->ExtraTlv.TlvType = TLV_ASCII;
				pWscV2Info->bEnableWpsV2 = TRUE;
#ifdef WSC_NFC_SUPPORT
				pWscControl->NfcPasswdCaculate = 2;
#endif /* WSC_NFC_SUPPORT */

#endif /* WSC_V2_SUPPORT */
			}
#endif /* WSC_AP_SUPPORT */


			for(i = 0; i < WLAN_MAX_NUM_OF_TIM; i++)
	        		mbss->TimBitmaps[i] = 0;
		}
		pAd->ApCfg.DtimCount  = 0;
		pAd->ApCfg.DtimPeriod = DEFAULT_DTIM_PERIOD;

		pAd->ApCfg.ErpIeContent = 0;

		pAd->ApCfg.StaIdleTimeout = MAC_TABLE_AGEOUT_TIME;

#ifdef IDS_SUPPORT
		/* Default disable IDS threshold and reset all IDS counters*/
		pAd->ApCfg.IdsEnable = FALSE;
		pAd->ApCfg.AuthFloodThreshold = 0;
		pAd->ApCfg.AssocReqFloodThreshold = 0;
		pAd->ApCfg.ReassocReqFloodThreshold = 0;
		pAd->ApCfg.ProbeReqFloodThreshold = 0;
		pAd->ApCfg.DisassocFloodThreshold = 0;
		pAd->ApCfg.DeauthFloodThreshold = 0;
		pAd->ApCfg.EapReqFloodThreshold = 0;
		RTMPClearAllIdsCounter(pAd);
#endif /* IDS_SUPPORT */

#ifdef WDS_SUPPORT
		APWdsInitialize(pAd);
#endif /* WDS_SUPPORT*/

#ifdef WSC_INCLUDED
		pAd->WriteWscCfgToDatFile = 0xFF;
		pAd->WriteWscCfgToAr9DatFile = FALSE;
#ifdef CONFIG_AP_SUPPORT
#ifdef RTMP_RBUS_SUPPORT
		pAd->bWscDriverAutoUpdateCfg = FALSE;
#else
		pAd->bWscDriverAutoUpdateCfg = TRUE;
#endif /* RTMP_RBUS_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
#endif /* WSC_INCLUDED */

#ifdef APCLI_SUPPORT
		pAd->ApCfg.FlgApCliIsUapsdInfoUpdated = FALSE;
		pAd->ApCfg.ApCliNum = MAX_APCLI_NUM;
#ifdef APCLI_CERT_SUPPORT
		pAd->bApCliCertTest = FALSE;
#endif /* APCLI_CERT_SUPPORT */
		for(j = 0; j < MAX_APCLI_NUM; j++)
		{
			APCLI_STRUCT *apcli_entry = &pAd->ApCfg.ApCliTab[j];
			struct wifi_dev *wdev = &apcli_entry->wdev;

			wdev->AuthMode = Ndis802_11AuthModeOpen;
			wdev->WepStatus = Ndis802_11WEPDisabled;
			wdev->bAutoTxRateSwitch = TRUE;
			wdev->DesiredTransmitSetting.field.MCS = MCS_AUTO;
			apcli_entry->UapsdInfo.bAPSDCapable = FALSE;
			apcli_entry->bPeerExist = FALSE;
#ifdef APCLI_CONNECTION_TRIAL
			apcli_entry->TrialCh = 0;//if the channel is 0, AP will connect the rootap is in the same channel with ra0.
#endif /* APCLI_CONNECTION_TRIAL */

#ifdef WPA_SUPPLICANT_SUPPORT
			apcli_entry->wdev.IEEE8021X=FALSE;
			apcli_entry->wpa_supplicant_info.IEEE8021x_required_keys=FALSE;
			apcli_entry->wpa_supplicant_info.bRSN_IE_FromWpaSupplicant=FALSE;
			apcli_entry->wpa_supplicant_info.bLostAp=FALSE;
			apcli_entry->bScanReqIsFromWebUI=FALSE;
			apcli_entry->bConfigChanged=FALSE;
			apcli_entry->wpa_supplicant_info.DesireSharedKeyId=0;
			apcli_entry->wpa_supplicant_info.WpaSupplicantUP=WPA_SUPPLICANT_DISABLE;
			apcli_entry->wpa_supplicant_info.WpaSupplicantScanCount=0;
			apcli_entry->wpa_supplicant_info.pWpsProbeReqIe=NULL;
			apcli_entry->wpa_supplicant_info.WpsProbeReqIeLen=0;
			apcli_entry->wpa_supplicant_info.pWpaAssocIe=NULL;
			apcli_entry->wpa_supplicant_info.WpaAssocIeLen=0;
			apcli_entry->SavedPMKNum=0;
			RTMPZeroMemory(apcli_entry->SavedPMK, (PMKID_NO * sizeof(BSSID_INFO)));
#endif/*WPA_SUPPLICANT_SUPPORT*/
#ifdef APCLI_AUTO_CONNECT_SUPPORT
			apcli_entry->ApCliSiteSurveyPeriod = 23;
#endif /* APCLI_AUTO_CONNECT_SUPPORT */
			apcli_entry->bBlockAssoc=FALSE;
			apcli_entry->MicErrCnt=0;

			apcli_entry->Valid = FALSE;
			apcli_entry->CfgSsidLen = 0;
			NdisZeroMemory(&(apcli_entry->CfgSsid), MAX_LEN_OF_SSID);
			NdisZeroMemory(apcli_entry->CfgApCliBssid, MAC_ADDR_LEN);
			NdisZeroMemory(apcli_entry->MlmeAux.Bssid, MAC_ADDR_LEN);
		}
#endif /* APCLI_SUPPORT */
		pAd->ApCfg.EntryClientCount = 0;
		pAd->ApCfg.ChangeTxOpClient = 0;
	}

#ifdef DYNAMIC_VGA_SUPPORT
	if (IS_MT76x2(pAd)) {
		pAd->CommonCfg.lna_vga_ctl.long_range_compensate_level = 0;
		pAd->CommonCfg.lna_vga_ctl.bDyncVgaEnable = FALSE;
		pAd->CommonCfg.lna_vga_ctl.nFalseCCATh = 800;
		pAd->CommonCfg.lna_vga_ctl.nLowFalseCCATh = 10;
	}

#ifdef RT6352
	if (IS_RT6352(pAd)) {
		pAd->CommonCfg.lna_vga_ctl.bDyncVgaEnable = TRUE;
		pAd->CommonCfg.lna_vga_ctl.nFalseCCATh = 600;
		pAd->CommonCfg.lna_vga_ctl.nLowFalseCCATh = 10;
	}
#endif
#endif /* DYNAMIC_VGA_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

#ifdef ETH_CONVERT_SUPPORT
	if (pAd->OpMode == OPMODE_STA)
	{
		NdisZeroMemory(pAd->EthConvert.EthCloneMac, MAC_ADDR_LEN);
		pAd->EthConvert.ECMode = ETH_CONVERT_MODE_DISABLE;
		pAd->EthConvert.CloneMacVaild = FALSE;
		/*pAd->EthConvert.nodeCount = 0;*/
		NdisZeroMemory(pAd->EthConvert.SSIDStr, MAX_LEN_OF_SSID);
		pAd->EthConvert.SSIDStrLen = 0;
		pAd->EthConvert.macAutoLearn = FALSE;
		pAd->StaCfg.bFragFlag = TRUE;
	}
#endif /* ETH_CONVERT_SUPPORT */

#ifdef IP_ASSEMBLY
	if (pAd->OpMode == OPMODE_STA)
	{
		pAd->StaCfg.bFragFlag = TRUE;
	}
#endif /* IP_ASSEMBLY */

	/*
		part IV. others
	*/

	/* dynamic BBP R66:sensibity tuning to overcome background noise*/
	pAd->BbpTuning.bEnable = TRUE;
	pAd->BbpTuning.FalseCcaLowerThreshold = 100;
	pAd->BbpTuning.FalseCcaUpperThreshold = 512;
	pAd->BbpTuning.R66Delta = 4;
	pAd->Mlme.bEnableAutoAntennaCheck = TRUE;

	/* Also initial R66CurrentValue, RTUSBResumeMsduTransmission might use this value.*/
	/* if not initial this value, the default value will be 0.*/
	pAd->BbpTuning.R66CurrentValue = 0x38;

#ifdef RTMP_BBP
	pAd->Bbp94 = BBPR94_DEFAULT;
#endif /* RTMP_BBP */
	pAd->BbpForCCK = FALSE;

	/* initialize MAC table and allocate spin lock*/
	NdisZeroMemory(&pAd->MacTab, sizeof(MAC_TABLE));
	InitializeQueueHeader(&pAd->MacTab.McastPsQueue);
	NdisAllocateSpinLock(pAd, &pAd->MacTabLock);

	/*RTMPInitTimer(pAd, &pAd->RECBATimer, RECBATimerTimeout, pAd, TRUE);*/
	/*RTMPSetTimer(&pAd->RECBATimer, REORDER_EXEC_INTV);*/


	pAd->CommonCfg.bWiFiTest = FALSE;
	pAd->CommonCfg.bMcastTest = FALSE;
#ifdef RTMP_MAC_PCI
    pAd->bPCIclkOff = FALSE;
#endif /* RTMP_MAC_PCI */

#ifdef CONFIG_AP_SUPPORT
	pAd->ApCfg.EntryLifeCheck = MAC_ENTRY_LIFE_CHECK_CNT;

#ifdef DOT11R_FT_SUPPORT
	FT_CfgInitial(pAd);
#endif /* DOT11R_FT_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
#ifdef PCIE_PS_SUPPORT
	RTMP_SET_PSFLAG(pAd, fRTMP_PS_CAN_GO_SLEEP);
#endif /* PCIE_PS_SUPPORT */
#ifdef DOT11Z_TDLS_SUPPORT
		pAd->StaCfg.TdlsInfo.bTDLSCapable = FALSE;
		pAd->StaCfg.TdlsInfo.TdlsChSwitchSupp = TRUE;
		pAd->StaCfg.TdlsInfo.TdlsPsmSupp = FALSE;
		pAd->StaCfg.TdlsInfo.TdlsKeyLifeTime = TDLS_LEY_LIFETIME;
#ifdef TDLS_AUTOLINK_SUPPORT
		initList(&pAd->StaCfg.TdlsInfo.TdlsDiscovPeerList);
		NdisAllocateSpinLock(&pAd->StaCfg.TdlsInfo.TdlsDiscovPeerListSemLock);
		initList(&pAd->StaCfg.TdlsInfo.TdlsBlackList);
		NdisAllocateSpinLock(&pAd->StaCfg.TdlsInfo.TdlsBlackListSemLock);

		pAd->StaCfg.TdlsInfo.TdlsAutoSetupRssiThreshold = TDLS_AUTO_SETUP_RSSI_THRESHOLD;
		pAd->StaCfg.TdlsInfo.TdlsAutoTeardownRssiThreshold = TDLS_AUTO_TEARDOWN_RSSI_THRESHOLD;
		pAd->StaCfg.TdlsInfo.TdlsRssiMeasurementPeriod = TDLS_RSSI_MEASUREMENT_PERIOD;
		pAd->StaCfg.TdlsInfo.TdlsDisabledPeriodByTeardown = TDLS_DISABLE_PERIOD_BY_TEARDOWN;
		pAd->StaCfg.TdlsInfo.TdlsAutoDiscoveryPeriod = TDLS_AUTO_DISCOVERY_PERIOD;
#endif /* TDLS_AUTOLINK_SUPPORT */
#endif /* DOT11Z_TDLS_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */

	pAd->RxAnt.Pair1PrimaryRxAnt = 0;
	pAd->RxAnt.Pair1SecondaryRxAnt = 1;

		pAd->RxAnt.EvaluatePeriod = 0;
		pAd->RxAnt.RcvPktNumWhenEvaluate = 0;
#ifdef CONFIG_STA_SUPPORT
		pAd->RxAnt.Pair1AvgRssi[0] = pAd->RxAnt.Pair1AvgRssi[1] = 0;
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
		pAd->RxAnt.Pair1AvgRssiGroup1[0] = pAd->RxAnt.Pair1AvgRssiGroup1[1] = 0;
		pAd->RxAnt.Pair1AvgRssiGroup2[0] = pAd->RxAnt.Pair1AvgRssiGroup2[1] = 0;
#endif /* CONFIG_AP_SUPPORT */


#ifdef TXRX_SW_ANTDIV_SUPPORT
		pAd->chipCap.bTxRxSwAntDiv = FALSE;
#endif /* TXRX_SW_ANTDIV_SUPPORT */


#if defined(AP_SCAN_SUPPORT) || defined(CONFIG_STA_SUPPORT)
	for (i = 0; i < MAX_LEN_OF_BSS_TABLE; i++)
	{
		BSS_ENTRY *pBssEntry = &pAd->ScanTab.BssEntry[i];

		if (pAd->ProbeRespIE[i].pIe)
			pBssEntry->pVarIeFromProbRsp = pAd->ProbeRespIE[i].pIe;
		else
			pBssEntry->pVarIeFromProbRsp = NULL;
	}
#endif /* defined(AP_SCAN_SUPPORT) || defined(CONFIG_STA_SUPPORT) */


#ifdef WSC_INCLUDED
	NdisZeroMemory(&pAd->CommonCfg.WscStaPbcProbeInfo, sizeof(WSC_STA_PBC_PROBE_INFO));
	pAd->CommonCfg.WscPBCOverlap = FALSE;
#endif /* WSC_INCLUDED */


#ifdef RMTP_RBUS_SUPPORT
#ifdef VIDEO_TURBINE_SUPPORT
	VideoConfigInit(pAd);
#endif /* VIDEO_TURBINE_SUPPORT */
#endif /* RMTP_RBUS_SUPPORT */


#ifdef WFD_SUPPORT
	WfdCfgInit(pAd);
#endif /* WFD_SUPPORT */


#if (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT)
	pAd->WOW_Cfg.bEnable = FALSE;
	pAd->WOW_Cfg.bWOWFirmware = FALSE;	/* load normal firmware */
	pAd->WOW_Cfg.bInBand = TRUE;		/* use in-band signal */
	pAd->WOW_Cfg.nSelectedGPIO = 1;
	pAd->WOW_Cfg.nDelay = 3; /* (3+1)*3 = 12 sec */
	pAd->WOW_Cfg.nHoldTime = 1; /* 1*10 = 10 ms */
	DBGPRINT(RT_DEBUG_OFF, ("WOW Enable %d, WOWFirmware %d\n", pAd->WOW_Cfg.bEnable, pAd->WOW_Cfg.bWOWFirmware));
#endif /* (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT) */

	/* 802.11H and DFS related params*/
	pAd->Dot11_H.CSCount = 0;
	pAd->Dot11_H.CSPeriod = 10;

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		pAd->Dot11_H.RDMode = RD_NORMAL_MODE;
#endif

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		pAd->Dot11_H.RDMode = RD_SILENCE_MODE;
#endif

	pAd->Dot11_H.ChMovingTime = 65;
	pAd->Dot11_H.bDFSIndoor = 1;


#ifdef MAC_REPEATER_SUPPORT
	for (i = 0; i < MAX_APCLI_NUM; i++)
	{
		for (j = 0; j < MAX_EXT_MAC_ADDR_SIZE; j++)
		{
			NdisZeroMemory(pAd->ApCfg.ApCliTab[i].RepeaterCli[j].OriginalAddress, MAC_ADDR_LEN);
			NdisZeroMemory(pAd->ApCfg.ApCliTab[i].RepeaterCli[j].CurrentAddress, MAC_ADDR_LEN);
			pAd->ApCfg.ApCliTab[i].RepeaterCli[j].CliConnectState = 0;
			pAd->ApCfg.ApCliTab[i].RepeaterCli[j].CliEnable= FALSE;
			pAd->ApCfg.ApCliTab[i].RepeaterCli[j].CliValid= FALSE;
			pAd->ApCfg.ApCliTab[i].RepeaterCli[j].bEthCli = FALSE;
		}
	}
	NdisAllocateSpinLock(pAd, &pAd->ApCfg.ReptCliEntryLock);
	pAd->ApCfg.RepeaterCliSize = 0;

	NdisZeroMemory(&pAd->ApCfg.ReptControl, sizeof(REPEATER_CTRL_STRUCT));
#endif /* MAC_REPEATER_SUPPORT */

#ifdef AP_PARTIAL_SCAN_SUPPORT
	pAd->ApCfg.bPartialScanning = FALSE;
	pAd->ApCfg.PartialScanChannelNum = DEFLAUT_PARTIAL_SCAN_CH_NUM;
	pAd->ApCfg.LastPartialScanChannel = 0;
	pAd->ApCfg.PartialScanBreakTime = 0;
#endif /* AP_PARTIAL_SCAN_SUPPORT */
#ifdef SMART_MESH
	pAd->ApCfg.ScanTime = 0;
#endif /* SMART_MESH  */

#ifdef RT6352
	if (IS_RT6352(pAd)) {
		pAd->Tx0_DPD_ALC_tag0 = 0;
		pAd->Tx0_DPD_ALC_tag1 = 0;
		pAd->Tx1_DPD_ALC_tag0 = 0;
		pAd->Tx1_DPD_ALC_tag1 = 0;
		pAd->Tx0_DPD_ALC_tag0_flag = 0x0;
		pAd->Tx0_DPD_ALC_tag1_flag = 0x0;
		pAd->Tx1_DPD_ALC_tag0_flag = 0x0;
		pAd->Tx1_DPD_ALC_tag1_flag = 0x0;
	}
#endif /* RT6352 */

#ifdef APCLI_SUPPORT
#ifdef APCLI_AUTO_CONNECT_SUPPORT
	pAd->ApCfg.ApCliAutoConnectRunning= FALSE;
	pAd->ApCfg.ApCliAutoConnectChannelSwitching = FALSE;
#endif /* APCLI_AUTO_CONNECT_SUPPORT */
#endif /* APCLI_SUPPORT */

#ifdef CONFIG_FPGA_MODE
	pAd->fpga_ctl.fpga_on = 0x0;
	pAd->fpga_ctl.tx_kick_cnt = 0;
	pAd->fpga_ctl.tx_data_phy = 0;
	pAd->fpga_ctl.tx_data_ldpc = 0;
	pAd->fpga_ctl.tx_data_mcs = 0;
	pAd->fpga_ctl.tx_data_bw = 0;
	pAd->fpga_ctl.tx_data_gi = 0;
	pAd->fpga_ctl.rx_data_phy = 0;
	pAd->fpga_ctl.rx_data_ldpc = 0;
	pAd->fpga_ctl.rx_data_mcs = 0;
	pAd->fpga_ctl.rx_data_bw = 0;
	pAd->fpga_ctl.rx_data_gi = 0;
#ifdef CAPTURE_MODE
	pAd->fpga_ctl.cap_type = 2; /* CAP_MODE_ADC8; */
	pAd->fpga_ctl.cap_trigger = 2; /* CAP_TRIGGER_AUTO; */
	pAd->fpga_ctl.trigger_offset = 200;
	pAd->fpga_ctl.cap_support = 0;
#endif /* CAPTURE_MODE */
#endif /* CONFIG_FPGA_MODE */

#ifdef MICROWAVE_OVEN_SUPPORT
	if (pAd->OpMode == OPMODE_AP)
		pAd->CommonCfg.MO_Cfg.bEnable = TRUE;
	else
		pAd->CommonCfg.MO_Cfg.bEnable = FALSE;
	pAd->CommonCfg.MO_Cfg.nFalseCCATh = MO_FALSE_CCA_TH;
#endif /* MICROWAVE_OVEN_SUPPORT */

#ifdef RT6352
	pAd->CommonCfg.bEnTemperatureTrack = FALSE;
#endif /* RT6352 */


#ifdef DOT11_VHT_AC
	pAd->CommonCfg.bNonVhtDisallow = FALSE;
#endif /* DOT11_VHT_AC */



	pAd->ed_chk = FALSE; //let country region to turn on
#ifdef ED_MONITOR
	pAd->ed_learning_time_threshold = 50; //5 sec
	pAd->ed_debug = FALSE;

#ifdef CONFIG_AP_SUPPORT
	pAd->ed_sta_threshold = 1;
	pAd->ed_ap_threshold = 1;
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
	pAd->ed_ap_scaned = 5;
	pAd->ed_current_ch_aps = 1;
#endif /* CONFIG_STA_SUPPORT */
	//change to common part
	pAd->ed_rssi_threshold = -80;

	pAd->ed_chk_period = 100;
	pAd->ed_threshold = 90;
	pAd->ed_false_cca_threshold = 180;	
	pAd->ed_block_tx_threshold = 2;
#endif /* ED_MONITOR */

#ifdef CONFIG_SNIFFER_SUPPORT
	pAd->monitor_ctrl.current_monitor_mode = 0;
#endif /* CONFIG_SNIFFER_SUPPORT */

#ifdef DMA_BUSY_RESET
#ifdef RTMP_PCI_SUPPORT
	pAd->PDMAWatchDogEn = 0;
	pAd->PDMAWatchDogDbg = 0 ;
	pAd->TxDMACheckTimes = 0;
	pAd->RxDMACheckTimes = 0;
	pAd->TxDMAResetCount = 0;
	pAd->RxDMAResetCount = 0;
#endif /* RTMP_PCI_SUPPORT */
#endif /* DMA_BUSY_RESET */

    pAd->CommonCfg.ManualTxopThreshold = 10; // Mbps
    pAd->CommonCfg.ManualTxopUpBound = 15; // Ratio
    pAd->CommonCfg.ManualTxopLowBound = 5; // Ratio

	DBGPRINT(RT_DEBUG_TRACE, ("<-- UserCfgInit\n"));
}


/* IRQL = PASSIVE_LEVEL*/
UCHAR BtoH(STRING ch)
{
	if (ch >= '0' && ch <= '9') return (ch - '0');        /* Handle numerals*/
	if (ch >= 'A' && ch <= 'F') return (ch - 'A' + 0xA);  /* Handle capitol hex digits*/
	if (ch >= 'a' && ch <= 'f') return (ch - 'a' + 0xA);  /* Handle small hex digits*/
	return(255);
}


/*
	FUNCTION: AtoH(char *, UCHAR *, int)

	PURPOSE:  Converts ascii string to network order hex

	PARAMETERS:
		src    - pointer to input ascii string
		dest   - pointer to output hex
		srclen - size of src

	COMMENTS:

		2 ascii bytes make a hex byte so must put 1st ascii byte of pair
		into upper nibble and 2nd ascii byte of pair into lower nibble.

	IRQL = PASSIVE_LEVEL
*/
VOID AtoH(PSTRING src, PUCHAR dest, INT srclen)
{
	PSTRING srcptr;
	PUCHAR destTemp;

	srcptr = src;
	destTemp = (PUCHAR) dest;

	while(srclen--)
	{
		*destTemp = BtoH(*srcptr++) << 4;    /* Put 1st ascii byte in upper nibble.*/
		*destTemp += BtoH(*srcptr++);      /* Add 2nd ascii byte to above.*/
		destTemp++;
	}
}


/*
========================================================================
Routine Description:
	Add a timer to the timer list.

Arguments:
	pAd				- WLAN control block pointer
	pRsc			- the OS resource

Return Value:
	None

Note:
========================================================================
*/
VOID RTMP_TimerListAdd(RTMP_ADAPTER *pAd, VOID *pRsc)
{
	LIST_HEADER *pRscList = &pAd->RscTimerCreateList;
	LIST_RESOURCE_OBJ_ENTRY *pObj;


	/* try to find old entry */
	pObj = (LIST_RESOURCE_OBJ_ENTRY *)(pRscList->pHead);
	while(1)
	{
		if (pObj == NULL)
			break;
		if ((ULONG)(pObj->pRscObj) == (ULONG)pRsc)
			return; /* exists */
		pObj = pObj->pNext;
	}

	/* allocate a timer record entry */
	os_alloc_mem(NULL, (UCHAR **)&(pObj), sizeof(LIST_RESOURCE_OBJ_ENTRY));
	if (pObj == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s: alloc timer obj fail!\n", __FUNCTION__));
		return;
	}
	else
	{
		pObj->pRscObj = pRsc;
		insertTailList(pRscList, (LIST_ENTRY *)pObj);
	}
}

VOID RTMP_TimerListRelease(RTMP_ADAPTER *pAd, VOID *pRsc)
{
	LIST_HEADER *pRscList = &pAd->RscTimerCreateList;
	LIST_RESOURCE_OBJ_ENTRY *pObj;
	LIST_ENTRY *pListEntry;

	pListEntry = pRscList->pHead;
	pObj = (LIST_RESOURCE_OBJ_ENTRY *)pListEntry;

	while (pObj)
	{
		if ((ULONG)(pObj->pRscObj) == (ULONG)pRsc)
		{
			pListEntry = (LIST_ENTRY *)pObj;
			break;
		}

		pListEntry = pListEntry->pNext;
		pObj = (LIST_RESOURCE_OBJ_ENTRY *)pListEntry;
	}

	if (pListEntry)
	{
		delEntryList(pRscList, pListEntry);

		/* free a timer record entry */
		DBGPRINT(RT_DEBUG_LOUD, ("%s: release timer obj %lx!\n", __FUNCTION__, (ULONG)pRsc));
		os_free_mem(NULL, pObj);
	}
}

/*
========================================================================
Routine Description:
	Cancel all timers in the timer list.

Arguments:
	pAd				- WLAN control block pointer

Return Value:
	None

Note:
========================================================================
*/
VOID RTMP_AllTimerListRelease(RTMP_ADAPTER *pAd)
{
	LIST_HEADER *pRscList = &pAd->RscTimerCreateList;
	LIST_RESOURCE_OBJ_ENTRY *pObj, *pObjOld;
	BOOLEAN Cancel;

	/* try to find old entry */
	pObj = (LIST_RESOURCE_OBJ_ENTRY *)(pRscList->pHead);
	while(1)
	{
		if (pObj == NULL)
			break;
		DBGPRINT(RT_DEBUG_LOUD, ("%s: Cancel timer obj %lx!\n", __FUNCTION__, (ULONG)(pObj->pRscObj)));
		pObjOld = pObj;
		pObj = pObj->pNext;
		RTMPReleaseTimer(pObjOld->pRscObj, &Cancel);

		//TODO: It will casued kernel panic on reboot
		//os_free_mem(NULL, pObjOld);
	}

	/* reset TimerList */
	initList(&pAd->RscTimerCreateList);
}


/*
	========================================================================

	Routine Description:
		Init timer objects

	Arguments:
		pAd			Pointer to our adapter
		pTimer				Timer structure
		pTimerFunc			Function to execute when timer expired
		Repeat				Ture for period timer

	Return Value:
		None

	Note:

	========================================================================
*/
VOID RTMPInitTimer(
	IN	RTMP_ADAPTER *pAd,
	IN	RALINK_TIMER_STRUCT *pTimer,
	IN	VOID *pTimerFunc,
	IN	VOID *pData,
	IN	BOOLEAN	 Repeat)
{
	RTMP_SEM_LOCK(&TimerSemLock);

	RTMP_TimerListAdd(pAd, pTimer);


	/* Set Valid to TRUE for later used.*/
	/* It will crash if we cancel a timer or set a timer */
	/* that we haven't initialize before.*/
	/* */
	pTimer->Valid      = TRUE;

	pTimer->PeriodicType = Repeat;
	pTimer->State      = FALSE;
	pTimer->cookie = (ULONG) pData;
	pTimer->pAd = pAd;

	RTMP_OS_Init_Timer(pAd, &pTimer->TimerObj,	pTimerFunc, (PVOID) pTimer, &pAd->RscTimerMemList);
	DBGPRINT(RT_DEBUG_LOUD,("%s: %lx\n",__FUNCTION__, (ULONG)pTimer));

	RTMP_SEM_UNLOCK(&TimerSemLock);
}


/*
	========================================================================

	Routine Description:
		Init timer objects

	Arguments:
		pTimer				Timer structure
		Value				Timer value in milliseconds

	Return Value:
		None

	Note:
		To use this routine, must call RTMPInitTimer before.

	========================================================================
*/
VOID RTMPSetTimer(RALINK_TIMER_STRUCT *pTimer, ULONG Value)
{
	RTMP_SEM_LOCK(&TimerSemLock);

	if (pTimer->Valid)
	{
		RTMP_ADAPTER *pAd;

		pAd = (RTMP_ADAPTER *)pTimer->pAd;
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST))
		{
			DBGPRINT_ERR(("RTMPSetTimer failed, Halt in Progress!\n"));
			RTMP_SEM_UNLOCK(&TimerSemLock);
			return;
		}

		pTimer->TimerValue = Value;
		pTimer->State      = FALSE;
		if (pTimer->PeriodicType == TRUE)
		{
			pTimer->Repeat = TRUE;
			RTMP_SetPeriodicTimer(&pTimer->TimerObj, Value);
		}
		else
		{
			pTimer->Repeat = FALSE;
			RTMP_OS_Add_Timer(&pTimer->TimerObj, Value);
		}

		DBGPRINT(RT_DEBUG_LOUD,("%s: %lx\n",__FUNCTION__, (ULONG)pTimer));
	}
	else
	{
		DBGPRINT_ERR(("RTMPSetTimer failed, Timer hasn't been initialize!\n"));
	}
	RTMP_SEM_UNLOCK(&TimerSemLock);
}


/*
	========================================================================

	Routine Description:
		Init timer objects

	Arguments:
		pTimer				Timer structure
		Value				Timer value in milliseconds

	Return Value:
		None

	Note:
		To use this routine, must call RTMPInitTimer before.

	========================================================================
*/
VOID RTMPModTimer(RALINK_TIMER_STRUCT *pTimer, ULONG Value)
{
	BOOLEAN	Cancel;


	RTMP_SEM_LOCK(&TimerSemLock);

	if (pTimer->Valid)
	{
		pTimer->TimerValue = Value;
		pTimer->State      = FALSE;
		if (pTimer->PeriodicType == TRUE)
		{
			RTMP_SEM_UNLOCK(&TimerSemLock);
			RTMPCancelTimer(pTimer, &Cancel);
			RTMPSetTimer(pTimer, Value);
		}
		else
		{
			RTMP_OS_Mod_Timer(&pTimer->TimerObj, Value);
			RTMP_SEM_UNLOCK(&TimerSemLock);
		}
		DBGPRINT(RT_DEBUG_LOUD,("%s: %lx\n",__FUNCTION__, (ULONG)pTimer));
	}
	else
	{
		DBGPRINT_ERR(("RTMPModTimer failed, Timer hasn't been initialize!\n"));
		RTMP_SEM_UNLOCK(&TimerSemLock);
	}
}


/*
	========================================================================

	Routine Description:
		Cancel timer objects

	Arguments:
		Adapter						Pointer to our adapter

	Return Value:
		None

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL

	Note:
		1.) To use this routine, must call RTMPInitTimer before.
		2.) Reset NIC to initial state AS IS system boot up time.

	========================================================================
*/
VOID RTMPCancelTimer(RALINK_TIMER_STRUCT *pTimer, BOOLEAN *pCancelled)
{
	// TODO: shiang-usw, check the purpose of this SemLock!
	RTMP_SEM_LOCK(&TimerSemLock);

	if (pTimer->Valid)
	{
		if (pTimer->State == FALSE)
			pTimer->Repeat = FALSE;

		RTMP_SEM_UNLOCK(&TimerSemLock);
		RTMP_OS_Del_Timer(&pTimer->TimerObj, pCancelled);
		RTMP_SEM_LOCK(&TimerSemLock);

		if (*pCancelled == TRUE)
			pTimer->State = TRUE;

#ifdef RTMP_TIMER_TASK_SUPPORT
		/* We need to go-through the TimerQ to findout this timer handler and remove it if */
		/*		it's still waiting for execution.*/
		RtmpTimerQRemove(pTimer->pAd, pTimer);
#endif /* RTMP_TIMER_TASK_SUPPORT */

		DBGPRINT(RT_DEBUG_LOUD,("%s: %lx\n",__FUNCTION__, (ULONG)pTimer));
	}
	else
	{
		DBGPRINT(RT_DEBUG_INFO,("RTMPCancelTimer failed, Timer hasn't been initialize!\n"));
	}

	RTMP_SEM_UNLOCK(&TimerSemLock);
}


VOID RTMPReleaseTimer(RALINK_TIMER_STRUCT *pTimer, BOOLEAN *pCancelled)
{
	RTMP_SEM_LOCK(&TimerSemLock);

	if (pTimer->Valid)
	{
		if (pTimer->State == FALSE)
			pTimer->Repeat = FALSE;

		RTMP_OS_Del_Timer(&pTimer->TimerObj, pCancelled);

		if (*pCancelled == TRUE)
			pTimer->State = TRUE;

#ifdef RTMP_TIMER_TASK_SUPPORT
		/* We need to go-through the TimerQ to findout this timer handler and remove it if */
		/*		it's still waiting for execution.*/
		RtmpTimerQRemove(pTimer->pAd, pTimer);
#endif /* RTMP_TIMER_TASK_SUPPORT */

		/* release timer */
		RTMP_OS_Release_Timer(&pTimer->TimerObj);

		pTimer->Valid = FALSE;

		RTMP_TimerListRelease(pTimer->pAd, pTimer);

		DBGPRINT(RT_DEBUG_LOUD,("%s: %lx\n",__FUNCTION__, (ULONG)pTimer));
	}
	else
	{
		DBGPRINT(RT_DEBUG_INFO,("RTMPReleasefailed, Timer hasn't been initialize!\n"));
	}

	RTMP_SEM_UNLOCK(&TimerSemLock);
}


/*
	========================================================================

	Routine Description:
		Enable RX

	Arguments:
		pAd						Pointer to our adapter

	Return Value:
		None

	IRQL <= DISPATCH_LEVEL

	Note:
		Before Enable RX, make sure you have enabled Interrupt.
	========================================================================
*/
VOID RTMPEnableRxTx(RTMP_ADAPTER *pAd)
{
	DBGPRINT(RT_DEBUG_TRACE, ("==> RTMPEnableRxTx\n"));

	RT28XXDMAEnable(pAd);

	AsicSetRxFilter(pAd);

	{
		RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, 0xc);
//+++Add by shiang for debug for pbf_loopback
//			RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, 0x2c);
//---Add by shiang for debug
//+++Add by shiang for debug invalid RxWI->WCID
#ifdef RT8592
#endif /* RT8592 */
//---Add by shiang for  debug invalid RxWI->WCID
	}

	DBGPRINT(RT_DEBUG_TRACE, ("<== RTMPEnableRxTx\n"));
}


void CfgInitHook(RTMP_ADAPTER *pAd)
{
	/*pAd->bBroadComHT = TRUE;*/
}


static INT RtmpChipOpsRegister(RTMP_ADAPTER *pAd, INT infType)
{
	RTMP_CHIP_OP *pChipOps = &pAd->chipOps;
	int ret = 0;

	NdisZeroMemory(pChipOps, sizeof(RTMP_CHIP_OP));
	NdisZeroMemory(&pAd->chipCap, sizeof(RTMP_CHIP_CAP));

	ret = RtmpChipOpsHook(pAd);

	if (ret) {
		DBGPRINT(RT_DEBUG_ERROR, ("chipOps hook error\n"));
		return ret;
	}

	/* MCU related */
	ChipOpsMCUHook(pAd, pAd->chipCap.MCUType);

	get_dev_config_idx(pAd);

	/* set eeprom related hook functions */
	ret = RtmpChipOpsEepromHook(pAd, infType);

	return ret;
}




INT RtmpRaDevCtrlInit(VOID *pAdSrc, RTMP_INF_TYPE infType)
{
	RTMP_ADAPTER *pAd = (PRTMP_ADAPTER)pAdSrc;

	/* Assign the interface type. We need use it when do register/EEPROM access.*/
	pAd->infType = infType;

#ifdef CONFIG_STA_SUPPORT
	pAd->OpMode = OPMODE_STA;
	printk("%s STA Driver version: %s\n", "MT76x2", STA_DRIVER_VERSION);
#endif /* CONFIG_STA_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
	pAd->OpMode = OPMODE_AP;
	printk("%s AP Driver version: %s\n", "MT76x2", AP_DRIVER_VERSION);
#endif /* CONFIG_AP_SUPPORT */

	DBGPRINT(RT_DEBUG_TRACE, ("pAd->infType=%d\n", pAd->infType));

#if defined(P2P_SUPPORT) || defined(RT_CFG80211_P2P_SUPPORT)
	pAd->OpMode = OPMODE_STA;
#endif /* P2P_SUPPORT */

#ifdef MULTIPLE_CARD_SUPPORT
#ifdef RTMP_FLASH_SUPPORT
/*	if ((IS_PCIE_INF(pAd))) */
	{
		/* specific for RT6855/RT6856 */
		pAd->E2P_OFFSET_IN_FLASH[0] = 0x40000;
		pAd->E2P_OFFSET_IN_FLASH[1] = 0x48000;
	}
#endif /* RTMP_FLASH_SUPPORT */
#endif /* MULTIPLE_CARD_SUPPORT */

	if (RtmpChipOpsRegister(pAd, infType))
		return FALSE;


#ifdef MULTIPLE_CARD_SUPPORT
{
	extern BOOLEAN RTMP_CardInfoRead(PRTMP_ADAPTER pAd);

	/* find its profile path*/
	pAd->MC_RowID = -1; /* use default profile path*/
	RTMP_CardInfoRead(pAd);

	if (pAd->MC_RowID == -1)
#ifdef CONFIG_AP_SUPPORT
		strcpy(pAd->MC_FileName, AP_PROFILE_PATH);
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		strcpy(pAd->MC_FileName, STA_PROFILE_PATH);
#endif /* CONFIG_STA_SUPPORT */

	DBGPRINT(RT_DEBUG_TRACE, ("MC> ROW = %d, PATH = %s\n", pAd->MC_RowID, pAd->MC_FileName));
}
#endif /* MULTIPLE_CARD_SUPPORT */


#ifdef MCS_LUT_SUPPORT
	if (pAd->chipCap.asic_caps & fASIC_CAP_MCS_LUT) {
		if (MAX_LEN_OF_MAC_TABLE <= 128) {
			RTMP_SET_MORE_FLAG(pAd, fASIC_CAP_MCS_LUT);
		} else {
			DBGPRINT(RT_DEBUG_WARN, ("%s(): MCS_LUT not used becasue MacTb size(%d) > 128!\n",
						__FUNCTION__, MAX_LEN_OF_MAC_TABLE));
		}
	}
#endif /* MCS_LUT_SUPPORT */
	return 0;
}


BOOLEAN RtmpRaDevCtrlExit(IN VOID *pAdSrc)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pAdSrc;
	INT index;

#ifdef MULTIPLE_CARD_SUPPORT
extern UINT8  MC_CardUsed[MAX_NUM_OF_MULTIPLE_CARD];

	if ((pAd->MC_RowID >= 0) && (pAd->MC_RowID <= MAX_NUM_OF_MULTIPLE_CARD))
		MC_CardUsed[pAd->MC_RowID] = 0; /* not clear MAC address*/
#endif /* MULTIPLE_CARD_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
#ifdef CREDENTIAL_STORE
		NdisFreeSpinLock(&pAd->StaCtIf.Lock);
#endif /* CREDENTIAL_STORE */
#endif /* CONFIG_STA_SUPPORT */

#ifdef RLT_MAC
	if ((IS_MT76x0(pAd) || IS_MT76x2(pAd))&& (pAd->WlanFunCtrl.field.WLAN_EN == 1))
	{
		rlt_wlan_chip_onoff(pAd, FALSE, FALSE);
	}
#endif /* RLT_MAC */


	/*
		Free ProbeRespIE Table
	*/
	for (index = 0; index < MAX_LEN_OF_BSS_TABLE; index++)
	{
		if (pAd->ProbeRespIE[index].pIe)
			os_free_mem(pAd, pAd->ProbeRespIE[index].pIe);
	}

#ifdef RESOURCE_PRE_ALLOC
	RTMPFreeTxRxRingMemory(pAd);
#endif /* RESOURCE_PRE_ALLOC */

	RTMPFreeAdapter(pAd);

	return TRUE;
}


#ifdef CONFIG_AP_SUPPORT
#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
VOID RTMP_11N_D3_TimerInit(RTMP_ADAPTER *pAd)
{
	RTMPInitTimer(pAd, &pAd->CommonCfg.Bss2040CoexistTimer, GET_TIMER_FUNCTION(Bss2040CoexistTimeOut), pAd, FALSE);
}
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */


#ifdef VENDOR_FEATURE3_SUPPORT
VOID RTMP_IO_WRITE32(
	PRTMP_ADAPTER pAd,
	UINT32 Offset,
	UINT32 Value)
{
	_RTMP_IO_WRITE32(pAd, Offset, Value);
}
#endif /* VENDOR_FEATURE3_SUPPORT */


#ifdef RTMP_MAC_PCI
VOID CMDHandler(RTMP_ADAPTER *pAd)
{
	PCmdQElmt cmdqelmt;
	UCHAR *pData;

	while (pAd && pAd->CmdQ.size > 0)
	{
		NdisAcquireSpinLock(&pAd->CmdQLock);
		RTThreadDequeueCmd(&pAd->CmdQ, &cmdqelmt);
		NdisReleaseSpinLock(&pAd->CmdQLock);

		if (cmdqelmt == NULL)
			break;

		pData = cmdqelmt->buffer;
		ASSERT(pData != NULL);

		if(!(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST) || RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)))
		{
			switch (cmdqelmt->command)
			{
#ifdef CONFIG_AP_SUPPORT
				case CMDTHREAD_CHAN_RESCAN:
					DBGPRINT(RT_DEBUG_TRACE, ("cmd> Re-scan channel! \n"));

					pAd->CommonCfg.Channel = AP_AUTO_CH_SEL(pAd, ChannelAlgCCA);

#ifdef DOT11_N_SUPPORT
					/* If WMODE_CAP_N(phymode) and BW=40 check extension channel, after select channel  */
					N_ChannelCheck(pAd);
#endif /* DOT11_N_SUPPORT */

					DBGPRINT(RT_DEBUG_TRACE, ("cmd> Switch to %d! \n", pAd->CommonCfg.Channel));
					APStop(pAd);
					APStartUp(pAd);

#ifdef AP_QLOAD_SUPPORT
					QBSS_LoadAlarmResume(pAd);
#endif /* AP_QLOAD_SUPPORT */
					break;
#endif /* CONFIG_AP_SUPPORT */

				case CMDTHREAD_REG_HINT:
#ifdef LINUX
#ifdef RT_CFG80211_SUPPORT
					RT_CFG80211_CRDA_REG_HINT(pAd, pData, cmdqelmt->bufferlength);
#endif /* RT_CFG80211_SUPPORT */
#endif /* LINUX */
					break;

				case CMDTHREAD_REG_HINT_11D:
#ifdef LINUX
#ifdef RT_CFG80211_SUPPORT
					RT_CFG80211_CRDA_REG_HINT11D(pAd, pData, cmdqelmt->bufferlength);
#endif /* RT_CFG80211_SUPPORT */
#endif /* LINUX */
					break;

				case CMDTHREAD_SCAN_END:
#ifdef LINUX
#ifdef RT_CFG80211_SUPPORT
					RT_CFG80211_SCAN_END(pAd, FALSE);
#endif /* RT_CFG80211_SUPPORT */
#endif /* LINUX */
					break;

				case CMDTHREAD_CONNECT_RESULT_INFORM:
#ifdef LINUX
#ifdef RT_CFG80211_SUPPORT
					RT_CFG80211_CONN_RESULT_INFORM(pAd,
												pAd->MlmeAux.Bssid,
												pData, cmdqelmt->bufferlength,
												pData, cmdqelmt->bufferlength,
												1);
#endif /* RT_CFG80211_SUPPORT */
#endif /* LINUX */
					break;

				default:
					DBGPRINT(RT_DEBUG_ERROR, ("--> Control Thread !! ERROR !! Unknown(cmdqelmt->command=0x%x) !! \n", cmdqelmt->command));
					break;
			}
		}

		if (cmdqelmt->CmdFromNdis == TRUE)
		{
			if (cmdqelmt->buffer != NULL)
				os_free_mem(pAd, cmdqelmt->buffer);
			os_free_mem(pAd, cmdqelmt);
		}
		else
		{
			if ((cmdqelmt->buffer != NULL) && (cmdqelmt->bufferlength != 0))
				os_free_mem(pAd, cmdqelmt->buffer);
			os_free_mem(pAd, cmdqelmt);
		}
	}
}
#endif /* RTMP_MAC_PCI */


VOID AntCfgInit(RTMP_ADAPTER *pAd)
{


#ifdef TXRX_SW_ANTDIV_SUPPORT
	/* EEPROM 0x34[15:12] = 0xF is invalid, 0x2~0x3 is TX/RX SW AntDiv */
	DBGPRINT(RT_DEBUG_OFF, ("%s: bTxRxSwAntDiv %d\n", __FUNCTION__, pAd->chipCap.bTxRxSwAntDiv));
	if (pAd->chipCap.bTxRxSwAntDiv)
	{
		DBGPRINT(RT_DEBUG_OFF, ("Antenna word %X/%d, AntDiv %d\n",
					pAd->Antenna.word, pAd->Antenna.field.BoardType, pAd->NicConfig2.field.AntDiversity));
	}
#endif /* TXRX_SW_ANTDIV_SUPPORT */

	{
		if (pAd->NicConfig2.field.AntOpt== 1) /* ant selected by efuse */
		{
			if (pAd->NicConfig2.field.AntDiversity == 0) /* main */
			{
				pAd->RxAnt.Pair1PrimaryRxAnt = 0;
				pAd->RxAnt.Pair1SecondaryRxAnt = 1;
			}
			else/* aux */
			{
				pAd->RxAnt.Pair1PrimaryRxAnt = 1;
				pAd->RxAnt.Pair1SecondaryRxAnt = 0;
			}
		}
		else if (pAd->NicConfig2.field.AntDiversity == 0) /* Ant div off: default ant is main */
		{
			pAd->RxAnt.Pair1PrimaryRxAnt = 0;
			pAd->RxAnt.Pair1SecondaryRxAnt = 1;
		}
		else if (pAd->NicConfig2.field.AntDiversity == 1)/* Ant div on */
		{/* eeprom on, but sw ant div support is not enabled: default ant is main */
			pAd->RxAnt.Pair1PrimaryRxAnt = 0;
			pAd->RxAnt.Pair1SecondaryRxAnt = 1;
		}
	}

	DBGPRINT(RT_DEBUG_OFF, ("%s: primary/secondary ant %d/%d\n",
					__FUNCTION__,
					pAd->RxAnt.Pair1PrimaryRxAnt,
					pAd->RxAnt.Pair1SecondaryRxAnt));
}


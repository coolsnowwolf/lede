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
	eeprom.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
*/
#include "rt_config.h"

struct chip_map{
	UINT32 ChipVersion;
	RTMP_STRING *name;
};

struct chip_map RTMP_CHIP_E2P_FILE_TABLE[] = {
	{0x3071,	"RT3092_PCIe_LNA_2T2R_ALC_V1_2.bin"},
	{0x3090,	"RT3092_PCIe_LNA_2T2R_ALC_V1_2.bin"},
	{0x3593,	"HMC_RT3593_PCIe_3T3R_V1_3.bin"},
	{0x5392,	"RT5392_PCIe_2T2R_ALC_V1_4.bin"},
	{0x5592,	"RT5592_PCIe_2T2R_V1_7.bin"},
	{0,}
};


struct chip_map chip_card_id_map[] ={
	{7620, ""},
};


INT rtmp_read_txmixer_gain_from_eeprom(RTMP_ADAPTER *pAd)
{
	UINT16 value;

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


	return TRUE;
}


INT rtmp_read_rssi_langain_from_eeprom(RTMP_ADAPTER *pAd)
{
	INT i;
	UINT16 value;

	/* Get RSSI Offset on EEPROM 0x9Ah & 0x9Ch.*/
	/* The valid value are (-10 ~ 10) */
	/* */
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

	return TRUE;
}


/*
	CountryRegion byte offset (38h)
*/
INT rtmp_read_country_region_from_eeporm(RTMP_ADAPTER *pAd)
{
	UINT16 value, value2;

	{
		value = pAd->EEPROMDefaultValue[EEPROM_COUNTRY_REG_OFFSET] >> 8;		/* 2.4G band*/
		value2 = pAd->EEPROMDefaultValue[EEPROM_COUNTRY_REG_OFFSET] & 0x00FF;	/* 5G band*/
	}

	if ((value <= REGION_MAXIMUM_BG_BAND) || (value == REGION_31_BG_BAND) || (value == REGION_32_BG_BAND) || (value == REGION_33_BG_BAND) )
		pAd->CommonCfg.CountryRegion = ((UCHAR) value) | EEPROM_IS_PROGRAMMED;

	if (value2 <= REGION_MAXIMUM_A_BAND)
		pAd->CommonCfg.CountryRegionForABand = ((UCHAR) value2) | EEPROM_IS_PROGRAMMED;

	return TRUE;
}


/*
	Read frequency offset setting from EEPROM which used for RF
*/
INT rtmp_read_freq_offset_from_eeprom(RTMP_ADAPTER *pAd)
{
	UINT16 value;

		RT28xx_EEPROM_READ16(pAd, EEPROM_FREQ_OFFSET, value);

	{
		if ((value & 0x00FF) != 0x00FF)
			pAd->RfFreqOffset = (ULONG) (value & 0x00FF);
		else
			pAd->RfFreqOffset = 0;
	}



	DBGPRINT(RT_DEBUG_TRACE, ("E2PROM: RF FreqOffset=0x%x \n", pAd->RfFreqOffset));

	return TRUE;
}


INT rtmp_read_txpwr_from_eeprom(RTMP_ADAPTER *pAd)
{
	/* if not return early. cause fail at emulation.*/
	/* Init the channel number for TX channel power*/
#ifdef MT7603
	if (IS_MT7603(pAd))
		mt7603_read_chl_pwr(pAd);
	else
#endif
		RTMPReadChannelPwr(pAd);

	RTMPReadTxPwrPerRate(pAd);



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
	InitSkuRateDiffTable(pAd);
#endif /* SINGLE_SKU_V2 */

	return TRUE;
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
INT NICReadEEPROMParameters(RTMP_ADAPTER *pAd, RTMP_STRING *mac_addr)
{
	USHORT i, value;
	EEPROM_VERSION_STRUC Version;
	EEPROM_ANTENNA_STRUC Antenna;
	EEPROM_NIC_CONFIG2_STRUC NicConfig2;
	USHORT  Addr01,Addr23,Addr45 ;

	DBGPRINT(RT_DEBUG_TRACE, ("%s()-->\n", __FUNCTION__));


	if (pAd->chipOps.eeinit)
	{
#ifndef MULTIPLE_CARD_SUPPORT
		/* If we are run in Multicard mode, the eeinit shall execute in RTMP_CardInfoRead() */
		pAd->chipOps.eeinit(pAd);
#endif /* MULTIPLE_CARD_SUPPORT */

	}

	/*Send EEprom parameter to FW*/
	#ifdef CONFIG_ATE
	if(!ATE_ON(pAd))
	#endif
	{
#ifdef LOAD_FW_ONE_TIME
		DBGPRINT(RT_DEBUG_ERROR, ("@@@  NICReadEEPROMParameters : pAd->FWLoad=%u \n",pAd->FWLoad));		
		if (pAd->FWLoad == 0)
#endif /* LOAD_FW_ONE_TIME */
			CmdEfusBufferModeSet(pAd);
	}

	/* Read MAC setting from EEPROM and record as permanent MAC address */
	DBGPRINT(RT_DEBUG_TRACE, ("Initialize MAC Address from E2PROM \n"));

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

	DBGPRINT(RT_DEBUG_TRACE, ("E2PROM MAC: =%02x:%02x:%02x:%02x:%02x:%02x\n",
								PRINT_MAC(pAd->PermanentAddress)));

	/* Assign the actually working MAC Address */
	if (pAd->bLocalAdminMAC)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("Use the MAC address what is assigned from Configuration file(.dat). \n"));
#if defined(BB_SOC)&&!defined(NEW_MBSSID_MODE)
		//BBUPrepareMAC(pAd, pAd->CurrentAddress);
		COPY_MAC_ADDR(pAd->PermanentAddress, pAd->CurrentAddress);
		printk("now bb MainSsid mac %02x:%02x:%02x:%02x:%02x:%02x\n",PRINT_MAC(pAd->CurrentAddress));
#endif
	}
	else if (mac_addr &&
			 strlen((RTMP_STRING *)mac_addr) == 17 &&
			 (strcmp(mac_addr, "00:00:00:00:00:00") != 0))
	{
		INT j;
		RTMP_STRING *macptr;

		macptr = (RTMP_STRING *) mac_addr;
		for (j=0; j<MAC_ADDR_LEN; j++)
		{
			AtoH(macptr, &pAd->CurrentAddress[j], 1);
			macptr=macptr+3;
		}

		DBGPRINT(RT_DEBUG_TRACE, ("Use the MAC address what is assigned from Moudle Parameter. \n"));
	}
	else
	{
		COPY_MAC_ADDR(pAd->CurrentAddress, pAd->PermanentAddress);
		DBGPRINT(RT_DEBUG_TRACE, ("Use the MAC address what is assigned from EEPROM. \n"));
	}

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


	{
		RT28xx_EEPROM_READ16(pAd, EEPROM_COUNTRY_REGION, value);	/* Country Region*/
		pAd->EEPROMDefaultValue[EEPROM_COUNTRY_REG_OFFSET] = value;
		DBGPRINT(RT_DEBUG_OFF, ("Country Region from e2p = %x\n", value));
	}

	for(i = 0; i < 8; i++)
	{
#if defined(RT65xx) || defined(MT7601) || defined(MT7603) || defined(MT7628)/* MT7650 EEPROM doesn't have those BBP setting @20121001 */
		if (IS_RT65XX(pAd) || IS_MT7601(pAd) || IS_MT7603(pAd) || IS_MT7628(pAd))
			break;
#endif /* defined(RT65xx) || defined(MT7601) || defined(MT7603) */

		RT28xx_EEPROM_READ16(pAd, EEPROM_BBP_BASE_OFFSET + i*2, value);
		pAd->EEPROMDefaultValue[i+EEPROM_BBP_ARRAY_OFFSET] = value;
	}

	/* We have to parse NIC configuration 0 at here.*/
	/* If TSSI did not have preloaded value, it should reset the TxAutoAgc to false*/
	/* Therefore, we have to read TxAutoAgc control beforehand.*/
	/* Read Tx AGC control bit*/
		Antenna.word = pAd->EEPROMDefaultValue[EEPROM_NIC_CFG1_OFFSET];





	// TODO: shiang, why we only check oxff00??
	if (((Antenna.word & 0xFF00) == 0xFF00) || IS_MT76x2(pAd))
/*	if (Antenna.word == 0xFFFF)*/
		RTMP_CHIP_ANTENNA_INFO_DEFAULT_RESET(pAd, &Antenna);

// TODO: shiang-7603
	if (IS_MT7603(pAd) || IS_MT7628(pAd))
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


	if (NicConfig2.field.DynamicTxAgcControl == 1) {
		pAd->bAutoTxAgcA = pAd->bAutoTxAgcG = TRUE;
	}
	else
		pAd->bAutoTxAgcA = pAd->bAutoTxAgcG = FALSE;

	/* Save value for future using */
	pAd->NicConfig2.word = NicConfig2.word;

	DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s(): RxPath = %d, TxPath = %d, RfIcType = %d\n",
					__FUNCTION__, Antenna.field.RxPath, Antenna.field.TxPath,
					Antenna.field.RfIcType));

	/* Save the antenna for future use*/
	pAd->Antenna.word = Antenna.word;

	/* Set the RfICType here, then we can initialize RFIC related operation callbacks*/
	pAd->Mlme.RealRxPath = (UCHAR) Antenna.field.RxPath;
	pAd->RfIcType = (UCHAR) Antenna.field.RfIcType;




#ifdef MT7603
	if (IS_MT7603(pAd))
		pAd->RfIcType = RFIC_7603;
#endif /* MT7603 */


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


	LoadTssiInfoFromEEPROM(pAd);

	pAd->BbpRssiToDbmDelta = 0x0;

	rtmp_read_freq_offset_from_eeprom(pAd);

	rtmp_read_country_region_from_eeporm(pAd);

	rtmp_read_rssi_langain_from_eeprom(pAd);

	rtmp_read_txmixer_gain_from_eeprom(pAd);

#ifdef LED_CONTROL_SUPPORT
	rtmp_read_led_setting_from_eeprom(pAd);
#endif /* LED_CONTROL_SUPPORT */

	rtmp_read_txpwr_from_eeprom(pAd);

#ifdef RTMP_EFUSE_SUPPORT
	RtmpEfuseSupportCheck(pAd);
#endif /* RTMP_EFUSE_SUPPORT */

#ifdef RTMP_INTERNAL_TX_ALC
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

	DBGPRINT(RT_DEBUG_TRACE, ("<--%s()\n", __FUNCTION__));

	return TRUE;
}


void rtmp_eeprom_of_platform(RTMP_ADAPTER *pAd)
{
#if defined(CONFIG_RT_FIRST_CARD_EEPROM) || defined(CONFIG_RT_SECOND_CARD_EEPROM)
	UCHAR e2p_dafault = 0;
#endif /* defined(CONFIG_RT_FIRST_CARD_EEPROM) || defined(CONFIG_RT_SECOND_CARD_EEPROM) */
#ifdef CONFIG_RT_FIRST_CARD_EEPROM
	if ( pAd->dev_idx == 0 )
	{
		if ( RTMPEqualMemory("efuse", CONFIG_RT_FIRST_CARD_EEPROM, 5) )
			e2p_dafault = E2P_EFUSE_MODE;
		if ( RTMPEqualMemory("prom", CONFIG_RT_FIRST_CARD_EEPROM, 4) )
			e2p_dafault = E2P_EEPROM_MODE;
		if ( RTMPEqualMemory("flash", CONFIG_RT_FIRST_CARD_EEPROM, 5) )
			e2p_dafault = E2P_FLASH_MODE;

		pAd->E2pAccessMode = e2p_dafault;
	}
#endif /* CONFIG_RT_FIRST_CARD_EEPROM */

#ifdef CONFIG_RT_SECOND_CARD_EEPROM
	if ( pAd->dev_idx == 1 )
	{
		if ( RTMPEqualMemory("efuse", CONFIG_RT_SECOND_CARD_EEPROM, 5) )
			e2p_dafault = E2P_EFUSE_MODE;
		if ( RTMPEqualMemory("prom", CONFIG_RT_SECOND_CARD_EEPROM, 4) )
			e2p_dafault = E2P_EEPROM_MODE;
		if ( RTMPEqualMemory("flash", CONFIG_RT_SECOND_CARD_EEPROM, 5) )
			e2p_dafault = E2P_FLASH_MODE;

		pAd->E2pAccessMode = e2p_dafault;
	}
#endif /* CONFIG_RT_SECOND_CARD_EEPROM */
}

UCHAR RtmpEepromGetDefault(RTMP_ADAPTER *pAd)
{
	UCHAR e2p_dafault = 0;

#ifdef RTMP_FLASH_SUPPORT
	if (pAd->infType == RTMP_DEV_INF_RBUS)
		e2p_dafault = E2P_FLASH_MODE;
	else
#endif /* RTMP_FLASH_SUPPORT */
	{
#ifdef RTMP_EFUSE_SUPPORT
		if (pAd->bUseEfuse)
			e2p_dafault = E2P_EFUSE_MODE;
		else
#endif /* RTMP_EFUSE_SUPPORT */
			e2p_dafault = E2P_EEPROM_MODE;
	}

	DBGPRINT(RT_DEBUG_OFF, ("%s::e2p_dafault=%d\n", __FUNCTION__, e2p_dafault));
	return e2p_dafault;
}


#if defined(RTMP_EFUSE_SUPPORT) && defined(RTMP_FLASH_SUPPORT)
static USHORT EE_FLASH_ID_LIST[]={





#ifdef MT7603
    0x7603,
#endif
};

#define EE_FLASH_ID_NUM  (sizeof(EE_FLASH_ID_LIST) / sizeof(USHORT))
#endif /* defined(RTMP_EFUSE_SUPPORT) && defined(RTMP_FLASH_SUPPORT) */




static NDIS_STATUS rtmp_ee_bin_init(PRTMP_ADAPTER pAd)
{
#ifdef CAL_FREE_IC_SUPPORT
	BOOLEAN bCalFree=0;
#endif /* CAL_FREE_IC_SUPPORT */

	rtmp_ee_load_from_bin(pAd);

#ifdef CAL_FREE_IC_SUPPORT
	RTMP_CAL_FREE_IC_CHECK(pAd,bCalFree);
	if (bCalFree)
	{
		DBGPRINT(RT_DEBUG_OFF, ("Cal Free IC!!\n"));
		RTMP_CAL_FREE_DATA_GET(pAd);
	}
	else
	{
		DBGPRINT(RT_DEBUG_OFF, ("Non Cal Free IC!!\n"));
	}
#endif /* CAL_FREE_IC_SUPPORT */

	return NDIS_STATUS_SUCCESS;
}

#if defined(RTMP_EFUSE_SUPPORT) && defined(RTMP_FLASH_SUPPORT)
static VOID RtmpEepromTypeAdjust(RTMP_ADAPTER *pAd, UCHAR *pE2pType)
{
	UINT EfuseFreeBlock=0;

	eFuseGetFreeBlockCount(pAd, &EfuseFreeBlock);	
	
	if (EfuseFreeBlock >= pAd->chipCap.EFUSE_RESERVED_SIZE)
	{
		DBGPRINT(RT_DEBUG_OFF, ("NVM is efuse and the information is too less to bring up the interface\n"));
		DBGPRINT(RT_DEBUG_OFF, ("Force to use Flash mode\n"));
		*pE2pType = E2P_FLASH_MODE;
	}
	else 
	{
		USHORT eeFlashId = 0;
		int listIdx;
		BOOLEAN bFound = FALSE;

		rtmp_ee_efuse_read16(pAd, 0, &eeFlashId);
		DBGPRINT(RT_DEBUG_OFF, ("%s:: eeFlashId = 0x%x.\n", __FUNCTION__, eeFlashId));
		for(listIdx =0 ; listIdx < EE_FLASH_ID_NUM; listIdx++)
		{
			if (eeFlashId == EE_FLASH_ID_LIST[listIdx])
			{			
				bFound = TRUE;
				break;
			}
		}

		if (bFound == FALSE)
		{
			*pE2pType = E2P_FLASH_MODE;
		}
	}
}
#endif /* defined(RTMP_EFUSE_SUPPORT) && defined(RTMP_FLASH_SUPPORT) */

INT RtmpChipOpsEepromHook(RTMP_ADAPTER *pAd, INT infType,INT forceMode)
{
	RTMP_CHIP_OP *pChipOps = &pAd->chipOps;
	UCHAR e2p_type;
#ifdef RTMP_PCI_SUPPORT
	UINT32 val;
#endif /* RTMP_PCI_SUPPORT */
	UCHAR e2p_default = 0;

	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
		return -1;

#ifdef RTMP_EFUSE_SUPPORT
	efuse_probe(pAd);
#endif /* RTMP_EFUSE_SUPPORT */

	rtmp_eeprom_of_platform(pAd);

	if(forceMode != E2P_NONE && forceMode < NUM_OF_E2P_MODE)
	{
		e2p_type = forceMode;
		DBGPRINT(RT_DEBUG_OFF, ("%s::forceMode: %d , infType: %d\n",
					__FUNCTION__, e2p_type, infType));
	}
	else
	{
		e2p_type = pAd->E2pAccessMode;

		DBGPRINT(RT_DEBUG_OFF, ("%s::e2p_type=%d, inf_Type=%d\n",
					__FUNCTION__, e2p_type, infType));

		e2p_default = RtmpEepromGetDefault(pAd);
		/* If e2p_type is out of range, get the default mode */
		e2p_type = ((e2p_type != 0) && (e2p_type < NUM_OF_E2P_MODE)) ? e2p_type : e2p_default;

		if (infType == RTMP_DEV_INF_RBUS)
		{
			e2p_type = E2P_FLASH_MODE;
			pChipOps->loadFirmware = NULL;
		}
#if defined(RTMP_EFUSE_SUPPORT) && defined(RTMP_FLASH_SUPPORT)
		else if (pAd->E2pAccessMode == E2P_NONE)
		{
			/*
				User doesn't set E2pAccessMode in profile, adjust access mode automatically here.
			*/
			RtmpEepromTypeAdjust(pAd, &e2p_type);
		}
#endif /* defined(RTMP_EFUSE_SUPPORT) && defined(RTMP_FLASH_SUPPORT) */

#ifdef RTMP_EFUSE_SUPPORT
		if (e2p_type != E2P_EFUSE_MODE)
			pAd->bUseEfuse = FALSE;
#endif /* RTMP_EFUSE_SUPPORT */

		DBGPRINT(RT_DEBUG_OFF, ("%s: E2P type(%d), E2pAccessMode = %d, E2P default = %d\n", __FUNCTION__, e2p_type, pAd->E2pAccessMode, e2p_default));
		pAd->eeprom_type = (e2p_type==E2P_EFUSE_MODE)  ? EEPROM_EFUSE: EEPROM_FLASH;
	}

	pAd->e2pCurMode = e2p_type;
	
	switch (e2p_type)
	{
		case E2P_EEPROM_MODE:
			break;
		case E2P_BIN_MODE:
		{
			pChipOps->eeinit = rtmp_ee_bin_init;
			pChipOps->eeread = rtmp_ee_bin_read16;
			pChipOps->eewrite = rtmp_ee_bin_write16;
			DBGPRINT(RT_DEBUG_OFF, ("NVM is BIN mode\n"));
			return 0;
		}

#ifdef RTMP_FLASH_SUPPORT
		case E2P_FLASH_MODE:
		{
			pChipOps->eeinit = rtmp_nv_init;
			pChipOps->eeread = rtmp_ee_flash_read;
			pChipOps->eewrite = rtmp_ee_flash_write;
			pAd->flash_offset = get_dev_eeprom_offset(pAd);
#ifdef CONFIG_RT_FIRST_CARD
			if ( pAd->dev_idx == 0 )
				pAd->flash_offset = CONFIG_RT_FIRST_IF_RF_OFFSET;
#endif /* CONFIG_RT_FIRST_CARD */
#ifdef CONFIG_RT_SECOND_CARD
			if ( pAd->dev_idx == 1 )
				pAd->flash_offset = CONFIG_RT_SECOND_IF_RF_OFFSET;
#endif /* CONFIG_RT_FIRST_CARD */
			DBGPRINT(RT_DEBUG_OFF, ("NVM is FLASH mode, flash_offset = 0x%x\n", pAd->flash_offset));
			return 0;
		}
#endif /* RTMP_FLASH_SUPPORT */

#ifdef RTMP_EFUSE_SUPPORT
		case E2P_EFUSE_MODE:
			if (pAd->bUseEfuse)
			{
				pChipOps->eeinit = eFuse_init;
				pChipOps->eeread = rtmp_ee_efuse_read16;
				pChipOps->eewrite = rtmp_ee_efuse_write16;
				DBGPRINT(RT_DEBUG_OFF, ("NVM is EFUSE mode\n"));
				return 0;
			}
			else
			{
				DBGPRINT(RT_DEBUG_ERROR, ("%s::hook efuse mode failed\n", __FUNCTION__));
				break;
			}
#endif /* RTMP_EFUSE_SUPPORT */
		default:
			DBGPRINT(RT_DEBUG_ERROR, ("%s: Do not support E2P type(%d), change to BIN mode\n", __FUNCTION__, e2p_type));

			pChipOps->eeinit = rtmp_ee_bin_init;
			pChipOps->eeread = rtmp_ee_bin_read16;
			pChipOps->eewrite = rtmp_ee_bin_write16;
			DBGPRINT(RT_DEBUG_OFF, ("NVM is BIN mode\n"));
			return 0;
	}

	/* Hook functions based on interface types for EEPROM */
	switch (infType)
	{
#ifdef RTMP_PCI_SUPPORT
		case RTMP_DEV_INF_PCI:
		case RTMP_DEV_INF_PCIE:
			RTMP_IO_READ32(pAd, E2PROM_CSR, &val);
			if (((val & 0x30) == 0) && (!IS_RT3290(pAd)))
				pAd->EEPROMAddressNum = 6; /* 93C46 */
			else
				pAd->EEPROMAddressNum = 8; /* 93C66 or 93C86 */

			pChipOps->eeinit = NULL;
			pChipOps->eeread = rtmp_ee_prom_read16;
			pChipOps->eewrite = rtmp_ee_prom_write16;
			break;
#endif /* RTMP_PCI_SUPPORT */


		default:
			DBGPRINT(RT_DEBUG_ERROR, ("%s::hook failed\n", __FUNCTION__));
			break;
	}

	DBGPRINT(RT_DEBUG_OFF, ("NVM is EEPROM mode\n"));
	return 0;
}


BOOLEAN rtmp_get_default_bin_file_by_chip(
	IN PRTMP_ADAPTER pAd,
	IN UINT32 	ChipVersion,
	OUT RTMP_STRING **pBinFileName)
{
	BOOLEAN found = FALSE;
	INT i;
	for (i = 0; RTMP_CHIP_E2P_FILE_TABLE[i].ChipVersion != 0; i++ )
	{
		if (RTMP_CHIP_E2P_FILE_TABLE[i].ChipVersion == ChipVersion)
		{
			*pBinFileName = RTMP_CHIP_E2P_FILE_TABLE[i].name;
			DBGPRINT(RT_DEBUG_OFF,
						("%s(): Found E2P bin file name:%s\n",
						__FUNCTION__, *pBinFileName));
			found = TRUE;
			break;
		}
	}

	if (found == TRUE)
		DBGPRINT(RT_DEBUG_OFF, ("%s::Found E2P bin file name=%s\n", __FUNCTION__, *pBinFileName));
	else
		DBGPRINT(RT_DEBUG_ERROR, ("%s::E2P bin file name not found\n", __FUNCTION__));

	return found;
}


BOOLEAN rtmp_ee_bin_read16(RTMP_ADAPTER *pAd, UINT16 Offset, UINT16 *pValue)
{
	BOOLEAN IsEmpty = 0;

	DBGPRINT(RT_DEBUG_INFO, ("%s::Read from EEPROM buffer\n", __FUNCTION__));
	NdisMoveMemory(pValue, &(pAd->EEPROMImage[Offset]), 2);
	*pValue = le2cpu16(*pValue);

	if ((*pValue == 0xffff) || (*pValue == 0x0000))
		IsEmpty = 1;

	return IsEmpty;
}


INT rtmp_ee_bin_write16(
	IN RTMP_ADAPTER 	*pAd,
	IN USHORT 			Offset,
	IN USHORT 			data)
{
	DBGPRINT(RT_DEBUG_INFO, ("%s::Write to EEPROM buffer\n", __FUNCTION__));
	data = le2cpu16(data);
	NdisMoveMemory(&(pAd->EEPROMImage[Offset]), &data, 2);

	return 0;
}


INT rtmp_ee_load_from_bin(
	IN PRTMP_ADAPTER 	pAd)
{
	RTMP_STRING *src = NULL;
	INT ret_val;
	RTMP_OS_FD srcf;
	RTMP_OS_FS_INFO osFSInfo;

#ifdef RT_SOC_SUPPORT
#ifdef MULTIPLE_CARD_SUPPORT
	RTMP_STRING bin_file_path[128];
	RTMP_STRING *bin_file_name = NULL;
	UINT32 chip_ver = (pAd->MACVersion >> 16);

	if (rtmp_get_default_bin_file_by_chip(pAd, chip_ver, &bin_file_name) == TRUE)
	{
		if (pAd->MC_RowID > 0)
			sprintf(bin_file_path, "%s%s", EEPROM_2ND_FILE_DIR, bin_file_name);
		else
			sprintf(bin_file_path, "%s%s", EEPROM_1ST_FILE_DIR, bin_file_name);

		src = bin_file_path;
	}
	else
#endif /* MULTIPLE_CARD_SUPPORT */
#endif /* RT_SOC_SUPPORT */
		src = BIN_FILE_PATH;

	DBGPRINT(RT_DEBUG_TRACE, ("%s::FileName=%s\n", __FUNCTION__, src));

	RtmpOSFSInfoChange(&osFSInfo, TRUE);

	if (src && *src)
	{
		srcf = RtmpOSFileOpen(src, O_RDONLY, 0);
		if (IS_FILE_OPEN_ERR(srcf))
		{
			DBGPRINT(RT_DEBUG_ERROR, ("%s::Error opening %s\n", __FUNCTION__, src));
			goto error;
		}
		else
		{
			NdisZeroMemory(pAd->EEPROMImage, MAX_EEPROM_BIN_FILE_SIZE);
			ret_val = RtmpOSFileRead(srcf, (RTMP_STRING *)pAd->EEPROMImage, MAX_EEPROM_BIN_FILE_SIZE);

			if (ret_val > 0)
				ret_val = NDIS_STATUS_SUCCESS;
			else
				DBGPRINT(RT_DEBUG_ERROR, ("%s::Read file \"%s\" failed(errCode=%d)!\n", __FUNCTION__, src, ret_val));
      		}
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s::Error src or srcf is null\n", __FUNCTION__));
		goto error;
	}

	ret_val = RtmpOSFileClose(srcf);

	if (ret_val)
		DBGPRINT(RT_DEBUG_ERROR, ("%s::Error %d closing %s\n", __FUNCTION__, -ret_val, src));

	RtmpOSFSInfoChange(&osFSInfo, FALSE);
	return TRUE;

error:
	if (pAd->chipCap.EEPROM_DEFAULT_BIN != NULL)
	{
		NdisMoveMemory(pAd->EEPROMImage, pAd->chipCap.EEPROM_DEFAULT_BIN,
		pAd->chipCap.EEPROM_DEFAULT_BIN_SIZE > MAX_EEPROM_BUFFER_SIZE?MAX_EEPROM_BUFFER_SIZE:pAd->chipCap.EEPROM_DEFAULT_BIN_SIZE);
		DBGPRINT(RT_DEBUG_TRACE, ("Load EEPROM Buffer from default BIN.\n"));
	}

	return FALSE;
}


INT rtmp_ee_write_to_bin(
	IN PRTMP_ADAPTER 	pAd)
{
	RTMP_STRING *src = NULL;
	INT ret_val;
	RTMP_OS_FD srcf;
	RTMP_OS_FS_INFO osFSInfo;
	const INT PCI_EFUSE_SIZE = 10;
	UCHAR pci_pm[PCI_EFUSE_SIZE];
	UCHAR pci_efuse[PCI_EFUSE_SIZE];
	INT i = 0;

#ifdef RT_SOC_SUPPORT
#ifdef MULTIPLE_CARD_SUPPORT
	RTMP_STRING bin_file_path[128];
	RTMP_STRING *bin_file_name = NULL;
	UINT32 chip_ver = (pAd->MACVersion >> 16);

	if (rtmp_get_default_bin_file_by_chip(pAd, chip_ver, &bin_file_name) == TRUE)
	{
		if (pAd->MC_RowID > 0)
			sprintf(bin_file_path, "%s%s", EEPROM_2ND_FILE_DIR, bin_file_name);
		else
			sprintf(bin_file_path, "%s%s", EEPROM_1ST_FILE_DIR, bin_file_name);

		src = bin_file_path;
	}
	else
#endif /* MULTIPLE_CARD_SUPPORT */
#endif /* RT_SOC_SUPPORT */
		src = BIN_FILE_PATH;

	DBGPRINT(RT_DEBUG_TRACE, ("%s::FileName=%s\n", __FUNCTION__, src));

	RtmpOSFSInfoChange(&osFSInfo, TRUE);

	if (src && *src)
	{
		srcf = RtmpOSFileOpen(src, O_WRONLY|O_CREAT, 0);

		if (IS_FILE_OPEN_ERR(srcf))
		{
			DBGPRINT(RT_DEBUG_ERROR, ("%s::Error opening %s\n", __FUNCTION__, src));
			return FALSE;
		}
		else
			RtmpOSFileWrite(srcf, (RTMP_STRING *)pAd->EEPROMImage, MAX_EEPROM_BIN_FILE_SIZE);
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s::Error src or srcf is null\n", __FUNCTION__));
		return FALSE;
	}

	ret_val = RtmpOSFileClose(srcf);

	if (ret_val)
		DBGPRINT(RT_DEBUG_ERROR, ("%s::Error %d closing %s\n", __FUNCTION__, -ret_val, src));

	RtmpOSFSInfoChange(&osFSInfo, FALSE);
#ifdef RTMP_MAC_PCI
#ifdef MT7603
	NdisZeroMemory(pci_pm, PCI_EFUSE_SIZE);
	NdisZeroMemory(pci_efuse, PCI_EFUSE_SIZE);
	/* 0x26 ~ 0x2F */
	for(i=0;i<PCI_EFUSE_SIZE;i+=2){
		UINT16 buf_reg = 0;
		buf_reg |= pAd->EEPROMImage[0x26+i+1];
		buf_reg <<= 8;
		buf_reg |= pAd->EEPROMImage[0x26+i];
		buf_reg = le2cpu16 (buf_reg);
      	NdisMoveMemory(&pci_pm[i], &buf_reg, 2);
	}
	eFuseRead(pAd, 0x26, (USHORT *)pci_efuse, PCI_EFUSE_SIZE);

	DBGPRINT(RT_DEBUG_TRACE, ("PCIE_PM: "));
	for(i=0;i<PCI_EFUSE_SIZE;i++){
		DBGPRINT(RT_DEBUG_TRACE, ("%x ",pci_pm[i]));
	}
	DBGPRINT(RT_DEBUG_TRACE, ("\n"));
	DBGPRINT(RT_DEBUG_TRACE, ("PCIE_EFUSE: "));
	for(i=0;i<PCI_EFUSE_SIZE;i++){
		DBGPRINT(RT_DEBUG_TRACE, ("%x ",pci_efuse[i]));
	}
	DBGPRINT(RT_DEBUG_TRACE, ("\n"));
	if (!NdisEqualMemory(pci_pm, pci_efuse, PCI_EFUSE_SIZE)){
		eFuseWrite(pAd, 0x26, (USHORT *)pci_pm, PCI_EFUSE_SIZE);
		DBGPRINT(RT_DEBUG_TRACE, ("%s, write pci efuse val  to efuse\n", __FUNCTION__));
	}else{
		DBGPRINT(RT_DEBUG_TRACE, ("%s, 0x26~0x2F efuse and bin file has same val\n", __FUNCTION__));
	}
#endif	/* MT7603 */
#endif /* RTMP_MAC_PCI */
	return TRUE;
}


INT Set_LoadEepromBufferFromBin_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT bEnable = simple_strtol(arg, 0, 10);
	INT result;

	if (bEnable < 0)
		return FALSE;
	else
	{
		DBGPRINT(RT_DEBUG_TRACE, ("Load EEPROM buffer from BIN, and change to BIN buffer mode\n"));
		result = rtmp_ee_load_from_bin(pAd);

		if ( result == FALSE )
		{
			if ( pAd->chipCap.EEPROM_DEFAULT_BIN != NULL )
			{
				NdisMoveMemory(pAd->EEPROMImage, pAd->chipCap.EEPROM_DEFAULT_BIN,
					pAd->chipCap.EEPROM_DEFAULT_BIN_SIZE > MAX_EEPROM_BUFFER_SIZE?MAX_EEPROM_BUFFER_SIZE:pAd->chipCap.EEPROM_DEFAULT_BIN_SIZE);
				DBGPRINT(RT_DEBUG_TRACE, ("Load EEPROM Buffer from default BIN.\n"));
			}

		}

		/* Change to BIN eeprom buffer mode */
		RtmpChipOpsEepromHook(pAd, pAd->infType,E2P_BIN_MODE);
		return TRUE;
	}
}


INT Set_EepromBufferWriteBack_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT e2p_mode = simple_strtol(arg, 0, 10);

	if (e2p_mode >= NUM_OF_E2P_MODE)
		return FALSE;

	switch (e2p_mode)
	{
#ifdef RTMP_EFUSE_SUPPORT
		case E2P_EFUSE_MODE:
			DBGPRINT(RT_DEBUG_OFF, ("Write EEPROM buffer back to eFuse\n"));
			rtmp_ee_write_to_efuse(pAd);
			break;
#endif /* RTMP_EFUSE_SUPPORT */

#ifdef RTMP_FLASH_SUPPORT
		case E2P_FLASH_MODE:
			DBGPRINT(RT_DEBUG_OFF, ("Write EEPROM buffer back to Flash\n"));
			rtmp_ee_flash_write_all(pAd, (PUSHORT)pAd->EEPROMImage);
			break;
#endif /* RTMP_FLASH_SUPPORT */

		case E2P_EEPROM_MODE:
			DBGPRINT(RT_DEBUG_OFF, ("Write EEPROM buffer back to EEPROM\n"));
			rtmp_ee_write_to_prom(pAd);
			break;

		case E2P_BIN_MODE:
			DBGPRINT(RT_DEBUG_OFF, ("Write EEPROM buffer back to BIN\n"));
			rtmp_ee_write_to_bin(pAd);
			break;

		default:
			DBGPRINT(RT_DEBUG_ERROR, ("%s::do not support this EEPROM access mode\n", __FUNCTION__));
			return FALSE;
	}

	return TRUE;
}


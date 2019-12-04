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


#ifdef CONFIG_RT_FIRST_CARD
#define FIRST_EEPROM_FILE_PATH	"/etc_ro/Wireless/RT2860/MT7615_EEPROM_1.bin"
#endif

#ifdef CONFIG_RT_SECOND_CARD
#define SECOND_EEPROM_FILE_PATH	"/etc_ro/Wireless/iNIC/MT7615_EEPROM_2.bin"
#endif

#if defined(PRE_CAL_TRX_SET1_SUPPORT) || defined(PRE_CAL_TRX_SET2_SUPPORT) || defined(RLM_CAL_CACHE_SUPPORT)
#ifdef CONFIG_RT_FIRST_CARD
#define FIRST_CAL_FILE_PATH	"/etc_ro/Wireless/RT2860/CALDATA1.bin"
#endif

#ifdef CONFIG_RT_SECOND_CARD
#define SECOND_CAL_FILE_PATH	"/etc_ro/Wireless/iNIC/CALDATA2.bin"
#endif
#endif /* PRE_CAL_TRX_SET1_SUPPORT */


struct chip_map {
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


struct chip_map chip_card_id_map[] = {
	{7620, ""},
};


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
		RT28xx_EEPROM_READ16(pAd, EEPROM_RSSI_BG_OFFSET + 2, value);
		{
			pAd->BGRssiOffset[2] = value & 0x00ff;
			pAd->ALNAGain1 = (value >> 8);
		}
	}
	{
		RT28xx_EEPROM_READ16(pAd, EEPROM_LNA_OFFSET, value);
		pAd->BLNAGain = value & 0x00ff;
		/* External LNA gain for 5GHz Band(CH36~CH64) */
		pAd->ALNAGain0 = (value >> 8);
		RT28xx_EEPROM_READ16(pAd, EEPROM_RSSI_A_OFFSET, value);
		pAd->ARssiOffset[0] = value & 0x00ff;
		pAd->ARssiOffset[1] = (value >> 8);
	}
	{
		RT28xx_EEPROM_READ16(pAd, (EEPROM_RSSI_A_OFFSET + 2), value);
		{
			pAd->ARssiOffset[2] = value & 0x00ff;
			pAd->ALNAGain2 = (value >> 8);
		}
	}

	if (((UCHAR)pAd->ALNAGain1 == 0xFF) || (pAd->ALNAGain1 == 0x00))
		pAd->ALNAGain1 = pAd->ALNAGain0;

	if (((UCHAR)pAd->ALNAGain2 == 0xFF) || (pAd->ALNAGain2 == 0x00))
		pAd->ALNAGain2 = pAd->ALNAGain0;

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ALNAGain0 = %d, ALNAGain1 = %d, ALNAGain2 = %d\n",
			 pAd->ALNAGain0, pAd->ALNAGain1, pAd->ALNAGain2));

	/* Validate 11a/b/g RSSI 0/1/2 offset.*/
	for (i = 0; i < 3; i++) {
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

	if ((value <= REGION_MAXIMUM_BG_BAND) || (value == REGION_31_BG_BAND) || (value == REGION_32_BG_BAND) ||
		(value == REGION_33_BG_BAND))
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
#ifdef RTMP_RBUS_SUPPORT

	if (pAd->infType == RTMP_DEV_INF_RBUS) {
		if (pAd->RfFreqDelta & 0x10)
			pAd->RfFreqOffset = (pAd->RfFreqOffset >= pAd->RfFreqDelta) ? (pAd->RfFreqOffset - (pAd->RfFreqDelta & 0xf)) : 0;
		else
			pAd->RfFreqOffset = ((pAd->RfFreqOffset + pAd->RfFreqDelta) < 0x40) ? (pAd->RfFreqOffset +
								(pAd->RfFreqDelta & 0xf)) : 0x3f;
	}

#endif /* RTMP_RBUS_SUPPORT */
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("E2PROM: RF FreqOffset=0x%x\n", pAd->RfFreqOffset));
	return TRUE;
}


INT rtmp_read_txpwr_from_eeprom(RTMP_ADAPTER *pAd)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	/* if not return early. cause fail at emulation.*/
	/* Init the channel number for TX channel power*/
	if (ops->read_chl_pwr)
		ops->read_chl_pwr(pAd);

	RTMPReadTxPwrPerRate(pAd);
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
	USHORT  Addr01, Addr23, Addr45;
#if defined(PRE_CAL_TRX_SET2_SUPPORT) || defined(PRE_CAL_MT7622_SUPPORT)
	UINT16 DoPreCal = 0;
#endif /* PRE_CAL_TRX_SET2_SUPPORT */
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

#ifdef PRE_CAL_MT7622_SUPPORT
	UINT32 ch;
#endif /* PRE_CAL_MT7622_SUPPORT */

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s()-->\n", __func__));

	if (ops->eeinit) {
#if defined(WCX_SUPPORT)
		/* do nothing */
#else
		/* If we are run in Multicard mode */
		ops->eeinit(pAd);
#endif /*WCX_SUPPORT */
#ifdef RF_LOCKDOWN

		/* Merge RF parameters in Effuse to E2p buffer */
		if (ops->merge_RF_lock_parameter != NULL)
			ops->merge_RF_lock_parameter(pAd);

		/* Replace Country code and Country Region in Profile by Effuse content */
		if (ops->Config_Effuse_Country != NULL)
			ops->Config_Effuse_Country(pAd);

#endif /* RF_LOCKDOWN */
	}

#ifdef PRE_CAL_MT7622_SUPPORT
	/* Check DoPreCal bits */
	RT28xx_EEPROM_READ16(pAd, 0x52, DoPreCal);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("\x1b[34m%s: EEPROM 0x52 %x\x1b[m\n", __func__, DoPreCal));
	if (pAd->E2pAccessMode == E2P_FLASH_MODE) {
		if (DoPreCal & (1 << 3)) {
			MtCmdSetTxLpfCal_7622(pAd);
			MtCmdSetTxDcIqCal_7622(pAd);
			for (ch = 1; ch < 14; ch++)
				MtCmdSetTxDpdCal_7622(pAd, ch);
		}
	}
#endif /*PRE_CAL_MT7622_SUPPORT*/

#ifdef PRE_CAL_TRX_SET2_SUPPORT
	/* Check DoPreCal bits */
	RT28xx_EEPROM_READ16(pAd, 0x52, DoPreCal);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\x1b[34m%s: EEPROM 0x52 %x\x1b[m\n", __FUNCTION__, DoPreCal));

	/* Pre Cal only supports Bin or Flash Mode */
	if (pAd->E2pAccessMode == E2P_FLASH_MODE || pAd->E2pAccessMode == E2P_BIN_MODE) {
	/* Restore when RLM cache is empty */
		if (!rlmCalCacheDone(pAd->rlmCalCache) && (DoPreCal & (1 << 2))) {
			INT32 ret = 0;
			INT32 ret_cal_data = NDIS_STATUS_SUCCESS;
			ret = os_alloc_mem(pAd, &pAd->PreCalReStoreBuffer, PRE_CAL_SIZE);/* Allocate 16K buffer*/

			if (ret != NDIS_STATUS_SUCCESS) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("\x1b[42m %s : Not enough memory for pre-cal restored buffer!!\x1b[m\n", __FUNCTION__));
			} else {
				/* Read pre-cal data from flash and store to pre-cal buffer*/
#ifdef RTMP_FLASH_SUPPORT
				if (pAd->E2pAccessMode == E2P_FLASH_MODE)
					RtmpFlashRead(pAd->hdev_ctrl, pAd->PreCalReStoreBuffer,
									get_dev_eeprom_offset(pAd) + PRECALPART_OFFSET, PRE_CAL_SIZE);
#endif /* RTMP_FLASH_SUPPORT */
				if (pAd->E2pAccessMode == E2P_BIN_MODE) {
					ret_cal_data = rtmp_cal_load_from_bin(pAd, pAd->PreCalReStoreBuffer, PRECALPART_OFFSET,
										 PRE_CAL_SIZE);

					if (ret_cal_data != NDIS_STATUS_SUCCESS) {
						/* Erase DoPreCal bit */
						DoPreCal &= ~(1 << 2);
						RT28xx_EEPROM_WRITE16(pAd, 0x52, DoPreCal);

						MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						("Reset EEPROM 0x52[2] bit, Let FW do On-line Calibration\n"));
					}
				}

				if (ret_cal_data == NDIS_STATUS_SUCCESS)
					ret = MtCmdPreCalReStoreProc(pAd, (INT32 *)pAd->PreCalReStoreBuffer);
			}
		}
	} else {
		/* Force Erase DoPreCal bit for any mode other than Bin & Flash*/
		DoPreCal &= ~(1 << 2);
		RT28xx_EEPROM_WRITE16(pAd, 0x52, DoPreCal);
	}
#endif /* PRE_CAL_TRX_SET2_SUPPORT */
#ifdef MT_DFS_SUPPORT	/*Dynamically enable or disable DFS calibration in firmware. Must be performed before power on calibration*/
	DfsSetCalibration(pAd, pAd->CommonCfg.DfsParameter.DisableDfsCal);
#endif
#ifdef MT_MAC
#if defined(MT7603) || defined(MT7628) || defined(MT7636) || defined(MT7615) || defined(MT7637) || defined(MT7622)
	/*Send EEprom parameter to FW*/
#ifdef CONFIG_ATE

	if (!ATE_ON(pAd))
#endif
	{
		/*configure PA/LNA setting by efuse*/
		chip_pa_lna_set(pAd, pAd->EEPROMImage);
		/*configure TSSI sestting by efuse*/
		chip_tssi_set(pAd, pAd->EEPROMImage);
		if (IS_MT7603(pAd) || IS_MT7628(pAd) || IS_MT7636(pAd) || IS_MT7615(pAd) || IS_MT7637(pAd) || IS_MT7622(pAd))
			MtCmdEfusBufferModeSet(pAd, pAd->eeprom_type);
	}

#endif /*#if defined(MT7603)||defined(MT7628)||defined(MT7636))||defined(MT7615)||defined(MT7622)*/
#endif /* MT_MAC */
	/* Read MAC setting from EEPROM and record as permanent MAC address */
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Initialize MAC Address from E2PROM\n"));
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
		pAd->PermanentAddress[0] = RandomByte(pAd) & 0xf8;

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("E2PROM MAC: =%02x:%02x:%02x:%02x:%02x:%02x\n",
			 PRINT_MAC(pAd->PermanentAddress)));

	/* Assign the actually working MAC Address */
	if (pAd->bLocalAdminMAC) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("Use the MAC address what is assigned from Configuration file(.dat).\n"));
#if defined(BB_SOC) && !defined(NEW_MBSSID_MODE)
		/* BBUPrepareMAC(pAd, pAd->CurrentAddress); */
		COPY_MAC_ADDR(pAd->PermanentAddress, pAd->CurrentAddress);
		printk("now bb MainSsid mac %02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(pAd->CurrentAddress));
#endif
	} else if (mac_addr &&
			   strlen((RTMP_STRING *)mac_addr) == 17 &&
			   (strcmp(mac_addr, "00:00:00:00:00:00") != 0)) {
		INT j;
		RTMP_STRING *macptr;
		macptr = (RTMP_STRING *) mac_addr;

		for (j = 0; j < MAC_ADDR_LEN; j++) {
			AtoH(macptr, &pAd->CurrentAddress[j], 1);
			macptr = macptr + 3;
		}

		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Use the MAC address what is assigned from Moudle Parameter.\n"));
	} else {
		COPY_MAC_ADDR(pAd->CurrentAddress, pAd->PermanentAddress);
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Use the MAC address what is assigned from EEPROM.\n"));
	}

	/* if E2PROM version mismatch with driver's expectation, then skip*/
	/* all subsequent E2RPOM retieval and set a system error bit to notify GUI*/
	RT28xx_EEPROM_READ16(pAd, EEPROM_VERSION_OFFSET, Version.word);
	pAd->EepromVersion = Version.field.Version + Version.field.FaeReleaseNumber * 256;
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("E2PROM: Version = %d, FAE release #%d\n", Version.field.Version,
			 Version.field.FaeReleaseNumber));
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
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Country Region from e2p = %x\n", value));
	}

	for (i = 0; i < 8; i++) {
#if defined(RLT_MAC) || defined(MT_MAC)

		/* TODO: shiang-MT7603, replace below chip based check to this one!! */
		if (IS_HIF_TYPE(pAd, HIF_RLT) || IS_HIF_TYPE(pAd, HIF_MT))
			break;

#endif /* defined(RLT_MAC) || defined(MT_MAC) */
		RT28xx_EEPROM_READ16(pAd, EEPROM_BBP_BASE_OFFSET + i * 2, value);
		pAd->EEPROMDefaultValue[i + EEPROM_BBP_ARRAY_OFFSET] = value;
	}

	/* We have to parse NIC configuration 0 at here. */
	/* If TSSI did not have preloaded value, it should reset the TxAutoAgc to false */
	/* Therefore, we have to read TxAutoAgc control beforehand. */
	/* Read Tx AGC control bit */
	Antenna.word = pAd->EEPROMDefaultValue[EEPROM_NIC_CFG1_OFFSET];

	/* TODO: shiang, why we only check oxff00?? */
	if (((Antenna.word & 0xFF00) == 0xFF00))
		/*	if (Antenna.word == 0xFFFF)*/
		RTMP_CHIP_ANTENNA_INFO_DEFAULT_RESET(pAd, &Antenna);

	/* TODO: shiang-7603 */
	/* TODO: shiang-MT7615 */
	if (IS_MT7603(pAd) || IS_MT7628(pAd) || IS_MT76x6(pAd) || IS_MT7615(pAd) || IS_MT7637(pAd) || IS_MT7622(pAd) ||
		IS_P18(pAd) || IS_MT7663(pAd))
		RTMP_CHIP_ANTENNA_INFO_DEFAULT_RESET(pAd, &Antenna);

#ifdef WSC_INCLUDED

	/* WSC hardware push button function 0811 */
#ifdef VENDOR_FEATURE7_SUPPORT
	WSC_HDR_BTN_MR_HDR_SUPPORT_SET(pAd, 1);		/* always enable PBC support */
#else
	if ((pAd->MACVersion == 0x28600100) || (pAd->MACVersion == 0x28700100))
		WSC_HDR_BTN_MR_HDR_SUPPORT_SET(pAd, NicConfig2.field.EnableWPSPBC);
#endif
#endif /* WSC_INCLUDED */
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		if (NicConfig2.word == 0xffff)
			NicConfig2.word = 0;
	}
#endif /* CONFIG_AP_SUPPORT */

	if (NicConfig2.field.DynamicTxAgcControl == 1)
		pAd->bAutoTxAgcA = pAd->bAutoTxAgcG = TRUE;
	else
		pAd->bAutoTxAgcA = pAd->bAutoTxAgcG = FALSE;

	/* Save value for future using */
	pAd->NicConfig2.word = NicConfig2.word;
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): RxPath = %d, TxPath = %d, RfIcType = %d\n",
			 __func__, Antenna.field.RxPath, Antenna.field.TxPath,
			 Antenna.field.RfIcType));
	/* Save the antenna for future use*/
	pAd->Antenna.word = Antenna.word;
	/* Set the RfICType here, then we can initialize RFIC related operation callbacks*/
	pAd->Mlme.RealRxPath = (UCHAR) Antenna.field.RxPath;
	pAd->RfIcType = (UCHAR) Antenna.field.RfIcType;
#ifdef MT7615

	/* TODO: shiang-MT7615 */
	if (IS_MT7615(pAd)) {
		POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;

		if (pObj->DeviceID == NIC7611_PCIe_DEVICE_ID)
			pAd->RfIcType = RFIC_7611;
		else if (pObj->DeviceID == NIC761A_PCIe_DEVICE_ID)
			pAd->RfIcType = RFIC_7615A;
		else
			pAd->RfIcType = RFIC_7615;
	}

#endif /* MT7615 */
#ifdef MT7622

	if (IS_MT7622(pAd))
		pAd->RfIcType = RFIC_7622;

#endif /* MT7622 */

	/* TODO: shiang-MT7615, how about the EEPROM value of MT series chip?? */
	if (!IS_MT7615(pAd) && !IS_MT7622(pAd) && !IS_P18(pAd) && !IS_MT7663(pAd)) {
		LoadTssiInfoFromEEPROM(pAd);
		pAd->BbpRssiToDbmDelta = 0x0;
		rtmp_read_freq_offset_from_eeprom(pAd);
		rtmp_read_country_region_from_eeporm(pAd);
		rtmp_read_rssi_langain_from_eeprom(pAd);
	}

#ifdef LED_CONTROL_SUPPORT
	rtmp_read_led_setting_from_eeprom(pAd);
#endif /* LED_CONTROL_SUPPORT */
	rtmp_read_txpwr_from_eeprom(pAd);
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: pAd->Antenna.field.BoardType = %d\n",
			 __func__,
			 pAd->Antenna.field.BoardType));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("<--%s()\n", __func__));
	return TRUE;
}


void rtmp_eeprom_of_platform(RTMP_ADAPTER *pAd)
{
	UCHAR e2p_dafault;
	e2p_dafault = E2P_NONE;
#ifdef CONFIG_RT_FIRST_CARD_EEPROM

	if (pAd->dev_idx == 0) {
		if (RTMPEqualMemory("efuse", CONFIG_RT_FIRST_CARD_EEPROM, 5))
			e2p_dafault = E2P_EFUSE_MODE;

		if (RTMPEqualMemory("prom", CONFIG_RT_FIRST_CARD_EEPROM, 4))
			e2p_dafault = E2P_EEPROM_MODE;

		if (RTMPEqualMemory("flash", CONFIG_RT_FIRST_CARD_EEPROM, 5))
			e2p_dafault = E2P_FLASH_MODE;

		pAd->E2pAccessMode = e2p_dafault;
	}

#endif /* CONFIG_RT_FIRST_CARD_EEPROM */
#ifdef CONFIG_RT_SECOND_CARD_EEPROM

	if (pAd->dev_idx == 1) {
		if (RTMPEqualMemory("efuse", CONFIG_RT_SECOND_CARD_EEPROM, 5))
			e2p_dafault = E2P_EFUSE_MODE;

		if (RTMPEqualMemory("prom", CONFIG_RT_SECOND_CARD_EEPROM, 4))
			e2p_dafault = E2P_EEPROM_MODE;

		if (RTMPEqualMemory("flash", CONFIG_RT_SECOND_CARD_EEPROM, 5))
			e2p_dafault = E2P_FLASH_MODE;

		pAd->E2pAccessMode = e2p_dafault;
	}

#endif /* CONFIG_RT_SECOND_CARD_EEPROM */
#ifdef CONFIG_RT_THIRD_CARD_EEPROM

	if (pAd->dev_idx == 2) {
		if (RTMPEqualMemory("efuse", CONFIG_RT_THIRD_CARD_EEPROM, 5))
			e2p_dafault = E2P_EFUSE_MODE;

		if (RTMPEqualMemory("prom", CONFIG_RT_THIRD_CARD_EEPROM, 4))
			e2p_dafault = E2P_EEPROM_MODE;

		if (RTMPEqualMemory("flash", CONFIG_RT_THIRD_CARD_EEPROM, 5))
			e2p_dafault = E2P_FLASH_MODE;

		pAd->E2pAccessMode = e2p_dafault;
	}

#endif /* CONFIG_RT_THIRD_CARD_EEPROM */
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

	/* SDIO load from NVRAM(BIN FILE) */
	if (pAd->infType == RTMP_DEV_INF_SDIO)
		e2p_dafault = E2P_BIN_MODE;

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s::e2p_dafault=%d\n", __func__, e2p_dafault));
	return e2p_dafault;
}


static NDIS_STATUS rtmp_ee_bin_init(PRTMP_ADAPTER pAd)
{
#ifdef CAL_FREE_IC_SUPPORT
	BOOLEAN bCalFree = 0;
#endif /* CAL_FREE_IC_SUPPORT */
	rtmp_ee_load_from_bin(pAd);
#ifdef PRE_CAL_TRX_SET1_SUPPORT
	{
		os_alloc_mem(pAd, &pAd->CalDCOCImage, DCOC_IMAGE_SIZE);
		os_alloc_mem(pAd, &pAd->CalDPDAPart1Image, TXDPD_IMAGE1_SIZE);
		os_alloc_mem(pAd, &pAd->CalDPDAPart2Image, TXDPD_IMAGE2_SIZE);
		NdisZeroMemory(pAd->CalDCOCImage, DCOC_IMAGE_SIZE);
		NdisZeroMemory(pAd->CalDPDAPart1Image, TXDPD_IMAGE1_SIZE);
		NdisZeroMemory(pAd->CalDPDAPart2Image, TXDPD_IMAGE2_SIZE);

		if (NDIS_STATUS_SUCCESS == rtmp_cal_load_from_bin(pAd, pAd->CalDCOCImage, 0, DCOC_IMAGE_SIZE))
			pAd->bDCOCReloaded = TRUE;
		else
			printk("%s(): bDCOCReloaded = false.\n", __func__);

		if ((NDIS_STATUS_SUCCESS ==
				rtmp_cal_load_from_bin(pAd, pAd->CalDPDAPart1Image, DPDPART1_OFFSET, TXDPD_IMAGE1_SIZE)) ||
			(NDIS_STATUS_SUCCESS ==
				rtmp_cal_load_from_bin(pAd, pAd->CalDPDAPart2Image, DPDPART2_OFFSET, TXDPD_IMAGE2_SIZE)))
			pAd->bDPDReloaded = TRUE;
		else
			printk("%s(): bDPDReloaded = false.\n", __func__);
	}
#endif /* PRE_CAL_TRX_SET1_SUPPORT */
#ifdef CAL_FREE_IC_SUPPORT
	RTMP_CAL_FREE_IC_CHECK(pAd, bCalFree);

	if (bCalFree) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Cal Free IC!!\n"));
		RTMP_CAL_FREE_DATA_GET(pAd);
	} else
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Non Cal Free IC!!\n"));

#endif /* CAL_FREE_IC_SUPPORT */
	return NDIS_STATUS_SUCCESS;
}



INT RtmpChipOpsEepromHook(RTMP_ADAPTER *pAd, INT infType, INT forceMode)
{
	RTMP_CHIP_OP *pChipOps = hc_get_chip_ops(pAd->hdev_ctrl);
	UCHAR e2p_type;
#ifdef RTMP_PCI_SUPPORT
	UINT32 val;
#endif /* RTMP_PCI_SUPPORT */
	UCHAR e2p_default = 0;
	EEPROM_CONTROL *pE2pCtrl = &pAd->E2pCtrl;

	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
		return -1;

#ifdef RTMP_EFUSE_SUPPORT
	efuse_probe(pAd);
#endif /* RTMP_EFUSE_SUPPORT */

	/* rtmp_eeprom_of_platform(pAd);  //for MT7615, only use E2pAccessMode parameter to get eeprom type */

	if (forceMode != E2P_NONE && forceMode < NUM_OF_E2P_MODE) {
		e2p_type = forceMode;
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s::forceMode: %d , infType: %d\n",
				 __func__, e2p_type, infType));
	} else {
		e2p_type = pAd->E2pAccessMode;
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s::e2p_type=%d, inf_Type=%d\n",
				 __func__, e2p_type, infType));
		e2p_default = RtmpEepromGetDefault(pAd);
		/* If e2p_type is out of range, get the default mode */
		e2p_type = ((e2p_type != 0) && (e2p_type < NUM_OF_E2P_MODE)) ? e2p_type : e2p_default;

		if (pAd->E2pAccessMode == E2P_NONE)
			pAd->E2pAccessMode = e2p_default;

		if (infType == RTMP_DEV_INF_RBUS) {
			e2p_type = E2P_FLASH_MODE;
		}

#ifdef RTMP_EFUSE_SUPPORT

		if (e2p_type != E2P_EFUSE_MODE)
			pAd->bUseEfuse = FALSE;

#endif /* RTMP_EFUSE_SUPPORT */
#if defined(MT7636_FPGA) || defined(MT7637_FPGA)
		e2p_type = E2P_BIN_MODE;
#endif /* RTMP_EFUSE_SUPPORT */
	}

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: E2P type(%d), E2pAccessMode = %d, E2P default = %d\n", __func__,
			 e2p_type, pAd->E2pAccessMode, e2p_default));
	pAd->eeprom_type = (e2p_type == E2P_EFUSE_MODE)  ? EEPROM_EFUSE : EEPROM_FLASH;
	pE2pCtrl->e2pCurMode = e2p_type;

	if (pAd->EEPROMImage == NULL)
		os_alloc_mem(pAd, &pAd->EEPROMImage, MAX_EEPROM_BUFFER_SIZE);

	switch (e2p_type) {
	case E2P_EEPROM_MODE:
		break;

	case E2P_BIN_MODE: {
		pChipOps->eeinit = rtmp_ee_bin_init;
		pChipOps->eeread = rtmp_ee_bin_read16;
		pChipOps->eewrite = rtmp_ee_bin_write16;
#ifdef BB_SOC
		pChipOps->eeread_range = rtmp_ee_bin_read_with_range;
		pChipOps->eewrite_range = rtmp_ee_bin_write_with_range;
#endif
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("NVM is BIN mode\n"));
		return 0;
	}

#ifdef RTMP_FLASH_SUPPORT

	case E2P_FLASH_MODE: {
		pChipOps->eeinit = rtmp_nv_init;
		pChipOps->eeread = rtmp_ee_flash_read;
		pChipOps->eewrite = rtmp_ee_flash_write;
		pChipOps->eeread_range = rtmp_ee_flash_read_with_range;
		pChipOps->eewrite_range = rtmp_ee_flash_write_with_range;

		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		 ("NVM is FLASH mode. dev_idx [%d] FLASH OFFSET [0x%X]\n", pAd->dev_idx, get_dev_eeprom_offset(pAd)));
		return 0;
	}

#endif /* RTMP_FLASH_SUPPORT */
#ifdef RTMP_EFUSE_SUPPORT

	case E2P_EFUSE_MODE:
		if (pAd->bUseEfuse) {
			pChipOps->eeinit = eFuse_init;
			pChipOps->eeread = rtmp_ee_efuse_read16;
			pChipOps->eewrite = rtmp_ee_efuse_write16;
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("NVM is EFUSE mode\n"));
			return 0;
		} else {
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s::hook efuse mode failed\n", __func__));
			break;
		}

#endif /* RTMP_EFUSE_SUPPORT */

	default:
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Do not support E2P type(%d), change to BIN mode\n", __func__,
				 e2p_type));
		pChipOps->eeinit = rtmp_ee_bin_init;
		pChipOps->eeread = rtmp_ee_bin_read16;
		pChipOps->eewrite = rtmp_ee_bin_write16;
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("NVM is BIN mode\n"));
		return 0;
	}

	/* Hook functions based on interface types for EEPROM */
	switch (infType) {
#ifdef RTMP_PCI_SUPPORT

	case RTMP_DEV_INF_PCI:
	case RTMP_DEV_INF_PCIE:
		RTMP_IO_READ32(pAd, E2PROM_CSR, &val);

		if ((val & 0x30) == 0)
			pAd->EEPROMAddressNum = 6; /* 93C46 */
		else
			pAd->EEPROMAddressNum = 8; /* 93C66 or 93C86 */

		pChipOps->eeinit = NULL;
		pChipOps->eeread = rtmp_ee_prom_read16;
		pChipOps->eewrite = rtmp_ee_prom_write16;
		break;
#endif /* RTMP_PCI_SUPPORT */

	default:
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s::hook failed\n", __func__));
		break;
	}

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("NVM is EEPROM mode\n"));
	return 0;
}


BOOLEAN rtmp_get_default_bin_file_by_chip(
	IN PRTMP_ADAPTER pAd,
	IN UINT32	ChipVersion,
	OUT RTMP_STRING **pBinFileName)
{
	BOOLEAN found = FALSE;
	INT i;

	for (i = 0; RTMP_CHIP_E2P_FILE_TABLE[i].ChipVersion != 0; i++) {
		if (RTMP_CHIP_E2P_FILE_TABLE[i].ChipVersion == ChipVersion) {
			*pBinFileName = RTMP_CHIP_E2P_FILE_TABLE[i].name;
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					 ("%s(): Found E2P bin file name:%s\n",
					  __func__, *pBinFileName));
			found = TRUE;
			break;
		}
	}

	if (found == TRUE)
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s::Found E2P bin file name=%s\n", __func__, *pBinFileName));
	else
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s::E2P bin file name not found\n", __func__));

	return found;
}


BOOLEAN rtmp_ee_bin_read16(RTMP_ADAPTER *pAd, UINT16 Offset, UINT16 *pValue)
{
	BOOLEAN IsEmpty = 0;
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s::Read from EEPROM buffer\n", __func__));
	NdisMoveMemory(pValue, &(pAd->EEPROMImage[Offset]), 2);
	*pValue = le2cpu16(*pValue);

	if ((*pValue == 0xffff) || (*pValue == 0x0000))
		IsEmpty = 1;

	return IsEmpty;
}


INT rtmp_ee_bin_write16(
	IN RTMP_ADAPTER	 *pAd,
	IN USHORT			Offset,
	IN USHORT			data)
{
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s::Write to EEPROM buffer\n", __func__));
	data = le2cpu16(data);
	NdisMoveMemory(&(pAd->EEPROMImage[Offset]), &data, 2);
	return 0;
}

#ifdef BB_SOC
BOOLEAN rtmp_ee_bin_read_with_range(PRTMP_ADAPTER pAd, UINT16 start, UINT16 Length, UCHAR *pbuf)
{
	BOOLEAN IsEmpty = 0;
	UINT16  u2Loop;
	UCHAR   ucValue = 0;

	memcpy(pbuf, pAd->EEPROMImage + start, Length);

	for (u2Loop = 0; u2Loop < Length; u2Loop++)
		ucValue |= pbuf[u2Loop];

	if ((ucValue == 0xff) || (ucValue == 0x00))
		IsEmpty = 1;

	return IsEmpty;
}

INT rtmp_ee_bin_write_with_range(PRTMP_ADAPTER pAd, USHORT start, USHORT Length, UCHAR *pbuf)
{
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s::Write data range to EEPROM buffer\n", __func__));
	NdisMoveMemory(&(pAd->EEPROMImage[start]), pbuf, Length);

	return 0;
}
#endif

INT rtmp_ee_load_from_bin(
	IN PRTMP_ADAPTER	pAd)
{
	RTMP_STRING *src = NULL;
	INT ret_val;
	RTMP_OS_FD srcf;
	RTMP_OS_FS_INFO osFSInfo;
	EEPROM_CONTROL *pE2pCtrl = &pAd->E2pCtrl;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	pE2pCtrl->e2pCurMode = E2P_BIN_MODE;
#ifdef RT_SOC_SUPPORT
#ifdef MULTIPLE_CARD_SUPPORT
	RTMP_STRING bin_file_path[128];
	RTMP_STRING *bin_file_name = NULL;
	UINT32 chip_ver = (pAd->MACVersion >> 16);

	if (rtmp_get_default_bin_file_by_chip(pAd, chip_ver, &bin_file_name) == TRUE) {
		if (pAd->MC_RowID > 0)
			sprintf(bin_file_path, "%s%s", EEPROM_2ND_FILE_DIR, bin_file_name);
		else
			sprintf(bin_file_path, "%s%s", EEPROM_1ST_FILE_DIR, bin_file_name);

		src = bin_file_path;
	} else
#endif /* MULTIPLE_CARD_SUPPORT */
#endif /* RT_SOC_SUPPORT */
	{
		if (cap->EEPROM_DEFAULT_BIN_FILE != NULL) {
			src = cap->EEPROM_DEFAULT_BIN_FILE;
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s::bin FileName=%s\n", __func__, cap->EEPROM_DEFAULT_BIN_FILE));
		} else {
			src = BIN_FILE_PATH;
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s::FileName=%s\n", __func__, src));
		}
	}

	RtmpOSFSInfoChange(&osFSInfo, TRUE);

	if (src && *src) {
		srcf = RtmpOSFileOpen(src, O_RDONLY, 0);

		if (IS_FILE_OPEN_ERR(srcf)) {
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("%s::Error opening %s\n", __func__, src));
			goto error;
		} else {
			NdisZeroMemory(pAd->EEPROMImage, MAX_EEPROM_BIN_FILE_SIZE);
			ret_val = RtmpOSFileRead(srcf, (RTMP_STRING *)pAd->EEPROMImage, MAX_EEPROM_BIN_FILE_SIZE);

			if (ret_val > 0) {
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("Load EEPROM Buffer from %s\n", src));
				ret_val = NDIS_STATUS_SUCCESS;
			} else {
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s::Read file \"%s\" failed(errCode=%d)!\n", __func__, src, ret_val));
			}
		}
	} else {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s::Error src or srcf is null\n", __func__));
		goto error;
	}

	ret_val = RtmpOSFileClose(srcf);

	if (ret_val)
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s::Error %d closing %s\n", __func__, -ret_val, src));

	pE2pCtrl->e2pSource = E2P_SRC_FROM_BIN;
	pE2pCtrl->BinSource = src;
	RtmpOSFSInfoChange(&osFSInfo, FALSE);
	return TRUE;
error:

	if (cap->EEPROM_DEFAULT_BIN != NULL) {
		NdisMoveMemory(pAd->EEPROMImage, cap->EEPROM_DEFAULT_BIN,
					   cap->EEPROM_DEFAULT_BIN_SIZE > MAX_EEPROM_BUFFER_SIZE ? MAX_EEPROM_BUFFER_SIZE :
					   cap->EEPROM_DEFAULT_BIN_SIZE);
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Load EEPROM Buffer from default BIN.\n"));
		pE2pCtrl->e2pSource = E2P_SRC_FROM_BIN;
		pE2pCtrl->BinSource = "Default bin";
	}

	return FALSE;
}

#if defined(PRE_CAL_TRX_SET1_SUPPORT) || defined(PRE_CAL_TRX_SET2_SUPPORT) || defined(RLM_CAL_CACHE_SUPPORT)
NDIS_STATUS rtmp_cal_load_from_bin(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR *buf,
	IN ULONG offset,
	IN ULONG len)
{
	RTMP_STRING *src = NULL;
	INT ret_val;
	RTMP_OS_FD srcf;
	RTMP_OS_FS_INFO osFSInfo;
#ifdef CONFIG_RT_FIRST_CARD

	if (pAd->dev_idx == 0)
		src = FIRST_CAL_FILE_PATH;
	else
#endif /* CONFIG_RT_FIRST_CARD */
#ifdef CONFIG_RT_SECOND_CARD
		if (pAd->dev_idx == 1)
			src = SECOND_CAL_FILE_PATH;
		else
#endif /* CONFIG_RT_SECOND_CARD */
			src = CAL_FILE_PATH;

	/* MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s::FileName=%s\n", __func__, src)); */
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s::FileName=%s\n", __func__, src));		/* Anjan - debug */
	RtmpOSFSInfoChange(&osFSInfo, TRUE);

	if (src && *src) {
		srcf = RtmpOSFileOpen(src, O_RDONLY, 0);

		if (IS_FILE_OPEN_ERR(srcf)) {
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("%s::Error opening %s\n", __func__, src));
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("\e[0;31m Run RXDCOC/TXDPD/PRECAL cmds to generate caldata files\e[m\n"));
			return NDIS_STATUS_FAILURE;
		} else {
			RtmpOSFileSeek(srcf, offset);
			ret_val = RtmpOSFileRead(srcf, (RTMP_STRING *)buf, len);

			if (ret_val > 0) {
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("Load from %s (Read = %d)\n", src, ret_val));
				ret_val = NDIS_STATUS_SUCCESS;
			} else {
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s::Read file \"%s\" failed(errCode=%d)!\n", __func__, src, ret_val));
			}
		}
	} else {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s::Error src or srcf is null\n", __func__));
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("FAIL!!! Load Cal Data from file.\n"));
		return NDIS_STATUS_FAILURE;
	}

	ret_val = RtmpOSFileClose(srcf);

	if (ret_val)
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s::Error %d closing %s\n", __func__, -ret_val, src));

	RtmpOSFSInfoChange(&osFSInfo, FALSE);
	return NDIS_STATUS_SUCCESS;
}


NDIS_STATUS rtmp_cal_write_to_bin(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR *buf,
	IN ULONG offset,
	IN ULONG len)
{
	RTMP_STRING *src = NULL;
	INT ret_val;
	RTMP_OS_FD srcf;
	RTMP_OS_FS_INFO osFSInfo;
#ifdef CONFIG_RT_FIRST_CARD

	if (pAd->dev_idx == 0)
		src = FIRST_CAL_FILE_PATH;
	else
#endif /* CONFIG_RT_FIRST_CARD */
#ifdef CONFIG_RT_SECOND_CARD
		if (pAd->dev_idx == 1)
			src = SECOND_CAL_FILE_PATH;
		else
#endif /* CONFIG_RT_SECOND_CARD */
			src = CAL_FILE_PATH;

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s::FileName=%s\n", __func__, src));
	RtmpOSFSInfoChange(&osFSInfo, TRUE);

	if (src && *src) {
		srcf = RtmpOSFileOpen(src, O_WRONLY | O_CREAT, 0);

		if (IS_FILE_OPEN_ERR(srcf)) {
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s::Error opening %s\n", __func__, src));
			return NDIS_STATUS_FAILURE;
		} else {
			RtmpOSFileSeek(srcf, offset);
			RtmpOSFileWrite(srcf, (RTMP_STRING *)buf, len);
		}
	} else {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s::Error src or srcf is null\n", __func__));
		return NDIS_STATUS_FAILURE;
	}

	ret_val = RtmpOSFileClose(srcf);

	if (ret_val)
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s::Error %d closing %s\n", __func__, -ret_val, src));

	RtmpOSFSInfoChange(&osFSInfo, FALSE);
	return NDIS_STATUS_SUCCESS;
}
#endif	/* PRE_CAL_TRX_SET1_SUPPORT */

INT rtmp_ee_write_to_bin(
	IN PRTMP_ADAPTER	pAd)
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

	if (rtmp_get_default_bin_file_by_chip(pAd, chip_ver, &bin_file_name) == TRUE) {
		if (pAd->MC_RowID > 0)
			sprintf(bin_file_path, "%s%s", EEPROM_2ND_FILE_DIR, bin_file_name);
		else
			sprintf(bin_file_path, "%s%s", EEPROM_1ST_FILE_DIR, bin_file_name);

		src = bin_file_path;
	} else
#endif /* MULTIPLE_CARD_SUPPORT */
#endif /* RT_SOC_SUPPORT */
	{
		struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

		if (cap->EEPROM_DEFAULT_BIN_FILE != NULL) {
			src = cap->EEPROM_DEFAULT_BIN_FILE;
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("%s::bin FileName=%s\n", __func__, cap->EEPROM_DEFAULT_BIN_FILE));
		} else {
			src = BIN_FILE_PATH;
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("%s::src FileName=%s\n", __func__, src));
		}
	}

	RtmpOSFSInfoChange(&osFSInfo, TRUE);

	if (src && *src) {
		srcf = RtmpOSFileOpen(src, O_WRONLY | O_CREAT, 0);

		if (IS_FILE_OPEN_ERR(srcf)) {
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s::Error opening %s\n", __func__, src));
			return FALSE;
		} else
			RtmpOSFileWrite(srcf, (RTMP_STRING *)pAd->EEPROMImage, MAX_EEPROM_BIN_FILE_SIZE);
	} else {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s::Error src or srcf is null\n", __func__));
		return FALSE;
	}

	ret_val = RtmpOSFileClose(srcf);

	if (ret_val)
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s::Error %d closing %s\n", __func__, -ret_val, src));

	RtmpOSFSInfoChange(&osFSInfo, FALSE);
	return TRUE;
}


INT Set_LoadEepromBufferFromBin_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	LONG bEnable = os_str_tol(arg, 0, 10);
	INT result;

	if (bEnable < 0)
		return FALSE;
	else {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Load EEPROM buffer from BIN, and change to BIN buffer mode\n"));
		result = rtmp_ee_load_from_bin(pAd);

		if (result == FALSE) {
			struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

			if (cap->EEPROM_DEFAULT_BIN != NULL) {
				NdisMoveMemory(pAd->EEPROMImage, cap->EEPROM_DEFAULT_BIN,
							   cap->EEPROM_DEFAULT_BIN_SIZE > MAX_EEPROM_BUFFER_SIZE ? MAX_EEPROM_BUFFER_SIZE :
							   cap->EEPROM_DEFAULT_BIN_SIZE);
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Load EEPROM Buffer from default BIN.\n"));
			}
		}

		/* Change to BIN eeprom buffer mode */
		RtmpChipOpsEepromHook(pAd, pAd->infType, E2P_BIN_MODE);
		return TRUE;
	}
}


INT Set_EepromBufferWriteBack_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT e2p_mode = os_str_tol(arg, 0, 10);

	if (e2p_mode >= NUM_OF_E2P_MODE)
		return FALSE;

	switch (e2p_mode) {
#ifdef RTMP_EFUSE_SUPPORT

	case E2P_EFUSE_MODE:
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Write EEPROM buffer back to eFuse\n"));
		rtmp_ee_write_to_efuse(pAd);
		break;
#endif /* RTMP_EFUSE_SUPPORT */
#ifdef RTMP_FLASH_SUPPORT

	case E2P_FLASH_MODE:
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Write EEPROM buffer back to Flash\n"));
		rtmp_ee_flash_write_all(pAd);
		break;
#endif /* RTMP_FLASH_SUPPORT */

	case E2P_EEPROM_MODE:
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Write EEPROM buffer back to EEPROM\n"));
		rtmp_ee_write_to_prom(pAd);
		break;

	case E2P_BIN_MODE:
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Write EEPROM buffer back to BIN\n"));
		rtmp_ee_write_to_bin(pAd);
		break;

	default:
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s::do not support this EEPROM access mode\n", __func__));
		return FALSE;
	}

	return TRUE;
}

INT Set_bufferMode_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT EepromType = os_str_tol(arg, 0, 10);
	MtCmdEfusBufferModeSet(pAd, EepromType);

	return TRUE;
}


static PCHAR e2p_mode[] = {"NONE", "Efuse", "Flash", "Eeprom", "Bin"};
static PCHAR e2p_src[] = {[E2P_SRC_FROM_EFUSE] = "Efuse",
						  [E2P_SRC_FROM_FLASH] = "Flash",
						  [E2P_SRC_FROM_EEPROM] = "Eeprom",
						  [E2P_SRC_FROM_BIN] = "Bin",
						  [E2P_SRC_FROM_FLASH_AND_EFUSE] = "Flash + ical data  from efuse(merge mode)",
						  [E2P_SRC_FROM_BIN_AND_EFUSE] = "Bin + ical data from efuse(merge mode)"
						 };
INT show_e2pinfo_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	EEPROM_CONTROL *pE2pCtrl = &pAd->E2pCtrl;
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("Default eeprom mode from profile: %s\n", e2p_mode[pAd->E2pAccessMode]));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("Current mode: %s\n", e2p_mode[pE2pCtrl->e2pCurMode]));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("E2p source: %s\n", e2p_src[pE2pCtrl->e2pSource]));
#ifdef CAL_FREE_IC_SUPPORT

	if (!(pE2pCtrl->e2pSource & E2P_SRC_FROM_EFUSE)
		&& ops->check_is_cal_free_merge) {
		if (ops->check_is_cal_free_merge(pAd))
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ical data merge: YES\n"));
		else
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ical data merge: No\n"));
	}

#endif

	if ((pE2pCtrl->e2pSource & E2P_SRC_FROM_BIN) && pE2pCtrl->BinSource)
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Bin file Source: %s\n", pE2pCtrl->BinSource));

	return TRUE;
}


#ifdef CAL_FREE_IC_SUPPORT
INT Set_LoadCalFreeData_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT value = os_str_tol(arg, 0, 10);
	EEPROM_CONTROL *pE2pCtrl = &pAd->E2pCtrl;

	if (value == 1) {
		BOOLEAN bCalFree = 0;
		RTMP_CAL_FREE_IC_CHECK(pAd, bCalFree);

		if (bCalFree) {
			RTMP_CAL_FREE_DATA_GET(pAd);
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Merge successfully"));

			if (pE2pCtrl->e2pCurMode == E2P_FLASH_MODE)
				MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, (",plz write back to flash"));

			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
		} else
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Merge fail\n"));
	} else if (value == 2) {
		RTMP_CAL_FREE_DATA_GET(pAd);
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Merge successfully"));
	} else
		return FALSE;

	return TRUE;
}

INT Set_CheckCalFree_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

	if (ops->check_is_cal_free_merge) {
		if (ops->check_is_cal_free_merge(pAd))
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("CalFree data has been merged!!\n"));
		else
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("CalFree data has not been merged!!\n"));
	} else {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Not Support CalFree Merge Check!\n"));
		return FALSE;
	}

	return TRUE;
}

#endif



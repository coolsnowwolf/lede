/****************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ****************************************************************************

    Module Name:
	cmm_cfg.c

    Abstract:
    Ralink WiFi Driver configuration related subroutines

    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
*/



#include "rt_config.h"

static BOOLEAN RT_isLegalCmdBeforeInfUp(
       IN PSTRING SetCmd);


INT ComputeChecksum(
	IN UINT PIN)
{
	INT digit_s;
    UINT accum = 0;

	PIN *= 10;
	accum += 3 * ((PIN / 10000000) % 10); 
	accum += 1 * ((PIN / 1000000) % 10); 
	accum += 3 * ((PIN / 100000) % 10); 
	accum += 1 * ((PIN / 10000) % 10); 
	accum += 3 * ((PIN / 1000) % 10); 
	accum += 1 * ((PIN / 100) % 10); 
	accum += 3 * ((PIN / 10) % 10); 

	digit_s = (accum % 10);
	return ((10 - digit_s) % 10);
} /* ComputeChecksum*/

UINT GenerateWpsPinCode(
	IN	PRTMP_ADAPTER	pAd,
    IN  BOOLEAN         bFromApcli,	
	IN	UCHAR			apidx)
{
	UCHAR	macAddr[MAC_ADDR_LEN];
	UINT 	iPin;
	UINT	checksum;

	NdisZeroMemory(macAddr, MAC_ADDR_LEN);

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
#ifdef APCLI_SUPPORT
	    if (bFromApcli)
	        NdisMoveMemory(&macAddr[0], pAd->ApCfg.ApCliTab[apidx].wdev.if_addr, MAC_ADDR_LEN);
	    else
#endif /* APCLI_SUPPORT */
		NdisMoveMemory(&macAddr[0], pAd->ApCfg.MBSSID[apidx].wdev.if_addr, MAC_ADDR_LEN);
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		NdisMoveMemory(&macAddr[0], pAd->CurrentAddress, MAC_ADDR_LEN);
#endif /* CONFIG_STA_SUPPORT */

	iPin = macAddr[3] * 256 * 256 + macAddr[4] * 256 + macAddr[5];

	iPin = iPin % 10000000;

	
	checksum = ComputeChecksum( iPin );
	iPin = iPin*10 + checksum;

	return iPin;
}


static char *phy_mode_str[]={"CCK", "OFDM", "HTMIX", "GF", "VHT"};
char* get_phymode_str(int Mode)
{
	if (Mode >= MODE_CCK && Mode <= MODE_VHT)
		return phy_mode_str[Mode];
	else
		return "N/A";
}


static UCHAR *phy_bw_str[] = {"20M", "40M", "80M", "10M"};
char* get_bw_str(int bandwidth)
{
	if (bandwidth >= BW_20 && bandwidth <= BW_10)
		return phy_bw_str[bandwidth];
	else
		return "N/A";
}


/* 
    ==========================================================================
    Description:
        Set Country Region to pAd->CommonCfg.CountryRegion.
        This command will not work, if the field of CountryRegion in eeprom is programmed.
        
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT RT_CfgSetCountryRegion(
	IN PRTMP_ADAPTER	pAd, 
	IN PSTRING			arg,
	IN INT				band)
{
	LONG region;
	UCHAR *pCountryRegion;
	
	region = simple_strtol(arg, 0, 10);

	if (band == BAND_24G)
		pCountryRegion = &pAd->CommonCfg.CountryRegion;
	else
		pCountryRegion = &pAd->CommonCfg.CountryRegionForABand;
	
    /*
               1. If this value is set before interface up, do not reject this value.
               2. Country can be set only when EEPROM not programmed
    */
    if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE) && (*pCountryRegion & EEPROM_IS_PROGRAMMED))
	{
		DBGPRINT(RT_DEBUG_ERROR, ("CfgSetCountryRegion():CountryRegion in eeprom was programmed\n"));
		return FALSE;
	}

	if((region >= 0) && 
	   (((band == BAND_24G) &&((region <= REGION_MAXIMUM_BG_BAND) || 
	   (region == REGION_31_BG_BAND) || (region == REGION_32_BG_BAND) || (region == REGION_33_BG_BAND) )) || 
	    ((band == BAND_5G) && (region <= REGION_MAXIMUM_A_BAND) ))
	  )
	{
		*pCountryRegion= (UCHAR) region;
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("CfgSetCountryRegion():region(%ld) out of range!\n", region));
		return FALSE;
	}

	return TRUE;
	
}


static UCHAR CFG_WMODE_MAP[]={
	PHY_11BG_MIXED, (WMODE_B | WMODE_G), /* 0 => B/G mixed */
	PHY_11B, (WMODE_B), /* 1 => B only */
	PHY_11A, (WMODE_A), /* 2 => A only */
	PHY_11ABG_MIXED, (WMODE_A | WMODE_B | WMODE_G), /* 3 => A/B/G mixed */
	PHY_11G, WMODE_G, /* 4 => G only */
	PHY_11ABGN_MIXED, (WMODE_B | WMODE_G | WMODE_GN | WMODE_A | WMODE_AN), /* 5 => A/B/G/GN/AN mixed */
	PHY_11N_2_4G, (WMODE_GN), /* 6 => N in 2.4G band only */
	PHY_11GN_MIXED, (WMODE_G | WMODE_GN), /* 7 => G/GN, i.e., no CCK mode */
	PHY_11AN_MIXED, (WMODE_A | WMODE_AN), /* 8 => A/N in 5 band */
	PHY_11BGN_MIXED, (WMODE_B | WMODE_G | WMODE_GN), /* 9 => B/G/GN mode*/
	PHY_11AGN_MIXED, (WMODE_G | WMODE_GN | WMODE_A | WMODE_AN), /* 10 => A/AN/G/GN mode, not support B mode */
	PHY_11N_5G, (WMODE_AN), /* 11 => only N in 5G band */
#ifdef DOT11_VHT_AC
	PHY_11VHT_N_ABG_MIXED, (WMODE_B | WMODE_G | WMODE_GN |WMODE_A | WMODE_AN | WMODE_AC), /* 12 => B/G/GN/A/AN/AC mixed*/
	PHY_11VHT_N_AG_MIXED, (WMODE_G | WMODE_GN |WMODE_A | WMODE_AN | WMODE_AC), /* 13 => G/GN/A/AN/AC mixed , no B mode */
	PHY_11VHT_N_A_MIXED, (WMODE_A | WMODE_AN | WMODE_AC), /* 14 => A/AC/AN mixed */
	PHY_11VHT_N_MIXED, (WMODE_AN | WMODE_AC), /* 15 => AC/AN mixed, but no A mode */
#endif /* DOT11_VHT_AC */
	PHY_MODE_MAX, WMODE_INVALID /* default phy mode if not match */
};


static PSTRING BAND_STR[] = {"Invalid", "2.4G", "5G", "2.4G/5G"};
static PSTRING WMODE_STR[]= {"", "A", "B", "G", "gN", "aN", "AC"};

UCHAR *wmode_2_str(UCHAR wmode)
{
	UCHAR *str;
	INT idx, pos, max_len;

	max_len = WMODE_COMP * 3;
	if (os_alloc_mem(NULL, &str, max_len) == NDIS_STATUS_SUCCESS)
	{
		NdisZeroMemory(str, max_len);
		pos = 0;
		for (idx = 0; idx < WMODE_COMP; idx++)
		{
			if (wmode & (1 << idx)) {
				if ((strlen(str) +  strlen(WMODE_STR[idx + 1])) >= (max_len - 1))
					break;
				if (strlen(str)) {
					NdisMoveMemory(&str[pos], "/", 1);
					pos++;
				}
				NdisMoveMemory(&str[pos], WMODE_STR[idx + 1], strlen(WMODE_STR[idx + 1]));
				pos += strlen(WMODE_STR[idx + 1]);
			}
			if (strlen(str) >= max_len)
				break;
		}

		return str;
	}
	else
		return NULL;
}


RT_802_11_PHY_MODE wmode_2_cfgmode(UCHAR wmode)
{
	INT i, mode_cnt = sizeof(CFG_WMODE_MAP) / sizeof(UCHAR);

	for (i = 1; i < mode_cnt; i+=2)
	{	
		if (CFG_WMODE_MAP[i] == wmode)
			return CFG_WMODE_MAP[i - 1];
	}

	DBGPRINT(RT_DEBUG_ERROR, ("%s(): Cannot get cfgmode by wmode(%x)\n", 
				__FUNCTION__, wmode));

	return 0;
}


UCHAR cfgmode_2_wmode(UCHAR cfg_mode)
{
	DBGPRINT(RT_DEBUG_OFF, ("cfg_mode=%d\n", cfg_mode));
	if (cfg_mode >= PHY_MODE_MAX)
		cfg_mode =  PHY_MODE_MAX;
	
	return CFG_WMODE_MAP[cfg_mode * 2 + 1];
}


BOOLEAN wmode_valid(RTMP_ADAPTER *pAd, enum WIFI_MODE wmode)
{
	if ((WMODE_CAP_5G(wmode) && (!PHY_CAP_5G(pAd->chipCap.phy_caps))) ||
		(WMODE_CAP_2G(wmode) && (!PHY_CAP_2G(pAd->chipCap.phy_caps))) ||
		(WMODE_CAP_N(wmode) && RTMP_TEST_MORE_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DOT_11N))
	)
		return FALSE;
	else
		return TRUE;
}


BOOLEAN wmode_band_equal(UCHAR smode, UCHAR tmode)
{
	BOOLEAN eq = FALSE;
	UCHAR *str1, *str2;
	
	if ((WMODE_CAP_5G(smode) == WMODE_CAP_5G(tmode)) &&
		(WMODE_CAP_2G(smode) == WMODE_CAP_2G(tmode)))
		eq = TRUE; 

	str1 = wmode_2_str(smode);
	str2 = wmode_2_str(tmode);
	if (str1 && str2)
	{
		DBGPRINT(RT_DEBUG_TRACE,
			("Old WirelessMode:%s(0x%x), "
			 "New WirelessMode:%s(0x%x)!\n",
			str1, smode, str2, tmode));
	}
	if (str1)
		os_free_mem(NULL, str1);
	if (str2)
		os_free_mem(NULL, str2);
		
	return eq;
}


/* 
    ==========================================================================
    Description:
        Set Wireless Mode
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT RT_CfgSetWirelessMode(RTMP_ADAPTER *pAd, PSTRING arg)
{
	LONG cfg_mode;
	UCHAR wmode, *mode_str;
#ifdef MT76x2
	RTMP_CHIP_CAP *pChipCap = &pAd->chipCap;
#endif /* MT76x2 */

	cfg_mode = simple_strtol(arg, 0, 10);

	/* check if chip support 5G band when WirelessMode is 5G band */
	wmode = cfgmode_2_wmode((UCHAR)cfg_mode);
	if ((wmode == WMODE_INVALID) || (!wmode_valid(pAd, wmode))) {
		DBGPRINT(RT_DEBUG_ERROR,
				("%s(): Invalid wireless mode(%ld, wmode=0x%x), ChipCap(%s)\n",
				__FUNCTION__, cfg_mode, wmode,
				BAND_STR[pAd->chipCap.phy_caps & 0x3]));
		return FALSE;
	}

#ifdef MT76x2
#ifdef DOT11_VHT_AC
	if (pChipCap->ac_off_mode && WMODE_CAP_AC(wmode)) {
		DBGPRINT(RT_DEBUG_ERROR, ("it doesn't support VHT AC!\n"));
		wmode &= ~(WMODE_AC);
	}
#endif /* DOT11_VHT_AC */
#endif /* MT76x2 */

	if (wmode_band_equal(pAd->CommonCfg.PhyMode, wmode) == TRUE)
		DBGPRINT(RT_DEBUG_OFF, ("wmode_band_equal(): Band Equal!\n"));
	else
		DBGPRINT(RT_DEBUG_OFF, ("wmode_band_equal(): Band Not Equal!\n"));
	
	pAd->CommonCfg.PhyMode = wmode;
	pAd->CommonCfg.cfg_wmode = wmode;
	
	mode_str = wmode_2_str(wmode);
	if (mode_str)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("%s(): Set WMODE=%s(0x%x)\n",
				__FUNCTION__, mode_str, wmode));
		os_free_mem(NULL, mode_str);
	}

	return TRUE;
}


/* maybe can be moved to GPL code, ap_mbss.c, but the code will be open */
#ifdef CONFIG_AP_SUPPORT
#ifdef MBSS_SUPPORT
static UCHAR RT_CfgMbssWirelessModeMaxGet(RTMP_ADAPTER *pAd)
{
	UCHAR wmode = 0, *mode_str;
	INT idx;
	struct wifi_dev *wdev;

	for(idx = 0; idx < pAd->ApCfg.BssidNum; idx++) {
		wdev = &pAd->ApCfg.MBSSID[idx].wdev;
		mode_str = wmode_2_str(wdev->PhyMode);
		if (mode_str)
		{
			DBGPRINT(RT_DEBUG_TRACE, ("%s(BSS%d): wmode=%s(0x%x)\n",
					__FUNCTION__, idx, mode_str, wdev->PhyMode));
			os_free_mem(pAd, mode_str);
		}
		wmode |= wdev->PhyMode;
	}

	mode_str = wmode_2_str(wmode);
	if (mode_str)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("%s(): Combined WirelessMode = %s(0x%x)\n", 
					__FUNCTION__, mode_str, wmode));
		os_free_mem(pAd, mode_str);
	}
	return wmode;
}


/* 
    ==========================================================================
    Description:
        Set Wireless Mode for MBSS
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT RT_CfgSetMbssWirelessMode(RTMP_ADAPTER *pAd, PSTRING arg)
{
	INT cfg_mode;
	UCHAR wmode;
#ifdef MT76x2
	RTMP_CHIP_CAP *pChipCap = &pAd->chipCap;
#endif /* MT76x2 */

	cfg_mode = simple_strtol(arg, 0, 10);

	wmode = cfgmode_2_wmode((UCHAR)cfg_mode);
	if ((wmode == WMODE_INVALID) || (!wmode_valid(pAd, wmode))) {
		DBGPRINT(RT_DEBUG_ERROR,
				("%s(): Invalid wireless mode(%d, wmode=0x%x), ChipCap(%s)\n",
				__FUNCTION__, cfg_mode, wmode,
				BAND_STR[pAd->chipCap.phy_caps & 0x3]));
		return FALSE;
	}
	
	if (WMODE_CAP_5G(wmode) && WMODE_CAP_2G(wmode))
	{
		DBGPRINT(RT_DEBUG_ERROR, ("AP cannot support 2.4G/5G band mxied mode!\n"));
		return FALSE;
	}


#ifdef MT76x2
#ifdef DOT11_VHT_AC
	if (pChipCap->ac_off_mode && WMODE_CAP_AC(wmode)) {
		DBGPRINT(RT_DEBUG_ERROR, ("it doesn't support VHT AC!\n"));
		wmode &= ~(WMODE_AC);
	}
#endif /* DOT11_VHT_AC */
#endif /* MT76x2 */

	if (pAd->ApCfg.BssidNum > 1)
	{
		/* pAd->CommonCfg.PhyMode = maximum capability of all MBSS */
		if (wmode_band_equal(pAd->CommonCfg.PhyMode, wmode) == TRUE)
		{
			wmode = RT_CfgMbssWirelessModeMaxGet(pAd);

			DBGPRINT(RT_DEBUG_TRACE,
					("mbss> Maximum phy mode = %d!\n", wmode));
		}
		else
		{
			UINT32 IdBss;

			/* replace all phy mode with the one with different band */
			DBGPRINT(RT_DEBUG_TRACE,
					("mbss> Different band with the current one!\n"));
			DBGPRINT(RT_DEBUG_TRACE,
					("mbss> Reset band of all BSS to the new one!\n"));

			for(IdBss=0; IdBss<pAd->ApCfg.BssidNum; IdBss++)
				pAd->ApCfg.MBSSID[IdBss].wdev.PhyMode = wmode;
		}
	}

	pAd->CommonCfg.PhyMode = wmode;
	pAd->CommonCfg.cfg_wmode = wmode;
	return TRUE;
}
#endif /* MBSS_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */


static BOOLEAN RT_isLegalCmdBeforeInfUp(
       IN PSTRING SetCmd)
{
		BOOLEAN TestFlag;
		TestFlag =	!strcmp(SetCmd, "Debug") ||
#ifdef CONFIG_APSTA_MIXED_SUPPORT
					!strcmp(SetCmd, "OpMode") ||
#endif /* CONFIG_APSTA_MIXED_SUPPORT */
#ifdef EXT_BUILD_CHANNEL_LIST
					!strcmp(SetCmd, "CountryCode") ||
					!strcmp(SetCmd, "DfsType") ||
					!strcmp(SetCmd, "ChannelListAdd") ||
					!strcmp(SetCmd, "ChannelListShow") ||
					!strcmp(SetCmd, "ChannelListDel") ||
#endif /* EXT_BUILD_CHANNEL_LIST */
#ifdef SINGLE_SKU
					!strcmp(SetCmd, "ModuleTxpower") ||
#endif /* SINGLE_SKU */
					FALSE; /* default */
       return TestFlag;
}


INT RT_CfgSetShortSlot(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	LONG ShortSlot;

	ShortSlot = simple_strtol(arg, 0, 10);

	if (ShortSlot == 1)
		pAd->CommonCfg.bUseShortSlotTime = TRUE;
	else if (ShortSlot == 0)
		pAd->CommonCfg.bUseShortSlotTime = FALSE;
	else
		return FALSE;  /*Invalid argument */

	return TRUE;
}


/* 
    ==========================================================================
    Description:
        Set WEP KEY base on KeyIdx
    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT	RT_CfgSetWepKey(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			keyString,
	IN	CIPHER_KEY		*pSharedKey,
	IN	INT				keyIdx)
{
	INT				KeyLen;
	INT				i;
	/*UCHAR			CipherAlg = CIPHER_NONE;*/
	BOOLEAN			bKeyIsHex = FALSE;

	/* TODO: Shall we do memset for the original key info??*/
	memset(pSharedKey, 0, sizeof(CIPHER_KEY));
	KeyLen = strlen(keyString);
	switch (KeyLen)
	{
		case 5: /*wep 40 Ascii type*/
		case 13: /*wep 104 Ascii type*/
			bKeyIsHex = FALSE;
			pSharedKey->KeyLen = KeyLen;
			NdisMoveMemory(pSharedKey->Key, keyString, KeyLen);
			break;
			
		case 10: /*wep 40 Hex type*/
		case 26: /*wep 104 Hex type*/
			for(i=0; i < KeyLen; i++)
			{
				if( !isxdigit(*(keyString+i)) )
					return FALSE;  /*Not Hex value;*/
			}
			bKeyIsHex = TRUE;
			pSharedKey->KeyLen = KeyLen/2 ;
			AtoH(keyString, pSharedKey->Key, pSharedKey->KeyLen);
			break;
			
		default: /*Invalid argument */
			DBGPRINT(RT_DEBUG_TRACE, ("RT_CfgSetWepKey(keyIdx=%d):Invalid argument (arg=%s)\n", keyIdx, keyString));
			return FALSE;
	}

	pSharedKey->CipherAlg = ((KeyLen % 5) ? CIPHER_WEP128 : CIPHER_WEP64);
	DBGPRINT(RT_DEBUG_TRACE, ("RT_CfgSetWepKey:(KeyIdx=%d,type=%s, Alg=%s)\n", 
						keyIdx, (bKeyIsHex == FALSE ? "Ascii" : "Hex"), CipherName[pSharedKey->CipherAlg]));

	return TRUE;
}


/* 
    ==========================================================================
    Description:
        Set WPA PSK key

    Arguments:
        pAdapter	Pointer to our adapter
        keyString	WPA pre-shared key string
        pHashStr	String used for password hash function
        hashStrLen	Lenght of the hash string
        pPMKBuf		Output buffer of WPAPSK key

    Return:
        TRUE if all parameters are OK, FALSE otherwise
    ==========================================================================
*/
INT RT_CfgSetWPAPSKKey(
	IN RTMP_ADAPTER	*pAd, 
	IN PSTRING		keyString,
	IN INT			keyStringLen,
	IN UCHAR		*pHashStr,
	IN INT			hashStrLen,
	OUT PUCHAR		pPMKBuf)
{
	UCHAR keyMaterial[40];

	if ((keyStringLen < 8) || (keyStringLen > 64))
	{
		DBGPRINT(RT_DEBUG_TRACE, ("WPAPSK Key length(%d) error, required 8 ~ 64 characters!(keyStr=%s)\n", 
									keyStringLen, keyString));
		return FALSE;
	}

	NdisZeroMemory(pPMKBuf, 32);
	if (keyStringLen == 64)
	{
	    AtoH(keyString, pPMKBuf, 32);
	}
	else
	{
	    RtmpPasswordHash(keyString, pHashStr, hashStrLen, keyMaterial);
	    NdisMoveMemory(pPMKBuf, keyMaterial, 32);		
	}

	return TRUE;
}

INT	RT_CfgSetFixedTxPhyMode(PSTRING arg)
{
	INT fix_tx_mode = FIXED_TXMODE_HT;
	ULONG value;


	if (rtstrcasecmp(arg, "OFDM") == TRUE)
		fix_tx_mode = FIXED_TXMODE_OFDM;
	else if (rtstrcasecmp(arg, "CCK") == TRUE)
	    fix_tx_mode = FIXED_TXMODE_CCK;
	else if (rtstrcasecmp(arg, "HT") == TRUE)
	    fix_tx_mode = FIXED_TXMODE_HT;
	else if (rtstrcasecmp(arg, "VHT") == TRUE)
		fix_tx_mode = FIXED_TXMODE_VHT;
	else
	{
		value = simple_strtol(arg, 0, 10);
		switch (value)
		{
			case FIXED_TXMODE_CCK:
			case FIXED_TXMODE_OFDM:
			case FIXED_TXMODE_HT:
			case FIXED_TXMODE_VHT:
				fix_tx_mode = value;
				break;
			default:
				fix_tx_mode = FIXED_TXMODE_HT;
		}
	}

	return fix_tx_mode;
					
}	

INT	RT_CfgSetMacAddress(
	IN 	PRTMP_ADAPTER 	pAd,
	IN	PSTRING			arg)
{
	INT	i, mac_len;
	
	/* Mac address acceptable format 01:02:03:04:05:06 length 17 */
	mac_len = strlen(arg);
	if(mac_len != 17)  
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s : invalid length (%d)\n", __FUNCTION__, mac_len));
		return FALSE;
	}

	if(strcmp(arg, "00:00:00:00:00:00") == 0)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s : invalid mac setting \n", __FUNCTION__));
		return FALSE;
	}

	for (i = 0; i < MAC_ADDR_LEN; i++)
	{
		AtoH(arg, &pAd->CurrentAddress[i], 1);
		arg = arg + 3;
	}	

	pAd->bLocalAdminMAC = TRUE;
	return TRUE;
}

INT	RT_CfgSetTxMCSProc(PSTRING arg, BOOLEAN *pAutoRate)
{
	INT	Value = simple_strtol(arg, 0, 10);
	INT	TxMcs;
	
	if ((Value >= 0 && Value <= 23) || (Value == 32)) /* 3*3*/
	{
		TxMcs = Value;
		*pAutoRate = FALSE;
	}
	else
	{		
		TxMcs = MCS_AUTO;
		*pAutoRate = TRUE;
	}

	return TxMcs;

}

INT	RT_CfgSetAutoFallBack(
	IN 	PRTMP_ADAPTER 	pAd,
	IN	PSTRING			arg)
{
	UCHAR AutoFallBack = (UCHAR)simple_strtol(arg, 0, 10);

	if (AutoFallBack)
		AutoFallBack = TRUE;
	else
		AutoFallBack = FALSE;

	AsicSetAutoFallBack(pAd, (AutoFallBack) ? TRUE : FALSE);
	DBGPRINT(RT_DEBUG_TRACE, ("RT_CfgSetAutoFallBack::(AutoFallBack=%d)\n", AutoFallBack));
	return TRUE;
}

#ifdef WSC_INCLUDED
INT	RT_CfgSetWscPinCode(
	IN RTMP_ADAPTER *pAd,
	IN PSTRING		pPinCodeStr,
	OUT PWSC_CTRL   pWscControl)
{
	UINT pinCode;

	pinCode = (UINT) simple_strtol(pPinCodeStr, 0, 10); /* When PinCode is 03571361, return value is 3571361.*/
	if (strlen(pPinCodeStr) == 4)
	{
		pWscControl->WscEnrolleePinCode = pinCode;
		pWscControl->WscEnrolleePinCodeLen = 4;
	}
	else if ( ValidateChecksum(pinCode) )
	{
		pWscControl->WscEnrolleePinCode = pinCode;
		pWscControl->WscEnrolleePinCodeLen = 8;
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("RT_CfgSetWscPinCode(): invalid Wsc PinCode (%d)\n", pinCode));
		return FALSE;
	}
	
	DBGPRINT(RT_DEBUG_TRACE, ("RT_CfgSetWscPinCode():Wsc PinCode=%d\n", pinCode));
	
	return TRUE;
	
}
#endif /* WSC_INCLUDED */

/*
========================================================================
Routine Description:
	Handler for CMD_RTPRIV_IOCTL_STA_SIOCGIWNAME.

Arguments:
	pAd				- WLAN control block pointer
	*pData			- the communication data pointer
	Data			- the communication data

Return Value:
	NDIS_STATUS_SUCCESS or NDIS_STATUS_FAILURE

Note:
========================================================================
*/
INT RtmpIoctl_rt_ioctl_giwname(
	IN	RTMP_ADAPTER			*pAd,
	IN	VOID					*pData,
	IN	ULONG					Data)
{
	UCHAR CurOpMode = OPMODE_AP;

	if (CurOpMode == OPMODE_AP)
	{
		strcpy(pData, "RTWIFI SoftAP");
	}

	return NDIS_STATUS_SUCCESS;
}


INT RTMP_COM_IoctlHandle(
	IN	VOID					*pAdSrc,
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN	INT						cmd,
	IN	USHORT					subcmd,
	IN	VOID					*pData,
	IN	ULONG					Data)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	INT Status = NDIS_STATUS_SUCCESS, i;


	pObj = pObj; /* avoid compile warning */

	switch(cmd)
	{
		case CMD_RTPRIV_IOCTL_NETDEV_GET:
		/* get main net_dev */
		{
			VOID **ppNetDev = (VOID **)pData;
			*ppNetDev = (VOID *)(pAd->net_dev);
		}
			break;

		case CMD_RTPRIV_IOCTL_NETDEV_SET:
			{
				struct wifi_dev *wdev = NULL;
				/* set main net_dev */
				pAd->net_dev = pData;

#ifdef CONFIG_AP_SUPPORT
				if (pAd->OpMode == OPMODE_AP) {
					pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev.if_dev = (void *)pData;
					pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev.func_dev = (void *)&pAd->ApCfg.MBSSID[MAIN_MBSSID];
					pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev.sys_handle = (void *)pAd;
					RTMP_OS_NETDEV_SET_WDEV(pData, &pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev);
					wdev = &pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev;
				}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
				if (pAd->OpMode == OPMODE_STA) {
					pAd->StaCfg.wdev.if_dev = pData;
					pAd->StaCfg.wdev.func_dev = (void *)&pAd->StaCfg;
					pAd->StaCfg.wdev.sys_handle = (void *)pAd;
					RTMP_OS_NETDEV_SET_WDEV(pData, &pAd->StaCfg.wdev);
					wdev = &pAd->StaCfg.wdev;
				}
#endif /* CONFIG_STA_SUPPORT */
				if (wdev) {
					if (rtmp_wdev_idx_reg(pAd, wdev) < 0) {
						DBGPRINT(RT_DEBUG_ERROR, ("Assign wdev idx for %s failed, free net device!\n",
								RTMP_OS_NETDEV_GET_DEVNAME(pAd->net_dev)));
						RtmpOSNetDevFree(pAd->net_dev);
					}
				}
				break;
			}
		case CMD_RTPRIV_IOCTL_OPMODE_GET:
		/* get Operation Mode */
			*(ULONG *)pData = pAd->OpMode;
			break;


		case CMD_RTPRIV_IOCTL_TASK_LIST_GET:
		/* get all Tasks */
		{
			RT_CMD_WAIT_QUEUE_LIST *pList = (RT_CMD_WAIT_QUEUE_LIST *)pData;

			pList->pMlmeTask = &pAd->mlmeTask;
#ifdef RTMP_TIMER_TASK_SUPPORT
			pList->pTimerTask = &pAd->timerTask;
#endif /* RTMP_TIMER_TASK_SUPPORT */
			pList->pCmdQTask = &pAd->cmdQTask;
#ifdef WSC_INCLUDED
			pList->pWscTask = &pAd->wscTask;
#endif /* WSC_INCLUDED */
		}
			break;

#ifdef RTMP_MAC_PCI
		case CMD_RTPRIV_IOCTL_IRQ_INIT:
			/* init IRQ */
			rtmp_irq_init(pAd);
			break;
#endif /* RTMP_MAC_PCI */

		case CMD_RTPRIV_IOCTL_IRQ_RELEASE:
			/* release IRQ */
			RTMP_OS_IRQ_RELEASE(pAd, pAd->net_dev);
			break;

#ifdef RTMP_MAC_PCI
		case CMD_RTPRIV_IOCTL_MSI_ENABLE:
			/* enable MSI */
			RTMP_MSI_ENABLE(pAd);
			*(ULONG **)pData = (ULONG *)(pObj->pci_dev);
			break;
#endif /* RTMP_MAC_PCI */

		case CMD_RTPRIV_IOCTL_NIC_NOT_EXIST:
			/* set driver state to fRTMP_ADAPTER_NIC_NOT_EXIST */
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST);
			break;

		case CMD_RTPRIV_IOCTL_MCU_SLEEP_CLEAR:
			RTMP_CLEAR_PSFLAG(pAd, fRTMP_PS_MCU_SLEEP);
			break;

#ifdef CONFIG_STA_SUPPORT
#ifdef CONFIG_PM
#ifdef USB_SUPPORT_SELECTIVE_SUSPEND
                case CMD_RTPRIV_IOCTL_USB_DEV_GET:
                /* get USB DEV */
                {
                        VOID **ppUsb_Dev = (VOID **)pData;
                        *ppUsb_Dev = (VOID *)(pObj->pUsb_Dev);
                }
                        break;

                case CMD_RTPRIV_IOCTL_USB_INTF_GET:
                /* get USB INTF */
                {
                        VOID **ppINTF = (VOID **)pData;
                        *ppINTF = (VOID *)(pObj->intf);
                }
                        break;

		case CMD_RTPRIV_IOCTL_ADAPTER_SUSPEND_SET:
		/* set driver state to fRTMP_ADAPTER_SUSPEND */
			RTMP_SET_FLAG(pAd,fRTMP_ADAPTER_SUSPEND);
			break;

		case CMD_RTPRIV_IOCTL_ADAPTER_SUSPEND_CLEAR:
		/* clear driver state to fRTMP_ADAPTER_SUSPEND */
			RTMP_CLEAR_FLAG(pAd,fRTMP_ADAPTER_SUSPEND);
			RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_MCU_SEND_IN_BAND_CMD);
			RTMP_CLEAR_PSFLAG(pAd, fRTMP_PS_MCU_SLEEP);
			break;

		case CMD_RTPRIV_IOCTL_ADAPTER_SEND_DISSASSOCIATE:
		/* clear driver state to fRTMP_ADAPTER_SUSPEND */
			if (INFRA_ON(pAd) &&
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
				/*				RtmpusecDelay(1000);*/
				RtmpOSWrielessEventSend(pAd->net_dev, RT_WLAN_EVENT_CGIWAP, -1, NULL, NULL, 0);
			}
			break;
			
		case CMD_RTPRIV_IOCTL_ADAPTER_SUSPEND_TEST:
		/* test driver state to fRTMP_ADAPTER_SUSPEND */
			*(UCHAR *)pData = RTMP_TEST_FLAG(pAd,fRTMP_ADAPTER_SUSPEND);
			break;

		case CMD_RTPRIV_IOCTL_ADAPTER_IDLE_RADIO_OFF_TEST:
		/* test driver state to fRTMP_ADAPTER_IDLE_RADIO_OFF */
			*(UCHAR *)pData = RTMP_TEST_FLAG(pAd,fRTMP_ADAPTER_IDLE_RADIO_OFF);
			break;

		case CMD_RTPRIV_IOCTL_ADAPTER_RT28XX_USB_ASICRADIO_OFF:
			ASIC_RADIO_OFF(pAd, SUSPEND_RADIO_OFF);
			break;

		case CMD_RTPRIV_IOCTL_ADAPTER_RT28XX_USB_ASICRADIO_ON:
			ASIC_RADIO_ON(pAd, RESUME_RADIO_ON);
			break;

#endif /* USB_SUPPORT_SELECTIVE_SUSPEND */

#if (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT)
		case CMD_RTPRIV_IOCTL_ADAPTER_RT28XX_WOW_STATUS:
			*(UCHAR *)pData = (UCHAR)pAd->WOW_Cfg.bEnable;
			break;

		case CMD_RTPRIV_IOCTL_ADAPTER_RT28XX_WOW_ENABLE:
			ASIC_WOW_ENABLE(pAd);
			break;

		case CMD_RTPRIV_IOCTL_ADAPTER_RT28XX_WOW_DISABLE:
			ASIC_WOW_DISABLE(pAd);
			break;
#endif /* (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT) */
#endif /* CONFIG_PM */	

		case CMD_RTPRIV_IOCTL_AP_BSSID_GET:
			if (pAd->StaCfg.wdev.PortSecured == WPA_802_1X_PORT_NOT_SECURED)
				NdisCopyMemory(pData, pAd->MlmeAux.Bssid, 6);
			else
				return NDIS_STATUS_FAILURE;
			break;

		case CMD_RTPRIV_IOCTL_ADAPTER_SUSPEND_SET:
		/* set driver state to fRTMP_ADAPTER_SUSPEND */
			RTMP_SET_FLAG(pAd,fRTMP_ADAPTER_SUSPEND);
			break;

		case CMD_RTPRIV_IOCTL_ADAPTER_SUSPEND_CLEAR:
		/* clear driver state to fRTMP_ADAPTER_SUSPEND */
			RTMP_CLEAR_FLAG(pAd,fRTMP_ADAPTER_SUSPEND);
			break;

		case CMD_RTPRIV_IOCTL_ADAPTER_RT28XX_USB_ASICRADIO_OFF:
		/* RT28xxUsbAsicRadioOff */
			//RT28xxUsbAsicRadioOff(pAd);
			ASIC_RADIO_OFF(pAd, SUSPEND_RADIO_OFF);
			break;

		case CMD_RTPRIV_IOCTL_ADAPTER_RT28XX_USB_ASICRADIO_ON:
		/* RT28xxUsbAsicRadioOn */
			//RT28xxUsbAsicRadioOn(pAd);
			ASIC_RADIO_ON(pAd, RESUME_RADIO_ON);
			break;
#endif /* CONFIG_STA_SUPPORT */

		case CMD_RTPRIV_IOCTL_SANITY_CHECK:
		/* sanity check before IOCTL */
			if ((!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE))
#ifdef IFUP_IN_PROBE
			|| (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS))
			|| (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS))
			|| (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
#endif /* IFUP_IN_PROBE */
			)
			{
				if(pData == NULL ||	RT_isLegalCmdBeforeInfUp((PSTRING) pData) == FALSE)
				return NDIS_STATUS_FAILURE;
			}
			break;

		case CMD_RTPRIV_IOCTL_SIOCGIWFREQ:
		/* get channel number */
			*(ULONG *)pData = pAd->CommonCfg.Channel;
			break;
#ifdef CONFIG_SNIFFER_SUPPORT
		case CMD_RTPRIV_IOCTL_SNIFF_INIT:
			Monitor_Init(pAd, pData);
			break;

		case CMD_RTPRIV_IOCTL_SNIFF_OPEN:
			if (Monitor_Open(pAd, pData) != TRUE)
				return NDIS_STATUS_FAILURE;
			break;

		case CMD_RTPRIV_IOCTL_SNIFF_CLOSE:
			if (Monitor_Close(pAd, pData) != TRUE)
				return NDIS_STATUS_FAILURE;
			break;

		case CMD_RTPRIV_IOCTL_SNIFF_REMOVE:
			Monitor_Remove(pAd);
			break;
#endif /*CONFIG_SNIFFER_SUPPORT*/



		case CMD_RTPRIV_IOCTL_BEACON_UPDATE:
		/* update all beacon contents */
#ifdef CONFIG_AP_SUPPORT
			APMakeAllBssBeacon(pAd);
			APUpdateAllBeaconFrame(pAd);
#endif /* CONFIG_AP_SUPPORT */
			break;

		case CMD_RTPRIV_IOCTL_RXPATH_GET:
		/* get the number of rx path */
			*(ULONG *)pData = pAd->Antenna.field.RxPath;
			break;

		case CMD_RTPRIV_IOCTL_CHAN_LIST_NUM_GET:
			*(ULONG *)pData = pAd->ChannelListNum;
			break;

		case CMD_RTPRIV_IOCTL_CHAN_LIST_GET:
		{
			UINT32 i;
			UCHAR *pChannel = (UCHAR *)pData;

			for (i = 1; i <= pAd->ChannelListNum; i++)
			{
				*pChannel = pAd->ChannelList[i-1].Channel;
				pChannel ++;
			}
		}
			break;

		case CMD_RTPRIV_IOCTL_FREQ_LIST_GET:
		{
			UINT32 i;
			UINT32 *pFreq = (UINT32 *)pData;
			UINT32 m;

			for (i = 1; i <= pAd->ChannelListNum; i++)
			{
				m = 2412000;
				MAP_CHANNEL_ID_TO_KHZ(pAd->ChannelList[i-1].Channel, m);
				(*pFreq) = m;
				pFreq ++;
			}
		}
			break;

#ifdef EXT_BUILD_CHANNEL_LIST
       case CMD_RTPRIV_SET_PRECONFIG_VALUE:
       /* Set some preconfigured value before interface up*/
           pAd->CommonCfg.DfsType = MAX_RD_REGION;
           break;
#endif /* EXT_BUILD_CHANNEL_LIST */



#ifdef RTMP_PCI_SUPPORT
		case CMD_RTPRIV_IOCTL_PCI_SUSPEND:
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF);
			break;

		case CMD_RTPRIV_IOCTL_PCI_RESUME:
			RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);
			RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF);
			break;

		case CMD_RTPRIV_IOCTL_PCI_CSR_SET:
			pAd->CSRBaseAddress = (PUCHAR)Data;
			DBGPRINT(RT_DEBUG_ERROR, ("pAd->CSRBaseAddress =0x%lx, csr_addr=0x%lx!\n", (ULONG)pAd->CSRBaseAddress, (ULONG)Data));
			break;

		case CMD_RTPRIV_IOCTL_PCIE_INIT:
			RTMPInitPCIeDevice(pData, pAd);
			break;
#endif /* RTMP_PCI_SUPPORT */

#ifdef RT_CFG80211_SUPPORT
		case CMD_RTPRIV_IOCTL_CFG80211_CFG_START:
			RT_CFG80211_REINIT(pAd);
			RT_CFG80211_CRDA_REG_RULE_APPLY(pAd);
			break;
#endif /* RT_CFG80211_SUPPORT */

#ifdef INF_PPA_SUPPORT
		case CMD_RTPRIV_IOCTL_INF_PPA_INIT:
			os_alloc_mem(NULL, (UCHAR **)&(pAd->pDirectpathCb), sizeof(PPA_DIRECTPATH_CB));
			break;

		case CMD_RTPRIV_IOCTL_INF_PPA_EXIT:
			if (ppa_hook_directpath_register_dev_fn && (pAd->PPAEnable == TRUE))
			{
				UINT status;
				status = ppa_hook_directpath_register_dev_fn(&pAd->g_if_id, pAd->net_dev, NULL, 0);
				DBGPRINT(RT_DEBUG_TRACE, ("Unregister PPA::status=%d, if_id=%d\n", status, pAd->g_if_id));
			}
			os_free_mem(NULL, pAd->pDirectpathCb);
			break;
#endif /* INF_PPA_SUPPORT*/

		case CMD_RTPRIV_IOCTL_VIRTUAL_INF_UP:
		/* interface up */
		{
			RT_CMD_INF_UP_DOWN *pInfConf = (RT_CMD_INF_UP_DOWN *)pData;

			if (VIRTUAL_IF_NUM(pAd) == 0)
			{
#ifdef DBG
                ULONG start, end, diff_ms;
                /* Get the current time for calculating startup time */
                NdisGetSystemUpTime(&start);
#endif /* DBG */	
				if (pInfConf->rt28xx_open(pAd->net_dev) != 0)
				{
					DBGPRINT(RT_DEBUG_TRACE, ("rt28xx_open return fail!\n"));
					return NDIS_STATUS_FAILURE;
				}
#ifdef DBG				
                /* Get the current time for calculating startup time */
                NdisGetSystemUpTime(&end); diff_ms = (end-start)*1000/OS_HZ;
                DBGPRINT(RT_DEBUG_OFF, ("WiFi Interface Up Time (%s): %lu.%03lus\n",
                        RTMP_OS_NETDEV_GET_DEVNAME(pAd->net_dev),diff_ms/1000,diff_ms%1000));
#endif /* DBG */
			}
			else
			{
#ifdef CONFIG_AP_SUPPORT
				extern VOID APMakeAllBssBeacon(IN PRTMP_ADAPTER pAd);
				extern VOID  APUpdateAllBeaconFrame(IN PRTMP_ADAPTER pAd);
				APMakeAllBssBeacon(pAd);
				APUpdateAllBeaconFrame(pAd);
#endif /* CONFIG_AP_SUPPORT */
			}
			VIRTUAL_IF_INC(pAd);
		}
			break;

		case CMD_RTPRIV_IOCTL_VIRTUAL_INF_DOWN:
		/* interface down */
		{
			RT_CMD_INF_UP_DOWN *pInfConf = (RT_CMD_INF_UP_DOWN *)pData;

			VIRTUAL_IF_DEC(pAd);
			if (VIRTUAL_IF_NUM(pAd) == 0)
            {
#ifdef DBG
                ULONG start, end, diff_ms;
                /* Get the current time for calculating startup time */
                NdisGetSystemUpTime(&start);
#endif /* DBG */	            
				pInfConf->rt28xx_close(pAd->net_dev);
#ifdef DBG				
                /* Get the current time for calculating startup time */
                NdisGetSystemUpTime(&end); diff_ms = (end-start)*1000/OS_HZ;
                DBGPRINT(RT_DEBUG_OFF, ("WiFi Interface Down Time (%s): %lu.%03lus\n",
                        RTMP_OS_NETDEV_GET_DEVNAME(pAd->net_dev),diff_ms/1000,diff_ms%1000));
#endif /* DBG */
            }
		}
			break;

		case CMD_RTPRIV_IOCTL_VIRTUAL_INF_GET:
		/* get virtual interface number */
			*(ULONG *)pData = VIRTUAL_IF_NUM(pAd);
			break;

		case CMD_RTPRIV_IOCTL_INF_TYPE_GET:
		/* get current interface type */
			*(ULONG *)pData = pAd->infType;
			break;

		case CMD_RTPRIV_IOCTL_INF_STATS_GET:
			/* get statistics */
			{			
				RT_CMD_STATS64 *pStats = (RT_CMD_STATS64 *)pData;
#ifdef CONFIG_AP_SUPPORT
				if(pAd->OpMode == OPMODE_AP)
				{
					INT index;
					MULTISSID_STRUCT *pMBSSID;
					
					for(index = 0; index < MAX_MBSSID_NUM(pAd); index++)
					{
						if (pAd->ApCfg.MBSSID[index].wdev.if_dev == (PNET_DEV)(pStats->pNetDev))
						{
							break;
						}
					}
						
					if(index >= MAX_MBSSID_NUM(pAd))
					{
						//reset counters
						NdisZeroMemory(pStats, sizeof(RT_CMD_STATS64));
						   
						DBGPRINT(RT_DEBUG_ERROR, ("CMD_RTPRIV_IOCTL_INF_STATS_GET: can not find mbss I/F\n"));
						return NDIS_STATUS_FAILURE;
					}
					
					pMBSSID = &pAd->ApCfg.MBSSID[index];
					
					pStats->rx_bytes = pMBSSID->ReceivedByteCount.QuadPart;
					pStats->tx_bytes = pMBSSID->TransmittedByteCount.QuadPart;
					pStats->rx_packets = pMBSSID->RxCount;
					pStats->tx_packets = pMBSSID->TxCount;
					pStats->rx_errors = pMBSSID->RxErrorCount;
					pStats->tx_errors = 0;
					pStats->multicast = pMBSSID->mcPktsRx; /* multicast packets received */
					pStats->collisions = 0;
					pStats->rx_over_errors = 0;
					pStats->rx_crc_errors = 0;
					pStats->rx_frame_errors = 0;
					pStats->rx_fifo_errors = 0;
				}
#endif
#ifdef CONFIG_STA_SUPPORT
				if(pAd->OpMode == OPMODE_STA)
				{
					pStats->rx_packets = pAd->WlanCounters.ReceivedFragmentCount.QuadPart;
					pStats->tx_packets = pAd->WlanCounters.TransmittedFragmentCount.QuadPart;
					pStats->rx_bytes = pAd->RalinkCounters.ReceivedByteCount;
					pStats->tx_bytes = pAd->RalinkCounters.TransmittedByteCount;
					pStats->rx_errors = pAd->Counters8023.RxErrors;
					pStats->tx_errors = pAd->Counters8023.TxErrors;
					pStats->multicast = pAd->WlanCounters.MulticastReceivedFrameCount.QuadPart;   /* multicast packets received*/
					pStats->collisions = 0;  /* Collision packets*/
					pStats->rx_over_errors = pAd->Counters8023.RxNoBuffer;                   /* receiver ring buff overflow*/
					pStats->rx_crc_errors = 0;/*pAd->WlanCounters.FCSErrorCount;      recved pkt with crc error*/
					pStats->rx_frame_errors = 0;          /* recv'd frame alignment error*/
					pStats->rx_fifo_errors = pAd->Counters8023.RxNoBuffer;                   /* recv'r fifo overrun*/
				}
#endif
			}
			break;

		case CMD_RTPRIV_IOCTL_INF_IW_STATUS_GET:
		/* get wireless statistics */
		{
			UCHAR CurOpMode = OPMODE_AP;
#ifdef CONFIG_AP_SUPPORT 
			PMAC_TABLE_ENTRY pMacEntry = NULL;
#endif /* CONFIG_AP_SUPPORT */
			RT_CMD_IW_STATS *pStats = (RT_CMD_IW_STATS *)pData;

			pStats->qual = 0;
			pStats->level = 0;
			pStats->noise = 0;
			pStats->pStats = pAd->iw_stats;
			
#ifdef CONFIG_STA_SUPPORT
			if (pAd->OpMode == OPMODE_STA)
			{
				CurOpMode = OPMODE_STA;
			}
#endif /* CONFIG_STA_SUPPORT */

			/*check if the interface is down*/
			if(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE))
				return NDIS_STATUS_FAILURE;	

#ifdef CONFIG_AP_SUPPORT
			if (CurOpMode == OPMODE_AP)
			{
#ifdef APCLI_SUPPORT
				if ((pStats->priv_flags == INT_APCLI)
					)
				{
					INT ApCliIdx = ApCliIfLookUp(pAd, (PUCHAR)pStats->dev_addr);
					if ((ApCliIdx >= 0) && VALID_WCID(pAd->ApCfg.ApCliTab[ApCliIdx].MacTabWCID))
						pMacEntry = &pAd->MacTab.Content[pAd->ApCfg.ApCliTab[ApCliIdx].MacTabWCID];
				}
				else
#endif /* APCLI_SUPPORT */
				{
					/*
						only AP client support wireless stats function.
						return NULL pointer for all other cases.
					*/
					pMacEntry = NULL;
				}
			}
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
			if (CurOpMode == OPMODE_STA)
				pStats->qual = ((pAd->Mlme.ChannelQuality * 12)/10 + 10);
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
			if (CurOpMode == OPMODE_AP)
			{
				if (pMacEntry != NULL)
					pStats->qual = ((pMacEntry->ChannelQuality * 12)/10 + 10);
				else
					pStats->qual = ((pAd->Mlme.ChannelQuality * 12)/10 + 10);
			}
#endif /* CONFIG_AP_SUPPORT */

			if (pStats->qual > 100)
				pStats->qual = 100;

#ifdef CONFIG_STA_SUPPORT
			if (CurOpMode == OPMODE_STA)
			{
				pStats->level =
					RTMPMaxRssi(pAd, pAd->StaCfg.RssiSample.AvgRssi0,
									pAd->StaCfg.RssiSample.AvgRssi1,
									pAd->StaCfg.RssiSample.AvgRssi2);
			}
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
			if (CurOpMode == OPMODE_AP)
			{
				if (pMacEntry != NULL)
					pStats->level =
						RTMPMaxRssi(pAd, pMacEntry->RssiSample.AvgRssi0,
										pMacEntry->RssiSample.AvgRssi1,
										pMacEntry->RssiSample.AvgRssi2);
			}
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
			pStats->noise = RTMPMaxRssi(pAd, pAd->ApCfg.RssiSample.AvgRssi0,
										pAd->ApCfg.RssiSample.AvgRssi1,
										pAd->ApCfg.RssiSample.AvgRssi2) -
										RTMPMinSnr(pAd, pAd->ApCfg.RssiSample.AvgSnr0,
										pAd->ApCfg.RssiSample.AvgSnr1);
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
			pStats->noise = RTMPMaxRssi(pAd, pAd->StaCfg.RssiSample.AvgRssi0,
										pAd->StaCfg.RssiSample.AvgRssi1,
										pAd->StaCfg.RssiSample.AvgRssi2) - 
										RTMPMinSnr(pAd, pAd->StaCfg.RssiSample.AvgSnr0, 
										pAd->StaCfg.RssiSample.AvgSnr1);
#endif /* CONFIG_STA_SUPPORT */
		}
			break;

		case CMD_RTPRIV_IOCTL_INF_MAIN_CREATE:
			*(VOID **)pData = RtmpPhyNetDevMainCreate(pAd);
			break;

		case CMD_RTPRIV_IOCTL_INF_MAIN_ID_GET:
			*(ULONG *)pData = INT_MAIN;
			break;

		case CMD_RTPRIV_IOCTL_INF_MAIN_CHECK:
			if (Data != INT_MAIN)
				return NDIS_STATUS_FAILURE;
			break;

		case CMD_RTPRIV_IOCTL_INF_P2P_CHECK:
			if (Data != INT_P2P)
				return NDIS_STATUS_FAILURE;
			break;

#ifdef WDS_SUPPORT
		case CMD_RTPRIV_IOCTL_WDS_INIT:
			WDS_Init(pAd, pData);
			break;

		case CMD_RTPRIV_IOCTL_WDS_REMOVE:
			WDS_Remove(pAd);
			break;

		case CMD_RTPRIV_IOCTL_WDS_STATS_GET:
			if (Data == INT_WDS)
			{
				if (WDS_StatsGet(pAd, pData) != TRUE)
					return NDIS_STATUS_FAILURE;
			}
			else
				return NDIS_STATUS_FAILURE;
			break;
#endif /* WDS_SUPPORT */

#ifdef APCLI_SUPPORT
		case CMD_RTPRIV_IOCTL_APCLI_STATS_GET:
			if (Data == INT_APCLI)
			{
				if (ApCli_StatsGet(pAd, pData) != TRUE)
					return NDIS_STATUS_FAILURE;
			}
			else
				return NDIS_STATUS_FAILURE;
			break;
#endif /* APCLI_SUPPORT */

#ifdef RALINK_ATE
#ifdef RALINK_QA
		case CMD_RTPRIV_IOCTL_ATE:
			RtmpDoAte(pAd, wrq, pData);
			break;
#endif /* RALINK_QA */ 
#endif /* RALINK_ATE */

		case CMD_RTPRIV_IOCTL_MAC_ADDR_GET:
			{
				UCHAR mac_addr[MAC_ADDR_LEN];
				USHORT Addr01, Addr23, Addr45;
					
				RT28xx_EEPROM_READ16(pAd, 0x04, Addr01);
				RT28xx_EEPROM_READ16(pAd, 0x06, Addr23);
				RT28xx_EEPROM_READ16(pAd, 0x08, Addr45);			
			
				mac_addr[0] = (UCHAR)(Addr01 & 0xff);
				mac_addr[1] = (UCHAR)(Addr01 >> 8);
				mac_addr[2] = (UCHAR)(Addr23 & 0xff);
				mac_addr[3] = (UCHAR)(Addr23 >> 8);
				mac_addr[4] = (UCHAR)(Addr45 & 0xff);
				mac_addr[5] = (UCHAR)(Addr45 >> 8);
			
				for(i=0; i<6; i++)
					*(UCHAR *)(pData+i) = mac_addr[i];
				break;
			}
#ifdef CONFIG_AP_SUPPORT
		case CMD_RTPRIV_IOCTL_AP_SIOCGIWRATEQ:
		/* handle for SIOCGIWRATEQ */
		{
			RT_CMD_IOCTL_RATE *pRate = (RT_CMD_IOCTL_RATE *)pData;
			HTTRANSMIT_SETTING HtPhyMode;
			UINT8 BW = 0, GI = 0;

#ifdef APCLI_SUPPORT
			if (pRate->priv_flags == INT_APCLI)
				memcpy(&HtPhyMode, &pAd->ApCfg.ApCliTab[pObj->ioctl_if].wdev.HTPhyMode, sizeof(HTTRANSMIT_SETTING));
			else
#endif /* APCLI_SUPPORT */
#ifdef WDS_SUPPORT
			if (pRate->priv_flags == INT_WDS)
				memcpy(&HtPhyMode, &pAd->WdsTab.WdsEntry[pObj->ioctl_if].wdev.HTPhyMode, sizeof(HTTRANSMIT_SETTING));
			else
#endif /* WDS_SUPPORT */
			{
				memcpy(&HtPhyMode, &pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev.HTPhyMode, sizeof(HTTRANSMIT_SETTING));
#ifdef MBSS_SUPPORT
				/* reset phy mode for MBSS */
				MBSS_PHY_MODE_RESET(pObj->ioctl_if, HtPhyMode);
#endif /* MBSS_SUPPORT */
			}

#ifdef DOT11_VHT_AC
			if (HtPhyMode.field.BW == BW_40 && pAd->CommonCfg.vht_bw == VHT_BW_80 && HtPhyMode.field.MODE >= MODE_VHT) {
				BW = 2;
				GI = pAd->CommonCfg.vht_sgi_80;
			}
			else
#endif /* DOT11_VHT_AC */
			{
				BW = HtPhyMode.field.BW;
				GI = HtPhyMode.field.ShortGI;
			}

			RtmpDrvRateGet(pAd, HtPhyMode.field.MODE, GI,
							BW, HtPhyMode.field.MCS,
							pAd->Antenna.field.TxPath,
							(UINT32 *)&pRate->BitRate);
		}
			break;
#endif /* CONFIG_AP_SUPPORT */

		case CMD_RTPRIV_IOCTL_SIOCGIWNAME:
			RtmpIoctl_rt_ioctl_giwname(pAd, pData, 0);
			break;

	}

#ifdef RT_CFG80211_SUPPORT
	if ((CMD_RTPRIV_IOCTL_80211_START <= cmd) &&
		(cmd <= CMD_RTPRIV_IOCTL_80211_END))
	{
		Status = CFG80211DRV_IoctlHandle(pAd, wrq, cmd, subcmd, pData, Data);
	}
#endif /* RT_CFG80211_SUPPORT */

	if (cmd >= CMD_RTPRIV_IOCTL_80211_COM_LATEST_ONE)
		return NDIS_STATUS_FAILURE;

	return Status;
}

/* 
    ==========================================================================
    Description:
        Issue a site survey command to driver
	Arguments:
	    pAdapter                    Pointer to our adapter
	    wrq                         Pointer to the ioctl argument

    Return Value:
        None

    Note:
        Usage: 
               1.) iwpriv ra0 set site_survey
    ==========================================================================
*/
INT Set_SiteSurvey_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	NDIS_802_11_SSID Ssid;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;

	//check if the interface is down
	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE))
	{
		DBGPRINT(RT_DEBUG_TRACE, ("INFO::Network is down!\n"));
		return -ENETDOWN;   
	}

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		if (MONITOR_ON(pAd))
    	{
        	DBGPRINT(RT_DEBUG_TRACE, ("!!! Driver is in Monitor Mode now !!!\n"));
        	return -EINVAL;
    	}
	}
#endif // CONFIG_STA_SUPPORT //

    NdisZeroMemory(&Ssid, sizeof(NDIS_802_11_SSID));

#ifdef CONFIG_AP_SUPPORT
#ifdef AP_SCAN_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		if ((strlen(arg) != 0) && (strlen(arg) <= MAX_LEN_OF_SSID))
    	{
        	NdisMoveMemory(Ssid.Ssid, arg, strlen(arg));
        	Ssid.SsidLength = strlen(arg);
		}
#ifdef CUSTOMER_DCC_FEATURE
		{
			UINT32	mac_val;
			RTMP_IO_READ32(pAd, CH_TIME_CFG, &mac_val);
			mac_val |= 0x11f;
			RTMP_IO_WRITE32(pAd, CH_TIME_CFG, mac_val);
		}
#endif

#ifdef AIRPLAY_SUPPORT 
		if(arg[0] == '0' && (arg[1] == 'x' || arg[1] == 'X'))
		{
				int ii,jj;
				CHAR temp[MAX_LEN_OF_SSID*2+1];

				NdisZeroMemory(Ssid.Ssid, MAX_LEN_OF_SSID);
				for(ii=2; ii<strlen(arg); ii++)
				{
						if(arg[ii] >= '0' && arg[ii] <= '9')
								temp[ii-2] = arg[ii] - '0';
						else if(arg[ii] >= 'A' && arg[ii] <= 'F')
								temp[ii-2] = arg[ii] - 'A' + 10;
						else if(arg[ii] >= 'a' && arg[ii] <= 'f')
								temp[ii-2] = arg[ii] - 'a' + 10;
				}

				temp[strlen(arg)-2]= '\0';
				DBGPRINT(RT_DEBUG_TRACE,("%s=>arg:",__FUNCTION__));
				for(ii=0; ii<strlen(arg)-2; ii++)
						DBGPRINT(RT_DEBUG_TRACE,("%x",temp[ii]));
				DBGPRINT(RT_DEBUG_TRACE,("\n"));

				jj=0;
				for(ii=0; ii<strlen(arg)-2; ii+=2)
				{
						if (jj > MAX_LEN_OF_SSID)
						{
								DBGPRINT(RT_DEBUG_TRACE, ("%s=> unicode SSID len error.",__FUNCTION__));
								NdisZeroMemory(&Ssid, sizeof(NDIS_802_11_SSID));
								goto ret;
						}

						Ssid.Ssid[jj++] = (UCHAR)(temp[ii]*16+temp[ii+1]);
				}
				Ssid.Ssid[jj] = '\0';
				Ssid.SsidLength = jj;

ret:

				DBGPRINT(RT_DEBUG_TRACE, ("%s=>SSID:",__FUNCTION__));
				for(ii=0; ii<jj; ii++)
						DBGPRINT(RT_DEBUG_TRACE,("%x",(UCHAR)Ssid.Ssid[ii]));
				DBGPRINT(RT_DEBUG_TRACE,("\n"));
		}
#endif /* AIRPLAY_SUPPORT */

#ifndef APCLI_CONNECTION_TRIAL
#ifdef APCLI_SUPPORT
		if (pObj->ioctl_if_type == INT_APCLI)
		{
			ApCliSiteSurvey(pAd, pObj->ioctl_if, &Ssid, SCAN_ACTIVE, FALSE);
		}
		else
#endif /* APCLI_SUPPORT */
		{
			if (Ssid.SsidLength == 0)
				ApSiteSurvey(pAd, &Ssid, SCAN_PASSIVE, FALSE);
			else
				ApSiteSurvey(pAd, &Ssid, SCAN_ACTIVE, FALSE);
		}
#else
		/*for shorter scan time. use active scan and send probe req.*/
		DBGPRINT(RT_DEBUG_TRACE, ("!!! Fast Scan for connection trial !!!\n"));
		ApSiteSurvey(pAd, &Ssid, FAST_SCAN_ACTIVE, FALSE);
#endif /* APCLI_CONNECTION_TRIAL */

		return TRUE;
	}
#endif /* AP_SCAN_SUPPORT */
#endif // CONFIG_AP_SUPPORT //

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		Ssid.SsidLength = 0; 
		if ((arg != NULL) &&
			(strlen(arg) <= MAX_LEN_OF_SSID))
		{
			RTMPMoveMemory(Ssid.Ssid, arg, strlen(arg));
			Ssid.SsidLength = strlen(arg);
		}

		pAd->StaCfg.bSkipAutoScanConn = TRUE;
		StaSiteSurvey(pAd, &Ssid, SCAN_ACTIVE);
	}
#endif // CONFIG_STA_SUPPORT //

	DBGPRINT(RT_DEBUG_TRACE, ("Set_SiteSurvey_Proc\n"));

    return TRUE;
}

INT	Set_Antenna_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	ANT_DIVERSITY_TYPE UsedAnt;
	int i;
	DBGPRINT(RT_DEBUG_OFF, ("==> Set_Antenna_Proc *******************\n"));

	for (i = 0; i < strlen(arg); i++)
		if (!isdigit(arg[i]))
			return -EINVAL;

	UsedAnt = simple_strtol(arg, 0, 10);

	switch (UsedAnt)
	{
		/* 2: Fix in the PHY Antenna CON1*/
		case ANT_FIX_ANT0:
			AsicSetRxAnt(pAd, 0);
			DBGPRINT(RT_DEBUG_OFF, ("<== Set_Antenna_Proc(Fix in Ant CON1), (%d,%d)\n", 
					pAd->RxAnt.Pair1PrimaryRxAnt, pAd->RxAnt.Pair1SecondaryRxAnt));
			break;
    	/* 3: Fix in the PHY Antenna CON2*/
		case ANT_FIX_ANT1:
			AsicSetRxAnt(pAd, 1);
			DBGPRINT(RT_DEBUG_OFF, ("<== %s(Fix in Ant CON2), (%d,%d)\n", 
							__FUNCTION__, pAd->RxAnt.Pair1PrimaryRxAnt, pAd->RxAnt.Pair1SecondaryRxAnt));
			break;
		default:
			DBGPRINT(RT_DEBUG_ERROR, ("<== %s(N/A cmd: %d), (%d,%d)\n", __FUNCTION__, UsedAnt,
					pAd->RxAnt.Pair1PrimaryRxAnt, pAd->RxAnt.Pair1SecondaryRxAnt));
			break;
	}
	
	return TRUE;
}
			



#ifdef RT6352
INT Set_RfBankSel_Proc(
    IN  PRTMP_ADAPTER pAd, 
    IN  PSTRING	arg)
{
	LONG RfBank;

	RfBank = simple_strtol(arg, 0, 10);

	pAd->RfBank = RfBank;

	return TRUE;
}

#ifdef RTMP_TEMPERATURE_CALIBRATION
INT Set_TemperatureCAL_Proc(
	IN	PRTMP_ADAPTER pAd, 
	IN	PSTRING arg)
{
	RT6352_Temperature_Init(pAd);
	return TRUE;
}
#endif /* RTMP_TEMPERATURE_CALIBRATION */
#endif /* RT6352 */

#ifdef MAC_REPEATER_SUPPORT
#ifdef MULTI_MAC_ADDR_EXT_SUPPORT
INT Set_EnMultiMacAddrExt_Proc(
	IN RTMP_ADAPTER	*pAd,
	IN PSTRING arg)
{
	UCHAR Enable;
	UINT32 MacReg;

	Enable = simple_strtol(arg, 0, 10);

	RTMP_IO_READ32(pAd, MAC_ADDR_EXT_EN, &MacReg);
	if (Enable)
	{
		MacReg |= 0x1;
		pAd->bUseMultiMacAddrExt = TRUE;
		DBGPRINT(RT_DEBUG_TRACE, ("==>UseMultiMacAddrExt (ON)\n"));
	}
	else
	{
		MacReg &= (~0x1);
		pAd->bUseMultiMacAddrExt = FALSE;
		DBGPRINT(RT_DEBUG_TRACE, ("==>UseMultiMacAddrExt (OFF)\n"));
	}
	RTMP_IO_WRITE32(pAd, MAC_ADDR_EXT_EN, MacReg);

	DBGPRINT(RT_DEBUG_WARN, ("UseMultiMacAddrExt = %d \n", pAd->bUseMultiMacAddrExt));

	return TRUE;
}

INT	Set_MultiMacAddrExt_Proc(
	IN	PRTMP_ADAPTER pAd, 
	IN	PSTRING arg)
{
	UCHAR tempMAC[6], idx;
	PSTRING token;
	STRING sepValue[] = ":", DASH = '-';
	ULONG offset, Addr;
	INT i;
	
	if(strlen(arg) < 19)  /*Mac address acceptable format 01:02:03:04:05:06 length 17 plus the "-" and tid value in decimal format.*/
		return FALSE;

	token = strchr(arg, DASH);
	if ((token != NULL) && (strlen(token)>1))
	{
		idx = (UCHAR) simple_strtol((token+1), 0, 10);

		if (idx > 15)
			return FALSE;
		
		*token = '\0';
		for (i = 0, token = rstrtok(arg, &sepValue[0]); token; token = rstrtok(NULL, &sepValue[0]), i++)
		{
			if((strlen(token) != 2) || (!isxdigit(*token)) || (!isxdigit(*(token+1))))
				return FALSE;
			AtoH(token, (&tempMAC[i]), 1);
		}

		if(i != 6)
			return FALSE;

		DBGPRINT(RT_DEBUG_OFF, ("\n%02x:%02x:%02x:%02x:%02x:%02x-%02x\n", 
								tempMAC[0], tempMAC[1], tempMAC[2], tempMAC[3], tempMAC[4], tempMAC[5], idx));

		offset = 0x1480 + (HW_WCID_ENTRY_SIZE * idx);	
		Addr = tempMAC[0] + (tempMAC[1] << 8) +(tempMAC[2] << 16) +(tempMAC[3] << 24);
		RTMP_IO_WRITE32(pAd, offset, Addr);
		Addr = tempMAC[4] + (tempMAC[5] << 8);
		RTMP_IO_WRITE32(pAd, offset + 4, Addr);	

		return TRUE;
	}

	return FALSE;
}
#endif /* MULTI_MAC_ADDR_EXT_SUPPORT */
#endif /* MAC_REPEATER_SUPPORT */

INT set_tssi_enable(RTMP_ADAPTER *pAd, PSTRING arg)
{
	UINT8 tssi_enable = 0;

	tssi_enable = simple_strtol(arg, 0, 10);

	if (tssi_enable == 1) {
		pAd->chipCap.tssi_enable = TRUE;
		DBGPRINT(RT_DEBUG_OFF, ("turn on TSSI mechanism\n")); 
	} else if (tssi_enable == 0) {
		pAd->chipCap.tssi_enable = FALSE;
		DBGPRINT(RT_DEBUG_OFF, ("turn off TSS mechanism\n"));
	} else {
		DBGPRINT(RT_DEBUG_OFF, ("illegal param(%u)\n", tssi_enable));
		return FALSE;
    }
	return TRUE;
}

#ifdef CONFIG_WIFI_TEST
INT set_pbf_loopback(RTMP_ADAPTER *pAd, PSTRING arg)
{
	UINT8 enable = 0;
	UINT32 value;

	enable = simple_strtol(arg, 0, 10);
	RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &value);

	if (enable == 1) {
		pAd->chipCap.pbf_loopback = TRUE;
		value |= PBF_LOOP_EN;
		DBGPRINT(RT_DEBUG_OFF, ("turn on pbf loopback\n"));
	} else if(enable == 0) {
		pAd->chipCap.pbf_loopback = FALSE;
		value &= ~PBF_LOOP_EN;
		DBGPRINT(RT_DEBUG_OFF, ("turn off pbf loopback\n"));
	} else {
		DBGPRINT(RT_DEBUG_OFF, ("illegal param(%d)\n"));
		return FALSE;
	}

	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, value);

	return TRUE;
}

INT set_pbf_rx_drop(RTMP_ADAPTER *pAd, PSTRING arg)
{
	UINT8 enable = 0;
	UINT32 value;

	enable = simple_strtol(arg, 0, 10);

#ifdef RLT_MAC
	if (pAd->chipCap.hif_type == HIF_RLT)
		RTMP_IO_READ32(pAd, RLT_PBF_CFG, &value);
#endif

	if (enable == 1) {
		pAd->chipCap.pbf_rx_drop = TRUE;
		value |= RX_DROP_MODE;
		DBGPRINT(RT_DEBUG_OFF, ("turn on pbf loopback\n"));
	} else if (enable == 0) {
		pAd->chipCap.pbf_rx_drop = FALSE;
		value &= ~RX_DROP_MODE;
		DBGPRINT(RT_DEBUG_OFF, ("turn off pbf loopback\n"));
	} else {
		DBGPRINT(RT_DEBUG_OFF, ("illegal param(%d)\n"));
		return FALSE;
	}

#ifdef RLT_MAC
	if (pAd->chipCap.hif_type == HIF_RLT)
		RTMP_IO_WRITE32(pAd, RLT_PBF_CFG, value);
#endif

	return TRUE;
}
#endif

#ifdef CONFIG_ANDES_SUPPORT
INT set_fw_debug(RTMP_ADAPTER *ad, PSTRING arg)
{
	UINT8 fw_debug_param;

	fw_debug_param = simple_strtol(arg, 0, 10);

	andes_fun_set(ad, LOG_FW_DEBUG_MSG, fw_debug_param);

	return TRUE;
}
#endif /* CONFIG_ANDES_SUPPORT */

#ifdef CUSTOMER_DCC_FEATURE
VOID EnableRadioChstats(
		IN	PRTMP_ADAPTER	pAd,
		IN	UINT32		mac_val)
{
	mac_val |= 0x15f; // enable channel status check
	RTMP_IO_WRITE32(pAd, CH_TIME_CFG, mac_val);
	/* Clear previous status */
	RTMP_IO_READ32(pAd, CH_BUSY_STA, &mac_val);
	RTMP_IO_READ32(pAd, CCA_BUSY_TIME, &mac_val);
	RTMP_IO_READ32(pAd, RX_STA_CNT1, &mac_val);
}

INT Set_ApEnableRadioChStats(
		IN	PRTMP_ADAPTER	pAd, 
		IN	PSTRING 	arg)
{	
	INT32			enable;
	static UINT32	save_mac_val=0;
	UINT32			mac_val;
	
	if(strlen(arg) > 1)
	{
		DBGPRINT(RT_DEBUG_ERROR,("wrong argument type   \n"));
		return FALSE;	
	}
	enable = simple_strtol(arg, 0, 10);
	if((enable != 1) && (enable != 0))
	{
		DBGPRINT(RT_DEBUG_ERROR,("wrong argument value   \n"));
		return FALSE;
	}
	
	if(enable == 1)
	{
				
		/* set the EnableChannelStatsCheck value to true and initialize the values to zero */
		pAd->EnableChannelStatsCheck = TRUE;
		
		pAd->ChannelStats.LastReadTime = 0;
		pAd->ChannelStats.TotalDuration = 0;
		pAd->ChannelStats.msec100counts = 0;
		
		pAd->ChannelStats.CCABusytime = 0;
		pAd->ChannelStats.ChBusytime = 0;
		pAd->ChannelStats.FalseCCACount = 0;
		pAd->ChannelStats.ChannelApActivity = 0;
		
		pAd->ChannelStats.ChBusyTimeAvg = 0;
		pAd->ChannelStats.CCABusyTimeAvg = 0;
		pAd->ChannelStats.FalseCCACountAvg = 0;
		pAd->ChannelStats.ChannelApActivityAvg = 0;
		
		pAd->ChannelStats.ChBusyTime1secValue = 0;
		pAd->ChannelStats.CCABusyTime1secValue = 0;
		pAd->ChannelStats.FalseCCACount1secValue = 0;
		pAd->ChannelStats.ChannelApActivity1secValue = 0;
		
		/* enable the radio channel stats and save the current status value */
		RTMP_IO_READ32(pAd, CH_TIME_CFG, &save_mac_val);
		mac_val = save_mac_val;
		EnableRadioChstats(pAd, mac_val);		
		
		return TRUE;
	}
	else if(enable == 0)
	{
		if(pAd->EnableChannelStatsCheck)
		{
			/* set the old values of register before enabling the radio chanel stats and set the EnableChannelStatsCheck to FALSE*/
			RTMP_IO_WRITE32(pAd, CH_TIME_CFG, save_mac_val);
			pAd->EnableChannelStatsCheck = FALSE;
		}
		/* set all channel stats values to zero */
		pAd->ChannelStats.TotalDuration = 0;
		pAd->ChannelStats.msec100counts = 0;

		pAd->ChannelStats.CCABusytime = 0;
		pAd->ChannelStats.ChBusytime = 0;
		pAd->ChannelStats.FalseCCACount = 0;
		pAd->ChannelStats.ChannelApActivity = 0;
		
		pAd->ChannelStats.ChBusyTimeAvg = 0;
		pAd->ChannelStats.CCABusyTimeAvg = 0;
		pAd->ChannelStats.FalseCCACountAvg = 0;
		pAd->ChannelStats.ChannelApActivityAvg = 0;
		
		return TRUE;
	}
	
	return FALSE;
}


INT Set_ApDisableSTAConnect_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	INT32 flag;
	if(strlen(arg) > 1)
	{
		DBGPRINT(RT_DEBUG_ERROR,("wrong argument type   \n"));
		return FALSE;	
	}
	flag = simple_strtol(arg, 0, 10);
	if((flag == 0) || (flag == 1))
	{
		pAd->ApDisableSTAConnectFlag = flag;
		return TRUE;
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR,("wrong argument value   \n"));
		return FALSE;
	}
}

INT Set_ApEnableBeaconTable_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	INT32 flag;
	if(strlen(arg) > 1)
	{
		DBGPRINT(RT_DEBUG_ERROR,("wrong argument type   \n"));
		return FALSE;	
	}
	flag = simple_strtol(arg, 0, 10);
	if((flag == 0) || (flag == 1))
	{
		BssTableInit(&pAd->AvailableBSS);
		pAd->ApEnableBeaconTable = flag;
		return TRUE;
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR,("wrong argument value   \n"));
		return FALSE;
	}
}

INT Set_ApScan_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	UINT channel =0;
	UINT timeout =0;
	INT32 i,j,count;
	CHAR scantype[8];
	CHAR temp[33];	
	UINT32			mac_val;

	//check if the interface is down
	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE))
	{
		DBGPRINT(RT_DEBUG_TRACE, ("INFO::Network is down!\n"));
		return -ENETDOWN;   
	}

	i = 0;
	j = 0;
	count = 0;
	while(arg[j] != '\0')
	{
		temp[i] = arg[j];
		j++;
		if(temp[i] == ':' || arg[j] == '\0' )
		{
		    if(temp[i] == ':')
			{	
                count++;
				switch(count)
				{
					case 1:
					    	temp[i]= '\0';
					    	if ((strlen(temp) != 0) && (strlen(temp) <= 7))
    				    	{
        				    	strcpy(scantype,temp);
						    	if(strcmp(scantype,"active") && strcmp(scantype,"passive"))
						    	{
							    	DBGPRINT(RT_DEBUG_ERROR,("wrong scan type argument \n"));
							    	return FALSE;
						    	}
					    	}
					    	else if(strlen(temp) > 7)
					    	{
						    	DBGPRINT(RT_DEBUG_ERROR,("wrong scan type argument \n"));
						    	return FALSE;
					    	}
					    	i = -1;
					    	break;
					case 2:
					    	temp[i] = '\0';
					    	if((strlen(temp) != 0) && (strlen(temp) <= 3))
					    	{
						    	channel=simple_strtol(temp, 0, 10);
						    	if(!ChannelSanity(pAd, channel)) 
						    	{
							    	DBGPRINT(RT_DEBUG_ERROR,("wrong channel number \n"));
							    	return FALSE;	
							    }
						    }
					    	else if(strlen(temp) > 3)
					    	{
						    	DBGPRINT(RT_DEBUG_ERROR,("wrong channel number \n"));
						    	return FALSE;
						    }
					    	i = -1;
					    	break;
					default:
					    	if(count > 2)
					    	{
						    	DBGPRINT(RT_DEBUG_ERROR,("wrong number of arguments \n"));
						    	return FALSE;
						    }
						    break;
				}
			}
			else if(arg[j] == '\0')
			{
				temp[i+1] = '\0';
			    if((strlen(temp) != 0) && (strlen(temp) <= 10) && ( simple_strtol(temp, 0, 10) < 0xffffffff))
				{
					timeout = simple_strtol(temp, 0, 10);
				}
				else if(strlen(temp))
				{
					DBGPRINT(RT_DEBUG_ERROR,("wrong Timeout value \n"));
					return FALSE;
				} 
			}
		}
		i++;

	}
	
	RTMP_IO_READ32(pAd, CH_TIME_CFG, &mac_val);
	mac_val |= 0x11f;
	RTMP_IO_WRITE32(pAd, CH_TIME_CFG, mac_val);
        //printk("%s %u %u \n", scantype, channel, timeout);
	if (!strcmp(scantype, "passive"))
			ApSiteSurveyNew(pAd,channel,timeout, SCAN_PASSIVE, FALSE);
	else if(!strcmp(scantype, "active"))
			ApSiteSurveyNew(pAd,channel, timeout, SCAN_ACTIVE, FALSE);
	
	DBGPRINT(RT_DEBUG_TRACE, ("Set_ApScan_Proc\n"));

    return TRUE;
}

#ifdef MBSS_802_11_STATISTICS
VOID RTMPIoctlQueryMbssStat(
	IN	PRTMP_ADAPTER	pAd, 
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq)
{
	int apidx;
	UINT64 temp;
	RT_MBSS_STATISTICS_TABLE *mbss_stat;

	os_alloc_mem(NULL, (UCHAR **)&mbss_stat, sizeof(RT_MBSS_STATISTICS_TABLE));
	if (mbss_stat == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s: Allocate memory fail!!!\n", __FUNCTION__));
		return;
	}
	NdisZeroMemory(mbss_stat, sizeof(RT_MBSS_STATISTICS_TABLE));
	mbss_stat->Num = pAd->ApCfg.BssidNum;
	
	for (apidx=0; apidx < pAd->ApCfg.BssidNum; apidx++)
	{		
		mbss_stat->MbssEntry[apidx].RxCount = pAd->ApCfg.MBSSID[apidx].RxCount;
		mbss_stat->MbssEntry[apidx].TxCount = pAd->ApCfg.MBSSID[apidx].TxCount;
		mbss_stat->MbssEntry[apidx].ReceivedByteCount = pAd->ApCfg.MBSSID[apidx].ReceivedByteCount;
		mbss_stat->MbssEntry[apidx].TransmittedByteCount = pAd->ApCfg.MBSSID[apidx].TransmittedByteCount;
		mbss_stat->MbssEntry[apidx].RxErrorCount = pAd->ApCfg.MBSSID[apidx].RxErrorCount;
		mbss_stat->MbssEntry[apidx].RxDropCount = pAd->ApCfg.MBSSID[apidx].RxDropCount;
		mbss_stat->MbssEntry[apidx].TxRetriedPktCount = pAd->ApCfg.MBSSID[apidx].TxRetriedPktCount;
#ifdef STATS_COUNT_SUPPORT
		mbss_stat->MbssEntry[apidx].TxErrorCount = pAd->ApCfg.MBSSID[apidx].TxErrorCount;
		mbss_stat->MbssEntry[apidx].TxDropCount = pAd->ApCfg.MBSSID[apidx].TxDropCount;
		mbss_stat->MbssEntry[apidx].UnicastPktsRx = pAd->ApCfg.MBSSID[apidx].ucPktsRx;
		mbss_stat->MbssEntry[apidx].UnicastPktsTx = pAd->ApCfg.MBSSID[apidx].ucPktsTx;
		mbss_stat->MbssEntry[apidx].MulticastPktsRx = pAd->ApCfg.MBSSID[apidx].mcPktsRx;
		mbss_stat->MbssEntry[apidx].MulticastPktsTx = pAd->ApCfg.MBSSID[apidx].mcPktsTx;
		mbss_stat->MbssEntry[apidx].BroadcastPktsRx = pAd->ApCfg.MBSSID[apidx].bcPktsRx;
		mbss_stat->MbssEntry[apidx].BroadcastPktsTx = pAd->ApCfg.MBSSID[apidx].bcPktsTx;
#endif // STATS_COUNT_SUPPORT //
		mbss_stat->MbssEntry[apidx].MGMTRxCount = pAd->ApCfg.MBSSID[apidx].MGMTRxCount;
		mbss_stat->MbssEntry[apidx].MGMTTxCount = pAd->ApCfg.MBSSID[apidx].MGMTTxCount;
		mbss_stat->MbssEntry[apidx].MGMTReceivedByteCount = pAd->ApCfg.MBSSID[apidx].MGMTReceivedByteCount;
		mbss_stat->MbssEntry[apidx].MGMTTransmittedByteCount = pAd->ApCfg.MBSSID[apidx].MGMTTransmittedByteCount;
		mbss_stat->MbssEntry[apidx].MGMTRxErrorCount = pAd->ApCfg.MBSSID[apidx].MGMTRxErrorCount;
		mbss_stat->MbssEntry[apidx].MGMTRxDropCount = pAd->ApCfg.MBSSID[apidx].MGMTRxDropCount;
		mbss_stat->MbssEntry[apidx].MGMTTxErrorCount = pAd->ApCfg.MBSSID[apidx].MGMTTxErrorCount;
		mbss_stat->MbssEntry[apidx].MGMTTxDropCount = pAd->ApCfg.MBSSID[apidx].MGMTTxDropCount;
		
		temp = pAd->ApCfg.MBSSID[apidx].ChannelUseTime;
		do_div(temp, 1000);
		mbss_stat->MbssEntry[apidx].ChannelUseTime = temp;
		
		pAd->ApCfg.MBSSID[apidx].RxCount = 0; 
		pAd->ApCfg.MBSSID[apidx].TxCount = 0;
		pAd->ApCfg.MBSSID[apidx].ReceivedByteCount = 0;
		pAd->ApCfg.MBSSID[apidx].TransmittedByteCount = 0;
		pAd->ApCfg.MBSSID[apidx].RxErrorCount = 0;
		pAd->ApCfg.MBSSID[apidx].RxDropCount = 0;
		pAd->ApCfg.MBSSID[apidx].TxRetriedPktCount = 0;
		pAd->ApCfg.MBSSID[apidx].TxErrorCount = 0;
		pAd->ApCfg.MBSSID[apidx].TxDropCount = 0;
		pAd->ApCfg.MBSSID[apidx].ucPktsRx = 0;
		pAd->ApCfg.MBSSID[apidx].ucPktsTx = 0;
		pAd->ApCfg.MBSSID[apidx].mcPktsRx = 0;
		pAd->ApCfg.MBSSID[apidx].mcPktsTx = 0;
		pAd->ApCfg.MBSSID[apidx].bcPktsRx = 0;
		pAd->ApCfg.MBSSID[apidx].bcPktsTx = 0;
		pAd->ApCfg.MBSSID[apidx].MGMTRxCount = 0;
		pAd->ApCfg.MBSSID[apidx].MGMTTxCount = 0;
		pAd->ApCfg.MBSSID[apidx].MGMTReceivedByteCount = 0;
		pAd->ApCfg.MBSSID[apidx].MGMTTransmittedByteCount = 0;
		pAd->ApCfg.MBSSID[apidx].MGMTRxErrorCount = 0;
		pAd->ApCfg.MBSSID[apidx].MGMTRxDropCount = 0;
		pAd->ApCfg.MBSSID[apidx].MGMTTxErrorCount = 0;
		pAd->ApCfg.MBSSID[apidx].MGMTTxDropCount = 0;
		pAd->ApCfg.MBSSID[apidx].ChannelUseTime = 0;
		
	}

	wrq->u.data.length = sizeof(RT_MBSS_STATISTICS_TABLE);
	if (copy_to_user(wrq->u.data.pointer, mbss_stat, wrq->u.data.length))
	{
		DBGPRINT(RT_DEBUG_TRACE, ("%s: copy_to_user() fail\n", __FUNCTION__));
	}
	if (mbss_stat != NULL)
		os_free_mem(NULL, mbss_stat);
}

VOID RTMPIoctlQuerySTAStat(
	IN	PRTMP_ADAPTER	pAd, 
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq)
{
	int i;
	UINT64 temp;
	PMAC_TABLE_ENTRY pEntry = NULL;
	RT_STA_STATISTICS_TABLE *sta_stat = NULL;

	os_alloc_mem(NULL, (UCHAR **)&sta_stat, sizeof(RT_STA_STATISTICS_TABLE));
	if (sta_stat == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s: Allocate memory fail!!!\n", __FUNCTION__));
		return;
	}

	NdisZeroMemory(sta_stat, sizeof(RT_STA_STATISTICS_TABLE));
	for (i = 0; i < MAX_LEN_OF_MAC_TABLE; i++)
	{
		pEntry = &pAd->MacTab.Content[i];
		if (pEntry && IS_ENTRY_CLIENT(pEntry) && pEntry->Sst == SST_ASSOC)
		{
			sta_stat->STAEntry[sta_stat->Num].ApIdx = pEntry->apidx;			
			COPY_MAC_ADDR(sta_stat->STAEntry[sta_stat->Num].Addr, pEntry->Addr);
			sta_stat->STAEntry[sta_stat->Num].RxCount = pEntry->RxCount;
			sta_stat->STAEntry[sta_stat->Num].TxCount = pEntry->TxCount;
			sta_stat->STAEntry[sta_stat->Num].ReceivedByteCount = pEntry->ReceivedByteCount;
			sta_stat->STAEntry[sta_stat->Num].TransmittedByteCount = pEntry->TransmittedByteCount;
			sta_stat->STAEntry[sta_stat->Num].RxErrorCount = pEntry->RxErrorCount;
			sta_stat->STAEntry[sta_stat->Num].RxDropCount = pEntry->RxDropCount;
			sta_stat->STAEntry[sta_stat->Num].TxErrorCount = pEntry->TxErrorCount;
			sta_stat->STAEntry[sta_stat->Num].TxDropCount = pEntry->TxDropCount;
			sta_stat->STAEntry[sta_stat->Num].TxRetriedPktCount = pEntry->TxRetriedPktCount;
			temp = pEntry->ChannelUseTime;
			do_div(temp, 1000);
			sta_stat->STAEntry[sta_stat->Num].ChannelUseTime = temp;	
			sta_stat->Num++;
	
			// clear STA Stats
			pEntry->RxCount = 0;
			pEntry->TxCount = 0;
			pEntry->ReceivedByteCount = 0;
			pEntry->TransmittedByteCount = 0;
			pEntry->RxErrorCount = 0;
			pEntry->RxDropCount = 0;
			pEntry->TxErrorCount = 0;
			pEntry->TxDropCount = 0;
			pEntry->TxRetriedPktCount = 0;
			pEntry->ChannelUseTime = 0;
		}
	} 

	wrq->u.data.length = sizeof(RT_STA_STATISTICS_TABLE);
	if (copy_to_user(wrq->u.data.pointer, sta_stat, wrq->u.data.length))
	{
		DBGPRINT(RT_DEBUG_TRACE, ("%s: copy_to_user() fail\n", __FUNCTION__));
	}
	if (sta_stat != NULL)
		os_free_mem(NULL, sta_stat);
}
#endif
#endif

char *obtw_delta_str[]=
{    
    "ofdm6m_", "ofdm9m_", "ofdm12m_", "ofdm18m_",
    "ht20mcs0_", "ht20mcs1_", "ht20mcs2_",     
    "ht40mcs0_", "ht40mcs1_", "ht40mcs2_", "ht40mcs32_",
    "vht80mcs0_", "vht80mcs1_", "vht80mcs2_"
};

#ifdef MT76x2
INT set_obtw_delta_proc(
    IN PRTMP_ADAPTER    pAd,
    IN PSTRING      arg)
{	
    CHAR *Pos = NULL;
    UINT8 DeltaVal = 0;
    int i;
    pAd->obtw_anyEnable = FALSE;
   	memset(pAd->obtw_delta_array, 0, sizeof(pAd->obtw_delta_array));
   	
    if (arg && strlen(arg)) {
        Pos = strstr(arg, "disable");
        if (Pos != NULL) {           
            DBGPRINT(RT_DEBUG_OFF, ("%s: disable\n", __FUNCTION__));
        }
    }

    for (i = 0; i < OBTW_DELTA_ELEMENT; i++)
    {
        if (arg && strlen(arg))
            Pos = strstr(arg, obtw_delta_str[i]);
        else           
            DBGPRINT(RT_DEBUG_OFF, ("%s: input arguments error , missing %s\n", __FUNCTION__ , obtw_delta_str[i]));

        if (Pos != NULL) {
            Pos = Pos + strlen(obtw_delta_str[i]);
            DeltaVal = simple_strtol(Pos, 0, 10);
            pAd->obtw_delta_array[i] = DeltaVal;            
            DBGPRINT(RT_DEBUG_OFF, ("%s: found %s, DeltaVal = %d\n", __FUNCTION__, obtw_delta_str[i], DeltaVal));
            if (DeltaVal > 0)
                pAd->obtw_anyEnable = TRUE;
        }
    }

	DBGPRINT(RT_DEBUG_OFF, ("%s: anyEnable=%d\n", __FUNCTION__, pAd->obtw_anyEnable));

	for(i=0;i<OBTW_DELTA_ELEMENT;i++)
		DBGPRINT(RT_DEBUG_OFF, ("%s : %s delta[%d] %d \n",__FUNCTION__,obtw_delta_str[i],i,pAd->obtw_delta_array[i]));
	
	
    return TRUE;
}

INT set_obtw_debug_proc(
    IN PRTMP_ADAPTER    pAd,
    IN PSTRING      arg)
{
	UINT8 debug_on;

	debug_on = simple_strtol(arg, 0, 10);
	if(debug_on!=0)
		pAd->obtw_debug_on = TRUE;
	else
		pAd->obtw_debug_on = FALSE;

	return TRUE;
}


#endif /* MT76x2 */

#ifdef ED_MONITOR
INT edcca_tx_stop_start(RTMP_ADAPTER *pAd, BOOLEAN stop)
{	
	UINT32 macCfg, macCfg_2, macStatus,MacValue;	
	UINT32 MTxCycle;
	ULONG stTime, mt_time, mr_time;
#ifdef CONFIG_STA_SUPPORT	
	static BOOLEAN orig_auto_reconnect_setting;
#endif /* CONFIG_STA_SUPPORT */

	/* Disable MAC Tx and wait MAC Tx/Rx status in idle state or direcyl enable tx */
	NdisGetSystemUpTime(&stTime);
	RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &macCfg);

	RTMP_IO_READ32(pAd, AUTO_RSP_CFG, &macCfg_2);

	if (stop == TRUE)
	{
		macCfg &= (~0x04);
		macCfg_2 &= (~0x01);
		
		/* Disable PA. */
		RTMP_IO_READ32(pAd, TX_PIN_CFG, &MacValue);
		RTMP_IO_WRITE32(pAd, TX_PIN_CFG, (MacValue & 0xfffff0f0));
		
#ifdef CONFIG_STA_SUPPORT
		orig_auto_reconnect_setting = pAd->StaCfg.bAutoReconnect;
		pAd->StaCfg.bAutoReconnect = FALSE;
#endif /* CONFIG_STA_SUPPORT */
	}else
	{
		macCfg |= 0x04;
		macCfg_2 |= 0x01;

		
		/* Enable PA */
		RTMP_IO_READ32(pAd, TX_PIN_CFG, &MacValue);
		RTMP_IO_WRITE32(pAd, TX_PIN_CFG, (MacValue | 0x00050f0f));

#ifdef CONFIG_STA_SUPPORT
		pAd->StaCfg.bAutoReconnect = orig_auto_reconnect_setting;
#endif /* CONFIG_STA_SUPPORT */

	}
	RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, macCfg);

	RTMP_IO_WRITE32(pAd, AUTO_RSP_CFG, macCfg_2);

			
	if (stop == TRUE)
	{
		for (MTxCycle = 0; MTxCycle < 10000; MTxCycle++)
		{
			RTMP_IO_READ32(pAd, MAC_STATUS_CFG, &macStatus);
			if (macStatus & 0x1)
				RtmpusecDelay(50);
			else
				break;
		}
		NdisGetSystemUpTime(&mt_time);
		mt_time -= stTime;
		if (MTxCycle == 10000)
		{
			DBGPRINT(RT_DEBUG_OFF, ("%s(cnt=%d,time=0x%lx):stop MTx,macStatus=0x%x!\n", 
				__FUNCTION__, MTxCycle, mt_time, macStatus));
		}
	}

	DBGPRINT(RT_DEBUG_OFF, ("%s():%s tx\n", 
				__FUNCTION__, ((stop == TRUE) ? "stop" : "start")));

	return TRUE;
}

#ifdef ED_SMART
INT ed_state_judge(RTMP_ADAPTER *pAd)
{	
	if((pAd->ed_trigger_cnt > pAd->ed_learning_time_threshold)
	&& (pAd->ed_false_cca_cnt > pAd->ed_learning_time_threshold)
	&& pAd->ed_vga_at_lowest_gain 
	)
	{
		if( pAd->ed_current_state == ED_OFF_AND_LEARNING )
		{
			UINT32 mac_val;
			RTMP_IO_READ32(pAd, TXOP_CTRL_CFG, &mac_val);
			mac_val |= ((1<<20) | (1<<7));  			
			RTMP_IO_WRITE32(pAd, TXOP_CTRL_CFG, mac_val);	

			pAd->ed_current_state = ED_TESTING;
			
			DBGPRINT(RT_DEBUG_OFF,("\n\n@@@@@@ %s : ed_testing !!\n\n",__FUNCTION__));
			RTMPSetTimer(&pAd->ed_timer, 1200000);
		}
		else if( pAd->ed_current_state == ED_TESTING )
		{
			//reset timer to 20 mins
			BOOLEAN cancelled;
			RTMPCancelTimer(&pAd->ed_timer, &cancelled);
			RTMPSetTimer(&pAd->ed_timer, 1200000);
			DBGPRINT(RT_DEBUG_TRACE,("\n\n@@@@@@ %s : reset ed_timer to 20 mins !!\n\n",__FUNCTION__));			
		}
	}

	return TRUE;
}

VOID ed_testing_timeout(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3) 
{
	PRTMP_ADAPTER	pAd = (PRTMP_ADAPTER)FunctionContext;
	DBGPRINT(RT_DEBUG_OFF, ("%s : reset pAd->ed_current_state: %d\n", __FUNCTION__, pAd->ed_current_state));
	pAd->ed_current_state = ED_OFF_AND_LEARNING;
}
#endif /* ED_SMART */

INT ed_status_read(RTMP_ADAPTER *pAd)
{
	UINT32 period_us = pAd->ed_chk_period * 1000;
	ULONG irqflag;
	BOOLEAN stop_edcca = FALSE;
	BOOLEAN stop_tx = FALSE;
		
	INT percent;
	RX_STA_CNT1_STRUC RxStaCnt1;
	UINT32 ch_idle_stat=0, ch_busy_stat=0, ed_2nd_stat=0, ed_stat=0;
	
	RTMP_IO_READ32(pAd, CH_IDLE_STA, &ch_idle_stat);
	RTMP_IO_READ32(pAd, 0x1140, &ed_stat);
	RTMP_IO_READ32(pAd, RX_STA_CNT1, &RxStaCnt1.word);

	RTMP_IRQ_LOCK(&pAd->irq_lock, irqflag);
	
	pAd->ch_idle_stat[pAd->ed_stat_lidx] = ch_idle_stat;
	pAd->ch_busy_stat[pAd->ed_stat_lidx] = ch_busy_stat;
	pAd->ed_2nd_stat[pAd->ed_stat_lidx] = ed_2nd_stat;
	pAd->ed_stat[pAd->ed_stat_lidx] = ed_stat;

	pAd->false_cca_stat[pAd->ed_stat_lidx] += RxStaCnt1.field.FalseCca;
	pAd->RalinkCounters.OneSecFalseCCACnt += RxStaCnt1.field.FalseCca;
			
	
	NdisGetSystemUpTime(&pAd->chk_time[pAd->ed_stat_lidx]);
		
	if ((pAd->ed_threshold > 0) && (period_us > 0) && (pAd->ed_block_tx_threshold > 0)) {
		percent = (pAd->ed_stat[pAd->ed_stat_lidx] * 100 ) / period_us;
		if (percent > 100)
			percent = 100;

		if (pAd->false_cca_stat[pAd->ed_stat_lidx] > pAd->ed_false_cca_threshold) 
		{
			pAd->ed_false_cca_cnt ++;						
		}
		else
		{
			pAd->ed_false_cca_cnt = 0;
		}

		
		if (percent > pAd->ed_threshold) 
		{
			pAd->ed_trigger_cnt++;
			pAd->ed_silent_cnt = 0;
		}
		else
		{
			pAd->ed_trigger_cnt = 0;
			pAd->ed_silent_cnt++;
		}
			
		if(pAd->ed_debug)
		{
			DBGPRINT(RT_DEBUG_ERROR, ("@@@ %s: false_cca_stat[%u]=%03u, ed_false_cca_cnt=%d vga_at_lowest_gain=%d one sec cca=%d  !!\n", 
					__FUNCTION__, pAd->ed_stat_lidx, pAd->false_cca_stat[pAd->ed_stat_lidx]
				, pAd->ed_false_cca_cnt,pAd->ed_vga_at_lowest_gain,pAd->RalinkCounters.OneSecFalseCCACnt));
			DBGPRINT(RT_DEBUG_ERROR, ("@@@ %s: pAd->ed_trigger_cnt=%d,	pAd->ed_silent_cnt=%u, percent=%d !!\n", 
				__FUNCTION__, pAd->ed_trigger_cnt, pAd->ed_silent_cnt
			,  percent));
		}
	}
	pAd->ed_trigger_stat[pAd->ed_stat_lidx] = pAd->ed_trigger_cnt;
	pAd->ed_silent_stat[pAd->ed_stat_lidx] = pAd->ed_silent_cnt;

	
	INC_RING_INDEX(pAd->ed_stat_lidx, ED_STAT_CNT);
	pAd->false_cca_stat[pAd->ed_stat_lidx] = 0;
	if (pAd->ed_stat_sidx == pAd->ed_stat_lidx) {
		INC_RING_INDEX(pAd->ed_stat_sidx, ED_STAT_CNT);
	}	
	RTMP_IRQ_UNLOCK(&pAd->irq_lock, irqflag);

		if(pAd->ed_chk == EDCCA_ON 
	#ifdef ED_SMART
		|| (pAd->ed_chk == EDCCA_SMART && pAd->ed_current_state == ED_TESTING)
	#endif
		)
	
		{
			if (pAd->ed_trigger_cnt > pAd->ed_block_tx_threshold) {
				if (pAd->ed_tx_stoped == FALSE) {
					pAd->ed_tx_stoped = TRUE;
				edcca_tx_stop_start(pAd, TRUE); 			
			}
		}

		if (pAd->ed_silent_cnt > pAd->ed_block_tx_threshold) {
			if (pAd->ed_tx_stoped == TRUE) {
				pAd->ed_tx_stoped = FALSE;
				edcca_tx_stop_start(pAd, FALSE);				
			}
		}
	}
	return TRUE;
}
	

//this function will be called in multi entry
INT ed_monitor_exit(RTMP_ADAPTER *pAd)
{
	ULONG irqflag;
	BOOLEAN old_ed_tx_stoped, old_ed_chk;
	BOOLEAN cancelled;
		
	RTMP_IRQ_LOCK(&pAd->irq_lock, irqflag);
	DBGPRINT(RT_DEBUG_OFF, ("@@@ %s : ===>\n", __FUNCTION__));
	
#ifdef ED_SMART	
	RTMPCancelTimer(&pAd->ed_timer, &cancelled);
#endif /*ED_SMART*/
	
	NdisZeroMemory(&pAd->ed_stat[0], sizeof(pAd->ed_stat));
	NdisZeroMemory(&pAd->ch_idle_stat[0], sizeof(pAd->ch_idle_stat));
	NdisZeroMemory(&pAd->ch_busy_stat[0], sizeof(pAd->ch_busy_stat));
	NdisZeroMemory(&pAd->chk_time[0], sizeof(pAd->chk_time));
	NdisZeroMemory(&pAd->ed_trigger_stat[0], sizeof(pAd->ed_trigger_stat));
	NdisZeroMemory(&pAd->ed_silent_stat[0], sizeof(pAd->ed_silent_stat));
	NdisZeroMemory(&pAd->false_cca_stat[0], sizeof(pAd->false_cca_stat));
	
	pAd->ed_stat_lidx = pAd->ed_stat_sidx = 0;
	pAd->ed_trigger_cnt = 0;
	pAd->ed_silent_cnt = 0;
	//ignore fisrt time's incorrect false cca 
	pAd->ed_false_cca_cnt = 0;

	old_ed_tx_stoped = pAd->ed_tx_stoped;
	old_ed_chk = pAd->ed_chk;

	pAd->ed_tx_stoped = FALSE;
	//also clear top level flags
	pAd->ed_chk = FALSE;
	DBGPRINT(RT_DEBUG_OFF, ("@@@ %s : <===\n", __FUNCTION__));
	RTMP_IRQ_UNLOCK(&pAd->irq_lock, irqflag);


	if (old_ed_tx_stoped)
	{
		edcca_tx_stop_start(pAd, FALSE);
	}

	if (old_ed_chk)
	{
		RTMP_CHIP_ASIC_SET_EDCCA(pAd,FALSE);
	}
	
	return TRUE;
}


// open & muanl cmd will call
INT ed_monitor_init(RTMP_ADAPTER *pAd)
{
	ULONG irqflag;
	TX_LINK_CFG_STRUC		TxLinkCfg;

	RTMP_IRQ_LOCK(&pAd->irq_lock, irqflag);
	DBGPRINT(RT_DEBUG_OFF, ("@@@ %s : ===>\n", __FUNCTION__));
	NdisZeroMemory(&pAd->ed_stat[0], sizeof(pAd->ed_stat));
	NdisZeroMemory(&pAd->ch_idle_stat[0], sizeof(pAd->ch_idle_stat));
	NdisZeroMemory(&pAd->ch_busy_stat[0], sizeof(pAd->ch_busy_stat));
	NdisZeroMemory(&pAd->chk_time[0], sizeof(pAd->chk_time));
	NdisZeroMemory(&pAd->ed_trigger_stat[0], sizeof(pAd->ed_trigger_stat));
	NdisZeroMemory(&pAd->ed_silent_stat[0], sizeof(pAd->ed_silent_stat));
	NdisZeroMemory(&pAd->false_cca_stat[0], sizeof(pAd->false_cca_stat));
	
	pAd->ed_stat_lidx = pAd->ed_stat_sidx = 0;
	pAd->ed_trigger_cnt = 0;
	pAd->ed_silent_cnt = 0;

	// ignore fisrt time's incorrect false cca
	pAd->ed_false_cca_cnt = 0;
	
	pAd->ed_tx_stoped = FALSE;
	//also set  top level flags
	if(pAd->ed_chk == 0)	//turn on if disabled , otherwise decide by profile
		pAd->ed_chk = 1; 

	RTMP_IO_READ32(pAd, TX_LINK_CFG, &TxLinkCfg.word);
	TxLinkCfg.field.TxRDGEn = 0;
	TxLinkCfg.field.TxCFAckEn = 0;
	RTMP_IO_WRITE32(pAd, TX_LINK_CFG, TxLinkCfg.word);

#ifdef ED_SMART
	RTMPInitTimer(pAd, &(pAd->ed_timer), GET_TIMER_FUNCTION(ed_testing_timeout), pAd, FALSE);
#endif /* ED_SMART */


	
	DBGPRINT(RT_DEBUG_OFF, ("@@@ %s : <===\n", __FUNCTION__));
	RTMP_IRQ_UNLOCK(&pAd->irq_lock, irqflag);

	RTMP_CHIP_ASIC_SET_EDCCA(pAd,TRUE);

	return TRUE;
}

INT set_ed_block_tx_thresh(RTMP_ADAPTER *pAd, PSTRING arg)
{
	UINT block_tx_threshold = simple_strtol(arg, 0, 10);

	pAd->ed_block_tx_threshold = block_tx_threshold;
	DBGPRINT(RT_DEBUG_OFF, ("%s(): ed_block_tx_threshold=%d\n",
				__FUNCTION__, pAd->ed_block_tx_threshold));

	return TRUE;	
}


INT set_ed_threshold(RTMP_ADAPTER *pAd, PSTRING arg)
{
	ULONG percent = simple_strtol(arg, 0, 10);

	if (percent > 100)
		pAd->ed_threshold = (percent % 100);
	else
		pAd->ed_threshold = percent;

	DBGPRINT(RT_DEBUG_OFF, ("%s(): ed_threshold=%d\n",
				__FUNCTION__, pAd->ed_threshold));

	return TRUE;
}
INT set_ed_learn_threshold(RTMP_ADAPTER *pAd, PSTRING arg)
{
	ULONG learn_th = simple_strtol(arg, 0, 10);

	pAd->ed_learning_time_threshold = learn_th;

	DBGPRINT(RT_DEBUG_OFF, ("%s(): learn_th=%d  * 100ms\n",
				__FUNCTION__, pAd->ed_learning_time_threshold));

	return TRUE;
}


INT set_ed_false_cca_threshold(RTMP_ADAPTER *pAd, PSTRING arg)
{
	ULONG false_cca_threshold = simple_strtol(arg, 0, 10);

	pAd->ed_false_cca_threshold = false_cca_threshold > 0 ? false_cca_threshold : 0;

	DBGPRINT(RT_DEBUG_OFF, ("%s(): ed_false_cca_threshold=%d\n",
				__FUNCTION__, pAd->ed_false_cca_threshold));

	return TRUE;
}


//let run-time turn on/off
INT set_ed_chk_proc(RTMP_ADAPTER *pAd, PSTRING arg)
{
	UINT ed_chk;
	
	ed_chk = simple_strtol(arg, 0, 10);

	DBGPRINT(RT_DEBUG_OFF, ("%s(): ed_chk=%d\n",
				__FUNCTION__, ed_chk));

	if (ed_chk == EDCCA_OFF) {
		pAd->ed_chk = ed_chk;
		ed_monitor_exit(pAd);
	} 
#ifdef ED_SMART
	else if(ed_chk == EDCCA_ON || ed_chk == EDCCA_SMART){
		pAd->ed_chk = ed_chk;
		ed_monitor_init(pAd);
	}
#endif
	else
	{
		pAd->ed_chk = EDCCA_ON;
		ed_monitor_init(pAd);
	}

	
	return TRUE;
}

#ifdef CONFIG_AP_SUPPORT
INT set_ed_sta_count_proc(RTMP_ADAPTER *pAd, PSTRING arg)
{
	UINT ed_sta_th;
	
	ed_sta_th = simple_strtol(arg, 0, 10);

	DBGPRINT(RT_DEBUG_OFF, ("%s(): ed_sta_th=%d\n",
				__FUNCTION__, ed_sta_th));

	pAd->ed_sta_threshold = ed_sta_th;

	return TRUE;
}


INT set_ed_ap_count_proc(RTMP_ADAPTER *pAd, PSTRING arg)
{
	UINT ed_ap_th;
	
	ed_ap_th = simple_strtol(arg, 0, 10);

	DBGPRINT(RT_DEBUG_OFF, ("%s(): ed_ap_th=%d\n",
				__FUNCTION__, ed_ap_th));

	pAd->ed_ap_threshold = ed_ap_th;

	return TRUE;
}
#endif /* CONFIG_AP_SUPPORT */


#ifdef CONFIG_STA_SUPPORT
INT set_ed_ap_scaned_count_proc(RTMP_ADAPTER *pAd, PSTRING arg)
{
	UINT ed_ap_scaned;
	
	ed_ap_scaned = simple_strtol(arg, 0, 10);

	DBGPRINT(RT_DEBUG_OFF, ("%s(): ed_ap_scaned=%d\n",
				__FUNCTION__, ed_ap_scaned));

	pAd->ed_ap_scaned = ed_ap_scaned;

	return TRUE;
}


INT set_ed_current_ch_ap_proc(RTMP_ADAPTER *pAd, PSTRING arg)
{
	UINT ed_current_ch_aps;
	
	ed_current_ch_aps = simple_strtol(arg, 0, 10);

	DBGPRINT(RT_DEBUG_OFF, ("%s(): ed_current_ch_aps=%d\n",
				__FUNCTION__, ed_current_ch_aps));

	pAd->ed_current_ch_aps = ed_current_ch_aps;

	return TRUE;
}
#endif /* CONFIG_STA_SUPPORT */



INT set_ed_current_rssi_threhold_proc(RTMP_ADAPTER *pAd, PSTRING arg)
{
	INT ed_rssi_threshold;
	
	ed_rssi_threshold = simple_strtol(arg, 0, 10);

	DBGPRINT(RT_DEBUG_OFF, ("%s(): ed_rssi_threshold=%d\n",
				__FUNCTION__, ed_rssi_threshold));

	pAd->ed_rssi_threshold = ed_rssi_threshold;

	return TRUE;
}
INT set_ed_debug_proc(RTMP_ADAPTER *pAd, PSTRING arg)
{
	INT ed_debug;
	
	ed_debug = simple_strtol(arg, 0, 10);

	DBGPRINT(RT_DEBUG_OFF, ("%s(): ed_debug=%d\n",
				__FUNCTION__, ed_debug));

	pAd->ed_debug = ed_debug;

	return TRUE;
}

INT show_ed_stat_proc(RTMP_ADAPTER *pAd, PSTRING arg)
{
	unsigned long irqflags;
	UINT32 ed_stat[ED_STAT_CNT], ed_2nd_stat[ED_STAT_CNT], false_cca_stat[ED_STAT_CNT];
	UINT32 silent_stat[ED_STAT_CNT], trigger_stat[ED_STAT_CNT]; 
	UINT32 busy_stat[ED_STAT_CNT], idle_stat[ED_STAT_CNT];
	ULONG chk_time[ED_STAT_CNT];
	INT period_us;
	UCHAR start, end, idx;
		
	RTMP_IRQ_LOCK(&pAd->irq_lock, irqflags);
	start = pAd->ed_stat_sidx;
	end = pAd->ed_stat_lidx;
	NdisMoveMemory(&ed_stat[0], &pAd->ed_stat[0], sizeof(ed_stat));
	NdisMoveMemory(&ed_2nd_stat[0], &pAd->ed_2nd_stat[0], sizeof(ed_2nd_stat));
	NdisMoveMemory(&busy_stat[0], &pAd->ch_busy_stat[0], sizeof(busy_stat));
	NdisMoveMemory(&idle_stat[0], &pAd->ch_idle_stat[0], sizeof(idle_stat));
	NdisMoveMemory(&chk_time[0], &pAd->chk_time[0], sizeof(chk_time));
	NdisMoveMemory(&trigger_stat[0], &pAd->ed_trigger_stat[0], sizeof(trigger_stat));
	NdisMoveMemory(&silent_stat[0], &pAd->ed_silent_stat[0], sizeof(silent_stat));
	NdisMoveMemory(&false_cca_stat[0], &pAd->false_cca_stat[0], sizeof(false_cca_stat));
	RTMP_IRQ_UNLOCK(&pAd->irq_lock, irqflags);

#ifdef CONFIG_AP_SUPPORT
	DBGPRINT(RT_DEBUG_OFF, ("Dump ChannelBusy Counts, ed_chk=%u, ed_current_state = %d, ed_ap_threshold=%u, false_cca_threshold=%u, ChkPeriod=%dms, ED_Threshold=%d%%, HitCntForBlockTx=%d\n", 
				pAd->ed_chk,pAd->ed_current_state, pAd->ed_ap_threshold, pAd->ed_false_cca_threshold,
				pAd->ed_chk_period, pAd->ed_threshold, pAd->ed_block_tx_threshold));
#endif

#ifdef CONFIG_STA_SUPPORT
	DBGPRINT(RT_DEBUG_OFF, ("Dump ChannelBusy Counts, ed_chk=%u, ed_ap_scaned=%u, ed_current_ch_aps=%u, ed_rssi_threshold=%d, false_cca_threshold=%u, ChkPeriod=%dms, ED_Threshold=%d%%, HitCntForBlockTx=%d\n", 
				pAd->ed_chk, pAd->ed_ap_scaned, pAd->ed_current_ch_aps, pAd->ed_rssi_threshold, pAd->ed_false_cca_threshold,
				pAd->ed_chk_period, pAd->ed_threshold, pAd->ed_block_tx_threshold));
#endif


	period_us = pAd->ed_chk_period * 1000;
	DBGPRINT(RT_DEBUG_OFF, ("TimeSlot:"));
	idx = start;
	do {
		DBGPRINT(RT_DEBUG_OFF, ("%ld  ", chk_time[idx]));
		INC_RING_INDEX(idx, ED_STAT_CNT);
	} while (idx != end);
	DBGPRINT(RT_DEBUG_OFF, ("\n"));

	DBGPRINT(RT_DEBUG_OFF, ("Dump ED_STAT\n"));
	DBGPRINT(RT_DEBUG_OFF, ("RawCnt:"));
	idx = start;
	do {
		DBGPRINT(RT_DEBUG_OFF, ("%d  ", ed_stat[idx]));
		INC_RING_INDEX(idx, ED_STAT_CNT);
	} while (idx != end);
	DBGPRINT(RT_DEBUG_OFF, ("\n"));

	DBGPRINT(RT_DEBUG_OFF, ("Percent:"));
	idx = start;
	do {
		DBGPRINT(RT_DEBUG_OFF, ("\t%d", (ed_stat[idx] * 100) / period_us));
		INC_RING_INDEX(idx, ED_STAT_CNT);
	} while (idx != end);
	DBGPRINT(RT_DEBUG_OFF, ("\n"));

	DBGPRINT(RT_DEBUG_OFF, ("FalseCCA:"));
	idx = start;
	do {
		DBGPRINT(RT_DEBUG_OFF, ("\t%d", false_cca_stat[idx]));
		INC_RING_INDEX(idx, ED_STAT_CNT);
	} while (idx != end);
	DBGPRINT(RT_DEBUG_OFF, ("\n"));
	
	DBGPRINT(RT_DEBUG_OFF, ("TriggerCnt:"));
	idx = start;
	do {
		DBGPRINT(RT_DEBUG_OFF, ("\t%d", trigger_stat[idx]));
		INC_RING_INDEX(idx, ED_STAT_CNT);
	} while (idx != end);

	DBGPRINT(RT_DEBUG_OFF, ("SilentCnt:"));
	idx = start;
	do {
		DBGPRINT(RT_DEBUG_OFF, ("\t%d", silent_stat[idx]));
		INC_RING_INDEX(idx, ED_STAT_CNT);
	} while (idx != end);
	DBGPRINT(RT_DEBUG_OFF, ("\n==========================\n"));


	DBGPRINT(RT_DEBUG_OFF, ("Dump ED_2nd_STAT\n"));
	DBGPRINT(RT_DEBUG_OFF, ("RawCnt:"));
	idx = start;
	do {
		DBGPRINT(RT_DEBUG_OFF, ("%d  ", ed_2nd_stat[idx]));
		INC_RING_INDEX(idx, ED_STAT_CNT);
	} while (idx != end);
	DBGPRINT(RT_DEBUG_OFF, ("\n"));

	DBGPRINT(RT_DEBUG_OFF, ("Percent:"));
	idx = start;
	do {
		DBGPRINT(RT_DEBUG_OFF, ("\t%d", (ed_2nd_stat[idx] * 100) / period_us));
		INC_RING_INDEX(idx, ED_STAT_CNT);
	} while (idx != end);
	DBGPRINT(RT_DEBUG_OFF, ("\n"));
	DBGPRINT(RT_DEBUG_OFF, ("\n==========================\n"));


	DBGPRINT(RT_DEBUG_OFF, ("Dump CH_IDLE_STAT\n"));
	DBGPRINT(RT_DEBUG_OFF, ("RawCnt:"));
	idx = start;
	do {
		DBGPRINT(RT_DEBUG_OFF, ("%d  ", idle_stat[idx]));
		INC_RING_INDEX(idx, ED_STAT_CNT);
	} while (idx != end);
	DBGPRINT(RT_DEBUG_OFF, ("\n"));

	DBGPRINT(RT_DEBUG_OFF, ("Percent:"));
	idx = start;
	do {
		DBGPRINT(RT_DEBUG_OFF, ("\t%d", (idle_stat[idx] *100)/ period_us));
		INC_RING_INDEX(idx, ED_STAT_CNT);
	} while (idx != end);
	DBGPRINT(RT_DEBUG_OFF, ("\n"));
	DBGPRINT(RT_DEBUG_OFF, ("\n==========================\n"));	

	DBGPRINT(RT_DEBUG_OFF, ("Dump CH_BUSY_STAT\n"));
	DBGPRINT(RT_DEBUG_OFF, ("RawCnt:"));
	idx = start;
	do {
		DBGPRINT(RT_DEBUG_OFF, ("%d  ", busy_stat[idx]));
		INC_RING_INDEX(idx, ED_STAT_CNT);
	} while (idx != end);
	DBGPRINT(RT_DEBUG_OFF, ("\n"));

	DBGPRINT(RT_DEBUG_OFF, ("Percent:"));
	idx = start;
	do {
		DBGPRINT(RT_DEBUG_OFF, ("\t%d", (busy_stat[idx] *100 )/ period_us));
		INC_RING_INDEX(idx, ED_STAT_CNT);
	} while (idx != end);
	DBGPRINT(RT_DEBUG_OFF, ("\n"));
	DBGPRINT(RT_DEBUG_OFF, ("\n==========================\n"));

	return TRUE;
}

INT report_ed_count(RTMP_ADAPTER *pAd, PSTRING arg)
{
	INT period_us = 0;
	INT irqflags = 0;
	UINT32 ed_stat, percentage = 0;


	period_us = pAd->ed_chk_period * 1000;
	ed_stat = pAd->ed_stat[pAd->ed_stat_lidx - 1];  
	/* report previous 100ms ed count, avoid incomplete data */
	percentage = (ed_stat * 100) / period_us;
	percentage = (percentage >= 100)?100:percentage;

	DBGPRINT(RT_DEBUG_OFF, ("=====> %s \n", __FUNCTION__));
	DBGPRINT(RT_DEBUG_OFF, ("ed_raw_count_in_100ms : %d    ed_percentage_in_100ms : %d \n"
	,ed_stat, percentage));
	return TRUE;
}

#endif /* ED_MONITOR */

INT set_channel_ed_monitor_enable(RTMP_ADAPTER *pAd, PSTRING arg)
{
	UCHAR ED_TH = (pAd->CommonCfg.Channel > 14)?0x0e:0x20;
	/* A band 0x0e , G band 0x20 , 20150331 */
	UINT32 mac_val = 0;
	UINT32 bbp_val;

	DBGPRINT(RT_DEBUG_OFF, ("=====> %s \n", __FUNCTION__));
	
	RTMP_IO_READ32(pAd, CH_TIME_CFG, &mac_val);
	mac_val |= 0x40;
	RTMP_IO_WRITE32(pAd, CH_TIME_CFG, mac_val);

	RTMP_IO_READ32(pAd, CH_TIME_CFG, &mac_val);
	mac_val |= 0x05; // enable channel status check
	RTMP_IO_WRITE32(pAd, CH_TIME_CFG, mac_val);

#ifdef RLT_BBP
	if (IS_MT76x0(pAd) || IS_MT76x2(pAd)) {
		RTMP_BBP_IO_READ32(pAd, AGC1_R2, &bbp_val);
		bbp_val = (bbp_val & 0xFFFF0000) | (ED_TH << 8) | ED_TH;
		RTMP_BBP_IO_WRITE32(pAd, AGC1_R2, bbp_val);
	}
#endif

	RTMP_IO_READ32(pAd, TXOP_CTRL_CFG, &mac_val);
	mac_val &= ~(1 << 20);
	RTMP_IO_WRITE32(pAd, TXOP_CTRL_CFG, mac_val);	
	
	return TRUE;
}

INT show_ed_cnt_for_channel_quality(RTMP_ADAPTER *pAd, PSTRING arg)
{
	if(pAd->ed_chk == FALSE)
	{
		set_channel_ed_monitor_enable(pAd, arg);
	}
	else
	{
#ifdef ED_MONITOR
		report_ed_count(pAd, arg);
#else
		DBGPRINT(RT_DEBUG_OFF, ("%s : unexpected ed_chk = %d  without ED_MONITOR compile flag\n", __FUNCTION__,pAd->ed_chk));
		pAd->ed_chk = 0;
#endif /* ED_MONITOR */
	}
	
	return TRUE;
}


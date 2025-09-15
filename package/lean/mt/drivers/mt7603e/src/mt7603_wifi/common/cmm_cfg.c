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

static BOOLEAN RT_isLegalCmdBeforeInfUp(RTMP_STRING *SetCmd);


INT ComputeChecksum(UINT PIN)
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


static UCHAR *phy_bw_str[] = {"20M", "40M", "80M", "BOTH", "10M"}; 
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
INT RT_CfgSetCountryRegion(RTMP_ADAPTER *pAd, RTMP_STRING *arg, INT band)
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

#ifdef DBG
static RTMP_STRING *BAND_STR[] = {"Invalid", "2.4G", "5G", "2.4G/5G"};
#endif /* DBG */
static RTMP_STRING *WMODE_STR[]= {"", "A", "B", "G", "gN", "aN", "AC"};

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
	INT i, mode_cnt = sizeof(CFG_WMODE_MAP) / (sizeof(UCHAR) * 2);

	for (i = 0; i < mode_cnt; i++)
	{
		if (CFG_WMODE_MAP[i*2+1] == wmode)
			return CFG_WMODE_MAP[i*2];
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

#ifdef CONFIG_AP_SUPPORT
#ifdef MBSS_SUPPORT
static BOOLEAN wmode_valid(RTMP_ADAPTER *pAd, enum WIFI_MODE wmode)
{
	if ((WMODE_CAP_5G(wmode) && (!PHY_CAP_5G(pAd->chipCap.phy_caps))) ||
		(WMODE_CAP_2G(wmode) && (!PHY_CAP_2G(pAd->chipCap.phy_caps))) ||
		(WMODE_CAP_N(wmode) && RTMP_TEST_MORE_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DOT_11N))
	)
		return FALSE;
	else
		return TRUE;
}
#endif /* MBSS_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

static BOOLEAN wmode_valid_and_correct(RTMP_ADAPTER *pAd, UCHAR* wmode)
{
	BOOLEAN ret = TRUE;

	if (*wmode == WMODE_INVALID)
		*wmode = (WMODE_B | WMODE_G | WMODE_GN |WMODE_A | WMODE_AN | WMODE_AC);

	while(1)
	{
		if (WMODE_CAP_5G(*wmode) && (!PHY_CAP_5G(pAd->chipCap.phy_caps)))
		{
			*wmode = *wmode & ~(WMODE_A | WMODE_AN | WMODE_AC);
		}
		else if (WMODE_CAP_2G(*wmode) && (!PHY_CAP_2G(pAd->chipCap.phy_caps)))
		{
			*wmode = *wmode & ~(WMODE_B | WMODE_G | WMODE_GN);
		}
		else if (WMODE_CAP_N(*wmode) && ((!PHY_CAP_N(pAd->chipCap.phy_caps)) || RTMP_TEST_MORE_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DOT_11N)))
		{
			*wmode = *wmode & ~(WMODE_GN | WMODE_AN);
		}
		else if (WMODE_CAP_AC(*wmode) && (!PHY_CAP_AC(pAd->chipCap.phy_caps)))
		{
			*wmode = *wmode & ~(WMODE_AC);
		}

		if ( *wmode == 0 )
		{
			*wmode = (WMODE_B | WMODE_G | WMODE_GN |WMODE_A | WMODE_AN | WMODE_AC);
			break;
		}
		else
			break;
	}

	return ret;
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
INT RT_CfgSetWirelessMode(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	LONG cfg_mode;
	UCHAR wmode, *mode_str;


	cfg_mode = simple_strtol(arg, 0, 10);

	/* check if chip support 5G band when WirelessMode is 5G band */
	wmode = cfgmode_2_wmode((UCHAR)cfg_mode);
	if (!wmode_valid_and_correct(pAd, &wmode)) {
		DBGPRINT(RT_DEBUG_ERROR,
				("%s(): Invalid wireless mode(%ld, wmode=0x%x), ChipCap(%s)\n",
				__FUNCTION__, cfg_mode, wmode,
				BAND_STR[pAd->chipCap.phy_caps & 0x3]));
		return FALSE;
	}

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
INT RT_CfgSetMbssWirelessMode(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT cfg_mode;
	UCHAR wmode;


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


static BOOLEAN RT_isLegalCmdBeforeInfUp(RTMP_STRING *SetCmd)
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


INT RT_CfgSetShortSlot(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
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
	IN	RTMP_STRING *keyString,
	IN	CIPHER_KEY		*pSharedKey,
	IN	INT				keyIdx)
{
	INT				KeyLen;
	INT				i;
	/*UCHAR			CipherAlg = CIPHER_NONE;*/
#ifdef DBG
	BOOLEAN			bKeyIsHex = FALSE;
#endif /* DBG */

	/* TODO: Shall we do memset for the original key info??*/
	memset(pSharedKey, 0, sizeof(CIPHER_KEY));
	KeyLen = strlen(keyString);
	switch (KeyLen)
	{
		case 5: /*wep 40 Ascii type*/
		case 13: /*wep 104 Ascii type*/
#ifdef MT_MAC
		case 16: /*wep 128 Ascii type*/
#endif
#ifdef DBG
			bKeyIsHex = FALSE;
#endif /* DBG */
			pSharedKey->KeyLen = KeyLen;
			NdisMoveMemory(pSharedKey->Key, keyString, KeyLen);
			break;
			
		case 10: /*wep 40 Hex type*/
		case 26: /*wep 104 Hex type*/
#ifdef MT_MAC
		case 32: /*wep 128 Hex type*/
#endif			
			for(i=0; i < KeyLen; i++)
			{
				if( !isxdigit(*(keyString+i)) )
					return FALSE;  /*Not Hex value;*/
			}
#ifdef DBG
			bKeyIsHex = TRUE;
#endif /* DBG */
			pSharedKey->KeyLen = KeyLen/2 ;
			AtoH(keyString, pSharedKey->Key, pSharedKey->KeyLen);
			break;
			
		default: /*Invalid argument */
			DBGPRINT(RT_DEBUG_TRACE, ("RT_CfgSetWepKey(keyIdx=%d):Invalid argument (arg=%s)\n", keyIdx, keyString));
			return FALSE;
	}

	pSharedKey->CipherAlg = ((KeyLen % 5) ? CIPHER_WEP128 : CIPHER_WEP64);
#ifdef MT_MAC
	if (KeyLen == 32)
		pSharedKey->CipherAlg = CIPHER_WEP152;
#endif		
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
	IN RTMP_STRING *keyString,
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


INT	RT_CfgSetFixedTxPhyMode(RTMP_STRING *arg)
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
				break;
		}
	}

	return fix_tx_mode;
					
}	


INT	RT_CfgSetMacAddress(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
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


INT	RT_CfgSetTxMCSProc(RTMP_STRING *arg, BOOLEAN *pAutoRate)
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


INT	RT_CfgSetAutoFallBack(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
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
	IN RTMP_STRING *pPinCodeStr,
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
					pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev.func_idx = MAIN_MBSSID;
					pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev.sys_handle = (void *)pAd;
					RTMP_OS_NETDEV_SET_WDEV(pData, &pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev);
					wdev = &pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev;
				}
#endif /* CONFIG_AP_SUPPORT */
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
				if(pData == NULL ||	RT_isLegalCmdBeforeInfUp((RTMP_STRING *) pData) == FALSE)
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
			//CFG_TODO
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

#ifdef RTMP_SDIO_SUPPORT
      case CMD_RTPRIV_IOCTL_SDIO_INIT:
         InitSDIODevice(pAd);
         break;
#endif

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
			// TODO: Shiang-usw, this function looks have some problem, need to revise!
			if (VIRTUAL_IF_NUM(pAd) == 0)
			{
#ifdef DBG
				ULONG start, end, diff_ms;
				/* Get the current time for calculating startup time */
				NdisGetSystemUpTime(&start);
#endif /* DBG */				
				VIRTUAL_IF_INC(pAd);
				if (pInfConf->rt28xx_open(pAd->net_dev) != 0)
				{
					VIRTUAL_IF_DEC(pAd);
					DBGPRINT(RT_DEBUG_TRACE, ("rt28xx_open return fail!\n"));
					return NDIS_STATUS_FAILURE;
				}

#ifdef DBG				
				/* Get the current time for calculating startup time */
				NdisGetSystemUpTime(&end); diff_ms = (end-start)*1000/OS_HZ;
				DBGPRINT(RT_DEBUG_ERROR, ("WiFi Startup Cost (%s): %lu.%03lus\n",
						RTMP_OS_NETDEV_GET_DEVNAME(pAd->net_dev),diff_ms/1000,diff_ms%1000));
#endif /* DBG */
			}
			else
			{
				VIRTUAL_IF_INC(pAd);
#ifdef CONFIG_AP_SUPPORT
				{
					extern VOID APMakeAllBssBeacon(IN PRTMP_ADAPTER pAd);
					extern VOID  APUpdateAllBeaconFrame(IN PRTMP_ADAPTER pAd);
					APMakeAllBssBeacon(pAd);
					APUpdateAllBeaconFrame(pAd);
				}
#endif /* CONFIG_AP_SUPPORT */
			}
		}
			break;

		case CMD_RTPRIV_IOCTL_VIRTUAL_INF_DOWN:
		/* interface down */
		{
			RT_CMD_INF_UP_DOWN *pInfConf = (RT_CMD_INF_UP_DOWN *)pData;

			VIRTUAL_IF_DEC(pAd);
			if (VIRTUAL_IF_NUM(pAd) == 0)
				pInfConf->rt28xx_close(pAd->net_dev);
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
				RT_CMD_STATS *pStats = (RT_CMD_STATS *)pData;
				pStats->pStats = pAd->stats;
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
					pStats->rx_frame_errors = 0; /* recv'd frame alignment error*/
					pStats->rx_fifo_errors = pAd->Counters8023.RxNoBuffer;                   /* recv'r fifo overrun*/
				}
#ifdef CONFIG_AP_SUPPORT
				else if(pAd->OpMode == OPMODE_AP)
				{
					INT index;
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
						pStats->rx_packets = 0;
						pStats->tx_packets = 0;
						pStats->rx_bytes = 0;
						pStats->tx_bytes = 0;
						pStats->rx_errors = 0;
						pStats->tx_errors = 0;
						pStats->multicast = 0;   /* multicast packets received*/
						pStats->collisions = 0;  /* Collision packets*/
						pStats->rx_over_errors = 0; /* receiver ring buff overflow*/
						pStats->rx_crc_errors = 0; /* recved pkt with crc error*/
						pStats->rx_frame_errors = 0; /* recv'd frame alignment error*/
						pStats->rx_fifo_errors = 0; /* recv'r fifo overrun*/
						   
						DBGPRINT(RT_DEBUG_ERROR, ("CMD_RTPRIV_IOCTL_INF_STATS_GET: can not find mbss I/F\n"));
						return NDIS_STATUS_FAILURE;
					}
					
					pStats->rx_packets = pAd->ApCfg.MBSSID[index].RxCount;
					pStats->tx_packets = pAd->ApCfg.MBSSID[index].TxCount;
					pStats->rx_bytes = pAd->ApCfg.MBSSID[index].ReceivedByteCount;
					pStats->tx_bytes = pAd->ApCfg.MBSSID[index].TransmittedByteCount;
					pStats->rx_errors = pAd->ApCfg.MBSSID[index].RxErrorCount;
					pStats->tx_errors = pAd->ApCfg.MBSSID[index].TxErrorCount;
					pStats->multicast = pAd->ApCfg.MBSSID[index].mcPktsRx; /* multicast packets received */
					pStats->collisions = 0;  /* Collision packets*/
					pStats->rx_over_errors = 0;                   /* receiver ring buff overflow*/
					pStats->rx_crc_errors = 0;/* recved pkt with crc error*/
					pStats->rx_frame_errors = 0;          /* recv'd frame alignment error*/
					pStats->rx_fifo_errors = 0;                   /* recv'r fifo overrun*/
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

#ifdef CONFIG_AP_SUPPORT
			if (CurOpMode == OPMODE_AP)
			{
				if (pMacEntry != NULL)
					pStats->level =
						RTMPMaxRssi(pAd, pMacEntry->RssiSample.AvgRssi[0],
										pMacEntry->RssiSample.AvgRssi[1],
										pMacEntry->RssiSample.AvgRssi[2]);
				else
					pStats->level = RTMPMaxRssi(pAd, pAd->ApCfg.RssiSample.AvgRssi[0],
                                                 pAd->ApCfg.RssiSample.AvgRssi[1],
                                                 pAd->ApCfg.RssiSample.AvgRssi[2]);	
			}
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
			pStats->noise = RTMPMaxRssi(pAd, pAd->ApCfg.RssiSample.AvgRssi[0],
										pAd->ApCfg.RssiSample.AvgRssi[1],
										pAd->ApCfg.RssiSample.AvgRssi[2]) -
										RTMPMinSnr(pAd, pAd->ApCfg.RssiSample.AvgSnr[0],
										pAd->ApCfg.RssiSample.AvgSnr[1]);
#endif /* CONFIG_AP_SUPPORT */
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

#ifdef CONFIG_ATE
#ifdef CONFIG_QA
		case CMD_RTPRIV_IOCTL_ATE:
			RtmpDoAte(pAd, wrq, pData);
			break;
#endif /* CONFIG_QA */ 
#endif /* CONFIG_ATE */

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

#ifdef APCLI_SUPPORT
			if (pRate->priv_flags == INT_APCLI)
				HtPhyMode = pAd->ApCfg.ApCliTab[pObj->ioctl_if].wdev.HTPhyMode;
			else
#endif /* APCLI_SUPPORT */
#ifdef WDS_SUPPORT
			if (pRate->priv_flags == INT_WDS)
				HtPhyMode = pAd->WdsTab.WdsEntry[pObj->ioctl_if].wdev.HTPhyMode;
			else
#endif /* WDS_SUPPORT */
				HtPhyMode = pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev.HTPhyMode;

			RtmpDrvMaxRateGet(pAd, HtPhyMode.field.MODE, HtPhyMode.field.ShortGI,
							HtPhyMode.field.BW, HtPhyMode.field.MCS,
							(UINT32 *)&pRate->BitRate);
		}
			break;
#endif /* CONFIG_AP_SUPPORT */

		case CMD_RTPRIV_IOCTL_SIOCGIWNAME:
			RtmpIoctl_rt_ioctl_giwname(pAd, pData, 0);
			break;

	}


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
INT Set_SiteSurvey_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	NDIS_802_11_SSID Ssid;
	
#ifdef CONFIG_AP_SUPPORT
#ifdef AP_SCAN_SUPPORT
#endif /* AP_SCAN_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
	struct wifi_dev *wdev = NULL;	
	UCHAR ifIndex;
	POS_COOKIE pObjt = (POS_COOKIE) pAd->OS_Cookie;
#endif

#ifdef CONFIG_AP_SUPPORT
	ifIndex = pObjt->ioctl_if;
#ifdef APCLI_SUPPORT
		if (pObjt->ioctl_if_type == INT_APCLI)
			wdev = &pAd->ApCfg.ApCliTab[pObjt->ioctl_if].wdev;
		else 
#endif
		if (pObjt->ioctl_if_type == INT_MBSSID)
			wdev = &pAd->ApCfg.MBSSID[pObjt->ioctl_if].wdev;
		else
			wdev = &pAd->ApCfg.MBSSID[0].wdev;
#endif

	//check if the interface is down
	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_IN_USE))
	{
		DBGPRINT(RT_DEBUG_TRACE, ("INFO::Network is down!\n"));
		return -ENETDOWN;   
	}


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

		if (Ssid.SsidLength == 0)
			ApSiteSurvey(pAd, &Ssid, SCAN_PASSIVE, FALSE, wdev);
		else
			ApSiteSurvey(pAd, &Ssid, SCAN_ACTIVE, FALSE, wdev);

		return TRUE;
	}
#endif /* AP_SCAN_SUPPORT */
#endif // CONFIG_AP_SUPPORT //


	DBGPRINT(RT_DEBUG_TRACE, ("Set_SiteSurvey_Proc\n"));

    return TRUE;
}

INT	Set_Antenna_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
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
			




#ifdef HW_TX_RATE_LOOKUP_SUPPORT
INT Set_HwTxRateLookUp_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR Enable;
	UINT32 MacReg;

	Enable = simple_strtol(arg, 0, 10);

	RTMP_IO_READ32(pAd, TX_FBK_LIMIT, &MacReg);
	if (Enable)
	{
		MacReg |= 0x00040000;
		pAd->bUseHwTxLURate = TRUE;
		DBGPRINT(RT_DEBUG_TRACE, ("==>UseHwTxLURate (ON)\n"));
	}
	else
	{
		MacReg &= (~0x00040000);
		pAd->bUseHwTxLURate = FALSE;
		DBGPRINT(RT_DEBUG_TRACE, ("==>UseHwTxLURate (OFF)\n"));
	}
	RTMP_IO_WRITE32(pAd, TX_FBK_LIMIT, MacReg);

	DBGPRINT(RT_DEBUG_WARN, ("UseHwTxLURate = %d \n", pAd->bUseHwTxLURate));

	return TRUE;
}
#endif /* HW_TX_RATE_LOOKUP_SUPPORT */

#ifdef MAC_REPEATER_SUPPORT
#ifdef MULTI_MAC_ADDR_EXT_SUPPORT
INT Set_EnMultiMacAddrExt_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR Enable = simple_strtol(arg, 0, 10);

	pAd->bUseMultiMacAddrExt = (Enable ? TRUE : FALSE);
	AsicSetMacAddrExt(pAd, pAd->bUseMultiMacAddrExt);

	DBGPRINT(RT_DEBUG_WARN, ("UseMultiMacAddrExt = %d, UseMultiMacAddrExt(%s)\n",
				pAd->bUseMultiMacAddrExt, (Enable ? "ON" : "OFF")));

	return TRUE;
}

INT	Set_MultiMacAddrExt_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR tempMAC[6], idx;
	RTMP_STRING *token;
	RTMP_STRING sepValue[] = ":", DASH = '-';
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

INT set_tssi_enable(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
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

#ifdef RLT_MAC
#ifdef CONFIG_WIFI_TEST
INT set_pbf_loopback(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8 enable = 0;
	UINT32 value;

	// TODO: shiang-7603
	if (pAd->chipCap.hif_type == HIF_MT) {
		DBGPRINT(RT_DEBUG_OFF, ("%s(): Not support for HIF_MT yet!\n",
					__FUNCTION__));
		return FALSE;
	}

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

INT set_pbf_rx_drop(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8 enable = 0;
	UINT32 value;

	// TODO: shiang-7603
	if (pAd->chipCap.hif_type == HIF_MT) {
		DBGPRINT(RT_DEBUG_OFF, ("%s(): Not support for HIF_MT yet!\n",
					__FUNCTION__));
		return FALSE;
	}

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

INT set_fw_debug(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT8 fw_debug_param;

	fw_debug_param = simple_strtol(arg, 0, 10);

#ifdef RLT_MAC
	AndesRltFunSet(pAd, LOG_FW_DEBUG_MSG, fw_debug_param);
#endif /* RLT_MAC */

	return TRUE;
}
#endif

#ifdef MT_MAC
INT set_get_fid(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
    //TODO: Carter, at present, only can read pkt in Port2(LMAC port)
    volatile UCHAR   q_idx = 0, loop = 0, dw_idx = 0;
    UINT32  head_fid_addr = 0, dw_content, next_fid_addr = 0;
    volatile UINT32  value = 0x00000000L;
    q_idx = simple_strtol(arg, 0, 10);

    value = 0x00400000 | (q_idx << 16);//port2. queue by input value.
    RTMP_IO_WRITE32(pAd, 0x8024, value);
    RTMP_IO_READ32(pAd, 0x8024, &head_fid_addr);//get head FID.
    head_fid_addr = head_fid_addr & 0xfff;

    if (head_fid_addr == 0xfff) {
        DBGPRINT(RT_DEBUG_ERROR, ("%s, q_idx:%d empty!!\n", __func__, q_idx));
        return TRUE;
    }

    value = (0 | (head_fid_addr << 16));
    while (1) {
        for (dw_idx = 0; dw_idx < 8; dw_idx++) {
            RTMP_IO_READ32(pAd, ((MT_PCI_REMAP_ADDR_1 + (((value & 0x0fff0000) >> 16) * 128)) + (dw_idx * 4)), &dw_content);//get head FID.
            DBGPRINT(RT_DEBUG_ERROR, ("pkt:%d, fid:%x, dw_idx = %d, dw_content = 0x%x\n", loop, ((value & 0x0fff0000) >> 16), dw_idx, dw_content));
        }
        RTMP_IO_WRITE32(pAd, 0x8028, value);
        RTMP_IO_READ32(pAd, 0x8028, &next_fid_addr);//get next FID.
        if ((next_fid_addr & 0xfff) == 0xfff) {
            return TRUE;
        }

        value = (0 | ((next_fid_addr & 0xffff) << 16));
        loop++;
        if (loop > 5) {
            return TRUE;
        }
    }
    return TRUE;
}


#ifdef RTMP_MAC_PCI
INT Set_PDMAWatchDog_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 Dbg;

    Dbg = simple_strtol(arg, 0, 10);

	if (Dbg == 1)
	{
		pAd->PDMAWatchDogEn = 1;
	}
	else if (Dbg == 0)
	{
		pAd->PDMAWatchDogEn = 0;
	}
	else if (Dbg == 2)
	{
		PDMAResetAndRecovery(pAd);
	}
	else if (Dbg == 3)
	{
		pAd->PDMAWatchDogDbg = 0;
	}
	else if (Dbg == 4)
	{
		pAd->PDMAWatchDogDbg = 1;
	}

	return TRUE;
}
#endif /* RTMP_MAC_PCI */

INT SetPSEWatchDog_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 Dbg;

    Dbg = simple_strtol(arg, 0, 10);

	if (Dbg == 1)
	{
		pAd->PSEWatchDogEn = 1;
	}
	else if (Dbg == 0)
	{
		pAd->PSEWatchDogEn = 0;
	}
#ifdef RTMP_MAC_PCI

	else if (Dbg == 2)
	{
		PSEResetAndRecovery(pAd);
	}
	else if (Dbg == 3)
	{
		DumpPseInfo(pAd);

	}
#endif /* RTMP_MAC_PCI */

#ifdef DMA_RESET_SUPPORT	
	else if (Dbg == 4)
	{
		pAd->bcn_reset_en=TRUE;
	}
	else if (Dbg == 5)
	{
		pAd->bcn_reset_en=FALSE;
	}	
#endif /* DMA_RESET_SUPPORT */

	return TRUE;
}

#ifdef TXRXCR_DEBUG_SUPPORT
INT SetTxRxCr_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 Dbg;
	UINT32 mac_val;
	UINT32 idx;
    Dbg = simple_strtol(arg, 0, 10);

	if (Dbg == 0)
	{
		RTMP_IO_WRITE32(pAd, 0x14108, 0x80);
		RTMP_IO_WRITE32(pAd, 0x14108, 0xc0);
		RTMP_IO_WRITE32(pAd, 0x14108, 0x80);

		RTMP_IO_READ32(pAd, 0x2c030, &mac_val);
		DBGPRINT(RT_DEBUG_ERROR, ("0x60330030=0x%x\n",mac_val));

 		RTMP_IO_READ32(pAd, 0x2c01c, &mac_val);
		DBGPRINT(RT_DEBUG_ERROR, ("0x6033001C=0x%x\n",mac_val));
	}
	else if (Dbg == 1)
	{

		DBGPRINT(RT_DEBUG_ERROR, ("=============================================\n"));
		DBGPRINT(RT_DEBUG_ERROR, ("RX Status Counter\n"));			
 		RTMP_IO_READ32(pAd, 0x1410c, &mac_val);
		DBGPRINT(RT_DEBUG_ERROR, ("0x6020410C=0x%x\n",mac_val));
		
		RTMP_IO_READ32(pAd, 0x14110, &mac_val);
		DBGPRINT(RT_DEBUG_ERROR, ("0x60204110=0x%x\n",mac_val));

		RTMP_IO_READ32(pAd, 0x14114, &mac_val);
		DBGPRINT(RT_DEBUG_ERROR, ("0x60204114=0x%x\n",mac_val));		

		RTMP_IO_READ32(pAd, 0x1411c, &mac_val);
		DBGPRINT(RT_DEBUG_ERROR, ("0x6020411C=0x%x\n",mac_val));

		RTMP_IO_READ32(pAd, 0x14120, &mac_val);
		DBGPRINT(RT_DEBUG_ERROR, ("0x60204120=0x%x\n",mac_val));

		RTMP_IO_READ32(pAd, 0x2c030, &mac_val);
		DBGPRINT(RT_DEBUG_ERROR, ("0x60330030=0x%x\n",mac_val));

		RTMP_IO_READ32(pAd, 0x2c01c, &mac_val);
		DBGPRINT(RT_DEBUG_ERROR, ("0x6033001C=0x%x\n",mac_val));

		DBGPRINT(RT_DEBUG_ERROR, ("=============================================\n"));
		DBGPRINT(RT_DEBUG_ERROR, ("CCA Status\n"));
 		RTMP_IO_WRITE32(pAd, 0x20014, 0x1F);
		RTMP_IO_WRITE32(pAd, 0x20018, 0x07070707);
		RTMP_IO_WRITE32(pAd, 0x2004c, 0x02021919);

		RTMP_IO_READ32(pAd, 0x20024, &mac_val);
		DBGPRINT(RT_DEBUG_ERROR, ("0x60000024=0x%x\n",mac_val));
		DBGPRINT(RT_DEBUG_ERROR, ("=============================================\n"));

		for (idx=0;idx<10;idx++)
		{
			RTMP_IO_READ32(pAd, 0x21518, &mac_val);
			DBGPRINT(RT_DEBUG_ERROR, ("0x60120118[%d]=0x%08x    ",idx, mac_val));
			if (idx==4)
				DBGPRINT(RT_DEBUG_ERROR, ("\n"));
		}

		DBGPRINT(RT_DEBUG_ERROR, ("\n"));		
		
		RTMP_IO_WRITE32(pAd, 0x20014, 0x1f);
		RTMP_IO_WRITE32(pAd, 0x20014, 0x07070707);

		RTMP_IO_WRITE32(pAd, 0x2004c, 0x00000101);
		DBGPRINT(RT_DEBUG_ERROR, ("write 0x6000004c=0x00000101\n"));

		for (idx=0;idx<10;idx++)
		{
			RTMP_IO_READ32(pAd, 0x20024, &mac_val);
			DBGPRINT(RT_DEBUG_ERROR, ("0x60000024[%d]=0x%08x    ",idx, mac_val));
			if (idx==4)
				DBGPRINT(RT_DEBUG_ERROR, ("\n"));
		}
		DBGPRINT(RT_DEBUG_ERROR, ("\n"));

		RTMP_IO_WRITE32(pAd, 0x2004c, 0x02021919);
		DBGPRINT(RT_DEBUG_ERROR, ("write 0x6000004c=0x02021919\n"));
		for (idx=0;idx<10;idx++)
		{
			RTMP_IO_READ32(pAd, 0x20024, &mac_val);
			DBGPRINT(RT_DEBUG_ERROR, ("0x60000024[%d]=0x%08x    ",idx, mac_val));
			if (idx==4)
				DBGPRINT(RT_DEBUG_ERROR, ("\n"));
		}

		DBGPRINT(RT_DEBUG_ERROR, ("\n"));
		
		RTMP_IO_WRITE32(pAd, 0x2004c, 0x10101212);

		DBGPRINT(RT_DEBUG_ERROR, ("write 0x6000004c=0x10101212\n"));
		for (idx=0;idx<10;idx++)
		{
			RTMP_IO_READ32(pAd, 0x20024, &mac_val);
			DBGPRINT(RT_DEBUG_ERROR, ("0x60000024[%d]=0x%08x    ",idx, mac_val));
			if (idx==4)
				DBGPRINT(RT_DEBUG_ERROR, ("\n"));
		}

		DBGPRINT(RT_DEBUG_ERROR, ("\n"));
		
		RTMP_IO_WRITE32(pAd, 0x2004c, 0x00000B0B);

		DBGPRINT(RT_DEBUG_ERROR, ("write 0x6000004c=0x00000B0B\n"));
		for (idx=0;idx<10;idx++)
		{
			RTMP_IO_READ32(pAd, 0x20024, &mac_val);
			DBGPRINT(RT_DEBUG_ERROR, ("0x60000024[%d]=0x%08x    ",idx, mac_val));
			if (idx==4)
				DBGPRINT(RT_DEBUG_ERROR, ("\n"));
		}

		DBGPRINT(RT_DEBUG_ERROR, ("\n"));
		DBGPRINT(RT_DEBUG_ERROR, ("=============================================\n"));

		RTMP_IO_WRITE32(pAd, 0x2004c, 0x13131414);
		RTMP_IO_READ32(pAd, 0x20024, &mac_val);

		DBGPRINT(RT_DEBUG_ERROR, ("0x6000004c[0x13131414]=0x%08x    ", mac_val));
		RTMP_IO_WRITE32(pAd, 0x2004c, 0x1A1A1B1B);
		RTMP_IO_READ32(pAd, 0x20024, &mac_val);	

		DBGPRINT(RT_DEBUG_ERROR, ("0x6000004c[0x1A1A1B1B]=0x%08x    ", mac_val));
		RTMP_IO_WRITE32(pAd, 0x2004c, 0x16161D1D);
		RTMP_IO_READ32(pAd, 0x20024, &mac_val);	

		DBGPRINT(RT_DEBUG_ERROR, ("0x6000004c[0x16161D1D]=0x%08x    ", mac_val));
		DBGPRINT(RT_DEBUG_ERROR, ("\n"));

		RTMP_IO_WRITE32(pAd, 0x2004c, 0x26262727);
		RTMP_IO_READ32(pAd, 0x20024, &mac_val);	

		DBGPRINT(RT_DEBUG_ERROR, ("0x6000004c[0x26262727]=0x%08x    ", mac_val));
		RTMP_IO_WRITE32(pAd, 0x2004c, 0x28282929);
		RTMP_IO_READ32(pAd, 0x20024, &mac_val);	

		DBGPRINT(RT_DEBUG_ERROR, ("0x6000004c[0x28282929]=0x%08x    ", mac_val));
		RTMP_IO_WRITE32(pAd, 0x2004c, 0x2A2A2B2B);
		RTMP_IO_READ32(pAd, 0x20024, &mac_val);	

		DBGPRINT(RT_DEBUG_ERROR, ("0x6000004c[0x2A2A2B2B]=0x%08x    ", mac_val));
		DBGPRINT(RT_DEBUG_ERROR, ("\n"));
		DBGPRINT(RT_DEBUG_ERROR, ("=============================================\n"));

		RTMP_IO_READ32(pAd, 0x21710, &mac_val);	
		DBGPRINT(RT_DEBUG_ERROR, ("0x60130110=0x%08x    ", mac_val));

		RTMP_IO_READ32(pAd, 0x21714, &mac_val);	
		DBGPRINT(RT_DEBUG_ERROR, ("0x60130114=0x%08x    ", mac_val));

		RTMP_IO_READ32(pAd, 0x21718, &mac_val);	
		DBGPRINT(RT_DEBUG_ERROR, ("0x60130118=0x%08x    ", mac_val));		
		DBGPRINT(RT_DEBUG_ERROR, ("\n"));		

		RTMP_IO_READ32(pAd, 0x2171c, &mac_val);
		DBGPRINT(RT_DEBUG_ERROR, ("0x60130118=0x%08x    ", mac_val));

		RTMP_IO_READ32(pAd, 0x21720, &mac_val);
		DBGPRINT(RT_DEBUG_ERROR, ("0x60130120=0x%08x    ", mac_val));		

		RTMP_IO_READ32(pAd, 0x21724, &mac_val);	
		DBGPRINT(RT_DEBUG_ERROR, ("0x60130124=0x%08x    ", mac_val));
		DBGPRINT(RT_DEBUG_ERROR, ("\n"));

		RTMP_IO_READ32(pAd, 0x21728, &mac_val);
		DBGPRINT(RT_DEBUG_ERROR, ("0x60130128=0x%08x    ", mac_val));
		DBGPRINT(RT_DEBUG_ERROR, ("\n"));
		DBGPRINT(RT_DEBUG_ERROR, ("=============================================\n"));
		
		RTMP_IO_WRITE32(pAd, 0x20014, 0x1f);
		RTMP_IO_WRITE32(pAd, 0x20018, 0x05050505);

		RTMP_IO_WRITE32(pAd, 0x2004c, 0x85858686);
		RTMP_IO_READ32(pAd, 0x20024, &mac_val);

		DBGPRINT(RT_DEBUG_ERROR, ("0x6000004c[0x85858686]=0x%08x\n", mac_val));

		RTMP_IO_WRITE32(pAd, 0x20018, 0x05050404);
		RTMP_IO_WRITE32(pAd, 0x2004c, 0x81812A2A);
		RTMP_IO_READ32(pAd, 0x20024, &mac_val);

		DBGPRINT(RT_DEBUG_ERROR, ("0x6000004c[0x81812A2A]=0x%08x\n", mac_val));
		DBGPRINT(RT_DEBUG_ERROR, ("\n\n\n\n\n\n\n\n\n\n"));
		DBGPRINT(RT_DEBUG_ERROR, ("=============================================\n"));
	}
	
	return TRUE;
}
#endif /* TXRXCR_DEBUG_SUPPORT */



INT set_fw_log(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 LogType;
    LogType = simple_strtol(arg, 0, 10);

	if (LogType < 3)
		CmdFwLog2Host(pAd, LogType);
	else
		DBGPRINT(RT_DEBUG_OFF, (":%s: Unknown Log Type = %d\n", __FUNCTION__, LogType));

	return TRUE;
}

#ifdef THERMAL_PROTECT_SUPPORT
INT set_thermal_protection_criteria_proc(
	IN PRTMP_ADAPTER	pAd, 
	IN RTMP_STRING		*arg)
{
	UINT8 HighEn;
	CHAR HighTempTh;
	UINT8 LowEn;
	CHAR LowTempTh;
	CHAR *Param;

	Param = rstrtok(arg, ",");

	if (Param != NULL)
	{
		HighEn = simple_strtol(Param, 0, 10);
	}
	else
	{
		goto error;
	}

	Param = rstrtok(NULL, ",");

	if (Param != NULL)
	{
		HighTempTh = simple_strtol(Param, 0, 10);
	}
	else
	{
		goto error;
	}

	Param = rstrtok(NULL, ",");

	if (Param != NULL)
	{
		LowEn = simple_strtol(Param, 0, 10);
	}
	else
	{
		goto error;
	}

	Param = rstrtok(NULL, ",");

	if (Param != NULL)
	{
		LowTempTh = simple_strtol(Param, 0, 10);
	}
	else
	{
		goto error;
	}


	CmdThermalProtect(pAd, HighEn, HighTempTh, LowEn, LowTempTh);	

	DBGPRINT(RT_DEBUG_OFF, ("%s: high_en=%d, high_thd = %d, low_en = %d, low_thd = %d\n", __FUNCTION__, HighEn, HighTempTh, LowEn, LowTempTh));
	
	return TRUE;

error:
	DBGPRINT(RT_DEBUG_OFF, ("iwpriv ra0 set tpc=high_en,high_thd,low_en,low_thd\n"));
	return TRUE;

}
#endif /* THERMAL_PROTECT_SUPPORT */

VOID StatRateToString(RTMP_ADAPTER *pAd, CHAR *Output, UCHAR TxRx, UINT32 RawData)
{
	extern UCHAR tmi_rate_map_ofdm[];
	extern UCHAR tmi_rate_map_cck_lp[];
	extern UCHAR tmi_rate_map_cck_sp[];
	UCHAR phy_mode, rate, preamble;
	CHAR *phyMode[5] = {"CCK", "OFDM", "MM", "GF", "VHT"};

	phy_mode = RawData>>13;
	rate = RawData & 0x3F;

	if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED))
		preamble = SHORT_PREAMBLE;
	else
		preamble = LONG_PREAMBLE;

	if ( TxRx == 0 )
		sprintf(Output+strlen(Output), "Last TX Rate                    = ");
	else
		sprintf(Output+strlen(Output), "Last RX Rate                    = ");

	if ( phy_mode == MODE_CCK ) {

		if ( TxRx == 0 )
		{
			if (preamble)
				rate = tmi_rate_map_cck_lp[rate];
			else
				rate = tmi_rate_map_cck_sp[rate];
		}
		
		if ( rate == TMI_TX_RATE_CCK_1M_LP )
			sprintf(Output+strlen(Output), "1M, ");
		else if ( rate == TMI_TX_RATE_CCK_2M_LP )
			sprintf(Output+strlen(Output), "2M, ");
		else if ( rate == TMI_TX_RATE_CCK_5M_LP )
			sprintf(Output+strlen(Output), "5M, ");
		else if ( rate == TMI_TX_RATE_CCK_11M_LP )
			sprintf(Output+strlen(Output), "11M, ");
		else if ( rate == TMI_TX_RATE_CCK_2M_SP )
			sprintf(Output+strlen(Output), "2M, ");
		else if ( rate == TMI_TX_RATE_CCK_5M_SP )
			sprintf(Output+strlen(Output), "5M, ");
		else if ( rate == TMI_TX_RATE_CCK_11M_SP )
			sprintf(Output+strlen(Output), "11M, ");
		else
			sprintf(Output+strlen(Output), "unkonw, ");

	} else if ( phy_mode == MODE_OFDM ) {

		if ( TxRx == 0 )
		{
			rate = tmi_rate_map_ofdm[rate];
		}

		if ( rate == TMI_TX_RATE_OFDM_6M )
			sprintf(Output+strlen(Output), "6M, ");
		else if ( rate == TMI_TX_RATE_OFDM_9M )
			sprintf(Output+strlen(Output), "9M, ");
		else if ( rate == TMI_TX_RATE_OFDM_12M )
			sprintf(Output+strlen(Output), "12M, ");
		else if ( rate == TMI_TX_RATE_OFDM_18M )
			sprintf(Output+strlen(Output), "18M, ");
		else if ( rate == TMI_TX_RATE_OFDM_24M )
			sprintf(Output+strlen(Output), "24M, ");
		else if ( rate == TMI_TX_RATE_OFDM_36M )
			sprintf(Output+strlen(Output), "36M, ");
		else if ( rate == TMI_TX_RATE_OFDM_48M )
			sprintf(Output+strlen(Output), "48M, ");
		else if ( rate == TMI_TX_RATE_OFDM_54M )
			sprintf(Output+strlen(Output), "54M, ");
		else
			sprintf(Output+strlen(Output), "unkonw, ");
	} else {
			sprintf(Output+strlen(Output), "MCS%d, ", rate);
	}
	sprintf(Output+strlen(Output), "%2dM, ", ((RawData>>7) & 0x1)? 40: 20);
	sprintf(Output+strlen(Output), "%cGI, ", ((RawData>>9) & 0x1)? 'S': 'L');
	sprintf(Output+strlen(Output), "%s%s\n", phyMode[(phy_mode) & 0x3], ((RawData>>10) & 0x3)? ", STBC": " ");

}


INT Set_themal_sensor(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	/* 0: get temperature; 1: get adc */
	UINT32 value;
	value = simple_strtol(arg, 0, 10);

	if ((value == 0) || (value == 1)) {
#if defined(MT7603) || defined(MT7628)
#ifdef DBG
		UINT32 temperature=0; 
		temperature = MtAsicGetThemalSensor(pAd, value);
		DBGPRINT(RT_DEBUG_OFF, ("%s: ThemalSensor = 0x%x\n", __FUNCTION__, temperature));
#endif /* DBG */
#else
		CmdGetThemalSensorResult(pAd, value);
#endif /* MT7603 ||MT7628  */
	} else
		DBGPRINT(RT_DEBUG_OFF, (":%s: 0: get temperature; 1: get adc\n", __FUNCTION__));

	return TRUE;
}


INT Set_rx_pspoll_filter_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 value;	
	value = simple_strtol(arg, 0, 10);

	pAd->rx_pspoll_filter = value;
	MtAsicSetRxPspollFilter(pAd, pAd->rx_pspoll_filter);

	DBGPRINT(RT_DEBUG_OFF, (":%s: rx_pspoll_filter=%d\n", __FUNCTION__, pAd->rx_pspoll_filter));
	return TRUE;
}

#ifdef SMART_CARRIER_SENSE_SUPPORT
INT SetSCSEnable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT32 value;
	
	value = simple_strtol(arg, 0, 10);
	

	if (value == 1)
	{
		pAd->SCSCtrl.SCSEnable= SCS_ENABLE;
		DBGPRINT(RT_DEBUG_OFF, ("==>%s (ON)\n", __FUNCTION__));
	}
	else if (value == 0)
	{
		pAd->SCSCtrl.SCSEnable= SCS_DISABLE;
		DBGPRINT(RT_DEBUG_OFF, ("==>%s (OFF)\n", __FUNCTION__));
		/* Restore to default */
		RTMP_IO_WRITE32(pAd, CR_AGC_0, pAd->SCSCtrl.CR_AGC_0_default);
		RTMP_IO_WRITE32(pAd, CR_AGC_0_RX1, pAd->SCSCtrl.CR_AGC_0_default);
		RTMP_IO_WRITE32(pAd, CR_AGC_3, pAd->SCSCtrl.CR_AGC_3_default);
		RTMP_IO_WRITE32(pAd, CR_AGC_3_RX1, pAd->SCSCtrl.CR_AGC_3_default);
		pAd->SCSCtrl.SCSStatus = SCS_STATUS_DEFAULT;
	}
	else
		DBGPRINT(RT_DEBUG_OFF, ("==>%s (Unknow value = %d)\n", __FUNCTION__, value));

	return TRUE;
}

INT SetSCSCfg_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32   Recv=0;
	INT	SCSMinRssiTolerance = 0, SCSTrafficThreshold = 0, FalseCcaUpBond = 0, FalseCcaLowBond = 0, FixedBond = 0, ForceMode = 0;

	Recv = sscanf(arg, "%d-%d-%d-%d-%d-%d", &(SCSMinRssiTolerance), &(SCSTrafficThreshold), &(FalseCcaUpBond), &(FalseCcaLowBond), &(FixedBond), &(ForceMode));
	if (Recv != 6){
		DBGPRINT(RT_DEBUG_OFF, ("Format Error!\n"));
		DBGPRINT(RT_DEBUG_OFF, ("iwpriv ra0 set SCSCfg=[MinRssiTolerance]-[TrafficThreshold]-[FalseCcaUpBoundary]-[FalseCcaLowBoundary]-[FixedBoundary]-[ForceMode]\n"));
		DBGPRINT(RT_DEBUG_OFF, ("PS: FiexedBond is Negative number. Ex:70 means -70dBm"));
	} else {
		pAd->SCSCtrl.SCSMinRssiTolerance = (UINT8)SCSMinRssiTolerance;	
		pAd->SCSCtrl.SCSTrafficThreshold = SCSTrafficThreshold;
		pAd->SCSCtrl.FalseCcaUpBond = (UINT16)FalseCcaUpBond;
		pAd->SCSCtrl.FalseCcaLowBond = (UINT16)FalseCcaLowBond;
		pAd->SCSCtrl.FixedRssiBond = (0 -(CHAR)FixedBond);	
		pAd->SCSCtrl.ForceMode = (BOOLEAN)ForceMode;
	}
	return TRUE;
}
#endif /* SMART_CARRIER_SENSE_SUPPORT */

#endif /* MT_MAC */
#ifdef SW_ATF_SUPPORT
/*Set badNodeEntry max and min enq threshold. Default set enq_badNodeMaxThr = 4,enq_badNodeMinThr = 3*/
INT SetBadNodePara_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Recv = 0;
	INT	minEnqThr, maxEnqThr;

	Recv = sscanf(arg, "%d-%d", &(minEnqThr), &(maxEnqThr));
	if (Recv != 2) {
		DBGPRINT(RT_DEBUG_OFF, ("Format Error!\n"));
		DBGPRINT(RT_DEBUG_OFF, ("iwpriv ra0 set badNodePara=[minEnqThr]-[maxEnqThr]\n"));
	} else {
		pAd->AtfParaSet.enq_badNodeMaxThr = maxEnqThr;
		pAd->AtfParaSet.enq_badNodeMinThr = minEnqThr;
		DBGPRINT(RT_DEBUG_OFF, ("minEnqThr = %d,maxEnqThr = %d\n",
			pAd->AtfParaSet.enq_badNodeMinThr, pAd->AtfParaSet.enq_badNodeMaxThr));
	}
	return TRUE;
}
/*Set goodNodeEntry max and min deq threshold. Default set deq_goodNodeMaxThr = 468,deq_goodNodeMinThr = 390*/
INT SetGoodNodePara_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Recv = 0;

	INT	minDeqThr, maxDeqThr;

	Recv = sscanf(arg, "%d-%d", &(minDeqThr), &(maxDeqThr));
	if (Recv != 2) {
		DBGPRINT(RT_DEBUG_OFF, ("Format Error!\n"));
		DBGPRINT(RT_DEBUG_OFF, ("iwpriv ra0 set goodNodePara=[minDeqThr]-[maxDeqThr]\n"));
	} else {
		pAd->AtfParaSet.deq_goodNodeMaxThr = maxDeqThr;
		pAd->AtfParaSet.deq_goodNodeMinThr = minDeqThr;
		DBGPRINT(RT_DEBUG_OFF, ("minDeqThr = %d,maxDeqThr = %d\n",
			pAd->AtfParaSet.deq_goodNodeMinThr, pAd->AtfParaSet.deq_goodNodeMaxThr));
	}
	return TRUE;
}
INT SetFixAtfPara_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 value;

	value = simple_strtol(arg, 0, 10);
	pAd->AtfParaSet.flagOnce = value;
	pAd->AtfParaSet.enq_badNodeCurrent = 4;
	DBGPRINT(RT_DEBUG_OFF, ("fixAtfPara:%d\n", pAd->AtfParaSet.flagOnce));
	return TRUE;
}
/*Set txThreshold,Default set wcidTxThr = 43*/
INT SetTxThr_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 value;

	value = simple_strtol(arg, 0, 10);
	pAd->AtfParaSet.wcidTxThr = value;
	DBGPRINT(RT_DEBUG_OFF, ("wcidTxThr:%d\n", pAd->AtfParaSet.wcidTxThr));
	return TRUE;

}
/*Set badNodeEntry min deq Threshold. Default set deq_badNodeMinThr = 43*/
INT SetBadNodeMinDeq_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 value;

	value = simple_strtol(arg, 0, 10);
	pAd->AtfParaSet.deq_badNodeMinThr = value;
	DBGPRINT(RT_DEBUG_OFF, ("badNodeMinDeqCnt:%d\n", pAd->AtfParaSet.deq_badNodeMinThr));
	return TRUE;
}
/*Set enq period time. Default set dropDelta = 2,periodTimer = 75um(75um = (1+2)*25um)*/
INT SetAtfDropDelta_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 value;

	value = simple_strtol(arg, 0, 10);
	pAd->AtfParaSet.dropDelta = value;
	DBGPRINT(RT_DEBUG_OFF, ("dropDelta:%d\n", pAd->AtfParaSet.dropDelta));
	return TRUE;

}
/*set atf false cca*/
INT SetAtfFalseCCAThr_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 value;

	value = simple_strtol(arg, 0, 10);
	pAd->AtfParaSet.atfFalseCCAThr = value;
	DBGPRINT(RT_DEBUG_OFF, ("atfFalseCCAThr:%d\n", pAd->AtfParaSet.atfFalseCCAThr));
	return TRUE;

}
#endif

#ifdef SINGLE_SKU_V2
INT SetSKUEnable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UCHAR value;
	
	value = simple_strtol(arg, 0, 10);
	
	if (value)
	{
		pAd->SKUEn = 1;
		DBGPRINT(RT_DEBUG_ERROR, ("==>SetSKUEnable_Proc (ON)\n"));
	}
	else
	{
		pAd->SKUEn = 0;
		DBGPRINT(RT_DEBUG_ERROR, ("==>SetSKUEnable_Proc (OFF)\n"));
	}

	AsicSwitchChannel(pAd, pAd->CommonCfg.Channel, FALSE);
	
	return TRUE;
}
#endif /* SINGLE_SKU_V2 */

/* run-time turn EDCCA on/off */
INT Set_ed_chk_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	UINT ed_chk = simple_strtol(arg, 0, 10);
	
	DBGPRINT(RT_DEBUG_OFF, ("%s()::ed_chk=%d\n", 
		__FUNCTION__, ed_chk));
	
	pAd->ed_chk = ed_chk;
	if (ed_chk != 0)
		RTMP_CHIP_ASIC_SET_EDCCA(pAd, TRUE);
	else
		RTMP_CHIP_ASIC_SET_EDCCA(pAd, FALSE);
	
	return TRUE;
}
INT ed_status_read(RTMP_ADAPTER *pAd)
{
	UINT32 period_us = pAd->ed_chk_period * 1000;
	ULONG irqflag=0;
	INT percent;
	BOOLEAN bIsCERegion = FALSE;
	UINT32 ed_th = 0;

	UINT32 ch_idle_stat=0, ch_busy_stat=0, ed_stat=0, rssi_stat=0;
	INT32 WBRssi0, WBRssi1, RefWBRssi;
	
	RTMP_IO_READ32(pAd, 0x2c050, &ed_stat);

	RTMP_IRQ_LOCK(&pAd->irq_lock, irqflag);
	
	pAd->ch_idle_stat[pAd->ed_stat_lidx] = ch_idle_stat;
	pAd->ch_busy_stat[pAd->ed_stat_lidx] = ch_busy_stat;	
	pAd->ed_stat[pAd->ed_stat_lidx] = ed_stat;
	NdisGetSystemUpTime(&pAd->chk_time[pAd->ed_stat_lidx]);

	RTMP_IO_READ32(pAd,RO_AGC_DEBUG_2,&rssi_stat);
	
	//IBRssi0 = (rssi_stat&0xFF000000)>>24;
	WBRssi0 = (rssi_stat&0x00FF0000)>>16;
	//IBRssi1 = (rssi_stat&0x0000FF00)>>8;
	WBRssi1 = (rssi_stat&0x000000FF);

	if(WBRssi0>=128)
		WBRssi0-=256;	

	if(WBRssi1>=128)
		WBRssi1-=256;

	RefWBRssi = max(WBRssi0,WBRssi1);
	pAd->rssi_stat[pAd->ed_stat_lidx] = RefWBRssi;
	if(RefWBRssi >= -40 && pAd->ed_big_rssi_count < pAd->ed_block_tx_threshold)
		pAd->ed_big_rssi_count++;
	else if(pAd->ed_big_rssi_count > 0)
		pAd->ed_big_rssi_count--;
	
	pAd->ed_big_rssi_stat[pAd->ed_stat_lidx] = pAd->ed_big_rssi_count * 100 / pAd->ed_block_tx_threshold;
	
	if ((pAd->ed_threshold > 0) && (period_us > 0) && (pAd->ed_block_tx_threshold > 0)) {
		percent = (pAd->ed_stat[pAd->ed_stat_lidx] * 100 ) / period_us;
		if (percent > 100)
			percent = 100;

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
			DBGPRINT(RT_DEBUG_ERROR, ("@@@ %s: pAd->ed_trigger_cnt=%d,	pAd->ed_silent_cnt=%u, percent=%d !!\n", 
				__FUNCTION__, pAd->ed_trigger_cnt, pAd->ed_silent_cnt
			,  percent));
		}
	}
	pAd->ed_trigger_stat[pAd->ed_stat_lidx] = pAd->ed_trigger_cnt;
	pAd->ed_silent_stat[pAd->ed_stat_lidx] = pAd->ed_silent_cnt;

	if(pAd->ed_chk != FALSE)
	{
		UINT32 macVal = 0, macVal2 = 0;
		bIsCERegion = GetEDCCASupport(pAd);
		ed_th = pAd->ed_th;

		RTMP_IO_READ32(pAd, WF_PHY_BASE + 0x0634, &macVal2);
		if (pAd->ed_trigger_cnt > pAd->ed_block_tx_threshold || pAd->ed_big_rssi_stat[pAd->ed_stat_lidx] < 50) {
			//EDCCA CERT
			if(pAd->ed_threshold_strict != TRUE)
			{
				pAd->ed_threshold_strict = TRUE;
				macVal = 0xD7C87D10;  		
				if((!bIsCERegion) && (ed_th < NON_CE_REGION_MAX_ED_TH)) {
					macVal = macVal & (~(0x7F << 8));
					macVal = macVal | (ed_th << 8);
				}
				RTMP_IO_WRITE32(pAd, WF_PHY_BASE + 0x0618, macVal);

				macVal2 &= 0xFFFFFFFE;
				RTMP_IO_WRITE32(pAd, WF_PHY_BASE + 0x0634, macVal2);
				
				DBGPRINT(RT_DEBUG_WARN, ("\n====\n@@@ %s: EDCCA TH - H\npAd->ed_trigger_cnt : %d > %d ||  pAd->ed_big_rssi_stat : %d < 50\n====\n"
				,__FUNCTION__,pAd->ed_trigger_cnt,pAd->ed_block_tx_threshold,pAd->ed_big_rssi_stat[pAd->ed_stat_lidx]));
				
			}
		}

		if (pAd->ed_silent_cnt > pAd->ed_block_tx_threshold && pAd->ed_big_rssi_stat[pAd->ed_stat_lidx] >= 50) {
			//EDCCA USER
			if(pAd->ed_threshold_strict != FALSE)
			{
				pAd->ed_threshold_strict = FALSE;
				macVal = 0xD7C87D0F;  
				if((!bIsCERegion) && (ed_th < NON_CE_REGION_MAX_ED_TH)) {
					macVal = macVal & (~(0x7F << 8));
					macVal = macVal | (ed_th << 8);
				}
				RTMP_IO_WRITE32(pAd, WF_PHY_BASE + 0x0618, macVal);

				macVal2 |= 0x1;
				RTMP_IO_WRITE32(pAd, WF_PHY_BASE + 0x0634, macVal2);
				
				DBGPRINT(RT_DEBUG_WARN, ("\n====\n@@@ %s: EDCCA TH - L\npAd->ed_silent_cnt : %d > %d && pAd->ed_big_rssi_stat : %d >= 50\n====\n"
				,__FUNCTION__,pAd->ed_silent_cnt,pAd->ed_block_tx_threshold,pAd->ed_big_rssi_stat[pAd->ed_stat_lidx]));
				
			}
		}
	}

        INC_RING_INDEX(pAd->ed_stat_lidx, ED_STAT_CNT);
        //pAd->false_cca_stat[pAd->ed_stat_lidx] = 0;
        if (pAd->ed_stat_sidx == pAd->ed_stat_lidx) {
                INC_RING_INDEX(pAd->ed_stat_sidx, ED_STAT_CNT);
        }

        RTMP_IRQ_UNLOCK(&pAd->irq_lock, irqflag);

       if (pAd->ed_trigger_cnt > pAd->ed_block_tx_threshold) {
               if (pAd->ed_tx_stoped == FALSE) {
                       pAd->ed_tx_stoped = TRUE;
                       DBGPRINT(RT_DEBUG_WARN, ("\n====\n### %s: EDCCA ed_tx_stoped is set to TRUE\n====\n", __FUNCTION__));
               }
       }

       if (pAd->ed_silent_cnt > pAd->ed_block_tx_threshold) {
               if (pAd->ed_tx_stoped == TRUE) {
                       pAd->ed_tx_stoped = FALSE;
                       DBGPRINT(RT_DEBUG_WARN, ("\n====\n### %s: EDCCA ed_tx_stoped is set to FALSE\n====\n", __FUNCTION__));
               }
       }

	return TRUE;
}

//this function will be called in multi entry
INT ed_monitor_exit(RTMP_ADAPTER *pAd)
{
	ULONG irqflag=0;

	RTMP_IRQ_LOCK(&pAd->irq_lock, irqflag);
	DBGPRINT(RT_DEBUG_OFF, ("@@@ %s : ===>\n", __FUNCTION__));
	
	NdisZeroMemory(&pAd->ed_stat[0], sizeof(pAd->ed_stat));
	NdisZeroMemory(&pAd->ch_idle_stat[0], sizeof(pAd->ch_idle_stat));
	NdisZeroMemory(&pAd->ch_busy_stat[0], sizeof(pAd->ch_busy_stat));
	NdisZeroMemory(&pAd->chk_time[0], sizeof(pAd->chk_time));
	NdisZeroMemory(&pAd->ed_trigger_stat[0], sizeof(pAd->ed_trigger_stat));
	NdisZeroMemory(&pAd->ed_silent_stat[0], sizeof(pAd->ed_silent_stat));	
	NdisZeroMemory(&pAd->rssi_stat[0], sizeof(pAd->rssi_stat));
	
	
	pAd->ed_stat_lidx = pAd->ed_stat_sidx = 0;
	pAd->ed_trigger_cnt = 0;
	pAd->ed_silent_cnt = 0;
	//ignore fisrt time's incorrect false cca 
	pAd->ed_false_cca_cnt = 0;

	pAd->ed_tx_stoped = FALSE;
	pAd->ed_threshold_strict = FALSE;
	//also clear top level flags
	pAd->ed_chk = FALSE;
	DBGPRINT(RT_DEBUG_OFF, ("@@@ %s : <===\n", __FUNCTION__));
	RTMP_IRQ_UNLOCK(&pAd->irq_lock, irqflag);

	RTMP_CHIP_ASIC_SET_EDCCA(pAd,FALSE);
	
	return TRUE;
}


// open & muanl cmd will call
INT ed_monitor_init(RTMP_ADAPTER *pAd)
{
	ULONG irqflag=0;

	RTMP_IRQ_LOCK(&pAd->irq_lock, irqflag);
	DBGPRINT(RT_DEBUG_OFF, ("@@@ %s : ===>\n", __FUNCTION__));
	NdisZeroMemory(&pAd->ed_stat[0], sizeof(pAd->ed_stat));
	NdisZeroMemory(&pAd->ch_idle_stat[0], sizeof(pAd->ch_idle_stat));
	NdisZeroMemory(&pAd->ch_busy_stat[0], sizeof(pAd->ch_busy_stat));
	NdisZeroMemory(&pAd->chk_time[0], sizeof(pAd->chk_time));
	NdisZeroMemory(&pAd->ed_trigger_stat[0], sizeof(pAd->ed_trigger_stat));
	NdisZeroMemory(&pAd->ed_silent_stat[0], sizeof(pAd->ed_silent_stat));	
	NdisZeroMemory(&pAd->rssi_stat[0], sizeof(pAd->rssi_stat));
	
	
	pAd->ed_stat_lidx = pAd->ed_stat_sidx = 0;
	pAd->ed_trigger_cnt = 0;
	pAd->ed_silent_cnt = 0;

	// ignore fisrt time's incorrect false cca
	pAd->ed_false_cca_cnt = 0;
	
	pAd->ed_tx_stoped = FALSE;	
	pAd->ed_threshold_strict = FALSE;
	//also set  top level flags
	if(pAd->ed_chk == 0)	//turn on if disabled , otherwise decide by profile
		pAd->ed_chk = 1; 

	DBGPRINT(RT_DEBUG_OFF, ("@@@ %s : <===\n", __FUNCTION__));
	RTMP_IRQ_UNLOCK(&pAd->irq_lock, irqflag);

	RTMP_CHIP_ASIC_SET_EDCCA(pAd,TRUE);
	return TRUE;
}

#ifdef DBG
INT show_ed_stat_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	unsigned long irqflags=0;
	UINT32 ed_stat[ED_STAT_CNT], ed_big_rssi_stat[ED_STAT_CNT], rssi_stat[ED_STAT_CNT];
	UINT32 silent_stat[ED_STAT_CNT], trigger_stat[ED_STAT_CNT]; 
	UINT32 busy_stat[ED_STAT_CNT], idle_stat[ED_STAT_CNT];
	ULONG chk_time[ED_STAT_CNT];
	INT period_us;
	UCHAR start, end, idx;
		
	RTMP_IRQ_LOCK(&pAd->irq_lock, irqflags);
	start = pAd->ed_stat_sidx;
	end = pAd->ed_stat_lidx;
	NdisMoveMemory(&ed_stat[0], &pAd->ed_stat[0], sizeof(ed_stat));
	NdisMoveMemory(&ed_big_rssi_stat[0], &pAd->ed_big_rssi_stat[0], sizeof(ed_big_rssi_stat));
	NdisMoveMemory(&busy_stat[0], &pAd->ch_busy_stat[0], sizeof(busy_stat));
	NdisMoveMemory(&idle_stat[0], &pAd->ch_idle_stat[0], sizeof(idle_stat));
	NdisMoveMemory(&chk_time[0], &pAd->chk_time[0], sizeof(chk_time));
	NdisMoveMemory(&trigger_stat[0], &pAd->ed_trigger_stat[0], sizeof(
trigger_stat));
	NdisMoveMemory(&silent_stat[0], &pAd->ed_silent_stat[0], sizeof(silent_stat));
	NdisMoveMemory(&rssi_stat[0], &pAd->rssi_stat[0], sizeof(
rssi_stat));
	RTMP_IRQ_UNLOCK(&pAd->irq_lock, irqflags);

#ifdef CONFIG_AP_SUPPORT
	DBGPRINT(RT_DEBUG_OFF, ("Dump ChannelBusy Counts, ed_chk=%u, false_cca_threshold=%u, ChkPeriod=%dms, ED_Threshold=%d%%, HitCntForBlockTx=%d\n", 
	pAd->ed_chk, pAd->ed_false_cca_threshold, pAd->ed_chk_period, pAd->ed_threshold, pAd->ed_block_tx_threshold));
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

	DBGPRINT(RT_DEBUG_OFF, ("RSSI:"));
	idx = start;
	do {
		DBGPRINT(RT_DEBUG_OFF, ("\t%d", rssi_stat[idx]));
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


	DBGPRINT(RT_DEBUG_OFF, ("BIG WBRSSI %% - Over -40dBm \n"));
	/*DBGPRINT(RT_DEBUG_OFF, ("RawCnt:"));
	idx = start;
	do {
		DBGPRINT(RT_DEBUG_OFF, ("%d  ", ed_2nd_stat[idx]));
		INC_RING_INDEX(idx, ED_STAT_CNT);
	} while (idx != end);
	DBGPRINT(RT_DEBUG_OFF, ("\n"));
	*/
	DBGPRINT(RT_DEBUG_OFF, ("Percent:"));
	idx = start;
	do {
		DBGPRINT(RT_DEBUG_OFF, ("\t%d", ed_big_rssi_stat[idx]));
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
#endif /* DBG */

INT	Set_RadioOn_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	UCHAR radio;

	radio = simple_strtol(arg, 0, 10);

	pAd->iwpriv_command = FALSE;
#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT) {
		pAd->iwpriv_command = TRUE;		
	}
#endif /* MT_MAC */

	if (radio)
	{
		MlmeRadioOn(pAd);
#ifdef FT_R1KH_KEEP
		pAd->ApCfg.FtTab.FT_RadioOff = FALSE;
#endif/* FT_R1KH_KEEP */
		DBGPRINT(RT_DEBUG_OFF, ("==>Set_RadioOn_Proc (ON)\n"));
	}
	else
	{
		MlmeRadioOff(pAd);
#ifdef FT_R1KH_KEEP
		pAd->ApCfg.FtTab.FT_RadioOff = TRUE;
#endif/* FT_R1KH_KEEP */
		DBGPRINT(RT_DEBUG_OFF, ("==>Set_RadioOn_Proc (OFF)\n"));
	}

	return TRUE;
}

#if defined(CUSTOMER_DCC_FEATURE) || defined(BAND_STEERING)
VOID EnableRadioChstats( 
		IN	PRTMP_ADAPTER 	pAd)
{
	UINT32   mac_val;
	
	/* Clear previous status */
	RTMP_IO_READ32(pAd, MIB_MSCR, &mac_val);
	DBGPRINT(RT_DEBUG_OFF, ("MIB Status Control=0x%x\n", mac_val));
	/*mt7603_set_ed_cca(pAd, 1);*/
	RTMP_IO_READ32(pAd, MIB_MSDR9, &mac_val); //	Ch Busy Time
	RTMP_IO_READ32(pAd, MIB_MSDR18, &mac_val); // 	p_ED Time
	AsicGetRxStat(pAd, HQA_RX_RESET_PHY_COUNT);
	
}
#endif

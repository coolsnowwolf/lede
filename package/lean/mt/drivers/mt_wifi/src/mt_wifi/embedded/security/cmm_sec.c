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
	cmm_sec.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
*/
#include "rt_config.h"

VOID SetWdevAuthMode(
	IN struct _SECURITY_CONFIG *pSecConfig,
	IN RTMP_STRING * arg)
{
	UINT32 AKMMap = 0;

	CLEAR_SEC_AKM(AKMMap);

	if (rtstrcasecmp(arg, "OPEN") == TRUE)
		SET_AKM_OPEN(AKMMap);
	else if (rtstrcasecmp(arg, "SHARED") == TRUE)
		SET_AKM_SHARED(AKMMap);
	else if (rtstrcasecmp(arg, "WEPAUTO") == TRUE) {
		SET_AKM_OPEN(AKMMap);
		SET_AKM_AUTOSWITCH(AKMMap);
	} else if (rtstrcasecmp(arg, "WPA") == TRUE)
		SET_AKM_WPA1(AKMMap);
	else if (rtstrcasecmp(arg, "WPAPSK") == TRUE)
		SET_AKM_WPA1PSK(AKMMap);
	else if (rtstrcasecmp(arg, "WPANONE") == TRUE)
		SET_AKM_WPANONE(AKMMap);
	else if (rtstrcasecmp(arg, "WPA2") == TRUE)
		SET_AKM_WPA2(AKMMap);
	else if (rtstrcasecmp(arg, "WPA2PSK") == TRUE)
		SET_AKM_WPA2PSK(AKMMap);
	else if (rtstrcasecmp(arg, "WPA3") == TRUE) {
		/* WPA3 code flow is same as WPA2, the usage of SEC_AKM_WPA3 is to force pmf on */
		SET_AKM_WPA2(AKMMap);
		SET_AKM_WPA3(AKMMap);
	}
#ifdef DOT11_SUITEB_SUPPORT
	else if (rtstrcasecmp(arg, "WPA3-192") == TRUE)
		SET_AKM_WPA3_192BIT(AKMMap);
#endif
#ifdef DOT11_SAE_SUPPORT
	else if (rtstrcasecmp(arg, "WPA3PSK") == TRUE)
		SET_AKM_SAE_SHA256(AKMMap);
	else if (rtstrcasecmp(arg, "WPA2PSKWPA3PSK") == TRUE) {
		SET_AKM_SAE_SHA256(AKMMap);
		SET_AKM_WPA2PSK(AKMMap);
	}
#endif /* DOT11_SAE_SUPPORT */
	else if (rtstrcasecmp(arg, "WPA1WPA2") == TRUE) {
		SET_AKM_WPA1(AKMMap);
		SET_AKM_WPA2(AKMMap);
	} else if (rtstrcasecmp(arg, "WPAPSKWPA2PSK") == TRUE) {
		SET_AKM_WPA1PSK(AKMMap);
		SET_AKM_WPA2PSK(AKMMap);
	} else if ((rtstrcasecmp(arg, "WPA_AES_WPA2_TKIPAES") == TRUE)
			   || (rtstrcasecmp(arg, "WPA_AES_WPA2_TKIP") == TRUE)
			   || (rtstrcasecmp(arg, "WPA_TKIP_WPA2_AES") == TRUE)
			   || (rtstrcasecmp(arg, "WPA_TKIP_WPA2_TKIPAES") == TRUE)
			   || (rtstrcasecmp(arg, "WPA_TKIPAES_WPA2_AES") == TRUE)
			   || (rtstrcasecmp(arg, "WPA_TKIPAES_WPA2_TKIPAES") == TRUE)
			   || (rtstrcasecmp(arg, "WPA_TKIPAES_WPA2_TKIP") == TRUE)) {
		SET_AKM_WPA1PSK(AKMMap);
		SET_AKM_WPA2PSK(AKMMap);
	} else if (rtstrcasecmp(arg, "OWE") == TRUE) {
		SET_AKM_OWE(AKMMap);
	}

	else {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s:: Not support (AuthMode=%s, len=%d)\n",
				 __func__, arg, (int) strlen(arg)));
	}

	if (AKMMap != 0x0)
		pSecConfig->AKMMap = AKMMap;

	MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s::AuthMode=0x%x\n",
			 __func__, pSecConfig->AKMMap));
}


VOID SetWdevEncrypMode(
	IN struct _SECURITY_CONFIG *pSecConfig,
	IN RTMP_STRING * arg)
{
	UINT Cipher = 0;

	if (rtstrcasecmp(arg, "NONE") == TRUE)
		SET_CIPHER_NONE(Cipher);
	else if (rtstrcasecmp(arg, "WEP") == TRUE)
		SET_CIPHER_WEP(Cipher);
	else if (rtstrcasecmp(arg, "TKIP") == TRUE)
		SET_CIPHER_TKIP(Cipher);
	else if ((rtstrcasecmp(arg, "AES") == TRUE) || (rtstrcasecmp(arg, "CCMP128") == TRUE))
		SET_CIPHER_CCMP128(Cipher);
	else if (rtstrcasecmp(arg, "CCMP256") == TRUE)
		SET_CIPHER_CCMP256(Cipher);
	else if (rtstrcasecmp(arg, "GCMP128") == TRUE)
		SET_CIPHER_GCMP128(Cipher);
	else if (rtstrcasecmp(arg, "GCMP256") == TRUE)
		SET_CIPHER_GCMP256(Cipher);
	else if ((rtstrcasecmp(arg, "TKIPAES") == TRUE) || (rtstrcasecmp(arg, "TKIPCCMP128") == TRUE)) {
		SET_CIPHER_TKIP(Cipher);
		SET_CIPHER_CCMP128(Cipher);
	} else if ((rtstrcasecmp(arg, "WPA_AES_WPA2_TKIPAES") == TRUE)
			   || (rtstrcasecmp(arg, "WPA_AES_WPA2_TKIP") == TRUE)
			   || (rtstrcasecmp(arg, "WPA_TKIP_WPA2_AES") == TRUE)
			   || (rtstrcasecmp(arg, "WPA_TKIP_WPA2_TKIPAES") == TRUE)
			   || (rtstrcasecmp(arg, "WPA_TKIPAES_WPA2_AES") == TRUE)
			   || (rtstrcasecmp(arg, "WPA_TKIPAES_WPA2_TKIPAES") == TRUE)
			   || (rtstrcasecmp(arg, "WPA_TKIPAES_WPA2_TKIP") == TRUE)) {
		SET_CIPHER_TKIP(Cipher);
		SET_CIPHER_CCMP128(Cipher);
	}

	else {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s:: Not support (EncrypType=%s, len=%d)\n",
				 __func__, arg, (int) strlen(arg)));
	}

	if (Cipher != 0x0) {
		pSecConfig->PairwiseCipher = Cipher;
		CLEAR_GROUP_CIPHER(pSecConfig);
	}

	MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s::PairwiseCipher=0x%x\n",
			 __func__, GET_PAIRWISE_CIPHER(pSecConfig)));
}


INT Set_SecAuthMode_Proc(
	IN RTMP_ADAPTER * pAd,
	IN RTMP_STRING * arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct _SECURITY_CONFIG *pSecConfig = pObj->pSecConfig;
	if (pSecConfig == NULL) {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s:: pSecConfig == NULL, arg=%s\n",
				 __func__, arg));
		return FALSE;
	}

	SetWdevAuthMode(pSecConfig, arg);
	return TRUE;
}

INT Set_SecEncrypType_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING * arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct _SECURITY_CONFIG *pSecConfig = pObj->pSecConfig;

	if (pSecConfig == NULL) {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s:: pSecConfig == NULL, arg=%s\n",
				 __func__, arg));
		return FALSE;
	}

	SetWdevEncrypMode(pSecConfig, arg);
	return TRUE;
}

INT Set_SecDefaultKeyID_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING * arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct _SECURITY_CONFIG *pSecConfig = pObj->pSecConfig;
	ULONG KeyIdx;

	if (pSecConfig == NULL) {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s:: pSecConfig == NULL, arg=%s\n",
				 __func__, arg));
		return FALSE;
	}

	KeyIdx = os_str_tol(arg, 0, 10);

	if ((KeyIdx >= 1) && (KeyIdx <= 4))
		pSecConfig->PairwiseKeyId = (UCHAR) (KeyIdx - 1);
	else
		return FALSE;

	MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s ==> DefaultKeyId=%d\n",
			 __func__, pSecConfig->PairwiseKeyId));
	return TRUE;
}


INT	Set_SecWPAPSK_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING * arg)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct _SECURITY_CONFIG *pSecConfig = pObj->pSecConfig;

	if (pSecConfig == NULL) {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s:: pSecConfig == NULL, arg=%s\n",
				 __func__, arg));
		return FALSE;
	}

	if (strlen(arg) < 65) {
		os_move_mem(pSecConfig->PSK, arg, strlen(arg));
		pSecConfig->PSK[strlen(arg)] = '\0';
	} else
		pSecConfig->PSK[0] = '\0';

	MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: PSK = %s\n",
			 __func__, arg));
#ifdef CONFIG_AP_SUPPORT
#ifdef WSC_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
		WSC_CTRL *pWscControl = NULL;

		if ((pObj->ioctl_if_type == INT_MAIN || pObj->ioctl_if_type == INT_MBSSID)) {
			UCHAR apidx = pObj->ioctl_if;

			pWscControl = &pAd->ApCfg.MBSSID[apidx].wdev.WscControl;
		}

#ifdef APCLI_SUPPORT
		else if (pObj->ioctl_if_type == INT_APCLI) {
			UCHAR    apcli_idx = pObj->ioctl_if;

			pWscControl = &pAd->ApCfg.ApCliTab[apcli_idx].wdev.WscControl;
		}

#endif /* APCLI_SUPPORT */

		if (pWscControl) {
			NdisZeroMemory(pWscControl->WpaPsk, 64);
			pWscControl->WpaPskLen = 0;
			pWscControl->WpaPskLen = strlen(arg);
			NdisMoveMemory(pWscControl->WpaPsk, arg, pWscControl->WpaPskLen);
		}
	}
#endif /* WSC_AP_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
	return TRUE;
}

INT Set_SecWEPKey_Proc(
	IN PRTMP_ADAPTER pAd,
	IN CHAR KeyId,
	IN RTMP_STRING * arg)
{
	INT retVal = FALSE;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct _SECURITY_CONFIG *pSecConfig = pObj->pSecConfig;

	if (pSecConfig == NULL) {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s:: pSecConfig == NULL, arg=%s\n",
				 __func__, arg));
		return FALSE;
	}

	retVal = ParseWebKey(pSecConfig, arg, KeyId, 0);
	MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s::KeyID=%d, key=%s\n",
			 __func__, KeyId, arg));
	return retVal;
}


INT Set_SecKey1_Proc(
	IN PRTMP_ADAPTER pAd,
	IN	RTMP_STRING * arg)
{
	return Set_SecWEPKey_Proc(pAd, 0, arg);
}

INT Set_SecKey2_Proc(
	IN PRTMP_ADAPTER pAd,
	IN	RTMP_STRING * arg)
{
	return Set_SecWEPKey_Proc(pAd, 1, arg);
}

INT Set_SecKey3_Proc(
	IN PRTMP_ADAPTER pAd,
	IN	RTMP_STRING * arg)
{
	return Set_SecWEPKey_Proc(pAd, 2, arg);
}

INT Set_SecKey4_Proc(
	IN PRTMP_ADAPTER pAd,
	IN	RTMP_STRING * arg)
{
	return Set_SecWEPKey_Proc(pAd, 3, arg);
}


RTMP_STRING *GetAuthModeStr(
	IN UINT32 authMode)
{
	if (IS_AKM_OPEN(authMode))
		return "OPEN";
	else if (IS_AKM_SHARED(authMode))
		return "SHARED";
	else if (IS_AKM_AUTOSWITCH(authMode))
		return "WEPAUTO";
	else if (IS_AKM_WPANONE(authMode))
		return "WPANONE";
	else if (IS_AKM_WPA1(authMode) && IS_AKM_WPA2(authMode))
		return "WPA1WPA2";
	else if (IS_AKM_WPA1PSK(authMode) && IS_AKM_WPA2PSK(authMode))
		return "WPAPSKWPA2PSK";
	else if (IS_AKM_WPA2PSK(authMode) && IS_AKM_WPA3PSK(authMode))
		return "WPA2PSKWPA3PSK";
	else if (IS_AKM_WPA3PSK(authMode))
		return "WPA3PSK";
	else if (IS_AKM_WPA1(authMode))
		return "WPA";
	else if (IS_AKM_WPA1PSK(authMode))
		return "WPAPSK";
	else if (IS_AKM_FT_WPA2(authMode))
		return "FT-WPA2";
	else if (IS_AKM_FT_WPA2PSK(authMode))
		return "FT-WPA2PSK";
	else if (IS_AKM_WPA3(authMode)) /* WPA3 will be always accompanied by WPA2, so it should put before the WPA2 */
		return "WPA3";
	else if (IS_AKM_WPA2(authMode))
		return "WPA2";
	else if (IS_AKM_WPA2PSK(authMode))
		return "WPA2PSK";
	else if (IS_AKM_WPA3_192BIT(authMode))
		return "WPA3-192";
	else if (IS_AKM_OWE(authMode))
		return "OWE";
	else
		return "UNKNOW";
}

RTMP_STRING *GetEncryModeStr(
	IN UINT32 encryMode)
{
	if (IS_CIPHER_NONE(encryMode))
		return "NONE";
	else if (IS_CIPHER_WEP(encryMode))
		return "WEP";
	else if (IS_CIPHER_TKIP(encryMode) && IS_CIPHER_CCMP128(encryMode))
		return "TKIPAES";
	else if (IS_CIPHER_TKIP(encryMode))
		return "TKIP";
	else if (IS_CIPHER_CCMP128(encryMode))
		return "AES";
	else if (IS_CIPHER_CCMP256(encryMode))
		return "CCMP256";
	else if (IS_CIPHER_GCMP128(encryMode))
		return "GCMP128";
	else if (IS_CIPHER_GCMP256(encryMode))
		return "GCMP256";
	else if (IS_CIPHER_BIP_CMAC128(encryMode))
		return "BIP-CMAC128";
	else if (IS_CIPHER_BIP_CMAC256(encryMode))
		return "BIP-CMAC256";
	else if (IS_CIPHER_BIP_GMAC128(encryMode))
		return "BIP-GMAC128";
	else if (IS_CIPHER_BIP_GMAC256(encryMode))
		return "BIP-GMAC256";
	else
		return "UNKNOW";
}

UINT32 SecAuthModeOldToNew(
	IN USHORT authMode)
{
	UINT32 AKMMap = 0;

	switch (authMode) {
	case Ndis802_11AuthModeOpen:
		SET_AKM_OPEN(AKMMap);
		break;

	case Ndis802_11AuthModeShared:
		SET_AKM_SHARED(AKMMap);
		break;

	case Ndis802_11AuthModeAutoSwitch:
		SET_AKM_AUTOSWITCH(AKMMap);
		break;

	case Ndis802_11AuthModeWPA:
		SET_AKM_WPA1(AKMMap);
		break;

	case Ndis802_11AuthModeWPAPSK:
		SET_AKM_WPA1PSK(AKMMap);
		break;

	case Ndis802_11AuthModeWPANone:
		SET_AKM_WPANONE(AKMMap);
		break;

	case Ndis802_11AuthModeWPA2:
		SET_AKM_WPA2(AKMMap);
		break;

	case Ndis802_11AuthModeWPA2PSK:
		SET_AKM_WPA2PSK(AKMMap);
		break;

	case Ndis802_11AuthModeWPA1WPA2:
		SET_AKM_WPA1(AKMMap);
		SET_AKM_WPA2(AKMMap);
		break;

	case Ndis802_11AuthModeWPA1PSKWPA2PSK:
		SET_AKM_WPA1PSK(AKMMap);
		SET_AKM_WPA2PSK(AKMMap);
		break;
	}

	return AKMMap;
}


UINT32 SecEncryModeOldToNew(
	IN USHORT encryMode)
{
	UINT32 EncryType = 0;

	switch (encryMode) {
	case Ndis802_11WEPDisabled:
		SET_CIPHER_NONE(EncryType);
		break;

	case Ndis802_11WEPEnabled:
		SET_CIPHER_WEP(EncryType);
		break;

	case Ndis802_11TKIPEnable:
		SET_CIPHER_TKIP(EncryType);
		break;

	case Ndis802_11AESEnable:
		SET_CIPHER_CCMP128(EncryType);
		break;

	case Ndis802_11TKIPAESMix:
		SET_CIPHER_TKIP(EncryType);
		SET_CIPHER_CCMP128(EncryType);
		break;
	}

	return EncryType;
}


USHORT SecAuthModeNewToOld(
	IN UINT32 authMode)
{
	if (IS_AKM_OPEN(authMode))
		return Ndis802_11AuthModeOpen;
	else if (IS_AKM_SHARED(authMode))
		return Ndis802_11AuthModeShared;
	else if (IS_AKM_AUTOSWITCH(authMode))
		return Ndis802_11AuthModeAutoSwitch;
	else if (IS_AKM_WPANONE(authMode))
		return Ndis802_11AuthModeWPANone;
	else if (IS_AKM_WPA1(authMode) && IS_AKM_WPA2(authMode))
		return Ndis802_11AuthModeWPA1WPA2;
	else if (IS_AKM_WPA1PSK(authMode) && IS_AKM_WPA2PSK(authMode))
		return Ndis802_11AuthModeWPA1PSKWPA2PSK;
	else if (IS_AKM_WPA1(authMode))
		return Ndis802_11AuthModeWPA;
	else if (IS_AKM_WPA1PSK(authMode))
		return Ndis802_11AuthModeWPAPSK;
	else if (IS_AKM_WPA2(authMode))
		return Ndis802_11AuthModeWPA2;
	else if (IS_AKM_WPA2PSK(authMode))
		return Ndis802_11AuthModeWPA2PSK;

	else
		return Ndis802_11AuthModeOpen;
}


USHORT SecEncryModeNewToOld(
	IN UINT32 encryMode)
{
	if (IS_CIPHER_NONE(encryMode))
		return Ndis802_11WEPDisabled;
	else if (IS_CIPHER_WEP(encryMode))
		return Ndis802_11WEPEnabled;
	else if (IS_CIPHER_TKIP(encryMode))
		return Ndis802_11TKIPEnable;
	else if (IS_CIPHER_CCMP128(encryMode))
		return Ndis802_11AESEnable;
	else if (IS_CIPHER_TKIP(encryMode) && IS_CIPHER_CCMP128(encryMode))
		return Ndis802_11TKIPAESMix;

	else
		return Ndis802_11WEPDisabled;
}


UINT8 SecHWCipherSuitMapping(
	IN UINT32 encryMode)
{
	if (IS_CIPHER_NONE(encryMode))
		return CIPHER_SUIT_NONE;
	else if (IS_CIPHER_WEP(encryMode))
		return CIPHER_SUIT_WEP_40;
	else if (IS_CIPHER_TKIP(encryMode))
		return CIPHER_SUIT_TKIP_W_MIC;
	else if (IS_CIPHER_CCMP128(encryMode))
		return CIPHER_SUIT_CCMP_W_MIC;
	else if (IS_CIPHER_CCMP256(encryMode))
		return CIPHER_SUIT_CCMP_256;
	else if (IS_CIPHER_GCMP128(encryMode))
		return CIPHER_SUIT_GCMP_128;
	else if (IS_CIPHER_GCMP256(encryMode))
		return CIPHER_SUIT_GCMP_256;

	else
		return CIPHER_SUIT_NONE;
}


INT ParseWebKey(
	IN  struct _SECURITY_CONFIG *pSecConfig,
	IN  RTMP_STRING *buffer,
	IN  INT KeyIdx,
	IN  INT Keylength)
{
	INT KeyLen = Keylength;
	SEC_KEY_INFO *pWebKey = &pSecConfig->WepKey[KeyIdx];
	INT i = 0;

	if (KeyLen == 0)
		KeyLen = strlen(buffer);

	switch (KeyLen) {
	case 5: /*wep 40 Ascii type*/
	case 13: /*wep 104 Ascii type*/
	case 16: /*wep 128 Ascii type*/
		NdisZeroMemory(pWebKey, sizeof(SEC_KEY_INFO));
		pWebKey->KeyLen = KeyLen;
		NdisMoveMemory(pWebKey->Key, buffer, KeyLen);
		break;

	case 10: /*wep 40 Hex type*/
	case 26: /*wep 104 Hex type*/
	case 32: /*wep 128 Hex type*/
		for (i = 0; i < KeyLen; i++) {
			if (!isxdigit(*(buffer + i)))
				return FALSE;  /*Not Hex value;*/
		}

		NdisZeroMemory(pWebKey, sizeof(SEC_KEY_INFO));
		pWebKey->KeyLen = KeyLen / 2;
		AtoH(buffer, pWebKey->Key, pWebKey->KeyLen);
		break;

	default: /*Invalid argument */
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s::(keyIdx=%d):Invalid argument (arg=%s)\n",
				 __func__, KeyIdx, buffer));
		return FALSE;
	}

	MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s::(KeyIdx=%d, Alg=0x%x)\n",
			 __func__, KeyIdx, pSecConfig->PairwiseCipher));
	return TRUE;
}


#ifdef DOT1X_SUPPORT
INT SetWdevOwnIPAddr(
	IN struct _SECURITY_CONFIG *pSecConfig,
	IN RTMP_STRING *arg)
{
	UINT32 ip_addr;

	if (rtinet_aton(arg, &ip_addr)) {
		pSecConfig->own_ip_addr = ip_addr;
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("own_ip_addr=%s(%x)\n", arg, pSecConfig->own_ip_addr));
	}

	return TRUE;
}

VOID ReadRadiusParameterFromFile(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *tmpbuf,
	IN RTMP_STRING *pBuffer)
{
	RTMP_STRING tok_str[16], *macptr;
	UINT32 ip_addr;
	INT i = 0;
	BOOLEAN bUsePrevFormat = FALSE;
	USHORT offset;
	struct wifi_dev *wdev = NULL;
	struct _SECURITY_CONFIG *pSecConfig = NULL;
#ifdef CONFIG_AP_SUPPORT
	INT apidx;
#endif /* CONFIG_AP_SUPPORT */
#ifdef RADIUS_ACCOUNTING_SUPPORT
	BOOLEAN				bAcctUsePrevFormat = FALSE;
	/* INT					acct_count[HW_BEACON_MAX_NUM]; */
#endif /*RADIUS_ACCOUNTING_SUPPORT*/

	/* own_ip_addr*/
	if (RTMPGetKeyParameter("own_ip_addr", tmpbuf, 32, pBuffer, TRUE)) {
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (apidx = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && apidx < pAd->ApCfg.BssidNum); macptr = rstrtok(NULL, ";"), apidx++) {
				wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
				pSecConfig = &wdev->SecConfig;
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("I/F(%s%d) ==> ",
						 INF_MBSSID_DEV_NAME, apidx));
				SetWdevOwnIPAddr(pSecConfig, macptr);
			}

			/* Apply to remaining MBSS*/
			if (apidx >= 1) {
				/*
				* own_ip_addr is global setting , don't need to merge in dbdc multi profile,
				* in this point, let all bss set the same own_ip_addr for safe
				*/
				for (apidx = 1; apidx < pAd->ApCfg.BssidNum; apidx++) {
					pSecConfig = &pAd->ApCfg.MBSSID[apidx].wdev.SecConfig;
					pSecConfig->own_ip_addr = pAd->ApCfg.MBSSID[0].wdev.SecConfig.own_ip_addr;
				}
			}
		}
#endif /* CONFIG_AP_SUPPORT */
	}

	if (RTMPGetKeyParameter("own_radius_port", tmpbuf, 32, pBuffer, TRUE)) {
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (apidx = 0, macptr = rstrtok(tmpbuf, ";");
				(macptr && apidx < pAd->ApCfg.BssidNum);
				macptr = rstrtok(NULL, ";"), apidx++) {

				wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
				pSecConfig = &wdev->SecConfig;
				MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
						("I/F(%s%d) ==> ", INF_MBSSID_DEV_NAME, apidx));

				pSecConfig->own_radius_port = simple_strtol(macptr, 0, 10);
			}

			/* Apply to remaining MBSS*/
			if (apidx >= 1) {
			/*
			*	own_radius_port is global setting , let all bss set the same own_radius_port
			*/
				for (apidx = 1; apidx < pAd->ApCfg.BssidNum; apidx++) {
					pSecConfig = &pAd->ApCfg.MBSSID[apidx].wdev.SecConfig;
					pSecConfig->own_radius_port =
						pAd->ApCfg.MBSSID[0].wdev.SecConfig.own_radius_port;
				}
			}
		}
#endif /* CONFIG_AP_SUPPORT */
	}

	/* session_timeout_interval*/
	if (RTMPGetKeyParameter("session_timeout_interval", tmpbuf, 32, pBuffer, TRUE)) {
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (apidx = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && apidx < pAd->ApCfg.BssidNum); macptr = rstrtok(NULL, ";"), apidx++) {
				wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
				pSecConfig = &wdev->SecConfig;
				pSecConfig->session_timeout_interval = os_str_tol(macptr, 0, 10);
				MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("I/F(%s%d) ==> session_timeout_interval=%d\n",
						 INF_MBSSID_DEV_NAME, apidx, pSecConfig->session_timeout_interval));
			}

			/* Apply to remaining MBSS*/
			if (apidx == 1) {
				for (apidx = 1; apidx < pAd->ApCfg.BssidNum; apidx++) {
					pSecConfig = &pAd->ApCfg.MBSSID[apidx].wdev.SecConfig;
					pSecConfig->session_timeout_interval = pAd->ApCfg.MBSSID[0].wdev.SecConfig.session_timeout_interval;
					MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("I/F(%s%d) ==> session_timeout_interval=%d\n",
							 INF_MBSSID_DEV_NAME, apidx, pSecConfig->session_timeout_interval));
				}
			}
		}
#endif /* CONFIG_AP_SUPPORT */
	}

	/* quiet_interval */
	if (RTMPGetKeyParameter("quiet_interval", tmpbuf, 32, pBuffer, TRUE)) {
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (apidx = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && apidx < pAd->ApCfg.BssidNum); macptr = rstrtok(NULL, ";"), apidx++) {
				wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
				pSecConfig = &wdev->SecConfig;
				pSecConfig->quiet_interval = os_str_tol(macptr, 0, 10);
				MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("I/F(%s%d) ==> quiet_interval=%d\n",
						 INF_MBSSID_DEV_NAME, apidx, pSecConfig->quiet_interval));
			}

			/* Apply to remaining MBSS*/
			if (apidx == 1) {
				for (apidx = 1; apidx < pAd->ApCfg.BssidNum; apidx++) {
					pSecConfig = &pAd->ApCfg.MBSSID[apidx].wdev.SecConfig;
					pSecConfig->quiet_interval = pAd->ApCfg.MBSSID[0].wdev.SecConfig.quiet_interval;
					MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("I/F(%s%d) ==> quiet_interval=%d\n",
							 INF_MBSSID_DEV_NAME, apidx, pSecConfig->quiet_interval));
				}
			}
		}
#endif /* CONFIG_AP_SUPPORT */
	}

	/* EAPifname*/
	if (RTMPGetKeyParameter("EAPifname", tmpbuf, 256, pBuffer, TRUE)) {
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (apidx = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && apidx < pAd->ApCfg.BssidNum); macptr = rstrtok(NULL, ";"), apidx++) {
				wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
				pSecConfig = &wdev->SecConfig;

				if (strlen(macptr) > 0 && strlen(macptr) <= IFNAMSIZ) {
					pSecConfig->EAPifname_len = strlen(macptr);
					NdisMoveMemory(pSecConfig->EAPifname, macptr, strlen(macptr));
					MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("I/F(%s%d) ==> EAPifname=%s, len=%d\n",
							 INF_MBSSID_DEV_NAME, apidx, pSecConfig->EAPifname, pSecConfig->EAPifname_len));
				}
			}
		}
#endif /* CONFIG_AP_SUPPORT */
	}

	/* PreAuthifname*/
	if (RTMPGetKeyParameter("PreAuthifname", tmpbuf, 256, pBuffer, TRUE)) {
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (apidx = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && apidx < pAd->ApCfg.BssidNum); macptr = rstrtok(NULL, ";"), apidx++) {
				wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
				pSecConfig = &wdev->SecConfig;

				if (strlen(macptr) > 0 && strlen(macptr) <= IFNAMSIZ) {
					pSecConfig->PreAuthifname_len = strlen(macptr);
					NdisMoveMemory(pSecConfig->PreAuthifname, macptr, strlen(macptr));
					MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("I/F(%s%d) ==> PreAuthifname=%s, len=%d\n",
							 INF_MBSSID_DEV_NAME, apidx, pSecConfig->PreAuthifname, pSecConfig->PreAuthifname_len));
				}
			}
		}
#endif /* CONFIG_AP_SUPPORT */
	}

	/* PreAuth */
	if (RTMPGetKeyParameter("PreAuth", tmpbuf, 256, pBuffer, TRUE)) {
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (apidx = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && apidx < pAd->ApCfg.BssidNum); macptr = rstrtok(NULL, ";"), apidx++) {
				wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
				pSecConfig = &wdev->SecConfig;

				if (os_str_tol(macptr, 0, 10) != 0)  /*Enable*/
					pSecConfig->PreAuth = TRUE;
				else /*Disable*/
					pSecConfig->PreAuth = FALSE;

				MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("I/F(%s%d) ==> PreAuth=%d\n",
						 INF_MBSSID_DEV_NAME, apidx, pSecConfig->PreAuth));
			}
		}
#endif /* CONFIG_AP_SUPPORT */
	}

	/* IEEE8021X */
	if (RTMPGetKeyParameter("IEEE8021X", tmpbuf, 256, pBuffer, TRUE)) {
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (apidx = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && apidx < pAd->ApCfg.BssidNum); macptr = rstrtok(NULL, ";"), apidx++) {
				wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
				pSecConfig = &wdev->SecConfig;

				if (os_str_tol(macptr, 0, 10) != 0)  /*Enable*/
					pSecConfig->IEEE8021X = TRUE;
				else /*Disable*/
					pSecConfig->IEEE8021X = FALSE;

				MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("IF(%s%d) ==> IEEE8021X=%d\n",
						 INF_MBSSID_DEV_NAME, apidx, pSecConfig->IEEE8021X));
			}
		}
#endif /* CONFIG_AP_SUPPORT */
	}

#ifdef RADIUS_ACCOUNTING_SUPPORT

	/*radius_request_cui
	if(RTMPGetKeyParameter("radius_request_cui", tmpbuf, 32, buffer, TRUE))
	{
		for (i = 0, macptr = rstrtok(tmpbuf,";"); macptr; macptr = rstrtok(NULL,";"), i++)
		{
			if (i >= pAd->ApCfg.BssidNum)
				break;

			if(os_str_tol(macptr, 0, 10) != 0)
				pAd->ApCfg.MBSSID[i].radius_request_cui = TRUE;
			else
				pAd->ApCfg.MBSSID[i].radius_request_cui = FALSE;

			DBGPRINT(RT_DEBUG_ERROR, ("IF(ra%d), radius_request_cui=%d\n", i, pAd->ApCfg.MBSSID[i].radius_request_cui));
		}
	}*/
	/*radius_acct_authentic*/
	if (RTMPGetKeyParameter("radius_acct_authentic", tmpbuf, 32, pBuffer, TRUE)) {
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (apidx = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && apidx < pAd->ApCfg.BssidNum); macptr = rstrtok(NULL, ";"), apidx++) {
				wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
				pSecConfig = &wdev->SecConfig;
				pSecConfig->radius_acct_authentic = os_str_tol(macptr, 0, 10);
				MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("IF(%s%d) ==> radius_acct_authentic=%d\n",
						 INF_MBSSID_DEV_NAME, apidx, pSecConfig->radius_acct_authentic));
			}
		}
	}

	/*acct_interim_interval*/
	if (RTMPGetKeyParameter("acct_interim_interval", tmpbuf, 32, pBuffer, TRUE)) {
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (apidx = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && apidx < pAd->ApCfg.BssidNum); macptr = rstrtok(NULL, ";"), apidx++) {
				wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
				pSecConfig = &wdev->SecConfig;
				pSecConfig->acct_interim_interval = os_str_tol(macptr, 0, 10);
				MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("IF(%s%d) ==> acct_interim_interval=%d\n",
						 INF_MBSSID_DEV_NAME, apidx, pSecConfig->acct_interim_interval));
			}
		}
	}

	/*acct_enable*/
	if (RTMPGetKeyParameter("acct_enable", tmpbuf, 32, pBuffer, TRUE)) {
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (apidx = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && apidx < pAd->ApCfg.BssidNum); macptr = rstrtok(NULL, ";"), apidx++) {
				wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
				pSecConfig = &wdev->SecConfig;
				pSecConfig->acct_enable = os_str_tol(macptr, 0, 10);
				MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("IF(%s%d) ==> acct_enable=%d\n",
						 INF_MBSSID_DEV_NAME, apidx, pSecConfig->acct_enable));
			}
		}
	}

#endif	/* RADIUS_ACCOUNTING_SUPPORT */
	/* RADIUS_Server */
	offset = 0;

	while (RTMPGetKeyParameterWithOffset("RADIUS_Server", tmpbuf, &offset, 256, pBuffer, TRUE)) {
		for (apidx = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && apidx < MAX_MBSSID_NUM(pAd)); macptr = rstrtok(NULL, ";"), apidx++) {
			wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
			pSecConfig = &wdev->SecConfig;

			if (rtinet_aton(macptr, &ip_addr) && (pSecConfig->radius_srv_num < MAX_RADIUS_SRV_NUM)) {
				pSecConfig->radius_srv_info[0].radius_ip = ip_addr;
				MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("IF(%s%d) ==> radius_ip(seq-%d)=%s\n",
						 INF_MBSSID_DEV_NAME, apidx, pSecConfig->radius_srv_num, macptr));
				pSecConfig->radius_srv_num++;
			}
		}
	}

	/* RADIUS_Port */
	offset = 0;

	while (RTMPGetKeyParameterWithOffset("RADIUS_Port", tmpbuf, &offset, 256, pBuffer, TRUE)) {
		for (apidx = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && apidx < MAX_MBSSID_NUM(pAd)); macptr = rstrtok(NULL, ";"), apidx++) {
			wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
			pSecConfig = &wdev->SecConfig;
			pSecConfig->radius_srv_info[0].radius_port = (UINT32) os_str_tol(macptr, 0, 10);
			MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("IF(%s%d) ==> radius_port(seq-%d)=%d\n",
					 INF_MBSSID_DEV_NAME, apidx, 0, pSecConfig->radius_srv_info[0].radius_port));
		}
	}

	/* RADIUS_Key  */
	offset = 0;

	while (RTMPGetKeyParameterWithOffset("RADIUS_Key", tmpbuf, &offset, 256, pBuffer, TRUE)) {
		if (strlen(tmpbuf) > 0)
			bUsePrevFormat = TRUE;

		for (apidx = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && apidx < MAX_MBSSID_NUM(pAd)); macptr = rstrtok(NULL, ";"), apidx++) {
			wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
			pSecConfig = &wdev->SecConfig;

			if (strlen(macptr) > 0) {
				RADIUS_SRV_INFO *p_radius_srv_info = &pSecConfig->radius_srv_info[0];

				p_radius_srv_info->radius_key_len = strlen(macptr) > 64 ? 64 : strlen(macptr);
				NdisMoveMemory(p_radius_srv_info->radius_key, macptr, p_radius_srv_info->radius_key_len);
				MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("IF(%s%d) ==> radius_key(seq-%d)=%s, len=%d\n",
						 INF_MBSSID_DEV_NAME, apidx, 0, macptr, p_radius_srv_info->radius_key_len));
			}
		}
	}

	if (!bUsePrevFormat) {
		for (i = 0; i < MAX_MBSSID_NUM(pAd); i++) {
			snprintf(tok_str, sizeof(tok_str), "RADIUS_Key%d", i + 1);
			offset = 0;
			wdev = &pAd->ApCfg.MBSSID[i].wdev;
			pSecConfig = &wdev->SecConfig;

			while (RTMPGetKeyParameterWithOffset(tok_str, tmpbuf, &offset, 128, pBuffer, FALSE)) {
				if (strlen(tmpbuf) > 0) {
					RADIUS_SRV_INFO *p_radius_srv_info = &pSecConfig->radius_srv_info[0];

					p_radius_srv_info->radius_key_len = strlen(tmpbuf) > 64 ? 64 : strlen(tmpbuf);
					NdisMoveMemory(p_radius_srv_info->radius_key, tmpbuf, p_radius_srv_info->radius_key_len);
					MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("IF(%s%d) ==> radius_key(seq-%d)=%s, len=%d\n",
							 INF_MBSSID_DEV_NAME, i, 0, p_radius_srv_info->radius_key, p_radius_srv_info->radius_key_len));
				}
			}
		}
	}

	/* NasIdX, X indicate the interface index(1~8) */
	for (i = 0; i < MAX_MBSSID_NUM(pAd); i++) {
		wdev = &pAd->ApCfg.MBSSID[i].wdev;
		pSecConfig = &wdev->SecConfig;
		snprintf(tok_str, sizeof(tok_str), "NasId%d", i + 1);

		if (RTMPGetKeyParameter(tok_str, tmpbuf, 33, pBuffer, FALSE)) {
			if (strlen(tmpbuf) > 0) {
				pSecConfig->NasIdLen = strlen(tmpbuf) > IFNAMSIZ ? IFNAMSIZ : strlen(tmpbuf);
				NdisMoveMemory(pSecConfig->NasId, tmpbuf, pSecConfig->NasIdLen);
				MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("IF(%s%d) ==> NAS-ID=%s, len=%d\n",
						 INF_MBSSID_DEV_NAME, i, pSecConfig->NasId, pSecConfig->NasIdLen));
			}
		}
	}

#ifdef RADIUS_ACCOUNTING_SUPPORT
	/* RADIUS_Acct_Server*/
	offset = 0;

	while (RTMPGetKeyParameterWithOffset("RADIUS_Acct_Server", tmpbuf, &offset, 256, pBuffer, TRUE)) {
		for (apidx = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && apidx < MAX_MBSSID_NUM(pAd)); macptr = rstrtok(NULL, ";"), apidx++) {
			wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
			pSecConfig = &wdev->SecConfig;

			if (rtinet_aton(macptr, &ip_addr) && (pSecConfig->radius_acct_srv_num < MAX_RADIUS_SRV_NUM)) {
				pSecConfig->radius_acct_srv_info[pSecConfig->radius_acct_srv_num].radius_ip = ip_addr;
				MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("IF(%s%d) ==> radius_acct_ip(seq-%d)=%s\n",
						 INF_MBSSID_DEV_NAME, apidx, pSecConfig->radius_acct_srv_num, macptr));
				pSecConfig->radius_acct_srv_num++;
			}
		}
	}

	/* RADIUS_Acct_Port*/
	offset = 0;

	while (RTMPGetKeyParameterWithOffset("RADIUS_Acct_Port", tmpbuf, &offset, 256, pBuffer, TRUE)) {
		for (apidx = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && apidx < MAX_MBSSID_NUM(pAd)); macptr = rstrtok(NULL, ";"), apidx++) {
			wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
			pSecConfig = &wdev->SecConfig;
			pSecConfig->radius_acct_srv_info[0].radius_port = (UINT32) os_str_tol(macptr, 0, 10);	/* TODO: idx */
			MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("IF(%s%d) ==> radius_acct_port(seq-%d)=%d\n",
					 INF_MBSSID_DEV_NAME, apidx, 0, pSecConfig->radius_acct_srv_info[0].radius_port));
		}
	}

	/* RADIUS_Key*/
	offset = 0;

	while (RTMPGetKeyParameterWithOffset("RADIUS_Acct_Key", tmpbuf, &offset, 256, pBuffer, TRUE)) {
		if (strlen(tmpbuf) > 0)
			bAcctUsePrevFormat = TRUE;

		for (apidx = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && apidx < MAX_MBSSID_NUM(pAd)); macptr = rstrtok(NULL, ";"), apidx++) {
			wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
			pSecConfig = &wdev->SecConfig;

			if (strlen(macptr) > 0) {
				RADIUS_SRV_INFO *p_radius_srv_info = &pSecConfig->radius_acct_srv_info[0];	/* TODO: idx */

				p_radius_srv_info->radius_key_len = strlen(macptr) > 64 ? 64 : strlen(macptr);
				NdisMoveMemory(p_radius_srv_info->radius_key, macptr, p_radius_srv_info->radius_key_len);
				MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("IF(%s%d) ==> radius_acct_key(seq-%d)=%s, len=%d\n",
						 INF_MBSSID_DEV_NAME, apidx, 0, macptr, p_radius_srv_info->radius_key_len));
			}
		}
	}

	if (!bAcctUsePrevFormat) {
		for (i = 0; i < MAX_MBSSID_NUM(pAd); i++) {
			snprintf(tok_str, sizeof(tok_str), "RADIUS_Acct_Key%d", i + 1);
			offset = 0;
			wdev = &pAd->ApCfg.MBSSID[i].wdev;
			pSecConfig = &wdev->SecConfig;

			while (RTMPGetKeyParameterWithOffset(tok_str, tmpbuf, &offset, 128, pBuffer, FALSE)) {
				if (strlen(tmpbuf) > 0) {
					RADIUS_SRV_INFO *p_radius_srv_info = &pSecConfig->radius_acct_srv_info[0];		/* TODO: idx */

					p_radius_srv_info->radius_key_len = strlen(tmpbuf) > 64 ? 64 : strlen(tmpbuf);
					NdisMoveMemory(p_radius_srv_info->radius_key, tmpbuf, p_radius_srv_info->radius_key_len);
					MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("IF(%s%d) ==> radius_acct_key(seq-%d)=%s, len=%d\n",
							 INF_MBSSID_DEV_NAME, i, 0, p_radius_srv_info->radius_key, p_radius_srv_info->radius_key_len));
				}
			}
		}
	}

#endif	/* RADIUS_ACCOUNTING_SUPPORT */
}


#ifdef CONFIG_AP_SUPPORT
VOID Dot1xIoctlQueryRadiusConf(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_IOCTL_INPUT_STRUCT * wrq)
{
	UCHAR apidx, srv_idx, keyidx, KeyLen = 0;
	UCHAR *mpool;
	PDOT1X_CMM_CONF pConf;
	struct _SECURITY_CONFIG *pSecConfigMain = NULL;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR main_apidx = (UCHAR) pObj->ioctl_if;
	UCHAR last_apidx = pAd->ApCfg.BssidNum - 1;

	MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s==>\n", __func__));

#ifdef MULTI_PROFILE
	if ((main_apidx == BSS0)
		&& (is_multi_profile_enable(pAd) == TRUE))
		last_apidx = multi_profile_get_pf1_num(pAd) - 1;
#endif
	if ((main_apidx > pAd->ApCfg.BssidNum - 1)
		|| (main_apidx > last_apidx)) {
	    MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():Invalid MBSSID index(%d)!\n",
		    __func__, main_apidx));
	    return;
	}


	pSecConfigMain = &pAd->ApCfg.MBSSID[main_apidx].wdev.SecConfig;

	/* Allocate memory */
	os_alloc_mem(NULL, (PUCHAR *)&mpool, sizeof(DOT1X_CMM_CONF));

	if (mpool == NULL) {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("!!!%s: out of resource!!!\n", __func__));
		return;
	}

	NdisZeroMemory(mpool, sizeof(DOT1X_CMM_CONF));
	pConf = (PDOT1X_CMM_CONF)mpool;
	/* get MBSS number */
	pConf->mbss_num = (last_apidx - main_apidx + 1);
	/* get own ip address */
	pConf->own_ip_addr = pSecConfigMain->own_ip_addr;
	/* get own radius port */
	pConf->own_radius_port = pSecConfigMain->own_radius_port;
	/* get retry interval */
	pConf->retry_interval = pSecConfigMain->retry_interval;
	/* get session timeout interval */
	pConf->session_timeout_interval = pSecConfigMain->session_timeout_interval;
	/* Get the quiet interval */
	pConf->quiet_interval = pSecConfigMain->quiet_interval;

	for (apidx = main_apidx; apidx <= last_apidx; apidx++) {
		struct _SECURITY_CONFIG *pSecConfig = &pAd->ApCfg.MBSSID[apidx].wdev.SecConfig;
		UCHAR apidx_locate = apidx - main_apidx;
		PDOT1X_BSS_INFO p1xBssInfo = &pConf->Dot1xBssInfo[apidx_locate];
#ifdef RADIUS_ACCOUNTING_SUPPORT
		PACCT_BSS_INFO pAcctBssInfo = &pConf->AcctBssInfo[apidx_locate];

		pAcctBssInfo->radius_srv_num = pSecConfig->radius_acct_srv_num;
#endif /* RADIUS_ACCOUNTING_SUPPORT */
		p1xBssInfo->radius_srv_num = pSecConfig->radius_srv_num;

		/* prepare radius ip, port and key */
		for (srv_idx = 0; srv_idx < pSecConfig->radius_srv_num; srv_idx++) {
			if (pSecConfig->radius_srv_info[srv_idx].radius_ip != 0) {
				p1xBssInfo->radius_srv_info[srv_idx].radius_ip = pSecConfig->radius_srv_info[srv_idx].radius_ip;
				p1xBssInfo->radius_srv_info[srv_idx].radius_port = pSecConfig->radius_srv_info[srv_idx].radius_port;
				p1xBssInfo->radius_srv_info[srv_idx].radius_key_len = pSecConfig->radius_srv_info[srv_idx].radius_key_len;

				if (pSecConfig->radius_srv_info[srv_idx].radius_key_len > 0) {
					NdisMoveMemory(p1xBssInfo->radius_srv_info[srv_idx].radius_key,
								   pSecConfig->radius_srv_info[srv_idx].radius_key,
								   pSecConfig->radius_srv_info[srv_idx].radius_key_len);
				}
			}
		}

#ifdef RADIUS_ACCOUNTING_SUPPORT

		/* prepare accounting radius ip, port and key */
		for (srv_idx = 0; srv_idx < pSecConfig->radius_acct_srv_num; srv_idx++) {
			if (pSecConfig->radius_acct_srv_info[srv_idx].radius_ip != 0) {
				pAcctBssInfo->radius_srv_info[srv_idx].radius_ip = pSecConfig->radius_acct_srv_info[srv_idx].radius_ip;
				pAcctBssInfo->radius_srv_info[srv_idx].radius_port = pSecConfig->radius_acct_srv_info[srv_idx].radius_port;
				pAcctBssInfo->radius_srv_info[srv_idx].radius_key_len = pSecConfig->radius_acct_srv_info[srv_idx].radius_key_len;

				if (pSecConfig->radius_acct_srv_info[srv_idx].radius_key_len > 0) {
					NdisMoveMemory(pAcctBssInfo->radius_srv_info[srv_idx].radius_key,
								   pSecConfig->radius_acct_srv_info[srv_idx].radius_key,
								   pSecConfig->radius_acct_srv_info[srv_idx].radius_key_len);
				}
			}
		}

#endif /* RADIUS_ACCOUNTING_SUPPORT */
		p1xBssInfo->ieee8021xWEP = (pSecConfig->IEEE8021X) ? 1 : 0;

		if (p1xBssInfo->ieee8021xWEP) {
			/* Default Key index, length and material */
			keyidx = pSecConfig->PairwiseKeyId;
			p1xBssInfo->key_index = keyidx;
			/* Determine if the key is valid. */
			KeyLen = pSecConfig->WepKey[keyidx].KeyLen;

			if (KeyLen == 5 || KeyLen == 13) {
				p1xBssInfo->key_length = KeyLen;
				NdisMoveMemory(p1xBssInfo->key_material, pSecConfig->WepKey[keyidx].Key, KeyLen);
			}
		}

		/* Get NAS-ID per BSS */
		if (pSecConfig->NasIdLen > 0) {
			p1xBssInfo->nasId_len = pSecConfig->NasIdLen;
			NdisMoveMemory(p1xBssInfo->nasId, pSecConfig->NasId, pSecConfig->NasIdLen);
		}

		/* get EAPifname */
		if (pSecConfig->EAPifname_len > 0) {
			pConf->EAPifname_len[apidx_locate] = pSecConfig->EAPifname_len;
			NdisMoveMemory(pConf->EAPifname[apidx_locate], pSecConfig->EAPifname, pSecConfig->EAPifname_len);
		}

		/* get PreAuthifname */
		if (pSecConfig->PreAuthifname_len > 0) {
			pConf->PreAuthifname_len[apidx_locate] = pSecConfig->PreAuthifname_len;
			NdisMoveMemory(pConf->PreAuthifname[apidx_locate], pSecConfig->PreAuthifname, pSecConfig->PreAuthifname_len);
		}

#ifdef RADIUS_ACCOUNTING_SUPPORT
		/* pAcctBssInfo->radius_request_cui = (pSecConfig->radius_request_cui) ? 1 : 0; */
		pAcctBssInfo->radius_acct_authentic = pSecConfig->radius_acct_authentic;
		pAcctBssInfo->acct_interim_interval = pSecConfig->acct_interim_interval;
		pAcctBssInfo->acct_enable = pSecConfig->acct_enable;
#endif /* RADIUS_ACCOUNTING_SUPPORT */
#ifdef RADIUS_MAC_ACL_SUPPORT
		/* Radius MAC Auth Config */
		pConf->RadiusAclEnable[apidx_locate] = pSecConfig->RadiusMacAuthCache.Policy;
		/* Radius MAC Auth Cache Timeout in 1XDaemon */
		pConf->AclCacheTimeout[apidx_locate] = pSecConfig->RadiusMacAuthCacheTimeout;
#endif /* RADIUS_MAC_ACL_SUPPORT */
	}

	wrq->u.data.length = sizeof(DOT1X_CMM_CONF);

	if (copy_to_user(wrq->u.data.pointer, pConf, wrq->u.data.length))
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: copy_to_user() fail\n", __func__));

	os_free_mem(mpool);
}

VOID Dot1xIoctlRadiusData(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_IOCTL_INPUT_STRUCT * wrq)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	struct _SECURITY_CONFIG *pSecConfig = NULL;

	MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, IF(ra%d)\n", __func__, pObj->ioctl_if));

	if (pObj->ioctl_if > pAd->ApCfg.BssidNum) {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():Invalid MBSSID index(%d)!\n",
				 __func__, pObj->ioctl_if));
		return;
	}

	pSecConfig = &pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev.SecConfig;

	if (IS_AKM_1X(pSecConfig->AKMMap)
		|| (pSecConfig->IEEE8021X == TRUE))
		WpaSend(pAd, (PUCHAR)wrq->u.data.pointer, wrq->u.data.length);
}


/*
    ==========================================================================
    Description:
		UI should not call this function, it only used by 802.1x daemon
	Arguments:
	    pAd		Pointer to our adapter
	    wrq		Pointer to the ioctl argument
    ==========================================================================
*/
VOID Dot1xIoctlAddWPAKey(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_IOCTL_INPUT_STRUCT * wrq)
{
	NDIS_AP_802_11_KEY	*pKey;
	ULONG				KeyIdx;
	MAC_TABLE_ENTRY		*pEntry;
	UCHAR				apidx;
	struct _SECURITY_CONFIG *pSecConfig = NULL;
	struct wifi_dev *wdev = NULL;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;

	apidx =	(UCHAR) pObj->ioctl_if;
	MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s-IF(ra%d)\n", __func__, apidx));
	pKey = (PNDIS_AP_802_11_KEY) wrq->u.data.pointer;
	pSecConfig = &pAd->ApCfg.MBSSID[apidx].wdev.SecConfig;
	wdev = &pAd->ApCfg.MBSSID[apidx].wdev;

	if (IS_AKM_1X(pSecConfig->AKMMap)) {
		UCHAR key_len = LEN_PMK;

		if ((pKey->KeyLength == 32) || (pKey->KeyLength == 64)) {
			pEntry = MacTableLookup(pAd, pKey->addr);

			if (pEntry != NULL) {
				INT k_offset = 0;
#ifdef DOT11R_FT_SUPPORT
				/* The key shall be the second 256 bits of the MSK. */
				if (IS_FT_RSN_STA(pEntry) && pKey->KeyLength == 64)
					k_offset = 32;

#endif /* DOT11R_FT_SUPPORT */
				if (IS_AKM_WPA3_192BIT(pSecConfig->AKMMap) && (pKey->KeyLength == 64))
					key_len = LEN_PMK_SHA384;

				NdisMoveMemory(pSecConfig->PMK, pKey->KeyMaterial + k_offset, key_len);
				hex_dump("PMK", pSecConfig->PMK, key_len);
			}
		}
	} else {	/* Old WEP stuff */
		ASIC_SEC_INFO Info = {0};

		if (pKey->KeyLength > 16)
			return;

		KeyIdx = pKey->KeyIndex & 0x0fffffff;

		if (KeyIdx < 4) {
			/* For Group key setting */
			if (pKey->KeyIndex & 0x80000000) {
				UINT8 Wcid;
				/* Default key for tx (shared key) */
				pSecConfig->GroupKeyId = (UCHAR) KeyIdx;

				/* set key material and key length */
				if (pKey->KeyLength > 16) {
					MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s-IF(ra%d) : Key length too long %d\n", __func__, apidx, pKey->KeyLength));
					pKey->KeyLength = 16;
				}

				pSecConfig->WepKey[KeyIdx].KeyLen = (UCHAR) pKey->KeyLength;
				NdisMoveMemory(pSecConfig->WepKey[KeyIdx].Key, &pKey->KeyMaterial, pKey->KeyLength);

				/* Set Ciper type */
				if (pKey->KeyLength == 5)
					SET_CIPHER_WEP40(pSecConfig->GroupCipher);
				else
					SET_CIPHER_WEP104(pSecConfig->GroupCipher);

				/* Get a specific WCID to record this MBSS key attribute */
				GET_GroupKey_WCID(wdev, Wcid);
				/* Set key material to Asic */
				os_zero_mem(&Info, sizeof(ASIC_SEC_INFO));
				Info.Operation = SEC_ASIC_ADD_GROUP_KEY;
				Info.Direction = SEC_ASIC_KEY_TX;
				Info.Wcid = Wcid;
				Info.BssIndex = apidx;
				Info.Cipher = pSecConfig->GroupCipher;
				Info.KeyIdx = pSecConfig->GroupKeyId;
				os_move_mem(&Info.PeerAddr[0], BROADCAST_ADDR, MAC_ADDR_LEN);
				os_move_mem(&Info.Key, &pSecConfig->WepKey[Info.KeyIdx], sizeof(SEC_KEY_INFO));
				HW_ADDREMOVE_KEYTABLE(pAd, &Info);
			} else { /* For Pairwise key setting */
				STA_TR_ENTRY *tr_entry = NULL;

				pEntry = MacTableLookup(pAd, pKey->addr);

				if (pEntry) {
					pSecConfig = &pEntry->SecConfig;
					pSecConfig->PairwiseKeyId = (UCHAR) KeyIdx;
					/* set key material and key length */
					pSecConfig->WepKey[KeyIdx].KeyLen = (UCHAR) pKey->KeyLength;
					NdisMoveMemory(pSecConfig->WepKey[KeyIdx].Key, &pKey->KeyMaterial, pKey->KeyLength);

					/* Set Ciper type */
					if (pKey->KeyLength == 5)
						SET_CIPHER_WEP40(pSecConfig->PairwiseCipher);
					else
						SET_CIPHER_WEP104(pSecConfig->PairwiseCipher);

					/* Set key material to Asic */
					os_zero_mem(&Info, sizeof(ASIC_SEC_INFO));
					Info.Operation = SEC_ASIC_ADD_PAIRWISE_KEY;
					Info.Direction = SEC_ASIC_KEY_BOTH;
					Info.Wcid = pEntry->wcid;
					Info.BssIndex = pEntry->func_tb_idx;
					Info.Cipher = pSecConfig->PairwiseCipher;
					Info.KeyIdx = pSecConfig->PairwiseKeyId;
					os_move_mem(&Info.PeerAddr[0], pEntry->Addr, MAC_ADDR_LEN);
					os_move_mem(&Info.Key, &pSecConfig->WepKey[Info.KeyIdx], sizeof(SEC_KEY_INFO));
					/* HW_ADDREMOVE_KEYTABLE(pAd, &Info); */
					/* open 802.1x port control and privacy filter */
					tr_entry = &pAd->MacTab.tr_entry[pEntry->wcid];
					tr_entry->PortSecured = WPA_802_1X_PORT_SECURED;
					pEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;
					WifiSysUpdatePortSecur(pAd, pEntry, &Info);
				}
			}
		}
	}
}


/*
    ==========================================================================
    Description:
		UI should not call this function, it only used by 802.1x daemon
	Arguments:
	    pAd		Pointer to our adapter
	    wrq		Pointer to the ioctl argument
    ==========================================================================
*/
VOID Dot1xIoctlStaticWepCopy(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_IOCTL_INPUT_STRUCT * wrq)
{
	MAC_TABLE_ENTRY  *pEntry;
	UCHAR MacAddr[MAC_ADDR_LEN];
	UCHAR apidx;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;

	apidx =	(UCHAR) pObj->ioctl_if;
	MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RTMPIoctlStaticWepCopy-IF(ra%d)\n", apidx));

	if (wrq->u.data.length != sizeof(MacAddr)) {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("RTMPIoctlStaticWepCopy: the length isn't match (%d)\n", wrq->u.data.length));
		return;
	} else {
		UINT32 len;

		len = copy_from_user(&MacAddr, wrq->u.data.pointer, wrq->u.data.length);
		pEntry = MacTableLookup(pAd, MacAddr);

		if (!pEntry) {
			MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("RTMPIoctlStaticWepCopy: the mac address isn't match\n"));
			return;
		} else {
			struct _SECURITY_CONFIG *pSecConfigEnrty = NULL;
			struct _SECURITY_CONFIG *pSecConfigProfile = NULL;
			STA_TR_ENTRY *tr_entry = NULL;
			ASIC_SEC_INFO Info = {0};

			pSecConfigProfile = &pAd->ApCfg.MBSSID[apidx].wdev.SecConfig;
			pSecConfigEnrty = &pEntry->SecConfig;
			pSecConfigEnrty->PairwiseKeyId = pSecConfigProfile->PairwiseKeyId;
			pSecConfigEnrty->PairwiseCipher = pSecConfigProfile->PairwiseCipher;
			os_move_mem(&pSecConfigEnrty->WepKey, &pSecConfigProfile->WepKey, sizeof(SEC_KEY_INFO) * SEC_KEY_NUM);
			/* Set key material to Asic */
			os_zero_mem(&Info, sizeof(ASIC_SEC_INFO));
			Info.Operation = SEC_ASIC_ADD_PAIRWISE_KEY;
			Info.Direction = SEC_ASIC_KEY_BOTH;
			Info.Wcid = pEntry->wcid;
			Info.BssIndex = pEntry->func_tb_idx;
			Info.Cipher = pEntry->SecConfig.PairwiseCipher;
			Info.KeyIdx = pEntry->SecConfig.PairwiseKeyId;
			os_move_mem(&Info.Key, &pEntry->SecConfig.WepKey[pEntry->SecConfig.PairwiseKeyId], sizeof(SEC_KEY_INFO));
			os_move_mem(&Info.PeerAddr[0], pEntry->Addr, MAC_ADDR_LEN);
			/* HW_ADDREMOVE_KEYTABLE(pAd, &Info); */
			/* open 802.1x port control and privacy filter */
			tr_entry = &pAd->MacTab.tr_entry[pEntry->wcid];
			tr_entry->PortSecured = WPA_802_1X_PORT_SECURED;
			pEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;
			WifiSysUpdatePortSecur(pAd, pEntry, &Info);
		}
	}

	return;
}
#endif /* CONFIG_AP_SUPPORT */
#endif /* DOT1X_SUPPORT */


#ifdef APCLI_SUPPORT
VOID ReadApcliSecParameterFromFile(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *tmpbuf,
	IN RTMP_STRING *pBuffer)
{
	RTMP_STRING *macptr;
	INT i, idx;
#ifdef DBDC_MODE
	INT apcli_idx;
	RTMP_STRING tok_str[16];
#endif
	struct _SECURITY_CONFIG *pSecConfig = NULL;

	/*ApCliAuthMode*/
	if (RTMPGetKeyParameter("ApCliAuthMode", tmpbuf, 255, pBuffer, TRUE)) {
		RTMP_STRING *orig_tmpbuf;

		orig_tmpbuf = tmpbuf;
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < MAX_APCLI_NUM); macptr = rstrtok(NULL, ";"), i++) {
			if ((i == 0) && (macptr != orig_tmpbuf))
				i = 1;
			pSecConfig = &pAd->ApCfg.ApCliTab[i].wdev.SecConfig;
			MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("I/F(apcli%d) ==> ", i));
			SetWdevAuthMode(pSecConfig, macptr);
		}
	}

	/*ApCliEncrypType*/
	if (RTMPGetKeyParameter("ApCliEncrypType", tmpbuf, 255, pBuffer, TRUE)) {
		RTMP_STRING *orig_tmpbuf;

		orig_tmpbuf = tmpbuf;
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < MAX_APCLI_NUM); macptr = rstrtok(NULL, ";"), i++) {
			if ((i == 0) && (macptr != orig_tmpbuf))
				i = 1;
			pSecConfig = &pAd->ApCfg.ApCliTab[i].wdev.SecConfig;
			MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("I/F(apcli%d) ==> ", i));
			SetWdevEncrypMode(pSecConfig, macptr);
		}
	}

#ifdef DBDC_MODE

	for (apcli_idx = 0; apcli_idx < MAX_APCLI_NUM; apcli_idx++) {
		pSecConfig = &pAd->ApCfg.ApCliTab[apcli_idx].wdev.SecConfig;

		if (apcli_idx == 0)
			snprintf(tok_str, sizeof(tok_str), "ApCliWPAPSK");
		else
			snprintf(tok_str, sizeof(tok_str), "ApCliWPAPSK%d", apcli_idx);

		if (RTMPGetKeyParameter(tok_str, tmpbuf, 65, pBuffer, FALSE)) {
			if (strlen(tmpbuf) < 65) {
				os_move_mem(pSecConfig->PSK, tmpbuf, strlen(tmpbuf));
				pSecConfig->PSK[strlen(tmpbuf)] = '\0';
			} else
				pSecConfig->PSK[0] = '\0';
		}
	}

#else

	/*ApCliWPAPSK*/
	if (RTMPGetKeyParameter("ApCliWPAPSK", tmpbuf, 255, pBuffer, FALSE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < MAX_APCLI_NUM); macptr = rstrtok(NULL, ";"), i++) {
			pSecConfig = &pAd->ApCfg.ApCliTab[i].wdev.SecConfig;

			if (strlen(macptr) < 65) {
				os_move_mem(pSecConfig->PSK, macptr, strlen(macptr));
				pSecConfig->PSK[strlen(macptr)] = '\0';
			} else
				pSecConfig->PSK[0] = '\0';
		}
	}

#endif

	/*ApCliDefaultKeyID*/
	if (RTMPGetKeyParameter("ApCliDefaultKeyID", tmpbuf, 255, pBuffer, TRUE)) {
		ULONG KeyIdx = 0;
		RTMP_STRING *orig_tmpbuf;

		orig_tmpbuf = tmpbuf;
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < MAX_APCLI_NUM); macptr = rstrtok(NULL, ";"), i++) {
			if ((i == 0) && (macptr != orig_tmpbuf))
				i = 1;
			pSecConfig = &pAd->ApCfg.ApCliTab[i].wdev.SecConfig;
			KeyIdx = os_str_tol(macptr, 0, 10);

			if ((KeyIdx >= 1) && (KeyIdx <= 4))
				pSecConfig->PairwiseKeyId = (UCHAR) (KeyIdx - 1);
			else
				pSecConfig->PairwiseKeyId = 0;

			MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("I/F(apcli%d)) ==> DefaultKeyId=%d\n",
					 i, pSecConfig->PairwiseKeyId));
		}
	}

	/*ApCliKeyXType, ApCliKeyXStr*/
	for (idx = 0; idx < 4; idx++) {
		RTMP_STRING tok_str[16];
		ULONG KeyType[MAX_APCLI_NUM];

		snprintf(tok_str, sizeof(tok_str),	"ApCliKey%dType", idx + 1);

		/*ApCliKey1Type*/
		if (RTMPGetKeyParameter(tok_str, tmpbuf, 128, pBuffer, TRUE)) {
			for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < MAX_APCLI_NUM); macptr = rstrtok(NULL, ";"), i++)
				KeyType[i] = os_str_tol(macptr, 0, 10);

#ifdef DBDC_MODE
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
				for (i = 0; i < MAX_APCLI_NUM; i++) {
					if (i == 0)
						snprintf(tok_str, sizeof(tok_str), "ApCliKey%dStr", idx + 1);
					else
						snprintf(tok_str, sizeof(tok_str), "ApCliKey%dStr%d", idx + 1, i);

					if (RTMPGetKeyParameter(tok_str, tmpbuf, 128, pBuffer, FALSE)) {
						pSecConfig = &pAd->ApCfg.ApCliTab[i].wdev.SecConfig;
						ParseWebKey(pSecConfig, tmpbuf, idx, 0);
					}
				}
			}
#else
			snprintf(tok_str, sizeof(tok_str), "ApCliKey%dStr", idx + 1);

			/*ApCliKey1Str*/
			if (RTMPGetKeyParameter(tok_str, tmpbuf, 512, pBuffer, FALSE)) {
				for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < MAX_APCLI_NUM); macptr = rstrtok(NULL, ";"), i++) {
					pSecConfig = &pAd->ApCfg.ApCliTab[i].wdev.SecConfig;
					ParseWebKey(pSecConfig, macptr, idx, 0);
				}
			}

#endif
		}
	}
}
#endif /* APCLI_SUPPORT */


#ifdef WDS_SUPPORT
VOID ReadWDSSecParameterFromFile(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *tmpbuf,
	IN RTMP_STRING *pBuffer)
{
	RTMP_STRING *macptr;
	INT i, idx;
	BOOLEAN	bUsePrevFormat = FALSE;
	struct _SECURITY_CONFIG *pSecConfig = NULL;

	/* WDS direct insert Key to Asic, not need do 4-way */
	/* WdsEncrypType */
	if (RTMPGetKeyParameter("WdsEncrypType", tmpbuf, 255, pBuffer, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < MAX_WDS_ENTRY); macptr = rstrtok(NULL, ";"), i++) {
			pSecConfig = &pAd->WdsTab.WdsEntry[i].wdev.SecConfig;
			MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("I/F(wds%d) ==> ", i));
			SetWdevEncrypMode(pSecConfig, macptr);
		}
	}

	/*WdsKey*/
	if (RTMPGetKeyParameter("WdsKey", tmpbuf, 255, pBuffer, FALSE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < MAX_WDS_ENTRY); macptr = rstrtok(NULL, ";"), i++) {
			pSecConfig = &pAd->WdsTab.WdsEntry[i].wdev.SecConfig;

			if ((strlen(macptr) > 0) && (strlen(macptr) < 65)) {
				os_move_mem(pSecConfig->PSK, macptr, strlen(macptr));
				pSecConfig->PSK[strlen(macptr)] = '\0';
				bUsePrevFormat = TRUE;
			} else
				pSecConfig->PSK[0] = '\0';
		}
	}

	/*WdsDefaultKeyID*/
	if (RTMPGetKeyParameter("WdsDefaultKeyID", tmpbuf, 255, pBuffer, TRUE)) {
		ULONG KeyIdx = 0;

		for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < MAX_APCLI_NUM); macptr = rstrtok(NULL, ";"), i++) {
			pSecConfig = &pAd->WdsTab.WdsEntry[i].wdev.SecConfig;
			KeyIdx = os_str_tol(macptr, 0, 10);

			if ((KeyIdx >= 1) && (KeyIdx <= 4))
				pSecConfig->PairwiseKeyId = (UCHAR) (KeyIdx - 1);
			else
				pSecConfig->PairwiseKeyId = 0;

			MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("I/F(wds%d)) ==> DefaultKeyId=%d\n",
					 i, pSecConfig->PairwiseKeyId));
		}
	}

	/*WdsXKey */
	if (bUsePrevFormat == FALSE) {
		for (idx = 0; idx < MAX_WDS_ENTRY; idx++) {
			RTMP_STRING tok_str[16];

			snprintf(tok_str, sizeof(tok_str),	"Wds%dKey", idx);
			pSecConfig = &pAd->WdsTab.WdsEntry[idx].wdev.SecConfig;

			if (RTMPGetKeyParameter(tok_str, tmpbuf, 128, pBuffer, FALSE)) {
				if (IS_CIPHER_WEP(pSecConfig->PairwiseCipher))
					ParseWebKey(pSecConfig, tmpbuf, pSecConfig->PairwiseKeyId, 0);
				else if (IS_CIPHER_TKIP(pSecConfig->PairwiseCipher)
						 || IS_CIPHER_CCMP128(pSecConfig->PairwiseCipher)
						 || IS_CIPHER_CCMP256(pSecConfig->PairwiseCipher)
						 || IS_CIPHER_GCMP128(pSecConfig->PairwiseCipher)
						 || IS_CIPHER_GCMP256(pSecConfig->PairwiseCipher)) {
					if (strlen(tmpbuf) < 65) {
						os_move_mem(pSecConfig->PSK, tmpbuf, strlen(tmpbuf));
						pSecConfig->PSK[strlen(tmpbuf)] = '\0';
					} else
						pSecConfig->PSK[0] = '\0';
				}
			}
		}
	}

}
#endif /* WDS_SUPPORT */


VOID ReadSecurityParameterFromFile(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *tmpbuf,
	IN RTMP_STRING *pBuffer)
{
	RTMP_STRING *macptr;
#ifdef CONFIG_AP_SUPPORT
	INT apidx;
#endif /* CONFIG_AP_SUPPORT */
	INT idx;
	struct wifi_dev *wdev = NULL;
	struct _SECURITY_CONFIG *pSecConfig = NULL;

	/*AuthMode*/
	if (RTMPGetKeyParameter("AuthMode", tmpbuf, MAX_PARAMETER_LEN, pBuffer, TRUE)) {
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (apidx = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && apidx < pAd->ApCfg.BssidNum); macptr = rstrtok(NULL, ";"), apidx++) {
				wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
				pSecConfig = &wdev->SecConfig;
				MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("I/F(%s%d) ==> ",
						 INF_MBSSID_DEV_NAME, apidx));
				SetWdevAuthMode(pSecConfig, macptr);
				wdev->PortSecured = WPA_802_1X_PORT_NOT_SECURED;
			}
		}
#endif /* CONFIG_AP_SUPPORT */
	}

	/*EncrypType*/
	if (RTMPGetKeyParameter("EncrypType", tmpbuf, MAX_PARAMETER_LEN, pBuffer, TRUE)) {
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (apidx = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), apidx++) {
				pSecConfig = &pAd->ApCfg.MBSSID[apidx].wdev.SecConfig;
				MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("I/F(%s%d) ==> ",
						 INF_MBSSID_DEV_NAME, apidx));
				SetWdevEncrypMode(pSecConfig, macptr);
			}
		}
#endif /* CONFIG_AP_SUPPORT */
	}

	/* Web DefaultKeyID */
	if (RTMPGetKeyParameter("DefaultKeyID", tmpbuf, MAX_PARAMETER_LEN, pBuffer, TRUE)) {
		ULONG KeyIdx = 0;
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
			for (apidx = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), apidx++) {
				pSecConfig = &pAd->ApCfg.MBSSID[apidx].wdev.SecConfig;
				KeyIdx = os_str_tol(macptr, 0, 10);

				if ((KeyIdx >= 1) && (KeyIdx <= 4))
					pSecConfig->PairwiseKeyId = (UCHAR) (KeyIdx - 1);
				else
					pSecConfig->PairwiseKeyId = 0;

				MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("I/F(%s%d)) ==> DefaultKeyId=%d\n",
						 INF_MBSSID_DEV_NAME, apidx, pSecConfig->PairwiseKeyId));
			}
		}
#endif /* CONFIG_AP_SUPPORT */
	}

	/* KeyType, KeyStr for WEP  */
	for (idx = 0; idx < 4; idx++) {
		INT i = 0;
		RTMP_STRING tok_str[16];
		ULONG KeyType[HW_BEACON_MAX_NUM];

		snprintf(tok_str, sizeof(tok_str), "Key%dType", idx + 1);

		if (RTMPGetKeyParameter(tok_str, tmpbuf, MAX_PARAMETER_LEN, pBuffer, TRUE)) {
			for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++)
				if (i < MAX_MBSSID_NUM(pAd))
					KeyType[i] = os_str_tol(macptr, 0, 10);

#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
				BOOLEAN bKeyxStryIsUsed = FALSE;

				for (i = 0; i < pAd->ApCfg.BssidNum; i++) {
					snprintf(tok_str, sizeof(tok_str), "Key%dStr%d", idx + 1, i + 1);

					if (RTMPGetKeyParameter(tok_str, tmpbuf, MAX_PARAMETER_LEN, pBuffer, FALSE)) {
						pSecConfig = &pAd->ApCfg.MBSSID[i].wdev.SecConfig;
						ParseWebKey(pSecConfig, tmpbuf, idx, 0);

						if (bKeyxStryIsUsed == FALSE)
							bKeyxStryIsUsed = TRUE;
					}
				}

				if (bKeyxStryIsUsed == FALSE) {
					snprintf(tok_str, sizeof(tok_str), "Key%dStr", idx + 1);

					if (RTMPGetKeyParameter(tok_str, tmpbuf, MAX_PARAMETER_LEN, pBuffer, FALSE)) {
						if (pAd->ApCfg.BssidNum == 1) {
							pSecConfig = &pAd->ApCfg.MBSSID[BSS0].wdev.SecConfig;
							ParseWebKey(pSecConfig, tmpbuf, idx, 0);
						} else {
							/* Anyway, we still do the legacy dissection of the whole KeyxStr string.*/
							for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr; macptr = rstrtok(NULL, ";"), i++) {
								pSecConfig = &pAd->ApCfg.MBSSID[i].wdev.SecConfig;
								ParseWebKey(pSecConfig, macptr, idx, 0);
							}
						}
					}
				}
			}
#endif /* CONFIG_AP_SUPPORT */
		}
	}

	ReadWPAParameterFromFile(pAd, tmpbuf, pBuffer);
#ifdef DOT1X_SUPPORT
	ReadRadiusParameterFromFile(pAd, tmpbuf, pBuffer);
#endif /* DOT1X_SUPPORT */
#ifdef APCLI_SUPPORT
	ReadApcliSecParameterFromFile(pAd, tmpbuf, pBuffer);
#endif /* APCLI_SUPPORT */
#ifdef WDS_SUPPORT
	ReadWDSSecParameterFromFile(pAd, tmpbuf, pBuffer);
#endif /* WDS_SUPPORT */
#ifdef DOT11W_PMF_SUPPORT
	rtmp_read_pmf_parameters_from_file(pAd, tmpbuf, pBuffer);
#endif /* DOT11W_PMF_SUPPORT */
}


VOID fill_wtbl_key_info_struc(
	IN struct _ASIC_SEC_INFO *pInfo,
	OUT CMD_WTBL_SECURITY_KEY_T * rWtblSecurityKey)
{
	if (IS_REMOVEKEY_OPERATION(pInfo)) {
		rWtblSecurityKey->ucAddRemove = 1;
		rWtblSecurityKey->ucKeyLen = sizeof(rWtblSecurityKey->aucKeyMaterial);
	} else {   /* Add Key */
		SEC_KEY_INFO *pSecKey = &pInfo->Key;

		rWtblSecurityKey->ucAddRemove = 0;
		rWtblSecurityKey->ucKeyId = pInfo->KeyIdx;

		if (pSecKey->KeyLen > sizeof(rWtblSecurityKey->aucKeyMaterial))
			rWtblSecurityKey->ucKeyLen = sizeof(rWtblSecurityKey->aucKeyMaterial);
		else
			rWtblSecurityKey->ucKeyLen = pSecKey->KeyLen;

		os_move_mem(rWtblSecurityKey->aucKeyMaterial, pSecKey->Key, rWtblSecurityKey->ucKeyLen);

		if (IS_CIPHER_WEP(pInfo->Cipher)) {
			if (rWtblSecurityKey->ucKeyLen == 5)
				rWtblSecurityKey->ucAlgorithmId = CIPHER_SUIT_WEP_40;
			else if (rWtblSecurityKey->ucKeyLen == 13)
				rWtblSecurityKey->ucAlgorithmId = CIPHER_SUIT_WEP_104;
			else if (rWtblSecurityKey->ucKeyLen == 16)
				rWtblSecurityKey->ucAlgorithmId = CIPHER_SUIT_WEP_128;
		} else if (IS_CIPHER_TKIP(pInfo->Cipher)) {
			rWtblSecurityKey->ucAlgorithmId = CIPHER_SUIT_TKIP_W_MIC;
			os_move_mem(&rWtblSecurityKey->aucKeyMaterial[16], pSecKey->RxMic, LEN_TKIP_MIC);
			os_move_mem(&rWtblSecurityKey->aucKeyMaterial[24], pSecKey->TxMic, LEN_TKIP_MIC);
		} else if (IS_CIPHER_CCMP128(pInfo->Cipher))
			rWtblSecurityKey->ucAlgorithmId = CIPHER_SUIT_CCMP_W_MIC;
		else if (IS_CIPHER_CCMP256(pInfo->Cipher))
			rWtblSecurityKey->ucAlgorithmId = CIPHER_SUIT_CCMP_256;
		else if (IS_CIPHER_GCMP128(pInfo->Cipher))
			rWtblSecurityKey->ucAlgorithmId = CIPHER_SUIT_GCMP_128;
		else if (IS_CIPHER_GCMP256(pInfo->Cipher))
			rWtblSecurityKey->ucAlgorithmId = CIPHER_SUIT_GCMP_256;

		else {
			MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): Not support Cipher[0x%x]\n",
					 __func__, __LINE__, pInfo->Cipher));
			return;
		}

		if ((pInfo->Direction == SEC_ASIC_KEY_TX)
			|| (pInfo->Direction == SEC_ASIC_KEY_BOTH)) {
			rWtblSecurityKey->ucRkv = 0;
			rWtblSecurityKey->ucIkv = 0;
		}

		if ((pInfo->Direction == SEC_ASIC_KEY_RX)
			|| (pInfo->Direction == SEC_ASIC_KEY_BOTH)) {
			rWtblSecurityKey->ucRkv = 1;

			if (IS_CIPHER_BIP_CMAC128(pInfo->Cipher)
				|| ((IS_CIPHER_CCMP128(pInfo->Cipher)) && (rWtblSecurityKey->ucKeyLen == 32)))
				rWtblSecurityKey->ucIkv = 1;
		}
	}
}

VOID process_pmkid(
	RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	MAC_TABLE_ENTRY *entry,
	INT CacheIdx)
{
	if (CacheIdx != INVALID_PMKID_IDX) {
		/* Enqueue a EAPOL-start message with the pEntry for WPAPSK State Machine */
		if ((entry->EnqueueEapolStartTimerRunning == EAPOL_START_DISABLE
#ifdef HOSTAPD_SUPPORT
			&& wdev->Hostapd == Hostapd_Disable
#endif /*HOSTAPD_SUPPORT*/
			)
#ifdef WSC_AP_SUPPORT
			&& !entry->bWscCapable
#endif /* WSC_AP_SUPPORT */
			) {
			/* Enqueue a EAPOL-start message with the pEntry */
			entry->EnqueueEapolStartTimerRunning = EAPOL_START_PSK;
			entry->SecConfig.Handshake.WpaState = AS_INITPSK;
			os_move_mem(&entry->SecConfig.Handshake.AAddr,
				wdev->bssid,
				MAC_ADDR_LEN);
			os_move_mem(&entry->SecConfig.Handshake.SAddr,
				entry->Addr,
				MAC_ADDR_LEN);
			RTMPSetTimer(&entry->SecConfig.StartFor4WayTimer,
				ENQUEUE_EAPOL_START_TIMER);
		}

		store_pmkid_cache_in_sec_config(pAd, entry, CacheIdx);

		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("ASSOC - 2.PMKID matched and start key cache algorithm\n"));
	} else {
		store_pmkid_cache_in_sec_config(pAd, entry, INVALID_PMKID_IDX);
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("ASSOC - 2.PMKID not found\n"));

		/* Enqueue a EAPOL-start message to trigger EAP SM */
		if (entry->EnqueueEapolStartTimerRunning == EAPOL_START_DISABLE
#ifdef HOSTAPD_SUPPORT
			&& wdev->Hostapd == Hostapd_Disable
#endif/*HOSTAPD_SUPPORT*/
		) {
			entry->EnqueueEapolStartTimerRunning = EAPOL_START_1X;
			RTMPSetTimer(&entry->SecConfig.StartFor4WayTimer, ENQUEUE_EAPOL_START_TIMER);
		}
	}
}

VOID store_pmkid_cache_in_sec_config(
	IN RTMP_ADAPTER *pAd,
	IN MAC_TABLE_ENTRY *pEntry,
	IN INT32 cache_idx)
{
	if (!pEntry) {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s(): pEntry is null\n", __func__));
		return;
	}

	MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_LOUD, ("EntryType = %d\n", pEntry->EntryType));

	if (cache_idx == INVALID_PMKID_IDX) {
		pEntry->SecConfig.pmkid = NULL;
		pEntry->SecConfig.pmk_cache = NULL;
	} else {
		if (IS_ENTRY_CLIENT(pEntry)) {
#ifdef CONFIG_AP_SUPPORT
				pEntry->SecConfig.pmkid = pAd->ApCfg.PMKIDCache.BSSIDInfo[cache_idx].PMKID;
				pEntry->SecConfig.pmk_cache = pAd->ApCfg.PMKIDCache.BSSIDInfo[cache_idx].PMK;
#endif
		}
	}
}

UCHAR is_pmkid_cache_in_sec_config(
	IN struct _SECURITY_CONFIG *pSecConfig)
{
	if (pSecConfig && pSecConfig->pmkid && pSecConfig->pmk_cache)
		return TRUE;
	else
		return FALSE;
}


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
	wpa.h

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
*/

#ifndef	__WPA_H__
#define	__WPA_H__

#ifndef ROUND_UP
#define ROUND_UP(__x, __y) \
	(((ULONG)((__x)+((__y)-1))) & ((ULONG)~((__y)-1)))
#endif

#define	SET_UINT16_TO_ARRARY(_V, _LEN)		\
	{											\
		_V[0] = ((UINT16)_LEN) >> 8;			\
		_V[1] = ((UINT16)_LEN & 0xFF);					\
	}

#define	INC_UINT16_TO_ARRARY(_V, _LEN)			\
	{												\
		UINT16	var_len;							\
		\
		var_len = (_V[0]<<8) | (_V[1]);				\
		var_len += _LEN;							\
		\
		_V[0] = (var_len & 0xFF00) >> 8;			\
		_V[1] = (var_len & 0xFF);					\
	}

#define	CONV_ARRARY_TO_UINT16(_V)	((_V[0]<<8) | (_V[1]))

#define	ADD_ONE_To_64BIT_VAR(_V)		\
	{										\
		UCHAR	cnt = LEN_KEY_DESC_REPLAY;	\
		do {								\
			cnt--;							\
			_V[cnt]++;						\
			if (cnt == 0)					\
				break;						\
		} while (_V[cnt] == 0);				\
	}

#define INC_TX_TSC(_tsc, _cnt)                          \
	{                                                       \
		INT i = 0;                                            \
		while (++_tsc[i] == 0x0) {                            \
			i++;                                            \
			if (i == (_cnt))                                \
				break;                                      \
		}                                                   \
	}

#define IS_WPA_CAPABILITY(a)       (((a) >= Ndis802_11AuthModeWPA) && ((a) <= Ndis802_11AuthModeWPA1PSKWPA2PSK))

/*
	WFA recommend to restrict the encryption type in 11n-HT mode.
	So, the WEP and TKIP shall not be allowed to use HT rate.
 */
#define IS_INVALID_HT_SECURITY(_mode)		\
	((IS_CIPHER_WEP(_mode) || IS_CIPHER_TKIP(_mode)) && (!IS_CIPHER_CCMP128(_mode)))

#define MIX_CIPHER_WPA_TKIP_ON(x)       (((x) & 0x08) != 0)
#define MIX_CIPHER_WPA_AES_ON(x)        (((x) & 0x04) != 0)
#define MIX_CIPHER_WPA2_TKIP_ON(x)      (((x) & 0x02) != 0)
#define MIX_CIPHER_WPA2_AES_ON(x)       (((x) & 0x01) != 0)

/* Some definition are different between Keneral mode and Daemon mode */
#ifdef WPA_DAEMON_MODE
/* The definition for Daemon mode */
#define WPA_GET_BSS_NUM(_pAd)		((_pAd)->mbss_num)

#define WPA_GET_PMK(_pAd, _pEntry, _pmk)					\
	{															\
		_pmk = _pAd->MBSS[_pEntry->func_tb_idx].PMK;					\
	}

#define WPA_GET_GTK(_pAd, _pEntry, _gtk)					\
	{															\
		_gtk = _pAd->MBSS[_pEntry->func_tb_idx].GTK;					\
	}

#define WPA_GET_GROUP_CIPHER(_pAd, _pEntry, _cipher)		\
	{															\
		_cipher = (_pAd)->MBSS[_pEntry->func_tb_idx].GroupEncrypType;	\
	}

#define WPA_GET_DEFAULT_KEY_ID(_pAd, _pEntry, _idx)			\
	{															\
		_idx = (_pAd)->MBSS[_pEntry->func_tb_idx].DefaultKeyId;		\
	}

#define WPA_GET_BMCST_TSC(_pAd, _pEntry, _tsc)				\
	{															\
		_tsc = 1;												\
	}

#define WPA_BSSID(_pAd, _apidx)		((_pAd)->MBSS[_apidx].wlan_addr)

#define WPA_OS_MALLOC(_p, _s)		\
	{									\
		_p = os_malloc(_s);			\
	}

#define WPA_OS_FREE(_p)		\
	{								\
		os_free(_p);				\
	}

#define WPA_GET_CURRENT_TIME(_time)		\
	{										\
		struct timeval tv;					\
		gettimeofday(&tv, NULL);			\
		*(_time) = tv.tv_sec;					\
	}

#else
/* The definition for Driver mode */

#if defined(CONFIG_AP_SUPPORT) && defined(CONFIG_STA_SUPPORT)
#define WPA_GET_BSS_NUM(_pAd)		(((_pAd)->OpMode == OPMODE_AP) ? (_pAd)->ApCfg.BssidNum : 1)
#define WPA_GET_GROUP_CIPHER(_pAd, _pEntry, _cipher)					\
	{																	\
		_cipher = Ndis802_11WEPDisabled;								\
		if ((_pAd)->OpMode == OPMODE_AP) {								\
			if (IS_ENTRY_APCLI(_pEntry) &&								\
				((_pEntry)->wdev_idx < MAX_APCLI_NUM))			\
				_cipher = (_pAd)->ApCfg.ApCliTab[(_pEntry)->wdev_idx].GroupCipher;	\
			else if ((_pEntry)->func_tb_idx < (_pAd)->ApCfg.BssidNum)			\
				_cipher = (_pAd)->ApCfg.MBSSID[_pEntry->func_tb_idx].GroupKeyWepStatus;\
		} else															\
			_cipher = (_pAd)->StaCfg[0].GroupCipher;						\
	}

#define WPA_BSSID(_pAd, _apidx)	(((_pAd)->OpMode == OPMODE_AP) ?\
								 (_pAd)->ApCfg.MBSSID[_apidx].Bssid :\
								 (_pAd)->CommonCfg.Bssid)
#elif defined(CONFIG_AP_SUPPORT)
#define WPA_GET_BSS_NUM(_pAd)		((_pAd)->ApCfg.BssidNum)
#define WPA_GET_GROUP_CIPHER(_pAd, _pEntry, _cipher)				\
	{																\
		_cipher = Ndis802_11WEPDisabled;							\
		if (IS_ENTRY_APCLI(_pEntry) &&								\
			((_pEntry)->wdev_idx < MAX_APCLI_NUM))			\
			_cipher = (_pAd)->ApCfg.ApCliTab[(_pEntry)->wdev_idx].GroupCipher;	\
		else if ((_pEntry)->func_tb_idx < (_pAd)->ApCfg.BssidNum)			\
			_cipher = (_pAd)->ApCfg.MBSSID[_pEntry->func_tb_idx].GroupKeyWepStatus;\
	}

#define WPA_BSSID(_pAd, _apidx)	((_pAd)->ApCfg.MBSSID[_apidx].Bssid)

#elif defined(CONFIG_STA_SUPPORT)
#define WPA_GET_BSS_NUM(_pAd)		1
#define WPA_GET_GROUP_CIPHER(_pAd, _pEntry, _cipher)				\
	{																\
		_cipher = (_pAd)->StaCfg[0].GroupCipher;						\
	}
#define WPA_BSSID(_pAd, _apidx)	((_pAd)->CommonCfg.Bssid)
#endif /* defined(CONFIG_STA_SUPPORT) */

#define WPA_OS_MALLOC(_p, _s)		\
	{									\
		os_alloc_mem(NULL, (PUCHAR *)&_p, _s);		\
	}

#define WPA_OS_FREE(_p)		\
	{							\
		os_free_mem(_p);	\
	}

#define WPA_GET_CURRENT_TIME(_time)		NdisGetSystemUpTime(_time);

#endif /* End of Driver Mode */

/*========================================
	The prototype is defined in cmm_wpa.c
  ========================================*/
void inc_iv_byte(
	UCHAR *iv,
	UINT len,
	UINT cnt);

BOOLEAN WpaMsgTypeSubst(
	IN UCHAR EAPType,
	OUT INT *MsgType);

int RtmpPasswordHash(
	char *password,
	unsigned char *ssid,
	int ssidlength,
	unsigned char *output);


PUINT8 WPA_ExtractSuiteFromRSNIE(
	IN PUINT8 rsnie,
	IN UINT rsnie_len,
	IN UINT8 type,
	OUT UINT8 * count);

VOID WpaShowAllsuite(
	IN PUINT8 rsnie,
	IN UINT rsnie_len);

VOID RTMPInsertRSNIE(
	IN PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN PUINT8 rsnie_ptr,
	IN UINT8 rsnie_len,
	IN PUINT8 pmkid_ptr,
	IN UINT8 pmkid_len);

/*
 =====================================
	function prototype in cmm_wpa.c
 =====================================
*/
VOID WpaStateMachineInit(
	IN  PRTMP_ADAPTER   pAd,
	IN  STATE_MACHINE * Sm,
	OUT STATE_MACHINE_FUNC Trans[]);

VOID WpaDerivePTK(
	IN UCHAR *PMK,
	IN UCHAR *ANonce,
	IN UCHAR *AA,
	IN UCHAR *SNonce,
	IN UCHAR *SA,
	OUT UCHAR *output,
	IN UINT len);

VOID WpaDerivePTK_KDF_256(
	IN UCHAR *PMK,
	IN UCHAR *ANonce,
	IN UCHAR *AA,
	IN UCHAR *SNonce,
	IN UCHAR *SA,
	OUT UCHAR *output,
	IN UINT	len);

VOID WpaDerivePTK_KDF_384(
	IN UCHAR *PMK,
	IN UCHAR *ANonce,
	IN UCHAR *AA,
	IN UCHAR *SNonce,
	IN UCHAR *SA,
	OUT UCHAR *output,
	IN UINT	len);


VOID WpaDeriveGTK(
	IN UCHAR *PMK,
	IN UCHAR *GNonce,
	IN UCHAR *AA,
	OUT UCHAR *output,
	IN UINT len);

VOID WPA_ConstructKdeHdr(
	IN UINT8 data_type,
	IN UINT8 data_len,
	OUT PUCHAR pBuf);

#if defined(SOFT_ENCRYPT) || defined(ADHOC_WPA2PSK_SUPPORT)
PCIPHER_KEY RTMPSwCipherKeySelection(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pIV,
	IN RX_BLK * pRxBlk,
	IN PMAC_TABLE_ENTRY pEntry);

NDIS_STATUS RTMPSoftDecryptionAction(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pHdr,
	IN UCHAR UserPriority,
	IN PCIPHER_KEY pKey,
	INOUT PUCHAR pData,
	INOUT UINT16 *DataByteCnt);

VOID RTMPSoftConstructIVHdr(
	IN UCHAR CipherAlg,
	IN UCHAR key_id,
	IN PUCHAR pTxIv,
	OUT PUCHAR pHdrIv,
	OUT UINT8 * hdr_iv_len);

VOID RTMPSoftEncryptionAction(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR CipherAlg,
	IN PUCHAR pHdr,
	IN PUCHAR pSrcBufData,
	IN UINT32 SrcBufLen,
	IN UCHAR KeyIdx,
	IN PCIPHER_KEY pKey,
	OUT UINT8 * ext_len);
#endif /* SOFT_ENCRYPT || ADHOC_WPA2PSK_SUPPORT */

VOID WPAInstallPairwiseKey(
	PRTMP_ADAPTER pAd,
	UINT8 BssIdx,
	PMAC_TABLE_ENTRY pEntry,
	BOOLEAN bAE);

VOID WPAInstallSharedKey(
	PRTMP_ADAPTER pAd,
	UINT8 GroupCipher,
	UINT8 BssIdx,
	UINT8 KeyIdx,
	UINT8 Wcid,
	BOOLEAN bAE,
	PUINT8 pGtk,
	UINT8 GtkLen);

VOID CalculateMIC(
	IN UCHAR KeyDescVer,
	IN UCHAR *PTK,
	OUT PEAPOL_PACKET pMsg);



RTMP_STRING *GetEapolMsgType(CHAR msg);


/*
 =====================================
	function prototype in cmm_wep.c
 =====================================
*/
UINT RTMP_CALC_FCS32(
	IN UINT Fcs,
	IN PUCHAR Cp,
	IN INT Len);

VOID RTMPConstructWEPIVHdr(
	IN UINT8 key_idx,
	IN UCHAR *pn,
	OUT UCHAR *iv_hdr);

BOOLEAN RTMPSoftEncryptWEP(
	IN PUCHAR pIvHdr,
	IN PSEC_KEY_INFO pKey,
	INOUT PUCHAR pData,
	IN ULONG DataByteCnt);

BOOLEAN RTMPSoftDecryptWEP(
	IN PSEC_KEY_INFO pKey,
	INOUT PUCHAR pData,
	INOUT UINT16 *DataByteCnt);



/*
 =====================================
	function prototype in cmm_tkip.c
 =====================================
*/
BOOLEAN RTMPSoftDecryptTKIP(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pHdr,
	IN UCHAR UserPriority,
	IN PCIPHER_KEY pKey,
	INOUT PUCHAR pData,
	IN UINT16 *DataByteCnt);

VOID TKIP_GTK_KEY_WRAP(
	IN UCHAR *key,
	IN UCHAR *iv,
	IN UCHAR *input_text,
	IN UINT32 input_len,
	OUT UCHAR *output_text);

VOID TKIP_GTK_KEY_UNWRAP(
	IN UCHAR *key,
	IN UCHAR *iv,
	IN UCHAR *input_text,
	IN UINT32 input_len,
	OUT UCHAR *output_text);

/*
 =====================================
	function prototype in cmm_aes.c
 =====================================
*/
BOOLEAN RTMPSoftDecryptAES(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pData,
	IN ULONG DataByteCnt,
	IN PCIPHER_KEY pWpaKey);

VOID RTMPConstructCCMPHdr(
	IN UINT8 key_idx,
	IN UCHAR *pn,
	OUT UCHAR *ccmp_hdr);

BOOLEAN RTMPSoftEncryptCCMP(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pHdr,
	IN PUCHAR pIV,
	IN PUCHAR pKey,
	INOUT PUCHAR pData,
	IN UINT32 DataLen);

BOOLEAN RTMPSoftDecryptCCMP(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pHdr,
	IN PCIPHER_KEY pKey,
	INOUT PUCHAR pData,
	INOUT UINT16 *DataLen);

VOID CCMP_test_vector(
	IN PRTMP_ADAPTER pAd,
	IN INT input);

#ifdef RT_CFG80211_SUPPORT
BOOLEAN RTMPIsValidIEs(
	IN UCHAR *Ies,
	IN INT32 Len);
const UCHAR *RTMPFindIE(
	IN UCHAR Eid,
	IN const UCHAR *Ies,
	IN INT32 Len);
const UCHAR *RTMPFindWPSIE(
	IN const UCHAR *Ies,
	IN INT32 Len);
#endif /* RT_CFG80211_SUPPORT */


/* --------------------Eddy---------------- */
VOID PRF(
	IN UCHAR *key,
	IN INT key_len,
	IN UCHAR *prefix,
	IN INT prefix_len,
	IN UCHAR *data,
	IN INT data_len,
	OUT UCHAR *output,
	IN INT len);

VOID KDF(
	IN PUINT8 key,
	IN INT key_len,
	IN PUINT8 label,
	IN INT label_len,
	IN PUINT8 data,
	IN INT data_len,
	OUT PUINT8 output,
	IN USHORT len);

VOID KDF_384(
	IN PUINT8 key,
	IN INT key_len,
	IN PUINT8 label,
	IN INT label_len,
	IN PUINT8 data,
	IN INT data_len,
	OUT PUINT8 output,
	IN USHORT len);

VOID HKDF_expand_sha256(IN UCHAR *secret,
			IN INT secret_len,
			IN UCHAR *info,
			IN INT info_len,
			OUT UCHAR *output,
			INT output_Len);

VOID HKDF_expand_sha384(IN UCHAR *secret,
			IN INT secret_len,
			IN UCHAR *info,
			IN INT info_len,
			OUT UCHAR *output,
			INT output_Len);

VOID GenRandom(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR *macAddr,
	OUT UCHAR *random);

VOID RTMPToWirelessSta(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY pEntry,
	IN PUCHAR pHeader802_3,
	IN UINT HdrLen,
	IN PUCHAR pData,
	IN UINT DataLen,
	IN BOOLEAN bClearFrame);

BOOLEAN RTMPCheckWPAframe(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY pEntry,
	IN PUCHAR pData,
	IN ULONG DataByteCount,
	IN UCHAR wdev_idx,
	IN BOOLEAN eth_frm);

INT WPAPasswordHash(
	IN CHAR *password,
	IN UCHAR *ssid,
	IN INT ssidlength,
	OUT UCHAR *output);

INT SetWPAPSKKey(
	IN RTMP_ADAPTER * pAd,
	IN RTMP_STRING *keyString,
	IN INT keyStringLen,
	IN UCHAR *pHashStr,
	IN INT hashStrLen,
	OUT PUCHAR pPMKBuf);

BOOLEAN rtmp_chk_tkip_mic(
	IN RTMP_ADAPTER * pAd,
	IN MAC_TABLE_ENTRY * pEntry,
	IN RX_BLK * pRxBlk);

VOID ReadWPAParameterFromFile(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *tmpbuf,
	IN RTMP_STRING *pBuffer);

VOID WPAMakeRSNIE(
	IN UINT32 wdev_type,
	IN struct _SECURITY_CONFIG *pSecConfig,
	IN PMAC_TABLE_ENTRY pEntry);

BOOLEAN wpa_rsne_sanity(
	IN PUCHAR rsnie_ptr,
	IN UCHAR rsnie_len,
	OUT UCHAR *end_field);


UINT WPAValidateRSNIE(
	IN struct _SECURITY_CONFIG *pSecConfigSelf,
	IN struct _SECURITY_CONFIG *pSecConfigEntry,
	IN PUCHAR pRsnIe,
	IN UCHAR rsnie_len);

VOID WpaDeriveGTK(
	IN UCHAR *PMK,
	IN UCHAR *GNonce,
	IN UCHAR *AA,
	OUT UCHAR *output,
	IN UINT len);

VOID WPAInstallKey(
	IN PRTMP_ADAPTER pAd,
	IN struct _ASIC_SEC_INFO *pInfo,
	IN BOOLEAN bAE,
	IN BOOLEAN is_install);

VOID WPACalculateMIC(
	IN UCHAR KeyDescVer,
	IN UINT32 AKMMap,
	IN UCHAR *PTK,
	IN UINT8 key_deri_alg,
	OUT PEAPOL_PACKET pMsg);

VOID WPAInsertRSNIE(
	IN PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN PUINT8 rsnie_ptr,
	IN UINT8  rsnie_len,
	IN PUINT8 pmkid_ptr,
	IN UINT8  pmkid_len);

VOID WPAConstructKdeHdr(
	IN UINT8 data_type,
	IN UINT8 data_len,
	OUT PUCHAR pBuf);

VOID WPAConstructEapolKeyData(
	IN PMAC_TABLE_ENTRY pEntry,
	IN UCHAR MsgType,
	IN	UCHAR keyDescVer,
	IN struct _SECURITY_CONFIG *pSecPairwise,
	IN struct _SECURITY_CONFIG *pSecGroup,
	OUT PEAPOL_PACKET pMsg);

VOID WPAConstructEapolMsg(
	IN PMAC_TABLE_ENTRY pEntry,
	IN UCHAR MsgType,
	IN struct _SECURITY_CONFIG *pSecPairwise,
	IN struct _SECURITY_CONFIG *pSecGroup,
	OUT PEAPOL_PACKET pMsg);

BOOLEAN WpaMessageSanity(
	IN PRTMP_ADAPTER pAd,
	IN PEAPOL_PACKET pMsg,
	IN ULONG MsgLen,
	IN UCHAR MsgType,
	IN struct _SECURITY_CONFIG *pSecConfig,
	IN MAC_TABLE_ENTRY * pEntry);

VOID WPABuildPairMsg1(
	IN  RTMP_ADAPTER * pAd,
	IN struct _SECURITY_CONFIG *pSecConfig,
	IN  MAC_TABLE_ENTRY * pEntry);

VOID WPABuildPairMsg2(
	IN  RTMP_ADAPTER * pAd,
	IN struct _SECURITY_CONFIG *pSecConfig,
	IN  MAC_TABLE_ENTRY * pEntry);

VOID WPABuildPairMsg3(
	IN  RTMP_ADAPTER * pAd,
	IN struct _SECURITY_CONFIG *pSecConfig,
	IN  MAC_TABLE_ENTRY * pEntry);

VOID WPABuildPairMsg4(
	IN  RTMP_ADAPTER * pAd,
	IN struct _SECURITY_CONFIG *pSecConfig,
	IN  MAC_TABLE_ENTRY * pEntry);

VOID WPABuildGroupMsg1(
	IN PRTMP_ADAPTER pAd,
	IN struct _SECURITY_CONFIG *pSecConfig,
	IN MAC_TABLE_ENTRY * pEntry);

VOID WPABuildGroupMsg2(
	IN PRTMP_ADAPTER pAd,
	IN struct _SECURITY_CONFIG *pSecConfig,
	IN MAC_TABLE_ENTRY * pEntry);

VOID PeerPairMsg1Action(
	IN RTMP_ADAPTER * pAd,
	IN MAC_TABLE_ENTRY * pEntry,
	IN struct _SECURITY_CONFIG *pSecConfig,
	IN MLME_QUEUE_ELEM * Elem);

VOID PeerPairMsg2Action(
	IN RTMP_ADAPTER * pAd,
	IN MAC_TABLE_ENTRY * pEntry,
	IN struct _SECURITY_CONFIG *pSecConfig,
	IN MLME_QUEUE_ELEM * Elem);

VOID PeerPairMsg3Action(
	IN RTMP_ADAPTER * pAd,
	IN MAC_TABLE_ENTRY * pEntry,
	IN struct _SECURITY_CONFIG *pSecConfig,
	IN MLME_QUEUE_ELEM * Elem);

VOID PeerPairMsg4Action(
	IN RTMP_ADAPTER * pAd,
	IN MAC_TABLE_ENTRY * pEntry,
	IN struct _SECURITY_CONFIG *pSecConfig,
	IN MLME_QUEUE_ELEM * Elem);

VOID PeerGroupMsg1Action(
	IN PRTMP_ADAPTER    pAd,
	IN MAC_TABLE_ENTRY * pEntry,
	IN struct _SECURITY_CONFIG *pSecConfig,
	IN MLME_QUEUE_ELEM * Elem);

VOID PeerGroupMsg2Action(
	IN PRTMP_ADAPTER    pAd,
	IN MAC_TABLE_ENTRY * pEntry,
	IN struct _SECURITY_CONFIG *pSecConfig,
	IN MLME_QUEUE_ELEM * Elem);

DECLARE_TIMER_FUNCTION(WPAStartFor4WayExec);
DECLARE_TIMER_FUNCTION(WPAStartFor2WayExec);
DECLARE_TIMER_FUNCTION(WPAHandshakeMsgRetryExec);
VOID WPAStartFor4WayExec(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

VOID WPAStartFor2WayExec(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

VOID WPAHandshakeMsgRetryExec(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

#endif


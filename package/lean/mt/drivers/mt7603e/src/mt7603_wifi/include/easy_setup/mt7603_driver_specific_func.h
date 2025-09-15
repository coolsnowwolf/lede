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
	cmm_ez.c

	Abstract:
	Easy Setup APIs

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
*/
#ifndef __EZ_driver_specific_fun_H__
#define __EZ_driver_specific_fun_H__
#ifdef WH_EZ_SETUP

#define SET_AUTHMODE_WPA2PSK(_Auth)  (_Auth = Ndis802_11AuthModeWPA2PSK)
#define SET_ENCRYTYPE_AES(_encry)  (_encry = Ndis802_11AESEnable)
/*#define IS_ADPTR_EZ_SETUP_ENABLED(pAd) (pAd->ApCfg.MBSSID[0].wdev.ez_driver_params.enable_easy_setup)*/
/*#define VALID_UCAST_ENTRY_WCID(_wcid)  (_wcid < MAX_LEN_OF_MAC_TABLE) move to rtmp_def.h, mbo also used.*/

#define CLEAR_SEC_AKM(_AKMMap)              (_AKMMap = 0x0)
#define SET_AKM_OPEN(_AKMMap)           (_AKMMap |= (1 << SEC_AKM_OPEN))
#define SET_AKM_SHARED(_AKMMap)       (_AKMMap |= (1 << SEC_AKM_SHARED))
#define SET_AKM_AUTOSWITCH(_AKMMap)       (_AKMMap |= (1 << SEC_AKM_AUTOSWITCH))
#define SET_AKM_WPA1(_AKMMap)          (_AKMMap |= (1 << SEC_AKM_WPA1))
#define SET_AKM_WPA1PSK(_AKMMap)    (_AKMMap |= (1 << SEC_AKM_WPA1PSK))
#define SET_AKM_WPANONE(_AKMMap)  (_AKMMap |= (1 << SEC_AKM_WPANone))
#define SET_AKM_WPA2(_AKMMap)          (_AKMMap |= (1 << SEC_AKM_WPA2))
#define SET_AKM_WPA2PSK(_AKMMap)    (_AKMMap |= (1 << SEC_AKM_WPA2PSK))
#define SET_AKM_FT_WPA2(_AKMMap)                  (_AKMMap |= (1 << SEC_AKM_FT_WPA2))
#define SET_AKM_FT_WPA2PSK(_AKMMap)            (_AKMMap |= (1 << SEC_AKM_FT_WPA2PSK))
#define SET_AKM_WPA2_SHA256(_AKMMap)         (_AKMMap |= (1 << SEC_AKM_WPA2_SHA256))
#define SET_AKM_WPA2PSK_SHA256(_AKMMap)   (_AKMMap |= (1 << SEC_AKM_WPA2PSK_SHA256))
#define SET_AKM_TDLS(_AKMMap)                           (_AKMMap |= (1 << SEC_AKM_TDLS))
#define SET_AKM_SAE_SHA256(_AKMMap)              (_AKMMap |= (1 << SEC_AKM_SAE_SHA256))
#define SET_AKM_FT_SAE_SHA256(_AKMMap)        (_AKMMap |= (1 << SEC_AKM_FT_SAE_SHA256))
#define SET_AKM_SUITEB_SHA256(_AKMMap)         (_AKMMap |= (1 << SEC_AKM_SUITEB_SHA256))
#define SET_AKM_SUITEB_SHA384(_AKMMap)         (_AKMMap |= (1 << SEC_AKM_SUITEB_SHA384))
#define SET_AKM_FT_WPA2_SHA384(_AKMMap)     (_AKMMap |= (1 << SEC_AKM_FT_WPA2_SHA384))
#ifdef WAPI_SUPPORT
#define SET_AKM_WAICERT(_AKMMap)                   (_AKMMap |= (1 << SEC_AKM_WAICERT))
#define SET_AKM_WPIPSK(_AKMMap)                     (_AKMMap |= (1 << SEC_AKM_WAIPSK))
#endif /* WAPI_SUPPORT */


#define IS_CIPHER_NONE(_Cipher)          (((_Cipher) & (1 << SEC_CIPHER_NONE)) > 0)
#define IS_CIPHER_WEP40(_Cipher)          (((_Cipher) & (1 << SEC_CIPHER_WEP40)) > 0)
#define IS_CIPHER_WEP104(_Cipher)        (((_Cipher) & (1 << SEC_CIPHER_WEP104)) > 0)
#define IS_CIPHER_WEP128(_Cipher)        (((_Cipher) & (1 << SEC_CIPHER_WEP128)) > 0)
#define IS_CIPHER_WEP(_Cipher)              (((_Cipher) & ((1 << SEC_CIPHER_WEP40) | (1 << SEC_CIPHER_WEP104) | (1 << SEC_CIPHER_WEP128))) > 0)
#define IS_CIPHER_TKIP(_Cipher)              (((_Cipher) & (1 << SEC_CIPHER_TKIP)) > 0)
#define IS_CIPHER_WEP_TKIP_ONLY(_Cipher)     ((IS_CIPHER_WEP(_Cipher) || IS_CIPHER_TKIP(_Cipher)) && (_Cipher < (1 << SEC_CIPHER_CCMP128)))
#define IS_CIPHER_CCMP128(_Cipher)      (((_Cipher) & (1 << SEC_CIPHER_CCMP128)) > 0)
#define IS_CIPHER_CCMP256(_Cipher)      (((_Cipher) & (1 << SEC_CIPHER_CCMP256)) > 0)
#define IS_CIPHER_GCMP128(_Cipher)     (((_Cipher) & (1 << SEC_CIPHER_GCMP128)) > 0)
#define IS_CIPHER_GCMP256(_Cipher)     (((_Cipher) & (1 << SEC_CIPHER_GCMP256)) > 0)
#define IS_CIPHER_BIP_CMAC128(_Cipher)     (((_Cipher) & (1 << SEC_CIPHER_BIP_CMAC128)) > 0)


#define IS_AKM_OPEN(_AKMMap)                           ((_AKMMap & (1 << SEC_AKM_OPEN)) > 0)
#define IS_AKM_SHARED(_AKMMap)                       ((_AKMMap & (1 << SEC_AKM_SHARED)) > 0)
#define IS_AKM_AUTOSWITCH(_AKMMap)              ((_AKMMap & (1 << SEC_AKM_AUTOSWITCH)) > 0)
#define IS_AKM_WPA1(_AKMMap)                           ((_AKMMap & (1 << SEC_AKM_WPA1)) > 0)
#define IS_AKM_WPA1PSK(_AKMMap)                    ((_AKMMap & (1 << SEC_AKM_WPA1PSK)) > 0)
#define IS_AKM_WPANONE(_AKMMap)                  ((_AKMMap & (1 << SEC_AKM_WPANone)) > 0)
#define IS_AKM_WPA2(_AKMMap)                          ((_AKMMap & (1 << SEC_AKM_WPA2)) > 0)
#define IS_AKM_WPA2PSK(_AKMMap)                    ((_AKMMap & (1 << SEC_AKM_WPA2PSK)) > 0)
#define IS_AKM_FT_WPA2(_AKMMap)                     ((_AKMMap & (1 << SEC_AKM_FT_WPA2)) > 0)
#define IS_AKM_FT_WPA2PSK(_AKMMap)              ((_AKMMap & (1 << SEC_AKM_FT_WPA2PSK)) > 0)
#define IS_AKM_WPA2_SHA256(_AKMMap)            ((_AKMMap & (1 << SEC_AKM_WPA2_SHA256)) > 0)
#define IS_AKM_WPA2PSK_SHA256(_AKMMap)      ((_AKMMap & (1 << SEC_AKM_WPA2PSK_SHA256)) > 0)
#define IS_AKM_TDLS(_AKMMap)                             ((_AKMMap & (1 << SEC_AKM_TDLS)) > 0)
#define IS_AKM_SAE_SHA256(_AKMMap)                ((_AKMMap & (1 << SEC_AKM_SAE_SHA256)) > 0)
#define IS_AKM_FT_SAE_SHA256(_AKMMap)          ((_AKMMap & (1 << SEC_AKM_FT_SAE_SHA256)) > 0)
#define IS_AKM_SUITEB_SHA256(_AKMMap)          ((_AKMMap & (1 << SEC_AKM_SUITEB_SHA256)) > 0)
#define IS_AKM_SUITEB_SHA384(_AKMMap)          ((_AKMMap & (1 << SEC_AKM_SUITEB_SHA384)) > 0)
#define IS_AKM_FT_WPA2_SHA384(_AKMMap)      ((_AKMMap & (1 << SEC_AKM_FT_WPA2_SHA384)) > 0)
#ifdef WAPI_SUPPORT
#define IS_AKM_WAICERT(_AKMMap)                      ((_AKMMap & (1 << SEC_AKM_WAICERT)) > 0)
#define IS_AKM_WPIPSK(_AKMMap)                        ((_AKMMap & (1 << SEC_AKM_WAIPSK)) > 0)
#endif /* WAPI_SUPPORT */

#define IS_AKM_PSK(_AKMMap)     (IS_AKM_WPA1PSK(_AKMMap)  \
                                                              || IS_AKM_WPA2PSK(_AKMMap))

#define IS_AKM_1X(_AKMMap)     (IS_AKM_WPA1(_AKMMap)  \
                                                              || IS_AKM_WPA2(_AKMMap))

#define IS_AKM_WPA_CAPABILITY(_AKMMap)     (IS_AKM_WPA1(_AKMMap)  \
                                                                              || IS_AKM_WPA1PSK(_AKMMap) \
                                                                              || IS_AKM_WPANONE(_AKMMap) \
                                                                              || IS_AKM_WPA2(_AKMMap) \
                                                                              || IS_AKM_WPA2PSK(_AKMMap) \
                                                                              || IS_AKM_WPA2_SHA256(_AKMMap) \
                                                                              || IS_AKM_WPA2PSK_SHA256(_AKMMap))
typedef enum _SEC_CIPHER_MODE {
	SEC_CIPHER_NONE,
	SEC_CIPHER_WEP40,
	SEC_CIPHER_WEP104,
	SEC_CIPHER_WEP128,
	SEC_CIPHER_TKIP,
	SEC_CIPHER_CCMP128,
	SEC_CIPHER_CCMP256,
	SEC_CIPHER_GCMP128,
	SEC_CIPHER_GCMP256,
	SEC_CIPHER_BIP_CMAC128,
	SEC_CIPHER_BIP_CMAC256,
	SEC_CIPHER_BIP_GMAC128,
	SEC_CIPHER_BIP_GMAC256,
	SEC_CIPHER_WPI_SMS4, /* WPI SMS4 support */
	SEC_CIPHER_MAX /* Not a real mode, defined as upper bound */
} SEC_CIPHER_MODE;


#define CLEAR_PAIRWISE_CIPHER(_SecConfig)      ((_SecConfig)->PairwiseCipher = 0x0)
#define CLEAR_GROUP_CIPHER(_SecConfig)          ((_SecConfig)->GroupCipher = 0x0)
#define GET_PAIRWISE_CIPHER(_SecConfig)         ((_SecConfig)->PairwiseCipher)
#define GET_GROUP_CIPHER(_SecConfig)              ((_SecConfig)->GroupCipher)
#define CLEAR_CIPHER(_cipher)               	    (_cipher  = 0x0)
#define SET_CIPHER_NONE(_cipher)               (_cipher |= (1 << SEC_CIPHER_NONE))
#define SET_CIPHER_WEP40(_cipher)             (_cipher |= (1 << SEC_CIPHER_WEP40))
#define SET_CIPHER_WEP104(_cipher)           (_cipher |= (1 << SEC_CIPHER_WEP104))
#define SET_CIPHER_WEP128(_cipher)           (_cipher |= (1 << SEC_CIPHER_WEP128))
#define SET_CIPHER_WEP(_cipher)                 (_cipher |= (1 << SEC_CIPHER_WEP40) | (1 << SEC_CIPHER_WEP104) | (1 << SEC_CIPHER_WEP128))
#define SET_CIPHER_TKIP(_cipher)                  (_cipher |= (1 << SEC_CIPHER_TKIP))
#define SET_CIPHER_CCMP128(_cipher)          (_cipher |= (1 << SEC_CIPHER_CCMP128))
#define SET_CIPHER_CCMP256(_cipher)          (_cipher |= (1 << SEC_CIPHER_CCMP256))
#define SET_CIPHER_GCMP128(_cipher)         (_cipher |= (1 << SEC_CIPHER_GCMP128))
#define SET_CIPHER_GCMP256(_cipher)         (_cipher |= (1 << SEC_CIPHER_GCMP256))
#ifdef WAPI_SUPPORT
#define SET_CIPHER_WPI_SMS4(_cipher)       (_cipher |= (1 << SEC_CIPHER_WPI_SMS4))
#endif /* WAPI_SUPPORT */


/* 802.11 authentication and key management */
typedef enum _SEC_AKM_MODE {
    SEC_AKM_OPEN,
    SEC_AKM_SHARED,
    SEC_AKM_AUTOSWITCH,
    SEC_AKM_WPA1, /* Enterprise security over 802.1x */
    SEC_AKM_WPA1PSK,
    SEC_AKM_WPANone, /* For Win IBSS, directly PTK, no handshark */
    SEC_AKM_WPA2, /* Enterprise security over 802.1x */
    SEC_AKM_WPA2PSK,
    SEC_AKM_FT_WPA2,
    SEC_AKM_FT_WPA2PSK,
    SEC_AKM_WPA2_SHA256,
    SEC_AKM_WPA2PSK_SHA256,
    SEC_AKM_TDLS,
    SEC_AKM_SAE_SHA256,
    SEC_AKM_FT_SAE_SHA256,
    SEC_AKM_SUITEB_SHA256,
    SEC_AKM_SUITEB_SHA384,
    SEC_AKM_FT_WPA2_SHA384,
    SEC_AKM_WAICERT, /* WAI certificate authentication */
    SEC_AKM_WAIPSK, /* WAI pre-shared key */
    SEC_AKM_MAX /* Not a real mode, defined as upper bound */
} SEC_AKM_MODE, *PSEC_AKM_MODE;

typedef enum{
	RUNNING = 0,
	PASS,
	FAIL
}LOOPBACK_STATUS;

enum BCN_UPDATE_REASON
{
	INTERFACE_STATE_CHANGE = 0, /*there is interface up or down, check related TXD handle*/
	IE_CHANGE = 1, /* simple IE change, just update the corresponding interface content. */
	AP_RENEW = 2, /* prepared All beacon for All active interface. */
	PRETBTT_UPDATE = 3, /* update function routine, source could from INT isr or timer or event notify. */
};

VOID UpdateBeaconHandler(		
	void *ad_obj,
	struct wifi_dev *wdev,
	UCHAR BCN_UPDATE_REASON);

INT	rtmp_set_channel(
	RTMP_ADAPTER *pAd, 
	struct wifi_dev *wdev, 
	UCHAR Channel);

UINT32 AuthMode_to_AKM_map(NDIS_802_11_AUTHENTICATION_MODE authMode);
NDIS_802_11_AUTHENTICATION_MODE AKM_map_to_AuthMode(UINT32 _AKMMap);
UINT32 WepStatus_to_PairwiseCipher(NDIS_802_11_WEP_STATUS WepStatus);
NDIS_802_11_WEP_STATUS PairwiseCipher_to_WepStatus(UINT32 PairwiseCipher);

#endif /* WH_EZ_SETUP */
#endif /* __EZ_chip_ops_api_H__*/

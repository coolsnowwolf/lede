#ifndef SEC_CMM_H
#define SEC_CMM_H

#include "rtmp_type.h"
#include "security/dot11i_wpa.h"

#ifdef DOT11W_PMF_SUPPORT
#include "security/pmf_cmm.h"
#endif /* DOT11W_PMF_SUPPORT */


#include "security/owe_cmm.h"

/* Key Related definitions */
#define SEC_SHARE_KEY_NUM 4
#define SEC_KEY_NUM SEC_SHARE_KEY_NUM
#define SEC_RSNIE_NUM  2 /* Support  IE, 1:WPA1_IE or WAPI_IE, 2: WPA2_IE */


typedef enum _SEC_RSNIE_TYPE {
	SEC_RSNIE_NONE,
	SEC_RSNIE_WPA1_IE,
	SEC_RSNIE_WPA2_IE,
	SEC_RSNIE_MAX /* Not a real mode, defined as upper bound */
} SEC_RSNIE_TYPE, *PSEC_RSNIE_TYPE;


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
	SEC_AKM_OWE,
	SEC_AKM_MAX /* Not a real mode, defined as upper bound */
} SEC_AKM_MODE, *PSEC_AKM_MODE;

enum SEC_KEY_DERI_ALG {
	SEC_KEY_DERI_SHA1 = 0,
	SEC_KEY_DERI_SHA256,
	SEC_KEY_DERI_SHA384,
	SEC_KEY_DERI_SHA512
};

enum RSN_FIELD {
	RSN_FIELD_NONE = 0,
	RSN_FIELD_GROUP_CIPHER,
	RSN_FIELD_PAIRWISE_CIPHER,
	RSN_FIELD_AKM,
	RSN_FIELD_RSN_CAP,
	RSN_FIELD_PMKID,
	RSN_FIELD_GROUP_MGMT_CIPHER,
	RSN_FIELD_EXTENSIBLE_ELE
};


#define GET_SEC_AKM(_SecConfig)              ((_SecConfig)->AKMMap)
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
#define SET_AKM_OWE(_AKMMap)     (_AKMMap |= (1 << SEC_AKM_OWE))

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
#define IS_AKM_OWE(_AKMMap)      ((_AKMMap & (1 << SEC_AKM_OWE)) > 0)

#ifdef DOT11_SUITEB_SUPPORT
#define IS_AKM_WPA3(_AKMMap)	(IS_AKM_SUITEB_SHA384(_AKMMap))
#define SET_AKM_WPA3(_AKMMap)	SET_AKM_SUITEB_SHA384(_AKMMap)
#else
#define IS_AKM_WPA3(_AKMMap)	(FALSE)
#define SET_AKM_WPA3(_AKMMap)
#endif

#ifdef DOT11_SAE_SUPPORT
#define IS_AKM_WPA3PSK(_AKMMap) (IS_AKM_SAE_SHA256(_AKMMap))
#define SET_AKM_WPA3PSK(_AKMMap) SET_AKM_SAE_SHA256(_AKMMap)
#else
#define IS_AKM_WPA3PSK(_AKMMap) (FALSE)
#define SET_AKM_WPA3PSK(_AKMMap)
#endif


#define IS_AKM_SAE(_AKMMap)     (IS_AKM_SAE_SHA256(_AKMMap)  \
				|| IS_AKM_FT_SAE_SHA256(_AKMMap))


#define IS_AKM_PSK(_AKMMap)     (IS_AKM_WPA1PSK(_AKMMap)  \
				|| IS_AKM_WPA2PSK(_AKMMap)\
				|| IS_AKM_WPA3PSK(_AKMMap)\
				|| IS_AKM_OWE(_AKMMap))

#define IS_AKM_1X(_AKMMap)     (IS_AKM_WPA1(_AKMMap)  \
								|| IS_AKM_WPA2(_AKMMap)\
								|| IS_AKM_WPA3(_AKMMap))


#define IS_AKM_WPA_CAPABILITY(_AKMMap)     (IS_AKM_WPA1(_AKMMap)  \
		|| IS_AKM_WPA1PSK(_AKMMap) \
		|| IS_AKM_WPANONE(_AKMMap) \
		|| IS_AKM_WPA2(_AKMMap) \
		|| IS_AKM_WPA2PSK(_AKMMap) \
		|| IS_AKM_WPA2_SHA256(_AKMMap) \
		|| IS_AKM_WPA2PSK_SHA256(_AKMMap)\
		|| IS_AKM_WPA3(_AKMMap)\
		|| IS_AKM_WPA3PSK(_AKMMap)\
		|| IS_AKM_OWE(_AKMMap))

#define IS_AKM_SHA256(_AKMMap)     (IS_AKM_FT_WPA2(_AKMMap)  \
		|| IS_AKM_FT_WPA2PSK(_AKMMap) \
		|| IS_AKM_WPA2PSK_SHA256(_AKMMap) \
		|| IS_AKM_WPA2_SHA256(_AKMMap) \
		|| IS_AKM_SAE_SHA256(_AKMMap) \
		|| IS_AKM_FT_SAE_SHA256(_AKMMap) \
		|| IS_AKM_SUITEB_SHA256(_AKMMap))


#define IS_AKM_SHA384(_AKMMap)     (IS_AKM_SUITEB_SHA384(_AKMMap) \
		|| IS_AKM_FT_WPA2_SHA384(_AKMMap))


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
	SEC_CIPHER_OWE_SHA256, /*Group 19, ECC sha 256*/
	SEC_CIPHER_OWE_SHA384, /*Group 20, ECC sha 384*/
	SEC_CIPHER_OWE_SHA521, /*Group 21, ECC sha 512, integrity with HMAC-SHA-521*/
	SEC_CIPHER_MAX /* Not a real mode, defined as upper bound */
} SEC_CIPHER_MODE;

#define CLEAR_PAIRWISE_CIPHER(_SecConfig)      ((_SecConfig)->PairwiseCipher = 0x0)
#define CLEAR_GROUP_CIPHER(_SecConfig)          ((_SecConfig)->GroupCipher = 0x0)
#define GET_PAIRWISE_CIPHER(_SecConfig)         ((_SecConfig)->PairwiseCipher)
#define GET_GROUP_CIPHER(_SecConfig)              ((_SecConfig)->GroupCipher)
#define CLEAR_CIPHER(_cipher)					(_cipher  = 0x0)
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
#define SET_CIPHER_BIP_CMAC128(_cipher)     (_cipher |= (1 << SEC_CIPHER_BIP_CMAC128))
#define SET_CIPHER_BIP_CMAC256(_cipher)     (_cipher |= (1 << SEC_CIPHER_BIP_CMAC256))
#define SET_CIPHER_BIP_GMAC128(_cipher)     (_cipher |= (1 << SEC_CIPHER_BIP_GMAC128))
#define SET_CIPHER_BIP_GMAC256(_cipher)     (_cipher |= (1 << SEC_CIPHER_BIP_GMAC256))
#define SET_CIPHER_OWE_SHA256(_cipher)     (_cipher |= (1 << SEC_CIPHER_OWE_SHA256))
#define SET_CIPHER_OWE_SHA384(_cipher)     (_cipher |= (1 << SEC_CIPHER_OWE_SHA384))
#define SET_CIPHER_OWE_SHA521(_cipher)     (_cipher |= (1 << SEC_CIPHER_OWE_SHA521))

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
#define IS_CIPHER_BIP_CMAC256(_Cipher)     (((_Cipher) & (1 << SEC_CIPHER_BIP_CMAC256)) > 0)
#define IS_CIPHER_BIP_GMAC128(_Cipher)     (((_Cipher) & (1 << SEC_CIPHER_BIP_GMAC128)) > 0)
#define IS_CIPHER_BIP_GMAC256(_Cipher)     (((_Cipher) & (1 << SEC_CIPHER_BIP_GMAC256)) > 0)
#define IS_CIPHER_OWE_SHA256(_Cipher)     (((_Cipher) & (1 << SEC_CIPHER_OWE_SHA256)) > 0)
#define IS_CIPHER_OWE_SHA384(_Cipher)     (((_Cipher) & (1 << SEC_CIPHER_OWE_SHA384)) > 0)
#define IS_CIPHER_OWE_SHA521(_Cipher)     (((_Cipher) & (1 << SEC_CIPHER_OWE_SHA521)) > 0)

#define IS_AKM_OPEN_ONLY(_AKMMap)		((_AKMMap) == (1 << SEC_AKM_OPEN))
#define IS_CIPHER_NONE_ONLY(_Cipher)    ((_Cipher) == (1 << SEC_CIPHER_NONE))
#define IS_SECURITY(_SecConfig)    ((!IS_AKM_OPEN_ONLY((_SecConfig)->AKMMap)) ||  (!IS_CIPHER_NONE_ONLY((_SecConfig)->PairwiseCipher)))
#define IS_NO_SECURITY(_SecConfig)    (IS_AKM_OPEN((_SecConfig)->AKMMap) &&  IS_CIPHER_NONE((_SecConfig)->PairwiseCipher))
#define IS_SECURITY_OPEN_WEP(_SecConfig)    (IS_AKM_OPEN((_SecConfig)->AKMMap)  && IS_CIPHER_WEP((_SecConfig)->PairwiseCipher))

#define IS_SECURITY_WPA2PSK_AES(_SecConfig)    ((IS_AKM_WPA2((_SecConfig)->AKMMap)  || IS_AKM_WPA2PSK((_SecConfig)->AKMMap))  \
		&& (IS_CIPHER_CCMP128((_SecConfig)->PairwiseCipher)))

#ifdef DOT1X_SUPPORT
#define IS_IEEE8021X(_SecConfig)     ((_SecConfig)->IEEE8021X == TRUE)
#endif


/* Need to remove --START */
#define IS_AKM_1X_Entry(_Entry)     (IS_AKM_WPA1((_Entry)->SecConfig.AKMMap)  \
									 || IS_AKM_WPA2((_Entry)->SecConfig.AKMMap) \
									 || IS_AKM_WPA3((_Entry)->SecConfig.AKMMap))

#ifdef DOT1X_SUPPORT
#define IS_IEEE8021X_Entry(_Entry)     ((_Entry)->SecConfig.IEEE8021X == TRUE)
#endif

#define IS_AKM_WPA_CAPABILITY_Entry(_Entry)     (IS_AKM_WPA1((_Entry)->SecConfig.AKMMap)  \
		|| IS_AKM_WPA1PSK((_Entry)->SecConfig.AKMMap) \
		|| IS_AKM_WPANONE((_Entry)->SecConfig.AKMMap) \
		|| IS_AKM_WPA2((_Entry)->SecConfig.AKMMap) \
		|| IS_AKM_WPA2PSK((_Entry)->SecConfig.AKMMap) \
		|| IS_AKM_WPA2_SHA256((_Entry)->SecConfig.AKMMap) \
		|| IS_AKM_WPA2PSK_SHA256((_Entry)->SecConfig.AKMMap) \
		|| IS_AKM_WPA3((_Entry)->SecConfig.AKMMap) \
		|| IS_AKM_WPA3PSK((_Entry)->SecConfig.AKMMap) \
		|| IS_AKM_OWE((_Entry)->SecConfig.AKMMap))

#define IS_AKM_PSK_Entry(_Entry)     (IS_AKM_WPA1PSK((_Entry)->SecConfig.AKMMap)  \
					  || IS_AKM_WPA2PSK((_Entry)->SecConfig.AKMMap) \
					  || IS_AKM_WPA3PSK((_Entry)->SecConfig.AKMMap) \
					  || IS_AKM_OWE((_Entry)->SecConfig.AKMMap))


#ifdef DOT11R_FT_SUPPORT
#undef IS_AKM_PSK_Entry
#define IS_AKM_PSK_Entry(_Entry) (IS_AKM_WPA1PSK((_Entry)->SecConfig.AKMMap)  \
								  || IS_AKM_WPA2PSK((_Entry)->SecConfig.AKMMap)   \
								  || IS_AKM_FT_WPA2PSK((_Entry)->SecConfig.AKMMap) \
								  || IS_AKM_WPA3PSK((_Entry)->SecConfig.AKMMap) \
								  || IS_AKM_OWE((_Entry)->SecConfig.AKMMap))
#endif /* DOT11R_FT_SUPPORT */

#define IS_AKM_FT_WPAPSKWPA2PSK_Entry(_Entry)		(IS_AKM_WPA1PSK((_Entry)->SecConfig.AKMMap)  \
		&& IS_AKM_WPA2PSK((_Entry)->SecConfig.AKMMap))

#define IS_SECURITY_Entry(_Entry)    (IS_SECURITY(&(_Entry)->SecConfig))
#define IS_AKM_WPA1_Entry(_Entry)                          (IS_AKM_WPA1((_Entry)->SecConfig.AKMMap))
#define IS_AKM_WPA1PSK_Entry(_Entry)                          (IS_AKM_WPA1PSK((_Entry)->SecConfig.AKMMap))
#define IS_AKM_WPA2_Entry(_Entry)                          (IS_AKM_WPA2((_Entry)->SecConfig.AKMMap))
#define IS_AKM_WPA2PSK_Entry(_Entry)                          (IS_AKM_WPA2PSK((_Entry)->SecConfig.AKMMap))
#define IS_AKM_WPA3_Entry(_Entry)                          (IS_AKM_WPA3((_Entry)->SecConfig.AKMMap))
#define IS_AKM_WPA3PSK_Entry(_Entry)                          (IS_AKM_WPA3PSK((_Entry)->SecConfig.AKMMap))
#define IS_AKM_OWE_Entry(_Entry)                          (IS_AKM_OWE((_Entry)->SecConfig.AKMMap))
#define IS_CIPHER_WEP_Entry(_Entry)              (IS_CIPHER_WEP((_Entry)->SecConfig.PairwiseCipher))
#define IS_CIPHER_TKIP_Entry(_Entry)              (IS_CIPHER_TKIP((_Entry)->SecConfig.PairwiseCipher))
#define IS_CIPHER_AES_Entry(_Entry)              (IS_CIPHER_CCMP128((_Entry)->SecConfig.PairwiseCipher))

#define IS_CIPHER_NONE_OR_WEP_Entry(_Entry)  (IS_NO_SECURITY(&(_Entry)->SecConfig) || IS_CIPHER_WEP((_Entry)->SecConfig.PairwiseCipher))

#define IS_SECURITY_OPEN_NONE_Entry(_Entry)    (IS_NO_SECURITY(&(_Entry)->SecConfig))

#define IS_SECURITY_SHARED_WEP_Entry(_Entry)    (IS_AKM_SHARED((_Entry)->SecConfig.AKMMap)  && IS_CIPHER_WEP((_Entry)->SecConfig.PairwiseCipher))
#define IS_SECURITY_AUTOSWITCH_Entry(_Entry)    (IS_AKM_AUTOSWITCH((_Entry)->SecConfig.AKMMap)  && IS_CIPHER_WEP((_Entry)->SecConfig.PairwiseCipher))
#define IS_NO_SECURITY_Entry(_Entry)    (IS_AKM_OPEN((_Entry)->SecConfig.AKMMap) &&  IS_CIPHER_NONE((_Entry)->SecConfig.PairwiseCipher))

#define SET_AKM_OPEN_Entry(_Entry)               (SET_AKM_OPEN((_Entry)->SecConfig.AKMMap))
#define SET_CIPHER_NONE_Entry(_Entry)          (SET_CIPHER_NONE((_Entry)->SecConfig.PairwiseCipher))

/* Need to remove --END */

typedef enum _SEC_GROUP_REKEY_METHOD {
	SEC_GROUP_REKEY_DISABLE,
	SEC_GROUP_REKEY_TIME,
	SEC_GROUP_REKEY_PACKET,
	SEC_GROUP_REKEY_MAX /* Not a real mode, defined as upper bound */
} SEC_GROUP_REKEY_METHOD;

#define DEFAULT_GROUP_REKEY_INTERVAL   3600 /* one hour */
#define MAX_GROUP_REKEY_INTERVAL          0x3ffffff

typedef struct _HANDSHAKE_PROFILE {
	UCHAR AAddr[MAC_ADDR_LEN]; /* For nonce and key calculate */
	UCHAR SAddr[MAC_ADDR_LEN]; /* For nonce and key calculate */
	UCHAR ANonce[LEN_KEY_DESC_NONCE];
	UCHAR SNonce[LEN_KEY_DESC_NONCE];
	UCHAR GNonce[LEN_KEY_DESC_NONCE];
	UCHAR ReplayCounter[LEN_KEY_DESC_REPLAY];
	UINT8 WpaState;
	UINT8 GTKState;
	UCHAR MsgType; /*Record 4 way/2 way status for message retry judgement */
	UCHAR RSC[6];
	RALINK_TIMER_STRUCT MsgRetryTimer;
	UCHAR MsgRetryCounter;
	BOOLEAN AllowInsPTK;
} HANDSHAKE_PROFILE, *PHANDSHAKE_PROFILE;


typedef struct _SECURITY_CONFIG {
	UINT32 AKMMap;

	/* WEP Key */
	SEC_KEY_INFO WepKey[SEC_KEY_NUM];

	/* Pairwise Key */
	UINT32 PairwiseCipher;
	UCHAR PairwiseKeyId;
	UCHAR PSK[LEN_PSK + 1]; /* Add "\0" length */
	UCHAR PMK[LEN_MAX_PMK];
	UCHAR PTK[LEN_MAX_PTK]; /* 512 bits max, KCK(16)+KEK(16)+TK(32) */

	/* Group Key */
	UINT32 GroupCipher;
	UCHAR GroupKeyId;
	UCHAR GMK[LEN_GMK];
	UCHAR GTK[LEN_MAX_GTK];
	/* Group Key control parameter */
	SEC_GROUP_REKEY_METHOD GroupReKeyMethod;
	ULONG GroupReKeyInterval; /* time-based: seconds, packet-based: kilo-packets */
	ULONG GroupPacketCounter;
	UCHAR GroupReKeyInstallCountDown; /*unit: second, Install key after 2 way completed or 1 seconds */

	ULONG PMKCachePeriod;

	UINT8 key_deri_alg;

	/* WPA/WPA2 4way database */
	RALINK_TIMER_STRUCT StartFor4WayTimer;
	RALINK_TIMER_STRUCT StartFor2WayTimer;
	RALINK_TIMER_STRUCT GroupRekeyTimer;
	HANDSHAKE_PROFILE Handshake;

	/* Dirty code for repeater */
	UINT STARec_Bssid;

#ifdef DOT11W_PMF_SUPPORT
	PMF_CFG PmfCfg;
#endif /* DOT11W_PMF_SUPPORT */

#if defined(CONFIG_HOTSPOT) && defined(CONFIG_AP_SUPPORT)
	UCHAR HsUniGTK[LEN_MAX_GTK]; /* for storing HS DGAF uni GTK */
#endif /* defined(CONFIG_HOTSPOT) && defined(CONFIG_AP_SUPPORT) */

	/* 802.1x */
#ifdef DOT1X_SUPPORT
	UINT32 own_ip_addr;
	UINT32 own_radius_port;
	UINT32 retry_interval;
	UINT32 session_timeout_interval;
	UINT32 quiet_interval;
	UCHAR EAPifname[IFNAMSIZ];	/* indicate as the binding interface for EAP negotiation. */
	UCHAR EAPifname_len;
	UCHAR PreAuthifname[IFNAMSIZ];	/* indicate as the binding interface for WPA2 Pre-authentication. */
	UCHAR PreAuthifname_len;
	BOOLEAN PreAuth;
	UINT8 NasId[IFNAMSIZ];
	UINT8 NasIdLen;
	UCHAR radius_srv_num;
	RADIUS_SRV_INFO radius_srv_info[MAX_RADIUS_SRV_NUM];
#ifdef RADIUS_ACCOUNTING_SUPPORT
	UINT8 radius_acct_srv_num;
	RADIUS_SRV_INFO radius_acct_srv_info[MAX_RADIUS_SRV_NUM];
	/* int radius_request_cui; */
	int radius_acct_authentic;
	int acct_interim_interval;
	int acct_enable;
#endif /*RADIUS_ACCOUNTING_SUPPORT*/
#ifdef RADIUS_MAC_ACL_SUPPORT
	RT_802_11_RADIUS_ACL RadiusMacAuthCache;
	UINT32 RadiusMacAuthCacheTimeout;
#endif /* RADIUS_MAC_ACL_SUPPORT */
#endif /* DOT1X_SUPPORT */

	/* pmk cahce */
	UCHAR *pmkid;
	UCHAR *pmk_cache;

#if defined(DOT1X_SUPPORT) || defined(WPA_SUPPLICANT_SUPPORT)
	BOOLEAN IEEE8021X; /* Only indicate if we are running in dynamic WEP mode (WEP+802.1x) */
#endif
	BOOLEAN is_eapol_encrypted;
	/* IE for WPA1/WPA2/WAPI */
	SEC_RSNIE_TYPE RSNE_Type[SEC_RSNIE_NUM];
	UCHAR RSNE_EID[SEC_RSNIE_NUM][1];
	UCHAR RSNE_Len[SEC_RSNIE_NUM];
	UCHAR RSNE_Content[SEC_RSNIE_NUM][MAX_LEN_OF_RSNIE];
	UCHAR LastGroupKeyId;
	UCHAR LastGTK[LEN_MAX_GTK];
#ifdef CONFIG_OWE_SUPPORT
	OWE_INFO owe;
#endif /*CONFIG_OWE_SUPPORT*/
} SECURITY_CONFIG, *PSECURITY_CONFIG;


/*******************************************************
   Security support by feature and wireless mode
  *******************************************************/

#ifdef CONFIG_AP_SUPPORT
/* List AP support AKMs */
#define AKM_AP_MASK     ((1 << SEC_AKM_WPA1) \
						 | (1 << SEC_AKM_WPA1PSK) \
						 | (1 << SEC_AKM_WPA2) \
						 | (1 << SEC_AKM_WPA2PSK) \
						 | (1 << SEC_AKM_WAICERT) \
						 | (1 << SEC_AKM_WAIPSK) \
						 | (1 << SEC_AKM_SAE_SHA256) \
						 | (1 << SEC_AKM_SUITEB_SHA384) \
						 | (1 << SEC_AKM_OWE) \
						)
#endif /* CONFIG_AP_SUPPORT */








#ifdef APCLI_SUPPORT
/* List apcli support AKMs and Ciphers */
#define AKM_APCLI_MASK     ((1 << SEC_AKM_WPA1PSK) \
							| (1 << SEC_AKM_WPA2PSK))
#endif /* APCLI_SUPPORT */

#define MAX_PARAMETER_LEN  600 /* worse case: WEP128 for MBSS0~15 = (32+1)*16=528 */

#endif /* SEC_CMM_H */


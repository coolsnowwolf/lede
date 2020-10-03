#ifndef OWE_CMM_H
#define OWE_CMM_H

#ifdef CONFIG_OWE_SUPPORT
#include "sae_cmm.h"

#define ECDH_GROUP_256 19
#define ECDH_GROUP_384 20
#define ECDH_GROUP_521 21

#define OWE_TRANSITION_ORG_ID	0x506f9a
#define OWE_TRANSITION_ORG_ID_LEN	3

#define APCLI_MAX_SUPPORTED_OWE_GROUPS 2


#ifdef APCLI_OWE_SUPPORT
extern UCHAR apcli_owe_supp_groups[];
#endif

typedef struct GNU_PACKED _EXT_ECDH_PARAMETER_IE {
	UCHAR ext_ie_id;
	UCHAR length;
	UCHAR ext_id_ecdh;
	UINT16 group;
	UCHAR public_key[128];
} EXT_ECDH_PARAMETER_IE, *PEXT_ECDH_PARAMETER_IE;

typedef struct owe_info {
	VOID *group_info;
	VOID *group_info_bi;
	VOID *peer_pub_key;/*actually, BIG_INTEGER_EC_POINT*/
	VOID *pub_key;/*actually, BIG_INTEGER_EC_POINT*/
	VOID *generator;/*actually, BIG_INTEGER_EC_POINT*/
	SAE_BN *priv_key;
	UCHAR curr_group;
	UCHAR last_try_group;
	BOOLEAN inited;
	UCHAR *pmkid;
} OWE_INFO;

INT process_ecdh_element(struct _RTMP_ADAPTER *ad,
			 VOID *pEntry_v,
			 EXT_ECDH_PARAMETER_IE *ext_ie_ptr,
			 UCHAR ie_len,
			 UCHAR type,
			 BOOLEAN update_only_grp_info);

INT build_owe_dh_ie(struct _RTMP_ADAPTER *ad,
		    VOID *pEntry_v,
		    UCHAR *buf,
		    UCHAR group);

INT init_owe_group(OWE_INFO *owe, UCHAR group);

INT deinit_owe_group(OWE_INFO *owe);

INT owe_calculate_secret(OWE_INFO *owe, SAE_BN **secret);

#ifdef CONFIG_AP_SUPPORT
USHORT owe_pmkid_ecdh_process(struct _RTMP_ADAPTER *pAd,
			      VOID *pEntry_v,
			      UCHAR *rsn_ie,
			      UCHAR rsn_ie_len,
			      EXT_ECDH_PARAMETER_IE *ecdh_ie,
			      UCHAR ecdh_ie_length,
			      UINT8 *pmkid,
			      UINT8 *pmkid_count,
			      UCHAR type);
#endif

BOOLEAN extract_pair_owe_bss_info(UCHAR *owe_vendor_ie,
				  UCHAR owe_vendor_ie_len,
				  UCHAR *pair_bssid,
				  UCHAR *pair_ssid,
				  UCHAR *pair_ssid_len,
				  UCHAR *pair_band,
				  UCHAR *pair_ch);

extern UCHAR OWE_TRANS_OUI[];

void wext_send_owe_trans_chan_event(PNET_DEV net_dev,
				UCHAR event_id,
				UCHAR *pair_bssid,
				UCHAR *pair_ssid,
				UCHAR *pair_ssid_len,
				UCHAR *pair_band,
				UCHAR *pair_ch);

#endif /*CONFIG_OWE_SUPPORT*/
#endif /* OWE_CMM_H */


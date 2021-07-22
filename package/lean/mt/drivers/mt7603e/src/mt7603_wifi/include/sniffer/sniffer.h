/*
 ***************************************************************************
 * MediaTek Inc. 
 *
 * All rights reserved. source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

	Module Name:
	sniffer.h
*/

#ifndef __SNIFFER_H__
#define __SNIFFER_H__

#include "radiotap.h"

struct sniffer_control {
	UINT8 sniffer_type;
};

#define RADIOTAP_TYPE 0
#define PRISM_TYPE 1

enum {
	DIDmsg_lnxind_wlansniffrm		= 0x00000044,
	DIDmsg_lnxind_wlansniffrm_hosttime	= 0x00010044,
	DIDmsg_lnxind_wlansniffrm_mactime	= 0x00020044,
	DIDmsg_lnxind_wlansniffrm_channel	= 0x00030044,
	DIDmsg_lnxind_wlansniffrm_rssi		= 0x00040044,
	DIDmsg_lnxind_wlansniffrm_sq		= 0x00050044,
	DIDmsg_lnxind_wlansniffrm_signal	= 0x00060044,
	DIDmsg_lnxind_wlansniffrm_noise		= 0x00070044,
	DIDmsg_lnxind_wlansniffrm_rate		= 0x00080044,
	DIDmsg_lnxind_wlansniffrm_istx		= 0x00090044,
	DIDmsg_lnxind_wlansniffrm_frmlen	= 0x000A0044
};
enum { 
P80211ENUM_msgitem_status_no_value = 0x00 
};

enum { 
P80211ENUM_truth_false = 0x00, 
P80211ENUM_truth_true = 0x01 
};


/* Definition from madwifi */
typedef struct {
        UINT32 did;
        UINT16 status;
        UINT16 len;
        UINT32 data;
} p80211item_uint32_t;

typedef struct {
        UINT32 msgcode;
        UINT32 msglen;
#define WLAN_DEVNAMELEN_MAX 16
        UINT8 devname[WLAN_DEVNAMELEN_MAX];
        p80211item_uint32_t hosttime;
        p80211item_uint32_t mactime;
        p80211item_uint32_t channel;
        p80211item_uint32_t rssi;
        p80211item_uint32_t sq;
        p80211item_uint32_t signal;
        p80211item_uint32_t noise;
        p80211item_uint32_t rate;
        p80211item_uint32_t istx;
        p80211item_uint32_t frmlen;
} wlan_ng_prism2_header;

#ifdef MONITOR_FLAG_11N_SNIFFER_SUPPORT
/*
	Note: 2009/11/10
	Used in WiFi Sigma Test Engine RT3593 (replace RT2883).
*/
#ifdef RT_BIG_ENDIAN
typedef struct _ETHEREAL_RADIO {
        UCHAR Flag_80211n;
        UCHAR signal_level; /* dBm */
        UCHAR data_rate; /* rate index */
        UCHAR channel; /* Channel number */
} ETHEREAL_RADIO, *PETHEREAL_RADIO;
#else
typedef struct _ETHEREAL_RADIO {
        UCHAR channel; /* Channel number */
        UCHAR data_rate; /* rate index */
        UCHAR signal_level; /* dBm */
        UCHAR Flag_80211n;      
} ETHEREAL_RADIO, *PETHEREAL_RADIO;
#endif

#define WIRESHARK_11N_FLAG_3x3             	0x01
#define WIRESHARK_11N_FLAG_GF              	0x02
#define WIRESHARK_11N_FLAG_AMPDU     	 	0x04
#define WIRESHARK_11N_FLAG_STBC          	0x08
#define WIRESHARK_11N_FLAG_SGI             	0x10
#define WIRESHARK_11N_FLAG_BW20U      		0x20
#define WIRESHARK_11N_FLAG_BW20D      		0x40
#define WIRESHARK_11N_FLAG_BW40             0x80
#endif /* MONITOR_FLAG_11N_SNIFFER_SUPPORT */

struct mtk_radiotap_header {
	struct ieee80211_radiotap_header rt_hdr;
	UCHAR variable[0];
};

#define HT_BW(p) ((p) & IEEE80211_RADIOTAP_MCS_BW_MASK) 
#define HT_GI(p) ((p << 2) & IEEE80211_RADIOTAP_MCS_SGI) 
#define HT_FORMAT(p) ((p << 3) & IEEE80211_RADIOTAP_MCS_FMT_GF)
#define HT_FEC_TYPE(p) ((p << 4) & IEEE80211_RADIOTAP_MCS_FEC_LDPC)

#define VHT_MCS_MASK 0x0f
#define GET_VHT_MCS(p) (((p) & VHT_MCS_MASK))
#define VHT_NSS_MASK (0x03 << 4)
#define GET_VHT_NSS(p) ((((p) & VHT_NSS_MASK) >> 4) + 1)


// copy from dot11_base.h, here we duplicate this only for sniffer use!
/* 2-byte Frame control field */
typedef struct GNU_PACKED _FC_FIELD{
#ifdef RT_BIG_ENDIAN
	UINT16 Order:1;		/* Strict order expected */
	UINT16 Wep:1;		/* Wep data */
	UINT16 MoreData:1;	/* More data bit */
	UINT16 PwrMgmt:1;	/* Power management bit */
	UINT16 Retry:1;		/* Retry status bit */
	UINT16 MoreFrag:1;	/* More fragment bit */
	UINT16 FrDs:1;		/* From DS indication */
	UINT16 ToDs:1;		/* To DS indication */
	UINT16 SubType:4;	/* MSDU subtype */
	UINT16 Type:2;		/* MSDU type */
	UINT16 Ver:2;		/* Protocol version */
#else
        UINT16 Ver:2;		/* Protocol version */
	UINT16 Type:2;		/* MSDU type, refer to FC_TYPE_XX */
	UINT16 SubType:4;	/* MSDU subtype, refer to  SUBTYPE_XXX */
	UINT16 ToDs:1;		/* To DS indication */
	UINT16 FrDs:1;		/* From DS indication */
	UINT16 MoreFrag:1;	/* More fragment bit */
	UINT16 Retry:1;		/* Retry status bit */
	UINT16 PwrMgmt:1;	/* Power management bit */
	UINT16 MoreData:1;	/* More data bit */
	UINT16 Wep:1;		/* Wep data */
	UINT16 Order:1;		/* Strict order expected */
#endif	/* !RT_BIG_ENDIAN */
} FC_FIELD;

typedef struct GNU_PACKED _DOT_11_HDR {
        FC_FIELD   FC;
        UINT16          Duration;
        UCHAR           Addr1[6];
        UCHAR           Addr2[6];
	UCHAR		Addr3[6];
#ifdef RT_BIG_ENDIAN
	UINT16		Sequence:12;
	UINT16		Frag:4;
#else
	UINT16		Frag:4;
	UINT16		Sequence:12;
#endif /* !RT_BIG_ENDIAN */
	UCHAR		Octet[0];
}DOT_11_HDR;


#endif

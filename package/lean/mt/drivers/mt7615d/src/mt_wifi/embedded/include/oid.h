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
	oid.h

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
*/
#ifndef _OID_H_
#define _OID_H_

/*#include <linux/wireless.h> */



/* new types for Media Specific Indications */
/* Extension channel offset */
#define EXTCHA_NONE			0
#define EXTCHA_ABOVE		0x1
#define EXTCHA_BELOW		0x3
#define EXTCHA_NOASSIGN		0xf

/* BW */
#define BAND_WIDTH_20		0
#define BAND_WIDTH_40		1
#define BAND_WIDTH_80		2
#define BAND_WIDTH_160		3
#define BAND_WIDTH_10		4	/* 802.11j has 10MHz. This definition is for internal usage. doesn't fill in the IE or other field. */
#define BAND_WIDTH_5		5
#define BAND_WIDTH_8080		6
#define BAND_WIDTH_BOTH		7	/* BW20 + BW40 */
#define BAND_WIDTH_25		8
#define BAND_WIDTH_NUM		9

/* SHORTGI */
#define GAP_INTERVAL_400	1	/* only support in HT mode */
#define GAP_INTERVAL_800	0
#define GAP_INTERVAL_BOTH	2

#define NdisMediaStateConnected			1
#define NdisMediaStateDisconnected		0

#define NdisApMediaStateConnected			1
#define NdisApMediaStateDisconnected		0


#define NDIS_802_11_LENGTH_SSID         32

#define MAC_ADDR_LEN			6
#define IEEE80211_ADDR_LEN		6	/* size of 802.11 address */
#define IEEE80211_NWID_LEN		32

#define NDIS_802_11_LENGTH_RATES        8
#define NDIS_802_11_LENGTH_RATES_EX     16

#define OID_P2P_DEVICE_NAME_LEN	32
/*#define MAX_NUM_OF_CHS					49 */ /* 14 channels @2.4G +  12@UNII + 4 @MMAC + 11 @HiperLAN2 + 7 @Japan + 1 as NULL terminationc */
/*#define MAX_NUM_OF_CHS				54 */ /* 14 channels @2.4G +  12@UNII(lower/middle) + 16@HiperLAN2 + 11@UNII(upper) + 0 @Japan + 1 as NULL termination */
#define MAX_NUMBER_OF_EVENT				10	/* entry # in EVENT table */

#ifdef BB_SOC
#define MAX_NUMBER_OF_MAC				8 /* if MAX_MBSSID_NUM is 8, this value can't be larger than 211 */
#else
#if defined(MT7603_FPGA) || defined(MT7628_FPGA) || defined(MT7636_FPGA) || defined(MT7637_FPGA)
#define MAX_NUMBER_OF_MAC				4	/* if MAX_MBSSID_NUM is 8, this value can't be larger than 211 */
#elif defined(MT7603) || defined(MT7628)
#define MAX_NUMBER_OF_MAC				33
#elif defined(MT7636)
#define MAX_NUMBER_OF_MAC				14
#elif defined(MT7637)
#define MAX_NUMBER_OF_MAC				75 /*sync with 7615 fixed value*/
#elif defined(MT76x2)
#ifdef MAC_REPEATER_SUPPORT				/* ((MAX_EXT_MAC_ADDR_SIZE + 1) * MAC_APCLI_NUM) */
#define MAX_NUMBER_OF_MAC               (116 - ((16 + 1) * 1))
#else
#define MAX_NUMBER_OF_MAC				 116
#endif /* MAC_REPEATER_SUPPORT */
#else
/*
    chip max wtbl idx = 128,
    as AP role, need a group key entry. so max it will occupy 16 for 16 mbss.

    max rept entry = 32,
    rept need apcli link, 1 for unicast, 1 for broadcast key from rootap.

    so 128 - 32 - 16 - 1 - 1  = 78.
*/
#define MAX_NUMBER_OF_MAC				75
#endif /* defined(MT7603_FPGA) || defined(MT7628_FPGA) */
#endif /* BB_SOC */

#define MAX_NUMBER_OF_ACL				128
#define MAX_LENGTH_OF_SUPPORT_RATES		12	/* 1, 2, 5.5, 11, 6, 9, 12, 18, 24, 36, 48, 54 */
#define MAX_NUMBER_OF_DLS_ENTRY			4


#define RT_QUERY_SIGNAL_CONTEXT				0x0402
#define RT_SET_IAPP_PID						0x0404
#define RT_SET_APD_PID						0x0405
#define RT_SET_DEL_MAC_ENTRY				0x0406
#define RT_QUERY_EVENT_TABLE				0x0407
#ifdef DOT11R_FT_SUPPORT
#define RT_SET_FT_STATION_NOTIFY			0x0408
#define RT_SET_FT_KEY_REQ					0x0409
#define RT_SET_FT_KEY_RSP					0x040a
#define RT_FT_KEY_SET						0x040b
#define RT_FT_DATA_ENCRYPT					0x040c
#define RT_FT_DATA_DECRYPT					0x040d
#define RT_FT_NEIGHBOR_REPORT				0x040e
#define RT_FT_NEIGHBOR_REQUEST				0x040f
#define RT_FT_NEIGHBOR_RESPONSE				0x0410
#define RT_FT_ACTION_FORWARD				0x0411
#endif /* DOT11R_FT_SUPPORT */
/* */
/* IEEE 802.11 OIDs */
/* */
#define	OID_GET_SET_TOGGLE			0x8000
#define	OID_GET_SET_FROM_UI			0x4000

#define	OID_802_11_NETWORK_TYPES_SUPPORTED			0x0103
#define	OID_802_11_NETWORK_TYPE_IN_USE				0x0104
#define	OID_802_11_RSSI_TRIGGER						0x0107
#define	RT_OID_802_11_RSSI							0x0108	/* rt2860 only */
#define	RT_OID_802_11_RSSI_1						0x0109	/* rt2860 only */
#define	RT_OID_802_11_RSSI_2						0x010A	/* rt2860 only */
#define	OID_802_11_NUMBER_OF_ANTENNAS				0x010B
#define	OID_802_11_RX_ANTENNA_SELECTED				0x010C
#define	OID_802_11_TX_ANTENNA_SELECTED				0x010D
#define	OID_802_11_SUPPORTED_RATES					0x010E
#define	OID_802_11_ADD_WEP							0x0112
#define	OID_802_11_REMOVE_WEP						0x0113
#define	OID_802_11_DISASSOCIATE						0x0114
#define	OID_802_11_PRIVACY_FILTER					0x0118
#define	OID_802_11_ASSOCIATION_INFORMATION			0x011E
#define	OID_802_11_TEST								0x011F


#define	RT_OID_802_11_COUNTRY_REGION				0x0507
#define	OID_802_11_BSSID_LIST_SCAN					0x0508
#define	OID_802_11_SSID								0x0509
#define	OID_802_11_BSSID							0x050A
#define	RT_OID_802_11_RADIO							0x050B
#define	RT_OID_802_11_PHY_MODE						0x050C
#define	RT_OID_802_11_STA_CONFIG					0x050D
#define	OID_802_11_DESIRED_RATES					0x050E
#define	RT_OID_802_11_PREAMBLE						0x050F
#define	OID_802_11_WEP_STATUS						0x0510
#define	OID_802_11_AUTHENTICATION_MODE				0x0511
#define	OID_802_11_INFRASTRUCTURE_MODE				0x0512
#define	RT_OID_802_11_RESET_COUNTERS				0x0513
#define	OID_802_11_RTS_THRESHOLD					0x0514
#define	OID_802_11_FRAGMENTATION_THRESHOLD			0x0515
#define	OID_802_11_POWER_MODE						0x0516
#define	OID_802_11_TX_POWER_LEVEL					0x0517
#define	RT_OID_802_11_ADD_WPA						0x0518
#define	OID_802_11_REMOVE_KEY						0x0519
#define	RT_OID_802_11_QUERY_PID						0x051A
#define	RT_OID_802_11_QUERY_VID						0x051B
#define	OID_802_11_ADD_KEY							0x0520
#define	OID_802_11_CONFIGURATION					0x0521
#define	OID_802_11_TX_PACKET_BURST					0x0522
#define	RT_OID_802_11_QUERY_NOISE_LEVEL				0x0523
#define	RT_OID_802_11_EXTRA_INFO					0x0524
#define	RT_OID_802_11_HARDWARE_REGISTER				0x0525
#define OID_802_11_ENCRYPTION_STATUS            OID_802_11_WEP_STATUS
#define OID_802_11_DEAUTHENTICATION                 0x0526
#define OID_802_11_DROP_UNENCRYPTED                 0x0527
#define OID_802_11_MIC_FAILURE_REPORT_FRAME         0x0528
#define OID_802_11_EAP_METHOD						0x0529
#define OID_802_11_ACL_LIST							0x052A
#define OID_802_11_ACL_ADD_ENTRY					0x052B
#define OID_802_11_ACL_DEL_ENTRY					0x052C
#define OID_802_11_ACL_SET_POLICY					0x052D

#ifdef VENDOR_FEATURE6_SUPPORT
#define OID_802_11_HT_STBC							0x052E
#define OID_802_11_UAPSD							0x052F
#define OID_802_11_AMSDU							0x0531
#define OID_802_11_AMPDU							0x0532
#define OID_802_11_APCFG							0x0533
#define OID_802_11_ASSOLIST							0x0534
#define OID_802_11_CURRENT_CRED 					0x0535
#define OID_802_11_PASSPHRASES					0x0536
#define OID_802_11_CHANNEL_WIDTH					0x0537
#define OID_802_11_BEACON_PERIOD					0x0538
#endif /* VENDOR_FEATURE6_SUPPORT */
#if (defined(VENDOR_FEATURE6_SUPPORT) || defined(CONFIG_MAP_SUPPORT))
#define OID_802_11_COEXISTENCE						0x0530
#endif
#ifdef AIR_MONITOR
#define OID_GET_AIR_MONITOR_RESULT					0x1802
#endif

/* For 802.1x daemin using */
#ifdef DOT1X_SUPPORT
#define OID_802_DOT1X_CONFIGURATION					0x0540
#define OID_802_DOT1X_PMKID_CACHE					0x0541
#define OID_802_DOT1X_RADIUS_DATA					0x0542
#define OID_802_DOT1X_WPA_KEY						0x0543
#define OID_802_DOT1X_STATIC_WEP_COPY				0x0544
#define OID_802_DOT1X_IDLE_TIMEOUT					0x0545
#define OID_802_DOT1X_RADIUS_ACL_NEW_CACHE          0x0546
#define OID_802_DOT1X_RADIUS_ACL_DEL_CACHE          0x0547
#define OID_802_DOT1X_RADIUS_ACL_CLEAR_CACHE        0x0548
#define OID_802_DOT1X_QUERY_STA_AID                 0x0549
#ifdef RADIUS_ACCOUNTING_SUPPORT
#define OID_802_DOT1X_QUERY_STA_DATA                 0x0550
#endif /*RADIUS_ACCOUNTING_SUPPORT*/
#define OID_802_DOT1X_QUERY_STA_RSN                 0x0551
#endif /* DOT1X_SUPPORT */

#define	RT_OID_DEVICE_NAME							0x0607
#define	RT_OID_VERSION_INFO							0x0608
#define	OID_802_11_BSSID_LIST						0x0609
#define	OID_802_3_CURRENT_ADDRESS					0x060A
#define	OID_GEN_MEDIA_CONNECT_STATUS				0x060B
#define	RT_OID_802_11_QUERY_LINK_STATUS				0x060C
#define	OID_802_11_RSSI								0x060D
#define	OID_802_11_STATISTICS						0x060E
#define	OID_GEN_RCV_OK								0x060F
#define	OID_GEN_RCV_NO_BUFFER						0x0610
#define	RT_OID_802_11_QUERY_EEPROM_VERSION			0x0611
#define	RT_OID_802_11_QUERY_FIRMWARE_VERSION		0x0612
#define	RT_OID_802_11_QUERY_LAST_RX_RATE			0x0613
#define	RT_OID_802_11_TX_POWER_LEVEL_1				0x0614
#define	RT_OID_802_11_QUERY_PIDVID					0x0615
/*for WPA_SUPPLICANT_SUPPORT */
#define OID_SET_COUNTERMEASURES                     0x0616
#define OID_802_11_SET_IEEE8021X                    0x0617
#define OID_802_11_SET_IEEE8021X_REQUIRE_KEY        0x0618
#define OID_802_11_PMKID                            0x0620
#define RT_OID_WPA_SUPPLICANT_SUPPORT               0x0621
#define RT_OID_WE_VERSION_COMPILED                  0x0622
#define RT_OID_NEW_DRIVER                           0x0623
#define	OID_AUTO_PROVISION_BSSID_LIST				0x0624
#define RT_OID_WPS_PROBE_REQ_IE						0x0625

#define	RT_OID_802_11_SNR_0							0x0630
#define	RT_OID_802_11_SNR_1							0x0631
#define	RT_OID_802_11_QUERY_LAST_TX_RATE			0x0632
#define	RT_OID_802_11_QUERY_HT_PHYMODE				0x0633
#define	RT_OID_802_11_SET_HT_PHYMODE				0x0634
#define	OID_802_11_RELOAD_DEFAULTS					0x0635
#define	RT_OID_802_11_QUERY_APSD_SETTING			0x0636
#define	RT_OID_802_11_SET_APSD_SETTING				0x0637
#define	RT_OID_802_11_QUERY_APSD_PSM				0x0638
#define	RT_OID_802_11_SET_APSD_PSM					0x0639
#define	RT_OID_802_11_QUERY_DLS						0x063A
#define	RT_OID_802_11_SET_DLS						0x063B
#define	RT_OID_802_11_QUERY_DLS_PARAM				0x063C
#define	RT_OID_802_11_SET_DLS_PARAM					0x063D
#define RT_OID_802_11_QUERY_WMM						0x063E
#define RT_OID_802_11_SET_WMM						0x063F
#define RT_OID_802_11_QUERY_IMME_BA_CAP				0x0640
#define RT_OID_802_11_SET_IMME_BA_CAP				0x0641
#define RT_OID_802_11_QUERY_BATABLE					0x0642
#define RT_OID_802_11_ADD_IMME_BA					0x0643
#define RT_OID_802_11_TEAR_IMME_BA					0x0644
#define RT_OID_DRIVER_DEVICE_NAME                   0x0645
#define RT_OID_802_11_QUERY_DAT_HT_PHYMODE          0x0646
#define OID_WAPP_EVENT                              0x0647
#define OID_802_11_SET_PSPXLINK_MODE				0x0648
/*+++ add by woody +++*/
#define OID_802_11_SET_PASSPHRASE					0x0649
#define RT_OID_802_11_QUERY_TX_PHYMODE                          0x0650
#define RT_OID_802_11_QUERY_MAP_REAL_TX_RATE                          0x0678
#define RT_OID_802_11_QUERY_MAP_REAL_RX_RATE                          0x0679
#define	RT_OID_802_11_SNR_2							0x067A
#ifdef TXBF_SUPPORT
#define RT_OID_802_11_QUERY_TXBF_TABLE				0x067C
#endif
#define RT_OID_802_11_PER_BSS_STATISTICS			0x067D

#define OID_802_11_VENDOR_IE_ADD				0x067E
#define OID_802_11_VENDOR_IE_UPDATE				0x067F
#define OID_802_11_VENDOR_IE_REMOVE				0x0680
#define OID_802_11_VENDOR_IE_SHOW				0x0681

#ifdef ACL_BLK_COUNT_SUPPORT
#define OID_802_11_ACL_BLK_REJCT_COUNT_STATICS		0x069b
#endif/*ACL_BLK_COUNT_SUPPORT*/

#ifdef RTMP_RBUS_SUPPORT
#define OID_802_11_QUERY_WirelessMode				0x0718
#endif /* RTMP_RBUS_SUPPORT */

#ifdef CUSTOMER_VENDOR_IE_SUPPORT
/*vendor ie oid: 0x1200~0x12ff*/
#define OID_VENDOR_IE_BASE		0x1200
enum vendor_ie_subcmd_oid {
	OID_SUBCMD_AP_VENDOR_IE_SET,
	OID_SUBCMD_AP_VENDOR_IE_DEL,

	NUM_OID_SUBCMD_VENDOR_IE,
	MAX_NUM_OID_SUBCMD_VENDOR_IE = NUM_OID_SUBCMD_VENDOR_IE - 1
};

#define OID_AP_VENDOR_IE_SET		(OID_VENDOR_IE_BASE | OID_SUBCMD_AP_VENDOR_IE_SET)/*0x1200*/
#define OID_AP_VENDOR_IE_DEL		(OID_VENDOR_IE_BASE | OID_SUBCMD_AP_VENDOR_IE_DEL)
#define RT_OID_AP_VENDOR_IE_SET		(OID_GET_SET_TOGGLE | OID_AP_VENDOR_IE_SET)/*0x9200*/
#define RT_OID_AP_VENDOR_IE_DEL		(OID_GET_SET_TOGGLE | OID_AP_VENDOR_IE_DEL)

#define OID_SET_OUI_FILTER			0x1220/**/
#define RT_OID_SET_OUI_FILTER		(OID_GET_SET_TOGGLE | OID_SET_OUI_FILTER)/*0x9220*/

#define RT_PROBE_REQ_REPORT_EVENT	0x1700

#endif /* CUSTOMER_VENDOR_IE_SUPPORT */

#ifdef HOSTAPD_SUPPORT
#define SIOCSIWGENIE	0x8B30
#define OID_HOSTAPD_SUPPORT               0x0661

#define HOSTAPD_OID_STATIC_WEP_COPY   0x0662
#define HOSTAPD_OID_GET_1X_GROUP_KEY   0x0663

#define HOSTAPD_OID_SET_STA_AUTHORIZED   0x0664
#define HOSTAPD_OID_SET_STA_DISASSOC   0x0665
#define HOSTAPD_OID_SET_STA_DEAUTH   0x0666
#define HOSTAPD_OID_DEL_KEY   0x0667
#define HOSTAPD_OID_SET_KEY   0x0668
#define HOSTAPD_OID_SET_802_1X   0x0669
#define HOSTAPD_OID_GET_SEQ   0x0670
#define HOSTAPD_OID_GETWPAIE                 0x0671
#define HOSTAPD_OID_COUNTERMEASURES 0x0672
#define HOSTAPD_OID_SET_WPAPSK 0x0673
#define HOSTAPD_OID_SET_WPS_BEACON_IE 0x0674
#define HOSTAPD_OID_SET_WPS_PROBE_RESP_IE 0x0675

#define	RT_HOSTAPD_OID_HOSTAPD_SUPPORT				(OID_GET_SET_TOGGLE |	OID_HOSTAPD_SUPPORT)
#define	RT_HOSTAPD_OID_STATIC_WEP_COPY				(OID_GET_SET_TOGGLE |	HOSTAPD_OID_STATIC_WEP_COPY)
#define	RT_HOSTAPD_OID_GET_1X_GROUP_KEY				(OID_GET_SET_TOGGLE |	HOSTAPD_OID_GET_1X_GROUP_KEY)
#define	RT_HOSTAPD_OID_SET_STA_AUTHORIZED			(OID_GET_SET_TOGGLE |	HOSTAPD_OID_SET_STA_AUTHORIZED)
#define	RT_HOSTAPD_OID_SET_STA_DISASSOC				(OID_GET_SET_TOGGLE |	HOSTAPD_OID_SET_STA_DISASSOC)
#define	RT_HOSTAPD_OID_SET_STA_DEAUTH				(OID_GET_SET_TOGGLE |	HOSTAPD_OID_SET_STA_DEAUTH)
#define	RT_HOSTAPD_OID_DEL_KEY						(OID_GET_SET_TOGGLE |	HOSTAPD_OID_DEL_KEY)
#define	RT_HOSTAPD_OID_SET_KEY						(OID_GET_SET_TOGGLE |	HOSTAPD_OID_SET_KEY)
#define	RT_HOSTAPD_OID_SET_802_1X						(OID_GET_SET_TOGGLE |	HOSTAPD_OID_SET_802_1X)
#define	RT_HOSTAPD_OID_COUNTERMEASURES				(OID_GET_SET_TOGGLE |	HOSTAPD_OID_COUNTERMEASURES)
#define	RT_HOSTAPD_OID_SET_WPAPSK				(OID_GET_SET_TOGGLE |	HOSTAPD_OID_SET_WPAPSK)
#define	RT_HOSTAPD_OID_SET_WPS_BEACON_IE				(OID_GET_SET_TOGGLE |	HOSTAPD_OID_SET_WPS_BEACON_IE)
#define	RT_HOSTAPD_OID_SET_WPS_PROBE_RESP_IE				(OID_GET_SET_TOGGLE |	HOSTAPD_OID_SET_WPS_PROBE_RESP_IE)

#define IEEE80211_IS_MULTICAST(_a) (*(_a) & 0x01)
#define	IEEE80211_KEYBUF_SIZE	16
#define	IEEE80211_MICBUF_SIZE	(8 + 8)	/* space for both tx+rx keys */
#define IEEE80211_TID_SIZE		17	/* total number of TIDs */

#define	IEEE80211_MLME_ASSOC		    1	/* associate station */
#define	IEEE80211_MLME_DISASSOC		    2	/* disassociate station */
#define	IEEE80211_MLME_DEAUTH		    3	/* deauthenticate station */
#define	IEEE80211_MLME_AUTHORIZE	    4	/* authorize station */
#define	IEEE80211_MLME_UNAUTHORIZE	    5	/* unauthorize station */
#define IEEE80211_MLME_CLEAR_STATS	    6	/* clear station statistic */
#define IEEE80211_1X_COPY_KEY			7	/* copy static-wep unicast key */

#define	IEEE80211_MAX_OPT_IE	256
#define IWEVEXPIRED	0x8C04

struct ieee80211req_mlme {
	UINT8 im_op;		/* operation to perform */
	UINT8 im_ssid_len;	/* length of optional ssid */
	UINT16 im_reason;	/* 802.11 reason code */
	UINT8 im_macaddr[IEEE80211_ADDR_LEN];
	UINT8 im_ssid[IEEE80211_NWID_LEN];
};

struct ieee80211req_key {
	UINT8 ik_type;		/* key/cipher type */
	UINT8 ik_pad;
	UINT16 ik_keyix;	/* key index */
	UINT8 ik_keylen;	/* key length in bytes */
	UINT8 ik_flags;
	UINT8 ik_macaddr[IEEE80211_ADDR_LEN];
	UINT64 ik_keyrsc;	/* key receive sequence counter */
	UINT64 ik_keytsc;	/* key transmit sequence counter */
	UINT8 ik_keydata[IEEE80211_KEYBUF_SIZE + IEEE80211_MICBUF_SIZE];
	int txkey;
};

struct ieee80211req_del_key {
	UINT8 idk_keyix;	/* key index */
	UINT8 idk_macaddr[IEEE80211_ADDR_LEN];
};

struct default_group_key {
	UINT16 ik_keyix;	/* key index */
	UINT8 ik_keylen;	/* key length in bytes */
	UINT8 ik_keydata[IEEE80211_KEYBUF_SIZE + IEEE80211_MICBUF_SIZE];
};

struct ieee80211req_wpaie {
	UINT8 wpa_macaddr[IEEE80211_ADDR_LEN];
	UINT8 rsn_ie[IEEE80211_MAX_OPT_IE];
};

struct hostapd_wpa_psk {
	struct hostapd_wpa_psk *next;
	int group;
	UCHAR psk[32];
	UCHAR addr[6];
};

#endif /*HOSTAPD_SUPPORT */

#define RT_OID_802_11_QUERY_TDLS_PARAM			0x0676
#define	RT_OID_802_11_QUERY_TDLS				0x0677

/* Ralink defined OIDs */
/* Dennis Lee move to platform specific */

#define	RT_OID_802_11_BSSID					  (OID_GET_SET_TOGGLE |	OID_802_11_BSSID)
#define	RT_OID_802_11_SSID					  (OID_GET_SET_TOGGLE |	OID_802_11_SSID)
#define	RT_OID_802_11_INFRASTRUCTURE_MODE	  (OID_GET_SET_TOGGLE |	OID_802_11_INFRASTRUCTURE_MODE)
#define	RT_OID_802_11_ADD_WEP				  (OID_GET_SET_TOGGLE |	OID_802_11_ADD_WEP)
#define	RT_OID_802_11_ADD_KEY				  (OID_GET_SET_TOGGLE |	OID_802_11_ADD_KEY)
#define	RT_OID_802_11_REMOVE_WEP			  (OID_GET_SET_TOGGLE |	OID_802_11_REMOVE_WEP)
#define	RT_OID_802_11_REMOVE_KEY			  (OID_GET_SET_TOGGLE |	OID_802_11_REMOVE_KEY)
#define	RT_OID_802_11_DISASSOCIATE			  (OID_GET_SET_TOGGLE |	OID_802_11_DISASSOCIATE)
#define	RT_OID_802_11_AUTHENTICATION_MODE	  (OID_GET_SET_TOGGLE |	OID_802_11_AUTHENTICATION_MODE)
#define	RT_OID_802_11_PRIVACY_FILTER		  (OID_GET_SET_TOGGLE |	OID_802_11_PRIVACY_FILTER)
#define	RT_OID_802_11_BSSID_LIST_SCAN		  (OID_GET_SET_TOGGLE |	OID_802_11_BSSID_LIST_SCAN)
#define	RT_OID_802_11_WEP_STATUS			  (OID_GET_SET_TOGGLE |	OID_802_11_WEP_STATUS)
#define	RT_OID_802_11_RELOAD_DEFAULTS		  (OID_GET_SET_TOGGLE |	OID_802_11_RELOAD_DEFAULTS)
#define	RT_OID_802_11_NETWORK_TYPE_IN_USE	  (OID_GET_SET_TOGGLE |	OID_802_11_NETWORK_TYPE_IN_USE)
#define	RT_OID_802_11_TX_POWER_LEVEL		  (OID_GET_SET_TOGGLE |	OID_802_11_TX_POWER_LEVEL)
#define	RT_OID_802_11_RSSI_TRIGGER			  (OID_GET_SET_TOGGLE |	OID_802_11_RSSI_TRIGGER)
#define	RT_OID_802_11_FRAGMENTATION_THRESHOLD (OID_GET_SET_TOGGLE |	OID_802_11_FRAGMENTATION_THRESHOLD)
#define	RT_OID_802_11_RTS_THRESHOLD			  (OID_GET_SET_TOGGLE |	OID_802_11_RTS_THRESHOLD)
#define	RT_OID_802_11_RX_ANTENNA_SELECTED	  (OID_GET_SET_TOGGLE |	OID_802_11_RX_ANTENNA_SELECTED)
#define	RT_OID_802_11_TX_ANTENNA_SELECTED	  (OID_GET_SET_TOGGLE |	OID_802_11_TX_ANTENNA_SELECTED)
#define	RT_OID_802_11_SUPPORTED_RATES		  (OID_GET_SET_TOGGLE |	OID_802_11_SUPPORTED_RATES)
#define	RT_OID_802_11_DESIRED_RATES			  (OID_GET_SET_TOGGLE |	OID_802_11_DESIRED_RATES)
#define	RT_OID_802_11_CONFIGURATION			  (OID_GET_SET_TOGGLE |	OID_802_11_CONFIGURATION)
#define	RT_OID_802_11_POWER_MODE			  (OID_GET_SET_TOGGLE |	OID_802_11_POWER_MODE)
#define RT_OID_802_11_SET_PSPXLINK_MODE		  (OID_GET_SET_TOGGLE |	OID_802_11_SET_PSPXLINK_MODE)
#define RT_OID_802_11_EAP_METHOD			  (OID_GET_SET_TOGGLE | OID_802_11_EAP_METHOD)
#define RT_OID_802_11_SET_PASSPHRASE		  (OID_GET_SET_TOGGLE | OID_802_11_SET_PASSPHRASE)

#ifdef DOT1X_SUPPORT
#define RT_OID_802_DOT1X_PMKID_CACHE		(OID_GET_SET_TOGGLE | OID_802_DOT1X_PMKID_CACHE)
#define RT_OID_802_DOT1X_RADIUS_DATA		(OID_GET_SET_TOGGLE | OID_802_DOT1X_RADIUS_DATA)
#define RT_OID_802_DOT1X_WPA_KEY			(OID_GET_SET_TOGGLE | OID_802_DOT1X_WPA_KEY)
#define RT_OID_802_DOT1X_STATIC_WEP_COPY	(OID_GET_SET_TOGGLE | OID_802_DOT1X_STATIC_WEP_COPY)
#define RT_OID_802_DOT1X_IDLE_TIMEOUT		(OID_GET_SET_TOGGLE | OID_802_DOT1X_IDLE_TIMEOUT)
#endif /* DOT1X_SUPPORT */

#define RT_OID_802_11_SET_TDLS_PARAM			(OID_GET_SET_TOGGLE | RT_OID_802_11_QUERY_TDLS_PARAM)
#define RT_OID_802_11_SET_TDLS				(OID_GET_SET_TOGGLE | RT_OID_802_11_QUERY_TDLS)


typedef enum _NDIS_802_11_STATUS_TYPE {
	Ndis802_11StatusType_Authentication,
	Ndis802_11StatusType_MediaStreamMode,
	Ndis802_11StatusType_PMKID_CandidateList,
	Ndis802_11StatusTypeMax	/* not a real type, defined as an upper bound */
} NDIS_802_11_STATUS_TYPE, *PNDIS_802_11_STATUS_TYPE;

typedef UCHAR NDIS_802_11_MAC_ADDRESS[6];

typedef struct _NDIS_802_11_STATUS_INDICATION {
	NDIS_802_11_STATUS_TYPE StatusType;
} NDIS_802_11_STATUS_INDICATION, *PNDIS_802_11_STATUS_INDICATION;

/* mask for authentication/integrity fields */
#define NDIS_802_11_AUTH_REQUEST_AUTH_FIELDS        0x0f

#define NDIS_802_11_AUTH_REQUEST_REAUTH             0x01
#define NDIS_802_11_AUTH_REQUEST_KEYUPDATE          0x02
#define NDIS_802_11_AUTH_REQUEST_PAIRWISE_ERROR     0x06
#define NDIS_802_11_AUTH_REQUEST_GROUP_ERROR        0x0E

typedef struct _NDIS_802_11_AUTHENTICATION_REQUEST {
	ULONG Length;		/* Length of structure */
	NDIS_802_11_MAC_ADDRESS Bssid;
	ULONG Flags;
} NDIS_802_11_AUTHENTICATION_REQUEST, *PNDIS_802_11_AUTHENTICATION_REQUEST;

/*Added new types for PMKID Candidate lists. */
typedef struct _PMKID_CANDIDATE {
	NDIS_802_11_MAC_ADDRESS BSSID;
	ULONG Flags;
} PMKID_CANDIDATE, *PPMKID_CANDIDATE;

typedef struct _NDIS_802_11_PMKID_CANDIDATE_LIST {
	ULONG Version;		/* Version of the structure */
	ULONG NumCandidates;	/* No. of pmkid candidates */
	PMKID_CANDIDATE CandidateList[1];
} NDIS_802_11_PMKID_CANDIDATE_LIST, *PNDIS_802_11_PMKID_CANDIDATE_LIST;

/*Flags for PMKID Candidate list structure */
#define NDIS_802_11_PMKID_CANDIDATE_PREAUTH_ENABLED	0x01

/* Added new types for OFDM 5G and 2.4G */
typedef enum _NDIS_802_11_NETWORK_TYPE {
	Ndis802_11FH,
	Ndis802_11DS,
	Ndis802_11OFDM5,
	Ndis802_11OFDM24,
	Ndis802_11Automode,
	Ndis802_11OFDM5_N,
	Ndis802_11OFDM24_N,
	Ndis802_11OFDM5_AC,
	Ndis802_11NetworkTypeMax	/* not a real type, defined as an upper bound */
} NDIS_802_11_NETWORK_TYPE, *PNDIS_802_11_NETWORK_TYPE;

typedef struct _NDIS_802_11_NETWORK_TYPE_LIST {
	UINT NumberOfItems;	/* in list below, at least 1 */
	NDIS_802_11_NETWORK_TYPE NetworkType[1];
} NDIS_802_11_NETWORK_TYPE_LIST, *PNDIS_802_11_NETWORK_TYPE_LIST;

typedef enum _NDIS_802_11_POWER_MODE {
	Ndis802_11PowerModeCAM,
	Ndis802_11PowerModeMAX_PSP,
	Ndis802_11PowerModeFast_PSP,
	Ndis802_11PowerModeLegacy_PSP,
	Ndis802_11PowerModeMax	/* not a real mode, defined as an upper bound */
} NDIS_802_11_POWER_MODE, *PNDIS_802_11_POWER_MODE;

typedef ULONG NDIS_802_11_TX_POWER_LEVEL;	/* in milliwatts */

/* */
/* Received Signal Strength Indication */
/* */
typedef LONG NDIS_802_11_RSSI;	/* in dBm */

typedef struct _NDIS_802_11_CONFIGURATION_FH {
	ULONG Length;		/* Length of structure */
	ULONG HopPattern;	/* As defined by 802.11, MSB set */
	ULONG HopSet;		/* to one if non-802.11 */
	ULONG DwellTime;	/* units are Kusec */
} NDIS_802_11_CONFIGURATION_FH, *PNDIS_802_11_CONFIGURATION_FH;

typedef struct _NDIS_802_11_CONFIGURATION {
	ULONG Length;		/* Length of structure */
	ULONG BeaconPeriod;	/* units are Kusec */
	ULONG ATIMWindow;	/* units are Kusec */
	ULONG DSConfig;		/* Frequency, units are kHz */
	NDIS_802_11_CONFIGURATION_FH FHConfig;
} NDIS_802_11_CONFIGURATION, *PNDIS_802_11_CONFIGURATION;

typedef struct _NDIS_802_11_STATISTICS {
	ULONG Length;		/* Length of structure */
	LARGE_INTEGER TransmittedFragmentCount;
	LARGE_INTEGER MulticastTransmittedFrameCount;
	LARGE_INTEGER FailedCount;
	LARGE_INTEGER RetryCount;
	LARGE_INTEGER MultipleRetryCount;
	LARGE_INTEGER RTSSuccessCount;
	LARGE_INTEGER RTSFailureCount;
	LARGE_INTEGER ACKFailureCount;
	LARGE_INTEGER FrameDuplicateCount;
	LARGE_INTEGER ReceivedFragmentCount;
	LARGE_INTEGER MulticastReceivedFrameCount;
	LARGE_INTEGER FCSErrorCount;
	LARGE_INTEGER TransmittedFrameCount;
	LARGE_INTEGER WEPUndecryptableCount;
	LARGE_INTEGER TKIPLocalMICFailures;
	LARGE_INTEGER TKIPRemoteMICErrors;
	LARGE_INTEGER TKIPICVErrors;
	LARGE_INTEGER TKIPCounterMeasuresInvoked;
	LARGE_INTEGER TKIPReplays;
	LARGE_INTEGER CCMPFormatErrors;
	LARGE_INTEGER CCMPReplays;
	LARGE_INTEGER CCMPDecryptErrors;
	LARGE_INTEGER FourWayHandshakeFailures;
} NDIS_802_11_STATISTICS, *PNDIS_802_11_STATISTICS;

typedef struct _MBSS_STATISTICS {
	LONG TxCount;
	ULONG RxCount;
	ULONG ReceivedByteCount;
	ULONG TransmittedByteCount;
	ULONG RxErrorCount;
	ULONG RxDropCount;
	ULONG TxErrorCount;
	ULONG TxDropCount;
	ULONG ucPktsTx;
	ULONG ucPktsRx;
	ULONG mcPktsTx;
	ULONG mcPktsRx;
	ULONG bcPktsTx;
	ULONG bcPktsRx;
} MBSS_STATISTICS, *PMBSS_STATISTICS;

typedef ULONG NDIS_802_11_KEY_INDEX;
typedef ULONGLONG NDIS_802_11_KEY_RSC;

#ifdef DOT1X_SUPPORT
#define MAX_RADIUS_SRV_NUM			2	/* 802.1x failover number */
#ifdef VENDOR_FEATURE7_SUPPORT
#define MAX_MBSSID_1X_NUM				8
#else
#define MAX_MBSSID_1X_NUM				16
#endif
/* The dot1x related structure.
   It's used to communicate with DOT1X daemon */
typedef struct GNU_PACKED _RADIUS_SRV_INFO {
	UINT32 radius_ip;
	UINT32 radius_port;
	UCHAR radius_key[64];
	UCHAR radius_key_len;
} RADIUS_SRV_INFO, *PRADIUS_SRV_INFO;

typedef struct GNU_PACKED _DOT1X_BSS_INFO {
	UCHAR radius_srv_num;
	RADIUS_SRV_INFO radius_srv_info[MAX_RADIUS_SRV_NUM];
	UCHAR ieee8021xWEP;	/* dynamic WEP */
	UCHAR key_index;
	UCHAR key_length;	/* length of key in bytes */
	UCHAR key_material[13];
	UCHAR nasId[IFNAMSIZ];
	UCHAR nasId_len;
} DOT1X_BSS_INFO, *PDOT1X_BSS_INFO;

#ifdef RADIUS_ACCOUNTING_SUPPORT
typedef struct GNU_PACKED _ACCT_BSS_INFO {
	unsigned char		radius_srv_num;
	RADIUS_SRV_INFO		radius_srv_info[MAX_RADIUS_SRV_NUM];
	/* int				radius_request_cui; */
	int				radius_acct_authentic;
	int					acct_interim_interval;
	int					acct_enable;
} ACCT_BSS_INFO, *PACCT_BSS_INFO;
#endif /*RADIUS_ACCOUNTING_SUPPORT*/

typedef struct GNU_PACKED _DOT1X_CMM_CONF {
	UINT32 Length;		/* Length of this structure */
	UCHAR mbss_num;		/* indicate multiple BSS number */
	UINT32 own_ip_addr;
	UINT32 own_radius_port;
	UINT32 retry_interval;
	UINT32 session_timeout_interval;
	UINT32 quiet_interval;
	UCHAR EAPifname[MAX_MBSSID_1X_NUM][IFNAMSIZ];
	UCHAR EAPifname_len[MAX_MBSSID_1X_NUM];
	UCHAR PreAuthifname[MAX_MBSSID_1X_NUM][IFNAMSIZ];
	UCHAR PreAuthifname_len[MAX_MBSSID_1X_NUM];
	DOT1X_BSS_INFO Dot1xBssInfo[MAX_MBSSID_1X_NUM];
#ifdef RADIUS_ACCOUNTING_SUPPORT
	ACCT_BSS_INFO AcctBssInfo[MAX_MBSSID_1X_NUM];
#endif /*RADIUS_ACCOUNTING_SUPPORT*/
#ifdef RADIUS_MAC_ACL_SUPPORT
	UCHAR RadiusAclEnable[MAX_MBSSID_1X_NUM];
	UINT32 AclCacheTimeout[MAX_MBSSID_1X_NUM];
#endif /* RADIUS_MAC_ACL_SUPPORT */
} DOT1X_CMM_CONF, *PDOT1X_CMM_CONF;

typedef struct GNU_PACKED _DOT1X_IDLE_TIMEOUT {
	UCHAR StaAddr[6];
	UINT32 idle_timeout;
} DOT1X_IDLE_TIMEOUT, *PDOT1X_IDLE_TIMEOUT;

typedef struct GNU_PACKED _DOT1X_QUERY_STA_AID {
	UCHAR StaAddr[MAC_ADDR_LEN];
	UINT aid;
} DOT1X_QUERY_STA_AID, *PDOT1X_QUERY_STA_AID;

struct GNU_PACKED DOT1X_QUERY_STA_RSN {
	UCHAR sta_addr[MAC_ADDR_LEN];
	UINT32 akm;
	UINT32 pairwise_cipher;
	UINT32 group_cipher;
	UINT32 group_mgmt_cipher;
};

#ifdef RADIUS_ACCOUNTING_SUPPORT
typedef struct GNU_PACKED _DOT1X_QUERY_STA_DATA {
	UCHAR StaAddr[MAC_ADDR_LEN];
	unsigned long rx_packets;
	unsigned long tx_packets;
	unsigned long rx_bytes;
	unsigned long tx_bytes;
} DOT1X_QUERY_STA_DATA, *PDOT1X_QUERY_STA_DATA;
#endif /*RADIUS_ACCOUNTING_SUPPORT*/
#endif /* DOT1X_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
typedef struct _NDIS_AP_802_11_KEY {
	UINT Length;		/* Length of this structure */
	UCHAR addr[6];
	UINT KeyIndex;
	UINT KeyLength;		/* length of key in bytes */
	UCHAR KeyMaterial[1];	/* variable length depending on above field */
} NDIS_AP_802_11_KEY, *PNDIS_AP_802_11_KEY;
#endif /* CONFIG_AP_SUPPORT */

#ifdef APCLI_SUPPORT
#endif /* APCLI_SUPPORT */



typedef struct _NDIS_802_11_REMOVE_KEY {
	UINT Length;		/* Length of this structure */
	UINT KeyIndex;
	NDIS_802_11_MAC_ADDRESS BSSID;
} NDIS_802_11_REMOVE_KEY, *PNDIS_802_11_REMOVE_KEY;

typedef struct _NDIS_802_11_WEP {
	UINT Length;		/* Length of this structure */
	UINT KeyIndex;		/* 0 is the per-client key, 1-N are the */
	/* global keys */
	UINT KeyLength;		/* length of key in bytes */
	UCHAR KeyMaterial[1];	/* variable length depending on above field */
} NDIS_802_11_WEP, *PNDIS_802_11_WEP;


/* Add new authentication modes */
typedef enum _NDIS_802_11_AUTHENTICATION_MODE {
	Ndis802_11AuthModeOpen,
	Ndis802_11AuthModeShared,
	Ndis802_11AuthModeAutoSwitch,
	Ndis802_11AuthModeWPA,
	Ndis802_11AuthModeWPAPSK,
	Ndis802_11AuthModeWPANone,
	Ndis802_11AuthModeWPA2,
	Ndis802_11AuthModeWPA2PSK,
	Ndis802_11AuthModeWPA1WPA2,
	Ndis802_11AuthModeWPA1PSKWPA2PSK,
	Ndis802_11AuthModeMax	/* Not a real mode, defined as upper bound */
} NDIS_802_11_AUTHENTICATION_MODE, *PNDIS_802_11_AUTHENTICATION_MODE;

typedef UCHAR NDIS_802_11_RATES[NDIS_802_11_LENGTH_RATES];	/* Set of 8 data rates */
typedef UCHAR NDIS_802_11_RATES_EX[NDIS_802_11_LENGTH_RATES_EX];	/* Set of 16 data rates */
typedef struct GNU_PACKED _NDIS80211PSK {
	UINT    WPAKeyLen;
	UCHAR   WPAKey[64];
} NDIS80211PSK;

typedef struct GNU_PACKED _NDIS_802_11_SSID {
	UINT SsidLength;	/* length of SSID field below, in bytes; */
	/* this can be zero. */
	UCHAR Ssid[NDIS_802_11_LENGTH_SSID];	/* SSID information field */
} NDIS_802_11_SSID, *PNDIS_802_11_SSID;

typedef struct GNU_PACKED _NDIS_WLAN_BSSID {
	ULONG Length;		/* Length of this structure */
	NDIS_802_11_MAC_ADDRESS MacAddress;	/* BSSID */
	UCHAR Reserved[2];
	NDIS_802_11_SSID Ssid;	/* SSID */
	ULONG Privacy;		/* WEP encryption requirement */
	NDIS_802_11_RSSI Rssi;	/* receive signal strength in dBm */
	NDIS_802_11_NETWORK_TYPE NetworkTypeInUse;
	NDIS_802_11_CONFIGURATION Configuration;
	NDIS_802_11_NETWORK_INFRASTRUCTURE InfrastructureMode;
	NDIS_802_11_RATES SupportedRates;
} NDIS_WLAN_BSSID, *PNDIS_WLAN_BSSID;

typedef struct GNU_PACKED _NDIS_802_11_BSSID_LIST {
	UINT NumberOfItems;	/* in list below, at least 1 */
	NDIS_WLAN_BSSID Bssid[1];
} NDIS_802_11_BSSID_LIST, *PNDIS_802_11_BSSID_LIST;

typedef struct {
	BOOLEAN bValid;		/* 1: variable contains valid value */
	USHORT StaNum;
	UCHAR ChannelUtilization;
	USHORT RemainingAdmissionControl;	/* in unit of 32-us */
} QBSS_LOAD_UI, *PQBSS_LOAD_UI;

/* Added Capabilities, IELength and IEs for each BSSID */
typedef struct GNU_PACKED _NDIS_WLAN_BSSID_EX {
	ULONG Length;		/* Length of this structure */
	NDIS_802_11_MAC_ADDRESS MacAddress;	/* BSSID */
	UCHAR WpsAP; /* 0x00: not support WPS, 0x01: support normal WPS, 0x02: support Ralink auto WPS, 0x04: support Samsung WAC */
	CHAR MinSNR;
	NDIS_802_11_SSID Ssid;	/* SSID */
	UINT Privacy;		/* WEP encryption requirement */
	NDIS_802_11_RSSI Rssi;	/* receive signal */
	/* strength in dBm */
	NDIS_802_11_NETWORK_TYPE NetworkTypeInUse;
	NDIS_802_11_CONFIGURATION Configuration;
	NDIS_802_11_NETWORK_INFRASTRUCTURE InfrastructureMode;
	NDIS_802_11_RATES_EX SupportedRates;
	ULONG IELength;
	UCHAR IEs[1];
} NDIS_WLAN_BSSID_EX, *PNDIS_WLAN_BSSID_EX;

typedef struct GNU_PACKED _NDIS_802_11_BSSID_LIST_EX {
	UINT NumberOfItems;	/* in list below, at least 1 */
	NDIS_WLAN_BSSID_EX Bssid[1];
} NDIS_802_11_BSSID_LIST_EX, *PNDIS_802_11_BSSID_LIST_EX;

typedef struct GNU_PACKED _NDIS_802_11_FIXED_IEs {
	UCHAR Timestamp[8];
	USHORT BeaconInterval;
	USHORT Capabilities;
} NDIS_802_11_FIXED_IEs, *PNDIS_802_11_FIXED_IEs;

typedef struct _NDIS_802_11_VARIABLE_IEs {
	UCHAR ElementID;
	UCHAR Length;		/* Number of bytes in data field */
	UCHAR data[1];
} NDIS_802_11_VARIABLE_IEs, *PNDIS_802_11_VARIABLE_IEs;

typedef ULONG NDIS_802_11_FRAGMENTATION_THRESHOLD;

typedef ULONG NDIS_802_11_RTS_THRESHOLD;

typedef ULONG NDIS_802_11_ANTENNA;

typedef enum _NDIS_802_11_PRIVACY_FILTER {
	Ndis802_11PrivFilterAcceptAll,
	Ndis802_11PrivFilter8021xWEP
} NDIS_802_11_PRIVACY_FILTER, *PNDIS_802_11_PRIVACY_FILTER;

/* Added new encryption types */
/* Also aliased typedef to new name */
typedef enum _NDIS_802_11_WEP_STATUS {
	Ndis802_11WEPEnabled,
	Ndis802_11Encryption1Enabled = Ndis802_11WEPEnabled,
	Ndis802_11WEPDisabled,
	Ndis802_11EncryptionDisabled = Ndis802_11WEPDisabled,
	Ndis802_11WEPKeyAbsent,
	Ndis802_11Encryption1KeyAbsent = Ndis802_11WEPKeyAbsent,
	Ndis802_11WEPNotSupported,
	Ndis802_11EncryptionNotSupported = Ndis802_11WEPNotSupported,
	Ndis802_11TKIPEnable,
	Ndis802_11Encryption2Enabled = Ndis802_11TKIPEnable,
	Ndis802_11Encryption2KeyAbsent,
	Ndis802_11AESEnable,
	Ndis802_11Encryption3Enabled = Ndis802_11AESEnable,
	Ndis802_11CCMP256Enable,
	Ndis802_11GCMP128Enable,
	Ndis802_11GCMP256Enable,
	Ndis802_11Encryption3KeyAbsent,
	Ndis802_11TKIPAESMix,
	Ndis802_11Encryption4Enabled = Ndis802_11TKIPAESMix,    /* TKIP or AES mix */
	Ndis802_11Encryption4KeyAbsent,
	Ndis802_11GroupWEP40Enabled,
	Ndis802_11GroupWEP104Enabled,
} NDIS_802_11_WEP_STATUS, *PNDIS_802_11_WEP_STATUS, NDIS_802_11_ENCRYPTION_STATUS, *PNDIS_802_11_ENCRYPTION_STATUS;

typedef enum _NDIS_802_11_RELOAD_DEFAULTS {
	Ndis802_11ReloadWEPKeys
} NDIS_802_11_RELOAD_DEFAULTS, *PNDIS_802_11_RELOAD_DEFAULTS;

#define NDIS_802_11_AI_REQFI_CAPABILITIES      1
#define NDIS_802_11_AI_REQFI_LISTENINTERVAL    2
#define NDIS_802_11_AI_REQFI_CURRENTAPADDRESS  4

#define NDIS_802_11_AI_RESFI_CAPABILITIES      1
#define NDIS_802_11_AI_RESFI_STATUSCODE        2
#define NDIS_802_11_AI_RESFI_ASSOCIATIONID     4

typedef struct _NDIS_802_11_AI_REQFI {
	USHORT Capabilities;
	USHORT ListenInterval;
	NDIS_802_11_MAC_ADDRESS CurrentAPAddress;
} NDIS_802_11_AI_REQFI, *PNDIS_802_11_AI_REQFI;

typedef struct _NDIS_802_11_AI_RESFI {
	USHORT Capabilities;
	USHORT StatusCode;
	USHORT AssociationId;
} NDIS_802_11_AI_RESFI, *PNDIS_802_11_AI_RESFI;

typedef struct _NDIS_802_11_ASSOCIATION_INFORMATION {
	ULONG Length;
	USHORT AvailableRequestFixedIEs;
	NDIS_802_11_AI_REQFI RequestFixedIEs;
	ULONG RequestIELength;
	ULONG OffsetRequestIEs;
	USHORT AvailableResponseFixedIEs;
	NDIS_802_11_AI_RESFI ResponseFixedIEs;
	ULONG ResponseIELength;
	ULONG OffsetResponseIEs;
} NDIS_802_11_ASSOCIATION_INFORMATION, *PNDIS_802_11_ASSOCIATION_INFORMATION;

typedef struct _NDIS_802_11_AUTHENTICATION_EVENT {
	NDIS_802_11_STATUS_INDICATION Status;
	NDIS_802_11_AUTHENTICATION_REQUEST Request[1];
} NDIS_802_11_AUTHENTICATION_EVENT, *PNDIS_802_11_AUTHENTICATION_EVENT;

/*
typedef struct _NDIS_802_11_TEST
{
    ULONG Length;
    ULONG Type;
    union
    {
	NDIS_802_11_AUTHENTICATION_EVENT AuthenticationEvent;
	NDIS_802_11_RSSI RssiTrigger;
    };
} NDIS_802_11_TEST, *PNDIS_802_11_TEST;
 */

/* 802.11 Media stream constraints, associated with OID_802_11_MEDIA_STREAM_MODE */
typedef enum _NDIS_802_11_MEDIA_STREAM_MODE {
	Ndis802_11MediaStreamOff,
	Ndis802_11MediaStreamOn,
} NDIS_802_11_MEDIA_STREAM_MODE, *PNDIS_802_11_MEDIA_STREAM_MODE;

/* PMKID Structures */
typedef UCHAR NDIS_802_11_PMKID_VALUE[16];
#define INVALID_PMKID_IDX	-1

#if defined(CONFIG_STA_SUPPORT) || defined(WPA_SUPPLICANT_SUPPORT) || defined(APCLI_SAE_SUPPORT) || defined(APCLI_OWE_SUPPORT)
typedef struct _BSSID_INFO {
	NDIS_802_11_MAC_ADDRESS BSSID;
	NDIS_802_11_PMKID_VALUE PMKID;
	UCHAR PMK[LEN_MAX_PMK];
#if defined(APCLI_SAE_SUPPORT) || defined(APCLI_OWE_SUPPORT)
	BOOLEAN Valid;
#endif
} BSSID_INFO, *PBSSID_INFO;

typedef struct _NDIS_802_11_PMKID {
	UINT Length;
	UINT BSSIDInfoCount;
	BSSID_INFO BSSIDInfo[1];
} NDIS_802_11_PMKID, *PNDIS_802_11_PMKID;
#endif /* defined(CONFIG_STA_SUPPORT) || defined(WPA_SUPPLICANT_SUPPORT) */

#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
#endif /* APCLI_SUPPORT */

typedef struct _AP_BSSID_INFO {
	UCHAR Mbssidx;
	NDIS_802_11_MAC_ADDRESS MAC;
	NDIS_802_11_PMKID_VALUE PMKID;
	UCHAR PMK[LEN_MAX_PMK];
	ULONG RefreshTime;
	BOOLEAN Valid;
} AP_BSSID_INFO, *PAP_BSSID_INFO;

#define MAX_PMKID_COUNT		128
typedef struct _NDIS_AP_802_11_PMKID {
	AP_BSSID_INFO BSSIDInfo[MAX_PMKID_COUNT];
} NDIS_AP_802_11_PMKID, *PNDIS_AP_802_11_PMKID;
#endif /* CONFIG_AP_SUPPORT */

typedef struct _NDIS_802_11_AUTHENTICATION_ENCRYPTION {
	NDIS_802_11_AUTHENTICATION_MODE AuthModeSupported;
	NDIS_802_11_ENCRYPTION_STATUS EncryptStatusSupported;
} NDIS_802_11_AUTHENTICATION_ENCRYPTION, *PNDIS_802_11_AUTHENTICATION_ENCRYPTION;

typedef struct _NDIS_802_11_CAPABILITY {
	ULONG Length;
	ULONG Version;
	ULONG NoOfPMKIDs;
	ULONG NoOfAuthEncryptPairsSupported;

	NDIS_802_11_AUTHENTICATION_ENCRYPTION
	AuthenticationEncryptionSupported[1];
} NDIS_802_11_CAPABILITY, *PNDIS_802_11_CAPABILITY;



#ifdef DBG
/*
	When use private ioctl oid get/set the configuration, we can use following flags to provide specific rules when handle the cmd
 */
#define RTPRIV_IOCTL_FLAG_UI			0x0001	/* Notidy this private cmd send by UI. */
#define RTPRIV_IOCTL_FLAG_NODUMPMSG	0x0002	/* Notify driver cannot dump msg to stdio/stdout when run this private ioctl cmd */
#define RTPRIV_IOCTL_FLAG_NOSPACE		0x0004	/* Notify driver didn't need copy msg to caller due to the caller didn't reserve space for this cmd */
#endif /* DBG */


#ifdef SNMP_SUPPORT
/*SNMP ieee 802dot11 , 2008_0220 */
/* dot11res(3) */
#define RT_OID_802_11_MANUFACTUREROUI			0x0700
#define RT_OID_802_11_MANUFACTURERNAME			0x0701
#define RT_OID_802_11_RESOURCETYPEIDNAME		0x0702

/* dot11smt(1) */
#define RT_OID_802_11_PRIVACYOPTIONIMPLEMENTED	0x0703
#define RT_OID_802_11_POWERMANAGEMENTMODE		0x0704
#define OID_802_11_WEPDEFAULTKEYVALUE			0x0705	/* read , write */
#define OID_802_11_WEPDEFAULTKEYID				0x0706
#define RT_OID_802_11_WEPKEYMAPPINGLENGTH		0x0707
#define OID_802_11_SHORTRETRYLIMIT				0x0708
#define OID_802_11_LONGRETRYLIMIT				0x0709
#define RT_OID_802_11_PRODUCTID					0x0710
#define RT_OID_802_11_MANUFACTUREID				0x0711
#endif /* SNMP_SUPPORT */

/* //dot11Phy(4) */
#if (defined(SNMP_SUPPORT) || defined(WH_EZ_SETUP) || defined(VENDOR_FEATURE6_SUPPORT))
#define OID_802_11_CURRENTCHANNEL				0x0712
#endif

/*dot11mac */
#define RT_OID_802_11_MAC_ADDRESS				0x0713
#define OID_802_11_BUILD_CHANNEL_EX				0x0714
#define OID_802_11_GET_CH_LIST					0x0715
#define OID_802_11_GET_COUNTRY_CODE				0x0716
#define OID_802_11_GET_CHANNEL_GEOGRAPHY		0x0717

/*#define RT_OID_802_11_STATISTICS              (OID_GET_SET_TOGGLE | OID_802_11_STATISTICS) */



#ifdef WSC_INCLUDED
#define RT_OID_WAC_REQ								0x0736
#define	RT_OID_WSC_AUTO_PROVISION_WITH_BSSID		0x0737
#define	RT_OID_WSC_AUTO_PROVISION					0x0738
#ifdef WSC_LED_SUPPORT
/*WPS LED MODE 10 for Dlink WPS LED */
#define RT_OID_LED_WPS_MODE10						0x0739
#endif /* WSC_LED_SUPPORT */
#endif /* WSC_INCLUDED */
#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
#define RT_OID_APCLI_WSC_PIN_CODE					0x074A
#endif /* APCLI_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
#define	RT_OID_WSC_FRAGMENT_SIZE					0x074D
#define	RT_OID_WSC_V2_SUPPORT						0x074E
#define	RT_OID_WSC_CONFIG_STATUS					0x074F
#define RT_OID_802_11_WSC_QUERY_PROFILE				0x0750
/* for consistency with RT61 */
#define RT_OID_WSC_QUERY_STATUS						0x0751
#define RT_OID_WSC_PIN_CODE							0x0752
#define RT_OID_WSC_UUID								0x0753
#define RT_OID_WSC_SET_SELECTED_REGISTRAR			0x0754
#define RT_OID_WSC_EAPMSG							0x0755
#define RT_OID_WSC_MANUFACTURER						0x0756
#define RT_OID_WSC_MODEL_NAME						0x0757
#define RT_OID_WSC_MODEL_NO							0x0758
#define RT_OID_WSC_SERIAL_NO						0x0759
#define RT_OID_WSC_READ_UFD_FILE					0x075A
#define RT_OID_WSC_WRITE_UFD_FILE					0x075B
#define RT_OID_WSC_QUERY_PEER_INFO_ON_RUNNING		0x075C
#define RT_OID_WSC_MAC_ADDRESS						0x0760
#ifdef VENDOR_FEATURE7_SUPPORT
#define ARRIS_OID_WSC_QUERY_STATE                   0x0AA0 /* Case to return WPS STATE */
#ifdef DOT11_N_SUPPORT
#define ARRIS_OID_HTEXTCHA_QUERY_VALUE              0x0AA1 /* Case to return HtExtCha */
#endif
#endif

#ifdef LLTD_SUPPORT
/* for consistency with RT61 */
#define RT_OID_GET_PHY_MODE                         0x761
#ifdef CONFIG_AP_SUPPORT
#define RT_OID_GET_LLTD_ASSO_TABLE                  0x762
#ifdef APCLI_SUPPORT
#define RT_OID_GET_REPEATER_AP_LINEAGE				0x763
#endif /* APCLI_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
#endif /* LLTD_SUPPORT */


#ifdef DOT11R_FT_SUPPORT
#define OID_802_11R_SUPPORT							0x0780
#define OID_802_11R_MDID							0x0781
#define OID_802_11R_R0KHID							0x0782
#define OID_802_11R_RIC								0x0783
#define OID_802_11R_OTD								0x0784
#define OID_802_11R_INFO							0x0785

#define	RT_OID_802_11R_SUPPORT						(OID_GET_SET_TOGGLE | OID_802_11R_SUPPORT)
#define RT_OID_802_11R_MDID							(OID_GET_SET_TOGGLE | OID_802_11R_MDID)
#define RT_OID_802_11R_R0KHID						(OID_GET_SET_TOGGLE | OID_802_11R_R0KHID)
#define	RT_OID_802_11R_RIC							(OID_GET_SET_TOGGLE | OID_802_11R_RIC)
#define RT_OID_802_11R_OTD							(OID_GET_SET_TOGGLE | OID_802_11R_OTD)
#define RT_OID_802_11R_INFO							(OID_GET_SET_TOGGLE | OID_802_11R_INFO)
#endif /* DOT11R_FT_SUPPORT */



#ifdef CON_WPS
#define RT_OID_WSC_SET_CON_WPS_STOP                 0x0764
#endif /* CON_WPS */

/* New for MeetingHouse Api support */
#define OID_MH_802_1X_SUPPORTED               0xFFEDC100

/* MIMO Tx parameter, ShortGI, MCS, STBC, etc.  these are fields in TXWI. Don't change this definition!!! */
typedef union _HTTRANSMIT_SETTING {
#ifdef RT_BIG_ENDIAN
	struct {
		USHORT MODE:3;	/* Use definition MODE_xxx. */
		USHORT iTxBF:1;
		USHORT eTxBF:1;
		USHORT STBC:1;	/* only support in HT/VHT mode with MCS0~7 */
		USHORT ShortGI:1;
		USHORT BW:2;	/* channel bandwidth 20MHz/40/80 MHz */
		USHORT ldpc:1;
		USHORT MCS:6;	/* MCS */
	} field;
#else
	struct {
		USHORT MCS:6;
		USHORT ldpc:1;
		USHORT BW:2;
		USHORT ShortGI:1;
		USHORT STBC:1;
		USHORT eTxBF:1;
		USHORT iTxBF:1;
		USHORT MODE:3;
	} field;
#endif
	USHORT word;
} HTTRANSMIT_SETTING, *PHTTRANSMIT_SETTING;

#ifdef DFS_VENDOR10_CUSTOM_FEATURE
typedef struct _CLIENT_INFO {
	UCHAR Addr[MAC_ADDR_LEN];
	ULONG LastRxTimeCount;
} CLIENT_INFO, *PCLIENT_INFO;


typedef struct MSG_CLIENT_LIST {
	UINT16	ClientCnt;
	CLIENT_INFO CLIENTLIST[128];
} MSG_CLIENT_LIST, *PMSG_CLIENT_LIST;
#endif

#ifdef VENDOR10_CUSTOM_RSSI_FEATURE
typedef struct MSG_RSSI_LIST {
	UCHAR Addr[MAC_ADDR_LEN];
	LONG CurRssi;
} MSG_RSSI_LIST, *PMSG_RSSI_LIST;
#endif

#ifdef OFFCHANNEL_SCAN_FEATURE
#define MAX_AWAY_CHANNEL 5
enum ASYNC_OFFCHANNEL_COMMAND_RSP {
	GET_OFFCHANNEL_INFO = 34,
	OFFCHANNEL_INFO_RSP,
	TRIGGER_DRIVER_CHANNEL_SWITCH,
	UPDATE_DRIVER_SORTED_CHANNEL_LIST,
	DFS_DRIVER_CHANNEL_SWITCH,
	DFS_RADAR_HIT,
	DFS_CHANNEL_NOP_CMPLT,
	DRIVER_CHANNEL_SWITCH_SUCCESSFUL
};


typedef struct operating_info {
	UINT8 channel;
	UCHAR cfg_ht_bw;
	UCHAR cfg_vht_bw;
	UCHAR RDDurRegion;
	UCHAR region;
	UCHAR is4x4Mode;
	UCHAR vht_cent_ch2;
} OPERATING_INFO, *POPERATING_INFO;

typedef struct _channel_info {
	UINT8	channel;
	UINT8	channel_idx;
	INT32	NF;
	UINT32  rx_time;
	UINT32	tx_time;
	UINT32	obss_time;
	UINT32	channel_busy_time;
	UINT8	dfs_req;
	UCHAR 	actual_measured_time;
} CHANNEL_INFO, *PCHANNEL_INFO;


struct msg_channel_list {
	CHANNEL_INFO CHANNELLIST[60];
};
typedef struct offchannel_param {
	UCHAR channel[MAX_AWAY_CHANNEL];
	UCHAR scan_type[MAX_AWAY_CHANNEL];
	UCHAR scan_time[MAX_AWAY_CHANNEL];
	UINT32 Num_of_Away_Channel;
} OFFCHANNEL_SCAN_PARAM, *POFFCHANNEL_SCAN_PARAM;

typedef struct sorted_list_info {
	UINT8 size;
	UINT8 SortedMaxChannelBusyTimeList[MAX_NUM_OF_CHANNELS+1];
	UINT8 SortedMinChannelBusyTimeList[MAX_NUM_OF_CHANNELS+1];

} SORTED_CHANNEL_LIST, *PSORTED_CHANNEL_LIST;


typedef struct _OFFCHANNEL_SCAN_MSG {
UINT8   Action;
UCHAR ifrn_name[32];
union {
				CHANNEL_INFO channel_data;
				OFFCHANNEL_SCAN_PARAM offchannel_param;
				OPERATING_INFO operating_ch_info;
				SORTED_CHANNEL_LIST sorted_channel_list;
} data;
} OFFCHANNEL_SCAN_MSG, *POFFCHANNEL_SCAN_MSG;
#endif

typedef enum _RT_802_11_PREAMBLE {
	Rt802_11PreambleLong,
	Rt802_11PreambleShort,
	Rt802_11PreambleAuto
} RT_802_11_PREAMBLE, *PRT_802_11_PREAMBLE;

typedef enum _RT_802_11_PHY_MODE {
	PHY_11BG_MIXED = 0,
	PHY_11B = 1,
	PHY_11A = 2,
	PHY_11ABG_MIXED = 3,
	PHY_11G = 4,
#ifdef DOT11_N_SUPPORT
	PHY_11ABGN_MIXED = 5,	/* both band   5 */
	PHY_11N_2_4G = 6,		/* 11n-only with 2.4G band      6 */
	PHY_11GN_MIXED = 7,		/* 2.4G band      7 */
	PHY_11AN_MIXED = 8,		/* 5G  band       8 */
	PHY_11BGN_MIXED = 9,	/* if check 802.11b.      9 */
	PHY_11AGN_MIXED = 10,	/* if check 802.11b.      10 */
	PHY_11N_5G = 11,		/* 11n-only with 5G band                11 */
#endif /* DOT11_N_SUPPORT */
#ifdef DOT11_VHT_AC
	PHY_11VHT_N_ABG_MIXED = 12, /* 12 -> AC/A/AN/B/G/GN mixed */
	PHY_11VHT_N_AG_MIXED = 13, /* 13 -> AC/A/AN/G/GN mixed  */
	PHY_11VHT_N_A_MIXED = 14, /* 14 -> AC/AN/A mixed in 5G band */
	PHY_11VHT_N_MIXED = 15, /* 15 -> AC/AN mixed in 5G band */
#endif /* DOT11_VHT_AC */
	PHY_MODE_MAX,
} RT_802_11_PHY_MODE;

#ifdef DOT11_VHT_AC
#define PHY_MODE_IS_5G_BAND(__Mode)	\
	((__Mode == PHY_11A) ||			\
	 (__Mode == PHY_11ABG_MIXED) ||	\
	 (__Mode == PHY_11ABGN_MIXED) ||	\
	 (__Mode == PHY_11AN_MIXED) ||	\
	 (__Mode == PHY_11AGN_MIXED) ||	\
	 (__Mode == PHY_11N_5G) ||\
	 (__Mode == PHY_11VHT_N_MIXED) ||\
	 (__Mode == PHY_11VHT_N_A_MIXED))
#elif defined(DOT11_N_SUPPORT)
#define PHY_MODE_IS_5G_BAND(__Mode)	\
	((__Mode == PHY_11A) ||			\
	 (__Mode == PHY_11ABG_MIXED) ||	\
	 (__Mode == PHY_11ABGN_MIXED) ||	\
	 (__Mode == PHY_11AN_MIXED) ||	\
	 (__Mode == PHY_11AGN_MIXED) ||	\
	 (__Mode == PHY_11N_5G))
#else

#define PHY_MODE_IS_5G_BAND(__Mode)	\
	((__Mode == PHY_11A) ||			\
	 (__Mode == PHY_11ABG_MIXED))
#endif /* DOT11_N_SUPPORT */

/* put all proprietery for-query objects here to reduce # of Query_OID */
typedef struct _RT_802_11_LINK_STATUS {
	ULONG CurrTxRate;	/* in units of 0.5Mbps */
	ULONG ChannelQuality;	/* 0..100 % */
	ULONG TxByteCount;	/* both ok and fail */
	ULONG RxByteCount;	/* both ok and fail */
	ULONG CentralChannel;	/* 40MHz central channel number */
} RT_802_11_LINK_STATUS, *PRT_802_11_LINK_STATUS;

#ifdef SYSTEM_LOG_SUPPORT
typedef struct _RT_802_11_EVENT_LOG {
	LARGE_INTEGER SystemTime;	/* timestammp via NdisGetCurrentSystemTime() */
	UCHAR Addr[MAC_ADDR_LEN];
	USHORT Event;		/* EVENT_xxx */
} RT_802_11_EVENT_LOG, *PRT_802_11_EVENT_LOG;

typedef struct _RT_802_11_EVENT_TABLE {
	ULONG Num;
	ULONG Rsv;		/* to align Log[] at LARGE_INEGER boundary */
	RT_802_11_EVENT_LOG Log[MAX_NUMBER_OF_EVENT];
} RT_802_11_EVENT_TABLE, *PRT_802_11_EVENT_TABLE;
#endif /* SYSTEM_LOG_SUPPORT */

/* MIMO Tx parameter, ShortGI, MCS, STBC, etc.  these are fields in TXWI. Don't change this definition!!! */
typedef union _MACHTTRANSMIT_SETTING {
	struct {
		USHORT MCS:7;	/* MCS */
		USHORT BW:1;	/*channel bandwidth 20MHz or 40 MHz */
		USHORT ShortGI:1;
		USHORT STBC:2;	/*SPACE */
		USHORT rsv:3;
		USHORT MODE:2;	/* Use definition MODE_xxx. */
	} field;
	USHORT word;
} MACHTTRANSMIT_SETTING, *PMACHTTRANSMIT_SETTING;

typedef struct _RT_802_11_MAC_ENTRY {
	UCHAR ApIdx;
	UCHAR Addr[MAC_ADDR_LEN];
	UCHAR Aid;
	UCHAR Psm;		/* 0:PWR_ACTIVE, 1:PWR_SAVE */
	UCHAR MimoPs;		/* 0:MMPS_STATIC, 1:MMPS_DYNAMIC, 3:MMPS_Enabled */
	CHAR AvgRssi0;
	CHAR AvgRssi1;
	CHAR AvgRssi2;
#ifdef CUSTOMER_DCC_FEATURE
	CHAR AvgRssi3;
#endif
	UINT32 ConnectedTime;
	HTTRANSMIT_SETTING TxRate;
#ifdef CUSTOMER_DCC_FEATURE
	UINT32 AvgSnr;
#endif /* CUSTOMER_DCC_FEATURE */
	UINT32 LastRxRate;
	/*
		sync with WEB UI's structure for ioctl usage.
	*/
#ifndef CUSTOMER_DCC_FEATURE
	SHORT StreamSnr[3];				/* BF SNR from RXWI. Units=0.25 dB. 22 dB offset removed */
	SHORT SoundingRespSnr[3];			/* SNR from Sounding Response. Units=0.25 dB. 22 dB offset removed */
	/*	SHORT TxPER;	*/					/* TX PER over the last second. Percent */
	/*	SHORT reserved;*/
#endif /* !CUSTOMER_DCC_FEATURE */
} RT_802_11_MAC_ENTRY, *PRT_802_11_MAC_ENTRY;

typedef struct _RT_802_11_MAC_TABLE {
	ULONG Num;
	RT_802_11_MAC_ENTRY Entry[MAX_NUMBER_OF_MAC];
} RT_802_11_MAC_TABLE, *PRT_802_11_MAC_TABLE;

#ifdef DOT11_N_SUPPORT
#ifdef TXBF_SUPPORT
typedef
struct {
	ULONG TxSuccessCount;
	ULONG TxRetryCount;
	ULONG TxFailCount;
	ULONG ETxSuccessCount;
	ULONG ETxRetryCount;
	ULONG ETxFailCount;
	ULONG ITxSuccessCount;
	ULONG ITxRetryCount;
	ULONG ITxFailCount;
} RT_COUNTER_TXBF;

typedef
struct {
	ULONG Num;
	RT_COUNTER_TXBF Entry[MAX_NUMBER_OF_MAC];
} RT_802_11_TXBF_TABLE;
#endif /* TXBF_SUPPORT */
#endif /* DOT11_N_SUPPORT */

/* structure for query/set hardware register - MAC, BBP, RF register */
typedef struct _RT_802_11_HARDWARE_REGISTER {
	ULONG HardwareType;	/* 0:MAC, 1:BBP, 2:RF register, 3:EEPROM */
	ULONG Offset;		/* Q/S register offset addr */
	ULONG Data;		/* R/W data buffer */
} RT_802_11_HARDWARE_REGISTER, *PRT_802_11_HARDWARE_REGISTER;

typedef struct _RT_802_11_AP_CONFIG {
	ULONG EnableTxBurst;	/* 0-disable, 1-enable */
	ULONG EnableTurboRate;	/* 0-disable, 1-enable 72/100mbps turbo rate */
	ULONG IsolateInterStaTraffic;	/* 0-disable, 1-enable isolation */
	ULONG HideSsid;		/* 0-disable, 1-enable hiding */
	ULONG UseBGProtection;	/* 0-AUTO, 1-always ON, 2-always OFF */
	ULONG UseShortSlotTime;	/* 0-no use, 1-use 9-us short slot time */
	ULONG Rsv1;		/* must be 0 */
	ULONG SystemErrorBitmap;	/* ignore upon SET, return system error upon QUERY */
} RT_802_11_AP_CONFIG, *PRT_802_11_AP_CONFIG;

/* structure to query/set STA_CONFIG */
typedef struct _RT_802_11_STA_CONFIG {
	ULONG EnableTxBurst;	/* 0-disable, 1-enable */
	ULONG EnableTurboRate;	/* 0-disable, 1-enable 72/100mbps turbo rate */
	ULONG UseBGProtection;	/* 0-AUTO, 1-always ON, 2-always OFF */
	ULONG UseShortSlotTime;	/* 0-no use, 1-use 9-us short slot time when applicable */
	ULONG AdhocMode;	/* 0-11b rates only (WIFI spec), 1 - b/g mixed, 2 - g only */
	ULONG HwRadioStatus;	/* 0-OFF, 1-ON, default is 1, Read-Only */
	ULONG Rsv1;		/* must be 0 */
	ULONG SystemErrorBitmap;	/* ignore upon SET, return system error upon QUERY */
} RT_802_11_STA_CONFIG, *PRT_802_11_STA_CONFIG;

/* */
/*  For OID Query or Set about BA structure */
/* */
typedef struct _OID_BACAP_STRUC {
	UCHAR RxBAWinLimit;
	UCHAR TxBAWinLimit;
	UCHAR Policy;		/* 0: DELAY_BA 1:IMMED_BA  (//BA Policy subfiled value in ADDBA frame)   2:BA-not use. other value invalid */
	UCHAR MpduDensity;	/* 0: DELAY_BA 1:IMMED_BA  (//BA Policy subfiled value in ADDBA frame)   2:BA-not use. other value invalid */
	UCHAR AmsduEnable;	/*Enable AMSDU transmisstion */
	UCHAR AmsduSize;	/* 0:3839, 1:7935 bytes. UINT  MSDUSizeToBytes[]        = { 3839, 7935}; */
	UCHAR MMPSmode;		/* MIMO power save more, 0:static, 1:dynamic, 2:rsv, 3:mimo enable */
	BOOLEAN AutoBA;		/* Auto BA will automatically */
} OID_BACAP_STRUC, *POID_BACAP_STRUC;

typedef struct _RT_802_11_ACL_ENTRY {
	UCHAR Addr[MAC_ADDR_LEN];
	USHORT Rsv;
#ifdef ACL_BLK_COUNT_SUPPORT
	ULONG Reject_Count;
#endif/* ACL_BLK_COUNT_SUPPORT*/
} RT_802_11_ACL_ENTRY, *PRT_802_11_ACL_ENTRY;

typedef struct GNU_PACKED _RT_802_11_ACL {
	ULONG Policy;		/* 0-disable, 1-positive list, 2-negative list */
	ULONG Num;
	RT_802_11_ACL_ENTRY Entry[MAX_NUMBER_OF_ACL];
} RT_802_11_ACL, *PRT_802_11_ACL;

#ifdef RADIUS_MAC_ACL_SUPPORT
typedef struct _RT_802_11_RADIUS_ACL_ENTRY {
	struct _RT_802_11_RADIUS_ACL_ENTRY *pNext;
	UCHAR Addr[MAC_ADDR_LEN];
	USHORT result;
} RT_802_11_RADIUS_ACL_ENTRY, *PRT_802_11_RADIUS_ACL_ENTRY;

typedef struct GNU_PACKED _RT_802_11_RADIUS_ACL {
#define RADIUS_MAC_AUTH_ENABLE 1
#define RADIUS_MAC_AUTH_DISABLE 0
	UCHAR Policy;
	UINT32 Timeout;
	LIST_HEADER cacheList;
} RT_802_11_RADIUS_ACL, *PRT_802_11_RADIUS_ACL;
#endif /* RADIUS_MAC_ACL_SUPPORT */

typedef struct _RT_802_11_WDS {
	ULONG Num;
	NDIS_802_11_MAC_ADDRESS Entry[24 /*MAX_NUM_OF_WDS_LINK */];
	ULONG KeyLength;
	UCHAR KeyMaterial[32];
} RT_802_11_WDS, *PRT_802_11_WDS;

typedef struct _RT_802_11_TX_RATES_ {
	UCHAR SupRateLen;
	UCHAR SupRate[MAX_LENGTH_OF_SUPPORT_RATES];
	UCHAR ExtRateLen;
	UCHAR ExtRate[MAX_LENGTH_OF_SUPPORT_RATES];
} RT_802_11_TX_RATES, *PRT_802_11_TX_RATES;

/* Definition of extra information code */
#define	GENERAL_LINK_UP			0x0	/* Link is Up */
#define	GENERAL_LINK_DOWN		0x1	/* Link is Down */
#define	HW_RADIO_OFF			0x2	/* Hardware radio off */
#define	SW_RADIO_OFF			0x3	/* Software radio off */
#define	AUTH_FAIL				0x4	/* Open authentication fail */
#define	AUTH_FAIL_KEYS			0x5	/* Shared authentication fail */
#define	ASSOC_FAIL				0x6	/* Association failed */
#define	EAP_MIC_FAILURE			0x7	/* Deauthencation because MIC failure */
#define	EAP_4WAY_TIMEOUT		0x8	/* Deauthencation on 4-way handshake timeout */
#define	EAP_GROUP_KEY_TIMEOUT	0x9	/* Deauthencation on group key handshake timeout */
#define	EAP_SUCCESS				0xa	/* EAP succeed */
#define	DETECT_RADAR_SIGNAL		0xb	/* Radar signal occur in current channel */
#define EXTRA_INFO_MAX			0xb	/* Indicate Last OID */

#define EXTRA_INFO_CLEAR		0xffffffff

/* This is OID setting structure. So only GF or MM as Mode. This is valid when our wirelss mode has 802.11n in use. */
typedef struct {
	RT_802_11_PHY_MODE PhyMode;	/* */
	UCHAR TransmitNo;
	UCHAR HtMode;		/*HTMODE_GF or HTMODE_MM */
	UINT8 ExtOffset;	/*extension channel above or below */
	UCHAR MCS;
	UCHAR BW;
	UCHAR STBC;
	UCHAR SHORTGI;
	UCHAR Channel;
	UCHAR BandIdx;
} OID_SET_HT_PHYMODE, *POID_SET_HT_PHYMODE;


#ifdef LLTD_SUPPORT
typedef struct _RT_LLTD_ASSOICATION_ENTRY {
	UCHAR Addr[MAC_ADDR_LEN];
	unsigned short MOR;	/* maximum operational rate */
	UCHAR phyMode;
} RT_LLTD_ASSOICATION_ENTRY, *PRT_LLTD_ASSOICATION_ENTRY;

typedef struct _RT_LLTD_ASSOICATION_TABLE {
	unsigned int Num;
	RT_LLTD_ASSOICATION_ENTRY Entry[MAX_NUMBER_OF_MAC];
} RT_LLTD_ASSOICATION_TABLE, *PRT_LLTD_ASSOICATION_TABLE;
#endif /* LLTD_SUPPORT */


#ifdef WSC_INCLUDED
#define RT_WSC_UPNP_EVENT_FLAG		0x109
#endif /* WSC_INCLUDED */



/*#define MAX_CUSTOM_LEN 128 */


typedef struct _RT_CHANNEL_LIST_INFO {
	UCHAR ChannelList[MAX_NUM_OF_CHS];	/* list all supported channels for site survey */
	UCHAR ChannelListNum;	/* number of channel in ChannelList[] */
} RT_CHANNEL_LIST_INFO, *PRT_CHANNEL_LIST_INFO;


/* WSC configured credential */
typedef struct _WSC_CREDENTIAL {
	NDIS_802_11_SSID SSID;	/* mandatory */
	USHORT AuthType;	/* mandatory, 1: open, 2: wpa-psk, 4: shared, 8:wpa, 0x10: wpa2, 0x20: wpa2-psk */
	USHORT EncrType;	/* mandatory, 1: none, 2: wep, 4: tkip, 8: aes */
	UCHAR Key[64];		/* mandatory, Maximum 64 byte */
	USHORT KeyLength;
	UCHAR MacAddr[MAC_ADDR_LEN];	/* mandatory, AP MAC address */
	UCHAR KeyIndex;		/* optional, default is 1 */
	UCHAR bFromUPnP;	/* TRUE: This credential is from external UPnP registrar */
#ifndef CONFIG_MAP_SUPPORT
	UCHAR Rsvd[2];		/* Make alignment */
#else
	UCHAR bss_role;		/*0-Fronthaul, 1-Backhaul*/
	UCHAR DevPeerRole;	/* Device role for the peer device sending M8 */
#endif
} WSC_CREDENTIAL, *PWSC_CREDENTIAL;

/* WSC configured profiles */
typedef struct _WSC_PROFILE {
	UINT ProfileCnt;
	UINT ApplyProfileIdx;	/* add by johnli, fix WPS test plan 5.1.1 */
	WSC_CREDENTIAL Profile[8];	/* Support up to 8 profiles */
} WSC_PROFILE, *PWSC_PROFILE;


#ifdef DOT11R_FT_SUPPORT
typedef struct _FT_CONFIG_INFO {
	UCHAR MdId[2];
	UCHAR R0KHId[49];
	UCHAR R0KHIdLen;
	BOOLEAN FtSupport;
	BOOLEAN FtRicSupport;
	BOOLEAN FtOtdSupport;
} FT_CONFIG_INFO, *PFT_CONFIG_INFO;
#endif /* DOT11R_FT_SUPPORT */







#ifdef OFFCHANNEL_SCAN_FEATURE
enum channel_monitor_return_code {
	CHANNEL_MONITOR_STRG_FAILURE = 0,
	CHANNEL_MONITOR_STRG_SUCCESS,
	CHANNEL_MONITOR_STRG_INVALID_ARG,
};

#endif


enum {
	OID_WIFI_TEST_BBP = 0x1000,
	OID_WIFI_TEST_RF = 0x1001,
	OID_WIFI_TEST_RF_BANK = 0x1002,
	OID_WIFI_TEST_MEM_MAP_INFO = 0x1003,
	OID_WIFI_TEST_BBP_NUM = 0x1004,
	OID_WIFI_TEST_RF_NUM = 0x1005,
	OID_WIFI_TEST_RF_BANK_OFFSET = 0x1006,
	OID_WIFI_TEST_MEM_MAP_NUM = 0x1007,
	OID_WIFI_TEST_BBP32 = 0x1008,
	OID_WIFI_TEST_MAC = 0x1009,
	OID_WIFI_TEST_MAC_NUM = 0x1010,
	OID_WIFI_TEST_E2P = 0x1011,
	OID_WIFI_TEST_E2P_NUM = 0x1012,
	OID_WIFI_TEST_PHY_MODE = 0x1013,
	OID_WIFI_TEST_RF_INDEX = 0x1014,
	OID_WIFI_TEST_RF_INDEX_OFFSET = 0x1015,
};

struct bbp_info {
	UINT32 bbp_start;
	UINT32 bbp_end;
	UINT8 bbp_value[0];
};

struct bbp32_info {
	UINT32 bbp_start;
	UINT32 bbp_end;
	UINT32 bbp_value[0];
};

struct rf_info {
	UINT16 rf_start;
	UINT16 rf_end;
	UINT8 rf_value[0];
};

struct rf_bank_info {
	UINT8 rf_bank;
	UINT16 rf_start;
	UINT16 rf_end;
	UINT8 rf_value[0];
};

struct rf_index_info {
	UINT8 rf_index;
	UINT16 rf_start;
	UINT16 rf_end;
	UINT32 rf_value[0];
};

struct mac_info {
	UINT32 mac_start;
	UINT32 mac_end;
	UINT32 mac_value[0];
};

struct mem_map_info {
	UINT32 base;
	UINT16 mem_map_start;
	UINT16 mem_map_end;
	UINT32 mem_map_value[0];
};

struct e2p_info {
	UINT16 e2p_start;
	UINT16 e2p_end;
	UINT16 e2p_value[0];
};

struct phy_mode_info {
	int data_phy;
	UINT8 data_bw;
	UINT8 data_ldpc;
	UINT8 data_mcs;
	UINT8 data_gi;
	UINT8 data_stbc;
};

struct anqp_req_data {
	UINT32 ifindex;
	UCHAR peer_mac_addr[6];
	UINT32 anqp_req_len;
	UCHAR anqp_req[0];
};

struct anqp_rsp_data {
	UINT32 ifindex;
	UCHAR peer_mac_addr[6];
	UINT16 status;
	UINT32 anqp_rsp_len;
	UCHAR anqp_rsp[0];
};

struct hs_onoff {
	UINT32 ifindex;
	UCHAR hs_onoff;
	UCHAR event_trigger;
	UCHAR event_type;
};

struct wapp_param_setting {
	UINT32 param;
	UINT32 value;
};

struct location_IE {
	UINT16 type;
	UINT8 len;
	UCHAR location_buf[0];
};

struct proxy_arp_entry {
	UINT32 ifindex;
	UCHAR ip_type;
	UCHAR from_ds;
	UCHAR IsDAD;
	UCHAR source_mac_addr[6];
	UCHAR target_mac_addr[6];
	UCHAR ip_addr[0];
};


struct security_type {
	UINT32 ifindex;
	UINT8 auth_mode;
	UINT8 encryp_type;
};

struct wnm_req_data {
	UINT32 ifindex;
	UCHAR peer_mac_addr[6];
	UINT32 type;
	UINT32 wnm_req_len;
	UCHAR wnm_req[0];
};

struct qosmap_data {
	UINT32 ifindex;
	UCHAR peer_mac_addr[6];
	UINT32 qosmap_len;
	UCHAR qosmap[0];
};


#define OID_802_11_WIFI_VER                     0x0920
#define OID_802_11_WAPP_SUPPORT_VER             0x0921
#define OID_802_11_WAPP_IE                      0x0922
#define OID_802_11_HS_ANQP_REQ                  0x0923
#define OID_802_11_HS_ANQP_RSP                  0x0924
#define OID_802_11_HS_ONOFF                     0x0925
#define OID_802_11_WAPP_PARAM_SETTING           0x0927
#define OID_802_11_WNM_BTM_REQ                  0x0928
#define OID_802_11_WNM_BTM_QUERY                0x0929
#define OID_802_11_WNM_BTM_RSP                  0x093a
#define OID_802_11_WNM_PROXY_ARP                0x093b
#define OID_802_11_WNM_IPV4_PROXY_ARP_LIST      0x093c
#define OID_802_11_WNM_IPV6_PROXY_ARP_LIST      0x093d
#define OID_802_11_SECURITY_TYPE                0x093e
#define OID_802_11_HS_RESET_RESOURCE            0x093f
#define OID_802_11_HS_AP_RELOAD                 0x0940
#define OID_802_11_HS_BSSID                     0x0941
#define OID_802_11_HS_OSU_SSID                  0x0942
/* #define OID_802_11_HS_OSU_NONTX               0x0944 */
#define OID_802_11_HS_SASN_ENABLE               0x0943
#define OID_802_11_WNM_NOTIFY_REQ               0x0944
#define OID_802_11_QOSMAP_CONFIGURE             0x0945
#define OID_802_11_GET_STA_HSINFO				0x0946
#define OID_802_11_BSS_LOAD						0x0947
#define OID_802_11_HS_LOCATION_DRV_INFORM_IE	0x0948
#define OID_802_11_INTERWORKING_ENABLE			0x0949


#define OID_BNDSTRG_MSG							0x0950
#define OID_BNDSTRG_GET_NVRAM					0x0951
#define OID_BNDSTRG_SET_NVRAM					0x0952

#define OID_802_11_MBO_MSG						0x0953
#define OID_NEIGHBOR_REPORT						0x0954

#ifdef OFFCHANNEL_SCAN_FEATURE
#define OID_OFFCHANNEL_INFO				0x0955
#define OID_802_11_CURRENT_CHANNEL_INFO 0x0956
#define OID_OPERATING_INFO				0x0957
#define OID_802_11_CHANNELINFO			0x0999
#endif

#define OID_WSC_UUID				0x0990
#define OID_SET_SSID				0x0992
#define OID_SET_PSK				0x0993

#ifdef DFS_VENDOR10_CUSTOM_FEATURE
#define OID_GET_RXDATA_LAPSE_TIME		0x0958
#endif

#ifdef VENDOR10_CUSTOM_RSSI_FEATURE
#define OID_GET_CURRENT_RSSI            0x0959
#define OID_SET_VENDOR10_RSSI			0x0980
#endif

/* for VOW SUPPORT web UI */
#ifdef CONFIG_AP_SUPPORT
#ifdef VOW_SUPPORT

/* not used for runtime enable/disable */
#define OID_802_11_VOW_BW_EN                    0x0960
#define OID_802_11_VOW_BW_AT_EN                 0x0961
#define OID_802_11_VOW_BW_TPUT_EN               0x0962

#define OID_802_11_VOW_ATF_EN                   0x0963
#define OID_802_11_VOW_RX_EN                    0x0964

#define OID_802_11_VOW_GROUP_MAX_RATE           0x0965
#define OID_802_11_VOW_GROUP_MIN_RATE           0x0966
#define OID_802_11_VOW_GROUP_MAX_RATIO          0x0967
#define OID_802_11_VOW_GROUP_MIN_RATIO          0x0968
#endif /* VOW_SUPPORT */
#ifdef CUSTOMER_DCC_FEATURE
#define OID_802_11_SCAN_BSSID_LIST              0x069b
#ifdef MEMORY_OPTIMIZATION
#define MAX_LEN_OF_BSS_TABLE             1
#define MAX_REORDERING_MPDU_NUM			 256
#else
#define MAX_LEN_OF_BSS_TABLE             256 /* 64 */
#define MAX_LEN_OF_BSS_SCAN_TABLE		128 /* Customer Chnages */
#if defined(MT7615) || defined(MT7622)
#define MAX_REORDERING_MPDU_NUM			 (512 * 4)
#else
#define MAX_REORDERING_MPDU_NUM			 512
#endif
#endif
typedef struct _DCC_CH {
	UINT8	ChannelNo;
	UINT32 	FalseCCA;
	UINT32	chanbusytime;
} DCC_CH, *PDCC_CH;

typedef struct _WLAN_BSSID_LIST {
   NDIS_802_11_MAC_ADDRESS        	Bssid;		/* BSSID */
   UCHAR 				WpsAP;		/* 0x00: not support WPS, 0x01: support normal WPS, 0x02: support Ralink auto WPS, 0x04: support Samsung WAC */
   USHORT				WscDPIDFromWpsAP;
   UCHAR				Privacy;    	/* WEP encryption requirement */
   UINT                    		Signal;       	/* receive signal strength in dBm */
   NDIS_802_11_NETWORK_TYPE    		wireless_mode;
   NDIS_802_11_AUTHENTICATION_MODE	AuthMode;
   NDIS_802_11_WEP_STATUS		Cipher;		/* Unicast Encryption Algorithm extract from VAR_IE */
   UINT8 				Channel;
   UINT8				ExtChannel;
   UCHAR   				BssType;
   UCHAR				SupportedRates[MAX_LENGTH_OF_SUPPORT_RATES];
   BOOLEAN     				bQbssLoadValid; /* 1: variable contains valid value */
   USHORT      				QbssLoadStaNum;
   UCHAR       				QbssLoadChannelUtilization;
   UCHAR  				Snr[4];
   UINT8				NumSpatialStream;
   CHAR					rssi[4];
   UCHAR  				vendorOUI0[3];
   UCHAR  				vendorOUI1[3];
   UINT8			   	ChannelWidth;
   NDIS_802_11_SSID			Ssid;		/* SSID */
} WLAN_BSSID_LIST, *PWLAN_BSSID_LIST;

typedef struct  _BSSID_LIST_RESULTS {
   UINT           			NumberOfItems;      	/* in list below, at least 1 */
   WLAN_BSSID_LIST 			BssidTable[MAX_LEN_OF_BSS_TABLE];
   DCC_CH				DccCh[30]; 		/*anand to handle 5g and 2g both channels */
} BSSID_LIST_RESULTS, *PBSSID_LIST_RESULTS;
#endif /* CUSTOMER_DCC_FEATURE */

#endif /* CONFIG_AP_SUPPORT */

#ifdef WIFI_SPECTRUM_SUPPORT
#define OID_802_11_WIFISPECTRUM_SET_PARAMETER				0x0970
#define OID_802_11_WIFISPECTRUM_GET_CAPTURE_STATUS			0x0971
#define OID_802_11_WIFISPECTRUM_DUMP_DATA                	0x0972
#define OID_802_11_WIFISPECTRUM_GET_CAPTURE_BW           	0x0973
#define OID_802_11_WIFISPECTRUM_GET_CENTRAL_FREQ         	0x0974
#endif /* WIFI_SPECTRUM_SUPPORT */

#ifdef CUSTOMER_RSG_FEATURE
#define OID_802_11_GET_CURRENT_CHANNEL_FALSE_CCA_AVG		0x0690
#define OID_802_11_GET_CURRENT_CHANNEL_CST_TIME_AVG		0x0691
#define	OID_802_11_GET_CURRENT_CHANNEL_BUSY_TIME_AVG		0x0692
#define OID_802_11_GET_CURRENT_CHANNEL_STATS 			0x0693
#define OID_802_11_GET_CURRENT_CHANNEL_AP_ACTIVITY_AVG  	0x0694
#define	OID_802_11_GET_RADIO_STATS_COUNT			0x0699

typedef struct _CURRENT_CHANNEL_STATS{
 UINT32		SamplePeriod;
 UINT32		FalseCCACount;
 UINT32  	ChannelApActivity;
 UINT32  	EdCcaBusyTime;
 UINT32   	ChannelBusyTime;
} CURRENT_CHANNEL_STATISTICS, *PCURRENT_CHANNEL_STATISTICS;

#endif

#ifdef CUSTOMER_DCC_FEATURE
#define OID_802_11_STA_STATISTICS				0x0695
#define OID_802_11_GET_CURRENT_CHANNEL_AP_TABLE			0x0696
#define OID_802_11_GET_ACCESS_CATEGORY_TRAFFIC_STATUS		0x0697
#define OID_802_11_GET_SCAN_RESULTS				0x0698
#define	OID_802_11_GET_RADIO_STATS_COUNT			0x0699
#define OID_802_11_MBSS_STATISTICS				0x069a
#define OID_802_11_SCAN_BSSID_LIST				0x069b

typedef struct __RT_MBSS_STAT_ENTRY{
	/* DATA counter */
	UINT32 RxCount;
	UINT32 TxCount;
	UINT32 ReceivedByteCount;
	UINT32 TransmittedByteCount;
	UINT32 RxErrorCount;
	UINT32 RxDropCount;
	UINT32 TxErrorCount;
	UINT32 TxDropCount;
	UINT32 UnicastPktsRx;
	UINT32 UnicastPktsTx;
	UINT32 MulticastPktsRx;
	UINT32 MulticastPktsTx;
	UINT32 BroadcastPktsRx;
	UINT32 BroadcastPktsTx;
	UINT32 TxRetriedPktCount;
	UINT32 ChannelUseTime;
	/* MGMT counter */
	UINT32 MGMTRxCount;
	UINT32 MGMTTxCount;
	UINT32 MGMTReceivedByteCount;
	UINT32 MGMTTransmittedByteCount;
	UINT32 MGMTRxErrorCount;
	UINT32 MGMTRxDropCount;
	UINT32 MGMTTxErrorCount;
	UINT32 MGMTTxDropCount;
} RT_MBSS_STAT_ENTRY;

typedef struct  __RT_MBSS_STATISTICS_TABLE{
	UINT32				Num;
	RT_MBSS_STAT_ENTRY	MbssEntry[8];
} RT_MBSS_STATISTICS_TABLE;

typedef struct  __RT_STA_STAT_ENTRY {
	UCHAR  ApIdx;
	UCHAR  Addr[MAC_ADDR_LEN];
	UINT32 RxCount;
	UINT32 TxCount;
	UINT32 ReceivedByteCount;
	UINT32 TransmittedByteCount;
	UINT32 RxErrorCount;
	UINT32 RxDropCount;
	UINT32 TxErrorCount;
	UINT32 TxDropCount;
	UINT32 TxRetriedPktCount;
	UINT32 ChannelUseTime;
} RT_STA_STAT_ENTRY;

typedef struct  __RT_STA_STATISTICS_TABLE {
	int Num;
	RT_STA_STAT_ENTRY	STAEntry[MAX_NUMBER_OF_MAC];
} RT_STA_STATISTICS_TABLE;

typedef struct _BSS_ENTRY_TABLE {
	UINT8   Bssid[MAC_ADDR_LEN];
	UINT16  SsidLen;
	UINT8	Ssid[33];
	UINT8 	Channel;
	UINT8	ChannelWidth;
	UINT8	ExtChannel;
	CHAR	RSSI;
	UINT8	SNR;
	UINT8	PhyMode;
	UINT8	NumSpatialStream;
} BSS_ENTRY_TABLE, *PBSS_ENTRY_TABLE;

typedef struct _BEACON_TABLE {
	UINT8 Num;
	BSS_ENTRY_TABLE BssTable[MAX_LEN_OF_BSS_TABLE];
} BEACON_TABLE, *PBEACON_TABLE;

typedef struct _SCAN_RESULTS {
	UINT32	ch_busy_time;
	UINT32	cca_err_cnt;
	UINT32	num_ap;
	BSS_ENTRY_TABLE BssTable[MAX_LEN_OF_BSS_TABLE];
} SCAN_RESULTS, *PSCAN_RESULTS;

#endif


#ifdef MT_DFS_SUPPORT
#define OID_DFS_ZERO_WAIT                       0x0985
#endif

#define OID_802_11_PROBE_RESPONSE		0x0986



#define MAX_CANDIDATE_NUM 5
#define OP_LEN 16
#define CH_LEN 30
#define REQ_LEN 30
#define SSID_LEN 33

#define OID_802_11_RRM_COMMAND  0x094C
#define OID_802_11_RRM_EVENT	0x094D
#define RT_QUERY_RRM_CAPABILITY	\
		(OID_GET_SET_FROM_UI|OID_802_11_RRM_COMMAND)

enum rrm_cmd_subid {
	OID_802_11_RRM_CMD_ENABLE = 0x01,
	OID_802_11_RRM_CMD_CAP,
	OID_802_11_RRM_CMD_SEND_BEACON_REQ,
	OID_802_11_RRM_CMD_QUERY_CAP,
	OID_802_11_RRM_CMD_SET_BEACON_REQ_PARAM,
	OID_802_11_RRM_CMD_SEND_NEIGHBOR_REPORT,
	OID_802_11_RRM_CMD_SET_NEIGHBOR_REPORT_PARAM,
	OID_802_11_RRM_CMD_HANDLE_NEIGHBOR_REQUEST_BY_DAEMON,
};

enum rrm_event_subid {
	OID_802_11_RRM_EVT_BEACON_REPORT = 0x01,
	OID_802_11_RRM_EVT_NEIGHBOR_REQUEST,
};

typedef struct GNU_PACKED rrm_command_s {
	UINT8 command_id;
	UINT32 command_len;
	UINT8 command_body[0];
} rrm_command_t, *p_rrm_command_t;

typedef struct GNU_PACKED rrm_event_s {
	UINT8 event_id;
	UINT32 event_len;
	UINT8 event_body[0];
} rrm_event_t, *p_rrm_event_t;

struct GNU_PACKED nr_req_data {
	UINT32 ifindex;
	UCHAR peer_mac_addr[6];
	UINT32 nr_req_len;
	UCHAR nr_req[0];
};

typedef struct GNU_PACKED nr_rsp_data_s {
	UINT32 ifindex;
	UINT8 dialog_token;
	UINT8 peer_address[MAC_ADDR_LEN];
	UINT32 nr_rsp_len;
	UINT8 nr_rsp[0];
} nr_rsp_data_t, *p_nr_rsp_data_t;

typedef struct GNU_PACKED bcn_req_data_s {
	UINT32 ifindex;
	UINT8 dialog_token;
	UINT8 peer_address[MAC_ADDR_LEN];
	UINT32 bcn_req_len;
	UINT8 bcn_req[0];
} bcn_req_data_t, *p_bcn_req_data_t;

/** @peer_address: mandatory; sta to send beacon request frame;
*      @num_rpt: optional; number of repetitions;
*      @regclass: only mandatory when channel is set to 0; operating class;
*      @channum: mandatory; channel number;
*      @random_ivl: optional; randomization interval; unit ms;
*		the upper bound of the random delay to be used prior to make measurement;
*      @duration: optional; measurement duration; unit ms;
*      @bssid: optional;
*      @mode: optional; measurement mode;
*      As default value 0 is a valid value in spec, so here need remap the value and the meaning;
*      1 for passive mode; 2 for active mode; 3 for beacon table;
*     @req_ssid: optional; subelement SSID;
*      @timeout: optional; unit s;
*      @rep_conditon: optional; subelement Beacon Reporting Information;
*      @ref_value: optional; subelement Beacon Reporting Information;
*      condition for report to be issued;
*      driver will send timeout event after timeout value if no beacon report received;
*      @detail: optional; subelement Reporting Detail;
*       As default value 0 is a valid value in spec, so here need remap the value and the meaning;
*      1 for no fixed length fields or elements;
*      2 for all fixed length fields and any requested elements in the request IE;
*      3 for all fixed length fields and elements
*      @op_class_len:  mandatory only when channel is set to 255;
*      @op_class_list: subelement Ap Channel Report;
*      @ch_list_len: mandatory only when channel is set to 255;
*      @ch_list: subelement Ap Channel Report;
*      if you want use all the channels in operating classes then use default value
*      otherwise specify all channels you want sta to do measurement
*      @request_len: optional;
*      @request: subelement Request; only valid when you specify request IDs
*/
typedef struct GNU_PACKED bcn_req_info_s {
	UINT8 peer_address[MAC_ADDR_LEN];
	UINT16 num_rpt;
	UINT8 regclass;
	UINT8 channum;
	UINT16 random_ivl;
	UINT16 duration;
	UINT8 bssid[MAC_ADDR_LEN];
	UINT8 mode;
	UINT8 req_ssid_len;
	UINT8 req_ssid[SSID_LEN];
	UINT32 timeout;
	UINT8 rep_conditon;
	UINT8 ref_value;
	UINT8 detail;
	UINT8 op_class_len;
	UINT8 op_class_list[OP_LEN];
	UINT8 ch_list_len;
	UINT8 ch_list[CH_LEN];
	UINT8 request_len;
	UINT8 request[REQ_LEN];
} bcn_req_info, *p_bcn_req_info;

typedef struct GNU_PACKED bcn_rsp_data_s {
	UINT8   dialog_token;
	UINT32  ifindex;
	UINT8   peer_address[6];
	UINT32  bcn_rsp_len;
	UINT8   bcn_rsp[0];
} bcn_rsp_data_t, *p_bcn_rsp_data_t;

/**
*	@channum: optional; channel number;
*	@phytype: optional; PHY type;
*	@regclass: optional; operating class;
*	@capinfo: optional; Same as AP's Capabilities Information field in Beacon;
*	@bssid: mandatory;
*	@preference: not used in neighbor report; optional in btm request;
*	 indicates the network preference for BSS transition to the BSS listed in this
*	 BSS Transition Candidate List Entries; 0 is a valid value in spec, but here
*	 need remap its meaning to not include preference IE in neighbor report
*	 response frame;
*	@is_ht:  optional; High Throughput;
*	@is_vht: optional; Very High Throughput;
*	@ap_reachability: optional; indicates whether the AP identified by this BSSID is
*	reachable by the STA that requested the neighbor report. For example,
*	the AP identified by this BSSID is reachable for the exchange of
*	preauthentication frames;
*	@security: optional;  indicates whether the AP identified by this BSSID supports
*	the same security provisioning as used by the STA in its current association;
*	@key_scope: optional; indicates whether the AP indicated by this BSSID has the
*	same authenticator as the AP sending the report;
*	@Mobility: optional; indicate whether the AP represented by this BSSID is
*	including an MDE in its Beacon frames and that the contents of that MDE are
*	identical to the MDE advertised by the AP sending the report;
*/
struct GNU_PACKED nr_info {
	UINT8 channum;
	UINT8 phytype;
	UINT8 regclass;
	UINT16 capinfo;
	UINT8 bssid[MAC_ADDR_LEN];
	UINT8 preference;
	UINT8 is_ht;
	UINT8 is_vht;
	UINT8 ap_reachability;
	UINT8 security;
	UINT8 key_scope;
	UINT8 mobility;
};

/**
*	@dialogtoken: mandatory; must the same with neighbor request from sta
*	or 0 on behalf of automatic report
*	@nrresp_info_count: mandatory; the num of  neighbor elements;must bigger
*	than 0 and not exceeds 5;
*	@nrresp_info: info of neighbor elements; mandatory;
*/
typedef struct GNU_PACKED rrm_nrrsp_info_custom_s {
	UINT8 peer_address[MAC_ADDR_LEN];
	UINT8 dialogtoken;
	UINT8 nrresp_info_count;
	struct nr_info nrresp_info[0];
} rrm_nrrsp_info_custom_t, *p_rrm_nrrsp_info_custom_t;


/*#ifdef WNM_NEW_API*/
#define URL_LEN 40

#define OID_802_11_WNM_COMMAND  0x094A
#define OID_802_11_WNM_EVENT	0x094B
#define RT_QUERY_WNM_CAPABILITY	\
	(OID_GET_SET_FROM_UI|OID_802_11_WNM_COMMAND)


enum wnm_cmd_subid {
	OID_802_11_WNM_CMD_ENABLE = 0x01,
	OID_802_11_WNM_CMD_CAP,
	OID_802_11_WNM_CMD_SEND_BTM_REQ,
	OID_802_11_WNM_CMD_QUERY_BTM_CAP,
	OID_802_11_WNM_CMD_SEND_BTM_REQ_IE,
	OID_802_11_WNM_CMD_SET_BTM_REQ_PARAM,
};

enum wnm_event_subid {
	OID_802_11_WNM_EVT_BTM_QUERY = 0x01,
	OID_802_11_WNM_EVT_BTM_RSP,
};

struct GNU_PACKED wnm_command {
	UINT8 command_id;
	UINT8 command_len;
	UINT8 command_body[0];
};

struct GNU_PACKED wnm_event {
	UINT8 event_id;
	UINT32 event_len;
	UINT8 event_body[0];
};

typedef struct GNU_PACKED btm_req_ie_data_s {
	UINT32 ifindex;
	UINT8 peer_mac_addr[6];
	UINT8 dialog_token;
	UINT32 timeout;
	UINT32 btm_req_len;
	UINT8 btm_req[0];
} btm_req_ie_data_t, *p_btm_req_ie_data_t;

struct GNU_PACKED btm_req_data {
	UINT32 ifindex;
	UCHAR peer_mac_addr[6];
	UINT32 btm_req_len;
	UCHAR btm_req[0];
};

struct GNU_PACKED btm_query_data {
	UINT32 ifindex;
	UCHAR peer_mac_addr[6];
	UINT32 btm_query_len;
	UCHAR btm_query[0];
};

struct GNU_PACKED btm_rsp_data {
	UINT32 ifindex;
	UCHAR peer_mac_addr[6];
#ifdef CONFIG_STEERING_API_SUPPORT
	UINT32 timestamp;
#endif
	UINT32 btm_rsp_len;
	UCHAR btm_rsp[0];
};

struct GNU_PACKED reduced_neighbor_list_data {
    u32 ifindex;
    u32 reduced_neighbor_list_len;
    char reduced_neighbor_list_req[0];
};

struct GNU_PACKED neighbor_list_data {
    u32 ifindex;
    u32 neighbor_list_len;
    char neighbor_list_req[0];
};


/**
*	@sta_mac: mandatory; mac of sta sending the frame;
*	@dialogtoken: optional; dialog token;
*	@reqmode: optional; request mode;
*	@disassoc_timer: optional; the time(TBTTs) after which the AP will issue
*		a Disassociation frame to this STA;
*	@valint: optional;  the number of beacon transmission times (TBTTs) until
*		the BSS transition candidate list is no longer valid;
*	@timeout: optional; driver will send timeout event after timeout value
*		if no beacon report received; unit s;
*	@TSF: optional; BSS Termination TSF;
*	@duration: optional; number of minutes for which the BSS is not present;
*	@url_len: optional;
*	@url: optional; only valid when you specify url;
*	@num_candidates: mandatory; num of candidates;
*	@candidates: mandatory; request mode; the num of candidate is no larger
*		than 5;
*/
typedef struct GNU_PACKED btm_reqinfo_s {
	UINT8 sta_mac[MAC_ADDR_LEN];
	UINT8 dialogtoken;
	UINT8 reqmode;
	UINT16 disassoc_timer;
	UINT8 valint;
	UINT32 timeout;
	UINT64 TSF;
	UINT16 duration;
	UINT8 url_len;
	UINT8 url[URL_LEN];
	UINT8 num_candidates;
	struct nr_info candidates[0];
} btm_reqinfo_t, *p_btm_reqinfo_t;

struct GNU_PACKED vie_op_data_s
{
	UINT32 frm_type_map;
	ULONG vie_length;/*the total length which starts from oui until the tail.*/
	UCHAR oui_oitype[4];/*used to comparism*/
	/*if add/remove/update, pass the content to driver. if show, repoort the content to user space.*/
	UCHAR app_ie_ctnt[255];/*ugly hard-code size.*/
};

#ifdef CHANNEL_SWITCH_MONITOR_CONFIG
#define OID_SET_CH_SWITCH_DURATION		0x1803
#define CH_SWITCH_MONITOR_DONE_EVENT_FLAG	0x1804
#endif
#ifdef CONFIG_STEERING_API_SUPPORT
#ifndef HASH_TABLE_SIZE
#define HASH_TABLE_SIZE                 256	/* Size of hash tab must be power of 2. */
#endif

#define	OID_802_11_BTM_ACTION_FRAME_REQUEST		0x06b0
#define OID_STEER_ACTION						0x06b1
#define OID_802_11_GET_OPERATING_CLASS			0x06b2
#define OID_NON_ASSOCIATED_STA_REPORT_CAPABLE   0x06b3
#define OID_NON_ASSOCIATED_STA_REPORT_ENABLE	0x06b4
#define OID_NON_ASSOCIATED_STA_REPORT			0x06b5
/* #define OID_802_11_BTM_ACTION_FRAME_RESPONSE		0x06b1 */

#define BLOCKED_LIST_MAX_TABLE_SIZE			128
#define NA_STA_PROBE_DATA_LIST_SIZE 		4
#define NA_STA_REPORT_SIZE 					2


typedef struct _BLACKLIST_ACTION {
	UINT Action;
	UINT Timer;
}BLACKLIST_ACTION, *PBLACKLIST_ACTION;

typedef struct _STEER_ACTION_TYPE {
	UCHAR 				StaMac[MAC_ADDR_LEN];
	INT   				Type;
	BLACKLIST_ACTION 	BlkListAction;
}STEER_ACTION_TYPE, *PSTEER_ACTION_TYPE;

typedef struct _BTM_REQUEST_ACTION_FRAME {
	UCHAR CategoryType[4];
	UCHAR MacAddr[MAC_ADDR_LEN];
	UCHAR Payload[1024];
}BTM_REQUEST_ACTION_FRAME, *PBTM_REQUEST_ACTION_FRAME;

#endif

#ifdef CUSTOMER_VENDOR_IE_SUPPORT
struct customer_vendor_ie {
	NDIS_SPIN_LOCK vendor_ie_lock;
	CHAR *pointer;
	UINT16 length;
};

struct customer_oui_filter {
	NDIS_SPIN_LOCK oui_filter_lock;
	CHAR *pointer;
	UINT16 length;
};

struct vendor_ie {
	UCHAR element_id;
	UCHAR len;
	UCHAR custom_ie[255];
};

struct probe_req_report {
	UCHAR band;/*0:2.4G, 1:5G*/
	UCHAR sta_mac[MAC_ADDR_LEN];
	struct vendor_ie vendor_ie;
};

#endif /* CUSTOMER_VENDOR_IE_SUPPORT */

#ifdef APCLI_OWE_SUPPORT
#define OID_802_11_OWE_TRANS_COMMAND  0x0990
#define OID_802_11_OWE_TRANS_EVENT	0x0991

enum owe_event_subid {
	OID_802_11_OWE_EVT_DIFF_BAND = 0x01,
	OID_802_11_OWE_EVT_SAME_BAND_DIFF_CHANNEL = 0x02,
};

struct GNU_PACKED owe_event {
	UINT8 event_id;
	UINT32 event_len;
	UINT8 event_body[0];
};

struct GNU_PACKED owe_trans_channel_change_info {
	UCHAR ifname[IFNAMSIZ];
	UCHAR pair_bssid[MAC_ADDR_LEN];
	UCHAR pair_ssid[NDIS_802_11_LENGTH_SSID];
	UCHAR pair_ssid_len;
	UCHAR pair_band;
	UCHAR pair_ch;
};

#endif

#define OID_802_11_CHAN_SANITY_FAIL_EVENT  0x09A0

#define OID_GET_CPU_TEMPERATURE 0x09A1

#define OID_QUERY_FEATURE_SUP_LIST 0x09A2

#define OID_GET_WSC_PROFILES 0x0994
#define OID_GET_MISC_CAP								0x0995
#define OID_GET_HT_CAP									0x0996
#define OID_GET_VHT_CAP									0x0997
#define OID_GET_CHAN_LIST								0x0998
#define OID_GET_OP_CLASS								0x0999
#define OID_GET_BSS_INFO								0x099A
#define OID_GET_AP_METRICS								0x099B
#define OID_GET_NOP_CHANNEL_LIST						0x099C

#ifdef ACS_CTCC_SUPPORT
#define OID_802_11_GET_ACS_CHANNEL_SCORE                0x2014

struct auto_ch_sel_score {
	UINT32 score;
	UINT32 channel;
};
struct auto_ch_sel_grp_member {
	UINT32 busy_time;
	UINT32 channel;
};
struct acs_channel_score {
 struct auto_ch_sel_score acs_channel_score[MAX_NUM_OF_CHANNELS+1];
 UINT32 acs_alg;
};
#endif
#endif /* _OID_H_ */


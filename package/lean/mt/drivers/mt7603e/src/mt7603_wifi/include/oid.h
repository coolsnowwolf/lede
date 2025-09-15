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

/* BW */
#define BAND_WIDTH_20		0
#define BAND_WIDTH_40		1
#define BAND_WIDTH_80		2
#define BAND_WIDTH_160	    3
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
/*#define MAX_NUM_OF_CHS             		54 */ /* 14 channels @2.4G +  12@UNII(lower/middle) + 16@HiperLAN2 + 11@UNII(upper) + 0 @Japan + 1 as NULL termination */
#define MAX_NUMBER_OF_EVENT				10	/* entry # in EVENT table */

/*JDC
#if defined (CONFIG_RT_FIRST_IF_MT7603E) && defined (CONFIG_RT_SECOND_IF_MT7615E)
#define CONFIG_RT_MAX_CLIENTS           48
#endif
#define CONFIG_MAX_CLIENTS CONFIG_RT_MAX_CLIENTS

#define MAX_NUMBER_OF_MAC			CONFIG_MAX_CLIENTS
*/
#ifdef BB_SOC
#define MAX_NUMBER_OF_MAC				16 // if MAX_MBSSID_NUM is 8, this value can't be larger than 211
#else
#if defined(MT7603_FPGA) || defined(MT7628_FPGA) || defined(MT7636_FPGA)
#define MAX_NUMBER_OF_MAC				4	/* if MAX_MBSSID_NUM is 8, this value can't be larger than 211 */
#elif defined(MT7603)
#ifdef ECONET_ALPHA_RELEASE
#define MAX_NUMBER_OF_MAC				8
#else
#ifdef MAC_REPEATER_SUPPORT
#ifdef MULTI_APCLI_SUPPORT
#define MAX_NUMBER_OF_MAC				(75 - (16 + 1) * 2)
#else
#define MAX_NUMBER_OF_MAC				(75 - (16 + 1) * 1)
#endif /* MULTI_APCLI_SUPPORT */
#else
#define MAX_NUMBER_OF_MAC            	75
#endif /* MAC_REPEATER_SUPPORT */
#endif
#elif defined(MT76x2)
#ifdef MAC_REPEATER_SUPPORT				//((MAX_EXT_MAC_ADDR_SIZE + 1) * MAC_APCLI_NUM)
#define MAX_NUMBER_OF_MAC               (116 - ((16 + 1) * 1))
#else
#define MAX_NUMBER_OF_MAC			   	 116
#endif /* MAC_REPEATER_SUPPORT */
#else
#define MAX_NUMBER_OF_MAC				32	/* if MAX_MBSSID_NUM is 8, this value can't be larger than 211 */
#endif /* defined(MT7603_FPGA) || defined(MT7628_FPGA) */
#endif /* BB_SOC */

#define MAX_NUMBER_OF_ACL				64
#define MAX_LENGTH_OF_SUPPORT_RATES		12	/* 1, 2, 5.5, 11, 6, 9, 12, 18, 24, 36, 48, 54 */
#define MAX_NUMBER_OF_DLS_ENTRY			4

#ifdef MAC_REPEATER_SUPPORT
/*seems different chip have different size , use 16 (minimum size) tmply */
#define MAX_NUMBER_OF_MAC_LIST			16  /* MAX_EXT_MAC_ADDR_SIZE */
#endif /* MAC_REPEATER_SUPPORT */


#define RT_QUERY_SIGNAL_CONTEXT				0x0402
#define RT_SET_IAPP_PID                 	0x0404
#define RT_SET_APD_PID						0x0405
#define RT_SET_DEL_MAC_ENTRY				0x0406
#define RT_QUERY_EVENT_TABLE            	0x0407
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
#define OID_802_11_FROAM_ACL_ADD_ENTRY				0x252B
#define OID_802_11_FROAM_ACL_DEL_ENTRY				0x252C

#if (defined(VENDOR_FEATURE6_SUPPORT) || defined(MAP_SUPPORT))
#define OID_802_11_COEXISTENCE						0x0530
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
#define RT_OID_802_11_QUERY_WMM              		0x063E
#define RT_OID_802_11_SET_WMM      					0x063F
#define RT_OID_802_11_QUERY_IMME_BA_CAP				0x0640
#define RT_OID_802_11_SET_IMME_BA_CAP				0x0641
#define RT_OID_802_11_QUERY_BATABLE					0x0642
#define RT_OID_802_11_ADD_IMME_BA					0x0643
#define RT_OID_802_11_TEAR_IMME_BA					0x0644
#define RT_OID_DRIVER_DEVICE_NAME                   0x0645
#define RT_OID_802_11_QUERY_DAT_HT_PHYMODE          0x0646
#define OID_WAPP_EVENT						        0x0647
#define OID_802_11_SET_PSPXLINK_MODE				0x0648
/*+++ add by woody +++*/

#ifdef WH_EZ_SETUP
#define OID_802_11_PASSPHRASE						0x0649
#else
#define OID_802_11_SET_PASSPHRASE					0x0649
#endif
#define RT_OID_802_11_QUERY_TX_PHYMODE                          0x0650
#define RT_OID_802_11_QUERY_MAP_REAL_TX_RATE                          0x0678
#define RT_OID_802_11_QUERY_MAP_REAL_RX_RATE                          0x0679
#define	RT_OID_802_11_SNR_2							0x067A
#define RT_OID_802_11_PER_BSS_STATISTICS			0x067D
#define OID_802_11_MBSS_GET_STA_COUNT					0x067E



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
#define IEEE80211_1X_COPY_KEY        	7	/* copy static-wep unicast key */

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
#ifdef WH_EZ_SETUP
#define RT_OID_802_11_SET_PASSPHRASE		  (OID_GET_SET_TOGGLE | OID_802_11_PASSPHRASE)
#else
#define RT_OID_802_11_SET_PASSPHRASE		  (OID_GET_SET_TOGGLE | OID_802_11_SET_PASSPHRASE)
#endif

#ifdef DOT1X_SUPPORT
#define RT_OID_802_DOT1X_PMKID_CACHE		(OID_GET_SET_TOGGLE | OID_802_DOT1X_PMKID_CACHE)
#define RT_OID_802_DOT1X_RADIUS_DATA		(OID_GET_SET_TOGGLE | OID_802_DOT1X_RADIUS_DATA)
#define RT_OID_802_DOT1X_WPA_KEY			(OID_GET_SET_TOGGLE | OID_802_DOT1X_WPA_KEY)
#define RT_OID_802_DOT1X_STATIC_WEP_COPY	(OID_GET_SET_TOGGLE | OID_802_DOT1X_STATIC_WEP_COPY)
#define RT_OID_802_DOT1X_IDLE_TIMEOUT		(OID_GET_SET_TOGGLE | OID_802_DOT1X_IDLE_TIMEOUT)
#endif /* DOT1X_SUPPORT */

#define RT_OID_802_11_SET_TDLS_PARAM			(OID_GET_SET_TOGGLE | RT_OID_802_11_QUERY_TDLS_PARAM)
#define RT_OID_802_11_SET_TDLS				(OID_GET_SET_TOGGLE | RT_OID_802_11_QUERY_TDLS)


#ifdef WAPI_SUPPORT
#define OID_802_11_WAPI_PID					0x06A0
#define OID_802_11_PORT_SECURE_STATE		0x06A1
#define OID_802_11_UCAST_KEY_INFO			0x06A2
#define OID_802_11_MCAST_TXIV				0x06A3
#define OID_802_11_MCAST_KEY_INFO			0x06A4
#define OID_802_11_WAPI_CONFIGURATION		0x06A5
#define OID_802_11_WAPI_IE					0x06A6

#define RT_OID_802_11_WAPI_PID				(OID_GET_SET_TOGGLE | OID_802_11_WAPI_PID)
#define RT_OID_802_11_PORT_SECURE_STATE		(OID_GET_SET_TOGGLE | OID_802_11_PORT_SECURE_STATE)
#define RT_OID_802_11_UCAST_KEY_INFO		(OID_GET_SET_TOGGLE | OID_802_11_UCAST_KEY_INFO)
#define RT_OID_802_11_MCAST_TXIV			(OID_GET_SET_TOGGLE | OID_802_11_MCAST_TXIV)
#define RT_OID_802_11_MCAST_KEY_INFO		(OID_GET_SET_TOGGLE | OID_802_11_MCAST_KEY_INFO)
#define RT_OID_802_11_WAPI_CONFIGURATION	(OID_GET_SET_TOGGLE | OID_802_11_WAPI_CONFIGURATION)
#define RT_OID_802_11_WAPI_IE				(OID_GET_SET_TOGGLE | OID_802_11_WAPI_IE)
#endif /* WAPI_SUPPORT */
#ifdef ACL_BLK_COUNT_SUPPORT
#define OID_802_11_ACL_BLK_REJCT_COUNT_STATICS			0x069b
#endif/*ACL_BLK_COUNT_SUPPORT*/

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
#ifdef WH_EZ_SETUP
#define IS_AKM_PSK_Entry(_Entry)   (((_Entry)->AuthMode == Ndis802_11AuthModeWPAPSK) || ((_Entry)->AuthMode == Ndis802_11AuthModeWPA2PSK))
#endif
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
#define MAX_MBSSID_1X_NUM			16  /* stay correspondence with 802.1x daemon */

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


#if (defined(CONFIG_STA_SUPPORT) || defined(WH_EZ_SETUP))
typedef struct _NDIS_802_11_PASSPHRASE {
	UINT KeyLength;		/* length of key in bytes */
	NDIS_802_11_MAC_ADDRESS BSSID;
	UCHAR KeyMaterial[1];	/* variable length depending on above field */
} NDIS_802_11_PASSPHRASE, *PNDIS_802_11_PASSPHRASE;
#endif

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
#ifdef WAPI_SUPPORT
	Ndis802_11AuthModeWAICERT,	/* WAI certificate authentication */
	Ndis802_11AuthModeWAIPSK,	/* WAI pre-shared key */
#endif				/* WAPI_SUPPORT */
	Ndis802_11AuthModeWPA3PSK,
	Ndis802_11AuthModeWPA2PSKWPA3PSK,
	Ndis802_11AuthModeOWE,
	Ndis802_11AuthModeMax	/* Not a real mode, defined as upper bound */
} NDIS_802_11_AUTHENTICATION_MODE, *PNDIS_802_11_AUTHENTICATION_MODE;

typedef UCHAR NDIS_802_11_RATES[NDIS_802_11_LENGTH_RATES];	/* Set of 8 data rates */
typedef UCHAR NDIS_802_11_RATES_EX[NDIS_802_11_LENGTH_RATES_EX];	/* Set of 16 data rates */

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
	Ndis802_11Encryption3KeyAbsent,
	Ndis802_11TKIPAESMix,
	Ndis802_11Encryption4Enabled = Ndis802_11TKIPAESMix,	/* TKIP or AES mix */
	Ndis802_11Encryption4KeyAbsent,
	Ndis802_11GroupWEP40Enabled,
	Ndis802_11GroupWEP104Enabled,
#ifdef WAPI_SUPPORT
	Ndis802_11EncryptionSMS4Enabled,	/* WPI SMS4 support */
#endif /* WAPI_SUPPORT */
#ifdef CONFIG_OWE_SUPPORT
	Ndis802_11CCMP128Enable,
	Ndis802_11CCMP256Enable,
#endif
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

#ifdef DOT11_SAE_SUPPORT
#define INVALID_PMKID_IDX	-1
#endif

#if defined(APCLI_SAE_SUPPORT) || defined(APCLI_OWE_SUPPORT)
#define LEN_MAX_PMK 64
#endif

#if defined(CONFIG_STA_SUPPORT) || defined(WPA_SUPPLICANT_SUPPORT) || defined(APCLI_SUPPORT)\
	|| defined(APCLI_SAE_SUPPORT) || defined(APCLI_OWE_SUPPORT)
typedef struct _BSSID_INFO {
	NDIS_802_11_MAC_ADDRESS BSSID;
	NDIS_802_11_PMKID_VALUE PMKID;
#if defined(APCLI_SAE_SUPPORT) || defined(APCLI_OWE_SUPPORT)
	UCHAR PMK[LEN_MAX_PMK];
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
	NDIS_802_11_MAC_ADDRESS MAC;
	NDIS_802_11_PMKID_VALUE PMKID;
	UCHAR PMK[32];
	ULONG RefreshTime;
	BOOLEAN Valid;
} AP_BSSID_INFO, *PAP_BSSID_INFO;

#define MAX_PMKID_COUNT		8
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
#ifdef STA_FORCE_ROAM_SUPPORT

enum FROAM_COMMAND {
	OID_FROAM_CMD_FROAM_ENABLED = 0x1,
	OID_FROAM_CMD_GET_THRESHOLD = 0x2,
};

typedef struct GNU_PACKED _froam_command_hdr {
	UINT8 command_id;
	UINT8 command_len;
}froam_command_hdr,*pfroam_command_hdr;

typedef struct GNU_PACKED _threshold_info{
	UINT8 sta_ageout_time;
	UINT8 mntr_ageout_time;
	UINT8 mntr_min_pkt_count;
	UINT8 mntr_min_time;
	UINT8 mntr_avg_rssi_pkt_count;
	char sta_detect_rssi_threshold;
	UINT8 acl_ageout_time;
	UINT8 acl_hold_time;
}threshold_info,*pthreshold_info;

typedef struct GNU_PACKED _cmd_froam_en{
	struct _froam_command_hdr hdr;
	UINT8 froam_en;
	UINT8 channel;	
}cmd_froam_en,*pcmd_froam_en;

typedef struct GNU_PACKED _cmd_threshold{
	struct _froam_command_hdr hdr;
	struct _threshold_info info;
}cmd_threshold,*pcmd_threshold;

enum FROAM_EVENT {	
	FROAM_EVT_STA_RSSI_LOW 	 = 0x01,
	FROAM_EVT_STA_RSSI_GOOD	 = 0x02,
	FROAM_EVT_STA_DISCONNECT = 0x03,
	FROAM_EVT_CLEAR_MNTR_LIST = 0x04,
	FROAM_EVT_CLEAR_ACL_LIST = 0x05,
	FROAM_EVT_STA_AGEOUT_TIME = 0x06,
	FROAM_EVT_MNTR_AGEOUT_TIME = 0x07,
	FROAM_EVT_ACL_AGEOUT_TIME = 0x08,
	FROAM_EVT_STA_DETECT_RSSI_THRESHOLD = 0x09,
	FROAM_EVT_MNTR_MIN_PKT_COUNT = 0x0A, // all new below
	FROAM_EVT_MNTR_MIN_TIME = 0x0B,
	FROAM_EVT_STA_AVG_RSSI_PKT_COUNT = 0x0D,
	FROAM_EVT_ACL_HOLD_TIME = 0x0C,
	FROAM_EVT_FROAM_SUPP = 0x0E,
};

typedef struct GNU_PACKED froam_event_s {
	UINT8 event_id;
	UINT8 event_len;
	UINT8 event_body[0];
}froam_event_t, *p_froam_event_t;

typedef struct GNU_PACKED _froam_event_hdr {
	UINT8 event_id;
	UINT8 event_len;
}froam_event_hdr,*p_froam_event_hdr;

#define LOW_RSSI_STA_RENOTIFY_TIME	10

typedef struct GNU_PACKED _froam_event_sta_low_rssi {
	struct _froam_event_hdr hdr;
	UINT8 mac[MAC_ADDR_LEN];
	UINT8 channel;
}froam_event_sta_low_rssi, *pfroam_event_sta_low_rssi;

typedef struct GNU_PACKED _froam_event_sta_good_rssi {
	struct _froam_event_hdr hdr;
	UINT8 mac[MAC_ADDR_LEN];
}froam_event_sta_good_rssi, *pfroam_event_sta_good_rssi;

typedef struct GNU_PACKED _froam_event_sta_disconn {
	struct _froam_event_hdr hdr;
	UINT8 mac[MAC_ADDR_LEN];
}froam_event_sta_disconn, *pfroam_event_sta_disconn;

typedef struct GNU_PACKED _froam_event_param_cfg {
	struct _froam_event_hdr hdr;
	UINT8 value;
}froam_event_param_cfg, *pfroam_event_param_cfg;
#endif

#ifdef WH_EZ_SETUP

#ifdef EZ_REGROUP_SUPPORT
enum REGROUP_COMMAND {
	OID_REGROUP_CMD_CLI_INFO				= 0x01,
	OID_REGROUP_CMD_CONNECTED_PEER_LIST		= 0x02,
	//OID_REGROUP_CMD_EZ_CAP_INFO			= 0x03,
	OID_REGROUP_CMD_SCAN					= 0x04,
	OID_REGROUP_CMD_CAND_LIST				= 0x05,
	OID_REGROUP_CMD_CLEAR_CAND_LIST 		= 0x06,
	OID_REGROUP_CMD_TRIGGER_REGRP			= 0x07,
	OID_REGROUP_CMD_TERMINATE_REGRP 		= 0x08,
	OID_REGROUP_CMD_REGRP_SUPP				= 0x09,
	OID_REGROUP_CMD_EN_REGRP_MODE			= 0x0A, // rename to avoid confusion with regrp supp
	OID_REGROUP_QUERY_INTERFACE_DETAILS	      	= 0x0B,
	OID_REGROUP_QUERY_NODE_NUMBER_WT	      		= 0x0C
};

typedef struct GNU_PACKED regrp_command_s
{
	UINT8 command_id;
	UINT8 command_len;
	UINT8 command_body[0];
} regrp_command_t, *p_regrp_command_t;

typedef struct GNU_PACKED _regrp_cmd_hdr {
	UINT8 command_id;
	UINT8 command_len;
}regrp_cmd_hdr,*pregrp_cmd_hdr;

enum REGROUP_EVENT {
	REGROUP_EVT_USER_REGROUP_REQ 	= 0x02,
	REGROUP_EVT_SCAN_COMPLETE	= 0x03,
	REGROUP_EVT_INTF_REGRP_DONE = 0x04,
	REGROUP_EVT_INTF_CONNECTED = 0x5,
	REGROUP_EVT_INTF_DISCONNECTED = 0x6,
};

typedef struct GNU_PACKED regrp_event_s
{
	UINT8 event_id;
	UINT8 event_len;
	UINT8 event_body[0];
} regrp_event_t, *p_regrp_event_t;

typedef struct GNU_PACKED _cmd_regrp_supp{
	struct _regrp_cmd_hdr hdr;
	UINT8 regrp_supp;
	UINT8 ez_supp;
	UINT8 channel;
	UINT8 configured;
}cmd_regrp_supp,*pcmd_regrp_supp;

enum REGRP_MODE{
	NON_REGRP_MODE = 0x0,		// ensure to set this back
	REGRP_MODE_BLOCKED = 0x1,
	REGRP_MODE_UNBLOCKED = 0x2,
	// one mroe mode thought off ??
};

typedef struct GNU_PACKED _cmd_regrp_mode{
	struct _regrp_cmd_hdr hdr;
	UINT8 mode;
}cmd_regrp_mode,*pcmd_regrp_mode;

#define HAS_INTERNET_BIT		 1
#define HAS_DIRECT_INTERNET_BIT	 2

//#define NETWORK_WEIGHT_LEN  (MAC_ADDR_LEN + 1)

//typedef struct GNU_PACKED _ez_cap_info {
//	unsigned int capability;
//}ez_cap_info,*pez_cap_info;


#define SCAN_GENERAL	1
#define SCAN_CONNECTED_SSID	2
#define SCAN_SPECIFIED_SSID	3

typedef struct GNU_PACKED _regrp_cmd_scan {
	struct _regrp_cmd_hdr hdr;
	UINT8 scan_type;
	UINT8 ssid[32];
}regrp_cmd_scan,*pregrp_cmd_scan;

#define MAX_EZ_PEERS	6
#define MAX_AP_CANDIDATES	(MAX_EZ_PEERS + 2) // considering 2 Non MAN Ap but no restriction

typedef struct GNU_PACKED _regrp_cmd_cand_list{
	struct _regrp_cmd_hdr hdr;
	u8 cand_count;
	UINT8 list[0];	// one or more entries of type struct _ntw_info
}regrp_cmd_cand_list,*pregrp_cmd_cand_list;

typedef struct GNU_PACKED _regrp_cmd_cfg_cand_list{
	struct _regrp_cmd_hdr hdr;
	u8 cand_count;
	u8 bssid[MAX_AP_CANDIDATES][MAC_ADDR_LEN];
}regrp_cmd_cfg_cand_list,*pregrp_cmd_cfg_cand_list;


typedef struct GNU_PACKED _drvr_cand_list{
	UCHAR bssid[MAC_ADDR_LEN];
	UCHAR attempted;
}rdrvr_cand_list,*pdrvr_cand_list;

#endif

#endif





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

/* //dot11Phy(4) */

#endif /* SNMP_SUPPORT */

#if (defined(SNMP_SUPPORT) || defined(WH_EZ_SETUP))
/* //dot11Phy(4) */
#define OID_802_11_CURRENTCHANNEL				0x0712

#endif /* SNMP_SUPPORT */

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

#define	RT_OID_802_11R_SUPPORT					  	(OID_GET_SET_TOGGLE | OID_802_11R_SUPPORT)
#define RT_OID_802_11R_MDID							(OID_GET_SET_TOGGLE | OID_802_11R_MDID)
#define RT_OID_802_11R_R0KHID						(OID_GET_SET_TOGGLE | OID_802_11R_R0KHID)
#define	RT_OID_802_11R_RIC					  		(OID_GET_SET_TOGGLE | OID_802_11R_RIC)
#define RT_OID_802_11R_OTD							(OID_GET_SET_TOGGLE | OID_802_11R_OTD)
#define RT_OID_802_11R_INFO							(OID_GET_SET_TOGGLE | OID_802_11R_INFO)
#endif /* DOT11R_FT_SUPPORT */


#ifdef WSC_NFC_SUPPORT
#define RT_OID_NFC_STATUS							0x0930
#endif /* WSC_NFC_SUPPORT */

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
	UINT32 ConnectedTime;
	MACHTTRANSMIT_SETTING TxRate;
	UINT32 LastRxRate;
} RT_802_11_MAC_ENTRY, *PRT_802_11_MAC_ENTRY;

typedef struct _RT_802_11_MAC_TABLE {
	ULONG Num;
	RT_802_11_MAC_ENTRY Entry[MAX_NUMBER_OF_MAC];
} RT_802_11_MAC_TABLE, *PRT_802_11_MAC_TABLE;

#ifdef DOT11_N_SUPPORT
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

#ifdef MAC_REPEATER_SUPPORT
typedef struct _RT_802_11_REALMAC_ENTRY {
	UCHAR Addr[MAC_ADDR_LEN];
	UCHAR bSet;	
	UCHAR bUsed;
} RT_802_11_REALMAC_ENTRY, *PRT_802_11_REALMAC_ENTRY;

typedef struct GNU_PACKED _RT_802_11_MACLIST {
	ULONG Num;
	RT_802_11_REALMAC_ENTRY Entry[MAX_NUMBER_OF_MAC_LIST];
} RT_802_11_MACLIST, *PRT_802_11_MACLIST;
#endif /* MAC_REPEATER_SUPPORT */

typedef struct _RT_802_11_WDS {
	ULONG Num;
	NDIS_802_11_MAC_ADDRESS Entry[24 /*MAX_NUM_OF_WDS_LINK */ ];
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
	UCHAR rsv;
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
	UCHAR Rsvd[2];		/* Make alignment */
} WSC_CREDENTIAL, *PWSC_CREDENTIAL;

/* WSC configured profiles */
typedef struct _WSC_PROFILE {
	UINT ProfileCnt;
	UINT ApplyProfileIdx;	/* add by johnli, fix WPS test plan 5.1.1 */
	WSC_CREDENTIAL Profile[8];	/* Support up to 8 profiles */
} WSC_PROFILE, *PWSC_PROFILE;

#ifdef WAPI_SUPPORT
typedef enum _WAPI_PORT_SECURE_STATE {
	WAPI_PORT_NOT_SECURED,
	WAPI_PORT_SECURED,
} WAPI_PORT_SECURE_STATE, *PWAPI_PORT_SECURE_STATE;

typedef struct _WAPI_PORT_SECURE_STRUCT {
	UCHAR Addr[MAC_ADDR_LEN];
	USHORT state;
} WAPI_PORT_SECURE_STRUCT, *PWAPI_PORT_SECURE_STRUCT;

typedef struct _WAPI_UCAST_KEY_STRUCT {
	UCHAR Addr[MAC_ADDR_LEN];
	USHORT key_id;
	UCHAR PTK[64];		/* unicast and additional key */
} WAPI_UCAST_KEY_STRUCT, *PWAPI_UCAST_KEY_STRUCT;

typedef struct _WAPI_MCAST_KEY_STRUCT {
	UINT32 key_id;
	UCHAR m_tx_iv[16];
	UCHAR key_announce[16];
	UCHAR NMK[16];		/* notify master key */
} WAPI_MCAST_KEY_STRUCT, *PWAPI_MCAST_KEY_STRUCT;

typedef struct _WAPI_WIE_STRUCT {
	UCHAR addr[6];
	UINT32 wie_len;
	UCHAR wie[90];		/* wapi information element */
} WAPI_WIE_STRUCT, *PWAPI_WIE_STRUCT;

#endif /* WAPI_SUPPORT */

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


#ifdef APCLI_SUPPORT
#endif /* APCLI_SUPPORT */






#ifdef AIRPLAY_SUPPORT
#define OID_AIRPLAY_IE_INSERT                   (0x0872)
#define OID_AIRPLAY_ENABLE                      (0x0873)
#endif/* AIRPLAY_SUPPORT*/

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

struct hs_param_setting {
	UINT32 param;
	UINT32 value;
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

#ifdef WH_EZ_SETUP
typedef struct GNU_PACKED internet_command_s
{
	BOOLEAN Net_status;
	
} internet_command_t, *p_internet_command_t;

#endif

#define OID_802_11_WIFI_VER                     0x0920
/*#define OID_802_11_HS_TEST                      0x0921*/
#define OID_802_11_WAPP_SUPPORT_VER             0x0921
/*#define OID_802_11_HS_IE                        0x0922*/
#define OID_802_11_WAPP_IE                      0x0922
#define OID_802_11_HS_ANQP_REQ                  0x0923
#define OID_802_11_HS_ANQP_RSP                  0x0924
#define OID_802_11_HS_ONOFF                     0x0925
/*#define OID_802_11_HS_PARAM_SETTING             0x0927*/
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
//#define OID_802_11_HS_OSU_NONTX               0x0944
#define OID_802_11_HS_SASN_ENABLE               0x0943
#define OID_802_11_WNM_NOTIFY_REQ               0x0944
#define OID_802_11_QOSMAP_CONFIGURE             0x0945
#define OID_802_11_GET_STA_HSINFO             	0x0946
#define OID_802_11_BSS_LOAD			           	0x0947
#define OID_802_11_INTERWORKING_ENABLE			0x0949

#define OID_WSC_UUID							0x0990
#define OID_SET_SSID							0x0992
#define OID_SET_PSK								0x0993

#define OID_GET_CPU_TEMPERATURE 0x09A1

#define OID_GET_WSC_PROFILES 0x0994
#define OID_GET_MISC_CAP								0x0995
#define OID_GET_HT_CAP									0x0996
#define OID_GET_VHT_CAP									0x0997
#define OID_GET_CHAN_LIST								0x0998
#define OID_GET_OP_CLASS								0x0999
#define OID_GET_BSS_INFO								0x099A
#define OID_GET_AP_METRICS								0x099B

#ifdef STA_FORCE_ROAM_SUPPORT
#define OID_FROAM_COMMAND   					0x0983
#define OID_FROAM_EVENT   						0x0984
#endif

#ifdef AIR_MONITOR
typedef struct GNU_PACKED _mntr_entry_info {
	u8 mac[MAC_ADDR_LEN];
	u8 channel;
	u8 index;
}mntr_entry_info, *pmntr_entry_info;

#define OID_AIR_MNTR_ADD_ENTRY   				0x0980
#define OID_AIR_MNTR_DEL_ENTRY					0x0981
#define OID_AIR_MNTR_SET_RULE					0x0982
#endif

#ifdef MBO_SUPPORT
#define OID_802_11_MBO_MSG						0x0953
#define OID_NEIGHBOR_REPORT						0x0954
#endif

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
struct wapp_param_setting {
	UINT32 param;
	UINT32 value;
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
*	@num_rpt: optional; number of repetitions;
*	@regclass: only mandatory when channel is set to 0; operating class;
*	@channum: mandatory; channel number;
*	@random_ivl: optional; randomization interval; unit ms;
*	the upper bound of the random delay to be used prior to make measurement;
*	@duration: optional; measurement duration; unit ms;
*	@bssid: optional;
*	@mode: optional; measurement mode;
*	As default value 0 is a valid value in spec, so here need remap the value and the meaning;
*	1 for passive mode; 2 for active mode; 3 for beacon table;
*	@req_ssid: optional; subelement SSID;
*	@timeout: optional; unit s;
*	@rep_conditon: optional; subelement Beacon Reporting Information;
*	@ref_value: optional; subelement Beacon Reporting Information;
*	condition for report to be issued;
*	driver will send timeout event after timeout value if no beacon report received;
*	@detail: optional; subelement Reporting Detail;
*	As default value 0 is a valid value in spec, so here need remap the value and the meaning;
*	1 for no fixed length fields or elements;
*	2 for all fixed length fields and any requested elements in the request IE;
*	3 for all fixed length fields and elements
*	@op_class_len:  mandatory only when channel is set to 255;
*	@op_class_list: subelement Ap Channel Report;
*	@ch_list_len: mandatory only when channel is set to 255;
*	@ch_list: subelement Ap Channel Report;
*	if you want use all the channels in operating classes then use default value
*	otherwise specify all channels you want sta to do measurement
*	@request_len: optional;
*	@request: subelement Request; only valid when you specify request IDs
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
*	indicates the network preference for BSS transition to the BSS listed in this
*	BSS Transition Candidate List Entries; 0 is a valid value in spec, but here
*	need remap its meaning to not include preference IE in neighbor report
*	response frame;
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
	UINT8 event_len;
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
	UINT32 btm_rsp_len;
	UCHAR btm_rsp[0];
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


#ifdef WH_EZ_SETUP
enum {
	OID_WH_EZ_ENABLE = 0x2000,
	OID_WH_EZ_CONF_STATUS = 0x2001,
	OID_WH_EZ_GROUP_ID = 0x2002,
	OID_WH_EZ_GEN_GROUP_ID = 0x2003,
	OID_WH_EZ_RSSI_THRESHOLD = 0x2004,
	OID_WH_EZ_INTERNET_COMMAND = 0x2005,
	OID_WH_EZ_GET_GUI_INFO = 0x2006,
#ifdef EZ_PUSH_BW_SUPPORT
	OID_WH_EZ_PUSH_BW = 0x2007,
#endif
	OID_WH_EZ_CUSTOM_DATA_CMD = 0x2008,
	OID_WH_EZ_CUSTOM_DATA_EVENT = 0x2009
};
#define OID_WH_EZ_UPDATE_STA_INFO				  0x2010
#define OID_WH_EZ_MAN_DEAMON_EVENT	 					 0x200A
#define OID_WH_EZ_MAN_TRIBAND_EZ_DEVINFO_EVENT	 			 0x200B
#define OID_WH_EZ_MAN_TRIBAND_NONEZ_DEVINFO_EVENT	 			 0x200c
#define OID_WH_EZ_MAN_TRIBAND_SCAN_COMPLETE_EVENT	 			 0x200d
#define OID_WH_EZ_MAN_PLUS_NONMAN_EZ_DEVINFO_EVENT	 			 0x200E
#define OID_WH_EZ_MAN_PLUS_NONMAN_NONEZ_DEVINFO_EVENT	 		 0x200F
#define OID_WH_EZ_MAN_CONF_EVENT	 					 0x2011
#define OID_WH_EZ_REGROUP_COMMAND                                0x2012
#define OID_WH_EZ_REGROUP_EVENT                                      0x2013


#endif /* WH_EZ_SETUP */

#ifdef ACS_CTCC_SUPPORT
#define OID_802_11_GET_ACS_CHANNEL_SCORE                0x2014
typedef struct _AUTO_CH_SEL_SCORE{
	UINT32 Score;
	UINT32 Channel;
} AUTO_CH_SEL_SCORE, *PAUTO_CH_SEL_SCORE;;
typedef struct _ACS_CHANNEL_SCORE{
 AUTO_CH_SEL_SCORE AcsChannelScore[MAX_NUM_OF_CHANNELS+1];
 UINT32 AcsAlg;
} ACS_CHANNEL_SCORE, *PACS_CHANNEL_SCORE;
#endif

#ifdef MIXMODE_SUPPORT
#define OID_MIX_MODE_BASE	0x1280
enum mix_mode_subcmd_oid {
	OID_SUBCMD_SET_MIXMODE,
	OID_SUBCMD_GET_RSSI,
	OID_SUBCMD_CANCEL_MIXMODE,
	NUM_OID_SUBCMD_MIXMODE,
	OID_SUBCMD_MIXMODE_MAX = NUM_OID_SUBCMD_MIXMODE - 1
};
#define OID_SET_MIXMODE		(OID_MIX_MODE_BASE | OID_SUBCMD_SET_MIXMODE)
#define OID_MIXMODE_GET_RSSI	(OID_MIX_MODE_BASE | OID_SUBCMD_GET_RSSI)
#define OID_CANCEL_MIXMODE	(OID_MIX_MODE_BASE | OID_SUBCMD_CANCEL_MIXMODE)
#define RT_OID_SET_MIXMODE	(OID_GET_SET_TOGGLE | OID_SET_MIXMODE)
#define RT_OID_CANCEL_MIXMODE	(OID_GET_SET_TOGGLE | OID_CANCEL_MIXMODE)
#define RT_MIX_MODE_EVENT_FLAG	0x1810
#endif /* MIXMODE_SUPPORT */

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


#endif /* _OID_H_ */


/*
 ***************************************************************************
 * Mediatek Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2011, Mediatek Technology, Inc.
 *
 * All rights reserved. Mediatek's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Mediatek Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Mediatek Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	wapp_cmm_type.h

	Abstract:

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/
/* This file is used by wifi driver and wapp.
   Keep data structure sync */

#ifndef __WAPP_TYPES_H__
#define __WAPP_TYPES_H__

#include "rt_config.h"
#include "mlme.h"
#include "rtmp_type.h"
#include "map.h"
#ifdef WAPP_SUPPORT
#define MAX_BSSLOAD_THRD			100
#endif /* WAPP_SUPPORT */

#ifndef GNU_PACKED
#define GNU_PACKED  (__attribute__ ((packed)))
#endif /* GNU_PACKED */

#ifndef MAC_ADDR_LEN
#define MAC_ADDR_LEN				6
#endif

#ifndef AC_NUM
#define AC_NUM						4
#endif

#define MAX_OP_CLASS 16
#define MAX_LEN_OF_SSID 32

typedef enum {
	WAPP_STA_INVALID,
	WAPP_STA_DISCONNECTED,
	WAPP_STA_CONNECTED,
} WAPP_STA_STATE;

typedef enum {
	WAPP_BSS_STOP = 0,
	WAPP_BSS_START,
} WAPP_BSS_STATE;

typedef enum {
	WAPP_AUTH = 0,
	WAPP_ASSOC,
} WAPP_CNNCT_STAGE;

typedef enum {
	WAPP_BSSLOAD_NORMAL = 0,
	WAPP_BSSLOAD_HIGH,
	WAPP_BSSLOAD_LOW,
} WAPP_BSSLOAD_STATE;

typedef enum {
	NOT_FAILURE = 0,
	AP_NOT_READY,
	ACL_CHECK_FAIL,
	BSSID_NOT_FOUND,
	BSSID_MISMATCH,
	BSSID_IF_NOT_READY,
	BND_STRG_CONNECT_CHECK_FAIL,
	DISALLOW_NEW_ASSOCI,
	EZ_CONNECT_DISALLOW,
	EZ_SETUP_FUNC_DISABLED,
	FT_ERROR,
	GO_UPDATE_NOT_COMPLETE,
	MLME_NO_RESOURCE,
	MLME_ASSOC_REJ_TEMP,
	MLME_UNABLE_HANDLE_STA,
	MLME_EZ_CNNCT_LOOP,
	MLME_REQ_WITH_INVALID_PARAM,
	MLME_REJECT_TIMEOUT,
	MLME_UNSPECIFY_FAILURE,
	NOT_FOUND_IN_RADIUS_ACL,
	PEER_REQ_SANITY_FAIL,
} WAPP_CNNCT_FAIL_REASON_LIST;

typedef enum {
	WAPP_APCLI_DISASSOCIATED = 0,
	WAPP_APCLI_ASSOCIATED,
} WAPP_APCLI_ASSOC_STATE;

typedef enum {
	WAPP_DEV_QUERY_RSP = 1,
	WAPP_HT_CAP_QUERY_RSP,
	WAPP_VHT_CAP_QUERY_RSP,
	WAPP_HE_CAP_QUERY_RSP,
	WAPP_MISC_CAP_QUERY_RSP,
	WAPP_CLI_QUERY_RSP,
	WAPP_CLI_LIST_QUERY_RSP,
	WAPP_CLI_JOIN_EVENT,
	WAPP_CLI_LEAVE_EVENT,
	WAPP_CLI_PROBE_EVENT,
	WAPP_CHN_LIST_RSP,
	WAPP_OP_CLASS_RSP,
	WAPP_BSS_INFO_RSP,
	WAPP_AP_METRIC_RSP,
	WAPP_CH_UTIL_QUERY_RSP,
	WAPP_AP_CONFIG_RSP,
	WAPP_APCLI_QUERY_RSP,
	MAP_BH_STA_WPS_DONE,
	MAP_TRIGGER_RSSI_STEER,
	WAPP_RCEV_BCN_REPORT,
	WAPP_RCEV_BCN_REPORT_COMPLETE,
	WAPP_RCEV_MONITOR_INFO,
	WAPP_BSSLOAD_RSP,
	WAPP_BSSLOAD_CROSSING,
	WAPP_BSS_STATE_CHANGE,
	WAPP_CH_CHANGE,
	WAPP_TX_POWER_CHANGE,
	WAPP_APCLI_ASSOC_STATE_CHANGE,
	WAPP_STA_RSSI_RSP,
	WAPP_CLI_ACTIVE_CHANGE,
	WAPP_CSA_EVENT,
	WAPP_STA_CNNCT_REJ,
	WAPP_APCLI_RSSI_RSP,
	WAPP_SCAN_RESULT_RSP,
	WAPP_MAP_VENDOR_IE,
	WAPP_WSC_SCAN_COMP_NOTIF,
	WAPP_MAP_WSC_CONFIG,
	WAPP_WSC_EAPOL_START_NOTIF,
	WAPP_WSC_EAPOL_COMPLETE_NOTIF,
	WAPP_SCAN_COMPLETE_NOTIF,
	WAPP_A4_ENTRY_MISSING_NOTIF,
	WAPP_RADAR_DETECT_NOTIF,
	WAPP_APCLI_ASSOC_STATE_CHANGE_VENDOR10,
} WAPP_EVENT_ID;

typedef enum {
	WAPP_DEV_QUERY_REQ = 1,
	WAPP_HT_CAP_QUERY_REQ,
	WAPP_VHT_CAP_QUERY_REQ,
	WAPP_HE_CAP_QUERY_REQ,
	WAPP_MISC_CAP_QUERY_REQ,
	WAPP_CLI_QUERY_REQ,
	WAPP_CLI_LIST_QUERY_REQ,
	WAPP_CHN_LIST_QUERY_REQ,
	WAPP_OP_CLASS_QUERY_REQ,
	WAPP_BSS_INFO_QUERY_REQ,
	WAPP_AP_METRIC_QUERY_REQ,
	WAPP_CH_UTIL_QUERY_REQ,
	WAPP_APCLI_QUERY_REQ,
	WAPP_BSS_START_REQ,
	WAPP_BSS_STOP_REQ,
	WAPP_TXPWR_PRCTG_REQ,
	WAPP_STEERING_POLICY_SET_REQ,
	WAPP_BSS_LOAD_THRD_SET_REQ,
	WAPP_AP_CONFIG_SET_REQ,
	WAPP_BSSLOAD_QUERY_REQ,
	WAPP_HECAP_QUERY_REQ,
	WAPP_STA_RSSI_QUERY_REQ,
	WAPP_APCLI_RSSI_QUERY_REQ,
	WAPP_GET_SCAN_RESULTS,
	WAPP_SEND_NULL_FRAMES,
	WAPP_WSC_PBC_EXEC,
	WAPP_WSC_SET_BH_PROFILE,
	WAPP_SET_SCAN_BH_SSIDS,
} WAPP_REQ_ID;

typedef enum {
	PARAM_DGAF_DISABLED,
	PARAM_PROXY_ARP,
	PARAM_L2_FILTER,
	PARAM_ICMPV4_DENY,
	PARAM_MMPDU_SIZE,
	PARAM_EXTERNAL_ANQP_SERVER_TEST,
	PARAM_GAS_COME_BACK_DELAY,
	PARAM_WNM_NOTIFICATION,
	PARAM_QOSMAP,
	PARAM_WNM_BSS_TRANSITION_MANAGEMENT,
} WAPP_PARAM;

typedef struct GNU_PACKED _WAPP_CONNECT_FAILURE_REASON {
	u8	connect_stage;
	u16	reason;
} WAPP_CONNECT_FAILURE_REASON;

typedef struct GNU_PACKED _wapp_dev_info {
	u32	ifindex;
	u8	ifname[IFNAMSIZ];
	u8	mac_addr[MAC_ADDR_LEN];
	u8	dev_type;
	u8	radio_id;
	u8	wireless_mode;
	uintptr_t	adpt_id;
	u8 dev_active;
} wapp_dev_info;

typedef struct GNU_PACKED _wdev_ht_cap {
	u8	tx_stream;
	u8	rx_stream;
	u8	sgi_20;
	u8	sgi_40;
	u8	ht_40;
} wdev_ht_cap;

typedef struct GNU_PACKED _wdev_vht_cap {
	u8	sup_tx_mcs[2];
	u8	sup_rx_mcs[2];
	u8	tx_stream;
	u8	rx_stream;
	u8	sgi_80;
	u8	sgi_160;
	u8	vht_160;
	u8	vht_8080;
	u8	su_bf;
	u8	mu_bf;
} wdev_vht_cap;

typedef struct GNU_PACKED _wdev_misc_cap {
	u8	max_num_of_cli;
	u8	max_num_of_bss;
	u8	num_of_bss;
	u8	max_num_of_block_cli;
} wdev_misc_cap;

struct GNU_PACKED map_cli_cap {
	u16 bw:2;
	u16 phy_mode:3;
	u16 nss:2;
	u16 btm_capable:1;
	u16 rrm_capable:1;
	u16 mbo_capable:1;
};

#define ASSOC_REQ_LEN 154
typedef struct GNU_PACKED _wapp_client_info {
	u8 mac_addr[MAC_ADDR_LEN];
	u8 bssid[MAC_ADDR_LEN];
	u8 sta_status; /* WAPP_STA_STATE */
	u16 assoc_time;
	u8 assoc_req[ASSOC_REQ_LEN];
	u16 downlink;
	u16 uplink;
	char uplink_rssi;
	/*traffic stats*/
	u32 bytes_sent;
	u32 bytes_received;
	u32 packets_sent;
	u32 packets_received;
	u32 tx_packets_errors;
	u32 rx_packets_errors;
	u32 retransmission_count;
	u16 link_availability;
	u8 assoc_req_len;
	u8 bLocalSteerDisallow;
	u8 bBTMSteerDisallow;
	u8 status;
	/* ht_cap */
	/* vht_cap */

	/* Throughput for Tx/Rx */
	u32 tx_tp;
	u32 rx_tp;
	struct map_cli_cap cli_caps;
} wapp_client_info;

struct GNU_PACKED chnList {
	u8 channel;
	u8 pref;
};

typedef struct GNU_PACKED _wdev_chn_info {
	u8		op_ch;
	u8		op_class;
	u8		band; /* 24g; 5g1; 5g2 */
	u8		ch_list_num;
	u8		non_op_chn_num;
	u16		dl_mcs;
	struct chnList ch_list[16];
	u8		non_op_ch_list[16];
} wdev_chn_info;

struct GNU_PACKED opClassInfo {
	u8	op_class;
	u8	num_of_ch;
	u8	ch_list[12];
};

typedef struct GNU_PACKED _wdev_op_class_info {
	u8		num_of_op_class;
	struct opClassInfo opClassInfo[MAX_OP_CLASS];
} wdev_op_class_info;

typedef struct GNU_PACKED _wdev_bss_info {
	u8 if_addr[MAC_ADDR_LEN];
	u8 bssid[MAC_ADDR_LEN];
	char ssid[MAX_LEN_OF_SSID + 1];
	u8 SsidLen;
	u8 map_role;
	UINT32 auth_mode;
	UINT32 enc_type;
	u8 key_len;
	u8 key[64 + 1];
	u8 hidden_ssid;
} wdev_bss_info;

typedef struct GNU_PACKED _wsc_apcli_config {
	char ssid[MAX_LEN_OF_SSID + 1];
	unsigned char SsidLen;
	unsigned short AuthType;
	unsigned short EncrType;
	unsigned char Key[64];
	unsigned short KeyLength;
	unsigned char KeyIndex;
	unsigned char bssid[MAC_ADDR_LEN];
	unsigned char peer_map_role;
	unsigned char own_map_role;
} wsc_apcli_config;

typedef struct GNU_PACKED _wsc_apcli_config_msg {
	unsigned int profile_count;
	wsc_apcli_config apcli_config[0];
} wsc_apcli_config_msg, *p_wsc_apcli_config_msg;

typedef struct GNU_PACKED _wdev_ap_metric {
	u8		bssid[MAC_ADDR_LEN];
	u8		cu;
	u8		ESPI_AC_BE[3];
	u8		ESPI_AC_BK[3];
	u8		ESPI_AC_VO[3];
	u8		ESPI_AC_VI[3];
} wdev_ap_metric;

typedef struct GNU_PACKED _wdev_ap_config {
	u8 sta_report_on_cop;
	u8 sta_report_not_cop;
	u8 rssi_steer;
} wdev_ap_config;

typedef struct GNU_PACKED _wdev_tx_power {
	char pwr_limit;
	u16 tx_pwr;
} wdev_tx_power;

/*Driver detects sta needed to steer*/
typedef struct GNU_PACKED _wdev_steer_sta {
	u8 mac_addr[MAC_ADDR_LEN];
} wdev_steer_sta;

#define PREQ_IE_LEN 128
typedef struct GNU_PACKED _wapp_probe_info {
	u8 mac_addr[MAC_ADDR_LEN];
	u8 channel;
	char rssi;
	u8 preq_len;
	u8 preq[PREQ_IE_LEN];
} wapp_probe_info;

#define BCN_RPT_LEN 200
typedef struct GNU_PACKED _wapp_bcn_rpt_info {
	u8 sta_addr[MAC_ADDR_LEN];
	u8 last_fragment;
	u16 bcn_rpt_len;
	u8 bcn_rpt[BCN_RPT_LEN];
} wapp_bcn_rpt_info;

typedef struct GNU_PACKED wapp_bhsta_info {
	u8 mac_addr[MAC_ADDR_LEN];
	u8 connected_bssid[MAC_ADDR_LEN];
} wapp_bhsta_info;

typedef struct GNU_PACKED _wdev_steer_policy {
	unsigned char steer_policy;
	unsigned char cu_thr;
	unsigned char rcpi_thr;
} wdev_steer_policy;

typedef struct GNU_PACKED _bssload_threshold {
	u8 high_bssload_thrd;
	u8 low_bssload_thrd;
} bssload_threshold;

typedef struct GNU_PACKED _wapp_bssload_info {
	u16 sta_cnt;
	u8 ch_util;
	u16 AvalAdmCap;
} wapp_bssload_info;

/* By air monitor*/
typedef struct GNU_PACKED _wapp_mnt_info {
	u8 sta_addr[MAC_ADDR_LEN];
	char rssi;
} wapp_mnt_info;

typedef struct GNU_PACKED _wapp_csa_info {
	u8 new_channel;
} wapp_csa_info;

typedef struct GNU_PACKED _wapp_bss_state_info {
	UINT32 interface_index;
	WAPP_BSS_STATE bss_state;
} wapp_bss_state_info;

typedef struct GNU_PACKED _wapp_ch_change_info {
	u_int32_t interface_index;
	u_int8_t new_ch;/*New channel IEEE number*/
} wapp_ch_change_info;

typedef struct GNU_PACKED _wapp_txpower_change_info {
	u_int32_t interface_index;
	u_int16_t new_tx_pwr;/*New TX power*/
} wapp_txpower_change_info;

typedef struct GNU_PACKED _wapp_apcli_association_info {
	UINT32 interface_index;
	WAPP_APCLI_ASSOC_STATE apcli_assoc_state;
	char rssi;
} wapp_apcli_association_info;

typedef struct GNU_PACKED _wapp_bssload_crossing_info {
	UINT32 interface_index;
	u8 bssload_high_thrd;
	u8 bssload_low_thrd;
	u8 bssload;
} wapp_bssload_crossing_info;

typedef struct GNU_PACKED _wapp_sta_cnnct_rejected_info {
	UINT32 interface_index;
	UCHAR sta_mac[MAC_ADDR_LEN];
	UCHAR bssid[MAC_ADDR_LEN];
	WAPP_CONNECT_FAILURE_REASON cnnct_fail;
} wapp_sta_cnnct_rej_info;

struct GNU_PACKED scan_bss_info {
	unsigned char Bssid[MAC_ADDR_LEN];
	unsigned char Channel;
	unsigned char CentralChannel;
	signed char Rssi;
	signed char MinSNR;
	unsigned char Privacy;

	unsigned char SsidLen;
	unsigned char Ssid[MAX_LEN_OF_SSID];

	unsigned char AuthMode;
	wdev_ht_cap ht_cap;
	wdev_vht_cap vht_cap;
	unsigned char map_vendor_ie_found;
	struct map_vendor_ie map_info;
};

struct GNU_PACKED wapp_scan_info {
	unsigned int interface_index;
	u8 more_bss;
	u8 bss_count;
	struct scan_bss_info bss[0];
};

struct GNU_PACKED wapp_wsc_scan_info {
	u8 bss_count;
	UCHAR	Uuid[16];
};

struct GNU_PACKED radar_notif_s
{
	unsigned int channel;
	unsigned int status;
};

typedef union GNU_PACKED _wapp_event_data {
	wapp_dev_info dev_info;
	wdev_ht_cap ht_cap;
	wdev_vht_cap vht_cap;
	wdev_misc_cap misc_cap;
	wapp_client_info cli_info;
	wdev_chn_info chn_list;
	wdev_op_class_info op_class;
	wdev_bss_info bss_info;
	wdev_ap_metric ap_metrics;
	wdev_ap_config ap_conf;
	wdev_tx_power tx_pwr;
	wdev_steer_sta str_sta;
	wapp_probe_info probe_info;
	wapp_bcn_rpt_info bcn_rpt_info;
	wapp_bssload_info bssload_info;
	wapp_bssload_crossing_info bssload_crossing_info;
	wapp_mnt_info mnt_info;
	wapp_bss_state_info bss_state_info;
	wapp_ch_change_info ch_change_info;
	wapp_txpower_change_info txpwr_change_info;
	wapp_apcli_association_info apcli_association_info;
	wapp_bhsta_info bhsta_info;
	wapp_csa_info csa_info;
	wapp_sta_cnnct_rej_info sta_cnnct_rej_info;
	u8 ch_util;
	struct wapp_scan_info scan_info;
	struct wapp_wsc_scan_info wsc_scan_info;
	UINT32 a4_missing_entry_ip;
	struct radar_notif_s radar_notif;
} wapp_event_data;

typedef struct GNU_PACKED _wapp_req_data {
	u32	ifindex;
	u8 mac_addr[MAC_ADDR_LEN];
	u32 value;
	bssload_threshold bssload_thrd;
	wdev_steer_policy str_policy;
	wdev_ap_config ap_conf;
	WSC_CREDENTIAL bh_wsc_profile;
#ifdef CONFIG_MAP_SUPPORT
	struct scan_BH_ssids scan_bh_ssids;
#endif
} wapp_req_data;

struct wapp_req {
	u8 req_id;
	u8 data_len;
	wapp_req_data data;
};

struct wapp_event {
	u8 len;
	u8 event_id;
	u32 ifindex;
	wapp_event_data data;
};
#endif /* __WAPP_TYPES_H__ */

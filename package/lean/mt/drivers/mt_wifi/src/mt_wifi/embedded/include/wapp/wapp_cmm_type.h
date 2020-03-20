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

#ifdef WAPP_SUPPORT
/* temp way for type def */
typedef UINT32 u32;
typedef UINT16 u16;
typedef UINT8 u8;
#endif /* WAPP_SUPPORT */

#ifndef GNU_PACKED
#define GNU_PACKED  (__attribute__ ((packed)))
#endif /* GNU_PACKED */

#ifndef MAC_ADDR_LEN
#define MAC_ADDR_LEN				6
#endif

#define MAX_OP_CLASS 16

typedef enum {
	WAPP_STA_INVALID,
	WAPP_STA_DISCONNECTED,
	WAPP_STA_CONNECTED,
} WAPP_STA_STATE;

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
	WAPP_TX_POWER_RSP,
	WAPP_APCLI_QUERY_RSP,
	MAP_BH_STA_WPS_DONE,
	MAP_TRIGGER_RSSI_STEER,
	WAPP_RCEV_BCN_REPORT,
	WAPP_RCEV_BCN_REPORT_COMPLETE,
	WAPP_RCEV_MONITOR_INFO,
	WAPP_BSSLOAD_RSP,
	WAPP_BSS_STOP_RSP,
	WAPP_CH_CHANGE,
	WAPP_TX_POWER_CHANGE,
	WAPP_APCLI_ASSOCIATED,
	WAPP_APCLI_DISASSOCIATED,
	WAPP_STA_RSSI_RSP,
	WAPP_CLI_ACTIVE_CHANGE,
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
	WAPP_TX_POWER_QUERT_REQ,
	WAPP_APCLI_QUERY_REQ,
	WAPP_BSS_START_REQ,
	WAPP_BSS_STOP_REQ,
	WAPP_TXPWR_PRCTG_REQ,
	WAPP_STEERING_POLICY_SET_REQ,
	WAPP_AP_CONFIG_SET_REQ,
	WAPP_BSSLOAD_QUERY_REQ,
	WAPP_HECAP_QUERY_REQ,
	WAPP_STA_RSSI_QUERY_REQ,
} WAPP_REQ_ID;

typedef struct GNU_PACKED _wapp_dev_info {
	u32	ifindex;
	u8	ifname[IFNAMSIZ];
	u8	mac_addr[MAC_ADDR_LEN];
	u8	dev_type;
	u8	radio_id;
	uintptr_t	adpt_id;
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

#define ASSOC_REQ_LEN 154
typedef struct GNU_PACKED _wapp_client_info {
	u8 mac_addr[MAC_ADDR_LEN];
	u8 bssid[MAC_ADDR_LEN];
	u8 sta_status; /* WAPP_STA_STATE */
	u16 assoc_time;
	u8 assoc_req[ASSOC_REQ_LEN];
	u8 downlink;
	u8 uplink;
	u8 uplink_rssi;
	/*traffic stats*/
	u32 bytes_sent;
	u32 bytes_received;
	u32 packets_sent;
	u32 packets_received;
	u32 tx_packets_errors;
	u32 rx_packets_errors;
	u32 retransmission_count;
	u16 link_availability;

	u8 bBSSMantSupport;
	u8 bLocalSteerDisallow;
	u8 bBTMSteerDisallow;
	u8 bIsActiveChange;
	/* ht_cap */
	/* vht_cap */
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
	u8		if_addr[MAC_ADDR_LEN];
	u8		bssid[MAC_ADDR_LEN];
	char	ssid[MAX_LEN_OF_SSID + 1];
	u8		SsidLen;
} wdev_bss_info;

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
	char tx_pwr;
} wdev_tx_power;

/*Driver detects sta needed to steer*/
typedef struct GNU_PACKED _wdev_steer_sta {
	u8 mac_addr[MAC_ADDR_LEN];
} wdev_steer_sta;

typedef struct GNU_PACKED _wapp_probe_info {
	u8 mac_addr[MAC_ADDR_LEN];
	u8 channel;
	u8 rssi;
} wapp_probe_info;

#define BCN_RPT_LEN 200
typedef struct GNU_PACKED _wapp_bcn_rpt_info {
	u8 sta_addr[MAC_ADDR_LEN];
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

typedef struct GNU_PACKED _wapp_bssload_info {
	u16 sta_cnt;
	u8 ch_util;
	u16 AvalAdmCap;
} wapp_bssload_info;

/* By air monitor*/
typedef struct GNU_PACKED _wapp_mnt_info {
	u8 rssi;
} wapp_mnt_info;

typedef struct GNU_PACKED _wapp_bss_stop_info {
	u_int32_t interface_index;
	/* u_int32_t  Reason; */
	/* u_int32_t Termination_duration; */
} wapp_bss_stop_info;

typedef struct GNU_PACKED _wapp_ch_change_info {
	u_int32_t interface_index;
	u_int8_t new_ch;/*New channel IEEE number*/
} wapp_ch_change_info;

typedef struct GNU_PACKED _wapp_txpower_change_info {
	u_int32_t interface_index;
	u_int16_t new_tx_pwr;/*New TX power*/
} wapp_txpower_change_info;

typedef struct GNU_PACKED _wapp_apcli_association_info {
	u_int32_t interface_index;
} wapp_apcli_association_info;

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
	wapp_mnt_info mnt_info;
	wapp_bss_stop_info bss_stop_info;
	wapp_ch_change_info ch_change_info;
	wapp_txpower_change_info txpwr_change_info;
	wapp_apcli_association_info apcli_association_info;
	wapp_bhsta_info bhsta_info;
	u8 ch_util;
} wapp_event_data;

typedef struct GNU_PACKED _wapp_req_data {
	u32	ifindex;
	u8 mac_addr[MAC_ADDR_LEN];
	u32 value;
	wdev_steer_policy str_policy;
	wdev_ap_config ap_conf;
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

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
	ap_vow.h
*/

#ifndef __AP_VOW_H_
#define __AP_VOW_H_

#define VOW_GEN_1	0
#define VOW_GEN_2	1

#define VOW_MAX_GROUP_NUM   16
#define VOW_MAX_STA_DWRR_NUM    8
#define VOW_MAX_WMM_SET_NUM 4

#define VOW_DEF_REFILL_PERIOD  3    /* 8us */
#define VOW_DEF_MIN_RATE_BUCKET_SIZE   1000/* x*1024B */
#define VOW_DEF_MAX_RATE_BUCKET_SIZE   1000/* x*1024B */
#define VOW_DEF_MIN_AIRTIME_BUCKET_SIZE 16 /* x*1.024ms */
#define VOW_DEF_MAX_AIRTIME_BUCKET_SIZE 16 /* x*1.024ms */
#define VOW_DEF_BACKLOG_SIZE    1000/* x*1024B */
#define VOW_DEF_MAX_WAIT_TIME   16 /* x*1.024ms for 11b */
#define VOW_DEF_BSS_DWRR_QUANTUM   16 /* x*256us */
#define VOW_DEF_STA_MAX_WAIT_TIME    64/* x*256us for 11b */
#define VOW_DEF_BSS_MAX_WAIT_TIME    64 /* x*256us for 11b */

/* G band */
#define VOW_DEF_GBAND_BK_BACKOFF    352 /* SIFS + 7 slot time + CW min(31) = 10 + 7*9 + 31*9 = 352us */
#define VOW_DEF_GBAND_BE_BACKOFF    316 /* SIFS + 3 slot time + CW min(31) = 10 + 3*9 + 31*9 = 316us */
#define VOW_DEF_GBAND_VI_BACKOFF    163 /* SIFS + 2 slot time + CW min(15) = 10 + 2*9 + 15*9 = 163us */
#define VOW_DEF_GBAND_VO_BACKOFF    91  /* SIFS + 2 slot time + CW min(7)  = 10 + 2*9 +  7*9 = 91us */

/* A band */
#define VOW_DEF_ABAND_BK_BACKOFF    358 /* SIFS + 7 slot time + CW min(31) = 16 + 7*9 + 31*9 = 358us */
#define VOW_DEF_ABAND_BE_BACKOFF    322 /* SIFS + 3 slot time + CW min(31) = 16 + 3*9 + 31*9 = 322us */
#define VOW_DEF_ABAND_VI_BACKOFF    169 /* SIFS + 2 slot time + CW min(15) = 16 + 2*9 + 15*9 = 169us */
#define VOW_DEF_ABAND_VO_BACKOFF    97  /* SIFS + 2 slot time + CW min(7)  = 16 + 2*9 +  7*9 = 97us */

/* for RX */
#define VOW_DEF_ED_OFFSET   4   /* OFDM CCA time */
#define VOW_DEF_GBAND_OBSS_BACKOFF   VOW_DEF_GBAND_BE_BACKOFF
#define VOW_DEF_ABAND_OBSS_BACKOFF   VOW_DEF_ABAND_BE_BACKOFF
#define VOW_DEF_NON_QOS_BACKOFF      VOW_DEF_ABAND_BE_BACKOFF

/* WMM backoff time selection */
#define VOW_WMM_ONE2ONE_MAPPING     1
#define VOW_WMM_SHARE_MAPPING       0

/* Weighted ATF */
#define VOW_WATF_LEVEL_NUM			4


/* MACRO for status */
#define VOW_IS_ENABLED(_pAd) ((_pAd != NULL) && \
							  ((_pAd->vow_cfg.en_bw_ctrl == TRUE) || \
							   (_pAd->vow_cfg.en_airtime_fairness == TRUE)))

/* for dummy group/sta id */
#define VOW_ALL_GROUP 0
#define VOW_ALL_STA	0

/* group id */
enum {
	VOW_GROUP_ID0,
	VOW_GROUP_ID1,
	VOW_GROUP_ID2,
	VOW_GROUP_ID3,
	VOW_GROUP_ID4,
	VOW_GROUP_ID5,
	VOW_GROUP_ID6,
	VOW_GROUP_ID7,
	VOW_GROUP_ID8,
	VOW_GROUP_ID9,
	VOW_GROUP_ID10,
	VOW_GROUP_ID11,
	VOW_GROUP_ID12,
	VOW_GROUP_ID13,
	VOW_GROUP_ID14,
	VOW_GROUP_ID15
};

enum {
	VOW_MIN,
	VOW_MAX
};

/* internal collision - search priority */
enum {
	VOW_WMM_AC_FIRST,
	VOW_WMM_SET_FIRST
};

/* station DWRR quantum index */
enum {
	VOW_STA_DWRR_IDX0,  /* unit: 256us */
	VOW_STA_DWRR_IDX1,
	VOW_STA_DWRR_IDX2,
	VOW_STA_DWRR_IDX3,
	VOW_STA_DWRR_IDX4,
	VOW_STA_DWRR_IDX5,
	VOW_STA_DWRR_IDX6,
	VOW_STA_DWRR_IDX7
};

/* station DWRR quantum */
enum {
	VOW_STA_DWRR_QUANTUM0 = 6,  /* unit: 256us */
	VOW_STA_DWRR_QUANTUM1 = 12,
	VOW_STA_DWRR_QUANTUM2 = 16,
	VOW_STA_DWRR_QUANTUM3 = 20,
	VOW_STA_DWRR_QUANTUM4 = 24,
	VOW_STA_DWRR_QUANTUM5 = 28,
	VOW_STA_DWRR_QUANTUM6 = 32,
	VOW_STA_DWRR_QUANTUM7 = 36
};
/* WMM set */
enum {
	VOW_WMM_SET0,
	VOW_WMM_SET1,
	VOW_WMM_SET2,
	VOW_WMM_SET3
};

/* AC priority change */
enum {
	VOW_DEFAULT_AC,
	VOW_HIGH_AC_TO_BE,
	VOW_HIGH_AC_TO_BK
};


/* token refill period */
typedef enum {
	VOW_REFILL_PERIOD_1US,
	VOW_REFILL_PERIOD_2US,
	VOW_REFILL_PERIOD_4US,
	VOW_REFILL_PERIOD_8US,
	VOW_REFILL_PERIOD_16US,
	VOW_REFILL_PERIOD_32US,
	VOW_REFILL_PERIOD_64US,
	VOW_REFILL_PERIOD_128US
} VOW_REFILL_PERIOD_T;

/* for group(BSS) */
typedef struct _VOW_BSS_USER_CFG_T {
	UINT16  min_rate;    /* guarantee rate(Mbps) */
	UINT16  max_rate;    /* ceiling rate(Mbps) */
	UINT8   min_airtime_ratio; /* guarantee airtime ratio(%) */
	UINT8   max_airtime_ratio; /* ceiling airtime ratio(%) */

	UINT16  min_ratebucket_size; /* guarantee rate bucket size(1024byte) */
	UINT16  max_ratebucket_size; /* ceiling rate bucket size(1024byte) */
	UINT16  max_backlog_size;    /* -1*max backlog size(1024byte) < token <= bucket size */
	UINT8   min_airtimebucket_size;   /* guarantee airtime token bucket size(1.024ms) */
	UINT8   max_airtimebucket_size;   /* ceiling airtime token bucket size(1.024ms) */
	UINT8   max_wait_time;    /* -1*maximum wait time(1.024ms) < token <= bucket size */
	UINT8   dwrr_quantum; /* for the same priority BSS, each BSS can TX quantum time(256us) */
	UINT16  min_rate_token;   /* the amount of guarantee rate token are filled periodically */
	UINT16  max_rate_token;   /* the amount of ceiling rate token are filled periodically */
	UINT16  min_airtime_token; /* the amount of guarantee airtime token are filled periodically */
	UINT16  max_airtime_token; /* the amount of ceiling airtime token are filled periodically */

	UINT8   band_idx;   /* belongs to which RF(Radio) band */

	BOOLEAN bw_on;   /* bandwidth control */
	BOOLEAN at_on;   /* airtime control */
} VOW_BSS_USER_CFG_T, *PVOW_BSS_USER_CFG_T;

/* for station */
typedef struct _VOW_STA_USER_CFG_T {
	UINT8       dwrr_quantum[WMM_NUM_OF_AC];   /* WMM AC */
	UINT8       group;      /* belongs to which control group */
	UINT8       wmm_idx;    /* belongs to which WMM group */
	UINT8       ac_change_rule; /* for dynamic AC changing. */
	BOOLEAN     paused; /* if this station TX is paused. */
} VOW_STA_USER_CFG_T, *PBW_STA_USER_CFG_T;

/* for control */
typedef struct _VOW_CFG_T {
	BOOLEAN en_bw_ctrl;  /* enable bandwidth(airtime) control */
	BOOLEAN en_bw_refill;   /* enable token refill */
	BOOLEAN en_airtime_fairness;    /* enable airtime fairness */
	BOOLEAN en_txop_no_change_bss;  /* enable HW doesn??¢t change BSS group in TXOP burst */
	BOOLEAN dbdc0_search_rule;  /* 1 WMM set first, 0  WMM AC first */
	BOOLEAN dbdc1_search_rule;  /* 1 WMM set first, 0  WMM AC first */
	UINT8   refill_period;  /* token refill period */
	UINT16  per_bss_enable; /* bitmap for per BSS group enable */
	UINT8   sta_max_wait_time;  /* for STA DWRR */
	UINT8   group_max_wait_time;  /* for STA DWRR */
	UINT8   vow_sta_dwrr_quantum[VOW_MAX_STA_DWRR_NUM]; /* STA DWRR quantum */
} VOW_CFG_T, *PVOW_CFG_T;

/* RX airtime */
typedef struct _VOW_RX_TIME_CFG_T {
	UINT8   ed_offset; /* normal WI-FI packet's duration of ED to CS for both CCK and OFDM. */
	UINT16  obss_backoff;   /* backoff threshold for OBSS */
	UINT8   wmm_backoff_sel[VOW_MAX_WMM_SET_NUM]; /* OM WMM backoff time selection */
	UINT16  wmm_backoff[VOW_MAX_WMM_SET_NUM][WMM_NUM_OF_AC]; /* WMM0~4 backoff time for 4 AC for MBSS or OM*/
	UINT16  om_wmm_backoff[WMM_NUM_OF_AC]; /* WMM0~4 backoff time for 4 AC for OM */
	UINT16  repeater_wmm_backoff[WMM_NUM_OF_AC]; /* WMM0~4 backoff time for 4 AC for repeater */
	UINT16  non_qos_backoff;    /* non-QOS packet backoff time */
	UINT8   bssid2wmm_set[VOW_MAX_GROUP_NUM];  /* BSSID belongs to which WMM set */
	BOOLEAN rx_time_en; /* enable RX time function, include non-wifi, OBSS and per-STA timer */
} VOW_RX_TIME_CFG_T, *PVOW_RX_TIME_CFG_T;


/* for airtime report */

/* for airtime estimator */
typedef struct _VOW_AT_ESTIMATOR_T {
	UINT8   at_estimator_en;
	UINT16  at_monitor_period;
} VOW_AT_ESTIMATOR_T, *P_VOW_AT_ESTIMATOR_T;

/* for bad node detector */
typedef struct _VOW_BAD_NODE_T {
	UINT8   bn_en;
	UINT16  bn_monitor_period;
	UINT16  bn_fallback_threshold;
	UINT16  bn_per_threshold;
} VOW_BAD_NODE_T, *P_VOW_BAD_NODE_T;

/* for UI - OID commands */
typedef struct _VOW_UI_CONFIG {
	UINT8   ssid_num;
	UINT16  val[0];
} VOW_UI_CONFIG, *P_VOW_UI_CONFIG;

/* for misc configuration */
typedef struct _VOW_MISC_CFG_T {
	BOOLEAN	rts_sta_lock;
	BOOLEAN keep_quantum;
	UINT8	tx_rr_count;
	BOOLEAN	measurement_mode;
	BOOLEAN max_backoff_bound_en;
	UINT8	max_backoff_bound;
	BOOLEAN	rts_failed_charge_time_en;
	BOOLEAN	zero_eifs_time;
	BOOLEAN	rx_rifs_mode;
	BOOLEAN	keep_vow_sram_setting;
	UINT8	keep_vow_sram_setting_bit;
} VOW_MISC_CFG_T;

/* function prototype */

/* weighted airtime fairness*/

typedef struct _VOW_WATF_ENTRY {
	UINT8 Addr[MAC_ADDR_LEN];
} VOW_WATF_ENTRY, *P_VOW_WATF_ENTRY;

typedef struct GNU_PACKED _VOW_WATF {
	UINT8 Num;
	VOW_WATF_ENTRY Entry[MAX_LEN_OF_MAC_TABLE];
} VOW_WATF, *PVOW_WATF;

/* for gen_1 & gen_2 - CR address offset */
typedef struct _VOW_CR_OFFSET_FOR_GEN_T {
	/* Is Gen_1 or Gen_2 */
	UINT8 VOW_GEN;

	/* VOW debug command 0x22/0x44 */
	UINT32 VOW_STA_SETTING_BEGIN;
	UINT32 VOW_STA_SETTING_END;
	UINT32 VOW_STA_BITMAP_BEGIN;
	UINT32 VOW_STA_BITMAP_END;
	UINT32 VOW_BSS_TOKEN_OFFSET;
	UINT32 VOW_STA_SETTING_FACTOR;

	/* STA setting */
	UINT32 VOW_STA_AC_PRIORITY_OFFSET;
	UINT32 VOW_STA_WMM_AC0_OFFSET;
	UINT32 VOW_STA_WMM_AC1_OFFSET;
	UINT32 VOW_STA_WMM_AC2_OFFSET;
	UINT32 VOW_STA_WMM_AC3_OFFSET;
	UINT32 VOW_STA_WMM_ID_OFFSET;
} VOW_CR_OFFSET_FOR_GEN_T, *PVOW_CR_OFFSET_FOR_GEN_T;


VOID vow_init(struct _RTMP_ADAPTER *pad);
VOID vow_init_CR_offset_gen_1_gen_2(struct _RTMP_ADAPTER *pad);
VOID vow_init_sta(struct _RTMP_ADAPTER *pad);
VOID vow_init_group(struct _RTMP_ADAPTER *pad);
VOID vow_init_rx(struct _RTMP_ADAPTER *pad);
VOID vow_init_misc(struct _RTMP_ADAPTER *pad);
VOID vow_reset(struct _RTMP_ADAPTER *pad);
VOID vow_reset_dvt(struct _RTMP_ADAPTER *pad);

UINT16 vow_convert_rate_token(struct _RTMP_ADAPTER *pad, UINT8 type, UINT8 group_id);
UINT16 vow_convert_airtime_token(struct _RTMP_ADAPTER *pad, UINT8 type, UINT8 group_id);
VOID vow_set_client(struct _RTMP_ADAPTER *pad, UINT8 group, UINT8 sta_id);
VOID vow_group_band_map(struct _RTMP_ADAPTER *pad, UCHAR band_idx, UCHAR group_idx);
VOID vow_mbss_grp_band_map(struct _RTMP_ADAPTER *pad, struct wifi_dev *wdev);
VOID vow_mbss_wmm_map(struct _RTMP_ADAPTER *pad, struct wifi_dev *wdev);
VOID vow_mbss_init(struct _RTMP_ADAPTER *pad, struct wifi_dev *wdev);
VOID vow_update_om_wmm(struct _RTMP_ADAPTER *pad, struct wifi_dev *wdev, struct _EDCA_PARM *pApEdcaParm);
BOOLEAN vow_is_enabled(struct _RTMP_ADAPTER *pad);
VOID vow_atf_off_init(struct _RTMP_ADAPTER *pad);
INT32 vow_set_sta(struct _RTMP_ADAPTER *pad, UINT8 sta_id, UINT32 subcmd);

INT set_vow_min_rate_token(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT set_vow_max_rate_token(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT set_vow_min_airtime_token(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT set_vow_max_airtime_token(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT set_vow_min_rate_bucket(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT set_vow_max_rate_bucket(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT set_vow_min_airtime_bucket(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT set_vow_max_airtime_bucket(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT set_vow_max_backlog_size(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT set_vow_max_wait_time(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT set_vow_group_dwrr_max_wait_time(
	IN  struct _RTMP_ADAPTER *pad,
	IN  RTMP_STRING * arg);

INT set_vow_sta_pause(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT set_vow_sta_group(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT set_vow_bw_en(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT set_vow_refill_en(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT set_vow_airtime_fairness_en(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT set_vow_txop_switch_bss_en(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT set_vow_dbdc_search_rule(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT set_vow_refill_period(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT set_vow_bss_en(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT set_vow_dwrr_max_wait_time(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT set_vow_airtime_ctrl_en(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT set_vow_bw_ctrl_en(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT set_vow_sta_dwrr_quantum(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT set_vow_sta_frr_quantum(
	IN	struct _RTMP_ADAPTER *pAd,
	IN	RTMP_STRING * arg);

INT set_vow_bss_dwrr_quantum(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT set_vow_sta_dwrr_quantum_id(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT set_vow_min_rate(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT set_vow_max_rate(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT set_vow_min_ratio(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT set_vow_max_ratio(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT set_vow_rx_counter_clr(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT set_vow_rx_airtime_en(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT set_vow_rx_ed_offset(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT set_vow_rx_obss_backoff(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT set_vow_rx_add_obss(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT set_vow_rx_add_non_wifi(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT set_vow_rx_wmm_backoff(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT set_vow_rx_non_qos_backoff(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT set_vow_rx_om_wmm_backoff(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT set_vow_rx_repeater_wmm_backoff(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT set_vow_rx_bss_wmmset(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT set_vow_rx_om_wmm_select(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT set_vow_charge_sta_dwrr(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT set_vow_charge_bw_time(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT set_vow_charge_bw_len(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT set_vow_charge_bw_dwrr(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT set_vow_life_time(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);
/* STA */

INT set_vow_sta_ac_priority(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT set_vow_sta_psm(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);
/* M2M test */
INT set_vow_pkt_ac(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT set_vow_monitor_sta(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT set_vow_monitor_bss(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT set_vow_monitor_mbss(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT set_vow_avg_num(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT set_vow_dvt_en(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT set_vow_cloned_wtbl(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT set_vow_sta_ack_all(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT set_vow_sta_ack(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT set_vow_sta_wmm_all(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT set_vow_sta_wmm(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT set_vow_sta_ac_all(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT set_vow_sta_ac(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT set_vow_tx_en(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT set_vow_tx_en_all(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT set_vow_sta_mbss(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT set_vow_show_sta(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT set_vow_show_mbss(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT show_vow_dump_vow(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT show_vow_dump_sta(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT show_vow_dump_bss_bitmap(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT show_vow_dump_bss(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT vow_show_bss_atoken(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT vow_show_bss_ltoken(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT vow_show_bss_dtoken(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT vow_show_sta_dtoken(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT vow_show_queue_status(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT set_vow_bcmc_en(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

VOID vow_display_info_periodic(
	IN  struct _RTMP_ADAPTER *pAd);

INT set_vow_q_len(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT set_vow_txs_en(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

/* for CR4 commands */
INT set_vow_sta_cnt(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT set_vow_sta_q(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT set_vow_sta2_q(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT set_vow_sta_th(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

/* show */
INT show_vow_rx_time(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT show_vow_get_sta_token(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT show_vow_sta_conf(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT show_vow_all_sta_conf(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT show_vow_bss_conf(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT show_vow_all_bss_conf(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT show_vow_info(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

/* airtime estimator */
INT set_vow_at_est_en(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT set_vow_at_mon_period(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT set_vow_group2band_map(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

/* bad node detector */
INT set_vow_bn_en(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT set_vow_bn_mon_period(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT set_vow_bn_fallback_th(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT set_vow_bn_per_th(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

/* VOW rx counter test */
INT set_vow_counter_test_en(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);
INT set_vow_counter_test_period(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);
INT set_vow_counter_test_band(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);
INT set_vow_counter_test_avgcnt(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);
INT set_vow_counter_test_target(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

/* help */

INT set_vow_help(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT show_vow_help(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

/* WATF */

VOID vow_reset_watf(
	IN struct _RTMP_ADAPTER *pad);

BOOLEAN vow_watf_is_enabled(
	IN struct _RTMP_ADAPTER *pad);

INT show_vow_watf_info(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT set_vow_watf_en(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT set_vow_watf_q(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT set_vow_watf_add_entry(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

INT set_vow_watf_del_entry(
	IN  struct _RTMP_ADAPTER *pAd,
	IN  RTMP_STRING * arg);

VOID set_vow_watf_sta_dwrr(
	IN	struct _RTMP_ADAPTER *pAd,
	IN	UINT8 *Addr,
	IN	UINT8 Wcid);
INT set_vow_fixed_rate(
    IN	struct _RTMP_ADAPTER *pAd,
    IN	RTMP_STRING * arg);

BOOLEAN vow_is_queue_full(
	IN  struct _RTMP_ADAPTER *pAd,
	UCHAR wcid,
	UCHAR qidx);

#endif /* __AP_VOW_H_ */

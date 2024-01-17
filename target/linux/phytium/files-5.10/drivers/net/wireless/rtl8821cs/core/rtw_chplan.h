/******************************************************************************
 *
 * Copyright(c) 2007 - 2018 Realtek Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 *****************************************************************************/
#ifndef __RTW_CHPLAN_H__
#define __RTW_CHPLAN_H__

#define RTW_CHPLAN_IOCTL_NULL			0xFFFE /* special value by ioctl: null(empty) chplan */
#define RTW_CHPLAN_IOCTL_UNSPECIFIED	0xFFFF /* special value by ioctl: no change (keep original) */

#define RTW_CHPLAN_NULL			0x1A
#define RTW_CHPLAN_WORLDWIDE	0x7F
#define RTW_CHPLAN_UNSPECIFIED	0xFF
#define RTW_CHPLAN_6G_NULL			0x00
#define RTW_CHPLAN_6G_WORLDWIDE		0x7F
#define RTW_CHPLAN_6G_UNSPECIFIED	0xFF

u8 rtw_chplan_get_default_regd_2g(u8 id);
u8 rtw_chplan_get_default_regd_5g(u8 id);
bool rtw_is_channel_plan_valid(u8 id);
bool rtw_regsty_is_excl_chs(struct registry_priv *regsty, u8 ch);

#if CONFIG_IEEE80211_BAND_6GHZ
u8 rtw_chplan_get_default_regd_6g(u8 id);
bool rtw_is_channel_plan_6g_valid(u8 id);
bool rtw_regsty_is_excl_chs_6g(struct registry_priv *regsty, u8 ch);
#endif

u8 rtw_chplan_is_chbw_valid(u8 id, u8 id_6g, BAND_TYPE band, u8 ch, u8 bw, u8 offset
	, bool allow_primary_passive, bool allow_passive, struct registry_priv *regsty);

enum regd_src_t {
	REGD_SRC_RTK_PRIV = 0, /* Regulatory settings from Realtek framework (Realtek defined or customized) */
	REGD_SRC_OS = 1, /* Regulatory settings from OS */
	REGD_SRC_NUM,
};

#define regd_src_is_valid(src) ((src) < REGD_SRC_NUM)

extern const char *_regd_src_str[];
#define regd_src_str(src) ((src) >= REGD_SRC_NUM ? _regd_src_str[REGD_SRC_NUM] : _regd_src_str[src])

struct _RT_CHANNEL_INFO;
u8 init_channel_set(_adapter *adapter);
bool rtw_chset_is_dfs_range(struct _RT_CHANNEL_INFO *chset, u32 hi, u32 lo);
bool rtw_chset_is_dfs_ch(struct _RT_CHANNEL_INFO *chset, u8 ch);
bool rtw_chset_is_dfs_chbw(struct _RT_CHANNEL_INFO *chset, u8 ch, u8 bw, u8 offset);
bool rtw_chinfo_allow_beacon_hint(struct _RT_CHANNEL_INFO *chinfo);
u8 rtw_process_beacon_hint(_adapter *adapter, WLAN_BSSID_EX *bss);

#define ALPHA2_FMT "%c%c"
#define ALPHA2_ARG(a2) ((is_alpha(a2[0]) || is_decimal(a2[0])) ? a2[0] : '-'), ((is_alpha(a2[1]) || is_decimal(a2[1])) ? a2[1] : '-')

#define WORLDWIDE_ALPHA2	"00"
#define UNSPEC_ALPHA2		"99"

#define IS_ALPHA2_WORLDWIDE(_alpha2)	(strncmp(_alpha2, WORLDWIDE_ALPHA2, 2) == 0)
#define IS_ALPHA2_UNSPEC(_alpha2)		(strncmp(_alpha2, UNSPEC_ALPHA2, 2) == 0)
#define SET_UNSPEC_ALPHA2(_alpha2)		do { _rtw_memcpy(_alpha2, UNSPEC_ALPHA2, 2); } while (0)

#define RTW_MODULE_RTL8821AE_HMC_M2		BIT0	/* RTL8821AE(HMC + M.2) */
#define RTW_MODULE_RTL8821AU			BIT1	/* RTL8821AU */
#define RTW_MODULE_RTL8812AENF_NGFF		BIT2	/* RTL8812AENF(8812AE+8761)_NGFF */
#define RTW_MODULE_RTL8812AEBT_HMC		BIT3	/* RTL8812AEBT(8812AE+8761)_HMC */
#define RTW_MODULE_RTL8188EE_HMC_M2		BIT4	/* RTL8188EE(HMC + M.2) */
#define RTW_MODULE_RTL8723BE_HMC_M2		BIT5	/* RTL8723BE(HMC + M.2) */
#define RTW_MODULE_RTL8723BS_NGFF1216	BIT6	/* RTL8723BS(NGFF1216) */
#define RTW_MODULE_RTL8192EEBT_HMC_M2	BIT7	/* RTL8192EEBT(8192EE+8761AU)_(HMC + M.2) */
#define RTW_MODULE_RTL8723DE_NGFF1630	BIT8	/* RTL8723DE(NGFF1630) */
#define RTW_MODULE_RTL8822BE			BIT9	/* RTL8822BE */
#define RTW_MODULE_RTL8821CE			BIT10	/* RTL8821CE */
#define RTW_MODULE_RTL8822CE			BIT11	/* RTL8822CE */

enum rtw_regd_inr {
	RTW_REGD_SET_BY_INIT = 0,
	RTW_REGD_SET_BY_USER = 1,
	RTW_REGD_SET_BY_COUNTRY_IE = 2,

	/* below is not used for REGD_SRC_RTK_PRIV */
	RTW_REGD_SET_BY_DRIVER = 3,
	RTW_REGD_SET_BY_CORE = 4,

	RTW_REGD_SET_BY_NUM,
};

extern const char *const _regd_inr_str[];
#define regd_inr_str(inr) (((inr) >= RTW_REGD_SET_BY_NUM) ? _regd_inr_str[RTW_REGD_SET_BY_NUM] : _regd_inr_str[(inr)])

enum rtw_regd {
	RTW_REGD_NA = 0,
	RTW_REGD_FCC = 1,
	RTW_REGD_MKK = 2,
	RTW_REGD_ETSI = 3,
	RTW_REGD_IC = 4,
	RTW_REGD_KCC = 5,
	RTW_REGD_NCC = 6,
	RTW_REGD_ACMA = 7,
	RTW_REGD_CHILE = 8,
	RTW_REGD_MEX = 9,
	RTW_REGD_WW,
	RTW_REGD_NUM,
};

extern const char *const _regd_str[];
#define regd_str(regd) (((regd) >= RTW_REGD_NUM) ? _regd_str[RTW_REGD_NA] : _regd_str[(regd)])

enum rtw_edcca_mode {
	RTW_EDCCA_NORMAL	= 0, /* normal */
	RTW_EDCCA_ADAPT		= 1, /* adaptivity */
	RTW_EDCCA_CS		= 2, /* carrier sense */

	RTW_EDCCA_MODE_NUM,
	RTW_EDCCA_DEF		= RTW_EDCCA_MODE_NUM, /* default (ref to domain code), used at country chplan map's override field */
};

extern const char *const _rtw_edcca_mode_str[];
#define rtw_edcca_mode_str(mode) (((mode) >= RTW_EDCCA_MODE_NUM) ? _rtw_edcca_mode_str[RTW_EDCCA_NORMAL] : _rtw_edcca_mode_str[(mode)])

enum rtw_dfs_regd {
	RTW_DFS_REGD_NONE	= 0,
	RTW_DFS_REGD_FCC	= 1,
	RTW_DFS_REGD_MKK	= 2,
	RTW_DFS_REGD_ETSI	= 3,
	RTW_DFS_REGD_NUM,
	RTW_DFS_REGD_AUTO	= 0xFF, /* follow channel plan */
};

extern const char *_rtw_dfs_regd_str[];
#define rtw_dfs_regd_str(region) (((region) >= RTW_DFS_REGD_NUM) ? _rtw_dfs_regd_str[RTW_DFS_REGD_NONE] : _rtw_dfs_regd_str[(region)])

typedef enum _REGULATION_TXPWR_LMT {
	TXPWR_LMT_NONE = 0, /* no limit */
	TXPWR_LMT_FCC = 1,
	TXPWR_LMT_MKK = 2,
	TXPWR_LMT_ETSI = 3,
	TXPWR_LMT_IC = 4,
	TXPWR_LMT_KCC = 5,
	TXPWR_LMT_NCC = 6,
	TXPWR_LMT_ACMA = 7,
	TXPWR_LMT_CHILE = 8,
	TXPWR_LMT_UKRAINE = 9,
	TXPWR_LMT_MEXICO = 10,
	TXPWR_LMT_CN = 11,
	TXPWR_LMT_QATAR = 12,
	TXPWR_LMT_WW, /* smallest of all available limit, keep last */

	TXPWR_LMT_NUM,
	TXPWR_LMT_DEF = TXPWR_LMT_NUM, /* default (ref to domain code), used at country chplan map's override field */
} REGULATION_TXPWR_LMT;

extern const char *const _txpwr_lmt_str[];
#define txpwr_lmt_str(regd) (((regd) >= TXPWR_LMT_NUM) ? _txpwr_lmt_str[TXPWR_LMT_NUM] : _txpwr_lmt_str[(regd)])

extern const REGULATION_TXPWR_LMT _txpwr_lmt_alternate[];
#define txpwr_lmt_alternate(ori) (((ori) > TXPWR_LMT_NUM) ? _txpwr_lmt_alternate[TXPWR_LMT_WW] : _txpwr_lmt_alternate[(ori)])

#define TXPWR_LMT_ALTERNATE_DEFINED(txpwr_lmt) (txpwr_lmt_alternate(txpwr_lmt) != txpwr_lmt)

extern const enum rtw_edcca_mode _rtw_regd_to_edcca_mode[RTW_REGD_NUM];
#define rtw_regd_to_edcca_mode(regd) (((regd) >= RTW_REGD_NUM) ? RTW_EDCCA_NORMAL : _rtw_regd_to_edcca_mode[(regd)])

extern const REGULATION_TXPWR_LMT _rtw_regd_to_txpwr_lmt[];
#define rtw_regd_to_txpwr_lmt(regd) (((regd) >= RTW_REGD_NUM) ? TXPWR_LMT_WW : _rtw_regd_to_txpwr_lmt[(regd)])

#define EDCCA_MODES_STR_LEN (((6 + 3 + 1) * BAND_MAX) + 1)
char *rtw_get_edcca_modes_str(char *buf, u8 modes[]);
void rtw_edcca_mode_update(struct dvobj_priv *dvobj);
u8 rtw_get_edcca_mode(struct dvobj_priv *dvobj, BAND_TYPE band);

#define TXPWR_NAMES_STR_LEN (((1 + 7 + 1) * BAND_MAX) + 1)
char *rtw_get_txpwr_lmt_names_str(char *buf, const char *names[], u8 unknown_bmp);

#define CHPLAN_PROTO_EN_AC		BIT0
#define CHPLAN_PROTO_EN_AX		BIT1
#define CHPLAN_PROTO_EN_ALL	0xFF

struct country_chplan {
	char alpha2[2];
	u8 chplan;
#if CONFIG_IEEE80211_BAND_6GHZ
	u8 chplan_6g;
#endif
	u8 edcca_mode_2g_override:2;
#if CONFIG_IEEE80211_BAND_5GHZ
	u8 edcca_mode_5g_override:2;
#endif
#if CONFIG_IEEE80211_BAND_6GHZ
	u8 edcca_mode_6g_override:2;
#endif
	u8 txpwr_lmt_override;
#if defined(CONFIG_80211AX_HE) || defined(CONFIG_80211AC_VHT)
	u8 proto_en;
#endif
};

#ifdef CONFIG_80211AC_VHT
#define COUNTRY_CHPLAN_EN_11AC(_ent) (((_ent)->proto_en & CHPLAN_PROTO_EN_AC) ? 1 : 0)
#else
#define COUNTRY_CHPLAN_EN_11AC(_ent) 0
#endif

#ifdef CONFIG_80211AX_HE
#define COUNTRY_CHPLAN_EN_11AX(_ent) (((_ent)->proto_en & CHPLAN_PROTO_EN_AX) ? 1 : 0)
#else
#define COUNTRY_CHPLAN_EN_11AX(_ent) 0
#endif

void rtw_get_chplan_worldwide(struct country_chplan *ent);
bool rtw_get_chplan_from_country(const char *country_code, struct country_chplan *ent);

void rtw_chplan_ioctl_input_mapping(u16 *chplan, u16 *chplan_6g);
bool rtw_chplan_ids_is_world_wide(u8 chplan, u8 chplan_6g);

u8 rtw_country_chplan_is_chbw_valid(struct country_chplan *ent, BAND_TYPE band, u8 ch, u8 bw, u8 offset
	, bool allow_primary_passive, bool allow_passive, struct registry_priv *regsty);

enum country_ie_slave_status {
	COUNTRY_IE_SLAVE_NOCOUNTRY		= 0,
	COUNTRY_IE_SLAVE_UNKNOWN		= 1,
	COUNTRY_IE_SLAVE_OPCH_NOEXIST	= 2,
	COUNTRY_IE_SLAVE_APPLICABLE		= 3,

	COUNTRY_IE_SLAVE_STATUS_NUM,
};

struct country_ie_slave_record {
	char alpha2[2]; /* country code get from connected AP of STA ifaces, "\x00\x00" is not set */
	BAND_TYPE band;
	u8 opch;
	enum country_ie_slave_status status;
};

#ifdef CONFIG_80211D
extern const char *const _cis_status_str[];
#define cis_status_str(s) (((s) >= COUNTRY_IE_SLAVE_STATUS_NUM) ? _cis_status_str[COUNTRY_IE_SLAVE_STATUS_NUM] : _cis_status_str[(s)])

void dump_country_ie_slave_records(void *sel, struct rf_ctl_t *rfctl, bool skip_noset);

enum country_ie_slave_status rtw_get_chplan_from_recv_country_ie(_adapter *adapter
		, BAND_TYPE band, u8 opch, const u8 *country_ie, struct country_chplan *ent
		, struct country_ie_slave_record *cisr, const char *caller_msg);

enum country_ie_slave_status rtw_get_chplan_from_cisrs(struct rf_ctl_t *rfctl
	, struct country_chplan *ent, const char *caller_msg);
#endif

void dump_country_chplan(void *sel, const struct country_chplan *ent, bool regd_info);
void dump_country_chplan_map(void *sel, bool regd_info);
void dump_country_list(void *sel);
void dump_chplan_id_list(void *sel);
void dump_chplan_country_list(void *sel);
#if CONFIG_IEEE80211_BAND_6GHZ
void dump_chplan_6g_id_list(void *sel);
void dump_chplan_6g_country_list(void *sel);
#endif
#ifdef CONFIG_RTW_DEBUG
void dump_chplan_test(void *sel);
#endif
void dump_chplan_ver(void *sel);

#endif /* __RTW_CHPLAN_H__ */

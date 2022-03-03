/* *
 * (c) Copyright 1997-2012, MediaTek, Inc.
 *
 * All rights reserved. MediaTek source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek Technology, Inc. is obtained.
 ***************************************************************************

*/

#ifndef __BE_EXPORT_H__
#define __BE_EXPORT_H__

#include "rtmp_def.h"

struct _RTMP_ADAPTER;

enum {
	WLAN_OPER_STATE_INVALID = 0,
	WLAN_OPER_STATE_VALID,
};

struct freq_cfg {
	UCHAR ht_bw;
	UCHAR vht_bw;
	UCHAR ext_cha;
	UCHAR prim_ch;
	UCHAR cen_ch_2;
};

struct freq_oper {
	UCHAR ht_bw;
	UCHAR vht_bw;
	UCHAR bw;
	UCHAR ext_cha;
	UCHAR prim_ch;
	UCHAR cen_ch_1;
	UCHAR cen_ch_2;
};

/*basic info obj */
/*
struct phy_cfg{
	UCHAR phy_mode;
	UCHAR prim_ch;
	UCHAR tx_stream;
	UCHAR rx_stream;
};

struct phy_op {
	UCHAR phy_mode;
	UCHAR prim_ch;
	UCHAR tx_stream;
	UCHAR rx_stream;
	UCHAR wdev_bw;
	UCHAR central1;
	UCHAR central2;
}

struct ht_info {
	UCHAR	ext_cha;
	UCHAR	ht_bw;
	UCHAR	oper_mode;
	UCHAR	mcs_set[16];
	BOOLEAN ht_en;
	BOOLEAN pre_nht_en;
	BOOLEAN gf;
	BOOLEAN	sgi_20;
	BOOLEAN sgi_40;
	BOOLEAN bss_coexist2040;
	BOOLEAN ldpc;
	BOOLEAN itx_bf;
	BOOLEAN etx_bf;
	BOOLEAN tx_stbc;
	BOOLEAN rx_stbc;
	struct ba_cap ba_cap;
};

struct ht_op_status {
	BOOLEAN obss_non_ht_exist;
	BOOLEAN non_gf_present;
	UCHAR	central_ch;
	HT_CAPABILITY_IE ht_cap;
	ADD_HT_INFO_IE addht;
	UINT16	non_gf_sta;
};

struct ba_cap {
	UCHAR mm_ps_mode;
	UCHAR amsdu_size;
	UCHAR mpdu_density;
	UCHAR policy;
	UCHAR tx_ba_win_limit;
	UCHAR rx_ba_win_limit;
	UCHAR max_ra_mpdu_factor;
	BOOLEAN amsdu_en;
	BOOLEAN auto_ba;
};

struct vht_info {
	BOOLEAN vht_en;
	BOOLEAN force_vht;
	UCHAR vht_bw;
	UCHAR vht_sgi;
	UCHAR vht_stbc;
	UCHAR vht_bw_signal;
	UCHAR vht_cent_ch;
	UCHAR vht_cent_ch2;
	UCHAR vht_mcs_cap;
	UCHAR vht_nss_cap;
	USHORT vht_tx_hrate;
	USHORT vht_rx_hrate;
	BOOLEAN ht20_forbid;
	BOOLEAN vht_ldpc;
	BOOLEAN g_band_256_qam;
};

struct vht_op_status{
};
*/

struct phy_op {
	UCHAR prim_ch;
	/*private attribute*/
	UCHAR wdev_bw;
	UCHAR cen_ch_1;
	UCHAR cen_ch_2;
	UINT8 tx_stream;
	UINT8 rx_stream;
};

struct ht_op {
	UCHAR	ext_cha;
	UCHAR	ht_bw;
	UCHAR	ht_stbc;
	UCHAR	ht_ldpc;
	UCHAR	ht_gi;
	UINT32	frag_thld;
	UCHAR	pkt_thld;
	UINT32	len_thld;
	UCHAR	retry_limit;
	UCHAR	l_sig_txop;
};

struct ht_op_status {
	/* Useful as AP. */
	ADD_HT_INFO_IE addht;
	HT_CAPABILITY_IE ht_cap_ie;
	/* counters */
	UINT16	non_gf_sta;
};

struct vht_op {
	UCHAR vht_bw;
	UCHAR vht_stbc;
	UCHAR vht_ldpc;
	UCHAR vht_sgi;
	UCHAR vht_bw_sig;
	UCHAR max_mpdu_len;
	UCHAR max_ampdu_exp;
};


struct vht_op_status {
	VHT_CAP_INFO vht_cap;
};

struct dev_rate_info {
	UCHAR SupRate[MAX_LEN_OF_SUPPORTED_RATES];
	UCHAR SupRateLen;
	UCHAR ExtRate[MAX_LEN_OF_SUPPORTED_RATES];
	UCHAR ExtRateLen;
	/* OID_802_11_DESIRED_RATES */
	UCHAR DesireRate[MAX_LEN_OF_SUPPORTED_RATES];
	UCHAR MaxDesiredRate;
	/* RATE_1, RATE_2, RATE_5_5, RATE_11 */
	UCHAR MaxTxRate;
	/* Tx rate index in Rate Switch Table */
	UCHAR TxRateIndex;
	/* RATE_1, RATE_2, RATE_5_5, RATE_11 */
	UCHAR MinTxRate;
	/* Same value to fill in TXD. TxRate is 6-bit */
	UCHAR TxRate;
#ifdef MIN_PHY_RATE_SUPPORT
	UCHAR MinPhyDataRate;
	UCHAR MinPhyBeaconRate;
	UCHAR MinPhyMgmtRate;
	UCHAR MinPhyBcMcRate;
	BOOL  LimitClientSupportRate;
	BOOL  DisableCCKRate;
	HTTRANSMIT_SETTING MinPhyDataRateTransmit;
	HTTRANSMIT_SETTING MinPhyBeaconRateTransmit;
	HTTRANSMIT_SETTING MinPhyMgmtRateTransmit;
	HTTRANSMIT_SETTING MinPhyBcMcRateTransmit;
#endif /* MIN_PHY_RATE_SUPPORT */

	/* MGMT frame PHY rate setting when operatin at Ht rate. */
	HTTRANSMIT_SETTING MlmeTransmit;
};

#define WLAN_OPER_OK	(0)
#define WLAN_OPER_FAIL	(-1)

/*
* Operate GET
*/
struct _ADD_HT_INFO_IE *wlan_operate_get_addht(struct wifi_dev *wdev);
UCHAR wlan_operate_get_bw(struct wifi_dev *wdev);
UCHAR wlan_operate_get_ht_bw(struct wifi_dev *wdev);
UCHAR wlan_operate_get_ht_stbc(struct wifi_dev *wdev);
UCHAR wlan_operate_get_ht_ldpc(struct wifi_dev *wdev);
UCHAR wlan_operate_get_ext_cha(struct wifi_dev *wdev);
#ifdef DOT11_VHT_AC
UCHAR wlan_operate_get_vht_bw(struct wifi_dev *wdev);
#endif /*DOT11_VHT_AC*/
UINT16 wlan_operate_get_non_gf_sta(struct wifi_dev *wdev);
UCHAR wlan_operate_get_prim_ch(struct wifi_dev *wdev);
UCHAR wlan_operate_get_cen_ch_2(struct wifi_dev *wdev);
UCHAR wlan_operate_get_cen_ch_1(struct wifi_dev *wdev);
UINT32 wlan_operate_get_frag_thld(struct wifi_dev *wdev);
UCHAR wlan_operate_get_rts_pkt_thld(struct wifi_dev *wdev);
UINT32 wlan_operate_get_rts_len_thld(struct wifi_dev *wdev);
UCHAR wlan_operate_get_rts_retry_limit(struct wifi_dev *wdev);
UINT8 wlan_operate_get_tx_stream(struct wifi_dev *wdev);
UINT8 wlan_operate_get_rx_stream(struct wifi_dev *wdev);
VOID *wlan_operate_get_ht_cap(struct wifi_dev *wdev);

VOID dump_ht_cap(struct wifi_dev *wdev);
/*
* Operate Set
*/

#ifdef DOT11_VHT_AC
INT32 wlan_operate_set_vht_bw(struct wifi_dev *wdev, UCHAR vht_bw);
#endif /*DOT11_VHT_AC*/
INT32 wlan_operate_set_support_ch_width_set(struct wifi_dev *wdev, UCHAR ch_width_set);
INT32 wlan_operate_set_ht_bw(struct wifi_dev *wdev, UCHAR ht_bw, UCHAR ext_cha);
INT32 wlan_operate_set_ht_stbc(struct wifi_dev *wdev, UCHAR ht_stbc);
INT32 wlan_operate_set_ht_ldpc(struct wifi_dev *wdev, UCHAR ht_ldpc);
INT32 wlan_operate_set_non_gf_sta(struct wifi_dev *wdev, UINT16 non_gf_sta);
INT32 wlan_operate_set_max_amsdu_len(struct wifi_dev *wdev, UCHAR len);
INT32 wlan_operate_set_prim_ch(struct wifi_dev *wdev, UCHAR prim_ch);
INT32 wlan_operate_set_cen_ch_2(struct wifi_dev *wdev, UCHAR cen_ch_2);
INT32 wlan_operate_set_phy(struct wifi_dev *wdev, struct freq_cfg *cfg);
INT32 wlan_operate_set_frag_thld(struct wifi_dev *wdev, UINT32 frag_thld);
INT32 wlan_operate_set_rts_pkt_thld(struct wifi_dev *wdev, UCHAR pkt_num);
INT32 wlan_operate_set_rts_len_thld(struct wifi_dev *wdev, UINT32 pkt_len);
INT32 wlan_operate_set_tx_stream(struct wifi_dev *wdev, UINT8 tx_stream);
INT32 wlan_operate_set_rx_stream(struct wifi_dev *wdev, UINT8 rx_stream);
INT32 wlan_operate_set_min_start_space(struct wifi_dev *wdev, UCHAR mpdu_density);
INT32 wlan_operate_set_mmps(struct wifi_dev *wdev, UCHAR mmps);
INT32 wlan_operate_set_ht_max_ampdu_len_exp(struct wifi_dev *wdev, UCHAR exp_factor);
INT32 wlan_operate_set_ht_delayed_ba(struct wifi_dev *wdev, UCHAR support);
INT32 wlan_operate_set_lsig_txop_protect(struct wifi_dev *wdev, UCHAR support);
INT32 wlan_operate_set_psmp(struct wifi_dev *wdev, UCHAR psmp);

/*
 * Operate Update
 */
VOID wlan_operate_update_ht_stbc(struct wifi_dev *wdev, UCHAR use_stbc);
VOID wlan_operate_update_ht_cap(struct wifi_dev *wdev);
/*
*
*/
VOID wlan_operate_init(struct wifi_dev *wdev);
VOID wlan_operate_exit(struct wifi_dev *wdev);
UCHAR wlan_operate_set_state(struct wifi_dev *wdev, UCHAR state);
UCHAR wlan_operate_get_state(struct wifi_dev *wdev);
BOOLEAN wlan_operate_scan(struct wifi_dev *wdev, UCHAR prim_ch);
#endif

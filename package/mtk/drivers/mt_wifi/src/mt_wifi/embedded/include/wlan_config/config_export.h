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

#ifndef __CONFIG_EXPORT_H__
#define __CONFIG_EXPORT_H__

struct _RTMP_ADAPTER;

struct wpf_data {
	UCHAR idx;
	void *dev;
	void *conf;
	void *oper;
};

struct wpf_ctrl {
	struct wpf_data pf[WDEV_NUM_MAX];
};

struct phy_cfg {
	UINT8 tx_stream;
	UINT8 rx_stream;
	UCHAR cen_ch_2;
	UCHAR ack_policy[WMM_NUM_OF_AC];
#ifdef TXBF_SUPPORT
	UCHAR ETxBfEnCond;
	UCHAR ITxBfEn;
#endif /* TXBF_SUPPORT */
};

struct ht_cfg {
	UCHAR	ext_cha;
	UCHAR	ht_bw;
	UCHAR	ht_stbc;
	UCHAR	ht_ldpc;
	UCHAR	ht_gi;
	UCHAR	ht_protect_en;
	UCHAR	ht_mode; /* mix_mode or gf_mode */
	UCHAR	gf_support;
	UCHAR	ht40_intolerant;
	UINT32	frag_thld;
	UCHAR	pkt_thld;
	UINT32	len_thld;
	UCHAR	min_mpdu_start_space;
	UCHAR	amsdu_en;
	UCHAR	mmps;
	/* EDCA parameters to be announced to its local BSS */
	struct _EDCA_PARM EdcaParm; 
};

struct vht_cfg {
	UCHAR 	vht_bw;
	UCHAR	vht_stbc;
	UCHAR	vht_ldpc;
	UCHAR	vht_sgi; /* including both bw80 & bw160 */
	UCHAR	vht_bw_sig; /* 0:n/a, 1:static, 2:dynamic */
};


/*for profile usage*/
VOID wpf_init(struct _RTMP_ADAPTER *ad);
VOID wpf_exit(struct _RTMP_ADAPTER *ad);
VOID wpf_config_exit(struct _RTMP_ADAPTER *ad);
VOID wpf_config_init(struct _RTMP_ADAPTER *ad);

/*
* Configure Get
*/
UCHAR wlan_config_get_ht_bw(struct wifi_dev *wdev);
UINT8 wlan_config_get_tx_stream(struct wifi_dev *wdev);
UINT8 wlan_config_get_rx_stream(struct wifi_dev *wdev);
#ifdef TXBF_SUPPORT
UCHAR wlan_config_get_etxbf(struct wifi_dev *wdev);
UCHAR wlan_config_get_itxbf(struct wifi_dev *wdev);
#endif /* TXBF_SUPPORT */
UCHAR wlan_config_get_ht_stbc(struct wifi_dev *wdev);
UCHAR wlan_config_get_ht_ldpc(struct wifi_dev *wdev);
#ifdef DOT11_VHT_AC
UCHAR wlan_config_get_vht_bw(struct wifi_dev *wdev);
UCHAR wlan_config_get_vht_stbc(struct wifi_dev *wdev);
UCHAR wlan_config_get_vht_ldpc(struct wifi_dev *wdev);
UCHAR wlan_config_get_vht_sgi(struct wifi_dev *wdev);
UCHAR wlan_config_get_vht_bw_sig(struct wifi_dev *wdev);
#endif /*DOT11_VHT_AC*/
UCHAR wlan_config_get_ext_cha(struct wifi_dev *wdev);
UCHAR wlan_config_get_cen_ch_2(struct wifi_dev *wdev);
UCHAR wlan_config_get_ack_policy(struct wifi_dev *wdev, UCHAR ac_id);
BOOLEAN wlan_config_get_edca_valid(struct wifi_dev *wdev);
struct _EDCA_PARM* wlan_config_get_ht_edca(struct wifi_dev *wdev);
UINT32 wlan_config_get_frag_thld(struct wifi_dev *wdev);
UINT32 wlan_config_get_rts_len_thld(struct wifi_dev *wdev);
UCHAR wlan_config_get_rts_pkt_thld(struct wifi_dev *wdev);
UCHAR wlan_config_get_ht_gi(struct wifi_dev *wdev);
UCHAR wlan_config_get_ht_protect_en(struct wifi_dev *wdev);
UCHAR wlan_config_get_ht_mode(struct wifi_dev *wdev);
UCHAR wlan_config_get_greenfield(struct wifi_dev *wdev);
UCHAR wlan_config_get_40M_intolerant(struct wifi_dev *wdev);
UCHAR wlan_config_get_amsdu_en(struct wifi_dev *wdev);
UCHAR wlan_config_get_min_mpdu_start_space(struct wifi_dev *wdev);
UCHAR wlan_config_get_mmps(struct wifi_dev *wdev);
/*
* Configure Set
*/
VOID wlan_config_set_ht_bw(struct wifi_dev *wdev, UCHAR ht_bw);
VOID wlan_config_set_ht_bw_all(struct wpf_ctrl *ctrl, UCHAR ht_bw);
VOID wlan_config_set_tx_stream(struct wifi_dev *wdev, UINT8 tx_stream);
VOID wlan_config_set_rx_stream(struct wifi_dev *wdev, UINT8 rx_stream);
#ifdef TXBF_SUPPORT
VOID wlan_config_set_etxbf(struct wifi_dev *wdev, UCHAR ETxBfEnCond);
VOID wlan_config_set_itxbf(struct wifi_dev *wdev, UCHAR ITxBfEn);
#endif /* TXBF_SUPPORT */
VOID wlan_config_set_ht_stbc(struct wifi_dev *wdev, UCHAR ht_stbc);
VOID wlan_config_set_ht_ldpc(struct wifi_dev *wdev, UCHAR ht_ldpc);
VOID wlan_config_set_ht_mode(struct wifi_dev *wdev, UCHAR ht_mode);
VOID wlan_config_set_40M_intolerant(struct wifi_dev *wdev, UCHAR val);
#ifdef DOT11_VHT_AC
VOID wlan_config_set_vht_stbc(struct wifi_dev *wdev, UCHAR vht_stbc);
VOID wlan_config_set_vht_ldpc(struct wifi_dev *wdev, UCHAR vht_ldpc);
VOID wlan_config_set_vht_sgi(struct wifi_dev *wdev, UCHAR vht_sgi);
VOID wlan_config_set_vht_bw_sig(struct wifi_dev *wdev, UCHAR vht_bw_sig);
VOID wlan_config_set_vht_bw(struct wifi_dev *wdev, UCHAR vht_bw);
VOID wlan_config_set_vht_bw_all(struct wpf_ctrl *ctrl, UCHAR vht_bw);
#endif /*DOT11_VHT_AC*/
VOID wlan_config_set_ht_ext_cha(struct wifi_dev *wdev, UCHAR ext_cha);
VOID wlan_config_set_ht_ext_cha_all(struct wpf_ctrl *ctrl, UCHAR ext_cha);
VOID wlan_config_set_ext_cha(struct wifi_dev *wdev, UCHAR ext_cha);
VOID wlan_config_set_cen_ch_2(struct wifi_dev *wdev, UCHAR cen_ch_2);
VOID wlan_config_set_cen_ch_2_all(struct wpf_ctrl *ctrl, UCHAR cen_ch_2);
VOID wlan_config_set_ack_policy(struct wifi_dev *wdev, UCHAR *policy);
VOID wlan_config_set_ack_policy_all(struct wpf_ctrl *ctrl,UCHAR *policy);
VOID wlan_config_set_edca_valid(struct wifi_dev *wdev, BOOLEAN bValid);
VOID wlan_config_set_edca_valid_all(struct wpf_ctrl *ctrl, BOOLEAN bValid);
VOID wlan_config_set_frag_thld(struct wifi_dev *wdev, UINT32 frag_thld);
VOID wlan_config_set_rts_len_thld(struct wifi_dev *wdev, UINT32 len_thld);
VOID wlan_config_set_rts_pkt_thld(struct wifi_dev *wdev, UCHAR pkt_thld);
VOID wlan_config_set_ht_gi(struct wifi_dev *wdev, UCHAR ht_git);
VOID wlan_config_set_ht_protect_en(struct wifi_dev *wdev, UCHAR ht_protect);
VOID wlan_config_set_amsdu_en(struct wifi_dev *wdev, UCHAR enable);
VOID wlan_config_set_min_mpdu_start_space(struct wifi_dev *wdev, UCHAR mpdu_density);
VOID wlan_config_set_mmps(struct wifi_dev *wdev, UCHAR mmps);
#endif

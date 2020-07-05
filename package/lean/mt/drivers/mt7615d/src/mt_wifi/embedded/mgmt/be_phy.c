/***************************************************************************
* MediaTek Inc.
* 4F, No. 2 Technology 5th Rd.
* Science-based Industrial Park
* Hsin-chu, Taiwan, R.O.C.
*
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

#include "rt_config.h"
#include "mgmt/be_internal.h"
#include "hdev/hdev.h"

/*
* define  constructor & deconstructor & method
*/
VOID phy_oper_init(struct wifi_dev *wdev, struct phy_op *obj)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	UINT8 ucTxPath = ad->Antenna.field.TxPath;
	UINT8 ucRxPath = ad->Antenna.field.RxPath;

#ifdef DBDC_MODE
	if (ad->CommonCfg.dbdc_mode) {
		UINT8 band_idx = HcGetBandByWdev(wdev);

		if (band_idx == DBDC_BAND0) {
			ucTxPath = ad->dbdc_band0_tx_path;
			ucRxPath = ad->dbdc_band0_rx_path;
		} else {
			ucTxPath = ad->dbdc_band1_tx_path;
			ucRxPath = ad->dbdc_band1_rx_path;
		}
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): Swap TX/RX Stream number to (%d,%d) since DBDC_MODE EN\n",
				 __func__, ucTxPath, ucRxPath));

	}
#endif

	obj->wdev_bw = BW_20;
	obj->tx_stream = wlan_config_get_tx_stream(wdev);
	obj->rx_stream = wlan_config_get_rx_stream(wdev);

	if ((obj->tx_stream == 0) || (obj->tx_stream > ucTxPath))
		obj->tx_stream = ucTxPath;

	if ((obj->rx_stream == 0) || (obj->rx_stream > ucRxPath))
		obj->rx_stream = ucRxPath;

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): operate TxStream = %d, RxStream = %d\n",
			 __func__, obj->tx_stream, obj->rx_stream));

}

VOID phy_oper_exit(struct phy_op *obj)
{
	os_zero_mem(obj, sizeof(*obj));
}

/*
* phy_freq internal related function
*/
static UCHAR phy_bw_adjust(UCHAR ht_bw, UCHAR vht_bw)
{
	UCHAR wdev_bw;

	if (ht_bw == HT_BW_40)
		ht_bw = HT_BW_40;
	else
		ht_bw = HT_BW_20;

	if (ht_bw == HT_BW_20)
		wdev_bw = BW_20;
	else {
#ifdef DOT11_VHT_AC

		if (vht_bw == VHT_BW_80)
			wdev_bw = BW_80;
		else if (vht_bw == VHT_BW_160)
			wdev_bw = BW_160;
		else if (vht_bw == VHT_BW_8080)
			wdev_bw = BW_8080;
		else
#endif /* DOT11_VHT_AC */
			wdev_bw = BW_40;
	}

	return wdev_bw;
}

static VOID phy_ht_vht_bw_adjust(UCHAR bw, UCHAR *ht_bw, UCHAR *vht_bw)
{
	if (bw < BW_40)
		*ht_bw = HT_BW_20;
	else
		*ht_bw = HT_BW_40;

#ifdef DOT11_VHT_AC
	*vht_bw = rf_bw_2_vht_bw(bw);
#endif /*DOT11_VHT_AC*/
	return;
}

static BOOLEAN phy_freq_adjust(struct wifi_dev *wdev, struct freq_cfg *cfg, struct freq_oper *op)
{
	UCHAR reg_cap_bw;
	UCHAR sec_ch_2_80_capable = 0;
	struct _RTMP_ADAPTER *ad = NULL;
	ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;

	/*initial to legacy setting*/
	if (cfg->prim_ch == 0) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s : no prim_ch value for adjust!\n", __func__));
		return FALSE;
	}

	op->ht_bw = HT_BW_20;
	op->ext_cha = EXTCHA_NONE;
#ifdef DOT11_VHT_AC
	op->vht_bw = VHT_BW_2040;
#endif /*DOT11_VHT_AC*/
	op->prim_ch = cfg->prim_ch;
	op->cen_ch_2 = 0;
	op->cen_ch_1 = op->prim_ch;
#ifdef DOT11_N_SUPPORT

	if (WMODE_CAP_N(wdev->PhyMode)) {
		op->ht_bw = cfg->ht_bw;
		op->ext_cha = cfg->ext_cha;
		ht_ext_cha_adjust(wdev->sys_handle, op->prim_ch, &op->ht_bw, &op->ext_cha, wdev);
	}

#ifdef DOT11_VHT_AC

	if (WMODE_CAP_AC(wdev->PhyMode))
		op->vht_bw = (op->ht_bw >= HT_BW_40) ? cfg->vht_bw : VHT_BW_2040;

#endif /*DOT11_VHT_AC*/
	op->bw = phy_bw_adjust(op->ht_bw, op->vht_bw);
	/*check region capability*/
	reg_cap_bw = get_channel_bw_cap(wdev, op->prim_ch);

	if ((op->bw > reg_cap_bw) && (!sec_ch_2_80_capable)) {
		if (!(op->bw == BW_8080 && reg_cap_bw == BW_160)) {
			op->bw = reg_cap_bw;
			phy_ht_vht_bw_adjust(op->bw, &op->ht_bw, &op->vht_bw);
		}
	}

	/*central ch*/
	if (op->bw == BW_40) {
		if (cal_ht_cent_ch(op->prim_ch, op->bw, op->ext_cha, &op->cen_ch_1) != TRUE) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s : buggy here.\n", __func__));
			return FALSE;
		}
	}

#ifdef DOT11_VHT_AC
	else if (op->bw > BW_40)
		op->cen_ch_1 = vht_cent_ch_freq(op->prim_ch, op->vht_bw);

	if (op->bw == BW_8080)
		op->cen_ch_2 = cfg->cen_ch_2;

#endif /*DOT11_VHT_AC*/
#endif /*DOT11_N_SUPPORT*/
	return TRUE;
}


static VOID phy_freq_update(struct wifi_dev *wdev, struct freq_oper *oper)
{
	struct wlan_operate *op = (struct wlan_operate *)wdev->wpf_op;
#ifdef RT_CFG80211_SUPPORT
	struct _RTMP_ADAPTER *ad = NULL;
	ADD_HT_INFO_IE *addht;
	addht = wlan_operate_get_addht(wdev);
	ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
#endif
	op->phy_oper.prim_ch = oper->prim_ch;
	operate_loader_prim_ch(op);
	op->phy_oper.cen_ch_1 = oper->cen_ch_1;
	op->phy_oper.cen_ch_2 = oper->cen_ch_2;
	op->phy_oper.wdev_bw = oper->bw;
	op->ht_oper.ht_bw = oper->ht_bw;
#ifdef RT_CFG80211_SUPPORT
	if ((ad->CommonCfg.LastBSSCoexist2040.field.BSS20WidthReq == 1) && (ad->MacTab.fAnyStaFortyIntolerant != TRUE) && ((ad->CommonCfg.BssCoexApCnt > 0))) {
		} else
#endif
	operate_loader_ht_bw(op);

	op->ht_oper.ext_cha = oper->ext_cha;
#ifdef RT_CFG80211_SUPPORT
    if ((ad->CommonCfg.LastBSSCoexist2040.field.BSS20WidthReq == 1) && (ad->MacTab.fAnyStaFortyIntolerant != TRUE) && ((ad->CommonCfg.BssCoexApCnt > 0))) {
		} else
#endif
	operate_loader_ext_cha(op);
#ifdef DOT11_VHT_AC
	op->vht_oper.vht_bw = oper->vht_bw;
	operate_loader_vht_bw(op);
#endif /*DOT11_VHT_AC*/
}

static VOID phy_freq_get_max(struct wifi_dev *wdev, struct freq_oper *result)
{
	struct wlan_operate *op = (struct wlan_operate *)wdev->wpf_op;

	if (op->phy_oper.prim_ch != result->prim_ch)
		return;

	/*bw*/
	if (op->phy_oper.wdev_bw > result->bw) {
		result->bw = op->phy_oper.wdev_bw;
		result->cen_ch_1 = op->phy_oper.cen_ch_1;
		result->cen_ch_2 = op->phy_oper.cen_ch_2;
		result->ext_cha = op->ht_oper.ext_cha;
	}
}

static VOID phy_freq_decision(struct wifi_dev *wdev, struct freq_oper *want, struct freq_oper *result)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	struct wifi_dev *cur_wdev;
	UCHAR i;
	/*basic setting*/
	os_move_mem(result, want, sizeof(struct freq_oper));

	/*check max capability for each operating*/
	for (i = 0; i < WDEV_NUM_MAX; i++) {
		cur_wdev = ad->wdev_list[i];

		if (cur_wdev &&
			wlan_operate_get_state(cur_wdev) &&
			wmode_band_equal(wdev->PhyMode, cur_wdev->PhyMode))
			phy_freq_get_max(cur_wdev, result);
	}
}

/*
* Utility
*/
VOID phy_freq_get_cfg(struct wifi_dev *wdev, struct freq_cfg *fcfg)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;
#ifdef BW_VENDOR10_CUSTOM_FEATURE
	struct _RTMP_ADAPTER *pAd = (struct _RTMP_ADAPTER *)wdev->sys_handle;
#endif

	os_zero_mem(fcfg, sizeof(struct freq_cfg));
	fcfg->prim_ch = wdev->channel;
	fcfg->cen_ch_2 = cfg->phy_conf.cen_ch_2;
#ifdef BW_VENDOR10_CUSTOM_FEATURE
	if (IS_APCLI_SYNC_PEER_DEAUTH_ENBL(pAd)) {
		fcfg->ht_bw = wlan_operate_get_ht_bw(wdev);
		fcfg->ext_cha = wlan_operate_get_ext_cha(wdev);
		fcfg->vht_bw = wlan_operate_get_vht_bw(wdev);
	} else {
#endif
	fcfg->ht_bw = cfg->ht_conf.ht_bw;
	fcfg->ext_cha = cfg->ht_conf.ext_cha;
	fcfg->vht_bw = cfg->vht_conf.vht_bw;
#ifdef BW_VENDOR10_CUSTOM_FEATURE
	}
#endif
}

/*
* Configure loader
*/

/*
* Operater loader
*/
VOID operate_loader_prim_ch(struct wlan_operate *op)
{
	UCHAR prim_ch = op->phy_oper.prim_ch;

	op->ht_status.addht.ControlChan = prim_ch;
}

/*
*
*/
VOID operate_loader_phy(struct wifi_dev *wdev, struct freq_cfg *cfg)
{
	struct freq_oper oper_dev;
	struct freq_oper oper_radio;
	struct radio_res res;
#ifdef CONFIG_AP_SUPPORT
#ifdef MT_DFS_SUPPORT
	struct _RTMP_ADAPTER *ad = NULL;
	if (wdev == NULL)
		return;
	ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
#endif
#endif

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("%s(): oper_cfg: prim_ch(%d), ht_bw(%d), extcha(%d), vht_bw(%d), cen_ch_2(%d), PhyMode=%d!\n", __func__,
			  cfg->prim_ch,
			  cfg->ht_bw,
			  cfg->ext_cha,
			  cfg->vht_bw,
			  cfg->cen_ch_2,
			  wdev->PhyMode));
	os_zero_mem(&oper_dev, sizeof(oper_dev));
	if (!phy_freq_adjust(wdev, cfg, &oper_dev))
		goto end;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("%s(): oper_dev after adjust: bw(%d), prim_ch(%d), cen_ch_1(%d), cen_ch_2(%d),ext_cha(%d)!\n", __func__,
			  oper_dev.bw,
			  oper_dev.prim_ch,
			  oper_dev.cen_ch_1,
			  oper_dev.cen_ch_2,
			  oper_dev.ext_cha));
	/*get last radio result for hdev check and update*/
	phy_freq_decision(wdev, &oper_dev, &oper_radio);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("%s(): oper_radio after decision: bw(%d), prim_ch(%d), cen_ch_1(%d), cen_ch_2(%d)!\n", __func__,
			  oper_radio.bw,
			  oper_radio.prim_ch,
			  oper_radio.cen_ch_1,
			  oper_radio.cen_ch_2));
	/*acquire radio resouce*/
	res.reason = REASON_NORMAL_SW;
	res.oper = &oper_radio;

#ifdef CONFIG_AP_SUPPORT
#ifdef MT_DFS_SUPPORT
	/*Perform CAC only for DFS Channel*/
	if (RadarChannelCheck(ad, oper_radio.prim_ch))
		DfsCacNormalStart(ad, wdev, RD_SILENCE_MODE);
#endif
#endif

	if (hc_radio_res_request(wdev, &res) != TRUE) {
		/*can't get radio resource, update operating to radio setting*/
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("%s(): oper_dev request radio fail! bw(%d), prim_ch(%d), cen_ch_1(%d), cen_ch_2(%d)!\n", __func__,
				  oper_dev.bw,
				  oper_dev.prim_ch,
				  oper_dev.cen_ch_1,
				  oper_dev.cen_ch_2));
		return;
	}

#ifdef CONFIG_AP_SUPPORT
#ifdef MT_DFS_SUPPORT
	DfsCacNormalStart(ad, wdev, RD_NORMAL_MODE);
	/*Perform CAC & Radar Detect only for DFS Channel*/
	if (RadarChannelCheck(ad, oper_radio.prim_ch)) {
		WrapDfsRadarDetectStart(ad, wdev);
	}
#endif
#endif

end:
	phy_freq_update(wdev, &oper_dev);
}


/*
* export function
*/

/*
* operation function
*/
UCHAR wlan_operate_get_bw(struct wifi_dev *wdev)
{
	struct wlan_operate *op = (struct wlan_operate *) wdev->wpf_op;

	return op->phy_oper.wdev_bw;
}

UCHAR wlan_operate_get_prim_ch(struct wifi_dev *wdev)
{
	struct wlan_operate *op = (struct wlan_operate *) wdev->wpf_op;

	return op->phy_oper.prim_ch;
}

INT32 wlan_operate_set_prim_ch(struct wifi_dev *wdev, UCHAR prim_ch)
{
	struct freq_cfg cfg;

	phy_freq_get_cfg(wdev, &cfg);
	cfg.prim_ch = prim_ch;
	operate_loader_phy(wdev, &cfg);
	return WLAN_OPER_OK;
}

INT32 wlan_operate_set_phy(struct wifi_dev *wdev, struct freq_cfg *cfg)
{
	operate_loader_phy(wdev, cfg);
	return WLAN_OPER_OK;
}


INT32 wlan_operate_set_tx_stream(struct wifi_dev *wdev, UINT8 tx_stream)
{
	struct wlan_operate *op = (struct wlan_operate *) wdev->wpf_op;

	op->phy_oper.tx_stream = tx_stream;
	return WLAN_OPER_OK;
}


INT32 wlan_operate_set_rx_stream(struct wifi_dev *wdev, UINT8 rx_stream)
{
	struct wlan_operate *op = (struct wlan_operate *) wdev->wpf_op;

	op->phy_oper.rx_stream = rx_stream;
	return WLAN_OPER_OK;
}


UCHAR wlan_operate_get_cen_ch_2(struct wifi_dev *wdev)
{
	struct wlan_operate *op = (struct wlan_operate *) wdev->wpf_op;

	return op->phy_oper.cen_ch_2;
}

INT32 wlan_operate_set_cen_ch_2(struct wifi_dev *wdev, UCHAR cen_ch_2)
{
	struct wlan_operate *op = (struct wlan_operate *) wdev->wpf_op;
	struct freq_cfg cfg;

	if (op->phy_oper.cen_ch_2 == cen_ch_2)
		return WLAN_OPER_OK;

	phy_freq_get_cfg(wdev, &cfg);
	cfg.cen_ch_2 = cen_ch_2;
	operate_loader_phy(wdev, &cfg);
	return WLAN_OPER_OK;
}

UCHAR wlan_operate_get_cen_ch_1(struct wifi_dev *wdev)
{
	struct wlan_operate *op = (struct wlan_operate *) wdev->wpf_op;

	return op->phy_oper.cen_ch_1;
}


UINT8 wlan_operate_get_tx_stream(struct wifi_dev *wdev)
{
	struct wlan_operate *op = (struct wlan_operate *) wdev->wpf_op;

	return op->phy_oper.tx_stream;
}


UINT8 wlan_operate_get_rx_stream(struct wifi_dev *wdev)
{
	struct wlan_operate *op = (struct wlan_operate *) wdev->wpf_op;

	return op->phy_oper.rx_stream;
}


BOOLEAN wlan_operate_scan(struct wifi_dev *wdev, UCHAR prim_ch)
{
	struct radio_res radio, *res = &radio;
	struct freq_oper oper;
	BOOLEAN ret;

	res->oper = &oper;
	oper.bw = BW_20;
	oper.cen_ch_1 = prim_ch;
	oper.ext_cha = EXTCHA_NONE;
	oper.ht_bw = HT_BW_20;
#ifdef DOT11_VHT_AC
	oper.cen_ch_2 = 0;
	oper.vht_bw = VHT_BW_2040;
#endif /*DOT11_VHT_AC*/
	oper.prim_ch = prim_ch;
	res->reason = REASON_NORMAL_SCAN;
	ret = hc_radio_res_request(wdev, res);
	return ret;
}

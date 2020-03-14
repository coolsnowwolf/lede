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


VOID dump_ht_cap(struct wifi_dev *wdev)
{
	HT_CAPABILITY_IE *ht_cap = NULL;

	if (wdev) {
		ht_cap = (HT_CAPABILITY_IE *)wlan_operate_get_ht_cap(wdev);

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("Caller: %pS\n", __builtin_return_address(0)));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("phy_mode=%d, ch=%d, wdev_type=%d\n",
				 wdev->PhyMode, wdev->channel, wdev->wdev_type));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("ht_cap->HtCapInfo: ldpc=%d,ch_width=%d,gf=%d,sgi20=%d,sgi40=%d,tx_stbc=%d,rx_stbc=%d,amsdu_size=%d\n",
				 ht_cap->HtCapInfo.ht_rx_ldpc, ht_cap->HtCapInfo.ChannelWidth,
				 ht_cap->HtCapInfo.GF, ht_cap->HtCapInfo.ShortGIfor20, ht_cap->HtCapInfo.ShortGIfor40,
				 ht_cap->HtCapInfo.TxSTBC, ht_cap->HtCapInfo.RxSTBC, ht_cap->HtCapInfo.AMsduSize));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("ht_cap->HtCapParm: mdpu_density=%d, ampdu_factor=%d\n",
			 ht_cap->HtCapParm.MpduDensity, ht_cap->HtCapParm.MaxRAmpduFactor));
	}
}

/*init radio*/
static VOID radio_operate_init(struct wifi_dev *wdev)
{
	struct freq_cfg fcfg;
#ifdef CONFIG_AP_SUPPORT
	struct _RTMP_ADAPTER *ad = NULL;

	if (wdev == NULL)
		return;
	ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;

	if (CheckNonOccupancyChannel(ad, wdev) == FALSE)
		wdev->channel = FirstChannel(ad, wdev);
#endif

	phy_freq_get_cfg(wdev, &fcfg);

#ifdef CONFIG_AP_SUPPORT
		/*Check RDMode before setting channel*/
#ifdef MT_DFS_SUPPORT
	if (!DfsBypassRadarStateCheck(wdev))
#endif
		RadarStateCheck(ad, wdev);
#endif
	operate_loader_phy(wdev, &fcfg);
}


/*
* private structure definition to prevent direct access
*/
VOID wlan_operate_init(struct wifi_dev *wdev)
{
	struct wlan_operate *obj = (struct wlan_operate *)wdev->wpf_op;

	if (!obj) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("%s : Can't find wlan operate for wdev.\n", __func__));
		return;
	}

	/*reset to default*/
	phy_oper_init(wdev, &obj->phy_oper);
	ht_oper_init(wdev, &obj->ht_oper);
	ht_op_status_init(wdev, &obj->ht_status);
#ifdef DOT11_VHT_AC
	vht_oper_init(wdev, &obj->vht_oper);
	vht_op_status_init(wdev, &obj->vht_status);
#endif /* DOT11_VHT_AC */
	/*adjust radio operate from configure*/
	radio_operate_init(wdev);

	dump_ht_cap(wdev);
}

VOID wlan_operate_exit(struct wifi_dev *wdev)
{
	struct wlan_operate *obj = (struct wlan_operate *)wdev->wpf_op;

	if (!obj) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("%s : Can't find wlan operate for wdev.\n", __func__));
		return;
	}

	phy_oper_exit(&obj->phy_oper);
	ht_oper_exit(&obj->ht_oper);
#ifdef DOT11_VHT_AC
	vht_oper_exit(&obj->vht_oper);
#endif /* DOT11_VHT_AC */
	ht_op_status_exit(&obj->ht_status);
#ifdef DOT11_VHT_AC
	vht_op_status_exit(&obj->vht_status);
#endif /* DOT11_VHT_AC */
}

UCHAR wlan_operate_get_state(struct wifi_dev *wdev)
{
	struct wlan_operate *obj = (struct wlan_operate *)wdev->wpf_op;

	if (!obj) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s : Can't find wlan operate for wdev.\n", __func__));
		return WLAN_OPER_STATE_INVALID;
	}

	return obj->state;
}

UCHAR wlan_operate_set_state(struct wifi_dev *wdev, UCHAR state)
{
	struct wlan_operate *obj = (struct wlan_operate *)wdev->wpf_op;

	if (!obj) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s : Can't find wlan operate for wdev.\n", __func__));
		return WLAN_OPER_FAIL;
	}

	obj->state = state;
	return WLAN_OPER_OK;
}


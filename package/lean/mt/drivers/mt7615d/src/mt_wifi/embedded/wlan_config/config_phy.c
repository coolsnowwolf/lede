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
#include "wlan_config/config_internal.h"


/*
* define  constructor & deconstructor & method
*/
/*
*basic phy related
*/
VOID phy_cfg_init(struct phy_cfg *obj)
{
#ifdef TXBF_SUPPORT
    obj->ETxBfEnCond = SUBF_OFF;
    obj->ITxBfEn = SUBF_OFF;
#endif /* TXBF_SUPPORT */
}

VOID phy_cfg_exit(struct phy_cfg *obj)
{
	os_zero_mem(obj, sizeof(struct phy_cfg));
}

/*
* Operater loader
*/


/*
* export function
*/
/*
* configure functio
*/
VOID wlan_config_set_cen_ch_2(struct wifi_dev *wdev, UCHAR cen_ch_2)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	cfg->phy_conf.cen_ch_2 = cen_ch_2;
}

VOID wlan_config_set_cen_ch_2_all(struct wpf_ctrl *ctrl, UCHAR cen_ch_2)
{
	struct wlan_config *cfg;
	unsigned int i;

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		cfg = (struct wlan_config *)ctrl->pf[i].conf;
		cfg->phy_conf.cen_ch_2 = cen_ch_2;
	}
}

/*
*Set
*/
VOID wlan_config_set_ack_policy(struct wifi_dev *wdev,UCHAR *policy)
{
	struct wlan_config *cfg = (struct wlan_config*)wdev->wpf_cfg;
	UCHAR i=0;
	for(i = 0; i < WMM_NUM_OF_AC ; i++){
		cfg->phy_conf.ack_policy[i] = policy[i];
	}
}

VOID wlan_config_set_ack_policy_all(struct wpf_ctrl *ctrl,UCHAR *policy)
{
	struct wlan_config *cfg;
	UCHAR i;
	UCHAR j;
	for(i=0;i<WDEV_NUM_MAX;i++){
		cfg = (struct wlan_config*)ctrl->pf[i].conf;
		for(j = 0; j < WMM_NUM_OF_AC ; j++){
			cfg->phy_conf.ack_policy[j] = policy[j];
		}
	}
}

VOID wlan_config_set_tx_stream(struct wifi_dev *wdev, UINT8 tx_stream)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	cfg->phy_conf.tx_stream = tx_stream;
}

VOID wlan_config_set_rx_stream(struct wifi_dev *wdev, UINT8 rx_stream)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	cfg->phy_conf.rx_stream = rx_stream;
}

#ifdef TXBF_SUPPORT
VOID wlan_config_set_etxbf(struct wifi_dev *wdev, UCHAR ETxBfEnCond)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	cfg->phy_conf.ETxBfEnCond = ETxBfEnCond;

}

VOID wlan_config_set_itxbf(struct wifi_dev *wdev, UCHAR ITxBfEn)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	cfg->phy_conf.ITxBfEn = ITxBfEn;

}
#endif /* TXBF_SUPPORT */

/*
*Get
*/
UCHAR wlan_config_get_cen_ch_2(struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;
	return cfg->phy_conf.cen_ch_2;
}

UCHAR wlan_config_get_ack_policy(struct wifi_dev *wdev, UCHAR ac_id)
{
	struct wlan_config *cfg = (struct wlan_config*)wdev->wpf_cfg;
	return cfg->phy_conf.ack_policy[ac_id];
}

UINT8 wlan_config_get_tx_stream(struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	return cfg->phy_conf.tx_stream;
}

UINT8 wlan_config_get_rx_stream(struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	return cfg->phy_conf.rx_stream;
}

#ifdef TXBF_SUPPORT
UCHAR wlan_config_get_etxbf(struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;
	return cfg->phy_conf.ETxBfEnCond;
}

UCHAR wlan_config_get_itxbf(struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;
	return cfg->phy_conf.ITxBfEn;
}
#endif /* TXBF_SUPPORT */

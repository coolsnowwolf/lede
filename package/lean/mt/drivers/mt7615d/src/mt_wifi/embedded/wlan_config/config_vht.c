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
*vht phy related
*/
#ifdef DOT11_VHT_AC
VOID vht_cfg_init(struct vht_cfg *obj)
{
	obj->vht_bw = VHT_BW_80;
	obj->vht_ldpc = TRUE;
	obj->vht_stbc = STBC_USE;
	obj->vht_sgi = GI_400;
	obj->vht_bw_sig = BW_SIGNALING_DISABLE;
}

VOID vht_cfg_exit(struct vht_cfg *obj)
{
	os_zero_mem(obj, sizeof(struct vht_cfg));
}

/*
*vht phy op related
*/
VOID vht_op_status_init(struct wifi_dev *wdev, struct vht_op_status *obj)
{
}

VOID vht_op_status_exit(struct vht_op_status *obj)
{
	os_zero_mem(obj, sizeof(struct vht_op_status));
}

/*
* exported operation function.
*/

/*
* exported configure function.
*/

/*
* Set
*/
VOID wlan_config_set_vht_bw(struct wifi_dev *wdev, UCHAR vht_bw)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	cfg->vht_conf.vht_bw = vht_bw;
}

VOID wlan_config_set_vht_bw_all(struct wpf_ctrl *ctrl, UCHAR vht_bw)
{
	struct wlan_config *cfg;
	unsigned int i;

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		cfg = (struct wlan_config *)ctrl->pf[i].conf;
		cfg->vht_conf.vht_bw = vht_bw;
	}
}

VOID wlan_config_set_vht_stbc(struct wifi_dev *wdev, UCHAR vht_stbc)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	cfg->vht_conf.vht_stbc = vht_stbc;
}

VOID wlan_config_set_vht_ldpc(struct wifi_dev *wdev, UCHAR vht_ldpc)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	cfg->vht_conf.vht_ldpc = vht_ldpc;
}

VOID wlan_config_set_vht_sgi(struct wifi_dev *wdev, UCHAR vht_sgi)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	cfg->vht_conf.vht_sgi = vht_sgi;
}

VOID wlan_config_set_vht_bw_sig(struct wifi_dev *wdev, UCHAR vht_bw_sig)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	cfg->vht_conf.vht_bw_sig = vht_bw_sig;
}

/*
* Get
*/
UCHAR wlan_config_get_vht_bw(struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	return cfg->vht_conf.vht_bw;
}

UCHAR wlan_config_get_vht_stbc(struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	return cfg->vht_conf.vht_stbc;
}

UCHAR wlan_config_get_vht_ldpc(struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	return cfg->vht_conf.vht_ldpc;
}

UCHAR wlan_config_get_vht_sgi(struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	return cfg->vht_conf.vht_sgi;
}

UCHAR wlan_config_get_vht_bw_sig(struct wifi_dev *wdev)
{
	struct wlan_config *cfg = (struct wlan_config *)wdev->wpf_cfg;

	return cfg->vht_conf.vht_bw_sig;
}

#endif /*DOT11_VHT_AC*/

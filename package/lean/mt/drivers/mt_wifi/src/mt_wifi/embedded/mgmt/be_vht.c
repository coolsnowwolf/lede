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


/*
*vht phy related
*/
#ifdef DOT11_VHT_AC
VOID vht_oper_init(struct wifi_dev *wdev, struct vht_op *obj)
{
	obj->vht_bw = VHT_BW_80;
}

VOID vht_oper_exit(struct vht_op *obj)
{
	os_zero_mem(obj, sizeof(*obj));
}


/*
* internal used configure loader
*/
/*
* exported operation function.
*/

VOID operate_loader_vht_bw(struct wlan_operate *op)
{
}
/*
* Set
*/
INT32 wlan_operate_set_vht_bw(struct wifi_dev *wdev, UCHAR vht_bw)
{
	struct wlan_operate *op = (struct wlan_operate *) wdev->wpf_op;
	UCHAR cap_vht_bw = wlan_config_get_vht_bw(wdev);
	INT32 ret = WLAN_OPER_OK;
	struct freq_cfg cfg;

	if (vht_bw == op->vht_oper.vht_bw)
		return ret;

	if (vht_bw  > cap_vht_bw) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s(): new vht_bw:%d > cap_vht_bw: %d, correct to cap_vht_bw\n",
				  __func__,
				  vht_bw,
				  cap_vht_bw
				 ));
		vht_bw = cap_vht_bw;
		ret = WLAN_OPER_FAIL;
	}

	/*configure loader*/
	phy_freq_get_cfg(wdev, &cfg);
	cfg.vht_bw = vht_bw;
	operate_loader_phy(wdev, &cfg);
	return ret;
}

/*
* Get
*/
UCHAR wlan_operate_get_vht_bw(struct wifi_dev *wdev)
{
	struct wlan_operate *op = (struct wlan_operate *)wdev->wpf_op;

	return op->vht_oper.vht_bw;
}
#endif /* DOT11_VHT_AC */

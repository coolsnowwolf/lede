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
* private structure definition to prevent direct access
*/
static VOID wlan_config_init(struct wlan_config *obj)
{
	phy_cfg_init(&obj->phy_conf);
	ht_cfg_init(&obj->ht_conf);
#ifdef DOT11_VHT_AC
	vht_cfg_init(&obj->vht_conf);
#endif /* DOT11_VHT_AC */
}

static VOID wlan_config_exit(struct wlan_config *obj)
{
	phy_cfg_exit(&obj->phy_conf);
	ht_cfg_exit(&obj->ht_conf);
#ifdef DOT11_VHT_AC
	vht_cfg_exit(&obj->vht_conf);
#endif /* DOT11_VHT_AC */
}

static INT32 wpf_config_acquire(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev)
{
	int i;
	struct wpf_ctrl *ctrl = &ad->wpf;
	struct wpf_data *pf;

	if (wdev->wpf_cfg || wdev->wpf_op)
		return 0;

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		pf = &ctrl->pf[i];

		if (pf->dev == NULL) {
			wdev->wpf_cfg = pf->conf;
			wdev->wpf_op = pf->oper;
			pf->dev = wdev;
			return 0;
		}
	}

	return -1;
}

static INT32 wpf_config_release(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev)
{
	int i;
	struct wpf_ctrl *ctrl = &ad->wpf;
	struct wpf_data *pf;

	if ((wdev->wpf_cfg == NULL) && (wdev->wpf_op == NULL))
		return 0;

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		pf = &ctrl->pf[i];

		if (pf->dev == wdev) {
			pf->dev = NULL;
			break;
		}
	}

	wdev->wpf_cfg = NULL;
	wdev->wpf_op = NULL;
	return 0;
}

VOID wpf_config_init(struct _RTMP_ADAPTER *ad)
{
	int i;
	struct wpf_ctrl *ctrl = &ad->wpf;
	struct wpf_data *pf;

	os_zero_mem(ctrl->pf, sizeof(ctrl->pf));

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		pf = &ctrl->pf[i];
		pf->idx = i;
		os_alloc_mem(NULL, (UCHAR **)&pf->conf, sizeof(struct wlan_config));

		if (pf->conf) {
			os_zero_mem(pf->conf, sizeof(struct wlan_config));
			wlan_config_init(pf->conf);
		}

		os_alloc_mem(NULL, (UCHAR **)&pf->oper, sizeof(struct wlan_operate));

		if (pf->oper)
			os_zero_mem(pf->oper, sizeof(struct wlan_operate));
	}
}

VOID wpf_config_exit(struct _RTMP_ADAPTER *ad)
{
	int i;
	struct wpf_ctrl *ctrl = &ad->wpf;
	struct wpf_data *pf;

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		pf = &ctrl->pf[i];
		pf->idx = i;

		if (pf->conf) {
			wlan_config_exit(pf->conf);
			os_free_mem(pf->conf);
		}

		pf->conf = NULL;

		if (pf->oper)
			os_free_mem(pf->oper);

		pf->oper = NULL;
	}

	os_zero_mem(ctrl->pf, sizeof(ctrl->pf));
}

/*
* assign order: ( MBSS | STA ) >  WDS  > APCLI > P2P > MESH
*/
VOID wpf_init(struct _RTMP_ADAPTER *ad)
{
	struct wifi_dev *wdev;
	int i;
	/*do not change order*/
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(ad) {
		for (i = 0; i < MAX_MBSSID_NUM(ad); i++) {
			wdev = &ad->ApCfg.MBSSID[i].wdev;

			if (wpf_config_acquire(ad, wdev) < 0) {
				MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s:[ERROR] wdev_cfg is full!\n",
						  __func__));
				return;
			}
		}

#ifdef WDS_SUPPORT

		for (i = 0; i < MAX_WDS_ENTRY; i++) {
			wdev = &ad->WdsTab.WdsEntry[i].wdev;

			if (wpf_config_acquire(ad, wdev) < 0) {
				MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s:[ERROR] wdev_cfg is full!\n",
						  __func__));
				return;
			}
		}

#endif /*WDS_SUPPORT*/
#ifdef APCLI_SUPPORT

		for (i = 0; i < MAX_APCLI_NUM; i++) {
			wdev = &ad->ApCfg.ApCliTab[i].wdev;

			if (wpf_config_acquire(ad, wdev) < 0) {
				MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s:[ERROR] wdev_cfg is full!\n",
						  __func__));
				return;
			}
		}

#endif /*APCLI_SUPPORT*/
	}
#endif /*CONFIG_AP_SUPPORT*/
}

VOID wpf_exit(struct _RTMP_ADAPTER *ad)
{
	struct wifi_dev *wdev;
	int i;
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(ad) {
		for (i = 0; i < MAX_MBSSID_NUM(ad); i++) {
			wdev = &ad->ApCfg.MBSSID[i].wdev;

			if (wpf_config_release(ad, wdev) < 0) {
				MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s:[ERROR] wdev_cfg is full!\n",
						  __func__));
				return;
			}
		}

#ifdef WDS_SUPPORT

		for (i = 0; i < MAX_WDS_ENTRY; i++) {
			wdev = &ad->WdsTab.WdsEntry[i].wdev;

			if (wpf_config_release(ad, wdev) < 0) {
				MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s:[ERROR] wdev_cfg is full!\n",
						  __func__));
				return;
			}
		}

#endif /*WDS_SUPPORT*/
#ifdef APCLI_SUPPORT

		for (i = 0; i < MAX_APCLI_NUM; i++) {
			wdev = &ad->ApCfg.ApCliTab[i].wdev;

			if (wpf_config_release(ad, wdev) < 0) {
				MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("%s:[ERROR] wdev_cfg is full!\n",
						  __func__));
				return;
			}
		}

#endif /*APCLI_SUPPORT*/
	}
#endif /*CONFIG_AP_SUPPORT*/
}

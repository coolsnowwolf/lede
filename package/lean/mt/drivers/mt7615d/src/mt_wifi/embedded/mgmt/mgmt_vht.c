/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:

	Abstract:

	Revision History:
	Who		When			What
	--------	----------		----------------------------------------------
*/

#include "rt_config.h"


#ifdef DOT11_VHT_AC

static char *vht_bw_str[] = {
	"20/40",
	"80",
	"160",
	"80+80",
	"invalid",
};

char *VhtBw2Str(INT VhtBw)
{
	if (VhtBw <= VHT_BW_8080)
		return vht_bw_str[VhtBw];
	else
		return vht_bw_str[4];
}
/*
	========================================================================
	Routine Description:
		Caller ensures we has 802.11ac support.
		Calls at setting VHT from AP/STASetinformation

	Arguments:
		pAd - Pointer to our adapter
		phymode  -

	========================================================================
*/
VOID RTMPSetVHT(
	IN RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev)
{
#ifdef VHT_TXBF_SUPPORT
	VHT_CAP_INFO *vht_cap = &pAd->CommonCfg.vht_cap_ie.vht_cap;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (cap->FlgHwTxBfCap) {
		/* Set ETxBF */
#ifdef MT_MAC
		mt_WrapSetVHTETxBFCap(pAd, wdev, vht_cap);
#else
		setVHTETxBFCap(pAd, vht_cap);
#endif
	}

#endif /* TXBF_SUPPORT */
}


VOID rtmp_set_vht(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, RT_PHY_INFO *phy_info)
{
	UCHAR vht_bw;

	if (!phy_info)
		return;

	vht_bw = wlan_config_get_vht_bw(wdev);

	if (phy_info->bVhtEnable) {
		if (vht_bw <= VHT_BW_8080)
			phy_info->vht_bw = vht_bw;
		else
			phy_info->vht_bw = VHT_BW_2040;
	}
}


INT SetCommonVHT(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	if (!WMODE_CAP_AC(wdev->PhyMode)) {
		/* Clear previous VHT information */
		return FALSE;
	}

	RTMPSetVHT(pAd, wdev);
	return TRUE;
}

#endif /* DOT11_VHT_AC */

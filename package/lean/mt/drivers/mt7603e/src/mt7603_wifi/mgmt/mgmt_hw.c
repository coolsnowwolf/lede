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
	Who 		When			What
	--------	----------		----------------------------------------------
*/

#include <rt_config.h>


INT dev_adjust_radio(RTMP_ADAPTER *pAd)
{
	struct hw_setting new_cfg, *hw_cfg = &pAd->hw_cfg;


	NdisZeroMemory(&new_cfg, sizeof(struct hw_setting));

	
	/* For all wdev, find the maximum inter-set */

	
	if (hw_cfg->bbp_bw != new_cfg.bbp_bw)
	{
		bbp_set_bw(pAd, new_cfg.bbp_bw);
		hw_cfg->bbp_bw = new_cfg.bbp_bw;
	}

	if (hw_cfg->cent_ch != new_cfg.cent_ch)
	{
		UINT8 ext_ch = EXTCHA_NONE;
		
		bbp_set_ctrlch(pAd, ext_ch);

#if defined(RTMP_MAC) || defined(RLT_MAC)
		if (pAd->chipCap.hif_type == HIF_RTMP || pAd->chipCap.hif_type == HIF_RLT)
			rtmp_mac_set_ctrlch(pAd, ext_ch);
#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */

#ifdef MT_MAC
		if (pAd->chipCap.hif_type == HIF_MT)
			mt_mac_set_ctrlch(pAd, ext_ch);
#endif /* MT_MAC */
	}

	return TRUE;
}


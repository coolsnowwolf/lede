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

#include "rt_config.h"


struct wifi_dev *get_wdev_by_idx(RTMP_ADAPTER *pAd, INT idx)
{
	struct wifi_dev *wdev = NULL;
	
	do
	{


#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT	
		if (idx >= MIN_NET_DEVICE_FOR_APCLI)
		{				
			idx -= MIN_NET_DEVICE_FOR_APCLI;		
			if (idx < MAX_APCLI_NUM)
			{
				wdev = &pAd->ApCfg.ApCliTab[idx].wdev;
				break;
			}
		}
#endif /* APCLI_SUPPORT */

		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{

#ifdef WDS_SUPPORT
			if (idx >= MIN_NET_DEVICE_FOR_WDS)
			{				
				idx -= MIN_NET_DEVICE_FOR_WDS;		
				if (idx < MAX_WDS_ENTRY)
				{
					wdev = &&pAd->WdsTab.WdsEntry[idx].wdev;
					break;
				}
			}
#endif /* WDS_SUPPORT */
			if ((idx < pAd->ApCfg.BssidNum) && (idx < MAX_MBSSID_NUM(pAd)) && (idx < HW_BEACON_MAX_NUM))
				wdev = &pAd->ApCfg.MBSSID[idx].wdev;

			break;
		}			
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{
			wdev = &pAd->StaCfg.wdev;
			break;
		}	
#endif /* CONFIG_STA_SUPPORT */
	} while (FALSE);

	if (wdev == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("RTMPSetIndividualHT: invalid idx(%d)\n", idx));
	}

	return wdev;
}


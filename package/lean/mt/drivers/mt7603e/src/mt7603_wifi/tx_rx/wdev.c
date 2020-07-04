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


INT rtmp_wdev_idx_unreg(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	INT idx;
	ULONG flags;

	if (!wdev)
		return -1;
#ifdef WH_EZ_SETUP
	if ((wdev->wdev_type == WDEV_TYPE_AP) || (wdev->wdev_type == WDEV_TYPE_STA)) {
		if(IS_CONF_EZ_SETUP_ENABLED(wdev))
			ez_exit(wdev);
	}
#endif /* WH_EZ_SETUP */

	RTMP_INT_LOCK(&pAd->irq_lock, flags);
	for (idx = 0; idx < WDEV_NUM_MAX; idx++) {
		if (pAd->wdev_list[idx] == wdev) {
			DBGPRINT(RT_DEBUG_WARN, 
					("unregister wdev(type:%d, idx:%d) from wdev_list\n",
					wdev->wdev_type, wdev->wdev_idx));
			pAd->wdev_list[idx] = NULL;
			wdev->wdev_idx = WDEV_NUM_MAX;
			break;
		}
	}

	if (idx == WDEV_NUM_MAX) {
		DBGPRINT(RT_DEBUG_ERROR, 
					("Cannot found wdev(%p, type:%d, idx:%d) in wdev_list\n",
					wdev, wdev->wdev_type, wdev->wdev_idx));
		DBGPRINT(RT_DEBUG_OFF, ("Dump wdev_list:\n"));
		for (idx = 0; idx < WDEV_NUM_MAX; idx++) {
			DBGPRINT(RT_DEBUG_OFF, ("Idx %d: 0x%p\n", idx, pAd->wdev_list[idx]));
		}
	}
	RTMP_INT_UNLOCK(&pAd->irq_lock, flags);

	return ((idx < WDEV_NUM_MAX) ? 0 : -1);

}


INT rtmp_wdev_idx_reg(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	INT idx;
	ULONG flags;

	if (!wdev)
		return -1;

	RTMP_INT_LOCK(&pAd->irq_lock, flags);
	for (idx = 0; idx < WDEV_NUM_MAX; idx++) {
		if (pAd->wdev_list[idx] == wdev) {
			DBGPRINT(RT_DEBUG_WARN, 
					("wdev(type:%d) already registered and idx(%d) %smatch\n",
					wdev->wdev_type, wdev->wdev_idx, 
					((idx != wdev->wdev_idx) ? "mis" : "")));
			break;
		}
		if (pAd->wdev_list[idx] == NULL) {
			pAd->wdev_list[idx] = wdev;
			break;
		}
	}

	wdev->wdev_idx = idx;
	if (idx < WDEV_NUM_MAX) {
		DBGPRINT(RT_DEBUG_TRACE, ("Assign wdev_idx=%d\n", idx));
	}
	RTMP_INT_UNLOCK(&pAd->irq_lock, flags);
#ifdef MAC_REPEATER_SUPPORT
            RxTrackingInit(wdev);
#endif /* MAC_REPEATER_SUPPORT */
	return ((idx < WDEV_NUM_MAX) ? idx : -1);
}


//#ifdef RTMP_MAC_PCI
INT wdev_bcn_buf_init(RTMP_ADAPTER *pAd, BCN_BUF_STRUC *bcn_info)
{
	//bcn_info->bBcnSntReq = FALSE;
	bcn_info->BcnBufIdx = HW_BEACON_MAX_NUM;
	bcn_info->cap_ie_pos = 0;

	if (!bcn_info->BeaconPkt) {
		RTMPAllocateNdisPacket(pAd, &bcn_info->BeaconPkt, NULL, 0, NULL, MAX_BEACON_SIZE);
	}
	else {
		DBGPRINT(RT_DEBUG_OFF, ("%s():BcnPkt is allocated!\n", __FUNCTION__));		
	}
	
#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
	{
//		ASSERT(bcn_info->bcn_state == BCN_TX_IDLE);
		bcn_info->bcn_state = BCN_TX_IDLE;
	}
#endif /* MT_MAC */

	return TRUE;
}


INT wdev_bcn_buf_deinit(RTMP_ADAPTER *pAd, BCN_BUF_STRUC *bcn_info)
{
#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
	{
		if (bcn_info->bcn_state != BCN_TX_IDLE) {
			DBGPRINT(RT_DEBUG_ERROR, ("%s(): Bcn not in idle(%d) when try to free it!\n",
						__FUNCTION__, bcn_info->bcn_state));
			return FALSE;
		}
		bcn_info->bcn_state = BCN_TX_STOP;
	}
#endif /* MT_MAC */

	if (bcn_info->BeaconPkt) {
		RTMPFreeNdisPacket(pAd, bcn_info->BeaconPkt);
		bcn_info->BeaconPkt = NULL;
	}

	return TRUE;
}
//#endif /* RTMP_MAC_PCI */

INT wdev_init(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UINT wdev_type)
{
#ifdef CONFIG_AP_SUPPORT
	if (wdev_type == WDEV_TYPE_AP) {
		//BSS_STRUCT *pMbss;

		wdev->wdev_type = WDEV_TYPE_AP;

		wdev->tx_pkt_allowed = ApAllowToSendPacket;
		wdev->tx_pkt_handle = APSendPacket;
		wdev->wdev_hard_tx = APHardTransmit;
		
		wdev->rx_pkt_allowed = ap_rx_pkt_allow;
		wdev->rx_ps_handle = ap_rx_ps_handle;
		wdev->rx_pkt_foward = ap_rx_foward_handle;

		//pMbss = (BSS_STRUCT *)wdev->func_dev;
#ifdef WH_EZ_SETUP
		if (IS_CONF_EZ_SETUP_ENABLED(wdev))
			ez_init(pAd, wdev, TRUE);
#endif /* WH_EZ_SETUP */

		return TRUE;
	}
#endif /* CONFIG_AP_SUPPORT */


	return FALSE;
}

#ifdef WH_EZ_SETUP
/**
 * @param pAd
 * @param Address input address
 *
 * Search wifi_dev according to Address
 *
 * @return wifi_dev
 */
struct wifi_dev *WdevSearchByAddress(RTMP_ADAPTER *pAd, UCHAR *Address)
{
	UINT16 Index;
	struct wifi_dev *wdev;

	NdisAcquireSpinLock(&pAd->WdevListLock);
	for (Index = 0; Index < WDEV_NUM_MAX; Index++)
	{
		wdev = pAd->wdev_list[Index];

		if (wdev)
		{
			if (MAC_ADDR_EQUAL(Address, wdev->if_addr))
			{
				NdisReleaseSpinLock(&pAd->WdevListLock);
				return wdev;
			}
		}
	}
	NdisReleaseSpinLock(&pAd->WdevListLock);

	MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: can not find registered wdev\n",
								__FUNCTION__));

	return NULL;
}

#endif
/**
 * @param pAd
 * @param Address input address
 *
 * Search wifi_dev according to Address
 *
 * @return wifi_dev
 */
struct wifi_dev *wdev_search_by_address(RTMP_ADAPTER *pAd, UCHAR *address)
{
	UINT16 Index;
	struct wifi_dev *wdev;
#ifdef MAC_REPEATER_SUPPORT
	REPEATER_CLIENT_ENTRY *rept_entry = NULL;
#endif
	/*NdisAcquireSpinLock(&pAd->WdevListLock);*/

	for (Index = 0; Index < WDEV_NUM_MAX; Index++) {
		wdev = pAd->wdev_list[Index];

		if (wdev) {
			if (MAC_ADDR_EQUAL(address, wdev->if_addr)) {
				/*NdisReleaseSpinLock(&pAd->WdevListLock);*/
				return wdev;
			}
		}
	}

	/*NdisReleaseSpinLock(&pAd->WdevListLock);*/
#ifdef MAC_REPEATER_SUPPORT

	/* if we cannot found wdev from A2, it might comes from Rept entry.
	 * cause rept must bind the bssid of apcli_link,
	 * search A3(Bssid) to find the corresponding wdev.
	 */
	if (pAd->ApCfg.bMACRepeaterEn) {
		rept_entry = lookup_rept_entry(pAd, address);

		if (rept_entry)
			return rept_entry->wdev;
	}

#endif
	MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: can not find registered wdev\n",
			 __func__));
	return NULL;
}


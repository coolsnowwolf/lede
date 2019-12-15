/****************************************************************************
 * Ralink Tech Inc.
 * Taiwan, R.O.C.
 *
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************/

/****************************************************************************

    Abstract:

    Support multi-BSS function.

    Note:
    1. Call RT28xx_MBSS_Init() in init function and
       call RT28xx_MBSS_Remove() in close function

    2. MAC of different BSS is initialized in APStartUp()

    3. BSS Index (0 ~ 15) of different rx packet is got in

    4. BSS Index (0 ~ 15) of different tx packet is assigned in

    5. BSS Index (0 ~ 15) of different BSS is got in tx_pkt_handle() by using

    6. BSS Index (0 ~ 15) of IOCTL command is put in pAd->OS_Cookie->ioctl_if

    7. Beacon of different BSS is enabled in APMakeAllBssBeacon() by writing 1
       to the register MAC_BSSID_DW1

    8. The number of MBSS can be 1, 2, 4, or 8

***************************************************************************/
#ifdef MBSS_SUPPORT


#include "rt_config.h"

#ifdef VENDOR_FEATURE7_SUPPORT
#ifndef ARRIS_MODULE_PRESENT
void (*f)(int, int, int, char*, int) = arris_event_send_hook_fn;
#endif /* !ARRIS_MODULE_PRESENT */
#endif
#ifdef MULTI_PROFILE
INT	multi_profile_devname_req(struct _RTMP_ADAPTER *ad, UCHAR *final_name, UCHAR *ifidx);
#endif /*MULTI_PROFILE*/

extern struct wifi_dev_ops ap_wdev_ops;

/* --------------------------------- Public -------------------------------- */
/*
========================================================================
Routine Description:
    Initialize Multi-BSS function.

Arguments:
    pAd			points to our adapter
    pDevMain		points to the main BSS network interface

Return Value:
    None

Note:
	1. Only create and initialize virtual network interfaces.
	2. No main network interface here.
	3. If you down ra0 and modify the BssNum of RT2860AP.dat/RT2870AP.dat,
		it will not work! You must rmmod rt2860ap.ko and lsmod rt2860ap.ko again.
========================================================================
*/
VOID MBSS_Init(RTMP_ADAPTER *pAd, RTMP_OS_NETDEV_OP_HOOK *pNetDevOps)
{
#define MBSS_MAX_DEV_NUM	32
	PNET_DEV pDevNew;
	INT32 IdBss, MaxNumBss;
	INT status;
	RTMP_OS_NETDEV_OP_HOOK netDevHook;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	/* sanity check to avoid redundant virtual interfaces are created */
	if (pAd->FlgMbssInit != FALSE)
		return;

	MaxNumBss = pAd->ApCfg.BssidNum;

	if (MaxNumBss > HW_BEACON_MAX_NUM)
		MaxNumBss = HW_BEACON_MAX_NUM;

	/* first IdBss must not be 0 (BSS0), must be 1 (BSS1) */
	for (IdBss = FIRST_MBSSID; IdBss < HW_BEACON_MAX_NUM; IdBss++) {
		pAd->ApCfg.MBSSID[IdBss].wdev.if_dev = NULL;
		pAd->ApCfg.MBSSID[IdBss].wdev.bcn_buf.BeaconPkt = NULL;
	}

	/* create virtual network interface */
	for (IdBss = FIRST_MBSSID; IdBss < MaxNumBss; IdBss++) {
		struct wifi_dev *wdev;
		UINT32 MC_RowID = 0, IoctlIF = 0;
		char *dev_name = NULL;
		INT32 Ret;
		BSS_STRUCT *pMbss = NULL;
		UCHAR ifidx = IdBss;
		UCHAR final_name[32]="";
		BOOLEAN autoSuffix = TRUE;
#ifdef MULTIPLE_CARD_SUPPORT
		MC_RowID = pAd->MC_RowID;
#endif /* MULTIPLE_CARD_SUPPORT */
#ifdef HOSTAPD_SUPPORT
		IoctlIF = pAd->IoctlIF;
#endif /* HOSTAPD_SUPPORT */

		dev_name = get_dev_name_prefix(pAd, INT_MBSSID);

		if(dev_name == NULL){
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("get_dev_name_prefix error!\n"));
			break;
		}
		snprintf(final_name,sizeof(final_name),"%s",dev_name);
#ifdef MULTI_PROFILE
		multi_profile_devname_req(pAd,final_name,&ifidx);
		if (ifidx == 0)
			autoSuffix = FALSE;
#endif /*MULTI_PROFILE*/
		pDevNew = RtmpOSNetDevCreate(MC_RowID, &IoctlIF, INT_MBSSID, ifidx,
						 sizeof(struct mt_dev_priv), final_name, autoSuffix);
#ifdef HOSTAPD_SUPPORT
		pAd->IoctlIF = IoctlIF;
#endif /* HOSTAPD_SUPPORT */

		if (pDevNew == NULL) {
			pAd->ApCfg.BssidNum = IdBss; /* re-assign new MBSS number */
			break;
		}
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Register MBSSID IF (%s)\n", RTMP_OS_NETDEV_GET_DEVNAME(pDevNew)));

		pMbss = &pAd->ApCfg.MBSSID[IdBss];
		wdev = &pAd->ApCfg.MBSSID[IdBss].wdev;
		Ret = wdev_init(pAd, wdev, WDEV_TYPE_AP, pDevNew, IdBss,
						(VOID *)&pAd->ApCfg.MBSSID[IdBss], (void *)pAd);

		if (!Ret) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Assign wdev idx for %s failed, free net device!\n",
					 RTMP_OS_NETDEV_GET_DEVNAME(pDevNew)));
			RtmpOSNetDevFree(pDevNew);
			break;
		}

		Ret = wdev_ops_register(wdev, WDEV_TYPE_AP, &ap_wdev_ops,
								cap->wmm_detect_method);

		if (!Ret) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("register wdev_ops %s failed, free net device!\n",
					  RTMP_OS_NETDEV_GET_DEVNAME(pDevNew)));
			RtmpOSNetDevFree(pDevNew);
			break;
		}

		RTMP_OS_NETDEV_SET_PRIV(pDevNew, pAd);
		RTMP_OS_NETDEV_SET_WDEV(pDevNew, wdev);
		/* init operation functions and flags */
		NdisCopyMemory(&netDevHook, pNetDevOps, sizeof(netDevHook));
		netDevHook.priv_flags = INT_MBSSID;
		netDevHook.needProtcted = TRUE;
		netDevHook.wdev = wdev;
		/* Init MAC address of virtual network interface */
		NdisMoveMemory(&netDevHook.devAddr[0], &wdev->bssid[0], MAC_ADDR_LEN);
		/* register this device to OS */
		status = RtmpOSNetDevAttach(pAd->OpMode, pDevNew, &netDevHook);
	}

	pAd->FlgMbssInit = TRUE;
}


/*
========================================================================
Routine Description:
    Remove Multi-BSS network interface.

Arguments:
	pAd			points to our adapter

Return Value:
    None

Note:
    FIRST_MBSSID = 1
    Main BSS is not removed here.
========================================================================
*/
VOID MBSS_Remove(RTMP_ADAPTER *pAd)
{
	struct wifi_dev *wdev;
	UINT IdBss;
	BSS_STRUCT *pMbss;
	INT32 MaxNumBss;

	if (!pAd)
		return;

	MaxNumBss = pAd->ApCfg.BssidNum;

	if (MaxNumBss > HW_BEACON_MAX_NUM)
		MaxNumBss = HW_BEACON_MAX_NUM;


	for (IdBss = FIRST_MBSSID; IdBss < MaxNumBss; IdBss++) {
		wdev = &pAd->ApCfg.MBSSID[IdBss].wdev;
		pMbss = &pAd->ApCfg.MBSSID[IdBss];

		if (pMbss)
			bcn_buf_deinit(pAd, &wdev->bcn_buf);

		if (wdev->if_dev) {
			RtmpOSNetDevProtect(1);
			RtmpOSNetDevDetach(wdev->if_dev);
			RtmpOSNetDevProtect(0);
			wdev_deinit(pAd, wdev);
			RtmpOSNetDevFree(wdev->if_dev);
			wdev->if_dev = NULL;
		}
	}
}


/*
========================================================================
Routine Description:
    Get multiple bss idx.

Arguments:
	pAd				points to our adapter
	pDev			which WLAN network interface

Return Value:
    0: close successfully
    otherwise: close fail

Note:
========================================================================
*/
INT32 RT28xx_MBSS_IdxGet(RTMP_ADAPTER *pAd, PNET_DEV pDev)
{
	INT32 BssId = -1;
	INT32 IdBss;

	if (!pAd || !pDev)
		return -1;

	for (IdBss = 0; IdBss < pAd->ApCfg.BssidNum; IdBss++) {
		if (pAd->ApCfg.MBSSID[IdBss].wdev.if_dev == pDev) {
			BssId = IdBss;
			break;
		}
	}

	return BssId;
}

#ifdef MT_MAC
INT32 ext_mbss_hw_cr_enable(PNET_DEV pDev)
{
	PRTMP_ADAPTER pAd;
	INT BssId;

	pAd = RTMP_OS_NETDEV_GET_PRIV(pDev);
	BssId = RT28xx_MBSS_IdxGet(pAd, pDev);
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("##### %s, BssId = %d\n", __func__, BssId));

	if (BssId < 0)
		return -1;

	if (!IS_HIF_TYPE(pAd, HIF_MT))
		return 0;

	AsicSetExtMbssEnableCR(pAd, BssId, TRUE);/* enable rmac 0_1~0_15 bit */
	AsicSetMbssHwCRSetting(pAd, BssId, TRUE);/* enable lp timing setting for 0_1~0_15 */
	return 0;
}


INT ext_mbss_hw_cr_disable(PNET_DEV pDev)
{
	PRTMP_ADAPTER pAd;
	INT BssId;

	pAd = RTMP_OS_NETDEV_GET_PRIV(pDev);
	BssId = RT28xx_MBSS_IdxGet(pAd, pDev);

	if (BssId < 0)
		return -1;

	if (!IS_HIF_TYPE(pAd, HIF_MT))
		return 0;

	AsicSetMbssHwCRSetting(pAd, BssId, FALSE);
	AsicSetExtMbssEnableCR(pAd, BssId, FALSE);
	return 0;
}
#endif /* MT_MAC */

#endif /* MBSS_SUPPORT */




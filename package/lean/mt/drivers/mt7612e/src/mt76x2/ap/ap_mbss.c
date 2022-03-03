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
       APHandleRxDoneInterrupt() by using FromWhichBSSID = pEntry->apidx;
       Or FromWhichBSSID = BSS0;

    4. BSS Index (0 ~ 15) of different tx packet is assigned in
       rt28xx_send_packets() by using RTMP_SET_PACKET_NET_DEVICE_MBSSID()
    5. BSS Index (0 ~ 15) of different BSS is got in APHardTransmit() by using
       RTMP_GET_PACKET_IF()

    6. BSS Index (0 ~ 15) of IOCTL command is put in pAd->OS_Cookie->ioctl_if

    7. Beacon of different BSS is enabled in APMakeAllBssBeacon() by writing 1
       to the register MAC_BSSID_DW1

    8. The number of MBSS can be 1, 2, 4, or 8

***************************************************************************/
#ifdef MBSS_SUPPORT


#include "rt_config.h"


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
	PNET_DEV pDevNew;
	INT32 IdBss, MaxNumBss;
	INT status;
	RTMP_OS_NETDEV_OP_HOOK netDevHook;

	MaxNumBss = pAd->ApCfg.BssidNum;
	if (MaxNumBss > MAX_MBSSID_NUM(pAd))
		MaxNumBss = MAX_MBSSID_NUM(pAd);

	/* sanity check to avoid redundant virtual interfaces are created */
	if (!pAd->FlgMbssInit)
	{
	/* first IdBss must not be 0 (BSS0), must be 1 (BSS1) */
	for(IdBss=FIRST_MBSSID; IdBss<MAX_MBSSID_NUM(pAd); IdBss++)
		pAd->ApCfg.MBSSID[IdBss].wdev.if_dev = NULL;
	}

	/* create virtual network interface */
	for(IdBss=FIRST_MBSSID; IdBss<MaxNumBss; IdBss++)
	{
		struct wifi_dev *wdev;
		UINT32 MC_RowID = 0, IoctlIF = 0;
		char *dev_name;
#ifdef MULTIPLE_CARD_SUPPORT
		MC_RowID = pAd->MC_RowID;
#endif /* MULTIPLE_CARD_SUPPORT */
#ifdef HOSTAPD_SUPPORT
		IoctlIF = pAd->IoctlIF;
#endif /* HOSTAPD_SUPPORT */

		if (pAd->ApCfg.MBSSID[IdBss].wdev.if_dev)
			continue;

		dev_name = get_dev_name_prefix(pAd, INT_MBSSID);
		pDevNew = RtmpOSNetDevCreate(MC_RowID, &IoctlIF, INT_MBSSID, IdBss, sizeof(struct mt_dev_priv), dev_name);
#ifdef HOSTAPD_SUPPORT
		pAd->IoctlIF = IoctlIF;
#endif /* HOSTAPD_SUPPORT */
		if (pDevNew == NULL)
		{
			pAd->ApCfg.BssidNum = IdBss; /* re-assign new MBSS number */
			break;
		}
		else
		{
			DBGPRINT(RT_DEBUG_TRACE, ("Register MBSSID IF (%s)\n", RTMP_OS_NETDEV_GET_DEVNAME(pDevNew)));
		}

		wdev = &pAd->ApCfg.MBSSID[IdBss].wdev;
		wdev->wdev_type = WDEV_TYPE_AP;
		wdev->func_dev = &pAd->ApCfg.MBSSID[IdBss];
		wdev->sys_handle = (void *)pAd;
		wdev->if_dev = pDevNew;
		if (rtmp_wdev_idx_reg(pAd, wdev) < 0) {
			DBGPRINT(RT_DEBUG_ERROR, ("Assign wdev idx for %s failed, free net device!\n",
						RTMP_OS_NETDEV_GET_DEVNAME(pDevNew)));
			RtmpOSNetDevFree(pDevNew);
			break;
		}
		wdev->tx_pkt_allowed = ApAllowToSendPacket;
		
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
		if (status != NDIS_STATUS_SUCCESS)
			DBGPRINT(RT_DEBUG_ERROR, ("Error in RtmpOSNetDevAttach() !!\n"));
		
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



	for(IdBss=FIRST_MBSSID; IdBss<MAX_MBSSID_NUM(pAd); IdBss++)
	{
		wdev = &pAd->ApCfg.MBSSID[IdBss].wdev;
		if (wdev->if_dev)
		{
			RtmpOSNetDevProtect(1);
			RtmpOSNetDevDetach(wdev->if_dev);
			RtmpOSNetDevProtect(0);
			rtmp_wdev_idx_unreg(pAd, wdev);
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


	for(IdBss=0; IdBss<pAd->ApCfg.BssidNum; IdBss++)
	{
		if (pAd->ApCfg.MBSSID[IdBss].wdev.if_dev == pDev)
		{
			BssId = IdBss;
			break;
		}
	}

	return BssId;
}


/*
========================================================================
Routine Description:
    Open a virtual network interface.

Arguments:
	pDev			which WLAN network interface

Return Value:
    0: open successfully
    otherwise: open fail

Note:
========================================================================
*/
INT MBSS_Open(PNET_DEV pDev)
{
	PRTMP_ADAPTER pAd;
	INT BssId;

	pAd = RTMP_OS_NETDEV_GET_PRIV(pDev);
	BssId = RT28xx_MBSS_IdxGet(pAd, pDev);
	if (BssId < 0)
		return -1;
    
#ifdef AIRPLAY_SUPPORT
	if (AIRPLAY_ON(pAd))
		pAd->ApCfg.MBSSID[BssId].bBcnSntReq = TRUE;
	else
		pAd->ApCfg.MBSSID[BssId].bBcnSntReq = FALSE;	
#else
	pAd->ApCfg.MBSSID[BssId].bBcnSntReq = TRUE;
#endif /* AIRPLAY_SUPPORT */

		return 0;
}


/*
========================================================================
Routine Description:
    Close a virtual network interface.

Arguments:
    pDev           which WLAN network interface

Return Value:
    0: close successfully
    otherwise: close fail

Note:
========================================================================
*/
INT MBSS_Close(PNET_DEV pDev)
{
	PRTMP_ADAPTER pAd;
	INT BssId;


	pAd = RTMP_OS_NETDEV_GET_PRIV(pDev);
	BssId = RT28xx_MBSS_IdxGet(pAd, pDev);
    if (BssId < 0)
        return -1;

	RTMP_OS_NETDEV_STOP_QUEUE(pDev);

	/* kick out all stas behind the Bss */
	MbssKickOutStas(pAd, BssId, REASON_DISASSOC_INACTIVE);

	pAd->ApCfg.MBSSID[BssId].bBcnSntReq = FALSE;

	APMakeAllBssBeacon(pAd);
	APUpdateAllBeaconFrame(pAd);

	return 0;
}

#endif /* MBSS_SUPPORT */


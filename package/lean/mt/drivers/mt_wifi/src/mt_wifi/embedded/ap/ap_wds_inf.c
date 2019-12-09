
/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

    Module Name:
    ap_wds.c

    Abstract:
    Support WDS function.

    Revision History:
    Who       When            What
    ------    ----------      ----------------------------------------------
    Fonchi    02-13-2007      created
*/
#define RTMP_MODULE_OS

#ifdef WDS_SUPPORT

/*#include "rt_config.h" */
#include "rtmp_comm.h"
#include "rt_os_util.h"
#include "rt_os_net.h"


NET_DEV_STATS *RT28xx_get_wds_ether_stats(PNET_DEV net_dev);


/* Register WDS interface */
VOID RT28xx_WDS_Init(VOID *pAd, PNET_DEV net_dev)
{
	RTMP_OS_NETDEV_OP_HOOK netDevOpHook;

	NdisZeroMemory((PUCHAR)&netDevOpHook, sizeof(RTMP_OS_NETDEV_OP_HOOK));
	netDevOpHook.open = wds_virtual_if_open;
	netDevOpHook.stop = wds_virtual_if_close;
	netDevOpHook.xmit = rt28xx_send_packets;
	netDevOpHook.ioctl = rt28xx_ioctl;
	netDevOpHook.get_stats = RT28xx_get_wds_ether_stats;
	NdisMoveMemory(&netDevOpHook.devAddr[0], RTMP_OS_NETDEV_GET_PHYADDR(net_dev), MAC_ADDR_LEN);
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("The new WDS interface MAC = %02X:%02X:%02X:%02X:%02X:%02X\n",
			 PRINT_MAC(netDevOpHook.devAddr)));
	RTMP_AP_IoctlHandle(pAd, NULL, CMD_RTPRIV_IOCTL_WDS_INIT,
						0, &netDevOpHook, 0);
}


INT wds_virtual_if_open(PNET_DEV pDev)
{
	VOID *pAd;

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: ===> %s\n",
		RTMP_OS_NETDEV_GET_DEVNAME(pDev), __func__));

	pAd = RTMP_OS_NETDEV_GET_PRIV(pDev);

	if (VIRTUAL_IF_INIT(pAd, pDev) != 0)
		return -1;

	if (VIRTUAL_IF_UP(pAd, pDev) != 0)
		return -1;

	/* increase MODULE use count */
	RT_MOD_INC_USE_COUNT();
	RT_MOD_HNAT_REG(pDev);
	RTMP_OS_NETDEV_START_QUEUE(pDev);
	return 0;
}


INT wds_virtual_if_close(PNET_DEV pDev)
{
	VOID *pAd;

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: ===> %s\n",
		RTMP_OS_NETDEV_GET_DEVNAME(pDev), __func__));

	pAd = RTMP_OS_NETDEV_GET_PRIV(pDev);
	/* RTMP_OS_NETDEV_CARRIER_OFF(pDev); */
	RTMP_OS_NETDEV_STOP_QUEUE(pDev);

	VIRTUAL_IF_DOWN(pAd, pDev);

	VIRTUAL_IF_DEINIT(pAd, pDev);

	RT_MOD_HNAT_DEREG(pDev);
	RT_MOD_DEC_USE_COUNT();
	return 0;
}


VOID RT28xx_WDS_Remove(VOID *pAd)
{
	RTMP_AP_IoctlHandle(pAd, NULL, CMD_RTPRIV_IOCTL_WDS_REMOVE, 0, NULL, 0);
}

#endif /* WDS_SUPPORT */

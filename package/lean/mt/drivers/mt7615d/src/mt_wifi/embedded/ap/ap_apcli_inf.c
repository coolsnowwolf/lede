/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2006, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

    Module Name:
    ap_apcli.c

    Abstract:
    Support AP-Client function.

    Note:
    1. Call RT28xx_ApCli_Init() in init function and
       call RT28xx_ApCli_Remove() in close function

    2. MAC of ApCli-interface is initialized in RT28xx_ApCli_Init()

    3. ApCli index (0) of different rx packet is got in

    4. ApCli index (0) of different tx packet is assigned in

    5. ApCli index (0) of different interface is got in tx_pkt_handle() by using

    6. ApCli index (0) of IOCTL command is put in pAd->OS_Cookie->ioctl_if

    8. The number of ApCli only can be 1

	9. apcli convert engine subroutines, we should just take care data packet.
    Revision History:
    Who             When            What
    --------------  ----------      ----------------------------------------------
    Shiang, Fonchi  02-13-2007      created
*/
#define RTMP_MODULE_OS

#ifdef APCLI_SUPPORT

/*#include "rt_config.h" */
#include "rtmp_comm.h"
#include "rt_os_util.h"
#include "rt_os_net.h"

/* --------------------------------- Public -------------------------------- */
NET_DEV_STATS *RT28xx_get_ether_stats(PNET_DEV net_dev);
/*
========================================================================
Routine Description:
    Init AP-Client function.

Arguments:
    pAd            points to our adapter
    main_dev_p      points to the main BSS network interface

Return Value:
    None

Note:
	1. Only create and initialize virtual network interfaces.
	2. No main network interface here.
========================================================================
*/
VOID RT28xx_ApCli_Init(VOID *pAd, PNET_DEV main_dev_p)
{
	RTMP_OS_NETDEV_OP_HOOK netDevOpHook;
	/* init operation functions */
	NdisZeroMemory(&netDevOpHook, sizeof(RTMP_OS_NETDEV_OP_HOOK));
	netDevOpHook.open = apcli_virtual_if_open;
	netDevOpHook.stop = apcli_virtual_if_close;
	netDevOpHook.xmit = rt28xx_send_packets;
	netDevOpHook.ioctl = rt28xx_ioctl;
	netDevOpHook.get_stats = RT28xx_get_ether_stats;
	RTMP_AP_IoctlHandle(pAd, NULL, CMD_RTPRIV_IOCTL_APC_INIT,
						0, &netDevOpHook, 0);
}

/*
========================================================================
Routine Description:
    Open a virtual network interface.

Arguments:
    dev_p           which WLAN network interface

Return Value:
    0: open successfully
    otherwise: open fail

Note:
========================================================================
*/
INT apcli_virtual_if_open(PNET_DEV pDev)
{
	VOID *pAd;

	pAd = RTMP_OS_NETDEV_GET_PRIV(pDev);
	ASSERT(pAd);
	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_OFF, ("%s: ===> %s\n",
		RTMP_OS_NETDEV_GET_DEVNAME(pDev), __func__));

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


/*
========================================================================
Routine Description:
    Close a virtual network interface.

Arguments:
    dev_p           which WLAN network interface

Return Value:
    0: close successfully
    otherwise: close fail

Note:
========================================================================
*/
INT apcli_virtual_if_close(PNET_DEV pDev)
{
	VOID *pAd;

	pAd = RTMP_OS_NETDEV_GET_PRIV(pDev);
	ASSERT(pAd);
	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_OFF, ("%s: ===> %s\n",
		RTMP_OS_NETDEV_GET_DEVNAME(pDev), __func__));

	VIRTUAL_IF_DOWN(pAd, pDev);

	VIRTUAL_IF_DEINIT(pAd, pDev);

	RT_MOD_HNAT_DEREG(pDev);
	RT_MOD_DEC_USE_COUNT();
	return 0;
}


/*
========================================================================
Routine Description:
    Remove ApCli-BSS network interface.

Arguments:
    pAd            points to our adapter

Return Value:
    None

Note:
========================================================================
*/
VOID RT28xx_ApCli_Remove(VOID *pAd)
{
	RTMP_AP_IoctlHandle(pAd, NULL, CMD_RTPRIV_IOCTL_APC_REMOVE, 0, NULL, 0);
}

#endif /* APCLI_SUPPORT */


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
    rt_main_dev.c

    Abstract:
    Create and register network interface.

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
*/


#define RTMP_MODULE_OS

/*#include "rt_config.h" */
#include "rtmp_comm.h"
#include "rt_os_util.h"
#include "rt_os_net.h"

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,24)
#ifndef SA_SHIRQ
#define SA_SHIRQ IRQF_SHARED
#endif
#endif

// TODO: shiang-6590, remove it when MP
#ifdef RTMP_MAC_PCI
MODULE_LICENSE("GPL");
#endif /* RTMP_MAC_PCI */
// TODO: End---


#ifdef BTCOEX_CONCURRENT
int CoexChannel;
int CoexCenctralChannel;
int CoexChannelBw;
#endif

/*---------------------------------------------------------------------*/
/* Private Variables Used                                              */
/*---------------------------------------------------------------------*/

PSTRING mac = "";		/* default 00:00:00:00:00:00 */
PSTRING mode = "";		/* supported mode: normal/ate/monitor;  default: normal */
PSTRING hostname = "";	/* default CMPC */

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,12)
MODULE_PARM (mac, "s");
MODULE_PARM(mode, "s");
#else
module_param (mac, charp, 0);
module_param(mode, charp, 0);
#endif
MODULE_PARM_DESC (mac, "rt_wifi: wireless mac addr");
MODULE_PARM_DESC (mode, "rt_wifi: wireless operation mode");

#ifdef OS_ABL_SUPPORT
RTMP_DRV_ABL_OPS RtmpDrvOps, *pRtmpDrvOps = &RtmpDrvOps;
RTMP_NET_ABL_OPS RtmpDrvNetOps, *pRtmpDrvNetOps = &RtmpDrvNetOps;
#endif /* OS_ABL_SUPPORT */


/*---------------------------------------------------------------------*/
/* Prototypes of Functions Used                                        */
/*---------------------------------------------------------------------*/

/* public function prototype */
int rt28xx_close(VOID *net_dev);
int rt28xx_open(VOID *net_dev);

/* private function prototype */
INT rt28xx_send_packets(IN struct sk_buff *skb_p, IN struct net_device *net_dev);

struct rtnl_link_stats64 *
RT28xx_get_ether_stats64(PNET_DEV net_dev, struct rtnl_link_stats64 *stats);


/*
========================================================================
Routine Description:
    Close raxx interface.

Arguments:
	*net_dev			the raxx interface pointer

Return Value:
    0					Open OK
	otherwise			Open Fail

Note:
	1. if open fail, kernel will not call the close function.
	2. Free memory for
		(1) Mlme Memory Handler:		MlmeHalt()
		(2) TX & RX:					RTMPFreeTxRxRingMemory()
		(3) BA Reordering: 				ba_reordering_resource_release()
========================================================================
*/
int MainVirtualIF_close(IN struct net_device *net_dev)
{
    VOID *pAd = NULL;

	pAd = RTMP_OS_NETDEV_GET_PRIV(net_dev);

	if (pAd == NULL)
		return 0;

	netif_carrier_off(net_dev);
	netif_stop_queue(net_dev);

	RTMPInfClose(pAd);

#ifdef IFUP_IN_PROBE
#else
	VIRTUAL_IF_DOWN(pAd);
#endif /* IFUP_IN_PROBE */

	RT_MOD_DEC_USE_COUNT();

	return 0; /* close ok */
}

/*
========================================================================
Routine Description:
    Open raxx interface.

Arguments:
	*net_dev			the raxx interface pointer

Return Value:
    0					Open OK
	otherwise			Open Fail

Note:
	1. if open fail, kernel will not call the close function.
	2. Free memory for
		(1) Mlme Memory Handler:		MlmeHalt()
		(2) TX & RX:					RTMPFreeTxRxRingMemory()
		(3) BA Reordering: 				ba_reordering_resource_release()
========================================================================
*/
int MainVirtualIF_open(struct net_device *net_dev)
{
	VOID *pAd = NULL;

	GET_PAD_FROM_NET_DEV(pAd, net_dev);

	if (pAd == NULL)
		return 0;

#ifdef CONFIG_AP_SUPPORT
	/* pAd->ApCfg.MBSSID[MAIN_MBSSID].bBcnSntReq = TRUE; */
	RTMP_DRIVER_AP_MAIN_OPEN(pAd);
#endif /* CONFIG_AP_SUPPORT */

#ifdef IFUP_IN_PROBE	
	while (RTMP_DRIVER_IOCTL_SANITY_CHECK(pAd, NULL) != NDIS_STATUS_SUCCESS)
	{
		OS_WAIT(10);
		DBGPRINT(RT_DEBUG_TRACE, ("Card not ready, NDIS_STATUS_SUCCESS!\n"));
	}
#else
	if (VIRTUAL_IF_UP(pAd) != 0)
		return -1;
#endif /* IFUP_IN_PROBE */	

	RT_MOD_INC_USE_COUNT();

	netif_start_queue(net_dev);
	netif_carrier_on(net_dev);
	netif_wake_queue(net_dev);

	return 0;
}


/*
========================================================================
Routine Description:
    Close raxx interface.

Arguments:
	*net_dev			the raxx interface pointer

Return Value:
    0					Open OK
	otherwise			Open Fail

Note:
	1. if open fail, kernel will not call the close function.
	2. Free memory for
		(1) Mlme Memory Handler:		MlmeHalt()
		(2) TX & RX:					RTMPFreeTxRxRingMemory()
		(3) BA Reordering: 				ba_reordering_resource_release()
========================================================================
*/
int rt28xx_close(VOID *dev)
{
	struct net_device * net_dev = (struct net_device *)dev;
    VOID	*pAd = NULL;

	GET_PAD_FROM_NET_DEV(pAd, net_dev);	

	DBGPRINT(RT_DEBUG_TRACE, ("===> rt28xx_close\n"));

	if (pAd == NULL)
		return 0;

	RTMPDrvClose(pAd, net_dev);


	DBGPRINT(RT_DEBUG_TRACE, ("<=== rt28xx_close\n"));
	return 0;
}


/*
========================================================================
Routine Description:
    Open raxx interface.

Arguments:
	*net_dev			the raxx interface pointer

Return Value:
    0					Open OK
	otherwise			Open Fail

Note:
========================================================================
*/
int rt28xx_open(VOID *dev)
{				 
	struct net_device * net_dev = (struct net_device *)dev;
	VOID *pAd = NULL;
	int retval = 0;
	ULONG OpMode;

#ifdef CONFIG_STA_SUPPORT
#ifdef CONFIG_PM
#ifdef USB_SUPPORT_SELECTIVE_SUSPEND
	struct usb_interface *intf;
	struct usb_device *pUsb_Dev;
	INT pm_usage_cnt;
#endif /* USB_SUPPORT_SELECTIVE_SUSPEND */
#endif /* CONFIG_PM */
#endif /* CONFIG_STA_SUPPORT */


	if (sizeof(ra_dma_addr_t) < sizeof(dma_addr_t))
		DBGPRINT(RT_DEBUG_ERROR, ("Fatal error for DMA address size!!!\n"));

	GET_PAD_FROM_NET_DEV(pAd, net_dev);
	if (pAd == NULL)
	{
		/* if 1st open fail, pAd will be free;
		   So the net_dev->priv will be NULL in 2rd open */
		return -1;
	}

	RTMP_DRIVER_MCU_SLEEP_CLEAR(pAd);

	RTMP_DRIVER_OP_MODE_GET(pAd, &OpMode);

#ifdef CONFIG_STA_SUPPORT
#ifdef CONFIG_PM
#ifdef USB_SUPPORT_SELECTIVE_SUSPEND

	RTMP_DRIVER_USB_DEV_GET(pAd, &pUsb_Dev);
	RTMP_DRIVER_USB_INTF_GET(pAd, &intf);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,32)
	pm_usage_cnt = atomic_read(&intf->pm_usage_cnt);	
#else
	pm_usage_cnt = intf->pm_usage_cnt;
#endif
	if (pm_usage_cnt == 0)
	{
		int res=1;

		res = usb_autopm_get_interface(intf);
		if (res)
		{
			DBGPRINT(RT_DEBUG_ERROR, ("rt28xx_open autopm_resume fail!\n"));
			return (-1);;
		}			
	}
#endif /* USB_SUPPORT_SELECTIVE_SUSPEND */
#endif /* CONFIG_PM */
#endif /* CONFIG_STA_SUPPORT */


#if WIRELESS_EXT >= 12
/*	if (RT_DEV_PRIV_FLAGS_GET(net_dev) == INT_MAIN) */
	if (RTMP_DRIVER_MAIN_INF_CHECK(pAd, RT_DEV_PRIV_FLAGS_GET(net_dev)) == NDIS_STATUS_SUCCESS)
	{
#ifdef CONFIG_APSTA_MIXED_SUPPORT
		if (OpMode == OPMODE_AP)
			net_dev->wireless_handlers = (struct iw_handler_def *) &rt28xx_ap_iw_handler_def;
#endif /* CONFIG_APSTA_MIXED_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		if (OpMode == OPMODE_STA)
			net_dev->wireless_handlers = (struct iw_handler_def *) &rt28xx_iw_handler_def;
#endif /* CONFIG_STA_SUPPORT */
	}
#endif /* WIRELESS_EXT >= 12 */

	/*
		Request interrupt service routine for PCI device
		register the interrupt routine with the os
		
		AP Channel auto-selection will be run in rt28xx_init(),
		so we must reqister IRQ hander here.
	*/
	RtmpOSIRQRequest(net_dev);

	/* Init IRQ parameters stored in pAd */
/*	rtmp_irq_init(pAd); */
	RTMP_DRIVER_IRQ_INIT(pAd);

	/* Chip & other init */
	if (rt28xx_init(pAd, mac, hostname) == FALSE)
		goto err;

#ifdef MBSS_SUPPORT
	/*
		the function can not be moved to RT2860_probe() even register_netdev()
	   is changed as register_netdevice().
		Or in some PC, kernel will panic (Fedora 4)
	*/
#if defined(P2P_APCLI_SUPPORT) || defined(RT_CFG80211_P2P_SUPPORT)
	
#else
	RT28xx_MBSS_Init(pAd, net_dev);
#endif /* P2P_APCLI_SUPPORT */
#endif /* MBSS_SUPPORT */

#ifdef WDS_SUPPORT
	RT28xx_WDS_Init(pAd, net_dev);
#endif /* WDS_SUPPORT */

#ifdef APCLI_SUPPORT
#if defined(P2P_APCLI_SUPPORT) || defined(RT_CFG80211_P2P_CONCURRENT_DEVICE)
#else
	RT28xx_ApCli_Init(pAd, net_dev);
#endif /* P2P_APCLI_SUPPORT || RT_CFG80211_P2P_CONCURRENT_DEVICE */
#endif /* APCLI_SUPPORT */

#ifdef CONFIG_SNIFFER_SUPPORT
	RT28xx_Monitor_Init(pAd, net_dev);
#endif /* CONFIG_SNIFFER_SUPPORT */



#ifdef RT_CFG80211_SUPPORT
#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE
	RTMP_CFG80211_DummyP2pIf_Init(pAd);
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE */	
#else
#endif /* RT_CFG80211_SUPPORT */

#ifdef LINUX
#ifdef RT_CFG80211_SUPPORT
	RTMP_DRIVER_CFG80211_START(pAd);
#endif /* RT_CFG80211_SUPPORT */
#endif /* LINUX */

	RTMPDrvOpen(pAd);

#ifdef VENDOR_FEATURE2_SUPPORT
	printk("Number of Packet Allocated in open = %lu\n", OS_NumOfPktAlloc);
	printk("Number of Packet Freed in open = %lu\n", OS_NumOfPktFree);
#endif /* VENDOR_FEATURE2_SUPPORT */

#ifdef BTCOEX_CONCURRENT
	CoexChannel =-1;
	CoexCenctralChannel = -1;
	CoexChannelBw = -1;
#endif

	return (retval);

err:
	RTMP_DRIVER_IRQ_RELEASE(pAd);

	return (-1);
}


PNET_DEV RtmpPhyNetDevInit(VOID *pAd, RTMP_OS_NETDEV_OP_HOOK *pNetDevHook)
{
	struct net_device *net_dev = NULL;
	ULONG InfId, OpMode;


	RTMP_DRIVER_MAIN_INF_GET(pAd, &InfId);

/*	net_dev = RtmpOSNetDevCreate(pAd, INT_MAIN, 0, sizeof(struct mt_dev_priv), INF_MAIN_DEV_NAME); */
	RTMP_DRIVER_MAIN_INF_CREATE(pAd, &net_dev);
	if (net_dev == NULL)
	{
		printk("%s(): main physical net device creation failed!\n", __FUNCTION__);
		return NULL;
	}

	NdisZeroMemory((unsigned char *)pNetDevHook, sizeof(RTMP_OS_NETDEV_OP_HOOK));
	pNetDevHook->open = MainVirtualIF_open;
	pNetDevHook->stop = MainVirtualIF_close;
	pNetDevHook->xmit = rt28xx_send_packets;
#ifdef IKANOS_VX_1X0
	pNetDevHook->xmit = IKANOS_DataFramesTx;
#endif /* IKANOS_VX_1X0 */
	pNetDevHook->ioctl = rt28xx_ioctl;
	pNetDevHook->priv_flags = InfId; /*INT_MAIN; */
	pNetDevHook->get_stats = RT28xx_get_ether_stats64;

	pNetDevHook->needProtcted = FALSE;

#if (WIRELESS_EXT < 21) && (WIRELESS_EXT >= 12)
	pNetDevHook->get_wstats = rt28xx_get_wireless_stats;
#endif

	RTMP_DRIVER_OP_MODE_GET(pAd, &OpMode);

	/* put private data structure */
	RTMP_OS_NETDEV_SET_PRIV(net_dev, pAd);

#ifdef CONFIG_STA_SUPPORT
#if WIRELESS_EXT >= 12
	if (OpMode == OPMODE_STA)
	{
		pNetDevHook->iw_handler = (void *)&rt28xx_iw_handler_def;
	}
#endif /*WIRELESS_EXT >= 12 */
#endif /* CONFIG_STA_SUPPORT */

#ifdef CONFIG_APSTA_MIXED_SUPPORT
#if WIRELESS_EXT >= 12
	if (OpMode == OPMODE_AP)
	{
		pNetDevHook->iw_handler = &rt28xx_ap_iw_handler_def;
	}
#endif /*WIRELESS_EXT >= 12 */
#endif /* CONFIG_APSTA_MIXED_SUPPORT */

	/* double-check if pAd is associated with the net_dev */
	if (RTMP_OS_NETDEV_GET_PRIV(net_dev) == NULL)
	{
		RtmpOSNetDevFree(net_dev);
		return NULL;
	}

	RTMP_DRIVER_NET_DEV_SET(pAd, net_dev);

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,24)
	SET_MODULE_OWNER(net_dev);
#endif 



	return net_dev;
}


VOID *RtmpNetEthConvertDevSearch(VOID *net_dev_, UCHAR *pData)
{
	struct net_device *pNetDev;


#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,24)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26)
	struct net_device *net_dev = (struct net_device *)net_dev_;
	struct net *net;
	net = dev_net(net_dev);
	
	BUG_ON(!net);
	for_each_netdev(net, pNetDev)
#else
	struct net *net;

	struct net_device *net_dev = (struct net_device *)net_dev_;
	BUG_ON(!net_dev->nd_net);
	net = net_dev->nd_net;
	for_each_netdev(net, pNetDev)
#endif
#else
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,22)
		for_each_netdev(pNetDev)
#else 
	for (pNetDev = dev_base; pNetDev; pNetDev = pNetDev->next)
#endif
#endif
	{
		if ((pNetDev->type == ARPHRD_ETHER)
			&& NdisEqualMemory(pNetDev->dev_addr, &pData[6], pNetDev->addr_len))
			break;
	}

	return (VOID *)pNetDev;
}




/*
========================================================================
Routine Description:
    The entry point for Linux kernel sent packet to our driver.

Arguments:
    sk_buff *skb 	the pointer refer to a sk_buffer.

Return Value:
    0	

Note:
	This function is the entry point of Tx Path for OS delivery packet to 
	our driver. You only can put OS-depened & STA/AP common handle procedures 
	in here.
========================================================================
*/
int rt28xx_packet_xmit(void *pkt)
{
	struct sk_buff *skb = (struct sk_buff *)pkt;
	struct net_device *net_dev = skb->dev;
	struct wifi_dev *wdev;
	PNDIS_PACKET pPacket = (PNDIS_PACKET)skb;

	wdev = RTMP_OS_NETDEV_GET_WDEV(net_dev);
	ASSERT(wdev);


	return RTMPSendPackets((NDIS_HANDLE)wdev, (PPNDIS_PACKET) &pPacket, 1,
							skb->len, RtmpNetEthConvertDevSearch);
}


/*
========================================================================
Routine Description:
    Send a packet to WLAN.

Arguments:
    skb_p           points to our adapter
    dev_p           which WLAN network interface

Return Value:
    0: transmit successfully
    otherwise: transmit fail

Note:
========================================================================
*/
int rt28xx_send_packets(struct sk_buff *skb, struct net_device *ndev)
{
	if (!(RTMP_OS_NETDEV_STATE_RUNNING(ndev)))
	{
		RELEASE_NDIS_PACKET(NULL, (PNDIS_PACKET)skb, NDIS_STATUS_FAILURE);
		return 0;
	}

	NdisZeroMemory((PUCHAR)&skb->cb[CB_OFF], 26);
	MEM_DBG_PKT_ALLOC_INC(skb);

	return rt28xx_packet_xmit(skb);
}


#if WIRELESS_EXT >= 12
/* This function will be called when query /proc */
struct iw_statistics *rt28xx_get_wireless_stats(struct net_device *net_dev)
{
	VOID *pAd = NULL;
	struct iw_statistics *pStats;
	RT_CMD_IW_STATS DrvIwStats, *pDrvIwStats = &DrvIwStats;


	GET_PAD_FROM_NET_DEV(pAd, net_dev);	


	DBGPRINT(RT_DEBUG_TRACE, ("rt28xx_get_wireless_stats --->\n"));

	pDrvIwStats->priv_flags = RT_DEV_PRIV_FLAGS_GET(net_dev);
	pDrvIwStats->dev_addr = (PUCHAR)net_dev->dev_addr;

	if (RTMP_DRIVER_IW_STATS_GET(pAd, pDrvIwStats) != NDIS_STATUS_SUCCESS)
		return NULL;

	pStats = (struct iw_statistics *)(pDrvIwStats->pStats);
	pStats->status = 0; /* Status - device dependent for now */


	pStats->qual.updated = 1;     /* Flags to know if updated */
#ifdef IW_QUAL_DBM
	pStats->qual.updated |= IW_QUAL_DBM;	/* Level + Noise are dBm */
#endif /* IW_QUAL_DBM */
	pStats->qual.qual = pDrvIwStats->qual;
	pStats->qual.level = pDrvIwStats->level;
	pStats->qual.noise = pDrvIwStats->noise;
	pStats->discard.nwid = 0;     /* Rx : Wrong nwid/essid */
	pStats->miss.beacon = 0;      /* Missed beacons/superframe */
	
	DBGPRINT(RT_DEBUG_TRACE, ("<--- rt28xx_get_wireless_stats\n"));
	return pStats;
}
#endif /* WIRELESS_EXT */


INT rt28xx_ioctl(PNET_DEV net_dev, struct ifreq *rq, INT cmd)
{
	VOID *pAd = NULL;
	INT ret = 0;
	ULONG OpMode;

	GET_PAD_FROM_NET_DEV(pAd, net_dev);	
	if (pAd == NULL)
	{
		/* if 1st open fail, pAd will be free;
		   So the net_dev->priv will be NULL in 2rd open */
		return -ENETDOWN;
	}

	RTMP_DRIVER_OP_MODE_GET(pAd, &OpMode);

#ifdef CONFIG_AP_SUPPORT
/*	IF_DEV_CONFIG_OPMODE_ON_AP(pAd) */
	RT_CONFIG_IF_OPMODE_ON_AP(OpMode)
	{
		ret = rt28xx_ap_ioctl(net_dev, rq, cmd);
	}
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
/*	IF_DEV_CONFIG_OPMODE_ON_STA(pAd) */
	RT_CONFIG_IF_OPMODE_ON_STA(OpMode)
	{
		ret = rt28xx_sta_ioctl(net_dev, rq, cmd);
	}
#endif /* CONFIG_STA_SUPPORT */

	return ret;
}


/*
    ========================================================================

    Routine Description:
        return ethernet statistics counter

    Arguments:
        net_dev                     Pointer to net_device

    Return Value:
        rtnl_link_stats64*

    Note:

    ========================================================================
*/
struct rtnl_link_stats64 *
RT28xx_get_ether_stats64(PNET_DEV net_dev, struct rtnl_link_stats64 *stats)
{
	RT_CMD_STATS64 WifStats;
    VOID *pAd = NULL;

	if (net_dev)
		GET_PAD_FROM_NET_DEV(pAd, net_dev);	

	if (!pAd)
		return NULL;

	WifStats.pNetDev = net_dev;
	RTMP_DRIVER_INF_STATS_GET(pAd, &WifStats);

	stats->rx_packets		= WifStats.rx_packets;
	stats->tx_packets		= WifStats.tx_packets;
	stats->rx_bytes			= WifStats.rx_bytes;
	stats->tx_bytes			= WifStats.tx_bytes;
	stats->rx_errors		= WifStats.rx_errors;
	stats->tx_errors		= WifStats.tx_errors;
	stats->rx_dropped		= 0;
	stats->tx_dropped		= 0;
	stats->multicast		= WifStats.multicast;
	stats->collisions		= WifStats.collisions;

	stats->rx_length_errors		= 0;
	stats->rx_over_errors		= WifStats.rx_over_errors;
	stats->rx_crc_errors		= 0;
	stats->rx_frame_errors		= WifStats.rx_frame_errors;
	stats->rx_fifo_errors		= WifStats.rx_fifo_errors;
	stats->rx_missed_errors		= 0;

	stats->tx_aborted_errors	= 0;
	stats->tx_carrier_errors	= 0;
	stats->tx_fifo_errors		= 0;
	stats->tx_heartbeat_errors	= 0;
	stats->tx_window_errors		= 0;

	stats->rx_compressed		= 0;
	stats->tx_compressed		= 0;
		
	return stats;
}


BOOLEAN RtmpPhyNetDevExit(VOID *pAd, PNET_DEV net_dev)
{
#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
#if defined(P2P_APCLI_SUPPORT) || defined(RT_CFG80211_P2P_CONCURRENT_DEVICE)
#else
	/* remove all AP-client virtual interfaces. */
	RT28xx_ApCli_Remove(pAd);
#endif /* P2P_APCLI_SUPPORT  || RT_CFG80211_P2P_CONCURRENT_DEVICE */
#endif /* APCLI_SUPPORT */

#ifdef WDS_SUPPORT
	/* remove all WDS virtual interfaces. */
	RT28xx_WDS_Remove(pAd);
#endif /* WDS_SUPPORT */

#ifdef MBSS_SUPPORT
#if defined(P2P_APCLI_SUPPORT) || defined(RT_CFG80211_P2P_SUPPORT)

#else
	RT28xx_MBSS_Remove(pAd);
#endif /* P2P_APCLI_SUPPORT */
#endif /* MBSS_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_SNIFFER_SUPPORT
	RT28xx_Monitor_Remove(pAd);
#endif	/* CONFIG_SNIFFER_SUPPORT */

#ifdef RT_CFG80211_SUPPORT
#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE
	RTMP_CFG80211_AllVirtualIF_Remove(pAd);
	RTMP_CFG80211_DummyP2pIf_Remove(pAd);
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE */	
#else
#endif /* RT_CFG80211_SUPPORT */

#ifdef INF_PPA_SUPPORT
	RTMP_DRIVER_INF_PPA_EXIT(pAd);
#endif /* INF_PPA_SUPPORT */

	/* Unregister network device */
	if (net_dev != NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("RtmpOSNetDevDetach(): RtmpOSNetDeviceDetach(), dev->name=%s!\n", net_dev->name));
		RtmpOSNetDevProtect(1);
		RtmpOSNetDevDetach(net_dev);
		RtmpOSNetDevProtect(0);
	}

	return TRUE;
	
}


/*******************************************************************************

	Device IRQ related functions.
	
 *******************************************************************************/
int RtmpOSIRQRequest(IN PNET_DEV pNetDev)
{
#if defined(RTMP_PCI_SUPPORT) || defined(RTMP_RBUS_SUPPORT)
	struct net_device *net_dev = pNetDev;
#endif
	ULONG infType;
	VOID *pAd = NULL;
	int retval = 0;
	
	GET_PAD_FROM_NET_DEV(pAd, pNetDev);	
	
	ASSERT(pAd);

	RTMP_DRIVER_INF_TYPE_GET(pAd, &infType);

#ifdef RTMP_PCI_SUPPORT
	if (infType == RTMP_DEV_INF_PCI || infType == RTMP_DEV_INF_PCIE)
	{
		struct pci_dev *pci_dev;
		RTMP_DRIVER_PCI_MSI_ENABLE(pAd, &pci_dev);
#ifdef CONFIG_ARCH_MT7623
		retval = request_irq(pci_dev->irq,  rt2860_interrupt, SA_SHIRQ|IRQF_TRIGGER_LOW, (net_dev)->name, (net_dev));
#else
		retval = request_irq(pci_dev->irq,  rt2860_interrupt, SA_SHIRQ, (net_dev)->name, (net_dev));
#endif
		if (retval != 0) 
			printk("RT2860: request_irq  ERROR(%d)\n", retval);
	}
#endif /* RTMP_PCI_SUPPORT */

#ifdef RTMP_RBUS_SUPPORT
	if (infType == RTMP_DEV_INF_RBUS)
	{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,22)
		if ((retval = request_irq(net_dev->irq, rt2860_interrupt, IRQF_SHARED, net_dev->name ,net_dev))) 
#else
		if ((retval = request_irq(net_dev->irq,rt2860_interrupt, SA_INTERRUPT, net_dev->name ,net_dev))) 
#endif
		{
			printk("RT2860: request_irq  ERROR(%d)\n", retval);
		}
	}
#endif /* RTMP_RBUS_SUPPORT */

	return retval; 
}

#ifdef WDS_SUPPORT
/*
    ========================================================================

    Routine Description:
        return ethernet statistics counter

    Arguments:
        net_dev                     Pointer to net_device

    Return Value:
        rtnl_link_stats64 *

    Note:

    ========================================================================
*/
struct rtnl_link_stats64 *
RT28xx_get_wds_ether_stats64(PNET_DEV net_dev, struct rtnl_link_stats64 *stats)
{
	RT_CMD_STATS64 WdsStats;
    VOID *pAd = NULL;

	if (net_dev)
		GET_PAD_FROM_NET_DEV(pAd, net_dev);

	if (!pAd)
		return NULL;

	WdsStats.pNetDev = net_dev;

			if (RTMP_COM_IoctlHandle(pAd, NULL, CMD_RTPRIV_IOCTL_WDS_STATS_GET,
			0, &WdsStats, RT_DEV_PRIV_FLAGS_GET(net_dev)) != NDIS_STATUS_SUCCESS)
				return NULL;

	stats->rx_packets		= WdsStats.rx_packets;
	stats->tx_packets		= WdsStats.tx_packets;
	stats->rx_bytes			= WdsStats.rx_bytes;
	stats->tx_bytes			= WdsStats.tx_bytes;
	stats->rx_errors		= WdsStats.rx_errors;
	stats->tx_errors		= 0;
	stats->rx_dropped		= 0;
	stats->tx_dropped		= 0;
	stats->multicast		= WdsStats.multicast;
	stats->collisions		= 0;

	stats->rx_length_errors		= 0;
	stats->rx_over_errors		= 0;
	stats->rx_crc_errors		= 0;
	stats->rx_frame_errors		= 0;
	stats->rx_fifo_errors		= 0;
	stats->rx_missed_errors		= 0;

	stats->tx_aborted_errors	= 0;
	stats->tx_carrier_errors	= 0;
	stats->tx_fifo_errors		= 0;
	stats->tx_heartbeat_errors	= 0;
	stats->tx_window_errors		= 0;

	stats->rx_compressed		= 0;
	stats->tx_compressed		= 0;

	return stats;
}
#endif /* WDS_SUPPORT */

#ifdef APCLI_SUPPORT
/*
    ========================================================================

    Routine Description:
        return ethernet statistics counter

    Arguments:
        net_dev                     Pointer to net_device

    Return Value:
        rtnl_link_stats64 *

    Note:

    ========================================================================
*/
struct rtnl_link_stats64 *
RT28xx_get_apcli_ether_stats64(PNET_DEV net_dev, struct rtnl_link_stats64 *stats)
{
	RT_CMD_STATS64 ApCliStats;
	VOID *pAd = NULL;

	if (net_dev)
		GET_PAD_FROM_NET_DEV(pAd, net_dev);

	if (!pAd)
			return NULL;

	ApCliStats.pNetDev = net_dev;
	if (RTMP_COM_IoctlHandle(pAd, NULL, CMD_RTPRIV_IOCTL_APCLI_STATS_GET,
			0, &ApCliStats, RT_DEV_PRIV_FLAGS_GET(net_dev)) != NDIS_STATUS_SUCCESS)
		return NULL;

	stats->rx_packets		= ApCliStats.rx_packets;
	stats->tx_packets		= ApCliStats.tx_packets;
	stats->rx_bytes			= ApCliStats.rx_bytes;
	stats->tx_bytes			= ApCliStats.tx_bytes;
	stats->rx_errors		= ApCliStats.rx_errors;
	stats->tx_errors		= 0;
	stats->rx_dropped		= 0;
	stats->tx_dropped		= 0;
	stats->multicast		= ApCliStats.multicast;
	stats->collisions		= 0;

	stats->rx_length_errors		= 0;
	stats->rx_over_errors		= 0;
	stats->rx_crc_errors		= 0;
	stats->rx_frame_errors		= 0;
	stats->rx_fifo_errors		= 0;
	stats->rx_missed_errors		= 0;

	stats->tx_aborted_errors	= 0;
	stats->tx_carrier_errors	= 0;
	stats->tx_fifo_errors		= 0;
	stats->tx_heartbeat_errors	= 0;
	stats->tx_window_errors		= 0;

	stats->rx_compressed		= 0;
	stats->tx_compressed		= 0;

	return stats;
}
#endif /* APCLI_SUPPORT */




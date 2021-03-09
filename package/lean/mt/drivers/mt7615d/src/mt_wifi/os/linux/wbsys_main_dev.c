/*
 ***************************************************************************
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

#ifdef RTMP_RBUS_SUPPORT

#define RTMP_MODULE_OS

#include "rt_config.h"
#include "os/wbsys_res.h"
#include <linux/dma-mapping.h>

#if defined(CONFIG_RA_CLASSIFIER) && (!defined(CONFIG_RA_CLASSIFIER_MODULE))
extern int (*ra_classifier_init_func)(void);
extern void (*ra_classifier_release_func)(void);
extern struct proc_dir_entry *proc_ptr, *proc_ralink_wl_video;
#endif

/*
*
*/
#ifdef MEM_ALLOC_INFO_SUPPORT

extern MEM_INFO_LIST MemInfoList;
extern MEM_INFO_LIST PktInfoList;

static void free_meminfo(void)
{
	UINT32 memalctotal, pktalctotal;

	memalctotal = ShowMemAllocInfo();
	pktalctotal = ShowPktAllocInfo();

	if ((memalctotal != 0) || (pktalctotal != 0)) {
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("Error: Memory leak!!\n"));
		ASSERT(0);
	}

	MIListExit(&MemInfoList);
	MIListExit(&PktInfoList);
}
#endif /* MEM_ALLOC_INFO_SUPPORT */

/*
*
*/
static int wbsys_probe(struct platform_device *pdev)
{
	struct resource *res;
	unsigned long base_addr;
	unsigned int dev_irq;
	struct net_device *net_dev;
	int rv;
	void *handle = NULL;
	RTMP_ADAPTER *pAd;
	RTMP_OS_NETDEV_OP_HOOK netDevHook;
	unsigned int Value;

	/*resource allocate*/
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	dev_irq = platform_get_irq(pdev, 0);
	base_addr = (unsigned long)devm_ioremap(&pdev->dev, res->start, resource_size(res));
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s(): irq=%d,base_addr=%lx\n", __func__, dev_irq, base_addr));

	if (dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(32))) {
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR,
				 ("set DMA mask failed!errno=%d\n", rv));
		goto err_out;
	}

	/*other global resource allocation*/
#ifdef MEM_ALLOC_INFO_SUPPORT
	MemInfoListInital();
#endif /* MEM_ALLOC_INFO_SUPPORT */
	/* RtmpDevInit */
	/* Allocate RTMP_ADAPTER adapter structure */
	os_alloc_mem(NULL, (UCHAR **)&handle, sizeof(struct os_cookie));

	if (!handle) {
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR, ("Allocate memory for os_cookie failed!\n"));
		goto err_out;
	}

	os_zero_mem(handle, sizeof(struct os_cookie));
#ifdef OS_ABL_FUNC_SUPPORT
	/* get DRIVER operations */
	RTMP_DRV_OPS_FUNCTION(pRtmpDrvOps, NULL, NULL, NULL);
#endif /* OS_ABL_FUNC_SUPPORT */
	rv = RTMPAllocAdapterBlock(handle, (VOID **)&pAd);

	if (rv != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR, (" RTMPAllocAdapterBlock !=  NDIS_STATUS_SUCCESS\n"));
		os_free_mem(handle);
		goto err_out;
	}

	/* Here are the RTMP_ADAPTER structure with rbus-bus specific parameters. */
	pAd->PciHif.CSRBaseAddress = (PUCHAR)base_addr;
	RTMP_IO_READ32(pAd, TOP_HCR, &Value);
	pAd->ChipID = Value;
	/*link platform dev to pAd*/
	((POS_COOKIE)handle)->pci_dev = (void *)pdev;
	/*link platform dev to pAd*/
	((POS_COOKIE)handle)->pDev = &pdev->dev;
	RtmpRaDevCtrlInit(pAd, RTMP_DEV_INF_RBUS);
	/*Prepare netdev for wifi dev, NetDevInit */
	net_dev = RtmpPhyNetDevInit(pAd, &netDevHook);

	if (net_dev == NULL)
		goto err_out_free_radev;

	/* Interrupt IRQ number */
	net_dev->irq = dev_irq;
	/* Save CSR virtual address and irq to device structure */
	net_dev->base_addr = base_addr;
	/*link net_dev to platform_dev*/
	platform_set_drvdata(pdev, net_dev);
	/*link platform_dev to net_dev*/
	SET_NETDEV_DEV(net_dev, &pdev->dev);
	/*All done, it's time to register the net device to kernel. */
	rv = RtmpOSNetDevAttach(pAd->OpMode, net_dev, &netDevHook);

	if (rv) {
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR, ("failed to call RtmpOSNetDevAttach(), rv=%d!\n", rv));
		goto err_out_free_netdev;
	}

	wl_proc_init();
	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE, ("%s: at CSR addr 0x%lx, IRQ %ld.\n",
			 net_dev->name, (ULONG)base_addr, (long int)net_dev->irq));
	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE, ("<=== wifi probe\n"));
#if defined(CONFIG_RA_CLASSIFIER) && (!defined(CONFIG_RA_CLASSIFIER_MODULE))
	proc_ptr = proc_ralink_wl_video;

	if (ra_classifier_init_func != NULL)
		ra_classifier_init_func();

#endif
	return 0;
err_out_free_netdev:
	RtmpOSNetDevFree(net_dev);
#ifdef MEM_ALLOC_INFO_SUPPORT
	free_meminfo();
#endif /* MEM_ALLOC_INFO_SUPPORT */
err_out_free_radev:
	/* free RTMP_ADAPTER strcuture and os_cookie*/
	RTMPFreeAdapter(pAd);
err_out:
	return -ENODEV;
}

/*
*
*/
static int wbsys_remove(struct platform_device *pdev)
{
	struct net_device *net_dev = platform_get_drvdata(pdev);
	RTMP_ADAPTER *pAd;

	if (net_dev == NULL)
		return -ENODEV;

	/* pAd = net_dev->priv*/
	GET_PAD_FROM_NET_DEV(pAd, net_dev);

	if (pAd != NULL) {
		RtmpPhyNetDevExit(pAd, net_dev);
		RtmpRaDevCtrlExit(pAd);
	} else
		RtmpOSNetDevDetach(net_dev);

	/* Free the root net_device. */
	RtmpOSNetDevFree(net_dev);
#if defined(CONFIG_RA_CLASSIFIER) && (!defined(CONFIG_RA_CLASSIFIER_MODULE))
	proc_ptr = proc_ralink_wl_video;

	if (ra_classifier_release_func != NULL)
		ra_classifier_release_func();

#endif
	wl_proc_exit();
#ifdef MEM_ALLOC_INFO_SUPPORT
	free_meminfo();
#endif /* MEM_ALLOC_INFO_SUPPORT */
	return 0;
}

static int wbsys_suspend(struct platform_device *pdev, pm_message_t state)
{
	INT32 retval = 0;
	struct net_device *net_dev = platform_get_drvdata(pdev);
	VOID *pAd = NULL;

	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR, ("===>%s()\n", __func__));

	if (net_dev == NULL)
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR, ("net_dev == NULL!\n"));
	else {
		UINT32 IfNum;

		GET_PAD_FROM_NET_DEV(pAd, net_dev);
		/* we can not use IFF_UP because ra0 down but ra1 up */
		/* and 1 suspend/resume function for 1 module, not for each interface */
		/* so Linux will call suspend/resume function once */
		RTMP_DRIVER_VIRTUAL_INF_NUM_GET(pAd, &IfNum);

		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR, ("%s::IfNum=%d\n", __func__, IfNum));

		if (IfNum > 0) {
			/* avoid users do suspend after interface is down */
			/* stop interface */
			netif_carrier_off(net_dev);
			netif_stop_queue(net_dev);
			/* mark device as removed from system and therefore no longer available */
			netif_device_detach(net_dev);
			RTMP_DRIVER_RBUS_SUSPEND(pAd);
			RT_MOD_HNAT_DEREG(net_dev);
			RT_MOD_DEC_USE_COUNT();
		}
	}

	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR, ("<===%s()\n", __func__));
	return retval;
}

static int wbsys_resume(struct platform_device *pdev)
{
	struct net_device *net_dev = platform_get_drvdata(pdev);
	VOID *pAd = NULL;

	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR, ("===>%s()\n", __func__));

	if (net_dev == NULL)
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR, ("net_dev == NULL!\n"));
	else
		GET_PAD_FROM_NET_DEV(pAd, net_dev);

	if (pAd != NULL) {
		UINT32 IfNum;
		/*
		 *	we can not use IFF_UP because ra0 down but ra1 up
		 *	and 1 suspend/resume function for 1 module, not for each interface
		 *	so Linux will call suspend/resume function once
		 */
		RTMP_DRIVER_VIRTUAL_INF_NUM_GET(pAd, &IfNum);

		if (IfNum > 0) {
			/* mark device as attached from system and restart if needed */
			netif_device_attach(net_dev);
			/* increase MODULE use count */
			RT_MOD_INC_USE_COUNT();
			RT_MOD_HNAT_REG(net_dev);
			RTMP_DRIVER_RBUS_RESUME(pAd);
			netif_start_queue(net_dev);
			netif_carrier_on(net_dev);
			netif_wake_queue(net_dev);
		}
	}

	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR, ("<=== %s()\n", __func__));
	return 0;
}

/*
* global resource preparing
*/
static struct platform_driver wbsys_driver = {
	.probe  = wbsys_probe,
	.remove = wbsys_remove,
#ifdef CONFIG_PM
	.suspend = wbsys_suspend,
	.resume = wbsys_resume,
#endif /* CONFIG_PM */
	.driver = {
		.name   = wbsys_string,
		.owner  = THIS_MODULE,
#ifdef CONFIG_OF
		.of_match_table = wbsys_of_ids,
#endif /*CONFIG_OF*/
	},
};

/*
*
 */
int __init wbsys_module_init(void)
{
	int ret;

	wbsys_dev_alloc(&wbsys_dev);
	ret = platform_driver_register(&wbsys_driver);
	return ret;
}


VOID __exit wbsys_module_exit(void)
{
	wbsys_dev_release(&wbsys_dev);
	platform_driver_unregister(&wbsys_driver);
}

#ifndef MULTI_INF_SUPPORT
module_init(wbsys_module_init);
module_exit(wbsys_module_exit);

MODULE_DESCRIPTION("Register for MTK WIFI driver HIF is internal bus");
MODULE_SUPPORTED_DEVICE("mtk wbsys platform driver");
#endif

#endif /* RTMP_RBUS_SUPPORT */


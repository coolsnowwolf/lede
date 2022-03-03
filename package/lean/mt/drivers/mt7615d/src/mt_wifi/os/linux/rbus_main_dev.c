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
    rbus_main_dev.c

    Abstract:
    Create and register network interface for RBUS based chipsets in linux platform.

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
*/
#ifdef RTMP_RBUS_SUPPORT

#define RTMP_MODULE_OS

#include "rt_config.h"

static struct net_device *rt2880_dev;
static struct platform_device *wbsys_pdev;

#if defined(CONFIG_RA_CLASSIFIER) && (!defined(CONFIG_RA_CLASSIFIER_MODULE))
extern int (*ra_classifier_init_func)(void);
extern void (*ra_classifier_release_func)(void);
extern struct proc_dir_entry *proc_ptr, *proc_ralink_wl_video;
#endif

#ifdef MEM_ALLOC_INFO_SUPPORT
extern MEM_INFO_LIST MemInfoList;
extern MEM_INFO_LIST PktInfoList;
#endif /*MEM_ALLOC_INFO_SUPPORT*/


int __init wbsys_module_init(void)
{
	struct net_device *net_dev;
	ULONG csr_addr;
	INT rv;
	void *handle = NULL;
	RTMP_ADAPTER *pAd;
	unsigned int dev_irq;
	RTMP_OS_NETDEV_OP_HOOK netDevHook;
	UINT32 Value;

	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE, ("===> rt2880_probe\n"));
#ifdef MEM_ALLOC_INFO_SUPPORT
	MemInfoListInital();
#endif /* MEM_ALLOC_INFO_SUPPORT */
	/*RtmpRaBusInit============================================ */
	/* map physical address to virtual address for accessing register */
	csr_addr = (unsigned long)ioremap(RTMP_MAC_CSR_ADDR, RTMP_MAC_CSR_LEN);
	dev_irq = RTMP_IRQ_NUM;
	/*RtmpDevInit============================================== */
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
	pAd->PciHif.CSRBaseAddress = (PUCHAR)csr_addr;
	RTMP_IO_READ32(pAd, TOP_HCR, &Value);
	pAd->ChipID = Value;
	/*assign pdev to handle before RtmpRaDevCtrlInit*/
	RtmpRaDevCtrlInit(pAd, RTMP_DEV_INF_RBUS);
	/*NetDevInit============================================== */
	net_dev = RtmpPhyNetDevInit(pAd, &netDevHook);

	if (net_dev == NULL)
		goto err_out_free_radev;

	/* Here are the net_device structure with pci-bus specific parameters. */
	net_dev->irq = dev_irq;			/* Interrupt IRQ number */
	net_dev->base_addr = csr_addr;		/* Save CSR virtual address and irq to device structure */
	/*is not a regular method*/
	((POS_COOKIE)handle)->pci_dev = (VOID *)wbsys_pdev;
	((POS_COOKIE)handle)->pDev = &net_dev->dev;
	RTMP_DRIVER_CHIP_PREPARE(pAd);
	/*All done, it's time to register the net device to kernel. */
	/* Register this device */
	rv = RtmpOSNetDevAttach(pAd->OpMode, net_dev, &netDevHook);

	if (rv) {
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR, ("failed to call RtmpOSNetDevAttach(), rv=%d!\n", rv));
		goto err_out_free_netdev;
	}

	/* due to we didn't have any hook point when do module remove, we use this static as our hook point. */
	rt2880_dev = net_dev;
	wl_proc_init();
	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE, ("%s: at CSR addr 0x%lx, IRQ %ld.\n", net_dev->name, (ULONG)csr_addr, (long int)net_dev->irq));
	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE, ("<=== rt2880_probe\n"));
#if defined(CONFIG_RA_CLASSIFIER) && (!defined(CONFIG_RA_CLASSIFIER_MODULE))
	proc_ptr = proc_ralink_wl_video;

	if (ra_classifier_init_func != NULL)
		ra_classifier_init_func();

#endif
	return 0;
err_out_free_netdev:
	RtmpOSNetDevFree(net_dev);
#ifdef MEM_ALLOC_INFO_SUPPORT
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
err_out_free_radev:
	/* free RTMP_ADAPTER strcuture and os_cookie*/
	RTMPFreeAdapter(pAd);
err_out:
	return -ENODEV;
}

VOID __exit wbsys_module_exit(void)
{
	struct net_device *net_dev = rt2880_dev;
	RTMP_ADAPTER *pAd;

	if (net_dev == NULL)
		return;

	/* pAd = net_dev->priv; */
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
}

/** @} */
/** @} */

#ifndef MULTI_INF_SUPPORT
module_init(wbsys_module_init);
module_exit(wbsys_module_exit);
#endif /* MULTI_INF_SUPPORT */

#endif /* RTMP_RBUS_SUPPORT */


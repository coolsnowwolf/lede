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
    pci_main_dev.c

    Abstract:
    Create and register network interface for PCI based chipsets in Linux platform.

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
*/

#define RTMP_MODULE_OS

#include "rtmp_comm.h"
#include "rtmp_chip.h"
#include "rt_os_util.h"
#include "rt_os_net.h"
#include <linux/pci.h>
#include "rt_config.h"

#if (KERNEL_VERSION(3, 8, 0) <= LINUX_VERSION_CODE)
#define DEVEXIT
#define DEVEXIT_P
#define DEVINIT
#define DEVINITDATA
#else
#define DEVEXIT __devexit
#define DEVEXIT_P __devexit_p
#define DEVINIT __devinit
#define DEVINITDATA __devinitdata
#endif

#ifdef MEM_ALLOC_INFO_SUPPORT
extern MEM_INFO_LIST MemInfoList;
extern MEM_INFO_LIST PktInfoList;
#endif /*MEM_ALLOC_INFO_SUPPORT*/
/**
 * @addtogroup hif
 * @{
 * @code PCI Device ID
 */

static struct pci_device_id rt_pci_tbl[] DEVINITDATA = {




#ifdef MT7615
	{PCI_DEVICE(MTK_PCI_VENDOR_ID, NIC7615_PCIe_DEVICE_ID)},
	{PCI_DEVICE(MTK_PCI_VENDOR_ID, NIC7616_PCIe_DEVICE_ID)},
	{PCI_DEVICE(MTK_PCI_VENDOR_ID, NIC761A_PCIe_DEVICE_ID)},
	{PCI_DEVICE(MTK_PCI_VENDOR_ID, NIC7611_PCIe_DEVICE_ID)},
#endif /* MT7615 */
	{} /* terminate list */
};

MODULE_DEVICE_TABLE(pci, rt_pci_tbl);

/** @endcode */
/** @} */

/**
 * @addtogroup hif HIF operation
 * @{
 * @name PCIe initialzation
 * @{
 */

/***************************************************************************
 *
 *	PCI device initialization related procedures.
 *
 ***************************************************************************/
#if (KERNEL_VERSION(2, 5, 0) <= LINUX_VERSION_CODE)
#ifdef CONFIG_PM

#if (KERNEL_VERSION(2, 6, 10) >= LINUX_VERSION_CODE)
#define pm_message_t u32
#endif

static int rt_pci_suspend(struct pci_dev *pci_dev, pm_message_t state)
{
	struct net_device *net_dev = pci_get_drvdata(pci_dev);
	VOID *pAd = NULL;
	INT32 retval = 0;

	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE, ("===>%s()\n", __func__));

	if (net_dev == NULL)
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR, ("net_dev == NULL!\n"));
	else {
		ULONG IfNum;

		GET_PAD_FROM_NET_DEV(pAd, net_dev);
		/* we can not use IFF_UP because ra0 down but ra1 up */
		/* and 1 suspend/resume function for 1 module, not for each interface */
		/* so Linux will call suspend/resume function once */
		RTMP_DRIVER_VIRTUAL_INF_NUM_GET(pAd, &IfNum);

		if (IfNum > 0) {
			/* avoid users do suspend after interface is down */
			/* stop interface */
			netif_carrier_off(net_dev);
			netif_stop_queue(net_dev);
			/* mark device as removed from system and therefore no longer available */
			netif_device_detach(net_dev);
			RTMP_DRIVER_PCI_SUSPEND(pAd);
			RT_MOD_HNAT_DEREG(net_dev);
			RT_MOD_DEC_USE_COUNT();
		}
	}

#if (KERNEL_VERSION(2, 6, 10) < LINUX_VERSION_CODE)
	/*
	 *	reference to http://vovo2000.com/type-lab/linux/kernel-api/linux-kernel-api.html
	 *	enable device to generate PME# when suspended
	 *	pci_choose_state(): Choose the power state of a PCI device to be suspended
	 */
	retval = pci_enable_wake(pci_dev, pci_choose_state(pci_dev, state), 1);
	/* save the PCI configuration space of a device before suspending */
	pci_save_state(pci_dev);
	/* disable PCI device after use */
	pci_disable_device(pci_dev);
	retval = pci_set_power_state(pci_dev, pci_choose_state(pci_dev, state));
#endif
	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE, ("<===%s()\n", __func__));
	return retval;
}


static int rt_pci_resume(struct pci_dev *pci_dev)
{
	struct net_device *net_dev = pci_get_drvdata(pci_dev);
	VOID *pAd = NULL;
#if (KERNEL_VERSION(2, 6, 10) < LINUX_VERSION_CODE)
	INT32 retval;
	/*  Set the power state of a PCI device
	 *
	 *	PCI has 4 power states, DO (normal) ~ D3(less power)
	 *	you can find that in include/linux/pci.h
	 *	#define PCI_D0          ((pci_power_t __force) 0)
	 *	#define PCI_D1          ((pci_power_t __force) 1)
	 *	#define PCI_D2          ((pci_power_t __force) 2)
	 *	#define PCI_D3hot       ((pci_power_t __force) 3)
	 *	#define PCI_D3cold      ((pci_power_t __force) 4)
	 *	#define PCI_UNKNOWN     ((pci_power_t __force) 5)
	 *	#define PCI_POWER_ERROR ((pci_power_t __force) -1)
	 */
	retval = pci_set_power_state(pci_dev, PCI_D0);
	/* restore the saved state of a PCI device */
	pci_restore_state(pci_dev);

	/* initialize device before it's used by a driver */
	if (pci_enable_device(pci_dev)) {
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR, ("pci enable fail!\n"));
		return 0;
	}

#endif
	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE, ("===>%s()\n", __func__));

	if (net_dev == NULL)
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR, ("net_dev == NULL!\n"));
	else
		GET_PAD_FROM_NET_DEV(pAd, net_dev);

	if (pAd != NULL) {
		ULONG IfNum;
		/*
		 *	we can not use IFF_UP because ra0 down but ra1 up
		 *	and 1 suspend/resume function for 1 module, not for each interface
		 *	so Linux will call suspend/resume function once
		 */
		RTMP_DRIVER_VIRTUAL_INF_NUM_GET(pAd, &IfNum);

		if (IfNum > 0)
			/*		if (VIRTUAL_IF_NUM(pAd) > 0) */
		{
			/* mark device as attached from system and restart if needed */
			netif_device_attach(net_dev);
			/* increase MODULE use count */
			RT_MOD_INC_USE_COUNT();
			RT_MOD_HNAT_REG(net_dev);
			/*			RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS); */
			/*			RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF); */
			RTMP_DRIVER_PCI_RESUME(pAd);
			netif_start_queue(net_dev);
			netif_carrier_on(net_dev);
			netif_wake_queue(net_dev);
		}
	}

	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE, ("<=== %s()\n", __func__));
	return 0;
}
#endif /* CONFIG_PM */
#endif

/*
 *	 PCI device probe & initialization function
 */
#ifdef INTELP6_SUPPORT
#ifdef MULTI_INF_SUPPORT
int g_AdapCount;
#endif
#endif
static int DEVINIT rt_pci_probe(struct pci_dev *pdev, const struct pci_device_id *pci_id)
{
	void *pAd = NULL, *handle;
	struct net_device *net_dev;
	char *print_name;
	unsigned long csr_addr;
	int rv = 0;
	RTMP_OS_NETDEV_OP_HOOK netDevHook;
	unsigned long OpMode;
	RT_CMD_PCIE_INIT pci_config;

	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE, ("===> %s()\n", __func__));
	/*PCIDevInit============================================== */
	/* wake up and enable device */
	rv = pci_enable_device(pdev);

	if (rv != 0) {
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR,
				 ("Enable PCI device failed, errno=%d!\n", rv));
		return rv;
	}

#if (KERNEL_VERSION(2, 5, 0) <= LINUX_VERSION_CODE)
	print_name = (char *)pci_name(pdev);
#else
	print_name = pdev->slot_name;
#endif /* LINUX_VERSION_CODE */

#if defined(PLATFORM_M_STB)
	if (IS_ENABLED(CONFIG_ARM64) && (!IS_ENABLED(CONFIG_ZONE_DMA))) {
		if ((rv = pci_set_dma_mask(pdev, DMA_BIT_MASK(64))) != 0) {
			MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR,
					 ("set DMA mask failed!errno=%d\n", rv));
			return rv;
		}

		if ((rv = pci_set_consistent_dma_mask(pdev, DMA_BIT_MASK(64))) != 0) {
			MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR,
					 ("set DMA consistent mask failed!errno=%d\n", rv));
			return rv;
		}
	}
	else
#endif
	if (!pci_set_dma_mask(pdev, DMA_BIT_MASK(32))) {
		/*
		 * pci_set_consistent_dma_mask() will always be able to set the same
		 * or a smaller mask as pci_set_dma_mask()
		 */
		pci_set_consistent_dma_mask(pdev, DMA_BIT_MASK(32));
	} else {
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR,
				 ("set DMA mask failed!errno=%d\n", rv));
		goto err_out;
	}

	/*msi request*/
#ifdef CONFIG_WIFI_MSI_SUPPORT
	if (RtmpOsPciMsiEnable(pdev) != 0) {
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR,
		 ("Request MSI resource failed, use INTx!\n"));
	}
#endif /*CONFIG_WIFI_MSI_SUPPORT*/
	rv = pci_request_regions(pdev, print_name);

	if (rv != 0) {
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR,
				 ("Request PCI resource failed, errno=%d!\n", rv));
		goto err_out;
	}

	/* map physical address to virtual address for accessing register */
	csr_addr = (unsigned long) ioremap(pci_resource_start(pdev, 0), pci_resource_len(pdev, 0));

	if (!csr_addr) {
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR,
				 ("ioremap failed for device %s, region 0x%lX @ 0x%lX\n",
				  print_name, (ULONG)pci_resource_len(pdev, 0),
				  (ULONG)pci_resource_start(pdev, 0)));
		goto err_out_free_res;
	} else {
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE,
				 ("%s: at 0x%lx, VA 0x%lx, IRQ %d.\n",  print_name,
				  (ULONG)pci_resource_start(pdev, 0), (ULONG)csr_addr, pdev->irq));
	}

	/* Set DMA master */
	pci_set_master(pdev);

	/* Allocate RTMP_ADAPTER adapter structure */
	os_alloc_mem(NULL, (UCHAR **)&handle, sizeof(struct os_cookie));

	if (handle == NULL) {
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR,
				 ("%s(): Allocate memory for os handle failed!\n",
				  __func__));
		goto err_out_iounmap;
	}

	memset(handle, 0, sizeof(struct os_cookie));
	((POS_COOKIE)handle)->pci_dev = pdev;
	((POS_COOKIE)handle)->pDev = &(pdev->dev);
#ifdef OS_ABL_FUNC_SUPPORT
	{
		RTMP_PCI_CONFIG PciConfig;

		PciConfig.ConfigVendorID = PCI_VENDOR_ID;
		/* get DRIVER operations */
		RTMP_DRV_OPS_FUNCTION(pRtmpDrvOps, NULL, &PciConfig, NULL);
	}
#endif /* OS_ABL_FUNC_SUPPORT */
	/* we may need the pdev for allocate structure of "RTMP_ADAPTER" */
	rv = RTMPAllocAdapterBlock(handle, &pAd);

	if (rv != NDIS_STATUS_SUCCESS)
		goto err_out_iounmap;

	RTMP_DRIVER_PCI_CSR_SET(pAd, csr_addr);
	RTMP_DRIVER_PCIE_INIT(pAd, &pci_config, pdev);

	if (pci_config.pci_init_succeed != TRUE)
		goto err_out_free_radev;

	/*NetDevInit============================================== */
	net_dev = RtmpPhyNetDevInit(pAd, &netDevHook);

	if (net_dev == NULL)
		goto err_out_free_radev;

	/* Here are the net_device structure with pci-bus specific parameters. */
	net_dev->irq = pdev->irq;
	net_dev->base_addr = csr_addr;
	pci_set_drvdata(pdev, net_dev);
	/* +++Add by shiang for jeffrey debug */
#ifdef LINUX
	net_dev->features &= (~NETIF_F_HIGHDMA);
#endif /* LINUX */
	/* ---Add by shiang for jeffrey debug */
	RTMP_DRIVER_CHIP_PREPARE(pAd);
	/*All done, it's time to register the net device to linux kernel. */
	/* Register this device */
	RTMP_DRIVER_OP_MODE_GET(pAd, &OpMode);
	rv = RtmpOSNetDevAttach(OpMode, net_dev, &netDevHook);

	if (rv)
		goto err_out_free_netdev;

#ifdef PRE_ASSIGN_MAC_ADDR
	{
		UCHAR mac_addr[MAC_ADDR_LEN];

		RTMP_DRIVER_MAC_ADDR_GET(pAd, &mac_addr[0]);
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE,
				 ("@%s MAC address: %02x:%02x:%02x:%02x:%02x:%02x\n",
				  __func__, mac_addr[0], mac_addr[1],
				  mac_addr[2], mac_addr[3],
				  mac_addr[4], mac_addr[5]));
		/* Set up the Mac address */
		RtmpOSNetDevAddrSet(OpMode, net_dev, &mac_addr[0], NULL);
	}
#endif /* PRE_ASSIGN_MAC_ADDR */
#ifdef INTELP6_SUPPORT
#ifdef MULTI_INF_SUPPORT
	g_AdapCount++;
	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("pci probe count=%d\n", g_AdapCount));
#endif
#endif
#ifdef WIFI_DIAG
	DiagCtrlAlloc(pAd);
#endif
	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE, ("<=%s()\n", __func__));
#ifdef CREATE_ALL_INTERFACE_AT_INIT
#ifdef MBSS_SUPPORT
		RT28xx_MBSS_Init(pAd, (PNET_DEV)net_dev);
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("[ARRIS MOD] MT7615E MBSS Initialized\n"));
#endif /* MBSS_SUPPORT */
#endif
	return 0; /* probe ok */
	/* --------------------------- ERROR HANDLE --------------------------- */
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
err_out_iounmap:
	iounmap((void *)(csr_addr));
	pci_release_regions(pdev);
err_out_free_res:
	pci_release_regions(pdev);
err_out:
	pci_disable_device(pdev);
	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR, ("<=== %s() failed with rv = %d!\n",
			 __func__, rv));
	return -ENODEV; /* probe fail */
}


static VOID DEVEXIT rt_pci_remove(struct pci_dev *pci_dev)
{
#ifdef INTELP6_SUPPORT
#ifdef MULTI_INF_SUPPORT
	static UINT16 pci_dev_count = MAX_NUM_OF_INF;

	while (pci_dev_count) {
#endif
#endif
		PNET_DEV net_dev;
		VOID *pAd = NULL;
		ULONG csr_addr;
#ifdef INTELP6_SUPPORT
#ifdef MULTI_INF_SUPPORT
		pci_dev_count--;
		pAd = adapt_list[pci_dev_count];
		if (pAd == NULL)
			continue;
		else
			pci_dev = rtmp_get_pci_dev(pAd);
#endif
#endif
		net_dev = pci_get_drvdata(pci_dev);
		csr_addr = net_dev->base_addr;
#ifndef INTELP6_SUPPORT
		GET_PAD_FROM_NET_DEV(pAd, net_dev);
#endif

		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE, ("===> %s()\n", __func__));

		if (pAd != NULL) {
#ifdef WIFI_DIAG
			DiagCtrlFree(pAd);
#endif
			/* Unregister/Free all allocated net_device. */
			RtmpPhyNetDevExit(pAd, net_dev);
			/* Free RTMP_ADAPTER related structures. */
			RtmpRaDevCtrlExit(pAd);
			/* Unmap CSR base address */
			iounmap((char *)(csr_addr));
			/* release memory region */
			pci_release_regions(pci_dev);
		} else {
			/* Unregister network device */
			RtmpOSNetDevDetach(net_dev);
			/* Unmap CSR base address */
			iounmap((char *)(net_dev->base_addr));
			/* release memory region */
			pci_release_regions(pci_dev);
		}
#ifdef CONFIG_WIFI_MSI_SUPPORT
		if (pci_dev->msi_enabled)
			RtmpOsPciMsiDisable(pci_dev);
#endif /*CONFIG_WIFI_MSI_SUPPORT*/

		/* Free the root net_device */
		RtmpOSNetDevFree(net_dev);
#ifdef INTELP6_SUPPORT
#ifdef MULTI_INF_SUPPORT
		g_AdapCount--;
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_OFF, ("In remove, pci probe count=%d\n", g_AdapCount));
	}
#endif
#endif
#ifdef MEM_ALLOC_INFO_SUPPORT
#ifdef INTELP6_SUPPORT
#ifdef MULTI_INF_SUPPORT
	if (g_AdapCount == 0)
#endif
#endif
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


/*
 *	Our PCI driver structure
 */
static struct pci_driver rt_pci_driver = {
name:
	RTMP_DRV_NAME,
id_table :
	rt_pci_tbl,
probe :
	rt_pci_probe,
#if LINUX_VERSION_CODE >= 0x20412
remove :
	DEVEXIT_P(rt_pci_remove),
#else
remove :
	__devexit(rt_pci_remove),
#endif

#if (KERNEL_VERSION(2, 5, 0) <= LINUX_VERSION_CODE)
#ifdef CONFIG_PM
suspend :
	rt_pci_suspend,
resume :
	rt_pci_resume,
#endif
#endif
};


/*
 *	Driver module load/unload function
 */
int __init rt_pci_init_module(void)
{
	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR, ("register %s\n", RTMP_DRV_NAME));
#ifdef MEM_ALLOC_INFO_SUPPORT
	MemInfoListInital();
#endif /* MEM_ALLOC_INFO_SUPPORT */
	/*
	 *	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR, ("DriverVersion: 2.7.0.2-Beta-121007\n"
	 *								"\tBBP:120824\n"
	 *								"\tRF :120813\n"));
	 */
#if (KERNEL_VERSION(2, 5, 0) <= LINUX_VERSION_CODE)
	return pci_register_driver(&rt_pci_driver);
#else
	return pci_module_init(&rt_pci_driver);
#endif
}


void __exit rt_pci_cleanup_module(void)
{
	pci_unregister_driver(&rt_pci_driver);
}

#ifdef MT7622_FPGA
late_initcall(rt_pci_init_module);
#endif /*MT7622_FPGA*/

/** @} */
/** @} */
#ifndef MULTI_INF_SUPPORT
module_init(rt_pci_init_module);
module_exit(rt_pci_cleanup_module);

#endif /* MULTI_INF_SUPPORT */

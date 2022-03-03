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
#include "rt_os_util.h"
#include "rt_os_net.h"
#include <linux/pci.h>

#ifdef MULTI_INF_SUPPORT
/* Index 0 for 2.4G, 1 for 5Ghz Card */
extern VOID* pAdGlobalList[2];
#endif /* MULTI_INF_SUPPORT */

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 8, 0)
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

/*
	Ralink PCI device table, include all supported chipsets
*/
static struct pci_device_id rt_pci_tbl[] DEVINITDATA =
{
#ifdef RT8592
	{PCI_DEVICE(NIC_PCI_VENDOR_ID, NIC8592_PCIe_DEVICE_ID)},
#endif /* RT8592 */
#ifdef MT76x2
	{PCI_DEVICE(0x1400, NIC7662_PCIe_DEVICE_ID)},
	{PCI_DEVICE(MTK_PCI_VENDOR_ID, NIC7662_PCIe_DEVICE_ID)},
	{PCI_DEVICE(MTK_PCI_VENDOR_ID, NIC7632_PCIe_DEVICE_ID)},
	{PCI_DEVICE(MTK_PCI_VENDOR_ID, NIC7612_PCIe_DEVICE_ID)},
	{PCI_DEVICE(MTK_PCI_VENDOR_ID, NIC7602_PCIe_DEVICE_ID)},
#endif /* MT76x2 */
	{} /* terminate list */
};

MODULE_DEVICE_TABLE(pci, rt_pci_tbl);


/***************************************************************************
 *
 *	PCI device initialization related procedures.
 *
 ***************************************************************************/
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0)
#ifdef CONFIG_PM

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,10)
#define pm_message_t u32
#endif

static int rt_pci_suspend(struct pci_dev *pci_dev, pm_message_t state)
{
	struct net_device *net_dev = pci_get_drvdata(pci_dev);
	VOID *pAd = NULL;
	INT32 retval = 0;


	DBGPRINT(RT_DEBUG_TRACE, ("===>%s()\n", __FUNCTION__));

	if (net_dev == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("net_dev == NULL!\n"));
	}
	else
	{
		ULONG IfNum;

		GET_PAD_FROM_NET_DEV(pAd, net_dev);

		/* we can not use IFF_UP because ra0 down but ra1 up */
		/* and 1 suspend/resume function for 1 module, not for each interface */
		/* so Linux will call suspend/resume function once */
		RTMP_DRIVER_VIRTUAL_INF_NUM_GET(pAd, &IfNum);
		if (IfNum > 0)
		{
			/* avoid users do suspend after interface is down */

			/* stop interface */
			netif_carrier_off(net_dev);
			netif_stop_queue(net_dev);

			/* mark device as removed from system and therefore no longer available */
			netif_device_detach(net_dev);

			/* mark halt flag */
/*			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS); */
/*			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF); */
			RTMP_DRIVER_PCI_SUSPEND(pAd);

			/* take down the device */
			rt28xx_close((PNET_DEV)net_dev);

			RT_MOD_DEC_USE_COUNT();
		}
	}

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,10)
	/*
		reference to http://vovo2000.com/type-lab/linux/kernel-api/linux-kernel-api.html
		enable device to generate PME# when suspended
		pci_choose_state(): Choose the power state of a PCI device to be suspended
	*/
	retval = pci_enable_wake(pci_dev, pci_choose_state(pci_dev, state), 1);
	/* save the PCI configuration space of a device before suspending */
	pci_save_state(pci_dev);
	/* disable PCI device after use */
	pci_disable_device(pci_dev);

	retval = pci_set_power_state(pci_dev, pci_choose_state(pci_dev, state));
#endif

	DBGPRINT(RT_DEBUG_TRACE, ("<===%s()\n", __FUNCTION__));
	return retval;
}


static int rt_pci_resume(struct pci_dev *pci_dev)
{
	struct net_device *net_dev = pci_get_drvdata(pci_dev);
	VOID *pAd = NULL;

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,10)
	INT32 retval;


	/*
		Set the power state of a PCI device

		PCI has 4 power states, DO (normal) ~ D3(less power)
		you can find that in include/linux/pci.h
		#define PCI_D0          ((pci_power_t __force) 0)
		#define PCI_D1          ((pci_power_t __force) 1)
		#define PCI_D2          ((pci_power_t __force) 2)
		#define PCI_D3hot       ((pci_power_t __force) 3)
		#define PCI_D3cold      ((pci_power_t __force) 4)
		#define PCI_UNKNOWN     ((pci_power_t __force) 5)
		#define PCI_POWER_ERROR ((pci_power_t __force) -1)
	*/
	retval = pci_set_power_state(pci_dev, PCI_D0);

	/* restore the saved state of a PCI device */
	pci_restore_state(pci_dev);

	/* initialize device before it's used by a driver */
	if (pci_enable_device(pci_dev))
	{
		printk("pci enable fail!\n");
		return 0;
	}
#endif

	DBGPRINT(RT_DEBUG_TRACE, ("===>%s()\n", __FUNCTION__));

	if (net_dev == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("net_dev == NULL!\n"));
	}
	else
		GET_PAD_FROM_NET_DEV(pAd, net_dev);

	if (pAd != NULL)
	{
		ULONG IfNum;

		/*
			we can not use IFF_UP because ra0 down but ra1 up
			and 1 suspend/resume function for 1 module, not for each interface
			so Linux will call suspend/resume function once
		*/
		RTMP_DRIVER_VIRTUAL_INF_NUM_GET(pAd, &IfNum);
		if (IfNum > 0)
/*		if (VIRTUAL_IF_NUM(pAd) > 0) */
		{
			/* mark device as attached from system and restart if needed */
			netif_device_attach(net_dev);

			if (rt28xx_open((PNET_DEV)net_dev) != 0)
			{
				/* open fail */
				DBGPRINT(RT_DEBUG_TRACE, ("<===%s()\n", __FUNCTION__));
				return 0;
			}

			/* increase MODULE use count */
			RT_MOD_INC_USE_COUNT();

/*			RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS); */
/*			RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF); */
			RTMP_DRIVER_PCI_RESUME(pAd);

			netif_start_queue(net_dev);
			netif_carrier_on(net_dev);
			netif_wake_queue(net_dev);
		}
	}

	DBGPRINT(RT_DEBUG_TRACE, ("<=== %s()\n", __FUNCTION__));
	return 0;
}
#endif /* CONFIG_PM */
#endif


/*
	PCI device probe & initialization function
*/
static int DEVINIT rt_pci_probe(struct pci_dev *pdev, const struct pci_device_id *pci_id)
{
	void *pAd = NULL, *handle;
	struct net_device *net_dev;
	const char *print_name;
	unsigned long csr_addr;
	int rv = 0;
	RTMP_OS_NETDEV_OP_HOOK netDevHook;
	unsigned long OpMode;
#ifdef DBG
    ULONG start, end, diff_ms;
    /* Get the current time for calculating startup time */
    NdisGetSystemUpTime(&start);
#endif /* DBG */	

	DBGPRINT(RT_DEBUG_TRACE, ("===> %s()\n", __FUNCTION__));

/*PCIDevInit============================================== */
	/* wake up and enable device */
	if ((rv = pci_enable_device(pdev))!= 0)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Enable PCI device failed, errno=%d!\n", rv));
		return rv;
	}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0)
	print_name = (char *) pci_name(pdev);
#else
	print_name = pdev->slot_name;
#endif /* LINUX_VERSION_CODE */

	if ((rv = pci_request_regions(pdev, print_name)) != 0)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Request PCI resource failed, errno=%d!\n", rv));
		goto err_out;
	}
	
	/* map physical address to virtual address for accessing register */
	csr_addr = (unsigned long) ioremap(pci_resource_start(pdev, 0), pci_resource_len(pdev, 0));
	if (!csr_addr)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("ioremap failed for device %s, region 0x%lX @ 0x%lX\n",
					print_name, (ULONG)pci_resource_len(pdev, 0), (ULONG)pci_resource_start(pdev, 0)));
		goto err_out_free_res;
	}
	else
	{
		DBGPRINT(RT_DEBUG_TRACE, ("%s: at 0x%lx, VA 0x%lx, IRQ %d. \n",  print_name, 
					(ULONG)pci_resource_start(pdev, 0), (ULONG)csr_addr, pdev->irq));
	}

#ifdef RT_SECURE_DMA
	/* Map secure memory area to our pci_dev, for future dma buffer memory allocations */
	/* The numbers here will need to be changed to match the platform. Currently 16MBytes
	 * are allocated at address 256Mbyte
	 */

	/* Example of code for ST platform */

	DBGPRINT(RT_DEBUG_OFF, ("%s(): RT_SECURE_DMA prepare\n", __FUNCTION__));

	if( ! dma_declare_coherent_memory(&(pdev->dev), 0x10000000UL, 0x10000000UL, 0x01000000UL,
									  DMA_MEMORY_MAP | DMA_MEMORY_EXCLUSIVE ) )
	{
		DBGPRINT(RT_DEBUG_OFF, ("%s(): dma_declare_coherent_memory failed!\n", __FUNCTION__));
		goto err_out_iounmap;
	}

#endif
	/* Set DMA master */
	pci_set_master(pdev);

	/* Allocate RTMP_ADAPTER adapter structure */
	os_alloc_mem(NULL, (UCHAR **)&handle, sizeof(struct os_cookie));
	if (handle == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s(): Allocate memory for os handle failed!\n", __FUNCTION__));
		goto err_out_iounmap;
	}
	memset(handle, 0, sizeof(struct os_cookie));

	((POS_COOKIE)handle)->pci_dev = pdev;

#ifdef OS_ABL_FUNC_SUPPORT
{
	RTMP_PCI_CONFIG PciConfig;
	PciConfig.ConfigVendorID = PCI_VENDOR_ID;
	/* get DRIVER operations */
	RTMP_DRV_OPS_FUNCTION(pRtmpDrvOps, NULL, &PciConfig, NULL);
}
#endif /* OS_ABL_FUNC_SUPPORT */

	rv = RTMPAllocAdapterBlock(handle, &pAd);	/* we may need the pdev for allocate structure of "RTMP_ADAPTER" */
	if (rv != NDIS_STATUS_SUCCESS) 
		goto err_out_iounmap;

#ifdef MULTI_INF_SUPPORT
	if (pAdGlobalList[0] == NULL)
		pAdGlobalList[0] = pAd;
	else if (pAdGlobalList[1] == NULL) 	
		pAdGlobalList[1] = pAd;
	else
		DBGPRINT(RT_DEBUG_ERROR, ("%s(): pAdGlobalList assign Error !\n", __FUNCTION__));	
#endif /* MULTI_INF_SUPPORT */

	RTMP_DRIVER_PCI_CSR_SET(pAd, csr_addr);

	RTMP_DRIVER_PCIE_INIT(pAd, pdev);

/*NetDevInit============================================== */
	net_dev = RtmpPhyNetDevInit(pAd, &netDevHook);
	if (net_dev == NULL)
		goto err_out_free_radev;
	
	/* Here are the net_device structure with pci-bus specific parameters. */
	net_dev->irq = pdev->irq;		/* Interrupt IRQ number */
	net_dev->base_addr = csr_addr;		/* Save CSR virtual address and irq to device structure */
	pci_set_drvdata(pdev, net_dev);	/* Set driver data */
	
#ifdef NATIVE_WPA_SUPPLICANT_SUPPORT
/* for supporting Network Manager */
	/*
		Set the sysfs physical device reference for the network logical device
		if set prior to registration will cause a symlink during initialization.
	*/
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
	SET_NETDEV_DEV(net_dev, &(pdev->dev));
#endif
#endif /* NATIVE_WPA_SUPPLICANT_SUPPORT */

/*All done, it's time to register the net device to linux kernel. */
	/* Register this device */
#ifdef RT_CFG80211_SUPPORT
{
/*	pAd->pCfgDev = &(pdev->dev); */
/*	pAd->CFG80211_Register = CFG80211_Register; */
/*	RTMP_DRIVER_CFG80211_INIT(pAd, pdev); */

	/*
		In 2.6.32, cfg80211 register must be before register_netdevice();
		We can not put the register in rt28xx_open();
		Or you will suffer NULL pointer in list_add of
		cfg80211_netdev_notifier_call().
	*/
	CFG80211_Register(pAd, &(pdev->dev), net_dev);
}
#endif /* RT_CFG80211_SUPPORT */

	RTMP_DRIVER_OP_MODE_GET(pAd, &OpMode);
	rv = RtmpOSNetDevAttach(OpMode, net_dev, &netDevHook);
	if (rv)
		goto err_out_free_netdev;

#ifdef CONFIG_STA_SUPPORT
/*	pAd->StaCfg.OriDevType = net_dev->type; */
	RTMP_DRIVER_STA_DEV_TYPE_SET(pAd, net_dev->type);
#endif /* CONFIG_STA_SUPPORT */

#ifdef PRE_ASSIGN_MAC_ADDR
{
	UCHAR mac_addr[MAC_ADDR_LEN];
	RTMP_DRIVER_MAC_ADDR_GET(pAd, &mac_addr[0]);
	DBGPRINT(RT_DEBUG_TRACE, ("@%s MAC address: %02x:%02x:%02x:%02x:%02x:%02x\n",
				__FUNCTION__, mac_addr[0], mac_addr[1],
				mac_addr[2], mac_addr[3],
				mac_addr[4], mac_addr[5]));
	/* Set up the Mac address */
	RtmpOSNetDevAddrSet(OpMode, net_dev, &mac_addr[0], NULL);
}
#endif /* PRE_ASSIGN_MAC_ADDR */

#ifdef DBG				
    /* Get the current time for calculating startup time */
    NdisGetSystemUpTime(&end); diff_ms = (end-start)*1000/OS_HZ;
    DBGPRINT(RT_DEBUG_OFF, ("WiFi PCI Startup Time: %lu.%03lus\n",diff_ms/1000,diff_ms%1000));
#endif /* DBG */

	DBGPRINT(RT_DEBUG_TRACE, ("<=== %s()\n", __FUNCTION__));

	return 0; /* probe ok */


	/* --------------------------- ERROR HANDLE --------------------------- */
err_out_free_netdev:
	RtmpOSNetDevFree(net_dev);
	
err_out_free_radev:
	/* free RTMP_ADAPTER strcuture and os_cookie*/
	RTMPFreeAdapter(pAd);

err_out_iounmap:
	iounmap((void *)(csr_addr));
	release_mem_region(pci_resource_start(pdev, 0), pci_resource_len(pdev, 0)); 
	
err_out_free_res:
	pci_release_regions(pdev);
	
err_out:
	pci_disable_device(pdev);

	DBGPRINT(RT_DEBUG_ERROR, ("<=== %s() failed with rv = %d!\n",
				__FUNCTION__, rv));

	return -ENODEV; /* probe fail */
}


static VOID DEVEXIT rt_pci_remove(struct pci_dev *pci_dev)
{
	PNET_DEV net_dev = pci_get_drvdata(pci_dev);
	VOID *pAd = NULL;
	ULONG csr_addr = net_dev->base_addr;


	GET_PAD_FROM_NET_DEV(pAd, net_dev);
	
	DBGPRINT(RT_DEBUG_TRACE, ("===> %s()\n", __FUNCTION__));

	if (pAd != NULL)
	{
		/* Unregister/Free all allocated net_device. */
		RtmpPhyNetDevExit(pAd, net_dev);

#ifdef RT_CFG80211_SUPPORT
		RTMP_DRIVER_80211_UNREGISTER(pAd, net_dev);
#endif /* RT_CFG80211_SUPPORT */

		/* Free RTMP_ADAPTER related structures. */
		RtmpRaDevCtrlExit(pAd);
		
		/* Unmap CSR base address */
		iounmap((char *)(csr_addr));
		
		/* release memory region */
		release_mem_region(pci_resource_start(pci_dev, 0), pci_resource_len(pci_dev, 0));
	}
	else
	{
		/* Unregister network device */
		RtmpOSNetDevDetach(net_dev);

		/* Unmap CSR base address */
		iounmap((char *)(net_dev->base_addr));

		/* release memory region */
		release_mem_region(pci_resource_start(pci_dev, 0), pci_resource_len(pci_dev, 0));
	}

#ifdef RT_SECURE_DMA
	dma_release_declared_memory(&(pci_dev->dev));
#endif
	/* Free the root net_device */
	RtmpOSNetDevFree(net_dev);
}


/*
	Our PCI driver structure
*/
static struct pci_driver rt_pci_driver =
{
	name: RTMP_DRV_NAME,
	id_table: rt_pci_tbl,
	probe: rt_pci_probe,
#if LINUX_VERSION_CODE >= 0x20412
	remove: DEVEXIT_P(rt_pci_remove),
#else
	remove: __devexit(rt_pci_remove),
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0)
#ifdef CONFIG_PM
	suspend:	rt_pci_suspend,
	resume: rt_pci_resume,
#endif
#endif
};


/*
	Driver module load/unload function
*/
int __init rt_pci_init_module(void)
{
	DBGPRINT(RT_DEBUG_ERROR, ("register %s\n", RTMP_DRV_NAME));

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0)
	return pci_register_driver(&rt_pci_driver);
#else
	return pci_module_init(&rt_pci_driver);
#endif
}


void __exit rt_pci_cleanup_module(void)
{
	pci_unregister_driver(&rt_pci_driver);
}

#ifndef MULTI_INF_SUPPORT
module_init(rt_pci_init_module);
module_exit(rt_pci_cleanup_module);

#ifdef CONFIG_STA_SUPPORT
#ifdef MODULE_VERSION
MODULE_VERSION(STA_DRIVER_VERSION);
#endif
#endif /* CONFIG_STA_SUPPORT */

#endif /* MULTI_INF_SUPPORT */

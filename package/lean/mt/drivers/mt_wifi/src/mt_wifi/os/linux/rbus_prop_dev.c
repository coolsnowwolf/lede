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
    rbus_prop_dev.c

    Abstract:
    Create and register network interface for RBUS based chipsets in linux platform.

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
*/
#ifdef RTMP_RBUS_SUPPORT
#define RTMP_MODULE_OS

#include "rt_config.h"

#if defined(CONFIG_RA_CLASSIFIER) && (!defined(CONFIG_RA_CLASSIFIER_MODULE))
extern int (*ra_classifier_init_func)(void);
extern void (*ra_classifier_release_func)(void);
extern struct proc_dir_entry *proc_ptr, *proc_ralink_wl_video;
#endif

#ifdef MEM_ALLOC_INFO_SUPPORT
extern MEM_INFO_LIST MemInfoList;
extern MEM_INFO_LIST PktInfoList;
#endif /*MEM_ALLOC_INFO_SUPPORT*/

static struct pci_device_id mt_rbus_tbl[] DEVINITDATA = {

#ifdef MT7622
	{PCI_DEVICE(0x14c3, 0x7622)},
#endif /* MT7622 */
	{} /* terminate list */
};

MODULE_DEVICE_TABLE(pci, mt_rbus_tbl);


#define RBUS_TSSI_CTRL_OFFSET 0x34
#define RBUS_PA_LNA_CTRL_OFFSET 0x38

int rbus_tssi_set(struct _RTMP_ADAPTER *ad, UCHAR mode)
{
	struct pci_dev *dev = ((POS_COOKIE)ad->OS_Cookie)->pci_dev;

	pci_write_config_byte(dev, RBUS_TSSI_CTRL_OFFSET, mode);
	return 0;
}

int rbus_pa_lna_set(struct _RTMP_ADAPTER *ad, UINT32 mode)
{
	struct pci_dev *dev = ((POS_COOKIE)ad->OS_Cookie)->pci_dev;

	pci_write_config_dword(dev, RBUS_PA_LNA_CTRL_OFFSET, mode);
	return 0;
}

static int DEVINIT mt_rbus_probe(struct pci_dev *pdev, const struct pci_device_id *pci_id)
{
	struct net_device *net_dev;
	ULONG csr_addr;
	INT rv;
	void *handle = NULL;
	RTMP_ADAPTER *pAd;
	RTMP_OS_NETDEV_OP_HOOK netDevHook;
	UINT32 Value;

	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE, ("===> rt2880_probe\n"));
#ifdef MEM_ALLOC_INFO_SUPPORT
	MemInfoListInital();
#endif /* MEM_ALLOC_INFO_SUPPORT */
	/* map physical address to virtual address for accessing register */
	csr_addr = (unsigned long)ioremap(pci_resource_start(pdev, 0), pci_resource_len(pdev, 0));
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
	/*is not a regular method*/
	((POS_COOKIE)handle)->pci_dev = (VOID *)pdev;
	((POS_COOKIE)handle)->pDev = &pdev->dev;
	RtmpRaDevCtrlInit(pAd, RTMP_DEV_INF_RBUS);
	net_dev = RtmpPhyNetDevInit(pAd, &netDevHook);

	if (net_dev == NULL)
		goto err_out_free_radev;

	/*assign net_dev as pdev's privdate*/
	pci_set_drvdata(pdev, net_dev);
	/* Here are the net_device structure with pci-bus specific parameters. */
	net_dev->irq = pdev->irq;			/* Interrupt IRQ number */
	net_dev->base_addr = csr_addr;		/* Save CSR virtual address and irq to device structure */

	RTMP_DRIVER_CHIP_PREPARE(pAd);
	/*All done, it's time to register the net device to kernel. */
	/* Register this device */
	rv = RtmpOSNetDevAttach(pAd->OpMode, net_dev, &netDevHook);

	if (rv) {
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR, ("failed to call RtmpOSNetDevAttach(), rv=%d!\n", rv));
		goto err_out_free_netdev;
	}
	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE, ("%s: at CSR addr 0x%lx, IRQ %ld.\n", net_dev->name, (ULONG)csr_addr, (long int)net_dev->irq));
	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE, ("<=== %s()\n", __func__));
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

static VOID DEVEXIT mt_rbus_remove(struct pci_dev *pci_dev)
{
	struct net_device *net_dev = pci_get_drvdata(pci_dev);
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

/*
 *	Our PCI driver structure
 */
static struct pci_driver mt_rbus_driver = {
name:
	"mt_rbus",
id_table :
	mt_rbus_tbl,
probe :
	mt_rbus_probe,
remove :
	DEVEXIT_P(mt_rbus_remove),
};


/*
 *	Driver module load/unload function
 */
int __init wbsys_module_init(void)
{
	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR, ("register %s\n", RTMP_DRV_NAME));
#ifdef MEM_ALLOC_INFO_SUPPORT
	MemInfoListInital();
#endif /* MEM_ALLOC_INFO_SUPPORT */
	return pci_register_driver(&mt_rbus_driver);

}


void __exit wbsys_module_exit(void)
{
	pci_unregister_driver(&mt_rbus_driver);
}

/** @} */
/** @} */

#ifndef MULTI_INF_SUPPORT
module_init(wbsys_module_init);
module_exit(wbsys_module_exit);
#endif /* MULTI_INF_SUPPORT */

#endif /* RTMP_RBUS_SUPPORT */


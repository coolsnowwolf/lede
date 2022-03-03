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

#ifdef RT_CFG80211_SUPPORT
#include <linux/fs.h>           /* chrdev allocation */
#include <linux/cdev.h>

struct file_operations dev_reg_fops = {
           .open = NULL,
           .release = NULL,
};

#define DEVNUM_NAME "cfg_rbus"	
#define DEVNUM_MINOR_START 0
#define DEVNUM_COUNT 1

static dev_t dev;
static struct device *dev_ret;
static struct cdev dev_reg_cdev;
static struct class *cl;

#endif /* RT_CFG80211_SUPPORT */
static struct net_device *rt2880_dev = NULL;

#if defined(CONFIG_RA_CLASSIFIER)&&(!defined(CONFIG_RA_CLASSIFIER_MODULE)) 	 
extern int (*ra_classifier_init_func) (void) ; 	 
extern void (*ra_classifier_release_func) (void) ; 	 
extern struct proc_dir_entry *proc_ptr, *proc_ralink_wl_video;	 
#endif

#ifdef MULTI_INF_SUPPORT
/* Index 0 for 2.4G, 1 for 5Ghz Card */
extern VOID* pAdGlobalList[2];
#endif /* MULTI_INF_SUPPORT */

int __init rt2880_module_init(void)
{
	struct net_device *net_dev;
	ULONG csr_addr;
	INT rv;
	PVOID *handle = NULL;
	RTMP_ADAPTER *pAd;
	unsigned int dev_irq;
	RTMP_OS_NETDEV_OP_HOOK netDevHook;
#ifdef DBG
    ULONG start, end, diff_ms;
    /* Get the current time for calculating startup time */
    NdisGetSystemUpTime(&start);
#endif /* DBG */

	DBGPRINT(RT_DEBUG_ERROR, ("===> rt2880_probe\n")); 

/*RtmpRaBusInit============================================ */
	/* map physical address to virtual address for accessing register */
	csr_addr = (unsigned long)RTMP_MAC_CSR_ADDR;
	dev_irq = RTMP_MAC_IRQ_NUM;

/*RtmpDevInit============================================== */
	/* Allocate RTMP_ADAPTER adapter structure */
/*	handle = kmalloc(sizeof(struct os_cookie) , GFP_KERNEL); */
	os_alloc_mem(NULL, (UCHAR **)&handle, sizeof(struct os_cookie));
	if (!handle)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Allocate memory for os_cookie failed!\n"));
		goto err_out;
	}
	NdisZeroMemory(handle, sizeof(struct os_cookie));

#ifdef OS_ABL_FUNC_SUPPORT
	/* get DRIVER operations */
	RTMP_DRV_OPS_FUNCTION(pRtmpDrvOps, NULL, NULL, NULL);
#endif /* OS_ABL_FUNC_SUPPORT */

	rv = RTMPAllocAdapterBlock(handle, (VOID **)&pAd);
	if (rv != NDIS_STATUS_SUCCESS)
	{
		DBGPRINT(RT_DEBUG_ERROR, (" RTMPAllocAdapterBlock !=  NDIS_STATUS_SUCCESS\n"));
/*		kfree(handle); */
		os_free_mem(NULL, handle);
		
		goto err_out;
	}

#ifdef MULTI_INF_SUPPORT
	if (pAdGlobalList[0] == NULL)
		pAdGlobalList[0] = pAd;
	else if (pAdGlobalList[1] == NULL) 	
		pAdGlobalList[1] = pAd;
	else
		DBGPRINT(RT_DEBUG_ERROR, ("%s(): pAdGlobalList assign Error !\n", __FUNCTION__));	
#endif /* MULTI_INF_SUPPORT */


	/* Here are the RTMP_ADAPTER structure with rbus-bus specific parameters. */
	pAd->CSRBaseAddress = (PUCHAR)csr_addr;

	RtmpRaDevCtrlInit(pAd, RTMP_DEV_INF_RBUS);


/*NetDevInit============================================== */
	net_dev = RtmpPhyNetDevInit(pAd, &netDevHook);
	if (net_dev == NULL)
		goto err_out_free_radev;

	/* Here are the net_device structure with pci-bus specific parameters. */
	net_dev->irq = dev_irq;			/* Interrupt IRQ number */
	net_dev->base_addr = csr_addr;		/* Save CSR virtual address and irq to device structure */
	((POS_COOKIE)handle)->pci_dev = NULL;

#ifdef CONFIG_STA_SUPPORT
    pAd->StaCfg.OriDevType = net_dev->type;
#endif /* CONFIG_STA_SUPPORT */


#ifdef RT_CFG80211_SUPPORT
	/*
		In 2.6.32, cfg80211 register must be before register_netdevice();
		We can not put the register in rt28xx_open();
		Or you will suffer NULL pointer in list_add of
		cfg80211_netdev_notifier_call().
	*/

	/* register a dummy char device for CFG80211_Register because RBUS don't have a real device */

	INT alloc_ret=0,cdev_err=0;
	static unsigned int dev_reg_major = 0;
	alloc_ret = alloc_chrdev_region(&dev, DEVNUM_MINOR_START, DEVNUM_COUNT, DEVNUM_NAME);
	if(alloc_ret){	         
	         DBGPRINT(RT_DEBUG_OFF, ("%s : could not allocate device\n", __func__));
	         goto err_out_free_radev;
	}
	else{
	         DBGPRINT(RT_DEBUG_OFF, ("%s : registered char dev with major number:%i, minor number:%i\n",
	                    __func__, MAJOR(dev), MINOR(dev)));
	         dev_reg_major = MAJOR(dev);
	}         
	cl = class_create(THIS_MODULE, "chardrv");
	dev_ret = device_create(cl, NULL, dev, NULL, "mynull");
	
	cdev_init(&dev_reg_cdev, &dev_reg_fops);
	dev_reg_cdev.owner = THIS_MODULE;	

	cdev_err = cdev_add(&dev_reg_cdev, MKDEV(dev_reg_major, 0), DEVNUM_COUNT);
	if(cdev_err){
		DBGPRINT(RT_DEBUG_OFF, ("%s : could not add cdev\n", __func__));
		goto err_out_free_radev;
	}
	if(dev_ret == NULL)
	{
		DBGPRINT(RT_DEBUG_OFF, ("%s : dev_ret is NULL!!!\n", __func__));
		goto err_out_free_radev;
	}
    /* register a dummy char device done */
    
	CFG80211_Register(pAd, dev_ret , net_dev);
		
#endif /* RT_CFG80211_SUPPORT */	


	/*All done, it's time to register the net device to kernel. */
	/* Register this device */
	rv = RtmpOSNetDevAttach(pAd->OpMode, net_dev, &netDevHook);
	if (rv)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("failed to call RtmpOSNetDevAttach(), rv=%d!\n", rv));
		goto err_out_free_netdev;
	}
		

	/* due to we didn't have any hook point when do module remove, we use this static as our hook point. */
	rt2880_dev = net_dev;
	
	wl_proc_init();

	DBGPRINT(RT_DEBUG_ERROR, ("%s: at CSR addr 0x%lx, IRQ %u. \n", net_dev->name, (ULONG)csr_addr, net_dev->irq));

#ifdef DBG				
    /* Get the current time for calculating startup time */
    NdisGetSystemUpTime(&end); diff_ms = (end-start)*1000/OS_HZ;
    DBGPRINT(RT_DEBUG_OFF, ("WiFi RBUS Startup Time: %lu.%03lus\n",diff_ms/1000,diff_ms%1000));
#endif /* DBG */

	DBGPRINT(RT_DEBUG_ERROR, ("<=== rt2880_probe\n"));

#if defined(CONFIG_RA_CLASSIFIER)&&(!defined(CONFIG_RA_CLASSIFIER_MODULE)) 	 
	proc_ptr = proc_ralink_wl_video;
	if(ra_classifier_init_func!=NULL)
		ra_classifier_init_func();
#endif

	return 0;

err_out_free_netdev:
	RtmpOSNetDevFree(net_dev);

err_out_free_radev:
	/* free RTMP_ADAPTER strcuture and os_cookie*/
	RTMPFreeAdapter(pAd);
		
err_out:
	return -ENODEV;
	
}


VOID __exit rt2880_module_exit(void)
{
	struct net_device *net_dev = rt2880_dev;
	RTMP_ADAPTER *pAd;


	if (net_dev == NULL)
		return;
	
	/* pAd = net_dev->priv; */
	GET_PAD_FROM_NET_DEV(pAd, net_dev);

	if (pAd != NULL)
	{
		RtmpPhyNetDevExit(pAd, net_dev);
		RtmpRaDevCtrlExit(pAd);
	}
	else
	{
		RtmpOSNetDevDetach(net_dev);
	}
	
	/* Free the root net_device. */
	RtmpOSNetDevFree(net_dev);
#ifdef RT_CFG80211_SUPPORT
	cdev_del(&dev_reg_cdev);
	device_destroy(cl, dev);
	class_destroy(cl);
	unregister_chrdev_region(dev, 1);
#endif /* RT_CFG80211_SUPPORT */
	
#if defined(CONFIG_RA_CLASSIFIER)&&(!defined(CONFIG_RA_CLASSIFIER_MODULE))
    proc_ptr = proc_ralink_wl_video; 	 
    if(ra_classifier_release_func!=NULL) 	 
	    ra_classifier_release_func(); 	 
#endif

	wl_proc_exit();
}


#ifndef MULTI_INF_SUPPORT
module_init(rt2880_module_init);
module_exit(rt2880_module_exit);
#endif /* MULTI_INF_SUPPORT */

#endif /* RTMP_RBUS_SUPPORT */


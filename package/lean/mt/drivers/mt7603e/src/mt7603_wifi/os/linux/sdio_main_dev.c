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
    sdio_main_dev.c

    Abstract:
    Create and register network interface for SDIO based chipsets in Linux platform.

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
*/

#define RTMP_MODULE_OS

#include "rtmp_comm.h"
#include "rt_os_util.h"
#include "rt_os_net.h"
#include "rt_config.h"


/***************************************************************************
 *
 *	SDIO device initialization related procedures.
 *
 ***************************************************************************/



static int DEVINIT rt_sdio_probe (struct sdio_func *func,const struct sdio_device_id *id)
{

   INT32 ret = 0;
	VOID *pAd = NULL, *handle = NULL;
	struct net_device *net_dev;
//	RTMP_STRING *print_name;
   INT32 rv = 0;
	RTMP_OS_NETDEV_OP_HOOK netDevHook;
	unsigned long OpMode;
   INT32 i = 0;

   ASSERT(func);
   ASSERT(id);

	DBGPRINT(RT_DEBUG_ERROR, ("SDIO probe ! register %s\n", RTMP_DRV_NAME));

	os_alloc_mem(NULL, (UCHAR **)&handle, sizeof(struct os_cookie));
	if (handle == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s(): Allocate memory for os handle failed!\n", __FUNCTION__));
		goto err;
	}
	memset(handle, 0, sizeof(struct os_cookie));
	((POS_COOKIE)handle)->sdio_dev = func;

   //handle->func = func;

   /*SDIO Init============================================== */

   for (i = 0; i < func->card->num_info; i++) {
       //printk(KERN_INFO DRV_NAME "info[%d]: %s\n", i, func->card->info[i]);
   }

   sdio_claim_host(func);
   ret = sdio_enable_func(func);
   sdio_release_host(func);
   if (ret) {
       //printk(KERN_INFO DRV_NAME"sdio_enable_func failed!\n");
       goto err;
   }

   sdio_claim_host(func);
   ret = sdio_set_block_size(func, 512);
   sdio_release_host(func);
   if (ret) {
       DBGPRINT(RT_DEBUG_ERROR, ("%s():sdio_set_block_size failed!\n",__FUNCTION__));
       goto err;
   }   

	/* Allocate RTMP_ADAPTER adapter structure ========================== */

	ret = RTMPAllocAdapterBlock(handle, &pAd);
	if (ret != NDIS_STATUS_SUCCESS) 
		goto err;
   
	RTMP_DRIVER_SDIO_INIT(pAd);

//   
   /*NetDevInit============================================== */
      net_dev = RtmpPhyNetDevInit(pAd, &netDevHook);
      if (net_dev == NULL)
      goto err_out_free_radev;
   
   /*SDIO interrupt init============================================== */

   sdio_claim_host(func);
   ret = sdio_claim_irq(func, rt_sdio_interrupt); /* Interrupt IRQ handler */
   sdio_release_host(func);
   if (ret) {
       DBGPRINT(RT_DEBUG_ERROR, ("%s():rt_sdio_interrupt failed!\n",__FUNCTION__));
       goto err_out_free_netdev;
   }
      
    DBGPRINT(RT_DEBUG_ERROR, ("SDIO probe finsih %s\n", RTMP_DRV_NAME));
   
      sdio_set_drvdata(func, net_dev);    /* Set driver data */

 #if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
      SET_NETDEV_DEV(net_dev, &(func->dev));
#endif

/*All done, it's time to register the net device to linux kernel. */
	/* Register this device */

	RTMP_DRIVER_OP_MODE_GET(pAd, &OpMode);
	rv = RtmpOSNetDevAttach(OpMode, net_dev, &netDevHook);
	if (rv) {
      DBGPRINT(RT_DEBUG_ERROR,("%s():DevAttach failed!\n", __FUNCTION__));
		goto err_out_free_netdev;
}

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

#ifdef EXT_BUILD_CHANNEL_LIST
   RTMP_DRIVER_SET_PRECONFIG_VALUE(pAd);
#endif /* EXT_BUILD_CHANNEL_LIST */

   return ret;


	/* --------------------------- ERROR HANDLE --------------------------- */	
   DBGPRINT(RT_DEBUG_ERROR, ("%s erro happen\n",__FUNCTION__));
err_out_free_netdev:
	RtmpOSNetDevFree(net_dev);

err_out_free_radev:
	RTMPFreeAdapter(pAd);

err:
   if (handle)
      os_free_mem(NULL, handle);

   return;
	}


static void rt_sdio_remove (struct sdio_func *func)
	{
   INT32 ret = 0;
   VOID* pAd;
	struct net_device *net_dev = sdio_get_drvdata(func);
	
   GET_PAD_FROM_NET_DEV(pAd, net_dev);

   /*NetDev de-Init============================================== */

 	if (pAd != NULL)
	{
		/* Unregister/Free all allocated net_device. */
		RtmpPhyNetDevExit(pAd, net_dev);


		/* Free RTMP_ADAPTER related structures. */
		RtmpRaDevCtrlExit(pAd);
	}
	else
	{
		/* Unregister network device */
		RtmpOSNetDevDetach(net_dev);
	}
   sdio_claim_host(func);
   ret = sdio_release_irq(func);
   if (ret != NDIS_STATUS_SUCCESS) 
{
      DBGPRINT(RT_DEBUG_ERROR, ("sdio_release_irq failure\n"));
}
   sdio_release_host(func);

	/* Free the root net_device */
	RtmpOSNetDevFree(net_dev);  

	

   sdio_claim_host(func);
	
   ret =  sdio_disable_func(func);
   if (ret != NDIS_STATUS_SUCCESS) 
	{
      DBGPRINT(RT_DEBUG_ERROR, ("sdio_disable_func failure\n"));
}
   sdio_release_host(func);


	
}

/*
	Our SDIO driver structure
*/
static const struct sdio_device_id rt_sdio_id_tbl[] = {      
{ SDIO_DEVICE(0x037a, 0x7606) }, /* SDIO1:WIFI MT7636 */       
   { /* end: all zeroes */ },
};


MODULE_DEVICE_TABLE(sdio, rt_sdio_id_tbl);

static struct sdio_driver rt_sdio_drv = {    
   .name = "rt_sdio_driver", /* MTK SDIO Client Driver */    
   .id_table = rt_sdio_id_tbl, /* all supported struct sdio_device_id table */    
	.probe = rt_sdio_probe,
	.remove = rt_sdio_remove,
   /*
   #if HIF_SDIO_SUPPORT_SUSPEND    
   .drv = {       .pm = &mtk_sdio_pmops,    },
   #endif
   */
};

/*
	Driver module load/unload function
*/
int __init rt_sdio_init_module(void)
{
	int ret;

	DBGPRINT(RT_DEBUG_ERROR, ("%s SDIO !!!! register\n", __FUNCTION__));
   ret = sdio_register_driver(&rt_sdio_drv);
//	ret = sdio_register_driver(&rt_sdio_driver);

	return ret;
}


void __exit rt_sdio_cleanup_module(void)
{
//	sdio_unregister_driver(&rt_sdio_driver);
   sdio_unregister_driver(&rt_sdio_drv);

}


#ifndef MULTI_INF_SUPPORT
module_init(rt_sdio_init_module);
module_exit(rt_sdio_cleanup_module);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("MediaTek Inc");
MODULE_DESCRIPTION("MediaTek SDIO Driver");
#endif /* MULTI_INF_SUPPORT */


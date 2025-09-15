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
    rt_pci_rbus.c

    Abstract:
    Create and register network interface.

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
*/

#define RTMP_MODULE_OS

/*#include <rt_config.h> */
#include "rtmp_comm.h"
#include "rt_os_util.h"
#include "rt_os_net.h"
#include <linux/pci.h>


IRQ_HANDLE_TYPE
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,19))
rt2860_interrupt(int irq, void *dev_instance);
#else
rt2860_interrupt(int irq, void *dev_instance, struct pt_regs *regs);
#endif


/***************************************************************************
  *
  *	Interface-depended memory allocation/Free related procedures.
  *		Mainly for Hardware TxDesc/RxDesc/MgmtDesc, DMA Memory for TxData/RxData, etc.,
  *
  **************************************************************************/


/***************************************************************************
  *
  *	interrupt handler related procedures.
  *
  **************************************************************************/
/*int print_int_count; */

IRQ_HANDLE_TYPE
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,19))
rt2860_interrupt(int irq, void *dev_instance)
#else
rt2860_interrupt(int irq, void *dev_instance, struct pt_regs *regs)
#endif
{
	struct net_device *net_dev = (struct net_device *) dev_instance;
	VOID *pAd = NULL;
#ifdef MULTI_CORE_SUPPORT
	unsigned long flags=0;
#endif /* MULTI_CORE_SUPPORT */

	GET_PAD_FROM_NET_DEV(pAd, net_dev);

	RTMPHandleInterrupt(pAd);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0)
	return  IRQ_HANDLED;
#endif

}


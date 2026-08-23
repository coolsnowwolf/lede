/*
 ***************************************************************************
 * MediaTeK Inc.
 * 4F, No. 2 Technology	5th	Rd.
 * Science-based Industrial	Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2012, MTK.
 *
 * All rights reserved.	MediaTeK's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of MediaTeK Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of MediaTeK, Inc. is obtained.
 ***************************************************************************

	Module Name:
	epon_timer.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	jq.zhu		2012/9/26		Create
*/


#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>
#include <linux/signal.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/init.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/proc_fs.h>
#include <linux/dma-mapping.h>
#include <linux/mii.h>
#include <linux/version.h>
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,36)
#include <linux/pktflow.h>
#endif
#include <linux/in.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/icmp.h>
#include <linux/jhash.h>
#include <linux/random.h>
#include <linux/if_arp.h>
#include <linux/if_ether.h>
#include <linux/if_vlan.h>
#include <linux/if_pppox.h>
#include <linux/ppp_defs.h>
#include <asm/io.h>
#include <asm/tc3162/cmdparse.h>
#include "epon/epon.h"

extern __u32 eponDebugLevel ;


/*_____________________________________________________________________________
**      function name: eponMacAddTimer
**      descriptions:
**            
**
**      parameters:
**           N/A
**
**      global:
**           N/A
**
**      return:
**	     	 N/A
**      call:
**      	 N/A
**      revision:
**       jqzhu
**____________________________________________________________________________
*/
int eponMacAddTimer(struct timer_list *timer, __u32 interval, eponTimerCallback callback, unsigned long param){
	eponDbgPrint(EPON_DEBUG_LEVEL_TRACE, "\r\nenter eponMacAddTimer");
	if((timer == NULL) ||( callback == NULL) ){
		eponDbgPrint(EPON_DEBUG_LEVEL_TRACE, "\r\neponMacAddTimer param error");
		return -1;
	}
	init_timer(timer);
	timer->expires = jiffies + (HZ / interval);
	timer->function = callback;
	timer->data = param;
	add_timer(timer);
	return 0;
}

int eponMacDelTimer(struct timer_list *timer){
	  return del_timer_sync(timer);
}


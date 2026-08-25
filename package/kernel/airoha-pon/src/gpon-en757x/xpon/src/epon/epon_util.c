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
	epon_util.c

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
#include <net/ip.h>
#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_helper.h>
#include <asm/io.h>
#include <asm/tc3162/cmdparse.h>

#include "epon/epon.h"

extern __u32 eponDebugLevel ;

/*_____________________________________________________________________________
**      function name: 
**      descriptions:
**            Show debug message in kernel mode.
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
static __u8 msg[256];

void  eponDbgPrint(__u32 debugLevel, char *fmt,...){
		va_list ap;
	
	if (eponDebugLevel >= debugLevel)
	{
		va_start(ap, fmt);
		
		vsnprintf(msg, 256, fmt, ap);	
		printk(msg);
		printk("[%d]", (uint)jiffies);
		//printk("\n");
	
	va_end(ap);
	}

	return ;
}




 int strToMacNum(__u8 *macstr, __u8 *macnum)
{
        int temp[6];
        int i;
        
        sscanf(macstr, "%02x:%02x:%02x:%02x:%02x:%02x", &temp[0],&temp[1],&temp[2],&temp[3],&temp[4],&temp[5]);
        for(i=0;i<6;i++){
                macnum[i] = (unsigned char)temp[i];
        }
        return 0;
}




 

#define ISDIGIT(x)	((x)>='0'&&(x)<='9')



 __u16
get16(
	__u8	*cp
)
{
	register __u16 x;

	x = *cp++;
	x <<= 8;
	x |= *cp;
	return x;
} /* get16 */

 __u32
get32 (
	__u8	*cp
)
{
	__u32 rval;

	rval = *cp++;
	rval <<= 8;
	rval |= *cp++;
	rval <<= 8;
	rval |= *cp++;
	rval <<= 8;
	rval |= *cp;

	return rval;
} /* get32 */

 __u8 *
put32(
	__u8	*cp,
	__u32	x
)
{
	*cp++ = x >> 24;
	*cp++ = x >> 16;
	*cp++ = x >> 8;
	*cp++ = x;
	return cp;
} /* put32 */

__u8 *
put16(
	__u8	*cp,
	__u16	x
)
{
	*cp++ = x >> 8;
	*cp++ = x;

	return cp;
} /* put16 */






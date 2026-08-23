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
	epon_hwtest.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	jq.zhu		2012/9/26		Create
*/



#ifdef EPON_MAC_HW_TEST


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
#include <asm/io.h>
#include <asm/tc3162/cmdparse.h>

#include "epon/epon.h"




//////////////register test///////////////////////////

__u32 eponRegTestPattern[] = {
	0xffffffff,
	0x00000000,
	0x55555555,
	0x33333333,
	0x69696969,
	0x5A5A5A5A,
	0xCCCCCCCC,
	0xAAAAAAAA,
};




extern eponMacHwtestReg_t *eponMacRegTable;


int eponRegPatternTest(__u32 pattern, eponMacHwtestReg_t *regTable_p,  __u32 maxIndex){
	int ret = 0;
	__u32 index = 0;
	__u32 value= 0;
	eponDbgPrint(EPON_DEBUG_LEVEL_TRACE, "pattern=0x%x", pattern);
	for(index = 0; index < maxIndex; index++ ){
		eponDbgPrint(EPON_DEBUG_LEVEL_TRACE, "\r\n eponRegPatternTest ! register addr= 0x%x , rwmask= 0x%x, pattern = 0x%x ,  value=0x%x" , regTable_p[index].addr, regTable_p[index].rwmask, pattern , value);
			
		WRITE_REG_WORD(regTable_p[index].addr , (pattern&regTable_p[index].rwmask));
		if(regTable_p[index].addr  == e_sync_time){
			//should PAUSE time
			READ_REG_WORD(e_glb_cfg);
			READ_REG_WORD(e_glb_cfg);
			READ_REG_WORD(e_glb_cfg);
			READ_REG_WORD(e_glb_cfg);
			READ_REG_WORD(e_glb_cfg);
		}
		value = READ_REG_WORD(regTable_p[index].addr);
		if((pattern&(regTable_p[index].rwmask)) != (value&(regTable_p[index].rwmask))){
			
			eponDbgPrint(EPON_DEBUG_LEVEL_ERR, "\r\n eponRegPatternTest fail!! register addr= 0x%x , rwmask= 0x%x, pattern = 0x%x ,  value=0x%x" , regTable_p[index].addr, regTable_p[index].rwmask, pattern , value);
			ret =-1;
		}
	}
	
	return ret;
}


int eponMacRegTest(__u32 times){
	__u32 patternIndex = 0;
	__u32 testTime = 0;
	int ret = 0;
	int rst = 0;
	for(testTime = 0;testTime<times; testTime++ ){
		for(patternIndex =0; patternIndex< sizeof(eponRegTestPattern)/4; patternIndex++){
			ret = eponRegPatternTest(eponRegTestPattern[patternIndex] , eponMacRegTable , eponMacGetRegTblSize());
			if(ret <0){
				rst = -1;
			}		
		}
		if(rst <0){
				return rst;
			}
	}
	return rst;
}

int eponRegDefCheck(eponMacHwtestReg_t *regTable_p,  __u32 maxIndex)
{
	int ret = 0;
	__u32 index = 0;
	__u32 cur_value= 0;

	for (index=0; index<maxIndex; index++)
	{
		cur_value = READ_REG_WORD(regTable_p[index].addr);
		if ((regTable_p[index].addr == e_local_time) || (regTable_p[index].addr == e_rpt_data) || (regTable_p[index].addr == e_rpt_len)
			|| (regTable_p[index].addr == e_dbg_prb_h32) || (regTable_p[index].addr == e_dbg_prb_l32))
			continue;
		if (cur_value != regTable_p[index].def_value)
		{
			printk("error: add:%x default: 0x%08lx real:0x%08lx\n", regTable_p[index].addr, regTable_p[index].def_value, cur_value);
			ret = -1;
		}
	}
		
	return ret;
}


////////////////////////////////////////////










#endif/*EPON_MAC_HW_TEST*/

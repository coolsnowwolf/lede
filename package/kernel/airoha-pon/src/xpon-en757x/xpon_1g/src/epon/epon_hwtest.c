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
#include <linux/types.h>

#include "epon/epon.h"
#include "common/drv_global.h"



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

eponMacRegHwTest_t *eponRegRstTable = NULL;
eponMacRegHwTest_t *eponLogRstTable = NULL;
eponMacHwtestReg_t *eponRegNotRstTable = NULL;
eponMacHwtestReg_t *eponRegDefValTable = NULL;

extern eponMacHwtestReg_t *eponMacRegTable;

int logRstTestFlag = 0;

void eponRegRstTableInit(void){
	int i = 0;
	eponRegRstTable = (eponMacRegHwTest_t *)kmalloc(sizeof(eponMacRegHwTest_t)*40, GFP_KERNEL) ;

	if(eponRegRstTable == NULL)
	{
	    printk("---[%s]---[%d]-- malloc fail\n", __FUNCTION__, __LINE__);
	    return;
	}

	eponRegRstTable[i].addr = (__u32)e_pending_gnt_num;
	eponRegRstTable[i].regMask = 0x0000ff00;
	i++;
		
	if(0 == logRstTestFlag){
		eponRegRstTable[i].addr = (__u32)e_llid0_dscvry_sts;
		eponRegRstTable[i].regMask = 0x0301ffff;
		i++;

		eponRegRstTable[i].addr = (__u32)e_llid1_dscvry_sts;
		eponRegRstTable[i].regMask = 0x0301ffff;
		i++;

		eponRegRstTable[i].addr = (__u32)e_llid2_dscvry_sts;
		eponRegRstTable[i].regMask = 0x0301ffff;
		i++;

		eponRegRstTable[i].addr = (__u32)e_llid3_dscvry_sts;
		eponRegRstTable[i].regMask = 0x0301ffff;
		i++;

		eponRegRstTable[i].addr = (__u32)e_llid4_dscvry_sts;
		eponRegRstTable[i].regMask = 0x0301ffff;
		i++;

		eponRegRstTable[i].addr = (__u32)e_llid5_dscvry_sts;
		eponRegRstTable[i].regMask = 0x0301ffff;
		i++;

		eponRegRstTable[i].addr = (__u32)e_llid6_dscvry_sts;
		eponRegRstTable[i].regMask = 0x0301ffff;
		i++;

		eponRegRstTable[i].addr = (__u32)e_llid7_dscvry_sts;
		eponRegRstTable[i].regMask = 0x0301ffff;
		i++;
	}else{
		eponRegRstTable[i].addr = (__u32)e_llid0_dscvry_sts;
		eponRegRstTable[i].regMask = 0x03000000;
		i++;

		eponRegRstTable[i].addr = (__u32)e_llid1_dscvry_sts;
		eponRegRstTable[i].regMask = 0x03000000;
		i++;

		eponRegRstTable[i].addr = (__u32)e_llid2_dscvry_sts;
		eponRegRstTable[i].regMask = 0x03000000;
		i++;

		eponRegRstTable[i].addr = (__u32)e_llid3_dscvry_sts;
		eponRegRstTable[i].regMask = 0x03000000;
		i++;

		eponRegRstTable[i].addr = (__u32)e_llid4_dscvry_sts;
		eponRegRstTable[i].regMask = 0x03000000;
		i++;

		eponRegRstTable[i].addr = (__u32)e_llid5_dscvry_sts;
		eponRegRstTable[i].regMask = 0x03000000;
		i++;

		eponRegRstTable[i].addr = (__u32)e_llid6_dscvry_sts;
		eponRegRstTable[i].regMask = 0x03000000;
		i++;

		eponRegRstTable[i].addr = (__u32)e_llid7_dscvry_sts;
		eponRegRstTable[i].regMask = 0x03000000;
		i++;
	}

	eponRegRstTable[i].addr = (__u32)e_local_time;
	eponRegRstTable[i].regMask = 0xffffffff;
	i++;

	if(0 == logRstTestFlag){
		eponRegRstTable[i].addr = (__u32)e_sync_time;
		eponRegRstTable[i].regMask = 0x0000ffff;
		i++;
	}

	eponRegRstTable[i].addr = (__u32)e_rxmbi_eth_cnt;
	eponRegRstTable[i].regMask = 0xffffffff;
	i++;	

	eponRegRstTable[i].addr = (__u32)e_rxmpi_eth_cnt;
	eponRegRstTable[i].regMask = 0xffffffff;
	i++;

	eponRegRstTable[i].addr = (__u32)e_txmbi_eth_cnt;
	eponRegRstTable[i].regMask = 0xffffffff;
	i++;

	eponRegRstTable[i].addr = (__u32)e_txmpi_eth_cnt;
	eponRegRstTable[i].regMask = 0xffffffff;
	i++;

	eponRegRstTable[i].addr = (__u32)e_oam_stat;
	eponRegRstTable[i].regMask = 0xffffffff;
	i++;

	eponRegRstTable[i].addr = (__u32)e_mpcp_stat;
	eponRegRstTable[i].regMask = 0xffffffff;
	i++;

	eponRegRstTable[i].addr = (__u32)e_mpcp_rgst_stat;
	eponRegRstTable[i].regMask = 0x0000ffff;
	i++;

	eponRegRstTable[i].addr = (__u32)e_gnt_pending_stat;
	eponRegRstTable[i].regMask = 0x00ff00ff;
	i++;

	eponRegRstTable[i].addr = (__u32)e_gnt_length_stat;
	eponRegRstTable[i].regMask = 0xffff0000;
	i++;

	eponRegRstTable[i].addr = (__u32)e_gnt_type_stat;
	eponRegRstTable[i].regMask = 0xffffffff;
	i++;

	eponRegRstTable[i].addr = (__u32)e_time_drft_stat;
	eponRegRstTable[i].regMask = 0x0000ffff;
	i++;

	eponRegRstTable[i].addr = (__u32)e_llid0_gnt_stat;
	eponRegRstTable[i].regMask = 0xffffffff;
	i++;

	eponRegRstTable[i].addr = (__u32)e_llid1_gnt_stat;
	eponRegRstTable[i].regMask = 0xffffffff;
	i++;

	eponRegRstTable[i].addr = (__u32)e_llid2_gnt_stat;
	eponRegRstTable[i].regMask = 0xffffffff;
	i++;

	eponRegRstTable[i].addr = (__u32)e_llid3_gnt_stat;
	eponRegRstTable[i].regMask = 0xffffffff;
	i++;

	eponRegRstTable[i].addr = (__u32)e_llid4_gnt_stat;
	eponRegRstTable[i].regMask = 0xffffffff;
	i++;

	eponRegRstTable[i].addr = (__u32)e_llid5_gnt_stat;
	eponRegRstTable[i].regMask = 0xffffffff;
	i++;

	eponRegRstTable[i].addr = (__u32)e_llid6_gnt_stat;
	eponRegRstTable[i].regMask = 0xffffffff;
	i++;

	eponRegRstTable[i].addr = (__u32)e_llid7_gnt_stat;
	eponRegRstTable[i].regMask = 0xffffffff;
	i++;
    
	eponRegRstTable[i].addr = (__u32)e_rpt_cnt;
	eponRegRstTable[i].regMask = 0x0000ffff;
	i++;

	eponRegRstTable[i].addr = (__u32)e_dscv_rdm_dly;
	eponRegRstTable[i].regMask = 0x0000ffff;
	i++;

	eponRegRstTable[i].addr = (__u32)e_trx_pkt_cnt;
	eponRegRstTable[i].regMask = 0xffffffff;
	i++;

	/*add mac register before here!
	Also need increase the kmalloc size!! */
	
	eponRegRstTable[i].addr = 0;
	eponRegRstTable[i].regMask = 0x0;
	
}
void eponLogRstTableInit(void){
	int i = 0;
	eponLogRstTable = (eponMacRegHwTest_t *)kmalloc(sizeof(eponMacRegHwTest_t)*10, GFP_KERNEL) ;
	if(eponLogRstTable == NULL)
	{
	    printk("---[%s]---[%d]-- malloc fail\n", __FUNCTION__, __LINE__);
	    return;
	}
	eponLogRstTable[i].addr = (__u32)e_llid0_dscvry_sts;
	eponLogRstTable[i].value = 0x00000000;
	eponLogRstTable[i].regMask = 0x1ffff;
	i++;

	eponLogRstTable[i].addr = (__u32)e_llid1_dscvry_sts;
	eponLogRstTable[i].value = 0x00000000;
	eponLogRstTable[i].regMask = 0x1ffff;
	i++;

	eponLogRstTable[i].addr = (__u32)e_llid2_dscvry_sts;
	eponLogRstTable[i].value = 0x00000000;
	eponLogRstTable[i].regMask = 0x1ffff;
	i++;

	eponLogRstTable[i].addr = (__u32)e_llid3_dscvry_sts;
	eponLogRstTable[i].value = 0x00000000;
	eponLogRstTable[i].regMask = 0x1ffff;
	i++;

	eponLogRstTable[i].addr = (__u32)e_llid4_dscvry_sts;
	eponLogRstTable[i].value = 0x00000000;
	eponLogRstTable[i].regMask = 0x1ffff;
	i++;

	eponLogRstTable[i].addr = (__u32)e_llid5_dscvry_sts;
	eponLogRstTable[i].value = 0x00000000;
	eponLogRstTable[i].regMask = 0x1ffff;
	i++;

	eponLogRstTable[i].addr = (__u32)e_llid6_dscvry_sts;
	eponLogRstTable[i].value = 0x00000000;
	eponLogRstTable[i].regMask = 0x1ffff;
	i++;

	eponLogRstTable[i].addr = (__u32)e_llid7_dscvry_sts;
	eponLogRstTable[i].value = 0x00000000;
	eponLogRstTable[i].regMask = 0x1ffff;
	i++;

	eponLogRstTable[i].addr = (__u32)e_sync_time;
	eponLogRstTable[i].value = 0x00000000;
	eponLogRstTable[i].regMask = 0xffff;
	i++;

	/*add mac register before here!
	Also need increase the kmalloc size!! */
	
	eponLogRstTable[i].addr = 0;
	eponLogRstTable[i].regMask = 0x0;
}

void eponRegNotRstTableInit(void){
	int i = 0;
	eponRegNotRstTable = (eponMacHwtestReg_t *)kmalloc(sizeof(eponMacHwtestReg_t)*100, GFP_KERNEL) ;

    if(eponRegNotRstTable == NULL)
    {
        printk("---[%s]---[%d]-- malloc fail\n", __FUNCTION__, __LINE__);
        return;
    }
	
	eponRegNotRstTable[i].addr = (__u32)e_glb_cfg ;//reset bit can not be set
	eponRegNotRstTable[i].def_value = 0x02800040;
	eponRegNotRstTable[i].rwmask = 0x7ffffff;
	i++;

	eponRegNotRstTable[i].addr = (__u32)e_int_en;//must disable all INT
	eponRegNotRstTable[i].def_value = 0x00000000;
	eponRegNotRstTable[i].rwmask = 0x07ffffff;
	i++;

	eponRegNotRstTable[i].addr = (__u32)e_llid0_3_cfg;
	eponRegNotRstTable[i].def_value = 0x00000000;
	eponRegNotRstTable[i].rwmask = 0x0f0f0f0f;
	i++;
	
	eponRegNotRstTable[i].addr = (__u32)e_llid4_7_cfg;
	eponRegNotRstTable[i].def_value = 0x00000000;
	eponRegNotRstTable[i].rwmask = 0x0f0f0f0f;
	i++;
    
	eponRegNotRstTable[i].addr = (__u32)e_rpt_cfg;
	eponRegNotRstTable[i].def_value = 0x00000000;
	eponRegNotRstTable[i].rwmask = 0xffffffff;
	i++;

	eponRegNotRstTable[i].addr = (__u32)e_tod_sync_x;
	eponRegNotRstTable[i].def_value = 0x00000000;
	eponRegNotRstTable[i].rwmask = 0xffffffff;
	i++;

	eponRegNotRstTable[i].addr = (__u32)e_new_tod_p2p_offset_sec_l32;
	eponRegNotRstTable[i].def_value = 0x00000000;
	eponRegNotRstTable[i].rwmask = 0xffffffff;
	i++;

	eponRegNotRstTable[i].addr = (__u32)e_slp_durt_max;
	eponRegNotRstTable[i].def_value = 0x00000000;
	eponRegNotRstTable[i].rwmask = 0xffffffff;
	i++;

	eponRegNotRstTable[i].addr = (__u32)e_slp_duration;
	eponRegNotRstTable[i].def_value = 0x00000000;
	eponRegNotRstTable[i].rwmask = 0xffffffff;
	i++;
	
	eponRegNotRstTable[i].addr = (__u32)e_act_duration;
	eponRegNotRstTable[i].def_value = 0x00000000;
	eponRegNotRstTable[i].rwmask = 0xffffffff;
	i++;
	
	eponRegNotRstTable[i].addr = (__u32)e_pwron_dly;
	eponRegNotRstTable[i].def_value = 0x00000000;
	eponRegNotRstTable[i].rwmask = 0xffffffff;
	i++;

	eponRegNotRstTable[i].addr = (__u32)e_txfetch_cfg;
	eponRegNotRstTable[i].def_value = 0x242a03e8;   /*driver init*/
	eponRegNotRstTable[i].rwmask = 0xffff0fff;
	i++;

	eponRegNotRstTable[i].addr = (__u32)e_tx_cal_cnst;
	eponRegNotRstTable[i].def_value = 0x2612010c;
	eponRegNotRstTable[i].rwmask = 0xffffff3f;
	i++;
	
	eponRegNotRstTable[i].addr = (__u32)e_laser_onoff_time;
	eponRegNotRstTable[i].def_value = 0x00002020;
	eponRegNotRstTable[i].rwmask = 0x0000ffff;
	i++;
	
	eponRegNotRstTable[i].addr = (__u32)e_grd_thrshld;
	eponRegNotRstTable[i].def_value = 0x00000008;
	eponRegNotRstTable[i].rwmask = 0x000000ff;
	i++;
	
	eponRegNotRstTable[i].addr = (__u32)e_mpcp_timeout_intvl;
	eponRegNotRstTable[i].def_value = 0x03b9aca0;
	eponRegNotRstTable[i].rwmask = 0xffffffff;
	i++;
	
	eponRegNotRstTable[i].addr = (__u32)e_rpt_timeout_intvl;
	eponRegNotRstTable[i].def_value = 0x002faf08;
	eponRegNotRstTable[i].rwmask = 0x00ffffff;
	i++;
	
	eponRegNotRstTable[i].addr = (__u32)e_max_future_gnt_time;
	eponRegNotRstTable[i].def_value = 0x03b9aca0;
	eponRegNotRstTable[i].rwmask = 0xffffffff;
	i++;
	
	eponRegNotRstTable[i].addr = (__u32)e_min_proc_time;
	eponRegNotRstTable[i].def_value = 0x00000400;
	eponRegNotRstTable[i].rwmask = 0x0000ffff;
	i++;
	
	eponRegNotRstTable[i].addr = (__u32)e_trx_adjust_time1;
	eponRegNotRstTable[i].def_value = 0x004ffff1;   /*driver init*/
	eponRegNotRstTable[i].rwmask = 0xffffffff;
	i++;
	
	eponRegNotRstTable[i].addr = (__u32)e_trx_adjust_time2;
	eponRegNotRstTable[i].def_value = 0x00000006;   /*driver init*/
	eponRegNotRstTable[i].rwmask = 0xffffffff;
	i++;
	
	eponRegNotRstTable[i].addr = (__u32)e_dbg_prb_sel;
	eponRegNotRstTable[i].def_value = 0x00000000;
	eponRegNotRstTable[i].rwmask = 0x00000f1f;
	i++;
    
	eponRegNotRstTable[i].addr = (__u32)e_snf_mpcp_oam_ctl;
	eponRegNotRstTable[i].def_value = 0x00000000;
	eponRegNotRstTable[i].rwmask = 0xffffffff;
	i++;
	
	eponRegNotRstTable[i].addr = (__u32)e_rpt_adj;
	eponRegNotRstTable[i].def_value = 0x01ff0000;
	eponRegNotRstTable[i].rwmask = 0x1ffffff;
	i++;

	eponRegNotRstTable[i].addr = (__u32)e_dyinggsp_w1;
	eponRegNotRstTable[i].def_value = 0x88090300;
	eponRegNotRstTable[i].rwmask = 0xffffffff;
	i++;
	
	eponRegNotRstTable[i].addr = (__u32)e_dyinggsp_w2;
	eponRegNotRstTable[i].def_value = 0x52000110;
	eponRegNotRstTable[i].rwmask = 0xffffffff;
	i++;
	
	eponRegNotRstTable[i].addr = (__u32)e_dyinggsp_w3;
	eponRegNotRstTable[i].def_value = 0x01000000;
	eponRegNotRstTable[i].rwmask = 0xffffffff;
	i++;
	
	eponRegNotRstTable[i].addr = (__u32)e_dyinggsp_w4;
	eponRegNotRstTable[i].def_value = 0x0f05ee00;
	eponRegNotRstTable[i].rwmask = 0xffffffff;
	i++;
	
	eponRegNotRstTable[i].addr = (__u32)e_dyinggsp_w5;
	eponRegNotRstTable[i].def_value = 0x13250022;
	eponRegNotRstTable[i].rwmask = 0xffffffff;
	i++;
	
	eponRegNotRstTable[i].addr = (__u32)e_dyinggsp_w6;
	eponRegNotRstTable[i].def_value = 0x01000210;
	eponRegNotRstTable[i].rwmask = 0xffffffff;
	i++;
	
	eponRegNotRstTable[i].addr = (__u32)e_dyinggsp_w7;
	eponRegNotRstTable[i].def_value = 0x01000000;
	eponRegNotRstTable[i].rwmask = 0xffffffff;
	i++;
	
	eponRegNotRstTable[i].addr = (__u32)e_dyinggsp_w8;
	eponRegNotRstTable[i].def_value = 0x0f05ee00;
	eponRegNotRstTable[i].rwmask = 0xffffffff;
	i++;
	
	eponRegNotRstTable[i].addr = (__u32)e_dyinggsp_w9;
	eponRegNotRstTable[i].def_value = 0x13250000;
	eponRegNotRstTable[i].rwmask = 0xffffffff;
	i++;
	
	eponRegNotRstTable[i].addr = (__u32)e_dyinggsp_w10;
	eponRegNotRstTable[i].def_value = 0x00000000;
	eponRegNotRstTable[i].rwmask = 0xffffffff;
	i++;
	
	eponRegNotRstTable[i].addr = (__u32)e_dyinggsp_w11;
	eponRegNotRstTable[i].def_value = 0x00000000;
	eponRegNotRstTable[i].rwmask = 0xffffffff;
	i++;
	
	eponRegNotRstTable[i].addr = (__u32)e_dyinggsp_w12;
	eponRegNotRstTable[i].def_value = 0x00000000;
	eponRegNotRstTable[i].rwmask = 0xffffffff;
	i++;
	
	eponRegNotRstTable[i].addr = (__u32)e_oam_kpalv_w1;
	eponRegNotRstTable[i].def_value = 0x00000000;
	eponRegNotRstTable[i].rwmask = 0xffffffff;
	i++;
	
	eponRegNotRstTable[i].addr = (__u32)e_oam_kpalv_w2;
	eponRegNotRstTable[i].def_value = 0x00000000;
	eponRegNotRstTable[i].rwmask = 0xffffffff;
	i++;
	
	eponRegNotRstTable[i].addr = (__u32)e_oam_kpalv_w3;
	eponRegNotRstTable[i].def_value = 0x00000000;
	eponRegNotRstTable[i].rwmask = 0xffffffff;
	i++;
	
	eponRegNotRstTable[i].addr = (__u32)e_oam_kpalv_w4;
	eponRegNotRstTable[i].def_value = 0x00000000;
	eponRegNotRstTable[i].rwmask = 0xffffffff;
	i++;
	
	eponRegNotRstTable[i].addr = (__u32)e_oam_kpalv_w5;
	eponRegNotRstTable[i].def_value = 0x00000000;
	eponRegNotRstTable[i].rwmask = 0xffffffff;
	i++;
	
	eponRegNotRstTable[i].addr = (__u32)e_oam_kpalv_w6;
	eponRegNotRstTable[i].def_value = 0x00000000;
	eponRegNotRstTable[i].rwmask = 0xffffffff;
	i++;
	
	eponRegNotRstTable[i].addr = (__u32)e_oam_kpalv_w7;
	eponRegNotRstTable[i].def_value = 0x00000000;
	eponRegNotRstTable[i].rwmask = 0xffffffff;
	i++;
	
	eponRegNotRstTable[i].addr = (__u32)e_oam_kpalv_w8;
	eponRegNotRstTable[i].def_value = 0x00000000;
	eponRegNotRstTable[i].rwmask = 0xffffffff;
	i++;
	
	eponRegNotRstTable[i].addr = (__u32)e_oam_kpalv_w9;
	eponRegNotRstTable[i].def_value = 0x00000000;
	eponRegNotRstTable[i].rwmask = 0xffffffff;
	i++;
	
	eponRegNotRstTable[i].addr = (__u32)e_oam_kpalv_w10;
	eponRegNotRstTable[i].def_value = 0x00000000;
	eponRegNotRstTable[i].rwmask = 0xffffffff;
	i++;
	
	eponRegNotRstTable[i].addr = (__u32)e_oam_kpalv_w11;
	eponRegNotRstTable[i].def_value = 0x00000000;
	eponRegNotRstTable[i].rwmask = 0xffffffff;
	i++;
	
	eponRegNotRstTable[i].addr = (__u32)e_oam_kpalv_w12;
	eponRegNotRstTable[i].def_value = 0x00000000;
	eponRegNotRstTable[i].rwmask = 0xffffffff;
	i++;
	
	eponRegNotRstTable[i].addr = (__u32)e_oam_kpalv_ctrl;
	eponRegNotRstTable[i].def_value = 0x00000000;
	eponRegNotRstTable[i].rwmask = 0xffffff00;
	i++;
	
	eponRegNotRstTable[i].addr = (__u32)e_tod_1pps_ctrl;
	eponRegNotRstTable[i].def_value = 0x01312d00;
	eponRegNotRstTable[i].rwmask = 0xffffffff;
	i++;
	
	eponRegNotRstTable[i].addr = (__u32)e_short_pkt_ctrl;
	eponRegNotRstTable[i].def_value = 0x00000040;
	eponRegNotRstTable[i].rwmask = 0xff;
	i++;
	
	eponRegNotRstTable[i].addr = (__u32)e_sniff_sp_tag;
	eponRegNotRstTable[i].def_value = 0x00010000;
	eponRegNotRstTable[i].rwmask = 0xffffffff;
	i++;

	eponRegNotRstTable[i].addr = (__u32)e_llid01_rpt_ctl;
	eponRegNotRstTable[i].def_value = 0x01ff01ff;
	eponRegNotRstTable[i].rwmask = 0x01ff01ff;
	i++;
	
	eponRegNotRstTable[i].addr = (__u32)e_llid23_rpt_ctl;
	eponRegNotRstTable[i].def_value = 0x01ff01ff;
	eponRegNotRstTable[i].rwmask = 0x01ff01ff;
	i++;
	
	eponRegNotRstTable[i].addr = (__u32)e_llid45_rpt_ctl;
	eponRegNotRstTable[i].def_value = 0x01ff01ff;
	eponRegNotRstTable[i].rwmask = 0x01ff01ff;
	i++;
	
	eponRegNotRstTable[i].addr = (__u32)e_llid67_rpt_ctl;
	eponRegNotRstTable[i].def_value = 0x01ff01ff;
	eponRegNotRstTable[i].rwmask = 0x01ff01ff;
	i++;

	eponRegNotRstTable[i].addr = (__u32)e_tx_prmb_ctl;
	eponRegNotRstTable[i].def_value = 0x55555500;
	eponRegNotRstTable[i].rwmask = 0xffffffff;
	i++;
	
	eponRegNotRstTable[i].addr = (__u32)e_tx_crc32_cfg;
	eponRegNotRstTable[i].def_value = 0x00000000;
	eponRegNotRstTable[i].rwmask = 0xffffffff;
	i++;
	
	eponRegNotRstTable[i].addr = (__u32)e_tx_prmb_ctl2;
	eponRegNotRstTable[i].def_value = 0xd5000000;
	eponRegNotRstTable[i].rwmask = 0xff000007;
	i++;

#if defined(TCSUPPORT_CPU_EN7523) || defined(TCSUPPORT_CPU_AN7552)
	eponRegNotRstTable[i].addr = (__u32)e_dscv_rdmdly_ctl;
	eponRegNotRstTable[i].def_value = 0x00000000;
	eponRegNotRstTable[i].rwmask = 0xffffffff;
	i++;

    eponRegNotRstTable[i].addr = (__u32)e_rpt_bigpkt_qs_ctl;
	eponRegNotRstTable[i].def_value = 0x00000000;
	eponRegNotRstTable[i].rwmask = 0xffffffff;
	i++;
#endif
	/*add mac register before here!
	Also need increase the kmalloc size!! */
	
	eponRegNotRstTable[i].addr = 0;
	eponRegNotRstTable[i].rwmask = 0x0;

}

void eponRegDefValTableInit(void){
    int i = 0;
	eponRegDefValTable = (eponMacHwtestReg_t *)kmalloc(sizeof(eponMacHwtestReg_t)*160, GFP_KERNEL) ;

    if(eponRegDefValTable == NULL)
    {
        printk("---[%s]---[%d]-- malloc fail\n", __FUNCTION__, __LINE__);
        return;
    }
	
	eponRegDefValTable[i].addr = (__u32)e_glb_cfg ;
	eponRegDefValTable[i].def_value = 0x02800040;
	eponRegDefValTable[i].rwmask = 0x1ffffff;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_int_status;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0x00000000;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_int_en;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0x00000000;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_rpt_mpcp_timeout_llid_idx;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0x00000000;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_dyinggsp_cfg;
	eponRegDefValTable[i].def_value = 0x00000100;
	eponRegDefValTable[i].rwmask = 0x8ff1037f;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_pending_gnt_num;
	eponRegDefValTable[i].def_value = 0x00000040;
	eponRegDefValTable[i].rwmask = 0x0000007f;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_llid0_3_cfg;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0x0f0f0f0f;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_llid4_7_cfg;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0x0f0f0f0f;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_llid_dscvry_ctrl;
	eponRegDefValTable[i].def_value = 0x00010000;
	eponRegDefValTable[i].rwmask = 0xc0001107;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_llid0_dscvry_sts;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0xc0000000;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_llid1_dscvry_sts;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0xc0000000;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_llid2_dscvry_sts;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0xc0000000;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_llid3_dscvry_sts;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0xc0000000;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_llid4_dscvry_sts;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0xc0000000;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_llid5_dscvry_sts;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0xc0000000;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_llid6_dscvry_sts;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0xc0000000;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_llid7_dscvry_sts;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0xc0000000;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_mac_addr_cfg;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0x8000000f;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_mac_addr_value;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0x00000000;   
	i++;
	eponRegDefValTable[i].addr = (__u32)e_security_key_cfg;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0x8000003f;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_key_value;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0x00000000;  
	i++;
	eponRegDefValTable[i].addr = (__u32)e_rpt_data;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0xffffffff;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_rpt_len;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0x0000013f;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_rpt_cfg;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0xffffffff;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_rpt_qthld_cfg;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0x00000000;   
	i++;
	eponRegDefValTable[i].addr = (__u32)e_tod_sync_x;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0xffffffff;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_tod_ltncy;
	eponRegDefValTable[i].def_value = 0x0000000d;
	eponRegDefValTable[i].rwmask = 0x00000000;
	i++;
	eponRegDefValTable[i].addr = (__u32)p2p_tx_tag1;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0x00000000;
	i++;
	eponRegDefValTable[i].addr = (__u32)p2p_tx_tag2;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0x00000000;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_new_tod_p2p_offset_sec_l32;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0xffffffff;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_new_tod_p2p_tod_offset_nsec;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0xffffffff;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_tod_p2p_tod_sec_l32;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0x00000000;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_tod_p2p_tod_nsec;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0x00000000;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_tod_period;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0xff;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_pwr_sv_cfg;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0x3700ffff;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_slp_durt_max;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0xffffffff;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_slp_duration;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0xffffffff;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_act_duration;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0xffffffff;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_pwron_dly;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0xffffffff;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_slp_duration_i;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0x00000000;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_txfetch_cfg;
	eponRegDefValTable[i].def_value = 0x102403e8;  
	eponRegDefValTable[i].rwmask = 0xffff0fff;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_sync_time;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0x00000000;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_tx_cal_cnst;
	eponRegDefValTable[i].def_value = 0x2612010c;
	eponRegDefValTable[i].rwmask = 0xffffff3f;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_laser_onoff_time;
	eponRegDefValTable[i].def_value = 0x00002020;
	eponRegDefValTable[i].rwmask = 0x0000ffff;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_grd_thrshld;
	eponRegDefValTable[i].def_value = 0x00000008;
	eponRegDefValTable[i].rwmask = 0x000000ff;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_mpcp_timeout_intvl;
	eponRegDefValTable[i].def_value = 0x03b9aca0;
	eponRegDefValTable[i].rwmask = 0xffffffff;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_rpt_timeout_intvl;
	eponRegDefValTable[i].def_value = 0x002faf08;
	eponRegDefValTable[i].rwmask = 0x00ffffff;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_max_future_gnt_time;
	eponRegDefValTable[i].def_value = 0x03b9aca0;
	eponRegDefValTable[i].rwmask = 0xffffffff;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_min_proc_time;
	eponRegDefValTable[i].def_value = 0x00000400;
	eponRegDefValTable[i].rwmask = 0x0000ffff;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_trx_adjust_time1;
	eponRegDefValTable[i].def_value = 0x00000000;   
	eponRegDefValTable[i].rwmask = 0xffffffff;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_trx_adjust_time2;
	eponRegDefValTable[i].def_value = 0x00000000;   
	eponRegDefValTable[i].rwmask = 0xffffffff;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_dbg_prb_sel;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0x00000f1f;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_dbg_prb_l32;
	eponRegDefValTable[i].def_value = 0x40000000;
	eponRegDefValTable[i].rwmask = 0x00000000;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_rxmbi_eth_cnt;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0x0;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_rxmpi_eth_cnt;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0x0;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_txmbi_eth_cnt;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0x0;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_txmpi_eth_cnt;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0x0;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_oam_stat;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0x0;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_mpcp_stat;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0x0;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_mpcp_rgst_stat;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0x0;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_gnt_pending_stat;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0x0;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_gnt_length_stat;
	eponRegDefValTable[i].def_value = 0x0000ffff;
	eponRegDefValTable[i].rwmask = 0x0;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_gnt_type_stat;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0x0;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_time_drft_stat;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0x0;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_llid0_gnt_stat;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0x0;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_llid1_gnt_stat;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0x0;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_llid2_gnt_stat;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0x0;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_llid3_gnt_stat;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0x0;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_llid4_gnt_stat;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0x0;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_llid5_gnt_stat;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0x0;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_llid6_gnt_stat;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0x0;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_llid7_gnt_stat;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0x0;
	i++;
#if defined(TCSUPPORT_CPU_EN7523) || defined(TCSUPPORT_CPU_AN7552)
	eponRegDefValTable[i].addr = (__u32)e_snf_mpcp_oam_ctl;
	eponRegDefValTable[i].def_value = 0x352b0000;
	eponRegDefValTable[i].rwmask = 0xffffffff;
	i++;
#else
    eponRegDefValTable[i].addr = (__u32)e_snf_mpcp_oam_ctl;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0xffffffff;
	i++;
#endif
	eponRegDefValTable[i].addr = (__u32)e_rpt_adj;
	eponRegDefValTable[i].def_value = 0x01ff0000;
	eponRegDefValTable[i].rwmask = 0x1ffffff;
	i++;
    eponRegDefValTable[i].addr = (__u32)e_rpt_cnt;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0x00000000;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_dyinggsp_w1;
	eponRegDefValTable[i].def_value = 0x88090300;
	eponRegDefValTable[i].rwmask = 0xffffffff;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_dyinggsp_w2;
	eponRegDefValTable[i].def_value = 0x52000110;
	eponRegDefValTable[i].rwmask = 0xffffffff;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_dyinggsp_w3;
	eponRegDefValTable[i].def_value = 0x01000000;
	eponRegDefValTable[i].rwmask = 0xffffffff;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_dyinggsp_w4;
	eponRegDefValTable[i].def_value = 0x0f05ee00;
	eponRegDefValTable[i].rwmask = 0xffffffff;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_dyinggsp_w5;
	eponRegDefValTable[i].def_value = 0x13250022;
	eponRegDefValTable[i].rwmask = 0xffffffff;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_dyinggsp_w6;
	eponRegDefValTable[i].def_value = 0x01000210;
	eponRegDefValTable[i].rwmask = 0xffffffff;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_dyinggsp_w7;
	eponRegDefValTable[i].def_value = 0x01000000;
	eponRegDefValTable[i].rwmask = 0xffffffff;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_dyinggsp_w8;
	eponRegDefValTable[i].def_value = 0x0f05ee00;
	eponRegDefValTable[i].rwmask = 0xffffffff;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_dyinggsp_w9;
	eponRegDefValTable[i].def_value = 0x13250000;
	eponRegDefValTable[i].rwmask = 0xffffffff;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_dyinggsp_w10;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0xffffffff;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_dyinggsp_w11;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0xffffffff;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_dyinggsp_w12;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0xffffffff;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_oam_kpalv_w1;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0xffffffff;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_oam_kpalv_w2;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0xffffffff;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_oam_kpalv_w3;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0xffffffff;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_oam_kpalv_w4;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0xffffffff;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_oam_kpalv_w5;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0xffffffff;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_oam_kpalv_w6;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0xffffffff;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_oam_kpalv_w7;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0xffffffff;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_oam_kpalv_w8;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0xffffffff;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_oam_kpalv_w9;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0xffffffff;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_oam_kpalv_w10;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0xffffffff;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_oam_kpalv_w11;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0xffffffff;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_oam_kpalv_w12;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0xffffffff;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_oam_kpalv_ctrl;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0xffffff00;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_tod_1pps_ctrl;
	eponRegDefValTable[i].def_value = 0x01312d00;
	eponRegDefValTable[i].rwmask = 0xffffffff;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_short_pkt_ctrl;
	eponRegDefValTable[i].def_value = 0x00000040;
	eponRegDefValTable[i].rwmask = 0xff;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_sniff_sp_tag;
	eponRegDefValTable[i].def_value = 0x00010000;
	eponRegDefValTable[i].rwmask = 0xffffffff;
	i++;
    eponRegDefValTable[i].addr = (__u32)e_dscv_rdm_dly;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0x00000000;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_llid01_rpt_ctl;
	eponRegDefValTable[i].def_value = 0x01ff01ff;
	eponRegDefValTable[i].rwmask = 0x01ff01ff;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_llid23_rpt_ctl;
	eponRegDefValTable[i].def_value = 0x01ff01ff;
	eponRegDefValTable[i].rwmask = 0x01ff01ff;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_llid45_rpt_ctl;
	eponRegDefValTable[i].def_value = 0x01ff01ff;
	eponRegDefValTable[i].rwmask = 0x01ff01ff;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_llid67_rpt_ctl;
	eponRegDefValTable[i].def_value = 0x01ff01ff;
	eponRegDefValTable[i].rwmask = 0x01ff01ff;
	i++;
    eponRegDefValTable[i].addr = (__u32)e_snf_pkt_cnt;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0x00000000;
	i++;
    eponRegDefValTable[i].addr = (__u32)e_rx_pkt_lch;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0x00000000;
	i++;
    eponRegDefValTable[i].addr = (__u32)e_rx_prmb_1st_wd;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0x00000000;
	i++;
#if defined(TCSUPPORT_CPU_EN7528) || defined(TCSUPPORT_CPU_EN7523) || defined(TCSUPPORT_CPU_AN7552)
    eponRegDefValTable[i].addr = (__u32)e_utili_check;
	eponRegDefValTable[i].def_value = 0x00000004;
	eponRegDefValTable[i].rwmask = 0x00000005;
	i++;
    eponRegDefValTable[i].addr = (__u32)e_utili_clk_cycle_cnth;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0x00000000;
	i++;
    eponRegDefValTable[i].addr = (__u32)e_utili_clk_cycle_cntl;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0x00000000;
	i++;
    eponRegDefValTable[i].addr = (__u32)e_total_gnt_sizeh;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0x00000000;
	i++;
    eponRegDefValTable[i].addr = (__u32)e_total_gnt_sizel;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0x00000000;
	i++;
    eponRegDefValTable[i].addr = (__u32)e_total_gnt_cnt;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0x00000000;
	i++;
    eponRegDefValTable[i].addr = (__u32)e_total_pkt_sizeh;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0x00000000;
	i++;
    eponRegDefValTable[i].addr = (__u32)e_total_pkt_sizel;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0x00000000;
	i++;
    eponRegDefValTable[i].addr = (__u32)e_total_pkt_cnt;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0x00000000;
	i++;
    eponRegDefValTable[i].addr = (__u32)e_rxerr_cnt;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0x00000000;
	i++;
    eponRegDefValTable[i].addr = (__u32)e_trx_pkt_cnt;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0x00000000;
	i++;
    eponRegDefValTable[i].addr = (__u32)e_rx_crc32_cnt;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0x00000000;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_tx_prmb_ctl;
	eponRegDefValTable[i].def_value = 0x55555500;
	eponRegDefValTable[i].rwmask = 0xffffffff;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_tx_crc32_cfg;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0xffffffff;
	i++;
	eponRegDefValTable[i].addr = (__u32)e_tx_prmb_ctl2;
	eponRegDefValTable[i].def_value = 0xd5000000;
	eponRegDefValTable[i].rwmask = 0xff000007;
	i++;
#endif

#if defined(TCSUPPORT_CPU_EN7523) || defined(TCSUPPORT_CPU_AN7552)
    eponRegDefValTable[i].addr = (__u32)e_dscv_rdmdly_ctl;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0xffffffff;
	i++;

    eponRegDefValTable[i].addr = (__u32)e_rpt_bigpkt_qs_ctl;
	eponRegDefValTable[i].def_value = 0x00000000;
	eponRegDefValTable[i].rwmask = 0xffffffff;
	i++;
#endif

	/*add mac register before here!
	Also need increase the kmalloc size!! */
	
	eponMacRegTable[i].addr = 0;
	eponMacRegTable[i].rwmask = 0x0;  

    return;
}

int eponRegPatternTest(__u32 pattern, eponMacHwtestReg_t *regTable_p,  __u32 maxIndex){
	int ret = 0;
	__u32 index = 0;
	__u32 value= 0;
	eponDbgPrint(EPON_DEBUG_LEVEL_TRACE, "pattern=0x%x", pattern);
	for(index = 0; index < maxIndex; index++ ){
		eponDbgPrint(EPON_DEBUG_LEVEL_TRACE, "\r\n eponRegPatternTest ! register addr= 0x%x , rwmask= 0x%x, pattern = 0x%x ,  value=0x%x" , regTable_p[index].addr, regTable_p[index].rwmask, pattern , value);
			
		WRITE_REG_WORD(regTable_p[index].addr , (pattern&regTable_p[index].rwmask));
		if(regTable_p[index].addr  == (__u32)e_sync_time){
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
		if ((regTable_p[index].addr == (__u32)e_local_time) || (regTable_p[index].addr == (__u32)e_rpt_data) || (regTable_p[index].addr == (__u32)e_rpt_len)
			|| (regTable_p[index].addr == (__u32)e_dbg_prb_h32) || (regTable_p[index].addr == (__u32)e_dbg_prb_l32))
			continue;
		if (cur_value != regTable_p[index].def_value)
		{
			printk("error: add:%x default: 0x%08x real:0x%08x\n", regTable_p[index].addr, regTable_p[index].def_value, cur_value);
			ret = -1;
		}
	}
		
	return ret;
}

int eponRegRstTest(void){
	int ret = 0;
	__u32 Raw = 0;
	__u32 index = 0;
	__u32 tableSize = 0;

	eponRegRstTableInit();
	if(NULL == eponRegRstTable){
		printk("ERR: eponRegRstTable is not initialized!!!\n");
		return -1;
	}else{
		while(eponRegRstTable[index].addr != 0)
			index++;

		tableSize =index;
	}
#ifdef TCSUPPORT_CPU_ARMV8
	if(0 == logRstTestFlag){
		Raw = GET_SSR3();
		Raw = Raw | (1 << 10);
		SET_SSR3(Raw);
	}else{
		Raw = GET_SSR3();
		Raw = Raw | (1 << 4);
		SET_SSR3(Raw);
	}
#else
	if(0 == logRstTestFlag){
		Raw = READ_REG_WORD(0xbfb00094);
		Raw = Raw | (1 << 10);
		WRITE_REG_WORD(0xbfb00094, Raw);
	}else{
		Raw = READ_REG_WORD(0xbfb00094);
		Raw = Raw | (1 << 4);
		WRITE_REG_WORD(0xbfb00094, Raw);
	}
#endif	
	for(index = 0 ; index < tableSize ; index++){
		Raw = READ_REG_WORD(eponRegRstTable[index].addr) & eponRegRstTable[index].regMask;
		if(Raw){
			printk("eponRegRstTest : Failed!!! register addr : %08x   value : %08x\n",eponRegRstTable[index].addr,Raw);
			ret = -1;
		}
	}

	Raw = READ_REG_WORD(e_gnt_length_stat) & 0xffff;
	if(0xffff != Raw){
		printk("eponRegRstTest : Failed!!! register addr : %08x   value : %08x\n",(__u32)e_gnt_length_stat,Raw);
	}

	kfree(eponRegRstTable);
	return ret;
}

int eponRegNotRstTest(__u32 pattern){
	int ret = 0;
	__u32 Raw = 0;
	__u32 index = 0;
	__u32 maxIndex = 0;

	eponRegNotRstTableInit();

	if(NULL == eponRegNotRstTable){
		printk("ERR: eponRegNotRstTable is not initialized!!!\n");
		return -1;
	}else{
		while(eponRegNotRstTable[maxIndex].addr != 0)
			maxIndex++;
	}

	for(index = 0; index < maxIndex; index++){
		WRITE_REG_WORD(eponRegNotRstTable[index].addr,pattern);
	}

	if(0 == logRstTestFlag){
#ifdef TCSUPPORT_CPU_ARMV8
		Raw = GET_SSR3();
		Raw = Raw | (1 << 10);
		SET_SSR3(Raw);			
#else		
		Raw = READ_REG_WORD(0xbfb00094);
		Raw = Raw | (1 << 10);
		WRITE_REG_WORD(0xbfb00094, Raw);
#endif	
	}else{
		__u32 tableSize = 0;
		__u32 *regValTable = NULL;
		
		eponLogRstTableInit();
		if(NULL == eponLogRstTable){
			printk("ERR: eponLogRstTable is not initialized!!!\n");
			return -1;
		}else{
			index = 0;
			while(eponLogRstTable[index].addr != 0)
				index++;

			tableSize = index;
		}
		
		regValTable = (__u32 *)kmalloc(tableSize*sizeof(__u32), GFP_KERNEL);

		if( regValTable == NULL)
		{
		    printk("---[%s]---[%d]-- malloc fail\n", __FUNCTION__, __LINE__);
		    return -1;
		}
		
		for(index = 0 ; index < tableSize ; index++){
			regValTable[index] = READ_REG_WORD(eponLogRstTable[index].addr) & eponLogRstTable[index].regMask;
		}
#ifdef TCSUPPORT_CPU_ARMV8
		Raw = GET_SSR3();
		Raw = Raw | (1 << 4);
		SET_SSR3(Raw);			
#else		
		Raw = READ_REG_WORD(0xbfb00094);
		Raw = Raw | (1 << 4);
		WRITE_REG_WORD(0xbfb00094, Raw);
#endif		
		for(index = 0 ; index < tableSize ; index++){
			Raw = READ_REG_WORD(eponLogRstTable[index].addr) & eponLogRstTable[index].regMask;
			if(Raw != regValTable[index]){
				printk("eponRegNotRstTest : Failed!!! register addr : %08x beforeReset : %08x afterReset : %08x\n",eponLogRstTable[index].addr,regValTable[index],Raw);
				ret = -1;
			}
		}

		kfree(eponLogRstTable);
		kfree(regValTable);
	}

	for(index = 0; index < maxIndex; index++){
		Raw = 0;
		Raw = READ_REG_WORD(eponRegNotRstTable[index].addr);
		if((pattern&(eponRegNotRstTable[index].rwmask)) != (Raw&(eponRegNotRstTable[index].rwmask))){
			printk("eponRegNotRstTest : Failed!!! register addr : %08x   value : %08x\n",eponRegNotRstTable[index].addr,Raw);
			ret = -1;
		}
	}

	kfree(eponRegNotRstTable);
	eponRegNotRstTable = NULL;
	
	return ret;
}

int eponMacRstTest(void){
    int ret = 0;
	__u32 Raw = 0, index = 0, maxIndex = 0;
    __u32 dbg_prb_h32_def = 0x800004;

	eponRegDefValTableInit();

	if(NULL == eponRegDefValTable){
		printk("ERR: eponRegDefValTable is not initialized!!!\n");
		return -1;
	}else{
		while(eponRegDefValTable[maxIndex].addr != 0)
			maxIndex++;
	}

    /*epon mac reset*/
    #ifdef TCSUPPORT_CPU_ARMV8
	/* reset mac */
	Raw = GET_SCU_RSTCTRL1();
	Raw |= SCU_EPON_MAC_RESET  ;
	SET_SCU_RSTCTRL1(Raw);
	udelay(1);
	/* release mac */
	Raw = GET_SCU_RSTCTRL1();
	Raw &= ~SCU_EPON_MAC_RESET ;
	SET_SCU_RSTCTRL1(Raw);
    #else
    Raw = READ_REG_WORD(REG_E_SW_RST);
	Raw |= (1<<31);
	WRITE_REG_WORD(REG_E_SW_RST , Raw);
	udelay(1);
	Raw &= 0x7fffffff;
	WRITE_REG_WORD(REG_E_SW_RST , Raw);
    #endif
    
    for(index = 0; index < maxIndex; index++){
        //printk("reg:0x%x \n",eponRegDefValTable[index].addr);
        Raw = READ_REG_WORD(eponRegDefValTable[index].addr);
        if(Raw != eponRegDefValTable[index].def_value){
            ret = -1;
            printk("ERR: index:%u reg[0x%x] hardware default value is 0x%08x, actually 0x%08x \n",index,
                eponRegDefValTable[index].addr,eponRegDefValTable[index].def_value,Raw);
        }
    }

    Raw = (READ_REG_WORD(e_dbg_prb_h32) & 0xfffffdff);/*e_dbg_prb_h32 bit9 is clock singnal,it's value maybe 0 or 1*/
    if(Raw != dbg_prb_h32_def){
        ret = -1;
        printk("ERR: reg[0x%x] hardware default value is 0x%08x, actually 0x%08x \n",(__u32)e_dbg_prb_h32,dbg_prb_h32_def,Raw);
    }

    kfree(eponRegDefValTable);
	eponRegDefValTable = NULL;

    return ret;
}
////////////////////////////////////////////
#endif/*EPON_MAC_HW_TEST*/

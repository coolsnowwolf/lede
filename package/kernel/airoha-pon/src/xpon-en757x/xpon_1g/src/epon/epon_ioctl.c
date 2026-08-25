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
	epon_ioctl.c

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
#include "common/phy_if_wrapper.h"
#include <asm/tc3162/ledcetrl.h>
#include "epon/epon.h"
#include <ecnt_hook/ecnt_hook_fe.h>
#include <ecnt_hook/ecnt_hook_qdma.h>
#include "common/xpon_led.h"

extern __u32 eponDebugLevel ;
extern int isAuthFail;
extern epon_t eponDrv;
extern __u8 eponOnuMacAddr[6];
extern __u8 g_silence_time;
extern __u32 auto_switch_oam_fav;
int rogueOnuDisableTxPowerFlag = 0;
int turnOffOpticalModuel = 0;
__u8 mpcpcnt_llid = 0;
uint16_t stackMpcpErrCnt = 0;

extern void get_register_count(XPON_DEBUG_STATISTIC *tmpReg);


__u8 g_rx_fec = 1;
__u8 g_llid_key_index[EPON_1G_MAX_LLID_NUM] = {0};

int eponSetLlidKey(__u8 llidIndex, __u8 keyIndex, __u8 *key){
	REG_e_security_key_cfg eponSecurityKeyCfgReg;
	__u32 keyValue = 0;
	__u32 timers = 0;
	__u8 *cp = NULL;
	memset(&eponSecurityKeyCfgReg , 0 , sizeof(REG_e_security_key_cfg));
	
	if((llidIndex >= EPON_1G_MAX_LLID_NUM)||(keyIndex >1)||(key == NULL)){
		eponDbgPrint(EPON_DEBUG_LEVEL_ERR, "\r\neponSetLlidKey param error");	
		return -1;
	}
	
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_XPON_HAL_API_EXT)
    g_llid_key_index[llidIndex] = keyIndex & 0xF;
#endif/*TCSUPPORT_COMPILE*/

#if 0
		//check done bit is ready
	while(timers < 100){
		eponSecurityKeyCfgReg.Raw = READ_REG_WORD(e_security_key_cfg);
		if(eponSecurityKeyCfgReg.Bits.key_rwcmd_done == 0){
			break;
		}
		timers++;
	}



	if(timers == 100){
		eponDbgPrint(EPON_DEBUG_LEVEL_ERR, "\r\n eponSetLlidKey set 1 timers == 100");
		return -1;
	}
#endif
	cp = (__u8 *)(&keyValue);
#ifdef __LITTLE_ENDIAN
	cp[2] = key[0];
	cp[1] = key[1];
	cp[0] = key[2];
#else
	cp++;
	memcpy(cp, key, 3);
#endif
	eponDbgPrint(EPON_DEBUG_LEVEL_TRACE, "eponSetLlidKey keyValue = 0x%x", keyValue);
	WRITE_REG_WORD(e_key_value , keyValue);
	
	eponSecurityKeyCfgReg.Bits.key_rwcmd = 1;
	eponSecurityKeyCfgReg.Bits.key_llid_index = (llidIndex&0x7);
	eponSecurityKeyCfgReg.Bits.key_idx = (keyIndex&0x01);
	eponDbgPrint(EPON_DEBUG_LEVEL_TRACE, "eponSetLlidKey eponSecurityKeyCfgReg = 0x%x", eponSecurityKeyCfgReg.Raw);	
	WRITE_REG_WORD(e_security_key_cfg , eponSecurityKeyCfgReg.Raw);
	
	//check done bit is ready
	timers = 0;
	while(timers < 100){
		eponSecurityKeyCfgReg.Raw = READ_REG_WORD(e_security_key_cfg);
		if(eponSecurityKeyCfgReg.Bits.key_rwcmd_done == 1){
			break;
		}
		timers++;
	}

	if(timers == 100){
		eponDbgPrint(EPON_DEBUG_LEVEL_ERR, "\r\n eponSetLlidKey set  timers == 100");
		return -1;
	}
	return 0;
	
}




int eponGetLlidKey(__u8 llidIndex, __u8 keyIndex, __u8 *key){
	REG_e_security_key_cfg eponSecurityKeyCfgReg;
	__u32 keyValue = 0;
	__u32 timers = 0;
	__u8 *cp = NULL;
	memset(&eponSecurityKeyCfgReg , 0 , sizeof(REG_e_security_key_cfg));

	
	if((llidIndex >= EPON_1G_MAX_LLID_NUM)||(keyIndex >1)||(key == NULL)){
		eponDbgPrint(EPON_DEBUG_LEVEL_ERR, "\r\neponSetLlidKey param error");	
		return -1;
	}


#if 0
		//check done bit is ready
	while(timers < 100){
		eponSecurityKeyCfgReg.Raw = READ_REG_WORD(e_security_key_cfg);
		if(eponSecurityKeyCfgReg.Bits.key_rwcmd_done == 0){
			break;
		}
		timers++;
	}

	if(timers == 100){
		eponDbgPrint(EPON_DEBUG_LEVEL_ERR, "\r\n eponSetLlidKey get 1 timers == 100");
		return -1;
	}

#endif
	
	eponSecurityKeyCfgReg.Bits.key_rwcmd = 0;
	eponSecurityKeyCfgReg.Bits.key_llid_index = (llidIndex&0x7);
	eponSecurityKeyCfgReg.Bits.key_idx = (keyIndex&0x01);
	WRITE_REG_WORD(e_security_key_cfg , eponSecurityKeyCfgReg.Raw);
	
	//check done bit is ready
	timers = 0;
	while(timers < 100){
		eponSecurityKeyCfgReg.Raw = READ_REG_WORD(e_security_key_cfg);
		if(eponSecurityKeyCfgReg.Bits.key_rwcmd_done == 1){
			break;
		}
		timers++;
	}

	if(timers == 100){
		eponDbgPrint(EPON_DEBUG_LEVEL_ERR, "\r\n eponSetLlidKey get  timers == 100");
		return -1;
	}
	
	keyValue = READ_REG_WORD(e_key_value);
	keyValue = ntohl(keyValue);
	cp = (__u8 *)&keyValue;
	cp++;
	memcpy(key , cp ,3);
	eponDbgPrint(EPON_DEBUG_LEVEL_TRACE, "eponSetLlidKey key=0x%x", keyValue);
	return 0;
	
}






int eponSetLlidFec(__u8 llidIndex, __u8 fecFlag){
	__u32 regAddr = 0;
	__u32 Raw = 0;
	__u8 offset = 0;
#if defined(TCSUPPORT_CPU_EN7528) || defined(TCSUPPORT_CPU_EN7523) || defined(TCSUPPORT_CPU_AN7552)
	__u32 fec_cfg = 0;
#endif


	eponDbgPrint(EPON_DEBUG_LEVEL_TRACE, "\r\nenter eponSetLlidFec");	
	if((llidIndex >= EPON_1G_MAX_LLID_NUM)){
		eponDbgPrint(EPON_DEBUG_LEVEL_ERR, "\r\neponSetLlidFec param error");	
		return -1;
	}


	if(llidIndex < 4){
		regAddr = (__u32)e_llid0_3_cfg;
		offset = llidIndex*8 + 3;
	}
	else{
		regAddr = (__u32)e_llid4_7_cfg;
		offset = (llidIndex - 4)*8 + 3;
	}

	Raw = READ_REG_WORD(regAddr);
	if(fecFlag == 1){//enable
		Raw |= (1<<offset);
	}else{//disable
		Raw &= ~(1<<offset);
	}
	
	WRITE_REG_WORD(regAddr , Raw);

#ifdef TCSUPPORT_CPU_EN7528
	fec_cfg = READ_FE_REG(0xbfb55630);
	if(fecFlag == 1){//enable
		fec_cfg |= (1 << llidIndex);
	}else{//disable
		fec_cfg &= ~(1 << llidIndex);
	}
	WRITE_FE_REG(0xbfb55630, fec_cfg);
#endif

#if defined(TCSUPPORT_CPU_EN7523) || defined(TCSUPPORT_CPU_AN7552)
		fec_cfg = READ_FE_REG(0xbfb57130);
		if(fecFlag == 1){//enable
			fec_cfg |= (1 << llidIndex);
		}else{//disable
			fec_cfg &= ~(1 << llidIndex);
		}
		WRITE_FE_REG(0xbfb57130, fec_cfg);
#endif

	return 0;
}



int eponGetLlidFec(__u8 llidIndex, __u8 *fecFlag){
	__u32 regAddr = 0;
	__u32 Raw = 0;
	__u8 offset = 0;
	
	if((llidIndex >= EPON_1G_MAX_LLID_NUM)){
		eponDbgPrint(EPON_DEBUG_LEVEL_ERR, "\r\neponSetLlidFec param error");	
		return -1;
	}

	if(llidIndex < 4){
		regAddr = (__u32)e_llid0_3_cfg;
		offset = llidIndex*8 + 3;
	}
	else{
		regAddr = (__u32)e_llid4_7_cfg;
		offset = (llidIndex - 4)*8 + 3;
	}

	Raw = READ_REG_WORD(regAddr);
	if(Raw & (1<<offset)){//enable
		*fecFlag = 1;
	}else{//disable
		*fecFlag = 0;
	}
	
	return 0;
}

typedef  union
{
  struct
  {
    	__u32 qsize_fec_adj:16;
	__u32 eponChn7QrptCfg:2;
	__u32 eponChn6QrptCfg:2;
	__u32 eponChn5QrptCfg:2;
	__u32 eponChn4QrptCfg:2;
	__u32 eponChn3QrptCfg:2;
	__u32 eponChn2QrptCfg:2;
	__u32 eponChn1QrptCfg:2;
	__u32 eponChn0QrptCfg:2;
  } Bits;
  __u32 Raw;
} eponRptCfgReg_t; 



int eponSetLlidThrshldNum(__u8 llidIndex, __u8 num){
	__u32 regAddr = (__u32)e_rpt_cfg;
	__u32 Raw =0;
	eponRptCfgReg_t eponRptCfgReg;
	eponRptCfgReg.Raw = 0;	


	eponDbgPrint(EPON_DEBUG_LEVEL_TRACE, "\r\nenter eponSetLlidThrshldNum llidIndex=%d ,num=%d ", llidIndex, num);	
	if((llidIndex >= EPON_1G_MAX_LLID_NUM)||(num > 3)){
		eponDbgPrint(EPON_DEBUG_LEVEL_ERR, "\r\neponSetLlidThrshldNum param error");	
		return -1;
	}
	Raw = num;
	Raw <<= (llidIndex*2);


	eponRptCfgReg.Raw = READ_REG_WORD(regAddr);
	eponRptCfgReg.Raw &= ~(3<<(llidIndex*2));
	eponRptCfgReg.Raw |= Raw;

	eponDbgPrint(EPON_DEBUG_LEVEL_TRACE, "eponSetLlidThrshldNum eponRptCfgReg.Raw=%x", eponRptCfgReg.Raw);	
	WRITE_REG_WORD(regAddr , eponRptCfgReg.Raw);
	return 0;
}





int eponGetLlidThrshldNum(__u8 llidIndex, __u8 *num){
	__u32 regAddr = (__u32)e_rpt_cfg;
	eponRptCfgReg_t eponRptCfgReg;
	eponRptCfgReg.Raw = 0;	
	


	eponDbgPrint(EPON_DEBUG_LEVEL_TRACE, "\r\nenter eponGetLlidQueNum");	
	if((llidIndex >= EPON_1G_MAX_LLID_NUM)){
		eponDbgPrint(EPON_DEBUG_LEVEL_ERR, "\r\neponGetLlidQueNum param error");	
		return -1;
	}

	eponRptCfgReg.Raw = READ_REG_WORD(regAddr);
	eponRptCfgReg.Raw &= (3<<(llidIndex*2));
	eponRptCfgReg.Raw >>= (llidIndex*2);
	num[0] = (__u8)eponRptCfgReg.Raw;

	eponDbgPrint(EPON_DEBUG_LEVEL_TRACE, "eponGetLlidQueNum eponRptCfgReg.Raw=%x", num[0]);	
	//WRITE_REG_WORD(regAddr , eponRptCfgReg.Raw);
	return 0;
}

typedef  union
{
  struct
  {
    __u32 qThrshldRwcmd:1;
	__u32 reserved:6;
	__u32 qThrshldRwcmd_done:1;
	__u32 qThrshldValue:16;
	__u32 thrPos:2;
	__u32 chnIdx:3;
	__u32 queueIdx:3;
  } Bits;
  __u32 Raw;
} eponQThrshldCfgReg_t; 




int eponSetLlidQueThrod(__u8 llidIndex, __u8 setIndex, __u8 bitmap,__u16 *threshold ){
	__u8 queueIndex = 0;
	eponQueueThreshold_t EponQThr;
	int ret = 0;

	if((llidIndex >= EPON_1G_MAX_LLID_NUM) || (threshold == NULL)){
		eponDbgPrint(EPON_DEBUG_LEVEL_ERR, "\r\eponSetLlidQueThrod parameters error");
		return -1;
	}

#if 0
	ret = epon_set_llid_report_bitmap(llidIndex, bitmap);
	if(ret != 0 ){
		eponDbgPrint(EPON_DEBUG_LEVEL_ERR, "\r\epon_set_llid_report_bitmap return error");	
		return -1;
	}
#endif

	for(queueIndex = 0; queueIndex< 8;queueIndex++){
		if(bitmap&(1<<queueIndex)){//set queue threshold
			EponQThr.channel = llidIndex;
			EponQThr.queue = queueIndex;
			EponQThr.thrIdx = setIndex;
			EponQThr.value = threshold[queueIndex];
			ret = epon_set_queue_threshold(&EponQThr) ;
			if(ret < 0 ){
				eponDbgPrint(EPON_DEBUG_LEVEL_ERR, "\r\neponSetLlidQueThrod error");	
				return -1;
			}
		}
	}
	return 0;
}





int eponGetLlidQueThrod(__u8 llidIndex, __u8 setIndex, __u8 *bitmap,__u16 *threshold ){
	__u8 queueIndex = 0;
	eponQueueThreshold_t EponQThr;
	int ret = 0;

	if((llidIndex >= EPON_1G_MAX_LLID_NUM) || (bitmap == NULL) || (threshold == NULL)){
		eponDbgPrint(EPON_DEBUG_LEVEL_ERR, "\r\eponGetLlidQueThrod parameters error");
		return -1;
	}
	
	if(epon_get_llid_report_bitmap(llidIndex, bitmap) != 0){
		eponDbgPrint(EPON_DEBUG_LEVEL_ERR, "\r\epon_get_llid_report_bitmap return error");	
		return -1;
	}
	
	for(queueIndex = 0; queueIndex< 8;queueIndex++){
		memset(&EponQThr, 0, sizeof(QDMA_EponQueueThreshold_T));
		if((*bitmap)&(1<<queueIndex)){//get queue threshold
			EponQThr.channel = llidIndex;
			EponQThr.queue = queueIndex ;
			EponQThr.thrIdx = setIndex;
			ret = epon_get_queue_threshold(&EponQThr);
			if(ret < 0){
				eponDbgPrint(EPON_DEBUG_LEVEL_ERR, "\r\neponGetLlidQueThrod error");	
				return -1;
			}
			threshold[queueIndex] = EponQThr.value;
			
		}else{
			threshold[queueIndex] = 0;
		}
	}
	
	return 0;
}



/*
	eponSetMpcpTime
	mpcpTime: MPCP timeout setting, ms
	return : 0: OK
			-1:fail
*/
int eponSetMpcpTime(__u16 mpcpTime ){
	__u32 Raw = 0;
	__u32 Raw_value = 0;
	Raw = mpcpTime/2;
	if(Raw > 0x3ff){
		eponDbgPrint(EPON_DEBUG_LEVEL_ERR, "\r\neponSetMpcpTime time is too big");	
		return -1;
	}else if (Raw < 7){
		/* Under some OLT, 13ms maybe too small, so the min is 14ms(Raw=7, unit is 2ms) */
		Raw = 7;
	}
	Raw &= 0x3ff;
	Raw_value = READ_REG_WORD(e_mpcp_timeout_intvl);
	Raw_value &= ~(0x3ff);
	Raw_value |= Raw;
	eponDbgPrint(EPON_DEBUG_LEVEL_TRACE, "eponSetMpcpTime time = %d", mpcpTime);	
	WRITE_REG_WORD(e_mpcp_timeout_intvl , Raw_value);
	return 0;	
}



/*
	eponGetMpcpTime
	mpcpTime: MPCP timeout setting, ms
	return : 0: OK
			-1:fail
*/
int eponGetMpcpTime(__u16 *mpcpTime ){
	__u32 Raw = 0;
	Raw = READ_REG_WORD(e_mpcp_timeout_intvl);
	Raw &= 0x3ff;
	Raw <<=1;
	*mpcpTime = Raw;
	eponDbgPrint(EPON_DEBUG_LEVEL_TRACE, "eponGetMpcpTime time = %d", *mpcpTime);	
	return 0;	
}



/*
eponSetHoldoverCfg

enable:
	0: disable
	1: enable
time: 
	ms
*/
int eponSetHoldoverCfg(__u8 enable, __u16 time){
	eponDbgPrint(EPON_DEBUG_LEVEL_TRACE, "eponSetHoldoverCfg enable = %d, time=%d", enable, time);	
	
	eponDrv.hldoverEnable = enable;
	eponDrv.hldOverTime = time;
	return 0;
}


/*
eponGetHoldoverCfg

enable:
	0: disable
	1: enable
time: 
	ms
*/

int eponGetHoldoverCfg(__u8 *enable, __u16 *time){
	eponDbgPrint(EPON_DEBUG_LEVEL_TRACE, "eponGetHoldoverCfg enable = %d, time=%d", eponDrv.hldoverEnable, eponDrv.hldOverTime);	
	*enable = eponDrv.hldoverEnable;
	*time = eponDrv.hldOverTime ;
	return 0;
}

/*______________________________________________________________________________
**	function name
**		eponGetMpcpCounter
**	description:
**		get mpcp cnt
**	parameters:
**		mpcpCnt
**	global:
**		None
**	return:
**		0:success
**     -1:fail
**	call:
**		REGISTER_ACTION_EPON_DBG_GET_STATIS_CNT
**	revision:
**		v1.0
**____________________________________________________________________________*/

int eponGetMpcpCounter(__u8 llidIndex, eponMpcpCnt_t *mpcpCnt )
{
	XPON_DEBUG_STATISTIC tmpReg;
	memset(&tmpReg, 0, sizeof(XPON_DEBUG_STATISTIC));

	mpcpcnt_llid = llidIndex;
	get_register_count(&tmpReg);
	
	if((llidIndex >= EPON_1G_MAX_LLID_NUM) || (mpcpCnt == NULL)){
		eponDbgPrint(EPON_DEBUG_LEVEL_ERR, "\r\eponGetMpcpCounter parameters error");
		return -1;
	}

 	eponDbgPrint(EPON_DEBUG_LEVEL_TRACE, "eponGetMpcpCounter");	
	if( (llidIndex >= EPON_1G_MAX_LLID_NUM) || (mpcpCnt == NULL) )
	{
		eponDbgPrint(EPON_DEBUG_LEVEL_ERR,"\r\n eponGetMpcpCounter param error");	
		return -1;
	}

	mpcpCnt->rxHecErrorCnt = (uint16_t)tmpReg.mpcpErrCnt + stackMpcpErrCnt;
	mpcpCnt->txRegQeqCnt = tmpReg.mpcpRgstReqCnt;
	mpcpCnt->rxRegCnt = tmpReg.mpcpRgstCnt;
	mpcpCnt->txRegAckCnt = tmpReg.mpcpRgstAckCnt;
	mpcpCnt->rxDiscGateCnt = tmpReg.mpcpDscvGateCnt;
	mpcpCnt->rxNormalGateCnt = tmpReg.mpcpNormalGateCnt;
	mpcpCnt->txReportCnt = tmpReg.mpcpTxReportCnt;
	
	return 0;
}

/*______________________________________________________________________________
**	function name
**		eponSetLedStatus
**	description:
**		set led status
**	parameters:
**		pon_led_status
**	global:
**		None
**	return:
**		0:success
**     -1:fail
**	call:
**		change_pon_led_status
**	revision:
**		v1.0
**____________________________________________________________________________*/
int eponSetLedStatus(XPON_ALARM_LED_Status_t pon_led_status)
{	
	change_pon_led_status(pon_led_status);
	return 0;
}

void eponUpdateStackMpcpErrCnt(void)
{	
	XPON_DEBUG_STATISTIC tmpReg;	
	memset(&tmpReg, 0, sizeof(XPON_DEBUG_STATISTIC));
	
	stackMpcpErrCnt = stackMpcpErrCnt + (uint16_t)tmpReg.mpcpErrCnt;		
	
	return;

}


/*______________________________________________________________________________
**	function name
**		eponSetStatisticClean
**	description:
**		set epon statistic clean
**	parameters:
**		isClearSta
**	global:
**		None
**	return:
**		0:success
**     -1:fail
**	call:
**		WRITE_REG_WORD
**	revision:
**		v1.0
**____________________________________________________________________________*/

int eponSetStatisticClean(uint8_t isClearSta)
{
	if(isClearSta){
		eponUpdateStackMpcpErrCnt();
		WRITE_REG_WORD(e_mpcp_stat,0);
    	WRITE_REG_WORD(e_mpcp_rgst_stat,0);
		WRITE_REG_WORD(e_rxmpi_eth_cnt,0);
		WRITE_REG_WORD(e_rxmbi_eth_cnt,0);
		WRITE_REG_WORD(e_txmpi_eth_cnt,0);
		WRITE_REG_WORD(e_txmbi_eth_cnt,0);
		WRITE_REG_WORD(e_llid0_gnt_stat,0);
		WRITE_REG_WORD(e_llid1_gnt_stat,0);
		WRITE_REG_WORD(e_llid2_gnt_stat,0);
		WRITE_REG_WORD(e_llid3_gnt_stat,0);
		WRITE_REG_WORD(e_llid4_gnt_stat,0);
		WRITE_REG_WORD(e_llid5_gnt_stat,0);
		WRITE_REG_WORD(e_llid6_gnt_stat,0);
		WRITE_REG_WORD(e_llid7_gnt_stat,0);
		WRITE_REG_WORD(e_rpt_cnt,0);
		WRITE_REG_WORD(e_oam_stat,0);
#if defined(TCSUPPORT_CPU_EN7528) || defined(TCSUPPORT_CPU_EN7523) || defined(TCSUPPORT_CPU_AN7552)
		WRITE_REG_WORD(e_rx_crc32_cnt,0);
#endif
		printk("Epon MAC statistic counter clean.\n");
	}
	return 0;
}



int eponGetDataTxCounter(eponTxCnt_t *txCnt ){

    FE_TxCnt_t Fe_txCnt = {0};
    
	eponDbgPrint(EPON_DEBUG_LEVEL_TRACE, "eponGetDataTxCounter");	
	
	if(txCnt == NULL){
		eponDbgPrint(EPON_DEBUG_LEVEL_ERR, "eponGetDataTxCounter txCnt == NULL");	
		return -1;
	}	

	
	FE_API_GET_ETH_TX_CNT(&Fe_txCnt);
	txCnt->txFrameCnt = Fe_txCnt.txFrameCnt;
	txCnt->txFrameLen = Fe_txCnt.txFrameLen;
	txCnt->txDropCnt = Fe_txCnt.txDropCnt;
	txCnt->txBroadcastCnt = Fe_txCnt.txBroadcastCnt;
	txCnt->txMulticastCnt = Fe_txCnt.txMulticastCnt;
	txCnt->txLess64Cnt = Fe_txCnt.txLess64Cnt;
	txCnt->txMore1518Cnt = Fe_txCnt.txMore1518Cnt;
	txCnt->txEq64Cnt = Fe_txCnt.txEq64Cnt;
	txCnt->txFrom65To127Cnt = Fe_txCnt.txFrom65To127Cnt;
	txCnt->txFrom128To255Cnt = Fe_txCnt.txFrom128To255Cnt;
	txCnt->txFrom256To511Cnt = Fe_txCnt.txFrom256To511Cnt;
	txCnt->txFrom512To1023Cnt = Fe_txCnt.txFrom512To1023Cnt;
	txCnt->txFrom1024To1518Cnt = Fe_txCnt.txFrom1024To1518Cnt;

	return 0;
}



int eponGetDataRxCounter(eponRxCnt_t *rxCnt){
    FE_RxCnt_t Fe_rxCnt={0};
    
	eponDbgPrint(EPON_DEBUG_LEVEL_TRACE, "eponGetDataRxCounter");	
	if(rxCnt == NULL){
		eponDbgPrint(EPON_DEBUG_LEVEL_ERR, "\r\neponGetDataRxCounter rxCnt == NULL");	
		return -1;
	}	

	
	FE_API_GET_ETH_RX_CNT(&Fe_rxCnt);
	rxCnt->rxFrameCnt = Fe_rxCnt.rxFrameCnt;
	rxCnt->rxFrameLen = Fe_rxCnt.rxFrameLen;
	rxCnt->rxDropCnt = Fe_rxCnt.rxDropCnt;
	rxCnt->rxBroadcastCnt = Fe_rxCnt.rxBroadcastCnt;
	rxCnt->rxMulticastCnt = Fe_rxCnt.rxMulticastCnt;
	rxCnt->rxCrcCnt = Fe_rxCnt.rxCrcCnt;
	rxCnt->rxFragFameCnt = Fe_rxCnt.rxFragFameCnt;
	rxCnt->rxJabberFameCnt = Fe_rxCnt.rxJabberFameCnt;
	rxCnt->rxLess64Cnt = Fe_rxCnt.rxLess64Cnt;
	rxCnt->rxMore1518Cnt = Fe_rxCnt.rxMore1518Cnt;
	rxCnt->rxEq64Cnt = Fe_rxCnt.rxEq64Cnt;
	rxCnt->rxFrom65To127Cnt = Fe_rxCnt.rxFrom65To127Cnt;
	rxCnt->rxFrom128To255Cnt = Fe_rxCnt.rxFrom128To255Cnt;
	rxCnt->rxFrom256To511Cnt = Fe_rxCnt.rxFrom256To511Cnt;
	rxCnt->rxFrom512To1023Cnt = Fe_rxCnt.rxFrom512To1023Cnt;
	rxCnt->rxFrom1024To1518Cnt = Fe_rxCnt.rxFrom1024To1518Cnt;

	return 0;
}


int eponClearGdm2MIB(__u8 rxCntClear, __u8 txCntClear){
	if(txCntClear&0x1)
		FE_API_SET_CLEAR_MIB(FE_GDM_SEL_GDMA2, FE_GDM_SEL_TX);
	else if(rxCntClear&0x1)
		FE_API_SET_CLEAR_MIB(FE_GDM_SEL_GDMA2, FE_GDM_SEL_RX);

	return 0;
}

#include "pwan/xpon_netif.h"
void eponMacSetTxbufUsage(__u32 llidMask)
{
	extern PWAN_GlbPriv_T *gpWanPriv;

	__u8 llidIndex = 0;
	__u8 llidCnt = 0;
	__u32 value = 0;
	for( llidIndex = 0; llidIndex< EPON_1G_MAX_LLID_NUM; llidIndex++ ){
		if(((1<<llidIndex)&llidMask) != 0)
			llidCnt++;
	}

	gpWanPriv->activeChannelNum = llidCnt;
	if(llidCnt <= 1){
		if (gpWanPriv->devCfg.flags.isQosUp == 1)
			WRITE_FE_REG(0xBFB51890, 0x800020C0);
		else
			WRITE_FE_REG(0xbfb51890,0);
	}
	else{
		value = 0x80/llidCnt;
		if (gpWanPriv->devCfg.flags.isQosUp == 1 && value > 0x20)
			value = 0x20;
		value <<= 8;
		value |= 0x80000080;
		WRITE_FE_REG(0xbfb51890,value);
	}
}

int eponMacSetLlidEnableMask(__u32 llidMask){
	__u8 llidIndex = 0;
	
	eponDbgPrint(EPON_DEBUG_LEVEL_ERR , "\r\nenter eponMacSetLlidEnableMask = %x", llidMask);	
	
	eponMacSetTxbufUsage(llidMask);
	
	if(eponDrv.llidMask == llidMask){
		eponDbgPrint(EPON_DEBUG_LEVEL_TRACE, "LLID mask is the same");
		return 0;
	}

	

	for( llidIndex = 0; llidIndex< EPON_1G_MAX_LLID_NUM; llidIndex++ ){
		if((((1<<llidIndex)&(eponDrv.llidMask)) == 0)&&(((1<<llidIndex)&(llidMask)) != 0)){
			eponLlidEnable(llidIndex);
		}
		if((((1<<llidIndex)&(eponDrv.llidMask)) != 0)&&(((1<<llidIndex)&(llidMask)) == 0)){
			eponLlidDisable(llidIndex);
		}
	}
	eponDrv.llidMask = llidMask;
	return 0;
}


void PhyTxLedConf(int value)
{	
	if (value == PHY_DISABLE)
	{	
#if defined(TCSUPPORT_CT_PON_CN_CN) 
		change_alarm_led_status(ALARM_LED_FLICKER);
#else
		change_alarm_led_status(ALARM_LED_ON);
#endif
	}
	else if(value == PHY_ENABLE)
	{
		if ((XPON_PHY_GET(PON_GET_PHY_LOS_STATUS) == PHY_NO_LOS_HAPPEN) /* no los and phy ready stats led should be configed */
			&& (XPON_PHY_GET(PON_GET_PHY_READY_STATUS) == PHY_TRUE)
			&& (TRUE != XPON_PHY_GET(PON_GET_PHY_GET_TX_POWER_EN_FLAG) )) /* enable should not config twice */
		{
			change_alarm_led_status(ALARM_LED_OFF);
#if !defined(TCSUPPORT_C9_ROST_LED)
			change_pon_led_status(ALARM_LED_FLICKER);
#endif
			}
	}
	return;
}

extern struct timer_list eponTxPwrDwnTmr;
void eponPhyTxPwrUp(TIMER_FUN_PAAM lparam){
	eponDbgPrint(1, "eponPhyTxPwrUp timer done, restart phy power\n");
	PhyTxLedConf(PHY_ENABLE);
	XPON_PHY_TX_POWER_CONFIG(PHY_ENABLE);
	turnOffOpticalModuel = 0;
	if (XPON_PHY_TX_ENABLE()==PHY_FAILURE){
		eponDbgPrint(1, "\r\n Epon set PHY Tx Power Enable failed!");
	}
}
int eponPhyTxPowerCtl(__u32 time)
{
	
	if (time == 0) {// re-enabled
#ifdef TCSUPPORT_CT_PON // for PHY CONT Burst	
		if (PHY_FALSE == XPON_PHY_GET(PON_GET_PHY_TX_LONG_FLAG))
#endif
		{
			eponDbgPrint(1, "Epon open phy power!\n");
            if (timer_pending(&eponTxPwrDwnTmr)){
			    del_timer(&eponTxPwrDwnTmr);
		    }
			rogueOnuDisableTxPowerFlag = 0;
			turnOffOpticalModuel = 0;
			PhyTxLedConf(PHY_ENABLE);
			XPON_PHY_TX_POWER_CONFIG(PHY_ENABLE);
			XPON_PHY_TX_ENABLE();
		}
	}else if (time == 65535) {// disabled tx power
		eponDbgPrint(1, "Epon close phy power forever!\n");
		rogueOnuDisableTxPowerFlag = 1;
		turnOffOpticalModuel = 1;
		/* clear timer when tx power disable  */
		if (timer_pending(&eponTxPwrDwnTmr)){
			del_timer(&eponTxPwrDwnTmr);
		}
		XPON_PHY_TX_DISABLE();
		XPON_PHY_TX_POWER_CONFIG(PHY_DISABLE);
		PhyTxLedConf(PHY_DISABLE);
	}else if (time > 0 && time < 65535) {
		eponDbgPrint(1, "\r\n Epon close phy power %d sec!", time);
		rogueOnuDisableTxPowerFlag = 1;
		turnOffOpticalModuel = 1;
	    XPON_PHY_TX_DISABLE();
		XPON_PHY_TX_POWER_CONFIG(PHY_DISABLE);
		PhyTxLedConf(PHY_DISABLE);
		eponTxPwrDwnTmr.expires = jiffies + time*HZ;
		mod_timer(&eponTxPwrDwnTmr, jiffies + time*HZ);
	}
	return 0;
}

int eponSetLlidRxFec(__u8 llidIndex, __u8 fecFlag)
{
    g_rx_fec = fecFlag;
    return 0;
}

int eponGetLlidRxFec(__u8 llidIndex, __u8 *fecFlag)
{
    *fecFlag = g_rx_fec;
    return 0;
}

int eponGetLlidKeyInuse(__u8 llidIndex, __u8 *keyIndex)
{
	if(llidIndex >= EPON_1G_MAX_LLID_NUM)
	{
		eponDbgPrint(EPON_DEBUG_LEVEL_ERR, "\r\neponSetLlidKey param error");	
		return -1;
	}

    *keyIndex = g_llid_key_index[llidIndex];
    
    return 0;
}
/***************************************************************
***************************************************************/
int eponGetOnuMac(unsigned char *onuMac){
	if(onuMac == NULL){
		eponDbgPrint(EPON_DEBUG_LEVEL_ERR, "\r\nonuMAC is NULL");	
		return -1;
	}
	
	memcpy(onuMac , eponOnuMacAddr ,6);
	eponDbgPrint(EPON_DEBUG_LEVEL_NONE,"MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
		eponOnuMacAddr[0],eponOnuMacAddr[1],eponOnuMacAddr[2],eponOnuMacAddr[3],eponOnuMacAddr[4],eponOnuMacAddr[5]);
    
	return 0;
}

int eponSetPonMacAddr(unsigned char *epon_mac)
{
    printk("line = %d, %02x:%02x:%02x:%02x:%02x:%02x", __LINE__, epon_mac[0],epon_mac[1],epon_mac[2],epon_mac[3],epon_mac[4],epon_mac[5]);
	memcpy(eponOnuMacAddr , epon_mac ,6);
	xmcs_report_event(XMCS_EVENT_TYPE_EPON, XMCS_EVENT_EPON_MAC_CHANGED, 0);
	return 0;
}

int eponGetLlidValue(__u8 llidIndex, __u16 *llidValue)
{
	eponLlidDscvStsReg_t llidDscvSts;

	if(llidIndex >= EPON_1G_MAX_LLID_NUM)
	{
		eponDbgPrint(EPON_DEBUG_LEVEL_ERR, "\r\neponGetLlidValue param error");	
		return -1;
	}
	
	llidDscvSts.Raw = READ_REG_WORD(e_llid0_dscvry_sts + llidIndex);

    *llidValue = llidDscvSts.Bits.llidValue;
	
    return 0;
}

int eponXponLedCtl(int value){
	switch(value){
		case XPON_LED_OFF_MODE:
			change_pon_led_status(ALARM_LED_OFF);
			break;
		case XPON_LED_FLICKER_MODE:
			change_pon_led_status(ALARM_LED_FLICKER);
			break;
		case XPON_LED_ON_MODE:
			change_pon_led_status(ALARM_LED_ON);
			break;
		case XPON_LED_SW_UPGRADE_MODE:
			change_pon_led_status(ALARM_LED_OFF);
			break;
			
		default:
			break;
	}
	return 0;
}

long eponMacIoctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int result = 0; 
	eponMacIoctl_t temp  = {0};
    eponMacIoctl_t *opt3 = NULL;
    
    result = copy_from_user(&temp, (eponMacIoctl_t *)arg, sizeof(eponMacIoctl_t));
	if(0 != result){
		printk("func:%s copy_from_user failed\n",__func__);
		return 0;
	}
    opt3 = &temp;
    
#ifdef TCSUPPORT_AUTOBENCH
#ifdef TCSUPPORT_CPU_ARMV8
	if((GET_WAN_CONF() & 0xff) != 1)
		return 0;
#else
	if((IO_GREG(0xbfb00070) & 0x7) != 1)
		return 0;
#endif
#endif
	eponDbgPrint(EPON_DEBUG_LEVEL_TRACE, "cmd = %d", cmd);	
	switch(cmd)
	{
	    case EPON_IOCTL_SET_LLID_ENABLE_MASK:
			eponDbgPrint(EPON_DEBUG_LEVEL_TRACE, "eponMacIoctl = EPON_IOCTL_SET_LLID_ENABLE_MASK");
			result = eponMacSetLlidEnableMask(opt3->param2);
			break;
		case EPON_IOCTL_LLID_DEREGISTER:
			eponDbgPrint(EPON_DEBUG_LEVEL_TRACE, "eponMacIoctl = EPON_IOCTL_LLID_DEREGISTER");
			result = eponMpcpLocalDergstr(opt3->llidIndex);
			break;
		case EPON_IOCTL_LLID_DISCV_RGSTACK:			
			eponDbgPrint(EPON_DEBUG_LEVEL_TRACE, "eponMacIoctl = EPON_IOCTL_LLID_DISCV_RGSTACK");
			
			result = eponMpcpSetDiscvRgstAck(opt3->param0 , opt3->info[0]);
			break;
		case EPON_IOCTL_SET_LLID_KEY:
			eponDbgPrint(EPON_DEBUG_LEVEL_TRACE, "eponMacIoctl = EPON_IOCTL_SET_LLID_KEY");
			result = eponSetLlidKey(opt3->llidIndex , opt3->param0 , opt3->info);
			break;
		case EPON_IOCTL_GET_LLID_KEY:
			eponDbgPrint(EPON_DEBUG_LEVEL_TRACE, "eponMacIoctl = EPON_IOCTL_GET_LLID_KEY");
			result = eponGetLlidKey(opt3->llidIndex , opt3->param0 , opt3->info);
			break;
		case EPON_IOCTL_SET_LLID_TX_FEC:
			eponDbgPrint(EPON_DEBUG_LEVEL_TRACE, "eponMacIoctl = EPON_IOCTL_SET_LLID_TX_FEC");
			result = eponSetLlidFec(opt3->llidIndex , opt3->param0);
			break;
		case EPON_IOCTL_GET_LLID_TX_FEC:
			eponDbgPrint(EPON_DEBUG_LEVEL_TRACE, "eponMacIoctl = EPON_IOCTL_GET_LLID_TX_FEC");
			result = eponGetLlidFec(opt3->llidIndex, &(opt3->param0));
			break;
		case EPON_IOCTL_MAC_RST:
			//eponMacRestart();
			eponMacReinit(FE_CHANNEL_RETIRE_ENABLE);
			eponDbgPrint(EPON_DEBUG_LEVEL_TRACE, "eponMacIoctl = EPON_IOCTL_MAC_RST");
			break;
		case EPON_IOCTL_SET_LLID_DBA_THRSHLD_NUM:
			eponDbgPrint(EPON_DEBUG_LEVEL_TRACE, "eponMacIoctl = EPON_IOCTL_SET_LLID_DBA_QUE_NUM");
			result = eponSetLlidThrshldNum(opt3->llidIndex , opt3->param0);
			break;
		case EPON_IOCTL_GET_LLID_DBA_THRSHLD_NUM:
			eponDbgPrint(EPON_DEBUG_LEVEL_TRACE, "eponMacIoctl = EPON_IOCTL_GET_LLID_DBA_QUE_NUM");
			result = eponGetLlidThrshldNum(opt3->llidIndex , opt3->info);
			break;
		case EPON_IOCTL_SET_LLID_DBA_THROD:
			eponDbgPrint(EPON_DEBUG_LEVEL_TRACE, "eponMacIoctl = EPON_IOCTL_SET_LLID_DBA_THROD");
			result = eponSetLlidQueThrod(opt3->llidIndex , opt3->param0 , opt3->info[0] , (__u16 *)&opt3->info[1]);
			break;
		case EPON_IOCTL_GET_LLID_DBA_THROD:
			eponDbgPrint(EPON_DEBUG_LEVEL_TRACE, "eponMacIoctl = EPON_IOCTL_GET_LLID_DBA_THROD");
			result = eponGetLlidQueThrod(opt3->llidIndex , opt3->param0 , &(opt3->info[0]) , (__u16 *)&opt3->info[1]);
			break;
		case EPON_IOCTL_SET_MPCP_TIME:
			eponDbgPrint(EPON_DEBUG_LEVEL_TRACE, "eponMacIoctl = EPON_IOCTL_SET_MPCP_TIME");
			result = eponSetMpcpTime(opt3->param1 );
			break;
		case EPON_IOCTL_GET_MPCP_TIME:
			eponDbgPrint(EPON_DEBUG_LEVEL_TRACE, "eponMacIoctl = EPON_IOCTL_GET_MPCP_TIME");
			result = eponGetMpcpTime(&(opt3->param1) );
			break;
		case EPON_IOCTL_SET_HOLDOVER_CFG:
			eponDbgPrint(EPON_DEBUG_LEVEL_TRACE, "eponMacIoctl = EPON_IOCTL_SET_HOLDOVER_CFG");
			result = eponSetHoldoverCfg(opt3->param0,  opt3->param1);
			break;
		case EPON_IOCTL_GET_HOLDOVER_CFG:
			eponDbgPrint(EPON_DEBUG_LEVEL_TRACE, "eponMacIoctl = EPON_IOCTL_GET_HOLDOVER_CFG");
			result = eponGetHoldoverCfg(&(opt3->param0), &(opt3->param1));
			break;
		case EPON_IOCTL_GET_TX_ETH_MIB:
			eponDbgPrint(EPON_DEBUG_LEVEL_TRACE, "eponMacIoctl = EPON_IOCTL_GET_TX_ETH_MIB");
			result = eponGetDataTxCounter((eponTxCnt_t *)(opt3->info) );
			break;
		case EPON_IOCTL_GET_RX_ETH_MIB:
			eponDbgPrint(EPON_DEBUG_LEVEL_TRACE, "eponMacIoctl = EPON_IOCTL_GET_RX_ETH_MIB");
			result = eponGetDataRxCounter((eponRxCnt_t *)(opt3->info) );
			break;		
		case EPON_IOCTL_CLR_TX_RX_MIB:
			eponDbgPrint(EPON_DEBUG_LEVEL_TRACE, "eponMacIoctl = EPON_IOCTL_CLR_TX_RX_MIB");
			result = eponClearGdm2MIB(opt3->info[0] , opt3->info[1] );
			break;	
		case EPON_IOCTL_POWER_CTL:
			eponDbgPrint(EPON_DEBUG_LEVEL_TRACE, "eponMacIoctl = EPON_IOCTL_POWER_CTL");
			result = eponPhyTxPowerCtl(opt3->param2);
			break;
		case EPON_IOCTL_AUTH_FAIL_CTL:
			eponDbgPrint(EPON_DEBUG_LEVEL_TRACE, "eponMacIoctl = EPON_IOCTL_AUTH_FAIL_CTL");
			isAuthFail = opt3->param0;
			break;
        case EPON_IOCTL_SET_SILENCETIME:
            g_silence_time = opt3->param1;
            break;
		case EPON_IOCTL_TXOAM_FAV_SET:
            eponMacTxOamFav(opt3->param2);
            break;
		case EPON_IOCTL_AUTO_SWITCH_OAM_FAV:
			auto_switch_oam_fav = opt3->param2;
			break;
        case EPON_IOCTL_SET_LLID_RX_FEC:
			result = eponSetLlidRxFec(opt3->llidIndex , opt3->param0);
			break;
        case EPON_IOCTL_GET_LLID_RX_FEC:
            result = eponGetLlidRxFec(opt3->llidIndex, &(opt3->param0));
            break;
        case EPON_IOCTL_SET_LLID_MAC:
            result = eponSetPonMacAddr(opt3->info);
            break;
        case EPON_IOCTL_GET_LLID_KEY_INUSE:
            result = eponGetLlidKeyInuse(opt3->llidIndex, &(opt3->param0));
            break;
		case EPON_IOCTL_GET_ONU_MAC :		
			result = eponGetOnuMac(opt3->info);
			break;
		case EPON_IOCTL_SET_DEBUG_LEVEL:
			eponDbgPrint(EPON_DEBUG_LEVEL_TRACE, "eponMacIoctl = EPON_IOCTL_SET_DEBUG_LEVEL");
			eponDebugLevel = opt3->param0;
			break;
		case EPON_IOCTL_SET_PHY_BURST_EN:
			eponDbgPrint(EPON_DEBUG_LEVEL_TRACE, "eponMacIoctl = EPON_IOCTL_SET_PHY_BURST_EN");
			result = XPON_PHY_SET_EPON_TS_CONTINUE_MODE(opt3->param2);
			break;
		case EPON_IOCTL_SET_DYING_GASP_MODE:
			eponDbgPrint(EPON_DEBUG_LEVEL_TRACE, "eponMacIoctl = EPON_IOCTL_SET_DYING_GASP_MODE");
			result = eponHwDygaspCtrl(opt3->param0);
			break;
		case EPON_IOCTL_GET_DYING_GASP_MODE:
			eponDbgPrint(EPON_DEBUG_LEVEL_TRACE, "eponMacIoctl = EPON_IOCTL_GET_DYING_GASP_MODE");
			result = eponDevGetDyingGaspMode(&(opt3->param0));
			break;
		case EPON_IOCTL_SET_DYING_GASP_NUM:
			eponDbgPrint(EPON_DEBUG_LEVEL_TRACE, "eponMacIoctl = EPON_IOCTL_SET_DYING_GASP_NUM");
			result = eponDevSetDyingGaspNum(opt3->param2);
			break;
		case EPON_IOCTL_GET_DYING_GASP_NUM:
			eponDbgPrint(EPON_DEBUG_LEVEL_TRACE, "eponMacIoctl = EPON_IOCTL_GET_DYING_GASP_NUM");
			result = eponDevGetDyingGaspNum(&(opt3->param2));
			break;
#ifdef TCSUPPORT_EPON_POWERSAVING
		case EPON_IOCTL_SET_EARLY_WAKEUP:
			eponDbgPrint(EPON_DEBUG_LEVEL_TRACE, "eponMacIoctl = EPON_IOCTL_SET_EARLY_WAKEUP");
			result = eponSetEarlyWakeUpConfig(opt3->param0,opt3->param2,opt3->param1);
			break;
#endif
		case EPON_IOCTL_SET_SNIFFER_MODE:
			eponDbgPrint(EPON_DEBUG_LEVEL_TRACE, "eponMacIoctl = EPON_IOCTL_SET_SNIFFER_MODE");
			result = eponSetSnifferModeConfig(opt3->param0,opt3->param1);
			break;
		case EPON_IOCTL_GET_LLID_VALUE:
			eponDbgPrint(EPON_DEBUG_LEVEL_TRACE, "eponMacIoctl = EPON_IOCTL_GET_LLID_VALUE");
			result = eponGetLlidValue(opt3->llidIndex, &(opt3->param1));
			break;
		case EPON_IOCTL_XPON_LED_CTL:
			eponDbgPrint(EPON_DEBUG_LEVEL_TRACE, "eponMacIoctl = EPON_IOCTL_XPON_LED_CTL");
			result = eponXponLedCtl(opt3->param2);
			break;
		case EPON_IOCTL_GET_LLID_MPCP_COUNTER:
			eponDbgPrint(EPON_DEBUG_LEVEL_TRACE, "eponMacIoctl = EPON_IOCTL_GET_LLID_MPCP_COUNTER");
			result = eponGetMpcpCounter(opt3->llidIndex, (eponMpcpCnt_t *)(opt3->info));
			result = copy_to_user((eponMacIoctl_t *)arg, opt3, sizeof(eponMacIoctl_t));
			break;
		case EPON_IOCTL_SET_XPON_LED_STATUS:
			eponDbgPrint(EPON_DEBUG_LEVEL_TRACE, "eponMacIoctl = EPON_IOCTL_SET_XPON_LED_STATUS");
			result = eponSetLedStatus(opt3->param0);
			break;
		case EPON_IOCTL_SET_CLEAR_STATISTICS:
			eponDbgPrint(EPON_DEBUG_LEVEL_TRACE, "eponMacIoctl = EPON_IOCTL_SET_CLEAR_STATISTICS");
			result = eponSetStatisticClean(opt3->param0);
			break;
			
		default:
			break;
	}

    if(0 != copy_to_user((eponMacIoctl_t *)arg, opt3, sizeof(eponMacIoctl_t))){
        printk("fuc:%s copy_to_user fail\n",__func__);
        return -1;
    }
	return result;
}






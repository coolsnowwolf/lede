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
#include "../../inc/common/phy_if_wrapper.h"
#include <asm/tc3162/ledcetrl.h>


#include "epon/epon.h"
#ifdef TCSUPPORT_CPU_EN7521
#include <ecnt_hook/ecnt_hook_fe.h>
#include <ecnt_hook/ecnt_hook_qdma.h>
#include "qdma_bmgr.h"
#else
#include "qdma_api.h"
#include "../../../raeth/fe_api.h"
#include "epon/fe_reg.h"
#endif


extern __u32 eponDebugLevel ;
extern int isAuthFail;
extern epon_t eponDrv;
extern __u8 eponOnuMacAddr[6];
extern __u8 g_silence_time;


int eponSetLlidKey(__u8 llidIndex, __u8 keyIndex, __u8 *key){
	REG_e_security_key_cfg eponSecurityKeyCfgReg;
	__u32 keyValue = 0;
	__u32 timers = 0;
	__u8 *cp = NULL;
	memset(&eponSecurityKeyCfgReg , 0 , sizeof(REG_e_security_key_cfg));
	
	if((llidIndex >= EPON_LLID_MAX_NUM)||(keyIndex >1)||(key == NULL)){
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
		eponDbgPrint(EPON_DEBUG_LEVEL_ERR, "\r\n eponSetLlidKey set 1 timers == 100");
		return -1;
	}
#endif
	cp = (__u8 *)(&keyValue);
	cp++;
	memcpy(cp , key , 3);
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

	
	if((llidIndex >= EPON_LLID_MAX_NUM)||(keyIndex >1)||(key == NULL)){
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

	eponDbgPrint(EPON_DEBUG_LEVEL_TRACE, "\r\nenter eponSetLlidFec");	
	if((llidIndex >= EPON_LLID_MAX_NUM)){
		eponDbgPrint(EPON_DEBUG_LEVEL_ERR, "\r\neponSetLlidFec param error");	
		return -1;
	}


	if(llidIndex < 4){
		regAddr = e_llid0_3_cfg;
		offset = llidIndex*8 + 3;
	}
	else{
		regAddr = e_llid4_7_cfg;
		offset = (llidIndex - 4)*8 + 3;
	}

	Raw = READ_REG_WORD(regAddr);
	if(fecFlag == 1){//enable
		Raw |= (1<<offset);
	}else{//disable
		Raw &= ~(1<<offset);
	}
	
	WRITE_REG_WORD(regAddr , Raw);
	return 0;
}



int eponGetLlidFec(__u8 llidIndex, int *fecFlag){
	__u32 regAddr = 0;
	__u32 Raw = 0;
	__u8 offset = 0;
	
	if((llidIndex >= EPON_LLID_MAX_NUM)){
		eponDbgPrint(EPON_DEBUG_LEVEL_ERR, "\r\neponSetLlidFec param error");	
		return -1;
	}

	if(llidIndex < 4){
		regAddr = e_llid0_3_cfg;
		offset = llidIndex*8 + 3;
	}
	else{
		regAddr = e_llid4_7_cfg;
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



#ifdef TCSUPPORT_CPU_EN7521
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
	__u32 regAddr = e_rpt_cfg;
	__u32 Raw =0;
	eponRptCfgReg_t eponRptCfgReg;
	eponRptCfgReg.Raw = 0;	


	eponDbgPrint(EPON_DEBUG_LEVEL_TRACE, "\r\nenter eponSetLlidThrshldNum llidIndex=%d ,num=%d ", llidIndex, num);	
	if((llidIndex >= EPON_LLID_MAX_NUM)||(num > 3)){
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
	__u32 regAddr = e_rpt_cfg;
	//__u32 mask =0;
	eponRptCfgReg_t eponRptCfgReg;
	eponRptCfgReg.Raw = 0;	
	


	eponDbgPrint(EPON_DEBUG_LEVEL_TRACE, "\r\nenter eponGetLlidQueNum");	
	if((llidIndex >= EPON_LLID_MAX_NUM)){
		eponDbgPrint(EPON_DEBUG_LEVEL_ERR, "\r\neponGetLlidQueNum param error");	
		return -1;
	}

	eponRptCfgReg.Raw = READ_REG_WORD(regAddr);
	eponRptCfgReg.Raw &= (3<<(llidIndex*2));
	eponRptCfgReg.Raw >>= (3<<(llidIndex*2));
	num[0] = (__u8)eponRptCfgReg.Raw;

	eponDbgPrint(EPON_DEBUG_LEVEL_TRACE, "eponGetLlidQueNum eponRptCfgReg.Raw=%x", num[0]);	
	//WRITE_REG_WORD(regAddr , eponRptCfgReg.Raw);
	return 0;
}
#endif

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




int eponSetLlidQueThrod(__u8 llidIndex, __u8 setIndex, __u8 mask,__u16 *threshold ){
	__u8 queueIndex = 0;
	//__u32 pollTime = 0;
#ifndef TCSUPPORT_CPU_EN7521
	QDMA_EponQueueThreshold_T EponQThr;
#else
	eponQueueThreshold_t EponQThr;
#endif
	int ret = 0;
	

	for(queueIndex = 0; queueIndex< 8;queueIndex++){
		if(mask&(1<<queueIndex)){//set queue threshold
			EponQThr.channel = llidIndex;
			EponQThr.queue = queueIndex;
			EponQThr.thrIdx = setIndex;
			EponQThr.value = threshold[queueIndex];
#ifndef TCSUPPORT_CPU_EN7521
			ret = qdma_set_epon_queue_threshold(&EponQThr) ;
#else
			ret = epon_set_queue_threshold(&EponQThr) ;
#endif
			if(ret < 0 ){
				eponDbgPrint(EPON_DEBUG_LEVEL_ERR, "\r\neponSetLlidQueThrod error");	
				return -1;
			}
		}
	}
	return 0;
}





int eponGetLlidQueThrod(__u8 llidIndex, __u8 setIndex, __u8 mask,__u16 *threshold ){
	__u8 queueIndex = 0;
#ifndef TCSUPPORT_CPU_EN7521
	QDMA_EponQueueThreshold_T EponQThr;
#else
	eponQueueThreshold_t EponQThr;
#endif
	int ret = 0;

	for(queueIndex = 0; queueIndex< 8;queueIndex++){
		memset(&EponQThr, 0, sizeof(QDMA_EponQueueThreshold_T));
		if(mask&(1<<queueIndex)){//set queue threshold
			EponQThr.channel = llidIndex;
			EponQThr.queue = queueIndex ;
			EponQThr.thrIdx = setIndex;
#ifndef TCSUPPORT_CPU_EN7521
			ret = qdma_get_epon_queue_threshold(&EponQThr); 
#else
			ret = epon_get_queue_threshold(&EponQThr);
#endif
			if(ret < 0){
				eponDbgPrint(EPON_DEBUG_LEVEL_ERR, "\r\neponGetLlidQueThrod error");	
				return -1;
			}
			
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



int eponGetDataTxCounter(eponTxCnt_t *txCnt ){
	eponDbgPrint(EPON_DEBUG_LEVEL_TRACE, "eponGetDataTxCounter");	
	
	if(txCnt == NULL){
		eponDbgPrint(EPON_DEBUG_LEVEL_ERR, "eponGetDataTxCounter txCnt == NULL");	
		return -1;
	}	
#ifdef TCSUPPORT_CPU_EN7521
	FE_TxCnt_t Fe_txCnt;
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
#else
	txCnt->txFrameCnt = fe_reg_read(GDMA2_TX_ETHCNT - FE_BASE);
	txCnt->txFrameLen = fe_reg_read(GDMA2_TX_ETHLENCNT - FE_BASE);
	txCnt->txDropCnt = fe_reg_read(GDMA2_TX_ETHDROPCNT - FE_BASE);
	txCnt->txBroadcastCnt = fe_reg_read(GDMA2_TX_ETHBCDCNT - FE_BASE);
	txCnt->txMulticastCnt = fe_reg_read(GDMA2_TX_ETHMULTICASTCNT - FE_BASE);
	txCnt->txLess64Cnt = fe_reg_read(GDMA2_TX_ETH_LESS64_CNT - FE_BASE);
	txCnt->txMore1518Cnt = fe_reg_read(GDMA2_TX_ETH_MORE1518_CNT - FE_BASE);
	txCnt->txEq64Cnt = fe_reg_read(GDMA2_TX_ETH_64_CNT - FE_BASE);
	txCnt->txFrom65To127Cnt = fe_reg_read(GDMA2_TX_ETH_65_TO_127_CNT - FE_BASE);
	txCnt->txFrom128To255Cnt = fe_reg_read(GDMA2_TX_ETH_128_TO_255_CNT - FE_BASE);
	txCnt->txFrom256To511Cnt = fe_reg_read(GDMA2_TX_ETH_256_TO_511_CNT - FE_BASE);
	txCnt->txFrom512To1023Cnt = fe_reg_read(GDMA2_TX_ETH_512_TO_1023_CNT - FE_BASE);
	txCnt->txFrom1024To1518Cnt = fe_reg_read(GDMA2_TX_ETH_1024_TO_1518_CNT - FE_BASE);
#endif	
	return 0;
}



int eponGetDataRxCounter(eponRxCnt_t *rxCnt){
	eponDbgPrint(EPON_DEBUG_LEVEL_TRACE, "eponGetDataRxCounter");	
	if(rxCnt == NULL){
		eponDbgPrint(EPON_DEBUG_LEVEL_ERR, "\r\neponGetDataRxCounter rxCnt == NULL");	
		return -1;
	}	
#ifdef TCSUPPORT_CPU_EN7521
	FE_RxCnt_t Fe_rxCnt;
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
#else
	rxCnt->rxFrameCnt = fe_reg_read(GDMA2_RX_ETHERPCNT - FE_BASE);
	rxCnt->rxFrameLen = fe_reg_read(GDMA2_RX_ETHERPLEN - FE_BASE);
	rxCnt->rxDropCnt = fe_reg_read(GDMA2_RX_ETHDROPCNT - FE_BASE);
	rxCnt->rxBroadcastCnt = fe_reg_read(GDMA2_RX_ETHBCCNT - FE_BASE);
	rxCnt->rxMulticastCnt = fe_reg_read(GDMA2_RX_ETHMCCNT - FE_BASE);
	rxCnt->rxCrcCnt = fe_reg_read( GDMA2_RX_ETHCRCCNT - FE_BASE);
	rxCnt->rxFragFameCnt = fe_reg_read(GDMA2_RX_ETHFRACCNT - FE_BASE);
	rxCnt->rxJabberFameCnt = fe_reg_read(GDMA2_RX_ETHJABCNT - FE_BASE);
	rxCnt->rxLess64Cnt = fe_reg_read(GDMA2_RX_ETHRUNTCNT - FE_BASE);
	rxCnt->rxMore1518Cnt = fe_reg_read(GDMA2_RX_ETHLONGCNT - FE_BASE);
	rxCnt->rxEq64Cnt = fe_reg_read(GDMA2_RX_ETH_64_CNT - FE_BASE);
	rxCnt->rxFrom65To127Cnt = fe_reg_read(GDMA2_RX_ETH_65_TO_127_CNT - FE_BASE);
	rxCnt->rxFrom128To255Cnt = fe_reg_read(GDMA2_RX_ETH_128_TO_255_CNT - FE_BASE);
	rxCnt->rxFrom256To511Cnt = fe_reg_read(GDMA2_RX_ETH_256_TO_511_CNT - FE_BASE);
	rxCnt->rxFrom512To1023Cnt = fe_reg_read(GDMA2_RX_ETH_512_TO_1023_CNT - FE_BASE);
	rxCnt->rxFrom1024To1518Cnt = fe_reg_read(GDMA2_RX_ETH_1024_TO_1518_CNT - FE_BASE);
#endif
	return 0;
}


int eponClearGdm2MIB(__u8 rxCntClear, __u8 txCntClear){
#ifdef TCSUPPORT_CPU_EN7521
	if(txCntClear&0x1)
		FE_API_SET_CLEAR_MIB(FE_GDM_SEL_GDMA2, FE_GDM_SEL_TX);
	else if(rxCntClear&0x1)
		FE_API_SET_CLEAR_MIB(FE_GDM_SEL_GDMA2, FE_GDM_SEL_RX);
#else
	__u32 Raw = fe_reg_read(GDMA2_MIB_CLR - FE_BASE);
	rxCntClear &= 0x1;
	txCntClear &= 0x1;
	Raw |= ((rxCntClear<<1)|(txCntClear));
	fe_reg_write((GDMA2_MIB_CLR - FE_BASE) , Raw);
#endif
	return 0;
}

#include "pwan/xpon_netif.h"
void eponMacSetTxbufUsage(__u32 llidMask)
{
	extern PWAN_GlbPriv_T *gpWanPriv;

	__u8 llidIndex = 0;
	__u8 llidCnt = 0;
	__u32 value = 0;
	for( llidIndex = 0; llidIndex< EPON_LLID_MAX_NUM; llidIndex++ ){
		if(((1<<llidIndex)&llidMask) != 0)
			llidCnt++;
	}

	gpWanPriv->activeChannelNum = llidCnt;
	if(llidCnt <= 1){
		if (gpWanPriv->devCfg.flags.isQosUp == 1)
			WRITE_REG_WORD(0xBFB51890, 0x800020C0);
		else
			WRITE_REG_WORD(0xbfb51890,0);
	}
	else{
		value = 0x80/llidCnt;
		if (gpWanPriv->devCfg.flags.isQosUp == 1 && value > 0x20)
			value = 0x20;
		value <<= 8;
		value |= 0x80000080;
		WRITE_REG_WORD(0xbfb51890,value);
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

	

	for( llidIndex = 0; llidIndex< EPON_LLID_MAX_NUM; llidIndex++ ){
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
#if defined(TCSUPPORT_CT_PON_CN_CN) 	
	if (value == PHY_DISABLE)
	{
		ledTurnOff(LED_XPON_LOS_ON_STATUS);
		ledTurnOn(LED_XPON_LOS_STATUS);
	}
	else if(value == PHY_ENABLE)
	{
		if ((XPON_PHY_GET(PON_GET_PHY_LOS_STATUS) == PHY_NO_LOS_HAPPEN) /* no los and phy ready stats led should be configed */
			&& (XPON_PHY_GET(PON_GET_PHY_READY_STATUS) == PHY_TRUE)
			&& (TRUE != XPON_PHY_GET(PON_GET_PHY_GET_TX_POWER_EN_FLAG) )) /* enable should not config twice */
		{
			ledTurnOff(LED_XPON_LOS_ON_STATUS);
			ledTurnOff(LED_XPON_LOS_STATUS);
#if !defined(TCSUPPORT_C9_ROST_LED)
			ledTurnOn(LED_XPON_TRYING_STATUS);
#endif
			}
		}
#endif
	return;
}

struct timer_list eponTxPwrDwnTmr = {0};
void eponPhyTxPwrUp(unsigned long lparam){
	eponDbgPrint(1, "eponPhyTxPwrUp timer done, restart phy power\n");
	PhyTxLedConf(PHY_ENABLE);
	XPON_PHY_TX_POWER_CONFIG(PHY_ENABLE);
	if (XPON_PHY_TX_ENABLE()==PHY_FAILURE){
		eponDbgPrint(1, "\r\n Epon set PHY Tx Power Enable failed!");
	}
}
int eponPhyTxPowerCtl(__u32 time){
		
	unsigned int write_data = 0 ;
	unsigned int read_data = 0 ;
	if (time == 0) {// re-enabled
#ifdef TCSUPPORT_CT_PON // for PHY CONT Burst	
		if (PHY_FALSE == XPON_PHY_GET(PON_GET_PHY_TX_LONG_FLAG))
#endif
		{
			eponDbgPrint(1, "Epon open phy power!\n");
			
			PhyTxLedConf(PHY_ENABLE);
			XPON_PHY_TX_POWER_CONFIG(PHY_ENABLE);
			XPON_PHY_TX_ENABLE();
		}
	}else if (time == 65535) {// disabled tx power
		eponDbgPrint(1, "Epon close phy power forever!\n");

		/* clear timer when tx power disable  */
		if (timer_pending(&eponTxPwrDwnTmr)){
			del_timer(&eponTxPwrDwnTmr);
		}
		XPON_PHY_TX_DISABLE();
		XPON_PHY_TX_POWER_CONFIG(PHY_DISABLE);
		PhyTxLedConf(PHY_DISABLE);
	}else if (time > 0 && time < 65535) {
		eponDbgPrint(1, "\r\n Epon close phy power %d sec!", time);
	    XPON_PHY_TX_DISABLE();
		XPON_PHY_TX_POWER_CONFIG(PHY_DISABLE);
		PhyTxLedConf(PHY_DISABLE);
		init_timer(&eponTxPwrDwnTmr);
		eponTxPwrDwnTmr.expires = jiffies + time*HZ;
		eponTxPwrDwnTmr.function = eponPhyTxPwrUp;
		eponTxPwrDwnTmr.data = 0;
		mod_timer(&eponTxPwrDwnTmr, jiffies + time*HZ);
	}
	return 0;
}


#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
long eponMacIoctl(struct file *file, unsigned int cmd,
	                unsigned long arg)
#else
int eponMacIoctl (struct inode *inode, struct file *filp,
                  unsigned int cmd, unsigned long arg)
#endif
{
	int result = 0; 
    //__u16 *opt=(__u32 *)arg;
	//__u8 *opt2 = (__u8*)arg;
	eponMacIoctl_t *opt3 = (eponMacIoctl_t *)arg;
	//QDMA_EponQueueThreshold_T EponQThr;

#ifdef TCSUPPORT_AUTOBENCH
	if(IO_GREG(0xbfb00070) != 1)
		return 0;
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
		case EPON_IOCTL_SET_LLID_FEC:
			eponDbgPrint(EPON_DEBUG_LEVEL_TRACE, "eponMacIoctl = EPON_IOCTL_SET_LLID_FEC");
			result = eponSetLlidFec(opt3->llidIndex , opt3->info[0]);
			break;
		case EPON_IOCTL_GET_LLID_FEC:
			eponDbgPrint(EPON_DEBUG_LEVEL_TRACE, "eponMacIoctl = EPON_IOCTL_GET_LLID_FEC");
			result = eponGetLlidFec(opt3->llidIndex, &(opt3->param2));
			break;
		case EPON_IOCTL_MAC_RST:
			//eponMacRestart();
			eponWanResetWithChannelRetire();
			eponDbgPrint(EPON_DEBUG_LEVEL_TRACE, "eponMacIoctl = EPON_IOCTL_MAC_RST");
			break;
		case EPON_IOCTL_SET_LLID_DBA_THRSHLD_NUM:
			eponDbgPrint(EPON_DEBUG_LEVEL_TRACE, "eponMacIoctl = EPON_IOCTL_SET_LLID_DBA_QUE_NUM");
			//result = eponSetLlidThrshldNum(opt3->llidIndex , opt3->param0);
#ifndef TCSUPPORT_CPU_EN7521 
			result = qdma_set_epon_report_mode(opt3->llidIndex, opt3->param0);
#else
			result = eponSetLlidThrshldNum(opt3->llidIndex , opt3->param0);
#endif
			break;
		case EPON_IOCTL_GET_LLID_DBA_THRSHLD_NUM:
			eponDbgPrint(EPON_DEBUG_LEVEL_TRACE, "eponMacIoctl = EPON_IOCTL_GET_LLID_DBA_QUE_NUM");
			//result = eponGetLlidThrshldNum(opt3->llidIndex , opt3->info);
#ifndef TCSUPPORT_CPU_EN7521 
			result = qdma_get_epon_report_mode(opt3->llidIndex) ;
			if(result >= 0){
				opt3->info[0] = (__u8)result;
			}
#else
			result = eponGetLlidThrshldNum(opt3->llidIndex , opt3->info);
#endif	
			break;
		case EPON_IOCTL_SET_LLID_DBA_THROD:
			eponDbgPrint(EPON_DEBUG_LEVEL_TRACE, "eponMacIoctl = EPON_IOCTL_SET_LLID_DBA_THROD");
			result = eponSetLlidQueThrod(opt3->llidIndex , opt3->param0 , opt3->info[0] , (__u16 *)&opt3->info[1]);
			break;
		case EPON_IOCTL_GET_LLID_DBA_THROD:
			eponDbgPrint(EPON_DEBUG_LEVEL_TRACE, "eponMacIoctl = EPON_IOCTL_GET_LLID_DBA_THROD");
			result = eponGetLlidQueThrod(opt3->llidIndex , opt3->param0 , opt3->info[0] , (__u16 *)&opt3->info[1]);
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
            g_silence_time = opt3->param0;
            break;
		case EPON_IOCTL_SET_DEBUG_LEVEL:
			eponDbgPrint(EPON_DEBUG_LEVEL_TRACE, "eponMacIoctl = EPON_IOCTL_SET_DEBUG_LEVEL");
			eponDebugLevel = opt3->param0;
			break;
#ifdef TCSUPPORT_CPU_EN7521
		case EPON_IOCTL_SET_PHY_BURST_EN:
			eponDbgPrint(EPON_DEBUG_LEVEL_TRACE, "eponMacIoctl = EPON_IOCTL_SET_PHY_BURST_EN");
			result = XPON_PHY_SET_EPON_TS_CONTINUE_MODE(opt3->param2);
			break;
#endif
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
			
		default:
			break;
	}
	return result;
}






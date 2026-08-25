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
#include "epon/epon_debug.h"
#include "epon/epon_act.h"
#include "epon/epon_dev.h"
#include <ecnt_hook/ecnt_hook_fe.h>
#include <ecnt_hook/ecnt_hook_qdma.h>
#include "../inc/common/xpon_led.h"

//#include "qdma_bmgr.h"
extern struct timer_list eponTxPwrDwnTmr;

int eponSetLlidRxFec(__u8 llidIndex, __u8 fecFlag){
    gp_epon_global_data->llid_entry[llidIndex].rx_fec_flag = fecFlag;
    return 0;
}

int eponGetLlidRxFec(__u8 llidIndex, __u8 *fecFlag){
	if(fecFlag == NULL)
	{
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"\r\n eponGetLlidRxFec param error");	
		return -1;
	}
	*fecFlag = gp_epon_global_data->llid_entry[llidIndex].rx_fec_flag;
	return 0;
}

int eponGetLlidKeyInuse(__u8 llidIndex, __u8 *keyIndex){
	if((llidIndex >= EPON_LLID_MAX_NUM)||(keyIndex == NULL))
	{
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"\r\n eponSetLlidKey param error");	
		return -1;
	}	
	*keyIndex = gp_epon_global_data->llid_entry[llidIndex].llid_key_index;

	return 0;
}

int eponSetPonMacAddr(unsigned char llidIndex, unsigned char *epon_mac){
	if(epon_mac == NULL)
	{
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"\r\n eponSetPonMacAddr param error");	
		return -1;
	}
	if(llidIndex == 0){
		memcpy(gp_epon_global_data->onu_mac_addr , epon_mac ,6);
	}
	gp_epon_global_data->llid_entry[llidIndex].llidIndex = llidIndex;
	memcpy(gp_epon_global_data->llid_entry[llidIndex].macAddr, epon_mac ,6);
	return 0;
}


/*______________________________________________________________________________
**	function name
**		eponSetLlidQueThrod
**	description:
**		set threshold value 
**	parameters:
**		llidIndex
**		setIndex
**		mask
**		threshold
**	global:
**		None
**	return:
**		0:success
**          -1:fail
**	call:
**		epon_set_queue_threshold
**	revision:
**		v1.0
**____________________________________________________________________________*/


int eponSetLlidQueThrod(__u8 llidIndex, __u8 setIndex, __u8 bitmap,__u16 *threshold ){
	__u8 queueIndex = 0;
	eponQueueThreshold_t EponQThr;
	int ret = 0;

	if((llidIndex >= EPON_LLID_MAX_NUM) || (threshold == NULL)){
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR, "\r\eponSetLlidQueThrod parameters error");
		return -1;
	}
	
	#if 0
	//ret = epon_set_llid_report_bitmap(llidIndex,bitmap);
	ret = UNION_IC_FUNCTION_HOOK(REGISTER_ACTION_EPON_SET_REPORT_BITMAP,const void * in,void * out);
	if(ret != 0){
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR, "\r\nepon_set_llid_report_bitmap return error");	
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
			//printk("Set queueIndex:%d threshold:%d\n",EponQThr.queue,EponQThr.value);
			if(ret < 0 ){
				DEBUG_MSG_EPON(MSG_LEVEL_ERROR, "\r\neponSetLlidQueThrod error");	
				return -1;
			}
		}
	}
	return 0;
}


/*______________________________________________________________________________
**	function name
**		eponGetLlidQueThrod
**	description:
**		get threshold value 
**	parameters:
**		llidIndex
**		setIndex
**		mask
**		threshold
**	global:
**		None
**	return:
**		0:success
**          -1:fail
**	call:
**		epon_get_queue_threshold
**	revision:
**		v1.0
**____________________________________________________________________________*/



int eponGetLlidQueThrod(__u8 llidIndex, __u8 setIndex, __u8 *bitmap,__u16 *threshold ){
	__u8 queueIndex = 0;
	eponQueueThreshold_t EponQThr;
	epon_llid_report_bitmap_t report_bitmap={0};
	int ret = 0;

	if((llidIndex >= EPON_LLID_MAX_NUM) || (bitmap == NULL) || (threshold == NULL)){
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR, "\r\eponGetLlidQueThrod parameters error");
		return -1;
	}

	report_bitmap.llidIndex = llidIndex;
	if(UNION_IC_FUNCTION_HOOK(REGISTER_ACTION_EPON_GET_REPORT_BITMAP, NULL, &report_bitmap) != EPON_SUCCESS)
	{
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR, "\r\epon_get_llid_report_bitmap return error");
		return -1;
	}
	else
	{
		*bitmap = report_bitmap.bitmap;
	}
	
	
	for(queueIndex = 0; queueIndex< 8;queueIndex++){
		memset(&EponQThr, 0, sizeof(QDMA_EponQueueThreshold_T));
		if((*bitmap)&(1<<queueIndex)){//get queue threshold
			EponQThr.channel = llidIndex;
			EponQThr.queue = queueIndex ;
			EponQThr.thrIdx = setIndex;

			ret = epon_get_queue_threshold(&EponQThr);
			if(ret < 0){
				DEBUG_MSG_EPON(MSG_LEVEL_ERROR, "\r\neponGetLlidQueThrod error");	
				return -1;
			}else{
				threshold[queueIndex] = EponQThr.value;
				//printk("Get queueIndex:%d threshold:%d\n",EponQThr.queue,EponQThr.value);
			}
			
		}else{
			threshold[queueIndex] = 0;
		}
	}
	
	return 0;
}



/*______________________________________________________________________________
**	function name
**		eponSetMpcpTime
**	description:
**		set mpcp timeout period 
**	parameters:
**		mpcpTime
**	global:
**		None
**	return:
**		0:success
**          -1:fail
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/

int eponSetMpcpTime(__u16 mpcpTime ){
	__u32 Raw = 0;
	__u32 Raw_value = 0;
	Raw = mpcpTime/2;
	if(Raw > 0x3ff){
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR, "\r\neponSetMpcpTime time is too big");	
		return -1;
	}else if (Raw < 7){
		/* Under some OLT, 13ms maybe too small, so the min is 14ms(Raw=7, unit is 2ms) */
		Raw = 7;
	}else{
		/*normal mpcp time*/
	}
	
	Raw &= 0x3ff;
	Raw_value = READ_REG_WORD(e_mpcp_timeout_intvl);
	Raw_value &= ~(0x3ff);
	Raw_value |= Raw;
	DEBUG_MSG_EPON(MSG_LEVEL_TRACE, "eponSetMpcpTime time = %d", mpcpTime);	
	WRITE_REG_WORD(e_mpcp_timeout_intvl , Raw_value);
	return 0;	
}



/*______________________________________________________________________________
**	function name
**		eponGetMpcpTime
**	description:
**		get mpcp timeout period 
**	parameters:
**		mpcpTime
**	global:
**		None
**	return:
**		0:success
**          -1:fail
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/

int eponGetMpcpTime(__u16 *mpcpTime ){
	__u32 Raw = 0;
	Raw = READ_REG_WORD(e_mpcp_timeout_intvl);
	Raw &= 0x3ff;
	Raw <<=1;
	*mpcpTime = Raw;
	DEBUG_MSG_EPON(MSG_LEVEL_TRACE, "eponGetMpcpTime time = %d", *mpcpTime);	
	return 0;	
}



/*______________________________________________________________________________
**	function name
**		eponSetHoldoverCfg
**	description:
**		enable/disable holdovercfg and set time
**	parameters:
**		enable
**		time
**	global:
**		gp_epon_global_data
**	return:
**		0:success
**          -1:fail
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/

int eponSetHoldoverCfg(__u8 enable, __u16 time){
	DEBUG_MSG_EPON(MSG_LEVEL_TRACE, "eponSetHoldoverCfg enable = %d, time=%d", enable, time);
	gp_epon_global_data->hold_over_enable = enable;
	gp_epon_global_data->hold_over_time = time;
	return 0;
}


/*______________________________________________________________________________
**	function name
**		eponGetHoldoverCfg
**	description:
**		get holdovercfg  time
**	parameters:
**		enable
**		time
**	global:
**		gp_epon_global_data
**	return:
**		0:success
**          -1:fail
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/

int eponGetHoldoverCfg(__u8 *enable, __u16 *time){
	DEBUG_MSG_EPON(MSG_LEVEL_TRACE, "eponGetHoldoverCfg enable = %d, time=%d", 
        gp_epon_global_data->hold_over_enable, gp_epon_global_data->hold_over_time);   
	*enable = gp_epon_global_data->hold_over_enable;
	*time 	= gp_epon_global_data->hold_over_time ;
	return 0;
}

/*______________________________________________________________________________
**	function name
**		eponGetDataTxCounter
**	description:
**		get fe tx cnt
**	parameters:
**		txCnt
**	global:
**		None
**	return:
**		0:success
**          -1:fail
**	call:
**		FE_API_GET_ETH_TX_CNT
**	revision:
**		v1.0
**____________________________________________________________________________*/


int eponGetDataTxCounter(eponTxCnt_t *txCnt )
{
    FE_TxCnt_t Fe_txCnt;
    
	DEBUG_MSG_EPON(MSG_LEVEL_TRACE, "eponGetDataTxCounter");	
	
	if(txCnt == NULL){
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR, "eponGetDataTxCounter txCnt == NULL");	
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


/*______________________________________________________________________________
**	function name
**		eponGetDataRxCounter
**	description:
**		get fe rx cnt
**	parameters:
**		txCnt
**	global:
**		None
**	return:
**		0:success
**          -1:fail
**	call:
**		FE_API_GET_ETH_RX_CNT
**	revision:
**		v1.0
**____________________________________________________________________________*/

int eponGetDataRxCounter(eponRxCnt_t *rxCnt)
{
    FE_RxCnt_t Fe_rxCnt;
    
	DEBUG_MSG_EPON(MSG_LEVEL_TRACE, "eponGetDataRxCounter");	
	if(rxCnt == NULL){
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR, "\r\neponGetDataRxCounter rxCnt == NULL");	
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

/*______________________________________________________________________________
**	function name
**		eponClearGdm2MIB
**	description:
**		clear gdma2 mib counter
**	parameters:
**		rxCntClear
**		txCntClear
**	global:
**		None
**	return:
**		0:success
**          -1:fail
**	call:
**		FE_API_SET_CLEAR_MIB
**	revision:
**		v1.0
**____________________________________________________________________________*/

int eponClearGdm2MIB(__u8 rxCntClear, __u8 txCntClear){

	if(txCntClear&0x1)
		FE_API_SET_CLEAR_MIB(FE_GDM_SEL_GDMA2, FE_GDM_SEL_TX);
	else if(rxCntClear&0x1)
		FE_API_SET_CLEAR_MIB(FE_GDM_SEL_GDMA2, FE_GDM_SEL_RX);
    else
        DEBUG_MSG_EPON(MSG_LEVEL_ERROR, "\r\neponClearGdm2MIB clear fail");
	return 0;
}

#include "pwan/xpon_netif.h"
/*______________________________________________________________________________
**	function name
**		eponMacSetTxbufUsage
**	description:
**		set qdma tx buffer usage
**	parameters:
**		llidMask
**	global:
**		gpWanPriv
**	return:
**		None
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/

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
    return;
}
/*______________________________________________________________________________
**	function name
**		eponMacSetLlidEnableMask
**	description:
**		set mac llid enable
**	parameters:
**		llidMask
**	global:
**		gp_epon_global_data
**	return:
**		0:success
**          -1:fail
**	call:
**		eponMacSetTxbufUsage
**	revision:
**		v1.0
**____________________________________________________________________________*/

int eponMacSetLlidEnableMask(__u32 llidMask){
	
	DEBUG_MSG_EPON(MSG_LEVEL_ERROR , "\r\nenter eponMacSetLlidEnableMask = %x", llidMask);	
	
	UNION_IC_FUNCTION_HOOK(PACKETS_ACTION_EPON_TX_BUFF_USAGE_CFG,&llidMask,NULL);
	if(gp_epon_global_data->llid_bit_mask == llidMask){
		DEBUG_MSG_EPON(MSG_LEVEL_TRACE, "LLID mask is the same");
		return 0;
	}

	gp_epon_global_data->llid_bit_mask = llidMask;
	return 0;
}

/*______________________________________________________________________________
**	function name
**		PhyTxLedConf
**	description:
**		set phy tx led conf
**	parameters:
**		value
**	global:
**		gp_epon_global_data
**	return:
**		None
**	call:
**		ledTurnOff
**          ledTurnOn
**	revision:
**		v1.0
**____________________________________________________________________________*/

void PhyTxLedConf(int value)
{
#if defined(TCSUPPORT_CT_PON_CN_CN) 
	if (value == PHY_DISABLE)
	{
		change_alarm_led_status(LED_FLICKER);
	}
	else if(value == PHY_ENABLE)
	{
		if ((XPON_PHY_GET(PON_GET_PHY_LOS_STATUS) == PHY_NO_LOS_HAPPEN) /* no los and phy ready stats led should be configed */
			&& (XPON_PHY_GET(PON_GET_PHY_READY_STATUS) == PHY_TRUE)
			&& (TRUE != XPON_PHY_GET(PON_GET_PHY_GET_TX_POWER_EN_FLAG) )) /* enable should not config twice */
		{
			change_alarm_led_status(LED_OFF);
#if !defined(TCSUPPORT_C9_ROST_LED)
			change_pon_led_status(LED_FLICKER);
#endif
		}
	}
#else
	if(value == PHY_DISABLE)
	{
		change_pon_led_status(LED_OFF);
		change_alarm_led_status(LED_ON);
	}
	else
	{
		if ((XPON_PHY_GET(PON_GET_PHY_LOS_STATUS) == PHY_NO_LOS_HAPPEN) /* no los and phy ready stats led should be configed */
			&& (XPON_PHY_GET(PON_GET_PHY_READY_STATUS) == PHY_TRUE)) 
		{
			if(TRUE != XPON_PHY_GET(PON_GET_PHY_GET_TX_POWER_EN_FLAG) )/* enable should not config twice */
			{
				change_alarm_led_status(LED_OFF);
			}
		}
		else
		{
			change_pon_led_status(LED_OFF);
			change_alarm_led_status(LED_FLICKER);
		}
	}
#endif
	return;
}
/*______________________________________________________________________________
**	function name
**		eponPhyTxPwrUp
**	description:
**		set phy tx power up
**	parameters:
**		lparam
**	global:
**		None
**	return:
**		None
**	call:
**		PhyTxLedConf
**          XPON_PHY_TX_POWER_CONFIG
**          XPON_PHY_TX_ENABLE
**	revision:
**		v1.0
**____________________________________________________________________________*/


void eponPhyTxPwrUp(TIMER_FUN_PAAM lparam){
	DEBUG_MSG_EPON(MSG_LEVEL_DEBUG, "eponPhyTxPwrUp timer done, restart phy power\n");

    PhyTxLedConf(PHY_ENABLE);
	
    gp_epon_global_data->txPower_flag = TRUE;
	XPON_PHY_TX_POWER_CONFIG(PHY_ENABLE);
	if (timer_pending(&eponTxPwrDwnTmr)){
		del_timer(&eponTxPwrDwnTmr);
	}
	if (XPON_PHY_TX_ENABLE()==PHY_FAILURE){
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR, "\r\n Epon set PHY Tx Power Enable failed!");
	}
	start_oamCnt_ckeck_timer();
    return;
}
/*______________________________________________________________________________
**	function name
**		eponPhyTxPowerCtl
**	description:
**		 phy tx power control
**	parameters:
**		time
**	global:
**		eponTxPwrDwnTmr
**	return:
**		0:success
**          -1:fail
**	call:
**		PhyTxLedConf
**          XPON_PHY_TX_POWER_CONFIG
**          XPON_PHY_TX_ENABLE
**          XPON_PHY_TX_DISABLE
**	revision:
**		v1.0
**____________________________________________________________________________*/

int eponPhyTxPowerCtl(__u32 time)
{
		
	if (time == 0) {// re-enabled
#ifdef TCSUPPORT_CT_PON // for PHY CONT Burst	
		if (PHY_FALSE == XPON_PHY_GET(PON_GET_PHY_TX_LONG_FLAG))
#endif
		{
			DEBUG_MSG_EPON(MSG_LEVEL_ERROR, "Epon open phy power!\n");		
            PhyTxLedConf(PHY_ENABLE);
            gp_epon_global_data->txPower_flag = TRUE;
			XPON_PHY_TX_POWER_CONFIG(PHY_ENABLE);
			XPON_PHY_TX_ENABLE();
			start_oamCnt_ckeck_timer();
			if (timer_pending(&eponTxPwrDwnTmr)){
				del_timer(&eponTxPwrDwnTmr);
			}
		}
	}else if (time == 65535) {// disabled tx power
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR, "Epon close phy power forever time: %d!\n",time);

		/* clear timer when tx power disable  */
		if (timer_pending(&eponTxPwrDwnTmr)){
			del_timer(&eponTxPwrDwnTmr);
		}
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR, "shut down phy power_1  time: %d!\n",time);
		XPON_PHY_TX_DISABLE();
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR, "shut down phy power_2!\n");
		XPON_PHY_TX_POWER_CONFIG(PHY_DISABLE);
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR, "shut down phy power_3!\n");
		xmcs_report_event(XMCS_EVENT_TYPE_EPON, XMCS_EVENT_EPON_TRAFFIC_STATUS_CHANGE, TRAFFIC_DOWN) ;
        PhyTxLedConf(PHY_DISABLE);
		EPON_STOP_TIMER(gp_epon_global_data->oamCnt_timer);
        gp_epon_global_data->txPower_flag = FALSE;
	}else if (time > 0 && time < 65535) {
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR, "\r\n Epon close phy power %d sec!", time);
	    XPON_PHY_TX_DISABLE();
		XPON_PHY_TX_POWER_CONFIG(PHY_DISABLE);
		xmcs_report_event(XMCS_EVENT_TYPE_EPON, XMCS_EVENT_EPON_TRAFFIC_STATUS_CHANGE, TRAFFIC_DOWN) ;
        PhyTxLedConf(PHY_DISABLE);
		EPON_STOP_TIMER(gp_epon_global_data->oamCnt_timer);
        gp_epon_global_data->txPower_flag = FALSE;
		eponTxPwrDwnTmr.expires = jiffies + time*HZ;
		mod_timer(&eponTxPwrDwnTmr, jiffies + time*HZ);
	}else
	    DEBUG_MSG_EPON(MSG_LEVEL_ERROR, "Epon close phy power forever time: %d! it's wrong\n",time);
	return 0;
}

/*______________________________________________________________________________
**	function name
**		eponGetRateMode
**	description:
**		 get rate mdoe
**	parameters:
**		mode
**	global:
**		None
**	return:
**		0:success
**          -1:fail
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/

int eponGetRateMode(__u8 *mode)
{
	__u32 Raw = 0;
    __u8 mode_bit = 0;

	UNION_IC_FUNCTION_HOOK(REGISTER_ACTION_EPON_GET_WAN_CONF, NULL, &Raw);
    mode_bit = (Raw & (0xff));

    switch(mode_bit)
    {
    	case REG_MODE_1G_EPON:
            *mode = RATE_1G_1G;
            break;
        case REG_MODE_10G_ASYM:
            *mode = RATE_10G_1G_ASYM;
            break;
        case REG_MODE_10G_SYM:
            *mode = RATE_10G_10G_SYM;
            break;
        default:
            break;
    }
    DEBUG_MSG_EPON(MSG_LEVEL_NOTIFY, "%s Raw is %x mode is = 0x%x",__FUNCTION__,Raw, *mode);
    return 0;
}
/*______________________________________________________________________________
**	function name
**		eponSetRateMode
**	description:
**		 set rate mdoe
**	parameters:
**		mode
**	global:
**		None
**	return:
**		0:success
**          -1:fail
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/

int eponSetRateMode(__u8 mode)
{
	__u32 Raw = 0;
    switch(mode)
    {
    	case XMCS_IF_WAN_DETECT_MODE_EPON:
			Raw = REG_MODE_1G_EPON;
            break;
        case XMCS_IF_WAN_DETECT_MODE_10G_1G_EPON:
			Raw = REG_MODE_10G_ASYM;
            break;
        case XMCS_IF_WAN_DETECT_MODE_10G_10G_EPON:
			Raw = REG_MODE_10G_SYM;
            break;
        default:
            break;
    }
	
    //IO_SREG(SCU_WAN_CONF_REG,Raw);
    DEBUG_MSG_EPON(MSG_LEVEL_NOTIFY, "%s Raw is %x mode is = 0x%x\n",__FUNCTION__,Raw, mode);
    return 0;
}

/*______________________________________________________________________________
**	function name
**		epon control xpon led
**	description:
**		 control xpon led
**	parameters:
**		value
**	global:
**		None
**	return:
**		0:success
**          -1:fail
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/

int eponXponLedCtl(int value){
	switch(value){
		case XPON_LED_OFF_MODE:
			change_pon_led_status(LED_OFF);
			break;
		case XPON_LED_FLICKER_MODE:
			change_pon_led_status(LED_FLICKER);
			break;
		case XPON_LED_ON_MODE:
			change_pon_led_status(LED_ON);
			break;
		case XPON_LED_SW_UPGRADE_MODE:
			change_pon_led_status(LED_OFF);
			break;
			
		default:
			break;
	}
	return 0;
}


void hexToStr(char* dst, __u8* src, int len){
    int pos = 0;
    int i, j;
    char tmp[2];
    for(i=0; i<len; i++){
        tmp[0] = (src[i]&0xF0)>>4;
        tmp[1] = src[i]&0x0F;
        for(j=0; j<2; j++){
            if(tmp[j]<10){
                tmp[j] += '0';
            }else{
                if(tmp[j]<16){
                    tmp[j] = tmp[j]-10+'a';
                }
            }
            dst[pos++] = tmp[j];
        }
        dst[pos++] = ' ';
    }
    dst[pos-1] = '\0';
}

void MacSecInfoPrint(unsigned char* info){
    eponMacSec_t* keyInfo = (eponMacSec_t*)info;
    __u16 portId = keyInfo->sci[6]<<8 | keyInfo->sci[7];
    char tmp[48];

    DEBUG_MSG_EPON(0, "Desec Info:");
    DEBUG_MSG_EPON(0, "AN: %d", keyInfo->an);
    DEBUG_MSG_EPON(0, "PN: %d", keyInfo->pn);
    DEBUG_MSG_EPON(0, "Confid Offset: %d", keyInfo->confid_offset);
    hexToStr(tmp, keyInfo->sci, 6);
    DEBUG_MSG_EPON(0, "SCI Srcaddr: %s", tmp); 
    DEBUG_MSG_EPON(0, "Port ID: %d", portId);
    hexToStr(tmp, keyInfo->sak, 16);
    DEBUG_MSG_EPON(0, "SAK: %s", tmp);
}

/* IEEE Std 802.1X-2010 - Table 11-6 - Confidentiality Offset */
enum confidentiality_offset {
	CONFIDENTIALITY_NONE      = 0,
	CONFIDENTIALITY_OFFSET_0  = 1,
	CONFIDENTIALITY_OFFSET_30 = 2,
	CONFIDENTIALITY_OFFSET_50 = 3,
};

void MacSecOffsetMap(unsigned char* info){
    eponMacSec_t* keyInfo = (eponMacSec_t*)info;
    
    switch(keyInfo->confid_offset){
        case CONFIDENTIALITY_OFFSET_0:
            keyInfo->confid_offset = 0;
            break;
        case CONFIDENTIALITY_OFFSET_30:
            keyInfo->confid_offset = 30;
            break;
        case CONFIDENTIALITY_OFFSET_50:
            keyInfo->confid_offset = 50;
            break;
        default:
            break;
    }
}

int eponSetMacSecInfo(unsigned char* info){
    MacSecOffsetMap(info);
    MacSecInfoPrint(info);

    /*set register*/
//    __u32 regVal = 0;
//    regVal = READ_REG_WORD(e_glb_cfg);
//    WRITE_REG_WORD(e_glb_cfg, regVal);

    return 0;
}

/*______________________________________________________________________________
**	function name
**		eponMacIoctl
**	description:
**		 mac ioctl
**	parameters:
**		inode
**		filp
**		cmd
**		arg
**	global:
**		None
**	return:
**		0:success
**          -1:fail
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
long eponMacIoctl(struct file *file, unsigned int cmd,
	                unsigned long arg)
#else
int eponMacIoctl (struct inode *inode, struct file *filp,
                  unsigned int cmd, unsigned long arg)
#endif
{
	int result = 0; 
	eponMacIoctl_t temp = {0};
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
	if((IO_GREG(0xbfb00070) & 0x7 ) != 1)
		return 0;	
#endif
#endif

	switch(cmd)
	{
	    case EPON_IOCTL_SET_LLID_ENABLE_MASK:
			DEBUG_MSG_EPON(MSG_LEVEL_DEBUG, "eponMacIoctl = EPON_IOCTL_SET_LLID_ENABLE_MASK");
			result = eponMacSetLlidEnableMask(opt3->param2);
			break;
		case EPON_IOCTL_LLID_DEREGISTER:
			DEBUG_MSG_EPON(MSG_LEVEL_DEBUG, "eponMacIoctl = EPON_IOCTL_LLID_DEREGISTER");
			result = epon_mpcp_local_deregister(opt3->llidIndex);
			break;
		case EPON_IOCTL_LLID_DISCV_RGSTACK:			
			DEBUG_MSG_EPON(MSG_LEVEL_DEBUG, "eponMacIoctl = EPON_IOCTL_LLID_DISCV_RGSTACK");
			
			result = epon_mpcp_set_register_ack_flag(opt3->param0 , opt3->info[0]);
			break;
		case EPON_IOCTL_SET_LLID_KEY:
			DEBUG_MSG_EPON(MSG_LEVEL_DEBUG, "eponMacIoctl = EPON_IOCTL_SET_LLID_KEY");
			result = UNION_IC_FUNCTION_HOOK(REGISTER_ACTION_EPON_SET_LLID_KEY,opt3,NULL);
			break;
		case EPON_IOCTL_GET_LLID_KEY:
			DEBUG_MSG_EPON(MSG_LEVEL_DEBUG, "eponMacIoctl = EPON_IOCTL_GET_LLID_KEY");
			result = UNION_IC_FUNCTION_HOOK(REGISTER_ACTION_EPON_GET_LLID_KEY,NULL,opt3);
			result = copy_to_user((eponMacIoctl_t *)arg, opt3, sizeof(eponMacIoctl_t));
			break;
		case EPON_IOCTL_SET_LLID_TX_FEC:
			DEBUG_MSG_EPON(MSG_LEVEL_DEBUG, "eponMacIoctl = EPON_IOCTL_SET_LLID_TX_FEC");
			result = UNION_IC_FUNCTION_HOOK(REGISTER_ACTION_EPON_SET_LLID_TX_FEC,opt3,NULL);
			break;
		case EPON_IOCTL_GET_LLID_TX_FEC:
			DEBUG_MSG_EPON(MSG_LEVEL_DEBUG, "eponMacIoctl = EPON_IOCTL_GET_LLID_TX_FEC");
			result = UNION_IC_FUNCTION_HOOK(REGISTER_ACTION_EPON_GET_LLID_TX_FEC,NULL,opt3);
			result = copy_to_user((eponMacIoctl_t *)arg, opt3, sizeof(eponMacIoctl_t));
			break;
		case EPON_IOCTL_SET_LLID_RX_FEC:
			result = eponSetLlidRxFec(opt3->llidIndex , opt3->param0);
			break;
		case EPON_IOCTL_GET_LLID_RX_FEC:
			result = eponGetLlidRxFec(opt3->llidIndex, &(opt3->param0));
			result = copy_to_user((eponMacIoctl_t *)arg, opt3, sizeof(eponMacIoctl_t));
			break;
		case EPON_IOCTL_SET_LLID_MAC:
			result = eponSetPonMacAddr(opt3->llidIndex, opt3->info);
			break;
		case EPON_IOCTL_GET_LLID_KEY_INUSE:
			result = eponGetLlidKeyInuse(opt3->llidIndex, &(opt3->param0));
			result = copy_to_user((eponMacIoctl_t *)arg, opt3, sizeof(eponMacIoctl_t));
			break;
		case EPON_IOCTL_MAC_RST:
			//eponMacRestart();
			epon_reset(NULL);
			DEBUG_MSG_EPON(MSG_LEVEL_DEBUG, "eponMacIoctl = EPON_IOCTL_MAC_RST");
			break;
		case EPON_IOCTL_SET_LLID_DBA_THRSHLD_NUM:
			DEBUG_MSG_EPON(MSG_LEVEL_DEBUG, "eponMacIoctl = EPON_IOCTL_SET_LLID_DBA_QUE_NUM");
			result = UNION_IC_FUNCTION_HOOK(REGISTER_ACTION_EPON_SET_LLID_THRSHLD_NUM,opt3,NULL);
			break;
		case EPON_IOCTL_GET_LLID_DBA_THRSHLD_NUM:
			DEBUG_MSG_EPON(MSG_LEVEL_DEBUG, "eponMacIoctl = EPON_IOCTL_GET_LLID_DBA_QUE_NUM");
			result = UNION_IC_FUNCTION_HOOK(REGISTER_ACTION_EPON_GET_LLID_THRSHLD_NUM,NULL,opt3);
			result = copy_to_user((eponMacIoctl_t *)arg, opt3, sizeof(eponMacIoctl_t));
			break;
		case EPON_IOCTL_SET_LLID_DBA_THROD:
			DEBUG_MSG_EPON(MSG_LEVEL_DEBUG, "eponMacIoctl = EPON_IOCTL_SET_LLID_DBA_THROD");
			result = eponSetLlidQueThrod(opt3->llidIndex , opt3->param0 , opt3->info[0] , (__u16 *)&opt3->info[1]);
			break;
		case EPON_IOCTL_GET_LLID_DBA_THROD:
			DEBUG_MSG_EPON(MSG_LEVEL_DEBUG, "eponMacIoctl = EPON_IOCTL_GET_LLID_DBA_THROD");
			result = eponGetLlidQueThrod(opt3->llidIndex , opt3->param0 , &(opt3->info[0]) , (__u16 *)&opt3->info[1]);
			result = copy_to_user((eponMacIoctl_t *)arg, opt3, sizeof(eponMacIoctl_t));
			break;
		case EPON_IOCTL_SET_MPCP_TIME:
			DEBUG_MSG_EPON(MSG_LEVEL_DEBUG, "eponMacIoctl = EPON_IOCTL_SET_MPCP_TIME");
			result = eponSetMpcpTime(opt3->param1 );
			break;
		case EPON_IOCTL_GET_MPCP_TIME:
			DEBUG_MSG_EPON(MSG_LEVEL_DEBUG, "eponMacIoctl = EPON_IOCTL_GET_MPCP_TIME");
			result = eponGetMpcpTime(&(opt3->param1) );
			result = copy_to_user((eponMacIoctl_t *)arg, opt3, sizeof(eponMacIoctl_t));
			break;
		case EPON_IOCTL_SET_HOLDOVER_CFG:
			DEBUG_MSG_EPON(MSG_LEVEL_DEBUG, "eponMacIoctl = EPON_IOCTL_SET_HOLDOVER_CFG");
			result = eponSetHoldoverCfg(opt3->param0,  opt3->param1);
			break;
		case EPON_IOCTL_GET_HOLDOVER_CFG:
			DEBUG_MSG_EPON(MSG_LEVEL_DEBUG, "eponMacIoctl = EPON_IOCTL_GET_HOLDOVER_CFG");
			result = eponGetHoldoverCfg(&(opt3->param0), &(opt3->param1));
			result = copy_to_user((eponMacIoctl_t *)arg, opt3, sizeof(eponMacIoctl_t));
			break;
		case EPON_IOCTL_GET_TX_ETH_MIB:
			DEBUG_MSG_EPON(MSG_LEVEL_DEBUG, "eponMacIoctl = EPON_IOCTL_GET_TX_ETH_MIB");
			result = eponGetDataTxCounter((eponTxCnt_t *)(opt3->info) );
			result = copy_to_user((eponMacIoctl_t *)arg, opt3, sizeof(eponMacIoctl_t));
			break;
		case EPON_IOCTL_GET_RX_ETH_MIB:
			DEBUG_MSG_EPON(MSG_LEVEL_DEBUG, "eponMacIoctl = EPON_IOCTL_GET_RX_ETH_MIB");
			result = eponGetDataRxCounter((eponRxCnt_t *)(opt3->info) );
			result = copy_to_user((eponMacIoctl_t *)arg, opt3, sizeof(eponMacIoctl_t));
			break;		
		case EPON_IOCTL_CLR_TX_RX_MIB:
			DEBUG_MSG_EPON(MSG_LEVEL_DEBUG, "eponMacIoctl = EPON_IOCTL_CLR_TX_RX_MIB");
			result = eponClearGdm2MIB(opt3->info[0] , opt3->info[1] );
			break;	
		case EPON_IOCTL_POWER_CTL:
			DEBUG_MSG_EPON(MSG_LEVEL_DEBUG, "eponMacIoctl = EPON_IOCTL_POWER_CTL");
			result = eponPhyTxPowerCtl(opt3->param2);
			break;
		case EPON_IOCTL_AUTH_FAIL_CTL:
			DEBUG_MSG_EPON(MSG_LEVEL_DEBUG, "eponMacIoctl = EPON_IOCTL_AUTH_FAIL_CTL");
			gp_epon_global_data->oam_auth_fail = opt3->param0;
			break;
		case EPON_IOCTL_SET_SILENCETIME: //////////////////////////////
			DEBUG_MSG_EPON(MSG_LEVEL_DEBUG, "eponMacIoctl = EPON_IOCTL_SET_SILENCETIME");
			gp_epon_global_data->silent_time_config = opt3->param1;
			break;
		case EPON_IOCTL_SET_DEBUG_LEVEL:
			DEBUG_MSG_EPON(MSG_LEVEL_DEBUG, "eponMacIoctl = EPON_IOCTL_SET_DEBUG_LEVEL");
            epon_debug_level = opt3->param0;
			break;
		case EPON_IOCTL_SET_PHY_BURST_EN:
			DEBUG_MSG_EPON(MSG_LEVEL_DEBUG, "eponMacIoctl = EPON_IOCTL_SET_PHY_BURST_EN");
			result = XPON_PHY_SET_EPON_TS_CONTINUE_MODE(opt3->param2);
			break;
		case EPON_IOCTL_SET_DYING_GASP_MODE:
			DEBUG_MSG_EPON(MSG_LEVEL_DEBUG, "eponMacIoctl = EPON_IOCTL_SET_DYING_GASP_MODE");
			result = UNION_IC_FUNCTION_HOOK(REGISTER_ACTION_EPON_SET_DYGASP_HW_EN,&opt3->param0,NULL);
			break;
		case EPON_IOCTL_GET_DYING_GASP_MODE:
			DEBUG_MSG_EPON(MSG_LEVEL_DEBUG, "eponMacIoctl = EPON_IOCTL_GET_DYING_GASP_MODE");
			result = UNION_IC_FUNCTION_HOOK(REGISTER_ACTION_EPON_GET_DYGASP_HW_EN,NULL,&opt3->param0);
			result = copy_to_user((eponMacIoctl_t *)arg, opt3, sizeof(eponMacIoctl_t));
			break;
		case EPON_IOCTL_SET_DYING_GASP_NUM:
			DEBUG_MSG_EPON(MSG_LEVEL_DEBUG, "eponMacIoctl = EPON_IOCTL_SET_DYING_GASP_NUM");
			result = UNION_IC_FUNCTION_HOOK(REGISTER_ACTION_EPON_SET_DYGASP_NUM,&opt3->param2,NULL);
			break;
		case EPON_IOCTL_GET_DYING_GASP_NUM:
			DEBUG_MSG_EPON(MSG_LEVEL_DEBUG, "eponMacIoctl = EPON_IOCTL_GET_DYING_GASP_NUM");
			result = UNION_IC_FUNCTION_HOOK(REGISTER_ACTION_EPON_GET_DYGASP_NUM,NULL,&opt3->param2);
			result = copy_to_user((eponMacIoctl_t *)arg, opt3, sizeof(eponMacIoctl_t));
			break;
		case EPON_IOCTL_SET_10G_LLID_KEY:
			DEBUG_MSG_EPON(MSG_LEVEL_DEBUG, "eponMacIoctl = EPON_IOCTL_SET_10G_LLID_KEY\n");
			result = UNION_IC_FUNCTION_HOOK(REGISTER_ACTION_EPON_SET_10G_LLID_KEY,opt3,NULL);
			break;
		case EPON_IOCTL_GET_10G_LLID_KEY:
			DEBUG_MSG_EPON(MSG_LEVEL_DEBUG,"eponMacIoctl = EPON_IOCTL_GET_10G_LLID_KEY\n");
			result = UNION_IC_FUNCTION_HOOK(REGISTER_ACTION_EPON_GET_10G_LLID_KEY,NULL,opt3);
			result = copy_to_user((eponMacIoctl_t *)arg, opt3, sizeof(eponMacIoctl_t));
			break;		
        case EPON_IOCTL_GET_EPON_MODE:
            DEBUG_MSG_EPON(MSG_LEVEL_DEBUG, "eponMacIoctl = EPON_IOCTL_GET_EPON_MODE\n");
            result = eponGetRateMode(&(opt3->param0));
			result = copy_to_user((eponMacIoctl_t *)arg, opt3, sizeof(eponMacIoctl_t));
			break;	
        case EPON_IOCTL_SET_EPON_MODE:
            DEBUG_MSG_EPON(MSG_LEVEL_DEBUG, "eponMacIoctl = EPON_IOCTL_SET_EPON_MODE\n");
            result = eponSetRateMode(opt3->param0);
			break;
		case EPON_IOCTL_SET_DPOE_LLID_KEY:
			DEBUG_MSG_EPON(MSG_LEVEL_DEBUG, "eponMacIoctl = EPON_IOCTL_SET_DPOE_LLID_KEY\n");			
			result = UNION_IC_FUNCTION_HOOK(REGISTER_ACTION_EPON_SET_DPOE_LLID_KEY,opt3,NULL);
			break;
		case EPON_IOCTL_SET_MACSEC_INFO:
			DEBUG_MSG_EPON(MSG_LEVEL_DEBUG, "eponMacIoctl = EPON_IOCTL_MACSEC_INFO");
			result = eponSetMacSecInfo(opt3->info);
			break;	
		case EPON_IOCTL_XPON_LED_CTL:
			DEBUG_MSG_EPON(MSG_LEVEL_DEBUG, "eponMacIoctl = EPON_IOCTL_XPON_LED_CTL");
			result = eponXponLedCtl(opt3->param2);
			break;
			
		default:
			break;
	}
	return result;
}






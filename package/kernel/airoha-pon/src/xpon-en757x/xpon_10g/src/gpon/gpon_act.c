/***************************************************************
Copyright Statement:

This software/firmware and related documentation (¡°EcoNet Software¡±) 
are protected under relevant copyright laws. The information contained herein 
is confidential and proprietary to EcoNet (HK) Limited (¡°EcoNet¡±) and/or 
its licensors. Without the prior written permission of EcoNet and/or its licensors, 
any reproduction, modification, use or disclosure of EcoNet Software, and 
information contained herein, in whole or in part, shall be strictly prohibited.

EcoNet (HK) Limited  EcoNet. ALL RIGHTS RESERVED.

BY OPENING OR USING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY 
ACKNOWLEDGES AND AGREES THAT THE SOFTWARE/FIRMWARE AND ITS 
DOCUMENTATIONS (¡°ECONET SOFTWARE¡±) RECEIVED FROM ECONET 
AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON AN ¡°AS IS¡± 
BASIS ONLY. ECONET EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, 
WHETHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, 
OR NON-INFRINGEMENT. NOR DOES ECONET PROVIDE ANY WARRANTY 
WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTIES WHICH 
MAY BE USED BY, INCORPORATED IN, OR SUPPLIED WITH THE ECONET SOFTWARE. 
RECEIVER AGREES TO LOOK ONLY TO SUCH THIRD PARTIES FOR ANY AND ALL 
WARRANTY CLAIMS RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES 
THAT IT IS RECEIVER¡¯S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD 
PARTY ALL PROPER LICENSES CONTAINED IN ECONET SOFTWARE.

ECONET SHALL NOT BE RESPONSIBLE FOR ANY ECONET SOFTWARE RELEASES 
MADE TO RECEIVER¡¯S SPECIFICATION OR CONFORMING TO A PARTICULAR 
STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND 
ECONET'S ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE ECONET 
SOFTWARE RELEASED HEREUNDER SHALL BE, AT ECONET'S SOLE OPTION, TO 
REVISE OR REPLACE THE ECONET SOFTWARE AT ISSUE OR REFUND ANY SOFTWARE 
LICENSE FEES OR SERVICE CHARGES PAID BY RECEIVER TO ECONET FOR SUCH 
ECONET SOFTWARE.
***************************************************************/

/************************************************************************
*                  I N C L U D E S
*************************************************************************
*/
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include "common/xpon_global.h"
#include "common/xpon_api.h"
#include <common/xpon_led.h>
#include "common/phy_if_wrapper.h"
#if defined(TCSUPPORT_XPON_LED)
#include <asm/tc3162/ledcetrl.h>
#endif
#include "gpon/gpon_recovery.h"
#include <ecnt_hook/ecnt_hook_fe.h>

/************************************************************************
*                  D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/
struct tasklet_struct taskLedConfig;
uint hw_cnt_enable = 1;

/************************************************************************
*                  M A C R O S
*************************************************************************
*/
#define MAX_INT_VALUE 4294967295U
#define MIN_THRESHOLD_VALUE 19531250

/************************************************************************
*                  D A T A   T Y P E S
*************************************************************************
*/

/************************************************************************
*                  E X T E R N A L   D A T A   D E C L A R A T I O N S
*************************************************************************
*/

/************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
*************************************************************************
*/

/************************************************************************
*                  P U B L I C   D A T A
*************************************************************************
*/

/************************************************************************
*                  P R I V A T E   D A T A
*************************************************************************
*/
uint gponTCLODSEvent = 0;
uint gponTCRestoreLODSEvent = 0;
uint gponTCReactivLODSEvent = 0;

/************************************************************************
*                  F U N C T I O N   D E F I N I T I O N S
*************************************************************************
*/
/*______________________________________________________________________________
**  function name
**		gpon_act_to1_timer_expires
**  description:
**		change state when TO1 timeout
**  parameters:
**		unsigned long arg
**  global:
**		none.
**  return:
**		none
**  call:
**		gpon_act_change_state.   
**  revision:
**		v1.0
**____________________________________________________________________________*/
void gpon_act_to1_timer_expires(TIMER_FUN_PAAM arg)
{

	if(GPON_CURR_STATE == GPON_10G_STATE_O4 ) {
		PON_MSG((MSG_ERR|MSG_ACT), "TO1 timer timeout. Current State:O%d, Next State:O2_3\n", GPON_CURR_STATE) ;
		gpon_act_change_state(GPON_10G_STATE_O2_3);
		gpon_set_alarmBit(SUF_INDEX);
	}else{
        PON_MSG((MSG_ERR|MSG_ACT), "TO1 timer timeout in wrong State:O%d\n", GPON_CURR_STATE) ;
	}
}
/*______________________________________________________________________________
**  function name
**		gpon_act_to2_timer_expires
**  description:
**		handle related processing when TO2 timeout
**  parameters:
**		unsigned long arg
**  global:
**		 none.
**  return:
**		none
**  call:
**		gpon_disable.
**		xmcs_report_event
**  revision:
**		v1.0
**____________________________________________________________________________*/
void gpon_act_to2_timer_expires(TIMER_FUN_PAAM arg)
{
	if(GPON_10G_STATE_O6 != GPON_CURR_STATE){
		return ;
	}
	gpGponPriv->typeBOnGoing = FALSE;
	
	PON_MSG((MSG_ERR|MSG_ACT), "TO2 timer timeout. Current State:O%d, Next State:O1\n", GPON_CURR_STATE) ;

	stop_omci_oam_monitor();
		
	gpon_disable(GPON_MAC_PLAIN_RESET);    
}
/*______________________________________________________________________________
**  function name
**		gpon_act_to3_timer_expires
**  description:
**		handle related processing when TO3 timeout, for NGPON2, start in O6
**  parameters:
**		unsigned long arg
**  global:
**		 none.
**  return:
**		none
**  call:
**		gpon_disable.
**		xmcs_report_event
**  revision:
**		v1.0
**____________________________________________________________________________*/
void gpon_act_to3_timer_expires(TIMER_FUN_PAAM arg)
{
	if(GPON_CURR_STATE != GPON_10G_STATE_O6) {
		PON_MSG((MSG_OAM | MSG_ERR), "gpon_act_to3_timer_expires in non-O6 state, do nothing\n") ;
		return ;
	}

	gpGponPriv->typeBOnGoing = FALSE;
	PON_MSG((MSG_ERR|MSG_ACT), "TO3 timer timeout. Current State:O%d, Next State:O8\n", GPON_CURR_STATE) ;

	swap_work_standby_channel();

	gpon_act_change_state(GPON_10G_STATE_O8) ;


	PON_MSG((MSG_OAM | MSG_ACT), "gpon_act_to3_timer_expires set channel ds/us from %d %d to %d %d\n",gpGponPriv->gponCfg.ng2.workDsChannel,
		gpGponPriv->gponCfg.ng2.standbyUsChannel,gpGponPriv->gponCfg.ng2.standbyDsChannel,gpGponPriv->gponCfg.ng2.standbyUsChannel) ;
}

/*______________________________________________________________________________
**  function name
**		gpon_act_to4_timer_expires
**  description:
**		handle related processing when TO4 timeout, for NGPON2, start in O8
**  parameters:
**		unsigned long arg
**  global:
**		 none.
**  return:
**		none
**  call:
**		gpon_disable.
**		xmcs_report_event
**  revision:
**		v1.0
**____________________________________________________________________________*/
void gpon_act_to4_timer_expires(TIMER_FUN_PAAM arg)
{
	if(GPON_CURR_STATE != GPON_10G_STATE_O8) {
		PON_MSG((MSG_OAM | MSG_ERR), "gpon_act_to4_timer_expires in non-O8 state, do nothing\n") ;
		return ;
	}

	PON_MSG((MSG_ERR|MSG_ACT), "TO4 timer timeout. Current State:O%d, Next State:O1\n", GPON_CURR_STATE) ;

	//free channel setting ?
	if(gpGponPriv->gponCfg.ng2.tuningOnGoing && gpGponPriv->gponCfg.ng2.rollbackFlag){
		//rollback to source DS/US channel
		swap_work_standby_channel();

		gpGponPriv->gponCfg.ng2.tuningOnGoing = 0;

		//send ROLLBACK
		ploam_send_tuning_resp_msg(gpGponPriv->gponCfg.ng2.preSeqNo,TUNING_RSP_ROLLBACK,NG2_ROLLBACK_COM_DS);
		GPON_START_TIMER(gpGponPriv->to4_timer,gpGponPriv->gponCfg.to4Timer) ;
	}else{
		//real los
		gpon_disable(GPON_MAC_PLAIN_RESET);
	}
}

/*______________________________________________________________________________
**  function name
**		gpon_act_to5_timer_expires
**  description:
**		handle related processing when TO5 timeout, for NGPON2, start in O9
**  parameters:
**		unsigned long arg
**  global:
**		 none.
**  return:
**		none
**  call:
**		gpon_disable.
**		xmcs_report_event
**  revision:
**		v1.0
**____________________________________________________________________________*/
void gpon_act_to5_timer_expires(TIMER_FUN_PAAM arg)
{
	if(GPON_CURR_STATE != GPON_10G_STATE_O9) {
		PON_MSG((MSG_OAM | MSG_ERR), "gpon_act_to5_timer_expires in non-O9 state, do nothing\n") ;
		return ;
	}

	PON_MSG((MSG_ERR|MSG_ACT), "TO5 timer timeout. Current State:O%d, Next State:O1\n", GPON_CURR_STATE) ;

	//free channel setting ?
	if(gpGponPriv->gponCfg.ng2.tuningOnGoing && gpGponPriv->gponCfg.ng2.rollbackFlag && (ng2_no_rollback == 0)){
		gpon_act_change_state(GPON_10G_STATE_O8) ;

		//rollback to source DS/US channel
		swap_work_standby_channel();

		gpGponPriv->gponCfg.ng2.tuningOnGoing = 0;

		//send ROLLBACK
		ploam_send_tuning_resp_msg(gpGponPriv->gponCfg.ng2.preSeqNo,TUNING_RSP_ROLLBACK,NG2_ROLLBACK_US_TUNR);
	}
	else{
		gpon_disable(GPON_MAC_PLAIN_RESET);
	}
}

#if 0
/*______________________________________________________________________________
**  function name
**		gpon_act_to6_timer_expires
**  description:
**		handle related processing when TO6 timeout, for NGPON2, start in O5
**  parameters:
**		unsigned long arg
**  global:
**		 none.
**  return:
**		none
**  call:
**		gpon_disable.
**		xmcs_report_event
**  revision:
**		v1.0
**____________________________________________________________________________*/
void gpon_act_to6_timer_expires(TIMER_FUN_PAAM arg)
{
	if(GPON_CURR_STATE != GPON_10G_STATE_O5) {
		PON_MSG((MSG_OAM | MSG_ERR), "gpon_act_to6_timer_expires in non-O5 state, do nothing\n") ;
		return ;
	}

	PON_MSG((MSG_ERR|MSG_ACT), "TO6 timer timeout. Current State:O%d, Next State:O1\n", GPON_CURR_STATE) ;

	gpon_disable(GPON_MAC_PLAIN_RESET);
	xmcs_report_event(XMCS_EVENT_TYPE_GPON, XMCS_EVENT_GPON_LOS, 0) ;
	CALL_USER_HOOK_PHY_EVENT_NOTIFY(XMCS_EVENT_TYPE_GPON, XMCS_EVENT_GPON_LOS);
}
#endif
/*______________________________________________________________________________
**  function name
**		gpon_act_toZ_timer_expires
**  description:
**		handle related processing when TOZ timeout, for NGPON2, start in O2-3
**  parameters:
**		unsigned long arg
**  global:
**		 none.
**  return:
**		none
**  call:
**		gpon_disable.   
**		xmcs_report_event
**  revision:
**		v1.0
**____________________________________________________________________________*/

void gpon_act_toZ_timer_expires(TIMER_FUN_PAAM arg)
{
	if(GPON_CURR_STATE != GPON_10G_STATE_O2_3) {
		PON_MSG((MSG_OAM | MSG_ERR), "gpon_act_toZ_timer_expires in non-O9 state, do nothing\n") ;
		return ;
	}

	PON_MSG((MSG_ERR|MSG_ACT), "TOZ timer timeout. Current State:O%d, Next State:O1\n", GPON_CURR_STATE) ;

	//loop channel
	gpon_disable(GPON_MAC_PLAIN_RESET);
}

/*______________________________________________________________________________
**  function name
**		gpon_act_hardware_timer
**  description:
**		update hardware cnt
**  parameters:
**		unsigned long arg
**  global:
**		 none.
**  return:
**		none
**  call:
**		gpon_act_init.   
**  revision:
**		v1.0
**____________________________________________________________________________*/

void gpon_act_hardware_timer(TIMER_FUN_PAAM arg)
{
	PWAN_NetPriv_T *pNetPriv = NULL;
	FE_RxCnt_t rxCnt = {0};
	FE_TxCnt_t txCnt = {0};
	struct net_device *dev = NULL ;
	static unsigned long prev_rx_packets = 0;
	static unsigned long prev_tx_packets = 0;
	unsigned long curr_rx_packets = 0; 
	unsigned long curr_tx_packets = 0;
	unsigned long delta_rx_packets = 0;
	unsigned long delta_tx_packets = 0;
	static unsigned long prev_rx_bytes = 0;
	static unsigned long prev_tx_bytes = 0;
	unsigned long curr_rx_bytes = 0; 
	unsigned long curr_tx_bytes = 0;
	unsigned long delta_rx_bytes = 0;
	unsigned long delta_tx_bytes = 0;

	/* get current dev cnt */
	dev = gpWanPriv->pPonNetDev[(PWAN_IfType_t)PWAN_IF_DATA] ;
	pNetPriv = (PWAN_NetPriv_T *)netdev_priv(dev);

	/* get hw cnt */
	FE_API_GET_ETH_RX_CNT(&rxCnt);
	FE_API_GET_ETH_TX_CNT(&txCnt);

	curr_rx_packets = rxCnt.rxFrameCnt;
	curr_tx_packets = txCnt.txFrameCnt;
	curr_rx_bytes = rxCnt.rxFrameLen; 
	curr_tx_bytes = txCnt.txFrameLen;

	/* cal delta cnt */
	/* pkts */
	if (curr_rx_packets >= prev_rx_packets)
	{
		delta_rx_packets = curr_rx_packets - prev_rx_packets;
	}
	else
	{
		delta_rx_packets = MAX_INT_VALUE - prev_rx_packets + curr_rx_packets;
	}

	if (curr_tx_packets >= prev_tx_packets)
	{
		delta_tx_packets = curr_tx_packets - prev_tx_packets;
	}
	else
	{
		delta_tx_packets = MAX_INT_VALUE - prev_tx_packets + curr_tx_packets;
	}
	/* bytes */
	if (curr_rx_bytes >= prev_rx_bytes)
	{
		delta_rx_bytes = curr_rx_bytes - prev_rx_bytes;
	}
	else
	{
		delta_rx_bytes = MAX_INT_VALUE - prev_rx_bytes + curr_rx_bytes;
	}

	if (curr_tx_bytes >= prev_tx_bytes)
	{
		delta_tx_bytes = curr_tx_bytes - prev_tx_bytes;
	}
	else
	{
		delta_tx_bytes = MAX_INT_VALUE - prev_tx_bytes + curr_tx_bytes;
	}

	/* current dev cnt add delta cnt */
	/* packets */
	if( hw_cnt_enable )
	{
		pNetPriv->stats.rx_packets += delta_rx_packets;
		pNetPriv->stats.tx_packets += delta_tx_packets;
		pNetPriv->stats.rx_bytes += delta_rx_bytes;
		pNetPriv->stats.tx_bytes += delta_tx_bytes;		
	}

	prev_rx_packets = curr_rx_packets;
	prev_tx_packets = curr_tx_packets;
	prev_rx_bytes = curr_rx_bytes;
	prev_tx_bytes = curr_tx_bytes;

	if (GPON_CURR_STATE != GPON_10G_STATE_O5)
	{
		GPON_STOP_TIMER(gpGponPriv->hardware_timer) ;
	}
	else
	{
		GPON_START_TIMER(gpGponPriv->hardware_timer, gpGponPriv->gponCfg.hardware_timer);
	}
}


/*______________________________________________________________________________
**  function name
**		gpon_act_increase_LODS_event
**  description:
**		statistics xgpon TC loss of downstream synchronization event
**  parameters:
**		lastState: gpon state
		newState: gpon state
**  global:
**		none.
**  return:
**		none
**  call:
**		gpon_act_change_state.   
**  revision:
**		v1.0
**____________________________________________________________________________*/
int gpon_act_increase_LODS_event(int lastState, int newState)
{
	if((GPON_10G_STATE_O5 == lastState) && (GPON_10G_STATE_O6 == newState)){
		gponTCLODSEvent++;
	}else if((GPON_10G_STATE_O6 == lastState) && (GPON_10G_STATE_O5 == newState)){
		gponTCRestoreLODSEvent++;
	}else if((GPON_10G_STATE_O6 == lastState) && (GPON_10G_STATE_O1 == newState)){
		gponTCReactivLODSEvent++;
	}else{
		PON_MSG(MSG_TRACE, "LODS evnet will not increased.\n") ;
	}
	return 0;
}
/*______________________________________________________________________________
**  function name
**		gpon_act_change_state
**  description:
**		change the onu state and also start different timer according to new state
**  parameters:
**		new_state: gpon state
**  global:
**		gpGponPriv.
**  return:
**		none
**  call:
**		GPON_START_TIMER.   
**  revision:
**		v1.0
**____________________________________________________________________________*/
void gpon_act_change_state(const uint new_state)
{
	REG_ACTIVATION_ST xgponActState;	
	uint lastState = 0;
	ulong flags = 0;
	char same_state = 0;
	REG_CUR_KIDX gponCurKeyIdx;

	spin_lock_irqsave(&gpGponPriv->act_lock, flags) ;

	if (new_state == GPON_CURR_STATE){
	   PON_MSG(MSG_ACT, "PLOAM: State non-change: original = O%d  new = O%d\n",GPON_CURR_STATE, new_state) ;
		 same_state = 1;
	}else{
        PON_MSG(MSG_ACT, "PLOAM: State change: original = O%d  new = O%d\n",GPON_CURR_STATE, new_state) ;
	}

	if((GPON_10G_STATE_O1 == new_state)||(GPON_10G_STATE_O2_3  == new_state)) {
		if(TRAFFIC_UP == gpGponPriv->gpon_traffic_status){
			/* report GPON event message */
			xmcs_report_event(XMCS_EVENT_TYPE_GPON, XMCS_EVENT_GPON_TRAFFIC_STATUS_CHANGE, 0) ;
			gpGponPriv->gpon_traffic_status = TRAFFIC_DOWN;
		}
		gponRecovery.mibFlag = 0;
	}
	/* Set the current state to MAC register */
	xgponActState.Raw = IO_GREG(ACTIVATION_ST) ;
	lastState = xgponActState.Bits.act_st ;
	xgponActState.Bits.act_st = new_state ;
	IO_SREG(ACTIVATION_ST, xgponActState.Raw) ;
	
	GPON_CURR_STATE = new_state;

	 if(GPON_CURR_STATE == GPON_10G_STATE_O1){
	 	XPON_PHY_TX_DISABLE();     
		if(NGPON2_MODE){
			ng2_system_config_reset();
			XPON_PHY_TX_ENABLE(); //in case disabled by Disable_Discovery
		}
	}
	else if(GPON_CURR_STATE == GPON_10G_STATE_O2_3){
		XPON_PHY_TX_ENABLE();
	}
	else if(GPON_CURR_STATE == GPON_10G_STATE_O4){
		if(NGPON2_MODE){
			GPON_STOP_TIMER(gpGponPriv->toZ_timer) ;
		}
		PON_MSG(MSG_ACT, "Start the T1 timer. Current State:O%d\n",GPON_CURR_STATE) ;     
		GPON_START_TIMER(gpGponPriv->to1_timer,gpGponPriv->gponCfg.to1Timer) ;
	} else if(GPON_CURR_STATE == GPON_10G_STATE_O6){
#if !defined(TCSUPPORT_CPU_AN7583)		
		gponDevRefreshEqdValue();
#endif	
		PON_MSG(MSG_ACT, "Start the T2/T3 timer. Current State:O%d protectFlag=%d\n", GPON_CURR_STATE,gpGponPriv->gponCfg.ng2.protectFlag) ; 
		if(NGPON2_MODE && gpGponPriv->gponCfg.ng2.protectFlag == PROTECT_ENABLE){
			GPON_START_TIMER(gpGponPriv->to3_timer,gpGponPriv->gponCfg.to3Timer) ;
		}else{
		GPON_START_TIMER(gpGponPriv->to2_timer,gpGponPriv->gponCfg.to2Timer) ;
		}
		gpGponPriv->typeBOnGoing = TRUE;
		gponCurKeyIdx.Raw=IO_GREG(CUR_KIDX);
		gpGponPriv->gponCurKeyIdx = gponCurKeyIdx;
		gponDevMacReset(XPON_RESET_HOLD_ON);
	} else if(GPON_CURR_STATE == GPON_10G_STATE_O5) {
		printk("ponTime:O5\n");
		GPON_START_TIMER(gpGponPriv->hardware_timer,gpGponPriv->gponCfg.hardware_timer);
		gpon_clear_alarmBit(DACT_INDEX);
		if(NGPON2_MODE){
			forgotten_onu_monitor();
		}
		gpGponPriv->activationCnt++;
	}else if (NGPON2_MODE && GPON_CURR_STATE == GPON_10G_STATE_O8){
#if !defined(TCSUPPORT_CPU_AN7583)		
		gponDevRefreshEqdValue();
#endif	
		GPON_START_TIMER(gpGponPriv->to4_timer,gpGponPriv->gponCfg.to4Timer) ;
	}else if (NGPON2_MODE && GPON_CURR_STATE == GPON_10G_STATE_O9){
		if(ng2_o4_to_09 == 1){
			printk("ponTime:O9\n");
		}
		else {
			gponDevMpiTxStop(XPON_RESET_RELEASE);
			GPON_START_TIMER(gpGponPriv->to5_timer,gpGponPriv->gponCfg.to5Timer) ;
			//reset burst profile according to  pon-id; ?

			 if(gponDevGetTxSyncRdy() == GPON_TCONT_CMD_SUCCESS)
			 {
			 	ng2_o9_recv_grant_handler();
			 }
		}
	}else if (NGPON2_MODE && GPON_CURR_STATE == GPON_10G_STATE_O2_3){
		GPON_START_TIMER(gpGponPriv->toZ_timer,gpGponPriv->gponCfg.toZTimer) ;
	}else{
		PON_MSG(MSG_ACT, "No timer start in current State:O%d\n", GPON_CURR_STATE) ; 
	}
	if(!same_state){
		xmcs_report_event(XMCS_EVENT_TYPE_GPON, XMCS_EVENT_GPON_STATE_CHANGE, new_state) ;
	}
	tasklet_schedule(&taskLedConfig);

	gpon_act_increase_LODS_event(lastState,new_state);
	
	spin_unlock_irqrestore(&gpGponPriv->act_lock, flags) ;

	PON_MSG(MSG_ACT, "Change the current state: O%d. Last State:O%d\n", GPON_CURR_STATE, lastState) ; 
}
/*______________________________________________________________________________
**  function name
**		gpon_act_led_config
**  description:
**		gpon config led
**  parameters:
**		none
**  global:
**		gpGponPriv.
**  return:
**		none
**  call:
**		led config.   
**  revision:
**		v1.0
**____________________________________________________________________________*/
void gpon_act_led_config(unsigned long arg)
{
	if(isFPGA){
		return ;
	}

    if(GPON_CURR_STATE == GPON_10G_STATE_O1  || GPON_CURR_STATE == GPON_10G_STATE_O7){
		if(GPON_CURR_STATE == GPON_10G_STATE_O7){
    		change_alarm_led_status(LED_ON);
		}else{
			change_alarm_led_status(LED_FLICKER);
		}
		change_pon_led_status(LED_OFF);
	} else if (GPON_CURR_STATE == GPON_10G_STATE_O5) {
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_CMCC)
			change_pon_led_status(LED_ON);
#endif	
		change_alarm_led_status(LED_OFF);
	}else if(GPON_CURR_STATE == GPON_10G_STATE_O6){
		//do nothing ,for typeB
	}else{
		
		change_pon_led_status(LED_FLICKER);
		change_alarm_led_status(LED_OFF);
	}
}
/*______________________________________________________________________________
**  function name
**		gpon_act_init
**  description:
**		init TO1 and TO2 timer
**  parameters:
**		none
**  global:
**		gpGponPriv.
**  return:
**		none
**  call:
**		init_timer.   
**  revision:
**		v1.0
**____________________________________________________________________________*/
void gpon_act_init(void)
{
    tasklet_init(&taskLedConfig,gpon_act_led_config,0);
	gpGponPriv->state = GPON_10G_STATE_O1;
	
	GPON_CREATE_TIMER(&gpGponPriv->to1_timer,gpon_act_to1_timer_expires,gpGponPriv->gponCfg.to1Timer);/* 10,000 ms */
	GPON_CREATE_TIMER(&gpGponPriv->to2_timer,gpon_act_to2_timer_expires,gpGponPriv->gponCfg.to2Timer);/* 100 ms */
	GPON_CREATE_TIMER(&gpGponPriv->hardware_timer,gpon_act_hardware_timer, GPON_HARDWARE_TIMER);
#if defined(TCSUPPORT_CPU_AN7583)
	GPON_CREATE_TIMER(&gpGponPriv->fe_chn_done_timer,set_FE_CHN_RETIRE_DONE,gpGponPriv->gponCfg.feChnSetDoneTimer);/* 100 ms */
#endif
	if(NGPON2_MODE){
		GPON_CREATE_TIMER(&gpGponPriv->to3_timer,gpon_act_to3_timer_expires,gpGponPriv->gponCfg.to3Timer);/* 80 ms */
		GPON_CREATE_TIMER(&gpGponPriv->to4_timer,gpon_act_to4_timer_expires,gpGponPriv->gponCfg.to4Timer);/* 2000 ms */
		GPON_CREATE_TIMER(&gpGponPriv->to5_timer,gpon_act_to5_timer_expires,gpGponPriv->gponCfg.to5Timer);/* 5000 ms */
		//GPON_CREATE_TIMER(&gpGponPriv->to6_timer,gpon_act_to6_timer_expires,gpGponPriv->gponCfg.to6Timer);/* 10050 ms */
		GPON_CREATE_TIMER(&gpGponPriv->toZ_timer,gpon_act_toZ_timer_expires,gpGponPriv->gponCfg.toZTimer);/* 20000 ms */
	}
}
/*______________________________________________________________________________
**  function name
**		gpon_act_deinit
**  description:
**		deinit TO1 and TO2 timer
**  parameters:
**		none
**  global:
**		gpGponPriv.
**  return:
**		none
**  call:
**		del_timer.   
**  revision:
**		v1.0
**____________________________________________________________________________*/
void gpon_act_deinit(void)
{
	tasklet_kill(&taskLedConfig);
	del_timer(&gpGponPriv->to1_timer);
	del_timer(&gpGponPriv->to2_timer);
#if defined(TCSUPPORT_CPU_AN7583)
	del_timer(&gpGponPriv->fe_chn_done_timer);
#endif
	if(NGPON2_MODE){
		del_timer(&gpGponPriv->to3_timer);
		del_timer(&gpGponPriv->to4_timer);
		del_timer(&gpGponPriv->to5_timer);
		//del_timer(&gpGponPriv->to6_timer);
		del_timer(&gpGponPriv->toZ_timer);
	}
    GPON_STOP_TIMER(gpWanPriv->gpon.gemMibTimer) ;
}

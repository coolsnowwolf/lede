/*
* File Name: gpon_act.c
* Description: GPON ONU activation
*
******************************************************************/
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/timer.h>
#include <linux/jiffies.h>

#include <common/drv_global.h>
#include <common/phy_if_wrapper.h>
#include <common/xpon_led.h>
#include <gpon/gpon_qos.h>
#include <gpon/gpon.h>
#ifdef TCSUPPORT_OPENWRT
#include "gpon/gpon_hotplug.h"
#endif

#if defined(TCSUPPORT_XPON_LED)
#include <asm/tc3162/ledcetrl.h>
#endif

#include <ecnt_hook/ecnt_hook_fe.h>

#define MAX_INT_VALUE 4294967295U
#define MIN_THRESHOLD_VALUE 19531250

uint hw_cnt_enable = 1;

struct tasklet_struct taskLedConfig;
struct tasklet_struct gponRecvPloamMessageTask;

extern int is_hwnat_dont_clean;

#ifndef TCSUPPORT_CPU_EN7521
/* fe_reset_flag only be used in MT7520 for 
  * reseting PON PHY. 
  * It is used when traffic and then PHY link down.
  */
extern atomic_t fe_reset_flag;/*here_20150620*/
#endif /* TCSUPPORT_CPU_EN7521 */

#define TO1_TIMEOUT_MAX_CNT	20
atomic_t to1_timeout_cnt = ATOMIC_INIT(TO1_TIMEOUT_MAX_CNT);
atomic_t to1_rst_cnt = ATOMIC_INIT(0);

void gpon_set_cpu_load_version(ENUM_LIMIT_TYPE type)
{
#ifdef TCSUPPORT_CPU_EN7521
	QDMA_RxRateLimitSet_T rxRateLimitSetPtr;
	int ret = 0;
	unsigned long mask = 0;

	rxRateLimitSetPtr.ringIdx = 0;
	
	if(type == SET_LIMIT)
	{
		rxRateLimitSetPtr.rateLimitValue = 100;
		mask = 0x8;
	}
	else if(type == RECOVER_LIMIT)
	{
		rxRateLimitSetPtr.rateLimitValue = 6000;
		mask = 0x7;
	}
	
	
	
	if(gTypeOfAction == 1)// 1-do the lan and wan limit
	{
		ret = QDMA_API_SET_RX_RATELIMIT(ECNT_QDMA_WAN,&rxRateLimitSetPtr);
		if(ret != 0)
		{	
			printk("limit the wan speed error!\n");
		}
		ret = QDMA_API_SET_RX_RATELIMIT(ECNT_QDMA_LAN,&rxRateLimitSetPtr);
		if(ret != 0)
		{	
			printk("limit the lan speed error!\n");
		}
		
	}
	else if(gTypeOfAction == 2)//do the rebind cpu action
	{
#ifndef TCSUPPORT_CPU_ARMV8
		irq_set_affinity_ex(23, (struct cpumask *)&mask );//bind the qdma_wan to cpu3
		irq_set_affinity_ex(22, (struct cpumask *)&mask );//bind the qdma_lan to cpu3
#endif
	}
	else
	{
		return;
	}
#endif
	return;
}


/*****************************************************************************
******************************************************************************/
void gpon_act_to1_timer_expires(TIMER_FUN_PAAM arg)
{
	
#ifndef TCSUPPORT_CPU_EN7521
		if(atomic_read(&fe_reset_flag) ) {
			return;
		}
#endif	
	if(GPON_CURR_STATE==GPON_STATE_O3 || GPON_CURR_STATE==GPON_STATE_O4) {
		PON_MSG((MSG_ERR|MSG_ACT|MSG_TYPEB), "TO1 timer timeout. Current State:O%d, Next State:O2\n", GPON_CURR_STATE) ;

		if(atomic_dec_and_test(&to1_timeout_cnt)){
			PON_MSG((MSG_ERR|MSG_ACT),"%s TO1_TIMEOUT_MAX_CNT:%d trigger mac & phy reset.\r\n", __FUNCTION__ , TO1_TIMEOUT_MAX_CNT);
			xmcs_set_connection_start(XPON_ENABLE);
			/*Reset the to1_timeout_cnt*/
			atomic_set(&to1_timeout_cnt,TO1_TIMEOUT_MAX_CNT);
			atomic_inc(&to1_rst_cnt);
		}

		/* Change the current state */		
		gpon_act_change_gpon_state(GPON_STATE_O2) ;
		gpon_set_alarmBit(SUF_INDEX);
	}
}

/*****************************************************************************
******************************************************************************/
void gpon_act_to2_timer_expires(TIMER_FUN_PAAM arg)
{
	if(GPON_STATE_O6 != GPON_CURR_STATE){
        return ;
    }
	        
	PON_MSG((MSG_ERR|MSG_ACT|MSG_TYPEB), "TO2 timer timeout. Current State:O%d, Next State:O1\n", GPON_CURR_STATE) ;
	
	gpon_set_cpu_load_version(RECOVER_LIMIT);

	/* ´Ë¶ÎÂß¼­´¦ÀíTO2 timerout Ç°¾Íphy ready µÄÇé¿ö*/
    /* Èç¹û²»Ìí¼ÓÕâ¶Î´¦Àí½«µ¼ÖÂONUÍ£ÁôÔÚO1 */
    if (TRUE == XPON_PHY_GET(PON_GET_PHY_LOS_STATUS)){
#ifdef TCSUPPORT_OPENWRT
		fiber_hotplug_event(REMOVE_FIBER);
#endif
        gpPonSysData->sysLinkStatus = PON_LINK_STATUS_OFF;
        gpPhyData->phy_link_status = PHY_LINK_STATUS_LOS;
        gpPonSysData->sysPrevLink = PON_LINK_STATUS_GPON;
    }else{
        gpon_act_change_gpon_state(GPON_STATE_O1);
		gpPhyData->phy_link_status = PHY_LINK_STATUS_READY;
        if(gpPonSysData->sysStartup == PON_WAN_START)
            gpPonSysData->sysLinkStatus = PON_LINK_STATUS_GPON ;
    }
	
	if(likely(gpGponPriv->gponCfg.flags.hotplug == XPON_DISABLE)) {
    	is_hwnat_dont_clean = 0; /* For real unplug fiber */
	}
    xmcs_report_event(XMCS_EVENT_TYPE_GPON, XMCS_EVENT_GPON_LOS, 0) ;
    #if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_XPON_HAL_API_EXT)
		XPON_PHY_NOTIFY_EVENT(PHY_XPON_STATE_NOTIFY_LOS) ;
    #endif/*TCSUPPORT_COMPILE*/
	
    stop_omci_oam_monitor();

#ifdef TCSUPPORT_CPU_EN7521
	XPON_DPRINT_MSG("call gpon disable.\n");
	gpon_disable() ;
#else
	gpon_disable_with_option(GPON_DEV_RESET_WITH_FE_RESET);
#endif
}

/*****************************************************************************
******************************************************************************/

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

	if (GPON_CURR_STATE != GPON_STATE_O5)
	{
		GPON_STOP_TIMER(gpGponPriv->hardware_timer) ;
	}
	else
	{
		GPON_START_TIMER(gpGponPriv->hardware_timer, gpGponPriv->gponCfg.hardware_timer);
	}
}



/*****************************************************************************
******************************************************************************/

void gpon_act_change_gpon_state(const ENUM_GponState_t new_state)
{
    REG_G_ACTIVATION_ST gponActState ;
    PHY_GponPreb_T pon_preb;
    uint last_state;
    ulong flags = 0;
    char same_state = 0;

	PON_MSG(MSG_OAM, "gpon_act_change_gpon_state enter, gpon_silence=%d,cur_status=O%d, new_state=O%d\n",
		gpGponPriv->gpon_silence,GPON_CURR_STATE, new_state) ; 

	if((gpGponPriv->gpon_silence == 1) &&(GPON_CURR_STATE == GPON_STATE_O2)
		&& (new_state == GPON_STATE_O3)){
		return;
	}
    spin_lock_irqsave(&gpGponPriv->act_lock, flags) ;
    
    if ( new_state == GPON_CURR_STATE )
    {
        PON_MSG(MSG_OAM, "PLOAM: State non-change: original = O%d  new = O%d\n",GPON_CURR_STATE, new_state) ; 
        same_state = 1;
    }
	if((GPON_CURR_STATE == GPON_STATE_O4) && (new_state != GPON_STATE_O5))
	{
		if((new_state == GPON_STATE_O1)){
			PON_MSG(MSG_ERR, "PLOAM: ONU detected LOS/LOF in O4\n") ; 
		}
		else if((new_state == GPON_STATE_O7)){
			PON_MSG(MSG_ERR, "PLOAM: ONU received disable request in O4\n") ; 
		}
		else{
			PON_MSG(MSG_ERR, "PLOAM: ONU TO1 timer expired or received deactivation request\n") ; 
		}
		PON_MSG(MSG_ERR, "PLOAM: May be SUF (The Ranging of this ONU has failed) happended\n") ; 		
	}
    if((GPON_STATE_O1 == new_state)||(GPON_STATE_O2  == new_state)||(GPON_STATE_O3 == new_state)||(GPON_STATE_O7 == new_state)) {
		if(TRAFFIC_UP == gpGponPriv->gpon_traffic_status){
			/* report GPON event message */
			xmcs_report_event(XMCS_EVENT_TYPE_GPON, XMCS_EVENT_GPON_TRAFFIC_STATUS_CHANGE, 0) ;
			gpGponPriv->gpon_traffic_status = TRAFFIC_DOWN;
		}
		gpGponPriv->pGponRecovery->mibFlag = 0;
	}
    if(new_state == GPON_STATE_O5) {
        printk("ponTime:O5\n");
		GPON_START_TIMER(gpGponPriv->hardware_timer,gpGponPriv->gponCfg.hardware_timer);
        gpon_clear_alarmBit(DACT_INDEX);
        pon_preb.mask = PHY_OPER_RANG_EN;
        pon_preb.oper_ranged_st = 0x3;
        XPON_PHY_SET_API(PON_SET_PHY_GPON_EXTEND_PREAMBLE, &pon_preb);
#ifdef TCSUPPORT_CPU_EN7521
        /* Enable filter upstream overhead and extended burst length PLOAM */
        gponDevSetFilterExtBurstLengthPLOAM(XPON_ENABLE);
        gponDevSetFilterUpstreamOverheadPLOAM(XPON_ENABLE);

        /* Store first psync to sof delay */
        if(gpGponPriv->gponCfg.phy_psync_to_sof_delay == UNKNOW_PSYNC_TO_SOF_DELAY) {
            gpGponPriv->gponCfg.phy_psync_to_sof_delay = XPON_PHY_GET(PON_GET_PHY_ROUND_TRIP_DELAY);
        }
#endif
    } else if (new_state == GPON_STATE_O4 || new_state == GPON_STATE_O3) {
        pon_preb.mask = PHY_OPER_RANG_EN;
        pon_preb.oper_ranged_st = 0x2;
        XPON_PHY_SET_API(PON_SET_PHY_GPON_EXTEND_PREAMBLE, &pon_preb);
    } else if(new_state == GPON_STATE_O2) {
        pon_preb.mask = PHY_T3_O4_PRE_EN | PHY_T3_O5_PRE_EN | PHY_EXT_BUR_MODE_EN | PHY_OPER_RANG_EN;
        pon_preb.t3_O4_preamble = 0;
        pon_preb.t3_O5_preamble = 0;
        pon_preb.extend_burst_mode = 0;
        pon_preb.oper_ranged_st = 0;
        XPON_PHY_SET_API(PON_SET_PHY_GPON_EXTEND_PREAMBLE, &pon_preb);
        XPON_PHY_SET_BIT_DELAY(0);
    }       
    
    /* Set the current state to MAC register */
    gponActState.Raw = IO_GREG(G_ACTIVATION_ST) ;
    last_state = gponActState.Bits.act_st;
    gponActState.Bits.act_st = new_state ;
    IO_SREG(G_ACTIVATION_ST, gponActState.Raw) ;

    /* Stop the TO1 or TO2 timer */
//  if((GPON_CURR_STATE == GPON_STATE_O3) || (GPON_CURR_STATE == GPON_STATE_O4)) {
//      GPON_STOP_TIMER(gpGponPriv->to1_timer) ;
//  } else if(GPON_CURR_STATE == GPON_STATE_O6) {
//      GPON_STOP_TIMER(gpGponPriv->to2_timer) ;
//  }
    PON_MSG(MSG_OAM, "PLOAM: State change O%d --->  O%d\n", GPON_CURR_STATE, new_state) ; 

    GPON_CURR_STATE = new_state;
    
    if( GPON_STATE_O5 == new_state ){
        /*Reset the to1_timeout_cnt*/
        atomic_set(&to1_timeout_cnt,TO1_TIMEOUT_MAX_CNT);
        gpGponPriv->bip_cnt_val = 0;
        /* clear bip error counter */
        XPON_PHY_COUNTER_CLEAR(PHY_BIP_CNT_CLR);
		gpGponPriv->activationCnt++;
    }
    else if (new_state != GPON_STATE_O5) {
        /* Disable All upstream */
    }

    /* Start the TO1 or TO2 timer */
	if((GPON_CURR_STATE == GPON_STATE_O3) || (GPON_CURR_STATE == GPON_STATE_O4)) {
		PON_MSG(MSG_ACT, "Start the T1 timer. Current State:O%d\n", GPON_CURR_STATE) ;		
		GPON_START_TIMER(gpGponPriv->to1_timer,gpGponPriv->gponCfg.to1Timer) ;
	} else if(GPON_CURR_STATE == GPON_STATE_O6) {
		PON_MSG(MSG_ACT, "Start the T2 timer. Current State:O%d\n", GPON_CURR_STATE) ;		
        GPON_START_TIMER(gpGponPriv->to2_timer,gpGponPriv->gponCfg.to2Timer);
    }
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_XPON_HAL_API) || defined(TCSUPPORT_XPON_HAL_API_EXT) || defined(TCSUPPORT_FWC_ENV) || defined(TCSUPPORT_FH_PON)
    if(!same_state){
        xmcs_report_event(XMCS_EVENT_TYPE_GPON, XMCS_EVENT_GPON_STATE_CHANGE, new_state) ;
    }
#endif/*TCSUPPORT_COMPILE*/
    tasklet_schedule(&taskLedConfig);
    spin_unlock_irqrestore(&gpGponPriv->act_lock, flags) ;
	PON_MSG(MSG_ACT|MSG_TYPEB, "Change the current state: O%d. Last State:O%d\n", GPON_CURR_STATE, last_state) ;		
}

/*****************************************************************************
******************************************************************************/
uint gpon_act_get_gpon_state(void)
{
	return GPON_CURR_STATE;
}

void gpon_act_led_config(unsigned long arg)
{
	if(isFPGA || (GPON_CURR_STATE == GPON_STATE_O6))  return ;
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_C5_XPON_AUTH_LED)
	if(GPON_CURR_STATE == GPON_STATE_O1)
#else/*TCSUPPORT_COMPILE*/
    if(GPON_CURR_STATE == GPON_STATE_O1 || GPON_CURR_STATE == GPON_STATE_O7)
#endif/*TCSUPPORT_COMPILE*/
	{
        	if(GPON_CURR_STATE == GPON_STATE_O7)
        	{
#if defined(TCSUPPORT_CT_PON_CN_CN)
			change_alarm_led_status(ALARM_LED_FLICKER);
#else
            change_alarm_led_status(ALARM_LED_ON);
#endif
       		}else{
				change_alarm_led_status(ALARM_LED_FLICKER);
			}
		change_pon_led_status(ALARM_LED_OFF);
#if defined(TCSUPPORT_C9_ROST_LED)
		change_customer_led_status(ALARM_LED_ON);
#endif

#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_TW_BOARD_CDS)
		change_customer_led_status(ALARM_LED_ON);
#endif/*TCSUPPORT_COMPILE*/
	} else if (GPON_CURR_STATE == GPON_STATE_O5) {
#if defined(TCSUPPORT_C9_ROST_LED)
		change_customer_led_status(ALARM_LED_OFF);
		change_pon_led_status(ALARM_LED_FLICKER);

#else	
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_CMCC)
		change_pon_led_status(ALARM_LED_ON);
#endif/*TCSUPPORT_COMPILE*/	

#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_TW_BOARD_CDS)
		change_customer_led_status(ALARM_LED_OFF);
#endif/*TCSUPPORT_COMPILE*/		
		change_alarm_led_status(ALARM_LED_OFF);
#endif
	}else{

#if defined(TCSUPPORT_C9_ROST_LED)
		change_pon_led_status(ALARM_LED_OFF);
		change_customer_led_status(ALARM_LED_ON);
#else		
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_TW_BOARD_CDS)
		change_customer_led_status(ALARM_LED_ON);
		change_pon_led_status(ALARM_LED_OFF);
#else/*TCSUPPORT_COMPILE*/
        if(GPON_CURR_STATE == GPON_STATE_O2){
    		change_pon_led_status(ALARM_LED_FLICKER);
        }
#endif/*TCSUPPORT_COMPILE*/		
#endif
		change_alarm_led_status(ALARM_LED_OFF);
	}
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_C5_XPON_AUTH_LED)
	if(GPON_CURR_STATE == GPON_STATE_O2 || GPON_CURR_STATE == GPON_STATE_O7) {
		change_customer_led_status(ALARM_LED_OFF);	
	} else if (GPON_CURR_STATE == GPON_STATE_O3) {	
		change_customer_led_status(ALARM_LED_FLICKER);
	} else{
		change_customer_led_status(ALARM_LED_ON);	
	}
#endif/*TCSUPPORT_COMPILE*/
}

/*****************************************************************************
******************************************************************************/
int gpon_act_init(void)
{
    tasklet_init(&taskLedConfig,gpon_act_led_config,0);
	gpGponPriv->state = GPON_STATE_O1;
	
	GPON_CREATE_TIMER(&gpGponPriv->to1_timer,gpon_act_to1_timer_expires,gpGponPriv->gponCfg.to1Timer);/* 10,000 ms */
	GPON_CREATE_TIMER(&gpGponPriv->to2_timer,gpon_act_to2_timer_expires,gpGponPriv->gponCfg.to2Timer);/* 100 ms */
	GPON_CREATE_TIMER(&gpGponPriv->hardware_timer,gpon_act_hardware_timer, GPON_HARDWARE_TIMER);

	return 0 ;	
}

/*****************************************************************************
*function :
*		gpon_act_deinit
*description : 
*		This function is used to deinit gpon act.  
*input :
*		N/A
*output :
*		return 0 on success
******************************************************************************/
int gpon_act_deinit(void)
{
	/* kill led task */
	tasklet_kill(&taskLedConfig);
	return 0 ;
}


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

#if defined(TCSUPPORT_XPON_LED)
#include <asm/tc3162/ledcetrl.h>
extern int led_xpon_status;
#endif

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
/*****************************************************************************
******************************************************************************/
void gpon_act_to1_timer_expires(unsigned long arg)
{

#ifndef TCSUPPORT_CPU_EN7521
		if(atomic_read(&fe_reset_flag) ) {
			return;
		}
#endif	

	if(GPON_CURR_STATE==GPON_STATE_O3 || GPON_CURR_STATE==GPON_STATE_O4) {
		PON_MSG((MSG_ERR|MSG_ACT), "TO1 timer timeout. Current State:O%d, Next State:O2\n", G_ACTIVATION_ST->Bits.act_st) ;

		if(atomic_dec_and_test(&to1_timeout_cnt)){
			PON_MSG((MSG_ERR|MSG_ACT),"%s TO1_TIMEOUT_MAX_CNT:%d trigger mac & phy reset.\r\n", __FUNCTION__ , TO1_TIMEOUT_MAX_CNT);
			xmcs_set_connection_start(XPON_ENABLE);
			/*Reset the to1_timeout_cnt*/
			atomic_set(&to1_timeout_cnt,TO1_TIMEOUT_MAX_CNT);
			atomic_inc(&to1_rst_cnt);
		}

		/* Change the current state */		
		gpon_act_change_gpon_state(GPON_STATE_O2) ;
	}
}

/*****************************************************************************
******************************************************************************/
void gpon_act_to2_timer_expires(unsigned long arg)
{
	if(GPON_STATE_O6 != GPON_CURR_STATE){
        return ;
    }
        
	PON_MSG((MSG_ERR|MSG_ACT), "TO2 timer timeout. Current State:O%d, Next State:O1\n", G_ACTIVATION_ST->Bits.act_st) ;
	/*enable qdma green drop function */
	xpon_set_qdma_qos(XPON_ENABLE);

	gpPonSysData->sysLinkStatus = PON_LINK_STATUS_OFF ;

	gpPonSysData->sysPrevLink = PON_LINK_STATUS_GPON ;
	
	if(likely(gpGponPriv->gponCfg.flags.hotplug == XPON_DISABLE)) {
    	is_hwnat_dont_clean = 0; /* For real unplug fiber */
	}
    gpPhyData->phy_link_status = PHY_LINK_STATUS_LOS;
    xmcs_report_event(XMCS_EVENT_TYPE_GPON, XMCS_EVENT_GPON_LOS, 0) ;
	
    stop_omci_oam_monitor();

#ifdef TCSUPPORT_CPU_EN7521
	gpon_disable() ;
#else
	gpon_disable_with_option(GPON_DEV_RESET_WITH_FE_RESET);
#endif
}

/*****************************************************************************
******************************************************************************/
void gpon_act_change_gpon_state(const uint new_state)
{
    REG_G_ACTIVATION_ST gponActState ;
    PHY_GponPreb_T pon_preb;
    uint last_state;
    ulong flags = 0;
    char same_state = 0;
    spin_lock_irqsave(&gpGponPriv->act_lock, flags) ;
    
    if ( new_state == GPON_CURR_STATE )
    {
        PON_MSG(MSG_OAM, "PLOAM: State non-change: original = O%d  new = O%d\n",GPON_CURR_STATE, new_state) ; 
        same_state = 1;
    }
    
    if(new_state == GPON_STATE_O5) {
        pon_preb.mask = PHY_OPER_RANG_EN;
        pon_preb.oper_ranged_st = 0x3;
        XPON_PHY_SET_API(PON_SET_PHY_GPON_EXTEND_PREAMBLE, &pon_preb);
#ifdef TCSUPPORT_CPU_EN7521
        /* Enable filter upstream overhead and extended burst length PLOAM */
        gponDevSetFilterExtBurstLengthPLOAM(XPON_ENABLE);
        gponDevSetFilterUpstreamOverheadPLOAM(XPON_ENABLE);

        /* Store first psync to sof delay */
        if(gpGponPriv->gponCfg.phy_psync_to_sof_delay == UNKNOW_PSYNC_TO_SOF_DELAY) {
            gpGponPriv->gponCfg.phy_psync_to_sof_delay = IO_GMASK(0xbfaf02d4, 0xffff, 0);
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
        #if 0
        if (phy_rx_fec_status()==PHY_TRUE)
            IO_SREG(G_RSP_TIME,  gpGponPriv->gponCfg.onuResponseTime + 0x20);
        else
        #endif
        gponDevSetResponseTime(gpGponPriv->gponCfg.onuResponseTime);
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
    }
    else if (new_state != GPON_STATE_O5) {
        /* Disable All upstream */
        gpWanPriv->devCfg.flags.isTxDropOmcc = 1;
        xpon_set_qdma_qos(XPON_ENABLE);
    }

    /* Start the TO1 or TO2 timer */
	if((GPON_CURR_STATE == GPON_STATE_O3) || (GPON_CURR_STATE == GPON_STATE_O4)) {
		PON_MSG(MSG_ACT, "Start the T1 timer. Current State:O%d\n", gponActState.Bits.act_st) ;		
		GPON_START_TIMER(gpGponPriv->to1_timer) ;
	} else if(GPON_CURR_STATE == GPON_STATE_O6) {
		PON_MSG(MSG_ACT, "Start the T2 timer. Current State:O%d\n", gponActState.Bits.act_st) ;		
        GPON_START_TIMER(gpGponPriv->to2_timer) ;
    }
    tasklet_schedule(&taskLedConfig);
    spin_unlock_irqrestore(&gpGponPriv->act_lock, flags) ;
	PON_MSG(MSG_ACT, "Change the current state: O%d. Last State:O%d\n", gponActState.Bits.act_st, last_state) ;		
}

/*****************************************************************************
******************************************************************************/
uint gpon_act_get_gpon_state(void)
{
	return GPON_CURR_STATE;
}

void gpon_act_led_config(void)
{
	if(isFPGA)  return ;
#if defined(TCSUPPORT_XPON_LED)
#if !defined(TCSUPPORT_C5_XPON_AUTH_LED) 
    if(GPON_CURR_STATE == GPON_STATE_O1 || GPON_CURR_STATE == GPON_STATE_O6 || GPON_CURR_STATE == GPON_STATE_O7)
#endif
	{
        	if(GPON_CURR_STATE == GPON_STATE_O7)
        	{
#if defined(TCSUPPORT_CT_PON_CN_CN)
			change_alarm_led_status(ALARM_LED_FLICKER);
#else
            change_alarm_led_status(ALARM_LED_ON);
#endif
       		}
		ledTurnOff(LED_XPON_STATUS);		
		ledTurnOff(LED_XPON_TRYING_STATUS);
#if defined(TCSUPPORT_C9_ROST_LED)
		ledTurnOn(LED_XPON_UNREG_STATUS);
#endif

		led_xpon_status = 0;
	} else if (GPON_CURR_STATE == GPON_STATE_O5) {
#if defined(TCSUPPORT_C9_ROST_LED)
		ledTurnOff(LED_XPON_STATUS);
		ledTurnOff(LED_XPON_UNREG_STATUS);
		ledTurnOn(LED_XPON_TRYING_STATUS); 				
#else	
		ledTurnOff(LED_XPON_TRYING_STATUS);		
		ledTurnOn(LED_XPON_STATUS);
		change_alarm_led_status(ALARM_LED_OFF);
#endif
		led_xpon_status = 2;		
	}else{

#if defined(TCSUPPORT_C9_ROST_LED)
		ledTurnOff(LED_XPON_STATUS);
		ledTurnOff(LED_XPON_TRYING_STATUS);
		ledTurnOn(LED_XPON_UNREG_STATUS); 				
#else		
#if !defined(TCSUPPORT_TW_BOARD_CDS) 
		ledTurnOff(LED_XPON_STATUS);	
		ledTurnOn(LED_XPON_TRYING_STATUS);
#endif
#endif
		led_xpon_status = 1;
	}
#endif
}

/*****************************************************************************
******************************************************************************/
int gpon_act_init(void)
{
    tasklet_init(&taskLedConfig,gpon_act_led_config,0);
	gpGponPriv->state = GPON_STATE_O1;
	
	init_timer(&gpGponPriv->to1_timer) ;
	gpGponPriv->to1_timer.data = gpGponPriv->gponCfg.to1Timer ; /* 10,000 ms */
	gpGponPriv->to1_timer.function = gpon_act_to1_timer_expires ;
	
	init_timer(&gpGponPriv->to2_timer) ;
	gpGponPriv->to2_timer.data = gpGponPriv->gponCfg.to2Timer ; /* 100 ms */
	gpGponPriv->to2_timer.function = gpon_act_to2_timer_expires ;

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


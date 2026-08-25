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

/*
* File Name: gpon.c
* Description: 
*
******************************************************************/
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/types.h>

#include "gpon/gpon.h"
#include "common/drv_global.h"
#include "common/xpon_daemon.h"
#include "common/phy_if_wrapper.h"
#include "common/xpon_led.h"
#include "gpon/gpon_init.h"
#include "pwan/gpon_wan.h"
#include "gpon/gpon_dev.h"
#include "gpon/gpon_power_management.h"
#ifdef TCSUPPORT_OPENWRT
#include "gpon/gpon_hotplug.h"
#endif
#include <ecnt_hook/ecnt_hook_fe.h>

GPON_Emergence_Info gEmergenceRecord[GPON_EMERGENCE_STATE_MAX_NUM];
atomic_t gEmerStateindex = ATOMIC_INIT(0);
extern void  dbgtoMem(__u32 debugLevel, char *fmt,...);
#if defined(TCSUPPORT_RA_HWNAT) && defined(TCSUPPORT_RA_HWNAT_ENHANCE_HOOK)
extern int is_hwnat_dont_clean;
#endif
#ifndef TCSUPPORT_CPU_EN7523
extern void modify_mac_internal_delay(void);
#endif
GPON_SD_SF_Info_t gGpon_SD_SF_Info = {0};
int gpon_dev_reset_with_o7_flag = 0;
#define GPON_SD_SF_ALARM_BASE     (2488320000u)
REG_G_INT_STATUS intStatusDbg ;

static uint xmcs_get_SD_SF_BIP_NUM(unchar thld)
{
    unchar tmp = thld;
    uint result = GPON_SD_SF_ALARM_BASE;

    while (tmp != 0){
        result /= 10;
        tmp --;
    }
    return result;
}

/******************************************************************************
******************************************************************************/
void prepare_gpon(preXponMode_t mode)
{
	static Xpon_Phy_Mode_t first_mode = PHY_UNKNOWN_CONFIG;
	Xpon_Phy_Mode_t current_mode = XPON_PHY_GET(PON_GET_PHY_MODE);
#ifndef TCSUPPORT_CPU_EN7521
    if(TRUE == gpPonSysData->gswRateLimitFlag){
        XPON_STOP_TIMER(gpPonSysData->gsw_p6_rate_timer);
        xpon_disable_cpu_protection();
    }
#endif
	if(!(isEN7580 || isEN7581 || isAN7583)){
		XPON_PHY_FW_READY_DISABLE();
		if(gpPhyData->working_mode == PHY_GPON_SYM_CONFIG){
			select_xpon_wan(PHY_GPON_SYM_CONFIG);
		} else {
			select_xpon_wan(PHY_GPON_CONFIG);
		}
		if(!(isEN7523 || isAN7552)){	
			if(current_mode != PHY_GPON_CONFIG){
				if(XPON_AUTO_MODE == mode){
					XPON_PHY_SET(PON_SET_PHY_SCU_RST);
				}
				XPON_PHY_SET_MODE(PHY_GPON_CONFIG);
			}
		}else{
			if((PHY_UNKNOWN_CONFIG == first_mode) || ((current_mode != PHY_GPON_CONFIG) && (current_mode != PHY_GPON_SYM_CONFIG))){
				if(XPON_AUTO_MODE == mode){
					XPON_PHY_SET(PON_SET_PHY_SCU_RST);
				}
				if(gpPhyData->working_mode == PHY_GPON_SYM_CONFIG){
					XPON_PHY_SET_MODE(PHY_GPON_SYM_CONFIG);
				} else {
					XPON_PHY_SET_MODE(PHY_GPON_CONFIG);
				}
				if(PHY_UNKNOWN_CONFIG == first_mode){
					first_mode = XPON_PHY_GET(PON_GET_PHY_MODE);
				}
			}
		}		
	}

    gpon_power_management_init() ;
    
	XPON_DPRINT_MSG("call gpon disable.\n");
    gpon_disable() ;
	if(isEN7580 || isEN7581 || isAN7583){
		/* enable FW ready */
		XPON_PHY_FW_READY_ENABLE();
	}
}

void schedule_gpon_dev_reset(GPON_DEV_RESET_TYPE_t type)
{
    XPON_DAEMON_Job_data_t job = {0};

#ifndef TCSUPPORT_CPU_EN7521
	if( GPON_DEV_RESET_WITH_FE_RESET == type){
	    schedule_fe_reset();
	    return;
	}
#endif

	if(PHY_LINK_STATUS_READY == gpPhyData->phy_link_status){
	    if (0 == gpGponPriv->emergencystate)
	        gpon_act_change_gpon_state(GPON_STATE_O2);
	}else{
	        gpon_act_change_gpon_state(GPON_STATE_O1);
	}

    XPON_DPRINT_MSG("#######About to do GPON MAC and XPON PHY Reset!#######\n");

    
    job.id       = XPON_DAEMON_JOB_GPON_DEV_RESET ;
    job.priority = XPON_DAEMON_JOB_PRIORITY_HIGH  ;

    xpon_daemon_job_enqueue(&job)  ;    
    wake_up_xpon_daemon() ;
}

int gpon_disable_with_option(GPON_DEV_RESET_TYPE_t reset_type)
{
	REG_G_GBL_CFG gponGlbCfg ;

	int ret = 0 ;
	
	if(!(isEN7580 || isEN7523 || isEN7581 || isAN7583 || isAN7552)){
		unregister_gpon_isr() ;
	}
	
	ret = gpon_qos_deinit() ;
	if(ret != 0) {
		PON_MSG(MSG_ERR, "GPON QoS deinitialization failed.\n") ;
		if(GPON_DEV_RESET_WITH_O7_RESET != reset_type)
		{		
			return ret ;
		}
	}

	/* reset the onu_id and omcc id */
	gpGponPriv->gponCfg.onu_id = GPON_UNASSIGN_ONU_ID ;
	gpGponPriv->gponCfg.omcc = GPON_UNASSIGN_GEM_ID ;
	gpGponPriv->gponCfg.omccGemEnCry = 0;
	gpGponPriv->gponCfg.flags.isRequestKey = 0 ;
    gpGponPriv->typeBOnGoing = 0;

	/* Disable the MAC tx FEC */
	gponGlbCfg.Raw = IO_GREG(G_GBL_CFG) ;
	gponGlbCfg.Bits.us_fec_en = 0 ;
	IO_SREG(G_GBL_CFG, gponGlbCfg.Raw) ;

	gwan_remove_all_tcont() ;
	gwan_remove_all_gemport_for_disable() ;

	/* If ONU is in O7 and receive disable sn message, Do not schedule work queue, call dev reset
	directly, and then enable PHY TX */
	if(GPON_DEV_RESET_WITH_O7_RESET == reset_type)
	{
		PON_MSG(MSG_OAM,"do dev and phy reset\n");
		gpon_dev_reset_with_o7_flag = 1;
		handle_xpon_daemon_job_gpon_dev_reset();
	}
	else
	{
		schedule_gpon_dev_reset(reset_type);
	}

	return 0 ;
}

/*****************************************************************************
******************************************************************************/
int gpon_enable(void)
{
	int ret = 0 ;

	if((GPON_CURR_STATE!=GPON_STATE_O1) && 
        (GPON_CURR_STATE!=GPON_STATE_O2) &&
        (GPON_CURR_STATE!=GPON_STATE_O7)) {
		return -EFAULT ;
	}
	
	FE_API_SET_GDMA_MISC_CONFIG(FE_GDM_SEL_GDMA2,FE_MISC_CONFIG_GPON);
	
	gponDevResetCtrl(XPON_DISABLE);
	ret = gpon_dev_init() ;
	if(ret != 0) {
		PON_MSG(MSG_ERR, "GPON device initialization failed.\n") ;
		return ret ;
	}
	ret = gpon_qos_init() ;
	if(ret != 0) {
		PON_MSG(MSG_ERR, "GPON QoS initialization failed.\n") ;
		return ret ;
	}

	if(!(isEN7580 || isEN7523 || isEN7581 || isAN7583 || isAN7552)){
		/* register QDMA ISR callback function */
		ret = register_gpon_isr();
	}
	
	if(ret != 0) {
		PON_MSG(MSG_ERR, "Register GPON ISR handler function failed.\n") ;
		return ret ;
	}

	/* restore some configuration before gpon enable */
	gponDevSetSerialNumber(gpGponPriv->gponCfg.sn) ;
	gponDevSetEncryptKey(gpGponPriv->gponCfg.key) ;

#if defined(TCSUPPORT_CPU_EN7526) || defined(TCSUPPORT_CPU_EN7527)
    /* add for RDI send timer */
    RDI_TIMER_EN = 1;
    if(RDI_SEND_ENABLE){
        GPON_STOP_TIMER(rdi_timer);
        GPON_START_TIMER(rdi_timer,RDI_SEND_DELAY);
    }	
#endif
	return 0 ;
}

#ifdef TCSUPPORT_CPU_EN7521
static void adjust_mac_internal_delay_fine_tune(void)
{
	uint psync_to_sof_delay_curr;
	uint last_dly;
	int psync_to_sof_delay_diff;
	int internalDelayFineTune;
	int retry = 10;
	int i;
	int cnt = 0;

	/* Restore first psync to sof delay */
	if(gpGponPriv->gponCfg.phy_psync_to_sof_delay != UNKNOW_PSYNC_TO_SOF_DELAY) {
		PON_MSG(MSG_DBG, "Adjust EqD: first psync_to_sof_delay 0x%x\n", gpGponPriv->gponCfg.phy_psync_to_sof_delay) ;
		/* This check is used to make sure phy register RTD value ready */
		for(i = 0; i < retry; i++) {
			mdelay(1);
			psync_to_sof_delay_curr = XPON_PHY_GET(PON_GET_PHY_ROUND_TRIP_DELAY);
			if(i == 0) {
				last_dly = psync_to_sof_delay_curr;
			} else if(last_dly == psync_to_sof_delay_curr) {
				cnt++;
			} else {
				last_dly = psync_to_sof_delay_curr;
				cnt = 0;
		}
			if(cnt == 3) {
				break;
			}
		}
		/* psync_to_sof_delay_diff > 0:phy delay is more than first 
		 * psync_to_sof_delay_diff < 0:phy delay is less than first 
		 */
		psync_to_sof_delay_diff = psync_to_sof_delay_curr - gpGponPriv->gponCfg.phy_psync_to_sof_delay;
		PON_MSG(MSG_DBG, "Adjust EqD: psync_to_sof_delay_diff d'%d\n", psync_to_sof_delay_diff) ;

		if(abs(psync_to_sof_delay_diff) <= 15) {
			/* shift psync_to_sof_delay_diff * 4 bits.
			 * The GPON MAC resolution is byte,
			 * so internalDelayFineTune = first internalDelayFineTune + psync_to_sof_delay_diff * 4 / 8
			 *                  = first internalDelayFineTune + psync_to_sof_delay_diff / 2 (bytes).
			 */
			/* internalDelayFineTune unit is 1byte time.
			 * MAC tx delay is inversely proportional to internalDelayFineTune.
			 */
			internalDelayFineTune = gpGponPriv->gponCfg.internalDelayFineTune +
									(psync_to_sof_delay_diff / 2);
	
			/* Set internalDelayFineTune */
			gponDevSetInternalDelayFineTune(internalDelayFineTune);
			PON_MSG(MSG_DBG, "Adjust EqD: internal delay fine tune 0x%x\n", internalDelayFineTune) ;
        }
    }
}
#endif /* TCSUPPORT_CPU_EN7521 */

/*****************************************************************************
******************************************************************************/
void gpon_detect_phy_ready(void)
{  
#ifdef TCSUPPORT_CPU_EN7521
	/* addjust MAC internal delay for EqD */
	if(!(isFPGA || isEN751627 || isEN7580 || isEN7523 || isEN7581 || isAN7583 || isAN7552) ) {
		adjust_mac_internal_delay_fine_tune();
	}
#endif /* TCSUPPORT_CPU_EN7521 */

	PON_MSG(MSG_INT|MSG_TYPEB, "GPON IRQ: gpon detect phy ready.\n") ;

    /************phy_link_status must set after set MAC internal Delay*************/
    
	gpPhyData->phy_link_status = PHY_LINK_STATUS_READY ;

	/*********************************************************************/
	
#ifdef TCSUPPORT_OPENWRT
	fiber_hotplug_event(INSERT_FIBER);
#endif
#if 0
    if ( (TRUE == XPON_PHY_GET(PON_GET_PHY_GET_TX_POWER_EN_FLAG) ) && \
         (XPON_ROUGE_STATE_FALSE == gpPonSysData->rogue_state ) ) {
    	if(GPON_STATE_O7 != GPON_CURR_STATE) {
            change_alarm_led_status(ALARM_LED_OFF);
        } else {
#if defined(TCSUPPORT_CT_PON_CN_CN)
						change_alarm_led_status(ALARM_LED_FLICKER);
#else
						change_alarm_led_status(ALARM_LED_ON);
#endif
        }
    }
#endif    
	if(GPON_CURR_STATE == GPON_STATE_O1) {	
		/* Change the current state */
		if(gpGponPriv->emergencystate) {
		    gponDevResetCtrl(XPON_DISABLE);
            gpon_act_change_gpon_state(GPON_STATE_O7) ;
			gpon_record_emergence_info(GPON_PHY_READY_REPORT_O7);
		} else {
            gpon_act_change_gpon_state(GPON_STATE_O2) ;
			gpon_set_cpu_load_version(SET_LIMIT);
        }
	}
	
	if(gpPonSysData->sysStartup == PON_WAN_START) {
		gpPonSysData->sysLinkStatus = PON_LINK_STATUS_GPON ;
		gpon_enable() ;
	}
	
	/* report GPON event message */
	xmcs_report_event(XMCS_EVENT_TYPE_GPON, XMCS_EVENT_GPON_PHY_READY, 0) ;
}

#ifndef TCSUPPORT_CPU_EN7521
/*****************************************************************************
******************************************************************************/
void gpon_reset_dbg_delay(void)
{
    REG_DBG_DLY readVal  = {.Raw = IO_GREG(DBG_DLY)};
    REG_DBG_DLY writeVal = {.Raw = gpGponPriv->gponCfg.reg4208};
   
    if( gpGponPriv->gponCfg.rstDbgDly & MAC_4208_PHY_RX_FIX ){
        readVal.Bits.phy_rx_dly_sel = writeVal.Bits.phy_rx_dly_sel;
    }else if( gpGponPriv->gponCfg.rstDbgDly & MAC_4208_PHY_RX_DLY ){
        readVal.Bits.fix_phy_rx_dly = writeVal.Bits.fix_phy_rx_dly;
    }else if( gpGponPriv->gponCfg.rstDbgDly & MAC_4208_INTERNAL_DLY ){
        readVal.Bits.fine_int_dly = writeVal.Bits.fine_int_dly;
    }else if( gpGponPriv->gponCfg.rstDbgDly & MAC_4208_PHY_TX_DLY ){
        readVal.Bits.phy_tx_dly = writeVal.Bits.phy_tx_dly;
    }

    IO_SREG(DBG_DLY,  writeVal.Raw);
}
#endif /* TCSUPPORT_CPU_EN7521 */

#ifdef TCSUPPORT_CPU_EN7521
static void disable_sniffer_mode(void)
{
	/* store sniffer mode status */
	gponDevGetSniffMode(&gpGponPriv->gponCfg.sniffer_mode);
	/* close sniffer mode */
	gponDevEnableSniffMode(XPON_DISABLE);
}
#endif

extern void gpon_dev_typeB_restore(void);
extern uint gSnMsgValue;

void gpon_typeb_dev_reset(void)
{
    XPON_DPRINT_MSG("#######Doing GPON MAC and XPON PHY Reset!#######\n");

	if(gpGponPriv->emergencystate) {
		XPON_PHY_TX_DISABLE();
		gpon_act_change_gpon_state(GPON_STATE_O7) ;
		gpon_record_emergence_info(GPON_SET_CONNECTION_REPORT_O7);
		return;
	}
	/* The PON_SET_PHY_STOP will disable phy fw ready */
	if(!(isEN7580 || isEN7581 || isAN7583)){
    XPON_PHY_SET(PON_SET_PHY_STOP) ;
	}
    gponDevResetCtrl(XPON_ENABLE) ;
	/* The PON_SET_PHY_START will not enable phy fw ready */
    if(!(isEN7580 || isEN7581 || isAN7583)){
	XPON_PHY_SET(PON_SET_PHY_START);
    }
	/* enable FW ready */
	XPON_PHY_FW_READY_ENABLE();
    XPON_START_TIMER(gpPhyData->trans_status_refresh_timer,1000);

	gpon_dev_init();
	gpon_qos_init();
}


/*****************************************************************************
******************************************************************************/
int gwan_remove_all_gemport_totally(void)
{
	ushort gemPortId ;
	int i ;
	
	for(i=0 ; i<GPON_GEMPORT_MAX_NUM ; i++) {
		if(gpWanPriv->gpon.gemPort[i].info.valid) {
			gemPortId = gpWanPriv->gpon.gemPort[i].info.portId ;
			gpWanPriv->gpon.gemIdToIndex[gemPortId] = GPON_GEM_IDX_MASK ;
			
			memset(&gpWanPriv->gpon.gemPort[i].info, 0, sizeof(GWAN_GemInfo_T)) ;
			memset(&gpWanPriv->gpon.gemPort[i].stats, 0, sizeof(struct net_device_stats)) ;
			gponDevSetGemInfo(gemPortId, XPON_DISABLE, XPON_DISABLE) ;
		}
	}
	gpWanPriv->gpon.gemNumbers = 0 ;
	return 0 ;
}


/*****************************************************************************
******************************************************************************/
void gpon_detect_los_lof(PHY_Event_Type_t id)
{
	PON_MSG(MSG_INT|MSG_TYPEB, "GPON IRQ: gpon detect los lof.\n") ;
	PON_MSG(MSG_ERR, "one bit err cnt:0x%08x\n", IO_GREG(DBG_GEM_HEC_ONE_ERR_CNT)) ; 
	PON_MSG(MSG_ERR, "two bits err cnt:0x%08x\n", IO_GREG(DBG_GEM_HEC_TWO_ERR_CNT)) ; 
	PON_MSG(MSG_ERR, "uncorrectable err cnt:0x%08x\n", IO_GREG(DBG_GEM_HEC_UC_ERR_CNT)) ; 
  
    if((GPON_STATE_O2 == GPON_CURR_STATE) || 
	   (GPON_STATE_O3 == GPON_CURR_STATE) || 
	   (GPON_STATE_O4 == GPON_CURR_STATE)) {

		if(id == PHY_EVENT_PHY_LOF_INT
			&& PHY_NO_LOS_HAPPEN == XPON_PHY_GET(PON_GET_PHY_LOS_STATUS)){
			return;
		}
		
		gpPonSysData->sysLinkStatus = PON_LINK_STATUS_OFF ;
		gpPhyData->phy_link_status  = PHY_LINK_STATUS_LOS ;
		/* report GPON event message */
		xmcs_report_event(XMCS_EVENT_TYPE_GPON, XMCS_EVENT_GPON_LOS, 0) ;
#ifdef TCSUPPORT_OPENWRT
		fiber_hotplug_event(REMOVE_FIBER);
#endif
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_XPON_HAL_API_EXT)
		XPON_PHY_NOTIFY_EVENT(PHY_XPON_STATE_NOTIFY_LOS) ;
#endif/*TCSUPPORT_COMPILE*/

		//xmcs_set_connection_start(PHY_ENABLE);
		/* Change the current state */
#ifdef TCSUPPORT_CPU_EN7521
		XPON_DPRINT_MSG("call gpon disable.\n");
		gpon_disable() ;
#else
		gpon_disable_with_option(GPON_DEV_RESET_WITH_FE_RESET);
#endif
	} else if( GPON_STATE_O5 == GPON_CURR_STATE) {

		if((!gpGponPriv->lofEnableFlag) && (id == PHY_EVENT_PHY_LOF_INT)){
			return;
		}
		gpPhyData->phy_link_status  = PHY_LINK_STATUS_LOS ;
#if defined(TCSUPPORT_RA_HWNAT) && defined(TCSUPPORT_RA_HWNAT_ENHANCE_HOOK)
		is_hwnat_dont_clean = 1;
#endif

#ifdef TCSUPPORT_CPU_EN7521
		/* addjust MAC internal delay for EqD */
		disable_sniffer_mode();
#else /* TCSUPPORT_CPU_EN7521 */
    	if ( (gpGponPriv->gponCfg.macPhyReset & (GPON_MAC_RESET_BIT | PHY_ANALOG_RESET_BIT | PHY_DIGITAL_RESET_BIT) ) )
    	{
    	   gpon_dev_typeB_reset();
    	}

    	if (gpGponPriv->gponCfg.rstDbgDly ){
    	   gpon_reset_dbg_delay();
    	}
#endif /* TCSUPPORT_CPU_EN7521 */
		/* Change the current state */
		gpon_act_change_gpon_state(GPON_STATE_O6) ;
#ifdef TCSUPPORT_CPU_EN7521
		/*here start the typeB action*/
		PON_MSG(MSG_INT, "GPON IRQ: here start the typeB action.\n") ;
#if defined(TCSUPPORT_CPU_EN7581) || defined(TCSUPPORT_CPU_EN7523)
		disable_cpu_us_traffic();
		gponDevGetEncryptKey(&gpGponPriv->pGponRecovery->recoveraesKey);
		gpon_typeb_dev_reset_for_typeB();
#else
		gwan_remove_all_tcont();
		gwan_remove_all_gemport_totally();
		gponDevGetEncryptKey(&gpGponPriv->pGponRecovery->recoveraesKey);
		gpon_typeb_dev_reset();
		gpon_dev_typeB_restore();
		if(0 != gwan_create_new_tcont(GPON_ONU_ID))
		{
			PON_MSG(MSG_INT, "GPON IRQ: gwan_create_new_tcont fail.\n") ;
		}
		if(0 != gwan_create_new_gemport(GPON_OMCC_ID, 0, GPON_OMCC_GEM_ENCRYPTION,GPON_ONU_ID))
		{
			PON_MSG(MSG_INT, "GPON IRQ: gwan_create_new_gemport fail.\n") ;
		}
		gponDevResetCtrl(XPON_DISABLE);
#endif
#endif
	}
	
	gpGponPriv->gponCfg.flags.dvtGponLosFlag = 0 ;
}

GPON_DYING_GASP_INFO_P gp_dying_info = NULL;
void gpon_init_dying_info(void)
{
	gp_dying_info	= 	&gpGponPriv->gponCfg.dying_info;
    memset(gp_dying_info,0,sizeof(GPON_DYING_GASP_INFO_T));
	gp_dying_info->opt_type = GPON_DG_SEND_ONEBYONE;
    gp_dying_info->total_send_cnt = GPON_DG_ONEBYONE_TOTAL_CNT;
    gp_dying_info->delay_time = GPON_DG_DEFAULT_DELAY_INTERVAL;
    
  	gpPonSysData->dyingGaspData.isGponHwFlag = GPON_SW;
}
void gpon_set_ploamu_int(unchar flag)
{
    REG_G_INT_ENABLE gponIntEnable;
    
	if(flag > 1) 	
        return;
    
    gponIntEnable.Raw = 0;
    gponIntEnable.Raw = IO_GREG(G_INT_ENABLE);
    gponIntEnable.Bits.ploamu_send_int_en =	flag;
    IO_SREG(G_INT_STATUS, 0xFFFFFFFF) ;
	IO_SREG(G_INT_ENABLE, gponIntEnable.Raw) ;
}

/*****************************************************************************
******************************************************************************/
void gpon_detect_dying_gasp(void)
{
	int i ;
	
	if(gp_dying_info->opt_type == GPON_DG_SEND_ONEBYONE)
    {
        gpon_set_ploamu_int(TRUE); 
        gp_dying_info->current_send_cnt = 0;
        //printk("gpon_detect_dying_gasp send one by one, set ploamu true\n");
    }
    
	for(i=0 ; i<1 ; i++) {
		ploam_send_dying_gasp() ;
	}

	//report GPON event message
	//gpon_event_report(...) ;
}

/*****************************************************************************
******************************************************************************/
void gpon_sn_request_threshold_crossed(void)
{
	//Get the MAC register
	//get the current tx_power_mode
	
	//Set the MAC register
	//set the new tx_power_mode (= (tx_power_mode + 1)%3)
}

void gpon_isr_send_rdi(void)
{
#if defined(TCSUPPORT_CPU_EN7526) || defined(TCSUPPORT_CPU_EN7527)
	gpon_software_rdi_send();
#else
#if !defined(TCSUPPORT_CPU_EN7523) && !defined(TCSUPPORT_CPU_EN7581)
	uint raw = 0;
	raw = IO_GREG(DBG_SW_RDI_CTRL);
	IO_SREG(DBG_SW_RDI_CTRL, raw|0x0001);	
#endif
#endif
	gpon_set_alarmBit(RDI_INDEX);
}

/*****************************************************************************
******************************************************************************/
bool sd_flag = false;
bool sf_flag = false;

void gpon_ber_interval_expires(TIMER_FUN_PAAM arg)
{
	uint bip_cnt ;
    uint interval;
	
	if(GPON_CURR_STATE == GPON_STATE_O5) {
		GPON_START_TIMER(gpGponPriv->ber_timer,gpGponPriv->gponCfg.berInterval) ;
	
		bip_cnt = XPON_PHY_GET(PON_GET_PHY_BIP_COUNTER) ;
        gpGponPriv->bip_cnt_val += bip_cnt ;
		//Send a REI PLOAM message
		ploam_send_rei_msg(bip_cnt, &gpGponPriv->gponCfg.reiSeq) ;

        interval = gpGponPriv->gponCfg.berInterval; /* ms */
        if((xmcs_get_SD_SF_BIP_NUM(gGpon_SD_SF_Info.SD_thld)) <= ((bip_cnt* 1000)/interval))
        {
            if(false == sd_flag)
            {
                printk(KERN_ERR "PLOAM: SD ALARM Signal degraded.\n") ;
                sd_flag = true;
            }
            gGpon_SD_SF_Info.SD_cnt++;
			gpon_set_alarmBit(SD_INDEX);
        }
        else if(true == sd_flag)
        {
            printk(KERN_ERR "PLOAM: SD ALARM Signal cleared.\n") ;
            sd_flag = false;
        }

        if((xmcs_get_SD_SF_BIP_NUM(gGpon_SD_SF_Info.SF_thld)) <= ((bip_cnt* 1000)/interval))
        {
            if(false == sf_flag)
            {
                printk(KERN_ERR "PLOAM: SF ALARM Signal failed.\n") ;
                sf_flag = true;
            }
            gGpon_SD_SF_Info.SF_cnt++;
            gpon_isr_send_rdi();
			gpon_set_alarmBit(SF_INDEX);
        }
        else if(true == sf_flag)
        {
            printk(KERN_ERR "PLOAM: SF ALARM Signal cleared.\n") ;
            sf_flag = false;
        }

	}
}

/*****************************************************************************
******************************************************************************/
void gpon_silence_interval_expires(TIMER_FUN_PAAM arg)
{
    /*Clear silence!*/
	gpGponPriv->gpon_silence = 0;
}

void gpon_start_silence(void)
{
	gpGponPriv->gpon_silence = 1;
	GPON_START_TIMER(gpGponPriv->silence_timer,gpGponPriv->gponCfg.silenceInterval) ;
}

void gpon_stop_silence(void)
{
	gpGponPriv->gpon_silence = 0;
	GPON_STOP_TIMER(gpGponPriv->silence_timer) ;
}
void gpon_update_silence(void)
{
	GPON_STOP_TIMER(gpGponPriv->silence_timer) ;
}

/*****************************************************************************
******************************************************************************/

static inline int is_same_ploam_msg_as_previous( PLOAM_RAW_General_T * pMsg)
{
    switch( pMsg->raw.msg_id ){
        case PLOAM_DOWN_MSG_RANGING_TIME:
            return !memcmp(&gpGponPriv->prePloamMsg, pMsg, 7) ;
            
        default:
            return !memcmp(&gpGponPriv->prePloamMsg, pMsg, sizeof(PLOAM_RAW_General_T)) ;
        }
}

static inline int should_ignore_ploam_msg(PLOAM_RAW_General_T * pPloamMsg)
{
    static int same_cnt = 0;
    int is_same_msg = 0;
    if(PON_WAN_START != gpPonSysData->sysStartup){
        PON_MSG(MSG_OAM, "WAN not start, ignore ploam msg:%8X\n", htonl(pPloamMsg->value[0])) ;
        return TRUE;
    }


    /* 3 consecutivly same ploam msg, only process once */
    is_same_msg = is_same_ploam_msg_as_previous(pPloamMsg);
    
    if(is_same_msg) same_cnt ++;

    if(is_same_msg && (same_cnt % 3) != 0 ){
	    PON_MSG(MSG_OAM, "Receive the PLOAM message same:%8X\n", htonl(pPloamMsg->value[0])) ;
        return TRUE;
    }else{
        memcpy(&gpGponPriv->prePloamMsg, pPloamMsg, sizeof(PLOAM_RAW_General_T)) ;
        same_cnt = 0;
    }

    return FALSE;
}

void gpon_recv_ploam_message(void)
{
	PLOAM_RAW_General_T ploamMsg;
	int depth=0;

	do {
		if((depth = gponDevGetPloamMsg(&ploamMsg)) <= 0) {
			PON_MSG(MSG_ERR, "Receive PLOAM message failed.\n") ;
            return;
		} else if(FALSE == should_ignore_ploam_msg(&ploamMsg) ){
            ploam_parser_down_message(&ploamMsg) ;
        }
	} while(depth>3) ;
	gpGponPriv->ploamMsgcounter.rxPloamMsgCnt++;
}

/*****************************************************************************
******************************************************************************/
int gpon_update_time_to_switch(void)
{
	uint sec, nanosec ;
	
	/* get the current tod from MAC */
	gponDevGetCurrentTod(&sec, &nanosec) ;
//	PON_MSG(MSG_TRACE, "Get the current TOD from GPON MAC. sec:%d, nanosec:%d\n", sec, nanosec) ;
	printk("Update the TOD to switch. sec:%d, nanosec:%d.\n\n", sec, nanosec) ;
	
	/* set the current second+1 to switch and enable 1pps update */
	//Set to Switch
	
	return 0 ;	
}

/*****************************************************************************
******************************************************************************/
void gpon_dbg_grp_err_check(void)
{	
	REG_DBG_GRP_0 dbgGrp_0;
	REG_DBG_GRP_1 dbgGrp_1;

	PON_MSG(MSG_ERR,"[%s][%d]:REG_DBG_GRP_0[%x],REG_DBG_GRP_1[%x]\n",__FUNCTION__,__LINE__,IO_GREG(DBG_GRP_0),IO_GREG(DBG_GRP_1));

	dbgGrp_0.Raw = IO_GREG(DBG_GRP_0);
	dbgGrp_1.Raw = IO_GREG(DBG_GRP_1);
	
	//if DBG_GRP_0 Bit:0/5/21 occur,then print status
	if(dbgGrp_0.Bits.ether_fcs_err)
	{
#if !defined(TCSUPPORT_CPU_EN7523) && !defined(TCSUPPORT_CPU_EN7581)
		tasklet_hi_schedule(&gpGponPriv->gpon_check_rx_loss);
#endif
		PON_MSG(MSG_ERR,"DBG_GRP_0:RX Ethernet FCS error!\n");
	}
	if (dbgGrp_0.Bits.mbi_pl_fifo_ovrn)
	{
		PON_MSG(MSG_ERR,"DBG_GRP_0:Payload FIFO in MBI module overrun!\n");
	}
	if (dbgGrp_0.Bits.aes_rdm_ciph_fifo_ovrn)
	{
		PON_MSG(MSG_ERR,"DBG_GRP_0:AES engine random cipher FIFO overrun occur!\n");
	}

	//clear the register DBG_GRP_0
	dbgGrp_0.Raw = 0xFFFFFFFF;
	IO_SREG(DBG_GRP_0, dbgGrp_0.Raw);

	//if DBG_GRP_1 Bit:0/1/2/3/4/5/6/7/8/16/19/24 occur,then print status
	if(dbgGrp_1.Bits.logd)
	{
		PON_MSG(MSG_ERR,"DBG_GRP_1:RX Loss of GEM Delineation!\n");
	}
	if(dbgGrp_1.Bits.blen_err)
	{
		PON_MSG(MSG_ERR,"DBG_GRP_1:Blen filed error!\n");
	}
	if(dbgGrp_1.Bits.bwm_uc_err)
	{
		PON_MSG(MSG_ERR,"DBG_GRP_1:BWM un-correctable error!\n");
	}
	if(dbgGrp_1.Bits.ploamd_crc_err)
	{
		PON_MSG(MSG_ERR,"DBG_GRP_1:RX PLOAMd CRC error!\n");
	}
	if(dbgGrp_1.Bits.mbi_hdr_fifo_ovrn)
	{
		PON_MSG(MSG_ERR,"DBG_GRP_1:MBI HDR FIFO Overrun error!\n");
	}
	if(dbgGrp_1.Bits.aes_ciph_txt_fifo_ovrn)
	{
		PON_MSG(MSG_ERR,"DBG_GRP_1:AES engine cipher text FIFO overrun!\n");
	}
	if(dbgGrp_1.Bits.aes_cryp_cnt_fifo_ovrn)
	{
		PON_MSG(MSG_ERR,"DBG_GRP_1:AES engine crypto counter FIFO overrun!\n");
	}
	if(dbgGrp_1.Bits.sgl_next_full)
	{
		PON_MSG(MSG_ERR,"DBG_GRP_1:BWM single FIFO next full!\n");
	}
	if(dbgGrp_1.Bits.bst_next_full)
	{
		PON_MSG(MSG_ERR,"DBG_GRP_1:BWM burst FIFO next full!\n");
	}
	if(dbgGrp_1.Bits.tx_late_start)
	{
		PON_MSG(MSG_ERR,"DBG_GRP_1:TX start time is later than TX counter!\n");
	}
	if(dbgGrp_1.Bits.tx_align_fifo_udrn)
	{
		PON_MSG(MSG_ERR,"DBG_GRP_1:TX alignment FIFO underrun!\n");
	}
	if(dbgGrp_1.Bits.alen_neq_zero)
	{
		PON_MSG(MSG_ERR,"DBG_GRP_1:Received Alen field is not equal to zero!\n");
	}
	
	//clear the register DBG_GRP_1
	dbgGrp_1.Raw = 0xFFFFFFFF;
	IO_SREG(DBG_GRP_1, dbgGrp_1.Raw);	

}

/*****************************************************************************
******************************************************************************/
void gpon_isr(void)
{
	REG_G_INT_STATUS intStatus ;
	REG_G_INT_ENABLE intEnable ;
#ifndef TCSUPPORT_CPU_EN7581
	REG_G_NEW_TOD_SEC_L32 gponTodSec ;
	REG_G_NEW_TOD_NANO_SEC gponTodNano ;
	uint sec, nanosec;
#endif
	uint spfCnt;
	unsigned long flags;

	intStatus.Raw = IO_GREG(G_INT_STATUS) ;
	intEnable.Raw = IO_GREG(G_INT_ENABLE) ;

		//Clear the interrupt status
		intStatus.Raw &= intEnable.Raw ;
		IO_SREG(G_INT_STATUS, intStatus.Raw) ;
	
    spin_lock_irqsave(&gpPonSysData->event_lock, flags) ;
    	
	if(intStatus.Raw & intEnable.Raw) {
		if(intStatus.Raw & GPON_INT_INDICATION) {
			if(intStatus.Bits.dying_gasp_send_int) {
				/* Clear pre dying gasp interrupt status */
				//IO_SBITS(0xBFB00084, SCU_DYING_GASP_STATUS) ; 
				PON_MSG(MSG_INT, "GPON IRQ: send a dying gasp ploam message interrupt.\n") ;
			} 
			if(intStatus.Bits.aes_key_switch_done_int) {
				PON_MSG(MSG_INT, "GPON IRQ: AES key switch done interrupt.\n") ;
			}
			if(intStatus.Bits.ranging_req_recv_int) {
				PON_MSG(MSG_INT, "GPON IRQ: Ranging_Request received interrupt.\n") ;
			}
            if(intStatus.Bits.sn_onu_send_o4_int) {
				PON_MSG(MSG_INT|MSG_TYPEB, "GPON IRQ: SN_ONU send in O4 state interrupt.DBG_GRP_0[%x], DBG_GRP_1[%x]\n", IO_GREG(DBG_GRP_0), IO_GREG(DBG_GRP_1)) ;
				gpGponPriv->ploamMsgcounter.txPloamMsgCnt++;
			}
			if(intStatus.Bits.sn_req_recv_int) {
				PON_MSG(MSG_INT|MSG_TYPEB, "GPON IRQ: SN_Request received interrupt.\n") ;
			}
			if(intStatus.Bits.sn_onu_send_o3_int) {
				PON_MSG(MSG_INT|MSG_TYPEB, "GPON IRQ: SN_ONU sent in O3 state interrupt.DBG_GRP_0[%x], DBG_GRP_1[%x]\n", IO_GREG(DBG_GRP_0), IO_GREG(DBG_GRP_1)) ;
				gpGponPriv->ploamMsgcounter.txPloamMsgCnt++;
			}
#if defined(TCSUPPORT_CPU_EN7580) || defined(TCSUPPORT_CPU_EN7528)
			if(intStatus.Bits.sn_req_recv_o5_int) {
				PON_MSG(MSG_INT, "GPON IRQ: SN request recv in O5 state interrupt.\n") ;
			}
#endif
#if		defined(TCSUPPORT_CPU_EN7523)
#if defined(TCSUPPORT_CPU_EN7581) || defined(TCSUPPORT_CPU_AN7552)
			if(intStatus.Bits.cal_gnt_zero_int)
			{
				PON_MSG(MSG_INT, "GPON IRQ: calculate grant zero interrupt.\n") ;
			}
#else
            if(intStatus.Bits.cal_gnt_size_done_int) {
				gponDevGetSuperframe(&spfCnt);
	            PON_MSG(MSG_INT, "GPON IRQ: calculate grant size done interrupt. cal_gnt_size_num:%u, Superframe counter:%08x\n",IO_GREG(CAL_GNT_SIZE_SUM),(spfCnt-1)) ;
            }
#endif
#endif
#ifdef TCSUPPORT_CPU_EN7521
			if(intStatus.Bits.o5_eqd_adj_done_int) {
				PON_MSG(MSG_INT, "GPON IRQ: EqD adjustment done interrupt in O5 state interrupt.\n") ;
			}
			if(intStatus.Bits.olt_ds_fec_chg_int) {
				PON_MSG(MSG_INT, "GPON IRQ: DS fec change interrupt.\n") ;
			}
			if(intStatus.Bits.onu_us_fec_chg_int) {
				PON_MSG(MSG_INT, "GPON IRQ: US fec change interrupt.\n") ;
			}
			if(intStatus.Bits.lwi_int) {
				PON_MSG(MSG_INT, "GPON IRQ: LWI interrupt.\n") ;
			}
			if(intStatus.Bits.fwi_int) {
				PON_MSG(MSG_INT, "GPON IRQ: FWI interrupt.\n") ;
			}
			if(intStatus.Bits.pop_up_recv_in_O6_int) {
				PON_MSG(MSG_INT, "GPON IRQ: POP_UP receive in O6 interrupt.\n") ;
			}
#endif			
		} 
		if(intStatus.Raw & GPON_INT_TOD) {
#ifndef TCSUPPORT_CPU_EN7581
			if(intStatus.Bits.tod_1pps_int) {
				if(gpGponPriv->gponCfg.flags.isTodUpdate) {
					gpGponPriv->gponCfg.flags.isTodUpdate = 0 ;
					gpon_update_time_to_switch() ;
					gponDevSet1PPSInt(XPON_DISABLE) ;
				}
				PON_MSG(MSG_INT, "GPON IRQ: ToD 1PPS interrupt.\n") ;
			} 
			if(intStatus.Bits.tod_update_done_int) {
					gponDevGetCurrentTod(&sec, &nanosec) ;
					gponTodSec.Raw = IO_GREG(G_NEW_TOD_SEC_L32);
					gponTodNano.Raw = IO_GREG(G_NEW_TOD_NANO_SEC);
					printk("Get the current TOD from GPON MAC. sec:%d, nanosec:%d, interval:%d\n", 
						sec, nanosec, (uint)((sec - gponTodSec.Bits.new_tod_sec_l32)*1000000000 + (int)(nanosec - gponTodNano.Bits.new_tod_nano_sec))) ;
				gpGponPriv->gponCfg.flags.isTodUpdate = 1 ;
				gponDevSet1PPSInt(XPON_ENABLE) ;
				PON_MSG(MSG_INT, "GPON IRQ: ToD updated done interrupt.\n") ;
			}
#endif
		}
		if(intStatus.Raw & GPON_INT_PLOAM) {
			if(intStatus.Bits.ploamu_send_int) {
				PON_MSG(MSG_INT, "GPON IRQ: A PLOAM message sent interrupt.\n") ;
                if((gp_dying_info->opt_type == GPON_DG_SEND_ONEBYONE) 
                && (gp_dying_info->dying_flag == TRUE))
                {
                	PON_MSG(MSG_ERR,"gpon isr rcv %d times dying gasp ploamu int\n",gp_dying_info->current_send_cnt+1);
                	if(gp_dying_info->current_send_cnt < (gp_dying_info->total_send_cnt - 1))
                    {
	                	gp_dying_info->current_send_cnt++;
                        delay1us(gp_dying_info->delay_time);
	                	ploam_send_dying_gasp();
                        PON_MSG(MSG_ERR,"gpon isr send %d times dying gasp\n",gp_dying_info->current_send_cnt+1);
                    }
                    else
                    {
                    	gp_dying_info->current_send_cnt = 0;
                        gpon_set_ploamu_int(FALSE);
                        gp_dying_info->dying_flag = FALSE;
                        PON_MSG(MSG_ERR,"gpon isr clear dying gasp status\n");
                    }
                }
			}
			if(intStatus.Bits.ploamd_recv_int) {
				PON_MSG(MSG_INT, "GPON IRQ: A PLOAM message received interrupt.DBG_GRP_0[%x], DBG_GRP_1[%x]\n", IO_GREG(DBG_GRP_0), IO_GREG(DBG_GRP_1)) ;
				gpon_recv_ploam_message();
			}
		}
		if(intStatus.Raw & GPON_INT_ERROR) {
//			gpon_dbg_grp_err_check();
			if(intStatus.Bits.bwm_us_fec_err_int) {
				PON_MSG((MSG_INT|MSG_ERR), "GPON IRQ: US FEC status error interrupt.\n") ;
			}
			if(intStatus.Bits.los_gem_del_int) {
#if/*TCSUPPORT_COMPILE*/ !defined(TCSUPPORT_XPON_HAL_API_NG)
#if defined(TCSUPPORT_CPU_EN7526) || defined(TCSUPPORT_CPU_EN7527)
				gpon_isr_send_rdi();
#endif
#endif/*TCSUPPORT_COMPILE*/
				gpon_set_alarmBit(LCDG_INDEX);
				PON_MSG((MSG_INT|MSG_ERR), "GPON IRQ: Loss of GEM delineation interrupt.\n") ;
			}
			if(intStatus.Bits.sn_req_crs_int) { 
				PON_MSG((MSG_INT|MSG_ERR), "GPON IRQ: SN_Request_Threshold crossed interrupt.\n") ;
			}   
			if(intStatus.Bits.rx_eof_err_int) {
				PON_MSG((MSG_INT|MSG_ERR), "GPON IRQ: PHY_RX_EOF signal error interrupt.\n") ;
			}
			if(intStatus.Bits.tx_late_start_int) {
				PON_MSG((MSG_INT|MSG_ERR), "GPON IRQ: Tx local time late interrupt.\n") ;
			}
			if(intStatus.Bits.bst_sgl_diff_int) {
				PON_MSG((MSG_INT|MSG_ERR), "GPON IRQ: Burst and single grant buffer mismatch interrupt.\n") ;
				gponDevGetSuperframe(&spfCnt);
				PON_MSG((MSG_INT|MSG_ERR), "GPON IRQ: Superframe counter:%08x\n", spfCnt) ;
			}
			if(intStatus.Bits.fifo_err_int) {
				PON_MSG((MSG_INT|MSG_ERR), "GPON IRQ: FIFO error interrupt\n") ;
			}
#ifdef TCSUPPORT_CPU_EN7521
			if(intStatus.Bits.bwm_stop_time_err_int) {
				if(gpGponPriv->gponCfg.flags.isBwmStopTimeErrInt == XPON_DISABLE) {
					gponDevSetBwmStopTimeInt(XPON_DISABLE);
				}
				gponDevGetSuperframe(&spfCnt);
				PON_MSG((MSG_INT|MSG_ERR), "GPON IRQ: OLT assigned stop time error when US FEC on interrupt.\n") ;
				PON_MSG((MSG_INT|MSG_ERR), "GPON IRQ: Superframe counter:%08x\n", spfCnt) ;
			}
			if(intStatus.Bits.rx_gem_intlv_err_int) {
				PON_MSG((MSG_INT|MSG_ERR), "GPON IRQ: Rx more than two interleaved GEM fragment interrupt.\n") ;
			}
			if(intStatus.Bits.bfifo_full_int) {
				PON_MSG((MSG_INT|MSG_ERR), "GPON IRQ: bfifo full interrupt.\n") ;
			}
			if(intStatus.Bits.sfifo_full_int) {
				PON_MSG((MSG_INT|MSG_ERR), "GPON IRQ: sfifo full interrupt.\n") ;
			}
#endif
			if(intStatus.Bits.rx_err_int) {
#if/*TCSUPPORT_COMPILE*/ !defined(TCSUPPORT_XPON_HAL_API_NG)
#if defined(TCSUPPORT_CPU_EN7526) || defined(TCSUPPORT_CPU_EN7527)
				gpon_isr_send_rdi();
#endif
#endif/*TCSUPPORT_COMPILE*/
				PON_MSG((MSG_INT|MSG_ERR), "GPON IRQ: Rx error interrupt.\n") ;
			}
#ifdef XPON_MAC_CONFIG_DEBUG
			intStatusDbg.Raw |= intStatus.Raw;
			if(gpPonSysData->debugLevel & (MSG_INT|MSG_ERR)) {
				gpon_dev_get_error_status();
			}
#endif
		}
#ifdef XPON_MAC_CONFIG_DEBUG
		if((gpPonSysData->debugLevel&MSG_DBG) && (intStatus.Raw&GPON_INT_ERROR)) {
			PON_MSG(MSG_DBG, "[4220:%.8x],[4224:%.8x],[4228:%.8x],[422C:%.8x],[4240:%.8x]\n", \
				IO_GREG(DBG_BWM_FILTER_CTRL), IO_GREG(DBG_BWM_SFIFO_STS), IO_GREG(DBG_GRP_0), IO_GREG(DBG_GRP_1),IO_GREG(DBG_PROBE_CTRL)) ;
			IO_SBITS(DBG_BWM_FILTER_CTRL, (0x07 << 29));
			PON_MSG(MSG_DBG, "[4300:%.8x],[4304:%.8x],[4308:%.8x],[430C:%.8x],[4310:%.8x]\n\n", \
				IO_GREG(DBG_RX_GEM_CNT), IO_GREG(DBG_RX_CRC_ERR_CNT), IO_GREG(DBG_RX_GTC_CNT),IO_GREG(DBG_TX_GEM_CNT), IO_GREG(DBG_TX_BST_CNT)) ;
			IO_SREG(DBG_GRP_0, 0xFFFFFFFF);
			IO_SREG(DBG_GRP_1, 0xFFFFFFFF);
		}
#endif
	} else {
		PON_MSG(MSG_INT, "GPON IRQ: NULL GPON ISR, Status:%.8x, Mask:%.8x\n", intStatus.Raw, intEnable.Raw) ;
	}

    spin_unlock_irqrestore(&gpPonSysData->event_lock, flags) ;
	gpon_check_alarm_jiffiesClk();
}

void gpon_record_emergence_info(unchar event)
{
	GPON_Emergence_Info *info = NULL;

	if(event >= GPON_EMERGENCE_STATE_MAX_NUM)
	{
		PON_MSG(MSG_ERR,"%s event exceed max num\n",__FUNCTION__);
		return;
	}
	
	if(atomic_read(&gEmerStateindex) >= GPON_EMERGENCE_STATE_MAX_NUM)
	{
		atomic_set(&gEmerStateindex,0);
		PON_MSG(MSG_DBG, "gEmerStateindex exceed max num set 0\n");
	}
	PON_MSG(MSG_DBG, "record gEmerStateindex %d \n",atomic_read(&gEmerStateindex));
	
	info = &gEmergenceRecord[atomic_read(&gEmerStateindex)];
	
	memset(info, 0, sizeof(GPON_Emergence_Info));
	
	info->isHappen = TRUE;
	info->time = ktime_get();
	info->event = event;
	info->emergenceState = gpGponPriv->gponCfg.emergencystate;
	atomic_inc(&gEmerStateindex);
}

static inline void gpon_print_time_information(ktime_t * time)
{    
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,36)
	u32 nsec = time->tv.nsec % 1000;
	u32 usec = (time->tv.nsec / 1000) % 1000;    
	u32 msec = (time->tv.nsec / 1000000) % 1000;   
	u32 sec = time->tv.sec % 60;
	u32 min = time->tv.sec / 60;
	printk("[%03dmin : %03ds : %03dms: %03dus : %03dns] ", min, sec, msec, usec, nsec);
#endif
}

void gpon_show_emergence_info(void)
{
	int i = 0;
	int printId = 0;
	GPON_Emergence_Info *info = NULL;
	
	for(i = 0; i < GPON_EMERGENCE_STATE_MAX_NUM; i++)
	{
		printId = (i + atomic_read(&gEmerStateindex)) % GPON_EMERGENCE_STATE_MAX_NUM;
		info = &gEmergenceRecord[printId];
		
		PON_MSG(MSG_DBG, "show printId %d, event %u ishappen %u\n",printId,info->event,info->isHappen);
		
		if(info->isHappen == TRUE)
		{
			gpon_print_time_information(&info->time);
			switch(info->event)
			{
				case GPON_DISABLE_SN_REPORT_O7:
					printk("GPON RCV DISABLE_SN REPORTO7          ");
					break;
				case GPON_DISABLE_SN_SET_EMERGNCE_STATE:
					printk("GPON RCV DISABLE_SN SET_EMERGNCE_STATE");
					break;
				case GPON_OMCI_SET_EMERGNCE_STATE:
					printk("GPON OMCI SET EMERGNCE_STATE          ");
					break;
				case GPON_PHY_READY_REPORT_O7:
					printk("GPON PHY READY REPORT_O7              ");
					break;
				case GPON_SET_CONNECTION_REPORT_O7:
					printk("GPON SET CONNECTION START REPORT_O7   ");
					break;	
				default:
					printk("No Info   ");
					break;
			}
			printk("  emergence state %s\n",(info->emergenceState)?"ON":"OFF");
		}
	}
}

void gpon_stop_timer(void)
{
	GPON_STOP_TIMER(gpGponPriv->to1_timer) ;
	GPON_STOP_TIMER(gpGponPriv->to2_timer) ;
	GPON_STOP_TIMER(gpGponPriv->ber_timer) ;
	GPON_STOP_TIMER(gpWanPriv->gpon.gemMibTimer) ;  
	GPON_STOP_TIMER(gpWanPriv->gpon.setQdmaTxBuffTimer) ;

#if defined(TCSUPPORT_CPU_EN7526) || defined(TCSUPPORT_CPU_EN7527)
    RDI_TIMER_EN = 0;
    if(RDI_SEND_ENABLE){
        GPON_STOP_TIMER(rdi_timer);
    }
#endif 
    stop_omci_oam_monitor();
}

int gpon_create_timer(struct timer_list *timer, gponTimerCallback callback, unsigned long param)
{
	if((timer == NULL) ||( callback == NULL) ){
		return -1;
	}
	
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,15,0) 
	init_timer(timer);
	timer->function = callback;
#else
	timer_setup(timer, callback, 0);
#endif
	timer->expires= param;

	return 0;
}

void gpon_set_alarmBit(ALARM_INDEX_T index)
{
	uint tmp = 0;
	PON_MSG(MSG_DBG, "gpon set alarm bit : %d.\n",index);
	tmp = *((uint *)&(gpGponPriv->gponAlarm)) | (1 << index);
	gpGponPriv->gponAlarm = *((GPON_Alarm_T *)&tmp);
	gpGponPriv->jiffiesClk[index] = jiffies + gpGponPriv->gponAlmKeepTime  * HZ;
}

void gpon_clear_alarmBit(ALARM_INDEX_T index)
{
	uint tmp = 0;
	PON_MSG(MSG_DBG, "gpon clear alarm bit : %d.\n",index);
	if(gpGponPriv->gponAlmKeepTime == 0){
		return;
	}
	tmp = *((uint *)&(gpGponPriv->gponAlarm)) & ~(1 << index);
	gpGponPriv->gponAlarm = *((GPON_Alarm_T *)&tmp);
}

static int should_alarm_clear_by_clk(ALARM_INDEX_T index){
	int ret = 1;

	switch(index){
		case TF_INDEX:
		case LODS_INDEX:
		case DIS_INDEX:
		case DACT_INDEX:
		case LOS_INDEX:
		case LOF_INDEX:
			ret = 0;
			break;
		default:
			break;
	}

	return ret;
}
void gpon_check_alarm_jiffiesClk(void)
{
	ALARM_INDEX_T index = RDI_INDEX;
	uint tmp = 0;

	if(gpGponPriv->gponAlmKeepTime == 0){
		return;
	}

	PON_MSG(MSG_INT, "GPON ISR: enter checking alarm.\n") ;
	for( index = RDI_INDEX; index < MAX_INDEX; index++){
		if(should_alarm_clear_by_clk(index) == 0)
			continue;
		if(time_after_eq(jiffies,gpGponPriv->jiffiesClk[index])){;
			tmp = *((uint *)&(gpGponPriv->gponAlarm)) & ~(1 << index);
			gpGponPriv->gponAlarm = *((GPON_Alarm_T *)&tmp);
		}
	}
}

void gpon_reset_alarm_jiffiesClk(void)
{
	ALARM_INDEX_T index = RDI_INDEX;
	uint tmp = *(uint *)&gpGponPriv->gponAlarm;
	for( index = RDI_INDEX; index < MAX_INDEX; index++){
		if(tmp &  1 << index)
			gpGponPriv->jiffiesClk[index] = jiffies + gpGponPriv->gponAlmKeepTime  * HZ;
	}
	printk("gpon_reset_alarm_jiffiesClk, gponAlmKeepTime=%x\n",gpGponPriv->gponAlmKeepTime);
}

void gpon_clear_all_alarm(void)
{
	memset(&gpGponPriv->gponAlarm,0,sizeof(gpGponPriv->gponAlarm));
	printk("gpon_clear_all_alarm\n");
}


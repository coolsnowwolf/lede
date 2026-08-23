/*
* File Name: gpon.c
* Description: 
*
******************************************************************/
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/types.h>

#include <gpon/gpon.h>
#include <common/drv_global.h>
#include <common/xpon_daemon.h>
#include <common/phy_if_wrapper.h>
#include <common/xpon_led.h>


GPON_Emergence_Info gEmergenceRecord[GPON_EMERGENCE_STATE_MAX_NUM]={0};
atomic_t gEmerStateindex = ATOMIC_INIT(0);
extern void  dbgtoMem(__u32 debugLevel, char *fmt,...);
extern void gpon_RDI_send(void);

/******************************************************************************
******************************************************************************/
void prepare_gpon(void)
{
#ifndef TCSUPPORT_CPU_EN7521
    if(TRUE == gpPonSysData->gswRateLimitFlag){
        XPON_STOP_TIMER(gpPonSysData->gsw_p6_rate_timer);
        xpon_disable_cpu_protection();
    }
#endif

    /*enable green drop function default*/
    xpon_set_qdma_qos(XPON_ENABLE);
	
    select_xpon_wan(PHY_GPON_CONFIG);
	XPON_PHY_SET_MODE(PHY_GPON_CONFIG);

    gpon_disable() ;
}

void schedule_gpon_dev_reset(GPON_DEV_RESET_TYPE_t type)
{
#ifndef TCSUPPORT_CPU_EN7521
    if( GPON_DEV_RESET_WITH_FE_RESET == type){
        schedule_fe_reset();
        return;
    }
#endif

    if(PHY_LINK_STATUS_READY == gpPhyData->phy_link_status){
        gpon_act_change_gpon_state(GPON_STATE_O2);
    }else{
        gpon_act_change_gpon_state(GPON_STATE_O1);
    }
    
    XPON_DPRINT_MSG("#######About to do GPON MAC and XPON PHY Reset!#######\n");

    XPON_DAEMON_Job_data_t job     ;
    job.id       = XPON_DAEMON_JOB_GPON_DEV_RESET ;
    job.priority = XPON_DAEMON_JOB_PRIORITY_HIGH  ;

    xpon_daemon_job_enqueue(&job)  ;    
    wake_up_xpon_daemon() ;
}

int gpon_disable_with_option(GPON_DEV_RESET_TYPE_t reset_type)
{
	REG_G_GBL_CFG gponGlbCfg ;

	int ret = 0 ;

	unregister_gpon_isr() ;
	
	ret = gpon_qos_deinit() ;
	if(ret != 0) {
		PON_MSG(MSG_ERR, "GPON QoS deinitialization failed.\n") ;
		return ret ;
	}

	/* reset the onu_id and omcc id */
	gpGponPriv->gponCfg.onu_id = GPON_UNASSIGN_ONU_ID ;
	gpGponPriv->gponCfg.omcc = GPON_UNASSIGN_GEM_ID ;
	gpGponPriv->gponCfg.flags.isRequestKey = 0 ;
    gpGponPriv->typeBOnGoing = 0;

	/* Disable the MAC tx FEC */
	gponGlbCfg.Raw = IO_GREG(G_GBL_CFG) ;
	gponGlbCfg.Bits.us_fec_en = 0 ;
	IO_SREG(G_GBL_CFG, gponGlbCfg.Raw) ;

	gwan_remove_all_tcont() ;
	gwan_remove_all_gemport_for_disable() ;

    schedule_gpon_dev_reset(reset_type);
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

	/* register QDMA ISR callback function */
	ret = register_gpon_isr();
	if(ret != 0) {
		PON_MSG(MSG_ERR, "Register GPON ISR handler function failed.\n") ;
		return ret ;
	}

	/* restore some configuration before gpon enable */
	gponDevSetSerialNumber(gpGponPriv->gponCfg.sn) ;
	gponDevSetEncryptKey(gpGponPriv->gponCfg.key) ;

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
		/* This check is used to make sure register 0xbfaf02d4 ready */
		for(i = 0; i < retry; i++) {
			mdelay(1);
			psync_to_sof_delay_curr = IO_GMASK(0xbfaf02d4, 0xffff, 0);
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
	if(!(isFPGA || isEN751627) ) {
		adjust_mac_internal_delay_fine_tune();
	}
#endif /* TCSUPPORT_CPU_EN7521 */

	PON_MSG(MSG_INT, "GPON IRQ: gpon detect phy ready.\n") ;
	
	if(gpPonSysData->sysPrevLink == PON_LINK_STATUS_EPON) {
		xmcs_report_event(XMCS_EVENT_TYPE_EPON, XMCS_EVENT_EPON_MODE_CHANGE, 0);
	}

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
    
	if(GPON_CURR_STATE == GPON_STATE_O1) {	
		/* Change the current state */
		if(gpGponPriv->emergencystate) {
            gpon_act_change_gpon_state(GPON_STATE_O7) ;
			gpon_record_emergence_info(GPON_PHY_READY_REPORT_O7);
		} else {
            gpon_act_change_gpon_state(GPON_STATE_O2) ;
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


/*****************************************************************************
******************************************************************************/
void gpon_detect_los_lof(void)
{
	PON_MSG(MSG_INT, "GPON IRQ: gpon detect los lof.\n") ;
	PON_MSG(MSG_ERR, "one bit err cnt:0x%08x\n", IO_GREG(DBG_GEM_HEC_ONE_ERR_CNT)) ; 
	PON_MSG(MSG_ERR, "two bits err cnt:0x%08x\n", IO_GREG(DBG_GEM_HEC_TWO_ERR_CNT)) ; 
	PON_MSG(MSG_ERR, "uncorrectable err cnt:0x%08x\n", IO_GREG(DBG_GEM_HEC_UC_ERR_CNT)) ; 
  
    /*turn on LED*/
    if (GPON_CURR_STATE == GPON_STATE_O7) {
#if defined(TCSUPPORT_CT_PON_CN_CN)
					change_alarm_led_status(ALARM_LED_FLICKER);
#else
					change_alarm_led_status(ALARM_LED_ON);
#endif
    }

	if((GPON_STATE_O2 == GPON_CURR_STATE) || 
	   (GPON_STATE_O3 == GPON_CURR_STATE) || 
	   (GPON_STATE_O4 == GPON_CURR_STATE)) {
		gpPonSysData->sysLinkStatus = PON_LINK_STATUS_OFF ;
		gpPhyData->phy_link_status  = PHY_LINK_STATUS_LOS ;
		/* report GPON event message */
		xmcs_report_event(XMCS_EVENT_TYPE_GPON, XMCS_EVENT_GPON_LOS, 0) ;


		//xmcs_set_connection_start(PHY_ENABLE);
		/* Change the current state */
#ifdef TCSUPPORT_CPU_EN7521
		gpon_disable() ;
#else
		gpon_disable_with_option(GPON_DEV_RESET_WITH_FE_RESET);
#endif
	} else if( GPON_STATE_O5 == GPON_CURR_STATE) {
#if defined(TCSUPPORT_RA_HWNAT) && defined(TCSUPPORT_RA_HWNAT_ENHANCE_HOOK)
		extern int is_hwnat_dont_clean;
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
	if(flag > 1) 	
        return;
    
	REG_G_INT_ENABLE gponIntEnable;
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

/*****************************************************************************
******************************************************************************/
void gpon_ber_interval_expires(unsigned long arg)
{
	uint bip_cnt ;
	
	if(GPON_CURR_STATE == GPON_STATE_O5) {
		GPON_START_TIMER(gpGponPriv->ber_timer) ;
	
		bip_cnt = XPON_PHY_GET(PON_GET_PHY_BIP_COUNTER) ;
        gpGponPriv->bip_cnt_val += bip_cnt ;
		//Send a REI PLOAM message
		ploam_send_rei_msg(bip_cnt, &gpGponPriv->gponCfg.reiSeq) ;
	}
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
        PON_MSG(MSG_OAM, "WAN not start, ignore ploam msg:%8X\n", pPloamMsg->value[0]) ;
        return TRUE;
    }


    /* 3 consecutivly same ploam msg, only process once */
    is_same_msg = is_same_ploam_msg_as_previous(pPloamMsg);
    
    if(is_same_msg) same_cnt ++;

    if(is_same_msg && (same_cnt % 3) != 0 ){
	    PON_MSG(MSG_OAM, "Receive the PLOAM message same:%8X\n", pPloamMsg->value[0]) ;
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
void gpon_isr(void)
{
	REG_G_INT_STATUS intStatus ;
	REG_G_INT_ENABLE intEnable ;
	uint sec, nanosec, spfCnt;

	intStatus.Raw = IO_GREG(G_INT_STATUS) ;
	intEnable.Raw = IO_GREG(G_INT_ENABLE) ;

		//Clear the interrupt status
		intStatus.Raw &= intEnable.Raw ;
		IO_SREG(G_INT_STATUS, intStatus.Raw) ;
	
    spin_lock(&gpPonSysData->event_lock);
    	
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
				PON_MSG(MSG_INT, "GPON IRQ: SN_ONU send in O4 state interrupt.\n") ;
			}
			if(intStatus.Bits.sn_req_recv_int) {
				PON_MSG(MSG_INT, "GPON IRQ: SN_Request received interrupt.\n") ;
			}
			if(intStatus.Bits.sn_onu_send_o3_int) {
				PON_MSG(MSG_INT, "GPON IRQ: SN_ONU sent in O3 state interrupt.\n") ;
			}
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
					printk("Get the current TOD from GPON MAC. sec:%d, nanosec:%d, interval:%d\n", sec, nanosec, (uint)((sec-G_NEW_TOD_SEC_L32->Bits.new_tod_sec_l32)*1000000000 + (int)(nanosec-G_NEW_TOD_NANO_SEC->Bits.new_tod_nano_sec))) ;
				gpGponPriv->gponCfg.flags.isTodUpdate = 1 ;
				gponDevSet1PPSInt(XPON_ENABLE) ;
				PON_MSG(MSG_INT, "GPON IRQ: ToD updated done interrupt.\n") ;
			}
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
				PON_MSG(MSG_INT, "GPON IRQ: A PLOAM message received interrupt.\n") ;
				gpon_recv_ploam_message();
			}
		}
		if(intStatus.Raw & GPON_INT_ERROR) {
			if(intStatus.Bits.los_gem_del_int) {
				gpon_RDI_send();
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
				dbgtoMem(0, "GPON IRQ: FIFO error interrupt.\n") ;
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
				gpon_RDI_send();
				PON_MSG((MSG_INT|MSG_ERR), "GPON IRQ: Rx error interrupt.\n") ;
			}
            PON_MSG((MSG_INT|MSG_ERR), "DBG_GRP_0(0xbfb64228):0x%x\n", IO_GREG(DBG_GRP_0)) ;
            PON_MSG((MSG_INT|MSG_ERR), "DBG_GRP_1(0xbfb6422C):0x%x\n", IO_GREG(DBG_GRP_1)) ;
            PON_MSG((MSG_INT|MSG_ERR), "GEM_HEC_ONE_ERR_CNT(0xbfb64330):0x%x\n", IO_GREG(DBG_GEM_HEC_ONE_ERR_CNT)) ;
            PON_MSG((MSG_INT|MSG_ERR), "GEM_HEC_TWO_ERR_CNT(0xbfb64334):0x%x\n", IO_GREG(DBG_GEM_HEC_TWO_ERR_CNT)) ;
            PON_MSG((MSG_INT|MSG_ERR), "GEM_HEC_UC_ERR_CNT(0xbfb64338):0x%x\n", IO_GREG(DBG_GEM_HEC_UC_ERR_CNT)) ;
		}
#ifdef XPON_MAC_CONFIG_DEBUG
		if((gpPonSysData->debugLevel&MSG_DBG) && (intStatus.Raw&GPON_INT_ERROR)) {
#ifdef TCSUPPORT_CPU_EN7521
			PON_MSG(MSG_DBG, "[4220:%.8x],[4224:%.8x],[4228:%.8x],[422C:%.8x],[4240:%.8x]\n", DBG_BWM_FILTER_CTRL->Raw, DBG_BWM_SFIFO_STS->Raw, DBG_GRP_0->Raw, DBG_GRP_1->Raw, DBG_PROBE_CTRL->Raw) ;
			IO_SBITS(DBG_BWM_FILTER_CTRL, (0x07 << 29));
#else
			PON_MSG(MSG_DBG, "[4224:%.8x],[4228:%.8x],[422C:%.8x],[4240:%.8x]\n", DBG_BWM_FIFO_STS->Raw, DBG_GRP_0->Raw, DBG_GRP_1->Raw, DBG_PROBE_CTRL->Raw) ;
#endif
			PON_MSG(MSG_DBG, "[4300:%.8x],[4304:%.8x],[4308:%.8x],[430C:%.8x],[4310:%.8x]\n\n", DBG_RX_GEM_CNT->Raw, DBG_RX_CRC_ERR_CNT->Raw, DBG_RX_GTC_CNT->Raw, DBG_TX_GEM_CNT->Raw, DBG_TX_BST_CNT->Raw) ;
			DBG_GRP_0->Raw = 0xFFFFFFFF ;
			DBG_GRP_1->Raw = 0xFFFFFFFF ;
		}
#endif
	} else {
		PON_MSG(MSG_INT, "GPON IRQ: NULL GPON ISR, Status:%.8x, Mask:%.8x\n", intStatus.Raw, intEnable.Raw) ;
	}

    spin_unlock(&gpPonSysData->event_lock);
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
    GPON_STOP_TIMER(gpWanPriv->txDropTimer) ;    
    stop_omci_oam_monitor();
}




/***************************************************************
Copyright Statement:

This software/firmware and related documentation (EcoNet Software) 
are protected under relevant copyright laws. The information contained herein 
is confidential and proprietary to EcoNet (HK) Limited (EcoNet) and/or 
its licensors. Without the prior written permission of EcoNet and/or its licensors, 
any reproduction, modification, use or disclosure of EcoNet Software, and 
information contained herein, in whole or in part, shall be strictly prohibited.

EcoNet (HK) Limited  EcoNet. ALL RIGHTS RESERVED.

BY OPENING OR USING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY 
ACKNOWLEDGES AND AGREES THAT THE SOFTWARE/FIRMWARE AND ITS 
DOCUMENTATIONS (ECONET SOFTWARE) RECEIVED FROM ECONET 
AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON AN AS IS 
BASIS ONLY. ECONET EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, 
WHETHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, 
OR NON-INFRINGEMENT. NOR DOES ECONET PROVIDE ANY WARRANTY 
WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTIES WHICH 
MAY BE USED BY, INCORPORATED IN, OR SUPPLIED WITH THE ECONET SOFTWARE. 
RECEIVER AGREES TO LOOK ONLY TO SUCH THIRD PARTIES FOR ANY AND ALL 
WARRANTY CLAIMS RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES 
THAT IT IS RECEIVERS SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD 
PARTY ALL PROPER LICENSES CONTAINED IN ECONET SOFTWARE.

ECONET SHALL NOT BE RESPONSIBLE FOR ANY ECONET SOFTWARE RELEASES 
MADE TO RECEIVERS SPECIFICATION OR CONFORMING TO A PARTICULAR 
STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND 
ECONET'S ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE ECONET 
SOFTWARE RELEASED HEREUNDER SHALL BE, AT ECONET'S SOLE OPTION, TO 
REVISE OR REPLACE THE ECONET SOFTWARE AT ISSUE OR REFUND ANY SOFTWARE 
LICENSE FEES OR SERVICE CHARGES PAID BY RECEIVER TO ECONET FOR SUCH 
ECONET SOFTWARE.
***************************************************************/

/*
* File Name: gpon_init.c
* Description: Initialization for GPON
*
******************************************************************/
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/timer.h>

#include "common/drv_global.h"
#include "common/phy_if_wrapper.h"
#include "gpon/gpon_act.h"
#include "gpon/gpon_dev.h"
#include "gpon/gpon_qos.h"
#include "gpon/gpon_power_management.h"
#include "gpon/gpon_recovery.h"

#ifdef TCSUPPORT_CPU_EN7521
#include <ecnt_hook/ecnt_hook_fe.h>
#endif


Pgpon_mac_reg_REGS g_gpon_mac_reg_BASE = NULL ;
#if defined(TCSUPPORT_CPU_EN7526) || defined(TCSUPPORT_CPU_EN7527)
unsigned char RDI_SEND_ENABLE = 0;
unsigned int RDI_SEND_TIMES = 0x1500000; /*send for 0x1500000 times */
unsigned int RDI_SEND_DELAY = 500; /* check RDI status every 500 ms */
unsigned int RDI_WATCHDOG_KICK = 100000;
unsigned char RDI_TIMER_EN = 0;
struct timer_list rdi_timer ;
atomic_t rdi_sending = ATOMIC_INIT(0);
#endif

#define  PHY_TX_EN_BIT_LEN_CONST (20)
#define  GPON_DEFAULT_SD_THRESHOLD       9
#define  GPON_DEFAULT_SF_THRESHOLD       5


#ifdef CONFIG_USE_FOR_TEST
extern struct workqueue_struct * gem_valid_work_queue;
#endif /* CONFIG_USE_FOR_TEST */

extern GPON_Emergence_Info gEmergenceRecord[GPON_EMERGENCE_STATE_MAX_NUM];

extern void tc3162wdog_kick(void);
#if defined(TCSUPPORT_CPU_EN7526) || defined(TCSUPPORT_CPU_EN7527)
void gpon_software_rdi_send(void)
{
    if (1 == RDI_SEND_ENABLE && (!(isEN7580 || isEN7523 || isEN7581 || isAN7583 || isAN7552))){
        atomic_set(&rdi_sending,1);
    }
    return;
}

void gpon_RDI_config(TIMER_FUN_PAAM arg)
{
	uint data = 0;
	uint i = 0;

    if(1 == atomic_read(&rdi_sending)){
        atomic_set(&rdi_sending,0);
    	data = IO_GREG(DBG_SW_RDI_CTRL);
    	data = data| 0x1;
        PON_MSG(MSG_ERR, "Sending RDI for 0x%x times by timer.\n",RDI_SEND_TIMES) ;
    	for(i = 0; i <=RDI_SEND_TIMES ; i ++){
            if (!RDI_TIMER_EN){
                PON_MSG(MSG_ERR, "ONU is doing reset\n") ;
                break;
            }
    		IO_SREG(DBG_SW_RDI_CTRL,data);
            if(0 == (i % RDI_WATCHDOG_KICK)){
                tc3162wdog_kick();
            }
    	}
        PON_MSG(MSG_ERR, "Sending RDI end.\n") ;
    }
    if(RDI_SEND_ENABLE && RDI_TIMER_EN){
        GPON_START_TIMER(rdi_timer,RDI_SEND_DELAY);
    }
    return;
}
#endif
void gpon_SD_SF_init(void)
{    
    gGpon_SD_SF_Info.SD_thld = GPON_DEFAULT_SD_THRESHOLD;
    gGpon_SD_SF_Info.SF_thld = GPON_DEFAULT_SF_THRESHOLD;
    gGpon_SD_SF_Info.SD_cnt = 0;
    gGpon_SD_SF_Info.SF_cnt = 0;
}

/*****************************************************************************
******************************************************************************/
static int gpon_init_private_data(GPON_GlbPriv_T *gpon_priv_p) 
{
    int ret = 0 ;
    unchar passwd[GPON_PASSWD_LENS] = {0x47, 0x43, 0x30, 0x30, 0x30, 0x36, 0x39, 0x46, 0x36, 0x00} ;
    unchar sn[GPON_SN_LENS] = {0x4D, 0x54, 0x4B, 0x47, 0x00, 0x00, 0x00, 0x01} ;
	char cage[8] = "BoB";
	char moduleVendor[GPON_MODULE_VENDOR_LEN] = "HUMAX";
	char moduleName[GPON_MODULE_NAME_LEN] = "AA001";
	char moduleVersion[GPON_MODULE_NAME_LEN] = "v1.0";
	char moduleFwVersion[GPON_MODULE_FW_VERSION_LEN] = "v1.1.1";
	char connector[24] = "SC";
	char status[GPON_MODULE_STATUS_LEN] = "Up";

	gpon_priv_p->bip_cnt_val = 0;
    memset(gpon_priv_p->gponCfg.sn, 0, sizeof(gpon_priv_p->gponCfg.sn)) ;
    memset(gpon_priv_p->gponCfg.passwd, 0, sizeof(gpon_priv_p->gponCfg.passwd)) ;
    
    gpon_priv_p->disableSnFlag      =           0 ;
    gpon_priv_p->emergencystate     =           0 ;
	if(isEN7523 || isEN7581 || isAN7583 || isAN7552)
		gpon_priv_p->lofEnableFlag		 =		1 ;
	else
		gpon_priv_p->lofEnableFlag		 =		0 ;
    gpon_priv_p->gpon_traffic_status     =      TRAFFIC_DOWN ;
    
    gpon_priv_p->gponCfg.onu_id =                       GPON_UNASSIGN_ONU_ID ;
    gpon_priv_p->gponCfg.keyIdx =                       0 ;
    gpon_priv_p->gponCfg.omcc =                         GPON_UNASSIGN_GEM_ID ;
    gpon_priv_p->gponCfg.reiSeq =                       0 ;
    gpon_priv_p->gponCfg.to1Timer =                     GPON_ACT_TO1_TIMER ;
    gpon_priv_p->gponCfg.to2Timer =                     GPON_ACT_TO2_TIMER ;
	gpon_priv_p->gponCfg.hardware_timer=                GPON_HARDWARE_TIMER;
    gpon_priv_p->gponCfg.berInterval =                  0 ;
    gpon_priv_p->gponCfg.silenceInterval =              GPON_ACT_SILENCE_TIMER ;
    gpon_priv_p->gponCfg.eqd =                          0 ;
    gpon_priv_p->gponCfg.byteDelay =                    0 ;
    gpon_priv_p->gponCfg.bitDelay =                     0 ;
    gpon_priv_p->gponCfg.eqdO4Offset =                  0 ;
    gpon_priv_p->gponCfg.eqdO5Offset =                  0 ;
    gpon_priv_p->gponCfg.t3PreambleOffset =             0 ;
    gpon_priv_p->gponCfg.dvtPcpCheck =                  0 ;
    gpon_priv_p->gponCfg.dvtPcpCounter =                0 ;
    gpon_priv_p->gponCfg.dvtPcpCheckErr =               0 ;
    gpon_priv_p->gponCfg.counter_type   =               GPON_COUNTER_TYPE_GEM ;
    gpon_priv_p->gponCfg.flags.isRequestKey =           0 ;
    gpon_priv_p->gponCfg.flags.isTodUpdate =            0 ;
    gpon_priv_p->gponCfg.flags.preambleFlag =           GPON_BURST_MODE_OVERHEAD_LEN_DEFAULT ;
    gpon_priv_p->gponCfg.flags.hotplug =                XPON_DISABLE;
    if(isEN7581 || isAN7583){
      if(isFPGA){
        gpon_priv_p->gponCfg.internalDelayFineTune =	0x6E;
      }else{
        gpon_priv_p->gponCfg.internalDelayFineTune =	0x0D;
      }
    }else if(isEN7523 || isAN7552){
      if(isFPGA){
        gpon_priv_p->gponCfg.internalDelayFineTune =	0x29;
      }else{
        gpon_priv_p->gponCfg.internalDelayFineTune =	0x0D;
      }
    }else{
      gpon_priv_p->gponCfg.internalDelayFineTune =		0x1C;
    }
    gpon_priv_p->gponCfg.onuResponseTime =              0x577;     
    gpon_priv_p->gponCfg.tx_power      =                0;
	gpon_priv_p->gponCfg.preamble_t3_pat   =            0xaa;
    gpon_priv_p->gponCfg.phy_guard_bit_num = PHY_TX_EN_BIT_LEN_CONST;
	gpon_priv_p->gponCfg.consid_tx_rx_ph   =            0;
	gpon_priv_p->gponCfg.sw_reset_flag     =            0;
#ifdef TCSUPPORT_CUC
    gpon_priv_p->gponCfg.dis_ranging_in_o5 =1;
#else
    gpon_priv_p->gponCfg.dis_ranging_in_o5 =0;
#endif

#ifdef TCSUPPORT_CPU_EN7521
    gpon_priv_p->gponCfg.flags.isPloamFilter =          1;
    gpon_priv_p->gponCfg.flags.isBwmStopTimeErrInt =    XPON_DISABLE;
    gpon_priv_p->gponCfg.phy_psync_to_sof_delay =       UNKNOW_PSYNC_TO_SOF_DELAY;
	/* sniffer mode */
	gpon_priv_p->gponCfg.sniffer_mode.enable = XPON_DISABLE;
	gpon_priv_p->gponCfg.sniffer_mode.lan_port = SNIFFER_MODE_LAN0;
	gpon_priv_p->gponCfg.sniffer_mode.packet_padding = XPON_ENABLE;
    /* Sleep count */
    gpon_priv_p->gponCfg.sleep_count =      POWER_MANAGEMENT_DEFAULT_SLEEP_COUNT;
    
    spin_lock_init(&gpon_priv_p->swReset_lock) ;
#else
    gpon_priv_p->gponCfg.reg4208 = 0x80800F00;
    gpon_priv_p->gponCfg.rstDbgDly = MAC_4208_PHY_RX_FIX | MAC_4208_PHY_RX_DLY | 
                                     MAC_4208_INTERNAL_DLY | MAC_4208_PHY_TX_DLY;
    /* do Mac Reset and Phy whole reset */
    gpon_priv_p->gponCfg.macPhyReset = GPON_MAC_RESET_BIT | PHY_ANALOG_RESET_BIT;
#endif  

    /* idle gem threshold */
    gpon_priv_p->gponCfg.idle_gem_thld =    GPON_IDLE_GEM_THLD;
	gpon_priv_p->gponCfg.rdiSendNum	= GPON_HARDWARE_RDI_SEND_NUM;
	gpon_priv_p->gponCfg.rdiErrCtrlEn= GPON_RDI_ERROR_SEND_EN;
	gpon_priv_p->gponCfg.rdiErrCtrlThreshold= GPON_RDI_ERROR_SEND_THRESHOLD;

    spin_lock_init(&gpon_priv_p->act_lock) ;

    gpon_priv_p->typeBOnGoing = 0;

    /* Set ONU password */
    memcpy(gpon_priv_p->gponCfg.passwd, passwd, GPON_PASSWD_LENS) ;
    
    /* Set the serial number */
    memcpy(gpon_priv_p->gponCfg.sn, sn, GPON_SN_LENS) ;

    memset(&gponRecovery, 0, sizeof(struct Gpon_Recovery_S));
    memset(&gEmergenceRecord, 0, sizeof(GPON_Emergence_Info)*GPON_EMERGENCE_STATE_MAX_NUM);

	gpon_priv_p->pGponRecovery = &gponRecovery;
	gpon_recovery_init();
    gpon_init_dying_info();
    spin_lock_init(&gpon_priv_p->mac_reset_lock) ;   
	gpon_SD_SF_init();

	memset(gpon_priv_p->gponCfg.phyMedModule.cage, 0, sizeof(gpon_priv_p->gponCfg.phyMedModule.cage));
	memset(gpon_priv_p->gponCfg.phyMedModule.moduleVendor, 0, sizeof(gpon_priv_p->gponCfg.phyMedModule.moduleVendor));
	memset(gpon_priv_p->gponCfg.phyMedModule.moduleName, 0, sizeof(gpon_priv_p->gponCfg.phyMedModule.moduleName));
	memset(gpon_priv_p->gponCfg.phyMedModule.moduleVersion, 0, sizeof(gpon_priv_p->gponCfg.phyMedModule.moduleVersion));
	memset(gpon_priv_p->gponCfg.phyMedModule.moduleFwVersion, 0, sizeof(gpon_priv_p->gponCfg.phyMedModule.moduleFwVersion));
	memset(gpon_priv_p->gponCfg.phyMedModule.connector, 0, sizeof(gpon_priv_p->gponCfg.phyMedModule.connector));
	memset(gpon_priv_p->gponCfg.phyMedModule.status, 0, sizeof(gpon_priv_p->gponCfg.phyMedModule.status));

	memcpy(gpon_priv_p->gponCfg.phyMedModule.cage, cage, 8);
	memcpy(gpon_priv_p->gponCfg.phyMedModule.moduleVendor, moduleVendor, GPON_MODULE_VENDOR_LEN);
	memcpy(gpon_priv_p->gponCfg.phyMedModule.moduleName, moduleName, GPON_MODULE_NAME_LEN);
	memcpy(gpon_priv_p->gponCfg.phyMedModule.moduleVersion, moduleVersion, GPON_MODULE_VERSION_LEN);
	memcpy(gpon_priv_p->gponCfg.phyMedModule.moduleFwVersion, moduleFwVersion, GPON_MODULE_FW_VERSION_LEN);
	memcpy(gpon_priv_p->gponCfg.phyMedModule.connector, connector, 24);
	memcpy(gpon_priv_p->gponCfg.phyMedModule.status, status, GPON_MODULE_STATUS_LEN);

	gpon_priv_p->gponAlmKeepTime = JIFFIES_TIMEOUT_HZ_CNT;

    return ret ;
}

/*****************************************************************************
******************************************************************************/
int register_gpon_isr(void)
{
#ifdef TCSUPPORT_CPU_EN7521
	return QDMA_API_REGISTER_HOOKFUNC(ECNT_QDMA_WAN, QDMA_CALLBACK_GPON_MAC_HANDLER, gpon_isr);
#else
	return qdma_register_callback_function(QDMA_CALLBACK_GPON_MAC_HANDLER, gpon_isr) ;
#endif
}

int unregister_gpon_isr(void)
{
#ifdef TCSUPPORT_CPU_EN7521
	return QDMA_API_UNREGISTER_HOOKFUNC(ECNT_QDMA_WAN, QDMA_CALLBACK_GPON_MAC_HANDLER);
#else
	return qdma_unregister_callback_function(QDMA_CALLBACK_GPON_MAC_HANDLER) ;
#endif	
}


int gpon_init(void)
{
	int ret = 0 ;
#ifdef TCSUPPORT_CPU_ARMV8
#if 0
	UINT32 raw = 0;		
#endif
	/* Base Register of  GPON */
	g_gpon_mac_reg_BASE = 0x0;
#else
	g_gpon_mac_reg_BASE = (Pgpon_mac_reg_REGS)(ioremap_nocache(CONFIG_GPON_BASE_ADDR, CONFIG_GPON_REG_RANGE)) ; 
	if(!g_gpon_mac_reg_BASE) {
		PON_MSG(MSG_ERR, "ioremap the GPON base address failed.\n") ;
		return -ENOMEM ;
	}
#endif
	ret = gpon_init_private_data(gpGponPriv) ;
	if(ret != 0) {
		PON_MSG(MSG_ERR, "GPON private data initialization failed.\n") ;
		return ret ;
	}
	if(isEN7580 || isEN7581 || isAN7583){
		if(gpPhyData->working_mode == PHY_GPON_CONFIG)
			XPON_PHY_SET_MODE(PHY_GPON_CONFIG);
		else
			XPON_PHY_SET_MODE(PHY_GPON_SYM_CONFIG);
	}

	ret = ploam_init() ;
	if(ret) {
		PON_MSG(MSG_ERR, "GPON PLOAM initialization failed.\n") ;
		return ret ;
	}

	ret = gpon_act_init() ;
	if(ret) {
		PON_MSG(MSG_ERR, "GPON ONU activation initialization failed.\n") ;
		return ret ;
	}

#ifdef TCSUPPORT_CPU_ARMV8
#if 0
	raw = GET_CLK_PER_ICG_ENABLE();
	raw = raw | 0x100;
	SET_CLK_PER_ICG_ENABLE(raw);
	raw = GET_TOD_DIVIDER_ENABLE();
	raw = raw | 0x2;
	SET_TOD_DIVIDER_ENABLE(raw);
#endif
#else
	/* enable ToD clock  */
	IO_SBITS(0xbfa200e4, 0x100);
	IO_SBITS(0xbfa200ec, 0x02);
#endif

#if defined(TCSUPPORT_CPU_EN7526) || defined(TCSUPPORT_CPU_EN7527)
	GPON_CREATE_TIMER(&rdi_timer,gpon_RDI_config,RDI_SEND_DELAY);
#endif

    PON_MSG(MSG_TRACE, "GPON initialization done\n") ;

	return ret ;
}

/*****************************************************************************
******************************************************************************/
void gpon_deinit(void)
{
#if TCSUPPORT_CPU_EN7521
	xpon_power_management_deinit();
#endif

    gpon_stop_timer();

    /*Clear silence!*/
	gpGponPriv->gpon_silence = 0;
	GPON_STOP_TIMER(gpGponPriv->silence_timer) ;

	tasklet_kill(&gpGponPriv->gpon_dev_reset);
    tasklet_kill(&gpGponPriv->gpon_channel_retire);
#if !defined(TCSUPPORT_CPU_EN7523) && !defined(TCSUPPORT_CPU_EN7581)
    tasklet_kill(&gpGponPriv->gpon_check_rx_loss);
#endif
	if(g_gpon_mac_reg_BASE) {
		iounmap(g_gpon_mac_reg_BASE) ;
		g_gpon_mac_reg_BASE = NULL ;
	}
	gpon_act_deinit();
    gpon_INT_deinit() ;
	
#ifdef CONFIG_USE_FOR_TEST
	if(gem_valid_work_queue)
		destroy_workqueue(gem_valid_work_queue);
#endif /* CONFIG_USE_FOR_TEST */
}

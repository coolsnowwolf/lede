/*
* File Name: gpon_init.c
* Description: Initialization for GPON
*
******************************************************************/
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/timer.h>

#include <ecnt_hook/ecnt_hook_net.h>
#include "common/drv_global.h"
#include "gpon/gpon_act.h"
#include "gpon/gpon_dev.h"
#include "gpon/gpon_qos.h"
#include "gpon/gpon_power_management.h"
#include "gpon/gpon_recovery.h"

#ifdef TCSUPPORT_CPU_EN7521
#include <ecnt_hook/ecnt_hook_fe.h>
#endif


Pgpon_mac_reg_REGS g_gpon_mac_reg_BASE = NULL ;
unsigned char RDI_SEND_ENABLE = 0;
unsigned int RDI_SEND_TIMES = 0x70000;
unsigned int RDI_SEND_DELAY  = 0x5;
static struct tasklet_struct gTaskRDI;

static unsigned int RDIState = GPON_RDI_TASK_DISABLE;

#ifdef CONFIG_USE_FOR_TEST
extern struct workqueue_struct * gem_valid_work_queue;
#endif /* CONFIG_USE_FOR_TEST */

void gpon_RDI_send(void)
{
    if (1 == RDI_SEND_ENABLE){
        gpon_RDI_task_state_set(GPON_RDI_TASK_ENABLE);
		tasklet_schedule(&gTaskRDI);
	}
    return;
}

void gpon_RDI_config(void)
{
	uint data = 0;
	int i = 0;
	data = IO_GREG(DBG_SW_RDI_CTRL);
	data = data| 0x1;
    PON_MSG(MSG_ERR, "Sending RDI.\n") ;
	for(i = 0; i <=RDI_SEND_TIMES ; i ++){
        if(GPON_RDI_TASK_ENABLE != RDIState || RDI_SEND_ENABLE != 1){
            return ;
        }
		IO_SREG(DBG_SW_RDI_CTRL,data);
		udelay(RDI_SEND_DELAY);
	}
    return;
}

void gpon_RDI_task_state_set(unsigned int state)
{
    static char isInited = 0;
    if((RDIState == state) ||
        ((isInited == 1) && (GPON_RDI_TASK_INIT == state)))
        return ;

    RDIState = state;
    if (GPON_RDI_TASK_ENABLE == state)
    {
        tasklet_enable(&gTaskRDI);
        PON_MSG(MSG_ERR, "RDI enable.\n") ;
    }
    else if (GPON_RDI_TASK_DISABLE == state)
    {
        tasklet_disable(&gTaskRDI);
        PON_MSG(MSG_ERR, "RDI disable.\n") ;
    }
    else if (GPON_RDI_TASK_INIT == state)
    {
        tasklet_init(&gTaskRDI,gpon_RDI_config,0);
        isInited = 1;
        PON_MSG(MSG_ERR, "RDI init.\n") ;
    }
    else
    {
        tasklet_kill(&gTaskRDI);
        PON_MSG(MSG_ERR, "RDI kill.\n") ;
    }
        
    return;
}
    
   
/*****************************************************************************
******************************************************************************/
static int gpon_init_private_data(GPON_GlbPriv_T *gpon_priv_p) 
{
    int ret = 0 ;
    unchar passwd[GPON_PASSWD_LENS] = {0x47, 0x43, 0x30, 0x30, 0x30, 0x36, 0x39, 0x46, 0x36, 0x00} ;
    unchar sn[GPON_SN_LENS] = {0x4D, 0x54, 0x4B, 0x47, 0x00, 0x00, 0x00, 0x01} ;

	gpon_priv_p->bip_cnt_val = 0;
    memset(gpon_priv_p->gponCfg.sn, 0, sizeof(gpon_priv_p->gponCfg.sn)) ;
    memset(gpon_priv_p->gponCfg.passwd, 0, sizeof(gpon_priv_p->gponCfg.passwd)) ;
    
    gpon_priv_p->disableSnFlag      =           0 ;
    gpon_priv_p->emergencystate     =           0 ;
    
    gpon_priv_p->gponCfg.onu_id =                       GPON_UNASSIGN_ONU_ID ;
    gpon_priv_p->gponCfg.keyIdx =                       0 ;
    gpon_priv_p->gponCfg.omcc =                         GPON_UNASSIGN_GEM_ID ;
    gpon_priv_p->gponCfg.reiSeq =                       0 ;
    gpon_priv_p->gponCfg.to1Timer =                     GPON_ACT_TO1_TIMER ;
    gpon_priv_p->gponCfg.to2Timer =                     GPON_ACT_TO2_TIMER ;
    gpon_priv_p->gponCfg.berInterval =                  0 ;
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
    gpon_priv_p->gponCfg.flags.sniffer =                0 ;
	gpon_priv_p->gponCfg.flags.hotplug =                XPON_DISABLE;
    gpon_priv_p->gponCfg.internalDelayFineTune =        0x1C;
    gpon_priv_p->gponCfg.onuResponseTime =              0x58b; 
    gpon_priv_p->gponCfg.tx_power      =                0;
	gpon_priv_p->gponCfg.preamble_t3_pat   =            0xaa;
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

    spin_lock_init(&gpon_priv_p->act_lock) ;

    gpon_priv_p->typeBOnGoing = 0;

    /* Set ONU password */
    memcpy(gpon_priv_p->gponCfg.passwd, passwd, GPON_PASSWD_LENS) ;
    
    /* Set the serial number */
    memcpy(gpon_priv_p->gponCfg.sn, sn, GPON_SN_LENS) ;

	gpon_priv_p->pGponRecovery = &gponRecovery;
	gpon_recovery_init();
    gpon_init_dying_info();
	
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
    gpon_RDI_task_state_set(GPON_RDI_TASK_DISABLE);
#ifdef TCSUPPORT_CPU_EN7521
	return QDMA_API_UNREGISTER_HOOKFUNC(ECNT_QDMA_WAN, QDMA_CALLBACK_GPON_MAC_HANDLER);
#else
	return qdma_unregister_callback_function(QDMA_CALLBACK_GPON_MAC_HANDLER) ;
#endif
}


int gpon_init(void)
{
	int ret = 0 ;
	
	/* Base Register of  GPON */
	g_gpon_mac_reg_BASE = (Pgpon_mac_reg_REGS)(ioremap_nocache(CONFIG_GPON_BASE_ADDR, CONFIG_GPON_REG_RANGE)) ; 
	if(!g_gpon_mac_reg_BASE) {
		PON_MSG(MSG_ERR, "ioremap the GPON base address failed.\n") ;
		return -ENOMEM ;
	}
	ret = gpon_init_private_data(gpGponPriv) ;
	if(ret != 0) {
		PON_MSG(MSG_ERR, "GPON private data initialization failed.\n") ;
		return ret ;
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

	ret = gpon_dev_init() ;
	if(ret != 0) {
		PON_MSG(MSG_ERR, "GPON device initialization failed.\n") ;
		return ret ;
	}

	/* stop MBI interface */
	gponDevMbiStop(XPON_ENABLE) ;

#ifdef TCSUPPORT_CPU_EN7521
	ret = gpon_power_management_init() ;
	if(ret) {
		PON_MSG(MSG_ERR, "GPON power management initialization failed.\n") ;
		return ret ;
	}
	/* Set internalDelayFineTune */
	gponDevSetInternalDelayFineTune(gpGponPriv->gponCfg.internalDelayFineTune);
	/* disable sniff mode */
	gponDevSetSniffMode(&gpGponPriv->gponCfg.sniffer_mode);
	/* DBA backdoor */
	gponDevSetDbaBackdoor(XPON_DISABLE);
	/* DBA slight modify */
	gponDevSetDbaSlightModify(XPON_DISABLE);

	/* enable ToD clock  */
	IO_SBITS(0xbfa200e4, 0x100);
	IO_SBITS(0xbfa200ec, 0x02);
#endif
    gpon_RDI_task_state_set(GPON_RDI_TASK_INIT);

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
	/* Stop the TO1 or TO2 timer */
	GPON_STOP_TIMER(gpGponPriv->to1_timer) ;
	GPON_STOP_TIMER(gpGponPriv->to2_timer) ;

	GPON_STOP_TIMER(gpGponPriv->ber_timer) ;

	if(g_gpon_mac_reg_BASE) {
		iounmap(g_gpon_mac_reg_BASE) ;
		g_gpon_mac_reg_BASE = NULL ;
	}
	gpon_act_deinit();
	
#ifdef CONFIG_USE_FOR_TEST
	if(gem_valid_work_queue)
		destroy_workqueue(gem_valid_work_queue);
#endif /* CONFIG_USE_FOR_TEST */

    gpon_RDI_task_state_set(GPON_RDI_TASK_KILL);
}

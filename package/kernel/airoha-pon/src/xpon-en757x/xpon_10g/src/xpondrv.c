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
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/workqueue.h>
#include <linux/kthread.h>
#include <ecnt_hook/ecnt_hook_pon_mac.h>
#include <ecnt_hook/ecnt_hook_qdma.h>
#include <ecnt_hook/ecnt_hook_pon_phy.h>
#include <xmcs/xmcs_const.h>
#include "common/xpon_global.h"
#include "gpon/gpon.h"
#include "gpon/gpon_init.h"
#include "common/phy_if_wrapper.h"
#include "common/xpon_led.h"
#include "tcversion.h"
#include "common/xpon_api.h"
#include "common/xpon_daemon.h"
#include "epon/epon_act.h"
#include <ecnt_hook/ecnt_hook_dgasp.h>
#include <ecnt_hook/ecnt_hook_lddla.h>



/************************************************************************
*                  D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/

extern int xpon_proc_dest(void);
extern int xpon_proc_init(void);
extern int eponGetTrafficStatus(void);

int init_status = 1;
static int xpon_mac_hook_dispatch(struct ecnt_data  * in_data);
struct ecnt_hook_ops xpondrv_hook_dispatch_ops = {
	.name = "xpon_mac_hook_dispatch",
	.hookfn = xpon_mac_hook_dispatch,
	.is_execute = 1,
	.maintype = ECNT_XPON_MAC,
	.subtype = ECNT_XPON_MAC_HOOK,
};
/************************************************************************
*                  M A C R O S
*************************************************************************
*/

/************************************************************************
*                  D A T A   T Y P E S
*************************************************************************
*/

/************************************************************************
*                  E X T E R N A L   D A T A   D E C L A R A T I O N S
*************************************************************************
*/
#ifdef TCSUPPORT_AUTOBENCH
extern int gpon_slt_test;
extern int epon_slt_test;
#endif   
extern int xmcs_set_connection_start(XPON_Mode_t mode);
extern int xmcs_set_link_detection(XMCSIF_WanDetectionMode_t detection);

extern struct list_head fix_reg_list;

int auto_mode_flag = 0;
int phyNoLosNoReadyCnt = 0;
int phyNoLosNoReadyHandleCycle = 5;

#ifdef TCSUPPORT_CPU_ARMV8_64
extern char get_onutype(void);
extern struct device* get_xpon_dev(void);
extern int get_xpon_irq(int index);
extern u32 GET_DMTC(void);
extern void SET_DMTC(u32 val);
#endif

/************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
*************************************************************************
*/
/************************************************************************
*                  P U B L I C   D A T A
*************************************************************************
*/
PON_SysData_T	 * gpPonSysData = NULL;
PWAN_GlbPriv_T   * gpWanPriv    = NULL;
MCS_GlbPriv_T   * gpMcsPriv    = NULL;
GPON_GlbPriv_T	 * gpGponPriv   = NULL;
EPON_GlbPriv_T    * gpEponPriv    = NULL;
PHY_FecCount_T gphyFecCount;
int mode = -1;
	
module_param(mode, int, 0);
/************************************************************************
*                  P R I V A T E   D A T A
*************************************************************************
*/

/************************************************************************
*                  F U N C T I O N   D E F I N I T I O N S
*************************************************************************
*/
/*______________________________________________________________________________
**	function name
**		xpon_dying_gasp_interrupt
**	description:
**		E-PON/XE-PON Dying Gasp interrupt handler. 
**	parameters:
**		irq: irq number
**		dev_id: device ID
**	global:
**		gpPonSysData
**	return:
**		constant IRQ_HANDLED
**	call:
**		xmit_dying_gasp_oam
**	revision:
**		v1.0
**____________________________________________________________________________*/
irqreturn_t xpon_dying_gasp_interrupt(int irq, void *dev_id)
{
#ifdef TCSUPPORT_CPU_ARMV8
	u32 raw = 0;	
    DGASP_OPTIMIZE_PON();
	
	/* Clear pre dying gasp interrupt status */
	raw = GET_DMTC();	
	raw = raw | SCU_DYING_GASP_STATUS;	
	SET_DMTC(raw);
#else
	/* Clear pre dying gasp interrupt status */
	IO_SBITS(0xBFB00084, SCU_DYING_GASP_STATUS) ;
#endif

	if (gpPonSysData->sysLinkStatus == PON_LINK_STATUS_EPON) {	
		if (!gpPonSysData->dyingGaspData.isEponHwFlag) {
			xmit_dying_gasp_oam();
		}
	}
#ifdef TCSUPPORT_CPU_ARMV8
	DGASP_OPTIMIZE_PON();
#endif
	return IRQ_HANDLED;
}
/*______________________________________________________________________________
**	function name
**		xpon_dying_gasp_init
**	description:
**		E-PON/XE-PON Dying Gasp interrupt handler init function.
**	parameters:
**		None
**	global:
**	return:
**		-ENODEV : Register Dying gasp irq fail.
**		-0 : Register Dying gasp irq scuess.
**	call:
**		request_irq
**	revision:
**		v1.0
**____________________________________________________________________________*/
static int xpon_dying_gasp_init(void)
{
	 /* Register Dying Gasp interrupt */
#ifdef TCSUPPORT_CPU_ARMV8_64
	 int irq = -1;
	 u32 raw = 0;
	 struct device* dev = NULL;
     dev = get_xpon_dev();
	 irq = get_xpon_irq(1);
	 
	 /* Clear pre dying gasp interrupt status */
	 raw = GET_DMTC();
	 raw = raw | SCU_DYING_GASP_STATUS;
	 SET_DMTC(raw);
	 
	 if(request_irq(irq, xpon_dying_gasp_interrupt, 0, "dying gasp", dev) != 0)
	 {
		 printk("Request the interrupt service routine fail, irq:%d.\n", irq) ;
		 return -1 ;
	 }
#else
	 /* Clear pre dying gasp interrupt status */
	IO_SBITS(0xBFB00084, SCU_DYING_GASP_STATUS) ; 

	if(request_irq(DYINGGASP_INT, xpon_dying_gasp_interrupt, 0, "dying gasp", NULL) != 0)
	{
		printk("Request the interrupt service routine fail, irq:%d.\n", DYINGGASP_INT) ;
		return -ENODEV ;
	}
#endif
	
	return 0;
}
/*______________________________________________________________________________
**	function name
**		trans_status_refresh
**	description:
**		refresh to get phy trans status params
**	parameters:
**		None
**	global:
**		gpPhyData
**	return:
**		None
**	call:
**	revision:
**		v1.0
**____________________________________________________________________________*/

static void trans_status_refresh(TIMER_FUN_PAAM arg)
{
	XPON_DAEMON_Job_data_t job = {0};
	
    if(!gpPhyData->trans_status_refresh_pending ){
        job.id       = XPON_DAEMON_JOB_GET_PHY_PARAM ;
        job.priority = XPON_DAEMON_JOB_PRIORITY_LOW  ;
        gpPhyData->trans_status_refresh_pending = 1;
        xpon_daemon_job_enqueue(&job) ;
        wake_up_xpon_daemon() ;
	}
	XPON_START_TIMER(gpPhyData->trans_status_refresh_timer,1000) ;/* 1,000 ms */
}

void traffic_status_refresh(TIMER_FUN_PAAM arg)
{
    static ENUM_PhyTrafficStatus last_status = TRAFFIC_STATUS_DOWN;
    ENUM_PhyTrafficStatus status = TRAFFIC_STATUS_DOWN;
    static uint8 is_first = 1;
    
    if (gpPonSysData->sysPonMode == XMCS_IF_WAN_DETECT_MODE_TURBO_EPON || gpPonSysData->sysPonMode == XMCS_IF_WAN_DETECT_MODE_1G_1G_EPON){
        status = ( 1 == eponGetTrafficStatus() )? TRAFFIC_STATUS_UP: TRAFFIC_STATUS_DOWN;
    }else {
        status = TRAFFIC_STATUS_DOWN;
    }

    if(last_status != status || is_first){
        last_status = status;
        if (gpPonSysData->sysPonMode == XMCS_IF_WAN_DETECT_MODE_TURBO_EPON || gpPonSysData->sysPonMode == XMCS_IF_WAN_DETECT_MODE_1G_1G_EPON){  
                XPON_PHY_SET_API(PON_SET_PHY_TRAFFIC_STATUS, &status);
		LDDLA_SET_API(LDDLA_SET_PHY_TRAFFIC_STATUS, &status);
        }
    }

    if(is_first)
        is_first = 0;

    if (PON_MAC_START == gpPonSysData->sysMACStartup){
        XPON_START_TIMER(gpPhyData->traffic_status_refresh_timer,1000);/* 1,000 ms */
	}
    return;
}


/*______________________________________________________________________________
**	function name
**		xpondrv_init_phy_data
**	description:
**		Global gpPhyData initalization
**	parameters:
**		None
**	global:
**		gpPhyData
**	return:
**		None
**	call:
**	revision:
**		v1.0
**____________________________________________________________________________*/
static void xpondrv_init_phy_data(void)
{
    gpPhyData->is_fix_mode   = FALSE;
    gpPhyData->calibrating   = FALSE;
    gpPhyData->working_mode  = PHY_UNKNOWN_CONFIG;
    gpPhyData->detect_mode   = PHY_UNKNOWN_CONFIG;
    gpPhyData->detect_status = XPON_PHY_SYNC_STATUS_NOT_START;
    gpPhyData->phy_link_status     = PHY_LINK_STATUS_UNKNOWN;

    spin_lock_init(&gpPhyData->trans_params_lock);
    memset(&gpPhyData->trans_params, 0 , sizeof(gpPhyData->trans_params));
	memset(&gpPhyData->trans_info, 0 , sizeof(gpPhyData->trans_info));
	GPON_CREATE_TIMER(&gpPhyData->trans_status_refresh_timer,trans_status_refresh,1000);/* 1,000 ms */
	GPON_CREATE_TIMER(&gpPhyData->traffic_status_refresh_timer,traffic_status_refresh,1000);/* 1,000 ms */
	xpon_get_transceiver_info(&gpPhyData->trans_info);

	memset(&gphyFecCount, 0, sizeof(PHY_FecCount_T));
}
/*______________________________________________________________________________
**	function name
**		xpondrv_init_global_data
**	description:
**		Global variables initalization.
**	parameters:
**		None.
**	global:
**		gpPonSysData.
**		gpEponPriv.
**		gpGponPriv.
**		gpWanPriv.
**		gpPhyData.
**	return:
**		None.
**	call:
**		xpondrv_init_phy_data
**	revision:
**		v1.0
**____________________________________________________________________________*/
static void xpondrv_init_global_data(void)
{	
	gpPonSysData->sysLinkStatus = PON_LINK_STATUS_OFF;
	gpPonSysData->sysPrevLink   = PON_LINK_STATUS_OFF;
    gpPonSysData->sysStartup    = PON_WAN_STOP ;

	gpEponPriv = &gpPonSysData->eponPrivData;
	gpGponPriv = &gpPonSysData->gponPrivData;

	gpWanPriv = &gpPonSysData->wanPrivData;
	gpMcsPriv = &gpPonSysData->xmcsPrivData;
    gpPhyData =  &gpPonSysData->phyCfg;

	gpPonSysData->debugLevel = 0 ;
	
    UNION_IC_FUNCTION_HOOK(ONLINE_ACTION_PON_GET_ONU_MODE_TYPE,NULL,NULL);
	if(mode != -1){
		gpPonSysData->sysPonMode = mode;
	}

	if(gpPonSysData->sysOnuType != PON_ONU_TYPE_HGU)
		gpPonSysData->sysOnuType  = PON_ONU_TYPE_SFU;
	else
	    gpPonSysData->sysOnuType = PON_ONU_TYPE_HGU;    

	gpPonSysData->dyingGaspData.isEponHwFlag = 1;
    gpPonSysData->rogue_state = XPON_ROUGE_STATE_FALSE;
    xpondrv_init_phy_data();
	
	init_waitqueue_head(&gpPonSysData->xpon_daemon.wq) ;
    memset(&gpPonSysData->xpon_daemon.job_queue, 0, sizeof(gpPonSysData->xpon_daemon.job_queue));
    spin_lock_init(&gpPonSysData->xpon_daemon.job_queue.lock);    
	gpPonSysData->xpon_daemon.task= kthread_run(xpon_daemon, NULL, "xpon_daemon");
	if(IS_ERR(gpPonSysData->xpon_daemon.task))
	{		
		printk("@%s>>%d--xpon_daemon init failed\n", __FUNCTION__, __LINE__);
	}
}
/*______________________________________________________________________________
**	function name
**		xpondrv_qdma_init
**	description:
**		WAN QDMA API & callback funciton initalization
**	parameters:
**		None.
**	global:
**		None.	
**	return:
**		None.
**	call:
**		xpon_set_qdma_qos
**	revision:
**		v1.0
**____________________________________________________________________________*/
static void xpondrv_qdma_init(void)
{
	QDMA_InitCfg_t qdmaInitCfg;
		
	memset(&qdmaInitCfg, 0x0, sizeof(QDMA_InitCfg_t));
	
	qdmaInitCfg.cbRecvPkts = pwan_cb_rx_packet;
	qdmaInitCfg.cbEventHandler = pwan_cb_event_handler;
	
	QDMA_API_INIT(ECNT_QDMA_WAN, &qdmaInitCfg);
	
	/*if rmmod the xpon_10g,the QDMA WAN RX INT will be disabled,so we should enable it in case*/
	QDMA_API_ENABLE_RXPKT_INT(ECNT_QDMA_WAN) ;
	QDMA_API_TX_DMA_MODE(ECNT_QDMA_WAN, QDMA_ENABLE);
	QDMA_API_RX_DMA_MODE(ECNT_QDMA_WAN, QDMA_ENABLE);
	QDMA_API_LOOPBACK_MODE(ECNT_QDMA_WAN, QDMA_LOOPBACK_DISABLE) ;
	
}

static void xpondrv_qdma_deinit(void)
{
	QDMA_InitCfg_t qdmaInitCfg;
	/*when rmmod the xpon_10g,we should disable QDMA WAN RX INT*/
	QDMA_API_DISABLE_RXPKT_INT(ECNT_QDMA_WAN) ;
	
	memset(&qdmaInitCfg, 0x0, sizeof(QDMA_InitCfg_t));
	QDMA_API_INIT(ECNT_QDMA_WAN, &qdmaInitCfg);
}

/*______________________________________________________________________________
**	function name
**		should_ignore_phy_los
**	description:
**		Bypass the phy los init.
**	parameters:
**		None.
**	global:
**		gpPhyData.
**	return:
**		.
**	call:
**	revision:
**		v1.0
**____________________________________________________________________________*/
static inline int should_ignore_phy_los(void)
{
    return ((PHY_LINK_STATUS_READY != gpPhyData->phy_link_status )  \
		|| (PHY_UNKNOWN_CONFIG == gpPhyData->working_mode) );
}
/*______________________________________________________________________________
**	function name
**		should_ignore_phy_ready
**	description:
**		Bypass the phy ready init.
**	parameters:
**		None.
**	global:
**		gpPhyData.
**	return:
**		.
**	call:
**	revision:
**		v1.0
**____________________________________________________________________________*/
static inline int should_ignore_phy_ready(void)
{
    return (PHY_LINK_STATUS_READY == gpPhyData->phy_link_status);
}

/*______________________________________________________________________________
**	function name
**		xpon_phy_los_handler
**	description:
**		xPON MAC for PHY LOS Event Handler.
**	parameters:
**		src: G/XG/XGS PON PHY event source.	
**		id:
**	global:
**		gpPonSysData	
**		gpPhyData	
**	return:
**		None.
**	call:
**		XPON_DPRINT_MSG
**		should_ignore_phy_los
**		gpon_phy_loss_handler
**		epon_msg_route_dispatch
**	revision:
**		v1.0
**____________________________________________________________________________*/
static void xpon_phy_los_handler(PHY_Event_Source_t src, PHY_Event_Type_t id)
{
   	XPON_DPRINT_MSG("id:%d\n", id);
    if(should_ignore_phy_los()){
        XPON_DPRINT_MSG("[%s] ignore phy los!\n", __FUNCTION__);
        return;
    }
	gpPonSysData->ponPhyStaus = PON_PHY_LOS;

    switch(gpPhyData->working_mode)
    {
        case PHY_GPON_CONFIG:
            gpon_phy_loss_handler(src);
            break;

        case PHY_EPON_CONFIG:
			epon_msg_route_dispatch(EPON_MSG_DETECT_LOS,NULL);
            break;

        default:
            dump_stack();
            printk("Illegal value of gpPhyData->working_mode: %d! %s:%d\n", \
                gpPhyData->working_mode, __FUNCTION__, __LINE__);
            break;
    }
}
/*______________________________________________________________________________
**	function name
**		xpon_phy_ready_handler
**	description:
**		xPON MAC for PHY READY Event Handler.
**	parameters:
**		src: G/XG/XGS PON PHY event source.	
**	global:
**		gpPonSysData	
**		gpPhyData	
**	return:
**		None.
**	call:
**		should_ignore_phy_ready
**		XPON_DPRINT_MSG
**		XPON_PHY_GET
**		gpon_phy_ready_handler
**		epon_msg_route_dispatch
**	revision:
**		v1.0
**____________________________________________________________________________*/
static void xpon_phy_ready_handler(PHY_Event_Source_t src)
{
    uint   job_done = FALSE;

    if(should_ignore_phy_ready() && !init_status){
        XPON_DPRINT_MSG("[%s] ignore phy ready!\n", __FUNCTION__);
        return;
    }
    if((gpPonSysData->sysPonMode == XMCS_IF_WAN_DETECT_MODE_10G_1G_EPON)  \
       || (gpPonSysData->sysPonMode == XMCS_IF_WAN_DETECT_MODE_1G_1G_EPON) \
       || (gpPonSysData->sysPonMode == XMCS_IF_WAN_DETECT_MODE_10G_10G_EPON) \
       || (gpPonSysData->sysPonMode == XMCS_IF_WAN_DETECT_MODE_EPON) \
       || (gpPonSysData->sysPonMode == XMCS_IF_WAN_DETECT_MODE_TURBO_EPON)
    ){
		CALL_USER_HOOK_PHY_EVENT_NOTIFY(XMCS_EVENT_TYPE_EPON, XMCS_EVENT_EPON_PHY_READY);	
    }else{
		CALL_USER_HOOK_PHY_EVENT_NOTIFY(XMCS_EVENT_TYPE_GPON, XMCS_EVENT_GPON_PHY_READY);
    }
    gpPonSysData->ponPhyStaus = PON_PHY_RDY;

	do{
        if(PHY_UNKNOWN_CONFIG == gpPhyData->working_mode ){
            gpPhyData->detect_status = XPON_PHY_SYNC_STATUS_SYNCED;
            gpPhyData->working_mode  = XPON_PHY_GET(PON_GET_PHY_MODE);
            continue;
        }

        gpPhyData->phy_link_status = PHY_LINK_STATUS_READY ;
		init_status = 0 ;
        switch(gpPhyData->working_mode)
        {
            case PHY_GPON_CONFIG:
                gpon_phy_ready_handler(src);
                job_done = TRUE;
                break;

            case PHY_EPON_CONFIG:
                epon_msg_route_dispatch(EPON_MSG_DETECT_READY,NULL);
                job_done = TRUE;
                break;
                
            default:
                dump_stack();
                printk("Illegal value of gpPhyData->working_mode: %d! %s:%d\n", 
                    gpPhyData->working_mode, __FUNCTION__, __LINE__);
                break;
        }
    } while(!job_done) ;

}
/*______________________________________________________________________________
**	function name
**		xpon_phy_start_rogue_handle
**	description:
**		Set XPON MAC act as rogue onu.
**	parameters:
**		None.
**	global:
**		gpPonSysData
**	return:
**		None.
**	call:
**		None.
**	revision:
**		v1.0
**____________________________________________________________________________*/
void xpon_phy_start_rogue_handle(void)
{
    gpPonSysData->rogue_state = XPON_ROUGE_STATE_TRUE;
}
/*______________________________________________________________________________
**	function name
**		xpon_phy_stop_rogue_handle
**	description:
**		Disable the rogue onu mode. 	
**	parameters:
**		None.
**	global:
**		gpGponPriv
**		gpPonSysData
**	return:
**		None.
**	call:
**		gpon_disable
**		XPON_PHY_TX_ENABLE
**		xmcs_report_event
**	revision:
**		v1.0
**____________________________________________________________________________*/
void xpon_phy_stop_rogue_handle(void)
{
    if(GPON_10G_STATE_O7  == GPON_CURR_STATE){
		gpGponPriv->emergencyState = 0;
		gpon_disable(GPON_MAC_PLAIN_RESET);
		XPON_PHY_TX_ENABLE();
		xmcs_report_event(XMCS_EVENT_TYPE_GPON, XMCS_EVENT_GPON_ENABLE, 0) ;
    }        
    gpPonSysData->rogue_state = XPON_ROUGE_STATE_FALSE ;
}
static void xpon_tx_power_handle(unchar opt)
{
	__u8 llidIndex = 0;
	if((PHY_EPON_CONFIG == gpPhyData->working_mode)
	&&(opt == XPON_PHY_TX_POWER_OFF))
	{
		for(llidIndex = 0; llidIndex < EPON_LLID_MAX_NUM; llidIndex++){
		epon_mpcp_local_deregister(llidIndex);
		}
	}
	CALL_USER_HOOK_FH_TX_POWER_HANDLE(opt);
	return;
}
static void xpon_phy_rongue_onu_handle(void)
{
	XPON_PHY_TX_DISABLE();
	gpPonSysData->ponRogueStatus = PON_STATUS_ROGUE;
	printk("Rogue ONU was detected!\n");
	return;
}

static int no_los_no_ready_counter(int phyEvent)
{
	if(phyEvent != PHY_EVENT_NO_LOS_NO_READY) {
		phyNoLosNoReadyCnt = 0;
		return NO_XPON_NO_LOS_NO_READY;
	}

	phyNoLosNoReadyCnt++;
	if(phyNoLosNoReadyCnt%phyNoLosNoReadyHandleCycle == 0) {
		phyNoLosNoReadyCnt = 0;
		return XPON_NO_LOS_NO_READY_COUNT;
	}

	return XPON_NO_LOS_NO_READY_STILL;
}

static void xpon_no_los_no_ready_handle(void)
{
	if(gpPhyData->trans_params.rx_power <= AUTOMODE_LOW_TXPOWER_THRESHOLD){
		XPON_DPRINT_MSG("=== low rx power threshold exceed, will not do auto mode switch ===\n");
		return;
	}

	if(auto_mode_flag == AUTOMODE_COMBO_OFF) {
		XPON_DPRINT_MSG("=== auto_mode_flag is OFF ===\n");
		return;
	}

	if(no_los_no_ready_counter(PHY_EVENT_NO_LOS_NO_READY) != XPON_NO_LOS_NO_READY_COUNT) {
		XPON_DPRINT_MSG("=== PHY_EVENT_NO_LOS_NO_READY not enough ===\n");
		return;
	}
	
	switch (auto_mode_flag)
	{
		case AUTOMODE_XEPON1G_TURBOEPON:
			if(gpPonSysData->sysPonMode == XMCS_IF_WAN_DETECT_MODE_1G_1G_EPON) {
				xmcs_report_event(XMCS_EVENT_TYPE_EPON, XMCS_EVENT_CHANGE_TO_TURBO_EPON, 0);
				XPON_DPRINT_MSG("[%s:%d] === Auto EPON ===\n", __FUNCTION__, __LINE__);
			} else if(gpPonSysData->sysPonMode == XMCS_IF_WAN_DETECT_MODE_TURBO_EPON) {
				xmcs_report_event(XMCS_EVENT_TYPE_EPON, XMCS_EVENT_CHANGE_TO_1G_1G_EPON, 0);
				XPON_DPRINT_MSG("[%s:%d] === Auto EPON ===\n", __FUNCTION__, __LINE__);
			} else {
				XPON_DPRINT_MSG("[%s:%d] === UnKnown Mode, do nothing ===\n", __FUNCTION__, __LINE__);
			}
			break;
		default:
			XPON_DPRINT_MSG("[%s:%d] === UnKnown auto_mode_flag, do nothing ===\n", __FUNCTION__, __LINE__);
			break;
	}
}

/*______________________________________________________________________________
**	function name
**		xpon_phy_event_dispatch
**	description:
**		Handle XPON PHY Event to dispatch to mac driver.
**	parameters:
**		pEvent: According the phy report event id to handle mac driver. 
**	global:
**		gpPonSysData
**		gpPhyData
**	return:
**		None.
**	call:
**		xmcs_set_connection_start
**		XPON_DPRINT_MSG
**		xpon_phy_los_handler
**		xpon_phy_ready_handler
**		xpon_phy_start_rogue_handle
**		xpon_phy_stop_rogue_handle
**	revision:
**		v1.0
**____________________________________________________________________________*/
static void xpon_phy_event_dispatch(PON_PHY_Event_data_t * pEvent)
{  

	if(PON_WAN_START != gpPonSysData->sysStartup){
        if((TRUE == gpPhyData->calibrating)  && (PHY_EVENT_CALIBRATION_STOP == pEvent->id)){
            gpPhyData->calibrating = FALSE;
            xmcs_set_connection_start(XPON_ENABLE);
        }
		else{
            XPON_DPRINT_MSG("PON WAN STOP! Ignore phy event:%d!\n", pEvent->id);
        }
    }
	else{
        XPON_DPRINT_MSG("Phy event:%d!\n", pEvent->id);
    }
    
    switch (pEvent->id)
    {
        case PHY_EVENT_TRANS_LOS_INT :
        case PHY_EVENT_PHY_ILLG_INT       :
        case PHY_EVENT_TRANS_LOS_ILLG_INT :
			gpon_set_alarmBit(LOS_INDEX);
			xpon_phy_los_handler(pEvent->src, pEvent->id);
            break;
		case PHY_EVENT_PHY_LOF_INT :
			gpon_set_alarmBit(LOF_INDEX);
			xpon_phy_los_handler(pEvent->src, pEvent->id);
            break;

        case PHY_EVENT_PHYRDY_INT:
			gpon_clear_alarmBit(LOS_INDEX);
			gpon_clear_alarmBit(LOF_INDEX);
			if(auto_mode_flag == AUTOMODE_XEPON1G_TURBOEPON)
				no_los_no_ready_counter(PHY_EVENT_PHYRDY_INT);
            xpon_phy_ready_handler(pEvent->src);
            break;
		case PHY_EVENT_NO_LOS_NO_READY:
			xpon_no_los_no_ready_handle();
			break;
		case PHY_EVENT_START_ROGUE_MODE:
            xpon_phy_start_rogue_handle();
			gpon_set_alarmBit(ROGUE_INDEX);
            break;

        case PHY_EVENT_STOP_ROGUE_MODE:
            xpon_phy_stop_rogue_handle();
            break;	
		
        case PHY_EVENT_CALIBRATION_START:
            gpPhyData->calibrating = TRUE;
            xmcs_set_connection_start(XPON_DISABLE);
            break;
            
        case PHY_EVENT_TX_POWER_ON:
            xpon_tx_power_handle(XPON_PHY_TX_POWER_ON);
            break;
			
        case PHY_EVENT_TX_POWER_OFF:
            xpon_tx_power_handle(XPON_PHY_TX_POWER_OFF);
            break;
        case PHY_EVENT_TF_INT:     
            xpon_phy_rongue_onu_handle();
            break;
		case PHY_EVENT_TRANS_INT:
        case PHY_EVENT_TRANS_SD_FAIL_INT:
        case PHY_EVENT_I2CM_INT:
			break;
        default:
            printk("Event %d handler not implemented! %s:%d\n", pEvent->id, __FUNCTION__, __LINE__);
            break;
    }
}
static int get_tcont_from_gemportid(uint gemport_id)
{
	uint gemIdx ;
	int channelId = -1;

	gemIdx = (gpWanPriv->gpon.gemIdToIndex[gemport_id] & GPON_GEM_IDX_MASK);

	if(gemIdx>=GPON_GEMPORT_MAX_NUM || !gpWanPriv->gpon.gemPort[gemIdx].info.valid) {
		return -1 ;
	}

	channelId = gpWanPriv->gpon.gemPort[gemIdx].info.channel;

	if(channelId == GPON_MULTICAST_CHANNEL)
	    return -1;

	return channelId;
	
}

#if defined (TCSUPPORT_AUTOBENCH)
int do_xpon_10g_slt_action(int mode)
{
	int delay_cnt = 0;

	if(mode == XMCS_IF_WAN_DETECT_MODE_XGPON || mode == XMCS_IF_WAN_DETECT_MODE_XGSPON){
		printk("%s SLT online testing\n", mode == XMCS_IF_WAN_DETECT_MODE_XGPON ? "XGPON" : "XGSPON");
		xmcs_set_link_detection(mode);
		xmcs_set_connection_start(XPON_ENABLE);
		for(delay_cnt = 0 ; delay_cnt < 600; delay_cnt++){
			if( gpon_slt_test != 0)
				break;
			msleep(50); 
			printk(".");
		}
		printk("\n");
		printk("delay = %d ms\n",delay_cnt*50);
		xmcs_set_connection_start(XPON_DISABLE);
		if(gpon_slt_test == 0){
			printk("%s SLT TEST FAIL!!\n", mode == XMCS_IF_WAN_DETECT_MODE_XGPON ? "XGPON" : "XGSPON");
			return ECNT_RETURN;
		}
		else{
			printk("%s SLT TEST SUCCESS!!\n", mode == XMCS_IF_WAN_DETECT_MODE_XGPON ? "XGPON" : "XGSPON");
			return ECNT_CONTINUE;
		}
	}
	else{
		printk("\nSLT Epon testing\n");
        xmcs_set_connection_start(XPON_DISABLE);
        xmcs_set_link_detection(XMCS_IF_WAN_DETECT_MODE_10G_1G_EPON);
        msleep(1);
        xmcs_set_connection_start(XPON_ENABLE);

        for(delay_cnt = 0 ; delay_cnt < 500; delay_cnt++)
        {
            if(epon_slt_test != 0)
            break;
            msleep(50);
            printk(".");
        }

        if(1 == epon_slt_test){
            printk("\nXEpon asy test success.\n");
            return ECNT_CONTINUE;
        }
        else{
            printk("\nXEpon asy test fail.\n");
            return ECNT_RETURN;
        }
	}
}
#endif

static int xpon_mac_pub_module_hook_dispatch(xpon_mac_hook_data_t *data)
{
    struct XMCS_GponOnuInfo_S onuInfo ;
    struct XMCS_GponSnPasswd_S gponSnPasswd ;
    
    switch (data->pub_info.type)
    {
        case XPON_SN_SET:
        {
            memset(&gponSnPasswd, 0, sizeof(struct XMCS_GponSnPasswd_S)) ;
            gpon_cmd_proc(GPON_IOG_ONU_INFO, (ulong)&onuInfo) ;
            memcpy(gponSnPasswd.passwd, onuInfo.passwd, GPON_PASSWD_LENS);
            memcpy(gponSnPasswd.sn, data->pub_info.sn, 8) ;
            gpon_cmd_proc(GPON_IOS_SN_PASSWD, (ulong)&gponSnPasswd) ;
            break;
        }
        case XPON_MAC_MODE_GET:
        {
            if (PON_LINK_STATUS_GPON == gpPonSysData->sysLinkStatus || PHY_GPON_CONFIG == gpPhyData->working_mode){
                *(data->pub_info.mode) = ECNT_XPON_MAC_MODE_GPON;
            }else if (PON_LINK_STATUS_EPON == gpPonSysData->sysLinkStatus || PHY_EPON_CONFIG == gpPhyData->working_mode){
                *(data->pub_info.mode) = ECNT_XPON_MAC_MODE_EPON;
            }else {
                *(data->pub_info.mode) = ECNT_XPON_MAC_MODE_OFF;
            }
            break;
        }
        case XPON_ONU_TYPE_GET:
        {
            if (0 != xmcs_get_onu_type((XMCSIF_OnuType_t *)& data->pub_info.onu_type) )
            {
                printk("Get onu type failed!\n");
                return ECNT_HOOK_ERROR;
            }
            break;
        }
        case XPON_TCONT_GET:
        {
            *(data->pub_info.gemtotcont.tcont) = get_tcont_from_gemportid(data->pub_info.gemtotcont.gemportid);
            break;
        }
        case XPON_GEMPORT_REMOVE:
        {
            if (0 != xmcs_remove_gem_port(data->pub_info.Remove_gemportID))
            {
                PON_MSG(MSG_ERR, "XPON_GEMPORT_REMOVE failed.\n");
                return ECNT_HOOK_ERROR;
            }
            break;
        }
        case XPON_QOS_SET:
        {
            if (0 != xpon_set_qos(data->pub_info.ponsetqos.enable,data->pub_info.ponsetqos.queueNum))
            {
                PON_MSG(MSG_ERR, "XPON_QOS_SET failed!\n");
                return ECNT_HOOK_ERROR;
            }
            break;
        }
        case XPON_TCONT_INFO_GET:
        {
            if (0 != xmcs_get_tcont_info(data->pub_info.pTcontInfo))
            {
                PON_MSG(MSG_ERR, "XPON_TCONT_INFO_GET failed!\n");
                return ECNT_HOOK_ERROR;
            }
            break;
        }
        case XPON_GEMPORT_CREATE:
        {
            if (0 != xmcs_create_gem_port(data->pub_info.pGemCreate))
            {
                PON_MSG(MSG_ERR, "XPON_GEMPORT_CREATE failed!\n");
                return ECNT_HOOK_ERROR;
            }
            break;
        }
	    case XPON_CHANNEL_QOS_SET:
        {
            if (0 != xmcs_set_channel_scheduler(data->pub_info.pScheduler))
            {
                PON_MSG(MSG_ERR, "XPON_CHANNEL_QOS_SET failed!\n");
                return ECNT_HOOK_ERROR;
            }
            break;
        }
	    case XPON_WANLINK_CONFIG_GET:
        {
            if (0 != xmcs_get_wan_link_status(data->pub_info.pSysLinkStatus))
            {
                PON_MSG(MSG_ERR, "XPON_WANLINK_CONFIG_GET failed!\n");
                return ECNT_HOOK_ERROR;
            }
            break;
        }
        case XPON_MULITCAST_ANI_GET:
        {
            data->pub_info.mulitcast_ani = mulitcast_ani;
            break;
        }
        case XPON_WAN_NET_START_XMIT:
        {
            data->pub_info.wan_xmit.ret = pwan_net_start_xmit(data->pub_info.wan_xmit.skb, data->pub_info.wan_xmit.dev);
            break;
        }
        case XPON_RESET:
        {
             if (0 != epon_reset(NULL))
             {
                  printk("epon reset failed!\n");
                  return ECNT_HOOK_ERROR;
              }
              break;
        }
        case XPON_EVENT_REPORT:
        {
            if (PON_LINK_STATUS_GPON == gpPonSysData->sysLinkStatus ){
                xmcs_report_event(XMCS_EVENT_TYPE_GPON ,data->pub_info.event.id,data->pub_info.event.vlaue);
            }else if (PON_LINK_STATUS_EPON == gpPonSysData->sysLinkStatus ){
                xmcs_report_event(XMCS_EVENT_TYPE_EPON ,data->pub_info.event.id,data->pub_info.event.vlaue);
            }
            break;
        }
#if defined (TCSUPPORT_AUTOBENCH)
        case XPON_SLT:
            epon_slt_test = 0;
			gpon_slt_test = 0;
#if defined (TCSUPPORT_CPU_EN7581)
			return(do_xpon_10g_slt_action(XMCS_IF_WAN_DETECT_MODE_XGSPON));
#else
			return(do_xpon_10g_slt_action(XMCS_IF_WAN_DETECT_MODE_10G_1G_EPON));
#endif
            break;
#endif
        default:
            panic("XPON MAC Driver not ready to receive event from module with id:%d pub_type:%d", data->src_module,data->pub_info.type);
            return ECNT_RETURN;
    }

    return ECNT_RETURN;
}
/*______________________________________________________________________________
**	function name
**		xpon_mac_hook_dispatch
**	description:
**		Dispatch the phy event/xpon mac interrupt handler.
**	parameters:
**		in_data: ECNT Hook raw data.
**	global:
**		gpPhyData
**	return:
**		ECNT_HOOK_ERROR: ECNT hook ok.
**		ECNT_RETURN: ECNT hook failure.
**	call:
**		xpon_phy_event_dispatch
**		epon_isr
**		gpon_isr
**	revision:
**		v1.0
**____________________________________________________________________________*/
static int xpon_mac_hook_dispatch(struct ecnt_data  * in_data)
{
    int ret = 0;
    xpon_mac_hook_data_t * data = (xpon_mac_hook_data_t *)in_data;
    
    switch (data->src_module)
    {
        case XPON_PHY_MODULE:
            xpon_phy_event_dispatch(data->pEvent);
            break;
        case XPON_PUB_MODULE:
            ret = xpon_mac_pub_module_hook_dispatch(data);
            break;
        case XPON_INT_MODULE:
            if(gpPhyData->working_mode == PHY_EPON_CONFIG)
                epon_isr();
            else
                gpon_isr();
            break;
        default:
            printk("XPON 10g MAC Driver not ready to receive event from module with id:%d,", data->src_module);
            return ECNT_HOOK_ERROR;
    }

    return ret;
}
/*______________________________________________________________________________
**	function name
**		xpon_stop_timer
**	description:
**		Delete the xpon mac timer handler function.
**	parameters:
**		None.		
**	global:
**		gpPhyData.
**	return:
**		None.
**	call:
**		gpon_stop_timer.
**	revision:
**		v1.0
**____________________________________________________________________________*/
void xpon_stop_timer(void)
{
    /*Don't access phy register, so timer don't stop*/
    if(PHY_GPON_CONFIG == gpPhyData->working_mode){
        gpon_stop_timer();
    }else if(PHY_EPON_CONFIG == gpPhyData->working_mode){
        /* todo: addd epon_stop_timer */
    }
}

/*______________________________________________________________________________
**	function name
**		xpondrv_cleanup
**	description:
**		XPON MAC Driver de-init function and clean up resource.
**	parameters:
**		None.	
**	global:
**		gpPonSysData.
**		xpondrv_hook_dispatch_ops.
**	return:
**		None.	
**	call:
**		gpon_deinit.	
**		xpon_proc_dest.	
**		epon_deinit.	
**          stop_omci_oam_monitor.
**		xpon_mci_destroy.	
**		pwan_destroy.
**	revision:
**		v1.0
**____________________________________________________________________________*/
void xpondrv_cleanup(void)
{
#ifdef TCSUPPORT_CPU_ARMV8_64
		int irq = -1;
		struct device *dev = NULL ;
#endif

	xpon_daemon_quit();
    gpon_deinit();
    xpon_proc_dest();    
    epon_deinit();       
    stop_omci_oam_monitor();

#ifdef TCSUPPORT_CPU_ARMV8
	irq = get_xpon_irq(1);
	dev = get_xpon_dev();
	free_irq(irq, dev);
#else
    free_irq(DYINGGASP_INT, NULL);
#endif

    xpon_mci_destroy();
    pwan_destroy(); 
	xpondrv_qdma_deinit();
    ecnt_unregister_hook(&xpondrv_hook_dispatch_ops);
    xpon_api_deinit();
	XPON_STOP_TIMER(gpPhyData->trans_status_refresh_timer) ;
	XPON_STOP_TIMER(gpPhyData->traffic_status_refresh_timer) ;
    if(gpPonSysData != NULL) {
        kfree(gpPonSysData) ;
        gpPonSysData = NULL ;
    }
}

/*______________________________________________________________________________
**	function name
**		xpondrv_init
**	description:
**		xpon mac driver init function. 
**	parameters:
**		None.	
**	global:
**		gpPonSysData.
**		xpondrv_hook_dispatch_op
**	return:
**		0: init ok
**		-EFAULT: init failure.
**	call:
**		XPON_DPRINT_MSG.	
**		xpondrv_init_global_data.	
**		omci_oam_monitor_init.	
**		pwan_init.
**		xpon_mci_init.	
**		epon_init.	
**		gpon_init.
**		xpondrv_qdma_init.
**		xpon_dying_gasp_init.	
**		xpon_proc_init.
**		xpondrv_cleanup.
**	revision:
**		v1.0
**____________________________________________________________________________*/
int xpondrv_init(void)
{
	printk("xPON driver initialing.\n");
	
    INIT_LIST_HEAD(&fix_reg_list);

	if(init_union_ic_function() != 0)
	{
		printk("Init UNION IC Table failed\n") ;
		goto ret ;
	}

	/* initial the global data memory */
	gpPonSysData = (PON_SysData_T *)kmalloc(sizeof(PON_SysData_T), GFP_KERNEL) ;
	if(gpPonSysData == NULL){
		printk("Alloc data struct memory failed\n") ;
		goto ret ;
	}
	else{
		memset(gpPonSysData, 0x0, sizeof(PON_SysData_T));
		XPON_DPRINT_MSG("Alloc data struct memory successful, %d\n", (uint)sizeof(PON_SysData_T)) ;
		xpondrv_init_global_data() ;
	}

    omci_oam_monitor_init(&gpPonSysData->Omci_Oam_Monitor) ;

	/* WAN/OAM/OMCI interface driver initizliation */
	if(pwan_init() != 0){
		printk("XPON WAN/OAM/OMCI interface driver initialization failed\n") ;
		goto ret ;
	}	

	/* XPON character device driver initizliation */
	if(xpon_mci_init() != 0) {
		printk("PON monitor and control interface driver initialization failed\n") ;
		goto ret ;
	}
	if (ECNT_REGISTER_SUCCESS != ecnt_register_hook(&xpondrv_hook_dispatch_ops)){
        panic("Register hook function failed! %s:%d", __FUNCTION__, __LINE__);
	}
	/* EPON initialization */
	if(epon_init() != 0) {
		printk("EPON initialization failed\n");
		goto ret ;
	}
	/* GPON initialization */
	if(gpon_init() != 0){
		printk("GPON initialization failed\n") ;
		goto ret ;
	}
	
	xpondrv_qdma_init();

	if(xpon_dying_gasp_init() != 0) {
		printk("xpon dying gasp initialization failed\n") ;
		goto ret ;
	}
	
	if(xpon_proc_init() != 0){
		printk("xpon vlan filter initialization failed\n") ;
		goto ret ;
	}

        /*LED flicker*/ 
        change_alarm_led_status(LED_FLICKER);

	gpPonSysData->sysMACStartup = PON_MAC_START;
	XPON_START_TIMER(gpPhyData->traffic_status_refresh_timer,1000) ;/* 1,000 ms */

	if(xpon_api_init() != 0){
		printk("xpon api initialization failed\n") ;
		goto ret ;
	}

   	printk("%s\n", MODULE_VERSION_10GXPONMAC);	
	return 0;	
	
ret:
	xpondrv_cleanup() ;
	return -EFAULT ;
}

module_init(xpondrv_init)
module_exit(xpondrv_cleanup)
MODULE_LICENSE("GPL");

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
#include <common/omci_oam_monitor.h>
#include <common/xpon_global.h>
#include <common/xpondrv.h>
#include "gpon/gpon_recovery.h"

static Omci_Oam_Monitor_Params_t *gpMonitorParams = NULL;
static Omci_Oam_Monitor_t *gpMonitor = NULL;
/*______________________________________________________________________________
**	function name
**		get_netif_rx_counter
**	description:
**		Counting as  omci/oam interface rx packet counter.
**	parameters:
**		None
**	global:
**		None
**	return:
**		RX OAM/OMCI packet counter.
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/
static inline unsigned long get_netif_rx_counter(void)
{
    unsigned long val = 0;
    PWAN_NetPriv_T *pNetPriv = NULL; 

    if (PHY_GPON_CONFIG == gpPhyData->working_mode){
        pNetPriv = netdev_priv(gpWanPriv->pPonNetDev[PWAN_IF_OMCI]) ;
    }else{
        pNetPriv = netdev_priv(gpWanPriv->pPonNetDev[PWAN_IF_OAM]) ;
    }
    
    val = pNetPriv->stats.rx_packets;
    return val ;
}
/*______________________________________________________________________________
**	function name
**		stop_omci_oam_monitor_not_online
**	description:
**		Stop xponCntRxPkt timer to count oam/omci packets. 
**	parameters:
**		None
**	global:
**		gpMonitor
**	return:
**		None
**	call:
**		XPON_STOP_TIMER
**	revision:
**		v1.0
**____________________________________________________________________________*/
void stop_omci_oam_monitor_not_online(void)
{
    gpMonitor->run_state = MONITOR_STOP;
    XPON_STOP_TIMER(gpMonitor->xponCntRxPkt); 
}
/*______________________________________________________________________________
**	function name
**		stop_omci_oam_monitor
**	description:
**		Stop xponCntRxPkt timer to count oam/omci packets. 
**	parameters:
**		None
**	global:
**		gpMonitor
**	return:
**		None
**	call:
**		XPON_STOP_TIMER
**	revision:
**		v1.0
**____________________________________________________________________________*/
void stop_omci_oam_monitor(void)
{
    if(gpMonitor->run_state != MONITOR_STOP)
	{
        gpMonitor->run_state = MONITOR_STOP;    
        XPON_STOP_TIMER(gpMonitor->xponCntRxPkt);
    }
}
/*______________________________________________________________________________
**	function name
**		start_omci_oam_monitor
**	description:
**		Init global gpMonitor and start xponCntRxPkt timer to count oam/omci packets. 
**	parameters:
**		None
**	global:
**		gpMonitor
**		gpMonitorParams
**		gpPhyData
**	return:
**		None
**	call:
**		XPON_DPRINT_MSG
**		get_netif_rx_counter
**		XPON_START_TIMER
**	revision:
**		v1.0
**____________________________________________________________________________*/
void start_omci_oam_monitor(void)
{
    memset(gpMonitor->pkt_cnt_diff, 0x0, sizeof(gpMonitor->pkt_cnt_diff) );

	gpMonitor->pkt_ever_arrived = FALSE;
    gpMonitor->timeout_cnt = 0;
    gpMonitor->diff_idx    = 0;
    gpMonitor->total_diff  = 0;

    gpMonitorParams = ((gpPhyData->working_mode == PHY_GPON_CONFIG) ?
                            &(gpMonitor->gpon_params) :
                            &(gpMonitor->epon_params) );

	gpMonitor->xponCntRxPkt.expires = gpMonitorParams->timer_interval;
    XPON_DPRINT_MSG("start monitor omci/oam pkts!\n");
    
    gpMonitor->last_pkt_cnt = get_netif_rx_counter();
    gpMonitor->run_state = MONITOR_RUNNING ;
 
    XPON_START_TIMER(gpMonitor->xponCntRxPkt,gpMonitorParams->timer_interval);
}
/*______________________________________________________________________________
**	function name
**		xpon_send_traffic_up_event
**	description:
**		Send traffic up event 
**	parameters:
**		None
**	global:
**		gpMonitor
**	return:
**		NULL
**	call:
**		NULL
**	revision:
**		v1.0
**____________________________________________________________________________*/
void xpon_send_traffic_up_event(void)
{
	if((TRAFFIC_DOWN == gpGponPriv->gpon_traffic_status)&&(MONITOR_DONE == gpMonitor->run_state)){
		xmcs_report_event(XMCS_EVENT_TYPE_GPON, XMCS_EVENT_GPON_TRAFFIC_STATUS_CHANGE, 1) ;
		gpGponPriv->gpon_traffic_status = TRAFFIC_UP;
		XPON_DPRINT_MSG("trafficup_dbg:[%s] [%d] Create Gemport set traffic up!\n",__FUNCTION__,__LINE__);
	}
	return;
}
/*______________________________________________________________________________
**	function name
**		is_really_up
**	description:
**		check if data gemport is assigned
**	parameters:
**		None
**	global:
**		gpWanPriv
**	return:
**		TRAFFIC_UP/TRAFFIC_DOWN
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/
static int is_really_up(void)
{
	int i = 0;
	for(i = 0; i<CONFIG_GPON_10G_MAX_GEMPORT; i++){
		if( (gpWanPriv->gpon.gemPort[i].info.valid) && 
			 (gpWanPriv->gpon.gemPort[i].info.portId != GPON_OMCC_ID) ){
			 break;
		}
	}
	if(CONFIG_GPON_10G_MAX_GEMPORT == i){
		return TRAFFIC_DOWN;
	}
	return TRAFFIC_UP;
}

/*______________________________________________________________________________
**	function name
**		monitor_done
**	description:
**		Monitor task done and clean hwnat entry 
**	parameters:
**		None
**	global:
**		gpPhyData
**		gpWanPriv
**		gpMonitor
**		gpPonSysData
**	return:
**		constant IRQ_HANDLED
**	call:
**		xpon_set_qdma_qos
**		XPON_DPRINT_MSG
**	revision:
**		v1.0
**____________________________________________________________________________*/
static void monitor_done(void)
{
	XPON_DPRINT_MSG("== check omci/oam conf done! ==\n");

	if (ra_sw_nat_hook_clean_table){
		ra_sw_nat_hook_clean_table();
	}
	
    gpMonitor->run_state = MONITOR_DONE ;
    gpPonSysData->onlineStartTime = jiffies;
		
	/* report GPON event message */	
	if((TRAFFIC_DOWN == gpGponPriv->gpon_traffic_status)&&(gpPonSysData->sysBBF247 || TRAFFIC_UP == is_really_up())){
		XPON_DPRINT_MSG("== report traffic up! ==\n");
		xmcs_report_event(XMCS_EVENT_TYPE_GPON, XMCS_EVENT_GPON_TRAFFIC_STATUS_CHANGE, 1) ;
		gpGponPriv->gpon_traffic_status = TRAFFIC_UP;		
	}

	XPON_DPRINT_MSG("[%s] [%d] moniter done gemport recover, tcontNum=%d !\n",__FUNCTION__,__LINE__,gpWanPriv->activeChannelNum);
	if(gpPhyData->working_mode == PHY_GPON_CONFIG && (GPON_CURR_STATE == GPON_10G_STATE_O4 || GPON_CURR_STATE == GPON_10G_STATE_O5)){
		gpon_exec_gem_recover();
	}

	if(gpPhyData->working_mode == PHY_EPON_CONFIG)
    {
    	xmcs_report_event(XMCS_EVENT_TYPE_EPON, XMCS_EVENT_EPON_TRAFFIC_STATUS_CHANGE, TRAFFIC_UP) ;
    	XPON_DPRINT_MSG("trafficup_dbg:[%s] [%d] epon, driver set traffic up!\n",
    		__FUNCTION__,__LINE__);
    }
}
/*______________________________________________________________________________
**	function name
**		xpon_count_rx_pkt
**	description:
**		Monitor main task for counting oam/omci rx packets to check onu on-line or not. 
**	parameters:
**		None
**	global:
**		gpMonitor
**		gpMonitorParams
**	return:
**		None
**	call:
**		XPON_DPRINT_MSG
**		monitor_done
**		get_netif_rx_counter
**		XPON_START_TIMER
**	revision:
**		v1.0
**____________________________________________________________________________*/
void xpon_count_rx_pkt(TIMER_FUN_PAAM arg)
{
	unsigned long current_pkt_cnt = 0;
    unsigned long diff = 0;

	if (gpMonitor->run_state != MONITOR_RUNNING){
		return;
	}

	gpMonitor->timeout_cnt++;
    
	if (gpMonitor->timeout_cnt > gpMonitorParams->max_timeout_cnt )
	{
        XPON_DPRINT_MSG("timeout cnt reach max value: %u\n", gpMonitorParams->max_timeout_cnt);
		monitor_done();
        return;
	}
    
	current_pkt_cnt = get_netif_rx_counter();
    diff = current_pkt_cnt - gpMonitor->last_pkt_cnt;
    
    if(FALSE == gpMonitor->pkt_ever_arrived)
    {
        if(diff != 0)
        {
            gpMonitor->pkt_ever_arrived = TRUE;
            gpMonitor->timeout_cnt = 1;
        }else{
            goto restart_timer;
        }
    }
    
	if (gpMonitor->timeout_cnt > gpMonitorParams->max_diff_cnt)
	{
        if(gpMonitor->diff_idx >= gpMonitorParams->max_diff_cnt){
            gpMonitor->diff_idx -= gpMonitorParams->max_diff_cnt;
        }

        gpMonitor->total_diff -= gpMonitor->pkt_cnt_diff[gpMonitor->diff_idx];
	}
    
    gpMonitor->total_diff += diff;    
	gpMonitor->pkt_cnt_diff[gpMonitor->diff_idx] = diff ;
    gpMonitor->last_pkt_cnt = current_pkt_cnt;

	if (gpMonitor->timeout_cnt >= gpMonitorParams->max_diff_cnt)
	{
    	XPON_DPRINT_MSG("== cnt_diff_total: %lu\n", gpMonitor->total_diff);
    	if (gpMonitor->total_diff < gpMonitorParams->rx_pkt_lmt)
    	{
    		monitor_done();
    	}
	}

    gpMonitor->diff_idx ++;

restart_timer:
	    XPON_START_TIMER(gpMonitor->xponCntRxPkt,gpMonitorParams->timer_interval);
}

/*______________________________________________________________________________
**	function name
**		print_help
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
static inline void print_help(void)
{
    printk("Command e.g.:\n"
           "    help        # show this message\n"
           "    [gpon|epon] maxCntIdx   # config maximum number for counting rx packets\n"
           "    [gpon|epon] rxPktLmt    # config rx packets limit\n"
           "    [gpon|epon] interval    # config timer interval\n"
           "    [gpon|epon] maxWaitTime # max wait time before wan traffic on (unit ms)"
           );
}
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
int xpon_prepare_params(char * cmdline, size_t * argc, char ** argv)
{
    size_t cnt = 0, pos = 0;
    enum{SEARCH_ARG, HIT_ARG} state = SEARCH_ARG;
    
    for(pos = 0; cmdline[pos] != 0; ++pos)
    {
        switch(state)
        {
            case SEARCH_ARG:
                if('\x20' != cmdline[pos] && '\n' != cmdline[pos])
                {
                    state = HIT_ARG;
                    argv[cnt] = & cmdline[pos];
                }
                break;

            case HIT_ARG:
                if('\x20' == cmdline[pos] || '\n' == cmdline[pos])
                {
                    cmdline[pos] = 0;
                    state = SEARCH_ARG;
                    ++ cnt;
                    if(cnt > MAX_CMD_LINE_ARGC)
                    {
                        return 1;
                    }
                }
                break;
        }
    }

    *argc = cnt;
    return 1;
}
/*______________________________________________________________________________
**	function name
**		xpon_omai_oam_monitor_read_proc
**	description:
**		The monitor task profile read function.
**	parameters:
**		buf: 
**		start:
**		off: 
**		count: 
**		eof: 
**		data: 
**	global:
**		None
**	return:
**		character length.
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/
int xpon_omai_oam_monitor_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	 int len = 0;

	 len = sprintf(buf,"%d\n", ((isEPONFWID) ? 1 : 0) );

	 if (len < off + count)
		 *eof = 1;
 
	 len -= off;
	 *start = buf + off;
	 if(len > count)
		 len = count;
	  if(len < 0)
		 len = 0;

	return len;
}

/*______________________________________________________________________________
**	function name
**		xpon_omci_oam_monitor_write_proc
**	description:
**		The monitor task profile write function.
**	parameters:
**		file: 
**		buffer:
**		count: 
**		data: 
**	global:
**		gpMonitorParams
**		gpMonitor

**	return:
**		Input character  string length.
**	call:
**		xpon_prepare_params
**		print_help
**	revision:
**		v1.0
**____________________________________________________________________________*/
int xpon_omci_oam_monitor_write_proc(struct file *file, const char *buffer, unsigned long count, void *data)
{
	char string[64] = {0}, *argv[MAX_CMD_LINE_ARGC];
    size_t argc = 0;
	
	if (count > sizeof(string) - 1)
		return -EINVAL ;

	if (copy_from_user(string, buffer, count))
		return -EFAULT ;
	
	string[count] = 0;
    
    xpon_prepare_params(string, &argc, argv);

    if (0 == argc)
    {
        printk("Command needed!\n");
        print_help();
        return count;
    }
  
    if (!strcmp("help", argv[0]))
	{
        print_help();
        return count;
    }

	if (!strcmp("showCfg", argv[0]))
	{
        goto print_config;
    }

	if (!strcmp("gpon", argv[0]))
	{
        gpMonitorParams = & gpMonitor->gpon_params ;
    }
    else if(!strcmp("epon", argv[0])){
        gpMonitorParams = & gpMonitor->epon_params ;
    }
    else{
        printk("Unknow command %s!\n", argv[0]);
        print_help();
        return count;
    }
    
    
    if (!strcmp("maxCntIdx", argv[1]))
	{
		if (3 != argc)
		{
            printk("Error! \"maxCntIdx\" command takes 1 integer param!\n");
            print_help();
            goto print_config;
        }
        sscanf(argv[2], "%u", &(gpMonitorParams->max_diff_cnt) );
		if (gpMonitorParams->max_diff_cnt > MAX_ARRAY_NUM)
		{
			gpMonitorParams->max_diff_cnt = MAX_ARRAY_NUM;
		}
		goto print_config;
    }

    else if(!strcmp("rxPktLmt", argv[1]))
	{
        if (3 != argc)
		{
            printk("Error! \"rxPktLmt\" command takes 1 integer param!\n");
            print_help();
            goto print_config;
        }
        sscanf(argv[2], "%u", &(gpMonitorParams->rx_pkt_lmt) );
        goto print_config;
    }

	else if (!strcmp("interval", argv[1]))
	{
        if (3 != argc){
            printk("Error! \"interval\" command takes 1 integer param!\n");
            print_help();
            goto print_config;
        }

        sscanf(argv[2], "%u", &(gpMonitorParams->timer_interval) );
        if( 0 == gpMonitorParams->timer_interval ) {
            if(PHY_GPON_CONFIG == gpPhyData->working_mode ){
                gpMonitorParams->timer_interval = GPON_TIMER_INTERVAL ;
            }else{
                gpMonitorParams->timer_interval = EPON_TIMER_INTERVAL ;
            }
        }
        gpMonitorParams->max_timeout_cnt = 
            gpMonitorParams->max_wait_time/gpMonitorParams->timer_interval;
        goto print_config;
    }

	else if (!strcmp("maxWaitTime", argv[1]))
	{
        if (3 != argc){
            printk("Error! \"interval\" command takes 1 integer param!\n");
            print_help();
            goto print_config;
        }

        sscanf(argv[2], "%u", &(gpMonitorParams->max_wait_time) );
        gpMonitorParams->max_timeout_cnt = 
            gpMonitorParams->max_wait_time/gpMonitorParams->timer_interval;
        goto print_config;
    }


    else {
        printk("Unknown sub command: \"%s\"\n", argv[1]);
        print_help();
        return count;
    }

print_config:
    printk("GPON Current Confing:\n"
           "    Max number for counting rx packets: %u\n"
           "    Limit number for rx diff packets: %u\n"
           "    Timer interval: %u\n"
           "    max wait time: %u\n\n"
           "EPON Current Confing:\n"
           "    Max number for counting rx packets: %u\n"
           "    Limit number for rx diff packets: %u\n"
           "    Timer interval: %u\n"
           "    max wait time: %u\n",
                gpMonitor->gpon_params.max_diff_cnt,
                gpMonitor->gpon_params.rx_pkt_lmt,
                gpMonitor->gpon_params.timer_interval,
                gpMonitor->gpon_params.max_wait_time,
                gpMonitor->epon_params.max_diff_cnt,
                gpMonitor->epon_params.rx_pkt_lmt,
                gpMonitor->epon_params.timer_interval,
                gpMonitor->epon_params.max_wait_time);
	
    return count;
}
/*______________________________________________________________________________
**	function name
**		omci_oam_monitor_init
**	description:
**		The monitor timer task initalization
**	parameters:
**		monitor_data_ptr: 
**	global:
**		gpMonitor
**		gpMonitorParams
**	return:
**		None
**	call:
**		xpon_count_rx_pkt
**	revision:
**		v1.0
**____________________________________________________________________________*/
void omci_oam_monitor_init(Omci_Oam_Monitor_t * monitor_data_ptr)
{
    if(NULL == monitor_data_ptr){
        return ;
    }
    
    gpMonitor = monitor_data_ptr;
	memset(gpMonitor, 0x0, sizeof(Omci_Oam_Monitor_t) );
    
	/* Init timer to count Rx packets for XPON */
	GPON_CREATE_TIMER(&gpMonitor->xponCntRxPkt,xpon_count_rx_pkt,1000);

    gpMonitor->gpon_params.max_diff_cnt   = GPON_MAX_CNT_IDX ;
    gpMonitor->gpon_params.rx_pkt_lmt    = GPON_RX_PKT_LMT  ;
    gpMonitor->gpon_params.timer_interval= GPON_TIMER_INTERVAL ;
    gpMonitor->gpon_params.max_wait_time = GPON_MAX_WAIT_TIME ;
    gpMonitor->gpon_params.max_timeout_cnt = 
        gpMonitor->gpon_params.max_wait_time/gpMonitor->gpon_params.timer_interval;

    gpMonitor->epon_params.max_diff_cnt   = EPON_MAX_CNT_IDX ;
    gpMonitor->epon_params.rx_pkt_lmt    = EPON_RX_PKT_LMT  ;
    gpMonitor->epon_params.timer_interval= EPON_TIMER_INTERVAL ;
    gpMonitor->epon_params.max_wait_time = EPON_MAX_WAIT_TIME ;
    gpMonitor->epon_params.max_timeout_cnt = 
        gpMonitor->epon_params.max_wait_time/gpMonitor->epon_params.timer_interval;

    gpMonitor->run_state = MONITOR_STOP ;
    gpMonitorParams = & gpMonitor->gpon_params ;
}


#include <common/omci_oam_monitor.h>
#include <common/drv_global.h>
#include <common/xpondrv.h>



#define GPON_RX_PKT_LMT					1
#define GPON_MAX_CNT_IDX				15
#define GPON_TIMER_INTERVAL				200
#define GPON_MAX_WAIT_TIME              60000
#define GPON_MAX_NO_OMCI_WAIT_TIME      15000

#define EPON_RX_PKT_LMT					30
#define EPON_MAX_CNT_IDX				10
#define EPON_TIMER_INTERVAL				600
#define EPON_MAX_WAIT_TIME              15000
#define EPON_MAX_NO_OAM_WAIT_TIME       15000

static Omci_Oam_Monitor_Params_t * gpMonitorParams = NULL;
static Omci_Oam_Monitor_t * gpMonitor = NULL;

static inline unsigned long get_netif_rx_counter(void)
{
    const char * dev_name = ((PHY_GPON_CONFIG == gpPhyData->working_mode ) ? 
                                "omci" : "oam");
	struct net_device * net_dev = dev_get_by_name(&init_net, dev_name);
    PWAN_NetPriv_T *pNetPriv = (PWAN_NetPriv_T *)netdev_priv(net_dev);
    
    return pNetPriv->stats.rx_packets ;
}

void stop_omci_oam_monitor_not_online(void)
{
#ifndef TCSUPPORT_CPU_EN7521
    gpPonSysData->gswRateLimitFlag = FALSE;
    /*close p6 egress traffic limit */
	regWrite32(0xbfb59640,0x0) ;
#endif
    gpMonitor->run_state = MONITOR_STOP ;
    XPON_STOP_TIMER(gpMonitor->xponCntRxPkt)  ;
  
}


void stop_omci_oam_monitor(void)
{
#ifndef TCSUPPORT_CPU_EN7521
    xpon_disable_cpu_protection();
#endif
    gpMonitor->run_state = MONITOR_STOP ;
    XPON_STOP_TIMER(gpMonitor->xponCntRxPkt)  ;
    
}

void start_omci_oam_monitor(void)
{
    memset(gpMonitor->pkt_cnt_diff, 0, sizeof(gpMonitor->pkt_cnt_diff) );
    gpMonitor->pkt_ever_arrived = FALSE;
    gpMonitor->timeout_cnt = 0;
    gpMonitor->diff_idx    = 0;
    gpMonitor->total_diff  = 0;

    gpMonitorParams = ((gpPhyData->working_mode == PHY_GPON_CONFIG) ?
                            &(gpMonitor->gpon_params) :
                            &(gpMonitor->epon_params) );

	gpMonitor->xponCntRxPkt.data = gpMonitorParams->timer_interval;

    XPON_DPRINT_MSG("start monitor omci/oam pkts!\n");
    
    gpMonitor->last_pkt_cnt = get_netif_rx_counter();
    gpMonitor->run_state = MONITOR_RUNNING ;
    

#ifndef TCSUPPORT_CPU_EN7521
    xpon_enable_cpu_protection();
#endif

    XPON_START_TIMER(gpMonitor->xponCntRxPkt);
}

static void monitor_done(void)
{
	unsigned long flags = 0;

	if (gpPhyData->working_mode == PHY_GPON_CONFIG)
	{
		xpon_set_qdma_qos(gpWanPriv->devCfg.flags.isQosUp);
	}

		if (ra_sw_nat_hook_clean_table)
		{
			ra_sw_nat_hook_clean_table();
		}
	
	XPON_DPRINT_MSG("== omci/oam conf done! ==\n");
#ifndef TCSUPPORT_CPU_EN7521
	spin_lock_irqsave(&gpWanPriv->txLock, flags) ;
	/* Reset tx packet counter */
	gpWanPriv->devCfg.txPacketCount = 0;
	spin_unlock_irqrestore(&gpWanPriv->txLock, flags) ;	
    xpon_disable_cpu_protection();
    enable_cpu_us_data_traffic();
#endif
    gpMonitor->run_state = MONITOR_DONE ;
    gpPonSysData->onlineStartTime = jiffies;
    gpMonitor->pkt_ever_arrived = FALSE;
}

void xpon_count_rx_pkt(void)
{
	unsigned long current_pkt_cnt = 0;
    unsigned long diff = 0;

	if (gpMonitor->run_state != MONITOR_RUNNING){
		return;
	}

	gpMonitor->timeout_cnt ++;
    
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
        }
        else
        {
            if (gpMonitor->timeout_cnt > gpMonitorParams->max_no_oam_timeout_cnt)
            {
                XPON_DPRINT_MSG("On oam packet timeout cnt reach max value: %u\n", gpMonitorParams->max_no_oam_timeout_cnt);
                monitor_done();
                return;
            }
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
    	XPON_DPRINT_MSG("== cnt_diff_total: %u\n", gpMonitor->total_diff);
    	if (gpMonitor->total_diff < gpMonitorParams->rx_pkt_lmt)
    	{
    		monitor_done();
            return;
    	}
	}

    gpMonitor->diff_idx ++;

restart_timer:
	    XPON_START_TIMER(gpMonitor->xponCntRxPkt);
}

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

static inline void print_help(void)
{
    printk("Command e.g.:\n"
           "    help        # show this message\n"
           "    [gpon|epon] maxCntIdx        # config maximum number for counting rx packets\n"
           "    [gpon|epon] rxPktLmt         # config rx packets limit\n"
           "    [gpon|epon] interval         # config timer interval\n"
           "    [gpon|epon] maxWaitTime      # max wait time before wan traffic on (unit ms)"
           "    [gpon|epon] maxNoOAMWaitTime # No OMCI/OAM packet received, max wait time before wan traffic on (unit ms)"
           );
}

int xpon_omci_oam_monitor_write_proc(struct file *file, const char *buffer, unsigned long count, void *data)
{
	char string[64] = {0}, *argv[MAX_CMD_LINE_ARGC];
    size_t argc = 0;
	unsigned char idx = 0;
	
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
        gpMonitor->xponCntRxPkt.data = gpMonitorParams->timer_interval;
        gpMonitorParams->max_timeout_cnt = 
            gpMonitorParams->max_wait_time/gpMonitorParams->timer_interval;
        gpMonitorParams->max_no_oam_timeout_cnt= 
            gpMonitorParams->max_no_oam_wait_time/gpMonitorParams->timer_interval;
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
    else if (!strcmp("maxNoOamWaitTime", argv[1]))
	{
        if (3 != argc){
            printk("Error! \"maxNoOamWaitTime\" command takes 1 integer param!\n");
            print_help();
            goto print_config;
        }

        sscanf(argv[2], "%u", &(gpMonitorParams->max_no_oam_wait_time) );
        gpMonitorParams->max_no_oam_timeout_cnt = 
            gpMonitorParams->max_no_oam_wait_time/gpMonitorParams->timer_interval;
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
           "    max wait time: %u\n"
           "    max no OMCI wait time: %u\n\n"
           "EPON Current Confing:\n"
           "    Max number for counting rx packets: %u\n"
           "    Limit number for rx diff packets: %u\n"
           "    Timer interval: %u\n"
           "    max wait time: %u\n"
           "    max no OAM wait time: %u\n",
                gpMonitor->gpon_params.max_diff_cnt,
                gpMonitor->gpon_params.rx_pkt_lmt,
                gpMonitor->gpon_params.timer_interval,
                gpMonitor->gpon_params.max_wait_time,
                gpMonitor->gpon_params.max_no_oam_wait_time,
                gpMonitor->epon_params.max_diff_cnt,
                gpMonitor->epon_params.rx_pkt_lmt,
                gpMonitor->epon_params.timer_interval,
                gpMonitor->epon_params.max_wait_time,
                gpMonitor->epon_params.max_no_oam_wait_time);
	
    return count;
}

void omci_oam_monitor_init(Omci_Oam_Monitor_t * monitor_data_ptr)
{
    if(NULL == monitor_data_ptr){
        return ;
    }
    
    gpMonitor = monitor_data_ptr ;
	memset(gpMonitor, 0, sizeof(Omci_Oam_Monitor_t) );
    
	/* Init timer to count Rx packets for XPON */
	init_timer(&gpMonitor->xponCntRxPkt);
	gpMonitor->xponCntRxPkt.data = 1000;
	gpMonitor->xponCntRxPkt.function = xpon_count_rx_pkt;

    gpMonitor->gpon_params.max_diff_cnt   = GPON_MAX_CNT_IDX ;
    gpMonitor->gpon_params.rx_pkt_lmt    = GPON_RX_PKT_LMT  ;
    gpMonitor->gpon_params.timer_interval= GPON_TIMER_INTERVAL ;
    gpMonitor->gpon_params.max_wait_time = GPON_MAX_WAIT_TIME ;
    gpMonitor->gpon_params.max_timeout_cnt = 
        gpMonitor->gpon_params.max_wait_time/gpMonitor->gpon_params.timer_interval;
    gpMonitor->gpon_params.max_no_oam_wait_time = GPON_MAX_NO_OMCI_WAIT_TIME;
    gpMonitor->gpon_params.max_no_oam_timeout_cnt = 
        gpMonitor->gpon_params.max_no_oam_wait_time/gpMonitor->gpon_params.timer_interval;

    gpMonitor->epon_params.max_diff_cnt   = EPON_MAX_CNT_IDX ;
    gpMonitor->epon_params.rx_pkt_lmt    = EPON_RX_PKT_LMT  ;
    gpMonitor->epon_params.timer_interval= EPON_TIMER_INTERVAL ;
    gpMonitor->epon_params.max_wait_time = EPON_MAX_WAIT_TIME ;
    gpMonitor->epon_params.max_timeout_cnt = 
        gpMonitor->epon_params.max_wait_time/gpMonitor->epon_params.timer_interval;
    gpMonitor->epon_params.max_no_oam_wait_time = EPON_MAX_NO_OAM_WAIT_TIME;
    gpMonitor->epon_params.max_no_oam_timeout_cnt = 
        gpMonitor->epon_params.max_no_oam_wait_time /gpMonitor->epon_params.timer_interval;

    gpMonitor->run_state = MONITOR_STOP ;
    gpMonitorParams = & gpMonitor->gpon_params ;
}


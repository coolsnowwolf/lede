#include "../inc/common/xpon_daemon.h"
#include "../inc/xmcs/xmcs_if.h"
#include "../inc/xmcs/xmcs_phy.h"
#include <linux/kernel.h>
#include "../inc/common/phy_if_wrapper.h"
#include "../inc/gpon/gpon_dev.h"

#ifndef TCSUPPORT_CPU_EN7521
int phy_down_reset(void);
#else
#include <ecnt_hook/ecnt_hook_fe.h>
#endif

void xpon_daemon_quit(void)
{
	if(!IS_ERR(gpPonSysData->xpon_daemon.task)){
		XPON_DAEMON_Job_data_t job	   ;
		job.id		 = XPON_DAEMON_JOB_QUIT ;
		job.priority = XPON_DAEMON_JOB_PRIORITY_HIGH  ;

		xpon_daemon_job_enqueue(&job)  ;	
		wake_up_xpon_daemon() ;
		kthread_stop(gpPonSysData->xpon_daemon.task);
	}
}
void xpon_daemon_job_enqueue(XPON_DAEMON_Job_data_t * job_data)
{
    XPON_DAEMON_Job_Queue_t * queue = &gpPonSysData->xpon_daemon.job_queue;
    
    ulong flags ;
    spin_lock_irqsave(&queue->lock, flags) ;
    
    uint idx = queue->in_index ;
    
    if(FALSE == queue->jobs[idx].valid){
        queue->jobs[idx].data       = *job_data ;      
        mb();
        queue->jobs[idx].valid      = TRUE ;
        mb();
        queue->in_index = JOB_QUEUE_IDX_INC(queue->in_index) ;
    }else{
        dump_stack();
        printk("xpon daemon job queue full! Drop jobs:%d\n", job_data->id);
    }
    // printk("!!!!!![%s] in_idx:%d out_idx:%d, job_id:%d, pri:%d\n", __FUNCTION__, queue->in_index, queue->out_index, job_data->id, job_data->priority) ;
    
    spin_unlock_irqrestore(&queue->lock, flags) ;
}

/* 
    Note: only xpon_daemon will dequeue, no need to use spinlock to protect 
*/
static int xpon_daemon_job_dequeue(XPON_DAEMON_Job_data_t * job_data)
{
    XPON_DAEMON_Job_Queue_t * queue = &gpPonSysData->xpon_daemon.job_queue;
    
    uint idx = queue->out_index ;
    
    if(FALSE == queue->jobs[idx].valid) {
        return FALSE ;
    }
    
    *job_data = queue->jobs[idx].data ;
    mb() ;
    queue->jobs[idx].valid = FALSE ;
    queue->out_index = JOB_QUEUE_IDX_INC(queue->out_index) ;

    // printk("!!!!!![%s] in_idx:%d out_idx:%d, job_id:%d, pri:%d\n", __FUNCTION__, queue->in_index, queue->out_index, job_data->id, job_data->priority) ;
    
    return TRUE;
}

static inline int xpon_daemon_job_dequeue_not_empty(void)
{
    XPON_DAEMON_Job_Queue_t * queue = &gpPonSysData->xpon_daemon.job_queue;

    return (TRUE == queue->jobs[queue->out_index].valid) ;
}

void xpon_stop_timer(void);


static void handle_xpon_daemon_job_get_phy_params(void)
{
    ulong flags;
    PHY_Trans_Status_t trans_status = {0};

	PON_MSG(MSG_TRACE, "%s\n", __FUNCTION__) ;

    XPON_PHY_GET_TRANS_STATUS(&trans_status); /* this may take a while */
    
    spin_lock(&gpPhyData->trans_params_lock)     ;
    gpPhyData->trans_params= trans_status.params ;
    spin_unlock(&gpPhyData->trans_params_lock)   ;
	
	if( (trans_status.alarms != 0) ){
        xmcs_detect_phy_trans_alarm(trans_status.alarms);
    }

    gpPhyData->trans_status_refresh_pending = 0;
}


#ifndef TCSUPPORT_CPU_EN7521
static void handle_xpon_daemon_job_fe_reset(void)
{
    XPON_DPRINT_MSG("#######Doing FE Reset!#######\n");
    XPON_PHY_SET(PON_SET_PHY_STOP);
    xpon_stop_timer();
	disable_cpu_us_traffic();
    if( 0!= phy_down_reset() ) /* FE reset failed */
    {
        printk("Error! FE Reset Failed!\n");
        enable_cpu_us_traffic();
        return;
    }
    
    gpPonSysData->fe_reset_happened = TRUE;

    if(PHY_GPON_CONFIG == gpPhyData->working_mode){
        /* lower CPU loading to make OMCI restart quicker */
        xpon_enable_cpu_protection();
        XPON_START_TIMER(gpPonSysData->gsw_p6_rate_timer);
    }

#if !defined(TCSUPPORT_XPON_HAL_API_EXT) && !defined(TCSUPPORT_XPON_HAL_API) && !defined(TCSUPPORT_FWC_ENV)
	else{
#endif
        xmcs_set_connection_start(XPON_ENABLE);
#if !defined(TCSUPPORT_XPON_HAL_API_EXT) && !defined(TCSUPPORT_XPON_HAL_API) && !defined(TCSUPPORT_FWC_ENV)
    }
#endif

}
#endif

#ifdef TCSUPPORT_CPU_EN7521
static void fe_gdm2_channel_retire(void)
{
	FE_API_SET_CHANNEL_RETIRE_ALL(FE_GDM_SEL_GDMA2, 32);
}
#endif

static void handle_xpon_daemon_job_gpon_dev_reset(void)
{
    int only_reset_mac;
	unsigned long flags;
#ifdef TCSUPPORT_CPU_EN7521
	only_reset_mac = 1;
#else
	only_reset_mac = (XPON_ROUGE_STATE_TRUE == gpPonSysData->rogue_state);
#endif

    XPON_DPRINT_MSG("#######Doing GPON MAC and XPON PHY Reset!#######\n");

	if(gpGponPriv->emergencystate) {
		XPON_PHY_TX_DISABLE();
		gpon_act_change_gpon_state(GPON_STATE_O7) ;
		gpon_record_emergence_info(GPON_SET_CONNECTION_REPORT_O7);
		return;
	}
    gpPonSysData->ponMacPhyReset = TRUE;

	/* The PON_SET_PHY_STOP will disable phy fw ready */
    XPON_PHY_SET(PON_SET_PHY_STOP) ;

    xpon_stop_timer();
	gponDevMbiStop(XPON_ENABLE) ;
	disable_cpu_us_traffic();         /* disable cpu traffic */

	/* channel retire */
#ifdef TCSUPPORT_CPU_EN7521
	fe_gdm2_channel_retire();
#else
	qdma_set_retire(CONFIG_GPON_MAX_TCONT);
#endif /* TCSUPPORT_CPU_EN7521 */

    if(only_reset_mac)
    {
		/* Reset GPON MAC */
		gpon_dev_reset_GPON_MAC();
    } 
    else /* Reset GPON MAC & PHY*/
    { 
        /* Reset GPON MAC */
		gpon_dev_reset_GPON_MAC();

        /* Reset GPON PHY*/
#ifdef CONFIG_USE_MT7520_ASIC       
	spin_lock_irqsave(&gpPonSysData->event_lock, flags) ;
		IO_CBITS(TOP_TEST_MISC0_CTRL, SCU_GPON_PHY_RESET) ;
		udelay(1) ;
		IO_SBITS(TOP_TEST_MISC0_CTRL, SCU_GPON_PHY_RESET) ; 
	spin_unlock_irqrestore(&gpPonSysData->event_lock, flags) ;
#endif 
        
        if(XPON_PHY_SET(PON_SET_PHY_DEV_INIT) != 0){ /*Init PHY */
            dump_stack();
            printk("%s:phy device initialization failed.\n", __FUNCTION__);
        }

        XPON_PHY_SET(PON_SET_PHY_TRANS_MODEL_SETTING);
        XPON_PHY_SET_MODE(PHY_GPON_CONFIG) ;
    }
    
#ifndef TCSUPPORT_CPU_EN7521
    if (gpGponPriv->gponCfg.rstDbgDly){
        gpon_reset_dbg_delay();
    }
    setBwmFilterCtrl();  /* OSBNB00047232 */
#endif
    
    gpPonSysData->ponMacPhyReset = FALSE;

	if(PHY_LINK_STATUS_READY == gpPhyData->phy_link_status) {
		gpon_act_change_gpon_state(GPON_STATE_O2) ;
		gpon_enable();
    } else {
        gpon_act_change_gpon_state(GPON_STATE_O1) ;
    }

	/* The PON_SET_PHY_START will not enable phy fw ready */
	XPON_PHY_SET(PON_SET_PHY_START);
	/* enable FW ready */
	XPON_PHY_FW_READY_ENABLE();
    
    XPON_START_TIMER(gpPhyData->trans_status_refresh_timer);
}

static int xpon_daemon_job_dispatch(XPON_DAEMON_Job_data_t * job)
{
	int ret = XD_SUCCESS;
	
    switch(job->id){
        case XPON_DAEMON_JOB_GET_PHY_PARAM:
            handle_xpon_daemon_job_get_phy_params();
            break;

        case XPON_DAEMON_JOB_GPON_DEV_RESET:
            handle_xpon_daemon_job_gpon_dev_reset();
            break;

        case XPON_DAEMON_JOB_EPON_DEV_RESET:
            break;

    #ifndef TCSUPPORT_CPU_EN7521
        case XPON_DAEMON_JOB_FE_RESET:
            handle_xpon_daemon_job_fe_reset();
            break;
    #endif
        case XPON_DAEMON_JOB_QUIT:
            ret = XD_EXIT;
            break;

        default:
            dump_stack();
            printk("[%s]job id not recognized: %d\n", __FUNCTION__, job->id);
            ret = XD_ERROR;
            break;
    }
    return ret;
}

int xpon_daemon(void * data)
{   
    XPON_DAEMON_Job_data_t jobs[XPON_DAEMON_JOB_QUEUE_SIZE]= {0} ;
    XPON_DAEMON_Job_data_t job_tmp ;

    uint job_idx = 0 ;
    uint job_num = 0 ;

	while(!kthread_should_stop())
	{
        /* block until there is any job need to be done */
		wait_event_interruptible(gpPonSysData->xpon_daemon.wq,  xpon_daemon_job_dequeue_not_empty() ); 
        
        /* because jobs have different priority, it needs to be sorted before run */
        job_num = 0;
        while(XPON_DAEMON_JOB_QUEUE_SIZE > job_num && (TRUE == xpon_daemon_job_dequeue(&job_tmp) ) )
        {
            for(job_idx = 0; job_idx < job_num ; job_idx ++)
            {
                if(job_tmp.priority > jobs[job_idx].priority)
                {
                    break;
                }
            }
            memmove(jobs + job_idx + 1, jobs + job_idx, sizeof(jobs[0]) * (job_num - job_idx) );
            jobs[job_idx] = job_tmp ;
            job_num ++ ;
        }
        
        //printk("[%s] job_num:%u\n", __FUNCTION__, job_num);
        /* do the jobs sequentially */
    
        for(job_idx = 0; job_idx < job_num; job_idx ++)
        {
            //printk("[%s] job id:%u\n", __FUNCTION__, jobs[job_idx].id);
            if(xpon_daemon_job_dispatch(&jobs[job_idx])!= XD_SUCCESS)
                    return 0;
        }
	}

    return 0;
}



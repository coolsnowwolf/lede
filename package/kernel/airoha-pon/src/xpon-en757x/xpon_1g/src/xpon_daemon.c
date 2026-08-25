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
#include <linux/kernel.h>

#include "common/xpon_daemon.h"
#include "xmcs/xmcs_if.h"
#include "xmcs/xmcs_phy.h"
#include "common/phy_if_wrapper.h"
#include "gpon/gpon_dev.h"
#include "gpon/gpon_init.h"
#include <linux/kthread.h>

#ifndef TCSUPPORT_CPU_EN7521
int phy_down_reset(void);
#else
#include <ecnt_hook/ecnt_hook_fe.h>
#endif
extern int deallocate_flag;
extern int gpon_dev_reset_with_o7_flag;

void xpon_daemon_quit(void)
{
	if(!IS_ERR(gpPonSysData->xpon_daemon.task)){
		XPON_DAEMON_Job_data_t job = {0};
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
    uint idx = 0;
    
    spin_lock_irqsave(&queue->lock, flags) ;
    
    idx = queue->in_index ;
    
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
	static uint preAlarm = 0;
	int temp = 0;
    PHY_Trans_Status_t trans_status;

    memset(&trans_status, 0, sizeof(PHY_Trans_Status_t));

	PON_MSG(MSG_TRACE, "%s\n", __FUNCTION__) ;

    if(!isFPGA)
        XPON_PHY_GET_TRANS_STATUS(&trans_status); /* this may take a while */
    
    spin_lock_bh(&gpPhyData->trans_params_lock)     ;
    gpPhyData->trans_params= trans_status.params ;
    spin_unlock_bh(&gpPhyData->trans_params_lock)   ;
	
	/*Òì»òÔËËãÅÐ¶ÏÊÇ·ñÎªÖØ¸´¸æ¾¯	
	1.ÈôÊÇÔò²»ÉÏ±¨	
	2.Èô²»ÊÇÔò½øÐÐÓëÔËËãµÃ³ö±¾´ÎÐèÒªÉÏ±¨µÄ¸æ¾¯Öµ*/
	temp = preAlarm ^ trans_status.alarms;
	if(temp){
		temp &= trans_status.alarms;
		preAlarm = trans_status.alarms = temp;
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
    
	gpPonSysData->fe_reset_flag = TRUE;
    gpPonSysData->fe_reset_happened = TRUE;

    if(PHY_GPON_CONFIG == gpPhyData->working_mode){
        /* lower CPU loading to make OMCI restart quicker */
        xpon_enable_cpu_protection();
        XPON_START_TIMER(gpPonSysData->gsw_p6_rate_timer,0);
    }

#if/*TCSUPPORT_COMPILE*/ !defined(TCSUPPORT_XPON_HAL_API_EXT) && !defined(TCSUPPORT_XPON_HAL_API) && !defined(TCSUPPORT_FWC_ENV)
	else{
#endif/*TCSUPPORT_COMPILE*/
        xmcs_set_connection_start(XPON_ENABLE);
#if/*TCSUPPORT_COMPILE*/ !defined(TCSUPPORT_XPON_HAL_API_EXT) && !defined(TCSUPPORT_XPON_HAL_API) && !defined(TCSUPPORT_FWC_ENV)
    }
#endif/*TCSUPPORT_COMPILE*/

}
#endif

#ifdef TCSUPPORT_CPU_EN7521
static void fe_gdm2_channel_retire(void)
{
	FE_API_SET_CHANNEL_RETIRE_ALL(FE_GDM_SEL_GDMA2, 32);
}
#endif

void channel_retire_func(unsigned long arg)
{   
    if(atomic_read(&gpGponPriv->retire_flag))
    {
        atomic_set(&gpGponPriv->retire_flag, 0);
/* channel retire */
#ifdef TCSUPPORT_CPU_EN7521
        fe_gdm2_channel_retire();
#else
	    qdma_set_retire(GPON_TCONT_MAX_NUM);
#endif /* TCSUPPORT_CPU_EN7521 */
        atomic_set(&gpGponPriv->retire_flag, 1);
    }
}

#if !defined(TCSUPPORT_CPU_EN7523) && !defined(TCSUPPORT_CPU_EN7581)
extern int mismatchCnt;
extern uint64_t preHwRxCnt;
extern uint64_t preSwRxCnt;
#endif
void handle_xpon_daemon_job_gpon_dev_reset(void)
{
    XPON_DPRINT_MSG("#######Doing GPON MAC and XPON PHY Reset!#######\n");

#if !defined(TCSUPPORT_CPU_EN7523) && !defined(TCSUPPORT_CPU_EN7581)
	mismatchCnt=0;
	preHwRxCnt=0;
	preSwRxCnt=0;
#endif
    gpPonSysData->ponMacPhyReset = TRUE;
	deallocate_flag = 0;

	if(gpPhyData->calibrating != TRUE){
		XPON_PHY_TX_DISABLE();
	}
	
	if(!(isEN7580 || isEN7581 || isAN7583)){
		/* The PON_SET_PHY_STOP will disable phy fw ready */
    	XPON_PHY_SET(PON_SET_PHY_STOP) ;
	}
	
    xpon_stop_timer();
	gponDevResetCtrl(XPON_ENABLE) ;
	disable_cpu_us_traffic();         /* disable cpu traffic */

    channel_retire_func(0);

	FE_API_SET_GDMA_MISC_CONFIG(FE_GDM_SEL_GDMA2,FE_MISC_CONFIG_NONE);

    gpPonSysData->ponMacPhyReset = FALSE;

    if(gpGponPriv->emergencystate) {
		XPON_PHY_TX_DISABLE();
		gpon_act_change_gpon_state(GPON_STATE_O7) ;
		gpon_record_emergence_info(GPON_SET_CONNECTION_REPORT_O7);
		if(PHY_LINK_STATUS_READY == gpPhyData->phy_link_status) 
		{
			gpon_enable();
		}
	} else if(PHY_LINK_STATUS_READY == gpPhyData->phy_link_status) {
    	gpon_act_change_gpon_state(GPON_STATE_O2) ;
    	gpon_enable();
	} else {
		gpon_act_change_gpon_state(GPON_STATE_O1) ;
	}

	if(!(isEN7580 || isEN7581 || isAN7583)){
		/* The PON_SET_PHY_START will not enable phy fw ready */
		XPON_PHY_SET(PON_SET_PHY_START);
		/* enable FW ready */
		XPON_PHY_FW_READY_ENABLE();
	}
  	if(gpon_dev_reset_with_o7_flag){
		gpon_dev_reset_with_o7_flag = 0;
		XPON_PHY_TX_ENABLE();
	}
	if(PON_WAN_START == gpPonSysData->sysStartup) 
    	XPON_START_TIMER(gpPhyData->trans_status_refresh_timer,1000);/* 1,000 ms */
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
    XPON_DAEMON_Job_data_t jobs[XPON_DAEMON_JOB_QUEUE_SIZE];
    XPON_DAEMON_Job_data_t job_tmp ;

    uint job_idx = 0 ;
    uint job_num = 0 ;

    memset(&jobs, 0, sizeof(XPON_DAEMON_Job_data_t)*XPON_DAEMON_JOB_QUEUE_SIZE);

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



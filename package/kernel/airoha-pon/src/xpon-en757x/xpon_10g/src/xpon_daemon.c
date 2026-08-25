/***************************************************************
Copyright Statement:

This software/firmware and related documentation (“EcoNet Software”) 
are protected under relevant copyright laws. The information contained herein 
is confidential and proprietary to EcoNet (HK) Limited (“EcoNet”) and/or 
its licensors. Without the prior written permission of EcoNet and/or its licensors, 
any reproduction, modification, use or disclosure of EcoNet Software, and 
information contained herein, in whole or in part, shall be strictly prohibited.

EcoNet (HK) Limited  EcoNet. ALL RIGHTS RESERVED.

BY OPENING OR USING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY 
ACKNOWLEDGES AND AGREES THAT THE SOFTWARE/FIRMWARE AND ITS 
DOCUMENTATIONS (“ECONET SOFTWARE”) RECEIVED FROM ECONET 
AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON AN “AS IS” 
BASIS ONLY. ECONET EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, 
WHETHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, 
OR NON-INFRINGEMENT. NOR DOES ECONET PROVIDE ANY WARRANTY 
WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTIES WHICH 
MAY BE USED BY, INCORPORATED IN, OR SUPPLIED WITH THE ECONET SOFTWARE. 
RECEIVER AGREES TO LOOK ONLY TO SUCH THIRD PARTIES FOR ANY AND ALL 
WARRANTY CLAIMS RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES 
THAT IT IS RECEIVER’S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD 
PARTY ALL PROPER LICENSES CONTAINED IN ECONET SOFTWARE.

ECONET SHALL NOT BE RESPONSIBLE FOR ANY ECONET SOFTWARE RELEASES 
MADE TO RECEIVER’S SPECIFICATION OR CONFORMING TO A PARTICULAR 
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
#include "common/xpon_daemon.h"
#include "xmcs/xmcs_if.h"
#include "xmcs/xmcs_phy.h"
#include <linux/kernel.h>
#include "common/phy_if_wrapper.h"
#include "gpon/gpon_dev.h"
#include <ecnt_hook/ecnt_hook_fe.h>
#include <linux/kthread.h>

/************************************************************************
*                  D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/
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
/************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
*************************************************************************
*/
/************************************************************************
*                  P U B L I C   D A T A
*************************************************************************
*/
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
**		wake_up_xpon_daemon
**	description:
**		wake up xpon daemon
**	parameters:
**		None
**	global:
**		gpPonSysData
**	return:
**		None
**	call:
**	revision:
**		v1.0
**____________________________________________________________________________*/
void wake_up_xpon_daemon(void)
{
	wake_up_interruptible(&gpPonSysData->xpon_daemon.wq);
}

/*______________________________________________________________________________
**	function name
**		xpon_daemon_quit
**	description:
**		xpon daemon quit
**	parameters:
**		None
**	global:
**		gpPonSysData
**	return:
**		None
**	call:
**	revision:
**		v1.0
**____________________________________________________________________________*/
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
/*______________________________________________________________________________
**	function name
**		xpon_daemon_job_enqueue
**	description:
**		xpon daemon job enqueue quit
**	parameters:
**		job_data
**	global:
**		gpPonSysData
**	return:
**		None
**	call:
**	revision:
**		v1.0
**____________________________________________________________________________*/
void xpon_daemon_job_enqueue(XPON_DAEMON_Job_data_t * job_data)
{
    XPON_DAEMON_Job_Queue_t * queue = &gpPonSysData->xpon_daemon.job_queue;
    uint idx = 0;
    ulong flags ;
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
    
    spin_unlock_irqrestore(&queue->lock, flags) ;
}
/*______________________________________________________________________________
**	function name
**		xpon_daemon_job_dequeue
**	description:
**		xpon daemon job dequeue
**	parameters:
**		job_data
**	global:
**		gpPonSysData
**	return:
**		0:  ok
**		others: failure.
**	revision:
**		v1.0
**____________________________________________________________________________*/
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
    
    return TRUE;
}
/*______________________________________________________________________________
**	function name
**		xpon_daemon_job_dequeue_not_empty
**	description:
**		xpon daemon job dequeue not empty
**	parameters:
**		none
**	global:
**		gpPonSysData
**	return:
**		0:  ok
**		others: failure.
**	revision:
**		v1.0
**____________________________________________________________________________*/
static inline int xpon_daemon_job_dequeue_not_empty(void)
{
    XPON_DAEMON_Job_Queue_t * queue = &gpPonSysData->xpon_daemon.job_queue;

    return (TRUE == queue->jobs[queue->out_index].valid) ;
}
/*______________________________________________________________________________
**	function name
**		handle_xpon_daemon_job_get_phy_params
**	description:
**		handle xpon daemon job get phy params
**	parameters:
**		none
**	global:
**		gpPonSysData
**	return:
**		0:  ok
**		others: failure.
**	revision:
**		v1.0
**____________________________________________________________________________*/
static void handle_xpon_daemon_job_get_phy_params(void)
{
    //ulong flags;
	static uint preAlarm = 0;
	int temp = 0;
    PHY_Trans_Status_t trans_status;

	PON_MSG(MSG_TRACE, "%s\n", __FUNCTION__) ;

	memset(&trans_status, 0, sizeof(PHY_Trans_Status_t));

    XPON_PHY_GET_TRANS_STATUS(&trans_status); /* this may take a while */
    
    spin_lock_bh(&gpPhyData->trans_params_lock)     ;
    gpPhyData->trans_params= trans_status.params ;
    spin_unlock_bh(&gpPhyData->trans_params_lock)   ;
	
	/*异或运算判断是否为重复告警	
	1.若是则不上报	
	2.若不是则进行与运算得出本次需要上报的告警值*/
	temp = preAlarm ^ trans_status.alarms;
	if(temp){
		temp &= trans_status.alarms;
		preAlarm = trans_status.alarms = temp;
		xmcs_detect_phy_trans_alarm(trans_status.alarms);
	}

    gpPhyData->trans_status_refresh_pending = 0;
}
/*______________________________________________________________________________
**	function name
**		xpon_daemon_job_dispatch
**	description:
**		xpon daemon job dispatch
**	parameters:
**		none
**	global:
**		gpPonSysData
**	return:
**		0:  ok
**		others: failure.
**	revision:
**		v1.0
**____________________________________________________________________________*/
static int xpon_daemon_job_dispatch(XPON_DAEMON_Job_data_t * job)
{
	int ret = XD_SUCCESS;
	
    switch(job->id){
        case XPON_DAEMON_JOB_GET_PHY_PARAM:
            handle_xpon_daemon_job_get_phy_params();
            break;

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
/*______________________________________________________________________________
**	function name
**		xpon_daemon
**	description:
**		xpon daemon task
**	parameters:
**		none
**	global:
**		gpPonSysData
**	return:
**		0:  ok
**		others: failure.
**	revision:
**		v1.0
**____________________________________________________________________________*/
int xpon_daemon(void * data)
{   
    XPON_DAEMON_Job_data_t jobs[XPON_DAEMON_JOB_QUEUE_SIZE] ;
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
        
        /* do the jobs sequentially */
    
        for(job_idx = 0; job_idx < job_num; job_idx ++)
        {
            if(xpon_daemon_job_dispatch(&jobs[job_idx])!= XD_SUCCESS)
                    return 0;
        }
	}

    return 0;
}



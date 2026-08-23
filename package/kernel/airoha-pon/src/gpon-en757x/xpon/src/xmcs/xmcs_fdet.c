/*
* File Name: xmcs_fdet.c
* Description: Fault Detection for xPON Management Control Subsystem
*
******************************************************************/
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/wait.h>

#include "common/drv_global.h"
#include "gpon/gpon_dev.h"
#include "xmcs/xmcs_fdet.h"

/***************************************************************
***************************************************************/
uint xmcs_event_check_valid(XMCS_EventType_t type, uint id, uint value)
{

    return TRUE;
}


/***************************************************************
***************************************************************/
void xmcs_report_event(XMCS_EventType_t type, uint id, uint value)
{
    ulong flags ;
    uint index ;
    
    enum {GPON_MODE, EPON_MODE};
    uint wan_mode = IO_GREG(SCU_WAN_CONF_REG) & 0x3;

    if(PON_WAN_STOP == gpPonSysData->sysStartup)
    {
        return;
    }
    
    if(FALSE == xmcs_event_check_valid(type,id,value))
    {
        return;
    }
    
    if((GPON_MODE == wan_mode) && (XMCS_EVENT_TYPE_EPON == type))
    {
        return;
    }

    if((EPON_MODE == wan_mode) && (XMCS_EVENT_TYPE_GPON == type))
    {
        return;
    }
    
    PON_MSG(MSG_INT, "xmcs_report_event begin ,type %d id %d value %u\n",type,id,value) ;
    spin_lock_irqsave(&gpMcsPriv->fdetLock, flags) ;

	index = gpMcsPriv->xmcsEventStatus.items ;
	
	if(index < XMCS_EVENT_MAX_ITEMS) 
	{
		gpMcsPriv->xmcsEventStatus.event[index].type = type ;
		gpMcsPriv->xmcsEventStatus.event[index].id = id ;
		gpMcsPriv->xmcsEventStatus.event[index].value = value ;
		gpMcsPriv->xmcsEventStatus.items++ ;
		
		if(index >= 16)
		{
#ifndef TCSUPPORT_AUTOBENCH
			PON_MSG(MSG_XMCS, "WARNNING: xmcs Event exceed 16,type %d id %d value %u\n",type,id,value);
#endif
		}
	}
	else
	{
#ifndef TCSUPPORT_AUTOBENCH
		printk("ERROR: xmcs Event exceed max num,type %d id %d value %u\n",type,id,value);
#endif
	}
	spin_unlock_irqrestore(&gpMcsPriv->fdetLock, flags) ;

    wake_up_interruptible(&gpMcsPriv->xmcsWaitQueue) ;
}

/***************************************************************
***************************************************************/
static int xmcs_waiting_event(struct XMCS_PonEventStatus_S *pPonEventStatus) 
{
    ulong flags ;
    struct XMCS_PonEventStatus_S tmp;

    wait_event_interruptible(gpMcsPriv->xmcsWaitQueue, (gpMcsPriv->xmcsEventStatus.items)) ;
    
    spin_lock_irqsave(&gpMcsPriv->fdetLock, flags) ;
    
    memcpy(&tmp, &gpMcsPriv->xmcsEventStatus, sizeof(struct XMCS_PonEventStatus_S)) ;
    memset(&gpMcsPriv->xmcsEventStatus, 0, sizeof(struct XMCS_PonEventStatus_S)) ;

    spin_unlock_irqrestore(&gpMcsPriv->fdetLock, flags) ;
    if(0!=copy_to_user(pPonEventStatus, &tmp, sizeof(struct XMCS_PonEventStatus_S))){
         return -EFAULT;
    };
    mb();
    
    return 0 ;
}

/******************************************************************************
 Descriptor:    ioctl function for set event_ctrlFlag
 Input Args:    pointer to Event_ctrlFlag_t
 Ret Value: success: 0
               fail: -EFAULT
******************************************************************************/
static int xmcs_set_event_ctrl(Event_ctrlFlag_t *ctrl_flag)
{
    u16 flags = 0;   
    Event_ctrlFlag_t tmp;

    if(0!=copy_from_user(&tmp, ctrl_flag, sizeof(Event_ctrlFlag_t))){
        return -EFAULT;
    }
    
    spin_lock_irqsave(&gpMcsPriv->fdetLock, flags);
    memcpy(&gpMcsPriv->ctrlFlag, &tmp, sizeof(Event_ctrlFlag_t));
    spin_unlock_irqrestore(&gpMcsPriv->fdetLock, flags);

    printk("driver layer event_ctrl_flag: %x\n",tmp);
    return 0;
}

/******************************************************************************
 Descriptor:    ioctl function for get event_ctrlFlag
 Input Args:    pointer to Event_ctrlFlag_t
 Ret Value: success: 0
               fail: -EFAULT
******************************************************************************/
static int xmcs_get_event_ctrl(Event_ctrlFlag_t *ctrl_flag)
{
    u16 flags = 0;  
    Event_ctrlFlag_t tmp;
    
    spin_lock_irqsave(&gpMcsPriv->fdetLock, flags);
    memcpy(&tmp, &gpMcsPriv->ctrlFlag, sizeof(Event_ctrlFlag_t));
    spin_unlock_irqrestore(&gpMcsPriv->fdetLock, flags);
    
    if(0!=copy_to_user(ctrl_flag, &tmp, sizeof(Event_ctrlFlag_t))){
        return -EFAULT;
    }

    return 0;
}

/***************************************************************
***************************************************************/
int fdet_cmd_proc(uint cmd, ulong arg) 
{
    int ret ;
    int i = 0;

    switch(cmd) {
        case FDET_IOG_WAITING_EVENT :
            ret = xmcs_waiting_event((struct XMCS_PonEventStatus_S *)arg) ;
            break ;
        case FDET_IOG_RMMOD_EVENT:
            i = module_refcount(THIS_MODULE);
            printk("\r\n[%s][%d](%d)\r\n",__FUNCTION__,__LINE__,i);
            /*because ioctl increase mod refcnt up 2*/
            for(i -= XPON_MODULE_DECS ; i >0; i--){
                module_put(THIS_MODULE);    
            }
            break ;
        case FDET_IOS_EVENT_CTRL:
            ret = xmcs_set_event_ctrl((Event_ctrlFlag_t*) arg);
            break;
        case FDET_IOG_EVENT_CTRL:
            ret = xmcs_get_event_ctrl((Event_ctrlFlag_t*) arg);
            break ;
        default:
            PON_MSG(MSG_ERR, "No such I/O command, cmd: %x\n", cmd) ;
            break ;
    }

    return ret ;
}


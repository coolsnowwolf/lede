/*
* File Name: xmcs_fdet.c
* Description: Fault Detection for xPON Management Control Subsystem
*
******************************************************************/
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/module.h>
#include "common/drv_global.h"
#include "gpon/gpon_dev.h"
#include "xmcs/xmcs_fdet.h"
#include "pwan/xpon_netif.h"
/***************************************************************
***************************************************************/
uint xmcs_event_check_valid(XMCS_EventType_t type, uint id, uint value)
{
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_XPON_HAL_API_EXT)
    if(xpon_check_emergence_state() == TRUE)
    {
        if((XMCS_EVENT_TYPE_GPON == type)
        && (XMCS_EVENT_GPON_STATE_CHANGE == id)
        && (GPON_STATE_O7 == value))
        {   /* GPON in emergence state just report O7 */
            PON_MSG(MSG_INT, "Now Gpon in emergence state just report O7!\n") ;
        }
        else if(XMCS_EVENT_TYPE_EPON == type)
        {   /* EPON in tx power off ,just report id replace by XMCS_EVENT_EPON_TX_POWER_OFF */
            PON_MSG(MSG_INT, "Now Gpon in emergence state or Epon in Tx Power Off return!\n") ;
            return FALSE;
        }
        else
        {
            PON_MSG(MSG_INT, "Now is in emergence state just return!\n") ;
            return FALSE;
        }
    }
#endif/*TCSUPPORT_COMPILE*/ 

    return TRUE;
}


/***************************************************************
***************************************************************/
void xmcs_report_event(XMCS_EventType_t type, uint id, uint value)
{
    ulong flags ;
    uint index ;
    
    enum {GPON_MODE, EPON_MODE};
	
#ifdef TCSUPPORT_CPU_ARMV8	
	uint wan_mode = GET_WAN_CONF() & 0xff;
#else
    uint wan_mode = IO_GREG(SCU_WAN_CONF_REG) & 0x3;
#endif

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
	if(id == XMCS_EVENT_GPON_TRAFFIC_STATUS_CHANGE){
		if(gpWanPriv->pPonNetDev[PWAN_IF_DATA]== NULL){
			PON_MSG(MSG_ERR, "Alloc Pon net interface device %d (alloc_netdev()) failed\n", PWAN_IF_DATA) ;
		}else{
			if(value == TRAFFIC_UP){
				if(!netif_carrier_ok(gpWanPriv->pPonNetDev[PWAN_IF_DATA]))
						netif_carrier_on(gpWanPriv->pPonNetDev[PWAN_IF_DATA]);
			}else{
				if(netif_carrier_ok(gpWanPriv->pPonNetDev[PWAN_IF_DATA]))
						netif_carrier_off(gpWanPriv->pPonNetDev[PWAN_IF_DATA]);
			}
		}
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
    ulong flags = 0;   
    Event_ctrlFlag_t tmp;

    if(0!=copy_from_user(&tmp, ctrl_flag, sizeof(Event_ctrlFlag_t))){
        return -EFAULT;
    }
    
    spin_lock_irqsave(&gpMcsPriv->fdetLock, flags);
    memcpy(&gpMcsPriv->ctrlFlag, &tmp, sizeof(Event_ctrlFlag_t));
    spin_unlock_irqrestore(&gpMcsPriv->fdetLock, flags);

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
    ulong flags = 0;  
    Event_ctrlFlag_t tmp;
    
    spin_lock_irqsave(&gpMcsPriv->fdetLock, flags);
    memcpy(&tmp, &gpMcsPriv->ctrlFlag, sizeof(Event_ctrlFlag_t));
    spin_unlock_irqrestore(&gpMcsPriv->fdetLock, flags);
    
    if(0!=copy_to_user(ctrl_flag, &tmp, sizeof(Event_ctrlFlag_t))){
        return -EFAULT;
    }

    return 0;
}

static int xmcs_wait_queue_clear(Event_ctrlFlag_t *ctrl_flag)
{
    gpMcsPriv->xmcsEventStatus.items = 1;
	wake_up_interruptible(&gpMcsPriv->xmcsWaitQueue) ;

    return 0;
}

/***************************************************************
***************************************************************/
int fdet_cmd_proc(uint cmd, ulong arg) 
{
    int ret = -1;
#if 0
    int i = 0;
#endif

#ifdef TCSUPPORT_CPU_ARMV8_64
		cmd = cmd & IOCTL_CMD;
#endif

    switch(cmd) {
        case FDET_IOG_WAITING_EVENT :
            ret = xmcs_waiting_event((struct XMCS_PonEventStatus_S *)arg) ;
            break ;
#if 0
        case FDET_IOG_RMMOD_EVENT:
            i = module_refcount(THIS_MODULE);
            printk("\r\n[%s][%d](%d)\r\n",__FUNCTION__,__LINE__,i);
            /*because ioctl increase mod refcnt up 2*/
            for(i -= XPON_MODULE_DECS ; i >0; i--){
                module_put(THIS_MODULE);    
            }
            break ;
#endif
        case FDET_IOS_EVENT_CTRL:
            ret = xmcs_set_event_ctrl((Event_ctrlFlag_t*) arg);
            break;
        case FDET_IOG_EVENT_CTRL:
            ret = xmcs_get_event_ctrl((Event_ctrlFlag_t*) arg);
            break ;
		case FDET_IOS_QUEUE_CLEAR:
			ret = xmcs_wait_queue_clear((Event_ctrlFlag_t*) arg);
			break;
        default:
            PON_MSG(MSG_ERR, "No such I/O command, cmd: %x\n", cmd) ;
            break ;
    }

    return ret ;
}


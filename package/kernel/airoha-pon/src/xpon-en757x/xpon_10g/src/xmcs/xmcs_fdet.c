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
* File Name: xmcs_fdet.c
* Description: Fault Detection for xPON Management Control Subsystem
*
******************************************************************/
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/module.h>
#include "common/xpon_global.h"
#include "gpon/gpon_dev.h"
#include "xmcs/xmcs_fdet.h"
#include "common/xpon_api.h"
#include "xpon_event_global.h"
#include "pwan/xpon_netif.h"


/***************************************************************
***************************************************************/
void xmcs_report_event_buf(XMCS_EventType_t type, uint id, uint value, unsigned char *buf)
{
    ulong flags = 0;
    uint index = 0;
    uint wan_mode = 0;
    enum {GPON_MODE, EPON_MODE};

    if((gpPonSysData->sysPonMode == XMCS_IF_WAN_DETECT_MODE_XGPON) \
        ||(gpPonSysData->sysPonMode == XMCS_IF_WAN_DETECT_MODE_XGSPON) \
        || (gpPonSysData->sysPonMode == XMCS_IF_WAN_DETECT_MODE_NGPON2_10G_10G) \
        || (gpPonSysData->sysPonMode == XMCS_IF_WAN_DETECT_MODE_NGPON2_10G_2G) \
        || (gpPonSysData->sysPonMode == XMCS_IF_WAN_DETECT_MODE_NGPON2_2G_2G)){
        wan_mode = GPON_MODE;
    }else if((gpPonSysData->sysPonMode == XMCS_IF_WAN_DETECT_MODE_10G_1G_EPON) \
        ||(gpPonSysData->sysPonMode == XMCS_IF_WAN_DETECT_MODE_10G_10G_EPON) \
        ||(gpPonSysData->sysPonMode == XMCS_IF_WAN_DETECT_MODE_1G_1G_EPON) ||(gpPonSysData->sysPonMode == XMCS_IF_WAN_DETECT_MODE_TURBO_EPON)){
        wan_mode = EPON_MODE;
    }else{    
        return;
    }
    
    if(PON_WAN_STOP == gpPonSysData->sysStartup)
    {
        return;
    }
	
    if(0 == CALL_USER_HOOK_XMCS_EVENT_CHECK_VAILD(type, id, value))
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
		memcpy(&gpMcsPriv->xmcsEventStatus.event[index].buf,buf,XMCS_MAX_BUFFER_LEN);
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

void xmcs_report_event(XMCS_EventType_t type, uint id, uint value)
{
	unsigned char buf[XMCS_MAX_BUFFER_LEN] = {0};

    xmcs_report_event_buf(type,id,value,buf);
}

/***************************************************************
***************************************************************/
static int xmcs_waiting_event(struct XMCS_PonEventStatus_S *pPonEventStatus) 
{
    ulong flags = 0;
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
    int ret = 0;
    int i = 0;

#ifdef TCSUPPORT_CPU_ARMV8_64
			cmd = cmd & IOCTL_CMD;
#endif

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
		case FDET_IOS_QUEUE_CLEAR:
			ret = xmcs_wait_queue_clear((Event_ctrlFlag_t*) arg);
			break;
        default:
            PON_MSG(MSG_ERR, "No such I/O command, cmd: %x\n", cmd) ;
            break ;
    }

    return ret ;
}

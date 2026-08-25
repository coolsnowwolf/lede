/***************************************************************
Copyright Statement:

This software/firmware and related documentation (��EcoNet Software��) 
are protected under relevant copyright laws. The information contained herein 
is confidential and proprietary to EcoNet (HK) Limited (��EcoNet��) and/or 
its licensors. Without the prior written permission of EcoNet and/or its licensors, 
any reproduction, modification, use or disclosure of EcoNet Software, and 
information contained herein, in whole or in part, shall be strictly prohibited.

EcoNet (HK) Limited  EcoNet. ALL RIGHTS RESERVED.

BY OPENING OR USING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY 
ACKNOWLEDGES AND AGREES THAT THE SOFTWARE/FIRMWARE AND ITS 
DOCUMENTATIONS (��ECONET SOFTWARE��) RECEIVED FROM ECONET 
AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON AN ��AS IS�� 
BASIS ONLY. ECONET EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, 
WHETHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, 
OR NON-INFRINGEMENT. NOR DOES ECONET PROVIDE ANY WARRANTY 
WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTIES WHICH 
MAY BE USED BY, INCORPORATED IN, OR SUPPLIED WITH THE ECONET SOFTWARE. 
RECEIVER AGREES TO LOOK ONLY TO SUCH THIRD PARTIES FOR ANY AND ALL 
WARRANTY CLAIMS RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES 
THAT IT IS RECEIVER��S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD 
PARTY ALL PROPER LICENSES CONTAINED IN ECONET SOFTWARE.

ECONET SHALL NOT BE RESPONSIBLE FOR ANY ECONET SOFTWARE RELEASES 
MADE TO RECEIVER��S SPECIFICATION OR CONFORMING TO A PARTICULAR 
STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND 
ECONET'S ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE ECONET 
SOFTWARE RELEASED HEREUNDER SHALL BE, AT ECONET'S SOLE OPTION, TO 
REVISE OR REPLACE THE ECONET SOFTWARE AT ISSUE OR REFUND ANY SOFTWARE 
LICENSE FEES OR SERVICE CHARGES PAID BY RECEIVER TO ECONET FOR SUCH 
ECONET SOFTWARE.
***************************************************************/
#ifndef _ECNT_HOOK_VOIP_H
#define _ECNT_HOOK_VOIP_H

/************************************************************************
*               I N C L U D E S
*************************************************************************
*/

#include <ecnt_hook/ecnt_hook.h>

/************************************************************************
*               D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/
#define ECNT_DRIVER_API  0
#define ECNT_DRIVER_SLT  1
#define ECNT_TASK_CPU_OCCUPANCY_CTRL_API  2

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,4,0)
typedef u64 __nocast cputime_t;
#endif

/************************************************************************
*               M A C R O S
*************************************************************************
*/

/************************************************************************
*               D A T A   T Y P E S
*************************************************************************
*/
typedef enum { 
    VOIP_FUNCTION_SLIC_SHUTDOWN,
    VOIP_FUNCTION_POWER_SAVE_MODE,
    VOIP_FUNCTION_MAX_NUM,
} VOIP_HookFunction_t ;

typedef struct ECNT_VOIP_Data {
	VOIP_HookFunction_t function_id;	/* need put at first item */
	int retValue;

	union {
		int enable;
	};
}ECNT_VOIP_Data_s;

typedef struct ECNT_VOIP_OCCUPANCY_CTRL_Data {
	struct task_struct *pTask;
	cputime_t *uTime;
	cputime_t *sTime;
}ECNT_VOIP_OCCUPANCY_CTRL_Data_s;


/************************************************************************
*               D A T A   D E C L A R A T I O N S
*************************************************************************
*/

/************************************************************************
*               F U N C T I O N   D E C L A R A T I O N S
                I N L I N E  F U N C T I O N  D E F I N I T I O N S
*************************************************************************
*/
static inline int VOIP_API_SLIC_SHUTDOWN(void){
    ECNT_VOIP_Data_s in_data;
    int ret=0;
    in_data.function_id = VOIP_FUNCTION_SLIC_SHUTDOWN;
    ret = __ECNT_HOOK(ECNT_VOIP, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
    if(ret != ECNT_HOOK_ERROR)
        return ECNT_CONTINUE;
    else
        return ECNT_HOOK_ERROR;
}

static inline int VOIP_API_POWER_SAVE_MODE(int enable){
    ECNT_VOIP_Data_s in_data;
    int ret=0;
    in_data.function_id = VOIP_FUNCTION_POWER_SAVE_MODE;
    in_data.enable = enable;

    ret = __ECNT_HOOK(ECNT_VOIP, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
    if(ret != ECNT_HOOK_ERROR)
        return ECNT_CONTINUE;
    else
        return ECNT_HOOK_ERROR;
}

static inline int VOIP_API_PCM_SLT(void){
    ECNT_VOIP_Data_s in_data;
    int ret=0;
    ret = __ECNT_HOOK(ECNT_VOIP, ECNT_DRIVER_SLT, (struct ecnt_data *)&in_data);
    return ret;
}

static inline int VOIP_API_OCCUPANCY_CTRL(struct task_struct *pTask,cputime_t *utime,cputime_t *stime){
    ECNT_VOIP_OCCUPANCY_CTRL_Data_s in_data;
    int ret=0;
    in_data.pTask = pTask;
    in_data.uTime = utime;
    in_data.sTime = stime;
    ret = __ECNT_HOOK(ECNT_VOIP, ECNT_TASK_CPU_OCCUPANCY_CTRL_API, (struct ecnt_data *)&in_data);
    return ret;
}

#endif /*_TCETHERVOIP_HOOK_H*/

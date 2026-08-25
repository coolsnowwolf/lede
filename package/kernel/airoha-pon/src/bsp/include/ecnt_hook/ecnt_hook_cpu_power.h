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

#ifndef _ECNT_HOOK_CPU_POWER_H
#define _ECNT_HOOK_CPU_POWER_H

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

typedef enum {
	CPU_POWER_FUNCTION_UP = 0,
	CPU_POWER_FUNCTION_DOWN,
	CPU_POWER_FUNCTION_GET_IDDQ,
	CPU_POWER_FUNCTION_MAX_NUM
} ECNT_CPU_POWER_Data ;


typedef struct ECNT_CPUPOWER_DATA
{
	ECNT_CPU_POWER_Data function_id;
	int retValue;
}ECNT_CPU_POWER_Data_s;


ecnt_ret_val ecnt_cpu_power_hook(struct ecnt_data *indata);


/************************************************************************
*               D A T A   D E C L A R A T I O N S
*************************************************************************
*/

/************************************************************************
*               F U N C T I O N   D E C L A R A T I O N S
                I N L I N E  F U N C T I O N  D E F I N I T I O N S
*************************************************************************
*/



static inline int TXPOWER_HOOK_CHECK_UP(void)
{
	struct ECNT_CPUPOWER_DATA power_data;
	int ret=0;

	power_data.function_id = CPU_POWER_FUNCTION_UP;

	ret = __ECNT_HOOK(ECNT_CPU_POWER, ECNT_DRIVER_API, (struct ecnt_data *)&power_data);
	if(ret != ECNT_HOOK_ERROR)
		return power_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}


static inline int TXPOWER_HOOK_CHECK_DOWN(void)
{
	struct ECNT_CPUPOWER_DATA power_data;
	int ret=0;

	power_data.function_id = CPU_POWER_FUNCTION_DOWN;

	ret = __ECNT_HOOK(ECNT_CPU_POWER, ECNT_DRIVER_API, (struct ecnt_data *)&power_data);
	if(ret != ECNT_HOOK_ERROR)
		return power_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int TXPOWER_HOOK_GET_IDDQ(void)
{
	struct ECNT_CPUPOWER_DATA power_data;
	int ret=0;

	power_data.function_id = CPU_POWER_FUNCTION_GET_IDDQ;

	ret = __ECNT_HOOK(ECNT_CPU_POWER, ECNT_DRIVER_API, (struct ecnt_data *)&power_data);

	return power_data.retValue;
}

#endif

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

#ifndef __LINUX_ENCT_HOOK_DGASP_H
#define __LINUX_ENCT_HOOK_DGASP_H

/************************************************************************
*               I N C L U D E S
*************************************************************************
*/

#include <ecnt_hook/ecnt_hook.h>

/************************************************************************
*               D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/


#define ECNT_DRIVER_API	0
#define xDSL			0
#define xPON			1

typedef enum {
    DGASP_FUNCTION_OPTIMIZE,
	DGASP_FUNCTION_CONFIG_RESET_VALUE,
    DGASP_FUNCTION_MAX_NUM,
} DGASP_HookFunction_t ;


typedef struct ECNT_DGASP_Data {
	DGASP_HookFunction_t function_id;	/* need put at first item */
	int retValue;
	
	union {
		int type;
		struct {
			unsigned long int value1;
			unsigned long int value2;
		} reset;
	} dgasp_private;
}ECNT_DGASP_Data_s;

/************************************************************************
*               D A T A   D E C L A R A T I O N S
*************************************************************************
*/
	
	

/************************************************************************
*               F U N C T I O N   D E C L A R A T I O N S
                I N L I N E  F U N C T I O N  D E F I N I T I O N S
*************************************************************************
*/
static inline int DGASP_CONFIG_RESET_VALUE(unsigned long int reset1_value, unsigned long int reset2_value)
{
    ECNT_DGASP_Data_s in_data = {0};
    int ret=0;

    in_data.function_id = DGASP_FUNCTION_CONFIG_RESET_VALUE;
	in_data.dgasp_private.reset.value1 = reset1_value;
	in_data.dgasp_private.reset.value2 = reset2_value;
	in_data.retValue = -1;
	
    ret = __ECNT_HOOK(ECNT_DGASP, ECNT_DRIVER_API, (struct ecnt_data *) (&in_data));
	if (ret == ECNT_CONTINUE)
	{
		return in_data.retValue;
	}
	else
	{
		return ECNT_HOOK_ERROR;
	}
}

static inline int DGASP_OPTIMIZE_PON(void)
{
    ECNT_DGASP_Data_s in_data = {0};
    int ret=0;

    in_data.function_id = DGASP_FUNCTION_OPTIMIZE;
	in_data.dgasp_private.type = xPON;
	in_data.retValue = -1;
	
    ret = __ECNT_HOOK(ECNT_DGASP, ECNT_DRIVER_API, (struct ecnt_data *) (&in_data));
	if (ret == ECNT_CONTINUE)
	{
		return in_data.retValue;
	}
	else
	{
		return ECNT_HOOK_ERROR;
	}
}

static inline int DGASP_OPTIMIZE_DSL(void)
{
    ECNT_DGASP_Data_s in_data = {0};
    int ret=0;

    in_data.function_id = DGASP_FUNCTION_OPTIMIZE;
	in_data.dgasp_private.type = xDSL;
	in_data.retValue = -1;
	
    ret = __ECNT_HOOK(ECNT_DGASP, ECNT_DRIVER_API, (struct ecnt_data *) (&in_data));
	if (ret == ECNT_CONTINUE)
	{
		return in_data.retValue;
	}
	else
	{
		return ECNT_HOOK_ERROR;
	}
}

#endif

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
#ifndef _ECNT_HOOK_AIR_EN8811_H_
#define _ECNT_HOOK_AIR_EN8811_H_

#include "ecnt_hook_air_en8811_type.h"

/************************************************************************
*                  I N C L U D E S
*************************************************************************
*/	

/************************************************************************
*                  D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/


/************************************************************************
*                  M A C R O S
*************************************************************************
*/


/************************************************************************
*                  D A T A	 T Y P E S
*************************************************************************
*/

/************************************************************************
*                  D A T A   D E C L A R A T I O N S
*************************************************************************
*/

/************************************************************************
*                  F U N C T I O N    D E C L A R A T I O N S
#                  I N L I N E    F U N C T I O N   D E F I N I T I O N S
*************************************************************************
*/
/*Use the default value for a single phy, for compatibility. */
static inline int AIR_EN8811_API_POWER_UP(int power_up)
{

	int ret = 0;
	ECNT_AIR_EN8811_Data_s in_data;

	in_data.function_id = AIR_EN8811_FUNCTION_POWER_UP;
	in_data.air_en8811_private.power_up = power_up;
        
    ret = __ECNT_HOOK(ECNT_AIR_EN8811,ECNT_DRIVER_API,(struct ecnt_data *)&in_data);

	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
	
}

/*Use the default value for a single phy, for compatibility. */
static inline int AIR_EN8811_GET_LINK_RATE(uint *speed_index, uint *duplex)
{
	int ret = 0;
	ECNT_AIR_EN8811_Data_s in_data;
	memset(&in_data, 0, sizeof(ECNT_AIR_EN8811_Data_s));

	in_data.function_id = AIR_EN8811_FUNCTION_LINK_RATE;      
    ret = __ECNT_HOOK(ECNT_AIR_EN8811,ECNT_DRIVER_API,(struct ecnt_data *)&in_data);

	*speed_index = in_data.air_en8811_private.link_info.speed_index;
	*duplex = in_data.air_en8811_private.link_info.duplex;

	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int EN8811_MULTI_SET_POWER_UP(uint serdes_id, int power_up)
{

    int ret = 0;
    ECNT_AIR_EN8811_Data_s in_data;

    in_data.function_id = AIR_EN8811_FUNCTION_MULTI_POWER_UP;
    in_data.serdes_id = serdes_id;
    in_data.air_en8811_private.power_up = power_up;
        
    ret = __ECNT_HOOK(ECNT_AIR_EN8811,ECNT_DRIVER_API,(struct ecnt_data *)&in_data);

    if(ret != ECNT_HOOK_ERROR)
        return in_data.retValue;
    else
        return ECNT_HOOK_ERROR;
    
}

static inline int EN8811_MULTI_GET_LINK_RATE(uint serdes_id, uint *speed_index, uint *duplex)
{
    int ret = 0;
    ECNT_AIR_EN8811_Data_s in_data;
    memset(&in_data, 0, sizeof(ECNT_AIR_EN8811_Data_s));

    in_data.serdes_id = serdes_id;
    in_data.function_id = AIR_EN8811_FUNCTION_MULTI_LINK_RATE;      
    ret = __ECNT_HOOK(ECNT_AIR_EN8811,ECNT_DRIVER_API,(struct ecnt_data *)&in_data);

    *speed_index = in_data.air_en8811_private.link_info.speed_index;
    *duplex = in_data.air_en8811_private.link_info.duplex;

    if(ret != ECNT_HOOK_ERROR)
        return in_data.retValue;
    else
        return ECNT_HOOK_ERROR;
}

static inline int EN8811_MULTI_GET_LINK_STATUS(uint serdes_id, bool *an, bool *link_status)
{
	int ret = 0;
	ECNT_AIR_EN8811_Data_s in_data;
	memset(&in_data, 0, sizeof(ECNT_AIR_EN8811_Data_s));
	in_data.serdes_id = serdes_id;
	in_data.function_id = AIR_EN8811_FUNCTION_LINK_STATUS;      
    ret = __ECNT_HOOK(ECNT_AIR_EN8811,ECNT_DRIVER_API,(struct ecnt_data *)&in_data);

	*an = in_data.air_en8811_private.link_info.autoneg_state;
	*link_status = in_data.air_en8811_private.link_info.link_state;
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

#endif /* _ECNT_HOOK_AIR_EN8811_H_ */


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
#ifndef _ECNT_HOOK_PCIEE_H_
#define _ECNT_HOOK_PCIE_H_


#include "ecnt_hook.h"
#include "ecnt_hook_pcie_type.h"

/************************************************************************
*                  D E F I N E S   &   C O N S T A N T S
************************************************************************/

/************************************************************************
*                  M A C R O S
************************************************************************/

/************************************************************************
*                  D A T A	 T Y P E S
************************************************************************/

/************************************************************************
*                  E X T E R N A L	 D A T A   D E C L A R A T I O N S
************************************************************************/

/************************************************************************
*                  F U N C T I O N   D E F I N I T I O N S
************************************************************************/

/************************************************************************
*                  P U B L I C   D A T A
************************************************************************/

/************************************************************************
*                  P R I V A T E   D A T A
************************************************************************/

/************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
************************************************************************/

static inline int PCIE_API_GET_CONFREG(int idx, unsigned  int offset, unsigned int* value)
{
	struct ecnt_pcie_data in_data; 
	int ret = 0;
	
	in_data.function_id = PCIE_GET_CONFREG; 
	in_data.idx = idx;
	in_data.conf.off = offset;
	ret = __ECNT_HOOK(ECNT_PCIE, ECNT_PCIE_API, (struct ecnt_data *)&in_data);
	*value = in_data.retValue;
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;

}


static inline int PCIE_API_SET_CONFREG(int idx, unsigned int offset,unsigned int value)
{
	struct ecnt_pcie_data in_data; 
	int ret = 0;
	
	in_data.function_id = PCIE_SET_CONFREG; 
	in_data.idx = idx;
	in_data.conf.off = offset;
	in_data.conf.val = value;
	ret = __ECNT_HOOK(ECNT_PCIE, ECNT_PCIE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;

}

static inline int PCIE_API_GET_ASPM(int idx, unsigned int* value)
{
	struct ecnt_pcie_data in_data; 
	int ret = 0;
	
	in_data.function_id = PCIE_GET_ASPM; 
	in_data.idx = idx;
	ret = __ECNT_HOOK(ECNT_PCIE, ECNT_PCIE_API, (struct ecnt_data *)&in_data);
	*value = in_data.retValue;
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;

}


static inline int PCIE_API_SET_ASPM(int idx, unsigned int value)
{
	struct ecnt_pcie_data in_data; 
	int ret = 0;
	
	in_data.function_id = PCIE_SET_ASPM; 
	in_data.idx = idx;
	in_data.conf.val = value;
	ret = __ECNT_HOOK(ECNT_PCIE, ECNT_PCIE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;

}

static inline int PCIE_API_GET_SPEED(int idx,unsigned int* value)
{
	struct ecnt_pcie_data in_data; 
	int ret = 0;
	
	in_data.function_id = PCIE_GET_SPEED; 
	in_data.idx = idx;
	ret = __ECNT_HOOK(ECNT_PCIE, ECNT_PCIE_API, (struct ecnt_data *)&in_data);
	*value = in_data.retValue;
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int PCIE_API_SET_SPEED(int idx,unsigned int value)
{
	struct ecnt_pcie_data in_data; 
	int ret = 0;
	
	in_data.function_id = PCIE_SET_SPEED; 
	in_data.idx = idx;
	in_data.conf.val = value;
	ret = __ECNT_HOOK(ECNT_PCIE, ECNT_PCIE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int PCIE_API_GET_COUNT(int idx, struct ecnt_pcie_count_data* pcnt)
{
	struct ecnt_pcie_data in_data; 
	int ret = 0;
	
	in_data.function_id = PCIE_GET_COUNT; 
	in_data.idx = idx;
	ret = __ECNT_HOOK(ECNT_PCIE, ECNT_PCIE_API, (struct ecnt_data *)&in_data);
	memcpy(pcnt,&in_data.cnt,sizeof (struct ecnt_pcie_count_data));
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;

}

static inline int PCIE_API_GET_LINKSTATE(int idx,unsigned int* value)
{
	struct ecnt_pcie_data in_data; 
	int ret = 0;
	
	in_data.function_id = PCIE_GET_LINKSTATE; 
	in_data.idx = idx;
	ret = __ECNT_HOOK(ECNT_PCIE, ECNT_PCIE_API, (struct ecnt_data *)&in_data);
	*value = in_data.retValue;
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int PCIE_API_SET_PHY_DBG(int idx,unsigned int* value)
{
	struct ecnt_pcie_data in_data; 
	int ret = 0;
	
	in_data.function_id = PCIE_SET_PHY_DBG; 
	in_data.idx = idx;
	in_data.conf.val = *value;
	ret = __ECNT_HOOK(ECNT_PCIE, ECNT_PCIE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;

}

static inline int PCIE_API_SET_POWER(int idx,unsigned int* value)
{
	struct ecnt_pcie_data in_data; 
	int ret = 0;
	
	in_data.function_id = PCIE_SET_POWER; 
	in_data.idx = idx;
	in_data.conf.val = *value;
	ret = __ECNT_HOOK(ECNT_PCIE, ECNT_PCIE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;

}

static inline int PCIE_AUTOBENCH_LOOPBACK(void)
{

	struct ecnt_pcie_data in_data;
	int ret=0;

	in_data.function_id = PCIE_FUNCTION_AUTOBENCH_LOOPBACK;

	ret = __ECNT_HOOK(ECNT_PCIE, ECNT_PCIE_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int PCIE_API_GET_DEVICEID(int idx,unsigned int* value)
{

	struct ecnt_pcie_data in_data;
	int ret=0;

	in_data.function_id = PCIE_GET_CHIPID;
	in_data.idx = idx;

	ret = __ECNT_HOOK(ECNT_PCIE, ECNT_PCIE_API, (struct ecnt_data *)&in_data);
	*value = in_data.retValue;
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

#endif /* _ECNT_HOOK_FE_H_ */



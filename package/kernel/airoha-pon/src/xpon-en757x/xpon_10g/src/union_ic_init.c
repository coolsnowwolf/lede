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
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/types.h>
#include "common/union_ic_init.h"
#include "epon/epon.h"


union_action_func union_ic_func[MAX_ACTION_NUM];
uint32_t g_epon_llid_dscv_stat[32] = {0};
reg_check_t *epon_reg =NULL;
reg_ind_check_t *indirect_reg = NULL;



int init_union_ic_function(void)
{
	int i = 0;
	
#if defined(TCSUPPORT_CPU_EN7581)
	ic_func_tbl_t *action = an7581_action;
	epon_reg = an7581_epon_reg;
	indirect_reg = an7581_indirect_reg;
#elif defined(TCSUPPORT_CPU_EN7580)
	ic_func_tbl_t *action = en7580_action;
	epon_reg = en7580_epon_reg;
	indirect_reg = NULL;
#else
	ic_func_tbl_t *action = NULL;
	epon_reg = NULL;
	indirect_reg = NULL;
#endif

	if(action == NULL)
	{
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"[%s]: action function table is NULL\n", __FUNCTION__);
		return -1;
	}

	for(i = 0; i < MAX_ACTION_NUM; i++)
	{
		union_ic_func[i] = NULL;
	}
	for(i = 0; i < MAX_ACTION_NUM; i++)
	{
		if(action[i].func_id == MAX_ACTION_NUM)
			break;
		union_ic_func[action[i].func_id]=action[i].func;
	}

	return 0;
}

int UNION_IC_FUNCTION_HOOK(UNION_IC_ACTION action_idx, const void *in, void *out)
{
	if(union_ic_func[action_idx] != NULL)
		return union_ic_func[action_idx](in,out);
	return EPON_SUCCESS;
}

void dump_pon_type_mode_info(PON_OnuType_t onu_type, XMCSIF_WanDetectionMode_t onu_mode)
{
	/*
	*   bit [1:0]: 0x0 unknow type, 0x1: SFU Mode, 0x2: HGU Mode
	*   bit 2: 0x0 not Combo Pon mode, 0x1: Combo Pon Mode
	*   bit 3: 0x0 not bbf247 mode, 0x1: bbf247 Mode
	*	bit [7:4]: 0x0 preversion ver or auto mode. 0x1: GPON, 0x2: EPON,0x3: XE-PON-Asym
	*		  0x4: XE-PON-Sym, 0x5: EPON for 10G MAC, 0x6: XG-PON, 0x7: XGS-PON,
	*	         0x8: NG-PON2 10G_10G, 0x9: NG-PON2 10G_2G, 0xA, NG-PON2 2G_2G.
	*		 reference XMCSIF_WanDetectionMode_t
	*/
	const char *mode_info[XMCS_IF_WAN_DETECT_MODE_MAX]={"auto","GPON","EPON","XEPON-ASYM","XEPON-SYM","XEPON_1G",
														"XGPON","XGSPON","NGPON2-10G_10G","NGPON2-10G_2G","NGPON2-2G_2G",
														"GPON-SYM","TURBO-EPON"};
	const char *type_info[3]={"unknown","SFU","HGU"};
	if(onu_type < 3){
		printk("**********PON MAC GET ONU_TYPE = %s**********\n", type_info[onu_type]);
	}else{
		printk("**********PON MAC GET ONU_TYPE = Unknown[type=%d]**********\n", onu_type);
	}

	if(onu_mode < XMCS_IF_WAN_DETECT_MODE_MAX){
		printk("**********PON MAC GET ONU_MODE = %s**********\n", mode_info[onu_mode]);
	}else{
		printk("**********PON MAC GET ONU_MODE = Unknown[mode=%d]**********\n", onu_mode);
	}
	
}


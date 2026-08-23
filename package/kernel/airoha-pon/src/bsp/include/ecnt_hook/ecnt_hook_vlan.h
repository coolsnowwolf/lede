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

#ifndef __LINUX_ENCT_HOOK_FH_VLAN_H
#define __LINUX_ENCT_HOOK_FH_VLAN_H

/************************************************************************
*               I N C L U D E S
*************************************************************************
*/

#include <ecnt_hook/ecnt_hook.h>
#include <linux/skbuff.h>

/************************************************************************
*               D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/


#define ECNT_DRIVER_API  0

/* Warning: same sequence with function array 'qdma_operation' */
typedef enum { 
    FH_FUNCTION_VLAN_FILTER_HANDLE,
    FH_FUNCTION_VLAN_TX_TRANS,
    FH_FUNCTION_VLAN_TX_PROC,
    FH_FUNCTION_VLAN_RX_PROC,
    FH_FUNCTION_DOUBLE_VLAN_RX_PROC,
    FH_FUNCTION_VLAN_PARSER,
    FH_FUNCTION_VLAN_OPERATION,
    FH_FUNCTION_DSCP_OPERATION,
    FH_FUNCTION_EWAN_VLAN_HANDLE,
    FH_FUNCTION_MAX_NUM,
} FH_VLAN_HookFunction_t ;

typedef struct ECNT_FH_VLAN_Data {
	FH_VLAN_HookFunction_t function_id;	/* need put at first item */
	int retValue;
	
	union {
		struct sk_buff *skb;
		struct{
			struct sk_buff *skb;
			int type;
		}handle;
	}private;
}ECNT_FH_VLAN_Data_s, *ECNT_FH_VLAN_Data_ptr;



/************************************************************************
*               D A T A   D E C L A R A T I O N S
*************************************************************************
*/
	
	

/************************************************************************
*               F U N C T I O N   D E C L A R A T I O N S
                I N L I N E  F U N C T I O N  D E F I N I T I O N S
*************************************************************************
*/
static inline int FH_VLAN_FILTER(struct sk_buff *skb){
    ECNT_FH_VLAN_Data_s in_data = {0};
    int ret=0;

    in_data.function_id = FH_FUNCTION_VLAN_FILTER_HANDLE;
	in_data.private.skb = skb;
	in_data.retValue = -1;
	
	//printk("[%s][%d]before function_id:%d, in_data.retValue:%d\n", __FUNCTION__, __LINE__, in_data.function_id, in_data.retValue);
    ret = __ECNT_HOOK(ECNT_VLAN, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret == ECNT_CONTINUE){
		//printk("[%s][%d]function_id:%d, in_data.retValue:%d\n", __FUNCTION__, __LINE__, in_data.function_id, in_data.retValue);
		return in_data.retValue;
	}
	else{
		return 0;
	}
}

static inline int FH_VLAN_TX_TRANS(struct sk_buff *skb){
    ECNT_FH_VLAN_Data_s in_data = {0};
    int ret=0;

    in_data.function_id = FH_FUNCTION_VLAN_TX_TRANS;
	in_data.private.skb = skb;
	in_data.retValue = -1;
	
	//printk("[%s][%d]before function_id:%d, in_data.retValue:%d\n", __FUNCTION__, __LINE__, in_data.function_id, in_data.retValue);
    ret = __ECNT_HOOK(ECNT_VLAN, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret == ECNT_CONTINUE){
		//printk("[%s][%d]function_id:%d, in_data.retValue:%d\n", __FUNCTION__, __LINE__, in_data.function_id, in_data.retValue);
		return in_data.retValue;
	}
	else{
		return 0;
	}
}

static inline int FH_VLAN_TX_PROC(struct sk_buff *skb){
    ECNT_FH_VLAN_Data_s in_data = {0};
    int ret=0;

    in_data.function_id = FH_FUNCTION_VLAN_TX_PROC;
	in_data.private.skb = skb;
	in_data.retValue = -1;
	
	//printk("[%s][%d]before function_id:%d, in_data.retValue:%d\n", __FUNCTION__, __LINE__, in_data.function_id, in_data.retValue);
    ret = __ECNT_HOOK(ECNT_VLAN, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret == ECNT_CONTINUE){
		//printk("[%s][%d]function_id:%d, in_data.retValue:%d\n", __FUNCTION__, __LINE__, in_data.function_id, in_data.retValue);
		return in_data.retValue;
	}
	else{
		return 0;
	}
}

static inline int FH_VLAN_RX_PROC(struct sk_buff *skb){
    ECNT_FH_VLAN_Data_s in_data = {0};
    int ret=0;

    in_data.function_id = FH_FUNCTION_VLAN_RX_PROC;
	in_data.private.skb = skb;
	in_data.retValue = -1;
	
	//printk("[%s][%d]before function_id:%d, in_data.retValue:%d\n", __FUNCTION__, __LINE__, in_data.function_id, in_data.retValue);
    ret = __ECNT_HOOK(ECNT_VLAN, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret == ECNT_CONTINUE){
		//printk("[%s][%d]function_id:%d, in_data.retValue:%d\n", __FUNCTION__, __LINE__, in_data.function_id, in_data.retValue);
		return in_data.retValue;
	}
	else{
		return 0;
	}
}

static inline int FH_DOUBLE_VLAN_RX_PROC(struct sk_buff *skb){
    ECNT_FH_VLAN_Data_s in_data = {0};
    int ret=0;

    in_data.function_id = FH_FUNCTION_DOUBLE_VLAN_RX_PROC;
	in_data.private.skb = skb;
	in_data.retValue = -1;
	
	//printk("[%s][%d]before function_id:%d, in_data.retValue:%d\n", __FUNCTION__, __LINE__, in_data.function_id, in_data.retValue);
    ret = __ECNT_HOOK(ECNT_VLAN, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret == ECNT_CONTINUE){
		//printk("[%s][%d]function_id:%d, in_data.retValue:%d\n", __FUNCTION__, __LINE__, in_data.function_id, in_data.retValue);
		return in_data.retValue;
	}
	else{
		return 0;
	}
}
	
static inline int FH_VLAN_PARSER(struct sk_buff *skb){
	ECNT_FH_VLAN_Data_s in_data = {0};
	int ret=0;

	in_data.function_id = FH_FUNCTION_VLAN_PARSER;
	in_data.private.skb = skb;
	in_data.retValue = -1;
	
	//printk("[%s][%d]before function_id:%d, in_data.retValue:%d\n", __FUNCTION__, __LINE__, in_data.function_id, in_data.retValue);
	ret = __ECNT_HOOK(ECNT_VLAN, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret == ECNT_CONTINUE){
		//printk("[%s][%d]function_id:%d, in_data.retValue:%d\n", __FUNCTION__, __LINE__, in_data.function_id, in_data.retValue);
		return in_data.retValue;
	}
	else{
		return 0;
	}
}

static inline struct sk_buff *FH_DSCP_OPERATION(struct sk_buff *skb){
	ECNT_FH_VLAN_Data_s in_data = {0};
	int ret=0;

	in_data.function_id = FH_FUNCTION_DSCP_OPERATION;
	in_data.private.skb = skb;
	in_data.retValue = -1;
	
	//printk("[%s][%d]before function_id:%d, in_data.retValue:%d\n", __FUNCTION__, __LINE__, in_data.function_id, in_data.retValue);
	ret = __ECNT_HOOK(ECNT_VLAN, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret == ECNT_CONTINUE){
		//printk("[%s][%d]function_id:%d, in_data.retValue:%d\n", __FUNCTION__, __LINE__, in_data.function_id, in_data.retValue);
		return in_data.private.skb;
	}
	else{
		return skb;
	}
}

static inline struct sk_buff *FH_VLAN_OPERATION(struct sk_buff *skb){
	ECNT_FH_VLAN_Data_s in_data = {0};
	int ret=0;

	in_data.function_id = FH_FUNCTION_VLAN_OPERATION;
	in_data.private.skb = skb;
	in_data.retValue = -1;
	
	//printk("[%s][%d]before function_id:%d, in_data.retValue:%d\n", __FUNCTION__, __LINE__, in_data.function_id, in_data.retValue);
	ret = __ECNT_HOOK(ECNT_VLAN, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret == ECNT_CONTINUE){
		//printk("[%s][%d]function_id:%d, in_data.retValue:%d\n", __FUNCTION__, __LINE__, in_data.function_id, in_data.retValue);
		return in_data.private.skb;
	}
	else{
		return skb;
	}
}
	
static inline int FH_EWAN_VLAN_HANDLE(struct sk_buff *skb, int type){
	ECNT_FH_VLAN_Data_s in_data = {0};
	int ret=0;

	in_data.function_id = FH_FUNCTION_EWAN_VLAN_HANDLE;
	in_data.private.handle.skb = skb;
	in_data.private.handle.type = type;
	in_data.retValue = 0;
	
	//printk("[%s][%d]before function_id:%d, in_data.retValue:%d\n", __FUNCTION__, __LINE__, in_data.function_id, in_data.retValue);
	ret = __ECNT_HOOK(ECNT_VLAN, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret == ECNT_CONTINUE){
		//printk("[%s][%d]function_id:%d, in_data.retValue:%d\n", __FUNCTION__, __LINE__, in_data.function_id, in_data.retValue);
		return in_data.retValue;
	}
	else{
		return 0;
	}
}

static inline int FH_EN7523_VLAN_FILTER(struct sk_buff *skb){
    ECNT_FH_VLAN_Data_s in_data = {0};
    int ret=0;

    in_data.function_id = FH_FUNCTION_VLAN_FILTER_HANDLE;
	in_data.private.skb = skb;
	in_data.retValue = -1;
	
    ret = __ECNT_HOOK(ECNT_VLAN_FILTER, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret == ECNT_CONTINUE){
		return in_data.retValue;
	}
	else{
		return 0;
	}
}
#endif

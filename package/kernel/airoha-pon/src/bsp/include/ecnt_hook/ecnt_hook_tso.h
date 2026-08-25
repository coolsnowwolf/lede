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

#ifndef __LINUX_ENCT_HOOK_TSO_H
#define __LINUX_ENCT_HOOK_TSO_H

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

/* Warning: same sequence with function array 'qdma_operation' */
typedef enum { 
	TSO_FUNCTION_MATCH_V4,
	TSO_FUNCTION_MATCH_V6,
	TSO_FUNCTION_TSO_LOCALOUT,
	TSO_FUNCTION_TSO_PINGPONG_V4,
	TSO_FUNCTION_SESSION_DESTROY_V4,
	TSO_FUNCTION_TRANSMIT_PACKET,
	TSO_FUNCTION_TDMA_DMA_CONFIG,
	TSO_FUNCTION_TDMA_DMA_ENABLE,
	TSO_FUNCTION_MAX_NUM,
} TSO_HookFunction_t ;

/*
	tsoRuleEntry_t is used for white list entry.
*/
typedef struct{
	__be16 sport;
	__be16 dport;
	__be32 saddr_v4;
	__be32 daddr_v4;
}tsoRuleEntry_v4_t;

typedef struct{
	__be16 sport;
	__be16 dport;
	struct in6_addr saddr_v6;
	struct in6_addr daddr_v6;
}tsoRuleEntry_v6_t;

typedef struct{
	tsoRuleEntry_v4_t *flow;
	void* tso_skb;
}tsoPingPong_v4_t;

typedef struct{
	u_int32_t mainType;
	struct sk_buff *skb;
	u_int32_t msg0;
	u_int32_t msg1;
	u_int32_t txRing;
}tsoTransInfo_t;

typedef struct{
	unchar *dataP;
	int dataLen;
	struct sk_buff *skb;
	unchar rxDone;
}tsoTransTestInfo_t;
typedef struct ECNT_TSO_Data {
	TSO_HookFunction_t function_id;	/* need put at first item */
	int retValue;
	
	union {
		tsoRuleEntry_v4_t *entry_v4;
		tsoRuleEntry_v6_t *entry_v6;
		struct sk_buff * tso_skb;
		tsoPingPong_v4_t pingpong_v4;
		struct sock *sk;
		tsoTransInfo_t   	*transInfo;
		tsoTransTestInfo_t	*dramTestCfgPtr;
	} tso_private;
}ECNT_TSO_Data_s;



/************************************************************************
*               D A T A   D E C L A R A T I O N S
*************************************************************************
*/
	
	

/************************************************************************
*               F U N C T I O N   D E C L A R A T I O N S
                I N L I N E  F U N C T I O N  D E F I N I T I O N S
*************************************************************************
*/
static inline int TSO_SET_SKB_MARK_V4(tsoRuleEntry_v4_t *entry){
	ECNT_TSO_Data_s in_data = {0};
	int ret=0;

	in_data.function_id = TSO_FUNCTION_MATCH_V4;
	in_data.tso_private.entry_v4 = entry;
	in_data.retValue = -1;
	
	//printk("before function_id:%d, in_data.retValue:%d\n", in_data.retValue);
	ret = __ECNT_HOOK(ECNT_TSO_LAN, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret == ECNT_CONTINUE){
		//printk("function_id:%d, in_data.retValue:%d\n", in_data.retValue);
		return in_data.retValue;
	}
	else{
		return ECNT_HOOK_ERROR;
	}
}

static inline int TSO_SET_SKB_MARK_V6(tsoRuleEntry_v6_t *entry){
	ECNT_TSO_Data_s in_data = {0};
	int ret=0;

	in_data.function_id = TSO_FUNCTION_MATCH_V6;
	in_data.tso_private.entry_v6 = entry;
	ret = __ECNT_HOOK(ECNT_TSO_LAN, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);

	return ret;
}

static inline int TSO_SHORTCUT_TO_PPE(struct sk_buff *skb){
	ECNT_TSO_Data_s in_data = {0};
	int ret=0;

	in_data.function_id = TSO_FUNCTION_TSO_LOCALOUT;
	in_data.tso_private.tso_skb = skb;
	in_data.retValue = -1;
	
	ret = __ECNT_HOOK(ECNT_TSO_LAN, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret == ECNT_CONTINUE){
		return in_data.retValue;
	}
	else{
		return ECNT_HOOK_ERROR;
	}
}

static inline int TSO_SEND_PINGPONG_V4(tsoRuleEntry_v4_t *entry, struct sk_buff *skb){
	ECNT_TSO_Data_s in_data = {0};
	int ret=0;

	in_data.function_id = TSO_FUNCTION_TSO_PINGPONG_V4;
	in_data.tso_private.pingpong_v4.flow = entry;
	in_data.tso_private.pingpong_v4.tso_skb = skb;
	in_data.retValue = -1;
	
	ret = __ECNT_HOOK(ECNT_TSO_LAN, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret == ECNT_CONTINUE){
		return in_data.retValue;
	}
	else{
		return ECNT_HOOK_ERROR;
	}
}

static inline int TSO_SESSION_DESTROY_V4(struct sock *sk){
	ECNT_TSO_Data_s in_data = {0};
	int ret=0;

	in_data.function_id = TSO_FUNCTION_SESSION_DESTROY_V4;
	in_data.tso_private.sk = sk;
	in_data.retValue = -1;

	ret = __ECNT_HOOK(ECNT_TSO_LAN, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	if(ret == ECNT_CONTINUE){
		return in_data.retValue;
	}
	else{
		return ECNT_HOOK_ERROR;
	}
}

static inline int TSO_TRANSMIT_PACKET(int mainType,struct sk_buff *skb,u_int32_t msg0,u_int32_t msg1,u_int32_t ringIdx){
	ECNT_TSO_Data_s in_data = {0};
	tsoTransInfo_t transInfo;
	int ret = 0;

	transInfo.mainType = mainType;
	transInfo.msg0 = msg0;
	transInfo.msg1 = msg1;
	transInfo.skb = skb;
	transInfo.txRing = ringIdx;

	in_data.function_id = TSO_FUNCTION_TRANSMIT_PACKET;
	in_data.tso_private.transInfo = &transInfo;
	ret = __ECNT_HOOK(ECNT_TSO_LAN, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);
	return ret;
}


static inline int TSO_TDMA_DMA_CONFIG(tsoTransTestInfo_t *dramTestCfgPtr){
	ECNT_TSO_Data_s in_data = {0};
	int ret=0;

	in_data.function_id = TSO_FUNCTION_TDMA_DMA_CONFIG;
	in_data.tso_private.dramTestCfgPtr = dramTestCfgPtr;
	ret = __ECNT_HOOK(ECNT_TSO_LAN, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);

	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
} 

static inline int TSO_TDMA_DMA_ENABLE(void){
	ECNT_TSO_Data_s in_data = {0};
	int ret=0;

	in_data.function_id = TSO_FUNCTION_TDMA_DMA_ENABLE;
	ret = __ECNT_HOOK(ECNT_TSO_LAN, ECNT_DRIVER_API, (struct ecnt_data *)&in_data);

	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
} 

#endif

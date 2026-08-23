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
#ifndef _ECNT_HOOK_ASYM_H
#define _ECNT_HOOK_ASYM_H

/************************************************************************
*               I N C L U D E S
*************************************************************************
*/
#include <ecnt_hook/ecnt_hook.h>



/************************************************************************
*               D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/


/************************************************************************
*               M A C R O S
*************************************************************************
*/
enum ECNT_ASYM_PKT_PHASE
{
	ECNT_ASYM_PKT_PHASE_SW_U = 0,
	ECNT_ASYM_PKT_PHASE_HW_U,
	ECNT_ASYM_PKT_PHASE_SW_D,
	ECNT_ASYM_PKT_PHASE_HW_D,
};

#define ECNT_ASYM_MARK_HANDLER_DONE (1<<31)
#define ECNT_ASYM_MARK_KA (1<<30)
#define ECNT_ASYM_MARK_BINDED (1<<29)

#define ECNT_ASYM_MARK_FOE_INDEX_MASK 0x3fff /*bit0~13*/

/************************************************************************
*               D A T A   T Y P E S
*************************************************************************
*/
enum ECNT_ASYM_SUBTYPE
{
    ECNT_ASYM_API,
};

typedef enum {
	ECNT_ASYM_PKT_HANDLER = 0,
	ECNT_ASYM_KA_HANDLER,
	ECNT_ASYM_FUNCTION_MAX_NUM,
} ECNT_ASYM_HookFunctionID_t ;

typedef enum{
	ECNT_ASYM_KA_POINT_LAN_PINGPONG = 0,
	ECNT_ASYM_KA_POINT_LAN_BIND,
	ECNT_ASYM_KA_POINT_WLAN,
}ECNT_ASYM_KA_POINT;

struct ecnt_asym_data_s
{
	ECNT_ASYM_HookFunctionID_t function_id;
	union
	{		
		struct 
		{
			struct sk_buff *skb;
			int* dir;
		}pkt_dir;
		struct 
		{
			struct sk_buff** pskb;
			int phase;
		}pkt_handler;
		struct
		{
			struct sk_buff* skb;
			int ka_point;
		}ka_handler;
	}api_data;
	int retValue;
};

/************************************************************************
*               D A T A   D E C L A R A T I O N S
*************************************************************************
*/


/************************************************************************
*               F U N C T I O N   D E C L A R A T I O N S
                I N L I N E  F U N C T I O N  D E F I N I T I O N S
*************************************************************************
*/
static inline int ECNT_ASYM_API_PKT_HANDLER(struct sk_buff** pskb,int phase) {
	struct ecnt_asym_data_s in_data; 
	int ret = 0;
	
	in_data.function_id = ECNT_ASYM_PKT_HANDLER; 
	in_data.api_data.pkt_handler.pskb = pskb;
	in_data.api_data.pkt_handler.phase = phase;
	ret = __ECNT_HOOK(ECNT_ASYM, ECNT_ASYM_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

static inline int ECNT_ASYM_API_KA_HANDLER(struct sk_buff* skb,int ka_point) {
	struct ecnt_asym_data_s in_data; 
	int ret = 0;
	
	in_data.function_id = ECNT_ASYM_KA_HANDLER; 
	in_data.api_data.ka_handler.skb = skb;	
	in_data.api_data.ka_handler.ka_point = ka_point;
	ret = __ECNT_HOOK(ECNT_ASYM, ECNT_ASYM_API, (struct ecnt_data *)&in_data);
	if(ret != ECNT_HOOK_ERROR)
		return in_data.retValue;
	else
		return ECNT_HOOK_ERROR;
}

#endif /* _ECNT_HOOK_L2TP_H */

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
#ifndef _XPON_API_H_
#define _XPON_API_H_


/************************************************************************
*                  I N C L U D E S
*************************************************************************
*/
#include <linux/skbuff.h>

/************************************************************************
*                  D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/

/************************************************************************
*                  M A C R O S
*************************************************************************
*/    




/************************************************************************
*                  D A T A     T Y P E S
*************************************************************************
*/



/***************************************
 struct definition
***************************************/

typedef struct {
	/* customer callbacks. */
	void (*multicastRxDataCallbackFunction)(struct sk_buff*) ;
	void (*phyEventNotifyCallbackFunction)(XMCS_EventType_t, uint);
	uint (*xmcsEventCheckValidCallbackFunction)(XMCS_EventType_t, uint, uint);
	int (*xponSetEmergenceStateCallbackFunction)(unchar);
	unchar (*fhTxPowerHandleCallbackFunction)(unchar);
	int (*xponSetOnuIdCallbackFunction)(ushort);
	int (*xponSetOmccIdCallbackFunction)(uint);
	void (*setPhyEventHandlerCallbackFunction)(int);
} XPON_Customer_hooks_t ;

/************************************************************************
*                  D A T A   D E C L A R A T I O N S
*************************************************************************
*/

extern XPON_Customer_hooks_t xpon_hooks;

/************************************************************************
*                  F U N C T I O N    D E C L A R A T I O N S
#                  I N L I N E    F U N C T I O N   D E F I N I T I O N S
*************************************************************************
*/

#define CALL_USER_HOOK_MULTICAST_RX_DATA(skb) \
if(xpon_hooks.multicastRxDataCallbackFunction){ \
	xpon_hooks.multicastRxDataCallbackFunction(skb); \
}

#define CALL_USER_HOOK_PHY_EVENT_NOTIFY(type, id) \
if(xpon_hooks.phyEventNotifyCallbackFunction){ \
	xpon_hooks.phyEventNotifyCallbackFunction(type, id); \
}

#define CALL_USER_HOOK_XMCS_EVENT_CHECK_VAILD(type, id, value) \
({\
int __ret = 0;\
if(xpon_hooks.xmcsEventCheckValidCallbackFunction){ \
	__ret = xpon_hooks.xmcsEventCheckValidCallbackFunction(type, id, value); \
}\
else{\
	__ret = 1;\
}\
__ret;\
})


#define CALL_USER_HOOK_SET_EMERGENCE_STATE(emergence) \
if(xpon_hooks.xponSetEmergenceStateCallbackFunction){ \
	xpon_hooks.xponSetEmergenceStateCallbackFunction(emergence); \
}

#define CALL_USER_HOOK_FH_TX_POWER_HANDLE(opt) \
if(xpon_hooks.fhTxPowerHandleCallbackFunction){ \
	xpon_hooks.fhTxPowerHandleCallbackFunction(opt); \
}

#define CALL_USER_HOOK_SET_ONU_ID(onuId) \
if(xpon_hooks.xponSetOnuIdCallbackFunction){ \
	xpon_hooks.xponSetOnuIdCallbackFunction(onuId); \
}

#define CALL_USER_HOOK_SET_OMCC_ID(omccId) \
if(xpon_hooks.xponSetOmccIdCallbackFunction){ \
	xpon_hooks.xponSetOmccIdCallbackFunction(omccId); \
}

#define CALL_USER_HOOK_SET_PHY_EVENT_HANDLER(stateNotifyType) \
if(xpon_hooks.setPhyEventHandlerCallbackFunction){ \
	xpon_hooks.setPhyEventHandlerCallbackFunction(stateNotifyType); \
}

int xpon_api_init(void);
void xpon_api_deinit(void);

#endif /* _XPON_API_H_ */



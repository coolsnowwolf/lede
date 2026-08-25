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
	
/************************************************************************
*				   I N C L U D E S
*************************************************************************
*/

#include "common/xpon_global.h"
#include "common/xpon_api.h"
#include "ecnt_hook/ecnt_hook.h"
#include "ecnt_hook/ecnt_hook_pon_customer.h"


/************************************************************************
*                  D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/
	
/************************************************************************
*				   M A C R O S
*************************************************************************
*/	


/************************************************************************
*				   D A T A	 T Y P E S
*************************************************************************
*/
	
/* ------------------ ECNT_Hook ------------------ */
typedef int (*xpon_api_op_t)(ECNT_XPON_CUSTOMER_Data_t *tso_data);


/************************************************************************
*				   E X T E R N A L	 D A T A   D E C L A R A T I O N S
*************************************************************************
*/



/************************************************************************
*				   F U N C T I O N	 D E C L A R A T I O N S
*************************************************************************
*/
ecnt_ret_val ecnt_xpon_customer_hook(struct ecnt_data *indata);
static int xpon_customer_callbacks_reset(void);
static int xpon_customer_callback_register(XPON_CALLBACK_Info_t* callback);
static int xpon_customer_callback_phy_event_handler(uint cmd_id, int stateNotifyType);




/************************************************************************
*				   P U B L I C	 D A T A
*************************************************************************
*/

/************************************************************************
*                  P R I V A T E   D A T A
*************************************************************************
*/


/* ------------------ ECNT_Hook ------------------ */


struct ecnt_hook_ops ecnt_driver_xpon_op = {
	.name = "xpon_customer_hook",
	.hookfn = ecnt_xpon_customer_hook,
	.maintype = ECNT_XPON_MAC,
	.is_execute = 1,
	.subtype = ECNT_XPON_CUSTOMER_HOOK,
	.priority = 1
};

XPON_Customer_hooks_t xpon_hooks = {0};


/************************************************************************
*                  F U N C T I O N   D E F I N I T I O N S
*************************************************************************
*/

/***********************     for eternal ECNT_HOOK API.    *******************/
/*______________________________________________________________________________
**	function name
**		ecnt_xpon_customer_hook
**	description:
**		ECNT HOOK for customer hook process.
**	parameters:
**		indata ecnt_data
**	global:
**		gpPonSysData
**	return:
**		ecnt_ret_val
**	call:
**		IO_GREG
**	revision:
**		v1.0
**____________________________________________________________________________*/
ecnt_ret_val ecnt_xpon_customer_hook(struct ecnt_data *indata)
{
    ecnt_ret_val ret = ECNT_HOOK_ERROR;	
	ECNT_XPON_CUSTOMER_Data_t *xpon_data = (ECNT_XPON_CUSTOMER_Data_t *)indata ;

	if(NULL == indata){
		printk("input param error!\n");
		return -ENODEV;
	}

	switch(xpon_data->function_id){
		case XPON_CUSTOMER_API_CALLBACK_RESET:
			ret = xpon_customer_callbacks_reset();
			break;
		case XPON_CUSTOMER_API_CALLBACK_REGISTER:
			ret = xpon_customer_callback_register(&xpon_data->xpon_customer_private.callback);
			break;
		case  XPON_CUSTOMER_API_CALLBACK_PHY_EVENT_HANDLER:
			ret = xpon_customer_callback_phy_event_handler(xpon_data->cmd_id, xpon_data->eventtype);
			break;
		default:
			ret = ECNT_HOOK_ERROR;
			printk("xpon_data->function_id is %d, is not recongnized! XPON_CUSTOMER_API_MAX_NUM is :%d\n", 
				xpon_data->function_id, XPON_CUSTOMER_API_MAX_NUM);
			break;
	}

	return ret;
}

/*______________________________________________________________________________
**	function name
**		xpon_customer_callbacks_reset
**	description:
**		reset all callback function pointers as NULL.
**	parameters:
**		void.
**	global:
**		xpon_hooks
**	return:
**		None
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/
static int xpon_customer_callbacks_reset(void)
{
	memset(&xpon_hooks, 0, sizeof(xpon_hooks));
	return 0;
}

/*______________________________________________________________________________
**	function name
**		xpon_customer_callback_register
**	description:
**		reset all callback function pointers as NULL.
**	parameters:
**		void.
**	global:
**		xpon_hooks
**	return:
**		None
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/
static int xpon_customer_callback_register(XPON_CALLBACK_Info_t* callback)
{
    int ret = 0;
    
	if(NULL == callback){
		printk("input param error!\n");
		return -ENODEV;
	}	
	
	switch(callback->callback_id){
		case ECNT_XPON_CALLBACK_MULTICAST_RX_DATA:
			xpon_hooks.multicastRxDataCallbackFunction = callback->callback_fn;
			break;
		case ECNT_XPON_CALLBACK_PHY_EVENT:
			xpon_hooks.phyEventNotifyCallbackFunction = callback->callback_fn;
			break;
		case ECNT_XPON_CALLBACK_XMCS_CHECK_EVENT_VALID:
			xpon_hooks.xmcsEventCheckValidCallbackFunction = callback->callback_fn;
			break;
		case ECNT_XPON_CALLBACK_SET_EMERGENCE_STATE:
			xpon_hooks.xponSetEmergenceStateCallbackFunction = callback->callback_fn;
			break;
		case ECNT_XPON_CALLBACK_FH_TX_POWER_HANDLE:
			xpon_hooks.fhTxPowerHandleCallbackFunction = callback->callback_fn;
			break;	
		case ECNT_XPON_CALLBACK_SET_ONU_ID:
			xpon_hooks.xponSetOnuIdCallbackFunction= callback->callback_fn;
			break;		
		case ECNT_XPON_CALLBACK_SET_OMCC_ID:
			xpon_hooks.xponSetOmccIdCallbackFunction= callback->callback_fn;
			break;
		case ECNT_XPON_CALLBACK_SET_PHY_EVENT_HANDLER:
			xpon_hooks.setPhyEventHandlerCallbackFunction= callback->callback_fn;
			break;

		default:
			ret = ECNT_HOOK_ERROR;
			printk(" Unknown callback id registered!\n");
			break;
	}
	
	return ret;
}

static int xpon_customer_callback_phy_event_handler(uint cmd_id, int stateNotifyType)
{
	if(PON_SET_PHY_NOTIFY_EVENT == cmd_id){
		CALL_USER_HOOK_SET_PHY_EVENT_HANDLER(stateNotifyType);
		return 0;
	}else{
		printk("event error!\n");
		return -ENODEV;
	}	
}


int xpon_api_init(void){

	if(ecnt_register_hook(&ecnt_driver_xpon_op)) {
		printk("ecnt_dev_fh_op register fail\n");
		return -ENODEV ;
	}
	return 0;
}

void xpon_api_deinit(void){

	ecnt_unregister_hook(&ecnt_driver_xpon_op);
	printk("deinit ecnt_dev_fh_op register\n");

}


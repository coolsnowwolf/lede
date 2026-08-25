#include <linux/module.h>
#include <linux/kernel.h>

#include "epon/epon.h"
#include "common/xpon_global.h"
#include "epon/epon_act.h"
#include "epon/epon_compile_option_wrapper.h"
#include "common/xpon_api.h"
#include "common/xpon_led.h"
#include "epon/epon_compile_option_wrapper.h"


/*______________________________________________________________________________
**	function name
**		epon_los_state
**	description:
**		set epon los status
**	parameters:
**		data
**	global:
**		gpPhyData
**	return:
**		0:success
**		-1:fail
**	call:
**		epon_event_report
**	revision:
**		v1.0
**____________________________________________________________________________*/
int epon_los_state(void *data)
{
	epon_event_report(XMCS_EVENT_EPON_LOS, 0);
    gpPhyData->phy_link_status = PHY_LINK_STATUS_LOS;
    DEBUG_MSG_EPON(MSG_LEVEL_DEBUG,"DEBUG: %s report LOS and set phy link down",__FUNCTION__);
    
    /*alarm led flicker, pon led off*/
    change_alarm_led_status(LED_FLICKER);
    change_pon_led_status(LED_OFF);

    return EPON_SUCCESS;
}

/*______________________________________________________________________________
**	function name
**		epon_typeb_hold_on_start
**	description:
**		epon typeB hold on
**	parameters:
**		data
**	global:
**		gp_epon_global_data
**	return:
**		0:success
**		-1:fail
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/
int epon_typeb_hold_on_start(void *data)
{
    EPON_DRIVER_SET(TYPE_B_ENABLE,TRUE);
    DEBUG_MSG_EPON(MSG_LEVEL_NOTIFY,"DEBUG: hold time: %d\n", gp_epon_global_data->hold_over_time);  
    EPON_START_TIMER(gp_epon_global_data->typeb_timer,gp_epon_global_data->hold_over_time);   
    DEBUG_MSG_EPON(MSG_LEVEL_NOTIFY,"DEBUG: epon typeB process start timer and set typeB enable flag");   
    return EPON_SUCCESS;
}

/*______________________________________________________________________________
**	function name
**		epon_detect_los_lof_handler
**	description:
**		epon detect phy los
**	parameters:
**		data
**	global:
**		None
**	return:
**		0:success
**		-1:fail
**	call:
**		epon_msg_route_dispatch
**	revision:
**		v1.0
**____________________________________________________________________________*/

int epon_detect_los_lof_handler(void *data)
{
	int ret = EPON_SUCCESS;

	DEBUG_MSG_EPON(MSG_LEVEL_NOTIFY,"DEBUG: phy detect losLof begin");
	CALL_USER_HOOK_PHY_EVENT_NOTIFY(XMCS_EVENT_TYPE_EPON, XMCS_EVENT_EPON_LOS);
	if(EPON_DRIVER_CHECK(HOLD_OVER_CFG_ENABLE,TRUE))
	{
		epon_msg_route_dispatch(EPON_MSG_LOS_ENABLE_TYPEB,MSG_INPUT_NULL);
		DEBUG_MSG_EPON(MSG_LEVEL_NOTIFY,"DEBUG:epon holdOver enable do typeb process");
		return EPON_SUCCESS;
	}

	DEBUG_MSG_EPON(MSG_LEVEL_NOTIFY,"DEBUG:epon holdOver disable do los process");
	ret = epon_msg_route_dispatch(EPON_MSG_TRUE_LOS,MSG_INPUT_NULL);
	if(EPON_SUCCESS != ret)
	{
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR,"ERROR: epon los process fail");
		return ret;
    }

	DEBUG_MSG_EPON(MSG_LEVEL_DEBUG,"DEBUG: phy detect losLof end");
	return ret;
}
/*______________________________________________________________________________
**	function name
**		epon_detect_phy_ready_handler
**	description:
**		epon detect phy ready
**	parameters:
**		data
**	global:
**		gpPonSysData
**	return:
**		0:success
**		-1:fail
**	call:
**		epon_msg_route_dispatch
**		xpon_set_qdma_qos
**	revision:
**		v1.0
**____________________________________________________________________________*/

int epon_detect_phy_ready_handler(void *data)
{
	DEBUG_MSG_EPON(MSG_LEVEL_DEBUG,"DEBUG: %s begin",__FUNCTION__);
    
	if(EPON_DRIVER_CHECK(TYPE_B_ENABLE,TRUE))
    {
    	DEBUG_MSG_EPON(MSG_LEVEL_NOTIFY,"DEBUG: epon in type B needn't to do epon start just return");
        EPON_DRIVER_SET(TYPE_B_ENABLE,FALSE);
        return EPON_SUCCESS;
    }
    
	if(EPON_DRIVER_CHECK(gpPonSysData->sysStartup,PON_WAN_START))
    {
        DEBUG_MSG_EPON(MSG_LEVEL_NOTIFY,"DEBUG: epon detect phy ready, epon start");
    	EPON_START_TASK(&gp_epon_global_data->epon_start_task);
    }
    
    gpPonSysData->sysLinkStatus = PON_LINK_STATUS_EPON ;

    DEBUG_MSG_EPON(MSG_LEVEL_DEBUG,"DEBUG: %s end",__FUNCTION__);
    
    change_alarm_led_status(LED_OFF);

    return EPON_SUCCESS;
}


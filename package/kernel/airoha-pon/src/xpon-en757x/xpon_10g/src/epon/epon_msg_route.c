#include <linux/module.h>
#include <linux/kernel.h>

#include "epon/epon.h"
#include "epon/epon_reg.h"
#include "epon/epon_act.h"
#include "epon/epon_dev.h"
#include "epon/epon_msg_route.h"
#include "common/xpon_global.h"
#include "common/xpondrv.h"
#include "epon/epon_compile_option_wrapper.h"
#include "common/phy_if_wrapper.h"

int epon_event_phy_true_los(void *data)
{
	int ret = EPON_SUCCESS;

	XPON_PHY_TX_DISABLE();
	ret = epon_los_state(data);
	if(ret != EPON_SUCCESS)
	{
		DEBUG_MSG_EPON(MSG_LEVEL_DEBUG,"[%s][FAILED] do function epon_los_state\n",__FUNCTION__);
        return ret;
	}
	ret = epon_stop(data);
	if(ret != EPON_SUCCESS)
	{
		DEBUG_MSG_EPON(MSG_LEVEL_DEBUG,"[%s][FAILED] do function epon_stop\n",__FUNCTION__);
        return ret;
	}
	ret = epon_reset(data);
	if(ret != EPON_SUCCESS)
	{
		DEBUG_MSG_EPON(MSG_LEVEL_DEBUG,"[%s][FAILED] do function epon_reset\n",__FUNCTION__);
        return ret;
	}
	return EPON_SUCCESS;
}

int epon_event_phy_true_ready(void *data)
{
	int ret = EPON_SUCCESS;

	ret = epon_phy_ready_hw_init(data);
	if(ret != EPON_SUCCESS)
	{
		DEBUG_MSG_EPON(MSG_LEVEL_DEBUG,"[%s][FAILED] do function epon_phy_ready_hw_init\n",__FUNCTION__);
        return ret;
	}
	ret = epon_phy_ready_sw_init(data);
	if(ret != EPON_SUCCESS)
	{
		DEBUG_MSG_EPON(MSG_LEVEL_DEBUG,"[%s][FAILED] do function epon_phy_ready_sw_init\n",__FUNCTION__);
        return ret;
	}

	return EPON_SUCCESS;
}

int epon_event_ack_register(void *data)
{
	int ret = EPON_SUCCESS;
	
	ret = epon_rcv_rgst_ack_change_state(data);
	if(ret != EPON_SUCCESS)
	{
		DEBUG_MSG_EPON(MSG_LEVEL_DEBUG,"[%s][FAILED] do function epon_rcv_rgst_ack_change_state\n",__FUNCTION__);
        return ret;
	}
	ret = epon_act_send_register_ack(data);
	if(ret != EPON_SUCCESS)
	{
		DEBUG_MSG_EPON(MSG_LEVEL_DEBUG,"[%s][FAILED] do function epon_act_send_register_ack\n",__FUNCTION__);
        return ret;
	}

	return EPON_SUCCESS;
}

int epon_event_re_register(void *data)
{
	int ret = EPON_SUCCESS;
	
	ret = epon_rcv_re_register_change_state(data);
	if(ret != EPON_SUCCESS)
	{
		DEBUG_MSG_EPON(MSG_LEVEL_DEBUG,"[%s][FAILED] do function epon_rcv_re_register_change_state\n",__FUNCTION__);
        return ret;
	}
	ret = epon_act_send_register_ack(data);
	if(ret != EPON_SUCCESS)
	{
		DEBUG_MSG_EPON(MSG_LEVEL_DEBUG,"[%s][FAILED] do function epon_act_send_register_ack\n",__FUNCTION__);
        return ret;
	}

	return EPON_SUCCESS;
}

int epon_event_de_register(void *data)
{
	int ret = EPON_SUCCESS;
	
	ret = epon_de_register_change_state(data);
	if(ret != EPON_SUCCESS)
	{
		DEBUG_MSG_EPON(MSG_LEVEL_DEBUG,"[%s][FAILED] do function epon_de_register_change_state\n",__FUNCTION__);
        return ret;
	}
	ret = epon_llid_reinit(data);
	if(ret != EPON_SUCCESS)
	{
		DEBUG_MSG_EPON(MSG_LEVEL_DEBUG,"[%s][FAILED] do function epon_llid_reinit\n",__FUNCTION__);
        return ret;
	}
	return EPON_SUCCESS;
}

msg_event_action_t e_msg_route_table[]= 
{
{EPON_MSG_DETECT_LOS,						"PHY LOS EVENT",						epon_detect_los_lof_handler},
{EPON_MSG_DETECT_READY,						"PHY READY EVENT",						epon_detect_phy_ready_handler},
{EPON_MSG_LOS_ENABLE_TYPEB,					"TYPE-B EVENT",							epon_typeb_hold_on_start},
{EPON_MSG_TRUE_LOS,							"PHY REAL LOS EVENT",					epon_event_phy_true_los},
{EPON_MSG_TRUE_READY,						"PHY REAL READY EVENT",					epon_event_phy_true_ready},
{EPON_MSG_RCV_DISCOVERY_GATE,				"RCV MPCP DISCOVERY GATE EVENT",		epon_mpcp_discovery_gate_handler},
{EPON_MSG_SEND_REGISTER_REQ,				"",										NULL},
{EPON_MSG_RCV_REGISTER_MSG,					"RCV MPCP REGISTER EVENT",				epon_mpcp_rcv_register_msg_handler},
{EPON_MSG_RCV_ACK_REGISTER,					"RCV MPCP ACK EVENT",					epon_event_ack_register},
{EPON_MSG_RCV_NACK_REGISTER,				"",										epon_rcv_nack_flag_msg_process},
{EPON_MSG_RCV_RE_REGISTER,					"RCV MPCP RE REGISTER EVENT",			epon_event_re_register},
{EPON_MSG_RCV_DE_REGISTER,					"RCV MPCP RE REGISTER EVENT",			epon_event_de_register},
{EPON_MSG_CHANHE_TO_SILENT,					"",										NULL},
{EPON_MSG_TX_POWER_CHANGE,					"",										NULL},

/*please add event action above*/
{EPON_MSG_MAX_NUM, "", NULL}
};

/*______________________________________________________________________________
**	function name
**		epon_msg_route_dispatch
**	description:
**		mpcp message dispatch
**	parameters:
**		type
**		paramters
**	global:
**		None
**	return:
**		0:success
**		-1:fail
**	call:
**		get_msg_route_table
**	revision:
**		v1.0
**____________________________________________________________________________*/

int epon_msg_route_dispatch(EPON_MSG_ROUTE_TYPE type,void *paramters)
{   
	if(type >=EPON_MSG_MAX_NUM || NULL == e_msg_route_table[type].func)
    {
    	DEBUG_MSG_EPON(MSG_LEVEL_DEBUG,"event type %d is not exist\n",type);
        return EPON_INPUT_POINT_NULL;
    }
    DEBUG_MSG_EPON(MSG_LEVEL_DEBUG,"event type %d[%s] has functions\n",type,e_msg_route_table[type].name);
    
	return e_msg_route_table[type].func(paramters);
}




/**
*@file
*@brief This file defines traffic flow related interfaces that should be implemented by chip vendors 
*@author Zhang Yan
*@date     2012.10.11
*@copyright 
*******************************************************************
* Copyright (c) 2012 Alcatel Lucent Shanghai Bell Software, Inc.
*             All Rights Reserved
*
* This source is confidential and proprietary and may not
* be used without the written permission of Alcatel Lucent
* Shanghai Bell, Inc.
********************************************************************
*@par Module History:
*/

#ifndef _HAL_ITF_FLOW_H_
#define _HAL_ITF_FLOW_H_

#include "stdio.h"
#include "string.h"
#include <stdlib.h>
#include <netinet/in.h>

#include "hal_type_flow.h"
#include "hal_type_ethernet.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif


/**
* To create an upstream ethernet flow of specified ethernet port, include the 
* traffic classification and vlan translation and pbit remark
* @param  flow flow parameters such as classification rule and actions
* @see  hal_stream_flow_info_t
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_stream_us_create_ethflow(hal_stream_flow_info_t* flow);

/**
* To delete an upstream ethernet flow
* @param  flow_id ethernet flow identifier
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_stream_us_delete_ethflow(unsigned int flow_id);


/**
* To create an downstream ethernet flow of specified ethernet port, include the 
* traffic classification and vlan translation and pbit remark
* @param  flow flow parameters such as classification rule and actions
* @see  hal_stream_flow_info_t
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_stream_ds_create_ethflow(hal_stream_flow_info_t* flow);

/**
* To delete an downstream ethernet flow
* @param  flow_id ethernet flow identifier
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_stream_ds_delete_ethflow(unsigned int flow_id);

/**
* To create an mcast ethernet flow 
* @param  flow flow parameters such as mcast ethflow and actions
* @see  hal_mcast_flow_info_t
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_stream_create_mcast_ethflow(hal_mcast_flow_info_t flow);

/**
* To create a map from an upstream ethernet flow to gem ports
* @param  flow_id ethernet flow identifier
* @param  gemports[] gem ports array with fixed size 8 and indexed by pbit (0-7)
*                    invalid gemports if gemport >= 4096
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_stream_us_map_ethflow_to_gemport(hal_port_domain_mode_t port_domain,unsigned int flow_id, unsigned short gemports[]);

/**
* To delete the map between an upstream ethernet flow and gem ports
* @param  flow_id ethernet flow identifier
* @param  gemports[] gem ports array with fixed size 8 and indexed by pbit (0-7)
*                    invalid gemports if gemport >= 4096
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_stream_us_unmap_ethflow_to_gemport(hal_port_domain_mode_t port_domain,unsigned int flow_id, unsigned short gemports[]);

/**
* To create a map from gem ports to an downstream ethernet flow
* @param  flow_id ethernet flow identifier
* @param  gemports[] gem ports array with fixed size 8 and indexed by pbit (0-7)
*                    invalid gemports if gemport >= 4096
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_stream_ds_map_gemport_to_ethflow(unsigned int flow_id, unsigned short gemports[]);

/**
* To delete the map between gem ports and an downstream ethernet flow
* @param  flow_id ethernet flow identifier
* @param  gemports[] gem ports array with fixed size 8 and indexed by pbit (0-7)
*                    invalid gemports if gemport >= 4096
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_stream_ds_unmap_gemport_to_ethflow(unsigned int flow_id, unsigned short gemports[]);

/**
* To print upstream ethflow info for debug, it is a debug command
* @param  appFlowID application flow id
* @return none
*/
void hal_stream_dbg_show_us_ethflow_info(unsigned int appFlowID);

/**
* To print downstream ethflow info for debug,it is a debug command
* @param  appFlowID application flow id
* @return none
*/
void hal_stream_dbg_show_ds_ethflow_info(unsigned int appFlowID);

/**
* To print upstream vlan action info for debug,it is a debug command
* @param  appFlowID application flow id
* @return none
*/
void hal_stream_dbg_show_us_vlan_action_info(unsigned int appFlowID);

/**
* To print downstream vlan action info for debug,it is a debug command
* @param  appFlowID application flow id
* @return none
*/
void hal_stream_dbg_show_ds_vlan_action_info(unsigned int appFlowID);

/**
* To get an mcast vlan action id
* @param  vlanEntryID vlan action identifier need to get
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
* @todo  mcast vlan should move to hal_itf_igmp.h
*/
hal_ret_t hal_stream_get_mcast_vlan_action_id(unsigned int *vlanEntryID);

/**
* To set the trace level of flow function in HAL module
* @param  trace_level the new trace level in hal
* @return none
* @todo trc level can be all hal using, not stream or others qos.
*/
void hal_stream_set_trc_level(unsigned int trace_level);

/**
* To get the downstream ethernet flow ID by application flow ID
* @param  appFlowID the applicaiton flow ID
* @return return the ethernet flow ID in hal
* @todo remove it with suffix "stream", and appFlowID , app_flow_id
*/
unsigned int hal_get_ds_ethflowId(unsigned int appFlowID);

/**
* To control the downstream broadcast forward to cpu or transparent
* @param  para the infroamtion of forward control
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
* @todo, cmd, name all is not clear, and  brlt only
*/

hal_ret_t hal_ctrl_ds_uni_gem_vid_to_spf(hal_dspforward_ctrl_t para);

/**
*called to set the meter id of wan_arp_table_miss wan 
*@deprecated, no one use it anymore
*/
hal_ret_t hal_set_wanArpTableMiss_meter_id(unsigned int meter_id);

/**
* To create unmatch ds flow for one port with tag or untag mode
* @param  hal_port_info the port associated with this gem port
* @param  dsTggingMode ds tag or untag
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_stream_ds_create_unmatch_ethflow(hal_port_info_t *hal_port_info
                                              , unsigned short dsTggingMode);

/**
* To print all ethflow info for debug,it is a debug command
* @return none
*/
void hal_stream_show_all_ethflow(void);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif




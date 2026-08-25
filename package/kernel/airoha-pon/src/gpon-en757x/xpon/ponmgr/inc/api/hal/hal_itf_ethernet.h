
/**
*@file
*@brief This file defines ethernet port related interfaces that should be implemented by chip vendors 
*@author Zhang Yan H
*@date     2012.10.17
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

#ifndef _HAL_ETHERNET_H_
#define _HAL_ETHERNET_H_

#include "hal_type_ethernet.h"

#ifdef __cplusplus
extern "C" {
#endif



/**
* To configure the port fowarding eligibility to other bridge port.
* The Mask determines if packets from a bridge port can be locally 
* switched to other bridge ports or blocked.             
* @param  fwdPortMask  the mask of forward eligibility
* @see hal_drv_file_t
* @see hal_fwd_port_mask_t
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
* @note bl  only
*/
hal_ret_t hal_eth_cfg_port_forwarding_eligibility(hal_port_info_t hal_port_info, unsigned int isEnable);

/**
* To configure the port fowarding eligibility between lan_port and wan_bridged_port.
* If the Mask is disable, the traffic between lan_port and wan_bridged_port are blocked. 
* Otherwise, the traffic are unblocked.
* @param  fwdPortMask  the mask of forward eligibility
* @see hal_drv_file_t
* @see hal_fwd_port_mask_t
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
* @note bl only
*/
hal_ret_t  hal_eth_cfg_port_service_flow(hal_port_info_t hal_port_info, hal_bool_t isEnable);

/**
* To configured the packets forword method when destination mac address not learned
* @param   hal_port_info the port_info of UNI port need to configured
* @param  mac_learning_mode if the value is true, then means drop the packets with unknown DA, else forword to CPU
* @see hal_bool_t
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
* @todo internal and init related,move to init
*/
hal_ret_t   hal_eth_cfg_port_forwarding_mode(hal_port_info_t hal_port_info, hal_bool_t mac_learning_mode);

/**
* To configured the port managed by which method, OMCI or TR069. 
*If the onu type  is true, then it need be managed by TR069, else  means  managed by OMCI   
* @param  hal_port_info the port_info of UNI port need to configured
* @param  isHGU  to configured the port managed by TR069 or OMCI
* @see hal_bool_t
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_eth_configure_port_domain(hal_port_info_t hal_port_info, hal_bool_t isHGU);

/**
* To get  the port managed by which method, OMCI or TR069. 
* @param  hal_port_info the port_info of UNI port need to configured
* @param  isHGU  to show the port managed by which domain. If it is TRUE, then managed by TR069, else OMCI
* @see hal_bool_t
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_eth_get_port_domain(hal_port_info_t hal_port_info, hal_bool_t* isHGU);


/**
* To set the phy information, include the phy speed, mode and status 
* @param  hal_port_info the port_info of UNI port need to configured
* @param  mode  the parameter of the port information need to set
* @see hal_lan_port_t
* @see hal_lan_attr_t
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_phy_set_eth_mode(hal_port_info_t  hal_port_info,hal_lan_attr_t mode);

/**
* To get the phy information, include the phy speed, mode and status 
* @param  hal_port_info the port_info of UNI port need to configured
* @param  mode  the parameter of the port information need to set
* @see hal_lan_port_t
* @see hal_lan_attr_t
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_phy_get_eth_mode(hal_port_info_t hal_port_info,hal_lan_attr_t * mode);
/**
* To get  the phy port by logic port. 
* @param  hal_port_info the port_info of UNI port need to configured
* @param  phy_port the physical port
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
* @deprecated
*/
hal_ret_t hal_eth_get_phy_port(hal_port_info_t hal_port_info, unsigned int *phy_port);

#ifdef __cplusplus
}
#endif

#endif





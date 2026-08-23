/**
 *@file
 *@brief This file defines security related interfaces that should be implemented by chip vendors
 *@author Zhang Yan H
 *@date     2012.10.22
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

#ifndef _HAL_ITF_SECURITY_H_
#define _HAL_ITF_SECURITY_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "hal_type_security.h"
#include "hal_type_common.h"

/**
* To control the up stream acl function. Enable/disable the special filter mode
* @param  port_id  the port id of the acl function need to be configured
* @param  mode  set the acl mode, use IP filter, MAC filter or IP+MAC,...
* @param  enable  enable or disable acl function
* @see hal_acl_mode_t
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_security_ctl_us_acl(hal_port_info_t hal_port_info, hal_acl_mode_t mode, unsigned int enable);

/**
* To set the up stream acl function. Set the policy of a special filter mode
* @param  port_id  the port id of the acl function need to be configured
* @param  mode  set the acl mode, use IP filter, MAC filter or IP+MAC,...
* @param  policy  set the policy, use white list or black list
* @see hal_acl_mode_t
* @see hal_acl_policy_t
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_security_set_us_acl_mode(hal_port_info_t hal_port_info, hal_acl_mode_t mode, hal_acl_policy_t policy);

/**
* To add a record of the up stream acl function
* @param  acl  the record need to be configured
* @see hal_acl_t
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_security_config_us_acl(hal_acl_t *acl);
hal_ret_t hal_security_config_us_acl_untag(hal_acl_t *acl);


/**
* To remove a record of the up stream acl function
* @param  port_id  the port id of the acl function need to be configured
* @param  rule_num  the rule number of the record in acl list need to be removed
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_security_remove_us_acl(hal_port_info_t hal_port_info
                                    , unsigned int rule_num
                                    , hal_acl_mode_t mode);

/**
*To add a IP rule to ebtables chain
*@param port_id  the port id of the acl function need to be configured
*@param ip  the ip will be used to filter
*@param policy  set the policy, use white list or black list
*@return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_security_add_ip_chain(hal_port_info_t hal_port_info, unsigned int ip, unsigned char policy);

/**
*To add a mac rule to ebtables chain
*@param port_id the port id of the acl function need to be configured
*@param mac_bytes the mac will be used to filter
*@param policy set the policy, use white list or black list
*@param is_oui set the acl mode,use mac_oui or mac
*@return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_security_add_mac_chain(hal_port_info_t hal_port_info,const unsigned char *mac_bytes, unsigned char policy, int is_oui);

/**
*To add a IP and mac rule to ebtables chain
*@param port_id the port id of the acl function need to be configured
*@param mac_bytes the mac will be used to filter
*@param ip  the ip will be used to filter
*@param policy set the policy, use white list or black list
*@return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_security_add_ip_mac_chain(hal_port_info_t hal_port_info, 
             const unsigned char *mac_bytes,unsigned int ip, unsigned char policy);

/**
* To set the up stream filter function.Initialize the ebtables.
*@param port_id the port id of the acl function need to be configured
*@param mode set the filter mode
*@return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_security_create_xtables_chain(hal_port_info_t hal_port_info, unsigned char mode);

/**
* To remove the up stream filter function.delete the ebtables.
*@param port_id the port id of the acl function need to be configured
*@param mode set the status
*@return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_security_destroy_xtables_chain(hal_port_info_t hal_port_info, unsigned char mode);
#ifdef __cplusplus
}
#endif

#endif

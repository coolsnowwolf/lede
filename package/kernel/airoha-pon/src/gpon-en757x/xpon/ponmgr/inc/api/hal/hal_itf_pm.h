/**
*@file
*@brief This file defines performance monitor related interfaces that should be implemented by chip vendors 
*@author Shen Jingguo
*@date     2013.3.10
*@copyright 
*******************************************************************
* Copyright (c) 2013 Alcatel Lucent Shanghai Bell Software, Inc.
*             All Rights Reserved
*
* This source is confidential and proprietary and may not
* be used without the written permission of Alcatel Lucent
* Shanghai Bell, Inc.
********************************************************************
*@par Module History:
*/

#ifndef _HAL_ITF_PM_H_
#define _HAL_ITF_PM_H_

#include "hal_type_common.h"
#include "hal_type_pm.h"
#include "hal_itf_misc.h"

#ifdef __cplusplus
extern "C"{
#endif
/**
* To start the statistics on the specified ethernet port
* @param  hal_port_info the port info of UNI port need to start statistics
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_pm_start_eth_cnt(hal_port_info_t hal_port_info);
/**
* To stop the statistics on the specified ethernet port
* @param  hal_port_info the port info of UNI port need to stop statistics
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_pm_stop_eth_cnt(hal_port_info_t hal_port_info);
/**
* To get the statistics on the specified ethernet port
* @param  hal_port_info the port info of UNI port need to stop statistics
* @param  cnt  the statistics value get from the chipset
* @see hal_port_count_t
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_pm_get_eth_cnt( hal_port_info_t hal_port_info, hal_port_count_t*cnt);

/**
* To start the statistics on the specified gem port
* @param  gemport the ID of gemport need to start statistics
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_pm_start_gemport_count(unsigned short gemport);
/**
* To stop the statistics on the specified gem port
* @param  gemport the ID of gemport need to stop statistics
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_pm_stop_gemport_count(unsigned short gemport);

/**
* To get the statistics on the specified gem port
* @param  gemport the ID of gemport need to start statistics
* @param  cnt  the statistics value get from the chipset
* @see hal_gemport_count_t
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_pm_get_gemport_count(unsigned short gemport, hal_gem_count_t * cnt);

/**
* To start the FEC statistics on the PON
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_pm_start_fec_cnt();

/**
* To stop the FEC statistics on the PON
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_pm_stop_fec_cnt();

/**
* To get the FEC statistics on the PON
* @param  cnt  the statistics value get from the chipset
* @see hal_fec_count_t
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
*/
hal_ret_t hal_pm_get_fec_cnt( hal_fec_count_t *cnt);


/**
 * To get the PM statistics of the ip subnet
 * @param  subnet indicate the subnetId and get the PM data
 * @see hal_wan_count_t
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
 */
hal_ret_t hal_pm_get_ip_subnet_counter( hal_wan_count_t *subnet);

/**
 * To get the PM statistics of the net device
 * @param  subnet indicate the net device ID and get the PM data
 * @see hal_wan_count_t
* @return return HAL_RET_FAILED if failed, return HAL_RET_SUCCESS if success
 */
hal_ret_t hal_pm_get_net_dev_counter( hal_wan_count_t *subnet);

hal_ret_t hal_pm_lx_ip_subnet_get_counter(hal_drv_file_t drvFile,hal_wan_netdev_stats_t *stats_netdev);

#ifdef __cplusplus
}
#endif
#endif


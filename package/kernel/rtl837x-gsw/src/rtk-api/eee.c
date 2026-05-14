/*
 * Copyright (C) 2013 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * $Revision$
 * $Date$
 *
 * Purpose : RTK switch high-level API for RTL8373
 * Feature : Here is a list of all functions and variables in EEE module.
 *
 */

#include <rtk_switch.h>
#include <rtk_error.h>
#include <eee.h>
#include <string.h>

#include <dal/dal_mgmt.h>


/* Function Name:
 *      rtk_eee_init
 * Description:
 *      Initial EEE function.
 * Input:
 *      None
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_ENABLE - Invalid enable input.
 * Note:
 *      This API can set EEE function to the specific port.
 *      The configuration of the port is as following:
 *      - DISABLE
 *      - ENABLE
 */

rtk_api_ret_t rtk_eee_init(void)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->eee_init)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->eee_init();
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_eee_macForceSpeedEn_set
 * Description:
 *      Set enable status of EEE for a Specified Speed,
 * Input:
 *      port - port id.
 *		speed	- a specified
 *      enable  - enable EEE status.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_ENABLE - Invalid enable input.
 * Note:
 *      This API can set EEE function to the specific port.
 *      The configuration of the port is as following:
 *      - DISABLE
 *      - ENABLE
 */
rtk_api_ret_t rtk_eee_macForceSpeedEn_set(rtk_port_t port, rtk_eee_speedInMacForceMode_t speed, rtk_enable_t enable)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->eee_macForceSpeedEn_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->eee_macForceSpeedEn_set(port, speed, enable);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_eee_macForceSpeedEn_get
 * Description:
 *      Get port_n status of EEE for a Specified Speed,
 * Input:
 *      port - port id.
 *		speed	- a specified
 * Output:
 *      pEnable - EEE status.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_ENABLE - Invalid enable input.
 * Note:
 *      This API can get EEE function to the specific port.
 *      The configuration of the port is as following:
 *      - DISABLE
 *      - ENABLE
 */
rtk_api_ret_t rtk_eee_macForceSpeedEn_get(rtk_port_t port, rtk_eee_speedInMacForceMode_t speed, rtk_enable_t *pEnable)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->eee_macForceSpeedEn_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->eee_macForceSpeedEn_get(port, speed, pEnable);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_eee_macForceAllSpeedEn_get
 * Description:
 *      Get port_n status of EEE for all Speed.
 *      port - port id.
 * Output:
 *      pState - EEE status.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_ENABLE - Invalid enable input.
 * Note:
 *      This API can get EEE function to the specific port.
 *      The configuration of the port is as following:
 *      - DISABLE
 *      - ENABLE
 */
rtk_api_ret_t rtk_eee_macForceAllSpeedEn_get(rtk_port_t port, rtk_uint32 *pState)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->eee_macForceAllSpeedEn_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->eee_macForceAllSpeedEn_get(port, pState);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_eee_portTxRxEn_set
 * Description:
 *      Set port_n Tx & Rx EEE capability.
 * Input:
 *      port - port id.
 *      rxEn - Enable or Disable Rx EEE capability.
 *      txEn - Enable or Disable Tx EEE capability.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_ENABLE - Invalid enable input.
 * Note:
 *      This API can config tx EEE and rx EEE capability to the specific port.
 *      The configuration of the port is as following:
 *      - DISABLE
 *      - ENABLE
 */
rtk_api_ret_t rtk_eee_portTxRxEn_set(rtk_port_t port, rtk_enable_t rxEn, rtk_enable_t txEn)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->eee_portTxRxEn_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->eee_portTxRxEn_set(port, rxEn, txEn);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_eee_portTxRxEn_set
 * Description:
 *      Set port_n Tx & Rx EEE capability.
 * Input:
 *      port - port id.
 *      rxEn - Enable or Disable Rx EEE capability.
 *      txEn - Enable or Disable Tx EEE capability.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_ENABLE - Invalid enable input.
 * Note:
 *      This API can config tx EEE and rx EEE capability to the specific port.
 *      The configuration of the port is as following:
 *      - DISABLE
 *      - ENABLE
 */
rtk_api_ret_t rtk_eee_portTxRxEn_get(rtk_port_t port, rtk_enable_t *pRxEn, rtk_enable_t *pTxEn)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->eee_portTxRxEn_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->eee_portTxRxEn_get(port, pRxEn, pTxEn);
    RTK_API_UNLOCK();

    return retVal;
}



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
 * Feature : Here is a list of all functions and variables in port module.
 *
 */

#include <rtk_switch.h>
#include <rtk_error.h>
#include <phy.h>
#include <rtl8373_asicdrv.h>
#include <string.h>
#include <dal/dal_mgmt.h>
#include <rtk_types.h>

/* Function Name:
*      rtk_phy_common_c45_an_restart
* Description:
*      Get ability advertisement for auto-negotiation of the specific port
* Input:
*      port - port id
* Output:
*      pAbility - pointer to PHY auto-negotiation abWility
* Return:
*      RT_ERR_OK     - OK
*      RT_ERR_FAILED - invalid parameter
* Note:
*      1000 is vendor specific in C45.
*/
rtk_api_ret_t rtk_phy_common_c45_an_restart(rtk_port_t port)
{
    rtk_api_ret_t retVal;
    if (NULL == RT_MAPPER->phy_common_c45_an_restart)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->phy_common_c45_an_restart(port);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_phy_common_c45_autoNegoEnable_get
 * Description:
 *      Get the auto-negotiation state of the specific port
 * Input:
 *      port - port id
 * Output:
 *      pEnable - pointer to output the auto-negotiation state
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
rtk_api_ret_t rtk_phy_common_c45_autoNegoEnable_get( rtk_port_t port, rtk_enable_t *pEnable)
{
    rtk_api_ret_t retVal;
    if (NULL == RT_MAPPER->phy_common_c45_autoNegoEnable_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->phy_common_c45_autoNegoEnable_get(port, pEnable);
    RTK_API_UNLOCK();

    return retVal;
}



/* Function Name:
 *      rtk_phy_common_c45_autoNegoEnable_set
 * Description:
 *      Set the auto-negotiation state of the specific port and restart auto-negotiation for enabled state.
 * Input:
 *      port - port id
 *      enable - auto-negotiation state
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
rtk_api_ret_t rtk_phy_common_c45_autoNegoEnable_set(rtk_port_t port, rtk_enable_t enable)
{
    rtk_api_ret_t retVal;
    if (NULL == RT_MAPPER->phy_common_c45_autoNegoEnable_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->phy_common_c45_autoNegoEnable_set(port, enable);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_phy_autoNegoAbility_set
 * Description:
 *      Set ability advertisement for auto-negotiation of the specific port
 * Input:
 *      port - port id
 *      pAbility  - auto-negotiation ability that is going to set to PHY
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
rtk_api_ret_t rtk_phy_autoNegoAbility_set(rtk_port_t port, rtk_port_phy_ability_t *pAbility)
{
    rtk_api_ret_t retVal;
    if (NULL == RT_MAPPER->phy_autoNegoAbility_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->phy_autoNegoAbility_set(port, pAbility);
    RTK_API_UNLOCK();

    return retVal;
}


/* Function Name:
 *      rtk_phy_conmmon_c45_autoSpeed_set
 * Description:
 *      Set phy ability for the specific port
 * Input:
 *      port - port id
 *      pAbility  - phy ability
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */

rtk_api_ret_t rtk_phy_common_c45_autoSpeed_set(rtk_port_t port, rtk_port_phy_ability_t *pAbility)
{
    rtk_api_ret_t retVal;
    if (NULL == RT_MAPPER->phy_common_c45_autoSpeed_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->phy_common_c45_autoSpeed_set(port, pAbility);
    RTK_API_UNLOCK();

    return retVal;

}



/* Function Name:
 *      rtk_phy_common_c45_speed_set
 * Description:
 *      Set speed of the specific port
 * Input:
 *      port          - port id
 *      speed         - link speed rtk_port_speed_t
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                 - OK
 *      RT_ERR_FAILED             - invalid parameter
 * Note:
 *      None
 */
rtk_api_ret_t rtk_phy_common_c45_speed_set(rtk_port_t port, rtk_port_speed_t speed)
{
    rtk_api_ret_t retVal;
    if (NULL == RT_MAPPER->phy_common_c45_speed_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->phy_common_c45_speed_set(port, speed);
    RTK_API_UNLOCK();

    return retVal;

}

/* Function Name:
 *      rtk_phy_common_c45_speed_get
 * Description:
 *      Get speed of the specific port
 * Input:
 *      port - port id
 * Output:
 *      pSpeed - pointer to PHY link speed
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
rtk_api_ret_t rtk_phy_common_c45_speed_get(rtk_port_t port, rtk_port_speed_t *pSpeed)
{
    rtk_api_ret_t retVal;
    if (NULL == RT_MAPPER->phy_common_c45_speed_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->phy_common_c45_speed_get(port, pSpeed);
    RTK_API_UNLOCK();

    return retVal;

}


/* Function Name:
 *      rtk_phy_common_c45_enable_set
 * Description:
 *      Set PHY interface state enable/disable of the specific port
 * Input:
 *      port          - port id
 *      enable        - admin configuration of PHY interface
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
rtk_api_ret_t rtk_phy_common_c45_enable_set(rtk_port_t port, rtk_enable_t enable)
{
    rtk_api_ret_t retVal;
    if (NULL == RT_MAPPER->phy_common_c45_enable_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->phy_common_c45_enable_set(port, enable);
    RTK_API_UNLOCK();

    return retVal;

}

/* Function Name:
 *      rtk_phy_common_c45_enable_get
 * Description:
 *      Get PHY interface state enable/disable of the specific port
 * Input:
 *      port          - port id
 * Output:
 *      pEnable       - pointer to admin configuration of PHY interface
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
rtk_api_ret_t rtk_phy_common_c45_enable_get( rtk_port_t port, rtk_enable_t *pEnable)
{
    rtk_api_ret_t retVal;
    if (NULL == RT_MAPPER->phy_common_c45_enable_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->phy_common_c45_enable_get(port, pEnable);
    RTK_API_UNLOCK();

    return retVal;

}

/* Function Name:
 *      rtk_phy_common_c45_duplex_set
 * Description:
 *      Set duplex of the specific port
 * Input:
 *      unit          - unit id
 *      port          - port id
 *      duplex        - duplex mode of the port, full or half
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
rtk_api_ret_t rtk_phy_common_c45_duplex_set(rtk_port_t port, rtk_port_duplex_t duplex)
{
    rtk_api_ret_t retVal;
    if (NULL == RT_MAPPER->phy_common_c45_duplex_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->phy_common_c45_duplex_set(port, duplex);
    RTK_API_UNLOCK();

    return retVal;

}

/* Function Name:
 *      rtk_phy_common_c45_duplex_get
 * Description:
 *      Get duplex of the specific port
 * Input:
 *      unit - unit id
 *      port - port id
 * Output:
 *      pDuplex - pointer to PHY duplex mode status
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
rtk_api_ret_t rtk_phy_common_c45_duplex_get(rtk_port_t port, rtk_port_duplex_t *pDuplex)
{
    rtk_api_ret_t retVal;
    if (NULL == RT_MAPPER->phy_common_c45_duplex_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->phy_common_c45_duplex_get(port, pDuplex);
    RTK_API_UNLOCK();

    return retVal;

}

/* Function Name:
 *      rtk_phy_common_c45_speedDuplexStatusResReg_get
 * Description:
 *      Get operational link speed-duplex status from proprietary register
 * Input:
 *      port    - port id
 * Output:
 *      pSpeed - pointer to PHY operational link speed
 *      pDuplex - pointer to PHY operational Duplex
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - Failed
 * Note:
 *      Only PHY that supports proprietary resolution register can use this driver!
 */
rtk_int32 rtk_phy_common_c45_speedDuplexStatusResReg_get(rtk_port_t port, rtk_port_speed_t *pSpeed, rtk_port_duplex_t *pDuplex)
{
    rtk_api_ret_t retVal;
    if (NULL == RT_MAPPER->phy_common_c45_speedDuplexStatusResReg_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->phy_common_c45_speedDuplexStatusResReg_get(port, pSpeed, pDuplex);
    RTK_API_UNLOCK();

    return retVal;

}
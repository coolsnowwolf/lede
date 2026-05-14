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
 * Purpose : RTK switch high-level API for RTL8367/RTL8367C
 * Feature : Here is a list of all functions and variables in Trunk module.
 *
 */

#include <rtk_switch.h>
#include <rtk_error.h>
#include <trunk.h>
#include <string.h>

#include <dal/dal_mgmt.h>

#if 1
/* Function Name:
 *      rtk_port_autoDos_set
 * Description:
 *      Set Auto Dos state
 * Input:
 *      type        - Auto DoS type
 *      state       - 1: Eanble(Drop), 0: Disable(Forward)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 * Note:
 *      The API can set Auto Dos state
 */
rtk_api_ret_t rtk_port_autoDos_set(rtk_port_autoDosType_t type, rtk_enable_t state)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->port_autoDos_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->port_autoDos_set(type, state);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_port_autoDos_get
 * Description:
 *      Get Auto Dos state
 * Input:
 *      type        - Auto DoS type
 * Output:
 *      pState      - 1: Eanble(Drop), 0: Disable(Forward)
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_NULL_POINTER         - Null Pointer
 * Note:
 *      The API can get Auto Dos state
 */
rtk_api_ret_t rtk_port_autoDos_get(rtk_port_autoDosType_t type, rtk_enable_t *pState)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->port_autoDos_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->port_autoDos_get(type, pState);
    RTK_API_UNLOCK();

    return retVal;
}

#endif


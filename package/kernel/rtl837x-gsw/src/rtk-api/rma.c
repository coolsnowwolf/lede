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
 * Feature : Here is a list of all functions and variables in RMA module.
 *
 */

#include <rtk_switch.h>
#include <rtk_error.h>
#include <rma.h>
#include <string.h>

#include <dal/dal_mgmt.h>



/* Function Name:
 *      rtk_rma_set
 * Description:
 *      Set RMA action, priority & leaky function.
 * Input:
 *      rmaAddr: 0x00 ~ 0x2f
 *      rmaParam:
 *            operation;
 *            discard_storm_filter;
 *            trap_priority;
 *            keep_format;
 *            vlan_leaky;
 *            portiso_leaky;
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      Reset MIB counter of ports. API will use global reset while port mask is all-ports.
 */
rtk_api_ret_t rtk_rma_set(rtk_uint32 rmaAddr, rtk_rmaParam_t* rmaParam)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->rma_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->rma_set(rmaAddr, rmaParam);
    RTK_API_UNLOCK();

    return retVal;
}



/* Function Name:
 *      rtk_rma_get
 * Description:
 *      Get RMA action, priority & leaky function.
 * Input:
 *      rmaAddr: 0x00 ~ 0x2f
 *      rmaParam:
 *            operation;
 *            discard_storm_filter;
 *            trap_priority;
 *            keep_format;
 *            vlan_leaky;
 *            portiso_leaky;
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      Reset MIB counter of ports. API will use global reset while port mask is all-ports.
 */
rtk_api_ret_t rtk_rma_get(rtk_uint32 rmaAddr, rtk_rmaParam_t* rmaParam)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->rma_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->rma_get(rmaAddr, rmaParam);
    RTK_API_UNLOCK();

    return retVal;
}



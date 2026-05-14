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
 * Feature : Here is a list of all functions and variables in ISOLATION module.
 *
 */

 #include <rtk_switch.h>
#include <rtk_error.h>
#include <isolation.h>
#include <string.h>

#include <dal/dal_mgmt.h>


/* Function Name:
 *      rtk_port_isolation_set
 * Description:
 *      Set permitted port isolation portmask
 * Input:
 *      port            - Physical port number (0~9)
 *      permitPortmask  - port mask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number
 *      RT_ERR_PORT_MASK    - Invalid portmask
 * Note:
 *      None
 */
ret_t rtk_port_isolation_set(rtk_port_t port, rtk_uint32 permitPortmask)
{
	    rtk_api_ret_t retVal;

	    if (NULL == RT_MAPPER->port_isolation_set)
	        return RT_ERR_DRIVER_NOT_FOUND;

	    RTK_API_LOCK();
	    retVal = RT_MAPPER->port_isolation_set(port, permitPortmask);
	    RTK_API_UNLOCK();

	    return retVal;
}

/* Function Name:
 *      rtk_port_isolation_get
 * Description:
 *      Get permitted port isolation portmask
 * Input:
 *      port                - Physical port number (0~9)
 *      pPermitPortmask     - port mask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number
 * Note:
 *      None
 */
ret_t rtk_port_isolation_get(rtk_port_t port, rtk_uint32 *pPermitPortmask)
{
		rtk_api_ret_t retVal;

		if (NULL == RT_MAPPER->port_isolation_get)
		return RT_ERR_DRIVER_NOT_FOUND;

		RTK_API_LOCK();
		retVal = RT_MAPPER->port_isolation_get(port, pPermitPortmask);
		RTK_API_UNLOCK();

		return retVal;
}



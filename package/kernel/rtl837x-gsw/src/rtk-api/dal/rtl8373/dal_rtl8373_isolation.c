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
#include <dal/rtl8373/dal_rtl8373_isolation.h>
#include <string.h>

#include <dal/rtl8373/rtl8373_asicdrv.h>

/* Function Name:
 *      dal_rtl8373_port_isolation_set
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
ret_t dal_rtl8373_port_isolation_set(rtk_port_t port, rtk_uint32 permitPortmask)
{
    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);

    if( permitPortmask > RTL8373_PORTMASK)
        return RT_ERR_PORT_MASK;

    return rtl8373_setAsicReg(RTL8373_PORT_ISO_PORT_PMSK_ADDR(port), permitPortmask);
}

/* Function Name:
 *      dal_rtl8373_port_isolation_get
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
ret_t dal_rtl8373_port_isolation_get(rtk_port_t port, rtk_uint32 *pPermitPortmask)
{
    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);

    if(NULL == pPermitPortmask)
        return RT_ERR_NULL_POINTER;

    return rtl8373_getAsicReg(RTL8373_PORT_ISO_PORT_PMSK_ADDR(port), pPermitPortmask);
}








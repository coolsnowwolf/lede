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
 * Purpose : RTK switch high-level API for RTL8367/RTL8373
 * Feature : Here is a list of all functions and variables in RMA module.
 *
 */

#include <rtk_switch.h>
#include <rtk_error.h>
#include <dal_rtl8373_dos.h>
#include <rtl8373_asicdrv.h>
#include <string.h>


/* Function Name:
 *      rtl8373_setAsicDos
 * Description:
 *      Set asic dos configuration
 * Input:
 *      index     - dos type
 *      enable     - 1:enable, 0:disable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t dal_rtl8373_asicDos_set(rtk_port_autoDosType_t index, rtk_uint32 enable)
{
    ret_t retVal;


    retVal = rtl8373_setAsicRegBit(RTL8373_ATK_PRVNT_CTRL_ADDR, index, enable);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}


/* Function Name:
 *      rtl8373_getAsicDos
 * Description:
 *      Get asic dos configuration
 * Input:
 *      index     - dos type
 *      pEnable     - 1:enable, 0:disable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t dal_rtl8373_asicDos_get(rtk_port_autoDosType_t index, rtk_uint32* pEnable)
{
    ret_t retVal;

    retVal = rtl8373_getAsicRegBit(RTL8373_ATK_PRVNT_CTRL_ADDR, index, pEnable);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}










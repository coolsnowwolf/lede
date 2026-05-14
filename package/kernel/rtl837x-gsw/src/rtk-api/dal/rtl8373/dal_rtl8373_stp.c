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
#include <dal_rtl8373_stp.h>
#include <rtl8373_asicdrv.h>
#include <string.h>


/* Function Name:
 *      dal_rtl8373_asicMstpPortStatus_set
 * Description:
 *      Set MSTP port status
 * Input:
 *      fid     - mstp index
 *      port     - 
 *      statys  - 0 disable  1 blocking     2 learning    3 forwarding
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t dal_rtl8373_asicMstpPortStatus_set(rtk_uint32 fid, rtk_uint32 port, rtk_uint32 status)
{
    ret_t retVal;

    if(fid > RTL8373_FIDMAX)
        return RT_ERR_INPUT;


    retVal = rtl8373_setAsicRegBits(RTL8373_MSPT_STATE_ADDR(fid), 0x3<<(port*2), status);
    if(retVal != RT_ERR_OK)
        return retVal;

    return retVal;
}


/* Function Name:
 *      dal_rtl8373_asicMstpPortStatus_get
 * Description:
 *      Get MSTP port status
 * Input:
 *      fid     - mstp index
 *      port     - 
 *      *pStatys  - 0 disable  1 blocking     2 learning    3 forwarding
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t dal_rtl8373_asicMstpPortStatus_get(rtk_uint32 fid, rtk_uint32 port, rtk_uint32* pStatus)
{
    ret_t retVal;

    if(fid > RTL8373_FIDMAX)
        return RT_ERR_INPUT;


    retVal = rtl8373_getAsicRegBits(RTL8373_MSPT_STATE_ADDR(fid), 0x3<<(port*2), pStatus);
    if(retVal != RT_ERR_OK)
        return retVal;

    return retVal;
}








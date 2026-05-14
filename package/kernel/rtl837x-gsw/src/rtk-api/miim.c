/*
 * Copyright (C) 2010 Realtek Semiconductor Corp.
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
 * Purpose : RTK switch high-level API
 * Feature : Here is a list of all functions and variables in this module.
 *
 */
 //#include "phydef.h"
 #include <rtk_switch.h>
#include <rtk_error.h>
#include "miim.h"
#include "ptp.h"
#include "phydef.h"

#include <string.h>
/* Function Name:
 *      phy_ptpRefTime_set
 * Description:
 *      Set the reference time of PHY of the specified port.
 * Input:
 *      timeStamp - reference timestamp value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 *      RT_ERR_PORT_NOT_SUPPORTED   - This function is not supported by the PHY of this port
 * Note:
 *      None
 */
ret_t phy_ptpRefTime_set( rtk_time_timeStamp_t timeStamp)
{
    rtk_switch_halCtrl_t   *pHalCtrl;
    ret_t           ret=0;
    
    if ((pHalCtrl = hal_ctrlInfo_get()) == NULL)
        return RT_ERR_FAILED;
    RTK_API_LOCK();
    ret = (pHalCtrl->pPhy_ctrl->pPhydrv->fPhydrv_ptpRefTime_set(timeStamp));
    RTK_API_UNLOCK();
    return ret;
}



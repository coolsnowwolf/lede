/*
 * Copyright (C) 2009-2016 Realtek Semiconductor Corp.
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
 * Purpose : PHY identify service APIs in the SDK.
 *
 * Feature : PHY identify service APIs
 *
 */

#ifndef __HAL_PHY_IDENTIFY_H__
#define __HAL_PHY_IDENTIFY_H__
#include "phydef.h"
/* Function Name:
 *      phy_identify_driver_find_blind
 * Description:
 *      Find PHY driver from all drivers
 * Input:
 * Output:
 *      None
 * Return:
 *      Otherwise - Pointer of PHY control that found
 * Note:
 *      None
 */
extern rt_phyctrl_t * phy_identify_driver_find_blindly(void);


/* Function Name:
 *      phy_identify_init
 * Description:
 *      Initial identify databases
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
extern rtk_api_ret_t phy_identify_init(void);


#endif
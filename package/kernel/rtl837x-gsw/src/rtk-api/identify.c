
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
#include <rtk_types.h>
#include <rtk_error.h>
#include "miim.h"
#include "ptp.h"
#include "identify.h"
#if defined(CONFIG_SDK_RTL8224)
#include "phy_rtl8224.h"
#endif
#include <string.h>

#if defined(CONFIG_SDK_RTL8224)
static rt_phyInfo_t rtl8224_hal_Ctrl =
{
    4,
    HWP_2_5GE
};
/* Function Name:
 *      _phy_identify_8371_serdes
 * Description:
 *      Identify the port is 8371 intra serdes PHY or not?
 * Input:
 *      model_id - model id
 *      rev_id   - revision id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - is intra serdes PHY
 *      RT_ERR_FAILED           - access failure or others
 *      RT_ERR_PHY_NOT_MATCH    - is not intra serdes PHY
 * Note:
 *      None
 */
static ret_t _phy_identify_8371_serdes(rtk_uint32 model_id, rtk_uint32 rev_id)
{

    model_id=rev_id=0;
    rev_id=model_id;
    
    return RT_ERR_OK;
}
#endif

/* supported internal PHY chip lists */
static rt_phyctrl_t supported_phys[] =
{
#if defined(CONFIG_SDK_RTL8224)
    {_phy_identify_8371_serdes, RTL8371_FAMILY_ID, PHY_MODEL_ID_NULL, RTK_PHYTYPE_RTL8224,NULL, phy_8224drv_mapper_get, 0, &rtl8224_hal_Ctrl},
#endif 
};


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
rt_phyctrl_t * phy_identify_driver_find_blindly(void)
{
    rtk_int32  size = 0, i;

    size = sizeof(supported_phys) / sizeof(rt_phyctrl_t);
    for (i = size - 1; i >= 0; i--)
    {
        if ((supported_phys[i].chk_func)(  supported_phys[i].phy_model_id, supported_phys[i].phy_rev_id) == RT_ERR_OK)
        {
            return (&supported_phys[i]);
        }
    }
    return NULL;
}

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
rtk_api_ret_t phy_identify_init(void)
{ 
   rtk_switch_halCtrl_t *pHalCtrl ;
   
    pHalCtrl = hal_ctrlInfo_get();
    pHalCtrl->pPhy_ctrl= phy_identify_driver_find_blindly();
    if(pHalCtrl->pPhy_ctrl != NULL)
    {
        pHalCtrl->pPhy_ctrl->pPhydrv = pHalCtrl->pPhy_ctrl->mapperInit_func();      
    }    

    return RT_ERR_OK;
}


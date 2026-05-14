/*
 * Copyright (C) 2018 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 * Purpose : chip symbol and data type definition in the SDK.
 *
 * Feature : chip symbol and data type definition
 *
 */

#if (!defined(CONFIG_DAL_RTL8373) && !defined(CONFIG_DAL_RTL8370UG))
#define CONFIG_DAL_ALL
#endif



#include <rtk_error.h>
#include <chip.h>
#include "identify.h"
#if defined(CONFIG_DAL_RTL8373) || defined(CONFIG_DAL_ALL)
#include <dal/rtl8373/rtl8373_asicdrv.h>
#endif
//#if defined(CONFIG_DAL_RTL8370UG) || defined(CONFIG_DAL_ALL)
//#include <dal/rtl8370ug/rtl8370ug_asicdrv.h>
//#endif

#if (!defined(FORCE_PROBE_RTL8373) && !defined(FORCE_PROBE_RTL8370UG) )
#define AUTO_PROBE 1
#else
#define AUTO_PROBE 0
#endif

#if (AUTO_PROBE || defined(FORCE_PROBE_RTL8373))
static rtk_switch_halCtrl_t rtl8371c_hal_Ctrl =
{
    /* Switch Chip */
    CHIP_RTL8373,

    /* Logical to Physical */
    {0, 1, 2, 3, 4, 5, 6, 7, 8, 9},

    /* Physical to Logical */
    {UTP_PORT0, UTP_PORT1, UTP_PORT2, UTP_PORT3, UTP_PORT4, UTP_PORT5, UTP_PORT6, UTP_PORT7, UTP_PORT8, UTP_PORT9},

    /* Port Type */
    {UTP_PORT, UTP_PORT, UTP_PORT, UTP_PORT, UTP_PORT, UTP_PORT, UTP_PORT, UTP_PORT, UTP_PORT, UTP_PORT},

    /* PTP port */
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },

    /* Valid port mask */
    ( (0x1 << UTP_PORT0) | (0x1 << UTP_PORT1) | (0x1 << UTP_PORT2) | (0x1 << UTP_PORT3) | (0x1 << UTP_PORT4) | (0x1 << UTP_PORT5) | (0x1 << UTP_PORT6) | (0x1 << UTP_PORT7) | (0x1 << UTP_PORT8) | (0x1 << UTP_PORT9) ),

    /* Valid UTP port mask */
    ( (0x1 << UTP_PORT0) | (0x1 << UTP_PORT1) | (0x1 << UTP_PORT2) | (0x1 << UTP_PORT3) | (0x1 << UTP_PORT4) | (0x1 << UTP_PORT5) | (0x1 << UTP_PORT6) | (0x1 << UTP_PORT7) | (0x1 << UTP_PORT8) | (0x1 << UTP_PORT9)),

    /* Valid EXT port mask */
    0x0,

    /* Valid CPU port mask */
    0x200,

    /* Minimum physical port number */
    0,

    /* Maxmum physical port number */
    9,

    /* Physical port mask */
    0x3FF,

    /* Combo Logical port ID */
    0,

    /* HSG Logical port ID */
    1,

    /* SGMII Logical portmask */
    (0x1 ),

    /* Max Meter ID */
    63,

    /* MAX LUT Address Number */
    4160,

    /* Trunk Group Mask */
    0x0f,
    
    /* Packet buffer page number */
    1024,
    
    /* default pPhy_ctrl */
    NULL
};
#endif


/* Function Name:
 *      switch_probe
 * Description:
 *      Probe switch
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Switch probed
 *      RT_ERR_FAILED   - Switch Unprobed.
 * Note:
 *
 */
rtk_api_ret_t switch_probe(switch_chip_t *pSwitchChip)
{
    rtk_uint32 retVal;
    rtk_uint32 regdata;
	
    if((retVal = rtl8373_getAsicReg(0x4, &regdata)) != RT_ERR_OK)
    	return retVal;

    regdata = regdata >> 8;

    if (regdata == 0x837300)
    {
        *pSwitchChip  = CHIP_RTL8373;
    }
    else if (regdata == 0x837200)
    {
        *pSwitchChip  = CHIP_RTL8372;
    }
    else if (regdata == 0x822400)
    {
        *pSwitchChip  = CHIP_RTL8224;
    }
    else if (regdata == 0x837370)
    {
        *pSwitchChip  = CHIP_RTL8373N;		 
    }
    else if (regdata == 0x837270)
    {
        *pSwitchChip  = CHIP_RTL8372N;		 
    }
    else if (regdata == 0x822470)
    {
        *pSwitchChip  = CHIP_RTL8224N;		 
    }
    else if (regdata == 0x8366A8)
    {
        *pSwitchChip  = CHIP_RTL8366U;
    }    
    else
        return RT_ERR_CHIP_NOT_FOUND;


    return RT_ERR_OK;        
}

/* Function Name:
 *      hal_find_device
 * Description:
 *      Find the mac chip from SDK supported mac device lists.
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      NULL        - Not found
 *      Otherwise   - Pointer of mac chip HAL structure that found
 * Note:
 *
 */
rtk_switch_halCtrl_t *hal_find_device(void)
{
    switch_chip_t switchChip;
    rtk_uint32  retVal;
  
    /* probe switch */
    if((retVal = switch_probe(&switchChip)) != RT_ERR_OK)
        return NULL;

    switch (switchChip)
    {
#if (AUTO_PROBE || defined(FORCE_PROBE_RTL8373))
        case CHIP_RTL8373:
        case CHIP_RTL8372:
        case CHIP_RTL8224:
        case CHIP_RTL8373N:
        case CHIP_RTL8372N:
        case CHIP_RTL8366U:
        case CHIP_RTL8224N:
            return &rtl8371c_hal_Ctrl;
#endif

        default:
            return NULL;
    }

    /* Not found */
    return NULL;
}

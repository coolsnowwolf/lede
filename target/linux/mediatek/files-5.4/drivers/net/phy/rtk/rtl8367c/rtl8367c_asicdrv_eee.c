/*
 * Copyright (C) 2013 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * Unless you and Realtek execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2,
 * available at https://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 *
 * $Revision: 48989 $
 * $Date: 2014-07-01 15:45:24 +0800 (週二, 01 七月 2014) $
 *
 * Purpose : RTL8370 switch high-level API for RTL8367C
 * Feature :
 *
 */

#include <rtl8367c_asicdrv_eee.h>
#include <rtl8367c_asicdrv_phy.h>

/*
@func ret_t | rtl8367c_setAsicEee100M | Set eee force mode function enable/disable.
@parm rtk_uint32 | port | The port number.
@parm rtk_uint32 | enabled | 1: enabled, 0: disabled.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    This API set the 100M EEE enable function.

*/
ret_t rtl8367c_setAsicEee100M(rtk_uint32 port, rtk_uint32 enable)
{
    rtk_api_ret_t   retVal;
    rtk_uint32      regData;

    if(port >= RTL8367C_PORTNO)
        return RT_ERR_PORT_ID;

    if (enable > 1)
        return RT_ERR_INPUT;

    if((retVal = rtl8367c_getAsicPHYOCPReg(port, EEE_OCP_PHY_ADDR, &regData)) != RT_ERR_OK)
        return retVal;

    if(enable)
        regData |= (0x0001 << 1);
    else
        regData &= ~(0x0001 << 1);

    if((retVal = rtl8367c_setAsicPHYOCPReg(port, EEE_OCP_PHY_ADDR, regData)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/*
@func ret_t | rtl8367c_getAsicEee100M | Get 100M eee enable/disable.
@parm rtk_uint32 | port | The port number.
@parm rtk_uint32* | enabled | 1: enabled, 0: disabled.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    This API get the 100M EEE function.
*/
ret_t rtl8367c_getAsicEee100M(rtk_uint32 port, rtk_uint32 *enable)
{
    rtk_api_ret_t   retVal;
    rtk_uint32      regData;

    if(port >= RTL8367C_PORTNO)
        return RT_ERR_PORT_ID;

    if((retVal = rtl8367c_getAsicPHYOCPReg(port, EEE_OCP_PHY_ADDR, &regData)) != RT_ERR_OK)
        return retVal;

    *enable = (regData & (0x0001 << 1)) ? ENABLED : DISABLED;
    return RT_ERR_OK;
}

/*
@func ret_t | rtl8367c_setAsicEeeGiga | Set eee force mode function enable/disable.
@parm rtk_uint32 | port | The port number.
@parm rtk_uint32 | enabled | 1: enabled, 0: disabled.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    This API set the 100M EEE enable function.

*/
ret_t rtl8367c_setAsicEeeGiga(rtk_uint32 port, rtk_uint32 enable)
{
    rtk_api_ret_t   retVal;
    rtk_uint32      regData;

    if(port >= RTL8367C_PORTNO)
        return RT_ERR_PORT_ID;

    if (enable > 1)
        return RT_ERR_INPUT;

    if((retVal = rtl8367c_getAsicPHYOCPReg(port, EEE_OCP_PHY_ADDR, &regData)) != RT_ERR_OK)
        return retVal;

    if(enable)
        regData |= (0x0001 << 2);
    else
        regData &= ~(0x0001 << 2);

    if((retVal = rtl8367c_setAsicPHYOCPReg(port, EEE_OCP_PHY_ADDR, regData)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/*
@func ret_t | rtl8367c_getAsicEeeGiga | Get 100M eee enable/disable.
@parm rtk_uint32 | port | The port number.
@parm rtk_uint32* | enabled | 1: enabled, 0: disabled.
@rvalue RT_ERR_OK | Success.
@rvalue RT_ERR_SMI | SMI access error.
@rvalue RT_ERR_INPUT | Invalid input parameter.
@comm
    This API get the 100M EEE function.
*/
ret_t rtl8367c_getAsicEeeGiga(rtk_uint32 port, rtk_uint32 *enable)
{
    rtk_api_ret_t   retVal;
    rtk_uint32      regData;

    if(port >= RTL8367C_PORTNO)
        return RT_ERR_PORT_ID;

    if((retVal = rtl8367c_getAsicPHYOCPReg(port, EEE_OCP_PHY_ADDR, &regData)) != RT_ERR_OK)
        return retVal;

    *enable = (regData & (0x0001 << 2)) ? ENABLED : DISABLED;
    return RT_ERR_OK;
}

/*
 * Copyright (C) 2013 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * Unless you and Realtek execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2,
 * available at https://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 *
 * $Revision: 48156 $
 * $Date: 2014-05-29 16:39:06 +0800 (週四, 29 五月 2014) $
 *
 * Purpose : RTK switch high-level API for RTL8367/RTL8367C
 * Feature : Here is a list of all functions and variables in EEE module.
 *
 */

#include <rtk_switch.h>
#include <rtk_error.h>
#include <eee.h>
#include <string.h>

#include <rtl8367c_asicdrv.h>
#include <rtl8367c_asicdrv_eee.h>
#include <rtl8367c_asicdrv_phy.h>

/* Function Name:
 *      rtk_eee_init
 * Description:
 *      EEE function initialization.
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 * Note:
 *      This API is used to initialize EEE status.
 */
rtk_api_ret_t rtk_eee_init(void)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if((retVal = rtl8367c_setAsicRegBit(0x0018, 10, 1)) != RT_ERR_OK)
        return retVal;

    if((retVal = rtl8367c_setAsicRegBit(0x0018, 11, 1)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_eee_portEnable_set
 * Description:
 *      Set enable status of EEE function.
 * Input:
 *      port - port id.
 *      enable - enable EEE status.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 *      RT_ERR_ENABLE - Invalid enable input.
 * Note:
 *      This API can set EEE function to the specific port.
 *      The configuration of the port is as following:
 *      - DISABLE
 *      - ENABLE
 */
rtk_api_ret_t rtk_eee_portEnable_set(rtk_port_t port, rtk_enable_t enable)
{
    rtk_api_ret_t   retVal;
    rtk_uint32      regData;
    rtk_uint32    phy_port;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check port is UTP port */
    RTK_CHK_PORT_IS_UTP(port);

    if (enable>=RTK_ENABLE_END)
        return RT_ERR_INPUT;

    phy_port = rtk_switch_port_L2P_get(port);

    if ((retVal = rtl8367c_setAsicEee100M(phy_port,enable))!=RT_ERR_OK)
        return retVal;
    if ((retVal = rtl8367c_setAsicEeeGiga(phy_port,enable))!=RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8367c_setAsicPHYReg(phy_port, RTL8367C_PHY_PAGE_ADDRESS, 0))!=RT_ERR_OK)
        return retVal;
    if ((retVal = rtl8367c_getAsicPHYReg(phy_port, 0, &regData))!=RT_ERR_OK)
        return retVal;
    regData |= 0x0200;
    if ((retVal = rtl8367c_setAsicPHYReg(phy_port, 0, regData))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_eee_portEnable_get
 * Description:
 *      Get enable status of EEE function
 * Input:
 *      port - Port id.
 * Output:
 *      pEnable - Back pressure status.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_ID - Invalid port number.
 * Note:
 *      This API can get EEE function to the specific port.
 *      The configuration of the port is as following:
 *      - DISABLE
 *      - ENABLE
 */

rtk_api_ret_t rtk_eee_portEnable_get(rtk_port_t port, rtk_enable_t *pEnable)
{
    rtk_api_ret_t retVal;
    rtk_uint32 regData1, regData2;
    rtk_uint32    phy_port;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check port is UTP port */
    RTK_CHK_PORT_IS_UTP(port);

    if(NULL == pEnable)
        return RT_ERR_NULL_POINTER;

    phy_port = rtk_switch_port_L2P_get(port);

    if ((retVal = rtl8367c_getAsicEee100M(phy_port,&regData1))!=RT_ERR_OK)
        return retVal;
    if ((retVal = rtl8367c_getAsicEeeGiga(phy_port,&regData2))!=RT_ERR_OK)
        return retVal;

    if (regData1==1&&regData2==1)
        *pEnable = ENABLED;
    else
        *pEnable = DISABLED;

    return RT_ERR_OK;
}



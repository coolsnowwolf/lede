/*
 * Copyright (C) 2013 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * Unless you and Realtek execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2,
 * available at https://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 *
 * $Revision: 76306 $
 * $Date: 2017-03-08 15:13:58 +0800 (週三, 08 三月 2017) $
 *
 * Purpose : RTK switch high-level API for RTL8367/RTL8367C
 * Feature : Here is a list of all functions and variables in Storm module.
 *
 */

#include <rtk_switch.h>
#include <rtk_error.h>
#include <storm.h>
#include <rate.h>
#include <string.h>
#include <rtl8367c_asicdrv.h>
#include <rtl8367c_asicdrv_storm.h>
#include <rtl8367c_asicdrv_meter.h>
#include <rtl8367c_asicdrv_rma.h>
#include <rtl8367c_asicdrv_igmp.h>

/* Function Name:
 *      rtk_rate_stormControlMeterIdx_set
 * Description:
 *      Set the storm control meter index.
 * Input:
 *      port       - port id
 *      storm_type - storm group type
 *      index       - storm control meter index.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID - Invalid port id
 *      RT_ERR_FILTER_METER_ID  - Invalid meter
 * Note:
 *
 */
rtk_api_ret_t rtk_rate_stormControlMeterIdx_set(rtk_port_t port, rtk_rate_storm_group_t stormType, rtk_uint32 index)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);

    if (stormType >= STORM_GROUP_END)
        return RT_ERR_SFC_UNKNOWN_GROUP;

    if (index > RTK_MAX_METER_ID)
        return RT_ERR_FILTER_METER_ID;

    switch (stormType)
    {
        case STORM_GROUP_UNKNOWN_UNICAST:
            if ((retVal = rtl8367c_setAsicStormFilterUnknownUnicastMeter(rtk_switch_port_L2P_get(port), index))!=RT_ERR_OK)
                return retVal;
            break;
        case STORM_GROUP_UNKNOWN_MULTICAST:
            if ((retVal = rtl8367c_setAsicStormFilterUnknownMulticastMeter(rtk_switch_port_L2P_get(port), index))!=RT_ERR_OK)
                return retVal;
            break;
        case STORM_GROUP_MULTICAST:
            if ((retVal = rtl8367c_setAsicStormFilterMulticastMeter(rtk_switch_port_L2P_get(port), index))!=RT_ERR_OK)
                return retVal;
            break;
        case STORM_GROUP_BROADCAST:
            if ((retVal = rtl8367c_setAsicStormFilterBroadcastMeter(rtk_switch_port_L2P_get(port), index))!=RT_ERR_OK)
                return retVal;
            break;
        default:
            break;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_rate_stormControlMeterIdx_get
 * Description:
 *      Get the storm control meter index.
 * Input:
 *      port       - port id
 *      storm_type - storm group type
 * Output:
 *      pIndex     - storm control meter index.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID - Invalid port id
 *      RT_ERR_FILTER_METER_ID  - Invalid meter
 * Note:
 *
 */
rtk_api_ret_t rtk_rate_stormControlMeterIdx_get(rtk_port_t port, rtk_rate_storm_group_t stormType, rtk_uint32 *pIndex)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);

    if (stormType >= STORM_GROUP_END)
        return RT_ERR_SFC_UNKNOWN_GROUP;

    if (NULL == pIndex )
        return RT_ERR_NULL_POINTER;

    switch (stormType)
    {
        case STORM_GROUP_UNKNOWN_UNICAST:
            if ((retVal = rtl8367c_getAsicStormFilterUnknownUnicastMeter(rtk_switch_port_L2P_get(port), pIndex))!=RT_ERR_OK)
                return retVal;
            break;
        case STORM_GROUP_UNKNOWN_MULTICAST:
            if ((retVal = rtl8367c_getAsicStormFilterUnknownMulticastMeter(rtk_switch_port_L2P_get(port), pIndex))!=RT_ERR_OK)
                return retVal;
            break;
        case STORM_GROUP_MULTICAST:
            if ((retVal = rtl8367c_getAsicStormFilterMulticastMeter(rtk_switch_port_L2P_get(port), pIndex))!=RT_ERR_OK)
                return retVal;
            break;
        case STORM_GROUP_BROADCAST:
            if ((retVal = rtl8367c_getAsicStormFilterBroadcastMeter(rtk_switch_port_L2P_get(port), pIndex))!=RT_ERR_OK)
                return retVal;
            break;
        default:
            break;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_rate_stormControlPortEnable_set
 * Description:
 *      Set enable status of storm control on specified port.
 * Input:
 *      port       - port id
 *      stormType  - storm group type
 *      enable     - enable status of storm control
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT          - The module is not initial
 *      RT_ERR_PORT_ID           - invalid port id
 *      RT_ERR_INPUT             - invalid input parameter
 * Note:
 *
 */
rtk_api_ret_t rtk_rate_stormControlPortEnable_set(rtk_port_t port, rtk_rate_storm_group_t stormType, rtk_enable_t enable)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);

    if (stormType >= STORM_GROUP_END)
        return RT_ERR_SFC_UNKNOWN_GROUP;

    if (enable >= RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    switch (stormType)
    {
        case STORM_GROUP_UNKNOWN_UNICAST:
            if ((retVal = rtl8367c_setAsicStormFilterUnknownUnicastEnable(rtk_switch_port_L2P_get(port), enable)) != RT_ERR_OK)
                return retVal;
            break;
        case STORM_GROUP_UNKNOWN_MULTICAST:
            if ((retVal = rtl8367c_setAsicStormFilterUnknownMulticastEnable(rtk_switch_port_L2P_get(port), enable)) != RT_ERR_OK)
                return retVal;
            break;
        case STORM_GROUP_MULTICAST:
            if ((retVal = rtl8367c_setAsicStormFilterMulticastEnable(rtk_switch_port_L2P_get(port), enable)) != RT_ERR_OK)
                return retVal;
            break;
        case STORM_GROUP_BROADCAST:
            if ((retVal = rtl8367c_setAsicStormFilterBroadcastEnable(rtk_switch_port_L2P_get(port), enable)) != RT_ERR_OK)
                return retVal;
            break;
        default:
            break;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_rate_stormControlPortEnable_set
 * Description:
 *      Set enable status of storm control on specified port.
 * Input:
 *      port       - port id
 *      stormType  - storm group type
 * Output:
 *      pEnable     - enable status of storm control
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT          - The module is not initial
 *      RT_ERR_PORT_ID           - invalid port id
 *      RT_ERR_INPUT             - invalid input parameter
 * Note:
 *
 */
rtk_api_ret_t rtk_rate_stormControlPortEnable_get(rtk_port_t port, rtk_rate_storm_group_t stormType, rtk_enable_t *pEnable)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);

    if (stormType >= STORM_GROUP_END)
        return RT_ERR_SFC_UNKNOWN_GROUP;

    if (NULL == pEnable)
        return RT_ERR_ENABLE;

    switch (stormType)
    {
        case STORM_GROUP_UNKNOWN_UNICAST:
            if ((retVal = rtl8367c_getAsicStormFilterUnknownUnicastEnable(rtk_switch_port_L2P_get(port), pEnable)) != RT_ERR_OK)
                return retVal;
            break;
        case STORM_GROUP_UNKNOWN_MULTICAST:
            if ((retVal = rtl8367c_getAsicStormFilterUnknownMulticastEnable(rtk_switch_port_L2P_get(port), pEnable)) != RT_ERR_OK)
                return retVal;
            break;
        case STORM_GROUP_MULTICAST:
            if ((retVal = rtl8367c_getAsicStormFilterMulticastEnable(rtk_switch_port_L2P_get(port), pEnable)) != RT_ERR_OK)
                return retVal;
            break;
        case STORM_GROUP_BROADCAST:
            if ((retVal = rtl8367c_getAsicStormFilterBroadcastEnable(rtk_switch_port_L2P_get(port), pEnable)) != RT_ERR_OK)
                return retVal;
            break;
        default:
            break;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_storm_bypass_set
 * Description:
 *      Set bypass storm filter control configuration.
 * Input:
 *      type    - Bypass storm filter control type.
 *      enable  - Bypass status.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_ENABLE       - Invalid IFG parameter
 * Note:
 *
 *      This API can set per-port bypass storm filter control frame type including RMA and IGMP.
 *      The bypass frame type is as following:
 *      - BYPASS_BRG_GROUP,
 *      - BYPASS_FD_PAUSE,
 *      - BYPASS_SP_MCAST,
 *      - BYPASS_1X_PAE,
 *      - BYPASS_UNDEF_BRG_04,
 *      - BYPASS_UNDEF_BRG_05,
 *      - BYPASS_UNDEF_BRG_06,
 *      - BYPASS_UNDEF_BRG_07,
 *      - BYPASS_PROVIDER_BRIDGE_GROUP_ADDRESS,
 *      - BYPASS_UNDEF_BRG_09,
 *      - BYPASS_UNDEF_BRG_0A,
 *      - BYPASS_UNDEF_BRG_0B,
 *      - BYPASS_UNDEF_BRG_0C,
 *      - BYPASS_PROVIDER_BRIDGE_GVRP_ADDRESS,
 *      - BYPASS_8021AB,
 *      - BYPASS_UNDEF_BRG_0F,
 *      - BYPASS_BRG_MNGEMENT,
 *      - BYPASS_UNDEFINED_11,
 *      - BYPASS_UNDEFINED_12,
 *      - BYPASS_UNDEFINED_13,
 *      - BYPASS_UNDEFINED_14,
 *      - BYPASS_UNDEFINED_15,
 *      - BYPASS_UNDEFINED_16,
 *      - BYPASS_UNDEFINED_17,
 *      - BYPASS_UNDEFINED_18,
 *      - BYPASS_UNDEFINED_19,
 *      - BYPASS_UNDEFINED_1A,
 *      - BYPASS_UNDEFINED_1B,
 *      - BYPASS_UNDEFINED_1C,
 *      - BYPASS_UNDEFINED_1D,
 *      - BYPASS_UNDEFINED_1E,
 *      - BYPASS_UNDEFINED_1F,
 *      - BYPASS_GMRP,
 *      - BYPASS_GVRP,
 *      - BYPASS_UNDEF_GARP_22,
 *      - BYPASS_UNDEF_GARP_23,
 *      - BYPASS_UNDEF_GARP_24,
 *      - BYPASS_UNDEF_GARP_25,
 *      - BYPASS_UNDEF_GARP_26,
 *      - BYPASS_UNDEF_GARP_27,
 *      - BYPASS_UNDEF_GARP_28,
 *      - BYPASS_UNDEF_GARP_29,
 *      - BYPASS_UNDEF_GARP_2A,
 *      - BYPASS_UNDEF_GARP_2B,
 *      - BYPASS_UNDEF_GARP_2C,
 *      - BYPASS_UNDEF_GARP_2D,
 *      - BYPASS_UNDEF_GARP_2E,
 *      - BYPASS_UNDEF_GARP_2F,
 *      - BYPASS_IGMP.
 *      - BYPASS_CDP.
 *      - BYPASS_CSSTP.
 *      - BYPASS_LLDP.
 */
rtk_api_ret_t rtk_storm_bypass_set(rtk_storm_bypass_t type, rtk_enable_t enable)
{
    rtk_api_ret_t retVal;
    rtl8367c_rma_t rmacfg;
    rtk_uint32 tmp;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (type >= BYPASS_END)
        return RT_ERR_INPUT;

    if (enable >= RTK_ENABLE_END)
        return RT_ERR_INPUT;

    if (type >= 0 && type <= BYPASS_UNDEF_GARP_2F)
    {
        if ((retVal = rtl8367c_getAsicRma(type, &rmacfg)) != RT_ERR_OK)
            return retVal;

        rmacfg.discard_storm_filter = enable;

        if ((retVal = rtl8367c_setAsicRma(type, &rmacfg)) != RT_ERR_OK)
            return retVal;
    }
    else if(type == BYPASS_IGMP)
    {
        if ((retVal = rtl8367c_setAsicIGMPBypassStormCTRL(enable)) != RT_ERR_OK)
            return retVal;
    }
    else if (type == BYPASS_CDP)
    {
        if ((retVal = rtl8367c_getAsicRmaCdp(&rmacfg)) != RT_ERR_OK)
            return retVal;

        rmacfg.discard_storm_filter = enable;

        if ((retVal = rtl8367c_setAsicRmaCdp(&rmacfg)) != RT_ERR_OK)
            return retVal;
    }
    else if (type  == BYPASS_CSSTP)
    {
        if ((retVal = rtl8367c_getAsicRmaCsstp(&rmacfg)) != RT_ERR_OK)
            return retVal;

        rmacfg.discard_storm_filter = enable;

        if ((retVal = rtl8367c_setAsicRmaCsstp(&rmacfg)) != RT_ERR_OK)
            return retVal;
    }
    else if (type  == BYPASS_LLDP)
    {
        if ((retVal = rtl8367c_getAsicRmaLldp(&tmp, &rmacfg)) != RT_ERR_OK)
            return retVal;

        rmacfg.discard_storm_filter = enable;

        if ((retVal = rtl8367c_setAsicRmaLldp(tmp, &rmacfg)) != RT_ERR_OK)
            return retVal;
    }
    else
        return RT_ERR_INPUT;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_storm_bypass_get
 * Description:
 *      Get bypass storm filter control configuration.
 * Input:
 *      type - Bypass storm filter control type.
 * Output:
 *      pEnable - Bypass status.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API can get per-port bypass storm filter control frame type including RMA and IGMP.
 *      The bypass frame type is as following:
 *      - BYPASS_BRG_GROUP,
 *      - BYPASS_FD_PAUSE,
 *      - BYPASS_SP_MCAST,
 *      - BYPASS_1X_PAE,
 *      - BYPASS_UNDEF_BRG_04,
 *      - BYPASS_UNDEF_BRG_05,
 *      - BYPASS_UNDEF_BRG_06,
 *      - BYPASS_UNDEF_BRG_07,
 *      - BYPASS_PROVIDER_BRIDGE_GROUP_ADDRESS,
 *      - BYPASS_UNDEF_BRG_09,
 *      - BYPASS_UNDEF_BRG_0A,
 *      - BYPASS_UNDEF_BRG_0B,
 *      - BYPASS_UNDEF_BRG_0C,
 *      - BYPASS_PROVIDER_BRIDGE_GVRP_ADDRESS,
 *      - BYPASS_8021AB,
 *      - BYPASS_UNDEF_BRG_0F,
 *      - BYPASS_BRG_MNGEMENT,
 *      - BYPASS_UNDEFINED_11,
 *      - BYPASS_UNDEFINED_12,
 *      - BYPASS_UNDEFINED_13,
 *      - BYPASS_UNDEFINED_14,
 *      - BYPASS_UNDEFINED_15,
 *      - BYPASS_UNDEFINED_16,
 *      - BYPASS_UNDEFINED_17,
 *      - BYPASS_UNDEFINED_18,
 *      - BYPASS_UNDEFINED_19,
 *      - BYPASS_UNDEFINED_1A,
 *      - BYPASS_UNDEFINED_1B,
 *      - BYPASS_UNDEFINED_1C,
 *      - BYPASS_UNDEFINED_1D,
 *      - BYPASS_UNDEFINED_1E,
 *      - BYPASS_UNDEFINED_1F,
 *      - BYPASS_GMRP,
 *      - BYPASS_GVRP,
 *      - BYPASS_UNDEF_GARP_22,
 *      - BYPASS_UNDEF_GARP_23,
 *      - BYPASS_UNDEF_GARP_24,
 *      - BYPASS_UNDEF_GARP_25,
 *      - BYPASS_UNDEF_GARP_26,
 *      - BYPASS_UNDEF_GARP_27,
 *      - BYPASS_UNDEF_GARP_28,
 *      - BYPASS_UNDEF_GARP_29,
 *      - BYPASS_UNDEF_GARP_2A,
 *      - BYPASS_UNDEF_GARP_2B,
 *      - BYPASS_UNDEF_GARP_2C,
 *      - BYPASS_UNDEF_GARP_2D,
 *      - BYPASS_UNDEF_GARP_2E,
 *      - BYPASS_UNDEF_GARP_2F,
 *      - BYPASS_IGMP.
 *      - BYPASS_CDP.
 *      - BYPASS_CSSTP.
 *      - BYPASS_LLDP.
 */
rtk_api_ret_t rtk_storm_bypass_get(rtk_storm_bypass_t type, rtk_enable_t *pEnable)
{
    rtk_api_ret_t retVal;
    rtl8367c_rma_t rmacfg;
    rtk_uint32 tmp;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (type >= BYPASS_END)
        return RT_ERR_INPUT;

    if(NULL == pEnable)
        return RT_ERR_NULL_POINTER;

    if (type >= 0 && type <= BYPASS_UNDEF_GARP_2F)
    {
        if ((retVal = rtl8367c_getAsicRma(type, &rmacfg)) != RT_ERR_OK)
            return retVal;

        *pEnable = rmacfg.discard_storm_filter;
    }
    else if(type == BYPASS_IGMP)
    {
        if ((retVal = rtl8367c_getAsicIGMPBypassStormCTRL(pEnable)) != RT_ERR_OK)
            return retVal;
    }
    else if (type == BYPASS_CDP)
    {
        if ((retVal = rtl8367c_getAsicRmaCdp(&rmacfg)) != RT_ERR_OK)
            return retVal;

        *pEnable = rmacfg.discard_storm_filter;
    }
    else if (type == BYPASS_CSSTP)
    {
        if ((retVal = rtl8367c_getAsicRmaCsstp(&rmacfg)) != RT_ERR_OK)
            return retVal;

        *pEnable = rmacfg.discard_storm_filter;
    }
    else if (type == BYPASS_LLDP)
    {
        if ((retVal = rtl8367c_getAsicRmaLldp(&tmp,&rmacfg)) != RT_ERR_OK)
            return retVal;

        *pEnable = rmacfg.discard_storm_filter;
    }
    else
        return RT_ERR_INPUT;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_rate_stormControlExtPortmask_set
 * Description:
 *      Set extension storm control port mask
 * Input:
 *      pPortmask  - port mask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT          - The module is not initial
 *      RT_ERR_INPUT             - invalid input parameter
 * Note:
 *
 */
rtk_api_ret_t rtk_rate_stormControlExtPortmask_set(rtk_portmask_t *pPortmask)
{
    rtk_api_ret_t retVal;
    rtk_uint32 pmask;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pPortmask)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtk_switch_portmask_L2P_get(pPortmask, &pmask)) != RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8367c_setAsicStormFilterExtEnablePortMask(pmask)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_rate_stormControlExtPortmask_get
 * Description:
 *      Set extension storm control port mask
 * Input:
 *      None
 * Output:
 *      pPortmask  - port mask
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT          - The module is not initial
 *      RT_ERR_INPUT             - invalid input parameter
 * Note:
 *
 */
rtk_api_ret_t rtk_rate_stormControlExtPortmask_get(rtk_portmask_t *pPortmask)
{
    rtk_api_ret_t retVal;
    rtk_uint32 pmask;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pPortmask)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getAsicStormFilterExtEnablePortMask(&pmask)) != RT_ERR_OK)
        return retVal;

    if ((retVal = rtk_switch_portmask_P2L_get(pmask, pPortmask)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_rate_stormControlExtEnable_set
 * Description:
 *      Set extension storm control state
 * Input:
 *      stormType   - storm group type
 *      enable      - extension storm control state
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT          - The module is not initial
 *      RT_ERR_INPUT             - invalid input parameter
 * Note:
 *
 */
rtk_api_ret_t rtk_rate_stormControlExtEnable_set(rtk_rate_storm_group_t stormType, rtk_enable_t enable)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (stormType >= STORM_GROUP_END)
        return RT_ERR_SFC_UNKNOWN_GROUP;

    if (enable >= RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    switch (stormType)
    {
        case STORM_GROUP_UNKNOWN_UNICAST:
            if ((retVal = rtl8367c_setAsicStormFilterExtUnknownUnicastEnable(enable)) != RT_ERR_OK)
                return retVal;
            break;
        case STORM_GROUP_UNKNOWN_MULTICAST:
            if ((retVal = rtl8367c_setAsicStormFilterExtUnknownMulticastEnable(enable)) != RT_ERR_OK)
                return retVal;
            break;
        case STORM_GROUP_MULTICAST:
            if ((retVal = rtl8367c_setAsicStormFilterExtMulticastEnable(enable)) != RT_ERR_OK)
                return retVal;
            break;
        case STORM_GROUP_BROADCAST:
            if ((retVal = rtl8367c_setAsicStormFilterExtBroadcastEnable(enable)) != RT_ERR_OK)
                return retVal;
            break;
        default:
            break;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_rate_stormControlExtEnable_get
 * Description:
 *      Get extension storm control state
 * Input:
 *      stormType   - storm group type
 * Output:
 *      pEnable     - extension storm control state
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT          - The module is not initial
 *      RT_ERR_INPUT             - invalid input parameter
 * Note:
 *
 */
rtk_api_ret_t rtk_rate_stormControlExtEnable_get(rtk_rate_storm_group_t stormType, rtk_enable_t *pEnable)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (stormType >= STORM_GROUP_END)
        return RT_ERR_SFC_UNKNOWN_GROUP;

    if (NULL == pEnable)
        return RT_ERR_NULL_POINTER;

    switch (stormType)
    {
        case STORM_GROUP_UNKNOWN_UNICAST:
            if ((retVal = rtl8367c_getAsicStormFilterExtUnknownUnicastEnable((rtk_uint32 *)pEnable)) != RT_ERR_OK)
                return retVal;
            break;
        case STORM_GROUP_UNKNOWN_MULTICAST:
            if ((retVal = rtl8367c_getAsicStormFilterExtUnknownMulticastEnable((rtk_uint32 *)pEnable)) != RT_ERR_OK)
                return retVal;
            break;
        case STORM_GROUP_MULTICAST:
            if ((retVal = rtl8367c_getAsicStormFilterExtMulticastEnable((rtk_uint32 *)pEnable)) != RT_ERR_OK)
                return retVal;
            break;
        case STORM_GROUP_BROADCAST:
            if ((retVal = rtl8367c_getAsicStormFilterExtBroadcastEnable((rtk_uint32 *)pEnable)) != RT_ERR_OK)
                return retVal;
            break;
        default:
            break;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_rate_stormControlExtMeterIdx_set
 * Description:
 *      Set extension storm control meter index
 * Input:
 *      stormType   - storm group type
 *      index       - extension storm control state
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT          - The module is not initial
 *      RT_ERR_INPUT             - invalid input parameter
 * Note:
 *
 */
rtk_api_ret_t rtk_rate_stormControlExtMeterIdx_set(rtk_rate_storm_group_t stormType, rtk_uint32 index)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (stormType >= STORM_GROUP_END)
        return RT_ERR_SFC_UNKNOWN_GROUP;

    if (index > RTK_MAX_METER_ID)
        return RT_ERR_FILTER_METER_ID;

    switch (stormType)
    {
        case STORM_GROUP_UNKNOWN_UNICAST:
            if ((retVal = rtl8367c_setAsicStormFilterExtUnknownUnicastMeter(index))!=RT_ERR_OK)
                return retVal;
            break;
        case STORM_GROUP_UNKNOWN_MULTICAST:
            if ((retVal = rtl8367c_setAsicStormFilterExtUnknownMulticastMeter(index))!=RT_ERR_OK)
                return retVal;
            break;
        case STORM_GROUP_MULTICAST:
            if ((retVal = rtl8367c_setAsicStormFilterExtMulticastMeter(index))!=RT_ERR_OK)
                return retVal;
            break;
        case STORM_GROUP_BROADCAST:
            if ((retVal = rtl8367c_setAsicStormFilterExtBroadcastMeter(index))!=RT_ERR_OK)
                return retVal;
            break;
        default:
            break;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_rate_stormControlExtMeterIdx_get
 * Description:
 *      Get extension storm control meter index
 * Input:
 *      stormType   - storm group type
 *      pIndex      - extension storm control state
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT          - The module is not initial
 *      RT_ERR_INPUT             - invalid input parameter
 * Note:
 *
 */
rtk_api_ret_t rtk_rate_stormControlExtMeterIdx_get(rtk_rate_storm_group_t stormType, rtk_uint32 *pIndex)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (stormType >= STORM_GROUP_END)
        return RT_ERR_SFC_UNKNOWN_GROUP;

    if(NULL == pIndex)
        return RT_ERR_NULL_POINTER;

    switch (stormType)
    {
        case STORM_GROUP_UNKNOWN_UNICAST:
            if ((retVal = rtl8367c_getAsicStormFilterExtUnknownUnicastMeter(pIndex))!=RT_ERR_OK)
                return retVal;
            break;
        case STORM_GROUP_UNKNOWN_MULTICAST:
            if ((retVal = rtl8367c_getAsicStormFilterExtUnknownMulticastMeter(pIndex))!=RT_ERR_OK)
                return retVal;
            break;
        case STORM_GROUP_MULTICAST:
            if ((retVal = rtl8367c_getAsicStormFilterExtMulticastMeter(pIndex))!=RT_ERR_OK)
                return retVal;
            break;
        case STORM_GROUP_BROADCAST:
            if ((retVal = rtl8367c_getAsicStormFilterExtBroadcastMeter(pIndex))!=RT_ERR_OK)
                return retVal;
            break;
        default:
            break;
    }

    return RT_ERR_OK;
}


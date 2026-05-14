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
 * Feature : Here is a list of all functions and variables in Storm module.
 *
 */

#include <rtk_switch.h>
#include <rtk_error.h>
#include <dal/rtl8373/dal_rtl8373_storm.h>
#include <string.h>

#include <dal/rtl8373/rtl8373_asicdrv.h>
#include <dal/rtl8373/dal_rtl8373_rma.h>
#include <dal/rtl8373/dal_rtl8373_igmp.h>


/* Function Name:
 *      dal_rtl8373_rate_stormControlMeterIdx_set
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
rtk_api_ret_t dal_rtl8373_rate_stormControlMeterIdx_set(rtk_port_t port, rtk_rate_storm_group_t stormType, rtk_uint32 index)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);

    if (stormType >= STORM_GROUP_END)
        return RT_ERR_SFC_UNKNOWN_GROUP;

    if (index > RTL8373_METERMAX)
        return RT_ERR_FILTER_METER_ID;

    switch (stormType)
    {
        case STORM_GROUP_UNKNOWN_UNICAST:
            if((retVal = rtl8373_setAsicRegBits(RTL8373_RX_STORM_UNUCAST_METER_ADDR(port), RTL8373_RX_STORM_UNUCAST_METER_RX_STROM_UNUCAST_MIDX_MASK(port), index)) != RT_ERR_OK)
                return retVal;
            break;
        case STORM_GROUP_UNKNOWN_MULTICAST:
            if((retVal = rtl8373_setAsicRegBits(RTL8373_RX_STORM_UNMCAST_METER_ADDR(port), RTL8373_RX_STORM_UNMCAST_METER_RX_STROM_UNMCAST_MIDX_MASK(port), index)) != RT_ERR_OK)
                return retVal;
            break;
        case STORM_GROUP_MULTICAST:
            if((retVal = rtl8373_setAsicRegBits(RTL8373_RX_STORM_MCAST_METER_ADDR(port), RTL8373_RX_STORM_MCAST_METER_RX_STROM_MCAST_MIDX_MASK(port), index)) != RT_ERR_OK)
                return retVal;
            break;
        case STORM_GROUP_BROADCAST:
            if((retVal = rtl8373_setAsicRegBits(RTL8373_RX_STORM_BCAST_METER_ADDR(port), RTL8373_RX_STORM_BCAST_METER_RX_STROM_BCAST_MIDX_MASK(port), index)) != RT_ERR_OK)
                return retVal;
            break;
        default:
            break;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_rate_stormControlMeterIdx_get
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
rtk_api_ret_t dal_rtl8373_rate_stormControlMeterIdx_get(rtk_port_t port, rtk_rate_storm_group_t stormType, rtk_uint32 *pIndex)
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
            if((retVal = rtl8373_getAsicRegBits(RTL8373_RX_STORM_UNUCAST_METER_ADDR(port), RTL8373_RX_STORM_UNUCAST_METER_RX_STROM_UNUCAST_MIDX_MASK(port), pIndex)) != RT_ERR_OK)
                return retVal;
            break;
        case STORM_GROUP_UNKNOWN_MULTICAST:
            if((retVal = rtl8373_getAsicRegBits(RTL8373_RX_STORM_UNMCAST_METER_ADDR(port), RTL8373_RX_STORM_UNMCAST_METER_RX_STROM_UNMCAST_MIDX_MASK(port), pIndex)) != RT_ERR_OK)
                return retVal;
            break;
        case STORM_GROUP_MULTICAST:
            if((retVal = rtl8373_getAsicRegBits(RTL8373_RX_STORM_MCAST_METER_ADDR(port), RTL8373_RX_STORM_MCAST_METER_RX_STROM_MCAST_MIDX_MASK(port), pIndex)) != RT_ERR_OK)
                return retVal;
            break;
        case STORM_GROUP_BROADCAST:
            if((retVal = rtl8373_getAsicRegBits(RTL8373_RX_STORM_BCAST_METER_ADDR(port), RTL8373_RX_STORM_BCAST_METER_RX_STROM_BCAST_MIDX_MASK(port), pIndex)) != RT_ERR_OK)
                return retVal;
            break;
        default:
            break;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_rate_stormControlPortEnable_set
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
rtk_api_ret_t dal_rtl8373_rate_stormControlPortEnable_set(rtk_port_t port, rtk_rate_storm_group_t stormType, rtk_enable_t enable)
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
            if ((retVal = rtl8373_setAsicRegBit(RTL8373_RX_STORM_UNUCAST_CTRL_ADDR(port), RTL8373_RX_STORM_UNUCAST_CTRL_RX_STROM_UNUCAST_EN_OFFSET(port), enable)) != RT_ERR_OK)
                return retVal;
            break;
        case STORM_GROUP_UNKNOWN_MULTICAST:
            if ((retVal = rtl8373_setAsicRegBit(RTL8373_RX_STORM_UNMCAST_CTRL_ADDR(port), RTL8373_RX_STORM_UNMCAST_CTRL_RX_STROM_UNMCAST_EN_OFFSET(port), enable)) != RT_ERR_OK)
                return retVal;
            break;
        case STORM_GROUP_MULTICAST:
            if ((retVal = rtl8373_setAsicRegBit(RTL8373_RX_STORM_MCAST_CTRL_ADDR(port), RTL8373_RX_STORM_MCAST_CTRL_RX_STROM_MCAST_EN_OFFSET(port), enable)) != RT_ERR_OK)
                return retVal;
            break;
        case STORM_GROUP_BROADCAST:
            if ((retVal = rtl8373_setAsicRegBit(RTL8373_RX_STORM_BCAST_CTRL_ADDR(port), RTL8373_RX_STORM_BCAST_CTRL_RX_STROM_BCAST_EN_OFFSET(port), enable)) != RT_ERR_OK)
                return retVal;
            break;
        default:
            break;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_rate_stormControlPortEnable_set
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
rtk_api_ret_t dal_rtl8373_rate_stormControlPortEnable_get(rtk_port_t port, rtk_rate_storm_group_t stormType, rtk_enable_t *pEnable)
{
    rtk_api_ret_t retVal;
    rtk_uint32 regData;

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
            if ((retVal = rtl8373_getAsicRegBit(RTL8373_RX_STORM_UNUCAST_CTRL_ADDR(port), RTL8373_RX_STORM_UNUCAST_CTRL_RX_STROM_UNUCAST_EN_OFFSET(port), &regData)) != RT_ERR_OK)
                return retVal;
            break;
        case STORM_GROUP_UNKNOWN_MULTICAST:
            if ((retVal = rtl8373_getAsicRegBit(RTL8373_RX_STORM_UNMCAST_CTRL_ADDR(port), RTL8373_RX_STORM_UNMCAST_CTRL_RX_STROM_UNMCAST_EN_OFFSET(port), &regData)) != RT_ERR_OK)
                return retVal;
            break;
        case STORM_GROUP_MULTICAST:
            if ((retVal = rtl8373_getAsicRegBit(RTL8373_RX_STORM_MCAST_CTRL_ADDR(port), RTL8373_RX_STORM_MCAST_CTRL_RX_STROM_MCAST_EN_OFFSET(port), &regData)) != RT_ERR_OK)
                return retVal;
            break;
        case STORM_GROUP_BROADCAST:
            if ((retVal = rtl8373_getAsicRegBit(RTL8373_RX_STORM_BCAST_CTRL_ADDR(port), RTL8373_RX_STORM_BCAST_CTRL_RX_STROM_BCAST_EN_OFFSET(port), &regData)) != RT_ERR_OK)
                return retVal;
            break;
        default:
            break;
    }

    *pEnable = (regData == 1) ? ENABLED : DISABLED;
    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_rate_stormControlExtPortmask_set
 * Description:
 *      Set externsion storm control port mask
 * Input:
 *      Portmask  - port mask
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
rtk_api_ret_t dal_rtl8373_rate_stormControlExtPortmask_set(rtk_uint32 portmask)
{
    rtk_api_ret_t retVal;
 
    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if( portmask > RTL8373_PORTMASK)
        return RT_ERR_PORT_MASK;

    if ((retVal = rtl8373_setAsicRegBits(RTL8373_CFG_STORM_EXT_ADDR, RTL8373_CFG_STORM_EXT_STORM_EXT_EN_PORTMASK_MASK, portmask)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_rate_stormControlExtPortmask_get
 * Description:
 *      Set externsion storm control port mask
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
rtk_api_ret_t dal_rtl8373_rate_stormControlExtPortmask_get(rtk_uint32 *pPortmask)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pPortmask)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8373_getAsicRegBits(RTL8373_CFG_STORM_EXT_ADDR, RTL8373_CFG_STORM_EXT_STORM_EXT_EN_PORTMASK_MASK, pPortmask)) != RT_ERR_OK)
         return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_rate_stormControlExtEnable_set
 * Description:
 *      Set externsion storm control state
 * Input:
 *      stormType   - storm group type
 *      enable      - externsion storm control state
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
rtk_api_ret_t dal_rtl8373_rate_stormControlExtEnable_set(rtk_rate_storm_group_t stormType, rtk_enable_t enable)
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
            if ((retVal = rtl8373_setAsicRegBit(RTL8373_CFG_STORM_EXT_ADDR, RTL8373_CFG_STORM_EXT_STORM_UNKNOWN_UCAST_EXT_EN_OFFSET, enable)) != RT_ERR_OK)
                return retVal;
            break;
        case STORM_GROUP_UNKNOWN_MULTICAST:
            if ((retVal = rtl8373_setAsicRegBit(RTL8373_CFG_STORM_EXT_ADDR, RTL8373_CFG_STORM_EXT_STORM_UNKNOWN_MCAST_EXT_EN_OFFSET, enable)) != RT_ERR_OK)
                return retVal;
            break;
        case STORM_GROUP_MULTICAST:
            if ((retVal = rtl8373_setAsicRegBit(RTL8373_CFG_STORM_EXT_ADDR, RTL8373_CFG_STORM_EXT_STORM_MCAST_EXT_EN_OFFSET, enable)) != RT_ERR_OK)
                return retVal;
            break;
        case STORM_GROUP_BROADCAST:
            if ((retVal = rtl8373_setAsicRegBit(RTL8373_CFG_STORM_EXT_ADDR, RTL8373_CFG_STORM_EXT_STORM_BCAST_EXT_EN_OFFSET, enable)) != RT_ERR_OK)
                return retVal;
            break;
        default:
            break;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_rate_stormControlExtEnable_get
 * Description:
 *      Get externsion storm control state
 * Input:
 *      stormType   - storm group type
 * Output:
 *      pEnable     - externsion storm control state
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT          - The module is not initial
 *      RT_ERR_INPUT             - invalid input parameter
 * Note:
 *
 */
rtk_api_ret_t dal_rtl8373_rate_stormControlExtEnable_get(rtk_rate_storm_group_t stormType, rtk_enable_t *pEnable)
{
    rtk_api_ret_t retVal;
    //rtk_uint32 regData;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (stormType >= STORM_GROUP_END)
        return RT_ERR_SFC_UNKNOWN_GROUP;

    if (NULL == pEnable)
        return RT_ERR_NULL_POINTER;

    switch (stormType)
    {
        case STORM_GROUP_UNKNOWN_UNICAST:
            if ((retVal = rtl8373_getAsicRegBit(RTL8373_CFG_STORM_EXT_ADDR, RTL8373_CFG_STORM_EXT_STORM_UNKNOWN_UCAST_EXT_EN_OFFSET, pEnable)) != RT_ERR_OK)
                return retVal;
            break;
        case STORM_GROUP_UNKNOWN_MULTICAST:
            if ((retVal = rtl8373_getAsicRegBit(RTL8373_CFG_STORM_EXT_ADDR, RTL8373_CFG_STORM_EXT_STORM_UNKNOWN_MCAST_EXT_EN_OFFSET, pEnable)) != RT_ERR_OK)
                return retVal;
            break;
        case STORM_GROUP_MULTICAST:
            if ((retVal = rtl8373_getAsicRegBit(RTL8373_CFG_STORM_EXT_ADDR, RTL8373_CFG_STORM_EXT_STORM_MCAST_EXT_EN_OFFSET, pEnable)) != RT_ERR_OK)
                return retVal;
            break;
        case STORM_GROUP_BROADCAST:
            if ((retVal = rtl8373_getAsicRegBit(RTL8373_CFG_STORM_EXT_ADDR, RTL8373_CFG_STORM_EXT_STORM_BCAST_EXT_EN_OFFSET, pEnable)) != RT_ERR_OK)
                return retVal;
            break;
        default:
            break;
    }

    //*pEnable = (regData == 1) ? ENABLED : DISABLED;
    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_rate_stormControlExtMeterIdx_set
 * Description:
 *      Set externsion storm control meter index
 * Input:
 *      stormType   - storm group type
 *      index       - externsion storm control state
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
rtk_api_ret_t dal_rtl8373_rate_stormControlExtMeterIdx_set(rtk_rate_storm_group_t stormType, rtk_uint32 index)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (stormType >= STORM_GROUP_END)
        return RT_ERR_SFC_UNKNOWN_GROUP;

    if (index > RTL8373_METERMAX)
        return RT_ERR_FILTER_METER_ID;

    switch (stormType)
    {
        case STORM_GROUP_UNKNOWN_UNICAST:
            if ((retVal = rtl8373_setAsicRegBits(RTL8373_STORM_EXT_MTRIDX_CFG_ADDR, RTL8373_STORM_EXT_MTRIDX_CFG_STORM_UNKNOWN_UCAST_EXT_METERID_MASK, index))!=RT_ERR_OK)
                return retVal;
            break;
        case STORM_GROUP_UNKNOWN_MULTICAST:
            if ((retVal = rtl8373_setAsicRegBits(RTL8373_STORM_EXT_MTRIDX_CFG_ADDR, RTL8373_STORM_EXT_MTRIDX_CFG_STORM_UNKNOWN_MCAST_EXT_METERID_MASK, index))!=RT_ERR_OK)
                return retVal;
            break;
        case STORM_GROUP_MULTICAST:
            if ((retVal = rtl8373_setAsicRegBits(RTL8373_STORM_EXT_MTRIDX_CFG_ADDR, RTL8373_STORM_EXT_MTRIDX_CFG_STORM_MCAST_EXT_METERID_MASK, index))!=RT_ERR_OK)
                return retVal;
            break;
        case STORM_GROUP_BROADCAST:
            if ((retVal = rtl8373_setAsicRegBits(RTL8373_STORM_EXT_MTRIDX_CFG_ADDR, RTL8373_STORM_EXT_MTRIDX_CFG_STORM_BCAST_EXT_METERID_MASK, index))!=RT_ERR_OK)
                return retVal;
            break;
        default:
            break;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_rate_stormControlExtMeterIdx_get
 * Description:
 *      Get externsion storm control meter index
 * Input:
 *      stormType   - storm group type      
 * Output:
 *      pIndex      - externsion storm control state
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT          - The module is not initial
 *      RT_ERR_INPUT             - invalid input parameter
 * Note:
 *
 */
rtk_api_ret_t dal_rtl8373_rate_stormControlExtMeterIdx_get(rtk_rate_storm_group_t stormType, rtk_uint32 *pIndex)
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
            if ((retVal = rtl8373_getAsicRegBits(RTL8373_STORM_EXT_MTRIDX_CFG_ADDR, RTL8373_STORM_EXT_MTRIDX_CFG_STORM_UNKNOWN_UCAST_EXT_METERID_MASK, pIndex))!=RT_ERR_OK)
                return retVal;
            break;
        case STORM_GROUP_UNKNOWN_MULTICAST:
            if ((retVal = rtl8373_getAsicRegBits(RTL8373_STORM_EXT_MTRIDX_CFG_ADDR, RTL8373_STORM_EXT_MTRIDX_CFG_STORM_UNKNOWN_MCAST_EXT_METERID_MASK, pIndex))!=RT_ERR_OK)
                return retVal;
            break;
        case STORM_GROUP_MULTICAST:
            if ((retVal = rtl8373_getAsicRegBits(RTL8373_STORM_EXT_MTRIDX_CFG_ADDR, RTL8373_STORM_EXT_MTRIDX_CFG_STORM_MCAST_EXT_METERID_MASK, pIndex))!=RT_ERR_OK)
                return retVal;
            break;
        case STORM_GROUP_BROADCAST:
            if ((retVal = rtl8373_getAsicRegBits(RTL8373_STORM_EXT_MTRIDX_CFG_ADDR, RTL8373_STORM_EXT_MTRIDX_CFG_STORM_BCAST_EXT_METERID_MASK, pIndex))!=RT_ERR_OK)
                return retVal;
            break;
        default:
            break;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8367c_storm_bypass_set
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
 *      This API can set per-port bypass stomr filter control frame type including RMA and igmp.
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
rtk_api_ret_t dal_rtl8373_storm_bypass_set(rtk_storm_bypass_t type, rtk_enable_t enable)
{
    rtk_api_ret_t retVal;
    rtk_rmaParam_t rmacfg;
    rtk_uint32 tmp;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (type >= BYPASS_END)
        return RT_ERR_INPUT;

    if (enable >= RTK_ENABLE_END)
        return RT_ERR_INPUT;

    if (type >= 0 && type <= BYPASS_UNDEF_GARP_2F)
    {
        if ((retVal = dal_rtl8373_asicRma_get(type, &rmacfg)) != RT_ERR_OK)
            return retVal;

        rmacfg.discard_storm_filter = enable;

        if ((retVal = dal_rtl8373_asicRma_set(type, &rmacfg)) != RT_ERR_OK)
            return retVal;
    }
    else if(type == BYPASS_IGMP)
    {
        if ((retVal = dal_rtl8373_setAsicIGMPBypassStormCTRL(enable)) != RT_ERR_OK)
            return retVal;
    }
    else if (type == BYPASS_CDP)
    {
        if ((retVal = dal_rtl8373_asicRmaCdp_get(&rmacfg)) != RT_ERR_OK)
            return retVal;

        rmacfg.discard_storm_filter = enable;

        if ((retVal = dal_rtl8373_asicRmaCdp_set(&rmacfg)) != RT_ERR_OK)
            return retVal;
    }
    else if (type  == BYPASS_CSSTP)
    {
        if ((retVal = dal_rtl8373_asicRmaCsstp_get(&rmacfg)) != RT_ERR_OK)
            return retVal;

        rmacfg.discard_storm_filter = enable;

        if ((retVal = dal_rtl8373_asicRmaCsstp_set(&rmacfg)) != RT_ERR_OK)
            return retVal;
    }
    else if (type  == BYPASS_LLDP)
    {
        if ((retVal = dal_rtl8373_asicRmaLldp_get(&tmp, &rmacfg)) != RT_ERR_OK)
            return retVal;

        rmacfg.discard_storm_filter = enable;

        if ((retVal = dal_rtl8373_asicRmaLldp_set(tmp, &rmacfg)) != RT_ERR_OK)
            return retVal;
    }
    else
        return RT_ERR_INPUT;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8367c_storm_bypass_get
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
 *      This API can get per-port bypass stomr filter control frame type including RMA and igmp.
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
rtk_api_ret_t dal_rtl8373_storm_bypass_get(rtk_storm_bypass_t type, rtk_enable_t *pEnable)
{
    rtk_api_ret_t retVal;
    rtk_rmaParam_t rmacfg;
    rtk_uint32 tmp;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (type >= BYPASS_END)
        return RT_ERR_INPUT;

    if(NULL == pEnable)
        return RT_ERR_NULL_POINTER;

    if (type >= 0 && type <= BYPASS_UNDEF_GARP_2F)
    {
        if ((retVal = dal_rtl8373_asicRma_get(type, &rmacfg)) != RT_ERR_OK)
            return retVal;

        *pEnable = rmacfg.discard_storm_filter;
    }
    else if(type == BYPASS_IGMP)
    {
        if ((retVal = dal_rtl8373_getAsicIGMPBypassStormCTRL(pEnable)) != RT_ERR_OK)
            return retVal;
    }
    else if (type == BYPASS_CDP)
    {
        if ((retVal = dal_rtl8373_asicRmaCdp_get(&rmacfg)) != RT_ERR_OK)
            return retVal;

        *pEnable = rmacfg.discard_storm_filter;
    }
    else if (type == BYPASS_CSSTP)
    {
        if ((retVal = dal_rtl8373_asicRmaCsstp_get(&rmacfg)) != RT_ERR_OK)
            return retVal;

        *pEnable = rmacfg.discard_storm_filter;
    }
    else if (type == BYPASS_LLDP)
    {
        if ((retVal = dal_rtl8373_asicRmaLldp_get(&tmp,&rmacfg)) != RT_ERR_OK)
            return retVal;

        *pEnable = rmacfg.discard_storm_filter;
    }
    else
        return RT_ERR_INPUT;

    return RT_ERR_OK;
}





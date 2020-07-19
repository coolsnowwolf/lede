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
 * Purpose : RTL8367C switch high-level API for RTL8367C
 * Feature : IGMP related functions
 *
 */
#include <rtl8367c_asicdrv_igmp.h>
/* Function Name:
 *      rtl8367c_setAsicIgmp
 * Description:
 *      Set IGMP/MLD state
 * Input:
 *      enabled     - 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicIgmp(rtk_uint32 enabled)
{
    ret_t retVal;

    /* Enable/Disable H/W IGMP/MLD */
    retVal = rtl8367c_setAsicRegBit(RTL8367C_REG_IGMP_MLD_CFG0, RTL8367C_IGMP_MLD_EN_OFFSET, enabled);

    return retVal;
}
/* Function Name:
 *      rtl8367c_getAsicIgmp
 * Description:
 *      Get IGMP/MLD state
 * Input:
 *      enabled     - 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicIgmp(rtk_uint32 *ptr_enabled)
{
    ret_t retVal;

    retVal = rtl8367c_getAsicRegBit(RTL8367C_REG_IGMP_MLD_CFG0, RTL8367C_IGMP_MLD_EN_OFFSET, ptr_enabled);
    return retVal;
}
/* Function Name:
 *      rtl8367c_setAsicIpMulticastVlanLeaky
 * Description:
 *      Set IP multicast VLAN Leaky function
 * Input:
 *      port        - Physical port number (0~7)
 *      enabled     - 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      When enabling this function,
 *      if the lookup result(forwarding portmap) of IP Multicast packet is over VLAN boundary,
 *      the packet can be forwarded across VLAN
 */
ret_t rtl8367c_setAsicIpMulticastVlanLeaky(rtk_uint32 port, rtk_uint32 enabled)
{
    ret_t  retVal;

    if(port > RTL8367C_PORTIDMAX)
        return RT_ERR_PORT_ID;

    retVal = rtl8367c_setAsicRegBit(RTL8367C_REG_IPMCAST_VLAN_LEAKY, port, enabled);

    return retVal;
}
/* Function Name:
 *      rtl8367c_getAsicIpMulticastVlanLeaky
 * Description:
 *      Get IP multicast VLAN Leaky function
 * Input:
 *      port        - Physical port number (0~7)
 *      enabled     - 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicIpMulticastVlanLeaky(rtk_uint32 port, rtk_uint32 *ptr_enabled)
{
    ret_t  retVal;

    if(port > RTL8367C_PORTIDMAX)
        return RT_ERR_PORT_ID;

    retVal = rtl8367c_getAsicRegBit(RTL8367C_REG_IPMCAST_VLAN_LEAKY, port, ptr_enabled);

    return retVal;
}

/* Function Name:
 *      rtl8367c_setAsicIGMPTableFullOP
 * Description:
 *      Set Table Full operation
 * Input:
 *      operation   - The operation should be taken when the IGMP table is full.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_OUT_OF_RANGE - input parameter is out of range
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicIGMPTableFullOP(rtk_uint32 operation)
{
    ret_t  retVal;

    if(operation >= TABLE_FULL_OP_END)
        return RT_ERR_OUT_OF_RANGE;

    /* Table full Operation */
    retVal = rtl8367c_setAsicRegBits(RTL8367C_REG_IGMP_MLD_CFG1, RTL8367C_TABLE_FULL_OP_MASK, operation);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367c_getAsicIGMPTableFullOP
 * Description:
 *      Get Table Full operation
 * Input:
 *      None
 * Output:
 *      poperation  - The operation should be taken when the IGMP table is full.
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicIGMPTableFullOP(rtk_uint32 *poperation)
{
    ret_t   retVal;
    rtk_uint32  value;

    /* Table full Operation */
    retVal = rtl8367c_getAsicRegBits(RTL8367C_REG_IGMP_MLD_CFG1, RTL8367C_TABLE_FULL_OP_MASK, &value);
    if(retVal != RT_ERR_OK)
        return retVal;

    *poperation = value;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367c_setAsicIGMPCRCErrOP
 * Description:
 *      Set the operation when ASIC receive a Checksum error packet
 * Input:
 *      operation   -The operation when ASIC receive a Checksum error packet
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_OUT_OF_RANGE - input parameter is out of range
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicIGMPCRCErrOP(rtk_uint32 operation)
{
    ret_t  retVal;

    if(operation >= CRC_ERR_OP_END)
        return RT_ERR_OUT_OF_RANGE;

    /* CRC Error Operation */
    retVal = rtl8367c_setAsicRegBits(RTL8367C_REG_IGMP_MLD_CFG0, RTL8367C_CKS_ERR_OP_MASK, operation);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367c_getAsicIGMPCRCErrOP
 * Description:
 *      Get the operation when ASIC receive a Checksum error packet
 * Input:
 *      None
 * Output:
 *      poperation  - The operation of Checksum error packet
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicIGMPCRCErrOP(rtk_uint32 *poperation)
{
    ret_t   retVal;
    rtk_uint32  value;

    /* CRC Error Operation */
    retVal = rtl8367c_getAsicRegBits(RTL8367C_REG_IGMP_MLD_CFG0, RTL8367C_CKS_ERR_OP_MASK, &value);
    if(retVal != RT_ERR_OK)
        return retVal;

    *poperation = value;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367c_setAsicIGMPFastLeaveEn
 * Description:
 *      Enable/Disable Fast Leave
 * Input:
 *      enabled - 1:enable Fast Leave; 0:disable Fast Leave
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicIGMPFastLeaveEn(rtk_uint32 enabled)
{
    ret_t  retVal;

    /* Fast Leave */
    retVal = rtl8367c_setAsicRegBits(RTL8367C_REG_IGMP_MLD_CFG0, RTL8367C_FAST_LEAVE_EN_MASK, (enabled >= 1) ? 1 : 0);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367c_getAsicIGMPFastLeaveEn
 * Description:
 *      Get Fast Leave state
 * Input:
 *      None
 * Output:
 *      penabled        - 1:enable Fast Leave; 0:disable Fast Leave
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicIGMPFastLeaveEn(rtk_uint32 *penabled)
{
    ret_t   retVal;
    rtk_uint32  value;

    /* Fast Leave */
    retVal = rtl8367c_getAsicRegBits(RTL8367C_REG_IGMP_MLD_CFG0, RTL8367C_FAST_LEAVE_EN_MASK, &value);
    if(retVal != RT_ERR_OK)
        return retVal;

    *penabled = value;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367c_setAsicIGMPLeaveTimer
 * Description:
 *      Set the Leave timer of IGMP/MLD
 * Input:
 *      leave_timer     - Leave timer
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_OUT_OF_RANGE - input parameter is out of range
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicIGMPLeaveTimer(rtk_uint32 leave_timer)
{
    ret_t  retVal;

    if(leave_timer > RTL8367C_MAX_LEAVE_TIMER)
        return RT_ERR_OUT_OF_RANGE;

    /* Leave timer */
    retVal = rtl8367c_setAsicRegBits(RTL8367C_REG_IGMP_MLD_CFG0, RTL8367C_LEAVE_TIMER_MASK, leave_timer);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367c_getAsicIGMPLeaveTimer
 * Description:
 *      Get the Leave timer of IGMP/MLD
 * Input:
 *      None
 * Output:
 *      pleave_timer    - Leave timer
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicIGMPLeaveTimer(rtk_uint32 *pleave_timer)
{
    ret_t   retVal;
    rtk_uint32  value;

    /* Leave timer */
    retVal = rtl8367c_getAsicRegBits(RTL8367C_REG_IGMP_MLD_CFG0, RTL8367C_LEAVE_TIMER_MASK, &value);
    if(retVal != RT_ERR_OK)
        return retVal;

    *pleave_timer = value;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367c_setAsicIGMPQueryInterval
 * Description:
 *      Set Query Interval of IGMP/MLD
 * Input:
 *      interval    - Query Interval
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_OUT_OF_RANGE - input parameter is out of range
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicIGMPQueryInterval(rtk_uint32 interval)
{
    ret_t  retVal;

    if(interval > RTL8367C_MAX_QUERY_INT)
        return RT_ERR_OUT_OF_RANGE;

    /* Query Interval */
    retVal = rtl8367c_setAsicReg(RTL8367C_REG_IGMP_MLD_CFG2, interval);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367c_getAsicIGMPQueryInterval
 * Description:
 *      Get Query Interval of IGMP/MLD
 * Input:
 *      None
 * Output:
 *      pinterval       - Query Interval
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicIGMPQueryInterval(rtk_uint32 *pinterval)
{
    ret_t   retVal;
    rtk_uint32  value;

    /* Query Interval */
    retVal = rtl8367c_getAsicReg(RTL8367C_REG_IGMP_MLD_CFG2, &value);
    if(retVal != RT_ERR_OK)
        return retVal;

    *pinterval = value;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367c_setAsicIGMPRobVar
 * Description:
 *      Set Robustness Variable of IGMP/MLD
 * Input:
 *      rob_var     - Robustness Variable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_OUT_OF_RANGE - input parameter is out of range
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicIGMPRobVar(rtk_uint32 rob_var)
{
    ret_t  retVal;

    if(rob_var > RTL8367C_MAX_ROB_VAR)
        return RT_ERR_OUT_OF_RANGE;

    /* Bourstness variable */
    retVal = rtl8367c_setAsicRegBits(RTL8367C_REG_IGMP_MLD_CFG0, RTL8367C_ROBURSTNESS_VAR_MASK, rob_var);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367c_getAsicIGMPRobVar
 * Description:
 *      Get Robustness Variable of IGMP/MLD
 * Input:
 *      none
 * Output:
 *      prob_var     - Robustness Variable
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicIGMPRobVar(rtk_uint32 *prob_var)
{
    ret_t   retVal;
    rtk_uint32  value;

    /* Bourstness variable */
    retVal = rtl8367c_getAsicRegBits(RTL8367C_REG_IGMP_MLD_CFG0, RTL8367C_ROBURSTNESS_VAR_MASK, &value);
    if(retVal != RT_ERR_OK)
        return retVal;

    *prob_var = value;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367c_setAsicIGMPStaticRouterPort
 * Description:
 *      Set IGMP static router port mask
 * Input:
 *      pmsk    - Static portmask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_MASK    - Invalid port mask
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicIGMPStaticRouterPort(rtk_uint32 pmsk)
{
    if(pmsk > RTL8367C_PORTMASK)
        return RT_ERR_PORT_MASK;

    return rtl8367c_setAsicRegBits(RTL8367C_REG_IGMP_STATIC_ROUTER_PORT, RTL8367C_IGMP_STATIC_ROUTER_PORT_MASK, pmsk);
}

/* Function Name:
 *      rtl8367c_getAsicIGMPStaticRouterPort
 * Description:
 *      Get IGMP static router port mask
 * Input:
 *      pmsk    - Static portmask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicIGMPStaticRouterPort(rtk_uint32 *pmsk)
{
    return rtl8367c_getAsicRegBits(RTL8367C_REG_IGMP_STATIC_ROUTER_PORT, RTL8367C_IGMP_STATIC_ROUTER_PORT_MASK, pmsk);
}

/* Function Name:
 *      rtl8367c_setAsicIGMPAllowDynamicRouterPort
 * Description:
 *      Set IGMP dynamic router port allow mask
 * Input:
 *      pmsk    - Allow dynamic router port mask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_MASK    - Invalid port mask
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicIGMPAllowDynamicRouterPort(rtk_uint32 pmsk)
{
    return rtl8367c_setAsicReg(RTL8367C_REG_IGMP_MLD_CFG4, pmsk);
}

/* Function Name:
 *      rtl8367c_getAsicIGMPAllowDynamicRouterPort
 * Description:
 *      Get IGMP dynamic router port allow mask
 * Input:
 *      None.
 * Output:
 *      pPmsk   - Allow dynamic router port mask
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_MASK    - Invalid port mask
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicIGMPAllowDynamicRouterPort(rtk_uint32 *pPmsk)
{
    return rtl8367c_getAsicReg(RTL8367C_REG_IGMP_MLD_CFG4, pPmsk);
}

/* Function Name:
 *      rtl8367c_getAsicIGMPdynamicRouterPort1
 * Description:
 *      Get 1st dynamic router port and timer
 * Input:
 *      port    - Physical port number (0~7)
 *      timer   - router port timer
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicIGMPdynamicRouterPort1(rtk_uint32 *port, rtk_uint32 *timer)
{
    ret_t   retVal;

    retVal = rtl8367c_getAsicRegBits(RTL8367C_REG_IGMP_DYNAMIC_ROUTER_PORT, RTL8367C_D_ROUTER_PORT_1_MASK, port);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8367c_getAsicRegBits(RTL8367C_REG_IGMP_DYNAMIC_ROUTER_PORT, RTL8367C_D_ROUTER_PORT_TMR_1_MASK, timer);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367c_getAsicIGMPdynamicRouterPort2
 * Description:
 *      Get 2nd dynamic router port and timer
 * Input:
 *      port    - Physical port number (0~7)
 *      timer   - router port timer
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicIGMPdynamicRouterPort2(rtk_uint32 *port, rtk_uint32 *timer)
{
    ret_t   retVal;

    retVal = rtl8367c_getAsicRegBits(RTL8367C_REG_IGMP_DYNAMIC_ROUTER_PORT, RTL8367C_D_ROUTER_PORT_2_MASK, port);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8367c_getAsicRegBits(RTL8367C_REG_IGMP_DYNAMIC_ROUTER_PORT, RTL8367C_D_ROUTER_PORT_TMR_2_MASK, timer);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367c_setAsicIGMPSuppression
 * Description:
 *      Set the suppression function
 * Input:
 *      report_supp_enabled     - Report suppression, 1:Enable, 0:disable
 *      leave_supp_enabled      - Leave suppression, 1:Enable, 0:disable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicIGMPSuppression(rtk_uint32 report_supp_enabled, rtk_uint32 leave_supp_enabled)
{
    ret_t   retVal;

    retVal = rtl8367c_setAsicRegBits(RTL8367C_REG_IGMP_MLD_CFG0, RTL8367C_REPORT_SUPPRESSION_MASK, report_supp_enabled);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8367c_setAsicRegBits(RTL8367C_REG_IGMP_MLD_CFG0, RTL8367C_LEAVE_SUPPRESSION_MASK, leave_supp_enabled);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367c_getAsicIGMPSuppression
 * Description:
 *      Get the suppression function
 * Input:
 *      report_supp_enabled     - Report suppression, 1:Enable, 0:disable
 *      leave_supp_enabled      - Leave suppression, 1:Enable, 0:disable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicIGMPSuppression(rtk_uint32 *report_supp_enabled, rtk_uint32 *leave_supp_enabled)
{
    ret_t   retVal;

    retVal = rtl8367c_getAsicRegBits(RTL8367C_REG_IGMP_MLD_CFG0, RTL8367C_REPORT_SUPPRESSION_MASK, report_supp_enabled);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8367c_getAsicRegBits(RTL8367C_REG_IGMP_MLD_CFG0, RTL8367C_LEAVE_SUPPRESSION_MASK, leave_supp_enabled);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367c_setAsicIGMPQueryRX
 * Description:
 *      Set port-based Query packet RX allowance
 * Input:
 *      port            - port number
 *      allow_query     - allowance of Query packet RX, 1:Allow, 0:Drop
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_PORT_ID  - Error PORT ID
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicIGMPQueryRX(rtk_uint32 port, rtk_uint32 allow_query)
{
    ret_t   retVal;

    if(port > RTL8367C_PORTIDMAX)
        return RT_ERR_PORT_ID;

    /* Allow Query */
    if (port < 8)
    {
        retVal = rtl8367c_setAsicRegBits(RTL8367C_REG_IGMP_PORT0_CONTROL + port, RTL8367C_IGMP_PORT0_CONTROL_ALLOW_QUERY_MASK, allow_query);
        if(retVal != RT_ERR_OK)
            return retVal;
    }
    else
    {
        retVal = rtl8367c_setAsicRegBits(RTL8367C_REG_IGMP_PORT8_CONTROL + port - 8, RTL8367C_IGMP_PORT0_CONTROL_ALLOW_QUERY_MASK, allow_query);
        if(retVal != RT_ERR_OK)
            return retVal;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367c_getAsicIGMPQueryRX
 * Description:
 *      Get port-based Query packet RX allowance
 * Input:
 *      port            - port number
 * Output:
 *      allow_query     - allowance of Query packet RX, 1:Allow, 0:Drop
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_PORT_ID  - Error PORT ID
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicIGMPQueryRX(rtk_uint32 port, rtk_uint32 *allow_query)
{
    ret_t   retVal;
    rtk_uint32  value;

    if(port > RTL8367C_PORTIDMAX)
        return RT_ERR_PORT_ID;

    /* Allow Query */
    if (port < 8)
    {
        retVal = rtl8367c_getAsicRegBits(RTL8367C_REG_IGMP_PORT0_CONTROL + port, RTL8367C_IGMP_PORT0_CONTROL_ALLOW_QUERY_MASK, &value);
        if(retVal != RT_ERR_OK)
            return retVal;
    }
    else
    {
        retVal = rtl8367c_getAsicRegBits(RTL8367C_REG_IGMP_PORT8_CONTROL + port - 8, RTL8367C_IGMP_PORT0_CONTROL_ALLOW_QUERY_MASK, &value);
        if(retVal != RT_ERR_OK)
            return retVal;
    }
    *allow_query = value;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367c_setAsicIGMPReportRX
 * Description:
 *      Set port-based Report packet RX allowance
 * Input:
 *      port            - port number
 *      allow_report    - allowance of Report packet RX, 1:Allow, 0:Drop
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_PORT_ID  - Error PORT ID
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicIGMPReportRX(rtk_uint32 port, rtk_uint32 allow_report)
{
    ret_t   retVal;

    if(port > RTL8367C_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(port < 8)
    {
    /* Allow Report */
        retVal = rtl8367c_setAsicRegBits(RTL8367C_REG_IGMP_PORT0_CONTROL + port, RTL8367C_IGMP_PORT0_CONTROL_ALLOW_REPORT_MASK, allow_report);
        if(retVal != RT_ERR_OK)
            return retVal;
    }
    else
    {
        retVal = rtl8367c_setAsicRegBits(RTL8367C_REG_IGMP_PORT8_CONTROL + port - 8, RTL8367C_IGMP_PORT0_CONTROL_ALLOW_REPORT_MASK, allow_report);
        if(retVal != RT_ERR_OK)
            return retVal;
    }
    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367c_getAsicIGMPReportRX
 * Description:
 *      Get port-based Report packet RX allowance
 * Input:
 *      port            - port number
 * Output:
 *      allow_report    - allowance of Report packet RX, 1:Allow, 0:Drop
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_PORT_ID  - Error PORT ID
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicIGMPReportRX(rtk_uint32 port, rtk_uint32 *allow_report)
{
    ret_t   retVal;
    rtk_uint32  value;

    if(port > RTL8367C_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(port < 8)
    {
        /* Allow Report */
        retVal = rtl8367c_getAsicRegBits(RTL8367C_REG_IGMP_PORT0_CONTROL + port, RTL8367C_IGMP_PORT0_CONTROL_ALLOW_REPORT_MASK, &value);
        if(retVal != RT_ERR_OK)
            return retVal;
    }
    else
    {
        retVal = rtl8367c_getAsicRegBits(RTL8367C_REG_IGMP_PORT8_CONTROL + port - 8, RTL8367C_IGMP_PORT0_CONTROL_ALLOW_REPORT_MASK, &value);
        if(retVal != RT_ERR_OK)
            return retVal;
    }
    *allow_report = value;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367c_setAsicIGMPLeaveRX
 * Description:
 *      Set port-based Leave packet RX allowance
 * Input:
 *      port            - port number
 *      allow_leave     - allowance of Leave packet RX, 1:Allow, 0:Drop
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_PORT_ID  - Error PORT ID
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicIGMPLeaveRX(rtk_uint32 port, rtk_uint32 allow_leave)
{
    ret_t   retVal;

    if(port > RTL8367C_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(port < 8)
    {
        /* Allow Leave */
        retVal = rtl8367c_setAsicRegBits(RTL8367C_REG_IGMP_PORT0_CONTROL + port, RTL8367C_IGMP_PORT0_CONTROL_ALLOW_LEAVE_MASK, allow_leave);
        if(retVal != RT_ERR_OK)
            return retVal;
    }
    else
    {
        retVal = rtl8367c_setAsicRegBits(RTL8367C_REG_IGMP_PORT8_CONTROL + port - 8, RTL8367C_IGMP_PORT0_CONTROL_ALLOW_LEAVE_MASK, allow_leave);
        if(retVal != RT_ERR_OK)
            return retVal;
    }
    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367c_getAsicIGMPLeaveRX
 * Description:
 *      Get port-based Leave packet RX allowance
 * Input:
 *      port            - port number
 * Output:
 *      allow_leave     - allowance of Leave packet RX, 1:Allow, 0:Drop
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_PORT_ID  - Error PORT ID
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicIGMPLeaveRX(rtk_uint32 port, rtk_uint32 *allow_leave)
{
    ret_t   retVal;
    rtk_uint32  value;

    if(port > RTL8367C_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(port < 8)
    {
    /* Allow Leave */
        retVal = rtl8367c_getAsicRegBits(RTL8367C_REG_IGMP_PORT0_CONTROL + port, RTL8367C_IGMP_PORT0_CONTROL_ALLOW_LEAVE_MASK, &value);
        if(retVal != RT_ERR_OK)
            return retVal;
    }
    else
    {
        retVal = rtl8367c_getAsicRegBits(RTL8367C_REG_IGMP_PORT8_CONTROL + port - 8, RTL8367C_IGMP_PORT0_CONTROL_ALLOW_LEAVE_MASK, &value);
        if(retVal != RT_ERR_OK)
            return retVal;
    }

    *allow_leave = value;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367c_setAsicIGMPMRPRX
 * Description:
 *      Set port-based Multicast Routing Protocol packet RX allowance
 * Input:
 *      port            - port number
 *      allow_mrp       - allowance of Multicast Routing Protocol packet RX, 1:Allow, 0:Drop
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_PORT_ID  - Error PORT ID
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicIGMPMRPRX(rtk_uint32 port, rtk_uint32 allow_mrp)
{
    ret_t   retVal;

    if(port > RTL8367C_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(port < 8)
    {
    /* Allow Multicast Routing Protocol */
        retVal = rtl8367c_setAsicRegBits(RTL8367C_REG_IGMP_PORT0_CONTROL + port, RTL8367C_IGMP_PORT0_CONTROL_ALLOW_MRP_MASK, allow_mrp);
        if(retVal != RT_ERR_OK)
            return retVal;
    }
    else
    {
        retVal = rtl8367c_setAsicRegBits(RTL8367C_REG_IGMP_PORT8_CONTROL + port - 8, RTL8367C_IGMP_PORT0_CONTROL_ALLOW_MRP_MASK, allow_mrp);
        if(retVal != RT_ERR_OK)
            return retVal;
    }
    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367c_getAsicIGMPMRPRX
 * Description:
 *      Get port-based Multicast Routing Protocol packet RX allowance
 * Input:
 *      port            - port number
 * Output:
 *      allow_mrp       - allowance of Multicast Routing Protocol packet RX, 1:Allow, 0:Drop
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_PORT_ID  - Error PORT ID
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicIGMPMRPRX(rtk_uint32 port, rtk_uint32 *allow_mrp)
{
    ret_t   retVal;
    rtk_uint32  value;

    if(port > RTL8367C_PORTIDMAX)
        return RT_ERR_PORT_ID;

    /* Allow Multicast Routing Protocol */
    if(port < 8)
    {
        retVal = rtl8367c_getAsicRegBits(RTL8367C_REG_IGMP_PORT0_CONTROL + port, RTL8367C_IGMP_PORT0_CONTROL_ALLOW_MRP_MASK, &value);
        if(retVal != RT_ERR_OK)
            return retVal;
    }
    else
    {
        retVal = rtl8367c_getAsicRegBits(RTL8367C_REG_IGMP_PORT8_CONTROL + port - 8, RTL8367C_IGMP_PORT0_CONTROL_ALLOW_MRP_MASK, &value);
        if(retVal != RT_ERR_OK)
            return retVal;
    }
    *allow_mrp = value;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367c_setAsicIGMPMcDataRX
 * Description:
 *      Set port-based Multicast data packet RX allowance
 * Input:
 *      port            - port number
 *      allow_mcdata    - allowance of Multicast data packet RX, 1:Allow, 0:Drop
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_PORT_ID  - Error PORT ID
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicIGMPMcDataRX(rtk_uint32 port, rtk_uint32 allow_mcdata)
{
    ret_t   retVal;

    if(port > RTL8367C_PORTIDMAX)
        return RT_ERR_PORT_ID;

    /* Allow Multicast Data */
    if(port < 8)
    {
        retVal = rtl8367c_setAsicRegBits(RTL8367C_REG_IGMP_PORT0_CONTROL + port, RTL8367C_IGMP_PORT0_CONTROL_ALLOW_MC_DATA_MASK, allow_mcdata);
        if(retVal != RT_ERR_OK)
            return retVal;
    }
    else
    {
        retVal = rtl8367c_setAsicRegBits(RTL8367C_REG_IGMP_PORT8_CONTROL + port - 8, RTL8367C_IGMP_PORT0_CONTROL_ALLOW_MC_DATA_MASK, allow_mcdata);
        if(retVal != RT_ERR_OK)
            return retVal;
    }
    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367c_getAsicIGMPMcDataRX
 * Description:
 *      Get port-based Multicast data packet RX allowance
 * Input:
 *      port            - port number
 * Output:
 *      allow_mcdata    - allowance of Multicast data packet RX, 1:Allow, 0:Drop
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_PORT_ID  - Error PORT ID
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicIGMPMcDataRX(rtk_uint32 port, rtk_uint32 *allow_mcdata)
{
    ret_t   retVal;
    rtk_uint32  value;

    if(port > RTL8367C_PORTIDMAX)
        return RT_ERR_PORT_ID;

    /* Allow Multicast data */
    if(port < 8)
    {
        retVal = rtl8367c_getAsicRegBits(RTL8367C_REG_IGMP_PORT0_CONTROL + port, RTL8367C_IGMP_PORT0_CONTROL_ALLOW_MC_DATA_MASK, &value);
        if(retVal != RT_ERR_OK)
            return retVal;
    }
    else
    {
        retVal = rtl8367c_getAsicRegBits(RTL8367C_REG_IGMP_PORT8_CONTROL + port - 8, RTL8367C_IGMP_PORT0_CONTROL_ALLOW_MC_DATA_MASK, &value);
        if(retVal != RT_ERR_OK)
            return retVal;
    }

    *allow_mcdata = value;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367c_setAsicIGMPv1Opeartion
 * Description:
 *      Set port-based IGMPv1 Control packet action
 * Input:
 *      port            - port number
 *      igmpv1_op       - IGMPv1 control packet action
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_PORT_ID  - Error PORT ID
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicIGMPv1Opeartion(rtk_uint32 port, rtk_uint32 igmpv1_op)
{
    ret_t   retVal;

    if(port > RTL8367C_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(igmpv1_op >= PROTOCOL_OP_END)
        return RT_ERR_INPUT;

    /* IGMPv1 operation */
    if(port < 8)
    {
        retVal = rtl8367c_setAsicRegBits(RTL8367C_REG_IGMP_PORT0_CONTROL + port, RTL8367C_IGMP_PORT0_CONTROL_IGMPV1_OP_MASK, igmpv1_op);
        if(retVal != RT_ERR_OK)
            return retVal;
    }
    else
    {
        retVal = rtl8367c_setAsicRegBits(RTL8367C_REG_IGMP_PORT8_CONTROL + port - 8, RTL8367C_IGMP_PORT0_CONTROL_IGMPV1_OP_MASK, igmpv1_op);
        if(retVal != RT_ERR_OK)
            return retVal;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367c_getAsicIGMPv1Opeartion
 * Description:
 *      Get port-based IGMPv1 Control packet action
 * Input:
 *      port            - port number
 * Output:
 *      igmpv1_op       - IGMPv1 control packet action
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_PORT_ID  - Error PORT ID
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicIGMPv1Opeartion(rtk_uint32 port, rtk_uint32 *igmpv1_op)
{
    ret_t   retVal;
    rtk_uint32  value;

    if(port > RTL8367C_PORTIDMAX)
        return RT_ERR_PORT_ID;

    /* IGMPv1 operation */
    if(port < 8)
    {
        retVal = rtl8367c_getAsicRegBits(RTL8367C_REG_IGMP_PORT0_CONTROL + port, RTL8367C_IGMP_PORT0_CONTROL_IGMPV1_OP_MASK, &value);
        if(retVal != RT_ERR_OK)
            return retVal;
    }
    else
    {
        retVal = rtl8367c_getAsicRegBits(RTL8367C_REG_IGMP_PORT8_CONTROL + port - 8, RTL8367C_IGMP_PORT0_CONTROL_IGMPV1_OP_MASK, &value);
        if(retVal != RT_ERR_OK)
            return retVal;
    }

    *igmpv1_op = value;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367c_setAsicIGMPv2Opeartion
 * Description:
 *      Set port-based IGMPv2 Control packet action
 * Input:
 *      port            - port number
 *      igmpv2_op       - IGMPv2 control packet action
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_PORT_ID  - Error PORT ID
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicIGMPv2Opeartion(rtk_uint32 port, rtk_uint32 igmpv2_op)
{
    ret_t   retVal;

    if(port > RTL8367C_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(igmpv2_op >= PROTOCOL_OP_END)
        return RT_ERR_INPUT;

    /* IGMPv2 operation */
    if(port < 8)
    {
        retVal = rtl8367c_setAsicRegBits(RTL8367C_REG_IGMP_PORT0_CONTROL + port, RTL8367C_IGMP_PORT0_CONTROL_IGMPV2_OP_MASK, igmpv2_op);
        if(retVal != RT_ERR_OK)
            return retVal;
    }
    else
    {
        retVal = rtl8367c_setAsicRegBits(RTL8367C_REG_IGMP_PORT8_CONTROL + port - 8, RTL8367C_IGMP_PORT0_CONTROL_IGMPV2_OP_MASK, igmpv2_op);
        if(retVal != RT_ERR_OK)
            return retVal;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367c_getAsicIGMPv2Opeartion
 * Description:
 *      Get port-based IGMPv2 Control packet action
 * Input:
 *      port            - port number
 * Output:
 *      igmpv2_op       - IGMPv2 control packet action
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_PORT_ID  - Error PORT ID
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicIGMPv2Opeartion(rtk_uint32 port, rtk_uint32 *igmpv2_op)
{
    ret_t   retVal;
    rtk_uint32  value;

    if(port > RTL8367C_PORTIDMAX)
        return RT_ERR_PORT_ID;

    /* IGMPv2 operation */
    if(port < 8)
    {
        retVal = rtl8367c_getAsicRegBits(RTL8367C_REG_IGMP_PORT0_CONTROL + port, RTL8367C_IGMP_PORT0_CONTROL_IGMPV2_OP_MASK, &value);
        if(retVal != RT_ERR_OK)
            return retVal;
    }
    else
    {
        retVal = rtl8367c_getAsicRegBits(RTL8367C_REG_IGMP_PORT8_CONTROL + port - 8, RTL8367C_IGMP_PORT0_CONTROL_IGMPV2_OP_MASK, &value);
        if(retVal != RT_ERR_OK)
            return retVal;
    }

    *igmpv2_op = value;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367c_setAsicIGMPv3Opeartion
 * Description:
 *      Set port-based IGMPv3 Control packet action
 * Input:
 *      port            - port number
 *      igmpv3_op       - IGMPv3 control packet action
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_PORT_ID  - Error PORT ID
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicIGMPv3Opeartion(rtk_uint32 port, rtk_uint32 igmpv3_op)
{
    ret_t   retVal;

    if(port > RTL8367C_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(igmpv3_op >= PROTOCOL_OP_END)
        return RT_ERR_INPUT;

    /* IGMPv3 operation */
    if(port < 8)
    {
        retVal = rtl8367c_setAsicRegBits(RTL8367C_REG_IGMP_PORT0_CONTROL + port, RTL8367C_IGMP_PORT0_CONTROL_IGMPV3_OP_MASK, igmpv3_op);
        if(retVal != RT_ERR_OK)
            return retVal;
    }
    else
    {
        retVal = rtl8367c_setAsicRegBits(RTL8367C_REG_IGMP_PORT8_CONTROL + port - 8, RTL8367C_IGMP_PORT0_CONTROL_IGMPV3_OP_MASK, igmpv3_op);
        if(retVal != RT_ERR_OK)
            return retVal;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367c_getAsicIGMPv3Opeartion
 * Description:
 *      Get port-based IGMPv3 Control packet action
 * Input:
 *      port            - port number
 * Output:
 *      igmpv3_op       - IGMPv3 control packet action
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_PORT_ID  - Error PORT ID
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicIGMPv3Opeartion(rtk_uint32 port, rtk_uint32 *igmpv3_op)
{
    ret_t   retVal;
    rtk_uint32  value;

    if(port > RTL8367C_PORTIDMAX)
        return RT_ERR_PORT_ID;

    /* IGMPv3 operation */
    if(port < 8)
    {
        retVal = rtl8367c_getAsicRegBits(RTL8367C_REG_IGMP_PORT0_CONTROL + port, RTL8367C_IGMP_PORT0_CONTROL_IGMPV3_OP_MASK, &value);
        if(retVal != RT_ERR_OK)
            return retVal;
    }
    else
    {
        retVal = rtl8367c_getAsicRegBits(RTL8367C_REG_IGMP_PORT8_CONTROL + port - 8, RTL8367C_IGMP_PORT0_CONTROL_IGMPV3_OP_MASK, &value);
        if(retVal != RT_ERR_OK)
            return retVal;
    }

    *igmpv3_op = value;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367c_setAsicMLDv1Opeartion
 * Description:
 *      Set port-based MLDv1 Control packet action
 * Input:
 *      port            - port number
 *      mldv1_op        - MLDv1 control packet action
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_PORT_ID  - Error PORT ID
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicMLDv1Opeartion(rtk_uint32 port, rtk_uint32 mldv1_op)
{
    ret_t   retVal;

    if(port > RTL8367C_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(mldv1_op >= PROTOCOL_OP_END)
        return RT_ERR_INPUT;

    /* MLDv1 operation */
    if(port < 8)
    {
        retVal = rtl8367c_setAsicRegBits(RTL8367C_REG_IGMP_PORT0_CONTROL + port, RTL8367C_IGMP_PORT0_CONTROL_MLDv1_OP_MASK, mldv1_op);
        if(retVal != RT_ERR_OK)
            return retVal;
    }
    else
    {
        retVal = rtl8367c_setAsicRegBits(RTL8367C_REG_IGMP_PORT8_CONTROL + port - 8, RTL8367C_IGMP_PORT0_CONTROL_MLDv1_OP_MASK, mldv1_op);
        if(retVal != RT_ERR_OK)
            return retVal;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367c_getAsicMLDv1Opeartion
 * Description:
 *      Get port-based MLDv1 Control packet action
 * Input:
 *      port            - port number
 * Output:
 *      mldv1_op        - MLDv1 control packet action
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_PORT_ID  - Error PORT ID
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicMLDv1Opeartion(rtk_uint32 port, rtk_uint32 *mldv1_op)
{
    ret_t   retVal;
    rtk_uint32  value;

    if(port > RTL8367C_PORTIDMAX)
        return RT_ERR_PORT_ID;

    /* MLDv1 operation */
    if(port < 8)
    {
        retVal = rtl8367c_getAsicRegBits(RTL8367C_REG_IGMP_PORT0_CONTROL + port, RTL8367C_IGMP_PORT0_CONTROL_MLDv1_OP_MASK, &value);
        if(retVal != RT_ERR_OK)
            return retVal;
    }
    else
    {
        retVal = rtl8367c_getAsicRegBits(RTL8367C_REG_IGMP_PORT8_CONTROL + port - 8, RTL8367C_IGMP_PORT0_CONTROL_MLDv1_OP_MASK, &value);
        if(retVal != RT_ERR_OK)
            return retVal;
    }

    *mldv1_op = value;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367c_setAsicMLDv2Opeartion
 * Description:
 *      Set port-based MLDv2 Control packet action
 * Input:
 *      port            - port number
 *      mldv2_op        - MLDv2 control packet action
 * Output:
 *      none
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_PORT_ID  - Error PORT ID
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicMLDv2Opeartion(rtk_uint32 port, rtk_uint32 mldv2_op)
{
    ret_t   retVal;

    if(port > RTL8367C_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(mldv2_op >= PROTOCOL_OP_END)
        return RT_ERR_INPUT;

    /* MLDv2 operation */
    if(port < 8)
    {
        retVal = rtl8367c_setAsicRegBits(RTL8367C_REG_IGMP_PORT0_CONTROL + port, RTL8367C_IGMP_PORT0_CONTROL_MLDv2_OP_MASK, mldv2_op);
        if(retVal != RT_ERR_OK)
            return retVal;
    }
    else
    {
        retVal = rtl8367c_setAsicRegBits(RTL8367C_REG_IGMP_PORT8_CONTROL + port - 8, RTL8367C_IGMP_PORT0_CONTROL_MLDv2_OP_MASK, mldv2_op);
        if(retVal != RT_ERR_OK)
            return retVal;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367c_getAsicMLDv2Opeartion
 * Description:
 *      Get port-based MLDv2 Control packet action
 * Input:
 *      port            - port number
 * Output:
 *      mldv2_op        - MLDv2 control packet action
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_PORT_ID  - Error PORT ID
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicMLDv2Opeartion(rtk_uint32 port, rtk_uint32 *mldv2_op)
{
    ret_t   retVal;
    rtk_uint32  value;

    if(port > RTL8367C_PORTIDMAX)
        return RT_ERR_PORT_ID;

    /* MLDv2 operation */
    if(port < 8)
    {
        retVal = rtl8367c_getAsicRegBits(RTL8367C_REG_IGMP_PORT0_CONTROL + port, RTL8367C_IGMP_PORT0_CONTROL_MLDv2_OP_MASK, &value);
        if(retVal != RT_ERR_OK)
            return retVal;
    }
    else
    {
        retVal = rtl8367c_getAsicRegBits(RTL8367C_REG_IGMP_PORT8_CONTROL + port - 8, RTL8367C_IGMP_PORT0_CONTROL_MLDv2_OP_MASK, &value);
        if(retVal != RT_ERR_OK)
            return retVal;
    }

    *mldv2_op = value;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367c_setAsicIGMPPortMAXGroup
 * Description:
 *      Set per-port Max group number
 * Input:
 *      port        - Physical port number (0~7)
 *      max_group   - max IGMP group
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicIGMPPortMAXGroup(rtk_uint32 port, rtk_uint32 max_group)
{
    ret_t retVal;

    if(port > RTL8367C_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(max_group > RTL8367C_IGMP_MAX_GOUP)
        return RT_ERR_OUT_OF_RANGE;

    if(port < 8)
    {
        retVal = rtl8367c_setAsicRegBits(RTL8367C_REG_IGMP_PORT01_MAX_GROUP + (port/2), RTL8367C_PORT0_MAX_GROUP_MASK << (RTL8367C_PORT1_MAX_GROUP_OFFSET * (port%2)), max_group);
        if(retVal != RT_ERR_OK)
            return retVal;
    }
    else
    {
        retVal = rtl8367c_setAsicRegBits(RTL8367C_REG_IGMP_PORT89_MAX_GROUP + (port/2), RTL8367C_PORT0_MAX_GROUP_MASK << (RTL8367C_PORT1_MAX_GROUP_OFFSET * (port%2)), max_group);
        if(retVal != RT_ERR_OK)
            return retVal;
    }

    return RT_ERR_OK;
}
/* Function Name:
 *      rtl8367c_getAsicIGMPPortMAXGroup
 * Description:
 *      Get per-port Max group number
 * Input:
 *      port        - Physical port number (0~7)
 *      max_group   - max IGMP group
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicIGMPPortMAXGroup(rtk_uint32 port, rtk_uint32 *max_group)
{
    ret_t   retVal;
    rtk_uint32  value;

    if(port > RTL8367C_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(port < 8)
    {
        retVal = rtl8367c_getAsicRegBits(RTL8367C_REG_IGMP_PORT01_MAX_GROUP + (port/2), RTL8367C_PORT0_MAX_GROUP_MASK << (RTL8367C_PORT1_MAX_GROUP_OFFSET * (port%2)), &value);
        if(retVal != RT_ERR_OK)
            return retVal;
    }
    else
    {
        retVal = rtl8367c_getAsicRegBits(RTL8367C_REG_IGMP_PORT89_MAX_GROUP + (port/2), RTL8367C_PORT0_MAX_GROUP_MASK << (RTL8367C_PORT1_MAX_GROUP_OFFSET * (port%2)), &value);
        if(retVal != RT_ERR_OK)
            return retVal;
    }

    *max_group = value;
    return RT_ERR_OK;
}
/* Function Name:
 *      rtl8367c_getAsicIGMPPortCurrentGroup
 * Description:
 *      Get per-port current group number
 * Input:
 *      port            - Physical port number (0~7)
 *      current_group   - current IGMP group
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicIGMPPortCurrentGroup(rtk_uint32 port, rtk_uint32 *current_group)
{
    ret_t   retVal;
    rtk_uint32  value;

    if(port > RTL8367C_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(port < 8)
    {
        retVal = rtl8367c_getAsicRegBits(RTL8367C_REG_IGMP_PORT01_CURRENT_GROUP + (port/2), RTL8367C_PORT0_CURRENT_GROUP_MASK << (RTL8367C_PORT1_CURRENT_GROUP_OFFSET * (port%2)), &value);
        if(retVal != RT_ERR_OK)
            return retVal;
    }
    else
    {
        retVal = rtl8367c_getAsicRegBits(RTL8367C_REG_IGMP_PORT89_CURRENT_GROUP + ((port - 8)/2), RTL8367C_PORT0_CURRENT_GROUP_MASK << (RTL8367C_PORT1_CURRENT_GROUP_OFFSET * (port%2)), &value);
        if(retVal != RT_ERR_OK)
            return retVal;
    }

    *current_group = value;
    return RT_ERR_OK;
}
/* Function Name:
 *      rtl8367c_getAsicIGMPGroup
 * Description:
 *      Get IGMP group
 * Input:
 *      idx     - Group index (0~255)
 *      valid   - valid bit
 *      grp     - IGMP group
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - Success
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_OUT_OF_RANGE     - Group index is out of range
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicIGMPGroup(rtk_uint32 idx, rtk_uint32 *valid, rtl8367c_igmpgroup *grp)
{
    ret_t   retVal;
    rtk_uint32  regAddr, regData;
    rtk_uint32  i;
    rtk_uint32  groupInfo = 0;

    if(idx > RTL8367C_IGMP_MAX_GOUP)
        return RT_ERR_OUT_OF_RANGE;

    /* Write ACS_ADR register for data bits */
    regAddr = RTL8367C_TABLE_ACCESS_ADDR_REG;
    regData = idx;
    retVal = rtl8367c_setAsicReg(regAddr, regData);
    if(retVal != RT_ERR_OK)
        return retVal;

    /* Write ACS_CMD register */
    regAddr = RTL8367C_TABLE_ACCESS_CTRL_REG;
    regData = RTL8367C_TABLE_ACCESS_REG_DATA(TB_OP_READ, TB_TARGET_IGMP_GROUP);
    retVal = rtl8367c_setAsicRegBits(regAddr, RTL8367C_TABLE_TYPE_MASK | RTL8367C_COMMAND_TYPE_MASK, regData);
    if(retVal != RT_ERR_OK)
        return retVal;

    /* Read Data Bits */
    regAddr = RTL8367C_TABLE_ACCESS_RDDATA_BASE;
    for(i = 0 ;i <= 1; i++)
    {
        retVal = rtl8367c_getAsicReg(regAddr, &regData);
        if(retVal != RT_ERR_OK)
            return retVal;

        groupInfo |= ((regData & 0xFFFF) << (i * 16));
        regAddr ++;
    }

    grp->p0_timer = groupInfo & 0x00000007;
    grp->p1_timer = (groupInfo >> 3) & 0x00000007;
    grp->p2_timer = (groupInfo >> 6) & 0x00000007;
    grp->p3_timer = (groupInfo >> 9) & 0x00000007;
    grp->p4_timer = (groupInfo >> 12) & 0x00000007;
    grp->p5_timer = (groupInfo >> 15) & 0x00000007;
    grp->p6_timer = (groupInfo >> 18) & 0x00000007;
    grp->p7_timer = (groupInfo >> 21) & 0x00000007;
    grp->report_supp_flag = (groupInfo >> 24) & 0x00000001;
    grp->p8_timer = (groupInfo >> 25) & 0x00000007;
    grp->p9_timer = (groupInfo >> 28) & 0x00000007;
    grp->p10_timer = (groupInfo >> 31) & 0x00000001;

    regAddr = RTL8367C_TABLE_ACCESS_RDDATA_BASE + 2;
    retVal = rtl8367c_getAsicReg(regAddr, &regData);
    if(retVal != RT_ERR_OK)
        return retVal;

    grp->p10_timer |= (regData & 0x00000003) << 1;

    /* Valid bit */
    retVal = rtl8367c_getAsicReg(RTL8367C_IGMP_GROUP_USAGE_REG(idx), &regData);
    if(retVal != RT_ERR_OK)
        return retVal;

    *valid = ((regData & (0x0001 << (idx %16))) != 0) ? 1 : 0;

    return RT_ERR_OK;
}
/* Function Name:
 *      rtl8367c_setAsicIpMulticastPortIsoLeaky
 * Description:
 *      Set IP multicast Port Isolation leaky
 * Input:
 *      port        - Physical port number (0~7)
 *      enabled     - 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicIpMulticastPortIsoLeaky(rtk_uint32 port, rtk_uint32 enabled)
{
    ret_t   retVal;

    if(port > RTL8367C_PORTIDMAX)
        return RT_ERR_PORT_ID;

    retVal = rtl8367c_setAsicRegBits(RTL8367C_IPMCAST_PORTISO_LEAKY_REG, (0x0001 << port), enabled);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367c_getAsicIpMulticastPortIsoLeaky
 * Description:
 *      Get IP multicast Port Isolation leaky
 * Input:
 *      port        - Physical port number (0~7)
 *      enabled     - 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicIpMulticastPortIsoLeaky(rtk_uint32 port, rtk_uint32 *enabled)
{
    ret_t   retVal;
    rtk_uint32  regData;

    retVal = rtl8367c_getAsicRegBits(RTL8367C_IPMCAST_PORTISO_LEAKY_REG, (0x0001 << port), &regData);
    if(retVal != RT_ERR_OK)
        return retVal;

    *enabled = regData;
    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367c_setAsicIGMPReportLeaveFlood
 * Description:
 *      Set IGMP/MLD Report/Leave flood
 * Input:
 *      flood   - 0: Reserved, 1: flooding to router ports, 2: flooding to all ports, 3: flooding to router port or to all ports if there is no router port
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicIGMPReportLeaveFlood(rtk_uint32 flood)
{
    ret_t   retVal;

    retVal = rtl8367c_setAsicRegBits(RTL8367C_REG_IGMP_MLD_CFG3, RTL8367C_REPORT_LEAVE_FORWARD_MASK, flood);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367c_getAsicIGMPReportLeaveFlood
 * Description:
 *      Get IGMP/MLD Report/Leave flood
 * Input:
 *      None
 * Output:
 *      pflood  - 0: Reserved, 1: flooding to router ports, 2: flooding to all ports, 3: flooding to router port or to all ports if there is no router port
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicIGMPReportLeaveFlood(rtk_uint32 *pFlood)
{
    ret_t   retVal;
    rtk_uint32  regData;

    retVal = rtl8367c_getAsicRegBits(RTL8367C_REG_IGMP_MLD_CFG3, RTL8367C_REPORT_LEAVE_FORWARD_MASK, &regData);
    if(retVal != RT_ERR_OK)
        return retVal;

    *pFlood = regData;
    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367c_setAsicIGMPDropLeaveZero
 * Description:
 *      Set the function of droppping Leave packet with group IP = 0.0.0.0
 * Input:
 *      drop    - 1: Drop, 0:Bypass
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicIGMPDropLeaveZero(rtk_uint32 drop)
{
    ret_t   retVal;

    retVal = rtl8367c_setAsicRegBit(RTL8367C_REG_IGMP_MLD_CFG1, RTL8367C_DROP_LEAVE_ZERO_OFFSET, drop);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367c_getAsicIGMPDropLeaveZero
 * Description:
 *      Get the function of droppping Leave packet with group IP = 0.0.0.0
 * Input:
 *      None
 * Output:
 *      pDrop    - 1: Drop, 0:Bypass
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicIGMPDropLeaveZero(rtk_uint32 *pDrop)
{
    ret_t   retVal;
    rtk_uint32  regData;

    retVal = rtl8367c_getAsicRegBit(RTL8367C_REG_IGMP_MLD_CFG1, RTL8367C_DROP_LEAVE_ZERO_OFFSET, &regData);
    if(retVal != RT_ERR_OK)
        return retVal;

    *pDrop = regData;
    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367c_setAsicIGMPBypassStormCTRL
 * Description:
 *      Set the function of bypass strom control for IGMP/MLD packet
 * Input:
 *      bypass    - 1: Bypass, 0:not bypass
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicIGMPBypassStormCTRL(rtk_uint32 bypass)
{
    ret_t   retVal;

    retVal = rtl8367c_setAsicRegBit(RTL8367C_REG_IGMP_MLD_CFG0, RTL8367C_IGMP_MLD_DISCARD_STORM_FILTER_OFFSET, bypass);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367c_getAsicIGMPBypassStormCTRL
 * Description:
 *      Set the function of bypass strom control for IGMP/MLD packet
 * Input:
 *      None
 * Output:
 *      pBypass    - 1: Bypass, 0:not bypass
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicIGMPBypassStormCTRL(rtk_uint32 *pBypass)
{
    ret_t   retVal;
    rtk_uint32  regData;

    retVal = rtl8367c_getAsicRegBit(RTL8367C_REG_IGMP_MLD_CFG0, RTL8367C_IGMP_MLD_DISCARD_STORM_FILTER_OFFSET, &regData);
    if(retVal != RT_ERR_OK)
        return retVal;

    *pBypass = regData;
    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367c_setAsicIGMPIsoLeaky
 * Description:
 *      Set Port Isolation leaky for IGMP/MLD packet
 * Input:
 *      leaky    - 1: Leaky, 0:not leaky
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicIGMPIsoLeaky(rtk_uint32 leaky)
{
    ret_t   retVal;

    retVal = rtl8367c_setAsicRegBit(RTL8367C_REG_IGMP_MLD_CFG0, RTL8367C_IGMP_MLD_PORTISO_LEAKY_OFFSET, leaky);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367c_getAsicIGMPIsoLeaky
 * Description:
 *      Get Port Isolation leaky for IGMP/MLD packet
 * Input:
 *      Noen
 * Output:
 *      pLeaky    - 1: Leaky, 0:not leaky
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicIGMPIsoLeaky(rtk_uint32 *pLeaky)
{
    ret_t   retVal;
    rtk_uint32  regData;

    retVal = rtl8367c_getAsicRegBit(RTL8367C_REG_IGMP_MLD_CFG0, RTL8367C_IGMP_MLD_PORTISO_LEAKY_OFFSET, &regData);
    if(retVal != RT_ERR_OK)
        return retVal;

    *pLeaky = regData;
    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367c_setAsicIGMPVLANLeaky
 * Description:
 *      Set VLAN leaky for IGMP/MLD packet
 * Input:
 *      leaky    - 1: Leaky, 0:not leaky
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicIGMPVLANLeaky(rtk_uint32 leaky)
{
    ret_t   retVal;

    retVal = rtl8367c_setAsicRegBit(RTL8367C_REG_IGMP_MLD_CFG0, RTL8367C_IGMP_MLD_VLAN_LEAKY_OFFSET, leaky);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367c_getAsicIGMPVLANLeaky
 * Description:
 *      Get VLAN leaky for IGMP/MLD packet
 * Input:
 *      Noen
 * Output:
 *      pLeaky    - 1: Leaky, 0:not leaky
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicIGMPVLANLeaky(rtk_uint32 *pLeaky)
{
    ret_t   retVal;
    rtk_uint32  regData;

    retVal = rtl8367c_getAsicRegBit(RTL8367C_REG_IGMP_MLD_CFG0, RTL8367C_IGMP_MLD_VLAN_LEAKY_OFFSET, &regData);
    if(retVal != RT_ERR_OK)
        return retVal;

    *pLeaky = regData;
    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367c_setAsicIGMPBypassGroup
 * Description:
 *      Set IGMP/MLD Bypass group
 * Input:
 *      bypassType  - Bypass type
 *      enabled     - enabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicIGMPBypassGroup(rtk_uint32 bypassType, rtk_uint32 enabled)
{
    ret_t   retVal;
    rtk_uint32 offset;

    switch(bypassType)
    {
        case BYPASS_224_0_0_X:
            offset = RTL8367C_IGMP_MLD_IP4_BYPASS_224_0_0_OFFSET;
            break;
        case BYPASS_224_0_1_X:
            offset = RTL8367C_IGMP_MLD_IP4_BYPASS_224_0_1_OFFSET;
            break;
        case BYPASS_239_255_255_X:
            offset = RTL8367C_IGMP_MLD_IP4_BYPASS_239_255_255_OFFSET;
            break;
        case BYPASS_IPV6_00XX:
            offset = RTL8367C_IGMP_MLD_IP6_BYPASS_OFFSET;
            break;
        default:
            return RT_ERR_INPUT;
    }

    retVal = rtl8367c_setAsicRegBit(RTL8367C_REG_IGMP_MLD_CFG3, offset, enabled);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367c_getAsicIGMPBypassGroup
 * Description:
 *      Get IGMP/MLD Bypass group
 * Input:
 *      bypassType  - Bypass type
 * Output:
 *      pEnabled    - enabled
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicIGMPBypassGroup(rtk_uint32 bypassType, rtk_uint32 *pEnabled)
{
    ret_t   retVal;
    rtk_uint32 offset;

    switch(bypassType)
    {
        case BYPASS_224_0_0_X:
            offset = RTL8367C_IGMP_MLD_IP4_BYPASS_224_0_0_OFFSET;
            break;
        case BYPASS_224_0_1_X:
            offset = RTL8367C_IGMP_MLD_IP4_BYPASS_224_0_1_OFFSET;
            break;
        case BYPASS_239_255_255_X:
            offset = RTL8367C_IGMP_MLD_IP4_BYPASS_239_255_255_OFFSET;
            break;
        case BYPASS_IPV6_00XX:
            offset = RTL8367C_IGMP_MLD_IP6_BYPASS_OFFSET;
            break;
        default:
            return RT_ERR_INPUT;
    }

    retVal = rtl8367c_getAsicRegBit(RTL8367C_REG_IGMP_MLD_CFG3, offset, pEnabled);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}


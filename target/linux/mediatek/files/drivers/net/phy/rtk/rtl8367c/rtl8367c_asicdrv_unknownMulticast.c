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
 * Feature : Unknown multicast related functions
 *
 */

#include <rtl8367c_asicdrv_unknownMulticast.h>

/* Function Name:
 *      rtl8367c_setAsicUnknownL2MulticastBehavior
 * Description:
 *      Set behavior of L2 multicast
 * Input:
 *      port    - Physical port number (0~7)
 *      behave  - 0: flooding, 1: drop, 2: trap
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number
 *      RT_ERR_NOT_ALLOWED  - Invalid operation
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicUnknownL2MulticastBehavior(rtk_uint32 port, rtk_uint32 behave)
{
    ret_t retVal;

    if(port >  RTL8367C_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(behave >= L2_UNKOWN_MULTICAST_END)
        return RT_ERR_NOT_ALLOWED;
    if(port < 8)
    {
        retVal = rtl8367c_setAsicRegBits(RTL8367C_UNKNOWN_L2_MULTICAST_REG(port), RTL8367C_UNKNOWN_L2_MULTICAST_MASK(port), behave);
        if(retVal != RT_ERR_OK)
            return retVal;
    }
    else
    {
        retVal = rtl8367c_setAsicRegBits(RTL8367C_REG_UNKNOWN_L2_MULTICAST_CTRL1, 3 << ((port - 8) << 1), behave);
        if(retVal != RT_ERR_OK)
            return retVal;
    }

    return RT_ERR_OK;
}
/* Function Name:
 *      rtl8367c_getAsicUnknownL2MulticastBehavior
 * Description:
 *      Get behavior of L2 multicast
 * Input:
 *      port    - Physical port number (0~7)
 *      pBehave     - 0: flooding, 1: drop, 2: trap
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicUnknownL2MulticastBehavior(rtk_uint32 port, rtk_uint32 *pBehave)
{
    ret_t retVal;

    if(port >  RTL8367C_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(port < 8)
    {
        retVal = rtl8367c_getAsicRegBits(RTL8367C_UNKNOWN_L2_MULTICAST_REG(port), RTL8367C_UNKNOWN_L2_MULTICAST_MASK(port), pBehave);
        if (retVal != RT_ERR_OK)
            return retVal;
    }
    else
    {
        retVal = rtl8367c_getAsicRegBits(RTL8367C_REG_UNKNOWN_L2_MULTICAST_CTRL1, 3 << ((port - 8) << 1), pBehave);
        if (retVal != RT_ERR_OK)
            return retVal;
    }

    return RT_ERR_OK;
}
/* Function Name:
 *      rtl8367c_setAsicUnknownIPv4MulticastBehavior
 * Description:
 *      Set behavior of IPv4 multicast
 * Input:
 *      port    - Physical port number (0~7)
 *      behave  - 0: flooding, 1: drop, 2: trap
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number
 *      RT_ERR_NOT_ALLOWED  - Invalid operation
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicUnknownIPv4MulticastBehavior(rtk_uint32 port, rtk_uint32 behave)
{
    if(port >  RTL8367C_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(behave >= L3_UNKOWN_MULTICAST_END)
        return RT_ERR_NOT_ALLOWED;

    return rtl8367c_setAsicRegBits(RTL8367C_UNKNOWN_IPV4_MULTICAST_REG(port), RTL8367C_UNKNOWN_IPV4_MULTICAST_MASK(port), behave);
}
/* Function Name:
 *      rtl8367c_getAsicUnknownIPv4MulticastBehavior
 * Description:
 *      Get behavior of IPv4 multicast
 * Input:
 *      port    - Physical port number (0~7)
 *      pBehave     - 0: flooding, 1: drop, 2: trap
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicUnknownIPv4MulticastBehavior(rtk_uint32 port, rtk_uint32 *pBehave)
{
    if(port >  RTL8367C_PORTIDMAX)
        return RT_ERR_PORT_ID;

    return rtl8367c_getAsicRegBits(RTL8367C_UNKNOWN_IPV4_MULTICAST_REG(port), RTL8367C_UNKNOWN_IPV4_MULTICAST_MASK(port), pBehave);
}
/* Function Name:
 *      rtl8367c_setAsicUnknownIPv6MulticastBehavior
 * Description:
 *      Set behavior of IPv6 multicast
 * Input:
 *      port    - Physical port number (0~7)
 *      behave  - 0: flooding, 1: drop, 2: trap
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number
 *      RT_ERR_NOT_ALLOWED  - Invalid operation
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicUnknownIPv6MulticastBehavior(rtk_uint32 port, rtk_uint32 behave)
{
    if(port >  RTL8367C_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(behave >= L3_UNKOWN_MULTICAST_END)
        return RT_ERR_NOT_ALLOWED;

    return rtl8367c_setAsicRegBits(RTL8367C_UNKNOWN_IPV6_MULTICAST_REG(port), RTL8367C_UNKNOWN_IPV6_MULTICAST_MASK(port), behave);
}
/* Function Name:
 *      rtl8367c_getAsicUnknownIPv6MulticastBehavior
 * Description:
 *      Get behavior of IPv6 multicast
 * Input:
 *      port    - Physical port number (0~7)
 *      pBehave     - 0: flooding, 1: drop, 2: trap
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicUnknownIPv6MulticastBehavior(rtk_uint32 port, rtk_uint32 *pBehave)
{
    if(port >  RTL8367C_PORTIDMAX)
        return RT_ERR_PORT_ID;

    return rtl8367c_getAsicRegBits(RTL8367C_UNKNOWN_IPV6_MULTICAST_REG(port), RTL8367C_UNKNOWN_IPV6_MULTICAST_MASK(port), pBehave);
}
/* Function Name:
 *      rtl8367c_setAsicUnknownMulticastTrapPriority
 * Description:
 *      Set trap priority of unknown multicast frame
 * Input:
 *      priority    - priority (0~7)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - Success
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_QOS_INT_PRIORITY - Invalid priority
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicUnknownMulticastTrapPriority(rtk_uint32 priority)
{
    if(priority > RTL8367C_PRIMAX)
        return RT_ERR_QOS_INT_PRIORITY;

    return rtl8367c_setAsicRegBits(RTL8367C_QOS_TRAP_PRIORITY_CTRL0_REG, RTL8367C_UNKNOWN_MC_PRIORTY_MASK, priority);
}
/* Function Name:
 *      rtl8367c_getAsicUnknownMulticastTrapPriority
 * Description:
 *      Get trap priority of unknown multicast frame
 * Input:
 *      pPriority   - priority (0~7)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicUnknownMulticastTrapPriority(rtk_uint32 *pPriority)
{
    return rtl8367c_getAsicRegBits(RTL8367C_QOS_TRAP_PRIORITY_CTRL0_REG, RTL8367C_UNKNOWN_MC_PRIORTY_MASK, pPriority);
}

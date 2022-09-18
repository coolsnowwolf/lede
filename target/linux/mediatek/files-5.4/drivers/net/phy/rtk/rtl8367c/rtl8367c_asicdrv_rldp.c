/*
 * Copyright (C) 2013 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * Unless you and Realtek execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2,
 * available at https://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 *
 * $Revision: 42321 $
 * $Date: 2013-08-26 13:51:29 +0800 (週一, 26 八月 2013) $
 *
 * Purpose : RTL8367C switch high-level API for RTL8367C
 * Feature : RLDP related functions
 *
 */

#include <rtl8367c_asicdrv_rldp.h>
/* Function Name:
 *      rtl8367c_setAsicRldp
 * Description:
 *      Set RLDP function enable/disable
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
ret_t rtl8367c_setAsicRldp(rtk_uint32 enabled)
{
    return rtl8367c_setAsicRegBit(RTL8367C_REG_RLDP_CTRL0, RTL8367C_RLDP_ENABLE_OFFSET, enabled);
}
/* Function Name:
 *      rtl8367c_getAsicRldp
 * Description:
 *      Get RLDP function enable/disable
 * Input:
 *      pEnabled    - 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicRldp(rtk_uint32 *pEnabled)
{
    return rtl8367c_getAsicRegBit(RTL8367C_REG_RLDP_CTRL0, RTL8367C_RLDP_ENABLE_OFFSET, pEnabled);
}
/* Function Name:
 *      rtl8367c_setAsicRldpEnable8051
 * Description:
 *      Set RLDP function handled by ASIC or 8051
 * Input:
 *      enabled     - 1: enabled 8051, 0: disabled 8051 (RLDP is handled by ASIC)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicRldpEnable8051(rtk_uint32 enabled)
{
    return rtl8367c_setAsicRegBit(RTL8367C_REG_RLDP_CTRL0, RTL8367C_RLDP_8051_ENABLE_OFFSET, enabled);
}
/* Function Name:
 *      rtl8367c_setAsicRldrtl8367c_getAsicRldpEnable8051pEnable8051
 * Description:
 *      Get RLDP function handled by ASIC or 8051
 * Input:
 *      pEnabled    - 1: enabled 8051, 0: disabled 8051 (RLDP is handled by ASIC)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicRldpEnable8051(rtk_uint32 *pEnabled)
{
    return rtl8367c_getAsicRegBit(RTL8367C_REG_RLDP_CTRL0, RTL8367C_RLDP_8051_ENABLE_OFFSET, pEnabled);
}
/* Function Name:
 *      rtl8367c_setAsicRldpCompareRandomNumber
 * Description:
 *      Set enable compare the random number field and seed field of RLDP frame
 * Input:
 *      enabled     - 1: enabled comparing random number, 0: disabled comparing random number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicRldpCompareRandomNumber(rtk_uint32 enabled)
{
    return rtl8367c_setAsicRegBit(RTL8367C_REG_RLDP_CTRL0, RTL8367C_RLDP_COMP_ID_OFFSET, enabled);
}
/* Function Name:
 *      rtl8367c_getAsicRldpCompareRandomNumber
 * Description:
 *      Get enable compare the random number field and seed field of RLDP frame
 * Input:
 *      pEnabled    - 1: enabled comparing random number, 0: disabled comparing random number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicRldpCompareRandomNumber(rtk_uint32 *pEnabled)
{
    return rtl8367c_getAsicRegBit(RTL8367C_REG_RLDP_CTRL0, RTL8367C_RLDP_COMP_ID_OFFSET, pEnabled);
}
/* Function Name:
 *      rtl8367c_setAsicRldpIndicatorSource
 * Description:
 *      Set buzzer and LED source when detecting a loop
 * Input:
 *      src     - 0: ASIC, 1: 8051
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicRldpIndicatorSource(rtk_uint32 src)
{
    return rtl8367c_setAsicRegBit(RTL8367C_REG_RLDP_CTRL0, RTL8367C_RLDP_INDICATOR_SOURCE_OFFSET, src);
}
/* Function Name:
 *      rtl8367c_getAsicRldpIndicatorSource
 * Description:
 *      Get buzzer and LED source when detecting a loop
 * Input:
 *      pSrc    - 0: ASIC, 1: 8051
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicRldpIndicatorSource(rtk_uint32 *pSrc)
{
    return rtl8367c_getAsicRegBit(RTL8367C_REG_RLDP_CTRL0, RTL8367C_RLDP_INDICATOR_SOURCE_OFFSET, pSrc);
}
/* Function Name:
 *      rtl8367c_setAsicRldpCheckingStatePara
 * Description:
 *      Set retry count and retry period of checking state
 * Input:
 *      retryCount  - 0~0xFF (times)
 *      retryPeriod - 0~0xFFFF (ms)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicRldpCheckingStatePara(rtk_uint32 retryCount, rtk_uint32 retryPeriod)
{
    ret_t retVal;

    if(retryCount > 0xFF)
        return RT_ERR_OUT_OF_RANGE;
    if(retryPeriod > RTL8367C_REGDATAMAX)
        return RT_ERR_OUT_OF_RANGE;

    retVal = rtl8367c_setAsicRegBits(RTL8367C_RLDP_RETRY_COUNT_REG, RTL8367C_RLDP_RETRY_COUNT_CHKSTATE_MASK, retryCount);
    if(retVal != RT_ERR_OK)
        return retVal;

    return rtl8367c_setAsicReg(RTL8367C_RLDP_RETRY_PERIOD_CHKSTATE_REG, retryPeriod);
}
/* Function Name:
 *      rtl8367c_getAsicRldpCheckingStatePara
 * Description:
 *      Get retry count and retry period of checking state
 * Input:
 *      pRetryCount     - 0~0xFF (times)
 *      pRetryPeriod    - 0~0xFFFF (ms)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicRldpCheckingStatePara(rtk_uint32 *pRetryCount, rtk_uint32 *pRetryPeriod)
{
    ret_t retVal;

    retVal = rtl8367c_getAsicRegBits(RTL8367C_RLDP_RETRY_COUNT_REG, RTL8367C_RLDP_RETRY_COUNT_CHKSTATE_MASK, pRetryCount);
    if(retVal != RT_ERR_OK)
        return retVal;

    return rtl8367c_getAsicReg(RTL8367C_RLDP_RETRY_PERIOD_CHKSTATE_REG, pRetryPeriod);
}
/* Function Name:
 *      rtl8367c_setAsicRldpLoopStatePara
 * Description:
 *      Set retry count and retry period of loop state
 * Input:
 *      retryCount  - 0~0xFF (times)
 *      retryPeriod - 0~0xFFFF (ms)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicRldpLoopStatePara(rtk_uint32 retryCount, rtk_uint32 retryPeriod)
{
    ret_t retVal;

    if(retryCount > 0xFF)
        return RT_ERR_OUT_OF_RANGE;

    if(retryPeriod > RTL8367C_REGDATAMAX)
        return RT_ERR_OUT_OF_RANGE;

    retVal = rtl8367c_setAsicRegBits(RTL8367C_RLDP_RETRY_COUNT_REG, RTL8367C_RLDP_RETRY_COUNT_LOOPSTATE_MASK, retryCount);
    if(retVal != RT_ERR_OK)
        return retVal;

    return rtl8367c_setAsicReg(RTL8367C_RLDP_RETRY_PERIOD_LOOPSTATE_REG, retryPeriod);
}
/* Function Name:
 *      rtl8367c_getAsicRldpLoopStatePara
 * Description:
 *      Get retry count and retry period of loop state
 * Input:
 *      pRetryCount     - 0~0xFF (times)
 *      pRetryPeriod    - 0~0xFFFF (ms)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicRldpLoopStatePara(rtk_uint32 *pRetryCount, rtk_uint32 *pRetryPeriod)
{
    ret_t retVal;

    retVal = rtl8367c_getAsicRegBits(RTL8367C_RLDP_RETRY_COUNT_REG, RTL8367C_RLDP_RETRY_COUNT_LOOPSTATE_MASK, pRetryCount);
    if(retVal != RT_ERR_OK)
        return retVal;

    return rtl8367c_getAsicReg(RTL8367C_RLDP_RETRY_PERIOD_LOOPSTATE_REG, pRetryPeriod);
}
/* Function Name:
 *      rtl8367c_setAsicRldpTxPortmask
 * Description:
 *      Set portmask that send/forward RLDP frame
 * Input:
 *      portmask    - 0~0xFF
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_MASK    - Invalid portmask
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicRldpTxPortmask(rtk_uint32 portmask)
{
    if(portmask > RTL8367C_PORTMASK)
        return RT_ERR_PORT_MASK;

    return rtl8367c_setAsicReg(RTL8367C_RLDP_TX_PMSK_REG, portmask);
}
/* Function Name:
 *      rtl8367c_getAsicRldpTxPortmask
 * Description:
 *      Get portmask that send/forward RLDP frame
 * Input:
 *      pPortmask   - 0~0xFF
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicRldpTxPortmask(rtk_uint32 *pPortmask)
{
    return rtl8367c_getAsicReg(RTL8367C_RLDP_TX_PMSK_REG, pPortmask);
}
/* Function Name:
 *      rtl8367c_setAsicRldpMagicNum
 * Description:
 *      Set Random seed of RLDP
 * Input:
 *      seed    - MAC
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicRldpMagicNum(ether_addr_t seed)
{
    ret_t retVal;
    rtk_uint32 regData;
    rtk_uint16 *accessPtr;
    rtk_uint32 i;

    accessPtr = (rtk_uint16*)&seed;

    for (i = 0; i < 3; i++)
    {
        regData = *accessPtr;
        retVal = rtl8367c_setAsicReg(RTL8367C_RLDP_MAGIC_NUM_REG_BASE + i, regData);
        if(retVal != RT_ERR_OK)
            return retVal;

        accessPtr++;
    }

    return retVal;
}
/* Function Name:
 *      rtl8367c_getAsicRldpMagicNum
 * Description:
 *      Get Random seed of RLDP
 * Input:
 *      pSeed   - MAC
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicRldpMagicNum(ether_addr_t *pSeed)
{
    ret_t retVal;
    rtk_uint32 regData;
    rtk_uint16 *accessPtr;
    rtk_uint32 i;

    accessPtr = (rtk_uint16*)pSeed;

    for(i = 0; i < 3; i++)
    {
        retVal = rtl8367c_getAsicReg(RTL8367C_RLDP_MAGIC_NUM_REG_BASE + i, &regData);
        if(retVal != RT_ERR_OK)
            return retVal;

        *accessPtr = regData;
        accessPtr++;
    }

    return retVal;
}

/* Function Name:
 *      rtl8367c_getAsicRldpLoopedPortmask
 * Description:
 *      Get looped portmask
 * Input:
 *      pPortmask   - 0~0xFF
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicRldpLoopedPortmask(rtk_uint32 *pPortmask)
{
    return rtl8367c_getAsicReg(RTL8367C_RLDP_LOOP_PMSK_REG, pPortmask);
}
/* Function Name:
 *      rtl8367c_getAsicRldpRandomNumber
 * Description:
 *      Get Random number of RLDP
 * Input:
 *      pRandNumber     - MAC
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicRldpRandomNumber(ether_addr_t *pRandNumber)
{
    ret_t retVal;
    rtk_uint32 regData;
    rtk_int16 accessPtr[3];
    rtk_uint32 i;

    for(i = 0; i < 3; i++)
    {
        retVal = rtl8367c_getAsicReg(RTL8367C_RLDP_RAND_NUM_REG_BASE+ i, &regData);
        if(retVal != RT_ERR_OK)
            return retVal;

        accessPtr[i] = regData;
    }

    memcpy(pRandNumber, accessPtr, 6);
    return retVal;
}
/* Function Name:
 *      rtl8367c_getAsicRldpLoopedPortmask
 * Description:
 *      Get port number of looped pair
 * Input:
 *      port        - Physical port number (0~7)
 *      pLoopedPair     - port (0~7)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicRldpLoopedPortPair(rtk_uint32 port, rtk_uint32 *pLoopedPair)
{
    if(port > RTL8367C_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(port < 8)
        return rtl8367c_getAsicRegBits(RTL8367C_RLDP_LOOP_PORT_REG(port), RTL8367C_RLDP_LOOP_PORT_MASK(port), pLoopedPair);
    else
        return rtl8367c_getAsicRegBits(RTL8367C_REG_RLDP_LOOP_PORT_REG4 + ((port - 8) >> 1), RTL8367C_RLDP_LOOP_PORT_MASK(port), pLoopedPair);
}
/* Function Name:
 *      rtl8367c_setAsicRlppTrap8051
 * Description:
 *      Set trap RLPP packet to 8051
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
ret_t rtl8367c_setAsicRlppTrap8051(rtk_uint32 enabled)
{
    return rtl8367c_setAsicRegBit(RTL8367C_REG_RLDP_CTRL0, RTL8367C_RLPP_8051_TRAP_OFFSET, enabled);
}
/* Function Name:
 *      rtl8367c_getAsicRlppTrap8051
 * Description:
 *      Get trap RLPP packet to 8051
 * Input:
 *      pEnabled    - 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicRlppTrap8051(rtk_uint32 *pEnabled)
{
    return rtl8367c_getAsicRegBit(RTL8367C_REG_RLDP_CTRL0, RTL8367C_RLPP_8051_TRAP_OFFSET, pEnabled);
}
/* Function Name:
 *      rtl8367c_setAsicRldpLeaveLoopedPortmask
 * Description:
 *      Clear leaved looped portmask
 * Input:
 *      portmask    - 0~0xFF
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicRldpLeaveLoopedPortmask(rtk_uint32 portmask)
{
    return rtl8367c_setAsicReg(RTL8367C_REG_RLDP_RELEASED_INDICATOR, portmask);
}
/* Function Name:
 *      rtl8367c_getAsicRldpLeaveLoopedPortmask
 * Description:
 *      Get leaved looped portmask
 * Input:
 *      pPortmask   - 0~0xFF
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicRldpLeaveLoopedPortmask(rtk_uint32 *pPortmask)
{
    return rtl8367c_getAsicReg(RTL8367C_REG_RLDP_RELEASED_INDICATOR, pPortmask);
}
/* Function Name:
 *      rtl8367c_setAsicRldpEnterLoopedPortmask
 * Description:
 *      Clear enter loop portmask
 * Input:
 *      portmask    - 0~0xFF
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicRldpEnterLoopedPortmask(rtk_uint32 portmask)
{
    return rtl8367c_setAsicReg(RTL8367C_REG_RLDP_LOOPED_INDICATOR, portmask);
}
/* Function Name:
 *      rtl8367c_getAsicRldpEnterLoopedPortmask
 * Description:
 *      Get enter loop portmask
 * Input:
 *      pPortmask   - 0~0xFF
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicRldpEnterLoopedPortmask(rtk_uint32 *pPortmask)
{
    return rtl8367c_getAsicReg(RTL8367C_REG_RLDP_LOOPED_INDICATOR, pPortmask);
}

/* Function Name:
 *      rtl8367c_setAsicRldpTriggerMode
 * Description:
 *      Set trigger RLDP mode
 * Input:
 *      mode    - 1: Periodically, 0: SA moving
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicRldpTriggerMode(rtk_uint32 enabled)
{
    return rtl8367c_setAsicRegBit(RTL8367C_REG_RLDP_CTRL0, RTL8367C_RLDP_TRIGGER_MODE_OFFSET, enabled);
}
/* Function Name:
 *      rtl8367c_getAsicRldpTriggerMode
 * Description:
 *      Get trigger RLDP mode
 * Input:
 *      pMode   - - 1: Periodically, 0: SA moving
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicRldpTriggerMode(rtk_uint32 *pEnabled)
{
    return rtl8367c_getAsicRegBit(RTL8367C_REG_RLDP_CTRL0, RTL8367C_RLDP_TRIGGER_MODE_OFFSET, pEnabled);
}

/* Function Name:
 *      rtl8367c_setAsicRldp8051Portmask
 * Description:
 *      Set 8051/CPU configured looped portmask
 * Input:
 *      portmask    - 0~0xFF
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_MASK    - Invalid portmask
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicRldp8051Portmask(rtk_uint32 portmask)
{
    ret_t retVal;
    if(portmask > RTL8367C_PORTMASK)
        return RT_ERR_PORT_MASK;

    retVal = rtl8367c_setAsicRegBits(RTL8367C_RLDP_CTRL0_REG,RTL8367C_RLDP_8051_LOOP_PORTMSK_MASK,portmask & 0xff);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8367c_setAsicRegBits(RTL8367C_REG_RLDP_CTRL5,RTL8367C_RLDP_CTRL5_MASK,(portmask >> 8) & 7);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}
/* Function Name:
 *      rtl8367c_getAsicRldp8051Portmask
 * Description:
 *      Get 8051/CPU configured looped portmask
 * Input:
 *      pPortmask   - 0~0xFF
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicRldp8051Portmask(rtk_uint32 *pPortmask)
{
    rtk_uint32 tmpPmsk;
    ret_t retVal;

    retVal = rtl8367c_getAsicRegBits(RTL8367C_RLDP_CTRL0_REG,RTL8367C_RLDP_8051_LOOP_PORTMSK_MASK,&tmpPmsk);
    if(retVal != RT_ERR_OK)
        return retVal;
    *pPortmask = tmpPmsk & 0xff;

    retVal = rtl8367c_getAsicRegBits(RTL8367C_REG_RLDP_CTRL5,RTL8367C_RLDP_CTRL5_MASK,&tmpPmsk);
    if(retVal != RT_ERR_OK)
        return retVal;
    *pPortmask |= (tmpPmsk & 7) <<8;

    return RT_ERR_OK;
}


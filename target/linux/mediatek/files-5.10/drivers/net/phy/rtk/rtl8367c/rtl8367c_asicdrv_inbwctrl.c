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
 * Feature : Ingress bandwidth control related functions
 *
 */
#include <rtl8367c_asicdrv_inbwctrl.h>
/* Function Name:
 *      rtl8367c_setAsicPortIngressBandwidth
 * Description:
 *      Set per-port total ingress bandwidth
 * Input:
 *      port        - Physical port number (0~7)
 *      bandwidth   - The total ingress bandwidth (unit: 8Kbps), 0x1FFFF:disable
 *      preifg      - Include preamble and IFG, 0:Exclude, 1:Include
 *      enableFC    - Action when input rate exceeds. 0: Drop   1: Flow Control
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
ret_t rtl8367c_setAsicPortIngressBandwidth(rtk_uint32 port, rtk_uint32 bandwidth, rtk_uint32 preifg, rtk_uint32 enableFC)
{
    ret_t retVal;
    rtk_uint32 regData;
    rtk_uint32 regAddr;

    /* Invalid input parameter */
    if(port >= RTL8367C_PORTNO)
        return RT_ERR_PORT_ID;

    if(bandwidth > RTL8367C_QOS_GRANULARTY_MAX)
        return RT_ERR_OUT_OF_RANGE;

    regAddr = RTL8367C_INGRESSBW_PORT_RATE_LSB_REG(port);
    regData = bandwidth & RTL8367C_QOS_GRANULARTY_LSB_MASK;
    retVal = rtl8367c_setAsicReg(regAddr, regData);
    if(retVal != RT_ERR_OK)
        return retVal;

    regAddr += 1;
    regData = (bandwidth & RTL8367C_QOS_GRANULARTY_MSB_MASK) >> RTL8367C_QOS_GRANULARTY_MSB_OFFSET;
    retVal = rtl8367c_setAsicRegBits(regAddr, RTL8367C_INGRESSBW_PORT0_RATE_CTRL1_INGRESSBW_RATE16_MASK, regData);
    if(retVal != RT_ERR_OK)
        return retVal;

    regAddr = RTL8367C_PORT_MISC_CFG_REG(port);
    retVal = rtl8367c_setAsicRegBit(regAddr, RTL8367C_PORT0_MISC_CFG_INGRESSBW_IFG_OFFSET, preifg);
    if(retVal != RT_ERR_OK)
        return retVal;

    regAddr = RTL8367C_PORT_MISC_CFG_REG(port);
    retVal = rtl8367c_setAsicRegBit(regAddr, RTL8367C_PORT0_MISC_CFG_INGRESSBW_FLOWCTRL_OFFSET, enableFC);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}
/* Function Name:
 *      rtl8367c_getAsicPortIngressBandwidth
 * Description:
 *      Get per-port total ingress bandwidth
 * Input:
 *      port        - Physical port number (0~7)
 *      pBandwidth  - The total ingress bandwidth (unit: 8Kbps), 0x1FFFF:disable
 *      pPreifg         - Include preamble and IFG, 0:Exclude, 1:Include
 *      pEnableFC   - Action when input rate exceeds. 0: Drop   1: Flow Control
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicPortIngressBandwidth(rtk_uint32 port, rtk_uint32* pBandwidth, rtk_uint32* pPreifg, rtk_uint32* pEnableFC)
{
    ret_t retVal;
    rtk_uint32 regData;
    rtk_uint32 regAddr;

    /* Invalid input parameter */
    if(port >= RTL8367C_PORTNO)
        return RT_ERR_PORT_ID;

    regAddr = RTL8367C_INGRESSBW_PORT_RATE_LSB_REG(port);
    retVal = rtl8367c_getAsicReg(regAddr, &regData);
    if(retVal != RT_ERR_OK)
        return retVal;

    *pBandwidth = regData;

    regAddr += 1;
    retVal = rtl8367c_getAsicRegBits(regAddr, RTL8367C_INGRESSBW_PORT0_RATE_CTRL1_INGRESSBW_RATE16_MASK, &regData);
    if(retVal != RT_ERR_OK)
        return retVal;

    *pBandwidth |= (regData << RTL8367C_QOS_GRANULARTY_MSB_OFFSET);

    regAddr = RTL8367C_PORT_MISC_CFG_REG(port);
    retVal = rtl8367c_getAsicRegBit(regAddr, RTL8367C_PORT0_MISC_CFG_INGRESSBW_IFG_OFFSET, pPreifg);
    if(retVal != RT_ERR_OK)
        return retVal;

    regAddr = RTL8367C_PORT_MISC_CFG_REG(port);
    retVal = rtl8367c_getAsicRegBit(regAddr, RTL8367C_PORT0_MISC_CFG_INGRESSBW_FLOWCTRL_OFFSET, pEnableFC);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}
/* Function Name:
 *      rtl8367c_setAsicPortIngressBandwidthBypass
 * Description:
 *      Set ingress bandwidth control bypasss 8899, RMA 01-80-C2-00-00-xx and IGMP
 * Input:
 *      enabled - 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicPortIngressBandwidthBypass(rtk_uint32 enabled)
{
    return rtl8367c_setAsicRegBit(RTL8367C_REG_SW_DUMMY0, RTL8367C_INGRESSBW_BYPASS_EN_OFFSET, enabled);
}
/* Function Name:
 *      rtl8367c_getAsicPortIngressBandwidthBypass
 * Description:
 *      Set ingress bandwidth control bypasss 8899, RMA 01-80-C2-00-00-xx and IGMP
 * Input:
 *      pEnabled - 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicPortIngressBandwidthBypass(rtk_uint32* pEnabled)
{
    return rtl8367c_getAsicRegBit(RTL8367C_REG_SW_DUMMY0, RTL8367C_INGRESSBW_BYPASS_EN_OFFSET, pEnabled);
}


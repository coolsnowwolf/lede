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
 * Feature : Proprietary CPU-tag related function drivers
 *
 */
#include <rtl8367c_asicdrv_cputag.h>
/* Function Name:
 *      rtl8367c_setAsicCputagEnable
 * Description:
 *      Set CPU tag function enable/disable
 * Input:
 *      enabled - 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_ENABLE   - Invalid enable/disable input
 * Note:
 *      If CPU tag function is disabled, CPU tag will not be added to frame
 *      forwarded to CPU port, and all ports cannot parse CPU tag.
 */
ret_t rtl8367c_setAsicCputagEnable(rtk_uint32 enabled)
{
    if(enabled > 1)
        return RT_ERR_ENABLE;

    return rtl8367c_setAsicRegBit(RTL8367C_REG_CPU_CTRL, RTL8367C_CPU_EN_OFFSET, enabled);
}
/* Function Name:
 *      rtl8367c_getAsicCputagEnable
 * Description:
 *      Get CPU tag function enable/disable
 * Input:
 *      pEnabled - 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicCputagEnable(rtk_uint32 *pEnabled)
{
    return rtl8367c_getAsicRegBit(RTL8367C_REG_CPU_CTRL, RTL8367C_CPU_EN_OFFSET, pEnabled);
}
/* Function Name:
 *      rtl8367c_setAsicCputagTrapPort
 * Description:
 *      Set CPU tag trap port
 * Input:
 *      port - port number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *     API can set destination port of trapping frame
 */
ret_t rtl8367c_setAsicCputagTrapPort(rtk_uint32 port)
{
    ret_t retVal;

    if(port >= RTL8367C_PORTNO)
        return RT_ERR_PORT_ID;

    retVal = rtl8367c_setAsicRegBits(RTL8367C_REG_CPU_CTRL, RTL8367C_CPU_TRAP_PORT_MASK, port & 7);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8367c_setAsicRegBits(RTL8367C_REG_CPU_CTRL, RTL8367C_CPU_TRAP_PORT_EXT_MASK, (port>>3) & 1);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}
/* Function Name:
 *      rtl8367c_getAsicCputagTrapPort
 * Description:
 *      Get CPU tag trap port
 * Input:
 *      pPort - port number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *     None
 */
ret_t rtl8367c_getAsicCputagTrapPort(rtk_uint32 *pPort)
{
    ret_t retVal;
    rtk_uint32 tmpPort;

    retVal = rtl8367c_getAsicRegBits(RTL8367C_REG_CPU_CTRL, RTL8367C_CPU_TRAP_PORT_MASK, &tmpPort);
    if(retVal != RT_ERR_OK)
        return retVal;
    *pPort = tmpPort;

    retVal = rtl8367c_getAsicRegBits(RTL8367C_REG_CPU_CTRL, RTL8367C_CPU_TRAP_PORT_EXT_MASK, &tmpPort);
    if(retVal != RT_ERR_OK)
        return retVal;
    *pPort |= (tmpPort & 1) << 3;

    return RT_ERR_OK;
}
/* Function Name:
 *      rtl8367c_setAsicCputagPortmask
 * Description:
 *      Set ports that can parse CPU tag
 * Input:
 *      portmask - port mask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_MASK    - Invalid portmask
 * Note:
 *     None
 */
ret_t rtl8367c_setAsicCputagPortmask(rtk_uint32 portmask)
{
    if(portmask > RTL8367C_PORTMASK)
        return RT_ERR_PORT_MASK;

    return rtl8367c_setAsicReg(RTL8367C_CPU_PORT_MASK_REG, portmask);
}
/* Function Name:
 *      rtl8367c_getAsicCputagPortmask
 * Description:
 *      Get ports that can parse CPU tag
 * Input:
 *      pPortmask - port mask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *     None
 */
ret_t rtl8367c_getAsicCputagPortmask(rtk_uint32 *pPortmask)
{
    return rtl8367c_getAsicReg(RTL8367C_CPU_PORT_MASK_REG, pPortmask);
}
/* Function Name:
 *      rtl8367c_setAsicCputagInsertMode
 * Description:
 *      Set CPU-tag insert mode
 * Input:
 *      mode - 0: insert to all packets; 1: insert to trapped packets; 2: don't insert
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_NOT_ALLOWED  - Actions not allowed by the function
 * Note:
 *     None
 */
ret_t rtl8367c_setAsicCputagInsertMode(rtk_uint32 mode)
{
    if(mode >= CPUTAG_INSERT_END)
        return RT_ERR_NOT_ALLOWED;

    return rtl8367c_setAsicRegBits(RTL8367C_REG_CPU_CTRL, RTL8367C_CPU_INSERTMODE_MASK, mode);
}
/* Function Name:
 *      rtl8367c_getAsicCputagInsertMode
 * Description:
 *      Get CPU-tag insert mode
 * Input:
 *      pMode - 0: insert to all packets; 1: insert to trapped packets; 2: don't insert
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *     None
 */
ret_t rtl8367c_getAsicCputagInsertMode(rtk_uint32 *pMode)
{
    return rtl8367c_getAsicRegBits(RTL8367C_REG_CPU_CTRL, RTL8367C_CPU_INSERTMODE_MASK, pMode);
}
/* Function Name:
 *      rtl8367c_setAsicCputagPriorityRemapping
 * Description:
 *      Set queue assignment of CPU port
 * Input:
 *      srcPri - internal priority (0~7)
 *      newPri - internal priority after remapping (0~7)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                   - Success
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_QOS_INT_PRIORITY     - Invalid priority
 * Note:
 *     None
 */
ret_t rtl8367c_setAsicCputagPriorityRemapping(rtk_uint32 srcPri, rtk_uint32 newPri)
{
    if((srcPri > RTL8367C_PRIMAX) || (newPri > RTL8367C_PRIMAX))
        return RT_ERR_QOS_INT_PRIORITY;

    return rtl8367c_setAsicRegBits(RTL8367C_QOS_PRIPORITY_REMAPPING_IN_CPU_REG(srcPri), RTL8367C_QOS_PRIPORITY_REMAPPING_IN_CPU_MASK(srcPri), newPri);
}
/* Function Name:
 *      rtl8367c_getAsicCputagPriorityRemapping
 * Description:
 *      Get queue assignment of CPU port
 * Input:
 *      srcPri - internal priority (0~7)
 *      pNewPri - internal priority after remapping (0~7)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                   - Success
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_QOS_INT_PRIORITY     - Invalid priority
 * Note:
 *     None
 */
ret_t rtl8367c_getAsicCputagPriorityRemapping(rtk_uint32 srcPri, rtk_uint32 *pNewPri)
{
    if(srcPri > RTL8367C_PRIMAX)
        return RT_ERR_QOS_INT_PRIORITY;

    return rtl8367c_getAsicRegBits(RTL8367C_QOS_PRIPORITY_REMAPPING_IN_CPU_REG(srcPri), RTL8367C_QOS_PRIPORITY_REMAPPING_IN_CPU_MASK(srcPri), pNewPri);
}
/* Function Name:
 *      rtl8367c_setAsicCputagPosition
 * Description:
 *      Set CPU tag insert position
 * Input:
 *      position - 1: After entire packet(before CRC field), 0: After MAC_SA (Default)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                   - Success
 *      RT_ERR_SMI                  - SMI access error
 * Note:
 *     None
 */
ret_t rtl8367c_setAsicCputagPosition(rtk_uint32 position)
{
    return rtl8367c_setAsicRegBit(RTL8367C_REG_CPU_CTRL, RTL8367C_CPU_TAG_POSITION_OFFSET, position);
}
/* Function Name:
 *      rtl8367c_getAsicCputagPosition
 * Description:
 *      Get CPU tag insert position
 * Input:
 *      pPostion - 1: After entire packet(before CRC field), 0: After MAC_SA (Default)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                   - Success
 *      RT_ERR_SMI                  - SMI access error
 * Note:
 *     None
 */
ret_t rtl8367c_getAsicCputagPosition(rtk_uint32* pPostion)
{
    return rtl8367c_getAsicRegBit(RTL8367C_REG_CPU_CTRL, RTL8367C_CPU_TAG_POSITION_OFFSET, pPostion);
}

/* Function Name:
 *      rtl8367c_setAsicCputagMode
 * Description:
 *      Set CPU tag mode
 * Input:
 *      mode - 1: 4bytes mode, 0: 8bytes mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_INPUT    - Invalid input parameters
 * Note:
 *      If CPU tag function is disabled, CPU tag will not be added to frame
 *      forwarded to CPU port, and all ports cannot parse CPU tag.
 */
ret_t rtl8367c_setAsicCputagMode(rtk_uint32 mode)
{
    if(mode > 1)
        return RT_ERR_INPUT;

    return rtl8367c_setAsicRegBit(RTL8367C_REG_CPU_CTRL, RTL8367C_CPU_TAG_FORMAT_OFFSET, mode);
}
/* Function Name:
 *      rtl8367c_getAsicCputagMode
 * Description:
 *      Get CPU tag mode
 * Input:
 *      pMode - 1: 4bytes mode, 0: 8bytes mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicCputagMode(rtk_uint32 *pMode)
{
    return rtl8367c_getAsicRegBit(RTL8367C_REG_CPU_CTRL, RTL8367C_CPU_TAG_FORMAT_OFFSET, pMode);
}
/* Function Name:
 *      rtl8367c_setAsicCputagRxMinLength
 * Description:
 *      Set CPU tag mode
 * Input:
 *      mode - 1: 64bytes, 0: 72bytes
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_INPUT    - Invalid input parameters
 * Note:
 *      If CPU tag function is disabled, CPU tag will not be added to frame
 *      forwarded to CPU port, and all ports cannot parse CPU tag.
 */
ret_t rtl8367c_setAsicCputagRxMinLength(rtk_uint32 mode)
{
    if(mode > 1)
        return RT_ERR_INPUT;

    return rtl8367c_setAsicRegBit(RTL8367C_REG_CPU_CTRL, RTL8367C_CPU_TAG_RXBYTECOUNT_OFFSET, mode);
}
/* Function Name:
 *      rtl8367c_getAsicCputagRxMinLength
 * Description:
 *      Get CPU tag mode
 * Input:
 *      pMode - 1: 64bytes, 0: 72bytes
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicCputagRxMinLength(rtk_uint32 *pMode)
{
    return rtl8367c_getAsicRegBit(RTL8367C_REG_CPU_CTRL, RTL8367C_CPU_TAG_RXBYTECOUNT_OFFSET, pMode);
}




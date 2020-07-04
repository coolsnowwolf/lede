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
 * Feature : Interrupt related functions
 *
 */
#include <rtl8367c_asicdrv_interrupt.h>
/* Function Name:
 *      rtl8367c_setAsicInterruptPolarity
 * Description:
 *      Set interrupt trigger polarity
 * Input:
 *      polarity    - 0:pull high 1: pull low
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicInterruptPolarity(rtk_uint32 polarity)
{
    return rtl8367c_setAsicRegBit(RTL8367C_REG_INTR_CTRL, RTL8367C_INTR_CTRL_OFFSET, polarity);
}
/* Function Name:
 *      rtl8367c_getAsicInterruptPolarity
 * Description:
 *      Get interrupt trigger polarity
 * Input:
 *      pPolarity   - 0:pull high 1: pull low
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicInterruptPolarity(rtk_uint32* pPolarity)
{
    return rtl8367c_getAsicRegBit(RTL8367C_REG_INTR_CTRL, RTL8367C_INTR_CTRL_OFFSET, pPolarity);
}
/* Function Name:
 *      rtl8367c_setAsicInterruptMask
 * Description:
 *      Set interrupt enable mask
 * Input:
 *      imr     - Interrupt mask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicInterruptMask(rtk_uint32 imr)
{
    return rtl8367c_setAsicReg(RTL8367C_REG_INTR_IMR, imr);
}
/* Function Name:
 *      rtl8367c_getAsicInterruptMask
 * Description:
 *      Get interrupt enable mask
 * Input:
 *      pImr    - Interrupt mask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicInterruptMask(rtk_uint32* pImr)
{
    return rtl8367c_getAsicReg(RTL8367C_REG_INTR_IMR, pImr);
}
/* Function Name:
 *      rtl8367c_setAsicInterruptMask
 * Description:
 *      Clear interrupt enable mask
 * Input:
 *      ims     - Interrupt status mask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      This API can be used to clear ASIC interrupt status and register will be cleared by writting 1.
 *      [0]:Link change,
 *      [1]:Share meter exceed,
 *      [2]:Learn number overed,
 *      [3]:Speed Change,
 *      [4]:Tx special congestion
 *      [5]:1 second green feature
 *      [6]:loop detection
 *      [7]:interrupt from 8051
 *      [8]:Cable diagnostic finish
 *      [9]:ACL action interrupt trigger
 *      [11]: Silent Start
 */
ret_t rtl8367c_setAsicInterruptStatus(rtk_uint32 ims)
{
    return rtl8367c_setAsicReg(RTL8367C_REG_INTR_IMS, ims);
}
/* Function Name:
 *      rtl8367c_getAsicInterruptStatus
 * Description:
 *      Get interrupt enable mask
 * Input:
 *      pIms    - Interrupt status mask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicInterruptStatus(rtk_uint32* pIms)
{
    return rtl8367c_getAsicReg(RTL8367C_REG_INTR_IMS, pIms);
}
/* Function Name:
 *      rtl8367c_setAsicInterruptRelatedStatus
 * Description:
 *      Clear interrupt status
 * Input:
 *      type    - per port Learn over, per-port speed change, per-port special congest, share meter exceed status
 *      status  - exceed status, write 1 to clear
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicInterruptRelatedStatus(rtk_uint32 type, rtk_uint32 status)
{
    CONST rtk_uint32 indicatorAddress[INTRST_END] = {RTL8367C_REG_LEARN_OVER_INDICATOR,
                                                    RTL8367C_REG_SPEED_CHANGE_INDICATOR,
                                                    RTL8367C_REG_SPECIAL_CONGEST_INDICATOR,
                                                    RTL8367C_REG_PORT_LINKDOWN_INDICATOR,
                                                    RTL8367C_REG_PORT_LINKUP_INDICATOR,
                                                    RTL8367C_REG_METER_OVERRATE_INDICATOR0,
                                                    RTL8367C_REG_METER_OVERRATE_INDICATOR1,
                                                    RTL8367C_REG_RLDP_LOOPED_INDICATOR,
                                                    RTL8367C_REG_RLDP_RELEASED_INDICATOR,
                                                    RTL8367C_REG_SYSTEM_LEARN_OVER_INDICATOR};

    if(type >= INTRST_END )
        return RT_ERR_OUT_OF_RANGE;

    return rtl8367c_setAsicReg(indicatorAddress[type], status);
}
/* Function Name:
 *      rtl8367c_getAsicInterruptRelatedStatus
 * Description:
 *      Get interrupt status
 * Input:
 *      type    - per port Learn over, per-port speed change, per-port special congest, share meter exceed status
 *      pStatus     - exceed status, write 1 to clear
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicInterruptRelatedStatus(rtk_uint32 type, rtk_uint32* pStatus)
{
    CONST rtk_uint32 indicatorAddress[INTRST_END] = {RTL8367C_REG_LEARN_OVER_INDICATOR,
                                                    RTL8367C_REG_SPEED_CHANGE_INDICATOR,
                                                    RTL8367C_REG_SPECIAL_CONGEST_INDICATOR,
                                                    RTL8367C_REG_PORT_LINKDOWN_INDICATOR,
                                                    RTL8367C_REG_PORT_LINKUP_INDICATOR,
                                                    RTL8367C_REG_METER_OVERRATE_INDICATOR0,
                                                    RTL8367C_REG_METER_OVERRATE_INDICATOR1,
                                                    RTL8367C_REG_RLDP_LOOPED_INDICATOR,
                                                    RTL8367C_REG_RLDP_RELEASED_INDICATOR,
                                                    RTL8367C_REG_SYSTEM_LEARN_OVER_INDICATOR};

    if(type >= INTRST_END )
        return RT_ERR_OUT_OF_RANGE;

    return rtl8367c_getAsicReg(indicatorAddress[type], pStatus);
}


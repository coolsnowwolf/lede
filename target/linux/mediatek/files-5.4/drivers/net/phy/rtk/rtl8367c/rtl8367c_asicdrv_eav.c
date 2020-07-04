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
 * Feature : Ethernet AV related functions
 *
 */

#include <rtl8367c_asicdrv_eav.h>
/* Function Name:
 *      rtl8367c_setAsicEavMacAddress
 * Description:
 *      Set PTP MAC address
 * Input:
 *      mac     - PTP mac
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicEavMacAddress(ether_addr_t mac)
{
    ret_t retVal;
    rtk_uint32 regData;
    rtk_uint8 *accessPtr;
    rtk_uint32 i;

    accessPtr =  (rtk_uint8*)&mac;

    regData = *accessPtr;
    accessPtr ++;
    regData = (regData << 8) | *accessPtr;
    accessPtr ++;
    for(i = 0; i <=2; i++)
    {
        retVal = rtl8367c_setAsicReg(RTL8367C_REG_MAC_ADDR_H - i, regData);
        if(retVal != RT_ERR_OK)
            return retVal;

        regData = *accessPtr;
        accessPtr ++;
        regData = (regData << 8) | *accessPtr;
        accessPtr ++;
    }

    return retVal;
}
/* Function Name:
 *      rtl8367c_getAsicEavMacAddress
 * Description:
 *      Get PTP MAC address
 * Input:
 *      None
 * Output:
 *      pMac     - PTP  mac
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicEavMacAddress(ether_addr_t *pMac)
{
    ret_t retVal;
    rtk_uint32 regData;
    rtk_uint8 *accessPtr;
    rtk_uint32 i;

    accessPtr = (rtk_uint8*)pMac;

    for(i = 0; i <= 2; i++)
    {
        retVal = rtl8367c_getAsicReg(RTL8367C_REG_MAC_ADDR_H - i, &regData);
        if(retVal != RT_ERR_OK)
            return retVal;

        *accessPtr = (regData & 0xFF00) >> 8;
        accessPtr ++;
        *accessPtr = regData & 0xFF;
        accessPtr ++;
    }

    return retVal;
}

/* Function Name:
 *      rtl8367c_setAsicEavTpid
 * Description:
 *      Set PTP parser tag TPID.
 * Input:
 *       outerTag - outter tag TPID
 *       innerTag  - inner tag TPID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *     None
 */
ret_t rtl8367c_setAsicEavTpid(rtk_uint32 outerTag, rtk_uint32 innerTag)
{
    ret_t retVal;

    if((retVal = rtl8367c_setAsicReg(RTL8367C_REG_OTAG_TPID, outerTag)) != RT_ERR_OK)
        return retVal;
    if((retVal = rtl8367c_setAsicReg(RTL8367C_REG_ITAG_TPID, innerTag)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}
/* Function Name:
 *      rtl8367c_getAsicEavTpid
 * Description:
 *      Get PTP parser tag TPID.
 * Input:
 *      None
 * Output:
 *       pOuterTag - outter tag TPID
 *       pInnerTag  - inner tag TPID
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicEavTpid(rtk_uint32* pOuterTag, rtk_uint32* pInnerTag)
{
    ret_t retVal;

    if((retVal = rtl8367c_getAsicReg(RTL8367C_REG_OTAG_TPID, pOuterTag)) != RT_ERR_OK)
        return retVal;
    if((retVal = rtl8367c_getAsicReg(RTL8367C_REG_ITAG_TPID, pInnerTag)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367c_setAsicEavSysTime
 * Description:
 *      Set PTP system time
 * Input:
 *      second - seconds
 *      nanoSecond - nano seconds
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      The time granuality is 8 nano seconds.
 */
ret_t rtl8367c_setAsicEavSysTime(rtk_uint32 second, rtk_uint32 nanoSecond)
{
    ret_t retVal;
    rtk_uint32 sec_h, sec_l, nsec8_h, nsec8_l;
    rtk_uint32 nano_second_8;
    rtk_uint32 regData, busyFlag, count;

    if(nanoSecond > RTL8367C_EAV_NANOSECONDMAX)
        return RT_ERR_INPUT;

    regData = 0;
    sec_h = second >>16;
    sec_l = second & 0xFFFF;
    nano_second_8 = nanoSecond >> 3;
    nsec8_h = (nano_second_8 >>16) & RTL8367C_PTP_TIME_NSEC_H_NSEC_MASK;
    nsec8_l = nano_second_8 &0xFFFF;

    if((retVal = rtl8367c_setAsicReg(RTL8367C_REG_PTP_TIME_SEC_H_SEC, sec_h)) != RT_ERR_OK)
        return retVal;
    if((retVal = rtl8367c_setAsicReg(RTL8367C_REG_PTP_TIME_SEC_L_SEC, sec_l)) != RT_ERR_OK)
        return retVal;
    if((retVal = rtl8367c_setAsicReg(RTL8367C_REG_PTP_TIME_NSEC_L_NSEC, nsec8_l)) != RT_ERR_OK)
        return retVal;

    regData = nsec8_h | (PTP_TIME_WRITE<<RTL8367C_PTP_TIME_NSEC_H_CMD_OFFSET) | RTL8367C_PTP_TIME_NSEC_H_EXEC_MASK;

    if((retVal = rtl8367c_setAsicReg(RTL8367C_REG_PTP_TIME_NSEC_H_NSEC, regData)) != RT_ERR_OK)
        return retVal;

    count = 0;
    do {
        if((retVal = rtl8367c_getAsicRegBit(RTL8367C_REG_PTP_TIME_NSEC_H_NSEC, RTL8367C_PTP_TIME_NSEC_H_EXEC_OFFSET, &busyFlag)) != RT_ERR_OK)
            return retVal;
        count++;
    } while ((busyFlag != 0)&&(count<5));

    if (busyFlag != 0)
        return RT_ERR_BUSYWAIT_TIMEOUT;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367c_getAsicEavSysTime
 * Description:
 *      Get PTP system time
 * Input:
 *      None
 * Output:
 *      second - seconds
 *      nanoSecond - nano seconds
 * Return:
 *      RT_ERR_OK     - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      The time granuality is 8 nano seconds.
 */
ret_t rtl8367c_getAsicEavSysTime(rtk_uint32* pSecond, rtk_uint32* pNanoSecond)
{
    ret_t retVal;
    rtk_uint32 sec_h, sec_l, nsec8_h, nsec8_l;
    rtk_uint32 nano_second_8;
    rtk_uint32 regData, busyFlag, count;

    regData = 0;
    regData = (PTP_TIME_READ<<RTL8367C_PTP_TIME_NSEC_H_CMD_OFFSET) | RTL8367C_PTP_TIME_NSEC_H_EXEC_MASK;

    if((retVal = rtl8367c_setAsicReg(RTL8367C_REG_PTP_TIME_NSEC_H_NSEC, regData)) != RT_ERR_OK)
        return retVal;

    count = 0;
    do {
        if((retVal = rtl8367c_getAsicRegBit(RTL8367C_REG_PTP_TIME_NSEC_H_NSEC, RTL8367C_PTP_TIME_NSEC_H_EXEC_OFFSET, &busyFlag)) != RT_ERR_OK)
            return retVal;
        count++;
    } while ((busyFlag != 0)&&(count<5));

    if (busyFlag != 0)
        return RT_ERR_BUSYWAIT_TIMEOUT;

    if((retVal = rtl8367c_getAsicReg(RTL8367C_REG_PTP_TIME_SEC_H_SEC_RD, &sec_h)) != RT_ERR_OK)
        return retVal;
    if((retVal = rtl8367c_getAsicReg(RTL8367C_REG_PTP_TIME_SEC_L_SEC_RD, &sec_l)) != RT_ERR_OK)
        return retVal;
    if((retVal = rtl8367c_getAsicRegBits(RTL8367C_REG_PTP_TIME_NSEC_H_NSEC_RD, RTL8367C_PTP_TIME_NSEC_H_NSEC_RD_MASK,&nsec8_h)) != RT_ERR_OK)
        return retVal;
    if((retVal = rtl8367c_getAsicReg(RTL8367C_REG_PTP_TIME_NSEC_L_NSEC_RD, &nsec8_l)) != RT_ERR_OK)
        return retVal;

    *pSecond = (sec_h<<16) | sec_l;
    nano_second_8 = (nsec8_h<<16) | nsec8_l;
    *pNanoSecond = nano_second_8<<3;

    return RT_ERR_OK;
}


/* Function Name:
 *      rtl8367c_setAsicEavSysTimeAdjust
 * Description:
 *      Set PTP system time adjust
 * Input:
 *      type - incresae or decrease
 *      second - seconds
 *      nanoSecond - nano seconds
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      Ethernet AV second offset of timer for tuning
 */
ret_t rtl8367c_setAsicEavSysTimeAdjust(rtk_uint32 type, rtk_uint32 second, rtk_uint32 nanoSecond)
{
    ret_t retVal;
    rtk_uint32 sec_h, sec_l, nsec8_h, nsec8_l;
    rtk_uint32 nano_second_8;
    rtk_uint32 regData, busyFlag, count;

    if (type >= PTP_TIME_ADJ_END)
        return RT_ERR_INPUT;
    if(nanoSecond > RTL8367C_EAV_NANOSECONDMAX)
        return RT_ERR_INPUT;

    regData = 0;
    sec_h = second >>16;
    sec_l = second & 0xFFFF;
    nano_second_8 = nanoSecond >> 3;
    nsec8_h = (nano_second_8 >>16) & RTL8367C_PTP_TIME_NSEC_H_NSEC_MASK;
    nsec8_l = nano_second_8 &0xFFFF;

    if((retVal = rtl8367c_setAsicReg(RTL8367C_REG_PTP_TIME_SEC_H_SEC, sec_h)) != RT_ERR_OK)
        return retVal;
    if((retVal = rtl8367c_setAsicReg(RTL8367C_REG_PTP_TIME_SEC_L_SEC, sec_l)) != RT_ERR_OK)
        return retVal;
    if((retVal = rtl8367c_setAsicReg(RTL8367C_REG_PTP_TIME_NSEC_L_NSEC, nsec8_l)) != RT_ERR_OK)
        return retVal;

    if (PTP_TIME_ADJ_INC == type)
        regData = nsec8_h | (PTP_TIME_INC<<RTL8367C_PTP_TIME_NSEC_H_CMD_OFFSET) | RTL8367C_PTP_TIME_NSEC_H_EXEC_MASK;
    else
        regData = nsec8_h | (PTP_TIME_DEC<<RTL8367C_PTP_TIME_NSEC_H_CMD_OFFSET) | RTL8367C_PTP_TIME_NSEC_H_EXEC_MASK;

    if((retVal = rtl8367c_setAsicReg(RTL8367C_REG_PTP_TIME_NSEC_H_NSEC, regData)) != RT_ERR_OK)
        return retVal;

    count = 0;
    do {
        if((retVal = rtl8367c_getAsicRegBit(RTL8367C_REG_PTP_TIME_NSEC_H_NSEC, RTL8367C_PTP_TIME_NSEC_H_EXEC_OFFSET, &busyFlag)) != RT_ERR_OK)
            return retVal;
        count++;
    } while ((busyFlag != 0)&&(count<5));

    if (busyFlag != 0)
        return RT_ERR_BUSYWAIT_TIMEOUT;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367c_setAsicEavSysTimeCtrl
 * Description:
 *      Set PTP system time control
 * Input:
 *      command - start or stop
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicEavSysTimeCtrl(rtk_uint32 control)
{
    ret_t  retVal;
    rtk_uint32 regData;

    if (control>=PTP_TIME_CTRL_END)
         return RT_ERR_INPUT;

    regData = 0;
    if (PTP_TIME_CTRL_START == control)
            regData = RTL8367C_CFG_TIMER_EN_FRC_MASK | RTL8367C_CFG_TIMER_1588_EN_MASK;
    else
        regData = 0;

    if((retVal = rtl8367c_setAsicReg(RTL8367C_REG_PTP_TIME_CFG, regData)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367c_getAsicEavSysTimeCtrl
 * Description:
 *      Get PTP system time control
 * Input:
 *      None
 * Output:
 *      pControl - start or stop
 * Return:
 *      RT_ERR_OK     - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicEavSysTimeCtrl(rtk_uint32* pControl)
{
    ret_t  retVal;
    rtk_uint32 regData;
    rtk_uint32 mask;

    mask = RTL8367C_CFG_TIMER_EN_FRC_MASK | RTL8367C_CFG_TIMER_1588_EN_MASK;

    if((retVal = rtl8367c_getAsicReg(RTL8367C_REG_PTP_TIME_CFG, &regData)) != RT_ERR_OK)
        return retVal;

    if( (regData & mask) == mask)
        *pControl = PTP_TIME_CTRL_START;
    else if( (regData & mask) == 0)
        *pControl = PTP_TIME_CTRL_STOP;
    else
        return RT_ERR_NOT_ALLOWED;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367c_setAsicEavInterruptMask
 * Description:
 *      Set PTP interrupt enable mask
 * Input:
 *      imr     - Interrupt mask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      [0]:TX_SYNC,
 *      [1]:TX_DELAY,
 *      [2]:TX_PDELAY_REQ,
 *      [3]:TX_PDELAY_RESP,
 *      [4]:RX_SYNC,
 *      [5]:RX_DELAY,
 *      [6]:RX_PDELAY_REQ,
 *      [7]:RX_PDELAY_RESP,
 */
ret_t rtl8367c_setAsicEavInterruptMask(rtk_uint32 imr)
{
    if ((imr&(RTL8367C_PTP_INTR_MASK<<8))>0)
         return RT_ERR_INPUT;

    return rtl8367c_setAsicRegBits(RTL8367C_REG_PTP_TIME_CFG2, RTL8367C_PTP_INTR_MASK, imr);
}
/* Function Name:
 *      rtl8367c_getAsicEavInterruptMask
 * Description:
 *      Get PTP interrupt enable mask
 * Input:
 *      pImr    - Interrupt mask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      [0]:TX_SYNC,
 *      [1]:TX_DELAY,
 *      [2]:TX_PDELAY_REQ,
 *      [3]:TX_PDELAY_RESP,
 *      [4]:RX_SYNC,
 *      [5]:RX_DELAY,
 *      [6]:RX_PDELAY_REQ,
 *      [7]:RX_PDELAY_RESP,
 */
ret_t rtl8367c_getAsicEavInterruptMask(rtk_uint32* pImr)
{
    return rtl8367c_getAsicRegBits(RTL8367C_REG_PTP_TIME_CFG2, RTL8367C_PTP_INTR_MASK, pImr);
}

/* Function Name:
 *      rtl8367c_getAsicEavInterruptStatus
 * Description:
 *      Get PTP interrupt port status mask
 * Input:
 *      pIms    - Interrupt mask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      [0]:p0 interrupt,
 *      [1]:p1 interrupt,
 *      [2]:p2 interrupt,
 *      [3]:p3 interrupt,
 *      [4]:p4 interrupt,
 */
ret_t rtl8367c_getAsicEavInterruptStatus(rtk_uint32* pIms)
{
    return rtl8367c_getAsicRegBits(RTL8367C_REG_PTP_INTERRUPT_CFG, RTL8367C_PTP_PORT_MASK, pIms);
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
 *      [0]:TX_SYNC,
 *      [1]:TX_DELAY,
 *      [2]:TX_PDELAY_REQ,
 *      [3]:TX_PDELAY_RESP,
 *      [4]:RX_SYNC,
 *      [5]:RX_DELAY,
 *      [6]:RX_PDELAY_REQ,
 *      [7]:RX_PDELAY_RESP,
 */
ret_t rtl8367c_setAsicEavPortInterruptStatus(rtk_uint32 port, rtk_uint32 ims)
{

    if(port > RTL8367C_PORTNO)
        return RT_ERR_PORT_ID;

    if(port < 5)
        return rtl8367c_setAsicRegBits(RTL8367C_EAV_PORT_CFG_REG(port), RTL8367C_PTP_INTR_MASK,ims);
    else if(port == 5)
        return rtl8367c_setAsicRegBits(RTL8367C_REG_P5_EAV_CFG, RTL8367C_PTP_INTR_MASK,ims);
    else if(port == 6)
        return rtl8367c_setAsicRegBits(RTL8367C_REG_P6_EAV_CFG, RTL8367C_PTP_INTR_MASK,ims);
    else if(port == 7)
        return rtl8367c_setAsicRegBits(RTL8367C_REG_P7_EAV_CFG, RTL8367C_PTP_INTR_MASK,ims);
    else if(port == 8)
        return rtl8367c_setAsicRegBits(RTL8367C_REG_P8_EAV_CFG, RTL8367C_PTP_INTR_MASK,ims);
    else if(port == 9)
        return rtl8367c_setAsicRegBits(RTL8367C_REG_P9_EAV_CFG, RTL8367C_PTP_INTR_MASK,ims);

    return RT_ERR_OK;
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
 *      [0]:TX_SYNC,
 *      [1]:TX_DELAY,
 *      [2]:TX_PDELAY_REQ,
 *      [3]:TX_PDELAY_RESP,
 *      [4]:RX_SYNC,
 *      [5]:RX_DELAY,
 *      [6]:RX_PDELAY_REQ,
 *      [7]:RX_PDELAY_RESP,
 */
ret_t rtl8367c_getAsicEavPortInterruptStatus(rtk_uint32 port, rtk_uint32* pIms)
{

    if(port > RTL8367C_PORTNO)
        return RT_ERR_PORT_ID;
    if(port < 5)
        return rtl8367c_getAsicRegBits(RTL8367C_EAV_PORT_CFG_REG(port), RTL8367C_PTP_INTR_MASK, pIms);
    else if(port == 5)
        return rtl8367c_getAsicRegBits(RTL8367C_REG_P5_EAV_CFG, RTL8367C_PTP_INTR_MASK, pIms);
    else if(port == 6)
        return rtl8367c_getAsicRegBits(RTL8367C_REG_P6_EAV_CFG, RTL8367C_PTP_INTR_MASK,pIms);
    else if(port == 7)
        return rtl8367c_getAsicRegBits(RTL8367C_REG_P7_EAV_CFG, RTL8367C_PTP_INTR_MASK,pIms);
    else if(port == 8)
        return rtl8367c_getAsicRegBits(RTL8367C_REG_P8_EAV_CFG, RTL8367C_PTP_INTR_MASK,pIms);
    else if(port == 9)
        return rtl8367c_getAsicRegBits(RTL8367C_REG_P9_EAV_CFG, RTL8367C_PTP_INTR_MASK,pIms);

    return RT_ERR_OK;

}


/* Function Name:
 *      rtl8367c_setAsicEavPortEnable
 * Description:
 *      Set per-port EAV function enable/disable
 * Input:
 *      port         - Physical port number (0~9)
 *      enabled     - 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      If EAV function is enabled, PTP event messgae packet will be attached PTP timestamp for trapping
 */
ret_t rtl8367c_setAsicEavPortEnable(rtk_uint32 port, rtk_uint32 enabled)
{
    if(port > RTL8367C_PORTNO)
        return RT_ERR_PORT_ID;

    if(port < 5)
        return rtl8367c_setAsicRegBit(RTL8367C_EAV_PORT_CFG_REG(port), RTL8367C_EAV_CFG_PTP_PHY_EN_EN_OFFSET, enabled);
    else if(port == 5)
        return rtl8367c_setAsicRegBit(RTL8367C_REG_P5_EAV_CFG, RTL8367C_EAV_CFG_PTP_PHY_EN_EN_OFFSET, enabled);
    else if(port == 6)
        return rtl8367c_setAsicRegBit(RTL8367C_REG_P6_EAV_CFG, RTL8367C_EAV_CFG_PTP_PHY_EN_EN_OFFSET, enabled);
    else if(port == 7)
        return rtl8367c_setAsicRegBit(RTL8367C_REG_P7_EAV_CFG, RTL8367C_EAV_CFG_PTP_PHY_EN_EN_OFFSET, enabled);
    else if(port == 8)
        return rtl8367c_setAsicRegBit(RTL8367C_REG_P8_EAV_CFG, RTL8367C_EAV_CFG_PTP_PHY_EN_EN_OFFSET, enabled);
    else if(port == 9)
        return rtl8367c_setAsicRegBit(RTL8367C_REG_P9_EAV_CFG, RTL8367C_EAV_CFG_PTP_PHY_EN_EN_OFFSET, enabled);


    return RT_ERR_OK;
}
/* Function Name:
 *      rtl8367c_getAsicEavPortEnable
 * Description:
 *      Get per-port EAV function enable/disable
 * Input:
 *      port         - Physical port number (0~9)
 *      pEnabled     - 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicEavPortEnable(rtk_uint32 port, rtk_uint32 *pEnabled)
{
    if(port > RTL8367C_PORTNO)
        return RT_ERR_PORT_ID;



    if(port < 5)
        return rtl8367c_getAsicRegBit(RTL8367C_EAV_PORT_CFG_REG(port), RTL8367C_EAV_CFG_PTP_PHY_EN_EN_OFFSET, pEnabled);
    else if(port == 5)
        return rtl8367c_getAsicRegBit(RTL8367C_REG_P5_EAV_CFG, RTL8367C_EAV_CFG_PTP_PHY_EN_EN_OFFSET, pEnabled);
    else if(port == 6)
        return rtl8367c_getAsicRegBit(RTL8367C_REG_P6_EAV_CFG, RTL8367C_EAV_CFG_PTP_PHY_EN_EN_OFFSET, pEnabled);
    else if(port == 7)
        return rtl8367c_getAsicRegBit(RTL8367C_REG_P7_EAV_CFG, RTL8367C_EAV_CFG_PTP_PHY_EN_EN_OFFSET, pEnabled);
    else if(port == 8)
        return rtl8367c_getAsicRegBit(RTL8367C_REG_P8_EAV_CFG, RTL8367C_EAV_CFG_PTP_PHY_EN_EN_OFFSET, pEnabled);
    else if(port == 9)
        return rtl8367c_getAsicRegBit(RTL8367C_REG_P9_EAV_CFG, RTL8367C_EAV_CFG_PTP_PHY_EN_EN_OFFSET, pEnabled);


    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367c_getAsicEavPortTimeStamp
 * Description:
 *      Get PTP port time stamp
 * Input:
 *      port         - Physical port number (0~9)
 *      type     -  PTP packet type
 * Output:
 *      timeStamp - seconds
 * Return:
 *      RT_ERR_OK     - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      The time granuality is 8 nano seconds.
 */
ret_t rtl8367c_getAsicEavPortTimeStamp(rtk_uint32 port, rtk_uint32 type, rtl8367c_ptp_time_stamp_t* timeStamp)
{
    ret_t retVal;
    rtk_uint32 sec_h, sec_l, nsec8_h, nsec8_l;
    rtk_uint32 nano_second_8;

    if(port > 9)
        return RT_ERR_PORT_ID;
    if(type >= PTP_PKT_TYPE_END)
        return RT_ERR_INPUT;

    if(port < 5){
        if((retVal = rtl8367c_getAsicReg(RTL8367C_REG_SEQ_ID(port, type), &timeStamp->sequence_id))!=  RT_ERR_OK)
            return retVal;
        if((retVal = rtl8367c_getAsicReg(RTL8367C_REG_PORT_SEC_H(port) , &sec_h)) != RT_ERR_OK)
           return retVal;
        if((retVal = rtl8367c_getAsicReg(RTL8367C_REG_PORT_SEC_L(port), &sec_l)) != RT_ERR_OK)
           return retVal;
        if((retVal = rtl8367c_getAsicRegBits(RTL8367C_REG_PORT_NSEC_H(port) , RTL8367C_PORT_NSEC_H_MASK,&nsec8_h)) != RT_ERR_OK)
           return retVal;
        if((retVal = rtl8367c_getAsicReg(RTL8367C_REG_PORT_NSEC_L(port) , &nsec8_l)) != RT_ERR_OK)
           return retVal;
    }else if(port == 5){
        if((retVal = rtl8367c_getAsicReg(RTL8367C_REG_P5_TX_SYNC_SEQ_ID+type, &timeStamp->sequence_id))!=  RT_ERR_OK)
            return retVal;
        if((retVal = rtl8367c_getAsicReg(RTL8367C_REG_P5_PORT_SEC_31_16, &sec_h)) != RT_ERR_OK)
           return retVal;
        if((retVal = rtl8367c_getAsicReg(RTL8367C_REG_P5_PORT_SEC_15_0, &sec_l)) != RT_ERR_OK)
           return retVal;
        if((retVal = rtl8367c_getAsicRegBits(RTL8367C_REG_P5_PORT_NSEC_26_16 , RTL8367C_PORT_NSEC_H_MASK,&nsec8_h)) != RT_ERR_OK)
           return retVal;
        if((retVal = rtl8367c_getAsicReg(RTL8367C_REG_P5_PORT_NSEC_15_0, &nsec8_l)) != RT_ERR_OK)
           return retVal;
    }else if(port == 6){
        if((retVal = rtl8367c_getAsicReg(RTL8367C_REG_P6_TX_SYNC_SEQ_ID+type, &timeStamp->sequence_id))!=  RT_ERR_OK)
            return retVal;
        if((retVal = rtl8367c_getAsicReg(RTL8367C_REG_P6_PORT_SEC_31_16, &sec_h)) != RT_ERR_OK)
           return retVal;
        if((retVal = rtl8367c_getAsicReg(RTL8367C_REG_P6_PORT_SEC_15_0, &sec_l)) != RT_ERR_OK)
           return retVal;
        if((retVal = rtl8367c_getAsicRegBits(RTL8367C_REG_P6_PORT_NSEC_26_16 , RTL8367C_PORT_NSEC_H_MASK,&nsec8_h)) != RT_ERR_OK)
           return retVal;
        if((retVal = rtl8367c_getAsicReg(RTL8367C_REG_P6_PORT_NSEC_15_0, &nsec8_l)) != RT_ERR_OK)
           return retVal;
    }else if(port == 7){
        if((retVal = rtl8367c_getAsicReg(RTL8367C_REG_P7_TX_SYNC_SEQ_ID+type, &timeStamp->sequence_id))!=  RT_ERR_OK)
            return retVal;
        if((retVal = rtl8367c_getAsicReg(RTL8367C_REG_P7_PORT_SEC_31_16, &sec_h)) != RT_ERR_OK)
           return retVal;
        if((retVal = rtl8367c_getAsicReg(RTL8367C_REG_P7_PORT_SEC_15_0, &sec_l)) != RT_ERR_OK)
           return retVal;
        if((retVal = rtl8367c_getAsicRegBits(RTL8367C_REG_P7_PORT_NSEC_26_16 , RTL8367C_PORT_NSEC_H_MASK,&nsec8_h)) != RT_ERR_OK)
           return retVal;
        if((retVal = rtl8367c_getAsicReg(RTL8367C_REG_P7_PORT_NSEC_15_0, &nsec8_l)) != RT_ERR_OK)
           return retVal;
    }else if(port == 8){
        if((retVal = rtl8367c_getAsicReg(RTL8367C_REG_P8_TX_SYNC_SEQ_ID+type, &timeStamp->sequence_id))!=  RT_ERR_OK)
            return retVal;
        if((retVal = rtl8367c_getAsicReg(RTL8367C_REG_P8_PORT_SEC_31_16, &sec_h)) != RT_ERR_OK)
           return retVal;
        if((retVal = rtl8367c_getAsicReg(RTL8367C_REG_P8_PORT_SEC_15_0, &sec_l)) != RT_ERR_OK)
           return retVal;
        if((retVal = rtl8367c_getAsicRegBits(RTL8367C_REG_P8_PORT_NSEC_26_16 , RTL8367C_PORT_NSEC_H_MASK,&nsec8_h)) != RT_ERR_OK)
           return retVal;
        if((retVal = rtl8367c_getAsicReg(RTL8367C_REG_P8_PORT_NSEC_15_0, &nsec8_l)) != RT_ERR_OK)
           return retVal;
    }else if(port == 9){
        if((retVal = rtl8367c_getAsicReg(RTL8367C_REG_P9_TX_SYNC_SEQ_ID+type, &timeStamp->sequence_id))!=  RT_ERR_OK)
            return retVal;
        if((retVal = rtl8367c_getAsicReg(RTL8367C_REG_P9_PORT_SEC_31_16, &sec_h)) != RT_ERR_OK)
           return retVal;
        if((retVal = rtl8367c_getAsicReg(RTL8367C_REG_P9_PORT_SEC_15_0, &sec_l)) != RT_ERR_OK)
           return retVal;
        if((retVal = rtl8367c_getAsicRegBits(RTL8367C_REG_P9_PORT_NSEC_26_16 , RTL8367C_PORT_NSEC_H_MASK,&nsec8_h)) != RT_ERR_OK)
           return retVal;
        if((retVal = rtl8367c_getAsicReg(RTL8367C_REG_P9_PORT_NSEC_15_0, &nsec8_l)) != RT_ERR_OK)
           return retVal;
    }

    timeStamp->second = (sec_h<<16) | sec_l;
    nano_second_8 = (nsec8_h<<16) | nsec8_l;
    timeStamp->nano_second = nano_second_8<<3;

    return RT_ERR_OK;
}


/* Function Name:
 *      rtl8367c_setAsicEavTrap
 * Description:
 *      Set per-port PTP packet trap to CPU
 * Input:
 *      port         - Physical port number (0~5)
 *      enabled     - 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      If EAV trap enabled, switch will trap PTP packet to CPU
 */
ret_t rtl8367c_setAsicEavTrap(rtk_uint32 port, rtk_uint32 enabled)
{
    if(port > RTL8367C_PORTNO)
        return RT_ERR_PORT_ID;

    return rtl8367c_setAsicRegBit(RTL8367C_REG_PTP_PORT0_CFG1 + (port * 0x20), RTL8367C_PTP_PORT0_CFG1_OFFSET, enabled);
}
/* Function Name:
 *      rtl8367c_getAsicEavTimeSyncEn
 * Description:
 *      Get per-port EPTP packet trap to CPU
 * Input:
 *      port         - Physical port number (0~5)
 *      pEnabled     - 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicEavTrap(rtk_uint32 port, rtk_uint32 *pEnabled)
{
    if(port > RTL8367C_PORTNO)
        return RT_ERR_PORT_ID;

    return rtl8367c_getAsicRegBit(RTL8367C_REG_PTP_PORT0_CFG1 + (port * 0x20), RTL8367C_PTP_PORT0_CFG1_OFFSET, pEnabled);
}

/* Function Name:
 *      rtl8367c_setAsicEavEnable
 * Description:
 *      Set per-port EAV function enable/disable
 * Input:
 *      port         - Physical port number (0~5)
 *      enabled     - 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      If EAV function is enabled, PTP event messgae packet will be attached PTP timestamp for trapping
 */
ret_t rtl8367c_setAsicEavEnable(rtk_uint32 port, rtk_uint32 enabled)
{
    if(port > RTL8367C_PORTNO)
        return RT_ERR_PORT_ID;

    return rtl8367c_setAsicRegBit(RTL8367C_REG_EAV_CTRL0, port, enabled);
}
/* Function Name:
 *      rtl8367c_getAsicEavEnable
 * Description:
 *      Get per-port EAV function enable/disable
 * Input:
 *      port         - Physical port number (0~5)
 *      pEnabled     - 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicEavEnable(rtk_uint32 port, rtk_uint32 *pEnabled)
{
    if(port > RTL8367C_PORTNO)
        return RT_ERR_PORT_ID;

    return rtl8367c_getAsicRegBit(RTL8367C_REG_EAV_CTRL0, port, pEnabled);
}
/* Function Name:
 *      rtl8367c_setAsicEavPriRemapping
 * Description:
 *      Set non-EAV streaming priority remapping
 * Input:
 *      srcpriority - Priority value
 *      priority     - Absolute priority value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                     - Success
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_QOS_INT_PRIORITY      - Invalid priority
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicEavPriRemapping(rtk_uint32 srcpriority, rtk_uint32 priority)
{
    if(srcpriority > RTL8367C_PRIMAX || priority > RTL8367C_PRIMAX)
        return RT_ERR_QOS_INT_PRIORITY;

    return rtl8367c_setAsicRegBits(RTL8367C_EAV_PRIORITY_REMAPPING_REG(srcpriority), RTL8367C_EAV_PRIORITY_REMAPPING_MASK(srcpriority),priority);
}
/* Function Name:
 *      rtl8367c_getAsicEavPriRemapping
 * Description:
 *      Get non-EAV streaming priority remapping
 * Input:
 *      srcpriority - Priority value
 *      pPriority     - Absolute priority value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                     - Success
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_QOS_INT_PRIORITY      - Invalid priority
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicEavPriRemapping(rtk_uint32 srcpriority, rtk_uint32 *pPriority)
{
    if(srcpriority > RTL8367C_PRIMAX )
        return RT_ERR_QOS_INT_PRIORITY;

    return rtl8367c_getAsicRegBits(RTL8367C_EAV_PRIORITY_REMAPPING_REG(srcpriority), RTL8367C_EAV_PRIORITY_REMAPPING_MASK(srcpriority),pPriority);
}


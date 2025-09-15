/*
 * Copyright (C) 2013 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * Unless you and Realtek execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2,
 * available at https://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 *
 * $Revision: 64716 $
 * $Date: 2015-12-31 16:31:55 +0800 (週四, 31 十二月 2015) $
 *
 * Purpose : RTL8367C switch high-level API for RTL8367C
 * Feature : RMA related functions
 *
 */

#include <rtl8367c_asicdrv_rma.h>
/* Function Name:
 *      rtl8367c_setAsicRma
 * Description:
 *      Set reserved multicast address for CPU trapping
 * Input:
 *      index     - reserved multicast LSB byte, 0x00~0x2F is available value
 *      pRmacfg     - type of RMA for trapping frame type setting
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_RMA_ADDR - Invalid RMA address index
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicRma(rtk_uint32 index, rtl8367c_rma_t* pRmacfg)
{
    rtk_uint32 regData = 0;
    ret_t retVal;

    if(index > RTL8367C_RMAMAX)
        return RT_ERR_RMA_ADDR;

    regData |= (pRmacfg->portiso_leaky & 0x0001);
    regData |= ((pRmacfg->vlan_leaky & 0x0001) << 1);
    regData |= ((pRmacfg->keep_format & 0x0001) << 2);
    regData |= ((pRmacfg->trap_priority & 0x0007) << 3);
    regData |= ((pRmacfg->discard_storm_filter & 0x0001) << 6);
    regData |= ((pRmacfg->operation & 0x0003) << 7);

    if( (index >= 0x4 && index <= 0x7) || (index >= 0x9 && index <= 0x0C) || (0x0F == index))
        index = 0x04;
    else if((index >= 0x13 && index <= 0x17) || (0x19 == index) || (index >= 0x1B && index <= 0x1f))
        index = 0x13;
    else if(index >= 0x22 && index <= 0x2F)
        index = 0x22;

    retVal = rtl8367c_setAsicRegBits(RTL8367C_REG_RMA_CTRL00, RTL8367C_TRAP_PRIORITY_MASK, pRmacfg->trap_priority);
    if(retVal != RT_ERR_OK)
        return retVal;

    return rtl8367c_setAsicReg(RTL8367C_REG_RMA_CTRL00+index, regData);
}
/* Function Name:
 *      rtl8367c_getAsicRma
 * Description:
 *      Get reserved multicast address for CPU trapping
 * Input:
 *      index     - reserved multicast LSB byte, 0x00~0x2F is available value
 *      rmacfg     - type of RMA for trapping frame type setting
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_RMA_ADDR - Invalid RMA address index
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicRma(rtk_uint32 index, rtl8367c_rma_t* pRmacfg)
{
    ret_t retVal;
    rtk_uint32 regData;

    if(index > RTL8367C_RMAMAX)
        return RT_ERR_RMA_ADDR;

    if( (index >= 0x4 && index <= 0x7) || (index >= 0x9 && index <= 0x0C) || (0x0F == index))
        index = 0x04;
    else if((index >= 0x13 && index <= 0x17) || (0x19 == index) || (index >= 0x1B && index <= 0x1f))
        index = 0x13;
    else if(index >= 0x22 && index <= 0x2F)
        index = 0x22;

    retVal = rtl8367c_getAsicReg(RTL8367C_REG_RMA_CTRL00+index, &regData);
    if(retVal != RT_ERR_OK)
        return retVal;

    pRmacfg->operation = ((regData >> 7) & 0x0003);
    pRmacfg->discard_storm_filter = ((regData >> 6) & 0x0001);
    pRmacfg->trap_priority = ((regData >> 3) & 0x0007);
    pRmacfg->keep_format = ((regData >> 2) & 0x0001);
    pRmacfg->vlan_leaky = ((regData >> 1) & 0x0001);
    pRmacfg->portiso_leaky = (regData & 0x0001);


    retVal = rtl8367c_getAsicRegBits(RTL8367C_REG_RMA_CTRL00, RTL8367C_TRAP_PRIORITY_MASK, &regData);
    if(retVal != RT_ERR_OK)
        return retVal;

    pRmacfg->trap_priority = regData;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367c_setAsicRmaCdp
 * Description:
 *      Set CDP(Cisco Discovery Protocol) for CPU trapping
 * Input:
 *      pRmacfg     - type of RMA for trapping frame type setting
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_RMA_ADDR - Invalid RMA address index
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicRmaCdp(rtl8367c_rma_t* pRmacfg)
{
    rtk_uint32 regData = 0;
    ret_t retVal;

    if(pRmacfg->operation >= RMAOP_END)
        return RT_ERR_RMA_ACTION;

    if(pRmacfg->trap_priority > RTL8367C_PRIMAX)
        return RT_ERR_QOS_INT_PRIORITY;

    regData |= (pRmacfg->portiso_leaky & 0x0001);
    regData |= ((pRmacfg->vlan_leaky & 0x0001) << 1);
    regData |= ((pRmacfg->keep_format & 0x0001) << 2);
    regData |= ((pRmacfg->trap_priority & 0x0007) << 3);
    regData |= ((pRmacfg->discard_storm_filter & 0x0001) << 6);
    regData |= ((pRmacfg->operation & 0x0003) << 7);

    retVal = rtl8367c_setAsicRegBits(RTL8367C_REG_RMA_CTRL00, RTL8367C_TRAP_PRIORITY_MASK, pRmacfg->trap_priority);
    if(retVal != RT_ERR_OK)
        return retVal;

    return rtl8367c_setAsicReg(RTL8367C_REG_RMA_CTRL_CDP, regData);
}
/* Function Name:
 *      rtl8367c_getAsicRmaCdp
 * Description:
 *      Get CDP(Cisco Discovery Protocol) for CPU trapping
 * Input:
 *      None
 * Output:
 *      pRmacfg     - type of RMA for trapping frame type setting
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_RMA_ADDR - Invalid RMA address index
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicRmaCdp(rtl8367c_rma_t* pRmacfg)
{
    ret_t retVal;
    rtk_uint32 regData;

    retVal = rtl8367c_getAsicReg(RTL8367C_REG_RMA_CTRL_CDP, &regData);
    if(retVal != RT_ERR_OK)
        return retVal;

    pRmacfg->operation = ((regData >> 7) & 0x0003);
    pRmacfg->discard_storm_filter = ((regData >> 6) & 0x0001);
    pRmacfg->trap_priority = ((regData >> 3) & 0x0007);
    pRmacfg->keep_format = ((regData >> 2) & 0x0001);
    pRmacfg->vlan_leaky = ((regData >> 1) & 0x0001);
    pRmacfg->portiso_leaky = (regData & 0x0001);

    retVal = rtl8367c_getAsicRegBits(RTL8367C_REG_RMA_CTRL00, RTL8367C_TRAP_PRIORITY_MASK, &regData);
    if(retVal != RT_ERR_OK)
        return retVal;

    pRmacfg->trap_priority = regData;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367c_setAsicRmaCsstp
 * Description:
 *      Set CSSTP(Cisco Shared Spanning Tree Protocol) for CPU trapping
 * Input:
 *      pRmacfg     - type of RMA for trapping frame type setting
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_RMA_ADDR - Invalid RMA address index
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicRmaCsstp(rtl8367c_rma_t* pRmacfg)
{
    rtk_uint32 regData = 0;
    ret_t retVal;

    if(pRmacfg->operation >= RMAOP_END)
        return RT_ERR_RMA_ACTION;

    if(pRmacfg->trap_priority > RTL8367C_PRIMAX)
        return RT_ERR_QOS_INT_PRIORITY;

    regData |= (pRmacfg->portiso_leaky & 0x0001);
    regData |= ((pRmacfg->vlan_leaky & 0x0001) << 1);
    regData |= ((pRmacfg->keep_format & 0x0001) << 2);
    regData |= ((pRmacfg->trap_priority & 0x0007) << 3);
    regData |= ((pRmacfg->discard_storm_filter & 0x0001) << 6);
    regData |= ((pRmacfg->operation & 0x0003) << 7);

    retVal = rtl8367c_setAsicRegBits(RTL8367C_REG_RMA_CTRL00, RTL8367C_TRAP_PRIORITY_MASK, pRmacfg->trap_priority);
    if(retVal != RT_ERR_OK)
        return retVal;

    return rtl8367c_setAsicReg(RTL8367C_REG_RMA_CTRL_CSSTP, regData);
}
/* Function Name:
 *      rtl8367c_getAsicRmaCsstp
 * Description:
 *      Get CSSTP(Cisco Shared Spanning Tree Protocol) for CPU trapping
 * Input:
 *      None
 * Output:
 *      pRmacfg     - type of RMA for trapping frame type setting
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_RMA_ADDR - Invalid RMA address index
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicRmaCsstp(rtl8367c_rma_t* pRmacfg)
{
    ret_t retVal;
    rtk_uint32 regData;

    retVal = rtl8367c_getAsicReg(RTL8367C_REG_RMA_CTRL_CSSTP, &regData);
    if(retVal != RT_ERR_OK)
        return retVal;

    pRmacfg->operation = ((regData >> 7) & 0x0003);
    pRmacfg->discard_storm_filter = ((regData >> 6) & 0x0001);
    pRmacfg->trap_priority = ((regData >> 3) & 0x0007);
    pRmacfg->keep_format = ((regData >> 2) & 0x0001);
    pRmacfg->vlan_leaky = ((regData >> 1) & 0x0001);
    pRmacfg->portiso_leaky = (regData & 0x0001);

    retVal = rtl8367c_getAsicRegBits(RTL8367C_REG_RMA_CTRL00, RTL8367C_TRAP_PRIORITY_MASK, &regData);
    if(retVal != RT_ERR_OK)
        return retVal;

    pRmacfg->trap_priority = regData;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtl8367c_setAsicRmaLldp
 * Description:
 *      Set LLDP for CPU trapping
 * Input:
 *      pRmacfg     - type of RMA for trapping frame type setting
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_RMA_ADDR - Invalid RMA address index
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicRmaLldp(rtk_uint32 enabled, rtl8367c_rma_t* pRmacfg)
{
    rtk_uint32 regData = 0;
    ret_t retVal;

    if(enabled > 1)
        return RT_ERR_ENABLE;

    if(pRmacfg->operation >= RMAOP_END)
        return RT_ERR_RMA_ACTION;

    if(pRmacfg->trap_priority > RTL8367C_PRIMAX)
        return RT_ERR_QOS_INT_PRIORITY;

    retVal = rtl8367c_setAsicRegBit(RTL8367C_REG_RMA_LLDP_EN, RTL8367C_RMA_LLDP_EN_OFFSET,enabled);
    if(retVal != RT_ERR_OK)
        return retVal;

    regData |= (pRmacfg->portiso_leaky & 0x0001);
    regData |= ((pRmacfg->vlan_leaky & 0x0001) << 1);
    regData |= ((pRmacfg->keep_format & 0x0001) << 2);
    regData |= ((pRmacfg->trap_priority & 0x0007) << 3);
    regData |= ((pRmacfg->discard_storm_filter & 0x0001) << 6);
    regData |= ((pRmacfg->operation & 0x0003) << 7);

    retVal = rtl8367c_setAsicRegBits(RTL8367C_REG_RMA_CTRL00, RTL8367C_TRAP_PRIORITY_MASK, pRmacfg->trap_priority);
    if(retVal != RT_ERR_OK)
        return retVal;

    return rtl8367c_setAsicReg(RTL8367C_REG_RMA_CTRL_LLDP, regData);
}
/* Function Name:
 *      rtl8367c_getAsicRmaLldp
 * Description:
 *      Get LLDP for CPU trapping
 * Input:
 *      None
 * Output:
 *      pRmacfg     - type of RMA for trapping frame type setting
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_RMA_ADDR - Invalid RMA address index
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicRmaLldp(rtk_uint32 *pEnabled, rtl8367c_rma_t* pRmacfg)
{
    ret_t retVal;
    rtk_uint32 regData;

    retVal = rtl8367c_getAsicRegBit(RTL8367C_REG_RMA_LLDP_EN, RTL8367C_RMA_LLDP_EN_OFFSET,pEnabled);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8367c_getAsicReg(RTL8367C_REG_RMA_CTRL_LLDP, &regData);
    if(retVal != RT_ERR_OK)
        return retVal;

    pRmacfg->operation = ((regData >> 7) & 0x0003);
    pRmacfg->discard_storm_filter = ((regData >> 6) & 0x0001);
    pRmacfg->trap_priority = ((regData >> 3) & 0x0007);
    pRmacfg->keep_format = ((regData >> 2) & 0x0001);
    pRmacfg->vlan_leaky = ((regData >> 1) & 0x0001);
    pRmacfg->portiso_leaky = (regData & 0x0001);

    retVal = rtl8367c_getAsicRegBits(RTL8367C_REG_RMA_CTRL00, RTL8367C_TRAP_PRIORITY_MASK, &regData);
    if(retVal != RT_ERR_OK)
        return retVal;

    pRmacfg->trap_priority = regData;

    return RT_ERR_OK;
}


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
 * Purpose : RTK switch high-level API for RTL8367/RTL8373
 * Feature : Here is a list of all functions and variables in RMA module.
 *
 */

#include <rtk_switch.h>
#include <rtk_error.h>
#include <dal_rtl8373_rma.h>
#include <rtl8373_asicdrv.h>
#include <string.h>


/* Function Name:
 *      dal_rtl8373_asicRma_set
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
ret_t dal_rtl8373_asicRma_set(rtk_uint32 index, rtk_rmaParam_t* pRmacfg)
{
    ret_t retVal;

    if(index > RTL8373_RMAMAX)
        return RT_ERR_RMA_ADDR;

    if( (index >= 0x4 && index <= 0x7) || (index >= 0x9 && index <= 0x0C) || (0x0F == index))
        index = 0x04;
    else if(index == 0x8)
        index = 0x5;
    else if(index == 0xD)
        index = 0x6;
    else if(index == 0xE)
        index = 0x7;
    else if(index == 0x10)
        index = 0x8;
    else if(index == 0x11)
        index = 0x9;
    else if(index == 0x12)
        index = 0xA;
    else if((index >= 0x13 && index <= 0x17) || (0x19 == index) || (index >= 0x1B && index <= 0x1f))
        index = 0xB;
    else if(index == 0x18)
        index = 0xC;
    else if(index == 0x1A)
        index = 0xD;
    else if(index == 0x20)
        index = 0xE;
    else if(index == 0x21)
        index = 0xF;
    else if(index >= 0x22 && index <= 0x2F)
        index = 0x10;

    retVal = rtl8373_setAsicRegBits(RTL8373_RMA_OP_CTRL_00_ADDR+index*4, RTL8373_RMA_OP_CTRL_00_RMA_ACT_00_MASK, pRmacfg->operation);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8373_setAsicRegBit(RTL8373_RMA_OP_CTRL_00_ADDR+index*4, RTL8373_RMA_OP_CTRL_00_DIS_STORM_CTRL_00_OFFSET, pRmacfg->discard_storm_filter);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8373_setAsicRegBit(RTL8373_RMA_OP_CTRL_00_ADDR+index*4, RTL8373_RMA_OP_CTRL_00_CKEEP_00_OFFSET, pRmacfg->keep_format);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8373_setAsicRegBit(RTL8373_RMA_OP_CTRL_00_ADDR+index*4, RTL8373_RMA_OP_CTRL_00_VLAN_LEAKY_00_OFFSET, pRmacfg->vlan_leaky);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8373_setAsicRegBit(RTL8373_RMA_OP_CTRL_00_ADDR+index*4, RTL8373_RMA_OP_CTRL_00_PISO_LEAKY_00_OFFSET, pRmacfg->portiso_leaky);
    if(retVal != RT_ERR_OK)
        return retVal;

    return rtl8373_setAsicRegBits(RTL8373_RMA_CFG_ADDR, RTL8373_RMA_CFG_RMA_TRAP_PRI_MASK, pRmacfg->trap_priority);
}


/* Function Name:
 *      dal_rtl8373_asicRma_get
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
ret_t dal_rtl8373_asicRma_get(rtk_uint32 index, rtk_rmaParam_t* pRmacfg)
{
    ret_t retVal;

    
    if( (index >= 0x4 && index <= 0x7) || (index >= 0x9 && index <= 0x0C) || (0x0F == index))
            index = 0x04;
    else if(index == 0x8)
        index = 0x5;
    else if(index == 0xD)
        index = 0x6;
    else if(index == 0xE)
        index = 0x7;
    else if(index == 0x10)
        index = 0x8;
    else if(index == 0x11)
        index = 0x9;
    else if(index == 0x12)
        index = 0xA;
    else if((index >= 0x13 && index <= 0x17) || (0x19 == index) || (index >= 0x1B && index <= 0x1f))
        index = 0xB;
    else if(index == 0x18)
        index = 0xC;
    else if(index == 0x1A)
        index = 0xD;
    else if(index == 0x20)
        index = 0xE;
    else if(index == 0x21)
        index = 0xF;
    else if(index >= 0x22 && index <= 0x2F)
        index = 0x10;
    

    retVal = rtl8373_getAsicRegBits(RTL8373_RMA_OP_CTRL_00_ADDR+index*4, RTL8373_RMA_OP_CTRL_00_RMA_ACT_00_MASK, &pRmacfg->operation);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8373_getAsicRegBit(RTL8373_RMA_OP_CTRL_00_ADDR+index*4, RTL8373_RMA_OP_CTRL_00_DIS_STORM_CTRL_00_OFFSET, &pRmacfg->discard_storm_filter);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8373_getAsicRegBit(RTL8373_RMA_OP_CTRL_00_ADDR+index*4, RTL8373_RMA_OP_CTRL_00_CKEEP_00_OFFSET, &pRmacfg->keep_format);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8373_getAsicRegBit(RTL8373_RMA_OP_CTRL_00_ADDR+index*4, RTL8373_RMA_OP_CTRL_00_VLAN_LEAKY_00_OFFSET, &pRmacfg->vlan_leaky);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8373_getAsicRegBit(RTL8373_RMA_OP_CTRL_00_ADDR+index*4, RTL8373_RMA_OP_CTRL_00_PISO_LEAKY_00_OFFSET, &pRmacfg->portiso_leaky);
    if(retVal != RT_ERR_OK)
        return retVal;


    retVal = rtl8373_getAsicRegBits(RTL8373_RMA_CFG_ADDR, RTL8373_RMA_CFG_RMA_TRAP_PRI_MASK, &pRmacfg->trap_priority);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_asicRmaCdp_set
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
ret_t dal_rtl8373_asicRmaCdp_set(rtk_rmaParam_t* pRmacfg)
{
    ret_t retVal;

    if(pRmacfg->operation >= RMAOP_END)
        return RT_ERR_RMA_ACTION;

    if(pRmacfg->trap_priority > RTL8373_PRIMAX)
        return RT_ERR_QOS_INT_PRIORITY;

    retVal = rtl8373_setAsicRegBits(RTL8373_RMA_OP_CTRL_CDP_ADDR, RTL8373_RMA_OP_CTRL_CDP_RMA_ACT_CDP_MASK, pRmacfg->operation);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8373_setAsicRegBit(RTL8373_RMA_OP_CTRL_CDP_ADDR, RTL8373_RMA_OP_CTRL_CDP_DIS_STORM_CTRL_CDP_OFFSET, pRmacfg->discard_storm_filter);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8373_setAsicRegBit(RTL8373_RMA_OP_CTRL_CDP_ADDR, RTL8373_RMA_OP_CTRL_CDP_CKEEP_CDP_OFFSET, pRmacfg->keep_format);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8373_setAsicRegBit(RTL8373_RMA_OP_CTRL_CDP_ADDR, RTL8373_RMA_OP_CTRL_CDP_VLAN_LEAKY_CDP_OFFSET, pRmacfg->vlan_leaky);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8373_setAsicRegBit(RTL8373_RMA_OP_CTRL_CDP_ADDR, RTL8373_RMA_OP_CTRL_CDP_PISO_LEAKY_CDP_OFFSET, pRmacfg->portiso_leaky);
    if(retVal != RT_ERR_OK)
        return retVal;

    return rtl8373_setAsicRegBits(RTL8373_RMA_CFG_ADDR, RTL8373_RMA_CFG_RMA_TRAP_PRI_MASK, pRmacfg->trap_priority);
}

/* Function Name:
 *      dal_rtl8373_asicRmaCdp_get
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
ret_t dal_rtl8373_asicRmaCdp_get(rtk_rmaParam_t* pRmacfg)
{
    ret_t retVal;

    retVal = rtl8373_getAsicRegBits(RTL8373_RMA_OP_CTRL_CDP_ADDR, RTL8373_RMA_OP_CTRL_CDP_RMA_ACT_CDP_MASK, &pRmacfg->operation);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8373_getAsicRegBit(RTL8373_RMA_OP_CTRL_CDP_ADDR, RTL8373_RMA_OP_CTRL_CDP_DIS_STORM_CTRL_CDP_OFFSET, &pRmacfg->discard_storm_filter);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8373_getAsicRegBit(RTL8373_RMA_OP_CTRL_CDP_ADDR, RTL8373_RMA_OP_CTRL_CDP_CKEEP_CDP_OFFSET, &pRmacfg->keep_format);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8373_getAsicRegBit(RTL8373_RMA_OP_CTRL_CDP_ADDR, RTL8373_RMA_OP_CTRL_CDP_VLAN_LEAKY_CDP_OFFSET, &pRmacfg->vlan_leaky);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8373_getAsicRegBit(RTL8373_RMA_OP_CTRL_CDP_ADDR, RTL8373_RMA_OP_CTRL_CDP_PISO_LEAKY_CDP_OFFSET, &pRmacfg->portiso_leaky);
    if(retVal != RT_ERR_OK)
        return retVal;


    retVal = rtl8373_getAsicRegBits(RTL8373_RMA_CFG_ADDR, RTL8373_RMA_CFG_RMA_TRAP_PRI_MASK, &pRmacfg->trap_priority);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;

}


/* Function Name:
 *      dal_rtl8373_asicRmaCsstp_set
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
ret_t dal_rtl8373_asicRmaCsstp_set(rtk_rmaParam_t* pRmacfg)
{
    ret_t retVal;

    if(pRmacfg->operation >= RMAOP_END)
        return RT_ERR_RMA_ACTION;

    if(pRmacfg->trap_priority > RTL8373_PRIMAX)
        return RT_ERR_QOS_INT_PRIORITY;

    retVal = rtl8373_setAsicRegBits(RTL8373_RMA_OP_CTRL_CSSTP_ADDR, RTL8373_RMA_OP_CTRL_CSSTP_RMA_ACT_CSSTP_MASK, pRmacfg->operation);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8373_setAsicRegBit(RTL8373_RMA_OP_CTRL_CSSTP_ADDR, RTL8373_RMA_OP_CTRL_CSSTP_DIS_STORM_CTRL_CSSTP_OFFSET, pRmacfg->discard_storm_filter);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8373_setAsicRegBit(RTL8373_RMA_OP_CTRL_CSSTP_ADDR, RTL8373_RMA_OP_CTRL_CSSTP_CKEEP_CSSTP_OFFSET, pRmacfg->keep_format);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8373_setAsicRegBit(RTL8373_RMA_OP_CTRL_CSSTP_ADDR, RTL8373_RMA_OP_CTRL_CSSTP_VLAN_LEAKY_CSSTP_OFFSET, pRmacfg->vlan_leaky);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8373_setAsicRegBit(RTL8373_RMA_OP_CTRL_CSSTP_ADDR, RTL8373_RMA_OP_CTRL_CSSTP_PISO_LEAKY_CSSTP_OFFSET, pRmacfg->portiso_leaky);
    if(retVal != RT_ERR_OK)
        return retVal;

    return rtl8373_setAsicRegBits(RTL8373_RMA_CFG_ADDR, RTL8373_RMA_CFG_RMA_TRAP_PRI_MASK, pRmacfg->trap_priority);
}


/* Function Name:
 *      dal_rtl8373_asicRmaCsstp_get
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
ret_t dal_rtl8373_asicRmaCsstp_get(rtk_rmaParam_t* pRmacfg)
{
    ret_t retVal;

    retVal = rtl8373_getAsicRegBits(RTL8373_RMA_OP_CTRL_CSSTP_ADDR, RTL8373_RMA_OP_CTRL_CSSTP_RMA_ACT_CSSTP_MASK, &pRmacfg->operation);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8373_getAsicRegBit(RTL8373_RMA_OP_CTRL_CSSTP_ADDR, RTL8373_RMA_OP_CTRL_CSSTP_DIS_STORM_CTRL_CSSTP_OFFSET, &pRmacfg->discard_storm_filter);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8373_getAsicRegBit(RTL8373_RMA_OP_CTRL_CSSTP_ADDR, RTL8373_RMA_OP_CTRL_CSSTP_CKEEP_CSSTP_OFFSET, &pRmacfg->keep_format);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8373_getAsicRegBit(RTL8373_RMA_OP_CTRL_CSSTP_ADDR, RTL8373_RMA_OP_CTRL_CSSTP_VLAN_LEAKY_CSSTP_OFFSET, &pRmacfg->vlan_leaky);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8373_getAsicRegBit(RTL8373_RMA_OP_CTRL_CSSTP_ADDR, RTL8373_RMA_OP_CTRL_CSSTP_PISO_LEAKY_CSSTP_OFFSET, &pRmacfg->portiso_leaky);
    if(retVal != RT_ERR_OK)
        return retVal;


    retVal = rtl8373_getAsicRegBits(RTL8373_RMA_CFG_ADDR, RTL8373_RMA_CFG_RMA_TRAP_PRI_MASK, &pRmacfg->trap_priority);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}



/* Function Name:
 *      dal_rtl8373_asicRmaLldp_set
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
ret_t dal_rtl8373_asicRmaLldp_set(rtk_uint32 enabled, rtk_rmaParam_t* pRmacfg)
{
    ret_t retVal;

    if(enabled > 1)
        return RT_ERR_ENABLE;

    if(pRmacfg->operation >= RMAOP_END)
        return RT_ERR_RMA_ACTION;

    if(pRmacfg->trap_priority > RTL8373_PRIMAX)
        return RT_ERR_QOS_INT_PRIORITY;

    retVal = rtl8373_setAsicRegBit(RTL8373_RMA_CFG_ADDR, RTL8373_RMA_CFG_LLDP_EN_OFFSET,enabled);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8373_setAsicRegBits(RTL8373_RMA_OP_CTRL_LLDP_ADDR, RTL8373_RMA_OP_CTRL_LLDP_RMA_ACT_LLDP_MASK, pRmacfg->operation);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8373_setAsicRegBit(RTL8373_RMA_OP_CTRL_LLDP_ADDR, RTL8373_RMA_OP_CTRL_LLDP_DIS_STORM_CTRL_LLDP_OFFSET, pRmacfg->discard_storm_filter);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8373_setAsicRegBit(RTL8373_RMA_OP_CTRL_LLDP_ADDR, RTL8373_RMA_OP_CTRL_LLDP_CKEEP_LLDP_OFFSET, pRmacfg->keep_format);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8373_setAsicRegBit(RTL8373_RMA_OP_CTRL_LLDP_ADDR, RTL8373_RMA_OP_CTRL_LLDP_VLAN_LEAKY_LLDP_OFFSET, pRmacfg->vlan_leaky);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8373_setAsicRegBit(RTL8373_RMA_OP_CTRL_LLDP_ADDR, RTL8373_RMA_OP_CTRL_LLDP_PISO_LEAKY_LLDP_OFFSET, pRmacfg->portiso_leaky);
    if(retVal != RT_ERR_OK)
        return retVal;

    return rtl8373_setAsicRegBits(RTL8373_RMA_CFG_ADDR, RTL8373_RMA_CFG_RMA_TRAP_PRI_MASK, pRmacfg->trap_priority);
}


/* Function Name:
 *      dal_rtl8373_asicRmaLldp_get
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
ret_t dal_rtl8373_asicRmaLldp_get(rtk_uint32 *pEnabled, rtk_rmaParam_t* pRmacfg)
{
    ret_t retVal;

    retVal = rtl8373_getAsicRegBit(RTL8373_RMA_CFG_ADDR, RTL8373_RMA_CFG_LLDP_EN_OFFSET,pEnabled);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8373_getAsicRegBits(RTL8373_RMA_OP_CTRL_LLDP_ADDR, RTL8373_RMA_OP_CTRL_LLDP_RMA_ACT_LLDP_MASK, &pRmacfg->operation);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8373_getAsicRegBit(RTL8373_RMA_OP_CTRL_LLDP_ADDR, RTL8373_RMA_OP_CTRL_LLDP_DIS_STORM_CTRL_LLDP_OFFSET, &pRmacfg->discard_storm_filter);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8373_getAsicRegBit(RTL8373_RMA_OP_CTRL_LLDP_ADDR, RTL8373_RMA_OP_CTRL_LLDP_CKEEP_LLDP_OFFSET, &pRmacfg->keep_format);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8373_getAsicRegBit(RTL8373_RMA_OP_CTRL_LLDP_ADDR, RTL8373_RMA_OP_CTRL_LLDP_VLAN_LEAKY_LLDP_OFFSET, &pRmacfg->vlan_leaky);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8373_getAsicRegBit(RTL8373_RMA_OP_CTRL_LLDP_ADDR, RTL8373_RMA_OP_CTRL_LLDP_PISO_LEAKY_LLDP_OFFSET, &pRmacfg->portiso_leaky);
    if(retVal != RT_ERR_OK)
        return retVal;


    retVal = rtl8373_getAsicRegBits(RTL8373_RMA_CFG_ADDR, RTL8373_RMA_CFG_RMA_TRAP_PRI_MASK, &pRmacfg->trap_priority);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_asicRmaTrapPri_set
 * Description:
 *      Set RMA function trap priority
 * Input:
 *      pri     - trap priority for all RMA
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_RMA_ADDR - Invalid RMA address index
 * Note:
 *      None
 */

ret_t dal_rtl8373_asicRmaTrapPri_set(rtk_uint32 pri)
{
	ret_t retVal;
	retVal = rtl8373_setAsicRegBits(RTL8373_RMA_CFG_ADDR, RTL8373_RMA_CFG_RMA_TRAP_PRI_MASK, pri);

	return retVal;
}


/* Function Name:
 *      dal_rtl8373_asicRmaTrapPri_get
 * Description:
 *      Set RMA function trap priority
 * Input:
 *      pri     - trap priority for all RMA
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_RMA_ADDR - Invalid RMA address index
 * Note:
 *      None
 */

ret_t dal_rtl8373_asicRmaTrapPri_get(rtk_uint32 * pri)
{
	ret_t retVal;
	retVal = rtl8373_getAsicRegBits(RTL8373_RMA_CFG_ADDR, RTL8373_RMA_CFG_RMA_TRAP_PRI_MASK, pri);

	return retVal;
}





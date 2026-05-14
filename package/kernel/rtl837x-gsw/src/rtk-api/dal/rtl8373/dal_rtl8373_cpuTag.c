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
 * Feature : Here is a list of all functions and variables in CPU module.
 *
 */

#include <rtk_switch.h>
#include <rtk_error.h>
#include <dal/rtl8373/dal_rtl8373_cpuTag.h>
#include <string.h>
#include <dal/rtl8373/rtl8373_asicdrv.h>

/* Function Name:
 *      dal_rtl8373_cpuTag_externalCpuPort_set
 * Description:
 *      Set external cpu port
 * Input:
 *      extCpuPort - port number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *     API can set destination port of trapping frame
 */
rtk_api_ret_t dal_rtl8373_cpuTag_externalCpuPort_set(rtk_uint32 extCpuPort)
{
    rtk_api_ret_t retVal = 0;

    if((extCpuPort > RTK_MAX_NUM_OF_PORT-2) && (extCpuPort  != 0xF))
        return RT_ERR_PORT_MASK;

    retVal = rtl8373_setAsicRegBits(RTL8373_EXT_CPU_CTRL_ADDR , RTL8373_EXT_CPU_CTRL_PORT_MASK, extCpuPort);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_cpuTag_externalCpuPort_get
 * Description:
 *      Get external cpu port
 * Input:
 *      None
 * Output:
 *      pExtCpuPort     - port number
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *     None
 */
rtk_api_ret_t dal_rtl8373_cpuTag_externalCpuPort_get(rtk_uint32 *pExtCpuPort)
{
    rtk_api_ret_t retVal= 0;

    if(pExtCpuPort == NULL)
        return RT_ERR_NULL_POINTER;

    retVal = rtl8373_getAsicRegBits(RTL8373_EXT_CPU_CTRL_ADDR , RTL8373_EXT_CPU_CTRL_PORT_MASK, pExtCpuPort);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_cpuTag_tpid_set
 * Description:
 *      Set  cpu tag protocol id
 * Input:
 *      tpid - protocol ID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *     None
 */
rtk_api_ret_t dal_rtl8373_cpuTag_tpid_set(rtk_uint32 tpid)
{
    rtk_api_ret_t retVal = 0;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();
    if(tpid >= RTK_MAX_NUM_OF_PROTO_TYPE)
        return RT_ERR_INPUT;

    retVal = rtl8373_setAsicRegBits(RTL8373_CPU_TAG_TPID_CTRL_ADDR, RTL8373_CPU_TAG_TPID_CTRL_TPID_MASK, tpid);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_cpuTag_tpid_get
 * Description:
 *      Get cpu tag protocol id
 * Input:
 *      None
 * Output:
 *      pTpid - protocol ID
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *     None
 */
rtk_api_ret_t dal_rtl8373_cpuTag_tpid_get(rtk_uint32 *pTpid)
{
    rtk_api_ret_t retVal = 0;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(pTpid == NULL)
        return RT_ERR_NULL_POINTER;
    
    retVal = rtl8373_getAsicRegBits(RTL8373_CPU_TAG_TPID_CTRL_ADDR, RTL8373_CPU_TAG_TPID_CTRL_TPID_MASK, pTpid);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_cpuTag_enable_set
 * Description:
 *      Set CPU port function enable/disable.
 * Input: 
 *      type - CPU type: internal cpu or external cpu
 *      status - CPU port function enable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameter.
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      The API can set CPU port function enable/disable.
 */
rtk_api_ret_t dal_rtl8373_cpuTag_enable_set(rtk_cpu_type_t type,  rtk_enable_t status)
{
    rtk_api_ret_t retVal = 0;
    rtk_uint32 extCpuPort = 0, awarePmsk = 0;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if ((type >= CPU_TYPE_END) || (status >= RTK_ENABLE_END))
        return RT_ERR_INPUT;

    /*update cpu tag aware portmask*/
    if ((retVal = rtl8373_getAsicRegBits(RTL8373_CPU_TAG_AWARE_CTRL_ADDR, RTL8373_CPU_TAG_AWARE_CTRL_PMSK_MASK, &awarePmsk)) != RT_ERR_OK)
        return retVal;

    if(type)
    {
        if ((retVal = rtl8373_getAsicRegBits(RTL8373_EXT_CPU_CTRL_ADDR, RTL8373_EXT_CPU_CTRL_PORT_MASK, &extCpuPort)) != RT_ERR_OK)
            return retVal;
        
        if(status == ENABLED)
            awarePmsk |= (1 << extCpuPort);
        else
            awarePmsk &= (~(1 << extCpuPort));

        /*cfg cpu tag enable status*/
        if ((retVal = rtl8373_setAsicRegBit(RTL8373_CPU_TAG_CTRL_ADDR, RTL8373_CPU_TAG_CTRL_EXT_CPUTAG_EN_OFFSET, (status == ENABLED ? 1:0))) != RT_ERR_OK)
            return retVal;
    }
    else
    {        
        if(status == ENABLED)
            awarePmsk |= INTERNAL_CPU_PMSK;
        else 
            awarePmsk &= (INTERNAL_CPU_PMSK-1);
        
        /*cfg cpu tag enable status*/
        if ((retVal = rtl8373_setAsicRegBit(RTL8373_CPU_TAG_CTRL_ADDR, RTL8373_CPU_TAG_CTRL_INT_CPUTAG_EN_OFFSET, (status == ENABLED ? 1:0))) != RT_ERR_OK)
            return retVal;
    }
    
    /*write_back cpu tag aware portmask*/
    if ((retVal = rtl8373_setAsicRegBits(RTL8373_CPU_TAG_AWARE_CTRL_ADDR, RTL8373_CPU_TAG_AWARE_CTRL_PMSK_MASK, awarePmsk)) != RT_ERR_OK)
        return retVal;    

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_cpuTag_enable_get
 * Description:
 *      Get CPU port and its setting.
 * Input:
 *      type - CPU type: internal cpu or external cpu
 * Output:
 *      pStatus - CPU port function enable
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_INPUT            - Invalid input parameters.
 *      RT_ERR_L2_NO_CPU_PORT   - CPU port is not exist
 * Note:
 *      The API can get CPU port function enable/disable.
 */
rtk_api_ret_t dal_rtl8373_cpuTag_enable_get(rtk_cpu_type_t type,  rtk_enable_t *pStatus)
{
    rtk_api_ret_t retVal = 0;
    rtk_uint32 regData = 0;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pStatus)
        return RT_ERR_NULL_POINTER;

    if (type >= CPU_TYPE_END) 
        return RT_ERR_INPUT;

    if ((retVal = rtl8373_getAsicReg(RTL8373_CPU_TAG_CTRL_ADDR, &regData)) != RT_ERR_OK)
        return retVal;
    
    if(!type)
        *pStatus = (rtk_enable_t)((regData & RTL8373_CPU_TAG_CTRL_INT_CPUTAG_EN_MASK) >> RTL8373_CPU_TAG_CTRL_INT_CPUTAG_EN_OFFSET);
    else
        *pStatus = (rtk_enable_t)((regData & RTL8373_CPU_TAG_CTRL_EXT_CPUTAG_EN_MASK) >> RTL8373_CPU_TAG_CTRL_EXT_CPUTAG_EN_OFFSET);

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_cpuTag_insertMode_set
 * Description:
 *      Set internal & external CPU port tag insert mode.
 * Input:
 *      type -  - CPU type: internal cpu or external cpu
 *      mode - CPU tag insert for packets egress from CPU port.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameter.
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      The API can set CPU  inserting proprietary CPU tag mode (Length/Type 0x8899)
 *      to the frame that transmitting to CPU port.
 *      The inset cpu tag mode is as following:
 *      - CPU_INSERT_TO_ALL
 *      - CPU_INSERT_TO_TRAPPING
 *      - CPU_INSERT_TO_NONE
 */
rtk_api_ret_t dal_rtl8373_cpuTag_insertMode_set(rtk_cpu_type_t type,  rtk_cpuTag_insertMode_t mode)
{
    rtk_api_ret_t retVal = 0;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();
    
    if( (mode >= CPU_INSERT_END) || (type >= CPU_TYPE_END))
        return RT_ERR_INPUT;
    if(type)
    {
        if ((retVal = rtl8373_setAsicRegBits(RTL8373_CPU_TAG_CTRL_ADDR, RTL8373_CPU_TAG_CTRL_EXT_CPUTAG_INSERTMOD_MASK, (rtk_uint32)(mode))) != RT_ERR_OK)
            return retVal;
    }
    else
    {
        if ((retVal = rtl8373_setAsicRegBits(RTL8373_CPU_TAG_CTRL_ADDR, RTL8373_CPU_TAG_CTRL_INT_CPUTAG_INSERTMOD_MASK, (rtk_uint32)(mode))) != RT_ERR_OK)
            return retVal;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_cpuTag_insertMode_get
 * Description:
 *      Get internal & external CPU port tag insert mode.
 * Input:
 *      type -  - CPU type: internal cpu or external cpu
 * Output:
 *      pMode - CPU tag insert for packets egress from CPU port, 0:all insert 1:Only for trapped packets 2:no insert.
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_INPUT            - Invalid input parameters.
 *      RT_ERR_L2_NO_CPU_PORT   - CPU port is not exist
 * Note:
 *      The API can get configured CPU insert mode.
 *      The inset cpu tag mode is as following:
 *      - CPU_INSERT_TO_ALL
 *      - CPU_INSERT_TO_TRAPPING
 *      - CPU_INSERT_TO_NONE
 */
rtk_api_ret_t dal_rtl8373_cpuTag_insertMode_get(rtk_cpu_type_t type,  rtk_cpuTag_insertMode_t *pMode)
{
    rtk_api_ret_t retVal = 0;
    rtk_uint32 regData = 0;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pMode)
        return RT_ERR_NULL_POINTER;

    if(type >= CPU_TYPE_END)
        return RT_ERR_INPUT;

    if ((retVal = rtl8373_getAsicReg(RTL8373_CPU_TAG_CTRL_ADDR, &regData)) != RT_ERR_OK)
        return retVal;

    if(type)
    {
        *pMode = (rtk_cpuTag_insertMode_t)((regData & RTL8373_CPU_TAG_CTRL_EXT_CPUTAG_INSERTMOD_MASK) >> RTL8373_CPU_TAG_CTRL_EXT_CPUTAG_INSERTMOD_OFFSET);
    } else {
        *pMode = (rtk_cpuTag_insertMode_t)((regData & RTL8373_CPU_TAG_CTRL_INT_CPUTAG_INSERTMOD_MASK) >> RTL8373_CPU_TAG_CTRL_INT_CPUTAG_INSERTMOD_OFFSET);
    }
    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_cpu_awarePort_set
 * Description:
 *      Set CPU aware port mask.
 * Input:
 *      portmask - Port mask.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_MASK      - Invalid port mask.
 * Note:
 *      The API can set configured CPU aware port mask.
 */
rtk_api_ret_t dal_rtl8373_cpuTag_awarePort_set(rtk_portmask_t *pPortmask)
{
    rtk_api_ret_t retVal = 0;
    rtk_uint32 phyMbrPmask = 0;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Valid port mask */
    if(NULL == pPortmask)
        return RT_ERR_NULL_POINTER;

    /* Check port mask valid */
    RTK_CHK_PORTMASK_VALID(pPortmask);

    if(rtk_switch_portmask_L2P_get(pPortmask, &phyMbrPmask) != RT_ERR_OK)
        return RT_ERR_FAILED;

    if ((retVal = rtl8373_setAsicRegBits(RTL8373_CPU_TAG_AWARE_CTRL_ADDR, RTL8373_CPU_TAG_AWARE_CTRL_PMSK_MASK, phyMbrPmask)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_cpu_awarePort_get
 * Description:
 *      Get CPU aware port mask.
 * Input:
 *      None
 * Output:
 *      pPortmask - Port mask.
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 * Note:
 *      The API can get configured CPU aware port mask.
 */
rtk_api_ret_t dal_rtl8373_cpuTag_awarePort_get(rtk_portmask_t *pPortmask)
{
    rtk_api_ret_t retVal = 0;
    rtk_uint32 pmsk = 0;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pPortmask)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8373_getAsicRegBits(RTL8373_CPU_TAG_AWARE_CTRL_ADDR, RTL8373_CPU_TAG_AWARE_CTRL_PMSK_MASK, &pmsk)) != RT_ERR_OK)
        return retVal;

    if(rtk_switch_portmask_P2L_get(pmsk, pPortmask) != RT_ERR_OK)
        return RT_ERR_FAILED;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_cpu_priRemap_set
 * Description:
 *      Configure CPU priorities mapping to internal absolute priority For internal CPU.
 * Input:
 *      type      - identify internal cpu or external cpu
 *      intPri     - internal priority value.
 *      newPri    - new internal priority value.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_INPUT            - Invalid input parameters.
 *      RT_ERR_VLAN_PRIORITY    - Invalid 1p priority.
 *      RT_ERR_QOS_INT_PRIORITY - Invalid priority.
 * Note:
 *      Priority of CPU tag assignment for internal asic priority, and it is used for queue usage and packet scheduling.
 */
rtk_api_ret_t dal_rtl8373_cpuTag_priRemap_set(rtk_cpu_type_t type, rtk_pri_t intPri, rtk_pri_t newPri)
{
    rtk_api_ret_t retVal = 0;
    rtk_uint32 regAddr = 0, bitsMask = 0;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if ((newPri > RTL8373_PRIMAX) || (intPri > RTL8373_PRIMAX))
        return  RT_ERR_VLAN_PRIORITY;
    if(type >= CPU_TYPE_END) 
        return RT_ERR_INPUT;
    
    if(type == INTERNAL_CPU)
        regAddr = RTL8373_INCPU_PRI_REMAP_ADDR;
    else
        regAddr = RTL8373_EXCPU_PRI_REMAP_ADDR;

    bitsMask = RTL8373_INCPU_PRI_REMAP_INTPRI0_TO_VAL_MASK << (intPri  << 2);

    if ((retVal = rtl8373_setAsicRegBits(regAddr, bitsMask, newPri)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_cpu_priRemap_get
 * Description:
 *      Configure CPU priorities mapping to internal absolute priority.
 * Input:
 *      type        - identify internal cpu or external cpu
 *      intPri     - internal priority value.
 * Output:
 *      pNewPri    - new internal priority value.
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_INPUT            - Invalid input parameters.
 *      RT_ERR_VLAN_PRIORITY    - Invalid 1p priority.
 *      RT_ERR_QOS_INT_PRIORITY - Invalid priority.
 * Note:
 *      Priority of CPU tag assignment for internal asic priority, and it is used for queue usage and packet scheduling.
 */
rtk_api_ret_t dal_rtl8373_cpuTag_priRemap_get(rtk_cpu_type_t type, rtk_pri_t intPri, rtk_pri_t *pNewPri)
{
    rtk_api_ret_t retVal = 0;
    rtk_uint32 regAddr = 0, bitsMask = 0;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pNewPri)
        return RT_ERR_NULL_POINTER;

    if(intPri > RTL8373_PRIMAX) 
        return  RT_ERR_QOS_INT_PRIORITY;

    if(type >= CPU_TYPE_END) 
        return RT_ERR_INPUT;

    if(type == INTERNAL_CPU)
        regAddr = RTL8373_INCPU_PRI_REMAP_ADDR;
    else
        regAddr = RTL8373_EXCPU_PRI_REMAP_ADDR;

    bitsMask = RTL8373_INCPU_PRI_REMAP_INTPRI0_TO_VAL_MASK << (intPri << 2);

    if ((retVal = rtl8373_getAsicRegBits(regAddr, bitsMask, pNewPri)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}



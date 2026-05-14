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
#include <cpuTag.h>
#include <string.h>

#include <dal/dal_mgmt.h>

/* Function Name:
 *      rtk_cpu_externalCpuPort_set
 * Description:
 *      Set external cpu port
 * Input:
 *      port                -port number
 * Output:
 *      None     - 
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *     None
 */
rtk_api_ret_t rtk_cpu_externalCpuPort_set(rtk_uint32 port)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->cpuTag_externalCpuPort_set)
        return RT_ERR_DRIVER_NOT_FOUND;
    
    RTK_API_LOCK();
    retVal = RT_MAPPER->cpuTag_externalCpuPort_set(port);    
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_cpu_externalCpuPort_get
 * Description:
 *      Get external cpu port
 * Input:
 *      None
 * Output:
 *      pPort     - port number
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *     None
 */
rtk_api_ret_t rtk_cpu_externalCpuPort_get(rtk_uint32 *pPort)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->cpuTag_externalCpuPort_get)
        return RT_ERR_DRIVER_NOT_FOUND;
    
    RTK_API_LOCK();
    retVal = RT_MAPPER->cpuTag_externalCpuPort_get(pPort);    
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_cpuTag_tpid_set
 * Description:
 *      Set cpu tag protocol id; default:0x8899
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
rtk_api_ret_t rtk_cpuTag_tpid_set(rtk_uint32 tpid)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->cpuTag_tpid_set)
        return RT_ERR_DRIVER_NOT_FOUND;
    
    RTK_API_LOCK();
    retVal = RT_MAPPER->cpuTag_tpid_set(tpid);    
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_cpuTag_tpid_get
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
rtk_api_ret_t rtk_cpuTag_tpid_get(rtk_uint32 *pTpid)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->cpuTag_tpid_get)
        return RT_ERR_DRIVER_NOT_FOUND;
    
    RTK_API_LOCK();
    retVal = RT_MAPPER->cpuTag_tpid_get(pTpid);    
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_cpuTag_enable_set
 * Description:
 *      Set internal CPU port & external CPU port function enable/disable.
 * Input:
 *       status - CPU port function status: enable or disable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameter.
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      The API can set internal CPU port & external CPU port function enable/disable.
 */
rtk_api_ret_t rtk_cpuTag_enable_set(rtk_cpu_type_t type,  rtk_enable_t  status)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->cpuTag_enable_set)
        return RT_ERR_DRIVER_NOT_FOUND;
    
    RTK_API_LOCK();
    retVal = RT_MAPPER->cpuTag_enable_set(type, status);    
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_cpuTag_enable_get
 * Description:
 *      Get internal CPU port & external CPU port status setting.
 * Input:
 *      None
 * Output:
 *      pStatus - CPU port function status
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_INPUT            - Invalid input parameters.
 *      RT_ERR_L2_NO_CPU_PORT   - CPU port is not exist
 * Note:
 *      The API can get internal CPU port & external CPU port  function enable/disable.
 */
rtk_api_ret_t rtk_cpuTag_enable_get(rtk_cpu_type_t type,  rtk_enable_t *pStatus)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->cpuTag_enable_get)
        return RT_ERR_DRIVER_NOT_FOUND;
    
    RTK_API_LOCK();
    retVal = RT_MAPPER->cpuTag_enable_get(type, pStatus);    
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_cpu_tagPort_set
 * Description:
 *      Set internal & external CPU tag insert mode.
 * Input:
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
 *      The API can set CPU port inserting proprietary CPU tag mode (Length/Type 0x8899)
 *      to the frame that transmitting to CPU port.
 *      The inset cpu tag mode is as following:
 *      - CPU_INSERT_TO_ALL
 *      - CPU_INSERT_TO_TRAPPING
 *      - CPU_INSERT_TO_NONE
 */
rtk_api_ret_t rtk_cpuTag_insertMode_set(rtk_cpu_type_t type,  rtk_cpuTag_insertMode_t mode)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->cpuTag_insertMode_set)
        return RT_ERR_DRIVER_NOT_FOUND;
    RTK_API_LOCK();
    retVal = RT_MAPPER->cpuTag_insertMode_set(type, mode);    
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_cpuTag_insertMode_get
 * Description:
 *      Get CPU  tag insert mode.
 * Input:
 *      None
 * Output:
 *      pMode - CPU tag insert for packets egress from CPU port, 0:all insert 1:Only for trapped packets 2:no insert.
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_INPUT            - Invalid input parameters.
 *      RT_ERR_L2_NO_CPU_PORT   - CPU port is not exist
 * Note:
 *      The API can get configured CPU tag insert mode.
 *      The inset cpu tag mode is as following:
 *      - CPU_INSERT_TO_ALL
 *      - CPU_INSERT_TO_TRAPPING
 *      - CPU_INSERT_TO_NONE
 */
rtk_api_ret_t rtk_cpuTag_insertMode_get(rtk_cpu_type_t type,  rtk_cpuTag_insertMode_t *pMode)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->cpuTag_insertMode_get)
        return RT_ERR_DRIVER_NOT_FOUND;
    
    RTK_API_LOCK();
    retVal = RT_MAPPER->cpuTag_insertMode_get(type, pMode);    
    RTK_API_UNLOCK();

    return retVal;
}


/* Function Name:
 *      rtk_cpuTag_awarePort_set
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
rtk_api_ret_t rtk_cpuTag_awarePort_set(rtk_portmask_t *pPortmask)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->cpuTag_awarePort_set)
        return RT_ERR_DRIVER_NOT_FOUND;
    
    RTK_API_LOCK();
    retVal = RT_MAPPER->cpuTag_awarePort_set(pPortmask);    
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_cpuTag_awarePort_get
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
rtk_api_ret_t rtk_cpuTag_awarePort_get(rtk_portmask_t *pPortmask)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->cpuTag_awarePort_get)
        return RT_ERR_DRIVER_NOT_FOUND;
    
    RTK_API_LOCK();
    retVal = RT_MAPPER->cpuTag_awarePort_get(pPortmask);    
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_cpuTag_priRemap_set
 * Description:
 *      Configure CPU priorities mapping to internal absolute priority.
 * Input:
 *      type        -cpu type: internal or external cpu
 *      int_pri     - internal priority value.
 *      new_pri    - new internal priority value.
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
rtk_api_ret_t rtk_cpuTag_priRemap_set(rtk_cpu_type_t type, rtk_pri_t intPri, rtk_pri_t newPri)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->cpuTag_priRemap_set)
        return RT_ERR_DRIVER_NOT_FOUND;
    
    RTK_API_LOCK();
    retVal = RT_MAPPER->cpuTag_priRemap_set(type, intPri, newPri);    
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_cpuTag_priRemap_get
 * Description:
 *      Configure CPU priorities mapping to internal absolute priority.
 * Input:
 *      type        -cpu type: internal or external cpu
 *      int_pri     - internal priority value.
 * Output:
 *      pNew_pri    - new internal priority value.
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
rtk_api_ret_t rtk_cpuTag_priRemap_get(rtk_cpu_type_t type, rtk_pri_t intPri, rtk_pri_t *pNewPri)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->cpuTag_priRemap_get)
        return RT_ERR_DRIVER_NOT_FOUND;
    
    RTK_API_LOCK();
    retVal = RT_MAPPER->cpuTag_priRemap_get(type, intPri, pNewPri);    
    RTK_API_UNLOCK();

    return retVal;
}



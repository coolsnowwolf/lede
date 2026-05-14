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
 * Purpose : RTL8373 switch high-level API
 *
 * Feature : The file includes CPU module high-layer API defination
 *
 */

#ifndef  __DAL_RTL8373_CPU_H__
#define  __DAL_RTL8373_CPU_H__

#include <cpuTag.h>

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
extern rtk_api_ret_t dal_rtl8373_cpuTag_externalCpuPort_set(rtk_uint32 extCpuPort);

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
extern rtk_api_ret_t dal_rtl8373_cpuTag_externalCpuPort_get(rtk_uint32 *pExtCpuPort);

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
extern rtk_api_ret_t dal_rtl8373_cpuTag_tpid_set(rtk_uint32 tpid);

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
extern rtk_api_ret_t dal_rtl8373_cpuTag_tpid_get(rtk_uint32 *pTpid);

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
extern rtk_api_ret_t dal_rtl8373_cpuTag_enable_set(rtk_cpu_type_t type,  rtk_enable_t status);

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
extern rtk_api_ret_t dal_rtl8373_cpuTag_enable_get(rtk_cpu_type_t type,  rtk_enable_t *pStatus);

/* Function Name:
 *      dal_rtl8373_cpuTag_insertMode_set
 * Description:
 *      Set internal & external CPU port tag insert mode.
 * Input:
 *      type - CPU type: internal cpu or external cpu
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
extern rtk_api_ret_t dal_rtl8373_cpuTag_insertMode_set(rtk_cpu_type_t type,  rtk_cpuTag_insertMode_t mode);

/* Function Name:
 *      dal_rtl8373_cpuTag_insertMode_get
 * Description:
 *      Get internal & external CPU port tag insert mode.
 * Input:
 *      type - CPU type: internal cpu or external cpu
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
extern rtk_api_ret_t dal_rtl8373_cpuTag_insertMode_get(rtk_cpu_type_t type,  rtk_cpuTag_insertMode_t *pMode);

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
extern rtk_api_ret_t dal_rtl8373_cpuTag_awarePort_set(rtk_portmask_t *pPortmask);

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
extern rtk_api_ret_t dal_rtl8373_cpuTag_awarePort_get(rtk_portmask_t *pPortmask);

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
extern rtk_api_ret_t dal_rtl8373_cpuTag_priRemap_set(rtk_cpu_type_t type, rtk_pri_t intPri, rtk_pri_t newPri);
   
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
extern rtk_api_ret_t dal_rtl8373_cpuTag_priRemap_get(rtk_cpu_type_t type, rtk_pri_t intPri, rtk_pri_t *pNewPri);


#endif /*  __DAL_RTL8373_CPU_H__ */

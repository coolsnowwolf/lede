/*
 * Copyright (C) 2013 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * Unless you and Realtek execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2,
 * available at https://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 *
 * Purpose : RTL8367/RTL8367C switch high-level API
 *
 * Feature : The file includes CPU module high-layer API defination
 *
 */

#ifndef __RTK_API_CPU_H__
#define __RTK_API_CPU_H__


/*
 * Data Type Declaration
 */
typedef enum rtk_cpu_insert_e
{
    CPU_INSERT_TO_ALL = 0,
    CPU_INSERT_TO_TRAPPING,
    CPU_INSERT_TO_NONE,
    CPU_INSERT_END
}rtk_cpu_insert_t;

typedef enum rtk_cpu_position_e
{
    CPU_POS_AFTER_SA = 0,
    CPU_POS_BEFORE_CRC,
    CPU_POS_END
}rtk_cpu_position_t;

typedef enum rtk_cpu_tag_length_e
{
    CPU_LEN_8BYTES = 0,
    CPU_LEN_4BYTES,
    CPU_LEN_END
}rtk_cpu_tag_length_t;


typedef enum rtk_cpu_rx_length_e
{
    CPU_RX_72BYTES = 0,
    CPU_RX_64BYTES,
    CPU_RX_END
}rtk_cpu_rx_length_t;


/* Function Name:
 *      rtk_cpu_enable_set
 * Description:
 *      Set CPU port function enable/disable.
 * Input:
 *      enable - CPU port function enable
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
extern rtk_api_ret_t rtk_cpu_enable_set(rtk_enable_t enable);

/* Function Name:
 *      rtk_cpu_enable_get
 * Description:
 *      Get CPU port and its setting.
 * Input:
 *      None
 * Output:
 *      pEnable - CPU port function enable
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_INPUT            - Invalid input parameters.
 *      RT_ERR_L2_NO_CPU_PORT   - CPU port is not exist
 * Note:
 *      The API can get CPU port function enable/disable.
 */
extern rtk_api_ret_t rtk_cpu_enable_get(rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_cpu_tagPort_set
 * Description:
 *      Set CPU port and CPU tag insert mode.
 * Input:
 *      port - Port id.
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
 *      The API can set CPU port and inserting proprietary CPU tag mode (Length/Type 0x8899)
 *      to the frame that transmitting to CPU port.
 *      The inset cpu tag mode is as following:
 *      - CPU_INSERT_TO_ALL
 *      - CPU_INSERT_TO_TRAPPING
 *      - CPU_INSERT_TO_NONE
 */
extern rtk_api_ret_t rtk_cpu_tagPort_set(rtk_port_t port, rtk_cpu_insert_t mode);

/* Function Name:
 *      rtk_cpu_tagPort_get
 * Description:
 *      Get CPU port and CPU tag insert mode.
 * Input:
 *      None
 * Output:
 *      pPort - Port id.
 *      pMode - CPU tag insert for packets egress from CPU port, 0:all insert 1:Only for trapped packets 2:no insert.
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_INPUT            - Invalid input parameters.
 *      RT_ERR_L2_NO_CPU_PORT   - CPU port is not exist
 * Note:
 *      The API can get configured CPU port and its setting.
 *      The inset cpu tag mode is as following:
 *      - CPU_INSERT_TO_ALL
 *      - CPU_INSERT_TO_TRAPPING
 *      - CPU_INSERT_TO_NONE
 */
extern rtk_api_ret_t rtk_cpu_tagPort_get(rtk_port_t *pPort, rtk_cpu_insert_t *pMode);

/* Function Name:
 *      rtk_cpu_awarePort_set
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
extern rtk_api_ret_t rtk_cpu_awarePort_set(rtk_portmask_t *pPortmask);


/* Function Name:
 *      rtk_cpu_awarePort_get
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
extern rtk_api_ret_t rtk_cpu_awarePort_get(rtk_portmask_t *pPortmask);

/* Function Name:
 *      rtk_cpu_tagPosition_set
 * Description:
 *      Set CPU tag position.
 * Input:
 *      position - CPU tag position.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT      - Invalid input.
 * Note:
 *      The API can set CPU tag position.
 */
extern rtk_api_ret_t rtk_cpu_tagPosition_set(rtk_cpu_position_t position);

/* Function Name:
 *      rtk_cpu_tagPosition_get
 * Description:
 *      Get CPU tag position.
 * Input:
 *      None
 * Output:
 *      pPosition - CPU tag position.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT      - Invalid input.
 * Note:
 *      The API can get CPU tag position.
 */
extern rtk_api_ret_t rtk_cpu_tagPosition_get(rtk_cpu_position_t *pPosition);

/* Function Name:
 *      rtk_cpu_tagLength_set
 * Description:
 *      Set CPU tag length.
 * Input:
 *      length - CPU tag length.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT      - Invalid input.
 * Note:
 *      The API can set CPU tag length.
 */
extern rtk_api_ret_t rtk_cpu_tagLength_set(rtk_cpu_tag_length_t length);

/* Function Name:
 *      rtk_cpu_tagLength_get
 * Description:
 *      Get CPU tag length.
 * Input:
 *      None
 * Output:
 *      pLength - CPU tag length.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT      - Invalid input.
 * Note:
 *      The API can get CPU tag length.
 */
extern rtk_api_ret_t rtk_cpu_tagLength_get(rtk_cpu_tag_length_t *pLength);

/* Function Name:
 *      rtk_cpu_acceptLength_set
 * Description:
 *      Set CPU accept  length.
 * Input:
 *      length - CPU tag length.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT      - Invalid input.
 * Note:
 *      The API can set CPU accept length.
 */
extern rtk_api_ret_t rtk_cpu_acceptLength_set(rtk_cpu_rx_length_t length);

/* Function Name:
 *      rtk_cpu_acceptLength_get
 * Description:
 *      Get CPU accept length.
 * Input:
 *      None
 * Output:
 *      pLength - CPU tag length.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT      - Invalid input.
 * Note:
 *      The API can get CPU accept length.
 */
extern rtk_api_ret_t rtk_cpu_acceptLength_get(rtk_cpu_rx_length_t *pLength);

/* Function Name:
 *      rtk_cpu_priRemap_set
 * Description:
 *      Configure CPU priorities mapping to internal absolute priority.
 * Input:
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
extern rtk_api_ret_t rtk_cpu_priRemap_set(rtk_pri_t int_pri, rtk_pri_t new_pri);

/* Function Name:
 *      rtk_cpu_priRemap_get
 * Description:
 *      Configure CPU priorities mapping to internal absolute priority.
 * Input:
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
extern rtk_api_ret_t rtk_cpu_priRemap_get(rtk_pri_t int_pri, rtk_pri_t *pNew_pri);


#endif /* __RTK_API_CPU_H__ */

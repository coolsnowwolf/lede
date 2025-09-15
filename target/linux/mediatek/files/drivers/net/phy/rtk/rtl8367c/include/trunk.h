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
 * Feature : The file includes Trunk module high-layer TRUNK defination
 *
 */

#ifndef __RTK_API_TRUNK_H__
#define __RTK_API_TRUNK_H__

/*
 * Data Type Declaration
 */
#define    RTK_TRUNK_DPORT_HASH_MASK     0x40
#define    RTK_TRUNK_SPORT_HASH_MASK     0x20
#define    RTK_TRUNK_DIP_HASH_MASK       0x10
#define    RTK_TRUNK_SIP_HASH_MASK       0x8
#define    RTK_TRUNK_DMAC_HASH_MASK      0x4
#define    RTK_TRUNK_SMAC_HASH_MASK      0x2
#define    RTK_TRUNK_SPA_HASH_MASK       0x1


#define RTK_MAX_NUM_OF_TRUNK_HASH_VAL               16

typedef struct  rtk_trunk_hashVal2Port_s
{
    rtk_uint8 value[RTK_MAX_NUM_OF_TRUNK_HASH_VAL];
} rtk_trunk_hashVal2Port_t;

typedef enum rtk_trunk_group_e
{
    TRUNK_GROUP0 = 0,
    TRUNK_GROUP1,
    TRUNK_GROUP2,
    TRUNK_GROUP3,
    TRUNK_GROUP_END
} rtk_trunk_group_t;

typedef enum rtk_trunk_separateType_e
{
    SEPARATE_NONE = 0,
    SEPARATE_FLOOD,
    SEPARATE_END

} rtk_trunk_separateType_t;

typedef enum rtk_trunk_mode_e
{
    TRUNK_MODE_NORMAL = 0,
    TRUNK_MODE_DUMB,
    TRUNK_MODE_END
} rtk_trunk_mode_t;

/* Function Name:
 *      rtk_trunk_port_set
 * Description:
 *      Set trunking group available port mask
 * Input:
 *      trk_gid                 - trunk group id
 *      pTrunk_member_portmask  - Logic trunking member port mask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_LA_TRUNK_ID  - Invalid trunking group
 *      RT_ERR_PORT_MASK    - Invalid portmask.
 * Note:
 *      The API can set port trunking group port mask. Each port trunking group has max 4 ports.
 *      If enabled port mask has less than 2 ports available setting, then this trunking group function is disabled.
 */
extern rtk_api_ret_t rtk_trunk_port_set(rtk_trunk_group_t trk_gid, rtk_portmask_t *pTrunk_member_portmask);

/* Function Name:
 *      rtk_trunk_port_get
 * Description:
 *      Get trunking group available port mask
 * Input:
 *      trk_gid - trunk group id
 * Output:
 *      pTrunk_member_portmask - Logic trunking member port mask
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_LA_TRUNK_ID  - Invalid trunking group
 * Note:
 *      The API can get 2 port trunking group.
 */
extern rtk_api_ret_t rtk_trunk_port_get(rtk_trunk_group_t trk_gid, rtk_portmask_t *pTrunk_member_portmask);

/* Function Name:
 *      rtk_trunk_distributionAlgorithm_set
 * Description:
 *      Set port trunking hash select sources
 * Input:
 *      trk_gid         - trunk group id
 *      algo_bitmask    - Bitmask of the distribution algorithm
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_LA_TRUNK_ID  - Invalid trunking group
 *      RT_ERR_LA_HASHMASK  - Hash algorithm selection error.
 *      RT_ERR_PORT_MASK    - Invalid portmask.
 * Note:
 *      The API can set port trunking hash algorithm sources.
 *      7 bits mask for link aggregation group0 hash parameter selection {DIP, SIP, DMAC, SMAC, SPA}
 *      - 0b0000001: SPA
 *      - 0b0000010: SMAC
 *      - 0b0000100: DMAC
 *      - 0b0001000: SIP
 *      - 0b0010000: DIP
 *      - 0b0100000: TCP/UDP Source Port
 *      - 0b1000000: TCP/UDP Destination Port
 *      Example:
 *      - 0b0000011: SMAC & SPA
 *      - Note that it could be an arbitrary combination or independent set
 */
extern rtk_api_ret_t rtk_trunk_distributionAlgorithm_set(rtk_trunk_group_t trk_gid, rtk_uint32 algo_bitmask);

/* Function Name:
 *      rtk_trunk_distributionAlgorithm_get
 * Description:
 *      Get port trunking hash select sources
 * Input:
 *      trk_gid - trunk group id
 * Output:
 *      pAlgo_bitmask -  Bitmask of the distribution algorithm
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_LA_TRUNK_ID  - Invalid trunking group
 * Note:
 *      The API can get port trunking hash algorithm sources.
 */
extern rtk_api_ret_t rtk_trunk_distributionAlgorithm_get(rtk_trunk_group_t trk_gid, rtk_uint32 *pAlgo_bitmask);

/* Function Name:
 *      rtk_trunk_trafficSeparate_set
 * Description:
 *      Set the traffic separation setting of a trunk group from the specified device.
 * Input:
 *      trk_gid      - trunk group id
 *      separateType     - traffic separation setting
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID     - invalid unit id
 *      RT_ERR_LA_TRUNK_ID - invalid trunk ID
 *      RT_ERR_LA_HASHMASK - invalid hash mask
 * Note:
 *      SEPARATE_NONE: disable traffic separation
 *      SEPARATE_FLOOD: trunk MSB link up port is dedicated to TX flooding (L2 lookup miss) traffic
 */
extern rtk_api_ret_t rtk_trunk_trafficSeparate_set(rtk_trunk_group_t trk_gid, rtk_trunk_separateType_t separateType);

/* Function Name:
 *      rtk_trunk_trafficSeparate_get
 * Description:
 *      Get the traffic separation setting of a trunk group from the specified device.
 * Input:
 *      trk_gid        - trunk group id
 * Output:
 *      pSeparateType   - pointer separated traffic type
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_LA_TRUNK_ID  - invalid trunk ID
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      SEPARATE_NONE: disable traffic separation
 *      SEPARATE_FLOOD: trunk MSB link up port is dedicated to TX flooding (L2 lookup miss) traffic
 */
extern rtk_api_ret_t rtk_trunk_trafficSeparate_get(rtk_trunk_group_t trk_gid, rtk_trunk_separateType_t *pSeparateType);


/* Function Name:
 *      rtk_trunk_mode_set
 * Description:
 *      Set the trunk mode to the specified device.
 * Input:
 *      mode - trunk mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_INPUT   - invalid input parameter
 * Note:
 *      The enum of the trunk mode as following
 *      - TRUNK_MODE_NORMAL
 *      - TRUNK_MODE_DUMB
 */
extern rtk_api_ret_t rtk_trunk_mode_set(rtk_trunk_mode_t mode);

/* Function Name:
 *      rtk_trunk_mode_get
 * Description:
 *      Get the trunk mode from the specified device.
 * Input:
 *      None
 * Output:
 *      pMode - pointer buffer of trunk mode
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      The enum of the trunk mode as following
 *      - TRUNK_MODE_NORMAL
 *      - TRUNK_MODE_DUMB
 */
extern rtk_api_ret_t rtk_trunk_mode_get(rtk_trunk_mode_t *pMode);

/* Function Name:
 *      rtk_trunk_trafficPause_set
 * Description:
 *      Set the traffic pause setting of a trunk group.
 * Input:
 *      trk_gid      - trunk group id
 *      enable       - traffic pause state
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_LA_TRUNK_ID - invalid trunk ID
 * Note:
 *      None.
 */
extern rtk_api_ret_t rtk_trunk_trafficPause_set(rtk_trunk_group_t trk_gid, rtk_enable_t enable);

/* Function Name:
 *      rtk_trunk_trafficPause_get
 * Description:
 *      Get the traffic pause setting of a trunk group.
 * Input:
 *      trk_gid        - trunk group id
 * Output:
 *      pEnable        - pointer of traffic pause state.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_LA_TRUNK_ID  - invalid trunk ID
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None.
 */
extern rtk_api_ret_t rtk_trunk_trafficPause_get(rtk_trunk_group_t trk_gid, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_trunk_hashMappingTable_set
 * Description:
 *      Set hash value to port array in the trunk group id from the specified device.
 * Input:
 *      trk_gid          - trunk group id
 *      pHash2Port_array - ports associate with the hash value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID            - invalid unit id
 *      RT_ERR_LA_TRUNK_ID        - invalid trunk ID
 *      RT_ERR_NULL_POINTER       - input parameter may be null pointer
 *      RT_ERR_LA_TRUNK_NOT_EXIST - the trunk doesn't exist
 *      RT_ERR_LA_NOT_MEMBER_PORT - the port is not a member port of the trunk
 *      RT_ERR_LA_CPUPORT         - CPU port can not be aggregated port
 * Note:
 *      Trunk group 0 & 1 shares the same hash mapping table.
 *      Trunk group 2 uses a independent table.
 */
extern rtk_api_ret_t rtk_trunk_hashMappingTable_set(rtk_trunk_group_t trk_gid, rtk_trunk_hashVal2Port_t *pHash2Port_array);

/* Function Name:
 *      rtk_trunk_hashMappingTable_get
 * Description:
 *      Get hash value to port array in the trunk group id from the specified device.
 * Input:
 *      trk_gid          - trunk group id
 * Output:
 *      pHash2Port_array - pointer buffer of ports associate with the hash value
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_LA_TRUNK_ID  - invalid trunk ID
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      Trunk group 0 & 1 shares the same hash mapping table.
 *      Trunk group 2 uses a independent table.
 */
extern rtk_api_ret_t rtk_trunk_hashMappingTable_get(rtk_trunk_group_t trk_gid, rtk_trunk_hashVal2Port_t *pHash2Port_array);

/* Function Name:
 *      rtk_trunk_portQueueEmpty_get
 * Description:
 *      Get the port mask which all queues are empty.
 * Input:
 *      None.
 * Output:
 *      pEmpty_portmask   - pointer empty port mask
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None.
 */
extern rtk_api_ret_t rtk_trunk_portQueueEmpty_get(rtk_portmask_t *pEmpty_portmask);

#endif /* __RTK_API_TRUNK_H__ */

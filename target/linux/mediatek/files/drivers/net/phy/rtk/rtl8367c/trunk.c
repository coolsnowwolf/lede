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
 * Purpose : RTK switch high-level API for RTL8367/RTL8367C
 * Feature : Here is a list of all functions and variables in Trunk module.
 *
 */

#include <rtk_switch.h>
#include <rtk_error.h>
#include <trunk.h>
#include <string.h>

#include <rtl8367c_asicdrv.h>
#include <rtl8367c_asicdrv_trunking.h>

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
rtk_api_ret_t rtk_trunk_port_set(rtk_trunk_group_t trk_gid, rtk_portmask_t *pTrunk_member_portmask)
{
    rtk_api_ret_t retVal;
    rtk_uint32 pmsk;
    rtk_uint32 regValue, type, tmp;


    if((retVal = rtl8367c_setAsicReg(0x13C2, 0x0249)) != RT_ERR_OK)
        return retVal;

    if((retVal = rtl8367c_getAsicReg(0x1300, &regValue)) != RT_ERR_OK)
        return retVal;

    if((retVal = rtl8367c_setAsicReg(0x13C2, 0x0000)) != RT_ERR_OK)
        return retVal;

    switch (regValue)
    {
        case 0x0276:
        case 0x0597:
        case 0x6367:
            type = 0;
            break;
        case 0x0652:
        case 0x6368:
            type = 1;
            break;
        case 0x0801:
        case 0x6511:
            type = 2;
            break;
        default:
            return RT_ERR_FAILED;
    }

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Trunk Group Valid */
    RTK_CHK_TRUNK_GROUP_VALID(trk_gid);

    if(NULL == pTrunk_member_portmask)
        return RT_ERR_NULL_POINTER;

    RTK_CHK_PORTMASK_VALID(pTrunk_member_portmask);

    if((retVal = rtk_switch_portmask_L2P_get(pTrunk_member_portmask, &pmsk)) != RT_ERR_OK)
        return retVal;

    if((type == 0) || (type == 1))
    {
        if ((pmsk | RTL8367C_PORT_TRUNK_GROUP_MASK_MASK(trk_gid)) != (rtk_uint32)RTL8367C_PORT_TRUNK_GROUP_MASK_MASK(trk_gid))
            return RT_ERR_PORT_MASK;

        pmsk = (pmsk & RTL8367C_PORT_TRUNK_GROUP_MASK_MASK(trk_gid)) >> RTL8367C_PORT_TRUNK_GROUP_MASK_OFFSET(trk_gid);
    }
    else if(type == 2)
    {
        tmp = 0;

        if(pmsk & 0x2)
            tmp |= 1;
        if(pmsk & 0x8)
            tmp |=2;
        if(pmsk & 0x80)
            tmp |=8;

        pmsk = tmp;
    }

    if ((retVal = rtl8367c_setAsicTrunkingGroup(trk_gid, pmsk)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

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
rtk_api_ret_t rtk_trunk_port_get(rtk_trunk_group_t trk_gid, rtk_portmask_t *pTrunk_member_portmask)
{
    rtk_api_ret_t retVal;
    rtk_uint32 pmsk;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Trunk Group Valid */
    RTK_CHK_TRUNK_GROUP_VALID(trk_gid);

    if ((retVal = rtl8367c_getAsicTrunkingGroup(trk_gid, &pmsk)) != RT_ERR_OK)
        return retVal;

    pmsk = pmsk << RTL8367C_PORT_TRUNK_GROUP_MASK_OFFSET(trk_gid);

    if((retVal = rtk_switch_portmask_P2L_get(pmsk, pTrunk_member_portmask)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_trunk_distributionAlgorithm_set
 * Description:
 *      Set port trunking hash select sources
 * Input:
 *      trk_gid         - trunk group id
 *      algo_bitmask   - Bitmask of the distribution algorithm
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
rtk_api_ret_t rtk_trunk_distributionAlgorithm_set(rtk_trunk_group_t trk_gid, rtk_uint32 algo_bitmask)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (trk_gid != RTK_WHOLE_SYSTEM)
        return RT_ERR_LA_TRUNK_ID;

    if (algo_bitmask >= 128)
        return RT_ERR_LA_HASHMASK;

    if ((retVal = rtl8367c_setAsicTrunkingHashSelect(algo_bitmask)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

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
rtk_api_ret_t rtk_trunk_distributionAlgorithm_get(rtk_trunk_group_t trk_gid, rtk_uint32 *pAlgo_bitmask)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (trk_gid != RTK_WHOLE_SYSTEM)
        return RT_ERR_LA_TRUNK_ID;

    if(NULL == pAlgo_bitmask)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getAsicTrunkingHashSelect((rtk_uint32 *)pAlgo_bitmask)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

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
rtk_api_ret_t rtk_trunk_trafficSeparate_set(rtk_trunk_group_t trk_gid, rtk_trunk_separateType_t separateType)
{
    rtk_api_ret_t retVal;
    rtk_uint32 enabled;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (trk_gid != RTK_WHOLE_SYSTEM)
        return RT_ERR_LA_TRUNK_ID;

    if(separateType >= SEPARATE_END)
        return RT_ERR_INPUT;

    enabled = (separateType == SEPARATE_FLOOD) ? ENABLED : DISABLED;
    if ((retVal = rtl8367c_setAsicTrunkingFlood(enabled)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

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
rtk_api_ret_t rtk_trunk_trafficSeparate_get(rtk_trunk_group_t trk_gid, rtk_trunk_separateType_t *pSeparateType)
{
    rtk_api_ret_t retVal;
    rtk_uint32 enabled;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (trk_gid != RTK_WHOLE_SYSTEM)
        return RT_ERR_LA_TRUNK_ID;

    if(NULL == pSeparateType)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getAsicTrunkingFlood(&enabled)) != RT_ERR_OK)
        return retVal;

    *pSeparateType = (enabled == ENABLED) ? SEPARATE_FLOOD : SEPARATE_NONE;
    return RT_ERR_OK;
}

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
rtk_api_ret_t rtk_trunk_mode_set(rtk_trunk_mode_t mode)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(mode >= TRUNK_MODE_END)
        return RT_ERR_INPUT;

    if ((retVal = rtl8367c_setAsicTrunkingMode((rtk_uint32)mode)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

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
rtk_api_ret_t rtk_trunk_mode_get(rtk_trunk_mode_t *pMode)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pMode)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getAsicTrunkingMode((rtk_uint32 *)pMode)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

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
rtk_api_ret_t rtk_trunk_trafficPause_set(rtk_trunk_group_t trk_gid, rtk_enable_t enable)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Trunk Group Valid */
    RTK_CHK_TRUNK_GROUP_VALID(trk_gid);

    if(enable >= RTK_ENABLE_END)
        return RT_ERR_INPUT;

    if ((retVal = rtl8367c_setAsicTrunkingFc((rtk_uint32)trk_gid, (rtk_uint32)enable)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

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
rtk_api_ret_t rtk_trunk_trafficPause_get(rtk_trunk_group_t trk_gid, rtk_enable_t *pEnable)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Trunk Group Valid */
    RTK_CHK_TRUNK_GROUP_VALID(trk_gid);

    if(NULL == pEnable)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getAsicTrunkingFc((rtk_uint32)trk_gid, (rtk_uint32 *)pEnable)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

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
rtk_api_ret_t rtk_trunk_hashMappingTable_set(rtk_trunk_group_t trk_gid, rtk_trunk_hashVal2Port_t *pHash2Port_array)
{
    rtk_api_ret_t retVal;
    rtk_uint32 hashValue;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Trunk Group Valid */
    RTK_CHK_TRUNK_GROUP_VALID(trk_gid);

    if(NULL == pHash2Port_array)
        return RT_ERR_NULL_POINTER;

    if(trk_gid <= TRUNK_GROUP1)
    {
        for(hashValue = 0; hashValue < RTK_MAX_NUM_OF_TRUNK_HASH_VAL; hashValue++)
        {
            if ((retVal = rtl8367c_setAsicTrunkingHashTable(hashValue, pHash2Port_array->value[hashValue])) != RT_ERR_OK)
                return retVal;
        }
    }
    else
    {
        for(hashValue = 0; hashValue < RTK_MAX_NUM_OF_TRUNK_HASH_VAL; hashValue++)
        {
            if ((retVal = rtl8367c_setAsicTrunkingHashTable1(hashValue, pHash2Port_array->value[hashValue])) != RT_ERR_OK)
                return retVal;
        }
    }

    return RT_ERR_OK;
}

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
rtk_api_ret_t rtk_trunk_hashMappingTable_get(rtk_trunk_group_t trk_gid, rtk_trunk_hashVal2Port_t *pHash2Port_array)
{
    rtk_api_ret_t retVal;
    rtk_uint32 hashValue;
    rtk_uint32 hashPort;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Trunk Group Valid */
    RTK_CHK_TRUNK_GROUP_VALID(trk_gid);

    if(NULL == pHash2Port_array)
        return RT_ERR_NULL_POINTER;

    if(trk_gid <= TRUNK_GROUP1)
    {
        for(hashValue = 0; hashValue < RTK_MAX_NUM_OF_TRUNK_HASH_VAL; hashValue++)
        {
            if ((retVal = rtl8367c_getAsicTrunkingHashTable(hashValue, &hashPort)) != RT_ERR_OK)
                return retVal;

            pHash2Port_array->value[hashValue] = hashPort;
        }
    }
    else
    {
        for(hashValue = 0; hashValue < RTK_MAX_NUM_OF_TRUNK_HASH_VAL; hashValue++)
        {
            if ((retVal = rtl8367c_getAsicTrunkingHashTable1(hashValue, &hashPort)) != RT_ERR_OK)
                return retVal;

            pHash2Port_array->value[hashValue] = hashPort;
        }
    }

    return RT_ERR_OK;
}

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
rtk_api_ret_t rtk_trunk_portQueueEmpty_get(rtk_portmask_t *pEmpty_portmask)
{
    rtk_api_ret_t retVal;
    rtk_uint32 pmask;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pEmpty_portmask)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getAsicQeueuEmptyStatus(&pmask)) != RT_ERR_OK)
        return retVal;

    if ((retVal = rtk_switch_portmask_P2L_get(pmask, pEmpty_portmask)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}


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
 * Feature : Here is a list of all functions and variables in TRUNK module.
 *
 */

#include <rtk_switch.h>
#include <rtk_error.h>
#include <dal_rtl8373_trunk.h>
#include <rtl8373_asicdrv.h>
#include <string.h>



/* Function Name:
 *      dal_rtl8373_trunk_port_set
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
rtk_api_ret_t dal_rtl8373_trunk_port_set(rtk_trunk_group_t trk_gid, rtk_portmask_t *pTrunk_member_portmask)
{
    rtk_api_ret_t retVal;
    rtk_uint32 pmsk;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Trunk Group Valid */
    RTK_CHK_TRUNK_GROUP_VALID(trk_gid);

    if(NULL == pTrunk_member_portmask)
        return RT_ERR_NULL_POINTER;

    RTK_CHK_PORTMASK_VALID(pTrunk_member_portmask);

    if((retVal = rtk_switch_portmask_L2P_get(pTrunk_member_portmask, &pmsk)) != RT_ERR_OK)
        return retVal;

   
    if ((retVal = rtl8373_setAsicRegBits(RTL8373_TRK_MBR_CTRL_ADDR(trk_gid), RTL8373_TRK_MBR_CTRL_TRK_PMSK_MASK, pmsk)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}



/* Function Name:
 *      dal_rtl8373_trunk_port_get
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
rtk_api_ret_t dal_rtl8373_trunk_port_get(rtk_trunk_group_t trk_gid, rtk_portmask_t *pTrunk_member_portmask)
{
    rtk_api_ret_t retVal;
    rtk_uint32 pmsk;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Trunk Group Valid */
    RTK_CHK_TRUNK_GROUP_VALID(trk_gid);

    if ((retVal = rtl8373_getAsicRegBits(RTL8373_TRK_MBR_CTRL_ADDR(trk_gid), RTL8373_TRK_MBR_CTRL_TRK_PMSK_MASK, &pmsk)) != RT_ERR_OK)
        return retVal;
    

    if((retVal = rtk_switch_portmask_P2L_get(pmsk, pTrunk_member_portmask)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_trunk_distributionAlgorithm_set
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
rtk_api_ret_t dal_rtl8373_trunk_distributionAlgorithm_set(rtk_trunk_group_t trk_gid, rtk_uint32 algo_bitmask)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Trunk Group Valid */
    RTK_CHK_TRUNK_GROUP_VALID(trk_gid);

    if (algo_bitmask >= 128)
        return RT_ERR_LA_HASHMASK;

    if ((retVal = rtl8373_setAsicRegBits(RTL8373_TRK_HASH_CTRL_ADDR(trk_gid), RTL8373_TRK_HASH_CTRL_HASH_MSK_MASK, algo_bitmask)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_trunk_distributionAlgorithm_get
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
rtk_api_ret_t dal_rtl8373_trunk_distributionAlgorithm_get(rtk_trunk_group_t trk_gid, rtk_uint32 *pAlgo_bitmask)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Trunk Group Valid */
    RTK_CHK_TRUNK_GROUP_VALID(trk_gid);

    if(NULL == pAlgo_bitmask)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8373_getAsicRegBits(RTL8373_TRK_HASH_CTRL_ADDR(trk_gid), RTL8373_TRK_HASH_CTRL_HASH_MSK_MASK, pAlgo_bitmask)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}



/* Function Name:
 *      dal_rtl8373_trunk_trafficSeparate_set
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
rtk_api_ret_t dal_rtl8373_trunk_trafficSeparate_set(rtk_trunk_group_t trk_gid, rtk_trunk_separateType_t separateType)
{
    rtk_api_ret_t retVal;
    rtk_uint32 enabled;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

	if(trk_gid >= TRUNK_GROUP_END)
		return RT_ERR_INPUT;

    if(separateType >= SEPARATE_END)
        return RT_ERR_INPUT;

    enabled = (separateType == SEPARATE_FLOOD) ? ENABLED : DISABLED;

    if ((retVal = rtl8373_setAsicRegBit(RTL8373_TRK_CTRL_ADDR, RTL8373_TRK_CTRL_TRUNK_FLD_OFFSET, enabled)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_trunk_trafficSeparate_get
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
rtk_api_ret_t dal_rtl8373_trunk_trafficSeparate_get(rtk_trunk_group_t trk_gid, rtk_trunk_separateType_t *pSeparateType)
{
    rtk_api_ret_t retVal;
    rtk_uint32 enabled;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

	if(trk_gid >= TRUNK_GROUP_END)
		return RT_ERR_INPUT;

    if(NULL == pSeparateType)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8373_getAsicRegBit(RTL8373_TRK_CTRL_ADDR, RTL8373_TRK_CTRL_TRUNK_FLD_OFFSET, &enabled)) != RT_ERR_OK)
        return retVal;

    *pSeparateType = (enabled == ENABLED) ? SEPARATE_FLOOD : SEPARATE_NONE;
    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_trunk_trafficPause_set
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
rtk_api_ret_t dal_rtl8373_trunk_trafficPause_set(rtk_trunk_group_t trk_gid, rtk_enable_t enable)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Trunk Group Valid */
    RTK_CHK_TRUNK_GROUP_VALID(trk_gid);

    if(enable >= RTK_ENABLE_END)
        return RT_ERR_INPUT;

    if((retVal = rtl8373_setAsicRegBit(RTL8373_TRK_FLOW_CTRL_ADDR(trk_gid), RTL8373_TRK_FLOW_CTRL_TRK_FLCTRL_EN_OFFSET, enable)) != RT_ERR_OK)
         return retVal;


    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_trunk_trafficPause_get
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
rtk_api_ret_t dal_rtl8373_trunk_trafficPause_get(rtk_trunk_group_t trk_gid, rtk_enable_t *pEnable)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Trunk Group Valid */
    RTK_CHK_TRUNK_GROUP_VALID(trk_gid);

    if(NULL == pEnable)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8373_getAsicRegBit(RTL8373_TRK_FLOW_CTRL_ADDR(trk_gid), RTL8373_TRK_FLOW_CTRL_TRK_FLCTRL_EN_OFFSET, pEnable)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}


/* Function Name:
 *      dal_rtl8373_trunk_portQueueEmpty_get
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
rtk_api_ret_t dal_rtl8373_trunk_portQueueEmpty_get(rtk_portmask_t *pEmpty_portmask)
{
    rtk_api_ret_t retVal;
    rtk_uint32 pmask;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pEmpty_portmask)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8373_getAsicReg(RTL8373_TRK_QUEUE_EMPTY_ADDR, &pmask)) != RT_ERR_OK)
        return retVal;

    if ((retVal = rtk_switch_portmask_P2L_get(pmask, pEmpty_portmask)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}










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
 * Feature : Here is a list of all functions and variables in VLAN module.
 *

 */


#include <rtk_switch.h>
#include <rtk_error.h>
#include <vlan.h>
//#include <rate.h>
#include <string.h>

#include <dal/dal_mgmt.h>

/* Function Name:
 *      rtk_vlan_init
 * Description:
 *      Initialize VLAN.
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      VLAN is disabled by default. User has to call this API to enable VLAN before
 *      using it. And It will set a default VLAN(vid 1) including all ports and set
 *      all ports PVID to the default VLAN.
 */
rtk_api_ret_t rtk_vlan_init(void)
{
    rtk_api_ret_t retVal = 0;
    
    if (NULL == RT_MAPPER->vlan_init)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->vlan_init();
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_vlan_set
 * Description:
 *      Set a VLAN entry.
 * Input:
 *      vid - VLAN ID to configure.
 *      pVlanCfg - VLAN Configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_INPUT                - Invalid input parameters.
 *      RT_ERR_L2_FID               - Invalid FID.
 *      RT_ERR_VLAN_PORT_MBR_EXIST  - Invalid member port mask.
 *      RT_ERR_VLAN_VID             - Invalid VID parameter.
 * Note:
 *
 */
rtk_api_ret_t rtk_vlan_set(rtk_vlan_t vid, rtk_vlan_entry_t *pVlanCfg)
{
    rtk_api_ret_t retVal = 0;

    if (NULL == RT_MAPPER->vlan_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->vlan_set(vid, pVlanCfg);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_vlan_get
 * Description:
 *      Get a VLAN entry.
 * Input:
 *      vid - VLAN ID to configure.
 * Output:
 *      pVlanCfg - VLAN Configuration
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_VLAN_VID     - Invalid VID parameter.
 * Note:
 *
 */
rtk_api_ret_t rtk_vlan_get(rtk_vlan_t vid, rtk_vlan_entry_t *pVlanCfg)
{
    rtk_api_ret_t retVal = 0;
    
    if (NULL == RT_MAPPER->vlan_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->vlan_get(vid, pVlanCfg);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_vlan_egrFilterEnable_set
 * Description:
 *      Set VLAN egress filter.
 * Input:
 *      egrFilter - Egress filtering
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_ENABLE       - Invalid input parameters.
 * Note:
 *
 */
rtk_api_ret_t rtk_vlan_egrFilterEnable_set(rtk_enable_t egrFilter)
{
    rtk_api_ret_t retVal = 0;
    
    if (NULL == RT_MAPPER->vlan_egrFilterEnable_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->vlan_egrFilterEnable_set(egrFilter);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_vlan_egrFilterEnable_get
 * Description:
 *      Get VLAN egress filter.
 * Input:
 *      pEgrFilter - Egress filtering
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_NULL_POINTER - NULL Pointer.
 * Note:
 *
 */
rtk_api_ret_t rtk_vlan_egrFilterEnable_get(rtk_enable_t *pEgrFilter)
{
    rtk_api_ret_t retVal = 0;
    
    if (NULL == RT_MAPPER->vlan_egrFilterEnable_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->vlan_egrFilterEnable_get(pEgrFilter);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *     rtk_vlan_portPvid_set
 * Description:
 *      Set port to specified VLAN ID(PVID).
 * Input:
 *      port - Port id.
 *      pvid - Specified VLAN ID.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                   - OK
 *      RT_ERR_FAILED               - Failed
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_PORT_ID              - Invalid port number.
 *      RT_ERR_VLAN_PRIORITY        - Invalid priority.
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND - VLAN entry not found.
 *      RT_ERR_VLAN_VID             - Invalid VID parameter.
 * Note:
 *       The API is used for Port-based VLAN. The untagged frame received from the
 *       port will be classified to the specified VLAN and assigned to the specified priority.
 */
rtk_api_ret_t rtk_vlan_portPvid_set(rtk_port_t port, rtk_vlan_t pvid)
{
    rtk_api_ret_t retVal = 0;

    if (NULL == RT_MAPPER->vlan_portPvid_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->vlan_portPvid_set(port, pvid);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_vlan_portPvid_get
 * Description:
 *      Get VLAN ID(PVID) on specified port.
 * Input:
 *      port - Port id.
 * Output:
 *      pPvid - Specified VLAN ID.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *     The API can get the PVID and 802.1p priority for the PVID of Port-based VLAN.
 */
rtk_api_ret_t rtk_vlan_portPvid_get(rtk_port_t port, rtk_vlan_t *pPvid)
{
    rtk_api_ret_t retVal = 0;
    
    if (NULL == RT_MAPPER->vlan_portPvid_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->vlan_portPvid_get(port, pPvid);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_vlan_portIgrFilterEnable_set
 * Description:
 *      Set VLAN ingress for each port.
 * Input:
 *      port - Port id.
 *      igrFilter - VLAN ingress function enable status.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number
 *      RT_ERR_ENABLE       - Invalid enable input
 * Note:
 *      The status of vlan ingress filter is as following:
 *      - DISABLED
 *      - ENABLED
 *      While VLAN function is enabled, ASIC will decide VLAN ID for each received frame and get belonged member
 *      ports from VLAN table. If received port is not belonged to VLAN member ports, ASIC will drop received frame if VLAN ingress function is enabled.
 */
rtk_api_ret_t rtk_vlan_portIgrFilterEnable_set(rtk_port_t port, rtk_enable_t igrFilter)
{
    rtk_api_ret_t retVal = 0;
    
    if (NULL == RT_MAPPER->vlan_portIgrFilterEnable_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->vlan_portIgrFilterEnable_set(port, igrFilter);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_vlan_portIgrFilterEnable_get
 * Description:
 *      Get VLAN Ingress Filter
 * Input:
 *      port        - Port id.
 * Output:
 *      pIgFilter - VLAN ingress function enable status.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *     The API can Get the VLAN ingress filter status.
 *     The status of vlan ingress filter is as following:
 *     - DISABLED
 *     - ENABLED
 */
rtk_api_ret_t rtk_vlan_portIgrFilterEnable_get(rtk_port_t port, rtk_enable_t *pIgFilter)
{
    rtk_api_ret_t retVal = 0;
    
    if (NULL == RT_MAPPER->vlan_portIgrFilterEnable_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->vlan_portIgrFilterEnable_get(port, pIgFilter);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_vlan_portAcceptFrameType_set
 * Description:
 *      Set VLAN accept_frame_type
 * Input:
 *      port                - Port id.
 *      acceptFrameType   - accept frame type
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                       - OK
 *      RT_ERR_FAILED                   - Failed
 *      RT_ERR_SMI                      - SMI access error
 *      RT_ERR_PORT_ID                  - Invalid port number.
 *      RT_ERR_VLAN_ACCEPT_FRAME_TYPE   - Invalid frame type.
 * Note:
 *      The API is used for checking 802.1Q tagged frames.
 *      The accept frame type as following:
 *      - ACCEPT_FRAME_TYPE_ALL
 *      - ACCEPT_FRAME_TYPE_TAG_ONLY
 *      - ACCEPT_FRAME_TYPE_UNTAG_ONLY
 */
rtk_api_ret_t rtk_vlan_portAcceptFrameType_set(rtk_port_t port, rtk_vlan_acceptFrameType_t acceptFrameType)
{
    rtk_api_ret_t retVal = 0;
    
    if (NULL == RT_MAPPER->vlan_portAcceptFrameType_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->vlan_portAcceptFrameType_set(port, acceptFrameType);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_vlan_portAcceptFrameType_get
 * Description:
 *      Get VLAN accept_frame_type
 * Input:
 *      port - Port id.
 * Output:
 *      pAcceptFrameType - accept frame type
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *     The API can Get the VLAN ingress filter.
 *     The accept frame type as following:
 *     - ACCEPT_FRAME_TYPE_ALL
 *     - ACCEPT_FRAME_TYPE_TAG_ONLY
 *     - ACCEPT_FRAME_TYPE_UNTAG_ONLY
 */
rtk_api_ret_t rtk_vlan_portAcceptFrameType_get(rtk_port_t port, rtk_vlan_acceptFrameType_t *pAcceptFrameType)
{
    rtk_api_ret_t retVal = 0;
    
    if (NULL == RT_MAPPER->vlan_portAcceptFrameType_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->vlan_portAcceptFrameType_get(port, pAcceptFrameType);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_vlan_tagMode_set
 * Description:
 *      Set CVLAN egress tag mode
 * Input:
 *      port        - Port id.
 *      tagMode    - The egress tag mode.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_INPUT        - Invalid input parameter.
 *      RT_ERR_ENABLE       - Invalid enable input.
 * Note:
 *      The API can set Egress tag mode. There are 4 mode for egress tag:
 *      - VLAN_EGRESS_TAG_MODE_ORIGINAL,
 *      - VLAN_EGRESS_TAG_MODE_KEEP_FORMAT,
 *      - VLAN_EGRESS_TAG_MODE_PRI.
 *      - VLAN_EGRESS_TAG_MODE_REAL_KEEP,
 */
rtk_api_ret_t rtk_vlan_tagMode_set(rtk_port_t port, rtk_vlan_egressTagMode_t tagMode)
{
    rtk_api_ret_t retVal = 0;
    
    if (NULL == RT_MAPPER->vlan_tagMode_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->vlan_tagMode_set(port, tagMode);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_vlan_tagMode_get
 * Description:
 *      Get CVLAN egress tag mode
 * Input:
 *      port - Port id.
 * Output:
 *      pTagMode - The egress tag mode.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      The API can get Egress tag mode. There are 4 mode for egress tag:
 *      - VLAN_EGRESS_TAG_MODE_ORIGINAL,
 *      - VLAN_EGRESS_TAG_MODE_KEEP_FORMAT,
 *      - VLAN_EGRESS_TAG_MODE_PRI.
 *      - VLAN_EGRESS_TAG_MODE_REAL_KEEP,
 */
rtk_api_ret_t rtk_vlan_tagMode_get(rtk_port_t port, rtk_vlan_egressTagMode_t *pTagMode)
{
    rtk_api_ret_t retVal = 0;

    if (NULL == RT_MAPPER->vlan_tagMode_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->vlan_tagMode_get(port, pTagMode);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_vlan_transparent_set
 * Description:
 *      Set VLAN transparent mode
 * Input:
 *      egrPort        - Egress Port id.
 *      pIgrPmsk      - Ingress Port Mask.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      None.
 */
rtk_api_ret_t rtk_vlan_transparent_set(rtk_port_t egrPort, rtk_portmask_t *pIgrPmsk)
{
    rtk_api_ret_t retVal = 0;
    
    if (NULL == RT_MAPPER->vlan_transparent_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->vlan_transparent_set(egrPort, pIgrPmsk);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_vlan_transparent_get
 * Description:
 *      Get VLAN transparent mode
 * Input:
 *      egrPort        - Egress Port id.
 * Output:
 *      pIgrPmsk      - Ingress Port Mask
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      None.
 */
rtk_api_ret_t rtk_vlan_transparent_get(rtk_port_t egrPort, rtk_portmask_t *pIgrPmsk)
{
    rtk_api_ret_t retVal = 0;
    
    if (NULL == RT_MAPPER->vlan_transparent_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->vlan_transparent_get(egrPort, pIgrPmsk);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_vlan_keep_set
 * Description:
 *      Set VLAN egress keep mode
 * Input:
 *      egrPort        - Egress Port id.
 *      pIgrPmsk      - Ingress Port Mask.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      None.
 */
rtk_api_ret_t rtk_vlan_keep_set(rtk_port_t egrPort, rtk_portmask_t *pIgrPmsk)
{
    rtk_api_ret_t retVal = 0;
    
    if (NULL == RT_MAPPER->vlan_keep_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->vlan_keep_set(egrPort, pIgrPmsk);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_vlan_keep_get
 * Description:
 *      Get VLAN egress keep mode
 * Input:
 *      egrPort        - Egress Port id.
 * Output:
 *      pIgrPmsk      - Ingress Port Mask
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      None.
 */
rtk_api_ret_t rtk_vlan_keep_get(rtk_port_t egrPort, rtk_portmask_t *pIgrPmsk)
{
    rtk_api_ret_t retVal = 0;

    if (NULL == RT_MAPPER->vlan_keep_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->vlan_keep_get(egrPort, pIgrPmsk);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_vlan_stg_set
 * Description:
 *      Set spanning tree group instance of the vlan to the specified device
 * Input:
 *      vid - Specified VLAN ID.
 *      stg - spanning tree group instance.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_MSTI         - Invalid msti parameter
 *      RT_ERR_INPUT        - Invalid input parameter.
 *      RT_ERR_VLAN_VID     - Invalid VID parameter.
 * Note:
 *      The API can set spanning tree group instance of the vlan to the specified device.
 */
rtk_api_ret_t rtk_vlan_stg_set(rtk_vlan_t vid, rtk_stp_msti_id_t stg)
{
    rtk_api_ret_t retVal = 0;
    
    if (NULL == RT_MAPPER->vlan_stg_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->vlan_stg_set(vid, stg);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_vlan_stg_get
 * Description:
 *      Get spanning tree group instance of the vlan to the specified device
 * Input:
 *      vid - Specified VLAN ID.
 * Output:
 *      pStg - spanning tree group instance.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_VLAN_VID     - Invalid VID parameter.
 * Note:
 *      The API can get spanning tree group instance of the vlan to the specified device.
 */
rtk_api_ret_t rtk_vlan_stg_get(rtk_vlan_t vid, rtk_stp_msti_id_t *pStg)
{
    rtk_api_ret_t retVal = 0;
    
    if (NULL == RT_MAPPER->vlan_stg_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->vlan_stg_get(vid, pStg);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_vlan_portFid_set
 * Description:
 *      Set port-based filtering database
 * Input:
 *      port - Port id.
 *      enable - ebable port-based FID
 *      fid - Specified filtering database.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_L2_FID - Invalid fid.
 *      RT_ERR_INPUT - Invalid input parameter.
 *      RT_ERR_PORT_ID - Invalid port ID.
 * Note:
 *      The API can set port-based filtering database. If the function is enabled, all input
 *      packets will be assigned to the port-based fid regardless vlan tag.
 */
rtk_api_ret_t rtk_vlan_portFid_set(rtk_port_t port, rtk_enable_t enable, rtk_fid_t fid)
{
    rtk_api_ret_t retVal = 0;

    if (NULL == RT_MAPPER->vlan_portFid_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->vlan_portFid_set(port, enable, fid);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_vlan_portFid_get
 * Description:
 *      Get port-based filtering database
 * Input:
 *      port - Port id.
 * Output:
 *      pEnable - ebable port-based FID
 *      pFid - Specified filtering database.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT - Invalid input parameters.
 *      RT_ERR_PORT_ID - Invalid port ID.
 * Note:
 *      The API can get port-based filtering database status. If the function is enabled, all input
 *      packets will be assigned to the port-based fid regardless vlan tag.
 */
rtk_api_ret_t rtk_vlan_portFid_get(rtk_port_t port, rtk_enable_t *pEnable, rtk_fid_t *pFid)
{
    rtk_api_ret_t retVal = 0;
    
    if (NULL == RT_MAPPER->vlan_portFid_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->vlan_portFid_get(port, pEnable, pFid);
    RTK_API_UNLOCK();

    return retVal;
}

#if 0
/* Function Name:

 *      rtk_stp_mstpState_set
 * Description:
 *      Configure spanning tree state per each port.
 * Input:
 *      port - Port id
 *      msti - Multiple spanning tree instance.
 *      stpState - Spanning tree state for msti
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_MSTI         - Invalid msti parameter.
 *      RT_ERR_MSTP_STATE   - Invalid STP state.
 * Note:
 *      System supports per-port multiple spanning tree state for each msti.
 *      There are four states supported by ASIC.
 *      - STP_STATE_DISABLED
 *      - STP_STATE_BLOCKING
 *      - STP_STATE_LEARNING
 *      - STP_STATE_FORWARDING
 */
rtk_api_ret_t rtk_stp_mstpState_set(rtk_stp_msti_id_t msti, rtk_port_t port, rtk_stp_state_t stpState)
{
    rtk_api_ret_t retVal = 0;
    
    if (NULL == RT_MAPPER->stp_mstpState_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->stp_mstpState_set(msti, port, stpState);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_stp_mstpState_get
 * Description:
 *      Get spanning tree state per each port.
 * Input:
 *      port - Port id.
 *      msti - Multiple spanning tree instance.
 * Output:
 *      pStpState - Spanning tree state for msti
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_MSTI         - Invalid msti parameter.
 * Note:
 *      System supports per-port multiple spanning tree state for each msti.
 *      There are four states supported by ASIC.
 *      - STP_STATE_DISABLED
 *      - STP_STATE_BLOCKING
 *      - STP_STATE_LEARNING
 *      - STP_STATE_FORWARDING
 */
rtk_api_ret_t rtk_stp_mstpState_get(rtk_stp_msti_id_t msti, rtk_port_t port, rtk_stp_state_t *pStpState)
{
    rtk_api_ret_t retVal = 0;
    
    if (NULL == RT_MAPPER->stp_mstpState_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->stp_mstpState_get(msti, port, pStpState);
    RTK_API_UNLOCK();

    return retVal;
}

#endif

/* Function Name:

 *      rtk_vlan_reservedVidAction_set
 * Description:
 *      Set Action of VLAN ID = 0 & 4095 tagged packet
 * Input:
 *      actionVid0     - Action for VID 0.
 *      actionVid4095  - Action for VID 4095.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Error Input
 * Note:
 *
 */
rtk_api_ret_t rtk_vlan_reservedVidAction_set(rtk_vlan_resVidAction_t actionVid0, rtk_vlan_resVidAction_t actionVid4095)
{
    rtk_api_ret_t retVal = 0;
    
    if (NULL == RT_MAPPER->vlan_reservedVidAction_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->vlan_reservedVidAction_set(actionVid0, actionVid4095);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_vlan_reservedVidAction_get
 * Description:
 *      Get Action of VLAN ID = 0 & 4095 tagged packet
 * Input:
 *      pActionVid0     - Action for VID 0.
 *      pActionVid4095  - Action for VID 4095.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_NULL_POINTER - NULL Pointer
 * Note:
 *
 */
rtk_api_ret_t rtk_vlan_reservedVidAction_get(rtk_vlan_resVidAction_t *pActionVid0, rtk_vlan_resVidAction_t *pActionVid4095)
{
    rtk_api_ret_t retVal = 0;
    
    if (NULL == RT_MAPPER->vlan_reservedVidAction_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->vlan_reservedVidAction_get(pActionVid0, pActionVid4095);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_vlan_realKeepRemarkEnable_set
 * Description:
 *      Set Real keep 1p remarking feature
 * Input:
 *      enabled     - State of 1p remarking at real keep packet
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Error Input
 * Note:
 *
 */
rtk_api_ret_t rtk_vlan_realKeepRemarkEnable_set(rtk_enable_t enabled)
{
    rtk_api_ret_t retVal = 0;

    if (NULL == RT_MAPPER->vlan_realKeepRemarkEnable_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->vlan_realKeepRemarkEnable_set(enabled);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_vlan_realKeepRemarkEnable_get
 * Description:
 *      Get Real keep 1p remarking feature
 * Input:
 *      None.
 * Output:
 *      pEnabled     - State of 1p remarking at real keep packet
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Error Input
 * Note:
 *
 */
rtk_api_ret_t rtk_vlan_realKeepRemarkEnable_get(rtk_enable_t *pEnabled)
{
    rtk_api_ret_t retVal = 0;
    
    if (NULL == RT_MAPPER->vlan_realKeepRemarkEnable_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->vlan_realKeepRemarkEnable_get(pEnabled);
    RTK_API_UNLOCK();

    return retVal;
}

/*******************************************************************************
* Function Name: rtk_vlan_disL2Learn_set
*
* Description:
*   config a L2 disable learning entry which based on vlan id
*Input:
*       index: entry index
*       pDisL2LearnCfg: L2 disable learning database
*Output:
*       None
*Return:
*       RT_ERR_OK           - OK
*       RT_ERR_FAILED       - Failed
*       RT_ERR_ENTRY_INDEX  - error entry
*       RT_ERR_ENABLE       - error action
*       RT_ERR_NULL_POINTER - NULL Pointer
*       RT_ERR_VLAN_VID     - Invalid VID parameter.

*Note:  None
*******************************************************************************/
rtk_api_ret_t rtk_vlan_disL2Learn_set(rtk_vlan_disL2_learn_t *pDisL2LearnCfg)
{
    rtk_api_ret_t retVal = 0;

    if (NULL == RT_MAPPER->vlan_disL2Learn_entry_set)
        return RT_ERR_DRIVER_NOT_FOUND;
    RTK_API_LOCK();
    retVal = RT_MAPPER->vlan_disL2Learn_entry_set(pDisL2LearnCfg);
    RTK_API_UNLOCK();

    return retVal;

}
/*******************************************************************************
* Function Name: rtk_vlan_disL2Learn_get
*
* Description:
*   get a L2 disable learning entry which based on vlan id
*Input:
*       index: entry index
*       pDisL2LearnCfg:  L2 disable learning database
*Output:
*       None
*Return:
*       RT_ERR_OK           - OK
*       RT_ERR_FAILED       - Failed
*       RT_ERR_ENTRY_INDEX  - error entry
*       RT_ERR_ENABLE       - error action
*       RT_ERR_NULL_POINTER - NULL Pointer
*       RT_ERR_VLAN_VID     - Invalid VID parameter.

*Note:  None
*******************************************************************************/
rtk_api_ret_t rtk_vlan_disL2Learn_get(rtk_uint32 index, rtk_vlan_disL2_learn_t *pDisL2LearnCfg)
{
    rtk_api_ret_t retVal = 0;
    
    if (NULL == RT_MAPPER->vlan_disL2Learn_entry_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->vlan_disL2Learn_entry_get(index, pDisL2LearnCfg);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_vlan_reset
 * Description:
 *      Reset VLAN
 * Input:
 *      None.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Error Input
 * Note:
 *
 */
rtk_api_ret_t rtk_vlan_reset(void)
{
    rtk_api_ret_t retVal = 0;
    
    if (NULL == RT_MAPPER->vlan_reset)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->vlan_reset();
    RTK_API_UNLOCK();

    return retVal;
}


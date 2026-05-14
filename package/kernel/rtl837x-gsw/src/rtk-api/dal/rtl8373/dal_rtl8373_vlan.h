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
 * Feature : The file includes Trap module high-layer VLAN defination
 *
 */

#ifndef __DAL_RTL8373_VLAN_H__
#define __DAL_RTL8373_VLAN_H__

#include <vlan.h>

typedef struct  USER_VLANTABLE{

    rtk_uint16  vid;
    rtk_uint16  mbr;
    rtk_uint16  untag;
    rtk_uint16  fid_msti;
    rtk_uint16  svlan_chk_ivl_svl;
    rtk_uint16  ivl_svl;

}dal_rtl8373_user_vlan4kentry;

extern ret_t _dal_rtl8373_setAsicVlan4kEntry(dal_rtl8373_user_vlan4kentry *pVlan4kEntry);
extern ret_t _dal_rtl8373_getAsicVlan4kEntry(dal_rtl8373_user_vlan4kentry *pVlan4kEntry);

/* Function Name:
 *      dal_rtl8373_vlan_init
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
extern rtk_api_ret_t dal_rtl8373_vlan_init(void);

/* Function Name:
 *      dal_rtl8373_vlan_set
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
extern rtk_api_ret_t dal_rtl8373_vlan_set(rtk_vlan_t vid, rtk_vlan_entry_t *pVlanCfg);

/* Function Name:
 *      dal_rtl8373_vlan_get
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
extern rtk_api_ret_t dal_rtl8373_vlan_get(rtk_vlan_t vid, rtk_vlan_entry_t *pVlanCfg);

/* Function Name:
 *      dal_rtl8373_vlan_egrFilterEnable_set
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
extern rtk_api_ret_t dal_rtl8373_vlan_egrFilterEnable_set(rtk_enable_t egrFilter);

/* Function Name:
 *      dal_rtl8373_vlan_egrFilterEnable_get
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
extern rtk_api_ret_t dal_rtl8373_vlan_egrFilterEnable_get(rtk_enable_t *pEgrFilter);

/* Function Name:
 *     dal_rtl8373_vlan_portPvid_set
 * Description:
 *      Set port to specified VLAN ID(PVID).
 * Input:
 *      port - Port id.
 *      pvid - Specified VLAN ID.
 *      priority - 802.1p priority for the PVID.
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
extern rtk_api_ret_t dal_rtl8373_vlan_portPvid_set(rtk_port_t port, rtk_vlan_t pvid);

/* Function Name:
 *      dal_rtl8373_vlan_portPvid_get
 * Description:
 *      Get VLAN ID(PVID) on specified port.
 * Input:
 *      port - Port id.
 * Output:
 *      pPvid - Specified VLAN ID.
 *      pPriority - 802.1p priority for the PVID.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *     The API can get the PVID and 802.1p priority for the PVID of Port-based VLAN.
 */
extern rtk_api_ret_t dal_rtl8373_vlan_portPvid_get(rtk_port_t port, rtk_vlan_t *pPvid);

/* Function Name:
 *      dal_rtl8373_vlan_portIgrFilterEnable_set
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
extern rtk_api_ret_t dal_rtl8373_vlan_portIgrFilterEnable_set(rtk_port_t port, rtk_enable_t igrFilter);

/* Function Name:
 *      dal_rtl8373_vlan_portIgrFilterEnable_get
 * Description:
 *      Get VLAN Ingress Filter
 * Input:
 *      port        - Port id.
 * Output:
 *      pIgrFilter - VLAN ingress function enable status.
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
extern rtk_api_ret_t dal_rtl8373_vlan_portIgrFilterEnable_get(rtk_port_t port, rtk_enable_t *pIgrFilter);

/* Function Name:
 *      dal_rtl8373_vlan_portAcceptFrameType_set
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
extern rtk_api_ret_t dal_rtl8373_vlan_portAcceptFrameType_set(rtk_port_t port, rtk_vlan_acceptFrameType_t acceptFrameType);

/* Function Name:
 *      dal_rtl8373_vlan_portAcceptFrameType_get
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
extern rtk_api_ret_t dal_rtl8373_vlan_portAcceptFrameType_get(rtk_port_t port, rtk_vlan_acceptFrameType_t *pAcceptFrameType);

/* Function Name:
 *      dal_rtl8373_vlan_tagMode_set
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
extern rtk_api_ret_t dal_rtl8373_vlan_tagMode_set(rtk_port_t port, rtk_vlan_egressTagMode_t tagMode);

/* Function Name:
 *      dal_rtl8373_vlan_tagMode_get
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
extern rtk_api_ret_t dal_rtl8373_vlan_tagMode_get(rtk_port_t port, rtk_vlan_egressTagMode_t *pTagMode);

/* Function Name:
 *      dal_rtl8373_vlan_transparent_set
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
extern rtk_api_ret_t dal_rtl8373_vlan_portTransparent_set(rtk_port_t egrPort, rtk_portmask_t *pIgrPmsk);

/* Function Name:
 *      dal_rtl8373_vlan_transparent_get
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
extern rtk_api_ret_t dal_rtl8373_vlan_portTransparent_get(rtk_port_t egrPort, rtk_portmask_t *pIgrPmsk);

/* Function Name:
 *      dal_rtl8373_vlan_keep_set
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
extern rtk_api_ret_t dal_rtl8373_vlan_keep_set(rtk_port_t egrPort, rtk_portmask_t *pIgrPmsk);

/* Function Name:
 *      dal_rtl8373_vlan_keep_get
 * Description:
 *      Get VLAN egress keep mode
 * Input:
 *      egr_port        - Egress Port id.
 * Output:
 *      pIgr_pmask      - Ingress Port Mask
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      None.
 */
extern rtk_api_ret_t dal_rtl8373_vlan_keep_get(rtk_port_t egrPort, rtk_portmask_t *pIgrPmsk);

/* Function Name:
 *      dal_rtl8373_vlan_stg_set
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
extern rtk_api_ret_t dal_rtl8373_vlan_stg_set(rtk_vlan_t vid, rtk_stp_msti_id_t stg);

/* Function Name:
 *      dal_rtl8373_vlan_stg_get
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
extern rtk_api_ret_t dal_rtl8373_vlan_stg_get(rtk_vlan_t vid, rtk_stp_msti_id_t *pStg);

/* Function Name:
 *      dal_rtl8373_vlan_portFid_set
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
extern rtk_api_ret_t dal_rtl8373_vlan_portFid_set(rtk_port_t port, rtk_enable_t enable, rtk_fid_t fid);

/* Function Name:
 *      dal_rtl8373_vlan_portFid_get
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
extern rtk_api_ret_t dal_rtl8373_vlan_portFid_get(rtk_port_t port, rtk_enable_t *pEnable, rtk_fid_t *pFid);

/* Function Name:
 *      dal_rtl8373_vlan_reservedVidAction_set
 * Description:
 *      Set Action of VLAN ID = 0 & 4095 tagged packet
 * Input:
 *      actVid0     - Action for VID 0.
 *      actVid4095  - Action for VID 4095.
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
extern rtk_api_ret_t dal_rtl8373_vlan_reservedVidAction_set(rtk_vlan_resVidAction_t actVid0, rtk_vlan_resVidAction_t actVid4095);

/* Function Name:
 *      dal_rtl8373_vlan_reservedVidAction_get
 * Description:
 *      Get Action of VLAN ID = 0 & 4095 tagged packet
 * Input:
 *      pActVid0     - Action for VID 0.
 *      pActVid4095  - Action for VID 4095.
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
extern rtk_api_ret_t dal_rtl8373_vlan_reservedVidAction_get(rtk_vlan_resVidAction_t *pActVid0, rtk_vlan_resVidAction_t *pActVid4095);

/* Function Name:
 *      dal_rtl8373_vlan_realKeepRemarkEnable_set
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
extern rtk_api_ret_t dal_rtl8373_vlan_realKeepRemarkEnable_set(rtk_enable_t enabled);

/* Function Name:
 *      dal_rtl8373_vlan_realKeepRemarkEnable_get
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
extern rtk_api_ret_t dal_rtl8373_vlan_realKeepRemarkEnable_get(rtk_enable_t *pEnabled);

/*******************************************************************************
* Function Name: dal_rtl8373_vlan_disL2Learn_entry_set
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
extern rtk_api_ret_t dal_rtl8373_vlan_disL2Learn_entry_set(rtk_vlan_disL2_learn_t *pDisL2LearnCfg);

/*******************************************************************************
* Function Name: dal_rtl8373_vlan_disL2Learn_entry_get
*
* Description:
*   get vlan based disable L2 entry data
*Input:
*       index: entry index
*       
*Output:
*       pDisL2LearnCfg: L2 disable learning database
*Return:
*       RT_ERR_OK           - OK
*       RT_ERR_FAILED       - Failed
*       RT_ERR_ENTRY_INDEX  - error entry
*       RT_ERR_ENABLE       - error action
*       RT_ERR_NULL_POINTER - NULL Pointer
*       RT_ERR_VLAN_VID     - Invalid VID parameter.

*Note:  None
*******************************************************************************/
extern rtk_api_ret_t dal_rtl8373_vlan_disL2Learn_entry_get(rtk_uint32 index, rtk_vlan_disL2_learn_t *pDisL2LearnCfg);

/* Function Name:
 *      dal_rtl8373_vlan_reset
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
extern rtk_api_ret_t dal_rtl8373_vlan_reset(void);

#endif /* __DAL_RTL8373_VLAN_H__ */

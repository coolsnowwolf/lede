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
 * Feature : The file includes Trap module high-layer VLAN defination
 *
 */

#ifndef __RTK_API_VLAN_H__
#define __RTK_API_VLAN_H__


/*
 * Data Type Declaration
 */
#define RTK_MAX_NUM_OF_PROTO_TYPE                   0xFFFF
#define RTK_MAX_NUM_OF_MSTI                         0xF
#define RTK_FID_MAX                                 0xF

typedef struct  rtk_vlan_cfg_s
{
    rtk_portmask_t  mbr;
    rtk_portmask_t  untag;
    rtk_uint16      ivl_en;
    rtk_uint16      fid_msti;
    rtk_uint16      envlanpol;
    rtk_uint16      meteridx;
    rtk_uint16      vbpen;
    rtk_uint16      vbpri;
}rtk_vlan_cfg_t;

typedef struct  rtk_vlan_mbrcfg_s
{
    rtk_uint16      evid;
    rtk_portmask_t  mbr;
    rtk_uint16      fid_msti;
    rtk_uint16      envlanpol;
    rtk_uint16      meteridx;
    rtk_uint16      vbpen;
    rtk_uint16      vbpri;
}rtk_vlan_mbrcfg_t;

typedef rtk_uint32  rtk_stp_msti_id_t;     /* MSTI ID  */

typedef enum rtk_stp_state_e
{
    STP_STATE_DISABLED = 0,
    STP_STATE_BLOCKING,
    STP_STATE_LEARNING,
    STP_STATE_FORWARDING,
    STP_STATE_END
} rtk_stp_state_t;

typedef rtk_uint32  rtk_vlan_proto_type_t;     /* protocol and port based VLAN protocol type  */


typedef enum rtk_vlan_acceptFrameType_e
{
    ACCEPT_FRAME_TYPE_ALL = 0,             /* untagged, priority-tagged and tagged */
    ACCEPT_FRAME_TYPE_TAG_ONLY,         /* tagged */
    ACCEPT_FRAME_TYPE_UNTAG_ONLY,     /* untagged and priority-tagged */
    ACCEPT_FRAME_TYPE_END
} rtk_vlan_acceptFrameType_t;


/* frame type of protocol vlan - reference 802.1v standard */
typedef enum rtk_vlan_protoVlan_frameType_e
{
    FRAME_TYPE_ETHERNET = 0,
    FRAME_TYPE_LLCOTHER,
    FRAME_TYPE_RFC1042,
    FRAME_TYPE_END
} rtk_vlan_protoVlan_frameType_t;

/* Protocol-and-port-based Vlan structure */
typedef struct rtk_vlan_protoAndPortInfo_s
{
    rtk_uint32                         proto_type;
    rtk_vlan_protoVlan_frameType_t frame_type;
    rtk_vlan_t                     cvid;
    rtk_pri_t                     cpri;
}rtk_vlan_protoAndPortInfo_t;

/* tagged mode of VLAN - reference realtek private specification */
typedef enum rtk_vlan_tagMode_e
{
    VLAN_TAG_MODE_ORIGINAL = 0,
    VLAN_TAG_MODE_KEEP_FORMAT,
    VLAN_TAG_MODE_PRI,
    VLAN_TAG_MODE_REAL_KEEP_FORMAT,
    VLAN_TAG_MODE_END
} rtk_vlan_tagMode_t;

typedef enum rtk_vlan_resVidAction_e
{
    RESVID_ACTION_UNTAG = 0,
    RESVID_ACTION_TAG,
    RESVID_ACTION_END
}
rtk_vlan_resVidAction_t;

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
extern rtk_api_ret_t rtk_vlan_init(void);

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
extern rtk_api_ret_t rtk_vlan_set(rtk_vlan_t vid, rtk_vlan_cfg_t *pVlanCfg);

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
extern rtk_api_ret_t rtk_vlan_get(rtk_vlan_t vid, rtk_vlan_cfg_t *pVlanCfg);

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
extern rtk_api_ret_t rtk_vlan_egrFilterEnable_set(rtk_enable_t egrFilter);

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
extern rtk_api_ret_t rtk_vlan_egrFilterEnable_get(rtk_enable_t *pEgrFilter);

/* Function Name:
 *      rtk_vlan_mbrCfg_set
 * Description:
 *      Set a VLAN Member Configuration entry by index.
 * Input:
 *      idx     - Index of VLAN Member Configuration.
 *      pMbrcfg - VLAN member Configuration.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_VLAN_VID     - Invalid VID parameter.
 * Note:
 *     Set a VLAN Member Configuration entry by index.
 */
extern rtk_api_ret_t rtk_vlan_mbrCfg_set(rtk_uint32 idx, rtk_vlan_mbrcfg_t *pMbrcfg);

/* Function Name:
 *      rtk_vlan_mbrCfg_get
 * Description:
 *      Get a VLAN Member Configuration entry by index.
 * Input:
 *      idx - Index of VLAN Member Configuration.
 * Output:
 *      pMbrcfg - VLAN member Configuration.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_VLAN_VID     - Invalid VID parameter.
 * Note:
 *     Get a VLAN Member Configuration entry by index.
 */
extern rtk_api_ret_t rtk_vlan_mbrCfg_get(rtk_uint32 idx, rtk_vlan_mbrcfg_t *pMbrcfg);

/* Function Name:
 *     rtk_vlan_portPvid_set
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
extern rtk_api_ret_t rtk_vlan_portPvid_set(rtk_port_t port, rtk_vlan_t pvid, rtk_pri_t priority);

/* Function Name:
 *      rtk_vlan_portPvid_get
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
extern rtk_api_ret_t rtk_vlan_portPvid_get(rtk_port_t port, rtk_vlan_t *pPvid, rtk_pri_t *pPriority);

/* Function Name:
 *      rtk_vlan_portIgrFilterEnable_set
 * Description:
 *      Set VLAN ingress for each port.
 * Input:
 *      port - Port id.
 *      igr_filter - VLAN ingress function enable status.
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
extern rtk_api_ret_t rtk_vlan_portIgrFilterEnable_set(rtk_port_t port, rtk_enable_t igr_filter);

/* Function Name:
 *      rtk_vlan_portIgrFilterEnable_get
 * Description:
 *      Get VLAN Ingress Filter
 * Input:
 *      port        - Port id.
 * Output:
 *      pIgr_filter - VLAN ingress function enable status.
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
extern rtk_api_ret_t rtk_vlan_portIgrFilterEnable_get(rtk_port_t port, rtk_enable_t *pIgr_filter);

/* Function Name:
 *      rtk_vlan_portAcceptFrameType_set
 * Description:
 *      Set VLAN accept_frame_type
 * Input:
 *      port                - Port id.
 *      accept_frame_type   - accept frame type
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
extern rtk_api_ret_t rtk_vlan_portAcceptFrameType_set(rtk_port_t port, rtk_vlan_acceptFrameType_t accept_frame_type);

/* Function Name:
 *      rtk_vlan_portAcceptFrameType_get
 * Description:
 *      Get VLAN accept_frame_type
 * Input:
 *      port - Port id.
 * Output:
 *      pAccept_frame_type - accept frame type
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
extern rtk_api_ret_t rtk_vlan_portAcceptFrameType_get(rtk_port_t port, rtk_vlan_acceptFrameType_t *pAccept_frame_type);

/* Function Name:
 *      rtk_vlan_tagMode_set
 * Description:
 *      Set CVLAN egress tag mode
 * Input:
 *      port        - Port id.
 *      tag_mode    - The egress tag mode.
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
 *      - VLAN_TAG_MODE_ORIGINAL,
 *      - VLAN_TAG_MODE_KEEP_FORMAT,
 *      - VLAN_TAG_MODE_PRI.
 *      - VLAN_TAG_MODE_REAL_KEEP_FORMAT,
 */
extern rtk_api_ret_t rtk_vlan_tagMode_set(rtk_port_t port, rtk_vlan_tagMode_t tag_mode);

/* Function Name:
 *      rtk_vlan_tagMode_get
 * Description:
 *      Get CVLAN egress tag mode
 * Input:
 *      port - Port id.
 * Output:
 *      pTag_mode - The egress tag mode.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      The API can get Egress tag mode. There are 4 mode for egress tag:
 *      - VLAN_TAG_MODE_ORIGINAL,
 *      - VLAN_TAG_MODE_KEEP_FORMAT,
 *      - VLAN_TAG_MODE_PRI.
 *      - VLAN_TAG_MODE_REAL_KEEP_FORMAT,
 */
extern rtk_api_ret_t rtk_vlan_tagMode_get(rtk_port_t port, rtk_vlan_tagMode_t *pTag_mode);

/* Function Name:
 *      rtk_vlan_transparent_set
 * Description:
 *      Set VLAN transparent mode
 * Input:
 *      egr_port        - Egress Port id.
 *      pIgr_pmask      - Ingress Port Mask.
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
extern rtk_api_ret_t rtk_vlan_transparent_set(rtk_port_t egr_port, rtk_portmask_t *pIgr_pmask);

/* Function Name:
 *      rtk_vlan_transparent_get
 * Description:
 *      Get VLAN transparent mode
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
extern rtk_api_ret_t rtk_vlan_transparent_get(rtk_port_t egr_port, rtk_portmask_t *pIgr_pmask);

/* Function Name:
 *      rtk_vlan_keep_set
 * Description:
 *      Set VLAN egress keep mode
 * Input:
 *      egr_port        - Egress Port id.
 *      pIgr_pmask      - Ingress Port Mask.
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
extern rtk_api_ret_t rtk_vlan_keep_set(rtk_port_t egr_port, rtk_portmask_t *pIgr_pmask);

/* Function Name:
 *      rtk_vlan_keep_get
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
extern rtk_api_ret_t rtk_vlan_keep_get(rtk_port_t egr_port, rtk_portmask_t *pIgr_pmask);

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
extern rtk_api_ret_t rtk_vlan_stg_set(rtk_vlan_t vid, rtk_stp_msti_id_t stg);

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
extern rtk_api_ret_t rtk_vlan_stg_get(rtk_vlan_t vid, rtk_stp_msti_id_t *pStg);

/* Function Name:
 *      rtk_vlan_protoAndPortBasedVlan_add
 * Description:
 *      Add the protocol-and-port-based vlan to the specified port of device.
 * Input:
 *      port  - Port id.
 *      pInfo - Protocol and port based VLAN configuration information.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_PORT_ID          - Invalid port number.
 *      RT_ERR_VLAN_VID         - Invalid VID parameter.
 *      RT_ERR_VLAN_PRIORITY    - Invalid priority.
 *      RT_ERR_TBL_FULL         - Table is full.
 *      RT_ERR_OUT_OF_RANGE     - input out of range.
 * Note:
 *      The incoming packet which match the protocol-and-port-based vlan will use the configure vid for ingress pipeline
 *      The frame type is shown in the following:
 *      - FRAME_TYPE_ETHERNET
 *      - FRAME_TYPE_RFC1042
 *      - FRAME_TYPE_LLCOTHER
 */
extern rtk_api_ret_t rtk_vlan_protoAndPortBasedVlan_add(rtk_port_t port, rtk_vlan_protoAndPortInfo_t *pInfo);

/* Function Name:
 *      rtk_vlan_protoAndPortBasedVlan_get
 * Description:
 *      Get the protocol-and-port-based vlan to the specified port of device.
 * Input:
 *      port - Port id.
 *      proto_type - protocol-and-port-based vlan protocol type.
 *      frame_type - protocol-and-port-based vlan frame type.
 * Output:
 *      pInfo - Protocol and port based VLAN configuration information.
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_PORT_ID          - Invalid port number.
 *      RT_ERR_OUT_OF_RANGE     - input out of range.
 *      RT_ERR_TBL_FULL         - Table is full.
 * Note:
 *     The incoming packet which match the protocol-and-port-based vlan will use the configure vid for ingress pipeline
 *     The frame type is shown in the following:
 *      - FRAME_TYPE_ETHERNET
 *      - FRAME_TYPE_RFC1042
 *      - FRAME_TYPE_LLCOTHER
 */
extern rtk_api_ret_t rtk_vlan_protoAndPortBasedVlan_get(rtk_port_t port, rtk_vlan_proto_type_t proto_type, rtk_vlan_protoVlan_frameType_t frame_type, rtk_vlan_protoAndPortInfo_t *pInfo);

/* Function Name:
 *      rtk_vlan_protoAndPortBasedVlan_del
 * Description:
 *      Delete the protocol-and-port-based vlan from the specified port of device.
 * Input:
 *      port        - Port id.
 *      proto_type  - protocol-and-port-based vlan protocol type.
 *      frame_type  - protocol-and-port-based vlan frame type.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_PORT_ID          - Invalid port number.
 *      RT_ERR_OUT_OF_RANGE     - input out of range.
 *      RT_ERR_TBL_FULL         - Table is full.
 * Note:
 *     The incoming packet which match the protocol-and-port-based vlan will use the configure vid for ingress pipeline
 *     The frame type is shown in the following:
 *      - FRAME_TYPE_ETHERNET
 *      - FRAME_TYPE_RFC1042
 *      - FRAME_TYPE_LLCOTHER
 */
extern rtk_api_ret_t rtk_vlan_protoAndPortBasedVlan_del(rtk_port_t port, rtk_vlan_proto_type_t proto_type, rtk_vlan_protoVlan_frameType_t frame_type);

/* Function Name:
 *      rtk_vlan_protoAndPortBasedVlan_delAll
 * Description:
 *     Delete all protocol-and-port-based vlans from the specified port of device.
 * Input:
 *      port - Port id.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_PORT_ID          - Invalid port number.
 *      RT_ERR_OUT_OF_RANGE     - input out of range.
 * Note:
 *     The incoming packet which match the protocol-and-port-based vlan will use the configure vid for ingress pipeline
 *     Delete all flow table protocol-and-port-based vlan entries.
 */
extern rtk_api_ret_t rtk_vlan_protoAndPortBasedVlan_delAll(rtk_port_t port);

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
extern rtk_api_ret_t rtk_vlan_portFid_set(rtk_port_t port, rtk_enable_t enable, rtk_fid_t fid);

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
extern rtk_api_ret_t rtk_vlan_portFid_get(rtk_port_t port, rtk_enable_t *pEnable, rtk_fid_t *pFid);

/* Function Name:
 *      rtk_vlan_UntagDscpPriorityEnable_set
 * Description:
 *      Set Untag DSCP priority assign
 * Input:
 *      enable - state of Untag DSCP priority assign
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_ENABLE          - Invalid input parameters.
 * Note:
 *
 */
extern rtk_api_ret_t rtk_vlan_UntagDscpPriorityEnable_set(rtk_enable_t enable);

/* Function Name:
 *      rtk_vlan_UntagDscpPriorityEnable_get
 * Description:
 *      Get Untag DSCP priority assign
 * Input:
 *      None
 * Output:
 *      pEnable - state of Untag DSCP priority assign
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_NULL_POINTER    - Null pointer
 * Note:
 *
 */
extern rtk_api_ret_t rtk_vlan_UntagDscpPriorityEnable_get(rtk_enable_t *pEnable);


/*Spanning Tree*/
/* Function Name:
 *      rtk_stp_mstpState_set
 * Description:
 *      Configure spanning tree state per each port.
 * Input:
 *      port - Port id
 *      msti - Multiple spanning tree instance.
 *      stp_state - Spanning tree state for msti
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
extern rtk_api_ret_t rtk_stp_mstpState_set(rtk_stp_msti_id_t msti, rtk_port_t port, rtk_stp_state_t stp_state);

/* Function Name:
 *      rtk_stp_mstpState_get
 * Description:
 *      Get spanning tree state per each port.
 * Input:
 *      port - Port id.
 *      msti - Multiple spanning tree instance.
 * Output:
 *      pStp_state - Spanning tree state for msti
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
extern rtk_api_ret_t rtk_stp_mstpState_get(rtk_stp_msti_id_t msti, rtk_port_t port, rtk_stp_state_t *pStp_state);

/* Function Name:
 *      rtk_vlan_checkAndCreateMbr
 * Description:
 *      Check and create Member configuration and return index
 * Input:
 *      vid  - VLAN id.
 * Output:
 *      pIndex  - Member configuration index
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_VLAN_VID     - Invalid VLAN ID.
 *      RT_ERR_VLAN_ENTRY_NOT_FOUND - VLAN not found
 *      RT_ERR_TBL_FULL     - Member Configuration table full
 * Note:
 *
 */
extern rtk_api_ret_t rtk_vlan_checkAndCreateMbr(rtk_vlan_t vid, rtk_uint32 *pIndex);

/* Function Name:
 *      rtk_vlan_reservedVidAction_set
 * Description:
 *      Set Action of VLAN ID = 0 & 4095 tagged packet
 * Input:
 *      action_vid0     - Action for VID 0.
 *      action_vid4095  - Action for VID 4095.
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
extern rtk_api_ret_t rtk_vlan_reservedVidAction_set(rtk_vlan_resVidAction_t action_vid0, rtk_vlan_resVidAction_t action_vid4095);

/* Function Name:
 *      rtk_vlan_reservedVidAction_get
 * Description:
 *      Get Action of VLAN ID = 0 & 4095 tagged packet
 * Input:
 *      pAction_vid0     - Action for VID 0.
 *      pAction_vid4095  - Action for VID 4095.
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
extern rtk_api_ret_t rtk_vlan_reservedVidAction_get(rtk_vlan_resVidAction_t *pAction_vid0, rtk_vlan_resVidAction_t *pAction_vid4095);

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
extern rtk_api_ret_t rtk_vlan_realKeepRemarkEnable_set(rtk_enable_t enabled);

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
extern rtk_api_ret_t rtk_vlan_realKeepRemarkEnable_get(rtk_enable_t *pEnabled);

/* Function Name:
 *      rtk_vlan_reset
 * Description:
 *      Reset VLAN
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
rtk_api_ret_t rtk_vlan_reset(void);

#endif /* __RTK_API_VLAN_H__ */

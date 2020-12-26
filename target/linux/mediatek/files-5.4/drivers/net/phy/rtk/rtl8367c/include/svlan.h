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
 * Feature : The file includes SVLAN module high-layer API defination
 *
 */

#ifndef __RTK_API_SVLAN_H__
#define __RTK_API_SVLAN_H__

typedef rtk_uint32 rtk_svlan_index_t;

typedef struct rtk_svlan_memberCfg_s{
    rtk_uint32 svid;
    rtk_portmask_t memberport;
    rtk_portmask_t untagport;
    rtk_uint32 fiden;
    rtk_uint32 fid;
    rtk_uint32 priority;
    rtk_uint32 efiden;
    rtk_uint32 efid;
}rtk_svlan_memberCfg_t;

typedef enum rtk_svlan_pri_ref_e
{
    REF_INTERNAL_PRI = 0,
    REF_CTAG_PRI,
    REF_SVLAN_PRI,
    REF_PB_PRI,
    REF_PRI_END
} rtk_svlan_pri_ref_t;


typedef rtk_uint32 rtk_svlan_tpid_t;

typedef enum rtk_svlan_untag_action_e
{
    UNTAG_DROP = 0,
    UNTAG_TRAP,
    UNTAG_ASSIGN,
    UNTAG_END
} rtk_svlan_untag_action_t;

typedef enum rtk_svlan_unmatch_action_e
{
    UNMATCH_DROP = 0,
    UNMATCH_TRAP,
    UNMATCH_ASSIGN,
    UNMATCH_END
} rtk_svlan_unmatch_action_t;

typedef enum rtk_svlan_unassign_action_e
{
    UNASSIGN_PBSVID = 0,
    UNASSIGN_TRAP,
    UNASSIGN_END
} rtk_svlan_unassign_action_t;


typedef enum rtk_svlan_lookupType_e
{
    SVLAN_LOOKUP_S64MBRCGF  = 0,
    SVLAN_LOOKUP_C4KVLAN,
    SVLAN_LOOKUP_END,

} rtk_svlan_lookupType_t;

/* Function Name:
 *      rtk_svlan_init
 * Description:
 *      Initialize SVLAN Configuration
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      Ether type of S-tag in 802.1ad is 0x88a8 and there are existed ether type 0x9100 and 0x9200 for Q-in-Q SLAN design.
 *      User can set mathced ether type as service provider supported protocol.
 */
extern rtk_api_ret_t rtk_svlan_init(void);

/* Function Name:
 *      rtk_svlan_servicePort_add
 * Description:
 *      Add one service port in the specified device
 * Input:
 *      port - Port id.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This API is setting which port is connected to provider switch. All frames receiving from this port must
 *      contain accept SVID in S-tag field.
 */
extern rtk_api_ret_t rtk_svlan_servicePort_add(rtk_port_t port);

/* Function Name:
 *      rtk_svlan_servicePort_get
 * Description:
 *      Get service ports in the specified device.
 * Input:
 *      None
 * Output:
 *      pSvlan_portmask - pointer buffer of svlan ports.
 * Return:
 *      RT_ERR_OK          - OK
 *      RT_ERR_FAILED      - Failed
 *      RT_ERR_SMI         - SMI access error
 * Note:
 *      This API is setting which port is connected to provider switch. All frames receiving from this port must
 *      contain accept SVID in S-tag field.
 */
extern rtk_api_ret_t rtk_svlan_servicePort_get(rtk_portmask_t *pSvlan_portmask);

/* Function Name:
 *      rtk_svlan_servicePort_del
 * Description:
 *      Delete one service port in the specified device
 * Input:
 *      port - Port id.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 * Note:
 *      This API is removing SVLAN service port in the specified device.
 */
extern rtk_api_ret_t rtk_svlan_servicePort_del(rtk_port_t port);

/* Function Name:
 *      rtk_svlan_tpidEntry_set
 * Description:
 *      Configure accepted S-VLAN ether type.
 * Input:
 *      svlan_tag_id - Ether type of S-tag frame parsing in uplink ports.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameter.
 * Note:
 *      Ether type of S-tag in 802.1ad is 0x88a8 and there are existed ether type 0x9100 and 0x9200 for Q-in-Q SLAN design.
 *      User can set mathced ether type as service provider supported protocol.
 */
extern rtk_api_ret_t rtk_svlan_tpidEntry_set(rtk_uint32 svlan_tag_id);

/* Function Name:
 *      rtk_svlan_tpidEntry_get
 * Description:
 *      Get accepted S-VLAN ether type setting.
 * Input:
 *      None
 * Output:
 *      pSvlan_tag_id -  Ether type of S-tag frame parsing in uplink ports.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      This API is setting which port is connected to provider switch. All frames receiving from this port must
 *      contain accept SVID in S-tag field.
 */
extern rtk_api_ret_t rtk_svlan_tpidEntry_get(rtk_uint32 *pSvlan_tag_id);

/* Function Name:
 *      rtk_svlan_priorityRef_set
 * Description:
 *      Set S-VLAN upstream priority reference setting.
 * Input:
 *      ref - reference selection parameter.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameter.
 * Note:
 *      The API can set the upstream SVLAN tag priority reference source. The related priority
 *      sources are as following:
 *      - REF_INTERNAL_PRI,
 *      - REF_CTAG_PRI,
 *      - REF_SVLAN_PRI,
 *      - REF_PB_PRI.
 */
extern rtk_api_ret_t rtk_svlan_priorityRef_set(rtk_svlan_pri_ref_t ref);

/* Function Name:
 *      rtk_svlan_priorityRef_get
 * Description:
 *      Get S-VLAN upstream priority reference setting.
 * Input:
 *      None
 * Output:
 *      pRef - reference selection parameter.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 * Note:
 *      The API can get the upstream SVLAN tag priority reference source. The related priority
 *      sources are as following:
 *      - REF_INTERNAL_PRI,
 *      - REF_CTAG_PRI,
 *      - REF_SVLAN_PRI,
 *      - REF_PB_PRI
 */
extern rtk_api_ret_t rtk_svlan_priorityRef_get(rtk_svlan_pri_ref_t *pRef);

/* Function Name:
 *      rtk_svlan_memberPortEntry_set
 * Description:
 *      Configure system SVLAN member content
 * Input:
 *      svid - SVLAN id
 *      psvlan_cfg - SVLAN member configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_INPUT            - Invalid input parameter.
 *      RT_ERR_SVLAN_VID        - Invalid SVLAN VID parameter.
 *      RT_ERR_PORT_MASK        - Invalid portmask.
 *      RT_ERR_SVLAN_TABLE_FULL - SVLAN configuration is full.
 * Note:
 *      The API can set system 64 accepted s-tag frame format. Only 64 SVID S-tag frame will be accpeted
 *      to receiving from uplink ports. Other SVID S-tag frame or S-untagged frame will be droped by default setup.
 *      - rtk_svlan_memberCfg_t->svid is SVID of SVLAN member configuration.
 *      - rtk_svlan_memberCfg_t->memberport is member port mask of SVLAN member configuration.
 *      - rtk_svlan_memberCfg_t->fid is filtering database of SVLAN member configuration.
 *      - rtk_svlan_memberCfg_t->priority is priority of SVLAN member configuration.
 */
extern rtk_api_ret_t rtk_svlan_memberPortEntry_set(rtk_uint32 svid_idx, rtk_svlan_memberCfg_t *psvlan_cfg);

/* Function Name:
 *      rtk_svlan_memberPortEntry_get
 * Description:
 *      Get SVLAN member Configure.
 * Input:
 *      svid - SVLAN id
 * Output:
 *      pSvlan_cfg - SVLAN member configuration
 * Return:
 *      RT_ERR_OK                       - OK
 *      RT_ERR_FAILED                   - Failed
 *      RT_ERR_SMI                      - SMI access error
 *      RT_ERR_SVLAN_ENTRY_NOT_FOUND    - specified svlan entry not found.
 *      RT_ERR_INPUT                    - Invalid input parameters.
 * Note:
 *      The API can get system 64 accepted s-tag frame format. Only 64 SVID S-tag frame will be accpeted
 *      to receiving from uplink ports. Other SVID S-tag frame or S-untagged frame will be droped.
 */
extern rtk_api_ret_t rtk_svlan_memberPortEntry_get(rtk_uint32 svid_idx, rtk_svlan_memberCfg_t *pSvlan_cfg);

/* Function Name:
 *      rtk_svlan_memberPortEntry_adv_set
 * Description:
 *      Configure system SVLAN member by index
 * Input:
 *      idx         - Index (0 ~ 63)
 *      psvlan_cfg  - SVLAN member configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_INPUT            - Invalid input parameter.
 *      RT_ERR_SVLAN_VID        - Invalid SVLAN VID parameter.
 *      RT_ERR_PORT_MASK        - Invalid portmask.
 *      RT_ERR_SVLAN_TABLE_FULL - SVLAN configuration is full.
 * Note:
 *      The API can set system 64 accepted s-tag frame format by index.
 *      - rtk_svlan_memberCfg_t->svid is SVID of SVLAN member configuration.
 *      - rtk_svlan_memberCfg_t->memberport is member port mask of SVLAN member configuration.
 *      - rtk_svlan_memberCfg_t->fid is filtering database of SVLAN member configuration.
 *      - rtk_svlan_memberCfg_t->priority is priority of SVLAN member configuration.
 */
extern rtk_api_ret_t rtk_svlan_memberPortEntry_adv_set(rtk_uint32 idx, rtk_svlan_memberCfg_t *pSvlan_cfg);

/* Function Name:
 *      rtk_svlan_memberPortEntry_adv_get
 * Description:
 *      Get SVLAN member Configure by index.
 * Input:
 *      idx         - Index (0 ~ 63)
 * Output:
 *      pSvlan_cfg  - SVLAN member configuration
 * Return:
 *      RT_ERR_OK                       - OK
 *      RT_ERR_FAILED                   - Failed
 *      RT_ERR_SMI                      - SMI access error
 *      RT_ERR_SVLAN_ENTRY_NOT_FOUND    - specified svlan entry not found.
 *      RT_ERR_INPUT                    - Invalid input parameters.
 * Note:
 *      The API can get system 64 accepted s-tag frame format. Only 64 SVID S-tag frame will be accpeted
 *      to receiving from uplink ports. Other SVID S-tag frame or S-untagged frame will be droped.
 */
extern rtk_api_ret_t rtk_svlan_memberPortEntry_adv_get(rtk_uint32 idx, rtk_svlan_memberCfg_t *pSvlan_cfg);

/* Function Name:
 *      rtk_svlan_defaultSvlan_set
 * Description:
 *      Configure default egress SVLAN.
 * Input:
 *      port - Source port
 *      svid - SVLAN id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                       - OK
 *      RT_ERR_FAILED                   - Failed
 *      RT_ERR_SMI                      - SMI access error
 *      RT_ERR_INPUT                    - Invalid input parameter.
 *      RT_ERR_SVLAN_VID                - Invalid SVLAN VID parameter.
 *      RT_ERR_SVLAN_ENTRY_NOT_FOUND    - specified svlan entry not found.
 * Note:
 *      The API can set port n S-tag format index while receiving frame from port n
 *      is transmit through uplink port with s-tag field
 */
extern rtk_api_ret_t rtk_svlan_defaultSvlan_set(rtk_port_t port, rtk_vlan_t svid);

/* Function Name:
 *      rtk_svlan_defaultSvlan_get
 * Description:
 *      Get the configure default egress SVLAN.
 * Input:
 *      port - Source port
 * Output:
 *      pSvid - SVLAN VID
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      The API can get port n S-tag format index while receiving frame from port n
 *      is transmit through uplink port with s-tag field
 */
extern rtk_api_ret_t rtk_svlan_defaultSvlan_get(rtk_port_t port, rtk_vlan_t *pSvid);

/* Function Name:
 *      rtk_svlan_c2s_add
 * Description:
 *      Configure SVLAN C2S table
 * Input:
 *      vid - VLAN ID
 *      src_port - Ingress Port
 *      svid - SVLAN VID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port ID.
 *      RT_ERR_SVLAN_VID    - Invalid SVLAN VID parameter.
 *      RT_ERR_VLAN_VID     - Invalid VID parameter.
 *      RT_ERR_OUT_OF_RANGE - input out of range.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      The API can set system C2S configuration. ASIC will check upstream's VID and assign related
 *      SVID to mathed packet. There are 128 SVLAN C2S configurations.
 */
extern rtk_api_ret_t rtk_svlan_c2s_add(rtk_vlan_t vid, rtk_port_t src_port, rtk_vlan_t svid);

/* Function Name:
 *      rtk_svlan_c2s_del
 * Description:
 *      Delete one C2S entry
 * Input:
 *      vid - VLAN ID
 *      src_port - Ingress Port
 *      svid - SVLAN VID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_VLAN_VID         - Invalid VID parameter.
 *      RT_ERR_PORT_ID          - Invalid port ID.
 *      RT_ERR_OUT_OF_RANGE     - input out of range.
 * Note:
 *      The API can delete system C2S configuration. There are 128 SVLAN C2S configurations.
 */
extern rtk_api_ret_t rtk_svlan_c2s_del(rtk_vlan_t vid, rtk_port_t src_port);

/* Function Name:
 *      rtk_svlan_c2s_get
 * Description:
 *      Get configure SVLAN C2S table
 * Input:
 *      vid - VLAN ID
 *      src_port - Ingress Port
 * Output:
 *      pSvid - SVLAN ID
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_PORT_ID      - Invalid port ID.
 *      RT_ERR_OUT_OF_RANGE - input out of range.
 * Note:
 *     The API can get system C2S configuration. There are 128 SVLAN C2S configurations.
 */
extern rtk_api_ret_t rtk_svlan_c2s_get(rtk_vlan_t vid, rtk_port_t src_port, rtk_vlan_t *pSvid);

/* Function Name:
 *      rtk_svlan_untag_action_set
 * Description:
 *      Configure Action of downstream Un-Stag packet
 * Input:
 *      action  - Action for UnStag
 *      svid    - The SVID assigned to UnStag packet
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                       - OK
 *      RT_ERR_FAILED                   - Failed
 *      RT_ERR_SMI                      - SMI access error
 *      RT_ERR_SVLAN_VID                - Invalid SVLAN VID parameter.
 *      RT_ERR_SVLAN_ENTRY_NOT_FOUND    - specified svlan entry not found.
 *      RT_ERR_OUT_OF_RANGE             - input out of range.
 *      RT_ERR_INPUT                    - Invalid input parameters.
 * Note:
 *      The API can configure action of downstream Un-Stag packet. A SVID assigned
 *      to the un-stag is also supported by this API. The parameter of svid is
 *      only referenced when the action is set to UNTAG_ASSIGN
 */
extern rtk_api_ret_t rtk_svlan_untag_action_set(rtk_svlan_untag_action_t action, rtk_vlan_t svid);

/* Function Name:
 *      rtk_svlan_untag_action_get
 * Description:
 *      Get Action of downstream Un-Stag packet
 * Input:
 *      None
 * Output:
 *      pAction  - Action for UnStag
 *      pSvid    - The SVID assigned to UnStag packet
 * Return:
 *      RT_ERR_OK                       - OK
 *      RT_ERR_FAILED                   - Failed
 *      RT_ERR_SMI                      - SMI access error
 *      RT_ERR_SVLAN_VID                - Invalid SVLAN VID parameter.
 *      RT_ERR_SVLAN_ENTRY_NOT_FOUND    - specified svlan entry not found.
 *      RT_ERR_OUT_OF_RANGE             - input out of range.
 *      RT_ERR_INPUT                    - Invalid input parameters.
 * Note:
 *      The API can Get action of downstream Un-Stag packet. A SVID assigned
 *      to the un-stag is also retrieved by this API. The parameter pSvid is
 *      only refernced when the action is UNTAG_ASSIGN
 */
extern rtk_api_ret_t rtk_svlan_untag_action_get(rtk_svlan_untag_action_t *pAction, rtk_vlan_t *pSvid);

/* Function Name:
 *      rtk_svlan_unmatch_action_set
 * Description:
 *      Configure Action of downstream Unmatch packet
 * Input:
 *      action  - Action for Unmatch
 *      svid    - The SVID assigned to Unmatch packet
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                       - OK
 *      RT_ERR_FAILED                   - Failed
 *      RT_ERR_SMI                      - SMI access error
 *      RT_ERR_SVLAN_VID                - Invalid SVLAN VID parameter.
 *      RT_ERR_SVLAN_ENTRY_NOT_FOUND    - specified svlan entry not found.
 *      RT_ERR_OUT_OF_RANGE             - input out of range.
 *      RT_ERR_INPUT                    - Invalid input parameters.
 * Note:
 *      The API can configure action of downstream Un-match packet. A SVID assigned
 *      to the un-match is also supported by this API. The parameter od svid is
 *      only refernced when the action is set to UNMATCH_ASSIGN
 */
extern rtk_api_ret_t rtk_svlan_unmatch_action_set(rtk_svlan_unmatch_action_t action, rtk_vlan_t svid);

/* Function Name:
 *      rtk_svlan_unmatch_action_get
 * Description:
 *      Get Action of downstream Unmatch packet
 * Input:
 *      None
 * Output:
 *      pAction  - Action for Unmatch
 *      pSvid    - The SVID assigned to Unmatch packet
 * Return:
 *      RT_ERR_OK                       - OK
 *      RT_ERR_FAILED                   - Failed
 *      RT_ERR_SMI                      - SMI access error
 *      RT_ERR_SVLAN_VID                - Invalid SVLAN VID parameter.
 *      RT_ERR_SVLAN_ENTRY_NOT_FOUND    - specified svlan entry not found.
 *      RT_ERR_OUT_OF_RANGE             - input out of range.
 *      RT_ERR_INPUT                    - Invalid input parameters.
 * Note:
 *      The API can Get action of downstream Un-match packet. A SVID assigned
 *      to the un-match is also retrieved by this API. The parameter pSvid is
 *      only refernced when the action is UNMATCH_ASSIGN
 */
extern rtk_api_ret_t rtk_svlan_unmatch_action_get(rtk_svlan_unmatch_action_t *pAction, rtk_vlan_t *pSvid);

/* Function Name:
 *      rtk_svlan_dmac_vidsel_set
 * Description:
 *      Set DMAC CVID selection
 * Input:
 *      port    - Port
 *      enable  - state of DMAC CVID Selection
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK                       - OK
 *      RT_ERR_FAILED                   - Failed
 *      RT_ERR_SMI                      - SMI access error
 *      RT_ERR_SVLAN_VID                - Invalid SVLAN VID parameter.
 *      RT_ERR_SVLAN_ENTRY_NOT_FOUND    - specified svlan entry not found.
 *      RT_ERR_OUT_OF_RANGE             - input out of range.
 *      RT_ERR_INPUT                    - Invalid input parameters.
 * Note:
 *      This API can set DMAC CVID Selection state
 */
extern rtk_api_ret_t rtk_svlan_dmac_vidsel_set(rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      rtk_svlan_dmac_vidsel_get
 * Description:
 *      Get DMAC CVID selection
 * Input:
 *      port    - Port
 * Output:
 *      pEnable - state of DMAC CVID Selection
 * Return:
 *      RT_ERR_OK                       - OK
 *      RT_ERR_FAILED                   - Failed
 *      RT_ERR_SMI                      - SMI access error
 *      RT_ERR_SVLAN_VID                - Invalid SVLAN VID parameter.
 *      RT_ERR_SVLAN_ENTRY_NOT_FOUND    - specified svlan entry not found.
 *      RT_ERR_OUT_OF_RANGE             - input out of range.
 *      RT_ERR_INPUT                    - Invalid input parameters.
 * Note:
 *      This API can get DMAC CVID Selection state
 */
extern rtk_api_ret_t rtk_svlan_dmac_vidsel_get(rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_svlan_ipmc2s_add
 * Description:
 *      add ip multicast address to SVLAN
 * Input:
 *      svid    - SVLAN VID
 *      ipmc    - ip multicast address
 *      ipmcMsk - ip multicast mask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                       - OK
 *      RT_ERR_FAILED                   - Failed
 *      RT_ERR_SMI                      - SMI access error
 *      RT_ERR_SVLAN_VID                - Invalid SVLAN VID parameter.
 *      RT_ERR_SVLAN_ENTRY_NOT_FOUND    - specified svlan entry not found.
 *      RT_ERR_OUT_OF_RANGE             - input out of range.
 *      RT_ERR_INPUT                    - Invalid input parameters.
 * Note:
 *      The API can set IP mutlicast to SVID configuration. If upstream packet is IPv4 multicast
 *      packet and DIP is matched MC2S configuration, ASIC will assign egress SVID to the packet.
 *      There are 32 SVLAN multicast configurations for IP and L2 multicast.
 */
extern rtk_api_ret_t rtk_svlan_ipmc2s_add(ipaddr_t ipmc, ipaddr_t ipmcMsk, rtk_vlan_t svid);

/* Function Name:
 *      rtk_svlan_ipmc2s_del
 * Description:
 *      delete ip multicast address to SVLAN
 * Input:
 *      ipmc    - ip multicast address
 *      ipmcMsk - ip multicast mask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_SVLAN_VID        - Invalid SVLAN VID parameter.
 *      RT_ERR_OUT_OF_RANGE     - input out of range.
 * Note:
 *      The API can delete IP mutlicast to SVID configuration. There are 32 SVLAN multicast configurations for IP and L2 multicast.
 */
extern rtk_api_ret_t rtk_svlan_ipmc2s_del(ipaddr_t ipmc, ipaddr_t ipmcMsk);

/* Function Name:
 *      rtk_svlan_ipmc2s_get
 * Description:
 *      Get ip multicast address to SVLAN
 * Input:
 *      ipmc    - ip multicast address
 *      ipmcMsk - ip multicast mask
 * Output:
 *      pSvid - SVLAN VID
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_OUT_OF_RANGE - input out of range.
 * Note:
 *      The API can get IP mutlicast to SVID configuration. There are 32 SVLAN multicast configurations for IP and L2 multicast.
 */
extern rtk_api_ret_t rtk_svlan_ipmc2s_get(ipaddr_t ipmc, ipaddr_t ipmcMsk, rtk_vlan_t *pSvid);

/* Function Name:
 *      rtk_svlan_l2mc2s_add
 * Description:
 *      Add L2 multicast address to SVLAN
 * Input:
 *      mac     - L2 multicast address
 *      macMsk  - L2 multicast address mask
 *      svid    - SVLAN VID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                       - OK
 *      RT_ERR_FAILED                   - Failed
 *      RT_ERR_SMI                      - SMI access error
 *      RT_ERR_SVLAN_VID                - Invalid SVLAN VID parameter.
 *      RT_ERR_SVLAN_ENTRY_NOT_FOUND    - specified svlan entry not found.
 *      RT_ERR_OUT_OF_RANGE             - input out of range.
 *      RT_ERR_INPUT                    - Invalid input parameters.
 * Note:
 *      The API can set L2 Mutlicast to SVID configuration. If upstream packet is L2 multicast
 *      packet and DMAC is matched, ASIC will assign egress SVID to the packet. There are 32
 *      SVLAN multicast configurations for IP and L2 multicast.
 */
extern rtk_api_ret_t rtk_svlan_l2mc2s_add(rtk_mac_t mac, rtk_mac_t macMsk, rtk_vlan_t svid);

/* Function Name:
 *      rtk_svlan_l2mc2s_del
 * Description:
 *      delete L2 multicast address to SVLAN
 * Input:
 *      mac     - L2 multicast address
 *      macMsk  - L2 multicast address mask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_SVLAN_VID        - Invalid SVLAN VID parameter.
 *      RT_ERR_OUT_OF_RANGE     - input out of range.
 * Note:
 *      The API can delete Mutlicast to SVID configuration. There are 32 SVLAN multicast configurations for IP and L2 multicast.
 */
extern rtk_api_ret_t rtk_svlan_l2mc2s_del(rtk_mac_t mac, rtk_mac_t macMsk);

/* Function Name:
 *      rtk_svlan_l2mc2s_get
 * Description:
 *      Get L2 multicast address to SVLAN
 * Input:
 *      mac     - L2 multicast address
 *      macMsk  - L2 multicast address mask
 * Output:
 *      pSvid   - SVLAN VID
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_INPUT            - Invalid input parameters.
 *      RT_ERR_OUT_OF_RANGE     - input out of range.
 * Note:
 *      The API can get L2 mutlicast to SVID configuration. There are 32 SVLAN multicast configurations for IP and L2 multicast.
 */
extern rtk_api_ret_t rtk_svlan_l2mc2s_get(rtk_mac_t mac, rtk_mac_t macMsk, rtk_vlan_t *pSvid);

/* Function Name:
 *      rtk_svlan_sp2c_add
 * Description:
 *      Add system SP2C configuration
 * Input:
 *      cvid        - VLAN ID
 *      dst_port    - Destination port of SVLAN to CVLAN configuration
 *      svid        - SVLAN VID
 *
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_SVLAN_VID    - Invalid SVLAN VID parameter.
 *      RT_ERR_VLAN_VID     - Invalid VID parameter.
 *      RT_ERR_OUT_OF_RANGE - input out of range.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      The API can add SVID & Destination Port to CVLAN configuration. The downstream frames with assigned
 *      SVID will be add C-tag with assigned CVID if the output port is the assigned destination port.
 *      There are 128 SP2C configurations.
 */
extern rtk_api_ret_t rtk_svlan_sp2c_add(rtk_vlan_t svid, rtk_port_t dst_port, rtk_vlan_t cvid);

/* Function Name:
 *      rtk_svlan_sp2c_get
 * Description:
 *      Get configure system SP2C content
 * Input:
 *      svid        - SVLAN VID
 *      dst_port    - Destination port of SVLAN to CVLAN configuration
 * Output:
 *      pCvid - VLAN ID
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_OUT_OF_RANGE - input out of range.
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_SVLAN_VID    - Invalid SVLAN VID parameter.
 * Note:
 *     The API can get SVID & Destination Port to CVLAN configuration. There are 128 SP2C configurations.
 */
extern rtk_api_ret_t rtk_svlan_sp2c_get(rtk_vlan_t svid, rtk_port_t dst_port, rtk_vlan_t *pCvid);

/* Function Name:
 *      rtk_svlan_sp2c_del
 * Description:
 *      Delete system SP2C configuration
 * Input:
 *      svid        - SVLAN VID
 *      dst_port    - Destination port of SVLAN to CVLAN configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_SVLAN_VID    - Invalid SVLAN VID parameter.
 *      RT_ERR_OUT_OF_RANGE - input out of range.
 * Note:
 *      The API can delete SVID & Destination Port to CVLAN configuration. There are 128 SP2C configurations.
 */
extern rtk_api_ret_t rtk_svlan_sp2c_del(rtk_vlan_t svid, rtk_port_t dst_port);


/* Function Name:
 *      rtk_svlan_lookupType_set
 * Description:
 *      Set lookup type of SVLAN
 * Input:
 *      type        - lookup type
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      none
 */
extern rtk_api_ret_t rtk_svlan_lookupType_set(rtk_svlan_lookupType_t type);

/* Function Name:
 *      rtk_svlan_lookupType_get
 * Description:
 *      Get lookup type of SVLAN
 * Input:
 *      pType       - lookup type
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      none
 */
extern rtk_api_ret_t rtk_svlan_lookupType_get(rtk_svlan_lookupType_t *pType);

/* Function Name:
 *      rtk_svlan_trapPri_set
 * Description:
 *      Set svlan trap priority
 * Input:
 *      priority - priority for trap packets
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_QOS_INT_PRIORITY
 * Note:
 *      None
 */
extern rtk_api_ret_t rtk_svlan_trapPri_set(rtk_pri_t priority);

/* Function Name:
 *      rtk_svlan_trapPri_get
 * Description:
 *      Get svlan trap priority
 * Input:
 *      None
 * Output:
 *      pPriority - priority for trap packets
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern rtk_api_ret_t rtk_svlan_trapPri_get(rtk_pri_t *pPriority);

/* Function Name:
 *      rtk_svlan_unassign_action_set
 * Description:
 *      Configure Action of upstream without svid assign action
 * Input:
 *      action  - Action for Un-assign
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                       - OK
 *      RT_ERR_FAILED                   - Failed
 *      RT_ERR_OUT_OF_RANGE             - input out of range.
 *      RT_ERR_INPUT                    - Invalid input parameters.
 * Note:
 *      The API can configure action of upstream Un-assign svid packet. If action is not
 *      trap to CPU, the port-based SVID sure be assign as system need
 */
extern rtk_api_ret_t rtk_svlan_unassign_action_set(rtk_svlan_unassign_action_t action);

/* Function Name:
 *      rtk_svlan_unassign_action_get
 * Description:
 *      Get action of upstream without svid assignment
 * Input:
 *      None
 * Output:
 *      pAction  - Action for Un-assign
 * Return:
 *      RT_ERR_OK                       - OK
 *      RT_ERR_FAILED                   - Failed
 * Note:
 *      None
 */
extern rtk_api_ret_t rtk_svlan_unassign_action_get(rtk_svlan_unassign_action_t *pAction);


/* Function Name:
 *      rtk_svlan_checkAndCreateMbr
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
 *      RT_ERR_TBL_FULL     - Member Configuration table full
 * Note:
 *
 */
extern rtk_api_ret_t rtk_svlan_checkAndCreateMbr(rtk_vlan_t vid, rtk_uint32 *pIndex);


#endif /* __RTK_API_SVLAN_H__ */

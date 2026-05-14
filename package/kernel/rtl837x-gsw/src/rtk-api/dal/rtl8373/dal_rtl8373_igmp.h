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
 * Purpose : RTL8367/RTL8373 switch high-level API
 *
 * Feature : The file includes IGMP module high-layer API defination
 *
 */

#ifndef __DAL_RTL8373_IGMP_H__
#define __DAL_RTL8373_IGMP_H__

#include <igmp.h>
#include <dal_rtl8373_igmp.h>
#include <rtl8373_asicdrv.h>

#define RTL8373_MAX_LEAVE_TIMER        (7)
#define RTL8373_MAX_QUERY_INT          (0xFFFF)
#define RTL8373_MAX_ROB_VAR            (7)

#define RTL8373_IGMP_GOUP_NO           (256)
#define RTL8373_IGMP_MAX_GOUP          (0xFF)
#define RTL8373_IGMP_GRP_BLEN          (3)
#define RTL8373_ROUTER_PORT_INVALID    (0xF)

enum RTL8373_IGMPTABLE_FULL_OP
{
    TABLE_FULL_FORWARD = 0,
    TABLE_FULL_DROP,
    TABLE_FULL_TRAP,
    TABLE_FULL_OP_END
};

enum RTL8373_CRC_ERR_OP
{
    CRC_ERR_DROP = 0,
    CRC_ERR_TRAP,
    CRC_ERR_FORWARD,
    CRC_ERR_OP_END
};

enum RTL8373_IGMP_MLD_PROTOCOL_OP
{
    PROTOCOL_OP_ASIC = 0,
    PROTOCOL_OP_FLOOD,
    PROTOCOL_OP_TRAP,
    PROTOCOL_OP_DROP,
    PROTOCOL_OP_END
};

enum RTL8373_IGMP_MLD_BYPASS_GROUP
{
    BYPASS_224_0_0_X = 0,
    BYPASS_224_0_1_X,
    BYPASS_239_255_255_X,
    BYPASS_IPV6_00XX,
    BYPASS_GROUP_END
};

typedef struct
{
    rtk_uint32 p0_timer;
    rtk_uint32 p1_timer;
    rtk_uint32 p2_timer;
    rtk_uint32 p3_timer;
    rtk_uint32 p4_timer;
    rtk_uint32 p5_timer;
    rtk_uint32 p6_timer;
    rtk_uint32 p7_timer;
    rtk_uint32 p8_timer;
    rtk_uint32 p9_timer;
    rtk_uint32 p10_timer;
    rtk_uint32 report_supp_flag;

}rtl8373_igmpgroup;

/* Function Name:
 *      dal_rtl8373_setAsicIGMPIsoLeaky
 * Description:
 *      Set Port Isolation leaky for IGMP/MLD packet
 * Input:
 *      leaky    - 1: Leaky, 0:not leaky
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
rtk_api_ret_t dal_rtl8373_setAsicIGMPIsoLeaky(rtk_uint32 leaky);

/* Function Name:
 *      dal_rtl8373_getAsicIGMPIsoLeaky
 * Description:
 *      Get Port Isolation leaky for IGMP/MLD packet
 * Input:
 *      Noen
 * Output:
 *      pLeaky    - 1: Leaky, 0:not leaky
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
rtk_api_ret_t dal_rtl8373_getAsicIGMPIsoLeaky(rtk_uint32 *pLeaky);
/* Function Name:
 *      rtl8373_setAsicIGMPVLANLeaky
 * Description:
 *      Set VLAN leaky for IGMP/MLD packet
 * Input:
 *      leaky    - 1: Leaky, 0:not leaky
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
rtk_api_ret_t dal_rtl8373_setAsicIGMPVLANLeaky(rtk_uint32 leaky);

/* Function Name:
 *      rtl8373_getAsicIGMPVLANLeaky
 * Description:
 *      Get VLAN leaky for IGMP/MLD packet
 * Input:
 *      Noen
 * Output:
 *      pLeaky    - 1: Leaky, 0:not leaky
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
rtk_api_ret_t dal_rtl8373_getAsicIGMPVLANLeaky(rtk_uint32 *pLeaky);
/* Function Name:
 *      dal_rtl8373_setAsicIpMulticastVlanLeaky
 * Description:
 *      Set IP multicast VLAN Leaky function
 * Input:
 *      port        - Physical port number (0~7)
 *      enabled     - 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      When enabling this function,
 *      if the lookup result(forwarding portmap) of IP Multicast packet is over VLAN boundary,
 *      the packet can be forwarded across VLAN
 */
rtk_api_ret_t dal_rtl8373_setAsicIpMulticastVlanLeaky(rtk_uint32 port, rtk_uint32 enabled);

/* Function Name:
 *      dal_rtl8373_getAsicIpMulticastVlanLeaky
 * Description:
 *      Get IP multicast VLAN Leaky function
 * Input:
 *      port        - Physical port number (0~7)
 *      enabled     - 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
rtk_api_ret_t dal_rtl8373_getAsicIpMulticastVlanLeaky(rtk_uint32 port, rtk_uint32 *ptr_enabled);

/* Function Name:
 *       dal_rtl8373_getAsicIGMPGroup
 * Description:
 *      Get IGMP group
 * Input:
 *      idx     - Group index (0~255)
 *      valid   - valid bit
 *      grp     - IGMP group
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - Success
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_OUT_OF_RANGE     - Group index is out of range
 * Note:
 *      None
 */
rtk_api_ret_t  dal_rtl8373_getAsicIGMPGroup(rtk_uint32 idx, rtk_uint32 *valid, rtl8373_igmpgroup *grp);



/* Function Name:
 *      dal_rtl8373_igmp_init
 * Description:
 *      This API enables H/W IGMP and set a default initial configuration.
 * Input:
 *      None.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 * Note:
 *      This API enables H/W IGMP and set a default initial configuration.
 */
extern rtk_api_ret_t dal_rtl8373_igmp_init(void);

/* Function Name:
 *      dal_rtl8373_igmp_state_set
 * Description:
 *      This API set H/W IGMP state.
 * Input:
 *      enabled     - H/W IGMP state
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT           - Error parameter
 * Note:
 *      This API set H/W IGMP state.
 */
extern rtk_api_ret_t dal_rtl8373_igmp_state_set(rtk_enable_t enabled);

/* Function Name:
 *      dal_rtl8373_igmp_state_get
 * Description:
 *      This API get H/W IGMP state.
 * Input:
 *      None.
 * Output:
 *      pEnabled        - H/W IGMP state
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT           - Error parameter
 * Note:
 *      This API set current H/W IGMP state.
 */
extern rtk_api_ret_t dal_rtl8373_igmp_state_get(rtk_enable_t *pEnabled);

/* Function Name:
 *      dal_rtl8373_igmp_static_router_port_set
 * Description:
 *      Configure static router port
 * Input:
 *      pPortmask    - Static Port mask
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_MASK       - Error parameter
 * Note:
 *      This API set static router port
 */
extern rtk_api_ret_t dal_rtl8373_igmp_static_router_port_set(rtk_portmask_t *pPortmask);

/* Function Name:
 *      dal_rtl8373_igmp_static_router_port_get
 * Description:
 *      Get static router port
 * Input:
 *      None.
 * Output:
 *      pPortmask       - Static port mask
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_MASK       - Error parameter
 * Note:
 *      This API get static router port
 */
extern rtk_api_ret_t dal_rtl8373_igmp_static_router_port_get(rtk_portmask_t *pPortmask);

/* Function Name:
 *      dal_rtl8373_igmp_protocol_set
 * Description:
 *      set IGMP/MLD protocol action
 * Input:
 *      port        - Port ID
 *      protocol    - IGMP/MLD protocol
 *      action      - Per-port and per-protocol IGMP action seeting
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_MASK       - Error parameter
 * Note:
 *      This API set IGMP/MLD protocol action
 */
extern rtk_api_ret_t dal_rtl8373_igmp_protocol_set(rtk_port_t port, rtk_igmp_protocol_t protocol, rtk_igmp_action_t action);

/* Function Name:
 *      dal_rtl8373_igmp_protocol_get
 * Description:
 *      set IGMP/MLD protocol action
 * Input:
 *      port        - Port ID
 *      protocol    - IGMP/MLD protocol
 *      action      - Per-port and per-protocol IGMP action seeting
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_MASK       - Error parameter
 * Note:
 *      This API set IGMP/MLD protocol action
 */
extern rtk_api_ret_t dal_rtl8373_igmp_protocol_get(rtk_port_t port, rtk_igmp_protocol_t protocol, rtk_igmp_action_t *pAction);

/* Function Name:
 *      dal_rtl8373_igmp_fastLeave_set
 * Description:
 *      set IGMP/MLD FastLeave state
 * Input:
 *      state       - ENABLED: Enable FastLeave, DISABLED: disable FastLeave
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_INPUT           - Error Input
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 * Note:
 *      This API set IGMP/MLD FastLeave state
 */
extern rtk_api_ret_t dal_rtl8373_igmp_fastLeave_set(rtk_enable_t state);

/* Function Name:
 *      dal_rtl8373_igmp_fastLeave_get
 * Description:
 *      get IGMP/MLD FastLeave state
 * Input:
 *      None
 * Output:
 *      pState      - ENABLED: Enable FastLeave, DISABLED: disable FastLeave
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_NULL_POINTER    - NULL pointer
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 * Note:
 *      This API get IGMP/MLD FastLeave state
 */
extern rtk_api_ret_t dal_rtl8373_igmp_fastLeave_get(rtk_enable_t *pState);

/* Function Name:
 *      dal_rtl8373_igmp_maxGroup_set
 * Description:
 *      Set per port multicast group learning limit.
 * Input:
 *      port        - Port ID
 *      group       - The number of multicast group learning limit.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_PORT_ID         - Error Port ID
 *      RT_ERR_OUT_OF_RANGE    - parameter out of range
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 * Note:
 *      This API set per port multicast group learning limit.
 */
extern rtk_api_ret_t dal_rtl8373_igmp_maxGroup_set(rtk_port_t port, rtk_uint32 group);

/* Function Name:
 *      dal_rtl8373_igmp_maxGroup_get
 * Description:
 *      Get per port multicast group learning limit.
 * Input:
 *      port        - Port ID
 * Output:
 *      pGroup      - The number of multicast group learning limit.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_PORT_ID         - Error Port ID
 *      RT_ERR_NULL_POINTER    - Null pointer
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 * Note:
 *      This API get per port multicast group learning limit.
 */
extern rtk_api_ret_t dal_rtl8373_igmp_maxGroup_get(rtk_port_t port, rtk_uint32 *pGroup);

/* Function Name:
 *      dal_rtl8373_igmp_currentGroup_get
 * Description:
 *      Get per port multicast group learning count.
 * Input:
 *      port        - Port ID
 * Output:
 *      pGroup      - The number of multicast group learning count.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_PORT_ID         - Error Port ID
 *      RT_ERR_NULL_POINTER    - Null pointer
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 * Note:
 *      This API get per port multicast group learning count.
 */
extern rtk_api_ret_t dal_rtl8373_igmp_currentGroup_get(rtk_port_t port, rtk_uint32 *pGroup);

/* Function Name:
 *      dal_rtl8373_igmp_tableFullAction_set
 * Description:
 *      set IGMP/MLD Table Full Action
 * Input:
 *      action      - Table Full Action
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_INPUT           - Error Input
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 * Note:
 */
extern rtk_api_ret_t dal_rtl8373_igmp_tableFullAction_set(rtk_igmp_tableFullAction_t action);

/* Function Name:
 *      dal_rtl8373_igmp_tableFullAction_get
 * Description:
 *      get IGMP/MLD Table Full Action
 * Input:
 *      None
 * Output:
 *      pAction     - Table Full Action
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_NULL_POINTER    - Null pointer
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 * Note:
 */
extern rtk_api_ret_t dal_rtl8373_igmp_tableFullAction_get(rtk_igmp_tableFullAction_t *pAction);

/* Function Name:
 *      dal_rtl8373_igmp_checksumErrorAction_set
 * Description:
 *      set IGMP/MLD Checksum Error Action
 * Input:
 *      action      - Checksum error Action
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_INPUT           - Error Input
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 * Note:
 */
extern rtk_api_ret_t dal_rtl8373_igmp_checksumErrorAction_set(rtk_igmp_checksumErrorAction_t action);

/* Function Name:
 *      dal_rtl8373_igmp_checksumErrorAction_get
 * Description:
 *      get IGMP/MLD Checksum Error Action
 * Input:
 *      None
 * Output:
 *      pAction     - Checksum error Action
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_NULL_POINTER    - Null pointer
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 * Note:
 */
extern rtk_api_ret_t dal_rtl8373_igmp_checksumErrorAction_get(rtk_igmp_checksumErrorAction_t *pAction);

/* Function Name:
 *      dal_rtl8373_igmp_leaveTimer_set
 * Description:
 *      set IGMP/MLD Leave timer
 * Input:
 *      timer       - Leave timer
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_INPUT           - Error Input
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 * Note:
 */
extern rtk_api_ret_t dal_rtl8373_igmp_leaveTimer_set(rtk_uint32 timer);

/* Function Name:
 *      dal_rtl8373_igmp_leaveTimer_get
 * Description:
 *      get IGMP/MLD Leave timer
 * Input:
 *      None
 * Output:
 *      pTimer      - Leave Timer.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_NULL_POINTER    - Null pointer
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 * Note:
 */
extern rtk_api_ret_t dal_rtl8373_igmp_leaveTimer_get(rtk_uint32 *pTimer);

/* Function Name:
 *      dal_rtl8373_igmp_queryInterval_set
 * Description:
 *      set IGMP/MLD Query Interval
 * Input:
 *      interval     - Query Interval
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_INPUT           - Error Input
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 * Note:
 */
extern rtk_api_ret_t dal_rtl8373_igmp_queryInterval_set(rtk_uint32 interval);

/* Function Name:
 *      dal_rtl8373_igmp_queryInterval_get
 * Description:
 *      get IGMP/MLD Query Interval
 * Input:
 *      None.
 * Output:
 *      pInterval   - Query Interval
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_NULL_POINTER    - Null pointer
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 * Note:
 */
extern rtk_api_ret_t dal_rtl8373_igmp_queryInterval_get(rtk_uint32 *pInterval);

/* Function Name:
 *      dal_rtl8373_igmp_robustness_set
 * Description:
 *      set IGMP/MLD Robustness value
 * Input:
 *      robustness     - Robustness value
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_INPUT           - Error Input
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 * Note:
 */
extern rtk_api_ret_t dal_rtl8373_igmp_robustness_set(rtk_uint32 robustness);

/* Function Name:
 *      dal_rtl8373_igmp_robustness_get
 * Description:
 *      get IGMP/MLD Robustness value
 * Input:
 *      None
 * Output:
 *      pRobustness     - Robustness value.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_NULL_POINTER    - Null pointer
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 * Note:
 */
extern rtk_api_ret_t dal_rtl8373_igmp_robustness_get(rtk_uint32 *pRobustness);

/* Function Name:
 *      dal_rtl8373_igmp_dynamicRouterRortAllow_set
 * Description:
 *      Configure dynamic router port allow option
 * Input:
 *      pPortmask    - Dynamic Port allow mask
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_MASK       - Error parameter
 * Note:
 *
 */
extern rtk_api_ret_t dal_rtl8373_igmp_dynamicRouterPortAllow_set(rtk_portmask_t *pPortmask);

/* Function Name:
 *      dal_rtl8373_igmp_dynamicRouterRortAllow_get
 * Description:
 *      Get dynamic router port allow option
 * Input:
 *      None.
 * Output:
 *      pPortmask    - Dynamic Port allow mask
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_MASK       - Error parameter
 * Note:
 *
 */
extern rtk_api_ret_t dal_rtl8373_igmp_dynamicRouterPortAllow_get(rtk_portmask_t *pPortmask);

/* Function Name:
 *      dal_rtl8373_igmp_dynamicRouterPort_get
 * Description:
 *      Get dynamic router port
 * Input:
 *      None.
 * Output:
 *      pDynamicRouterPort    - Dynamic Router Port
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_NULL_POINTER    - Null pointer
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_MASK       - Error parameter
 * Note:
 *
 */
extern rtk_api_ret_t dal_rtl8373_igmp_dynamicRouterPort_get(rtk_igmp_dynamicRouterPort_t *pDynamicRouterPort);

/* Function Name:
 *      dal_rtl8373_igmp_suppressionEnable_set
 * Description:
 *      Configure IGMPv1/v2 & MLDv1 Report/Leave/Done suppression
 * Input:
 *      reportSuppression   - Report suppression
 *      leaveSuppression    - Leave suppression
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT           - Error Input
 * Note:
 *
 */
extern rtk_api_ret_t dal_rtl8373_igmp_suppressionEnable_set(rtk_enable_t reportSuppression, rtk_enable_t leaveSuppression);

/* Function Name:
 *      dal_rtl8373_igmp_suppressionEnable_get
 * Description:
 *      Get IGMPv1/v2 & MLDv1 Report/Leave/Done suppression
 * Input:
 *      None
 * Output:
 *      pReportSuppression  - Report suppression
 *      pLeaveSuppression   - Leave suppression
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_NULL_POINTER    - Null pointer
 * Note:
 *
 */
extern rtk_api_ret_t dal_rtl8373_igmp_suppressionEnable_get(rtk_enable_t *pReportSuppression, rtk_enable_t *pLeaveSuppression);

/* Function Name:
 *      dal_rtl8373_igmp_portRxPktEnable_set
 * Description:
 *      Configure IGMP/MLD RX Packet configuration
 * Input:
 *      port       - Port ID
 *      pRxCfg     - RX Packet Configuration
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT           - Error Input
 *      RT_ERR_NULL_POINTER    - Null pointer
 * Note:
 *
 */
extern rtk_api_ret_t dal_rtl8373_igmp_portRxPktEnable_set(rtk_port_t port, rtk_igmp_rxPktEnable_t *pRxCfg);

/* Function Name:
 *      dal_rtl8373_igmp_portRxPktEnable_get
 * Description:
 *      Get IGMP/MLD RX Packet configuration
 * Input:
 *      port       - Port ID
 *      pRxCfg     - RX Packet Configuration
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT           - Error Input
 *      RT_ERR_NULL_POINTER    - Null pointer
 * Note:
 *
 */
extern rtk_api_ret_t dal_rtl8373_igmp_portRxPktEnable_get(rtk_port_t port, rtk_igmp_rxPktEnable_t *pRxCfg);
/* Function Name:
 *      dal_rtl8373_setAsicIGMPBypassStormCTRL
 * Description:
 *      Set the function of bypass strom control for IGMP/MLD packet
 * Input:
 *      bypass    - 1: Bypass, 0:not bypass
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
extern rtk_api_ret_t dal_rtl8373_setAsicIGMPBypassStormCTRL(rtk_uint32 bypass);

/* Function Name:
 *      dal_rtl8373_getAsicIGMPBypassStormCTRL
 * Description:
 *      Set the function of bypass strom control for IGMP/MLD packet
 * Input:
 *      None
 * Output:
 *      pBypass    - 1: Bypass, 0:not bypass
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 * Note:
 *      None
 */
extern rtk_api_ret_t dal_rtl8373_getAsicIGMPBypassStormCTRL(rtk_uint32 *pBypass);
/* Function Name:
 *      dal_rtl8373_igmp_groupInfo_get
 * Description:
 *      Get IGMP/MLD Group database
 * Input:
 *      indes       - Index (0~255)
 * Output:
 *      pGroup      - Group database information.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT           - Error Input
 *      RT_ERR_NULL_POINTER    - Null pointer
 * Note:
 *
 */
extern rtk_api_ret_t dal_rtl8373_igmp_groupInfo_get(rtk_uint32 index, rtk_igmp_groupInfo_t *pGroup);

/* Function Name:
 *      dal_rtl8373_igmp_ReportLeaveFwdAction_set
 * Description:
 *      Set Report Leave packet forwarding action
 * Input:
 *      action      - Action
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT           - Error Input
 * Note:
 *
 */
extern rtk_api_ret_t dal_rtl8373_igmp_ReportLeaveFwdAction_set(rtk_igmp_ReportLeaveFwdAct_t action);

/* Function Name:
 *      dal_rtl8373_igmp_ReportLeaveFwdAction_get
 * Description:
 *      Get Report Leave packet forwarding action
 * Input:
 *      action      - Action
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT           - Error Input
 *      RT_ERR_NULL_POINTER    - Null Pointer
 * Note:
 *
 */
extern rtk_api_ret_t dal_rtl8373_igmp_ReportLeaveFwdAction_get(rtk_igmp_ReportLeaveFwdAct_t *pAction);

/* Function Name:
 *      dal_rtl8373_igmp_dropLeaveZeroEnable_set
 * Description:
 *      Set the function of droppping Leave packet with group IP = 0.0.0.0
 * Input:
 *      enabled      - Action 1: drop, 0:pass
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT           - Error Input
 * Note:
 *
 */
extern rtk_api_ret_t dal_rtl8373_igmp_dropLeaveZeroEnable_set(rtk_enable_t enabled);

/* Function Name:
 *      dal_rtl8373_igmp_dropLeaveZeroEnable_get
 * Description:
 *      Get the function of droppping Leave packet with group IP = 0.0.0.0
 * Input:
 *      None
 * Output:
 *      pEnabled.   - Action 1: drop, 0:pass
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT           - Error Input
 *      RT_ERR_NULL_POINTER    - Null Pointer
 * Note:
 *
 */
extern rtk_api_ret_t dal_rtl8373_igmp_dropLeaveZeroEnable_get(rtk_enable_t *pEnabled);

/* Function Name:
 *      dal_rtl8373_igmp_bypassGroupRange_set
 * Description:
 *      Set Bypass group
 * Input:
 *      group       - bypassed group
 *      enabled     - enabled 1: Bypassed, 0: not bypass
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT           - Error Input
 * Note:
 *
 */
extern rtk_api_ret_t dal_rtl8373_igmp_bypassGroupRange_set(rtk_igmp_bypassGroup_t group, rtk_enable_t enabled);

/* Function Name:
 *      dal_rtl8373_igmp_bypassGroupRange_get
 * Description:
 *      get Bypass group
 * Input:
 *      group       - bypassed group
 * Output:
 *      pEnable     - enabled 1: Bypassed, 0: not bypass
 * Return:
 *      RT_ERR_OK              - OK
 *      RT_ERR_FAILED          - Failed
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_INPUT           - Error Input
 *      RT_ERR_NULL_POINTER    - Null Pointer
 * Note:
 *
 */
extern rtk_api_ret_t dal_rtl8373_igmp_bypassGroupRange_get(rtk_igmp_bypassGroup_t group, rtk_enable_t *pEnable);

#endif /* __DAL_RTL8373_IGMP_H__ */

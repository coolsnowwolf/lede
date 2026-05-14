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
 * Purpose : RTL8367/RTL8367C switch high-level API
 *
 * Feature : The file includes IGMP module high-layer API defination
 *
 */

#ifndef __RTK_API_IGMP_H__
#define __RTK_API_IGMP_H__

/*
 * Data Type Declaration
 */
typedef enum rtk_igmp_type_e
{
    IGMP_IPV4 = 0,
    IGMP_PPPOE_IPV4,
    IGMP_MLD,
    IGMP_PPPOE_MLD,
    IGMP_TYPE_END
} rtk_igmp_type_t;

typedef enum rtk_trap_igmp_action_e
{
    IGMP_ACTION_FORWARD = 0,
    IGMP_ACTION_TRAP2CPU,
    IGMP_ACTION_DROP,
    IGMP_ACTION_ASIC,
    IGMP_ACTION_END
} rtk_igmp_action_t;

typedef enum rtk_igmp_protocol_e
{
    PROTOCOL_IGMPv1 = 0,
    PROTOCOL_IGMPv2,
    PROTOCOL_IGMPv3,
    PROTOCOL_MLDv1,
    PROTOCOL_MLDv2,
    PROTOCOL_END
} rtk_igmp_protocol_t;

typedef enum rtk_igmp_tableFullAction_e
{
    IGMP_TABLE_FULL_FORWARD = 0,
    IGMP_TABLE_FULL_DROP,
    IGMP_TABLE_FULL_TRAP,
    IGMP_TABLE_FULL_OP_END
}rtk_igmp_tableFullAction_t;

typedef enum rtk_igmp_checksumErrorAction_e
{
    IGMP_CRC_ERR_DROP = 0,
    IGMP_CRC_ERR_TRAP,
    IGMP_CRC_ERR_FORWARD,
    IGMP_CRC_ERR_OP_END
}rtk_igmp_checksumErrorAction_t;

typedef enum rtk_igmp_bypassGroup_e
{
    IGMP_BYPASS_224_0_0_X = 0,
    IGMP_BYPASS_224_0_1_X,
    IGMP_BYPASS_239_255_255_X,
    IGMP_BYPASS_IPV6_00XX,
    IGMP_BYPASS_GROUP_END
}rtk_igmp_bypassGroup_t;


typedef struct rtk_igmp_dynamicRouterPort_s
{
    rtk_enable_t    dynamicRouterPort0Valid;
    rtk_port_t      dynamicRouterPort0;
    rtk_uint32      dynamicRouterPort0Timer;
    rtk_enable_t    dynamicRouterPort1Valid;
    rtk_port_t      dynamicRouterPort1;
    rtk_uint32      dynamicRouterPort1Timer;

}rtk_igmp_dynamicRouterPort_t;

typedef struct rtk_igmp_rxPktEnable_s
{
    rtk_enable_t rxQuery;
    rtk_enable_t rxReport;
    rtk_enable_t rxLeave;
    rtk_enable_t rxMRP;
    rtk_enable_t rxMcast;
}rtk_igmp_rxPktEnable_t;

typedef struct rtk_igmp_groupInfo_s
{
    rtk_enable_t    valid;
    rtk_portmask_t  member;
    rtk_uint32      timer[RTK_PORT_MAX];
    rtk_uint32      reportSuppFlag;
}rtk_igmp_groupInfo_t;

typedef enum rtk_igmp_ReportLeaveFwdAct_e
{
    IGMP_REPORT_LEAVE_TO_ROUTER = 0,
    IGMP_REPORT_LEAVE_TO_ALLPORT,
    IGMP_REPORT_LEAVE_TO_ROUTER_PORT_ADV,
    IGMP_REPORT_LEAVE_ACT_END
}rtk_igmp_ReportLeaveFwdAct_t;

/* Function Name:
 *      rtk_igmp_init
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
extern rtk_api_ret_t rtk_igmp_init(void);

/* Function Name:
 *      rtk_igmp_state_set
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
extern rtk_api_ret_t rtk_igmp_state_set(rtk_enable_t enabled);

/* Function Name:
 *      rtk_igmp_state_get
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
extern rtk_api_ret_t rtk_igmp_state_get(rtk_enable_t *pEnabled);

/* Function Name:
 *      rtk_igmp_static_router_port_set
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
extern rtk_api_ret_t rtk_igmp_static_router_port_set(rtk_portmask_t *pPortmask);

/* Function Name:
 *      rtk_igmp_static_router_port_get
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
extern rtk_api_ret_t rtk_igmp_static_router_port_get(rtk_portmask_t *pPortmask);

/* Function Name:
 *      rtk_igmp_protocol_set
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
extern rtk_api_ret_t rtk_igmp_protocol_set(rtk_port_t port, rtk_igmp_protocol_t protocol, rtk_igmp_action_t action);

/* Function Name:
 *      rtk_igmp_protocol_get
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
extern rtk_api_ret_t rtk_igmp_protocol_get(rtk_port_t port, rtk_igmp_protocol_t protocol, rtk_igmp_action_t *pAction);

/* Function Name:
 *      rtk_igmp_fastLeave_set
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
extern rtk_api_ret_t rtk_igmp_fastLeave_set(rtk_enable_t state);

/* Function Name:
 *      rtk_igmp_fastLeave_get
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
extern rtk_api_ret_t rtk_igmp_fastLeave_get(rtk_enable_t *pState);

/* Function Name:
 *      rtk_igmp_maxGroup_set
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
extern rtk_api_ret_t rtk_igmp_maxGroup_set(rtk_port_t port, rtk_uint32 group);

/* Function Name:
 *      rtk_igmp_maxGroup_get
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
extern rtk_api_ret_t rtk_igmp_maxGroup_get(rtk_port_t port, rtk_uint32 *pGroup);

/* Function Name:
 *      rtk_igmp_currentGroup_get
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
extern rtk_api_ret_t rtk_igmp_currentGroup_get(rtk_port_t port, rtk_uint32 *pGroup);

/* Function Name:
 *      rtk_igmp_tableFullAction_set
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
extern rtk_api_ret_t rtk_igmp_tableFullAction_set(rtk_igmp_tableFullAction_t action);

/* Function Name:
 *      rtk_igmp_tableFullAction_get
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
extern rtk_api_ret_t rtk_igmp_tableFullAction_get(rtk_igmp_tableFullAction_t *pAction);

/* Function Name:
 *      rtk_igmp_checksumErrorAction_set
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
extern rtk_api_ret_t rtk_igmp_checksumErrorAction_set(rtk_igmp_checksumErrorAction_t action);

/* Function Name:
 *      rtk_igmp_checksumErrorAction_get
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
extern rtk_api_ret_t rtk_igmp_checksumErrorAction_get(rtk_igmp_checksumErrorAction_t *pAction);

/* Function Name:
 *      rtk_igmp_leaveTimer_set
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
extern rtk_api_ret_t rtk_igmp_leaveTimer_set(rtk_uint32 timer);

/* Function Name:
 *      rtk_igmp_leaveTimer_get
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
extern rtk_api_ret_t rtk_igmp_leaveTimer_get(rtk_uint32 *pTimer);

/* Function Name:
 *      rtk_igmp_queryInterval_set
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
extern rtk_api_ret_t rtk_igmp_queryInterval_set(rtk_uint32 interval);

/* Function Name:
 *      rtk_igmp_queryInterval_get
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
extern rtk_api_ret_t rtk_igmp_queryInterval_get(rtk_uint32 *pInterval);

/* Function Name:
 *      rtk_igmp_robustness_set
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
extern rtk_api_ret_t rtk_igmp_robustness_set(rtk_uint32 robustness);

/* Function Name:
 *      rtk_igmp_robustness_get
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
extern rtk_api_ret_t rtk_igmp_robustness_get(rtk_uint32 *pRobustness);

/* Function Name:
 *      rtk_igmp_dynamicRouterRortAllow_set
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
extern rtk_api_ret_t rtk_igmp_dynamicRouterPortAllow_set(rtk_portmask_t *pPortmask);

/* Function Name:
 *      rtk_igmp_dynamicRouterRortAllow_get
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
extern rtk_api_ret_t rtk_igmp_dynamicRouterPortAllow_get(rtk_portmask_t *pPortmask);

/* Function Name:
 *      rtk_igmp_dynamicRouterPort_get
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
extern rtk_api_ret_t rtk_igmp_dynamicRouterPort_get(rtk_igmp_dynamicRouterPort_t *pDynamicRouterPort);

/* Function Name:
 *      rtk_igmp_suppressionEnable_set
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
extern rtk_api_ret_t rtk_igmp_suppressionEnable_set(rtk_enable_t reportSuppression, rtk_enable_t leaveSuppression);

/* Function Name:
 *      rtk_igmp_suppressionEnable_get
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
extern rtk_api_ret_t rtk_igmp_suppressionEnable_get(rtk_enable_t *pReportSuppression, rtk_enable_t *pLeaveSuppression);

/* Function Name:
 *      rtk_igmp_portRxPktEnable_set
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
extern rtk_api_ret_t rtk_igmp_portRxPktEnable_set(rtk_port_t port, rtk_igmp_rxPktEnable_t *pRxCfg);

/* Function Name:
 *      rtk_igmp_portRxPktEnable_get
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
extern rtk_api_ret_t rtk_igmp_portRxPktEnable_get(rtk_port_t port, rtk_igmp_rxPktEnable_t *pRxCfg);

/* Function Name:
 *      rtk_igmp_groupInfo_get
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
extern rtk_api_ret_t rtk_igmp_groupInfo_get(rtk_uint32 index, rtk_igmp_groupInfo_t *pGroup);

/* Function Name:
 *      rtk_igmp_ReportLeaveFwdAction_set
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
extern rtk_api_ret_t rtk_igmp_ReportLeaveFwdAction_set(rtk_igmp_ReportLeaveFwdAct_t action);

/* Function Name:
 *      rtk_igmp_ReportLeaveFwdAction_get
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
extern rtk_api_ret_t rtk_igmp_ReportLeaveFwdAction_get(rtk_igmp_ReportLeaveFwdAct_t *pAction);

/* Function Name:
 *      rtk_igmp_dropLeaveZeroEnable_set
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
extern rtk_api_ret_t rtk_igmp_dropLeaveZeroEnable_set(rtk_enable_t enabled);

/* Function Name:
 *      rtk_igmp_dropLeaveZeroEnable_get
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
extern rtk_api_ret_t rtk_igmp_dropLeaveZeroEnable_get(rtk_enable_t *pEnabled);

/* Function Name:
 *      rtk_igmp_bypassGroupRange_set
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
extern rtk_api_ret_t rtk_igmp_bypassGroupRange_set(rtk_igmp_bypassGroup_t group, rtk_enable_t enabled);

/* Function Name:
 *      rtk_igmp_bypassGroupRange_get
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
extern rtk_api_ret_t rtk_igmp_bypassGroupRange_get(rtk_igmp_bypassGroup_t group, rtk_enable_t *pEnable);

#endif /* __RTK_API_IGMP_H__ */

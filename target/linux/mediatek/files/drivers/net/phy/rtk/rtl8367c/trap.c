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
 * Feature : Here is a list of all functions and variables in Trap module.
 *
 */

#include <rtk_switch.h>
#include <rtk_error.h>
#include <trap.h>
#include <string.h>

#include <rtl8367c_asicdrv.h>
#include <rtl8367c_asicdrv_port.h>
#include <rtl8367c_asicdrv_igmp.h>
#include <rtl8367c_asicdrv_rma.h>
#include <rtl8367c_asicdrv_eav.h>
#include <rtl8367c_asicdrv_oam.h>
#include <rtl8367c_asicdrv_svlan.h>
#include <rtl8367c_asicdrv_unknownMulticast.h>
#include <rtl8367c_asicdrv_dot1x.h>

/* Function Name:
 *      rtk_trap_unknownUnicastPktAction_set
 * Description:
 *      Set unknown unicast packet action configuration.
 * Input:
 *      port            - ingress port ID for unknown unicast packet
 *      ucast_action    - Unknown unicast action.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                  - OK
 *      RT_ERR_FAILED              - Failed
 *      RT_ERR_SMI                 - SMI access error
 *      RT_ERR_NOT_ALLOWED         - Invalid action.
 *      RT_ERR_INPUT               - Invalid input parameters.
 * Note:
 *      This API can set unknown unicast packet action configuration.
 *      The unknown unicast action is as following:
 *          - UCAST_ACTION_FORWARD_PMASK
 *          - UCAST_ACTION_DROP
 *          - UCAST_ACTION_TRAP2CPU
 *          - UCAST_ACTION_FLOODING
 */
rtk_api_ret_t rtk_trap_unknownUnicastPktAction_set(rtk_port_t port, rtk_trap_ucast_action_t ucast_action)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* check port valid */
    RTK_CHK_PORT_VALID(port);

    if (ucast_action >= UCAST_ACTION_END)
        return RT_ERR_INPUT;

    if ((retVal = rtl8367c_setAsicPortUnknownDaBehavior(rtk_switch_port_L2P_get(port), ucast_action)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_trap_unknownUnicastPktAction_get
 * Description:
 *      Get unknown unicast packet action configuration.
 * Input:
 *      port            - ingress port ID for unknown unicast packet
 * Output:
 *      pUcast_action   - Unknown unicast action.
 * Return:
 *      RT_ERR_OK                  - OK
 *      RT_ERR_FAILED              - Failed
 *      RT_ERR_SMI                 - SMI access error
 *      RT_ERR_NOT_ALLOWED         - Invalid action.
 *      RT_ERR_INPUT               - Invalid input parameters.
 *      RT_ERR_NULL_POINTER        - Null pointer
 * Note:
 *      This API can get unknown unicast packet action configuration.
 *      The unknown unicast action is as following:
 *          - UCAST_ACTION_FORWARD_PMASK
 *          - UCAST_ACTION_DROP
 *          - UCAST_ACTION_TRAP2CPU
 *          - UCAST_ACTION_FLOODING
 */
rtk_api_ret_t rtk_trap_unknownUnicastPktAction_get(rtk_port_t port, rtk_trap_ucast_action_t *pUcast_action)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* check port valid */
    RTK_CHK_PORT_VALID(port);

    if (NULL == pUcast_action)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getAsicPortUnknownDaBehavior(rtk_switch_port_L2P_get(port), pUcast_action)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_trap_unknownMacPktAction_set
 * Description:
 *      Set unknown source MAC packet action configuration.
 * Input:
 *      ucast_action    - Unknown source MAC action.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                  - OK
 *      RT_ERR_FAILED              - Failed
 *      RT_ERR_SMI                 - SMI access error
 *      RT_ERR_NOT_ALLOWED         - Invalid action.
 *      RT_ERR_INPUT               - Invalid input parameters.
 * Note:
 *      This API can set unknown unicast packet action configuration.
 *      The unknown unicast action is as following:
 *          - UCAST_ACTION_FORWARD_PMASK
 *          - UCAST_ACTION_DROP
 *          - UCAST_ACTION_TRAP2CPU
 */
rtk_api_ret_t rtk_trap_unknownMacPktAction_set(rtk_trap_ucast_action_t ucast_action)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (ucast_action >= UCAST_ACTION_FLOODING)
        return RT_ERR_INPUT;

    if ((retVal = rtl8367c_setAsicPortUnknownSaBehavior(ucast_action)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_trap_unknownMacPktAction_get
 * Description:
 *      Get unknown source MAC packet action configuration.
 * Input:
 *      None.
 * Output:
 *      pUcast_action   - Unknown source MAC action.
 * Return:
 *      RT_ERR_OK                  - OK
 *      RT_ERR_FAILED              - Failed
 *      RT_ERR_SMI                 - SMI access error
 *      RT_ERR_NULL_POINTER        - Null Pointer.
 *      RT_ERR_INPUT               - Invalid input parameters.
 * Note:
 *
 */
rtk_api_ret_t rtk_trap_unknownMacPktAction_get(rtk_trap_ucast_action_t *pUcast_action)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pUcast_action)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getAsicPortUnknownSaBehavior(pUcast_action)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_trap_unmatchMacPktAction_set
 * Description:
 *      Set unmatch source MAC packet action configuration.
 * Input:
 *      ucast_action    - Unknown source MAC action.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                  - OK
 *      RT_ERR_FAILED              - Failed
 *      RT_ERR_SMI                 - SMI access error
 *      RT_ERR_NOT_ALLOWED         - Invalid action.
 *      RT_ERR_INPUT               - Invalid input parameters.
 * Note:
 *      This API can set unknown unicast packet action configuration.
 *      The unknown unicast action is as following:
 *          - UCAST_ACTION_FORWARD_PMASK
 *          - UCAST_ACTION_DROP
 *          - UCAST_ACTION_TRAP2CPU
 */
rtk_api_ret_t rtk_trap_unmatchMacPktAction_set(rtk_trap_ucast_action_t ucast_action)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (ucast_action >= UCAST_ACTION_FLOODING)
        return RT_ERR_INPUT;

    if ((retVal = rtl8367c_setAsicPortUnmatchedSaBehavior(ucast_action)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_trap_unmatchMacPktAction_get
 * Description:
 *      Get unmatch source MAC packet action configuration.
 * Input:
 *      None.
 * Output:
 *      pUcast_action   - Unknown source MAC action.
 * Return:
 *      RT_ERR_OK                  - OK
 *      RT_ERR_FAILED              - Failed
 *      RT_ERR_SMI                 - SMI access error
 *      RT_ERR_NOT_ALLOWED         - Invalid action.
 *      RT_ERR_INPUT               - Invalid input parameters.
 * Note:
 *      This API can set unknown unicast packet action configuration.
 *      The unknown unicast action is as following:
 *          - UCAST_ACTION_FORWARD_PMASK
 *          - UCAST_ACTION_DROP
 *          - UCAST_ACTION_TRAP2CPU
 */
rtk_api_ret_t rtk_trap_unmatchMacPktAction_get(rtk_trap_ucast_action_t *pUcast_action)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pUcast_action)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getAsicPortUnmatchedSaBehavior(pUcast_action)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_trap_unmatchMacMoving_set
 * Description:
 *      Set unmatch source MAC packet moving state.
 * Input:
 *      port        - Port ID.
 *      enable      - ENABLED: allow SA moving, DISABLE: don't allow SA moving.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                  - OK
 *      RT_ERR_FAILED              - Failed
 *      RT_ERR_SMI                 - SMI access error
 *      RT_ERR_NOT_ALLOWED         - Invalid action.
 *      RT_ERR_INPUT               - Invalid input parameters.
 * Note:
 */
rtk_api_ret_t rtk_trap_unmatchMacMoving_set(rtk_port_t port, rtk_enable_t enable)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* check port valid */
    RTK_CHK_PORT_VALID(port);

    if(enable >= RTK_ENABLE_END)
        return RT_ERR_INPUT;

    if ((retVal = rtl8367c_setAsicPortUnmatchedSaMoving(rtk_switch_port_L2P_get(port), enable)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_trap_unmatchMacMoving_get
 * Description:
 *      Set unmatch source MAC packet moving state.
 * Input:
 *      port        - Port ID.
 * Output:
 *      pEnable     - ENABLED: allow SA moving, DISABLE: don't allow SA moving.
 * Return:
 *      RT_ERR_OK                  - OK
 *      RT_ERR_FAILED              - Failed
 *      RT_ERR_SMI                 - SMI access error
 *      RT_ERR_NOT_ALLOWED         - Invalid action.
 *      RT_ERR_INPUT               - Invalid input parameters.
 * Note:
 */
rtk_api_ret_t rtk_trap_unmatchMacMoving_get(rtk_port_t port, rtk_enable_t *pEnable)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* check port valid */
    RTK_CHK_PORT_VALID(port);

    if(NULL == pEnable)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getAsicPortUnmatchedSaMoving(rtk_switch_port_L2P_get(port), pEnable)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_trap_unknownMcastPktAction_set
 * Description:
 *      Set behavior of unknown multicast
 * Input:
 *      port            - Port id.
 *      type            - unknown multicast packet type.
 *      mcast_action    - unknown multicast action.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID         - Invalid port number.
 *      RT_ERR_NOT_ALLOWED     - Invalid action.
 *      RT_ERR_INPUT         - Invalid input parameters.
 * Note:
 *      When receives an unknown multicast packet, switch may trap, drop or flood this packet
 *      (1) The unknown multicast packet type is as following:
 *          - MCAST_L2
 *          - MCAST_IPV4
 *          - MCAST_IPV6
 *      (2) The unknown multicast action is as following:
 *          - MCAST_ACTION_FORWARD
 *          - MCAST_ACTION_DROP
 *          - MCAST_ACTION_TRAP2CPU
 */
rtk_api_ret_t rtk_trap_unknownMcastPktAction_set(rtk_port_t port, rtk_mcast_type_t type, rtk_trap_mcast_action_t mcast_action)
{
    rtk_api_ret_t retVal;
    rtk_uint32 rawAction;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);

    if (type >= MCAST_END)
        return RT_ERR_INPUT;

    if (mcast_action >= MCAST_ACTION_END)
        return RT_ERR_INPUT;


    switch (type)
    {
        case MCAST_L2:
            if (MCAST_ACTION_ROUTER_PORT == mcast_action)
                return RT_ERR_INPUT;
            else if(MCAST_ACTION_DROP_EX_RMA == mcast_action)
                rawAction = L2_UNKOWN_MULTICAST_DROP_EXCLUDE_RMA;
            else
                rawAction = (rtk_uint32)mcast_action;

            if ((retVal = rtl8367c_setAsicUnknownL2MulticastBehavior(rtk_switch_port_L2P_get(port), rawAction)) != RT_ERR_OK)
                return retVal;

            break;
        case MCAST_IPV4:
            if (MCAST_ACTION_DROP_EX_RMA == mcast_action)
                return RT_ERR_INPUT;
            else
                rawAction = (rtk_uint32)mcast_action;

            if ((retVal = rtl8367c_setAsicUnknownIPv4MulticastBehavior(rtk_switch_port_L2P_get(port), rawAction)) != RT_ERR_OK)
                return retVal;

            break;
        case MCAST_IPV6:
            if (MCAST_ACTION_DROP_EX_RMA == mcast_action)
                return RT_ERR_INPUT;
            else
                rawAction = (rtk_uint32)mcast_action;

            if ((retVal = rtl8367c_setAsicUnknownIPv6MulticastBehavior(rtk_switch_port_L2P_get(port), rawAction)) != RT_ERR_OK)
                return retVal;

            break;
        default:
            break;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_trap_unknownMcastPktAction_get
 * Description:
 *      Get behavior of unknown multicast
 * Input:
 *      type - unknown multicast packet type.
 * Output:
 *      pMcast_action - unknown multicast action.
 * Return:
 *      RT_ERR_OK                  - OK
 *      RT_ERR_FAILED              - Failed
 *      RT_ERR_SMI                 - SMI access error
 *      RT_ERR_PORT_ID             - Invalid port number.
 *      RT_ERR_NOT_ALLOWED         - Invalid operation.
 *      RT_ERR_INPUT             - Invalid input parameters.
 * Note:
 *      When receives an unknown multicast packet, switch may trap, drop or flood this packet
 *      (1) The unknown multicast packet type is as following:
 *          - MCAST_L2
 *          - MCAST_IPV4
 *          - MCAST_IPV6
 *      (2) The unknown multicast action is as following:
 *          - MCAST_ACTION_FORWARD
 *          - MCAST_ACTION_DROP
 *          - MCAST_ACTION_TRAP2CPU
 */
rtk_api_ret_t rtk_trap_unknownMcastPktAction_get(rtk_port_t port, rtk_mcast_type_t type, rtk_trap_mcast_action_t *pMcast_action)
{
    rtk_api_ret_t retVal;
    rtk_uint32 rawAction;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);

    if (type >= MCAST_END)
        return RT_ERR_INPUT;

    if(NULL == pMcast_action)
        return RT_ERR_NULL_POINTER;

    switch (type)
    {
        case MCAST_L2:
            if ((retVal = rtl8367c_getAsicUnknownL2MulticastBehavior(rtk_switch_port_L2P_get(port), &rawAction)) != RT_ERR_OK)
                return retVal;

            if(L2_UNKOWN_MULTICAST_DROP_EXCLUDE_RMA == rawAction)
                *pMcast_action = MCAST_ACTION_DROP_EX_RMA;
            else
                *pMcast_action = (rtk_trap_mcast_action_t)rawAction;

            break;
        case MCAST_IPV4:
            if ((retVal = rtl8367c_getAsicUnknownIPv4MulticastBehavior(rtk_switch_port_L2P_get(port), &rawAction)) != RT_ERR_OK)
                return retVal;

            *pMcast_action = (rtk_trap_mcast_action_t)rawAction;
            break;
        case MCAST_IPV6:
            if ((retVal = rtl8367c_getAsicUnknownIPv6MulticastBehavior(rtk_switch_port_L2P_get(port), &rawAction)) != RT_ERR_OK)
                return retVal;

            *pMcast_action = (rtk_trap_mcast_action_t)rawAction;
            break;
        default:
            break;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_trap_lldpEnable_set
 * Description:
 *      Set LLDP enable.
 * Input:
 *      enabled - LLDP enable, 0: follow RMA, 1: use LLDP action.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                  - OK
 *      RT_ERR_FAILED              - Failed
 *      RT_ERR_SMI                 - SMI access error
 *      RT_ERR_NOT_ALLOWED         - Invalid action.
 *      RT_ERR_INPUT             - Invalid input parameters.
 * Note:
 *      - DMAC                                                 Assignment
 *      - 01:80:c2:00:00:0e ethertype = 0x88CC    LLDP
 *      - 01:80:c2:00:00:03 ethertype = 0x88CC
 *      - 01:80:c2:00:00:00 ethertype = 0x88CC

 */
rtk_api_ret_t rtk_trap_lldpEnable_set(rtk_enable_t enabled)
{
    rtk_api_ret_t retVal;
    rtl8367c_rma_t rmacfg;
    rtk_enable_t tmp;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (enabled >= RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    if ((retVal = rtl8367c_getAsicRmaLldp(&tmp, &rmacfg)) != RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8367c_setAsicRmaLldp(enabled, &rmacfg)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_trap_lldpEnable_get
 * Description:
 *      Get LLDP status.
 * Input:
 *      None
 * Output:
 *      pEnabled - LLDP enable, 0: follow RMA, 1: use LLDP action.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT         - Invalid input parameters.
 * Note:
 *      LLDP is as following definition.
 *      - DMAC                                                 Assignment
 *      - 01:80:c2:00:00:0e ethertype = 0x88CC    LLDP
 *      - 01:80:c2:00:00:03 ethertype = 0x88CC
 *      - 01:80:c2:00:00:00 ethertype = 0x88CC
 */
rtk_api_ret_t rtk_trap_lldpEnable_get(rtk_enable_t *pEnabled)
{
    rtk_api_ret_t retVal;
    rtl8367c_rma_t rmacfg;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pEnabled)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getAsicRmaLldp(pEnabled, &rmacfg)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_trap_reasonTrapToCpuPriority_set
 * Description:
 *      Set priority value of a packet that trapped to CPU port according to specific reason.
 * Input:
 *      type     - reason that trap to CPU port.
 *      priority - internal priority that is going to be set for specific trap reason.
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - Invalid input parameter
 * Note:
 *      Currently the trap reason that supported are listed as follows:
 *      - TRAP_REASON_RMA
 *      - TRAP_REASON_OAM
 *      - TRAP_REASON_1XUNAUTH
 *      - TRAP_REASON_VLANSTACK
 *      - TRAP_REASON_UNKNOWNMC
 */
rtk_api_ret_t rtk_trap_reasonTrapToCpuPriority_set(rtk_trap_reason_type_t type, rtk_pri_t priority)
{
    rtk_api_ret_t retVal;
    rtl8367c_rma_t rmacfg;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (type >= TRAP_REASON_END)
        return RT_ERR_INPUT;

    if (priority > RTL8367C_PRIMAX)
        return  RT_ERR_QOS_INT_PRIORITY;

    switch (type)
    {
        case TRAP_REASON_RMA:
            if ((retVal = rtl8367c_getAsicRma(0, &rmacfg)) != RT_ERR_OK)
                return retVal;
            rmacfg.trap_priority= priority;
            if ((retVal = rtl8367c_setAsicRma(0, &rmacfg)) != RT_ERR_OK)
                return retVal;

            break;
        case TRAP_REASON_OAM:
            if ((retVal = rtl8367c_setAsicOamCpuPri(priority)) != RT_ERR_OK)
                return retVal;

            break;
        case TRAP_REASON_1XUNAUTH:
            if ((retVal = rtl8367c_setAsic1xTrapPriority(priority)) != RT_ERR_OK)
                return retVal;

            break;
        case TRAP_REASON_VLANSTACK:
            if ((retVal = rtl8367c_setAsicSvlanTrapPriority(priority)) != RT_ERR_OK)
                return retVal;

            break;
        case TRAP_REASON_UNKNOWNMC:
            if ((retVal = rtl8367c_setAsicUnknownMulticastTrapPriority(priority)) != RT_ERR_OK)
                return retVal;

            break;
        default:
            return RT_ERR_CHIP_NOT_SUPPORTED;
    }


    return RT_ERR_OK;
}


/* Function Name:
 *      rtk_trap_reasonTrapToCpuPriority_get
 * Description:
 *      Get priority value of a packet that trapped to CPU port according to specific reason.
 * Input:
 *      type      - reason that trap to CPU port.
 * Output:
 *      pPriority - configured internal priority for such reason.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - Invalid input parameter
 *      RT_ERR_NULL_POINTER - NULL pointer
 * Note:
 *      Currently the trap reason that supported are listed as follows:
 *      - TRAP_REASON_RMA
 *      - TRAP_REASON_OAM
 *      - TRAP_REASON_1XUNAUTH
 *      - TRAP_REASON_VLANSTACK
 *      - TRAP_REASON_UNKNOWNMC
 */
rtk_api_ret_t rtk_trap_reasonTrapToCpuPriority_get(rtk_trap_reason_type_t type, rtk_pri_t *pPriority)
{
    rtk_api_ret_t retVal;
    rtl8367c_rma_t rmacfg;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (type >= TRAP_REASON_END)
        return RT_ERR_INPUT;

    if(NULL == pPriority)
        return RT_ERR_NULL_POINTER;

    switch (type)
    {
        case TRAP_REASON_RMA:
            if ((retVal = rtl8367c_getAsicRma(0, &rmacfg)) != RT_ERR_OK)
                return retVal;
            *pPriority = rmacfg.trap_priority;

            break;
        case TRAP_REASON_OAM:
            if ((retVal = rtl8367c_getAsicOamCpuPri(pPriority)) != RT_ERR_OK)
                return retVal;

            break;
        case TRAP_REASON_1XUNAUTH:
            if ((retVal = rtl8367c_getAsic1xTrapPriority(pPriority)) != RT_ERR_OK)
                return retVal;

            break;
        case TRAP_REASON_VLANSTACK:
            if ((retVal = rtl8367c_getAsicSvlanTrapPriority(pPriority)) != RT_ERR_OK)
                return retVal;

            break;
        case TRAP_REASON_UNKNOWNMC:
            if ((retVal = rtl8367c_getAsicUnknownMulticastTrapPriority(pPriority)) != RT_ERR_OK)
                return retVal;

            break;
        default:
            return RT_ERR_CHIP_NOT_SUPPORTED;

    }

    return RT_ERR_OK;
}



/* Function Name:
 *      rtk_trap_rmaAction_set
 * Description:
 *      Set Reserved multicast address action configuration.
 * Input:
 *      type    - rma type.
 *      rma_action - RMA action.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *
 *      There are 48 types of Reserved Multicast Address frame for application usage.
 *      (1)They are as following definition.
 *      - TRAP_BRG_GROUP,
 *      - TRAP_FD_PAUSE,
 *      - TRAP_SP_MCAST,
 *      - TRAP_1X_PAE,
 *      - TRAP_UNDEF_BRG_04,
 *      - TRAP_UNDEF_BRG_05,
 *      - TRAP_UNDEF_BRG_06,
 *      - TRAP_UNDEF_BRG_07,
 *      - TRAP_PROVIDER_BRIDGE_GROUP_ADDRESS,
 *      - TRAP_UNDEF_BRG_09,
 *      - TRAP_UNDEF_BRG_0A,
 *      - TRAP_UNDEF_BRG_0B,
 *      - TRAP_UNDEF_BRG_0C,
 *      - TRAP_PROVIDER_BRIDGE_GVRP_ADDRESS,
 *      - TRAP_8021AB,
 *      - TRAP_UNDEF_BRG_0F,
 *      - TRAP_BRG_MNGEMENT,
 *      - TRAP_UNDEFINED_11,
 *      - TRAP_UNDEFINED_12,
 *      - TRAP_UNDEFINED_13,
 *      - TRAP_UNDEFINED_14,
 *      - TRAP_UNDEFINED_15,
 *      - TRAP_UNDEFINED_16,
 *      - TRAP_UNDEFINED_17,
 *      - TRAP_UNDEFINED_18,
 *      - TRAP_UNDEFINED_19,
 *      - TRAP_UNDEFINED_1A,
 *      - TRAP_UNDEFINED_1B,
 *      - TRAP_UNDEFINED_1C,
 *      - TRAP_UNDEFINED_1D,
 *      - TRAP_UNDEFINED_1E,
 *      - TRAP_UNDEFINED_1F,
 *      - TRAP_GMRP,
 *      - TRAP_GVRP,
 *      - TRAP_UNDEF_GARP_22,
 *      - TRAP_UNDEF_GARP_23,
 *      - TRAP_UNDEF_GARP_24,
 *      - TRAP_UNDEF_GARP_25,
 *      - TRAP_UNDEF_GARP_26,
 *      - TRAP_UNDEF_GARP_27,
 *      - TRAP_UNDEF_GARP_28,
 *      - TRAP_UNDEF_GARP_29,
 *      - TRAP_UNDEF_GARP_2A,
 *      - TRAP_UNDEF_GARP_2B,
 *      - TRAP_UNDEF_GARP_2C,
 *      - TRAP_UNDEF_GARP_2D,
 *      - TRAP_UNDEF_GARP_2E,
 *      - TRAP_UNDEF_GARP_2F,
 *      - TRAP_CDP.
 *      - TRAP_CSSTP.
 *      - TRAP_LLDP.
 *      (2) The RMA action is as following:
 *      - RMA_ACTION_FORWARD
 *      - RMA_ACTION_TRAP2CPU
 *      - RMA_ACTION_DROP
 *      - RMA_ACTION_FORWARD_EXCLUDE_CPU
 */
rtk_api_ret_t rtk_trap_rmaAction_set(rtk_trap_type_t type, rtk_trap_rma_action_t rma_action)
{
    rtk_api_ret_t retVal;
    rtl8367c_rma_t rmacfg;
    rtk_uint32 tmp;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (type >= TRAP_END)
        return RT_ERR_INPUT;

    if (rma_action >= RMA_ACTION_END)
        return RT_ERR_RMA_ACTION;

    if (type >= 0 && type <= TRAP_UNDEF_GARP_2F)
    {
        if ((retVal = rtl8367c_getAsicRma(type, &rmacfg)) != RT_ERR_OK)
            return retVal;

        rmacfg.operation = rma_action;

        if ((retVal = rtl8367c_setAsicRma(type, &rmacfg)) != RT_ERR_OK)
            return retVal;
    }
    else if (type == TRAP_CDP)
    {
        if ((retVal = rtl8367c_getAsicRmaCdp(&rmacfg)) != RT_ERR_OK)
            return retVal;

        rmacfg.operation = rma_action;

        if ((retVal = rtl8367c_setAsicRmaCdp(&rmacfg)) != RT_ERR_OK)
            return retVal;
    }
    else if (type  == TRAP_CSSTP)
    {
        if ((retVal = rtl8367c_getAsicRmaCsstp(&rmacfg)) != RT_ERR_OK)
            return retVal;

        rmacfg.operation = rma_action;

        if ((retVal = rtl8367c_setAsicRmaCsstp(&rmacfg)) != RT_ERR_OK)
            return retVal;
    }
    else if (type  == TRAP_LLDP)
    {
        if ((retVal = rtl8367c_getAsicRmaLldp(&tmp, &rmacfg)) != RT_ERR_OK)
            return retVal;

        rmacfg.operation = rma_action;

        if ((retVal = rtl8367c_setAsicRmaLldp(tmp, &rmacfg)) != RT_ERR_OK)
            return retVal;
    }
    else
        return RT_ERR_INPUT;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_trap_rmaAction_get
 * Description:
 *      Get Reserved multicast address action configuration.
 * Input:
 *      type - rma type.
 * Output:
 *      pRma_action - RMA action.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      There are 48 types of Reserved Multicast Address frame for application usage.
 *      (1)They are as following definition.
 *      - TRAP_BRG_GROUP,
 *      - TRAP_FD_PAUSE,
 *      - TRAP_SP_MCAST,
 *      - TRAP_1X_PAE,
 *      - TRAP_UNDEF_BRG_04,
 *      - TRAP_UNDEF_BRG_05,
 *      - TRAP_UNDEF_BRG_06,
 *      - TRAP_UNDEF_BRG_07,
 *      - TRAP_PROVIDER_BRIDGE_GROUP_ADDRESS,
 *      - TRAP_UNDEF_BRG_09,
 *      - TRAP_UNDEF_BRG_0A,
 *      - TRAP_UNDEF_BRG_0B,
 *      - TRAP_UNDEF_BRG_0C,
 *      - TRAP_PROVIDER_BRIDGE_GVRP_ADDRESS,
 *      - TRAP_8021AB,
 *      - TRAP_UNDEF_BRG_0F,
 *      - TRAP_BRG_MNGEMENT,
 *      - TRAP_UNDEFINED_11,
 *      - TRAP_UNDEFINED_12,
 *      - TRAP_UNDEFINED_13,
 *      - TRAP_UNDEFINED_14,
 *      - TRAP_UNDEFINED_15,
 *      - TRAP_UNDEFINED_16,
 *      - TRAP_UNDEFINED_17,
 *      - TRAP_UNDEFINED_18,
 *      - TRAP_UNDEFINED_19,
 *      - TRAP_UNDEFINED_1A,
 *      - TRAP_UNDEFINED_1B,
 *      - TRAP_UNDEFINED_1C,
 *      - TRAP_UNDEFINED_1D,
 *      - TRAP_UNDEFINED_1E,
 *      - TRAP_UNDEFINED_1F,
 *      - TRAP_GMRP,
 *      - TRAP_GVRP,
 *      - TRAP_UNDEF_GARP_22,
 *      - TRAP_UNDEF_GARP_23,
 *      - TRAP_UNDEF_GARP_24,
 *      - TRAP_UNDEF_GARP_25,
 *      - TRAP_UNDEF_GARP_26,
 *      - TRAP_UNDEF_GARP_27,
 *      - TRAP_UNDEF_GARP_28,
 *      - TRAP_UNDEF_GARP_29,
 *      - TRAP_UNDEF_GARP_2A,
 *      - TRAP_UNDEF_GARP_2B,
 *      - TRAP_UNDEF_GARP_2C,
 *      - TRAP_UNDEF_GARP_2D,
 *      - TRAP_UNDEF_GARP_2E,
 *      - TRAP_UNDEF_GARP_2F,
 *      - TRAP_CDP.
 *      - TRAP_CSSTP.
 *      - TRAP_LLDP.
 *      (2) The RMA action is as following:
 *      - RMA_ACTION_FORWARD
 *      - RMA_ACTION_TRAP2CPU
 *      - RMA_ACTION_DROP
 *      - RMA_ACTION_FORWARD_EXCLUDE_CPU
 */
rtk_api_ret_t rtk_trap_rmaAction_get(rtk_trap_type_t type, rtk_trap_rma_action_t *pRma_action)
{
    rtk_api_ret_t retVal;
    rtl8367c_rma_t rmacfg;
    rtk_uint32 tmp;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (type >= TRAP_END)
        return RT_ERR_INPUT;

    if(NULL == pRma_action)
        return RT_ERR_NULL_POINTER;

    if (type >= 0 && type <= TRAP_UNDEF_GARP_2F)
    {
        if ((retVal = rtl8367c_getAsicRma(type, &rmacfg)) != RT_ERR_OK)
            return retVal;

        *pRma_action = rmacfg.operation;
    }
    else if (type == TRAP_CDP)
    {
        if ((retVal = rtl8367c_getAsicRmaCdp(&rmacfg)) != RT_ERR_OK)
            return retVal;

        *pRma_action = rmacfg.operation;
    }
    else if (type == TRAP_CSSTP)
    {
        if ((retVal = rtl8367c_getAsicRmaCsstp(&rmacfg)) != RT_ERR_OK)
            return retVal;

        *pRma_action = rmacfg.operation;
    }
    else if (type == TRAP_LLDP)
    {
        if ((retVal = rtl8367c_getAsicRmaLldp(&tmp,&rmacfg)) != RT_ERR_OK)
            return retVal;

        *pRma_action = rmacfg.operation;
    }
    else
        return RT_ERR_INPUT;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_trap_rmaKeepFormat_set
 * Description:
 *      Set Reserved multicast address keep format configuration.
 * Input:
 *      type    - rma type.
 *      enable - enable keep format.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_ENABLE       - Invalid IFG parameter
 * Note:
 *
 *      There are 48 types of Reserved Multicast Address frame for application usage.
 *      They are as following definition.
 *      - TRAP_BRG_GROUP,
 *      - TRAP_FD_PAUSE,
 *      - TRAP_SP_MCAST,
 *      - TRAP_1X_PAE,
 *      - TRAP_UNDEF_BRG_04,
 *      - TRAP_UNDEF_BRG_05,
 *      - TRAP_UNDEF_BRG_06,
 *      - TRAP_UNDEF_BRG_07,
 *      - TRAP_PROVIDER_BRIDGE_GROUP_ADDRESS,
 *      - TRAP_UNDEF_BRG_09,
 *      - TRAP_UNDEF_BRG_0A,
 *      - TRAP_UNDEF_BRG_0B,
 *      - TRAP_UNDEF_BRG_0C,
 *      - TRAP_PROVIDER_BRIDGE_GVRP_ADDRESS,
 *      - TRAP_8021AB,
 *      - TRAP_UNDEF_BRG_0F,
 *      - TRAP_BRG_MNGEMENT,
 *      - TRAP_UNDEFINED_11,
 *      - TRAP_UNDEFINED_12,
 *      - TRAP_UNDEFINED_13,
 *      - TRAP_UNDEFINED_14,
 *      - TRAP_UNDEFINED_15,
 *      - TRAP_UNDEFINED_16,
 *      - TRAP_UNDEFINED_17,
 *      - TRAP_UNDEFINED_18,
 *      - TRAP_UNDEFINED_19,
 *      - TRAP_UNDEFINED_1A,
 *      - TRAP_UNDEFINED_1B,
 *      - TRAP_UNDEFINED_1C,
 *      - TRAP_UNDEFINED_1D,
 *      - TRAP_UNDEFINED_1E,
 *      - TRAP_UNDEFINED_1F,
 *      - TRAP_GMRP,
 *      - TRAP_GVRP,
 *      - TRAP_UNDEF_GARP_22,
 *      - TRAP_UNDEF_GARP_23,
 *      - TRAP_UNDEF_GARP_24,
 *      - TRAP_UNDEF_GARP_25,
 *      - TRAP_UNDEF_GARP_26,
 *      - TRAP_UNDEF_GARP_27,
 *      - TRAP_UNDEF_GARP_28,
 *      - TRAP_UNDEF_GARP_29,
 *      - TRAP_UNDEF_GARP_2A,
 *      - TRAP_UNDEF_GARP_2B,
 *      - TRAP_UNDEF_GARP_2C,
 *      - TRAP_UNDEF_GARP_2D,
 *      - TRAP_UNDEF_GARP_2E,
 *      - TRAP_UNDEF_GARP_2F,
 *      - TRAP_CDP.
 *      - TRAP_CSSTP.
 *      - TRAP_LLDP.
 */
rtk_api_ret_t rtk_trap_rmaKeepFormat_set(rtk_trap_type_t type, rtk_enable_t enable)
{
    rtk_api_ret_t retVal;
    rtl8367c_rma_t rmacfg;
    rtk_uint32 tmp;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (type >= TRAP_END)
        return RT_ERR_INPUT;

    if (enable >= RTK_ENABLE_END)
        return RT_ERR_INPUT;

    if (type >= 0 && type <= TRAP_UNDEF_GARP_2F)
    {
        if ((retVal = rtl8367c_getAsicRma(type, &rmacfg)) != RT_ERR_OK)
            return retVal;

        rmacfg.keep_format = enable;

        if ((retVal = rtl8367c_setAsicRma(type, &rmacfg)) != RT_ERR_OK)
            return retVal;
    }
    else if (type == TRAP_CDP)
    {
        if ((retVal = rtl8367c_getAsicRmaCdp(&rmacfg)) != RT_ERR_OK)
            return retVal;

        rmacfg.keep_format = enable;

        if ((retVal = rtl8367c_setAsicRmaCdp(&rmacfg)) != RT_ERR_OK)
            return retVal;
    }
    else if (type  == TRAP_CSSTP)
    {
        if ((retVal = rtl8367c_getAsicRmaCsstp(&rmacfg)) != RT_ERR_OK)
            return retVal;

        rmacfg.keep_format = enable;

        if ((retVal = rtl8367c_setAsicRmaCsstp(&rmacfg)) != RT_ERR_OK)
            return retVal;
    }
    else if (type  == TRAP_LLDP)
    {
        if ((retVal = rtl8367c_getAsicRmaLldp(&tmp, &rmacfg)) != RT_ERR_OK)
            return retVal;

        rmacfg.keep_format = enable;

        if ((retVal = rtl8367c_setAsicRmaLldp(tmp, &rmacfg)) != RT_ERR_OK)
            return retVal;
    }
    else
        return RT_ERR_INPUT;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_trap_rmaKeepFormat_get
 * Description:
 *      Get Reserved multicast address action configuration.
 * Input:
 *      type - rma type.
 * Output:
 *      pEnable - keep format status.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      There are 48 types of Reserved Multicast Address frame for application usage.
 *      They are as following definition.
 *      - TRAP_BRG_GROUP,
 *      - TRAP_FD_PAUSE,
 *      - TRAP_SP_MCAST,
 *      - TRAP_1X_PAE,
 *      - TRAP_UNDEF_BRG_04,
 *      - TRAP_UNDEF_BRG_05,
 *      - TRAP_UNDEF_BRG_06,
 *      - TRAP_UNDEF_BRG_07,
 *      - TRAP_PROVIDER_BRIDGE_GROUP_ADDRESS,
 *      - TRAP_UNDEF_BRG_09,
 *      - TRAP_UNDEF_BRG_0A,
 *      - TRAP_UNDEF_BRG_0B,
 *      - TRAP_UNDEF_BRG_0C,
 *      - TRAP_PROVIDER_BRIDGE_GVRP_ADDRESS,
 *      - TRAP_8021AB,
 *      - TRAP_UNDEF_BRG_0F,
 *      - TRAP_BRG_MNGEMENT,
 *      - TRAP_UNDEFINED_11,
 *      - TRAP_UNDEFINED_12,
 *      - TRAP_UNDEFINED_13,
 *      - TRAP_UNDEFINED_14,
 *      - TRAP_UNDEFINED_15,
 *      - TRAP_UNDEFINED_16,
 *      - TRAP_UNDEFINED_17,
 *      - TRAP_UNDEFINED_18,
 *      - TRAP_UNDEFINED_19,
 *      - TRAP_UNDEFINED_1A,
 *      - TRAP_UNDEFINED_1B,
 *      - TRAP_UNDEFINED_1C,
 *      - TRAP_UNDEFINED_1D,
 *      - TRAP_UNDEFINED_1E,
 *      - TRAP_UNDEFINED_1F,
 *      - TRAP_GMRP,
 *      - TRAP_GVRP,
 *      - TRAP_UNDEF_GARP_22,
 *      - TRAP_UNDEF_GARP_23,
 *      - TRAP_UNDEF_GARP_24,
 *      - TRAP_UNDEF_GARP_25,
 *      - TRAP_UNDEF_GARP_26,
 *      - TRAP_UNDEF_GARP_27,
 *      - TRAP_UNDEF_GARP_28,
 *      - TRAP_UNDEF_GARP_29,
 *      - TRAP_UNDEF_GARP_2A,
 *      - TRAP_UNDEF_GARP_2B,
 *      - TRAP_UNDEF_GARP_2C,
 *      - TRAP_UNDEF_GARP_2D,
 *      - TRAP_UNDEF_GARP_2E,
 *      - TRAP_UNDEF_GARP_2F,
 *      - TRAP_CDP.
 *      - TRAP_CSSTP.
 *      - TRAP_LLDP.
 */
rtk_api_ret_t rtk_trap_rmaKeepFormat_get(rtk_trap_type_t type, rtk_enable_t *pEnable)
{
    rtk_api_ret_t retVal;
    rtl8367c_rma_t rmacfg;
    rtk_uint32 tmp;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (type >= TRAP_END)
        return RT_ERR_INPUT;

    if(NULL == pEnable)
        return RT_ERR_NULL_POINTER;

    if (type >= 0 && type <= TRAP_UNDEF_GARP_2F)
    {
        if ((retVal = rtl8367c_getAsicRma(type, &rmacfg)) != RT_ERR_OK)
            return retVal;

        *pEnable = rmacfg.keep_format;
    }
    else if (type == TRAP_CDP)
    {
        if ((retVal = rtl8367c_getAsicRmaCdp(&rmacfg)) != RT_ERR_OK)
            return retVal;

        *pEnable = rmacfg.keep_format;
    }
    else if (type == TRAP_CSSTP)
    {
        if ((retVal = rtl8367c_getAsicRmaCsstp(&rmacfg)) != RT_ERR_OK)
            return retVal;

        *pEnable = rmacfg.keep_format;
    }
    else if (type == TRAP_LLDP)
    {
        if ((retVal = rtl8367c_getAsicRmaLldp(&tmp,&rmacfg)) != RT_ERR_OK)
            return retVal;

        *pEnable = rmacfg.keep_format;
    }
    else
        return RT_ERR_INPUT;

    return RT_ERR_OK;
}




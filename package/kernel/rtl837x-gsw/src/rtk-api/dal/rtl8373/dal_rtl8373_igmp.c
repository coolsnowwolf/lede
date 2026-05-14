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
 * Feature : Here is a list of all functions and variables in IGMP module.
 *
 */

#include <rtk_switch.h>
#include <rtk_error.h>
#include <dal/rtl8373/dal_rtl8373_igmp.h>
#include <string.h>

#include <rtl8373_asicdrv.h>
#include <dal_rtl8373_lut.h>
//#include <rtl8373_asicdrv_lut.h>

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
rtk_api_ret_t dal_rtl8373_igmp_init(void)
{
    rtk_api_ret_t retVal;
    rtk_port_t port;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if ((retVal = dal_rtl8373_l2_ipMcastAddrLookup_set(LOOKUP_IP))!=RT_ERR_OK)
        return retVal;

    RTK_SCAN_ALL_PHY_PORTMASK(port)
    {
      
        /* IGMPv1 operation */
        retVal = rtl8373_setAsicRegBits(RTL8373_IGMP_PORT_CTRL_ADDR(port), RTL8373_IGMP_PORT_CTRL_IGMPV1_OP_MASK, PROTOCOL_OP_ASIC);
        if(retVal != RT_ERR_OK)
            return retVal;

        /* IGMPv2 operation */
        retVal = rtl8373_setAsicRegBits(RTL8373_IGMP_PORT_CTRL_ADDR(port), RTL8373_IGMP_PORT_CTRL_IGMPV2_OP_MASK, PROTOCOL_OP_ASIC);
        if(retVal != RT_ERR_OK)
            return retVal;
        
          /* IGMPv3 operation */
        retVal = rtl8373_setAsicRegBits(RTL8373_IGMP_PORT_CTRL_ADDR(port), RTL8373_IGMP_PORT_CTRL_IGMPV3_OP_MASK, PROTOCOL_OP_FLOOD);
        if(retVal != RT_ERR_OK)
            return retVal;

         /* MLDv1 operation */
        retVal = rtl8373_setAsicRegBits(RTL8373_IGMP_PORT_CTRL_ADDR(port), RTL8373_IGMP_PORT_CTRL_MLDV1_OP_MASK, PROTOCOL_OP_ASIC);
        if(retVal != RT_ERR_OK)
            return retVal;

         /* MLDv2 operation */
        retVal = rtl8373_setAsicRegBits(RTL8373_IGMP_PORT_CTRL_ADDR(port), RTL8373_IGMP_PORT_CTRL_MLDV2_OP_MASK, PROTOCOL_OP_FLOOD);
        if(retVal != RT_ERR_OK)
            return retVal;
      
    }

    if ((retVal = rtl8373_setAsicRegBits(RTL8373_IGMP_ROUTER_PORT_CRTL_ADDR, RTL8373_IGMP_ROUTER_PORT_CRTL_ALLOW_DYN_ROTR_PMSK_MASK,rtk_switch_phyPortMask_get()))!=RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8373_setAsicRegBit(RTL8373_IGMP_CTRL_ADDR, RTL8373_IGMP_CTRL_FAST_LEAVE_EN_OFFSET, ENABLED))!=RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8373_setAsicRegBits(RTL8373_IGMP_CTRL_ADDR, RTL8373_IGMP_CTRL_REPORT_LEAVE_FWD_MASK, ENABLED))!=RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8373_setAsicRegBit(RTL8373_IGMP_CTRL_ADDR, RTL8373_IGMP_CTRL_IGMP_MLD_EN_OFFSET, ENABLED))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

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
rtk_api_ret_t dal_rtl8373_igmp_state_set(rtk_enable_t enabled)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (enabled >= RTK_ENABLE_END)
        return RT_ERR_INPUT;

    if ((retVal = rtl8373_setAsicRegBit(RTL8373_IGMP_CTRL_ADDR, RTL8373_IGMP_CTRL_IGMP_MLD_EN_OFFSET, enabled))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

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
rtk_api_ret_t dal_rtl8373_igmp_state_get(rtk_enable_t *pEnabled)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(pEnabled == NULL)
        return RT_ERR_NULL_POINTER;

    if ((retVal =  rtl8373_getAsicRegBit(RTL8373_IGMP_CTRL_ADDR, RTL8373_IGMP_CTRL_IGMP_MLD_EN_OFFSET, pEnabled))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

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
rtk_api_ret_t dal_rtl8373_igmp_static_router_port_set(rtk_portmask_t *pPortmask)
{
    rtk_api_ret_t retVal;
    rtk_uint32 pmask;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Valid port mask */
    if(pPortmask == NULL)
        return RT_ERR_NULL_POINTER;

    RTK_CHK_PORTMASK_VALID(pPortmask);

    if ((retVal = rtk_switch_portmask_L2P_get(pPortmask, &pmask))!=RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8373_setAsicRegBits(RTL8373_IGMP_ROUTER_PORT_CRTL_ADDR, RTL8373_IGMP_ROUTER_PORT_CRTL_STIC_PMSK_MASK, pmask))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

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
rtk_api_ret_t dal_rtl8373_igmp_static_router_port_get(rtk_portmask_t *pPortmask)
{
    rtk_api_ret_t retVal;
    rtk_uint32 pmask;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(pPortmask == NULL)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8373_getAsicRegBits(RTL8373_IGMP_ROUTER_PORT_CRTL_ADDR, RTL8373_IGMP_ROUTER_PORT_CRTL_STIC_PMSK_MASK, &pmask))!=RT_ERR_OK)
        return retVal;

    if ((retVal = rtk_switch_portmask_P2L_get(pmask, pPortmask))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

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
rtk_api_ret_t dal_rtl8373_igmp_protocol_set(rtk_port_t port, rtk_igmp_protocol_t protocol, rtk_igmp_action_t action)
{
    rtk_uint32      operation;
    rtk_api_ret_t   retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check port valid */
    RTK_CHK_PORT_VALID(port);

    if(protocol >= PROTOCOL_END)
        return RT_ERR_INPUT;

    if(action >= IGMP_ACTION_END)
        return RT_ERR_INPUT;

    switch(action)
    {
        case IGMP_ACTION_FORWARD:
            operation = PROTOCOL_OP_FLOOD;
            break;
        case IGMP_ACTION_TRAP2CPU:
            operation = PROTOCOL_OP_TRAP;
            break;
        case IGMP_ACTION_DROP:
            operation = PROTOCOL_OP_DROP;
            break;
        case IGMP_ACTION_ASIC:
            operation = PROTOCOL_OP_ASIC;
            break;
        default:
            return RT_ERR_INPUT;
    }

    switch(protocol)
    {
        case PROTOCOL_IGMPv1:
            if ((retVal = rtl8373_setAsicRegBits(RTL8373_IGMP_PORT_CTRL_ADDR(rtk_switch_port_L2P_get(port)), RTL8373_IGMP_PORT_CTRL_IGMPV1_OP_MASK,operation))!=RT_ERR_OK)
                return retVal;
            
            break;
        case PROTOCOL_IGMPv2:
             if ((retVal = rtl8373_setAsicRegBits(RTL8373_IGMP_PORT_CTRL_ADDR(rtk_switch_port_L2P_get(port)), RTL8373_IGMP_PORT_CTRL_IGMPV2_OP_MASK,operation))!=RT_ERR_OK)
                return retVal;

            break;
        case PROTOCOL_IGMPv3:
           if ((retVal = rtl8373_setAsicRegBits(RTL8373_IGMP_PORT_CTRL_ADDR(rtk_switch_port_L2P_get(port)), RTL8373_IGMP_PORT_CTRL_IGMPV3_OP_MASK,operation))!=RT_ERR_OK)
                return retVal;

            break;
        case PROTOCOL_MLDv1:
            if ((retVal = rtl8373_setAsicRegBits(RTL8373_IGMP_PORT_CTRL_ADDR(rtk_switch_port_L2P_get(port)), RTL8373_IGMP_PORT_CTRL_MLDV1_OP_MASK,operation))!=RT_ERR_OK)
                return retVal;

            break;
        case PROTOCOL_MLDv2:
             if ((retVal = rtl8373_setAsicRegBits(RTL8373_IGMP_PORT_CTRL_ADDR(rtk_switch_port_L2P_get(port)), RTL8373_IGMP_PORT_CTRL_MLDV2_OP_MASK,operation))!=RT_ERR_OK)
                return retVal;

            break;
        default:
            return RT_ERR_INPUT;

    }

    return RT_ERR_OK;
}

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
rtk_api_ret_t dal_rtl8373_igmp_protocol_get(rtk_port_t port, rtk_igmp_protocol_t protocol, rtk_igmp_action_t *pAction)
{
    rtk_uint32      operation;
    rtk_api_ret_t   retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check port valid */
    RTK_CHK_PORT_VALID(port);

    if(protocol >= PROTOCOL_END)
        return RT_ERR_INPUT;

    if(pAction == NULL)
        return RT_ERR_NULL_POINTER;

    switch(protocol)
    {
        case PROTOCOL_IGMPv1:
            if ((retVal = rtl8373_getAsicRegBits(RTL8373_IGMP_PORT_CTRL_ADDR(rtk_switch_port_L2P_get(port)), RTL8373_IGMP_PORT_CTRL_IGMPV1_OP_MASK, &operation))!=RT_ERR_OK)
                return retVal;

            break;
        case PROTOCOL_IGMPv2:
             if ((retVal = rtl8373_getAsicRegBits(RTL8373_IGMP_PORT_CTRL_ADDR(rtk_switch_port_L2P_get(port)), RTL8373_IGMP_PORT_CTRL_IGMPV2_OP_MASK, &operation))!=RT_ERR_OK)
                return retVal;

            break;
        case PROTOCOL_IGMPv3:
            if ((retVal = rtl8373_getAsicRegBits(RTL8373_IGMP_PORT_CTRL_ADDR(rtk_switch_port_L2P_get(port)), RTL8373_IGMP_PORT_CTRL_IGMPV3_OP_MASK, &operation))!=RT_ERR_OK)
                return retVal;

            break;
        case PROTOCOL_MLDv1:
           if ((retVal = rtl8373_getAsicRegBits(RTL8373_IGMP_PORT_CTRL_ADDR(rtk_switch_port_L2P_get(port)), RTL8373_IGMP_PORT_CTRL_MLDV1_OP_MASK, &operation))!=RT_ERR_OK)
                return retVal;

            break;
        case PROTOCOL_MLDv2:
            if ((retVal = rtl8373_getAsicRegBits(RTL8373_IGMP_PORT_CTRL_ADDR(rtk_switch_port_L2P_get(port)), RTL8373_IGMP_PORT_CTRL_MLDV2_OP_MASK, &operation))!=RT_ERR_OK)
                return retVal;

            break;
        default:
            return RT_ERR_INPUT;

    }

    switch(operation)
    {
        case PROTOCOL_OP_FLOOD:
            *pAction = IGMP_ACTION_FORWARD;
            break;
        case PROTOCOL_OP_TRAP:
            *pAction = IGMP_ACTION_TRAP2CPU;
            break;
        case PROTOCOL_OP_DROP:
            *pAction = IGMP_ACTION_DROP;
            break;
        case PROTOCOL_OP_ASIC:
            *pAction = IGMP_ACTION_ASIC;
            break;
        default: 
            return RT_ERR_FAILED;

    }

    return RT_ERR_OK;
}

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
rtk_api_ret_t dal_rtl8373_igmp_fastLeave_set(rtk_enable_t state)
{
    rtk_api_ret_t   retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(state >= RTK_ENABLE_END)
        return RT_ERR_INPUT;

    if ((retVal =rtl8373_setAsicRegBit(RTL8373_IGMP_CTRL_ADDR, RTL8373_IGMP_CTRL_FAST_LEAVE_EN_OFFSET, (state >= 1) ? 1 : 0))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

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
rtk_api_ret_t dal_rtl8373_igmp_fastLeave_get(rtk_enable_t *pState)
{
    rtk_uint32      fast_leave;
    rtk_api_ret_t   retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(pState == NULL)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8373_getAsicRegBit(RTL8373_IGMP_CTRL_ADDR, RTL8373_IGMP_CTRL_FAST_LEAVE_EN_OFFSET, &fast_leave))!=RT_ERR_OK)
        return retVal;

    *pState = ((fast_leave == 1) ? ENABLED : DISABLED);
    return RT_ERR_OK;
}

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
rtk_api_ret_t dal_rtl8373_igmp_maxGroup_set(rtk_port_t port, rtk_uint32 group)
{
    rtk_api_ret_t   retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check port valid */
    RTK_CHK_PORT_VALID(port);

    if(group > RTL8373_IGMP_MAX_GOUP)
        return RT_ERR_OUT_OF_RANGE;

    if ((retVal = rtl8373_setAsicRegBits(RTL8373_IGMP_PORT_CTRL_ADDR(rtk_switch_port_L2P_get(port)), RTL8373_IGMP_PORT_CTRL_MAX_GROUP_NUM_MASK , group))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

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
rtk_api_ret_t dal_rtl8373_igmp_maxGroup_get(rtk_port_t port, rtk_uint32 *pGroup)
{
    rtk_api_ret_t   retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check port valid */
    RTK_CHK_PORT_VALID(port);

    if(pGroup == NULL)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8373_getAsicRegBits(RTL8373_IGMP_PORT_CTRL_ADDR(rtk_switch_port_L2P_get(port)), RTL8373_IGMP_PORT_CTRL_MAX_GROUP_NUM_MASK , pGroup))!=RT_ERR_OK)
        return retVal;
    
    return RT_ERR_OK;
}

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
rtk_api_ret_t dal_rtl8373_igmp_currentGroup_get(rtk_port_t port, rtk_uint32 *pGroup)
{
    rtk_api_ret_t   retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check port valid */
    RTK_CHK_PORT_VALID(port);

    if(pGroup == NULL)
        return RT_ERR_NULL_POINTER;

    retVal = rtl8373_getAsicRegBits(RTL8373_PORT_CURR_GROUP_ADDR(port), RTL8373_PORT_CURR_GROUP_NUM_MASK(port) , pGroup);
    if(retVal != RT_ERR_OK)
        return retVal;
    
    return RT_ERR_OK;
}

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
rtk_api_ret_t dal_rtl8373_igmp_tableFullAction_set(rtk_igmp_tableFullAction_t action)
{
    rtk_api_ret_t   retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(action >= IGMP_TABLE_FULL_OP_END)
        return RT_ERR_INPUT;

      /* Table full Operation */
    retVal = rtl8373_setAsicRegBits(RTL8373_IGMP_CTRL_ADDR, RTL8373_IGMP_CTRL_TABLE_FULL_OP_MASK, action);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

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
rtk_api_ret_t dal_rtl8373_igmp_tableFullAction_get(rtk_igmp_tableFullAction_t *pAction)
{
    rtk_api_ret_t   retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pAction)
        return RT_ERR_NULL_POINTER;

     /* Table full Operation */
    retVal = rtl8373_getAsicRegBits(RTL8373_IGMP_CTRL_ADDR, RTL8373_IGMP_CTRL_TABLE_FULL_OP_MASK, pAction);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

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
rtk_api_ret_t dal_rtl8373_igmp_checksumErrorAction_set(rtk_igmp_checksumErrorAction_t action)
{
    rtk_api_ret_t   retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(action >= IGMP_CRC_ERR_OP_END)
        return RT_ERR_INPUT;

        /* CRC Error Operation */
    retVal = rtl8373_setAsicRegBits(RTL8373_IGMP_CTRL_ADDR, RTL8373_IGMP_CTRL_CKS_ERR_OP_MASK, action);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

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
rtk_api_ret_t dal_rtl8373_igmp_checksumErrorAction_get(rtk_igmp_checksumErrorAction_t *pAction)
{
    rtk_api_ret_t   retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pAction)
        return RT_ERR_NULL_POINTER;

      /* CRC Error Operation */
    retVal = rtl8373_getAsicRegBits(RTL8373_IGMP_CTRL_ADDR, RTL8373_IGMP_CTRL_CKS_ERR_OP_MASK, pAction);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

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
rtk_api_ret_t dal_rtl8373_igmp_leaveTimer_set(rtk_uint32 timer)
{
    rtk_api_ret_t   retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(timer > RTL8373_MAX_LEAVE_TIMER)
        return RT_ERR_INPUT;

      /* Leave timer */
    retVal = rtl8373_setAsicRegBits(RTL8373_IGMP_CTRL_ADDR, RTL8373_IGMP_CTRL_LEAVE_TIMER_MASK, timer);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

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
rtk_api_ret_t dal_rtl8373_igmp_leaveTimer_get(rtk_uint32 *pTimer)
{
    rtk_api_ret_t   retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pTimer)
        return RT_ERR_NULL_POINTER;

       /* Leave timer */
    retVal = rtl8373_getAsicRegBits(RTL8373_IGMP_CTRL_ADDR, RTL8373_IGMP_CTRL_LEAVE_TIMER_MASK, pTimer);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

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
rtk_api_ret_t dal_rtl8373_igmp_queryInterval_set(rtk_uint32 interval)
{
    rtk_api_ret_t   retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(interval > RTL8373_MAX_QUERY_INT)
        return RT_ERR_INPUT;

        /* Query Interval */
    retVal = rtl8373_setAsicReg(RTL8373_IGMP_QUERY_INTVL_ADDR, interval);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

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
rtk_api_ret_t dal_rtl8373_igmp_queryInterval_get(rtk_uint32 *pInterval)
{
    rtk_api_ret_t   retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pInterval)
        return RT_ERR_NULL_POINTER;

     /* Query Interval */
    retVal = rtl8373_getAsicReg(RTL8373_IGMP_QUERY_INTVL_ADDR, pInterval);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

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
rtk_api_ret_t dal_rtl8373_setAsicIGMPIsoLeaky(rtk_uint32 leaky)
{
    rtk_api_ret_t   retVal;

    retVal = rtl8373_setAsicRegBit(RTL8373_IGMP_CTRL_ADDR, RTL8373_IGMP_CTRL_IGMP_MLD_PORTISO_LKY_OFFSET, leaky);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

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
rtk_api_ret_t dal_rtl8373_getAsicIGMPIsoLeaky(rtk_uint32 *pLeaky)
{
    rtk_api_ret_t   retVal;
    rtk_uint32  regData;

    retVal = rtl8373_getAsicRegBit(RTL8373_IGMP_CTRL_ADDR, RTL8373_IGMP_CTRL_IGMP_MLD_PORTISO_LKY_OFFSET, &regData);
    if(retVal != RT_ERR_OK)
        return retVal;

    *pLeaky = regData;
    return RT_ERR_OK;
}
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
rtk_api_ret_t dal_rtl8373_setAsicIGMPVLANLeaky(rtk_uint32 leaky)
{
    rtk_api_ret_t   retVal;

    retVal = rtl8373_setAsicRegBit(RTL8373_IGMP_CTRL_ADDR, RTL8373_IGMP_CTRL_IGMP_MLD_VLAN_LKY_OFFSET, leaky);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

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
rtk_api_ret_t dal_rtl8373_getAsicIGMPVLANLeaky(rtk_uint32 *pLeaky)
{
    rtk_api_ret_t   retVal;
    rtk_uint32  regData;

    retVal = rtl8373_getAsicRegBit(RTL8373_IGMP_CTRL_ADDR, RTL8373_IGMP_CTRL_IGMP_MLD_VLAN_LKY_OFFSET, &regData);
    if(retVal != RT_ERR_OK)
        return retVal;

    *pLeaky = regData;
    return RT_ERR_OK;
}
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
rtk_api_ret_t dal_rtl8373_setAsicIpMulticastVlanLeaky(rtk_uint32 port, rtk_uint32 enabled)
{
    rtk_api_ret_t  retVal;

    if(port > RTL8373_PORTIDMAX)
        return RT_ERR_PORT_ID;

    retVal = rtl8373_setAsicRegBit(RTL8373_IPMUL_NO_VLAN_EGRESS_ADDR(port),RTL8373_IPMUL_NO_VLAN_EGRESS_IPMUL_VLAN_LEAKY_OFFSET(port),enabled);

    return retVal;
}
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
rtk_api_ret_t dal_rtl8373_getAsicIpMulticastVlanLeaky(rtk_uint32 port, rtk_uint32 *ptr_enabled)
{
    rtk_api_ret_t  retVal;

    if(port > RTL8373_PORTIDMAX)
        return RT_ERR_PORT_ID;

    retVal = rtl8373_getAsicRegBit(RTL8373_IPMUL_NO_VLAN_EGRESS_ADDR(port),RTL8373_IPMUL_NO_VLAN_EGRESS_IPMUL_VLAN_LEAKY_OFFSET(port), ptr_enabled);

    return retVal;
}
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
rtk_api_ret_t dal_rtl8373_setAsicIGMPBypassStormCTRL(rtk_uint32 bypass)
{
    rtk_api_ret_t   retVal;

    retVal = rtl8373_setAsicRegBit(RTL8373_IGMP_CTRL_ADDR, RTL8373_IGMP_CTRL_IGMP_MLD_DISC_STORM_FLTR_OFFSET, bypass);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

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
rtk_api_ret_t dal_rtl8373_getAsicIGMPBypassStormCTRL(rtk_uint32 *pBypass)
{
    rtk_api_ret_t   retVal;
    rtk_uint32  regData;

    retVal = rtl8373_getAsicRegBit(RTL8373_IGMP_CTRL_ADDR, RTL8373_IGMP_CTRL_IGMP_MLD_DISC_STORM_FLTR_OFFSET, &regData);
    if(retVal != RT_ERR_OK)
        return retVal;

    *pBypass = regData;
    return RT_ERR_OK;
}
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
rtk_api_ret_t dal_rtl8373_igmp_robustness_set(rtk_uint32 robustness)
{
    rtk_api_ret_t   retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(robustness > RTL8373_MAX_ROB_VAR)
        return RT_ERR_INPUT;

     /* Bourstness variable */
    retVal = rtl8373_setAsicRegBits(RTL8373_IGMP_CTRL_ADDR, RTL8373_IGMP_CTRL_ROBUSTNESS_VAR_MASK, robustness);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

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
rtk_api_ret_t dal_rtl8373_igmp_robustness_get(rtk_uint32 *pRobustness)
{
    rtk_api_ret_t   retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pRobustness)
        return RT_ERR_NULL_POINTER;

     /* Bourstness variable */
    retVal = rtl8373_getAsicRegBits(RTL8373_IGMP_CTRL_ADDR, RTL8373_IGMP_CTRL_ROBUSTNESS_VAR_MASK, pRobustness);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

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
rtk_api_ret_t dal_rtl8373_igmp_dynamicRouterPortAllow_set(rtk_portmask_t *pPortmask)
{
    rtk_api_ret_t   retVal;
    rtk_uint32 pmask;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pPortmask)
        return RT_ERR_NULL_POINTER;

    RTK_CHK_PORTMASK_VALID(pPortmask);

    if ((retVal = rtk_switch_portmask_L2P_get(pPortmask, &pmask))!=RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8373_setAsicRegBits(RTL8373_IGMP_ROUTER_PORT_CRTL_ADDR, RTL8373_IGMP_ROUTER_PORT_CRTL_ALLOW_DYN_ROTR_PMSK_MASK,pmask))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

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
 *      RT_ERR_NULL_POINTER    - Null pointer
 *      RT_ERR_SMI             - SMI access error
 *      RT_ERR_PORT_MASK       - Error parameter
 * Note:
 *
 */
rtk_api_ret_t dal_rtl8373_igmp_dynamicRouterPortAllow_get(rtk_portmask_t *pPortmask)
{
    rtk_api_ret_t   retVal;
    rtk_uint32 pmask;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pPortmask)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8373_getAsicRegBits(RTL8373_IGMP_ROUTER_PORT_CRTL_ADDR, RTL8373_IGMP_ROUTER_PORT_CRTL_ALLOW_DYN_ROTR_PMSK_MASK,&pmask))!=RT_ERR_OK)
        return retVal;

    if ((retVal = rtk_switch_portmask_P2L_get(pmask, pPortmask))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

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
rtk_api_ret_t dal_rtl8373_igmp_dynamicRouterPort_get(rtk_igmp_dynamicRouterPort_t *pDynamicRouterPort)
{
    rtk_api_ret_t   retVal;
    rtk_uint32 port;
    rtk_uint32 timer;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pDynamicRouterPort)
        return RT_ERR_NULL_POINTER;

   retVal = rtl8373_getAsicRegBits(RTL8373_IGMP_DYN_ROUTER_INFO_ADDR, RTL8373_IGMP_DYN_ROUTER_INFO_PORT1_ID_MASK, &port);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8373_getAsicRegBits(RTL8373_IGMP_DYN_ROUTER_INFO_ADDR, RTL8373_IGMP_DYN_ROUTER_INFO_TIMER1_MASK, &timer);
    if(retVal != RT_ERR_OK)
        return retVal;

    if (port == RTL8373_ROUTER_PORT_INVALID)
    {
        pDynamicRouterPort->dynamicRouterPort0Valid = DISABLED;
        pDynamicRouterPort->dynamicRouterPort0      = 0;
        pDynamicRouterPort->dynamicRouterPort0Timer = 0;
    }
    else
    {
        pDynamicRouterPort->dynamicRouterPort0Valid = ENABLED;
        pDynamicRouterPort->dynamicRouterPort0      = rtk_switch_port_P2L_get(port);
        pDynamicRouterPort->dynamicRouterPort0Timer = timer;
    }

     retVal = rtl8373_getAsicRegBits(RTL8373_IGMP_DYN_ROUTER_INFO_ADDR, RTL8373_IGMP_DYN_ROUTER_INFO_PORT2_ID_MASK, &port);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8373_getAsicRegBits(RTL8373_IGMP_DYN_ROUTER_INFO_ADDR, RTL8373_IGMP_DYN_ROUTER_INFO_TIMER2_MASK, &timer);
    if(retVal != RT_ERR_OK)
        return retVal;

    if (port == RTL8373_ROUTER_PORT_INVALID)
    {
        pDynamicRouterPort->dynamicRouterPort1Valid = DISABLED;
        pDynamicRouterPort->dynamicRouterPort1      = 0;
        pDynamicRouterPort->dynamicRouterPort1Timer = 0;
    }
    else
    {
        pDynamicRouterPort->dynamicRouterPort1Valid = ENABLED;
        pDynamicRouterPort->dynamicRouterPort1      = rtk_switch_port_P2L_get(port);
        pDynamicRouterPort->dynamicRouterPort1Timer = timer;
    }

    return RT_ERR_OK;
}

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
rtk_api_ret_t dal_rtl8373_igmp_suppressionEnable_set(rtk_enable_t reportSuppression, rtk_enable_t leaveSuppression)
{
    rtk_api_ret_t   retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(reportSuppression >= RTK_ENABLE_END)
        return RT_ERR_INPUT;

    if(leaveSuppression >= RTK_ENABLE_END)
        return RT_ERR_INPUT;

     retVal = rtl8373_setAsicRegBits(RTL8373_IGMP_CTRL_ADDR, RTL8373_IGMP_CTRL_REPORT_SUPPRESSION_MASK, reportSuppression);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8373_setAsicRegBits(RTL8373_IGMP_CTRL_ADDR, RTL8373_IGMP_CTRL_LEAVE_SUPPRESSION_MASK, leaveSuppression);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

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
rtk_api_ret_t dal_rtl8373_igmp_suppressionEnable_get(rtk_enable_t *pReportSuppression, rtk_enable_t *pLeaveSuppression)
{
    rtk_api_ret_t   retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pReportSuppression)
        return RT_ERR_NULL_POINTER;

    if(NULL == pLeaveSuppression)
        return RT_ERR_NULL_POINTER;

     retVal = rtl8373_getAsicRegBits(RTL8373_IGMP_CTRL_ADDR, RTL8373_IGMP_CTRL_REPORT_SUPPRESSION_MASK, pReportSuppression);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8373_getAsicRegBits(RTL8373_IGMP_CTRL_ADDR, RTL8373_IGMP_CTRL_LEAVE_SUPPRESSION_MASK, pLeaveSuppression);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

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
rtk_api_ret_t dal_rtl8373_igmp_portRxPktEnable_set(rtk_port_t port, rtk_igmp_rxPktEnable_t *pRxCfg)
{
    rtk_api_ret_t   retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check port valid */
    RTK_CHK_PORT_VALID(port);

    if(NULL == pRxCfg)
        return RT_ERR_NULL_POINTER;

    if(pRxCfg->rxQuery >= RTK_ENABLE_END)
        return RT_ERR_INPUT;

    if(pRxCfg->rxReport >= RTK_ENABLE_END)
        return RT_ERR_INPUT;

    if(pRxCfg->rxLeave >= RTK_ENABLE_END)
        return RT_ERR_INPUT;

    if(pRxCfg->rxMRP >= RTK_ENABLE_END)
        return RT_ERR_INPUT;

    if(pRxCfg->rxMcast >= RTK_ENABLE_END)
        return RT_ERR_INPUT;

    retVal = rtl8373_setAsicRegBits(RTL8373_IGMP_PORT_CTRL_ADDR(rtk_switch_port_L2P_get(port)), RTL8373_IGMP_PORT_CTRL_ALLOW_QUERY_MASK, (rtk_uint32)pRxCfg->rxQuery);
        if(retVal != RT_ERR_OK)
            return retVal;
    retVal = rtl8373_setAsicRegBits(RTL8373_IGMP_PORT_CTRL_ADDR(rtk_switch_port_L2P_get(port)), RTL8373_IGMP_PORT_CTRL_ALLOW_REPORT_MASK, (rtk_uint32)pRxCfg->rxReport);
        if(retVal != RT_ERR_OK)
            return retVal;
    retVal = rtl8373_setAsicRegBits(RTL8373_IGMP_PORT_CTRL_ADDR(rtk_switch_port_L2P_get(port)), RTL8373_IGMP_PORT_CTRL_ALLOW_LEAVE_MASK, (rtk_uint32)pRxCfg->rxLeave);
        if(retVal != RT_ERR_OK)
            return retVal;
    retVal = rtl8373_setAsicRegBits(RTL8373_IGMP_PORT_CTRL_ADDR(rtk_switch_port_L2P_get(port)), RTL8373_IGMP_PORT_CTRL_ALLOW_MRP_MASK, (rtk_uint32)pRxCfg->rxMRP);
        if(retVal != RT_ERR_OK)
            return retVal;
    retVal = rtl8373_setAsicRegBits(RTL8373_IGMP_PORT_CTRL_ADDR(rtk_switch_port_L2P_get(port)), RTL8373_IGMP_PORT_CTRL_ALLOW_MC_DATA_MASK, (rtk_uint32)pRxCfg->rxMcast);
        if(retVal != RT_ERR_OK)
            return retVal;
  
    return RT_ERR_OK;
}

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
rtk_api_ret_t dal_rtl8373_igmp_portRxPktEnable_get(rtk_port_t port, rtk_igmp_rxPktEnable_t *pRxCfg)
{
    rtk_api_ret_t   retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();
    /* Check port valid */
    RTK_CHK_PORT_VALID(port);

    if(NULL == pRxCfg)
        return RT_ERR_NULL_POINTER;

    retVal = rtl8373_getAsicRegBits(RTL8373_IGMP_PORT_CTRL_ADDR(rtk_switch_port_L2P_get(port)),RTL8373_IGMP_PORT_CTRL_ALLOW_QUERY_MASK, (rtk_uint32 *)&(pRxCfg->rxQuery));
        if(retVal != RT_ERR_OK)
            return retVal;
    retVal = rtl8373_getAsicRegBits(RTL8373_IGMP_PORT_CTRL_ADDR(rtk_switch_port_L2P_get(port)),RTL8373_IGMP_PORT_CTRL_ALLOW_REPORT_MASK, (rtk_uint32 *)&(pRxCfg->rxReport));
        if(retVal != RT_ERR_OK)
            return retVal;
    retVal = rtl8373_getAsicRegBits(RTL8373_IGMP_PORT_CTRL_ADDR(rtk_switch_port_L2P_get(port)),RTL8373_IGMP_PORT_CTRL_ALLOW_LEAVE_MASK, (rtk_uint32 *)&(pRxCfg->rxLeave));
        if(retVal != RT_ERR_OK)
            return retVal;
    retVal = rtl8373_getAsicRegBits(RTL8373_IGMP_PORT_CTRL_ADDR(rtk_switch_port_L2P_get(port)),RTL8373_IGMP_PORT_CTRL_ALLOW_MRP_MASK, (rtk_uint32 *)&(pRxCfg->rxMRP));
        if(retVal != RT_ERR_OK)
            return retVal;
    retVal = rtl8373_getAsicRegBits(RTL8373_IGMP_PORT_CTRL_ADDR(rtk_switch_port_L2P_get(port)),RTL8373_IGMP_PORT_CTRL_ALLOW_MC_DATA_MASK, (rtk_uint32 *)&(pRxCfg->rxMcast));
        if(retVal != RT_ERR_OK)
            return retVal;  

    return RT_ERR_OK;
}

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
rtk_api_ret_t  dal_rtl8373_getAsicIGMPGroup(rtk_uint32 idx, rtk_uint32 *valid, rtl8373_igmpgroup *grp)
{
    ret_t   retVal;
    rtk_uint32  regAddr, regData;
    rtk_uint32  groupInfo = 0;

    if(idx > RTL8373_IGMP_MAX_GOUP)
        return RT_ERR_OUT_OF_RANGE;


    /* Write ACS_ADR register for data bits */
    regAddr = RTL8373_ITA_CTRL0_ADDR;
    regData = (1|(TB_OP_READ << 1)|(TB_TARGET_IGMP_GROUP << 8)|(idx<<16));

    retVal = rtl8373_setAsicReg(regAddr, regData);
    if(retVal != RT_ERR_OK)
        return retVal;

    /* Write ACS_CMD register
    regAddr = RTL8373_TABLE_ACCESS_CTRL_REG;
    regData = RTL8373_TABLE_ACCESS_REG_DATA(TB_OP_READ, TB_TARGET_IGMP_GROUP);
    retVal = rtl8373_setAsicRegBits(regAddr, RTL8373_TABLE_TYPE_MASK | RTL8373_COMMAND_TYPE_MASK, regData);
    if(retVal != RT_ERR_OK)
        return retVal;*/

    retVal = rtl8373_getAsicReg(RTL8373_ITA_READ_DATA0_ADDR(0), &groupInfo);
    if(retVal != RT_ERR_OK)
        return retVal;

    grp->p0_timer = groupInfo & 0x00000007;
    grp->p1_timer = (groupInfo >> 3) & 0x00000007;
    grp->p2_timer = (groupInfo >> 6) & 0x00000007;
    grp->p3_timer = (groupInfo >> 9) & 0x00000007;
    grp->p4_timer = (groupInfo >> 12) & 0x00000007;
    grp->p5_timer = (groupInfo >> 15) & 0x00000007;
    grp->p6_timer = (groupInfo >> 18) & 0x00000007;
    grp->p7_timer = (groupInfo >> 21) & 0x00000007;
    grp->report_supp_flag = (groupInfo >> 24) & 0x00000001;
    grp->p8_timer = (groupInfo >> 25) & 0x00000007;
    grp->p9_timer = (groupInfo >> 28) & 0x00000007;
    grp->p10_timer = (groupInfo >> 31) & 0x00000001;


    retVal = rtl8373_getAsicReg(RTL8373_ITA_READ_DATA0_ADDR(1), &regData);
    if(retVal != RT_ERR_OK)
        return retVal;


    grp->p10_timer |= (regData & 0x00000003) << 1;

    /* Valid bit */
    retVal = rtl8373_getAsicReg(RTL8373_IGMP_TBL_USAGE_ADDR(idx), &regData);
    if(retVal != RT_ERR_OK)
        return retVal;

    *valid = ((regData & (0x0001 << (idx %32))) != 0) ? 1 : 0;

    return RT_ERR_OK;
}
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
rtk_api_ret_t dal_rtl8373_igmp_groupInfo_get(rtk_uint32 index, rtk_igmp_groupInfo_t *pGroup)
{
    rtk_api_ret_t   retVal;
    rtk_uint32      valid;
    rtl8373_igmpgroup  grp;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check index */
    if(index > RTL8373_IGMP_MAX_GOUP)
        return RT_ERR_INPUT;

    if(NULL == pGroup)
        return RT_ERR_NULL_POINTER;

    if ((retVal = dal_rtl8373_getAsicIGMPGroup(index, &valid, &grp))!=RT_ERR_OK)
        return retVal;

    memset(pGroup, 0x00, sizeof(rtk_igmp_groupInfo_t));
    pGroup->valid = valid;
    pGroup->reportSuppFlag = grp.report_supp_flag;

    if(grp.p0_timer != 0)
    {
        RTK_PORTMASK_PORT_SET((pGroup->member), rtk_switch_port_P2L_get(0));
        pGroup->timer[rtk_switch_port_P2L_get(0)] = grp.p0_timer;
    }

    if(grp.p1_timer != 0)
    {
        RTK_PORTMASK_PORT_SET((pGroup->member), rtk_switch_port_P2L_get(1));
        pGroup->timer[rtk_switch_port_P2L_get(1)] = grp.p1_timer;
    }

    if(grp.p2_timer != 0)
    {
        RTK_PORTMASK_PORT_SET((pGroup->member), rtk_switch_port_P2L_get(2));
        pGroup->timer[rtk_switch_port_P2L_get(2)] = grp.p2_timer;
    }

    if(grp.p3_timer != 0)
    {
        RTK_PORTMASK_PORT_SET((pGroup->member), rtk_switch_port_P2L_get(3));
        pGroup->timer[rtk_switch_port_P2L_get(3)] = grp.p3_timer;
    }

    if(grp.p4_timer != 0)
    {
        RTK_PORTMASK_PORT_SET((pGroup->member), rtk_switch_port_P2L_get(4));
        pGroup->timer[rtk_switch_port_P2L_get(4)] = grp.p4_timer;
    }

    if(grp.p5_timer != 0)
    {
        RTK_PORTMASK_PORT_SET((pGroup->member), rtk_switch_port_P2L_get(5));
        pGroup->timer[rtk_switch_port_P2L_get(5)] = grp.p5_timer;
    }

    if(grp.p6_timer != 0)
    {
        RTK_PORTMASK_PORT_SET((pGroup->member), rtk_switch_port_P2L_get(6));
        pGroup->timer[rtk_switch_port_P2L_get(6)] = grp.p6_timer;
    }

    if(grp.p7_timer != 0)
    {
        RTK_PORTMASK_PORT_SET((pGroup->member), rtk_switch_port_P2L_get(7));
        pGroup->timer[rtk_switch_port_P2L_get(7)] = grp.p7_timer;
    }

    if(grp.p8_timer != 0)
    {
        RTK_PORTMASK_PORT_SET((pGroup->member), rtk_switch_port_P2L_get(8));
        pGroup->timer[rtk_switch_port_P2L_get(8)] = grp.p8_timer;
    }

    if(grp.p9_timer != 0)
    {
        RTK_PORTMASK_PORT_SET((pGroup->member), rtk_switch_port_P2L_get(9));
        pGroup->timer[rtk_switch_port_P2L_get(9)] = grp.p9_timer;
    }

    if(grp.p10_timer != 0)
    {
        RTK_PORTMASK_PORT_SET((pGroup->member), rtk_switch_port_P2L_get(10));
        pGroup->timer[rtk_switch_port_P2L_get(10)] = grp.p10_timer;
    }

    return RT_ERR_OK;
}

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
rtk_api_ret_t dal_rtl8373_igmp_ReportLeaveFwdAction_set(rtk_igmp_ReportLeaveFwdAct_t action)
{
    rtk_api_ret_t   retVal;
    rtk_uint32      regData;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    switch(action)
    {
        case IGMP_REPORT_LEAVE_TO_ROUTER:
            regData = 1;
            break;
        case IGMP_REPORT_LEAVE_TO_ALLPORT:
            regData = 2;
            break;
        case IGMP_REPORT_LEAVE_TO_ROUTER_PORT_ADV:
            regData = 3;
            break;
        default:
            return RT_ERR_INPUT;
    }

    retVal = rtl8373_setAsicRegBits(RTL8373_IGMP_CTRL_ADDR, RTL8373_IGMP_CTRL_REPORT_LEAVE_FWD_MASK, regData);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

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
rtk_api_ret_t dal_rtl8373_igmp_ReportLeaveFwdAction_get(rtk_igmp_ReportLeaveFwdAct_t *pAction)
{
    rtk_api_ret_t   retVal;
    rtk_uint32      regData;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pAction)
        return RT_ERR_NULL_POINTER;

    retVal = rtl8373_getAsicRegBits(RTL8373_IGMP_CTRL_ADDR, RTL8373_IGMP_CTRL_REPORT_LEAVE_FWD_MASK, &regData);
    if(retVal != RT_ERR_OK)
        return retVal;

   
    *pAction=regData;
   
    return RT_ERR_OK;
}

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
rtk_api_ret_t dal_rtl8373_igmp_dropLeaveZeroEnable_set(rtk_enable_t enabled)
{
    rtk_api_ret_t   retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(enabled >= RTK_ENABLE_END)
        return RT_ERR_INPUT;

    retVal = rtl8373_setAsicRegBit(RTL8373_IGMP_CTRL_ADDR, RTL8373_IGMP_CTRL_DROP_LEAVE_ZERO_OFFSET, enabled);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;

}

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
rtk_api_ret_t dal_rtl8373_igmp_dropLeaveZeroEnable_get(rtk_enable_t *pEnabled)
{
    rtk_api_ret_t   retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(NULL == pEnabled)
        return RT_ERR_NULL_POINTER;

    retVal = rtl8373_getAsicRegBit(RTL8373_IGMP_CTRL_ADDR, RTL8373_IGMP_CTRL_DROP_LEAVE_ZERO_OFFSET, pEnabled);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;

}

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
rtk_api_ret_t dal_rtl8373_igmp_bypassGroupRange_set(rtk_igmp_bypassGroup_t group, rtk_enable_t enabled)
{
    rtk_api_ret_t   retVal;
    rtk_uint32 offset;
    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(group >= IGMP_BYPASS_GROUP_END)
        return RT_ERR_INPUT;

    if(enabled >= RTK_ENABLE_END)
        return RT_ERR_INPUT;

    switch(group)
    {
        case BYPASS_224_0_0_X:
            offset = RTL8373_IGMP_CTRL_IGMP_MLD_IP4_BYPASS_224_0_0_OFFSET;
            break;
        case BYPASS_224_0_1_X:
            offset = RTL8373_IGMP_CTRL_IGMP_MLD_IP4_BYPASS_224_0_1_OFFSET;
            break;
        case BYPASS_239_255_255_X:
            offset = RTL8373_IGMP_CTRL_IGMP_MLD_IP4_BYPASS_239_255_255_OFFSET;
            break;
        case BYPASS_IPV6_00XX:
            offset = RTL8373_IGMP_CTRL_IGMP_MLD_IP6_BYPASS_OFFSET;
            break;
        default:
            return RT_ERR_INPUT;
    }

    retVal = rtl8373_setAsicRegBit(RTL8373_IGMP_CTRL_ADDR, offset, enabled);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

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
rtk_api_ret_t dal_rtl8373_igmp_bypassGroupRange_get(rtk_igmp_bypassGroup_t group, rtk_enable_t *pEnable)
{
    rtk_api_ret_t   retVal;
    rtk_uint32 offset;
    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if(group >= IGMP_BYPASS_GROUP_END)
        return RT_ERR_INPUT;

    if(NULL == pEnable)
        return RT_ERR_NULL_POINTER;
 
    switch(group)
    {
        case BYPASS_224_0_0_X:
            offset = RTL8373_IGMP_CTRL_IGMP_MLD_IP4_BYPASS_224_0_0_OFFSET;
            break;
        case BYPASS_224_0_1_X:
            offset = RTL8373_IGMP_CTRL_IGMP_MLD_IP4_BYPASS_224_0_1_OFFSET;
            break;
        case BYPASS_239_255_255_X:
            offset = RTL8373_IGMP_CTRL_IGMP_MLD_IP4_BYPASS_239_255_255_OFFSET;
            break;
        case BYPASS_IPV6_00XX:
            offset = RTL8373_IGMP_CTRL_IGMP_MLD_IP6_BYPASS_OFFSET;
            break;
        default:
            return RT_ERR_INPUT;
    }

    retVal = rtl8373_getAsicRegBit(RTL8373_IGMP_CTRL_ADDR, offset, pEnable);
    if(retVal != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}


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
 * Purpose : RTL8367C switch high-level API for RTL8367C
 * Feature : QoS related functions
 *
 */

#include <rtl8367c_asicdrv_qos.h>
/* Function Name:
 *      rtl8367c_setAsicPriorityDot1qRemapping
 * Description:
 *      Set 802.1Q absolutely priority
 * Input:
 *      srcpriority - Priority value
 *      priority     - Absolute priority value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                 - Success
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_QOS_INT_PRIORITY    - Invalid priority
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicPriorityDot1qRemapping(rtk_uint32 srcpriority, rtk_uint32 priority )
{
    if((srcpriority > RTL8367C_PRIMAX) || (priority > RTL8367C_PRIMAX))
        return RT_ERR_QOS_INT_PRIORITY;

    return rtl8367c_setAsicRegBits(RTL8367C_QOS_1Q_PRIORITY_REMAPPING_REG(srcpriority), RTL8367C_QOS_1Q_PRIORITY_REMAPPING_MASK(srcpriority),priority);
}
/* Function Name:
 *      rtl8367c_getAsicPriorityDot1qRemapping
 * Description:
 *      Get 802.1Q absolutely priority
 * Input:
 *      srcpriority - Priority value
 *      pPriority     - Absolute priority value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicPriorityDot1qRemapping(rtk_uint32 srcpriority, rtk_uint32 *pPriority )
{
    if(srcpriority > RTL8367C_PRIMAX )
        return RT_ERR_QOS_INT_PRIORITY;

    return rtl8367c_getAsicRegBits(RTL8367C_QOS_1Q_PRIORITY_REMAPPING_REG(srcpriority), RTL8367C_QOS_1Q_PRIORITY_REMAPPING_MASK(srcpriority), pPriority);
}
/* Function Name:
 *      rtl8367c_setAsicPriorityPortBased
 * Description:
 *      Set port based priority
 * Input:
 *      port         - Physical port number (0~7)
 *      priority     - Priority value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                 - Success
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_PORT_ID          - Invalid port number
 *      RT_ERR_QOS_INT_PRIORITY    - Invalid priority
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicPriorityPortBased(rtk_uint32 port, rtk_uint32 priority )
{
    ret_t retVal;

    if(port > RTL8367C_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(priority > RTL8367C_PRIMAX )
        return RT_ERR_QOS_INT_PRIORITY;

    if(port < 8)
    {
        retVal = rtl8367c_setAsicRegBits(RTL8367C_QOS_PORTBASED_PRIORITY_REG(port), RTL8367C_QOS_PORTBASED_PRIORITY_MASK(port), priority);
        if(retVal != RT_ERR_OK)
            return retVal;
    }
    else
    {
        retVal = rtl8367c_setAsicRegBits(RTL8367C_REG_QOS_PORTBASED_PRIORITY_CTRL2, 0x7 << ((port - 8) << 2), priority);
        if(retVal != RT_ERR_OK)
            return retVal;
    }

    return RT_ERR_OK;
}
/* Function Name:
 *      rtl8367c_getAsicPriorityPortBased
 * Description:
 *      Get port based priority
 * Input:
 *      port         - Physical port number (0~7)
 *      pPriority     - Priority value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK         - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicPriorityPortBased(rtk_uint32 port, rtk_uint32 *pPriority )
{
    ret_t retVal;

    if(port > RTL8367C_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(port < 8)
    {
        retVal = rtl8367c_getAsicRegBits(RTL8367C_QOS_PORTBASED_PRIORITY_REG(port), RTL8367C_QOS_PORTBASED_PRIORITY_MASK(port), pPriority);
        if(retVal != RT_ERR_OK)
            return retVal;
    }
    else
    {
        retVal = rtl8367c_getAsicRegBits(RTL8367C_REG_QOS_PORTBASED_PRIORITY_CTRL2, 0x7 << ((port - 8) << 2), pPriority);
        if(retVal != RT_ERR_OK)
            return retVal;
    }

    return RT_ERR_OK;
}
/* Function Name:
 *      rtl8367c_setAsicPriorityDscpBased
 * Description:
 *      Set DSCP-based priority
 * Input:
 *      dscp         - DSCP value
 *      priority     - Priority value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                 - Success
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_QOS_DSCP_VALUE    - Invalid DSCP value
 *      RT_ERR_QOS_INT_PRIORITY    - Invalid priority
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicPriorityDscpBased(rtk_uint32 dscp, rtk_uint32 priority )
{
    if(priority > RTL8367C_PRIMAX )
        return RT_ERR_QOS_INT_PRIORITY;

    if(dscp > RTL8367C_DSCPMAX)
        return RT_ERR_QOS_DSCP_VALUE;

    return rtl8367c_setAsicRegBits(RTL8367C_QOS_DSCP_TO_PRIORITY_REG(dscp), RTL8367C_QOS_DSCP_TO_PRIORITY_MASK(dscp), priority);
}
/* Function Name:
 *      rtl8367c_getAsicPriorityDscpBased
 * Description:
 *      Get DSCP-based priority
 * Input:
 *      dscp         - DSCP value
 *      pPriority     - Priority value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                 - Success
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_QOS_INT_PRIORITY    - Invalid priority
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicPriorityDscpBased(rtk_uint32 dscp, rtk_uint32 *pPriority )
{
    if(dscp > RTL8367C_DSCPMAX)
        return RT_ERR_QOS_DSCP_VALUE;

    return rtl8367c_getAsicRegBits(RTL8367C_QOS_DSCP_TO_PRIORITY_REG(dscp), RTL8367C_QOS_DSCP_TO_PRIORITY_MASK(dscp), pPriority);
}
/* Function Name:
 *      rtl8367c_setAsicPriorityDecision
 * Description:
 *      Set priority decision table
 * Input:
 *      prisrc         - Priority decision source
 *      decisionPri - Decision priority assignment
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                     - Success
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_QOS_INT_PRIORITY        - Invalid priority
 *      RT_ERR_QOS_SEL_PRI_SOURCE    - Invalid priority decision source parameter
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicPriorityDecision(rtk_uint32 index, rtk_uint32 prisrc, rtk_uint32 decisionPri)
{
    ret_t retVal;

    if(index >= PRIDEC_IDX_END )
        return RT_ERR_ENTRY_INDEX;

    if(prisrc >= PRIDEC_END )
        return RT_ERR_QOS_SEL_PRI_SOURCE;

    if(decisionPri > RTL8367C_DECISIONPRIMAX )
        return RT_ERR_QOS_INT_PRIORITY;

    switch(index)
    {
        case PRIDEC_IDX0:
            if((retVal = rtl8367c_setAsicRegBits(RTL8367C_QOS_INTERNAL_PRIORITY_DECISION_REG(prisrc), RTL8367C_QOS_INTERNAL_PRIORITY_DECISION_MASK(prisrc), decisionPri))!=  RT_ERR_OK)
                return retVal;
            break;
        case PRIDEC_IDX1:
            if((retVal = rtl8367c_setAsicRegBits(RTL8367C_QOS_INTERNAL_PRIORITY_DECISION2_REG(prisrc), RTL8367C_QOS_INTERNAL_PRIORITY_DECISION2_MASK(prisrc), decisionPri))!=  RT_ERR_OK)
                return retVal;
            break;
        default:
            break;
    };

    return RT_ERR_OK;


}

/* Function Name:
 *      rtl8367c_getAsicPriorityDecision
 * Description:
 *      Get priority decision table
 * Input:
 *      prisrc         - Priority decision source
 *      pDecisionPri - Decision priority assignment
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                     - Success
 *      RT_ERR_SMI                  - SMI access error
 *      RT_ERR_QOS_SEL_PRI_SOURCE    - Invalid priority decision source parameter
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicPriorityDecision(rtk_uint32 index, rtk_uint32 prisrc, rtk_uint32* pDecisionPri)
{
    ret_t retVal;

    if(index >= PRIDEC_IDX_END )
        return RT_ERR_ENTRY_INDEX;

    if(prisrc >= PRIDEC_END )
        return RT_ERR_QOS_SEL_PRI_SOURCE;

    switch(index)
    {
        case PRIDEC_IDX0:
            if((retVal = rtl8367c_getAsicRegBits(RTL8367C_QOS_INTERNAL_PRIORITY_DECISION_REG(prisrc), RTL8367C_QOS_INTERNAL_PRIORITY_DECISION_MASK(prisrc), pDecisionPri))!=  RT_ERR_OK)
                return retVal;
            break;
        case PRIDEC_IDX1:
            if((retVal = rtl8367c_getAsicRegBits(RTL8367C_QOS_INTERNAL_PRIORITY_DECISION2_REG(prisrc), RTL8367C_QOS_INTERNAL_PRIORITY_DECISION2_MASK(prisrc), pDecisionPri))!=  RT_ERR_OK)
                return retVal;
            break;
        default:
            break;
    };

    return RT_ERR_OK;

}

/* Function Name:
 *      rtl8367c_setAsicPortPriorityDecisionIndex
 * Description:
 *      Set priority decision index for each port
 * Input:
 *      port     - Physical port number (0~7)
 *      index     - Table index
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK             - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number
 *      RT_ERR_QUEUE_NUM      - Invalid queue number
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicPortPriorityDecisionIndex(rtk_uint32 port, rtk_uint32 index )
{
    if(port > RTL8367C_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(index >= PRIDEC_IDX_END)
        return RT_ERR_ENTRY_INDEX;

    return rtl8367c_setAsicRegBit(RTL8367C_QOS_INTERNAL_PRIORITY_DECISION_IDX_CTRL, port, index);
}
/* Function Name:
 *      rtl8367c_getAsicPortPriorityDecisionIndex
 * Description:
 *      Get priority decision index  for each port
 * Input:
 *      port     - Physical port number (0~7)
 *      pIndex     - Table index
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK             - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicPortPriorityDecisionIndex(rtk_uint32 port, rtk_uint32 *pIndex )
{
    if(port > RTL8367C_PORTIDMAX)
        return RT_ERR_PORT_ID;

    return rtl8367c_getAsicRegBit(RTL8367C_QOS_INTERNAL_PRIORITY_DECISION_IDX_CTRL, port, pIndex);
}

/* Function Name:
 *      rtl8367c_setAsicOutputQueueMappingIndex
 * Description:
 *      Set output queue number for each port
 * Input:
 *      port     - Physical port number (0~7)
 *      index     - Mapping table index
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK             - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number
 *      RT_ERR_QUEUE_NUM      - Invalid queue number
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicOutputQueueMappingIndex(rtk_uint32 port, rtk_uint32 index )
{
    if(port > RTL8367C_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(index >= RTL8367C_QUEUENO)
        return RT_ERR_QUEUE_NUM;

    return rtl8367c_setAsicRegBits(RTL8367C_QOS_PORT_QUEUE_NUMBER_REG(port), RTL8367C_QOS_PORT_QUEUE_NUMBER_MASK(port), index);
}
/* Function Name:
 *      rtl8367c_getAsicOutputQueueMappingIndex
 * Description:
 *      Get output queue number for each port
 * Input:
 *      port     - Physical port number (0~7)
 *      pIndex     - Mapping table index
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK             - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicOutputQueueMappingIndex(rtk_uint32 port, rtk_uint32 *pIndex )
{
    if(port > RTL8367C_PORTIDMAX)
        return RT_ERR_PORT_ID;

    return rtl8367c_getAsicRegBits(RTL8367C_QOS_PORT_QUEUE_NUMBER_REG(port), RTL8367C_QOS_PORT_QUEUE_NUMBER_MASK(port), pIndex);
}
/* Function Name:
 *      rtl8367c_setAsicPriorityToQIDMappingTable
 * Description:
 *      Set priority to QID mapping table parameters
 * Input:
 *      index         - Mapping table index
 *      priority     - The priority value
 *      qid         - Queue id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                 - Success
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_QUEUE_ID          - Invalid queue id
 *      RT_ERR_QUEUE_NUM          - Invalid queue number
 *      RT_ERR_QOS_INT_PRIORITY - Invalid priority
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicPriorityToQIDMappingTable(rtk_uint32 index, rtk_uint32 priority, rtk_uint32 qid )
{
    if(index >= RTL8367C_QUEUENO)
        return RT_ERR_QUEUE_NUM;

    if(priority > RTL8367C_PRIMAX)
        return RT_ERR_QOS_INT_PRIORITY;

    if(qid > RTL8367C_QIDMAX)
        return RT_ERR_QUEUE_ID;

    return rtl8367c_setAsicRegBits(RTL8367C_QOS_1Q_PRIORITY_TO_QID_REG(index, priority), RTL8367C_QOS_1Q_PRIORITY_TO_QID_MASK(priority), qid);
}
/* Function Name:
 *      rtl8367c_getAsicPriorityToQIDMappingTable
 * Description:
 *      Get priority to QID mapping table parameters
 * Input:
 *      index         - Mapping table index
 *      priority     - The priority value
 *      pQid         - Queue id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                 - Success
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_QUEUE_NUM          - Invalid queue number
 *      RT_ERR_QOS_INT_PRIORITY - Invalid priority
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicPriorityToQIDMappingTable(rtk_uint32 index, rtk_uint32 priority, rtk_uint32* pQid)
{
    if(index >= RTL8367C_QUEUENO)
        return RT_ERR_QUEUE_NUM;

    if(priority > RTL8367C_PRIMAX)
        return RT_ERR_QOS_INT_PRIORITY;

    return rtl8367c_getAsicRegBits(RTL8367C_QOS_1Q_PRIORITY_TO_QID_REG(index, priority), RTL8367C_QOS_1Q_PRIORITY_TO_QID_MASK(priority), pQid);
}
/* Function Name:
 *      rtl8367c_setAsicRemarkingDot1pAbility
 * Description:
 *      Set 802.1p remarking ability
 * Input:
 *      port     - Physical port number (0~7)
 *      enabled - 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK             - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicRemarkingDot1pAbility(rtk_uint32 port, rtk_uint32 enabled)
{
    return rtl8367c_setAsicRegBit(RTL8367C_PORT_MISC_CFG_REG(port), RTL8367C_1QREMARK_ENABLE_OFFSET, enabled);
}
/* Function Name:
 *      rtl8367c_getAsicRemarkingDot1pAbility
 * Description:
 *      Get 802.1p remarking ability
 * Input:
 *      port     - Physical port number (0~7)
 *      pEnabled - 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicRemarkingDot1pAbility(rtk_uint32 port, rtk_uint32* pEnabled)
{
    return rtl8367c_getAsicRegBit(RTL8367C_PORT_MISC_CFG_REG(port), RTL8367C_1QREMARK_ENABLE_OFFSET, pEnabled);
}
/* Function Name:
 *      rtl8367c_setAsicRemarkingDot1pParameter
 * Description:
 *      Set 802.1p remarking parameter
 * Input:
 *      priority     - Priority value
 *      newPriority - New priority value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                 - Success
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_QOS_INT_PRIORITY - Invalid priority
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicRemarkingDot1pParameter(rtk_uint32 priority, rtk_uint32 newPriority )
{
    if(priority > RTL8367C_PRIMAX || newPriority > RTL8367C_PRIMAX)
        return RT_ERR_QOS_INT_PRIORITY;

    return rtl8367c_setAsicRegBits(RTL8367C_QOS_1Q_REMARK_REG(priority), RTL8367C_QOS_1Q_REMARK_MASK(priority), newPriority);
}
/* Function Name:
 *      rtl8367c_getAsicRemarkingDot1pParameter
 * Description:
 *      Get 802.1p remarking parameter
 * Input:
 *      priority     - Priority value
 *      pNewPriority - New priority value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                 - Success
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_QOS_INT_PRIORITY - Invalid priority
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicRemarkingDot1pParameter(rtk_uint32 priority, rtk_uint32 *pNewPriority )
{
    if(priority > RTL8367C_PRIMAX )
        return RT_ERR_QOS_INT_PRIORITY;

    return rtl8367c_getAsicRegBits(RTL8367C_QOS_1Q_REMARK_REG(priority), RTL8367C_QOS_1Q_REMARK_MASK(priority), pNewPriority);
}

/* Function Name:
 *      rtl8367c_setAsicRemarkingDot1pSrc
 * Description:
 *      Set remarking source of 802.1p remarking.
 * Input:
 *      type      - remarking source
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter

 * Note:
 *      The API can configure 802.1p remark functionality to map original DSCP value or internal
 *      priority to TX DSCP value.
 */
ret_t rtl8367c_setAsicRemarkingDot1pSrc(rtk_uint32 type)
{

    if(type >= DOT1P_PRISEL_END )
        return RT_ERR_QOS_SEL_PRI_SOURCE;

    return rtl8367c_setAsicRegBit(RTL8367C_REG_RMK_CFG_SEL_CTRL, RTL8367C_RMK_1Q_CFG_SEL_OFFSET, type);
}


/* Function Name:
 *      rtl8367c_getAsicRemarkingDot1pSrc
 * Description:
 *      Get remarking source of 802.1p remarking.
 * Output:
 *      pType      - remarking source
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer

 * Note:
 *      None
 */
ret_t rtl8367c_getAsicRemarkingDot1pSrc(rtk_uint32 *pType)
{
    return rtl8367c_getAsicRegBit(RTL8367C_REG_RMK_CFG_SEL_CTRL, RTL8367C_RMK_1Q_CFG_SEL_OFFSET, pType);
}





/* Function Name:
 *      rtl8367c_setAsicRemarkingDscpAbility
 * Description:
 *      Set DSCP remarking ability
 * Input:
 *      enabled     - 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicRemarkingDscpAbility(rtk_uint32 enabled)
{
    return rtl8367c_setAsicRegBit(RTL8367C_REMARKING_CTRL_REG, RTL8367C_REMARKING_DSCP_ENABLE_OFFSET, enabled);
}
/* Function Name:
 *      rtl8367c_getAsicRemarkingDscpAbility
 * Description:
 *      Get DSCP remarking ability
 * Input:
 *      enabled     - 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicRemarkingDscpAbility(rtk_uint32* pEnabled)
{
    return rtl8367c_getAsicRegBit(RTL8367C_REMARKING_CTRL_REG, RTL8367C_REMARKING_DSCP_ENABLE_OFFSET, pEnabled);
}
/* Function Name:
 *      rtl8367c_setAsicRemarkingDscpParameter
 * Description:
 *      Set DSCP remarking parameter
 * Input:
 *      priority     - Priority value
 *      newDscp     - New DSCP value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                 - Success
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_QOS_DSCP_VALUE    - Invalid DSCP value
 *      RT_ERR_QOS_INT_PRIORITY    - Invalid priority
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicRemarkingDscpParameter(rtk_uint32 priority, rtk_uint32 newDscp )
{
    if(priority > RTL8367C_PRIMAX )
        return RT_ERR_QOS_INT_PRIORITY;

    if(newDscp > RTL8367C_DSCPMAX)
        return RT_ERR_QOS_DSCP_VALUE;

    return rtl8367c_setAsicRegBits(RTL8367C_QOS_DSCP_REMARK_REG(priority), RTL8367C_QOS_DSCP_REMARK_MASK(priority), newDscp);
}
/* Function Name:
 *      rtl8367c_getAsicRemarkingDscpParameter
 * Description:
 *      Get DSCP remarking parameter
 * Input:
 *      priority     - Priority value
 *      pNewDscp     - New DSCP value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                 - Success
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_QOS_INT_PRIORITY    - Invalid priority
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicRemarkingDscpParameter(rtk_uint32 priority, rtk_uint32* pNewDscp )
{
    if(priority > RTL8367C_PRIMAX )
        return RT_ERR_QOS_INT_PRIORITY;

    return rtl8367c_getAsicRegBits(RTL8367C_QOS_DSCP_REMARK_REG(priority), RTL8367C_QOS_DSCP_REMARK_MASK(priority), pNewDscp);
}

/* Function Name:
 *      rtl8367c_setAsicRemarkingDscpSrc
 * Description:
 *      Set remarking source of DSCP remarking.
 * Input:
 *      type      - remarking source
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter

 * Note:
 *      The API can configure DSCP remark functionality to map original DSCP value or internal
 *      priority to TX DSCP value.
 */
ret_t rtl8367c_setAsicRemarkingDscpSrc(rtk_uint32 type)
{

    if(type >= DSCP_PRISEL_END )
        return RT_ERR_QOS_SEL_PRI_SOURCE;

    return rtl8367c_setAsicRegBits(RTL8367C_REG_RMK_CFG_SEL_CTRL, RTL8367C_RMK_DSCP_CFG_SEL_MASK, type);
}


/* Function Name:
 *      rtl8367c_getAsicRemarkingDscpSrc
 * Description:
 *      Get remarking source of DSCP remarking.
 * Output:
 *      pType      - remarking source
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer

 * Note:
 *      None
 */
ret_t rtl8367c_getAsicRemarkingDscpSrc(rtk_uint32 *pType)
{
    return rtl8367c_getAsicRegBits(RTL8367C_REG_RMK_CFG_SEL_CTRL, RTL8367C_RMK_DSCP_CFG_SEL_MASK, pType);
}

/* Function Name:
 *      rtl8367c_setAsicRemarkingDscp2Dscp
 * Description:
 *      Set DSCP to remarked DSCP mapping.
 * Input:
 *      dscp    - DSCP value
 *      rmkDscp - remarked DSCP value
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID          - Invalid unit id
 *      RT_ERR_QOS_DSCP_VALUE   - Invalid dscp value
 * Note:
 *      dscp parameter can be DSCP value or internal priority according to configuration of API
 *      dal_apollomp_qos_dscpRemarkSrcSel_set(), because DSCP remark functionality can map original DSCP
 *      value or internal priority to TX DSCP value.
 */
ret_t rtl8367c_setAsicRemarkingDscp2Dscp(rtk_uint32 dscp, rtk_uint32 rmkDscp)
{
    if((dscp > RTL8367C_DSCPMAX ) || (rmkDscp > RTL8367C_DSCPMAX))
        return RT_ERR_QOS_INT_PRIORITY;

    return rtl8367c_setAsicRegBits(RTL8367C_QOS_DSCP_TO_DSCP_REG(dscp), RTL8367C_QOS_DSCP_TO_DSCP_MASK(dscp), rmkDscp);
}

/* Function Name:
 *      rtl8367c_getAsicRemarkingDscp2Dscp
 * Description:
 *      Get DSCP to remarked DSCP mapping.
 * Input:
 *      dscp    - DSCP value
 * Output:
 *      pRmkDscp   - remarked DSCP value
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_QOS_DSCP_VALUE   - Invalid dscp value
 *      RT_ERR_NULL_POINTER     - NULL pointer
 * Note:
 *      None.
 */
ret_t rtl8367c_getAsicRemarkingDscp2Dscp(rtk_uint32 dscp, rtk_uint32 *pRmkDscp)
{
    if(dscp > RTL8367C_DSCPMAX)
        return RT_ERR_QOS_DSCP_VALUE;

    return rtl8367c_getAsicRegBits(RTL8367C_QOS_DSCP_TO_DSCP_REG(dscp), RTL8367C_QOS_DSCP_TO_DSCP_MASK(dscp), pRmkDscp);

}


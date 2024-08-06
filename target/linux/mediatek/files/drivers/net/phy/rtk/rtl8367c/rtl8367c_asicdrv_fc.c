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
 * Feature : Flow control related functions
 *
 */

#include <rtl8367c_asicdrv_fc.h>
/* Function Name:
 *      rtl8367c_setAsicFlowControlSelect
 * Description:
 *      Set system flow control type
 * Input:
 *      select      - System flow control type 1: Ingress flow control 0:Egress flow control
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicFlowControlSelect(rtk_uint32 select)
{
    return rtl8367c_setAsicRegBit(RTL8367C_REG_FLOWCTRL_CTRL0, RTL8367C_FLOWCTRL_TYPE_OFFSET, select);
}
/* Function Name:
 *      rtl8367c_getAsicFlowControlSelect
 * Description:
 *      Get system flow control type
 * Input:
 *      pSelect         - System flow control type 1: Ingress flow control 0:Egress flow control
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicFlowControlSelect(rtk_uint32 *pSelect)
{
    return rtl8367c_getAsicRegBit(RTL8367C_REG_FLOWCTRL_CTRL0, RTL8367C_FLOWCTRL_TYPE_OFFSET, pSelect);
}
/* Function Name:
 *      rtl8367c_setAsicFlowControlJumboMode
 * Description:
 *      Set Jumbo threshold for flow control
 * Input:
 *      enabled         - Jumbo mode flow control 1: Enable 0:Disable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicFlowControlJumboMode(rtk_uint32 enabled)
{
    return rtl8367c_setAsicRegBit(RTL8367C_REG_FLOWCTRL_JUMBO_SIZE, RTL8367C_JUMBO_MODE_OFFSET, enabled);
}
/* Function Name:
 *      rtl8367c_getAsicFlowControlJumboMode
 * Description:
 *      Get Jumbo threshold for flow control
 * Input:
 *      pEnabled        - Jumbo mode flow control 1: Enable 0:Disable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicFlowControlJumboMode(rtk_uint32* pEnabled)
{
    return rtl8367c_getAsicRegBit(RTL8367C_REG_FLOWCTRL_JUMBO_SIZE, RTL8367C_JUMBO_MODE_OFFSET, pEnabled);
}
/* Function Name:
 *      rtl8367c_setAsicFlowControlJumboModeSize
 * Description:
 *      Set Jumbo size for Jumbo mode flow control
 * Input:
 *      size        - Jumbo size 0:3Kbytes 1:4Kbytes 2:6Kbytes 3:9Kbytes
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicFlowControlJumboModeSize(rtk_uint32 size)
{
    if(size >= FC_JUMBO_SIZE_END)
        return RT_ERR_OUT_OF_RANGE;

    return rtl8367c_setAsicRegBits(RTL8367C_REG_FLOWCTRL_JUMBO_SIZE, RTL8367C_JUMBO_SIZE_MASK, size);
}
/* Function Name:
 *      rtl8367c_getAsicFlowControlJumboModeSize
 * Description:
 *      Get Jumbo size for Jumbo mode flow control
 * Input:
 *      pSize       - Jumbo size 0:3Kbytes 1:4Kbytes 2:6Kbytes 3:9Kbytes
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicFlowControlJumboModeSize(rtk_uint32* pSize)
{
    return rtl8367c_getAsicRegBits(RTL8367C_REG_FLOWCTRL_JUMBO_SIZE, RTL8367C_JUMBO_SIZE_MASK, pSize);
}

/* Function Name:
 *      rtl8367c_setAsicFlowControlQueueEgressEnable
 * Description:
 *      Set flow control ability for each queue
 * Input:
 *      port    - Physical port number (0~7)
 *      qid     - Queue id
 *      enabled - 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 *      RT_ERR_QUEUE_ID - Invalid queue id
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicFlowControlQueueEgressEnable(rtk_uint32 port, rtk_uint32 qid, rtk_uint32 enabled)
{
    if(port > RTL8367C_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(qid > RTL8367C_QIDMAX)
        return RT_ERR_QUEUE_ID;

    return rtl8367c_setAsicRegBit(RTL8367C_FLOWCRTL_EGRESS_QUEUE_ENABLE_REG(port), RTL8367C_FLOWCRTL_EGRESS_QUEUE_ENABLE_REG_OFFSET(port)+ qid, enabled);
}
/* Function Name:
 *      rtl8367c_getAsicFlowControlQueueEgressEnable
 * Description:
 *      Get flow control ability for each queue
 * Input:
 *      port    - Physical port number (0~7)
 *      qid     - Queue id
 *      pEnabled - 1: enabled, 0: disabled
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 *      RT_ERR_QUEUE_ID - Invalid queue id
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicFlowControlQueueEgressEnable(rtk_uint32 port, rtk_uint32 qid, rtk_uint32* pEnabled)
{
    if(port > RTL8367C_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(qid > RTL8367C_QIDMAX)
        return RT_ERR_QUEUE_ID;

    return  rtl8367c_getAsicRegBit(RTL8367C_FLOWCRTL_EGRESS_QUEUE_ENABLE_REG(port), RTL8367C_FLOWCRTL_EGRESS_QUEUE_ENABLE_REG_OFFSET(port)+ qid, pEnabled);
}
/* Function Name:
 *      rtl8367c_setAsicFlowControlDropAll
 * Description:
 *      Set system-based drop parameters
 * Input:
 *      dropall     - Whole system drop threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicFlowControlDropAll(rtk_uint32 dropall)
{
    if(dropall >= RTL8367C_PAGE_NUMBER)
        return RT_ERR_OUT_OF_RANGE;

    return rtl8367c_setAsicRegBits(RTL8367C_REG_FLOWCTRL_CTRL0, RTL8367C_DROP_ALL_THRESHOLD_MASK, dropall);
}
/* Function Name:
 *      rtl8367c_getAsicFlowControlDropAll
 * Description:
 *      Get system-based drop parameters
 * Input:
 *      pDropall    - Whole system drop threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicFlowControlDropAll(rtk_uint32* pDropall)
{
    return rtl8367c_getAsicRegBits(RTL8367C_REG_FLOWCTRL_CTRL0, RTL8367C_DROP_ALL_THRESHOLD_MASK, pDropall);
}
/* Function Name:
 *      rtl8367c_setAsicFlowControlPauseAll
 * Description:
 *      Set system-based all ports enable flow control parameters
 * Input:
 *      threshold   - Whole system pause all threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicFlowControlPauseAllThreshold(rtk_uint32 threshold)
{
    if(threshold >= RTL8367C_PAGE_NUMBER)
        return RT_ERR_OUT_OF_RANGE;

    return rtl8367c_setAsicRegBits(RTL8367C_REG_FLOWCTRL_ALL_ON, RTL8367C_FLOWCTRL_ALL_ON_THRESHOLD_MASK, threshold);
}
/* Function Name:
 *      rtl8367c_getAsicFlowControlPauseAllThreshold
 * Description:
 *      Get system-based all ports enable flow control parameters
 * Input:
 *      pThreshold  - Whole system pause all threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicFlowControlPauseAllThreshold(rtk_uint32 *pThreshold)
{
    return rtl8367c_getAsicRegBits(RTL8367C_REG_FLOWCTRL_ALL_ON, RTL8367C_FLOWCTRL_ALL_ON_THRESHOLD_MASK, pThreshold);
}
/* Function Name:
 *      rtl8367c_setAsicFlowControlSystemThreshold
 * Description:
 *      Set system-based flow control parameters
 * Input:
 *      onThreshold     - Flow control turn ON threshold
 *      offThreshold    - Flow control turn OFF threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicFlowControlSystemThreshold(rtk_uint32 onThreshold, rtk_uint32 offThreshold)
{
    ret_t retVal;

    if((onThreshold >= RTL8367C_PAGE_NUMBER) || (offThreshold >= RTL8367C_PAGE_NUMBER))
        return RT_ERR_OUT_OF_RANGE;

    retVal = rtl8367c_setAsicRegBits(RTL8367C_REG_FLOWCTRL_SYS_OFF, RTL8367C_FLOWCTRL_SYS_OFF_MASK, offThreshold);

    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8367c_setAsicRegBits(RTL8367C_REG_FLOWCTRL_SYS_ON, RTL8367C_FLOWCTRL_SYS_ON_MASK, onThreshold);

    return retVal;
}
/* Function Name:
 *      rtl8367c_getAsicFlowControlSystemThreshold
 * Description:
 *      Get system-based flow control parameters
 * Input:
 *      pOnThreshold    - Flow control turn ON threshold
 *      pOffThreshold   - Flow control turn OFF threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicFlowControlSystemThreshold(rtk_uint32 *pOnThreshold, rtk_uint32 *pOffThreshold)
{
    ret_t retVal;

    retVal = rtl8367c_getAsicRegBits(RTL8367C_REG_FLOWCTRL_SYS_OFF, RTL8367C_FLOWCTRL_SYS_OFF_MASK, pOffThreshold);

    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8367c_getAsicRegBits(RTL8367C_REG_FLOWCTRL_SYS_ON, RTL8367C_FLOWCTRL_SYS_ON_MASK, pOnThreshold);

    return retVal;
}
/* Function Name:
 *      rtl8367c_setAsicFlowControlSharedThreshold
 * Description:
 *      Set share-based flow control parameters
 * Input:
 *      onThreshold     - Flow control turn ON threshold
 *      offThreshold    - Flow control turn OFF threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicFlowControlSharedThreshold(rtk_uint32 onThreshold, rtk_uint32 offThreshold)
{
    ret_t retVal;

    if((onThreshold >= RTL8367C_PAGE_NUMBER) || (offThreshold >= RTL8367C_PAGE_NUMBER))
        return RT_ERR_OUT_OF_RANGE;

    retVal = rtl8367c_setAsicRegBits(RTL8367C_REG_FLOWCTRL_SHARE_OFF, RTL8367C_FLOWCTRL_SHARE_OFF_MASK, offThreshold);

    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8367c_setAsicRegBits(RTL8367C_REG_FLOWCTRL_SHARE_ON, RTL8367C_FLOWCTRL_SHARE_ON_MASK, onThreshold);

    return retVal;
}
/* Function Name:
 *      rtl8367c_getAsicFlowControlSharedThreshold
 * Description:
 *      Get share-based flow control parameters
 * Input:
 *      pOnThreshold    - Flow control turn ON threshold
 *      pOffThreshold   - Flow control turn OFF threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicFlowControlSharedThreshold(rtk_uint32 *pOnThreshold, rtk_uint32 *pOffThreshold)
{
    ret_t retVal;

    retVal = rtl8367c_getAsicRegBits(RTL8367C_REG_FLOWCTRL_SHARE_OFF, RTL8367C_FLOWCTRL_SHARE_OFF_MASK, pOffThreshold);

    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8367c_getAsicRegBits(RTL8367C_REG_FLOWCTRL_SHARE_ON, RTL8367C_FLOWCTRL_SHARE_ON_MASK, pOnThreshold);

    return retVal;
}
/* Function Name:
 *      rtl8367c_setAsicFlowControlPortThreshold
 * Description:
 *      Set Port-based flow control parameters
 * Input:
 *      onThreshold     - Flow control turn ON threshold
 *      offThreshold    - Flow control turn OFF threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicFlowControlPortThreshold(rtk_uint32 onThreshold, rtk_uint32 offThreshold)
{
    ret_t retVal;

    if((onThreshold >= RTL8367C_PAGE_NUMBER) || (offThreshold >= RTL8367C_PAGE_NUMBER))
        return RT_ERR_OUT_OF_RANGE;

    retVal = rtl8367c_setAsicRegBits(RTL8367C_REG_FLOWCTRL_PORT_OFF, RTL8367C_FLOWCTRL_PORT_OFF_MASK, offThreshold);

    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8367c_setAsicRegBits(RTL8367C_REG_FLOWCTRL_PORT_ON, RTL8367C_FLOWCTRL_PORT_ON_MASK, onThreshold);

    return retVal;
}
/* Function Name:
 *      rtl8367c_getAsicFlowControlPortThreshold
 * Description:
 *      Get Port-based flow control parameters
 * Input:
 *      pOnThreshold    - Flow control turn ON threshold
 *      pOffThreshold   - Flow control turn OFF threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicFlowControlPortThreshold(rtk_uint32 *pOnThreshold, rtk_uint32 *pOffThreshold)
{
    ret_t retVal;

    retVal = rtl8367c_getAsicRegBits(RTL8367C_REG_FLOWCTRL_PORT_OFF, RTL8367C_FLOWCTRL_PORT_OFF_MASK, pOffThreshold);

    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8367c_getAsicRegBits(RTL8367C_REG_FLOWCTRL_PORT_ON, RTL8367C_FLOWCTRL_PORT_ON_MASK, pOnThreshold);

    return retVal;
}
/* Function Name:
 *      rtl8367c_setAsicFlowControlPortPrivateThreshold
 * Description:
 *      Set Port-private-based flow control parameters
 * Input:
 *      onThreshold     - Flow control turn ON threshold
 *      offThreshold    - Flow control turn OFF threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicFlowControlPortPrivateThreshold(rtk_uint32 onThreshold, rtk_uint32 offThreshold)
{
    ret_t retVal;

    if((onThreshold >= RTL8367C_PAGE_NUMBER) || (offThreshold >= RTL8367C_PAGE_NUMBER))
        return RT_ERR_OUT_OF_RANGE;

    retVal = rtl8367c_setAsicRegBits(RTL8367C_REG_FLOWCTRL_PORT_PRIVATE_OFF, RTL8367C_FLOWCTRL_PORT_PRIVATE_OFF_MASK, offThreshold);

    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8367c_setAsicRegBits(RTL8367C_REG_FLOWCTRL_PORT_PRIVATE_ON, RTL8367C_FLOWCTRL_PORT_PRIVATE_ON_MASK, onThreshold);

    return retVal;
}
/* Function Name:
 *      rtl8367c_getAsicFlowControlPortPrivateThreshold
 * Description:
 *      Get Port-private-based flow control parameters
 * Input:
 *      pOnThreshold    - Flow control turn ON threshold
 *      pOffThreshold   - Flow control turn OFF threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicFlowControlPortPrivateThreshold(rtk_uint32 *pOnThreshold, rtk_uint32 *pOffThreshold)
{
    ret_t retVal;

    retVal = rtl8367c_getAsicRegBits(RTL8367C_REG_FLOWCTRL_PORT_PRIVATE_OFF, RTL8367C_FLOWCTRL_PORT_PRIVATE_OFF_MASK, pOffThreshold);

    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8367c_getAsicRegBits(RTL8367C_REG_FLOWCTRL_PORT_PRIVATE_ON, RTL8367C_FLOWCTRL_PORT_PRIVATE_ON_MASK, pOnThreshold);

    return retVal;
}
/* Function Name:
 *      rtl8367c_setAsicFlowControlSystemDropThreshold
 * Description:
 *      Set system-based drop parameters
 * Input:
 *      onThreshold     - Drop turn ON threshold
 *      offThreshold    - Drop turn OFF threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicFlowControlSystemDropThreshold(rtk_uint32 onThreshold, rtk_uint32 offThreshold)
{
    ret_t retVal;

    if((onThreshold >= RTL8367C_PAGE_NUMBER) || (offThreshold >= RTL8367C_PAGE_NUMBER))
        return RT_ERR_OUT_OF_RANGE;

    retVal = rtl8367c_setAsicRegBits(RTL8367C_REG_FLOWCTRL_FCOFF_SYS_OFF, RTL8367C_FLOWCTRL_FCOFF_SYS_OFF_MASK, offThreshold);

    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8367c_setAsicRegBits(RTL8367C_REG_FLOWCTRL_FCOFF_SYS_ON, RTL8367C_FLOWCTRL_FCOFF_SYS_ON_MASK, onThreshold);

    return retVal;
}
/* Function Name:
 *      rtl8367c_getAsicFlowControlSystemDropThreshold
 * Description:
 *      Get system-based drop parameters
 * Input:
 *      pOnThreshold    - Drop turn ON threshold
 *      pOffThreshold   - Drop turn OFF threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicFlowControlSystemDropThreshold(rtk_uint32 *pOnThreshold, rtk_uint32 *pOffThreshold)
{
    ret_t retVal;

    retVal = rtl8367c_getAsicRegBits(RTL8367C_REG_FLOWCTRL_FCOFF_SYS_OFF, RTL8367C_FLOWCTRL_FCOFF_SYS_OFF_MASK, pOffThreshold);

    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8367c_getAsicRegBits(RTL8367C_REG_FLOWCTRL_FCOFF_SYS_ON, RTL8367C_FLOWCTRL_FCOFF_SYS_ON_MASK, pOnThreshold);

    return retVal;
}
/* Function Name:
 *      rtl8367c_setAsicFlowControlSharedDropThreshold
 * Description:
 *      Set share-based fdrop parameters
 * Input:
 *      onThreshold     - Drop turn ON threshold
 *      offThreshold    - Drop turn OFF threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicFlowControlSharedDropThreshold(rtk_uint32 onThreshold, rtk_uint32 offThreshold)
{
    ret_t retVal;

    if((onThreshold >= RTL8367C_PAGE_NUMBER) || (offThreshold >= RTL8367C_PAGE_NUMBER))
        return RT_ERR_OUT_OF_RANGE;

    retVal = rtl8367c_setAsicRegBits(RTL8367C_REG_FLOWCTRL_FCOFF_SHARE_OFF, RTL8367C_FLOWCTRL_FCOFF_SHARE_OFF_MASK, offThreshold);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8367c_setAsicRegBits(RTL8367C_REG_FLOWCTRL_FCOFF_SHARE_ON, RTL8367C_FLOWCTRL_FCOFF_SHARE_ON_MASK, onThreshold);

    return retVal;
}
/* Function Name:
 *      rtl8367c_getAsicFlowControlSharedDropThreshold
 * Description:
 *      Get share-based fdrop parameters
 * Input:
 *      pOnThreshold    - Drop turn ON threshold
 *      pOffThreshold   - Drop turn OFF threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicFlowControlSharedDropThreshold(rtk_uint32 *pOnThreshold, rtk_uint32 *pOffThreshold)
{
    ret_t retVal;

    retVal = rtl8367c_getAsicRegBits(RTL8367C_REG_FLOWCTRL_FCOFF_SHARE_OFF, RTL8367C_FLOWCTRL_FCOFF_SHARE_OFF_MASK, pOffThreshold);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8367c_getAsicRegBits(RTL8367C_REG_FLOWCTRL_FCOFF_SHARE_ON, RTL8367C_FLOWCTRL_FCOFF_SHARE_ON_MASK, pOnThreshold);

    return retVal;
}
/* Function Name:
 *      rtl8367c_setAsicFlowControlPortDropThreshold
 * Description:
 *      Set Port-based drop parameters
 * Input:
 *      onThreshold     - Drop turn ON threshold
 *      offThreshold    - Drop turn OFF threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicFlowControlPortDropThreshold(rtk_uint32 onThreshold, rtk_uint32 offThreshold)
{
    ret_t retVal;

    if((onThreshold >= RTL8367C_PAGE_NUMBER) || (offThreshold >= RTL8367C_PAGE_NUMBER))
        return RT_ERR_OUT_OF_RANGE;

    retVal = rtl8367c_setAsicRegBits(RTL8367C_REG_FLOWCTRL_FCOFF_PORT_OFF, RTL8367C_FLOWCTRL_FCOFF_PORT_OFF_MASK, offThreshold);

    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8367c_setAsicRegBits(RTL8367C_REG_FLOWCTRL_FCOFF_PORT_ON, RTL8367C_FLOWCTRL_FCOFF_PORT_ON_MASK, onThreshold);

    return retVal;
}
/* Function Name:
 *      rtl8367c_getAsicFlowControlPortDropThreshold
 * Description:
 *      Get Port-based drop parameters
 * Input:
 *      pOnThreshold    - Drop turn ON threshold
 *      pOffThreshold   - Drop turn OFF threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicFlowControlPortDropThreshold(rtk_uint32 *pOnThreshold, rtk_uint32 *pOffThreshold)
{
    ret_t retVal;

    retVal = rtl8367c_getAsicRegBits(RTL8367C_REG_FLOWCTRL_FCOFF_PORT_OFF, RTL8367C_FLOWCTRL_FCOFF_PORT_OFF_MASK, pOffThreshold);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8367c_getAsicRegBits(RTL8367C_REG_FLOWCTRL_FCOFF_PORT_ON, RTL8367C_FLOWCTRL_FCOFF_PORT_ON_MASK, pOnThreshold);

    return retVal;
}
/* Function Name:
 *      rtl8367c_setAsicFlowControlPortPrivateDropThreshold
 * Description:
 *      Set Port-private-based drop parameters
 * Input:
 *      onThreshold     - Drop turn ON threshold
 *      offThreshold    - Drop turn OFF threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicFlowControlPortPrivateDropThreshold(rtk_uint32 onThreshold, rtk_uint32 offThreshold)
{
    ret_t retVal;

    if((onThreshold >= RTL8367C_PAGE_NUMBER) || (offThreshold >= RTL8367C_PAGE_NUMBER))
        return RT_ERR_OUT_OF_RANGE;

    retVal = rtl8367c_setAsicRegBits(RTL8367C_REG_FLOWCTRL_FCOFF_PORT_PRIVATE_OFF, RTL8367C_FLOWCTRL_FCOFF_PORT_PRIVATE_OFF_MASK, offThreshold);

    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8367c_setAsicRegBits(RTL8367C_REG_FLOWCTRL_FCOFF_PORT_PRIVATE_ON, RTL8367C_FLOWCTRL_FCOFF_PORT_PRIVATE_ON_MASK, onThreshold);

    return retVal;
}
/* Function Name:
 *      rtl8367c_getAsicFlowControlPortPrivateDropThreshold
 * Description:
 *      Get Port-private-based drop parameters
 * Input:
 *      pOnThreshold    - Drop turn ON threshold
 *      pOffThreshold   - Drop turn OFF threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicFlowControlPortPrivateDropThreshold(rtk_uint32 *pOnThreshold, rtk_uint32 *pOffThreshold)
{
    ret_t retVal;

    retVal = rtl8367c_getAsicRegBits(RTL8367C_REG_FLOWCTRL_FCOFF_PORT_PRIVATE_OFF, RTL8367C_FLOWCTRL_FCOFF_PORT_PRIVATE_OFF_MASK, pOffThreshold);
    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8367c_getAsicRegBits(RTL8367C_REG_FLOWCTRL_FCOFF_PORT_PRIVATE_ON, RTL8367C_FLOWCTRL_FCOFF_PORT_PRIVATE_ON_MASK, pOnThreshold);

    return retVal;
}
/* Function Name:
 *      rtl8367c_setAsicFlowControlSystemJumboThreshold
 * Description:
 *      Set Jumbo system-based flow control parameters
 * Input:
 *      onThreshold     - Flow control turn ON threshold
 *      offThreshold    - Flow control turn OFF threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicFlowControlSystemJumboThreshold(rtk_uint32 onThreshold, rtk_uint32 offThreshold)
{
    ret_t retVal;

    if((onThreshold >= RTL8367C_PAGE_NUMBER) || (offThreshold >= RTL8367C_PAGE_NUMBER))
        return RT_ERR_OUT_OF_RANGE;

    retVal = rtl8367c_setAsicRegBits(RTL8367C_REG_FLOWCTRL_JUMBO_SYS_OFF, RTL8367C_FLOWCTRL_JUMBO_SYS_OFF_MASK, offThreshold);

    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8367c_setAsicRegBits(RTL8367C_REG_FLOWCTRL_JUMBO_SYS_ON, RTL8367C_FLOWCTRL_JUMBO_SYS_ON_MASK, onThreshold);

    return retVal;
}
/* Function Name:
 *      rtl8367c_getAsicFlowControlSystemJumboThreshold
 * Description:
 *      Get Jumbo system-based flow control parameters
 * Input:
 *      pOnThreshold    - Flow control turn ON threshold
 *      pOffThreshold   - Flow control turn OFF threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicFlowControlSystemJumboThreshold(rtk_uint32 *pOnThreshold, rtk_uint32 *pOffThreshold)
{
    ret_t retVal;

    retVal = rtl8367c_getAsicRegBits(RTL8367C_REG_FLOWCTRL_JUMBO_SYS_OFF, RTL8367C_FLOWCTRL_JUMBO_SYS_OFF_MASK, pOffThreshold);

    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8367c_getAsicRegBits(RTL8367C_REG_FLOWCTRL_JUMBO_SYS_ON, RTL8367C_FLOWCTRL_JUMBO_SYS_ON_MASK, pOnThreshold);

    return retVal;
}
/* Function Name:
 *      rtl8367c_setAsicFlowControlSharedJumboThreshold
 * Description:
 *      Set Jumbo share-based flow control parameters
 * Input:
 *      onThreshold     - Flow control turn ON threshold
 *      offThreshold    - Flow control turn OFF threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicFlowControlSharedJumboThreshold(rtk_uint32 onThreshold, rtk_uint32 offThreshold)
{
    ret_t retVal;

    if((onThreshold >= RTL8367C_PAGE_NUMBER) || (offThreshold >= RTL8367C_PAGE_NUMBER))
        return RT_ERR_OUT_OF_RANGE;

    retVal = rtl8367c_setAsicRegBits(RTL8367C_REG_FLOWCTRL_JUMBO_SHARE_OFF, RTL8367C_FLOWCTRL_JUMBO_SHARE_OFF_MASK, offThreshold);

    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8367c_setAsicRegBits(RTL8367C_REG_FLOWCTRL_JUMBO_SHARE_ON, RTL8367C_FLOWCTRL_JUMBO_SHARE_ON_MASK, onThreshold);

    return retVal;
}
/* Function Name:
 *      rtl8367c_getAsicFlowControlSharedJumboThreshold
 * Description:
 *      Get Jumbo share-based flow control parameters
 * Input:
 *      pOnThreshold    - Flow control turn ON threshold
 *      pOffThreshold   - Flow control turn OFF threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicFlowControlSharedJumboThreshold(rtk_uint32 *pOnThreshold, rtk_uint32 *pOffThreshold)
{
    ret_t retVal;

    retVal = rtl8367c_getAsicRegBits(RTL8367C_REG_FLOWCTRL_JUMBO_SHARE_OFF, RTL8367C_FLOWCTRL_JUMBO_SHARE_OFF_MASK, pOffThreshold);

    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8367c_getAsicRegBits(RTL8367C_REG_FLOWCTRL_JUMBO_SHARE_ON, RTL8367C_FLOWCTRL_JUMBO_SHARE_ON_MASK, pOnThreshold);

    return retVal;
}
/* Function Name:
 *      rtl8367c_setAsicFlowControlPortJumboThreshold
 * Description:
 *      Set Jumbo Port-based flow control parameters
 * Input:
 *      onThreshold     - Flow control turn ON threshold
 *      offThreshold    - Flow control turn OFF threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicFlowControlPortJumboThreshold(rtk_uint32 onThreshold, rtk_uint32 offThreshold)
{
    ret_t retVal;

    if((onThreshold >= RTL8367C_PAGE_NUMBER) || (offThreshold >= RTL8367C_PAGE_NUMBER))
        return RT_ERR_OUT_OF_RANGE;

    retVal = rtl8367c_setAsicRegBits(RTL8367C_REG_FLOWCTRL_JUMBO_PORT_OFF, RTL8367C_FLOWCTRL_JUMBO_PORT_OFF_MASK, offThreshold);

    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8367c_setAsicRegBits(RTL8367C_REG_FLOWCTRL_JUMBO_PORT_ON, RTL8367C_FLOWCTRL_JUMBO_PORT_ON_MASK, onThreshold);

    return retVal;
}
/* Function Name:
 *      rtl8367c_getAsicFlowControlPortJumboThreshold
 * Description:
 *      Get Jumbo Port-based flow control parameters
 * Input:
 *      pOnThreshold    - Flow control turn ON threshold
 *      pOffThreshold   - Flow control turn OFF threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicFlowControlPortJumboThreshold(rtk_uint32 *pOnThreshold, rtk_uint32 *pOffThreshold)
{
    ret_t retVal;

    retVal = rtl8367c_getAsicRegBits(RTL8367C_REG_FLOWCTRL_JUMBO_PORT_OFF, RTL8367C_FLOWCTRL_JUMBO_PORT_OFF_MASK, pOffThreshold);

    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8367c_getAsicRegBits(RTL8367C_REG_FLOWCTRL_JUMBO_PORT_ON, RTL8367C_FLOWCTRL_JUMBO_PORT_ON_MASK, pOnThreshold);

    return retVal;
}
/* Function Name:
 *      rtl8367c_setAsicFlowControlPortPrivateJumboThreshold
 * Description:
 *      Set Jumbo Port-private-based flow control parameters
 * Input:
 *      onThreshold     - Flow control turn ON threshold
 *      offThreshold    - Flow control turn OFF threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicFlowControlPortPrivateJumboThreshold(rtk_uint32 onThreshold, rtk_uint32 offThreshold)
{
    ret_t retVal;

    if((onThreshold >= RTL8367C_PAGE_NUMBER) || (offThreshold >= RTL8367C_PAGE_NUMBER))
        return RT_ERR_OUT_OF_RANGE;

    retVal = rtl8367c_setAsicRegBits(RTL8367C_REG_FLOWCTRL_JUMBO_PORT_PRIVATE_OFF, RTL8367C_FLOWCTRL_JUMBO_PORT_PRIVATE_OFF_MASK, offThreshold);

    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8367c_setAsicRegBits(RTL8367C_REG_FLOWCTRL_JUMBO_PORT_PRIVATE_ON, RTL8367C_FLOWCTRL_JUMBO_PORT_PRIVATE_ON_MASK, onThreshold);

    return retVal;
}
/* Function Name:
 *      rtl8367c_getAsicFlowControlPortPrivateJumboThreshold
 * Description:
 *      Get Jumbo Port-private-based flow control parameters
 * Input:
 *      pOnThreshold    - Flow control turn ON threshold
 *      pOffThreshold   - Flow control turn OFF threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicFlowControlPortPrivateJumboThreshold(rtk_uint32 *pOnThreshold, rtk_uint32 *pOffThreshold)
{
    ret_t retVal;

    retVal = rtl8367c_getAsicRegBits(RTL8367C_REG_FLOWCTRL_JUMBO_PORT_PRIVATE_OFF, RTL8367C_FLOWCTRL_JUMBO_PORT_PRIVATE_OFF_MASK, pOffThreshold);

    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8367c_getAsicRegBits(RTL8367C_REG_FLOWCTRL_JUMBO_PORT_PRIVATE_ON, RTL8367C_FLOWCTRL_JUMBO_PORT_PRIVATE_ON_MASK, pOnThreshold);

    return retVal;
}



/* Function Name:
 *      rtl8367c_setAsicEgressFlowControlQueueDropThreshold
 * Description:
 *      Set Queue-based egress flow control turn on or ingress flow control drop on threshold
 * Input:
 *      qid         - The queue id
 *      threshold   - Queue-based flown control/drop turn ON threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 *      RT_ERR_QUEUE_ID     - Invalid queue id
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicEgressFlowControlQueueDropThreshold(rtk_uint32 qid, rtk_uint32 threshold)
{
    if( threshold >= RTL8367C_PAGE_NUMBER)
        return RT_ERR_OUT_OF_RANGE;

    if(qid > RTL8367C_QIDMAX)
        return RT_ERR_QUEUE_ID;

    return rtl8367c_setAsicRegBits(RTL8367C_FLOWCTRL_QUEUE_DROP_ON_REG(qid), RTL8367C_FLOWCTRL_QUEUE_DROP_ON_MASK, threshold);
}
/* Function Name:
 *      rtl8367c_getAsicEgressFlowControlQueueDropThreshold
 * Description:
 *      Get Queue-based egress flow control turn on or ingress flow control drop on threshold
 * Input:
 *      qid         - The queue id
 *      pThreshold  - Queue-based flown control/drop turn ON threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_QUEUE_ID     - Invalid queue id
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicEgressFlowControlQueueDropThreshold(rtk_uint32 qid, rtk_uint32 *pThreshold)
{
    if(qid > RTL8367C_QIDMAX)
      return RT_ERR_QUEUE_ID;

    return rtl8367c_getAsicRegBits(RTL8367C_FLOWCTRL_QUEUE_DROP_ON_REG(qid), RTL8367C_FLOWCTRL_QUEUE_DROP_ON_MASK, pThreshold);
}
/* Function Name:
 *      rtl8367c_setAsicEgressFlowControlPortDropThreshold
 * Description:
 *      Set port-based egress flow control turn on or ingress flow control drop on threshold
 * Input:
 *      port        - Physical port number (0~7)
 *      threshold   - Queue-based flown control/drop turn ON threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicEgressFlowControlPortDropThreshold(rtk_uint32 port, rtk_uint32 threshold)
{
    if(port > RTL8367C_PORTIDMAX)
      return RT_ERR_PORT_ID;

    if(threshold >= RTL8367C_PAGE_NUMBER)
      return RT_ERR_OUT_OF_RANGE;

    return rtl8367c_setAsicRegBits(RTL8367C_FLOWCTRL_PORT_DROP_ON_REG(port), RTL8367C_FLOWCTRL_PORT_DROP_ON_MASK, threshold);
}
/* Function Name:
 *      rtl8367c_setAsicEgressFlowControlPortDropThreshold
 * Description:
 *      Set port-based egress flow control turn on or ingress flow control drop on threshold
 * Input:
 *      port        - Physical port number (0~7)
 *      pThreshold  - Queue-based flown control/drop turn ON threshold
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicEgressFlowControlPortDropThreshold(rtk_uint32 port, rtk_uint32 *pThreshold)
{
    if(port > RTL8367C_PORTIDMAX)
        return RT_ERR_PORT_ID;

    return rtl8367c_getAsicRegBits(RTL8367C_FLOWCTRL_PORT_DROP_ON_REG(port), RTL8367C_FLOWCTRL_PORT_DROP_ON_MASK, pThreshold);
}
/* Function Name:
 *      rtl8367c_setAsicEgressFlowControlPortDropGap
 * Description:
 *      Set port-based egress flow control turn off or ingress flow control drop off gap
 * Input:
 *      gap     - Flow control/drop turn OFF threshold = turn ON threshold - gap
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicEgressFlowControlPortDropGap(rtk_uint32 gap)
{
    if(gap >= RTL8367C_PAGE_NUMBER)
        return RT_ERR_OUT_OF_RANGE;

    return rtl8367c_setAsicRegBits(RTL8367C_REG_FLOWCTRL_PORT_GAP, RTL8367C_FLOWCTRL_PORT_GAP_MASK, gap);
}
/* Function Name:
 *      rtl8367c_getAsicEgressFlowControlPortDropGap
 * Description:
 *      Get port-based egress flow control turn off or ingress flow control drop off gap
 * Input:
 *      pGap    - Flow control/drop turn OFF threshold = turn ON threshold - gap
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicEgressFlowControlPortDropGap(rtk_uint32 *pGap)
{
    return rtl8367c_getAsicRegBits(RTL8367C_REG_FLOWCTRL_PORT_GAP, RTL8367C_FLOWCTRL_PORT_GAP_MASK, pGap);
}
/* Function Name:
 *      rtl8367c_setAsicEgressFlowControlQueueDropGap
 * Description:
 *      Set Queue-based egress flow control turn off or ingress flow control drop off gap
 * Input:
 *      gap     - Flow control/drop turn OFF threshold = turn ON threshold - gap
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicEgressFlowControlQueueDropGap(rtk_uint32 gap)
{
    if(gap >= RTL8367C_PAGE_NUMBER)
        return RT_ERR_OUT_OF_RANGE;

    return rtl8367c_setAsicRegBits(RTL8367C_REG_FLOWCTRL_QUEUE_GAP, RTL8367C_FLOWCTRL_QUEUE_GAP_MASK, gap);
}
/* Function Name:
 *      rtl8367c_getAsicEgressFlowControlQueueDropGap
 * Description:
 *      Get Queue-based egress flow control turn off or ingress flow control drop off gap
 * Input:
 *      pGap    - Flow control/drop turn OFF threshold = turn ON threshold - gap
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicEgressFlowControlQueueDropGap(rtk_uint32 *pGap)
{
    return rtl8367c_getAsicRegBits(RTL8367C_REG_FLOWCTRL_QUEUE_GAP, RTL8367C_FLOWCTRL_QUEUE_GAP_MASK, pGap);
}
/* Function Name:
 *      rtl8367c_getAsicEgressQueueEmptyPortMask
 * Description:
 *      Get queue empty port mask
 * Input:
 *      pPortmask   -  Queue empty port mask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicEgressQueueEmptyPortMask(rtk_uint32 *pPortmask)
{
    return rtl8367c_getAsicReg(RTL8367C_REG_PORT_QEMPTY, pPortmask);
}
/* Function Name:
 *      rtl8367c_getAsicTotalPage
 * Description:
 *      Get system total page usage number
 * Input:
 *      pPageCount  -  page usage number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicTotalPage(rtk_uint32 *pPageCount)
{
    return rtl8367c_getAsicRegBits(RTL8367C_REG_FLOWCTRL_TOTAL_PAGE_COUNTER, RTL8367C_FLOWCTRL_TOTAL_PAGE_COUNTER_MASK, pPageCount);
}
/* Function Name:
 *      rtl8367c_getAsicPulbicPage
 * Description:
 *      Get system public page usage number
 * Input:
 *      pPageCount  -  page usage number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicPulbicPage(rtk_uint32 *pPageCount)
{
    return rtl8367c_getAsicRegBits(RTL8367C_REG_FLOWCTRL_PUBLIC_PAGE_COUNTER, RTL8367C_FLOWCTRL_PUBLIC_PAGE_COUNTER_MASK, pPageCount);
}
/* Function Name:
 *      rtl8367c_getAsicMaxTotalPage
 * Description:
 *      Get system total page max usage number
 * Input:
 *      pPageCount  -  page usage number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicMaxTotalPage(rtk_uint32 *pPageCount)
{
    return rtl8367c_getAsicRegBits(RTL8367C_REG_FLOWCTRL_TOTAL_PAGE_MAX, RTL8367C_FLOWCTRL_TOTAL_PAGE_MAX_MASK, pPageCount);
}
/* Function Name:
 *      rtl8367c_getAsicPulbicPage
 * Description:
 *      Get system public page max usage number
 * Input:
 *      pPageCount  -  page usage number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicMaxPulbicPage(rtk_uint32 *pPageCount)
{
    return rtl8367c_getAsicRegBits(RTL8367C_REG_FLOWCTRL_PUBLIC_PAGE_MAX, RTL8367C_FLOWCTRL_PUBLIC_PAGE_MAX_MASK, pPageCount);
}
/* Function Name:
 *      rtl8367c_getAsicPortPage
 * Description:
 *      Get per-port page usage number
 * Input:
 *      port        -  Physical port number (0~7)
 *      pPageCount  -  page usage number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicPortPage(rtk_uint32 port, rtk_uint32 *pPageCount)
{
    if(port > RTL8367C_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(port < 8)
        return rtl8367c_getAsicRegBits(RTL8367C_FLOWCTRL_PORT_PAGE_COUNTER_REG(port), RTL8367C_FLOWCTRL_PORT_PAGE_COUNTER_MASK, pPageCount);
    else
        return rtl8367c_getAsicRegBits(RTL8367C_REG_FLOWCTRL_PORT8_PAGE_COUNTER+port - 8, RTL8367C_FLOWCTRL_PORT_PAGE_COUNTER_MASK, pPageCount);
}
/* Function Name:
 *      rtl8367c_getAsicPortPage
 * Description:
 *      Get per-port page max usage number
 * Input:
 *      port        -  Physical port number (0~7)
 *      pPageCount  -  page usage number
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicPortPageMax(rtk_uint32 port, rtk_uint32 *pPageCount)
{
    if(port > RTL8367C_PORTIDMAX)
        return RT_ERR_PORT_ID;
    if(port < 8)
        return rtl8367c_getAsicRegBits(RTL8367C_FLOWCTRL_PORT_PAGE_MAX_REG(port), RTL8367C_FLOWCTRL_PORT_PAGE_MAX_MASK, pPageCount);
    else
        return rtl8367c_getAsicRegBits(RTL8367C_REG_FLOWCTRL_PORT0_PAGE_MAX+port-8, RTL8367C_FLOWCTRL_PORT_PAGE_MAX_MASK, pPageCount);


}

/* Function Name:
 *      rtl8367c_setAsicFlowControlEgressPortIndep
 * Description:
 *      Set per-port egress flow control independent
 * Input:
 *      port        - Physical port number (0~7)
 *      enabled     - Egress port flow control usage 1:enable 0:disable.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicFlowControlEgressPortIndep(rtk_uint32 port, rtk_uint32 enable)
{
    if(port > RTL8367C_PORTIDMAX)
        return RT_ERR_PORT_ID;

    return rtl8367c_setAsicRegBit(RTL8367C_REG_PORT0_MISC_CFG + (port *0x20), RTL8367C_PORT0_MISC_CFG_FLOWCTRL_INDEP_OFFSET,enable);
}

/* Function Name:
 *      rtl8367c_getAsicFlowControlEgressPortIndep
 * Description:
 *      Get per-port egress flow control independent
 * Input:
 *      port        - Physical port number (0~7)
 *      enabled     - Egress port flow control usage 1:enable 0:disable.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicFlowControlEgressPortIndep(rtk_uint32 port, rtk_uint32 *pEnable)
{
    return rtl8367c_getAsicRegBit(RTL8367C_REG_PORT0_MISC_CFG + (port *0x20),RTL8367C_PORT0_MISC_CFG_FLOWCTRL_INDEP_OFFSET,pEnable);
}

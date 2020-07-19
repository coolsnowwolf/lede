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
 * Feature : Packet Scheduling related functions
 *
 */

#include <rtl8367c_asicdrv_scheduling.h>
/* Function Name:
 *      rtl8367c_setAsicLeakyBucketParameter
 * Description:
 *      Set Leaky Bucket Paramters
 * Input:
 *      tick    - Tick is used for time slot size unit
 *      token   - Token is used for adding budget in each time slot
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_TICK     - Invalid TICK
 *      RT_ERR_TOKEN    - Invalid TOKEN
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicLeakyBucketParameter(rtk_uint32 tick, rtk_uint32 token)
{
    ret_t retVal;

    if(tick > 0xFF)
        return RT_ERR_TICK;

    if(token > 0xFF)
        return RT_ERR_TOKEN;

    retVal = rtl8367c_setAsicRegBits(RTL8367C_LEAKY_BUCKET_TICK_REG, RTL8367C_LEAKY_BUCKET_TICK_MASK, tick);

    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8367c_setAsicRegBits(RTL8367C_LEAKY_BUCKET_TOKEN_REG, RTL8367C_LEAKY_BUCKET_TOKEN_MASK, token);

    return retVal;
}
/* Function Name:
 *      rtl8367c_getAsicLeakyBucketParameter
 * Description:
 *      Get Leaky Bucket Paramters
 * Input:
 *      tick    - Tick is used for time slot size unit
 *      token   - Token is used for adding budget in each time slot
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicLeakyBucketParameter(rtk_uint32 *tick, rtk_uint32 *token)
{
    ret_t retVal;

    retVal = rtl8367c_getAsicRegBits(RTL8367C_LEAKY_BUCKET_TICK_REG, RTL8367C_LEAKY_BUCKET_TICK_MASK, tick);

    if(retVal != RT_ERR_OK)
        return retVal;

    retVal = rtl8367c_getAsicRegBits(RTL8367C_LEAKY_BUCKET_TOKEN_REG, RTL8367C_LEAKY_BUCKET_TOKEN_MASK, token);

    return retVal;
}
/* Function Name:
 *      rtl8367c_setAsicAprMeter
 * Description:
 *      Set per-port per-queue APR shared meter index
 * Input:
 *      port    - Physical port number (0~10)
 *      qid     - Queue id
 *      apridx  - dedicated shared meter index for APR (0~7)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - Success
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_PORT_ID          - Invalid port number
 *      RT_ERR_QUEUE_ID         - Invalid queue id
 *      RT_ERR_FILTER_METER_ID  - Invalid meter
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicAprMeter(rtk_uint32 port, rtk_uint32 qid, rtk_uint32 apridx)
{
    ret_t retVal;
    rtk_uint32 regAddr;

    if(port > RTL8367C_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(qid > RTL8367C_QIDMAX)
        return RT_ERR_QUEUE_ID;

    if(apridx > RTL8367C_PORT_QUEUE_METER_INDEX_MAX)
        return RT_ERR_FILTER_METER_ID;

    if(port < 8)
        retVal = rtl8367c_setAsicRegBits(RTL8367C_SCHEDULE_PORT_APR_METER_REG(port, qid), RTL8367C_SCHEDULE_PORT_APR_METER_MASK(qid), apridx);
    else {
        regAddr = RTL8367C_REG_SCHEDULE_PORT8_APR_METER_CTRL0 + ((port-8) << 1) + (qid / 5);
        retVal = rtl8367c_setAsicRegBits(regAddr, RTL8367C_SCHEDULE_PORT_APR_METER_MASK(qid), apridx);
    }

    return retVal;
}
/* Function Name:
 *      rtl8367c_getAsicAprMeter
 * Description:
 *      Get per-port per-queue APR shared meter index
 * Input:
 *      port    - Physical port number (0~10)
 *      qid     - Queue id
 *      apridx  - dedicated shared meter index for APR (0~7)
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
ret_t rtl8367c_getAsicAprMeter(rtk_uint32 port, rtk_uint32 qid, rtk_uint32 *apridx)
{
    ret_t retVal;
    rtk_uint32 regAddr;

    if(port > RTL8367C_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(qid > RTL8367C_QIDMAX)
        return RT_ERR_QUEUE_ID;

    if(port < 8)
        retVal = rtl8367c_getAsicRegBits(RTL8367C_SCHEDULE_PORT_APR_METER_REG(port, qid), RTL8367C_SCHEDULE_PORT_APR_METER_MASK(qid), apridx);
    else {
        regAddr = RTL8367C_REG_SCHEDULE_PORT8_APR_METER_CTRL0 + ((port-8) << 1) + (qid / 5);
        retVal = rtl8367c_getAsicRegBits(regAddr, RTL8367C_SCHEDULE_PORT_APR_METER_MASK(qid), apridx);
    }

    return retVal;
}
/* Function Name:
 *      rtl8367c_setAsicAprEnable
 * Description:
 *      Set per-port APR enable
 * Input:
 *      port        - Physical port number (0~7)
 *      aprEnable   - APR enable seting 1:enable 0:disable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicAprEnable(rtk_uint32 port, rtk_uint32 aprEnable)
{
    ret_t retVal;

    if(port > RTL8367C_PORTIDMAX)
        return RT_ERR_PORT_ID;

    retVal = rtl8367c_setAsicRegBit(RTL8367C_SCHEDULE_APR_CTRL_REG, RTL8367C_SCHEDULE_APR_CTRL_OFFSET(port), aprEnable);

    return retVal;
}
/* Function Name:
 *      rtl8367c_getAsicAprEnable
 * Description:
 *      Get per-port APR enable
 * Input:
 *      port        - Physical port number (0~7)
 *      aprEnable   - APR enable seting 1:enable 0:disable
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK       - Success
 *      RT_ERR_SMI      - SMI access error
 *      RT_ERR_PORT_ID  - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicAprEnable(rtk_uint32 port, rtk_uint32 *aprEnable)
{
    ret_t retVal;

    if(port > RTL8367C_PORTIDMAX)
        return RT_ERR_PORT_ID;

    retVal = rtl8367c_getAsicRegBit(RTL8367C_SCHEDULE_APR_CTRL_REG, RTL8367C_SCHEDULE_APR_CTRL_OFFSET(port), aprEnable);

    return retVal;
}
/* Function Name:
 *      rtl8367c_setAsicWFQWeight
 * Description:
 *      Set weight  of a queue
 * Input:
 *      port    - Physical port number (0~10)
 *      qid     - The queue ID wanted to set
 *      qWeight - The weight value wanted to set (valid:0~127)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - Success
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_PORT_ID          - Invalid port number
 *      RT_ERR_QUEUE_ID         - Invalid queue id
 *      RT_ERR_QOS_QUEUE_WEIGHT - Invalid queue weight
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicWFQWeight(rtk_uint32 port, rtk_uint32 qid, rtk_uint32 qWeight)
{
    ret_t retVal;

    /* Invalid input parameter */
    if(port > RTL8367C_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(qid > RTL8367C_QIDMAX)
        return RT_ERR_QUEUE_ID;

    if(qWeight > RTL8367C_QWEIGHTMAX && qid > 0)
        return RT_ERR_QOS_QUEUE_WEIGHT;

    retVal = rtl8367c_setAsicReg(RTL8367C_SCHEDULE_PORT_QUEUE_WFQ_WEIGHT_REG(port, qid), qWeight);

    return retVal;
}
/* Function Name:
 *      rtl8367c_getAsicWFQWeight
 * Description:
 *      Get weight  of a queue
 * Input:
 *      port    - Physical port number (0~10)
 *      qid     - The queue ID wanted to set
 *      qWeight - The weight value wanted to set (valid:0~127)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - Success
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_PORT_ID          - Invalid port number
 *      RT_ERR_QUEUE_ID         - Invalid queue id
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicWFQWeight(rtk_uint32 port, rtk_uint32 qid, rtk_uint32 *qWeight)
{
    ret_t retVal;


    /* Invalid input parameter */
    if(port  > RTL8367C_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(qid > RTL8367C_QIDMAX)
        return RT_ERR_QUEUE_ID;


    retVal = rtl8367c_getAsicReg(RTL8367C_SCHEDULE_PORT_QUEUE_WFQ_WEIGHT_REG(port, qid), qWeight);

    return retVal;
}
/* Function Name:
 *      rtl8367c_setAsicWFQBurstSize
 * Description:
 *      Set WFQ leaky bucket burst size
 * Input:
 *      burstsize   - Leaky bucket burst size, unit byte
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicWFQBurstSize(rtk_uint32 burstsize)
{
    ret_t retVal;

    retVal = rtl8367c_setAsicReg(RTL8367C_SCHEDULE_WFQ_BURST_SIZE_REG, burstsize);

    return retVal;
}
/* Function Name:
 *      rtl8367c_getAsicWFQBurstSize
 * Description:
 *      Get WFQ leaky bucket burst size
 * Input:
 *      burstsize   - Leaky bucket burst size, unit byte
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicWFQBurstSize(rtk_uint32 *burstsize)
{
    ret_t retVal;

    retVal = rtl8367c_getAsicReg(RTL8367C_SCHEDULE_WFQ_BURST_SIZE_REG, burstsize);

    return retVal;
}
/* Function Name:
 *      rtl8367c_setAsicQueueType
 * Description:
 *      Set type of a queue
 * Input:
 *      port        - Physical port number (0~10)
 *      qid         - The queue ID wanted to set
 *      queueType   - The specified queue type. 0b0: Strict priority, 0b1: WFQ
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
ret_t rtl8367c_setAsicQueueType(rtk_uint32 port, rtk_uint32 qid, rtk_uint32 queueType)
{
    ret_t retVal;

    /* Invalid input parameter */
    if(port > RTL8367C_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(qid > RTL8367C_QIDMAX)
        return RT_ERR_QUEUE_ID;

    /* Set Related Registers */
    retVal = rtl8367c_setAsicRegBit(RTL8367C_SCHEDULE_QUEUE_TYPE_REG(port), RTL8367C_SCHEDULE_QUEUE_TYPE_OFFSET(port, qid),queueType);

    return retVal;
}
/* Function Name:
 *      rtl8367c_getAsicQueueType
 * Description:
 *      Get type of a queue
 * Input:
 *      port        - Physical port number (0~7)
 *      qid         - The queue ID wanted to set
 *      queueType   - The specified queue type. 0b0: Strict priority, 0b1: WFQ
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
ret_t rtl8367c_getAsicQueueType(rtk_uint32 port, rtk_uint32 qid, rtk_uint32 *queueType)
{
    ret_t retVal;

    /* Invalid input parameter */
    if(port  > RTL8367C_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(qid > RTL8367C_QIDMAX)
        return RT_ERR_QUEUE_ID;

    retVal = rtl8367c_getAsicRegBit(RTL8367C_SCHEDULE_QUEUE_TYPE_REG(port), RTL8367C_SCHEDULE_QUEUE_TYPE_OFFSET(port, qid),queueType);

    return retVal;
}
/* Function Name:
 *      rtl8367c_setAsicPortEgressRate
 * Description:
 *      Set per-port egress rate
 * Input:
 *      port        - Physical port number (0~10)
 *      rate        - Egress rate
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number
 *      RT_ERR_QOS_EBW_RATE - Invalid bandwidth/rate
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicPortEgressRate(rtk_uint32 port, rtk_uint32 rate)
{
    ret_t retVal;
    rtk_uint32 regAddr, regData;

    if(port > RTL8367C_PORTIDMAX)
        return RT_ERR_PORT_ID;

    if(rate > RTL8367C_QOS_GRANULARTY_MAX)
        return RT_ERR_QOS_EBW_RATE;

    regAddr = RTL8367C_PORT_EGRESSBW_LSB_REG(port);
    regData = RTL8367C_QOS_GRANULARTY_LSB_MASK & rate;

    retVal = rtl8367c_setAsicReg(regAddr, regData);

    if(retVal != RT_ERR_OK)
        return retVal;

    regAddr = RTL8367C_PORT_EGRESSBW_MSB_REG(port);
    regData = (RTL8367C_QOS_GRANULARTY_MSB_MASK & rate) >> RTL8367C_QOS_GRANULARTY_MSB_OFFSET;

    retVal = rtl8367c_setAsicRegBits(regAddr, RTL8367C_PORT6_EGRESSBW_CTRL1_MASK, regData);

    return retVal;
}
/* Function Name:
 *      rtl8367c_getAsicPortEgressRate
 * Description:
 *      Get per-port egress rate
 * Input:
 *      port        - Physical port number (0~10)
 *      rate        - Egress rate
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - Success
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicPortEgressRate(rtk_uint32 port, rtk_uint32 *rate)
{
    ret_t retVal;
    rtk_uint32 regAddr, regData,regData2;

    if(port > RTL8367C_PORTIDMAX)
        return RT_ERR_PORT_ID;

    regAddr = RTL8367C_PORT_EGRESSBW_LSB_REG(port);

    retVal = rtl8367c_getAsicReg(regAddr, &regData);
    if(retVal != RT_ERR_OK)
        return retVal;

    regAddr = RTL8367C_PORT_EGRESSBW_MSB_REG(port);
    retVal = rtl8367c_getAsicRegBits(regAddr, RTL8367C_PORT6_EGRESSBW_CTRL1_MASK, &regData2);
    if(retVal != RT_ERR_OK)
        return retVal;

    *rate = regData | (regData2 << RTL8367C_QOS_GRANULARTY_MSB_OFFSET);

    return RT_ERR_OK;
}
/* Function Name:
 *      rtl8367c_setAsicPortEgressRateIfg
 * Description:
 *      Set per-port egress rate calculate include/exclude IFG
 * Input:
 *      ifg     - 1:include IFG 0:exclude IFG
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_setAsicPortEgressRateIfg(rtk_uint32 ifg)
{
    ret_t retVal;

    retVal = rtl8367c_setAsicRegBit(RTL8367C_REG_SCHEDULE_WFQ_CTRL, RTL8367C_SCHEDULE_WFQ_CTRL_OFFSET, ifg);

    return retVal;
}
/* Function Name:
 *      rtl8367c_getAsicPortEgressRateIfg
 * Description:
 *      Get per-port egress rate calculate include/exclude IFG
 * Input:
 *      ifg     - 1:include IFG 0:exclude IFG
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK   - Success
 *      RT_ERR_SMI  - SMI access error
 * Note:
 *      None
 */
ret_t rtl8367c_getAsicPortEgressRateIfg(rtk_uint32 *ifg)
{
    ret_t retVal;

    retVal = rtl8367c_getAsicRegBit(RTL8367C_REG_SCHEDULE_WFQ_CTRL, RTL8367C_SCHEDULE_WFQ_CTRL_OFFSET, ifg);

    return retVal;
}

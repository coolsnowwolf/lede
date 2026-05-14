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
 * Purpose : RTK switch high-level API for RTL8373
 * Feature : Here is a list of all functions and variables in ShareMeter module.
 *
 */

#include <rtk_switch.h>
#include <rtk_error.h>
#include <dal/rtl8373/dal_rtl8373_sharemeter.h>
#include <string.h>
#include <sharemeter.h>

#include <dal/rtl8373/rtl8373_asicdrv.h>
#if 0
#define    RTL8373_SCHEDULE_PORT_APR_METER_REG(port, queue)    (RTL8373_REG_SCHEDULE_PORT0_APR_METER_CTRL0 + (port << 2) + (queue / 5))
#define    RTL8373_SCHEDULE_PORT_APR_METER_MASK(queue)         (RTL8373_SCHEDULE_PORT0_APR_METER_CTRL0_QUEUE0_APR_METER_MASK << (3 * (queue % 5)))

#define    RTL8373_MAX_NUM_OF_QUEUE     (8)
#endif

/* Function Name:
 *      dal_rtl8373_rate_shareMeter_set
 * Description:
 *      Set meter configuration
 * Input:
 *      index       - shared meter index(0 - 63)
 *      type        - shared meter type
 *      rate        - rate of share meter
 *      ifg_include - include IFG or not, ENABLE:include DISABLE:exclude
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_FILTER_METER_ID  - Invalid meter
 *      RT_ERR_RATE             - Invalid rate
 *      RT_ERR_INPUT            - Invalid input parameters
 * Note:
 *      The API can set shared meter rate and ifg include for each meter.
 *      The rate unit is 1 kbps and the range is from 0 to 0x989680 if type is METER_TYPE_KBPS and
 *      the granularity of rate is 1 kbps.
 *      The rate unit is packets per second and the range is 0 ~ 0xE310B8 if type is METER_TYPE_PPS.
 *      The ifg_include parameter is used
 *      for rate calculation with/without inter-frame-gap and preamble.
 */
rtk_api_ret_t dal_rtl8373_rate_shareMeter_set(rtk_meter_id_t index, rtk_meter_type_t type, rtk_rate_t rate, rtk_enable_t ifg_include)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (index > RTL8373_METERMAX)
        return RT_ERR_FILTER_METER_ID;

    if (type >= METER_TYPE_END)
        return RT_ERR_INPUT;

    if (ifg_include >= RTK_ENABLE_END)
        return RT_ERR_INPUT;

    switch (type)
    {
        case METER_TYPE_KBPS:
            if (rate > RTL8373_QOS_RATE_INPUT_MAX_HSG)
                return RT_ERR_RATE ;

            if((retVal = rtl8373_setAsicReg(RTL8373_SHARED_METER_RATE_CTRL_ADDR(index), rate)) != RT_ERR_OK)
                return retVal;        

            break;
        case METER_TYPE_PPS:
            if (rate > RTL8373_QOS_PPS_INPUT_MAX)
                return RT_ERR_RATE ;

            if((retVal = rtl8373_setAsicReg(RTL8373_SHARED_METER_RATE_CTRL_ADDR(index), rate)) != RT_ERR_OK)
                return retVal;

            break;
        default:
            return RT_ERR_INPUT;
    }

    /*Set IFG */
    if((retVal = rtl8373_setAsicRegBit(RTL8373_SHARED_METER_IPG_CTRL_ADDR(index), RTL8373_SHARED_METER_IPG_CTRL_IPG_CNTR_OFFSET(index), ifg_include)) != RT_ERR_OK)
                return retVal;

    /* Set Type */
    if ((retVal = rtl8373_setAsicRegBit(RTL8373_SHARED_METER_MODE_ADDR(index), RTL8373_SHARED_METER_MODE_LB_MODE_OFFSET(index), (rtk_uint32)type)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_rate_shareMeter_get
 * Description:
 *      Get meter configuration
 * Input:
 *      index        - shared meter index(0 - 63)
 * Output:
 *      pType        - Meter Type
 *      pRate        - pointer of rate of share meter
 *      pIfg_include - include IFG or not, ENABLE:include DISABLE:exclude
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_FILTER_METER_ID  - Invalid meter
 * Note:
 *
 */
rtk_api_ret_t dal_rtl8373_rate_shareMeter_get(rtk_meter_id_t index, rtk_meter_type_t *pType, rtk_rate_t *pRate, rtk_enable_t *pIfg_include)
{
    rtk_api_ret_t retVal;
    rtk_uint32 regData;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (index > RTL8373_METERMAX)
        return RT_ERR_FILTER_METER_ID;

    if(NULL == pType)
        return RT_ERR_NULL_POINTER;

    if(NULL == pRate)
        return RT_ERR_NULL_POINTER;

    if(NULL == pIfg_include)
        return RT_ERR_NULL_POINTER;

    /* 19-bits Rate */
     if((retVal = rtl8373_getAsicReg(RTL8373_SHARED_METER_RATE_CTRL_ADDR(index), pRate)) != RT_ERR_OK)
         return retVal;

    /* IFG */
    if((retVal = rtl8373_getAsicRegBit(RTL8373_SHARED_METER_IPG_CTRL_ADDR(index), RTL8373_SHARED_METER_IPG_CTRL_IPG_CNTR_OFFSET(index), pIfg_include)) != RT_ERR_OK)
        return retVal;

    *pIfg_include = (regData == 1) ? ENABLED : DISABLED;

    /* Type */
    if ((retVal = rtl8373_getAsicRegBit(RTL8373_SHARED_METER_MODE_ADDR(index), RTL8373_SHARED_METER_MODE_LB_MODE_OFFSET(index), &regData)) != RT_ERR_OK)
        return retVal;

    *pType = (regData == 0) ? METER_TYPE_KBPS : METER_TYPE_PPS;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_rate_shareMeterBucket_set
 * Description:
 *      Set meter Bucket Size
 * Input:
 *      index        - shared meter index(0 - 63)
 *      bucket_size  - Bucket Size
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_INPUT            - Error Input
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_FILTER_METER_ID  - Invalid meter
 * Note:
 *      The API can set shared meter bucket size.
 */
rtk_api_ret_t dal_rtl8373_rate_shareMeterBucket_set(rtk_meter_id_t index, rtk_uint32 bucket_size)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (index > RTL8373_METERMAX)
        return RT_ERR_FILTER_METER_ID;

    if(bucket_size > RTL8373_METERBUCKETSIZEMAX)
        return RT_ERR_INPUT;

    if ((retVal = rtl8373_setAsicReg(RTL8373_SHARED_METER_BURST_CTRL_ADDR(index), bucket_size)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_rate_shareMeterBucket_get
 * Description:
 *      Get meter Bucket Size
 * Input:
 *      index        - shared meter index(0 - 63)
 * Output:
 *      pBucket_size - Bucket Size
 * Return:
 *      RT_ERR_OK               - OK
 *      RT_ERR_FAILED           - Failed
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_FILTER_METER_ID  - Invalid meter
 * Note:
 *      The API can get shared meter bucket size.
 */
rtk_api_ret_t dal_rtl8373_rate_shareMeterBucket_get(rtk_meter_id_t index, rtk_uint32 *pBucket_size)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (index > RTL8373_METERMAX)
        return RT_ERR_FILTER_METER_ID;

    if(NULL == pBucket_size)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8373_getAsicReg(RTL8373_SHARED_METER_BURST_CTRL_ADDR(index), pBucket_size)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_rate_shareMeterExceedStatus_set
 * Description:
 *      Clear shared meter status
 * Input:
 *      index       - share meter index (0 - 63)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - Success
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_FILTER_METER_ID  - Invalid meter
 * Note:
 *      None
 */
ret_t dal_rtl8373_rate_shareMeterExceedStatus_set(rtk_uint32 index)
{
    if(index > RTL8373_METERMAX)
        return RT_ERR_FILTER_METER_ID;

    return rtl8373_setAsicRegBit(RTL8373_SHARED_METER_EXCEED_ADDR(index), RTL8373_SHARED_METER_EXCEED_LB_EXCEED_OFFSET(index), 1);

}

/* Function Name:
 *      dal_rtl8373_rate_shareMeterExceedStatus_get
 * Description:
 *      Get shared meter status
 * Input:
 *      index   - share meter index (0-63)
 *      pStatus     - 0: rate doesn't exceed    1: rate exceeds
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - Success
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_FILTER_METER_ID  - Invalid meter
 * Note:
 *      If rate is over rate*16Kbps of a meter, the state bit of this meter is set to 1.
 */
ret_t dal_rtl8373_rate_shareMeterExceedStatus_get(rtk_uint32 index, rtk_uint32* pStatus)
{
    if(index > RTL8373_METERMAX)
        return RT_ERR_FILTER_METER_ID;

    return rtl8373_getAsicRegBit(RTL8373_SHARED_METER_EXCEED_ADDR(index), RTL8373_SHARED_METER_EXCEED_LB_EXCEED_OFFSET(index), pStatus);
}

/* Function Name:
 *      dal_rtl8373_rate_shareMeterICPUExceedStatus_set
 * Description:
 *      Clear shared meter ICPU status
 * Input:
 *      index       - meter index (0 - 63)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - Success
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_FILTER_METER_ID  - Invalid meter
 * Note:
 *      None
 */
ret_t dal_rtl8373_rate_shareMeterICPUExceedStatus_set(rtk_uint32 index)
{
    if(index > RTL8373_METERMAX)
        return RT_ERR_FILTER_METER_ID;

    return rtl8373_setAsicRegBit(RTL8373_SHARED_METER_EXCEED_ICPU_ADDR(index), RTL8373_SHARED_METER_EXCEED_ICPU_LB_EXCEED_ICPU_OFFSET(index), 1);

}

/* Function Name:
 *      dal_rtl8373_rate_shareMeterICPUExceedStatus_get
 * Description:
 *      Get shared meter ICPU exceed status
 * Input:
 *      index   -  meter index (0 - 63)
 *      pStatus     - 0: rate doesn't exceed    1: rate exceeds
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK               - Success
 *      RT_ERR_SMI              - SMI access error
 *      RT_ERR_FILTER_METER_ID  - Invalid meter
 * Note:
 *      If rate is over rate*16Kbps of a meter, the state bit of this meter is set to 1.
 */
ret_t dal_rtl8373_rate_shareMeterICPUExceedStatus_get(rtk_uint32 index, rtk_uint32* pStatus)
{
    if(index > RTL8373_METERMAX)
        return RT_ERR_FILTER_METER_ID;

    return rtl8373_getAsicRegBit(RTL8373_SHARED_METER_EXCEED_ICPU_ADDR(index), RTL8373_SHARED_METER_EXCEED_ICPU_LB_EXCEED_ICPU_OFFSET(index), pStatus);
}

#if 0
/* Function Name:
 *      dal_rtl8373_rate_igrBandwidthCtrlRate_set
 * Description:
 *      Set port ingress bandwidth control
 * Input:
 *      port        - Port id
 *      rate        - Rate of share meter
 *      ifg_include - include IFG or not, ENABLE:include DISABLE:exclude
 *      fc_enable   - enable flow control or not, ENABLE:use flow control DISABLE:drop
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_ENABLE       - Invalid IFG parameter.
 *      RT_ERR_INBW_RATE    - Invalid ingress rate parameter.
 * Note:
 *      The rate unit is 1 kbps and the range is from 8k to 1048568k. The granularity of rate is 8 kbps.
 *      The ifg_include parameter is used for rate calculation with/without inter-frame-gap and preamble.
 */
rtk_api_ret_t dal_rtl8373_rate_igrBandwidthCtrlRate_set(rtk_port_t port, rtk_rate_t rate, rtk_enable_t ifg_include, rtk_enable_t fc_enable)
{
    rtk_api_ret_t retVal;
    rtk_uint32 regAddr, regData;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);

    if(ifg_include >= RTK_ENABLE_END)
        return RT_ERR_INPUT;

    if(fc_enable >= RTK_ENABLE_END)
        return RT_ERR_INPUT;

    if(rtk_switch_isHsgPort(port) == RT_ERR_OK)
    {
        if (rate > rtl8373_QOS_RATE_INPUT_MAX_HSG)
            return RT_ERR_QOS_EBW_RATE ;
    }
    else
    {
        if (rate > rtl8373_QOS_RATE_INPUT_MAX)
            return RT_ERR_QOS_EBW_RATE ;
    }

    regAddr = rtl8373_REG_INGRESSBW_PORT0_RATE_CTRL0 + (rtk_switch_port_L2P_get(port) * 0x20);
    regData = (rate >> 3) & 0xFFFF;
    if((retVal = rtl8373_setAsicReg(regAddr, regData)) != RT_ERR_OK)
        return retVal;

    regAddr += 1;
    regData = ((rate >> 3) & 0x70000) >> 16;
    if((retVal = rtl8373_setAsicRegBits(regAddr, rtl8373_INGRESSBW_PORT0_RATE_CTRL1_INGRESSBW_RATE16_MASK, regData)) != RT_ERR_OK)
        return retVal;

    regAddr = rtl8373_REG_PORT0_MISC_CFG + (rtk_switch_port_L2P_get(port) * 0x20);
    if((retVal = rtl8373_setAsicRegBit(regAddr, rtl8373_PORT0_MISC_CFG_INGRESSBW_IFG_OFFSET, (ifg_include == ENABLED) ? 1 : 0)) != RT_ERR_OK)
        return retVal;

    regAddr = rtl8373_REG_PORT0_MISC_CFG + (rtk_switch_port_L2P_get(port) * 0x20);
    if((retVal = rtl8373_setAsicRegBit(regAddr, rtl8373_PORT0_MISC_CFG_INGRESSBW_FLOWCTRL_OFFSET, (fc_enable == ENABLED) ? 1 : 0)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_rate_igrBandwidthCtrlRate_get
 * Description:
 *      Get port ingress bandwidth control
 * Input:
 *      port - Port id
 * Output:
 *      pRate           - Rate of share meter
 *      pIfg_include    - Rate's calculation including IFG, ENABLE:include DISABLE:exclude
 *      pFc_enable      - enable flow control or not, ENABLE:use flow control DISABLE:drop
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *     The rate unit is 1 kbps and the range is from 8k to 1048568k. The granularity of rate is 8 kbps.
 *     The ifg_include parameter is used for rate calculation with/without inter-frame-gap and preamble.
 */
rtk_api_ret_t dal_rtl8373_rate_igrBandwidthCtrlRate_get(rtk_port_t port, rtk_rate_t *pRate, rtk_enable_t *pIfg_include, rtk_enable_t *pFc_enable)
{
    rtk_api_ret_t retVal;
    rtk_uint32 regAddr, regData;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);

    if(NULL == pIfg_include)
        return RT_ERR_NULL_POINTER;

    if(NULL == pFc_enable)
        return RT_ERR_NULL_POINTER;

    regAddr = rtl8373_REG_INGRESSBW_PORT0_RATE_CTRL0 + (rtk_switch_port_L2P_get(port) * 0x20);
    if((retVal = rtl8373_getAsicReg(regAddr, &regData)) != RT_ERR_OK)
        return retVal;

    *pRate = regData;

    regAddr += 1;
    if((retVal = rtl8373_getAsicRegBits(regAddr, rtl8373_INGRESSBW_PORT0_RATE_CTRL1_INGRESSBW_RATE16_MASK, &regData)) != RT_ERR_OK)
        return retVal;

    *pRate |= (regData << 16);
    *pRate = (*pRate << 3);

    regAddr =  rtl8373_REG_PORT0_MISC_CFG + (rtk_switch_port_L2P_get(port) * 0x20);
    if((retVal = rtl8373_getAsicRegBit(regAddr, rtl8373_PORT0_MISC_CFG_INGRESSBW_IFG_OFFSET, &regData)) != RT_ERR_OK)
        return retVal;

    *pIfg_include = (regData == 1) ? ENABLED : DISABLED;

    regAddr =  rtl8373_REG_PORT0_MISC_CFG + (rtk_switch_port_L2P_get(port) * 0x20);
    if((retVal = rtl8373_getAsicRegBit(regAddr, rtl8373_PORT0_MISC_CFG_INGRESSBW_FLOWCTRL_OFFSET, &regData)) != RT_ERR_OK)
        return retVal;

    *pFc_enable = (regData == 1) ? ENABLED : DISABLED;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_rate_egrBandwidthCtrlRate_set
 * Description:
 *      Set port egress bandwidth control
 * Input:
 *      port        - Port id
 *      rate        - Rate of egress bandwidth
 *      ifg_include - include IFG or not, ENABLE:include DISABLE:exclude
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 *      RT_ERR_QOS_EBW_RATE - Invalid egress bandwidth/rate
 * Note:
 *     The rate unit is 1 kbps and the range is from 8k to 1048568k. The granularity of rate is 8 kbps.
 *     The ifg_include parameter is used for rate calculation with/without inter-frame-gap and preamble.
 */
rtk_api_ret_t dal_rtl8373_rate_egrBandwidthCtrlRate_set( rtk_port_t port, rtk_rate_t rate,  rtk_enable_t ifg_include)
{
    rtk_api_ret_t retVal;
    rtk_uint32 regAddr, regData;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);

    if(rtk_switch_isHsgPort(port) == RT_ERR_OK)
    {
        if (rate > rtl8373_QOS_RATE_INPUT_MAX_HSG)
            return RT_ERR_QOS_EBW_RATE ;
    }
    else
    {
        if (rate > rtl8373_QOS_RATE_INPUT_MAX)
            return RT_ERR_QOS_EBW_RATE ;
    }

    if (ifg_include >= RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    regAddr = rtl8373_REG_PORT0_EGRESSBW_CTRL0 + (rtk_switch_port_L2P_get(port) * 2);
    regData = (rate >> 3) & 0xFFFF;

    if((retVal = rtl8373_setAsicReg(regAddr, regData)) != RT_ERR_OK)
        return retVal;

    if(rtk_switch_isHsgPort(port) == RT_ERR_OK)
    {
        regAddr = rtl8373_REG_PORT0_EGRESSBW_CTRL1 + (rtk_switch_port_L2P_get(port) * 2);
        regData = ((rate >> 3) & 0x70000) >> 16;

        if((retVal = rtl8373_setAsicRegBits(regAddr, rtl8373_PORT6_EGRESSBW_CTRL1_MASK, regData)) != RT_ERR_OK)
            return retVal;
    }
    else
    {
        regAddr = rtl8373_REG_PORT0_EGRESSBW_CTRL1 + (rtk_switch_port_L2P_get(port) * 2);
        regData = ((rate >> 3) & 0x10000) >> 16;

        if((retVal = rtl8373_setAsicRegBits(regAddr, rtl8373_PORT0_EGRESSBW_CTRL1_MASK, regData)) != RT_ERR_OK)
            return retVal;
    }

    if((retVal = rtl8373_setAsicRegBit(rtl8373_REG_SCHEDULE_WFQ_CTRL, rtl8373_SCHEDULE_WFQ_CTRL_OFFSET, (ifg_include == ENABLED) ? 1 : 0)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_rate_egrBandwidthCtrlRate_get
 * Description:
 *      Get port egress bandwidth control
 * Input:
 *      port - Port id
 * Output:
 *      pRate           - Rate of egress bandwidth
 *      pIfg_include    - Rate's calculation including IFG, ENABLE:include DISABLE:exclude
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *     The rate unit is 1 kbps and the range is from 8k to 1048568k. The granularity of rate is 8 kbps.
 *     The ifg_include parameter is used for rate calculation with/without inter-frame-gap and preamble.
 */
rtk_api_ret_t dal_rtl8373_rate_egrBandwidthCtrlRate_get(rtk_port_t port, rtk_rate_t *pRate, rtk_enable_t *pIfg_include)
{
    rtk_api_ret_t retVal;
    rtk_uint32 regAddr, regData, regData2;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);

    if(NULL == pRate)
        return RT_ERR_NULL_POINTER;

    if(NULL == pIfg_include)
        return RT_ERR_NULL_POINTER;

    regAddr = rtl8373_REG_PORT0_EGRESSBW_CTRL0 + (rtk_switch_port_L2P_get(port) * 2);
    if((retVal = rtl8373_getAsicReg(regAddr, &regData)) != RT_ERR_OK)
        return retVal;

    regAddr = rtl8373_REG_PORT0_EGRESSBW_CTRL1 + (rtk_switch_port_L2P_get(port) * 2);
    retVal = rtl8373_getAsicRegBits(regAddr, rtl8373_PORT6_EGRESSBW_CTRL1_MASK, &regData2);
    if(retVal != RT_ERR_OK)
        return retVal;

    *pRate = ((regData | (regData2 << 16)) << 3);

    if((retVal = rtl8373_getAsicRegBit(rtl8373_REG_SCHEDULE_WFQ_CTRL, rtl8373_SCHEDULE_WFQ_CTRL_OFFSET, &regData)) != RT_ERR_OK)
        return retVal;

    *pIfg_include = (regData == 1) ? ENABLED : DISABLED;
    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_rate_egrQueueBwCtrlEnable_get
 * Description:
 *      Get enable status of egress bandwidth control on specified queue.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      queue   - queue id
 * Output:
 *      pEnable - Pointer to enable status of egress queue bandwidth control
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_QUEUE_ID         - invalid queue id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
rtk_api_ret_t dal_rtl8373_rate_egrQueueBwCtrlEnable_get(rtk_port_t port, rtk_qid_t queue, rtk_enable_t *pEnable)
{
    rtk_api_ret_t retVal;
    rtk_uint32 aprEnable;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);

    /*for whole port function, the queue value should be 0xFF*/
    if (queue != RTK_WHOLE_SYSTEM)
        return RT_ERR_QUEUE_ID;

    if(NULL == pEnable)
        return RT_ERR_NULL_POINTER;

    if((retVal = rtl8373_getAsicRegBit(rtl8373_REG_SCHEDULE_APR_CTRL0, rtk_switch_port_L2P_get(port), &aprEnable)) != RT_ERR_OK)
        return retVal;

    *pEnable = (aprEnable == 1) ? ENABLED : DISABLED;
    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_rate_egrQueueBwCtrlEnable_set
 * Description:
 *      Set enable status of egress bandwidth control on specified queue.
 * Input:
 *      port   - port id
 *      queue  - queue id
 *      enable - enable status of egress queue bandwidth control
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_QUEUE_ID         - invalid queue id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
rtk_api_ret_t dal_rtl8373_rate_egrQueueBwCtrlEnable_set(rtk_port_t port, rtk_qid_t queue, rtk_enable_t enable)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);

    /*for whole port function, the queue value should be 0xFF*/
    if (queue != RTK_WHOLE_SYSTEM)
        return RT_ERR_QUEUE_ID;

    if (enable>=RTK_ENABLE_END)
        return RT_ERR_INPUT;

    if((retVal = rtl8373_setAsicRegBit(rtl8373_REG_SCHEDULE_APR_CTRL0, rtk_switch_port_L2P_get(port), (enable == ENABLED) ? 1 : 0)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_rate_egrQueueBwCtrlRate_get
 * Description:
 *      Get rate of egress bandwidth control on specified queue.
 * Input:
 *      port  - port id
 *      queue - queue id
 *      pIndex - shared meter index
 * Output:
 *      pRate - pointer to rate of egress queue bandwidth control
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_QUEUE_ID         - invalid queue id
 *      RT_ERR_FILTER_METER_ID  - Invalid meter id
 * Note:
 *    The actual rate control is set in shared meters.
 *    The unit of granularity is 8Kbps.
 */
rtk_api_ret_t dal_rtl8373_rate_egrQueueBwCtrlRate_get(rtk_port_t port, rtk_qid_t queue, rtk_meter_id_t *pIndex)
{
    rtk_api_ret_t retVal;
    rtk_uint32 apridx;
    rtk_uint32 phy_port;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);

    if (queue >= rtl8373_MAX_NUM_OF_QUEUE)
        return RT_ERR_QUEUE_ID;

    if(NULL == pIndex)
        return RT_ERR_NULL_POINTER;

    phy_port = rtk_switch_port_L2P_get(port);
    if((retVal = rtl8373_getAsicRegBits(rtl8373_SCHEDULE_PORT_APR_METER_REG(phy_port, queue), rtl8373_SCHEDULE_PORT_APR_METER_MASK(phy_port), &apridx)) != RT_ERR_OK)
        return retVal;

    *pIndex = apridx + ((rtk_switch_port_L2P_get(port) % 4) * 8);
     return RT_ERR_OK;
}

/* Function Name:
 *      dal_rtl8373_rate_egrQueueBwCtrlRate_set
 * Description:
 *      Set rate of egress bandwidth control on specified queue.
 * Input:
 *      port  - port id
 *      queue - queue id
 *      index - shared meter index
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_QUEUE_ID         - invalid queue id
 *      RT_ERR_FILTER_METER_ID  - Invalid meter id
 * Note:
 *    The actual rate control is set in shared meters.
 *    The unit of granularity is 8Kbps.
 */
rtk_api_ret_t dal_rtl8373_rate_egrQueueBwCtrlRate_set(rtk_port_t port, rtk_qid_t queue, rtk_meter_id_t index)
{
    rtk_api_ret_t retVal;
    rtk_uint32 offset_idx;
    rtk_uint32 phy_port;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);

    if (queue >= rtl8373_MAX_NUM_OF_QUEUE)
        return RT_ERR_QUEUE_ID;

    if (index > RTK_MAX_METER_ID)
        return RT_ERR_FILTER_METER_ID;

    phy_port = rtk_switch_port_L2P_get(port);
    if (index < ((phy_port % 4) * 8) ||  index > (7 + ((phy_port % 4) * 8)))
        return RT_ERR_FILTER_METER_ID;

    offset_idx = index - ((phy_port % 4) * 8);

    if ((retVal = rtl8373_setAsicRegBits(rtl8373_SCHEDULE_PORT_APR_METER_REG(phy_port, queue), rtl8373_SCHEDULE_PORT_APR_METER_MASK(queue), offset_idx))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

#endif




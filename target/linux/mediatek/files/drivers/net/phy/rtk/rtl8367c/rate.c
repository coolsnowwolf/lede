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
 * Feature : Here is a list of all functions and variables in rate module.
 *
 */

#include <rtk_switch.h>
#include <rtk_error.h>
#include <rate.h>
#include <qos.h>
#include <string.h>

#include <rtl8367c_asicdrv.h>
#include <rtl8367c_asicdrv_meter.h>
#include <rtl8367c_asicdrv_inbwctrl.h>
#include <rtl8367c_asicdrv_scheduling.h>

/* Function Name:
 *      rtk_rate_shareMeter_set
 * Description:
 *      Set meter configuration
 * Input:
 *      index       - shared meter index
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
 *      The rate unit is 1 kbps and the range is from 8k to 1048568k if type is METER_TYPE_KBPS and
 *      the granularity of rate is 8 kbps.
 *      The rate unit is packets per second and the range is 1 ~ 0x1FFF if type is METER_TYPE_PPS.
 *      The ifg_include parameter is used
 *      for rate calculation with/without inter-frame-gap and preamble.
 */
rtk_api_ret_t rtk_rate_shareMeter_set(rtk_meter_id_t index, rtk_meter_type_t type, rtk_rate_t rate, rtk_enable_t ifg_include)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (index > RTK_MAX_METER_ID)
        return RT_ERR_FILTER_METER_ID;

    if (type >= METER_TYPE_END)
        return RT_ERR_INPUT;

    if (ifg_include >= RTK_ENABLE_END)
        return RT_ERR_INPUT;

    switch (type)
    {
        case METER_TYPE_KBPS:
            if (rate > RTL8367C_QOS_RATE_INPUT_MAX_HSG || rate < RTL8367C_QOS_RATE_INPUT_MIN)
                return RT_ERR_RATE ;

            if ((retVal = rtl8367c_setAsicShareMeter(index, rate >> 3, ifg_include)) != RT_ERR_OK)
                return retVal;

            break;
        case METER_TYPE_PPS:
            if (rate > RTL8367C_QOS_PPS_INPUT_MAX || rate < RTL8367C_QOS_PPS_INPUT_MIN)
                return RT_ERR_RATE ;

            if ((retVal = rtl8367c_setAsicShareMeter(index, rate, ifg_include)) != RT_ERR_OK)
                return retVal;

            break;
        default:
            return RT_ERR_INPUT;
    }

    /* Set Type */
    if ((retVal = rtl8367c_setAsicShareMeterType(index, (rtk_uint32)type)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_rate_shareMeter_get
 * Description:
 *      Get meter configuration
 * Input:
 *      index        - shared meter index
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
rtk_api_ret_t rtk_rate_shareMeter_get(rtk_meter_id_t index, rtk_meter_type_t *pType, rtk_rate_t *pRate, rtk_enable_t *pIfg_include)
{
    rtk_api_ret_t retVal;
    rtk_uint32 regData;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (index > RTK_MAX_METER_ID)
        return RT_ERR_FILTER_METER_ID;

    if(NULL == pType)
        return RT_ERR_NULL_POINTER;

    if(NULL == pRate)
        return RT_ERR_NULL_POINTER;

    if(NULL == pIfg_include)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getAsicShareMeter(index, &regData, pIfg_include)) != RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8367c_getAsicShareMeterType(index, (rtk_uint32 *)pType)) != RT_ERR_OK)
        return retVal;

    if(*pType == METER_TYPE_KBPS)
        *pRate = regData<<3;
    else
        *pRate = regData;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_rate_shareMeterBucket_set
 * Description:
 *      Set meter Bucket Size
 * Input:
 *      index        - shared meter index
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
rtk_api_ret_t rtk_rate_shareMeterBucket_set(rtk_meter_id_t index, rtk_uint32 bucket_size)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (index > RTK_MAX_METER_ID)
        return RT_ERR_FILTER_METER_ID;

    if(bucket_size > RTL8367C_METERBUCKETSIZEMAX)
        return RT_ERR_INPUT;

    if ((retVal = rtl8367c_setAsicShareMeterBucketSize(index, bucket_size)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_rate_shareMeterBucket_get
 * Description:
 *      Get meter Bucket Size
 * Input:
 *      index        - shared meter index
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
rtk_api_ret_t rtk_rate_shareMeterBucket_get(rtk_meter_id_t index, rtk_uint32 *pBucket_size)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    if (index > RTK_MAX_METER_ID)
        return RT_ERR_FILTER_METER_ID;

    if(NULL == pBucket_size)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getAsicShareMeterBucketSize(index, pBucket_size)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_rate_igrBandwidthCtrlRate_set
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
rtk_api_ret_t rtk_rate_igrBandwidthCtrlRate_set(rtk_port_t port, rtk_rate_t rate, rtk_enable_t ifg_include, rtk_enable_t fc_enable)
{
    rtk_api_ret_t retVal;

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
        if ((rate > RTL8367C_QOS_RATE_INPUT_MAX_HSG) || (rate < RTL8367C_QOS_RATE_INPUT_MIN))
            return RT_ERR_QOS_EBW_RATE ;
    }
    else
    {
        if ((rate > RTL8367C_QOS_RATE_INPUT_MAX) || (rate < RTL8367C_QOS_RATE_INPUT_MIN))
            return RT_ERR_QOS_EBW_RATE ;
    }

    if ((retVal = rtl8367c_setAsicPortIngressBandwidth(rtk_switch_port_L2P_get(port), rate>>3, ifg_include,fc_enable)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_rate_igrBandwidthCtrlRate_get
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
rtk_api_ret_t rtk_rate_igrBandwidthCtrlRate_get(rtk_port_t port, rtk_rate_t *pRate, rtk_enable_t *pIfg_include, rtk_enable_t *pFc_enable)
{
    rtk_api_ret_t retVal;
    rtk_uint32 regData;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);

    if(NULL == pIfg_include)
        return RT_ERR_NULL_POINTER;

    if(NULL == pFc_enable)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getAsicPortIngressBandwidth(rtk_switch_port_L2P_get(port), &regData, pIfg_include, pFc_enable)) != RT_ERR_OK)
        return retVal;

    *pRate = regData<<3;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_rate_egrBandwidthCtrlRate_set
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
rtk_api_ret_t rtk_rate_egrBandwidthCtrlRate_set( rtk_port_t port, rtk_rate_t rate,  rtk_enable_t ifg_include)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);

    if(rtk_switch_isHsgPort(port) == RT_ERR_OK)
    {
        if ((rate > RTL8367C_QOS_RATE_INPUT_MAX_HSG) || (rate < RTL8367C_QOS_RATE_INPUT_MIN))
            return RT_ERR_QOS_EBW_RATE ;
    }
    else
    {
        if ((rate > RTL8367C_QOS_RATE_INPUT_MAX) || (rate < RTL8367C_QOS_RATE_INPUT_MIN))
            return RT_ERR_QOS_EBW_RATE ;
    }

    if (ifg_include >= RTK_ENABLE_END)
        return RT_ERR_ENABLE;

    if ((retVal = rtl8367c_setAsicPortEgressRate(rtk_switch_port_L2P_get(port), rate>>3)) != RT_ERR_OK)
        return retVal;

    if ((retVal = rtl8367c_setAsicPortEgressRateIfg(ifg_include)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_rate_egrBandwidthCtrlRate_get
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
rtk_api_ret_t rtk_rate_egrBandwidthCtrlRate_get(rtk_port_t port, rtk_rate_t *pRate, rtk_enable_t *pIfg_include)
{
    rtk_api_ret_t retVal;
    rtk_uint32 regData;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);

    if(NULL == pRate)
        return RT_ERR_NULL_POINTER;

    if(NULL == pIfg_include)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getAsicPortEgressRate(rtk_switch_port_L2P_get(port), &regData)) != RT_ERR_OK)
        return retVal;

    *pRate = regData << 3;

    if ((retVal = rtl8367c_getAsicPortEgressRateIfg((rtk_uint32*)pIfg_include)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}


/* Function Name:
 *      rtk_rate_egrQueueBwCtrlEnable_get
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
rtk_api_ret_t rtk_rate_egrQueueBwCtrlEnable_get(rtk_port_t port, rtk_qid_t queue, rtk_enable_t *pEnable)
{
    rtk_api_ret_t retVal;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);

    /*for whole port function, the queue value should be 0xFF*/
    if (queue != RTK_WHOLE_SYSTEM)
        return RT_ERR_QUEUE_ID;

    if(NULL == pEnable)
        return RT_ERR_NULL_POINTER;

    if ((retVal = rtl8367c_getAsicAprEnable(rtk_switch_port_L2P_get(port),pEnable))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_rate_egrQueueBwCtrlEnable_set
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
rtk_api_ret_t rtk_rate_egrQueueBwCtrlEnable_set(rtk_port_t port, rtk_qid_t queue, rtk_enable_t enable)
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

    if ((retVal = rtl8367c_setAsicAprEnable(rtk_switch_port_L2P_get(port), enable))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_rate_egrQueueBwCtrlRate_get
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
rtk_api_ret_t rtk_rate_egrQueueBwCtrlRate_get(rtk_port_t port, rtk_qid_t queue, rtk_meter_id_t *pIndex)
{
    rtk_api_ret_t retVal;
    rtk_uint32 offset_idx;
    rtk_uint32 phy_port;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);

    if (queue >= RTK_MAX_NUM_OF_QUEUE)
        return RT_ERR_QUEUE_ID;

    if(NULL == pIndex)
        return RT_ERR_NULL_POINTER;

    phy_port = rtk_switch_port_L2P_get(port);
    if ((retVal=rtl8367c_getAsicAprMeter(phy_port, queue,&offset_idx))!=RT_ERR_OK)
        return retVal;

    *pIndex = offset_idx + ((phy_port%4)*8);

     return RT_ERR_OK;
}


/* Function Name:
 *      rtk_rate_egrQueueBwCtrlRate_set
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
rtk_api_ret_t rtk_rate_egrQueueBwCtrlRate_set(rtk_port_t port, rtk_qid_t queue, rtk_meter_id_t index)
{
    rtk_api_ret_t retVal;
    rtk_uint32 offset_idx;
    rtk_uint32 phy_port;

    /* Check initialization state */
    RTK_CHK_INIT_STATE();

    /* Check Port Valid */
    RTK_CHK_PORT_VALID(port);

    if (queue >= RTK_MAX_NUM_OF_QUEUE)
        return RT_ERR_QUEUE_ID;

    if (index > RTK_MAX_METER_ID)
        return RT_ERR_FILTER_METER_ID;

    phy_port = rtk_switch_port_L2P_get(port);
    if (index < ((phy_port%4)*8) ||  index > (7 + (phy_port%4)*8))
        return RT_ERR_FILTER_METER_ID;

    offset_idx = index - ((phy_port%4)*8);

    if ((retVal=rtl8367c_setAsicAprMeter(phy_port,queue,offset_idx))!=RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;
}


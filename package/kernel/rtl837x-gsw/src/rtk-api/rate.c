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
 * Feature : Here is a list of all functions and variables in ingress bandwitdh and egress bandwitdh module.
 *
 */

#include <rtk_switch.h>
#include <rtk_error.h>
#include <rate.h>
#include <string.h>

#include <dal/dal_mgmt.h>

/* Function Name:
 *      rtk_rate_igrBwCtrlPortEn_set
 * Description:
 *      Enable or disable port ingress bandwidth control 
 * Input:
 *      port        - Port id
 *      bwEn        - enable ingress bandwidth control or not
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
 *     
 */
rtk_api_ret_t rtk_rate_igrBwCtrlPortEn_set(rtk_port_t port, rtk_enable_t bwEn)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->rate_igrBwCtrlPortEn_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->rate_igrBwCtrlPortEn_set(port, bwEn);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_rate_igrBwCtrlPortEn_get
 * Description:
 *      Enable or disable port ingress bandwidth control 
 * Input:
 *      port        - Port id
 *
 * Output:
 *      pBwEn        - Port ingress bandwidth control state
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_ENABLE       - Invalid IFG parameter.
 *      RT_ERR_INBW_RATE    - Invalid ingress rate parameter.
 * Note:
 *     
 */
rtk_api_ret_t rtk_rate_igrBwCtrlPortEn_get(rtk_port_t port, rtk_enable_t *pBwEn)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->rate_igrBwCtrlPortEn_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->rate_igrBwCtrlPortEn_get(port, pBwEn);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_rate_igrBwCtrlRate_set
 * Description:
 *      Set port ingress bandwidth control rate and FC config
 * Input:
 *      port        - Port id
 *      rate        - Rate of share meter (uint: kpbs)
 *      fcEn	    - enable flow control or not, ENABLE:use flow control  DISABLE:drop
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
 *      The rate unit is 16 kbps and the range is from 16k to 10G. The granularity of rate is 16 kbps.
 *      The ifg_include parameter is used for rate calculation with/without inter-frame-gap and preamble.
 */
rtk_api_ret_t rtk_rate_igrBwCtrlRate_set(rtk_port_t port, rtk_rate_t rate, rtk_enable_t fcEn)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->rate_igrBwCtrlRate_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->rate_igrBwCtrlRate_set(port, rate, fcEn);
    RTK_API_UNLOCK();

    return retVal;	
}

/* Function Name:
 *      rtk_rate_igrBwCtrlRate_get
 * Description:
 *      Get port ingress bandwidth control rate and FC config
 * Input:
 *      port        - Port id
 *
 * Output:
 *      pRate        - Rate of share meter(uint: kpbs)
 *      pFcEn        - enable flow control or not, ENABLE:use flow control DISABLE:drop
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_ENABLE       - Invalid IFG parameter.
 *      RT_ERR_INBW_RATE    - Invalid ingress rate parameter.
 * Note:
 *      The rate unit is 16 kbps and the range is from 16k to 10G. The granularity of rate is 16 kbps.
 *      The ifg_include parameter is used for rate calculation with/without inter-frame-gap and preamble.
 */
rtk_api_ret_t rtk_rate_igrBwCtrlRate_get(rtk_port_t port, rtk_rate_t *pRate, rtk_enable_t *pFcEn)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->rate_igrBwCtrlRate_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->rate_igrBwCtrlRate_get(port, pRate, pFcEn);
    RTK_API_UNLOCK();

    return retVal;	
}

/* Function Name:
 *      rtk_rate_igrBwCtrlIfg_set
 * Description:
 *      Set ingress bandwidth control include Preamble and IFG or not
 * Input:
 *      ifgInclude - include or not, ENABLE:include DISABLE:exclude
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_ENABLE       - Invalid IFG parameter.
 *      RT_ERR_INBW_RATE    - Invalid ingress rate parameter.
 * Note:
 *
 */
rtk_api_ret_t rtk_rate_igrBwCtrlIfg_set(rtk_enable_t ifgInclude)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->rate_igrBwCtrlIfg_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->rate_igrBwCtrlIfg_set(ifgInclude);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_rate_igrBwCtrlIfg_get
 * Description:
 *      Get ingress bandwidth control include 8B Preamble and 12B IFG or not
 * Input:
 *      None
 * Output:
 *      ifgInclude - include or not, ENABLE:include DISABLE:exclude
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_ENABLE       - Invalid IFG parameter.
 *      RT_ERR_INBW_RATE    - Invalid ingress rate parameter.
 * Note:
 *
 */
rtk_api_ret_t rtk_rate_igrBwCtrlIfg_get(rtk_enable_t *pIfgInclude)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->rate_igrBwCtrlIfg_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->rate_igrBwCtrlIfg_get(pIfgInclude);
    RTK_API_UNLOCK();

    return retVal;
}

/* Function Name:
 *      rtk_rate_igrBwCtrlCongestSts_get
 * Description:
 *      Get port_n ingress bandwidth exceed leaky bucket high-on or not
 * Input:
 *      port        - port Idx
 * Output:
 *      pCongestSts - Indicate ingress bandwidth exceed Pn_IGR_LB_ HIGH _ON for port n.
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_ENABLE       - Invalid IFG parameter.
 *      RT_ERR_INBW_RATE    - Invalid ingress rate parameter.
 * Note:
 *
 */
rtk_api_ret_t rtk_rate_igrBwCtrlCongestSts_get(rtk_port_t port, rtk_rate_igrBwCongestSts_t *pCongestSts)
{
    rtk_api_ret_t retVal;

    if (NULL == RT_MAPPER->rate_igrBwCtrlCongestSts_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->rate_igrBwCtrlCongestSts_get(port, pCongestSts);
    RTK_API_UNLOCK();

    return retVal;
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

    if (NULL == RT_MAPPER->rate_egrBandwidthCtrlRate_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->rate_egrBandwidthCtrlRate_set(port, rate, ifg_include);
    RTK_API_UNLOCK();

    return retVal;
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

    if (NULL == RT_MAPPER->rate_egrBandwidthCtrlRate_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->rate_egrBandwidthCtrlRate_get(port, pRate, pIfg_include);
    RTK_API_UNLOCK();

    return retVal;
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

    if (NULL == RT_MAPPER->rate_egrQueueBwCtrlEnable_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->rate_egrQueueBwCtrlEnable_get(port, queue, pEnable);
    RTK_API_UNLOCK();

    return retVal;
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

    if (NULL == RT_MAPPER->rate_egrQueueBwCtrlEnable_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->rate_egrQueueBwCtrlEnable_set(port, queue, enable);
    RTK_API_UNLOCK();

    return retVal;
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

    if (NULL == RT_MAPPER->rate_egrQueueBwCtrlRate_get)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->rate_egrQueueBwCtrlRate_get(port, queue, pIndex);
    RTK_API_UNLOCK();

    return retVal;
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

    if (NULL == RT_MAPPER->rate_egrQueueBwCtrlRate_set)
        return RT_ERR_DRIVER_NOT_FOUND;

    RTK_API_LOCK();
    retVal = RT_MAPPER->rate_egrQueueBwCtrlRate_set(port, queue, index);
    RTK_API_UNLOCK();

    return retVal;
}


